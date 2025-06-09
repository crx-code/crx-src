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
   | Author: Andrew Sitnikov <sitnikov@infonet.ee>                        |
   +----------------------------------------------------------------------+
*/

#include "crx.h"

#include <sys/types.h>

#ifdef HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif

#ifdef CRX_WIN32
#include "win32/ipc.h"
#endif

#ifdef HAVE_FTOK
/* {{{ Convert a pathname and a project identifier to a System V IPC key */
CRX_FUNCTION(ftok)
{
	char *pathname, *proj;
	size_t pathname_len, proj_len;
	key_t k;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_PATH(pathname, pathname_len)
		C_PARAM_STRING(proj, proj_len)
	CREX_PARSE_PARAMETERS_END();

	if (pathname_len == 0){
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	}

	if (proj_len != 1){
		crex_argument_value_error(2, "must be a single character");
		RETURN_THROWS();
	}

	if (crx_check_open_basedir(pathname)) {
		RETURN_LONG(-1);
	}

	k = ftok(pathname, proj[0]);
	if (k == -1) {
		crx_error_docref(NULL, E_WARNING, "ftok() failed - %s", strerror(errno));
	}

	RETURN_LONG(k);
}
/* }}} */
#endif
