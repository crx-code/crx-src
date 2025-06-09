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
   | Author:  Jim Winstead <jimw@crx.net>                                 |
   +----------------------------------------------------------------------+
 */

#include "crx.h"
#include "fopen_wrappers.h"
#include "crx_globals.h"

#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif

#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif

#if defined(__APPLE__)
  /*
   Apple statvfs has an interger overflow in libc copying to statvfs.
   cvt_statfs_to_statvfs(struct statfs *from, struct statvfs *to) {
   to->f_blocks = (fsblkcnt_t)from->f_blocks;
   */
#  undef HAVE_SYS_STATVFS_H
#  undef HAVE_STATVFS
#endif

#if defined(HAVE_SYS_STATVFS_H) && defined(HAVE_STATVFS)
# include <sys/statvfs.h>
#elif defined(HAVE_SYS_STATFS_H) && defined(HAVE_STATFS)
# include <sys/statfs.h>
#elif defined(HAVE_SYS_MOUNT_H) && defined(HAVE_STATFS)
# include <sys/mount.h>
#endif

#ifdef HAVE_PWD_H
# ifdef CRX_WIN32
#  include "win32/pwd.h"
# else
#  include <pwd.h>
# endif
#endif

#if HAVE_GRP_H
# include <grp.h>
#endif

#ifdef HAVE_UTIME
# ifdef CRX_WIN32
#  include <sys/utime.h>
# else
#  include <utime.h>
# endif
#endif

#ifdef CRX_WIN32
#include "win32/winutil.h"
#endif

#include "basic_functions.h"
#include "crx_filestat.h"

CRX_RINIT_FUNCTION(filestat) /* {{{ */
{
	BG(CurrentStatFile)=NULL;
	BG(CurrentLStatFile)=NULL;
	return SUCCESS;
}
/* }}} */

CRX_RSHUTDOWN_FUNCTION(filestat) /* {{{ */
{
	if (BG(CurrentStatFile)) {
		crex_string_release(BG(CurrentStatFile));
		BG(CurrentStatFile) = NULL;
	}
	if (BG(CurrentLStatFile)) {
		crex_string_release(BG(CurrentLStatFile));
		BG(CurrentLStatFile) = NULL;
	}
	return SUCCESS;
}
/* }}} */

static crex_result crx_disk_total_space(char *path, double *space) /* {{{ */
#if defined(WINDOWS) /* {{{ */
{
	ULARGE_INTEGER FreeBytesAvailableToCaller;
	ULARGE_INTEGER TotalNumberOfBytes;
	ULARGE_INTEGER TotalNumberOfFreeBytes;
	CRX_WIN32_IOUTIL_INIT_W(path)

	if (GetDiskFreeSpaceExW(pathw, &FreeBytesAvailableToCaller, &TotalNumberOfBytes, &TotalNumberOfFreeBytes) == 0) {
		char *err = crx_win_err();
		crx_error_docref(NULL, E_WARNING, "%s", err);
		crx_win_err_free(err);
		CRX_WIN32_IOUTIL_CLEANUP_W()
		return FAILURE;
	}

	/* i know - this is ugly, but i works <thies@thieso.net> */
	*space = TotalNumberOfBytes.HighPart * (double) (((crex_ulong)1) << 31) * 2.0 + TotalNumberOfBytes.LowPart;

	CRX_WIN32_IOUTIL_CLEANUP_W()

	return SUCCESS;
}
/* }}} */
#else /* {{{ if !defined(WINDOWS) */
{
	double bytestotal = 0;
#if defined(HAVE_SYS_STATVFS_H) && defined(HAVE_STATVFS)
	struct statvfs buf;
#elif (defined(HAVE_SYS_STATFS_H) || defined(HAVE_SYS_MOUNT_H)) && defined(HAVE_STATFS)
	struct statfs buf;
#endif

#if defined(HAVE_SYS_STATVFS_H) && defined(HAVE_STATVFS)
	if (statvfs(path, &buf)) {
		crx_error_docref(NULL, E_WARNING, "%s", strerror(errno));
		return FAILURE;
	}
	if (buf.f_frsize) {
		bytestotal = (((double)buf.f_blocks) * ((double)buf.f_frsize));
	} else {
		bytestotal = (((double)buf.f_blocks) * ((double)buf.f_bsize));
	}

#elif (defined(HAVE_SYS_STATFS_H) || defined(HAVE_SYS_MOUNT_H)) && defined(HAVE_STATFS)
	if (statfs(path, &buf)) {
		crx_error_docref(NULL, E_WARNING, "%s", strerror(errno));
		return FAILURE;
	}
	bytestotal = (((double)buf.f_bsize) * ((double)buf.f_blocks));
#endif

	*space = bytestotal;
	return SUCCESS;
}
#endif
/* }}} */
/* }}} */

/* {{{ Get total disk space for filesystem that path is on */
CRX_FUNCTION(disk_total_space)
{
	double bytestotal;
	char *path, fullpath[MAXPATHLEN];
	size_t path_len;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_PATH(path, path_len)
	CREX_PARSE_PARAMETERS_END();

	if (!expand_filepath(path, fullpath)) {
		RETURN_FALSE;
	}

	if (crx_check_open_basedir(fullpath)) {
		RETURN_FALSE;
	}

	if (crx_disk_total_space(fullpath, &bytestotal) == SUCCESS) {
		RETURN_DOUBLE(bytestotal);
	}
	RETURN_FALSE;
}
/* }}} */

static crex_result crx_disk_free_space(char *path, double *space) /* {{{ */
#if defined(WINDOWS) /* {{{ */
{
	ULARGE_INTEGER FreeBytesAvailableToCaller;
	ULARGE_INTEGER TotalNumberOfBytes;
	ULARGE_INTEGER TotalNumberOfFreeBytes;
	CRX_WIN32_IOUTIL_INIT_W(path)

	if (GetDiskFreeSpaceExW(pathw, &FreeBytesAvailableToCaller, &TotalNumberOfBytes, &TotalNumberOfFreeBytes) == 0) {
		char *err = crx_win_err();
		crx_error_docref(NULL, E_WARNING, "%s", err);
		crx_win_err_free(err);
		CRX_WIN32_IOUTIL_CLEANUP_W()
		return FAILURE;
	}

	*space = FreeBytesAvailableToCaller.HighPart * (double) (1ULL << 32)  + FreeBytesAvailableToCaller.LowPart;

	CRX_WIN32_IOUTIL_CLEANUP_W()

	return SUCCESS;
}
#else /* {{{ if !defined(WINDOWS) */
{
	double bytesfree = 0;
#if defined(HAVE_SYS_STATVFS_H) && defined(HAVE_STATVFS)
	struct statvfs buf;
#elif (defined(HAVE_SYS_STATFS_H) || defined(HAVE_SYS_MOUNT_H)) && defined(HAVE_STATFS)
	struct statfs buf;
#endif

#if defined(HAVE_SYS_STATVFS_H) && defined(HAVE_STATVFS)
	if (statvfs(path, &buf)) {
		crx_error_docref(NULL, E_WARNING, "%s", strerror(errno));
		return FAILURE;
	}
	if (buf.f_frsize) {
		bytesfree = (((double)buf.f_bavail) * ((double)buf.f_frsize));
	} else {
		bytesfree = (((double)buf.f_bavail) * ((double)buf.f_bsize));
	}
#elif (defined(HAVE_SYS_STATFS_H) || defined(HAVE_SYS_MOUNT_H)) && defined(HAVE_STATFS)
	if (statfs(path, &buf)) {
		crx_error_docref(NULL, E_WARNING, "%s", strerror(errno));
		return FAILURE;
	}
	bytesfree = (((double)buf.f_bsize) * ((double)buf.f_bavail));
#endif

	*space = bytesfree;
	return SUCCESS;
}
#endif
/* }}} */
/* }}} */

/* {{{ Get free disk space for filesystem that path is on */
CRX_FUNCTION(disk_free_space)
{
	double bytesfree;
	char *path, fullpath[MAXPATHLEN];
	size_t path_len;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_PATH(path, path_len)
	CREX_PARSE_PARAMETERS_END();

	if (!expand_filepath(path, fullpath)) {
		RETURN_FALSE;
	}

	if (crx_check_open_basedir(fullpath)) {
		RETURN_FALSE;
	}

	if (crx_disk_free_space(fullpath, &bytesfree) == SUCCESS) {
		RETURN_DOUBLE(bytesfree);
	}
	RETURN_FALSE;
}
/* }}} */

#ifndef CRX_WIN32
CRXAPI crex_result crx_get_gid_by_name(const char *name, gid_t *gid)
{
#if defined(ZTS) && defined(HAVE_GETGRNAM_R) && defined(_SC_GETGR_R_SIZE_MAX)
		struct group gr;
		struct group *retgrptr;
		long grbuflen = sysconf(_SC_GETGR_R_SIZE_MAX);
		char *grbuf;

		if (grbuflen < 1) {
			return FAILURE;
		}

		grbuf = emalloc(grbuflen);
		if (getgrnam_r(name, &gr, grbuf, grbuflen, &retgrptr) != 0 || retgrptr == NULL) {
			efree(grbuf);
			return FAILURE;
		}
		efree(grbuf);
		*gid = gr.gr_gid;
#else
		struct group *gr = getgrnam(name);

		if (!gr) {
			return FAILURE;
		}
		*gid = gr->gr_gid;
#endif
		return SUCCESS;
}
#endif

static void crx_do_chgrp(INTERNAL_FUNCTION_PARAMETERS, int do_lchgrp) /* {{{ */
{
	char *filename;
	size_t filename_len;
	crex_string *group_str;
	crex_long group_long;
#if !defined(WINDOWS)
	gid_t gid;
	int ret;
#endif
	crx_stream_wrapper *wrapper;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_PATH(filename, filename_len)
		C_PARAM_STR_OR_LONG(group_str, group_long)
	CREX_PARSE_PARAMETERS_END();

	wrapper = crx_stream_locate_url_wrapper(filename, NULL, 0);
	if(wrapper != &crx_plain_files_wrapper || strncasecmp("file://", filename, 7) == 0) {
		if(wrapper && wrapper->wops->stream_metadata) {
			int option;
			void *value;
			if (group_str) {
				option = CRX_STREAM_META_GROUP_NAME;
				value = ZSTR_VAL(group_str);
			} else {
				option = CRX_STREAM_META_GROUP;
				value = &group_long;
			}

			if(wrapper->wops->stream_metadata(wrapper, filename, option, value, NULL)) {
				RETURN_TRUE;
			} else {
				RETURN_FALSE;
			}
		} else {
#ifndef WINDOWS
/* On Windows, we expect regular chgrp to fail silently by default */
			crx_error_docref(NULL, E_WARNING, "Cannot call chgrp() for a non-standard stream");
#endif
			RETURN_FALSE;
		}
	}

#ifdef WINDOWS
	/* We have no native chgrp on Windows, nothing left to do if stream doesn't have own implementation */
	RETURN_FALSE;
#else
	if (group_str) {
		if (crx_get_gid_by_name(ZSTR_VAL(group_str), &gid) != SUCCESS) {
			crx_error_docref(NULL, E_WARNING, "Unable to find gid for %s", ZSTR_VAL(group_str));
			RETURN_FALSE;
		}
	} else {
		gid = (gid_t) group_long;
	}

	/* Check the basedir */
	if (crx_check_open_basedir(filename)) {
		RETURN_FALSE;
	}

	if (do_lchgrp) {
#ifdef HAVE_LCHOWN
		ret = VCWD_LCHOWN(filename, -1, gid);
#endif
	} else {
		ret = VCWD_CHOWN(filename, -1, gid);
	}
	if (ret == -1) {
		crx_error_docref(NULL, E_WARNING, "%s", strerror(errno));
		RETURN_FALSE;
	}
	RETURN_TRUE;
#endif
}
/* }}} */

/* {{{ Change file group */
CRX_FUNCTION(chgrp)
{
	crx_do_chgrp(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ Change symlink group */
#ifdef HAVE_LCHOWN
CRX_FUNCTION(lchgrp)
{
	crx_do_chgrp(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
#endif
/* }}} */

#ifndef CRX_WIN32
CRXAPI crex_result crx_get_uid_by_name(const char *name, uid_t *uid)
{
#if defined(ZTS) && defined(_SC_GETPW_R_SIZE_MAX) && defined(HAVE_GETPWNAM_R)
		struct passwd pw;
		struct passwd *retpwptr = NULL;
		long pwbuflen = sysconf(_SC_GETPW_R_SIZE_MAX);
		char *pwbuf;

		if (pwbuflen < 1) {
			return FAILURE;
		}

		pwbuf = emalloc(pwbuflen);
		if (getpwnam_r(name, &pw, pwbuf, pwbuflen, &retpwptr) != 0 || retpwptr == NULL) {
			efree(pwbuf);
			return FAILURE;
		}
		efree(pwbuf);
		*uid = pw.pw_uid;
#else
		struct passwd *pw = getpwnam(name);

		if (!pw) {
			return FAILURE;
		}
		*uid = pw->pw_uid;
#endif
		return SUCCESS;
}
#endif

static void crx_do_chown(INTERNAL_FUNCTION_PARAMETERS, int do_lchown) /* {{{ */
{
	char *filename;
	size_t filename_len;
	crex_string *user_str;
	crex_long user_long;
#if !defined(WINDOWS)
	uid_t uid;
	int ret;
#endif
	crx_stream_wrapper *wrapper;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_PATH(filename, filename_len)
		C_PARAM_STR_OR_LONG(user_str, user_long)
	CREX_PARSE_PARAMETERS_END();

	wrapper = crx_stream_locate_url_wrapper(filename, NULL, 0);
	if(wrapper != &crx_plain_files_wrapper || strncasecmp("file://", filename, 7) == 0) {
		if(wrapper && wrapper->wops->stream_metadata) {
			int option;
			void *value;
			if (user_str) {
				option = CRX_STREAM_META_OWNER_NAME;
				value = ZSTR_VAL(user_str);
			} else {
				option = CRX_STREAM_META_OWNER;
				value = &user_long;
			}

			if(wrapper->wops->stream_metadata(wrapper, filename, option, value, NULL)) {
				RETURN_TRUE;
			} else {
				RETURN_FALSE;
			}
		} else {
#ifndef WINDOWS
/* On Windows, we expect regular chown to fail silently by default */
			crx_error_docref(NULL, E_WARNING, "Cannot call chown() for a non-standard stream");
#endif
			RETURN_FALSE;
		}
	}

#ifdef WINDOWS
	/* We have no native chown on Windows, nothing left to do if stream doesn't have own implementation */
	RETURN_FALSE;
#else

	if (user_str) {
		if (crx_get_uid_by_name(ZSTR_VAL(user_str), &uid) != SUCCESS) {
			crx_error_docref(NULL, E_WARNING, "Unable to find uid for %s", ZSTR_VAL(user_str));
			RETURN_FALSE;
		}
	} else {
		uid = (uid_t) user_long;
	}

	/* Check the basedir */
	if (crx_check_open_basedir(filename)) {
		RETURN_FALSE;
	}

	if (do_lchown) {
#ifdef HAVE_LCHOWN
		ret = VCWD_LCHOWN(filename, uid, -1);
#endif
	} else {
		ret = VCWD_CHOWN(filename, uid, -1);
	}
	if (ret == -1) {
		crx_error_docref(NULL, E_WARNING, "%s", strerror(errno));
		RETURN_FALSE;
	}
	RETURN_TRUE;
#endif
}
/* }}} */


/* {{{ Change file owner */
CRX_FUNCTION(chown)
{
	crx_do_chown(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ Change file owner */
#ifdef HAVE_LCHOWN
CRX_FUNCTION(lchown)
{
	RETVAL_TRUE;
	crx_do_chown(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
#endif
/* }}} */

/* {{{ Change file mode */
CRX_FUNCTION(chmod)
{
	char *filename;
	size_t filename_len;
	crex_long mode;
	int ret;
	mode_t imode;
	crx_stream_wrapper *wrapper;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_PATH(filename, filename_len)
		C_PARAM_LONG(mode)
	CREX_PARSE_PARAMETERS_END();

	wrapper = crx_stream_locate_url_wrapper(filename, NULL, 0);
	if(wrapper != &crx_plain_files_wrapper || strncasecmp("file://", filename, 7) == 0) {
		if(wrapper && wrapper->wops->stream_metadata) {
			if(wrapper->wops->stream_metadata(wrapper, filename, CRX_STREAM_META_ACCESS, &mode, NULL)) {
				RETURN_TRUE;
			} else {
				RETURN_FALSE;
			}
		} else {
			crx_error_docref(NULL, E_WARNING, "Cannot call chmod() for a non-standard stream");
			RETURN_FALSE;
		}
	}

	/* Check the basedir */
	if (crx_check_open_basedir(filename)) {
		RETURN_FALSE;
	}

	imode = (mode_t) mode;

	ret = VCWD_CHMOD(filename, imode);
	if (ret == -1) {
		crx_error_docref(NULL, E_WARNING, "%s", strerror(errno));
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

#ifdef HAVE_UTIME
/* {{{ Set modification time of file */
CRX_FUNCTION(touch)
{
	char *filename;
	size_t filename_len;
	crex_long filetime = 0, fileatime = 0;
	bool filetime_is_null = 1, fileatime_is_null = 1;
	int ret;
	FILE *file;
	struct utimbuf newtimebuf;
	struct utimbuf *newtime = &newtimebuf;
	crx_stream_wrapper *wrapper;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_PATH(filename, filename_len)
		C_PARAM_OPTIONAL
		C_PARAM_LONG_OR_NULL(filetime, filetime_is_null)
		C_PARAM_LONG_OR_NULL(fileatime, fileatime_is_null)
	CREX_PARSE_PARAMETERS_END();

	if (!filename_len) {
		RETURN_FALSE;
	}

	if (filetime_is_null && fileatime_is_null) {
		newtime = NULL;
	} else if (!filetime_is_null && fileatime_is_null) {
		newtime->modtime = newtime->actime = filetime;
	} else if (filetime_is_null && !fileatime_is_null) {
		crex_argument_value_error(2, "cannot be null when argument #3 ($atime) is an integer");
		RETURN_THROWS();
	} else {
		newtime->modtime = filetime;
		newtime->actime = fileatime;
	}

	wrapper = crx_stream_locate_url_wrapper(filename, NULL, 0);
	if(wrapper != &crx_plain_files_wrapper || strncasecmp("file://", filename, 7) == 0) {
		if(wrapper && wrapper->wops->stream_metadata) {
			if(wrapper->wops->stream_metadata(wrapper, filename, CRX_STREAM_META_TOUCH, newtime, NULL)) {
				RETURN_TRUE;
			} else {
				RETURN_FALSE;
			}
		} else {
			crx_stream *stream;
			if(!filetime_is_null || !fileatime_is_null) {
				crx_error_docref(NULL, E_WARNING, "Cannot call touch() for a non-standard stream");
				RETURN_FALSE;
			}
			stream = crx_stream_open_wrapper_ex(filename, "c", REPORT_ERRORS, NULL, NULL);
			if(stream != NULL) {
				crx_stream_close(stream);
				RETURN_TRUE;
			} else {
				RETURN_FALSE;
			}
		}
	}

	/* Check the basedir */
	if (crx_check_open_basedir(filename)) {
		RETURN_FALSE;
	}

	/* create the file if it doesn't exist already */
	if (VCWD_ACCESS(filename, F_OK) != 0) {
		file = VCWD_FOPEN(filename, "w");
		if (file == NULL) {
			crx_error_docref(NULL, E_WARNING, "Unable to create file %s because %s", filename, strerror(errno));
			RETURN_FALSE;
		}
		fclose(file);
	}

	ret = VCWD_UTIME(filename, newtime);
	if (ret == -1) {
		crx_error_docref(NULL, E_WARNING, "Utime failed: %s", strerror(errno));
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */
#endif

/* {{{ crx_clear_stat_cache() */
CRXAPI void crx_clear_stat_cache(bool clear_realpath_cache, const char *filename, size_t filename_len)
{
	/* always clear CurrentStatFile and CurrentLStatFile even if filename is not NULL
	 * as it may contain outdated data (e.g. "nlink" for a directory when deleting a file
	 * in this directory, as shown by lstat_stat_variation9.crxt) */
	if (BG(CurrentStatFile)) {
		crex_string_release(BG(CurrentStatFile));
		BG(CurrentStatFile) = NULL;
	}
	if (BG(CurrentLStatFile)) {
		crex_string_release(BG(CurrentLStatFile));
		BG(CurrentLStatFile) = NULL;
	}
	if (clear_realpath_cache) {
		if (filename != NULL) {
			realpath_cache_del(filename, filename_len);
		} else {
			realpath_cache_clean();
		}
	}
}
/* }}} */

/* {{{ Clear file stat cache */
CRX_FUNCTION(clearstatcache)
{
	bool  clear_realpath_cache = 0;
	char      *filename             = NULL;
	size_t     filename_len         = 0;

	CREX_PARSE_PARAMETERS_START(0, 2)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(clear_realpath_cache)
		C_PARAM_PATH(filename, filename_len)
	CREX_PARSE_PARAMETERS_END();

	crx_clear_stat_cache(clear_realpath_cache, filename, filename_len);
}
/* }}} */

#define IS_LINK_OPERATION(__t) ((__t) == FS_TYPE || (__t) == FS_IS_LINK || (__t) == FS_LSTAT || (__t) == FS_LPERMS)
#define IS_EXISTS_CHECK(__t) ((__t) == FS_EXISTS  || (__t) == FS_IS_W || (__t) == FS_IS_R || (__t) == FS_IS_X || (__t) == FS_IS_FILE || (__t) == FS_IS_DIR || (__t) == FS_IS_LINK || (__t) == FS_LPERMS)
#define IS_ABLE_CHECK(__t) ((__t) == FS_IS_R || (__t) == FS_IS_W || (__t) == FS_IS_X)
#define IS_ACCESS_CHECK(__t) (IS_ABLE_CHECK(type) || (__t) == FS_EXISTS)

/* {{{ crx_stat */
CRXAPI void crx_stat(crex_string *filename, int type, zval *return_value)
{
	crx_stream_statbuf ssb = {0};
	crex_stat_t *stat_sb = &ssb.sb;
	int flags = 0, rmask=S_IROTH, wmask=S_IWOTH, xmask=S_IXOTH; /* access rights defaults to other */
	const char *local = NULL;
	crx_stream_wrapper *wrapper = NULL;

	if (IS_ACCESS_CHECK(type)) {
		if (!ZSTR_LEN(filename) || CHECK_NULL_PATH(ZSTR_VAL(filename), ZSTR_LEN(filename))) {
			if (ZSTR_LEN(filename) && !IS_EXISTS_CHECK(type)) {
				crx_error_docref(NULL, E_WARNING, "Filename contains null byte");
			}
			RETURN_FALSE;
		}

		if ((wrapper = crx_stream_locate_url_wrapper(ZSTR_VAL(filename), &local, 0)) == &crx_plain_files_wrapper
				&& crx_check_open_basedir(local)) {
			RETURN_FALSE;
		}

		if (wrapper == &crx_plain_files_wrapper) {
			char realpath[MAXPATHLEN];
			const char *file_path_to_check;
			/* if the wrapper is not found, we need to expand path to match open behavior */
			if (EXPECTED(!crx_is_stream_path(local) || expand_filepath(local, realpath) == NULL)) {
				file_path_to_check = local;
			} else {
				file_path_to_check = realpath;
			}
			switch (type) {
#ifdef F_OK
				case FS_EXISTS:
					RETURN_BOOL(VCWD_ACCESS(file_path_to_check, F_OK) == 0);
					break;
#endif
#ifdef W_OK
				case FS_IS_W:
					RETURN_BOOL(VCWD_ACCESS(file_path_to_check, W_OK) == 0);
					break;
#endif
#ifdef R_OK
				case FS_IS_R:
					RETURN_BOOL(VCWD_ACCESS(file_path_to_check, R_OK) == 0);
					break;
#endif
#ifdef X_OK
				case FS_IS_X:
					RETURN_BOOL(VCWD_ACCESS(file_path_to_check, X_OK) == 0);
					break;
#endif
			}
		}
	}

	if (IS_LINK_OPERATION(type)) {
		flags |= CRX_STREAM_URL_STAT_LINK;
	}
	if (IS_EXISTS_CHECK(type)) {
		flags |= CRX_STREAM_URL_STAT_QUIET;
	}

	do {
		/* Try to hit the cache first */
		if (flags & CRX_STREAM_URL_STAT_LINK) {
			if (filename == BG(CurrentLStatFile)
			 || (BG(CurrentLStatFile)
			  && crex_string_equal_content(filename, BG(CurrentLStatFile)))) {
				stat_sb = &BG(lssb).sb;
				break;
			}
		} else {
			if (filename == BG(CurrentStatFile)
			 || (BG(CurrentStatFile)
			  && crex_string_equal_content(filename, BG(CurrentStatFile)))) {
				stat_sb = &BG(ssb).sb;
				break;
			}
		}

		if (!wrapper) {
			if (!ZSTR_LEN(filename) || CHECK_NULL_PATH(ZSTR_VAL(filename), ZSTR_LEN(filename))) {
				if (ZSTR_LEN(filename) && !IS_EXISTS_CHECK(type)) {
					crx_error_docref(NULL, E_WARNING, "Filename contains null byte");
				}
				RETURN_FALSE;
			}

			if ((wrapper = crx_stream_locate_url_wrapper(ZSTR_VAL(filename), &local, 0)) == &crx_plain_files_wrapper
			 && crx_check_open_basedir(local)) {
				RETURN_FALSE;
			}
		}

		if (!wrapper
		 || !wrapper->wops->url_stat
		 || wrapper->wops->url_stat(wrapper, local, flags | CRX_STREAM_URL_STAT_IGNORE_OPEN_BASEDIR, &ssb, NULL)) {
			/* Error Occurred */
			if (!IS_EXISTS_CHECK(type)) {
				crx_error_docref(NULL, E_WARNING, "%sstat failed for %s", IS_LINK_OPERATION(type) ? "L" : "", ZSTR_VAL(filename));
			}
			RETURN_FALSE;
		}

		/* Drop into cache */
		if (flags & CRX_STREAM_URL_STAT_LINK) {
			if (BG(CurrentLStatFile)) {
				crex_string_release(BG(CurrentLStatFile));
			}
			BG(CurrentLStatFile) = crex_string_copy(filename);
			memcpy(&BG(lssb), &ssb, sizeof(crx_stream_statbuf));
		}
		if (!(flags & CRX_STREAM_URL_STAT_LINK)
		 || !S_ISLNK(ssb.sb.st_mode)) {
			if (BG(CurrentStatFile)) {
				crex_string_release(BG(CurrentStatFile));
			}
			BG(CurrentStatFile) = crex_string_copy(filename);
			memcpy(&BG(ssb), &ssb, sizeof(crx_stream_statbuf));
		}
	} while (0);

	if (type >= FS_IS_W && type <= FS_IS_X) {
		if(stat_sb->st_uid==getuid()) {
			rmask=S_IRUSR;
			wmask=S_IWUSR;
			xmask=S_IXUSR;
		} else if(stat_sb->st_gid==getgid()) {
			rmask=S_IRGRP;
			wmask=S_IWGRP;
			xmask=S_IXGRP;
		} else {
			int   groups, n, i;
			gid_t *gids;

			groups = getgroups(0, NULL);
			if(groups > 0) {
				gids=(gid_t *)safe_emalloc(groups, sizeof(gid_t), 0);
				n=getgroups(groups, gids);
				for(i=0;i<n;i++){
					if(stat_sb->st_gid==gids[i]) {
						rmask=S_IRGRP;
						wmask=S_IWGRP;
						xmask=S_IXGRP;
						break;
					}
				}
				efree(gids);
			}
		}
	}

	if (IS_ABLE_CHECK(type) && getuid() == 0) {
		/* root has special perms on plain_wrapper */
		if (wrapper == &crx_plain_files_wrapper) {
			if (type == FS_IS_X) {
				xmask = S_IXROOT;
			} else {
				RETURN_TRUE;
			}
		}
	}

	switch (type) {
	case FS_PERMS:
	case FS_LPERMS:
		RETURN_LONG((crex_long)stat_sb->st_mode);
	case FS_INODE:
		RETURN_LONG((crex_long)stat_sb->st_ino);
	case FS_SIZE:
		RETURN_LONG((crex_long)stat_sb->st_size);
	case FS_OWNER:
		RETURN_LONG((crex_long)stat_sb->st_uid);
	case FS_GROUP:
		RETURN_LONG((crex_long)stat_sb->st_gid);
	case FS_ATIME:
		RETURN_LONG((crex_long)stat_sb->st_atime);
	case FS_MTIME:
		RETURN_LONG((crex_long)stat_sb->st_mtime);
	case FS_CTIME:
		RETURN_LONG((crex_long)stat_sb->st_ctime);
	case FS_TYPE:
		if (S_ISLNK(stat_sb->st_mode)) {
			RETURN_STRING("link");
		}
		switch(stat_sb->st_mode & S_IFMT) {
		case S_IFIFO: RETURN_STRING("fifo");
		case S_IFCHR: RETURN_STRING("char");
		case S_IFDIR: RETURN_STRING("dir");
		case S_IFBLK: RETURN_STRING("block");
		case S_IFREG: RETURN_STR(ZSTR_KNOWN(CREX_STR_FILE)); /* "file" */
#if defined(S_IFSOCK) && !defined(CRX_WIN32)
		case S_IFSOCK: RETURN_STRING("socket");
#endif
		}
		crx_error_docref(NULL, E_NOTICE, "Unknown file type (%d)", stat_sb->st_mode&S_IFMT);
		RETURN_STRING("unknown");
	case FS_IS_W:
		RETURN_BOOL((stat_sb->st_mode & wmask) != 0);
	case FS_IS_R:
		RETURN_BOOL((stat_sb->st_mode & rmask) != 0);
	case FS_IS_X:
		RETURN_BOOL((stat_sb->st_mode & xmask) != 0);
	case FS_IS_FILE:
		RETURN_BOOL(S_ISREG(stat_sb->st_mode));
	case FS_IS_DIR:
		RETURN_BOOL(S_ISDIR(stat_sb->st_mode));
	case FS_IS_LINK:
		RETURN_BOOL(S_ISLNK(stat_sb->st_mode));
	case FS_EXISTS:
		RETURN_TRUE; /* the false case was done earlier */
	case FS_LSTAT:
		/* FALLTHROUGH */
	case FS_STAT: {
		char *stat_sb_names[] = {
			"dev", "ino", "mode", "nlink", "uid", "gid", "rdev",
			"size", "atime", "mtime", "ctime", "blksize", "blocks"
		};
		zval stat_dev, stat_ino, stat_mode, stat_nlink, stat_uid, stat_gid, stat_rdev,
			stat_size, stat_atime, stat_mtime, stat_ctime, stat_blksize, stat_blocks;
		zval *stat_sb_addresses[] = {
			&stat_dev, &stat_ino, &stat_mode, &stat_nlink, &stat_uid, &stat_gid, &stat_rdev,
			&stat_size, &stat_atime, &stat_mtime, &stat_ctime, &stat_blksize, &stat_blocks
		};
		size_t i, size_stat_sb = sizeof(stat_sb_addresses) / sizeof(*stat_sb_addresses);

		array_init(return_value);

		ZVAL_LONG(&stat_dev, stat_sb->st_dev);
		ZVAL_LONG(&stat_ino, stat_sb->st_ino);
		ZVAL_LONG(&stat_mode, stat_sb->st_mode);
		ZVAL_LONG(&stat_nlink, stat_sb->st_nlink);
		ZVAL_LONG(&stat_uid, stat_sb->st_uid);
		ZVAL_LONG(&stat_gid, stat_sb->st_gid);
#ifdef HAVE_STRUCT_STAT_ST_RDEV
		ZVAL_LONG(&stat_rdev, stat_sb->st_rdev);
#else
		ZVAL_LONG(&stat_rdev, -1);
#endif
		ZVAL_LONG(&stat_size, stat_sb->st_size);
		ZVAL_LONG(&stat_atime, stat_sb->st_atime);
		ZVAL_LONG(&stat_mtime, stat_sb->st_mtime);
		ZVAL_LONG(&stat_ctime, stat_sb->st_ctime);
#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
		ZVAL_LONG(&stat_blksize, stat_sb->st_blksize);
#else
		ZVAL_LONG(&stat_blksize,-1);
#endif
#ifdef HAVE_STRUCT_STAT_ST_BLOCKS
		ZVAL_LONG(&stat_blocks, stat_sb->st_blocks);
#else
		ZVAL_LONG(&stat_blocks,-1);
#endif
		for (i = 0; i < size_stat_sb; i++) {
			/* Store numeric indexes in proper order */
			crex_hash_next_index_insert(C_ARRVAL_P(return_value), stat_sb_addresses[i]);
		}

		for (i = 0; i < size_stat_sb; i++) {
			/* Store string indexes referencing the same zval */
			crex_hash_str_add_new(C_ARRVAL_P(return_value), stat_sb_names[i], strlen(stat_sb_names[i]), stat_sb_addresses[i]);
		}

		return;
	    }
	}
	crx_error_docref(NULL, E_WARNING, "Didn't understand stat call");
	RETURN_FALSE;
}
/* }}} */

/* another quickie macro to make defining similar functions easier */
/* {{{ FileFunction(name, funcnum) */
#define FileFunction(name, funcnum) \
CREX_NAMED_FUNCTION(name) { \
	crex_string *filename; \
	\
	CREX_PARSE_PARAMETERS_START(1, 1) \
		C_PARAM_STR(filename) \
	CREX_PARSE_PARAMETERS_END(); \
	\
	crx_stat(filename, funcnum, return_value); \
}
/* }}} */

/* {{{ Get file permissions */
FileFunction(CRX_FN(fileperms), FS_PERMS)
/* }}} */

/* {{{ Get file inode */
FileFunction(CRX_FN(fileinode), FS_INODE)
/* }}} */

/* {{{ Get file size */
FileFunction(CRX_FN(filesize), FS_SIZE)
/* }}} */

/* {{{ Get file owner */
FileFunction(CRX_FN(fileowner), FS_OWNER)
/* }}} */

/* {{{ Get file group */
FileFunction(CRX_FN(filegroup), FS_GROUP)
/* }}} */

/* {{{ Get last access time of file */
FileFunction(CRX_FN(fileatime), FS_ATIME)
/* }}} */

/* {{{ Get last modification time of file */
FileFunction(CRX_FN(filemtime), FS_MTIME)
/* }}} */

/* {{{ Get inode modification time of file */
FileFunction(CRX_FN(filectime), FS_CTIME)
/* }}} */

/* {{{ Get file type */
FileFunction(CRX_FN(filetype), FS_TYPE)
/* }}} */

/* {{{ Returns true if file can be written */
FileFunction(CRX_FN(is_writable), FS_IS_W)
/* }}} */

/* {{{ Returns true if file can be read */
FileFunction(CRX_FN(is_readable), FS_IS_R)
/* }}} */

/* {{{ Returns true if file is executable */
FileFunction(CRX_FN(is_executable), FS_IS_X)
/* }}} */

/* {{{ Returns true if file is a regular file */
FileFunction(CRX_FN(is_file), FS_IS_FILE)
/* }}} */

/* {{{ Returns true if file is directory */
FileFunction(CRX_FN(is_dir), FS_IS_DIR)
/* }}} */

/* {{{ Returns true if file is symbolic link */
FileFunction(CRX_FN(is_link), FS_IS_LINK)
/* }}} */

/* {{{ Returns true if filename exists */
FileFunction(CRX_FN(file_exists), FS_EXISTS)
/* }}} */

/* {{{ Give information about a file or symbolic link */
FileFunction(CRX_FN(lstat), FS_LSTAT)
/* }}} */

/* {{{ Give information about a file */
FileFunction(CRX_FN(stat), FS_STAT)
/* }}} */

/* {{{ Get current size of realpath cache */
CRX_FUNCTION(realpath_cache_size)
{
	CREX_PARSE_PARAMETERS_NONE();

	RETURN_LONG(realpath_cache_size());
}

/* {{{ Get current size of realpath cache */
CRX_FUNCTION(realpath_cache_get)
{
	realpath_cache_bucket **buckets = realpath_cache_get_buckets(), **end = buckets + realpath_cache_max_buckets();

	CREX_PARSE_PARAMETERS_NONE();

	array_init(return_value);
	while(buckets < end) {
		realpath_cache_bucket *bucket = *buckets;
		while(bucket) {
			zval entry;

			array_init(&entry);

			/* bucket->key is unsigned long */
			if (CREX_LONG_MAX >= bucket->key) {
				add_assoc_long_ex(&entry, "key", sizeof("key") - 1, bucket->key);
			} else {
				add_assoc_double_ex(&entry, "key", sizeof("key") - 1, (double)bucket->key);
			}
			add_assoc_bool_ex(&entry, "is_dir", sizeof("is_dir") - 1, bucket->is_dir);
			add_assoc_stringl_ex(&entry, "realpath", sizeof("realpath") - 1, bucket->realpath, bucket->realpath_len);
			add_assoc_long_ex(&entry, "expires", sizeof("expires") - 1, bucket->expires);
#ifdef CRX_WIN32
			add_assoc_bool_ex(&entry, "is_rvalid", sizeof("is_rvalid") - 1, bucket->is_rvalid);
			add_assoc_bool_ex(&entry, "is_wvalid", sizeof("is_wvalid") - 1, bucket->is_wvalid);
			add_assoc_bool_ex(&entry, "is_readable", sizeof("is_readable") - 1, bucket->is_readable);
			add_assoc_bool_ex(&entry, "is_writable", sizeof("is_writable") - 1, bucket->is_writable);
#endif
			crex_hash_str_update(C_ARRVAL_P(return_value), bucket->path, bucket->path_len, &entry);
			bucket = bucket->next;
		}
		buckets++;
	}
}
