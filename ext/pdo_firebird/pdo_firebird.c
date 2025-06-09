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
  | Author: Ard Biesheuvel <abies@crx.net>                               |
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
#include "crx_pdo_firebird.h"
#include "crx_pdo_firebird_int.h"

/* {{{ pdo_firebird_deps */
static const crex_module_dep pdo_firebird_deps[] = {
	CREX_MOD_REQUIRED("pdo")
	CREX_MOD_END
};
/* }}} */

crex_module_entry pdo_firebird_module_entry = { /* {{{ */
	STANDARD_MODULE_HEADER_EX, NULL,
	pdo_firebird_deps,
	"PDO_Firebird",
	NULL,
	CRX_MINIT(pdo_firebird),
	CRX_MSHUTDOWN(pdo_firebird),
	NULL,
	NULL,
	CRX_MINFO(pdo_firebird),
	CRX_PDO_FIREBIRD_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PDO_FIREBIRD
CREX_GET_MODULE(pdo_firebird)
#endif

CRX_MINIT_FUNCTION(pdo_firebird) /* {{{ */
{
	REGISTER_PDO_CLASS_CONST_LONG("FB_ATTR_DATE_FORMAT", (crex_long) PDO_FB_ATTR_DATE_FORMAT);
	REGISTER_PDO_CLASS_CONST_LONG("FB_ATTR_TIME_FORMAT", (crex_long) PDO_FB_ATTR_TIME_FORMAT);
	REGISTER_PDO_CLASS_CONST_LONG("FB_ATTR_TIMESTAMP_FORMAT", (crex_long) PDO_FB_ATTR_TIMESTAMP_FORMAT);

	if (FAILURE == crx_pdo_register_driver(&pdo_firebird_driver)) {
		return FAILURE;
	}

#ifdef CREX_SIGNALS
	/* firebird replaces some signals at runtime, suppress warnings. */
	SIGG(check) = 0;
#endif

	return SUCCESS;
}
/* }}} */

CRX_MSHUTDOWN_FUNCTION(pdo_firebird) /* {{{ */
{
	crx_pdo_unregister_driver(&pdo_firebird_driver);

	return SUCCESS;
}
/* }}} */

CRX_MINFO_FUNCTION(pdo_firebird) /* {{{ */
{
	char version[64];
	isc_get_client_version(version);

	crx_info_print_table_start();
	crx_info_print_table_row(2, "PDO Driver for Firebird", "enabled");
	crx_info_print_table_row(2, "Client Library Version", version);
	crx_info_print_table_end();
}
/* }}} */
