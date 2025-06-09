/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 606e6ceba2381588b28e25e140fbcfec8a4dbe84 */

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XSLTProcessor_importStylesheet, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, stylesheet, IS_OBJECT, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_XSLTProcessor_transformToDoc, 0, 1, MAY_BE_OBJECT|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, document, IS_OBJECT, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, returnClass, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XSLTProcessor_transformToUri, 0, 2, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, document, IS_OBJECT, 0)
	CREX_ARG_TYPE_INFO(0, uri, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_XSLTProcessor_transformToXml, 0, 1, MAY_BE_STRING|MAY_BE_NULL|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, document, IS_OBJECT, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XSLTProcessor_setParameter, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 0)
	CREX_ARG_TYPE_MASK(0, name, MAY_BE_ARRAY|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, value, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_XSLTProcessor_getParameter, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XSLTProcessor_removeParameter, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, namespace, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XSLTProcessor_hasExsltSupport, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XSLTProcessor_registerCRXFunctions, 0, 0, IS_VOID, 0)
	CREX_ARG_TYPE_MASK(0, functions, MAY_BE_ARRAY|MAY_BE_STRING|MAY_BE_NULL, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_XSLTProcessor_setProfiling, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XSLTProcessor_setSecurityPrefs, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, preferences, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_XSLTProcessor_getSecurityPrefs, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()


CREX_METHOD(XSLTProcessor, importStylesheet);
CREX_METHOD(XSLTProcessor, transformToDoc);
CREX_METHOD(XSLTProcessor, transformToUri);
CREX_METHOD(XSLTProcessor, transformToXml);
CREX_METHOD(XSLTProcessor, setParameter);
CREX_METHOD(XSLTProcessor, getParameter);
CREX_METHOD(XSLTProcessor, removeParameter);
CREX_METHOD(XSLTProcessor, hasExsltSupport);
CREX_METHOD(XSLTProcessor, registerCRXFunctions);
CREX_METHOD(XSLTProcessor, setProfiling);
CREX_METHOD(XSLTProcessor, setSecurityPrefs);
CREX_METHOD(XSLTProcessor, getSecurityPrefs);


static const crex_function_entry class_XSLTProcessor_methods[] = {
	CREX_ME(XSLTProcessor, importStylesheet, arginfo_class_XSLTProcessor_importStylesheet, CREX_ACC_PUBLIC)
	CREX_ME(XSLTProcessor, transformToDoc, arginfo_class_XSLTProcessor_transformToDoc, CREX_ACC_PUBLIC)
	CREX_ME(XSLTProcessor, transformToUri, arginfo_class_XSLTProcessor_transformToUri, CREX_ACC_PUBLIC)
	CREX_ME(XSLTProcessor, transformToXml, arginfo_class_XSLTProcessor_transformToXml, CREX_ACC_PUBLIC)
	CREX_ME(XSLTProcessor, setParameter, arginfo_class_XSLTProcessor_setParameter, CREX_ACC_PUBLIC)
	CREX_ME(XSLTProcessor, getParameter, arginfo_class_XSLTProcessor_getParameter, CREX_ACC_PUBLIC)
	CREX_ME(XSLTProcessor, removeParameter, arginfo_class_XSLTProcessor_removeParameter, CREX_ACC_PUBLIC)
	CREX_ME(XSLTProcessor, hasExsltSupport, arginfo_class_XSLTProcessor_hasExsltSupport, CREX_ACC_PUBLIC)
	CREX_ME(XSLTProcessor, registerCRXFunctions, arginfo_class_XSLTProcessor_registerCRXFunctions, CREX_ACC_PUBLIC)
	CREX_ME(XSLTProcessor, setProfiling, arginfo_class_XSLTProcessor_setProfiling, CREX_ACC_PUBLIC)
	CREX_ME(XSLTProcessor, setSecurityPrefs, arginfo_class_XSLTProcessor_setSecurityPrefs, CREX_ACC_PUBLIC)
	CREX_ME(XSLTProcessor, getSecurityPrefs, arginfo_class_XSLTProcessor_getSecurityPrefs, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static void register_crx_xsl_symbols(int module_number)
{
	REGISTER_LONG_CONSTANT("XSL_CLONE_AUTO", 0, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSL_CLONE_NEVER", -1, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSL_CLONE_ALWAYS", 1, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSL_SECPREF_NONE", XSL_SECPREF_NONE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSL_SECPREF_READ_FILE", XSL_SECPREF_READ_FILE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSL_SECPREF_WRITE_FILE", XSL_SECPREF_WRITE_FILE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSL_SECPREF_CREATE_DIRECTORY", XSL_SECPREF_CREATE_DIRECTORY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSL_SECPREF_READ_NETWORK", XSL_SECPREF_READ_NETWORK, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSL_SECPREF_WRITE_NETWORK", XSL_SECPREF_WRITE_NETWORK, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XSL_SECPREF_DEFAULT", XSL_SECPREF_DEFAULT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBXSLT_VERSION", LIBXSLT_VERSION, CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("LIBXSLT_DOTTED_VERSION", LIBXSLT_DOTTED_VERSION, CONST_PERSISTENT);
#if defined(HAVE_XSL_EXSLT)
	REGISTER_LONG_CONSTANT("LIBEXSLT_VERSION", LIBEXSLT_VERSION, CONST_PERSISTENT);
#endif
#if defined(HAVE_XSL_EXSLT)
	REGISTER_STRING_CONSTANT("LIBEXSLT_DOTTED_VERSION", LIBEXSLT_DOTTED_VERSION, CONST_PERSISTENT);
#endif
}

static crex_class_entry *register_class_XSLTProcessor(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "XSLTProcessor", class_XSLTProcessor_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	return class_entry;
}
