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
  | Author: George Schlossnagle <george@omniti.com>                      |
  |         Johannes Schlueter <johannes@mysql.com>                      |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "crx_ini.h"
#include "ext/standard/info.h"
#include "pdo/crx_pdo.h"
#include "pdo/crx_pdo_driver.h"
#include "crx_pdo_mysql.h"
#include "crx_pdo_mysql_int.h"

#ifdef COMPILE_DL_PDO_MYSQL
#ifdef ZTS
CREX_TSRMLS_CACHE_DEFINE()
#endif
CREX_GET_MODULE(pdo_mysql)
#endif

CREX_DECLARE_MODULE_GLOBALS(pdo_mysql)

/*
 The default socket location is sometimes defined by configure.
 With libmysql `mysql_config --socket` will fill PDO_MYSQL_UNIX_ADDR
 and the user can use --with-mysql-sock=SOCKET which will fill
 PDO_MYSQL_UNIX_ADDR. If both aren't set we're using mysqlnd and use
 /tmp/mysql.sock as default on *nix and NULL for Windows (default
 named pipe name is set in mysqlnd).
*/
#ifndef PDO_MYSQL_UNIX_ADDR
# ifdef CRX_MYSQL_UNIX_SOCK_ADDR
#  define PDO_MYSQL_UNIX_ADDR CRX_MYSQL_UNIX_SOCK_ADDR
# else
#  ifndef CRX_WIN32
#   define PDO_MYSQL_UNIX_ADDR "/tmp/mysql.sock"
#  else
#   define PDO_MYSQL_UNIX_ADDR NULL
#  endif
# endif
#endif

#ifdef PDO_USE_MYSQLND
#include "ext/mysqlnd/mysqlnd_reverse_api.h"
static MYSQLND * pdo_mysql_convert_zv_to_mysqlnd(zval * zv)
{
	if (C_TYPE_P(zv) == IS_OBJECT && instanceof_function(C_OBJCE_P(zv), crx_pdo_get_dbh_ce())) {
		pdo_dbh_t * dbh = C_PDO_DBH_P(zv);

		CREX_ASSERT(dbh);

		if (dbh->driver != &pdo_mysql_driver) {
			crx_error_docref(NULL, E_WARNING, "Provided PDO instance is not using MySQL but %s", dbh->driver->driver_name);
			return NULL;
		}

		return ((pdo_mysql_db_handle *)dbh->driver_data)->server;
	}
	return NULL;
}

static const MYSQLND_REVERSE_API pdo_mysql_reverse_api = {
	&pdo_mysql_module_entry,
	pdo_mysql_convert_zv_to_mysqlnd
};
#endif


/* {{{ CRX_INI_BEGIN */
CRX_INI_BEGIN()
#ifndef CRX_WIN32
	STD_CRX_INI_ENTRY("pdo_mysql.default_socket", PDO_MYSQL_UNIX_ADDR, CRX_INI_SYSTEM, OnUpdateStringUnempty, default_socket, crex_pdo_mysql_globals, pdo_mysql_globals)
#endif
#if PDO_DBG_ENABLED
	STD_CRX_INI_ENTRY("pdo_mysql.debug",	NULL, CRX_INI_SYSTEM, OnUpdateString, debug, crex_pdo_mysql_globals, pdo_mysql_globals)
#endif
CRX_INI_END()
/* }}} */

/* true global environment */

/* {{{ CRX_MINIT_FUNCTION */
static CRX_MINIT_FUNCTION(pdo_mysql)
{
	REGISTER_INI_ENTRIES();

	REGISTER_PDO_CLASS_CONST_LONG("MYSQL_ATTR_USE_BUFFERED_QUERY", (crex_long)PDO_MYSQL_ATTR_USE_BUFFERED_QUERY);
	REGISTER_PDO_CLASS_CONST_LONG("MYSQL_ATTR_LOCAL_INFILE", (crex_long)PDO_MYSQL_ATTR_LOCAL_INFILE);
	REGISTER_PDO_CLASS_CONST_LONG("MYSQL_ATTR_INIT_COMMAND", (crex_long)PDO_MYSQL_ATTR_INIT_COMMAND);
#ifndef PDO_USE_MYSQLND
	REGISTER_PDO_CLASS_CONST_LONG("MYSQL_ATTR_MAX_BUFFER_SIZE", (crex_long)PDO_MYSQL_ATTR_MAX_BUFFER_SIZE);
	REGISTER_PDO_CLASS_CONST_LONG("MYSQL_ATTR_READ_DEFAULT_FILE", (crex_long)PDO_MYSQL_ATTR_READ_DEFAULT_FILE);
	REGISTER_PDO_CLASS_CONST_LONG("MYSQL_ATTR_READ_DEFAULT_GROUP", (crex_long)PDO_MYSQL_ATTR_READ_DEFAULT_GROUP);
#endif
	REGISTER_PDO_CLASS_CONST_LONG("MYSQL_ATTR_COMPRESS", (crex_long)PDO_MYSQL_ATTR_COMPRESS);
	REGISTER_PDO_CLASS_CONST_LONG("MYSQL_ATTR_DIRECT_QUERY", (crex_long)PDO_MYSQL_ATTR_DIRECT_QUERY);
	REGISTER_PDO_CLASS_CONST_LONG("MYSQL_ATTR_FOUND_ROWS", (crex_long)PDO_MYSQL_ATTR_FOUND_ROWS);
	REGISTER_PDO_CLASS_CONST_LONG("MYSQL_ATTR_IGNORE_SPACE", (crex_long)PDO_MYSQL_ATTR_IGNORE_SPACE);
	REGISTER_PDO_CLASS_CONST_LONG("MYSQL_ATTR_SSL_KEY", (crex_long)PDO_MYSQL_ATTR_SSL_KEY);
	REGISTER_PDO_CLASS_CONST_LONG("MYSQL_ATTR_SSL_CERT", (crex_long)PDO_MYSQL_ATTR_SSL_CERT);
	REGISTER_PDO_CLASS_CONST_LONG("MYSQL_ATTR_SSL_CA", (crex_long)PDO_MYSQL_ATTR_SSL_CA);
	REGISTER_PDO_CLASS_CONST_LONG("MYSQL_ATTR_SSL_CAPATH", (crex_long)PDO_MYSQL_ATTR_SSL_CAPATH);
	REGISTER_PDO_CLASS_CONST_LONG("MYSQL_ATTR_SSL_CIPHER", (crex_long)PDO_MYSQL_ATTR_SSL_CIPHER);
#if MYSQL_VERSION_ID > 50605 || defined(PDO_USE_MYSQLND)
	 REGISTER_PDO_CLASS_CONST_LONG("MYSQL_ATTR_SERVER_PUBLIC_KEY", (crex_long)PDO_MYSQL_ATTR_SERVER_PUBLIC_KEY);
#endif
	REGISTER_PDO_CLASS_CONST_LONG("MYSQL_ATTR_MULTI_STATEMENTS", (crex_long)PDO_MYSQL_ATTR_MULTI_STATEMENTS);
#ifdef PDO_USE_MYSQLND
	REGISTER_PDO_CLASS_CONST_LONG("MYSQL_ATTR_SSL_VERIFY_SERVER_CERT", (crex_long)PDO_MYSQL_ATTR_SSL_VERIFY_SERVER_CERT);
#endif
#if MYSQL_VERSION_ID >= 80021 || defined(PDO_USE_MYSQLND)
	REGISTER_PDO_CLASS_CONST_LONG("MYSQL_ATTR_LOCAL_INFILE_DIRECTORY", (crex_long)PDO_MYSQL_ATTR_LOCAL_INFILE_DIRECTORY);
#endif

#ifdef PDO_USE_MYSQLND
	mysqlnd_reverse_api_register_api(&pdo_mysql_reverse_api);
#endif

	return crx_pdo_register_driver(&pdo_mysql_driver);
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION */
static CRX_MSHUTDOWN_FUNCTION(pdo_mysql)
{
	crx_pdo_unregister_driver(&pdo_mysql_driver);
#ifdef PDO_USE_MYSQLND
	UNREGISTER_INI_ENTRIES();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
static CRX_MINFO_FUNCTION(pdo_mysql)
{
	crx_info_print_table_start();

	crx_info_print_table_row(2, "PDO Driver for MySQL", "enabled");
	crx_info_print_table_row(2, "Client API version", mysql_get_client_info());

	crx_info_print_table_end();

#ifndef CRX_WIN32
	DISPLAY_INI_ENTRIES();
#endif
}
/* }}} */


#if defined(PDO_USE_MYSQLND) && PDO_DBG_ENABLED
/* {{{ CRX_RINIT_FUNCTION */
static CRX_RINIT_FUNCTION(pdo_mysql)
{
	if (PDO_MYSQL_G(debug)) {
		MYSQLND_DEBUG *dbg = mysqlnd_debug_init(mysqlnd_debug_std_no_trace_funcs);
		if (!dbg) {
			return FAILURE;
		}
		dbg->m->set_mode(dbg, PDO_MYSQL_G(debug));
		PDO_MYSQL_G(dbg) = dbg;
	}

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_RSHUTDOWN_FUNCTION */
static CRX_RSHUTDOWN_FUNCTION(pdo_mysql)
{
	MYSQLND_DEBUG *dbg = PDO_MYSQL_G(dbg);
	PDO_DBG_ENTER("RSHUTDOWN");
	if (dbg) {
		dbg->m->close(dbg);
		dbg->m->free_handle(dbg);
		PDO_MYSQL_G(dbg) = NULL;
	}

	return SUCCESS;
}
/* }}} */
#endif

/* {{{ CRX_GINIT_FUNCTION */
static CRX_GINIT_FUNCTION(pdo_mysql)
{
#if defined(COMPILE_DL_PDO_MYSQL) && defined(ZTS)
CREX_TSRMLS_CACHE_UPDATE();
#endif
#ifndef CRX_WIN32
	pdo_mysql_globals->default_socket = NULL;
#endif
#if PDO_DBG_ENABLED
	pdo_mysql_globals->debug = NULL;	/* The actual string */
	pdo_mysql_globals->dbg = NULL;	/* The DBG object*/
#endif
}
/* }}} */

/* {{{ pdo_mysql_deps[] */
static const crex_module_dep pdo_mysql_deps[] = {
	CREX_MOD_REQUIRED("pdo")
#ifdef PDO_USE_MYSQLND
	CREX_MOD_REQUIRED("mysqlnd")
#endif
	CREX_MOD_END
};
/* }}} */

/* {{{ pdo_mysql_module_entry */
crex_module_entry pdo_mysql_module_entry = {
	STANDARD_MODULE_HEADER_EX, NULL,
	pdo_mysql_deps,
	"pdo_mysql",
	NULL,
	CRX_MINIT(pdo_mysql),
	CRX_MSHUTDOWN(pdo_mysql),
#if defined(PDO_USE_MYSQLND) && PDO_DBG_ENABLED
	CRX_RINIT(pdo_mysql),
	CRX_RSHUTDOWN(pdo_mysql),
#else
	NULL,
	NULL,
#endif
	CRX_MINFO(pdo_mysql),
	CRX_PDO_MYSQL_VERSION,
	CRX_MODULE_GLOBALS(pdo_mysql),
	CRX_GINIT(pdo_mysql),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */
