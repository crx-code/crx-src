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

#ifndef CRX_XMLREADER_H
#define CRX_XMLREADER_H

extern crex_module_entry xmlreader_module_entry;
#define crxext_xmlreader_ptr &xmlreader_module_entry

#include "crx_version.h"
#define CRX_XMLREADER_VERSION CRX_VERSION

#ifdef ZTS
#include "TSRM.h"
#endif

#include "ext/libxml/crx_libxml.h"
#include <libxml/xmlreader.h>

/* If xmlreader and dom both are compiled statically,
   no DLL import should be used in xmlreader for dom symbols. */
#ifdef CRX_WIN32
# if defined(HAVE_DOM) && !defined(COMPILE_DL_DOM) && !defined(COMPILE_DL_XMLREADER)
#  define DOM_LOCAL_DEFINES 1
# endif
#endif

typedef struct _xmlreader_object {
	xmlTextReaderPtr ptr;
	/* strings must be set in input buffer as copy is required */
	xmlParserInputBufferPtr input;
	void *schema;
	HashTable *prop_handler;
	crex_object  std;
} xmlreader_object;

static inline xmlreader_object *crx_xmlreader_fetch_object(crex_object *obj) {
	return (xmlreader_object *)((char*)(obj) - XtOffsetOf(xmlreader_object, std));
}

#define C_XMLREADER_P(zv) crx_xmlreader_fetch_object(C_OBJ_P((zv)))

CRX_MINIT_FUNCTION(xmlreader);
CRX_MSHUTDOWN_FUNCTION(xmlreader);
CRX_MINFO_FUNCTION(xmlreader);

#endif	/* CRX_XMLREADER_H */
