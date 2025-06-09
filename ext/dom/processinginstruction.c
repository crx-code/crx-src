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
* class DOMProcessingInstruction extends DOMNode
*
* URL: https://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#ID-1004215813
* Since:
*/

/* {{{ */
CRX_METHOD(DOMProcessingInstruction, __main)
{
	xmlNodePtr nodep = NULL, oldnode = NULL;
	dom_object *intern;
	char *name, *value = NULL;
	size_t name_len, value_len;
	int name_valid;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s|s", &name, &name_len, &value, &value_len) == FAILURE) {
		RETURN_THROWS();
	}

	name_valid = xmlValidateName((xmlChar *) name, 0);
	if (name_valid != 0) {
		crx_dom_throw_error(INVALID_CHARACTER_ERR, 1);
		RETURN_THROWS();
	}

	nodep = xmlNewPI((xmlChar *) name, (xmlChar *) value);

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
/* }}} end DOMProcessingInstruction::__main */

/* {{{ target	string
readonly=yes
URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#ID-1478689192
Since:
*/
int dom_processinginstruction_target_read(dom_object *obj, zval *retval)
{
	xmlNodePtr nodep = dom_object_get_node(obj);

	if (nodep == NULL) {
		crx_dom_throw_error(INVALID_STATE_ERR, 1);
		return FAILURE;
	}

	ZVAL_STRING(retval, (char *) (nodep->name));

	return SUCCESS;
}

/* }}} */

/* {{{ data	string
readonly=no
URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#ID-837822393
Since:
*/
int dom_processinginstruction_data_read(dom_object *obj, zval *retval)
{
	xmlNodePtr nodep = dom_object_get_node(obj);

	if (nodep == NULL) {
		crx_dom_throw_error(INVALID_STATE_ERR, 1);
		return FAILURE;
	}

	crx_dom_get_content_into_zval(nodep, retval, false);

	return SUCCESS;
}

int dom_processinginstruction_data_write(dom_object *obj, zval *newval)
{
	xmlNode *nodep = dom_object_get_node(obj);

	if (nodep == NULL) {
		crx_dom_throw_error(INVALID_STATE_ERR, 1);
		return FAILURE;
	}

	/* Typed property, this is already a string */
	CREX_ASSERT(C_TYPE_P(newval) == IS_STRING);
	crex_string *str = C_STR_P(newval);

	xmlNodeSetContentLen(nodep, (xmlChar *) ZSTR_VAL(str), ZSTR_LEN(str));

	return SUCCESS;
}

/* }}} */

#endif
