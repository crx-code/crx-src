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
  | Author: John Coggeshall <john@crx.net>                               |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "crx_tidy.h"

#ifdef HAVE_TIDY

#include "crx_ini.h"
#include "ext/standard/info.h"

#ifdef HAVE_TIDY_H
#include "tidy.h"
#elif defined(HAVE_TIDYP_H)
#include "tidyp.h"
#endif

#ifdef HAVE_TIDYBUFFIO_H
#include "tidybuffio.h"
#else
#include "buffio.h"
#endif

#include "tidy_arginfo.h"

/* compatibility with older versions of libtidy */
#ifndef TIDY_CALL
#define TIDY_CALL
#endif

/* {{{ ext/tidy macros */
#define FIX_BUFFER(bptr) do { if ((bptr)->size) { (bptr)->bp[(bptr)->size-1] = '\0'; } } while(0)

#define TIDY_SET_CONTEXT \
    zval *object = getThis();

#define TIDY_FETCH_OBJECT	\
	CRXTidyObj *obj;	\
	zval *object; \
	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "O", &object, tidy_ce_doc) == FAILURE) {	\
		RETURN_THROWS();	\
	}	\
	obj = C_TIDY_P(object);	\

#define TIDY_FETCH_INITIALIZED_OBJECT \
	TIDY_FETCH_OBJECT; \
	if (!obj->ptdoc->initialized) { \
		crex_throw_error(NULL, "tidy object is not initialized"); \
		return; \
	}

#define TIDY_FETCH_ONLY_OBJECT	\
	CRXTidyObj *obj;	\
	TIDY_SET_CONTEXT; \
	if (crex_parse_parameters_none() == FAILURE) {	\
		RETURN_THROWS();	\
	}	\
	obj = C_TIDY_P(object);	\

#define TIDY_APPLY_CONFIG(_doc, _val_str, _val_ht) \
	if (_val_ht) { \
		_crx_tidy_apply_config_array(_doc, _val_ht); \
	} else if (_val_str) { \
		TIDY_OPEN_BASE_DIR_CHECK(ZSTR_VAL(_val_str)); \
		crx_tidy_load_config(_doc, ZSTR_VAL(_val_str)); \
	}

#define TIDY_OPEN_BASE_DIR_CHECK(filename) \
if (crx_check_open_basedir(filename)) { \
	RETURN_FALSE; \
} \

#define TIDY_SET_DEFAULT_CONFIG(_doc) \
	if (TG(default_config) && TG(default_config)[0]) { \
		crx_tidy_load_config(_doc, TG(default_config)); \
	}
/* }}} */

/* {{{ ext/tidy structs */
typedef struct _CRXTidyDoc CRXTidyDoc;
typedef struct _CRXTidyObj CRXTidyObj;

typedef enum {
	is_node,
	is_doc
} tidy_obj_type;

typedef enum {
	is_root_node,
	is_html_node,
	is_head_node,
	is_body_node
} tidy_base_nodetypes;

struct _CRXTidyDoc {
	TidyDoc			doc;
	TidyBuffer		*errbuf;
	unsigned int	ref_count;
	unsigned int    initialized:1;
};

struct _CRXTidyObj {
	TidyNode		node;
	tidy_obj_type	type;
	CRXTidyDoc		*ptdoc;
	crex_object		std;
};

static inline CRXTidyObj *crx_tidy_fetch_object(crex_object *obj) {
	return (CRXTidyObj *)((char*)(obj) - XtOffsetOf(CRXTidyObj, std));
}

#define C_TIDY_P(zv) crx_tidy_fetch_object(C_OBJ_P((zv)))
/* }}} */

/* {{{ ext/tidy prototypes */
static crex_string *crx_tidy_file_to_mem(char *, bool);
static void tidy_object_free_storage(crex_object *);
static crex_object *tidy_object_new_node(crex_class_entry *);
static crex_object *tidy_object_new_doc(crex_class_entry *);
static zval *tidy_instantiate(crex_class_entry *, zval *);
static crex_result tidy_doc_cast_handler(crex_object *, zval *, int);
static crex_result tidy_node_cast_handler(crex_object *, zval *, int);
static void tidy_doc_update_properties(CRXTidyObj *);
static void tidy_add_node_default_properties(CRXTidyObj *);
static void *crx_tidy_get_opt_val(CRXTidyDoc *, TidyOption, TidyOptionType *);
static void crx_tidy_create_node(INTERNAL_FUNCTION_PARAMETERS, tidy_base_nodetypes);
static int _crx_tidy_set_tidy_opt(TidyDoc, char *, zval *);
static int _crx_tidy_apply_config_array(TidyDoc doc, HashTable *ht_options);
static CRX_INI_MH(crx_tidy_set_clean_output);
static void crx_tidy_clean_output_start(const char *name, size_t name_len);
static crx_output_handler *crx_tidy_output_handler_init(const char *handler_name, size_t handler_name_len, size_t chunk_size, int flags);
static int crx_tidy_output_handler(void **nothing, crx_output_context *output_context);

static CRX_MINIT_FUNCTION(tidy);
static CRX_MSHUTDOWN_FUNCTION(tidy);
static CRX_RINIT_FUNCTION(tidy);
static CRX_RSHUTDOWN_FUNCTION(tidy);
static CRX_MINFO_FUNCTION(tidy);

CREX_DECLARE_MODULE_GLOBALS(tidy)

CRX_INI_BEGIN()
STD_CRX_INI_ENTRY("tidy.default_config",	"",		CRX_INI_SYSTEM,		OnUpdateString,				default_config,		crex_tidy_globals,	tidy_globals)
STD_CRX_INI_BOOLEAN("tidy.clean_output",	"0",	CRX_INI_USER,		crx_tidy_set_clean_output,	clean_output,		crex_tidy_globals,	tidy_globals)
CRX_INI_END()

static crex_class_entry *tidy_ce_doc, *tidy_ce_node;

static crex_object_handlers tidy_object_handlers_doc;
static crex_object_handlers tidy_object_handlers_node;

crex_module_entry tidy_module_entry = {
	STANDARD_MODULE_HEADER,
	"tidy",
	ext_functions,
	CRX_MINIT(tidy),
	CRX_MSHUTDOWN(tidy),
	CRX_RINIT(tidy),
	CRX_RSHUTDOWN(tidy),
	CRX_MINFO(tidy),
	CRX_TIDY_VERSION,
	CRX_MODULE_GLOBALS(tidy),
	NULL,
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_TIDY
#ifdef ZTS
CREX_TSRMLS_CACHE_DEFINE()
#endif
CREX_GET_MODULE(tidy)
#endif

static void* TIDY_CALL crx_tidy_malloc(size_t len)
{
	return emalloc(len);
}

static void* TIDY_CALL crx_tidy_realloc(void *buf, size_t len)
{
	return erealloc(buf, len);
}

static void TIDY_CALL crx_tidy_free(void *buf)
{
	efree(buf);
}

static void TIDY_CALL crx_tidy_panic(ctmbstr msg)
{
	crx_error_docref(NULL, E_ERROR, "Could not allocate memory for tidy! (Reason: %s)", (char *)msg);
}

static void crx_tidy_load_config(TidyDoc doc, const char *path)
{
	int ret = tidyLoadConfig(doc, path);
	if (ret < 0) {
		crx_error_docref(NULL, E_WARNING, "Could not load the Tidy configuration file \"%s\"", path);
	} else if (ret > 0) {
		crx_error_docref(NULL, E_NOTICE, "There were errors while parsing the Tidy configuration file \"%s\"", path);
	}
}

static int _crx_tidy_set_tidy_opt(TidyDoc doc, char *optname, zval *value)
{
	TidyOption opt = tidyGetOptionByName(doc, optname);
	crex_string *str, *tmp_str;
	crex_long lval;

	if (!opt) {
		crx_error_docref(NULL, E_WARNING, "Unknown Tidy configuration option \"%s\"", optname);
		return FAILURE;
	}

	if (tidyOptIsReadOnly(opt)) {
		crx_error_docref(NULL, E_WARNING, "Attempting to set read-only option \"%s\"", optname);
		return FAILURE;
	}

	switch(tidyOptGetType(opt)) {
		case TidyString:
			str = zval_get_tmp_string(value, &tmp_str);
			if (tidyOptSetValue(doc, tidyOptGetId(opt), ZSTR_VAL(str))) {
				crex_tmp_string_release(tmp_str);
				return SUCCESS;
			}
			crex_tmp_string_release(tmp_str);
			break;

		case TidyInteger:
			lval = zval_get_long(value);
			if (tidyOptSetInt(doc, tidyOptGetId(opt), lval)) {
				return SUCCESS;
			}
			break;

		case TidyBoolean:
			lval = zval_get_long(value);
			if (tidyOptSetBool(doc, tidyOptGetId(opt), lval)) {
				return SUCCESS;
			}
			break;

		default:
			crx_error_docref(NULL, E_WARNING, "Unable to determine type of configuration option");
			break;
	}

	return FAILURE;
}

static void crx_tidy_quick_repair(INTERNAL_FUNCTION_PARAMETERS, bool is_file)
{
	char *enc = NULL;
	size_t enc_len = 0;
	TidyDoc doc;
	TidyBuffer *errbuf;
	crex_string *data, *arg1, *config_str = NULL;
	HashTable *config_ht = NULL;

	if (is_file) {
		bool use_include_path = 0;

		CREX_PARSE_PARAMETERS_START(1, 4)
			C_PARAM_PATH_STR(arg1)
			C_PARAM_OPTIONAL
			C_PARAM_ARRAY_HT_OR_STR_OR_NULL(config_ht, config_str)
			C_PARAM_STRING(enc, enc_len)
			C_PARAM_BOOL(use_include_path)
		CREX_PARSE_PARAMETERS_END();

		if (!(data = crx_tidy_file_to_mem(ZSTR_VAL(arg1), use_include_path))) {
			RETURN_FALSE;
		}
	} else {
		CREX_PARSE_PARAMETERS_START(1, 3)
			C_PARAM_STR(arg1)
			C_PARAM_OPTIONAL
			C_PARAM_ARRAY_HT_OR_STR_OR_NULL(config_ht, config_str)
			C_PARAM_STRING(enc, enc_len)
		CREX_PARSE_PARAMETERS_END();

		data = arg1;
	}

	if (CREX_SIZE_T_UINT_OVFL(ZSTR_LEN(data))) {
		crex_argument_value_error(1, "is too long");
		RETURN_THROWS();
	}

	doc = tidyCreate();
	errbuf = emalloc(sizeof(TidyBuffer));
	tidyBufInit(errbuf);

	if (tidySetErrorBuffer(doc, errbuf) != 0) {
		tidyBufFree(errbuf);
		efree(errbuf);
		tidyRelease(doc);
		crx_error_docref(NULL, E_ERROR, "Could not set Tidy error buffer");
	}

	tidyOptSetBool(doc, TidyForceOutput, yes);
	tidyOptSetBool(doc, TidyMark, no);

	TIDY_SET_DEFAULT_CONFIG(doc);

	TIDY_APPLY_CONFIG(doc, config_str, config_ht);

	if(enc_len) {
		if (tidySetCharEncoding(doc, enc) < 0) {
			crx_error_docref(NULL, E_WARNING, "Could not set encoding \"%s\"", enc);
			RETVAL_FALSE;
		}
	}

	if (data) {
		TidyBuffer buf;

		tidyBufInit(&buf);
		tidyBufAttach(&buf, (byte *) ZSTR_VAL(data), (uint32_t)ZSTR_LEN(data));

		if (tidyParseBuffer(doc, &buf) < 0) {
			crx_error_docref(NULL, E_WARNING, "%s", errbuf->bp);
			RETVAL_FALSE;
		} else {
			if (tidyCleanAndRepair(doc) >= 0) {
				TidyBuffer output;
				tidyBufInit(&output);

				tidySaveBuffer (doc, &output);
				FIX_BUFFER(&output);
				RETVAL_STRINGL((char *) output.bp, output.size ? output.size-1 : 0);
				tidyBufFree(&output);
			} else {
				RETVAL_FALSE;
			}
		}
	}

	if (is_file) {
		crex_string_release_ex(data, 0);
	}

	tidyBufFree(errbuf);
	efree(errbuf);
	tidyRelease(doc);
}

static crex_string *crx_tidy_file_to_mem(char *filename, bool use_include_path)
{
	crx_stream *stream;
	crex_string *data = NULL;

	if (!(stream = crx_stream_open_wrapper(filename, "rb", (use_include_path ? USE_PATH : 0), NULL))) {
		return NULL;
	}
	if ((data = crx_stream_copy_to_mem(stream, CRX_STREAM_COPY_ALL, 0)) == NULL) {
		data = ZSTR_EMPTY_ALLOC();
	}
	crx_stream_close(stream);

	return data;
}

static void tidy_object_free_storage(crex_object *object)
{
	CRXTidyObj *intern = crx_tidy_fetch_object(object);

	crex_object_std_dtor(&intern->std);

	if (intern->ptdoc) {
		intern->ptdoc->ref_count--;

		if (intern->ptdoc->ref_count <= 0) {
			tidyBufFree(intern->ptdoc->errbuf);
			efree(intern->ptdoc->errbuf);
			tidyRelease(intern->ptdoc->doc);
			efree(intern->ptdoc);
		}
	}
}

static crex_object *tidy_object_new(crex_class_entry *class_type, crex_object_handlers *handlers, tidy_obj_type objtype)
{
	CRXTidyObj *intern;

	intern = crex_object_alloc(sizeof(CRXTidyObj), class_type);
	crex_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	switch(objtype) {
		case is_node:
			break;

		case is_doc:
			intern->ptdoc = emalloc(sizeof(CRXTidyDoc));
			intern->ptdoc->doc = tidyCreate();
			intern->ptdoc->ref_count = 1;
			intern->ptdoc->initialized = 0;
			intern->ptdoc->errbuf = emalloc(sizeof(TidyBuffer));
			tidyBufInit(intern->ptdoc->errbuf);

			if (tidySetErrorBuffer(intern->ptdoc->doc, intern->ptdoc->errbuf) != 0) {
				tidyBufFree(intern->ptdoc->errbuf);
				efree(intern->ptdoc->errbuf);
				tidyRelease(intern->ptdoc->doc);
				efree(intern->ptdoc);
				efree(intern);
				crx_error_docref(NULL, E_ERROR, "Could not set Tidy error buffer");
			}

			tidyOptSetBool(intern->ptdoc->doc, TidyForceOutput, yes);
			tidyOptSetBool(intern->ptdoc->doc, TidyMark, no);

			TIDY_SET_DEFAULT_CONFIG(intern->ptdoc->doc);
			break;
	}

	intern->std.handlers = handlers;

	return &intern->std;
}

static crex_object *tidy_object_new_node(crex_class_entry *class_type)
{
	return tidy_object_new(class_type, &tidy_object_handlers_node, is_node);
}

static crex_object *tidy_object_new_doc(crex_class_entry *class_type)
{
	return tidy_object_new(class_type, &tidy_object_handlers_doc, is_doc);
}

static zval *tidy_instantiate(crex_class_entry *pce, zval *object)
{
	object_init_ex(object, pce);
	return object;
}

static crex_result tidy_doc_cast_handler(crex_object *in, zval *out, int type)
{
	TidyBuffer output;
	CRXTidyObj *obj;

	switch (type) {
		case IS_LONG:
		case _IS_NUMBER:
			ZVAL_LONG(out, 0);
			break;

		case IS_DOUBLE:
			ZVAL_DOUBLE(out, 0);
			break;

		case _IS_BOOL:
			ZVAL_TRUE(out);
			break;

		case IS_STRING:
			obj = crx_tidy_fetch_object(in);
			tidyBufInit(&output);
			tidySaveBuffer (obj->ptdoc->doc, &output);
			if (output.size) {
				ZVAL_STRINGL(out, (char *) output.bp, output.size-1);
			} else {
				ZVAL_EMPTY_STRING(out);
			}
			tidyBufFree(&output);
			break;

		default:
			return FAILURE;
	}

	return SUCCESS;
}

static crex_result tidy_node_cast_handler(crex_object *in, zval *out, int type)
{
	TidyBuffer buf;
	CRXTidyObj *obj;

	switch(type) {
		case IS_LONG:
		case _IS_NUMBER:
			ZVAL_LONG(out, 0);
			break;

		case IS_DOUBLE:
			ZVAL_DOUBLE(out, 0);
			break;

		case _IS_BOOL:
			ZVAL_TRUE(out);
			break;

		case IS_STRING:
			obj = crx_tidy_fetch_object(in);
			tidyBufInit(&buf);
			if (obj->ptdoc) {
				tidyNodeGetText(obj->ptdoc->doc, obj->node, &buf);
				ZVAL_STRINGL(out, (char *) buf.bp, buf.size-1);
			} else {
				ZVAL_EMPTY_STRING(out);
			}
			tidyBufFree(&buf);
			break;

		default:
			return FAILURE;
	}

	return SUCCESS;
}

static void tidy_doc_update_properties(CRXTidyObj *obj)
{
	TidyBuffer output;

	tidyBufInit(&output);
	tidySaveBuffer (obj->ptdoc->doc, &output);

	if (output.size) {
		crex_update_property_stringl(
			tidy_ce_doc,
			&obj->std,
			"value",
			sizeof("value") - 1,
			(char*) output.bp,
			output.size-1
		);
	}

	tidyBufFree(&output);

	if (obj->ptdoc->errbuf->size) {
		crex_update_property_stringl(
			tidy_ce_doc,
			&obj->std,
			"errorBuffer",
			sizeof("errorBuffer") - 1,
			(char*) obj->ptdoc->errbuf->bp,
			obj->ptdoc->errbuf->size-1
		);
	}
}

static void tidy_add_node_default_properties(CRXTidyObj *obj)
{
	TidyBuffer buf;
	TidyAttr	tempattr;
	TidyNode	tempnode;
	zval attribute, children, temp;
	CRXTidyObj *newobj;
	char *name;

	tidyBufInit(&buf);
	tidyNodeGetText(obj->ptdoc->doc, obj->node, &buf);

	crex_update_property_stringl(
		tidy_ce_node,
		&obj->std,
		"value",
		sizeof("value") - 1,
		buf.size ? (char *) buf.bp : "",
		buf.size ? buf.size - 1 : 0
	);

	tidyBufFree(&buf);

	name = (char *) tidyNodeGetName(obj->node);

	crex_update_property_string(
		tidy_ce_node,
		&obj->std,
		"name",
		sizeof("name") - 1,
		name ? name : ""
	);

	crex_update_property_long(
		tidy_ce_node,
		&obj->std,
		"type",
		sizeof("type") - 1,
		tidyNodeGetType(obj->node)
	);

	crex_update_property_long(
		tidy_ce_node,
		&obj->std,
		"line",
		sizeof("line") - 1,
		tidyNodeLine(obj->node)
	);

	crex_update_property_long(
		tidy_ce_node,
		&obj->std,
		"column",
		sizeof("column") - 1,
		tidyNodeColumn(obj->node)
	);

	crex_update_property_bool(
		tidy_ce_node,
		&obj->std,
		"proprietary",
		sizeof("proprietary") - 1,
		tidyNodeIsProp(obj->ptdoc->doc, obj->node)
	);

	switch(tidyNodeGetType(obj->node)) {
		case TidyNode_Root:
		case TidyNode_DocType:
		case TidyNode_Text:
		case TidyNode_Comment:
			crex_update_property_null(
				tidy_ce_node,
				&obj->std,
				"id",
				sizeof("id") - 1
			);
			break;

		default:
			crex_update_property_long(
				tidy_ce_node,
				&obj->std,
				"id",
				sizeof("id") - 1,
				tidyNodeGetId(obj->node)
			);
	}

	tempattr = tidyAttrFirst(obj->node);

	if (tempattr) {
		char *name, *val;
		array_init(&attribute);

		do {
			name = (char *)tidyAttrName(tempattr);
			val = (char *)tidyAttrValue(tempattr);
			if (name && val) {
				add_assoc_string(&attribute, name, val);
			}
		} while((tempattr = tidyAttrNext(tempattr)));
	} else {
		ZVAL_NULL(&attribute);
	}

	crex_update_property(
		tidy_ce_node,
		&obj->std,
		"attribute",
		sizeof("attribute") - 1,
		&attribute
	);

	zval_ptr_dtor(&attribute);

	tempnode = tidyGetChild(obj->node);

	if (tempnode) {
		array_init(&children);
		do {
			tidy_instantiate(tidy_ce_node, &temp);
			newobj = C_TIDY_P(&temp);
			newobj->node = tempnode;
			newobj->type = is_node;
			newobj->ptdoc = obj->ptdoc;
			newobj->ptdoc->ref_count++;

			tidy_add_node_default_properties(newobj);
			add_next_index_zval(&children, &temp);

		} while((tempnode = tidyGetNext(tempnode)));

	} else {
		ZVAL_NULL(&children);
	}

	crex_update_property(
		tidy_ce_node,
		&obj->std,
		"child",
		sizeof("child") - 1,
		&children
	);

	zval_ptr_dtor(&children);
}

static void *crx_tidy_get_opt_val(CRXTidyDoc *ptdoc, TidyOption opt, TidyOptionType *type)
{
	*type = tidyOptGetType(opt);

	switch (*type) {
		case TidyString: {
			char *val = (char *) tidyOptGetValue(ptdoc->doc, tidyOptGetId(opt));
			if (val) {
				return (void *) crex_string_init(val, strlen(val), 0);
			} else {
				return (void *) ZSTR_EMPTY_ALLOC();
			}
		}
			break;

		case TidyInteger:
			return (void *) (uintptr_t) tidyOptGetInt(ptdoc->doc, tidyOptGetId(opt));
			break;

		case TidyBoolean:
			return (void *) tidyOptGetBool(ptdoc->doc, tidyOptGetId(opt));
			break;
	}

	/* should not happen */
	return NULL;
}

static void crx_tidy_create_node(INTERNAL_FUNCTION_PARAMETERS, tidy_base_nodetypes node_type)
{
	CRXTidyObj *newobj;
	TidyNode node;
	TIDY_FETCH_OBJECT;

	switch (node_type) {
		case is_root_node:
			node = tidyGetRoot(obj->ptdoc->doc);
			break;

		case is_html_node:
			node = tidyGetHtml(obj->ptdoc->doc);
			break;

		case is_head_node:
			node = tidyGetHead(obj->ptdoc->doc);
			break;

		case is_body_node:
			node = tidyGetBody(obj->ptdoc->doc);
			break;

		EMPTY_SWITCH_DEFAULT_CASE()
	}

	if (!node) {
		RETURN_NULL();
	}

	tidy_instantiate(tidy_ce_node, return_value);
	newobj = C_TIDY_P(return_value);
	newobj->type  = is_node;
	newobj->ptdoc = obj->ptdoc;
	newobj->node  = node;
	newobj->ptdoc->ref_count++;

	tidy_add_node_default_properties(newobj);
}

static int _crx_tidy_apply_config_array(TidyDoc doc, HashTable *ht_options)
{
	zval *opt_val;
	crex_string *opt_name;

	if (!HT_IS_PACKED(ht_options)) {
		CREX_HASH_MAP_FOREACH_STR_KEY_VAL(ht_options, opt_name, opt_val) {
			if (opt_name == NULL) {
				continue;
			}
			_crx_tidy_set_tidy_opt(doc, ZSTR_VAL(opt_name), opt_val);
		} CREX_HASH_FOREACH_END();
	}
	return SUCCESS;
}

static int crx_tidy_parse_string(CRXTidyObj *obj, char *string, uint32_t len, char *enc)
{
	TidyBuffer buf;

	if(enc) {
		if (tidySetCharEncoding(obj->ptdoc->doc, enc) < 0) {
			crx_error_docref(NULL, E_WARNING, "Could not set encoding \"%s\"", enc);
			return FAILURE;
		}
	}

	obj->ptdoc->initialized = 1;

	tidyBufInit(&buf);
	tidyBufAttach(&buf, (byte *) string, len);
	if (tidyParseBuffer(obj->ptdoc->doc, &buf) < 0) {
		crx_error_docref(NULL, E_WARNING, "%s", obj->ptdoc->errbuf->bp);
		return FAILURE;
	}
	tidy_doc_update_properties(obj);

	return SUCCESS;
}

static CRX_MINIT_FUNCTION(tidy)
{
	tidySetMallocCall(crx_tidy_malloc);
	tidySetReallocCall(crx_tidy_realloc);
	tidySetFreeCall(crx_tidy_free);
	tidySetPanicCall(crx_tidy_panic);

	REGISTER_INI_ENTRIES();

	tidy_ce_doc = register_class_tidy();
	tidy_ce_doc->create_object = tidy_object_new_doc;
	memcpy(&tidy_object_handlers_doc, &std_object_handlers, sizeof(crex_object_handlers));
	tidy_object_handlers_doc.clone_obj = NULL;

	tidy_ce_node = register_class_tidyNode();
	tidy_ce_node->create_object = tidy_object_new_node;
	memcpy(&tidy_object_handlers_node, &std_object_handlers, sizeof(crex_object_handlers));
	tidy_object_handlers_node.clone_obj = NULL;

	tidy_object_handlers_doc.cast_object = tidy_doc_cast_handler;
	tidy_object_handlers_node.cast_object = tidy_node_cast_handler;

	tidy_object_handlers_node.offset = tidy_object_handlers_doc.offset = XtOffsetOf(CRXTidyObj, std);
	tidy_object_handlers_node.free_obj = tidy_object_handlers_doc.free_obj = tidy_object_free_storage;

	register_tidy_symbols(module_number);

	crx_output_handler_alias_register(CREX_STRL("ob_tidyhandler"), crx_tidy_output_handler_init);

	return SUCCESS;
}

static CRX_RINIT_FUNCTION(tidy)
{
#if defined(COMPILE_DL_TIDY) && defined(ZTS)
	CREX_TSRMLS_CACHE_UPDATE();
#endif

	crx_tidy_clean_output_start(CREX_STRL("ob_tidyhandler"));

	return SUCCESS;
}

static CRX_RSHUTDOWN_FUNCTION(tidy)
{
	TG(clean_output) = INI_ORIG_BOOL("tidy.clean_output");

	return SUCCESS;
}

static CRX_MSHUTDOWN_FUNCTION(tidy)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

static CRX_MINFO_FUNCTION(tidy)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "Tidy support", "enabled");
#ifdef HAVE_TIDYBUFFIO_H
	crx_info_print_table_row(2, "libTidy Version", (char *)tidyLibraryVersion());
#elif defined(HAVE_TIDYP_H)
	crx_info_print_table_row(2, "libtidyp Version", (char *)tidyVersion());
#endif
#ifdef HAVE_TIDYRELEASEDATE
	crx_info_print_table_row(2, "libTidy Release", (char *)tidyReleaseDate());
#endif
	crx_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}

static CRX_INI_MH(crx_tidy_set_clean_output)
{
	int status;
	bool value;

	value = crex_ini_parse_bool(new_value);

	if (stage == CRX_INI_STAGE_RUNTIME) {
		status = crx_output_get_status();

		if (value && (status & CRX_OUTPUT_WRITTEN)) {
			crx_error_docref(NULL, E_WARNING, "Cannot enable tidy.clean_output - there has already been output");
			return FAILURE;
		}
		if (status & CRX_OUTPUT_SENT) {
			crx_error_docref(NULL, E_WARNING, "Cannot change tidy.clean_output - headers already sent");
			return FAILURE;
		}
	}

	status = OnUpdateBool(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);

	if (stage == CRX_INI_STAGE_RUNTIME && value) {
		if (!crx_output_handler_started(CREX_STRL("ob_tidyhandler"))) {
			crx_tidy_clean_output_start(CREX_STRL("ob_tidyhandler"));
		}
	}

	return status;
}

/*
 * NOTE: tidy does not support iterative/cumulative parsing, so chunk-sized output handler is not possible
 */

static void crx_tidy_clean_output_start(const char *name, size_t name_len)
{
	crx_output_handler *h;

	if (TG(clean_output) && (h = crx_tidy_output_handler_init(name, name_len, 0, CRX_OUTPUT_HANDLER_STDFLAGS))) {
		crx_output_handler_start(h);
	}
}

static crx_output_handler *crx_tidy_output_handler_init(const char *handler_name, size_t handler_name_len, size_t chunk_size, int flags)
{
	if (chunk_size) {
		crx_error_docref(NULL, E_WARNING, "Cannot use a chunk size for ob_tidyhandler");
		return NULL;
	}
	if (!TG(clean_output)) {
		TG(clean_output) = 1;
	}
	return crx_output_handler_create_internal(handler_name, handler_name_len, crx_tidy_output_handler, chunk_size, flags);
}

static int crx_tidy_output_handler(void **nothing, crx_output_context *output_context)
{
	int status = FAILURE;
	TidyDoc doc;
	TidyBuffer inbuf, outbuf, errbuf;

	if (TG(clean_output) && (output_context->op & CRX_OUTPUT_HANDLER_START) && (output_context->op & CRX_OUTPUT_HANDLER_FINAL)) {
		doc = tidyCreate();
		tidyBufInit(&errbuf);

		if (0 == tidySetErrorBuffer(doc, &errbuf)) {
			tidyOptSetBool(doc, TidyForceOutput, yes);
			tidyOptSetBool(doc, TidyMark, no);

			if (CREX_SIZE_T_UINT_OVFL(output_context->in.used)) {
				crx_error_docref(NULL, E_WARNING, "Input string is too long");
				return status;
			}

			TIDY_SET_DEFAULT_CONFIG(doc);

			tidyBufInit(&inbuf);
			tidyBufAttach(&inbuf, (byte *) output_context->in.data, (uint32_t)output_context->in.used);

			if (0 <= tidyParseBuffer(doc, &inbuf) && 0 <= tidyCleanAndRepair(doc)) {
				tidyBufInit(&outbuf);
				tidySaveBuffer(doc, &outbuf);
				FIX_BUFFER(&outbuf);
				output_context->out.data = (char *) outbuf.bp;
				output_context->out.used = outbuf.size ? outbuf.size-1 : 0;
				output_context->out.free = 1;
				status = SUCCESS;
			}
		}

		tidyRelease(doc);
		tidyBufFree(&errbuf);
	}

	return status;
}

/* {{{ Parse a document stored in a string */
CRX_FUNCTION(tidy_parse_string)
{
	char *enc = NULL;
	size_t enc_len = 0;
	crex_string *input, *options_str = NULL;
	HashTable *options_ht = NULL;
	CRXTidyObj *obj;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_STR(input)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT_OR_STR_OR_NULL(options_ht, options_str)
		C_PARAM_STRING_OR_NULL(enc, enc_len)
	CREX_PARSE_PARAMETERS_END();

	if (CREX_SIZE_T_UINT_OVFL(ZSTR_LEN(input))) {
		crex_argument_value_error(1, "is too long");
		RETURN_THROWS();
	}

	tidy_instantiate(tidy_ce_doc, return_value);
	obj = C_TIDY_P(return_value);

	TIDY_APPLY_CONFIG(obj->ptdoc->doc, options_str, options_ht);

	if (crx_tidy_parse_string(obj, ZSTR_VAL(input), (uint32_t)ZSTR_LEN(input), enc) == FAILURE) {
		zval_ptr_dtor(return_value);
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Return warnings and errors which occurred parsing the specified document*/
CRX_FUNCTION(tidy_get_error_buffer)
{
	TIDY_FETCH_OBJECT;

	if (obj->ptdoc->errbuf && obj->ptdoc->errbuf->bp) {
		RETURN_STRINGL((char*)obj->ptdoc->errbuf->bp, obj->ptdoc->errbuf->size-1);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Return a string representing the parsed tidy markup */
CRX_FUNCTION(tidy_get_output)
{
	TidyBuffer output;
	TIDY_FETCH_OBJECT;

	tidyBufInit(&output);
	tidySaveBuffer(obj->ptdoc->doc, &output);
	FIX_BUFFER(&output);
	RETVAL_STRINGL((char *) output.bp, output.size ? output.size-1 : 0);
	tidyBufFree(&output);
}
/* }}} */

/* {{{ Parse markup in file or URI */
CRX_FUNCTION(tidy_parse_file)
{
	char *enc = NULL;
	size_t enc_len = 0;
	bool use_include_path = 0;
	crex_string *inputfile, *contents, *options_str = NULL;
	HashTable *options_ht = NULL;

	CRXTidyObj *obj;

	CREX_PARSE_PARAMETERS_START(1, 4)
		C_PARAM_PATH_STR(inputfile)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT_OR_STR_OR_NULL(options_ht, options_str)
		C_PARAM_STRING_OR_NULL(enc, enc_len)
		C_PARAM_BOOL(use_include_path)
	CREX_PARSE_PARAMETERS_END();

	if (!(contents = crx_tidy_file_to_mem(ZSTR_VAL(inputfile), use_include_path))) {
		crx_error_docref(NULL, E_WARNING, "Cannot load \"%s\" into memory%s", ZSTR_VAL(inputfile), (use_include_path) ? " (using include path)" : "");
		RETURN_FALSE;
	}

	if (CREX_SIZE_T_UINT_OVFL(ZSTR_LEN(contents))) {
		crex_string_release_ex(contents, 0);
		crex_value_error("Input string is too long");
		RETURN_THROWS();
	}

	tidy_instantiate(tidy_ce_doc, return_value);
	obj = C_TIDY_P(return_value);

	TIDY_APPLY_CONFIG(obj->ptdoc->doc, options_str, options_ht);

	if (crx_tidy_parse_string(obj, ZSTR_VAL(contents), (uint32_t)ZSTR_LEN(contents), enc) == FAILURE) {
		zval_ptr_dtor(return_value);
		RETVAL_FALSE;
	}

	crex_string_release_ex(contents, 0);
}
/* }}} */

/* {{{ Execute configured cleanup and repair operations on parsed markup */
CRX_FUNCTION(tidy_clean_repair)
{
	TIDY_FETCH_OBJECT;

	if (tidyCleanAndRepair(obj->ptdoc->doc) >= 0) {
		tidy_doc_update_properties(obj);
		RETURN_TRUE;
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ Repair a string using an optionally provided configuration file */
CRX_FUNCTION(tidy_repair_string)
{
	crx_tidy_quick_repair(INTERNAL_FUNCTION_PARAM_PASSTHRU, false);
}
/* }}} */

/* {{{ Repair a file using an optionally provided configuration file */
CRX_FUNCTION(tidy_repair_file)
{
	crx_tidy_quick_repair(INTERNAL_FUNCTION_PARAM_PASSTHRU, true);
}
/* }}} */

/* {{{ Run configured diagnostics on parsed and repaired markup. */
CRX_FUNCTION(tidy_diagnose)
{
	TIDY_FETCH_OBJECT;

	if (obj->ptdoc->initialized && tidyRunDiagnostics(obj->ptdoc->doc) >= 0) {
		tidy_doc_update_properties(obj);
		RETURN_TRUE;
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ Get release date (version) for Tidy library */
CRX_FUNCTION(tidy_get_release)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

#ifdef HAVE_TIDYRELEASEDATE
	RETURN_STRING((char *)tidyReleaseDate());
#else
	RETURN_STRING((char *)"unknown");
#endif
}
/* }}} */


#ifdef HAVE_TIDYOPTGETDOC
/* {{{ Returns the documentation for the given option name */
CRX_FUNCTION(tidy_get_opt_doc)
{
	CRXTidyObj *obj;
	char *optval, *optname;
	size_t optname_len;
	TidyOption opt;
	zval *object;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "Os", &object, tidy_ce_doc, &optname, &optname_len) == FAILURE) {
		RETURN_THROWS();
	}

	obj = C_TIDY_P(object);

	opt = tidyGetOptionByName(obj->ptdoc->doc, optname);

	if (!opt) {
		crex_argument_value_error(getThis() ? 1 : 2, "is an invalid configuration option, \"%s\" given", optname);
		RETURN_THROWS();
	}

	if ( (optval = (char *) tidyOptGetDoc(obj->ptdoc->doc, opt)) ) {
		RETURN_STRING(optval);
	}

	RETURN_FALSE;
}
/* }}} */
#endif


/* {{{ Get current Tidy configuration */
CRX_FUNCTION(tidy_get_config)
{
	TidyIterator itOpt;
	char *opt_name;
	void *opt_value;
	TidyOptionType optt;

	TIDY_FETCH_OBJECT;

	itOpt = tidyGetOptionList(obj->ptdoc->doc);

	array_init(return_value);

	while (itOpt) {
		TidyOption opt = tidyGetNextOption(obj->ptdoc->doc, &itOpt);

		opt_name = (char *)tidyOptGetName(opt);
		opt_value = crx_tidy_get_opt_val(obj->ptdoc, opt, &optt);
		switch (optt) {
			case TidyString:
				add_assoc_str(return_value, opt_name, (crex_string*)opt_value);
				break;

			case TidyInteger:
				add_assoc_long(return_value, opt_name, (crex_long)opt_value);
				break;

			case TidyBoolean:
				add_assoc_bool(return_value, opt_name, opt_value ? 1 : 0);
				break;
		}
	}

	return;
}
/* }}} */

/* {{{ Get status of specified document. */
CRX_FUNCTION(tidy_get_status)
{
	TIDY_FETCH_OBJECT;

	RETURN_LONG(tidyStatus(obj->ptdoc->doc));
}
/* }}} */

/* {{{ Get the Detected HTML version for the specified document. */
CRX_FUNCTION(tidy_get_html_ver)
{
	TIDY_FETCH_INITIALIZED_OBJECT;

	RETURN_LONG(tidyDetectedHtmlVersion(obj->ptdoc->doc));
}
/* }}} */

/* {{{ Indicates if the document is a XHTML document. */
CRX_FUNCTION(tidy_is_xhtml)
{
	TIDY_FETCH_INITIALIZED_OBJECT;

	RETURN_BOOL(tidyDetectedXhtml(obj->ptdoc->doc));
}
/* }}} */

/* {{{ Indicates if the document is a generic (non HTML/XHTML) XML document. */
CRX_FUNCTION(tidy_is_xml)
{
	TIDY_FETCH_INITIALIZED_OBJECT;

	RETURN_BOOL(tidyDetectedGenericXml(obj->ptdoc->doc));
}
/* }}} */

/* {{{ Returns the Number of Tidy errors encountered for specified document. */
CRX_FUNCTION(tidy_error_count)
{
	TIDY_FETCH_OBJECT;

	RETURN_LONG(tidyErrorCount(obj->ptdoc->doc));
}
/* }}} */

/* {{{ Returns the Number of Tidy warnings encountered for specified document. */
CRX_FUNCTION(tidy_warning_count)
{
	TIDY_FETCH_OBJECT;

	RETURN_LONG(tidyWarningCount(obj->ptdoc->doc));
}
/* }}} */

/* {{{ Returns the Number of Tidy accessibility warnings encountered for specified document. */
CRX_FUNCTION(tidy_access_count)
{
	TIDY_FETCH_OBJECT;

	RETURN_LONG(tidyAccessWarningCount(obj->ptdoc->doc));
}
/* }}} */

/* {{{ Returns the Number of Tidy configuration errors encountered for specified document. */
CRX_FUNCTION(tidy_config_count)
{
	TIDY_FETCH_OBJECT;

	RETURN_LONG(tidyConfigErrorCount(obj->ptdoc->doc));
}
/* }}} */

/* {{{ Returns the value of the specified configuration option for the tidy document. */
CRX_FUNCTION(tidy_getopt)
{
	CRXTidyObj *obj;
	char *optname;
	void *optval;
	size_t optname_len;
	TidyOption opt;
	TidyOptionType optt;
	zval *object;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "Os", &object, tidy_ce_doc, &optname, &optname_len) == FAILURE) {
		RETURN_THROWS();
	}

	obj = C_TIDY_P(object);

	opt = tidyGetOptionByName(obj->ptdoc->doc, optname);

	if (!opt) {
		crex_argument_value_error(getThis() ? 1 : 2, "is an invalid configuration option, \"%s\" given", optname);
		RETURN_THROWS();
	}

	optval = crx_tidy_get_opt_val(obj->ptdoc, opt, &optt);
	switch (optt) {
		case TidyString:
			RETVAL_STR((crex_string*)optval);
			return;

		case TidyInteger:
			RETURN_LONG((crex_long)optval);
			break;

		case TidyBoolean:
			if (optval) {
				RETURN_TRUE;
			} else {
				RETURN_FALSE;
			}
			break;

		default:
			crx_error_docref(NULL, E_WARNING, "Unable to determine type of configuration option");
			break;
	}

	RETURN_FALSE;
}
/* }}} */

CRX_METHOD(tidy, __main)
{
	char *enc = NULL;
	size_t enc_len = 0;
	bool use_include_path = 0;
	HashTable *options_ht = NULL;
	crex_string *contents, *inputfile = NULL, *options_str = NULL;
	CRXTidyObj *obj;

	CREX_PARSE_PARAMETERS_START(0, 4)
		C_PARAM_OPTIONAL
		C_PARAM_PATH_STR_OR_NULL(inputfile)
		C_PARAM_ARRAY_HT_OR_STR_OR_NULL(options_ht, options_str)
		C_PARAM_STRING_OR_NULL(enc, enc_len)
		C_PARAM_BOOL(use_include_path)
	CREX_PARSE_PARAMETERS_END();

	TIDY_SET_CONTEXT;
	obj = C_TIDY_P(object);

	if (inputfile) {
		if (!(contents = crx_tidy_file_to_mem(ZSTR_VAL(inputfile), use_include_path))) {
			crx_error_docref(NULL, E_WARNING, "Cannot load \"%s\" into memory%s", ZSTR_VAL(inputfile), (use_include_path) ? " (using include path)" : "");
			return;
		}

		if (CREX_SIZE_T_UINT_OVFL(ZSTR_LEN(contents))) {
			crex_string_release_ex(contents, 0);
			crex_value_error("Input string is too long");
			RETURN_THROWS();
		}

		TIDY_APPLY_CONFIG(obj->ptdoc->doc, options_str, options_ht);

		crx_tidy_parse_string(obj, ZSTR_VAL(contents), (uint32_t)ZSTR_LEN(contents), enc);

		crex_string_release_ex(contents, 0);
	}
}

CRX_METHOD(tidy, parseFile)
{
	char *enc = NULL;
	size_t enc_len = 0;
	bool use_include_path = 0;
	HashTable *options_ht = NULL;
	crex_string *inputfile, *contents, *options_str = NULL;
	CRXTidyObj *obj;

	CREX_PARSE_PARAMETERS_START(1, 4)
		C_PARAM_PATH_STR(inputfile)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT_OR_STR_OR_NULL(options_ht, options_str)
		C_PARAM_STRING_OR_NULL(enc, enc_len)
		C_PARAM_BOOL(use_include_path)
	CREX_PARSE_PARAMETERS_END();

	TIDY_SET_CONTEXT;
	obj = C_TIDY_P(object);

	if (!(contents = crx_tidy_file_to_mem(ZSTR_VAL(inputfile), use_include_path))) {
		crx_error_docref(NULL, E_WARNING, "Cannot load \"%s\" into memory%s", ZSTR_VAL(inputfile), (use_include_path) ? " (using include path)" : "");
		RETURN_FALSE;
	}

	if (CREX_SIZE_T_UINT_OVFL(ZSTR_LEN(contents))) {
		crex_string_release_ex(contents, 0);
		crex_value_error("Input string is too long");
		RETURN_THROWS();
	}

	TIDY_APPLY_CONFIG(obj->ptdoc->doc, options_str, options_ht);

	if (crx_tidy_parse_string(obj, ZSTR_VAL(contents), (uint32_t)ZSTR_LEN(contents), enc) == FAILURE) {
		RETVAL_FALSE;
	} else {
		RETVAL_TRUE;
	}

	crex_string_release_ex(contents, 0);
}

CRX_METHOD(tidy, parseString)
{
	char *enc = NULL;
	size_t enc_len = 0;
	HashTable *options_ht = NULL;
	CRXTidyObj *obj;
	crex_string *input, *options_str = NULL;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_STR(input)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT_OR_STR_OR_NULL(options_ht, options_str)
		C_PARAM_STRING_OR_NULL(enc, enc_len)
	CREX_PARSE_PARAMETERS_END();

	if (CREX_SIZE_T_UINT_OVFL(ZSTR_LEN(input))) {
		crex_argument_value_error(1, "is too long");
		RETURN_THROWS();
	}

	TIDY_SET_CONTEXT;
	obj = C_TIDY_P(object);

	TIDY_APPLY_CONFIG(obj->ptdoc->doc, options_str, options_ht);

	if(crx_tidy_parse_string(obj, ZSTR_VAL(input), (uint32_t)ZSTR_LEN(input), enc) == SUCCESS) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}


/* {{{ Returns a TidyNode Object representing the root of the tidy parse tree */
CRX_FUNCTION(tidy_get_root)
{
	crx_tidy_create_node(INTERNAL_FUNCTION_PARAM_PASSTHRU, is_root_node);
}
/* }}} */

/* {{{ Returns a TidyNode Object starting from the <HTML> tag of the tidy parse tree */
CRX_FUNCTION(tidy_get_html)
{
	crx_tidy_create_node(INTERNAL_FUNCTION_PARAM_PASSTHRU, is_html_node);
}
/* }}} */

/* {{{ Returns a TidyNode Object starting from the <HEAD> tag of the tidy parse tree */
CRX_FUNCTION(tidy_get_head)
{
	crx_tidy_create_node(INTERNAL_FUNCTION_PARAM_PASSTHRU, is_head_node);
}
/* }}} */

/* {{{ Returns a TidyNode Object starting from the <BODY> tag of the tidy parse tree */
CRX_FUNCTION(tidy_get_body)
{
	crx_tidy_create_node(INTERNAL_FUNCTION_PARAM_PASSTHRU, is_body_node);
}
/* }}} */

/* {{{ Returns true if this node has children */
CRX_METHOD(tidyNode, hasChildren)
{
	TIDY_FETCH_ONLY_OBJECT;

	if (tidyGetChild(obj->node)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Returns true if this node has siblings */
CRX_METHOD(tidyNode, hasSiblings)
{
	TIDY_FETCH_ONLY_OBJECT;

	if (obj->node && tidyGetNext(obj->node)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Returns true if this node represents a comment */
CRX_METHOD(tidyNode, isComment)
{
	TIDY_FETCH_ONLY_OBJECT;

	if (tidyNodeGetType(obj->node) == TidyNode_Comment) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Returns true if this node is part of a HTML document */
CRX_METHOD(tidyNode, isHtml)
{
	TIDY_FETCH_ONLY_OBJECT;

	switch (tidyNodeGetType(obj->node)) {
		case TidyNode_Start:
		case TidyNode_End:
		case TidyNode_StartEnd:
			RETURN_TRUE;
		default:
			RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Returns true if this node represents text (no markup) */
CRX_METHOD(tidyNode, isText)
{
	TIDY_FETCH_ONLY_OBJECT;

	if (tidyNodeGetType(obj->node) == TidyNode_Text) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Returns true if this node is JSTE */
CRX_METHOD(tidyNode, isJste)
{
	TIDY_FETCH_ONLY_OBJECT;

	if (tidyNodeGetType(obj->node) == TidyNode_Jste) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Returns true if this node is ASP */
CRX_METHOD(tidyNode, isAsp)
{
	TIDY_FETCH_ONLY_OBJECT;

	if (tidyNodeGetType(obj->node) == TidyNode_Asp) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Returns true if this node is CRX */
CRX_METHOD(tidyNode, isCrx)
{
	TIDY_FETCH_ONLY_OBJECT;

	if (tidyNodeGetType(obj->node) == TidyNode_Crx) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Returns the parent node if available or NULL */
CRX_METHOD(tidyNode, getParent)
{
	TidyNode	parent_node;
	CRXTidyObj *newobj;
	TIDY_FETCH_ONLY_OBJECT;

	parent_node = tidyGetParent(obj->node);
	if(parent_node) {
		tidy_instantiate(tidy_ce_node, return_value);
		newobj = C_TIDY_P(return_value);
		newobj->node = parent_node;
		newobj->type = is_node;
		newobj->ptdoc = obj->ptdoc;
		newobj->ptdoc->ref_count++;
		tidy_add_node_default_properties(newobj);
	} else {
		ZVAL_NULL(return_value);
	}
}
/* }}} */


/* {{{ __mainor for tidyNode. */
CRX_METHOD(tidyNode, __main)
{
	crex_throw_error(NULL, "You should not create a tidyNode manually");
}
/* }}} */

#endif
