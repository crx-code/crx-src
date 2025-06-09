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
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   +----------------------------------------------------------------------+
 */

#ifndef CRX_H
#define CRX_H

#ifdef HAVE_DMALLOC
#include <dmalloc.h>
#endif

#define CRX_API_VERSION 20230831
#define CRX_HAVE_STREAMS
#define YYDEBUG 0
#define CRX_DEFAULT_CHARSET "UTF-8"

#include "crx_version.h"
#include "crex.h"
#include "crex_sort.h"
#include "crx_compat.h"

#include "crex_API.h"

#define crx_sprintf sprintf

/* Operating system family definition */
#ifdef CRX_WIN32
# define CRX_OS_FAMILY			"Windows"
#elif defined(BSD) || defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
# define CRX_OS_FAMILY			"BSD"
#elif defined(__APPLE__) || defined(__MACH__)
# define CRX_OS_FAMILY			"Darwin"
#elif defined(__sun__)
# define CRX_OS_FAMILY			"Solaris"
#elif defined(__linux__)
# define CRX_OS_FAMILY			"Linux"
#else
# define CRX_OS_FAMILY			"Unknown"
#endif

/* CRX's DEBUG value must match Crex's CREX_DEBUG value */
#undef CRX_DEBUG
#define CRX_DEBUG CREX_DEBUG

#ifdef CRX_WIN32
#	include "tsrm_win32.h"
#	ifdef CRX_EXPORTS
#		define CRXAPI __declspec(dllexport)
#	else
#		define CRXAPI __declspec(dllimport)
#	endif
#	define CRX_DIR_SEPARATOR '\\'
#	define CRX_EOL "\r\n"
#else
#	if defined(__GNUC__) && __GNUC__ >= 4
#		define CRXAPI __attribute__ ((visibility("default")))
#	else
#		define CRXAPI
#	endif
#	define CRX_DIR_SEPARATOR '/'
#	define CRX_EOL "\n"
#endif

/* Windows specific defines */
#ifdef CRX_WIN32
# define CRX_PROG_SENDMAIL		"Built in mailer"
# define WIN32_LEAN_AND_MEAN
# define NOOPENFILE

# include <io.h>
# include <malloc.h>
# include <direct.h>
# include <stdlib.h>
# include <stdio.h>
# include <stdarg.h>
# include <sys/types.h>
# include <process.h>

typedef int uid_t;
typedef int gid_t;
typedef char * caddr_t;
typedef int pid_t;

# define M_TWOPI        (M_PI * 2.0)
# define off_t			_off_t

# define lstat(x, y)	crx_sys_lstat(x, y)
# define chdir(path)	_chdir(path)
# define mkdir(a, b)	_mkdir(a)
# define rmdir(a)		_rmdir(a)
# define getpid			_getpid
# define crx_sleep(t)	SleepEx(t*1000, TRUE)

# ifndef getcwd
#  define getcwd(a, b)	_getcwd(a, b)
# endif
#endif

#if CRX_DEBUG
#undef NDEBUG
#else
#ifndef NDEBUG
#define NDEBUG
#endif
#endif
#include <assert.h>

#ifdef HAVE_UNIX_H
#include <unix.h>
#endif

#if HAVE_ALLOCA_H
#include <alloca.h>
#endif

#if HAVE_BUILD_DEFS_H
#include <build-defs.h>
#endif

/*
 * This is a fast version of strlcpy which should be used, if you
 * know the size of the destination buffer and if you know
 * the length of the source string.
 *
 * size is the allocated number of bytes of dst
 * src_size is the number of bytes excluding the NUL of src
 */

#define CRX_STRLCPY(dst, src, size, src_size)	\
	{											\
		size_t crx_str_len;						\
												\
		if (src_size >= size)					\
			crx_str_len = size - 1;				\
		else									\
			crx_str_len = src_size;				\
		memcpy(dst, src, crx_str_len);			\
		dst[crx_str_len] = '\0';				\
	}

#ifndef HAVE_STRLCPY
BEGIN_EXTERN_C()
CRXAPI size_t crx_strlcpy(char *dst, const char *src, size_t siz);
END_EXTERN_C()
#undef strlcpy
#define strlcpy crx_strlcpy
#define HAVE_STRLCPY 1
#define USE_STRLCPY_CRX_IMPL 1
#endif

#ifndef HAVE_STRLCAT
BEGIN_EXTERN_C()
CRXAPI size_t crx_strlcat(char *dst, const char *src, size_t siz);
END_EXTERN_C()
#undef strlcat
#define strlcat crx_strlcat
#define HAVE_STRLCAT 1
#define USE_STRLCAT_CRX_IMPL 1
#endif

#ifndef HAVE_EXPLICIT_BZERO
BEGIN_EXTERN_C()
CRXAPI void crx_explicit_bzero(void *dst, size_t siz);
END_EXTERN_C()
#undef explicit_bzero
#define explicit_bzero crx_explicit_bzero
#endif

BEGIN_EXTERN_C()
CRXAPI int crx_safe_bcmp(const crex_string *a, const crex_string *b);
END_EXTERN_C()

#ifndef HAVE_STRTOK_R
BEGIN_EXTERN_C()
char *strtok_r(char *s, const char *delim, char **last);
END_EXTERN_C()
#endif

#ifndef HAVE_SOCKLEN_T
# ifdef CRX_WIN32
typedef int socklen_t;
# else
typedef unsigned int socklen_t;
# endif
#endif

#define CREATE_MUTEX(a, b)
#define SET_MUTEX(a)
#define FREE_MUTEX(a)

/*
 * Then the ODBC support can use both iodbc and Solid,
 * uncomment this.
 * #define HAVE_ODBC (HAVE_IODBC|HAVE_SOLID)
 */

#include <stdlib.h>
#include <ctype.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdarg.h>

#include "crex_hash.h"
#include "crex_alloc.h"
#include "crex_stack.h"
#include <string.h>

#if HAVE_PWD_H
# ifdef CRX_WIN32
#include "win32/param.h"
# else
#include <pwd.h>
#include <sys/param.h>
# endif
#endif

#include <limits.h>

#ifndef LONG_MAX
#define LONG_MAX 2147483647L
#endif

#ifndef LONG_MIN
#define LONG_MIN (- LONG_MAX - 1)
#endif

#ifndef INT_MAX
#define INT_MAX 2147483647
#endif

#ifndef INT_MIN
#define INT_MIN (- INT_MAX - 1)
#endif

#define CRX_DOUBLE_MAX_LENGTH CREX_DOUBLE_MAX_LENGTH

#define CRX_GCC_VERSION CREX_GCC_VERSION
#define CRX_ATTRIBUTE_MALLOC CREX_ATTRIBUTE_MALLOC
#define CRX_ATTRIBUTE_FORMAT CREX_ATTRIBUTE_FORMAT

BEGIN_EXTERN_C()
#include "snprintf.h"
END_EXTERN_C()
#include "spprintf.h"

#define EXEC_INPUT_BUF 4096

#define CRX_MIME_TYPE "application/x-httpd-crx"

/* macros */
#define STR_PRINT(str)	((str)?(str):"")

#ifndef MAXPATHLEN
# ifdef CRX_WIN32
#  include "win32/ioutil.h"
#  define MAXPATHLEN CRX_WIN32_IOUTIL_MAXPATHLEN
# elif PATH_MAX
#  define MAXPATHLEN PATH_MAX
# elif defined(MAX_PATH)
#  define MAXPATHLEN MAX_PATH
# else
#  define MAXPATHLEN 256    /* Should be safe for any weird systems that do not define it */
# endif
#endif

#define crx_ignore_value(x) CREX_IGNORE_VALUE(x)

/* global variables */
#ifndef CRX_WIN32
#define crx_sleep sleep
extern char **environ;
#endif	/* ifndef CRX_WIN32 */

#ifdef CRX_PWRITE_64
ssize_t pwrite(int, void *, size_t, off64_t);
#endif

#ifdef CRX_PREAD_64
ssize_t pread(int, void *, size_t, off64_t);
#endif

BEGIN_EXTERN_C()
void crxerror(char *error);
CRXAPI size_t crx_write(void *buf, size_t size);
CRXAPI size_t crx_printf(const char *format, ...) CRX_ATTRIBUTE_FORMAT(printf, 1, 2);
CRXAPI size_t crx_printf_unchecked(const char *format, ...);
CRXAPI bool crx_during_module_startup(void);
CRXAPI bool crx_during_module_shutdown(void);
CRXAPI bool crx_get_module_initialized(void);
#ifdef HAVE_SYSLOG_H
#include "crx_syslog.h"
#define crx_log_err(msg) crx_log_err_with_severity(msg, LOG_NOTICE)
#else
#define crx_log_err(msg) crx_log_err_with_severity(msg, 5)
#endif
CRXAPI CREX_COLD void crx_log_err_with_severity(const char *log_message, int syslog_type_int);
int Debug(char *format, ...) CRX_ATTRIBUTE_FORMAT(printf, 1, 2);
int cfgparse(void);
END_EXTERN_C()

#define crx_error crex_error
#define error_handling_t crex_error_handling_t

BEGIN_EXTERN_C()
static inline CREX_ATTRIBUTE_DEPRECATED void crx_set_error_handling(error_handling_t error_handling, crex_class_entry *exception_class)
{
	crex_replace_error_handling(error_handling, exception_class, NULL);
}
static inline CREX_ATTRIBUTE_DEPRECATED void crx_std_error_handling(void) {}

CRXAPI CREX_COLD void crx_verror(const char *docref, const char *params, int type, const char *format, va_list args) CRX_ATTRIBUTE_FORMAT(printf, 4, 0);

/* CRXAPI void crx_error(int type, const char *format, ...); */
CRXAPI CREX_COLD void crx_error_docref(const char *docref, int type, const char *format, ...)
	CRX_ATTRIBUTE_FORMAT(printf, 3, 4);
CRXAPI CREX_COLD void crx_error_docref1(const char *docref, const char *param1, int type, const char *format, ...)
	CRX_ATTRIBUTE_FORMAT(printf, 4, 5);
CRXAPI CREX_COLD void crx_error_docref2(const char *docref, const char *param1, const char *param2, int type, const char *format, ...)
	CRX_ATTRIBUTE_FORMAT(printf, 5, 6);
#ifdef CRX_WIN32
CRXAPI CREX_COLD void crx_win32_docref1_from_error(DWORD error, const char *param1);
CRXAPI CREX_COLD void crx_win32_docref2_from_error(DWORD error, const char *param1, const char *param2);
#endif
END_EXTERN_C()

#define crexerror crxerror
#define crexlex crxlex

#define crxparse crexparse
#define crxrestart crexrestart
#define crxin crexin

#define crx_memnstr crex_memnstr
#define crx_memnistr crex_memnistr

/* functions */
BEGIN_EXTERN_C()
CRXAPI extern int (*crx_register_internal_extensions_func)(void);
CRXAPI int crx_register_internal_extensions(void);
CRXAPI void crx_register_pre_request_shutdown(void (*func)(void *), void *userdata);
CRXAPI void crx_com_initialize(void);
CRXAPI char *crx_get_current_user(void);

CRXAPI const char *crx_get_internal_encoding(void);
CRXAPI const char *crx_get_input_encoding(void);
CRXAPI const char *crx_get_output_encoding(void);
CRXAPI extern void (*crx_internal_encoding_changed)(void);
END_EXTERN_C()

/* CRX-named Crex macro wrappers */
#define CRX_FN					CREX_FN
#define CRX_MN					CREX_MN
#define CRX_NAMED_FUNCTION		CREX_NAMED_FUNCTION
#define CRX_FUNCTION			CREX_FUNCTION
#define CRX_METHOD  			CREX_METHOD

#define CRX_RAW_NAMED_FE CREX_RAW_NAMED_FE
#define CRX_NAMED_FE	CREX_NAMED_FE
#define CRX_FE			CREX_FE
#define CRX_DEP_FE      CREX_DEP_FE
#define CRX_FALIAS		CREX_FALIAS
#define CRX_DEP_FALIAS	CREX_DEP_FALIAS
#define CRX_ME          CREX_ME
#define CRX_MALIAS      CREX_MALIAS
#define CRX_ABSTRACT_ME CREX_ABSTRACT_ME
#define CRX_ME_MAPPING  CREX_ME_MAPPING
#define CRX_FE_END      CREX_FE_END

#define CRX_MODULE_STARTUP_N	CREX_MODULE_STARTUP_N
#define CRX_MODULE_SHUTDOWN_N	CREX_MODULE_SHUTDOWN_N
#define CRX_MODULE_ACTIVATE_N	CREX_MODULE_ACTIVATE_N
#define CRX_MODULE_DEACTIVATE_N	CREX_MODULE_DEACTIVATE_N
#define CRX_MODULE_INFO_N		CREX_MODULE_INFO_N

#define CRX_MODULE_STARTUP_D	CREX_MODULE_STARTUP_D
#define CRX_MODULE_SHUTDOWN_D	CREX_MODULE_SHUTDOWN_D
#define CRX_MODULE_ACTIVATE_D	CREX_MODULE_ACTIVATE_D
#define CRX_MODULE_DEACTIVATE_D	CREX_MODULE_DEACTIVATE_D
#define CRX_MODULE_INFO_D		CREX_MODULE_INFO_D

/* Compatibility macros */
#define CRX_MINIT		CREX_MODULE_STARTUP_N
#define CRX_MSHUTDOWN	CREX_MODULE_SHUTDOWN_N
#define CRX_RINIT		CREX_MODULE_ACTIVATE_N
#define CRX_RSHUTDOWN	CREX_MODULE_DEACTIVATE_N
#define CRX_MINFO		CREX_MODULE_INFO_N
#define CRX_GINIT		CREX_GINIT
#define CRX_GSHUTDOWN	CREX_GSHUTDOWN

#define CRX_MINIT_FUNCTION		CREX_MODULE_STARTUP_D
#define CRX_MSHUTDOWN_FUNCTION	CREX_MODULE_SHUTDOWN_D
#define CRX_RINIT_FUNCTION		CREX_MODULE_ACTIVATE_D
#define CRX_RSHUTDOWN_FUNCTION	CREX_MODULE_DEACTIVATE_D
#define CRX_MINFO_FUNCTION		CREX_MODULE_INFO_D
#define CRX_GINIT_FUNCTION		CREX_GINIT_FUNCTION
#define CRX_GSHUTDOWN_FUNCTION	CREX_GSHUTDOWN_FUNCTION

#define CRX_MODULE_GLOBALS		CREX_MODULE_GLOBALS


/* Output support */
#include "main/crx_output.h"


#include "crx_streams.h"
#include "crx_memory_streams.h"
#include "fopen_wrappers.h"


/* Virtual current working directory support */
#include "crex_virtual_cwd.h"

#include "crex_constants.h"

/* connection status states */
#define CRX_CONNECTION_NORMAL  0
#define CRX_CONNECTION_ABORTED 1
#define CRX_CONNECTION_TIMEOUT 2

#include "crx_reentrancy.h"

/* the following typedefs are deprecated and will be removed in CRX
 * 9.0; use the standard C99 types instead */
typedef bool crex_bool;
typedef intptr_t crex_intptr_t;
typedef uintptr_t crex_uintptr_t;

#endif
