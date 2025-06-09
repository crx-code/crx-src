/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: dc41dddeea1ae117c6f2f3447afb29bf6623b757 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pdo_drivers, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()


CREX_FUNCTION(pdo_drivers);


static const crex_function_entry ext_functions[] = {
	CREX_FE(pdo_drivers, arginfo_pdo_drivers)
	CREX_FE_END
};


static const crex_function_entry class_PDOException_methods[] = {
	CREX_FE_END
};

static crex_class_entry *register_class_PDOException(crex_class_entry *class_entry_RuntimeException)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "PDOException", class_PDOException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_RuntimeException);

	zval property_code_default_value;
	ZVAL_LONG(&property_code_default_value, 0);
	crex_string *property_code_name = crex_string_init("code", sizeof("code") - 1, 1);
	crex_declare_typed_property(class_entry, property_code_name, &property_code_default_value, CREX_ACC_PROTECTED, NULL, (crex_type) CREX_TYPE_INIT_NONE(0));
	crex_string_release(property_code_name);

	zval property_errorInfo_default_value;
	ZVAL_NULL(&property_errorInfo_default_value);
	crex_string *property_errorInfo_name = crex_string_init("errorInfo", sizeof("errorInfo") - 1, 1);
	crex_declare_typed_property(class_entry, property_errorInfo_name, &property_errorInfo_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_ARRAY|MAY_BE_NULL));
	crex_string_release(property_errorInfo_name);

	return class_entry;
}
