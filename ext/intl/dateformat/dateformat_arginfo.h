/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: c3aabab98e4864276f6cb0afb2e3fefad0386481 */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_IntlDateFormatter___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, dateType, IS_LONG, 0, "IntlDateFormatter::FULL")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeType, IS_LONG, 0, "IntlDateFormatter::FULL")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, timezone, "null")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, calendar, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pattern, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_IntlDateFormatter_create, 0, 1, IntlDateFormatter, 1)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, dateType, IS_LONG, 0, "IntlDateFormatter::FULL")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeType, IS_LONG, 0, "IntlDateFormatter::FULL")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, timezone, "null")
	CREX_ARG_OBJ_TYPE_MASK(0, calendar, IntlCalendar, MAY_BE_LONG|MAY_BE_NULL, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pattern, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlDateFormatter_getDateType, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
CREX_END_ARG_INFO()

#define arginfo_class_IntlDateFormatter_getTimeType arginfo_class_IntlDateFormatter_getDateType

#define arginfo_class_IntlDateFormatter_getCalendar arginfo_class_IntlDateFormatter_getDateType

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlDateFormatter_setCalendar, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, calendar, IntlCalendar, MAY_BE_LONG|MAY_BE_NULL, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlDateFormatter_getTimeZoneId, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_IntlDateFormatter_getCalendarObject, 0, 0, IntlCalendar, MAY_BE_FALSE|MAY_BE_NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_IntlDateFormatter_getTimeZone, 0, 0, IntlTimeZone, MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlDateFormatter_setTimeZone, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, timezone)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlDateFormatter_setPattern, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_IntlDateFormatter_getPattern arginfo_class_IntlDateFormatter_getTimeZoneId

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlDateFormatter_getLocale, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_LONG, 0, "ULOC_ACTUAL_LOCALE")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlDateFormatter_setLenient, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, lenient, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlDateFormatter_isLenient, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlDateFormatter_format, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO(0, datetime)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlDateFormatter_formatObject, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO(0, datetime)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, format, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, locale, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlDateFormatter_parse, 0, 1, MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, offset, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlDateFormatter_localtime, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, offset, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlDateFormatter_getErrorCode, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlDateFormatter_getErrorMessage, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()


CREX_METHOD(IntlDateFormatter, __main);
CREX_FUNCTION(datefmt_create);
CREX_FUNCTION(datefmt_get_datetype);
CREX_FUNCTION(datefmt_get_timetype);
CREX_FUNCTION(datefmt_get_calendar);
CREX_FUNCTION(datefmt_set_calendar);
CREX_FUNCTION(datefmt_get_timezone_id);
CREX_FUNCTION(datefmt_get_calendar_object);
CREX_FUNCTION(datefmt_get_timezone);
CREX_FUNCTION(datefmt_set_timezone);
CREX_FUNCTION(datefmt_set_pattern);
CREX_FUNCTION(datefmt_get_pattern);
CREX_FUNCTION(datefmt_get_locale);
CREX_FUNCTION(datefmt_set_lenient);
CREX_FUNCTION(datefmt_is_lenient);
CREX_FUNCTION(datefmt_format);
CREX_FUNCTION(datefmt_format_object);
CREX_FUNCTION(datefmt_parse);
CREX_FUNCTION(datefmt_localtime);
CREX_FUNCTION(datefmt_get_error_code);
CREX_FUNCTION(datefmt_get_error_message);


static const crex_function_entry class_IntlDateFormatter_methods[] = {
	CREX_ME(IntlDateFormatter, __main, arginfo_class_IntlDateFormatter___main, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(create, datefmt_create, arginfo_class_IntlDateFormatter_create, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getDateType, datefmt_get_datetype, arginfo_class_IntlDateFormatter_getDateType, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getTimeType, datefmt_get_timetype, arginfo_class_IntlDateFormatter_getTimeType, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getCalendar, datefmt_get_calendar, arginfo_class_IntlDateFormatter_getCalendar, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setCalendar, datefmt_set_calendar, arginfo_class_IntlDateFormatter_setCalendar, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getTimeZoneId, datefmt_get_timezone_id, arginfo_class_IntlDateFormatter_getTimeZoneId, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getCalendarObject, datefmt_get_calendar_object, arginfo_class_IntlDateFormatter_getCalendarObject, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getTimeZone, datefmt_get_timezone, arginfo_class_IntlDateFormatter_getTimeZone, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setTimeZone, datefmt_set_timezone, arginfo_class_IntlDateFormatter_setTimeZone, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setPattern, datefmt_set_pattern, arginfo_class_IntlDateFormatter_setPattern, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getPattern, datefmt_get_pattern, arginfo_class_IntlDateFormatter_getPattern, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getLocale, datefmt_get_locale, arginfo_class_IntlDateFormatter_getLocale, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setLenient, datefmt_set_lenient, arginfo_class_IntlDateFormatter_setLenient, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(isLenient, datefmt_is_lenient, arginfo_class_IntlDateFormatter_isLenient, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(format, datefmt_format, arginfo_class_IntlDateFormatter_format, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(formatObject, datefmt_format_object, arginfo_class_IntlDateFormatter_formatObject, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(parse, datefmt_parse, arginfo_class_IntlDateFormatter_parse, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(localtime, datefmt_localtime, arginfo_class_IntlDateFormatter_localtime, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getErrorCode, datefmt_get_error_code, arginfo_class_IntlDateFormatter_getErrorCode, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getErrorMessage, datefmt_get_error_message, arginfo_class_IntlDateFormatter_getErrorMessage, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_IntlDateFormatter(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "IntlDateFormatter", class_IntlDateFormatter_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;

	zval const_FULL_value;
	ZVAL_LONG(&const_FULL_value, UDAT_FULL);
	crex_string *const_FULL_name = crex_string_init_interned("FULL", sizeof("FULL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FULL_name, &const_FULL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FULL_name);

	zval const_LONG_value;
	ZVAL_LONG(&const_LONG_value, UDAT_LONG);
	crex_string *const_LONG_name = crex_string_init_interned("LONG", sizeof("LONG") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LONG_name, &const_LONG_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LONG_name);

	zval const_MEDIUM_value;
	ZVAL_LONG(&const_MEDIUM_value, UDAT_MEDIUM);
	crex_string *const_MEDIUM_name = crex_string_init_interned("MEDIUM", sizeof("MEDIUM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_MEDIUM_name, &const_MEDIUM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_MEDIUM_name);

	zval const_SHORT_value;
	ZVAL_LONG(&const_SHORT_value, UDAT_SHORT);
	crex_string *const_SHORT_name = crex_string_init_interned("SHORT", sizeof("SHORT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SHORT_name, &const_SHORT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SHORT_name);

	zval const_NONE_value;
	ZVAL_LONG(&const_NONE_value, UDAT_NONE);
	crex_string *const_NONE_name = crex_string_init_interned("NONE", sizeof("NONE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NONE_name, &const_NONE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NONE_name);

	zval const_RELATIVE_FULL_value;
	ZVAL_LONG(&const_RELATIVE_FULL_value, UDAT_FULL_RELATIVE);
	crex_string *const_RELATIVE_FULL_name = crex_string_init_interned("RELATIVE_FULL", sizeof("RELATIVE_FULL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_RELATIVE_FULL_name, &const_RELATIVE_FULL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_RELATIVE_FULL_name);

	zval const_RELATIVE_LONG_value;
	ZVAL_LONG(&const_RELATIVE_LONG_value, UDAT_LONG_RELATIVE);
	crex_string *const_RELATIVE_LONG_name = crex_string_init_interned("RELATIVE_LONG", sizeof("RELATIVE_LONG") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_RELATIVE_LONG_name, &const_RELATIVE_LONG_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_RELATIVE_LONG_name);

	zval const_RELATIVE_MEDIUM_value;
	ZVAL_LONG(&const_RELATIVE_MEDIUM_value, UDAT_MEDIUM_RELATIVE);
	crex_string *const_RELATIVE_MEDIUM_name = crex_string_init_interned("RELATIVE_MEDIUM", sizeof("RELATIVE_MEDIUM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_RELATIVE_MEDIUM_name, &const_RELATIVE_MEDIUM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_RELATIVE_MEDIUM_name);

	zval const_RELATIVE_SHORT_value;
	ZVAL_LONG(&const_RELATIVE_SHORT_value, UDAT_SHORT_RELATIVE);
	crex_string *const_RELATIVE_SHORT_name = crex_string_init_interned("RELATIVE_SHORT", sizeof("RELATIVE_SHORT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_RELATIVE_SHORT_name, &const_RELATIVE_SHORT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_RELATIVE_SHORT_name);

	zval const_GREGORIAN_value;
	ZVAL_LONG(&const_GREGORIAN_value, UCAL_GREGORIAN);
	crex_string *const_GREGORIAN_name = crex_string_init_interned("GREGORIAN", sizeof("GREGORIAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_GREGORIAN_name, &const_GREGORIAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_GREGORIAN_name);

	zval const_TRADITIONAL_value;
	ZVAL_LONG(&const_TRADITIONAL_value, UCAL_TRADITIONAL);
	crex_string *const_TRADITIONAL_name = crex_string_init_interned("TRADITIONAL", sizeof("TRADITIONAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_TRADITIONAL_name, &const_TRADITIONAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_TRADITIONAL_name);

	return class_entry;
}
