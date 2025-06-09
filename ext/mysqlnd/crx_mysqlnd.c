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
  | Authors: Andrey Hristov <andrey@crx.net>                             |
  |          Ulf Wendel <uw@crx.net>                                     |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "crx.h"
#include "mysqlnd.h"
#include "mysqlnd_priv.h"
#include "mysqlnd_debug.h"
#include "mysqlnd_statistics.h"
#include "mysqlnd_reverse_api.h"
#include "ext/standard/info.h"
#include "crex_smart_str.h"

/* {{{ mysqlnd_minfo_print_hash */
CRXAPI void
mysqlnd_minfo_print_hash(zval *values)
{
	zval *values_entry;
	crex_string	*string_key;

	CREX_HASH_MAP_FOREACH_STR_KEY_VAL(C_ARRVAL_P(values), string_key, values_entry) {
		convert_to_string(values_entry);
		crx_info_print_table_row(2, ZSTR_VAL(string_key), C_STRVAL_P(values_entry));
	} CREX_HASH_FOREACH_END();
}
/* }}} */


/* {{{ mysqlnd_minfo_dump_loaded_plugins */
static int
mysqlnd_minfo_dump_loaded_plugins(zval *el, void * buf)
{
	smart_str * buffer = (smart_str *) buf;
	struct st_mysqlnd_plugin_header * plugin_header = (struct st_mysqlnd_plugin_header *)C_PTR_P(el);
	if (plugin_header->plugin_name) {
		if (buffer->s) {
			smart_str_appendc(buffer, ',');
		}
		smart_str_appends(buffer, plugin_header->plugin_name);
	}
	return CREX_HASH_APPLY_KEEP;
}
/* }}} */


/* {{{ mysqlnd_minfo_dump_api_plugins */
static void
mysqlnd_minfo_dump_api_plugins(smart_str * buffer)
{
	HashTable *ht = mysqlnd_reverse_api_get_api_list();
	MYSQLND_REVERSE_API *ext;

	CREX_HASH_MAP_FOREACH_PTR(ht, ext) {
		if (buffer->s) {
			smart_str_appendc(buffer, ',');
		}
		smart_str_appends(buffer, ext->module->name);
	} CREX_HASH_FOREACH_END();
}
/* }}} */


/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(mysqlnd)
{
	char buf[32];

	crx_info_print_table_start();
	crx_info_print_table_row(2, "mysqlnd", "enabled");
	crx_info_print_table_row(2, "Version", mysqlnd_get_client_info());
	crx_info_print_table_row(2, "Compression",
#ifdef MYSQLND_COMPRESSION_ENABLED
								"supported");
#else
								"not supported");
#endif
	crx_info_print_table_row(2, "core SSL",
#ifdef MYSQLND_SSL_SUPPORTED
								"supported");
#else
								"not supported");
#endif
	crx_info_print_table_row(2, "extended SSL",
#ifdef MYSQLND_HAVE_SSL
								"supported");
#else
								"not supported");
#endif
	snprintf(buf, sizeof(buf), CREX_LONG_FMT, MYSQLND_G(net_cmd_buffer_size));
	crx_info_print_table_row(2, "Command buffer size", buf);
	snprintf(buf, sizeof(buf), CREX_LONG_FMT, MYSQLND_G(net_read_buffer_size));
	crx_info_print_table_row(2, "Read buffer size", buf);
	snprintf(buf, sizeof(buf), CREX_LONG_FMT, MYSQLND_G(net_read_timeout));
	crx_info_print_table_row(2, "Read timeout", buf);
	crx_info_print_table_row(2, "Collecting statistics", MYSQLND_G(collect_statistics)? "Yes":"No");
	crx_info_print_table_row(2, "Collecting memory statistics", MYSQLND_G(collect_memory_statistics)? "Yes":"No");

	crx_info_print_table_row(2, "Tracing", MYSQLND_G(debug)? MYSQLND_G(debug):"n/a");

	/* loaded plugins */
	{
		smart_str tmp_str = {0};
		mysqlnd_plugin_apply_with_argument(mysqlnd_minfo_dump_loaded_plugins, &tmp_str);
		smart_str_0(&tmp_str);
		crx_info_print_table_row(2, "Loaded plugins", tmp_str.s? ZSTR_VAL(tmp_str.s) : "");
		smart_str_free(&tmp_str);

		mysqlnd_minfo_dump_api_plugins(&tmp_str);
		smart_str_0(&tmp_str);
		crx_info_print_table_row(2, "API Extensions", tmp_str.s? ZSTR_VAL(tmp_str.s) : "");
		smart_str_free(&tmp_str);
	}

	crx_info_print_table_end();
}
/* }}} */


CRXAPI CREX_DECLARE_MODULE_GLOBALS(mysqlnd)


/* {{{ CRX_GINIT_FUNCTION */
static CRX_GINIT_FUNCTION(mysqlnd)
{
#if defined(COMPILE_DL_MYSQLND) && defined(ZTS)
	CREX_TSRMLS_CACHE_UPDATE();
#endif
	mysqlnd_globals->collect_statistics = TRUE;
	mysqlnd_globals->collect_memory_statistics = FALSE;
	mysqlnd_globals->debug = NULL;	/* The actual string */
	mysqlnd_globals->dbg = NULL;	/* The DBG object*/
	mysqlnd_globals->trace_alloc_settings = NULL;
	mysqlnd_globals->trace_alloc = NULL;
	mysqlnd_globals->net_cmd_buffer_size = MYSQLND_NET_CMD_BUFFER_MIN_SIZE;
	mysqlnd_globals->net_read_buffer_size = 32768;
	mysqlnd_globals->net_read_timeout = 31536000;
	mysqlnd_globals->log_mask = 0;
	mysqlnd_globals->mempool_default_size = 16000;
	mysqlnd_globals->sha256_server_public_key = NULL;
}
/* }}} */


/* {{{ CRX_INI_MH */
static CRX_INI_MH(OnUpdateNetCmdBufferSize)
{
	crex_long long_value = CREX_ATOL(ZSTR_VAL(new_value));
	if (long_value < MYSQLND_NET_CMD_BUFFER_MIN_SIZE) {
		return FAILURE;
	}
	MYSQLND_G(net_cmd_buffer_size) = long_value;

	return SUCCESS;
}
/* }}} */


/* {{{ CRX_INI_BEGIN */
CRX_INI_BEGIN()
	STD_CRX_INI_BOOLEAN("mysqlnd.collect_statistics",	"1", 	CRX_INI_ALL,	OnUpdateBool,	collect_statistics, crex_mysqlnd_globals, mysqlnd_globals)
	STD_CRX_INI_BOOLEAN("mysqlnd.collect_memory_statistics","0",CRX_INI_SYSTEM, OnUpdateBool,	collect_memory_statistics, crex_mysqlnd_globals, mysqlnd_globals)
	STD_CRX_INI_ENTRY("mysqlnd.debug",					NULL, 	CRX_INI_SYSTEM, OnUpdateString,	debug, crex_mysqlnd_globals, mysqlnd_globals)
	STD_CRX_INI_ENTRY("mysqlnd.trace_alloc",			NULL, 	CRX_INI_SYSTEM, OnUpdateString,	trace_alloc_settings, crex_mysqlnd_globals, mysqlnd_globals)
	STD_CRX_INI_ENTRY("mysqlnd.net_cmd_buffer_size",	MYSQLND_NET_CMD_BUFFER_MIN_SIZE_STR,	CRX_INI_ALL,	OnUpdateNetCmdBufferSize,	net_cmd_buffer_size,	crex_mysqlnd_globals,		mysqlnd_globals)
	STD_CRX_INI_ENTRY("mysqlnd.net_read_buffer_size",	"32768",CRX_INI_ALL,	OnUpdateLong,	net_read_buffer_size,	crex_mysqlnd_globals,		mysqlnd_globals)
	STD_CRX_INI_ENTRY("mysqlnd.net_read_timeout",		"86400",CRX_INI_ALL,	OnUpdateLong,	net_read_timeout, crex_mysqlnd_globals, mysqlnd_globals)
	STD_CRX_INI_ENTRY("mysqlnd.log_mask",				"0", 	CRX_INI_ALL,	OnUpdateLong,	log_mask, crex_mysqlnd_globals, mysqlnd_globals)
	STD_CRX_INI_ENTRY("mysqlnd.mempool_default_size","16000",   CRX_INI_ALL,	OnUpdateLong,	mempool_default_size,	crex_mysqlnd_globals,		mysqlnd_globals)
	STD_CRX_INI_ENTRY("mysqlnd.sha256_server_public_key",NULL, 	CRX_INI_PERDIR, OnUpdateString,	sha256_server_public_key, crex_mysqlnd_globals, mysqlnd_globals)
CRX_INI_END()
/* }}} */


/* {{{ CRX_MINIT_FUNCTION */
static CRX_MINIT_FUNCTION(mysqlnd)
{
	REGISTER_INI_ENTRIES();

	mysqlnd_library_init();
	return SUCCESS;
}
/* }}} */


/* {{{ CRX_MSHUTDOWN_FUNCTION */
static CRX_MSHUTDOWN_FUNCTION(mysqlnd)
{
	mysqlnd_library_end();

	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */


#if CRX_DEBUG
/* {{{ CRX_RINIT_FUNCTION */
static CRX_RINIT_FUNCTION(mysqlnd)
{
	if (MYSQLND_G(debug)) {
		struct st_mysqlnd_plugin_trace_log * trace_log_plugin = mysqlnd_plugin_find("debug_trace");
		MYSQLND_G(dbg) = NULL;
		if (trace_log_plugin) {
			MYSQLND_DEBUG * dbg = trace_log_plugin->methods.trace_instance_init(mysqlnd_debug_std_no_trace_funcs);
			MYSQLND_DEBUG * trace_alloc = trace_log_plugin->methods.trace_instance_init(NULL);
			if (!dbg || !trace_alloc) {
				return FAILURE;
			}
			dbg->m->set_mode(dbg, MYSQLND_G(debug));
			trace_alloc->m->set_mode(trace_alloc, MYSQLND_G(trace_alloc_settings));
			MYSQLND_G(dbg) = dbg;
			MYSQLND_G(trace_alloc) = trace_alloc;
		}
	}
	return SUCCESS;
}
/* }}} */
#endif


#if CRX_DEBUG
/* {{{ CRX_RSHUTDOWN_FUNCTION */
static CRX_RSHUTDOWN_FUNCTION(mysqlnd)
{
	MYSQLND_DEBUG * dbg = MYSQLND_G(dbg);
	MYSQLND_DEBUG * trace_alloc = MYSQLND_G(trace_alloc);
	DBG_ENTER("RSHUTDOWN");
	if (dbg) {
		dbg->m->close(dbg);
		dbg->m->free_handle(dbg);
		MYSQLND_G(dbg) = NULL;
	}
	if (trace_alloc) {
		trace_alloc->m->close(trace_alloc);
		trace_alloc->m->free_handle(trace_alloc);
		MYSQLND_G(trace_alloc) = NULL;
	}
	return SUCCESS;
}
/* }}} */
#endif


static const crex_module_dep mysqlnd_deps[] = {
	CREX_MOD_REQUIRED("standard")
	CREX_MOD_END
};

/* {{{ mysqlnd_module_entry */
crex_module_entry mysqlnd_module_entry = {
	STANDARD_MODULE_HEADER_EX,
	NULL,
	mysqlnd_deps,
	"mysqlnd",
	NULL,
	CRX_MINIT(mysqlnd),
	CRX_MSHUTDOWN(mysqlnd),
#if CRX_DEBUG
	CRX_RINIT(mysqlnd),
#else
	NULL,
#endif
#if CRX_DEBUG
	CRX_RSHUTDOWN(mysqlnd),
#else
	NULL,
#endif
	CRX_MINFO(mysqlnd),
	CRX_MYSQLND_VERSION,
	CRX_MODULE_GLOBALS(mysqlnd),
	CRX_GINIT(mysqlnd),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

/* {{{ COMPILE_DL_MYSQLND */
#ifdef COMPILE_DL_MYSQLND
#ifdef ZTS
CREX_TSRMLS_CACHE_DEFINE()
#endif
CREX_GET_MODULE(mysqlnd)
#endif
/* }}} */
