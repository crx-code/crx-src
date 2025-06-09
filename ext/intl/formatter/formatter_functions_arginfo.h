/* This is a generated file, edit the .stub.crx file instead. */

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_numfmt_create, 0, 2, NumberFormatter, 1)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, style, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_numfmt_format, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, fmt, NumberFormatter, 0)
	CREX_ARG_TYPE_MASK(0, value, MAY_BE_LONG|MAY_BE_DOUBLE)
	CREX_ARG_TYPE_INFO(0, type, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_numfmt_parse, 0, 2, MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, fmt, NumberFormatter, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	CREX_ARG_INFO(1, position)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_numfmt_format_currency, 0, 3, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, fmt, NumberFormatter, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, currency, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_numfmt_parse_currency, 0, 3, MAY_BE_DOUBLE|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, fmt, NumberFormatter, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_STRING, 0)
	CREX_ARG_INFO(1, currency)
	CREX_ARG_INFO(1, position)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_numfmt_set_attribute, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, fmt, NumberFormatter, 0)
	CREX_ARG_TYPE_INFO(0, attr, IS_LONG, 0)
	CREX_ARG_INFO(0, value)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_numfmt_get_attribute, 0, 2, double, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, fmt, NumberFormatter, 0)
	CREX_ARG_TYPE_INFO(0, attr, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_numfmt_set_text_attribute, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, fmt, NumberFormatter, 0)
	CREX_ARG_TYPE_INFO(0, attr, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_numfmt_get_text_attribute, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, fmt, NumberFormatter, 0)
	CREX_ARG_TYPE_INFO(0, attr, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_numfmt_set_symbol arginfo_numfmt_set_text_attribute

#define arginfo_numfmt_get_symbol arginfo_numfmt_get_text_attribute

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_numfmt_set_pattern, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, fmt, NumberFormatter, 0)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_numfmt_get_pattern, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, fmt, NumberFormatter, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_numfmt_get_locale, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, fmt, NumberFormatter, 0)
	CREX_ARG_TYPE_INFO(0, type, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_numfmt_get_error_code, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, fmt, NumberFormatter, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_numfmt_get_error_message, 0, 1, IS_STRING, 0)
	CREX_ARG_OBJ_INFO(0, fmt, NumberFormatter, 0)
CREX_END_ARG_INFO()
