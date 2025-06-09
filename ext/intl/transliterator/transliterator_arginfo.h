/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: dc2edb1b4d2c4fde746a2f29613ffba447cb5ee6 */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_Transliterator___main, 0, 0, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_Transliterator_create, 0, 1, Transliterator, 1)
	CREX_ARG_TYPE_INFO(0, id, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, direction, IS_LONG, 0, "Transliterator::FORWARD")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_Transliterator_createFromRules, 0, 1, Transliterator, 1)
	CREX_ARG_TYPE_INFO(0, rules, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, direction, IS_LONG, 0, "Transliterator::FORWARD")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_Transliterator_createInverse, 0, 0, Transliterator, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_Transliterator_listIDs, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_Transliterator_transliterate, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, start, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, end, IS_LONG, 0, "-1")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_Transliterator_getErrorCode, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_Transliterator_getErrorMessage, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
CREX_END_ARG_INFO()


CREX_METHOD(Transliterator, __main);
CREX_FUNCTION(transliterator_create);
CREX_FUNCTION(transliterator_create_from_rules);
CREX_FUNCTION(transliterator_create_inverse);
CREX_FUNCTION(transliterator_list_ids);
CREX_FUNCTION(transliterator_transliterate);
CREX_FUNCTION(transliterator_get_error_code);
CREX_FUNCTION(transliterator_get_error_message);


static const crex_function_entry class_Transliterator_methods[] = {
	CREX_ME(Transliterator, __main, arginfo_class_Transliterator___main, CREX_ACC_PRIVATE|CREX_ACC_FINAL)
	CREX_ME_MAPPING(create, transliterator_create, arginfo_class_Transliterator_create, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(createFromRules, transliterator_create_from_rules, arginfo_class_Transliterator_createFromRules, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(createInverse, transliterator_create_inverse, arginfo_class_Transliterator_createInverse, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(listIDs, transliterator_list_ids, arginfo_class_Transliterator_listIDs, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(transliterate, transliterator_transliterate, arginfo_class_Transliterator_transliterate, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getErrorCode, transliterator_get_error_code, arginfo_class_Transliterator_getErrorCode, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getErrorMessage, transliterator_get_error_message, arginfo_class_Transliterator_getErrorMessage, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_Transliterator(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Transliterator", class_Transliterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;

	zval const_FORWARD_value;
	ZVAL_LONG(&const_FORWARD_value, TRANSLITERATOR_FORWARD);
	crex_string *const_FORWARD_name = crex_string_init_interned("FORWARD", sizeof("FORWARD") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FORWARD_name, &const_FORWARD_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FORWARD_name);

	zval const_REVERSE_value;
	ZVAL_LONG(&const_REVERSE_value, TRANSLITERATOR_REVERSE);
	crex_string *const_REVERSE_name = crex_string_init_interned("REVERSE", sizeof("REVERSE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_REVERSE_name, &const_REVERSE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_REVERSE_name);

	zval property_id_default_value;
	ZVAL_UNDEF(&property_id_default_value);
	crex_string *property_id_name = crex_string_init("id", sizeof("id") - 1, 1);
	crex_declare_typed_property(class_entry, property_id_name, &property_id_default_value, CREX_ACC_PUBLIC|CREX_ACC_READONLY, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_id_name);

	return class_entry;
}
