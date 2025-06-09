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
   | Author: Anatol Belski <ab@crx.net>                                   |
   +----------------------------------------------------------------------+
 */

#include "crx.h"
#include "ipc.h"

#include <windows.h>
#include <sys/stat.h>

#include "ioutil.h"

CRX_WIN32_IPC_API key_t
ftok(const char *pathname, int proj_id)
{/*{{{*/
	HANDLE fh;
	struct _stat st;
	BY_HANDLE_FILE_INFORMATION bhfi;
	key_t ret;
	CRX_WIN32_IOUTIL_INIT_W(pathname)

	if (!pathw) {
		return (key_t)-1;
	}

	if (_wstat(pathw, &st) < 0) {
		CRX_WIN32_IOUTIL_CLEANUP_W()
		return (key_t)-1;
	}

	if ((fh = CreateFileW(pathw, FILE_GENERIC_READ, CRX_WIN32_IOUTIL_DEFAULT_SHARE_MODE, 0, OPEN_EXISTING, 0, 0)) == INVALID_HANDLE_VALUE) {
		CRX_WIN32_IOUTIL_CLEANUP_W()
		return (key_t)-1;
	}

	if (!GetFileInformationByHandle(fh, &bhfi)) {
		CRX_WIN32_IOUTIL_CLEANUP_W()
		CloseHandle(fh);
		return (key_t)-1;
	}

	ret = (key_t) ((proj_id & 0xff) << 24 | (st.st_dev & 0xff) << 16 | (bhfi.nFileIndexLow & 0xffff));

	CloseHandle(fh);
	CRX_WIN32_IOUTIL_CLEANUP_W()

	return ret;
}/*}}}*/
