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
  | Author: Edin Kadribasic <edink@emini.dk>                             |
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
#include "crx_pdo_pgsql.h"
#include "crx_pdo_pgsql_int.h"

/* {{{ pdo_sqlite_deps */
static const crex_module_dep pdo_pgsql_deps[] = {
	CREX_MOD_REQUIRED("pdo")
	CREX_MOD_END
};
/* }}} */

/* {{{ pdo_pgsql_module_entry */
crex_module_entry pdo_pgsql_module_entry = {
	STANDARD_MODULE_HEADER_EX, NULL,
	pdo_pgsql_deps,
	"pdo_pgsql",
	NULL,
	CRX_MINIT(pdo_pgsql),
	CRX_MSHUTDOWN(pdo_pgsql),
	NULL,
	NULL,
	CRX_MINFO(pdo_pgsql),
	CRX_PDO_PGSQL_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PDO_PGSQL
CREX_GET_MODULE(pdo_pgsql)
#endif

/* true global environment */

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(pdo_pgsql)
{
	REGISTER_PDO_CLASS_CONST_LONG("PGSQL_ATTR_DISABLE_PREPARES", PDO_PGSQL_ATTR_DISABLE_PREPARES);
	REGISTER_PDO_CLASS_CONST_LONG("PGSQL_TRANSACTION_IDLE", (crex_long)PGSQL_TRANSACTION_IDLE);
	REGISTER_PDO_CLASS_CONST_LONG("PGSQL_TRANSACTION_ACTIVE", (crex_long)PGSQL_TRANSACTION_ACTIVE);
	REGISTER_PDO_CLASS_CONST_LONG("PGSQL_TRANSACTION_INTRANS", (crex_long)PGSQL_TRANSACTION_INTRANS);
	REGISTER_PDO_CLASS_CONST_LONG("PGSQL_TRANSACTION_INERROR", (crex_long)PGSQL_TRANSACTION_INERROR);
	REGISTER_PDO_CLASS_CONST_LONG("PGSQL_TRANSACTION_UNKNOWN", (crex_long)PGSQL_TRANSACTION_UNKNOWN);

	return crx_pdo_register_driver(&pdo_pgsql_driver);
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION */
CRX_MSHUTDOWN_FUNCTION(pdo_pgsql)
{
	crx_pdo_unregister_driver(&pdo_pgsql_driver);
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(pdo_pgsql)
{
	char buf[16];

	crx_info_print_table_start();
	crx_info_print_table_row(2, "PDO Driver for PostgreSQL", "enabled");
	pdo_libpq_version(buf, sizeof(buf));
	crx_info_print_table_row(2, "PostgreSQL(libpq) Version", buf);
	crx_info_print_table_end();
}
/* }}} */
