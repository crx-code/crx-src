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
   | Author: Jim Winstead <jimw@crx.net>                                  |
   +----------------------------------------------------------------------+
*/

#include "crx.h"
#include "pageinfo.h"
#include "SAPI.h"

#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_PWD_H
#ifdef CRX_WIN32
#include "win32/pwd.h"
#else
#include <pwd.h>
#endif
#endif
#if HAVE_GRP_H
# include <grp.h>
#endif
#ifdef CRX_WIN32
#undef getgid
#define getgroups(a, b) 0
#define getgid() 1
#define getuid() 1
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#ifdef CRX_WIN32
#include <process.h>
#endif

#include "ext/standard/basic_functions.h"

/* {{{ crx_statpage */
CRXAPI void crx_statpage(void)
{
	crex_stat_t *pstat = NULL;

	pstat = sapi_get_stat();

	if (BG(page_uid)==-1 || BG(page_gid)==-1) {
		if(pstat) {
			BG(page_uid)   = pstat->st_uid;
			BG(page_gid)   = pstat->st_gid;
			BG(page_inode) = pstat->st_ino;
			BG(page_mtime) = pstat->st_mtime;
		} else { /* handler for situations where there is no source file, ex. crx -r */
			BG(page_uid) = getuid();
			BG(page_gid) = getgid();
		}
	}
}
/* }}} */

/* {{{ crx_getuid */
crex_long crx_getuid(void)
{
	crx_statpage();
	return (BG(page_uid));
}
/* }}} */

crex_long crx_getgid(void)
{
	crx_statpage();
	return (BG(page_gid));
}

/* {{{ Get CRX script owner's UID */
CRX_FUNCTION(getmyuid)
{
	crex_long uid;

	CREX_PARSE_PARAMETERS_NONE();

	uid = crx_getuid();
	if (uid < 0) {
		RETURN_FALSE;
	} else {
		RETURN_LONG(uid);
	}
}
/* }}} */

/* {{{ Get CRX script owner's GID */
CRX_FUNCTION(getmygid)
{
	crex_long gid;

	CREX_PARSE_PARAMETERS_NONE();

	gid = crx_getgid();
	if (gid < 0) {
		RETURN_FALSE;
	} else {
		RETURN_LONG(gid);
	}
}
/* }}} */

/* {{{ Get current process ID */
CRX_FUNCTION(getmypid)
{
	crex_long pid;

	CREX_PARSE_PARAMETERS_NONE();

	pid = getpid();
	if (pid < 0) {
		RETURN_FALSE;
	} else {
		RETURN_LONG(pid);
	}
}
/* }}} */

/* {{{ Get the inode of the current script being parsed */
CRX_FUNCTION(getmyinode)
{
	CREX_PARSE_PARAMETERS_NONE();

	crx_statpage();
	if (BG(page_inode) < 0) {
		RETURN_FALSE;
	} else {
		RETURN_LONG(BG(page_inode));
	}
}
/* }}} */

CRXAPI time_t crx_getlastmod(void)
{
	crx_statpage();
	return BG(page_mtime);
}

/* {{{ Get time of last page modification */
CRX_FUNCTION(getlastmod)
{
	crex_long lm;

	CREX_PARSE_PARAMETERS_NONE();

	lm = crx_getlastmod();
	if (lm < 0) {
		RETURN_FALSE;
	} else {
		RETURN_LONG(lm);
	}
}
/* }}} */
