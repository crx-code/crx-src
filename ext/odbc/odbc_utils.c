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
   | Author: Calvin Buckley <calvin@cmpct.info>                           |
   +----------------------------------------------------------------------+
*/

#include "crx.h"
#include "crx_odbc_utils.h"

/*
 * Utility functions for dealing with ODBC connection strings and other common
 * functionality.
 *
 * While useful for PDO_ODBC too, this lives in ext/odbc because there isn't a
 * better place for it.
 */

CRX_FUNCTION(odbc_connection_string_is_quoted)
{
	crex_string *str;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(str)
	CREX_PARSE_PARAMETERS_END();

	bool is_quoted = crx_odbc_connstr_is_quoted(ZSTR_VAL(str));

	RETURN_BOOL(is_quoted);
}

CRX_FUNCTION(odbc_connection_string_should_quote)
{
	crex_string *str;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(str)
	CREX_PARSE_PARAMETERS_END();

	bool should_quote = crx_odbc_connstr_should_quote(ZSTR_VAL(str));

	RETURN_BOOL(should_quote);
}

CRX_FUNCTION(odbc_connection_string_quote)
{
	crex_string *str;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(str)
	CREX_PARSE_PARAMETERS_END();

	size_t new_size = crx_odbc_connstr_estimate_quote_length(ZSTR_VAL(str));
	crex_string *new_string = crex_string_alloc(new_size, 0);
	crx_odbc_connstr_quote(ZSTR_VAL(new_string), ZSTR_VAL(str), new_size);
	/* reset length */
	ZSTR_LEN(new_string) = strlen(ZSTR_VAL(new_string));
	RETURN_STR(new_string);
}
