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
* class DOMNodeList
*
* URL: https://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#ID-536297177
* Since:
*/

static crex_always_inline void objmap_cache_release_cached_obj(dom_nnodemap_object *objmap)
{
	if (objmap->cached_obj) {
		/* Since the DOM is a tree there can be no cycles. */
		if (GC_DELREF(&objmap->cached_obj->std) == 0) {
			crex_objects_store_del(&objmap->cached_obj->std);
		}
		objmap->cached_obj = NULL;
		objmap->cached_obj_index = 0;
	}
}

static crex_always_inline void reset_objmap_cache(dom_nnodemap_object *objmap)
{
	objmap_cache_release_cached_obj(objmap);
	objmap->cached_length = -1;
}

int crx_dom_get_nodelist_length(dom_object *obj)
{
	dom_nnodemap_object *objmap = (dom_nnodemap_object *) obj->ptr;
	if (!objmap) {
		return 0;
	}

	if (objmap->ht) {
		return xmlHashSize(objmap->ht);
	}

	if (objmap->nodetype == DOM_NODESET) {
		HashTable *nodeht = HASH_OF(&objmap->baseobj_zv);
		return crex_hash_num_elements(nodeht);
	}

	xmlNodePtr nodep = dom_object_get_node(objmap->baseobj);
	if (!nodep) {
		return 0;
	}

	if (!crx_dom_is_cache_tag_stale_from_node(&objmap->cache_tag, nodep)) {
		if (objmap->cached_length >= 0) {
			return objmap->cached_length;
		}
		/* Only the length is out-of-date, the cache tag is still valid.
		 * Therefore, only overwrite the length and keep the currently cached object. */
	} else {
		crx_dom_mark_cache_tag_up_to_date_from_node(&objmap->cache_tag, nodep);
		reset_objmap_cache(objmap);
	}

	int count = 0;
	if (objmap->nodetype == XML_ATTRIBUTE_NODE || objmap->nodetype == XML_ELEMENT_NODE) {
		xmlNodePtr curnode = nodep->children;
		if (curnode) {
			count++;
			while (curnode->next != NULL) {
				count++;
				curnode = curnode->next;
			}
		}
	} else {
		xmlNodePtr basep = nodep;
		if (nodep->type == XML_DOCUMENT_NODE || nodep->type == XML_HTML_DOCUMENT_NODE) {
			nodep = xmlDocGetRootElement((xmlDoc *) nodep);
		} else {
			nodep = nodep->children;
		}
		dom_get_elements_by_tag_name_ns_raw(
			basep, nodep, (char *) objmap->ns, (char *) objmap->local, &count, INT_MAX - 1 /* because of <= */);
	}

	objmap->cached_length = count;

	return count;
}

/* {{{ length	int
readonly=yes
URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#ID-203510337
Since:
*/
int dom_nodelist_length_read(dom_object *obj, zval *retval)
{
	ZVAL_LONG(retval, crx_dom_get_nodelist_length(obj));
	return SUCCESS;
}


/* {{{ */
CRX_METHOD(DOMNodeList, count)
{
	zval *id;
	dom_object *intern;

	id = CREX_THIS;
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_DOMOBJ_P(id);
	RETURN_LONG(crx_dom_get_nodelist_length(intern));
}
/* }}} end dom_nodelist_count */

void crx_dom_nodelist_get_item_into_zval(dom_nnodemap_object *objmap, crex_long index, zval *return_value)
{
	int ret;
	xmlNodePtr itemnode = NULL;
	bool cache_itemnode = false;
	if (index >= 0) {
		if (objmap != NULL) {
			if (objmap->ht) {
				if (objmap->nodetype == XML_ENTITY_NODE) {
					itemnode = crx_dom_libxml_hash_iter(objmap->ht, index);
				} else {
					itemnode = crx_dom_libxml_notation_iter(objmap->ht, index);
				}
			} else {
				if (objmap->nodetype == DOM_NODESET) {
					HashTable *nodeht = HASH_OF(&objmap->baseobj_zv);
					zval *entry = crex_hash_index_find(nodeht, index);
					if (entry) {
						ZVAL_COPY(return_value, entry);
						return;
					}
				} else if (objmap->baseobj) {
					xmlNodePtr basep = dom_object_get_node(objmap->baseobj);
					if (basep) {
						xmlNodePtr nodep = basep;
						/* For now we're only able to use cache for forward search.
						 * TODO: in the future we could extend the logic of the node list such that backwards searches
						 *       are also possible. */
						bool restart = true;
						int relative_index = index;
						if (index >= objmap->cached_obj_index && objmap->cached_obj && !crx_dom_is_cache_tag_stale_from_node(&objmap->cache_tag, nodep)) {
							xmlNodePtr cached_obj_xml_node = dom_object_get_node(objmap->cached_obj);

							/* The node cannot be NULL if the cache is valid. If it is NULL, then it means we
							 * forgot an invalidation somewhere. Take the defensive programming approach and invalidate
							 * it here if it's NULL (except in debug mode where we would want to catch this). */
							if (UNEXPECTED(cached_obj_xml_node == NULL)) {
#if CREX_DEBUG
								CREX_UNREACHABLE();
#endif
								reset_objmap_cache(objmap);
							} else {
								restart = false;
								relative_index -= objmap->cached_obj_index;
								nodep = cached_obj_xml_node;
							}
						}
						int count = 0;
						if (objmap->nodetype == XML_ATTRIBUTE_NODE || objmap->nodetype == XML_ELEMENT_NODE) {
							if (restart) {
								nodep = nodep->children;
							}
							while (count < relative_index && nodep != NULL) {
								count++;
								nodep = nodep->next;
							}
							itemnode = nodep;
						} else {
							if (restart) {
								if (basep->type == XML_DOCUMENT_NODE || basep->type == XML_HTML_DOCUMENT_NODE) {
									nodep = xmlDocGetRootElement((xmlDoc*) basep);
								} else {
									nodep = basep->children;
								}
							}
							itemnode = dom_get_elements_by_tag_name_ns_raw(basep, nodep, (char *) objmap->ns, (char *) objmap->local, &count, relative_index);
						}
						cache_itemnode = true;
					}
				}
			}
		}

		if (itemnode) {
			DOM_RET_OBJ(itemnode, &ret, objmap->baseobj);
			if (cache_itemnode) {
				/* Hold additional reference for the cache, must happen before releasing the cache
				 * because we might be the last reference holder.
				 * Instead of storing and copying zvals, we store the object pointer directly.
				 * This saves us some bytes because a pointer is smaller than a zval.
				 * This also means we have to manually refcount the objects here, and remove the reference count
				 * in reset_objmap_cache() and the destructor. */
				dom_object *cached_obj = C_DOMOBJ_P(return_value);
				GC_ADDREF(&cached_obj->std);
				/* If the tag is stale, all cached data is useless. Otherwise only the cached object is useless. */
				if (crx_dom_is_cache_tag_stale_from_node(&objmap->cache_tag, itemnode)) {
					crx_dom_mark_cache_tag_up_to_date_from_node(&objmap->cache_tag, itemnode);
					reset_objmap_cache(objmap);
				} else {
					objmap_cache_release_cached_obj(objmap);
				}
				objmap->cached_obj_index = index;
				objmap->cached_obj = cached_obj;
			}
			return;
		}
	}

	RETVAL_NULL();
}

/* {{{ URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#ID-844377136
Since:
*/
CRX_METHOD(DOMNodeList, item)
{
	crex_long index;
	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(index)
	CREX_PARSE_PARAMETERS_END();

	zval *id = CREX_THIS;
	dom_object *intern = C_DOMOBJ_P(id);
	dom_nnodemap_object *objmap = intern->ptr;
	crx_dom_nodelist_get_item_into_zval(objmap, index, return_value);
}
/* }}} end dom_nodelist_item */

CREX_METHOD(DOMNodeList, getIterator)
{
	if (crex_parse_parameters_none() == FAILURE) {
		return;
	}

	crex_create_internal_iterator_zval(return_value, CREX_THIS);
}

#endif
