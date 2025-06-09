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

#ifndef CREX_MULTIBYTE_H
#define CREX_MULTIBYTE_H

typedef struct _crex_encoding crex_encoding;

typedef size_t (*crex_encoding_filter)(unsigned char **str, size_t *str_length, const unsigned char *buf, size_t length);

typedef const crex_encoding* (*crex_encoding_fetcher)(const char *encoding_name);
typedef const char* (*crex_encoding_name_getter)(const crex_encoding *encoding);
typedef bool (*crex_encoding_lexer_compatibility_checker)(const crex_encoding *encoding);
typedef const crex_encoding *(*crex_encoding_detector)(const unsigned char *string, size_t length, const crex_encoding **list, size_t list_size);
typedef size_t (*crex_encoding_converter)(unsigned char **to, size_t *to_length, const unsigned char *from, size_t from_length, const crex_encoding *encoding_to, const crex_encoding *encoding_from);
typedef crex_result (*crex_encoding_list_parser)(const char *encoding_list, size_t encoding_list_len, const crex_encoding ***return_list, size_t *return_size, bool persistent);
typedef const crex_encoding *(*crex_encoding_internal_encoding_getter)(void);
typedef crex_result (*crex_encoding_internal_encoding_setter)(const crex_encoding *encoding);

typedef struct _crex_multibyte_functions {
    const char *provider_name;
    crex_encoding_fetcher encoding_fetcher;
    crex_encoding_name_getter encoding_name_getter;
    crex_encoding_lexer_compatibility_checker lexer_compatibility_checker;
    crex_encoding_detector encoding_detector;
    crex_encoding_converter encoding_converter;
    crex_encoding_list_parser encoding_list_parser;
    crex_encoding_internal_encoding_getter internal_encoding_getter;
    crex_encoding_internal_encoding_setter internal_encoding_setter;
} crex_multibyte_functions;

/*
 * crex multibyte APIs
 */
BEGIN_EXTERN_C()

CREX_API extern const crex_encoding *crex_multibyte_encoding_utf32be;
CREX_API extern const crex_encoding *crex_multibyte_encoding_utf32le;
CREX_API extern const crex_encoding *crex_multibyte_encoding_utf16be;
CREX_API extern const crex_encoding *crex_multibyte_encoding_utf16le;
CREX_API extern const crex_encoding *crex_multibyte_encoding_utf8;

/* multibyte utility functions */
CREX_API crex_result crex_multibyte_set_functions(const crex_multibyte_functions *functions);
CREX_API void crex_multibyte_restore_functions(void);
CREX_API const crex_multibyte_functions *crex_multibyte_get_functions(void);

CREX_API const crex_encoding *crex_multibyte_fetch_encoding(const char *name);
CREX_API const char *crex_multibyte_get_encoding_name(const crex_encoding *encoding);
CREX_API int crex_multibyte_check_lexer_compatibility(const crex_encoding *encoding);
CREX_API const crex_encoding *crex_multibyte_encoding_detector(const unsigned char *string, size_t length, const crex_encoding **list, size_t list_size);
CREX_API size_t crex_multibyte_encoding_converter(unsigned char **to, size_t *to_length, const unsigned char *from, size_t from_length, const crex_encoding *encoding_to, const crex_encoding *encoding_from);
CREX_API crex_result crex_multibyte_parse_encoding_list(const char *encoding_list, size_t encoding_list_len, const crex_encoding ***return_list, size_t *return_size, bool persistent);

CREX_API const crex_encoding *crex_multibyte_get_internal_encoding(void);
CREX_API const crex_encoding *crex_multibyte_get_script_encoding(void);
CREX_API int crex_multibyte_set_script_encoding(const crex_encoding **encoding_list, size_t encoding_list_size);
CREX_API crex_result crex_multibyte_set_internal_encoding(const crex_encoding *encoding);
CREX_API crex_result crex_multibyte_set_script_encoding_by_string(const char *new_value, size_t new_value_length);

END_EXTERN_C()

#endif /* CREX_MULTIBYTE_H */
