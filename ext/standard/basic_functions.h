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

#ifndef BASIC_FUNCTIONS_H
#define BASIC_FUNCTIONS_H

#include <sys/stat.h>
#include <wchar.h>

#include "crx_filestat.h"

#include "crex_highlight.h"

#include "url_scanner_ex.h"

/* for MT_N */
#include "ext/random/crx_random.h"

#if defined(_WIN32) && !defined(__clang__)
#include <intrin.h>
#endif

extern crex_module_entry basic_functions_module;
#define basic_functions_module_ptr &basic_functions_module

CRX_MINIT_FUNCTION(basic);
CRX_MSHUTDOWN_FUNCTION(basic);
CRX_RINIT_FUNCTION(basic);
CRX_RSHUTDOWN_FUNCTION(basic);
CRX_MINFO_FUNCTION(basic);

CREX_API void crx_get_highlight_struct(crex_syntax_highlighter_ini *syntax_highlighter_ini);

CRX_MINIT_FUNCTION(user_filters);
CRX_RSHUTDOWN_FUNCTION(user_filters);
CRX_RSHUTDOWN_FUNCTION(browscap);

/* Left for BC (not binary safe!) */
CRXAPI int _crx_error_log(int opt_err, const char *message, const char *opt, const char *headers);
CRXAPI int _crx_error_log_ex(int opt_err, const char *message, size_t message_len, const char *opt, const char *headers);
CRXAPI int crx_prefix_varname(zval *result, crex_string *prefix, const char *var_name, size_t var_name_len, bool add_underscore);

/* Deprecated type aliases -- use the standard types instead */
typedef uint32_t crx_uint32;
typedef int32_t crx_int32;

typedef struct _crx_basic_globals {
	HashTable *user_shutdown_function_names;
	HashTable putenv_ht;
	crex_string *strtok_string;
	crex_string *ctype_string; /* current LC_CTYPE locale (or NULL for 'C') */
	bool locale_changed;   /* locale was changed and has to be restored */
	char *strtok_last;
	char strtok_table[256];
	size_t strtok_len;
	crex_fcall_info user_compare_fci;
	crex_fcall_info_cache user_compare_fci_cache;
	crex_llist *user_tick_functions;

	zval active_ini_file_section;

	/* pageinfo.c */
	crex_long page_uid;
	crex_long page_gid;
	crex_long page_inode;
	time_t page_mtime;

	/* filestat.c && main/streams/streams.c */
	crex_string *CurrentStatFile, *CurrentLStatFile;
	crx_stream_statbuf ssb, lssb;

	/* syslog.c */
	char *syslog_device;

	/* var.c */
	unsigned serialize_lock; /* whether to use the locally supplied var_hash instead (__sleep/__wakeup) */
	struct {
		struct crx_serialize_data *data;
		unsigned level;
	} serialize;
	struct {
		struct crx_unserialize_data *data;
		unsigned level;
	} unserialize;

	/* url_scanner_ex.re */
	url_adapt_state_ex_t url_adapt_session_ex;
	HashTable url_adapt_session_hosts_ht;
	url_adapt_state_ex_t url_adapt_output_ex;
	HashTable url_adapt_output_hosts_ht;
	HashTable *user_filter_map;

	/* file.c */
#if defined(_REENTRANT)
	mbstate_t mblen_state;
#endif

	int umask;
	crex_long unserialize_max_depth;
} crx_basic_globals;

#ifdef ZTS
#define BG(v) CREX_TSRMG(basic_globals_id, crx_basic_globals *, v)
CRXAPI extern int basic_globals_id;
#else
#define BG(v) (basic_globals.v)
CRXAPI extern crx_basic_globals basic_globals;
#endif

CRXAPI crex_string *crx_getenv(const char *str, size_t str_len);

CRXAPI double crx_get_nan(void);
CRXAPI double crx_get_inf(void);

typedef struct _crx_shutdown_function_entry {
	crex_fcall_info fci;
	crex_fcall_info_cache fci_cache;
} crx_shutdown_function_entry;

CRXAPI extern bool register_user_shutdown_function(const char *function_name, size_t function_len, crx_shutdown_function_entry *shutdown_function_entry);
CRXAPI extern bool remove_user_shutdown_function(const char *function_name, size_t function_len);
CRXAPI extern bool append_user_shutdown_function(crx_shutdown_function_entry *shutdown_function_entry);

CRXAPI void crx_call_shutdown_functions(void);
CRXAPI void crx_free_shutdown_functions(void);


#endif /* BASIC_FUNCTIONS_H */
