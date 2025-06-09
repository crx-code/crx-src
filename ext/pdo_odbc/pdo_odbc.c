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
#include "crx_pdo_odbc.h"
#include "crx_pdo_odbc_int.h"
#include "pdo_odbc_arginfo.h"

/* {{{ pdo_odbc_deps[] */
static const crex_module_dep pdo_odbc_deps[] = {
	CREX_MOD_REQUIRED("pdo")
	CREX_MOD_END
};
/* }}} */

/* {{{ pdo_odbc_module_entry */
crex_module_entry pdo_odbc_module_entry = {
	STANDARD_MODULE_HEADER_EX, NULL,
	pdo_odbc_deps,
	"PDO_ODBC",
	NULL,
	CRX_MINIT(pdo_odbc),
	CRX_MSHUTDOWN(pdo_odbc),
	NULL,
	NULL,
	CRX_MINFO(pdo_odbc),
	CRX_PDO_ODBC_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PDO_ODBC
CREX_GET_MODULE(pdo_odbc)
#endif

#ifdef SQL_ATTR_CONNECTION_POOLING
crex_ulong pdo_odbc_pool_on = SQL_CP_OFF;
crex_ulong pdo_odbc_pool_mode = SQL_CP_ONE_PER_HENV;
#endif

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(pdo_odbc)
{
#ifdef SQL_ATTR_CONNECTION_POOLING
	char *pooling_val = NULL;
#endif

	if (FAILURE == crx_pdo_register_driver(&pdo_odbc_driver)) {
		return FAILURE;
	}

#ifdef SQL_ATTR_CONNECTION_POOLING
	/* ugh, we don't really like .ini stuff in PDO, but since ODBC connection
	 * pooling is process wide, we can't set it from within the scope of a
	 * request without affecting others, which goes against our isolated request
	 * policy.  So, we use cfg_get_string here to check it this once.
	 * */
	if (FAILURE == cfg_get_string("pdo_odbc.connection_pooling", &pooling_val) || pooling_val == NULL) {
		pooling_val = "strict";
	}
	if (strcasecmp(pooling_val, "strict") == 0 || strcmp(pooling_val, "1") == 0) {
		pdo_odbc_pool_on = SQL_CP_ONE_PER_HENV;
		pdo_odbc_pool_mode = SQL_CP_STRICT_MATCH;
	} else if (strcasecmp(pooling_val, "relaxed") == 0) {
		pdo_odbc_pool_on = SQL_CP_ONE_PER_HENV;
		pdo_odbc_pool_mode = SQL_CP_RELAXED_MATCH;
	} else if (*pooling_val == '\0' || strcasecmp(pooling_val, "off") == 0) {
		pdo_odbc_pool_on = SQL_CP_OFF;
	} else {
		crx_error_docref(NULL, E_CORE_ERROR, "Error in pdo_odbc.connection_pooling configuration. Value must be one of \"strict\", \"relaxed\", or \"off\"");
		return FAILURE;
	}

	if (pdo_odbc_pool_on != SQL_CP_OFF) {
		SQLSetEnvAttr(SQL_NULL_HANDLE, SQL_ATTR_CONNECTION_POOLING, (void*)pdo_odbc_pool_on, 0);
	}
#endif

	register_pdo_odbc_symbols(module_number);

	REGISTER_PDO_CLASS_CONST_LONG("ODBC_ATTR_USE_CURSOR_LIBRARY", PDO_ODBC_ATTR_USE_CURSOR_LIBRARY);
	REGISTER_PDO_CLASS_CONST_LONG("ODBC_ATTR_ASSUME_UTF8", PDO_ODBC_ATTR_ASSUME_UTF8);
	REGISTER_PDO_CLASS_CONST_LONG("ODBC_SQL_USE_IF_NEEDED", SQL_CUR_USE_IF_NEEDED);
	REGISTER_PDO_CLASS_CONST_LONG("ODBC_SQL_USE_DRIVER", SQL_CUR_USE_DRIVER);
	REGISTER_PDO_CLASS_CONST_LONG("ODBC_SQL_USE_ODBC", SQL_CUR_USE_ODBC);

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION */
CRX_MSHUTDOWN_FUNCTION(pdo_odbc)
{
	crx_pdo_unregister_driver(&pdo_odbc_driver);
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(pdo_odbc)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "PDO Driver for ODBC (" PDO_ODBC_TYPE ")" , "enabled");
#ifdef SQL_ATTR_CONNECTION_POOLING
	crx_info_print_table_row(2, "ODBC Connection Pooling",	pdo_odbc_pool_on == SQL_CP_OFF ?
			"Disabled" : (pdo_odbc_pool_mode == SQL_CP_STRICT_MATCH ? "Enabled, strict matching" : "Enabled, relaxed matching"));
#else
	crx_info_print_table_row(2, "ODBC Connection Pooling", "Not supported in this build");
#endif
	crx_info_print_table_end();
}
/* }}} */
