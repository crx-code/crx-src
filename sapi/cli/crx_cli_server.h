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
   | Author: Moriyoshi Koizumi <moriyoshi@crx.net>                        |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_CLI_SERVER_H
#define CRX_CLI_SERVER_H

#include "SAPI.h"

extern const crex_function_entry server_additional_functions[];
extern sapi_module_struct cli_server_sapi_module;
extern int do_cli_server(int argc, char **argv);

CREX_BEGIN_MODULE_GLOBALS(cli_server)
	short color;
CREX_END_MODULE_GLOBALS(cli_server)

#ifdef ZTS
#define CLI_SERVER_G(v) CREX_TSRMG(cli_server_globals_id, crex_cli_server_globals *, v)
CREX_TSRMLS_CACHE_EXTERN()
#else
#define CLI_SERVER_G(v) (cli_server_globals.v)
#endif

#endif /* CRX_CLI_SERVER_H */
