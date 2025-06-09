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
   | Author: Sterling Hughes <sterling@crx.net>                           |
   +----------------------------------------------------------------------+
*/

#define CREX_INCLUDE_FULL_WINDOWS_HEADERS

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "Crex/crex_smart_str.h"

#include "curl_private.h"

#include <curl/curl.h>
#include <curl/multi.h>

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#define SAVE_CURLM_ERROR(__handle, __err) (__handle)->err.no = (int) __err;

/* CurlMultiHandle class */

crex_class_entry *curl_multi_ce;

static inline crx_curlm *curl_multi_from_obj(crex_object *obj) {
	return (crx_curlm *)((char *)(obj) - XtOffsetOf(crx_curlm, std));
}

#define C_CURL_MULTI_P(zv) curl_multi_from_obj(C_OBJ_P(zv))

/* {{{ Returns a new cURL multi handle */
CRX_FUNCTION(curl_multi_init)
{
	crx_curlm *mh;

	CREX_PARSE_PARAMETERS_NONE();

	object_init_ex(return_value, curl_multi_ce);
	mh = C_CURL_MULTI_P(return_value);
	mh->multi = curl_multi_init();

	crex_llist_init(&mh->easyh, sizeof(zval), _crx_curl_multi_cleanup_list, 0);
}
/* }}} */

/* {{{ Add a normal cURL handle to a cURL multi handle */
CRX_FUNCTION(curl_multi_add_handle)
{
	zval      *z_mh;
	zval      *z_ch;
	crx_curlm *mh;
	crx_curl  *ch;
	CURLMcode error = CURLM_OK;

	CREX_PARSE_PARAMETERS_START(2,2)
		C_PARAM_OBJECT_OF_CLASS(z_mh, curl_multi_ce)
		C_PARAM_OBJECT_OF_CLASS(z_ch, curl_ce)
	CREX_PARSE_PARAMETERS_END();

	mh = C_CURL_MULTI_P(z_mh);
	ch = C_CURL_P(z_ch);

	_crx_curl_verify_handlers(ch, /* reporterror */ true);

	_crx_curl_cleanup_handle(ch);

	C_ADDREF_P(z_ch);
	crex_llist_add_element(&mh->easyh, z_ch);

	error = curl_multi_add_handle(mh->multi, ch->cp);
	SAVE_CURLM_ERROR(mh, error);

	RETURN_LONG((crex_long) error);
}
/* }}} */

void _crx_curl_multi_cleanup_list(void *data) /* {{{ */
{
	zval *z_ch = (zval *)data;

	zval_ptr_dtor(z_ch);
}
/* }}} */

/* Used internally as comparison routine passed to crex_list_del_element */
static int curl_compare_objects( zval *z1, zval *z2 ) /* {{{ */
{
	return (C_TYPE_P(z1) == C_TYPE_P(z2) &&
			C_TYPE_P(z1) == IS_OBJECT &&
			C_OBJ_P(z1) == C_OBJ_P(z2));
}
/* }}} */

/* Used to find the crx_curl resource for a given curl easy handle */
static zval *_crx_curl_multi_find_easy_handle(crx_curlm *mh, CURL *easy) /* {{{ */
{
	crx_curl 			*tmp_ch;
	crex_llist_position pos;
	zval				*pz_ch_temp;

	for(pz_ch_temp = (zval *)crex_llist_get_first_ex(&mh->easyh, &pos); pz_ch_temp;
		pz_ch_temp = (zval *)crex_llist_get_next_ex(&mh->easyh, &pos)) {
		tmp_ch = C_CURL_P(pz_ch_temp);

		if (tmp_ch->cp == easy) {
			return pz_ch_temp;
		}
	}

	return NULL;
}
/* }}} */

/* {{{ Remove a multi handle from a set of cURL handles */
CRX_FUNCTION(curl_multi_remove_handle)
{
	zval      *z_mh;
	zval      *z_ch;
	crx_curlm *mh;
	crx_curl  *ch;
	CURLMcode error = CURLM_OK;

	CREX_PARSE_PARAMETERS_START(2,2)
		C_PARAM_OBJECT_OF_CLASS(z_mh, curl_multi_ce)
		C_PARAM_OBJECT_OF_CLASS(z_ch, curl_ce)
	CREX_PARSE_PARAMETERS_END();

	mh = C_CURL_MULTI_P(z_mh);
	ch = C_CURL_P(z_ch);

	error = curl_multi_remove_handle(mh->multi, ch->cp);
	SAVE_CURLM_ERROR(mh, error);

	RETVAL_LONG((crex_long) error);
	crex_llist_del_element(&mh->easyh, z_ch, (int (*)(void *, void *))curl_compare_objects);

}
/* }}} */

/* {{{ Get all the sockets associated with the cURL extension, which can then be "selected" */
CRX_FUNCTION(curl_multi_select)
{
	zval           *z_mh;
	crx_curlm      *mh;
	double          timeout = 1.0;
	int             numfds = 0;
	CURLMcode error = CURLM_OK;

	CREX_PARSE_PARAMETERS_START(1,2)
		C_PARAM_OBJECT_OF_CLASS(z_mh, curl_multi_ce)
		C_PARAM_OPTIONAL
		C_PARAM_DOUBLE(timeout)
	CREX_PARSE_PARAMETERS_END();

	mh = C_CURL_MULTI_P(z_mh);

	error = curl_multi_wait(mh->multi, NULL, 0, (unsigned long) (timeout * 1000.0), &numfds);
	if (CURLM_OK != error) {
		SAVE_CURLM_ERROR(mh, error);
		RETURN_LONG(-1);
	}

	RETURN_LONG(numfds);
}
/* }}} */

/* {{{ Run the sub-connections of the current cURL handle */
CRX_FUNCTION(curl_multi_exec)
{
	zval      *z_mh;
	zval      *z_still_running;
	crx_curlm *mh;
	int        still_running;
	CURLMcode error = CURLM_OK;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_OBJECT_OF_CLASS(z_mh, curl_multi_ce)
		C_PARAM_ZVAL(z_still_running)
	CREX_PARSE_PARAMETERS_END();

	mh = C_CURL_MULTI_P(z_mh);

	{
		crex_llist_position pos;
		crx_curl *ch;
		zval	*pz_ch;

		for (pz_ch = (zval *)crex_llist_get_first_ex(&mh->easyh, &pos); pz_ch;
			pz_ch = (zval *)crex_llist_get_next_ex(&mh->easyh, &pos)) {
			ch = C_CURL_P(pz_ch);

			_crx_curl_verify_handlers(ch, /* reporterror */ true);
		}
	}

	still_running = zval_get_long(z_still_running);
	error = curl_multi_perform(mh->multi, &still_running);
	CREX_TRY_ASSIGN_REF_LONG(z_still_running, still_running);

	SAVE_CURLM_ERROR(mh, error);
	RETURN_LONG((crex_long) error);
}
/* }}} */

/* {{{ Return the content of a cURL handle if CURLOPT_RETURNTRANSFER is set */
CRX_FUNCTION(curl_multi_getcontent)
{
	zval     *z_ch;
	crx_curl *ch;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_OBJECT_OF_CLASS(z_ch, curl_ce)
	CREX_PARSE_PARAMETERS_END();

	ch = C_CURL_P(z_ch);

	if (ch->handlers.write->method == CRX_CURL_RETURN) {
		if (!ch->handlers.write->buf.s) {
			RETURN_EMPTY_STRING();
		}
		smart_str_0(&ch->handlers.write->buf);
		RETURN_STR_COPY(ch->handlers.write->buf.s);
	}

	RETURN_NULL();
}
/* }}} */

/* {{{ Get information about the current transfers */
CRX_FUNCTION(curl_multi_info_read)
{
	zval      *z_mh;
	crx_curlm *mh;
	CURLMsg	  *tmp_msg;
	int        queued_msgs;
	zval      *zmsgs_in_queue = NULL;
	crx_curl  *ch;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_OBJECT_OF_CLASS(z_mh, curl_multi_ce)
		C_PARAM_OPTIONAL
		C_PARAM_ZVAL(zmsgs_in_queue)
	CREX_PARSE_PARAMETERS_END();

	mh = C_CURL_MULTI_P(z_mh);

	tmp_msg = curl_multi_info_read(mh->multi, &queued_msgs);
	if (tmp_msg == NULL) {
		RETURN_FALSE;
	}

	if (zmsgs_in_queue) {
		CREX_TRY_ASSIGN_REF_LONG(zmsgs_in_queue, queued_msgs);
	}

	array_init(return_value);
	add_assoc_long(return_value, "msg", tmp_msg->msg);
	add_assoc_long(return_value, "result", tmp_msg->data.result);

	/* find the original easy curl handle */
	{
		zval *pz_ch = _crx_curl_multi_find_easy_handle(mh, tmp_msg->easy_handle);
		if (pz_ch != NULL) {
			/* we must save result to be able to read error message */
			ch = C_CURL_P(pz_ch);
			SAVE_CURL_ERROR(ch, tmp_msg->data.result);

			C_ADDREF_P(pz_ch);
			add_assoc_zval(return_value, "handle", pz_ch);
		}
	}
}
/* }}} */

/* {{{ Close a set of cURL handles */
CRX_FUNCTION(curl_multi_close)
{
	crx_curlm *mh;
	zval *z_mh;

	crex_llist_position pos;
	zval *pz_ch;

	CREX_PARSE_PARAMETERS_START(1,1)
		C_PARAM_OBJECT_OF_CLASS(z_mh, curl_multi_ce)
	CREX_PARSE_PARAMETERS_END();

	mh = C_CURL_MULTI_P(z_mh);

	for (pz_ch = (zval *)crex_llist_get_first_ex(&mh->easyh, &pos); pz_ch;
		pz_ch = (zval *)crex_llist_get_next_ex(&mh->easyh, &pos)) {
		crx_curl *ch = C_CURL_P(pz_ch);
		_crx_curl_verify_handlers(ch, /* reporterror */ true);
		curl_multi_remove_handle(mh->multi, ch->cp);
	}
	crex_llist_clean(&mh->easyh);
}
/* }}} */

/* {{{ Return an integer containing the last multi curl error number */
CRX_FUNCTION(curl_multi_errno)
{
	zval        *z_mh;
	crx_curlm   *mh;

	CREX_PARSE_PARAMETERS_START(1,1)
		C_PARAM_OBJECT_OF_CLASS(z_mh, curl_multi_ce)
	CREX_PARSE_PARAMETERS_END();

	mh = C_CURL_MULTI_P(z_mh);

	RETURN_LONG(mh->err.no);
}
/* }}} */

/* {{{ return string describing error code */
CRX_FUNCTION(curl_multi_strerror)
{
	crex_long code;
	const char *str;

	CREX_PARSE_PARAMETERS_START(1,1)
		C_PARAM_LONG(code)
	CREX_PARSE_PARAMETERS_END();

	str = curl_multi_strerror(code);
	if (str) {
		RETURN_STRING(str);
	} else {
		RETURN_NULL();
	}
}
/* }}} */

#if LIBCURL_VERSION_NUM >= 0x072C00 /* Available since 7.44.0 */

static int _crx_server_push_callback(CURL *parent_ch, CURL *easy, size_t num_headers, struct curl_pushheaders *push_headers, void *userp) /* {{{ */
{
	crx_curl 				*ch;
	crx_curl 				*parent;
	crx_curlm 				*mh 			= (crx_curlm *)userp;
	size_t 					rval 			= CURL_PUSH_DENY;
	crx_curl_callback 		*t 				= mh->handlers.server_push;
	zval					*pz_parent_ch 	= NULL;
	zval 					pz_ch;
	zval 					headers;
	zval 					retval;
	char 					*header;
	crex_result				error;
	crex_fcall_info 		fci 			= empty_fcall_info;

	pz_parent_ch = _crx_curl_multi_find_easy_handle(mh, parent_ch);
	if (pz_parent_ch == NULL) {
		return rval;
	}

	if (UNEXPECTED(crex_fcall_info_init(&t->func_name, 0, &fci, &t->fci_cache, NULL, NULL) == FAILURE)) {
		crx_error_docref(NULL, E_WARNING, "Cannot call the CURLMOPT_PUSHFUNCTION");
		return rval;
	}

	parent = C_CURL_P(pz_parent_ch);

	ch = init_curl_handle_into_zval(&pz_ch);
	ch->cp = easy;
	_crx_setup_easy_copy_handlers(ch, parent);

	size_t i;
	array_init(&headers);
	for(i=0; i<num_headers; i++) {
		header = curl_pushheader_bynum(push_headers, i);
		add_next_index_string(&headers, header);
	}

	CREX_ASSERT(pz_parent_ch);
	zval call_args[3] = {*pz_parent_ch, pz_ch, headers};

	fci.param_count = 3;
	fci.params = call_args;
	fci.retval = &retval;

	error = crex_call_function(&fci, &t->fci_cache);
	zval_ptr_dtor_nogc(&headers);

	if (error == FAILURE) {
		crx_error_docref(NULL, E_WARNING, "Cannot call the CURLMOPT_PUSHFUNCTION");
	} else if (!C_ISUNDEF(retval)) {
		if (CURL_PUSH_DENY != zval_get_long(&retval)) {
		    rval = CURL_PUSH_OK;
			crex_llist_add_element(&mh->easyh, &pz_ch);
		} else {
			/* libcurl will free this easy handle, avoid double free */
			ch->cp = NULL;
		}
	}

	return rval;
}
/* }}} */

#endif

static bool _crx_curl_multi_setopt(crx_curlm *mh, crex_long option, zval *zvalue, zval *return_value) /* {{{ */
{
	CURLMcode error = CURLM_OK;

	switch (option) {
		case CURLMOPT_PIPELINING:
		case CURLMOPT_MAXCONNECTS:
#if LIBCURL_VERSION_NUM >= 0x071e00 /* 7.30.0 */
		case CURLMOPT_CHUNK_LENGTH_PENALTY_SIZE:
		case CURLMOPT_CONTENT_LENGTH_PENALTY_SIZE:
		case CURLMOPT_MAX_HOST_CONNECTIONS:
		case CURLMOPT_MAX_PIPELINE_LENGTH:
		case CURLMOPT_MAX_TOTAL_CONNECTIONS:
#endif
#if LIBCURL_VERSION_NUM >= 0x074300 /* Available since 7.67.0 */
		case CURLMOPT_MAX_CONCURRENT_STREAMS:
#endif
		{
			crex_long lval = zval_get_long(zvalue);

			if (option == CURLMOPT_PIPELINING && (lval & 1)) {
#if LIBCURL_VERSION_NUM >= 0x073e00 /* 7.62.0 */
				crx_error_docref(NULL, E_WARNING, "CURLPIPE_HTTP1 is no longer supported");
#else
				crx_error_docref(NULL, E_DEPRECATED, "CURLPIPE_HTTP1 is deprecated");
#endif
			}
			error = curl_multi_setopt(mh->multi, option, lval);
			break;
		}
#if LIBCURL_VERSION_NUM > 0x072D00 /* Available since 7.45.0 */
		case CURLMOPT_PUSHFUNCTION:
			if (mh->handlers.server_push == NULL) {
				mh->handlers.server_push = ecalloc(1, sizeof(crx_curl_callback));
			} else if (!C_ISUNDEF(mh->handlers.server_push->func_name)) {
				zval_ptr_dtor(&mh->handlers.server_push->func_name);
				mh->handlers.server_push->fci_cache = empty_fcall_info_cache;
			}

			ZVAL_COPY(&mh->handlers.server_push->func_name, zvalue);
			error = curl_multi_setopt(mh->multi, CURLMOPT_PUSHFUNCTION, _crx_server_push_callback);
			if (error != CURLM_OK) {
				return false;
			}
			error = curl_multi_setopt(mh->multi, CURLMOPT_PUSHDATA, mh);
			break;
#endif
		default:
			crex_argument_value_error(2, "is not a valid cURL multi option");
			error = CURLM_UNKNOWN_OPTION;
			break;
	}

	SAVE_CURLM_ERROR(mh, error);

	return error == CURLM_OK;
}
/* }}} */

/* {{{ Set an option for the curl multi handle */
CRX_FUNCTION(curl_multi_setopt)
{
	zval       *z_mh, *zvalue;
	crex_long        options;
	crx_curlm *mh;

	CREX_PARSE_PARAMETERS_START(3,3)
		C_PARAM_OBJECT_OF_CLASS(z_mh, curl_multi_ce)
		C_PARAM_LONG(options)
		C_PARAM_ZVAL(zvalue)
	CREX_PARSE_PARAMETERS_END();

	mh = C_CURL_MULTI_P(z_mh);

	if (_crx_curl_multi_setopt(mh, options, zvalue, return_value)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* CurlMultiHandle class */

static crex_object *curl_multi_create_object(crex_class_entry *class_type) {
	crx_curlm *intern = crex_object_alloc(sizeof(crx_curlm), class_type);

	crex_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	return &intern->std;
}

static crex_function *curl_multi_get_constructor(crex_object *object) {
	crex_throw_error(NULL, "Cannot directly construct CurlMultiHandle, use curl_multi_init() instead");
	return NULL;
}

static void curl_multi_free_obj(crex_object *object)
{
	crx_curlm *mh = curl_multi_from_obj(object);

	crex_llist_position pos;
	crx_curl *ch;
	zval	*pz_ch;

	if (!mh->multi) {
		/* Can happen if constructor throws. */
		crex_object_std_dtor(&mh->std);
		return;
	}

	for (pz_ch = (zval *)crex_llist_get_first_ex(&mh->easyh, &pos); pz_ch;
		pz_ch = (zval *)crex_llist_get_next_ex(&mh->easyh, &pos)) {
		if (!(OBJ_FLAGS(C_OBJ_P(pz_ch)) & IS_OBJ_FREE_CALLED)) {
			ch = C_CURL_P(pz_ch);
			_crx_curl_verify_handlers(ch, /* reporterror */ false);
		}
	}

	curl_multi_cleanup(mh->multi);
	crex_llist_clean(&mh->easyh);
	if (mh->handlers.server_push) {
		zval_ptr_dtor(&mh->handlers.server_push->func_name);
		efree(mh->handlers.server_push);
	}

	crex_object_std_dtor(&mh->std);
}

static HashTable *curl_multi_get_gc(crex_object *object, zval **table, int *n)
{
	crx_curlm *curl_multi = curl_multi_from_obj(object);

	crex_get_gc_buffer *gc_buffer = crex_get_gc_buffer_create();

	if (curl_multi->handlers.server_push) {
		crex_get_gc_buffer_add_zval(gc_buffer, &curl_multi->handlers.server_push->func_name);
	}

	crex_llist_position pos;
	for (zval *pz_ch = (zval *) crex_llist_get_first_ex(&curl_multi->easyh, &pos); pz_ch;
		pz_ch = (zval *) crex_llist_get_next_ex(&curl_multi->easyh, &pos)) {
		crex_get_gc_buffer_add_zval(gc_buffer, pz_ch);
	}

	crex_get_gc_buffer_use(gc_buffer, table, n);

	return crex_std_get_properties(object);
}

static crex_object_handlers curl_multi_handlers;

void curl_multi_register_handlers(void) {
	curl_multi_ce->create_object = curl_multi_create_object;
	curl_multi_ce->default_object_handlers = &curl_multi_handlers;

	memcpy(&curl_multi_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	curl_multi_handlers.offset = XtOffsetOf(crx_curlm, std);
	curl_multi_handlers.free_obj = curl_multi_free_obj;
	curl_multi_handlers.get_gc = curl_multi_get_gc;
	curl_multi_handlers.get_constructor = curl_multi_get_constructor;
	curl_multi_handlers.clone_obj = NULL;
	curl_multi_handlers.cast_object = curl_cast_object;
	curl_multi_handlers.compare = crex_objects_not_comparable;
}
