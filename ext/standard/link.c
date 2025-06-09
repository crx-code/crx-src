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
   | Author:                                                              |
   +----------------------------------------------------------------------+
 */

#include "crx.h"
#include "crx_filestat.h"
#include "crx_globals.h"

#if defined(HAVE_SYMLINK) || defined(CRX_WIN32)

#ifdef CRX_WIN32
#include <WinBase.h>
#endif

#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifndef CRX_WIN32
#include <sys/stat.h>
#endif
#include <string.h>
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
#include <errno.h>
#include <ctype.h>

#include "crx_string.h"

#ifndef VOLUME_NAME_NT
#define VOLUME_NAME_NT 0x2
#endif

#ifndef VOLUME_NAME_DOS
#define VOLUME_NAME_DOS 0x0
#endif

/* {{{ Return the target of a symbolic link */
CRX_FUNCTION(readlink)
{
	char *link;
	size_t link_len;
	char buff[MAXPATHLEN];
	ssize_t ret;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_PATH(link, link_len)
	CREX_PARSE_PARAMETERS_END();

	if (crx_check_open_basedir(link)) {
		RETURN_FALSE;
	}

	ret = crx_sys_readlink(link, buff, MAXPATHLEN-1);

	if (ret == -1) {
#ifdef CRX_WIN32
		crx_error_docref(NULL, E_WARNING, "readlink failed to read the symbolic link (%s), error %d", link, GetLastError());
#else
		crx_error_docref(NULL, E_WARNING, "%s", strerror(errno));
#endif
		RETURN_FALSE;
	}
	/* Append NULL to the end of the string */
	buff[ret] = '\0';

	RETURN_STRINGL(buff, ret);
}
/* }}} */

/* {{{ Returns the st_dev field of the UNIX C stat structure describing the link */
CRX_FUNCTION(linkinfo)
{
	char *link;
	char *dirname;
	size_t link_len;
	crex_stat_t sb = {0};
	int ret;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_PATH(link, link_len)
	CREX_PARSE_PARAMETERS_END();

	dirname = estrndup(link, link_len);
	crx_dirname(dirname, link_len);

	if (crx_check_open_basedir(dirname)) {
		efree(dirname);
		RETURN_FALSE;
	}

	ret = VCWD_LSTAT(link, &sb);
	if (ret == -1) {
		crx_error_docref(NULL, E_WARNING, "%s", strerror(errno));
		efree(dirname);
		RETURN_LONG(C_L(-1));
	}

	efree(dirname);
	RETURN_LONG((crex_long) sb.st_dev);
}
/* }}} */

/* {{{ Create a symbolic link */
CRX_FUNCTION(symlink)
{
	char *topath, *frompath;
	size_t topath_len, frompath_len;
	int ret;
	char source_p[MAXPATHLEN];
	char dest_p[MAXPATHLEN];
	char dirname[MAXPATHLEN];
	size_t len;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_PATH(topath, topath_len)
		C_PARAM_PATH(frompath, frompath_len)
	CREX_PARSE_PARAMETERS_END();

	if (!expand_filepath(frompath, source_p)) {
		crx_error_docref(NULL, E_WARNING, "No such file or directory");
		RETURN_FALSE;
	}

	memcpy(dirname, source_p, sizeof(source_p));
	len = crx_dirname(dirname, strlen(dirname));

	if (!expand_filepath_ex(topath, dest_p, dirname, len)) {
		crx_error_docref(NULL, E_WARNING, "No such file or directory");
		RETURN_FALSE;
	}

	if (crx_stream_locate_url_wrapper(source_p, NULL, STREAM_LOCATE_WRAPPERS_ONLY) ||
		crx_stream_locate_url_wrapper(dest_p, NULL, STREAM_LOCATE_WRAPPERS_ONLY) )
	{
		crx_error_docref(NULL, E_WARNING, "Unable to symlink to a URL");
		RETURN_FALSE;
	}

	if (crx_check_open_basedir(dest_p)) {
		RETURN_FALSE;
	}

	if (crx_check_open_basedir(source_p)) {
		RETURN_FALSE;
	}

	/* For the source, an expanded path must be used (in ZTS an other thread could have changed the CWD).
	 * For the target the exact string given by the user must be used, relative or not, existing or not.
	 * The target is relative to the link itself, not to the CWD. */
	ret = crx_sys_symlink(topath, source_p);

	if (ret == -1) {
		crx_error_docref(NULL, E_WARNING, "%s", strerror(errno));
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Create a hard link */
CRX_FUNCTION(link)
{
	char *topath, *frompath;
	size_t topath_len, frompath_len;
	int ret;
	char source_p[MAXPATHLEN];
	char dest_p[MAXPATHLEN];

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_PATH(topath, topath_len)
		C_PARAM_PATH(frompath, frompath_len)
	CREX_PARSE_PARAMETERS_END();

	if (!expand_filepath(frompath, source_p) || !expand_filepath(topath, dest_p)) {
		crx_error_docref(NULL, E_WARNING, "No such file or directory");
		RETURN_FALSE;
	}

	if (crx_stream_locate_url_wrapper(source_p, NULL, STREAM_LOCATE_WRAPPERS_ONLY) ||
		crx_stream_locate_url_wrapper(dest_p, NULL, STREAM_LOCATE_WRAPPERS_ONLY) )
	{
		crx_error_docref(NULL, E_WARNING, "Unable to link to a URL");
		RETURN_FALSE;
	}

	if (crx_check_open_basedir(dest_p)) {
		RETURN_FALSE;
	}

	if (crx_check_open_basedir(source_p)) {
		RETURN_FALSE;
	}

#ifndef ZTS
	ret = crx_sys_link(topath, frompath);
#else
	ret = crx_sys_link(dest_p, source_p);
#endif
	if (ret == -1) {
		crx_error_docref(NULL, E_WARNING, "%s", strerror(errno));
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

#endif
