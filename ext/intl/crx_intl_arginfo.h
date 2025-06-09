/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: c32e74bddb55455f69083a302bcaf52f654b1293 */

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_intlcal_create_instance, 0, 0, IntlCalendar, 1)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, timezone, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, locale, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_intlcal_get_keyword_values_for_locale, 0, 3, IntlIterator, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, keyword, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, onlyCommon, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intlcal_get_now, 0, 0, IS_DOUBLE, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intlcal_get_available_locales, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_intlcal_get, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
	CREX_ARG_TYPE_INFO(0, field, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_intlcal_get_time, 0, 1, MAY_BE_DOUBLE|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intlcal_set_time, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
	CREX_ARG_TYPE_INFO(0, timestamp, IS_DOUBLE, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intlcal_add, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
	CREX_ARG_TYPE_INFO(0, field, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intlcal_set_time_zone, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
	CREX_ARG_INFO(0, timezone)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intlcal_after, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
	CREX_ARG_OBJ_INFO(0, other, IntlCalendar, 0)
CREX_END_ARG_INFO()

#define arginfo_intlcal_before arginfo_intlcal_after

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intlcal_set, 0, 3, IS_TRUE, 0)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
	CREX_ARG_TYPE_INFO(0, year, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, month, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, dayOfMonth, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, hour, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, minute, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, second, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intlcal_roll, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
	CREX_ARG_TYPE_INFO(0, field, IS_LONG, 0)
	CREX_ARG_INFO(0, value)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intlcal_clear, 0, 1, IS_TRUE, 0)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, field, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_intlcal_field_difference, 0, 3, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
	CREX_ARG_TYPE_INFO(0, timestamp, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, field, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_intlcal_get_actual_maximum arginfo_intlcal_get

#define arginfo_intlcal_get_actual_minimum arginfo_intlcal_get

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_intlcal_get_day_of_week_type, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
	CREX_ARG_TYPE_INFO(0, dayOfWeek, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_intlcal_get_first_day_of_week, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
CREX_END_ARG_INFO()

#define arginfo_intlcal_get_least_maximum arginfo_intlcal_get

#define arginfo_intlcal_get_greatest_minimum arginfo_intlcal_get

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_intlcal_get_locale, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
	CREX_ARG_TYPE_INFO(0, type, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_intlcal_get_maximum arginfo_intlcal_get

#define arginfo_intlcal_get_minimal_days_in_first_week arginfo_intlcal_get_first_day_of_week

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intlcal_set_minimal_days_in_first_week, 0, 2, IS_TRUE, 0)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
	CREX_ARG_TYPE_INFO(0, days, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_intlcal_get_minimum arginfo_intlcal_get

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_intlcal_get_time_zone, 0, 1, IntlTimeZone, MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intlcal_get_type, 0, 1, IS_STRING, 0)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
CREX_END_ARG_INFO()

#define arginfo_intlcal_get_weekend_transition arginfo_intlcal_get_day_of_week_type

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intlcal_in_daylight_time, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
CREX_END_ARG_INFO()

#define arginfo_intlcal_is_lenient arginfo_intlcal_in_daylight_time

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intlcal_is_set, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
	CREX_ARG_TYPE_INFO(0, field, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_intlcal_is_equivalent_to arginfo_intlcal_after

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intlcal_is_weekend, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timestamp, IS_DOUBLE, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intlcal_set_first_day_of_week, 0, 2, IS_TRUE, 0)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
	CREX_ARG_TYPE_INFO(0, dayOfWeek, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intlcal_set_lenient, 0, 2, IS_TRUE, 0)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
	CREX_ARG_TYPE_INFO(0, lenient, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intlcal_get_repeated_wall_time_option, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
CREX_END_ARG_INFO()

#define arginfo_intlcal_equals arginfo_intlcal_after

#define arginfo_intlcal_get_skipped_wall_time_option arginfo_intlcal_get_repeated_wall_time_option

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intlcal_set_repeated_wall_time_option, 0, 2, IS_TRUE, 0)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
	CREX_ARG_TYPE_INFO(0, option, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_intlcal_set_skipped_wall_time_option arginfo_intlcal_set_repeated_wall_time_option

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_intlcal_from_date_time, 0, 1, IntlCalendar, 1)
	CREX_ARG_OBJ_TYPE_MASK(0, datetime, DateTime, MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, locale, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_intlcal_to_date_time, 0, 1, DateTime, MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
CREX_END_ARG_INFO()

#define arginfo_intlcal_get_error_code arginfo_intlcal_get_first_day_of_week

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_intlcal_get_error_message, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, calendar, IntlCalendar, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_intlgregcal_create_instance, 0, 0, IntlGregorianCalendar, 1)
	CREX_ARG_INFO(0, timezoneOrYear)
	CREX_ARG_INFO(0, localeOrMonth)
	CREX_ARG_INFO(0, day)
	CREX_ARG_INFO(0, hour)
	CREX_ARG_INFO(0, minute)
	CREX_ARG_INFO(0, second)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intlgregcal_set_gregorian_change, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, calendar, IntlGregorianCalendar, 0)
	CREX_ARG_TYPE_INFO(0, timestamp, IS_DOUBLE, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intlgregcal_get_gregorian_change, 0, 1, IS_DOUBLE, 0)
	CREX_ARG_OBJ_INFO(0, calendar, IntlGregorianCalendar, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intlgregcal_is_leap_year, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, calendar, IntlGregorianCalendar, 0)
	CREX_ARG_TYPE_INFO(0, year, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_collator_create, 0, 1, Collator, 1)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_collator_compare, 0, 3, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, object, Collator, 0)
	CREX_ARG_TYPE_INFO(0, string1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string2, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_collator_get_attribute, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, object, Collator, 0)
	CREX_ARG_TYPE_INFO(0, attribute, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_collator_set_attribute, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, object, Collator, 0)
	CREX_ARG_TYPE_INFO(0, attribute, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_collator_get_strength, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, object, Collator, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_collator_set_strength, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, object, Collator, 0)
	CREX_ARG_TYPE_INFO(0, strength, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_collator_sort, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, object, Collator, 0)
	CREX_ARG_TYPE_INFO(1, array, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "Collator::SORT_REGULAR")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_collator_sort_with_sort_keys, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, object, Collator, 0)
	CREX_ARG_TYPE_INFO(1, array, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_collator_asort arginfo_collator_sort

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_collator_get_locale, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, object, Collator, 0)
	CREX_ARG_TYPE_INFO(0, type, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_collator_get_error_code, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, object, Collator, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_collator_get_error_message, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, object, Collator, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_collator_get_sort_key, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, object, Collator, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intl_get_error_code, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intl_get_error_message, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intl_is_failure, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, errorCode, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intl_error_name, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, errorCode, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_datefmt_create, 0, 1, IntlDateFormatter, 1)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, dateType, IS_LONG, 0, "IntlDateFormatter::FULL")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeType, IS_LONG, 0, "IntlDateFormatter::FULL")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, timezone, "null")
	CREX_ARG_OBJ_TYPE_MASK(0, calendar, IntlCalendar, MAY_BE_LONG|MAY_BE_NULL, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pattern, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_datefmt_get_datetype, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, formatter, IntlDateFormatter, 0)
CREX_END_ARG_INFO()

#define arginfo_datefmt_get_timetype arginfo_datefmt_get_datetype

#define arginfo_datefmt_get_calendar arginfo_datefmt_get_datetype

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_datefmt_set_calendar, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, formatter, IntlDateFormatter, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, calendar, IntlCalendar, MAY_BE_LONG|MAY_BE_NULL, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_datefmt_get_timezone_id, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, formatter, IntlDateFormatter, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_datefmt_get_calendar_object, 0, 1, IntlCalendar, MAY_BE_FALSE|MAY_BE_NULL)
	CREX_ARG_OBJ_INFO(0, formatter, IntlDateFormatter, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_datefmt_get_timezone, 0, 1, IntlTimeZone, MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, formatter, IntlDateFormatter, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_datefmt_set_timezone, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, formatter, IntlDateFormatter, 0)
	CREX_ARG_INFO(0, timezone)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_datefmt_set_pattern, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, formatter, IntlDateFormatter, 0)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_datefmt_get_pattern arginfo_datefmt_get_timezone_id

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_datefmt_get_locale, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, formatter, IntlDateFormatter, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_LONG, 0, "ULOC_ACTUAL_LOCALE")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_datefmt_set_lenient, 0, 2, IS_VOID, 0)
	CREX_ARG_OBJ_INFO(0, formatter, IntlDateFormatter, 0)
	CREX_ARG_TYPE_INFO(0, lenient, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_datefmt_is_lenient, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, formatter, IntlDateFormatter, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_datefmt_format, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, formatter, IntlDateFormatter, 0)
	CREX_ARG_INFO(0, datetime)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_datefmt_format_object, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO(0, datetime)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, format, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, locale, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_datefmt_parse, 0, 2, MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, formatter, IntlDateFormatter, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, offset, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_datefmt_localtime, 0, 2, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, formatter, IntlDateFormatter, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, offset, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_datefmt_get_error_code, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, formatter, IntlDateFormatter, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_datefmt_get_error_message, 0, 1, IS_STRING, 0)
	CREX_ARG_OBJ_INFO(0, formatter, IntlDateFormatter, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_numfmt_create, 0, 2, NumberFormatter, 1)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, style, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pattern, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_numfmt_format, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, formatter, NumberFormatter, 0)
	CREX_ARG_TYPE_MASK(0, num, MAY_BE_LONG|MAY_BE_DOUBLE, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_LONG, 0, "NumberFormatter::TYPE_DEFAULT")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_numfmt_parse, 0, 2, MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, formatter, NumberFormatter, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_LONG, 0, "NumberFormatter::TYPE_DOUBLE")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, offset, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_numfmt_format_currency, 0, 3, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, formatter, NumberFormatter, 0)
	CREX_ARG_TYPE_INFO(0, amount, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, currency, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_numfmt_parse_currency, 0, 3, MAY_BE_DOUBLE|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, formatter, NumberFormatter, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_INFO(1, currency)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, offset, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_numfmt_set_attribute, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, formatter, NumberFormatter, 0)
	CREX_ARG_TYPE_INFO(0, attribute, IS_LONG, 0)
	CREX_ARG_TYPE_MASK(0, value, MAY_BE_LONG|MAY_BE_DOUBLE, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_numfmt_get_attribute, 0, 2, MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, formatter, NumberFormatter, 0)
	CREX_ARG_TYPE_INFO(0, attribute, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_numfmt_set_text_attribute, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, formatter, NumberFormatter, 0)
	CREX_ARG_TYPE_INFO(0, attribute, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_numfmt_get_text_attribute, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, formatter, NumberFormatter, 0)
	CREX_ARG_TYPE_INFO(0, attribute, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_numfmt_set_symbol, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, formatter, NumberFormatter, 0)
	CREX_ARG_TYPE_INFO(0, symbol, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_numfmt_get_symbol, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, formatter, NumberFormatter, 0)
	CREX_ARG_TYPE_INFO(0, symbol, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_numfmt_set_pattern, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, formatter, NumberFormatter, 0)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_numfmt_get_pattern, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, formatter, NumberFormatter, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_numfmt_get_locale, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, formatter, NumberFormatter, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_LONG, 0, "ULOC_ACTUAL_LOCALE")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_numfmt_get_error_code, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, formatter, NumberFormatter, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_numfmt_get_error_message, 0, 1, IS_STRING, 0)
	CREX_ARG_OBJ_INFO(0, formatter, NumberFormatter, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_grapheme_strlen, 0, 1, MAY_BE_LONG|MAY_BE_FALSE|MAY_BE_NULL)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_grapheme_strpos, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, haystack, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, needle, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

#define arginfo_grapheme_stripos arginfo_grapheme_strpos

#define arginfo_grapheme_strrpos arginfo_grapheme_strpos

#define arginfo_grapheme_strripos arginfo_grapheme_strpos

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_grapheme_substr, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_grapheme_strstr, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, haystack, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, needle, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, beforeNeedle, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

#define arginfo_grapheme_stristr arginfo_grapheme_strstr

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_grapheme_extract, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, haystack, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, size, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_LONG, 0, "GRAPHEME_EXTR_COUNT")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, next, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_idn_to_ascii, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, domain, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "IDNA_DEFAULT")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, variant, IS_LONG, 0, "INTL_IDNA_VARIANT_UTS46")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, idna_info, "null")
CREX_END_ARG_INFO()

#define arginfo_idn_to_utf8 arginfo_idn_to_ascii

#define arginfo_locale_get_default arginfo_intl_get_error_message

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_locale_set_default, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_locale_get_primary_language, 0, 1, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_locale_get_script arginfo_locale_get_primary_language

#define arginfo_locale_get_region arginfo_locale_get_primary_language

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_locale_get_keywords, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE|MAY_BE_NULL)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_locale_get_display_script, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, displayLocale, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_locale_get_display_region arginfo_locale_get_display_script

#define arginfo_locale_get_display_name arginfo_locale_get_display_script

#define arginfo_locale_get_display_language arginfo_locale_get_display_script

#define arginfo_locale_get_display_variant arginfo_locale_get_display_script

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_locale_compose, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, subtags, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_locale_parse, 0, 1, IS_ARRAY, 1)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_locale_get_all_variants arginfo_locale_parse

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_locale_filter_matches, 0, 2, _IS_BOOL, 1)
	CREX_ARG_TYPE_INFO(0, languageTag, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, canonicalize, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

#define arginfo_locale_canonicalize arginfo_locale_get_primary_language

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_locale_lookup, 0, 2, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, languageTag, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, canonicalize, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, defaultLocale, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_locale_accept_from_http, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, header, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_msgfmt_create, 0, 2, MessageFormatter, 1)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_msgfmt_format, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, formatter, MessageFormatter, 0)
	CREX_ARG_TYPE_INFO(0, values, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_msgfmt_format_message, 0, 3, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, values, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_msgfmt_parse, 0, 2, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, formatter, MessageFormatter, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_msgfmt_parse_message, 0, 3, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, message, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_msgfmt_set_pattern, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, formatter, MessageFormatter, 0)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_msgfmt_get_pattern, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, formatter, MessageFormatter, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_msgfmt_get_locale, 0, 1, IS_STRING, 0)
	CREX_ARG_OBJ_INFO(0, formatter, MessageFormatter, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_msgfmt_get_error_code, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, formatter, MessageFormatter, 0)
CREX_END_ARG_INFO()

#define arginfo_msgfmt_get_error_message arginfo_msgfmt_get_locale

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_normalizer_normalize, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, form, IS_LONG, 0, "Normalizer::FORM_C")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_normalizer_is_normalized, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, form, IS_LONG, 0, "Normalizer::FORM_C")
CREX_END_ARG_INFO()

#if U_ICU_VERSION_MAJOR_NUM >= 56
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_normalizer_get_raw_decomposition, 0, 1, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, form, IS_LONG, 0, "Normalizer::FORM_C")
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_resourcebundle_create, 0, 2, ResourceBundle, 1)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, bundle, IS_STRING, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, fallback, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_resourcebundle_get, 0, 2, IS_MIXED, 0)
	CREX_ARG_OBJ_INFO(0, bundle, ResourceBundle, 0)
	CREX_ARG_INFO(0, index)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, fallback, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_resourcebundle_count, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, bundle, ResourceBundle, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_resourcebundle_locales, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, bundle, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_resourcebundle_get_error_code arginfo_resourcebundle_count

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_resourcebundle_get_error_message, 0, 1, IS_STRING, 0)
	CREX_ARG_OBJ_INFO(0, bundle, ResourceBundle, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_intltz_count_equivalent_ids, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, timezoneId, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_intltz_create_default, 0, 0, IntlTimeZone, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_intltz_create_enumeration, 0, 0, IntlIterator, MAY_BE_FALSE)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, countryOrRawOffset, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_intltz_create_time_zone, 0, 1, IntlTimeZone, 1)
	CREX_ARG_TYPE_INFO(0, timezoneId, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_intltz_create_time_zone_id_enumeration, 0, 1, IntlIterator, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, region, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, rawOffset, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_intltz_from_date_time_zone, 0, 1, IntlTimeZone, 1)
	CREX_ARG_OBJ_INFO(0, timezone, DateTimeZone, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_intltz_get_canonical_id, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, timezoneId, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, isSystemId, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_intltz_get_display_name, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, timezone, IntlTimeZone, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, dst, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_LONG, 0, "IntlTimeZone::DISPLAY_LONG")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, locale, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intltz_get_dst_savings, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, timezone, IntlTimeZone, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_intltz_get_equivalent_id, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, timezoneId, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_intltz_get_error_code, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, timezone, IntlTimeZone, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_intltz_get_error_message, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, timezone, IntlTimeZone, 0)
CREX_END_ARG_INFO()

#define arginfo_intltz_get_gmt arginfo_intltz_create_default

#define arginfo_intltz_get_id arginfo_intltz_get_error_message

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intltz_get_offset, 0, 5, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, timezone, IntlTimeZone, 0)
	CREX_ARG_TYPE_INFO(0, timestamp, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, local, _IS_BOOL, 0)
	CREX_ARG_INFO(1, rawOffset)
	CREX_ARG_INFO(1, dstOffset)
CREX_END_ARG_INFO()

#define arginfo_intltz_get_raw_offset arginfo_intltz_get_dst_savings

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_intltz_get_region, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, timezoneId, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_intltz_get_tz_data_version, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
CREX_END_ARG_INFO()

#define arginfo_intltz_get_unknown arginfo_intltz_create_default

#if U_ICU_VERSION_MAJOR_NUM >= 52
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_intltz_get_windows_id, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, timezoneId, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#if U_ICU_VERSION_MAJOR_NUM >= 52
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_intltz_get_id_for_windows_id, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, timezoneId, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, region, IS_STRING, 1, "null")
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intltz_has_same_rules, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, timezone, IntlTimeZone, 0)
	CREX_ARG_OBJ_INFO(0, other, IntlTimeZone, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_intltz_to_date_time_zone, 0, 1, DateTimeZone, MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, timezone, IntlTimeZone, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intltz_use_daylight_time, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, timezone, IntlTimeZone, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_transliterator_create, 0, 1, Transliterator, 1)
	CREX_ARG_TYPE_INFO(0, id, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, direction, IS_LONG, 0, "Transliterator::FORWARD")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_transliterator_create_from_rules, 0, 1, Transliterator, 1)
	CREX_ARG_TYPE_INFO(0, rules, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, direction, IS_LONG, 0, "Transliterator::FORWARD")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_transliterator_list_ids, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_transliterator_create_inverse, 0, 1, Transliterator, 1)
	CREX_ARG_OBJ_INFO(0, transliterator, Transliterator, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_transliterator_transliterate, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_TYPE_MASK(0, transliterator, Transliterator, MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, start, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, end, IS_LONG, 0, "-1")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_transliterator_get_error_code, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, transliterator, Transliterator, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_transliterator_get_error_message, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, transliterator, Transliterator, 0)
CREX_END_ARG_INFO()


CREX_FUNCTION(intlcal_create_instance);
CREX_FUNCTION(intlcal_get_keyword_values_for_locale);
CREX_FUNCTION(intlcal_get_now);
CREX_FUNCTION(intlcal_get_available_locales);
CREX_FUNCTION(intlcal_get);
CREX_FUNCTION(intlcal_get_time);
CREX_FUNCTION(intlcal_set_time);
CREX_FUNCTION(intlcal_add);
CREX_FUNCTION(intlcal_set_time_zone);
CREX_FUNCTION(intlcal_after);
CREX_FUNCTION(intlcal_before);
CREX_FUNCTION(intlcal_set);
CREX_FUNCTION(intlcal_roll);
CREX_FUNCTION(intlcal_clear);
CREX_FUNCTION(intlcal_field_difference);
CREX_FUNCTION(intlcal_get_actual_maximum);
CREX_FUNCTION(intlcal_get_actual_minimum);
CREX_FUNCTION(intlcal_get_day_of_week_type);
CREX_FUNCTION(intlcal_get_first_day_of_week);
CREX_FUNCTION(intlcal_get_least_maximum);
CREX_FUNCTION(intlcal_get_greatest_minimum);
CREX_FUNCTION(intlcal_get_locale);
CREX_FUNCTION(intlcal_get_maximum);
CREX_FUNCTION(intlcal_get_minimal_days_in_first_week);
CREX_FUNCTION(intlcal_set_minimal_days_in_first_week);
CREX_FUNCTION(intlcal_get_minimum);
CREX_FUNCTION(intlcal_get_time_zone);
CREX_FUNCTION(intlcal_get_type);
CREX_FUNCTION(intlcal_get_weekend_transition);
CREX_FUNCTION(intlcal_in_daylight_time);
CREX_FUNCTION(intlcal_is_lenient);
CREX_FUNCTION(intlcal_is_set);
CREX_FUNCTION(intlcal_is_equivalent_to);
CREX_FUNCTION(intlcal_is_weekend);
CREX_FUNCTION(intlcal_set_first_day_of_week);
CREX_FUNCTION(intlcal_set_lenient);
CREX_FUNCTION(intlcal_get_repeated_wall_time_option);
CREX_FUNCTION(intlcal_equals);
CREX_FUNCTION(intlcal_get_skipped_wall_time_option);
CREX_FUNCTION(intlcal_set_repeated_wall_time_option);
CREX_FUNCTION(intlcal_set_skipped_wall_time_option);
CREX_FUNCTION(intlcal_from_date_time);
CREX_FUNCTION(intlcal_to_date_time);
CREX_FUNCTION(intlcal_get_error_code);
CREX_FUNCTION(intlcal_get_error_message);
CREX_FUNCTION(intlgregcal_create_instance);
CREX_FUNCTION(intlgregcal_set_gregorian_change);
CREX_FUNCTION(intlgregcal_get_gregorian_change);
CREX_FUNCTION(intlgregcal_is_leap_year);
CREX_FUNCTION(collator_create);
CREX_FUNCTION(collator_compare);
CREX_FUNCTION(collator_get_attribute);
CREX_FUNCTION(collator_set_attribute);
CREX_FUNCTION(collator_get_strength);
CREX_FUNCTION(collator_set_strength);
CREX_FUNCTION(collator_sort);
CREX_FUNCTION(collator_sort_with_sort_keys);
CREX_FUNCTION(collator_asort);
CREX_FUNCTION(collator_get_locale);
CREX_FUNCTION(collator_get_error_code);
CREX_FUNCTION(collator_get_error_message);
CREX_FUNCTION(collator_get_sort_key);
CREX_FUNCTION(intl_get_error_code);
CREX_FUNCTION(intl_get_error_message);
CREX_FUNCTION(intl_is_failure);
CREX_FUNCTION(intl_error_name);
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
CREX_FUNCTION(grapheme_strlen);
CREX_FUNCTION(grapheme_strpos);
CREX_FUNCTION(grapheme_stripos);
CREX_FUNCTION(grapheme_strrpos);
CREX_FUNCTION(grapheme_strripos);
CREX_FUNCTION(grapheme_substr);
CREX_FUNCTION(grapheme_strstr);
CREX_FUNCTION(grapheme_stristr);
CREX_FUNCTION(grapheme_extract);
CREX_FUNCTION(idn_to_ascii);
CREX_FUNCTION(idn_to_utf8);
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
CREX_FUNCTION(locale_canonicalize);
CREX_FUNCTION(locale_lookup);
CREX_FUNCTION(locale_accept_from_http);
CREX_FUNCTION(msgfmt_create);
CREX_FUNCTION(msgfmt_format);
CREX_FUNCTION(msgfmt_format_message);
CREX_FUNCTION(msgfmt_parse);
CREX_FUNCTION(msgfmt_parse_message);
CREX_FUNCTION(msgfmt_set_pattern);
CREX_FUNCTION(msgfmt_get_pattern);
CREX_FUNCTION(msgfmt_get_locale);
CREX_FUNCTION(msgfmt_get_error_code);
CREX_FUNCTION(msgfmt_get_error_message);
CREX_FUNCTION(normalizer_normalize);
CREX_FUNCTION(normalizer_is_normalized);
#if U_ICU_VERSION_MAJOR_NUM >= 56
CREX_FUNCTION(normalizer_get_raw_decomposition);
#endif
CREX_FUNCTION(resourcebundle_create);
CREX_FUNCTION(resourcebundle_get);
CREX_FUNCTION(resourcebundle_count);
CREX_FUNCTION(resourcebundle_locales);
CREX_FUNCTION(resourcebundle_get_error_code);
CREX_FUNCTION(resourcebundle_get_error_message);
CREX_FUNCTION(intltz_count_equivalent_ids);
CREX_FUNCTION(intltz_create_default);
CREX_FUNCTION(intltz_create_enumeration);
CREX_FUNCTION(intltz_create_time_zone);
CREX_FUNCTION(intltz_create_time_zone_id_enumeration);
CREX_FUNCTION(intltz_from_date_time_zone);
CREX_FUNCTION(intltz_get_canonical_id);
CREX_FUNCTION(intltz_get_display_name);
CREX_FUNCTION(intltz_get_dst_savings);
CREX_FUNCTION(intltz_get_equivalent_id);
CREX_FUNCTION(intltz_get_error_code);
CREX_FUNCTION(intltz_get_error_message);
CREX_FUNCTION(intltz_get_gmt);
CREX_FUNCTION(intltz_get_id);
CREX_FUNCTION(intltz_get_offset);
CREX_FUNCTION(intltz_get_raw_offset);
CREX_FUNCTION(intltz_get_region);
CREX_FUNCTION(intltz_get_tz_data_version);
CREX_FUNCTION(intltz_get_unknown);
#if U_ICU_VERSION_MAJOR_NUM >= 52
CREX_FUNCTION(intltz_get_windows_id);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 52
CREX_FUNCTION(intltz_get_id_for_windows_id);
#endif
CREX_FUNCTION(intltz_has_same_rules);
CREX_FUNCTION(intltz_to_date_time_zone);
CREX_FUNCTION(intltz_use_daylight_time);
CREX_FUNCTION(transliterator_create);
CREX_FUNCTION(transliterator_create_from_rules);
CREX_FUNCTION(transliterator_list_ids);
CREX_FUNCTION(transliterator_create_inverse);
CREX_FUNCTION(transliterator_transliterate);
CREX_FUNCTION(transliterator_get_error_code);
CREX_FUNCTION(transliterator_get_error_message);


static const crex_function_entry ext_functions[] = {
	CREX_FE(intlcal_create_instance, arginfo_intlcal_create_instance)
	CREX_FE(intlcal_get_keyword_values_for_locale, arginfo_intlcal_get_keyword_values_for_locale)
	CREX_FE(intlcal_get_now, arginfo_intlcal_get_now)
	CREX_FE(intlcal_get_available_locales, arginfo_intlcal_get_available_locales)
	CREX_FE(intlcal_get, arginfo_intlcal_get)
	CREX_FE(intlcal_get_time, arginfo_intlcal_get_time)
	CREX_FE(intlcal_set_time, arginfo_intlcal_set_time)
	CREX_FE(intlcal_add, arginfo_intlcal_add)
	CREX_FE(intlcal_set_time_zone, arginfo_intlcal_set_time_zone)
	CREX_FE(intlcal_after, arginfo_intlcal_after)
	CREX_FE(intlcal_before, arginfo_intlcal_before)
	CREX_FE(intlcal_set, arginfo_intlcal_set)
	CREX_FE(intlcal_roll, arginfo_intlcal_roll)
	CREX_FE(intlcal_clear, arginfo_intlcal_clear)
	CREX_FE(intlcal_field_difference, arginfo_intlcal_field_difference)
	CREX_FE(intlcal_get_actual_maximum, arginfo_intlcal_get_actual_maximum)
	CREX_FE(intlcal_get_actual_minimum, arginfo_intlcal_get_actual_minimum)
	CREX_FE(intlcal_get_day_of_week_type, arginfo_intlcal_get_day_of_week_type)
	CREX_FE(intlcal_get_first_day_of_week, arginfo_intlcal_get_first_day_of_week)
	CREX_FE(intlcal_get_least_maximum, arginfo_intlcal_get_least_maximum)
	CREX_FE(intlcal_get_greatest_minimum, arginfo_intlcal_get_greatest_minimum)
	CREX_FE(intlcal_get_locale, arginfo_intlcal_get_locale)
	CREX_FE(intlcal_get_maximum, arginfo_intlcal_get_maximum)
	CREX_FE(intlcal_get_minimal_days_in_first_week, arginfo_intlcal_get_minimal_days_in_first_week)
	CREX_FE(intlcal_set_minimal_days_in_first_week, arginfo_intlcal_set_minimal_days_in_first_week)
	CREX_FE(intlcal_get_minimum, arginfo_intlcal_get_minimum)
	CREX_FE(intlcal_get_time_zone, arginfo_intlcal_get_time_zone)
	CREX_FE(intlcal_get_type, arginfo_intlcal_get_type)
	CREX_FE(intlcal_get_weekend_transition, arginfo_intlcal_get_weekend_transition)
	CREX_FE(intlcal_in_daylight_time, arginfo_intlcal_in_daylight_time)
	CREX_FE(intlcal_is_lenient, arginfo_intlcal_is_lenient)
	CREX_FE(intlcal_is_set, arginfo_intlcal_is_set)
	CREX_FE(intlcal_is_equivalent_to, arginfo_intlcal_is_equivalent_to)
	CREX_FE(intlcal_is_weekend, arginfo_intlcal_is_weekend)
	CREX_FE(intlcal_set_first_day_of_week, arginfo_intlcal_set_first_day_of_week)
	CREX_FE(intlcal_set_lenient, arginfo_intlcal_set_lenient)
	CREX_FE(intlcal_get_repeated_wall_time_option, arginfo_intlcal_get_repeated_wall_time_option)
	CREX_FE(intlcal_equals, arginfo_intlcal_equals)
	CREX_FE(intlcal_get_skipped_wall_time_option, arginfo_intlcal_get_skipped_wall_time_option)
	CREX_FE(intlcal_set_repeated_wall_time_option, arginfo_intlcal_set_repeated_wall_time_option)
	CREX_FE(intlcal_set_skipped_wall_time_option, arginfo_intlcal_set_skipped_wall_time_option)
	CREX_FE(intlcal_from_date_time, arginfo_intlcal_from_date_time)
	CREX_FE(intlcal_to_date_time, arginfo_intlcal_to_date_time)
	CREX_FE(intlcal_get_error_code, arginfo_intlcal_get_error_code)
	CREX_FE(intlcal_get_error_message, arginfo_intlcal_get_error_message)
	CREX_FE(intlgregcal_create_instance, arginfo_intlgregcal_create_instance)
	CREX_FE(intlgregcal_set_gregorian_change, arginfo_intlgregcal_set_gregorian_change)
	CREX_FE(intlgregcal_get_gregorian_change, arginfo_intlgregcal_get_gregorian_change)
	CREX_FE(intlgregcal_is_leap_year, arginfo_intlgregcal_is_leap_year)
	CREX_FE(collator_create, arginfo_collator_create)
	CREX_FE(collator_compare, arginfo_collator_compare)
	CREX_FE(collator_get_attribute, arginfo_collator_get_attribute)
	CREX_FE(collator_set_attribute, arginfo_collator_set_attribute)
	CREX_FE(collator_get_strength, arginfo_collator_get_strength)
	CREX_FE(collator_set_strength, arginfo_collator_set_strength)
	CREX_FE(collator_sort, arginfo_collator_sort)
	CREX_FE(collator_sort_with_sort_keys, arginfo_collator_sort_with_sort_keys)
	CREX_FE(collator_asort, arginfo_collator_asort)
	CREX_FE(collator_get_locale, arginfo_collator_get_locale)
	CREX_FE(collator_get_error_code, arginfo_collator_get_error_code)
	CREX_FE(collator_get_error_message, arginfo_collator_get_error_message)
	CREX_FE(collator_get_sort_key, arginfo_collator_get_sort_key)
	CREX_FE(intl_get_error_code, arginfo_intl_get_error_code)
	CREX_FE(intl_get_error_message, arginfo_intl_get_error_message)
	CREX_FE(intl_is_failure, arginfo_intl_is_failure)
	CREX_FE(intl_error_name, arginfo_intl_error_name)
	CREX_FE(datefmt_create, arginfo_datefmt_create)
	CREX_FE(datefmt_get_datetype, arginfo_datefmt_get_datetype)
	CREX_FE(datefmt_get_timetype, arginfo_datefmt_get_timetype)
	CREX_FE(datefmt_get_calendar, arginfo_datefmt_get_calendar)
	CREX_FE(datefmt_set_calendar, arginfo_datefmt_set_calendar)
	CREX_FE(datefmt_get_timezone_id, arginfo_datefmt_get_timezone_id)
	CREX_FE(datefmt_get_calendar_object, arginfo_datefmt_get_calendar_object)
	CREX_FE(datefmt_get_timezone, arginfo_datefmt_get_timezone)
	CREX_FE(datefmt_set_timezone, arginfo_datefmt_set_timezone)
	CREX_FE(datefmt_set_pattern, arginfo_datefmt_set_pattern)
	CREX_FE(datefmt_get_pattern, arginfo_datefmt_get_pattern)
	CREX_FE(datefmt_get_locale, arginfo_datefmt_get_locale)
	CREX_FE(datefmt_set_lenient, arginfo_datefmt_set_lenient)
	CREX_FE(datefmt_is_lenient, arginfo_datefmt_is_lenient)
	CREX_FE(datefmt_format, arginfo_datefmt_format)
	CREX_FE(datefmt_format_object, arginfo_datefmt_format_object)
	CREX_FE(datefmt_parse, arginfo_datefmt_parse)
	CREX_FE(datefmt_localtime, arginfo_datefmt_localtime)
	CREX_FE(datefmt_get_error_code, arginfo_datefmt_get_error_code)
	CREX_FE(datefmt_get_error_message, arginfo_datefmt_get_error_message)
	CREX_FE(numfmt_create, arginfo_numfmt_create)
	CREX_FE(numfmt_format, arginfo_numfmt_format)
	CREX_FE(numfmt_parse, arginfo_numfmt_parse)
	CREX_FE(numfmt_format_currency, arginfo_numfmt_format_currency)
	CREX_FE(numfmt_parse_currency, arginfo_numfmt_parse_currency)
	CREX_FE(numfmt_set_attribute, arginfo_numfmt_set_attribute)
	CREX_FE(numfmt_get_attribute, arginfo_numfmt_get_attribute)
	CREX_FE(numfmt_set_text_attribute, arginfo_numfmt_set_text_attribute)
	CREX_FE(numfmt_get_text_attribute, arginfo_numfmt_get_text_attribute)
	CREX_FE(numfmt_set_symbol, arginfo_numfmt_set_symbol)
	CREX_FE(numfmt_get_symbol, arginfo_numfmt_get_symbol)
	CREX_FE(numfmt_set_pattern, arginfo_numfmt_set_pattern)
	CREX_FE(numfmt_get_pattern, arginfo_numfmt_get_pattern)
	CREX_FE(numfmt_get_locale, arginfo_numfmt_get_locale)
	CREX_FE(numfmt_get_error_code, arginfo_numfmt_get_error_code)
	CREX_FE(numfmt_get_error_message, arginfo_numfmt_get_error_message)
	CREX_FE(grapheme_strlen, arginfo_grapheme_strlen)
	CREX_FE(grapheme_strpos, arginfo_grapheme_strpos)
	CREX_FE(grapheme_stripos, arginfo_grapheme_stripos)
	CREX_FE(grapheme_strrpos, arginfo_grapheme_strrpos)
	CREX_FE(grapheme_strripos, arginfo_grapheme_strripos)
	CREX_FE(grapheme_substr, arginfo_grapheme_substr)
	CREX_FE(grapheme_strstr, arginfo_grapheme_strstr)
	CREX_FE(grapheme_stristr, arginfo_grapheme_stristr)
	CREX_FE(grapheme_extract, arginfo_grapheme_extract)
	CREX_FE(idn_to_ascii, arginfo_idn_to_ascii)
	CREX_FE(idn_to_utf8, arginfo_idn_to_utf8)
	CREX_FE(locale_get_default, arginfo_locale_get_default)
	CREX_FE(locale_set_default, arginfo_locale_set_default)
	CREX_FE(locale_get_primary_language, arginfo_locale_get_primary_language)
	CREX_FE(locale_get_script, arginfo_locale_get_script)
	CREX_FE(locale_get_region, arginfo_locale_get_region)
	CREX_FE(locale_get_keywords, arginfo_locale_get_keywords)
	CREX_FE(locale_get_display_script, arginfo_locale_get_display_script)
	CREX_FE(locale_get_display_region, arginfo_locale_get_display_region)
	CREX_FE(locale_get_display_name, arginfo_locale_get_display_name)
	CREX_FE(locale_get_display_language, arginfo_locale_get_display_language)
	CREX_FE(locale_get_display_variant, arginfo_locale_get_display_variant)
	CREX_FE(locale_compose, arginfo_locale_compose)
	CREX_FE(locale_parse, arginfo_locale_parse)
	CREX_FE(locale_get_all_variants, arginfo_locale_get_all_variants)
	CREX_FE(locale_filter_matches, arginfo_locale_filter_matches)
	CREX_FE(locale_canonicalize, arginfo_locale_canonicalize)
	CREX_FE(locale_lookup, arginfo_locale_lookup)
	CREX_FE(locale_accept_from_http, arginfo_locale_accept_from_http)
	CREX_FE(msgfmt_create, arginfo_msgfmt_create)
	CREX_FE(msgfmt_format, arginfo_msgfmt_format)
	CREX_FE(msgfmt_format_message, arginfo_msgfmt_format_message)
	CREX_FE(msgfmt_parse, arginfo_msgfmt_parse)
	CREX_FE(msgfmt_parse_message, arginfo_msgfmt_parse_message)
	CREX_FE(msgfmt_set_pattern, arginfo_msgfmt_set_pattern)
	CREX_FE(msgfmt_get_pattern, arginfo_msgfmt_get_pattern)
	CREX_FE(msgfmt_get_locale, arginfo_msgfmt_get_locale)
	CREX_FE(msgfmt_get_error_code, arginfo_msgfmt_get_error_code)
	CREX_FE(msgfmt_get_error_message, arginfo_msgfmt_get_error_message)
	CREX_FE(normalizer_normalize, arginfo_normalizer_normalize)
	CREX_FE(normalizer_is_normalized, arginfo_normalizer_is_normalized)
#if U_ICU_VERSION_MAJOR_NUM >= 56
	CREX_FE(normalizer_get_raw_decomposition, arginfo_normalizer_get_raw_decomposition)
#endif
	CREX_FE(resourcebundle_create, arginfo_resourcebundle_create)
	CREX_FE(resourcebundle_get, arginfo_resourcebundle_get)
	CREX_FE(resourcebundle_count, arginfo_resourcebundle_count)
	CREX_FE(resourcebundle_locales, arginfo_resourcebundle_locales)
	CREX_FE(resourcebundle_get_error_code, arginfo_resourcebundle_get_error_code)
	CREX_FE(resourcebundle_get_error_message, arginfo_resourcebundle_get_error_message)
	CREX_FE(intltz_count_equivalent_ids, arginfo_intltz_count_equivalent_ids)
	CREX_FE(intltz_create_default, arginfo_intltz_create_default)
	CREX_FE(intltz_create_enumeration, arginfo_intltz_create_enumeration)
	CREX_FE(intltz_create_time_zone, arginfo_intltz_create_time_zone)
	CREX_FE(intltz_create_time_zone_id_enumeration, arginfo_intltz_create_time_zone_id_enumeration)
	CREX_FE(intltz_from_date_time_zone, arginfo_intltz_from_date_time_zone)
	CREX_FE(intltz_get_canonical_id, arginfo_intltz_get_canonical_id)
	CREX_FE(intltz_get_display_name, arginfo_intltz_get_display_name)
	CREX_FE(intltz_get_dst_savings, arginfo_intltz_get_dst_savings)
	CREX_FE(intltz_get_equivalent_id, arginfo_intltz_get_equivalent_id)
	CREX_FE(intltz_get_error_code, arginfo_intltz_get_error_code)
	CREX_FE(intltz_get_error_message, arginfo_intltz_get_error_message)
	CREX_FE(intltz_get_gmt, arginfo_intltz_get_gmt)
	CREX_FE(intltz_get_id, arginfo_intltz_get_id)
	CREX_FE(intltz_get_offset, arginfo_intltz_get_offset)
	CREX_FE(intltz_get_raw_offset, arginfo_intltz_get_raw_offset)
	CREX_FE(intltz_get_region, arginfo_intltz_get_region)
	CREX_FE(intltz_get_tz_data_version, arginfo_intltz_get_tz_data_version)
	CREX_FE(intltz_get_unknown, arginfo_intltz_get_unknown)
#if U_ICU_VERSION_MAJOR_NUM >= 52
	CREX_FE(intltz_get_windows_id, arginfo_intltz_get_windows_id)
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 52
	CREX_FE(intltz_get_id_for_windows_id, arginfo_intltz_get_id_for_windows_id)
#endif
	CREX_FE(intltz_has_same_rules, arginfo_intltz_has_same_rules)
	CREX_FE(intltz_to_date_time_zone, arginfo_intltz_to_date_time_zone)
	CREX_FE(intltz_use_daylight_time, arginfo_intltz_use_daylight_time)
	CREX_FE(transliterator_create, arginfo_transliterator_create)
	CREX_FE(transliterator_create_from_rules, arginfo_transliterator_create_from_rules)
	CREX_FE(transliterator_list_ids, arginfo_transliterator_list_ids)
	CREX_FE(transliterator_create_inverse, arginfo_transliterator_create_inverse)
	CREX_FE(transliterator_transliterate, arginfo_transliterator_transliterate)
	CREX_FE(transliterator_get_error_code, arginfo_transliterator_get_error_code)
	CREX_FE(transliterator_get_error_message, arginfo_transliterator_get_error_message)
	CREX_FE_END
};


static const crex_function_entry class_IntlException_methods[] = {
	CREX_FE_END
};

static void register_crx_intl_symbols(int module_number)
{
	REGISTER_LONG_CONSTANT("INTL_MAX_LOCALE_LEN", INTL_MAX_LOCALE_LEN, CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("INTL_ICU_VERSION", U_ICU_VERSION, CONST_PERSISTENT);
#if defined(U_ICU_DATA_VERSION)
	REGISTER_STRING_CONSTANT("INTL_ICU_DATA_VERSION", U_ICU_DATA_VERSION, CONST_PERSISTENT);
#endif
	REGISTER_LONG_CONSTANT("GRAPHEME_EXTR_COUNT", GRAPHEME_EXTRACT_TYPE_COUNT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GRAPHEME_EXTR_MAXBYTES", GRAPHEME_EXTRACT_TYPE_MAXBYTES, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GRAPHEME_EXTR_MAXCHARS", GRAPHEME_EXTRACT_TYPE_MAXCHARS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IDNA_DEFAULT", UIDNA_DEFAULT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IDNA_ALLOW_UNASSIGNED", UIDNA_ALLOW_UNASSIGNED, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IDNA_USE_STD3_RULES", UIDNA_USE_STD3_RULES, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IDNA_CHECK_BIDI", UIDNA_CHECK_BIDI, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IDNA_CHECK_CONTEXTJ", UIDNA_CHECK_CONTEXTJ, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IDNA_NONTRANSITIONAL_TO_ASCII", UIDNA_NONTRANSITIONAL_TO_ASCII, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IDNA_NONTRANSITIONAL_TO_UNICODE", UIDNA_NONTRANSITIONAL_TO_UNICODE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("INTL_IDNA_VARIANT_UTS46", INTL_IDN_VARIANT_UTS46, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IDNA_ERROR_EMPTY_LABEL", UIDNA_ERROR_EMPTY_LABEL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IDNA_ERROR_LABEL_TOO_LONG", UIDNA_ERROR_LABEL_TOO_LONG, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IDNA_ERROR_DOMAIN_NAME_TOO_LONG", UIDNA_ERROR_DOMAIN_NAME_TOO_LONG, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IDNA_ERROR_LEADING_HYPHEN", UIDNA_ERROR_LEADING_HYPHEN, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IDNA_ERROR_TRAILING_HYPHEN", UIDNA_ERROR_TRAILING_HYPHEN, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IDNA_ERROR_HYPHEN_3_4", UIDNA_ERROR_HYPHEN_3_4, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IDNA_ERROR_LEADING_COMBINING_MARK", UIDNA_ERROR_LEADING_COMBINING_MARK, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IDNA_ERROR_DISALLOWED", UIDNA_ERROR_DISALLOWED, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IDNA_ERROR_PUNYCODE", UIDNA_ERROR_PUNYCODE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IDNA_ERROR_LABEL_HAS_DOT", UIDNA_ERROR_LABEL_HAS_DOT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IDNA_ERROR_INVALID_ACE_LABEL", UIDNA_ERROR_INVALID_ACE_LABEL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IDNA_ERROR_BIDI", UIDNA_ERROR_BIDI, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IDNA_ERROR_CONTEXTJ", UIDNA_ERROR_CONTEXTJ, CONST_PERSISTENT);
}

static crex_class_entry *register_class_IntlException(crex_class_entry *class_entry_Exception)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "IntlException", class_IntlException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Exception);

	return class_entry;
}
