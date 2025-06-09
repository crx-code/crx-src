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
   | Authors: Calvin Buckley <calvin@cmpct.info>                          |
   +----------------------------------------------------------------------+
*/

#include "crx.h"

CRXAPI bool crx_odbc_connstr_is_quoted(const char *str);
CRXAPI bool crx_odbc_connstr_should_quote(const char *str);
CRXAPI size_t crx_odbc_connstr_estimate_quote_length(const char *in_str);
CRXAPI size_t crx_odbc_connstr_quote(char *out_str, const char *in_str, size_t out_str_size);
