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
  |         Marcus Boerger <helly@crx.net>                               |
  |         Rob Richards <rrichards@crx.net>                             |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_SIMPLEXML_EXPORTS_H
#define CRX_SIMPLEXML_EXPORTS_H

#include "crx_simplexml.h"

#define SKIP_TEXT(__p) \
	if ((__p)->type == XML_TEXT_NODE) { \
		goto next_iter; \
	}

#define GET_NODE(__s, __n) { \
	if ((__s)->node && (__s)->node->node) { \
		__n = (__s)->node->node; \
	} else { \
		__n = NULL; \
		crex_throw_error(NULL, "SimpleXMLElement is not properly initialized"); \
	} \
}

CRX_SXE_API crex_object *sxe_object_new(crex_class_entry *ce);

static inline crx_sxe_object *crx_sxe_fetch_object(crex_object *obj) /* {{{ */ {
	return (crx_sxe_object *)((char*)(obj) - XtOffsetOf(crx_sxe_object, zo));
}
/* }}} */

#define C_SXEOBJ_P(zv) crx_sxe_fetch_object(C_OBJ_P((zv)))

typedef struct {
	crex_object_iterator  intern;
	crx_sxe_object        *sxe;
} crx_sxe_iterator;

CRX_SXE_API void crx_sxe_rewind_iterator(crx_sxe_object *sxe);
CRX_SXE_API void crx_sxe_move_forward_iterator(crx_sxe_object *sxe);

#endif /* CRX_SIMPLEXML_EXPORTS_H */
