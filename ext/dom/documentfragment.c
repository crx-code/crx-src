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

/*
* class DOMDocumentFragment extends DOMNode
*
* URL: https://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#ID-B63ED1A3
* Since:
*/

/* {{{ */
CRX_METHOD(DOMDocumentFragment, __main)
{
	xmlNodePtr nodep = NULL, oldnode = NULL;
	dom_object *intern;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	nodep = xmlNewDocFragment(NULL);

	if (!nodep) {
		crx_dom_throw_error(INVALID_STATE_ERR, 1);
		RETURN_THROWS();
	}

	intern = C_DOMOBJ_P(CREX_THIS);
	oldnode = dom_object_get_node(intern);
	if (oldnode != NULL) {
		crx_libxml_node_decrement_resource((crx_libxml_node_object *)intern);
	}
	crx_libxml_increment_node_ptr((crx_libxml_node_object *)intern, nodep, (void *)intern);
}
/* }}} end DOMDocumentFragment::__main */

/* {{{ */
CRX_METHOD(DOMDocumentFragment, appendXML) {
	zval *id;
	xmlNode *nodep;
	dom_object *intern;
	char *data = NULL;
	size_t data_len = 0;
	int err;
	xmlNodePtr lst;

	id = CREX_THIS;
	if (crex_parse_parameters(CREX_NUM_ARGS(), "s", &data, &data_len) == FAILURE) {
		RETURN_THROWS();
	}

	DOM_GET_OBJ(nodep, id, xmlNodePtr, intern);

	if (dom_node_is_read_only(nodep) == SUCCESS) {
		crx_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, dom_get_strict_error(intern->document));
		RETURN_FALSE;
	}

	if (data) {
		CRX_LIBXML_SANITIZE_GLOBALS(parse);
		err = xmlParseBalancedChunkMemory(nodep->doc, NULL, NULL, 0, (xmlChar *) data, &lst);
		CRX_LIBXML_RESTORE_GLOBALS(parse);
		if (err != 0) {
			RETURN_FALSE;
		}

		xmlAddChildList(nodep,lst);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ URL: https://dom.spec.whatwg.org/#dom-parentnode-append
Since: DOM Living Standard (DOM4)
*/
CRX_METHOD(DOMDocumentFragment, append)
{
	uint32_t argc = 0;
	zval *args;
	dom_object *intern;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "*", &args, &argc) == FAILURE) {
		RETURN_THROWS();
	}

	DOM_GET_THIS_INTERN(intern);

	dom_parent_node_append(intern, args, argc);
}
/* }}} */

/* {{{ URL: https://dom.spec.whatwg.org/#dom-parentnode-prepend
Since: DOM Living Standard (DOM4)
*/
CRX_METHOD(DOMDocumentFragment, prepend)
{
	uint32_t argc = 0;
	zval *args;
	dom_object *intern;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "*", &args, &argc) == FAILURE) {
		RETURN_THROWS();
	}

	DOM_GET_THIS_INTERN(intern);

	dom_parent_node_prepend(intern, args, argc);
}
/* }}} */

/* {{{ URL: https://dom.spec.whatwg.org/#dom-parentnode-replacechildren
Since:
*/
CRX_METHOD(DOMDocumentFragment, replaceChildren)
{
	uint32_t argc = 0;
	zval *args;
	dom_object *intern;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "*", &args, &argc) == FAILURE) {
		RETURN_THROWS();
	}

	DOM_GET_THIS_INTERN(intern);

	dom_parent_node_replace_children(intern, args, argc);
}
/* }}} */

#endif
