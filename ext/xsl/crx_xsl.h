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
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_XSL_H
#define CRX_XSL_H

extern crex_module_entry xsl_module_entry;
#define crxext_xsl_ptr &xsl_module_entry

#include "crx_version.h"
#define CRX_XSL_VERSION CRX_VERSION

#ifdef ZTS
#include "TSRM.h"
#endif

#include <libxslt/xsltconfig.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/xsltutils.h>
#include <libxslt/transform.h>
#include <libxslt/security.h>
#ifdef HAVE_XSL_EXSLT
#include <libexslt/exslt.h>
#include <libexslt/exsltconfig.h>
#endif

#include "../dom/xml_common.h"

#include <libxslt/extensions.h>
#include <libxml/xpathInternals.h>

#define XSL_SECPREF_NONE 0
#define XSL_SECPREF_READ_FILE 2
#define XSL_SECPREF_WRITE_FILE 4
#define XSL_SECPREF_CREATE_DIRECTORY 8
#define XSL_SECPREF_READ_NETWORK 16
#define XSL_SECPREF_WRITE_NETWORK 32
/* Default == disable all write access ==  XSL_SECPREF_WRITE_NETWORK | XSL_SECPREF_CREATE_DIRECTORY | XSL_SECPREF_WRITE_FILE */
#define XSL_SECPREF_DEFAULT 44

typedef struct _xsl_object {
	void *ptr;
	HashTable *prop_handler;
	zval handle;
	HashTable *parameter;
	int hasKeys;
	int registerCrxFunctions;
	HashTable *registered_crxfunctions;
	HashTable *node_list;
	crx_libxml_node_object *doc;
	char *profiling;
	crex_long securityPrefs;
	int securityPrefsSet;
	crex_object  std;
} xsl_object;

static inline xsl_object *crx_xsl_fetch_object(crex_object *obj) {
	return (xsl_object *)((char*)(obj) - XtOffsetOf(xsl_object, std));
}

#define C_XSL_P(zv) crx_xsl_fetch_object(C_OBJ_P((zv)))

void crx_xsl_set_object(zval *wrapper, void *obj);
void xsl_objects_free_storage(crex_object *object);
void crx_xsl_create_object(xsltStylesheetPtr obj, zval *wrapper_in, zval *return_value );

void xsl_ext_function_string_crx(xmlXPathParserContextPtr ctxt, int nargs);
void xsl_ext_function_object_crx(xmlXPathParserContextPtr ctxt, int nargs);

CRX_MINIT_FUNCTION(xsl);
CRX_MSHUTDOWN_FUNCTION(xsl);
CRX_RINIT_FUNCTION(xsl);
CRX_RSHUTDOWN_FUNCTION(xsl);
CRX_MINFO_FUNCTION(xsl);

#endif	/* CRX_XSL_H */
