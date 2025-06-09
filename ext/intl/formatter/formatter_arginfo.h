/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 3b050eaf6f2f54e3726b04a17d40b06ad610a724 */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_NumberFormatter___main, 0, 0, 2)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, style, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pattern, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_NumberFormatter_create, 0, 2, NumberFormatter, 1)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, style, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pattern, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_NumberFormatter_format, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_MASK(0, num, MAY_BE_LONG|MAY_BE_DOUBLE, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_LONG, 0, "NumberFormatter::TYPE_DEFAULT")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_NumberFormatter_parse, 0, 1, MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_LONG, 0, "NumberFormatter::TYPE_DOUBLE")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, offset, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_NumberFormatter_formatCurrency, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, amount, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, currency, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_NumberFormatter_parseCurrency, 0, 2, MAY_BE_DOUBLE|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_INFO(1, currency)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, offset, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_NumberFormatter_setAttribute, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, attribute, IS_LONG, 0)
	CREX_ARG_TYPE_MASK(0, value, MAY_BE_LONG|MAY_BE_DOUBLE, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_NumberFormatter_getAttribute, 0, 1, MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, attribute, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_NumberFormatter_setTextAttribute, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, attribute, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_NumberFormatter_getTextAttribute, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, attribute, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_NumberFormatter_setSymbol, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, symbol, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_NumberFormatter_getSymbol, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, symbol, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_NumberFormatter_setPattern, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_NumberFormatter_getPattern, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_NumberFormatter_getLocale, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_LONG, 0, "ULOC_ACTUAL_LOCALE")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_NumberFormatter_getErrorCode, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_NumberFormatter_getErrorMessage, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()


CREX_METHOD(NumberFormatter, __main);
CREX_FUNCTION(numfmt_create);
CREX_FUNCTION(numfmt_format);
CREX_FUNCTION(numfmt_parse);
CREX_FUNCTION(numfmt_format_currency);
CREX_FUNCTION(numfmt_parse_currency);
CREX_FUNCTION(numfmt_set_attribute);
CREX_FUNCTION(numfmt_get_attribute);
CREX_FUNCTION(numfmt_set_text_attribute);
CREX_FUNCTION(numfmt_get_text_attribute);
CREX_FUNCTION(numfmt_set_symbol);
CREX_FUNCTION(numfmt_get_symbol);
CREX_FUNCTION(numfmt_set_pattern);
CREX_FUNCTION(numfmt_get_pattern);
CREX_FUNCTION(numfmt_get_locale);
CREX_FUNCTION(numfmt_get_error_code);
CREX_FUNCTION(numfmt_get_error_message);


static const crex_function_entry class_NumberFormatter_methods[] = {
	CREX_ME(NumberFormatter, __main, arginfo_class_NumberFormatter___main, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(create, numfmt_create, arginfo_class_NumberFormatter_create, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(format, numfmt_format, arginfo_class_NumberFormatter_format, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(parse, numfmt_parse, arginfo_class_NumberFormatter_parse, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(formatCurrency, numfmt_format_currency, arginfo_class_NumberFormatter_formatCurrency, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(parseCurrency, numfmt_parse_currency, arginfo_class_NumberFormatter_parseCurrency, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setAttribute, numfmt_set_attribute, arginfo_class_NumberFormatter_setAttribute, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getAttribute, numfmt_get_attribute, arginfo_class_NumberFormatter_getAttribute, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setTextAttribute, numfmt_set_text_attribute, arginfo_class_NumberFormatter_setTextAttribute, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getTextAttribute, numfmt_get_text_attribute, arginfo_class_NumberFormatter_getTextAttribute, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setSymbol, numfmt_set_symbol, arginfo_class_NumberFormatter_setSymbol, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getSymbol, numfmt_get_symbol, arginfo_class_NumberFormatter_getSymbol, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setPattern, numfmt_set_pattern, arginfo_class_NumberFormatter_setPattern, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getPattern, numfmt_get_pattern, arginfo_class_NumberFormatter_getPattern, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getLocale, numfmt_get_locale, arginfo_class_NumberFormatter_getLocale, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getErrorCode, numfmt_get_error_code, arginfo_class_NumberFormatter_getErrorCode, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getErrorMessage, numfmt_get_error_message, arginfo_class_NumberFormatter_getErrorMessage, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_NumberFormatter(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "NumberFormatter", class_NumberFormatter_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;

	zval const_PATTERN_DECIMAL_value;
	ZVAL_LONG(&const_PATTERN_DECIMAL_value, UNUM_PATTERN_DECIMAL);
	crex_string *const_PATTERN_DECIMAL_name = crex_string_init_interned("PATTERN_DECIMAL", sizeof("PATTERN_DECIMAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PATTERN_DECIMAL_name, &const_PATTERN_DECIMAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PATTERN_DECIMAL_name);

	zval const_DECIMAL_value;
	ZVAL_LONG(&const_DECIMAL_value, UNUM_DECIMAL);
	crex_string *const_DECIMAL_name = crex_string_init_interned("DECIMAL", sizeof("DECIMAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DECIMAL_name, &const_DECIMAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DECIMAL_name);

	zval const_CURRENCY_value;
	ZVAL_LONG(&const_CURRENCY_value, UNUM_CURRENCY);
	crex_string *const_CURRENCY_name = crex_string_init_interned("CURRENCY", sizeof("CURRENCY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CURRENCY_name, &const_CURRENCY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CURRENCY_name);

	zval const_PERCENT_value;
	ZVAL_LONG(&const_PERCENT_value, UNUM_PERCENT);
	crex_string *const_PERCENT_name = crex_string_init_interned("PERCENT", sizeof("PERCENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PERCENT_name, &const_PERCENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PERCENT_name);

	zval const_SCIENTIFIC_value;
	ZVAL_LONG(&const_SCIENTIFIC_value, UNUM_SCIENTIFIC);
	crex_string *const_SCIENTIFIC_name = crex_string_init_interned("SCIENTIFIC", sizeof("SCIENTIFIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SCIENTIFIC_name, &const_SCIENTIFIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SCIENTIFIC_name);

	zval const_SPELLOUT_value;
	ZVAL_LONG(&const_SPELLOUT_value, UNUM_SPELLOUT);
	crex_string *const_SPELLOUT_name = crex_string_init_interned("SPELLOUT", sizeof("SPELLOUT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SPELLOUT_name, &const_SPELLOUT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SPELLOUT_name);

	zval const_ORDINAL_value;
	ZVAL_LONG(&const_ORDINAL_value, UNUM_ORDINAL);
	crex_string *const_ORDINAL_name = crex_string_init_interned("ORDINAL", sizeof("ORDINAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ORDINAL_name, &const_ORDINAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ORDINAL_name);

	zval const_DURATION_value;
	ZVAL_LONG(&const_DURATION_value, UNUM_DURATION);
	crex_string *const_DURATION_name = crex_string_init_interned("DURATION", sizeof("DURATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DURATION_name, &const_DURATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DURATION_name);

	zval const_PATTERN_RULEBASED_value;
	ZVAL_LONG(&const_PATTERN_RULEBASED_value, UNUM_PATTERN_RULEBASED);
	crex_string *const_PATTERN_RULEBASED_name = crex_string_init_interned("PATTERN_RULEBASED", sizeof("PATTERN_RULEBASED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PATTERN_RULEBASED_name, &const_PATTERN_RULEBASED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PATTERN_RULEBASED_name);

	zval const_IGNORE_value;
	ZVAL_LONG(&const_IGNORE_value, UNUM_IGNORE);
	crex_string *const_IGNORE_name = crex_string_init_interned("IGNORE", sizeof("IGNORE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_IGNORE_name, &const_IGNORE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_IGNORE_name);
#if U_ICU_VERSION_MAJOR_NUM >= 53

	zval const_CURRENCY_ACCOUNTING_value;
	ZVAL_LONG(&const_CURRENCY_ACCOUNTING_value, UNUM_CURRENCY_ACCOUNTING);
	crex_string *const_CURRENCY_ACCOUNTING_name = crex_string_init_interned("CURRENCY_ACCOUNTING", sizeof("CURRENCY_ACCOUNTING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CURRENCY_ACCOUNTING_name, &const_CURRENCY_ACCOUNTING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CURRENCY_ACCOUNTING_name);
#endif

	zval const_DEFAULT_STYLE_value;
	ZVAL_LONG(&const_DEFAULT_STYLE_value, UNUM_DEFAULT);
	crex_string *const_DEFAULT_STYLE_name = crex_string_init_interned("DEFAULT_STYLE", sizeof("DEFAULT_STYLE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DEFAULT_STYLE_name, &const_DEFAULT_STYLE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DEFAULT_STYLE_name);

	zval const_ROUND_CEILING_value;
	ZVAL_LONG(&const_ROUND_CEILING_value, UNUM_ROUND_CEILING);
	crex_string *const_ROUND_CEILING_name = crex_string_init_interned("ROUND_CEILING", sizeof("ROUND_CEILING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ROUND_CEILING_name, &const_ROUND_CEILING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ROUND_CEILING_name);

	zval const_ROUND_FLOOR_value;
	ZVAL_LONG(&const_ROUND_FLOOR_value, UNUM_ROUND_FLOOR);
	crex_string *const_ROUND_FLOOR_name = crex_string_init_interned("ROUND_FLOOR", sizeof("ROUND_FLOOR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ROUND_FLOOR_name, &const_ROUND_FLOOR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ROUND_FLOOR_name);

	zval const_ROUND_DOWN_value;
	ZVAL_LONG(&const_ROUND_DOWN_value, UNUM_ROUND_DOWN);
	crex_string *const_ROUND_DOWN_name = crex_string_init_interned("ROUND_DOWN", sizeof("ROUND_DOWN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ROUND_DOWN_name, &const_ROUND_DOWN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ROUND_DOWN_name);

	zval const_ROUND_UP_value;
	ZVAL_LONG(&const_ROUND_UP_value, UNUM_ROUND_UP);
	crex_string *const_ROUND_UP_name = crex_string_init_interned("ROUND_UP", sizeof("ROUND_UP") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ROUND_UP_name, &const_ROUND_UP_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ROUND_UP_name);

	zval const_ROUND_HALFEVEN_value;
	ZVAL_LONG(&const_ROUND_HALFEVEN_value, UNUM_ROUND_HALFEVEN);
	crex_string *const_ROUND_HALFEVEN_name = crex_string_init_interned("ROUND_HALFEVEN", sizeof("ROUND_HALFEVEN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ROUND_HALFEVEN_name, &const_ROUND_HALFEVEN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ROUND_HALFEVEN_name);

	zval const_ROUND_HALFDOWN_value;
	ZVAL_LONG(&const_ROUND_HALFDOWN_value, UNUM_ROUND_HALFDOWN);
	crex_string *const_ROUND_HALFDOWN_name = crex_string_init_interned("ROUND_HALFDOWN", sizeof("ROUND_HALFDOWN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ROUND_HALFDOWN_name, &const_ROUND_HALFDOWN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ROUND_HALFDOWN_name);

	zval const_ROUND_HALFUP_value;
	ZVAL_LONG(&const_ROUND_HALFUP_value, UNUM_ROUND_HALFUP);
	crex_string *const_ROUND_HALFUP_name = crex_string_init_interned("ROUND_HALFUP", sizeof("ROUND_HALFUP") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ROUND_HALFUP_name, &const_ROUND_HALFUP_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ROUND_HALFUP_name);

	zval const_PAD_BEFORE_PREFIX_value;
	ZVAL_LONG(&const_PAD_BEFORE_PREFIX_value, UNUM_PAD_BEFORE_PREFIX);
	crex_string *const_PAD_BEFORE_PREFIX_name = crex_string_init_interned("PAD_BEFORE_PREFIX", sizeof("PAD_BEFORE_PREFIX") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PAD_BEFORE_PREFIX_name, &const_PAD_BEFORE_PREFIX_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PAD_BEFORE_PREFIX_name);

	zval const_PAD_AFTER_PREFIX_value;
	ZVAL_LONG(&const_PAD_AFTER_PREFIX_value, UNUM_PAD_AFTER_PREFIX);
	crex_string *const_PAD_AFTER_PREFIX_name = crex_string_init_interned("PAD_AFTER_PREFIX", sizeof("PAD_AFTER_PREFIX") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PAD_AFTER_PREFIX_name, &const_PAD_AFTER_PREFIX_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PAD_AFTER_PREFIX_name);

	zval const_PAD_BEFORE_SUFFIX_value;
	ZVAL_LONG(&const_PAD_BEFORE_SUFFIX_value, UNUM_PAD_BEFORE_SUFFIX);
	crex_string *const_PAD_BEFORE_SUFFIX_name = crex_string_init_interned("PAD_BEFORE_SUFFIX", sizeof("PAD_BEFORE_SUFFIX") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PAD_BEFORE_SUFFIX_name, &const_PAD_BEFORE_SUFFIX_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PAD_BEFORE_SUFFIX_name);

	zval const_PAD_AFTER_SUFFIX_value;
	ZVAL_LONG(&const_PAD_AFTER_SUFFIX_value, UNUM_PAD_AFTER_SUFFIX);
	crex_string *const_PAD_AFTER_SUFFIX_name = crex_string_init_interned("PAD_AFTER_SUFFIX", sizeof("PAD_AFTER_SUFFIX") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PAD_AFTER_SUFFIX_name, &const_PAD_AFTER_SUFFIX_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PAD_AFTER_SUFFIX_name);

	zval const_PARSE_INT_ONLY_value;
	ZVAL_LONG(&const_PARSE_INT_ONLY_value, UNUM_PARSE_INT_ONLY);
	crex_string *const_PARSE_INT_ONLY_name = crex_string_init_interned("PARSE_INT_ONLY", sizeof("PARSE_INT_ONLY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PARSE_INT_ONLY_name, &const_PARSE_INT_ONLY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PARSE_INT_ONLY_name);

	zval const_GROUPING_USED_value;
	ZVAL_LONG(&const_GROUPING_USED_value, UNUM_GROUPING_USED);
	crex_string *const_GROUPING_USED_name = crex_string_init_interned("GROUPING_USED", sizeof("GROUPING_USED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_GROUPING_USED_name, &const_GROUPING_USED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_GROUPING_USED_name);

	zval const_DECIMAL_ALWAYS_SHOWN_value;
	ZVAL_LONG(&const_DECIMAL_ALWAYS_SHOWN_value, UNUM_DECIMAL_ALWAYS_SHOWN);
	crex_string *const_DECIMAL_ALWAYS_SHOWN_name = crex_string_init_interned("DECIMAL_ALWAYS_SHOWN", sizeof("DECIMAL_ALWAYS_SHOWN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DECIMAL_ALWAYS_SHOWN_name, &const_DECIMAL_ALWAYS_SHOWN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DECIMAL_ALWAYS_SHOWN_name);

	zval const_MAX_INTEGER_DIGITS_value;
	ZVAL_LONG(&const_MAX_INTEGER_DIGITS_value, UNUM_MAX_INTEGER_DIGITS);
	crex_string *const_MAX_INTEGER_DIGITS_name = crex_string_init_interned("MAX_INTEGER_DIGITS", sizeof("MAX_INTEGER_DIGITS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_MAX_INTEGER_DIGITS_name, &const_MAX_INTEGER_DIGITS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_MAX_INTEGER_DIGITS_name);

	zval const_MIN_INTEGER_DIGITS_value;
	ZVAL_LONG(&const_MIN_INTEGER_DIGITS_value, UNUM_MIN_INTEGER_DIGITS);
	crex_string *const_MIN_INTEGER_DIGITS_name = crex_string_init_interned("MIN_INTEGER_DIGITS", sizeof("MIN_INTEGER_DIGITS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_MIN_INTEGER_DIGITS_name, &const_MIN_INTEGER_DIGITS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_MIN_INTEGER_DIGITS_name);

	zval const_INTEGER_DIGITS_value;
	ZVAL_LONG(&const_INTEGER_DIGITS_value, UNUM_INTEGER_DIGITS);
	crex_string *const_INTEGER_DIGITS_name = crex_string_init_interned("INTEGER_DIGITS", sizeof("INTEGER_DIGITS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_INTEGER_DIGITS_name, &const_INTEGER_DIGITS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_INTEGER_DIGITS_name);

	zval const_MAX_FRACTION_DIGITS_value;
	ZVAL_LONG(&const_MAX_FRACTION_DIGITS_value, UNUM_MAX_FRACTION_DIGITS);
	crex_string *const_MAX_FRACTION_DIGITS_name = crex_string_init_interned("MAX_FRACTION_DIGITS", sizeof("MAX_FRACTION_DIGITS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_MAX_FRACTION_DIGITS_name, &const_MAX_FRACTION_DIGITS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_MAX_FRACTION_DIGITS_name);

	zval const_MIN_FRACTION_DIGITS_value;
	ZVAL_LONG(&const_MIN_FRACTION_DIGITS_value, UNUM_MIN_FRACTION_DIGITS);
	crex_string *const_MIN_FRACTION_DIGITS_name = crex_string_init_interned("MIN_FRACTION_DIGITS", sizeof("MIN_FRACTION_DIGITS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_MIN_FRACTION_DIGITS_name, &const_MIN_FRACTION_DIGITS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_MIN_FRACTION_DIGITS_name);

	zval const_FRACTION_DIGITS_value;
	ZVAL_LONG(&const_FRACTION_DIGITS_value, UNUM_FRACTION_DIGITS);
	crex_string *const_FRACTION_DIGITS_name = crex_string_init_interned("FRACTION_DIGITS", sizeof("FRACTION_DIGITS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FRACTION_DIGITS_name, &const_FRACTION_DIGITS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FRACTION_DIGITS_name);

	zval const_MULTIPLIER_value;
	ZVAL_LONG(&const_MULTIPLIER_value, UNUM_MULTIPLIER);
	crex_string *const_MULTIPLIER_name = crex_string_init_interned("MULTIPLIER", sizeof("MULTIPLIER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_MULTIPLIER_name, &const_MULTIPLIER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_MULTIPLIER_name);

	zval const_GROUPING_SIZE_value;
	ZVAL_LONG(&const_GROUPING_SIZE_value, UNUM_GROUPING_SIZE);
	crex_string *const_GROUPING_SIZE_name = crex_string_init_interned("GROUPING_SIZE", sizeof("GROUPING_SIZE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_GROUPING_SIZE_name, &const_GROUPING_SIZE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_GROUPING_SIZE_name);

	zval const_ROUNDING_MODE_value;
	ZVAL_LONG(&const_ROUNDING_MODE_value, UNUM_ROUNDING_MODE);
	crex_string *const_ROUNDING_MODE_name = crex_string_init_interned("ROUNDING_MODE", sizeof("ROUNDING_MODE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ROUNDING_MODE_name, &const_ROUNDING_MODE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ROUNDING_MODE_name);

	zval const_ROUNDING_INCREMENT_value;
	ZVAL_LONG(&const_ROUNDING_INCREMENT_value, UNUM_ROUNDING_INCREMENT);
	crex_string *const_ROUNDING_INCREMENT_name = crex_string_init_interned("ROUNDING_INCREMENT", sizeof("ROUNDING_INCREMENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ROUNDING_INCREMENT_name, &const_ROUNDING_INCREMENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ROUNDING_INCREMENT_name);

	zval const_FORMAT_WIDTH_value;
	ZVAL_LONG(&const_FORMAT_WIDTH_value, UNUM_FORMAT_WIDTH);
	crex_string *const_FORMAT_WIDTH_name = crex_string_init_interned("FORMAT_WIDTH", sizeof("FORMAT_WIDTH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FORMAT_WIDTH_name, &const_FORMAT_WIDTH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FORMAT_WIDTH_name);

	zval const_PADDING_POSITION_value;
	ZVAL_LONG(&const_PADDING_POSITION_value, UNUM_PADDING_POSITION);
	crex_string *const_PADDING_POSITION_name = crex_string_init_interned("PADDING_POSITION", sizeof("PADDING_POSITION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PADDING_POSITION_name, &const_PADDING_POSITION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PADDING_POSITION_name);

	zval const_SECONDARY_GROUPING_SIZE_value;
	ZVAL_LONG(&const_SECONDARY_GROUPING_SIZE_value, UNUM_SECONDARY_GROUPING_SIZE);
	crex_string *const_SECONDARY_GROUPING_SIZE_name = crex_string_init_interned("SECONDARY_GROUPING_SIZE", sizeof("SECONDARY_GROUPING_SIZE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SECONDARY_GROUPING_SIZE_name, &const_SECONDARY_GROUPING_SIZE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SECONDARY_GROUPING_SIZE_name);

	zval const_SIGNIFICANT_DIGITS_USED_value;
	ZVAL_LONG(&const_SIGNIFICANT_DIGITS_USED_value, UNUM_SIGNIFICANT_DIGITS_USED);
	crex_string *const_SIGNIFICANT_DIGITS_USED_name = crex_string_init_interned("SIGNIFICANT_DIGITS_USED", sizeof("SIGNIFICANT_DIGITS_USED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SIGNIFICANT_DIGITS_USED_name, &const_SIGNIFICANT_DIGITS_USED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SIGNIFICANT_DIGITS_USED_name);

	zval const_MIN_SIGNIFICANT_DIGITS_value;
	ZVAL_LONG(&const_MIN_SIGNIFICANT_DIGITS_value, UNUM_MIN_SIGNIFICANT_DIGITS);
	crex_string *const_MIN_SIGNIFICANT_DIGITS_name = crex_string_init_interned("MIN_SIGNIFICANT_DIGITS", sizeof("MIN_SIGNIFICANT_DIGITS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_MIN_SIGNIFICANT_DIGITS_name, &const_MIN_SIGNIFICANT_DIGITS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_MIN_SIGNIFICANT_DIGITS_name);

	zval const_MAX_SIGNIFICANT_DIGITS_value;
	ZVAL_LONG(&const_MAX_SIGNIFICANT_DIGITS_value, UNUM_MAX_SIGNIFICANT_DIGITS);
	crex_string *const_MAX_SIGNIFICANT_DIGITS_name = crex_string_init_interned("MAX_SIGNIFICANT_DIGITS", sizeof("MAX_SIGNIFICANT_DIGITS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_MAX_SIGNIFICANT_DIGITS_name, &const_MAX_SIGNIFICANT_DIGITS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_MAX_SIGNIFICANT_DIGITS_name);

	zval const_LENIENT_PARSE_value;
	ZVAL_LONG(&const_LENIENT_PARSE_value, UNUM_LENIENT_PARSE);
	crex_string *const_LENIENT_PARSE_name = crex_string_init_interned("LENIENT_PARSE", sizeof("LENIENT_PARSE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LENIENT_PARSE_name, &const_LENIENT_PARSE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LENIENT_PARSE_name);

	zval const_POSITIVE_PREFIX_value;
	ZVAL_LONG(&const_POSITIVE_PREFIX_value, UNUM_POSITIVE_PREFIX);
	crex_string *const_POSITIVE_PREFIX_name = crex_string_init_interned("POSITIVE_PREFIX", sizeof("POSITIVE_PREFIX") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_POSITIVE_PREFIX_name, &const_POSITIVE_PREFIX_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_POSITIVE_PREFIX_name);

	zval const_POSITIVE_SUFFIX_value;
	ZVAL_LONG(&const_POSITIVE_SUFFIX_value, UNUM_POSITIVE_SUFFIX);
	crex_string *const_POSITIVE_SUFFIX_name = crex_string_init_interned("POSITIVE_SUFFIX", sizeof("POSITIVE_SUFFIX") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_POSITIVE_SUFFIX_name, &const_POSITIVE_SUFFIX_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_POSITIVE_SUFFIX_name);

	zval const_NEGATIVE_PREFIX_value;
	ZVAL_LONG(&const_NEGATIVE_PREFIX_value, UNUM_NEGATIVE_PREFIX);
	crex_string *const_NEGATIVE_PREFIX_name = crex_string_init_interned("NEGATIVE_PREFIX", sizeof("NEGATIVE_PREFIX") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NEGATIVE_PREFIX_name, &const_NEGATIVE_PREFIX_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NEGATIVE_PREFIX_name);

	zval const_NEGATIVE_SUFFIX_value;
	ZVAL_LONG(&const_NEGATIVE_SUFFIX_value, UNUM_NEGATIVE_SUFFIX);
	crex_string *const_NEGATIVE_SUFFIX_name = crex_string_init_interned("NEGATIVE_SUFFIX", sizeof("NEGATIVE_SUFFIX") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NEGATIVE_SUFFIX_name, &const_NEGATIVE_SUFFIX_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NEGATIVE_SUFFIX_name);

	zval const_PADDING_CHARACTER_value;
	ZVAL_LONG(&const_PADDING_CHARACTER_value, UNUM_PADDING_CHARACTER);
	crex_string *const_PADDING_CHARACTER_name = crex_string_init_interned("PADDING_CHARACTER", sizeof("PADDING_CHARACTER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PADDING_CHARACTER_name, &const_PADDING_CHARACTER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PADDING_CHARACTER_name);

	zval const_CURRENCY_CODE_value;
	ZVAL_LONG(&const_CURRENCY_CODE_value, UNUM_CURRENCY_CODE);
	crex_string *const_CURRENCY_CODE_name = crex_string_init_interned("CURRENCY_CODE", sizeof("CURRENCY_CODE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CURRENCY_CODE_name, &const_CURRENCY_CODE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CURRENCY_CODE_name);

	zval const_DEFAULT_RULESET_value;
	ZVAL_LONG(&const_DEFAULT_RULESET_value, UNUM_DEFAULT_RULESET);
	crex_string *const_DEFAULT_RULESET_name = crex_string_init_interned("DEFAULT_RULESET", sizeof("DEFAULT_RULESET") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DEFAULT_RULESET_name, &const_DEFAULT_RULESET_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DEFAULT_RULESET_name);

	zval const_PUBLIC_RULESETS_value;
	ZVAL_LONG(&const_PUBLIC_RULESETS_value, UNUM_PUBLIC_RULESETS);
	crex_string *const_PUBLIC_RULESETS_name = crex_string_init_interned("PUBLIC_RULESETS", sizeof("PUBLIC_RULESETS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PUBLIC_RULESETS_name, &const_PUBLIC_RULESETS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PUBLIC_RULESETS_name);

	zval const_DECIMAL_SEPARATOR_SYMBOL_value;
	ZVAL_LONG(&const_DECIMAL_SEPARATOR_SYMBOL_value, UNUM_DECIMAL_SEPARATOR_SYMBOL);
	crex_string *const_DECIMAL_SEPARATOR_SYMBOL_name = crex_string_init_interned("DECIMAL_SEPARATOR_SYMBOL", sizeof("DECIMAL_SEPARATOR_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DECIMAL_SEPARATOR_SYMBOL_name, &const_DECIMAL_SEPARATOR_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DECIMAL_SEPARATOR_SYMBOL_name);

	zval const_GROUPING_SEPARATOR_SYMBOL_value;
	ZVAL_LONG(&const_GROUPING_SEPARATOR_SYMBOL_value, UNUM_GROUPING_SEPARATOR_SYMBOL);
	crex_string *const_GROUPING_SEPARATOR_SYMBOL_name = crex_string_init_interned("GROUPING_SEPARATOR_SYMBOL", sizeof("GROUPING_SEPARATOR_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_GROUPING_SEPARATOR_SYMBOL_name, &const_GROUPING_SEPARATOR_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_GROUPING_SEPARATOR_SYMBOL_name);

	zval const_PATTERN_SEPARATOR_SYMBOL_value;
	ZVAL_LONG(&const_PATTERN_SEPARATOR_SYMBOL_value, UNUM_PATTERN_SEPARATOR_SYMBOL);
	crex_string *const_PATTERN_SEPARATOR_SYMBOL_name = crex_string_init_interned("PATTERN_SEPARATOR_SYMBOL", sizeof("PATTERN_SEPARATOR_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PATTERN_SEPARATOR_SYMBOL_name, &const_PATTERN_SEPARATOR_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PATTERN_SEPARATOR_SYMBOL_name);

	zval const_PERCENT_SYMBOL_value;
	ZVAL_LONG(&const_PERCENT_SYMBOL_value, UNUM_PERCENT_SYMBOL);
	crex_string *const_PERCENT_SYMBOL_name = crex_string_init_interned("PERCENT_SYMBOL", sizeof("PERCENT_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PERCENT_SYMBOL_name, &const_PERCENT_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PERCENT_SYMBOL_name);

	zval const_ZERO_DIGIT_SYMBOL_value;
	ZVAL_LONG(&const_ZERO_DIGIT_SYMBOL_value, UNUM_ZERO_DIGIT_SYMBOL);
	crex_string *const_ZERO_DIGIT_SYMBOL_name = crex_string_init_interned("ZERO_DIGIT_SYMBOL", sizeof("ZERO_DIGIT_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ZERO_DIGIT_SYMBOL_name, &const_ZERO_DIGIT_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ZERO_DIGIT_SYMBOL_name);

	zval const_DIGIT_SYMBOL_value;
	ZVAL_LONG(&const_DIGIT_SYMBOL_value, UNUM_DIGIT_SYMBOL);
	crex_string *const_DIGIT_SYMBOL_name = crex_string_init_interned("DIGIT_SYMBOL", sizeof("DIGIT_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DIGIT_SYMBOL_name, &const_DIGIT_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DIGIT_SYMBOL_name);

	zval const_MINUS_SIGN_SYMBOL_value;
	ZVAL_LONG(&const_MINUS_SIGN_SYMBOL_value, UNUM_MINUS_SIGN_SYMBOL);
	crex_string *const_MINUS_SIGN_SYMBOL_name = crex_string_init_interned("MINUS_SIGN_SYMBOL", sizeof("MINUS_SIGN_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_MINUS_SIGN_SYMBOL_name, &const_MINUS_SIGN_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_MINUS_SIGN_SYMBOL_name);

	zval const_PLUS_SIGN_SYMBOL_value;
	ZVAL_LONG(&const_PLUS_SIGN_SYMBOL_value, UNUM_PLUS_SIGN_SYMBOL);
	crex_string *const_PLUS_SIGN_SYMBOL_name = crex_string_init_interned("PLUS_SIGN_SYMBOL", sizeof("PLUS_SIGN_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PLUS_SIGN_SYMBOL_name, &const_PLUS_SIGN_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PLUS_SIGN_SYMBOL_name);

	zval const_CURRENCY_SYMBOL_value;
	ZVAL_LONG(&const_CURRENCY_SYMBOL_value, UNUM_CURRENCY_SYMBOL);
	crex_string *const_CURRENCY_SYMBOL_name = crex_string_init_interned("CURRENCY_SYMBOL", sizeof("CURRENCY_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CURRENCY_SYMBOL_name, &const_CURRENCY_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CURRENCY_SYMBOL_name);

	zval const_INTL_CURRENCY_SYMBOL_value;
	ZVAL_LONG(&const_INTL_CURRENCY_SYMBOL_value, UNUM_INTL_CURRENCY_SYMBOL);
	crex_string *const_INTL_CURRENCY_SYMBOL_name = crex_string_init_interned("INTL_CURRENCY_SYMBOL", sizeof("INTL_CURRENCY_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_INTL_CURRENCY_SYMBOL_name, &const_INTL_CURRENCY_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_INTL_CURRENCY_SYMBOL_name);

	zval const_MONETARY_SEPARATOR_SYMBOL_value;
	ZVAL_LONG(&const_MONETARY_SEPARATOR_SYMBOL_value, UNUM_MONETARY_SEPARATOR_SYMBOL);
	crex_string *const_MONETARY_SEPARATOR_SYMBOL_name = crex_string_init_interned("MONETARY_SEPARATOR_SYMBOL", sizeof("MONETARY_SEPARATOR_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_MONETARY_SEPARATOR_SYMBOL_name, &const_MONETARY_SEPARATOR_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_MONETARY_SEPARATOR_SYMBOL_name);

	zval const_EXPONENTIAL_SYMBOL_value;
	ZVAL_LONG(&const_EXPONENTIAL_SYMBOL_value, UNUM_EXPONENTIAL_SYMBOL);
	crex_string *const_EXPONENTIAL_SYMBOL_name = crex_string_init_interned("EXPONENTIAL_SYMBOL", sizeof("EXPONENTIAL_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_EXPONENTIAL_SYMBOL_name, &const_EXPONENTIAL_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_EXPONENTIAL_SYMBOL_name);

	zval const_PERMILL_SYMBOL_value;
	ZVAL_LONG(&const_PERMILL_SYMBOL_value, UNUM_PERMILL_SYMBOL);
	crex_string *const_PERMILL_SYMBOL_name = crex_string_init_interned("PERMILL_SYMBOL", sizeof("PERMILL_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PERMILL_SYMBOL_name, &const_PERMILL_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PERMILL_SYMBOL_name);

	zval const_PAD_ESCAPE_SYMBOL_value;
	ZVAL_LONG(&const_PAD_ESCAPE_SYMBOL_value, UNUM_PAD_ESCAPE_SYMBOL);
	crex_string *const_PAD_ESCAPE_SYMBOL_name = crex_string_init_interned("PAD_ESCAPE_SYMBOL", sizeof("PAD_ESCAPE_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PAD_ESCAPE_SYMBOL_name, &const_PAD_ESCAPE_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PAD_ESCAPE_SYMBOL_name);

	zval const_INFINITY_SYMBOL_value;
	ZVAL_LONG(&const_INFINITY_SYMBOL_value, UNUM_INFINITY_SYMBOL);
	crex_string *const_INFINITY_SYMBOL_name = crex_string_init_interned("INFINITY_SYMBOL", sizeof("INFINITY_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_INFINITY_SYMBOL_name, &const_INFINITY_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_INFINITY_SYMBOL_name);

	zval const_NAN_SYMBOL_value;
	ZVAL_LONG(&const_NAN_SYMBOL_value, UNUM_NAN_SYMBOL);
	crex_string *const_NAN_SYMBOL_name = crex_string_init_interned("NAN_SYMBOL", sizeof("NAN_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NAN_SYMBOL_name, &const_NAN_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NAN_SYMBOL_name);

	zval const_SIGNIFICANT_DIGIT_SYMBOL_value;
	ZVAL_LONG(&const_SIGNIFICANT_DIGIT_SYMBOL_value, UNUM_SIGNIFICANT_DIGIT_SYMBOL);
	crex_string *const_SIGNIFICANT_DIGIT_SYMBOL_name = crex_string_init_interned("SIGNIFICANT_DIGIT_SYMBOL", sizeof("SIGNIFICANT_DIGIT_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SIGNIFICANT_DIGIT_SYMBOL_name, &const_SIGNIFICANT_DIGIT_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SIGNIFICANT_DIGIT_SYMBOL_name);

	zval const_MONETARY_GROUPING_SEPARATOR_SYMBOL_value;
	ZVAL_LONG(&const_MONETARY_GROUPING_SEPARATOR_SYMBOL_value, UNUM_MONETARY_GROUPING_SEPARATOR_SYMBOL);
	crex_string *const_MONETARY_GROUPING_SEPARATOR_SYMBOL_name = crex_string_init_interned("MONETARY_GROUPING_SEPARATOR_SYMBOL", sizeof("MONETARY_GROUPING_SEPARATOR_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_MONETARY_GROUPING_SEPARATOR_SYMBOL_name, &const_MONETARY_GROUPING_SEPARATOR_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_MONETARY_GROUPING_SEPARATOR_SYMBOL_name);

	zval const_TYPE_DEFAULT_value;
	ZVAL_LONG(&const_TYPE_DEFAULT_value, FORMAT_TYPE_DEFAULT);
	crex_string *const_TYPE_DEFAULT_name = crex_string_init_interned("TYPE_DEFAULT", sizeof("TYPE_DEFAULT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_TYPE_DEFAULT_name, &const_TYPE_DEFAULT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_TYPE_DEFAULT_name);

	zval const_TYPE_INT32_value;
	ZVAL_LONG(&const_TYPE_INT32_value, FORMAT_TYPE_INT32);
	crex_string *const_TYPE_INT32_name = crex_string_init_interned("TYPE_INT32", sizeof("TYPE_INT32") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_TYPE_INT32_name, &const_TYPE_INT32_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_TYPE_INT32_name);

	zval const_TYPE_INT64_value;
	ZVAL_LONG(&const_TYPE_INT64_value, FORMAT_TYPE_INT64);
	crex_string *const_TYPE_INT64_name = crex_string_init_interned("TYPE_INT64", sizeof("TYPE_INT64") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_TYPE_INT64_name, &const_TYPE_INT64_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_TYPE_INT64_name);

	zval const_TYPE_DOUBLE_value;
	ZVAL_LONG(&const_TYPE_DOUBLE_value, FORMAT_TYPE_DOUBLE);
	crex_string *const_TYPE_DOUBLE_name = crex_string_init_interned("TYPE_DOUBLE", sizeof("TYPE_DOUBLE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_TYPE_DOUBLE_name, &const_TYPE_DOUBLE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_TYPE_DOUBLE_name);

	zval const_TYPE_CURRENCY_value;
	ZVAL_LONG(&const_TYPE_CURRENCY_value, FORMAT_TYPE_CURRENCY);
	crex_string *const_TYPE_CURRENCY_name = crex_string_init_interned("TYPE_CURRENCY", sizeof("TYPE_CURRENCY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_TYPE_CURRENCY_name, &const_TYPE_CURRENCY_value, CREX_ACC_PUBLIC|CREX_ACC_DEPRECATED, NULL);
	crex_string_release(const_TYPE_CURRENCY_name);

	return class_entry;
}
