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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#if defined(HAVE_LIBXML) && defined(HAVE_DOM)
#include "crx_dom.h"
#include "crex_interfaces.h"

/*
* class DOMNamedNodeMap
*
* URL: https://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#core-ID-1780488922
* Since:
*/

int crx_dom_get_namednodemap_length(dom_object *obj)
{
	dom_nnodemap_object *objmap = (dom_nnodemap_object *) obj->ptr;
	if (!objmap) {
		return 0;
	}

	if (objmap->nodetype == XML_NOTATION_NODE || objmap->nodetype == XML_ENTITY_NODE) {
		return objmap->ht ? xmlHashSize(objmap->ht) : 0;
	}

	int count = 0;
	xmlNodePtr nodep = dom_object_get_node(objmap->baseobj);
	if (nodep) {
		xmlAttrPtr curnode = nodep->properties;
		if (curnode) {
			count++;
			while (curnode->next != NULL) {
				count++;
				curnode = curnode->next;
			}
		}
	}

	return count;
}

/* {{{ length	int
readonly=yes
URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#core-ID-6D0FB19E
Since:
*/
int dom_namednodemap_length_read(dom_object *obj, zval *retval)
{
	ZVAL_LONG(retval, crx_dom_get_namednodemap_length(obj));
	return SUCCESS;
}

/* }}} */

xmlNodePtr crx_dom_named_node_map_get_named_item(dom_nnodemap_object *objmap, const char *named, bool may_transform)
{
	xmlNodePtr itemnode = NULL;
	if (objmap != NULL) {
		if ((objmap->nodetype == XML_NOTATION_NODE) ||
			objmap->nodetype == XML_ENTITY_NODE) {
			if (objmap->ht) {
				if (objmap->nodetype == XML_ENTITY_NODE) {
					itemnode = (xmlNodePtr)xmlHashLookup(objmap->ht, (const xmlChar *) named);
				} else {
					xmlNotationPtr notep = xmlHashLookup(objmap->ht, (const xmlChar *) named);
					if (notep) {
						if (may_transform) {
							itemnode = create_notation(notep->name, notep->PublicID, notep->SystemID);
						} else {
							itemnode = (xmlNodePtr) notep;
						}
					}
				}
			}
		} else {
			xmlNodePtr nodep = dom_object_get_node(objmap->baseobj);
			if (nodep) {
				itemnode = (xmlNodePtr)xmlHasProp(nodep, (const xmlChar *) named);
			}
		}
	}
	return itemnode;
}

void crx_dom_named_node_map_get_named_item_into_zval(dom_nnodemap_object *objmap, const char *named, zval *return_value)
{
	int ret;
	xmlNodePtr itemnode = crx_dom_named_node_map_get_named_item(objmap, named, true);
	if (itemnode) {
		DOM_RET_OBJ(itemnode, &ret, objmap->baseobj);
	} else {
		RETURN_NULL();
	}
}

/* {{{ URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#core-ID-1074577549
Since:
*/
CRX_METHOD(DOMNamedNodeMap, getNamedItem)
{
	size_t namedlen;
	char *named;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s", &named, &namedlen) == FAILURE) {
		RETURN_THROWS();
	}

	zval *id = CREX_THIS;
	dom_nnodemap_object *objmap = C_DOMOBJ_P(id)->ptr;
	crx_dom_named_node_map_get_named_item_into_zval(objmap, named, return_value);
}
/* }}} end dom_namednodemap_get_named_item */

xmlNodePtr crx_dom_named_node_map_get_item(dom_nnodemap_object *objmap, crex_long index)
{
	xmlNodePtr itemnode = NULL;
	if (objmap != NULL) {
		if ((objmap->nodetype == XML_NOTATION_NODE) ||
			objmap->nodetype == XML_ENTITY_NODE) {
			if (objmap->ht) {
				if (objmap->nodetype == XML_ENTITY_NODE) {
					itemnode = crx_dom_libxml_hash_iter(objmap->ht, index);
				} else {
					itemnode = crx_dom_libxml_notation_iter(objmap->ht, index);
				}
			}
		} else {
			xmlNodePtr nodep = dom_object_get_node(objmap->baseobj);
			if (nodep) {
				xmlNodePtr curnode = (xmlNodePtr)nodep->properties;
				crex_long count = 0;
				while (count < index && curnode != NULL) {
					count++;
					curnode = (xmlNodePtr)curnode->next;
				}
				itemnode = curnode;
			}
		}
	}
	return itemnode;
}

void crx_dom_named_node_map_get_item_into_zval(dom_nnodemap_object *objmap, crex_long index, zval *return_value)
{
	int ret;
	xmlNodePtr itemnode = crx_dom_named_node_map_get_item(objmap, index);
	if (itemnode) {
		DOM_RET_OBJ(itemnode, &ret, objmap->baseobj);
	} else {
		RETURN_NULL();
	}
}

/* {{{ URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#core-ID-349467F9
Since:
*/
CRX_METHOD(DOMNamedNodeMap, item)
{
	crex_long index;
	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(index)
	CREX_PARSE_PARAMETERS_END();
	if (index < 0 || CREX_LONG_INT_OVFL(index)) {
		crex_argument_value_error(1, "must be between 0 and %d", INT_MAX);
		RETURN_THROWS();
	}

	zval *id = CREX_THIS;
	dom_object *intern = C_DOMOBJ_P(id);
	dom_nnodemap_object *objmap = intern->ptr;
	crx_dom_named_node_map_get_item_into_zval(objmap, index, return_value);
}
/* }}} end dom_namednodemap_item */

/* {{{ URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#core-ID-getNamedItemNS
Since: DOM Level 2
*/
CRX_METHOD(DOMNamedNodeMap, getNamedItemNS)
{
	zval *id;
	int ret;
	size_t namedlen=0, urilen=0;
	dom_object *intern;
	xmlNodePtr itemnode = NULL;
	char *uri, *named;

	dom_nnodemap_object *objmap;
	xmlNodePtr nodep;
	xmlNotation *notep = NULL;

	id = CREX_THIS;
	if (crex_parse_parameters(CREX_NUM_ARGS(), "s!s", &uri, &urilen, &named, &namedlen) == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_DOMOBJ_P(id);

	objmap = (dom_nnodemap_object *)intern->ptr;

	if (objmap != NULL) {
		if ((objmap->nodetype == XML_NOTATION_NODE) ||
			objmap->nodetype == XML_ENTITY_NODE) {
			if (objmap->ht) {
				if (objmap->nodetype == XML_ENTITY_NODE) {
					itemnode = (xmlNodePtr)xmlHashLookup(objmap->ht, (xmlChar *) named);
				} else {
					notep = (xmlNotation *)xmlHashLookup(objmap->ht, (xmlChar *) named);
					if (notep) {
						itemnode = create_notation(notep->name, notep->PublicID, notep->SystemID);
					}
				}
			}
		} else {
			nodep = dom_object_get_node(objmap->baseobj);
			if (nodep) {
				itemnode = (xmlNodePtr)xmlHasNsProp(nodep, (xmlChar *) named, (xmlChar *) uri);
			}
		}
	}

	if (itemnode) {
		DOM_RET_OBJ(itemnode, &ret, objmap->baseobj);
		return;
	} else {
		RETVAL_NULL();
	}
}
/* }}} end dom_namednodemap_get_named_item_ns */

/* {{{ */
CRX_METHOD(DOMNamedNodeMap, count)
{
	zval *id;
	dom_object *intern;

	id = CREX_THIS;
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_DOMOBJ_P(id);
	RETURN_LONG(crx_dom_get_namednodemap_length(intern));
}
/* }}} end dom_namednodemap_count */

CRX_METHOD(DOMNamedNodeMap, getIterator)
{
	if (crex_parse_parameters_none() == FAILURE) {
		return;
	}

	crex_create_internal_iterator_zval(return_value, CREX_THIS);
}

#endif
