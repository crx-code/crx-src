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
   | Author: Kristian Koehntopp <kris@koehntopp.de>                       |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include <unistd.h>
#include "ext/standard/info.h"
#include "ext/standard/crx_string.h"
#include "crx_posix.h"

#ifdef HAVE_POSIX

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/times.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#ifdef HAVE_SYS_MKDEV_H
# include <sys/mkdev.h>
#endif
#ifdef HAVE_SYS_SYSMACROS_H
# include <sys/sysmacros.h>
#endif

#include "posix_arginfo.h"

CREX_DECLARE_MODULE_GLOBALS(posix)
static CRX_MINFO_FUNCTION(posix);

/* {{{ CRX_MINFO_FUNCTION */
static CRX_MINFO_FUNCTION(posix)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "POSIX support", "enabled");
	crx_info_print_table_end();
}
/* }}} */

static CRX_GINIT_FUNCTION(posix) /* {{{ */
{
#if defined(COMPILE_DL_POSIX) && defined(ZTS)
	CREX_TSRMLS_CACHE_UPDATE();
#endif
	posix_globals->last_error = 0;
}
/* }}} */

/* {{{ CRX_MINIT_FUNCTION(posix) */
static CRX_MINIT_FUNCTION(posix)
{
	register_posix_symbols(module_number);

	return SUCCESS;
}
/* }}} */

/* {{{ posix_module_entry */
crex_module_entry posix_module_entry = {
	STANDARD_MODULE_HEADER,
	"posix",
	ext_functions,
	CRX_MINIT(posix),
	NULL,
	NULL,
	NULL,
	CRX_MINFO(posix),
	CRX_POSIX_VERSION,
	CRX_MODULE_GLOBALS(posix),
	CRX_GINIT(posix),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_POSIX
#ifdef ZTS
CREX_TSRMLS_CACHE_DEFINE()
#endif
CREX_GET_MODULE(posix)
#endif

#define CRX_POSIX_RETURN_LONG_FUNC(func_name)	\
	CREX_PARSE_PARAMETERS_NONE();	\
	RETURN_LONG(func_name());

#define CRX_POSIX_SINGLE_ARG_FUNC(func_name)	\
	crex_long val;	\
	CREX_PARSE_PARAMETERS_START(1, 1) \
		C_PARAM_LONG(val) \
	CREX_PARSE_PARAMETERS_END(); \
	if (func_name(val) < 0) {	\
		POSIX_G(last_error) = errno;	\
		RETURN_FALSE;	\
	}	\
	RETURN_TRUE;

/* {{{ Send a signal to a process (POSIX.1, 3.3.2) */

CRX_FUNCTION(posix_kill)
{
	crex_long pid, sig;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_LONG(pid)
		C_PARAM_LONG(sig)
	CREX_PARSE_PARAMETERS_END();

	if (kill(pid, sig) < 0) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Get the current process id (POSIX.1, 4.1.1) */
CRX_FUNCTION(posix_getpid)
{
	CRX_POSIX_RETURN_LONG_FUNC(getpid);
}
/* }}} */

/* {{{ Get the parent process id (POSIX.1, 4.1.1) */
CRX_FUNCTION(posix_getppid)
{
	CRX_POSIX_RETURN_LONG_FUNC(getppid);
}
/* }}} */

/* {{{ Get the current user id (POSIX.1, 4.2.1) */
CRX_FUNCTION(posix_getuid)
{
	CRX_POSIX_RETURN_LONG_FUNC(getuid);
}
/* }}} */

/* {{{ Get the current group id (POSIX.1, 4.2.1) */
CRX_FUNCTION(posix_getgid)
{
	CRX_POSIX_RETURN_LONG_FUNC(getgid);
}
/* }}} */

/* {{{ Get the current effective user id (POSIX.1, 4.2.1) */
CRX_FUNCTION(posix_geteuid)
{
	CRX_POSIX_RETURN_LONG_FUNC(geteuid);
}
/* }}} */

/* {{{ Get the current effective group id (POSIX.1, 4.2.1) */
CRX_FUNCTION(posix_getegid)
{
	CRX_POSIX_RETURN_LONG_FUNC(getegid);
}
/* }}} */

/* {{{ Set user id (POSIX.1, 4.2.2) */
CRX_FUNCTION(posix_setuid)
{
	CRX_POSIX_SINGLE_ARG_FUNC(setuid);
}
/* }}} */

/* {{{ Set group id (POSIX.1, 4.2.2) */
CRX_FUNCTION(posix_setgid)
{
	CRX_POSIX_SINGLE_ARG_FUNC(setgid);
}
/* }}} */

/* {{{ Set effective user id */
#ifdef HAVE_SETEUID
CRX_FUNCTION(posix_seteuid)
{
	CRX_POSIX_SINGLE_ARG_FUNC(seteuid);
}
#endif
/* }}} */

/* {{{ Set effective group id */
#ifdef HAVE_SETEGID
CRX_FUNCTION(posix_setegid)
{
	CRX_POSIX_SINGLE_ARG_FUNC(setegid);
}
#endif
/* }}} */

/* {{{ Get supplementary group id's (POSIX.1, 4.2.3) */
#ifdef HAVE_GETGROUPS
CRX_FUNCTION(posix_getgroups)
{
	gid_t *gidlist;
	int    result;
	int    i;

	CREX_PARSE_PARAMETERS_NONE();

	/* MacOS may return more than NGROUPS_MAX groups.
	 * Fetch the actual number of groups and create an appropriate allocation. */
	if ((result = getgroups(0, NULL)) < 0) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	gidlist = emalloc(sizeof(gid_t) * result);
	if ((result = getgroups(result, gidlist)) < 0) {
		POSIX_G(last_error) = errno;
		efree(gidlist);
		RETURN_FALSE;
	}

	array_init(return_value);

	for (i=0; i<result; i++) {
		add_next_index_long(return_value, gidlist[i]);
	}
	efree(gidlist);
}
#endif
/* }}} */

/* {{{ Get user name (POSIX.1, 4.2.4) */
#ifdef HAVE_GETLOGIN
CRX_FUNCTION(posix_getlogin)
{
	char *p;

	CREX_PARSE_PARAMETERS_NONE();

	if (NULL == (p = getlogin())) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	RETURN_STRING(p);
}
#endif
/* }}} */

/* {{{ Get current process group id (POSIX.1, 4.3.1) */
CRX_FUNCTION(posix_getpgrp)
{
	CRX_POSIX_RETURN_LONG_FUNC(getpgrp);
}
/* }}} */

/* {{{ Create session and set process group id (POSIX.1, 4.3.2) */
#ifdef HAVE_SETSID
CRX_FUNCTION(posix_setsid)
{
	CRX_POSIX_RETURN_LONG_FUNC(setsid);
}
#endif
/* }}} */

/* {{{ Set process group id for job control (POSIX.1, 4.3.3) */
CRX_FUNCTION(posix_setpgid)
{
	crex_long pid, pgid;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_LONG(pid)
		C_PARAM_LONG(pgid)
	CREX_PARSE_PARAMETERS_END();

	if (setpgid(pid, pgid) < 0) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Get the process group id of the specified process (This is not a POSIX function, but a SVR4ism, so we compile conditionally) */
#ifdef HAVE_GETPGID
CRX_FUNCTION(posix_getpgid)
{
	crex_long val;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(val)
	CREX_PARSE_PARAMETERS_END();

	if ((val = getpgid(val)) < 0) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}
	RETURN_LONG(val);
}
#endif
/* }}} */

/* {{{ Get process group id of session leader (This is not a POSIX function, but a SVR4ism, so be compile conditionally) */
#ifdef HAVE_GETSID
CRX_FUNCTION(posix_getsid)
{
	crex_long val;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(val)
	CREX_PARSE_PARAMETERS_END();

	if ((val = getsid(val)) < 0) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}
	RETURN_LONG(val);
}
#endif
/* }}} */

/* {{{ Get system name (POSIX.1, 4.4.1) */
CRX_FUNCTION(posix_uname)
{
	struct utsname u;

	CREX_PARSE_PARAMETERS_NONE();

	if (uname(&u) < 0) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	array_init(return_value);

	add_assoc_string(return_value, "sysname",  u.sysname);
	add_assoc_string(return_value, "nodename", u.nodename);
	add_assoc_string(return_value, "release",  u.release);
	add_assoc_string(return_value, "version",  u.version);
	add_assoc_string(return_value, "machine",  u.machine);

#if defined(_GNU_SOURCE) && !defined(DARWIN) && defined(HAVE_UTSNAME_DOMAINNAME)
	add_assoc_string(return_value, "domainname", u.domainname);
#endif
}
/* }}} */

/* POSIX.1, 4.5.1 time() - Get System Time
							already covered by CRX
 */

/* {{{ Get process times (POSIX.1, 4.5.2) */
CRX_FUNCTION(posix_times)
{
	struct tms t;
	clock_t    ticks;

	CREX_PARSE_PARAMETERS_NONE();

	if ((ticks = times(&t)) == -1) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	array_init(return_value);

	add_assoc_long(return_value, "ticks",	ticks);			/* clock ticks */
	add_assoc_long(return_value, "utime",	t.tms_utime);	/* user time */
	add_assoc_long(return_value, "stime",	t.tms_stime);	/* system time */
	add_assoc_long(return_value, "cutime",	t.tms_cutime);	/* user time of children */
	add_assoc_long(return_value, "cstime",	t.tms_cstime);	/* system time of children */
}
/* }}} */

/* POSIX.1, 4.6.1 getenv() - Environment Access
							already covered by CRX
*/

/* {{{ Generate terminal path name (POSIX.1, 4.7.1) */
#ifdef HAVE_CTERMID
CRX_FUNCTION(posix_ctermid)
{
	char  buffer[L_ctermid];

	CREX_PARSE_PARAMETERS_NONE();

	if (NULL == ctermid(buffer)) {
		/* Found no documentation how the defined behaviour is when this
		 * function fails
		 */
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	RETURN_STRING(buffer);
}
#endif
/* }}} */

/* Checks if the provides resource is a stream and if it provides a file descriptor */
static int crx_posix_stream_get_fd(zval *zfp, crex_long *fd) /* {{{ */
{
	crx_stream *stream;

	crx_stream_from_zval_no_verify(stream, zfp);

	if (stream == NULL) {
		return 0;
	}

	/* get the fd.
	 * NB: Most other code will NOT use the CRX_STREAM_CAST_INTERNAL flag when casting.
	 * It is only used here so that the buffered data warning is not displayed.
	 */
	if (crx_stream_can_cast(stream, CRX_STREAM_AS_FD_FOR_SELECT | CRX_STREAM_CAST_INTERNAL) == SUCCESS) {
		crx_stream_cast(stream, CRX_STREAM_AS_FD_FOR_SELECT | CRX_STREAM_CAST_INTERNAL, (void*)fd, 0);
	} else if (crx_stream_can_cast(stream, CRX_STREAM_AS_FD | CRX_STREAM_CAST_INTERNAL) == SUCCESS) {
		crx_stream_cast(stream, CRX_STREAM_AS_FD | CRX_STREAM_CAST_INTERNAL, (void*)fd, 0);
	} else {
		crx_error_docref(NULL, E_WARNING, "Could not use stream of type '%s'",
				stream->ops->label);
		return 0;
	}
	return 1;
}
/* }}} */

/* {{{ Determine terminal device name (POSIX.1, 4.7.2) */
CRX_FUNCTION(posix_ttyname)
{
	zval *z_fd;
	char *p;
	crex_long fd = 0;
#if defined(ZTS) && defined(HAVE_TTYNAME_R) && defined(_SC_TTY_NAME_MAX)
	crex_long buflen;
#endif

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(z_fd)
	CREX_PARSE_PARAMETERS_END();

	if (C_TYPE_P(z_fd) == IS_RESOURCE) {
		if (!crx_posix_stream_get_fd(z_fd, &fd)) {
			RETURN_FALSE;
		}
	} else {
		if (!crex_parse_arg_long(z_fd, &fd, /* is_null */ NULL, /* check_null */ false, /* arg_num */ 1)) {
			crx_error_docref(NULL, E_WARNING, "Argument #1 ($file_descriptor) must be of type int|resource, %s given",
				crex_zval_value_name(z_fd));
			fd = zval_get_long(z_fd);
		}
		/* fd must fit in an int and be positive */
		if (fd < 0 || fd > INT_MAX) {
			crx_error_docref(NULL, E_WARNING, "Argument #1 ($file_descriptor) must be between 0 and %d", INT_MAX);
			RETURN_FALSE;
		}
	}
#if defined(ZTS) && defined(HAVE_TTYNAME_R) && defined(_SC_TTY_NAME_MAX)
	buflen = sysconf(_SC_TTY_NAME_MAX);
	if (buflen < 1) {
		RETURN_FALSE;
	}
	p = emalloc(buflen);

	if (ttyname_r(fd, p, buflen)) {
		POSIX_G(last_error) = errno;
		efree(p);
		RETURN_FALSE;
	}
	RETVAL_STRING(p);
	efree(p);
#else
	if (NULL == (p = ttyname(fd))) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}
	RETURN_STRING(p);
#endif
}
/* }}} */

/* {{{ Determine if filedesc is a tty (POSIX.1, 4.7.1) */
CRX_FUNCTION(posix_isatty)
{
	zval *z_fd;
	crex_long fd = 0;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(z_fd)
	CREX_PARSE_PARAMETERS_END();

	if (C_TYPE_P(z_fd) == IS_RESOURCE) {
		if (!crx_posix_stream_get_fd(z_fd, &fd)) {
			RETURN_FALSE;
		}
	} else {
		if (!crex_parse_arg_long(z_fd, &fd, /* is_null */ NULL, /* check_null */ false, /* arg_num */ 1)) {
			crx_error_docref(NULL, E_WARNING, "Argument #1 ($file_descriptor) must be of type int|resource, %s given",
				crex_zval_value_name(z_fd));
			RETURN_FALSE;
		}
	}

	/* A valid file descriptor must fit in an int and be positive */
	if (fd < 0 || fd > INT_MAX) {
		RETURN_FALSE;
	}
	if (isatty(fd)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/*
	POSIX.1, 4.8.1 sysconf()
	POSIX.1, 5.7.1 pathconf(), fpathconf()

	POSIX.1, 5.1.2 opendir(), readdir(), rewinddir(), closedir()
	POSIX.1, 5.2.1 chdir()
				already supported by CRX
 */

/* {{{ Get working directory pathname (POSIX.1, 5.2.2) */
CRX_FUNCTION(posix_getcwd)
{
	char  buffer[MAXPATHLEN];
	char *p;

	CREX_PARSE_PARAMETERS_NONE();

	p = VCWD_GETCWD(buffer, MAXPATHLEN);
	if (!p) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	RETURN_STRING(buffer);
}
/* }}} */

/*
	POSIX.1, 5.3.x open(), creat(), umask()
	POSIX.1, 5.4.1 link()
		already supported by CRX.
 */

/* {{{ Make a FIFO special file (POSIX.1, 5.4.2) */
#ifdef HAVE_MKFIFO
CRX_FUNCTION(posix_mkfifo)
{
	crex_string *path;
	crex_long mode;
	int     result;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_PATH_STR(path)
		C_PARAM_LONG(mode)
	CREX_PARSE_PARAMETERS_END();

	if (crx_check_open_basedir_ex(ZSTR_VAL(path), 0)) {
		RETURN_FALSE;
	}

	result = mkfifo(ZSTR_VAL(path), mode);
	if (result < 0) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
#endif
/* }}} */

/* {{{ Make a special or ordinary file (POSIX.1) */
#ifdef HAVE_MKNOD
CRX_FUNCTION(posix_mknod)
{
	crex_string *path;
	crex_long mode;
	crex_long major = 0, minor = 0;
	int result;
	dev_t crx_dev = 0;

	CREX_PARSE_PARAMETERS_START(2, 4)
		C_PARAM_PATH_STR(path)
		C_PARAM_LONG(mode)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(major)
		C_PARAM_LONG(minor)
	CREX_PARSE_PARAMETERS_END();

	if (crx_check_open_basedir_ex(ZSTR_VAL(path), 0)) {
		RETURN_FALSE;
	}

	if ((mode & S_IFCHR) || (mode & S_IFBLK)) {
		if (major == 0) {
			crex_argument_value_error(3, "cannot be 0 for the POSIX_S_IFCHR and POSIX_S_IFBLK modes");
			RETURN_THROWS();
		} else {
#if defined(HAVE_MAKEDEV) || defined(makedev)
			crx_dev = makedev(major, minor);
#else
			crx_error_docref(NULL, E_WARNING, "Cannot create a block or character device, creating a normal file instead");
#endif
		}
	}

	result = mknod(ZSTR_VAL(path), mode, crx_dev);
	if (result < 0) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
#endif
/* }}} */

/* Takes a pointer to posix group and a pointer to an already initialized ZVAL
 * array container and fills the array with the posix group member data. */
int crx_posix_group_to_array(struct group *g, zval *array_group) /* {{{ */
{
	zval array_members;
	int count;

	if (NULL == g)
		return 0;

	if (array_group == NULL || C_TYPE_P(array_group) != IS_ARRAY)
		return 0;

	array_init(&array_members);

	add_assoc_string(array_group, "name", g->gr_name);
	if (g->gr_passwd) {
		add_assoc_string(array_group, "passwd", g->gr_passwd);
	} else {
		add_assoc_null(array_group, "passwd");
	}
	for (count = 0;; count++) {
		/* gr_mem entries may be misaligned on macos. */
		char *gr_mem;
		memcpy(&gr_mem, &g->gr_mem[count], sizeof(char *));
		if (!gr_mem) {
			break;
		}

		add_next_index_string(&array_members, gr_mem);
	}
	crex_hash_str_update(C_ARRVAL_P(array_group), "members", sizeof("members")-1, &array_members);
	add_assoc_long(array_group, "gid", g->gr_gid);
	return 1;
}
/* }}} */

/*
	POSIX.1, 5.5.1 unlink()
	POSIX.1, 5.5.2 rmdir()
	POSIX.1, 5.5.3 rename()
	POSIX.1, 5.6.x stat(), chmod(), utime() already supported by CRX.
*/

/* {{{ Determine accessibility of a file (POSIX.1 5.6.3) */
CRX_FUNCTION(posix_access)
{
	crex_long mode = 0;
	size_t filename_len, ret;
	char *filename, *path;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_PATH(filename, filename_len)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(mode)
	CREX_PARSE_PARAMETERS_END();

	path = expand_filepath(filename, NULL);
	if (!path) {
		POSIX_G(last_error) = EIO;
		RETURN_FALSE;
	}

	if (crx_check_open_basedir_ex(path, 0)) {
		efree(path);
		POSIX_G(last_error) = EPERM;
		RETURN_FALSE;
	}

	ret = access(path, mode);
	efree(path);

	if (ret) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

#ifdef HAVE_EACCESS
CRX_FUNCTION(posix_eaccess)
{
	crex_long mode = 0;
	size_t filename_len, ret;
	char *filename, *path;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_PATH(filename, filename_len)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(mode)
	CREX_PARSE_PARAMETERS_END();

	path = expand_filepath(filename, NULL);
	if (!path) {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	}

	if (crx_check_open_basedir_ex(path, 0)) {
		efree(path);
		POSIX_G(last_error) = EPERM;
		RETURN_FALSE;
	}

	ret = eaccess(path, mode);
	efree(path);

	if (ret) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
#endif

/* }}} */

/*
	POSIX.1, 6.x most I/O functions already supported by CRX.
	POSIX.1, 7.x tty functions, TODO
	POSIX.1, 8.x interactions with other C language functions
	POSIX.1, 9.x system database access
*/

/* {{{ Group database access (POSIX.1, 9.2.1) */
CRX_FUNCTION(posix_getgrnam)
{
	char *name;
	struct group *g;
	size_t name_len;
#if defined(ZTS) && defined(HAVE_GETGRNAM_R) && defined(_SC_GETGR_R_SIZE_MAX)
	struct group gbuf;
	long buflen;
	char *buf;
#endif

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STRING(name, name_len)
	CREX_PARSE_PARAMETERS_END();

#if defined(ZTS) && defined(HAVE_GETGRNAM_R) && defined(_SC_GETGR_R_SIZE_MAX)
	buflen = sysconf(_SC_GETGR_R_SIZE_MAX);
	if (buflen < 1) {
		RETURN_FALSE;
	}
	buf = emalloc(buflen);
try_again:
	g = &gbuf;

	if (getgrnam_r(name, g, buf, buflen, &g) || g == NULL) {
		if (errno == ERANGE) {
			buflen *= 2;
			buf = erealloc(buf, buflen);
			goto try_again;
		}
		POSIX_G(last_error) = errno;
		efree(buf);
		RETURN_FALSE;
	}
#else
	if (NULL == (g = getgrnam(name))) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}
#endif
	array_init(return_value);

	if (!crx_posix_group_to_array(g, return_value)) {
		crex_array_destroy(C_ARR_P(return_value));
		crx_error_docref(NULL, E_WARNING, "Unable to convert posix group to array");
		RETVAL_FALSE;
	}
#if defined(ZTS) && defined(HAVE_GETGRNAM_R) && defined(_SC_GETGR_R_SIZE_MAX)
	efree(buf);
#endif
}
/* }}} */

/* {{{ Group database access (POSIX.1, 9.2.1) */
CRX_FUNCTION(posix_getgrgid)
{
	crex_long gid;
#if defined(ZTS) && defined(HAVE_GETGRGID_R) && defined(_SC_GETGR_R_SIZE_MAX)
	int ret;
	struct group _g;
	struct group *retgrptr = NULL;
	long grbuflen;
	char *grbuf;
#endif
	struct group *g;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(gid)
	CREX_PARSE_PARAMETERS_END();

#if defined(ZTS) && defined(HAVE_GETGRGID_R) && defined(_SC_GETGR_R_SIZE_MAX)

	grbuflen = sysconf(_SC_GETGR_R_SIZE_MAX);
	if (grbuflen < 1) {
		RETURN_FALSE;
	}

	grbuf = emalloc(grbuflen);

try_again:
	ret = getgrgid_r(gid, &_g, grbuf, grbuflen, &retgrptr);
	if (ret || retgrptr == NULL) {
		if (errno == ERANGE) {
			grbuflen *= 2;
			grbuf = erealloc(grbuf, grbuflen);
			goto try_again;
		}
		POSIX_G(last_error) = ret;
		efree(grbuf);
		RETURN_FALSE;
	}
	g = &_g;
#else
	if (NULL == (g = getgrgid(gid))) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}
#endif
	array_init(return_value);

	if (!crx_posix_group_to_array(g, return_value)) {
		crex_array_destroy(C_ARR_P(return_value));
		crx_error_docref(NULL, E_WARNING, "Unable to convert posix group struct to array");
		RETVAL_FALSE;
	}
#if defined(ZTS) && defined(HAVE_GETGRGID_R) && defined(_SC_GETGR_R_SIZE_MAX)
	efree(grbuf);
#endif
}
/* }}} */

int crx_posix_passwd_to_array(struct passwd *pw, zval *return_value) /* {{{ */
{
	if (NULL == pw)
		return 0;
	if (NULL == return_value || C_TYPE_P(return_value) != IS_ARRAY)
		return 0;

	add_assoc_string(return_value, "name",      pw->pw_name);
	add_assoc_string(return_value, "passwd",    pw->pw_passwd);
	add_assoc_long  (return_value, "uid",       pw->pw_uid);
	add_assoc_long  (return_value, "gid",		pw->pw_gid);
	add_assoc_string(return_value, "gecos",     pw->pw_gecos);
	add_assoc_string(return_value, "dir",       pw->pw_dir);
	add_assoc_string(return_value, "shell",     pw->pw_shell);
	return 1;
}
/* }}} */

/* {{{ User database access (POSIX.1, 9.2.2) */
CRX_FUNCTION(posix_getpwnam)
{
	struct passwd *pw;
	char *name;
	size_t name_len;
#if defined(ZTS) && defined(_SC_GETPW_R_SIZE_MAX) && defined(HAVE_GETPWNAM_R)
	struct passwd pwbuf;
	long buflen;
	char *buf;
#endif

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STRING(name, name_len)
	CREX_PARSE_PARAMETERS_END();

#if defined(ZTS) && defined(_SC_GETPW_R_SIZE_MAX) && defined(HAVE_GETPWNAM_R)
	buflen = sysconf(_SC_GETPW_R_SIZE_MAX);
	if (buflen < 1) {
		RETURN_FALSE;
	}
	buf = emalloc(buflen);
	pw = &pwbuf;

try_again:
	if (getpwnam_r(name, pw, buf, buflen, &pw) || pw == NULL) {
		if (errno == ERANGE) {
			buflen *= 2;
			buf = erealloc(buf, buflen);
			goto try_again;
		}
		efree(buf);
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}
#else
	if (NULL == (pw = getpwnam(name))) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}
#endif
	array_init(return_value);

	if (!crx_posix_passwd_to_array(pw, return_value)) {
		crex_array_destroy(C_ARR_P(return_value));
		crx_error_docref(NULL, E_WARNING, "Unable to convert posix passwd struct to array");
		RETVAL_FALSE;
	}
#if defined(ZTS) && defined(_SC_GETPW_R_SIZE_MAX) && defined(HAVE_GETPWNAM_R)
	efree(buf);
#endif
}
/* }}} */

/* {{{ User database access (POSIX.1, 9.2.2) */
CRX_FUNCTION(posix_getpwuid)
{
	crex_long uid;
#if defined(ZTS) && defined(_SC_GETPW_R_SIZE_MAX) && defined(HAVE_GETPWUID_R)
	struct passwd _pw;
	struct passwd *retpwptr = NULL;
	long pwbuflen;
	char *pwbuf;
	int ret;
#endif
	struct passwd *pw;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(uid)
	CREX_PARSE_PARAMETERS_END();

#if defined(ZTS) && defined(_SC_GETPW_R_SIZE_MAX) && defined(HAVE_GETPWUID_R)
	pwbuflen = sysconf(_SC_GETPW_R_SIZE_MAX);
	if (pwbuflen < 1) {
		RETURN_FALSE;
	}
	pwbuf = emalloc(pwbuflen);

try_again:
	ret = getpwuid_r(uid, &_pw, pwbuf, pwbuflen, &retpwptr);
	if (ret || retpwptr == NULL) {
		if (errno == ERANGE) {
			pwbuflen *= 2;
			pwbuf = erealloc(pwbuf, pwbuflen);
			goto try_again;
		}
		POSIX_G(last_error) = ret;
		efree(pwbuf);
		RETURN_FALSE;
	}
	pw = &_pw;
#else
	if (NULL == (pw = getpwuid(uid))) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}
#endif
	array_init(return_value);

	if (!crx_posix_passwd_to_array(pw, return_value)) {
		crex_array_destroy(C_ARR_P(return_value));
		crx_error_docref(NULL, E_WARNING, "Unable to convert posix passwd struct to array");
		RETVAL_FALSE;
	}
#if defined(ZTS) && defined(_SC_GETPW_R_SIZE_MAX) && defined(HAVE_GETPWUID_R)
	efree(pwbuf);
#endif
}
/* }}} */


#ifdef HAVE_GETRLIMIT

#define UNLIMITED_STRING "unlimited"

/* {{{ posix_addlimit */
static int posix_addlimit(int limit, const char *name, zval *return_value) {
	int result;
	struct rlimit rl;
	char hard[80];
	char soft[80];

	snprintf(hard, 80, "hard %s", name);
	snprintf(soft, 80, "soft %s", name);

	result = getrlimit(limit, &rl);
	if (result < 0) {
		POSIX_G(last_error) = errno;
		return FAILURE;
	}

	if (rl.rlim_cur == RLIM_INFINITY) {
		add_assoc_stringl(return_value, soft, UNLIMITED_STRING, sizeof(UNLIMITED_STRING)-1);
	} else {
		add_assoc_long(return_value, soft, rl.rlim_cur);
	}

	if (rl.rlim_max == RLIM_INFINITY) {
		add_assoc_stringl(return_value, hard, UNLIMITED_STRING, sizeof(UNLIMITED_STRING)-1);
	} else {
		add_assoc_long(return_value, hard, rl.rlim_max);
	}

	return SUCCESS;
}
/* }}} */

/* {{{ limits[] */
static const struct limitlist {
	int limit;
	const char *name;
} limits[] = {
#ifdef RLIMIT_CORE
	{ RLIMIT_CORE,	"core" },
#endif

#ifdef RLIMIT_DATA
	{ RLIMIT_DATA,	"data" },
#endif

#ifdef RLIMIT_STACK
	{ RLIMIT_STACK,	"stack" },
#endif

#ifdef RLIMIT_VMEM
	{ RLIMIT_VMEM, "virtualmem" },
#endif

#ifdef RLIMIT_AS
	{ RLIMIT_AS, "totalmem" },
#endif

#ifdef RLIMIT_RSS
	{ RLIMIT_RSS, "rss" },
#endif

#ifdef RLIMIT_NPROC
	{ RLIMIT_NPROC, "maxproc" },
#endif

#ifdef RLIMIT_MEMLOCK
	{ RLIMIT_MEMLOCK, "memlock" },
#endif

#ifdef RLIMIT_CPU
	{ RLIMIT_CPU,	"cpu" },
#endif

#ifdef RLIMIT_FSIZE
	{ RLIMIT_FSIZE, "filesize" },
#endif

#ifdef RLIMIT_NOFILE
	{ RLIMIT_NOFILE, "openfiles" },
#endif

#ifdef RLIMIT_OFILE
	{ RLIMIT_OFILE, "openfiles" },
#endif

#ifdef RLIMIT_KQUEUES
	{ RLIMIT_KQUEUES, "kqueues" },
#endif

#ifdef RLIMIT_NPTS
	{ RLIMIT_NPTS, "npts" },
#endif

	{ 0, NULL }
};
/* }}} */


/* {{{ Get system resource consumption limits (This is not a POSIX function, but a BSDism and a SVR4ism. We compile conditionally) */
CRX_FUNCTION(posix_getrlimit)
{
	const struct limitlist *l = NULL;
	crex_long res;
	bool res_is_null = true;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_LONG_OR_NULL(res, res_is_null)
	CREX_PARSE_PARAMETERS_END();

	if (res_is_null) {
		array_init(return_value);

		for (l=limits; l->name; l++) {
			if (posix_addlimit(l->limit, l->name, return_value) == FAILURE) {
				crex_array_destroy(C_ARR_P(return_value));
				RETURN_FALSE;
			}
		}
	} else {
		struct rlimit rl;
		int result = getrlimit(res, &rl);
		if (result < 0) {
			POSIX_G(last_error) = errno;
			RETURN_FALSE;
		}

		array_init(return_value);
		if (rl.rlim_cur == RLIM_INFINITY) {
			add_next_index_stringl(return_value, UNLIMITED_STRING, sizeof(UNLIMITED_STRING)-1);
		} else {
			add_next_index_long(return_value, rl.rlim_cur);
		}

		if (rl.rlim_max == RLIM_INFINITY) {
			add_next_index_stringl(return_value, UNLIMITED_STRING, sizeof(UNLIMITED_STRING)-1);
		} else {
			add_next_index_long(return_value, rl.rlim_max);
		}
	}
}
/* }}} */

#endif /* HAVE_GETRLIMIT */

#ifdef HAVE_SETRLIMIT
/* {{{ Set system resource consumption limits (POSIX.1-2001) */
CRX_FUNCTION(posix_setrlimit)
{
	struct rlimit rl;
	crex_long res, cur, max;

	CREX_PARSE_PARAMETERS_START(3, 3)
		C_PARAM_LONG(res)
		C_PARAM_LONG(cur)
		C_PARAM_LONG(max)
	CREX_PARSE_PARAMETERS_END();

	rl.rlim_cur = cur;
	rl.rlim_max = max;

	if (setrlimit(res, &rl) == -1) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

#endif /* HAVE_SETRLIMIT */


/* {{{ Retrieve the error number set by the last posix function which failed. */
CRX_FUNCTION(posix_get_last_error)
{
	CREX_PARSE_PARAMETERS_NONE();

	RETURN_LONG(POSIX_G(last_error));
}
/* }}} */

/* {{{ Retrieve the system error message associated with the given errno. */
CRX_FUNCTION(posix_strerror)
{
	crex_long error;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(error)
	CREX_PARSE_PARAMETERS_END();

	RETURN_STRING(strerror(error));
}
/* }}} */

#endif

#ifdef HAVE_INITGROUPS
/* {{{ Calculate the group access list for the user specified in name. */
CRX_FUNCTION(posix_initgroups)
{
	crex_long basegid;
	char *name;
	size_t name_len;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_STRING(name, name_len)
		C_PARAM_LONG(basegid)
	CREX_PARSE_PARAMETERS_END();

	if (name_len == 0) {
		RETURN_FALSE;
	}

	RETURN_BOOL(!initgroups((const char *)name, basegid));
}
/* }}} */
#endif

CRX_FUNCTION(posix_sysconf)
{
	crex_long conf_id;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(conf_id)
	CREX_PARSE_PARAMETERS_END();

	RETURN_LONG(sysconf(conf_id));
}

#ifdef HAVE_PATHCONF
CRX_FUNCTION(posix_pathconf)
{
	crex_long name, ret;
	char *path;
	size_t path_len;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_PATH(path, path_len)
		C_PARAM_LONG(name);
	CREX_PARSE_PARAMETERS_END();

	if (path_len == 0) {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	} else if (crx_check_open_basedir(path)) {
		crx_error_docref(NULL, E_WARNING, "Invalid path supplied: %s", path);
		RETURN_FALSE;
	}

	ret = pathconf(path, name);

	if (ret < 0 && errno != 0) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	RETURN_LONG(ret);
}
#endif

#ifdef HAVE_FPATHCONF
CRX_FUNCTION(posix_fpathconf)
{
	crex_long name, ret, fd = 0;
	zval *z_fd;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_ZVAL(z_fd)
		C_PARAM_LONG(name);
	CREX_PARSE_PARAMETERS_END();

	if (C_TYPE_P(z_fd) == IS_RESOURCE) {
		if (!crx_posix_stream_get_fd(z_fd, &fd)) {
			RETURN_FALSE;
		}
	} else {
		if (!crex_parse_arg_long(z_fd, &fd, /* is_null */ NULL, /* check_null */ false, /* arg_num */ 1)) {
			crex_argument_type_error(1, "must be of type int|resource, %s given",
				crex_zval_value_name(z_fd));
			RETURN_THROWS();
		}
	}

	ret = fpathconf(fd, name);

	if (ret < 0 && errno != 0) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	RETURN_LONG(ret);
}
#endif
