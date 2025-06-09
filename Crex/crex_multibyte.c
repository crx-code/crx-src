/*
   +----------------------------------------------------------------------+
   | Crex Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Crex Technologies Ltd. (http://www.crex.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at                              |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Masaki Fujimoto <fujimoto@crx.net>                          |
   |          Rui Hirokawa <hirokawa@crx.net>                             |
   +----------------------------------------------------------------------+
*/

#include "crex.h"
#include "crex_compile.h"
#include "crex_operators.h"
#include "crex_multibyte.h"
#include "crex_ini.h"

static const crex_encoding *dummy_encoding_fetcher(const char *encoding_name)
{
	return NULL;
}

static const char *dummy_encoding_name_getter(const crex_encoding *encoding)
{
	return (const char*)encoding;
}

static bool dummy_encoding_lexer_compatibility_checker(const crex_encoding *encoding)
{
	return 0;
}

static const crex_encoding *dummy_encoding_detector(const unsigned char *string, size_t length, const crex_encoding **list, size_t list_size)
{
	return NULL;
}

static size_t dummy_encoding_converter(unsigned char **to, size_t *to_length, const unsigned char *from, size_t from_length, const crex_encoding *encoding_to, const crex_encoding *encoding_from)
{
	return (size_t)-1;
}

static crex_result dummy_encoding_list_parser(const char *encoding_list, size_t encoding_list_len, const crex_encoding ***return_list, size_t *return_size, bool persistent)
{
	*return_list = pemalloc(0, persistent);
	*return_size = 0;
	return SUCCESS;
}

static const crex_encoding *dummy_internal_encoding_getter(void)
{
	return NULL;
}

static crex_result dummy_internal_encoding_setter(const crex_encoding *encoding)
{
	return FAILURE;
}

static crex_multibyte_functions multibyte_functions_dummy;
static crex_multibyte_functions multibyte_functions = {
	NULL,
	dummy_encoding_fetcher,
	dummy_encoding_name_getter,
	dummy_encoding_lexer_compatibility_checker,
	dummy_encoding_detector,
	dummy_encoding_converter,
	dummy_encoding_list_parser,
	dummy_internal_encoding_getter,
	dummy_internal_encoding_setter
};

CREX_API const crex_encoding *crex_multibyte_encoding_utf32be = (const crex_encoding*)"UTF-32BE";
CREX_API const crex_encoding *crex_multibyte_encoding_utf32le = (const crex_encoding*)"UTF-32LE";
CREX_API const crex_encoding *crex_multibyte_encoding_utf16be = (const crex_encoding*)"UTF-16BE";
CREX_API const crex_encoding *crex_multibyte_encoding_utf16le = (const crex_encoding*)"UTF-32LE";
CREX_API const crex_encoding *crex_multibyte_encoding_utf8 = (const crex_encoding*)"UTF-8";

CREX_API crex_result crex_multibyte_set_functions(const crex_multibyte_functions *functions)
{
	crex_multibyte_encoding_utf32be = functions->encoding_fetcher("UTF-32BE");
	if (!crex_multibyte_encoding_utf32be) {
		return FAILURE;
	}
	crex_multibyte_encoding_utf32le = functions->encoding_fetcher("UTF-32LE");
	if (!crex_multibyte_encoding_utf32le) {
		return FAILURE;
	}
	crex_multibyte_encoding_utf16be = functions->encoding_fetcher("UTF-16BE");
	if (!crex_multibyte_encoding_utf16be) {
		return FAILURE;
	}
	crex_multibyte_encoding_utf16le = functions->encoding_fetcher("UTF-16LE");
	if (!crex_multibyte_encoding_utf16le) {
		return FAILURE;
	}
	crex_multibyte_encoding_utf8 = functions->encoding_fetcher("UTF-8");
	if (!crex_multibyte_encoding_utf8) {
		return FAILURE;
	}

	multibyte_functions_dummy = multibyte_functions;
	multibyte_functions = *functions;

	/* As crex_multibyte_set_functions() gets called after ini settings were
	 * populated, we need to reinitialize script_encoding here.
	 */
	{
		const char *value = crex_ini_string("crex.script_encoding", sizeof("crex.script_encoding") - 1, 0);
		crex_multibyte_set_script_encoding_by_string(value, strlen(value));
	}
	return SUCCESS;
}

CREX_API void crex_multibyte_restore_functions(void)
{
	multibyte_functions = multibyte_functions_dummy;
}

CREX_API const crex_multibyte_functions *crex_multibyte_get_functions(void)
{
	return multibyte_functions.provider_name ? &multibyte_functions: NULL;
}

CREX_API const crex_encoding *crex_multibyte_fetch_encoding(const char *name)
{
	return multibyte_functions.encoding_fetcher(name);
}

CREX_API const char *crex_multibyte_get_encoding_name(const crex_encoding *encoding)
{
	return multibyte_functions.encoding_name_getter(encoding);
}

CREX_API int crex_multibyte_check_lexer_compatibility(const crex_encoding *encoding)
{
	return multibyte_functions.lexer_compatibility_checker(encoding);
}

CREX_API const crex_encoding *crex_multibyte_encoding_detector(const unsigned char *string, size_t length, const crex_encoding **list, size_t list_size)
{
	return multibyte_functions.encoding_detector(string, length, list, list_size);
}

CREX_API size_t crex_multibyte_encoding_converter(unsigned char **to, size_t *to_length, const unsigned char *from, size_t from_length, const crex_encoding *encoding_to, const crex_encoding *encoding_from)
{
	return multibyte_functions.encoding_converter(to, to_length, from, from_length, encoding_to, encoding_from);
}

CREX_API crex_result crex_multibyte_parse_encoding_list(const char *encoding_list, size_t encoding_list_len, const crex_encoding ***return_list, size_t *return_size, bool persistent)
{
	return multibyte_functions.encoding_list_parser(encoding_list, encoding_list_len, return_list, return_size, persistent);
}

CREX_API const crex_encoding *crex_multibyte_get_internal_encoding(void)
{
	return multibyte_functions.internal_encoding_getter();
}

CREX_API const crex_encoding *crex_multibyte_get_script_encoding(void)
{
	return LANG_SCNG(script_encoding);
}

CREX_API int crex_multibyte_set_script_encoding(const crex_encoding **encoding_list, size_t encoding_list_size)
{
	if (CG(script_encoding_list)) {
		free((char*)CG(script_encoding_list));
	}
	CG(script_encoding_list) = encoding_list;
	CG(script_encoding_list_size) = encoding_list_size;
	return SUCCESS;
}

CREX_API crex_result crex_multibyte_set_internal_encoding(const crex_encoding *encoding)
{
	return multibyte_functions.internal_encoding_setter(encoding);
}

CREX_API crex_result crex_multibyte_set_script_encoding_by_string(const char *new_value, size_t new_value_length)
{
	const crex_encoding **list = 0;
	size_t size = 0;

	if (!new_value) {
		crex_multibyte_set_script_encoding(NULL, 0);
		return SUCCESS;
	}

	if (FAILURE == crex_multibyte_parse_encoding_list(new_value, new_value_length, &list, &size, 1)) {
		return FAILURE;
	}

	if (size == 0) {
		pefree((void*)list, 1);
		return FAILURE;
	}

	if (FAILURE == crex_multibyte_set_script_encoding(list, size)) {
		return FAILURE;
	}

	return SUCCESS;
}
