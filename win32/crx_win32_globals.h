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

#ifndef CRX_WIN32_GLOBALS_H
#define CRX_WIN32_GLOBALS_H

/* misc globals for thread-safety under win32 */

#include "win32/sendmail.h"

typedef struct _crx_win32_core_globals crx_win32_core_globals;

#ifdef ZTS
# define PW32G(v)		CREX_TSRMG(crx_win32_core_globals_id, crx_win32_core_globals*, v)
extern CRXAPI int crx_win32_core_globals_id;
#else
# define PW32G(v)		(the_crx_win32_core_globals.v)
extern CRXAPI struct _crx_win32_core_globals the_crx_win32_core_globals;
#endif

struct _crx_win32_core_globals {
	/* syslog */
	char *log_header;
	HANDLE log_source;

	HKEY       registry_key;
	HANDLE     registry_event;
	HashTable *registry_directories;

	char   mail_buffer[MAIL_BUFFER_SIZE];
	SOCKET mail_socket;
	char   mail_host[HOST_NAME_LEN];
	char   mail_local_host[HOST_NAME_LEN];
};

void crx_win32_core_globals_ctor(void *vg);
void crx_win32_core_globals_dtor(void *vg);
CRX_RSHUTDOWN_FUNCTION(win32_core_globals);

#endif
