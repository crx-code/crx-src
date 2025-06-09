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
* class DOMEntityReference extends DOMNode
*
* URL: https://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#ID-11C98490
* Since:
*/

/* {{{ */
CRX_METHOD(DOMEntityReference, __main)
{
	xmlNode *node;
	xmlNodePtr oldnode = NULL;
	dom_object *intern;
	char *name;
	size_t name_len, name_valid;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s", &name, &name_len) == FAILURE) {
		RETURN_THROWS();
	}

	name_valid = xmlValidateName((xmlChar *) name, 0);
	if (name_valid != 0) {
		crx_dom_throw_error(INVALID_CHARACTER_ERR, 1);
		RETURN_THROWS();
	}

	node = xmlNewReference(NULL, (xmlChar *) name);

	if (!node) {
		crx_dom_throw_error(INVALID_STATE_ERR, 1);
		RETURN_THROWS();
	}

	intern = C_DOMOBJ_P(CREX_THIS);
	oldnode = dom_object_get_node(intern);
	if (oldnode != NULL) {
		crx_libxml_node_decrement_resource((crx_libxml_node_object *)intern);
	}
	crx_libxml_increment_node_ptr((crx_libxml_node_object *)intern, node, (void *)intern);
}
/* }}} end DOMEntityReference::__main */

#endif
