/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 06c88dc2fb5582a6d21c11aee6ac0a0538e70cbc */

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_simplexml_load_file, 0, 1, SimpleXMLElement, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, class_name, IS_STRING, 1, "SimpleXMLElement::class")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, namespace_or_prefix, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, is_prefix, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_simplexml_load_string, 0, 1, SimpleXMLElement, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, class_name, IS_STRING, 1, "SimpleXMLElement::class")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, namespace_or_prefix, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, is_prefix, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_simplexml_import_dom, 0, 1, SimpleXMLElement, 1)
	CREX_ARG_OBJ_TYPE_MASK(0, node, SimpleXMLElement|DOMNode, 0, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, class_name, IS_STRING, 1, "SimpleXMLElement::class")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_SimpleXMLElement_xpath, 0, 1, MAY_BE_ARRAY|MAY_BE_NULL|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, expression, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SimpleXMLElement_registerXPathNamespace, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, prefix, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_SimpleXMLElement_asXML, 0, 0, MAY_BE_STRING|MAY_BE_BOOL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, filename, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_class_SimpleXMLElement_saveXML arginfo_class_SimpleXMLElement_asXML

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SimpleXMLElement_getNamespaces, 0, 0, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, recursive, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_SimpleXMLElement_getDocNamespaces, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, recursive, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, fromRoot, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_SimpleXMLElement_children, 0, 0, SimpleXMLElement, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, namespaceOrPrefix, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, isPrefix, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

#define arginfo_class_SimpleXMLElement_attributes arginfo_class_SimpleXMLElement_children

CREX_BEGIN_ARG_INFO_EX(arginfo_class_SimpleXMLElement___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, dataIsURL, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, namespaceOrPrefix, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, isPrefix, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_SimpleXMLElement_addChild, 0, 1, SimpleXMLElement, 1)
	CREX_ARG_TYPE_INFO(0, qualifiedName, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, value, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, namespace, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SimpleXMLElement_addAttribute, 0, 2, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, qualifiedName, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, namespace, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SimpleXMLElement_getName, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SimpleXMLElement___toString, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SimpleXMLElement_count, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SimpleXMLElement_rewind, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SimpleXMLElement_valid, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_SimpleXMLElement_current, 0, 0, SimpleXMLElement, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SimpleXMLElement_key arginfo_class_SimpleXMLElement_getName

#define arginfo_class_SimpleXMLElement_next arginfo_class_SimpleXMLElement_rewind

#define arginfo_class_SimpleXMLElement_hasChildren arginfo_class_SimpleXMLElement_valid

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_SimpleXMLElement_getChildren, 0, 0, SimpleXMLElement, 1)
CREX_END_ARG_INFO()


CREX_FUNCTION(simplexml_load_file);
CREX_FUNCTION(simplexml_load_string);
CREX_FUNCTION(simplexml_import_dom);
CREX_METHOD(SimpleXMLElement, xpath);
CREX_METHOD(SimpleXMLElement, registerXPathNamespace);
CREX_METHOD(SimpleXMLElement, asXML);
CREX_METHOD(SimpleXMLElement, getNamespaces);
CREX_METHOD(SimpleXMLElement, getDocNamespaces);
CREX_METHOD(SimpleXMLElement, children);
CREX_METHOD(SimpleXMLElement, attributes);
CREX_METHOD(SimpleXMLElement, __main);
CREX_METHOD(SimpleXMLElement, addChild);
CREX_METHOD(SimpleXMLElement, addAttribute);
CREX_METHOD(SimpleXMLElement, getName);
CREX_METHOD(SimpleXMLElement, __toString);
CREX_METHOD(SimpleXMLElement, count);
CREX_METHOD(SimpleXMLElement, rewind);
CREX_METHOD(SimpleXMLElement, valid);
CREX_METHOD(SimpleXMLElement, current);
CREX_METHOD(SimpleXMLElement, key);
CREX_METHOD(SimpleXMLElement, next);
CREX_METHOD(SimpleXMLElement, hasChildren);
CREX_METHOD(SimpleXMLElement, getChildren);


static const crex_function_entry ext_functions[] = {
	CREX_FE(simplexml_load_file, arginfo_simplexml_load_file)
	CREX_FE(simplexml_load_string, arginfo_simplexml_load_string)
	CREX_FE(simplexml_import_dom, arginfo_simplexml_import_dom)
	CREX_FE_END
};


static const crex_function_entry class_SimpleXMLElement_methods[] = {
	CREX_ME(SimpleXMLElement, xpath, arginfo_class_SimpleXMLElement_xpath, CREX_ACC_PUBLIC)
	CREX_ME(SimpleXMLElement, registerXPathNamespace, arginfo_class_SimpleXMLElement_registerXPathNamespace, CREX_ACC_PUBLIC)
	CREX_ME(SimpleXMLElement, asXML, arginfo_class_SimpleXMLElement_asXML, CREX_ACC_PUBLIC)
	CREX_MALIAS(SimpleXMLElement, saveXML, asXML, arginfo_class_SimpleXMLElement_saveXML, CREX_ACC_PUBLIC)
	CREX_ME(SimpleXMLElement, getNamespaces, arginfo_class_SimpleXMLElement_getNamespaces, CREX_ACC_PUBLIC)
	CREX_ME(SimpleXMLElement, getDocNamespaces, arginfo_class_SimpleXMLElement_getDocNamespaces, CREX_ACC_PUBLIC)
	CREX_ME(SimpleXMLElement, children, arginfo_class_SimpleXMLElement_children, CREX_ACC_PUBLIC)
	CREX_ME(SimpleXMLElement, attributes, arginfo_class_SimpleXMLElement_attributes, CREX_ACC_PUBLIC)
	CREX_ME(SimpleXMLElement, __main, arginfo_class_SimpleXMLElement___main, CREX_ACC_PUBLIC)
	CREX_ME(SimpleXMLElement, addChild, arginfo_class_SimpleXMLElement_addChild, CREX_ACC_PUBLIC)
	CREX_ME(SimpleXMLElement, addAttribute, arginfo_class_SimpleXMLElement_addAttribute, CREX_ACC_PUBLIC)
	CREX_ME(SimpleXMLElement, getName, arginfo_class_SimpleXMLElement_getName, CREX_ACC_PUBLIC)
	CREX_ME(SimpleXMLElement, __toString, arginfo_class_SimpleXMLElement___toString, CREX_ACC_PUBLIC)
	CREX_ME(SimpleXMLElement, count, arginfo_class_SimpleXMLElement_count, CREX_ACC_PUBLIC)
	CREX_ME(SimpleXMLElement, rewind, arginfo_class_SimpleXMLElement_rewind, CREX_ACC_PUBLIC)
	CREX_ME(SimpleXMLElement, valid, arginfo_class_SimpleXMLElement_valid, CREX_ACC_PUBLIC)
	CREX_ME(SimpleXMLElement, current, arginfo_class_SimpleXMLElement_current, CREX_ACC_PUBLIC)
	CREX_ME(SimpleXMLElement, key, arginfo_class_SimpleXMLElement_key, CREX_ACC_PUBLIC)
	CREX_ME(SimpleXMLElement, next, arginfo_class_SimpleXMLElement_next, CREX_ACC_PUBLIC)
	CREX_ME(SimpleXMLElement, hasChildren, arginfo_class_SimpleXMLElement_hasChildren, CREX_ACC_PUBLIC)
	CREX_ME(SimpleXMLElement, getChildren, arginfo_class_SimpleXMLElement_getChildren, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_SimpleXMLIterator_methods[] = {
	CREX_FE_END
};

static crex_class_entry *register_class_SimpleXMLElement(crex_class_entry *class_entry_Stringable, crex_class_entry *class_entry_Countable, crex_class_entry *class_entry_RecursiveIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SimpleXMLElement", class_SimpleXMLElement_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;
	crex_class_implements(class_entry, 3, class_entry_Stringable, class_entry_Countable, class_entry_RecursiveIterator);

	return class_entry;
}

static crex_class_entry *register_class_SimpleXMLIterator(crex_class_entry *class_entry_SimpleXMLElement)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SimpleXMLIterator", class_SimpleXMLIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_SimpleXMLElement);

	return class_entry;
}
