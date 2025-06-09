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
   | Author: Hartmut Holzgraefe <hholzgra@crx.net>                        |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "crx_ini.h"
#include "crx_ctype.h"
#include "ctype_arginfo.h"
#include "SAPI.h"
#include "ext/standard/info.h"

#include <ctype.h>

#ifdef HAVE_CTYPE

static CRX_MINFO_FUNCTION(ctype);

/* }}} */

/* {{{ ctype_module_entry */
crex_module_entry ctype_module_entry = {
	STANDARD_MODULE_HEADER,
	"ctype",
	ext_functions,
	NULL,
	NULL,
	NULL,
	NULL,
	CRX_MINFO(ctype),
	CRX_CTYPE_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_CTYPE
CREX_GET_MODULE(ctype)
#endif

/* {{{ CRX_MINFO_FUNCTION */
static CRX_MINFO_FUNCTION(ctype)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "ctype functions", "enabled");
	crx_info_print_table_end();
}
/* }}} */

/* Slow fallback for deprecated cases defined in a no-inline function to not bloat code. */
static crex_never_inline void ctype_fallback(zval *c, zval *return_value, int (*iswhat)(int), bool allow_digits, bool allow_minus)
{
	crx_error_docref(NULL, E_DEPRECATED,
		"Argument of type %s will be interpreted as string in the future", crex_zval_type_name(c));
	if (C_TYPE_P(c) == IS_LONG) {
		if (C_LVAL_P(c) <= 255 && C_LVAL_P(c) >= 0) {
			RETURN_BOOL(iswhat((int)C_LVAL_P(c)));
		} else if (C_LVAL_P(c) >= -128 && C_LVAL_P(c) < 0) {
			RETURN_BOOL(iswhat((int)C_LVAL_P(c) + 256));
		} else if (C_LVAL_P(c) >= 0) {
			RETURN_BOOL(allow_digits);
		} else {
			RETURN_BOOL(allow_minus);
		}
	} else {
		RETURN_FALSE;
	}
}

/* Define as a macro such that iswhat can use the macro version instead of the function version.
 * This heavily reduces the overhead. (GH-11997) */
#define ctype_impl(iswhat, allow_digits, allow_minus) do { 					\
		zval *c;															\
																			\
		CREX_PARSE_PARAMETERS_START(1, 1);									\
			C_PARAM_ZVAL(c)													\
		CREX_PARSE_PARAMETERS_END();										\
																			\
		if (C_TYPE_P(c) == IS_STRING) {										\
			char *p = C_STRVAL_P(c), *e = C_STRVAL_P(c) + C_STRLEN_P(c);	\
			if (e == p) {													\
				RETURN_FALSE;												\
			}																\
			while (p < e) {													\
				if (!iswhat((int)*(unsigned char *)(p++))) {				\
					RETURN_FALSE;											\
				}															\
			}																\
			RETURN_TRUE;													\
		}																	\
																			\
		ctype_fallback(c, return_value, iswhat, allow_digits, allow_minus);	\
	} while (0);

/* {{{ Checks for alphanumeric character(s) */
CRX_FUNCTION(ctype_alnum)
{
	ctype_impl(isalnum, 1, 0);
}
/* }}} */

/* {{{ Checks for alphabetic character(s) */
CRX_FUNCTION(ctype_alpha)
{
	ctype_impl(isalpha, 0, 0);
}
/* }}} */

/* {{{ Checks for control character(s) */
CRX_FUNCTION(ctype_cntrl)
{
	ctype_impl(iscntrl, 0, 0);
}
/* }}} */

/* {{{ Checks for numeric character(s) */
CRX_FUNCTION(ctype_digit)
{
	ctype_impl(isdigit, 1, 0);
}
/* }}} */

/* {{{ Checks for lowercase character(s)  */
CRX_FUNCTION(ctype_lower)
{
	ctype_impl(islower, 0, 0);
}
/* }}} */

/* {{{ Checks for any printable character(s) except space */
CRX_FUNCTION(ctype_graph)
{
	ctype_impl(isgraph, 1, 1);
}
/* }}} */

/* {{{ Checks for printable character(s) */
CRX_FUNCTION(ctype_print)
{
	ctype_impl(isprint, 1, 1);
}
/* }}} */

/* {{{ Checks for any printable character which is not whitespace or an alphanumeric character */
CRX_FUNCTION(ctype_punct)
{
	ctype_impl(ispunct, 0, 0);
}
/* }}} */

/* {{{ Checks for whitespace character(s)*/
CRX_FUNCTION(ctype_space)
{
	ctype_impl(isspace, 0, 0);
}
/* }}} */

/* {{{ Checks for uppercase character(s) */
CRX_FUNCTION(ctype_upper)
{
	ctype_impl(isupper, 0, 0);
}
/* }}} */

/* {{{ Checks for character(s) representing a hexadecimal digit */
CRX_FUNCTION(ctype_xdigit)
{
	ctype_impl(isxdigit, 1, 0);
}
/* }}} */

#endif	/* HAVE_CTYPE */
