/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: ada449c81e2e3657dbbff7b77ce2410f9f5b1a9a */

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Locale_getDefault, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_Locale_setDefault, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Locale_getPrimaryLanguage, 0, 1, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_Locale_getScript arginfo_class_Locale_getPrimaryLanguage

#define arginfo_class_Locale_getRegion arginfo_class_Locale_getPrimaryLanguage

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_Locale_getKeywords, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE|MAY_BE_NULL)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_Locale_getDisplayScript, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, displayLocale, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_class_Locale_getDisplayRegion arginfo_class_Locale_getDisplayScript

#define arginfo_class_Locale_getDisplayName arginfo_class_Locale_getDisplayScript

#define arginfo_class_Locale_getDisplayLanguage arginfo_class_Locale_getDisplayScript

#define arginfo_class_Locale_getDisplayVariant arginfo_class_Locale_getDisplayScript

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_Locale_composeLocale, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, subtags, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Locale_parseLocale, 0, 1, IS_ARRAY, 1)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_Locale_getAllVariants arginfo_class_Locale_parseLocale

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Locale_filterMatches, 0, 2, _IS_BOOL, 1)
	CREX_ARG_TYPE_INFO(0, languageTag, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, canonicalize, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Locale_lookup, 0, 2, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, languageTag, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, canonicalize, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, defaultLocale, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_class_Locale_canonicalize arginfo_class_Locale_getPrimaryLanguage

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_Locale_acceptFromHttp, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, header, IS_STRING, 0)
CREX_END_ARG_INFO()


CREX_FUNCTION(locale_get_default);
CREX_FUNCTION(locale_set_default);
CREX_FUNCTION(locale_get_primary_language);
CREX_FUNCTION(locale_get_script);
CREX_FUNCTION(locale_get_region);
CREX_FUNCTION(locale_get_keywords);
CREX_FUNCTION(locale_get_display_script);
CREX_FUNCTION(locale_get_display_region);
CREX_FUNCTION(locale_get_display_name);
CREX_FUNCTION(locale_get_display_language);
CREX_FUNCTION(locale_get_display_variant);
CREX_FUNCTION(locale_compose);
CREX_FUNCTION(locale_parse);
CREX_FUNCTION(locale_get_all_variants);
CREX_FUNCTION(locale_filter_matches);
CREX_FUNCTION(locale_lookup);
CREX_FUNCTION(locale_canonicalize);
CREX_FUNCTION(locale_accept_from_http);


static const crex_function_entry class_Locale_methods[] = {
	CREX_ME_MAPPING(getDefault, locale_get_default, arginfo_class_Locale_getDefault, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(setDefault, locale_set_default, arginfo_class_Locale_setDefault, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getPrimaryLanguage, locale_get_primary_language, arginfo_class_Locale_getPrimaryLanguage, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getScript, locale_get_script, arginfo_class_Locale_getScript, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getRegion, locale_get_region, arginfo_class_Locale_getRegion, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getKeywords, locale_get_keywords, arginfo_class_Locale_getKeywords, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getDisplayScript, locale_get_display_script, arginfo_class_Locale_getDisplayScript, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getDisplayRegion, locale_get_display_region, arginfo_class_Locale_getDisplayRegion, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getDisplayName, locale_get_display_name, arginfo_class_Locale_getDisplayName, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getDisplayLanguage, locale_get_display_language, arginfo_class_Locale_getDisplayLanguage, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getDisplayVariant, locale_get_display_variant, arginfo_class_Locale_getDisplayVariant, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(composeLocale, locale_compose, arginfo_class_Locale_composeLocale, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(parseLocale, locale_parse, arginfo_class_Locale_parseLocale, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getAllVariants, locale_get_all_variants, arginfo_class_Locale_getAllVariants, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(filterMatches, locale_filter_matches, arginfo_class_Locale_filterMatches, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(lookup, locale_lookup, arginfo_class_Locale_lookup, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(canonicalize, locale_canonicalize, arginfo_class_Locale_canonicalize, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(acceptFromHttp, locale_accept_from_http, arginfo_class_Locale_acceptFromHttp, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_FE_END
};

static crex_class_entry *register_class_Locale(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Locale", class_Locale_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	zval const_ACTUAL_LOCALE_value;
	ZVAL_LONG(&const_ACTUAL_LOCALE_value, ULOC_ACTUAL_LOCALE);
	crex_string *const_ACTUAL_LOCALE_name = crex_string_init_interned("ACTUAL_LOCALE", sizeof("ACTUAL_LOCALE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ACTUAL_LOCALE_name, &const_ACTUAL_LOCALE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ACTUAL_LOCALE_name);

	zval const_VALID_LOCALE_value;
	ZVAL_LONG(&const_VALID_LOCALE_value, ULOC_VALID_LOCALE);
	crex_string *const_VALID_LOCALE_name = crex_string_init_interned("VALID_LOCALE", sizeof("VALID_LOCALE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_VALID_LOCALE_name, &const_VALID_LOCALE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_VALID_LOCALE_name);

	zval const_DEFAULT_LOCALE_value;
	ZVAL_NULL(&const_DEFAULT_LOCALE_value);
	crex_string *const_DEFAULT_LOCALE_name = crex_string_init_interned("DEFAULT_LOCALE", sizeof("DEFAULT_LOCALE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DEFAULT_LOCALE_name, &const_DEFAULT_LOCALE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DEFAULT_LOCALE_name);

	zval const_LANG_TAG_value;
	crex_string *const_LANG_TAG_value_str = crex_string_init(LOC_LANG_TAG, strlen(LOC_LANG_TAG), 1);
	ZVAL_STR(&const_LANG_TAG_value, const_LANG_TAG_value_str);
	crex_string *const_LANG_TAG_name = crex_string_init_interned("LANG_TAG", sizeof("LANG_TAG") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LANG_TAG_name, &const_LANG_TAG_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LANG_TAG_name);

	zval const_EXTLANG_TAG_value;
	crex_string *const_EXTLANG_TAG_value_str = crex_string_init(LOC_EXTLANG_TAG, strlen(LOC_EXTLANG_TAG), 1);
	ZVAL_STR(&const_EXTLANG_TAG_value, const_EXTLANG_TAG_value_str);
	crex_string *const_EXTLANG_TAG_name = crex_string_init_interned("EXTLANG_TAG", sizeof("EXTLANG_TAG") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_EXTLANG_TAG_name, &const_EXTLANG_TAG_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_EXTLANG_TAG_name);

	zval const_SCRIPT_TAG_value;
	crex_string *const_SCRIPT_TAG_value_str = crex_string_init(LOC_SCRIPT_TAG, strlen(LOC_SCRIPT_TAG), 1);
	ZVAL_STR(&const_SCRIPT_TAG_value, const_SCRIPT_TAG_value_str);
	crex_string *const_SCRIPT_TAG_name = crex_string_init_interned("SCRIPT_TAG", sizeof("SCRIPT_TAG") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SCRIPT_TAG_name, &const_SCRIPT_TAG_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SCRIPT_TAG_name);

	zval const_REGION_TAG_value;
	crex_string *const_REGION_TAG_value_str = crex_string_init(LOC_REGION_TAG, strlen(LOC_REGION_TAG), 1);
	ZVAL_STR(&const_REGION_TAG_value, const_REGION_TAG_value_str);
	crex_string *const_REGION_TAG_name = crex_string_init_interned("REGION_TAG", sizeof("REGION_TAG") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_REGION_TAG_name, &const_REGION_TAG_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_REGION_TAG_name);

	zval const_VARIANT_TAG_value;
	crex_string *const_VARIANT_TAG_value_str = crex_string_init(LOC_VARIANT_TAG, strlen(LOC_VARIANT_TAG), 1);
	ZVAL_STR(&const_VARIANT_TAG_value, const_VARIANT_TAG_value_str);
	crex_string *const_VARIANT_TAG_name = crex_string_init_interned("VARIANT_TAG", sizeof("VARIANT_TAG") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_VARIANT_TAG_name, &const_VARIANT_TAG_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_VARIANT_TAG_name);

	zval const_GRANDFATHERED_LANG_TAG_value;
	crex_string *const_GRANDFATHERED_LANG_TAG_value_str = crex_string_init(LOC_GRANDFATHERED_LANG_TAG, strlen(LOC_GRANDFATHERED_LANG_TAG), 1);
	ZVAL_STR(&const_GRANDFATHERED_LANG_TAG_value, const_GRANDFATHERED_LANG_TAG_value_str);
	crex_string *const_GRANDFATHERED_LANG_TAG_name = crex_string_init_interned("GRANDFATHERED_LANG_TAG", sizeof("GRANDFATHERED_LANG_TAG") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_GRANDFATHERED_LANG_TAG_name, &const_GRANDFATHERED_LANG_TAG_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_GRANDFATHERED_LANG_TAG_name);

	zval const_PRIVATE_TAG_value;
	crex_string *const_PRIVATE_TAG_value_str = crex_string_init(LOC_PRIVATE_TAG, strlen(LOC_PRIVATE_TAG), 1);
	ZVAL_STR(&const_PRIVATE_TAG_value, const_PRIVATE_TAG_value_str);
	crex_string *const_PRIVATE_TAG_name = crex_string_init_interned("PRIVATE_TAG", sizeof("PRIVATE_TAG") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PRIVATE_TAG_name, &const_PRIVATE_TAG_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PRIVATE_TAG_name);

	return class_entry;
}
