/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 1eb2511da8ecb00132a00d1f3c95e03f9463db55 */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_IntlCalendar___main, 0, 0, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_IntlCalendar_createInstance, 0, 0, IntlCalendar, 1)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, timezone, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, locale, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlCalendar_equals, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, other, IntlCalendar, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlCalendar_fieldDifference, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, timestamp, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, field, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlCalendar_add, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, field, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_IntlCalendar_after arginfo_class_IntlCalendar_equals

#define arginfo_class_IntlCalendar_before arginfo_class_IntlCalendar_equals

CREX_BEGIN_ARG_INFO_EX(arginfo_class_IntlCalendar_clear, 0, 0, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, field, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_IntlCalendar_fromDateTime, 0, 1, IntlCalendar, 1)
	CREX_ARG_OBJ_TYPE_MASK(0, datetime, DateTime, MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, locale, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlCalendar_get, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, field, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_IntlCalendar_getActualMaximum arginfo_class_IntlCalendar_get

#define arginfo_class_IntlCalendar_getActualMinimum arginfo_class_IntlCalendar_get

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlCalendar_getAvailableLocales, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlCalendar_getDayOfWeekType, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, dayOfWeek, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlCalendar_getErrorCode, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlCalendar_getErrorMessage, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
CREX_END_ARG_INFO()

#define arginfo_class_IntlCalendar_getFirstDayOfWeek arginfo_class_IntlCalendar_getErrorCode

#define arginfo_class_IntlCalendar_getGreatestMinimum arginfo_class_IntlCalendar_get

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_IntlCalendar_getKeywordValuesForLocale, 0, 3, IntlIterator, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, keyword, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, onlyCommon, _IS_BOOL, 0)
CREX_END_ARG_INFO()

#define arginfo_class_IntlCalendar_getLeastMaximum arginfo_class_IntlCalendar_get

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlCalendar_getLocale, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, type, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_IntlCalendar_getMaximum arginfo_class_IntlCalendar_get

#define arginfo_class_IntlCalendar_getMinimalDaysInFirstWeek arginfo_class_IntlCalendar_getErrorCode

CREX_BEGIN_ARG_INFO_EX(arginfo_class_IntlCalendar_setMinimalDaysInFirstWeek, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, days, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_IntlCalendar_getMinimum arginfo_class_IntlCalendar_get

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlCalendar_getNow, 0, 0, IS_DOUBLE, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlCalendar_getRepeatedWallTimeOption, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_IntlCalendar_getSkippedWallTimeOption arginfo_class_IntlCalendar_getRepeatedWallTimeOption

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlCalendar_getTime, 0, 0, MAY_BE_DOUBLE|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_IntlCalendar_getTimeZone, 0, 0, IntlTimeZone, MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlCalendar_getType, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_IntlCalendar_getWeekendTransition arginfo_class_IntlCalendar_getDayOfWeekType

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlCalendar_inDaylightTime, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

#define arginfo_class_IntlCalendar_isEquivalentTo arginfo_class_IntlCalendar_equals

#define arginfo_class_IntlCalendar_isLenient arginfo_class_IntlCalendar_inDaylightTime

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlCalendar_isWeekend, 0, 0, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timestamp, IS_DOUBLE, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlCalendar_roll, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, field, IS_LONG, 0)
	CREX_ARG_INFO(0, value)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlCalendar_isSet, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, field, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_IntlCalendar_set, 0, 0, 2)
	CREX_ARG_TYPE_INFO(0, year, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, month, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, dayOfMonth, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, hour, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, minute, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, second, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_IntlCalendar_setDate, 0, 3, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, year, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, month, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, dayOfMonth, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_IntlCalendar_setDateTime, 0, 5, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, year, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, month, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, dayOfMonth, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, hour, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, minute, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, second, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_IntlCalendar_setFirstDayOfWeek, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, dayOfWeek, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_IntlCalendar_setLenient, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, lenient, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_IntlCalendar_setRepeatedWallTimeOption, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, option, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_IntlCalendar_setSkippedWallTimeOption arginfo_class_IntlCalendar_setRepeatedWallTimeOption

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlCalendar_setTime, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, timestamp, IS_DOUBLE, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlCalendar_setTimeZone, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, timezone)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_IntlCalendar_toDateTime, 0, 0, DateTime, MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_IntlGregorianCalendar_createFromDate, 0, 3, IS_STATIC, 0)
	CREX_ARG_TYPE_INFO(0, year, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, month, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, dayOfMonth, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_IntlGregorianCalendar_createFromDateTime, 0, 5, IS_STATIC, 0)
	CREX_ARG_TYPE_INFO(0, year, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, month, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, dayOfMonth, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, hour, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, minute, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, second, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_IntlGregorianCalendar___main, 0, 0, 0)
	CREX_ARG_INFO(0, timezoneOrYear)
	CREX_ARG_INFO(0, localeOrMonth)
	CREX_ARG_INFO(0, day)
	CREX_ARG_INFO(0, hour)
	CREX_ARG_INFO(0, minute)
	CREX_ARG_INFO(0, second)
CREX_END_ARG_INFO()

#define arginfo_class_IntlGregorianCalendar_setGregorianChange arginfo_class_IntlCalendar_setTime

#define arginfo_class_IntlGregorianCalendar_getGregorianChange arginfo_class_IntlCalendar_getNow

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlGregorianCalendar_isLeapYear, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, year, IS_LONG, 0)
CREX_END_ARG_INFO()


CREX_METHOD(IntlCalendar, __main);
CREX_FUNCTION(intlcal_create_instance);
CREX_FUNCTION(intlcal_equals);
CREX_FUNCTION(intlcal_field_difference);
CREX_FUNCTION(intlcal_add);
CREX_FUNCTION(intlcal_after);
CREX_FUNCTION(intlcal_before);
CREX_FUNCTION(intlcal_clear);
CREX_FUNCTION(intlcal_from_date_time);
CREX_FUNCTION(intlcal_get);
CREX_FUNCTION(intlcal_get_actual_maximum);
CREX_FUNCTION(intlcal_get_actual_minimum);
CREX_FUNCTION(intlcal_get_available_locales);
CREX_FUNCTION(intlcal_get_day_of_week_type);
CREX_FUNCTION(intlcal_get_error_code);
CREX_FUNCTION(intlcal_get_error_message);
CREX_FUNCTION(intlcal_get_first_day_of_week);
CREX_FUNCTION(intlcal_get_greatest_minimum);
CREX_FUNCTION(intlcal_get_keyword_values_for_locale);
CREX_FUNCTION(intlcal_get_least_maximum);
CREX_FUNCTION(intlcal_get_locale);
CREX_FUNCTION(intlcal_get_maximum);
CREX_FUNCTION(intlcal_get_minimal_days_in_first_week);
CREX_FUNCTION(intlcal_set_minimal_days_in_first_week);
CREX_FUNCTION(intlcal_get_minimum);
CREX_FUNCTION(intlcal_get_now);
CREX_FUNCTION(intlcal_get_repeated_wall_time_option);
CREX_FUNCTION(intlcal_get_skipped_wall_time_option);
CREX_FUNCTION(intlcal_get_time);
CREX_FUNCTION(intlcal_get_time_zone);
CREX_FUNCTION(intlcal_get_type);
CREX_FUNCTION(intlcal_get_weekend_transition);
CREX_FUNCTION(intlcal_in_daylight_time);
CREX_FUNCTION(intlcal_is_equivalent_to);
CREX_FUNCTION(intlcal_is_lenient);
CREX_FUNCTION(intlcal_is_weekend);
CREX_FUNCTION(intlcal_roll);
CREX_FUNCTION(intlcal_is_set);
CREX_FUNCTION(intlcal_set);
CREX_METHOD(IntlCalendar, setDate);
CREX_METHOD(IntlCalendar, setDateTime);
CREX_FUNCTION(intlcal_set_first_day_of_week);
CREX_FUNCTION(intlcal_set_lenient);
CREX_FUNCTION(intlcal_set_repeated_wall_time_option);
CREX_FUNCTION(intlcal_set_skipped_wall_time_option);
CREX_FUNCTION(intlcal_set_time);
CREX_FUNCTION(intlcal_set_time_zone);
CREX_FUNCTION(intlcal_to_date_time);
CREX_METHOD(IntlGregorianCalendar, createFromDate);
CREX_METHOD(IntlGregorianCalendar, createFromDateTime);
CREX_METHOD(IntlGregorianCalendar, __main);
CREX_FUNCTION(intlgregcal_set_gregorian_change);
CREX_FUNCTION(intlgregcal_get_gregorian_change);
CREX_FUNCTION(intlgregcal_is_leap_year);


static const crex_function_entry class_IntlCalendar_methods[] = {
	CREX_ME(IntlCalendar, __main, arginfo_class_IntlCalendar___main, CREX_ACC_PRIVATE)
	CREX_ME_MAPPING(createInstance, intlcal_create_instance, arginfo_class_IntlCalendar_createInstance, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(equals, intlcal_equals, arginfo_class_IntlCalendar_equals, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(fieldDifference, intlcal_field_difference, arginfo_class_IntlCalendar_fieldDifference, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(add, intlcal_add, arginfo_class_IntlCalendar_add, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(after, intlcal_after, arginfo_class_IntlCalendar_after, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(before, intlcal_before, arginfo_class_IntlCalendar_before, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(clear, intlcal_clear, arginfo_class_IntlCalendar_clear, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(fromDateTime, intlcal_from_date_time, arginfo_class_IntlCalendar_fromDateTime, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(get, intlcal_get, arginfo_class_IntlCalendar_get, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getActualMaximum, intlcal_get_actual_maximum, arginfo_class_IntlCalendar_getActualMaximum, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getActualMinimum, intlcal_get_actual_minimum, arginfo_class_IntlCalendar_getActualMinimum, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getAvailableLocales, intlcal_get_available_locales, arginfo_class_IntlCalendar_getAvailableLocales, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getDayOfWeekType, intlcal_get_day_of_week_type, arginfo_class_IntlCalendar_getDayOfWeekType, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getErrorCode, intlcal_get_error_code, arginfo_class_IntlCalendar_getErrorCode, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getErrorMessage, intlcal_get_error_message, arginfo_class_IntlCalendar_getErrorMessage, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getFirstDayOfWeek, intlcal_get_first_day_of_week, arginfo_class_IntlCalendar_getFirstDayOfWeek, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getGreatestMinimum, intlcal_get_greatest_minimum, arginfo_class_IntlCalendar_getGreatestMinimum, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getKeywordValuesForLocale, intlcal_get_keyword_values_for_locale, arginfo_class_IntlCalendar_getKeywordValuesForLocale, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getLeastMaximum, intlcal_get_least_maximum, arginfo_class_IntlCalendar_getLeastMaximum, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getLocale, intlcal_get_locale, arginfo_class_IntlCalendar_getLocale, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getMaximum, intlcal_get_maximum, arginfo_class_IntlCalendar_getMaximum, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getMinimalDaysInFirstWeek, intlcal_get_minimal_days_in_first_week, arginfo_class_IntlCalendar_getMinimalDaysInFirstWeek, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setMinimalDaysInFirstWeek, intlcal_set_minimal_days_in_first_week, arginfo_class_IntlCalendar_setMinimalDaysInFirstWeek, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getMinimum, intlcal_get_minimum, arginfo_class_IntlCalendar_getMinimum, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getNow, intlcal_get_now, arginfo_class_IntlCalendar_getNow, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getRepeatedWallTimeOption, intlcal_get_repeated_wall_time_option, arginfo_class_IntlCalendar_getRepeatedWallTimeOption, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getSkippedWallTimeOption, intlcal_get_skipped_wall_time_option, arginfo_class_IntlCalendar_getSkippedWallTimeOption, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getTime, intlcal_get_time, arginfo_class_IntlCalendar_getTime, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getTimeZone, intlcal_get_time_zone, arginfo_class_IntlCalendar_getTimeZone, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getType, intlcal_get_type, arginfo_class_IntlCalendar_getType, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getWeekendTransition, intlcal_get_weekend_transition, arginfo_class_IntlCalendar_getWeekendTransition, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(inDaylightTime, intlcal_in_daylight_time, arginfo_class_IntlCalendar_inDaylightTime, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(isEquivalentTo, intlcal_is_equivalent_to, arginfo_class_IntlCalendar_isEquivalentTo, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(isLenient, intlcal_is_lenient, arginfo_class_IntlCalendar_isLenient, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(isWeekend, intlcal_is_weekend, arginfo_class_IntlCalendar_isWeekend, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(roll, intlcal_roll, arginfo_class_IntlCalendar_roll, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(isSet, intlcal_is_set, arginfo_class_IntlCalendar_isSet, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(set, intlcal_set, arginfo_class_IntlCalendar_set, CREX_ACC_PUBLIC)
	CREX_ME(IntlCalendar, setDate, arginfo_class_IntlCalendar_setDate, CREX_ACC_PUBLIC)
	CREX_ME(IntlCalendar, setDateTime, arginfo_class_IntlCalendar_setDateTime, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setFirstDayOfWeek, intlcal_set_first_day_of_week, arginfo_class_IntlCalendar_setFirstDayOfWeek, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setLenient, intlcal_set_lenient, arginfo_class_IntlCalendar_setLenient, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setRepeatedWallTimeOption, intlcal_set_repeated_wall_time_option, arginfo_class_IntlCalendar_setRepeatedWallTimeOption, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setSkippedWallTimeOption, intlcal_set_skipped_wall_time_option, arginfo_class_IntlCalendar_setSkippedWallTimeOption, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setTime, intlcal_set_time, arginfo_class_IntlCalendar_setTime, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setTimeZone, intlcal_set_time_zone, arginfo_class_IntlCalendar_setTimeZone, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(toDateTime, intlcal_to_date_time, arginfo_class_IntlCalendar_toDateTime, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_IntlGregorianCalendar_methods[] = {
	CREX_ME(IntlGregorianCalendar, createFromDate, arginfo_class_IntlGregorianCalendar_createFromDate, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlGregorianCalendar, createFromDateTime, arginfo_class_IntlGregorianCalendar_createFromDateTime, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlGregorianCalendar, __main, arginfo_class_IntlGregorianCalendar___main, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setGregorianChange, intlgregcal_set_gregorian_change, arginfo_class_IntlGregorianCalendar_setGregorianChange, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getGregorianChange, intlgregcal_get_gregorian_change, arginfo_class_IntlGregorianCalendar_getGregorianChange, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(isLeapYear, intlgregcal_is_leap_year, arginfo_class_IntlGregorianCalendar_isLeapYear, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_IntlCalendar(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "IntlCalendar", class_IntlCalendar_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;

	zval const_FIELD_ERA_value;
	ZVAL_LONG(&const_FIELD_ERA_value, UCAL_ERA);
	crex_string *const_FIELD_ERA_name = crex_string_init_interned("FIELD_ERA", sizeof("FIELD_ERA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_ERA_name, &const_FIELD_ERA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_ERA_name);

	zval const_FIELD_YEAR_value;
	ZVAL_LONG(&const_FIELD_YEAR_value, UCAL_YEAR);
	crex_string *const_FIELD_YEAR_name = crex_string_init_interned("FIELD_YEAR", sizeof("FIELD_YEAR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_YEAR_name, &const_FIELD_YEAR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_YEAR_name);

	zval const_FIELD_MONTH_value;
	ZVAL_LONG(&const_FIELD_MONTH_value, UCAL_MONTH);
	crex_string *const_FIELD_MONTH_name = crex_string_init_interned("FIELD_MONTH", sizeof("FIELD_MONTH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_MONTH_name, &const_FIELD_MONTH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_MONTH_name);

	zval const_FIELD_WEEK_OF_YEAR_value;
	ZVAL_LONG(&const_FIELD_WEEK_OF_YEAR_value, UCAL_WEEK_OF_YEAR);
	crex_string *const_FIELD_WEEK_OF_YEAR_name = crex_string_init_interned("FIELD_WEEK_OF_YEAR", sizeof("FIELD_WEEK_OF_YEAR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_WEEK_OF_YEAR_name, &const_FIELD_WEEK_OF_YEAR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_WEEK_OF_YEAR_name);

	zval const_FIELD_WEEK_OF_MONTH_value;
	ZVAL_LONG(&const_FIELD_WEEK_OF_MONTH_value, UCAL_WEEK_OF_MONTH);
	crex_string *const_FIELD_WEEK_OF_MONTH_name = crex_string_init_interned("FIELD_WEEK_OF_MONTH", sizeof("FIELD_WEEK_OF_MONTH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_WEEK_OF_MONTH_name, &const_FIELD_WEEK_OF_MONTH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_WEEK_OF_MONTH_name);

	zval const_FIELD_DATE_value;
	ZVAL_LONG(&const_FIELD_DATE_value, UCAL_DATE);
	crex_string *const_FIELD_DATE_name = crex_string_init_interned("FIELD_DATE", sizeof("FIELD_DATE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_DATE_name, &const_FIELD_DATE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_DATE_name);

	zval const_FIELD_DAY_OF_YEAR_value;
	ZVAL_LONG(&const_FIELD_DAY_OF_YEAR_value, UCAL_DAY_OF_YEAR);
	crex_string *const_FIELD_DAY_OF_YEAR_name = crex_string_init_interned("FIELD_DAY_OF_YEAR", sizeof("FIELD_DAY_OF_YEAR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_DAY_OF_YEAR_name, &const_FIELD_DAY_OF_YEAR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_DAY_OF_YEAR_name);

	zval const_FIELD_DAY_OF_WEEK_value;
	ZVAL_LONG(&const_FIELD_DAY_OF_WEEK_value, UCAL_DAY_OF_WEEK);
	crex_string *const_FIELD_DAY_OF_WEEK_name = crex_string_init_interned("FIELD_DAY_OF_WEEK", sizeof("FIELD_DAY_OF_WEEK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_DAY_OF_WEEK_name, &const_FIELD_DAY_OF_WEEK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_DAY_OF_WEEK_name);

	zval const_FIELD_DAY_OF_WEEK_IN_MONTH_value;
	ZVAL_LONG(&const_FIELD_DAY_OF_WEEK_IN_MONTH_value, UCAL_DAY_OF_WEEK_IN_MONTH);
	crex_string *const_FIELD_DAY_OF_WEEK_IN_MONTH_name = crex_string_init_interned("FIELD_DAY_OF_WEEK_IN_MONTH", sizeof("FIELD_DAY_OF_WEEK_IN_MONTH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_DAY_OF_WEEK_IN_MONTH_name, &const_FIELD_DAY_OF_WEEK_IN_MONTH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_DAY_OF_WEEK_IN_MONTH_name);

	zval const_FIELD_AM_PM_value;
	ZVAL_LONG(&const_FIELD_AM_PM_value, UCAL_AM_PM);
	crex_string *const_FIELD_AM_PM_name = crex_string_init_interned("FIELD_AM_PM", sizeof("FIELD_AM_PM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_AM_PM_name, &const_FIELD_AM_PM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_AM_PM_name);

	zval const_FIELD_HOUR_value;
	ZVAL_LONG(&const_FIELD_HOUR_value, UCAL_HOUR);
	crex_string *const_FIELD_HOUR_name = crex_string_init_interned("FIELD_HOUR", sizeof("FIELD_HOUR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_HOUR_name, &const_FIELD_HOUR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_HOUR_name);

	zval const_FIELD_HOUR_OF_DAY_value;
	ZVAL_LONG(&const_FIELD_HOUR_OF_DAY_value, UCAL_HOUR_OF_DAY);
	crex_string *const_FIELD_HOUR_OF_DAY_name = crex_string_init_interned("FIELD_HOUR_OF_DAY", sizeof("FIELD_HOUR_OF_DAY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_HOUR_OF_DAY_name, &const_FIELD_HOUR_OF_DAY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_HOUR_OF_DAY_name);

	zval const_FIELD_MINUTE_value;
	ZVAL_LONG(&const_FIELD_MINUTE_value, UCAL_MINUTE);
	crex_string *const_FIELD_MINUTE_name = crex_string_init_interned("FIELD_MINUTE", sizeof("FIELD_MINUTE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_MINUTE_name, &const_FIELD_MINUTE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_MINUTE_name);

	zval const_FIELD_SECOND_value;
	ZVAL_LONG(&const_FIELD_SECOND_value, UCAL_SECOND);
	crex_string *const_FIELD_SECOND_name = crex_string_init_interned("FIELD_SECOND", sizeof("FIELD_SECOND") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_SECOND_name, &const_FIELD_SECOND_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_SECOND_name);

	zval const_FIELD_MILLISECOND_value;
	ZVAL_LONG(&const_FIELD_MILLISECOND_value, UCAL_MILLISECOND);
	crex_string *const_FIELD_MILLISECOND_name = crex_string_init_interned("FIELD_MILLISECOND", sizeof("FIELD_MILLISECOND") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_MILLISECOND_name, &const_FIELD_MILLISECOND_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_MILLISECOND_name);

	zval const_FIELD_ZONE_OFFSET_value;
	ZVAL_LONG(&const_FIELD_ZONE_OFFSET_value, UCAL_ZONE_OFFSET);
	crex_string *const_FIELD_ZONE_OFFSET_name = crex_string_init_interned("FIELD_ZONE_OFFSET", sizeof("FIELD_ZONE_OFFSET") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_ZONE_OFFSET_name, &const_FIELD_ZONE_OFFSET_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_ZONE_OFFSET_name);

	zval const_FIELD_DST_OFFSET_value;
	ZVAL_LONG(&const_FIELD_DST_OFFSET_value, UCAL_DST_OFFSET);
	crex_string *const_FIELD_DST_OFFSET_name = crex_string_init_interned("FIELD_DST_OFFSET", sizeof("FIELD_DST_OFFSET") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_DST_OFFSET_name, &const_FIELD_DST_OFFSET_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_DST_OFFSET_name);

	zval const_FIELD_YEAR_WOY_value;
	ZVAL_LONG(&const_FIELD_YEAR_WOY_value, UCAL_YEAR_WOY);
	crex_string *const_FIELD_YEAR_WOY_name = crex_string_init_interned("FIELD_YEAR_WOY", sizeof("FIELD_YEAR_WOY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_YEAR_WOY_name, &const_FIELD_YEAR_WOY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_YEAR_WOY_name);

	zval const_FIELD_DOW_LOCAL_value;
	ZVAL_LONG(&const_FIELD_DOW_LOCAL_value, UCAL_DOW_LOCAL);
	crex_string *const_FIELD_DOW_LOCAL_name = crex_string_init_interned("FIELD_DOW_LOCAL", sizeof("FIELD_DOW_LOCAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_DOW_LOCAL_name, &const_FIELD_DOW_LOCAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_DOW_LOCAL_name);

	zval const_FIELD_EXTENDED_YEAR_value;
	ZVAL_LONG(&const_FIELD_EXTENDED_YEAR_value, UCAL_EXTENDED_YEAR);
	crex_string *const_FIELD_EXTENDED_YEAR_name = crex_string_init_interned("FIELD_EXTENDED_YEAR", sizeof("FIELD_EXTENDED_YEAR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_EXTENDED_YEAR_name, &const_FIELD_EXTENDED_YEAR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_EXTENDED_YEAR_name);

	zval const_FIELD_JULIAN_DAY_value;
	ZVAL_LONG(&const_FIELD_JULIAN_DAY_value, UCAL_JULIAN_DAY);
	crex_string *const_FIELD_JULIAN_DAY_name = crex_string_init_interned("FIELD_JULIAN_DAY", sizeof("FIELD_JULIAN_DAY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_JULIAN_DAY_name, &const_FIELD_JULIAN_DAY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_JULIAN_DAY_name);

	zval const_FIELD_MILLISECONDS_IN_DAY_value;
	ZVAL_LONG(&const_FIELD_MILLISECONDS_IN_DAY_value, UCAL_MILLISECONDS_IN_DAY);
	crex_string *const_FIELD_MILLISECONDS_IN_DAY_name = crex_string_init_interned("FIELD_MILLISECONDS_IN_DAY", sizeof("FIELD_MILLISECONDS_IN_DAY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_MILLISECONDS_IN_DAY_name, &const_FIELD_MILLISECONDS_IN_DAY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_MILLISECONDS_IN_DAY_name);

	zval const_FIELD_IS_LEAP_MONTH_value;
	ZVAL_LONG(&const_FIELD_IS_LEAP_MONTH_value, UCAL_IS_LEAP_MONTH);
	crex_string *const_FIELD_IS_LEAP_MONTH_name = crex_string_init_interned("FIELD_IS_LEAP_MONTH", sizeof("FIELD_IS_LEAP_MONTH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_IS_LEAP_MONTH_name, &const_FIELD_IS_LEAP_MONTH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_IS_LEAP_MONTH_name);

	zval const_FIELD_FIELD_COUNT_value;
	ZVAL_LONG(&const_FIELD_FIELD_COUNT_value, UCAL_FIELD_COUNT);
	crex_string *const_FIELD_FIELD_COUNT_name = crex_string_init_interned("FIELD_FIELD_COUNT", sizeof("FIELD_FIELD_COUNT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_FIELD_COUNT_name, &const_FIELD_FIELD_COUNT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_FIELD_COUNT_name);

	zval const_FIELD_DAY_OF_MONTH_value;
	ZVAL_LONG(&const_FIELD_DAY_OF_MONTH_value, UCAL_DAY_OF_MONTH);
	crex_string *const_FIELD_DAY_OF_MONTH_name = crex_string_init_interned("FIELD_DAY_OF_MONTH", sizeof("FIELD_DAY_OF_MONTH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FIELD_DAY_OF_MONTH_name, &const_FIELD_DAY_OF_MONTH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FIELD_DAY_OF_MONTH_name);

	zval const_DOW_SUNDAY_value;
	ZVAL_LONG(&const_DOW_SUNDAY_value, UCAL_SUNDAY);
	crex_string *const_DOW_SUNDAY_name = crex_string_init_interned("DOW_SUNDAY", sizeof("DOW_SUNDAY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DOW_SUNDAY_name, &const_DOW_SUNDAY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DOW_SUNDAY_name);

	zval const_DOW_MONDAY_value;
	ZVAL_LONG(&const_DOW_MONDAY_value, UCAL_MONDAY);
	crex_string *const_DOW_MONDAY_name = crex_string_init_interned("DOW_MONDAY", sizeof("DOW_MONDAY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DOW_MONDAY_name, &const_DOW_MONDAY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DOW_MONDAY_name);

	zval const_DOW_TUESDAY_value;
	ZVAL_LONG(&const_DOW_TUESDAY_value, UCAL_TUESDAY);
	crex_string *const_DOW_TUESDAY_name = crex_string_init_interned("DOW_TUESDAY", sizeof("DOW_TUESDAY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DOW_TUESDAY_name, &const_DOW_TUESDAY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DOW_TUESDAY_name);

	zval const_DOW_WEDNESDAY_value;
	ZVAL_LONG(&const_DOW_WEDNESDAY_value, UCAL_WEDNESDAY);
	crex_string *const_DOW_WEDNESDAY_name = crex_string_init_interned("DOW_WEDNESDAY", sizeof("DOW_WEDNESDAY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DOW_WEDNESDAY_name, &const_DOW_WEDNESDAY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DOW_WEDNESDAY_name);

	zval const_DOW_THURSDAY_value;
	ZVAL_LONG(&const_DOW_THURSDAY_value, UCAL_THURSDAY);
	crex_string *const_DOW_THURSDAY_name = crex_string_init_interned("DOW_THURSDAY", sizeof("DOW_THURSDAY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DOW_THURSDAY_name, &const_DOW_THURSDAY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DOW_THURSDAY_name);

	zval const_DOW_FRIDAY_value;
	ZVAL_LONG(&const_DOW_FRIDAY_value, UCAL_FRIDAY);
	crex_string *const_DOW_FRIDAY_name = crex_string_init_interned("DOW_FRIDAY", sizeof("DOW_FRIDAY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DOW_FRIDAY_name, &const_DOW_FRIDAY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DOW_FRIDAY_name);

	zval const_DOW_SATURDAY_value;
	ZVAL_LONG(&const_DOW_SATURDAY_value, UCAL_SATURDAY);
	crex_string *const_DOW_SATURDAY_name = crex_string_init_interned("DOW_SATURDAY", sizeof("DOW_SATURDAY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DOW_SATURDAY_name, &const_DOW_SATURDAY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DOW_SATURDAY_name);

	zval const_DOW_TYPE_WEEKDAY_value;
	ZVAL_LONG(&const_DOW_TYPE_WEEKDAY_value, UCAL_WEEKDAY);
	crex_string *const_DOW_TYPE_WEEKDAY_name = crex_string_init_interned("DOW_TYPE_WEEKDAY", sizeof("DOW_TYPE_WEEKDAY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DOW_TYPE_WEEKDAY_name, &const_DOW_TYPE_WEEKDAY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DOW_TYPE_WEEKDAY_name);

	zval const_DOW_TYPE_WEEKEND_value;
	ZVAL_LONG(&const_DOW_TYPE_WEEKEND_value, UCAL_WEEKEND);
	crex_string *const_DOW_TYPE_WEEKEND_name = crex_string_init_interned("DOW_TYPE_WEEKEND", sizeof("DOW_TYPE_WEEKEND") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DOW_TYPE_WEEKEND_name, &const_DOW_TYPE_WEEKEND_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DOW_TYPE_WEEKEND_name);

	zval const_DOW_TYPE_WEEKEND_OFFSET_value;
	ZVAL_LONG(&const_DOW_TYPE_WEEKEND_OFFSET_value, UCAL_WEEKEND_ONSET);
	crex_string *const_DOW_TYPE_WEEKEND_OFFSET_name = crex_string_init_interned("DOW_TYPE_WEEKEND_OFFSET", sizeof("DOW_TYPE_WEEKEND_OFFSET") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DOW_TYPE_WEEKEND_OFFSET_name, &const_DOW_TYPE_WEEKEND_OFFSET_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DOW_TYPE_WEEKEND_OFFSET_name);

	zval const_DOW_TYPE_WEEKEND_CEASE_value;
	ZVAL_LONG(&const_DOW_TYPE_WEEKEND_CEASE_value, UCAL_WEEKEND_CEASE);
	crex_string *const_DOW_TYPE_WEEKEND_CEASE_name = crex_string_init_interned("DOW_TYPE_WEEKEND_CEASE", sizeof("DOW_TYPE_WEEKEND_CEASE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DOW_TYPE_WEEKEND_CEASE_name, &const_DOW_TYPE_WEEKEND_CEASE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DOW_TYPE_WEEKEND_CEASE_name);

	zval const_WALLTIME_FIRST_value;
	ZVAL_LONG(&const_WALLTIME_FIRST_value, UCAL_WALLTIME_FIRST);
	crex_string *const_WALLTIME_FIRST_name = crex_string_init_interned("WALLTIME_FIRST", sizeof("WALLTIME_FIRST") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WALLTIME_FIRST_name, &const_WALLTIME_FIRST_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WALLTIME_FIRST_name);

	zval const_WALLTIME_LAST_value;
	ZVAL_LONG(&const_WALLTIME_LAST_value, UCAL_WALLTIME_LAST);
	crex_string *const_WALLTIME_LAST_name = crex_string_init_interned("WALLTIME_LAST", sizeof("WALLTIME_LAST") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WALLTIME_LAST_name, &const_WALLTIME_LAST_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WALLTIME_LAST_name);

	zval const_WALLTIME_NEXT_VALID_value;
	ZVAL_LONG(&const_WALLTIME_NEXT_VALID_value, UCAL_WALLTIME_NEXT_VALID);
	crex_string *const_WALLTIME_NEXT_VALID_name = crex_string_init_interned("WALLTIME_NEXT_VALID", sizeof("WALLTIME_NEXT_VALID") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WALLTIME_NEXT_VALID_name, &const_WALLTIME_NEXT_VALID_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WALLTIME_NEXT_VALID_name);

	return class_entry;
}

static crex_class_entry *register_class_IntlGregorianCalendar(crex_class_entry *class_entry_IntlCalendar)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "IntlGregorianCalendar", class_IntlGregorianCalendar_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_IntlCalendar);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}
