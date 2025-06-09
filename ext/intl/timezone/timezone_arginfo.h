/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 0617d5cab74655058d97581f60f3a486e2875beb */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_IntlTimeZone___main, 0, 0, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlTimeZone_countEquivalentIDs, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, timezoneId, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_IntlTimeZone_createDefault, 0, 0, IntlTimeZone, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_IntlTimeZone_createEnumeration, 0, 0, IntlIterator, MAY_BE_FALSE)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, countryOrRawOffset, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_IntlTimeZone_createTimeZone, 0, 1, IntlTimeZone, 1)
	CREX_ARG_TYPE_INFO(0, timezoneId, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_IntlTimeZone_createTimeZoneIDEnumeration, 0, 1, IntlIterator, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, region, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, rawOffset, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_IntlTimeZone_fromDateTimeZone, 0, 1, IntlTimeZone, 1)
	CREX_ARG_OBJ_INFO(0, timezone, DateTimeZone, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlTimeZone_getCanonicalID, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, timezoneId, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, isSystemId, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlTimeZone_getDisplayName, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, dst, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_LONG, 0, "IntlTimeZone::DISPLAY_LONG")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, locale, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlTimeZone_getDSTSavings, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlTimeZone_getEquivalentID, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, timezoneId, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlTimeZone_getErrorCode, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlTimeZone_getErrorMessage, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
CREX_END_ARG_INFO()

#define arginfo_class_IntlTimeZone_getGMT arginfo_class_IntlTimeZone_createDefault

#define arginfo_class_IntlTimeZone_getID arginfo_class_IntlTimeZone_getErrorMessage

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlTimeZone_getOffset, 0, 4, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, timestamp, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, local, _IS_BOOL, 0)
	CREX_ARG_INFO(1, rawOffset)
	CREX_ARG_INFO(1, dstOffset)
CREX_END_ARG_INFO()

#define arginfo_class_IntlTimeZone_getRawOffset arginfo_class_IntlTimeZone_getDSTSavings

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlTimeZone_getRegion, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, timezoneId, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_IntlTimeZone_getTZDataVersion arginfo_class_IntlTimeZone_getErrorMessage

#define arginfo_class_IntlTimeZone_getUnknown arginfo_class_IntlTimeZone_createDefault

#if U_ICU_VERSION_MAJOR_NUM >= 52
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlTimeZone_getWindowsID, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, timezoneId, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#if U_ICU_VERSION_MAJOR_NUM >= 52
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlTimeZone_getIDForWindowsID, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, timezoneId, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, region, IS_STRING, 1, "null")
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlTimeZone_hasSameRules, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, other, IntlTimeZone, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_IntlTimeZone_toDateTimeZone, 0, 0, DateTimeZone, MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlTimeZone_useDaylightTime, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()


CREX_METHOD(IntlTimeZone, __main);
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


static const crex_function_entry class_IntlTimeZone_methods[] = {
	CREX_ME(IntlTimeZone, __main, arginfo_class_IntlTimeZone___main, CREX_ACC_PRIVATE)
	CREX_ME_MAPPING(countEquivalentIDs, intltz_count_equivalent_ids, arginfo_class_IntlTimeZone_countEquivalentIDs, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(createDefault, intltz_create_default, arginfo_class_IntlTimeZone_createDefault, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(createEnumeration, intltz_create_enumeration, arginfo_class_IntlTimeZone_createEnumeration, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(createTimeZone, intltz_create_time_zone, arginfo_class_IntlTimeZone_createTimeZone, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(createTimeZoneIDEnumeration, intltz_create_time_zone_id_enumeration, arginfo_class_IntlTimeZone_createTimeZoneIDEnumeration, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(fromDateTimeZone, intltz_from_date_time_zone, arginfo_class_IntlTimeZone_fromDateTimeZone, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getCanonicalID, intltz_get_canonical_id, arginfo_class_IntlTimeZone_getCanonicalID, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getDisplayName, intltz_get_display_name, arginfo_class_IntlTimeZone_getDisplayName, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getDSTSavings, intltz_get_dst_savings, arginfo_class_IntlTimeZone_getDSTSavings, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getEquivalentID, intltz_get_equivalent_id, arginfo_class_IntlTimeZone_getEquivalentID, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getErrorCode, intltz_get_error_code, arginfo_class_IntlTimeZone_getErrorCode, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getErrorMessage, intltz_get_error_message, arginfo_class_IntlTimeZone_getErrorMessage, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getGMT, intltz_get_gmt, arginfo_class_IntlTimeZone_getGMT, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getID, intltz_get_id, arginfo_class_IntlTimeZone_getID, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getOffset, intltz_get_offset, arginfo_class_IntlTimeZone_getOffset, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getRawOffset, intltz_get_raw_offset, arginfo_class_IntlTimeZone_getRawOffset, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getRegion, intltz_get_region, arginfo_class_IntlTimeZone_getRegion, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getTZDataVersion, intltz_get_tz_data_version, arginfo_class_IntlTimeZone_getTZDataVersion, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getUnknown, intltz_get_unknown, arginfo_class_IntlTimeZone_getUnknown, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
#if U_ICU_VERSION_MAJOR_NUM >= 52
	CREX_ME_MAPPING(getWindowsID, intltz_get_windows_id, arginfo_class_IntlTimeZone_getWindowsID, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 52
	CREX_ME_MAPPING(getIDForWindowsID, intltz_get_id_for_windows_id, arginfo_class_IntlTimeZone_getIDForWindowsID, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
#endif
	CREX_ME_MAPPING(hasSameRules, intltz_has_same_rules, arginfo_class_IntlTimeZone_hasSameRules, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(toDateTimeZone, intltz_to_date_time_zone, arginfo_class_IntlTimeZone_toDateTimeZone, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(useDaylightTime, intltz_use_daylight_time, arginfo_class_IntlTimeZone_useDaylightTime, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_IntlTimeZone(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "IntlTimeZone", class_IntlTimeZone_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;

	zval const_DISPLAY_SHORT_value;
	ZVAL_LONG(&const_DISPLAY_SHORT_value, TimeZone::SHORT);
	crex_string *const_DISPLAY_SHORT_name = crex_string_init_interned("DISPLAY_SHORT", sizeof("DISPLAY_SHORT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DISPLAY_SHORT_name, &const_DISPLAY_SHORT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DISPLAY_SHORT_name);

	zval const_DISPLAY_LONG_value;
	ZVAL_LONG(&const_DISPLAY_LONG_value, TimeZone::LONG);
	crex_string *const_DISPLAY_LONG_name = crex_string_init_interned("DISPLAY_LONG", sizeof("DISPLAY_LONG") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DISPLAY_LONG_name, &const_DISPLAY_LONG_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DISPLAY_LONG_name);

	zval const_DISPLAY_SHORT_GENERIC_value;
	ZVAL_LONG(&const_DISPLAY_SHORT_GENERIC_value, TimeZone::SHORT_GENERIC);
	crex_string *const_DISPLAY_SHORT_GENERIC_name = crex_string_init_interned("DISPLAY_SHORT_GENERIC", sizeof("DISPLAY_SHORT_GENERIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DISPLAY_SHORT_GENERIC_name, &const_DISPLAY_SHORT_GENERIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DISPLAY_SHORT_GENERIC_name);

	zval const_DISPLAY_LONG_GENERIC_value;
	ZVAL_LONG(&const_DISPLAY_LONG_GENERIC_value, TimeZone::LONG_GENERIC);
	crex_string *const_DISPLAY_LONG_GENERIC_name = crex_string_init_interned("DISPLAY_LONG_GENERIC", sizeof("DISPLAY_LONG_GENERIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DISPLAY_LONG_GENERIC_name, &const_DISPLAY_LONG_GENERIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DISPLAY_LONG_GENERIC_name);

	zval const_DISPLAY_SHORT_GMT_value;
	ZVAL_LONG(&const_DISPLAY_SHORT_GMT_value, TimeZone::SHORT_GMT);
	crex_string *const_DISPLAY_SHORT_GMT_name = crex_string_init_interned("DISPLAY_SHORT_GMT", sizeof("DISPLAY_SHORT_GMT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DISPLAY_SHORT_GMT_name, &const_DISPLAY_SHORT_GMT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DISPLAY_SHORT_GMT_name);

	zval const_DISPLAY_LONG_GMT_value;
	ZVAL_LONG(&const_DISPLAY_LONG_GMT_value, TimeZone::LONG_GMT);
	crex_string *const_DISPLAY_LONG_GMT_name = crex_string_init_interned("DISPLAY_LONG_GMT", sizeof("DISPLAY_LONG_GMT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DISPLAY_LONG_GMT_name, &const_DISPLAY_LONG_GMT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DISPLAY_LONG_GMT_name);

	zval const_DISPLAY_SHORT_COMMONLY_USED_value;
	ZVAL_LONG(&const_DISPLAY_SHORT_COMMONLY_USED_value, TimeZone::SHORT_COMMONLY_USED);
	crex_string *const_DISPLAY_SHORT_COMMONLY_USED_name = crex_string_init_interned("DISPLAY_SHORT_COMMONLY_USED", sizeof("DISPLAY_SHORT_COMMONLY_USED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DISPLAY_SHORT_COMMONLY_USED_name, &const_DISPLAY_SHORT_COMMONLY_USED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DISPLAY_SHORT_COMMONLY_USED_name);

	zval const_DISPLAY_GENERIC_LOCATION_value;
	ZVAL_LONG(&const_DISPLAY_GENERIC_LOCATION_value, TimeZone::GENERIC_LOCATION);
	crex_string *const_DISPLAY_GENERIC_LOCATION_name = crex_string_init_interned("DISPLAY_GENERIC_LOCATION", sizeof("DISPLAY_GENERIC_LOCATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DISPLAY_GENERIC_LOCATION_name, &const_DISPLAY_GENERIC_LOCATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DISPLAY_GENERIC_LOCATION_name);

	zval const_TYPE_ANY_value;
	ZVAL_LONG(&const_TYPE_ANY_value, UCAL_ZONE_TYPE_ANY);
	crex_string *const_TYPE_ANY_name = crex_string_init_interned("TYPE_ANY", sizeof("TYPE_ANY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_TYPE_ANY_name, &const_TYPE_ANY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_TYPE_ANY_name);

	zval const_TYPE_CANONICAL_value;
	ZVAL_LONG(&const_TYPE_CANONICAL_value, UCAL_ZONE_TYPE_CANONICAL);
	crex_string *const_TYPE_CANONICAL_name = crex_string_init_interned("TYPE_CANONICAL", sizeof("TYPE_CANONICAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_TYPE_CANONICAL_name, &const_TYPE_CANONICAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_TYPE_CANONICAL_name);

	zval const_TYPE_CANONICAL_LOCATION_value;
	ZVAL_LONG(&const_TYPE_CANONICAL_LOCATION_value, UCAL_ZONE_TYPE_CANONICAL_LOCATION);
	crex_string *const_TYPE_CANONICAL_LOCATION_name = crex_string_init_interned("TYPE_CANONICAL_LOCATION", sizeof("TYPE_CANONICAL_LOCATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_TYPE_CANONICAL_LOCATION_name, &const_TYPE_CANONICAL_LOCATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_TYPE_CANONICAL_LOCATION_name);

	return class_entry;
}
