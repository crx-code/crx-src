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
  |         Pierre-A. Joye <pajoye@crx.net>                              |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_XMLWRITER_H
#define CRX_XMLWRITER_H

extern crex_module_entry xmlwriter_module_entry;
#define crxext_xmlwriter_ptr &xmlwriter_module_entry

#include "crx_version.h"
#define CRX_XMLWRITER_VERSION CRX_VERSION

#ifdef ZTS
#include "TSRM.h"
#endif

#include <libxml/tree.h>
#include <libxml/xmlwriter.h>
#include <libxml/uri.h>

/* Extends crex object */
typedef struct _ze_xmlwriter_object {
	xmlTextWriterPtr ptr;
	xmlBufferPtr output;
	crex_object std;
} ze_xmlwriter_object;

static inline ze_xmlwriter_object *crx_xmlwriter_fetch_object(crex_object *obj) {
	return (ze_xmlwriter_object *)((char*)(obj) - XtOffsetOf(ze_xmlwriter_object, std));
}

#define C_XMLWRITER_P(zv) crx_xmlwriter_fetch_object(C_OBJ_P((zv)))

#endif	/* CRX_XMLWRITER_H */
