/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 141073d610f862b525406fb7f48ac58b6691080e */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mb_language, 0, 0, MAY_BE_STRING|MAY_BE_BOOL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, language, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mb_internal_encoding, 0, 0, MAY_BE_STRING|MAY_BE_BOOL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mb_http_input, 0, 0, MAY_BE_ARRAY|MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_mb_http_output arginfo_mb_internal_encoding

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mb_detect_order, 0, 0, MAY_BE_ARRAY|MAY_BE_BOOL)
	CREX_ARG_TYPE_MASK(0, encoding, MAY_BE_ARRAY|MAY_BE_STRING|MAY_BE_NULL, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mb_substitute_character, 0, 0, MAY_BE_STRING|MAY_BE_LONG|MAY_BE_BOOL)
	CREX_ARG_TYPE_MASK(0, substitute_character, MAY_BE_STRING|MAY_BE_LONG|MAY_BE_NULL, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mb_preferred_mime_name, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, encoding, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_parse_str, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_INFO(1, result)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_output_handler, 0, 2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, status, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_str_split, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 0, "1")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_strlen, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mb_strpos, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, haystack, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, needle, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_mb_strrpos arginfo_mb_strpos

#define arginfo_mb_stripos arginfo_mb_strpos

#define arginfo_mb_strripos arginfo_mb_strpos

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mb_strstr, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, haystack, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, needle, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, before_needle, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_mb_strrchr arginfo_mb_strstr

#define arginfo_mb_stristr arginfo_mb_strstr

#define arginfo_mb_strrichr arginfo_mb_strstr

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_substr_count, 0, 2, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, haystack, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, needle, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_substr, 0, 2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, start, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_mb_strcut arginfo_mb_substr

#define arginfo_mb_strwidth arginfo_mb_strlen

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_strimwidth, 0, 3, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, start, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, trim_marker, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mb_convert_encoding, 0, 2, MAY_BE_ARRAY|MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_MASK(0, string, MAY_BE_ARRAY|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO(0, to_encoding, IS_STRING, 0)
	CREX_ARG_TYPE_MASK(0, from_encoding, MAY_BE_ARRAY|MAY_BE_STRING|MAY_BE_NULL, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_convert_case, 0, 2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_strtoupper, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_mb_strtolower arginfo_mb_strtoupper

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mb_detect_encoding, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_MASK(0, encodings, MAY_BE_ARRAY|MAY_BE_STRING|MAY_BE_NULL, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, strict, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_list_encodings, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_encoding_aliases, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, encoding, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_encode_mimeheader, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, charset, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, transfer_encoding, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, newline, IS_STRING, 0, "\"\\r\\n\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, indent, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_decode_mimeheader, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_convert_kana, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_STRING, 0, "\"KV\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mb_convert_variables, 0, 3, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, to_encoding, IS_STRING, 0)
	CREX_ARG_TYPE_MASK(0, from_encoding, MAY_BE_ARRAY|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO(1, var, IS_MIXED, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(1, vars, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_encode_numericentity, 0, 2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, map, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, hex, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_decode_numericentity, 0, 2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, map, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_send_mail, 0, 3, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, to, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, subject, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, message, IS_STRING, 0)
	CREX_ARG_TYPE_MASK(0, additional_headers, MAY_BE_ARRAY|MAY_BE_STRING, "[]")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, additional_params, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mb_get_info, 0, 0, MAY_BE_ARRAY|MAY_BE_STRING|MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_STRING, 0, "\"all\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_check_encoding, 0, 0, _IS_BOOL, 0)
	CREX_ARG_TYPE_MASK(0, value, MAY_BE_ARRAY|MAY_BE_STRING|MAY_BE_NULL, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_mb_scrub arginfo_mb_strtoupper

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mb_ord, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mb_chr, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, codepoint, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_str_pad, 0, 2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pad_string, IS_STRING, 0, "\" \"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pad_type, IS_LONG, 0, "STR_PAD_RIGHT")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#if defined(HAVE_MBREGEX)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mb_regex_encoding, 0, 0, MAY_BE_STRING|MAY_BE_BOOL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_MBREGEX)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_ereg, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, matches, "null")
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_MBREGEX)
#define arginfo_mb_eregi arginfo_mb_ereg
#endif

#if defined(HAVE_MBREGEX)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mb_ereg_replace, 0, 3, MAY_BE_STRING|MAY_BE_FALSE|MAY_BE_NULL)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, replacement, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_STRING, 1, "null")
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_MBREGEX)
#define arginfo_mb_eregi_replace arginfo_mb_ereg_replace
#endif

#if defined(HAVE_MBREGEX)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mb_ereg_replace_callback, 0, 3, MAY_BE_STRING|MAY_BE_FALSE|MAY_BE_NULL)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_STRING, 1, "null")
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_MBREGEX)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mb_split, 0, 2, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, limit, IS_LONG, 0, "-1")
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_MBREGEX)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_ereg_match, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_STRING, 1, "null")
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_MBREGEX)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_ereg_search, 0, 0, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pattern, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_STRING, 1, "null")
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_MBREGEX)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mb_ereg_search_pos, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pattern, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_STRING, 1, "null")
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_MBREGEX)
#define arginfo_mb_ereg_search_regs arginfo_mb_ereg_search_pos
#endif

#if defined(HAVE_MBREGEX)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_ereg_search_init, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pattern, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_STRING, 1, "null")
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_MBREGEX)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mb_ereg_search_getregs, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_MBREGEX)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_ereg_search_getpos, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_MBREGEX)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_ereg_search_setpos, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_MBREGEX)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mb_regex_set_options, 0, 0, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_STRING, 1, "null")
CREX_END_ARG_INFO()
#endif


CREX_FUNCTION(mb_language);
CREX_FUNCTION(mb_internal_encoding);
CREX_FUNCTION(mb_http_input);
CREX_FUNCTION(mb_http_output);
CREX_FUNCTION(mb_detect_order);
CREX_FUNCTION(mb_substitute_character);
CREX_FUNCTION(mb_preferred_mime_name);
CREX_FUNCTION(mb_parse_str);
CREX_FUNCTION(mb_output_handler);
CREX_FUNCTION(mb_str_split);
CREX_FUNCTION(mb_strlen);
CREX_FUNCTION(mb_strpos);
CREX_FUNCTION(mb_strrpos);
CREX_FUNCTION(mb_stripos);
CREX_FUNCTION(mb_strripos);
CREX_FUNCTION(mb_strstr);
CREX_FUNCTION(mb_strrchr);
CREX_FUNCTION(mb_stristr);
CREX_FUNCTION(mb_strrichr);
CREX_FUNCTION(mb_substr_count);
CREX_FUNCTION(mb_substr);
CREX_FUNCTION(mb_strcut);
CREX_FUNCTION(mb_strwidth);
CREX_FUNCTION(mb_strimwidth);
CREX_FUNCTION(mb_convert_encoding);
CREX_FUNCTION(mb_convert_case);
CREX_FUNCTION(mb_strtoupper);
CREX_FUNCTION(mb_strtolower);
CREX_FUNCTION(mb_detect_encoding);
CREX_FUNCTION(mb_list_encodings);
CREX_FUNCTION(mb_encoding_aliases);
CREX_FUNCTION(mb_encode_mimeheader);
CREX_FUNCTION(mb_decode_mimeheader);
CREX_FUNCTION(mb_convert_kana);
CREX_FUNCTION(mb_convert_variables);
CREX_FUNCTION(mb_encode_numericentity);
CREX_FUNCTION(mb_decode_numericentity);
CREX_FUNCTION(mb_send_mail);
CREX_FUNCTION(mb_get_info);
CREX_FUNCTION(mb_check_encoding);
CREX_FUNCTION(mb_scrub);
CREX_FUNCTION(mb_ord);
CREX_FUNCTION(mb_chr);
CREX_FUNCTION(mb_str_pad);
#if defined(HAVE_MBREGEX)
CREX_FUNCTION(mb_regex_encoding);
#endif
#if defined(HAVE_MBREGEX)
CREX_FUNCTION(mb_ereg);
#endif
#if defined(HAVE_MBREGEX)
CREX_FUNCTION(mb_eregi);
#endif
#if defined(HAVE_MBREGEX)
CREX_FUNCTION(mb_ereg_replace);
#endif
#if defined(HAVE_MBREGEX)
CREX_FUNCTION(mb_eregi_replace);
#endif
#if defined(HAVE_MBREGEX)
CREX_FUNCTION(mb_ereg_replace_callback);
#endif
#if defined(HAVE_MBREGEX)
CREX_FUNCTION(mb_split);
#endif
#if defined(HAVE_MBREGEX)
CREX_FUNCTION(mb_ereg_match);
#endif
#if defined(HAVE_MBREGEX)
CREX_FUNCTION(mb_ereg_search);
#endif
#if defined(HAVE_MBREGEX)
CREX_FUNCTION(mb_ereg_search_pos);
#endif
#if defined(HAVE_MBREGEX)
CREX_FUNCTION(mb_ereg_search_regs);
#endif
#if defined(HAVE_MBREGEX)
CREX_FUNCTION(mb_ereg_search_init);
#endif
#if defined(HAVE_MBREGEX)
CREX_FUNCTION(mb_ereg_search_getregs);
#endif
#if defined(HAVE_MBREGEX)
CREX_FUNCTION(mb_ereg_search_getpos);
#endif
#if defined(HAVE_MBREGEX)
CREX_FUNCTION(mb_ereg_search_setpos);
#endif
#if defined(HAVE_MBREGEX)
CREX_FUNCTION(mb_regex_set_options);
#endif


static const crex_function_entry ext_functions[] = {
	CREX_FE(mb_language, arginfo_mb_language)
	CREX_FE(mb_internal_encoding, arginfo_mb_internal_encoding)
	CREX_FE(mb_http_input, arginfo_mb_http_input)
	CREX_FE(mb_http_output, arginfo_mb_http_output)
	CREX_FE(mb_detect_order, arginfo_mb_detect_order)
	CREX_FE(mb_substitute_character, arginfo_mb_substitute_character)
	CREX_FE(mb_preferred_mime_name, arginfo_mb_preferred_mime_name)
	CREX_FE(mb_parse_str, arginfo_mb_parse_str)
	CREX_FE(mb_output_handler, arginfo_mb_output_handler)
	CREX_FE(mb_str_split, arginfo_mb_str_split)
	CREX_FE(mb_strlen, arginfo_mb_strlen)
	CREX_FE(mb_strpos, arginfo_mb_strpos)
	CREX_FE(mb_strrpos, arginfo_mb_strrpos)
	CREX_FE(mb_stripos, arginfo_mb_stripos)
	CREX_FE(mb_strripos, arginfo_mb_strripos)
	CREX_FE(mb_strstr, arginfo_mb_strstr)
	CREX_FE(mb_strrchr, arginfo_mb_strrchr)
	CREX_FE(mb_stristr, arginfo_mb_stristr)
	CREX_FE(mb_strrichr, arginfo_mb_strrichr)
	CREX_FE(mb_substr_count, arginfo_mb_substr_count)
	CREX_FE(mb_substr, arginfo_mb_substr)
	CREX_FE(mb_strcut, arginfo_mb_strcut)
	CREX_FE(mb_strwidth, arginfo_mb_strwidth)
	CREX_FE(mb_strimwidth, arginfo_mb_strimwidth)
	CREX_FE(mb_convert_encoding, arginfo_mb_convert_encoding)
	CREX_FE(mb_convert_case, arginfo_mb_convert_case)
	CREX_FE(mb_strtoupper, arginfo_mb_strtoupper)
	CREX_FE(mb_strtolower, arginfo_mb_strtolower)
	CREX_FE(mb_detect_encoding, arginfo_mb_detect_encoding)
	CREX_FE(mb_list_encodings, arginfo_mb_list_encodings)
	CREX_FE(mb_encoding_aliases, arginfo_mb_encoding_aliases)
	CREX_FE(mb_encode_mimeheader, arginfo_mb_encode_mimeheader)
	CREX_FE(mb_decode_mimeheader, arginfo_mb_decode_mimeheader)
	CREX_FE(mb_convert_kana, arginfo_mb_convert_kana)
	CREX_FE(mb_convert_variables, arginfo_mb_convert_variables)
	CREX_FE(mb_encode_numericentity, arginfo_mb_encode_numericentity)
	CREX_FE(mb_decode_numericentity, arginfo_mb_decode_numericentity)
	CREX_FE(mb_send_mail, arginfo_mb_send_mail)
	CREX_FE(mb_get_info, arginfo_mb_get_info)
	CREX_FE(mb_check_encoding, arginfo_mb_check_encoding)
	CREX_FE(mb_scrub, arginfo_mb_scrub)
	CREX_FE(mb_ord, arginfo_mb_ord)
	CREX_FE(mb_chr, arginfo_mb_chr)
	CREX_FE(mb_str_pad, arginfo_mb_str_pad)
#if defined(HAVE_MBREGEX)
	CREX_FE(mb_regex_encoding, arginfo_mb_regex_encoding)
#endif
#if defined(HAVE_MBREGEX)
	CREX_FE(mb_ereg, arginfo_mb_ereg)
#endif
#if defined(HAVE_MBREGEX)
	CREX_FE(mb_eregi, arginfo_mb_eregi)
#endif
#if defined(HAVE_MBREGEX)
	CREX_FE(mb_ereg_replace, arginfo_mb_ereg_replace)
#endif
#if defined(HAVE_MBREGEX)
	CREX_FE(mb_eregi_replace, arginfo_mb_eregi_replace)
#endif
#if defined(HAVE_MBREGEX)
	CREX_FE(mb_ereg_replace_callback, arginfo_mb_ereg_replace_callback)
#endif
#if defined(HAVE_MBREGEX)
	CREX_FE(mb_split, arginfo_mb_split)
#endif
#if defined(HAVE_MBREGEX)
	CREX_FE(mb_ereg_match, arginfo_mb_ereg_match)
#endif
#if defined(HAVE_MBREGEX)
	CREX_FE(mb_ereg_search, arginfo_mb_ereg_search)
#endif
#if defined(HAVE_MBREGEX)
	CREX_FE(mb_ereg_search_pos, arginfo_mb_ereg_search_pos)
#endif
#if defined(HAVE_MBREGEX)
	CREX_FE(mb_ereg_search_regs, arginfo_mb_ereg_search_regs)
#endif
#if defined(HAVE_MBREGEX)
	CREX_FE(mb_ereg_search_init, arginfo_mb_ereg_search_init)
#endif
#if defined(HAVE_MBREGEX)
	CREX_FE(mb_ereg_search_getregs, arginfo_mb_ereg_search_getregs)
#endif
#if defined(HAVE_MBREGEX)
	CREX_FE(mb_ereg_search_getpos, arginfo_mb_ereg_search_getpos)
#endif
#if defined(HAVE_MBREGEX)
	CREX_FE(mb_ereg_search_setpos, arginfo_mb_ereg_search_setpos)
#endif
#if defined(HAVE_MBREGEX)
	CREX_FE(mb_regex_set_options, arginfo_mb_regex_set_options)
#endif
	CREX_FE_END
};

static void register_mbstring_symbols(int module_number)
{
#if defined(HAVE_MBREGEX)
	REGISTER_STRING_CONSTANT("MB_ONIGURUMA_VERSION", crx_mb_oniguruma_version, CONST_PERSISTENT);
#endif
	REGISTER_LONG_CONSTANT("MB_CASE_UPPER", CRX_UNICODE_CASE_UPPER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MB_CASE_LOWER", CRX_UNICODE_CASE_LOWER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MB_CASE_TITLE", CRX_UNICODE_CASE_TITLE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MB_CASE_FOLD", CRX_UNICODE_CASE_FOLD, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MB_CASE_UPPER_SIMPLE", CRX_UNICODE_CASE_UPPER_SIMPLE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MB_CASE_LOWER_SIMPLE", CRX_UNICODE_CASE_LOWER_SIMPLE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MB_CASE_TITLE_SIMPLE", CRX_UNICODE_CASE_TITLE_SIMPLE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MB_CASE_FOLD_SIMPLE", CRX_UNICODE_CASE_FOLD_SIMPLE, CONST_PERSISTENT);
}
