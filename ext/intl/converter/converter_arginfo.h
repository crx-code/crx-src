/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 4012b69ba043f9780ea2e92714c7f2daa47e928e */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_UConverter___main, 0, 0, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, destination_encoding, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, source_encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_UConverter_convert, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, str, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, reverse, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_UConverter_fromUCallback, 0, 4, MAY_BE_STRING|MAY_BE_LONG|MAY_BE_ARRAY|MAY_BE_NULL)
	CREX_ARG_TYPE_INFO(0, reason, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, source, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, codePoint, IS_LONG, 0)
	CREX_ARG_INFO(1, error)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_UConverter_getAliases, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE|MAY_BE_NULL)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_UConverter_getAvailable, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_UConverter_getDestinationEncoding, 0, 0, MAY_BE_STRING|MAY_BE_FALSE|MAY_BE_NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_UConverter_getDestinationType, 0, 0, MAY_BE_LONG|MAY_BE_FALSE|MAY_BE_NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_UConverter_getErrorCode, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_UConverter_getErrorMessage, 0, 0, IS_STRING, 1)
CREX_END_ARG_INFO()

#define arginfo_class_UConverter_getSourceEncoding arginfo_class_UConverter_getDestinationEncoding

#define arginfo_class_UConverter_getSourceType arginfo_class_UConverter_getDestinationType

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_UConverter_getStandards, 0, 0, IS_ARRAY, 1)
CREX_END_ARG_INFO()

#define arginfo_class_UConverter_getSubstChars arginfo_class_UConverter_getDestinationEncoding

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_UConverter_reasonText, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, reason, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_UConverter_setDestinationEncoding, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, encoding, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_UConverter_setSourceEncoding arginfo_class_UConverter_setDestinationEncoding

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_UConverter_setSubstChars, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, chars, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_UConverter_toUCallback, 0, 4, MAY_BE_STRING|MAY_BE_LONG|MAY_BE_ARRAY|MAY_BE_NULL)
	CREX_ARG_TYPE_INFO(0, reason, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, source, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, codeUnits, IS_STRING, 0)
	CREX_ARG_INFO(1, error)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_UConverter_transcode, 0, 3, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, str, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, toEncoding, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, fromEncoding, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 1, "null")
CREX_END_ARG_INFO()


CREX_METHOD(UConverter, __main);
CREX_METHOD(UConverter, convert);
CREX_METHOD(UConverter, fromUCallback);
CREX_METHOD(UConverter, getAliases);
CREX_METHOD(UConverter, getAvailable);
CREX_METHOD(UConverter, getDestinationEncoding);
CREX_METHOD(UConverter, getDestinationType);
CREX_METHOD(UConverter, getErrorCode);
CREX_METHOD(UConverter, getErrorMessage);
CREX_METHOD(UConverter, getSourceEncoding);
CREX_METHOD(UConverter, getSourceType);
CREX_METHOD(UConverter, getStandards);
CREX_METHOD(UConverter, getSubstChars);
CREX_METHOD(UConverter, reasonText);
CREX_METHOD(UConverter, setDestinationEncoding);
CREX_METHOD(UConverter, setSourceEncoding);
CREX_METHOD(UConverter, setSubstChars);
CREX_METHOD(UConverter, toUCallback);
CREX_METHOD(UConverter, transcode);


static const crex_function_entry class_UConverter_methods[] = {
	CREX_ME(UConverter, __main, arginfo_class_UConverter___main, CREX_ACC_PUBLIC)
	CREX_ME(UConverter, convert, arginfo_class_UConverter_convert, CREX_ACC_PUBLIC)
	CREX_ME(UConverter, fromUCallback, arginfo_class_UConverter_fromUCallback, CREX_ACC_PUBLIC)
	CREX_ME(UConverter, getAliases, arginfo_class_UConverter_getAliases, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(UConverter, getAvailable, arginfo_class_UConverter_getAvailable, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(UConverter, getDestinationEncoding, arginfo_class_UConverter_getDestinationEncoding, CREX_ACC_PUBLIC)
	CREX_ME(UConverter, getDestinationType, arginfo_class_UConverter_getDestinationType, CREX_ACC_PUBLIC)
	CREX_ME(UConverter, getErrorCode, arginfo_class_UConverter_getErrorCode, CREX_ACC_PUBLIC)
	CREX_ME(UConverter, getErrorMessage, arginfo_class_UConverter_getErrorMessage, CREX_ACC_PUBLIC)
	CREX_ME(UConverter, getSourceEncoding, arginfo_class_UConverter_getSourceEncoding, CREX_ACC_PUBLIC)
	CREX_ME(UConverter, getSourceType, arginfo_class_UConverter_getSourceType, CREX_ACC_PUBLIC)
	CREX_ME(UConverter, getStandards, arginfo_class_UConverter_getStandards, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(UConverter, getSubstChars, arginfo_class_UConverter_getSubstChars, CREX_ACC_PUBLIC)
	CREX_ME(UConverter, reasonText, arginfo_class_UConverter_reasonText, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(UConverter, setDestinationEncoding, arginfo_class_UConverter_setDestinationEncoding, CREX_ACC_PUBLIC)
	CREX_ME(UConverter, setSourceEncoding, arginfo_class_UConverter_setSourceEncoding, CREX_ACC_PUBLIC)
	CREX_ME(UConverter, setSubstChars, arginfo_class_UConverter_setSubstChars, CREX_ACC_PUBLIC)
	CREX_ME(UConverter, toUCallback, arginfo_class_UConverter_toUCallback, CREX_ACC_PUBLIC)
	CREX_ME(UConverter, transcode, arginfo_class_UConverter_transcode, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_FE_END
};

static crex_class_entry *register_class_UConverter(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "UConverter", class_UConverter_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;

	zval const_REASON_UNASSIGNED_value;
	ZVAL_LONG(&const_REASON_UNASSIGNED_value, UCNV_UNASSIGNED);
	crex_string *const_REASON_UNASSIGNED_name = crex_string_init_interned("REASON_UNASSIGNED", sizeof("REASON_UNASSIGNED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_REASON_UNASSIGNED_name, &const_REASON_UNASSIGNED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_REASON_UNASSIGNED_name);

	zval const_REASON_ILLEGAL_value;
	ZVAL_LONG(&const_REASON_ILLEGAL_value, UCNV_ILLEGAL);
	crex_string *const_REASON_ILLEGAL_name = crex_string_init_interned("REASON_ILLEGAL", sizeof("REASON_ILLEGAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_REASON_ILLEGAL_name, &const_REASON_ILLEGAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_REASON_ILLEGAL_name);

	zval const_REASON_IRREGULAR_value;
	ZVAL_LONG(&const_REASON_IRREGULAR_value, UCNV_IRREGULAR);
	crex_string *const_REASON_IRREGULAR_name = crex_string_init_interned("REASON_IRREGULAR", sizeof("REASON_IRREGULAR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_REASON_IRREGULAR_name, &const_REASON_IRREGULAR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_REASON_IRREGULAR_name);

	zval const_REASON_RESET_value;
	ZVAL_LONG(&const_REASON_RESET_value, UCNV_RESET);
	crex_string *const_REASON_RESET_name = crex_string_init_interned("REASON_RESET", sizeof("REASON_RESET") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_REASON_RESET_name, &const_REASON_RESET_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_REASON_RESET_name);

	zval const_REASON_CLOSE_value;
	ZVAL_LONG(&const_REASON_CLOSE_value, UCNV_CLOSE);
	crex_string *const_REASON_CLOSE_name = crex_string_init_interned("REASON_CLOSE", sizeof("REASON_CLOSE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_REASON_CLOSE_name, &const_REASON_CLOSE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_REASON_CLOSE_name);

	zval const_REASON_CLONE_value;
	ZVAL_LONG(&const_REASON_CLONE_value, UCNV_CLONE);
	crex_string *const_REASON_CLONE_name = crex_string_init_interned("REASON_CLONE", sizeof("REASON_CLONE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_REASON_CLONE_name, &const_REASON_CLONE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_REASON_CLONE_name);

	zval const_UNSUPPORTED_CONVERTER_value;
	ZVAL_LONG(&const_UNSUPPORTED_CONVERTER_value, UCNV_UNSUPPORTED_CONVERTER);
	crex_string *const_UNSUPPORTED_CONVERTER_name = crex_string_init_interned("UNSUPPORTED_CONVERTER", sizeof("UNSUPPORTED_CONVERTER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_UNSUPPORTED_CONVERTER_name, &const_UNSUPPORTED_CONVERTER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_UNSUPPORTED_CONVERTER_name);

	zval const_SBCS_value;
	ZVAL_LONG(&const_SBCS_value, UCNV_SBCS);
	crex_string *const_SBCS_name = crex_string_init_interned("SBCS", sizeof("SBCS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SBCS_name, &const_SBCS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SBCS_name);

	zval const_DBCS_value;
	ZVAL_LONG(&const_DBCS_value, UCNV_DBCS);
	crex_string *const_DBCS_name = crex_string_init_interned("DBCS", sizeof("DBCS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DBCS_name, &const_DBCS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DBCS_name);

	zval const_MBCS_value;
	ZVAL_LONG(&const_MBCS_value, UCNV_MBCS);
	crex_string *const_MBCS_name = crex_string_init_interned("MBCS", sizeof("MBCS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_MBCS_name, &const_MBCS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_MBCS_name);

	zval const_LATIN_1_value;
	ZVAL_LONG(&const_LATIN_1_value, UCNV_LATIN_1);
	crex_string *const_LATIN_1_name = crex_string_init_interned("LATIN_1", sizeof("LATIN_1") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LATIN_1_name, &const_LATIN_1_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LATIN_1_name);

	zval const_UTF8_value;
	ZVAL_LONG(&const_UTF8_value, UCNV_UTF8);
	crex_string *const_UTF8_name = crex_string_init_interned("UTF8", sizeof("UTF8") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_UTF8_name, &const_UTF8_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_UTF8_name);

	zval const_UTF16_BigEndian_value;
	ZVAL_LONG(&const_UTF16_BigEndian_value, UCNV_UTF16_BigEndian);
	crex_string *const_UTF16_BigEndian_name = crex_string_init_interned("UTF16_BigEndian", sizeof("UTF16_BigEndian") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_UTF16_BigEndian_name, &const_UTF16_BigEndian_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_UTF16_BigEndian_name);

	zval const_UTF16_LittleEndian_value;
	ZVAL_LONG(&const_UTF16_LittleEndian_value, UCNV_UTF16_LittleEndian);
	crex_string *const_UTF16_LittleEndian_name = crex_string_init_interned("UTF16_LittleEndian", sizeof("UTF16_LittleEndian") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_UTF16_LittleEndian_name, &const_UTF16_LittleEndian_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_UTF16_LittleEndian_name);

	zval const_UTF32_BigEndian_value;
	ZVAL_LONG(&const_UTF32_BigEndian_value, UCNV_UTF32_BigEndian);
	crex_string *const_UTF32_BigEndian_name = crex_string_init_interned("UTF32_BigEndian", sizeof("UTF32_BigEndian") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_UTF32_BigEndian_name, &const_UTF32_BigEndian_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_UTF32_BigEndian_name);

	zval const_UTF32_LittleEndian_value;
	ZVAL_LONG(&const_UTF32_LittleEndian_value, UCNV_UTF32_LittleEndian);
	crex_string *const_UTF32_LittleEndian_name = crex_string_init_interned("UTF32_LittleEndian", sizeof("UTF32_LittleEndian") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_UTF32_LittleEndian_name, &const_UTF32_LittleEndian_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_UTF32_LittleEndian_name);

	zval const_EBCDIC_STATEFUL_value;
	ZVAL_LONG(&const_EBCDIC_STATEFUL_value, UCNV_EBCDIC_STATEFUL);
	crex_string *const_EBCDIC_STATEFUL_name = crex_string_init_interned("EBCDIC_STATEFUL", sizeof("EBCDIC_STATEFUL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_EBCDIC_STATEFUL_name, &const_EBCDIC_STATEFUL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_EBCDIC_STATEFUL_name);

	zval const_ISO_2022_value;
	ZVAL_LONG(&const_ISO_2022_value, UCNV_ISO_2022);
	crex_string *const_ISO_2022_name = crex_string_init_interned("ISO_2022", sizeof("ISO_2022") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ISO_2022_name, &const_ISO_2022_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ISO_2022_name);

	zval const_LMBCS_1_value;
	ZVAL_LONG(&const_LMBCS_1_value, UCNV_LMBCS_1);
	crex_string *const_LMBCS_1_name = crex_string_init_interned("LMBCS_1", sizeof("LMBCS_1") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LMBCS_1_name, &const_LMBCS_1_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LMBCS_1_name);

	zval const_LMBCS_2_value;
	ZVAL_LONG(&const_LMBCS_2_value, UCNV_LMBCS_2);
	crex_string *const_LMBCS_2_name = crex_string_init_interned("LMBCS_2", sizeof("LMBCS_2") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LMBCS_2_name, &const_LMBCS_2_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LMBCS_2_name);

	zval const_LMBCS_3_value;
	ZVAL_LONG(&const_LMBCS_3_value, UCNV_LMBCS_3);
	crex_string *const_LMBCS_3_name = crex_string_init_interned("LMBCS_3", sizeof("LMBCS_3") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LMBCS_3_name, &const_LMBCS_3_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LMBCS_3_name);

	zval const_LMBCS_4_value;
	ZVAL_LONG(&const_LMBCS_4_value, UCNV_LMBCS_4);
	crex_string *const_LMBCS_4_name = crex_string_init_interned("LMBCS_4", sizeof("LMBCS_4") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LMBCS_4_name, &const_LMBCS_4_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LMBCS_4_name);

	zval const_LMBCS_5_value;
	ZVAL_LONG(&const_LMBCS_5_value, UCNV_LMBCS_5);
	crex_string *const_LMBCS_5_name = crex_string_init_interned("LMBCS_5", sizeof("LMBCS_5") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LMBCS_5_name, &const_LMBCS_5_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LMBCS_5_name);

	zval const_LMBCS_6_value;
	ZVAL_LONG(&const_LMBCS_6_value, UCNV_LMBCS_6);
	crex_string *const_LMBCS_6_name = crex_string_init_interned("LMBCS_6", sizeof("LMBCS_6") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LMBCS_6_name, &const_LMBCS_6_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LMBCS_6_name);

	zval const_LMBCS_8_value;
	ZVAL_LONG(&const_LMBCS_8_value, UCNV_LMBCS_8);
	crex_string *const_LMBCS_8_name = crex_string_init_interned("LMBCS_8", sizeof("LMBCS_8") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LMBCS_8_name, &const_LMBCS_8_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LMBCS_8_name);

	zval const_LMBCS_11_value;
	ZVAL_LONG(&const_LMBCS_11_value, UCNV_LMBCS_11);
	crex_string *const_LMBCS_11_name = crex_string_init_interned("LMBCS_11", sizeof("LMBCS_11") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LMBCS_11_name, &const_LMBCS_11_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LMBCS_11_name);

	zval const_LMBCS_16_value;
	ZVAL_LONG(&const_LMBCS_16_value, UCNV_LMBCS_16);
	crex_string *const_LMBCS_16_name = crex_string_init_interned("LMBCS_16", sizeof("LMBCS_16") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LMBCS_16_name, &const_LMBCS_16_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LMBCS_16_name);

	zval const_LMBCS_17_value;
	ZVAL_LONG(&const_LMBCS_17_value, UCNV_LMBCS_17);
	crex_string *const_LMBCS_17_name = crex_string_init_interned("LMBCS_17", sizeof("LMBCS_17") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LMBCS_17_name, &const_LMBCS_17_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LMBCS_17_name);

	zval const_LMBCS_18_value;
	ZVAL_LONG(&const_LMBCS_18_value, UCNV_LMBCS_18);
	crex_string *const_LMBCS_18_name = crex_string_init_interned("LMBCS_18", sizeof("LMBCS_18") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LMBCS_18_name, &const_LMBCS_18_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LMBCS_18_name);

	zval const_LMBCS_19_value;
	ZVAL_LONG(&const_LMBCS_19_value, UCNV_LMBCS_19);
	crex_string *const_LMBCS_19_name = crex_string_init_interned("LMBCS_19", sizeof("LMBCS_19") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LMBCS_19_name, &const_LMBCS_19_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LMBCS_19_name);

	zval const_LMBCS_LAST_value;
	ZVAL_LONG(&const_LMBCS_LAST_value, UCNV_LMBCS_LAST);
	crex_string *const_LMBCS_LAST_name = crex_string_init_interned("LMBCS_LAST", sizeof("LMBCS_LAST") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LMBCS_LAST_name, &const_LMBCS_LAST_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LMBCS_LAST_name);

	zval const_HC_value;
	ZVAL_LONG(&const_HC_value, UCNV_HZ);
	crex_string *const_HC_name = crex_string_init_interned("HZ", sizeof("HZ") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_HC_name, &const_HC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_HC_name);

	zval const_SCSU_value;
	ZVAL_LONG(&const_SCSU_value, UCNV_SCSU);
	crex_string *const_SCSU_name = crex_string_init_interned("SCSU", sizeof("SCSU") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SCSU_name, &const_SCSU_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SCSU_name);

	zval const_ISCII_value;
	ZVAL_LONG(&const_ISCII_value, UCNV_ISCII);
	crex_string *const_ISCII_name = crex_string_init_interned("ISCII", sizeof("ISCII") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ISCII_name, &const_ISCII_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ISCII_name);

	zval const_US_ASCII_value;
	ZVAL_LONG(&const_US_ASCII_value, UCNV_US_ASCII);
	crex_string *const_US_ASCII_name = crex_string_init_interned("US_ASCII", sizeof("US_ASCII") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_US_ASCII_name, &const_US_ASCII_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_US_ASCII_name);

	zval const_UTF7_value;
	ZVAL_LONG(&const_UTF7_value, UCNV_UTF7);
	crex_string *const_UTF7_name = crex_string_init_interned("UTF7", sizeof("UTF7") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_UTF7_name, &const_UTF7_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_UTF7_name);

	zval const_BOCU1_value;
	ZVAL_LONG(&const_BOCU1_value, UCNV_BOCU1);
	crex_string *const_BOCU1_name = crex_string_init_interned("BOCU1", sizeof("BOCU1") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BOCU1_name, &const_BOCU1_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BOCU1_name);

	zval const_UTF16_value;
	ZVAL_LONG(&const_UTF16_value, UCNV_UTF16);
	crex_string *const_UTF16_name = crex_string_init_interned("UTF16", sizeof("UTF16") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_UTF16_name, &const_UTF16_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_UTF16_name);

	zval const_UTF32_value;
	ZVAL_LONG(&const_UTF32_value, UCNV_UTF32);
	crex_string *const_UTF32_name = crex_string_init_interned("UTF32", sizeof("UTF32") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_UTF32_name, &const_UTF32_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_UTF32_name);

	zval const_CESU8_value;
	ZVAL_LONG(&const_CESU8_value, UCNV_CESU8);
	crex_string *const_CESU8_name = crex_string_init_interned("CESU8", sizeof("CESU8") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CESU8_name, &const_CESU8_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CESU8_name);

	zval const_IMAP_MAILBOX_value;
	ZVAL_LONG(&const_IMAP_MAILBOX_value, UCNV_IMAP_MAILBOX);
	crex_string *const_IMAP_MAILBOX_name = crex_string_init_interned("IMAP_MAILBOX", sizeof("IMAP_MAILBOX") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_IMAP_MAILBOX_name, &const_IMAP_MAILBOX_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_IMAP_MAILBOX_name);

	return class_entry;
}
