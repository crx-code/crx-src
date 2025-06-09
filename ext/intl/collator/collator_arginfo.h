/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: be1f74889b55b997f8b2647fe65ab86cf4d72c0b */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_Collator___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_Collator_create, 0, 1, Collator, 1)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_Collator_compare, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string2, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Collator_sort, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(1, array, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "Collator::SORT_REGULAR")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Collator_sortWithSortKeys, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(1, array, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_class_Collator_asort arginfo_class_Collator_sort

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_Collator_getAttribute, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, attribute, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Collator_setAttribute, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, attribute, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Collator_getStrength, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_Collator_setStrength, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, strength, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_Collator_getLocale, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, type, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_Collator_getErrorCode, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_Collator_getErrorMessage, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_Collator_getSortKey, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
CREX_END_ARG_INFO()


CREX_METHOD(Collator, __main);
CREX_FUNCTION(collator_create);
CREX_FUNCTION(collator_compare);
CREX_FUNCTION(collator_sort);
CREX_FUNCTION(collator_sort_with_sort_keys);
CREX_FUNCTION(collator_asort);
CREX_FUNCTION(collator_get_attribute);
CREX_FUNCTION(collator_set_attribute);
CREX_FUNCTION(collator_get_strength);
CREX_FUNCTION(collator_set_strength);
CREX_FUNCTION(collator_get_locale);
CREX_FUNCTION(collator_get_error_code);
CREX_FUNCTION(collator_get_error_message);
CREX_FUNCTION(collator_get_sort_key);


static const crex_function_entry class_Collator_methods[] = {
	CREX_ME(Collator, __main, arginfo_class_Collator___main, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(create, collator_create, arginfo_class_Collator_create, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(compare, collator_compare, arginfo_class_Collator_compare, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(sort, collator_sort, arginfo_class_Collator_sort, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(sortWithSortKeys, collator_sort_with_sort_keys, arginfo_class_Collator_sortWithSortKeys, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(asort, collator_asort, arginfo_class_Collator_asort, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getAttribute, collator_get_attribute, arginfo_class_Collator_getAttribute, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setAttribute, collator_set_attribute, arginfo_class_Collator_setAttribute, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getStrength, collator_get_strength, arginfo_class_Collator_getStrength, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setStrength, collator_set_strength, arginfo_class_Collator_setStrength, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getLocale, collator_get_locale, arginfo_class_Collator_getLocale, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getErrorCode, collator_get_error_code, arginfo_class_Collator_getErrorCode, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getErrorMessage, collator_get_error_message, arginfo_class_Collator_getErrorMessage, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getSortKey, collator_get_sort_key, arginfo_class_Collator_getSortKey, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static void register_collator_symbols(int module_number)
{
	REGISTER_LONG_CONSTANT("ULOC_ACTUAL_LOCALE", ULOC_ACTUAL_LOCALE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ULOC_VALID_LOCALE", ULOC_VALID_LOCALE, CONST_PERSISTENT);
}

static crex_class_entry *register_class_Collator(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Collator", class_Collator_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;

	zval const_DEFAULT_VALUE_value;
	ZVAL_LONG(&const_DEFAULT_VALUE_value, UCOL_DEFAULT);
	crex_string *const_DEFAULT_VALUE_name = crex_string_init_interned("DEFAULT_VALUE", sizeof("DEFAULT_VALUE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DEFAULT_VALUE_name, &const_DEFAULT_VALUE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DEFAULT_VALUE_name);

	zval const_PRIMARY_value;
	ZVAL_LONG(&const_PRIMARY_value, UCOL_PRIMARY);
	crex_string *const_PRIMARY_name = crex_string_init_interned("PRIMARY", sizeof("PRIMARY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PRIMARY_name, &const_PRIMARY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PRIMARY_name);

	zval const_SECONDARY_value;
	ZVAL_LONG(&const_SECONDARY_value, UCOL_SECONDARY);
	crex_string *const_SECONDARY_name = crex_string_init_interned("SECONDARY", sizeof("SECONDARY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SECONDARY_name, &const_SECONDARY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SECONDARY_name);

	zval const_TERTIARY_value;
	ZVAL_LONG(&const_TERTIARY_value, UCOL_TERTIARY);
	crex_string *const_TERTIARY_name = crex_string_init_interned("TERTIARY", sizeof("TERTIARY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_TERTIARY_name, &const_TERTIARY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_TERTIARY_name);

	zval const_DEFAULT_STRENGTH_value;
	ZVAL_LONG(&const_DEFAULT_STRENGTH_value, UCOL_DEFAULT_STRENGTH);
	crex_string *const_DEFAULT_STRENGTH_name = crex_string_init_interned("DEFAULT_STRENGTH", sizeof("DEFAULT_STRENGTH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DEFAULT_STRENGTH_name, &const_DEFAULT_STRENGTH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DEFAULT_STRENGTH_name);

	zval const_QUATERNARY_value;
	ZVAL_LONG(&const_QUATERNARY_value, UCOL_QUATERNARY);
	crex_string *const_QUATERNARY_name = crex_string_init_interned("QUATERNARY", sizeof("QUATERNARY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_QUATERNARY_name, &const_QUATERNARY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_QUATERNARY_name);

	zval const_IDENTICAL_value;
	ZVAL_LONG(&const_IDENTICAL_value, UCOL_IDENTICAL);
	crex_string *const_IDENTICAL_name = crex_string_init_interned("IDENTICAL", sizeof("IDENTICAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_IDENTICAL_name, &const_IDENTICAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_IDENTICAL_name);

	zval const_OFF_value;
	ZVAL_LONG(&const_OFF_value, UCOL_OFF);
	crex_string *const_OFF_name = crex_string_init_interned("OFF", sizeof("OFF") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_OFF_name, &const_OFF_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_OFF_name);

	zval const_ON_value;
	ZVAL_LONG(&const_ON_value, UCOL_ON);
	crex_string *const_ON_name = crex_string_init_interned("ON", sizeof("ON") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ON_name, &const_ON_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ON_name);

	zval const_SHIFTED_value;
	ZVAL_LONG(&const_SHIFTED_value, UCOL_SHIFTED);
	crex_string *const_SHIFTED_name = crex_string_init_interned("SHIFTED", sizeof("SHIFTED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SHIFTED_name, &const_SHIFTED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SHIFTED_name);

	zval const_NON_IGNORABLE_value;
	ZVAL_LONG(&const_NON_IGNORABLE_value, UCOL_NON_IGNORABLE);
	crex_string *const_NON_IGNORABLE_name = crex_string_init_interned("NON_IGNORABLE", sizeof("NON_IGNORABLE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NON_IGNORABLE_name, &const_NON_IGNORABLE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NON_IGNORABLE_name);

	zval const_LOWER_FIRST_value;
	ZVAL_LONG(&const_LOWER_FIRST_value, UCOL_LOWER_FIRST);
	crex_string *const_LOWER_FIRST_name = crex_string_init_interned("LOWER_FIRST", sizeof("LOWER_FIRST") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LOWER_FIRST_name, &const_LOWER_FIRST_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LOWER_FIRST_name);

	zval const_UPPER_FIRST_value;
	ZVAL_LONG(&const_UPPER_FIRST_value, UCOL_UPPER_FIRST);
	crex_string *const_UPPER_FIRST_name = crex_string_init_interned("UPPER_FIRST", sizeof("UPPER_FIRST") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_UPPER_FIRST_name, &const_UPPER_FIRST_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_UPPER_FIRST_name);

	zval const_FRENCH_COLLATION_value;
	ZVAL_LONG(&const_FRENCH_COLLATION_value, UCOL_FRENCH_COLLATION);
	crex_string *const_FRENCH_COLLATION_name = crex_string_init_interned("FRENCH_COLLATION", sizeof("FRENCH_COLLATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FRENCH_COLLATION_name, &const_FRENCH_COLLATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FRENCH_COLLATION_name);

	zval const_ALTERNATE_HANDLING_value;
	ZVAL_LONG(&const_ALTERNATE_HANDLING_value, UCOL_ALTERNATE_HANDLING);
	crex_string *const_ALTERNATE_HANDLING_name = crex_string_init_interned("ALTERNATE_HANDLING", sizeof("ALTERNATE_HANDLING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ALTERNATE_HANDLING_name, &const_ALTERNATE_HANDLING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ALTERNATE_HANDLING_name);

	zval const_CASE_FIRST_value;
	ZVAL_LONG(&const_CASE_FIRST_value, UCOL_CASE_FIRST);
	crex_string *const_CASE_FIRST_name = crex_string_init_interned("CASE_FIRST", sizeof("CASE_FIRST") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CASE_FIRST_name, &const_CASE_FIRST_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CASE_FIRST_name);

	zval const_CASE_LEVEL_value;
	ZVAL_LONG(&const_CASE_LEVEL_value, UCOL_CASE_LEVEL);
	crex_string *const_CASE_LEVEL_name = crex_string_init_interned("CASE_LEVEL", sizeof("CASE_LEVEL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CASE_LEVEL_name, &const_CASE_LEVEL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CASE_LEVEL_name);

	zval const_NORMALIZATION_MODE_value;
	ZVAL_LONG(&const_NORMALIZATION_MODE_value, UCOL_NORMALIZATION_MODE);
	crex_string *const_NORMALIZATION_MODE_name = crex_string_init_interned("NORMALIZATION_MODE", sizeof("NORMALIZATION_MODE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NORMALIZATION_MODE_name, &const_NORMALIZATION_MODE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NORMALIZATION_MODE_name);

	zval const_STRENGTH_value;
	ZVAL_LONG(&const_STRENGTH_value, UCOL_STRENGTH);
	crex_string *const_STRENGTH_name = crex_string_init_interned("STRENGTH", sizeof("STRENGTH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_STRENGTH_name, &const_STRENGTH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_STRENGTH_name);

	zval const_HIRAGANA_QUATERNARY_MODE_value;
	ZVAL_LONG(&const_HIRAGANA_QUATERNARY_MODE_value, UCOL_HIRAGANA_QUATERNARY_MODE);
	crex_string *const_HIRAGANA_QUATERNARY_MODE_name = crex_string_init_interned("HIRAGANA_QUATERNARY_MODE", sizeof("HIRAGANA_QUATERNARY_MODE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_HIRAGANA_QUATERNARY_MODE_name, &const_HIRAGANA_QUATERNARY_MODE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_HIRAGANA_QUATERNARY_MODE_name);

	zval const_NUMERIC_COLLATION_value;
	ZVAL_LONG(&const_NUMERIC_COLLATION_value, UCOL_NUMERIC_COLLATION);
	crex_string *const_NUMERIC_COLLATION_name = crex_string_init_interned("NUMERIC_COLLATION", sizeof("NUMERIC_COLLATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NUMERIC_COLLATION_name, &const_NUMERIC_COLLATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NUMERIC_COLLATION_name);

	zval const_SORT_REGULAR_value;
	ZVAL_LONG(&const_SORT_REGULAR_value, COLLATOR_SORT_REGULAR);
	crex_string *const_SORT_REGULAR_name = crex_string_init_interned("SORT_REGULAR", sizeof("SORT_REGULAR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SORT_REGULAR_name, &const_SORT_REGULAR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SORT_REGULAR_name);

	zval const_SORT_STRING_value;
	ZVAL_LONG(&const_SORT_STRING_value, COLLATOR_SORT_STRING);
	crex_string *const_SORT_STRING_name = crex_string_init_interned("SORT_STRING", sizeof("SORT_STRING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SORT_STRING_name, &const_SORT_STRING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SORT_STRING_name);

	zval const_SORT_NUMERIC_value;
	ZVAL_LONG(&const_SORT_NUMERIC_value, COLLATOR_SORT_NUMERIC);
	crex_string *const_SORT_NUMERIC_name = crex_string_init_interned("SORT_NUMERIC", sizeof("SORT_NUMERIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SORT_NUMERIC_name, &const_SORT_NUMERIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SORT_NUMERIC_name);

	return class_entry;
}
