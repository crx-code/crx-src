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
  | Author: Christian Stocker <chregu@crx.net>                           |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "crx_ini.h"
#include "ext/standard/info.h"
#include "crx_xsl.h"
#include "crx_xsl_arginfo.h"

crex_class_entry *xsl_xsltprocessor_class_entry;
static crex_object_handlers xsl_object_handlers;

static const crex_module_dep xsl_deps[] = {
	CREX_MOD_REQUIRED("libxml")
	CREX_MOD_REQUIRED("dom")
	CREX_MOD_END
};

/* {{{ xsl_module_entry */
crex_module_entry xsl_module_entry = {
	STANDARD_MODULE_HEADER_EX, NULL,
	xsl_deps,
	"xsl",
	NULL,
	CRX_MINIT(xsl),
	CRX_MSHUTDOWN(xsl),
	NULL,
	NULL,
	CRX_MINFO(xsl),
	CRX_XSL_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_XSL
CREX_GET_MODULE(xsl)
#endif

/* {{{ xsl_objects_free_storage */
void xsl_objects_free_storage(crex_object *object)
{
	xsl_object *intern = crx_xsl_fetch_object(object);

	crex_object_std_dtor(&intern->std);

	if (intern->parameter) {
		crex_hash_destroy(intern->parameter);
		FREE_HASHTABLE(intern->parameter);
	}

	if (intern->registered_crxfunctions) {
		crex_hash_destroy(intern->registered_crxfunctions);
		FREE_HASHTABLE(intern->registered_crxfunctions);
	}

	if (intern->node_list) {
		crex_hash_destroy(intern->node_list);
		FREE_HASHTABLE(intern->node_list);
	}

	if (intern->doc) {
		crx_libxml_decrement_doc_ref(intern->doc);
		efree(intern->doc);
	}

	if (intern->ptr) {
		/* free wrapper */
		if (((xsltStylesheetPtr) intern->ptr)->_private != NULL) {
			((xsltStylesheetPtr) intern->ptr)->_private = NULL;
		}

		xsltFreeStylesheet((xsltStylesheetPtr) intern->ptr);
		intern->ptr = NULL;
	}
	if (intern->profiling) {
		efree(intern->profiling);
	}
}
/* }}} */

/* {{{ xsl_objects_new */
crex_object *xsl_objects_new(crex_class_entry *class_type)
{
	xsl_object *intern;

	intern = crex_object_alloc(sizeof(xsl_object), class_type);
	intern->securityPrefs = XSL_SECPREF_DEFAULT;

	crex_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);
	intern->parameter = crex_new_array(0);
	intern->registered_crxfunctions = crex_new_array(0);

	return &intern->std;
}
/* }}} */

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(xsl)
{
	memcpy(&xsl_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	xsl_object_handlers.offset = XtOffsetOf(xsl_object, std);
	xsl_object_handlers.clone_obj = NULL;
	xsl_object_handlers.free_obj = xsl_objects_free_storage;

	xsl_xsltprocessor_class_entry = register_class_XSLTProcessor();
	xsl_xsltprocessor_class_entry->create_object = xsl_objects_new;
	xsl_xsltprocessor_class_entry->default_object_handlers = &xsl_object_handlers;

#ifdef HAVE_XSL_EXSLT
	exsltRegisterAll();
#endif

	xsltRegisterExtModuleFunction ((const xmlChar *) "functionString",
				   (const xmlChar *) "http://crx.net/xsl",
				   xsl_ext_function_string_crx);
	xsltRegisterExtModuleFunction ((const xmlChar *) "function",
				   (const xmlChar *) "http://crx.net/xsl",
				   xsl_ext_function_object_crx);
	xsltSetGenericErrorFunc(NULL, crx_libxml_error_handler);

	register_crx_xsl_symbols(module_number);

	return SUCCESS;
}
/* }}} */

/* {{{ xsl_object_get_data */
zval *xsl_object_get_data(void *obj)
{
	zval *dom_wrapper;
	dom_wrapper = ((xsltStylesheetPtr) obj)->_private;
	return dom_wrapper;
}
/* }}} */

/* {{{ xsl_object_set_data */
static void xsl_object_set_data(void *obj, zval *wrapper)
{
	((xsltStylesheetPtr) obj)->_private = wrapper;
}
/* }}} */

/* {{{ crx_xsl_set_object */
void crx_xsl_set_object(zval *wrapper, void *obj)
{
	xsl_object *object;

	object = C_XSL_P(wrapper);
	object->ptr = obj;
	xsl_object_set_data(obj, wrapper);
}
/* }}} */

/* {{{ crx_xsl_create_object */
void crx_xsl_create_object(xsltStylesheetPtr obj, zval *wrapper_in, zval *return_value )
{
	zval *wrapper;
	crex_class_entry *ce;

	if (!obj) {
		wrapper = wrapper_in;
		ZVAL_NULL(wrapper);
		return;
	}

	if ((wrapper = xsl_object_get_data((void *) obj))) {
		ZVAL_COPY(wrapper, wrapper_in);
		return;
	}

	if (!wrapper_in) {
		wrapper = return_value;
	} else {
		wrapper = wrapper_in;
	}


	ce = xsl_xsltprocessor_class_entry;

	if (!wrapper_in) {
		object_init_ex(wrapper, ce);
	}
	crx_xsl_set_object(wrapper, (void *) obj);

	return;
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION */
CRX_MSHUTDOWN_FUNCTION(xsl)
{
	xsltUnregisterExtModuleFunction ((const xmlChar *) "functionString",
				   (const xmlChar *) "http://crx.net/xsl");
	xsltUnregisterExtModuleFunction ((const xmlChar *) "function",
				   (const xmlChar *) "http://crx.net/xsl");
	xsltSetGenericErrorFunc(NULL, NULL);
	xsltCleanupGlobals();

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(xsl)
{
	crx_info_print_table_start();
	{
		char buffer[128];
		int major, minor, subminor;

		crx_info_print_table_row(2, "XSL", "enabled");
		major = xsltLibxsltVersion/10000;
		minor = (xsltLibxsltVersion - major * 10000) / 100;
		subminor = (xsltLibxsltVersion - major * 10000 - minor * 100);
		snprintf(buffer, 128, "%d.%d.%d", major, minor, subminor);
		crx_info_print_table_row(2, "libxslt Version", buffer);
		major = xsltLibxmlVersion/10000;
		minor = (xsltLibxmlVersion - major * 10000) / 100;
		subminor = (xsltLibxmlVersion - major * 10000 - minor * 100);
		snprintf(buffer, 128, "%d.%d.%d", major, minor, subminor);
		crx_info_print_table_row(2, "libxslt compiled against libxml Version", buffer);
	}
#ifdef HAVE_XSL_EXSLT
	crx_info_print_table_row(2, "EXSLT", "enabled");
	crx_info_print_table_row(2, "libexslt Version", LIBEXSLT_DOTTED_VERSION);
#endif
	crx_info_print_table_end();
}
/* }}} */
