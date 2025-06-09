%require "3.0"
%code top {
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

#include "crx.h"
#include "crx_json.h"
#include "crx_json_parser.h"

#define YYDEBUG 0

#if YYDEBUG
int json_yydebug = 1;
#endif

#ifdef _MSC_VER
#define YYMALLOC malloc
#define YYFREE free
#endif

#define CRX_JSON_DEPTH_DEC --parser->depth
#define CRX_JSON_DEPTH_INC \
	if (parser->max_depth && parser->depth >= parser->max_depth) { \
		parser->scanner.errcode = CRX_JSON_ERROR_DEPTH; \
		YYERROR; \
	} \
	++parser->depth

}

%define api.prefix {crx_json_yy}
%define api.pure full
%param  { crx_json_parser *parser  }

%union {
	zval value;
}


%token <value> CRX_JSON_T_NUL
%token <value> CRX_JSON_T_TRUE
%token <value> CRX_JSON_T_FALSE
%token <value> CRX_JSON_T_INT
%token <value> CRX_JSON_T_DOUBLE
%token <value> CRX_JSON_T_STRING
%token <value> CRX_JSON_T_ESTRING
%token CRX_JSON_T_EOI
%token CRX_JSON_T_ERROR

%type <value> start object key value array
%type <value> members member elements element

%destructor { zval_ptr_dtor_nogc(&$$); } <value>

%code {
static int crx_json_yylex(union YYSTYPE *value, crx_json_parser *parser);
static void crx_json_yyerror(crx_json_parser *parser, char const *msg);
static int crx_json_parser_array_create(crx_json_parser *parser, zval *array);
static int crx_json_parser_object_create(crx_json_parser *parser, zval *array);

}

%% /* Rules */

start:
		value CRX_JSON_T_EOI
			{
				ZVAL_COPY_VALUE(&$$, &$1);
				ZVAL_COPY_VALUE(parser->return_value, &$1);
				(void) crx_json_yynerrs;
				YYACCEPT;
			}
;

object:
		'{'
			{
				CRX_JSON_DEPTH_INC;
				if (parser->methods.object_start && FAILURE == parser->methods.object_start(parser)) {
					YYERROR;
				}
			}
		members object_end
			{
				ZVAL_COPY_VALUE(&$$, &$3);
				CRX_JSON_DEPTH_DEC;
				if (parser->methods.object_end && FAILURE == parser->methods.object_end(parser, &$$)) {
					YYERROR;
				}
			}
;

object_end:
		'}'
	|	']'
			{
				parser->scanner.errcode = CRX_JSON_ERROR_STATE_MISMATCH;
				YYERROR;
			}
;

members:
		%empty
			{
				if ((parser->scanner.options & CRX_JSON_OBJECT_AS_ARRAY) && parser->methods.object_create == crx_json_parser_object_create) {
					ZVAL_EMPTY_ARRAY(&$$);
				} else {
					parser->methods.object_create(parser, &$$);
				}
			}
	|	member
;

member:
		key ':' value
			{
				parser->methods.object_create(parser, &$$);
				if (parser->methods.object_update(parser, &$$, C_STR($1), &$3) == FAILURE) {
					YYERROR;
				}
			}
	|	member ',' key ':' value
			{
				if (parser->methods.object_update(parser, &$1, C_STR($3), &$5) == FAILURE) {
					YYERROR;
				}
				ZVAL_COPY_VALUE(&$$, &$1);
			}
;

array:
		'['
			{
				CRX_JSON_DEPTH_INC;
				if (parser->methods.array_start && FAILURE == parser->methods.array_start(parser)) {
					YYERROR;
				}
			}
		elements array_end
			{
				ZVAL_COPY_VALUE(&$$, &$3);
				CRX_JSON_DEPTH_DEC;
				if (parser->methods.array_end && FAILURE == parser->methods.array_end(parser, &$$)) {
					YYERROR;
				}
			}
;

array_end:
		']'
	|	'}'
			{
				parser->scanner.errcode = CRX_JSON_ERROR_STATE_MISMATCH;
				YYERROR;
			}
;

elements:
		%empty
			{
				if (parser->methods.array_create == crx_json_parser_array_create) {
					ZVAL_EMPTY_ARRAY(&$$);
				} else {
					parser->methods.array_create(parser, &$$);
				}
			}
	|	element
;

element:
		value
			{
				parser->methods.array_create(parser, &$$);
				parser->methods.array_append(parser, &$$, &$1);
			}
	|	element ',' value
			{
				parser->methods.array_append(parser, &$1, &$3);
				ZVAL_COPY_VALUE(&$$, &$1);
			}
;

key:
		CRX_JSON_T_STRING
	|	CRX_JSON_T_ESTRING
;

value:
		object
	|	array
	|	CRX_JSON_T_STRING
	|	CRX_JSON_T_ESTRING
	|	CRX_JSON_T_INT
	|	CRX_JSON_T_DOUBLE
	|	CRX_JSON_T_NUL
	|	CRX_JSON_T_TRUE
	|	CRX_JSON_T_FALSE
;

%% /* Functions */

static int crx_json_parser_array_create(crx_json_parser *parser, zval *array)
{
	array_init(array);
	return SUCCESS;
}

static int crx_json_parser_array_append(crx_json_parser *parser, zval *array, zval *zvalue)
{
	crex_hash_next_index_insert(C_ARRVAL_P(array), zvalue);
	return SUCCESS;
}

static int crx_json_parser_object_create(crx_json_parser *parser, zval *object)
{
	if (parser->scanner.options & CRX_JSON_OBJECT_AS_ARRAY) {
		array_init(object);
	} else {
		object_init(object);
	}
	return SUCCESS;
}

static int crx_json_parser_object_update(crx_json_parser *parser, zval *object, crex_string *key, zval *zvalue)
{
	/* if JSON_OBJECT_AS_ARRAY is set */
	if (C_TYPE_P(object) == IS_ARRAY) {
		crex_symtable_update(C_ARRVAL_P(object), key, zvalue);
	} else {
		if (ZSTR_LEN(key) > 0 && ZSTR_VAL(key)[0] == '\0') {
			parser->scanner.errcode = CRX_JSON_ERROR_INVALID_PROPERTY_NAME;
			crex_string_release_ex(key, 0);
			zval_ptr_dtor_nogc(zvalue);
			zval_ptr_dtor_nogc(object);
			return FAILURE;
		}
		crex_std_write_property(C_OBJ_P(object), key, zvalue, NULL);
		C_TRY_DELREF_P(zvalue);
	}
	crex_string_release_ex(key, 0);

	return SUCCESS;
}

static int crx_json_parser_array_create_validate(crx_json_parser *parser, zval *array)
{
	ZVAL_NULL(array);
	return SUCCESS;
}

static int crx_json_parser_array_append_validate(crx_json_parser *parser, zval *array, zval *zvalue)
{
	return SUCCESS;
}

static int crx_json_parser_object_create_validate(crx_json_parser *parser, zval *object)
{
	ZVAL_NULL(object);
	return SUCCESS;
}

static int crx_json_parser_object_update_validate(crx_json_parser *parser, zval *object, crex_string *key, zval *zvalue)
{
	return SUCCESS;
}

static int crx_json_yylex(union YYSTYPE *value, crx_json_parser *parser)
{
	int token = crx_json_scan(&parser->scanner);

	bool validate = parser->methods.array_create == crx_json_parser_array_create_validate
		&& parser->methods.array_append == crx_json_parser_array_append_validate
		&& parser->methods.object_create == crx_json_parser_object_create_validate
		&& parser->methods.object_update == crx_json_parser_object_update_validate;

	if (validate) {
		zval_ptr_dtor_str(&(parser->scanner.value));
		ZVAL_UNDEF(&value->value);
	} else {
		value->value = parser->scanner.value;
	}

	return token;
}

static void crx_json_yyerror(crx_json_parser *parser, char const *msg)
{
	if (!parser->scanner.errcode) {
		parser->scanner.errcode = CRX_JSON_ERROR_SYNTAX;
	}
}

CRX_JSON_API crx_json_error_code crx_json_parser_error_code(const crx_json_parser *parser)
{
	return parser->scanner.errcode;
}

static const crx_json_parser_methods default_parser_methods =
{
	crx_json_parser_array_create,
	crx_json_parser_array_append,
	NULL,
	NULL,
	crx_json_parser_object_create,
	crx_json_parser_object_update,
	NULL,
	NULL,
};

static const crx_json_parser_methods validate_parser_methods =
{
	crx_json_parser_array_create_validate,
	crx_json_parser_array_append_validate,
	NULL,
	NULL,
	crx_json_parser_object_create_validate,
	crx_json_parser_object_update_validate,
	NULL,
	NULL,
};

CRX_JSON_API void crx_json_parser_init_ex(crx_json_parser *parser,
		zval *return_value,
		const char *str,
		size_t str_len,
		int options,
		int max_depth,
		const crx_json_parser_methods *parser_methods)
{
	memset(parser, 0, sizeof(crx_json_parser));
	crx_json_scanner_init(&parser->scanner, str, str_len, options);
	parser->depth = 1;
	parser->max_depth = max_depth;
	parser->return_value = return_value;
	memcpy(&parser->methods, parser_methods, sizeof(crx_json_parser_methods));
}

CRX_JSON_API void crx_json_parser_init(crx_json_parser *parser,
		zval *return_value,
		const char *str,
		size_t str_len,
		int options,
		int max_depth)
{
	crx_json_parser_init_ex(
			parser,
			return_value,
			str,
			str_len,
			options,
			max_depth,
			&default_parser_methods);
}

CRX_JSON_API int crx_json_parse(crx_json_parser *parser)
{
	return crx_json_yyparse(parser);
}

const crx_json_parser_methods* crx_json_get_validate_methods(void)
{
	return &validate_parser_methods;
}
