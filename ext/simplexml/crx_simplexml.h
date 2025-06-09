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

#ifndef CRX_SIMPLEXML_H
#define CRX_SIMPLEXML_H

extern crex_module_entry simplexml_module_entry;
#define crxext_simplexml_ptr &simplexml_module_entry

#include "crx_version.h"
#define CRX_SIMPLEXML_VERSION CRX_VERSION

#ifdef ZTS
#include "TSRM.h"
#endif

#include "ext/libxml/crx_libxml.h"
#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/tree.h>
#include <libxml/uri.h>
#include <libxml/xmlerror.h>
#include <libxml/xinclude.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/xpointer.h>
#include <libxml/xmlschemas.h>

CRX_MINIT_FUNCTION(simplexml);
CRX_MSHUTDOWN_FUNCTION(simplexml);
CRX_MINFO_FUNCTION(simplexml);

typedef enum {
	SXE_ITER_NONE     = 0,
	SXE_ITER_ELEMENT  = 1,
	SXE_ITER_CHILD    = 2,
	SXE_ITER_ATTRLIST = 3
} SXE_ITER;

typedef struct {
	crx_libxml_node_ptr *node;
	crx_libxml_ref_obj *document;
	HashTable *properties;
	xmlXPathContextPtr xpath;
	struct {
		xmlChar               *name;
		xmlChar               *nsprefix;
		int                   isprefix;
		SXE_ITER              type;
		zval                  data;
	} iter;
	zval tmp;
	crex_function *fptr_count;
	crex_object zo;
} crx_sxe_object;

#ifdef CRX_WIN32
#	ifdef CRX_SIMPLEXML_EXPORTS
#		define CRX_SXE_API __declspec(dllexport)
#	else
#		define CRX_SXE_API __declspec(dllimport)
#	endif
#else
#	define CRX_SXE_API CREX_API
#endif

extern CRX_SXE_API crex_class_entry *ce_SimpleXMLIterator;
extern CRX_SXE_API crex_class_entry *ce_SimpleXMLElement;

CRX_SXE_API crex_class_entry *sxe_get_element_class_entry(void);

#endif
