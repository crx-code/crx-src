/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 59e9bd9f059c27835f78c5b95372731d265a228a */

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlChar_hasBinaryProperty, 0, 2, _IS_BOOL, 1)
	CREX_ARG_TYPE_MASK(0, codepoint, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO(0, property, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlChar_charAge, 0, 1, IS_ARRAY, 1)
	CREX_ARG_TYPE_MASK(0, codepoint, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlChar_charDigitValue, 0, 1, IS_LONG, 1)
	CREX_ARG_TYPE_MASK(0, codepoint, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

#define arginfo_class_IntlChar_charDirection arginfo_class_IntlChar_charDigitValue

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlChar_charFromName, 0, 1, IS_LONG, 1)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_LONG, 0, "IntlChar::UNICODE_CHAR_NAME")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlChar_charMirror, 0, 1, MAY_BE_LONG|MAY_BE_STRING|MAY_BE_NULL)
	CREX_ARG_TYPE_MASK(0, codepoint, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlChar_charName, 0, 1, IS_STRING, 1)
	CREX_ARG_TYPE_MASK(0, codepoint, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_LONG, 0, "IntlChar::UNICODE_CHAR_NAME")
CREX_END_ARG_INFO()

#define arginfo_class_IntlChar_charType arginfo_class_IntlChar_charDigitValue

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlChar_chr, 0, 1, IS_STRING, 1)
	CREX_ARG_TYPE_MASK(0, codepoint, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlChar_digit, 0, 1, MAY_BE_LONG|MAY_BE_FALSE|MAY_BE_NULL)
	CREX_ARG_TYPE_MASK(0, codepoint, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, base, IS_LONG, 0, "10")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlChar_enumCharNames, 0, 3, _IS_BOOL, 0)
	CREX_ARG_TYPE_MASK(0, start, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_MASK(0, end, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_LONG, 0, "IntlChar::UNICODE_CHAR_NAME")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlChar_enumCharTypes, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlChar_foldCase, 0, 1, MAY_BE_LONG|MAY_BE_STRING|MAY_BE_NULL)
	CREX_ARG_TYPE_MASK(0, codepoint, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "IntlChar::FOLD_CASE_DEFAULT")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlChar_forDigit, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, digit, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, base, IS_LONG, 0, "10")
CREX_END_ARG_INFO()

#if U_ICU_VERSION_MAJOR_NUM >= 52
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlChar_getBidiPairedBracket, 0, 1, MAY_BE_LONG|MAY_BE_STRING|MAY_BE_NULL)
	CREX_ARG_TYPE_MASK(0, codepoint, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()
#endif

#define arginfo_class_IntlChar_getBlockCode arginfo_class_IntlChar_charDigitValue

#define arginfo_class_IntlChar_getCombiningClass arginfo_class_IntlChar_charDigitValue

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlChar_getFC_NFKC_Closure, 0, 1, MAY_BE_STRING|MAY_BE_FALSE|MAY_BE_NULL)
	CREX_ARG_TYPE_MASK(0, codepoint, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlChar_getIntPropertyMaxValue, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, property, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_IntlChar_getIntPropertyMinValue arginfo_class_IntlChar_getIntPropertyMaxValue

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlChar_getIntPropertyValue, 0, 2, IS_LONG, 1)
	CREX_ARG_TYPE_MASK(0, codepoint, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO(0, property, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlChar_getNumericValue, 0, 1, IS_DOUBLE, 1)
	CREX_ARG_TYPE_MASK(0, codepoint, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlChar_getPropertyEnum, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, alias, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlChar_getPropertyName, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, property, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_LONG, 0, "IntlChar::LONG_PROPERTY_NAME")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlChar_getPropertyValueEnum, 0, 2, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, property, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_IntlChar_getPropertyValueName, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, property, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_LONG, 0, "IntlChar::LONG_PROPERTY_NAME")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlChar_getUnicodeVersion, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlChar_isalnum, 0, 1, _IS_BOOL, 1)
	CREX_ARG_TYPE_MASK(0, codepoint, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

#define arginfo_class_IntlChar_isalpha arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isbase arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isblank arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_iscntrl arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isdefined arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isdigit arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isgraph arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isIDIgnorable arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isIDPart arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isIDStart arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isISOControl arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isJavaIDPart arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isJavaIDStart arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isJavaSpaceChar arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_islower arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isMirrored arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isprint arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_ispunct arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isspace arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_istitle arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isUAlphabetic arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isULowercase arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isupper arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isUUppercase arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isUWhiteSpace arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isWhitespace arginfo_class_IntlChar_isalnum

#define arginfo_class_IntlChar_isxdigit arginfo_class_IntlChar_isalnum

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlChar_ord, 0, 1, IS_LONG, 1)
	CREX_ARG_TYPE_MASK(0, character, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

#define arginfo_class_IntlChar_tolower arginfo_class_IntlChar_charMirror

#define arginfo_class_IntlChar_totitle arginfo_class_IntlChar_charMirror

#define arginfo_class_IntlChar_toupper arginfo_class_IntlChar_charMirror


CREX_METHOD(IntlChar, hasBinaryProperty);
CREX_METHOD(IntlChar, charAge);
CREX_METHOD(IntlChar, charDigitValue);
CREX_METHOD(IntlChar, charDirection);
CREX_METHOD(IntlChar, charFromName);
CREX_METHOD(IntlChar, charMirror);
CREX_METHOD(IntlChar, charName);
CREX_METHOD(IntlChar, charType);
CREX_METHOD(IntlChar, chr);
CREX_METHOD(IntlChar, digit);
CREX_METHOD(IntlChar, enumCharNames);
CREX_METHOD(IntlChar, enumCharTypes);
CREX_METHOD(IntlChar, foldCase);
CREX_METHOD(IntlChar, forDigit);
#if U_ICU_VERSION_MAJOR_NUM >= 52
CREX_METHOD(IntlChar, getBidiPairedBracket);
#endif
CREX_METHOD(IntlChar, getBlockCode);
CREX_METHOD(IntlChar, getCombiningClass);
CREX_METHOD(IntlChar, getFC_NFKC_Closure);
CREX_METHOD(IntlChar, getIntPropertyMaxValue);
CREX_METHOD(IntlChar, getIntPropertyMinValue);
CREX_METHOD(IntlChar, getIntPropertyValue);
CREX_METHOD(IntlChar, getNumericValue);
CREX_METHOD(IntlChar, getPropertyEnum);
CREX_METHOD(IntlChar, getPropertyName);
CREX_METHOD(IntlChar, getPropertyValueEnum);
CREX_METHOD(IntlChar, getPropertyValueName);
CREX_METHOD(IntlChar, getUnicodeVersion);
CREX_METHOD(IntlChar, isalnum);
CREX_METHOD(IntlChar, isalpha);
CREX_METHOD(IntlChar, isbase);
CREX_METHOD(IntlChar, isblank);
CREX_METHOD(IntlChar, iscntrl);
CREX_METHOD(IntlChar, isdefined);
CREX_METHOD(IntlChar, isdigit);
CREX_METHOD(IntlChar, isgraph);
CREX_METHOD(IntlChar, isIDIgnorable);
CREX_METHOD(IntlChar, isIDPart);
CREX_METHOD(IntlChar, isIDStart);
CREX_METHOD(IntlChar, isISOControl);
CREX_METHOD(IntlChar, isJavaIDPart);
CREX_METHOD(IntlChar, isJavaIDStart);
CREX_METHOD(IntlChar, isJavaSpaceChar);
CREX_METHOD(IntlChar, islower);
CREX_METHOD(IntlChar, isMirrored);
CREX_METHOD(IntlChar, isprint);
CREX_METHOD(IntlChar, ispunct);
CREX_METHOD(IntlChar, isspace);
CREX_METHOD(IntlChar, istitle);
CREX_METHOD(IntlChar, isUAlphabetic);
CREX_METHOD(IntlChar, isULowercase);
CREX_METHOD(IntlChar, isupper);
CREX_METHOD(IntlChar, isUUppercase);
CREX_METHOD(IntlChar, isUWhiteSpace);
CREX_METHOD(IntlChar, isWhitespace);
CREX_METHOD(IntlChar, isxdigit);
CREX_METHOD(IntlChar, ord);
CREX_METHOD(IntlChar, tolower);
CREX_METHOD(IntlChar, totitle);
CREX_METHOD(IntlChar, toupper);


static const crex_function_entry class_IntlChar_methods[] = {
	CREX_ME(IntlChar, hasBinaryProperty, arginfo_class_IntlChar_hasBinaryProperty, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, charAge, arginfo_class_IntlChar_charAge, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, charDigitValue, arginfo_class_IntlChar_charDigitValue, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, charDirection, arginfo_class_IntlChar_charDirection, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, charFromName, arginfo_class_IntlChar_charFromName, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, charMirror, arginfo_class_IntlChar_charMirror, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, charName, arginfo_class_IntlChar_charName, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, charType, arginfo_class_IntlChar_charType, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, chr, arginfo_class_IntlChar_chr, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, digit, arginfo_class_IntlChar_digit, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, enumCharNames, arginfo_class_IntlChar_enumCharNames, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, enumCharTypes, arginfo_class_IntlChar_enumCharTypes, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, foldCase, arginfo_class_IntlChar_foldCase, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, forDigit, arginfo_class_IntlChar_forDigit, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
#if U_ICU_VERSION_MAJOR_NUM >= 52
	CREX_ME(IntlChar, getBidiPairedBracket, arginfo_class_IntlChar_getBidiPairedBracket, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
#endif
	CREX_ME(IntlChar, getBlockCode, arginfo_class_IntlChar_getBlockCode, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, getCombiningClass, arginfo_class_IntlChar_getCombiningClass, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, getFC_NFKC_Closure, arginfo_class_IntlChar_getFC_NFKC_Closure, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, getIntPropertyMaxValue, arginfo_class_IntlChar_getIntPropertyMaxValue, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, getIntPropertyMinValue, arginfo_class_IntlChar_getIntPropertyMinValue, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, getIntPropertyValue, arginfo_class_IntlChar_getIntPropertyValue, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, getNumericValue, arginfo_class_IntlChar_getNumericValue, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, getPropertyEnum, arginfo_class_IntlChar_getPropertyEnum, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, getPropertyName, arginfo_class_IntlChar_getPropertyName, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, getPropertyValueEnum, arginfo_class_IntlChar_getPropertyValueEnum, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, getPropertyValueName, arginfo_class_IntlChar_getPropertyValueName, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, getUnicodeVersion, arginfo_class_IntlChar_getUnicodeVersion, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isalnum, arginfo_class_IntlChar_isalnum, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isalpha, arginfo_class_IntlChar_isalpha, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isbase, arginfo_class_IntlChar_isbase, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isblank, arginfo_class_IntlChar_isblank, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, iscntrl, arginfo_class_IntlChar_iscntrl, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isdefined, arginfo_class_IntlChar_isdefined, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isdigit, arginfo_class_IntlChar_isdigit, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isgraph, arginfo_class_IntlChar_isgraph, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isIDIgnorable, arginfo_class_IntlChar_isIDIgnorable, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isIDPart, arginfo_class_IntlChar_isIDPart, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isIDStart, arginfo_class_IntlChar_isIDStart, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isISOControl, arginfo_class_IntlChar_isISOControl, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isJavaIDPart, arginfo_class_IntlChar_isJavaIDPart, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isJavaIDStart, arginfo_class_IntlChar_isJavaIDStart, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isJavaSpaceChar, arginfo_class_IntlChar_isJavaSpaceChar, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, islower, arginfo_class_IntlChar_islower, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isMirrored, arginfo_class_IntlChar_isMirrored, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isprint, arginfo_class_IntlChar_isprint, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, ispunct, arginfo_class_IntlChar_ispunct, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isspace, arginfo_class_IntlChar_isspace, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, istitle, arginfo_class_IntlChar_istitle, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isUAlphabetic, arginfo_class_IntlChar_isUAlphabetic, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isULowercase, arginfo_class_IntlChar_isULowercase, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isupper, arginfo_class_IntlChar_isupper, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isUUppercase, arginfo_class_IntlChar_isUUppercase, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isUWhiteSpace, arginfo_class_IntlChar_isUWhiteSpace, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isWhitespace, arginfo_class_IntlChar_isWhitespace, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, isxdigit, arginfo_class_IntlChar_isxdigit, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, ord, arginfo_class_IntlChar_ord, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, tolower, arginfo_class_IntlChar_tolower, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, totitle, arginfo_class_IntlChar_totitle, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlChar, toupper, arginfo_class_IntlChar_toupper, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_FE_END
};

static crex_class_entry *register_class_IntlChar(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "IntlChar", class_IntlChar_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	zval const_UNICODE_VERSION_value;
	crex_string *const_UNICODE_VERSION_value_str = crex_string_init(U_UNICODE_VERSION, strlen(U_UNICODE_VERSION), 1);
	ZVAL_STR(&const_UNICODE_VERSION_value, const_UNICODE_VERSION_value_str);
	crex_string *const_UNICODE_VERSION_name = crex_string_init_interned("UNICODE_VERSION", sizeof("UNICODE_VERSION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_UNICODE_VERSION_name, &const_UNICODE_VERSION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_UNICODE_VERSION_name);

	zval const_CODEPOINT_MIN_value;
	ZVAL_LONG(&const_CODEPOINT_MIN_value, UCHAR_MIN_VALUE);
	crex_string *const_CODEPOINT_MIN_name = crex_string_init_interned("CODEPOINT_MIN", sizeof("CODEPOINT_MIN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CODEPOINT_MIN_name, &const_CODEPOINT_MIN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CODEPOINT_MIN_name);

	zval const_CODEPOINT_MAX_value;
	ZVAL_LONG(&const_CODEPOINT_MAX_value, UCHAR_MAX_VALUE);
	crex_string *const_CODEPOINT_MAX_name = crex_string_init_interned("CODEPOINT_MAX", sizeof("CODEPOINT_MAX") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CODEPOINT_MAX_name, &const_CODEPOINT_MAX_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CODEPOINT_MAX_name);

	zval const_NO_NUMERIC_VALUE_value;
	ZVAL_DOUBLE(&const_NO_NUMERIC_VALUE_value, U_NO_NUMERIC_VALUE);
	crex_string *const_NO_NUMERIC_VALUE_name = crex_string_init_interned("NO_NUMERIC_VALUE", sizeof("NO_NUMERIC_VALUE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NO_NUMERIC_VALUE_name, &const_NO_NUMERIC_VALUE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NO_NUMERIC_VALUE_name);

	zval const_PROPERTY_ALPHABETIC_value;
	ZVAL_LONG(&const_PROPERTY_ALPHABETIC_value, UCHAR_ALPHABETIC);
	crex_string *const_PROPERTY_ALPHABETIC_name = crex_string_init_interned("PROPERTY_ALPHABETIC", sizeof("PROPERTY_ALPHABETIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_ALPHABETIC_name, &const_PROPERTY_ALPHABETIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_ALPHABETIC_name);

	zval const_PROPERTY_BINARY_START_value;
	ZVAL_LONG(&const_PROPERTY_BINARY_START_value, UCHAR_BINARY_START);
	crex_string *const_PROPERTY_BINARY_START_name = crex_string_init_interned("PROPERTY_BINARY_START", sizeof("PROPERTY_BINARY_START") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_BINARY_START_name, &const_PROPERTY_BINARY_START_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_BINARY_START_name);

	zval const_PROPERTY_ASCII_HEX_DIGIT_value;
	ZVAL_LONG(&const_PROPERTY_ASCII_HEX_DIGIT_value, UCHAR_ASCII_HEX_DIGIT);
	crex_string *const_PROPERTY_ASCII_HEX_DIGIT_name = crex_string_init_interned("PROPERTY_ASCII_HEX_DIGIT", sizeof("PROPERTY_ASCII_HEX_DIGIT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_ASCII_HEX_DIGIT_name, &const_PROPERTY_ASCII_HEX_DIGIT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_ASCII_HEX_DIGIT_name);

	zval const_PROPERTY_BIDI_CONTROL_value;
	ZVAL_LONG(&const_PROPERTY_BIDI_CONTROL_value, UCHAR_BIDI_CONTROL);
	crex_string *const_PROPERTY_BIDI_CONTROL_name = crex_string_init_interned("PROPERTY_BIDI_CONTROL", sizeof("PROPERTY_BIDI_CONTROL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_BIDI_CONTROL_name, &const_PROPERTY_BIDI_CONTROL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_BIDI_CONTROL_name);

	zval const_PROPERTY_BIDI_MIRRORED_value;
	ZVAL_LONG(&const_PROPERTY_BIDI_MIRRORED_value, UCHAR_BIDI_MIRRORED);
	crex_string *const_PROPERTY_BIDI_MIRRORED_name = crex_string_init_interned("PROPERTY_BIDI_MIRRORED", sizeof("PROPERTY_BIDI_MIRRORED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_BIDI_MIRRORED_name, &const_PROPERTY_BIDI_MIRRORED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_BIDI_MIRRORED_name);

	zval const_PROPERTY_DASH_value;
	ZVAL_LONG(&const_PROPERTY_DASH_value, UCHAR_DASH);
	crex_string *const_PROPERTY_DASH_name = crex_string_init_interned("PROPERTY_DASH", sizeof("PROPERTY_DASH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_DASH_name, &const_PROPERTY_DASH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_DASH_name);

	zval const_PROPERTY_DEFAULT_IGNORABLE_CODE_POINT_value;
	ZVAL_LONG(&const_PROPERTY_DEFAULT_IGNORABLE_CODE_POINT_value, UCHAR_DEFAULT_IGNORABLE_CODE_POINT);
	crex_string *const_PROPERTY_DEFAULT_IGNORABLE_CODE_POINT_name = crex_string_init_interned("PROPERTY_DEFAULT_IGNORABLE_CODE_POINT", sizeof("PROPERTY_DEFAULT_IGNORABLE_CODE_POINT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_DEFAULT_IGNORABLE_CODE_POINT_name, &const_PROPERTY_DEFAULT_IGNORABLE_CODE_POINT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_DEFAULT_IGNORABLE_CODE_POINT_name);

	zval const_PROPERTY_DEPRECATED_value;
	ZVAL_LONG(&const_PROPERTY_DEPRECATED_value, UCHAR_DEPRECATED);
	crex_string *const_PROPERTY_DEPRECATED_name = crex_string_init_interned("PROPERTY_DEPRECATED", sizeof("PROPERTY_DEPRECATED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_DEPRECATED_name, &const_PROPERTY_DEPRECATED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_DEPRECATED_name);

	zval const_PROPERTY_DIACRITIC_value;
	ZVAL_LONG(&const_PROPERTY_DIACRITIC_value, UCHAR_DIACRITIC);
	crex_string *const_PROPERTY_DIACRITIC_name = crex_string_init_interned("PROPERTY_DIACRITIC", sizeof("PROPERTY_DIACRITIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_DIACRITIC_name, &const_PROPERTY_DIACRITIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_DIACRITIC_name);

	zval const_PROPERTY_EXTENDER_value;
	ZVAL_LONG(&const_PROPERTY_EXTENDER_value, UCHAR_EXTENDER);
	crex_string *const_PROPERTY_EXTENDER_name = crex_string_init_interned("PROPERTY_EXTENDER", sizeof("PROPERTY_EXTENDER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_EXTENDER_name, &const_PROPERTY_EXTENDER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_EXTENDER_name);

	zval const_PROPERTY_FULL_COMPOSITION_EXCLUSION_value;
	ZVAL_LONG(&const_PROPERTY_FULL_COMPOSITION_EXCLUSION_value, UCHAR_FULL_COMPOSITION_EXCLUSION);
	crex_string *const_PROPERTY_FULL_COMPOSITION_EXCLUSION_name = crex_string_init_interned("PROPERTY_FULL_COMPOSITION_EXCLUSION", sizeof("PROPERTY_FULL_COMPOSITION_EXCLUSION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_FULL_COMPOSITION_EXCLUSION_name, &const_PROPERTY_FULL_COMPOSITION_EXCLUSION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_FULL_COMPOSITION_EXCLUSION_name);

	zval const_PROPERTY_GRAPHEME_BASE_value;
	ZVAL_LONG(&const_PROPERTY_GRAPHEME_BASE_value, UCHAR_GRAPHEME_BASE);
	crex_string *const_PROPERTY_GRAPHEME_BASE_name = crex_string_init_interned("PROPERTY_GRAPHEME_BASE", sizeof("PROPERTY_GRAPHEME_BASE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_GRAPHEME_BASE_name, &const_PROPERTY_GRAPHEME_BASE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_GRAPHEME_BASE_name);

	zval const_PROPERTY_GRAPHEME_EXTEND_value;
	ZVAL_LONG(&const_PROPERTY_GRAPHEME_EXTEND_value, UCHAR_GRAPHEME_EXTEND);
	crex_string *const_PROPERTY_GRAPHEME_EXTEND_name = crex_string_init_interned("PROPERTY_GRAPHEME_EXTEND", sizeof("PROPERTY_GRAPHEME_EXTEND") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_GRAPHEME_EXTEND_name, &const_PROPERTY_GRAPHEME_EXTEND_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_GRAPHEME_EXTEND_name);

	zval const_PROPERTY_GRAPHEME_LINK_value;
	ZVAL_LONG(&const_PROPERTY_GRAPHEME_LINK_value, UCHAR_GRAPHEME_LINK);
	crex_string *const_PROPERTY_GRAPHEME_LINK_name = crex_string_init_interned("PROPERTY_GRAPHEME_LINK", sizeof("PROPERTY_GRAPHEME_LINK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_GRAPHEME_LINK_name, &const_PROPERTY_GRAPHEME_LINK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_GRAPHEME_LINK_name);

	zval const_PROPERTY_HEX_DIGIT_value;
	ZVAL_LONG(&const_PROPERTY_HEX_DIGIT_value, UCHAR_HEX_DIGIT);
	crex_string *const_PROPERTY_HEX_DIGIT_name = crex_string_init_interned("PROPERTY_HEX_DIGIT", sizeof("PROPERTY_HEX_DIGIT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_HEX_DIGIT_name, &const_PROPERTY_HEX_DIGIT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_HEX_DIGIT_name);

	zval const_PROPERTY_HYPHEN_value;
	ZVAL_LONG(&const_PROPERTY_HYPHEN_value, UCHAR_HYPHEN);
	crex_string *const_PROPERTY_HYPHEN_name = crex_string_init_interned("PROPERTY_HYPHEN", sizeof("PROPERTY_HYPHEN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_HYPHEN_name, &const_PROPERTY_HYPHEN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_HYPHEN_name);

	zval const_PROPERTY_ID_CONTINUE_value;
	ZVAL_LONG(&const_PROPERTY_ID_CONTINUE_value, UCHAR_ID_CONTINUE);
	crex_string *const_PROPERTY_ID_CONTINUE_name = crex_string_init_interned("PROPERTY_ID_CONTINUE", sizeof("PROPERTY_ID_CONTINUE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_ID_CONTINUE_name, &const_PROPERTY_ID_CONTINUE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_ID_CONTINUE_name);

	zval const_PROPERTY_ID_START_value;
	ZVAL_LONG(&const_PROPERTY_ID_START_value, UCHAR_ID_START);
	crex_string *const_PROPERTY_ID_START_name = crex_string_init_interned("PROPERTY_ID_START", sizeof("PROPERTY_ID_START") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_ID_START_name, &const_PROPERTY_ID_START_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_ID_START_name);

	zval const_PROPERTY_IDEOGRAPHIC_value;
	ZVAL_LONG(&const_PROPERTY_IDEOGRAPHIC_value, UCHAR_IDEOGRAPHIC);
	crex_string *const_PROPERTY_IDEOGRAPHIC_name = crex_string_init_interned("PROPERTY_IDEOGRAPHIC", sizeof("PROPERTY_IDEOGRAPHIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_IDEOGRAPHIC_name, &const_PROPERTY_IDEOGRAPHIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_IDEOGRAPHIC_name);

	zval const_PROPERTY_IDS_BINARY_OPERATOR_value;
	ZVAL_LONG(&const_PROPERTY_IDS_BINARY_OPERATOR_value, UCHAR_IDS_BINARY_OPERATOR);
	crex_string *const_PROPERTY_IDS_BINARY_OPERATOR_name = crex_string_init_interned("PROPERTY_IDS_BINARY_OPERATOR", sizeof("PROPERTY_IDS_BINARY_OPERATOR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_IDS_BINARY_OPERATOR_name, &const_PROPERTY_IDS_BINARY_OPERATOR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_IDS_BINARY_OPERATOR_name);

	zval const_PROPERTY_IDS_TRINARY_OPERATOR_value;
	ZVAL_LONG(&const_PROPERTY_IDS_TRINARY_OPERATOR_value, UCHAR_IDS_TRINARY_OPERATOR);
	crex_string *const_PROPERTY_IDS_TRINARY_OPERATOR_name = crex_string_init_interned("PROPERTY_IDS_TRINARY_OPERATOR", sizeof("PROPERTY_IDS_TRINARY_OPERATOR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_IDS_TRINARY_OPERATOR_name, &const_PROPERTY_IDS_TRINARY_OPERATOR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_IDS_TRINARY_OPERATOR_name);

	zval const_PROPERTY_JOIN_CONTROL_value;
	ZVAL_LONG(&const_PROPERTY_JOIN_CONTROL_value, UCHAR_JOIN_CONTROL);
	crex_string *const_PROPERTY_JOIN_CONTROL_name = crex_string_init_interned("PROPERTY_JOIN_CONTROL", sizeof("PROPERTY_JOIN_CONTROL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_JOIN_CONTROL_name, &const_PROPERTY_JOIN_CONTROL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_JOIN_CONTROL_name);

	zval const_PROPERTY_LOGICAL_ORDER_EXCEPTION_value;
	ZVAL_LONG(&const_PROPERTY_LOGICAL_ORDER_EXCEPTION_value, UCHAR_LOGICAL_ORDER_EXCEPTION);
	crex_string *const_PROPERTY_LOGICAL_ORDER_EXCEPTION_name = crex_string_init_interned("PROPERTY_LOGICAL_ORDER_EXCEPTION", sizeof("PROPERTY_LOGICAL_ORDER_EXCEPTION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_LOGICAL_ORDER_EXCEPTION_name, &const_PROPERTY_LOGICAL_ORDER_EXCEPTION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_LOGICAL_ORDER_EXCEPTION_name);

	zval const_PROPERTY_LOWERCASE_value;
	ZVAL_LONG(&const_PROPERTY_LOWERCASE_value, UCHAR_LOWERCASE);
	crex_string *const_PROPERTY_LOWERCASE_name = crex_string_init_interned("PROPERTY_LOWERCASE", sizeof("PROPERTY_LOWERCASE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_LOWERCASE_name, &const_PROPERTY_LOWERCASE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_LOWERCASE_name);

	zval const_PROPERTY_MATH_value;
	ZVAL_LONG(&const_PROPERTY_MATH_value, UCHAR_MATH);
	crex_string *const_PROPERTY_MATH_name = crex_string_init_interned("PROPERTY_MATH", sizeof("PROPERTY_MATH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_MATH_name, &const_PROPERTY_MATH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_MATH_name);

	zval const_PROPERTY_NONCHARACTER_CODE_POINT_value;
	ZVAL_LONG(&const_PROPERTY_NONCHARACTER_CODE_POINT_value, UCHAR_NONCHARACTER_CODE_POINT);
	crex_string *const_PROPERTY_NONCHARACTER_CODE_POINT_name = crex_string_init_interned("PROPERTY_NONCHARACTER_CODE_POINT", sizeof("PROPERTY_NONCHARACTER_CODE_POINT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_NONCHARACTER_CODE_POINT_name, &const_PROPERTY_NONCHARACTER_CODE_POINT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_NONCHARACTER_CODE_POINT_name);

	zval const_PROPERTY_QUOTATION_MARK_value;
	ZVAL_LONG(&const_PROPERTY_QUOTATION_MARK_value, UCHAR_QUOTATION_MARK);
	crex_string *const_PROPERTY_QUOTATION_MARK_name = crex_string_init_interned("PROPERTY_QUOTATION_MARK", sizeof("PROPERTY_QUOTATION_MARK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_QUOTATION_MARK_name, &const_PROPERTY_QUOTATION_MARK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_QUOTATION_MARK_name);

	zval const_PROPERTY_RADICAL_value;
	ZVAL_LONG(&const_PROPERTY_RADICAL_value, UCHAR_RADICAL);
	crex_string *const_PROPERTY_RADICAL_name = crex_string_init_interned("PROPERTY_RADICAL", sizeof("PROPERTY_RADICAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_RADICAL_name, &const_PROPERTY_RADICAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_RADICAL_name);

	zval const_PROPERTY_SOFT_DOTTED_value;
	ZVAL_LONG(&const_PROPERTY_SOFT_DOTTED_value, UCHAR_SOFT_DOTTED);
	crex_string *const_PROPERTY_SOFT_DOTTED_name = crex_string_init_interned("PROPERTY_SOFT_DOTTED", sizeof("PROPERTY_SOFT_DOTTED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_SOFT_DOTTED_name, &const_PROPERTY_SOFT_DOTTED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_SOFT_DOTTED_name);

	zval const_PROPERTY_TERMINAL_PUNCTUATION_value;
	ZVAL_LONG(&const_PROPERTY_TERMINAL_PUNCTUATION_value, UCHAR_TERMINAL_PUNCTUATION);
	crex_string *const_PROPERTY_TERMINAL_PUNCTUATION_name = crex_string_init_interned("PROPERTY_TERMINAL_PUNCTUATION", sizeof("PROPERTY_TERMINAL_PUNCTUATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_TERMINAL_PUNCTUATION_name, &const_PROPERTY_TERMINAL_PUNCTUATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_TERMINAL_PUNCTUATION_name);

	zval const_PROPERTY_UNIFIED_IDEOGRAPH_value;
	ZVAL_LONG(&const_PROPERTY_UNIFIED_IDEOGRAPH_value, UCHAR_UNIFIED_IDEOGRAPH);
	crex_string *const_PROPERTY_UNIFIED_IDEOGRAPH_name = crex_string_init_interned("PROPERTY_UNIFIED_IDEOGRAPH", sizeof("PROPERTY_UNIFIED_IDEOGRAPH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_UNIFIED_IDEOGRAPH_name, &const_PROPERTY_UNIFIED_IDEOGRAPH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_UNIFIED_IDEOGRAPH_name);

	zval const_PROPERTY_UPPERCASE_value;
	ZVAL_LONG(&const_PROPERTY_UPPERCASE_value, UCHAR_UPPERCASE);
	crex_string *const_PROPERTY_UPPERCASE_name = crex_string_init_interned("PROPERTY_UPPERCASE", sizeof("PROPERTY_UPPERCASE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_UPPERCASE_name, &const_PROPERTY_UPPERCASE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_UPPERCASE_name);

	zval const_PROPERTY_WHITE_SPACE_value;
	ZVAL_LONG(&const_PROPERTY_WHITE_SPACE_value, UCHAR_WHITE_SPACE);
	crex_string *const_PROPERTY_WHITE_SPACE_name = crex_string_init_interned("PROPERTY_WHITE_SPACE", sizeof("PROPERTY_WHITE_SPACE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_WHITE_SPACE_name, &const_PROPERTY_WHITE_SPACE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_WHITE_SPACE_name);

	zval const_PROPERTY_XID_CONTINUE_value;
	ZVAL_LONG(&const_PROPERTY_XID_CONTINUE_value, UCHAR_XID_CONTINUE);
	crex_string *const_PROPERTY_XID_CONTINUE_name = crex_string_init_interned("PROPERTY_XID_CONTINUE", sizeof("PROPERTY_XID_CONTINUE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_XID_CONTINUE_name, &const_PROPERTY_XID_CONTINUE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_XID_CONTINUE_name);

	zval const_PROPERTY_XID_START_value;
	ZVAL_LONG(&const_PROPERTY_XID_START_value, UCHAR_XID_START);
	crex_string *const_PROPERTY_XID_START_name = crex_string_init_interned("PROPERTY_XID_START", sizeof("PROPERTY_XID_START") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_XID_START_name, &const_PROPERTY_XID_START_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_XID_START_name);

	zval const_PROPERTY_CASE_SENSITIVE_value;
	ZVAL_LONG(&const_PROPERTY_CASE_SENSITIVE_value, UCHAR_CASE_SENSITIVE);
	crex_string *const_PROPERTY_CASE_SENSITIVE_name = crex_string_init_interned("PROPERTY_CASE_SENSITIVE", sizeof("PROPERTY_CASE_SENSITIVE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_CASE_SENSITIVE_name, &const_PROPERTY_CASE_SENSITIVE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_CASE_SENSITIVE_name);

	zval const_PROPERTY_S_TERM_value;
	ZVAL_LONG(&const_PROPERTY_S_TERM_value, UCHAR_S_TERM);
	crex_string *const_PROPERTY_S_TERM_name = crex_string_init_interned("PROPERTY_S_TERM", sizeof("PROPERTY_S_TERM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_S_TERM_name, &const_PROPERTY_S_TERM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_S_TERM_name);

	zval const_PROPERTY_VARIATION_SELECTOR_value;
	ZVAL_LONG(&const_PROPERTY_VARIATION_SELECTOR_value, UCHAR_VARIATION_SELECTOR);
	crex_string *const_PROPERTY_VARIATION_SELECTOR_name = crex_string_init_interned("PROPERTY_VARIATION_SELECTOR", sizeof("PROPERTY_VARIATION_SELECTOR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_VARIATION_SELECTOR_name, &const_PROPERTY_VARIATION_SELECTOR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_VARIATION_SELECTOR_name);

	zval const_PROPERTY_NFD_INERT_value;
	ZVAL_LONG(&const_PROPERTY_NFD_INERT_value, UCHAR_NFD_INERT);
	crex_string *const_PROPERTY_NFD_INERT_name = crex_string_init_interned("PROPERTY_NFD_INERT", sizeof("PROPERTY_NFD_INERT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_NFD_INERT_name, &const_PROPERTY_NFD_INERT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_NFD_INERT_name);

	zval const_PROPERTY_NFKD_INERT_value;
	ZVAL_LONG(&const_PROPERTY_NFKD_INERT_value, UCHAR_NFKD_INERT);
	crex_string *const_PROPERTY_NFKD_INERT_name = crex_string_init_interned("PROPERTY_NFKD_INERT", sizeof("PROPERTY_NFKD_INERT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_NFKD_INERT_name, &const_PROPERTY_NFKD_INERT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_NFKD_INERT_name);

	zval const_PROPERTY_NFC_INERT_value;
	ZVAL_LONG(&const_PROPERTY_NFC_INERT_value, UCHAR_NFC_INERT);
	crex_string *const_PROPERTY_NFC_INERT_name = crex_string_init_interned("PROPERTY_NFC_INERT", sizeof("PROPERTY_NFC_INERT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_NFC_INERT_name, &const_PROPERTY_NFC_INERT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_NFC_INERT_name);

	zval const_PROPERTY_NFKC_INERT_value;
	ZVAL_LONG(&const_PROPERTY_NFKC_INERT_value, UCHAR_NFKC_INERT);
	crex_string *const_PROPERTY_NFKC_INERT_name = crex_string_init_interned("PROPERTY_NFKC_INERT", sizeof("PROPERTY_NFKC_INERT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_NFKC_INERT_name, &const_PROPERTY_NFKC_INERT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_NFKC_INERT_name);

	zval const_PROPERTY_SEGMENT_STARTER_value;
	ZVAL_LONG(&const_PROPERTY_SEGMENT_STARTER_value, UCHAR_SEGMENT_STARTER);
	crex_string *const_PROPERTY_SEGMENT_STARTER_name = crex_string_init_interned("PROPERTY_SEGMENT_STARTER", sizeof("PROPERTY_SEGMENT_STARTER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_SEGMENT_STARTER_name, &const_PROPERTY_SEGMENT_STARTER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_SEGMENT_STARTER_name);

	zval const_PROPERTY_PATTERN_SYNTAX_value;
	ZVAL_LONG(&const_PROPERTY_PATTERN_SYNTAX_value, UCHAR_PATTERN_SYNTAX);
	crex_string *const_PROPERTY_PATTERN_SYNTAX_name = crex_string_init_interned("PROPERTY_PATTERN_SYNTAX", sizeof("PROPERTY_PATTERN_SYNTAX") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_PATTERN_SYNTAX_name, &const_PROPERTY_PATTERN_SYNTAX_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_PATTERN_SYNTAX_name);

	zval const_PROPERTY_PATTERN_WHITE_SPACE_value;
	ZVAL_LONG(&const_PROPERTY_PATTERN_WHITE_SPACE_value, UCHAR_PATTERN_WHITE_SPACE);
	crex_string *const_PROPERTY_PATTERN_WHITE_SPACE_name = crex_string_init_interned("PROPERTY_PATTERN_WHITE_SPACE", sizeof("PROPERTY_PATTERN_WHITE_SPACE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_PATTERN_WHITE_SPACE_name, &const_PROPERTY_PATTERN_WHITE_SPACE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_PATTERN_WHITE_SPACE_name);

	zval const_PROPERTY_POSIX_ALNUM_value;
	ZVAL_LONG(&const_PROPERTY_POSIX_ALNUM_value, UCHAR_POSIX_ALNUM);
	crex_string *const_PROPERTY_POSIX_ALNUM_name = crex_string_init_interned("PROPERTY_POSIX_ALNUM", sizeof("PROPERTY_POSIX_ALNUM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_POSIX_ALNUM_name, &const_PROPERTY_POSIX_ALNUM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_POSIX_ALNUM_name);

	zval const_PROPERTY_POSIX_BLANK_value;
	ZVAL_LONG(&const_PROPERTY_POSIX_BLANK_value, UCHAR_POSIX_BLANK);
	crex_string *const_PROPERTY_POSIX_BLANK_name = crex_string_init_interned("PROPERTY_POSIX_BLANK", sizeof("PROPERTY_POSIX_BLANK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_POSIX_BLANK_name, &const_PROPERTY_POSIX_BLANK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_POSIX_BLANK_name);

	zval const_PROPERTY_POSIX_GRAPH_value;
	ZVAL_LONG(&const_PROPERTY_POSIX_GRAPH_value, UCHAR_POSIX_GRAPH);
	crex_string *const_PROPERTY_POSIX_GRAPH_name = crex_string_init_interned("PROPERTY_POSIX_GRAPH", sizeof("PROPERTY_POSIX_GRAPH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_POSIX_GRAPH_name, &const_PROPERTY_POSIX_GRAPH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_POSIX_GRAPH_name);

	zval const_PROPERTY_POSIX_PRINT_value;
	ZVAL_LONG(&const_PROPERTY_POSIX_PRINT_value, UCHAR_POSIX_PRINT);
	crex_string *const_PROPERTY_POSIX_PRINT_name = crex_string_init_interned("PROPERTY_POSIX_PRINT", sizeof("PROPERTY_POSIX_PRINT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_POSIX_PRINT_name, &const_PROPERTY_POSIX_PRINT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_POSIX_PRINT_name);

	zval const_PROPERTY_POSIX_XDIGIT_value;
	ZVAL_LONG(&const_PROPERTY_POSIX_XDIGIT_value, UCHAR_POSIX_XDIGIT);
	crex_string *const_PROPERTY_POSIX_XDIGIT_name = crex_string_init_interned("PROPERTY_POSIX_XDIGIT", sizeof("PROPERTY_POSIX_XDIGIT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_POSIX_XDIGIT_name, &const_PROPERTY_POSIX_XDIGIT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_POSIX_XDIGIT_name);

	zval const_PROPERTY_CASED_value;
	ZVAL_LONG(&const_PROPERTY_CASED_value, UCHAR_CASED);
	crex_string *const_PROPERTY_CASED_name = crex_string_init_interned("PROPERTY_CASED", sizeof("PROPERTY_CASED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_CASED_name, &const_PROPERTY_CASED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_CASED_name);

	zval const_PROPERTY_CASE_IGNORABLE_value;
	ZVAL_LONG(&const_PROPERTY_CASE_IGNORABLE_value, UCHAR_CASE_IGNORABLE);
	crex_string *const_PROPERTY_CASE_IGNORABLE_name = crex_string_init_interned("PROPERTY_CASE_IGNORABLE", sizeof("PROPERTY_CASE_IGNORABLE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_CASE_IGNORABLE_name, &const_PROPERTY_CASE_IGNORABLE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_CASE_IGNORABLE_name);

	zval const_PROPERTY_CHANGES_WHEN_LOWERCASED_value;
	ZVAL_LONG(&const_PROPERTY_CHANGES_WHEN_LOWERCASED_value, UCHAR_CHANGES_WHEN_LOWERCASED);
	crex_string *const_PROPERTY_CHANGES_WHEN_LOWERCASED_name = crex_string_init_interned("PROPERTY_CHANGES_WHEN_LOWERCASED", sizeof("PROPERTY_CHANGES_WHEN_LOWERCASED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_CHANGES_WHEN_LOWERCASED_name, &const_PROPERTY_CHANGES_WHEN_LOWERCASED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_CHANGES_WHEN_LOWERCASED_name);

	zval const_PROPERTY_CHANGES_WHEN_UPPERCASED_value;
	ZVAL_LONG(&const_PROPERTY_CHANGES_WHEN_UPPERCASED_value, UCHAR_CHANGES_WHEN_UPPERCASED);
	crex_string *const_PROPERTY_CHANGES_WHEN_UPPERCASED_name = crex_string_init_interned("PROPERTY_CHANGES_WHEN_UPPERCASED", sizeof("PROPERTY_CHANGES_WHEN_UPPERCASED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_CHANGES_WHEN_UPPERCASED_name, &const_PROPERTY_CHANGES_WHEN_UPPERCASED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_CHANGES_WHEN_UPPERCASED_name);

	zval const_PROPERTY_CHANGES_WHEN_TITLECASED_value;
	ZVAL_LONG(&const_PROPERTY_CHANGES_WHEN_TITLECASED_value, UCHAR_CHANGES_WHEN_TITLECASED);
	crex_string *const_PROPERTY_CHANGES_WHEN_TITLECASED_name = crex_string_init_interned("PROPERTY_CHANGES_WHEN_TITLECASED", sizeof("PROPERTY_CHANGES_WHEN_TITLECASED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_CHANGES_WHEN_TITLECASED_name, &const_PROPERTY_CHANGES_WHEN_TITLECASED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_CHANGES_WHEN_TITLECASED_name);

	zval const_PROPERTY_CHANGES_WHEN_CASEFOLDED_value;
	ZVAL_LONG(&const_PROPERTY_CHANGES_WHEN_CASEFOLDED_value, UCHAR_CHANGES_WHEN_CASEFOLDED);
	crex_string *const_PROPERTY_CHANGES_WHEN_CASEFOLDED_name = crex_string_init_interned("PROPERTY_CHANGES_WHEN_CASEFOLDED", sizeof("PROPERTY_CHANGES_WHEN_CASEFOLDED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_CHANGES_WHEN_CASEFOLDED_name, &const_PROPERTY_CHANGES_WHEN_CASEFOLDED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_CHANGES_WHEN_CASEFOLDED_name);

	zval const_PROPERTY_CHANGES_WHEN_CASEMAPPED_value;
	ZVAL_LONG(&const_PROPERTY_CHANGES_WHEN_CASEMAPPED_value, UCHAR_CHANGES_WHEN_CASEMAPPED);
	crex_string *const_PROPERTY_CHANGES_WHEN_CASEMAPPED_name = crex_string_init_interned("PROPERTY_CHANGES_WHEN_CASEMAPPED", sizeof("PROPERTY_CHANGES_WHEN_CASEMAPPED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_CHANGES_WHEN_CASEMAPPED_name, &const_PROPERTY_CHANGES_WHEN_CASEMAPPED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_CHANGES_WHEN_CASEMAPPED_name);

	zval const_PROPERTY_CHANGES_WHEN_NFKC_CASEFOLDED_value;
	ZVAL_LONG(&const_PROPERTY_CHANGES_WHEN_NFKC_CASEFOLDED_value, UCHAR_CHANGES_WHEN_NFKC_CASEFOLDED);
	crex_string *const_PROPERTY_CHANGES_WHEN_NFKC_CASEFOLDED_name = crex_string_init_interned("PROPERTY_CHANGES_WHEN_NFKC_CASEFOLDED", sizeof("PROPERTY_CHANGES_WHEN_NFKC_CASEFOLDED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_CHANGES_WHEN_NFKC_CASEFOLDED_name, &const_PROPERTY_CHANGES_WHEN_NFKC_CASEFOLDED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_CHANGES_WHEN_NFKC_CASEFOLDED_name);

	zval const_PROPERTY_BINARY_LIMIT_value;
	ZVAL_LONG(&const_PROPERTY_BINARY_LIMIT_value, UCHAR_BINARY_LIMIT);
	crex_string *const_PROPERTY_BINARY_LIMIT_name = crex_string_init_interned("PROPERTY_BINARY_LIMIT", sizeof("PROPERTY_BINARY_LIMIT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_BINARY_LIMIT_name, &const_PROPERTY_BINARY_LIMIT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_BINARY_LIMIT_name);

	zval const_PROPERTY_BIDI_CLASS_value;
	ZVAL_LONG(&const_PROPERTY_BIDI_CLASS_value, UCHAR_BIDI_CLASS);
	crex_string *const_PROPERTY_BIDI_CLASS_name = crex_string_init_interned("PROPERTY_BIDI_CLASS", sizeof("PROPERTY_BIDI_CLASS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_BIDI_CLASS_name, &const_PROPERTY_BIDI_CLASS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_BIDI_CLASS_name);

	zval const_PROPERTY_INT_START_value;
	ZVAL_LONG(&const_PROPERTY_INT_START_value, UCHAR_INT_START);
	crex_string *const_PROPERTY_INT_START_name = crex_string_init_interned("PROPERTY_INT_START", sizeof("PROPERTY_INT_START") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_INT_START_name, &const_PROPERTY_INT_START_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_INT_START_name);

	zval const_PROPERTY_BLOCK_value;
	ZVAL_LONG(&const_PROPERTY_BLOCK_value, UCHAR_BLOCK);
	crex_string *const_PROPERTY_BLOCK_name = crex_string_init_interned("PROPERTY_BLOCK", sizeof("PROPERTY_BLOCK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_BLOCK_name, &const_PROPERTY_BLOCK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_BLOCK_name);

	zval const_PROPERTY_CANONICAL_COMBINING_CLASS_value;
	ZVAL_LONG(&const_PROPERTY_CANONICAL_COMBINING_CLASS_value, UCHAR_CANONICAL_COMBINING_CLASS);
	crex_string *const_PROPERTY_CANONICAL_COMBINING_CLASS_name = crex_string_init_interned("PROPERTY_CANONICAL_COMBINING_CLASS", sizeof("PROPERTY_CANONICAL_COMBINING_CLASS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_CANONICAL_COMBINING_CLASS_name, &const_PROPERTY_CANONICAL_COMBINING_CLASS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_CANONICAL_COMBINING_CLASS_name);

	zval const_PROPERTY_DECOMPOSITION_TYPE_value;
	ZVAL_LONG(&const_PROPERTY_DECOMPOSITION_TYPE_value, UCHAR_DECOMPOSITION_TYPE);
	crex_string *const_PROPERTY_DECOMPOSITION_TYPE_name = crex_string_init_interned("PROPERTY_DECOMPOSITION_TYPE", sizeof("PROPERTY_DECOMPOSITION_TYPE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_DECOMPOSITION_TYPE_name, &const_PROPERTY_DECOMPOSITION_TYPE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_DECOMPOSITION_TYPE_name);

	zval const_PROPERTY_EAST_ASIAN_WIDTH_value;
	ZVAL_LONG(&const_PROPERTY_EAST_ASIAN_WIDTH_value, UCHAR_EAST_ASIAN_WIDTH);
	crex_string *const_PROPERTY_EAST_ASIAN_WIDTH_name = crex_string_init_interned("PROPERTY_EAST_ASIAN_WIDTH", sizeof("PROPERTY_EAST_ASIAN_WIDTH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_EAST_ASIAN_WIDTH_name, &const_PROPERTY_EAST_ASIAN_WIDTH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_EAST_ASIAN_WIDTH_name);

	zval const_PROPERTY_GENERAL_CATEGORY_value;
	ZVAL_LONG(&const_PROPERTY_GENERAL_CATEGORY_value, UCHAR_GENERAL_CATEGORY);
	crex_string *const_PROPERTY_GENERAL_CATEGORY_name = crex_string_init_interned("PROPERTY_GENERAL_CATEGORY", sizeof("PROPERTY_GENERAL_CATEGORY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_GENERAL_CATEGORY_name, &const_PROPERTY_GENERAL_CATEGORY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_GENERAL_CATEGORY_name);

	zval const_PROPERTY_JOINING_GROUP_value;
	ZVAL_LONG(&const_PROPERTY_JOINING_GROUP_value, UCHAR_JOINING_GROUP);
	crex_string *const_PROPERTY_JOINING_GROUP_name = crex_string_init_interned("PROPERTY_JOINING_GROUP", sizeof("PROPERTY_JOINING_GROUP") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_JOINING_GROUP_name, &const_PROPERTY_JOINING_GROUP_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_JOINING_GROUP_name);

	zval const_PROPERTY_JOINING_TYPE_value;
	ZVAL_LONG(&const_PROPERTY_JOINING_TYPE_value, UCHAR_JOINING_TYPE);
	crex_string *const_PROPERTY_JOINING_TYPE_name = crex_string_init_interned("PROPERTY_JOINING_TYPE", sizeof("PROPERTY_JOINING_TYPE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_JOINING_TYPE_name, &const_PROPERTY_JOINING_TYPE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_JOINING_TYPE_name);

	zval const_PROPERTY_LINE_BREAK_value;
	ZVAL_LONG(&const_PROPERTY_LINE_BREAK_value, UCHAR_LINE_BREAK);
	crex_string *const_PROPERTY_LINE_BREAK_name = crex_string_init_interned("PROPERTY_LINE_BREAK", sizeof("PROPERTY_LINE_BREAK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_LINE_BREAK_name, &const_PROPERTY_LINE_BREAK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_LINE_BREAK_name);

	zval const_PROPERTY_NUMERIC_TYPE_value;
	ZVAL_LONG(&const_PROPERTY_NUMERIC_TYPE_value, UCHAR_NUMERIC_TYPE);
	crex_string *const_PROPERTY_NUMERIC_TYPE_name = crex_string_init_interned("PROPERTY_NUMERIC_TYPE", sizeof("PROPERTY_NUMERIC_TYPE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_NUMERIC_TYPE_name, &const_PROPERTY_NUMERIC_TYPE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_NUMERIC_TYPE_name);

	zval const_PROPERTY_SCRIPT_value;
	ZVAL_LONG(&const_PROPERTY_SCRIPT_value, UCHAR_SCRIPT);
	crex_string *const_PROPERTY_SCRIPT_name = crex_string_init_interned("PROPERTY_SCRIPT", sizeof("PROPERTY_SCRIPT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_SCRIPT_name, &const_PROPERTY_SCRIPT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_SCRIPT_name);

	zval const_PROPERTY_HANGUL_SYLLABLE_TYPE_value;
	ZVAL_LONG(&const_PROPERTY_HANGUL_SYLLABLE_TYPE_value, UCHAR_HANGUL_SYLLABLE_TYPE);
	crex_string *const_PROPERTY_HANGUL_SYLLABLE_TYPE_name = crex_string_init_interned("PROPERTY_HANGUL_SYLLABLE_TYPE", sizeof("PROPERTY_HANGUL_SYLLABLE_TYPE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_HANGUL_SYLLABLE_TYPE_name, &const_PROPERTY_HANGUL_SYLLABLE_TYPE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_HANGUL_SYLLABLE_TYPE_name);

	zval const_PROPERTY_NFD_QUICK_CHECK_value;
	ZVAL_LONG(&const_PROPERTY_NFD_QUICK_CHECK_value, UCHAR_NFD_QUICK_CHECK);
	crex_string *const_PROPERTY_NFD_QUICK_CHECK_name = crex_string_init_interned("PROPERTY_NFD_QUICK_CHECK", sizeof("PROPERTY_NFD_QUICK_CHECK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_NFD_QUICK_CHECK_name, &const_PROPERTY_NFD_QUICK_CHECK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_NFD_QUICK_CHECK_name);

	zval const_PROPERTY_NFKD_QUICK_CHECK_value;
	ZVAL_LONG(&const_PROPERTY_NFKD_QUICK_CHECK_value, UCHAR_NFKD_QUICK_CHECK);
	crex_string *const_PROPERTY_NFKD_QUICK_CHECK_name = crex_string_init_interned("PROPERTY_NFKD_QUICK_CHECK", sizeof("PROPERTY_NFKD_QUICK_CHECK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_NFKD_QUICK_CHECK_name, &const_PROPERTY_NFKD_QUICK_CHECK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_NFKD_QUICK_CHECK_name);

	zval const_PROPERTY_NFC_QUICK_CHECK_value;
	ZVAL_LONG(&const_PROPERTY_NFC_QUICK_CHECK_value, UCHAR_NFC_QUICK_CHECK);
	crex_string *const_PROPERTY_NFC_QUICK_CHECK_name = crex_string_init_interned("PROPERTY_NFC_QUICK_CHECK", sizeof("PROPERTY_NFC_QUICK_CHECK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_NFC_QUICK_CHECK_name, &const_PROPERTY_NFC_QUICK_CHECK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_NFC_QUICK_CHECK_name);

	zval const_PROPERTY_NFKC_QUICK_CHECK_value;
	ZVAL_LONG(&const_PROPERTY_NFKC_QUICK_CHECK_value, UCHAR_NFKC_QUICK_CHECK);
	crex_string *const_PROPERTY_NFKC_QUICK_CHECK_name = crex_string_init_interned("PROPERTY_NFKC_QUICK_CHECK", sizeof("PROPERTY_NFKC_QUICK_CHECK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_NFKC_QUICK_CHECK_name, &const_PROPERTY_NFKC_QUICK_CHECK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_NFKC_QUICK_CHECK_name);

	zval const_PROPERTY_LEAD_CANONICAL_COMBINING_CLASS_value;
	ZVAL_LONG(&const_PROPERTY_LEAD_CANONICAL_COMBINING_CLASS_value, UCHAR_LEAD_CANONICAL_COMBINING_CLASS);
	crex_string *const_PROPERTY_LEAD_CANONICAL_COMBINING_CLASS_name = crex_string_init_interned("PROPERTY_LEAD_CANONICAL_COMBINING_CLASS", sizeof("PROPERTY_LEAD_CANONICAL_COMBINING_CLASS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_LEAD_CANONICAL_COMBINING_CLASS_name, &const_PROPERTY_LEAD_CANONICAL_COMBINING_CLASS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_LEAD_CANONICAL_COMBINING_CLASS_name);

	zval const_PROPERTY_TRAIL_CANONICAL_COMBINING_CLASS_value;
	ZVAL_LONG(&const_PROPERTY_TRAIL_CANONICAL_COMBINING_CLASS_value, UCHAR_TRAIL_CANONICAL_COMBINING_CLASS);
	crex_string *const_PROPERTY_TRAIL_CANONICAL_COMBINING_CLASS_name = crex_string_init_interned("PROPERTY_TRAIL_CANONICAL_COMBINING_CLASS", sizeof("PROPERTY_TRAIL_CANONICAL_COMBINING_CLASS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_TRAIL_CANONICAL_COMBINING_CLASS_name, &const_PROPERTY_TRAIL_CANONICAL_COMBINING_CLASS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_TRAIL_CANONICAL_COMBINING_CLASS_name);

	zval const_PROPERTY_GRAPHEME_CLUSTER_BREAK_value;
	ZVAL_LONG(&const_PROPERTY_GRAPHEME_CLUSTER_BREAK_value, UCHAR_GRAPHEME_CLUSTER_BREAK);
	crex_string *const_PROPERTY_GRAPHEME_CLUSTER_BREAK_name = crex_string_init_interned("PROPERTY_GRAPHEME_CLUSTER_BREAK", sizeof("PROPERTY_GRAPHEME_CLUSTER_BREAK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_GRAPHEME_CLUSTER_BREAK_name, &const_PROPERTY_GRAPHEME_CLUSTER_BREAK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_GRAPHEME_CLUSTER_BREAK_name);

	zval const_PROPERTY_SENTENCE_BREAK_value;
	ZVAL_LONG(&const_PROPERTY_SENTENCE_BREAK_value, UCHAR_SENTENCE_BREAK);
	crex_string *const_PROPERTY_SENTENCE_BREAK_name = crex_string_init_interned("PROPERTY_SENTENCE_BREAK", sizeof("PROPERTY_SENTENCE_BREAK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_SENTENCE_BREAK_name, &const_PROPERTY_SENTENCE_BREAK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_SENTENCE_BREAK_name);

	zval const_PROPERTY_WORD_BREAK_value;
	ZVAL_LONG(&const_PROPERTY_WORD_BREAK_value, UCHAR_WORD_BREAK);
	crex_string *const_PROPERTY_WORD_BREAK_name = crex_string_init_interned("PROPERTY_WORD_BREAK", sizeof("PROPERTY_WORD_BREAK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_WORD_BREAK_name, &const_PROPERTY_WORD_BREAK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_WORD_BREAK_name);
#if U_ICU_VERSION_MAJOR_NUM >= 52

	zval const_PROPERTY_BIDI_PAIRED_BRACKET_TYPE_value;
	ZVAL_LONG(&const_PROPERTY_BIDI_PAIRED_BRACKET_TYPE_value, UCHAR_BIDI_PAIRED_BRACKET_TYPE);
	crex_string *const_PROPERTY_BIDI_PAIRED_BRACKET_TYPE_name = crex_string_init_interned("PROPERTY_BIDI_PAIRED_BRACKET_TYPE", sizeof("PROPERTY_BIDI_PAIRED_BRACKET_TYPE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_BIDI_PAIRED_BRACKET_TYPE_name, &const_PROPERTY_BIDI_PAIRED_BRACKET_TYPE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_BIDI_PAIRED_BRACKET_TYPE_name);
#endif

	zval const_PROPERTY_INT_LIMIT_value;
	ZVAL_LONG(&const_PROPERTY_INT_LIMIT_value, UCHAR_INT_LIMIT);
	crex_string *const_PROPERTY_INT_LIMIT_name = crex_string_init_interned("PROPERTY_INT_LIMIT", sizeof("PROPERTY_INT_LIMIT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_INT_LIMIT_name, &const_PROPERTY_INT_LIMIT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_INT_LIMIT_name);

	zval const_PROPERTY_GENERAL_CATEGORY_MASK_value;
	ZVAL_LONG(&const_PROPERTY_GENERAL_CATEGORY_MASK_value, UCHAR_GENERAL_CATEGORY_MASK);
	crex_string *const_PROPERTY_GENERAL_CATEGORY_MASK_name = crex_string_init_interned("PROPERTY_GENERAL_CATEGORY_MASK", sizeof("PROPERTY_GENERAL_CATEGORY_MASK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_GENERAL_CATEGORY_MASK_name, &const_PROPERTY_GENERAL_CATEGORY_MASK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_GENERAL_CATEGORY_MASK_name);

	zval const_PROPERTY_MASK_START_value;
	ZVAL_LONG(&const_PROPERTY_MASK_START_value, UCHAR_MASK_START);
	crex_string *const_PROPERTY_MASK_START_name = crex_string_init_interned("PROPERTY_MASK_START", sizeof("PROPERTY_MASK_START") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_MASK_START_name, &const_PROPERTY_MASK_START_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_MASK_START_name);

	zval const_PROPERTY_MASK_LIMIT_value;
	ZVAL_LONG(&const_PROPERTY_MASK_LIMIT_value, UCHAR_MASK_LIMIT);
	crex_string *const_PROPERTY_MASK_LIMIT_name = crex_string_init_interned("PROPERTY_MASK_LIMIT", sizeof("PROPERTY_MASK_LIMIT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_MASK_LIMIT_name, &const_PROPERTY_MASK_LIMIT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_MASK_LIMIT_name);

	zval const_PROPERTY_NUMERIC_VALUE_value;
	ZVAL_LONG(&const_PROPERTY_NUMERIC_VALUE_value, UCHAR_NUMERIC_VALUE);
	crex_string *const_PROPERTY_NUMERIC_VALUE_name = crex_string_init_interned("PROPERTY_NUMERIC_VALUE", sizeof("PROPERTY_NUMERIC_VALUE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_NUMERIC_VALUE_name, &const_PROPERTY_NUMERIC_VALUE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_NUMERIC_VALUE_name);

	zval const_PROPERTY_DOUBLE_START_value;
	ZVAL_LONG(&const_PROPERTY_DOUBLE_START_value, UCHAR_DOUBLE_START);
	crex_string *const_PROPERTY_DOUBLE_START_name = crex_string_init_interned("PROPERTY_DOUBLE_START", sizeof("PROPERTY_DOUBLE_START") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_DOUBLE_START_name, &const_PROPERTY_DOUBLE_START_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_DOUBLE_START_name);

	zval const_PROPERTY_DOUBLE_LIMIT_value;
	ZVAL_LONG(&const_PROPERTY_DOUBLE_LIMIT_value, UCHAR_DOUBLE_LIMIT);
	crex_string *const_PROPERTY_DOUBLE_LIMIT_name = crex_string_init_interned("PROPERTY_DOUBLE_LIMIT", sizeof("PROPERTY_DOUBLE_LIMIT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_DOUBLE_LIMIT_name, &const_PROPERTY_DOUBLE_LIMIT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_DOUBLE_LIMIT_name);

	zval const_PROPERTY_AGE_value;
	ZVAL_LONG(&const_PROPERTY_AGE_value, UCHAR_AGE);
	crex_string *const_PROPERTY_AGE_name = crex_string_init_interned("PROPERTY_AGE", sizeof("PROPERTY_AGE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_AGE_name, &const_PROPERTY_AGE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_AGE_name);

	zval const_PROPERTY_STRING_START_value;
	ZVAL_LONG(&const_PROPERTY_STRING_START_value, UCHAR_STRING_START);
	crex_string *const_PROPERTY_STRING_START_name = crex_string_init_interned("PROPERTY_STRING_START", sizeof("PROPERTY_STRING_START") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_STRING_START_name, &const_PROPERTY_STRING_START_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_STRING_START_name);

	zval const_PROPERTY_BIDI_MIRRORING_GLYPH_value;
	ZVAL_LONG(&const_PROPERTY_BIDI_MIRRORING_GLYPH_value, UCHAR_BIDI_MIRRORING_GLYPH);
	crex_string *const_PROPERTY_BIDI_MIRRORING_GLYPH_name = crex_string_init_interned("PROPERTY_BIDI_MIRRORING_GLYPH", sizeof("PROPERTY_BIDI_MIRRORING_GLYPH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_BIDI_MIRRORING_GLYPH_name, &const_PROPERTY_BIDI_MIRRORING_GLYPH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_BIDI_MIRRORING_GLYPH_name);

	zval const_PROPERTY_CASE_FOLDING_value;
	ZVAL_LONG(&const_PROPERTY_CASE_FOLDING_value, UCHAR_CASE_FOLDING);
	crex_string *const_PROPERTY_CASE_FOLDING_name = crex_string_init_interned("PROPERTY_CASE_FOLDING", sizeof("PROPERTY_CASE_FOLDING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_CASE_FOLDING_name, &const_PROPERTY_CASE_FOLDING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_CASE_FOLDING_name);

	zval const_PROPERTY_ISO_COMMENT_value;
	ZVAL_LONG(&const_PROPERTY_ISO_COMMENT_value, UCHAR_ISO_COMMENT);
	crex_string *const_PROPERTY_ISO_COMMENT_name = crex_string_init_interned("PROPERTY_ISO_COMMENT", sizeof("PROPERTY_ISO_COMMENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_ISO_COMMENT_name, &const_PROPERTY_ISO_COMMENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_ISO_COMMENT_name);

	zval const_PROPERTY_LOWERCASE_MAPPING_value;
	ZVAL_LONG(&const_PROPERTY_LOWERCASE_MAPPING_value, UCHAR_LOWERCASE_MAPPING);
	crex_string *const_PROPERTY_LOWERCASE_MAPPING_name = crex_string_init_interned("PROPERTY_LOWERCASE_MAPPING", sizeof("PROPERTY_LOWERCASE_MAPPING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_LOWERCASE_MAPPING_name, &const_PROPERTY_LOWERCASE_MAPPING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_LOWERCASE_MAPPING_name);

	zval const_PROPERTY_NAME_value;
	ZVAL_LONG(&const_PROPERTY_NAME_value, UCHAR_NAME);
	crex_string *const_PROPERTY_NAME_name = crex_string_init_interned("PROPERTY_NAME", sizeof("PROPERTY_NAME") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_NAME_name, &const_PROPERTY_NAME_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_NAME_name);

	zval const_PROPERTY_SIMPLE_CASE_FOLDING_value;
	ZVAL_LONG(&const_PROPERTY_SIMPLE_CASE_FOLDING_value, UCHAR_SIMPLE_CASE_FOLDING);
	crex_string *const_PROPERTY_SIMPLE_CASE_FOLDING_name = crex_string_init_interned("PROPERTY_SIMPLE_CASE_FOLDING", sizeof("PROPERTY_SIMPLE_CASE_FOLDING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_SIMPLE_CASE_FOLDING_name, &const_PROPERTY_SIMPLE_CASE_FOLDING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_SIMPLE_CASE_FOLDING_name);

	zval const_PROPERTY_SIMPLE_LOWERCASE_MAPPING_value;
	ZVAL_LONG(&const_PROPERTY_SIMPLE_LOWERCASE_MAPPING_value, UCHAR_SIMPLE_LOWERCASE_MAPPING);
	crex_string *const_PROPERTY_SIMPLE_LOWERCASE_MAPPING_name = crex_string_init_interned("PROPERTY_SIMPLE_LOWERCASE_MAPPING", sizeof("PROPERTY_SIMPLE_LOWERCASE_MAPPING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_SIMPLE_LOWERCASE_MAPPING_name, &const_PROPERTY_SIMPLE_LOWERCASE_MAPPING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_SIMPLE_LOWERCASE_MAPPING_name);

	zval const_PROPERTY_SIMPLE_TITLECASE_MAPPING_value;
	ZVAL_LONG(&const_PROPERTY_SIMPLE_TITLECASE_MAPPING_value, UCHAR_SIMPLE_TITLECASE_MAPPING);
	crex_string *const_PROPERTY_SIMPLE_TITLECASE_MAPPING_name = crex_string_init_interned("PROPERTY_SIMPLE_TITLECASE_MAPPING", sizeof("PROPERTY_SIMPLE_TITLECASE_MAPPING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_SIMPLE_TITLECASE_MAPPING_name, &const_PROPERTY_SIMPLE_TITLECASE_MAPPING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_SIMPLE_TITLECASE_MAPPING_name);

	zval const_PROPERTY_SIMPLE_UPPERCASE_MAPPING_value;
	ZVAL_LONG(&const_PROPERTY_SIMPLE_UPPERCASE_MAPPING_value, UCHAR_SIMPLE_UPPERCASE_MAPPING);
	crex_string *const_PROPERTY_SIMPLE_UPPERCASE_MAPPING_name = crex_string_init_interned("PROPERTY_SIMPLE_UPPERCASE_MAPPING", sizeof("PROPERTY_SIMPLE_UPPERCASE_MAPPING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_SIMPLE_UPPERCASE_MAPPING_name, &const_PROPERTY_SIMPLE_UPPERCASE_MAPPING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_SIMPLE_UPPERCASE_MAPPING_name);

	zval const_PROPERTY_TITLECASE_MAPPING_value;
	ZVAL_LONG(&const_PROPERTY_TITLECASE_MAPPING_value, UCHAR_TITLECASE_MAPPING);
	crex_string *const_PROPERTY_TITLECASE_MAPPING_name = crex_string_init_interned("PROPERTY_TITLECASE_MAPPING", sizeof("PROPERTY_TITLECASE_MAPPING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_TITLECASE_MAPPING_name, &const_PROPERTY_TITLECASE_MAPPING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_TITLECASE_MAPPING_name);

	zval const_PROPERTY_UNICODE_1_NAME_value;
	ZVAL_LONG(&const_PROPERTY_UNICODE_1_NAME_value, UCHAR_UNICODE_1_NAME);
	crex_string *const_PROPERTY_UNICODE_1_NAME_name = crex_string_init_interned("PROPERTY_UNICODE_1_NAME", sizeof("PROPERTY_UNICODE_1_NAME") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_UNICODE_1_NAME_name, &const_PROPERTY_UNICODE_1_NAME_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_UNICODE_1_NAME_name);

	zval const_PROPERTY_UPPERCASE_MAPPING_value;
	ZVAL_LONG(&const_PROPERTY_UPPERCASE_MAPPING_value, UCHAR_UPPERCASE_MAPPING);
	crex_string *const_PROPERTY_UPPERCASE_MAPPING_name = crex_string_init_interned("PROPERTY_UPPERCASE_MAPPING", sizeof("PROPERTY_UPPERCASE_MAPPING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_UPPERCASE_MAPPING_name, &const_PROPERTY_UPPERCASE_MAPPING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_UPPERCASE_MAPPING_name);
#if U_ICU_VERSION_MAJOR_NUM >= 52

	zval const_PROPERTY_BIDI_PAIRED_BRACKET_value;
	ZVAL_LONG(&const_PROPERTY_BIDI_PAIRED_BRACKET_value, UCHAR_BIDI_PAIRED_BRACKET);
	crex_string *const_PROPERTY_BIDI_PAIRED_BRACKET_name = crex_string_init_interned("PROPERTY_BIDI_PAIRED_BRACKET", sizeof("PROPERTY_BIDI_PAIRED_BRACKET") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_BIDI_PAIRED_BRACKET_name, &const_PROPERTY_BIDI_PAIRED_BRACKET_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_BIDI_PAIRED_BRACKET_name);
#endif

	zval const_PROPERTY_STRING_LIMIT_value;
	ZVAL_LONG(&const_PROPERTY_STRING_LIMIT_value, UCHAR_STRING_LIMIT);
	crex_string *const_PROPERTY_STRING_LIMIT_name = crex_string_init_interned("PROPERTY_STRING_LIMIT", sizeof("PROPERTY_STRING_LIMIT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_STRING_LIMIT_name, &const_PROPERTY_STRING_LIMIT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_STRING_LIMIT_name);

	zval const_PROPERTY_SCRIPT_EXTENSIONS_value;
	ZVAL_LONG(&const_PROPERTY_SCRIPT_EXTENSIONS_value, UCHAR_SCRIPT_EXTENSIONS);
	crex_string *const_PROPERTY_SCRIPT_EXTENSIONS_name = crex_string_init_interned("PROPERTY_SCRIPT_EXTENSIONS", sizeof("PROPERTY_SCRIPT_EXTENSIONS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_SCRIPT_EXTENSIONS_name, &const_PROPERTY_SCRIPT_EXTENSIONS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_SCRIPT_EXTENSIONS_name);

	zval const_PROPERTY_OTHER_PROPERTY_START_value;
	ZVAL_LONG(&const_PROPERTY_OTHER_PROPERTY_START_value, UCHAR_OTHER_PROPERTY_START);
	crex_string *const_PROPERTY_OTHER_PROPERTY_START_name = crex_string_init_interned("PROPERTY_OTHER_PROPERTY_START", sizeof("PROPERTY_OTHER_PROPERTY_START") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_OTHER_PROPERTY_START_name, &const_PROPERTY_OTHER_PROPERTY_START_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_OTHER_PROPERTY_START_name);

	zval const_PROPERTY_OTHER_PROPERTY_LIMIT_value;
	ZVAL_LONG(&const_PROPERTY_OTHER_PROPERTY_LIMIT_value, UCHAR_OTHER_PROPERTY_LIMIT);
	crex_string *const_PROPERTY_OTHER_PROPERTY_LIMIT_name = crex_string_init_interned("PROPERTY_OTHER_PROPERTY_LIMIT", sizeof("PROPERTY_OTHER_PROPERTY_LIMIT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_OTHER_PROPERTY_LIMIT_name, &const_PROPERTY_OTHER_PROPERTY_LIMIT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_OTHER_PROPERTY_LIMIT_name);

	zval const_PROPERTY_INVALID_CODE_value;
	ZVAL_LONG(&const_PROPERTY_INVALID_CODE_value, UCHAR_INVALID_CODE);
	crex_string *const_PROPERTY_INVALID_CODE_name = crex_string_init_interned("PROPERTY_INVALID_CODE", sizeof("PROPERTY_INVALID_CODE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_INVALID_CODE_name, &const_PROPERTY_INVALID_CODE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_INVALID_CODE_name);

	zval const_CHAR_CATEGORY_UNASSIGNED_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_UNASSIGNED_value, U_UNASSIGNED);
	crex_string *const_CHAR_CATEGORY_UNASSIGNED_name = crex_string_init_interned("CHAR_CATEGORY_UNASSIGNED", sizeof("CHAR_CATEGORY_UNASSIGNED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_UNASSIGNED_name, &const_CHAR_CATEGORY_UNASSIGNED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_UNASSIGNED_name);

	zval const_CHAR_CATEGORY_GENERAL_OTHER_TYPES_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_GENERAL_OTHER_TYPES_value, U_GENERAL_OTHER_TYPES);
	crex_string *const_CHAR_CATEGORY_GENERAL_OTHER_TYPES_name = crex_string_init_interned("CHAR_CATEGORY_GENERAL_OTHER_TYPES", sizeof("CHAR_CATEGORY_GENERAL_OTHER_TYPES") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_GENERAL_OTHER_TYPES_name, &const_CHAR_CATEGORY_GENERAL_OTHER_TYPES_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_GENERAL_OTHER_TYPES_name);

	zval const_CHAR_CATEGORY_UPPERCASE_LETTER_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_UPPERCASE_LETTER_value, U_UPPERCASE_LETTER);
	crex_string *const_CHAR_CATEGORY_UPPERCASE_LETTER_name = crex_string_init_interned("CHAR_CATEGORY_UPPERCASE_LETTER", sizeof("CHAR_CATEGORY_UPPERCASE_LETTER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_UPPERCASE_LETTER_name, &const_CHAR_CATEGORY_UPPERCASE_LETTER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_UPPERCASE_LETTER_name);

	zval const_CHAR_CATEGORY_LOWERCASE_LETTER_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_LOWERCASE_LETTER_value, U_LOWERCASE_LETTER);
	crex_string *const_CHAR_CATEGORY_LOWERCASE_LETTER_name = crex_string_init_interned("CHAR_CATEGORY_LOWERCASE_LETTER", sizeof("CHAR_CATEGORY_LOWERCASE_LETTER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_LOWERCASE_LETTER_name, &const_CHAR_CATEGORY_LOWERCASE_LETTER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_LOWERCASE_LETTER_name);

	zval const_CHAR_CATEGORY_TITLECASE_LETTER_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_TITLECASE_LETTER_value, U_TITLECASE_LETTER);
	crex_string *const_CHAR_CATEGORY_TITLECASE_LETTER_name = crex_string_init_interned("CHAR_CATEGORY_TITLECASE_LETTER", sizeof("CHAR_CATEGORY_TITLECASE_LETTER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_TITLECASE_LETTER_name, &const_CHAR_CATEGORY_TITLECASE_LETTER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_TITLECASE_LETTER_name);

	zval const_CHAR_CATEGORY_MODIFIER_LETTER_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_MODIFIER_LETTER_value, U_MODIFIER_LETTER);
	crex_string *const_CHAR_CATEGORY_MODIFIER_LETTER_name = crex_string_init_interned("CHAR_CATEGORY_MODIFIER_LETTER", sizeof("CHAR_CATEGORY_MODIFIER_LETTER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_MODIFIER_LETTER_name, &const_CHAR_CATEGORY_MODIFIER_LETTER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_MODIFIER_LETTER_name);

	zval const_CHAR_CATEGORY_OTHER_LETTER_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_OTHER_LETTER_value, U_OTHER_LETTER);
	crex_string *const_CHAR_CATEGORY_OTHER_LETTER_name = crex_string_init_interned("CHAR_CATEGORY_OTHER_LETTER", sizeof("CHAR_CATEGORY_OTHER_LETTER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_OTHER_LETTER_name, &const_CHAR_CATEGORY_OTHER_LETTER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_OTHER_LETTER_name);

	zval const_CHAR_CATEGORY_NON_SPACING_MARK_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_NON_SPACING_MARK_value, U_NON_SPACING_MARK);
	crex_string *const_CHAR_CATEGORY_NON_SPACING_MARK_name = crex_string_init_interned("CHAR_CATEGORY_NON_SPACING_MARK", sizeof("CHAR_CATEGORY_NON_SPACING_MARK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_NON_SPACING_MARK_name, &const_CHAR_CATEGORY_NON_SPACING_MARK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_NON_SPACING_MARK_name);

	zval const_CHAR_CATEGORY_ENCLOSING_MARK_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_ENCLOSING_MARK_value, U_ENCLOSING_MARK);
	crex_string *const_CHAR_CATEGORY_ENCLOSING_MARK_name = crex_string_init_interned("CHAR_CATEGORY_ENCLOSING_MARK", sizeof("CHAR_CATEGORY_ENCLOSING_MARK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_ENCLOSING_MARK_name, &const_CHAR_CATEGORY_ENCLOSING_MARK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_ENCLOSING_MARK_name);

	zval const_CHAR_CATEGORY_COMBINING_SPACING_MARK_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_COMBINING_SPACING_MARK_value, U_COMBINING_SPACING_MARK);
	crex_string *const_CHAR_CATEGORY_COMBINING_SPACING_MARK_name = crex_string_init_interned("CHAR_CATEGORY_COMBINING_SPACING_MARK", sizeof("CHAR_CATEGORY_COMBINING_SPACING_MARK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_COMBINING_SPACING_MARK_name, &const_CHAR_CATEGORY_COMBINING_SPACING_MARK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_COMBINING_SPACING_MARK_name);

	zval const_CHAR_CATEGORY_DECIMAL_DIGIT_NUMBER_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_DECIMAL_DIGIT_NUMBER_value, U_DECIMAL_DIGIT_NUMBER);
	crex_string *const_CHAR_CATEGORY_DECIMAL_DIGIT_NUMBER_name = crex_string_init_interned("CHAR_CATEGORY_DECIMAL_DIGIT_NUMBER", sizeof("CHAR_CATEGORY_DECIMAL_DIGIT_NUMBER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_DECIMAL_DIGIT_NUMBER_name, &const_CHAR_CATEGORY_DECIMAL_DIGIT_NUMBER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_DECIMAL_DIGIT_NUMBER_name);

	zval const_CHAR_CATEGORY_LETTER_NUMBER_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_LETTER_NUMBER_value, U_LETTER_NUMBER);
	crex_string *const_CHAR_CATEGORY_LETTER_NUMBER_name = crex_string_init_interned("CHAR_CATEGORY_LETTER_NUMBER", sizeof("CHAR_CATEGORY_LETTER_NUMBER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_LETTER_NUMBER_name, &const_CHAR_CATEGORY_LETTER_NUMBER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_LETTER_NUMBER_name);

	zval const_CHAR_CATEGORY_OTHER_NUMBER_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_OTHER_NUMBER_value, U_OTHER_NUMBER);
	crex_string *const_CHAR_CATEGORY_OTHER_NUMBER_name = crex_string_init_interned("CHAR_CATEGORY_OTHER_NUMBER", sizeof("CHAR_CATEGORY_OTHER_NUMBER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_OTHER_NUMBER_name, &const_CHAR_CATEGORY_OTHER_NUMBER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_OTHER_NUMBER_name);

	zval const_CHAR_CATEGORY_SPACE_SEPARATOR_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_SPACE_SEPARATOR_value, U_SPACE_SEPARATOR);
	crex_string *const_CHAR_CATEGORY_SPACE_SEPARATOR_name = crex_string_init_interned("CHAR_CATEGORY_SPACE_SEPARATOR", sizeof("CHAR_CATEGORY_SPACE_SEPARATOR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_SPACE_SEPARATOR_name, &const_CHAR_CATEGORY_SPACE_SEPARATOR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_SPACE_SEPARATOR_name);

	zval const_CHAR_CATEGORY_LINE_SEPARATOR_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_LINE_SEPARATOR_value, U_LINE_SEPARATOR);
	crex_string *const_CHAR_CATEGORY_LINE_SEPARATOR_name = crex_string_init_interned("CHAR_CATEGORY_LINE_SEPARATOR", sizeof("CHAR_CATEGORY_LINE_SEPARATOR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_LINE_SEPARATOR_name, &const_CHAR_CATEGORY_LINE_SEPARATOR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_LINE_SEPARATOR_name);

	zval const_CHAR_CATEGORY_PARAGRAPH_SEPARATOR_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_PARAGRAPH_SEPARATOR_value, U_PARAGRAPH_SEPARATOR);
	crex_string *const_CHAR_CATEGORY_PARAGRAPH_SEPARATOR_name = crex_string_init_interned("CHAR_CATEGORY_PARAGRAPH_SEPARATOR", sizeof("CHAR_CATEGORY_PARAGRAPH_SEPARATOR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_PARAGRAPH_SEPARATOR_name, &const_CHAR_CATEGORY_PARAGRAPH_SEPARATOR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_PARAGRAPH_SEPARATOR_name);

	zval const_CHAR_CATEGORY_CONTROL_CHAR_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_CONTROL_CHAR_value, U_CONTROL_CHAR);
	crex_string *const_CHAR_CATEGORY_CONTROL_CHAR_name = crex_string_init_interned("CHAR_CATEGORY_CONTROL_CHAR", sizeof("CHAR_CATEGORY_CONTROL_CHAR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_CONTROL_CHAR_name, &const_CHAR_CATEGORY_CONTROL_CHAR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_CONTROL_CHAR_name);

	zval const_CHAR_CATEGORY_FORMAT_CHAR_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_FORMAT_CHAR_value, U_FORMAT_CHAR);
	crex_string *const_CHAR_CATEGORY_FORMAT_CHAR_name = crex_string_init_interned("CHAR_CATEGORY_FORMAT_CHAR", sizeof("CHAR_CATEGORY_FORMAT_CHAR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_FORMAT_CHAR_name, &const_CHAR_CATEGORY_FORMAT_CHAR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_FORMAT_CHAR_name);

	zval const_CHAR_CATEGORY_PRIVATE_USE_CHAR_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_PRIVATE_USE_CHAR_value, U_PRIVATE_USE_CHAR);
	crex_string *const_CHAR_CATEGORY_PRIVATE_USE_CHAR_name = crex_string_init_interned("CHAR_CATEGORY_PRIVATE_USE_CHAR", sizeof("CHAR_CATEGORY_PRIVATE_USE_CHAR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_PRIVATE_USE_CHAR_name, &const_CHAR_CATEGORY_PRIVATE_USE_CHAR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_PRIVATE_USE_CHAR_name);

	zval const_CHAR_CATEGORY_SURROGATE_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_SURROGATE_value, U_SURROGATE);
	crex_string *const_CHAR_CATEGORY_SURROGATE_name = crex_string_init_interned("CHAR_CATEGORY_SURROGATE", sizeof("CHAR_CATEGORY_SURROGATE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_SURROGATE_name, &const_CHAR_CATEGORY_SURROGATE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_SURROGATE_name);

	zval const_CHAR_CATEGORY_DASH_PUNCTUATION_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_DASH_PUNCTUATION_value, U_DASH_PUNCTUATION);
	crex_string *const_CHAR_CATEGORY_DASH_PUNCTUATION_name = crex_string_init_interned("CHAR_CATEGORY_DASH_PUNCTUATION", sizeof("CHAR_CATEGORY_DASH_PUNCTUATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_DASH_PUNCTUATION_name, &const_CHAR_CATEGORY_DASH_PUNCTUATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_DASH_PUNCTUATION_name);

	zval const_CHAR_CATEGORY_START_PUNCTUATION_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_START_PUNCTUATION_value, U_START_PUNCTUATION);
	crex_string *const_CHAR_CATEGORY_START_PUNCTUATION_name = crex_string_init_interned("CHAR_CATEGORY_START_PUNCTUATION", sizeof("CHAR_CATEGORY_START_PUNCTUATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_START_PUNCTUATION_name, &const_CHAR_CATEGORY_START_PUNCTUATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_START_PUNCTUATION_name);

	zval const_CHAR_CATEGORY_END_PUNCTUATION_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_END_PUNCTUATION_value, U_END_PUNCTUATION);
	crex_string *const_CHAR_CATEGORY_END_PUNCTUATION_name = crex_string_init_interned("CHAR_CATEGORY_END_PUNCTUATION", sizeof("CHAR_CATEGORY_END_PUNCTUATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_END_PUNCTUATION_name, &const_CHAR_CATEGORY_END_PUNCTUATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_END_PUNCTUATION_name);

	zval const_CHAR_CATEGORY_CONNECTOR_PUNCTUATION_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_CONNECTOR_PUNCTUATION_value, U_CONNECTOR_PUNCTUATION);
	crex_string *const_CHAR_CATEGORY_CONNECTOR_PUNCTUATION_name = crex_string_init_interned("CHAR_CATEGORY_CONNECTOR_PUNCTUATION", sizeof("CHAR_CATEGORY_CONNECTOR_PUNCTUATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_CONNECTOR_PUNCTUATION_name, &const_CHAR_CATEGORY_CONNECTOR_PUNCTUATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_CONNECTOR_PUNCTUATION_name);

	zval const_CHAR_CATEGORY_OTHER_PUNCTUATION_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_OTHER_PUNCTUATION_value, U_OTHER_PUNCTUATION);
	crex_string *const_CHAR_CATEGORY_OTHER_PUNCTUATION_name = crex_string_init_interned("CHAR_CATEGORY_OTHER_PUNCTUATION", sizeof("CHAR_CATEGORY_OTHER_PUNCTUATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_OTHER_PUNCTUATION_name, &const_CHAR_CATEGORY_OTHER_PUNCTUATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_OTHER_PUNCTUATION_name);

	zval const_CHAR_CATEGORY_MATH_SYMBOL_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_MATH_SYMBOL_value, U_MATH_SYMBOL);
	crex_string *const_CHAR_CATEGORY_MATH_SYMBOL_name = crex_string_init_interned("CHAR_CATEGORY_MATH_SYMBOL", sizeof("CHAR_CATEGORY_MATH_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_MATH_SYMBOL_name, &const_CHAR_CATEGORY_MATH_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_MATH_SYMBOL_name);

	zval const_CHAR_CATEGORY_CURRENCY_SYMBOL_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_CURRENCY_SYMBOL_value, U_CURRENCY_SYMBOL);
	crex_string *const_CHAR_CATEGORY_CURRENCY_SYMBOL_name = crex_string_init_interned("CHAR_CATEGORY_CURRENCY_SYMBOL", sizeof("CHAR_CATEGORY_CURRENCY_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_CURRENCY_SYMBOL_name, &const_CHAR_CATEGORY_CURRENCY_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_CURRENCY_SYMBOL_name);

	zval const_CHAR_CATEGORY_MODIFIER_SYMBOL_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_MODIFIER_SYMBOL_value, U_MODIFIER_SYMBOL);
	crex_string *const_CHAR_CATEGORY_MODIFIER_SYMBOL_name = crex_string_init_interned("CHAR_CATEGORY_MODIFIER_SYMBOL", sizeof("CHAR_CATEGORY_MODIFIER_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_MODIFIER_SYMBOL_name, &const_CHAR_CATEGORY_MODIFIER_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_MODIFIER_SYMBOL_name);

	zval const_CHAR_CATEGORY_OTHER_SYMBOL_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_OTHER_SYMBOL_value, U_OTHER_SYMBOL);
	crex_string *const_CHAR_CATEGORY_OTHER_SYMBOL_name = crex_string_init_interned("CHAR_CATEGORY_OTHER_SYMBOL", sizeof("CHAR_CATEGORY_OTHER_SYMBOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_OTHER_SYMBOL_name, &const_CHAR_CATEGORY_OTHER_SYMBOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_OTHER_SYMBOL_name);

	zval const_CHAR_CATEGORY_INITIAL_PUNCTUATION_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_INITIAL_PUNCTUATION_value, U_INITIAL_PUNCTUATION);
	crex_string *const_CHAR_CATEGORY_INITIAL_PUNCTUATION_name = crex_string_init_interned("CHAR_CATEGORY_INITIAL_PUNCTUATION", sizeof("CHAR_CATEGORY_INITIAL_PUNCTUATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_INITIAL_PUNCTUATION_name, &const_CHAR_CATEGORY_INITIAL_PUNCTUATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_INITIAL_PUNCTUATION_name);

	zval const_CHAR_CATEGORY_FINAL_PUNCTUATION_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_FINAL_PUNCTUATION_value, U_FINAL_PUNCTUATION);
	crex_string *const_CHAR_CATEGORY_FINAL_PUNCTUATION_name = crex_string_init_interned("CHAR_CATEGORY_FINAL_PUNCTUATION", sizeof("CHAR_CATEGORY_FINAL_PUNCTUATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_FINAL_PUNCTUATION_name, &const_CHAR_CATEGORY_FINAL_PUNCTUATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_FINAL_PUNCTUATION_name);

	zval const_CHAR_CATEGORY_CHAR_CATEGORY_COUNT_value;
	ZVAL_LONG(&const_CHAR_CATEGORY_CHAR_CATEGORY_COUNT_value, U_CHAR_CATEGORY_COUNT);
	crex_string *const_CHAR_CATEGORY_CHAR_CATEGORY_COUNT_name = crex_string_init_interned("CHAR_CATEGORY_CHAR_CATEGORY_COUNT", sizeof("CHAR_CATEGORY_CHAR_CATEGORY_COUNT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_CATEGORY_CHAR_CATEGORY_COUNT_name, &const_CHAR_CATEGORY_CHAR_CATEGORY_COUNT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_CATEGORY_CHAR_CATEGORY_COUNT_name);

	zval const_CHAR_DIRECTION_LEFT_TO_RIGHT_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_LEFT_TO_RIGHT_value, U_LEFT_TO_RIGHT);
	crex_string *const_CHAR_DIRECTION_LEFT_TO_RIGHT_name = crex_string_init_interned("CHAR_DIRECTION_LEFT_TO_RIGHT", sizeof("CHAR_DIRECTION_LEFT_TO_RIGHT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_LEFT_TO_RIGHT_name, &const_CHAR_DIRECTION_LEFT_TO_RIGHT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_LEFT_TO_RIGHT_name);

	zval const_CHAR_DIRECTION_RIGHT_TO_LEFT_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_RIGHT_TO_LEFT_value, U_RIGHT_TO_LEFT);
	crex_string *const_CHAR_DIRECTION_RIGHT_TO_LEFT_name = crex_string_init_interned("CHAR_DIRECTION_RIGHT_TO_LEFT", sizeof("CHAR_DIRECTION_RIGHT_TO_LEFT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_RIGHT_TO_LEFT_name, &const_CHAR_DIRECTION_RIGHT_TO_LEFT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_RIGHT_TO_LEFT_name);

	zval const_CHAR_DIRECTION_EUROPEAN_NUMBER_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_EUROPEAN_NUMBER_value, U_EUROPEAN_NUMBER);
	crex_string *const_CHAR_DIRECTION_EUROPEAN_NUMBER_name = crex_string_init_interned("CHAR_DIRECTION_EUROPEAN_NUMBER", sizeof("CHAR_DIRECTION_EUROPEAN_NUMBER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_EUROPEAN_NUMBER_name, &const_CHAR_DIRECTION_EUROPEAN_NUMBER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_EUROPEAN_NUMBER_name);

	zval const_CHAR_DIRECTION_EUROPEAN_NUMBER_SEPARATOR_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_EUROPEAN_NUMBER_SEPARATOR_value, U_EUROPEAN_NUMBER_SEPARATOR);
	crex_string *const_CHAR_DIRECTION_EUROPEAN_NUMBER_SEPARATOR_name = crex_string_init_interned("CHAR_DIRECTION_EUROPEAN_NUMBER_SEPARATOR", sizeof("CHAR_DIRECTION_EUROPEAN_NUMBER_SEPARATOR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_EUROPEAN_NUMBER_SEPARATOR_name, &const_CHAR_DIRECTION_EUROPEAN_NUMBER_SEPARATOR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_EUROPEAN_NUMBER_SEPARATOR_name);

	zval const_CHAR_DIRECTION_EUROPEAN_NUMBER_TERMINATOR_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_EUROPEAN_NUMBER_TERMINATOR_value, U_EUROPEAN_NUMBER_TERMINATOR);
	crex_string *const_CHAR_DIRECTION_EUROPEAN_NUMBER_TERMINATOR_name = crex_string_init_interned("CHAR_DIRECTION_EUROPEAN_NUMBER_TERMINATOR", sizeof("CHAR_DIRECTION_EUROPEAN_NUMBER_TERMINATOR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_EUROPEAN_NUMBER_TERMINATOR_name, &const_CHAR_DIRECTION_EUROPEAN_NUMBER_TERMINATOR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_EUROPEAN_NUMBER_TERMINATOR_name);

	zval const_CHAR_DIRECTION_ARABIC_NUMBER_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_ARABIC_NUMBER_value, U_ARABIC_NUMBER);
	crex_string *const_CHAR_DIRECTION_ARABIC_NUMBER_name = crex_string_init_interned("CHAR_DIRECTION_ARABIC_NUMBER", sizeof("CHAR_DIRECTION_ARABIC_NUMBER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_ARABIC_NUMBER_name, &const_CHAR_DIRECTION_ARABIC_NUMBER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_ARABIC_NUMBER_name);

	zval const_CHAR_DIRECTION_COMMON_NUMBER_SEPARATOR_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_COMMON_NUMBER_SEPARATOR_value, U_COMMON_NUMBER_SEPARATOR);
	crex_string *const_CHAR_DIRECTION_COMMON_NUMBER_SEPARATOR_name = crex_string_init_interned("CHAR_DIRECTION_COMMON_NUMBER_SEPARATOR", sizeof("CHAR_DIRECTION_COMMON_NUMBER_SEPARATOR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_COMMON_NUMBER_SEPARATOR_name, &const_CHAR_DIRECTION_COMMON_NUMBER_SEPARATOR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_COMMON_NUMBER_SEPARATOR_name);

	zval const_CHAR_DIRECTION_BLOCK_SEPARATOR_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_BLOCK_SEPARATOR_value, U_BLOCK_SEPARATOR);
	crex_string *const_CHAR_DIRECTION_BLOCK_SEPARATOR_name = crex_string_init_interned("CHAR_DIRECTION_BLOCK_SEPARATOR", sizeof("CHAR_DIRECTION_BLOCK_SEPARATOR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_BLOCK_SEPARATOR_name, &const_CHAR_DIRECTION_BLOCK_SEPARATOR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_BLOCK_SEPARATOR_name);

	zval const_CHAR_DIRECTION_SEGMENT_SEPARATOR_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_SEGMENT_SEPARATOR_value, U_SEGMENT_SEPARATOR);
	crex_string *const_CHAR_DIRECTION_SEGMENT_SEPARATOR_name = crex_string_init_interned("CHAR_DIRECTION_SEGMENT_SEPARATOR", sizeof("CHAR_DIRECTION_SEGMENT_SEPARATOR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_SEGMENT_SEPARATOR_name, &const_CHAR_DIRECTION_SEGMENT_SEPARATOR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_SEGMENT_SEPARATOR_name);

	zval const_CHAR_DIRECTION_WHITE_SPACE_NEUTRAL_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_WHITE_SPACE_NEUTRAL_value, U_WHITE_SPACE_NEUTRAL);
	crex_string *const_CHAR_DIRECTION_WHITE_SPACE_NEUTRAL_name = crex_string_init_interned("CHAR_DIRECTION_WHITE_SPACE_NEUTRAL", sizeof("CHAR_DIRECTION_WHITE_SPACE_NEUTRAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_WHITE_SPACE_NEUTRAL_name, &const_CHAR_DIRECTION_WHITE_SPACE_NEUTRAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_WHITE_SPACE_NEUTRAL_name);

	zval const_CHAR_DIRECTION_OTHER_NEUTRAL_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_OTHER_NEUTRAL_value, U_OTHER_NEUTRAL);
	crex_string *const_CHAR_DIRECTION_OTHER_NEUTRAL_name = crex_string_init_interned("CHAR_DIRECTION_OTHER_NEUTRAL", sizeof("CHAR_DIRECTION_OTHER_NEUTRAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_OTHER_NEUTRAL_name, &const_CHAR_DIRECTION_OTHER_NEUTRAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_OTHER_NEUTRAL_name);

	zval const_CHAR_DIRECTION_LEFT_TO_RIGHT_EMBEDDING_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_LEFT_TO_RIGHT_EMBEDDING_value, U_LEFT_TO_RIGHT_EMBEDDING);
	crex_string *const_CHAR_DIRECTION_LEFT_TO_RIGHT_EMBEDDING_name = crex_string_init_interned("CHAR_DIRECTION_LEFT_TO_RIGHT_EMBEDDING", sizeof("CHAR_DIRECTION_LEFT_TO_RIGHT_EMBEDDING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_LEFT_TO_RIGHT_EMBEDDING_name, &const_CHAR_DIRECTION_LEFT_TO_RIGHT_EMBEDDING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_LEFT_TO_RIGHT_EMBEDDING_name);

	zval const_CHAR_DIRECTION_LEFT_TO_RIGHT_OVERRIDE_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_LEFT_TO_RIGHT_OVERRIDE_value, U_LEFT_TO_RIGHT_OVERRIDE);
	crex_string *const_CHAR_DIRECTION_LEFT_TO_RIGHT_OVERRIDE_name = crex_string_init_interned("CHAR_DIRECTION_LEFT_TO_RIGHT_OVERRIDE", sizeof("CHAR_DIRECTION_LEFT_TO_RIGHT_OVERRIDE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_LEFT_TO_RIGHT_OVERRIDE_name, &const_CHAR_DIRECTION_LEFT_TO_RIGHT_OVERRIDE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_LEFT_TO_RIGHT_OVERRIDE_name);

	zval const_CHAR_DIRECTION_RIGHT_TO_LEFT_ARABIC_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_RIGHT_TO_LEFT_ARABIC_value, U_RIGHT_TO_LEFT_ARABIC);
	crex_string *const_CHAR_DIRECTION_RIGHT_TO_LEFT_ARABIC_name = crex_string_init_interned("CHAR_DIRECTION_RIGHT_TO_LEFT_ARABIC", sizeof("CHAR_DIRECTION_RIGHT_TO_LEFT_ARABIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_RIGHT_TO_LEFT_ARABIC_name, &const_CHAR_DIRECTION_RIGHT_TO_LEFT_ARABIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_RIGHT_TO_LEFT_ARABIC_name);

	zval const_CHAR_DIRECTION_RIGHT_TO_LEFT_EMBEDDING_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_RIGHT_TO_LEFT_EMBEDDING_value, U_RIGHT_TO_LEFT_EMBEDDING);
	crex_string *const_CHAR_DIRECTION_RIGHT_TO_LEFT_EMBEDDING_name = crex_string_init_interned("CHAR_DIRECTION_RIGHT_TO_LEFT_EMBEDDING", sizeof("CHAR_DIRECTION_RIGHT_TO_LEFT_EMBEDDING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_RIGHT_TO_LEFT_EMBEDDING_name, &const_CHAR_DIRECTION_RIGHT_TO_LEFT_EMBEDDING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_RIGHT_TO_LEFT_EMBEDDING_name);

	zval const_CHAR_DIRECTION_RIGHT_TO_LEFT_OVERRIDE_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_RIGHT_TO_LEFT_OVERRIDE_value, U_RIGHT_TO_LEFT_OVERRIDE);
	crex_string *const_CHAR_DIRECTION_RIGHT_TO_LEFT_OVERRIDE_name = crex_string_init_interned("CHAR_DIRECTION_RIGHT_TO_LEFT_OVERRIDE", sizeof("CHAR_DIRECTION_RIGHT_TO_LEFT_OVERRIDE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_RIGHT_TO_LEFT_OVERRIDE_name, &const_CHAR_DIRECTION_RIGHT_TO_LEFT_OVERRIDE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_RIGHT_TO_LEFT_OVERRIDE_name);

	zval const_CHAR_DIRECTION_POP_DIRECTIONAL_FORMAT_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_POP_DIRECTIONAL_FORMAT_value, U_POP_DIRECTIONAL_FORMAT);
	crex_string *const_CHAR_DIRECTION_POP_DIRECTIONAL_FORMAT_name = crex_string_init_interned("CHAR_DIRECTION_POP_DIRECTIONAL_FORMAT", sizeof("CHAR_DIRECTION_POP_DIRECTIONAL_FORMAT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_POP_DIRECTIONAL_FORMAT_name, &const_CHAR_DIRECTION_POP_DIRECTIONAL_FORMAT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_POP_DIRECTIONAL_FORMAT_name);

	zval const_CHAR_DIRECTION_DIR_NON_SPACING_MARK_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_DIR_NON_SPACING_MARK_value, U_DIR_NON_SPACING_MARK);
	crex_string *const_CHAR_DIRECTION_DIR_NON_SPACING_MARK_name = crex_string_init_interned("CHAR_DIRECTION_DIR_NON_SPACING_MARK", sizeof("CHAR_DIRECTION_DIR_NON_SPACING_MARK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_DIR_NON_SPACING_MARK_name, &const_CHAR_DIRECTION_DIR_NON_SPACING_MARK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_DIR_NON_SPACING_MARK_name);

	zval const_CHAR_DIRECTION_BOUNDARY_NEUTRAL_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_BOUNDARY_NEUTRAL_value, U_BOUNDARY_NEUTRAL);
	crex_string *const_CHAR_DIRECTION_BOUNDARY_NEUTRAL_name = crex_string_init_interned("CHAR_DIRECTION_BOUNDARY_NEUTRAL", sizeof("CHAR_DIRECTION_BOUNDARY_NEUTRAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_BOUNDARY_NEUTRAL_name, &const_CHAR_DIRECTION_BOUNDARY_NEUTRAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_BOUNDARY_NEUTRAL_name);
#if U_ICU_VERSION_MAJOR_NUM >= 52

	zval const_CHAR_DIRECTION_FIRST_STRONG_ISOLATE_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_FIRST_STRONG_ISOLATE_value, U_FIRST_STRONG_ISOLATE);
	crex_string *const_CHAR_DIRECTION_FIRST_STRONG_ISOLATE_name = crex_string_init_interned("CHAR_DIRECTION_FIRST_STRONG_ISOLATE", sizeof("CHAR_DIRECTION_FIRST_STRONG_ISOLATE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_FIRST_STRONG_ISOLATE_name, &const_CHAR_DIRECTION_FIRST_STRONG_ISOLATE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_FIRST_STRONG_ISOLATE_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 52

	zval const_CHAR_DIRECTION_LEFT_TO_RIGHT_ISOLATE_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_LEFT_TO_RIGHT_ISOLATE_value, U_LEFT_TO_RIGHT_ISOLATE);
	crex_string *const_CHAR_DIRECTION_LEFT_TO_RIGHT_ISOLATE_name = crex_string_init_interned("CHAR_DIRECTION_LEFT_TO_RIGHT_ISOLATE", sizeof("CHAR_DIRECTION_LEFT_TO_RIGHT_ISOLATE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_LEFT_TO_RIGHT_ISOLATE_name, &const_CHAR_DIRECTION_LEFT_TO_RIGHT_ISOLATE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_LEFT_TO_RIGHT_ISOLATE_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 52

	zval const_CHAR_DIRECTION_RIGHT_TO_LEFT_ISOLATE_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_RIGHT_TO_LEFT_ISOLATE_value, U_RIGHT_TO_LEFT_ISOLATE);
	crex_string *const_CHAR_DIRECTION_RIGHT_TO_LEFT_ISOLATE_name = crex_string_init_interned("CHAR_DIRECTION_RIGHT_TO_LEFT_ISOLATE", sizeof("CHAR_DIRECTION_RIGHT_TO_LEFT_ISOLATE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_RIGHT_TO_LEFT_ISOLATE_name, &const_CHAR_DIRECTION_RIGHT_TO_LEFT_ISOLATE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_RIGHT_TO_LEFT_ISOLATE_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 52

	zval const_CHAR_DIRECTION_POP_DIRECTIONAL_ISOLATE_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_POP_DIRECTIONAL_ISOLATE_value, U_POP_DIRECTIONAL_ISOLATE);
	crex_string *const_CHAR_DIRECTION_POP_DIRECTIONAL_ISOLATE_name = crex_string_init_interned("CHAR_DIRECTION_POP_DIRECTIONAL_ISOLATE", sizeof("CHAR_DIRECTION_POP_DIRECTIONAL_ISOLATE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_POP_DIRECTIONAL_ISOLATE_name, &const_CHAR_DIRECTION_POP_DIRECTIONAL_ISOLATE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_POP_DIRECTIONAL_ISOLATE_name);
#endif

	zval const_CHAR_DIRECTION_CHAR_DIRECTION_COUNT_value;
	ZVAL_LONG(&const_CHAR_DIRECTION_CHAR_DIRECTION_COUNT_value, U_CHAR_DIRECTION_COUNT);
	crex_string *const_CHAR_DIRECTION_CHAR_DIRECTION_COUNT_name = crex_string_init_interned("CHAR_DIRECTION_CHAR_DIRECTION_COUNT", sizeof("CHAR_DIRECTION_CHAR_DIRECTION_COUNT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_DIRECTION_CHAR_DIRECTION_COUNT_name, &const_CHAR_DIRECTION_CHAR_DIRECTION_COUNT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_DIRECTION_CHAR_DIRECTION_COUNT_name);

	zval const_BLOCK_CODE_NO_BLOCK_value;
	ZVAL_LONG(&const_BLOCK_CODE_NO_BLOCK_value, UBLOCK_NO_BLOCK);
	crex_string *const_BLOCK_CODE_NO_BLOCK_name = crex_string_init_interned("BLOCK_CODE_NO_BLOCK", sizeof("BLOCK_CODE_NO_BLOCK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_NO_BLOCK_name, &const_BLOCK_CODE_NO_BLOCK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_NO_BLOCK_name);

	zval const_BLOCK_CODE_BASIC_LATIN_value;
	ZVAL_LONG(&const_BLOCK_CODE_BASIC_LATIN_value, UBLOCK_BASIC_LATIN);
	crex_string *const_BLOCK_CODE_BASIC_LATIN_name = crex_string_init_interned("BLOCK_CODE_BASIC_LATIN", sizeof("BLOCK_CODE_BASIC_LATIN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_BASIC_LATIN_name, &const_BLOCK_CODE_BASIC_LATIN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_BASIC_LATIN_name);

	zval const_BLOCK_CODE_LATIN_1_SUPPLEMENT_value;
	ZVAL_LONG(&const_BLOCK_CODE_LATIN_1_SUPPLEMENT_value, UBLOCK_LATIN_1_SUPPLEMENT);
	crex_string *const_BLOCK_CODE_LATIN_1_SUPPLEMENT_name = crex_string_init_interned("BLOCK_CODE_LATIN_1_SUPPLEMENT", sizeof("BLOCK_CODE_LATIN_1_SUPPLEMENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_LATIN_1_SUPPLEMENT_name, &const_BLOCK_CODE_LATIN_1_SUPPLEMENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_LATIN_1_SUPPLEMENT_name);

	zval const_BLOCK_CODE_LATIN_EXTENDED_A_value;
	ZVAL_LONG(&const_BLOCK_CODE_LATIN_EXTENDED_A_value, UBLOCK_LATIN_EXTENDED_A);
	crex_string *const_BLOCK_CODE_LATIN_EXTENDED_A_name = crex_string_init_interned("BLOCK_CODE_LATIN_EXTENDED_A", sizeof("BLOCK_CODE_LATIN_EXTENDED_A") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_LATIN_EXTENDED_A_name, &const_BLOCK_CODE_LATIN_EXTENDED_A_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_LATIN_EXTENDED_A_name);

	zval const_BLOCK_CODE_LATIN_EXTENDED_B_value;
	ZVAL_LONG(&const_BLOCK_CODE_LATIN_EXTENDED_B_value, UBLOCK_LATIN_EXTENDED_B);
	crex_string *const_BLOCK_CODE_LATIN_EXTENDED_B_name = crex_string_init_interned("BLOCK_CODE_LATIN_EXTENDED_B", sizeof("BLOCK_CODE_LATIN_EXTENDED_B") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_LATIN_EXTENDED_B_name, &const_BLOCK_CODE_LATIN_EXTENDED_B_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_LATIN_EXTENDED_B_name);

	zval const_BLOCK_CODE_IPA_EXTENSIONS_value;
	ZVAL_LONG(&const_BLOCK_CODE_IPA_EXTENSIONS_value, UBLOCK_IPA_EXTENSIONS);
	crex_string *const_BLOCK_CODE_IPA_EXTENSIONS_name = crex_string_init_interned("BLOCK_CODE_IPA_EXTENSIONS", sizeof("BLOCK_CODE_IPA_EXTENSIONS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_IPA_EXTENSIONS_name, &const_BLOCK_CODE_IPA_EXTENSIONS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_IPA_EXTENSIONS_name);

	zval const_BLOCK_CODE_SPACING_MODIFIER_LETTERS_value;
	ZVAL_LONG(&const_BLOCK_CODE_SPACING_MODIFIER_LETTERS_value, UBLOCK_SPACING_MODIFIER_LETTERS);
	crex_string *const_BLOCK_CODE_SPACING_MODIFIER_LETTERS_name = crex_string_init_interned("BLOCK_CODE_SPACING_MODIFIER_LETTERS", sizeof("BLOCK_CODE_SPACING_MODIFIER_LETTERS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SPACING_MODIFIER_LETTERS_name, &const_BLOCK_CODE_SPACING_MODIFIER_LETTERS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SPACING_MODIFIER_LETTERS_name);

	zval const_BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_value;
	ZVAL_LONG(&const_BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_value, UBLOCK_COMBINING_DIACRITICAL_MARKS);
	crex_string *const_BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_name = crex_string_init_interned("BLOCK_CODE_COMBINING_DIACRITICAL_MARKS", sizeof("BLOCK_CODE_COMBINING_DIACRITICAL_MARKS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_name, &const_BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_name);

	zval const_BLOCK_CODE_GREEK_value;
	ZVAL_LONG(&const_BLOCK_CODE_GREEK_value, UBLOCK_GREEK);
	crex_string *const_BLOCK_CODE_GREEK_name = crex_string_init_interned("BLOCK_CODE_GREEK", sizeof("BLOCK_CODE_GREEK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_GREEK_name, &const_BLOCK_CODE_GREEK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_GREEK_name);

	zval const_BLOCK_CODE_CYRILLIC_value;
	ZVAL_LONG(&const_BLOCK_CODE_CYRILLIC_value, UBLOCK_CYRILLIC);
	crex_string *const_BLOCK_CODE_CYRILLIC_name = crex_string_init_interned("BLOCK_CODE_CYRILLIC", sizeof("BLOCK_CODE_CYRILLIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CYRILLIC_name, &const_BLOCK_CODE_CYRILLIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CYRILLIC_name);

	zval const_BLOCK_CODE_ARMENIAN_value;
	ZVAL_LONG(&const_BLOCK_CODE_ARMENIAN_value, UBLOCK_ARMENIAN);
	crex_string *const_BLOCK_CODE_ARMENIAN_name = crex_string_init_interned("BLOCK_CODE_ARMENIAN", sizeof("BLOCK_CODE_ARMENIAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ARMENIAN_name, &const_BLOCK_CODE_ARMENIAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ARMENIAN_name);

	zval const_BLOCK_CODE_HEBREW_value;
	ZVAL_LONG(&const_BLOCK_CODE_HEBREW_value, UBLOCK_HEBREW);
	crex_string *const_BLOCK_CODE_HEBREW_name = crex_string_init_interned("BLOCK_CODE_HEBREW", sizeof("BLOCK_CODE_HEBREW") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_HEBREW_name, &const_BLOCK_CODE_HEBREW_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_HEBREW_name);

	zval const_BLOCK_CODE_ARABIC_value;
	ZVAL_LONG(&const_BLOCK_CODE_ARABIC_value, UBLOCK_ARABIC);
	crex_string *const_BLOCK_CODE_ARABIC_name = crex_string_init_interned("BLOCK_CODE_ARABIC", sizeof("BLOCK_CODE_ARABIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ARABIC_name, &const_BLOCK_CODE_ARABIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ARABIC_name);

	zval const_BLOCK_CODE_SYRIAC_value;
	ZVAL_LONG(&const_BLOCK_CODE_SYRIAC_value, UBLOCK_SYRIAC);
	crex_string *const_BLOCK_CODE_SYRIAC_name = crex_string_init_interned("BLOCK_CODE_SYRIAC", sizeof("BLOCK_CODE_SYRIAC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SYRIAC_name, &const_BLOCK_CODE_SYRIAC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SYRIAC_name);

	zval const_BLOCK_CODE_THAANA_value;
	ZVAL_LONG(&const_BLOCK_CODE_THAANA_value, UBLOCK_THAANA);
	crex_string *const_BLOCK_CODE_THAANA_name = crex_string_init_interned("BLOCK_CODE_THAANA", sizeof("BLOCK_CODE_THAANA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_THAANA_name, &const_BLOCK_CODE_THAANA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_THAANA_name);

	zval const_BLOCK_CODE_DEVANAGARI_value;
	ZVAL_LONG(&const_BLOCK_CODE_DEVANAGARI_value, UBLOCK_DEVANAGARI);
	crex_string *const_BLOCK_CODE_DEVANAGARI_name = crex_string_init_interned("BLOCK_CODE_DEVANAGARI", sizeof("BLOCK_CODE_DEVANAGARI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_DEVANAGARI_name, &const_BLOCK_CODE_DEVANAGARI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_DEVANAGARI_name);

	zval const_BLOCK_CODE_BENGALI_value;
	ZVAL_LONG(&const_BLOCK_CODE_BENGALI_value, UBLOCK_BENGALI);
	crex_string *const_BLOCK_CODE_BENGALI_name = crex_string_init_interned("BLOCK_CODE_BENGALI", sizeof("BLOCK_CODE_BENGALI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_BENGALI_name, &const_BLOCK_CODE_BENGALI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_BENGALI_name);

	zval const_BLOCK_CODE_GURMUKHI_value;
	ZVAL_LONG(&const_BLOCK_CODE_GURMUKHI_value, UBLOCK_GURMUKHI);
	crex_string *const_BLOCK_CODE_GURMUKHI_name = crex_string_init_interned("BLOCK_CODE_GURMUKHI", sizeof("BLOCK_CODE_GURMUKHI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_GURMUKHI_name, &const_BLOCK_CODE_GURMUKHI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_GURMUKHI_name);

	zval const_BLOCK_CODE_GUJARATI_value;
	ZVAL_LONG(&const_BLOCK_CODE_GUJARATI_value, UBLOCK_GUJARATI);
	crex_string *const_BLOCK_CODE_GUJARATI_name = crex_string_init_interned("BLOCK_CODE_GUJARATI", sizeof("BLOCK_CODE_GUJARATI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_GUJARATI_name, &const_BLOCK_CODE_GUJARATI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_GUJARATI_name);

	zval const_BLOCK_CODE_ORIYA_value;
	ZVAL_LONG(&const_BLOCK_CODE_ORIYA_value, UBLOCK_ORIYA);
	crex_string *const_BLOCK_CODE_ORIYA_name = crex_string_init_interned("BLOCK_CODE_ORIYA", sizeof("BLOCK_CODE_ORIYA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ORIYA_name, &const_BLOCK_CODE_ORIYA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ORIYA_name);

	zval const_BLOCK_CODE_TAMIL_value;
	ZVAL_LONG(&const_BLOCK_CODE_TAMIL_value, UBLOCK_TAMIL);
	crex_string *const_BLOCK_CODE_TAMIL_name = crex_string_init_interned("BLOCK_CODE_TAMIL", sizeof("BLOCK_CODE_TAMIL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_TAMIL_name, &const_BLOCK_CODE_TAMIL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_TAMIL_name);

	zval const_BLOCK_CODE_TELUGU_value;
	ZVAL_LONG(&const_BLOCK_CODE_TELUGU_value, UBLOCK_TELUGU);
	crex_string *const_BLOCK_CODE_TELUGU_name = crex_string_init_interned("BLOCK_CODE_TELUGU", sizeof("BLOCK_CODE_TELUGU") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_TELUGU_name, &const_BLOCK_CODE_TELUGU_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_TELUGU_name);

	zval const_BLOCK_CODE_KANNADA_value;
	ZVAL_LONG(&const_BLOCK_CODE_KANNADA_value, UBLOCK_KANNADA);
	crex_string *const_BLOCK_CODE_KANNADA_name = crex_string_init_interned("BLOCK_CODE_KANNADA", sizeof("BLOCK_CODE_KANNADA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_KANNADA_name, &const_BLOCK_CODE_KANNADA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_KANNADA_name);

	zval const_BLOCK_CODE_MALAYALAM_value;
	ZVAL_LONG(&const_BLOCK_CODE_MALAYALAM_value, UBLOCK_MALAYALAM);
	crex_string *const_BLOCK_CODE_MALAYALAM_name = crex_string_init_interned("BLOCK_CODE_MALAYALAM", sizeof("BLOCK_CODE_MALAYALAM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MALAYALAM_name, &const_BLOCK_CODE_MALAYALAM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MALAYALAM_name);

	zval const_BLOCK_CODE_SINHALA_value;
	ZVAL_LONG(&const_BLOCK_CODE_SINHALA_value, UBLOCK_SINHALA);
	crex_string *const_BLOCK_CODE_SINHALA_name = crex_string_init_interned("BLOCK_CODE_SINHALA", sizeof("BLOCK_CODE_SINHALA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SINHALA_name, &const_BLOCK_CODE_SINHALA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SINHALA_name);

	zval const_BLOCK_CODE_THAI_value;
	ZVAL_LONG(&const_BLOCK_CODE_THAI_value, UBLOCK_THAI);
	crex_string *const_BLOCK_CODE_THAI_name = crex_string_init_interned("BLOCK_CODE_THAI", sizeof("BLOCK_CODE_THAI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_THAI_name, &const_BLOCK_CODE_THAI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_THAI_name);

	zval const_BLOCK_CODE_LAO_value;
	ZVAL_LONG(&const_BLOCK_CODE_LAO_value, UBLOCK_LAO);
	crex_string *const_BLOCK_CODE_LAO_name = crex_string_init_interned("BLOCK_CODE_LAO", sizeof("BLOCK_CODE_LAO") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_LAO_name, &const_BLOCK_CODE_LAO_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_LAO_name);

	zval const_BLOCK_CODE_TIBETAN_value;
	ZVAL_LONG(&const_BLOCK_CODE_TIBETAN_value, UBLOCK_TIBETAN);
	crex_string *const_BLOCK_CODE_TIBETAN_name = crex_string_init_interned("BLOCK_CODE_TIBETAN", sizeof("BLOCK_CODE_TIBETAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_TIBETAN_name, &const_BLOCK_CODE_TIBETAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_TIBETAN_name);

	zval const_BLOCK_CODE_MYANMAR_value;
	ZVAL_LONG(&const_BLOCK_CODE_MYANMAR_value, UBLOCK_MYANMAR);
	crex_string *const_BLOCK_CODE_MYANMAR_name = crex_string_init_interned("BLOCK_CODE_MYANMAR", sizeof("BLOCK_CODE_MYANMAR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MYANMAR_name, &const_BLOCK_CODE_MYANMAR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MYANMAR_name);

	zval const_BLOCK_CODE_GEORGIAN_value;
	ZVAL_LONG(&const_BLOCK_CODE_GEORGIAN_value, UBLOCK_GEORGIAN);
	crex_string *const_BLOCK_CODE_GEORGIAN_name = crex_string_init_interned("BLOCK_CODE_GEORGIAN", sizeof("BLOCK_CODE_GEORGIAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_GEORGIAN_name, &const_BLOCK_CODE_GEORGIAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_GEORGIAN_name);

	zval const_BLOCK_CODE_HANGUL_JAMO_value;
	ZVAL_LONG(&const_BLOCK_CODE_HANGUL_JAMO_value, UBLOCK_HANGUL_JAMO);
	crex_string *const_BLOCK_CODE_HANGUL_JAMO_name = crex_string_init_interned("BLOCK_CODE_HANGUL_JAMO", sizeof("BLOCK_CODE_HANGUL_JAMO") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_HANGUL_JAMO_name, &const_BLOCK_CODE_HANGUL_JAMO_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_HANGUL_JAMO_name);

	zval const_BLOCK_CODE_ETHIOPIC_value;
	ZVAL_LONG(&const_BLOCK_CODE_ETHIOPIC_value, UBLOCK_ETHIOPIC);
	crex_string *const_BLOCK_CODE_ETHIOPIC_name = crex_string_init_interned("BLOCK_CODE_ETHIOPIC", sizeof("BLOCK_CODE_ETHIOPIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ETHIOPIC_name, &const_BLOCK_CODE_ETHIOPIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ETHIOPIC_name);

	zval const_BLOCK_CODE_CHEROKEE_value;
	ZVAL_LONG(&const_BLOCK_CODE_CHEROKEE_value, UBLOCK_CHEROKEE);
	crex_string *const_BLOCK_CODE_CHEROKEE_name = crex_string_init_interned("BLOCK_CODE_CHEROKEE", sizeof("BLOCK_CODE_CHEROKEE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CHEROKEE_name, &const_BLOCK_CODE_CHEROKEE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CHEROKEE_name);

	zval const_BLOCK_CODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_value;
	ZVAL_LONG(&const_BLOCK_CODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_value, UBLOCK_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS);
	crex_string *const_BLOCK_CODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_name = crex_string_init_interned("BLOCK_CODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS", sizeof("BLOCK_CODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_name, &const_BLOCK_CODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_name);

	zval const_BLOCK_CODE_OGHAM_value;
	ZVAL_LONG(&const_BLOCK_CODE_OGHAM_value, UBLOCK_OGHAM);
	crex_string *const_BLOCK_CODE_OGHAM_name = crex_string_init_interned("BLOCK_CODE_OGHAM", sizeof("BLOCK_CODE_OGHAM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_OGHAM_name, &const_BLOCK_CODE_OGHAM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_OGHAM_name);

	zval const_BLOCK_CODE_RUNIC_value;
	ZVAL_LONG(&const_BLOCK_CODE_RUNIC_value, UBLOCK_RUNIC);
	crex_string *const_BLOCK_CODE_RUNIC_name = crex_string_init_interned("BLOCK_CODE_RUNIC", sizeof("BLOCK_CODE_RUNIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_RUNIC_name, &const_BLOCK_CODE_RUNIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_RUNIC_name);

	zval const_BLOCK_CODE_KHMER_value;
	ZVAL_LONG(&const_BLOCK_CODE_KHMER_value, UBLOCK_KHMER);
	crex_string *const_BLOCK_CODE_KHMER_name = crex_string_init_interned("BLOCK_CODE_KHMER", sizeof("BLOCK_CODE_KHMER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_KHMER_name, &const_BLOCK_CODE_KHMER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_KHMER_name);

	zval const_BLOCK_CODE_MONGOLIAN_value;
	ZVAL_LONG(&const_BLOCK_CODE_MONGOLIAN_value, UBLOCK_MONGOLIAN);
	crex_string *const_BLOCK_CODE_MONGOLIAN_name = crex_string_init_interned("BLOCK_CODE_MONGOLIAN", sizeof("BLOCK_CODE_MONGOLIAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MONGOLIAN_name, &const_BLOCK_CODE_MONGOLIAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MONGOLIAN_name);

	zval const_BLOCK_CODE_LATIN_EXTENDED_ADDITIONAL_value;
	ZVAL_LONG(&const_BLOCK_CODE_LATIN_EXTENDED_ADDITIONAL_value, UBLOCK_LATIN_EXTENDED_ADDITIONAL);
	crex_string *const_BLOCK_CODE_LATIN_EXTENDED_ADDITIONAL_name = crex_string_init_interned("BLOCK_CODE_LATIN_EXTENDED_ADDITIONAL", sizeof("BLOCK_CODE_LATIN_EXTENDED_ADDITIONAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_LATIN_EXTENDED_ADDITIONAL_name, &const_BLOCK_CODE_LATIN_EXTENDED_ADDITIONAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_LATIN_EXTENDED_ADDITIONAL_name);

	zval const_BLOCK_CODE_GREEK_EXTENDED_value;
	ZVAL_LONG(&const_BLOCK_CODE_GREEK_EXTENDED_value, UBLOCK_GREEK_EXTENDED);
	crex_string *const_BLOCK_CODE_GREEK_EXTENDED_name = crex_string_init_interned("BLOCK_CODE_GREEK_EXTENDED", sizeof("BLOCK_CODE_GREEK_EXTENDED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_GREEK_EXTENDED_name, &const_BLOCK_CODE_GREEK_EXTENDED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_GREEK_EXTENDED_name);

	zval const_BLOCK_CODE_GENERAL_PUNCTUATION_value;
	ZVAL_LONG(&const_BLOCK_CODE_GENERAL_PUNCTUATION_value, UBLOCK_GENERAL_PUNCTUATION);
	crex_string *const_BLOCK_CODE_GENERAL_PUNCTUATION_name = crex_string_init_interned("BLOCK_CODE_GENERAL_PUNCTUATION", sizeof("BLOCK_CODE_GENERAL_PUNCTUATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_GENERAL_PUNCTUATION_name, &const_BLOCK_CODE_GENERAL_PUNCTUATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_GENERAL_PUNCTUATION_name);

	zval const_BLOCK_CODE_SUPERSCRIPTS_AND_SUBSCRIPTS_value;
	ZVAL_LONG(&const_BLOCK_CODE_SUPERSCRIPTS_AND_SUBSCRIPTS_value, UBLOCK_SUPERSCRIPTS_AND_SUBSCRIPTS);
	crex_string *const_BLOCK_CODE_SUPERSCRIPTS_AND_SUBSCRIPTS_name = crex_string_init_interned("BLOCK_CODE_SUPERSCRIPTS_AND_SUBSCRIPTS", sizeof("BLOCK_CODE_SUPERSCRIPTS_AND_SUBSCRIPTS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SUPERSCRIPTS_AND_SUBSCRIPTS_name, &const_BLOCK_CODE_SUPERSCRIPTS_AND_SUBSCRIPTS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SUPERSCRIPTS_AND_SUBSCRIPTS_name);

	zval const_BLOCK_CODE_CURRENCY_SYMBOLS_value;
	ZVAL_LONG(&const_BLOCK_CODE_CURRENCY_SYMBOLS_value, UBLOCK_CURRENCY_SYMBOLS);
	crex_string *const_BLOCK_CODE_CURRENCY_SYMBOLS_name = crex_string_init_interned("BLOCK_CODE_CURRENCY_SYMBOLS", sizeof("BLOCK_CODE_CURRENCY_SYMBOLS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CURRENCY_SYMBOLS_name, &const_BLOCK_CODE_CURRENCY_SYMBOLS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CURRENCY_SYMBOLS_name);

	zval const_BLOCK_CODE_COMBINING_MARKS_FOR_SYMBOLS_value;
	ZVAL_LONG(&const_BLOCK_CODE_COMBINING_MARKS_FOR_SYMBOLS_value, UBLOCK_COMBINING_MARKS_FOR_SYMBOLS);
	crex_string *const_BLOCK_CODE_COMBINING_MARKS_FOR_SYMBOLS_name = crex_string_init_interned("BLOCK_CODE_COMBINING_MARKS_FOR_SYMBOLS", sizeof("BLOCK_CODE_COMBINING_MARKS_FOR_SYMBOLS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_COMBINING_MARKS_FOR_SYMBOLS_name, &const_BLOCK_CODE_COMBINING_MARKS_FOR_SYMBOLS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_COMBINING_MARKS_FOR_SYMBOLS_name);

	zval const_BLOCK_CODE_LETTERLIKE_SYMBOLS_value;
	ZVAL_LONG(&const_BLOCK_CODE_LETTERLIKE_SYMBOLS_value, UBLOCK_LETTERLIKE_SYMBOLS);
	crex_string *const_BLOCK_CODE_LETTERLIKE_SYMBOLS_name = crex_string_init_interned("BLOCK_CODE_LETTERLIKE_SYMBOLS", sizeof("BLOCK_CODE_LETTERLIKE_SYMBOLS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_LETTERLIKE_SYMBOLS_name, &const_BLOCK_CODE_LETTERLIKE_SYMBOLS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_LETTERLIKE_SYMBOLS_name);

	zval const_BLOCK_CODE_NUMBER_FORMS_value;
	ZVAL_LONG(&const_BLOCK_CODE_NUMBER_FORMS_value, UBLOCK_NUMBER_FORMS);
	crex_string *const_BLOCK_CODE_NUMBER_FORMS_name = crex_string_init_interned("BLOCK_CODE_NUMBER_FORMS", sizeof("BLOCK_CODE_NUMBER_FORMS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_NUMBER_FORMS_name, &const_BLOCK_CODE_NUMBER_FORMS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_NUMBER_FORMS_name);

	zval const_BLOCK_CODE_ARROWS_value;
	ZVAL_LONG(&const_BLOCK_CODE_ARROWS_value, UBLOCK_ARROWS);
	crex_string *const_BLOCK_CODE_ARROWS_name = crex_string_init_interned("BLOCK_CODE_ARROWS", sizeof("BLOCK_CODE_ARROWS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ARROWS_name, &const_BLOCK_CODE_ARROWS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ARROWS_name);

	zval const_BLOCK_CODE_MATHEMATICAL_OPERATORS_value;
	ZVAL_LONG(&const_BLOCK_CODE_MATHEMATICAL_OPERATORS_value, UBLOCK_MATHEMATICAL_OPERATORS);
	crex_string *const_BLOCK_CODE_MATHEMATICAL_OPERATORS_name = crex_string_init_interned("BLOCK_CODE_MATHEMATICAL_OPERATORS", sizeof("BLOCK_CODE_MATHEMATICAL_OPERATORS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MATHEMATICAL_OPERATORS_name, &const_BLOCK_CODE_MATHEMATICAL_OPERATORS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MATHEMATICAL_OPERATORS_name);

	zval const_BLOCK_CODE_MISCELLANEOUS_TECHNICAL_value;
	ZVAL_LONG(&const_BLOCK_CODE_MISCELLANEOUS_TECHNICAL_value, UBLOCK_MISCELLANEOUS_TECHNICAL);
	crex_string *const_BLOCK_CODE_MISCELLANEOUS_TECHNICAL_name = crex_string_init_interned("BLOCK_CODE_MISCELLANEOUS_TECHNICAL", sizeof("BLOCK_CODE_MISCELLANEOUS_TECHNICAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MISCELLANEOUS_TECHNICAL_name, &const_BLOCK_CODE_MISCELLANEOUS_TECHNICAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MISCELLANEOUS_TECHNICAL_name);

	zval const_BLOCK_CODE_CONTROL_PICTURES_value;
	ZVAL_LONG(&const_BLOCK_CODE_CONTROL_PICTURES_value, UBLOCK_CONTROL_PICTURES);
	crex_string *const_BLOCK_CODE_CONTROL_PICTURES_name = crex_string_init_interned("BLOCK_CODE_CONTROL_PICTURES", sizeof("BLOCK_CODE_CONTROL_PICTURES") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CONTROL_PICTURES_name, &const_BLOCK_CODE_CONTROL_PICTURES_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CONTROL_PICTURES_name);

	zval const_BLOCK_CODE_OPTICAL_CHARACTER_RECOGNITION_value;
	ZVAL_LONG(&const_BLOCK_CODE_OPTICAL_CHARACTER_RECOGNITION_value, UBLOCK_OPTICAL_CHARACTER_RECOGNITION);
	crex_string *const_BLOCK_CODE_OPTICAL_CHARACTER_RECOGNITION_name = crex_string_init_interned("BLOCK_CODE_OPTICAL_CHARACTER_RECOGNITION", sizeof("BLOCK_CODE_OPTICAL_CHARACTER_RECOGNITION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_OPTICAL_CHARACTER_RECOGNITION_name, &const_BLOCK_CODE_OPTICAL_CHARACTER_RECOGNITION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_OPTICAL_CHARACTER_RECOGNITION_name);

	zval const_BLOCK_CODE_ENCLOSED_ALPHANUMERICS_value;
	ZVAL_LONG(&const_BLOCK_CODE_ENCLOSED_ALPHANUMERICS_value, UBLOCK_ENCLOSED_ALPHANUMERICS);
	crex_string *const_BLOCK_CODE_ENCLOSED_ALPHANUMERICS_name = crex_string_init_interned("BLOCK_CODE_ENCLOSED_ALPHANUMERICS", sizeof("BLOCK_CODE_ENCLOSED_ALPHANUMERICS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ENCLOSED_ALPHANUMERICS_name, &const_BLOCK_CODE_ENCLOSED_ALPHANUMERICS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ENCLOSED_ALPHANUMERICS_name);

	zval const_BLOCK_CODE_BOX_DRAWING_value;
	ZVAL_LONG(&const_BLOCK_CODE_BOX_DRAWING_value, UBLOCK_BOX_DRAWING);
	crex_string *const_BLOCK_CODE_BOX_DRAWING_name = crex_string_init_interned("BLOCK_CODE_BOX_DRAWING", sizeof("BLOCK_CODE_BOX_DRAWING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_BOX_DRAWING_name, &const_BLOCK_CODE_BOX_DRAWING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_BOX_DRAWING_name);

	zval const_BLOCK_CODE_BLOCK_ELEMENTS_value;
	ZVAL_LONG(&const_BLOCK_CODE_BLOCK_ELEMENTS_value, UBLOCK_BLOCK_ELEMENTS);
	crex_string *const_BLOCK_CODE_BLOCK_ELEMENTS_name = crex_string_init_interned("BLOCK_CODE_BLOCK_ELEMENTS", sizeof("BLOCK_CODE_BLOCK_ELEMENTS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_BLOCK_ELEMENTS_name, &const_BLOCK_CODE_BLOCK_ELEMENTS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_BLOCK_ELEMENTS_name);

	zval const_BLOCK_CODE_GEOMETRIC_SHAPES_value;
	ZVAL_LONG(&const_BLOCK_CODE_GEOMETRIC_SHAPES_value, UBLOCK_GEOMETRIC_SHAPES);
	crex_string *const_BLOCK_CODE_GEOMETRIC_SHAPES_name = crex_string_init_interned("BLOCK_CODE_GEOMETRIC_SHAPES", sizeof("BLOCK_CODE_GEOMETRIC_SHAPES") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_GEOMETRIC_SHAPES_name, &const_BLOCK_CODE_GEOMETRIC_SHAPES_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_GEOMETRIC_SHAPES_name);

	zval const_BLOCK_CODE_MISCELLANEOUS_SYMBOLS_value;
	ZVAL_LONG(&const_BLOCK_CODE_MISCELLANEOUS_SYMBOLS_value, UBLOCK_MISCELLANEOUS_SYMBOLS);
	crex_string *const_BLOCK_CODE_MISCELLANEOUS_SYMBOLS_name = crex_string_init_interned("BLOCK_CODE_MISCELLANEOUS_SYMBOLS", sizeof("BLOCK_CODE_MISCELLANEOUS_SYMBOLS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MISCELLANEOUS_SYMBOLS_name, &const_BLOCK_CODE_MISCELLANEOUS_SYMBOLS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MISCELLANEOUS_SYMBOLS_name);

	zval const_BLOCK_CODE_DINGBATS_value;
	ZVAL_LONG(&const_BLOCK_CODE_DINGBATS_value, UBLOCK_DINGBATS);
	crex_string *const_BLOCK_CODE_DINGBATS_name = crex_string_init_interned("BLOCK_CODE_DINGBATS", sizeof("BLOCK_CODE_DINGBATS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_DINGBATS_name, &const_BLOCK_CODE_DINGBATS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_DINGBATS_name);

	zval const_BLOCK_CODE_BRAILLE_PATTERNS_value;
	ZVAL_LONG(&const_BLOCK_CODE_BRAILLE_PATTERNS_value, UBLOCK_BRAILLE_PATTERNS);
	crex_string *const_BLOCK_CODE_BRAILLE_PATTERNS_name = crex_string_init_interned("BLOCK_CODE_BRAILLE_PATTERNS", sizeof("BLOCK_CODE_BRAILLE_PATTERNS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_BRAILLE_PATTERNS_name, &const_BLOCK_CODE_BRAILLE_PATTERNS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_BRAILLE_PATTERNS_name);

	zval const_BLOCK_CODE_CJK_RADICALS_SUPPLEMENT_value;
	ZVAL_LONG(&const_BLOCK_CODE_CJK_RADICALS_SUPPLEMENT_value, UBLOCK_CJK_RADICALS_SUPPLEMENT);
	crex_string *const_BLOCK_CODE_CJK_RADICALS_SUPPLEMENT_name = crex_string_init_interned("BLOCK_CODE_CJK_RADICALS_SUPPLEMENT", sizeof("BLOCK_CODE_CJK_RADICALS_SUPPLEMENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CJK_RADICALS_SUPPLEMENT_name, &const_BLOCK_CODE_CJK_RADICALS_SUPPLEMENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CJK_RADICALS_SUPPLEMENT_name);

	zval const_BLOCK_CODE_KANGXI_RADICALS_value;
	ZVAL_LONG(&const_BLOCK_CODE_KANGXI_RADICALS_value, UBLOCK_KANGXI_RADICALS);
	crex_string *const_BLOCK_CODE_KANGXI_RADICALS_name = crex_string_init_interned("BLOCK_CODE_KANGXI_RADICALS", sizeof("BLOCK_CODE_KANGXI_RADICALS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_KANGXI_RADICALS_name, &const_BLOCK_CODE_KANGXI_RADICALS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_KANGXI_RADICALS_name);

	zval const_BLOCK_CODE_IDEOGRAPHIC_DESCRIPTION_CHARACTERS_value;
	ZVAL_LONG(&const_BLOCK_CODE_IDEOGRAPHIC_DESCRIPTION_CHARACTERS_value, UBLOCK_IDEOGRAPHIC_DESCRIPTION_CHARACTERS);
	crex_string *const_BLOCK_CODE_IDEOGRAPHIC_DESCRIPTION_CHARACTERS_name = crex_string_init_interned("BLOCK_CODE_IDEOGRAPHIC_DESCRIPTION_CHARACTERS", sizeof("BLOCK_CODE_IDEOGRAPHIC_DESCRIPTION_CHARACTERS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_IDEOGRAPHIC_DESCRIPTION_CHARACTERS_name, &const_BLOCK_CODE_IDEOGRAPHIC_DESCRIPTION_CHARACTERS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_IDEOGRAPHIC_DESCRIPTION_CHARACTERS_name);

	zval const_BLOCK_CODE_CJK_SYMBOLS_AND_PUNCTUATION_value;
	ZVAL_LONG(&const_BLOCK_CODE_CJK_SYMBOLS_AND_PUNCTUATION_value, UBLOCK_CJK_SYMBOLS_AND_PUNCTUATION);
	crex_string *const_BLOCK_CODE_CJK_SYMBOLS_AND_PUNCTUATION_name = crex_string_init_interned("BLOCK_CODE_CJK_SYMBOLS_AND_PUNCTUATION", sizeof("BLOCK_CODE_CJK_SYMBOLS_AND_PUNCTUATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CJK_SYMBOLS_AND_PUNCTUATION_name, &const_BLOCK_CODE_CJK_SYMBOLS_AND_PUNCTUATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CJK_SYMBOLS_AND_PUNCTUATION_name);

	zval const_BLOCK_CODE_HIRAGANA_value;
	ZVAL_LONG(&const_BLOCK_CODE_HIRAGANA_value, UBLOCK_HIRAGANA);
	crex_string *const_BLOCK_CODE_HIRAGANA_name = crex_string_init_interned("BLOCK_CODE_HIRAGANA", sizeof("BLOCK_CODE_HIRAGANA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_HIRAGANA_name, &const_BLOCK_CODE_HIRAGANA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_HIRAGANA_name);

	zval const_BLOCK_CODE_KATAKANA_value;
	ZVAL_LONG(&const_BLOCK_CODE_KATAKANA_value, UBLOCK_KATAKANA);
	crex_string *const_BLOCK_CODE_KATAKANA_name = crex_string_init_interned("BLOCK_CODE_KATAKANA", sizeof("BLOCK_CODE_KATAKANA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_KATAKANA_name, &const_BLOCK_CODE_KATAKANA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_KATAKANA_name);

	zval const_BLOCK_CODE_BOPOMOFO_value;
	ZVAL_LONG(&const_BLOCK_CODE_BOPOMOFO_value, UBLOCK_BOPOMOFO);
	crex_string *const_BLOCK_CODE_BOPOMOFO_name = crex_string_init_interned("BLOCK_CODE_BOPOMOFO", sizeof("BLOCK_CODE_BOPOMOFO") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_BOPOMOFO_name, &const_BLOCK_CODE_BOPOMOFO_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_BOPOMOFO_name);

	zval const_BLOCK_CODE_HANGUL_COMPATIBILITY_JAMO_value;
	ZVAL_LONG(&const_BLOCK_CODE_HANGUL_COMPATIBILITY_JAMO_value, UBLOCK_HANGUL_COMPATIBILITY_JAMO);
	crex_string *const_BLOCK_CODE_HANGUL_COMPATIBILITY_JAMO_name = crex_string_init_interned("BLOCK_CODE_HANGUL_COMPATIBILITY_JAMO", sizeof("BLOCK_CODE_HANGUL_COMPATIBILITY_JAMO") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_HANGUL_COMPATIBILITY_JAMO_name, &const_BLOCK_CODE_HANGUL_COMPATIBILITY_JAMO_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_HANGUL_COMPATIBILITY_JAMO_name);

	zval const_BLOCK_CODE_KANBUN_value;
	ZVAL_LONG(&const_BLOCK_CODE_KANBUN_value, UBLOCK_KANBUN);
	crex_string *const_BLOCK_CODE_KANBUN_name = crex_string_init_interned("BLOCK_CODE_KANBUN", sizeof("BLOCK_CODE_KANBUN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_KANBUN_name, &const_BLOCK_CODE_KANBUN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_KANBUN_name);

	zval const_BLOCK_CODE_BOPOMOFO_EXTENDED_value;
	ZVAL_LONG(&const_BLOCK_CODE_BOPOMOFO_EXTENDED_value, UBLOCK_BOPOMOFO_EXTENDED);
	crex_string *const_BLOCK_CODE_BOPOMOFO_EXTENDED_name = crex_string_init_interned("BLOCK_CODE_BOPOMOFO_EXTENDED", sizeof("BLOCK_CODE_BOPOMOFO_EXTENDED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_BOPOMOFO_EXTENDED_name, &const_BLOCK_CODE_BOPOMOFO_EXTENDED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_BOPOMOFO_EXTENDED_name);

	zval const_BLOCK_CODE_ENCLOSED_CJK_LETTERS_AND_MONTHS_value;
	ZVAL_LONG(&const_BLOCK_CODE_ENCLOSED_CJK_LETTERS_AND_MONTHS_value, UBLOCK_ENCLOSED_CJK_LETTERS_AND_MONTHS);
	crex_string *const_BLOCK_CODE_ENCLOSED_CJK_LETTERS_AND_MONTHS_name = crex_string_init_interned("BLOCK_CODE_ENCLOSED_CJK_LETTERS_AND_MONTHS", sizeof("BLOCK_CODE_ENCLOSED_CJK_LETTERS_AND_MONTHS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ENCLOSED_CJK_LETTERS_AND_MONTHS_name, &const_BLOCK_CODE_ENCLOSED_CJK_LETTERS_AND_MONTHS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ENCLOSED_CJK_LETTERS_AND_MONTHS_name);

	zval const_BLOCK_CODE_CJK_COMPATIBILITY_value;
	ZVAL_LONG(&const_BLOCK_CODE_CJK_COMPATIBILITY_value, UBLOCK_CJK_COMPATIBILITY);
	crex_string *const_BLOCK_CODE_CJK_COMPATIBILITY_name = crex_string_init_interned("BLOCK_CODE_CJK_COMPATIBILITY", sizeof("BLOCK_CODE_CJK_COMPATIBILITY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CJK_COMPATIBILITY_name, &const_BLOCK_CODE_CJK_COMPATIBILITY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CJK_COMPATIBILITY_name);

	zval const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A_value;
	ZVAL_LONG(&const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A_value, UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A);
	crex_string *const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A_name = crex_string_init_interned("BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A", sizeof("BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A_name, &const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A_name);

	zval const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_value;
	ZVAL_LONG(&const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_value, UBLOCK_CJK_UNIFIED_IDEOGRAPHS);
	crex_string *const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_name = crex_string_init_interned("BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS", sizeof("BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_name, &const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_name);

	zval const_BLOCK_CODE_YI_SYLLABLES_value;
	ZVAL_LONG(&const_BLOCK_CODE_YI_SYLLABLES_value, UBLOCK_YI_SYLLABLES);
	crex_string *const_BLOCK_CODE_YI_SYLLABLES_name = crex_string_init_interned("BLOCK_CODE_YI_SYLLABLES", sizeof("BLOCK_CODE_YI_SYLLABLES") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_YI_SYLLABLES_name, &const_BLOCK_CODE_YI_SYLLABLES_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_YI_SYLLABLES_name);

	zval const_BLOCK_CODE_YI_RADICALS_value;
	ZVAL_LONG(&const_BLOCK_CODE_YI_RADICALS_value, UBLOCK_YI_RADICALS);
	crex_string *const_BLOCK_CODE_YI_RADICALS_name = crex_string_init_interned("BLOCK_CODE_YI_RADICALS", sizeof("BLOCK_CODE_YI_RADICALS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_YI_RADICALS_name, &const_BLOCK_CODE_YI_RADICALS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_YI_RADICALS_name);

	zval const_BLOCK_CODE_HANGUL_SYLLABLES_value;
	ZVAL_LONG(&const_BLOCK_CODE_HANGUL_SYLLABLES_value, UBLOCK_HANGUL_SYLLABLES);
	crex_string *const_BLOCK_CODE_HANGUL_SYLLABLES_name = crex_string_init_interned("BLOCK_CODE_HANGUL_SYLLABLES", sizeof("BLOCK_CODE_HANGUL_SYLLABLES") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_HANGUL_SYLLABLES_name, &const_BLOCK_CODE_HANGUL_SYLLABLES_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_HANGUL_SYLLABLES_name);

	zval const_BLOCK_CODE_HIGH_SURROGATES_value;
	ZVAL_LONG(&const_BLOCK_CODE_HIGH_SURROGATES_value, UBLOCK_HIGH_SURROGATES);
	crex_string *const_BLOCK_CODE_HIGH_SURROGATES_name = crex_string_init_interned("BLOCK_CODE_HIGH_SURROGATES", sizeof("BLOCK_CODE_HIGH_SURROGATES") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_HIGH_SURROGATES_name, &const_BLOCK_CODE_HIGH_SURROGATES_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_HIGH_SURROGATES_name);

	zval const_BLOCK_CODE_HIGH_PRIVATE_USE_SURROGATES_value;
	ZVAL_LONG(&const_BLOCK_CODE_HIGH_PRIVATE_USE_SURROGATES_value, UBLOCK_HIGH_PRIVATE_USE_SURROGATES);
	crex_string *const_BLOCK_CODE_HIGH_PRIVATE_USE_SURROGATES_name = crex_string_init_interned("BLOCK_CODE_HIGH_PRIVATE_USE_SURROGATES", sizeof("BLOCK_CODE_HIGH_PRIVATE_USE_SURROGATES") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_HIGH_PRIVATE_USE_SURROGATES_name, &const_BLOCK_CODE_HIGH_PRIVATE_USE_SURROGATES_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_HIGH_PRIVATE_USE_SURROGATES_name);

	zval const_BLOCK_CODE_LOW_SURROGATES_value;
	ZVAL_LONG(&const_BLOCK_CODE_LOW_SURROGATES_value, UBLOCK_LOW_SURROGATES);
	crex_string *const_BLOCK_CODE_LOW_SURROGATES_name = crex_string_init_interned("BLOCK_CODE_LOW_SURROGATES", sizeof("BLOCK_CODE_LOW_SURROGATES") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_LOW_SURROGATES_name, &const_BLOCK_CODE_LOW_SURROGATES_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_LOW_SURROGATES_name);

	zval const_BLOCK_CODE_PRIVATE_USE_AREA_value;
	ZVAL_LONG(&const_BLOCK_CODE_PRIVATE_USE_AREA_value, UBLOCK_PRIVATE_USE_AREA);
	crex_string *const_BLOCK_CODE_PRIVATE_USE_AREA_name = crex_string_init_interned("BLOCK_CODE_PRIVATE_USE_AREA", sizeof("BLOCK_CODE_PRIVATE_USE_AREA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_PRIVATE_USE_AREA_name, &const_BLOCK_CODE_PRIVATE_USE_AREA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_PRIVATE_USE_AREA_name);

	zval const_BLOCK_CODE_PRIVATE_USE_value;
	ZVAL_LONG(&const_BLOCK_CODE_PRIVATE_USE_value, UBLOCK_PRIVATE_USE);
	crex_string *const_BLOCK_CODE_PRIVATE_USE_name = crex_string_init_interned("BLOCK_CODE_PRIVATE_USE", sizeof("BLOCK_CODE_PRIVATE_USE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_PRIVATE_USE_name, &const_BLOCK_CODE_PRIVATE_USE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_PRIVATE_USE_name);

	zval const_BLOCK_CODE_CJK_COMPATIBILITY_IDEOGRAPHS_value;
	ZVAL_LONG(&const_BLOCK_CODE_CJK_COMPATIBILITY_IDEOGRAPHS_value, UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS);
	crex_string *const_BLOCK_CODE_CJK_COMPATIBILITY_IDEOGRAPHS_name = crex_string_init_interned("BLOCK_CODE_CJK_COMPATIBILITY_IDEOGRAPHS", sizeof("BLOCK_CODE_CJK_COMPATIBILITY_IDEOGRAPHS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CJK_COMPATIBILITY_IDEOGRAPHS_name, &const_BLOCK_CODE_CJK_COMPATIBILITY_IDEOGRAPHS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CJK_COMPATIBILITY_IDEOGRAPHS_name);

	zval const_BLOCK_CODE_ALPHABETIC_PRESENTATION_FORMS_value;
	ZVAL_LONG(&const_BLOCK_CODE_ALPHABETIC_PRESENTATION_FORMS_value, UBLOCK_ALPHABETIC_PRESENTATION_FORMS);
	crex_string *const_BLOCK_CODE_ALPHABETIC_PRESENTATION_FORMS_name = crex_string_init_interned("BLOCK_CODE_ALPHABETIC_PRESENTATION_FORMS", sizeof("BLOCK_CODE_ALPHABETIC_PRESENTATION_FORMS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ALPHABETIC_PRESENTATION_FORMS_name, &const_BLOCK_CODE_ALPHABETIC_PRESENTATION_FORMS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ALPHABETIC_PRESENTATION_FORMS_name);

	zval const_BLOCK_CODE_ARABIC_PRESENTATION_FORMS_A_value;
	ZVAL_LONG(&const_BLOCK_CODE_ARABIC_PRESENTATION_FORMS_A_value, UBLOCK_ARABIC_PRESENTATION_FORMS_A);
	crex_string *const_BLOCK_CODE_ARABIC_PRESENTATION_FORMS_A_name = crex_string_init_interned("BLOCK_CODE_ARABIC_PRESENTATION_FORMS_A", sizeof("BLOCK_CODE_ARABIC_PRESENTATION_FORMS_A") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ARABIC_PRESENTATION_FORMS_A_name, &const_BLOCK_CODE_ARABIC_PRESENTATION_FORMS_A_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ARABIC_PRESENTATION_FORMS_A_name);

	zval const_BLOCK_CODE_COMBINING_HALF_MARKS_value;
	ZVAL_LONG(&const_BLOCK_CODE_COMBINING_HALF_MARKS_value, UBLOCK_COMBINING_HALF_MARKS);
	crex_string *const_BLOCK_CODE_COMBINING_HALF_MARKS_name = crex_string_init_interned("BLOCK_CODE_COMBINING_HALF_MARKS", sizeof("BLOCK_CODE_COMBINING_HALF_MARKS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_COMBINING_HALF_MARKS_name, &const_BLOCK_CODE_COMBINING_HALF_MARKS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_COMBINING_HALF_MARKS_name);

	zval const_BLOCK_CODE_CJK_COMPATIBILITY_FORMS_value;
	ZVAL_LONG(&const_BLOCK_CODE_CJK_COMPATIBILITY_FORMS_value, UBLOCK_CJK_COMPATIBILITY_FORMS);
	crex_string *const_BLOCK_CODE_CJK_COMPATIBILITY_FORMS_name = crex_string_init_interned("BLOCK_CODE_CJK_COMPATIBILITY_FORMS", sizeof("BLOCK_CODE_CJK_COMPATIBILITY_FORMS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CJK_COMPATIBILITY_FORMS_name, &const_BLOCK_CODE_CJK_COMPATIBILITY_FORMS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CJK_COMPATIBILITY_FORMS_name);

	zval const_BLOCK_CODE_SMALL_FORM_VARIANTS_value;
	ZVAL_LONG(&const_BLOCK_CODE_SMALL_FORM_VARIANTS_value, UBLOCK_SMALL_FORM_VARIANTS);
	crex_string *const_BLOCK_CODE_SMALL_FORM_VARIANTS_name = crex_string_init_interned("BLOCK_CODE_SMALL_FORM_VARIANTS", sizeof("BLOCK_CODE_SMALL_FORM_VARIANTS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SMALL_FORM_VARIANTS_name, &const_BLOCK_CODE_SMALL_FORM_VARIANTS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SMALL_FORM_VARIANTS_name);

	zval const_BLOCK_CODE_ARABIC_PRESENTATION_FORMS_B_value;
	ZVAL_LONG(&const_BLOCK_CODE_ARABIC_PRESENTATION_FORMS_B_value, UBLOCK_ARABIC_PRESENTATION_FORMS_B);
	crex_string *const_BLOCK_CODE_ARABIC_PRESENTATION_FORMS_B_name = crex_string_init_interned("BLOCK_CODE_ARABIC_PRESENTATION_FORMS_B", sizeof("BLOCK_CODE_ARABIC_PRESENTATION_FORMS_B") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ARABIC_PRESENTATION_FORMS_B_name, &const_BLOCK_CODE_ARABIC_PRESENTATION_FORMS_B_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ARABIC_PRESENTATION_FORMS_B_name);

	zval const_BLOCK_CODE_SPECIALS_value;
	ZVAL_LONG(&const_BLOCK_CODE_SPECIALS_value, UBLOCK_SPECIALS);
	crex_string *const_BLOCK_CODE_SPECIALS_name = crex_string_init_interned("BLOCK_CODE_SPECIALS", sizeof("BLOCK_CODE_SPECIALS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SPECIALS_name, &const_BLOCK_CODE_SPECIALS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SPECIALS_name);

	zval const_BLOCK_CODE_HALFWIDTH_AND_FULLWIDTH_FORMS_value;
	ZVAL_LONG(&const_BLOCK_CODE_HALFWIDTH_AND_FULLWIDTH_FORMS_value, UBLOCK_HALFWIDTH_AND_FULLWIDTH_FORMS);
	crex_string *const_BLOCK_CODE_HALFWIDTH_AND_FULLWIDTH_FORMS_name = crex_string_init_interned("BLOCK_CODE_HALFWIDTH_AND_FULLWIDTH_FORMS", sizeof("BLOCK_CODE_HALFWIDTH_AND_FULLWIDTH_FORMS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_HALFWIDTH_AND_FULLWIDTH_FORMS_name, &const_BLOCK_CODE_HALFWIDTH_AND_FULLWIDTH_FORMS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_HALFWIDTH_AND_FULLWIDTH_FORMS_name);

	zval const_BLOCK_CODE_OLD_ITALIC_value;
	ZVAL_LONG(&const_BLOCK_CODE_OLD_ITALIC_value, UBLOCK_OLD_ITALIC);
	crex_string *const_BLOCK_CODE_OLD_ITALIC_name = crex_string_init_interned("BLOCK_CODE_OLD_ITALIC", sizeof("BLOCK_CODE_OLD_ITALIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_OLD_ITALIC_name, &const_BLOCK_CODE_OLD_ITALIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_OLD_ITALIC_name);

	zval const_BLOCK_CODE_GOTHIC_value;
	ZVAL_LONG(&const_BLOCK_CODE_GOTHIC_value, UBLOCK_GOTHIC);
	crex_string *const_BLOCK_CODE_GOTHIC_name = crex_string_init_interned("BLOCK_CODE_GOTHIC", sizeof("BLOCK_CODE_GOTHIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_GOTHIC_name, &const_BLOCK_CODE_GOTHIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_GOTHIC_name);

	zval const_BLOCK_CODE_DESERET_value;
	ZVAL_LONG(&const_BLOCK_CODE_DESERET_value, UBLOCK_DESERET);
	crex_string *const_BLOCK_CODE_DESERET_name = crex_string_init_interned("BLOCK_CODE_DESERET", sizeof("BLOCK_CODE_DESERET") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_DESERET_name, &const_BLOCK_CODE_DESERET_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_DESERET_name);

	zval const_BLOCK_CODE_BYZANTINE_MUSICAL_SYMBOLS_value;
	ZVAL_LONG(&const_BLOCK_CODE_BYZANTINE_MUSICAL_SYMBOLS_value, UBLOCK_BYZANTINE_MUSICAL_SYMBOLS);
	crex_string *const_BLOCK_CODE_BYZANTINE_MUSICAL_SYMBOLS_name = crex_string_init_interned("BLOCK_CODE_BYZANTINE_MUSICAL_SYMBOLS", sizeof("BLOCK_CODE_BYZANTINE_MUSICAL_SYMBOLS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_BYZANTINE_MUSICAL_SYMBOLS_name, &const_BLOCK_CODE_BYZANTINE_MUSICAL_SYMBOLS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_BYZANTINE_MUSICAL_SYMBOLS_name);

	zval const_BLOCK_CODE_MUSICAL_SYMBOLS_value;
	ZVAL_LONG(&const_BLOCK_CODE_MUSICAL_SYMBOLS_value, UBLOCK_MUSICAL_SYMBOLS);
	crex_string *const_BLOCK_CODE_MUSICAL_SYMBOLS_name = crex_string_init_interned("BLOCK_CODE_MUSICAL_SYMBOLS", sizeof("BLOCK_CODE_MUSICAL_SYMBOLS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MUSICAL_SYMBOLS_name, &const_BLOCK_CODE_MUSICAL_SYMBOLS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MUSICAL_SYMBOLS_name);

	zval const_BLOCK_CODE_MATHEMATICAL_ALPHANUMERIC_SYMBOLS_value;
	ZVAL_LONG(&const_BLOCK_CODE_MATHEMATICAL_ALPHANUMERIC_SYMBOLS_value, UBLOCK_MATHEMATICAL_ALPHANUMERIC_SYMBOLS);
	crex_string *const_BLOCK_CODE_MATHEMATICAL_ALPHANUMERIC_SYMBOLS_name = crex_string_init_interned("BLOCK_CODE_MATHEMATICAL_ALPHANUMERIC_SYMBOLS", sizeof("BLOCK_CODE_MATHEMATICAL_ALPHANUMERIC_SYMBOLS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MATHEMATICAL_ALPHANUMERIC_SYMBOLS_name, &const_BLOCK_CODE_MATHEMATICAL_ALPHANUMERIC_SYMBOLS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MATHEMATICAL_ALPHANUMERIC_SYMBOLS_name);

	zval const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B_value;
	ZVAL_LONG(&const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B_value, UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B);
	crex_string *const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B_name = crex_string_init_interned("BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B", sizeof("BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B_name, &const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B_name);

	zval const_BLOCK_CODE_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT_value;
	ZVAL_LONG(&const_BLOCK_CODE_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT_value, UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT);
	crex_string *const_BLOCK_CODE_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT_name = crex_string_init_interned("BLOCK_CODE_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT", sizeof("BLOCK_CODE_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT_name, &const_BLOCK_CODE_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT_name);

	zval const_BLOCK_CODE_TAGS_value;
	ZVAL_LONG(&const_BLOCK_CODE_TAGS_value, UBLOCK_TAGS);
	crex_string *const_BLOCK_CODE_TAGS_name = crex_string_init_interned("BLOCK_CODE_TAGS", sizeof("BLOCK_CODE_TAGS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_TAGS_name, &const_BLOCK_CODE_TAGS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_TAGS_name);

	zval const_BLOCK_CODE_CYRILLIC_SUPPLEMENT_value;
	ZVAL_LONG(&const_BLOCK_CODE_CYRILLIC_SUPPLEMENT_value, UBLOCK_CYRILLIC_SUPPLEMENT);
	crex_string *const_BLOCK_CODE_CYRILLIC_SUPPLEMENT_name = crex_string_init_interned("BLOCK_CODE_CYRILLIC_SUPPLEMENT", sizeof("BLOCK_CODE_CYRILLIC_SUPPLEMENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CYRILLIC_SUPPLEMENT_name, &const_BLOCK_CODE_CYRILLIC_SUPPLEMENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CYRILLIC_SUPPLEMENT_name);

	zval const_BLOCK_CODE_CYRILLIC_SUPPLEMENTARY_value;
	ZVAL_LONG(&const_BLOCK_CODE_CYRILLIC_SUPPLEMENTARY_value, UBLOCK_CYRILLIC_SUPPLEMENTARY);
	crex_string *const_BLOCK_CODE_CYRILLIC_SUPPLEMENTARY_name = crex_string_init_interned("BLOCK_CODE_CYRILLIC_SUPPLEMENTARY", sizeof("BLOCK_CODE_CYRILLIC_SUPPLEMENTARY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CYRILLIC_SUPPLEMENTARY_name, &const_BLOCK_CODE_CYRILLIC_SUPPLEMENTARY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CYRILLIC_SUPPLEMENTARY_name);

	zval const_BLOCK_CODE_TAGALOG_value;
	ZVAL_LONG(&const_BLOCK_CODE_TAGALOG_value, UBLOCK_TAGALOG);
	crex_string *const_BLOCK_CODE_TAGALOG_name = crex_string_init_interned("BLOCK_CODE_TAGALOG", sizeof("BLOCK_CODE_TAGALOG") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_TAGALOG_name, &const_BLOCK_CODE_TAGALOG_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_TAGALOG_name);

	zval const_BLOCK_CODE_HANUNOO_value;
	ZVAL_LONG(&const_BLOCK_CODE_HANUNOO_value, UBLOCK_HANUNOO);
	crex_string *const_BLOCK_CODE_HANUNOO_name = crex_string_init_interned("BLOCK_CODE_HANUNOO", sizeof("BLOCK_CODE_HANUNOO") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_HANUNOO_name, &const_BLOCK_CODE_HANUNOO_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_HANUNOO_name);

	zval const_BLOCK_CODE_BUHID_value;
	ZVAL_LONG(&const_BLOCK_CODE_BUHID_value, UBLOCK_BUHID);
	crex_string *const_BLOCK_CODE_BUHID_name = crex_string_init_interned("BLOCK_CODE_BUHID", sizeof("BLOCK_CODE_BUHID") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_BUHID_name, &const_BLOCK_CODE_BUHID_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_BUHID_name);

	zval const_BLOCK_CODE_TAGBANWA_value;
	ZVAL_LONG(&const_BLOCK_CODE_TAGBANWA_value, UBLOCK_TAGBANWA);
	crex_string *const_BLOCK_CODE_TAGBANWA_name = crex_string_init_interned("BLOCK_CODE_TAGBANWA", sizeof("BLOCK_CODE_TAGBANWA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_TAGBANWA_name, &const_BLOCK_CODE_TAGBANWA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_TAGBANWA_name);

	zval const_BLOCK_CODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A_value;
	ZVAL_LONG(&const_BLOCK_CODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A_value, UBLOCK_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A);
	crex_string *const_BLOCK_CODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A_name = crex_string_init_interned("BLOCK_CODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A", sizeof("BLOCK_CODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A_name, &const_BLOCK_CODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A_name);

	zval const_BLOCK_CODE_SUPPLEMENTAL_ARROWS_A_value;
	ZVAL_LONG(&const_BLOCK_CODE_SUPPLEMENTAL_ARROWS_A_value, UBLOCK_SUPPLEMENTAL_ARROWS_A);
	crex_string *const_BLOCK_CODE_SUPPLEMENTAL_ARROWS_A_name = crex_string_init_interned("BLOCK_CODE_SUPPLEMENTAL_ARROWS_A", sizeof("BLOCK_CODE_SUPPLEMENTAL_ARROWS_A") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SUPPLEMENTAL_ARROWS_A_name, &const_BLOCK_CODE_SUPPLEMENTAL_ARROWS_A_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SUPPLEMENTAL_ARROWS_A_name);

	zval const_BLOCK_CODE_SUPPLEMENTAL_ARROWS_B_value;
	ZVAL_LONG(&const_BLOCK_CODE_SUPPLEMENTAL_ARROWS_B_value, UBLOCK_SUPPLEMENTAL_ARROWS_B);
	crex_string *const_BLOCK_CODE_SUPPLEMENTAL_ARROWS_B_name = crex_string_init_interned("BLOCK_CODE_SUPPLEMENTAL_ARROWS_B", sizeof("BLOCK_CODE_SUPPLEMENTAL_ARROWS_B") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SUPPLEMENTAL_ARROWS_B_name, &const_BLOCK_CODE_SUPPLEMENTAL_ARROWS_B_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SUPPLEMENTAL_ARROWS_B_name);

	zval const_BLOCK_CODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B_value;
	ZVAL_LONG(&const_BLOCK_CODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B_value, UBLOCK_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B);
	crex_string *const_BLOCK_CODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B_name = crex_string_init_interned("BLOCK_CODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B", sizeof("BLOCK_CODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B_name, &const_BLOCK_CODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B_name);

	zval const_BLOCK_CODE_SUPPLEMENTAL_MATHEMATICAL_OPERATORS_value;
	ZVAL_LONG(&const_BLOCK_CODE_SUPPLEMENTAL_MATHEMATICAL_OPERATORS_value, UBLOCK_SUPPLEMENTAL_MATHEMATICAL_OPERATORS);
	crex_string *const_BLOCK_CODE_SUPPLEMENTAL_MATHEMATICAL_OPERATORS_name = crex_string_init_interned("BLOCK_CODE_SUPPLEMENTAL_MATHEMATICAL_OPERATORS", sizeof("BLOCK_CODE_SUPPLEMENTAL_MATHEMATICAL_OPERATORS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SUPPLEMENTAL_MATHEMATICAL_OPERATORS_name, &const_BLOCK_CODE_SUPPLEMENTAL_MATHEMATICAL_OPERATORS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SUPPLEMENTAL_MATHEMATICAL_OPERATORS_name);

	zval const_BLOCK_CODE_KATAKANA_PHONETIC_EXTENSIONS_value;
	ZVAL_LONG(&const_BLOCK_CODE_KATAKANA_PHONETIC_EXTENSIONS_value, UBLOCK_KATAKANA_PHONETIC_EXTENSIONS);
	crex_string *const_BLOCK_CODE_KATAKANA_PHONETIC_EXTENSIONS_name = crex_string_init_interned("BLOCK_CODE_KATAKANA_PHONETIC_EXTENSIONS", sizeof("BLOCK_CODE_KATAKANA_PHONETIC_EXTENSIONS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_KATAKANA_PHONETIC_EXTENSIONS_name, &const_BLOCK_CODE_KATAKANA_PHONETIC_EXTENSIONS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_KATAKANA_PHONETIC_EXTENSIONS_name);

	zval const_BLOCK_CODE_VARIATION_SELECTORS_value;
	ZVAL_LONG(&const_BLOCK_CODE_VARIATION_SELECTORS_value, UBLOCK_VARIATION_SELECTORS);
	crex_string *const_BLOCK_CODE_VARIATION_SELECTORS_name = crex_string_init_interned("BLOCK_CODE_VARIATION_SELECTORS", sizeof("BLOCK_CODE_VARIATION_SELECTORS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_VARIATION_SELECTORS_name, &const_BLOCK_CODE_VARIATION_SELECTORS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_VARIATION_SELECTORS_name);

	zval const_BLOCK_CODE_SUPPLEMENTARY_PRIVATE_USE_AREA_A_value;
	ZVAL_LONG(&const_BLOCK_CODE_SUPPLEMENTARY_PRIVATE_USE_AREA_A_value, UBLOCK_SUPPLEMENTARY_PRIVATE_USE_AREA_A);
	crex_string *const_BLOCK_CODE_SUPPLEMENTARY_PRIVATE_USE_AREA_A_name = crex_string_init_interned("BLOCK_CODE_SUPPLEMENTARY_PRIVATE_USE_AREA_A", sizeof("BLOCK_CODE_SUPPLEMENTARY_PRIVATE_USE_AREA_A") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SUPPLEMENTARY_PRIVATE_USE_AREA_A_name, &const_BLOCK_CODE_SUPPLEMENTARY_PRIVATE_USE_AREA_A_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SUPPLEMENTARY_PRIVATE_USE_AREA_A_name);

	zval const_BLOCK_CODE_SUPPLEMENTARY_PRIVATE_USE_AREA_B_value;
	ZVAL_LONG(&const_BLOCK_CODE_SUPPLEMENTARY_PRIVATE_USE_AREA_B_value, UBLOCK_SUPPLEMENTARY_PRIVATE_USE_AREA_B);
	crex_string *const_BLOCK_CODE_SUPPLEMENTARY_PRIVATE_USE_AREA_B_name = crex_string_init_interned("BLOCK_CODE_SUPPLEMENTARY_PRIVATE_USE_AREA_B", sizeof("BLOCK_CODE_SUPPLEMENTARY_PRIVATE_USE_AREA_B") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SUPPLEMENTARY_PRIVATE_USE_AREA_B_name, &const_BLOCK_CODE_SUPPLEMENTARY_PRIVATE_USE_AREA_B_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SUPPLEMENTARY_PRIVATE_USE_AREA_B_name);

	zval const_BLOCK_CODE_LIMBU_value;
	ZVAL_LONG(&const_BLOCK_CODE_LIMBU_value, UBLOCK_LIMBU);
	crex_string *const_BLOCK_CODE_LIMBU_name = crex_string_init_interned("BLOCK_CODE_LIMBU", sizeof("BLOCK_CODE_LIMBU") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_LIMBU_name, &const_BLOCK_CODE_LIMBU_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_LIMBU_name);

	zval const_BLOCK_CODE_TAI_LE_value;
	ZVAL_LONG(&const_BLOCK_CODE_TAI_LE_value, UBLOCK_TAI_LE);
	crex_string *const_BLOCK_CODE_TAI_LE_name = crex_string_init_interned("BLOCK_CODE_TAI_LE", sizeof("BLOCK_CODE_TAI_LE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_TAI_LE_name, &const_BLOCK_CODE_TAI_LE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_TAI_LE_name);

	zval const_BLOCK_CODE_KHMER_SYMBOLS_value;
	ZVAL_LONG(&const_BLOCK_CODE_KHMER_SYMBOLS_value, UBLOCK_KHMER_SYMBOLS);
	crex_string *const_BLOCK_CODE_KHMER_SYMBOLS_name = crex_string_init_interned("BLOCK_CODE_KHMER_SYMBOLS", sizeof("BLOCK_CODE_KHMER_SYMBOLS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_KHMER_SYMBOLS_name, &const_BLOCK_CODE_KHMER_SYMBOLS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_KHMER_SYMBOLS_name);

	zval const_BLOCK_CODE_PHONETIC_EXTENSIONS_value;
	ZVAL_LONG(&const_BLOCK_CODE_PHONETIC_EXTENSIONS_value, UBLOCK_PHONETIC_EXTENSIONS);
	crex_string *const_BLOCK_CODE_PHONETIC_EXTENSIONS_name = crex_string_init_interned("BLOCK_CODE_PHONETIC_EXTENSIONS", sizeof("BLOCK_CODE_PHONETIC_EXTENSIONS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_PHONETIC_EXTENSIONS_name, &const_BLOCK_CODE_PHONETIC_EXTENSIONS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_PHONETIC_EXTENSIONS_name);

	zval const_BLOCK_CODE_MISCELLANEOUS_SYMBOLS_AND_ARROWS_value;
	ZVAL_LONG(&const_BLOCK_CODE_MISCELLANEOUS_SYMBOLS_AND_ARROWS_value, UBLOCK_MISCELLANEOUS_SYMBOLS_AND_ARROWS);
	crex_string *const_BLOCK_CODE_MISCELLANEOUS_SYMBOLS_AND_ARROWS_name = crex_string_init_interned("BLOCK_CODE_MISCELLANEOUS_SYMBOLS_AND_ARROWS", sizeof("BLOCK_CODE_MISCELLANEOUS_SYMBOLS_AND_ARROWS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MISCELLANEOUS_SYMBOLS_AND_ARROWS_name, &const_BLOCK_CODE_MISCELLANEOUS_SYMBOLS_AND_ARROWS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MISCELLANEOUS_SYMBOLS_AND_ARROWS_name);

	zval const_BLOCK_CODE_YIJING_HEXAGRAM_SYMBOLS_value;
	ZVAL_LONG(&const_BLOCK_CODE_YIJING_HEXAGRAM_SYMBOLS_value, UBLOCK_YIJING_HEXAGRAM_SYMBOLS);
	crex_string *const_BLOCK_CODE_YIJING_HEXAGRAM_SYMBOLS_name = crex_string_init_interned("BLOCK_CODE_YIJING_HEXAGRAM_SYMBOLS", sizeof("BLOCK_CODE_YIJING_HEXAGRAM_SYMBOLS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_YIJING_HEXAGRAM_SYMBOLS_name, &const_BLOCK_CODE_YIJING_HEXAGRAM_SYMBOLS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_YIJING_HEXAGRAM_SYMBOLS_name);

	zval const_BLOCK_CODE_LINEAR_B_SYLLABARY_value;
	ZVAL_LONG(&const_BLOCK_CODE_LINEAR_B_SYLLABARY_value, UBLOCK_LINEAR_B_SYLLABARY);
	crex_string *const_BLOCK_CODE_LINEAR_B_SYLLABARY_name = crex_string_init_interned("BLOCK_CODE_LINEAR_B_SYLLABARY", sizeof("BLOCK_CODE_LINEAR_B_SYLLABARY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_LINEAR_B_SYLLABARY_name, &const_BLOCK_CODE_LINEAR_B_SYLLABARY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_LINEAR_B_SYLLABARY_name);

	zval const_BLOCK_CODE_LINEAR_B_IDEOGRAMS_value;
	ZVAL_LONG(&const_BLOCK_CODE_LINEAR_B_IDEOGRAMS_value, UBLOCK_LINEAR_B_IDEOGRAMS);
	crex_string *const_BLOCK_CODE_LINEAR_B_IDEOGRAMS_name = crex_string_init_interned("BLOCK_CODE_LINEAR_B_IDEOGRAMS", sizeof("BLOCK_CODE_LINEAR_B_IDEOGRAMS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_LINEAR_B_IDEOGRAMS_name, &const_BLOCK_CODE_LINEAR_B_IDEOGRAMS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_LINEAR_B_IDEOGRAMS_name);

	zval const_BLOCK_CODE_AEGEAN_NUMBERS_value;
	ZVAL_LONG(&const_BLOCK_CODE_AEGEAN_NUMBERS_value, UBLOCK_AEGEAN_NUMBERS);
	crex_string *const_BLOCK_CODE_AEGEAN_NUMBERS_name = crex_string_init_interned("BLOCK_CODE_AEGEAN_NUMBERS", sizeof("BLOCK_CODE_AEGEAN_NUMBERS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_AEGEAN_NUMBERS_name, &const_BLOCK_CODE_AEGEAN_NUMBERS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_AEGEAN_NUMBERS_name);

	zval const_BLOCK_CODE_UGARITIC_value;
	ZVAL_LONG(&const_BLOCK_CODE_UGARITIC_value, UBLOCK_UGARITIC);
	crex_string *const_BLOCK_CODE_UGARITIC_name = crex_string_init_interned("BLOCK_CODE_UGARITIC", sizeof("BLOCK_CODE_UGARITIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_UGARITIC_name, &const_BLOCK_CODE_UGARITIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_UGARITIC_name);

	zval const_BLOCK_CODE_SHAVIAN_value;
	ZVAL_LONG(&const_BLOCK_CODE_SHAVIAN_value, UBLOCK_SHAVIAN);
	crex_string *const_BLOCK_CODE_SHAVIAN_name = crex_string_init_interned("BLOCK_CODE_SHAVIAN", sizeof("BLOCK_CODE_SHAVIAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SHAVIAN_name, &const_BLOCK_CODE_SHAVIAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SHAVIAN_name);

	zval const_BLOCK_CODE_OSMANYA_value;
	ZVAL_LONG(&const_BLOCK_CODE_OSMANYA_value, UBLOCK_OSMANYA);
	crex_string *const_BLOCK_CODE_OSMANYA_name = crex_string_init_interned("BLOCK_CODE_OSMANYA", sizeof("BLOCK_CODE_OSMANYA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_OSMANYA_name, &const_BLOCK_CODE_OSMANYA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_OSMANYA_name);

	zval const_BLOCK_CODE_CYPRIOT_SYLLABARY_value;
	ZVAL_LONG(&const_BLOCK_CODE_CYPRIOT_SYLLABARY_value, UBLOCK_CYPRIOT_SYLLABARY);
	crex_string *const_BLOCK_CODE_CYPRIOT_SYLLABARY_name = crex_string_init_interned("BLOCK_CODE_CYPRIOT_SYLLABARY", sizeof("BLOCK_CODE_CYPRIOT_SYLLABARY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CYPRIOT_SYLLABARY_name, &const_BLOCK_CODE_CYPRIOT_SYLLABARY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CYPRIOT_SYLLABARY_name);

	zval const_BLOCK_CODE_TAI_XUAN_JING_SYMBOLS_value;
	ZVAL_LONG(&const_BLOCK_CODE_TAI_XUAN_JING_SYMBOLS_value, UBLOCK_TAI_XUAN_JING_SYMBOLS);
	crex_string *const_BLOCK_CODE_TAI_XUAN_JING_SYMBOLS_name = crex_string_init_interned("BLOCK_CODE_TAI_XUAN_JING_SYMBOLS", sizeof("BLOCK_CODE_TAI_XUAN_JING_SYMBOLS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_TAI_XUAN_JING_SYMBOLS_name, &const_BLOCK_CODE_TAI_XUAN_JING_SYMBOLS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_TAI_XUAN_JING_SYMBOLS_name);

	zval const_BLOCK_CODE_VARIATION_SELECTORS_SUPPLEMENT_value;
	ZVAL_LONG(&const_BLOCK_CODE_VARIATION_SELECTORS_SUPPLEMENT_value, UBLOCK_VARIATION_SELECTORS_SUPPLEMENT);
	crex_string *const_BLOCK_CODE_VARIATION_SELECTORS_SUPPLEMENT_name = crex_string_init_interned("BLOCK_CODE_VARIATION_SELECTORS_SUPPLEMENT", sizeof("BLOCK_CODE_VARIATION_SELECTORS_SUPPLEMENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_VARIATION_SELECTORS_SUPPLEMENT_name, &const_BLOCK_CODE_VARIATION_SELECTORS_SUPPLEMENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_VARIATION_SELECTORS_SUPPLEMENT_name);

	zval const_BLOCK_CODE_ANCIENT_GREEK_MUSICAL_NOTATION_value;
	ZVAL_LONG(&const_BLOCK_CODE_ANCIENT_GREEK_MUSICAL_NOTATION_value, UBLOCK_ANCIENT_GREEK_MUSICAL_NOTATION);
	crex_string *const_BLOCK_CODE_ANCIENT_GREEK_MUSICAL_NOTATION_name = crex_string_init_interned("BLOCK_CODE_ANCIENT_GREEK_MUSICAL_NOTATION", sizeof("BLOCK_CODE_ANCIENT_GREEK_MUSICAL_NOTATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ANCIENT_GREEK_MUSICAL_NOTATION_name, &const_BLOCK_CODE_ANCIENT_GREEK_MUSICAL_NOTATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ANCIENT_GREEK_MUSICAL_NOTATION_name);

	zval const_BLOCK_CODE_ANCIENT_GREEK_NUMBERS_value;
	ZVAL_LONG(&const_BLOCK_CODE_ANCIENT_GREEK_NUMBERS_value, UBLOCK_ANCIENT_GREEK_NUMBERS);
	crex_string *const_BLOCK_CODE_ANCIENT_GREEK_NUMBERS_name = crex_string_init_interned("BLOCK_CODE_ANCIENT_GREEK_NUMBERS", sizeof("BLOCK_CODE_ANCIENT_GREEK_NUMBERS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ANCIENT_GREEK_NUMBERS_name, &const_BLOCK_CODE_ANCIENT_GREEK_NUMBERS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ANCIENT_GREEK_NUMBERS_name);

	zval const_BLOCK_CODE_ARABIC_SUPPLEMENT_value;
	ZVAL_LONG(&const_BLOCK_CODE_ARABIC_SUPPLEMENT_value, UBLOCK_ARABIC_SUPPLEMENT);
	crex_string *const_BLOCK_CODE_ARABIC_SUPPLEMENT_name = crex_string_init_interned("BLOCK_CODE_ARABIC_SUPPLEMENT", sizeof("BLOCK_CODE_ARABIC_SUPPLEMENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ARABIC_SUPPLEMENT_name, &const_BLOCK_CODE_ARABIC_SUPPLEMENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ARABIC_SUPPLEMENT_name);

	zval const_BLOCK_CODE_BUGINESE_value;
	ZVAL_LONG(&const_BLOCK_CODE_BUGINESE_value, UBLOCK_BUGINESE);
	crex_string *const_BLOCK_CODE_BUGINESE_name = crex_string_init_interned("BLOCK_CODE_BUGINESE", sizeof("BLOCK_CODE_BUGINESE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_BUGINESE_name, &const_BLOCK_CODE_BUGINESE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_BUGINESE_name);

	zval const_BLOCK_CODE_CJK_STROKES_value;
	ZVAL_LONG(&const_BLOCK_CODE_CJK_STROKES_value, UBLOCK_CJK_STROKES);
	crex_string *const_BLOCK_CODE_CJK_STROKES_name = crex_string_init_interned("BLOCK_CODE_CJK_STROKES", sizeof("BLOCK_CODE_CJK_STROKES") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CJK_STROKES_name, &const_BLOCK_CODE_CJK_STROKES_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CJK_STROKES_name);

	zval const_BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_SUPPLEMENT_value;
	ZVAL_LONG(&const_BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_SUPPLEMENT_value, UBLOCK_COMBINING_DIACRITICAL_MARKS_SUPPLEMENT);
	crex_string *const_BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_SUPPLEMENT_name = crex_string_init_interned("BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_SUPPLEMENT", sizeof("BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_SUPPLEMENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_SUPPLEMENT_name, &const_BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_SUPPLEMENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_SUPPLEMENT_name);

	zval const_BLOCK_CODE_COPTIC_value;
	ZVAL_LONG(&const_BLOCK_CODE_COPTIC_value, UBLOCK_COPTIC);
	crex_string *const_BLOCK_CODE_COPTIC_name = crex_string_init_interned("BLOCK_CODE_COPTIC", sizeof("BLOCK_CODE_COPTIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_COPTIC_name, &const_BLOCK_CODE_COPTIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_COPTIC_name);

	zval const_BLOCK_CODE_ETHIOPIC_EXTENDED_value;
	ZVAL_LONG(&const_BLOCK_CODE_ETHIOPIC_EXTENDED_value, UBLOCK_ETHIOPIC_EXTENDED);
	crex_string *const_BLOCK_CODE_ETHIOPIC_EXTENDED_name = crex_string_init_interned("BLOCK_CODE_ETHIOPIC_EXTENDED", sizeof("BLOCK_CODE_ETHIOPIC_EXTENDED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ETHIOPIC_EXTENDED_name, &const_BLOCK_CODE_ETHIOPIC_EXTENDED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ETHIOPIC_EXTENDED_name);

	zval const_BLOCK_CODE_ETHIOPIC_SUPPLEMENT_value;
	ZVAL_LONG(&const_BLOCK_CODE_ETHIOPIC_SUPPLEMENT_value, UBLOCK_ETHIOPIC_SUPPLEMENT);
	crex_string *const_BLOCK_CODE_ETHIOPIC_SUPPLEMENT_name = crex_string_init_interned("BLOCK_CODE_ETHIOPIC_SUPPLEMENT", sizeof("BLOCK_CODE_ETHIOPIC_SUPPLEMENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ETHIOPIC_SUPPLEMENT_name, &const_BLOCK_CODE_ETHIOPIC_SUPPLEMENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ETHIOPIC_SUPPLEMENT_name);

	zval const_BLOCK_CODE_GEORGIAN_SUPPLEMENT_value;
	ZVAL_LONG(&const_BLOCK_CODE_GEORGIAN_SUPPLEMENT_value, UBLOCK_GEORGIAN_SUPPLEMENT);
	crex_string *const_BLOCK_CODE_GEORGIAN_SUPPLEMENT_name = crex_string_init_interned("BLOCK_CODE_GEORGIAN_SUPPLEMENT", sizeof("BLOCK_CODE_GEORGIAN_SUPPLEMENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_GEORGIAN_SUPPLEMENT_name, &const_BLOCK_CODE_GEORGIAN_SUPPLEMENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_GEORGIAN_SUPPLEMENT_name);

	zval const_BLOCK_CODE_GLAGOLITIC_value;
	ZVAL_LONG(&const_BLOCK_CODE_GLAGOLITIC_value, UBLOCK_GLAGOLITIC);
	crex_string *const_BLOCK_CODE_GLAGOLITIC_name = crex_string_init_interned("BLOCK_CODE_GLAGOLITIC", sizeof("BLOCK_CODE_GLAGOLITIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_GLAGOLITIC_name, &const_BLOCK_CODE_GLAGOLITIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_GLAGOLITIC_name);

	zval const_BLOCK_CODE_KHAROSHTHI_value;
	ZVAL_LONG(&const_BLOCK_CODE_KHAROSHTHI_value, UBLOCK_KHAROSHTHI);
	crex_string *const_BLOCK_CODE_KHAROSHTHI_name = crex_string_init_interned("BLOCK_CODE_KHAROSHTHI", sizeof("BLOCK_CODE_KHAROSHTHI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_KHAROSHTHI_name, &const_BLOCK_CODE_KHAROSHTHI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_KHAROSHTHI_name);

	zval const_BLOCK_CODE_MODIFIER_TONE_LETTERS_value;
	ZVAL_LONG(&const_BLOCK_CODE_MODIFIER_TONE_LETTERS_value, UBLOCK_MODIFIER_TONE_LETTERS);
	crex_string *const_BLOCK_CODE_MODIFIER_TONE_LETTERS_name = crex_string_init_interned("BLOCK_CODE_MODIFIER_TONE_LETTERS", sizeof("BLOCK_CODE_MODIFIER_TONE_LETTERS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MODIFIER_TONE_LETTERS_name, &const_BLOCK_CODE_MODIFIER_TONE_LETTERS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MODIFIER_TONE_LETTERS_name);

	zval const_BLOCK_CODE_NEW_TAI_LUE_value;
	ZVAL_LONG(&const_BLOCK_CODE_NEW_TAI_LUE_value, UBLOCK_NEW_TAI_LUE);
	crex_string *const_BLOCK_CODE_NEW_TAI_LUE_name = crex_string_init_interned("BLOCK_CODE_NEW_TAI_LUE", sizeof("BLOCK_CODE_NEW_TAI_LUE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_NEW_TAI_LUE_name, &const_BLOCK_CODE_NEW_TAI_LUE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_NEW_TAI_LUE_name);

	zval const_BLOCK_CODE_OLD_PERSIAN_value;
	ZVAL_LONG(&const_BLOCK_CODE_OLD_PERSIAN_value, UBLOCK_OLD_PERSIAN);
	crex_string *const_BLOCK_CODE_OLD_PERSIAN_name = crex_string_init_interned("BLOCK_CODE_OLD_PERSIAN", sizeof("BLOCK_CODE_OLD_PERSIAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_OLD_PERSIAN_name, &const_BLOCK_CODE_OLD_PERSIAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_OLD_PERSIAN_name);

	zval const_BLOCK_CODE_PHONETIC_EXTENSIONS_SUPPLEMENT_value;
	ZVAL_LONG(&const_BLOCK_CODE_PHONETIC_EXTENSIONS_SUPPLEMENT_value, UBLOCK_PHONETIC_EXTENSIONS_SUPPLEMENT);
	crex_string *const_BLOCK_CODE_PHONETIC_EXTENSIONS_SUPPLEMENT_name = crex_string_init_interned("BLOCK_CODE_PHONETIC_EXTENSIONS_SUPPLEMENT", sizeof("BLOCK_CODE_PHONETIC_EXTENSIONS_SUPPLEMENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_PHONETIC_EXTENSIONS_SUPPLEMENT_name, &const_BLOCK_CODE_PHONETIC_EXTENSIONS_SUPPLEMENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_PHONETIC_EXTENSIONS_SUPPLEMENT_name);

	zval const_BLOCK_CODE_SUPPLEMENTAL_PUNCTUATION_value;
	ZVAL_LONG(&const_BLOCK_CODE_SUPPLEMENTAL_PUNCTUATION_value, UBLOCK_SUPPLEMENTAL_PUNCTUATION);
	crex_string *const_BLOCK_CODE_SUPPLEMENTAL_PUNCTUATION_name = crex_string_init_interned("BLOCK_CODE_SUPPLEMENTAL_PUNCTUATION", sizeof("BLOCK_CODE_SUPPLEMENTAL_PUNCTUATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SUPPLEMENTAL_PUNCTUATION_name, &const_BLOCK_CODE_SUPPLEMENTAL_PUNCTUATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SUPPLEMENTAL_PUNCTUATION_name);

	zval const_BLOCK_CODE_SYLOTI_NAGRI_value;
	ZVAL_LONG(&const_BLOCK_CODE_SYLOTI_NAGRI_value, UBLOCK_SYLOTI_NAGRI);
	crex_string *const_BLOCK_CODE_SYLOTI_NAGRI_name = crex_string_init_interned("BLOCK_CODE_SYLOTI_NAGRI", sizeof("BLOCK_CODE_SYLOTI_NAGRI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SYLOTI_NAGRI_name, &const_BLOCK_CODE_SYLOTI_NAGRI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SYLOTI_NAGRI_name);

	zval const_BLOCK_CODE_TIFINAGH_value;
	ZVAL_LONG(&const_BLOCK_CODE_TIFINAGH_value, UBLOCK_TIFINAGH);
	crex_string *const_BLOCK_CODE_TIFINAGH_name = crex_string_init_interned("BLOCK_CODE_TIFINAGH", sizeof("BLOCK_CODE_TIFINAGH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_TIFINAGH_name, &const_BLOCK_CODE_TIFINAGH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_TIFINAGH_name);

	zval const_BLOCK_CODE_VERTICAL_FORMS_value;
	ZVAL_LONG(&const_BLOCK_CODE_VERTICAL_FORMS_value, UBLOCK_VERTICAL_FORMS);
	crex_string *const_BLOCK_CODE_VERTICAL_FORMS_name = crex_string_init_interned("BLOCK_CODE_VERTICAL_FORMS", sizeof("BLOCK_CODE_VERTICAL_FORMS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_VERTICAL_FORMS_name, &const_BLOCK_CODE_VERTICAL_FORMS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_VERTICAL_FORMS_name);

	zval const_BLOCK_CODE_NKO_value;
	ZVAL_LONG(&const_BLOCK_CODE_NKO_value, UBLOCK_NKO);
	crex_string *const_BLOCK_CODE_NKO_name = crex_string_init_interned("BLOCK_CODE_NKO", sizeof("BLOCK_CODE_NKO") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_NKO_name, &const_BLOCK_CODE_NKO_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_NKO_name);

	zval const_BLOCK_CODE_BALINESE_value;
	ZVAL_LONG(&const_BLOCK_CODE_BALINESE_value, UBLOCK_BALINESE);
	crex_string *const_BLOCK_CODE_BALINESE_name = crex_string_init_interned("BLOCK_CODE_BALINESE", sizeof("BLOCK_CODE_BALINESE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_BALINESE_name, &const_BLOCK_CODE_BALINESE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_BALINESE_name);

	zval const_BLOCK_CODE_LATIN_EXTENDED_C_value;
	ZVAL_LONG(&const_BLOCK_CODE_LATIN_EXTENDED_C_value, UBLOCK_LATIN_EXTENDED_C);
	crex_string *const_BLOCK_CODE_LATIN_EXTENDED_C_name = crex_string_init_interned("BLOCK_CODE_LATIN_EXTENDED_C", sizeof("BLOCK_CODE_LATIN_EXTENDED_C") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_LATIN_EXTENDED_C_name, &const_BLOCK_CODE_LATIN_EXTENDED_C_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_LATIN_EXTENDED_C_name);

	zval const_BLOCK_CODE_LATIN_EXTENDED_D_value;
	ZVAL_LONG(&const_BLOCK_CODE_LATIN_EXTENDED_D_value, UBLOCK_LATIN_EXTENDED_D);
	crex_string *const_BLOCK_CODE_LATIN_EXTENDED_D_name = crex_string_init_interned("BLOCK_CODE_LATIN_EXTENDED_D", sizeof("BLOCK_CODE_LATIN_EXTENDED_D") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_LATIN_EXTENDED_D_name, &const_BLOCK_CODE_LATIN_EXTENDED_D_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_LATIN_EXTENDED_D_name);

	zval const_BLOCK_CODE_PHAGS_PA_value;
	ZVAL_LONG(&const_BLOCK_CODE_PHAGS_PA_value, UBLOCK_PHAGS_PA);
	crex_string *const_BLOCK_CODE_PHAGS_PA_name = crex_string_init_interned("BLOCK_CODE_PHAGS_PA", sizeof("BLOCK_CODE_PHAGS_PA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_PHAGS_PA_name, &const_BLOCK_CODE_PHAGS_PA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_PHAGS_PA_name);

	zval const_BLOCK_CODE_PHOENICIAN_value;
	ZVAL_LONG(&const_BLOCK_CODE_PHOENICIAN_value, UBLOCK_PHOENICIAN);
	crex_string *const_BLOCK_CODE_PHOENICIAN_name = crex_string_init_interned("BLOCK_CODE_PHOENICIAN", sizeof("BLOCK_CODE_PHOENICIAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_PHOENICIAN_name, &const_BLOCK_CODE_PHOENICIAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_PHOENICIAN_name);

	zval const_BLOCK_CODE_CUNEIFORM_value;
	ZVAL_LONG(&const_BLOCK_CODE_CUNEIFORM_value, UBLOCK_CUNEIFORM);
	crex_string *const_BLOCK_CODE_CUNEIFORM_name = crex_string_init_interned("BLOCK_CODE_CUNEIFORM", sizeof("BLOCK_CODE_CUNEIFORM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CUNEIFORM_name, &const_BLOCK_CODE_CUNEIFORM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CUNEIFORM_name);

	zval const_BLOCK_CODE_CUNEIFORM_NUMBERS_AND_PUNCTUATION_value;
	ZVAL_LONG(&const_BLOCK_CODE_CUNEIFORM_NUMBERS_AND_PUNCTUATION_value, UBLOCK_CUNEIFORM_NUMBERS_AND_PUNCTUATION);
	crex_string *const_BLOCK_CODE_CUNEIFORM_NUMBERS_AND_PUNCTUATION_name = crex_string_init_interned("BLOCK_CODE_CUNEIFORM_NUMBERS_AND_PUNCTUATION", sizeof("BLOCK_CODE_CUNEIFORM_NUMBERS_AND_PUNCTUATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CUNEIFORM_NUMBERS_AND_PUNCTUATION_name, &const_BLOCK_CODE_CUNEIFORM_NUMBERS_AND_PUNCTUATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CUNEIFORM_NUMBERS_AND_PUNCTUATION_name);

	zval const_BLOCK_CODE_COUNTING_ROD_NUMERALS_value;
	ZVAL_LONG(&const_BLOCK_CODE_COUNTING_ROD_NUMERALS_value, UBLOCK_COUNTING_ROD_NUMERALS);
	crex_string *const_BLOCK_CODE_COUNTING_ROD_NUMERALS_name = crex_string_init_interned("BLOCK_CODE_COUNTING_ROD_NUMERALS", sizeof("BLOCK_CODE_COUNTING_ROD_NUMERALS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_COUNTING_ROD_NUMERALS_name, &const_BLOCK_CODE_COUNTING_ROD_NUMERALS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_COUNTING_ROD_NUMERALS_name);

	zval const_BLOCK_CODE_SUNDANESE_value;
	ZVAL_LONG(&const_BLOCK_CODE_SUNDANESE_value, UBLOCK_SUNDANESE);
	crex_string *const_BLOCK_CODE_SUNDANESE_name = crex_string_init_interned("BLOCK_CODE_SUNDANESE", sizeof("BLOCK_CODE_SUNDANESE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SUNDANESE_name, &const_BLOCK_CODE_SUNDANESE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SUNDANESE_name);

	zval const_BLOCK_CODE_LEPCHA_value;
	ZVAL_LONG(&const_BLOCK_CODE_LEPCHA_value, UBLOCK_LEPCHA);
	crex_string *const_BLOCK_CODE_LEPCHA_name = crex_string_init_interned("BLOCK_CODE_LEPCHA", sizeof("BLOCK_CODE_LEPCHA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_LEPCHA_name, &const_BLOCK_CODE_LEPCHA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_LEPCHA_name);

	zval const_BLOCK_CODE_OL_CHIKI_value;
	ZVAL_LONG(&const_BLOCK_CODE_OL_CHIKI_value, UBLOCK_OL_CHIKI);
	crex_string *const_BLOCK_CODE_OL_CHIKI_name = crex_string_init_interned("BLOCK_CODE_OL_CHIKI", sizeof("BLOCK_CODE_OL_CHIKI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_OL_CHIKI_name, &const_BLOCK_CODE_OL_CHIKI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_OL_CHIKI_name);

	zval const_BLOCK_CODE_CYRILLIC_EXTENDED_A_value;
	ZVAL_LONG(&const_BLOCK_CODE_CYRILLIC_EXTENDED_A_value, UBLOCK_CYRILLIC_EXTENDED_A);
	crex_string *const_BLOCK_CODE_CYRILLIC_EXTENDED_A_name = crex_string_init_interned("BLOCK_CODE_CYRILLIC_EXTENDED_A", sizeof("BLOCK_CODE_CYRILLIC_EXTENDED_A") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CYRILLIC_EXTENDED_A_name, &const_BLOCK_CODE_CYRILLIC_EXTENDED_A_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CYRILLIC_EXTENDED_A_name);

	zval const_BLOCK_CODE_VAI_value;
	ZVAL_LONG(&const_BLOCK_CODE_VAI_value, UBLOCK_VAI);
	crex_string *const_BLOCK_CODE_VAI_name = crex_string_init_interned("BLOCK_CODE_VAI", sizeof("BLOCK_CODE_VAI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_VAI_name, &const_BLOCK_CODE_VAI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_VAI_name);

	zval const_BLOCK_CODE_CYRILLIC_EXTENDED_B_value;
	ZVAL_LONG(&const_BLOCK_CODE_CYRILLIC_EXTENDED_B_value, UBLOCK_CYRILLIC_EXTENDED_B);
	crex_string *const_BLOCK_CODE_CYRILLIC_EXTENDED_B_name = crex_string_init_interned("BLOCK_CODE_CYRILLIC_EXTENDED_B", sizeof("BLOCK_CODE_CYRILLIC_EXTENDED_B") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CYRILLIC_EXTENDED_B_name, &const_BLOCK_CODE_CYRILLIC_EXTENDED_B_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CYRILLIC_EXTENDED_B_name);

	zval const_BLOCK_CODE_SAURASHTRA_value;
	ZVAL_LONG(&const_BLOCK_CODE_SAURASHTRA_value, UBLOCK_SAURASHTRA);
	crex_string *const_BLOCK_CODE_SAURASHTRA_name = crex_string_init_interned("BLOCK_CODE_SAURASHTRA", sizeof("BLOCK_CODE_SAURASHTRA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SAURASHTRA_name, &const_BLOCK_CODE_SAURASHTRA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SAURASHTRA_name);

	zval const_BLOCK_CODE_KAYAH_LI_value;
	ZVAL_LONG(&const_BLOCK_CODE_KAYAH_LI_value, UBLOCK_KAYAH_LI);
	crex_string *const_BLOCK_CODE_KAYAH_LI_name = crex_string_init_interned("BLOCK_CODE_KAYAH_LI", sizeof("BLOCK_CODE_KAYAH_LI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_KAYAH_LI_name, &const_BLOCK_CODE_KAYAH_LI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_KAYAH_LI_name);

	zval const_BLOCK_CODE_REJANG_value;
	ZVAL_LONG(&const_BLOCK_CODE_REJANG_value, UBLOCK_REJANG);
	crex_string *const_BLOCK_CODE_REJANG_name = crex_string_init_interned("BLOCK_CODE_REJANG", sizeof("BLOCK_CODE_REJANG") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_REJANG_name, &const_BLOCK_CODE_REJANG_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_REJANG_name);

	zval const_BLOCK_CODE_CHAM_value;
	ZVAL_LONG(&const_BLOCK_CODE_CHAM_value, UBLOCK_CHAM);
	crex_string *const_BLOCK_CODE_CHAM_name = crex_string_init_interned("BLOCK_CODE_CHAM", sizeof("BLOCK_CODE_CHAM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CHAM_name, &const_BLOCK_CODE_CHAM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CHAM_name);

	zval const_BLOCK_CODE_ANCIENT_SYMBOLS_value;
	ZVAL_LONG(&const_BLOCK_CODE_ANCIENT_SYMBOLS_value, UBLOCK_ANCIENT_SYMBOLS);
	crex_string *const_BLOCK_CODE_ANCIENT_SYMBOLS_name = crex_string_init_interned("BLOCK_CODE_ANCIENT_SYMBOLS", sizeof("BLOCK_CODE_ANCIENT_SYMBOLS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ANCIENT_SYMBOLS_name, &const_BLOCK_CODE_ANCIENT_SYMBOLS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ANCIENT_SYMBOLS_name);

	zval const_BLOCK_CODE_PHAISTOS_DISC_value;
	ZVAL_LONG(&const_BLOCK_CODE_PHAISTOS_DISC_value, UBLOCK_PHAISTOS_DISC);
	crex_string *const_BLOCK_CODE_PHAISTOS_DISC_name = crex_string_init_interned("BLOCK_CODE_PHAISTOS_DISC", sizeof("BLOCK_CODE_PHAISTOS_DISC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_PHAISTOS_DISC_name, &const_BLOCK_CODE_PHAISTOS_DISC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_PHAISTOS_DISC_name);

	zval const_BLOCK_CODE_LYCIAN_value;
	ZVAL_LONG(&const_BLOCK_CODE_LYCIAN_value, UBLOCK_LYCIAN);
	crex_string *const_BLOCK_CODE_LYCIAN_name = crex_string_init_interned("BLOCK_CODE_LYCIAN", sizeof("BLOCK_CODE_LYCIAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_LYCIAN_name, &const_BLOCK_CODE_LYCIAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_LYCIAN_name);

	zval const_BLOCK_CODE_CARIAN_value;
	ZVAL_LONG(&const_BLOCK_CODE_CARIAN_value, UBLOCK_CARIAN);
	crex_string *const_BLOCK_CODE_CARIAN_name = crex_string_init_interned("BLOCK_CODE_CARIAN", sizeof("BLOCK_CODE_CARIAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CARIAN_name, &const_BLOCK_CODE_CARIAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CARIAN_name);

	zval const_BLOCK_CODE_LYDIAN_value;
	ZVAL_LONG(&const_BLOCK_CODE_LYDIAN_value, UBLOCK_LYDIAN);
	crex_string *const_BLOCK_CODE_LYDIAN_name = crex_string_init_interned("BLOCK_CODE_LYDIAN", sizeof("BLOCK_CODE_LYDIAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_LYDIAN_name, &const_BLOCK_CODE_LYDIAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_LYDIAN_name);

	zval const_BLOCK_CODE_MAHJONG_TILES_value;
	ZVAL_LONG(&const_BLOCK_CODE_MAHJONG_TILES_value, UBLOCK_MAHJONG_TILES);
	crex_string *const_BLOCK_CODE_MAHJONG_TILES_name = crex_string_init_interned("BLOCK_CODE_MAHJONG_TILES", sizeof("BLOCK_CODE_MAHJONG_TILES") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MAHJONG_TILES_name, &const_BLOCK_CODE_MAHJONG_TILES_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MAHJONG_TILES_name);

	zval const_BLOCK_CODE_DOMINO_TILES_value;
	ZVAL_LONG(&const_BLOCK_CODE_DOMINO_TILES_value, UBLOCK_DOMINO_TILES);
	crex_string *const_BLOCK_CODE_DOMINO_TILES_name = crex_string_init_interned("BLOCK_CODE_DOMINO_TILES", sizeof("BLOCK_CODE_DOMINO_TILES") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_DOMINO_TILES_name, &const_BLOCK_CODE_DOMINO_TILES_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_DOMINO_TILES_name);

	zval const_BLOCK_CODE_SAMARITAN_value;
	ZVAL_LONG(&const_BLOCK_CODE_SAMARITAN_value, UBLOCK_SAMARITAN);
	crex_string *const_BLOCK_CODE_SAMARITAN_name = crex_string_init_interned("BLOCK_CODE_SAMARITAN", sizeof("BLOCK_CODE_SAMARITAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SAMARITAN_name, &const_BLOCK_CODE_SAMARITAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SAMARITAN_name);

	zval const_BLOCK_CODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED_value;
	ZVAL_LONG(&const_BLOCK_CODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED_value, UBLOCK_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED);
	crex_string *const_BLOCK_CODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED_name = crex_string_init_interned("BLOCK_CODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED", sizeof("BLOCK_CODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED_name, &const_BLOCK_CODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED_name);

	zval const_BLOCK_CODE_TAI_THAM_value;
	ZVAL_LONG(&const_BLOCK_CODE_TAI_THAM_value, UBLOCK_TAI_THAM);
	crex_string *const_BLOCK_CODE_TAI_THAM_name = crex_string_init_interned("BLOCK_CODE_TAI_THAM", sizeof("BLOCK_CODE_TAI_THAM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_TAI_THAM_name, &const_BLOCK_CODE_TAI_THAM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_TAI_THAM_name);

	zval const_BLOCK_CODE_VEDIC_EXTENSIONS_value;
	ZVAL_LONG(&const_BLOCK_CODE_VEDIC_EXTENSIONS_value, UBLOCK_VEDIC_EXTENSIONS);
	crex_string *const_BLOCK_CODE_VEDIC_EXTENSIONS_name = crex_string_init_interned("BLOCK_CODE_VEDIC_EXTENSIONS", sizeof("BLOCK_CODE_VEDIC_EXTENSIONS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_VEDIC_EXTENSIONS_name, &const_BLOCK_CODE_VEDIC_EXTENSIONS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_VEDIC_EXTENSIONS_name);

	zval const_BLOCK_CODE_LISU_value;
	ZVAL_LONG(&const_BLOCK_CODE_LISU_value, UBLOCK_LISU);
	crex_string *const_BLOCK_CODE_LISU_name = crex_string_init_interned("BLOCK_CODE_LISU", sizeof("BLOCK_CODE_LISU") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_LISU_name, &const_BLOCK_CODE_LISU_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_LISU_name);

	zval const_BLOCK_CODE_BAMUM_value;
	ZVAL_LONG(&const_BLOCK_CODE_BAMUM_value, UBLOCK_BAMUM);
	crex_string *const_BLOCK_CODE_BAMUM_name = crex_string_init_interned("BLOCK_CODE_BAMUM", sizeof("BLOCK_CODE_BAMUM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_BAMUM_name, &const_BLOCK_CODE_BAMUM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_BAMUM_name);

	zval const_BLOCK_CODE_COMMON_INDIC_NUMBER_FORMS_value;
	ZVAL_LONG(&const_BLOCK_CODE_COMMON_INDIC_NUMBER_FORMS_value, UBLOCK_COMMON_INDIC_NUMBER_FORMS);
	crex_string *const_BLOCK_CODE_COMMON_INDIC_NUMBER_FORMS_name = crex_string_init_interned("BLOCK_CODE_COMMON_INDIC_NUMBER_FORMS", sizeof("BLOCK_CODE_COMMON_INDIC_NUMBER_FORMS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_COMMON_INDIC_NUMBER_FORMS_name, &const_BLOCK_CODE_COMMON_INDIC_NUMBER_FORMS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_COMMON_INDIC_NUMBER_FORMS_name);

	zval const_BLOCK_CODE_DEVANAGARI_EXTENDED_value;
	ZVAL_LONG(&const_BLOCK_CODE_DEVANAGARI_EXTENDED_value, UBLOCK_DEVANAGARI_EXTENDED);
	crex_string *const_BLOCK_CODE_DEVANAGARI_EXTENDED_name = crex_string_init_interned("BLOCK_CODE_DEVANAGARI_EXTENDED", sizeof("BLOCK_CODE_DEVANAGARI_EXTENDED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_DEVANAGARI_EXTENDED_name, &const_BLOCK_CODE_DEVANAGARI_EXTENDED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_DEVANAGARI_EXTENDED_name);

	zval const_BLOCK_CODE_HANGUL_JAMO_EXTENDED_A_value;
	ZVAL_LONG(&const_BLOCK_CODE_HANGUL_JAMO_EXTENDED_A_value, UBLOCK_HANGUL_JAMO_EXTENDED_A);
	crex_string *const_BLOCK_CODE_HANGUL_JAMO_EXTENDED_A_name = crex_string_init_interned("BLOCK_CODE_HANGUL_JAMO_EXTENDED_A", sizeof("BLOCK_CODE_HANGUL_JAMO_EXTENDED_A") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_HANGUL_JAMO_EXTENDED_A_name, &const_BLOCK_CODE_HANGUL_JAMO_EXTENDED_A_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_HANGUL_JAMO_EXTENDED_A_name);

	zval const_BLOCK_CODE_JAVANESE_value;
	ZVAL_LONG(&const_BLOCK_CODE_JAVANESE_value, UBLOCK_JAVANESE);
	crex_string *const_BLOCK_CODE_JAVANESE_name = crex_string_init_interned("BLOCK_CODE_JAVANESE", sizeof("BLOCK_CODE_JAVANESE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_JAVANESE_name, &const_BLOCK_CODE_JAVANESE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_JAVANESE_name);

	zval const_BLOCK_CODE_MYANMAR_EXTENDED_A_value;
	ZVAL_LONG(&const_BLOCK_CODE_MYANMAR_EXTENDED_A_value, UBLOCK_MYANMAR_EXTENDED_A);
	crex_string *const_BLOCK_CODE_MYANMAR_EXTENDED_A_name = crex_string_init_interned("BLOCK_CODE_MYANMAR_EXTENDED_A", sizeof("BLOCK_CODE_MYANMAR_EXTENDED_A") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MYANMAR_EXTENDED_A_name, &const_BLOCK_CODE_MYANMAR_EXTENDED_A_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MYANMAR_EXTENDED_A_name);

	zval const_BLOCK_CODE_TAI_VIET_value;
	ZVAL_LONG(&const_BLOCK_CODE_TAI_VIET_value, UBLOCK_TAI_VIET);
	crex_string *const_BLOCK_CODE_TAI_VIET_name = crex_string_init_interned("BLOCK_CODE_TAI_VIET", sizeof("BLOCK_CODE_TAI_VIET") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_TAI_VIET_name, &const_BLOCK_CODE_TAI_VIET_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_TAI_VIET_name);

	zval const_BLOCK_CODE_MEETEI_MAYEK_value;
	ZVAL_LONG(&const_BLOCK_CODE_MEETEI_MAYEK_value, UBLOCK_MEETEI_MAYEK);
	crex_string *const_BLOCK_CODE_MEETEI_MAYEK_name = crex_string_init_interned("BLOCK_CODE_MEETEI_MAYEK", sizeof("BLOCK_CODE_MEETEI_MAYEK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MEETEI_MAYEK_name, &const_BLOCK_CODE_MEETEI_MAYEK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MEETEI_MAYEK_name);

	zval const_BLOCK_CODE_HANGUL_JAMO_EXTENDED_B_value;
	ZVAL_LONG(&const_BLOCK_CODE_HANGUL_JAMO_EXTENDED_B_value, UBLOCK_HANGUL_JAMO_EXTENDED_B);
	crex_string *const_BLOCK_CODE_HANGUL_JAMO_EXTENDED_B_name = crex_string_init_interned("BLOCK_CODE_HANGUL_JAMO_EXTENDED_B", sizeof("BLOCK_CODE_HANGUL_JAMO_EXTENDED_B") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_HANGUL_JAMO_EXTENDED_B_name, &const_BLOCK_CODE_HANGUL_JAMO_EXTENDED_B_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_HANGUL_JAMO_EXTENDED_B_name);

	zval const_BLOCK_CODE_IMPERIAL_ARAMAIC_value;
	ZVAL_LONG(&const_BLOCK_CODE_IMPERIAL_ARAMAIC_value, UBLOCK_IMPERIAL_ARAMAIC);
	crex_string *const_BLOCK_CODE_IMPERIAL_ARAMAIC_name = crex_string_init_interned("BLOCK_CODE_IMPERIAL_ARAMAIC", sizeof("BLOCK_CODE_IMPERIAL_ARAMAIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_IMPERIAL_ARAMAIC_name, &const_BLOCK_CODE_IMPERIAL_ARAMAIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_IMPERIAL_ARAMAIC_name);

	zval const_BLOCK_CODE_OLD_SOUTH_ARABIAN_value;
	ZVAL_LONG(&const_BLOCK_CODE_OLD_SOUTH_ARABIAN_value, UBLOCK_OLD_SOUTH_ARABIAN);
	crex_string *const_BLOCK_CODE_OLD_SOUTH_ARABIAN_name = crex_string_init_interned("BLOCK_CODE_OLD_SOUTH_ARABIAN", sizeof("BLOCK_CODE_OLD_SOUTH_ARABIAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_OLD_SOUTH_ARABIAN_name, &const_BLOCK_CODE_OLD_SOUTH_ARABIAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_OLD_SOUTH_ARABIAN_name);

	zval const_BLOCK_CODE_AVESTAN_value;
	ZVAL_LONG(&const_BLOCK_CODE_AVESTAN_value, UBLOCK_AVESTAN);
	crex_string *const_BLOCK_CODE_AVESTAN_name = crex_string_init_interned("BLOCK_CODE_AVESTAN", sizeof("BLOCK_CODE_AVESTAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_AVESTAN_name, &const_BLOCK_CODE_AVESTAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_AVESTAN_name);

	zval const_BLOCK_CODE_INSCRIPTIONAL_PARTHIAN_value;
	ZVAL_LONG(&const_BLOCK_CODE_INSCRIPTIONAL_PARTHIAN_value, UBLOCK_INSCRIPTIONAL_PARTHIAN);
	crex_string *const_BLOCK_CODE_INSCRIPTIONAL_PARTHIAN_name = crex_string_init_interned("BLOCK_CODE_INSCRIPTIONAL_PARTHIAN", sizeof("BLOCK_CODE_INSCRIPTIONAL_PARTHIAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_INSCRIPTIONAL_PARTHIAN_name, &const_BLOCK_CODE_INSCRIPTIONAL_PARTHIAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_INSCRIPTIONAL_PARTHIAN_name);

	zval const_BLOCK_CODE_INSCRIPTIONAL_PAHLAVI_value;
	ZVAL_LONG(&const_BLOCK_CODE_INSCRIPTIONAL_PAHLAVI_value, UBLOCK_INSCRIPTIONAL_PAHLAVI);
	crex_string *const_BLOCK_CODE_INSCRIPTIONAL_PAHLAVI_name = crex_string_init_interned("BLOCK_CODE_INSCRIPTIONAL_PAHLAVI", sizeof("BLOCK_CODE_INSCRIPTIONAL_PAHLAVI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_INSCRIPTIONAL_PAHLAVI_name, &const_BLOCK_CODE_INSCRIPTIONAL_PAHLAVI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_INSCRIPTIONAL_PAHLAVI_name);

	zval const_BLOCK_CODE_OLD_TURKIC_value;
	ZVAL_LONG(&const_BLOCK_CODE_OLD_TURKIC_value, UBLOCK_OLD_TURKIC);
	crex_string *const_BLOCK_CODE_OLD_TURKIC_name = crex_string_init_interned("BLOCK_CODE_OLD_TURKIC", sizeof("BLOCK_CODE_OLD_TURKIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_OLD_TURKIC_name, &const_BLOCK_CODE_OLD_TURKIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_OLD_TURKIC_name);

	zval const_BLOCK_CODE_RUMI_NUMERAL_SYMBOLS_value;
	ZVAL_LONG(&const_BLOCK_CODE_RUMI_NUMERAL_SYMBOLS_value, UBLOCK_RUMI_NUMERAL_SYMBOLS);
	crex_string *const_BLOCK_CODE_RUMI_NUMERAL_SYMBOLS_name = crex_string_init_interned("BLOCK_CODE_RUMI_NUMERAL_SYMBOLS", sizeof("BLOCK_CODE_RUMI_NUMERAL_SYMBOLS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_RUMI_NUMERAL_SYMBOLS_name, &const_BLOCK_CODE_RUMI_NUMERAL_SYMBOLS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_RUMI_NUMERAL_SYMBOLS_name);

	zval const_BLOCK_CODE_KAITHI_value;
	ZVAL_LONG(&const_BLOCK_CODE_KAITHI_value, UBLOCK_KAITHI);
	crex_string *const_BLOCK_CODE_KAITHI_name = crex_string_init_interned("BLOCK_CODE_KAITHI", sizeof("BLOCK_CODE_KAITHI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_KAITHI_name, &const_BLOCK_CODE_KAITHI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_KAITHI_name);

	zval const_BLOCK_CODE_EGYPTIAN_HIEROGLYPHS_value;
	ZVAL_LONG(&const_BLOCK_CODE_EGYPTIAN_HIEROGLYPHS_value, UBLOCK_EGYPTIAN_HIEROGLYPHS);
	crex_string *const_BLOCK_CODE_EGYPTIAN_HIEROGLYPHS_name = crex_string_init_interned("BLOCK_CODE_EGYPTIAN_HIEROGLYPHS", sizeof("BLOCK_CODE_EGYPTIAN_HIEROGLYPHS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_EGYPTIAN_HIEROGLYPHS_name, &const_BLOCK_CODE_EGYPTIAN_HIEROGLYPHS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_EGYPTIAN_HIEROGLYPHS_name);

	zval const_BLOCK_CODE_ENCLOSED_ALPHANUMERIC_SUPPLEMENT_value;
	ZVAL_LONG(&const_BLOCK_CODE_ENCLOSED_ALPHANUMERIC_SUPPLEMENT_value, UBLOCK_ENCLOSED_ALPHANUMERIC_SUPPLEMENT);
	crex_string *const_BLOCK_CODE_ENCLOSED_ALPHANUMERIC_SUPPLEMENT_name = crex_string_init_interned("BLOCK_CODE_ENCLOSED_ALPHANUMERIC_SUPPLEMENT", sizeof("BLOCK_CODE_ENCLOSED_ALPHANUMERIC_SUPPLEMENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ENCLOSED_ALPHANUMERIC_SUPPLEMENT_name, &const_BLOCK_CODE_ENCLOSED_ALPHANUMERIC_SUPPLEMENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ENCLOSED_ALPHANUMERIC_SUPPLEMENT_name);

	zval const_BLOCK_CODE_ENCLOSED_IDEOGRAPHIC_SUPPLEMENT_value;
	ZVAL_LONG(&const_BLOCK_CODE_ENCLOSED_IDEOGRAPHIC_SUPPLEMENT_value, UBLOCK_ENCLOSED_IDEOGRAPHIC_SUPPLEMENT);
	crex_string *const_BLOCK_CODE_ENCLOSED_IDEOGRAPHIC_SUPPLEMENT_name = crex_string_init_interned("BLOCK_CODE_ENCLOSED_IDEOGRAPHIC_SUPPLEMENT", sizeof("BLOCK_CODE_ENCLOSED_IDEOGRAPHIC_SUPPLEMENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ENCLOSED_IDEOGRAPHIC_SUPPLEMENT_name, &const_BLOCK_CODE_ENCLOSED_IDEOGRAPHIC_SUPPLEMENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ENCLOSED_IDEOGRAPHIC_SUPPLEMENT_name);

	zval const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C_value;
	ZVAL_LONG(&const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C_value, UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C);
	crex_string *const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C_name = crex_string_init_interned("BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C", sizeof("BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C_name, &const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C_name);

	zval const_BLOCK_CODE_MANDAIC_value;
	ZVAL_LONG(&const_BLOCK_CODE_MANDAIC_value, UBLOCK_MANDAIC);
	crex_string *const_BLOCK_CODE_MANDAIC_name = crex_string_init_interned("BLOCK_CODE_MANDAIC", sizeof("BLOCK_CODE_MANDAIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MANDAIC_name, &const_BLOCK_CODE_MANDAIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MANDAIC_name);

	zval const_BLOCK_CODE_BATAK_value;
	ZVAL_LONG(&const_BLOCK_CODE_BATAK_value, UBLOCK_BATAK);
	crex_string *const_BLOCK_CODE_BATAK_name = crex_string_init_interned("BLOCK_CODE_BATAK", sizeof("BLOCK_CODE_BATAK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_BATAK_name, &const_BLOCK_CODE_BATAK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_BATAK_name);

	zval const_BLOCK_CODE_ETHIOPIC_EXTENDED_A_value;
	ZVAL_LONG(&const_BLOCK_CODE_ETHIOPIC_EXTENDED_A_value, UBLOCK_ETHIOPIC_EXTENDED_A);
	crex_string *const_BLOCK_CODE_ETHIOPIC_EXTENDED_A_name = crex_string_init_interned("BLOCK_CODE_ETHIOPIC_EXTENDED_A", sizeof("BLOCK_CODE_ETHIOPIC_EXTENDED_A") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ETHIOPIC_EXTENDED_A_name, &const_BLOCK_CODE_ETHIOPIC_EXTENDED_A_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ETHIOPIC_EXTENDED_A_name);

	zval const_BLOCK_CODE_BRAHMI_value;
	ZVAL_LONG(&const_BLOCK_CODE_BRAHMI_value, UBLOCK_BRAHMI);
	crex_string *const_BLOCK_CODE_BRAHMI_name = crex_string_init_interned("BLOCK_CODE_BRAHMI", sizeof("BLOCK_CODE_BRAHMI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_BRAHMI_name, &const_BLOCK_CODE_BRAHMI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_BRAHMI_name);

	zval const_BLOCK_CODE_BAMUM_SUPPLEMENT_value;
	ZVAL_LONG(&const_BLOCK_CODE_BAMUM_SUPPLEMENT_value, UBLOCK_BAMUM_SUPPLEMENT);
	crex_string *const_BLOCK_CODE_BAMUM_SUPPLEMENT_name = crex_string_init_interned("BLOCK_CODE_BAMUM_SUPPLEMENT", sizeof("BLOCK_CODE_BAMUM_SUPPLEMENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_BAMUM_SUPPLEMENT_name, &const_BLOCK_CODE_BAMUM_SUPPLEMENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_BAMUM_SUPPLEMENT_name);

	zval const_BLOCK_CODE_KANA_SUPPLEMENT_value;
	ZVAL_LONG(&const_BLOCK_CODE_KANA_SUPPLEMENT_value, UBLOCK_KANA_SUPPLEMENT);
	crex_string *const_BLOCK_CODE_KANA_SUPPLEMENT_name = crex_string_init_interned("BLOCK_CODE_KANA_SUPPLEMENT", sizeof("BLOCK_CODE_KANA_SUPPLEMENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_KANA_SUPPLEMENT_name, &const_BLOCK_CODE_KANA_SUPPLEMENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_KANA_SUPPLEMENT_name);

	zval const_BLOCK_CODE_PLAYING_CARDS_value;
	ZVAL_LONG(&const_BLOCK_CODE_PLAYING_CARDS_value, UBLOCK_PLAYING_CARDS);
	crex_string *const_BLOCK_CODE_PLAYING_CARDS_name = crex_string_init_interned("BLOCK_CODE_PLAYING_CARDS", sizeof("BLOCK_CODE_PLAYING_CARDS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_PLAYING_CARDS_name, &const_BLOCK_CODE_PLAYING_CARDS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_PLAYING_CARDS_name);

	zval const_BLOCK_CODE_MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS_value;
	ZVAL_LONG(&const_BLOCK_CODE_MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS_value, UBLOCK_MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS);
	crex_string *const_BLOCK_CODE_MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS_name = crex_string_init_interned("BLOCK_CODE_MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS", sizeof("BLOCK_CODE_MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS_name, &const_BLOCK_CODE_MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS_name);

	zval const_BLOCK_CODE_EMOTICONS_value;
	ZVAL_LONG(&const_BLOCK_CODE_EMOTICONS_value, UBLOCK_EMOTICONS);
	crex_string *const_BLOCK_CODE_EMOTICONS_name = crex_string_init_interned("BLOCK_CODE_EMOTICONS", sizeof("BLOCK_CODE_EMOTICONS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_EMOTICONS_name, &const_BLOCK_CODE_EMOTICONS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_EMOTICONS_name);

	zval const_BLOCK_CODE_TRANSPORT_AND_MAP_SYMBOLS_value;
	ZVAL_LONG(&const_BLOCK_CODE_TRANSPORT_AND_MAP_SYMBOLS_value, UBLOCK_TRANSPORT_AND_MAP_SYMBOLS);
	crex_string *const_BLOCK_CODE_TRANSPORT_AND_MAP_SYMBOLS_name = crex_string_init_interned("BLOCK_CODE_TRANSPORT_AND_MAP_SYMBOLS", sizeof("BLOCK_CODE_TRANSPORT_AND_MAP_SYMBOLS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_TRANSPORT_AND_MAP_SYMBOLS_name, &const_BLOCK_CODE_TRANSPORT_AND_MAP_SYMBOLS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_TRANSPORT_AND_MAP_SYMBOLS_name);

	zval const_BLOCK_CODE_ALCHEMICAL_SYMBOLS_value;
	ZVAL_LONG(&const_BLOCK_CODE_ALCHEMICAL_SYMBOLS_value, UBLOCK_ALCHEMICAL_SYMBOLS);
	crex_string *const_BLOCK_CODE_ALCHEMICAL_SYMBOLS_name = crex_string_init_interned("BLOCK_CODE_ALCHEMICAL_SYMBOLS", sizeof("BLOCK_CODE_ALCHEMICAL_SYMBOLS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ALCHEMICAL_SYMBOLS_name, &const_BLOCK_CODE_ALCHEMICAL_SYMBOLS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ALCHEMICAL_SYMBOLS_name);

	zval const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D_value;
	ZVAL_LONG(&const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D_value, UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D);
	crex_string *const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D_name = crex_string_init_interned("BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D", sizeof("BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D_name, &const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D_name);

	zval const_BLOCK_CODE_ARABIC_EXTENDED_A_value;
	ZVAL_LONG(&const_BLOCK_CODE_ARABIC_EXTENDED_A_value, UBLOCK_ARABIC_EXTENDED_A);
	crex_string *const_BLOCK_CODE_ARABIC_EXTENDED_A_name = crex_string_init_interned("BLOCK_CODE_ARABIC_EXTENDED_A", sizeof("BLOCK_CODE_ARABIC_EXTENDED_A") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ARABIC_EXTENDED_A_name, &const_BLOCK_CODE_ARABIC_EXTENDED_A_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ARABIC_EXTENDED_A_name);

	zval const_BLOCK_CODE_ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS_value;
	ZVAL_LONG(&const_BLOCK_CODE_ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS_value, UBLOCK_ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS);
	crex_string *const_BLOCK_CODE_ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS_name = crex_string_init_interned("BLOCK_CODE_ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS", sizeof("BLOCK_CODE_ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS_name, &const_BLOCK_CODE_ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS_name);

	zval const_BLOCK_CODE_CHAKMA_value;
	ZVAL_LONG(&const_BLOCK_CODE_CHAKMA_value, UBLOCK_CHAKMA);
	crex_string *const_BLOCK_CODE_CHAKMA_name = crex_string_init_interned("BLOCK_CODE_CHAKMA", sizeof("BLOCK_CODE_CHAKMA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CHAKMA_name, &const_BLOCK_CODE_CHAKMA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CHAKMA_name);

	zval const_BLOCK_CODE_MEETEI_MAYEK_EXTENSIONS_value;
	ZVAL_LONG(&const_BLOCK_CODE_MEETEI_MAYEK_EXTENSIONS_value, UBLOCK_MEETEI_MAYEK_EXTENSIONS);
	crex_string *const_BLOCK_CODE_MEETEI_MAYEK_EXTENSIONS_name = crex_string_init_interned("BLOCK_CODE_MEETEI_MAYEK_EXTENSIONS", sizeof("BLOCK_CODE_MEETEI_MAYEK_EXTENSIONS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MEETEI_MAYEK_EXTENSIONS_name, &const_BLOCK_CODE_MEETEI_MAYEK_EXTENSIONS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MEETEI_MAYEK_EXTENSIONS_name);

	zval const_BLOCK_CODE_MEROITIC_CURSIVE_value;
	ZVAL_LONG(&const_BLOCK_CODE_MEROITIC_CURSIVE_value, UBLOCK_MEROITIC_CURSIVE);
	crex_string *const_BLOCK_CODE_MEROITIC_CURSIVE_name = crex_string_init_interned("BLOCK_CODE_MEROITIC_CURSIVE", sizeof("BLOCK_CODE_MEROITIC_CURSIVE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MEROITIC_CURSIVE_name, &const_BLOCK_CODE_MEROITIC_CURSIVE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MEROITIC_CURSIVE_name);

	zval const_BLOCK_CODE_MEROITIC_HIEROGLYPHS_value;
	ZVAL_LONG(&const_BLOCK_CODE_MEROITIC_HIEROGLYPHS_value, UBLOCK_MEROITIC_HIEROGLYPHS);
	crex_string *const_BLOCK_CODE_MEROITIC_HIEROGLYPHS_name = crex_string_init_interned("BLOCK_CODE_MEROITIC_HIEROGLYPHS", sizeof("BLOCK_CODE_MEROITIC_HIEROGLYPHS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MEROITIC_HIEROGLYPHS_name, &const_BLOCK_CODE_MEROITIC_HIEROGLYPHS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MEROITIC_HIEROGLYPHS_name);

	zval const_BLOCK_CODE_MIAO_value;
	ZVAL_LONG(&const_BLOCK_CODE_MIAO_value, UBLOCK_MIAO);
	crex_string *const_BLOCK_CODE_MIAO_name = crex_string_init_interned("BLOCK_CODE_MIAO", sizeof("BLOCK_CODE_MIAO") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MIAO_name, &const_BLOCK_CODE_MIAO_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MIAO_name);

	zval const_BLOCK_CODE_SHARADA_value;
	ZVAL_LONG(&const_BLOCK_CODE_SHARADA_value, UBLOCK_SHARADA);
	crex_string *const_BLOCK_CODE_SHARADA_name = crex_string_init_interned("BLOCK_CODE_SHARADA", sizeof("BLOCK_CODE_SHARADA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SHARADA_name, &const_BLOCK_CODE_SHARADA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SHARADA_name);

	zval const_BLOCK_CODE_SORA_SOMPENG_value;
	ZVAL_LONG(&const_BLOCK_CODE_SORA_SOMPENG_value, UBLOCK_SORA_SOMPENG);
	crex_string *const_BLOCK_CODE_SORA_SOMPENG_name = crex_string_init_interned("BLOCK_CODE_SORA_SOMPENG", sizeof("BLOCK_CODE_SORA_SOMPENG") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SORA_SOMPENG_name, &const_BLOCK_CODE_SORA_SOMPENG_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SORA_SOMPENG_name);

	zval const_BLOCK_CODE_SUNDANESE_SUPPLEMENT_value;
	ZVAL_LONG(&const_BLOCK_CODE_SUNDANESE_SUPPLEMENT_value, UBLOCK_SUNDANESE_SUPPLEMENT);
	crex_string *const_BLOCK_CODE_SUNDANESE_SUPPLEMENT_name = crex_string_init_interned("BLOCK_CODE_SUNDANESE_SUPPLEMENT", sizeof("BLOCK_CODE_SUNDANESE_SUPPLEMENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SUNDANESE_SUPPLEMENT_name, &const_BLOCK_CODE_SUNDANESE_SUPPLEMENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SUNDANESE_SUPPLEMENT_name);

	zval const_BLOCK_CODE_TAKRI_value;
	ZVAL_LONG(&const_BLOCK_CODE_TAKRI_value, UBLOCK_TAKRI);
	crex_string *const_BLOCK_CODE_TAKRI_name = crex_string_init_interned("BLOCK_CODE_TAKRI", sizeof("BLOCK_CODE_TAKRI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_TAKRI_name, &const_BLOCK_CODE_TAKRI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_TAKRI_name);
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_BASSA_VAH_value;
	ZVAL_LONG(&const_BLOCK_CODE_BASSA_VAH_value, UBLOCK_BASSA_VAH);
	crex_string *const_BLOCK_CODE_BASSA_VAH_name = crex_string_init_interned("BLOCK_CODE_BASSA_VAH", sizeof("BLOCK_CODE_BASSA_VAH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_BASSA_VAH_name, &const_BLOCK_CODE_BASSA_VAH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_BASSA_VAH_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_CAUCASIAN_ALBANIAN_value;
	ZVAL_LONG(&const_BLOCK_CODE_CAUCASIAN_ALBANIAN_value, UBLOCK_CAUCASIAN_ALBANIAN);
	crex_string *const_BLOCK_CODE_CAUCASIAN_ALBANIAN_name = crex_string_init_interned("BLOCK_CODE_CAUCASIAN_ALBANIAN", sizeof("BLOCK_CODE_CAUCASIAN_ALBANIAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_CAUCASIAN_ALBANIAN_name, &const_BLOCK_CODE_CAUCASIAN_ALBANIAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_CAUCASIAN_ALBANIAN_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_COPTIC_EPACT_NUMBERS_value;
	ZVAL_LONG(&const_BLOCK_CODE_COPTIC_EPACT_NUMBERS_value, UBLOCK_COPTIC_EPACT_NUMBERS);
	crex_string *const_BLOCK_CODE_COPTIC_EPACT_NUMBERS_name = crex_string_init_interned("BLOCK_CODE_COPTIC_EPACT_NUMBERS", sizeof("BLOCK_CODE_COPTIC_EPACT_NUMBERS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_COPTIC_EPACT_NUMBERS_name, &const_BLOCK_CODE_COPTIC_EPACT_NUMBERS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_COPTIC_EPACT_NUMBERS_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_EXTENDED_value;
	ZVAL_LONG(&const_BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_EXTENDED_value, UBLOCK_COMBINING_DIACRITICAL_MARKS_EXTENDED);
	crex_string *const_BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_EXTENDED_name = crex_string_init_interned("BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_EXTENDED", sizeof("BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_EXTENDED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_EXTENDED_name, &const_BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_EXTENDED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_COMBINING_DIACRITICAL_MARKS_EXTENDED_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_DUPLOYAN_value;
	ZVAL_LONG(&const_BLOCK_CODE_DUPLOYAN_value, UBLOCK_DUPLOYAN);
	crex_string *const_BLOCK_CODE_DUPLOYAN_name = crex_string_init_interned("BLOCK_CODE_DUPLOYAN", sizeof("BLOCK_CODE_DUPLOYAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_DUPLOYAN_name, &const_BLOCK_CODE_DUPLOYAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_DUPLOYAN_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_ELBASAN_value;
	ZVAL_LONG(&const_BLOCK_CODE_ELBASAN_value, UBLOCK_ELBASAN);
	crex_string *const_BLOCK_CODE_ELBASAN_name = crex_string_init_interned("BLOCK_CODE_ELBASAN", sizeof("BLOCK_CODE_ELBASAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ELBASAN_name, &const_BLOCK_CODE_ELBASAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ELBASAN_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_GEOMETRIC_SHAPES_EXTENDED_value;
	ZVAL_LONG(&const_BLOCK_CODE_GEOMETRIC_SHAPES_EXTENDED_value, UBLOCK_GEOMETRIC_SHAPES_EXTENDED);
	crex_string *const_BLOCK_CODE_GEOMETRIC_SHAPES_EXTENDED_name = crex_string_init_interned("BLOCK_CODE_GEOMETRIC_SHAPES_EXTENDED", sizeof("BLOCK_CODE_GEOMETRIC_SHAPES_EXTENDED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_GEOMETRIC_SHAPES_EXTENDED_name, &const_BLOCK_CODE_GEOMETRIC_SHAPES_EXTENDED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_GEOMETRIC_SHAPES_EXTENDED_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_GRANTHA_value;
	ZVAL_LONG(&const_BLOCK_CODE_GRANTHA_value, UBLOCK_GRANTHA);
	crex_string *const_BLOCK_CODE_GRANTHA_name = crex_string_init_interned("BLOCK_CODE_GRANTHA", sizeof("BLOCK_CODE_GRANTHA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_GRANTHA_name, &const_BLOCK_CODE_GRANTHA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_GRANTHA_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_KHOJKI_value;
	ZVAL_LONG(&const_BLOCK_CODE_KHOJKI_value, UBLOCK_KHOJKI);
	crex_string *const_BLOCK_CODE_KHOJKI_name = crex_string_init_interned("BLOCK_CODE_KHOJKI", sizeof("BLOCK_CODE_KHOJKI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_KHOJKI_name, &const_BLOCK_CODE_KHOJKI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_KHOJKI_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_KHUDAWADI_value;
	ZVAL_LONG(&const_BLOCK_CODE_KHUDAWADI_value, UBLOCK_KHUDAWADI);
	crex_string *const_BLOCK_CODE_KHUDAWADI_name = crex_string_init_interned("BLOCK_CODE_KHUDAWADI", sizeof("BLOCK_CODE_KHUDAWADI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_KHUDAWADI_name, &const_BLOCK_CODE_KHUDAWADI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_KHUDAWADI_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_LATIN_EXTENDED_E_value;
	ZVAL_LONG(&const_BLOCK_CODE_LATIN_EXTENDED_E_value, UBLOCK_LATIN_EXTENDED_E);
	crex_string *const_BLOCK_CODE_LATIN_EXTENDED_E_name = crex_string_init_interned("BLOCK_CODE_LATIN_EXTENDED_E", sizeof("BLOCK_CODE_LATIN_EXTENDED_E") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_LATIN_EXTENDED_E_name, &const_BLOCK_CODE_LATIN_EXTENDED_E_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_LATIN_EXTENDED_E_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_LINEAR_A_value;
	ZVAL_LONG(&const_BLOCK_CODE_LINEAR_A_value, UBLOCK_LINEAR_A);
	crex_string *const_BLOCK_CODE_LINEAR_A_name = crex_string_init_interned("BLOCK_CODE_LINEAR_A", sizeof("BLOCK_CODE_LINEAR_A") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_LINEAR_A_name, &const_BLOCK_CODE_LINEAR_A_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_LINEAR_A_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_MAHAJANI_value;
	ZVAL_LONG(&const_BLOCK_CODE_MAHAJANI_value, UBLOCK_MAHAJANI);
	crex_string *const_BLOCK_CODE_MAHAJANI_name = crex_string_init_interned("BLOCK_CODE_MAHAJANI", sizeof("BLOCK_CODE_MAHAJANI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MAHAJANI_name, &const_BLOCK_CODE_MAHAJANI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MAHAJANI_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_MANICHAEAN_value;
	ZVAL_LONG(&const_BLOCK_CODE_MANICHAEAN_value, UBLOCK_MANICHAEAN);
	crex_string *const_BLOCK_CODE_MANICHAEAN_name = crex_string_init_interned("BLOCK_CODE_MANICHAEAN", sizeof("BLOCK_CODE_MANICHAEAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MANICHAEAN_name, &const_BLOCK_CODE_MANICHAEAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MANICHAEAN_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_MENDE_KIKAKUI_value;
	ZVAL_LONG(&const_BLOCK_CODE_MENDE_KIKAKUI_value, UBLOCK_MENDE_KIKAKUI);
	crex_string *const_BLOCK_CODE_MENDE_KIKAKUI_name = crex_string_init_interned("BLOCK_CODE_MENDE_KIKAKUI", sizeof("BLOCK_CODE_MENDE_KIKAKUI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MENDE_KIKAKUI_name, &const_BLOCK_CODE_MENDE_KIKAKUI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MENDE_KIKAKUI_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_MODI_value;
	ZVAL_LONG(&const_BLOCK_CODE_MODI_value, UBLOCK_MODI);
	crex_string *const_BLOCK_CODE_MODI_name = crex_string_init_interned("BLOCK_CODE_MODI", sizeof("BLOCK_CODE_MODI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MODI_name, &const_BLOCK_CODE_MODI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MODI_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_MRO_value;
	ZVAL_LONG(&const_BLOCK_CODE_MRO_value, UBLOCK_MRO);
	crex_string *const_BLOCK_CODE_MRO_name = crex_string_init_interned("BLOCK_CODE_MRO", sizeof("BLOCK_CODE_MRO") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MRO_name, &const_BLOCK_CODE_MRO_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MRO_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_MYANMAR_EXTENDED_B_value;
	ZVAL_LONG(&const_BLOCK_CODE_MYANMAR_EXTENDED_B_value, UBLOCK_MYANMAR_EXTENDED_B);
	crex_string *const_BLOCK_CODE_MYANMAR_EXTENDED_B_name = crex_string_init_interned("BLOCK_CODE_MYANMAR_EXTENDED_B", sizeof("BLOCK_CODE_MYANMAR_EXTENDED_B") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_MYANMAR_EXTENDED_B_name, &const_BLOCK_CODE_MYANMAR_EXTENDED_B_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_MYANMAR_EXTENDED_B_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_NABATAEAN_value;
	ZVAL_LONG(&const_BLOCK_CODE_NABATAEAN_value, UBLOCK_NABATAEAN);
	crex_string *const_BLOCK_CODE_NABATAEAN_name = crex_string_init_interned("BLOCK_CODE_NABATAEAN", sizeof("BLOCK_CODE_NABATAEAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_NABATAEAN_name, &const_BLOCK_CODE_NABATAEAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_NABATAEAN_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_OLD_NORTH_ARABIAN_value;
	ZVAL_LONG(&const_BLOCK_CODE_OLD_NORTH_ARABIAN_value, UBLOCK_OLD_NORTH_ARABIAN);
	crex_string *const_BLOCK_CODE_OLD_NORTH_ARABIAN_name = crex_string_init_interned("BLOCK_CODE_OLD_NORTH_ARABIAN", sizeof("BLOCK_CODE_OLD_NORTH_ARABIAN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_OLD_NORTH_ARABIAN_name, &const_BLOCK_CODE_OLD_NORTH_ARABIAN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_OLD_NORTH_ARABIAN_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_OLD_PERMIC_value;
	ZVAL_LONG(&const_BLOCK_CODE_OLD_PERMIC_value, UBLOCK_OLD_PERMIC);
	crex_string *const_BLOCK_CODE_OLD_PERMIC_name = crex_string_init_interned("BLOCK_CODE_OLD_PERMIC", sizeof("BLOCK_CODE_OLD_PERMIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_OLD_PERMIC_name, &const_BLOCK_CODE_OLD_PERMIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_OLD_PERMIC_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_ORNAMENTAL_DINGBATS_value;
	ZVAL_LONG(&const_BLOCK_CODE_ORNAMENTAL_DINGBATS_value, UBLOCK_ORNAMENTAL_DINGBATS);
	crex_string *const_BLOCK_CODE_ORNAMENTAL_DINGBATS_name = crex_string_init_interned("BLOCK_CODE_ORNAMENTAL_DINGBATS", sizeof("BLOCK_CODE_ORNAMENTAL_DINGBATS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_ORNAMENTAL_DINGBATS_name, &const_BLOCK_CODE_ORNAMENTAL_DINGBATS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_ORNAMENTAL_DINGBATS_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_PAHAWH_HMONG_value;
	ZVAL_LONG(&const_BLOCK_CODE_PAHAWH_HMONG_value, UBLOCK_PAHAWH_HMONG);
	crex_string *const_BLOCK_CODE_PAHAWH_HMONG_name = crex_string_init_interned("BLOCK_CODE_PAHAWH_HMONG", sizeof("BLOCK_CODE_PAHAWH_HMONG") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_PAHAWH_HMONG_name, &const_BLOCK_CODE_PAHAWH_HMONG_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_PAHAWH_HMONG_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_PALMYRENE_value;
	ZVAL_LONG(&const_BLOCK_CODE_PALMYRENE_value, UBLOCK_PALMYRENE);
	crex_string *const_BLOCK_CODE_PALMYRENE_name = crex_string_init_interned("BLOCK_CODE_PALMYRENE", sizeof("BLOCK_CODE_PALMYRENE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_PALMYRENE_name, &const_BLOCK_CODE_PALMYRENE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_PALMYRENE_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_PAU_CIN_HAU_value;
	ZVAL_LONG(&const_BLOCK_CODE_PAU_CIN_HAU_value, UBLOCK_PAU_CIN_HAU);
	crex_string *const_BLOCK_CODE_PAU_CIN_HAU_name = crex_string_init_interned("BLOCK_CODE_PAU_CIN_HAU", sizeof("BLOCK_CODE_PAU_CIN_HAU") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_PAU_CIN_HAU_name, &const_BLOCK_CODE_PAU_CIN_HAU_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_PAU_CIN_HAU_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_PSALTER_PAHLAVI_value;
	ZVAL_LONG(&const_BLOCK_CODE_PSALTER_PAHLAVI_value, UBLOCK_PSALTER_PAHLAVI);
	crex_string *const_BLOCK_CODE_PSALTER_PAHLAVI_name = crex_string_init_interned("BLOCK_CODE_PSALTER_PAHLAVI", sizeof("BLOCK_CODE_PSALTER_PAHLAVI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_PSALTER_PAHLAVI_name, &const_BLOCK_CODE_PSALTER_PAHLAVI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_PSALTER_PAHLAVI_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_SHORTHAND_FORMAT_CONTROLS_value;
	ZVAL_LONG(&const_BLOCK_CODE_SHORTHAND_FORMAT_CONTROLS_value, UBLOCK_SHORTHAND_FORMAT_CONTROLS);
	crex_string *const_BLOCK_CODE_SHORTHAND_FORMAT_CONTROLS_name = crex_string_init_interned("BLOCK_CODE_SHORTHAND_FORMAT_CONTROLS", sizeof("BLOCK_CODE_SHORTHAND_FORMAT_CONTROLS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SHORTHAND_FORMAT_CONTROLS_name, &const_BLOCK_CODE_SHORTHAND_FORMAT_CONTROLS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SHORTHAND_FORMAT_CONTROLS_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_SIDDHAM_value;
	ZVAL_LONG(&const_BLOCK_CODE_SIDDHAM_value, UBLOCK_SIDDHAM);
	crex_string *const_BLOCK_CODE_SIDDHAM_name = crex_string_init_interned("BLOCK_CODE_SIDDHAM", sizeof("BLOCK_CODE_SIDDHAM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SIDDHAM_name, &const_BLOCK_CODE_SIDDHAM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SIDDHAM_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_SINHALA_ARCHAIC_NUMBERS_value;
	ZVAL_LONG(&const_BLOCK_CODE_SINHALA_ARCHAIC_NUMBERS_value, UBLOCK_SINHALA_ARCHAIC_NUMBERS);
	crex_string *const_BLOCK_CODE_SINHALA_ARCHAIC_NUMBERS_name = crex_string_init_interned("BLOCK_CODE_SINHALA_ARCHAIC_NUMBERS", sizeof("BLOCK_CODE_SINHALA_ARCHAIC_NUMBERS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SINHALA_ARCHAIC_NUMBERS_name, &const_BLOCK_CODE_SINHALA_ARCHAIC_NUMBERS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SINHALA_ARCHAIC_NUMBERS_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_SUPPLEMENTAL_ARROWS_C_value;
	ZVAL_LONG(&const_BLOCK_CODE_SUPPLEMENTAL_ARROWS_C_value, UBLOCK_SUPPLEMENTAL_ARROWS_C);
	crex_string *const_BLOCK_CODE_SUPPLEMENTAL_ARROWS_C_name = crex_string_init_interned("BLOCK_CODE_SUPPLEMENTAL_ARROWS_C", sizeof("BLOCK_CODE_SUPPLEMENTAL_ARROWS_C") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_SUPPLEMENTAL_ARROWS_C_name, &const_BLOCK_CODE_SUPPLEMENTAL_ARROWS_C_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_SUPPLEMENTAL_ARROWS_C_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_TIRHUTA_value;
	ZVAL_LONG(&const_BLOCK_CODE_TIRHUTA_value, UBLOCK_TIRHUTA);
	crex_string *const_BLOCK_CODE_TIRHUTA_name = crex_string_init_interned("BLOCK_CODE_TIRHUTA", sizeof("BLOCK_CODE_TIRHUTA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_TIRHUTA_name, &const_BLOCK_CODE_TIRHUTA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_TIRHUTA_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_BLOCK_CODE_WARANG_CITI_value;
	ZVAL_LONG(&const_BLOCK_CODE_WARANG_CITI_value, UBLOCK_WARANG_CITI);
	crex_string *const_BLOCK_CODE_WARANG_CITI_name = crex_string_init_interned("BLOCK_CODE_WARANG_CITI", sizeof("BLOCK_CODE_WARANG_CITI") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_WARANG_CITI_name, &const_BLOCK_CODE_WARANG_CITI_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_WARANG_CITI_name);
#endif

	zval const_BLOCK_CODE_COUNT_value;
	ZVAL_LONG(&const_BLOCK_CODE_COUNT_value, UBLOCK_COUNT);
	crex_string *const_BLOCK_CODE_COUNT_name = crex_string_init_interned("BLOCK_CODE_COUNT", sizeof("BLOCK_CODE_COUNT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_COUNT_name, &const_BLOCK_CODE_COUNT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_COUNT_name);

	zval const_BLOCK_CODE_INVALID_CODE_value;
	ZVAL_LONG(&const_BLOCK_CODE_INVALID_CODE_value, UBLOCK_INVALID_CODE);
	crex_string *const_BLOCK_CODE_INVALID_CODE_name = crex_string_init_interned("BLOCK_CODE_INVALID_CODE", sizeof("BLOCK_CODE_INVALID_CODE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BLOCK_CODE_INVALID_CODE_name, &const_BLOCK_CODE_INVALID_CODE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BLOCK_CODE_INVALID_CODE_name);
#if U_ICU_VERSION_MAJOR_NUM >= 52

	zval const_BPT_NONE_value;
	ZVAL_LONG(&const_BPT_NONE_value, U_BPT_NONE);
	crex_string *const_BPT_NONE_name = crex_string_init_interned("BPT_NONE", sizeof("BPT_NONE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BPT_NONE_name, &const_BPT_NONE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BPT_NONE_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 52

	zval const_BPT_OPEN_value;
	ZVAL_LONG(&const_BPT_OPEN_value, U_BPT_OPEN);
	crex_string *const_BPT_OPEN_name = crex_string_init_interned("BPT_OPEN", sizeof("BPT_OPEN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BPT_OPEN_name, &const_BPT_OPEN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BPT_OPEN_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 52

	zval const_BPT_CLOSE_value;
	ZVAL_LONG(&const_BPT_CLOSE_value, U_BPT_CLOSE);
	crex_string *const_BPT_CLOSE_name = crex_string_init_interned("BPT_CLOSE", sizeof("BPT_CLOSE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BPT_CLOSE_name, &const_BPT_CLOSE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BPT_CLOSE_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 52

	zval const_BPT_COUNT_value;
	ZVAL_LONG(&const_BPT_COUNT_value, U_BPT_COUNT);
	crex_string *const_BPT_COUNT_name = crex_string_init_interned("BPT_COUNT", sizeof("BPT_COUNT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BPT_COUNT_name, &const_BPT_COUNT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BPT_COUNT_name);
#endif

	zval const_EA_NEUTRAL_value;
	ZVAL_LONG(&const_EA_NEUTRAL_value, U_EA_NEUTRAL);
	crex_string *const_EA_NEUTRAL_name = crex_string_init_interned("EA_NEUTRAL", sizeof("EA_NEUTRAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_EA_NEUTRAL_name, &const_EA_NEUTRAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_EA_NEUTRAL_name);

	zval const_EA_AMBIGUOUS_value;
	ZVAL_LONG(&const_EA_AMBIGUOUS_value, U_EA_AMBIGUOUS);
	crex_string *const_EA_AMBIGUOUS_name = crex_string_init_interned("EA_AMBIGUOUS", sizeof("EA_AMBIGUOUS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_EA_AMBIGUOUS_name, &const_EA_AMBIGUOUS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_EA_AMBIGUOUS_name);

	zval const_EA_HALFWIDTH_value;
	ZVAL_LONG(&const_EA_HALFWIDTH_value, U_EA_HALFWIDTH);
	crex_string *const_EA_HALFWIDTH_name = crex_string_init_interned("EA_HALFWIDTH", sizeof("EA_HALFWIDTH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_EA_HALFWIDTH_name, &const_EA_HALFWIDTH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_EA_HALFWIDTH_name);

	zval const_EA_FULLWIDTH_value;
	ZVAL_LONG(&const_EA_FULLWIDTH_value, U_EA_FULLWIDTH);
	crex_string *const_EA_FULLWIDTH_name = crex_string_init_interned("EA_FULLWIDTH", sizeof("EA_FULLWIDTH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_EA_FULLWIDTH_name, &const_EA_FULLWIDTH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_EA_FULLWIDTH_name);

	zval const_EA_NARROW_value;
	ZVAL_LONG(&const_EA_NARROW_value, U_EA_NARROW);
	crex_string *const_EA_NARROW_name = crex_string_init_interned("EA_NARROW", sizeof("EA_NARROW") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_EA_NARROW_name, &const_EA_NARROW_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_EA_NARROW_name);

	zval const_EA_WIDE_value;
	ZVAL_LONG(&const_EA_WIDE_value, U_EA_WIDE);
	crex_string *const_EA_WIDE_name = crex_string_init_interned("EA_WIDE", sizeof("EA_WIDE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_EA_WIDE_name, &const_EA_WIDE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_EA_WIDE_name);

	zval const_EA_COUNT_value;
	ZVAL_LONG(&const_EA_COUNT_value, U_EA_COUNT);
	crex_string *const_EA_COUNT_name = crex_string_init_interned("EA_COUNT", sizeof("EA_COUNT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_EA_COUNT_name, &const_EA_COUNT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_EA_COUNT_name);

	zval const_UNICODE_CHAR_NAME_value;
	ZVAL_LONG(&const_UNICODE_CHAR_NAME_value, U_UNICODE_CHAR_NAME);
	crex_string *const_UNICODE_CHAR_NAME_name = crex_string_init_interned("UNICODE_CHAR_NAME", sizeof("UNICODE_CHAR_NAME") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_UNICODE_CHAR_NAME_name, &const_UNICODE_CHAR_NAME_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_UNICODE_CHAR_NAME_name);

	zval const_UNICODE_10_CHAR_NAME_value;
	ZVAL_LONG(&const_UNICODE_10_CHAR_NAME_value, U_UNICODE_10_CHAR_NAME);
	crex_string *const_UNICODE_10_CHAR_NAME_name = crex_string_init_interned("UNICODE_10_CHAR_NAME", sizeof("UNICODE_10_CHAR_NAME") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_UNICODE_10_CHAR_NAME_name, &const_UNICODE_10_CHAR_NAME_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_UNICODE_10_CHAR_NAME_name);

	zval const_EXTENDED_CHAR_NAME_value;
	ZVAL_LONG(&const_EXTENDED_CHAR_NAME_value, U_EXTENDED_CHAR_NAME);
	crex_string *const_EXTENDED_CHAR_NAME_name = crex_string_init_interned("EXTENDED_CHAR_NAME", sizeof("EXTENDED_CHAR_NAME") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_EXTENDED_CHAR_NAME_name, &const_EXTENDED_CHAR_NAME_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_EXTENDED_CHAR_NAME_name);

	zval const_CHAR_NAME_ALIAS_value;
	ZVAL_LONG(&const_CHAR_NAME_ALIAS_value, U_CHAR_NAME_ALIAS);
	crex_string *const_CHAR_NAME_ALIAS_name = crex_string_init_interned("CHAR_NAME_ALIAS", sizeof("CHAR_NAME_ALIAS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_NAME_ALIAS_name, &const_CHAR_NAME_ALIAS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_NAME_ALIAS_name);

	zval const_CHAR_NAME_CHOICE_COUNT_value;
	ZVAL_LONG(&const_CHAR_NAME_CHOICE_COUNT_value, U_CHAR_NAME_CHOICE_COUNT);
	crex_string *const_CHAR_NAME_CHOICE_COUNT_name = crex_string_init_interned("CHAR_NAME_CHOICE_COUNT", sizeof("CHAR_NAME_CHOICE_COUNT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHAR_NAME_CHOICE_COUNT_name, &const_CHAR_NAME_CHOICE_COUNT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHAR_NAME_CHOICE_COUNT_name);

	zval const_SHORT_PROPERTY_NAME_value;
	ZVAL_LONG(&const_SHORT_PROPERTY_NAME_value, U_SHORT_PROPERTY_NAME);
	crex_string *const_SHORT_PROPERTY_NAME_name = crex_string_init_interned("SHORT_PROPERTY_NAME", sizeof("SHORT_PROPERTY_NAME") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SHORT_PROPERTY_NAME_name, &const_SHORT_PROPERTY_NAME_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SHORT_PROPERTY_NAME_name);

	zval const_LONG_PROPERTY_NAME_value;
	ZVAL_LONG(&const_LONG_PROPERTY_NAME_value, U_LONG_PROPERTY_NAME);
	crex_string *const_LONG_PROPERTY_NAME_name = crex_string_init_interned("LONG_PROPERTY_NAME", sizeof("LONG_PROPERTY_NAME") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LONG_PROPERTY_NAME_name, &const_LONG_PROPERTY_NAME_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LONG_PROPERTY_NAME_name);

	zval const_PROPERTY_NAME_CHOICE_COUNT_value;
	ZVAL_LONG(&const_PROPERTY_NAME_CHOICE_COUNT_value, U_PROPERTY_NAME_CHOICE_COUNT);
	crex_string *const_PROPERTY_NAME_CHOICE_COUNT_name = crex_string_init_interned("PROPERTY_NAME_CHOICE_COUNT", sizeof("PROPERTY_NAME_CHOICE_COUNT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PROPERTY_NAME_CHOICE_COUNT_name, &const_PROPERTY_NAME_CHOICE_COUNT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PROPERTY_NAME_CHOICE_COUNT_name);

	zval const_DT_NONE_value;
	ZVAL_LONG(&const_DT_NONE_value, U_DT_NONE);
	crex_string *const_DT_NONE_name = crex_string_init_interned("DT_NONE", sizeof("DT_NONE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DT_NONE_name, &const_DT_NONE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DT_NONE_name);

	zval const_DT_CANONICAL_value;
	ZVAL_LONG(&const_DT_CANONICAL_value, U_DT_CANONICAL);
	crex_string *const_DT_CANONICAL_name = crex_string_init_interned("DT_CANONICAL", sizeof("DT_CANONICAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DT_CANONICAL_name, &const_DT_CANONICAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DT_CANONICAL_name);

	zval const_DT_COMPAT_value;
	ZVAL_LONG(&const_DT_COMPAT_value, U_DT_COMPAT);
	crex_string *const_DT_COMPAT_name = crex_string_init_interned("DT_COMPAT", sizeof("DT_COMPAT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DT_COMPAT_name, &const_DT_COMPAT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DT_COMPAT_name);

	zval const_DT_CIRCLE_value;
	ZVAL_LONG(&const_DT_CIRCLE_value, U_DT_CIRCLE);
	crex_string *const_DT_CIRCLE_name = crex_string_init_interned("DT_CIRCLE", sizeof("DT_CIRCLE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DT_CIRCLE_name, &const_DT_CIRCLE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DT_CIRCLE_name);

	zval const_DT_FINAL_value;
	ZVAL_LONG(&const_DT_FINAL_value, U_DT_FINAL);
	crex_string *const_DT_FINAL_name = crex_string_init_interned("DT_FINAL", sizeof("DT_FINAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DT_FINAL_name, &const_DT_FINAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DT_FINAL_name);

	zval const_DT_FONT_value;
	ZVAL_LONG(&const_DT_FONT_value, U_DT_FONT);
	crex_string *const_DT_FONT_name = crex_string_init_interned("DT_FONT", sizeof("DT_FONT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DT_FONT_name, &const_DT_FONT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DT_FONT_name);

	zval const_DT_FRACTION_value;
	ZVAL_LONG(&const_DT_FRACTION_value, U_DT_FRACTION);
	crex_string *const_DT_FRACTION_name = crex_string_init_interned("DT_FRACTION", sizeof("DT_FRACTION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DT_FRACTION_name, &const_DT_FRACTION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DT_FRACTION_name);

	zval const_DT_INITIAL_value;
	ZVAL_LONG(&const_DT_INITIAL_value, U_DT_INITIAL);
	crex_string *const_DT_INITIAL_name = crex_string_init_interned("DT_INITIAL", sizeof("DT_INITIAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DT_INITIAL_name, &const_DT_INITIAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DT_INITIAL_name);

	zval const_DT_ISOLATED_value;
	ZVAL_LONG(&const_DT_ISOLATED_value, U_DT_ISOLATED);
	crex_string *const_DT_ISOLATED_name = crex_string_init_interned("DT_ISOLATED", sizeof("DT_ISOLATED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DT_ISOLATED_name, &const_DT_ISOLATED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DT_ISOLATED_name);

	zval const_DT_MEDIAL_value;
	ZVAL_LONG(&const_DT_MEDIAL_value, U_DT_MEDIAL);
	crex_string *const_DT_MEDIAL_name = crex_string_init_interned("DT_MEDIAL", sizeof("DT_MEDIAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DT_MEDIAL_name, &const_DT_MEDIAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DT_MEDIAL_name);

	zval const_DT_NARROW_value;
	ZVAL_LONG(&const_DT_NARROW_value, U_DT_NARROW);
	crex_string *const_DT_NARROW_name = crex_string_init_interned("DT_NARROW", sizeof("DT_NARROW") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DT_NARROW_name, &const_DT_NARROW_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DT_NARROW_name);

	zval const_DT_NOBREAK_value;
	ZVAL_LONG(&const_DT_NOBREAK_value, U_DT_NOBREAK);
	crex_string *const_DT_NOBREAK_name = crex_string_init_interned("DT_NOBREAK", sizeof("DT_NOBREAK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DT_NOBREAK_name, &const_DT_NOBREAK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DT_NOBREAK_name);

	zval const_DT_SMALL_value;
	ZVAL_LONG(&const_DT_SMALL_value, U_DT_SMALL);
	crex_string *const_DT_SMALL_name = crex_string_init_interned("DT_SMALL", sizeof("DT_SMALL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DT_SMALL_name, &const_DT_SMALL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DT_SMALL_name);

	zval const_DT_SQUARE_value;
	ZVAL_LONG(&const_DT_SQUARE_value, U_DT_SQUARE);
	crex_string *const_DT_SQUARE_name = crex_string_init_interned("DT_SQUARE", sizeof("DT_SQUARE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DT_SQUARE_name, &const_DT_SQUARE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DT_SQUARE_name);

	zval const_DT_SUB_value;
	ZVAL_LONG(&const_DT_SUB_value, U_DT_SUB);
	crex_string *const_DT_SUB_name = crex_string_init_interned("DT_SUB", sizeof("DT_SUB") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DT_SUB_name, &const_DT_SUB_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DT_SUB_name);

	zval const_DT_SUPER_value;
	ZVAL_LONG(&const_DT_SUPER_value, U_DT_SUPER);
	crex_string *const_DT_SUPER_name = crex_string_init_interned("DT_SUPER", sizeof("DT_SUPER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DT_SUPER_name, &const_DT_SUPER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DT_SUPER_name);

	zval const_DT_VERTICAL_value;
	ZVAL_LONG(&const_DT_VERTICAL_value, U_DT_VERTICAL);
	crex_string *const_DT_VERTICAL_name = crex_string_init_interned("DT_VERTICAL", sizeof("DT_VERTICAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DT_VERTICAL_name, &const_DT_VERTICAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DT_VERTICAL_name);

	zval const_DT_WIDE_value;
	ZVAL_LONG(&const_DT_WIDE_value, U_DT_WIDE);
	crex_string *const_DT_WIDE_name = crex_string_init_interned("DT_WIDE", sizeof("DT_WIDE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DT_WIDE_name, &const_DT_WIDE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DT_WIDE_name);

	zval const_DT_COUNT_value;
	ZVAL_LONG(&const_DT_COUNT_value, U_DT_COUNT);
	crex_string *const_DT_COUNT_name = crex_string_init_interned("DT_COUNT", sizeof("DT_COUNT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DT_COUNT_name, &const_DT_COUNT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DT_COUNT_name);

	zval const_JT_NON_JOINING_value;
	ZVAL_LONG(&const_JT_NON_JOINING_value, U_JT_NON_JOINING);
	crex_string *const_JT_NON_JOINING_name = crex_string_init_interned("JT_NON_JOINING", sizeof("JT_NON_JOINING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JT_NON_JOINING_name, &const_JT_NON_JOINING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JT_NON_JOINING_name);

	zval const_JT_JOIN_CAUSING_value;
	ZVAL_LONG(&const_JT_JOIN_CAUSING_value, U_JT_JOIN_CAUSING);
	crex_string *const_JT_JOIN_CAUSING_name = crex_string_init_interned("JT_JOIN_CAUSING", sizeof("JT_JOIN_CAUSING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JT_JOIN_CAUSING_name, &const_JT_JOIN_CAUSING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JT_JOIN_CAUSING_name);

	zval const_JT_DUAL_JOINING_value;
	ZVAL_LONG(&const_JT_DUAL_JOINING_value, U_JT_DUAL_JOINING);
	crex_string *const_JT_DUAL_JOINING_name = crex_string_init_interned("JT_DUAL_JOINING", sizeof("JT_DUAL_JOINING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JT_DUAL_JOINING_name, &const_JT_DUAL_JOINING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JT_DUAL_JOINING_name);

	zval const_JT_LEFT_JOINING_value;
	ZVAL_LONG(&const_JT_LEFT_JOINING_value, U_JT_LEFT_JOINING);
	crex_string *const_JT_LEFT_JOINING_name = crex_string_init_interned("JT_LEFT_JOINING", sizeof("JT_LEFT_JOINING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JT_LEFT_JOINING_name, &const_JT_LEFT_JOINING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JT_LEFT_JOINING_name);

	zval const_JT_RIGHT_JOINING_value;
	ZVAL_LONG(&const_JT_RIGHT_JOINING_value, U_JT_RIGHT_JOINING);
	crex_string *const_JT_RIGHT_JOINING_name = crex_string_init_interned("JT_RIGHT_JOINING", sizeof("JT_RIGHT_JOINING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JT_RIGHT_JOINING_name, &const_JT_RIGHT_JOINING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JT_RIGHT_JOINING_name);

	zval const_JT_TRANSPARENT_value;
	ZVAL_LONG(&const_JT_TRANSPARENT_value, U_JT_TRANSPARENT);
	crex_string *const_JT_TRANSPARENT_name = crex_string_init_interned("JT_TRANSPARENT", sizeof("JT_TRANSPARENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JT_TRANSPARENT_name, &const_JT_TRANSPARENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JT_TRANSPARENT_name);

	zval const_JT_COUNT_value;
	ZVAL_LONG(&const_JT_COUNT_value, U_JT_COUNT);
	crex_string *const_JT_COUNT_name = crex_string_init_interned("JT_COUNT", sizeof("JT_COUNT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JT_COUNT_name, &const_JT_COUNT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JT_COUNT_name);

	zval const_JG_NO_JOINING_GROUP_value;
	ZVAL_LONG(&const_JG_NO_JOINING_GROUP_value, U_JG_NO_JOINING_GROUP);
	crex_string *const_JG_NO_JOINING_GROUP_name = crex_string_init_interned("JG_NO_JOINING_GROUP", sizeof("JG_NO_JOINING_GROUP") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_NO_JOINING_GROUP_name, &const_JG_NO_JOINING_GROUP_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_NO_JOINING_GROUP_name);

	zval const_JG_AIN_value;
	ZVAL_LONG(&const_JG_AIN_value, U_JG_AIN);
	crex_string *const_JG_AIN_name = crex_string_init_interned("JG_AIN", sizeof("JG_AIN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_AIN_name, &const_JG_AIN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_AIN_name);

	zval const_JG_ALAPH_value;
	ZVAL_LONG(&const_JG_ALAPH_value, U_JG_ALAPH);
	crex_string *const_JG_ALAPH_name = crex_string_init_interned("JG_ALAPH", sizeof("JG_ALAPH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_ALAPH_name, &const_JG_ALAPH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_ALAPH_name);

	zval const_JG_ALEF_value;
	ZVAL_LONG(&const_JG_ALEF_value, U_JG_ALEF);
	crex_string *const_JG_ALEF_name = crex_string_init_interned("JG_ALEF", sizeof("JG_ALEF") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_ALEF_name, &const_JG_ALEF_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_ALEF_name);

	zval const_JG_BEH_value;
	ZVAL_LONG(&const_JG_BEH_value, U_JG_BEH);
	crex_string *const_JG_BEH_name = crex_string_init_interned("JG_BEH", sizeof("JG_BEH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_BEH_name, &const_JG_BEH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_BEH_name);

	zval const_JG_BETH_value;
	ZVAL_LONG(&const_JG_BETH_value, U_JG_BETH);
	crex_string *const_JG_BETH_name = crex_string_init_interned("JG_BETH", sizeof("JG_BETH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_BETH_name, &const_JG_BETH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_BETH_name);

	zval const_JG_DAL_value;
	ZVAL_LONG(&const_JG_DAL_value, U_JG_DAL);
	crex_string *const_JG_DAL_name = crex_string_init_interned("JG_DAL", sizeof("JG_DAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_DAL_name, &const_JG_DAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_DAL_name);

	zval const_JG_DALATH_RISH_value;
	ZVAL_LONG(&const_JG_DALATH_RISH_value, U_JG_DALATH_RISH);
	crex_string *const_JG_DALATH_RISH_name = crex_string_init_interned("JG_DALATH_RISH", sizeof("JG_DALATH_RISH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_DALATH_RISH_name, &const_JG_DALATH_RISH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_DALATH_RISH_name);

	zval const_JG_E_value;
	ZVAL_LONG(&const_JG_E_value, U_JG_E);
	crex_string *const_JG_E_name = crex_string_init_interned("JG_E", sizeof("JG_E") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_E_name, &const_JG_E_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_E_name);

	zval const_JG_FEH_value;
	ZVAL_LONG(&const_JG_FEH_value, U_JG_FEH);
	crex_string *const_JG_FEH_name = crex_string_init_interned("JG_FEH", sizeof("JG_FEH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_FEH_name, &const_JG_FEH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_FEH_name);

	zval const_JG_FINAL_SEMKATH_value;
	ZVAL_LONG(&const_JG_FINAL_SEMKATH_value, U_JG_FINAL_SEMKATH);
	crex_string *const_JG_FINAL_SEMKATH_name = crex_string_init_interned("JG_FINAL_SEMKATH", sizeof("JG_FINAL_SEMKATH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_FINAL_SEMKATH_name, &const_JG_FINAL_SEMKATH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_FINAL_SEMKATH_name);

	zval const_JG_GAF_value;
	ZVAL_LONG(&const_JG_GAF_value, U_JG_GAF);
	crex_string *const_JG_GAF_name = crex_string_init_interned("JG_GAF", sizeof("JG_GAF") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_GAF_name, &const_JG_GAF_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_GAF_name);

	zval const_JG_GAMAL_value;
	ZVAL_LONG(&const_JG_GAMAL_value, U_JG_GAMAL);
	crex_string *const_JG_GAMAL_name = crex_string_init_interned("JG_GAMAL", sizeof("JG_GAMAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_GAMAL_name, &const_JG_GAMAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_GAMAL_name);

	zval const_JG_HAH_value;
	ZVAL_LONG(&const_JG_HAH_value, U_JG_HAH);
	crex_string *const_JG_HAH_name = crex_string_init_interned("JG_HAH", sizeof("JG_HAH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_HAH_name, &const_JG_HAH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_HAH_name);

	zval const_JG_TEH_MARBUTA_GOAL_value;
	ZVAL_LONG(&const_JG_TEH_MARBUTA_GOAL_value, U_JG_TEH_MARBUTA_GOAL);
	crex_string *const_JG_TEH_MARBUTA_GOAL_name = crex_string_init_interned("JG_TEH_MARBUTA_GOAL", sizeof("JG_TEH_MARBUTA_GOAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_TEH_MARBUTA_GOAL_name, &const_JG_TEH_MARBUTA_GOAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_TEH_MARBUTA_GOAL_name);

	zval const_JG_HAMZA_ON_HEH_GOAL_value;
	ZVAL_LONG(&const_JG_HAMZA_ON_HEH_GOAL_value, U_JG_HAMZA_ON_HEH_GOAL);
	crex_string *const_JG_HAMZA_ON_HEH_GOAL_name = crex_string_init_interned("JG_HAMZA_ON_HEH_GOAL", sizeof("JG_HAMZA_ON_HEH_GOAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_HAMZA_ON_HEH_GOAL_name, &const_JG_HAMZA_ON_HEH_GOAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_HAMZA_ON_HEH_GOAL_name);

	zval const_JG_HE_value;
	ZVAL_LONG(&const_JG_HE_value, U_JG_HE);
	crex_string *const_JG_HE_name = crex_string_init_interned("JG_HE", sizeof("JG_HE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_HE_name, &const_JG_HE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_HE_name);

	zval const_JG_HEH_value;
	ZVAL_LONG(&const_JG_HEH_value, U_JG_HEH);
	crex_string *const_JG_HEH_name = crex_string_init_interned("JG_HEH", sizeof("JG_HEH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_HEH_name, &const_JG_HEH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_HEH_name);

	zval const_JG_HEH_GOAL_value;
	ZVAL_LONG(&const_JG_HEH_GOAL_value, U_JG_HEH_GOAL);
	crex_string *const_JG_HEH_GOAL_name = crex_string_init_interned("JG_HEH_GOAL", sizeof("JG_HEH_GOAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_HEH_GOAL_name, &const_JG_HEH_GOAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_HEH_GOAL_name);

	zval const_JG_HETH_value;
	ZVAL_LONG(&const_JG_HETH_value, U_JG_HETH);
	crex_string *const_JG_HETH_name = crex_string_init_interned("JG_HETH", sizeof("JG_HETH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_HETH_name, &const_JG_HETH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_HETH_name);

	zval const_JG_KAF_value;
	ZVAL_LONG(&const_JG_KAF_value, U_JG_KAF);
	crex_string *const_JG_KAF_name = crex_string_init_interned("JG_KAF", sizeof("JG_KAF") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_KAF_name, &const_JG_KAF_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_KAF_name);

	zval const_JG_KAPH_value;
	ZVAL_LONG(&const_JG_KAPH_value, U_JG_KAPH);
	crex_string *const_JG_KAPH_name = crex_string_init_interned("JG_KAPH", sizeof("JG_KAPH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_KAPH_name, &const_JG_KAPH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_KAPH_name);

	zval const_JG_KNOTTED_HEH_value;
	ZVAL_LONG(&const_JG_KNOTTED_HEH_value, U_JG_KNOTTED_HEH);
	crex_string *const_JG_KNOTTED_HEH_name = crex_string_init_interned("JG_KNOTTED_HEH", sizeof("JG_KNOTTED_HEH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_KNOTTED_HEH_name, &const_JG_KNOTTED_HEH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_KNOTTED_HEH_name);

	zval const_JG_LAM_value;
	ZVAL_LONG(&const_JG_LAM_value, U_JG_LAM);
	crex_string *const_JG_LAM_name = crex_string_init_interned("JG_LAM", sizeof("JG_LAM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_LAM_name, &const_JG_LAM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_LAM_name);

	zval const_JG_LAMADH_value;
	ZVAL_LONG(&const_JG_LAMADH_value, U_JG_LAMADH);
	crex_string *const_JG_LAMADH_name = crex_string_init_interned("JG_LAMADH", sizeof("JG_LAMADH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_LAMADH_name, &const_JG_LAMADH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_LAMADH_name);

	zval const_JG_MEEM_value;
	ZVAL_LONG(&const_JG_MEEM_value, U_JG_MEEM);
	crex_string *const_JG_MEEM_name = crex_string_init_interned("JG_MEEM", sizeof("JG_MEEM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MEEM_name, &const_JG_MEEM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MEEM_name);

	zval const_JG_MIM_value;
	ZVAL_LONG(&const_JG_MIM_value, U_JG_MIM);
	crex_string *const_JG_MIM_name = crex_string_init_interned("JG_MIM", sizeof("JG_MIM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MIM_name, &const_JG_MIM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MIM_name);

	zval const_JG_NOON_value;
	ZVAL_LONG(&const_JG_NOON_value, U_JG_NOON);
	crex_string *const_JG_NOON_name = crex_string_init_interned("JG_NOON", sizeof("JG_NOON") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_NOON_name, &const_JG_NOON_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_NOON_name);

	zval const_JG_NUN_value;
	ZVAL_LONG(&const_JG_NUN_value, U_JG_NUN);
	crex_string *const_JG_NUN_name = crex_string_init_interned("JG_NUN", sizeof("JG_NUN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_NUN_name, &const_JG_NUN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_NUN_name);

	zval const_JG_PE_value;
	ZVAL_LONG(&const_JG_PE_value, U_JG_PE);
	crex_string *const_JG_PE_name = crex_string_init_interned("JG_PE", sizeof("JG_PE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_PE_name, &const_JG_PE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_PE_name);

	zval const_JG_QAF_value;
	ZVAL_LONG(&const_JG_QAF_value, U_JG_QAF);
	crex_string *const_JG_QAF_name = crex_string_init_interned("JG_QAF", sizeof("JG_QAF") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_QAF_name, &const_JG_QAF_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_QAF_name);

	zval const_JG_QAPH_value;
	ZVAL_LONG(&const_JG_QAPH_value, U_JG_QAPH);
	crex_string *const_JG_QAPH_name = crex_string_init_interned("JG_QAPH", sizeof("JG_QAPH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_QAPH_name, &const_JG_QAPH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_QAPH_name);

	zval const_JG_REH_value;
	ZVAL_LONG(&const_JG_REH_value, U_JG_REH);
	crex_string *const_JG_REH_name = crex_string_init_interned("JG_REH", sizeof("JG_REH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_REH_name, &const_JG_REH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_REH_name);

	zval const_JG_REVERSED_PE_value;
	ZVAL_LONG(&const_JG_REVERSED_PE_value, U_JG_REVERSED_PE);
	crex_string *const_JG_REVERSED_PE_name = crex_string_init_interned("JG_REVERSED_PE", sizeof("JG_REVERSED_PE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_REVERSED_PE_name, &const_JG_REVERSED_PE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_REVERSED_PE_name);

	zval const_JG_SAD_value;
	ZVAL_LONG(&const_JG_SAD_value, U_JG_SAD);
	crex_string *const_JG_SAD_name = crex_string_init_interned("JG_SAD", sizeof("JG_SAD") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_SAD_name, &const_JG_SAD_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_SAD_name);

	zval const_JG_SADHE_value;
	ZVAL_LONG(&const_JG_SADHE_value, U_JG_SADHE);
	crex_string *const_JG_SADHE_name = crex_string_init_interned("JG_SADHE", sizeof("JG_SADHE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_SADHE_name, &const_JG_SADHE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_SADHE_name);

	zval const_JG_SEEN_value;
	ZVAL_LONG(&const_JG_SEEN_value, U_JG_SEEN);
	crex_string *const_JG_SEEN_name = crex_string_init_interned("JG_SEEN", sizeof("JG_SEEN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_SEEN_name, &const_JG_SEEN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_SEEN_name);

	zval const_JG_SEMKATH_value;
	ZVAL_LONG(&const_JG_SEMKATH_value, U_JG_SEMKATH);
	crex_string *const_JG_SEMKATH_name = crex_string_init_interned("JG_SEMKATH", sizeof("JG_SEMKATH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_SEMKATH_name, &const_JG_SEMKATH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_SEMKATH_name);

	zval const_JG_SHIN_value;
	ZVAL_LONG(&const_JG_SHIN_value, U_JG_SHIN);
	crex_string *const_JG_SHIN_name = crex_string_init_interned("JG_SHIN", sizeof("JG_SHIN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_SHIN_name, &const_JG_SHIN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_SHIN_name);

	zval const_JG_SWASH_KAF_value;
	ZVAL_LONG(&const_JG_SWASH_KAF_value, U_JG_SWASH_KAF);
	crex_string *const_JG_SWASH_KAF_name = crex_string_init_interned("JG_SWASH_KAF", sizeof("JG_SWASH_KAF") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_SWASH_KAF_name, &const_JG_SWASH_KAF_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_SWASH_KAF_name);

	zval const_JG_SYRIAC_WAW_value;
	ZVAL_LONG(&const_JG_SYRIAC_WAW_value, U_JG_SYRIAC_WAW);
	crex_string *const_JG_SYRIAC_WAW_name = crex_string_init_interned("JG_SYRIAC_WAW", sizeof("JG_SYRIAC_WAW") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_SYRIAC_WAW_name, &const_JG_SYRIAC_WAW_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_SYRIAC_WAW_name);

	zval const_JG_TAH_value;
	ZVAL_LONG(&const_JG_TAH_value, U_JG_TAH);
	crex_string *const_JG_TAH_name = crex_string_init_interned("JG_TAH", sizeof("JG_TAH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_TAH_name, &const_JG_TAH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_TAH_name);

	zval const_JG_TAW_value;
	ZVAL_LONG(&const_JG_TAW_value, U_JG_TAW);
	crex_string *const_JG_TAW_name = crex_string_init_interned("JG_TAW", sizeof("JG_TAW") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_TAW_name, &const_JG_TAW_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_TAW_name);

	zval const_JG_TEH_MARBUTA_value;
	ZVAL_LONG(&const_JG_TEH_MARBUTA_value, U_JG_TEH_MARBUTA);
	crex_string *const_JG_TEH_MARBUTA_name = crex_string_init_interned("JG_TEH_MARBUTA", sizeof("JG_TEH_MARBUTA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_TEH_MARBUTA_name, &const_JG_TEH_MARBUTA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_TEH_MARBUTA_name);

	zval const_JG_TETH_value;
	ZVAL_LONG(&const_JG_TETH_value, U_JG_TETH);
	crex_string *const_JG_TETH_name = crex_string_init_interned("JG_TETH", sizeof("JG_TETH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_TETH_name, &const_JG_TETH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_TETH_name);

	zval const_JG_WAW_value;
	ZVAL_LONG(&const_JG_WAW_value, U_JG_WAW);
	crex_string *const_JG_WAW_name = crex_string_init_interned("JG_WAW", sizeof("JG_WAW") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_WAW_name, &const_JG_WAW_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_WAW_name);

	zval const_JG_YEH_value;
	ZVAL_LONG(&const_JG_YEH_value, U_JG_YEH);
	crex_string *const_JG_YEH_name = crex_string_init_interned("JG_YEH", sizeof("JG_YEH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_YEH_name, &const_JG_YEH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_YEH_name);

	zval const_JG_YEH_BARREE_value;
	ZVAL_LONG(&const_JG_YEH_BARREE_value, U_JG_YEH_BARREE);
	crex_string *const_JG_YEH_BARREE_name = crex_string_init_interned("JG_YEH_BARREE", sizeof("JG_YEH_BARREE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_YEH_BARREE_name, &const_JG_YEH_BARREE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_YEH_BARREE_name);

	zval const_JG_YEH_WITH_TAIL_value;
	ZVAL_LONG(&const_JG_YEH_WITH_TAIL_value, U_JG_YEH_WITH_TAIL);
	crex_string *const_JG_YEH_WITH_TAIL_name = crex_string_init_interned("JG_YEH_WITH_TAIL", sizeof("JG_YEH_WITH_TAIL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_YEH_WITH_TAIL_name, &const_JG_YEH_WITH_TAIL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_YEH_WITH_TAIL_name);

	zval const_JG_YUDH_value;
	ZVAL_LONG(&const_JG_YUDH_value, U_JG_YUDH);
	crex_string *const_JG_YUDH_name = crex_string_init_interned("JG_YUDH", sizeof("JG_YUDH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_YUDH_name, &const_JG_YUDH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_YUDH_name);

	zval const_JG_YUDH_HE_value;
	ZVAL_LONG(&const_JG_YUDH_HE_value, U_JG_YUDH_HE);
	crex_string *const_JG_YUDH_HE_name = crex_string_init_interned("JG_YUDH_HE", sizeof("JG_YUDH_HE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_YUDH_HE_name, &const_JG_YUDH_HE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_YUDH_HE_name);

	zval const_JG_ZAIN_value;
	ZVAL_LONG(&const_JG_ZAIN_value, U_JG_ZAIN);
	crex_string *const_JG_ZAIN_name = crex_string_init_interned("JG_ZAIN", sizeof("JG_ZAIN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_ZAIN_name, &const_JG_ZAIN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_ZAIN_name);

	zval const_JG_FE_value;
	ZVAL_LONG(&const_JG_FE_value, U_JG_FE);
	crex_string *const_JG_FE_name = crex_string_init_interned("JG_FE", sizeof("JG_FE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_FE_name, &const_JG_FE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_FE_name);

	zval const_JG_KHAPH_value;
	ZVAL_LONG(&const_JG_KHAPH_value, U_JG_KHAPH);
	crex_string *const_JG_KHAPH_name = crex_string_init_interned("JG_KHAPH", sizeof("JG_KHAPH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_KHAPH_name, &const_JG_KHAPH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_KHAPH_name);

	zval const_JG_ZHAIN_value;
	ZVAL_LONG(&const_JG_ZHAIN_value, U_JG_ZHAIN);
	crex_string *const_JG_ZHAIN_name = crex_string_init_interned("JG_ZHAIN", sizeof("JG_ZHAIN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_ZHAIN_name, &const_JG_ZHAIN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_ZHAIN_name);

	zval const_JG_BURUSHASKI_YEH_BARREE_value;
	ZVAL_LONG(&const_JG_BURUSHASKI_YEH_BARREE_value, U_JG_BURUSHASKI_YEH_BARREE);
	crex_string *const_JG_BURUSHASKI_YEH_BARREE_name = crex_string_init_interned("JG_BURUSHASKI_YEH_BARREE", sizeof("JG_BURUSHASKI_YEH_BARREE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_BURUSHASKI_YEH_BARREE_name, &const_JG_BURUSHASKI_YEH_BARREE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_BURUSHASKI_YEH_BARREE_name);

	zval const_JG_FARSI_YEH_value;
	ZVAL_LONG(&const_JG_FARSI_YEH_value, U_JG_FARSI_YEH);
	crex_string *const_JG_FARSI_YEH_name = crex_string_init_interned("JG_FARSI_YEH", sizeof("JG_FARSI_YEH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_FARSI_YEH_name, &const_JG_FARSI_YEH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_FARSI_YEH_name);

	zval const_JG_NYA_value;
	ZVAL_LONG(&const_JG_NYA_value, U_JG_NYA);
	crex_string *const_JG_NYA_name = crex_string_init_interned("JG_NYA", sizeof("JG_NYA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_NYA_name, &const_JG_NYA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_NYA_name);

	zval const_JG_ROHINGYA_YEH_value;
	ZVAL_LONG(&const_JG_ROHINGYA_YEH_value, U_JG_ROHINGYA_YEH);
	crex_string *const_JG_ROHINGYA_YEH_name = crex_string_init_interned("JG_ROHINGYA_YEH", sizeof("JG_ROHINGYA_YEH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_ROHINGYA_YEH_name, &const_JG_ROHINGYA_YEH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_ROHINGYA_YEH_name);
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_ALEPH_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_ALEPH_value, U_JG_MANICHAEAN_ALEPH);
	crex_string *const_JG_MANICHAEAN_ALEPH_name = crex_string_init_interned("JG_MANICHAEAN_ALEPH", sizeof("JG_MANICHAEAN_ALEPH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_ALEPH_name, &const_JG_MANICHAEAN_ALEPH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_ALEPH_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_AYIN_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_AYIN_value, U_JG_MANICHAEAN_AYIN);
	crex_string *const_JG_MANICHAEAN_AYIN_name = crex_string_init_interned("JG_MANICHAEAN_AYIN", sizeof("JG_MANICHAEAN_AYIN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_AYIN_name, &const_JG_MANICHAEAN_AYIN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_AYIN_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_BETH_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_BETH_value, U_JG_MANICHAEAN_BETH);
	crex_string *const_JG_MANICHAEAN_BETH_name = crex_string_init_interned("JG_MANICHAEAN_BETH", sizeof("JG_MANICHAEAN_BETH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_BETH_name, &const_JG_MANICHAEAN_BETH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_BETH_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_DALETH_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_DALETH_value, U_JG_MANICHAEAN_DALETH);
	crex_string *const_JG_MANICHAEAN_DALETH_name = crex_string_init_interned("JG_MANICHAEAN_DALETH", sizeof("JG_MANICHAEAN_DALETH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_DALETH_name, &const_JG_MANICHAEAN_DALETH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_DALETH_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_DHAMEDH_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_DHAMEDH_value, U_JG_MANICHAEAN_DHAMEDH);
	crex_string *const_JG_MANICHAEAN_DHAMEDH_name = crex_string_init_interned("JG_MANICHAEAN_DHAMEDH", sizeof("JG_MANICHAEAN_DHAMEDH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_DHAMEDH_name, &const_JG_MANICHAEAN_DHAMEDH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_DHAMEDH_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_FIVE_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_FIVE_value, U_JG_MANICHAEAN_FIVE);
	crex_string *const_JG_MANICHAEAN_FIVE_name = crex_string_init_interned("JG_MANICHAEAN_FIVE", sizeof("JG_MANICHAEAN_FIVE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_FIVE_name, &const_JG_MANICHAEAN_FIVE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_FIVE_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_GIMEL_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_GIMEL_value, U_JG_MANICHAEAN_GIMEL);
	crex_string *const_JG_MANICHAEAN_GIMEL_name = crex_string_init_interned("JG_MANICHAEAN_GIMEL", sizeof("JG_MANICHAEAN_GIMEL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_GIMEL_name, &const_JG_MANICHAEAN_GIMEL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_GIMEL_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_HETH_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_HETH_value, U_JG_MANICHAEAN_HETH);
	crex_string *const_JG_MANICHAEAN_HETH_name = crex_string_init_interned("JG_MANICHAEAN_HETH", sizeof("JG_MANICHAEAN_HETH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_HETH_name, &const_JG_MANICHAEAN_HETH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_HETH_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_HUNDRED_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_HUNDRED_value, U_JG_MANICHAEAN_HUNDRED);
	crex_string *const_JG_MANICHAEAN_HUNDRED_name = crex_string_init_interned("JG_MANICHAEAN_HUNDRED", sizeof("JG_MANICHAEAN_HUNDRED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_HUNDRED_name, &const_JG_MANICHAEAN_HUNDRED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_HUNDRED_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_KAPH_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_KAPH_value, U_JG_MANICHAEAN_KAPH);
	crex_string *const_JG_MANICHAEAN_KAPH_name = crex_string_init_interned("JG_MANICHAEAN_KAPH", sizeof("JG_MANICHAEAN_KAPH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_KAPH_name, &const_JG_MANICHAEAN_KAPH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_KAPH_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_LAMEDH_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_LAMEDH_value, U_JG_MANICHAEAN_LAMEDH);
	crex_string *const_JG_MANICHAEAN_LAMEDH_name = crex_string_init_interned("JG_MANICHAEAN_LAMEDH", sizeof("JG_MANICHAEAN_LAMEDH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_LAMEDH_name, &const_JG_MANICHAEAN_LAMEDH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_LAMEDH_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_MEM_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_MEM_value, U_JG_MANICHAEAN_MEM);
	crex_string *const_JG_MANICHAEAN_MEM_name = crex_string_init_interned("JG_MANICHAEAN_MEM", sizeof("JG_MANICHAEAN_MEM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_MEM_name, &const_JG_MANICHAEAN_MEM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_MEM_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_NUN_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_NUN_value, U_JG_MANICHAEAN_NUN);
	crex_string *const_JG_MANICHAEAN_NUN_name = crex_string_init_interned("JG_MANICHAEAN_NUN", sizeof("JG_MANICHAEAN_NUN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_NUN_name, &const_JG_MANICHAEAN_NUN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_NUN_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_ONE_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_ONE_value, U_JG_MANICHAEAN_ONE);
	crex_string *const_JG_MANICHAEAN_ONE_name = crex_string_init_interned("JG_MANICHAEAN_ONE", sizeof("JG_MANICHAEAN_ONE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_ONE_name, &const_JG_MANICHAEAN_ONE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_ONE_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_PE_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_PE_value, U_JG_MANICHAEAN_PE);
	crex_string *const_JG_MANICHAEAN_PE_name = crex_string_init_interned("JG_MANICHAEAN_PE", sizeof("JG_MANICHAEAN_PE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_PE_name, &const_JG_MANICHAEAN_PE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_PE_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_QOPH_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_QOPH_value, U_JG_MANICHAEAN_QOPH);
	crex_string *const_JG_MANICHAEAN_QOPH_name = crex_string_init_interned("JG_MANICHAEAN_QOPH", sizeof("JG_MANICHAEAN_QOPH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_QOPH_name, &const_JG_MANICHAEAN_QOPH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_QOPH_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_RESH_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_RESH_value, U_JG_MANICHAEAN_RESH);
	crex_string *const_JG_MANICHAEAN_RESH_name = crex_string_init_interned("JG_MANICHAEAN_RESH", sizeof("JG_MANICHAEAN_RESH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_RESH_name, &const_JG_MANICHAEAN_RESH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_RESH_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_SADHE_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_SADHE_value, U_JG_MANICHAEAN_SADHE);
	crex_string *const_JG_MANICHAEAN_SADHE_name = crex_string_init_interned("JG_MANICHAEAN_SADHE", sizeof("JG_MANICHAEAN_SADHE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_SADHE_name, &const_JG_MANICHAEAN_SADHE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_SADHE_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_SAMEKH_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_SAMEKH_value, U_JG_MANICHAEAN_SAMEKH);
	crex_string *const_JG_MANICHAEAN_SAMEKH_name = crex_string_init_interned("JG_MANICHAEAN_SAMEKH", sizeof("JG_MANICHAEAN_SAMEKH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_SAMEKH_name, &const_JG_MANICHAEAN_SAMEKH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_SAMEKH_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_TAW_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_TAW_value, U_JG_MANICHAEAN_TAW);
	crex_string *const_JG_MANICHAEAN_TAW_name = crex_string_init_interned("JG_MANICHAEAN_TAW", sizeof("JG_MANICHAEAN_TAW") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_TAW_name, &const_JG_MANICHAEAN_TAW_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_TAW_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_TEN_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_TEN_value, U_JG_MANICHAEAN_TEN);
	crex_string *const_JG_MANICHAEAN_TEN_name = crex_string_init_interned("JG_MANICHAEAN_TEN", sizeof("JG_MANICHAEAN_TEN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_TEN_name, &const_JG_MANICHAEAN_TEN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_TEN_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_TETH_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_TETH_value, U_JG_MANICHAEAN_TETH);
	crex_string *const_JG_MANICHAEAN_TETH_name = crex_string_init_interned("JG_MANICHAEAN_TETH", sizeof("JG_MANICHAEAN_TETH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_TETH_name, &const_JG_MANICHAEAN_TETH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_TETH_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_THAMEDH_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_THAMEDH_value, U_JG_MANICHAEAN_THAMEDH);
	crex_string *const_JG_MANICHAEAN_THAMEDH_name = crex_string_init_interned("JG_MANICHAEAN_THAMEDH", sizeof("JG_MANICHAEAN_THAMEDH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_THAMEDH_name, &const_JG_MANICHAEAN_THAMEDH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_THAMEDH_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_TWENTY_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_TWENTY_value, U_JG_MANICHAEAN_TWENTY);
	crex_string *const_JG_MANICHAEAN_TWENTY_name = crex_string_init_interned("JG_MANICHAEAN_TWENTY", sizeof("JG_MANICHAEAN_TWENTY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_TWENTY_name, &const_JG_MANICHAEAN_TWENTY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_TWENTY_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_WAW_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_WAW_value, U_JG_MANICHAEAN_WAW);
	crex_string *const_JG_MANICHAEAN_WAW_name = crex_string_init_interned("JG_MANICHAEAN_WAW", sizeof("JG_MANICHAEAN_WAW") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_WAW_name, &const_JG_MANICHAEAN_WAW_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_WAW_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_YODH_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_YODH_value, U_JG_MANICHAEAN_YODH);
	crex_string *const_JG_MANICHAEAN_YODH_name = crex_string_init_interned("JG_MANICHAEAN_YODH", sizeof("JG_MANICHAEAN_YODH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_YODH_name, &const_JG_MANICHAEAN_YODH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_YODH_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_MANICHAEAN_ZAYIN_value;
	ZVAL_LONG(&const_JG_MANICHAEAN_ZAYIN_value, U_JG_MANICHAEAN_ZAYIN);
	crex_string *const_JG_MANICHAEAN_ZAYIN_name = crex_string_init_interned("JG_MANICHAEAN_ZAYIN", sizeof("JG_MANICHAEAN_ZAYIN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_MANICHAEAN_ZAYIN_name, &const_JG_MANICHAEAN_ZAYIN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_MANICHAEAN_ZAYIN_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54

	zval const_JG_STRAIGHT_WAW_value;
	ZVAL_LONG(&const_JG_STRAIGHT_WAW_value, U_JG_STRAIGHT_WAW);
	crex_string *const_JG_STRAIGHT_WAW_name = crex_string_init_interned("JG_STRAIGHT_WAW", sizeof("JG_STRAIGHT_WAW") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_STRAIGHT_WAW_name, &const_JG_STRAIGHT_WAW_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_STRAIGHT_WAW_name);
#endif

	zval const_JG_COUNT_value;
	ZVAL_LONG(&const_JG_COUNT_value, U_JG_COUNT);
	crex_string *const_JG_COUNT_name = crex_string_init_interned("JG_COUNT", sizeof("JG_COUNT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_JG_COUNT_name, &const_JG_COUNT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_JG_COUNT_name);

	zval const_GCB_OTHER_value;
	ZVAL_LONG(&const_GCB_OTHER_value, U_GCB_OTHER);
	crex_string *const_GCB_OTHER_name = crex_string_init_interned("GCB_OTHER", sizeof("GCB_OTHER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_GCB_OTHER_name, &const_GCB_OTHER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_GCB_OTHER_name);

	zval const_GCB_CONTROL_value;
	ZVAL_LONG(&const_GCB_CONTROL_value, U_GCB_CONTROL);
	crex_string *const_GCB_CONTROL_name = crex_string_init_interned("GCB_CONTROL", sizeof("GCB_CONTROL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_GCB_CONTROL_name, &const_GCB_CONTROL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_GCB_CONTROL_name);

	zval const_GCB_CR_value;
	ZVAL_LONG(&const_GCB_CR_value, U_GCB_CR);
	crex_string *const_GCB_CR_name = crex_string_init_interned("GCB_CR", sizeof("GCB_CR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_GCB_CR_name, &const_GCB_CR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_GCB_CR_name);

	zval const_GCB_EXTEND_value;
	ZVAL_LONG(&const_GCB_EXTEND_value, U_GCB_EXTEND);
	crex_string *const_GCB_EXTEND_name = crex_string_init_interned("GCB_EXTEND", sizeof("GCB_EXTEND") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_GCB_EXTEND_name, &const_GCB_EXTEND_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_GCB_EXTEND_name);

	zval const_GCB_L_value;
	ZVAL_LONG(&const_GCB_L_value, U_GCB_L);
	crex_string *const_GCB_L_name = crex_string_init_interned("GCB_L", sizeof("GCB_L") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_GCB_L_name, &const_GCB_L_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_GCB_L_name);

	zval const_GCB_LF_value;
	ZVAL_LONG(&const_GCB_LF_value, U_GCB_LF);
	crex_string *const_GCB_LF_name = crex_string_init_interned("GCB_LF", sizeof("GCB_LF") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_GCB_LF_name, &const_GCB_LF_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_GCB_LF_name);

	zval const_GCB_LV_value;
	ZVAL_LONG(&const_GCB_LV_value, U_GCB_LV);
	crex_string *const_GCB_LV_name = crex_string_init_interned("GCB_LV", sizeof("GCB_LV") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_GCB_LV_name, &const_GCB_LV_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_GCB_LV_name);

	zval const_GCB_LVT_value;
	ZVAL_LONG(&const_GCB_LVT_value, U_GCB_LVT);
	crex_string *const_GCB_LVT_name = crex_string_init_interned("GCB_LVT", sizeof("GCB_LVT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_GCB_LVT_name, &const_GCB_LVT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_GCB_LVT_name);

	zval const_GCB_T_value;
	ZVAL_LONG(&const_GCB_T_value, U_GCB_T);
	crex_string *const_GCB_T_name = crex_string_init_interned("GCB_T", sizeof("GCB_T") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_GCB_T_name, &const_GCB_T_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_GCB_T_name);

	zval const_GCB_V_value;
	ZVAL_LONG(&const_GCB_V_value, U_GCB_V);
	crex_string *const_GCB_V_name = crex_string_init_interned("GCB_V", sizeof("GCB_V") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_GCB_V_name, &const_GCB_V_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_GCB_V_name);

	zval const_GCB_SPACING_MARK_value;
	ZVAL_LONG(&const_GCB_SPACING_MARK_value, U_GCB_SPACING_MARK);
	crex_string *const_GCB_SPACING_MARK_name = crex_string_init_interned("GCB_SPACING_MARK", sizeof("GCB_SPACING_MARK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_GCB_SPACING_MARK_name, &const_GCB_SPACING_MARK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_GCB_SPACING_MARK_name);

	zval const_GCB_PREPEND_value;
	ZVAL_LONG(&const_GCB_PREPEND_value, U_GCB_PREPEND);
	crex_string *const_GCB_PREPEND_name = crex_string_init_interned("GCB_PREPEND", sizeof("GCB_PREPEND") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_GCB_PREPEND_name, &const_GCB_PREPEND_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_GCB_PREPEND_name);

	zval const_GCB_REGIONAL_INDICATOR_value;
	ZVAL_LONG(&const_GCB_REGIONAL_INDICATOR_value, U_GCB_REGIONAL_INDICATOR);
	crex_string *const_GCB_REGIONAL_INDICATOR_name = crex_string_init_interned("GCB_REGIONAL_INDICATOR", sizeof("GCB_REGIONAL_INDICATOR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_GCB_REGIONAL_INDICATOR_name, &const_GCB_REGIONAL_INDICATOR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_GCB_REGIONAL_INDICATOR_name);

	zval const_GCB_COUNT_value;
	ZVAL_LONG(&const_GCB_COUNT_value, U_GCB_COUNT);
	crex_string *const_GCB_COUNT_name = crex_string_init_interned("GCB_COUNT", sizeof("GCB_COUNT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_GCB_COUNT_name, &const_GCB_COUNT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_GCB_COUNT_name);

	zval const_WB_OTHER_value;
	ZVAL_LONG(&const_WB_OTHER_value, U_WB_OTHER);
	crex_string *const_WB_OTHER_name = crex_string_init_interned("WB_OTHER", sizeof("WB_OTHER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WB_OTHER_name, &const_WB_OTHER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WB_OTHER_name);

	zval const_WB_ALETTER_value;
	ZVAL_LONG(&const_WB_ALETTER_value, U_WB_ALETTER);
	crex_string *const_WB_ALETTER_name = crex_string_init_interned("WB_ALETTER", sizeof("WB_ALETTER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WB_ALETTER_name, &const_WB_ALETTER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WB_ALETTER_name);

	zval const_WB_FORMAT_value;
	ZVAL_LONG(&const_WB_FORMAT_value, U_WB_FORMAT);
	crex_string *const_WB_FORMAT_name = crex_string_init_interned("WB_FORMAT", sizeof("WB_FORMAT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WB_FORMAT_name, &const_WB_FORMAT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WB_FORMAT_name);

	zval const_WB_KATAKANA_value;
	ZVAL_LONG(&const_WB_KATAKANA_value, U_WB_KATAKANA);
	crex_string *const_WB_KATAKANA_name = crex_string_init_interned("WB_KATAKANA", sizeof("WB_KATAKANA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WB_KATAKANA_name, &const_WB_KATAKANA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WB_KATAKANA_name);

	zval const_WB_MIDLETTER_value;
	ZVAL_LONG(&const_WB_MIDLETTER_value, U_WB_MIDLETTER);
	crex_string *const_WB_MIDLETTER_name = crex_string_init_interned("WB_MIDLETTER", sizeof("WB_MIDLETTER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WB_MIDLETTER_name, &const_WB_MIDLETTER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WB_MIDLETTER_name);

	zval const_WB_MIDNUM_value;
	ZVAL_LONG(&const_WB_MIDNUM_value, U_WB_MIDNUM);
	crex_string *const_WB_MIDNUM_name = crex_string_init_interned("WB_MIDNUM", sizeof("WB_MIDNUM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WB_MIDNUM_name, &const_WB_MIDNUM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WB_MIDNUM_name);

	zval const_WB_NUMERIC_value;
	ZVAL_LONG(&const_WB_NUMERIC_value, U_WB_NUMERIC);
	crex_string *const_WB_NUMERIC_name = crex_string_init_interned("WB_NUMERIC", sizeof("WB_NUMERIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WB_NUMERIC_name, &const_WB_NUMERIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WB_NUMERIC_name);

	zval const_WB_EXTENDNUMLET_value;
	ZVAL_LONG(&const_WB_EXTENDNUMLET_value, U_WB_EXTENDNUMLET);
	crex_string *const_WB_EXTENDNUMLET_name = crex_string_init_interned("WB_EXTENDNUMLET", sizeof("WB_EXTENDNUMLET") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WB_EXTENDNUMLET_name, &const_WB_EXTENDNUMLET_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WB_EXTENDNUMLET_name);

	zval const_WB_CR_value;
	ZVAL_LONG(&const_WB_CR_value, U_WB_CR);
	crex_string *const_WB_CR_name = crex_string_init_interned("WB_CR", sizeof("WB_CR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WB_CR_name, &const_WB_CR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WB_CR_name);

	zval const_WB_EXTEND_value;
	ZVAL_LONG(&const_WB_EXTEND_value, U_WB_EXTEND);
	crex_string *const_WB_EXTEND_name = crex_string_init_interned("WB_EXTEND", sizeof("WB_EXTEND") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WB_EXTEND_name, &const_WB_EXTEND_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WB_EXTEND_name);

	zval const_WB_LF_value;
	ZVAL_LONG(&const_WB_LF_value, U_WB_LF);
	crex_string *const_WB_LF_name = crex_string_init_interned("WB_LF", sizeof("WB_LF") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WB_LF_name, &const_WB_LF_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WB_LF_name);

	zval const_WB_MIDNUMLET_value;
	ZVAL_LONG(&const_WB_MIDNUMLET_value, U_WB_MIDNUMLET);
	crex_string *const_WB_MIDNUMLET_name = crex_string_init_interned("WB_MIDNUMLET", sizeof("WB_MIDNUMLET") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WB_MIDNUMLET_name, &const_WB_MIDNUMLET_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WB_MIDNUMLET_name);

	zval const_WB_NEWLINE_value;
	ZVAL_LONG(&const_WB_NEWLINE_value, U_WB_NEWLINE);
	crex_string *const_WB_NEWLINE_name = crex_string_init_interned("WB_NEWLINE", sizeof("WB_NEWLINE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WB_NEWLINE_name, &const_WB_NEWLINE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WB_NEWLINE_name);

	zval const_WB_REGIONAL_INDICATOR_value;
	ZVAL_LONG(&const_WB_REGIONAL_INDICATOR_value, U_WB_REGIONAL_INDICATOR);
	crex_string *const_WB_REGIONAL_INDICATOR_name = crex_string_init_interned("WB_REGIONAL_INDICATOR", sizeof("WB_REGIONAL_INDICATOR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WB_REGIONAL_INDICATOR_name, &const_WB_REGIONAL_INDICATOR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WB_REGIONAL_INDICATOR_name);
#if U_ICU_VERSION_MAJOR_NUM >= 52

	zval const_WB_HEBREW_LETTER_value;
	ZVAL_LONG(&const_WB_HEBREW_LETTER_value, U_WB_HEBREW_LETTER);
	crex_string *const_WB_HEBREW_LETTER_name = crex_string_init_interned("WB_HEBREW_LETTER", sizeof("WB_HEBREW_LETTER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WB_HEBREW_LETTER_name, &const_WB_HEBREW_LETTER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WB_HEBREW_LETTER_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 52

	zval const_WB_SINGLE_QUOTE_value;
	ZVAL_LONG(&const_WB_SINGLE_QUOTE_value, U_WB_SINGLE_QUOTE);
	crex_string *const_WB_SINGLE_QUOTE_name = crex_string_init_interned("WB_SINGLE_QUOTE", sizeof("WB_SINGLE_QUOTE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WB_SINGLE_QUOTE_name, &const_WB_SINGLE_QUOTE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WB_SINGLE_QUOTE_name);
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 52

	zval const_WB_DOUBLE_QUOTE_value;
	ZVAL_LONG(&const_WB_DOUBLE_QUOTE_value, U_WB_DOUBLE_QUOTE);
	crex_string *const_WB_DOUBLE_QUOTE_name = crex_string_init_interned("WB_DOUBLE_QUOTE", sizeof("WB_DOUBLE_QUOTE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WB_DOUBLE_QUOTE_name, &const_WB_DOUBLE_QUOTE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WB_DOUBLE_QUOTE_name);
#endif

	zval const_WB_COUNT_value;
	ZVAL_LONG(&const_WB_COUNT_value, U_WB_COUNT);
	crex_string *const_WB_COUNT_name = crex_string_init_interned("WB_COUNT", sizeof("WB_COUNT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_WB_COUNT_name, &const_WB_COUNT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_WB_COUNT_name);

	zval const_SB_OTHER_value;
	ZVAL_LONG(&const_SB_OTHER_value, U_SB_OTHER);
	crex_string *const_SB_OTHER_name = crex_string_init_interned("SB_OTHER", sizeof("SB_OTHER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SB_OTHER_name, &const_SB_OTHER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SB_OTHER_name);

	zval const_SB_ATERM_value;
	ZVAL_LONG(&const_SB_ATERM_value, U_SB_ATERM);
	crex_string *const_SB_ATERM_name = crex_string_init_interned("SB_ATERM", sizeof("SB_ATERM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SB_ATERM_name, &const_SB_ATERM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SB_ATERM_name);

	zval const_SB_CLOSE_value;
	ZVAL_LONG(&const_SB_CLOSE_value, U_SB_CLOSE);
	crex_string *const_SB_CLOSE_name = crex_string_init_interned("SB_CLOSE", sizeof("SB_CLOSE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SB_CLOSE_name, &const_SB_CLOSE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SB_CLOSE_name);

	zval const_SB_FORMAT_value;
	ZVAL_LONG(&const_SB_FORMAT_value, U_SB_FORMAT);
	crex_string *const_SB_FORMAT_name = crex_string_init_interned("SB_FORMAT", sizeof("SB_FORMAT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SB_FORMAT_name, &const_SB_FORMAT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SB_FORMAT_name);

	zval const_SB_LOWER_value;
	ZVAL_LONG(&const_SB_LOWER_value, U_SB_LOWER);
	crex_string *const_SB_LOWER_name = crex_string_init_interned("SB_LOWER", sizeof("SB_LOWER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SB_LOWER_name, &const_SB_LOWER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SB_LOWER_name);

	zval const_SB_NUMERIC_value;
	ZVAL_LONG(&const_SB_NUMERIC_value, U_SB_NUMERIC);
	crex_string *const_SB_NUMERIC_name = crex_string_init_interned("SB_NUMERIC", sizeof("SB_NUMERIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SB_NUMERIC_name, &const_SB_NUMERIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SB_NUMERIC_name);

	zval const_SB_OLETTER_value;
	ZVAL_LONG(&const_SB_OLETTER_value, U_SB_OLETTER);
	crex_string *const_SB_OLETTER_name = crex_string_init_interned("SB_OLETTER", sizeof("SB_OLETTER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SB_OLETTER_name, &const_SB_OLETTER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SB_OLETTER_name);

	zval const_SB_SEP_value;
	ZVAL_LONG(&const_SB_SEP_value, U_SB_SEP);
	crex_string *const_SB_SEP_name = crex_string_init_interned("SB_SEP", sizeof("SB_SEP") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SB_SEP_name, &const_SB_SEP_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SB_SEP_name);

	zval const_SB_SP_value;
	ZVAL_LONG(&const_SB_SP_value, U_SB_SP);
	crex_string *const_SB_SP_name = crex_string_init_interned("SB_SP", sizeof("SB_SP") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SB_SP_name, &const_SB_SP_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SB_SP_name);

	zval const_SB_STERM_value;
	ZVAL_LONG(&const_SB_STERM_value, U_SB_STERM);
	crex_string *const_SB_STERM_name = crex_string_init_interned("SB_STERM", sizeof("SB_STERM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SB_STERM_name, &const_SB_STERM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SB_STERM_name);

	zval const_SB_UPPER_value;
	ZVAL_LONG(&const_SB_UPPER_value, U_SB_UPPER);
	crex_string *const_SB_UPPER_name = crex_string_init_interned("SB_UPPER", sizeof("SB_UPPER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SB_UPPER_name, &const_SB_UPPER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SB_UPPER_name);

	zval const_SB_CR_value;
	ZVAL_LONG(&const_SB_CR_value, U_SB_CR);
	crex_string *const_SB_CR_name = crex_string_init_interned("SB_CR", sizeof("SB_CR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SB_CR_name, &const_SB_CR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SB_CR_name);

	zval const_SB_EXTEND_value;
	ZVAL_LONG(&const_SB_EXTEND_value, U_SB_EXTEND);
	crex_string *const_SB_EXTEND_name = crex_string_init_interned("SB_EXTEND", sizeof("SB_EXTEND") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SB_EXTEND_name, &const_SB_EXTEND_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SB_EXTEND_name);

	zval const_SB_LF_value;
	ZVAL_LONG(&const_SB_LF_value, U_SB_LF);
	crex_string *const_SB_LF_name = crex_string_init_interned("SB_LF", sizeof("SB_LF") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SB_LF_name, &const_SB_LF_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SB_LF_name);

	zval const_SB_SCONTINUE_value;
	ZVAL_LONG(&const_SB_SCONTINUE_value, U_SB_SCONTINUE);
	crex_string *const_SB_SCONTINUE_name = crex_string_init_interned("SB_SCONTINUE", sizeof("SB_SCONTINUE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SB_SCONTINUE_name, &const_SB_SCONTINUE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SB_SCONTINUE_name);

	zval const_SB_COUNT_value;
	ZVAL_LONG(&const_SB_COUNT_value, U_SB_COUNT);
	crex_string *const_SB_COUNT_name = crex_string_init_interned("SB_COUNT", sizeof("SB_COUNT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SB_COUNT_name, &const_SB_COUNT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SB_COUNT_name);

	zval const_LB_UNKNOWN_value;
	ZVAL_LONG(&const_LB_UNKNOWN_value, U_LB_UNKNOWN);
	crex_string *const_LB_UNKNOWN_name = crex_string_init_interned("LB_UNKNOWN", sizeof("LB_UNKNOWN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_UNKNOWN_name, &const_LB_UNKNOWN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_UNKNOWN_name);

	zval const_LB_AMBIGUOUS_value;
	ZVAL_LONG(&const_LB_AMBIGUOUS_value, U_LB_AMBIGUOUS);
	crex_string *const_LB_AMBIGUOUS_name = crex_string_init_interned("LB_AMBIGUOUS", sizeof("LB_AMBIGUOUS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_AMBIGUOUS_name, &const_LB_AMBIGUOUS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_AMBIGUOUS_name);

	zval const_LB_ALPHABETIC_value;
	ZVAL_LONG(&const_LB_ALPHABETIC_value, U_LB_ALPHABETIC);
	crex_string *const_LB_ALPHABETIC_name = crex_string_init_interned("LB_ALPHABETIC", sizeof("LB_ALPHABETIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_ALPHABETIC_name, &const_LB_ALPHABETIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_ALPHABETIC_name);

	zval const_LB_BREAK_BOTH_value;
	ZVAL_LONG(&const_LB_BREAK_BOTH_value, U_LB_BREAK_BOTH);
	crex_string *const_LB_BREAK_BOTH_name = crex_string_init_interned("LB_BREAK_BOTH", sizeof("LB_BREAK_BOTH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_BREAK_BOTH_name, &const_LB_BREAK_BOTH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_BREAK_BOTH_name);

	zval const_LB_BREAK_AFTER_value;
	ZVAL_LONG(&const_LB_BREAK_AFTER_value, U_LB_BREAK_AFTER);
	crex_string *const_LB_BREAK_AFTER_name = crex_string_init_interned("LB_BREAK_AFTER", sizeof("LB_BREAK_AFTER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_BREAK_AFTER_name, &const_LB_BREAK_AFTER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_BREAK_AFTER_name);

	zval const_LB_BREAK_BEFORE_value;
	ZVAL_LONG(&const_LB_BREAK_BEFORE_value, U_LB_BREAK_BEFORE);
	crex_string *const_LB_BREAK_BEFORE_name = crex_string_init_interned("LB_BREAK_BEFORE", sizeof("LB_BREAK_BEFORE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_BREAK_BEFORE_name, &const_LB_BREAK_BEFORE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_BREAK_BEFORE_name);

	zval const_LB_MANDATORY_BREAK_value;
	ZVAL_LONG(&const_LB_MANDATORY_BREAK_value, U_LB_MANDATORY_BREAK);
	crex_string *const_LB_MANDATORY_BREAK_name = crex_string_init_interned("LB_MANDATORY_BREAK", sizeof("LB_MANDATORY_BREAK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_MANDATORY_BREAK_name, &const_LB_MANDATORY_BREAK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_MANDATORY_BREAK_name);

	zval const_LB_CONTINGENT_BREAK_value;
	ZVAL_LONG(&const_LB_CONTINGENT_BREAK_value, U_LB_CONTINGENT_BREAK);
	crex_string *const_LB_CONTINGENT_BREAK_name = crex_string_init_interned("LB_CONTINGENT_BREAK", sizeof("LB_CONTINGENT_BREAK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_CONTINGENT_BREAK_name, &const_LB_CONTINGENT_BREAK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_CONTINGENT_BREAK_name);

	zval const_LB_CLOSE_PUNCTUATION_value;
	ZVAL_LONG(&const_LB_CLOSE_PUNCTUATION_value, U_LB_CLOSE_PUNCTUATION);
	crex_string *const_LB_CLOSE_PUNCTUATION_name = crex_string_init_interned("LB_CLOSE_PUNCTUATION", sizeof("LB_CLOSE_PUNCTUATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_CLOSE_PUNCTUATION_name, &const_LB_CLOSE_PUNCTUATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_CLOSE_PUNCTUATION_name);

	zval const_LB_COMBINING_MARK_value;
	ZVAL_LONG(&const_LB_COMBINING_MARK_value, U_LB_COMBINING_MARK);
	crex_string *const_LB_COMBINING_MARK_name = crex_string_init_interned("LB_COMBINING_MARK", sizeof("LB_COMBINING_MARK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_COMBINING_MARK_name, &const_LB_COMBINING_MARK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_COMBINING_MARK_name);

	zval const_LB_CARRIAGE_RETURN_value;
	ZVAL_LONG(&const_LB_CARRIAGE_RETURN_value, U_LB_CARRIAGE_RETURN);
	crex_string *const_LB_CARRIAGE_RETURN_name = crex_string_init_interned("LB_CARRIAGE_RETURN", sizeof("LB_CARRIAGE_RETURN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_CARRIAGE_RETURN_name, &const_LB_CARRIAGE_RETURN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_CARRIAGE_RETURN_name);

	zval const_LB_EXCLAMATION_value;
	ZVAL_LONG(&const_LB_EXCLAMATION_value, U_LB_EXCLAMATION);
	crex_string *const_LB_EXCLAMATION_name = crex_string_init_interned("LB_EXCLAMATION", sizeof("LB_EXCLAMATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_EXCLAMATION_name, &const_LB_EXCLAMATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_EXCLAMATION_name);

	zval const_LB_GLUE_value;
	ZVAL_LONG(&const_LB_GLUE_value, U_LB_GLUE);
	crex_string *const_LB_GLUE_name = crex_string_init_interned("LB_GLUE", sizeof("LB_GLUE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_GLUE_name, &const_LB_GLUE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_GLUE_name);

	zval const_LB_HYPHEN_value;
	ZVAL_LONG(&const_LB_HYPHEN_value, U_LB_HYPHEN);
	crex_string *const_LB_HYPHEN_name = crex_string_init_interned("LB_HYPHEN", sizeof("LB_HYPHEN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_HYPHEN_name, &const_LB_HYPHEN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_HYPHEN_name);

	zval const_LB_IDEOGRAPHIC_value;
	ZVAL_LONG(&const_LB_IDEOGRAPHIC_value, U_LB_IDEOGRAPHIC);
	crex_string *const_LB_IDEOGRAPHIC_name = crex_string_init_interned("LB_IDEOGRAPHIC", sizeof("LB_IDEOGRAPHIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_IDEOGRAPHIC_name, &const_LB_IDEOGRAPHIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_IDEOGRAPHIC_name);

	zval const_LB_INSEPARABLE_value;
	ZVAL_LONG(&const_LB_INSEPARABLE_value, U_LB_INSEPARABLE);
	crex_string *const_LB_INSEPARABLE_name = crex_string_init_interned("LB_INSEPARABLE", sizeof("LB_INSEPARABLE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_INSEPARABLE_name, &const_LB_INSEPARABLE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_INSEPARABLE_name);

	zval const_LB_INSEPERABLE_value;
	ZVAL_LONG(&const_LB_INSEPERABLE_value, U_LB_INSEPERABLE);
	crex_string *const_LB_INSEPERABLE_name = crex_string_init_interned("LB_INSEPERABLE", sizeof("LB_INSEPERABLE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_INSEPERABLE_name, &const_LB_INSEPERABLE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_INSEPERABLE_name);

	zval const_LB_INFIX_NUMERIC_value;
	ZVAL_LONG(&const_LB_INFIX_NUMERIC_value, U_LB_INFIX_NUMERIC);
	crex_string *const_LB_INFIX_NUMERIC_name = crex_string_init_interned("LB_INFIX_NUMERIC", sizeof("LB_INFIX_NUMERIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_INFIX_NUMERIC_name, &const_LB_INFIX_NUMERIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_INFIX_NUMERIC_name);

	zval const_LB_LINE_FEED_value;
	ZVAL_LONG(&const_LB_LINE_FEED_value, U_LB_LINE_FEED);
	crex_string *const_LB_LINE_FEED_name = crex_string_init_interned("LB_LINE_FEED", sizeof("LB_LINE_FEED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_LINE_FEED_name, &const_LB_LINE_FEED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_LINE_FEED_name);

	zval const_LB_NONSTARTER_value;
	ZVAL_LONG(&const_LB_NONSTARTER_value, U_LB_NONSTARTER);
	crex_string *const_LB_NONSTARTER_name = crex_string_init_interned("LB_NONSTARTER", sizeof("LB_NONSTARTER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_NONSTARTER_name, &const_LB_NONSTARTER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_NONSTARTER_name);

	zval const_LB_NUMERIC_value;
	ZVAL_LONG(&const_LB_NUMERIC_value, U_LB_NUMERIC);
	crex_string *const_LB_NUMERIC_name = crex_string_init_interned("LB_NUMERIC", sizeof("LB_NUMERIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_NUMERIC_name, &const_LB_NUMERIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_NUMERIC_name);

	zval const_LB_OPEN_PUNCTUATION_value;
	ZVAL_LONG(&const_LB_OPEN_PUNCTUATION_value, U_LB_OPEN_PUNCTUATION);
	crex_string *const_LB_OPEN_PUNCTUATION_name = crex_string_init_interned("LB_OPEN_PUNCTUATION", sizeof("LB_OPEN_PUNCTUATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_OPEN_PUNCTUATION_name, &const_LB_OPEN_PUNCTUATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_OPEN_PUNCTUATION_name);

	zval const_LB_POSTFIX_NUMERIC_value;
	ZVAL_LONG(&const_LB_POSTFIX_NUMERIC_value, U_LB_POSTFIX_NUMERIC);
	crex_string *const_LB_POSTFIX_NUMERIC_name = crex_string_init_interned("LB_POSTFIX_NUMERIC", sizeof("LB_POSTFIX_NUMERIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_POSTFIX_NUMERIC_name, &const_LB_POSTFIX_NUMERIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_POSTFIX_NUMERIC_name);

	zval const_LB_PREFIX_NUMERIC_value;
	ZVAL_LONG(&const_LB_PREFIX_NUMERIC_value, U_LB_PREFIX_NUMERIC);
	crex_string *const_LB_PREFIX_NUMERIC_name = crex_string_init_interned("LB_PREFIX_NUMERIC", sizeof("LB_PREFIX_NUMERIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_PREFIX_NUMERIC_name, &const_LB_PREFIX_NUMERIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_PREFIX_NUMERIC_name);

	zval const_LB_QUOTATION_value;
	ZVAL_LONG(&const_LB_QUOTATION_value, U_LB_QUOTATION);
	crex_string *const_LB_QUOTATION_name = crex_string_init_interned("LB_QUOTATION", sizeof("LB_QUOTATION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_QUOTATION_name, &const_LB_QUOTATION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_QUOTATION_name);

	zval const_LB_COMPLEX_CONTEXT_value;
	ZVAL_LONG(&const_LB_COMPLEX_CONTEXT_value, U_LB_COMPLEX_CONTEXT);
	crex_string *const_LB_COMPLEX_CONTEXT_name = crex_string_init_interned("LB_COMPLEX_CONTEXT", sizeof("LB_COMPLEX_CONTEXT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_COMPLEX_CONTEXT_name, &const_LB_COMPLEX_CONTEXT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_COMPLEX_CONTEXT_name);

	zval const_LB_SURROGATE_value;
	ZVAL_LONG(&const_LB_SURROGATE_value, U_LB_SURROGATE);
	crex_string *const_LB_SURROGATE_name = crex_string_init_interned("LB_SURROGATE", sizeof("LB_SURROGATE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_SURROGATE_name, &const_LB_SURROGATE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_SURROGATE_name);

	zval const_LB_SPACE_value;
	ZVAL_LONG(&const_LB_SPACE_value, U_LB_SPACE);
	crex_string *const_LB_SPACE_name = crex_string_init_interned("LB_SPACE", sizeof("LB_SPACE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_SPACE_name, &const_LB_SPACE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_SPACE_name);

	zval const_LB_BREAK_SYMBOLS_value;
	ZVAL_LONG(&const_LB_BREAK_SYMBOLS_value, U_LB_BREAK_SYMBOLS);
	crex_string *const_LB_BREAK_SYMBOLS_name = crex_string_init_interned("LB_BREAK_SYMBOLS", sizeof("LB_BREAK_SYMBOLS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_BREAK_SYMBOLS_name, &const_LB_BREAK_SYMBOLS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_BREAK_SYMBOLS_name);

	zval const_LB_ZWSPACE_value;
	ZVAL_LONG(&const_LB_ZWSPACE_value, U_LB_ZWSPACE);
	crex_string *const_LB_ZWSPACE_name = crex_string_init_interned("LB_ZWSPACE", sizeof("LB_ZWSPACE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_ZWSPACE_name, &const_LB_ZWSPACE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_ZWSPACE_name);

	zval const_LB_NEXT_LINE_value;
	ZVAL_LONG(&const_LB_NEXT_LINE_value, U_LB_NEXT_LINE);
	crex_string *const_LB_NEXT_LINE_name = crex_string_init_interned("LB_NEXT_LINE", sizeof("LB_NEXT_LINE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_NEXT_LINE_name, &const_LB_NEXT_LINE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_NEXT_LINE_name);

	zval const_LB_WORD_JOINER_value;
	ZVAL_LONG(&const_LB_WORD_JOINER_value, U_LB_WORD_JOINER);
	crex_string *const_LB_WORD_JOINER_name = crex_string_init_interned("LB_WORD_JOINER", sizeof("LB_WORD_JOINER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_WORD_JOINER_name, &const_LB_WORD_JOINER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_WORD_JOINER_name);

	zval const_LB_H2_value;
	ZVAL_LONG(&const_LB_H2_value, U_LB_H2);
	crex_string *const_LB_H2_name = crex_string_init_interned("LB_H2", sizeof("LB_H2") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_H2_name, &const_LB_H2_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_H2_name);

	zval const_LB_H3_value;
	ZVAL_LONG(&const_LB_H3_value, U_LB_H3);
	crex_string *const_LB_H3_name = crex_string_init_interned("LB_H3", sizeof("LB_H3") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_H3_name, &const_LB_H3_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_H3_name);

	zval const_LB_JL_value;
	ZVAL_LONG(&const_LB_JL_value, U_LB_JL);
	crex_string *const_LB_JL_name = crex_string_init_interned("LB_JL", sizeof("LB_JL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_JL_name, &const_LB_JL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_JL_name);

	zval const_LB_JT_value;
	ZVAL_LONG(&const_LB_JT_value, U_LB_JT);
	crex_string *const_LB_JT_name = crex_string_init_interned("LB_JT", sizeof("LB_JT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_JT_name, &const_LB_JT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_JT_name);

	zval const_LB_JV_value;
	ZVAL_LONG(&const_LB_JV_value, U_LB_JV);
	crex_string *const_LB_JV_name = crex_string_init_interned("LB_JV", sizeof("LB_JV") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_JV_name, &const_LB_JV_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_JV_name);

	zval const_LB_CLOSE_PARENTHESIS_value;
	ZVAL_LONG(&const_LB_CLOSE_PARENTHESIS_value, U_LB_CLOSE_PARENTHESIS);
	crex_string *const_LB_CLOSE_PARENTHESIS_name = crex_string_init_interned("LB_CLOSE_PARENTHESIS", sizeof("LB_CLOSE_PARENTHESIS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_CLOSE_PARENTHESIS_name, &const_LB_CLOSE_PARENTHESIS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_CLOSE_PARENTHESIS_name);

	zval const_LB_CONDITIONAL_JAPANESE_STARTER_value;
	ZVAL_LONG(&const_LB_CONDITIONAL_JAPANESE_STARTER_value, U_LB_CONDITIONAL_JAPANESE_STARTER);
	crex_string *const_LB_CONDITIONAL_JAPANESE_STARTER_name = crex_string_init_interned("LB_CONDITIONAL_JAPANESE_STARTER", sizeof("LB_CONDITIONAL_JAPANESE_STARTER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_CONDITIONAL_JAPANESE_STARTER_name, &const_LB_CONDITIONAL_JAPANESE_STARTER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_CONDITIONAL_JAPANESE_STARTER_name);

	zval const_LB_HEBREW_LETTER_value;
	ZVAL_LONG(&const_LB_HEBREW_LETTER_value, U_LB_HEBREW_LETTER);
	crex_string *const_LB_HEBREW_LETTER_name = crex_string_init_interned("LB_HEBREW_LETTER", sizeof("LB_HEBREW_LETTER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_HEBREW_LETTER_name, &const_LB_HEBREW_LETTER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_HEBREW_LETTER_name);

	zval const_LB_REGIONAL_INDICATOR_value;
	ZVAL_LONG(&const_LB_REGIONAL_INDICATOR_value, U_LB_REGIONAL_INDICATOR);
	crex_string *const_LB_REGIONAL_INDICATOR_name = crex_string_init_interned("LB_REGIONAL_INDICATOR", sizeof("LB_REGIONAL_INDICATOR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_REGIONAL_INDICATOR_name, &const_LB_REGIONAL_INDICATOR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_REGIONAL_INDICATOR_name);

	zval const_LB_COUNT_value;
	ZVAL_LONG(&const_LB_COUNT_value, U_LB_COUNT);
	crex_string *const_LB_COUNT_name = crex_string_init_interned("LB_COUNT", sizeof("LB_COUNT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LB_COUNT_name, &const_LB_COUNT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LB_COUNT_name);

	zval const_NT_NONE_value;
	ZVAL_LONG(&const_NT_NONE_value, U_NT_NONE);
	crex_string *const_NT_NONE_name = crex_string_init_interned("NT_NONE", sizeof("NT_NONE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NT_NONE_name, &const_NT_NONE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NT_NONE_name);

	zval const_NT_DECIMAL_value;
	ZVAL_LONG(&const_NT_DECIMAL_value, U_NT_DECIMAL);
	crex_string *const_NT_DECIMAL_name = crex_string_init_interned("NT_DECIMAL", sizeof("NT_DECIMAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NT_DECIMAL_name, &const_NT_DECIMAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NT_DECIMAL_name);

	zval const_NT_DIGIT_value;
	ZVAL_LONG(&const_NT_DIGIT_value, U_NT_DIGIT);
	crex_string *const_NT_DIGIT_name = crex_string_init_interned("NT_DIGIT", sizeof("NT_DIGIT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NT_DIGIT_name, &const_NT_DIGIT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NT_DIGIT_name);

	zval const_NT_NUMERIC_value;
	ZVAL_LONG(&const_NT_NUMERIC_value, U_NT_NUMERIC);
	crex_string *const_NT_NUMERIC_name = crex_string_init_interned("NT_NUMERIC", sizeof("NT_NUMERIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NT_NUMERIC_name, &const_NT_NUMERIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NT_NUMERIC_name);

	zval const_NT_COUNT_value;
	ZVAL_LONG(&const_NT_COUNT_value, U_NT_COUNT);
	crex_string *const_NT_COUNT_name = crex_string_init_interned("NT_COUNT", sizeof("NT_COUNT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NT_COUNT_name, &const_NT_COUNT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NT_COUNT_name);

	zval const_HST_NOT_APPLICABLE_value;
	ZVAL_LONG(&const_HST_NOT_APPLICABLE_value, U_HST_NOT_APPLICABLE);
	crex_string *const_HST_NOT_APPLICABLE_name = crex_string_init_interned("HST_NOT_APPLICABLE", sizeof("HST_NOT_APPLICABLE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_HST_NOT_APPLICABLE_name, &const_HST_NOT_APPLICABLE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_HST_NOT_APPLICABLE_name);

	zval const_HST_LEADING_JAMO_value;
	ZVAL_LONG(&const_HST_LEADING_JAMO_value, U_HST_LEADING_JAMO);
	crex_string *const_HST_LEADING_JAMO_name = crex_string_init_interned("HST_LEADING_JAMO", sizeof("HST_LEADING_JAMO") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_HST_LEADING_JAMO_name, &const_HST_LEADING_JAMO_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_HST_LEADING_JAMO_name);

	zval const_HST_VOWEL_JAMO_value;
	ZVAL_LONG(&const_HST_VOWEL_JAMO_value, U_HST_VOWEL_JAMO);
	crex_string *const_HST_VOWEL_JAMO_name = crex_string_init_interned("HST_VOWEL_JAMO", sizeof("HST_VOWEL_JAMO") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_HST_VOWEL_JAMO_name, &const_HST_VOWEL_JAMO_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_HST_VOWEL_JAMO_name);

	zval const_HST_TRAILING_JAMO_value;
	ZVAL_LONG(&const_HST_TRAILING_JAMO_value, U_HST_TRAILING_JAMO);
	crex_string *const_HST_TRAILING_JAMO_name = crex_string_init_interned("HST_TRAILING_JAMO", sizeof("HST_TRAILING_JAMO") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_HST_TRAILING_JAMO_name, &const_HST_TRAILING_JAMO_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_HST_TRAILING_JAMO_name);

	zval const_HST_LV_SYLLABLE_value;
	ZVAL_LONG(&const_HST_LV_SYLLABLE_value, U_HST_LV_SYLLABLE);
	crex_string *const_HST_LV_SYLLABLE_name = crex_string_init_interned("HST_LV_SYLLABLE", sizeof("HST_LV_SYLLABLE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_HST_LV_SYLLABLE_name, &const_HST_LV_SYLLABLE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_HST_LV_SYLLABLE_name);

	zval const_HST_LVT_SYLLABLE_value;
	ZVAL_LONG(&const_HST_LVT_SYLLABLE_value, U_HST_LVT_SYLLABLE);
	crex_string *const_HST_LVT_SYLLABLE_name = crex_string_init_interned("HST_LVT_SYLLABLE", sizeof("HST_LVT_SYLLABLE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_HST_LVT_SYLLABLE_name, &const_HST_LVT_SYLLABLE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_HST_LVT_SYLLABLE_name);

	zval const_HST_COUNT_value;
	ZVAL_LONG(&const_HST_COUNT_value, U_HST_COUNT);
	crex_string *const_HST_COUNT_name = crex_string_init_interned("HST_COUNT", sizeof("HST_COUNT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_HST_COUNT_name, &const_HST_COUNT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_HST_COUNT_name);

	zval const_FOLD_CASE_DEFAULT_value;
	ZVAL_LONG(&const_FOLD_CASE_DEFAULT_value, U_FOLD_CASE_DEFAULT);
	crex_string *const_FOLD_CASE_DEFAULT_name = crex_string_init_interned("FOLD_CASE_DEFAULT", sizeof("FOLD_CASE_DEFAULT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FOLD_CASE_DEFAULT_name, &const_FOLD_CASE_DEFAULT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FOLD_CASE_DEFAULT_name);

	zval const_FOLD_CASE_EXCLUDE_SPECIAL_I_value;
	ZVAL_LONG(&const_FOLD_CASE_EXCLUDE_SPECIAL_I_value, U_FOLD_CASE_EXCLUDE_SPECIAL_I);
	crex_string *const_FOLD_CASE_EXCLUDE_SPECIAL_I_name = crex_string_init_interned("FOLD_CASE_EXCLUDE_SPECIAL_I", sizeof("FOLD_CASE_EXCLUDE_SPECIAL_I") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FOLD_CASE_EXCLUDE_SPECIAL_I_name, &const_FOLD_CASE_EXCLUDE_SPECIAL_I_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FOLD_CASE_EXCLUDE_SPECIAL_I_name);

	return class_entry;
}
