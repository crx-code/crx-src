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

#include "crx.h"
#include "crx_assert.h"
#include "crx_crypt.h"
#include "crx_streams.h"
#include "crx_main.h"
#include "crx_globals.h"
#include "crx_variables.h"
#include "crx_ini.h"
#include "crx_image.h"
#include "crx_standard.h"
#include "crx_math.h"
#include "crx_http.h"
#include "crx_incomplete_class.h"
#include "crx_getopt.h"
#include "crx_ext_syslog.h"
#include "ext/standard/info.h"
#include "ext/session/crx_session.h"
#include "crex_exceptions.h"
#include "crex_attributes.h"
#include "crex_ini.h"
#include "crex_operators.h"
#include "ext/standard/crx_dns.h"
#include "ext/standard/crx_uuencode.h"
#include "ext/standard/crc32_x86.h"

#ifdef CRX_WIN32
#include "win32/crx_win32_globals.h"
#include "win32/time.h"
#include "win32/ioutil.h"
#endif

typedef struct yy_buffer_state *YY_BUFFER_STATE;

#include "crex.h"
#include "crex_ini_scanner.h"
#include "crex_language_scanner.h"
#include <crex_language_parser.h>

#include "crex_portability.h"

#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

#ifndef CRX_WIN32
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifndef CRX_WIN32
# include <netdb.h>
#else
#include "win32/inet.h"
#endif

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <string.h>
#include <locale.h>
#ifdef HAVE_LANGINFO_H
# include <langinfo.h>
#endif

#ifdef HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif

#ifdef HAVE_SYS_LOADAVG_H
# include <sys/loadavg.h>
#endif

#ifdef CRX_WIN32
# include "win32/unistd.h"
#endif

#ifndef INADDR_NONE
# define INADDR_NONE ((crex_ulong) -1)
#endif

#include "crex_globals.h"
#include "crx_globals.h"
#include "SAPI.h"
#include "crx_ticks.h"

#ifdef ZTS
CRXAPI int basic_globals_id;
#else
CRXAPI crx_basic_globals basic_globals;
#endif

#include "crx_fopen_wrappers.h"
#include "streamsfuncs.h"
#include "basic_functions_arginfo.h"

typedef struct _user_tick_function_entry {
	crex_fcall_info fci;
	crex_fcall_info_cache fci_cache;
	bool calling;
} user_tick_function_entry;

#if HAVE_PUTENV
typedef struct {
	char *putenv_string;
	char *previous_value;
	crex_string *key;
} putenv_entry;
#endif

/* some prototypes for local functions */
static void user_shutdown_function_dtor(zval *zv);
static void user_tick_function_dtor(user_tick_function_entry *tick_function_entry);

static const crex_module_dep standard_deps[] = { /* {{{ */
	CREX_MOD_OPTIONAL("session")
	CREX_MOD_END
};
/* }}} */

crex_module_entry basic_functions_module = { /* {{{ */
	STANDARD_MODULE_HEADER_EX,
	NULL,
	standard_deps,
	"standard",					/* extension name */
	ext_functions,				/* function list */
	CRX_MINIT(basic),			/* process startup */
	CRX_MSHUTDOWN(basic),		/* process shutdown */
	CRX_RINIT(basic),			/* request startup */
	CRX_RSHUTDOWN(basic),		/* request shutdown */
	CRX_MINFO(basic),			/* extension info */
	CRX_STANDARD_VERSION,		/* extension version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef HAVE_PUTENV
static void crx_putenv_destructor(zval *zv) /* {{{ */
{
	putenv_entry *pe = C_PTR_P(zv);

	if (pe->previous_value) {
# ifdef CRX_WIN32
		/* MSVCRT has a bug in putenv() when setting a variable that
		 * is already set; if the SetEnvironmentVariable() API call
		 * fails, the Crt will double free() a string.
		 * We try to avoid this by setting our own value first */
		SetEnvironmentVariable(ZSTR_VAL(pe->key), "bugbug");
# endif
		putenv(pe->previous_value);
# ifdef CRX_WIN32
		efree(pe->previous_value);
# endif
	} else {
# ifdef HAVE_UNSETENV
		unsetenv(ZSTR_VAL(pe->key));
# elif defined(CRX_WIN32)
		SetEnvironmentVariable(ZSTR_VAL(pe->key), NULL);
# ifndef ZTS
		_putenv_s(ZSTR_VAL(pe->key), "");
# endif
# else
		char **env;

		for (env = environ; env != NULL && *env != NULL; env++) {
			if (!strncmp(*env, ZSTR_VAL(pe->key), ZSTR_LEN(pe->key))
					&& (*env)[ZSTR_LEN(pe->key)] == '=') {	/* found it */
				*env = "";
				break;
			}
		}
# endif
	}
#ifdef HAVE_TZSET
	/* don't forget to reset the various libc globals that
	 * we might have changed by an earlier call to tzset(). */
	if (crex_string_equals_literal_ci(pe->key, "TZ")) {
		tzset();
	}
#endif

	free(pe->putenv_string);
	crex_string_release(pe->key);
	efree(pe);
}
/* }}} */
#endif

static void basic_globals_ctor(crx_basic_globals *basic_globals_p) /* {{{ */
{
	BG(umask) = -1;
	BG(user_tick_functions) = NULL;
	BG(user_filter_map) = NULL;
	BG(serialize_lock) = 0;

	memset(&BG(serialize), 0, sizeof(BG(serialize)));
	memset(&BG(unserialize), 0, sizeof(BG(unserialize)));

	memset(&BG(url_adapt_session_ex), 0, sizeof(BG(url_adapt_session_ex)));
	memset(&BG(url_adapt_output_ex), 0, sizeof(BG(url_adapt_output_ex)));

	BG(url_adapt_session_ex).type = 1;
	BG(url_adapt_output_ex).type  = 0;

	crex_hash_init(&BG(url_adapt_session_hosts_ht), 0, NULL, NULL, 1);
	crex_hash_init(&BG(url_adapt_output_hosts_ht), 0, NULL, NULL, 1);

#if defined(_REENTRANT)
	memset(&BG(mblen_state), 0, sizeof(BG(mblen_state)));
#endif

	BG(page_uid) = -1;
	BG(page_gid) = -1;

	BG(syslog_device) = NULL;
}
/* }}} */

static void basic_globals_dtor(crx_basic_globals *basic_globals_p) /* {{{ */
{
	if (basic_globals_p->url_adapt_session_ex.tags) {
		crex_hash_destroy(basic_globals_p->url_adapt_session_ex.tags);
		free(basic_globals_p->url_adapt_session_ex.tags);
	}
	if (basic_globals_p->url_adapt_output_ex.tags) {
		crex_hash_destroy(basic_globals_p->url_adapt_output_ex.tags);
		free(basic_globals_p->url_adapt_output_ex.tags);
	}

	crex_hash_destroy(&basic_globals_p->url_adapt_session_hosts_ht);
	crex_hash_destroy(&basic_globals_p->url_adapt_output_hosts_ht);
}
/* }}} */

CRXAPI double crx_get_nan(void) /* {{{ */
{
	return CREX_NAN;
}
/* }}} */

CRXAPI double crx_get_inf(void) /* {{{ */
{
	return CREX_INFINITY;
}
/* }}} */

#define BASIC_MINIT_SUBMODULE(module) \
	if (CRX_MINIT(module)(INIT_FUNC_ARGS_PASSTHRU) != SUCCESS) {\
		return FAILURE; \
	}

#define BASIC_RINIT_SUBMODULE(module) \
	CRX_RINIT(module)(INIT_FUNC_ARGS_PASSTHRU);

#define BASIC_MINFO_SUBMODULE(module) \
	CRX_MINFO(module)(CREX_MODULE_INFO_FUNC_ARGS_PASSTHRU);

#define BASIC_RSHUTDOWN_SUBMODULE(module) \
	CRX_RSHUTDOWN(module)(SHUTDOWN_FUNC_ARGS_PASSTHRU);

#define BASIC_MSHUTDOWN_SUBMODULE(module) \
	CRX_MSHUTDOWN(module)(SHUTDOWN_FUNC_ARGS_PASSTHRU);

CRX_MINIT_FUNCTION(basic) /* {{{ */
{
#ifdef ZTS
	ts_allocate_id(&basic_globals_id, sizeof(crx_basic_globals), (ts_allocate_ctor) basic_globals_ctor, (ts_allocate_dtor) basic_globals_dtor);
# ifdef CRX_WIN32
	ts_allocate_id(&crx_win32_core_globals_id, sizeof(crx_win32_core_globals), (ts_allocate_ctor)crx_win32_core_globals_ctor, (ts_allocate_dtor)crx_win32_core_globals_dtor );
# endif
#else
	basic_globals_ctor(&basic_globals);
# ifdef CRX_WIN32
	crx_win32_core_globals_ctor(&the_crx_win32_core_globals);
# endif
#endif

	register_basic_functions_symbols(module_number);

	crx_ce_incomplete_class = register_class___CRX_Incomplete_Class();
	crx_register_incomplete_class_handlers();

	assertion_error_ce = register_class_AssertionError(crex_ce_error);

	BASIC_MINIT_SUBMODULE(var)
	BASIC_MINIT_SUBMODULE(file)
	BASIC_MINIT_SUBMODULE(pack)
	BASIC_MINIT_SUBMODULE(browscap)
	BASIC_MINIT_SUBMODULE(standard_filters)
	BASIC_MINIT_SUBMODULE(user_filters)
	BASIC_MINIT_SUBMODULE(password)

#ifdef ZTS
	BASIC_MINIT_SUBMODULE(localeconv)
#endif

#ifdef CREX_INTRIN_SSE4_2_FUNC_PTR
	BASIC_MINIT_SUBMODULE(string_intrin)
#endif

#ifdef CREX_INTRIN_SSE4_2_PCLMUL_FUNC_PTR
	BASIC_MINIT_SUBMODULE(crc32_x86_intrin)
#endif

#if defined(CREX_INTRIN_AVX2_FUNC_PTR) || defined(CREX_INTRIN_SSSE3_FUNC_PTR)
	BASIC_MINIT_SUBMODULE(base64_intrin)
#endif

	BASIC_MINIT_SUBMODULE(crypt)

	BASIC_MINIT_SUBMODULE(dir)
#ifdef HAVE_SYSLOG_H
	BASIC_MINIT_SUBMODULE(syslog)
#endif
	BASIC_MINIT_SUBMODULE(array)
	BASIC_MINIT_SUBMODULE(assert)
	BASIC_MINIT_SUBMODULE(url_scanner_ex)
#ifdef CRX_CAN_SUPPORT_PROC_OPEN
	BASIC_MINIT_SUBMODULE(proc_open)
#endif
	BASIC_MINIT_SUBMODULE(exec)

	BASIC_MINIT_SUBMODULE(user_streams)

	crx_register_url_stream_wrapper("crx", &crx_stream_crx_wrapper);
	crx_register_url_stream_wrapper("file", &crx_plain_files_wrapper);
#ifdef HAVE_GLOB
	crx_register_url_stream_wrapper("glob", &crx_glob_stream_wrapper);
#endif
	crx_register_url_stream_wrapper("data", &crx_stream_rfc2397_wrapper);
	crx_register_url_stream_wrapper("http", &crx_stream_http_wrapper);
	crx_register_url_stream_wrapper("ftp", &crx_stream_ftp_wrapper);

	return SUCCESS;
}
/* }}} */

CRX_MSHUTDOWN_FUNCTION(basic) /* {{{ */
{
#ifdef ZTS
	ts_free_id(basic_globals_id);
#ifdef CRX_WIN32
	ts_free_id(crx_win32_core_globals_id);
#endif
#else
	basic_globals_dtor(&basic_globals);
#ifdef CRX_WIN32
	crx_win32_core_globals_dtor(&the_crx_win32_core_globals);
#endif
#endif

	crx_unregister_url_stream_wrapper("crx");
	crx_unregister_url_stream_wrapper("http");
	crx_unregister_url_stream_wrapper("ftp");

	BASIC_MSHUTDOWN_SUBMODULE(browscap)
	BASIC_MSHUTDOWN_SUBMODULE(array)
	BASIC_MSHUTDOWN_SUBMODULE(assert)
	BASIC_MSHUTDOWN_SUBMODULE(url_scanner_ex)
	BASIC_MSHUTDOWN_SUBMODULE(file)
	BASIC_MSHUTDOWN_SUBMODULE(standard_filters)
#ifdef ZTS
	BASIC_MSHUTDOWN_SUBMODULE(localeconv)
#endif
	BASIC_MSHUTDOWN_SUBMODULE(crypt)
	BASIC_MSHUTDOWN_SUBMODULE(password)

	return SUCCESS;
}
/* }}} */

CRX_RINIT_FUNCTION(basic) /* {{{ */
{
	memset(BG(strtok_table), 0, 256);

	BG(serialize_lock) = 0;
	memset(&BG(serialize), 0, sizeof(BG(serialize)));
	memset(&BG(unserialize), 0, sizeof(BG(unserialize)));

	BG(strtok_string) = NULL;
	BG(strtok_last) = NULL;
	BG(ctype_string) = NULL;
	BG(locale_changed) = 0;
	BG(user_compare_fci) = empty_fcall_info;
	BG(user_compare_fci_cache) = empty_fcall_info_cache;
	BG(page_uid) = -1;
	BG(page_gid) = -1;
	BG(page_inode) = -1;
	BG(page_mtime) = -1;
#ifdef HAVE_PUTENV
	crex_hash_init(&BG(putenv_ht), 1, NULL, crx_putenv_destructor, 0);
#endif
	BG(user_shutdown_function_names) = NULL;

	CRX_RINIT(filestat)(INIT_FUNC_ARGS_PASSTHRU);
	BASIC_RINIT_SUBMODULE(dir)
	BASIC_RINIT_SUBMODULE(url_scanner_ex)

	/* Setup default context */
	FG(default_context) = NULL;

	/* Default to global wrappers only */
	FG(stream_wrappers) = NULL;

	/* Default to global filters only */
	FG(stream_filters) = NULL;

	return SUCCESS;
}
/* }}} */

CRX_RSHUTDOWN_FUNCTION(basic) /* {{{ */
{
	if (BG(strtok_string)) {
		crex_string_release(BG(strtok_string));
		BG(strtok_string) = NULL;
	}
#ifdef HAVE_PUTENV
	tsrm_env_lock();
	crex_hash_destroy(&BG(putenv_ht));
	tsrm_env_unlock();
#endif

	if (BG(umask) != -1) {
		umask(BG(umask));
	}

	/* Check if locale was changed and change it back
	 * to the value in startup environment */
	if (BG(locale_changed)) {
		setlocale(LC_ALL, "C");
		crex_reset_lc_ctype_locale();
		crex_update_current_locale();
		if (BG(ctype_string)) {
			crex_string_release_ex(BG(ctype_string), 0);
			BG(ctype_string) = NULL;
		}
	}

	/* FG(stream_wrappers) and FG(stream_filters) are destroyed
	 * during crx_request_shutdown() */

	CRX_RSHUTDOWN(filestat)(SHUTDOWN_FUNC_ARGS_PASSTHRU);
#ifdef HAVE_SYSLOG_H
	BASIC_RSHUTDOWN_SUBMODULE(syslog);
#endif
	BASIC_RSHUTDOWN_SUBMODULE(assert)
	BASIC_RSHUTDOWN_SUBMODULE(url_scanner_ex)
	BASIC_RSHUTDOWN_SUBMODULE(streams)
#ifdef CRX_WIN32
	BASIC_RSHUTDOWN_SUBMODULE(win32_core_globals)
#endif

	if (BG(user_tick_functions)) {
		crex_llist_destroy(BG(user_tick_functions));
		efree(BG(user_tick_functions));
		BG(user_tick_functions) = NULL;
	}

	BASIC_RSHUTDOWN_SUBMODULE(user_filters)
	BASIC_RSHUTDOWN_SUBMODULE(browscap)

	BG(page_uid) = -1;
	BG(page_gid) = -1;
	return SUCCESS;
}
/* }}} */

CRX_MINFO_FUNCTION(basic) /* {{{ */
{
	crx_info_print_table_start();
	BASIC_MINFO_SUBMODULE(dl)
	BASIC_MINFO_SUBMODULE(mail)
	crx_info_print_table_end();
	BASIC_MINFO_SUBMODULE(assert)
}
/* }}} */

/* {{{ Given the name of a constant this function will return the constant's associated value */
CRX_FUNCTION(constant)
{
	crex_string *const_name;
	zval *c;
	crex_class_entry *scope;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(const_name)
	CREX_PARSE_PARAMETERS_END();

	scope = crex_get_executed_scope();
	c = crex_get_constant_ex(const_name, scope, CREX_FETCH_CLASS_EXCEPTION);
	if (!c) {
		RETURN_THROWS();
	}

	ZVAL_COPY_OR_DUP(return_value, c);
	if (C_TYPE_P(return_value) == IS_CONSTANT_AST) {
		if (UNEXPECTED(zval_update_constant_ex(return_value, scope) != SUCCESS)) {
			RETURN_THROWS();
		}
	}
}
/* }}} */

#ifdef HAVE_INET_NTOP
/* {{{ Converts a packed inet address to a human readable IP address string */
CRX_FUNCTION(inet_ntop)
{
	char *address;
	size_t address_len;
	int af = AF_INET;
	char buffer[40];

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STRING(address, address_len)
	CREX_PARSE_PARAMETERS_END();

#ifdef HAVE_IPV6
	if (address_len == 16) {
		af = AF_INET6;
	} else
#endif
	if (address_len != 4) {
		RETURN_FALSE;
	}

	if (!inet_ntop(af, address, buffer, sizeof(buffer))) {
		RETURN_FALSE;
	}

	RETURN_STRING(buffer);
}
/* }}} */
#endif /* HAVE_INET_NTOP */

#ifdef HAVE_INET_PTON
/* {{{ Converts a human readable IP address to a packed binary string */
CRX_FUNCTION(inet_pton)
{
	int ret, af = AF_INET;
	char *address;
	size_t address_len;
	char buffer[17];

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STRING(address, address_len)
	CREX_PARSE_PARAMETERS_END();

	memset(buffer, 0, sizeof(buffer));

#ifdef HAVE_IPV6
	if (strchr(address, ':')) {
		af = AF_INET6;
	} else
#endif
	if (!strchr(address, '.')) {
		RETURN_FALSE;
	}

	ret = inet_pton(af, address, buffer);

	if (ret <= 0) {
		RETURN_FALSE;
	}

	RETURN_STRINGL(buffer, af == AF_INET ? 4 : 16);
}
/* }}} */
#endif /* HAVE_INET_PTON */

/* {{{ Converts a string containing an (IPv4) Internet Protocol dotted address into a proper address */
CRX_FUNCTION(ip2long)
{
	char *addr;
	size_t addr_len;
#ifdef HAVE_INET_PTON
	struct in_addr ip;
#else
	crex_ulong ip;
#endif

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STRING(addr, addr_len)
	CREX_PARSE_PARAMETERS_END();

#ifdef HAVE_INET_PTON
	if (addr_len == 0 || inet_pton(AF_INET, addr, &ip) != 1) {
		RETURN_FALSE;
	}
	RETURN_LONG(ntohl(ip.s_addr));
#else
	if (addr_len == 0 || (ip = inet_addr(addr)) == INADDR_NONE) {
		/* The only special case when we should return -1 ourselves,
		 * because inet_addr() considers it wrong. We return 0xFFFFFFFF and
		 * not -1 or ~0 because of 32/64bit issues. */
		if (addr_len == sizeof("255.255.255.255") - 1 &&
			!memcmp(addr, "255.255.255.255", sizeof("255.255.255.255") - 1)
		) {
			RETURN_LONG(0xFFFFFFFF);
		}
		RETURN_FALSE;
	}
	RETURN_LONG(ntohl(ip));
#endif
}
/* }}} */

/* {{{ Converts an (IPv4) Internet network address into a string in Internet standard dotted format */
CRX_FUNCTION(long2ip)
{
	crex_ulong ip;
	crex_long sip;
	struct in_addr myaddr;
#ifdef HAVE_INET_PTON
	char str[40];
#endif

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(sip)
	CREX_PARSE_PARAMETERS_END();

	/* autoboxes on 32bit platforms, but that's expected */
	ip = (crex_ulong)sip;

	myaddr.s_addr = htonl(ip);
#ifdef HAVE_INET_PTON
	if (inet_ntop(AF_INET, &myaddr, str, sizeof(str))) {
		RETURN_STRING(str);
	} else {
		RETURN_FALSE;
	}
#else
	RETURN_STRING(inet_ntoa(myaddr));
#endif
}
/* }}} */

/********************
 * System Functions *
 ********************/

CRXAPI crex_string *crx_getenv(const char *str, size_t str_len) {
#ifdef CRX_WIN32
	{
		wchar_t *keyw = crx_win32_cp_conv_any_to_w(str, str_len, CRX_WIN32_CP_IGNORE_LEN_P);
		if (!keyw) {
			return NULL;
		}

		SetLastError(0);
		/* If the given buffer is not large enough to hold the data, the return value is
		 * the buffer size,  in characters, required to hold the string and its terminating
		 * null character. We use this return value to alloc the final buffer. */
		wchar_t dummybuf;
		DWORD size = GetEnvironmentVariableW(keyw, &dummybuf, 0);
		if (GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
			/* The environment variable doesn't exist. */
			free(keyw);
			return NULL;
		}

		if (size == 0) {
			/* env exists, but it is empty */
			free(keyw);
			return ZSTR_EMPTY_ALLOC();
		}

		wchar_t *valw = emalloc((size + 1) * sizeof(wchar_t));
		size = GetEnvironmentVariableW(keyw, valw, size);
		if (size == 0) {
			/* has been removed between the two calls */
			free(keyw);
			efree(valw);
			return ZSTR_EMPTY_ALLOC();
		} else {
			char *ptr = crx_win32_cp_w_to_any(valw);
			crex_string *result = crex_string_init(ptr, strlen(ptr), 0);
			free(ptr);
			free(keyw);
			efree(valw);
			return result;
		}
	}
#else
	tsrm_env_lock();

	/* system method returns a const */
	char *ptr = getenv(str);
	crex_string *result = NULL;
	if (ptr) {
		result = crex_string_init(ptr, strlen(ptr), 0);
	}

	tsrm_env_unlock();
	return result;
#endif
}

/* {{{ Get the value of an environment variable or every available environment variable
   if no varname is present  */
CRX_FUNCTION(getenv)
{
	char *str = NULL;
	size_t str_len;
	bool local_only = 0;

	CREX_PARSE_PARAMETERS_START(0, 2)
		C_PARAM_OPTIONAL
		C_PARAM_STRING_OR_NULL(str, str_len)
		C_PARAM_BOOL(local_only)
	CREX_PARSE_PARAMETERS_END();

	if (!str) {
		array_init(return_value);
		crx_import_environment_variables(return_value);
		return;
	}

	if (!local_only) {
		/* SAPI method returns an emalloc()'d string */
		char *ptr = sapi_getenv(str, str_len);
		if (ptr) {
			// TODO: avoid reallocation ???
			RETVAL_STRING(ptr);
			efree(ptr);
			return;
		}
	}

	crex_string *res = crx_getenv(str, str_len);
	if (res) {
		RETURN_STR(res);
	}
	RETURN_FALSE;
}
/* }}} */

#ifdef HAVE_PUTENV
/* {{{ Set the value of an environment variable */
CRX_FUNCTION(putenv)
{
	char *setting;
	size_t setting_len;
	char *p, **env;
	putenv_entry pe;
#ifdef CRX_WIN32
	const char *value = NULL;
	int error_code;
#endif

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STRING(setting, setting_len)
	CREX_PARSE_PARAMETERS_END();

	if (setting_len == 0 || setting[0] == '=') {
		crex_argument_value_error(1, "must have a valid syntax");
		RETURN_THROWS();
	}

	pe.putenv_string = crex_strndup(setting, setting_len);
	if ((p = strchr(setting, '='))) {
		pe.key = crex_string_init(setting, p - setting, 0);
#ifdef CRX_WIN32
		value = p + 1;
#endif
	} else {
		pe.key = crex_string_init(setting, setting_len, 0);
	}

	tsrm_env_lock();
	crex_hash_del(&BG(putenv_ht), pe.key);

	/* find previous value */
	pe.previous_value = NULL;
	for (env = environ; env != NULL && *env != NULL; env++) {
		if (!strncmp(*env, ZSTR_VAL(pe.key), ZSTR_LEN(pe.key))
				&& (*env)[ZSTR_LEN(pe.key)] == '=') {	/* found it */
#ifdef CRX_WIN32
			/* must copy previous value because MSVCRT's putenv can free the string without notice */
			pe.previous_value = estrdup(*env);
#else
			pe.previous_value = *env;
#endif
			break;
		}
	}

#ifdef HAVE_UNSETENV
	if (!p) { /* no '=' means we want to unset it */
		unsetenv(pe.putenv_string);
	}
	if (!p || putenv(pe.putenv_string) == 0) { /* success */
#else
# ifndef CRX_WIN32
	if (putenv(pe.putenv_string) == 0) { /* success */
# else
		wchar_t *keyw, *valw = NULL;

		keyw = crx_win32_cp_any_to_w(ZSTR_VAL(pe.key));
		if (value) {
			valw = crx_win32_cp_any_to_w(value);
		}
		/* valw may be NULL, but the failed conversion still needs to be checked. */
		if (!keyw || !valw && value) {
			tsrm_env_unlock();
			free(pe.putenv_string);
			crex_string_release(pe.key);
			free(keyw);
			free(valw);
			RETURN_FALSE;
		}

	error_code = SetEnvironmentVariableW(keyw, valw);

	if (error_code != 0
# ifndef ZTS
	/* We need both SetEnvironmentVariable and _putenv here as some
		dependency lib could use either way to read the environment.
		Obviously the CRT version will be useful more often. But
		generally, doing both brings us on the safe track at least
		in NTS build. */
	&& _wputenv_s(keyw, valw ? valw : L"") == 0
# endif
	) { /* success */
# endif
#endif
		crex_hash_add_mem(&BG(putenv_ht), pe.key, &pe, sizeof(putenv_entry));
#ifdef HAVE_TZSET
		if (crex_string_equals_literal_ci(pe.key, "TZ")) {
			tzset();
		}
#endif
		tsrm_env_unlock();
#ifdef CRX_WIN32
		free(keyw);
		free(valw);
#endif
		RETURN_TRUE;
	} else {
		free(pe.putenv_string);
		crex_string_release(pe.key);
#ifdef CRX_WIN32
		free(keyw);
		free(valw);
#endif
		RETURN_FALSE;
	}
}
/* }}} */
#endif

/* {{{ free_argv()
   Free the memory allocated to an argv array. */
static void free_argv(char **argv, int argc)
{
	int i;

	if (argv) {
		for (i = 0; i < argc; i++) {
			if (argv[i]) {
				efree(argv[i]);
			}
		}
		efree(argv);
	}
}
/* }}} */

/* {{{ free_longopts()
   Free the memory allocated to an longopt array. */
static void free_longopts(opt_struct *longopts)
{
	opt_struct *p;

	if (longopts) {
		for (p = longopts; p && p->opt_char != '-'; p++) {
			if (p->opt_name != NULL) {
				efree((char *)(p->opt_name));
			}
		}
	}
}
/* }}} */

/* {{{ parse_opts()
   Convert the typical getopt input characters to the crx_getopt struct array */
static int parse_opts(char * opts, opt_struct ** result)
{
	opt_struct * paras = NULL;
	unsigned int i, count = 0;
	unsigned int opts_len = (unsigned int)strlen(opts);

	for (i = 0; i < opts_len; i++) {
		if ((opts[i] >= 48 && opts[i] <= 57) ||
			(opts[i] >= 65 && opts[i] <= 90) ||
			(opts[i] >= 97 && opts[i] <= 122)
		) {
			count++;
		}
	}

	paras = safe_emalloc(sizeof(opt_struct), count, 0);
	memset(paras, 0, sizeof(opt_struct) * count);
	*result = paras;
	while ( (*opts >= 48 && *opts <= 57) || /* 0 - 9 */
			(*opts >= 65 && *opts <= 90) || /* A - Z */
			(*opts >= 97 && *opts <= 122)   /* a - z */
	) {
		paras->opt_char = *opts;
		paras->need_param = *(++opts) == ':';
		paras->opt_name = NULL;
		if (paras->need_param == 1) {
			opts++;
			if (*opts == ':') {
				paras->need_param++;
				opts++;
			}
		}
		paras++;
	}
	return count;
}
/* }}} */

/* {{{ Get options from the command line argument list */
CRX_FUNCTION(getopt)
{
	char *options = NULL, **argv = NULL;
	char opt[2] = { '\0' };
	char *optname;
	int argc = 0, o;
	size_t options_len = 0, len;
	char *crx_optarg = NULL;
	int crx_optind = 1;
	zval val, *args = NULL, *p_longopts = NULL;
	zval *zoptind = NULL;
	size_t optname_len = 0;
	opt_struct *opts, *orig_opts;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_STRING(options, options_len)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY(p_longopts)
		C_PARAM_ZVAL(zoptind)
	CREX_PARSE_PARAMETERS_END();

	/* Init zoptind to 1 */
	if (zoptind) {
		CREX_TRY_ASSIGN_REF_LONG(zoptind, 1);
	}

	/* Get argv from the global symbol table. We calculate argc ourselves
	 * in order to be on the safe side, even though it is also available
	 * from the symbol table. */
	if ((C_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) == IS_ARRAY || crex_is_auto_global(ZSTR_KNOWN(CREX_STR_AUTOGLOBAL_SERVER))) &&
		((args = crex_hash_find_ex_ind(C_ARRVAL_P(&PG(http_globals)[TRACK_VARS_SERVER]), ZSTR_KNOWN(CREX_STR_ARGV), 1)) != NULL ||
		(args = crex_hash_find_ex_ind(&EG(symbol_table), ZSTR_KNOWN(CREX_STR_ARGV), 1)) != NULL)
	) {
		int pos = 0;
		zval *entry;

		if (C_TYPE_P(args) != IS_ARRAY) {
			RETURN_FALSE;
		}
		argc = crex_hash_num_elements(C_ARRVAL_P(args));

		/* Attempt to allocate enough memory to hold all of the arguments
		 * and a trailing NULL */
		argv = (char **) safe_emalloc(sizeof(char *), (argc + 1), 0);

		/* Iterate over the hash to construct the argv array. */
		CREX_HASH_FOREACH_VAL(C_ARRVAL_P(args), entry) {
			crex_string *tmp_arg_str;
			crex_string *arg_str = zval_get_tmp_string(entry, &tmp_arg_str);

			argv[pos++] = estrdup(ZSTR_VAL(arg_str));

			crex_tmp_string_release(tmp_arg_str);
		} CREX_HASH_FOREACH_END();

		/* The C Standard requires argv[argc] to be NULL - this might
		 * keep some getopt implementations happy. */
		argv[argc] = NULL;
	} else {
		/* Return false if we can't find argv. */
		RETURN_FALSE;
	}

	len = parse_opts(options, &opts);

	if (p_longopts) {
		int count;
		zval *entry;

		count = crex_hash_num_elements(C_ARRVAL_P(p_longopts));

		/* the first <len> slots are filled by the one short ops
		 * we now extend our array and jump to the new added structs */
		opts = (opt_struct *) safe_erealloc(opts, sizeof(opt_struct), (len + count + 1), 0);
		orig_opts = opts;
		opts += len;

		memset(opts, 0, count * sizeof(opt_struct));

		/* Iterate over the hash to construct the argv array. */
		CREX_HASH_FOREACH_VAL(C_ARRVAL_P(p_longopts), entry) {
			crex_string *tmp_arg_str;
			crex_string *arg_str = zval_get_tmp_string(entry, &tmp_arg_str);

			opts->need_param = 0;
			opts->opt_name = estrdup(ZSTR_VAL(arg_str));
			len = strlen(opts->opt_name);
			if ((len > 0) && (opts->opt_name[len - 1] == ':')) {
				opts->need_param++;
				opts->opt_name[len - 1] = '\0';
				if ((len > 1) && (opts->opt_name[len - 2] == ':')) {
					opts->need_param++;
					opts->opt_name[len - 2] = '\0';
				}
			}
			opts->opt_char = 0;
			opts++;

			crex_tmp_string_release(tmp_arg_str);
		} CREX_HASH_FOREACH_END();
	} else {
		opts = (opt_struct*) erealloc(opts, sizeof(opt_struct) * (len + 1));
		orig_opts = opts;
		opts += len;
	}

	/* crx_getopt want to identify the last param */
	opts->opt_char   = '-';
	opts->need_param = 0;
	opts->opt_name   = NULL;

	/* Initialize the return value as an array. */
	array_init(return_value);

	/* after our pointer arithmetic jump back to the first element */
	opts = orig_opts;

	while ((o = crx_getopt(argc, argv, opts, &crx_optarg, &crx_optind, 0, 1)) != -1) {
		/* Skip unknown arguments. */
		if (o == CRX_GETOPT_INVALID_ARG) {
			continue;
		}

		/* Prepare the option character and the argument string. */
		if (o == 0) {
			optname = opts[crx_optidx].opt_name;
		} else {
			if (o == 1) {
				o = '-';
			}
			opt[0] = o;
			optname = opt;
		}

		if (crx_optarg != NULL) {
			/* keep the arg as binary, since the encoding is not known */
			ZVAL_STRING(&val, crx_optarg);
		} else {
			ZVAL_FALSE(&val);
		}

		/* Add this option / argument pair to the result hash. */
		optname_len = strlen(optname);
		if (!(optname_len > 1 && optname[0] == '0') && is_numeric_string(optname, optname_len, NULL, NULL, 0) == IS_LONG) {
			/* numeric string */
			int optname_int = atoi(optname);
			if ((args = crex_hash_index_find(C_ARRVAL_P(return_value), optname_int)) != NULL) {
				if (C_TYPE_P(args) != IS_ARRAY) {
					convert_to_array(args);
				}
				crex_hash_next_index_insert(C_ARRVAL_P(args), &val);
			} else {
				crex_hash_index_update(C_ARRVAL_P(return_value), optname_int, &val);
			}
		} else {
			/* other strings */
			if ((args = crex_hash_str_find(C_ARRVAL_P(return_value), optname, strlen(optname))) != NULL) {
				if (C_TYPE_P(args) != IS_ARRAY) {
					convert_to_array(args);
				}
				crex_hash_next_index_insert(C_ARRVAL_P(args), &val);
			} else {
				crex_hash_str_add(C_ARRVAL_P(return_value), optname, strlen(optname), &val);
			}
		}

		crx_optarg = NULL;
	}

	/* Set zoptind to crx_optind */
	if (zoptind) {
		CREX_TRY_ASSIGN_REF_LONG(zoptind, crx_optind);
	}

	free_longopts(orig_opts);
	efree(orig_opts);
	free_argv(argv, argc);
}
/* }}} */

/* {{{ Flush the output buffer */
CRX_FUNCTION(flush)
{
	CREX_PARSE_PARAMETERS_NONE();

	sapi_flush();
}
/* }}} */

/* {{{ Delay for a given number of seconds */
CRX_FUNCTION(sleep)
{
	crex_long num;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(num)
	CREX_PARSE_PARAMETERS_END();

	if (num < 0) {
		crex_argument_value_error(1, "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	RETURN_LONG(crx_sleep((unsigned int)num));
}
/* }}} */

/* {{{ Delay for a given number of micro seconds */
CRX_FUNCTION(usleep)
{
	crex_long num;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(num)
	CREX_PARSE_PARAMETERS_END();

	if (num < 0) {
		crex_argument_value_error(1, "must be greater than or equal to 0");
		RETURN_THROWS();
	}

#ifdef HAVE_USLEEP
	usleep((unsigned int)num);
#endif
}
/* }}} */

#ifdef HAVE_NANOSLEEP
/* {{{ Delay for a number of seconds and nano seconds */
CRX_FUNCTION(time_nanosleep)
{
	crex_long tv_sec, tv_nsec;
	struct timespec crx_req, crx_rem;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_LONG(tv_sec)
		C_PARAM_LONG(tv_nsec)
	CREX_PARSE_PARAMETERS_END();

	if (tv_sec < 0) {
		crex_argument_value_error(1, "must be greater than or equal to 0");
		RETURN_THROWS();
	}
	if (tv_nsec < 0) {
		crex_argument_value_error(2, "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	crx_req.tv_sec = (time_t) tv_sec;
	crx_req.tv_nsec = (long)tv_nsec;
	if (!nanosleep(&crx_req, &crx_rem)) {
		RETURN_TRUE;
	} else if (errno == EINTR) {
		array_init(return_value);
		add_assoc_long_ex(return_value, "seconds", sizeof("seconds")-1, crx_rem.tv_sec);
		add_assoc_long_ex(return_value, "nanoseconds", sizeof("nanoseconds")-1, crx_rem.tv_nsec);
		return;
	} else if (errno == EINVAL) {
		crex_value_error("Nanoseconds was not in the range 0 to 999 999 999 or seconds was negative");
		RETURN_THROWS();
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ Make the script sleep until the specified time */
CRX_FUNCTION(time_sleep_until)
{
	double target_secs;
	struct timeval tm;
	struct timespec crx_req, crx_rem;
	uint64_t current_ns, target_ns, diff_ns;
	const uint64_t ns_per_sec = 1000000000;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(target_secs)
	CREX_PARSE_PARAMETERS_END();

	if (gettimeofday((struct timeval *) &tm, NULL) != 0) {
		RETURN_FALSE;
	}

	target_ns = (uint64_t) (target_secs * ns_per_sec);
	current_ns = ((uint64_t) tm.tv_sec) * ns_per_sec + ((uint64_t) tm.tv_usec) * 1000;
	if (target_ns < current_ns) {
		crx_error_docref(NULL, E_WARNING, "Argument #1 ($timestamp) must be greater than or equal to the current time");
		RETURN_FALSE;
	}

	diff_ns = target_ns - current_ns;
	crx_req.tv_sec = (time_t) (diff_ns / ns_per_sec);
	crx_req.tv_nsec = (long) (diff_ns % ns_per_sec);

	while (nanosleep(&crx_req, &crx_rem)) {
		if (errno == EINTR) {
			crx_req.tv_sec = crx_rem.tv_sec;
			crx_req.tv_nsec = crx_rem.tv_nsec;
		} else {
			RETURN_FALSE;
		}
	}

	RETURN_TRUE;
}
/* }}} */
#endif

/* {{{ Get the name of the owner of the current CRX script */
CRX_FUNCTION(get_current_user)
{
	CREX_PARSE_PARAMETERS_NONE();

	RETURN_STRING(crx_get_current_user());
}
/* }}} */

#define ZVAL_SET_INI_STR(zv, val) do { \
	if (ZSTR_IS_INTERNED(val)) { \
		ZVAL_INTERNED_STR(zv, val); \
	} else if (ZSTR_LEN(val) == 0) { \
		ZVAL_EMPTY_STRING(zv); \
	} else if (ZSTR_LEN(val) == 1) { \
		ZVAL_CHAR(zv, ZSTR_VAL(val)[0]); \
	} else if (!(GC_FLAGS(val) & GC_PERSISTENT)) { \
		ZVAL_NEW_STR(zv, crex_string_copy(val)); \
	} else { \
		ZVAL_NEW_STR(zv, crex_string_init(ZSTR_VAL(val), ZSTR_LEN(val), 0)); \
	} \
} while (0)

static void add_config_entries(HashTable *hash, zval *return_value);

/* {{{ add_config_entry */
static void add_config_entry(crex_ulong h, crex_string *key, zval *entry, zval *retval)
{
	if (C_TYPE_P(entry) == IS_STRING) {
		zval str_zv;
		ZVAL_SET_INI_STR(&str_zv, C_STR_P(entry));
		if (key) {
			add_assoc_zval_ex(retval, ZSTR_VAL(key), ZSTR_LEN(key), &str_zv);
		} else {
			add_index_zval(retval, h, &str_zv);
		}
	} else if (C_TYPE_P(entry) == IS_ARRAY) {
		zval tmp;
		array_init(&tmp);
		add_config_entries(C_ARRVAL_P(entry), &tmp);
		crex_hash_update(C_ARRVAL_P(retval), key, &tmp);
	}
}
/* }}} */

/* {{{ add_config_entries */
static void add_config_entries(HashTable *hash, zval *return_value) /* {{{ */
{
	crex_ulong h;
	crex_string *key;
	zval *zv;

	CREX_HASH_FOREACH_KEY_VAL(hash, h, key, zv)
		add_config_entry(h, key, zv, return_value);
	CREX_HASH_FOREACH_END();
}
/* }}} */

/* {{{ Get the value of a CRX configuration option */
CRX_FUNCTION(get_cfg_var)
{
	crex_string *varname;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(varname)
	CREX_PARSE_PARAMETERS_END();

	zval *retval = cfg_get_entry_ex(varname);

	if (retval) {
		if (C_TYPE_P(retval) == IS_ARRAY) {
			array_init(return_value);
			add_config_entries(C_ARRVAL_P(retval), return_value);
			return;
		} else {
			ZVAL_SET_INI_STR(return_value, C_STR_P(retval));
		}
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/*
	1st arg = error message
	2nd arg = error option
	3rd arg = optional parameters (email address or tcp address)
	4th arg = used for additional headers if email

error options:
	0 = send to crx_error_log (uses syslog or file depending on ini setting)
	1 = send via email to 3rd parameter 4th option = additional headers
	2 = send via tcp/ip to 3rd parameter (name or ip:port)
	3 = save to file in 3rd parameter
	4 = send to SAPI logger directly
*/

/* {{{ Send an error message somewhere */
CRX_FUNCTION(error_log)
{
	char *message, *opt = NULL, *headers = NULL;
	size_t message_len, opt_len = 0, headers_len = 0;
	crex_long erropt = 0;

	CREX_PARSE_PARAMETERS_START(1, 4)
		C_PARAM_STRING(message, message_len)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(erropt)
		C_PARAM_PATH_OR_NULL(opt, opt_len)
		C_PARAM_STRING_OR_NULL(headers, headers_len)
	CREX_PARSE_PARAMETERS_END();

	if (_crx_error_log_ex((int) erropt, message, message_len, opt, headers) == FAILURE) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* For BC (not binary-safe!) */
CRXAPI int _crx_error_log(int opt_err, const char *message, const char *opt, const char *headers) /* {{{ */
{
	return _crx_error_log_ex(opt_err, message, (opt_err == 3) ? strlen(message) : 0, opt, headers);
}
/* }}} */

CRXAPI int _crx_error_log_ex(int opt_err, const char *message, size_t message_len, const char *opt, const char *headers) /* {{{ */
{
	crx_stream *stream = NULL;
	size_t nbytes;

	switch (opt_err)
	{
		case 1:		/*send an email */
			if (!crx_mail(opt, "CRX error_log message", message, headers, NULL)) {
				return FAILURE;
			}
			break;

		case 2:		/*send to an address */
			crex_value_error("TCP/IP option is not available for error logging");
			return FAILURE;

		case 3:		/*save to a file */
			stream = crx_stream_open_wrapper(opt, "a", REPORT_ERRORS, NULL);
			if (!stream) {
				return FAILURE;
			}
			nbytes = crx_stream_write(stream, message, message_len);
			crx_stream_close(stream);
			if (nbytes != message_len) {
				return FAILURE;
			}
			break;

		case 4: /* send to SAPI */
			if (sapi_module.log_message) {
				sapi_module.log_message(message, -1);
			} else {
				return FAILURE;
			}
			break;

		default:
			crx_log_err_with_severity(message, LOG_NOTICE);
			break;
	}
	return SUCCESS;
}
/* }}} */

/* {{{ Get the last occurred error as associative array. Returns NULL if there hasn't been an error yet. */
CRX_FUNCTION(error_get_last)
{
	CREX_PARSE_PARAMETERS_NONE();

	if (PG(last_error_message)) {
		zval tmp;
		array_init(return_value);

		ZVAL_LONG(&tmp, PG(last_error_type));
		crex_hash_update(C_ARR_P(return_value), ZSTR_KNOWN(CREX_STR_TYPE), &tmp);

		ZVAL_STR_COPY(&tmp, PG(last_error_message));
		crex_hash_update(C_ARR_P(return_value), ZSTR_KNOWN(CREX_STR_MESSAGE), &tmp);

		ZVAL_STR_COPY(&tmp, PG(last_error_file));
		crex_hash_update(C_ARR_P(return_value), ZSTR_KNOWN(CREX_STR_FILE), &tmp);

		ZVAL_LONG(&tmp, PG(last_error_lineno));
		crex_hash_update(C_ARR_P(return_value), ZSTR_KNOWN(CREX_STR_LINE), &tmp);
	}
}
/* }}} */

/* {{{ Clear the last occurred error. */
CRX_FUNCTION(error_clear_last)
{
	CREX_PARSE_PARAMETERS_NONE();

	if (PG(last_error_message)) {
		PG(last_error_type) = 0;
		PG(last_error_lineno) = 0;

		crex_string_release(PG(last_error_message));
		PG(last_error_message) = NULL;

		if (PG(last_error_file)) {
			crex_string_release(PG(last_error_file));
			PG(last_error_file) = NULL;
		}
	}
}
/* }}} */

/* {{{ Call a user function which is the first parameter
   Warning: This function is special-cased by crex_compile.c and so is usually bypassed */
CRX_FUNCTION(call_user_func)
{
	zval retval;
	crex_fcall_info fci;
	crex_fcall_info_cache fci_cache;

	CREX_PARSE_PARAMETERS_START(1, -1)
		C_PARAM_FUNC(fci, fci_cache)
		C_PARAM_VARIADIC_WITH_NAMED(fci.params, fci.param_count, fci.named_params)
	CREX_PARSE_PARAMETERS_END();

	fci.retval = &retval;

	if (crex_call_function(&fci, &fci_cache) == SUCCESS && C_TYPE(retval) != IS_UNDEF) {
		if (C_ISREF(retval)) {
			crex_unwrap_reference(&retval);
		}
		ZVAL_COPY_VALUE(return_value, &retval);
	}
}
/* }}} */

/* {{{ Call a user function which is the first parameter with the arguments contained in array
   Warning: This function is special-cased by crex_compile.c and so is usually bypassed */
CRX_FUNCTION(call_user_func_array)
{
	zval retval;
	HashTable *params;
	crex_fcall_info fci;
	crex_fcall_info_cache fci_cache;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_FUNC(fci, fci_cache)
		C_PARAM_ARRAY_HT(params)
	CREX_PARSE_PARAMETERS_END();

	fci.named_params = params;
	fci.retval = &retval;

	if (crex_call_function(&fci, &fci_cache) == SUCCESS && C_TYPE(retval) != IS_UNDEF) {
		if (C_ISREF(retval)) {
			crex_unwrap_reference(&retval);
		}
		ZVAL_COPY_VALUE(return_value, &retval);
	}
}
/* }}} */

/* {{{ Call a user function which is the first parameter */
CRX_FUNCTION(forward_static_call)
{
	zval retval;
	crex_fcall_info fci;
	crex_fcall_info_cache fci_cache;
	crex_class_entry *called_scope;

	CREX_PARSE_PARAMETERS_START(1, -1)
		C_PARAM_FUNC(fci, fci_cache)
		C_PARAM_VARIADIC('*', fci.params, fci.param_count)
	CREX_PARSE_PARAMETERS_END();

	if (!EX(prev_execute_data)->func->common.scope) {
		crex_throw_error(NULL, "Cannot call forward_static_call() when no class scope is active");
		RETURN_THROWS();
	}

	fci.retval = &retval;

	called_scope = crex_get_called_scope(execute_data);
	if (called_scope && fci_cache.calling_scope &&
		instanceof_function(called_scope, fci_cache.calling_scope)) {
			fci_cache.called_scope = called_scope;
	}

	if (crex_call_function(&fci, &fci_cache) == SUCCESS && C_TYPE(retval) != IS_UNDEF) {
		if (C_ISREF(retval)) {
			crex_unwrap_reference(&retval);
		}
		ZVAL_COPY_VALUE(return_value, &retval);
	}
}
/* }}} */

/* {{{ Call a static method which is the first parameter with the arguments contained in array */
CRX_FUNCTION(forward_static_call_array)
{
	zval retval;
	HashTable *params;
	crex_fcall_info fci;
	crex_fcall_info_cache fci_cache;
	crex_class_entry *called_scope;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_FUNC(fci, fci_cache)
		C_PARAM_ARRAY_HT(params)
	CREX_PARSE_PARAMETERS_END();

	fci.retval = &retval;
	/* Add positional arguments */
	fci.named_params = params;

	called_scope = crex_get_called_scope(execute_data);
	if (called_scope && fci_cache.calling_scope &&
		instanceof_function(called_scope, fci_cache.calling_scope)) {
			fci_cache.called_scope = called_scope;
	}

	if (crex_call_function(&fci, &fci_cache) == SUCCESS && C_TYPE(retval) != IS_UNDEF) {
		if (C_ISREF(retval)) {
			crex_unwrap_reference(&retval);
		}
		ZVAL_COPY_VALUE(return_value, &retval);
	}
}
/* }}} */

static void fci_addref(crex_fcall_info *fci, crex_fcall_info_cache *fci_cache)
{
	C_TRY_ADDREF(fci->function_name);
	if (fci_cache->object) {
		GC_ADDREF(fci_cache->object);
	}
}

static void fci_release(crex_fcall_info *fci, crex_fcall_info_cache *fci_cache)
{
	zval_ptr_dtor(&fci->function_name);
	if (fci_cache->object) {
		crex_object_release(fci_cache->object);
	}
}

void user_shutdown_function_dtor(zval *zv) /* {{{ */
{
	crx_shutdown_function_entry *shutdown_function_entry = C_PTR_P(zv);

	crex_fcall_info_args_clear(&shutdown_function_entry->fci, true);
	fci_release(&shutdown_function_entry->fci, &shutdown_function_entry->fci_cache);
	efree(shutdown_function_entry);
}
/* }}} */

void user_tick_function_dtor(user_tick_function_entry *tick_function_entry) /* {{{ */
{
	crex_fcall_info_args_clear(&tick_function_entry->fci, true);
	fci_release(&tick_function_entry->fci, &tick_function_entry->fci_cache);
}
/* }}} */

static int user_shutdown_function_call(zval *zv) /* {{{ */
{
	crx_shutdown_function_entry *shutdown_function_entry = C_PTR_P(zv);
	zval retval;
	crex_result call_status;

	/* set retval zval for FCI struct */
	shutdown_function_entry->fci.retval = &retval;
	call_status = crex_call_function(&shutdown_function_entry->fci, &shutdown_function_entry->fci_cache);
	CREX_ASSERT(call_status == SUCCESS);
	zval_ptr_dtor(&retval);

	return 0;
}
/* }}} */

static void user_tick_function_call(user_tick_function_entry *tick_fe) /* {{{ */
{
	/* Prevent re-entrant calls to the same user ticks function */
	if (!tick_fe->calling) {
		zval tmp;

		/* set tmp zval */
		tick_fe->fci.retval = &tmp;

		tick_fe->calling = true;
		crex_call_function(&tick_fe->fci, &tick_fe->fci_cache);

		/* Destroy return value */
		zval_ptr_dtor(&tmp);
		tick_fe->calling = false;
	}
}
/* }}} */

static void run_user_tick_functions(int tick_count, void *arg) /* {{{ */
{
	crex_llist_apply(BG(user_tick_functions), (llist_apply_func_t) user_tick_function_call);
}
/* }}} */

static int user_tick_function_compare(user_tick_function_entry * tick_fe1, user_tick_function_entry * tick_fe2) /* {{{ */
{
	zval *func1 = &tick_fe1->fci.function_name;
	zval *func2 = &tick_fe2->fci.function_name;
	int ret;

	if (C_TYPE_P(func1) == IS_STRING && C_TYPE_P(func2) == IS_STRING) {
		ret = crex_binary_zval_strcmp(func1, func2) == 0;
	} else if (C_TYPE_P(func1) == IS_ARRAY && C_TYPE_P(func2) == IS_ARRAY) {
		ret = crex_compare_arrays(func1, func2) == 0;
	} else if (C_TYPE_P(func1) == IS_OBJECT && C_TYPE_P(func2) == IS_OBJECT) {
		ret = crex_compare_objects(func1, func2) == 0;
	} else {
		ret = 0;
	}

	if (ret && tick_fe1->calling) {
		crex_throw_error(NULL, "Registered tick function cannot be unregistered while it is being executed");
		return 0;
	}
	return ret;
}
/* }}} */

CRXAPI void crx_call_shutdown_functions(void) /* {{{ */
{
	if (BG(user_shutdown_function_names)) {
		crex_try {
			crex_hash_apply(BG(user_shutdown_function_names), user_shutdown_function_call);
		} crex_end_try();
	}
}
/* }}} */

CRXAPI void crx_free_shutdown_functions(void) /* {{{ */
{
	if (BG(user_shutdown_function_names))
		crex_try {
			crex_hash_destroy(BG(user_shutdown_function_names));
			FREE_HASHTABLE(BG(user_shutdown_function_names));
			BG(user_shutdown_function_names) = NULL;
		} crex_catch {
			/* maybe shutdown method call exit, we just ignore it */
			FREE_HASHTABLE(BG(user_shutdown_function_names));
			BG(user_shutdown_function_names) = NULL;
		} crex_end_try();
}
/* }}} */

/* {{{ Register a user-level function to be called on request termination */
CRX_FUNCTION(register_shutdown_function)
{
	crx_shutdown_function_entry entry;
	zval *params = NULL;
	uint32_t param_count = 0;
	bool status;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "f*", &entry.fci, &entry.fci_cache, &params, &param_count) == FAILURE) {
		RETURN_THROWS();
	}

	fci_addref(&entry.fci, &entry.fci_cache);
	crex_fcall_info_argp(&entry.fci, param_count, params);

	status = append_user_shutdown_function(&entry);
	CREX_ASSERT(status);
}
/* }}} */

CRXAPI bool register_user_shutdown_function(const char *function_name, size_t function_len, crx_shutdown_function_entry *shutdown_function_entry) /* {{{ */
{
	if (!BG(user_shutdown_function_names)) {
		ALLOC_HASHTABLE(BG(user_shutdown_function_names));
		crex_hash_init(BG(user_shutdown_function_names), 0, NULL, user_shutdown_function_dtor, 0);
	}

	crex_hash_str_update_mem(BG(user_shutdown_function_names), function_name, function_len, shutdown_function_entry, sizeof(crx_shutdown_function_entry));
	return 1;
}
/* }}} */

CRXAPI bool remove_user_shutdown_function(const char *function_name, size_t function_len) /* {{{ */
{
	if (BG(user_shutdown_function_names)) {
		return crex_hash_str_del(BG(user_shutdown_function_names), function_name, function_len) != FAILURE;
	}

	return 0;
}
/* }}} */

CRXAPI bool append_user_shutdown_function(crx_shutdown_function_entry *shutdown_function_entry) /* {{{ */
{
	if (!BG(user_shutdown_function_names)) {
		ALLOC_HASHTABLE(BG(user_shutdown_function_names));
		crex_hash_init(BG(user_shutdown_function_names), 0, NULL, user_shutdown_function_dtor, 0);
	}

	return crex_hash_next_index_insert_mem(BG(user_shutdown_function_names), shutdown_function_entry, sizeof(crx_shutdown_function_entry)) != NULL;
}
/* }}} */

CREX_API void crx_get_highlight_struct(crex_syntax_highlighter_ini *syntax_highlighter_ini) /* {{{ */
{
	syntax_highlighter_ini->highlight_comment = INI_STR("highlight.comment");
	syntax_highlighter_ini->highlight_default = INI_STR("highlight.default");
	syntax_highlighter_ini->highlight_html    = INI_STR("highlight.html");
	syntax_highlighter_ini->highlight_keyword = INI_STR("highlight.keyword");
	syntax_highlighter_ini->highlight_string  = INI_STR("highlight.string");
}
/* }}} */

/* {{{ Syntax highlight a source file */
CRX_FUNCTION(highlight_file)
{
	char *filename;
	size_t filename_len;
	int ret;
	crex_syntax_highlighter_ini syntax_highlighter_ini;
	bool i = 0;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_PATH(filename, filename_len)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(i)
	CREX_PARSE_PARAMETERS_END();

	if (crx_check_open_basedir(filename)) {
		RETURN_FALSE;
	}

	if (i) {
		crx_output_start_default();
	}

	crx_get_highlight_struct(&syntax_highlighter_ini);

	ret = highlight_file(filename, &syntax_highlighter_ini);

	if (ret == FAILURE) {
		if (i) {
			crx_output_end();
		}
		RETURN_FALSE;
	}

	if (i) {
		crx_output_get_contents(return_value);
		crx_output_discard();
		CREX_ASSERT(C_TYPE_P(return_value) == IS_STRING);
	} else {
		RETURN_TRUE;
	}
}
/* }}} */

/* {{{ Return source with stripped comments and whitespace */
CRX_FUNCTION(crx_strip_whitespace)
{
	crex_string *filename;
	crex_lex_state original_lex_state;
	crex_file_handle file_handle;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_PATH_STR(filename)
	CREX_PARSE_PARAMETERS_END();

	crx_output_start_default();

	crex_stream_init_filename_ex(&file_handle, filename);
	crex_save_lexical_state(&original_lex_state);
	if (open_file_for_scanning(&file_handle) == FAILURE) {
		crex_restore_lexical_state(&original_lex_state);
		crx_output_end();
		crex_destroy_file_handle(&file_handle);
		RETURN_EMPTY_STRING();
	}

	crex_strip();

	crex_restore_lexical_state(&original_lex_state);

	crx_output_get_contents(return_value);
	crx_output_discard();
	crex_destroy_file_handle(&file_handle);
}
/* }}} */

/* {{{ Syntax highlight a string or optionally return it */
CRX_FUNCTION(highlight_string)
{
	crex_string *str;
	crex_syntax_highlighter_ini syntax_highlighter_ini;
	char *hicompiled_string_description;
	bool i = 0;
	int old_error_reporting = EG(error_reporting);

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STR(str)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(i)
	CREX_PARSE_PARAMETERS_END();

	if (i) {
		crx_output_start_default();
	}

	EG(error_reporting) = E_ERROR;

	crx_get_highlight_struct(&syntax_highlighter_ini);

	hicompiled_string_description = crex_make_compiled_string_description("highlighted code");

	highlight_string(str, &syntax_highlighter_ini, hicompiled_string_description);
	efree(hicompiled_string_description);

	EG(error_reporting) = old_error_reporting;

	if (i) {
		crx_output_get_contents(return_value);
		crx_output_discard();
		CREX_ASSERT(C_TYPE_P(return_value) == IS_STRING);
	} else {
		// TODO Make this function void?
		RETURN_TRUE;
	}
}
/* }}} */

/* {{{ Get interpreted size from the ini shorthand syntax */
CRX_FUNCTION(ini_parse_quantity)
{
	crex_string *shorthand;
	crex_string *errstr;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(shorthand)
	CREX_PARSE_PARAMETERS_END();

	RETVAL_LONG(crex_ini_parse_quantity(shorthand, &errstr));

	if (errstr) {
		crex_error(E_WARNING, "%s", ZSTR_VAL(errstr));
		crex_string_release(errstr);
	}
}
/* }}} */

/* {{{ Get a configuration option */
CRX_FUNCTION(ini_get)
{
	crex_string *varname, *val;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(varname)
	CREX_PARSE_PARAMETERS_END();

	val = crex_ini_get_value(varname);

	if (!val) {
		RETURN_FALSE;
	}

	ZVAL_SET_INI_STR(return_value, val);
}
/* }}} */

/* {{{ Get all configuration options */
CRX_FUNCTION(ini_get_all)
{
	char *extname = NULL;
	size_t extname_len = 0, module_number = 0;
	crex_module_entry *module;
	bool details = 1;
	crex_string *key;
	crex_ini_entry *ini_entry;


	CREX_PARSE_PARAMETERS_START(0, 2)
		C_PARAM_OPTIONAL
		C_PARAM_STRING_OR_NULL(extname, extname_len)
		C_PARAM_BOOL(details)
	CREX_PARSE_PARAMETERS_END();

	crex_ini_sort_entries();

	if (extname) {
		if ((module = crex_hash_str_find_ptr(&module_registry, extname, extname_len)) == NULL) {
			crx_error_docref(NULL, E_WARNING, "Extension \"%s\" cannot be found", extname);
			RETURN_FALSE;
		}
		module_number = module->module_number;
	}

	array_init(return_value);
	CREX_HASH_MAP_FOREACH_STR_KEY_PTR(EG(ini_directives), key, ini_entry) {
		zval option;

		if (module_number != 0 && ini_entry->module_number != module_number) {
			continue;
		}

		if (key == NULL || ZSTR_VAL(key)[0] != 0) {
			if (details) {
				array_init(&option);

				if (ini_entry->orig_value) {
					add_assoc_str(&option, "global_value", crex_string_copy(ini_entry->orig_value));
				} else if (ini_entry->value) {
					add_assoc_str(&option, "global_value", crex_string_copy(ini_entry->value));
				} else {
					add_assoc_null(&option, "global_value");
				}

				if (ini_entry->value) {
					add_assoc_str(&option, "local_value", crex_string_copy(ini_entry->value));
				} else {
					add_assoc_null(&option, "local_value");
				}

				add_assoc_long(&option, "access", ini_entry->modifiable);

				crex_symtable_update(C_ARRVAL_P(return_value), ini_entry->name, &option);
			} else {
				if (ini_entry->value) {
					zval zv;

					ZVAL_STR_COPY(&zv, ini_entry->value);
					crex_symtable_update(C_ARRVAL_P(return_value), ini_entry->name, &zv);
				} else {
					crex_symtable_update(C_ARRVAL_P(return_value), ini_entry->name, &EG(uninitialized_zval));
				}
			}
		}
	} CREX_HASH_FOREACH_END();
}
/* }}} */

static int crx_ini_check_path(char *option_name, size_t option_len, char *new_option_name, size_t new_option_len) /* {{{ */
{
	if (option_len + 1 != new_option_len) {
		return 0;
	}

	return !strncmp(option_name, new_option_name, option_len);
}
/* }}} */

/* {{{ Set a configuration option, returns false on error and the old value of the configuration option on success */
CRX_FUNCTION(ini_set)
{
	crex_string *varname;
	zval *new_value;
	crex_string *val;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_STR(varname)
		C_PARAM_ZVAL(new_value)
	CREX_PARSE_PARAMETERS_END();

	if (C_TYPE_P(new_value) > IS_STRING) {
		crex_argument_type_error(2, "must be of type string|int|float|bool|null");
		RETURN_THROWS();
	}

	val = crex_ini_get_value(varname);

	if (val) {
		ZVAL_SET_INI_STR(return_value, val);
	} else {
		RETVAL_FALSE;
	}

	crex_string *new_value_tmp_str;
	crex_string *new_value_str = zval_get_tmp_string(new_value, &new_value_tmp_str);

#define _CHECK_PATH(var, var_len, ini) crx_ini_check_path(var, var_len, ini, sizeof(ini))
	/* open basedir check */
	if (PG(open_basedir)) {
		if (_CHECK_PATH(ZSTR_VAL(varname), ZSTR_LEN(varname), "error_log") ||
			_CHECK_PATH(ZSTR_VAL(varname), ZSTR_LEN(varname), "java.class.path") ||
			_CHECK_PATH(ZSTR_VAL(varname), ZSTR_LEN(varname), "java.home") ||
			_CHECK_PATH(ZSTR_VAL(varname), ZSTR_LEN(varname), "mail.log") ||
			_CHECK_PATH(ZSTR_VAL(varname), ZSTR_LEN(varname), "java.library.path") ||
			_CHECK_PATH(ZSTR_VAL(varname), ZSTR_LEN(varname), "vpopmail.directory")) {
			if (crx_check_open_basedir(ZSTR_VAL(new_value_str))) {
				zval_ptr_dtor_str(return_value);
				crex_tmp_string_release(new_value_tmp_str);
				RETURN_FALSE;
			}
		}
	}
#undef _CHECK_PATH

	if (crex_alter_ini_entry_ex(varname, new_value_str, CRX_INI_USER, CRX_INI_STAGE_RUNTIME, 0) == FAILURE) {
		zval_ptr_dtor_str(return_value);
		RETVAL_FALSE;
	}
	crex_tmp_string_release(new_value_tmp_str);
}
/* }}} */

/* {{{ Restore the value of a configuration option specified by varname */
CRX_FUNCTION(ini_restore)
{
	crex_string *varname;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(varname)
	CREX_PARSE_PARAMETERS_END();

	crex_restore_ini_entry(varname, CRX_INI_STAGE_RUNTIME);
}
/* }}} */

/* {{{ Sets the include_path configuration option */
CRX_FUNCTION(set_include_path)
{
	crex_string *new_value;
	char *old_value;
	crex_string *key;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_PATH_STR(new_value)
	CREX_PARSE_PARAMETERS_END();

	old_value = crex_ini_string("include_path", sizeof("include_path") - 1, 0);
	/* copy to return here, because alter might free it! */
	if (old_value) {
		RETVAL_STRING(old_value);
	} else {
		RETVAL_FALSE;
	}

	key = ZSTR_INIT_LITERAL("include_path", 0);
	if (crex_alter_ini_entry_ex(key, new_value, CRX_INI_USER, CRX_INI_STAGE_RUNTIME, 0) == FAILURE) {
		crex_string_release_ex(key, 0);
		zval_ptr_dtor_str(return_value);
		RETURN_FALSE;
	}
	crex_string_release_ex(key, 0);
}
/* }}} */

/* {{{ Get the current include_path configuration option */
CRX_FUNCTION(get_include_path)
{
	char *str;

	CREX_PARSE_PARAMETERS_NONE();

	str = crex_ini_string("include_path", sizeof("include_path") - 1, 0);

	if (str == NULL) {
		RETURN_FALSE;
	}

	RETURN_STRING(str);
}
/* }}} */

/* {{{ Prints out or returns information about the specified variable */
CRX_FUNCTION(print_r)
{
	zval *var;
	bool do_return = 0;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_ZVAL(var)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(do_return)
	CREX_PARSE_PARAMETERS_END();

	if (do_return) {
		RETURN_STR(crex_print_zval_r_to_str(var, 0));
	} else {
		crex_print_zval_r(var, 0);
		RETURN_TRUE;
	}
}
/* }}} */

/* {{{ Returns true if client disconnected */
CRX_FUNCTION(connection_aborted)
{
	CREX_PARSE_PARAMETERS_NONE();

	RETURN_LONG(PG(connection_status) & CRX_CONNECTION_ABORTED);
}
/* }}} */

/* {{{ Returns the connection status bitfield */
CRX_FUNCTION(connection_status)
{
	CREX_PARSE_PARAMETERS_NONE();

	RETURN_LONG(PG(connection_status));
}
/* }}} */

/* {{{ Set whether we want to ignore a user abort event or not */
CRX_FUNCTION(ignore_user_abort)
{
	bool arg = 0;
	bool arg_is_null = 1;
	int old_setting;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL_OR_NULL(arg, arg_is_null)
	CREX_PARSE_PARAMETERS_END();

	old_setting = (unsigned short)PG(ignore_user_abort);

	if (!arg_is_null) {
		crex_string *key = ZSTR_INIT_LITERAL("ignore_user_abort", 0);
		crex_alter_ini_entry_chars(key, arg ? "1" : "0", 1, CRX_INI_USER, CRX_INI_STAGE_RUNTIME);
		crex_string_release_ex(key, 0);
	}

	RETURN_LONG(old_setting);
}
/* }}} */

#ifdef HAVE_GETSERVBYNAME
/* {{{ Returns port associated with service. Protocol must be "tcp" or "udp" */
CRX_FUNCTION(getservbyname)
{
	crex_string *name;
	char *proto;
	size_t proto_len;
	struct servent *serv;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_STR(name)
		C_PARAM_STRING(proto, proto_len)
	CREX_PARSE_PARAMETERS_END();


/* empty string behaves like NULL on windows implementation of
   getservbyname. Let be portable instead. */
#ifdef CRX_WIN32
	if (proto_len == 0) {
		RETURN_FALSE;
	}
#endif

	serv = getservbyname(ZSTR_VAL(name), proto);

#ifdef _AIX
	/*
        On AIX, imap is only known as imap2 in /etc/services, while on Linux imap is an alias for imap2.
        If a request for imap gives no result, we try again with imap2.
        */
	if (serv == NULL && crex_string_equals_literal(name, "imap")) {
		serv = getservbyname("imap2", proto);
	}
#endif
	if (serv == NULL) {
		RETURN_FALSE;
	}

	RETURN_LONG(ntohs(serv->s_port));
}
/* }}} */
#endif

#ifdef HAVE_GETSERVBYPORT
/* {{{ Returns service name associated with port. Protocol must be "tcp" or "udp" */
CRX_FUNCTION(getservbyport)
{
	char *proto;
	size_t proto_len;
	crex_long port;
	struct servent *serv;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_LONG(port)
		C_PARAM_STRING(proto, proto_len)
	CREX_PARSE_PARAMETERS_END();

	serv = getservbyport(htons((unsigned short) port), proto);

	if (serv == NULL) {
		RETURN_FALSE;
	}

	RETURN_STRING(serv->s_name);
}
/* }}} */
#endif

#ifdef HAVE_GETPROTOBYNAME
/* {{{ Returns protocol number associated with name as per /etc/protocols */
CRX_FUNCTION(getprotobyname)
{
	char *name;
	size_t name_len;
	struct protoent *ent;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STRING(name, name_len)
	CREX_PARSE_PARAMETERS_END();

	ent = getprotobyname(name);

	if (ent == NULL) {
		RETURN_FALSE;
	}

	RETURN_LONG(ent->p_proto);
}
/* }}} */
#endif

#ifdef HAVE_GETPROTOBYNUMBER
/* {{{ Returns protocol name associated with protocol number proto */
CRX_FUNCTION(getprotobynumber)
{
	crex_long proto;
	struct protoent *ent;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(proto)
	CREX_PARSE_PARAMETERS_END();

	ent = getprotobynumber((int)proto);

	if (ent == NULL) {
		RETURN_FALSE;
	}

	RETURN_STRING(ent->p_name);
}
/* }}} */
#endif

/* {{{ Registers a tick callback function */
CRX_FUNCTION(register_tick_function)
{
	user_tick_function_entry tick_fe;
	zval *params = NULL;
	uint32_t param_count = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "f*", &tick_fe.fci, &tick_fe.fci_cache, &params, &param_count) == FAILURE) {
		RETURN_THROWS();
	}

	tick_fe.calling = false;
	fci_addref(&tick_fe.fci, &tick_fe.fci_cache);
	crex_fcall_info_argp(&tick_fe.fci, param_count, params);

	if (!BG(user_tick_functions)) {
		BG(user_tick_functions) = (crex_llist *) emalloc(sizeof(crex_llist));
		crex_llist_init(BG(user_tick_functions),
						sizeof(user_tick_function_entry),
						(llist_dtor_func_t) user_tick_function_dtor, 0);
		crx_add_tick_function(run_user_tick_functions, NULL);
	}

	crex_llist_add_element(BG(user_tick_functions), &tick_fe);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Unregisters a tick callback function */
CRX_FUNCTION(unregister_tick_function)
{
	user_tick_function_entry tick_fe;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_FUNC(tick_fe.fci, tick_fe.fci_cache)
	CREX_PARSE_PARAMETERS_END();

	if (!BG(user_tick_functions)) {
		return;
	}

	crex_llist_del_element(BG(user_tick_functions), &tick_fe, (int (*)(void *, void *)) user_tick_function_compare);
}
/* }}} */

/* {{{ Check if file was created by rfc1867 upload */
CRX_FUNCTION(is_uploaded_file)
{
	char *path;
	size_t path_len;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_PATH(path, path_len)
	CREX_PARSE_PARAMETERS_END();

	if (!SG(rfc1867_uploaded_files)) {
		RETURN_FALSE;
	}

	if (crex_hash_str_exists(SG(rfc1867_uploaded_files), path, path_len)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Move a file if and only if it was created by an upload */
CRX_FUNCTION(move_uploaded_file)
{
	char *path, *new_path;
	size_t path_len, new_path_len;
	bool successful = 0;

#ifndef CRX_WIN32
	int oldmask; int ret;
#endif

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_STRING(path, path_len)
		C_PARAM_PATH(new_path, new_path_len)
	CREX_PARSE_PARAMETERS_END();

	if (!SG(rfc1867_uploaded_files)) {
		RETURN_FALSE;
	}

	if (!crex_hash_str_exists(SG(rfc1867_uploaded_files), path, path_len)) {
		RETURN_FALSE;
	}

	if (crx_check_open_basedir(new_path)) {
		RETURN_FALSE;
	}

	if (VCWD_RENAME(path, new_path) == 0) {
		successful = 1;
#ifndef CRX_WIN32
		oldmask = umask(077);
		umask(oldmask);

		ret = VCWD_CHMOD(new_path, 0666 & ~oldmask);

		if (ret == -1) {
			crx_error_docref(NULL, E_WARNING, "%s", strerror(errno));
		}
#endif
	} else if (crx_copy_file_ex(path, new_path, STREAM_DISABLE_OPEN_BASEDIR) == SUCCESS) {
		VCWD_UNLINK(path);
		successful = 1;
	}

	if (successful) {
		crex_hash_str_del(SG(rfc1867_uploaded_files), path, path_len);
	} else {
		crx_error_docref(NULL, E_WARNING, "Unable to move \"%s\" to \"%s\"", path, new_path);
	}

	RETURN_BOOL(successful);
}
/* }}} */

/* {{{ crx_simple_ini_parser_cb */
static void crx_simple_ini_parser_cb(zval *arg1, zval *arg2, zval *arg3, int callback_type, zval *arr)
{
	switch (callback_type) {

		case CREX_INI_PARSER_ENTRY:
			if (!arg2) {
				/* bare string - nothing to do */
				break;
			}
			C_TRY_ADDREF_P(arg2);
			crex_symtable_update(C_ARRVAL_P(arr), C_STR_P(arg1), arg2);
			break;

		case CREX_INI_PARSER_POP_ENTRY:
		{
			zval hash, *find_hash;

			if (!arg2) {
				/* bare string - nothing to do */
				break;
			}

			/* entry in the form x[a]=b where x might need to be an array index */
			if (!(C_STRLEN_P(arg1) > 1 && C_STRVAL_P(arg1)[0] == '0') && is_numeric_string(C_STRVAL_P(arg1), C_STRLEN_P(arg1), NULL, NULL, 0) == IS_LONG) {
				crex_ulong key = (crex_ulong) CREX_STRTOUL(C_STRVAL_P(arg1), NULL, 0);
				if ((find_hash = crex_hash_index_find(C_ARRVAL_P(arr), key)) == NULL) {
					array_init(&hash);
					find_hash = crex_hash_index_add_new(C_ARRVAL_P(arr), key, &hash);
				}
			} else {
				if ((find_hash = crex_hash_find(C_ARRVAL_P(arr), C_STR_P(arg1))) == NULL) {
					array_init(&hash);
					find_hash = crex_hash_add_new(C_ARRVAL_P(arr), C_STR_P(arg1), &hash);
				}
			}

			if (C_TYPE_P(find_hash) != IS_ARRAY) {
				zval_ptr_dtor_nogc(find_hash);
				array_init(find_hash);
			}

			if (!arg3 || (C_TYPE_P(arg3) == IS_STRING && C_STRLEN_P(arg3) == 0)) {
				C_TRY_ADDREF_P(arg2);
				add_next_index_zval(find_hash, arg2);
			} else {
				array_set_zval_key(C_ARRVAL_P(find_hash), arg3, arg2);
			}
		}
		break;

		case CREX_INI_PARSER_SECTION:
			break;
	}
}
/* }}} */

/* {{{ crx_ini_parser_cb_with_sections */
static void crx_ini_parser_cb_with_sections(zval *arg1, zval *arg2, zval *arg3, int callback_type, zval *arr)
{
	if (callback_type == CREX_INI_PARSER_SECTION) {
		array_init(&BG(active_ini_file_section));
		crex_symtable_update(C_ARRVAL_P(arr), C_STR_P(arg1), &BG(active_ini_file_section));
	} else if (arg2) {
		zval *active_arr;

		if (C_TYPE(BG(active_ini_file_section)) != IS_UNDEF) {
			active_arr = &BG(active_ini_file_section);
		} else {
			active_arr = arr;
		}

		crx_simple_ini_parser_cb(arg1, arg2, arg3, callback_type, active_arr);
	}
}
/* }}} */

/* {{{ Parse configuration file */
CRX_FUNCTION(parse_ini_file)
{
	crex_string *filename = NULL;
	bool process_sections = 0;
	crex_long scanner_mode = CREX_INI_SCANNER_NORMAL;
	crex_file_handle fh;
	crex_ini_parser_cb_t ini_parser_cb;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_PATH_STR(filename)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(process_sections)
		C_PARAM_LONG(scanner_mode)
	CREX_PARSE_PARAMETERS_END();

	if (ZSTR_LEN(filename) == 0) {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	}

	/* Set callback function */
	if (process_sections) {
		ZVAL_UNDEF(&BG(active_ini_file_section));
		ini_parser_cb = (crex_ini_parser_cb_t) crx_ini_parser_cb_with_sections;
	} else {
		ini_parser_cb = (crex_ini_parser_cb_t) crx_simple_ini_parser_cb;
	}

	/* Setup filehandle */
	crex_stream_init_filename_ex(&fh, filename);

	array_init(return_value);
	if (crex_parse_ini_file(&fh, 0, (int)scanner_mode, ini_parser_cb, return_value) == FAILURE) {
		crex_array_destroy(C_ARR_P(return_value));
		RETVAL_FALSE;
	}
	crex_destroy_file_handle(&fh);
}
/* }}} */

/* {{{ Parse configuration string */
CRX_FUNCTION(parse_ini_string)
{
	char *string = NULL, *str = NULL;
	size_t str_len = 0;
	bool process_sections = 0;
	crex_long scanner_mode = CREX_INI_SCANNER_NORMAL;
	crex_ini_parser_cb_t ini_parser_cb;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_STRING(str, str_len)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(process_sections)
		C_PARAM_LONG(scanner_mode)
	CREX_PARSE_PARAMETERS_END();

	if (INT_MAX - str_len < CREX_MMAP_AHEAD) {
		RETVAL_FALSE;
	}

	/* Set callback function */
	if (process_sections) {
		ZVAL_UNDEF(&BG(active_ini_file_section));
		ini_parser_cb = (crex_ini_parser_cb_t) crx_ini_parser_cb_with_sections;
	} else {
		ini_parser_cb = (crex_ini_parser_cb_t) crx_simple_ini_parser_cb;
	}

	/* Setup string */
	string = (char *) emalloc(str_len + CREX_MMAP_AHEAD);
	memcpy(string, str, str_len);
	memset(string + str_len, 0, CREX_MMAP_AHEAD);

	array_init(return_value);
	if (crex_parse_ini_string(string, 0, (int)scanner_mode, ini_parser_cb, return_value) == FAILURE) {
		crex_array_destroy(C_ARR_P(return_value));
		RETVAL_FALSE;
	}
	efree(string);
}
/* }}} */

#if CREX_DEBUG
/* This function returns an array of ALL valid ini options with values and
 *  is not the same as ini_get_all() which returns only registered ini options. Only useful for devs to debug crx.ini scanner/parser! */
CRX_FUNCTION(config_get_hash) /* {{{ */
{
	CREX_PARSE_PARAMETERS_NONE();

	HashTable *hash = crx_ini_get_configuration_hash();

	array_init(return_value);
	add_config_entries(hash, return_value);
}
/* }}} */
#endif

#ifdef HAVE_GETLOADAVG
/* {{{ */
CRX_FUNCTION(sys_getloadavg)
{
	double load[3];

	CREX_PARSE_PARAMETERS_NONE();

	if (getloadavg(load, 3) == -1) {
		RETURN_FALSE;
	} else {
		array_init(return_value);
		add_index_double(return_value, 0, load[0]);
		add_index_double(return_value, 1, load[1]);
		add_index_double(return_value, 2, load[2]);
	}
}
/* }}} */
#endif
