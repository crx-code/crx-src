/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: a20d21c1796ebb43028856f0ec2d53dcaded6cc0 */

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_dom_import_simplexml, 0, 1, DOMElement, 0)
	CREX_ARG_TYPE_INFO(0, node, IS_OBJECT, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMCdataSection___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMComment___main, 0, 0, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_DOMParentNode_append, 0, 0, IS_VOID, 0)
	CREX_ARG_VARIADIC_INFO(0, nodes)
CREX_END_ARG_INFO()

#define arginfo_class_DOMParentNode_prepend arginfo_class_DOMParentNode_append

#define arginfo_class_DOMParentNode_replaceChildren arginfo_class_DOMParentNode_append

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_DOMChildNode_remove, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DOMChildNode_before arginfo_class_DOMParentNode_append

#define arginfo_class_DOMChildNode_after arginfo_class_DOMParentNode_append

#define arginfo_class_DOMChildNode_replaceWith arginfo_class_DOMParentNode_append

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_DOMNode___sleep, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DOMNode___wakeup arginfo_class_DOMChildNode_remove

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMNode_appendChild, 0, 0, 1)
	CREX_ARG_OBJ_INFO(0, node, DOMNode, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_DOMNode_C14N, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, exclusive, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, withComments, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, xpath, IS_ARRAY, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, nsPrefixes, IS_ARRAY, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_DOMNode_C14NFile, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, uri, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, exclusive, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, withComments, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, xpath, IS_ARRAY, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, nsPrefixes, IS_ARRAY, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMNode_cloneNode, 0, 0, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, deep, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMNode_getLineNo, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMNode_getNodePath, 0, 0, IS_STRING, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMNode_hasAttributes, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DOMNode_hasChildNodes arginfo_class_DOMNode_hasAttributes

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMNode_insertBefore, 0, 0, 1)
	CREX_ARG_OBJ_INFO(0, node, DOMNode, 0)
	CREX_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, child, DOMNode, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMNode_isDefaultNamespace, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMNode_isSameNode, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, otherNode, DOMNode, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_DOMNode_isEqualNode, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, otherNode, DOMNode, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMNode_isSupported, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, feature, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, version, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMNode_lookupNamespaceURI, 0, 1, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, prefix, IS_STRING, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMNode_lookupPrefix, 0, 1, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMNode_normalize, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMNode_removeChild, 0, 0, 1)
	CREX_ARG_OBJ_INFO(0, child, DOMNode, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMNode_replaceChild, 0, 0, 2)
	CREX_ARG_OBJ_INFO(0, node, DOMNode, 0)
	CREX_ARG_OBJ_INFO(0, child, DOMNode, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_DOMNode_contains, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, other, DOMNode|DOMNameSpaceNode, MAY_BE_NULL, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_DOMNode_getRootNode, 0, 0, DOMNode, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_class_DOMNameSpaceNode___sleep arginfo_class_DOMNode___sleep

#define arginfo_class_DOMNameSpaceNode___wakeup arginfo_class_DOMChildNode_remove

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMImplementation_getFeature, 0, 2, IS_NEVER, 0)
	CREX_ARG_TYPE_INFO(0, feature, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, version, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DOMImplementation_hasFeature arginfo_class_DOMNode_isSupported

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMImplementation_createDocumentType, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, qualifiedName, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, publicId, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, systemId, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMImplementation_createDocument, 0, 0, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, namespace, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, qualifiedName, IS_STRING, 0, "\"\"")
	CREX_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, doctype, DOMDocumentType, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMDocumentFragment___main, 0, 0, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMDocumentFragment_appendXML, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DOMDocumentFragment_append arginfo_class_DOMParentNode_append

#define arginfo_class_DOMDocumentFragment_prepend arginfo_class_DOMParentNode_append

#define arginfo_class_DOMDocumentFragment_replaceChildren arginfo_class_DOMParentNode_append

#define arginfo_class_DOMNodeList_count arginfo_class_DOMNode_getLineNo

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_DOMNodeList_getIterator, 0, 0, Iterator, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMNodeList_item, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMCharacterData_appendData, 0, 1, IS_TRUE, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMCharacterData_substringData, 0, 0, 2)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, count, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMCharacterData_insertData, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMCharacterData_deleteData, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, count, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMCharacterData_replaceData, 0, 3, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, count, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DOMCharacterData_replaceWith arginfo_class_DOMParentNode_append

#define arginfo_class_DOMCharacterData_remove arginfo_class_DOMChildNode_remove

#define arginfo_class_DOMCharacterData_before arginfo_class_DOMParentNode_append

#define arginfo_class_DOMCharacterData_after arginfo_class_DOMParentNode_append

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMAttr___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, value, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

#define arginfo_class_DOMAttr_isId arginfo_class_DOMNode_hasAttributes

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMElement___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, qualifiedName, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, value, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, namespace, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMElement_getAttribute, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, qualifiedName, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DOMElement_getAttributeNames arginfo_class_DOMNode___sleep

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMElement_getAttributeNS, 0, 2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, localName, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMElement_getAttributeNode, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, qualifiedName, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMElement_getAttributeNodeNS, 0, 0, 2)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, localName, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DOMElement_getElementsByTagName, 0, 1, DOMNodeList, 0)
	CREX_ARG_TYPE_INFO(0, qualifiedName, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DOMElement_getElementsByTagNameNS, 0, 2, DOMNodeList, 0)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, localName, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMElement_hasAttribute, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, qualifiedName, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMElement_hasAttributeNS, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, localName, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DOMElement_removeAttribute arginfo_class_DOMElement_hasAttribute

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMElement_removeAttributeNS, 0, 2, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, localName, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMElement_removeAttributeNode, 0, 0, 1)
	CREX_ARG_OBJ_INFO(0, attr, DOMAttr, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMElement_setAttribute, 0, 0, 2)
	CREX_ARG_TYPE_INFO(0, qualifiedName, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMElement_setAttributeNS, 0, 3, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, qualifiedName, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DOMElement_setAttributeNode arginfo_class_DOMElement_removeAttributeNode

#define arginfo_class_DOMElement_setAttributeNodeNS arginfo_class_DOMElement_removeAttributeNode

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMElement_setIdAttribute, 0, 2, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, qualifiedName, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, isId, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMElement_setIdAttributeNS, 0, 3, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, qualifiedName, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, isId, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMElement_setIdAttributeNode, 0, 2, IS_VOID, 0)
	CREX_ARG_OBJ_INFO(0, attr, DOMAttr, 0)
	CREX_ARG_TYPE_INFO(0, isId, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_DOMElement_toggleAttribute, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, qualifiedName, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, force, _IS_BOOL, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_class_DOMElement_remove arginfo_class_DOMChildNode_remove

#define arginfo_class_DOMElement_before arginfo_class_DOMParentNode_append

#define arginfo_class_DOMElement_after arginfo_class_DOMParentNode_append

#define arginfo_class_DOMElement_replaceWith arginfo_class_DOMParentNode_append

#define arginfo_class_DOMElement_append arginfo_class_DOMParentNode_append

#define arginfo_class_DOMElement_prepend arginfo_class_DOMParentNode_append

#define arginfo_class_DOMElement_replaceChildren arginfo_class_DOMParentNode_append

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_DOMElement_insertAdjacentElement, 0, 2, DOMElement, 1)
	CREX_ARG_TYPE_INFO(0, where, IS_STRING, 0)
	CREX_ARG_OBJ_INFO(0, element, DOMElement, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_DOMElement_insertAdjacentText, 0, 2, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, where, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMDocument___main, 0, 0, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, version, IS_STRING, 0, "\"1.0\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMDocument_createAttribute, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, localName, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMDocument_createAttributeNS, 0, 0, 2)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, qualifiedName, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DOMDocument_createCDATASection arginfo_class_DOMCdataSection___main

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DOMDocument_createComment, 0, 1, DOMComment, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DOMDocument_createDocumentFragment, 0, 0, DOMDocumentFragment, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMDocument_createElement, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, localName, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, value, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMDocument_createElementNS, 0, 0, 2)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, qualifiedName, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, value, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMDocument_createEntityReference, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMDocument_createProcessingInstruction, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, target, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DOMDocument_createTextNode, 0, 1, DOMText, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DOMDocument_getElementById, 0, 1, DOMElement, 1)
	CREX_ARG_TYPE_INFO(0, elementId, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DOMDocument_getElementsByTagName arginfo_class_DOMElement_getElementsByTagName

#define arginfo_class_DOMDocument_getElementsByTagNameNS arginfo_class_DOMElement_getElementsByTagNameNS

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMDocument_importNode, 0, 0, 1)
	CREX_ARG_OBJ_INFO(0, node, DOMNode, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, deep, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMDocument_load, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMDocument_loadXML, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, source, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

#define arginfo_class_DOMDocument_normalizeDocument arginfo_class_DOMNode_normalize

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMDocument_registerNodeClass, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, baseClass, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, extendedClass, IS_STRING, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_DOMDocument_save, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

#if defined(LIBXML_HTML_ENABLED)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMDocument_loadHTML, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, source, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
CREX_END_ARG_INFO()
#endif

#if defined(LIBXML_HTML_ENABLED)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMDocument_loadHTMLFile, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
CREX_END_ARG_INFO()
#endif

#if defined(LIBXML_HTML_ENABLED)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_DOMDocument_saveHTML, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, node, DOMNode, 1, "null")
CREX_END_ARG_INFO()
#endif

#if defined(LIBXML_HTML_ENABLED)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_DOMDocument_saveHTMLFile, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_DOMDocument_saveXML, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, node, DOMNode, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

#if defined(LIBXML_SCHEMAS_ENABLED)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMDocument_schemaValidate, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMDocument_schemaValidateSource, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, source, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMDocument_relaxNGValidate, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#if defined(LIBXML_SCHEMAS_ENABLED)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMDocument_relaxNGValidateSource, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, source, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#define arginfo_class_DOMDocument_validate arginfo_class_DOMNode_hasAttributes

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_DOMDocument_xinclude, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_DOMDocument_adoptNode, 0, 1, DOMNode, MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, node, DOMNode, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DOMDocument_append arginfo_class_DOMParentNode_append

#define arginfo_class_DOMDocument_prepend arginfo_class_DOMParentNode_append

#define arginfo_class_DOMDocument_replaceChildren arginfo_class_DOMParentNode_append

#define arginfo_class_DOMText___main arginfo_class_DOMComment___main

#define arginfo_class_DOMText_isWhitespaceInElementContent arginfo_class_DOMNode_hasAttributes

#define arginfo_class_DOMText_isElementContentWhitespace arginfo_class_DOMNode_hasAttributes

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMText_splitText, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DOMNamedNodeMap_getNamedItem, 0, 1, DOMNode, 1)
	CREX_ARG_TYPE_INFO(0, qualifiedName, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DOMNamedNodeMap_getNamedItemNS, 0, 2, DOMNode, 1)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, localName, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DOMNamedNodeMap_item, 0, 1, DOMNode, 1)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DOMNamedNodeMap_count arginfo_class_DOMNode_getLineNo

#define arginfo_class_DOMNamedNodeMap_getIterator arginfo_class_DOMNodeList_getIterator

#define arginfo_class_DOMEntityReference___main arginfo_class_DOMDocument_createEntityReference

#define arginfo_class_DOMProcessingInstruction___main arginfo_class_DOMAttr___main

#if defined(LIBXML_XPATH_ENABLED)
CREX_BEGIN_ARG_INFO_EX(arginfo_class_DOMXPath___main, 0, 0, 1)
	CREX_ARG_OBJ_INFO(0, document, DOMDocument, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, registerNodeNS, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()
#endif

#if defined(LIBXML_XPATH_ENABLED)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMXPath_evaluate, 0, 1, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, expression, IS_STRING, 0)
	CREX_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, contextNode, DOMNode, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, registerNodeNS, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()
#endif

#if defined(LIBXML_XPATH_ENABLED)
#define arginfo_class_DOMXPath_query arginfo_class_DOMXPath_evaluate
#endif

#if defined(LIBXML_XPATH_ENABLED)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMXPath_registerNamespace, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, prefix, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#if defined(LIBXML_XPATH_ENABLED)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DOMXPath_registerCrxFunctions, 0, 0, IS_VOID, 0)
	CREX_ARG_TYPE_MASK(0, restrict, MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_NULL, "null")
CREX_END_ARG_INFO()
#endif


CREX_FUNCTION(dom_import_simplexml);
CREX_METHOD(DOMCdataSection, __main);
CREX_METHOD(DOMComment, __main);
CREX_METHOD(DOMNode, __sleep);
CREX_METHOD(DOMNode, __wakeup);
CREX_METHOD(DOMNode, appendChild);
CREX_METHOD(DOMNode, C14N);
CREX_METHOD(DOMNode, C14NFile);
CREX_METHOD(DOMNode, cloneNode);
CREX_METHOD(DOMNode, getLineNo);
CREX_METHOD(DOMNode, getNodePath);
CREX_METHOD(DOMNode, hasAttributes);
CREX_METHOD(DOMNode, hasChildNodes);
CREX_METHOD(DOMNode, insertBefore);
CREX_METHOD(DOMNode, isDefaultNamespace);
CREX_METHOD(DOMNode, isSameNode);
CREX_METHOD(DOMNode, isEqualNode);
CREX_METHOD(DOMNode, isSupported);
CREX_METHOD(DOMNode, lookupNamespaceURI);
CREX_METHOD(DOMNode, lookupPrefix);
CREX_METHOD(DOMNode, normalize);
CREX_METHOD(DOMNode, removeChild);
CREX_METHOD(DOMNode, replaceChild);
CREX_METHOD(DOMNode, contains);
CREX_METHOD(DOMNode, getRootNode);
CREX_METHOD(DOMImplementation, getFeature);
CREX_METHOD(DOMImplementation, hasFeature);
CREX_METHOD(DOMImplementation, createDocumentType);
CREX_METHOD(DOMImplementation, createDocument);
CREX_METHOD(DOMDocumentFragment, __main);
CREX_METHOD(DOMDocumentFragment, appendXML);
CREX_METHOD(DOMDocumentFragment, append);
CREX_METHOD(DOMDocumentFragment, prepend);
CREX_METHOD(DOMDocumentFragment, replaceChildren);
CREX_METHOD(DOMNodeList, count);
CREX_METHOD(DOMNodeList, getIterator);
CREX_METHOD(DOMNodeList, item);
CREX_METHOD(DOMCharacterData, appendData);
CREX_METHOD(DOMCharacterData, substringData);
CREX_METHOD(DOMCharacterData, insertData);
CREX_METHOD(DOMCharacterData, deleteData);
CREX_METHOD(DOMCharacterData, replaceData);
CREX_METHOD(DOMCharacterData, replaceWith);
CREX_METHOD(DOMCharacterData, remove);
CREX_METHOD(DOMCharacterData, before);
CREX_METHOD(DOMCharacterData, after);
CREX_METHOD(DOMAttr, __main);
CREX_METHOD(DOMAttr, isId);
CREX_METHOD(DOMElement, __main);
CREX_METHOD(DOMElement, getAttribute);
CREX_METHOD(DOMElement, getAttributeNames);
CREX_METHOD(DOMElement, getAttributeNS);
CREX_METHOD(DOMElement, getAttributeNode);
CREX_METHOD(DOMElement, getAttributeNodeNS);
CREX_METHOD(DOMElement, getElementsByTagName);
CREX_METHOD(DOMElement, getElementsByTagNameNS);
CREX_METHOD(DOMElement, hasAttribute);
CREX_METHOD(DOMElement, hasAttributeNS);
CREX_METHOD(DOMElement, removeAttribute);
CREX_METHOD(DOMElement, removeAttributeNS);
CREX_METHOD(DOMElement, removeAttributeNode);
CREX_METHOD(DOMElement, setAttribute);
CREX_METHOD(DOMElement, setAttributeNS);
CREX_METHOD(DOMElement, setAttributeNode);
CREX_METHOD(DOMElement, setAttributeNodeNS);
CREX_METHOD(DOMElement, setIdAttribute);
CREX_METHOD(DOMElement, setIdAttributeNS);
CREX_METHOD(DOMElement, setIdAttributeNode);
CREX_METHOD(DOMElement, toggleAttribute);
CREX_METHOD(DOMElement, remove);
CREX_METHOD(DOMElement, before);
CREX_METHOD(DOMElement, after);
CREX_METHOD(DOMElement, replaceWith);
CREX_METHOD(DOMElement, append);
CREX_METHOD(DOMElement, prepend);
CREX_METHOD(DOMElement, replaceChildren);
CREX_METHOD(DOMElement, insertAdjacentElement);
CREX_METHOD(DOMElement, insertAdjacentText);
CREX_METHOD(DOMDocument, __main);
CREX_METHOD(DOMDocument, createAttribute);
CREX_METHOD(DOMDocument, createAttributeNS);
CREX_METHOD(DOMDocument, createCDATASection);
CREX_METHOD(DOMDocument, createComment);
CREX_METHOD(DOMDocument, createDocumentFragment);
CREX_METHOD(DOMDocument, createElement);
CREX_METHOD(DOMDocument, createElementNS);
CREX_METHOD(DOMDocument, createEntityReference);
CREX_METHOD(DOMDocument, createProcessingInstruction);
CREX_METHOD(DOMDocument, createTextNode);
CREX_METHOD(DOMDocument, getElementById);
CREX_METHOD(DOMDocument, getElementsByTagName);
CREX_METHOD(DOMDocument, getElementsByTagNameNS);
CREX_METHOD(DOMDocument, importNode);
CREX_METHOD(DOMDocument, load);
CREX_METHOD(DOMDocument, loadXML);
CREX_METHOD(DOMDocument, normalizeDocument);
CREX_METHOD(DOMDocument, registerNodeClass);
CREX_METHOD(DOMDocument, save);
#if defined(LIBXML_HTML_ENABLED)
CREX_METHOD(DOMDocument, loadHTML);
#endif
#if defined(LIBXML_HTML_ENABLED)
CREX_METHOD(DOMDocument, loadHTMLFile);
#endif
#if defined(LIBXML_HTML_ENABLED)
CREX_METHOD(DOMDocument, saveHTML);
#endif
#if defined(LIBXML_HTML_ENABLED)
CREX_METHOD(DOMDocument, saveHTMLFile);
#endif
CREX_METHOD(DOMDocument, saveXML);
#if defined(LIBXML_SCHEMAS_ENABLED)
CREX_METHOD(DOMDocument, schemaValidate);
#endif
#if defined(LIBXML_SCHEMAS_ENABLED)
CREX_METHOD(DOMDocument, schemaValidateSource);
#endif
#if defined(LIBXML_SCHEMAS_ENABLED)
CREX_METHOD(DOMDocument, relaxNGValidate);
#endif
#if defined(LIBXML_SCHEMAS_ENABLED)
CREX_METHOD(DOMDocument, relaxNGValidateSource);
#endif
CREX_METHOD(DOMDocument, validate);
CREX_METHOD(DOMDocument, xinclude);
CREX_METHOD(DOMDocument, adoptNode);
CREX_METHOD(DOMDocument, append);
CREX_METHOD(DOMDocument, prepend);
CREX_METHOD(DOMDocument, replaceChildren);
CREX_METHOD(DOMText, __main);
CREX_METHOD(DOMText, isWhitespaceInElementContent);
CREX_METHOD(DOMText, splitText);
CREX_METHOD(DOMNamedNodeMap, getNamedItem);
CREX_METHOD(DOMNamedNodeMap, getNamedItemNS);
CREX_METHOD(DOMNamedNodeMap, item);
CREX_METHOD(DOMNamedNodeMap, count);
CREX_METHOD(DOMNamedNodeMap, getIterator);
CREX_METHOD(DOMEntityReference, __main);
CREX_METHOD(DOMProcessingInstruction, __main);
#if defined(LIBXML_XPATH_ENABLED)
CREX_METHOD(DOMXPath, __main);
#endif
#if defined(LIBXML_XPATH_ENABLED)
CREX_METHOD(DOMXPath, evaluate);
#endif
#if defined(LIBXML_XPATH_ENABLED)
CREX_METHOD(DOMXPath, query);
#endif
#if defined(LIBXML_XPATH_ENABLED)
CREX_METHOD(DOMXPath, registerNamespace);
#endif
#if defined(LIBXML_XPATH_ENABLED)
CREX_METHOD(DOMXPath, registerCrxFunctions);
#endif


static const crex_function_entry ext_functions[] = {
	CREX_FE(dom_import_simplexml, arginfo_dom_import_simplexml)
	CREX_FE_END
};


static const crex_function_entry class_DOMDocumentType_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_DOMCdataSection_methods[] = {
	CREX_ME(DOMCdataSection, __main, arginfo_class_DOMCdataSection___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_DOMComment_methods[] = {
	CREX_ME(DOMComment, __main, arginfo_class_DOMComment___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_DOMParentNode_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(DOMParentNode, append, arginfo_class_DOMParentNode_append, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(DOMParentNode, prepend, arginfo_class_DOMParentNode_prepend, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(DOMParentNode, replaceChildren, arginfo_class_DOMParentNode_replaceChildren, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_FE_END
};


static const crex_function_entry class_DOMChildNode_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(DOMChildNode, remove, arginfo_class_DOMChildNode_remove, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(DOMChildNode, before, arginfo_class_DOMChildNode_before, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(DOMChildNode, after, arginfo_class_DOMChildNode_after, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(DOMChildNode, replaceWith, arginfo_class_DOMChildNode_replaceWith, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_FE_END
};


static const crex_function_entry class_DOMNode_methods[] = {
	CREX_ME(DOMNode, __sleep, arginfo_class_DOMNode___sleep, CREX_ACC_PUBLIC)
	CREX_ME(DOMNode, __wakeup, arginfo_class_DOMNode___wakeup, CREX_ACC_PUBLIC)
	CREX_ME(DOMNode, appendChild, arginfo_class_DOMNode_appendChild, CREX_ACC_PUBLIC)
	CREX_ME(DOMNode, C14N, arginfo_class_DOMNode_C14N, CREX_ACC_PUBLIC)
	CREX_ME(DOMNode, C14NFile, arginfo_class_DOMNode_C14NFile, CREX_ACC_PUBLIC)
	CREX_ME(DOMNode, cloneNode, arginfo_class_DOMNode_cloneNode, CREX_ACC_PUBLIC)
	CREX_ME(DOMNode, getLineNo, arginfo_class_DOMNode_getLineNo, CREX_ACC_PUBLIC)
	CREX_ME(DOMNode, getNodePath, arginfo_class_DOMNode_getNodePath, CREX_ACC_PUBLIC)
	CREX_ME(DOMNode, hasAttributes, arginfo_class_DOMNode_hasAttributes, CREX_ACC_PUBLIC)
	CREX_ME(DOMNode, hasChildNodes, arginfo_class_DOMNode_hasChildNodes, CREX_ACC_PUBLIC)
	CREX_ME(DOMNode, insertBefore, arginfo_class_DOMNode_insertBefore, CREX_ACC_PUBLIC)
	CREX_ME(DOMNode, isDefaultNamespace, arginfo_class_DOMNode_isDefaultNamespace, CREX_ACC_PUBLIC)
	CREX_ME(DOMNode, isSameNode, arginfo_class_DOMNode_isSameNode, CREX_ACC_PUBLIC)
	CREX_ME(DOMNode, isEqualNode, arginfo_class_DOMNode_isEqualNode, CREX_ACC_PUBLIC)
	CREX_ME(DOMNode, isSupported, arginfo_class_DOMNode_isSupported, CREX_ACC_PUBLIC)
	CREX_ME(DOMNode, lookupNamespaceURI, arginfo_class_DOMNode_lookupNamespaceURI, CREX_ACC_PUBLIC)
	CREX_ME(DOMNode, lookupPrefix, arginfo_class_DOMNode_lookupPrefix, CREX_ACC_PUBLIC)
	CREX_ME(DOMNode, normalize, arginfo_class_DOMNode_normalize, CREX_ACC_PUBLIC)
	CREX_ME(DOMNode, removeChild, arginfo_class_DOMNode_removeChild, CREX_ACC_PUBLIC)
	CREX_ME(DOMNode, replaceChild, arginfo_class_DOMNode_replaceChild, CREX_ACC_PUBLIC)
	CREX_ME(DOMNode, contains, arginfo_class_DOMNode_contains, CREX_ACC_PUBLIC)
	CREX_ME(DOMNode, getRootNode, arginfo_class_DOMNode_getRootNode, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_DOMNameSpaceNode_methods[] = {
	CREX_MALIAS(DOMNode, __sleep, __sleep, arginfo_class_DOMNameSpaceNode___sleep, CREX_ACC_PUBLIC)
	CREX_MALIAS(DOMNode, __wakeup, __wakeup, arginfo_class_DOMNameSpaceNode___wakeup, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_DOMImplementation_methods[] = {
	CREX_ME(DOMImplementation, getFeature, arginfo_class_DOMImplementation_getFeature, CREX_ACC_PUBLIC)
	CREX_ME(DOMImplementation, hasFeature, arginfo_class_DOMImplementation_hasFeature, CREX_ACC_PUBLIC)
	CREX_ME(DOMImplementation, createDocumentType, arginfo_class_DOMImplementation_createDocumentType, CREX_ACC_PUBLIC)
	CREX_ME(DOMImplementation, createDocument, arginfo_class_DOMImplementation_createDocument, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_DOMDocumentFragment_methods[] = {
	CREX_ME(DOMDocumentFragment, __main, arginfo_class_DOMDocumentFragment___main, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocumentFragment, appendXML, arginfo_class_DOMDocumentFragment_appendXML, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocumentFragment, append, arginfo_class_DOMDocumentFragment_append, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocumentFragment, prepend, arginfo_class_DOMDocumentFragment_prepend, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocumentFragment, replaceChildren, arginfo_class_DOMDocumentFragment_replaceChildren, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_DOMNodeList_methods[] = {
	CREX_ME(DOMNodeList, count, arginfo_class_DOMNodeList_count, CREX_ACC_PUBLIC)
	CREX_ME(DOMNodeList, getIterator, arginfo_class_DOMNodeList_getIterator, CREX_ACC_PUBLIC)
	CREX_ME(DOMNodeList, item, arginfo_class_DOMNodeList_item, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_DOMCharacterData_methods[] = {
	CREX_ME(DOMCharacterData, appendData, arginfo_class_DOMCharacterData_appendData, CREX_ACC_PUBLIC)
	CREX_ME(DOMCharacterData, substringData, arginfo_class_DOMCharacterData_substringData, CREX_ACC_PUBLIC)
	CREX_ME(DOMCharacterData, insertData, arginfo_class_DOMCharacterData_insertData, CREX_ACC_PUBLIC)
	CREX_ME(DOMCharacterData, deleteData, arginfo_class_DOMCharacterData_deleteData, CREX_ACC_PUBLIC)
	CREX_ME(DOMCharacterData, replaceData, arginfo_class_DOMCharacterData_replaceData, CREX_ACC_PUBLIC)
	CREX_ME(DOMCharacterData, replaceWith, arginfo_class_DOMCharacterData_replaceWith, CREX_ACC_PUBLIC)
	CREX_ME(DOMCharacterData, remove, arginfo_class_DOMCharacterData_remove, CREX_ACC_PUBLIC)
	CREX_ME(DOMCharacterData, before, arginfo_class_DOMCharacterData_before, CREX_ACC_PUBLIC)
	CREX_ME(DOMCharacterData, after, arginfo_class_DOMCharacterData_after, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_DOMAttr_methods[] = {
	CREX_ME(DOMAttr, __main, arginfo_class_DOMAttr___main, CREX_ACC_PUBLIC)
	CREX_ME(DOMAttr, isId, arginfo_class_DOMAttr_isId, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_DOMElement_methods[] = {
	CREX_ME(DOMElement, __main, arginfo_class_DOMElement___main, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, getAttribute, arginfo_class_DOMElement_getAttribute, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, getAttributeNames, arginfo_class_DOMElement_getAttributeNames, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, getAttributeNS, arginfo_class_DOMElement_getAttributeNS, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, getAttributeNode, arginfo_class_DOMElement_getAttributeNode, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, getAttributeNodeNS, arginfo_class_DOMElement_getAttributeNodeNS, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, getElementsByTagName, arginfo_class_DOMElement_getElementsByTagName, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, getElementsByTagNameNS, arginfo_class_DOMElement_getElementsByTagNameNS, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, hasAttribute, arginfo_class_DOMElement_hasAttribute, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, hasAttributeNS, arginfo_class_DOMElement_hasAttributeNS, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, removeAttribute, arginfo_class_DOMElement_removeAttribute, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, removeAttributeNS, arginfo_class_DOMElement_removeAttributeNS, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, removeAttributeNode, arginfo_class_DOMElement_removeAttributeNode, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, setAttribute, arginfo_class_DOMElement_setAttribute, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, setAttributeNS, arginfo_class_DOMElement_setAttributeNS, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, setAttributeNode, arginfo_class_DOMElement_setAttributeNode, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, setAttributeNodeNS, arginfo_class_DOMElement_setAttributeNodeNS, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, setIdAttribute, arginfo_class_DOMElement_setIdAttribute, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, setIdAttributeNS, arginfo_class_DOMElement_setIdAttributeNS, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, setIdAttributeNode, arginfo_class_DOMElement_setIdAttributeNode, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, toggleAttribute, arginfo_class_DOMElement_toggleAttribute, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, remove, arginfo_class_DOMElement_remove, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, before, arginfo_class_DOMElement_before, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, after, arginfo_class_DOMElement_after, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, replaceWith, arginfo_class_DOMElement_replaceWith, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, append, arginfo_class_DOMElement_append, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, prepend, arginfo_class_DOMElement_prepend, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, replaceChildren, arginfo_class_DOMElement_replaceChildren, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, insertAdjacentElement, arginfo_class_DOMElement_insertAdjacentElement, CREX_ACC_PUBLIC)
	CREX_ME(DOMElement, insertAdjacentText, arginfo_class_DOMElement_insertAdjacentText, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_DOMDocument_methods[] = {
	CREX_ME(DOMDocument, __main, arginfo_class_DOMDocument___main, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, createAttribute, arginfo_class_DOMDocument_createAttribute, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, createAttributeNS, arginfo_class_DOMDocument_createAttributeNS, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, createCDATASection, arginfo_class_DOMDocument_createCDATASection, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, createComment, arginfo_class_DOMDocument_createComment, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, createDocumentFragment, arginfo_class_DOMDocument_createDocumentFragment, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, createElement, arginfo_class_DOMDocument_createElement, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, createElementNS, arginfo_class_DOMDocument_createElementNS, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, createEntityReference, arginfo_class_DOMDocument_createEntityReference, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, createProcessingInstruction, arginfo_class_DOMDocument_createProcessingInstruction, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, createTextNode, arginfo_class_DOMDocument_createTextNode, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, getElementById, arginfo_class_DOMDocument_getElementById, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, getElementsByTagName, arginfo_class_DOMDocument_getElementsByTagName, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, getElementsByTagNameNS, arginfo_class_DOMDocument_getElementsByTagNameNS, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, importNode, arginfo_class_DOMDocument_importNode, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, load, arginfo_class_DOMDocument_load, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, loadXML, arginfo_class_DOMDocument_loadXML, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, normalizeDocument, arginfo_class_DOMDocument_normalizeDocument, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, registerNodeClass, arginfo_class_DOMDocument_registerNodeClass, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, save, arginfo_class_DOMDocument_save, CREX_ACC_PUBLIC)
#if defined(LIBXML_HTML_ENABLED)
	CREX_ME(DOMDocument, loadHTML, arginfo_class_DOMDocument_loadHTML, CREX_ACC_PUBLIC)
#endif
#if defined(LIBXML_HTML_ENABLED)
	CREX_ME(DOMDocument, loadHTMLFile, arginfo_class_DOMDocument_loadHTMLFile, CREX_ACC_PUBLIC)
#endif
#if defined(LIBXML_HTML_ENABLED)
	CREX_ME(DOMDocument, saveHTML, arginfo_class_DOMDocument_saveHTML, CREX_ACC_PUBLIC)
#endif
#if defined(LIBXML_HTML_ENABLED)
	CREX_ME(DOMDocument, saveHTMLFile, arginfo_class_DOMDocument_saveHTMLFile, CREX_ACC_PUBLIC)
#endif
	CREX_ME(DOMDocument, saveXML, arginfo_class_DOMDocument_saveXML, CREX_ACC_PUBLIC)
#if defined(LIBXML_SCHEMAS_ENABLED)
	CREX_ME(DOMDocument, schemaValidate, arginfo_class_DOMDocument_schemaValidate, CREX_ACC_PUBLIC)
#endif
#if defined(LIBXML_SCHEMAS_ENABLED)
	CREX_ME(DOMDocument, schemaValidateSource, arginfo_class_DOMDocument_schemaValidateSource, CREX_ACC_PUBLIC)
#endif
#if defined(LIBXML_SCHEMAS_ENABLED)
	CREX_ME(DOMDocument, relaxNGValidate, arginfo_class_DOMDocument_relaxNGValidate, CREX_ACC_PUBLIC)
#endif
#if defined(LIBXML_SCHEMAS_ENABLED)
	CREX_ME(DOMDocument, relaxNGValidateSource, arginfo_class_DOMDocument_relaxNGValidateSource, CREX_ACC_PUBLIC)
#endif
	CREX_ME(DOMDocument, validate, arginfo_class_DOMDocument_validate, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, xinclude, arginfo_class_DOMDocument_xinclude, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, adoptNode, arginfo_class_DOMDocument_adoptNode, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, append, arginfo_class_DOMDocument_append, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, prepend, arginfo_class_DOMDocument_prepend, CREX_ACC_PUBLIC)
	CREX_ME(DOMDocument, replaceChildren, arginfo_class_DOMDocument_replaceChildren, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_DOMException_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_DOMText_methods[] = {
	CREX_ME(DOMText, __main, arginfo_class_DOMText___main, CREX_ACC_PUBLIC)
	CREX_ME(DOMText, isWhitespaceInElementContent, arginfo_class_DOMText_isWhitespaceInElementContent, CREX_ACC_PUBLIC)
	CREX_MALIAS(DOMText, isElementContentWhitespace, isWhitespaceInElementContent, arginfo_class_DOMText_isElementContentWhitespace, CREX_ACC_PUBLIC)
	CREX_ME(DOMText, splitText, arginfo_class_DOMText_splitText, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_DOMNamedNodeMap_methods[] = {
	CREX_ME(DOMNamedNodeMap, getNamedItem, arginfo_class_DOMNamedNodeMap_getNamedItem, CREX_ACC_PUBLIC)
	CREX_ME(DOMNamedNodeMap, getNamedItemNS, arginfo_class_DOMNamedNodeMap_getNamedItemNS, CREX_ACC_PUBLIC)
	CREX_ME(DOMNamedNodeMap, item, arginfo_class_DOMNamedNodeMap_item, CREX_ACC_PUBLIC)
	CREX_ME(DOMNamedNodeMap, count, arginfo_class_DOMNamedNodeMap_count, CREX_ACC_PUBLIC)
	CREX_ME(DOMNamedNodeMap, getIterator, arginfo_class_DOMNamedNodeMap_getIterator, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_DOMEntity_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_DOMEntityReference_methods[] = {
	CREX_ME(DOMEntityReference, __main, arginfo_class_DOMEntityReference___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_DOMNotation_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_DOMProcessingInstruction_methods[] = {
	CREX_ME(DOMProcessingInstruction, __main, arginfo_class_DOMProcessingInstruction___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};


#if defined(LIBXML_XPATH_ENABLED)
static const crex_function_entry class_DOMXPath_methods[] = {
	CREX_ME(DOMXPath, __main, arginfo_class_DOMXPath___main, CREX_ACC_PUBLIC)
	CREX_ME(DOMXPath, evaluate, arginfo_class_DOMXPath_evaluate, CREX_ACC_PUBLIC)
	CREX_ME(DOMXPath, query, arginfo_class_DOMXPath_query, CREX_ACC_PUBLIC)
	CREX_ME(DOMXPath, registerNamespace, arginfo_class_DOMXPath_registerNamespace, CREX_ACC_PUBLIC)
	CREX_ME(DOMXPath, registerCrxFunctions, arginfo_class_DOMXPath_registerCrxFunctions, CREX_ACC_PUBLIC)
	CREX_FE_END
};
#endif

static void register_crx_dom_symbols(int module_number)
{
	REGISTER_LONG_CONSTANT("XML_ELEMENT_NODE", XML_ELEMENT_NODE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_NODE", XML_ATTRIBUTE_NODE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_TEXT_NODE", XML_TEXT_NODE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_CDATA_SECTION_NODE", XML_CDATA_SECTION_NODE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ENTITY_REF_NODE", XML_ENTITY_REF_NODE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ENTITY_NODE", XML_ENTITY_NODE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_PI_NODE", XML_PI_NODE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_COMMENT_NODE", XML_COMMENT_NODE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_DOCUMENT_NODE", XML_DOCUMENT_NODE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_DOCUMENT_TYPE_NODE", XML_DOCUMENT_TYPE_NODE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_DOCUMENT_FRAG_NODE", XML_DOCUMENT_FRAG_NODE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_NOTATION_NODE", XML_NOTATION_NODE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_HTML_DOCUMENT_NODE", XML_HTML_DOCUMENT_NODE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_DTD_NODE", XML_DTD_NODE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ELEMENT_DECL_NODE", XML_ELEMENT_DECL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_DECL_NODE", XML_ATTRIBUTE_DECL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ENTITY_DECL_NODE", XML_ENTITY_DECL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_NAMESPACE_DECL_NODE", XML_NAMESPACE_DECL, CONST_PERSISTENT);
#if defined(XML_GLOBAL_NAMESPACE)
	REGISTER_LONG_CONSTANT("XML_GLOBAL_NAMESPACE", XML_GLOBAL_NAMESPACE, CONST_PERSISTENT);
#endif
	REGISTER_LONG_CONSTANT("XML_LOCAL_NAMESPACE", XML_LOCAL_NAMESPACE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_CDATA", XML_ATTRIBUTE_CDATA, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_ID", XML_ATTRIBUTE_ID, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_IDREF", XML_ATTRIBUTE_IDREF, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_IDREFS", XML_ATTRIBUTE_IDREFS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_ENTITY", XML_ATTRIBUTE_ENTITIES, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_NMTOKEN", XML_ATTRIBUTE_NMTOKEN, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_NMTOKENS", XML_ATTRIBUTE_NMTOKENS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_ENUMERATION", XML_ATTRIBUTE_ENUMERATION, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XML_ATTRIBUTE_NOTATION", XML_ATTRIBUTE_NOTATION, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_CRX_ERR", CRX_ERR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_INDEX_SIZE_ERR", INDEX_SIZE_ERR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOMSTRING_SIZE_ERR", DOMSTRING_SIZE_ERR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_HIERARCHY_REQUEST_ERR", HIERARCHY_REQUEST_ERR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_WRONG_DOCUMENT_ERR", WRONG_DOCUMENT_ERR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_INVALID_CHARACTER_ERR", INVALID_CHARACTER_ERR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_NO_DATA_ALLOWED_ERR", NO_DATA_ALLOWED_ERR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_NO_MODIFICATION_ALLOWED_ERR", NO_MODIFICATION_ALLOWED_ERR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_NOT_FOUND_ERR", NOT_FOUND_ERR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_NOT_SUPPORTED_ERR", NOT_SUPPORTED_ERR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_INUSE_ATTRIBUTE_ERR", INUSE_ATTRIBUTE_ERR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_INVALID_STATE_ERR", INVALID_STATE_ERR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_SYNTAX_ERR", SYNTAX_ERR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_INVALID_MODIFICATION_ERR", INVALID_MODIFICATION_ERR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_NAMESPACE_ERR", NAMESPACE_ERR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_INVALID_ACCESS_ERR", INVALID_ACCESS_ERR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DOM_VALIDATION_ERR", VALIDATION_ERR, CONST_PERSISTENT);
}

static crex_class_entry *register_class_DOMDocumentType(crex_class_entry *class_entry_DOMNode)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMDocumentType", class_DOMDocumentType_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_DOMNode);

	zval property_name_default_value;
	ZVAL_UNDEF(&property_name_default_value);
	crex_string *property_name_name = crex_string_init("name", sizeof("name") - 1, 1);
	crex_declare_typed_property(class_entry, property_name_name, &property_name_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_name_name);

	zval property_entities_default_value;
	ZVAL_UNDEF(&property_entities_default_value);
	crex_string *property_entities_name = crex_string_init("entities", sizeof("entities") - 1, 1);
	crex_string *property_entities_class_DOMNamedNodeMap = crex_string_init("DOMNamedNodeMap", sizeof("DOMNamedNodeMap")-1, 1);
	crex_declare_typed_property(class_entry, property_entities_name, &property_entities_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_entities_class_DOMNamedNodeMap, 0, 0));
	crex_string_release(property_entities_name);

	zval property_notations_default_value;
	ZVAL_UNDEF(&property_notations_default_value);
	crex_string *property_notations_name = crex_string_init("notations", sizeof("notations") - 1, 1);
	crex_string *property_notations_class_DOMNamedNodeMap = crex_string_init("DOMNamedNodeMap", sizeof("DOMNamedNodeMap")-1, 1);
	crex_declare_typed_property(class_entry, property_notations_name, &property_notations_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_notations_class_DOMNamedNodeMap, 0, 0));
	crex_string_release(property_notations_name);

	zval property_publicId_default_value;
	ZVAL_UNDEF(&property_publicId_default_value);
	crex_string *property_publicId_name = crex_string_init("publicId", sizeof("publicId") - 1, 1);
	crex_declare_typed_property(class_entry, property_publicId_name, &property_publicId_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_publicId_name);

	zval property_systemId_default_value;
	ZVAL_UNDEF(&property_systemId_default_value);
	crex_string *property_systemId_name = crex_string_init("systemId", sizeof("systemId") - 1, 1);
	crex_declare_typed_property(class_entry, property_systemId_name, &property_systemId_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_systemId_name);

	zval property_internalSubset_default_value;
	ZVAL_UNDEF(&property_internalSubset_default_value);
	crex_string *property_internalSubset_name = crex_string_init("internalSubset", sizeof("internalSubset") - 1, 1);
	crex_declare_typed_property(class_entry, property_internalSubset_name, &property_internalSubset_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_internalSubset_name);

	return class_entry;
}

static crex_class_entry *register_class_DOMCdataSection(crex_class_entry *class_entry_DOMText)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMCdataSection", class_DOMCdataSection_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_DOMText);

	return class_entry;
}

static crex_class_entry *register_class_DOMComment(crex_class_entry *class_entry_DOMCharacterData)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMComment", class_DOMComment_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_DOMCharacterData);

	return class_entry;
}

static crex_class_entry *register_class_DOMParentNode(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMParentNode", class_DOMParentNode_methods);
	class_entry = crex_register_internal_interface(&ce);

	return class_entry;
}

static crex_class_entry *register_class_DOMChildNode(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMChildNode", class_DOMChildNode_methods);
	class_entry = crex_register_internal_interface(&ce);

	return class_entry;
}

static crex_class_entry *register_class_DOMNode(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMNode", class_DOMNode_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	zval property_nodeName_default_value;
	ZVAL_UNDEF(&property_nodeName_default_value);
	crex_string *property_nodeName_name = crex_string_init("nodeName", sizeof("nodeName") - 1, 1);
	crex_declare_typed_property(class_entry, property_nodeName_name, &property_nodeName_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_nodeName_name);

	zval property_nodeValue_default_value;
	ZVAL_UNDEF(&property_nodeValue_default_value);
	crex_string *property_nodeValue_name = crex_string_init("nodeValue", sizeof("nodeValue") - 1, 1);
	crex_declare_typed_property(class_entry, property_nodeValue_name, &property_nodeValue_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_nodeValue_name);

	zval property_nodeType_default_value;
	ZVAL_UNDEF(&property_nodeType_default_value);
	crex_string *property_nodeType_name = crex_string_init("nodeType", sizeof("nodeType") - 1, 1);
	crex_declare_typed_property(class_entry, property_nodeType_name, &property_nodeType_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_nodeType_name);

	zval property_parentNode_default_value;
	ZVAL_UNDEF(&property_parentNode_default_value);
	crex_string *property_parentNode_name = crex_string_init("parentNode", sizeof("parentNode") - 1, 1);
	crex_string *property_parentNode_class_DOMNode = crex_string_init("DOMNode", sizeof("DOMNode")-1, 1);
	crex_declare_typed_property(class_entry, property_parentNode_name, &property_parentNode_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_parentNode_class_DOMNode, 0, MAY_BE_NULL));
	crex_string_release(property_parentNode_name);

	zval property_parentElement_default_value;
	ZVAL_UNDEF(&property_parentElement_default_value);
	crex_string *property_parentElement_name = crex_string_init("parentElement", sizeof("parentElement") - 1, 1);
	crex_string *property_parentElement_class_DOMElement = crex_string_init("DOMElement", sizeof("DOMElement")-1, 1);
	crex_declare_typed_property(class_entry, property_parentElement_name, &property_parentElement_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_parentElement_class_DOMElement, 0, MAY_BE_NULL));
	crex_string_release(property_parentElement_name);

	zval property_childNodes_default_value;
	ZVAL_UNDEF(&property_childNodes_default_value);
	crex_string *property_childNodes_name = crex_string_init("childNodes", sizeof("childNodes") - 1, 1);
	crex_string *property_childNodes_class_DOMNodeList = crex_string_init("DOMNodeList", sizeof("DOMNodeList")-1, 1);
	crex_declare_typed_property(class_entry, property_childNodes_name, &property_childNodes_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_childNodes_class_DOMNodeList, 0, 0));
	crex_string_release(property_childNodes_name);

	zval property_firstChild_default_value;
	ZVAL_UNDEF(&property_firstChild_default_value);
	crex_string *property_firstChild_name = crex_string_init("firstChild", sizeof("firstChild") - 1, 1);
	crex_string *property_firstChild_class_DOMNode = crex_string_init("DOMNode", sizeof("DOMNode")-1, 1);
	crex_declare_typed_property(class_entry, property_firstChild_name, &property_firstChild_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_firstChild_class_DOMNode, 0, MAY_BE_NULL));
	crex_string_release(property_firstChild_name);

	zval property_lastChild_default_value;
	ZVAL_UNDEF(&property_lastChild_default_value);
	crex_string *property_lastChild_name = crex_string_init("lastChild", sizeof("lastChild") - 1, 1);
	crex_string *property_lastChild_class_DOMNode = crex_string_init("DOMNode", sizeof("DOMNode")-1, 1);
	crex_declare_typed_property(class_entry, property_lastChild_name, &property_lastChild_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_lastChild_class_DOMNode, 0, MAY_BE_NULL));
	crex_string_release(property_lastChild_name);

	zval property_previousSibling_default_value;
	ZVAL_UNDEF(&property_previousSibling_default_value);
	crex_string *property_previousSibling_name = crex_string_init("previousSibling", sizeof("previousSibling") - 1, 1);
	crex_string *property_previousSibling_class_DOMNode = crex_string_init("DOMNode", sizeof("DOMNode")-1, 1);
	crex_declare_typed_property(class_entry, property_previousSibling_name, &property_previousSibling_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_previousSibling_class_DOMNode, 0, MAY_BE_NULL));
	crex_string_release(property_previousSibling_name);

	zval property_nextSibling_default_value;
	ZVAL_UNDEF(&property_nextSibling_default_value);
	crex_string *property_nextSibling_name = crex_string_init("nextSibling", sizeof("nextSibling") - 1, 1);
	crex_string *property_nextSibling_class_DOMNode = crex_string_init("DOMNode", sizeof("DOMNode")-1, 1);
	crex_declare_typed_property(class_entry, property_nextSibling_name, &property_nextSibling_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_nextSibling_class_DOMNode, 0, MAY_BE_NULL));
	crex_string_release(property_nextSibling_name);

	zval property_attributes_default_value;
	ZVAL_UNDEF(&property_attributes_default_value);
	crex_string *property_attributes_name = crex_string_init("attributes", sizeof("attributes") - 1, 1);
	crex_string *property_attributes_class_DOMNamedNodeMap = crex_string_init("DOMNamedNodeMap", sizeof("DOMNamedNodeMap")-1, 1);
	crex_declare_typed_property(class_entry, property_attributes_name, &property_attributes_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_attributes_class_DOMNamedNodeMap, 0, MAY_BE_NULL));
	crex_string_release(property_attributes_name);

	zval property_isConnected_default_value;
	ZVAL_UNDEF(&property_isConnected_default_value);
	crex_string *property_isConnected_name = crex_string_init("isConnected", sizeof("isConnected") - 1, 1);
	crex_declare_typed_property(class_entry, property_isConnected_name, &property_isConnected_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property_isConnected_name);

	zval property_ownerDocument_default_value;
	ZVAL_UNDEF(&property_ownerDocument_default_value);
	crex_string *property_ownerDocument_name = crex_string_init("ownerDocument", sizeof("ownerDocument") - 1, 1);
	crex_string *property_ownerDocument_class_DOMDocument = crex_string_init("DOMDocument", sizeof("DOMDocument")-1, 1);
	crex_declare_typed_property(class_entry, property_ownerDocument_name, &property_ownerDocument_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_ownerDocument_class_DOMDocument, 0, MAY_BE_NULL));
	crex_string_release(property_ownerDocument_name);

	zval property_namespaceURI_default_value;
	ZVAL_UNDEF(&property_namespaceURI_default_value);
	crex_string *property_namespaceURI_name = crex_string_init("namespaceURI", sizeof("namespaceURI") - 1, 1);
	crex_declare_typed_property(class_entry, property_namespaceURI_name, &property_namespaceURI_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_namespaceURI_name);

	zval property_prefix_default_value;
	ZVAL_UNDEF(&property_prefix_default_value);
	crex_string *property_prefix_name = crex_string_init("prefix", sizeof("prefix") - 1, 1);
	crex_declare_typed_property(class_entry, property_prefix_name, &property_prefix_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_prefix_name);

	zval property_localName_default_value;
	ZVAL_UNDEF(&property_localName_default_value);
	crex_string *property_localName_name = crex_string_init("localName", sizeof("localName") - 1, 1);
	crex_declare_typed_property(class_entry, property_localName_name, &property_localName_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_localName_name);

	zval property_baseURI_default_value;
	ZVAL_UNDEF(&property_baseURI_default_value);
	crex_string *property_baseURI_name = crex_string_init("baseURI", sizeof("baseURI") - 1, 1);
	crex_declare_typed_property(class_entry, property_baseURI_name, &property_baseURI_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_baseURI_name);

	zval property_textContent_default_value;
	ZVAL_UNDEF(&property_textContent_default_value);
	crex_string *property_textContent_name = crex_string_init("textContent", sizeof("textContent") - 1, 1);
	crex_declare_typed_property(class_entry, property_textContent_name, &property_textContent_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_textContent_name);

	return class_entry;
}

static crex_class_entry *register_class_DOMNameSpaceNode(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMNameSpaceNode", class_DOMNameSpaceNode_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	zval property_nodeName_default_value;
	ZVAL_UNDEF(&property_nodeName_default_value);
	crex_string *property_nodeName_name = crex_string_init("nodeName", sizeof("nodeName") - 1, 1);
	crex_declare_typed_property(class_entry, property_nodeName_name, &property_nodeName_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_nodeName_name);

	zval property_nodeValue_default_value;
	ZVAL_UNDEF(&property_nodeValue_default_value);
	crex_string *property_nodeValue_name = crex_string_init("nodeValue", sizeof("nodeValue") - 1, 1);
	crex_declare_typed_property(class_entry, property_nodeValue_name, &property_nodeValue_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_nodeValue_name);

	zval property_nodeType_default_value;
	ZVAL_UNDEF(&property_nodeType_default_value);
	crex_string *property_nodeType_name = crex_string_init("nodeType", sizeof("nodeType") - 1, 1);
	crex_declare_typed_property(class_entry, property_nodeType_name, &property_nodeType_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_nodeType_name);

	zval property_prefix_default_value;
	ZVAL_UNDEF(&property_prefix_default_value);
	crex_string *property_prefix_name = crex_string_init("prefix", sizeof("prefix") - 1, 1);
	crex_declare_typed_property(class_entry, property_prefix_name, &property_prefix_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_prefix_name);

	zval property_localName_default_value;
	ZVAL_UNDEF(&property_localName_default_value);
	crex_string *property_localName_name = crex_string_init("localName", sizeof("localName") - 1, 1);
	crex_declare_typed_property(class_entry, property_localName_name, &property_localName_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_localName_name);

	zval property_namespaceURI_default_value;
	ZVAL_UNDEF(&property_namespaceURI_default_value);
	crex_string *property_namespaceURI_name = crex_string_init("namespaceURI", sizeof("namespaceURI") - 1, 1);
	crex_declare_typed_property(class_entry, property_namespaceURI_name, &property_namespaceURI_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_namespaceURI_name);

	zval property_isConnected_default_value;
	ZVAL_UNDEF(&property_isConnected_default_value);
	crex_string *property_isConnected_name = crex_string_init("isConnected", sizeof("isConnected") - 1, 1);
	crex_declare_typed_property(class_entry, property_isConnected_name, &property_isConnected_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property_isConnected_name);

	zval property_ownerDocument_default_value;
	ZVAL_UNDEF(&property_ownerDocument_default_value);
	crex_string *property_ownerDocument_name = crex_string_init("ownerDocument", sizeof("ownerDocument") - 1, 1);
	crex_string *property_ownerDocument_class_DOMDocument = crex_string_init("DOMDocument", sizeof("DOMDocument")-1, 1);
	crex_declare_typed_property(class_entry, property_ownerDocument_name, &property_ownerDocument_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_ownerDocument_class_DOMDocument, 0, MAY_BE_NULL));
	crex_string_release(property_ownerDocument_name);

	zval property_parentNode_default_value;
	ZVAL_UNDEF(&property_parentNode_default_value);
	crex_string *property_parentNode_name = crex_string_init("parentNode", sizeof("parentNode") - 1, 1);
	crex_string *property_parentNode_class_DOMNode = crex_string_init("DOMNode", sizeof("DOMNode")-1, 1);
	crex_declare_typed_property(class_entry, property_parentNode_name, &property_parentNode_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_parentNode_class_DOMNode, 0, MAY_BE_NULL));
	crex_string_release(property_parentNode_name);

	zval property_parentElement_default_value;
	ZVAL_UNDEF(&property_parentElement_default_value);
	crex_string *property_parentElement_name = crex_string_init("parentElement", sizeof("parentElement") - 1, 1);
	crex_string *property_parentElement_class_DOMElement = crex_string_init("DOMElement", sizeof("DOMElement")-1, 1);
	crex_declare_typed_property(class_entry, property_parentElement_name, &property_parentElement_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_parentElement_class_DOMElement, 0, MAY_BE_NULL));
	crex_string_release(property_parentElement_name);

	return class_entry;
}

static crex_class_entry *register_class_DOMImplementation(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMImplementation", class_DOMImplementation_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	return class_entry;
}

static crex_class_entry *register_class_DOMDocumentFragment(crex_class_entry *class_entry_DOMNode, crex_class_entry *class_entry_DOMParentNode)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMDocumentFragment", class_DOMDocumentFragment_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_DOMNode);
	crex_class_implements(class_entry, 1, class_entry_DOMParentNode);

	zval property_firstElementChild_default_value;
	ZVAL_UNDEF(&property_firstElementChild_default_value);
	crex_string *property_firstElementChild_name = crex_string_init("firstElementChild", sizeof("firstElementChild") - 1, 1);
	crex_string *property_firstElementChild_class_DOMElement = crex_string_init("DOMElement", sizeof("DOMElement")-1, 1);
	crex_declare_typed_property(class_entry, property_firstElementChild_name, &property_firstElementChild_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_firstElementChild_class_DOMElement, 0, MAY_BE_NULL));
	crex_string_release(property_firstElementChild_name);

	zval property_lastElementChild_default_value;
	ZVAL_UNDEF(&property_lastElementChild_default_value);
	crex_string *property_lastElementChild_name = crex_string_init("lastElementChild", sizeof("lastElementChild") - 1, 1);
	crex_string *property_lastElementChild_class_DOMElement = crex_string_init("DOMElement", sizeof("DOMElement")-1, 1);
	crex_declare_typed_property(class_entry, property_lastElementChild_name, &property_lastElementChild_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_lastElementChild_class_DOMElement, 0, MAY_BE_NULL));
	crex_string_release(property_lastElementChild_name);

	zval property_childElementCount_default_value;
	ZVAL_UNDEF(&property_childElementCount_default_value);
	crex_string *property_childElementCount_name = crex_string_init("childElementCount", sizeof("childElementCount") - 1, 1);
	crex_declare_typed_property(class_entry, property_childElementCount_name, &property_childElementCount_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_childElementCount_name);

	return class_entry;
}

static crex_class_entry *register_class_DOMNodeList(crex_class_entry *class_entry_IteratorAggregate, crex_class_entry *class_entry_Countable)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMNodeList", class_DOMNodeList_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	crex_class_implements(class_entry, 2, class_entry_IteratorAggregate, class_entry_Countable);

	zval property_length_default_value;
	ZVAL_UNDEF(&property_length_default_value);
	crex_string *property_length_name = crex_string_init("length", sizeof("length") - 1, 1);
	crex_declare_typed_property(class_entry, property_length_name, &property_length_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_length_name);

	return class_entry;
}

static crex_class_entry *register_class_DOMCharacterData(crex_class_entry *class_entry_DOMNode, crex_class_entry *class_entry_DOMChildNode)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMCharacterData", class_DOMCharacterData_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_DOMNode);
	crex_class_implements(class_entry, 1, class_entry_DOMChildNode);

	zval property_data_default_value;
	ZVAL_UNDEF(&property_data_default_value);
	crex_string *property_data_name = crex_string_init("data", sizeof("data") - 1, 1);
	crex_declare_typed_property(class_entry, property_data_name, &property_data_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_data_name);

	zval property_length_default_value;
	ZVAL_UNDEF(&property_length_default_value);
	crex_string *property_length_name = crex_string_init("length", sizeof("length") - 1, 1);
	crex_declare_typed_property(class_entry, property_length_name, &property_length_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_length_name);

	zval property_previousElementSibling_default_value;
	ZVAL_UNDEF(&property_previousElementSibling_default_value);
	crex_string *property_previousElementSibling_name = crex_string_init("previousElementSibling", sizeof("previousElementSibling") - 1, 1);
	crex_string *property_previousElementSibling_class_DOMElement = crex_string_init("DOMElement", sizeof("DOMElement")-1, 1);
	crex_declare_typed_property(class_entry, property_previousElementSibling_name, &property_previousElementSibling_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_previousElementSibling_class_DOMElement, 0, MAY_BE_NULL));
	crex_string_release(property_previousElementSibling_name);

	zval property_nextElementSibling_default_value;
	ZVAL_UNDEF(&property_nextElementSibling_default_value);
	crex_string *property_nextElementSibling_name = crex_string_init("nextElementSibling", sizeof("nextElementSibling") - 1, 1);
	crex_string *property_nextElementSibling_class_DOMElement = crex_string_init("DOMElement", sizeof("DOMElement")-1, 1);
	crex_declare_typed_property(class_entry, property_nextElementSibling_name, &property_nextElementSibling_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_nextElementSibling_class_DOMElement, 0, MAY_BE_NULL));
	crex_string_release(property_nextElementSibling_name);

	return class_entry;
}

static crex_class_entry *register_class_DOMAttr(crex_class_entry *class_entry_DOMNode)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMAttr", class_DOMAttr_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_DOMNode);

	zval property_name_default_value;
	ZVAL_UNDEF(&property_name_default_value);
	crex_string *property_name_name = crex_string_init("name", sizeof("name") - 1, 1);
	crex_declare_typed_property(class_entry, property_name_name, &property_name_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_name_name);

	zval property_specified_default_value;
	ZVAL_TRUE(&property_specified_default_value);
	crex_string *property_specified_name = crex_string_init("specified", sizeof("specified") - 1, 1);
	crex_declare_typed_property(class_entry, property_specified_name, &property_specified_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property_specified_name);

	zval property_value_default_value;
	ZVAL_UNDEF(&property_value_default_value);
	crex_string *property_value_name = crex_string_init("value", sizeof("value") - 1, 1);
	crex_declare_typed_property(class_entry, property_value_name, &property_value_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_value_name);

	zval property_ownerElement_default_value;
	ZVAL_UNDEF(&property_ownerElement_default_value);
	crex_string *property_ownerElement_name = crex_string_init("ownerElement", sizeof("ownerElement") - 1, 1);
	crex_string *property_ownerElement_class_DOMElement = crex_string_init("DOMElement", sizeof("DOMElement")-1, 1);
	crex_declare_typed_property(class_entry, property_ownerElement_name, &property_ownerElement_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_ownerElement_class_DOMElement, 0, MAY_BE_NULL));
	crex_string_release(property_ownerElement_name);

	zval property_schemaTypeInfo_default_value;
	ZVAL_NULL(&property_schemaTypeInfo_default_value);
	crex_string *property_schemaTypeInfo_name = crex_string_init("schemaTypeInfo", sizeof("schemaTypeInfo") - 1, 1);
	crex_declare_typed_property(class_entry, property_schemaTypeInfo_name, &property_schemaTypeInfo_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_ANY));
	crex_string_release(property_schemaTypeInfo_name);

	return class_entry;
}

static crex_class_entry *register_class_DOMElement(crex_class_entry *class_entry_DOMNode, crex_class_entry *class_entry_DOMParentNode, crex_class_entry *class_entry_DOMChildNode)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMElement", class_DOMElement_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_DOMNode);
	crex_class_implements(class_entry, 2, class_entry_DOMParentNode, class_entry_DOMChildNode);

	zval property_tagName_default_value;
	ZVAL_UNDEF(&property_tagName_default_value);
	crex_string *property_tagName_name = crex_string_init("tagName", sizeof("tagName") - 1, 1);
	crex_declare_typed_property(class_entry, property_tagName_name, &property_tagName_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_tagName_name);

	zval property_className_default_value;
	ZVAL_UNDEF(&property_className_default_value);
	crex_string *property_className_name = crex_string_init("className", sizeof("className") - 1, 1);
	crex_declare_typed_property(class_entry, property_className_name, &property_className_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_className_name);

	zval property_id_default_value;
	ZVAL_UNDEF(&property_id_default_value);
	crex_string *property_id_name = crex_string_init("id", sizeof("id") - 1, 1);
	crex_declare_typed_property(class_entry, property_id_name, &property_id_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_id_name);

	zval property_schemaTypeInfo_default_value;
	ZVAL_NULL(&property_schemaTypeInfo_default_value);
	crex_string *property_schemaTypeInfo_name = crex_string_init("schemaTypeInfo", sizeof("schemaTypeInfo") - 1, 1);
	crex_declare_typed_property(class_entry, property_schemaTypeInfo_name, &property_schemaTypeInfo_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_ANY));
	crex_string_release(property_schemaTypeInfo_name);

	zval property_firstElementChild_default_value;
	ZVAL_UNDEF(&property_firstElementChild_default_value);
	crex_string *property_firstElementChild_name = crex_string_init("firstElementChild", sizeof("firstElementChild") - 1, 1);
	crex_string *property_firstElementChild_class_DOMElement = crex_string_init("DOMElement", sizeof("DOMElement")-1, 1);
	crex_declare_typed_property(class_entry, property_firstElementChild_name, &property_firstElementChild_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_firstElementChild_class_DOMElement, 0, MAY_BE_NULL));
	crex_string_release(property_firstElementChild_name);

	zval property_lastElementChild_default_value;
	ZVAL_UNDEF(&property_lastElementChild_default_value);
	crex_string *property_lastElementChild_name = crex_string_init("lastElementChild", sizeof("lastElementChild") - 1, 1);
	crex_string *property_lastElementChild_class_DOMElement = crex_string_init("DOMElement", sizeof("DOMElement")-1, 1);
	crex_declare_typed_property(class_entry, property_lastElementChild_name, &property_lastElementChild_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_lastElementChild_class_DOMElement, 0, MAY_BE_NULL));
	crex_string_release(property_lastElementChild_name);

	zval property_childElementCount_default_value;
	ZVAL_UNDEF(&property_childElementCount_default_value);
	crex_string *property_childElementCount_name = crex_string_init("childElementCount", sizeof("childElementCount") - 1, 1);
	crex_declare_typed_property(class_entry, property_childElementCount_name, &property_childElementCount_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_childElementCount_name);

	zval property_previousElementSibling_default_value;
	ZVAL_UNDEF(&property_previousElementSibling_default_value);
	crex_string *property_previousElementSibling_name = crex_string_init("previousElementSibling", sizeof("previousElementSibling") - 1, 1);
	crex_string *property_previousElementSibling_class_DOMElement = crex_string_init("DOMElement", sizeof("DOMElement")-1, 1);
	crex_declare_typed_property(class_entry, property_previousElementSibling_name, &property_previousElementSibling_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_previousElementSibling_class_DOMElement, 0, MAY_BE_NULL));
	crex_string_release(property_previousElementSibling_name);

	zval property_nextElementSibling_default_value;
	ZVAL_UNDEF(&property_nextElementSibling_default_value);
	crex_string *property_nextElementSibling_name = crex_string_init("nextElementSibling", sizeof("nextElementSibling") - 1, 1);
	crex_string *property_nextElementSibling_class_DOMElement = crex_string_init("DOMElement", sizeof("DOMElement")-1, 1);
	crex_declare_typed_property(class_entry, property_nextElementSibling_name, &property_nextElementSibling_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_nextElementSibling_class_DOMElement, 0, MAY_BE_NULL));
	crex_string_release(property_nextElementSibling_name);

	return class_entry;
}

static crex_class_entry *register_class_DOMDocument(crex_class_entry *class_entry_DOMNode, crex_class_entry *class_entry_DOMParentNode)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMDocument", class_DOMDocument_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_DOMNode);
	crex_class_implements(class_entry, 1, class_entry_DOMParentNode);

	zval property_doctype_default_value;
	ZVAL_UNDEF(&property_doctype_default_value);
	crex_string *property_doctype_name = crex_string_init("doctype", sizeof("doctype") - 1, 1);
	crex_string *property_doctype_class_DOMDocumentType = crex_string_init("DOMDocumentType", sizeof("DOMDocumentType")-1, 1);
	crex_declare_typed_property(class_entry, property_doctype_name, &property_doctype_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_doctype_class_DOMDocumentType, 0, MAY_BE_NULL));
	crex_string_release(property_doctype_name);

	zval property_implementation_default_value;
	ZVAL_UNDEF(&property_implementation_default_value);
	crex_string *property_implementation_name = crex_string_init("implementation", sizeof("implementation") - 1, 1);
	crex_string *property_implementation_class_DOMImplementation = crex_string_init("DOMImplementation", sizeof("DOMImplementation")-1, 1);
	crex_declare_typed_property(class_entry, property_implementation_name, &property_implementation_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_implementation_class_DOMImplementation, 0, 0));
	crex_string_release(property_implementation_name);

	zval property_documentElement_default_value;
	ZVAL_UNDEF(&property_documentElement_default_value);
	crex_string *property_documentElement_name = crex_string_init("documentElement", sizeof("documentElement") - 1, 1);
	crex_string *property_documentElement_class_DOMElement = crex_string_init("DOMElement", sizeof("DOMElement")-1, 1);
	crex_declare_typed_property(class_entry, property_documentElement_name, &property_documentElement_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_documentElement_class_DOMElement, 0, MAY_BE_NULL));
	crex_string_release(property_documentElement_name);

	zval property_actualEncoding_default_value;
	ZVAL_UNDEF(&property_actualEncoding_default_value);
	crex_string *property_actualEncoding_name = crex_string_init("actualEncoding", sizeof("actualEncoding") - 1, 1);
	crex_declare_typed_property(class_entry, property_actualEncoding_name, &property_actualEncoding_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_actualEncoding_name);

	zval property_encoding_default_value;
	ZVAL_UNDEF(&property_encoding_default_value);
	crex_string *property_encoding_name = crex_string_init("encoding", sizeof("encoding") - 1, 1);
	crex_declare_typed_property(class_entry, property_encoding_name, &property_encoding_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_encoding_name);

	zval property_xmlEncoding_default_value;
	ZVAL_UNDEF(&property_xmlEncoding_default_value);
	crex_string *property_xmlEncoding_name = crex_string_init("xmlEncoding", sizeof("xmlEncoding") - 1, 1);
	crex_declare_typed_property(class_entry, property_xmlEncoding_name, &property_xmlEncoding_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_xmlEncoding_name);

	zval property_standalone_default_value;
	ZVAL_UNDEF(&property_standalone_default_value);
	crex_string *property_standalone_name = crex_string_init("standalone", sizeof("standalone") - 1, 1);
	crex_declare_typed_property(class_entry, property_standalone_name, &property_standalone_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property_standalone_name);

	zval property_xmlStandalone_default_value;
	ZVAL_UNDEF(&property_xmlStandalone_default_value);
	crex_string *property_xmlStandalone_name = crex_string_init("xmlStandalone", sizeof("xmlStandalone") - 1, 1);
	crex_declare_typed_property(class_entry, property_xmlStandalone_name, &property_xmlStandalone_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property_xmlStandalone_name);

	zval property_version_default_value;
	ZVAL_UNDEF(&property_version_default_value);
	crex_string *property_version_name = crex_string_init("version", sizeof("version") - 1, 1);
	crex_declare_typed_property(class_entry, property_version_name, &property_version_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_version_name);

	zval property_xmlVersion_default_value;
	ZVAL_UNDEF(&property_xmlVersion_default_value);
	crex_string *property_xmlVersion_name = crex_string_init("xmlVersion", sizeof("xmlVersion") - 1, 1);
	crex_declare_typed_property(class_entry, property_xmlVersion_name, &property_xmlVersion_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_xmlVersion_name);

	zval property_strictErrorChecking_default_value;
	ZVAL_UNDEF(&property_strictErrorChecking_default_value);
	crex_string *property_strictErrorChecking_name = crex_string_init("strictErrorChecking", sizeof("strictErrorChecking") - 1, 1);
	crex_declare_typed_property(class_entry, property_strictErrorChecking_name, &property_strictErrorChecking_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property_strictErrorChecking_name);

	zval property_documentURI_default_value;
	ZVAL_UNDEF(&property_documentURI_default_value);
	crex_string *property_documentURI_name = crex_string_init("documentURI", sizeof("documentURI") - 1, 1);
	crex_declare_typed_property(class_entry, property_documentURI_name, &property_documentURI_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_documentURI_name);

	zval property_config_default_value;
	ZVAL_UNDEF(&property_config_default_value);
	crex_string *property_config_name = crex_string_init("config", sizeof("config") - 1, 1);
	crex_declare_typed_property(class_entry, property_config_name, &property_config_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_ANY));
	crex_string_release(property_config_name);

	zval property_formatOutput_default_value;
	ZVAL_UNDEF(&property_formatOutput_default_value);
	crex_string *property_formatOutput_name = crex_string_init("formatOutput", sizeof("formatOutput") - 1, 1);
	crex_declare_typed_property(class_entry, property_formatOutput_name, &property_formatOutput_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property_formatOutput_name);

	zval property_validateOnParse_default_value;
	ZVAL_UNDEF(&property_validateOnParse_default_value);
	crex_string *property_validateOnParse_name = crex_string_init("validateOnParse", sizeof("validateOnParse") - 1, 1);
	crex_declare_typed_property(class_entry, property_validateOnParse_name, &property_validateOnParse_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property_validateOnParse_name);

	zval property_resolveExternals_default_value;
	ZVAL_UNDEF(&property_resolveExternals_default_value);
	crex_string *property_resolveExternals_name = crex_string_init("resolveExternals", sizeof("resolveExternals") - 1, 1);
	crex_declare_typed_property(class_entry, property_resolveExternals_name, &property_resolveExternals_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property_resolveExternals_name);

	zval property_preserveWhiteSpace_default_value;
	ZVAL_UNDEF(&property_preserveWhiteSpace_default_value);
	crex_string *property_preserveWhiteSpace_name = crex_string_init("preserveWhiteSpace", sizeof("preserveWhiteSpace") - 1, 1);
	crex_declare_typed_property(class_entry, property_preserveWhiteSpace_name, &property_preserveWhiteSpace_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property_preserveWhiteSpace_name);

	zval property_recover_default_value;
	ZVAL_UNDEF(&property_recover_default_value);
	crex_string *property_recover_name = crex_string_init("recover", sizeof("recover") - 1, 1);
	crex_declare_typed_property(class_entry, property_recover_name, &property_recover_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property_recover_name);

	zval property_substituteEntities_default_value;
	ZVAL_UNDEF(&property_substituteEntities_default_value);
	crex_string *property_substituteEntities_name = crex_string_init("substituteEntities", sizeof("substituteEntities") - 1, 1);
	crex_declare_typed_property(class_entry, property_substituteEntities_name, &property_substituteEntities_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property_substituteEntities_name);

	zval property_firstElementChild_default_value;
	ZVAL_UNDEF(&property_firstElementChild_default_value);
	crex_string *property_firstElementChild_name = crex_string_init("firstElementChild", sizeof("firstElementChild") - 1, 1);
	crex_string *property_firstElementChild_class_DOMElement = crex_string_init("DOMElement", sizeof("DOMElement")-1, 1);
	crex_declare_typed_property(class_entry, property_firstElementChild_name, &property_firstElementChild_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_firstElementChild_class_DOMElement, 0, MAY_BE_NULL));
	crex_string_release(property_firstElementChild_name);

	zval property_lastElementChild_default_value;
	ZVAL_UNDEF(&property_lastElementChild_default_value);
	crex_string *property_lastElementChild_name = crex_string_init("lastElementChild", sizeof("lastElementChild") - 1, 1);
	crex_string *property_lastElementChild_class_DOMElement = crex_string_init("DOMElement", sizeof("DOMElement")-1, 1);
	crex_declare_typed_property(class_entry, property_lastElementChild_name, &property_lastElementChild_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_lastElementChild_class_DOMElement, 0, MAY_BE_NULL));
	crex_string_release(property_lastElementChild_name);

	zval property_childElementCount_default_value;
	ZVAL_UNDEF(&property_childElementCount_default_value);
	crex_string *property_childElementCount_name = crex_string_init("childElementCount", sizeof("childElementCount") - 1, 1);
	crex_declare_typed_property(class_entry, property_childElementCount_name, &property_childElementCount_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_childElementCount_name);

	return class_entry;
}

static crex_class_entry *register_class_DOMException(crex_class_entry *class_entry_Exception)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMException", class_DOMException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Exception);
	class_entry->ce_flags |= CREX_ACC_FINAL;

	zval property_code_default_value;
	ZVAL_LONG(&property_code_default_value, 0);
	crex_string *property_code_name = crex_string_init("code", sizeof("code") - 1, 1);
	crex_declare_typed_property(class_entry, property_code_name, &property_code_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_NONE(0));
	crex_string_release(property_code_name);

	return class_entry;
}

static crex_class_entry *register_class_DOMText(crex_class_entry *class_entry_DOMCharacterData)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMText", class_DOMText_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_DOMCharacterData);

	zval property_wholeText_default_value;
	ZVAL_UNDEF(&property_wholeText_default_value);
	crex_string *property_wholeText_name = crex_string_init("wholeText", sizeof("wholeText") - 1, 1);
	crex_declare_typed_property(class_entry, property_wholeText_name, &property_wholeText_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_wholeText_name);

	return class_entry;
}

static crex_class_entry *register_class_DOMNamedNodeMap(crex_class_entry *class_entry_IteratorAggregate, crex_class_entry *class_entry_Countable)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMNamedNodeMap", class_DOMNamedNodeMap_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	crex_class_implements(class_entry, 2, class_entry_IteratorAggregate, class_entry_Countable);

	zval property_length_default_value;
	ZVAL_UNDEF(&property_length_default_value);
	crex_string *property_length_name = crex_string_init("length", sizeof("length") - 1, 1);
	crex_declare_typed_property(class_entry, property_length_name, &property_length_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_length_name);

	return class_entry;
}

static crex_class_entry *register_class_DOMEntity(crex_class_entry *class_entry_DOMNode)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMEntity", class_DOMEntity_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_DOMNode);

	zval property_publicId_default_value;
	ZVAL_UNDEF(&property_publicId_default_value);
	crex_string *property_publicId_name = crex_string_init("publicId", sizeof("publicId") - 1, 1);
	crex_declare_typed_property(class_entry, property_publicId_name, &property_publicId_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_publicId_name);

	zval property_systemId_default_value;
	ZVAL_UNDEF(&property_systemId_default_value);
	crex_string *property_systemId_name = crex_string_init("systemId", sizeof("systemId") - 1, 1);
	crex_declare_typed_property(class_entry, property_systemId_name, &property_systemId_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_systemId_name);

	zval property_notationName_default_value;
	ZVAL_UNDEF(&property_notationName_default_value);
	crex_string *property_notationName_name = crex_string_init("notationName", sizeof("notationName") - 1, 1);
	crex_declare_typed_property(class_entry, property_notationName_name, &property_notationName_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_notationName_name);

	zval property_actualEncoding_default_value;
	ZVAL_NULL(&property_actualEncoding_default_value);
	crex_string *property_actualEncoding_name = crex_string_init("actualEncoding", sizeof("actualEncoding") - 1, 1);
	crex_declare_typed_property(class_entry, property_actualEncoding_name, &property_actualEncoding_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_actualEncoding_name);

	zval property_encoding_default_value;
	ZVAL_NULL(&property_encoding_default_value);
	crex_string *property_encoding_name = crex_string_init("encoding", sizeof("encoding") - 1, 1);
	crex_declare_typed_property(class_entry, property_encoding_name, &property_encoding_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_encoding_name);

	zval property_version_default_value;
	ZVAL_NULL(&property_version_default_value);
	crex_string *property_version_name = crex_string_init("version", sizeof("version") - 1, 1);
	crex_declare_typed_property(class_entry, property_version_name, &property_version_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_version_name);

	return class_entry;
}

static crex_class_entry *register_class_DOMEntityReference(crex_class_entry *class_entry_DOMNode)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMEntityReference", class_DOMEntityReference_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_DOMNode);

	return class_entry;
}

static crex_class_entry *register_class_DOMNotation(crex_class_entry *class_entry_DOMNode)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMNotation", class_DOMNotation_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_DOMNode);

	zval property_publicId_default_value;
	ZVAL_UNDEF(&property_publicId_default_value);
	crex_string *property_publicId_name = crex_string_init("publicId", sizeof("publicId") - 1, 1);
	crex_declare_typed_property(class_entry, property_publicId_name, &property_publicId_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_publicId_name);

	zval property_systemId_default_value;
	ZVAL_UNDEF(&property_systemId_default_value);
	crex_string *property_systemId_name = crex_string_init("systemId", sizeof("systemId") - 1, 1);
	crex_declare_typed_property(class_entry, property_systemId_name, &property_systemId_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_systemId_name);

	return class_entry;
}

static crex_class_entry *register_class_DOMProcessingInstruction(crex_class_entry *class_entry_DOMNode)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMProcessingInstruction", class_DOMProcessingInstruction_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_DOMNode);

	zval property_target_default_value;
	ZVAL_UNDEF(&property_target_default_value);
	crex_string *property_target_name = crex_string_init("target", sizeof("target") - 1, 1);
	crex_declare_typed_property(class_entry, property_target_name, &property_target_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_target_name);

	zval property_data_default_value;
	ZVAL_UNDEF(&property_data_default_value);
	crex_string *property_data_name = crex_string_init("data", sizeof("data") - 1, 1);
	crex_declare_typed_property(class_entry, property_data_name, &property_data_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_data_name);

	return class_entry;
}

#if defined(LIBXML_XPATH_ENABLED)
static crex_class_entry *register_class_DOMXPath(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DOMXPath", class_DOMXPath_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;

	zval property_document_default_value;
	ZVAL_UNDEF(&property_document_default_value);
	crex_string *property_document_name = crex_string_init("document", sizeof("document") - 1, 1);
	crex_string *property_document_class_DOMDocument = crex_string_init("DOMDocument", sizeof("DOMDocument")-1, 1);
	crex_declare_typed_property(class_entry, property_document_name, &property_document_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_document_class_DOMDocument, 0, 0));
	crex_string_release(property_document_name);

	zval property_registerNodeNamespaces_default_value;
	ZVAL_UNDEF(&property_registerNodeNamespaces_default_value);
	crex_string *property_registerNodeNamespaces_name = crex_string_init("registerNodeNamespaces", sizeof("registerNodeNamespaces") - 1, 1);
	crex_declare_typed_property(class_entry, property_registerNodeNamespaces_name, &property_registerNodeNamespaces_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property_registerNodeNamespaces_name);

	return class_entry;
}
#endif
