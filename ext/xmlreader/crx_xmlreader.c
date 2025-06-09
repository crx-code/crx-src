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
  | Author: Rob Richards <rrichards@crx.net>                             |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "crx.h"
#include "crx_ini.h"
#include "ext/standard/info.h"
#include "crex_observer.h"
#include "crx_xmlreader.h"
#ifdef HAVE_DOM
#include "ext/dom/xml_common.h"
#include "ext/dom/dom_ce.h"
#endif
#include <libxml/xmlreader.h>
#include <libxml/uri.h>
#include "crx_xmlreader_arginfo.h"

crex_class_entry *xmlreader_class_entry;

static crex_object_handlers xmlreader_object_handlers;

static HashTable xmlreader_prop_handlers;

static crex_internal_function xmlreader_open_fn;
static crex_internal_function xmlreader_xml_fn;

typedef int (*xmlreader_read_int_t)(xmlTextReaderPtr reader);
typedef unsigned char *(*xmlreader_read_char_t)(xmlTextReaderPtr reader);
typedef const unsigned char *(*xmlreader_read_const_char_t)(xmlTextReaderPtr reader);
typedef int (*xmlreader_write_t)(xmlreader_object *obj, zval *newval);

typedef unsigned char *(*xmlreader_read_one_char_t)(xmlTextReaderPtr reader, const unsigned char *);

typedef struct _xmlreader_prop_handler {
	xmlreader_read_int_t read_int_func;
	xmlreader_read_const_char_t read_char_func;
	xmlreader_write_t write_func;
	int type;
} xmlreader_prop_handler;

#define XMLREADER_LOAD_STRING 0
#define XMLREADER_LOAD_FILE 1

/* {{{ xmlreader_register_prop_handler */
static void xmlreader_register_prop_handler(HashTable *prop_handler, char *name, xmlreader_read_int_t read_int_func, xmlreader_read_const_char_t read_char_func, int rettype)
{
	xmlreader_prop_handler hnd;
	crex_string *str;

	hnd.read_char_func = read_char_func;
	hnd.read_int_func = read_int_func;
	hnd.type = rettype;
	str = crex_string_init_interned(name, strlen(name), 1);
	crex_hash_add_mem(prop_handler, str, &hnd, sizeof(xmlreader_prop_handler));
	crex_string_release_ex(str, 1);
}
/* }}} */

/* {{{ xmlreader_property_reader */
static int xmlreader_property_reader(xmlreader_object *obj, xmlreader_prop_handler *hnd, zval *rv)
{
	const xmlChar *retchar = NULL;
	int retint = 0;

	if (obj->ptr != NULL) {
		if (hnd->read_char_func) {
			retchar = hnd->read_char_func(obj->ptr);
		} else {
			if (hnd->read_int_func) {
				retint = hnd->read_int_func(obj->ptr);
				if (retint == -1) {
					crex_throw_error(NULL, "Failed to read property due to libxml error");
					return FAILURE;
				}
			}
		}
	}

	switch (hnd->type) {
		case IS_STRING:
			if (retchar) {
				ZVAL_STRING(rv, (char *) retchar);
			} else {
				ZVAL_EMPTY_STRING(rv);
			}
			break;
		case _IS_BOOL:
			ZVAL_BOOL(rv, retint);
			break;
		case IS_LONG:
			ZVAL_LONG(rv, retint);
			break;
		EMPTY_SWITCH_DEFAULT_CASE()
	}

	return SUCCESS;
}
/* }}} */

/* {{{ xmlreader_get_property_ptr_ptr */
zval *xmlreader_get_property_ptr_ptr(crex_object *object, crex_string *name, int type, void **cache_slot)
{
	xmlreader_object *obj;
	zval *retval = NULL;
	xmlreader_prop_handler *hnd = NULL;

	obj = crx_xmlreader_fetch_object(object);

	if (obj->prop_handler != NULL) {
		hnd = crex_hash_find_ptr(obj->prop_handler, name);
	}

	if (hnd == NULL) {
		retval = crex_std_get_property_ptr_ptr(object, name, type, cache_slot);
	}

	return retval;
}
/* }}} */

/* {{{ xmlreader_read_property */
zval *xmlreader_read_property(crex_object *object, crex_string *name, int type, void **cache_slot, zval *rv)
{
	xmlreader_object *obj;
	zval *retval = NULL;
	xmlreader_prop_handler *hnd = NULL;

	obj = crx_xmlreader_fetch_object(object);

	if (obj->prop_handler != NULL) {
		hnd = crex_hash_find_ptr(obj->prop_handler, name);
	}

	if (hnd != NULL) {
		if (xmlreader_property_reader(obj, hnd, rv) == FAILURE) {
			retval = &EG(uninitialized_zval);
		} else {
			retval = rv;
		}
	} else {
		retval = crex_std_read_property(object, name, type, cache_slot, rv);
	}

	return retval;
}
/* }}} */

/* {{{ xmlreader_write_property */
zval *xmlreader_write_property(crex_object *object, crex_string *name, zval *value, void **cache_slot)
{
	xmlreader_object *obj;
	xmlreader_prop_handler *hnd = NULL;

	obj = crx_xmlreader_fetch_object(object);

	if (obj->prop_handler != NULL) {
		hnd = crex_hash_find_ptr(obj->prop_handler, name);
	}
	if (hnd != NULL) {
		crex_throw_error(NULL, "Cannot write to read-only property");
	} else {
		value = crex_std_write_property(object, name, value, cache_slot);
	}

	return value;
}
/* }}} */

/* {{{ */
static crex_function *xmlreader_get_method(crex_object **obj, crex_string *name, const zval *key)
{
	if (ZSTR_LEN(name) == sizeof("open") - 1
			&& (ZSTR_VAL(name)[0] == 'o' || ZSTR_VAL(name)[0] == 'O')
			&& (ZSTR_VAL(name)[1] == 'p' || ZSTR_VAL(name)[1] == 'P')
			&& (ZSTR_VAL(name)[2] == 'e' || ZSTR_VAL(name)[2] == 'E')
			&& (ZSTR_VAL(name)[3] == 'n' || ZSTR_VAL(name)[3] == 'N')) {
		return (crex_function*)&xmlreader_open_fn;
	} else if (ZSTR_LEN(name) == sizeof("xml") - 1
			&& (ZSTR_VAL(name)[0] == 'x' || ZSTR_VAL(name)[0] == 'X')
			&& (ZSTR_VAL(name)[1] == 'm' || ZSTR_VAL(name)[1] == 'M')
			&& (ZSTR_VAL(name)[2] == 'l' || ZSTR_VAL(name)[2] == 'L')) {
		return (crex_function*)&xmlreader_xml_fn;
	}
	return crex_std_get_method(obj, name, key);;
}
/* }}} */

/* {{{ _xmlreader_get_valid_file_path */
/* _xmlreader_get_valid_file_path and _xmlreader_get_relaxNG should be made a
	common function in libxml extension as code is common to a few xml extensions */
char *_xmlreader_get_valid_file_path(char *source, char *resolved_path, int resolved_path_len ) {
	xmlURI *uri;
	xmlChar *escsource;
	char *file_dest;
	int isFileUri = 0;

	uri = xmlCreateURI();
	if (uri == NULL) {
		return NULL;
	}
	escsource = xmlURIEscapeStr((xmlChar *)source, (xmlChar *)":");
	xmlParseURIReference(uri, (const char *)escsource);
	xmlFree(escsource);

	if (uri->scheme != NULL) {
		/* absolute file uris - libxml only supports localhost or empty host */
		if (strncasecmp(source, "file:///",8) == 0) {
			isFileUri = 1;
#ifdef CRX_WIN32
			source += 8;
#else
			source += 7;
#endif
		} else if (strncasecmp(source, "file://localhost/",17) == 0) {
			isFileUri = 1;
#ifdef CRX_WIN32
			source += 17;
#else
			source += 16;
#endif
		}
	}

	file_dest = source;

	if ((uri->scheme == NULL || isFileUri)) {
		if (!VCWD_REALPATH(source, resolved_path) && !expand_filepath(source, resolved_path)) {
			xmlFreeURI(uri);
			return NULL;
		}
		file_dest = resolved_path;
	}

	xmlFreeURI(uri);

	return file_dest;
}
/* }}} */

#ifdef LIBXML_SCHEMAS_ENABLED
/* {{{ _xmlreader_get_relaxNG */
static xmlRelaxNGPtr _xmlreader_get_relaxNG(char *source, size_t source_len, size_t type,
											xmlRelaxNGValidityErrorFunc error_func,
											xmlRelaxNGValidityWarningFunc warn_func)
{
	char *valid_file = NULL;
	xmlRelaxNGParserCtxtPtr parser = NULL;
	xmlRelaxNGPtr           sptr;
	char resolved_path[MAXPATHLEN + 1];

	switch (type) {
	case XMLREADER_LOAD_FILE:
		valid_file = _xmlreader_get_valid_file_path(source, resolved_path, MAXPATHLEN );
		if (!valid_file) {
			return NULL;
		}
		parser = xmlRelaxNGNewParserCtxt(valid_file);
		break;
	case XMLREADER_LOAD_STRING:
		parser = xmlRelaxNGNewMemParserCtxt(source, source_len);
		/* If loading from memory, we need to set the base directory for the document
		   but it is not apparent how to do that for schema's */
		break;
	default:
		return NULL;
	}

	if (parser == NULL) {
		return NULL;
	}

	CRX_LIBXML_SANITIZE_GLOBALS(parse);
	if (error_func || warn_func) {
		xmlRelaxNGSetParserErrors(parser,
			(xmlRelaxNGValidityErrorFunc) error_func,
			(xmlRelaxNGValidityWarningFunc) warn_func,
			parser);
	}
	sptr = xmlRelaxNGParse(parser);
	xmlRelaxNGFreeParserCtxt(parser);
	CRX_LIBXML_RESTORE_GLOBALS(parse);

	return sptr;
}
/* }}} */
#endif

static const crex_module_dep xmlreader_deps[] = {
	CREX_MOD_REQUIRED("libxml")
	CREX_MOD_END
};

/* {{{ xmlreader_module_entry */
crex_module_entry xmlreader_module_entry = {
	STANDARD_MODULE_HEADER_EX, NULL,
	xmlreader_deps,
	"xmlreader",
	NULL,
	CRX_MINIT(xmlreader),
	CRX_MSHUTDOWN(xmlreader),
	NULL,
	NULL,
	CRX_MINFO(xmlreader),
	CRX_XMLREADER_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_XMLREADER
CREX_GET_MODULE(xmlreader)
#endif

/* {{{ xmlreader_objects_clone */
void xmlreader_objects_clone(void *object, void **object_clone)
{
	/* TODO */
}
/* }}} */

/* {{{ xmlreader_free_resources */
static void xmlreader_free_resources(xmlreader_object *intern) {
	if (intern->input) {
		xmlFreeParserInputBuffer(intern->input);
		intern->input = NULL;
	}

	if (intern->ptr) {
		xmlFreeTextReader(intern->ptr);
		intern->ptr = NULL;
	}
#ifdef LIBXML_SCHEMAS_ENABLED
	if (intern->schema) {
		xmlRelaxNGFree((xmlRelaxNGPtr) intern->schema);
		intern->schema = NULL;
	}
#endif
}
/* }}} */

/* {{{ xmlreader_objects_free_storage */
void xmlreader_objects_free_storage(crex_object *object)
{
	xmlreader_object *intern = crx_xmlreader_fetch_object(object);

	crex_object_std_dtor(&intern->std);

	xmlreader_free_resources(intern);
}
/* }}} */

/* {{{ xmlreader_objects_new */
crex_object *xmlreader_objects_new(crex_class_entry *class_type)
{
	xmlreader_object *intern;

	intern = crex_object_alloc(sizeof(xmlreader_object), class_type);
	crex_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);
	intern->prop_handler = &xmlreader_prop_handlers;

	return &intern->std;
}
/* }}} */

/* {{{ crx_xmlreader_string_arg */
static void crx_xmlreader_string_arg(INTERNAL_FUNCTION_PARAMETERS, xmlreader_read_one_char_t internal_function) {
	zval *id;
	size_t name_len = 0;
	char *retchar = NULL;
	xmlreader_object *intern;
	char *name;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s", &name, &name_len) == FAILURE) {
		RETURN_THROWS();
	}

	if (!name_len) {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	}

	id = CREX_THIS;

	intern = C_XMLREADER_P(id);
	if (intern->ptr) {
		retchar = (char *)internal_function(intern->ptr, (const unsigned char *)name);
	}
	if (retchar) {
		RETVAL_STRING(retchar);
		xmlFree(retchar);
		return;
	} else {
		RETVAL_NULL();
	}
}
/* }}} */

/* {{{ crx_xmlreader_no_arg */
static void crx_xmlreader_no_arg(INTERNAL_FUNCTION_PARAMETERS, xmlreader_read_int_t internal_function) {
	zval *id;
	int retval;
	xmlreader_object *intern;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	id = CREX_THIS;

	intern = C_XMLREADER_P(id);
	if (intern->ptr) {
		retval = internal_function(intern->ptr);
		if (retval == 1) {
			RETURN_TRUE;
		}
	}

	RETURN_FALSE;
}
/* }}} */

static void crx_xmlreader_free_prop_handler(zval *el) /* {{{ */ {
	pefree(C_PTR_P(el), 1);
} /* }}} */

/* {{{ crx_xmlreader_no_arg_string */
static void crx_xmlreader_no_arg_string(INTERNAL_FUNCTION_PARAMETERS, xmlreader_read_char_t internal_function) {
	zval *id;
	char *retchar = NULL;
	xmlreader_object *intern;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	id = CREX_THIS;

	intern = C_XMLREADER_P(id);
	if (intern->ptr) {
		retchar = (char *)internal_function(intern->ptr);
	}
	if (retchar) {
		RETVAL_STRING(retchar);
		xmlFree(retchar);
		return;
	} else {
		RETVAL_EMPTY_STRING();
	}
}
/* }}} */

/* {{{ crx_xmlreader_set_relaxng_schema */
static void crx_xmlreader_set_relaxng_schema(INTERNAL_FUNCTION_PARAMETERS, int type) {
#ifdef LIBXML_SCHEMAS_ENABLED
	zval *id;
	size_t source_len = 0;
	int retval = -1;
	xmlreader_object *intern;
	xmlRelaxNGPtr schema = NULL;
	char *source;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "p!", &source, &source_len) == FAILURE) {
		RETURN_THROWS();
	}

	if (source != NULL && !source_len) {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	}

	id = CREX_THIS;

	intern = C_XMLREADER_P(id);
	if (intern->ptr) {
		if (source) {
			schema =  _xmlreader_get_relaxNG(source, source_len, type, NULL, NULL);
			if (schema) {
				retval = xmlTextReaderRelaxNGSetSchema(intern->ptr, schema);
			}
		} else {
			/* unset the associated relaxNG context and schema if one exists */
			retval = xmlTextReaderRelaxNGSetSchema(intern->ptr, NULL);
		}

		if (retval == 0) {
			if (intern->schema) {
				xmlRelaxNGFree((xmlRelaxNGPtr) intern->schema);
			}

			intern->schema = schema;

			RETURN_TRUE;
		} else {
			crx_error_docref(NULL, E_WARNING, "Schema contains errors");
			RETURN_FALSE;
		}
	} else {
		crex_throw_error(NULL, "Schema must be set prior to reading");
		RETURN_THROWS();
	}
#else
	crx_error_docref(NULL, E_WARNING, "No schema support built into libxml");
	RETURN_FALSE;
#endif
}
/* }}} */

/* {{{ Closes xmlreader - current frees resources until xmlTextReaderClose is fixed in libxml */
CRX_METHOD(XMLReader, close)
{
	zval *id;
	xmlreader_object *intern;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	id = CREX_THIS;
	intern = C_XMLREADER_P(id);
	/* libxml is segfaulting in versions up to 2.6.8 using xmlTextReaderClose so for
	now we will free the whole reader when close is called as it would get rebuilt on
	a new load anyways */
	xmlreader_free_resources(intern);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Get value of an attribute from current element */
CRX_METHOD(XMLReader, getAttribute)
{
	crx_xmlreader_string_arg(INTERNAL_FUNCTION_PARAM_PASSTHRU, xmlTextReaderGetAttribute);
}
/* }}} */

/* {{{ Get value of an attribute at index from current element */
CRX_METHOD(XMLReader, getAttributeNo)
{
	zval *id;
	crex_long attr_pos;
	char *retchar = NULL;
	xmlreader_object *intern;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &attr_pos) == FAILURE) {
		RETURN_THROWS();
	}

	id = CREX_THIS;

	intern = C_XMLREADER_P(id);
	if (intern->ptr) {
		retchar = (char *)xmlTextReaderGetAttributeNo(intern->ptr, attr_pos);
	}
	if (retchar) {
		RETVAL_STRING(retchar);
		xmlFree(retchar);
	}
}
/* }}} */

/* {{{ Get value of a attribute via name and namespace from current element */
CRX_METHOD(XMLReader, getAttributeNs)
{
	zval *id;
	size_t name_len = 0, ns_uri_len = 0;
	xmlreader_object *intern;
	char *name, *ns_uri, *retchar = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss", &name, &name_len, &ns_uri, &ns_uri_len) == FAILURE) {
		RETURN_THROWS();
	}

	if (name_len == 0) {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	}

	if (ns_uri_len == 0) {
		crex_argument_value_error(2, "cannot be empty");
		RETURN_THROWS();
	}

	id = CREX_THIS;

	intern = C_XMLREADER_P(id);
	if (intern->ptr) {
		retchar = (char *)xmlTextReaderGetAttributeNs(intern->ptr, (xmlChar *)name, (xmlChar *)ns_uri);
	}
	if (retchar) {
		RETVAL_STRING(retchar);
		xmlFree(retchar);
	}
}
/* }}} */

/* {{{ Indicates whether given property (one of the parser option constants) is set or not on parser */
CRX_METHOD(XMLReader, getParserProperty)
{
	zval *id;
	crex_long property;
	xmlreader_object *intern;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &property) == FAILURE) {
		RETURN_THROWS();
	}

	id = CREX_THIS;

	intern = C_XMLREADER_P(id);
	if (!intern || !intern->ptr) {
		crex_throw_error(NULL, "Cannot access parser properties before loading data");
		RETURN_THROWS();
	}

	int retval = xmlTextReaderGetParserProp(intern->ptr,property);
	if (retval == -1) {
		crex_argument_value_error(1, "must be a valid parser property");
		RETURN_THROWS();
	}

	RETURN_BOOL(retval);
}
/* }}} */

/* {{{ Returns boolean indicating if parsed document is valid or not.
Must set XMLREADER_LOADDTD or XMLREADER_VALIDATE parser option prior to the first call to read
or this method will always return FALSE */
CRX_METHOD(XMLReader, isValid)
{
	crx_xmlreader_no_arg(INTERNAL_FUNCTION_PARAM_PASSTHRU, xmlTextReaderIsValid);
}
/* }}} */

/* {{{ Return namespaceURI for associated prefix on current node */
CRX_METHOD(XMLReader, lookupNamespace)
{
	crx_xmlreader_string_arg(INTERNAL_FUNCTION_PARAM_PASSTHRU, xmlTextReaderLookupNamespace);
}
/* }}} */

/* {{{ Positions reader at specified attribute - Returns TRUE on success and FALSE on failure */
CRX_METHOD(XMLReader, moveToAttribute)
{
	zval *id;
	size_t name_len = 0;
	int retval;
	xmlreader_object *intern;
	char *name;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s", &name, &name_len) == FAILURE) {
		RETURN_THROWS();
	}

	if (name_len == 0) {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	}

	id = CREX_THIS;

	intern = C_XMLREADER_P(id);
	if (intern->ptr) {
		retval = xmlTextReaderMoveToAttribute(intern->ptr, (xmlChar *)name);
		if (retval == 1) {
			RETURN_TRUE;
		}
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ Positions reader at attribute at specified index.
Returns TRUE on success and FALSE on failure */
CRX_METHOD(XMLReader, moveToAttributeNo)
{
	zval *id;
	crex_long attr_pos;
	int retval;
	xmlreader_object *intern;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &attr_pos) == FAILURE) {
		RETURN_THROWS();
	}

	id = CREX_THIS;

	intern = C_XMLREADER_P(id);
	if (intern->ptr) {
		retval = xmlTextReaderMoveToAttributeNo(intern->ptr, attr_pos);
		if (retval == 1) {
			RETURN_TRUE;
		}
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ Positions reader at attribute spcified by name and namespaceURI.
Returns TRUE on success and FALSE on failure */
CRX_METHOD(XMLReader, moveToAttributeNs)
{
	zval *id;
	size_t name_len=0, ns_uri_len=0;
	int retval;
	xmlreader_object *intern;
	char *name, *ns_uri;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss", &name, &name_len, &ns_uri, &ns_uri_len) == FAILURE) {
		RETURN_THROWS();
	}

	if (name_len == 0) {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	}

	if (ns_uri_len == 0) {
		crex_argument_value_error(2, "cannot be empty");
		RETURN_THROWS();
	}

	id = CREX_THIS;

	intern = C_XMLREADER_P(id);
	if (intern->ptr) {
		retval = xmlTextReaderMoveToAttributeNs(intern->ptr, (xmlChar *)name, (xmlChar *)ns_uri);
		if (retval == 1) {
			RETURN_TRUE;
		}
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ Moves the position of the current instance to the node that contains the current Attribute node. */
CRX_METHOD(XMLReader, moveToElement)
{
	crx_xmlreader_no_arg(INTERNAL_FUNCTION_PARAM_PASSTHRU, xmlTextReaderMoveToElement);
}
/* }}} */

/* {{{ Moves the position of the current instance to the first attribute associated with the current node. */
CRX_METHOD(XMLReader, moveToFirstAttribute)
{
	crx_xmlreader_no_arg(INTERNAL_FUNCTION_PARAM_PASSTHRU, xmlTextReaderMoveToFirstAttribute);
}
/* }}} */

/* {{{ Moves the position of the current instance to the next attribute associated with the current node. */
CRX_METHOD(XMLReader, moveToNextAttribute)
{
	crx_xmlreader_no_arg(INTERNAL_FUNCTION_PARAM_PASSTHRU, xmlTextReaderMoveToNextAttribute);
}
/* }}} */

/* {{{ Moves the position of the current instance to the next node in the stream. */
CRX_METHOD(XMLReader, read)
{
	zval *id;
	int retval;
	xmlreader_object *intern;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	id = CREX_THIS;
	intern = C_XMLREADER_P(id);
	if (!intern->ptr) {
		crex_throw_error(NULL, "Data must be loaded before reading");
		RETURN_THROWS();
	}

	retval = xmlTextReaderRead(intern->ptr);
	if (retval == -1) {
		RETURN_FALSE;
	} else {
		RETURN_BOOL(retval);
	}
}
/* }}} */

/* {{{ Moves the position of the current instance to the next node in the stream. */
CRX_METHOD(XMLReader, next)
{
	zval *id;
	int retval;
	size_t name_len=0;
	xmlreader_object *intern;
	char *name = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|s!", &name, &name_len) == FAILURE) {
		RETURN_THROWS();
	}

	id = CREX_THIS;
	intern = C_XMLREADER_P(id);
	if (intern->ptr) {
		retval = xmlTextReaderNext(intern->ptr);
		while (name != NULL && retval == 1) {
			if (xmlStrEqual(xmlTextReaderConstLocalName(intern->ptr), (xmlChar *)name)) {
				RETURN_TRUE;
			}
			retval = xmlTextReaderNext(intern->ptr);
		}
		if (retval == -1) {
			RETURN_FALSE;
		} else {
			RETURN_BOOL(retval);
		}
	}

	crex_throw_error(NULL, "Data must be loaded before reading");
}
/* }}} */

/* {{{ Sets the URI that the XMLReader will parse. */
CRX_METHOD(XMLReader, open)
{
	zval *id;
	size_t source_len = 0, encoding_len = 0;
	crex_long options = 0;
	xmlreader_object *intern = NULL;
	char *source, *valid_file = NULL;
	char *encoding = NULL;
	char resolved_path[MAXPATHLEN + 1];
	xmlTextReaderPtr reader = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "p|s!l", &source, &source_len, &encoding, &encoding_len, &options) == FAILURE) {
		RETURN_THROWS();
	}

	id = getThis();
	if (id != NULL) {
		CREX_ASSERT(instanceof_function(C_OBJCE_P(id), xmlreader_class_entry));
		intern = C_XMLREADER_P(id);
		xmlreader_free_resources(intern);
	}

	if (!source_len) {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	}

	if (encoding && CHECK_NULL_PATH(encoding, encoding_len)) {
		crx_error_docref(NULL, E_WARNING, "Encoding must not contain NUL bytes");
		RETURN_FALSE;
	}

	valid_file = _xmlreader_get_valid_file_path(source, resolved_path, MAXPATHLEN );

	if (valid_file) {
		CRX_LIBXML_SANITIZE_GLOBALS(reader_for_file);
		reader = xmlReaderForFile(valid_file, encoding, options);
		CRX_LIBXML_RESTORE_GLOBALS(reader_for_file);
	}

	if (reader == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to open source data");
		RETURN_FALSE;
	}

	if (id == NULL) {
		object_init_ex(return_value, xmlreader_class_entry);
		intern = C_XMLREADER_P(return_value);
		intern->ptr = reader;
		return;
	}

	intern->ptr = reader;

	RETURN_TRUE;

}
/* }}} */

/* Not Yet Implemented in libxml - functions exist just not coded
CRX_METHOD(XMLReader, resetState)
{

}
*/

/* {{{ Reads the contents of the current node, including child nodes and markup. */
CRX_METHOD(XMLReader, readInnerXml)
{
	crx_xmlreader_no_arg_string(INTERNAL_FUNCTION_PARAM_PASSTHRU, xmlTextReaderReadInnerXml);
}
/* }}} */

/* {{{ Reads the contents of the current node, including child nodes and markup. */
CRX_METHOD(XMLReader, readOuterXml)
{
	crx_xmlreader_no_arg_string(INTERNAL_FUNCTION_PARAM_PASSTHRU, xmlTextReaderReadOuterXml);
}
/* }}} */

/* {{{ Reads the contents of an element or a text node as a string. */
CRX_METHOD(XMLReader, readString)
{
	crx_xmlreader_no_arg_string(INTERNAL_FUNCTION_PARAM_PASSTHRU, xmlTextReaderReadString);
}
/* }}} */

/* {{{ Use W3C XSD schema to validate the document as it is processed. Activation is only possible before the first Read(). */
CRX_METHOD(XMLReader, setSchema)
{
#ifdef LIBXML_SCHEMAS_ENABLED
	zval *id;
	size_t source_len = 0;
	int retval = -1;
	xmlreader_object *intern;
	char *source;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "p!", &source, &source_len) == FAILURE) {
		RETURN_THROWS();
	}

	if (source != NULL && !source_len) {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	}

	id = CREX_THIS;

	intern = C_XMLREADER_P(id);
	if (intern && intern->ptr) {
		CRX_LIBXML_SANITIZE_GLOBALS(schema);
		retval = xmlTextReaderSchemaValidate(intern->ptr, source);
		CRX_LIBXML_RESTORE_GLOBALS(schema);

		if (retval == 0) {
			RETURN_TRUE;
		} else {
			crx_error_docref(NULL, E_WARNING, "Schema contains errors");
			RETURN_FALSE;
		}
	} else {
		crex_throw_error(NULL, "Schema must be set prior to reading");
		RETURN_THROWS();
	}
#else
	crx_error_docref(NULL, E_WARNING, "No schema support built into libxml");
	RETURN_FALSE;
#endif
}
/* }}} */

/* {{{ Sets parser property (one of the parser option constants).
Properties must be set after open() or XML() and before the first read() is called */
CRX_METHOD(XMLReader, setParserProperty)
{
	zval *id;
	crex_long property;
	bool value;
	xmlreader_object *intern;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "lb", &property, &value) == FAILURE) {
		RETURN_THROWS();
	}

	id = CREX_THIS;

	intern = C_XMLREADER_P(id);
	if (!intern || !intern->ptr) {
		crex_throw_error(NULL, "Cannot access parser properties before loading data");
		RETURN_THROWS();
	}

	int retval = xmlTextReaderSetParserProp(intern->ptr,property, value);
	if (retval == -1) {
		crex_argument_value_error(1, "must be a valid parser property");
		RETURN_THROWS();
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Sets the string that the XMLReader will parse. */
CRX_METHOD(XMLReader, setRelaxNGSchema)
{
	crx_xmlreader_set_relaxng_schema(INTERNAL_FUNCTION_PARAM_PASSTHRU, XMLREADER_LOAD_FILE);
}
/* }}} */

/* {{{ Sets the string that the XMLReader will parse. */
CRX_METHOD(XMLReader, setRelaxNGSchemaSource)
{
	crx_xmlreader_set_relaxng_schema(INTERNAL_FUNCTION_PARAM_PASSTHRU, XMLREADER_LOAD_STRING);
}
/* }}} */

/* TODO
XMLPUBFUN int XMLCALL
		    xmlTextReaderSetSchema	(xmlTextReaderPtr reader,
		    				 xmlSchemaPtr schema);
*/

/* {{{ Sets the string that the XMLReader will parse. */
CRX_METHOD(XMLReader, XML)
{
	zval *id;
	size_t source_len = 0, encoding_len = 0;
	crex_long options = 0;
	xmlreader_object *intern = NULL;
	char *source, *uri = NULL, *encoding = NULL;
	int resolved_path_len, ret = 0;
	char *directory=NULL, resolved_path[MAXPATHLEN + 1];
	xmlParserInputBufferPtr inputbfr;
	xmlTextReaderPtr reader;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s|s!l", &source, &source_len, &encoding, &encoding_len, &options) == FAILURE) {
		RETURN_THROWS();
	}

	id = getThis();
	if (id != NULL) {
		CREX_ASSERT(instanceof_function(C_OBJCE_P(id), xmlreader_class_entry));
		intern = C_XMLREADER_P(id);
		xmlreader_free_resources(intern);
	}

	if (!source_len) {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	}

	if (encoding && CHECK_NULL_PATH(encoding, encoding_len)) {
		crx_error_docref(NULL, E_WARNING, "Encoding must not contain NUL bytes");
		RETURN_FALSE;
	}

	inputbfr = xmlParserInputBufferCreateMem(source, source_len, XML_CHAR_ENCODING_NONE);

	if (inputbfr != NULL) {
/* Get the URI of the current script so that we can set the base directory in libxml */
#ifdef HAVE_GETCWD
		directory = VCWD_GETCWD(resolved_path, MAXPATHLEN);
#elif defined(HAVE_GETWD)
		directory = VCWD_GETWD(resolved_path);
#endif
		if (directory) {
			resolved_path_len = strlen(resolved_path);
			if (resolved_path[resolved_path_len - 1] != DEFAULT_SLASH) {
				resolved_path[resolved_path_len] = DEFAULT_SLASH;
				resolved_path[++resolved_path_len] = '\0';
			}
			uri = (char *) xmlCanonicPath((const xmlChar *) resolved_path);
		}
		CRX_LIBXML_SANITIZE_GLOBALS(text_reader);
		reader = xmlNewTextReader(inputbfr, uri);

		if (reader != NULL) {
			ret = xmlTextReaderSetup(reader, NULL, uri, encoding, options);
			if (ret == 0) {
				if (id == NULL) {
					object_init_ex(return_value, xmlreader_class_entry);
					intern = C_XMLREADER_P(return_value);
				} else {
					RETVAL_TRUE;
				}
				intern->input = inputbfr;
				intern->ptr = reader;

				if (uri) {
					xmlFree(uri);
				}

				CRX_LIBXML_RESTORE_GLOBALS(text_reader);
				return;
			}
		}
		CRX_LIBXML_RESTORE_GLOBALS(text_reader);
	}

	if (uri) {
		xmlFree(uri);
	}

	if (inputbfr) {
		xmlFreeParserInputBuffer(inputbfr);
	}
	crx_error_docref(NULL, E_WARNING, "Unable to load source data");
	RETURN_FALSE;
}
/* }}} */

/* {{{ Moves the position of the current instance to the next node in the stream. */
CRX_METHOD(XMLReader, expand)
{
#ifdef HAVE_DOM
	zval *id, *basenode = NULL;
	int ret;
	xmlreader_object *intern;
	xmlNode *node, *nodec;
	xmlDocPtr docp = NULL;
	crx_libxml_node_object *domobj = NULL;

	id = CREX_THIS;
	if (crex_parse_parameters(CREX_NUM_ARGS(), "|O!", &basenode, dom_node_class_entry) == FAILURE) {
		RETURN_THROWS();
	}

	if (basenode != NULL) {
		NODE_GET_OBJ(node, basenode, xmlNodePtr, domobj);
		docp = node->doc;
	}

	intern = C_XMLREADER_P(id);

	if (intern->ptr) {
		node = xmlTextReaderExpand(intern->ptr);

		if (node == NULL) {
			crx_error_docref(NULL, E_WARNING, "An Error Occurred while expanding");
			RETURN_FALSE;
		} else {
			nodec = xmlDocCopyNode(node, docp, 1);
			if (nodec == NULL) {
				crx_error_docref(NULL, E_NOTICE, "Cannot expand this node type");
				RETURN_FALSE;
			} else {
				DOM_RET_OBJ(nodec, &ret, (dom_object *)domobj);
			}
		}
	} else {
		crex_throw_error(NULL, "Data must be loaded before expanding");
		RETURN_THROWS();
	}
#else
	zval *dummy;
	if (crex_parse_parameters(CREX_NUM_ARGS(), "|z!", &dummy) == FAILURE) {
		RETURN_THROWS();
	}

	crex_throw_error(NULL, "XMLReader::expand() requires the DOM extension to be enabled");
	RETURN_THROWS();
#endif
}
/* }}} */

static crex_result (*prev_crex_post_startup_cb)(void);
static crex_result xmlreader_fixup_temporaries(void) {
	if (CREX_OBSERVER_ENABLED) {
		++xmlreader_open_fn.T;
		++xmlreader_xml_fn.T;
	}
	if (prev_crex_post_startup_cb) {
		return prev_crex_post_startup_cb();
	}
	return SUCCESS;
}

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(xmlreader)
{

	memcpy(&xmlreader_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	xmlreader_object_handlers.offset = XtOffsetOf(xmlreader_object, std);
	xmlreader_object_handlers.free_obj = xmlreader_objects_free_storage;
	xmlreader_object_handlers.read_property = xmlreader_read_property;
	xmlreader_object_handlers.write_property = xmlreader_write_property;
	xmlreader_object_handlers.get_property_ptr_ptr = xmlreader_get_property_ptr_ptr;
	xmlreader_object_handlers.get_method = xmlreader_get_method;
	xmlreader_object_handlers.clone_obj = NULL;

	xmlreader_class_entry = register_class_XMLReader();
	xmlreader_class_entry->create_object = xmlreader_objects_new;
	xmlreader_class_entry->default_object_handlers = &xmlreader_object_handlers;

	memcpy(&xmlreader_open_fn, crex_hash_str_find_ptr(&xmlreader_class_entry->function_table, "open", sizeof("open")-1), sizeof(crex_internal_function));
	xmlreader_open_fn.fn_flags &= ~CREX_ACC_STATIC;
	memcpy(&xmlreader_xml_fn, crex_hash_str_find_ptr(&xmlreader_class_entry->function_table, "xml", sizeof("xml")-1), sizeof(crex_internal_function));
	xmlreader_xml_fn.fn_flags &= ~CREX_ACC_STATIC;

	prev_crex_post_startup_cb = crex_post_startup_cb;
	crex_post_startup_cb = xmlreader_fixup_temporaries;

	crex_hash_init(&xmlreader_prop_handlers, 0, NULL, crx_xmlreader_free_prop_handler, 1);
	xmlreader_register_prop_handler(&xmlreader_prop_handlers, "attributeCount", xmlTextReaderAttributeCount, NULL, IS_LONG);
	xmlreader_register_prop_handler(&xmlreader_prop_handlers, "baseURI", NULL, xmlTextReaderConstBaseUri, IS_STRING);
	xmlreader_register_prop_handler(&xmlreader_prop_handlers, "depth", xmlTextReaderDepth, NULL, IS_LONG);
	xmlreader_register_prop_handler(&xmlreader_prop_handlers, "hasAttributes", xmlTextReaderHasAttributes, NULL, _IS_BOOL);
	xmlreader_register_prop_handler(&xmlreader_prop_handlers, "hasValue", xmlTextReaderHasValue, NULL, _IS_BOOL);
	xmlreader_register_prop_handler(&xmlreader_prop_handlers, "isDefault", xmlTextReaderIsDefault, NULL, _IS_BOOL);
	xmlreader_register_prop_handler(&xmlreader_prop_handlers, "isEmptyElement", xmlTextReaderIsEmptyElement, NULL, _IS_BOOL);
	xmlreader_register_prop_handler(&xmlreader_prop_handlers, "localName", NULL, xmlTextReaderConstLocalName, IS_STRING);
	xmlreader_register_prop_handler(&xmlreader_prop_handlers, "name", NULL, xmlTextReaderConstName, IS_STRING);
	xmlreader_register_prop_handler(&xmlreader_prop_handlers, "namespaceURI", NULL, xmlTextReaderConstNamespaceUri, IS_STRING);
	xmlreader_register_prop_handler(&xmlreader_prop_handlers, "nodeType", xmlTextReaderNodeType, NULL, IS_LONG);
	xmlreader_register_prop_handler(&xmlreader_prop_handlers, "prefix", NULL, xmlTextReaderConstPrefix, IS_STRING);
	xmlreader_register_prop_handler(&xmlreader_prop_handlers, "value", NULL, xmlTextReaderConstValue, IS_STRING);
	xmlreader_register_prop_handler(&xmlreader_prop_handlers, "xmlLang", NULL, xmlTextReaderConstXmlLang, IS_STRING);

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION */
CRX_MSHUTDOWN_FUNCTION(xmlreader)
{
	crex_hash_destroy(&xmlreader_prop_handlers);
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(xmlreader)
{
	crx_info_print_table_start();
	{
		crx_info_print_table_row(2, "XMLReader", "enabled");
	}
	crx_info_print_table_end();
}
/* }}} */
