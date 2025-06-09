/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 0d09bd2f3b0a155cef25ca343319ecf470424d71 */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_CURLFile___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mime_type, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, posted_filename, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_CURLFile_getFilename, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_CURLFile_getMimeType arginfo_class_CURLFile_getFilename

#define arginfo_class_CURLFile_getPostFilename arginfo_class_CURLFile_getFilename

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_CURLFile_setMimeType, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, mime_type, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_CURLFile_setPostFilename, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, posted_filename, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_CURLStringFile___main, 0, 0, 2)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, postname, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mime, IS_STRING, 0, "\"application/octet-stream\"")
CREX_END_ARG_INFO()


CREX_METHOD(CURLFile, __main);
CREX_METHOD(CURLFile, getFilename);
CREX_METHOD(CURLFile, getMimeType);
CREX_METHOD(CURLFile, getPostFilename);
CREX_METHOD(CURLFile, setMimeType);
CREX_METHOD(CURLFile, setPostFilename);
CREX_METHOD(CURLStringFile, __main);


static const crex_function_entry class_CURLFile_methods[] = {
	CREX_ME(CURLFile, __main, arginfo_class_CURLFile___main, CREX_ACC_PUBLIC)
	CREX_ME(CURLFile, getFilename, arginfo_class_CURLFile_getFilename, CREX_ACC_PUBLIC)
	CREX_ME(CURLFile, getMimeType, arginfo_class_CURLFile_getMimeType, CREX_ACC_PUBLIC)
	CREX_ME(CURLFile, getPostFilename, arginfo_class_CURLFile_getPostFilename, CREX_ACC_PUBLIC)
	CREX_ME(CURLFile, setMimeType, arginfo_class_CURLFile_setMimeType, CREX_ACC_PUBLIC)
	CREX_ME(CURLFile, setPostFilename, arginfo_class_CURLFile_setPostFilename, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_CURLStringFile_methods[] = {
	CREX_ME(CURLStringFile, __main, arginfo_class_CURLStringFile___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_CURLFile(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "CURLFile", class_CURLFile_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;

	zval property_name_default_value;
	ZVAL_EMPTY_STRING(&property_name_default_value);
	crex_string *property_name_name = crex_string_init("name", sizeof("name") - 1, 1);
	crex_declare_typed_property(class_entry, property_name_name, &property_name_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_name_name);

	zval property_mime_default_value;
	ZVAL_EMPTY_STRING(&property_mime_default_value);
	crex_string *property_mime_name = crex_string_init("mime", sizeof("mime") - 1, 1);
	crex_declare_typed_property(class_entry, property_mime_name, &property_mime_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_mime_name);

	zval property_postname_default_value;
	ZVAL_EMPTY_STRING(&property_postname_default_value);
	crex_string *property_postname_name = crex_string_init("postname", sizeof("postname") - 1, 1);
	crex_declare_typed_property(class_entry, property_postname_name, &property_postname_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_postname_name);

	return class_entry;
}

static crex_class_entry *register_class_CURLStringFile(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "CURLStringFile", class_CURLStringFile_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	zval property_data_default_value;
	ZVAL_UNDEF(&property_data_default_value);
	crex_string *property_data_name = crex_string_init("data", sizeof("data") - 1, 1);
	crex_declare_typed_property(class_entry, property_data_name, &property_data_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_data_name);

	zval property_postname_default_value;
	ZVAL_UNDEF(&property_postname_default_value);
	crex_string *property_postname_name = crex_string_init("postname", sizeof("postname") - 1, 1);
	crex_declare_typed_property(class_entry, property_postname_name, &property_postname_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_postname_name);

	zval property_mime_default_value;
	ZVAL_UNDEF(&property_mime_default_value);
	crex_string *property_mime_name = crex_string_init("mime", sizeof("mime") - 1, 1);
	crex_declare_typed_property(class_entry, property_mime_name, &property_mime_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_mime_name);

	return class_entry;
}
