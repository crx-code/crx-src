/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 4751b68b857ffbf53cab6d1aa88fe8f6120d4fc6 */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_XMLReader_close, 0, 0, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLReader_getAttribute, 0, 1, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLReader_getAttributeNo, 0, 1, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLReader_getAttributeNs, 0, 2, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLReader_getParserProperty, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, property, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLReader_isValid, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLReader_lookupNamespace, 0, 1, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, prefix, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLReader_moveToAttribute, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLReader_moveToAttributeNo, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLReader_moveToAttributeNs, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_XMLReader_moveToElement arginfo_class_XMLReader_isValid

#define arginfo_class_XMLReader_moveToFirstAttribute arginfo_class_XMLReader_isValid

#define arginfo_class_XMLReader_moveToNextAttribute arginfo_class_XMLReader_isValid

#define arginfo_class_XMLReader_read arginfo_class_XMLReader_isValid

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLReader_next, 0, 0, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, name, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_XMLReader_open, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, uri, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLReader_readInnerXml, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_XMLReader_readOuterXml arginfo_class_XMLReader_readInnerXml

#define arginfo_class_XMLReader_readString arginfo_class_XMLReader_readInnerXml

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLReader_setSchema, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLReader_setParserProperty, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, property, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, _IS_BOOL, 0)
CREX_END_ARG_INFO()

#define arginfo_class_XMLReader_setRelaxNGSchema arginfo_class_XMLReader_setSchema

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLReader_setRelaxNGSchemaSource, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, source, IS_STRING, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_XMLReader_XML, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, source, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_XMLReader_expand, 0, 0, DOMNode, MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, baseNode, DOMNode, 1, "null")
CREX_END_ARG_INFO()


CREX_METHOD(XMLReader, close);
CREX_METHOD(XMLReader, getAttribute);
CREX_METHOD(XMLReader, getAttributeNo);
CREX_METHOD(XMLReader, getAttributeNs);
CREX_METHOD(XMLReader, getParserProperty);
CREX_METHOD(XMLReader, isValid);
CREX_METHOD(XMLReader, lookupNamespace);
CREX_METHOD(XMLReader, moveToAttribute);
CREX_METHOD(XMLReader, moveToAttributeNo);
CREX_METHOD(XMLReader, moveToAttributeNs);
CREX_METHOD(XMLReader, moveToElement);
CREX_METHOD(XMLReader, moveToFirstAttribute);
CREX_METHOD(XMLReader, moveToNextAttribute);
CREX_METHOD(XMLReader, read);
CREX_METHOD(XMLReader, next);
CREX_METHOD(XMLReader, open);
CREX_METHOD(XMLReader, readInnerXml);
CREX_METHOD(XMLReader, readOuterXml);
CREX_METHOD(XMLReader, readString);
CREX_METHOD(XMLReader, setSchema);
CREX_METHOD(XMLReader, setParserProperty);
CREX_METHOD(XMLReader, setRelaxNGSchema);
CREX_METHOD(XMLReader, setRelaxNGSchemaSource);
CREX_METHOD(XMLReader, XML);
CREX_METHOD(XMLReader, expand);


static const crex_function_entry class_XMLReader_methods[] = {
	CREX_ME(XMLReader, close, arginfo_class_XMLReader_close, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, getAttribute, arginfo_class_XMLReader_getAttribute, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, getAttributeNo, arginfo_class_XMLReader_getAttributeNo, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, getAttributeNs, arginfo_class_XMLReader_getAttributeNs, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, getParserProperty, arginfo_class_XMLReader_getParserProperty, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, isValid, arginfo_class_XMLReader_isValid, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, lookupNamespace, arginfo_class_XMLReader_lookupNamespace, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, moveToAttribute, arginfo_class_XMLReader_moveToAttribute, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, moveToAttributeNo, arginfo_class_XMLReader_moveToAttributeNo, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, moveToAttributeNs, arginfo_class_XMLReader_moveToAttributeNs, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, moveToElement, arginfo_class_XMLReader_moveToElement, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, moveToFirstAttribute, arginfo_class_XMLReader_moveToFirstAttribute, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, moveToNextAttribute, arginfo_class_XMLReader_moveToNextAttribute, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, read, arginfo_class_XMLReader_read, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, next, arginfo_class_XMLReader_next, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, open, arginfo_class_XMLReader_open, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(XMLReader, readInnerXml, arginfo_class_XMLReader_readInnerXml, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, readOuterXml, arginfo_class_XMLReader_readOuterXml, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, readString, arginfo_class_XMLReader_readString, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, setSchema, arginfo_class_XMLReader_setSchema, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, setParserProperty, arginfo_class_XMLReader_setParserProperty, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, setRelaxNGSchema, arginfo_class_XMLReader_setRelaxNGSchema, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, setRelaxNGSchemaSource, arginfo_class_XMLReader_setRelaxNGSchemaSource, CREX_ACC_PUBLIC)
	CREX_ME(XMLReader, XML, arginfo_class_XMLReader_XML, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(XMLReader, expand, arginfo_class_XMLReader_expand, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_XMLReader(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "XMLReader", class_XMLReader_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	zval const_NONE_value;
	ZVAL_LONG(&const_NONE_value, XML_READER_TYPE_NONE);
	crex_string *const_NONE_name = crex_string_init_interned("NONE", sizeof("NONE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NONE_name, &const_NONE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NONE_name);

	zval const_ELEMENT_value;
	ZVAL_LONG(&const_ELEMENT_value, XML_READER_TYPE_ELEMENT);
	crex_string *const_ELEMENT_name = crex_string_init_interned("ELEMENT", sizeof("ELEMENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ELEMENT_name, &const_ELEMENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ELEMENT_name);

	zval const_ATTRIBUTE_value;
	ZVAL_LONG(&const_ATTRIBUTE_value, XML_READER_TYPE_ATTRIBUTE);
	crex_string *const_ATTRIBUTE_name = crex_string_init_interned("ATTRIBUTE", sizeof("ATTRIBUTE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTRIBUTE_name, &const_ATTRIBUTE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTRIBUTE_name);

	zval const_TEXT_value;
	ZVAL_LONG(&const_TEXT_value, XML_READER_TYPE_TEXT);
	crex_string *const_TEXT_name = crex_string_init_interned("TEXT", sizeof("TEXT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_TEXT_name, &const_TEXT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_TEXT_name);

	zval const_CDATA_value;
	ZVAL_LONG(&const_CDATA_value, XML_READER_TYPE_CDATA);
	crex_string *const_CDATA_name = crex_string_init_interned("CDATA", sizeof("CDATA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CDATA_name, &const_CDATA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CDATA_name);

	zval const_ENTITY_REF_value;
	ZVAL_LONG(&const_ENTITY_REF_value, XML_READER_TYPE_ENTITY_REFERENCE);
	crex_string *const_ENTITY_REF_name = crex_string_init_interned("ENTITY_REF", sizeof("ENTITY_REF") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ENTITY_REF_name, &const_ENTITY_REF_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ENTITY_REF_name);

	zval const_ENTITY_value;
	ZVAL_LONG(&const_ENTITY_value, XML_READER_TYPE_ENTITY);
	crex_string *const_ENTITY_name = crex_string_init_interned("ENTITY", sizeof("ENTITY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ENTITY_name, &const_ENTITY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ENTITY_name);

	zval const_PI_value;
	ZVAL_LONG(&const_PI_value, XML_READER_TYPE_PROCESSING_INSTRUCTION);
	crex_string *const_PI_name = crex_string_init_interned("PI", sizeof("PI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PI_name, &const_PI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PI_name);

	zval const_COMMENT_value;
	ZVAL_LONG(&const_COMMENT_value, XML_READER_TYPE_COMMENT);
	crex_string *const_COMMENT_name = crex_string_init_interned("COMMENT", sizeof("COMMENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_COMMENT_name, &const_COMMENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_COMMENT_name);

	zval const_DOC_value;
	ZVAL_LONG(&const_DOC_value, XML_READER_TYPE_DOCUMENT);
	crex_string *const_DOC_name = crex_string_init_interned("DOC", sizeof("DOC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DOC_name, &const_DOC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DOC_name);

	zval const_DOC_TYPE_value;
	ZVAL_LONG(&const_DOC_TYPE_value, XML_READER_TYPE_DOCUMENT_TYPE);
	crex_string *const_DOC_TYPE_name = crex_string_init_interned("DOC_TYPE", sizeof("DOC_TYPE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DOC_TYPE_name, &const_DOC_TYPE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DOC_TYPE_name);

	zval const_DOC_FRAGMENT_value;
	ZVAL_LONG(&const_DOC_FRAGMENT_value, XML_READER_TYPE_DOCUMENT_FRAGMENT);
	crex_string *const_DOC_FRAGMENT_name = crex_string_init_interned("DOC_FRAGMENT", sizeof("DOC_FRAGMENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DOC_FRAGMENT_name, &const_DOC_FRAGMENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DOC_FRAGMENT_name);

	zval const_NOTATION_value;
	ZVAL_LONG(&const_NOTATION_value, XML_READER_TYPE_NOTATION);
	crex_string *const_NOTATION_name = crex_string_init_interned("NOTATION", sizeof("NOTATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NOTATION_name, &const_NOTATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NOTATION_name);

	zval const_WHITESPACE_value;
	ZVAL_LONG(&const_WHITESPACE_value, XML_READER_TYPE_WHITESPACE);
	crex_string *const_WHITESPACE_name = crex_string_init_interned("WHITESPACE", sizeof("WHITESPACE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WHITESPACE_name, &const_WHITESPACE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WHITESPACE_name);

	zval const_SIGNIFICANT_WHITESPACE_value;
	ZVAL_LONG(&const_SIGNIFICANT_WHITESPACE_value, XML_READER_TYPE_SIGNIFICANT_WHITESPACE);
	crex_string *const_SIGNIFICANT_WHITESPACE_name = crex_string_init_interned("SIGNIFICANT_WHITESPACE", sizeof("SIGNIFICANT_WHITESPACE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SIGNIFICANT_WHITESPACE_name, &const_SIGNIFICANT_WHITESPACE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SIGNIFICANT_WHITESPACE_name);

	zval const_END_ELEMENT_value;
	ZVAL_LONG(&const_END_ELEMENT_value, XML_READER_TYPE_END_ELEMENT);
	crex_string *const_END_ELEMENT_name = crex_string_init_interned("END_ELEMENT", sizeof("END_ELEMENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_END_ELEMENT_name, &const_END_ELEMENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_END_ELEMENT_name);

	zval const_END_ENTITY_value;
	ZVAL_LONG(&const_END_ENTITY_value, XML_READER_TYPE_END_ENTITY);
	crex_string *const_END_ENTITY_name = crex_string_init_interned("END_ENTITY", sizeof("END_ENTITY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_END_ENTITY_name, &const_END_ENTITY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_END_ENTITY_name);

	zval const_XML_DECLARATION_value;
	ZVAL_LONG(&const_XML_DECLARATION_value, XML_READER_TYPE_XML_DECLARATION);
	crex_string *const_XML_DECLARATION_name = crex_string_init_interned("XML_DECLARATION", sizeof("XML_DECLARATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_XML_DECLARATION_name, &const_XML_DECLARATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_XML_DECLARATION_name);

	zval const_LOADDTD_value;
	ZVAL_LONG(&const_LOADDTD_value, XML_PARSER_LOADDTD);
	crex_string *const_LOADDTD_name = crex_string_init_interned("LOADDTD", sizeof("LOADDTD") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LOADDTD_name, &const_LOADDTD_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LOADDTD_name);

	zval const_DEFAULTATTRS_value;
	ZVAL_LONG(&const_DEFAULTATTRS_value, XML_PARSER_DEFAULTATTRS);
	crex_string *const_DEFAULTATTRS_name = crex_string_init_interned("DEFAULTATTRS", sizeof("DEFAULTATTRS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DEFAULTATTRS_name, &const_DEFAULTATTRS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DEFAULTATTRS_name);

	zval const_VALIDATE_value;
	ZVAL_LONG(&const_VALIDATE_value, XML_PARSER_VALIDATE);
	crex_string *const_VALIDATE_name = crex_string_init_interned("VALIDATE", sizeof("VALIDATE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_VALIDATE_name, &const_VALIDATE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_VALIDATE_name);

	zval const_SUBST_ENTITIES_value;
	ZVAL_LONG(&const_SUBST_ENTITIES_value, XML_PARSER_SUBST_ENTITIES);
	crex_string *const_SUBST_ENTITIES_name = crex_string_init_interned("SUBST_ENTITIES", sizeof("SUBST_ENTITIES") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SUBST_ENTITIES_name, &const_SUBST_ENTITIES_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SUBST_ENTITIES_name);

	zval property_attributeCount_default_value;
	ZVAL_UNDEF(&property_attributeCount_default_value);
	crex_string *property_attributeCount_name = crex_string_init("attributeCount", sizeof("attributeCount") - 1, 1);
	crex_declare_typed_property(class_entry, property_attributeCount_name, &property_attributeCount_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_attributeCount_name);

	zval property_baseURI_default_value;
	ZVAL_UNDEF(&property_baseURI_default_value);
	crex_string *property_baseURI_name = crex_string_init("baseURI", sizeof("baseURI") - 1, 1);
	crex_declare_typed_property(class_entry, property_baseURI_name, &property_baseURI_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_baseURI_name);

	zval property_depth_default_value;
	ZVAL_UNDEF(&property_depth_default_value);
	crex_string *property_depth_name = crex_string_init("depth", sizeof("depth") - 1, 1);
	crex_declare_typed_property(class_entry, property_depth_name, &property_depth_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_depth_name);

	zval property_hasAttributes_default_value;
	ZVAL_UNDEF(&property_hasAttributes_default_value);
	crex_string *property_hasAttributes_name = crex_string_init("hasAttributes", sizeof("hasAttributes") - 1, 1);
	crex_declare_typed_property(class_entry, property_hasAttributes_name, &property_hasAttributes_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property_hasAttributes_name);

	zval property_hasValue_default_value;
	ZVAL_UNDEF(&property_hasValue_default_value);
	crex_string *property_hasValue_name = crex_string_init("hasValue", sizeof("hasValue") - 1, 1);
	crex_declare_typed_property(class_entry, property_hasValue_name, &property_hasValue_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property_hasValue_name);

	zval property_isDefault_default_value;
	ZVAL_UNDEF(&property_isDefault_default_value);
	crex_string *property_isDefault_name = crex_string_init("isDefault", sizeof("isDefault") - 1, 1);
	crex_declare_typed_property(class_entry, property_isDefault_name, &property_isDefault_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property_isDefault_name);

	zval property_isEmptyElement_default_value;
	ZVAL_UNDEF(&property_isEmptyElement_default_value);
	crex_string *property_isEmptyElement_name = crex_string_init("isEmptyElement", sizeof("isEmptyElement") - 1, 1);
	crex_declare_typed_property(class_entry, property_isEmptyElement_name, &property_isEmptyElement_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property_isEmptyElement_name);

	zval property_localName_default_value;
	ZVAL_UNDEF(&property_localName_default_value);
	crex_string *property_localName_name = crex_string_init("localName", sizeof("localName") - 1, 1);
	crex_declare_typed_property(class_entry, property_localName_name, &property_localName_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_localName_name);

	zval property_name_default_value;
	ZVAL_UNDEF(&property_name_default_value);
	crex_string *property_name_name = crex_string_init("name", sizeof("name") - 1, 1);
	crex_declare_typed_property(class_entry, property_name_name, &property_name_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_name_name);

	zval property_namespaceURI_default_value;
	ZVAL_UNDEF(&property_namespaceURI_default_value);
	crex_string *property_namespaceURI_name = crex_string_init("namespaceURI", sizeof("namespaceURI") - 1, 1);
	crex_declare_typed_property(class_entry, property_namespaceURI_name, &property_namespaceURI_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_namespaceURI_name);

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

	zval property_value_default_value;
	ZVAL_UNDEF(&property_value_default_value);
	crex_string *property_value_name = crex_string_init("value", sizeof("value") - 1, 1);
	crex_declare_typed_property(class_entry, property_value_name, &property_value_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_value_name);

	zval property_xmlLang_default_value;
	ZVAL_UNDEF(&property_xmlLang_default_value);
	crex_string *property_xmlLang_name = crex_string_init("xmlLang", sizeof("xmlLang") - 1, 1);
	crex_declare_typed_property(class_entry, property_xmlLang_name, &property_xmlLang_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_xmlLang_name);

	return class_entry;
}
