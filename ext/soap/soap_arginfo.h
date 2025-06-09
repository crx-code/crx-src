/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 4dfc98696d4bc5e36610bdf03de906dbae049cf3 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_use_soap_error_handler, 0, 0, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, enable, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_is_soap_fault, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, object, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_SoapParam___main, 0, 0, 2)
	CREX_ARG_TYPE_INFO(0, data, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_SoapHeader___main, 0, 0, 2)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mustUnderstand, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_MASK(0, actor, MAY_BE_STRING|MAY_BE_LONG|MAY_BE_NULL, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_SoapFault___main, 0, 0, 2)
	CREX_ARG_TYPE_MASK(0, code, MAY_BE_ARRAY|MAY_BE_STRING|MAY_BE_NULL, NULL)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, actor, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, details, IS_MIXED, 0, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, name, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, headerFault, IS_MIXED, 0, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SoapFault___toString, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_SoapVar___main, 0, 0, 2)
	CREX_ARG_TYPE_INFO(0, data, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, encoding, IS_LONG, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, typeName, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, typeNamespace, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, nodeName, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, nodeNamespace, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_SoapServer___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, wsdl, IS_STRING, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SoapServer_fault, 0, 2, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, code, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, actor, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, details, IS_MIXED, 0, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, name, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SoapServer_addSoapHeader, 0, 1, IS_VOID, 0)
	CREX_ARG_OBJ_INFO(0, header, SoapHeader, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SoapServer_setPersistence, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SoapServer_setClass, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, class, IS_STRING, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(0, args, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SoapServer_setObject, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, object, IS_OBJECT, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SoapServer_getFunctions, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SoapServer_addFunction, 0, 1, IS_VOID, 0)
	CREX_ARG_INFO(0, functions)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SoapServer_handle, 0, 0, IS_VOID, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, request, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_class_SoapClient___main arginfo_class_SoapServer___main

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SoapClient___call, 0, 2, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, args, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SoapClient___soapCall, 0, 2, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, args, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 1, "null")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, inputHeaders, "null")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, outputHeaders, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SoapClient___getFunctions, 0, 0, IS_ARRAY, 1)
CREX_END_ARG_INFO()

#define arginfo_class_SoapClient___getTypes arginfo_class_SoapClient___getFunctions

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SoapClient___getLastRequest, 0, 0, IS_STRING, 1)
CREX_END_ARG_INFO()

#define arginfo_class_SoapClient___getLastResponse arginfo_class_SoapClient___getLastRequest

#define arginfo_class_SoapClient___getLastRequestHeaders arginfo_class_SoapClient___getLastRequest

#define arginfo_class_SoapClient___getLastResponseHeaders arginfo_class_SoapClient___getLastRequest

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SoapClient___doRequest, 0, 4, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, request, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, location, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, action, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, version, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, oneWay, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SoapClient___setCookie, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, value, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_class_SoapClient___getCookies arginfo_class_SoapServer_getFunctions

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SoapClient___setSoapHeaders, 0, 0, _IS_BOOL, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, headers, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SoapClient___setLocation, 0, 0, IS_STRING, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, location, IS_STRING, 1, "null")
CREX_END_ARG_INFO()


CREX_FUNCTION(use_soap_error_handler);
CREX_FUNCTION(is_soap_fault);
CREX_METHOD(SoapParam, __main);
CREX_METHOD(SoapHeader, __main);
CREX_METHOD(SoapFault, __main);
CREX_METHOD(SoapFault, __toString);
CREX_METHOD(SoapVar, __main);
CREX_METHOD(SoapServer, __main);
CREX_METHOD(SoapServer, fault);
CREX_METHOD(SoapServer, addSoapHeader);
CREX_METHOD(SoapServer, setPersistence);
CREX_METHOD(SoapServer, setClass);
CREX_METHOD(SoapServer, setObject);
CREX_METHOD(SoapServer, getFunctions);
CREX_METHOD(SoapServer, addFunction);
CREX_METHOD(SoapServer, handle);
CREX_METHOD(SoapClient, __main);
CREX_METHOD(SoapClient, __call);
CREX_METHOD(SoapClient, __soapCall);
CREX_METHOD(SoapClient, __getFunctions);
CREX_METHOD(SoapClient, __getTypes);
CREX_METHOD(SoapClient, __getLastRequest);
CREX_METHOD(SoapClient, __getLastResponse);
CREX_METHOD(SoapClient, __getLastRequestHeaders);
CREX_METHOD(SoapClient, __getLastResponseHeaders);
CREX_METHOD(SoapClient, __doRequest);
CREX_METHOD(SoapClient, __setCookie);
CREX_METHOD(SoapClient, __getCookies);
CREX_METHOD(SoapClient, __setSoapHeaders);
CREX_METHOD(SoapClient, __setLocation);


static const crex_function_entry ext_functions[] = {
	CREX_FE(use_soap_error_handler, arginfo_use_soap_error_handler)
	CREX_FE(is_soap_fault, arginfo_is_soap_fault)
	CREX_FE_END
};


static const crex_function_entry class_SoapParam_methods[] = {
	CREX_ME(SoapParam, __main, arginfo_class_SoapParam___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_SoapHeader_methods[] = {
	CREX_ME(SoapHeader, __main, arginfo_class_SoapHeader___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_SoapFault_methods[] = {
	CREX_ME(SoapFault, __main, arginfo_class_SoapFault___main, CREX_ACC_PUBLIC)
	CREX_ME(SoapFault, __toString, arginfo_class_SoapFault___toString, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_SoapVar_methods[] = {
	CREX_ME(SoapVar, __main, arginfo_class_SoapVar___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_SoapServer_methods[] = {
	CREX_ME(SoapServer, __main, arginfo_class_SoapServer___main, CREX_ACC_PUBLIC)
	CREX_ME(SoapServer, fault, arginfo_class_SoapServer_fault, CREX_ACC_PUBLIC)
	CREX_ME(SoapServer, addSoapHeader, arginfo_class_SoapServer_addSoapHeader, CREX_ACC_PUBLIC)
	CREX_ME(SoapServer, setPersistence, arginfo_class_SoapServer_setPersistence, CREX_ACC_PUBLIC)
	CREX_ME(SoapServer, setClass, arginfo_class_SoapServer_setClass, CREX_ACC_PUBLIC)
	CREX_ME(SoapServer, setObject, arginfo_class_SoapServer_setObject, CREX_ACC_PUBLIC)
	CREX_ME(SoapServer, getFunctions, arginfo_class_SoapServer_getFunctions, CREX_ACC_PUBLIC)
	CREX_ME(SoapServer, addFunction, arginfo_class_SoapServer_addFunction, CREX_ACC_PUBLIC)
	CREX_ME(SoapServer, handle, arginfo_class_SoapServer_handle, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_SoapClient_methods[] = {
	CREX_ME(SoapClient, __main, arginfo_class_SoapClient___main, CREX_ACC_PUBLIC)
	CREX_ME(SoapClient, __call, arginfo_class_SoapClient___call, CREX_ACC_PUBLIC)
	CREX_ME(SoapClient, __soapCall, arginfo_class_SoapClient___soapCall, CREX_ACC_PUBLIC)
	CREX_ME(SoapClient, __getFunctions, arginfo_class_SoapClient___getFunctions, CREX_ACC_PUBLIC)
	CREX_ME(SoapClient, __getTypes, arginfo_class_SoapClient___getTypes, CREX_ACC_PUBLIC)
	CREX_ME(SoapClient, __getLastRequest, arginfo_class_SoapClient___getLastRequest, CREX_ACC_PUBLIC)
	CREX_ME(SoapClient, __getLastResponse, arginfo_class_SoapClient___getLastResponse, CREX_ACC_PUBLIC)
	CREX_ME(SoapClient, __getLastRequestHeaders, arginfo_class_SoapClient___getLastRequestHeaders, CREX_ACC_PUBLIC)
	CREX_ME(SoapClient, __getLastResponseHeaders, arginfo_class_SoapClient___getLastResponseHeaders, CREX_ACC_PUBLIC)
	CREX_ME(SoapClient, __doRequest, arginfo_class_SoapClient___doRequest, CREX_ACC_PUBLIC)
	CREX_ME(SoapClient, __setCookie, arginfo_class_SoapClient___setCookie, CREX_ACC_PUBLIC)
	CREX_ME(SoapClient, __getCookies, arginfo_class_SoapClient___getCookies, CREX_ACC_PUBLIC)
	CREX_ME(SoapClient, __setSoapHeaders, arginfo_class_SoapClient___setSoapHeaders, CREX_ACC_PUBLIC)
	CREX_ME(SoapClient, __setLocation, arginfo_class_SoapClient___setLocation, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static void register_soap_symbols(int module_number)
{
	REGISTER_LONG_CONSTANT("SOAP_1_1", SOAP_1_1, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_1_2", SOAP_1_2, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_PERSISTENCE_SESSION", SOAP_PERSISTENCE_SESSION, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_PERSISTENCE_REQUEST", SOAP_PERSISTENCE_REQUEST, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_FUNCTIONS_ALL", SOAP_FUNCTIONS_ALL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_ENCODED", SOAP_ENCODED, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_LITERAL", SOAP_LITERAL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_RPC", SOAP_RPC, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_DOCUMENT", SOAP_DOCUMENT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_ACTOR_NEXT", SOAP_ACTOR_NEXT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_ACTOR_NONE", SOAP_ACTOR_NONE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_ACTOR_UNLIMATERECEIVER", SOAP_ACTOR_UNLIMATERECEIVER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_COMPRESSION_ACCEPT", SOAP_COMPRESSION_ACCEPT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_COMPRESSION_GZIP", SOAP_COMPRESSION_GZIP, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_COMPRESSION_DEFLATE", SOAP_COMPRESSION_DEFLATE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_AUTHENTICATION_BASIC", SOAP_AUTHENTICATION_BASIC, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_AUTHENTICATION_DIGEST", SOAP_AUTHENTICATION_DIGEST, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("UNKNOWN_TYPE", UNKNOWN_TYPE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_STRING", XSD_STRING, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_BOOLEAN", XSD_BOOLEAN, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_DECIMAL", XSD_DECIMAL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_FLOAT", XSD_FLOAT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_DOUBLE", XSD_DOUBLE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_DURATION", XSD_DURATION, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_DATETIME", XSD_DATETIME, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_TIME", XSD_TIME, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_DATE", XSD_DATE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_GYEARMONTH", XSD_GYEARMONTH, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_GYEAR", XSD_GYEAR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_GMONTHDAY", XSD_GMONTHDAY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_GDAY", XSD_GDAY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_GMONTH", XSD_GMONTH, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_HEXBINARY", XSD_HEXBINARY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_BASE64BINARY", XSD_BASE64BINARY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_ANYURI", XSD_ANYURI, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_QNAME", XSD_QNAME, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_NOTATION", XSD_NOTATION, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_NORMALIZEDSTRING", XSD_NORMALIZEDSTRING, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_TOKEN", XSD_TOKEN, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_LANGUAGE", XSD_LANGUAGE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_NMTOKEN", XSD_NMTOKEN, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_NAME", XSD_NAME, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_NCNAME", XSD_NCNAME, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_ID", XSD_ID, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_IDREF", XSD_IDREF, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_IDREFS", XSD_IDREFS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_ENTITY", XSD_ENTITY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_ENTITIES", XSD_ENTITIES, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_INTEGER", XSD_INTEGER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_NONPOSITIVEINTEGER", XSD_NONPOSITIVEINTEGER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_NEGATIVEINTEGER", XSD_NEGATIVEINTEGER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_LONG", XSD_LONG, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_INT", XSD_INT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_SHORT", XSD_SHORT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_BYTE", XSD_BYTE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_NONNEGATIVEINTEGER", XSD_NONNEGATIVEINTEGER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_UNSIGNEDLONG", XSD_UNSIGNEDLONG, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_UNSIGNEDINT", XSD_UNSIGNEDINT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_UNSIGNEDSHORT", XSD_UNSIGNEDSHORT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_UNSIGNEDBYTE", XSD_UNSIGNEDBYTE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_POSITIVEINTEGER", XSD_POSITIVEINTEGER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_NMTOKENS", XSD_NMTOKENS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_ANYTYPE", XSD_ANYTYPE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_ANYXML", XSD_ANYXML, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("APACHE_MAP", APACHE_MAP, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_ENC_OBJECT", SOAP_ENC_OBJECT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_ENC_ARRAY", SOAP_ENC_ARRAY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSD_1999_TIMEINSTANT", XSD_1999_TIMEINSTANT, CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("XSD_NAMESPACE", XSD_NAMESPACE, CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("XSD_1999_NAMESPACE", XSD_1999_NAMESPACE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_SINGLE_ELEMENT_ARRAYS", SOAP_SINGLE_ELEMENT_ARRAYS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_WAIT_ONE_WAY_CALLS", SOAP_WAIT_ONE_WAY_CALLS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_USE_XSI_ARRAY_TYPE", SOAP_USE_XSI_ARRAY_TYPE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("WSDL_CACHE_NONE", WSDL_CACHE_NONE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("WSDL_CACHE_DISK", WSDL_CACHE_DISK, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("WSDL_CACHE_MEMORY", WSDL_CACHE_MEMORY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("WSDL_CACHE_BOTH", WSDL_CACHE_BOTH, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_SSL_METHOD_TLS", SOAP_SSL_METHOD_TLS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_SSL_METHOD_SSLv2", SOAP_SSL_METHOD_SSLv2, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_SSL_METHOD_SSLv3", SOAP_SSL_METHOD_SSLv3, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SOAP_SSL_METHOD_SSLv23", SOAP_SSL_METHOD_SSLv23, CONST_PERSISTENT);
}

static crex_class_entry *register_class_SoapParam(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SoapParam", class_SoapParam_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	zval property_param_name_default_value;
	ZVAL_UNDEF(&property_param_name_default_value);
	crex_string *property_param_name_name = crex_string_init("param_name", sizeof("param_name") - 1, 1);
	crex_declare_typed_property(class_entry, property_param_name_name, &property_param_name_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_param_name_name);

	zval property_param_data_default_value;
	ZVAL_UNDEF(&property_param_data_default_value);
	crex_string *property_param_data_name = crex_string_init("param_data", sizeof("param_data") - 1, 1);
	crex_declare_typed_property(class_entry, property_param_data_name, &property_param_data_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_ANY));
	crex_string_release(property_param_data_name);

	return class_entry;
}

static crex_class_entry *register_class_SoapHeader(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SoapHeader", class_SoapHeader_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	zval property_namespace_default_value;
	ZVAL_UNDEF(&property_namespace_default_value);
	crex_string *property_namespace_name = crex_string_init("namespace", sizeof("namespace") - 1, 1);
	crex_declare_typed_property(class_entry, property_namespace_name, &property_namespace_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_namespace_name);

	zval property_name_default_value;
	ZVAL_UNDEF(&property_name_default_value);
	crex_string *property_name_name = crex_string_init("name", sizeof("name") - 1, 1);
	crex_declare_typed_property(class_entry, property_name_name, &property_name_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_name_name);

	zval property_data_default_value;
	ZVAL_NULL(&property_data_default_value);
	crex_string *property_data_name = crex_string_init("data", sizeof("data") - 1, 1);
	crex_declare_typed_property(class_entry, property_data_name, &property_data_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_ANY));
	crex_string_release(property_data_name);

	zval property_mustUnderstand_default_value;
	ZVAL_UNDEF(&property_mustUnderstand_default_value);
	crex_string *property_mustUnderstand_name = crex_string_init("mustUnderstand", sizeof("mustUnderstand") - 1, 1);
	crex_declare_typed_property(class_entry, property_mustUnderstand_name, &property_mustUnderstand_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property_mustUnderstand_name);

	zval property_actor_default_value;
	ZVAL_UNDEF(&property_actor_default_value);
	crex_string *property_actor_name = crex_string_init("actor", sizeof("actor") - 1, 1);
	crex_declare_typed_property(class_entry, property_actor_name, &property_actor_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_LONG|MAY_BE_NULL));
	crex_string_release(property_actor_name);

	return class_entry;
}

static crex_class_entry *register_class_SoapFault(crex_class_entry *class_entry_Exception)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SoapFault", class_SoapFault_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Exception);

	zval property_faultstring_default_value;
	ZVAL_UNDEF(&property_faultstring_default_value);
	crex_string *property_faultstring_name = crex_string_init("faultstring", sizeof("faultstring") - 1, 1);
	crex_declare_typed_property(class_entry, property_faultstring_name, &property_faultstring_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_faultstring_name);

	zval property_faultcode_default_value;
	ZVAL_NULL(&property_faultcode_default_value);
	crex_string *property_faultcode_name = crex_string_init("faultcode", sizeof("faultcode") - 1, 1);
	crex_declare_typed_property(class_entry, property_faultcode_name, &property_faultcode_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_faultcode_name);

	zval property_faultcodens_default_value;
	ZVAL_NULL(&property_faultcodens_default_value);
	crex_string *property_faultcodens_name = crex_string_init("faultcodens", sizeof("faultcodens") - 1, 1);
	crex_declare_typed_property(class_entry, property_faultcodens_name, &property_faultcodens_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_faultcodens_name);

	zval property_faultactor_default_value;
	ZVAL_NULL(&property_faultactor_default_value);
	crex_string *property_faultactor_name = crex_string_init("faultactor", sizeof("faultactor") - 1, 1);
	crex_declare_typed_property(class_entry, property_faultactor_name, &property_faultactor_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_faultactor_name);

	zval property_detail_default_value;
	ZVAL_NULL(&property_detail_default_value);
	crex_string *property_detail_name = crex_string_init("detail", sizeof("detail") - 1, 1);
	crex_declare_typed_property(class_entry, property_detail_name, &property_detail_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_ANY));
	crex_string_release(property_detail_name);

	zval property__name_default_value;
	ZVAL_NULL(&property__name_default_value);
	crex_string *property__name_name = crex_string_init("_name", sizeof("_name") - 1, 1);
	crex_declare_typed_property(class_entry, property__name_name, &property__name_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property__name_name);

	zval property_headerfault_default_value;
	ZVAL_NULL(&property_headerfault_default_value);
	crex_string *property_headerfault_name = crex_string_init("headerfault", sizeof("headerfault") - 1, 1);
	crex_declare_typed_property(class_entry, property_headerfault_name, &property_headerfault_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_ANY));
	crex_string_release(property_headerfault_name);

	return class_entry;
}

static crex_class_entry *register_class_SoapVar(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SoapVar", class_SoapVar_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	zval property_enc_type_default_value;
	ZVAL_UNDEF(&property_enc_type_default_value);
	crex_string *property_enc_type_name = crex_string_init("enc_type", sizeof("enc_type") - 1, 1);
	crex_declare_typed_property(class_entry, property_enc_type_name, &property_enc_type_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_enc_type_name);

	zval property_enc_value_default_value;
	ZVAL_NULL(&property_enc_value_default_value);
	crex_string *property_enc_value_name = crex_string_init("enc_value", sizeof("enc_value") - 1, 1);
	crex_declare_typed_property(class_entry, property_enc_value_name, &property_enc_value_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_ANY));
	crex_string_release(property_enc_value_name);

	zval property_enc_stype_default_value;
	ZVAL_NULL(&property_enc_stype_default_value);
	crex_string *property_enc_stype_name = crex_string_init("enc_stype", sizeof("enc_stype") - 1, 1);
	crex_declare_typed_property(class_entry, property_enc_stype_name, &property_enc_stype_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_enc_stype_name);

	zval property_enc_ns_default_value;
	ZVAL_NULL(&property_enc_ns_default_value);
	crex_string *property_enc_ns_name = crex_string_init("enc_ns", sizeof("enc_ns") - 1, 1);
	crex_declare_typed_property(class_entry, property_enc_ns_name, &property_enc_ns_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_enc_ns_name);

	zval property_enc_name_default_value;
	ZVAL_NULL(&property_enc_name_default_value);
	crex_string *property_enc_name_name = crex_string_init("enc_name", sizeof("enc_name") - 1, 1);
	crex_declare_typed_property(class_entry, property_enc_name_name, &property_enc_name_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_enc_name_name);

	zval property_enc_namens_default_value;
	ZVAL_NULL(&property_enc_namens_default_value);
	crex_string *property_enc_namens_name = crex_string_init("enc_namens", sizeof("enc_namens") - 1, 1);
	crex_declare_typed_property(class_entry, property_enc_namens_name, &property_enc_namens_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_enc_namens_name);

	return class_entry;
}

static crex_class_entry *register_class_SoapServer(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SoapServer", class_SoapServer_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	zval property___soap_fault_default_value;
	ZVAL_NULL(&property___soap_fault_default_value);
	crex_string *property___soap_fault_name = crex_string_init("__soap_fault", sizeof("__soap_fault") - 1, 1);
	crex_string *property___soap_fault_class_SoapFault = crex_string_init("SoapFault", sizeof("SoapFault")-1, 1);
	crex_declare_typed_property(class_entry, property___soap_fault_name, &property___soap_fault_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property___soap_fault_class_SoapFault, 0, MAY_BE_NULL));
	crex_string_release(property___soap_fault_name);

	return class_entry;
}

static crex_class_entry *register_class_SoapClient(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SoapClient", class_SoapClient_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	zval property_uri_default_value;
	ZVAL_NULL(&property_uri_default_value);
	crex_string *property_uri_name = crex_string_init("uri", sizeof("uri") - 1, 1);
	crex_declare_typed_property(class_entry, property_uri_name, &property_uri_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_uri_name);

	zval property_style_default_value;
	ZVAL_NULL(&property_style_default_value);
	crex_string *property_style_name = crex_string_init("style", sizeof("style") - 1, 1);
	crex_declare_typed_property(class_entry, property_style_name, &property_style_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG|MAY_BE_NULL));
	crex_string_release(property_style_name);

	zval property_use_default_value;
	ZVAL_NULL(&property_use_default_value);
	crex_string *property_use_name = crex_string_init("use", sizeof("use") - 1, 1);
	crex_declare_typed_property(class_entry, property_use_name, &property_use_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG|MAY_BE_NULL));
	crex_string_release(property_use_name);

	zval property_location_default_value;
	ZVAL_NULL(&property_location_default_value);
	crex_string *property_location_name = crex_string_init("location", sizeof("location") - 1, 1);
	crex_declare_typed_property(class_entry, property_location_name, &property_location_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_location_name);

	zval property_trace_default_value;
	ZVAL_FALSE(&property_trace_default_value);
	crex_string *property_trace_name = crex_string_init("trace", sizeof("trace") - 1, 1);
	crex_declare_typed_property(class_entry, property_trace_name, &property_trace_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property_trace_name);

	zval property_compression_default_value;
	ZVAL_NULL(&property_compression_default_value);
	crex_string *property_compression_name = crex_string_init("compression", sizeof("compression") - 1, 1);
	crex_declare_typed_property(class_entry, property_compression_name, &property_compression_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG|MAY_BE_NULL));
	crex_string_release(property_compression_name);

	zval property_sdl_default_value;
	ZVAL_NULL(&property_sdl_default_value);
	crex_string *property_sdl_name = crex_string_init("sdl", sizeof("sdl") - 1, 1);
	crex_declare_typed_property(class_entry, property_sdl_name, &property_sdl_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_NONE(0));
	crex_string_release(property_sdl_name);

	zval property_typemap_default_value;
	ZVAL_NULL(&property_typemap_default_value);
	crex_string *property_typemap_name = crex_string_init("typemap", sizeof("typemap") - 1, 1);
	crex_declare_typed_property(class_entry, property_typemap_name, &property_typemap_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_NONE(0));
	crex_string_release(property_typemap_name);

	zval property_httpsocket_default_value;
	ZVAL_NULL(&property_httpsocket_default_value);
	crex_string *property_httpsocket_name = crex_string_init("httpsocket", sizeof("httpsocket") - 1, 1);
	crex_declare_typed_property(class_entry, property_httpsocket_name, &property_httpsocket_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_NONE(0));
	crex_string_release(property_httpsocket_name);

	zval property_httpurl_default_value;
	ZVAL_NULL(&property_httpurl_default_value);
	crex_string *property_httpurl_name = crex_string_init("httpurl", sizeof("httpurl") - 1, 1);
	crex_declare_typed_property(class_entry, property_httpurl_name, &property_httpurl_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_NONE(0));
	crex_string_release(property_httpurl_name);

	zval property__login_default_value;
	ZVAL_NULL(&property__login_default_value);
	crex_string *property__login_name = crex_string_init("_login", sizeof("_login") - 1, 1);
	crex_declare_typed_property(class_entry, property__login_name, &property__login_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property__login_name);

	zval property__password_default_value;
	ZVAL_NULL(&property__password_default_value);
	crex_string *property__password_name = crex_string_init("_password", sizeof("_password") - 1, 1);
	crex_declare_typed_property(class_entry, property__password_name, &property__password_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property__password_name);

	zval property__use_digest_default_value;
	ZVAL_FALSE(&property__use_digest_default_value);
	crex_string *property__use_digest_name = crex_string_init("_use_digest", sizeof("_use_digest") - 1, 1);
	crex_declare_typed_property(class_entry, property__use_digest_name, &property__use_digest_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property__use_digest_name);

	zval property__digest_default_value;
	ZVAL_NULL(&property__digest_default_value);
	crex_string *property__digest_name = crex_string_init("_digest", sizeof("_digest") - 1, 1);
	crex_declare_typed_property(class_entry, property__digest_name, &property__digest_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property__digest_name);

	zval property__proxy_host_default_value;
	ZVAL_NULL(&property__proxy_host_default_value);
	crex_string *property__proxy_host_name = crex_string_init("_proxy_host", sizeof("_proxy_host") - 1, 1);
	crex_declare_typed_property(class_entry, property__proxy_host_name, &property__proxy_host_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property__proxy_host_name);

	zval property__proxy_port_default_value;
	ZVAL_NULL(&property__proxy_port_default_value);
	crex_string *property__proxy_port_name = crex_string_init("_proxy_port", sizeof("_proxy_port") - 1, 1);
	crex_declare_typed_property(class_entry, property__proxy_port_name, &property__proxy_port_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG|MAY_BE_NULL));
	crex_string_release(property__proxy_port_name);

	zval property__proxy_login_default_value;
	ZVAL_NULL(&property__proxy_login_default_value);
	crex_string *property__proxy_login_name = crex_string_init("_proxy_login", sizeof("_proxy_login") - 1, 1);
	crex_declare_typed_property(class_entry, property__proxy_login_name, &property__proxy_login_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property__proxy_login_name);

	zval property__proxy_password_default_value;
	ZVAL_NULL(&property__proxy_password_default_value);
	crex_string *property__proxy_password_name = crex_string_init("_proxy_password", sizeof("_proxy_password") - 1, 1);
	crex_declare_typed_property(class_entry, property__proxy_password_name, &property__proxy_password_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property__proxy_password_name);

	zval property__exceptions_default_value;
	ZVAL_TRUE(&property__exceptions_default_value);
	crex_string *property__exceptions_name = crex_string_init("_exceptions", sizeof("_exceptions") - 1, 1);
	crex_declare_typed_property(class_entry, property__exceptions_name, &property__exceptions_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property__exceptions_name);

	zval property__encoding_default_value;
	ZVAL_NULL(&property__encoding_default_value);
	crex_string *property__encoding_name = crex_string_init("_encoding", sizeof("_encoding") - 1, 1);
	crex_declare_typed_property(class_entry, property__encoding_name, &property__encoding_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property__encoding_name);

	zval property__classmap_default_value;
	ZVAL_NULL(&property__classmap_default_value);
	crex_string *property__classmap_name = crex_string_init("_classmap", sizeof("_classmap") - 1, 1);
	crex_declare_typed_property(class_entry, property__classmap_name, &property__classmap_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_ARRAY|MAY_BE_NULL));
	crex_string_release(property__classmap_name);

	zval property__features_default_value;
	ZVAL_NULL(&property__features_default_value);
	crex_string *property__features_name = crex_string_init("_features", sizeof("_features") - 1, 1);
	crex_declare_typed_property(class_entry, property__features_name, &property__features_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG|MAY_BE_NULL));
	crex_string_release(property__features_name);

	zval property__connection_timeout_default_value;
	ZVAL_LONG(&property__connection_timeout_default_value, 0);
	crex_string *property__connection_timeout_name = crex_string_init("_connection_timeout", sizeof("_connection_timeout") - 1, 1);
	crex_declare_typed_property(class_entry, property__connection_timeout_name, &property__connection_timeout_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property__connection_timeout_name);

	zval property__stream_context_default_value;
	ZVAL_NULL(&property__stream_context_default_value);
	crex_string *property__stream_context_name = crex_string_init("_stream_context", sizeof("_stream_context") - 1, 1);
	crex_declare_typed_property(class_entry, property__stream_context_name, &property__stream_context_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_NONE(0));
	crex_string_release(property__stream_context_name);

	zval property__user_agent_default_value;
	ZVAL_NULL(&property__user_agent_default_value);
	crex_string *property__user_agent_name = crex_string_init("_user_agent", sizeof("_user_agent") - 1, 1);
	crex_declare_typed_property(class_entry, property__user_agent_name, &property__user_agent_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property__user_agent_name);

	zval property__keep_alive_default_value;
	ZVAL_TRUE(&property__keep_alive_default_value);
	crex_string *property__keep_alive_name = crex_string_init("_keep_alive", sizeof("_keep_alive") - 1, 1);
	crex_declare_typed_property(class_entry, property__keep_alive_name, &property__keep_alive_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property__keep_alive_name);

	zval property__ssl_method_default_value;
	ZVAL_NULL(&property__ssl_method_default_value);
	crex_string *property__ssl_method_name = crex_string_init("_ssl_method", sizeof("_ssl_method") - 1, 1);
	crex_declare_typed_property(class_entry, property__ssl_method_name, &property__ssl_method_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG|MAY_BE_NULL));
	crex_string_release(property__ssl_method_name);

	zval property__soap_version_default_value;
	ZVAL_UNDEF(&property__soap_version_default_value);
	crex_string *property__soap_version_name = crex_string_init("_soap_version", sizeof("_soap_version") - 1, 1);
	crex_declare_typed_property(class_entry, property__soap_version_name, &property__soap_version_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property__soap_version_name);

	zval property__use_proxy_default_value;
	ZVAL_NULL(&property__use_proxy_default_value);
	crex_string *property__use_proxy_name = crex_string_init("_use_proxy", sizeof("_use_proxy") - 1, 1);
	crex_declare_typed_property(class_entry, property__use_proxy_name, &property__use_proxy_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG|MAY_BE_NULL));
	crex_string_release(property__use_proxy_name);

	zval property__cookies_default_value;
	ZVAL_EMPTY_ARRAY(&property__cookies_default_value);
	crex_string *property__cookies_name = crex_string_init("_cookies", sizeof("_cookies") - 1, 1);
	crex_declare_typed_property(class_entry, property__cookies_name, &property__cookies_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_ARRAY));
	crex_string_release(property__cookies_name);

	zval property___default_headers_default_value;
	ZVAL_NULL(&property___default_headers_default_value);
	crex_string *property___default_headers_name = crex_string_init("__default_headers", sizeof("__default_headers") - 1, 1);
	crex_declare_typed_property(class_entry, property___default_headers_name, &property___default_headers_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_ARRAY|MAY_BE_NULL));
	crex_string_release(property___default_headers_name);

	zval property___soap_fault_default_value;
	ZVAL_NULL(&property___soap_fault_default_value);
	crex_string *property___soap_fault_name = crex_string_init("__soap_fault", sizeof("__soap_fault") - 1, 1);
	crex_string *property___soap_fault_class_SoapFault = crex_string_init("SoapFault", sizeof("SoapFault")-1, 1);
	crex_declare_typed_property(class_entry, property___soap_fault_name, &property___soap_fault_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property___soap_fault_class_SoapFault, 0, MAY_BE_NULL));
	crex_string_release(property___soap_fault_name);

	zval property___last_request_default_value;
	ZVAL_NULL(&property___last_request_default_value);
	crex_string *property___last_request_name = crex_string_init("__last_request", sizeof("__last_request") - 1, 1);
	crex_declare_typed_property(class_entry, property___last_request_name, &property___last_request_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property___last_request_name);

	zval property___last_response_default_value;
	ZVAL_NULL(&property___last_response_default_value);
	crex_string *property___last_response_name = crex_string_init("__last_response", sizeof("__last_response") - 1, 1);
	crex_declare_typed_property(class_entry, property___last_response_name, &property___last_response_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property___last_response_name);

	zval property___last_request_headers_default_value;
	ZVAL_NULL(&property___last_request_headers_default_value);
	crex_string *property___last_request_headers_name = crex_string_init("__last_request_headers", sizeof("__last_request_headers") - 1, 1);
	crex_declare_typed_property(class_entry, property___last_request_headers_name, &property___last_request_headers_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property___last_request_headers_name);

	zval property___last_response_headers_default_value;
	ZVAL_NULL(&property___last_response_headers_default_value);
	crex_string *property___last_response_headers_name = crex_string_init("__last_response_headers", sizeof("__last_response_headers") - 1, 1);
	crex_declare_typed_property(class_entry, property___last_response_headers_name, &property___last_response_headers_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property___last_response_headers_name);

	return class_entry;
}
