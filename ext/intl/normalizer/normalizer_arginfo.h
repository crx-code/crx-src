/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 451d8875edd23e737c147fb9aab37aa8220d731d */

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_Normalizer_normalize, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, form, IS_LONG, 0, "Normalizer::FORM_C")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Normalizer_isNormalized, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, form, IS_LONG, 0, "Normalizer::FORM_C")
CREX_END_ARG_INFO()

#if U_ICU_VERSION_MAJOR_NUM >= 56
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Normalizer_getRawDecomposition, 0, 1, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, form, IS_LONG, 0, "Normalizer::FORM_C")
CREX_END_ARG_INFO()
#endif


CREX_FUNCTION(normalizer_normalize);
CREX_FUNCTION(normalizer_is_normalized);
#if U_ICU_VERSION_MAJOR_NUM >= 56
CREX_FUNCTION(normalizer_get_raw_decomposition);
#endif


static const crex_function_entry class_Normalizer_methods[] = {
	CREX_ME_MAPPING(normalize, normalizer_normalize, arginfo_class_Normalizer_normalize, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(isNormalized, normalizer_is_normalized, arginfo_class_Normalizer_isNormalized, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
#if U_ICU_VERSION_MAJOR_NUM >= 56
	CREX_ME_MAPPING(getRawDecomposition, normalizer_get_raw_decomposition, arginfo_class_Normalizer_getRawDecomposition, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
#endif
	CREX_FE_END
};

static crex_class_entry *register_class_Normalizer(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Normalizer", class_Normalizer_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	zval const_FORM_D_value;
	ZVAL_LONG(&const_FORM_D_value, NORMALIZER_FORM_D);
	crex_string *const_FORM_D_name = crex_string_init_interned("FORM_D", sizeof("FORM_D") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FORM_D_name, &const_FORM_D_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FORM_D_name);

	zval const_NFD_value;
	ZVAL_LONG(&const_NFD_value, NORMALIZER_NFD);
	crex_string *const_NFD_name = crex_string_init_interned("NFD", sizeof("NFD") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NFD_name, &const_NFD_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NFD_name);

	zval const_FORM_KD_value;
	ZVAL_LONG(&const_FORM_KD_value, NORMALIZER_FORM_KD);
	crex_string *const_FORM_KD_name = crex_string_init_interned("FORM_KD", sizeof("FORM_KD") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FORM_KD_name, &const_FORM_KD_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FORM_KD_name);

	zval const_NFKD_value;
	ZVAL_LONG(&const_NFKD_value, NORMALIZER_NFKD);
	crex_string *const_NFKD_name = crex_string_init_interned("NFKD", sizeof("NFKD") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NFKD_name, &const_NFKD_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NFKD_name);

	zval const_FORM_C_value;
	ZVAL_LONG(&const_FORM_C_value, NORMALIZER_FORM_C);
	crex_string *const_FORM_C_name = crex_string_init_interned("FORM_C", sizeof("FORM_C") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FORM_C_name, &const_FORM_C_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FORM_C_name);

	zval const_NFC_value;
	ZVAL_LONG(&const_NFC_value, NORMALIZER_NFC);
	crex_string *const_NFC_name = crex_string_init_interned("NFC", sizeof("NFC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NFC_name, &const_NFC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NFC_name);

	zval const_FORM_KC_value;
	ZVAL_LONG(&const_FORM_KC_value, NORMALIZER_FORM_KC);
	crex_string *const_FORM_KC_name = crex_string_init_interned("FORM_KC", sizeof("FORM_KC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FORM_KC_name, &const_FORM_KC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FORM_KC_name);

	zval const_NFKC_value;
	ZVAL_LONG(&const_NFKC_value, NORMALIZER_NFKC);
	crex_string *const_NFKC_name = crex_string_init_interned("NFKC", sizeof("NFKC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NFKC_name, &const_NFKC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NFKC_name);
#if U_ICU_VERSION_MAJOR_NUM >= 56

	zval const_FORM_KC_CF_value;
	ZVAL_LONG(&const_FORM_KC_CF_value, NORMALIZER_FORM_KC_CF);
	crex_string *const_FORM_KC_CF_name = crex_string_init_interned("FORM_KC_CF", sizeof("FORM_KC_CF") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FORM_KC_CF_name, &const_FORM_KC_CF_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FORM_KC_CF_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 56

	zval const_NFKC_CF_value;
	ZVAL_LONG(&const_NFKC_CF_value, NORMALIZER_NFKC_CF);
	crex_string *const_NFKC_CF_name = crex_string_init_interned("NFKC_CF", sizeof("NFKC_CF") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NFKC_CF_name, &const_NFKC_CF_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NFKC_CF_name);
#endif

	return class_entry;
}
