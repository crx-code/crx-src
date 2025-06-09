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
  | Author: Wez Furlong <wez@crx.net>                                    |
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
#include "crx_pdo_sqlite.h"
#include "crx_pdo_sqlite_int.h"
#include "crex_exceptions.h"

/* {{{ pdo_sqlite_deps */
static const crex_module_dep pdo_sqlite_deps[] = {
	CREX_MOD_REQUIRED("pdo")
	CREX_MOD_END
};
/* }}} */

/* {{{ pdo_sqlite_module_entry */
crex_module_entry pdo_sqlite_module_entry = {
	STANDARD_MODULE_HEADER_EX, NULL,
	pdo_sqlite_deps,
	"pdo_sqlite",
	NULL,
	CRX_MINIT(pdo_sqlite),
	CRX_MSHUTDOWN(pdo_sqlite),
	NULL,
	NULL,
	CRX_MINFO(pdo_sqlite),
	CRX_PDO_SQLITE_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#if defined(COMPILE_DL_PDO_SQLITE) || defined(COMPILE_DL_PDO_SQLITE_EXTERNAL)
CREX_GET_MODULE(pdo_sqlite)
#endif

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(pdo_sqlite)
{
#ifdef SQLITE_DETERMINISTIC
	REGISTER_PDO_CLASS_CONST_LONG("SQLITE_DETERMINISTIC", (crex_long)SQLITE_DETERMINISTIC);
#endif

	REGISTER_PDO_CLASS_CONST_LONG("SQLITE_ATTR_OPEN_FLAGS", (crex_long)PDO_SQLITE_ATTR_OPEN_FLAGS);
	REGISTER_PDO_CLASS_CONST_LONG("SQLITE_OPEN_READONLY", (crex_long)SQLITE_OPEN_READONLY);
	REGISTER_PDO_CLASS_CONST_LONG("SQLITE_OPEN_READWRITE", (crex_long)SQLITE_OPEN_READWRITE);
	REGISTER_PDO_CLASS_CONST_LONG("SQLITE_OPEN_CREATE", (crex_long)SQLITE_OPEN_CREATE);
	REGISTER_PDO_CLASS_CONST_LONG("SQLITE_ATTR_READONLY_STATEMENT", (crex_long)PDO_SQLITE_ATTR_READONLY_STATEMENT);
	REGISTER_PDO_CLASS_CONST_LONG("SQLITE_ATTR_EXTENDED_RESULT_CODES", (crex_long)PDO_SQLITE_ATTR_EXTENDED_RESULT_CODES);

	return crx_pdo_register_driver(&pdo_sqlite_driver);
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION */
CRX_MSHUTDOWN_FUNCTION(pdo_sqlite)
{
	crx_pdo_unregister_driver(&pdo_sqlite_driver);
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(pdo_sqlite)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "PDO Driver for SQLite 3.x", "enabled");
	crx_info_print_table_row(2, "SQLite Library", sqlite3_libversion());
	crx_info_print_table_end();
}
/* }}} */
