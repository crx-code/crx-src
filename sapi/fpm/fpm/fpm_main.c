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
   | Authors: Rasmus Lerdorf <rasmus@lerdorf.on.ca>                       |
   |          Stig Bakken <ssb@crx.net>                                   |
   |          Zeev Suraski <zeev@crx.net>                                 |
   | FastCGI: Ben Mansell <crx@slimyhorror.com>                           |
   |          Shane Caraveo <shane@caraveo.com>                           |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#include "crx.h"
#include "crx_globals.h"
#include "crx_variables.h"
#include "crx_ini_builder.h"
#include "crex_modules.h"
#include "crx.h"
#include "crex_ini_scanner.h"
#include "crex_globals.h"
#include "crex_stream.h"

#include "SAPI.h"

#include <stdio.h>
#include "crx.h"

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <signal.h>

#include <locale.h>

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

#include "crex.h"
#include "crex_extensions.h"
#include "crx_ini.h"
#include "crx_globals.h"
#include "crx_main.h"
#include "fopen_wrappers.h"
#include "ext/standard/crx_standard.h"

#ifdef __riscos__
# include <unixlib/local.h>
int __riscosify_control = __RISCOSIFY_STRICT_UNIX_SPECS;
#endif

#include "crex_compile.h"
#include "crex_execute.h"
#include "crex_highlight.h"

#include "crx_getopt.h"

#include "http_status_codes.h"

#include "fastcgi.h"

#include <crx_config.h>
#include "fpm.h"
#include "fpm_main_arginfo.h"
#include "fpm_request.h"
#include "fpm_status.h"
#include "fpm_signals.h"
#include "fpm_stdio.h"
#include "fpm_conf.h"
#include "fpm_crx.h"
#include "fpm_log.h"
#include "zlog.h"

/* XXX this will need to change later when threaded fastcgi is implemented.  shane */
struct sigaction act, old_term, old_quit, old_int;

static void (*crx_crx_import_environment_variables)(zval *array_ptr);

/* these globals used for forking children on unix systems */

/**
 * Set to non-zero if we are the parent process
 */
static int parent = 1;

static int request_body_fd;
static int fpm_is_running = 0;

static char *sapi_cgibin_getenv(const char *name, size_t name_len);
static void fastcgi_ini_parser(zval *arg1, zval *arg2, zval *arg3, int callback_type, void *arg);

#define CRX_MODE_STANDARD	1
#define CRX_MODE_HIGHLIGHT	2
#define CRX_MODE_INDENT		3
#define CRX_MODE_LINT		4
#define CRX_MODE_STRIP		5

static char *crx_optarg = NULL;
static int crx_optind = 1;
static crex_module_entry cgi_module_entry;

static const opt_struct OPTIONS[] = {
	{'c', 1, "crx-ini"},
	{'d', 1, "define"},
	{'e', 0, "profile-info"},
	{'h', 0, "help"},
	{'i', 0, "info"},
	{'m', 0, "modules"},
	{'n', 0, "no-crx-ini"},
	{'?', 0, "usage"},/* help alias (both '?' and 'usage') */
	{'v', 0, "version"},
	{'y', 1, "fpm-config"},
	{'t', 0, "test"},
	{'p', 1, "prefix"},
	{'g', 1, "pid"},
	{'R', 0, "allow-to-run-as-root"},
	{'D', 0, "daemonize"},
	{'F', 0, "nodaemonize"},
	{'O', 0, "force-stderr"},
	{'-', 0, NULL} /* end of args */
};

typedef struct _crx_cgi_globals_struct {
	bool rfc2616_headers;
	bool nph;
	bool fix_pathinfo;
	bool force_redirect;
	bool discard_path;
	bool fcgi_logging;
	bool fcgi_logging_request_started;
	char *redirect_status_env;
	HashTable user_config_cache;
	char *error_header;
	char *fpm_config;
} crx_cgi_globals_struct;

/* {{{ user_config_cache
 *
 * Key for each cache entry is dirname(PATH_TRANSLATED).
 *
 * NOTE: Each cache entry config_hash contains the combination from all user ini files found in
 *       the path starting from doc_root through to dirname(PATH_TRANSLATED).  There is no point
 *       storing per-file entries as it would not be possible to detect added / deleted entries
 *       between separate files.
 */
typedef struct _user_config_cache_entry {
	time_t expires;
	HashTable *user_config;
} user_config_cache_entry;

static void user_config_cache_entry_dtor(zval *el)
{
	user_config_cache_entry *entry = (user_config_cache_entry *)C_PTR_P(el);
	crex_hash_destroy(entry->user_config);
	free(entry->user_config);
	free(entry);
}
/* }}} */

#ifdef ZTS
static int crx_cgi_globals_id;
#define CGIG(v) CREX_TSRMG(crx_cgi_globals_id, crx_cgi_globals_struct *, v)
#else
static crx_cgi_globals_struct crx_cgi_globals;
#define CGIG(v) (crx_cgi_globals.v)
#endif

static int module_name_cmp(Bucket *f, Bucket *s) /* {{{ */
{
	return strcasecmp(	((crex_module_entry *) C_PTR(f->val))->name,
						((crex_module_entry *) C_PTR(s->val))->name);
}
/* }}} */

static void print_modules(void) /* {{{ */
{
	HashTable sorted_registry;
	crex_module_entry *module;

	crex_hash_init(&sorted_registry, 50, NULL, NULL, 1);
	crex_hash_copy(&sorted_registry, &module_registry, NULL);
	crex_hash_sort(&sorted_registry, module_name_cmp, 0);
	CREX_HASH_MAP_FOREACH_PTR(&sorted_registry, module) {
		crx_printf("%s\n", module->name);
	} CREX_HASH_FOREACH_END();
	crex_hash_destroy(&sorted_registry);
}
/* }}} */

static void print_extension_info(crex_extension *ext) /* {{{ */
{
	crx_printf("%s\n", ext->name);
}
/* }}} */

static int extension_name_cmp(const crex_llist_element **f, const crex_llist_element **s) /* {{{ */
{
	crex_extension *fe = (crex_extension*)(*f)->data;
	crex_extension *se = (crex_extension*)(*s)->data;
	return strcmp(fe->name, se->name);
}
/* }}} */

static void print_extensions(void) /* {{{ */
{
	crex_llist sorted_exts;

	crex_llist_copy(&sorted_exts, &crex_extensions);
	sorted_exts.dtor = NULL;
	crex_llist_sort(&sorted_exts, extension_name_cmp);
	crex_llist_apply(&sorted_exts, (llist_apply_func_t) print_extension_info);
	crex_llist_destroy(&sorted_exts);
}
/* }}} */

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

static inline size_t sapi_cgibin_single_write(const char *str, uint32_t str_length) /* {{{ */
{
	ssize_t ret;

	/* sapi has started which means everything must be send through fcgi */
	if (fpm_is_running) {
		fcgi_request *request = (fcgi_request*) SG(server_context);
		ret = fcgi_write(request, FCGI_STDOUT, str, str_length);
		if (ret <= 0) {
			return 0;
		}
		return (size_t)ret;
	}

	/* sapi has not started, output to stdout instead of fcgi */
#ifdef CRX_WRITE_STDOUT
	ret = write(STDOUT_FILENO, str, str_length);
	if (ret <= 0) {
		return 0;
	}
	return (size_t)ret;
#else
	return fwrite(str, 1, MIN(str_length, 16384), stdout);
#endif
}
/* }}} */

static size_t sapi_cgibin_ub_write(const char *str, size_t str_length) /* {{{ */
{
	const char *ptr = str;
	uint32_t remaining = str_length;
	size_t ret;

	while (remaining > 0) {
		ret = sapi_cgibin_single_write(ptr, remaining);
		if (!ret) {
			crx_handle_aborted_connection();
			return str_length - remaining;
		}
		ptr += ret;
		remaining -= ret;
	}

	return str_length;
}
/* }}} */

static void sapi_cgibin_flush(void *server_context) /* {{{ */
{
	/* fpm has started, let use fcgi instead of stdout */
	if (fpm_is_running) {
		fcgi_request *request = (fcgi_request*) server_context;
		if (!parent && request && !fcgi_flush(request, 0)) {
			crx_handle_aborted_connection();
		}
		return;
	}

	/* fpm has not started yet, let use stdout instead of fcgi */
	if (fflush(stdout) == EOF) {
		crx_handle_aborted_connection();
	}
}
/* }}} */

#define SAPI_CGI_MAX_HEADER_LENGTH 1024

static int sapi_cgi_send_headers(sapi_headers_struct *sapi_headers) /* {{{ */
{
	char buf[SAPI_CGI_MAX_HEADER_LENGTH];
	sapi_header_struct *h;
	crex_llist_position pos;
	bool ignore_status = 0;
	int response_status = SG(sapi_headers).http_response_code;

	if (SG(request_info).no_headers == 1) {
		return  SAPI_HEADER_SENT_SUCCESSFULLY;
	}

	if (CGIG(nph) || SG(sapi_headers).http_response_code != 200)
	{
		int len;
		bool has_status = 0;

		if (CGIG(rfc2616_headers) && SG(sapi_headers).http_status_line) {
			char *s;
			len = slprintf(buf, SAPI_CGI_MAX_HEADER_LENGTH, "%s", SG(sapi_headers).http_status_line);
			if ((s = strchr(SG(sapi_headers).http_status_line, ' '))) {
				response_status = atoi((s + 1));
			}

			if (len > SAPI_CGI_MAX_HEADER_LENGTH) {
				len = SAPI_CGI_MAX_HEADER_LENGTH;
			}

		} else {
			char *s;

			if (SG(sapi_headers).http_status_line &&
				(s = strchr(SG(sapi_headers).http_status_line, ' ')) != 0 &&
				(s - SG(sapi_headers).http_status_line) >= 5 &&
				strncasecmp(SG(sapi_headers).http_status_line, "HTTP/", 5) == 0
			) {
				len = slprintf(buf, sizeof(buf), "Status:%s", s);
				response_status = atoi((s + 1));
			} else {
				h = (sapi_header_struct*)crex_llist_get_first_ex(&sapi_headers->headers, &pos);
				while (h) {
					if (h->header_len > sizeof("Status:") - 1 &&
						strncasecmp(h->header, "Status:", sizeof("Status:") - 1) == 0
					) {
						has_status = 1;
						break;
					}
					h = (sapi_header_struct*)crex_llist_get_next_ex(&sapi_headers->headers, &pos);
				}
				if (!has_status) {
					http_response_status_code_pair *err = (http_response_status_code_pair*)http_status_map;

					while (err->code != 0) {
						if (err->code == SG(sapi_headers).http_response_code) {
							break;
						}
						err++;
					}
					if (err->str) {
						len = slprintf(buf, sizeof(buf), "Status: %d %s", SG(sapi_headers).http_response_code, err->str);
					} else {
						len = slprintf(buf, sizeof(buf), "Status: %d", SG(sapi_headers).http_response_code);
					}
				}
			}
		}

		if (!has_status) {
			CRXWRITE_H(buf, len);
			CRXWRITE_H("\r\n", 2);
			ignore_status = 1;
		}
	}

	h = (sapi_header_struct*)crex_llist_get_first_ex(&sapi_headers->headers, &pos);
	while (h) {
		/* prevent CRLFCRLF */
		if (h->header_len) {
			if (h->header_len > sizeof("Status:") - 1 &&
				strncasecmp(h->header, "Status:", sizeof("Status:") - 1) == 0
			) {
				if (!ignore_status) {
					ignore_status = 1;
					CRXWRITE_H(h->header, h->header_len);
					CRXWRITE_H("\r\n", 2);
				}
			} else if (response_status == 304 && h->header_len > sizeof("Content-Type:") - 1 &&
				strncasecmp(h->header, "Content-Type:", sizeof("Content-Type:") - 1) == 0
			) {
				h = (sapi_header_struct*)crex_llist_get_next_ex(&sapi_headers->headers, &pos);
				continue;
			} else {
				CRXWRITE_H(h->header, h->header_len);
				CRXWRITE_H("\r\n", 2);
			}
		}
		h = (sapi_header_struct*)crex_llist_get_next_ex(&sapi_headers->headers, &pos);
	}
	CRXWRITE_H("\r\n", 2);

	return SAPI_HEADER_SENT_SUCCESSFULLY;
}
/* }}} */

#ifndef STDIN_FILENO
# define STDIN_FILENO 0
#endif

#ifndef HAVE_ATTRIBUTE_WEAK
static void fpm_fcgi_log(int type, const char *fmt, ...) /* {{{ */
#else
void fcgi_log(int type, const char *fmt, ...)
#endif
{
	va_list args;
	va_start(args, fmt);
	vzlog("", 0, type, fmt, args);
	va_end(args);
}
/* }}} */

static size_t sapi_cgi_read_post(char *buffer, size_t count_bytes) /* {{{ */
{
	uint32_t read_bytes = 0;
	int tmp_read_bytes;
	size_t remaining = SG(request_info).content_length - SG(read_post_bytes);

	if (remaining < count_bytes) {
		count_bytes = remaining;
	}
	while (read_bytes < count_bytes) {
		fcgi_request *request = (fcgi_request*) SG(server_context);
		if (request_body_fd == -1) {
			char *request_body_filename = FCGI_GETENV(request, "REQUEST_BODY_FILE");

			if (request_body_filename && *request_body_filename) {
				request_body_fd = open(request_body_filename, O_RDONLY);

				if (0 > request_body_fd) {
					crx_error(E_WARNING, "REQUEST_BODY_FILE: open('%s') failed: %s (%d)",
							request_body_filename, strerror(errno), errno);
					return 0;
				}
			}
		}

		/* If REQUEST_BODY_FILE variable not available - read post body from fastcgi stream */
		if (request_body_fd < 0) {
			tmp_read_bytes = fcgi_read(request, buffer + read_bytes, count_bytes - read_bytes);
		} else {
			tmp_read_bytes = read(request_body_fd, buffer + read_bytes, count_bytes - read_bytes);
		}
		if (tmp_read_bytes <= 0) {
			break;
		}
		read_bytes += tmp_read_bytes;
	}
	return read_bytes;
}
/* }}} */

static char *sapi_cgibin_getenv(const char *name, size_t name_len) /* {{{ */
{
	/* if fpm has started, use fcgi env */
	if (fpm_is_running) {
		fcgi_request *request = (fcgi_request*) SG(server_context);
		return fcgi_getenv(request, name, name_len);
	}

	/* if fpm has not started yet, use std env */
	return getenv(name);
}
/* }}} */

#if 0
static char *_sapi_cgibin_putenv(char *name, char *value) /* {{{ */
{
	int name_len;

	if (!name) {
		return NULL;
	}
	name_len = strlen(name);

	fcgi_request *request = (fcgi_request*) SG(server_context);
	return fcgi_putenv(request, name, name_len, value);
}
/* }}} */
#endif

static char *sapi_cgi_read_cookies(void) /* {{{ */
{
	fcgi_request *request = (fcgi_request*) SG(server_context);

	return FCGI_GETENV(request, "HTTP_COOKIE");
}
/* }}} */

static void cgi_crx_load_env_var(const char *var, unsigned int var_len, char *val, unsigned int val_len, void *arg) /* {{{ */
{
	zval *array_ptr = (zval *) arg;
	int filter_arg = (C_ARR_P(array_ptr) == C_ARR(PG(http_globals)[TRACK_VARS_ENV])) ? PARSE_ENV : PARSE_SERVER;
	size_t new_val_len;

	if (sapi_module.input_filter(filter_arg, var, &val, strlen(val), &new_val_len)) {
		crx_register_variable_safe(var, val, new_val_len, array_ptr);
	}
}
/* }}} */

void cgi_crx_import_environment_variables(zval *array_ptr) /* {{{ */
{
	fcgi_request *request = NULL;

	if (C_TYPE(PG(http_globals)[TRACK_VARS_ENV]) == IS_ARRAY &&
		C_ARR_P(array_ptr) != C_ARR(PG(http_globals)[TRACK_VARS_ENV]) &&
		crex_hash_num_elements(C_ARRVAL(PG(http_globals)[TRACK_VARS_ENV])) > 0
	) {
		crex_array_destroy(C_ARR_P(array_ptr));
		C_ARR_P(array_ptr) = crex_array_dup(C_ARR(PG(http_globals)[TRACK_VARS_ENV]));
		return;
	} else if (C_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) == IS_ARRAY &&
		C_ARR_P(array_ptr) != C_ARR(PG(http_globals)[TRACK_VARS_SERVER]) &&
		crex_hash_num_elements(C_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER])) > 0
	) {
		crex_array_destroy(C_ARR_P(array_ptr));
		C_ARR_P(array_ptr) = crex_array_dup(C_ARR(PG(http_globals)[TRACK_VARS_SERVER]));
		return;
	}

	/* call crx's original import as a catch-all */
	crx_crx_import_environment_variables(array_ptr);

	request = (fcgi_request*) SG(server_context);
	fcgi_loadenv(request, cgi_crx_load_env_var, array_ptr);
}
/* }}} */

static void sapi_cgi_register_variables(zval *track_vars_array) /* {{{ */
{
	size_t crx_self_len;
	char *crx_self;

	/* In CGI mode, we consider the environment to be a part of the server
	 * variables
	 */
	crx_import_environment_variables(track_vars_array);

	if (CGIG(fix_pathinfo)) {
		char *script_name = SG(request_info).request_uri;
		unsigned int script_name_len = script_name ? strlen(script_name) : 0;
		char *path_info = sapi_cgibin_getenv("PATH_INFO", sizeof("PATH_INFO") - 1);
		unsigned int path_info_len = path_info ? strlen(path_info) : 0;

		crx_self_len = script_name_len + path_info_len;
		crx_self = emalloc(crx_self_len + 1);

		/* Concat script_name and path_info into crx_self */
		if (script_name) {
			memcpy(crx_self, script_name, script_name_len + 1);
		}
		if (path_info) {
			memcpy(crx_self + script_name_len, path_info, path_info_len + 1);
		}

		/* Build the special-case CRX_SELF variable for the CGI version */
		if (sapi_module.input_filter(PARSE_SERVER, "CRX_SELF", &crx_self, crx_self_len, &crx_self_len)) {
			crx_register_variable_safe("CRX_SELF", crx_self, crx_self_len, track_vars_array);
		}
		efree(crx_self);
	} else {
		crx_self = SG(request_info).request_uri ? SG(request_info).request_uri : "";
		crx_self_len = strlen(crx_self);
		if (sapi_module.input_filter(PARSE_SERVER, "CRX_SELF", &crx_self, crx_self_len, &crx_self_len)) {
			crx_register_variable_safe("CRX_SELF", crx_self, crx_self_len, track_vars_array);
		}
	}
}
/* }}} */

/* {{{ sapi_cgi_log_fastcgi
 *
 * Ignore level, we want to send all messages through fastcgi
 */
void sapi_cgi_log_fastcgi(int level, char *message, size_t len)
{

	fcgi_request *request = (fcgi_request*) SG(server_context);

	/* message is written to FCGI_STDERR if following conditions are met:
	 * - logging is enabled (fastcgi.logging in crx.ini)
	 * - we are currently dealing with a request
	 * - the message is not empty
	 */
	if (CGIG(fcgi_logging) && request && message && len > 0) {
		if (CGIG(fcgi_logging_request_started)) {
			fcgi_write(request, FCGI_STDERR, "; ", 2);
		} else {
			CGIG(fcgi_logging_request_started) = true;
		}
		if (fcgi_write(request, FCGI_STDERR, message, len) < 0) {
			crx_handle_aborted_connection();
		}
	}
}
/* }}} */

/* {{{ sapi_cgi_log_message */
static void sapi_cgi_log_message(const char *message, int syslog_type_int)
{
	zlog_msg(ZLOG_NOTICE, "CRX message: ", message);
}
/* }}} */

/* {{{ crx_cgi_ini_activate_user_config */
static void crx_cgi_ini_activate_user_config(char *path, int path_len, const char *doc_root, int doc_root_len)
{
	char *ptr;
	time_t request_time = sapi_get_request_time();
	user_config_cache_entry *entry = crex_hash_str_find_ptr(&CGIG(user_config_cache), path, path_len);

	/* Find cached config entry: If not found, create one */
	if (!entry) {
		entry = pemalloc(sizeof(user_config_cache_entry), 1);
		entry->expires = 0;
		entry->user_config = (HashTable *) pemalloc(sizeof(HashTable), 1);
		crex_hash_init(entry->user_config, 0, NULL, config_zval_dtor, 1);
		crex_hash_str_update_ptr(&CGIG(user_config_cache), path, path_len, entry);
	}

	/* Check whether cache entry has expired and rescan if it is */
	if (request_time > entry->expires) {
		char * real_path;
		int real_path_len;
		char *s1, *s2;
		int s_len;

		/* Clear the expired config */
		crex_hash_clean(entry->user_config);

		if (!IS_ABSOLUTE_PATH(path, path_len)) {
			real_path = tsrm_realpath(path, NULL);
			if (real_path == NULL) {
				return;
			}
			real_path_len = strlen(real_path);
			path = real_path;
			path_len = real_path_len;
		}

		if (path_len > doc_root_len) {
			s1 = (char *) doc_root;
			s2 = path;
			s_len = doc_root_len;
		} else {
			s1 = path;
			s2 = (char *) doc_root;
			s_len = path_len;
		}

		/* we have to test if path is part of DOCUMENT_ROOT.
		  if it is inside the docroot, we scan the tree up to the docroot
			to find more user.ini, if not we only scan the current path.
		  */
		if (strncmp(s1, s2, s_len) == 0) {
			ptr = s2 + doc_root_len;
			while ((ptr = strchr(ptr, DEFAULT_SLASH)) != NULL) {
				*ptr = 0;
				crx_parse_user_ini_file(path, PG(user_ini_filename), entry->user_config);
				*ptr = '/';
				ptr++;
			}
		} else {
			crx_parse_user_ini_file(path, PG(user_ini_filename), entry->user_config);
		}

		entry->expires = request_time + PG(user_ini_cache_ttl);
	}

	/* Activate ini entries with values from the user config hash */
	crx_ini_activate_config(entry->user_config, CRX_INI_PERDIR, CRX_INI_STAGE_HTACCESS);
}
/* }}} */

static int sapi_cgi_activate(void) /* {{{ */
{
	fcgi_request *request = (fcgi_request*) SG(server_context);
	char *path, *doc_root, *server_name;
	uint32_t path_len, doc_root_len, server_name_len;

	/* PATH_TRANSLATED should be defined at this stage but better safe than sorry :) */
	if (!SG(request_info).path_translated) {
		return FAILURE;
	}

	if (crx_ini_has_per_host_config()) {
		/* Activate per-host-system-configuration defined in crx.ini and stored into configuration_hash during startup */
		server_name = FCGI_GETENV(request, "SERVER_NAME");
		/* SERVER_NAME should also be defined at this stage..but better check it anyway */
		if (server_name) {
			server_name_len = strlen(server_name);
			server_name = estrndup(server_name, server_name_len);
			crex_str_tolower(server_name, server_name_len);
			crx_ini_activate_per_host_config(server_name, server_name_len);
			efree(server_name);
		}
	}

	if (crx_ini_has_per_dir_config() ||
		(PG(user_ini_filename) && *PG(user_ini_filename))
	) {
		/* Prepare search path */
		path_len = strlen(SG(request_info).path_translated);

		/* Make sure we have trailing slash! */
		if (!IS_SLASH(SG(request_info).path_translated[path_len])) {
			path = emalloc(path_len + 2);
			memcpy(path, SG(request_info).path_translated, path_len + 1);
			path_len = crex_dirname(path, path_len);
			path[path_len++] = DEFAULT_SLASH;
		} else {
			path = estrndup(SG(request_info).path_translated, path_len);
			path_len = crex_dirname(path, path_len);
		}
		path[path_len] = 0;

		/* Activate per-dir-system-configuration defined in crx.ini and stored into configuration_hash during startup */
		crx_ini_activate_per_dir_config(path, path_len); /* Note: for global settings sake we check from root to path */

		/* Load and activate user ini files in path starting from DOCUMENT_ROOT */
		if (PG(user_ini_filename) && *PG(user_ini_filename)) {
			doc_root = FCGI_GETENV(request, "DOCUMENT_ROOT");
			/* DOCUMENT_ROOT should also be defined at this stage..but better check it anyway */
			if (doc_root) {
				doc_root_len = strlen(doc_root);
				if (doc_root_len > 0 && IS_SLASH(doc_root[doc_root_len - 1])) {
					--doc_root_len;
				}

				crx_cgi_ini_activate_user_config(path, path_len, doc_root, doc_root_len);
			}
		}

		efree(path);
	}

	return SUCCESS;
}
/* }}} */

static int sapi_cgi_deactivate(void) /* {{{ */
{
	/* flush only when SAPI was started. The reasons are:
		1. SAPI Deactivate is called from two places: module init and request shutdown
		2. When the first call occurs and the request is not set up, flush fails on FastCGI.
	*/
	if (SG(sapi_started)) {
		if (!parent && !fcgi_finish_request((fcgi_request*)SG(server_context), 0)) {
			crx_handle_aborted_connection();
		}
	}
	return SUCCESS;
}
/* }}} */

static int crx_cgi_startup(sapi_module_struct *sapi_module) /* {{{ */
{
	return crx_module_startup(sapi_module, &cgi_module_entry);
}
/* }}} */

/* {{{ sapi_module_struct cgi_sapi_module */
static sapi_module_struct cgi_sapi_module = {
	"fpm-fcgi",						/* name */
	"FPM/FastCGI",					/* pretty name */

	crx_cgi_startup,				/* startup */
	crx_module_shutdown_wrapper,	/* shutdown */

	sapi_cgi_activate,				/* activate */
	sapi_cgi_deactivate,			/* deactivate */

	sapi_cgibin_ub_write,			/* unbuffered write */
	sapi_cgibin_flush,				/* flush */
	NULL,							/* get uid */
	sapi_cgibin_getenv,				/* getenv */

	crx_error,						/* error handler */

	NULL,							/* header handler */
	sapi_cgi_send_headers,			/* send headers handler */
	NULL,							/* send header handler */

	sapi_cgi_read_post,				/* read POST data */
	sapi_cgi_read_cookies,			/* read Cookies */

	sapi_cgi_register_variables,	/* register server variables */
	sapi_cgi_log_message,			/* Log message */
	NULL,							/* Get request time */
	NULL,							/* Child terminate */

	STANDARD_SAPI_MODULE_PROPERTIES
};
/* }}} */

/* {{{ crx_cgi_usage */
static void crx_cgi_usage(char *argv0)
{
	char *prog;

	prog = strrchr(argv0, '/');
	if (prog) {
		prog++;
	} else {
		prog = "crx";
	}

	crx_printf(	"Usage: %s [-n] [-e] [-h] [-i] [-m] [-v] [-t] [-p <prefix>] [-g <pid>] [-c <file>] [-d foo[=bar]] [-y <file>] [-D] [-F [-O]]\n"
				"  -c <path>|<file> Look for crx.ini file in this directory\n"
				"  -n               No crx.ini file will be used\n"
				"  -d foo[=bar]     Define INI entry foo with value 'bar'\n"
				"  -e               Generate extended information for debugger/profiler\n"
				"  -h               This help\n"
				"  -i               CRX information\n"
				"  -m               Show compiled in modules\n"
				"  -v               Version number\n"
				"  -p, --prefix <dir>\n"
				"                   Specify alternative prefix path to FastCGI process manager (default: %s).\n"
				"  -g, --pid <file>\n"
				"                   Specify the PID file location.\n"
				"  -y, --fpm-config <file>\n"
				"                   Specify alternative path to FastCGI process manager config file.\n"
				"  -t, --test       Test FPM configuration and exit\n"
				"  -D, --daemonize  force to run in background, and ignore daemonize option from config file\n"
				"  -F, --nodaemonize\n"
				"                   force to stay in foreground, and ignore daemonize option from config file\n"
                                "  -O, --force-stderr\n"
                                "                   force output to stderr in nodaemonize even if stderr is not a TTY\n"
				"  -R, --allow-to-run-as-root\n"
				"                   Allow pool to run as root (disabled by default)\n",
				prog, CRX_PREFIX);
}
/* }}} */

/* {{{ is_valid_path
 *
 * some server configurations allow '..' to slip through in the
 * translated path.   We'll just refuse to handle such a path.
 */
static int is_valid_path(const char *path)
{
	const char *p;

	if (!path) {
		return 0;
	}
	p = strstr(path, "..");
	if (p) {
		if ((p == path || IS_SLASH(*(p-1))) &&
			(*(p+2) == 0 || IS_SLASH(*(p+2)))
		) {
			return 0;
		}
		while (1) {
			p = strstr(p+1, "..");
			if (!p) {
				break;
			}
			if (IS_SLASH(*(p-1)) &&
				(*(p+2) == 0 || IS_SLASH(*(p+2)))
			) {
					return 0;
			}
		}
	}
	return 1;
}
/* }}} */

/* {{{ init_request_info

  initializes request_info structure

  specifically in this section we handle proper translations
  for:

  PATH_INFO
	derived from the portion of the URI path following
	the script name but preceding any query data
	may be empty

  PATH_TRANSLATED
    derived by taking any path-info component of the
	request URI and performing any virtual-to-physical
	translation appropriate to map it onto the server's
	document repository structure

	empty if PATH_INFO is empty

	The env var PATH_TRANSLATED **IS DIFFERENT** than the
	request_info.path_translated variable, the latter should
	match SCRIPT_FILENAME instead.

  SCRIPT_NAME
    set to a URL path that could identify the CGI script
	rather than the interpreter.  CRX_SELF is set to this

  REQUEST_URI
    uri section following the domain:port part of a URI

  SCRIPT_FILENAME
    The virtual-to-physical translation of SCRIPT_NAME (as per
	PATH_TRANSLATED)

  These settings are documented at
  http://cgi-spec.golux.com/


  Based on the following URL request:

  http://localhost/info.crx/test?a=b

  should produce, which btw is the same as if
  we were running under mod_cgi on apache (ie. not
  using ScriptAlias directives):

  PATH_INFO=/test
  PATH_TRANSLATED=/docroot/test
  SCRIPT_NAME=/info.crx
  REQUEST_URI=/info.crx/test?a=b
  SCRIPT_FILENAME=/docroot/info.crx
  QUERY_STRING=a=b

  but what we get is (cgi/mod_fastcgi under apache):

  PATH_INFO=/info.crx/test
  PATH_TRANSLATED=/docroot/info.crx/test
  SCRIPT_NAME=/crx/crx-cgi  (from the Action setting I suppose)
  REQUEST_URI=/info.crx/test?a=b
  SCRIPT_FILENAME=/path/to/crx/bin/crx-cgi  (Action setting translated)
  QUERY_STRING=a=b

  Comments in the code below refer to using the above URL in a request

 */
static void init_request_info(void)
{
	fcgi_request *request = (fcgi_request*) SG(server_context);
	char *env_script_filename = FCGI_GETENV(request, "SCRIPT_FILENAME");
	char *env_path_translated = FCGI_GETENV(request, "PATH_TRANSLATED");
	char *script_path_translated = env_script_filename;
	char *ini;
	int apache_was_here = 0;

	/* some broken servers do not have script_filename or argv0
	 * an example, IIS configured in some ways.  then they do more
	 * broken stuff and set path_translated to the cgi script location */
	if (!script_path_translated && env_path_translated) {
		script_path_translated = env_path_translated;
	}

	/* initialize the defaults */
	SG(request_info).path_translated = NULL;
	SG(request_info).request_method = FCGI_GETENV(request, "REQUEST_METHOD");
	SG(request_info).proto_num = 1000;
	SG(request_info).query_string = NULL;
	SG(request_info).request_uri = NULL;
	SG(request_info).content_type = NULL;
	SG(request_info).content_length = 0;
	SG(sapi_headers).http_response_code = 200;

	/* if script_path_translated is not set, then there is no point to carry on
	 * as the response is 404 and there is no further processing. */
	if (script_path_translated) {
		const char *auth;
		char *content_length = FCGI_GETENV(request, "CONTENT_LENGTH");
		char *content_type = FCGI_GETENV(request, "CONTENT_TYPE");
		char *env_path_info = FCGI_GETENV(request, "PATH_INFO");
		char *env_script_name = FCGI_GETENV(request, "SCRIPT_NAME");

		/* Hack for buggy IIS that sets incorrect PATH_INFO */
		char *env_server_software = FCGI_GETENV(request, "SERVER_SOFTWARE");
		if (env_server_software &&
			env_script_name &&
			env_path_info &&
			strncmp(env_server_software, "Microsoft-IIS", sizeof("Microsoft-IIS") - 1) == 0 &&
			strncmp(env_path_info, env_script_name, strlen(env_script_name)) == 0
		) {
			env_path_info = FCGI_PUTENV(request, "ORIG_PATH_INFO", env_path_info);
			env_path_info += strlen(env_script_name);
			if (*env_path_info == 0) {
				env_path_info = NULL;
			}
			env_path_info = FCGI_PUTENV(request, "PATH_INFO", env_path_info);
		}

#define APACHE_PROXY_FCGI_PREFIX "proxy:fcgi://"
#define APACHE_PROXY_BALANCER_PREFIX "proxy:balancer://"
		/* Fix proxy URLs in SCRIPT_FILENAME generated by Apache mod_proxy_fcgi and mod_proxy_balancer:
		 *     proxy:fcgi://localhost:9000/some-dir/info.crx/test?foo=bar
		 *     proxy:balancer://localhost:9000/some-dir/info.crx/test?foo=bar
		 * should be changed to:
		 *     /some-dir/info.crx/test
		 * See: http://bugs.crx.net/bug.crx?id=54152
		 *      http://bugs.crx.net/bug.crx?id=62172
		 *      https://issues.apache.org/bugzilla/show_bug.cgi?id=50851
		 */
		if (env_script_filename &&
			strncasecmp(env_script_filename, APACHE_PROXY_FCGI_PREFIX, sizeof(APACHE_PROXY_FCGI_PREFIX) - 1) == 0) {
			/* advance to first character of hostname */
			char *p = env_script_filename + (sizeof(APACHE_PROXY_FCGI_PREFIX) - 1);
			while (*p != '\0' && *p != '/') {
				p++;	/* move past hostname and port */
			}
			if (*p != '\0') {
				/* Copy path portion in place to avoid memory leak.  Note
				 * that this also affects what script_path_translated points
				 * to. */
				memmove(env_script_filename, p, strlen(p) + 1);
				apache_was_here = 1;
			}
			/* ignore query string if sent by Apache (RewriteRule) */
			p = strchr(env_script_filename, '?');
			if (p) {
				*p =0;
			}
		}

		if (env_script_filename &&
			strncasecmp(env_script_filename, APACHE_PROXY_BALANCER_PREFIX, sizeof(APACHE_PROXY_BALANCER_PREFIX) - 1) == 0) {
			/* advance to first character of hostname */
			char *p = env_script_filename + (sizeof(APACHE_PROXY_BALANCER_PREFIX) - 1);
			while (*p != '\0' && *p != '/') {
				p++;	/* move past hostname and port */
			}
			if (*p != '\0') {
				/* Copy path portion in place to avoid memory leak.  Note
				 * that this also affects what script_path_translated points
				 * to. */
				memmove(env_script_filename, p, strlen(p) + 1);
				apache_was_here = 1;
			}
			/* ignore query string if sent by Apache (RewriteRule) */
			p = strchr(env_script_filename, '?');
			if (p) {
				*p =0;
			}
		}

		if (CGIG(fix_pathinfo)) {
			struct stat st;
			char *real_path = NULL;
			char *env_redirect_url = FCGI_GETENV(request, "REDIRECT_URL");
			char *env_document_root = FCGI_GETENV(request, "DOCUMENT_ROOT");
			char *orig_path_translated = env_path_translated;
			char *orig_path_info = env_path_info;
			char *orig_script_name = env_script_name;
			char *orig_script_filename = env_script_filename;
			int script_path_translated_len;

			if (!env_document_root && PG(doc_root)) {
				env_document_root = FCGI_PUTENV(request, "DOCUMENT_ROOT", PG(doc_root));
			}

			if (!apache_was_here && env_path_translated != NULL && env_redirect_url != NULL &&
			    env_path_translated != script_path_translated &&
			    strcmp(env_path_translated, script_path_translated) != 0) {
				/*
				 * pretty much apache specific.  If we have a redirect_url
				 * then our script_filename and script_name point to the
				 * crx executable
				 * we don't want to do this for the new mod_proxy_fcgi approach,
				 * where redirect_url may also exist but the below will break
				 * with rewrites to PATH_INFO, hence the !apache_was_here check
				 */
				script_path_translated = env_path_translated;
				/* we correct SCRIPT_NAME now in case we don't have PATH_INFO */
				env_script_name = env_redirect_url;
			}

#ifdef __riscos__
			/* Convert path to unix format*/
			__riscosify_control |= __RISCOSIFY_DONT_CHECK_DIR;
			script_path_translated = __unixify(script_path_translated, 0, NULL, 1, 0);
#endif

			/*
			 * if the file doesn't exist, try to extract PATH_INFO out
			 * of it by stat'ing back through the '/'
			 * this fixes url's like /info.crx/test
			 */
			if (script_path_translated &&
				(script_path_translated_len = strlen(script_path_translated)) > 0 &&
				(script_path_translated[script_path_translated_len-1] == '/' ||
				(real_path = tsrm_realpath(script_path_translated, NULL)) == NULL)
			) {
				char *pt = estrndup(script_path_translated, script_path_translated_len);
				int len = script_path_translated_len;
				char *ptr;

				if (pt) {
					while ((ptr = strrchr(pt, '/')) || (ptr = strrchr(pt, '\\'))) {
						*ptr = 0;
						if (stat(pt, &st) == 0 && S_ISREG(st.st_mode)) {
							/*
							 * okay, we found the base script!
							 * work out how many chars we had to strip off;
							 * then we can modify PATH_INFO
							 * accordingly
							 *
							 * we now have the makings of
							 * PATH_INFO=/test
							 * SCRIPT_FILENAME=/docroot/info.crx
							 *
							 * we now need to figure out what docroot is.
							 * if DOCUMENT_ROOT is set, this is easy, otherwise,
							 * we have to play the game of hide and seek to figure
							 * out what SCRIPT_NAME should be
							 */
							int ptlen = strlen(pt);
							int slen = len - ptlen;
							int pilen = env_path_info ? strlen(env_path_info) : 0;
							int tflag = 0;
							char *path_info;
							if (apache_was_here) {
								/* recall that PATH_INFO won't exist */
								path_info = script_path_translated + ptlen;
								tflag = (slen != 0 && (!orig_path_info || strcmp(orig_path_info, path_info) != 0));
							} else {
								path_info = (env_path_info && pilen > slen) ? env_path_info + pilen - slen : NULL;
								tflag = path_info && (orig_path_info != path_info);
							}

							if (tflag) {
								if (orig_path_info) {
									char old;

									FCGI_PUTENV(request, "ORIG_PATH_INFO", orig_path_info);
									old = path_info[0];
									path_info[0] = 0;
									if (!orig_script_name ||
										strcmp(orig_script_name, env_path_info) != 0) {
										if (orig_script_name) {
											FCGI_PUTENV(request, "ORIG_SCRIPT_NAME", orig_script_name);
										}
										SG(request_info).request_uri = FCGI_PUTENV(request, "SCRIPT_NAME", env_path_info);
									} else {
										SG(request_info).request_uri = orig_script_name;
									}
									path_info[0] = old;
								} else if (apache_was_here && env_script_name) {
									/* Using mod_proxy_fcgi and ProxyPass, apache cannot set PATH_INFO
									 * As we can extract PATH_INFO from PATH_TRANSLATED
									 * it is probably also in SCRIPT_NAME and need to be removed
									 */
									char *decoded_path_info = NULL;
									size_t decoded_path_info_len = 0;
									if (strchr(path_info, '%')) {
										decoded_path_info = estrdup(path_info);
										decoded_path_info_len = crx_url_decode(decoded_path_info, strlen(path_info));
									}
									size_t snlen = strlen(env_script_name);
									size_t env_script_file_info_start = 0;
									if (
										(
											snlen > slen &&
											!strcmp(env_script_name + (env_script_file_info_start = snlen - slen), path_info)
										) ||
										(
											decoded_path_info &&
											snlen > decoded_path_info_len &&
											!strcmp(env_script_name + (env_script_file_info_start = snlen - decoded_path_info_len), decoded_path_info)
										)
									) {
										FCGI_PUTENV(request, "ORIG_SCRIPT_NAME", orig_script_name);
										env_script_name[env_script_file_info_start] = 0;
										SG(request_info).request_uri = FCGI_PUTENV(request, "SCRIPT_NAME", env_script_name);
									}
									if (decoded_path_info) {
										efree(decoded_path_info);
									}
								}
								env_path_info = FCGI_PUTENV(request, "PATH_INFO", path_info);
							}
							if (!orig_script_filename ||
								strcmp(orig_script_filename, pt) != 0) {
								if (orig_script_filename) {
									FCGI_PUTENV(request, "ORIG_SCRIPT_FILENAME", orig_script_filename);
								}
								script_path_translated = FCGI_PUTENV(request, "SCRIPT_FILENAME", pt);
							}

							/* figure out docroot
							 * SCRIPT_FILENAME minus SCRIPT_NAME
							 */
							if (env_document_root) {
								int l = strlen(env_document_root);
								int path_translated_len = 0;
								char *path_translated = NULL;

								if (l && env_document_root[l - 1] == '/') {
									--l;
								}

								/* we have docroot, so we should have:
								 * DOCUMENT_ROOT=/docroot
								 * SCRIPT_FILENAME=/docroot/info.crx
								 */

								/* PATH_TRANSLATED = DOCUMENT_ROOT + PATH_INFO */
								path_translated_len = l + (env_path_info ? strlen(env_path_info) : 0);
								path_translated = (char *) emalloc(path_translated_len + 1);
								memcpy(path_translated, env_document_root, l);
								if (env_path_info) {
									memcpy(path_translated + l, env_path_info, (path_translated_len - l));
								}
								path_translated[path_translated_len] = '\0';
								if (orig_path_translated) {
									FCGI_PUTENV(request, "ORIG_PATH_TRANSLATED", orig_path_translated);
								}
								env_path_translated = FCGI_PUTENV(request, "PATH_TRANSLATED", path_translated);
								efree(path_translated);
							} else if (	env_script_name &&
										strstr(pt, env_script_name)
							) {
								/* PATH_TRANSLATED = PATH_TRANSLATED - SCRIPT_NAME + PATH_INFO */
								int ptlen = strlen(pt) - strlen(env_script_name);
								int path_translated_len = ptlen + (env_path_info ? strlen(env_path_info) : 0);
								char *path_translated = NULL;

								path_translated = (char *) emalloc(path_translated_len + 1);
								memcpy(path_translated, pt, ptlen);
								if (env_path_info) {
									memcpy(path_translated + ptlen, env_path_info, path_translated_len - ptlen);
								}
								path_translated[path_translated_len] = '\0';
								if (orig_path_translated) {
									FCGI_PUTENV(request, "ORIG_PATH_TRANSLATED", orig_path_translated);
								}
								env_path_translated = FCGI_PUTENV(request, "PATH_TRANSLATED", path_translated);
								efree(path_translated);
							}
							break;
						}
					}
				} else {
					ptr = NULL;
				}
				if (!ptr) {
					/*
					 * if we stripped out all the '/' and still didn't find
					 * a valid path... we will fail, badly. of course we would
					 * have failed anyway... we output 'no input file' now.
					 */
					if (orig_script_filename) {
						FCGI_PUTENV(request, "ORIG_SCRIPT_FILENAME", orig_script_filename);
					}
					script_path_translated = FCGI_PUTENV(request, "SCRIPT_FILENAME", NULL);
					SG(sapi_headers).http_response_code = 404;
				}
				if (!SG(request_info).request_uri) {
					if (!orig_script_name ||
						strcmp(orig_script_name, env_script_name) != 0) {
						if (orig_script_name) {
							FCGI_PUTENV(request, "ORIG_SCRIPT_NAME", orig_script_name);
						}
						SG(request_info).request_uri = FCGI_PUTENV(request, "SCRIPT_NAME", env_script_name);
					} else {
						SG(request_info).request_uri = orig_script_name;
					}
				}
				if (pt) {
					efree(pt);
				}
			} else {
				/* make sure original values are remembered in ORIG_ copies if we've changed them */
				if (!orig_script_filename ||
					(script_path_translated != orig_script_filename &&
					strcmp(script_path_translated, orig_script_filename) != 0)) {
					if (orig_script_filename) {
						FCGI_PUTENV(request, "ORIG_SCRIPT_FILENAME", orig_script_filename);
					}
					script_path_translated = FCGI_PUTENV(request, "SCRIPT_FILENAME", script_path_translated);
				}
				if (!apache_was_here && env_redirect_url) {
					/* if we used PATH_TRANSLATED to work around Apache mod_fastcgi (but not mod_proxy_fcgi,
					 * hence !apache_was_here) weirdness, strip info accordingly */
					if (orig_path_info) {
						FCGI_PUTENV(request, "ORIG_PATH_INFO", orig_path_info);
						FCGI_PUTENV(request, "PATH_INFO", NULL);
					}
					if (orig_path_translated) {
						FCGI_PUTENV(request, "ORIG_PATH_TRANSLATED", orig_path_translated);
						FCGI_PUTENV(request, "PATH_TRANSLATED", NULL);
					}
				}
				if (env_script_name != orig_script_name) {
					if (orig_script_name) {
						FCGI_PUTENV(request, "ORIG_SCRIPT_NAME", orig_script_name);
					}
					SG(request_info).request_uri = FCGI_PUTENV(request, "SCRIPT_NAME", env_script_name);
				} else {
					SG(request_info).request_uri = env_script_name;
				}
				efree(real_path);
			}
		} else {
			/* pre 4.3 behaviour, shouldn't be used but provides BC */
			if (env_path_info) {
				SG(request_info).request_uri = env_path_info;
			} else {
				SG(request_info).request_uri = env_script_name;
			}
			if (!CGIG(discard_path) && env_path_translated) {
				script_path_translated = env_path_translated;
			}
		}

		if (is_valid_path(script_path_translated)) {
			SG(request_info).path_translated = estrdup(script_path_translated);
		}

		/* FIXME - Work out proto_num here */
		SG(request_info).query_string = FCGI_GETENV(request, "QUERY_STRING");
		SG(request_info).content_type = (content_type ? content_type : "" );
		SG(request_info).content_length = (content_length ? atol(content_length) : 0);

		/* The CGI RFC allows servers to pass on unvalidated Authorization data */
		auth = FCGI_GETENV(request, "HTTP_AUTHORIZATION");
		crx_handle_auth_data(auth);
	}

	/* INI stuff */
	ini = FCGI_GETENV(request, "CRX_VALUE");
	if (ini) {
		int mode = CREX_INI_USER;
		char *tmp;
		spprintf(&tmp, 0, "%s\n", ini);
		crex_parse_ini_string(tmp, 1, CREX_INI_SCANNER_NORMAL, (crex_ini_parser_cb_t)fastcgi_ini_parser, &mode);
		efree(tmp);
	}

	ini = FCGI_GETENV(request, "CRX_ADMIN_VALUE");
	if (ini) {
		int mode = CREX_INI_SYSTEM;
		char *tmp;
		spprintf(&tmp, 0, "%s\n", ini);
		crex_parse_ini_string(tmp, 1, CREX_INI_SCANNER_NORMAL, (crex_ini_parser_cb_t)fastcgi_ini_parser, &mode);
		efree(tmp);
	}
}
/* }}} */

static fcgi_request *fpm_init_request(int listen_fd) /* {{{ */ {
	fcgi_request *req = fcgi_init_request(listen_fd,
		fpm_request_accepting,
		fpm_request_reading_headers,
		fpm_request_finished);
	return req;
}
/* }}} */

static void fastcgi_ini_parser(zval *arg1, zval *arg2, zval *arg3, int callback_type, void *arg) /* {{{ */
{
	int *mode = (int *)arg;
	char *key;
	char *value = NULL;
	struct key_value_s kv;

	if (!mode || !arg1) return;

	if (callback_type != CREX_INI_PARSER_ENTRY) {
		zlog(ZLOG_ERROR, "Passing INI directive through FastCGI: only classic entries are allowed");
		return;
	}

	key = C_STRVAL_P(arg1);

	if (!key || strlen(key) < 1) {
		zlog(ZLOG_ERROR, "Passing INI directive through FastCGI: empty key");
		return;
	}

	if (arg2) {
		value = C_STRVAL_P(arg2);
	}

	if (!value) {
		zlog(ZLOG_ERROR, "Passing INI directive through FastCGI: empty value for key '%s'", key);
		return;
	}

	kv.key = key;
	kv.value = value;
	kv.next = NULL;
	if (fpm_crx_apply_defines_ex(&kv, *mode) == -1) {
		zlog(ZLOG_ERROR, "Passing INI directive through FastCGI: unable to set '%s'", key);
	}
}
/* }}} */

CRX_INI_BEGIN()
	STD_CRX_INI_BOOLEAN("cgi.rfc2616_headers",     "0",  CRX_INI_ALL,    OnUpdateBool,   rfc2616_headers, crx_cgi_globals_struct, crx_cgi_globals)
	STD_CRX_INI_BOOLEAN("cgi.nph",                 "0",  CRX_INI_ALL,    OnUpdateBool,   nph, crx_cgi_globals_struct, crx_cgi_globals)
	STD_CRX_INI_BOOLEAN("cgi.force_redirect",      "1",  CRX_INI_SYSTEM, OnUpdateBool,   force_redirect, crx_cgi_globals_struct, crx_cgi_globals)
	STD_CRX_INI_ENTRY("cgi.redirect_status_env", NULL, CRX_INI_SYSTEM, OnUpdateString, redirect_status_env, crx_cgi_globals_struct, crx_cgi_globals)
	STD_CRX_INI_BOOLEAN("cgi.fix_pathinfo",        "1",  CRX_INI_SYSTEM, OnUpdateBool,   fix_pathinfo, crx_cgi_globals_struct, crx_cgi_globals)
	STD_CRX_INI_BOOLEAN("cgi.discard_path",        "0",  CRX_INI_SYSTEM, OnUpdateBool,   discard_path, crx_cgi_globals_struct, crx_cgi_globals)
	STD_CRX_INI_BOOLEAN("fastcgi.logging",         "1",  CRX_INI_SYSTEM, OnUpdateBool,   fcgi_logging, crx_cgi_globals_struct, crx_cgi_globals)
	STD_CRX_INI_ENTRY("fastcgi.error_header",    NULL, CRX_INI_SYSTEM, OnUpdateString, error_header, crx_cgi_globals_struct, crx_cgi_globals)
	STD_CRX_INI_ENTRY("fpm.config",    NULL, CRX_INI_SYSTEM, OnUpdateString, fpm_config, crx_cgi_globals_struct, crx_cgi_globals)
CRX_INI_END()

/* {{{ crx_cgi_globals_ctor */
static void crx_cgi_globals_ctor(crx_cgi_globals_struct *crx_cgi_globals)
{
	crx_cgi_globals->rfc2616_headers = 0;
	crx_cgi_globals->nph = 0;
	crx_cgi_globals->force_redirect = 1;
	crx_cgi_globals->redirect_status_env = NULL;
	crx_cgi_globals->fix_pathinfo = 1;
	crx_cgi_globals->discard_path = 0;
	crx_cgi_globals->fcgi_logging = 1;
	crx_cgi_globals->fcgi_logging_request_started = false;
	crex_hash_init(&crx_cgi_globals->user_config_cache, 0, NULL, user_config_cache_entry_dtor, 1);
	crx_cgi_globals->error_header = NULL;
	crx_cgi_globals->fpm_config = NULL;
}
/* }}} */

/* {{{ CRX_MINIT_FUNCTION */
static CRX_MINIT_FUNCTION(cgi)
{
#ifdef ZTS
	ts_allocate_id(&crx_cgi_globals_id, sizeof(crx_cgi_globals_struct), (ts_allocate_ctor) crx_cgi_globals_ctor, NULL);
#else
	crx_cgi_globals_ctor(&crx_cgi_globals);
#endif
	REGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION */
static CRX_MSHUTDOWN_FUNCTION(cgi)
{
	crex_hash_destroy(&CGIG(user_config_cache));

	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
static CRX_MINFO_FUNCTION(cgi)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "crx-fpm", "active");
	crx_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

CRX_FUNCTION(fastcgi_finish_request) /* {{{ */
{
	fcgi_request *request = (fcgi_request*) SG(server_context);

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if (!fcgi_is_closed(request)) {
		crx_output_end_all();
		crx_header();

		fcgi_end(request);
		fcgi_close(request, 0, 0);
		RETURN_TRUE;
	}

	RETURN_FALSE;

}
/* }}} */

CRX_FUNCTION(apache_request_headers) /* {{{ */
{
	fcgi_request *request;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	array_init(return_value);
	if ((request = (fcgi_request*) SG(server_context))) {
		fcgi_loadenv(request, sapi_add_request_header, return_value);
	}
} /* }}} */

/* {{{ Returns the status of the fastcgi process manager */
CRX_FUNCTION(fpm_get_status) /* {{{ */
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if (fpm_status_export_to_zval(return_value)) {
		RETURN_FALSE;
	}
}
/* }}} */

static crex_module_entry cgi_module_entry = {
	STANDARD_MODULE_HEADER,
	"cgi-fcgi",
	ext_functions,
	CRX_MINIT(cgi),
	CRX_MSHUTDOWN(cgi),
	NULL,
	NULL,
	CRX_MINFO(cgi),
	CRX_VERSION,
	STANDARD_MODULE_PROPERTIES
};

/* {{{ main */
int main(int argc, char *argv[])
{
	int exit_status = FPM_EXIT_OK;
	int cgi = 0, c, use_extended_info = 0;
	crex_file_handle file_handle;

	/* temporary locals */
	int orig_optind = crx_optind;
	char *orig_optarg = crx_optarg;
	struct crx_ini_builder ini_builder;
	/* end of temporary locals */

	int max_requests = 0;
	int requests = 0;
	int fcgi_fd = 0;
	fcgi_request *request;
	char *fpm_config = NULL;
	char *fpm_prefix = NULL;
	char *fpm_pid = NULL;
	int test_conf = 0;
	int force_daemon = -1;
	int force_stderr = 0;
	int crx_information = 0;
	int crx_allow_to_run_as_root = 0;
#if CREX_RC_DEBUG
	bool old_rc_debug;
#endif

#if defined(SIGPIPE) && defined(SIG_IGN)
	signal(SIGPIPE, SIG_IGN); /* ignore SIGPIPE in standalone mode so
								that sockets created via fsockopen()
								don't kill CRX if the remote site
								closes it.  in apache|apxs mode apache
								does that for us!  thies@thieso.net
								20000419 */

	if (0 > fpm_signals_init_mask() || 0 > fpm_signals_block()) {
		zlog(ZLOG_WARNING, "Could die in the case of too early reload signal");
	}
	zlog(ZLOG_DEBUG, "Blocked some signals");
#endif

#ifdef ZTS
	crx_tsrm_startup();
#endif

	crex_signal_startup();

	sapi_startup(&cgi_sapi_module);
	cgi_sapi_module.crx_ini_path_override = NULL;
	cgi_sapi_module.crx_ini_ignore_cwd = 1;

#ifndef HAVE_ATTRIBUTE_WEAK
	fcgi_set_logger(fpm_fcgi_log);
#endif

	fcgi_init();

	crx_ini_builder_init(&ini_builder);

	while ((c = crx_getopt(argc, argv, OPTIONS, &crx_optarg, &crx_optind, 0, 2)) != -1) {
		switch (c) {
			case 'c':
				if (cgi_sapi_module.crx_ini_path_override) {
					free(cgi_sapi_module.crx_ini_path_override);
				}
				cgi_sapi_module.crx_ini_path_override = strdup(crx_optarg);
				break;

			case 'n':
				cgi_sapi_module.crx_ini_ignore = 1;
				break;

			case 'd':
				/* define ini entries on command line */
				crx_ini_builder_define(&ini_builder, crx_optarg);
				/* main can terminate without finishing or deiniting the ini builder, call finish each iteration to avoid leaking the buffer */
				cgi_sapi_module.ini_entries = crx_ini_builder_finish(&ini_builder);
				break;

			case 'y':
				fpm_config = crx_optarg;
				break;

			case 'p':
				fpm_prefix = crx_optarg;
				break;

			case 'g':
				fpm_pid = crx_optarg;
				break;

			case 'e': /* enable extended info output */
				use_extended_info = 1;
				break;

			case 't':
				test_conf++;
				break;

			case 'm': /* list compiled in modules */
				cgi_sapi_module.startup(&cgi_sapi_module);
				crx_output_activate();
				SG(headers_sent) = 1;
				crx_printf("[CRX Modules]\n");
				print_modules();
				crx_printf("\n[Crex Modules]\n");
				print_extensions();
				crx_printf("\n");
				crx_output_end_all();
				crx_output_deactivate();
				fcgi_shutdown();
				exit_status = FPM_EXIT_OK;
				goto out;

			case 'i': /* crx info & quit */
				crx_information = 1;
				break;

			case 'R': /* allow to run as root */
				crx_allow_to_run_as_root = 1;
				break;

			case 'D': /* daemonize */
				force_daemon = 1;
				break;

			case 'F': /* nodaemonize */
				force_daemon = 0;
				break;

			case 'O': /* force stderr even on non tty */
				force_stderr = 1;
				break;

			default:
			case 'h':
			case '?':
			case CRX_GETOPT_INVALID_ARG:
				cgi_sapi_module.startup(&cgi_sapi_module);
				crx_output_activate();
				SG(headers_sent) = 1;
				crx_cgi_usage(argv[0]);
				crx_output_end_all();
				crx_output_deactivate();
				fcgi_shutdown();
				exit_status = (c != CRX_GETOPT_INVALID_ARG) ? FPM_EXIT_OK : FPM_EXIT_USAGE;
				goto out;

			case 'v': /* show crx version & quit */
				cgi_sapi_module.startup(&cgi_sapi_module);
				if (crx_request_startup() == FAILURE) {
					SG(server_context) = NULL;
					crx_module_shutdown();
					return FPM_EXIT_SOFTWARE;
				}
				SG(headers_sent) = 1;
				SG(request_info).no_headers = 1;

#if CREX_DEBUG
				crx_printf("CRX %s (%s) (built: %s %s) (DEBUG)\nCopyright (c) The CRX Group\n%s", CRX_VERSION, sapi_module.name, __DATE__,        __TIME__, get_crex_version());
#else
				crx_printf("CRX %s (%s) (built: %s %s)\nCopyright (c) The CRX Group\n%s", CRX_VERSION, sapi_module.name, __DATE__, __TIME__,      get_crex_version());
#endif
				crx_request_shutdown((void *) 0);
				fcgi_shutdown();
				exit_status = FPM_EXIT_OK;
				goto out;
		}
	}

	cgi_sapi_module.ini_entries = crx_ini_builder_finish(&ini_builder);

	if (crx_information) {
		cgi_sapi_module.crxinfo_as_text = 1;
		cgi_sapi_module.startup(&cgi_sapi_module);
		if (crx_request_startup() == FAILURE) {
			SG(server_context) = NULL;
			crx_module_shutdown();
			return FPM_EXIT_SOFTWARE;
		}
		SG(headers_sent) = 1;
		SG(request_info).no_headers = 1;
		crx_print_info(0xFFFFFFFF);
		crx_request_shutdown((void *) 0);
		fcgi_shutdown();
		exit_status = FPM_EXIT_OK;
		goto out;
	}

	/* No other args are permitted here as there is no interactive mode */
	if (argc != crx_optind) {
		cgi_sapi_module.startup(&cgi_sapi_module);
		crx_output_activate();
		SG(headers_sent) = 1;
		crx_cgi_usage(argv[0]);
		crx_output_end_all();
		crx_output_deactivate();
		fcgi_shutdown();
		exit_status = FPM_EXIT_USAGE;
		goto out;
	}

	crx_optind = orig_optind;
	crx_optarg = orig_optarg;

#ifdef ZTS
	SG(request_info).path_translated = NULL;
#endif

	cgi_sapi_module.additional_functions = NULL;
	cgi_sapi_module.executable_location = argv[0];

	/* startup after we get the above ini override se we get things right */
	if (cgi_sapi_module.startup(&cgi_sapi_module) == FAILURE) {
#ifdef ZTS
		tsrm_shutdown();
#endif
		return FPM_EXIT_SOFTWARE;
	}

	if (use_extended_info) {
		CG(compiler_options) |= CREX_COMPILE_EXTENDED_INFO;
	}

	/* check force_cgi after startup, so we have proper output */
	if (cgi && CGIG(force_redirect)) {
		/* Apache will generate REDIRECT_STATUS,
		 * Netscape and redirect.so will generate HTTP_REDIRECT_STATUS.
		 * redirect.so and installation instructions available from
		 * http://www.koehntopp.de/crx.
		 *   -- kk@netuse.de
		 */
		if (!getenv("REDIRECT_STATUS") &&
			!getenv ("HTTP_REDIRECT_STATUS") &&
			/* this is to allow a different env var to be configured
			 * in case some server does something different than above */
			(!CGIG(redirect_status_env) || !getenv(CGIG(redirect_status_env)))
		) {
			crex_try {
				SG(sapi_headers).http_response_code = 400;
				PUTS("<b>Security Alert!</b> The CRX CGI cannot be accessed directly.\n\n\
<p>This CRX CGI binary was compiled with force-cgi-redirect enabled.  This\n\
means that a page will only be served up if the REDIRECT_STATUS CGI variable is\n\
set, e.g. via an Apache Action directive.</p>\n\
<p>For more information as to <i>why</i> this behaviour exists, see the <a href=\"http://crx.net/security.cgi-bin\">\
manual page for CGI security</a>.</p>\n\
<p>For more information about changing this behaviour or re-enabling this webserver,\n\
consult the installation file that came with this distribution, or visit \n\
<a href=\"http://crx.net/install.windows\">the manual page</a>.</p>\n");
			} crex_catch {
			} crex_end_try();
#if defined(ZTS) && !CRX_DEBUG
			/* XXX we're crashing here in msvc6 debug builds at
			 * crx_message_handler_for_crex:839 because
			 * SG(request_info).path_translated is an invalid pointer.
			 * It still happens even though I set it to null, so something
			 * weird is going on.
			 */
			tsrm_shutdown();
#endif
			return FPM_EXIT_SOFTWARE;
		}
	}

#if CREX_RC_DEBUG
	old_rc_debug = crex_rc_debug;
	crex_rc_debug = 0;
#endif

	enum fpm_init_return_status ret = fpm_init(argc, argv, fpm_config ? fpm_config : CGIG(fpm_config), fpm_prefix, fpm_pid, test_conf, crx_allow_to_run_as_root, force_daemon, force_stderr);

#if CREX_RC_DEBUG
	crex_rc_debug = old_rc_debug;
#endif

	if (ret == FPM_INIT_ERROR) {
		if (fpm_globals.send_config_pipe[1]) {
			int writeval = 0;
			zlog(ZLOG_DEBUG, "Sending \"0\" (error) to parent via fd=%d", fpm_globals.send_config_pipe[1]);
			crex_quiet_write(fpm_globals.send_config_pipe[1], &writeval, sizeof(writeval));
			close(fpm_globals.send_config_pipe[1]);
		}
		exit_status = FPM_EXIT_CONFIG;
		goto out;
	} else if (ret == FPM_INIT_EXIT_OK) {
		exit_status = FPM_EXIT_OK;
		goto out;
	}

	if (fpm_globals.send_config_pipe[1]) {
		int writeval = 1;
		zlog(ZLOG_DEBUG, "Sending \"1\" (OK) to parent via fd=%d", fpm_globals.send_config_pipe[1]);
		crex_quiet_write(fpm_globals.send_config_pipe[1], &writeval, sizeof(writeval));
		close(fpm_globals.send_config_pipe[1]);
	}
	fpm_is_running = 1;

	fcgi_fd = fpm_run(&max_requests);
	parent = 0;

	/* onced forked tell zlog to also send messages through sapi_cgi_log_fastcgi() */
	zlog_set_external_logger(sapi_cgi_log_fastcgi);

	/* make crx call us to get _ENV vars */
	crx_crx_import_environment_variables = crx_import_environment_variables;
	crx_import_environment_variables = cgi_crx_import_environment_variables;

	/* library is already initialized, now init our request */
	request = fpm_init_request(fcgi_fd);

	crex_first_try {
		while (EXPECTED(fcgi_accept_request(request) >= 0)) {
			char *primary_script = NULL;
			request_body_fd = -1;
			SG(server_context) = (void *) request;
			CGIG(fcgi_logging_request_started) = false;
			init_request_info();

			fpm_request_info();

			/* request startup only after we've done all we can to
			 *            get path_translated */
			if (UNEXPECTED(crx_request_startup() == FAILURE)) {
				fcgi_finish_request(request, 1);
				SG(server_context) = NULL;
				crx_module_shutdown();
				return FPM_EXIT_SOFTWARE;
			}

			/* check if request_method has been sent.
			 * if not, it's certainly not an HTTP over fcgi request */
			if (UNEXPECTED(!SG(request_info).request_method)) {
				goto fastcgi_request_done;
			}

			if (UNEXPECTED(fpm_status_handle_request())) {
				goto fastcgi_request_done;
			}

			/* If path_translated is NULL, terminate here with a 404 */
			if (UNEXPECTED(!SG(request_info).path_translated)) {
				crex_try {
					zlog(ZLOG_DEBUG, "Primary script unknown");
					SG(sapi_headers).http_response_code = 404;
					PUTS("File not found.\n");
				} crex_catch {
				} crex_end_try();
				goto fastcgi_request_done;
			}

			if (UNEXPECTED(fpm_crx_limit_extensions(SG(request_info).path_translated))) {
				SG(sapi_headers).http_response_code = 403;
				PUTS("Access denied.\n");
				goto fastcgi_request_done;
			}

			/*
			 * have to duplicate SG(request_info).path_translated to be able to log errors
			 * crx_fopen_primary_script seems to delete SG(request_info).path_translated on failure
			 */
			primary_script = estrdup(SG(request_info).path_translated);

			/* path_translated exists, we can continue ! */
			if (UNEXPECTED(crx_fopen_primary_script(&file_handle) == FAILURE)) {
				crex_try {
					zlog(ZLOG_ERROR, "Unable to open primary script: %s (%s)", primary_script, strerror(errno));
					if (errno == EACCES) {
						SG(sapi_headers).http_response_code = 403;
						PUTS("Access denied.\n");
					} else {
						SG(sapi_headers).http_response_code = 404;
						PUTS("No input file specified.\n");
					}
				} crex_catch {
				} crex_end_try();
				/* We want to serve more requests if this is fastcgi so cleanup and continue,
				 * request shutdown is handled later. */
			} else {
				fpm_request_executing();

				/* Reset exit status from the previous execution */
				EG(exit_status) = 0;

				crx_execute_script(&file_handle);
			}

			/* Without opcache, or the first time with opcache, the file handle will be placed
			 * in the CG(open_files) list by open_file_for_scanning(). Starting from the second
			 * request in opcache, the file handle won't be in the list and therefore won't be destroyed for us. */
			if (!file_handle.in_list) {
				crex_destroy_file_handle(&file_handle);
			}

fastcgi_request_done:
			if (EXPECTED(primary_script)) {
				efree(primary_script);
			}

			if (UNEXPECTED(request_body_fd != -1)) {
				close(request_body_fd);
			}
			request_body_fd = -2;

			if (UNEXPECTED(EG(exit_status) == 255)) {
				if (CGIG(error_header) && *CGIG(error_header) && !SG(headers_sent)) {
					sapi_header_line ctr = {0};

					ctr.line = CGIG(error_header);
					ctr.line_len = strlen(CGIG(error_header));
					sapi_header_op(SAPI_HEADER_REPLACE, &ctr);
				}
			}

			fpm_request_end();
			fpm_log_write(NULL);

			efree(SG(request_info).path_translated);
			SG(request_info).path_translated = NULL;

			crx_request_shutdown((void *) 0);

			fpm_stdio_flush_child();

			requests++;
			if (UNEXPECTED(max_requests && (requests == max_requests))) {
				fcgi_request_set_keep(request, 0);
				fcgi_finish_request(request, 0);
				break;
			}
			/* end of fastcgi loop */
		}
		fcgi_destroy_request(request);
		fcgi_shutdown();

		if (cgi_sapi_module.crx_ini_path_override) {
			free(cgi_sapi_module.crx_ini_path_override);
		}
		crx_ini_builder_deinit(&ini_builder);
	} crex_catch {
		exit_status = FPM_EXIT_SOFTWARE;
	} crex_end_try();

out:

	SG(server_context) = NULL;
	crx_module_shutdown();

	if (parent) {
		sapi_shutdown();
	}

#ifdef ZTS
	tsrm_shutdown();
#endif

	return exit_status;
}
/* }}} */
