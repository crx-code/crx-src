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
   |          Rasmus Lerdorf <rasmus@lerdorf.on.ca>                       |
   |          Zeev Suraski <zeev@crx.net>                                 |
   +----------------------------------------------------------------------+
*/

/* {{{ includes */

#define CREX_INCLUDE_FULL_WINDOWS_HEADERS

#include "crx.h"
#include <stdio.h>
#include <fcntl.h>
#ifdef CRX_WIN32
#include "win32/time.h"
#include "win32/signal.h"
#include "win32/crx_win32_globals.h"
#include "win32/winutil.h"
#include <process.h>
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <signal.h>
#include <locale.h>
#include "crex.h"
#include "crex_types.h"
#include "crex_extensions.h"
#include "crx_ini.h"
#include "crx_globals.h"
#include "crx_main.h"
#include "crx_syslog.h"
#include "fopen_wrappers.h"
#include "ext/standard/crx_standard.h"
#include "ext/date/crx_date.h"
#include "crx_variables.h"
#include "ext/standard/credits.h"
#ifdef CRX_WIN32
#include <io.h>
#include "win32/crx_registry.h"
#include "ext/standard/flock_compat.h"
#endif
#include "crx_syslog.h"
#include "Crex/crex_exceptions.h"

#if CRX_SIGCHILD
#include <sys/types.h>
#include <sys/wait.h>
#endif

#include "crex_compile.h"
#include "crex_execute.h"
#include "crex_highlight.h"
#include "crex_extensions.h"
#include "crex_ini.h"
#include "crex_dtrace.h"
#include "crex_observer.h"
#include "crex_system_id.h"

#include "crx_content_types.h"
#include "crx_ticks.h"
#include "crx_streams.h"
#include "crx_open_temporary_file.h"

#include "SAPI.h"
#include "rfc1867.h"

#include "ext/standard/html_tables.h"
#include "main_arginfo.h"
/* }}} */

CRXAPI int (*crx_register_internal_extensions_func)(void) = crx_register_internal_extensions;

#ifndef ZTS
crx_core_globals core_globals;
#else
CRXAPI int core_globals_id;
CRXAPI size_t core_globals_offset;
#endif

#define SAFE_FILENAME(f) ((f)?(f):"-")

CRXAPI const char *crx_version(void)
{
	return CRX_VERSION;
}

CRXAPI unsigned int crx_version_id(void)
{
	return CRX_VERSION_ID;
}

/* {{{ CRX_INI_MH */
static CRX_INI_MH(OnSetFacility)
{
	const crex_string *facility = new_value;

#ifdef LOG_AUTH
	if (crex_string_equals_literal(facility, "LOG_AUTH") || crex_string_equals_literal(facility, "auth")
			|| crex_string_equals_literal(facility, "security")) {
		PG(syslog_facility) = LOG_AUTH;
		return SUCCESS;
	}
#endif
#ifdef LOG_AUTHPRIV
	if (crex_string_equals_literal(facility, "LOG_AUTHPRIV") || crex_string_equals_literal(facility, "authpriv")) {
		PG(syslog_facility) = LOG_AUTHPRIV;
		return SUCCESS;
	}
#endif
#ifdef LOG_CRON
	if (crex_string_equals_literal(facility, "LOG_CRON") || crex_string_equals_literal(facility, "cron")) {
		PG(syslog_facility) = LOG_CRON;
		return SUCCESS;
	}
#endif
#ifdef LOG_DAEMON
	if (crex_string_equals_literal(facility, "LOG_DAEMON") || crex_string_equals_literal(facility, "daemon")) {
		PG(syslog_facility) = LOG_DAEMON;
		return SUCCESS;
	}
#endif
#ifdef LOG_FTP
	if (crex_string_equals_literal(facility, "LOG_FTP") || crex_string_equals_literal(facility, "ftp")) {
		PG(syslog_facility) = LOG_FTP;
		return SUCCESS;
	}
#endif
#ifdef LOG_KERN
	if (crex_string_equals_literal(facility, "LOG_KERN") || crex_string_equals_literal(facility, "kern")) {
		PG(syslog_facility) = LOG_KERN;
		return SUCCESS;
	}
#endif
#ifdef LOG_LPR
	if (crex_string_equals_literal(facility, "LOG_LPR") || crex_string_equals_literal(facility, "lpr")) {
		PG(syslog_facility) = LOG_LPR;
		return SUCCESS;
	}
#endif
#ifdef LOG_MAIL
	if (crex_string_equals_literal(facility, "LOG_MAIL") || crex_string_equals_literal(facility, "mail")) {
		PG(syslog_facility) = LOG_MAIL;
		return SUCCESS;
	}
#endif
#ifdef LOG_INTERNAL_MARK
	if (crex_string_equals_literal(facility, "LOG_INTERNAL_MARK") || crex_string_equals_literal(facility, "mark")) {
		PG(syslog_facility) = LOG_INTERNAL_MARK;
		return SUCCESS;
	}
#endif
#ifdef LOG_NEWS
	if (crex_string_equals_literal(facility, "LOG_NEWS") || crex_string_equals_literal(facility, "news")) {
		PG(syslog_facility) = LOG_NEWS;
		return SUCCESS;
	}
#endif
#ifdef LOG_SYSLOG
	if (crex_string_equals_literal(facility, "LOG_SYSLOG") || crex_string_equals_literal(facility, "syslog")) {
		PG(syslog_facility) = LOG_SYSLOG;
		return SUCCESS;
	}
#endif
#ifdef LOG_USER
	if (crex_string_equals(facility, ZSTR_KNOWN(CREX_STR_USER)) || crex_string_equals_literal(facility, "LOG_USER")) {
		PG(syslog_facility) = LOG_USER;
		return SUCCESS;
	}
#endif
#ifdef LOG_UUCP
	if (crex_string_equals_literal(facility, "LOG_UUCP") || crex_string_equals_literal(facility, "uucp")) {
		PG(syslog_facility) = LOG_UUCP;
		return SUCCESS;
	}
#endif
#ifdef LOG_LOCAL0
	if (crex_string_equals_literal(facility, "LOG_LOCAL0") || crex_string_equals_literal(facility, "local0")) {
		PG(syslog_facility) = LOG_LOCAL0;
		return SUCCESS;
	}
#endif
#ifdef LOG_LOCAL1
	if (crex_string_equals_literal(facility, "LOG_LOCAL1") || crex_string_equals_literal(facility, "local1")) {
		PG(syslog_facility) = LOG_LOCAL1;
		return SUCCESS;
	}
#endif
#ifdef LOG_LOCAL2
	if (crex_string_equals_literal(facility, "LOG_LOCAL2") || crex_string_equals_literal(facility, "local2")) {
		PG(syslog_facility) = LOG_LOCAL2;
		return SUCCESS;
	}
#endif
#ifdef LOG_LOCAL3
	if (crex_string_equals_literal(facility, "LOG_LOCAL3") || crex_string_equals_literal(facility, "local3")) {
		PG(syslog_facility) = LOG_LOCAL3;
		return SUCCESS;
	}
#endif
#ifdef LOG_LOCAL4
	if (crex_string_equals_literal(facility, "LOG_LOCAL4") || crex_string_equals_literal(facility, "local4")) {
		PG(syslog_facility) = LOG_LOCAL4;
		return SUCCESS;
	}
#endif
#ifdef LOG_LOCAL5
	if (crex_string_equals_literal(facility, "LOG_LOCAL5") || crex_string_equals_literal(facility, "local5")) {
		PG(syslog_facility) = LOG_LOCAL5;
		return SUCCESS;
	}
#endif
#ifdef LOG_LOCAL6
	if (crex_string_equals_literal(facility, "LOG_LOCAL6") || crex_string_equals_literal(facility, "local6")) {
		PG(syslog_facility) = LOG_LOCAL6;
		return SUCCESS;
	}
#endif
#ifdef LOG_LOCAL7
	if (crex_string_equals_literal(facility, "LOG_LOCAL7") || crex_string_equals_literal(facility, "local7")) {
		PG(syslog_facility) = LOG_LOCAL7;
		return SUCCESS;
	}
#endif

	return FAILURE;
}
/* }}} */

/* {{{ CRX_INI_MH */
static CRX_INI_MH(OnSetPrecision)
{
	crex_long i = CREX_ATOL(ZSTR_VAL(new_value));
	if (i >= -1) {
		EG(precision) = i;
		return SUCCESS;
	} else {
		return FAILURE;
	}
}
/* }}} */

/* {{{ CRX_INI_MH */
static CRX_INI_MH(OnSetSerializePrecision)
{
	crex_long i = CREX_ATOL(ZSTR_VAL(new_value));
	if (i >= -1) {
		PG(serialize_precision) = i;
		return SUCCESS;
	} else {
		return FAILURE;
	}
}
/* }}} */

/* {{{ CRX_INI_MH */
static CRX_INI_MH(OnChangeMemoryLimit)
{
	size_t value;
	if (new_value) {
		value = crex_ini_parse_uquantity_warn(new_value, entry->name);
	} else {
		value = C_L(1)<<30;		/* effectively, no limit */
	}
	if (crex_set_memory_limit(value) == FAILURE) {
		/* When the memory limit is reset to the original level during deactivation, we may be
		 * using more memory than the original limit while shutdown is still in progress.
		 * Ignore a failure for now, and set the memory limit when the memory manager has been
		 * shut down and the minimal amount of memory is used. */
		if (stage != CREX_INI_STAGE_DEACTIVATE) {
			crex_error(E_WARNING, "Failed to set memory limit to %zd bytes (Current memory usage is %zd bytes)", value, crex_memory_usage(true));
			return FAILURE;
		}
	}
	PG(memory_limit) = value;
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_INI_MH */
static CRX_INI_MH(OnSetLogFilter)
{
	const crex_string *filter = new_value;

	if (crex_string_equals_literal(filter, "all")) {
		PG(syslog_filter) = CRX_SYSLOG_FILTER_ALL;
		return SUCCESS;
	}
	if (crex_string_equals_literal(filter, "no-ctrl")) {
		PG(syslog_filter) = CRX_SYSLOG_FILTER_NO_CTRL;
		return SUCCESS;
	}
	if (crex_string_equals_literal(filter, "ascii")) {
		PG(syslog_filter) = CRX_SYSLOG_FILTER_ASCII;
		return SUCCESS;
	}
	if (crex_string_equals_literal(filter, "raw")) {
		PG(syslog_filter) = CRX_SYSLOG_FILTER_RAW;
		return SUCCESS;
	}

	return FAILURE;
}
/* }}} */

/* {{{ crx_disable_classes */
static void crx_disable_classes(void)
{
	char *s = NULL, *e;

	if (!*(INI_STR("disable_classes"))) {
		return;
	}

	e = PG(disable_classes) = strdup(INI_STR("disable_classes"));

	while (*e) {
		switch (*e) {
			case ' ':
			case ',':
				if (s) {
					*e = '\0';
					crex_disable_class(s, e-s);
					s = NULL;
				}
				break;
			default:
				if (!s) {
					s = e;
				}
				break;
		}
		e++;
	}
	if (s) {
		crex_disable_class(s, e-s);
	}
}
/* }}} */

/* {{{ crx_binary_init */
static void crx_binary_init(void)
{
	char *binary_location = NULL;
#ifdef CRX_WIN32
	binary_location = (char *)pemalloc(MAXPATHLEN, 1);
	if (GetModuleFileName(0, binary_location, MAXPATHLEN) == 0) {
		pefree(binary_location, 1);
		binary_location = NULL;
	}
#else
	if (sapi_module.executable_location) {
		binary_location = (char *)pemalloc(MAXPATHLEN, 1);
		if (!strchr(sapi_module.executable_location, '/')) {
			char *envpath, *path;
			bool found = false;

			if ((envpath = getenv("PATH")) != NULL) {
				char *search_dir, search_path[MAXPATHLEN];
				char *last = NULL;
				crex_stat_t s = {0};

				path = estrdup(envpath);
				search_dir = crx_strtok_r(path, ":", &last);

				while (search_dir) {
					snprintf(search_path, MAXPATHLEN, "%s/%s", search_dir, sapi_module.executable_location);
					if (VCWD_REALPATH(search_path, binary_location) && !VCWD_ACCESS(binary_location, X_OK) && VCWD_STAT(binary_location, &s) == 0 && S_ISREG(s.st_mode)) {
						found = true;
						break;
					}
					search_dir = crx_strtok_r(NULL, ":", &last);
				}
				efree(path);
			}
			if (!found) {
				pefree(binary_location, 1);
				binary_location = NULL;
			}
		} else if (!VCWD_REALPATH(sapi_module.executable_location, binary_location) || VCWD_ACCESS(binary_location, X_OK)) {
			pefree(binary_location, 1);
			binary_location = NULL;
		}
	}
#endif
	PG(crx_binary) = binary_location;
}
/* }}} */

/* {{{ CRX_INI_MH */
static CRX_INI_MH(OnUpdateTimeout)
{
	if (stage==CRX_INI_STAGE_STARTUP) {
		/* Don't set a timeout on startup, only per-request */
		EG(timeout_seconds) = CREX_ATOL(ZSTR_VAL(new_value));
		return SUCCESS;
	}
	crex_unset_timeout();
	EG(timeout_seconds) = CREX_ATOL(ZSTR_VAL(new_value));
	if (stage != CRX_INI_STAGE_DEACTIVATE) {
		/*
		 * If we're restoring INI values, we shouldn't reset the timer.
		 * Otherwise, the timer is active when CRX is idle, such as the
		 * CLI web server or CGI. Running a script will re-activate
		 * the timeout, so it's not needed to do so at script end.
		 */
		crex_set_timeout(EG(timeout_seconds), 0);
	}
	return SUCCESS;
}
/* }}} */

/* {{{ crx_get_display_errors_mode() helper function */
static uint8_t crx_get_display_errors_mode(crex_string *value)
{
	if (!value) {
		return CRX_DISPLAY_ERRORS_STDOUT;
	}

	if (crex_string_equals_literal_ci(value, "on")) {
		return CRX_DISPLAY_ERRORS_STDOUT;
	}
	if (crex_string_equals_literal_ci(value, "yes")) {
		return CRX_DISPLAY_ERRORS_STDOUT;
	}

	if (crex_string_equals_literal_ci(value, "true")) {
		return CRX_DISPLAY_ERRORS_STDOUT;
	}
	if (crex_string_equals_literal_ci(value, "stderr")) {
		return CRX_DISPLAY_ERRORS_STDERR;
	}
	if (crex_string_equals_literal_ci(value, "stdout")) {
		return CRX_DISPLAY_ERRORS_STDOUT;
	}

	uint8_t mode = CREX_ATOL(ZSTR_VAL(value));
	if (mode && mode != CRX_DISPLAY_ERRORS_STDOUT && mode != CRX_DISPLAY_ERRORS_STDERR) {
		return CRX_DISPLAY_ERRORS_STDOUT;
	}

	return mode;
}
/* }}} */

/* {{{ CRX_INI_MH */
static CRX_INI_MH(OnUpdateDisplayErrors)
{
	PG(display_errors) = crx_get_display_errors_mode(new_value);

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_INI_DISP */
static CRX_INI_DISP(display_errors_mode)
{
	uint8_t mode;
	bool cgi_or_cli;
	crex_string *temporary_value;

	if (type == CREX_INI_DISPLAY_ORIG && ini_entry->modified) {
		temporary_value = (ini_entry->orig_value ? ini_entry->orig_value : NULL );
	} else if (ini_entry->value) {
		temporary_value = ini_entry->value;
	} else {
		temporary_value = NULL;
	}

	mode = crx_get_display_errors_mode(temporary_value);

	/* Display 'On' for other SAPIs instead of STDOUT or STDERR */
	cgi_or_cli = (!strcmp(sapi_module.name, "cli") || !strcmp(sapi_module.name, "cgi") || !strcmp(sapi_module.name, "crxdbg"));

	switch (mode) {
		case CRX_DISPLAY_ERRORS_STDERR:
			if (cgi_or_cli ) {
				PUTS("STDERR");
			} else {
				PUTS("On");
			}
			break;

		case CRX_DISPLAY_ERRORS_STDOUT:
			if (cgi_or_cli ) {
				PUTS("STDOUT");
			} else {
				PUTS("On");
			}
			break;

		default:
			PUTS("Off");
			break;
	}
}
/* }}} */

CRXAPI const char *crx_get_internal_encoding(void) {
	if (PG(internal_encoding) && PG(internal_encoding)[0]) {
		return PG(internal_encoding);
	} else if (SG(default_charset) && SG(default_charset)[0]) {
		return SG(default_charset);
	}
	return "UTF-8";
}

CRXAPI const char *crx_get_input_encoding(void) {
	if (PG(input_encoding) && PG(input_encoding)[0]) {
		return PG(input_encoding);
	} else if (SG(default_charset) && SG(default_charset)[0]) {
		return SG(default_charset);
	}
	return "UTF-8";
}

CRXAPI const char *crx_get_output_encoding(void) {
	if (PG(output_encoding) && PG(output_encoding)[0]) {
		return PG(output_encoding);
	} else if (SG(default_charset) && SG(default_charset)[0]) {
		return SG(default_charset);
	}
	return "UTF-8";
}

CRXAPI void (*crx_internal_encoding_changed)(void) = NULL;

/* {{{ CRX_INI_MH */
static CRX_INI_MH(OnUpdateDefaultCharset)
{
	if (memchr(ZSTR_VAL(new_value), '\0', ZSTR_LEN(new_value))
		|| strpbrk(ZSTR_VAL(new_value), "\r\n")) {
		return FAILURE;
	}
	OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
	if (crx_internal_encoding_changed) {
		crx_internal_encoding_changed();
	}
	if (new_value) {
#ifdef CRX_WIN32
		crx_win32_cp_do_update(ZSTR_VAL(new_value));
#endif
	}
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_INI_MH */
static CRX_INI_MH(OnUpdateDefaultMimeTye)
{
	if (memchr(ZSTR_VAL(new_value), '\0', ZSTR_LEN(new_value))
		|| strpbrk(ZSTR_VAL(new_value), "\r\n")) {
		return FAILURE;
	}
	return OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
}
/* }}} */

/* {{{ CRX_INI_MH */
static CRX_INI_MH(OnUpdateInternalEncoding)
{
	OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
	if (crx_internal_encoding_changed) {
		crx_internal_encoding_changed();
	}
	if (new_value) {
#ifdef CRX_WIN32
		crx_win32_cp_do_update(ZSTR_VAL(new_value));
#endif
	}
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_INI_MH */
static CRX_INI_MH(OnUpdateInputEncoding)
{
	OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
	if (crx_internal_encoding_changed) {
		crx_internal_encoding_changed();
	}
	if (new_value) {
#ifdef CRX_WIN32
		crx_win32_cp_do_update(NULL);
#endif
	}
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_INI_MH */
static CRX_INI_MH(OnUpdateOutputEncoding)
{
	OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
	if (crx_internal_encoding_changed) {
		crx_internal_encoding_changed();
	}
	if (new_value) {
#ifdef CRX_WIN32
		crx_win32_cp_do_update(NULL);
#endif
	}
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_INI_MH */
static CRX_INI_MH(OnUpdateErrorLog)
{
	/* Only do the safemode/open_basedir check at runtime */
	if ((stage == CRX_INI_STAGE_RUNTIME || stage == CRX_INI_STAGE_HTACCESS) &&
			new_value && crex_string_equals_literal(new_value, "syslog")) {
		if (PG(open_basedir) && crx_check_open_basedir(ZSTR_VAL(new_value))) {
			return FAILURE;
		}
	}
	OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_INI_MH */
static CRX_INI_MH(OnUpdateMailLog)
{
	/* Only do the safemode/open_basedir check at runtime */
	if ((stage == CRX_INI_STAGE_RUNTIME || stage == CRX_INI_STAGE_HTACCESS) && new_value) {
		if (PG(open_basedir) && crx_check_open_basedir(ZSTR_VAL(new_value))) {
			return FAILURE;
		}
	}
	OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_INI_MH */
static CRX_INI_MH(OnChangeMailForceExtra)
{
	/* Don't allow changing it in htaccess */
	if (stage == CRX_INI_STAGE_HTACCESS) {
			return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

/* defined in browscap.c */
CRX_INI_MH(OnChangeBrowscap);


/* Need to be read from the environment (?):
 * CRX_AUTO_PREPEND_FILE
 * CRX_AUTO_APPEND_FILE
 * CRX_DOCUMENT_ROOT
 * CRX_USER_DIR
 * CRX_INCLUDE_PATH
 */

 /* Windows use the internal mail */
#if defined(CRX_WIN32)
# define DEFAULT_SENDMAIL_PATH NULL
#else
# define DEFAULT_SENDMAIL_PATH CRX_PROG_SENDMAIL " -t -i"
#endif

/* {{{ CRX_INI */
CRX_INI_BEGIN()
	CRX_INI_ENTRY_EX("highlight.comment",		HL_COMMENT_COLOR,	CRX_INI_ALL,	NULL,			crx_ini_color_displayer_cb)
	CRX_INI_ENTRY_EX("highlight.default",		HL_DEFAULT_COLOR,	CRX_INI_ALL,	NULL,			crx_ini_color_displayer_cb)
	CRX_INI_ENTRY_EX("highlight.html",			HL_HTML_COLOR,		CRX_INI_ALL,	NULL,			crx_ini_color_displayer_cb)
	CRX_INI_ENTRY_EX("highlight.keyword",		HL_KEYWORD_COLOR,	CRX_INI_ALL,	NULL,			crx_ini_color_displayer_cb)
	CRX_INI_ENTRY_EX("highlight.string",		HL_STRING_COLOR,	CRX_INI_ALL,	NULL,			crx_ini_color_displayer_cb)

	STD_CRX_INI_ENTRY_EX("display_errors",		"1",		CRX_INI_ALL,		OnUpdateDisplayErrors,	display_errors,			crx_core_globals,	core_globals, display_errors_mode)
	STD_CRX_INI_BOOLEAN("display_startup_errors",	"1",	CRX_INI_ALL,		OnUpdateBool,			display_startup_errors,	crx_core_globals,	core_globals)
	STD_CRX_INI_BOOLEAN("enable_dl",			"1",		CRX_INI_SYSTEM,		OnUpdateBool,			enable_dl,				crx_core_globals,	core_globals)
	STD_CRX_INI_BOOLEAN("expose_crx",			"1",		CRX_INI_SYSTEM,		OnUpdateBool,			expose_crx,				crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("docref_root", 			"", 		CRX_INI_ALL,		OnUpdateString,			docref_root,			crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("docref_ext",				"",			CRX_INI_ALL,		OnUpdateString,			docref_ext,				crx_core_globals,	core_globals)
	STD_CRX_INI_BOOLEAN("html_errors",			"1",		CRX_INI_ALL,		OnUpdateBool,			html_errors,			crx_core_globals,	core_globals)
	STD_CRX_INI_BOOLEAN("xmlrpc_errors",		"0",		CRX_INI_SYSTEM,		OnUpdateBool,			xmlrpc_errors,			crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("xmlrpc_error_number",	"0",		CRX_INI_ALL,		OnUpdateLong,			xmlrpc_error_number,	crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("max_input_time",			"-1",	CRX_INI_SYSTEM|CRX_INI_PERDIR,		OnUpdateLong,			max_input_time,	crx_core_globals,	core_globals)
	STD_CRX_INI_BOOLEAN("ignore_user_abort",	"0",		CRX_INI_ALL,		OnUpdateBool,			ignore_user_abort,		crx_core_globals,	core_globals)
	STD_CRX_INI_BOOLEAN("implicit_flush",		"0",		CRX_INI_ALL,		OnUpdateBool,			implicit_flush,			crx_core_globals,	core_globals)
	STD_CRX_INI_BOOLEAN("log_errors",			"0",		CRX_INI_ALL,		OnUpdateBool,			log_errors,				crx_core_globals,	core_globals)
	STD_CRX_INI_BOOLEAN("ignore_repeated_errors",	"0",	CRX_INI_ALL,		OnUpdateBool,			ignore_repeated_errors,	crx_core_globals,	core_globals)
	STD_CRX_INI_BOOLEAN("ignore_repeated_source",	"0",	CRX_INI_ALL,		OnUpdateBool,			ignore_repeated_source,	crx_core_globals,	core_globals)
	STD_CRX_INI_BOOLEAN("report_memleaks",		"1",		CRX_INI_ALL,		OnUpdateBool,			report_memleaks,		crx_core_globals,	core_globals)
	STD_CRX_INI_BOOLEAN("report_crex_debug",	"0",		CRX_INI_ALL,		OnUpdateBool,			report_crex_debug,		crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("output_buffering",		"0",		CRX_INI_PERDIR|CRX_INI_SYSTEM,	OnUpdateLong,	output_buffering,		crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("output_handler",			NULL,		CRX_INI_PERDIR|CRX_INI_SYSTEM,	OnUpdateString,	output_handler,		crx_core_globals,	core_globals)
	STD_CRX_INI_BOOLEAN("register_argc_argv",	"1",		CRX_INI_PERDIR|CRX_INI_SYSTEM,	OnUpdateBool,	register_argc_argv,		crx_core_globals,	core_globals)
	STD_CRX_INI_BOOLEAN("auto_globals_jit",		"1",		CRX_INI_PERDIR|CRX_INI_SYSTEM,	OnUpdateBool,	auto_globals_jit,	crx_core_globals,	core_globals)
	STD_CRX_INI_BOOLEAN("short_open_tag",	DEFAULT_SHORT_OPEN_TAG,	CRX_INI_SYSTEM|CRX_INI_PERDIR,		OnUpdateBool,			short_tags,				crex_compiler_globals,	compiler_globals)

	STD_CRX_INI_ENTRY("unserialize_callback_func",	NULL,	CRX_INI_ALL,		OnUpdateString,			unserialize_callback_func,	crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("serialize_precision",	"-1",	CRX_INI_ALL,		OnSetSerializePrecision,			serialize_precision,	crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("arg_separator.output",	"&",		CRX_INI_ALL,		OnUpdateStringUnempty,	arg_separator.output,	crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("arg_separator.input",	"&",		CRX_INI_SYSTEM|CRX_INI_PERDIR,	OnUpdateStringUnempty,	arg_separator.input,	crx_core_globals,	core_globals)

	STD_CRX_INI_ENTRY("auto_append_file",		NULL,		CRX_INI_SYSTEM|CRX_INI_PERDIR,		OnUpdateString,			auto_append_file,		crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("auto_prepend_file",		NULL,		CRX_INI_SYSTEM|CRX_INI_PERDIR,		OnUpdateString,			auto_prepend_file,		crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("doc_root",				NULL,		CRX_INI_SYSTEM,		OnUpdateStringUnempty,	doc_root,				crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("default_charset",		CRX_DEFAULT_CHARSET,	CRX_INI_ALL,	OnUpdateDefaultCharset,			default_charset,		sapi_globals_struct, sapi_globals)
	STD_CRX_INI_ENTRY("default_mimetype",		SAPI_DEFAULT_MIMETYPE,	CRX_INI_ALL,	OnUpdateDefaultMimeTye,			default_mimetype,		sapi_globals_struct, sapi_globals)
	STD_CRX_INI_ENTRY("internal_encoding",		NULL,			CRX_INI_ALL,	OnUpdateInternalEncoding,	internal_encoding,	crx_core_globals, core_globals)
	STD_CRX_INI_ENTRY("input_encoding",			NULL,			CRX_INI_ALL,	OnUpdateInputEncoding,				input_encoding,		crx_core_globals, core_globals)
	STD_CRX_INI_ENTRY("output_encoding",		NULL,			CRX_INI_ALL,	OnUpdateOutputEncoding,				output_encoding,	crx_core_globals, core_globals)
	STD_CRX_INI_ENTRY("error_log",				NULL,			CRX_INI_ALL,		OnUpdateErrorLog,				error_log,				crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("error_log_mode",			"0644",			CRX_INI_ALL,		OnUpdateLong,					error_log_mode,			crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("extension_dir",			CRX_EXTENSION_DIR,		CRX_INI_SYSTEM,		OnUpdateStringUnempty,	extension_dir,			crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("sys_temp_dir",			NULL,		CRX_INI_SYSTEM,		OnUpdateStringUnempty,	sys_temp_dir,			crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("include_path",			CRX_INCLUDE_PATH,		CRX_INI_ALL,		OnUpdateStringUnempty,	include_path,			crx_core_globals,	core_globals)
	CRX_INI_ENTRY("max_execution_time",			"30",		CRX_INI_ALL,			OnUpdateTimeout)
	STD_CRX_INI_ENTRY("open_basedir",			NULL,		CRX_INI_ALL,		OnUpdateBaseDir,			open_basedir,			crx_core_globals,	core_globals)

	STD_CRX_INI_BOOLEAN("file_uploads",			"1",		CRX_INI_SYSTEM,		OnUpdateBool,			file_uploads,			crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("upload_max_filesize",	"2M",		CRX_INI_SYSTEM|CRX_INI_PERDIR,		OnUpdateLong,			upload_max_filesize,	crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("post_max_size",			"8M",		CRX_INI_SYSTEM|CRX_INI_PERDIR,		OnUpdateLong,			post_max_size,			sapi_globals_struct,sapi_globals)
	STD_CRX_INI_ENTRY("upload_tmp_dir",			NULL,		CRX_INI_SYSTEM,		OnUpdateStringUnempty,	upload_tmp_dir,			crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("max_input_nesting_level", "64",		CRX_INI_SYSTEM|CRX_INI_PERDIR,		OnUpdateLongGEZero,	max_input_nesting_level,			crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("max_input_vars",			"1000",		CRX_INI_SYSTEM|CRX_INI_PERDIR,		OnUpdateLongGEZero,	max_input_vars,						crx_core_globals,	core_globals)

	STD_CRX_INI_ENTRY("user_dir",				NULL,		CRX_INI_SYSTEM,		OnUpdateString,			user_dir,				crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("variables_order",		"EGPCS",	CRX_INI_SYSTEM|CRX_INI_PERDIR,		OnUpdateStringUnempty,	variables_order,		crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("request_order",			NULL,		CRX_INI_SYSTEM|CRX_INI_PERDIR,		OnUpdateString,	request_order,		crx_core_globals,	core_globals)

	STD_CRX_INI_ENTRY("error_append_string",	NULL,		CRX_INI_ALL,		OnUpdateString,			error_append_string,	crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("error_prepend_string",	NULL,		CRX_INI_ALL,		OnUpdateString,			error_prepend_string,	crx_core_globals,	core_globals)

	CRX_INI_ENTRY("SMTP",						"localhost",CRX_INI_ALL,		NULL)
	CRX_INI_ENTRY("smtp_port",					"25",		CRX_INI_ALL,		NULL)
	STD_CRX_INI_BOOLEAN("mail.add_x_header",			"0",		CRX_INI_SYSTEM|CRX_INI_PERDIR,		OnUpdateBool,			mail_x_header,			crx_core_globals,	core_globals)
	STD_CRX_INI_BOOLEAN("mail.mixed_lf_and_crlf",			"0",		CRX_INI_SYSTEM|CRX_INI_PERDIR,		OnUpdateBool,			mail_mixed_lf_and_crlf,			crx_core_globals,	core_globals)
	STD_CRX_INI_ENTRY("mail.log",					NULL,		CRX_INI_SYSTEM|CRX_INI_PERDIR,		OnUpdateMailLog,			mail_log,			crx_core_globals,	core_globals)
	CRX_INI_ENTRY("browscap",					NULL,		CRX_INI_SYSTEM,		OnChangeBrowscap)
	CRX_INI_ENTRY("memory_limit",				"128M",		CRX_INI_ALL,		OnChangeMemoryLimit)
	CRX_INI_ENTRY("precision",					"14",		CRX_INI_ALL,		OnSetPrecision)
	CRX_INI_ENTRY("sendmail_from",				NULL,		CRX_INI_ALL,		NULL)
	CRX_INI_ENTRY("sendmail_path",	DEFAULT_SENDMAIL_PATH,	CRX_INI_SYSTEM,		NULL)
	CRX_INI_ENTRY("mail.force_extra_parameters",NULL,		CRX_INI_SYSTEM|CRX_INI_PERDIR,		OnChangeMailForceExtra)
	CRX_INI_ENTRY("disable_functions",			"",			CRX_INI_SYSTEM,		NULL)
	CRX_INI_ENTRY("disable_classes",			"",			CRX_INI_SYSTEM,		NULL)
	CRX_INI_ENTRY("max_file_uploads",			"20",			CRX_INI_SYSTEM|CRX_INI_PERDIR,		NULL)
	CRX_INI_ENTRY("max_multipart_body_parts",	"-1",			CRX_INI_SYSTEM|CRX_INI_PERDIR,		NULL)

	STD_CRX_INI_BOOLEAN("allow_url_fopen",		"1",		CRX_INI_SYSTEM,		OnUpdateBool,		allow_url_fopen,		crx_core_globals,		core_globals)
	STD_CRX_INI_BOOLEAN("allow_url_include",	"0",		CRX_INI_SYSTEM,		OnUpdateBool,		allow_url_include,		crx_core_globals,		core_globals)
	STD_CRX_INI_BOOLEAN("enable_post_data_reading",	"1",	CRX_INI_SYSTEM|CRX_INI_PERDIR,	OnUpdateBool,	enable_post_data_reading,	crx_core_globals,	core_globals)

	STD_CRX_INI_ENTRY("realpath_cache_size",	"4096K",	CRX_INI_SYSTEM,		OnUpdateLong,	realpath_cache_size_limit,	virtual_cwd_globals,	cwd_globals)
	STD_CRX_INI_ENTRY("realpath_cache_ttl",		"120",		CRX_INI_SYSTEM,		OnUpdateLong,	realpath_cache_ttl,			virtual_cwd_globals,	cwd_globals)

	STD_CRX_INI_ENTRY("user_ini.filename",		".user.ini",	CRX_INI_SYSTEM,		OnUpdateString,		user_ini_filename,	crx_core_globals,		core_globals)
	STD_CRX_INI_ENTRY("user_ini.cache_ttl",		"300",			CRX_INI_SYSTEM,		OnUpdateLong,		user_ini_cache_ttl,	crx_core_globals,		core_globals)
	STD_CRX_INI_ENTRY("hard_timeout",			"2",			CRX_INI_SYSTEM,		OnUpdateLong,		hard_timeout,		crex_executor_globals,	executor_globals)
#ifdef CRX_WIN32
	STD_CRX_INI_BOOLEAN("windows.show_crt_warning",		"0",		CRX_INI_ALL,		OnUpdateBool,			windows_show_crt_warning,			crx_core_globals,	core_globals)
#endif
	STD_CRX_INI_ENTRY("syslog.facility",		"LOG_USER",		CRX_INI_SYSTEM,		OnSetFacility,		syslog_facility,	crx_core_globals,		core_globals)
	STD_CRX_INI_ENTRY("syslog.ident",		"crx",			CRX_INI_SYSTEM,		OnUpdateString,		syslog_ident,		crx_core_globals,		core_globals)
	STD_CRX_INI_ENTRY("syslog.filter",		"no-ctrl",		CRX_INI_ALL,		OnSetLogFilter,		syslog_filter,		crx_core_globals, 		core_globals)
CRX_INI_END()
/* }}} */

/* True globals (no need for thread safety */
/* But don't make them a single int bitfield */
static bool module_initialized = false;
static bool module_startup = true;
static bool module_shutdown = false;

/* {{{ crx_during_module_startup */
CRXAPI bool crx_during_module_startup(void)
{
	return module_startup;
}
/* }}} */

/* {{{ crx_during_module_shutdown */
CRXAPI bool crx_during_module_shutdown(void)
{
	return module_shutdown;
}
/* }}} */

/* {{{ crx_get_module_initialized */
CRXAPI bool crx_get_module_initialized(void)
{
	return module_initialized;
}
/* }}} */

/* {{{ crx_log_err_with_severity */
CRXAPI CREX_COLD void crx_log_err_with_severity(const char *log_message, int syslog_type_int)
{
	int fd = -1;
	time_t error_time;

	if (PG(in_error_log)) {
		/* prevent recursive invocation */
		return;
	}
	PG(in_error_log) = 1;

	/* Try to use the specified logging location. */
	if (PG(error_log) != NULL) {
		int error_log_mode;

#ifdef HAVE_SYSLOG_H
		if (!strcmp(PG(error_log), "syslog")) {
			crx_syslog(syslog_type_int, "%s", log_message);
			PG(in_error_log) = 0;
			return;
		}
#endif

		error_log_mode = 0644;

		if (PG(error_log_mode) > 0 && PG(error_log_mode) <= 0777) {
			error_log_mode = PG(error_log_mode);
		}

		fd = VCWD_OPEN_MODE(PG(error_log), O_CREAT | O_APPEND | O_WRONLY, error_log_mode);
		if (fd != -1) {
			char *tmp;
			size_t len;
			crex_string *error_time_str;

			time(&error_time);
#ifdef ZTS
			if (!crx_during_module_startup()) {
				error_time_str = crx_format_date("d-M-Y H:i:s e", 13, error_time, 1);
			} else {
				error_time_str = crx_format_date("d-M-Y H:i:s e", 13, error_time, 0);
			}
#else
			error_time_str = crx_format_date("d-M-Y H:i:s e", 13, error_time, 1);
#endif
			len = spprintf(&tmp, 0, "[%s] %s%s", ZSTR_VAL(error_time_str), log_message, CRX_EOL);
#ifdef CRX_WIN32
			crx_flock(fd, LOCK_EX);
			/* XXX should eventually write in a loop if len > UINT_MAX */
			crx_ignore_value(write(fd, tmp, (unsigned)len));
			crx_flock(fd, LOCK_UN);
#else
			crx_ignore_value(write(fd, tmp, len));
#endif
			efree(tmp);
			crex_string_free(error_time_str);
			close(fd);
			PG(in_error_log) = 0;
			return;
		}
	}

	/* Otherwise fall back to the default logging location, if we have one */

	if (sapi_module.log_message) {
		sapi_module.log_message(log_message, syslog_type_int);
	}
	PG(in_error_log) = 0;
}
/* }}} */

/* {{{ crx_write
   wrapper for modules to use CRXWRITE */
CRXAPI size_t crx_write(void *buf, size_t size)
{
	return CRXWRITE(buf, size);
}
/* }}} */

/* {{{ crx_printf */
CRXAPI size_t crx_printf(const char *format, ...)
{
	va_list args;
	size_t ret;
	char *buffer;
	size_t size;

	va_start(args, format);
	size = vspprintf(&buffer, 0, format, args);
	ret = CRXWRITE(buffer, size);
	efree(buffer);
	va_end(args);

	return ret;
}
/* }}} */

/* {{{ crx_printf_unchecked */
CRXAPI size_t crx_printf_unchecked(const char *format, ...)
{
	va_list args;
	size_t ret;
	char *buffer;
	size_t size;

	va_start(args, format);
	size = vspprintf(&buffer, 0, format, args);
	ret = CRXWRITE(buffer, size);
	efree(buffer);
	va_end(args);

	return ret;
}
/* }}} */

static crex_string *escape_html(const char *buffer, size_t buffer_len) {
	crex_string *result = crx_escape_html_entities_ex(
		(const unsigned char *) buffer, buffer_len, 0, ENT_COMPAT,
		/* charset_hint */ NULL, /* double_encode */ 1, /* quiet */ 1);
	if (!result || ZSTR_LEN(result) == 0) {
		/* Retry with substituting invalid chars on fail. */
		result = crx_escape_html_entities_ex(
			(const unsigned char *) buffer, buffer_len, 0, ENT_COMPAT | ENT_HTML_SUBSTITUTE_ERRORS,
			/* charset_hint */ NULL, /* double_encode */ 1, /* quiet */ 1);
	}
	return result;
}

/* {{{ crx_verror */
/* crx_verror is called from crx_error_docref<n> functions.
 * Its purpose is to unify error messages and automatically generate clickable
 * html error messages if corresponding ini setting (html_errors) is activated.
 * See: CODING_STANDARDS.md for details.
 */
CRXAPI CREX_COLD void crx_verror(const char *docref, const char *params, int type, const char *format, va_list args)
{
	crex_string *replace_buffer = NULL, *replace_origin = NULL;
	char *buffer = NULL, *docref_buf = NULL, *target = NULL;
	char *docref_target = "", *docref_root = "";
	char *p;
	int buffer_len = 0;
	const char *space = "";
	const char *class_name = "";
	const char *function;
	int origin_len;
	char *origin;
	crex_string *message;
	int is_function = 0;

	/* get error text into buffer and escape for html if necessary */
	buffer_len = (int)vspprintf(&buffer, 0, format, args);

	if (PG(html_errors)) {
		replace_buffer = escape_html(buffer, buffer_len);
		efree(buffer);

		if (replace_buffer) {
			buffer = ZSTR_VAL(replace_buffer);
			buffer_len = (int)ZSTR_LEN(replace_buffer);
		} else {
			buffer = "";
			buffer_len = 0;
		}
	}

	/* which function caused the problem if any at all */
	if (crx_during_module_startup()) {
		function = "CRX Startup";
	} else if (crx_during_module_shutdown()) {
		function = "CRX Shutdown";
	} else if (PG(during_request_startup)) {
		function = "CRX Request Startup";
	} else if (EG(current_execute_data) &&
				EG(current_execute_data)->func &&
				CREX_USER_CODE(EG(current_execute_data)->func->common.type) &&
				EG(current_execute_data)->opline &&
				EG(current_execute_data)->opline->opcode == CREX_INCLUDE_OR_EVAL
	) {
		switch (EG(current_execute_data)->opline->extended_value) {
			case CREX_EVAL:
				function = "eval";
				is_function = 1;
				break;
			case CREX_INCLUDE:
				function = "include";
				is_function = 1;
				break;
			case CREX_INCLUDE_ONCE:
				function = "include_once";
				is_function = 1;
				break;
			case CREX_REQUIRE:
				function = "require";
				is_function = 1;
				break;
			case CREX_REQUIRE_ONCE:
				function = "require_once";
				is_function = 1;
				break;
			default:
				function = "Unknown";
		}
	} else if ((function = get_active_function_name()) && strlen(function)) {
		is_function = 1;
		class_name = get_active_class_name(&space);
	} else if (EG(flags) & EG_FLAGS_IN_SHUTDOWN) {
		function = "CRX Request Shutdown";
	} else {
		function = "Unknown";
	}

	/* if we still have memory then format the origin */
	if (is_function) {
		origin_len = (int)spprintf(&origin, 0, "%s%s%s(%s)", class_name, space, function, params);
	} else {
		origin_len = (int)spprintf(&origin, 0, "%s", function);
	}

	if (PG(html_errors)) {
		replace_origin = escape_html(origin, origin_len);
		efree(origin);
		origin = ZSTR_VAL(replace_origin);
	}

	/* origin and buffer available, so let's come up with the error message */
	if (docref && docref[0] == '#') {
		docref_target = strchr(docref, '#');
		docref = NULL;
	}

	/* no docref given but function is known (the default) */
	if (!docref && is_function) {
		int doclen;
		while (*function == '_') {
			function++;
		}
		if (space[0] == '\0') {
			doclen = (int)spprintf(&docref_buf, 0, "function.%s", function);
		} else {
			doclen = (int)spprintf(&docref_buf, 0, "%s.%s", class_name, function);
		}
		while((p = strchr(docref_buf, '_')) != NULL) {
			*p = '-';
		}
		crex_str_tolower(docref_buf, doclen);
		docref = docref_buf;
	}

	/* we have a docref for a function AND
	 * - we show errors in html mode AND
	 * - the user wants to see the links
	 */
	if (docref && is_function && PG(html_errors) && strlen(PG(docref_root))) {
		if (strncmp(docref, "http://", 7)) {
			/* We don't have 'http://' so we use docref_root */

			char *ref;  /* temp copy for duplicated docref */

			docref_root = PG(docref_root);

			ref = estrdup(docref);
			if (docref_buf) {
				efree(docref_buf);
			}
			docref_buf = ref;
			/* strip of the target if any */
			p = strrchr(ref, '#');
			if (p) {
				target = estrdup(p);
				if (target) {
					docref_target = target;
					*p = '\0';
				}
			}
			/* add the extension if it is set in ini */
			if (PG(docref_ext) && strlen(PG(docref_ext))) {
				spprintf(&docref_buf, 0, "%s%s", ref, PG(docref_ext));
				efree(ref);
			}
			docref = docref_buf;
		}
		/* display html formatted or only show the additional links */
		if (PG(html_errors)) {
			message = crex_strpprintf(0, "%s [<a href='%s%s%s'>%s</a>]: %s", origin, docref_root, docref, docref_target, docref, buffer);
		} else {
			message = crex_strpprintf(0, "%s [%s%s%s]: %s", origin, docref_root, docref, docref_target, buffer);
		}
		if (target) {
			efree(target);
		}
	} else {
		message = crex_strpprintf(0, "%s: %s", origin, buffer);
	}
	if (replace_origin) {
		crex_string_free(replace_origin);
	} else {
		efree(origin);
	}
	if (docref_buf) {
		efree(docref_buf);
	}

	if (replace_buffer) {
		crex_string_free(replace_buffer);
	} else {
		efree(buffer);
	}

	crex_error_zstr(type, message);
	crex_string_release(message);
}
/* }}} */

/* {{{ crx_error_docref */
/* Generate an error which links to docref or the crx.net documentation if docref is NULL */
CRXAPI CREX_COLD void crx_error_docref(const char *docref, int type, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	crx_verror(docref, "", type, format, args);
	va_end(args);
}
/* }}} */

/* {{{ crx_error_docref1 */
/* See: CODING_STANDARDS.md for details. */
CRXAPI CREX_COLD void crx_error_docref1(const char *docref, const char *param1, int type, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	crx_verror(docref, param1, type, format, args);
	va_end(args);
}
/* }}} */

/* {{{ crx_error_docref2 */
/* See: CODING_STANDARDS.md for details. */
CRXAPI CREX_COLD void crx_error_docref2(const char *docref, const char *param1, const char *param2, int type, const char *format, ...)
{
	char *params;
	va_list args;

	spprintf(&params, 0, "%s,%s", param1, param2);
	va_start(args, format);
	crx_verror(docref, params ? params : "...", type, format, args);
	va_end(args);
	if (params) {
		efree(params);
	}
}
/* }}} */

#ifdef CRX_WIN32
CRXAPI CREX_COLD void crx_win32_docref1_from_error(DWORD error, const char *param1) {
	char *buf = crx_win32_error_to_msg(error);
	size_t buf_len;

	buf_len = strlen(buf);
	if (buf_len >= 2) {
		buf[buf_len - 1] = '\0';
		buf[buf_len - 2] = '\0';
	}
	crx_error_docref1(NULL, param1, E_WARNING, "%s (code: %lu)", buf, error);
	crx_win32_error_msg_free(buf);
}

CRXAPI CREX_COLD void crx_win32_docref2_from_error(DWORD error, const char *param1, const char *param2) {
	char *buf = crx_win32_error_to_msg(error);
	crx_error_docref2(NULL, param1, param2, E_WARNING, "%s (code: %lu)", buf, error);
	crx_win32_error_msg_free(buf);
}
#endif

/* {{{ crx_html_puts */
CRXAPI void crx_html_puts(const char *str, size_t size)
{
	crex_html_puts(str, size);
}
/* }}} */

static void clear_last_error(void) {
	if (PG(last_error_message)) {
		crex_string_release(PG(last_error_message));
		PG(last_error_message) = NULL;
	}
	if (PG(last_error_file)) {
		crex_string_release(PG(last_error_file));
		PG(last_error_file) = NULL;
	}
}

#if CREX_DEBUG
/* {{{ report_crex_debug_error_notify_cb */
static void report_crex_debug_error_notify_cb(int type, crex_string *error_filename, uint32_t error_lineno, crex_string *message)
{
	if (PG(report_crex_debug)) {
		bool trigger_break;

		switch (type) {
			case E_ERROR:
			case E_CORE_ERROR:
			case E_COMPILE_ERROR:
			case E_USER_ERROR:
				trigger_break=1;
				break;
			default:
				trigger_break=0;
				break;
		}

		crex_output_debug_string(trigger_break, "%s(%" PRIu32 ") : %s", ZSTR_VAL(error_filename), error_lineno, ZSTR_VAL(message));
	}
}
/* }}} */
#endif

/* {{{ crx_error_cb
 extended error handling function */
static CREX_COLD void crx_error_cb(int orig_type, crex_string *error_filename, const uint32_t error_lineno, crex_string *message)
{
	bool display;
	int type = orig_type & E_ALL;

	/* check for repeated errors to be ignored */
	if (PG(ignore_repeated_errors) && PG(last_error_message)) {
		/* no check for PG(last_error_file) is needed since it cannot
		 * be NULL if PG(last_error_message) is not NULL */
		if (!crex_string_equals(PG(last_error_message), message)
			|| (!PG(ignore_repeated_source)
				&& ((PG(last_error_lineno) != (int)error_lineno)
					|| !crex_string_equals(PG(last_error_file), error_filename)))) {
			display = 1;
		} else {
			display = 0;
		}
	} else {
		display = 1;
	}

	/* according to error handling mode, throw exception or show it */
	if (EG(error_handling) == EH_THROW) {
		switch (type) {
			case E_WARNING:
			case E_CORE_WARNING:
			case E_COMPILE_WARNING:
			case E_USER_WARNING:
				/* throw an exception if we are in EH_THROW mode and the type is warning.
				 * fatal errors are real errors and cannot be made exceptions.
				 * exclude deprecated for the sake of BC to old damaged code.
				 * notices are no errors and are not treated as such like E_WARNINGS.
				 * DO NOT overwrite a pending exception.
				 */
				if (!EG(exception)) {
					crex_throw_error_exception(EG(exception_class), message, 0, type);
				}
				return;
			default:
				break;
		}
	}

	/* store the error if it has changed */
	if (display) {
		clear_last_error();
		if (!error_filename) {
			error_filename = ZSTR_KNOWN(CREX_STR_UNKNOWN_CAPITALIZED);
		}
		PG(last_error_type) = type;
		PG(last_error_message) = crex_string_copy(message);
		PG(last_error_file) = crex_string_copy(error_filename);
		PG(last_error_lineno) = error_lineno;
	}

	if (crex_alloc_in_memory_limit_error_reporting()) {
		crx_output_discard_all();
	}

	/* display/log the error if necessary */
	if (display && ((EG(error_reporting) & type) || (type & E_CORE))
		&& (PG(log_errors) || PG(display_errors) || (!module_initialized))) {
		char *error_type_str;
		int syslog_type_int = LOG_NOTICE;

		switch (type) {
			case E_ERROR:
			case E_CORE_ERROR:
			case E_COMPILE_ERROR:
			case E_USER_ERROR:
				error_type_str = "Fatal error";
				syslog_type_int = LOG_ERR;
				break;
			case E_RECOVERABLE_ERROR:
				error_type_str = "Recoverable fatal error";
				syslog_type_int = LOG_ERR;
				break;
			case E_WARNING:
			case E_CORE_WARNING:
			case E_COMPILE_WARNING:
			case E_USER_WARNING:
				error_type_str = "Warning";
				syslog_type_int = LOG_WARNING;
				break;
			case E_PARSE:
				error_type_str = "Parse error";
				syslog_type_int = LOG_ERR;
				break;
			case E_NOTICE:
			case E_USER_NOTICE:
				error_type_str = "Notice";
				syslog_type_int = LOG_NOTICE;
				break;
			case E_STRICT:
				error_type_str = "Strict Standards";
				syslog_type_int = LOG_INFO;
				break;
			case E_DEPRECATED:
			case E_USER_DEPRECATED:
				error_type_str = "Deprecated";
				syslog_type_int = LOG_INFO;
				break;
			default:
				error_type_str = "Unknown error";
				break;
		}

		if (PG(log_errors)
				|| (!module_initialized && (!PG(display_startup_errors) || !PG(display_errors)))) {
			char *log_buffer;
#ifdef CRX_WIN32
			if (type == E_CORE_ERROR || type == E_CORE_WARNING) {
				syslog(LOG_ALERT, "CRX %s: %s (%s)", error_type_str, ZSTR_VAL(message), GetCommandLine());
			}
#endif
			spprintf(&log_buffer, 0, "CRX %s:  %s in %s on line %" PRIu32, error_type_str, ZSTR_VAL(message), ZSTR_VAL(error_filename), error_lineno);
			crx_log_err_with_severity(log_buffer, syslog_type_int);
			efree(log_buffer);
		}

		if (PG(display_errors) && ((module_initialized && !PG(during_request_startup)) || (PG(display_startup_errors)))) {
			if (PG(xmlrpc_errors)) {
				crx_printf("<?xml version=\"1.0\"?><methodResponse><fault><value><struct><member><name>faultCode</name><value><int>" CREX_LONG_FMT "</int></value></member><member><name>faultString</name><value><string>%s:%s in %s on line %" PRIu32 "</string></value></member></struct></value></fault></methodResponse>", PG(xmlrpc_error_number), error_type_str, ZSTR_VAL(message), ZSTR_VAL(error_filename), error_lineno);
			} else {
				char *prepend_string = INI_STR("error_prepend_string");
				char *append_string = INI_STR("error_append_string");

				if (PG(html_errors)) {
					if (type == E_ERROR || type == E_PARSE) {
						crex_string *buf = escape_html(ZSTR_VAL(message), ZSTR_LEN(message));
						crx_printf("%s<br />\n<b>%s</b>:  %s in <b>%s</b> on line <b>%" PRIu32 "</b><br />\n%s", STR_PRINT(prepend_string), error_type_str, ZSTR_VAL(buf), ZSTR_VAL(error_filename), error_lineno, STR_PRINT(append_string));
						crex_string_free(buf);
					} else {
						crx_printf("%s<br />\n<b>%s</b>:  %s in <b>%s</b> on line <b>%" PRIu32 "</b><br />\n%s", STR_PRINT(prepend_string), error_type_str, ZSTR_VAL(message), ZSTR_VAL(error_filename), error_lineno, STR_PRINT(append_string));
					}
				} else {
					/* Write CLI/CGI errors to stderr if display_errors = "stderr" */
					if ((!strcmp(sapi_module.name, "cli") || !strcmp(sapi_module.name, "cgi") || !strcmp(sapi_module.name, "crxdbg")) &&
						PG(display_errors) == CRX_DISPLAY_ERRORS_STDERR
					) {
						fprintf(stderr, "%s: %s in %s on line %" PRIu32 "\n", error_type_str, ZSTR_VAL(message), ZSTR_VAL(error_filename), error_lineno);
#ifdef CRX_WIN32
						fflush(stderr);
#endif
					} else {
						crx_printf("%s\n%s: %s in %s on line %" PRIu32 "\n%s", STR_PRINT(prepend_string), error_type_str, ZSTR_VAL(message), ZSTR_VAL(error_filename), error_lineno, STR_PRINT(append_string));
					}
				}
			}
		}
	}

	/* Bail out if we can't recover */
	switch (type) {
		case E_CORE_ERROR:
			if(!module_initialized) {
				/* bad error in module startup - no way we can live with this */
				exit(-2);
			}
		CREX_FALLTHROUGH;
		case E_ERROR:
		case E_RECOVERABLE_ERROR:
		case E_PARSE:
		case E_COMPILE_ERROR:
		case E_USER_ERROR:
			EG(exit_status) = 255;
			if (module_initialized) {
				if (!PG(display_errors) &&
				    !SG(headers_sent) &&
					SG(sapi_headers).http_response_code == 200
				) {
					sapi_header_line ctr = {0};

					ctr.line = "HTTP/1.0 500 Internal Server Error";
					ctr.line_len = sizeof("HTTP/1.0 500 Internal Server Error") - 1;
					sapi_header_op(SAPI_HEADER_REPLACE, &ctr);
				}
				/* the parser would return 1 (failure), we can bail out nicely */
				if (!(orig_type & E_DONT_BAIL)) {
					/* restore memory limit */
					crex_set_memory_limit(PG(memory_limit));
					crex_objects_store_mark_destructed(&EG(objects_store));
					crex_bailout();
					return;
				}
			}
			break;
	}
}
/* }}} */

/* {{{ crx_get_current_user */
CRXAPI char *crx_get_current_user(void)
{
	crex_stat_t *pstat = NULL;

	if (SG(request_info).current_user) {
		return SG(request_info).current_user;
	}

	/* FIXME: I need to have this somehow handled if
	USE_SAPI is defined, because cgi will also be
	interfaced in USE_SAPI */

	pstat = sapi_get_stat();

	if (!pstat) {
		return "";
	} else {
#ifdef CRX_WIN32
		char *name = crx_win32_get_username();
		int len;

		if (!name) {
			return "";
		}
		len = (int)strlen(name);
		name[len] = '\0';
		SG(request_info).current_user_length = len;
		SG(request_info).current_user = estrndup(name, len);
		free(name);
		return SG(request_info).current_user;
#else
		struct passwd *pwd;
#if defined(ZTS) && defined(HAVE_GETPWUID_R) && defined(_SC_GETPW_R_SIZE_MAX)
		struct passwd _pw;
		struct passwd *retpwptr = NULL;
		int pwbuflen = sysconf(_SC_GETPW_R_SIZE_MAX);
		char *pwbuf;

		if (pwbuflen < 1) {
			return "";
		}
		pwbuf = emalloc(pwbuflen);
		if (getpwuid_r(pstat->st_uid, &_pw, pwbuf, pwbuflen, &retpwptr) != 0) {
			efree(pwbuf);
			return "";
		}
		if (retpwptr == NULL) {
			efree(pwbuf);
			return "";
		}
		pwd = &_pw;
#else
		if ((pwd=getpwuid(pstat->st_uid))==NULL) {
			return "";
		}
#endif
		SG(request_info).current_user_length = strlen(pwd->pw_name);
		SG(request_info).current_user = estrndup(pwd->pw_name, SG(request_info).current_user_length);
#if defined(ZTS) && defined(HAVE_GETPWUID_R) && defined(_SC_GETPW_R_SIZE_MAX)
		efree(pwbuf);
#endif
		return SG(request_info).current_user;
#endif
	}
}
/* }}} */

/* {{{ Sets the maximum time a script can run */
CRX_FUNCTION(set_time_limit)
{
	crex_long new_timeout;
	char *new_timeout_str;
	size_t new_timeout_strlen;
	crex_string *key;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &new_timeout) == FAILURE) {
		RETURN_THROWS();
	}

	new_timeout_strlen = crex_spprintf(&new_timeout_str, 0, CREX_LONG_FMT, new_timeout);

	key = ZSTR_INIT_LITERAL("max_execution_time", 0);
	if (crex_alter_ini_entry_chars_ex(key, new_timeout_str, new_timeout_strlen, CRX_INI_USER, CRX_INI_STAGE_RUNTIME, 0) == SUCCESS) {
		RETVAL_TRUE;
	} else {
		RETVAL_FALSE;
	}
	crex_string_release_ex(key, 0);
	efree(new_timeout_str);
}
/* }}} */

/* {{{ crx_fopen_wrapper_for_crex */
static FILE *crx_fopen_wrapper_for_crex(crex_string *filename, crex_string **opened_path)
{
	*opened_path = filename;
	return crx_stream_open_wrapper_as_file(ZSTR_VAL(filename), "rb", USE_PATH|REPORT_ERRORS|STREAM_OPEN_FOR_INCLUDE|STREAM_OPEN_FOR_CREX_STREAM, opened_path);
}
/* }}} */

static void crx_crex_stream_closer(void *handle) /* {{{ */
{
	crx_stream_close((crx_stream*)handle);
}
/* }}} */

static size_t crx_crex_stream_fsizer(void *handle) /* {{{ */
{
	crx_stream *stream = handle;
	crx_stream_statbuf ssb;

	/* File size reported by stat() may be inaccurate if stream filters are used.
	 * TODO: Should stat() be generally disabled if filters are used? */
	if (stream->readfilters.head) {
		return 0;
	}

	if (crx_stream_stat(stream, &ssb) == 0) {
		return ssb.sb.st_size;
	}
	return 0;
}
/* }}} */

static crex_result crx_stream_open_for_crex(crex_file_handle *handle) /* {{{ */
{
	return crx_stream_open_for_crex_ex(handle, USE_PATH|REPORT_ERRORS|STREAM_OPEN_FOR_INCLUDE);
}
/* }}} */

CRXAPI crex_result crx_stream_open_for_crex_ex(crex_file_handle *handle, int mode) /* {{{ */
{
	crex_string *opened_path;
	crex_string *filename;
	crx_stream *stream;

	CREX_ASSERT(handle->type == CREX_HANDLE_FILENAME);
	opened_path = filename = handle->filename;
	stream = crx_stream_open_wrapper((char *)ZSTR_VAL(filename), "rb", mode | STREAM_OPEN_FOR_CREX_STREAM, &opened_path);
	if (stream) {
		memset(handle, 0, sizeof(crex_file_handle));
		handle->type = CREX_HANDLE_STREAM;
		handle->filename = filename;
		handle->opened_path = opened_path;
		handle->handle.stream.handle  = stream;
		handle->handle.stream.reader  = (crex_stream_reader_t)_crx_stream_read;
		handle->handle.stream.fsizer  = crx_crex_stream_fsizer;
		handle->handle.stream.isatty  = 0;
		handle->handle.stream.closer = crx_crex_stream_closer;
		/* suppress warning if this stream is not explicitly closed */
		crx_stream_auto_cleanup(stream);
		/* Disable buffering to avoid double buffering between CRX and Crex streams. */
		crx_stream_set_option(stream, CRX_STREAM_OPTION_READ_BUFFER, CRX_STREAM_BUFFER_NONE, NULL);

		return SUCCESS;
	}
	return FAILURE;
}
/* }}} */

static crex_string *crx_resolve_path_for_crex(crex_string *filename) /* {{{ */
{
	return crx_resolve_path(ZSTR_VAL(filename), ZSTR_LEN(filename), PG(include_path));
}
/* }}} */

/* {{{ crx_get_configuration_directive_for_crex */
static zval *crx_get_configuration_directive_for_crex(crex_string *name)
{
	return cfg_get_entry_ex(name);
}
/* }}} */

/* {{{ crx_free_request_globals */
static void crx_free_request_globals(void)
{
	clear_last_error();
	if (PG(crx_sys_temp_dir)) {
		efree(PG(crx_sys_temp_dir));
		PG(crx_sys_temp_dir) = NULL;
	}

	EG(filename_override) = NULL;
	EG(lineno_override) = -1;
}
/* }}} */

/* {{{ crx_message_handler_for_crex */
static CREX_COLD void crx_message_handler_for_crex(crex_long message, const void *data)
{
	switch (message) {
		case ZMSG_FAILED_INCLUDE_FOPEN: {
			char *tmp = estrdup((char *) data);
			crx_error_docref("function.include", E_WARNING, "Failed opening '%s' for inclusion (include_path='%s')", crx_strip_url_passwd(tmp), STR_PRINT(PG(include_path)));
			efree(tmp);
			break;
		}
		case ZMSG_FAILED_REQUIRE_FOPEN: {
			char *tmp = estrdup((char *) data);
			crex_throw_error(NULL, "Failed opening required '%s' (include_path='%s')", crx_strip_url_passwd(tmp), STR_PRINT(PG(include_path)));
			efree(tmp);
			break;
		}
		case ZMSG_FAILED_HIGHLIGHT_FOPEN: {
			char *tmp = estrdup((char *) data);
			crx_error_docref(NULL, E_WARNING, "Failed opening '%s' for highlighting", crx_strip_url_passwd(tmp));
			efree(tmp);
			break;
		}
		case ZMSG_MEMORY_LEAK_DETECTED:
		case ZMSG_MEMORY_LEAK_REPEATED:
#if CREX_DEBUG
			if (EG(error_reporting) & E_WARNING) {
				char memory_leak_buf[1024];

				if (message==ZMSG_MEMORY_LEAK_DETECTED) {
					crex_leak_info *t = (crex_leak_info *) data;

					snprintf(memory_leak_buf, 512, "%s(%" PRIu32 ") :  Freeing " CREX_ADDR_FMT " (%zu bytes), script=%s\n", t->filename, t->lineno, (size_t)t->addr, t->size, SAFE_FILENAME(SG(request_info).path_translated));
					if (t->orig_filename) {
						char relay_buf[512];

						snprintf(relay_buf, 512, "%s(%" PRIu32 ") : Actual location (location was relayed)\n", t->orig_filename, t->orig_lineno);
						strlcat(memory_leak_buf, relay_buf, sizeof(memory_leak_buf));
					}
				} else {
					unsigned long leak_count = (uintptr_t) data;

					snprintf(memory_leak_buf, 512, "Last leak repeated %lu time%s\n", leak_count, (leak_count>1?"s":""));
				}
#	if defined(CRX_WIN32)
				if (IsDebuggerPresent()) {
					OutputDebugString(memory_leak_buf);
				} else {
					fprintf(stderr, "%s", memory_leak_buf);
				}
#	else
				fprintf(stderr, "%s", memory_leak_buf);
#	endif
			}
#endif
			break;
		case ZMSG_MEMORY_LEAKS_GRAND_TOTAL:
#if CREX_DEBUG
			if (EG(error_reporting) & E_WARNING) {
				char memory_leak_buf[512];

				snprintf(memory_leak_buf, 512, "=== Total %d memory leaks detected ===\n", *((uint32_t *) data));
#	if defined(CRX_WIN32)
				if (IsDebuggerPresent()) {
					OutputDebugString(memory_leak_buf);
				} else {
					fprintf(stderr, "%s", memory_leak_buf);
				}
#	else
				fprintf(stderr, "%s", memory_leak_buf);
#	endif
			}
#endif
			break;
		case ZMSG_LOG_SCRIPT_NAME: {
				struct tm *ta, tmbuf;
				time_t curtime;
				char *datetime_str, asctimebuf[52];
				char memory_leak_buf[4096];

				time(&curtime);
				ta = crx_localtime_r(&curtime, &tmbuf);
				datetime_str = crx_asctime_r(ta, asctimebuf);
				if (datetime_str) {
					datetime_str[strlen(datetime_str)-1]=0;	/* get rid of the trailing newline */
					snprintf(memory_leak_buf, sizeof(memory_leak_buf), "[%s]  Script:  '%s'\n", datetime_str, SAFE_FILENAME(SG(request_info).path_translated));
				} else {
					snprintf(memory_leak_buf, sizeof(memory_leak_buf), "[null]  Script:  '%s'\n", SAFE_FILENAME(SG(request_info).path_translated));
				}
#	if defined(CRX_WIN32)
				if (IsDebuggerPresent()) {
					OutputDebugString(memory_leak_buf);
				} else {
					fprintf(stderr, "%s", memory_leak_buf);
				}
#	else
				fprintf(stderr, "%s", memory_leak_buf);
#	endif
			}
			break;
	}
}
/* }}} */


void crx_on_timeout(int seconds)
{
	PG(connection_status) |= CRX_CONNECTION_TIMEOUT;
}

#if CRX_SIGCHILD
/* {{{ sigchld_handler */
static void sigchld_handler(int apar)
{
	int errno_save = errno;

	while (waitpid(-1, NULL, WNOHANG) > 0);
	signal(SIGCHLD, sigchld_handler);

	errno = errno_save;
}
/* }}} */
#endif

/* {{{ crx_request_startup */
crex_result crx_request_startup(void)
{
	crex_result retval = SUCCESS;

	crex_interned_strings_activate();

#ifdef HAVE_DTRACE
	DTRACE_REQUEST_STARTUP(SAFE_FILENAME(SG(request_info).path_translated), SAFE_FILENAME(SG(request_info).request_uri), (char *)SAFE_FILENAME(SG(request_info).request_method));
#endif /* HAVE_DTRACE */

#ifdef CRX_WIN32
# if defined(ZTS)
	_configthreadlocale(_ENABLE_PER_THREAD_LOCALE);
# endif
	PG(com_initialized) = 0;
#endif

#if CRX_SIGCHILD
	signal(SIGCHLD, sigchld_handler);
#endif

	crex_try {
		PG(in_error_log) = 0;
		PG(during_request_startup) = 1;

		crx_output_activate();

		/* initialize global variables */
		PG(modules_activated) = 0;
		PG(header_is_being_sent) = 0;
		PG(connection_status) = CRX_CONNECTION_NORMAL;
		PG(in_user_include) = 0;

		crex_activate();
		sapi_activate();

#ifdef CREX_SIGNALS
		crex_signal_activate();
#endif

		if (PG(max_input_time) == -1) {
			crex_set_timeout(EG(timeout_seconds), 1);
		} else {
			crex_set_timeout(PG(max_input_time), 1);
		}

		/* Disable realpath cache if an open_basedir is set */
		if (PG(open_basedir) && *PG(open_basedir)) {
			CWDG(realpath_cache_size_limit) = 0;
		}

		if (PG(expose_crx) && !SG(headers_sent)) {
			sapi_add_header(SAPI_CRX_VERSION_HEADER, sizeof(SAPI_CRX_VERSION_HEADER)-1, 1);
		}

		if (PG(output_handler) && PG(output_handler)[0]) {
			zval oh;

			ZVAL_STRING(&oh, PG(output_handler));
			crx_output_start_user(&oh, 0, CRX_OUTPUT_HANDLER_STDFLAGS);
			zval_ptr_dtor(&oh);
		} else if (PG(output_buffering)) {
			crx_output_start_user(NULL, PG(output_buffering) > 1 ? PG(output_buffering) : 0, CRX_OUTPUT_HANDLER_STDFLAGS);
		} else if (PG(implicit_flush)) {
			crx_output_set_implicit_flush(1);
		}

		/* We turn this off in crx_execute_script() */
		/* PG(during_request_startup) = 0; */

		crx_hash_environment();
		crex_activate_modules();
		PG(modules_activated)=1;
	} crex_catch {
		retval = FAILURE;
	} crex_end_try();

	SG(sapi_started) = 1;

	return retval;
}
/* }}} */

/* {{{ crx_request_shutdown */
void crx_request_shutdown(void *dummy)
{
	bool report_memleaks;

	EG(flags) |= EG_FLAGS_IN_SHUTDOWN;

	report_memleaks = PG(report_memleaks);

	/* EG(current_execute_data) points into nirvana and therefore cannot be safely accessed
	 * inside crex_executor callback functions.
	 */
	EG(current_execute_data) = NULL;

	crx_deactivate_ticks();

	/* 0. Call any open observer end handlers that are still open after a crex_bailout */
	if (CREX_OBSERVER_ENABLED) {
		crex_observer_fcall_end_all();
	}

	/* 1. Call all possible shutdown functions registered with register_shutdown_function() */
	if (PG(modules_activated)) {
		crx_call_shutdown_functions();
	}

	/* 2. Call all possible __destruct() functions */
	crex_try {
		crex_call_destructors();
	} crex_end_try();

	/* 3. Flush all output buffers */
	crex_try {
		crx_output_end_all();
	} crex_end_try();

	/* 4. Reset max_execution_time (no longer executing crx code after response sent) */
	crex_try {
		crex_unset_timeout();
	} crex_end_try();

	/* 5. Call all extensions RSHUTDOWN functions */
	if (PG(modules_activated)) {
		crex_deactivate_modules();
	}

	/* 6. Shutdown output layer (send the set HTTP headers, cleanup output handlers, etc.) */
	crex_try {
		crx_output_deactivate();
	} crex_end_try();

	/* 7. Free shutdown functions */
	if (PG(modules_activated)) {
		crx_free_shutdown_functions();
	}

	/* 8. Destroy super-globals */
	crex_try {
		int i;

		for (i=0; i<NUM_TRACK_VARS; i++) {
			zval_ptr_dtor(&PG(http_globals)[i]);
		}
	} crex_end_try();

	/* 9. Shutdown scanner/executor/compiler and restore ini entries */
	crex_deactivate();

	/* 10. free request-bound globals */
	crx_free_request_globals();

	/* 11. Call all extensions post-RSHUTDOWN functions */
	crex_try {
		crex_post_deactivate_modules();
	} crex_end_try();

	/* 12. SAPI related shutdown*/
	crex_try {
		sapi_deactivate_module();
	} crex_end_try();
	/* free SAPI stuff */
	sapi_deactivate_destroy();

	/* 13. free virtual CWD memory */
	virtual_cwd_deactivate();

	/* 14. Destroy stream hashes */
	crex_try {
		crx_shutdown_stream_hashes();
	} crex_end_try();

	/* 15. Free Willy (here be crashes) */
	crex_arena_destroy(CG(arena));
	crex_interned_strings_deactivate();
	crex_try {
		shutdown_memory_manager(CG(unclean_shutdown) || !report_memleaks, 0);
	} crex_end_try();

	/* Reset memory limit, as the reset during INI_STAGE_DEACTIVATE may have failed.
	 * At this point, no memory beyond a single chunk should be in use. */
	crex_set_memory_limit(PG(memory_limit));

	/* 16. Deactivate Crex signals */
#ifdef CREX_SIGNALS
	crex_signal_deactivate();
#endif

#ifdef CRX_WIN32
	if (PG(com_initialized)) {
		CoUninitialize();
		PG(com_initialized) = 0;
	}
#endif

#ifdef HAVE_DTRACE
	DTRACE_REQUEST_SHUTDOWN(SAFE_FILENAME(SG(request_info).path_translated), SAFE_FILENAME(SG(request_info).request_uri), (char *)SAFE_FILENAME(SG(request_info).request_method));
#endif /* HAVE_DTRACE */
}
/* }}} */

/* {{{ crx_com_initialize */
CRXAPI void crx_com_initialize(void)
{
#ifdef CRX_WIN32
	if (!PG(com_initialized)) {
		if (CoInitialize(NULL) == S_OK) {
			PG(com_initialized) = 1;
		}
	}
#endif
}
/* }}} */

#ifdef ZTS
/* {{{ core_globals_ctor */
static void core_globals_ctor(crx_core_globals *core_globals)
{
	memset(core_globals, 0, sizeof(*core_globals));
	crx_startup_ticks();
}
/* }}} */
#endif

/* {{{ core_globals_dtor */
static void core_globals_dtor(crx_core_globals *core_globals)
{
	/* These should have been freed earlier. */
	CREX_ASSERT(!core_globals->last_error_message);
	CREX_ASSERT(!core_globals->last_error_file);

	if (core_globals->disable_classes) {
		free(core_globals->disable_classes);
	}
	if (core_globals->crx_binary) {
		free(core_globals->crx_binary);
	}

	crx_shutdown_ticks(core_globals);
}
/* }}} */

CRX_MINFO_FUNCTION(crx_core) { /* {{{ */
	crx_info_print_table_start();
	crx_info_print_table_row(2, "CRX Version", CRX_VERSION);
	crx_info_print_table_end();
	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ crx_register_extensions */
crex_result crx_register_extensions(crex_module_entry * const * ptr, int count)
{
	crex_module_entry * const * end = ptr + count;

	while (ptr < end) {
		if (*ptr) {
			if (crex_register_internal_module(*ptr)==NULL) {
				return FAILURE;
			}
		}
		ptr++;
	}
	return SUCCESS;
}

#ifdef CRX_WIN32
static _invalid_parameter_handler old_invalid_parameter_handler;

void dummy_invalid_parameter_handler(
		const wchar_t *expression,
		const wchar_t *function,
		const wchar_t *file,
		unsigned int   line,
		uintptr_t      pReserved)
{
	static int called = 0;
	char buf[1024];
	int len;

	if (!called) {
			if(PG(windows_show_crt_warning)) {
			called = 1;
			if (function) {
				if (file) {
					len = _snprintf(buf, sizeof(buf)-1, "Invalid parameter detected in CRT function '%ws' (%ws:%u)", function, file, line);
				} else {
					len = _snprintf(buf, sizeof(buf)-1, "Invalid parameter detected in CRT function '%ws'", function);
				}
			} else {
				len = _snprintf(buf, sizeof(buf)-1, "Invalid CRT parameter detected (function not known)");
			}
			crex_error(E_WARNING, "%s", buf);
			called = 0;
		}
	}
}
#endif

/* {{{ crx_module_startup */
crex_result crx_module_startup(sapi_module_struct *sf, crex_module_entry *additional_module)
{
	crex_utility_functions zuf;
	crex_utility_values zuv;
	crex_result retval = SUCCESS;
	int module_number = 0;
	crex_module_entry *module;

#ifdef CRX_WIN32
	WORD wVersionRequested = MAKEWORD(2, 0);
	WSADATA wsaData;

	old_invalid_parameter_handler =
		_set_invalid_parameter_handler(dummy_invalid_parameter_handler);
	if (old_invalid_parameter_handler != NULL) {
		_set_invalid_parameter_handler(old_invalid_parameter_handler);
	}

	/* Disable the message box for assertions.*/
	_CrtSetReportMode(_CRT_ASSERT, 0);
#endif

#ifdef ZTS
	(void)ts_resource(0);
#endif

#ifdef CRX_WIN32
	if (!crx_win32_init_random_bytes()) {
		fprintf(stderr, "\ncrypt algorithm provider initialization failed\n");
		return FAILURE;
	}
#endif

	module_shutdown = false;
	module_startup = true;
	sapi_initialize_empty_request();
	sapi_activate();

	if (module_initialized) {
		return SUCCESS;
	}

	sapi_module = *sf;

	crx_output_startup();

#ifdef ZTS
	ts_allocate_fast_id(&core_globals_id, &core_globals_offset, sizeof(crx_core_globals), (ts_allocate_ctor) core_globals_ctor, (ts_allocate_dtor) core_globals_dtor);
#ifdef CRX_WIN32
	ts_allocate_id(&crx_win32_core_globals_id, sizeof(crx_win32_core_globals), (ts_allocate_ctor) crx_win32_core_globals_ctor, (ts_allocate_dtor) crx_win32_core_globals_dtor);
#endif
#else
	memset(&core_globals, 0, sizeof(core_globals));
	crx_startup_ticks();
#endif
	gc_globals_ctor();

	zuf.error_function = crx_error_cb;
	zuf.printf_function = crx_printf;
	zuf.write_function = crx_output_write;
	zuf.fopen_function = crx_fopen_wrapper_for_crex;
	zuf.message_handler = crx_message_handler_for_crex;
	zuf.get_configuration_directive = crx_get_configuration_directive_for_crex;
	zuf.ticks_function = crx_run_ticks;
	zuf.on_timeout = crx_on_timeout;
	zuf.stream_open_function = crx_stream_open_for_crex;
	zuf.printf_to_smart_string_function = crx_printf_to_smart_string;
	zuf.printf_to_smart_str_function = crx_printf_to_smart_str;
	zuf.getenv_function = sapi_getenv;
	zuf.resolve_path_function = crx_resolve_path_for_crex;
	crex_startup(&zuf);
	crex_reset_lc_ctype_locale();
	crex_update_current_locale();

	crex_observer_startup();
#if CREX_DEBUG
	crex_observer_error_register(report_crex_debug_error_notify_cb);
#endif

#if HAVE_TZSET
	tzset();
#endif

#ifdef CRX_WIN32
	char *img_err;
	if (!crx_win32_crt_compatible(&img_err)) {
		crx_error(E_CORE_WARNING, img_err);
		efree(img_err);
		return FAILURE;
	}

	/* start up winsock services */
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		fprintf(stderr, "\nwinsock.dll unusable. %d\n", WSAGetLastError());
		return FAILURE;
	}
	crx_win32_signal_ctrl_handler_init();
#endif

	le_index_ptr = crex_register_list_destructors_ex(NULL, NULL, "index pointer", 0);

    register_main_symbols(module_number);

    REGISTER_MAIN_STRINGL_CONSTANT("CRX_SAPI", sapi_module.name, strlen(sapi_module.name), CONST_PERSISTENT | CONST_NO_FILE_CACHE);

	crx_binary_init();
	if (PG(crx_binary)) {
		REGISTER_MAIN_STRINGL_CONSTANT("CRX_BINARY", PG(crx_binary), strlen(PG(crx_binary)), CONST_PERSISTENT | CONST_NO_FILE_CACHE);
	} else {
		REGISTER_MAIN_STRINGL_CONSTANT("CRX_BINARY", "", 0, CONST_PERSISTENT | CONST_NO_FILE_CACHE);
	}

	/* this will read in crx.ini, set up the configuration parameters,
	   load crex extensions and register crx function extensions
	   to be loaded later */
	crex_stream_init();
	if (crx_init_config() == FAILURE) {
		return FAILURE;
	}
	crex_stream_shutdown();

	/* Register CRX core ini entries */
	crex_register_ini_entries_ex(ini_entries, module_number, MODULE_PERSISTENT);

	/* Register Crex ini entries */
	crex_register_standard_ini_entries();

#ifdef CREX_WIN32
	/* Until the current ini values was setup, the current cp is 65001.
		If the actual ini values are different, some stuff needs to be updated.
		It concerns at least main_cwd_state and there might be more. As we're
		still in the startup phase, lets use the chance and reinit the relevant
		item according to the current codepage. Still, if ini_set() is used
		later on, a more intelligent way to update such stuff is needed.
		Startup/shutdown routines could involve touching globals and thus
		can't always be used on demand. */
	if (!crx_win32_cp_use_unicode()) {
		virtual_cwd_main_cwd_init(1);
	}
#endif

	/* Disable realpath cache if an open_basedir is set */
	if (PG(open_basedir) && *PG(open_basedir)) {
		CWDG(realpath_cache_size_limit) = 0;
	}

	PG(have_called_openlog) = 0;

	/* initialize stream wrappers registry
	 * (this uses configuration parameters from crx.ini)
	 */
	if (crx_init_stream_wrappers(module_number) == FAILURE)	{
		fprintf(stderr, "CRX:  Unable to initialize stream url wrappers.\n");
		return FAILURE;
	}

	zuv.html_errors = 1;
	crx_startup_auto_globals();
	crex_set_utility_values(&zuv);
	crx_startup_sapi_content_types();

	/* Begin to fingerprint the process state */
	crex_startup_system_id();

	/* startup extensions statically compiled in */
	if (crx_register_internal_extensions_func() == FAILURE) {
		fprintf(stderr, "Unable to start builtin modules\n");
		return FAILURE;
	}

	/* start additional CRX extensions */
	if (additional_module && (crex_register_internal_module(additional_module) == NULL)) {
		return FAILURE;
	}

	/* load and startup extensions compiled as shared objects (aka DLLs)
	   as requested by crx.ini entries
	   these are loaded after initialization of internal extensions
	   as extensions *might* rely on things from ext/standard
	   which is always an internal extension and to be initialized
	   ahead of all other internals
	 */
	crx_ini_register_extensions();
	crex_startup_modules();

	/* start Crex extensions */
	crex_startup_extensions();

	crex_collect_module_handlers();

	/* register additional functions */
	if (sapi_module.additional_functions) {
		if ((module = crex_hash_str_find_ptr(&module_registry, "standard", sizeof("standard")-1)) != NULL) {
			EG(current_module) = module;
			crex_register_functions(NULL, sapi_module.additional_functions, NULL, MODULE_PERSISTENT);
			EG(current_module) = NULL;
		}
	}

	/* disable certain classes and functions as requested by crx.ini */
	crex_disable_functions(INI_STR("disable_functions"));
	crx_disable_classes();

	/* make core report what it should */
	if ((module = crex_hash_str_find_ptr(&module_registry, "core", sizeof("core")-1)) != NULL) {
		module->version = CRX_VERSION;
		module->info_func = CRX_MINFO(crx_core);
	}

	/* freeze the list of observer fcall_init handlers */
	crex_observer_post_startup();

	/* Extensions that add engine hooks after this point do so at their own peril */
	crex_finalize_system_id();

	module_initialized = true;

	if (crex_post_startup() != SUCCESS) {
		return FAILURE;
	}

	/* Check for deprecated directives */
	/* NOTE: If you add anything here, remember to add it to build/Makefile.global! */
	{
		struct {
			const long error_level;
			const char *phrase;
			const char *directives[18]; /* Remember to change this if the number of directives change */
		} directives[2] = {
			{
				E_DEPRECATED,
				"Directive '%s' is deprecated",
				{
					"allow_url_include",
					NULL
				}
			},
			{
				E_CORE_ERROR,
				"Directive '%s' is no longer available in CRX",
				{
					"allow_call_time_pass_reference",
					"asp_tags",
					"define_syslog_variables",
					"highlight.bg",
					"magic_quotes_gpc",
					"magic_quotes_runtime",
					"magic_quotes_sybase",
					"register_globals",
					"register_long_arrays",
					"safe_mode",
					"safe_mode_gid",
					"safe_mode_include_dir",
					"safe_mode_exec_dir",
					"safe_mode_allowed_env_vars",
					"safe_mode_protected_env_vars",
					"crex.ze1_compatibility_mode",
					"track_errors",
					NULL
				}
			}
		};

		unsigned int i;

		crex_try {
			/* 2 = Count of deprecation structs */
			for (i = 0; i < 2; i++) {
				const char **p = directives[i].directives;

				while(*p) {
					crex_long value;

					if (cfg_get_long((char*)*p, &value) == SUCCESS && value) {
						crex_error(directives[i].error_level, directives[i].phrase, *p);
					}

					++p;
				}
			}
		} crex_catch {
			retval = FAILURE;
		} crex_end_try();
	}

	virtual_cwd_deactivate();

	sapi_deactivate();
	module_startup = false;

	/* Don't leak errors from startup into the per-request phase. */
	clear_last_error();
	shutdown_memory_manager(1, 0);
	virtual_cwd_activate();

	crex_interned_strings_switch_storage(1);

#if CREX_RC_DEBUG
	if (retval == SUCCESS) {
		crex_rc_debug = 1;
	}
#endif

	/* we're done */
	return retval;
}
/* }}} */

/* {{{ crx_module_shutdown_wrapper */
int crx_module_shutdown_wrapper(sapi_module_struct *sapi_globals)
{
	crx_module_shutdown();
	return SUCCESS;
}
/* }}} */

/* {{{ crx_module_shutdown */
void crx_module_shutdown(void)
{
	int module_number=0;

	module_shutdown = true;

	if (!module_initialized) {
		return;
	}

	crex_interned_strings_switch_storage(0);

#if CREX_RC_DEBUG
	crex_rc_debug = 0;
#endif

#ifdef CRX_WIN32
	(void)crx_win32_shutdown_random_bytes();
	crx_win32_signal_ctrl_handler_shutdown();
#endif

	sapi_flush();

	crex_shutdown();

#ifdef CRX_WIN32
	/*close winsock */
	WSACleanup();
#endif

	/* Destroys filter & transport registries too */
	crx_shutdown_stream_wrappers(module_number);

	crex_unregister_ini_entries_ex(module_number, MODULE_PERSISTENT);

	/* close down the ini config */
	crx_shutdown_config();
	clear_last_error();

#ifndef ZTS
	crex_ini_shutdown();
	shutdown_memory_manager(CG(unclean_shutdown), 1);
#else
	crex_ini_global_shutdown();
#endif

	crx_output_shutdown();

#ifndef ZTS
	crex_interned_strings_dtor();
#endif

	if (crex_post_shutdown_cb) {
		void (*cb)(void) = crex_post_shutdown_cb;

		crex_post_shutdown_cb = NULL;
		cb();
	}

	module_initialized = false;

#ifndef ZTS
	core_globals_dtor(&core_globals);
	gc_globals_dtor();
#else
	ts_free_id(core_globals_id);
#endif

#ifdef CRX_WIN32
	if (old_invalid_parameter_handler == NULL) {
		_set_invalid_parameter_handler(old_invalid_parameter_handler);
	}
#endif

	crex_observer_shutdown();
}
/* }}} */

/* {{{ crx_execute_script */
CRXAPI bool crx_execute_script(crex_file_handle *primary_file)
{
	crex_file_handle *prepend_file_p = NULL, *append_file_p = NULL;
	crex_file_handle prepend_file, append_file;
#ifdef HAVE_BROKEN_GETCWD
	volatile int old_cwd_fd = -1;
#else
	char *old_cwd;
	ALLOCA_FLAG(use_heap)
#endif
	bool retval = false;

#ifndef HAVE_BROKEN_GETCWD
# define OLD_CWD_SIZE 4096
	old_cwd = do_alloca(OLD_CWD_SIZE, use_heap);
	old_cwd[0] = '\0';
#endif

	crex_try {
		char realfile[MAXPATHLEN];

#ifdef CRX_WIN32
		if(primary_file->filename) {
			UpdateIniFromRegistry(ZSTR_VAL(primary_file->filename));
		}
#endif

		PG(during_request_startup) = 0;

		if (primary_file->filename && !(SG(options) & SAPI_OPTION_NO_CHDIR)) {
#ifdef HAVE_BROKEN_GETCWD
			/* this looks nasty to me */
			old_cwd_fd = open(".", 0);
#else
			crx_ignore_value(VCWD_GETCWD(old_cwd, OLD_CWD_SIZE-1));
#endif
			VCWD_CHDIR_FILE(ZSTR_VAL(primary_file->filename));
		}

		/* Only lookup the real file path and add it to the included_files list if already opened
		 *   otherwise it will get opened and added to the included_files list in crex_execute_scripts
		 */
		if (primary_file->filename &&
			!crex_string_equals_literal(primary_file->filename, "Standard input code") &&
			primary_file->opened_path == NULL &&
			primary_file->type != CREX_HANDLE_FILENAME
		) {
			if (expand_filepath(ZSTR_VAL(primary_file->filename), realfile)) {
				primary_file->opened_path = crex_string_init(realfile, strlen(realfile), 0);
				crex_hash_add_empty_element(&EG(included_files), primary_file->opened_path);
			}
		}

		if (PG(auto_prepend_file) && PG(auto_prepend_file)[0]) {
			crex_stream_init_filename(&prepend_file, PG(auto_prepend_file));
			prepend_file_p = &prepend_file;
		}

		if (PG(auto_append_file) && PG(auto_append_file)[0]) {
			crex_stream_init_filename(&append_file, PG(auto_append_file));
			append_file_p = &append_file;
		}
		if (PG(max_input_time) != -1) {
#ifdef CRX_WIN32
			crex_unset_timeout();
#endif
			crex_set_timeout(INI_INT("max_execution_time"), 0);
		}

		retval = (crex_execute_scripts(CREX_REQUIRE, NULL, 3, prepend_file_p, primary_file, append_file_p) == SUCCESS);
	} crex_end_try();

	if (prepend_file_p) {
		crex_destroy_file_handle(prepend_file_p);
	}

	if (append_file_p) {
		crex_destroy_file_handle(append_file_p);
	}

	if (EG(exception)) {
		crex_try {
			crex_exception_error(EG(exception), E_ERROR);
		} crex_end_try();
	}

#ifdef HAVE_BROKEN_GETCWD
	if (old_cwd_fd != -1) {
		fchdir(old_cwd_fd);
		close(old_cwd_fd);
	}
#else
	if (old_cwd[0] != '\0') {
		crx_ignore_value(VCWD_CHDIR(old_cwd));
	}
	free_alloca(old_cwd, use_heap);
#endif
	return retval;
}
/* }}} */

/* {{{ crx_execute_simple_script */
CRXAPI int crx_execute_simple_script(crex_file_handle *primary_file, zval *ret)
{
	char *old_cwd;
	ALLOCA_FLAG(use_heap)

	EG(exit_status) = 0;
#define OLD_CWD_SIZE 4096
	old_cwd = do_alloca(OLD_CWD_SIZE, use_heap);
	old_cwd[0] = '\0';

	crex_try {
#ifdef CRX_WIN32
		if(primary_file->filename) {
			UpdateIniFromRegistry(ZSTR_VAL(primary_file->filename));
		}
#endif

		PG(during_request_startup) = 0;

		if (primary_file->filename && !(SG(options) & SAPI_OPTION_NO_CHDIR)) {
			crx_ignore_value(VCWD_GETCWD(old_cwd, OLD_CWD_SIZE-1));
			VCWD_CHDIR_FILE(ZSTR_VAL(primary_file->filename));
		}
		crex_execute_scripts(CREX_REQUIRE, ret, 1, primary_file);
	} crex_end_try();

	if (old_cwd[0] != '\0') {
		crx_ignore_value(VCWD_CHDIR(old_cwd));
	}

	free_alloca(old_cwd, use_heap);
	return EG(exit_status);
}
/* }}} */

/* {{{ crx_handle_aborted_connection */
CRXAPI void crx_handle_aborted_connection(void)
{

	PG(connection_status) = CRX_CONNECTION_ABORTED;
	crx_output_set_status(CRX_OUTPUT_DISABLED);

	if (!PG(ignore_user_abort)) {
		crex_bailout();
	}
}
/* }}} */

/* {{{ crx_handle_auth_data */
CRXAPI int crx_handle_auth_data(const char *auth)
{
	int ret = -1;
	size_t auth_len = auth != NULL ? strlen(auth) : 0;

	if (auth && auth_len > 0 && crex_binary_strncasecmp(auth, auth_len, "Basic ", sizeof("Basic ")-1, sizeof("Basic ")-1) == 0) {
		char *pass;
		crex_string *user;

		user = crx_base64_decode((const unsigned char*)auth + 6, auth_len - 6);
		if (user) {
			pass = strchr(ZSTR_VAL(user), ':');
			if (pass) {
				*pass++ = '\0';
				SG(request_info).auth_user = estrndup(ZSTR_VAL(user), ZSTR_LEN(user));
				SG(request_info).auth_password = estrdup(pass);
				ret = 0;
			}
			crex_string_free(user);
		}
	}

	if (ret == -1) {
		SG(request_info).auth_user = SG(request_info).auth_password = NULL;
	} else {
		SG(request_info).auth_digest = NULL;
	}

	if (ret == -1 && auth && auth_len > 0 && crex_binary_strncasecmp(auth, auth_len, "Digest ", sizeof("Digest ")-1, sizeof("Digest ")-1) == 0) {
		SG(request_info).auth_digest = estrdup(auth + 7);
		ret = 0;
	}

	if (ret == -1) {
		SG(request_info).auth_digest = NULL;
	}

	return ret;
}
/* }}} */

/* {{{ crx_lint_script */
CRXAPI crex_result crx_lint_script(crex_file_handle *file)
{
	crex_op_array *op_array;
	crex_result retval = FAILURE;

	crex_try {
		op_array = crex_compile_file(file, CREX_INCLUDE);

		if (op_array) {
			destroy_op_array(op_array);
			efree(op_array);
			retval = SUCCESS;
		}
	} crex_end_try();
	if (EG(exception)) {
		crex_exception_error(EG(exception), E_ERROR);
	}

	return retval;
}
/* }}} */

#ifdef ZTS
/* {{{ crx_reserve_tsrm_memory */
CRXAPI void crx_reserve_tsrm_memory(void)
{
	tsrm_reserve(
		TSRM_ALIGNED_SIZE(sizeof(crex_compiler_globals)) +
		TSRM_ALIGNED_SIZE(sizeof(crex_executor_globals)) +
		TSRM_ALIGNED_SIZE(sizeof(crex_crx_scanner_globals)) +
		TSRM_ALIGNED_SIZE(sizeof(crex_ini_scanner_globals)) +
		TSRM_ALIGNED_SIZE(sizeof(virtual_cwd_globals)) +
#ifdef CREX_SIGNALS
		TSRM_ALIGNED_SIZE(sizeof(crex_signal_globals_t)) +
#endif
		TSRM_ALIGNED_SIZE(crex_mm_globals_size()) +
		TSRM_ALIGNED_SIZE(crex_gc_globals_size()) +
		TSRM_ALIGNED_SIZE(sizeof(crx_core_globals)) +
		TSRM_ALIGNED_SIZE(sizeof(sapi_globals_struct))
	);
}
/* }}} */

CRXAPI bool crx_tsrm_startup_ex(int expected_threads)
{
	bool ret = tsrm_startup(expected_threads, 1, 0, NULL);
	crx_reserve_tsrm_memory();
	(void)ts_resource(0);
	return ret;
}

/* {{{ crx_tsrm_startup */
CRXAPI bool crx_tsrm_startup(void)
{
	return crx_tsrm_startup_ex(1);
}
/* }}} */
#endif
