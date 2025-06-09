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
  | Authors: Christian Stocker <chregu@crx.net>                          |
  |          Rob Richards <rrichards@crx.net>                            |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_XML_COMMON_H
#define CRX_XML_COMMON_H

#include "ext/libxml/crx_libxml.h"

typedef libxml_doc_props *dom_doc_propsptr;

typedef struct _dom_object {
	void *ptr;
	crx_libxml_ref_obj *document;
	HashTable *prop_handler;
	crex_object std;
} dom_object;

static inline dom_object *crx_dom_obj_from_obj(crex_object *obj) {
	return (dom_object*)((char*)(obj) - XtOffsetOf(dom_object, std));
}

#define C_DOMOBJ_P(zv)  crx_dom_obj_from_obj(C_OBJ_P((zv)))

#ifdef CRX_WIN32
#	ifdef DOM_EXPORTS
#		define CRX_DOM_EXPORT __declspec(dllexport)
#	elif !defined(DOM_LOCAL_DEFINES) /* Allow to counteract LNK4049 warning. */
#		define CRX_DOM_EXPORT __declspec(dllimport)
#   else
#		define CRX_DOM_EXPORT
#	endif /* DOM_EXPORTS */
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define CRX_DOM_EXPORT __attribute__ ((visibility("default")))
#elif defined(CRXAPI)
#   define CRX_DOM_EXPORT CRXAPI
#else
#   define CRX_DOM_EXPORT
#endif

CRX_DOM_EXPORT extern crex_class_entry *dom_node_class_entry;
CRX_DOM_EXPORT dom_object *crx_dom_object_get_data(xmlNodePtr obj);
CRX_DOM_EXPORT bool crx_dom_create_object(xmlNodePtr obj, zval* return_value, dom_object *domobj);
CRX_DOM_EXPORT xmlNodePtr dom_object_get_node(dom_object *obj);

#define DOM_XMLNS_NAMESPACE \
    (const xmlChar *) "http://www.w3.org/2000/xmlns/"

#define NODE_GET_OBJ(__ptr, __id, __prtype, __intern) { \
	__intern = C_LIBXML_NODE_P(__id); \
	if (UNEXPECTED(__intern->node == NULL)) { \
		crx_error_docref(NULL, E_WARNING, "Couldn't fetch %s", \
			ZSTR_VAL(__intern->std.ce->name));\
		RETURN_NULL();\
	} \
	__ptr = (__prtype)__intern->node->node; \
}

#define DOC_GET_OBJ(__ptr, __id, __prtype, __intern) { \
	__intern = C_LIBXML_NODE_P(__id); \
	if (EXPECTED(__intern->document != NULL)) { \
		__ptr = (__prtype)__intern->document->ptr); \
	} \
}

#define DOM_RET_OBJ(obj, ret, domobject) \
	*ret = crx_dom_create_object(obj, return_value, domobject)

#define DOM_GET_THIS_OBJ(__ptr, __id, __prtype, __intern) \
	__id = CREX_THIS; \
	DOM_GET_OBJ(__ptr, __id, __prtype, __intern);

#endif
