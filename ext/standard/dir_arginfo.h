/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: b3890e798e609e306027b4717ce0c5e782884087 */

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Directory_close, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

#define arginfo_class_Directory_rewind arginfo_class_Directory_close

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_Directory_read, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
CREX_END_ARG_INFO()


CREX_FUNCTION(closedir);
CREX_FUNCTION(rewinddir);
CREX_FUNCTION(readdir);


static const crex_function_entry class_Directory_methods[] = {
	CREX_ME_MAPPING(close, closedir, arginfo_class_Directory_close, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(rewind, rewinddir, arginfo_class_Directory_rewind, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(read, readdir, arginfo_class_Directory_read, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_Directory(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Directory", class_Directory_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	zval property_path_default_value;
	ZVAL_UNDEF(&property_path_default_value);
	crex_string *property_path_name = crex_string_init("path", sizeof("path") - 1, 1);
	crex_declare_typed_property(class_entry, property_path_name, &property_path_default_value, CREX_ACC_PUBLIC|CREX_ACC_READONLY, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_path_name);

	zval property_handle_default_value;
	ZVAL_UNDEF(&property_handle_default_value);
	crex_string *property_handle_name = crex_string_init("handle", sizeof("handle") - 1, 1);
	crex_declare_typed_property(class_entry, property_handle_name, &property_handle_default_value, CREX_ACC_PUBLIC|CREX_ACC_READONLY, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_ANY));
	crex_string_release(property_handle_name);

	return class_entry;
}
