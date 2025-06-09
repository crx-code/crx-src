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
   | Author: Zeev Suraski <zeev@crx.net>                                  |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_GLOBALS_H
#define CRX_GLOBALS_H

#include "crex_globals.h"

#include <stdint.h>

typedef struct _crx_core_globals crx_core_globals;

#ifdef ZTS
# define PG(v) CREX_TSRMG_FAST(core_globals_offset, crx_core_globals *, v)
extern CRXAPI int core_globals_id;
extern CRXAPI size_t core_globals_offset;
#else
# define PG(v) (core_globals.v)
extern CREX_API struct _crx_core_globals core_globals;
#endif

/* Error display modes */
#define CRX_DISPLAY_ERRORS_STDOUT	1
#define CRX_DISPLAY_ERRORS_STDERR	2

/* Track vars */
#define TRACK_VARS_POST		0
#define TRACK_VARS_GET		1
#define TRACK_VARS_COOKIE	2
#define TRACK_VARS_SERVER	3
#define TRACK_VARS_ENV		4
#define TRACK_VARS_FILES	5
#define TRACK_VARS_REQUEST	6

struct _crx_tick_function_entry;

typedef struct _arg_separators {
	char *output;
	char *input;
} arg_separators;

struct _crx_core_globals {
	crex_long output_buffering;

	bool implicit_flush;

	bool enable_dl;

	uint8_t display_errors;
	bool display_startup_errors;
	bool log_errors;
	bool ignore_repeated_errors;
	bool ignore_repeated_source;
	bool report_memleaks;

	char *output_handler;

	char *unserialize_callback_func;
	crex_long serialize_precision;

	crex_long memory_limit;
	crex_long max_input_time;

	char *error_log;

	char *doc_root;
	char *user_dir;
	char *include_path;
	char *open_basedir;
	bool open_basedir_modified;
	char *extension_dir;
	char *crx_binary;
	char *sys_temp_dir;

	char *upload_tmp_dir;
	crex_long upload_max_filesize;

	char *error_append_string;
	char *error_prepend_string;

	char *auto_prepend_file;
	char *auto_append_file;

	char *input_encoding;
	char *internal_encoding;
	char *output_encoding;

	arg_separators arg_separator;

	char *variables_order;

	HashTable rfc1867_protected_variables;

	short connection_status;
	bool ignore_user_abort;

	unsigned char header_is_being_sent;

	crex_llist tick_functions;

	zval http_globals[6];

	bool expose_crx;

	bool register_argc_argv;
	bool auto_globals_jit;

	bool html_errors;
	bool xmlrpc_errors;

	char *docref_root;
	char *docref_ext;

	crex_long xmlrpc_error_number;

	bool activated_auto_globals[8];

	bool modules_activated;
	bool file_uploads;
	bool during_request_startup;
	bool allow_url_fopen;
	bool enable_post_data_reading;
	bool report_crex_debug;

	int last_error_type;
	int last_error_lineno;
	crex_string *last_error_message;
	crex_string *last_error_file;

	char *crx_sys_temp_dir;

	char *disable_classes;
	crex_long max_input_nesting_level;
	crex_long max_input_vars;

	char *user_ini_filename;
	crex_long user_ini_cache_ttl;

	char *request_order;

	char *mail_log;
	bool mail_x_header;
	bool mail_mixed_lf_and_crlf;

	bool in_error_log;

	bool allow_url_include;
#ifdef CRX_WIN32
	bool com_initialized;
#endif
	bool in_user_include;

#ifdef CRX_WIN32
	bool windows_show_crt_warning;
#endif

	bool have_called_openlog;
	crex_long syslog_facility;
	char *syslog_ident;
	crex_long syslog_filter;
	crex_long error_log_mode;
};


#endif /* CRX_GLOBALS_H */
