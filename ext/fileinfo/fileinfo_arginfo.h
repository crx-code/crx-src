/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 94697d59958fb55a431bfa4786158b5db3c1ae0e */

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_finfo_open, 0, 0, finfo, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "FILEINFO_NONE")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, magic_database, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_finfo_close, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, finfo, finfo, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_finfo_set_flags, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, finfo, finfo, 0)
	CREX_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_finfo_file, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, finfo, finfo, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "FILEINFO_NONE")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, context, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_finfo_buffer, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, finfo, finfo, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "FILEINFO_NONE")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, context, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mime_content_type, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO(0, filename)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_finfo___main, 0, 0, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "FILEINFO_NONE")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, magic_database, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_finfo_file, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "FILEINFO_NONE")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, context, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_finfo_buffer, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "FILEINFO_NONE")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, context, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_finfo_set_flags, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
CREX_END_ARG_INFO()


CREX_FUNCTION(finfo_open);
CREX_FUNCTION(finfo_close);
CREX_FUNCTION(finfo_set_flags);
CREX_FUNCTION(finfo_file);
CREX_FUNCTION(finfo_buffer);
CREX_FUNCTION(mime_content_type);


static const crex_function_entry ext_functions[] = {
	CREX_FE(finfo_open, arginfo_finfo_open)
	CREX_FE(finfo_close, arginfo_finfo_close)
	CREX_FE(finfo_set_flags, arginfo_finfo_set_flags)
	CREX_FE(finfo_file, arginfo_finfo_file)
	CREX_FE(finfo_buffer, arginfo_finfo_buffer)
	CREX_FE(mime_content_type, arginfo_mime_content_type)
	CREX_FE_END
};


static const crex_function_entry class_finfo_methods[] = {
	CREX_ME_MAPPING(__main, finfo_open, arginfo_class_finfo___main, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(file, finfo_file, arginfo_class_finfo_file, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(buffer, finfo_buffer, arginfo_class_finfo_buffer, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(set_flags, finfo_set_flags, arginfo_class_finfo_set_flags, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static void register_fileinfo_symbols(int module_number)
{
	REGISTER_LONG_CONSTANT("FILEINFO_NONE", MAGIC_NONE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FILEINFO_SYMLINK", MAGIC_SYMLINK, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FILEINFO_MIME", MAGIC_MIME, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FILEINFO_MIME_TYPE", MAGIC_MIME_TYPE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FILEINFO_MIME_ENCODING", MAGIC_MIME_ENCODING, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FILEINFO_DEVICES", MAGIC_DEVICES, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FILEINFO_CONTINUE", MAGIC_CONTINUE, CONST_PERSISTENT);
#if defined(MAGIC_PRESERVE_ATIME)
	REGISTER_LONG_CONSTANT("FILEINFO_PRESERVE_ATIME", MAGIC_PRESERVE_ATIME, CONST_PERSISTENT);
#endif
#if defined(MAGIC_RAW)
	REGISTER_LONG_CONSTANT("FILEINFO_RAW", MAGIC_RAW, CONST_PERSISTENT);
#endif
	REGISTER_LONG_CONSTANT("FILEINFO_APPLE", MAGIC_APPLE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FILEINFO_EXTENSION", MAGIC_EXTENSION, CONST_PERSISTENT);
}

static crex_class_entry *register_class_finfo(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "finfo", class_finfo_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}
