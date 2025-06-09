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
   | Author: Wez Furlong <wez@crx.net>                                    |
   +----------------------------------------------------------------------+
*/

#include "crx.h"
#include "crx_win32_globals.h"
#include "syslog.h"

#ifdef ZTS
CRXAPI int crx_win32_core_globals_id;
#else
crx_win32_core_globals the_crx_win32_core_globals;
#endif

void crx_win32_core_globals_ctor(void *vg)
{/*{{{*/
	crx_win32_core_globals *wg = (crx_win32_core_globals*)vg;
	memset(wg, 0, sizeof(*wg));

	wg->mail_socket = INVALID_SOCKET;

	wg->log_source = INVALID_HANDLE_VALUE;
}/*}}}*/

void crx_win32_core_globals_dtor(void *vg)
{/*{{{*/
	crx_win32_core_globals *wg = (crx_win32_core_globals*)vg;

	if (wg->registry_key) {
		RegCloseKey(wg->registry_key);
		wg->registry_key = NULL;
	}
	if (wg->registry_event) {
		CloseHandle(wg->registry_event);
		wg->registry_event = NULL;
	}
	if (wg->registry_directories) {
		crex_hash_destroy(wg->registry_directories);
		free(wg->registry_directories);
		wg->registry_directories = NULL;
	}

	if (INVALID_SOCKET != wg->mail_socket) {
		closesocket(wg->mail_socket);
		wg->mail_socket = INVALID_SOCKET;
	}
}/*}}}*/


CRX_RSHUTDOWN_FUNCTION(win32_core_globals)
{/*{{{*/
	closelog();

	return SUCCESS;
}/*}}}*/
