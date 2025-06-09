/*
  +----------------------------------------------------------------------+
  | Copyright (c) The CRX Group                                          |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the CRX license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | https://www.crx.net/license/3_01.txt                                 |
  | If you did not receive a copy of the CRX license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@crx.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Jakub Zelenka <bukka@crx.net>                                |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_JSON_PARSER_H
#define	CRX_JSON_PARSER_H

#include "crx.h"
#include "crx_json_scanner.h"

typedef struct _crx_json_parser crx_json_parser;

typedef int (*crx_json_parser_func_array_create_t)(
		crx_json_parser *parser, zval *array);
typedef int (*crx_json_parser_func_array_append_t)(
		crx_json_parser *parser, zval *array, zval *zvalue);
typedef int (*crx_json_parser_func_array_start_t)(
		crx_json_parser *parser);
typedef int (*crx_json_parser_func_array_end_t)(
		crx_json_parser *parser, zval *object);
typedef int (*crx_json_parser_func_object_create_t)(
		crx_json_parser *parser, zval *object);
typedef int (*crx_json_parser_func_object_update_t)(
		crx_json_parser *parser, zval *object, crex_string *key, zval *zvalue);
typedef int (*crx_json_parser_func_object_start_t)(
		crx_json_parser *parser);
typedef int (*crx_json_parser_func_object_end_t)(
		crx_json_parser *parser, zval *object);

typedef struct _crx_json_parser_methods {
	crx_json_parser_func_array_create_t array_create;
	crx_json_parser_func_array_append_t array_append;
	crx_json_parser_func_array_start_t array_start;
	crx_json_parser_func_array_end_t array_end;
	crx_json_parser_func_object_create_t object_create;
	crx_json_parser_func_object_update_t object_update;
	crx_json_parser_func_object_start_t object_start;
	crx_json_parser_func_object_end_t object_end;
} crx_json_parser_methods;

struct _crx_json_parser {
	crx_json_scanner scanner;
	zval *return_value;
	int depth;
	int max_depth;
	crx_json_parser_methods methods;
};

CRX_JSON_API void crx_json_parser_init_ex(
		crx_json_parser *parser,
		zval *return_value,
		const char *str,
		size_t str_len,
		int options,
		int max_depth,
		const crx_json_parser_methods *methods);

CRX_JSON_API void crx_json_parser_init(
		crx_json_parser *parser,
		zval *return_value,
		const char *str,
		size_t str_len,
		int options,
		int max_depth);

CRX_JSON_API crx_json_error_code crx_json_parser_error_code(const crx_json_parser *parser);

CRX_JSON_API int crx_json_parse(crx_json_parser *parser);

int crx_json_yyparse(crx_json_parser *parser);

const crx_json_parser_methods* crx_json_get_validate_methods(void);

#endif	/* CRX_JSON_PARSER_H */
