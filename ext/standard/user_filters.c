/*
   +----------------------------------------------------------------------+
   | Copyright (c) The CRX Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the CRX license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | https://www.crx.net/license/3_01.txt                                 |
   | If you did not receive a copy of the CRX license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crx.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors:                                                             |
   | Wez Furlong (wez@thebrainroom.com)                                   |
   | Sara Golemon (pollita@crx.net)                                       |
   +----------------------------------------------------------------------+
*/

#include "crx.h"
#include "crx_globals.h"
#include "ext/standard/basic_functions.h"
#include "ext/standard/file.h"
#include "ext/standard/user_filters_arginfo.h"

#define CRX_STREAM_BRIGADE_RES_NAME	"userfilter.bucket brigade"
#define CRX_STREAM_BUCKET_RES_NAME "userfilter.bucket"
#define CRX_STREAM_FILTER_RES_NAME "userfilter.filter"

struct crx_user_filter_data {
	crex_class_entry *ce;
	/* variable length; this *must* be last in the structure */
	crex_string *classname;
};

/* to provide context for calling into the next filter from user-space */
static int le_bucket_brigade;
static int le_bucket;

/* define the base filter class */

CRX_METHOD(crx_user_filter, filter)
{
	zval *in, *out, *consumed;
	bool closing;
	if (crex_parse_parameters(CREX_NUM_ARGS(), "rrzb", &in, &out, &consumed, &closing) == FAILURE) {
		RETURN_THROWS();
	}

	RETURN_LONG(PSFS_ERR_FATAL);
}

CRX_METHOD(crx_user_filter, onCreate)
{
	CREX_PARSE_PARAMETERS_NONE();

	RETURN_TRUE;
}

CRX_METHOD(crx_user_filter, onClose)
{
	CREX_PARSE_PARAMETERS_NONE();
}

static crex_class_entry *user_filter_class_entry;

static CREX_RSRC_DTOR_FUNC(crx_bucket_dtor)
{
	crx_stream_bucket *bucket = (crx_stream_bucket *)res->ptr;
	if (bucket) {
		crx_stream_bucket_delref(bucket);
		bucket = NULL;
	}
}

CRX_MINIT_FUNCTION(user_filters)
{
	/* init the filter class ancestor */
	user_filter_class_entry = register_class_crx_user_filter();

	/* Filters will dispose of their brigades */
	le_bucket_brigade = crex_register_list_destructors_ex(NULL, NULL, CRX_STREAM_BRIGADE_RES_NAME, module_number);
	/* Brigades will dispose of their buckets */
	le_bucket = crex_register_list_destructors_ex(crx_bucket_dtor, NULL, CRX_STREAM_BUCKET_RES_NAME, module_number);

	if (le_bucket_brigade == FAILURE) {
		return FAILURE;
	}

	register_user_filters_symbols(module_number);

	return SUCCESS;
}

CRX_RSHUTDOWN_FUNCTION(user_filters)
{
	if (BG(user_filter_map)) {
		crex_hash_destroy(BG(user_filter_map));
		efree(BG(user_filter_map));
		BG(user_filter_map) = NULL;
	}

	return SUCCESS;
}

static void userfilter_dtor(crx_stream_filter *thisfilter)
{
	zval *obj = &thisfilter->abstract;
	zval retval;

	if (C_ISUNDEF_P(obj)) {
		/* If there's no object associated then there's nothing to dispose of */
		return;
	}

	crex_string *func_name = ZSTR_INIT_LITERAL("onclose", 0);
	crex_call_method_if_exists(C_OBJ_P(obj), func_name, &retval, 0, NULL);
	crex_string_release(func_name);

	zval_ptr_dtor(&retval);

	/* kill the object */
	zval_ptr_dtor(obj);
}

crx_stream_filter_status_t userfilter_filter(
			crx_stream *stream,
			crx_stream_filter *thisfilter,
			crx_stream_bucket_brigade *buckets_in,
			crx_stream_bucket_brigade *buckets_out,
			size_t *bytes_consumed,
			int flags
			)
{
	int ret = PSFS_ERR_FATAL;
	zval *obj = &thisfilter->abstract;
	zval func_name;
	zval retval;
	zval args[4];
	int call_result;

	/* the userfilter object probably doesn't exist anymore */
	if (CG(unclean_shutdown)) {
		return ret;
	}

	/* Make sure the stream is not closed while the filter callback executes. */
	uint32_t orig_no_fclose = stream->flags & CRX_STREAM_FLAG_NO_FCLOSE;
	stream->flags |= CRX_STREAM_FLAG_NO_FCLOSE;

	zval *stream_prop = crex_hash_str_find_ind(C_OBJPROP_P(obj), "stream", sizeof("stream")-1);
	if (stream_prop) {
		/* Give the userfilter class a hook back to the stream */
		zval_ptr_dtor(stream_prop);
		crx_stream_to_zval(stream, stream_prop);
		C_ADDREF_P(stream_prop);
	}

	ZVAL_STRINGL(&func_name, "filter", sizeof("filter")-1);

	/* Setup calling arguments */
	ZVAL_RES(&args[0], crex_register_resource(buckets_in, le_bucket_brigade));
	ZVAL_RES(&args[1], crex_register_resource(buckets_out, le_bucket_brigade));

	if (bytes_consumed) {
		ZVAL_LONG(&args[2], *bytes_consumed);
	} else {
		ZVAL_NULL(&args[2]);
	}
	ZVAL_MAKE_REF(&args[2]);

	ZVAL_BOOL(&args[3], flags & PSFS_FLAG_FLUSH_CLOSE);

	call_result = call_user_function(NULL,
			obj,
			&func_name,
			&retval,
			4, args);

	zval_ptr_dtor(&func_name);

	if (call_result == SUCCESS && C_TYPE(retval) != IS_UNDEF) {
		convert_to_long(&retval);
		ret = (int)C_LVAL(retval);
	} else if (call_result == FAILURE) {
		crx_error_docref(NULL, E_WARNING, "Failed to call filter function");
	}

	if (bytes_consumed) {
		*bytes_consumed = zval_get_long(&args[2]);
	}

	if (buckets_in->head) {
		crx_stream_bucket *bucket;

		crx_error_docref(NULL, E_WARNING, "Unprocessed filter buckets remaining on input brigade");
		while ((bucket = buckets_in->head)) {
			/* Remove unconsumed buckets from the brigade */
			crx_stream_bucket_unlink(bucket);
			crx_stream_bucket_delref(bucket);
		}
	}
	if (ret != PSFS_PASS_ON) {
		crx_stream_bucket *bucket = buckets_out->head;
		while (bucket != NULL) {
			crx_stream_bucket_unlink(bucket);
			crx_stream_bucket_delref(bucket);
			bucket = buckets_out->head;
		}
	}

	/* filter resources are cleaned up by the stream destructor,
	 * keeping a reference to the stream resource here would prevent it
	 * from being destroyed properly */
	if (stream_prop) {
		convert_to_null(stream_prop);
	}

	zval_ptr_dtor(&args[3]);
	zval_ptr_dtor(&args[2]);
	zval_ptr_dtor(&args[1]);
	zval_ptr_dtor(&args[0]);

	stream->flags &= ~CRX_STREAM_FLAG_NO_FCLOSE;
	stream->flags |= orig_no_fclose;

	return ret;
}

static const crx_stream_filter_ops userfilter_ops = {
	userfilter_filter,
	userfilter_dtor,
	"user-filter"
};

static crx_stream_filter *user_filter_factory_create(const char *filtername,
		zval *filterparams, uint8_t persistent)
{
	struct crx_user_filter_data *fdat = NULL;
	crx_stream_filter *filter;
	zval obj;
	zval retval;
	size_t len;

	/* some sanity checks */
	if (persistent) {
		crx_error_docref(NULL, E_WARNING,
				"Cannot use a user-space filter with a persistent stream");
		return NULL;
	}

	len = strlen(filtername);

	/* determine the classname/class entry */
	if (NULL == (fdat = crex_hash_str_find_ptr(BG(user_filter_map), (char*)filtername, len))) {
		char *period;

		/* Userspace Filters using ambiguous wildcards could cause problems.
           i.e.: myfilter.foo.bar will always call into myfilter.foo.*
                 never seeing myfilter.*
           TODO: Allow failed userfilter creations to continue
                 scanning through the list */
		if ((period = strrchr(filtername, '.'))) {
			char *wildcard = safe_emalloc(len, 1, 3);

			/* Search for wildcard matches instead */
			memcpy(wildcard, filtername, len + 1); /* copy \0 */
			period = wildcard + (period - filtername);
			while (period) {
				CREX_ASSERT(period[0] == '.');
				period[1] = '*';
				period[2] = '\0';
				if (NULL != (fdat = crex_hash_str_find_ptr(BG(user_filter_map), wildcard, strlen(wildcard)))) {
					period = NULL;
				} else {
					*period = '\0';
					period = strrchr(wildcard, '.');
				}
			}
			efree(wildcard);
		}
		CREX_ASSERT(fdat);
	}

	/* bind the classname to the actual class */
	if (fdat->ce == NULL) {
		if (NULL == (fdat->ce = crex_lookup_class(fdat->classname))) {
			crx_error_docref(NULL, E_WARNING,
					"User-filter \"%s\" requires class \"%s\", but that class is not defined",
					filtername, ZSTR_VAL(fdat->classname));
			return NULL;
		}
	}

	/* create the object */
	if (object_init_ex(&obj, fdat->ce) == FAILURE) {
		return NULL;
	}

	filter = crx_stream_filter_alloc(&userfilter_ops, NULL, 0);
	if (filter == NULL) {
		zval_ptr_dtor(&obj);
		return NULL;
	}

	/* filtername */
	add_property_string(&obj, "filtername", (char*)filtername);

	/* and the parameters, if any */
	if (filterparams) {
		add_property_zval(&obj, "params", filterparams);
	} else {
		add_property_null(&obj, "params");
	}

	/* invoke the constructor */
	crex_string *func_name = ZSTR_INIT_LITERAL("oncreate", 0);
	crex_call_method_if_exists(C_OBJ(obj), func_name, &retval, 0, NULL);
	crex_string_release(func_name);

	if (C_TYPE(retval) != IS_UNDEF) {
		if (C_TYPE(retval) == IS_FALSE) {
			/* User reported filter creation error "return false;" */
			zval_ptr_dtor(&retval);

			/* Kill the filter (safely) */
			ZVAL_UNDEF(&filter->abstract);
			crx_stream_filter_free(filter);

			/* Kill the object */
			zval_ptr_dtor(&obj);

			/* Report failure to filter_alloc */
			return NULL;
		}
		zval_ptr_dtor(&retval);
	}

	ZVAL_OBJ(&filter->abstract, C_OBJ(obj));

	return filter;
}

static const crx_stream_filter_factory user_filter_factory = {
	user_filter_factory_create
};

static void filter_item_dtor(zval *zv)
{
	struct crx_user_filter_data *fdat = C_PTR_P(zv);
	crex_string_release_ex(fdat->classname, 0);
	efree(fdat);
}

/* {{{ Return a bucket object from the brigade for operating on */
CRX_FUNCTION(stream_bucket_make_writeable)
{
	zval *zbrigade, zbucket;
	crx_stream_bucket_brigade *brigade;
	crx_stream_bucket *bucket;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(zbrigade)
	CREX_PARSE_PARAMETERS_END();

	if ((brigade = (crx_stream_bucket_brigade*)crex_fetch_resource(
					C_RES_P(zbrigade), CRX_STREAM_BRIGADE_RES_NAME, le_bucket_brigade)) == NULL) {
		RETURN_THROWS();
	}

	ZVAL_NULL(return_value);

	if (brigade->head && (bucket = crx_stream_bucket_make_writeable(brigade->head))) {
		ZVAL_RES(&zbucket, crex_register_resource(bucket, le_bucket));
		object_init(return_value);
		add_property_zval(return_value, "bucket", &zbucket);
		/* add_property_zval increments the refcount which is unwanted here */
		zval_ptr_dtor(&zbucket);
		add_property_stringl(return_value, "data", bucket->buf, bucket->buflen);
		add_property_long(return_value, "datalen", bucket->buflen);
	}
}
/* }}} */

/* {{{ crx_stream_bucket_attach */
static void crx_stream_bucket_attach(int append, INTERNAL_FUNCTION_PARAMETERS)
{
	zval *zbrigade, *zobject;
	zval *pzbucket, *pzdata;
	crx_stream_bucket_brigade *brigade;
	crx_stream_bucket *bucket;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_RESOURCE(zbrigade)
		C_PARAM_OBJECT(zobject)
	CREX_PARSE_PARAMETERS_END();

	if (NULL == (pzbucket = crex_hash_str_find_deref(C_OBJPROP_P(zobject), "bucket", sizeof("bucket")-1))) {
		crex_argument_value_error(2, "must be an object that has a \"bucket\" property");
		RETURN_THROWS();
	}

	if ((brigade = (crx_stream_bucket_brigade*)crex_fetch_resource(
					C_RES_P(zbrigade), CRX_STREAM_BRIGADE_RES_NAME, le_bucket_brigade)) == NULL) {
		RETURN_THROWS();
	}

	if ((bucket = (crx_stream_bucket *)crex_fetch_resource_ex(pzbucket, CRX_STREAM_BUCKET_RES_NAME, le_bucket)) == NULL) {
		RETURN_THROWS();
	}

	if (NULL != (pzdata = crex_hash_str_find_deref(C_OBJPROP_P(zobject), "data", sizeof("data")-1)) && C_TYPE_P(pzdata) == IS_STRING) {
		if (!bucket->own_buf) {
			bucket = crx_stream_bucket_make_writeable(bucket);
		}
		if (bucket->buflen != C_STRLEN_P(pzdata)) {
			bucket->buf = perealloc(bucket->buf, C_STRLEN_P(pzdata), bucket->is_persistent);
			bucket->buflen = C_STRLEN_P(pzdata);
		}
		memcpy(bucket->buf, C_STRVAL_P(pzdata), bucket->buflen);
	}

	if (append) {
		crx_stream_bucket_append(brigade, bucket);
	} else {
		crx_stream_bucket_prepend(brigade, bucket);
	}
	/* This is a hack necessary to accommodate situations where bucket is appended to the stream
 	 * multiple times. See bug35916.crxt for reference.
	 */
	if (bucket->refcount == 1) {
		bucket->refcount++;
	}
}
/* }}} */

/* {{{ Prepend bucket to brigade */
CRX_FUNCTION(stream_bucket_prepend)
{
	crx_stream_bucket_attach(0, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ Append bucket to brigade */
CRX_FUNCTION(stream_bucket_append)
{
	crx_stream_bucket_attach(1, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ Create a new bucket for use on the current stream */
CRX_FUNCTION(stream_bucket_new)
{
	zval *zstream, zbucket;
	crx_stream *stream;
	char *buffer;
	char *pbuffer;
	size_t buffer_len;
	crx_stream_bucket *bucket;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_ZVAL(zstream)
		C_PARAM_STRING(buffer, buffer_len)
	CREX_PARSE_PARAMETERS_END();

	crx_stream_from_zval(stream, zstream);
	pbuffer = pemalloc(buffer_len, crx_stream_is_persistent(stream));

	memcpy(pbuffer, buffer, buffer_len);

	bucket = crx_stream_bucket_new(stream, pbuffer, buffer_len, 1, crx_stream_is_persistent(stream));

	ZVAL_RES(&zbucket, crex_register_resource(bucket, le_bucket));
	object_init(return_value);
	add_property_zval(return_value, "bucket", &zbucket);
	/* add_property_zval increments the refcount which is unwanted here */
	zval_ptr_dtor(&zbucket);
	add_property_stringl(return_value, "data", bucket->buf, bucket->buflen);
	add_property_long(return_value, "datalen", bucket->buflen);
}
/* }}} */

/* {{{ Returns a list of registered filters */
CRX_FUNCTION(stream_get_filters)
{
	crex_string *filter_name;
	HashTable *filters_hash;

	CREX_PARSE_PARAMETERS_NONE();

	array_init(return_value);

	filters_hash = crx_get_stream_filters_hash();

	if (filters_hash && !HT_IS_PACKED(filters_hash)) {
		CREX_HASH_MAP_FOREACH_STR_KEY(filters_hash, filter_name) {
			if (filter_name) {
				add_next_index_str(return_value, crex_string_copy(filter_name));
			}
		} CREX_HASH_FOREACH_END();
	}
	/* It's okay to return an empty array if no filters are registered */
}
/* }}} */

/* {{{ Registers a custom filter handler class */
CRX_FUNCTION(stream_filter_register)
{
	crex_string *filtername, *classname;
	struct crx_user_filter_data *fdat;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_STR(filtername)
		C_PARAM_STR(classname)
	CREX_PARSE_PARAMETERS_END();

	if (!ZSTR_LEN(filtername)) {
		crex_argument_value_error(1, "must be a non-empty string");
		RETURN_THROWS();
	}

	if (!ZSTR_LEN(classname)) {
		crex_argument_value_error(2, "must be a non-empty string");
		RETURN_THROWS();
	}

	if (!BG(user_filter_map)) {
		BG(user_filter_map) = (HashTable*) emalloc(sizeof(HashTable));
		crex_hash_init(BG(user_filter_map), 8, NULL, (dtor_func_t) filter_item_dtor, 0);
	}

	fdat = ecalloc(1, sizeof(struct crx_user_filter_data));
	fdat->classname = crex_string_copy(classname);

	if (crex_hash_add_ptr(BG(user_filter_map), filtername, fdat) != NULL &&
			crx_stream_filter_register_factory_volatile(filtername, &user_filter_factory) == SUCCESS) {
		RETVAL_TRUE;
	} else {
		crex_string_release_ex(classname, 0);
		efree(fdat);
		RETVAL_FALSE;
	}
}
/* }}} */
