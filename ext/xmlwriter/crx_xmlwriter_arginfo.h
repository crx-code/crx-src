/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 820ad2d68166b189b9163c2c3dfcc76806d41b7d */

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_xmlwriter_open_uri, 0, 1, XMLWriter, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, uri, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_xmlwriter_open_memory, 0, 0, XMLWriter, MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xmlwriter_set_indent, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, writer, XMLWriter, 0)
	CREX_ARG_TYPE_INFO(0, enable, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xmlwriter_set_indent_string, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, writer, XMLWriter, 0)
	CREX_ARG_TYPE_INFO(0, indentation, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xmlwriter_start_comment, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, writer, XMLWriter, 0)
CREX_END_ARG_INFO()

#define arginfo_xmlwriter_end_comment arginfo_xmlwriter_start_comment

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xmlwriter_start_attribute, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, writer, XMLWriter, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_xmlwriter_end_attribute arginfo_xmlwriter_start_comment

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xmlwriter_write_attribute, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, writer, XMLWriter, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xmlwriter_start_attribute_ns, 0, 4, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, writer, XMLWriter, 0)
	CREX_ARG_TYPE_INFO(0, prefix, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xmlwriter_write_attribute_ns, 0, 5, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, writer, XMLWriter, 0)
	CREX_ARG_TYPE_INFO(0, prefix, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, value, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_xmlwriter_start_element arginfo_xmlwriter_start_attribute

#define arginfo_xmlwriter_end_element arginfo_xmlwriter_start_comment

#define arginfo_xmlwriter_full_end_element arginfo_xmlwriter_start_comment

#define arginfo_xmlwriter_start_element_ns arginfo_xmlwriter_start_attribute_ns

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xmlwriter_write_element, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, writer, XMLWriter, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, content, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xmlwriter_write_element_ns, 0, 4, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, writer, XMLWriter, 0)
	CREX_ARG_TYPE_INFO(0, prefix, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, content, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xmlwriter_start_pi, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, writer, XMLWriter, 0)
	CREX_ARG_TYPE_INFO(0, target, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_xmlwriter_end_pi arginfo_xmlwriter_start_comment

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xmlwriter_write_pi, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, writer, XMLWriter, 0)
	CREX_ARG_TYPE_INFO(0, target, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, content, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_xmlwriter_start_cdata arginfo_xmlwriter_start_comment

#define arginfo_xmlwriter_end_cdata arginfo_xmlwriter_start_comment

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xmlwriter_write_cdata, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, writer, XMLWriter, 0)
	CREX_ARG_TYPE_INFO(0, content, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_xmlwriter_text arginfo_xmlwriter_write_cdata

#define arginfo_xmlwriter_write_raw arginfo_xmlwriter_write_cdata

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xmlwriter_start_document, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, writer, XMLWriter, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, version, IS_STRING, 1, "\"1.0\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, standalone, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_xmlwriter_end_document arginfo_xmlwriter_start_comment

#define arginfo_xmlwriter_write_comment arginfo_xmlwriter_write_cdata

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xmlwriter_start_dtd, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, writer, XMLWriter, 0)
	CREX_ARG_TYPE_INFO(0, qualifiedName, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, publicId, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, systemId, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_xmlwriter_end_dtd arginfo_xmlwriter_start_comment

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xmlwriter_write_dtd, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, writer, XMLWriter, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, publicId, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, systemId, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, content, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xmlwriter_start_dtd_element, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, writer, XMLWriter, 0)
	CREX_ARG_TYPE_INFO(0, qualifiedName, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_xmlwriter_end_dtd_element arginfo_xmlwriter_start_comment

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xmlwriter_write_dtd_element, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, writer, XMLWriter, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, content, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_xmlwriter_start_dtd_attlist arginfo_xmlwriter_start_attribute

#define arginfo_xmlwriter_end_dtd_attlist arginfo_xmlwriter_start_comment

#define arginfo_xmlwriter_write_dtd_attlist arginfo_xmlwriter_write_dtd_element

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xmlwriter_start_dtd_entity, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, writer, XMLWriter, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, isParam, _IS_BOOL, 0)
CREX_END_ARG_INFO()

#define arginfo_xmlwriter_end_dtd_entity arginfo_xmlwriter_start_comment

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xmlwriter_write_dtd_entity, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, writer, XMLWriter, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, content, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, isParam, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, publicId, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, systemId, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, notationData, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xmlwriter_output_memory, 0, 1, IS_STRING, 0)
	CREX_ARG_OBJ_INFO(0, writer, XMLWriter, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flush, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_xmlwriter_flush, 0, 1, MAY_BE_STRING|MAY_BE_LONG)
	CREX_ARG_OBJ_INFO(0, writer, XMLWriter, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, empty, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLWriter_openUri, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, uri, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLWriter_openMemory, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLWriter_setIndent, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, enable, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLWriter_setIndentString, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, indentation, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_XMLWriter_startComment arginfo_class_XMLWriter_openMemory

#define arginfo_class_XMLWriter_endComment arginfo_class_XMLWriter_openMemory

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLWriter_startAttribute, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_XMLWriter_endAttribute arginfo_class_XMLWriter_openMemory

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLWriter_writeAttribute, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLWriter_startAttributeNs, 0, 3, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, prefix, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLWriter_writeAttributeNs, 0, 4, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, prefix, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, value, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_XMLWriter_startElement arginfo_class_XMLWriter_startAttribute

#define arginfo_class_XMLWriter_endElement arginfo_class_XMLWriter_openMemory

#define arginfo_class_XMLWriter_fullEndElement arginfo_class_XMLWriter_openMemory

#define arginfo_class_XMLWriter_startElementNs arginfo_class_XMLWriter_startAttributeNs

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLWriter_writeElement, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, content, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLWriter_writeElementNs, 0, 3, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, prefix, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, content, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLWriter_startPi, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, target, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_XMLWriter_endPi arginfo_class_XMLWriter_openMemory

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLWriter_writePi, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, target, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, content, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_XMLWriter_startCdata arginfo_class_XMLWriter_openMemory

#define arginfo_class_XMLWriter_endCdata arginfo_class_XMLWriter_openMemory

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLWriter_writeCdata, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, content, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_XMLWriter_text arginfo_class_XMLWriter_writeCdata

#define arginfo_class_XMLWriter_writeRaw arginfo_class_XMLWriter_writeCdata

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLWriter_startDocument, 0, 0, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, version, IS_STRING, 1, "\"1.0\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, standalone, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_class_XMLWriter_endDocument arginfo_class_XMLWriter_openMemory

#define arginfo_class_XMLWriter_writeComment arginfo_class_XMLWriter_writeCdata

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLWriter_startDtd, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, qualifiedName, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, publicId, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, systemId, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_class_XMLWriter_endDtd arginfo_class_XMLWriter_openMemory

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLWriter_writeDtd, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, publicId, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, systemId, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, content, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLWriter_startDtdElement, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, qualifiedName, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_XMLWriter_endDtdElement arginfo_class_XMLWriter_openMemory

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLWriter_writeDtdElement, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, content, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_XMLWriter_startDtdAttlist arginfo_class_XMLWriter_startAttribute

#define arginfo_class_XMLWriter_endDtdAttlist arginfo_class_XMLWriter_openMemory

#define arginfo_class_XMLWriter_writeDtdAttlist arginfo_class_XMLWriter_writeDtdElement

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLWriter_startDtdEntity, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, isParam, _IS_BOOL, 0)
CREX_END_ARG_INFO()

#define arginfo_class_XMLWriter_endDtdEntity arginfo_class_XMLWriter_openMemory

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLWriter_writeDtdEntity, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, content, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, isParam, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, publicId, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, systemId, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, notationData, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XMLWriter_outputMemory, 0, 0, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flush, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_XMLWriter_flush, 0, 0, MAY_BE_STRING|MAY_BE_LONG)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, empty, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()


CREX_FUNCTION(xmlwriter_open_uri);
CREX_FUNCTION(xmlwriter_open_memory);
CREX_FUNCTION(xmlwriter_set_indent);
CREX_FUNCTION(xmlwriter_set_indent_string);
CREX_FUNCTION(xmlwriter_start_comment);
CREX_FUNCTION(xmlwriter_end_comment);
CREX_FUNCTION(xmlwriter_start_attribute);
CREX_FUNCTION(xmlwriter_end_attribute);
CREX_FUNCTION(xmlwriter_write_attribute);
CREX_FUNCTION(xmlwriter_start_attribute_ns);
CREX_FUNCTION(xmlwriter_write_attribute_ns);
CREX_FUNCTION(xmlwriter_start_element);
CREX_FUNCTION(xmlwriter_end_element);
CREX_FUNCTION(xmlwriter_full_end_element);
CREX_FUNCTION(xmlwriter_start_element_ns);
CREX_FUNCTION(xmlwriter_write_element);
CREX_FUNCTION(xmlwriter_write_element_ns);
CREX_FUNCTION(xmlwriter_start_pi);
CREX_FUNCTION(xmlwriter_end_pi);
CREX_FUNCTION(xmlwriter_write_pi);
CREX_FUNCTION(xmlwriter_start_cdata);
CREX_FUNCTION(xmlwriter_end_cdata);
CREX_FUNCTION(xmlwriter_write_cdata);
CREX_FUNCTION(xmlwriter_text);
CREX_FUNCTION(xmlwriter_write_raw);
CREX_FUNCTION(xmlwriter_start_document);
CREX_FUNCTION(xmlwriter_end_document);
CREX_FUNCTION(xmlwriter_write_comment);
CREX_FUNCTION(xmlwriter_start_dtd);
CREX_FUNCTION(xmlwriter_end_dtd);
CREX_FUNCTION(xmlwriter_write_dtd);
CREX_FUNCTION(xmlwriter_start_dtd_element);
CREX_FUNCTION(xmlwriter_end_dtd_element);
CREX_FUNCTION(xmlwriter_write_dtd_element);
CREX_FUNCTION(xmlwriter_start_dtd_attlist);
CREX_FUNCTION(xmlwriter_end_dtd_attlist);
CREX_FUNCTION(xmlwriter_write_dtd_attlist);
CREX_FUNCTION(xmlwriter_start_dtd_entity);
CREX_FUNCTION(xmlwriter_end_dtd_entity);
CREX_FUNCTION(xmlwriter_write_dtd_entity);
CREX_FUNCTION(xmlwriter_output_memory);
CREX_FUNCTION(xmlwriter_flush);


static const crex_function_entry ext_functions[] = {
	CREX_FE(xmlwriter_open_uri, arginfo_xmlwriter_open_uri)
	CREX_FE(xmlwriter_open_memory, arginfo_xmlwriter_open_memory)
	CREX_FE(xmlwriter_set_indent, arginfo_xmlwriter_set_indent)
	CREX_FE(xmlwriter_set_indent_string, arginfo_xmlwriter_set_indent_string)
	CREX_FE(xmlwriter_start_comment, arginfo_xmlwriter_start_comment)
	CREX_FE(xmlwriter_end_comment, arginfo_xmlwriter_end_comment)
	CREX_FE(xmlwriter_start_attribute, arginfo_xmlwriter_start_attribute)
	CREX_FE(xmlwriter_end_attribute, arginfo_xmlwriter_end_attribute)
	CREX_FE(xmlwriter_write_attribute, arginfo_xmlwriter_write_attribute)
	CREX_FE(xmlwriter_start_attribute_ns, arginfo_xmlwriter_start_attribute_ns)
	CREX_FE(xmlwriter_write_attribute_ns, arginfo_xmlwriter_write_attribute_ns)
	CREX_FE(xmlwriter_start_element, arginfo_xmlwriter_start_element)
	CREX_FE(xmlwriter_end_element, arginfo_xmlwriter_end_element)
	CREX_FE(xmlwriter_full_end_element, arginfo_xmlwriter_full_end_element)
	CREX_FE(xmlwriter_start_element_ns, arginfo_xmlwriter_start_element_ns)
	CREX_FE(xmlwriter_write_element, arginfo_xmlwriter_write_element)
	CREX_FE(xmlwriter_write_element_ns, arginfo_xmlwriter_write_element_ns)
	CREX_FE(xmlwriter_start_pi, arginfo_xmlwriter_start_pi)
	CREX_FE(xmlwriter_end_pi, arginfo_xmlwriter_end_pi)
	CREX_FE(xmlwriter_write_pi, arginfo_xmlwriter_write_pi)
	CREX_FE(xmlwriter_start_cdata, arginfo_xmlwriter_start_cdata)
	CREX_FE(xmlwriter_end_cdata, arginfo_xmlwriter_end_cdata)
	CREX_FE(xmlwriter_write_cdata, arginfo_xmlwriter_write_cdata)
	CREX_FE(xmlwriter_text, arginfo_xmlwriter_text)
	CREX_FE(xmlwriter_write_raw, arginfo_xmlwriter_write_raw)
	CREX_FE(xmlwriter_start_document, arginfo_xmlwriter_start_document)
	CREX_FE(xmlwriter_end_document, arginfo_xmlwriter_end_document)
	CREX_FE(xmlwriter_write_comment, arginfo_xmlwriter_write_comment)
	CREX_FE(xmlwriter_start_dtd, arginfo_xmlwriter_start_dtd)
	CREX_FE(xmlwriter_end_dtd, arginfo_xmlwriter_end_dtd)
	CREX_FE(xmlwriter_write_dtd, arginfo_xmlwriter_write_dtd)
	CREX_FE(xmlwriter_start_dtd_element, arginfo_xmlwriter_start_dtd_element)
	CREX_FE(xmlwriter_end_dtd_element, arginfo_xmlwriter_end_dtd_element)
	CREX_FE(xmlwriter_write_dtd_element, arginfo_xmlwriter_write_dtd_element)
	CREX_FE(xmlwriter_start_dtd_attlist, arginfo_xmlwriter_start_dtd_attlist)
	CREX_FE(xmlwriter_end_dtd_attlist, arginfo_xmlwriter_end_dtd_attlist)
	CREX_FE(xmlwriter_write_dtd_attlist, arginfo_xmlwriter_write_dtd_attlist)
	CREX_FE(xmlwriter_start_dtd_entity, arginfo_xmlwriter_start_dtd_entity)
	CREX_FE(xmlwriter_end_dtd_entity, arginfo_xmlwriter_end_dtd_entity)
	CREX_FE(xmlwriter_write_dtd_entity, arginfo_xmlwriter_write_dtd_entity)
	CREX_FE(xmlwriter_output_memory, arginfo_xmlwriter_output_memory)
	CREX_FE(xmlwriter_flush, arginfo_xmlwriter_flush)
	CREX_FE_END
};


static const crex_function_entry class_XMLWriter_methods[] = {
	CREX_ME_MAPPING(openUri, xmlwriter_open_uri, arginfo_class_XMLWriter_openUri, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(openMemory, xmlwriter_open_memory, arginfo_class_XMLWriter_openMemory, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setIndent, xmlwriter_set_indent, arginfo_class_XMLWriter_setIndent, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setIndentString, xmlwriter_set_indent_string, arginfo_class_XMLWriter_setIndentString, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(startComment, xmlwriter_start_comment, arginfo_class_XMLWriter_startComment, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(endComment, xmlwriter_end_comment, arginfo_class_XMLWriter_endComment, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(startAttribute, xmlwriter_start_attribute, arginfo_class_XMLWriter_startAttribute, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(endAttribute, xmlwriter_end_attribute, arginfo_class_XMLWriter_endAttribute, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(writeAttribute, xmlwriter_write_attribute, arginfo_class_XMLWriter_writeAttribute, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(startAttributeNs, xmlwriter_start_attribute_ns, arginfo_class_XMLWriter_startAttributeNs, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(writeAttributeNs, xmlwriter_write_attribute_ns, arginfo_class_XMLWriter_writeAttributeNs, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(startElement, xmlwriter_start_element, arginfo_class_XMLWriter_startElement, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(endElement, xmlwriter_end_element, arginfo_class_XMLWriter_endElement, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(fullEndElement, xmlwriter_full_end_element, arginfo_class_XMLWriter_fullEndElement, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(startElementNs, xmlwriter_start_element_ns, arginfo_class_XMLWriter_startElementNs, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(writeElement, xmlwriter_write_element, arginfo_class_XMLWriter_writeElement, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(writeElementNs, xmlwriter_write_element_ns, arginfo_class_XMLWriter_writeElementNs, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(startPi, xmlwriter_start_pi, arginfo_class_XMLWriter_startPi, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(endPi, xmlwriter_end_pi, arginfo_class_XMLWriter_endPi, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(writePi, xmlwriter_write_pi, arginfo_class_XMLWriter_writePi, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(startCdata, xmlwriter_start_cdata, arginfo_class_XMLWriter_startCdata, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(endCdata, xmlwriter_end_cdata, arginfo_class_XMLWriter_endCdata, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(writeCdata, xmlwriter_write_cdata, arginfo_class_XMLWriter_writeCdata, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(text, xmlwriter_text, arginfo_class_XMLWriter_text, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(writeRaw, xmlwriter_write_raw, arginfo_class_XMLWriter_writeRaw, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(startDocument, xmlwriter_start_document, arginfo_class_XMLWriter_startDocument, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(endDocument, xmlwriter_end_document, arginfo_class_XMLWriter_endDocument, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(writeComment, xmlwriter_write_comment, arginfo_class_XMLWriter_writeComment, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(startDtd, xmlwriter_start_dtd, arginfo_class_XMLWriter_startDtd, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(endDtd, xmlwriter_end_dtd, arginfo_class_XMLWriter_endDtd, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(writeDtd, xmlwriter_write_dtd, arginfo_class_XMLWriter_writeDtd, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(startDtdElement, xmlwriter_start_dtd_element, arginfo_class_XMLWriter_startDtdElement, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(endDtdElement, xmlwriter_end_dtd_element, arginfo_class_XMLWriter_endDtdElement, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(writeDtdElement, xmlwriter_write_dtd_element, arginfo_class_XMLWriter_writeDtdElement, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(startDtdAttlist, xmlwriter_start_dtd_attlist, arginfo_class_XMLWriter_startDtdAttlist, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(endDtdAttlist, xmlwriter_end_dtd_attlist, arginfo_class_XMLWriter_endDtdAttlist, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(writeDtdAttlist, xmlwriter_write_dtd_attlist, arginfo_class_XMLWriter_writeDtdAttlist, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(startDtdEntity, xmlwriter_start_dtd_entity, arginfo_class_XMLWriter_startDtdEntity, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(endDtdEntity, xmlwriter_end_dtd_entity, arginfo_class_XMLWriter_endDtdEntity, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(writeDtdEntity, xmlwriter_write_dtd_entity, arginfo_class_XMLWriter_writeDtdEntity, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(outputMemory, xmlwriter_output_memory, arginfo_class_XMLWriter_outputMemory, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(flush, xmlwriter_flush, arginfo_class_XMLWriter_flush, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_XMLWriter(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "XMLWriter", class_XMLWriter_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	return class_entry;
}
