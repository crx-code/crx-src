/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 4367fa431d3e4814e42d9aa514c10cae1d842d8f */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_iconv_strlen, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_iconv_substr, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_iconv_strpos, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, haystack, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, needle, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_iconv_strrpos, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, haystack, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, needle, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_iconv_mime_encode, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, field_name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, field_value, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_iconv_mime_decode, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_iconv_mime_decode_headers, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, headers, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_iconv, 0, 3, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, from_encoding, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, to_encoding, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_iconv_set_encoding, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, type, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, encoding, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_iconv_get_encoding, 0, 0, MAY_BE_ARRAY|MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_STRING, 0, "\"all\"")
CREX_END_ARG_INFO()


CREX_FUNCTION(iconv_strlen);
CREX_FUNCTION(iconv_substr);
CREX_FUNCTION(iconv_strpos);
CREX_FUNCTION(iconv_strrpos);
CREX_FUNCTION(iconv_mime_encode);
CREX_FUNCTION(iconv_mime_decode);
CREX_FUNCTION(iconv_mime_decode_headers);
CREX_FUNCTION(iconv);
CREX_FUNCTION(iconv_set_encoding);
CREX_FUNCTION(iconv_get_encoding);


static const crex_function_entry ext_functions[] = {
	CREX_FE(iconv_strlen, arginfo_iconv_strlen)
	CREX_FE(iconv_substr, arginfo_iconv_substr)
	CREX_FE(iconv_strpos, arginfo_iconv_strpos)
	CREX_FE(iconv_strrpos, arginfo_iconv_strrpos)
	CREX_FE(iconv_mime_encode, arginfo_iconv_mime_encode)
	CREX_FE(iconv_mime_decode, arginfo_iconv_mime_decode)
	CREX_FE(iconv_mime_decode_headers, arginfo_iconv_mime_decode_headers)
	CREX_FE(iconv, arginfo_iconv)
	CREX_FE(iconv_set_encoding, arginfo_iconv_set_encoding)
	CREX_FE(iconv_get_encoding, arginfo_iconv_get_encoding)
	CREX_FE_END
};

static void register_iconv_symbols(int module_number)
{
	REGISTER_STRING_CONSTANT("ICONV_IMPL", CRX_ICONV_IMPL_VALUE, CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("ICONV_VERSION", get_iconv_version(), CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ICONV_MIME_DECODE_STRICT", CRX_ICONV_MIME_DECODE_STRICT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ICONV_MIME_DECODE_CONTINUE_ON_ERROR", CRX_ICONV_MIME_DECODE_CONTINUE_ON_ERROR, CONST_PERSISTENT);
}
