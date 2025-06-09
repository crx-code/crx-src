%require "3.0"
%{
/*
   +----------------------------------------------------------------------+
   | Crex Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Crex Technologies Ltd. (http://www.crex.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Zeev Suraski <zeev@crx.net>                                 |
   |          Jani Taskinen <jani@crx.net>                                |
   +----------------------------------------------------------------------+
*/

#define DEBUG_CFG_PARSER 0

#include "crex.h"
#include "crex_API.h"
#include "crex_ini.h"
#include "crex_constants.h"
#include "crex_ini_scanner.h"
#include "crex_extensions.h"

#ifdef CREX_WIN32
#include "win32/syslog.h"
#endif

int ini_parse(void);

#define CREX_INI_PARSER_CB	(CG(ini_parser_param))->ini_parser_cb
#define CREX_INI_PARSER_ARG	(CG(ini_parser_param))->arg

#ifdef _MSC_VER
#define YYMALLOC malloc
#define YYFREE free
#endif

#define CREX_SYSTEM_INI CG(ini_parser_unbuffered_errors)
#define INI_ZVAL_IS_NUMBER 1

static int get_int_val(zval *op) {
	switch (C_TYPE_P(op)) {
		case IS_LONG:
			return C_LVAL_P(op);
		case IS_DOUBLE:
			return (int)C_DVAL_P(op);
		case IS_STRING:
		{
			int val = atoi(C_STRVAL_P(op));
			crex_string_free(C_STR_P(op));
			return val;
		}
		EMPTY_SWITCH_DEFAULT_CASE()
	}
}

/* {{{ crex_ini_do_op() */
static void crex_ini_do_op(char type, zval *result, zval *op1, zval *op2)
{
	int i_result;
	int i_op1, i_op2;
	int str_len;
	char str_result[MAX_LENGTH_OF_LONG+1];

	i_op1 = get_int_val(op1);
	i_op2 = op2 ? get_int_val(op2) : 0;

	switch (type) {
		case '|':
			i_result = i_op1 | i_op2;
			break;
		case '&':
			i_result = i_op1 & i_op2;
			break;
		case '^':
			i_result = i_op1 ^ i_op2;
			break;
		case '~':
			i_result = ~i_op1;
			break;
		case '!':
			i_result = !i_op1;
			break;
		default:
			i_result = 0;
			break;
	}

	if (INI_SCNG(scanner_mode) != CREX_INI_SCANNER_TYPED) {
		str_len = sprintf(str_result, "%d", i_result);
		ZVAL_NEW_STR(result, crex_string_init(str_result, str_len, CREX_SYSTEM_INI));
	} else {
		ZVAL_LONG(result, i_result);
	}
}
/* }}} */

/* {{{ crex_ini_init_string() */
static void crex_ini_init_string(zval *result)
{
	if (CREX_SYSTEM_INI) {
		ZVAL_EMPTY_PSTRING(result);
	} else {
		ZVAL_EMPTY_STRING(result);
	}
	C_EXTRA_P(result) = 0;
}
/* }}} */

/* {{{ crex_ini_add_string() */
static void crex_ini_add_string(zval *result, zval *op1, zval *op2)
{
	int length, op1_len;

	if (C_TYPE_P(op1) != IS_STRING) {
		/* CREX_ASSERT(!C_REFCOUNTED_P(op1)); */
		if (CREX_SYSTEM_INI) {
			crex_string *tmp_str;
			crex_string *str = zval_get_tmp_string(op1, &tmp_str);
			ZVAL_PSTRINGL(op1, ZSTR_VAL(str), ZSTR_LEN(str));
			crex_tmp_string_release(tmp_str);
		} else {
			ZVAL_STR(op1, zval_get_string_func(op1));
		}
	}
	op1_len = (int)C_STRLEN_P(op1);

	if (C_TYPE_P(op2) != IS_STRING) {
		convert_to_string(op2);
	}
	length = op1_len + (int)C_STRLEN_P(op2);

	ZVAL_NEW_STR(result, crex_string_extend(C_STR_P(op1), length, CREX_SYSTEM_INI));
	memcpy(C_STRVAL_P(result) + op1_len, C_STRVAL_P(op2), C_STRLEN_P(op2) + 1);
}
/* }}} */

/* {{{ crex_ini_get_constant() */
static void crex_ini_get_constant(zval *result, zval *name)
{
	zval *c, tmp;

	/* If name contains ':' it is not a constant. Bug #26893. */
	if (!memchr(C_STRVAL_P(name), ':', C_STRLEN_P(name))
		   	&& (c = crex_get_constant(C_STR_P(name))) != 0) {
		if (C_TYPE_P(c) != IS_STRING) {
			ZVAL_COPY_OR_DUP(&tmp, c);
			if (C_OPT_CONSTANT(tmp)) {
				zval_update_constant_ex(&tmp, NULL);
			}
			convert_to_string(&tmp);
			c = &tmp;
		}
		ZVAL_NEW_STR(result, crex_string_init(C_STRVAL_P(c), C_STRLEN_P(c), CREX_SYSTEM_INI));
		if (c == &tmp) {
			crex_string_release(C_STR(tmp));
		}
		crex_string_free(C_STR_P(name));
	} else {
		*result = *name;
	}
}
/* }}} */

/* {{{ crex_ini_get_var() */
static void crex_ini_get_var(zval *result, zval *name, zval *fallback)
{
	zval *curval;
	char *envvar;

	/* Fetch configuration option value */
	if ((curval = crex_get_configuration_directive(C_STR_P(name))) != NULL) {
		ZVAL_NEW_STR(result, crex_string_init(C_STRVAL_P(curval), C_STRLEN_P(curval), CREX_SYSTEM_INI));
	/* ..or if not found, try ENV */
	} else if ((envvar = crex_getenv(C_STRVAL_P(name), C_STRLEN_P(name))) != NULL ||
			   (envvar = getenv(C_STRVAL_P(name))) != NULL) {
		ZVAL_NEW_STR(result, crex_string_init(envvar, strlen(envvar), CREX_SYSTEM_INI));
	/* ..or if not defined, try fallback value */
	} else if (fallback) {
		ZVAL_NEW_STR(result, crex_string_init(C_STRVAL_P(fallback), strlen(C_STRVAL_P(fallback)), CREX_SYSTEM_INI));
	} else {
		crex_ini_init_string(result);
	}

}
/* }}} */

/* {{{ ini_error() */
static CREX_COLD void ini_error(const char *msg)
{
	char *error_buf;
	int error_buf_len;

	const char *const currently_parsed_filename = crex_ini_scanner_get_filename();
	if (currently_parsed_filename) {
		error_buf_len = 128 + (int)strlen(msg) + (int)strlen(currently_parsed_filename); /* should be more than enough */
		error_buf = (char *) emalloc(error_buf_len);

		sprintf(error_buf, "%s in %s on line %d\n", msg, currently_parsed_filename, crex_ini_scanner_get_lineno());
	} else {
		error_buf = estrdup("Invalid configuration directive\n");
	}

	if (CG(ini_parser_unbuffered_errors)) {
#ifdef CREX_WIN32
		syslog(LOG_ALERT, "CRX: %s (%s)", error_buf, GetCommandLine());
#endif
		fprintf(stderr, "CRX:  %s", error_buf);
	} else {
		crex_error(E_WARNING, "%s", error_buf);
	}
	efree(error_buf);
}
/* }}} */

/* {{{ crex_parse_ini_file() */
CREX_API crex_result crex_parse_ini_file(crex_file_handle *fh, bool unbuffered_errors, int scanner_mode, crex_ini_parser_cb_t ini_parser_cb, void *arg)
{
	int retval;
	crex_ini_parser_param ini_parser_param;

	ini_parser_param.ini_parser_cb = ini_parser_cb;
	ini_parser_param.arg = arg;
	CG(ini_parser_param) = &ini_parser_param;

	if (crex_ini_open_file_for_scanning(fh, scanner_mode) == FAILURE) {
		return FAILURE;
	}

	CG(ini_parser_unbuffered_errors) = unbuffered_errors;
	retval = ini_parse();

	shutdown_ini_scanner();

	if (retval == 0) {
		return SUCCESS;
	} else {
		return FAILURE;
	}
}
/* }}} */

/* {{{ crex_parse_ini_string() */
CREX_API crex_result crex_parse_ini_string(const char *str, bool unbuffered_errors, int scanner_mode, crex_ini_parser_cb_t ini_parser_cb, void *arg)
{
	int retval;
	crex_ini_parser_param ini_parser_param;

	ini_parser_param.ini_parser_cb = ini_parser_cb;
	ini_parser_param.arg = arg;
	CG(ini_parser_param) = &ini_parser_param;

	if (crex_ini_prepare_string_for_scanning(str, scanner_mode) == FAILURE) {
		return FAILURE;
	}

	CG(ini_parser_unbuffered_errors) = unbuffered_errors;
	retval = ini_parse();

	shutdown_ini_scanner();

	if (retval == 0) {
		return SUCCESS;
	} else {
		return FAILURE;
	}
}
/* }}} */

/* {{{ zval_ini_dtor() */
static void zval_ini_dtor(zval *zv)
{
	if (C_TYPE_P(zv) == IS_STRING) {
		if (CREX_SYSTEM_INI) {
			GC_MAKE_PERSISTENT_LOCAL(C_STR_P(zv));
		}
		crex_string_release(C_STR_P(zv));
	}
}
/* }}} */

static inline crex_result convert_to_number(zval *retval, const char *str, const int str_len)
{
	uint8_t type;
	int overflow;
	crex_long lval;
	double dval;

	if ((type = is_numeric_string_ex(str, str_len, &lval, &dval, 0, &overflow, NULL)) != 0) {
		if (type == IS_LONG) {
			ZVAL_LONG(retval, lval);
			return SUCCESS;
		} else if (type == IS_DOUBLE && !overflow) {
			ZVAL_DOUBLE(retval, dval);
			return SUCCESS;
		}
	}

	return FAILURE;
}

static void normalize_value(zval *zv)
{
	if (INI_SCNG(scanner_mode) != CREX_INI_SCANNER_TYPED) {
		return;
	}

	CREX_ASSERT(C_EXTRA_P(zv) == 0 || C_EXTRA_P(zv) == INI_ZVAL_IS_NUMBER);
	if (C_EXTRA_P(zv) == INI_ZVAL_IS_NUMBER && C_TYPE_P(zv) == IS_STRING) {
		zval number_rv;
		if (convert_to_number(&number_rv, C_STRVAL_P(zv), C_STRLEN_P(zv)) == SUCCESS) {
			zval_ptr_dtor(zv);
			ZVAL_COPY_VALUE(zv, &number_rv);
		}
	}
}

%}

%expect 0
%define api.prefix {ini_}
%define api.pure full
%define api.value.type {zval}
%define parse.error verbose

%token END 0 "end of file"
%token TC_SECTION
%token TC_RAW
%token TC_CONSTANT
%token TC_NUMBER
%token TC_STRING
%token TC_WHITESPACE
%token TC_LABEL
%token TC_OFFSET
%token TC_DOLLAR_CURLY
%token TC_VARNAME
%token TC_QUOTED_STRING
%token TC_FALLBACK
%token BOOL_TRUE
%token BOOL_FALSE
%token NULL_NULL
%token END_OF_LINE
%token '=' ':' ',' '.' '"' '\'' '^' '+' '-' '/' '*' '%' '$' '~' '<' '>' '?' '@' '{' '}'
%left '|' '&' '^'
%precedence '~' '!'

%destructor { zval_ini_dtor(&$$); } TC_RAW TC_CONSTANT TC_NUMBER TC_STRING TC_WHITESPACE TC_LABEL TC_OFFSET TC_VARNAME BOOL_TRUE BOOL_FALSE NULL_NULL cfg_var_ref constant_literal constant_string encapsed_list expr option_offset section_string_or_value string_or_value var_string_list var_string_list_section

%%

statement_list:
		statement_list statement
	|	%empty { (void) ini_nerrs; }
;

statement:
		TC_SECTION section_string_or_value ']' {
#if DEBUG_CFG_PARSER
			printf("SECTION: [%s]\n", C_STRVAL($2));
#endif
			CREX_INI_PARSER_CB(&$2, NULL, NULL, CREX_INI_PARSER_SECTION, CREX_INI_PARSER_ARG);
			crex_string_release(C_STR($2));
		}
	|	TC_LABEL '=' string_or_value {
#if DEBUG_CFG_PARSER
			printf("NORMAL: '%s' = '%s'\n", C_STRVAL($1), C_STRVAL($3));
#endif
			CREX_INI_PARSER_CB(&$1, &$3, NULL, CREX_INI_PARSER_ENTRY, CREX_INI_PARSER_ARG);
			if (CREX_SYSTEM_INI) {
				GC_MAKE_PERSISTENT_LOCAL(C_STR($1));
			}
			crex_string_release(C_STR($1));
			zval_ini_dtor(&$3);
		}
	|	TC_OFFSET option_offset ']' '=' string_or_value {
#if DEBUG_CFG_PARSER
			printf("OFFSET: '%s'[%s] = '%s'\n", C_STRVAL($1), C_STRVAL($2), C_STRVAL($5));
#endif
			CREX_INI_PARSER_CB(&$1, &$5, &$2, CREX_INI_PARSER_POP_ENTRY, CREX_INI_PARSER_ARG);
			crex_string_release(C_STR($1));
			zval_ini_dtor(&$2);
			zval_ini_dtor(&$5);
		}
	|	TC_LABEL	{ CREX_INI_PARSER_CB(&$1, NULL, NULL, CREX_INI_PARSER_ENTRY, CREX_INI_PARSER_ARG); crex_string_release(C_STR($1)); }
	|	END_OF_LINE
;

section_string_or_value:
		var_string_list_section			{ $$ = $1; }
	|	%empty						{ crex_ini_init_string(&$$); }
;

string_or_value:
		expr							{ $$ = $1; normalize_value(&$$); }
	|	BOOL_TRUE						{ $$ = $1; }
	|	BOOL_FALSE						{ $$ = $1; }
	|	NULL_NULL						{ $$ = $1; }
	|	END_OF_LINE						{ crex_ini_init_string(&$$); }
;

option_offset:
		var_string_list					{ $$ = $1; }
	|	%empty						{ crex_ini_init_string(&$$); }
;

encapsed_list:
		encapsed_list cfg_var_ref		{ crex_ini_add_string(&$$, &$1, &$2); crex_string_free(C_STR($2)); }
	|	encapsed_list TC_QUOTED_STRING	{ crex_ini_add_string(&$$, &$1, &$2); crex_string_free(C_STR($2)); }
	|	%empty						{ crex_ini_init_string(&$$); }
;

var_string_list_section:
		cfg_var_ref						{ $$ = $1; }
	|	constant_literal				{ $$ = $1; }
	|	'"' encapsed_list '"'			{ $$ = $2; }
	|	var_string_list_section cfg_var_ref 	{ crex_ini_add_string(&$$, &$1, &$2); crex_string_free(C_STR($2)); }
	|	var_string_list_section constant_literal	{ crex_ini_add_string(&$$, &$1, &$2); crex_string_free(C_STR($2)); }
	|	var_string_list_section '"' encapsed_list '"'  { crex_ini_add_string(&$$, &$1, &$3); crex_string_free(C_STR($3)); }
;

var_string_list:
		cfg_var_ref						{ $$ = $1; }
	|	constant_string					{ $$ = $1; }
	|	'"' encapsed_list '"'			{ $$ = $2; }
	|	var_string_list cfg_var_ref 	{ crex_ini_add_string(&$$, &$1, &$2); crex_string_free(C_STR($2)); }
	|	var_string_list constant_string	{ crex_ini_add_string(&$$, &$1, &$2); crex_string_free(C_STR($2)); }
	|	var_string_list '"' encapsed_list '"'  { crex_ini_add_string(&$$, &$1, &$3); crex_string_free(C_STR($3)); }
;

expr:
		var_string_list					{ $$ = $1; }
	|	expr '|' expr					{ crex_ini_do_op('|', &$$, &$1, &$3); }
	|	expr '&' expr					{ crex_ini_do_op('&', &$$, &$1, &$3); }
	|	expr '^' expr					{ crex_ini_do_op('^', &$$, &$1, &$3); }
	|	'~' expr						{ crex_ini_do_op('~', &$$, &$2, NULL); }
	|	'!'	expr						{ crex_ini_do_op('!', &$$, &$2, NULL); }
	|	'(' expr ')'					{ $$ = $2; }
;

cfg_var_ref:
		TC_DOLLAR_CURLY TC_VARNAME '}'				{ crex_ini_get_var(&$$, &$2, NULL); crex_string_free(C_STR($2)); }
	|	TC_DOLLAR_CURLY TC_VARNAME TC_FALLBACK fallback '}'	{ crex_ini_get_var(&$$, &$2, &$4); crex_string_free(C_STR($2)); crex_string_free(C_STR($4)); }
;


fallback:
		var_string_list	{ $$ = $1; }
	|	%empty			{ crex_ini_init_string(&$$); }
;

constant_literal:
		TC_CONSTANT						{ $$ = $1; }
	|	TC_RAW							{ $$ = $1; /*printf("TC_RAW: '%s'\n", C_STRVAL($1));*/ }
	|	TC_NUMBER						{ $$ = $1; /*printf("TC_NUMBER: '%s'\n", C_STRVAL($1));*/ }
	|	TC_STRING						{ $$ = $1; /*printf("TC_STRING: '%s'\n", C_STRVAL($1));*/ }
	|	TC_WHITESPACE					{ $$ = $1; /*printf("TC_WHITESPACE: '%s'\n", C_STRVAL($1));*/ }
;

constant_string:
		TC_CONSTANT						{ crex_ini_get_constant(&$$, &$1); }
	|	TC_RAW							{ $$ = $1; /*printf("TC_RAW: '%s'\n", C_STRVAL($1));*/ }
	|	TC_NUMBER {
			$$ = $1;
			C_EXTRA($$) = INI_ZVAL_IS_NUMBER;
			/*printf("TC_NUMBER: '%s'\n", C_STRVAL($1));*/
		}
	|	TC_STRING						{ $$ = $1; /*printf("TC_STRING: '%s'\n", C_STRVAL($1));*/ }
	|	TC_WHITESPACE					{ $$ = $1; /*printf("TC_WHITESPACE: '%s'\n", C_STRVAL($1));*/ }
;
