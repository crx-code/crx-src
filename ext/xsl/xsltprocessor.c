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
#include "crx_xsl.h"
#include "ext/libxml/crx_libxml.h"

/* {{{ crx_xsl_xslt_string_to_xpathexpr()
   Translates a string to a XPath Expression */
static char *crx_xsl_xslt_string_to_xpathexpr(const char *str)
{
	const xmlChar *string = (const xmlChar *)str;

	xmlChar *value;
	int str_len;

	str_len = xmlStrlen(string) + 3;

	if (xmlStrchr(string, '"')) {
		if (xmlStrchr(string, '\'')) {
			crx_error_docref(NULL, E_WARNING, "Cannot create XPath expression (string contains both quote and double-quotes)");
			return NULL;
		}
		value = (xmlChar*) safe_emalloc (str_len, sizeof(xmlChar), 0);
		snprintf((char*)value, str_len, "'%s'", string);
	} else {
		value = (xmlChar*) safe_emalloc (str_len, sizeof(xmlChar), 0);
		snprintf((char *)value, str_len, "\"%s\"", string);
	}
	return (char *) value;
}
/* }}} */

/* {{{ crx_xsl_xslt_make_params()
   Translates a CRX array to a libxslt parameters array */
static char **crx_xsl_xslt_make_params(HashTable *parht, int xpath_params)
{

	int parsize;
	zval *value;
	char *xpath_expr;
	crex_string *string_key;
	char **params = NULL;
	int i = 0;

	parsize = (2 * crex_hash_num_elements(parht) + 1) * sizeof(char *);
	params = (char **)safe_emalloc((2 * crex_hash_num_elements(parht) + 1), sizeof(char *), 0);
	memset((char *)params, 0, parsize);

	CREX_HASH_MAP_FOREACH_STR_KEY_VAL(parht, string_key, value) {
		CREX_ASSERT(string_key != NULL);
		if (C_TYPE_P(value) != IS_STRING) {
			if (!try_convert_to_string(value)) {
				efree(params);
				return NULL;
			}
		}

		if (!xpath_params) {
			xpath_expr = crx_xsl_xslt_string_to_xpathexpr(C_STRVAL_P(value));
		} else {
			xpath_expr = estrndup(C_STRVAL_P(value), C_STRLEN_P(value));
		}
		if (xpath_expr) {
			params[i++] = estrndup(ZSTR_VAL(string_key), ZSTR_LEN(string_key));
			params[i++] = xpath_expr;
		}
	} CREX_HASH_FOREACH_END();

	params[i++] = NULL;

	return params;
}
/* }}} */

static void xsl_ext_function_crx(xmlXPathParserContextPtr ctxt, int nargs, int type) /* {{{ */
{
	xsltTransformContextPtr tctxt;
	zval *args = NULL;
	zval retval;
	int i;
	int error = 0;
	crex_fcall_info fci;
	zval handler;
	xmlXPathObjectPtr obj;
	char *str;
	xsl_object *intern;
	crex_string *callable = NULL;


	if (! crex_is_executing()) {
		xsltGenericError(xsltGenericErrorContext,
		"xsltExtFunctionTest: Function called from outside of CRX\n");
		error = 1;
	} else {
		tctxt = xsltXPathGetTransformContext(ctxt);
		if (tctxt == NULL) {
			xsltGenericError(xsltGenericErrorContext,
			"xsltExtFunctionTest: failed to get the transformation context\n");
			error = 1;
		} else {
			intern = (xsl_object*)tctxt->_private;
			if (intern == NULL) {
				xsltGenericError(xsltGenericErrorContext,
				"xsltExtFunctionTest: failed to get the internal object\n");
				error = 1;
			}
			else if (intern->registerCrxFunctions == 0) {
				xsltGenericError(xsltGenericErrorContext,
				"xsltExtFunctionTest: CRX Object did not register CRX functions\n");
				error = 1;
			}
		}
	}

	if (error == 1) {
		for (i = nargs - 1; i >= 0; i--) {
			obj = valuePop(ctxt);
			if (obj) {
				xmlXPathFreeObject(obj);
			}
		}
		return;
	}

	if (UNEXPECTED(nargs == 0)) {
		crex_throw_error(NULL, "Function name must be passed as the first argument");
		return;
	}

	fci.param_count = nargs - 1;
	if (fci.param_count > 0) {
		args = safe_emalloc(fci.param_count, sizeof(zval), 0);
	}
	/* Reverse order to pop values off ctxt stack */
	for (i = fci.param_count - 1; i >= 0; i--) {
		obj = valuePop(ctxt);
		if (obj == NULL) {
			ZVAL_NULL(&args[i]);
			continue;
		}
		switch (obj->type) {
			case XPATH_STRING:
				ZVAL_STRING(&args[i], (char *)obj->stringval);
				break;
			case XPATH_BOOLEAN:
				ZVAL_BOOL(&args[i],  obj->boolval);
				break;
			case XPATH_NUMBER:
				ZVAL_DOUBLE(&args[i], obj->floatval);
				break;
			case XPATH_NODESET:
				if (type == 1) {
					str = (char*)xmlXPathCastToString(obj);
					ZVAL_STRING(&args[i], str);
					xmlFree(str);
				} else if (type == 2) {
					int j;
					dom_object *domintern = (dom_object *)intern->doc;
					if (obj->nodesetval && obj->nodesetval->nodeNr > 0) {
						array_init(&args[i]);
						for (j = 0; j < obj->nodesetval->nodeNr; j++) {
							xmlNodePtr node = obj->nodesetval->nodeTab[j];
							zval child;
							/* not sure, if we need this... it's copied from xpath.c */
							if (node->type == XML_NAMESPACE_DECL) {
								xmlNsPtr curns;
								xmlNodePtr nsparent;

								nsparent = node->_private;
								curns = xmlNewNs(NULL, node->name, NULL);
								if (node->children) {
									curns->prefix = xmlStrdup((xmlChar *)node->children);
								}
								if (node->children) {
									node = xmlNewDocNode(node->doc, NULL, (xmlChar *) node->children, node->name);
								} else {
									node = xmlNewDocNode(node->doc, NULL, (const xmlChar *) "xmlns", node->name);
								}
								node->type = XML_NAMESPACE_DECL;
								node->parent = nsparent;
								node->ns = curns;
							} else {
								/**
								 * Upon freeing libxslt's context, every document which is not the *main* document will be freed by libxslt.
								 * If a node of a document which is *not the main* document gets returned to userland, we'd free the node twice:
								 * first by the cleanup of the xslt context, and then by our own refcounting mechanism.
								 * To prevent this, we'll take a copy if the node is not from the main document.
								 * It is important that we do not copy the node unconditionally, because that means that:
								 *  - modifications to the node will only modify the copy, and not the original
								 *  - accesses to the parent, path, ... will not work
								 */
								xsltTransformContextPtr transform_ctxt = (xsltTransformContextPtr) ctxt->context->extra;
								if (node->doc != transform_ctxt->document->doc) {
									node = xmlDocCopyNode(node, domintern->document->ptr, 1);
								}
							}

							crx_dom_create_object(node, &child, domintern);
							add_next_index_zval(&args[i], &child);
						}
					} else {
						ZVAL_EMPTY_ARRAY(&args[i]);
					}
				}
				break;
			default:
				str = (char *) xmlXPathCastToString(obj);
				ZVAL_STRING(&args[i], str);
				xmlFree(str);
		}
		xmlXPathFreeObject(obj);
	}

	fci.size = sizeof(fci);
	fci.named_params = NULL;
	if (fci.param_count > 0) {
		fci.params = args;
	} else {
		fci.params = NULL;
	}

	/* Last element of the stack is the function name */
	obj = valuePop(ctxt);
	if (obj == NULL || obj->stringval == NULL) {
		crx_error_docref(NULL, E_WARNING, "Handler name must be a string");
		xmlXPathFreeObject(obj);
		valuePush(ctxt, xmlXPathNewString((const xmlChar *) ""));
		if (fci.param_count > 0) {
			for (i = 0; i < nargs - 1; i++) {
				zval_ptr_dtor(&args[i]);
			}
			efree(args);
		}
		return;
	}
	ZVAL_STRING(&handler, (char *) obj->stringval);
	xmlXPathFreeObject(obj);

	ZVAL_COPY_VALUE(&fci.function_name, &handler);
	fci.object = NULL;
	fci.retval = &retval;
	if (!crex_make_callable(&handler, &callable)) {
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "Unable to call handler %s()", ZSTR_VAL(callable));
		}
		valuePush(ctxt, xmlXPathNewString((const xmlChar *) ""));
	} else if ( intern->registerCrxFunctions == 2 && crex_hash_exists(intern->registered_crxfunctions, callable) == 0) {
		crx_error_docref(NULL, E_WARNING, "Not allowed to call handler '%s()'", ZSTR_VAL(callable));
		/* Push an empty string, so that we at least have an xslt result... */
		valuePush(ctxt, xmlXPathNewString((const xmlChar *) ""));
	} else {
		crex_call_function(&fci, NULL);
		if (C_ISUNDEF(retval)) {
			/* Exception thrown, don't do anything further. */
		} else if (C_TYPE(retval) == IS_OBJECT && instanceof_function(C_OBJCE(retval), dom_node_class_entry)) {
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
		} else if (C_TYPE(retval) == IS_TRUE || C_TYPE(retval) == IS_FALSE) {
			valuePush(ctxt, xmlXPathNewBoolean(C_TYPE(retval) == IS_TRUE));
		} else if (C_TYPE(retval) == IS_OBJECT) {
			crx_error_docref(NULL, E_WARNING, "A CRX Object cannot be converted to a XPath-string");
			valuePush(ctxt, xmlXPathNewString((const xmlChar *) ""));
		} else {
			convert_to_string(&retval);
			valuePush(ctxt, xmlXPathNewString((xmlChar *) C_STRVAL(retval)));
		}
		zval_ptr_dtor(&retval);
	}
	crex_string_release_ex(callable, 0);
	zval_ptr_dtor_nogc(&handler);
	if (fci.param_count > 0) {
		for (i = 0; i < nargs - 1; i++) {
			zval_ptr_dtor(&args[i]);
		}
		efree(args);
	}
}
/* }}} */

void xsl_ext_function_string_crx(xmlXPathParserContextPtr ctxt, int nargs) /* {{{ */
{
	xsl_ext_function_crx(ctxt, nargs, 1);
}
/* }}} */

void xsl_ext_function_object_crx(xmlXPathParserContextPtr ctxt, int nargs) /* {{{ */
{
	xsl_ext_function_crx(ctxt, nargs, 2);
}
/* }}} */

/* {{{ URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#
Since:
*/
CRX_METHOD(XSLTProcessor, importStylesheet)
{
	zval *id, *docp = NULL;
	xmlDoc *doc = NULL, *newdoc = NULL;
	xsltStylesheetPtr sheetp, oldsheetp;
	xsl_object *intern;
	int clone_docu = 0;
	xmlNode *nodep = NULL;
	zval *cloneDocu, rv;
	crex_string *member;

	id = CREX_THIS;
	if (crex_parse_parameters(CREX_NUM_ARGS(), "o", &docp) == FAILURE) {
		RETURN_THROWS();
	}

	nodep = crx_libxml_import_node(docp);

	if (nodep) {
		doc = nodep->doc;
	}
	if (doc == NULL) {
		crex_argument_value_error(1, "must be a valid XML node");
		RETURN_THROWS();
	}

	/* libxslt uses _private, so we must copy the imported
	stylesheet document otherwise the node proxies will be a mess */
	newdoc = xmlCopyDoc(doc, 1);
	xmlNodeSetBase((xmlNodePtr) newdoc, (xmlChar *)doc->URL);
	CRX_LIBXML_SANITIZE_GLOBALS(parse);
	xmlSubstituteEntitiesDefault(1);
	xmlLoadExtDtdDefaultValue = XML_DETECT_IDS | XML_COMPLETE_ATTRS;

	sheetp = xsltParseStylesheetDoc(newdoc);
	CRX_LIBXML_RESTORE_GLOBALS(parse);

	if (!sheetp) {
		xmlFreeDoc(newdoc);
		RETURN_FALSE;
	}

	intern = C_XSL_P(id);

	member = ZSTR_INIT_LITERAL("cloneDocument", 0);
	cloneDocu = crex_std_read_property(C_OBJ_P(id), member, BP_VAR_IS, NULL, &rv);
	if (C_TYPE_P(cloneDocu) != IS_NULL) {
		convert_to_long(cloneDocu);
		clone_docu = C_LVAL_P(cloneDocu);
	}
	crex_string_release_ex(member, 0);
	if (clone_docu == 0) {
		/* check if the stylesheet is using xsl:key, if yes, we have to clone the document _always_ before a transformation */
		nodep = xmlDocGetRootElement(sheetp->doc);
		if (nodep && (nodep = nodep->children)) {
			while (nodep) {
				if (nodep->type == XML_ELEMENT_NODE && xmlStrEqual(nodep->name, (const xmlChar *) "key") && xmlStrEqual(nodep->ns->href, XSLT_NAMESPACE)) {
					intern->hasKeys = 1;
					break;
				}
				nodep = nodep->next;
			}
		}
	} else {
		intern->hasKeys = clone_docu;
	}

	if ((oldsheetp = (xsltStylesheetPtr)intern->ptr)) {
		/* free wrapper */
		if (((xsltStylesheetPtr) intern->ptr)->_private != NULL) {
			((xsltStylesheetPtr) intern->ptr)->_private = NULL;
		}
		xsltFreeStylesheet((xsltStylesheetPtr) intern->ptr);
		intern->ptr = NULL;
	}

	crx_xsl_set_object(id, sheetp);
	RETVAL_TRUE;
}
/* }}} end XSLTProcessor::importStylesheet */

static xmlDocPtr crx_xsl_apply_stylesheet(zval *id, xsl_object *intern, xsltStylesheetPtr style, zval *docp) /* {{{ */
{
	xmlDocPtr newdocp = NULL;
	xmlDocPtr doc = NULL;
	xmlNodePtr node = NULL;
	xsltTransformContextPtr ctxt;
	crx_libxml_node_object *object;
	char **params = NULL;
	int clone;
	zval *doXInclude, rv;
	crex_string *member;
	FILE *f;
	int secPrefsError = 0;
	int secPrefsValue;
	xsltSecurityPrefsPtr secPrefs = NULL;

	node = crx_libxml_import_node(docp);

	if (node) {
		doc = node->doc;
	}

	if (doc == NULL) {
		crex_argument_value_error(1, "must be a valid XML node");
		return NULL;
	}

	if (style == NULL) {
		crex_string *name = get_active_function_or_method_name();
		crex_throw_error(NULL, "%s() can only be called after a stylesheet has been imported",
			ZSTR_VAL(name));
		crex_string_release(name);
		return NULL;
	}

	if (intern->profiling) {
		if (crx_check_open_basedir(intern->profiling)) {
			f = NULL;
		} else {
			f = VCWD_FOPEN(intern->profiling, "w");
		}
	} else {
		f = NULL;
	}

	if (intern->parameter) {
		params = crx_xsl_xslt_make_params(intern->parameter, 0);
	}

	intern->doc = emalloc(sizeof(crx_libxml_node_object));
	memset(intern->doc, 0, sizeof(crx_libxml_node_object));

	if (intern->hasKeys == 1) {
		doc = xmlCopyDoc(doc, 1);
	} else {
		object = C_LIBXML_NODE_P(docp);
		intern->doc->document = object->document;
	}

	crx_libxml_increment_doc_ref(intern->doc, doc);

	ctxt = xsltNewTransformContext(style, doc);
	ctxt->_private = (void *) intern;

	member = ZSTR_INIT_LITERAL("doXInclude", 0);
	doXInclude = crex_std_read_property(C_OBJ_P(id), member, BP_VAR_IS, NULL, &rv);
	if (C_TYPE_P(doXInclude) != IS_NULL) {
		convert_to_long(doXInclude);
		ctxt->xinclude = C_LVAL_P(doXInclude);
	}
	crex_string_release_ex(member, 0);

	secPrefsValue = intern->securityPrefs;

	/* if securityPrefs is set to NONE, we don't have to do any checks, but otherwise... */
	if (secPrefsValue != XSL_SECPREF_NONE) {
		secPrefs = xsltNewSecurityPrefs();
		if (secPrefsValue & XSL_SECPREF_READ_FILE ) {
			if (0 != xsltSetSecurityPrefs(secPrefs, XSLT_SECPREF_READ_FILE, xsltSecurityForbid)) {
				secPrefsError = 1;
			}
		}
		if (secPrefsValue & XSL_SECPREF_WRITE_FILE ) {
			if (0 != xsltSetSecurityPrefs(secPrefs, XSLT_SECPREF_WRITE_FILE, xsltSecurityForbid)) {
				secPrefsError = 1;
			}
		}
		if (secPrefsValue & XSL_SECPREF_CREATE_DIRECTORY ) {
			if (0 != xsltSetSecurityPrefs(secPrefs, XSLT_SECPREF_CREATE_DIRECTORY, xsltSecurityForbid)) {
				secPrefsError = 1;
			}
		}
		if (secPrefsValue & XSL_SECPREF_READ_NETWORK) {
			if (0 != xsltSetSecurityPrefs(secPrefs, XSLT_SECPREF_READ_NETWORK, xsltSecurityForbid)) {
				secPrefsError = 1;
			}
		}
		if (secPrefsValue & XSL_SECPREF_WRITE_NETWORK) {
			if (0 != xsltSetSecurityPrefs(secPrefs, XSLT_SECPREF_WRITE_NETWORK, xsltSecurityForbid)) {
				secPrefsError = 1;
			}
		}

		if (0 != xsltSetCtxtSecurityPrefs(secPrefs, ctxt)) {
			secPrefsError = 1;
		}
	}

	if (secPrefsError == 1) {
		crx_error_docref(NULL, E_WARNING, "Can't set libxslt security properties, not doing transformation for security reasons");
	} else {
		newdocp = xsltApplyStylesheetUser(style, doc, (const char**) params,  NULL, f, ctxt);
	}
	if (f) {
		fclose(f);
	}

	xsltFreeTransformContext(ctxt);
	if (secPrefs) {
		xsltFreeSecurityPrefs(secPrefs);
	}

	if (intern->node_list != NULL) {
		crex_hash_destroy(intern->node_list);
		FREE_HASHTABLE(intern->node_list);
		intern->node_list = NULL;
	}

	crx_libxml_decrement_doc_ref(intern->doc);
	efree(intern->doc);
	intern->doc = NULL;

	if (params) {
		clone = 0;
		while(params[clone]) {
			efree(params[clone++]);
		}
		efree(params);
	}

	return newdocp;

}
/* }}} */

/* {{{ URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#
Since:
*/
CRX_METHOD(XSLTProcessor, transformToDoc)
{
	zval *id, *docp = NULL;
	xmlDoc *newdocp;
	xsltStylesheetPtr sheetp;
	crex_string *ret_class = NULL;
	xsl_object *intern;

	id = CREX_THIS;
	intern = C_XSL_P(id);
	sheetp = (xsltStylesheetPtr) intern->ptr;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "o|S!", &docp, &ret_class) == FAILURE) {
		RETURN_THROWS();
	}

	newdocp = crx_xsl_apply_stylesheet(id, intern, sheetp, docp);

	if (newdocp) {
		if (ret_class) {
			crex_string *curclass_name;
			crex_class_entry *curce, *ce;
			crx_libxml_node_object *interndoc;

			curce = C_OBJCE_P(docp);
			curclass_name = curce->name;
			while (curce->parent != NULL) {
				curce = curce->parent;
			}

			ce = crex_lookup_class(ret_class);
			if (ce == NULL || !instanceof_function(ce, curce)) {
				xmlFreeDoc(newdocp);
				crex_argument_type_error(2, "must be a class name compatible with %s, \"%s\" given",
					ZSTR_VAL(curclass_name), ZSTR_VAL(ret_class)
				);
				RETURN_THROWS();
			}

			object_init_ex(return_value, ce);

			interndoc = C_LIBXML_NODE_P(return_value);
			crx_libxml_increment_doc_ref(interndoc, newdocp);
			crx_libxml_increment_node_ptr(interndoc, (xmlNodePtr)newdocp, (void *)interndoc);
		} else {
			crx_dom_create_object((xmlNodePtr) newdocp, return_value, NULL);
		}
	} else {
		RETURN_FALSE;
	}

}
/* }}} end XSLTProcessor::transformToDoc */

/* {{{ */
CRX_METHOD(XSLTProcessor, transformToUri)
{
	zval *id, *docp = NULL;
	xmlDoc *newdocp;
	xsltStylesheetPtr sheetp;
	int ret;
	size_t uri_len;
	char *uri;
	xsl_object *intern;

	id = CREX_THIS;
	intern = C_XSL_P(id);
	sheetp = (xsltStylesheetPtr) intern->ptr;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "op", &docp, &uri, &uri_len) == FAILURE) {
		RETURN_THROWS();
	}

	newdocp = crx_xsl_apply_stylesheet(id, intern, sheetp, docp);

	ret = -1;
	if (newdocp) {
		ret = xsltSaveResultToFilename(uri, newdocp, sheetp, 0);
		xmlFreeDoc(newdocp);
	}

	RETVAL_LONG(ret);
}
/* }}} end XSLTProcessor::transformToUri */

/* {{{ */
CRX_METHOD(XSLTProcessor, transformToXml)
{
	zval *id, *docp = NULL;
	xmlDoc *newdocp;
	xsltStylesheetPtr sheetp;
	int ret;
	xmlChar *doc_txt_ptr;
	int doc_txt_len;
	xsl_object *intern;

	id = CREX_THIS;
	intern = C_XSL_P(id);
	sheetp = (xsltStylesheetPtr) intern->ptr;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "o", &docp) == FAILURE) {
		RETURN_THROWS();
	}

	newdocp = crx_xsl_apply_stylesheet(id, intern, sheetp, docp);

	ret = -1;
	if (newdocp) {
		ret = xsltSaveResultToString(&doc_txt_ptr, &doc_txt_len, newdocp, sheetp);
		if (doc_txt_ptr && doc_txt_len) {
			RETVAL_STRINGL((char *) doc_txt_ptr, doc_txt_len);
			xmlFree(doc_txt_ptr);
		}
		xmlFreeDoc(newdocp);
	}

	if (ret < 0) {
		RETURN_FALSE;
	}
}
/* }}} end XSLTProcessor::transformToXml */

/* {{{ */
CRX_METHOD(XSLTProcessor, setParameter)
{

	zval *id = CREX_THIS;
	zval *entry, new_string;
	HashTable *array_value;
	xsl_object *intern;
	char *namespace;
	size_t namespace_len;
	crex_string *string_key, *name, *value = NULL;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STRING(namespace, namespace_len)
		C_PARAM_ARRAY_HT_OR_STR(array_value, name)
		C_PARAM_OPTIONAL
		C_PARAM_STR_OR_NULL(value)
	CREX_PARSE_PARAMETERS_END();

	intern = C_XSL_P(id);

	if (array_value) {
		if (value) {
			crex_argument_value_error(3, "must be null when argument #2 ($name) is an array");
			RETURN_THROWS();
		}

		CREX_HASH_FOREACH_STR_KEY_VAL(array_value, string_key, entry) {
			zval tmp;
			crex_string *str;

			if (string_key == NULL) {
				crex_argument_type_error(2, "must contain only string keys");
				RETURN_THROWS();
			}
			str = zval_try_get_string(entry);
			if (UNEXPECTED(!str)) {
				RETURN_THROWS();
			}
			ZVAL_STR(&tmp, str);
			crex_hash_update(intern->parameter, string_key, &tmp);
		} CREX_HASH_FOREACH_END();
		RETURN_TRUE;
	} else {
		if (!value) {
			crex_argument_value_error(3, "cannot be null when argument #2 ($name) is a string");
			RETURN_THROWS();
		}

		ZVAL_STR_COPY(&new_string, value);

		crex_hash_update(intern->parameter, name, &new_string);
		RETURN_TRUE;
	}
}
/* }}} end XSLTProcessor::setParameter */

/* {{{ */
CRX_METHOD(XSLTProcessor, getParameter)
{
	zval *id = CREX_THIS;
	char *namespace;
	size_t namespace_len = 0;
	zval *value;
	crex_string *name;
	xsl_object *intern;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sS", &namespace, &namespace_len, &name) == FAILURE) {
		RETURN_THROWS();
	}
	intern = C_XSL_P(id);
	if ((value = crex_hash_find(intern->parameter, name)) != NULL) {
		RETURN_STR(zval_get_string(value));
	} else {
		RETURN_FALSE;
	}
}
/* }}} end XSLTProcessor::getParameter */

/* {{{ */
CRX_METHOD(XSLTProcessor, removeParameter)
{
	zval *id = CREX_THIS;
	size_t namespace_len = 0;
	char *namespace;
	crex_string *name;
	xsl_object *intern;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sS", &namespace, &namespace_len, &name) == FAILURE) {
		RETURN_THROWS();
	}
	intern = C_XSL_P(id);
	if (crex_hash_del(intern->parameter, name) == SUCCESS) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} end XSLTProcessor::removeParameter */

/* {{{ */
CRX_METHOD(XSLTProcessor, registerCRXFunctions)
{
	zval *id = CREX_THIS;
	xsl_object *intern;
	zval *entry, new_string;
	crex_string *restrict_str = NULL;
	HashTable *restrict_ht = NULL;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT_OR_STR_OR_NULL(restrict_ht, restrict_str)
	CREX_PARSE_PARAMETERS_END();

	intern = C_XSL_P(id);

	if (restrict_ht) {
		CREX_HASH_FOREACH_VAL(restrict_ht, entry) {
			crex_string *str = zval_try_get_string(entry);
			if (UNEXPECTED(!str)) {
				return;
			}
			ZVAL_LONG(&new_string, 1);
			crex_hash_update(intern->registered_crxfunctions, str, &new_string);
			crex_string_release(str);
		} CREX_HASH_FOREACH_END();

		intern->registerCrxFunctions = 2;
	} else if (restrict_str) {
		ZVAL_LONG(&new_string, 1);
		crex_hash_update(intern->registered_crxfunctions, restrict_str, &new_string);
		intern->registerCrxFunctions = 2;
	} else {
		intern->registerCrxFunctions = 1;
	}
}
/* }}} end XSLTProcessor::registerCRXFunctions(); */

/* {{{ */
CRX_METHOD(XSLTProcessor, setProfiling)
{
	zval *id = CREX_THIS;
	xsl_object *intern;
	char *filename = NULL;
	size_t filename_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "p!", &filename, &filename_len) == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_XSL_P(id);
	if (intern->profiling) {
		efree(intern->profiling);
	}
	if (filename != NULL) {
		intern->profiling = estrndup(filename, filename_len);
	} else {
		intern->profiling = NULL;
	}

	RETURN_TRUE;
}
/* }}} end XSLTProcessor::setProfiling */

/* {{{ */
CRX_METHOD(XSLTProcessor, setSecurityPrefs)
{
	zval *id = CREX_THIS;
	xsl_object *intern;
	crex_long securityPrefs, oldSecurityPrefs;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &securityPrefs) == FAILURE) {
		RETURN_THROWS();
	}
	intern = C_XSL_P(id);
	oldSecurityPrefs = intern->securityPrefs;
	intern->securityPrefs = securityPrefs;
	/* set this to 1 so that we know, it was set through this method. Can be removed, when we remove the ini setting */
	intern->securityPrefsSet = 1;
	RETURN_LONG(oldSecurityPrefs);
}
/* }}} end XSLTProcessor::setSecurityPrefs */

/* {{{ */
CRX_METHOD(XSLTProcessor, getSecurityPrefs)
{
	zval *id = CREX_THIS;
	xsl_object *intern;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_XSL_P(id);

	RETURN_LONG(intern->securityPrefs);
}
/* }}} end XSLTProcessor::getSecurityPrefs */

/* {{{ */
CRX_METHOD(XSLTProcessor, hasExsltSupport)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

#ifdef HAVE_XSL_EXSLT
	RETURN_TRUE;
#else
	RETURN_FALSE;
#endif
}
/* }}} end XSLTProcessor::hasExsltSupport(); */
