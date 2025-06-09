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

#define CRX_DOM_XPATH_QUERY 0
#define CRX_DOM_XPATH_EVALUATE 1

/*
* class DOMXPath
*/

#ifdef LIBXML_XPATH_ENABLED

static void dom_xpath_ext_function_crx(xmlXPathParserContextPtr ctxt, int nargs, int type) /* {{{ */
{
	zval retval;
	int result, i;
	int error = 0;
	crex_fcall_info fci;
	xmlXPathObjectPtr obj;
	char *str;
	crex_string *callable = NULL;
	dom_xpath_object *intern;


	if (! crex_is_executing()) {
		xmlGenericError(xmlGenericErrorContext,
		"xmlExtFunctionTest: Function called from outside of CRX\n");
		error = 1;
	} else {
		intern = (dom_xpath_object *) ctxt->context->userData;
		if (intern == NULL) {
			xmlGenericError(xmlGenericErrorContext,
			"xmlExtFunctionTest: failed to get the internal object\n");
			error = 1;
		}
		else if (intern->registerCrxFunctions == 0) {
			xmlGenericError(xmlGenericErrorContext,
			"xmlExtFunctionTest: CRX Object did not register CRX functions\n");
			error = 1;
		}
	}

	if (error == 1) {
		for (i = nargs - 1; i >= 0; i--) {
			obj = valuePop(ctxt);
			xmlXPathFreeObject(obj);
		}
		return;
	}

	if (UNEXPECTED(nargs == 0)) {
		crex_throw_error(NULL, "Function name must be passed as the first argument");
		return;
	}

	fci.param_count = nargs - 1;
	if (fci.param_count > 0) {
		fci.params = safe_emalloc(fci.param_count, sizeof(zval), 0);
	}
	/* Reverse order to pop values off ctxt stack */
	for (i = fci.param_count - 1; i >= 0; i--) {
		obj = valuePop(ctxt);
		switch (obj->type) {
			case XPATH_STRING:
				ZVAL_STRING(&fci.params[i],  (char *)obj->stringval);
				break;
			case XPATH_BOOLEAN:
				ZVAL_BOOL(&fci.params[i],  obj->boolval);
				break;
			case XPATH_NUMBER:
				ZVAL_DOUBLE(&fci.params[i], obj->floatval);
				break;
			case XPATH_NODESET:
				if (type == 1) {
					str = (char *)xmlXPathCastToString(obj);
					ZVAL_STRING(&fci.params[i], str);
					xmlFree(str);
				} else if (type == 2) {
					int j;
					if (obj->nodesetval && obj->nodesetval->nodeNr > 0) {
						array_init(&fci.params[i]);
						for (j = 0; j < obj->nodesetval->nodeNr; j++) {
							xmlNodePtr node = obj->nodesetval->nodeTab[j];
							zval child;
							/* not sure, if we need this... it's copied from xpath.c */
							if (node->type == XML_NAMESPACE_DECL) {
								xmlNodePtr nsparent = node->_private;
								xmlNsPtr original = (xmlNsPtr) node;

								/* Make sure parent dom object exists, so we can take an extra reference. */
								zval parent_zval; /* don't destroy me, my lifetime is transfered to the fake namespace decl */
								crx_dom_create_object(nsparent, &parent_zval, &intern->dom);
								dom_object *parent_intern = C_DOMOBJ_P(&parent_zval);

								node = crx_dom_create_fake_namespace_decl(nsparent, original, &child, parent_intern);
							} else {
								crx_dom_create_object(node, &child, &intern->dom);
							}
							add_next_index_zval(&fci.params[i], &child);
						}
					} else {
						ZVAL_EMPTY_ARRAY(&fci.params[i]);
					}
				}
				break;
			default:
			ZVAL_STRING(&fci.params[i], (char *)xmlXPathCastToString(obj));
		}
		xmlXPathFreeObject(obj);
	}

	fci.size = sizeof(fci);

	/* Last element of the stack is the function name */
	obj = valuePop(ctxt);
	if (obj->stringval == NULL) {
		crex_type_error("Handler name must be a string");
		xmlXPathFreeObject(obj);
		goto cleanup_no_callable;
	}
	ZVAL_STRING(&fci.function_name, (char *) obj->stringval);
	xmlXPathFreeObject(obj);

	fci.object = NULL;
	fci.named_params = NULL;
	fci.retval = &retval;

	if (!crex_make_callable(&fci.function_name, &callable)) {
		crex_throw_error(NULL, "Unable to call handler %s()", ZSTR_VAL(callable));
		goto cleanup;
	} else if (intern->registerCrxFunctions == 2 && crex_hash_exists(intern->registered_crxfunctions, callable) == 0) {
		crex_throw_error(NULL, "Not allowed to call handler '%s()'.", ZSTR_VAL(callable));
		goto cleanup;
	} else {
		result = crex_call_function(&fci, NULL);
		if (result == SUCCESS && C_TYPE(retval) != IS_UNDEF) {
			if (C_TYPE(retval) == IS_OBJECT && instanceof_function(C_OBJCE(retval), dom_node_class_entry)) {
				xmlNode *nodep;
				dom_object *obj;
				if (intern->node_list == NULL) {
					intern->node_list = crex_new_array(0);
				}
				C_ADDREF(retval);
				crex_hash_next_index_insert(intern->node_list, &retval);
				obj = C_DOMOBJ_P(&retval);
				nodep = dom_object_get_node(obj);
				valuePush(ctxt, xmlXPathNewNodeSet(nodep));
			} else if (C_TYPE(retval) == IS_FALSE || C_TYPE(retval) == IS_TRUE) {
				valuePush(ctxt, xmlXPathNewBoolean(C_TYPE(retval) == IS_TRUE));
			} else if (C_TYPE(retval) == IS_OBJECT) {
				crex_type_error("A CRX Object cannot be converted to a XPath-string");
				return;
			} else {
				crex_string *str = zval_get_string(&retval);
				valuePush(ctxt, xmlXPathNewString((xmlChar *) ZSTR_VAL(str)));
				crex_string_release_ex(str, 0);
			}
			zval_ptr_dtor(&retval);
		}
	}
cleanup:
	crex_string_release_ex(callable, 0);
	zval_ptr_dtor_nogc(&fci.function_name);
cleanup_no_callable:
	if (fci.param_count > 0) {
		for (i = 0; i < nargs - 1; i++) {
			zval_ptr_dtor(&fci.params[i]);
		}
		efree(fci.params);
	}
}
/* }}} */

static void dom_xpath_ext_function_string_crx(xmlXPathParserContextPtr ctxt, int nargs) /* {{{ */
{
	dom_xpath_ext_function_crx(ctxt, nargs, 1);
}
/* }}} */

static void dom_xpath_ext_function_object_crx(xmlXPathParserContextPtr ctxt, int nargs) /* {{{ */
{
	dom_xpath_ext_function_crx(ctxt, nargs, 2);
}
/* }}} */

/* {{{ */
CRX_METHOD(DOMXPath, __main)
{
	zval *doc;
	bool register_node_ns = 1;
	xmlDocPtr docp = NULL;
	dom_object *docobj;
	dom_xpath_object *intern;
	xmlXPathContextPtr ctx, oldctx;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O|b", &doc, dom_document_class_entry, &register_node_ns) == FAILURE) {
		RETURN_THROWS();
	}

	DOM_GET_OBJ(docp, doc, xmlDocPtr, docobj);

	ctx = xmlXPathNewContext(docp);
	if (ctx == NULL) {
		crx_dom_throw_error(INVALID_STATE_ERR, 1);
		RETURN_THROWS();
	}

	intern = C_XPATHOBJ_P(CREX_THIS);
	if (intern != NULL) {
		oldctx = (xmlXPathContextPtr)intern->dom.ptr;
		if (oldctx != NULL) {
			crx_libxml_decrement_doc_ref((crx_libxml_node_object *) &intern->dom);
			xmlXPathFreeContext(oldctx);
		}

		xmlXPathRegisterFuncNS (ctx, (const xmlChar *) "functionString",
					   (const xmlChar *) "http://crx.net/xpath",
					   dom_xpath_ext_function_string_crx);
		xmlXPathRegisterFuncNS (ctx, (const xmlChar *) "function",
					   (const xmlChar *) "http://crx.net/xpath",
					   dom_xpath_ext_function_object_crx);

		intern->dom.ptr = ctx;
		ctx->userData = (void *)intern;
		intern->dom.document = docobj->document;
		intern->register_node_ns = register_node_ns;
		crx_libxml_increment_doc_ref((crx_libxml_node_object *) &intern->dom, docp);
	}
}
/* }}} end DOMXPath::__main */

/* {{{ document DOMDocument*/
int dom_xpath_document_read(dom_object *obj, zval *retval)
{
	xmlDoc *docp = NULL;
	xmlXPathContextPtr ctx = (xmlXPathContextPtr) obj->ptr;

	if (ctx) {
		docp = (xmlDocPtr) ctx->doc;
	}

	crx_dom_create_object((xmlNodePtr) docp, retval, obj);
	return SUCCESS;
}
/* }}} */

/* {{{ registerNodeNamespaces bool*/
static inline dom_xpath_object *crx_xpath_obj_from_dom_obj(dom_object *obj) {
	return (dom_xpath_object*)((char*)(obj) - XtOffsetOf(dom_xpath_object, dom));
}

int dom_xpath_register_node_ns_read(dom_object *obj, zval *retval)
{
	ZVAL_BOOL(retval, crx_xpath_obj_from_dom_obj(obj)->register_node_ns);

	return SUCCESS;
}

int dom_xpath_register_node_ns_write(dom_object *obj, zval *newval)
{
	crx_xpath_obj_from_dom_obj(obj)->register_node_ns = crex_is_true(newval);

	return SUCCESS;
}
/* }}} */

/* {{{ */
CRX_METHOD(DOMXPath, registerNamespace)
{
	zval *id;
	xmlXPathContextPtr ctxp;
	size_t prefix_len, ns_uri_len;
	dom_xpath_object *intern;
	unsigned char *prefix, *ns_uri;

	id = CREX_THIS;
	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss", &prefix, &prefix_len, &ns_uri, &ns_uri_len) == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_XPATHOBJ_P(id);

	ctxp = (xmlXPathContextPtr) intern->dom.ptr;
	if (ctxp == NULL) {
		crex_throw_error(NULL, "Invalid XPath Context");
		RETURN_THROWS();
	}

	if (xmlXPathRegisterNs(ctxp, prefix, ns_uri) != 0) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

static void dom_xpath_iter(zval *baseobj, dom_object *intern) /* {{{ */
{
	dom_nnodemap_object *mapptr = (dom_nnodemap_object *) intern->ptr;

	ZVAL_COPY_VALUE(&mapptr->baseobj_zv, baseobj);
	mapptr->nodetype = DOM_NODESET;
}
/* }}} */

static void crx_xpath_eval(INTERNAL_FUNCTION_PARAMETERS, int type) /* {{{ */
{
	zval *id, retval, *context = NULL;
	xmlXPathContextPtr ctxp;
	xmlNodePtr nodep = NULL;
	xmlXPathObjectPtr xpathobjp;
	size_t expr_len, nsnbr = 0, xpath_type;
	dom_xpath_object *intern;
	dom_object *nodeobj;
	char *expr;
	xmlDoc *docp = NULL;
	xmlNsPtr *ns = NULL;
	bool register_node_ns;

	id = CREX_THIS;
	intern = C_XPATHOBJ_P(id);
	register_node_ns = intern->register_node_ns;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s|O!b", &expr, &expr_len, &context, dom_node_class_entry, &register_node_ns) == FAILURE) {
		RETURN_THROWS();
	}

	ctxp = (xmlXPathContextPtr) intern->dom.ptr;
	if (ctxp == NULL) {
		crex_throw_error(NULL, "Invalid XPath Context");
		RETURN_THROWS();
	}

	docp = (xmlDocPtr) ctxp->doc;
	if (docp == NULL) {
		crx_error_docref(NULL, E_WARNING, "Invalid XPath Document Pointer");
		RETURN_FALSE;
	}

	if (context != NULL) {
		DOM_GET_OBJ(nodep, context, xmlNodePtr, nodeobj);
	}

	if (!nodep) {
		nodep = xmlDocGetRootElement(docp);
	}

	if (nodep && docp != nodep->doc) {
		crex_throw_error(NULL, "Node from wrong document");
		RETURN_THROWS();
	}

	ctxp->node = nodep;

	if (register_node_ns) {
		/* Register namespaces in the node */
		ns = xmlGetNsList(docp, nodep);

		if (ns != NULL) {
			while (ns[nsnbr] != NULL)
			nsnbr++;
		}
	}


	ctxp->namespaces = ns;
	ctxp->nsNr = nsnbr;

	xpathobjp = xmlXPathEvalExpression((xmlChar *) expr, ctxp);
	ctxp->node = NULL;

	if (ns != NULL) {
		xmlFree(ns);
		ctxp->namespaces = NULL;
		ctxp->nsNr = 0;
	}

	if (! xpathobjp) {
		/* TODO Add Warning? */
		RETURN_FALSE;
	}

	if (type == CRX_DOM_XPATH_QUERY) {
		xpath_type = XPATH_NODESET;
	} else {
		xpath_type = xpathobjp->type;
	}

	switch (xpath_type) {

		case  XPATH_NODESET:
		{
			int i;
			xmlNodeSetPtr nodesetp;

			if (xpathobjp->type == XPATH_NODESET && NULL != (nodesetp = xpathobjp->nodesetval) && nodesetp->nodeNr) {

				array_init(&retval);
				for (i = 0; i < nodesetp->nodeNr; i++) {
					xmlNodePtr node = nodesetp->nodeTab[i];
					zval child;

					if (node->type == XML_NAMESPACE_DECL) {
						xmlNodePtr nsparent = node->_private;
						xmlNsPtr original = (xmlNsPtr) node;

						/* Make sure parent dom object exists, so we can take an extra reference. */
						zval parent_zval; /* don't destroy me, my lifetime is transfered to the fake namespace decl */
						crx_dom_create_object(nsparent, &parent_zval, &intern->dom);
						dom_object *parent_intern = C_DOMOBJ_P(&parent_zval);

						node = crx_dom_create_fake_namespace_decl(nsparent, original, &child, parent_intern);
					} else {
						crx_dom_create_object(node, &child, &intern->dom);
					}
					add_next_index_zval(&retval, &child);
				}
			} else {
				ZVAL_EMPTY_ARRAY(&retval);
			}
			crx_dom_create_iterator(return_value, DOM_NODELIST);
			nodeobj = C_DOMOBJ_P(return_value);
			dom_xpath_iter(&retval, nodeobj);
			break;
		}

		case XPATH_BOOLEAN:
			RETVAL_BOOL(xpathobjp->boolval);
			break;

		case XPATH_NUMBER:
			RETVAL_DOUBLE(xpathobjp->floatval);
			break;

		case XPATH_STRING:
			RETVAL_STRING((char *) xpathobjp->stringval);
			break;

		default:
			RETVAL_NULL();
			break;
	}

	xmlXPathFreeObject(xpathobjp);
}
/* }}} */

/* {{{ */
CRX_METHOD(DOMXPath, query)
{
	crx_xpath_eval(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_DOM_XPATH_QUERY);
}
/* }}} end dom_xpath_query */

/* {{{ */
CRX_METHOD(DOMXPath, evaluate)
{
	crx_xpath_eval(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_DOM_XPATH_EVALUATE);
}
/* }}} end dom_xpath_evaluate */

/* {{{ */
CRX_METHOD(DOMXPath, registerCrxFunctions)
{
	zval *id = CREX_THIS;
	dom_xpath_object *intern = C_XPATHOBJ_P(id);
	zval *entry, new_string;
	crex_string *name = NULL;
	HashTable *ht = NULL;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT_OR_STR_OR_NULL(ht, name)
	CREX_PARSE_PARAMETERS_END();

	if (ht) {
		CREX_HASH_FOREACH_VAL(ht, entry) {
			crex_string *str = zval_get_string(entry);
			ZVAL_LONG(&new_string, 1);
			crex_hash_update(intern->registered_crxfunctions, str, &new_string);
			crex_string_release_ex(str, 0);
		} CREX_HASH_FOREACH_END();
		intern->registerCrxFunctions = 2;
	} else if (name) {
		ZVAL_LONG(&new_string, 1);
		crex_hash_update(intern->registered_crxfunctions, name, &new_string);
		intern->registerCrxFunctions = 2;
	} else {
		intern->registerCrxFunctions = 1;
	}

}
/* }}} end dom_xpath_register_crx_functions */

#endif /* LIBXML_XPATH_ENABLED */

#endif
