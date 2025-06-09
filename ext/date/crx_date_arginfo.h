/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 0f204ac6646be79b515189a384fce9bcea9a4f42 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_strtotime, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, datetime, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, baseTimestamp, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_date, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timestamp, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_idate, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timestamp, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_gmdate arginfo_date

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mktime, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, hour, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, minute, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, second, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, month, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, day, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, year, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_gmmktime arginfo_mktime

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_checkdate, 0, 3, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, month, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, day, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, year, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_strftime, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timestamp, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_gmstrftime arginfo_strftime

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_time, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_localtime, 0, 0, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timestamp, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, associative, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_getdate, 0, 0, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timestamp, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_date_create, 0, 0, DateTime, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, datetime, IS_STRING, 0, "\"now\"")
	CREX_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, timezone, DateTimeZone, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_date_create_immutable, 0, 0, DateTimeImmutable, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, datetime, IS_STRING, 0, "\"now\"")
	CREX_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, timezone, DateTimeZone, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_date_create_from_format, 0, 2, DateTime, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, datetime, IS_STRING, 0)
	CREX_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, timezone, DateTimeZone, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_date_create_immutable_from_format, 0, 2, DateTimeImmutable, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, datetime, IS_STRING, 0)
	CREX_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, timezone, DateTimeZone, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_date_parse, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, datetime, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_date_parse_from_format, 0, 2, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, datetime, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_date_get_last_errors, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_date_format, 0, 2, IS_STRING, 0)
	CREX_ARG_OBJ_INFO(0, object, DateTimeInterface, 0)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_date_modify, 0, 2, DateTime, MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, object, DateTime, 0)
	CREX_ARG_TYPE_INFO(0, modifier, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_date_add, 0, 2, DateTime, 0)
	CREX_ARG_OBJ_INFO(0, object, DateTime, 0)
	CREX_ARG_OBJ_INFO(0, interval, DateInterval, 0)
CREX_END_ARG_INFO()

#define arginfo_date_sub arginfo_date_add

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_date_timezone_get, 0, 1, DateTimeZone, MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, object, DateTimeInterface, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_date_timezone_set, 0, 2, DateTime, 0)
	CREX_ARG_OBJ_INFO(0, object, DateTime, 0)
	CREX_ARG_OBJ_INFO(0, timezone, DateTimeZone, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_date_offset_get, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, object, DateTimeInterface, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_date_diff, 0, 2, DateInterval, 0)
	CREX_ARG_OBJ_INFO(0, baseObject, DateTimeInterface, 0)
	CREX_ARG_OBJ_INFO(0, targetObject, DateTimeInterface, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, absolute, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_date_time_set, 0, 3, DateTime, 0)
	CREX_ARG_OBJ_INFO(0, object, DateTime, 0)
	CREX_ARG_TYPE_INFO(0, hour, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, minute, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, second, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, microsecond, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_date_date_set, 0, 4, DateTime, 0)
	CREX_ARG_OBJ_INFO(0, object, DateTime, 0)
	CREX_ARG_TYPE_INFO(0, year, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, month, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, day, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_date_isodate_set, 0, 3, DateTime, 0)
	CREX_ARG_OBJ_INFO(0, object, DateTime, 0)
	CREX_ARG_TYPE_INFO(0, year, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, week, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, dayOfWeek, IS_LONG, 0, "1")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_date_timestamp_set, 0, 2, DateTime, 0)
	CREX_ARG_OBJ_INFO(0, object, DateTime, 0)
	CREX_ARG_TYPE_INFO(0, timestamp, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_date_timestamp_get arginfo_date_offset_get

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_timezone_open, 0, 1, DateTimeZone, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, timezone, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_timezone_name_get, 0, 1, IS_STRING, 0)
	CREX_ARG_OBJ_INFO(0, object, DateTimeZone, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_timezone_name_from_abbr, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, abbr, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, utcOffset, IS_LONG, 0, "-1")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, isDST, IS_LONG, 0, "-1")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_timezone_offset_get, 0, 2, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, object, DateTimeZone, 0)
	CREX_ARG_OBJ_INFO(0, datetime, DateTimeInterface, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_timezone_transitions_get, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, object, DateTimeZone, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timestampBegin, IS_LONG, 0, "CRX_INT_MIN")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timestampEnd, IS_LONG, 0, "CRX_INT_MAX")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_timezone_location_get, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, object, DateTimeZone, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_timezone_identifiers_list, 0, 0, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timezoneGroup, IS_LONG, 0, "DateTimeZone::ALL")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, countryCode, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_timezone_abbreviations_list, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_timezone_version_get, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_date_interval_create_from_date_string, 0, 1, DateInterval, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, datetime, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_date_interval_format, 0, 2, IS_STRING, 0)
	CREX_ARG_OBJ_INFO(0, object, DateInterval, 0)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_date_default_timezone_set, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, timezoneId, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_date_default_timezone_get arginfo_timezone_version_get

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_date_sunrise, 0, 1, MAY_BE_STRING|MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, timestamp, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, returnFormat, IS_LONG, 0, "SUNFUNCS_RET_STRING")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, latitude, IS_DOUBLE, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, longitude, IS_DOUBLE, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, zenith, IS_DOUBLE, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, utcOffset, IS_DOUBLE, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_date_sunset arginfo_date_sunrise

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_date_sun_info, 0, 3, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, timestamp, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, latitude, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, longitude, IS_DOUBLE, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DateTimeInterface_format, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_DateTimeInterface_getTimezone, 0, 0, DateTimeZone, MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DateTimeInterface_getOffset, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DateTimeInterface_getTimestamp arginfo_class_DateTimeInterface_getOffset

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DateTimeInterface_diff, 0, 1, DateInterval, 0)
	CREX_ARG_OBJ_INFO(0, targetObject, DateTimeInterface, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, absolute, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DateTimeInterface___wakeup, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DateTimeInterface___serialize arginfo_timezone_abbreviations_list

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_DateTimeInterface___unserialize, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DateTime___main, 0, 0, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, datetime, IS_STRING, 0, "\"now\"")
	CREX_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, timezone, DateTimeZone, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_class_DateTime___serialize arginfo_timezone_abbreviations_list

#define arginfo_class_DateTime___unserialize arginfo_class_DateTimeInterface___unserialize

#define arginfo_class_DateTime___wakeup arginfo_class_DateTimeInterface___wakeup

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DateTime___set_state, 0, 1, DateTime, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DateTime_createFromImmutable, 0, 1, IS_STATIC, 0)
	CREX_ARG_OBJ_INFO(0, object, DateTimeImmutable, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_DateTime_createFromInterface, 0, 1, DateTime, 0)
	CREX_ARG_OBJ_INFO(0, object, DateTimeInterface, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_DateTime_createFromFormat, 0, 2, DateTime, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, datetime, IS_STRING, 0)
	CREX_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, timezone, DateTimeZone, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_DateTime_getLastErrors, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
CREX_END_ARG_INFO()

#define arginfo_class_DateTime_format arginfo_class_DateTimeInterface_format

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_DateTime_modify, 0, 1, DateTime, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, modifier, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DateTime_add, 0, 1, DateTime, 0)
	CREX_ARG_OBJ_INFO(0, interval, DateInterval, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DateTime_sub arginfo_class_DateTime_add

#define arginfo_class_DateTime_getTimezone arginfo_class_DateTimeInterface_getTimezone

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DateTime_setTimezone, 0, 1, DateTime, 0)
	CREX_ARG_OBJ_INFO(0, timezone, DateTimeZone, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DateTime_getOffset arginfo_class_DateTimeInterface_getOffset

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DateTime_setTime, 0, 2, DateTime, 0)
	CREX_ARG_TYPE_INFO(0, hour, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, minute, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, second, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, microsecond, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DateTime_setDate, 0, 3, DateTime, 0)
	CREX_ARG_TYPE_INFO(0, year, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, month, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, day, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DateTime_setISODate, 0, 2, DateTime, 0)
	CREX_ARG_TYPE_INFO(0, year, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, week, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, dayOfWeek, IS_LONG, 0, "1")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DateTime_setTimestamp, 0, 1, DateTime, 0)
	CREX_ARG_TYPE_INFO(0, timestamp, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DateTime_getTimestamp arginfo_class_DateTimeInterface_getOffset

#define arginfo_class_DateTime_diff arginfo_class_DateTimeInterface_diff

#define arginfo_class_DateTimeImmutable___main arginfo_class_DateTime___main

#define arginfo_class_DateTimeImmutable___serialize arginfo_timezone_abbreviations_list

#define arginfo_class_DateTimeImmutable___unserialize arginfo_class_DateTimeInterface___unserialize

#define arginfo_class_DateTimeImmutable___wakeup arginfo_class_DateTimeInterface___wakeup

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DateTimeImmutable___set_state, 0, 1, DateTimeImmutable, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_DateTimeImmutable_createFromFormat, 0, 2, DateTimeImmutable, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, datetime, IS_STRING, 0)
	CREX_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, timezone, DateTimeZone, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_class_DateTimeImmutable_getLastErrors arginfo_class_DateTime_getLastErrors

#define arginfo_class_DateTimeImmutable_format arginfo_class_DateTimeInterface_format

#define arginfo_class_DateTimeImmutable_getTimezone arginfo_class_DateTimeInterface_getTimezone

#define arginfo_class_DateTimeImmutable_getOffset arginfo_class_DateTimeInterface_getOffset

#define arginfo_class_DateTimeImmutable_getTimestamp arginfo_class_DateTimeInterface_getOffset

#define arginfo_class_DateTimeImmutable_diff arginfo_class_DateTimeInterface_diff

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_DateTimeImmutable_modify, 0, 1, DateTimeImmutable, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, modifier, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DateTimeImmutable_add, 0, 1, DateTimeImmutable, 0)
	CREX_ARG_OBJ_INFO(0, interval, DateInterval, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DateTimeImmutable_sub arginfo_class_DateTimeImmutable_add

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DateTimeImmutable_setTimezone, 0, 1, DateTimeImmutable, 0)
	CREX_ARG_OBJ_INFO(0, timezone, DateTimeZone, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DateTimeImmutable_setTime, 0, 2, DateTimeImmutable, 0)
	CREX_ARG_TYPE_INFO(0, hour, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, minute, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, second, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, microsecond, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DateTimeImmutable_setDate, 0, 3, DateTimeImmutable, 0)
	CREX_ARG_TYPE_INFO(0, year, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, month, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, day, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DateTimeImmutable_setISODate, 0, 2, DateTimeImmutable, 0)
	CREX_ARG_TYPE_INFO(0, year, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, week, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, dayOfWeek, IS_LONG, 0, "1")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DateTimeImmutable_setTimestamp, 0, 1, DateTimeImmutable, 0)
	CREX_ARG_TYPE_INFO(0, timestamp, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DateTimeImmutable_createFromMutable, 0, 1, IS_STATIC, 0)
	CREX_ARG_OBJ_INFO(0, object, DateTime, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_DateTimeImmutable_createFromInterface, 0, 1, DateTimeImmutable, 0)
	CREX_ARG_OBJ_INFO(0, object, DateTimeInterface, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DateTimeZone___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, timezone, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DateTimeZone_getName, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DateTimeZone_getOffset, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, datetime, DateTimeInterface, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_DateTimeZone_getTransitions, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timestampBegin, IS_LONG, 0, "CRX_INT_MIN")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timestampEnd, IS_LONG, 0, "CRX_INT_MAX")
CREX_END_ARG_INFO()

#define arginfo_class_DateTimeZone_getLocation arginfo_class_DateTime_getLastErrors

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DateTimeZone_listAbbreviations, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DateTimeZone_listIdentifiers, 0, 0, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timezoneGroup, IS_LONG, 0, "DateTimeZone::ALL")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, countryCode, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_class_DateTimeZone___serialize arginfo_timezone_abbreviations_list

#define arginfo_class_DateTimeZone___unserialize arginfo_class_DateTimeInterface___unserialize

#define arginfo_class_DateTimeZone___wakeup arginfo_class_DateTimeInterface___wakeup

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DateTimeZone___set_state, 0, 1, DateTimeZone, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DateInterval___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, duration, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_DateInterval_createFromDateString, 0, 1, DateInterval, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, datetime, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DateInterval_format arginfo_class_DateTimeInterface_format

#define arginfo_class_DateInterval___serialize arginfo_timezone_abbreviations_list

#define arginfo_class_DateInterval___unserialize arginfo_class_DateTimeInterface___unserialize

#define arginfo_class_DateInterval___wakeup arginfo_class_DateTimeInterface___wakeup

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DateInterval___set_state, 0, 1, DateInterval, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_DatePeriod_createFromISO8601String, 0, 1, IS_STATIC, 0)
	CREX_ARG_TYPE_INFO(0, specification, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DatePeriod___main, 0, 0, 1)
	CREX_ARG_INFO(0, start)
	CREX_ARG_INFO(0, interval)
	CREX_ARG_INFO(0, end)
	CREX_ARG_INFO(0, options)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DatePeriod_getStartDate, 0, 0, DateTimeInterface, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DatePeriod_getEndDate, 0, 0, DateTimeInterface, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DatePeriod_getDateInterval, 0, 0, DateInterval, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DatePeriod_getRecurrences, 0, 0, IS_LONG, 1)
CREX_END_ARG_INFO()

#define arginfo_class_DatePeriod___serialize arginfo_timezone_abbreviations_list

#define arginfo_class_DatePeriod___unserialize arginfo_class_DateTimeInterface___unserialize

#define arginfo_class_DatePeriod___wakeup arginfo_class_DateTimeInterface___wakeup

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_DatePeriod___set_state, 0, 1, DatePeriod, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_DatePeriod_getIterator, 0, 0, Iterator, 0)
CREX_END_ARG_INFO()


CREX_FUNCTION(strtotime);
CREX_FUNCTION(date);
CREX_FUNCTION(idate);
CREX_FUNCTION(gmdate);
CREX_FUNCTION(mktime);
CREX_FUNCTION(gmmktime);
CREX_FUNCTION(checkdate);
CREX_FUNCTION(strftime);
CREX_FUNCTION(gmstrftime);
CREX_FUNCTION(time);
CREX_FUNCTION(localtime);
CREX_FUNCTION(getdate);
CREX_FUNCTION(date_create);
CREX_FUNCTION(date_create_immutable);
CREX_FUNCTION(date_create_from_format);
CREX_FUNCTION(date_create_immutable_from_format);
CREX_FUNCTION(date_parse);
CREX_FUNCTION(date_parse_from_format);
CREX_FUNCTION(date_get_last_errors);
CREX_FUNCTION(date_format);
CREX_FUNCTION(date_modify);
CREX_FUNCTION(date_add);
CREX_FUNCTION(date_sub);
CREX_FUNCTION(date_timezone_get);
CREX_FUNCTION(date_timezone_set);
CREX_FUNCTION(date_offset_get);
CREX_FUNCTION(date_diff);
CREX_FUNCTION(date_time_set);
CREX_FUNCTION(date_date_set);
CREX_FUNCTION(date_isodate_set);
CREX_FUNCTION(date_timestamp_set);
CREX_FUNCTION(date_timestamp_get);
CREX_FUNCTION(timezone_open);
CREX_FUNCTION(timezone_name_get);
CREX_FUNCTION(timezone_name_from_abbr);
CREX_FUNCTION(timezone_offset_get);
CREX_FUNCTION(timezone_transitions_get);
CREX_FUNCTION(timezone_location_get);
CREX_FUNCTION(timezone_identifiers_list);
CREX_FUNCTION(timezone_abbreviations_list);
CREX_FUNCTION(timezone_version_get);
CREX_FUNCTION(date_interval_create_from_date_string);
CREX_FUNCTION(date_interval_format);
CREX_FUNCTION(date_default_timezone_set);
CREX_FUNCTION(date_default_timezone_get);
CREX_FUNCTION(date_sunrise);
CREX_FUNCTION(date_sunset);
CREX_FUNCTION(date_sun_info);
CREX_METHOD(DateTime, __main);
CREX_METHOD(DateTime, __serialize);
CREX_METHOD(DateTime, __unserialize);
CREX_METHOD(DateTime, __wakeup);
CREX_METHOD(DateTime, __set_state);
CREX_METHOD(DateTime, createFromImmutable);
CREX_METHOD(DateTime, createFromInterface);
CREX_METHOD(DateTime, modify);
CREX_METHOD(DateTime, sub);
CREX_METHOD(DateTimeImmutable, __main);
CREX_METHOD(DateTimeImmutable, __serialize);
CREX_METHOD(DateTimeImmutable, __unserialize);
CREX_METHOD(DateTimeImmutable, __wakeup);
CREX_METHOD(DateTimeImmutable, __set_state);
CREX_METHOD(DateTimeImmutable, modify);
CREX_METHOD(DateTimeImmutable, add);
CREX_METHOD(DateTimeImmutable, sub);
CREX_METHOD(DateTimeImmutable, setTimezone);
CREX_METHOD(DateTimeImmutable, setTime);
CREX_METHOD(DateTimeImmutable, setDate);
CREX_METHOD(DateTimeImmutable, setISODate);
CREX_METHOD(DateTimeImmutable, setTimestamp);
CREX_METHOD(DateTimeImmutable, createFromMutable);
CREX_METHOD(DateTimeImmutable, createFromInterface);
CREX_METHOD(DateTimeZone, __main);
CREX_METHOD(DateTimeZone, __serialize);
CREX_METHOD(DateTimeZone, __unserialize);
CREX_METHOD(DateTimeZone, __wakeup);
CREX_METHOD(DateTimeZone, __set_state);
CREX_METHOD(DateInterval, __main);
CREX_METHOD(DateInterval, createFromDateString);
CREX_METHOD(DateInterval, __serialize);
CREX_METHOD(DateInterval, __unserialize);
CREX_METHOD(DateInterval, __wakeup);
CREX_METHOD(DateInterval, __set_state);
CREX_METHOD(DatePeriod, createFromISO8601String);
CREX_METHOD(DatePeriod, __main);
CREX_METHOD(DatePeriod, getStartDate);
CREX_METHOD(DatePeriod, getEndDate);
CREX_METHOD(DatePeriod, getDateInterval);
CREX_METHOD(DatePeriod, getRecurrences);
CREX_METHOD(DatePeriod, __serialize);
CREX_METHOD(DatePeriod, __unserialize);
CREX_METHOD(DatePeriod, __wakeup);
CREX_METHOD(DatePeriod, __set_state);
CREX_METHOD(DatePeriod, getIterator);


static const crex_function_entry ext_functions[] = {
	CREX_FE(strtotime, arginfo_strtotime)
	CREX_FE(date, arginfo_date)
	CREX_FE(idate, arginfo_idate)
	CREX_FE(gmdate, arginfo_gmdate)
	CREX_FE(mktime, arginfo_mktime)
	CREX_FE(gmmktime, arginfo_gmmktime)
	CREX_FE(checkdate, arginfo_checkdate)
	CREX_DEP_FE(strftime, arginfo_strftime)
	CREX_DEP_FE(gmstrftime, arginfo_gmstrftime)
	CREX_FE(time, arginfo_time)
	CREX_FE(localtime, arginfo_localtime)
	CREX_FE(getdate, arginfo_getdate)
	CREX_FE(date_create, arginfo_date_create)
	CREX_FE(date_create_immutable, arginfo_date_create_immutable)
	CREX_FE(date_create_from_format, arginfo_date_create_from_format)
	CREX_FE(date_create_immutable_from_format, arginfo_date_create_immutable_from_format)
	CREX_FE(date_parse, arginfo_date_parse)
	CREX_FE(date_parse_from_format, arginfo_date_parse_from_format)
	CREX_FE(date_get_last_errors, arginfo_date_get_last_errors)
	CREX_FE(date_format, arginfo_date_format)
	CREX_FE(date_modify, arginfo_date_modify)
	CREX_FE(date_add, arginfo_date_add)
	CREX_FE(date_sub, arginfo_date_sub)
	CREX_FE(date_timezone_get, arginfo_date_timezone_get)
	CREX_FE(date_timezone_set, arginfo_date_timezone_set)
	CREX_FE(date_offset_get, arginfo_date_offset_get)
	CREX_FE(date_diff, arginfo_date_diff)
	CREX_FE(date_time_set, arginfo_date_time_set)
	CREX_FE(date_date_set, arginfo_date_date_set)
	CREX_FE(date_isodate_set, arginfo_date_isodate_set)
	CREX_FE(date_timestamp_set, arginfo_date_timestamp_set)
	CREX_FE(date_timestamp_get, arginfo_date_timestamp_get)
	CREX_FE(timezone_open, arginfo_timezone_open)
	CREX_FE(timezone_name_get, arginfo_timezone_name_get)
	CREX_FE(timezone_name_from_abbr, arginfo_timezone_name_from_abbr)
	CREX_FE(timezone_offset_get, arginfo_timezone_offset_get)
	CREX_FE(timezone_transitions_get, arginfo_timezone_transitions_get)
	CREX_FE(timezone_location_get, arginfo_timezone_location_get)
	CREX_FE(timezone_identifiers_list, arginfo_timezone_identifiers_list)
	CREX_FE(timezone_abbreviations_list, arginfo_timezone_abbreviations_list)
	CREX_FE(timezone_version_get, arginfo_timezone_version_get)
	CREX_FE(date_interval_create_from_date_string, arginfo_date_interval_create_from_date_string)
	CREX_FE(date_interval_format, arginfo_date_interval_format)
	CREX_FE(date_default_timezone_set, arginfo_date_default_timezone_set)
	CREX_FE(date_default_timezone_get, arginfo_date_default_timezone_get)
	CREX_DEP_FE(date_sunrise, arginfo_date_sunrise)
	CREX_DEP_FE(date_sunset, arginfo_date_sunset)
	CREX_FE(date_sun_info, arginfo_date_sun_info)
	CREX_FE_END
};


static const crex_function_entry class_DateTimeInterface_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(DateTimeInterface, format, arginfo_class_DateTimeInterface_format, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(DateTimeInterface, getTimezone, arginfo_class_DateTimeInterface_getTimezone, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(DateTimeInterface, getOffset, arginfo_class_DateTimeInterface_getOffset, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(DateTimeInterface, getTimestamp, arginfo_class_DateTimeInterface_getTimestamp, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(DateTimeInterface, diff, arginfo_class_DateTimeInterface_diff, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(DateTimeInterface, __wakeup, arginfo_class_DateTimeInterface___wakeup, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(DateTimeInterface, __serialize, arginfo_class_DateTimeInterface___serialize, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(DateTimeInterface, __unserialize, arginfo_class_DateTimeInterface___unserialize, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_FE_END
};


static const crex_function_entry class_DateTime_methods[] = {
	CREX_ME(DateTime, __main, arginfo_class_DateTime___main, CREX_ACC_PUBLIC)
	CREX_ME(DateTime, __serialize, arginfo_class_DateTime___serialize, CREX_ACC_PUBLIC)
	CREX_ME(DateTime, __unserialize, arginfo_class_DateTime___unserialize, CREX_ACC_PUBLIC)
	CREX_ME(DateTime, __wakeup, arginfo_class_DateTime___wakeup, CREX_ACC_PUBLIC)
	CREX_ME(DateTime, __set_state, arginfo_class_DateTime___set_state, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(DateTime, createFromImmutable, arginfo_class_DateTime_createFromImmutable, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(DateTime, createFromInterface, arginfo_class_DateTime_createFromInterface, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(createFromFormat, date_create_from_format, arginfo_class_DateTime_createFromFormat, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getLastErrors, date_get_last_errors, arginfo_class_DateTime_getLastErrors, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(format, date_format, arginfo_class_DateTime_format, CREX_ACC_PUBLIC)
	CREX_ME(DateTime, modify, arginfo_class_DateTime_modify, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(add, date_add, arginfo_class_DateTime_add, CREX_ACC_PUBLIC)
	CREX_ME(DateTime, sub, arginfo_class_DateTime_sub, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getTimezone, date_timezone_get, arginfo_class_DateTime_getTimezone, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setTimezone, date_timezone_set, arginfo_class_DateTime_setTimezone, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getOffset, date_offset_get, arginfo_class_DateTime_getOffset, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setTime, date_time_set, arginfo_class_DateTime_setTime, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setDate, date_date_set, arginfo_class_DateTime_setDate, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setISODate, date_isodate_set, arginfo_class_DateTime_setISODate, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(setTimestamp, date_timestamp_set, arginfo_class_DateTime_setTimestamp, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getTimestamp, date_timestamp_get, arginfo_class_DateTime_getTimestamp, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(diff, date_diff, arginfo_class_DateTime_diff, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_DateTimeImmutable_methods[] = {
	CREX_ME(DateTimeImmutable, __main, arginfo_class_DateTimeImmutable___main, CREX_ACC_PUBLIC)
	CREX_ME(DateTimeImmutable, __serialize, arginfo_class_DateTimeImmutable___serialize, CREX_ACC_PUBLIC)
	CREX_ME(DateTimeImmutable, __unserialize, arginfo_class_DateTimeImmutable___unserialize, CREX_ACC_PUBLIC)
	CREX_ME(DateTimeImmutable, __wakeup, arginfo_class_DateTimeImmutable___wakeup, CREX_ACC_PUBLIC)
	CREX_ME(DateTimeImmutable, __set_state, arginfo_class_DateTimeImmutable___set_state, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(createFromFormat, date_create_immutable_from_format, arginfo_class_DateTimeImmutable_createFromFormat, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getLastErrors, date_get_last_errors, arginfo_class_DateTimeImmutable_getLastErrors, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(format, date_format, arginfo_class_DateTimeImmutable_format, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getTimezone, date_timezone_get, arginfo_class_DateTimeImmutable_getTimezone, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getOffset, date_offset_get, arginfo_class_DateTimeImmutable_getOffset, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getTimestamp, date_timestamp_get, arginfo_class_DateTimeImmutable_getTimestamp, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(diff, date_diff, arginfo_class_DateTimeImmutable_diff, CREX_ACC_PUBLIC)
	CREX_ME(DateTimeImmutable, modify, arginfo_class_DateTimeImmutable_modify, CREX_ACC_PUBLIC)
	CREX_ME(DateTimeImmutable, add, arginfo_class_DateTimeImmutable_add, CREX_ACC_PUBLIC)
	CREX_ME(DateTimeImmutable, sub, arginfo_class_DateTimeImmutable_sub, CREX_ACC_PUBLIC)
	CREX_ME(DateTimeImmutable, setTimezone, arginfo_class_DateTimeImmutable_setTimezone, CREX_ACC_PUBLIC)
	CREX_ME(DateTimeImmutable, setTime, arginfo_class_DateTimeImmutable_setTime, CREX_ACC_PUBLIC)
	CREX_ME(DateTimeImmutable, setDate, arginfo_class_DateTimeImmutable_setDate, CREX_ACC_PUBLIC)
	CREX_ME(DateTimeImmutable, setISODate, arginfo_class_DateTimeImmutable_setISODate, CREX_ACC_PUBLIC)
	CREX_ME(DateTimeImmutable, setTimestamp, arginfo_class_DateTimeImmutable_setTimestamp, CREX_ACC_PUBLIC)
	CREX_ME(DateTimeImmutable, createFromMutable, arginfo_class_DateTimeImmutable_createFromMutable, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(DateTimeImmutable, createFromInterface, arginfo_class_DateTimeImmutable_createFromInterface, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_FE_END
};


static const crex_function_entry class_DateTimeZone_methods[] = {
	CREX_ME(DateTimeZone, __main, arginfo_class_DateTimeZone___main, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getName, timezone_name_get, arginfo_class_DateTimeZone_getName, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getOffset, timezone_offset_get, arginfo_class_DateTimeZone_getOffset, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getTransitions, timezone_transitions_get, arginfo_class_DateTimeZone_getTransitions, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getLocation, timezone_location_get, arginfo_class_DateTimeZone_getLocation, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(listAbbreviations, timezone_abbreviations_list, arginfo_class_DateTimeZone_listAbbreviations, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(listIdentifiers, timezone_identifiers_list, arginfo_class_DateTimeZone_listIdentifiers, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(DateTimeZone, __serialize, arginfo_class_DateTimeZone___serialize, CREX_ACC_PUBLIC)
	CREX_ME(DateTimeZone, __unserialize, arginfo_class_DateTimeZone___unserialize, CREX_ACC_PUBLIC)
	CREX_ME(DateTimeZone, __wakeup, arginfo_class_DateTimeZone___wakeup, CREX_ACC_PUBLIC)
	CREX_ME(DateTimeZone, __set_state, arginfo_class_DateTimeZone___set_state, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_FE_END
};


static const crex_function_entry class_DateInterval_methods[] = {
	CREX_ME(DateInterval, __main, arginfo_class_DateInterval___main, CREX_ACC_PUBLIC)
	CREX_ME(DateInterval, createFromDateString, arginfo_class_DateInterval_createFromDateString, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(format, date_interval_format, arginfo_class_DateInterval_format, CREX_ACC_PUBLIC)
	CREX_ME(DateInterval, __serialize, arginfo_class_DateInterval___serialize, CREX_ACC_PUBLIC)
	CREX_ME(DateInterval, __unserialize, arginfo_class_DateInterval___unserialize, CREX_ACC_PUBLIC)
	CREX_ME(DateInterval, __wakeup, arginfo_class_DateInterval___wakeup, CREX_ACC_PUBLIC)
	CREX_ME(DateInterval, __set_state, arginfo_class_DateInterval___set_state, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_FE_END
};


static const crex_function_entry class_DatePeriod_methods[] = {
	CREX_ME(DatePeriod, createFromISO8601String, arginfo_class_DatePeriod_createFromISO8601String, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(DatePeriod, __main, arginfo_class_DatePeriod___main, CREX_ACC_PUBLIC)
	CREX_ME(DatePeriod, getStartDate, arginfo_class_DatePeriod_getStartDate, CREX_ACC_PUBLIC)
	CREX_ME(DatePeriod, getEndDate, arginfo_class_DatePeriod_getEndDate, CREX_ACC_PUBLIC)
	CREX_ME(DatePeriod, getDateInterval, arginfo_class_DatePeriod_getDateInterval, CREX_ACC_PUBLIC)
	CREX_ME(DatePeriod, getRecurrences, arginfo_class_DatePeriod_getRecurrences, CREX_ACC_PUBLIC)
	CREX_ME(DatePeriod, __serialize, arginfo_class_DatePeriod___serialize, CREX_ACC_PUBLIC)
	CREX_ME(DatePeriod, __unserialize, arginfo_class_DatePeriod___unserialize, CREX_ACC_PUBLIC)
	CREX_ME(DatePeriod, __wakeup, arginfo_class_DatePeriod___wakeup, CREX_ACC_PUBLIC)
	CREX_ME(DatePeriod, __set_state, arginfo_class_DatePeriod___set_state, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(DatePeriod, getIterator, arginfo_class_DatePeriod_getIterator, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_DateError_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_DateObjectError_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_DateRangeError_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_DateException_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_DateInvalidTimeZoneException_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_DateInvalidOperationException_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_DateMalformedStringException_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_DateMalformedIntervalStringException_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_DateMalformedPeriodStringException_methods[] = {
	CREX_FE_END
};

static void register_crx_date_symbols(int module_number)
{
	REGISTER_STRING_CONSTANT("DATE_ATOM", DATE_FORMAT_RFC3339, CONST_PERSISTENT);
	CREX_ASSERT(strcmp(DATE_FORMAT_RFC3339, "Y-m-d\\TH:i:sP") == 0);
	REGISTER_STRING_CONSTANT("DATE_COOKIE", DATE_FORMAT_COOKIE, CONST_PERSISTENT);
	CREX_ASSERT(strcmp(DATE_FORMAT_COOKIE, "l, d-M-Y H:i:s T") == 0);
	REGISTER_STRING_CONSTANT("DATE_ISO8601", DATE_FORMAT_ISO8601, CONST_PERSISTENT);
	CREX_ASSERT(strcmp(DATE_FORMAT_ISO8601, "Y-m-d\\TH:i:sO") == 0);
	REGISTER_STRING_CONSTANT("DATE_ISO8601_EXPANDED", DATE_FORMAT_ISO8601_EXPANDED, CONST_PERSISTENT);
	CREX_ASSERT(strcmp(DATE_FORMAT_ISO8601_EXPANDED, "X-m-d\\TH:i:sP") == 0);
	REGISTER_STRING_CONSTANT("DATE_RFC822", DATE_FORMAT_RFC822, CONST_PERSISTENT);
	CREX_ASSERT(strcmp(DATE_FORMAT_RFC822, "D, d M y H:i:s O") == 0);
	REGISTER_STRING_CONSTANT("DATE_RFC850", DATE_FORMAT_RFC850, CONST_PERSISTENT);
	CREX_ASSERT(strcmp(DATE_FORMAT_RFC850, "l, d-M-y H:i:s T") == 0);
	REGISTER_STRING_CONSTANT("DATE_RFC1036", DATE_FORMAT_RFC1036, CONST_PERSISTENT);
	CREX_ASSERT(strcmp(DATE_FORMAT_RFC1036, "D, d M y H:i:s O") == 0);
	REGISTER_STRING_CONSTANT("DATE_RFC1123", DATE_FORMAT_RFC1123, CONST_PERSISTENT);
	CREX_ASSERT(strcmp(DATE_FORMAT_RFC1123, "D, d M Y H:i:s O") == 0);
	REGISTER_STRING_CONSTANT("DATE_RFC7231", DATE_FORMAT_RFC7231, CONST_PERSISTENT);
	CREX_ASSERT(strcmp(DATE_FORMAT_RFC7231, "D, d M Y H:i:s \\G\\M\\T") == 0);
	REGISTER_STRING_CONSTANT("DATE_RFC2822", DATE_FORMAT_RFC2822, CONST_PERSISTENT);
	CREX_ASSERT(strcmp(DATE_FORMAT_RFC2822, "D, d M Y H:i:s O") == 0);
	REGISTER_STRING_CONSTANT("DATE_RFC3339", DATE_FORMAT_RFC3339, CONST_PERSISTENT);
	CREX_ASSERT(strcmp(DATE_FORMAT_RFC3339, "Y-m-d\\TH:i:sP") == 0);
	REGISTER_STRING_CONSTANT("DATE_RFC3339_EXTENDED", DATE_FORMAT_RFC3339_EXTENDED, CONST_PERSISTENT);
	CREX_ASSERT(strcmp(DATE_FORMAT_RFC3339_EXTENDED, "Y-m-d\\TH:i:s.vP") == 0);
	REGISTER_STRING_CONSTANT("DATE_RSS", DATE_FORMAT_RFC1123, CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("DATE_W3C", DATE_FORMAT_RFC3339, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SUNFUNCS_RET_TIMESTAMP", SUNFUNCS_RET_TIMESTAMP, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SUNFUNCS_RET_STRING", SUNFUNCS_RET_STRING, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SUNFUNCS_RET_DOUBLE", SUNFUNCS_RET_DOUBLE, CONST_PERSISTENT);
}

static crex_class_entry *register_class_DateTimeInterface(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DateTimeInterface", class_DateTimeInterface_methods);
	class_entry = crex_register_internal_interface(&ce);

	zval const_ATOM_value;
	crex_string *const_ATOM_value_str = crex_string_init(DATE_FORMAT_RFC3339, strlen(DATE_FORMAT_RFC3339), 1);
	ZVAL_STR(&const_ATOM_value, const_ATOM_value_str);
	crex_string *const_ATOM_name = crex_string_init_interned("ATOM", sizeof("ATOM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATOM_name, &const_ATOM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATOM_name);

	zval const_COOKIE_value;
	crex_string *const_COOKIE_value_str = crex_string_init(DATE_FORMAT_COOKIE, strlen(DATE_FORMAT_COOKIE), 1);
	ZVAL_STR(&const_COOKIE_value, const_COOKIE_value_str);
	crex_string *const_COOKIE_name = crex_string_init_interned("COOKIE", sizeof("COOKIE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_COOKIE_name, &const_COOKIE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_COOKIE_name);

	zval const_ISO8601_value;
	crex_string *const_ISO8601_value_str = crex_string_init(DATE_FORMAT_ISO8601, strlen(DATE_FORMAT_ISO8601), 1);
	ZVAL_STR(&const_ISO8601_value, const_ISO8601_value_str);
	crex_string *const_ISO8601_name = crex_string_init_interned("ISO8601", sizeof("ISO8601") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ISO8601_name, &const_ISO8601_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ISO8601_name);

	zval const_ISO8601_EXPANDED_value;
	crex_string *const_ISO8601_EXPANDED_value_str = crex_string_init(DATE_FORMAT_ISO8601_EXPANDED, strlen(DATE_FORMAT_ISO8601_EXPANDED), 1);
	ZVAL_STR(&const_ISO8601_EXPANDED_value, const_ISO8601_EXPANDED_value_str);
	crex_string *const_ISO8601_EXPANDED_name = crex_string_init_interned("ISO8601_EXPANDED", sizeof("ISO8601_EXPANDED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ISO8601_EXPANDED_name, &const_ISO8601_EXPANDED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ISO8601_EXPANDED_name);

	zval const_RFC822_value;
	crex_string *const_RFC822_value_str = crex_string_init(DATE_FORMAT_RFC822, strlen(DATE_FORMAT_RFC822), 1);
	ZVAL_STR(&const_RFC822_value, const_RFC822_value_str);
	crex_string *const_RFC822_name = crex_string_init_interned("RFC822", sizeof("RFC822") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_RFC822_name, &const_RFC822_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_RFC822_name);

	zval const_RFC850_value;
	crex_string *const_RFC850_value_str = crex_string_init(DATE_FORMAT_RFC850, strlen(DATE_FORMAT_RFC850), 1);
	ZVAL_STR(&const_RFC850_value, const_RFC850_value_str);
	crex_string *const_RFC850_name = crex_string_init_interned("RFC850", sizeof("RFC850") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_RFC850_name, &const_RFC850_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_RFC850_name);

	zval const_RFC1036_value;
	crex_string *const_RFC1036_value_str = crex_string_init(DATE_FORMAT_RFC1036, strlen(DATE_FORMAT_RFC1036), 1);
	ZVAL_STR(&const_RFC1036_value, const_RFC1036_value_str);
	crex_string *const_RFC1036_name = crex_string_init_interned("RFC1036", sizeof("RFC1036") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_RFC1036_name, &const_RFC1036_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_RFC1036_name);

	zval const_RFC1123_value;
	crex_string *const_RFC1123_value_str = crex_string_init(DATE_FORMAT_RFC1123, strlen(DATE_FORMAT_RFC1123), 1);
	ZVAL_STR(&const_RFC1123_value, const_RFC1123_value_str);
	crex_string *const_RFC1123_name = crex_string_init_interned("RFC1123", sizeof("RFC1123") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_RFC1123_name, &const_RFC1123_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_RFC1123_name);

	zval const_RFC7231_value;
	crex_string *const_RFC7231_value_str = crex_string_init(DATE_FORMAT_RFC7231, strlen(DATE_FORMAT_RFC7231), 1);
	ZVAL_STR(&const_RFC7231_value, const_RFC7231_value_str);
	crex_string *const_RFC7231_name = crex_string_init_interned("RFC7231", sizeof("RFC7231") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_RFC7231_name, &const_RFC7231_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_RFC7231_name);

	zval const_RFC2822_value;
	crex_string *const_RFC2822_value_str = crex_string_init(DATE_FORMAT_RFC2822, strlen(DATE_FORMAT_RFC2822), 1);
	ZVAL_STR(&const_RFC2822_value, const_RFC2822_value_str);
	crex_string *const_RFC2822_name = crex_string_init_interned("RFC2822", sizeof("RFC2822") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_RFC2822_name, &const_RFC2822_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_RFC2822_name);

	zval const_RFC3339_value;
	crex_string *const_RFC3339_value_str = crex_string_init(DATE_FORMAT_RFC3339, strlen(DATE_FORMAT_RFC3339), 1);
	ZVAL_STR(&const_RFC3339_value, const_RFC3339_value_str);
	crex_string *const_RFC3339_name = crex_string_init_interned("RFC3339", sizeof("RFC3339") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_RFC3339_name, &const_RFC3339_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_RFC3339_name);

	zval const_RFC3339_EXTENDED_value;
	crex_string *const_RFC3339_EXTENDED_value_str = crex_string_init(DATE_FORMAT_RFC3339_EXTENDED, strlen(DATE_FORMAT_RFC3339_EXTENDED), 1);
	ZVAL_STR(&const_RFC3339_EXTENDED_value, const_RFC3339_EXTENDED_value_str);
	crex_string *const_RFC3339_EXTENDED_name = crex_string_init_interned("RFC3339_EXTENDED", sizeof("RFC3339_EXTENDED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_RFC3339_EXTENDED_name, &const_RFC3339_EXTENDED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_RFC3339_EXTENDED_name);

	zval const_RSS_value;
	crex_string *const_RSS_value_str = crex_string_init(DATE_FORMAT_RFC1123, strlen(DATE_FORMAT_RFC1123), 1);
	ZVAL_STR(&const_RSS_value, const_RSS_value_str);
	crex_string *const_RSS_name = crex_string_init_interned("RSS", sizeof("RSS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_RSS_name, &const_RSS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_RSS_name);

	zval const_W3C_value;
	crex_string *const_W3C_value_str = crex_string_init(DATE_FORMAT_RFC3339, strlen(DATE_FORMAT_RFC3339), 1);
	ZVAL_STR(&const_W3C_value, const_W3C_value_str);
	crex_string *const_W3C_name = crex_string_init_interned("W3C", sizeof("W3C") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_W3C_name, &const_W3C_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_W3C_name);

	return class_entry;
}

static crex_class_entry *register_class_DateTime(crex_class_entry *class_entry_DateTimeInterface)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DateTime", class_DateTime_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	crex_class_implements(class_entry, 1, class_entry_DateTimeInterface);

	return class_entry;
}

static crex_class_entry *register_class_DateTimeImmutable(crex_class_entry *class_entry_DateTimeInterface)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DateTimeImmutable", class_DateTimeImmutable_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	crex_class_implements(class_entry, 1, class_entry_DateTimeInterface);

	return class_entry;
}

static crex_class_entry *register_class_DateTimeZone(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DateTimeZone", class_DateTimeZone_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	zval const_AFRICA_value;
	ZVAL_LONG(&const_AFRICA_value, CRX_DATE_TIMEZONE_GROUP_AFRICA);
	crex_string *const_AFRICA_name = crex_string_init_interned("AFRICA", sizeof("AFRICA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_AFRICA_name, &const_AFRICA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_AFRICA_name);

	zval const_AMERICA_value;
	ZVAL_LONG(&const_AMERICA_value, CRX_DATE_TIMEZONE_GROUP_AMERICA);
	crex_string *const_AMERICA_name = crex_string_init_interned("AMERICA", sizeof("AMERICA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_AMERICA_name, &const_AMERICA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_AMERICA_name);

	zval const_ANTARCTICA_value;
	ZVAL_LONG(&const_ANTARCTICA_value, CRX_DATE_TIMEZONE_GROUP_ANTARCTICA);
	crex_string *const_ANTARCTICA_name = crex_string_init_interned("ANTARCTICA", sizeof("ANTARCTICA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ANTARCTICA_name, &const_ANTARCTICA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ANTARCTICA_name);

	zval const_ARCTIC_value;
	ZVAL_LONG(&const_ARCTIC_value, CRX_DATE_TIMEZONE_GROUP_ARCTIC);
	crex_string *const_ARCTIC_name = crex_string_init_interned("ARCTIC", sizeof("ARCTIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ARCTIC_name, &const_ARCTIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ARCTIC_name);

	zval const_ASIA_value;
	ZVAL_LONG(&const_ASIA_value, CRX_DATE_TIMEZONE_GROUP_ASIA);
	crex_string *const_ASIA_name = crex_string_init_interned("ASIA", sizeof("ASIA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ASIA_name, &const_ASIA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ASIA_name);

	zval const_ATLANTIC_value;
	ZVAL_LONG(&const_ATLANTIC_value, CRX_DATE_TIMEZONE_GROUP_ATLANTIC);
	crex_string *const_ATLANTIC_name = crex_string_init_interned("ATLANTIC", sizeof("ATLANTIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATLANTIC_name, &const_ATLANTIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATLANTIC_name);

	zval const_AUSTRALIA_value;
	ZVAL_LONG(&const_AUSTRALIA_value, CRX_DATE_TIMEZONE_GROUP_AUSTRALIA);
	crex_string *const_AUSTRALIA_name = crex_string_init_interned("AUSTRALIA", sizeof("AUSTRALIA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_AUSTRALIA_name, &const_AUSTRALIA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_AUSTRALIA_name);

	zval const_EUROPE_value;
	ZVAL_LONG(&const_EUROPE_value, CRX_DATE_TIMEZONE_GROUP_EUROPE);
	crex_string *const_EUROPE_name = crex_string_init_interned("EUROPE", sizeof("EUROPE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_EUROPE_name, &const_EUROPE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_EUROPE_name);

	zval const_INDIAN_value;
	ZVAL_LONG(&const_INDIAN_value, CRX_DATE_TIMEZONE_GROUP_INDIAN);
	crex_string *const_INDIAN_name = crex_string_init_interned("INDIAN", sizeof("INDIAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_INDIAN_name, &const_INDIAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_INDIAN_name);

	zval const_PACIFIC_value;
	ZVAL_LONG(&const_PACIFIC_value, CRX_DATE_TIMEZONE_GROUP_PACIFIC);
	crex_string *const_PACIFIC_name = crex_string_init_interned("PACIFIC", sizeof("PACIFIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PACIFIC_name, &const_PACIFIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PACIFIC_name);

	zval const_UTC_value;
	ZVAL_LONG(&const_UTC_value, CRX_DATE_TIMEZONE_GROUP_UTC);
	crex_string *const_UTC_name = crex_string_init_interned("UTC", sizeof("UTC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_UTC_name, &const_UTC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_UTC_name);

	zval const_ALL_value;
	ZVAL_LONG(&const_ALL_value, CRX_DATE_TIMEZONE_GROUP_ALL);
	crex_string *const_ALL_name = crex_string_init_interned("ALL", sizeof("ALL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ALL_name, &const_ALL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ALL_name);

	zval const_ALL_WITH_BC_value;
	ZVAL_LONG(&const_ALL_WITH_BC_value, CRX_DATE_TIMEZONE_GROUP_ALL_W_BC);
	crex_string *const_ALL_WITH_BC_name = crex_string_init_interned("ALL_WITH_BC", sizeof("ALL_WITH_BC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ALL_WITH_BC_name, &const_ALL_WITH_BC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ALL_WITH_BC_name);

	zval const_PER_COUNTRY_value;
	ZVAL_LONG(&const_PER_COUNTRY_value, CRX_DATE_TIMEZONE_PER_COUNTRY);
	crex_string *const_PER_COUNTRY_name = crex_string_init_interned("PER_COUNTRY", sizeof("PER_COUNTRY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PER_COUNTRY_name, &const_PER_COUNTRY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PER_COUNTRY_name);

	return class_entry;
}

static crex_class_entry *register_class_DateInterval(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DateInterval", class_DateInterval_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	return class_entry;
}

static crex_class_entry *register_class_DatePeriod(crex_class_entry *class_entry_IteratorAggregate)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DatePeriod", class_DatePeriod_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	crex_class_implements(class_entry, 1, class_entry_IteratorAggregate);

	zval const_EXCLUDE_START_DATE_value;
	ZVAL_LONG(&const_EXCLUDE_START_DATE_value, CRX_DATE_PERIOD_EXCLUDE_START_DATE);
	crex_string *const_EXCLUDE_START_DATE_name = crex_string_init_interned("EXCLUDE_START_DATE", sizeof("EXCLUDE_START_DATE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_EXCLUDE_START_DATE_name, &const_EXCLUDE_START_DATE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_EXCLUDE_START_DATE_name);

	zval const_INCLUDE_END_DATE_value;
	ZVAL_LONG(&const_INCLUDE_END_DATE_value, CRX_DATE_PERIOD_INCLUDE_END_DATE);
	crex_string *const_INCLUDE_END_DATE_name = crex_string_init_interned("INCLUDE_END_DATE", sizeof("INCLUDE_END_DATE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_INCLUDE_END_DATE_name, &const_INCLUDE_END_DATE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_INCLUDE_END_DATE_name);

	zval property_start_default_value;
	ZVAL_UNDEF(&property_start_default_value);
	crex_string *property_start_name = crex_string_init("start", sizeof("start") - 1, 1);
	crex_string *property_start_class_DateTimeInterface = crex_string_init("DateTimeInterface", sizeof("DateTimeInterface")-1, 1);
	crex_declare_typed_property(class_entry, property_start_name, &property_start_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_start_class_DateTimeInterface, 0, MAY_BE_NULL));
	crex_string_release(property_start_name);

	zval property_current_default_value;
	ZVAL_UNDEF(&property_current_default_value);
	crex_string *property_current_name = crex_string_init("current", sizeof("current") - 1, 1);
	crex_string *property_current_class_DateTimeInterface = crex_string_init("DateTimeInterface", sizeof("DateTimeInterface")-1, 1);
	crex_declare_typed_property(class_entry, property_current_name, &property_current_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_current_class_DateTimeInterface, 0, MAY_BE_NULL));
	crex_string_release(property_current_name);

	zval property_end_default_value;
	ZVAL_UNDEF(&property_end_default_value);
	crex_string *property_end_name = crex_string_init("end", sizeof("end") - 1, 1);
	crex_string *property_end_class_DateTimeInterface = crex_string_init("DateTimeInterface", sizeof("DateTimeInterface")-1, 1);
	crex_declare_typed_property(class_entry, property_end_name, &property_end_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_end_class_DateTimeInterface, 0, MAY_BE_NULL));
	crex_string_release(property_end_name);

	zval property_interval_default_value;
	ZVAL_UNDEF(&property_interval_default_value);
	crex_string *property_interval_name = crex_string_init("interval", sizeof("interval") - 1, 1);
	crex_string *property_interval_class_DateInterval = crex_string_init("DateInterval", sizeof("DateInterval")-1, 1);
	crex_declare_typed_property(class_entry, property_interval_name, &property_interval_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_interval_class_DateInterval, 0, MAY_BE_NULL));
	crex_string_release(property_interval_name);

	zval property_recurrences_default_value;
	ZVAL_UNDEF(&property_recurrences_default_value);
	crex_string *property_recurrences_name = crex_string_init("recurrences", sizeof("recurrences") - 1, 1);
	crex_declare_typed_property(class_entry, property_recurrences_name, &property_recurrences_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_recurrences_name);

	zval property_include_start_date_default_value;
	ZVAL_UNDEF(&property_include_start_date_default_value);
	crex_string *property_include_start_date_name = crex_string_init("include_start_date", sizeof("include_start_date") - 1, 1);
	crex_declare_typed_property(class_entry, property_include_start_date_name, &property_include_start_date_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property_include_start_date_name);

	zval property_include_end_date_default_value;
	ZVAL_UNDEF(&property_include_end_date_default_value);
	crex_string *property_include_end_date_name = crex_string_init("include_end_date", sizeof("include_end_date") - 1, 1);
	crex_declare_typed_property(class_entry, property_include_end_date_name, &property_include_end_date_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_BOOL));
	crex_string_release(property_include_end_date_name);

	return class_entry;
}

static crex_class_entry *register_class_DateError(crex_class_entry *class_entry_Error)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DateError", class_DateError_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Error);
	class_entry->ce_flags |= CREX_ACC_NO_DYNAMIC_PROPERTIES;

	return class_entry;
}

static crex_class_entry *register_class_DateObjectError(crex_class_entry *class_entry_DateError)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DateObjectError", class_DateObjectError_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_DateError);
	class_entry->ce_flags |= CREX_ACC_NO_DYNAMIC_PROPERTIES;

	return class_entry;
}

static crex_class_entry *register_class_DateRangeError(crex_class_entry *class_entry_DateError)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DateRangeError", class_DateRangeError_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_DateError);
	class_entry->ce_flags |= CREX_ACC_NO_DYNAMIC_PROPERTIES;

	return class_entry;
}

static crex_class_entry *register_class_DateException(crex_class_entry *class_entry_Exception)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DateException", class_DateException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Exception);
	class_entry->ce_flags |= CREX_ACC_NO_DYNAMIC_PROPERTIES;

	return class_entry;
}

static crex_class_entry *register_class_DateInvalidTimeZoneException(crex_class_entry *class_entry_DateException)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DateInvalidTimeZoneException", class_DateInvalidTimeZoneException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_DateException);
	class_entry->ce_flags |= CREX_ACC_NO_DYNAMIC_PROPERTIES;

	return class_entry;
}

static crex_class_entry *register_class_DateInvalidOperationException(crex_class_entry *class_entry_DateException)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DateInvalidOperationException", class_DateInvalidOperationException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_DateException);
	class_entry->ce_flags |= CREX_ACC_NO_DYNAMIC_PROPERTIES;

	return class_entry;
}

static crex_class_entry *register_class_DateMalformedStringException(crex_class_entry *class_entry_DateException)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DateMalformedStringException", class_DateMalformedStringException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_DateException);
	class_entry->ce_flags |= CREX_ACC_NO_DYNAMIC_PROPERTIES;

	return class_entry;
}

static crex_class_entry *register_class_DateMalformedIntervalStringException(crex_class_entry *class_entry_DateException)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DateMalformedIntervalStringException", class_DateMalformedIntervalStringException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_DateException);
	class_entry->ce_flags |= CREX_ACC_NO_DYNAMIC_PROPERTIES;

	return class_entry;
}

static crex_class_entry *register_class_DateMalformedPeriodStringException(crex_class_entry *class_entry_DateException)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DateMalformedPeriodStringException", class_DateMalformedPeriodStringException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_DateException);
	class_entry->ce_flags |= CREX_ACC_NO_DYNAMIC_PROPERTIES;

	return class_entry;
}
