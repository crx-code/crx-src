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
   | Author: Rasmus Lerdorf <rasmus@lerdorf.on.ca>                        |
   +----------------------------------------------------------------------+
*/

#ifndef EXEC_H
#define EXEC_H

CRX_MINIT_FUNCTION(proc_open);
CRX_MINIT_FUNCTION(exec);

CRXAPI crex_string *crx_escape_shell_cmd(const char *str);
CRXAPI crex_string *crx_escape_shell_arg(const char *str);
CRXAPI int crx_exec(int type, const char *cmd, zval *array, zval *return_value);

#endif /* EXEC_H */
