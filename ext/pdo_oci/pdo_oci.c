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
#include "crx_pdo_oci.h"
#include "crx_pdo_oci_int.h"
#ifdef ZTS
#include <TSRM/TSRM.h>
#endif

/* {{{ pdo_oci_module_entry */

static const crex_module_dep pdo_oci_deps[] = {
	CREX_MOD_REQUIRED("pdo")
	CREX_MOD_END
};

crex_module_entry pdo_oci_module_entry = {
	STANDARD_MODULE_HEADER_EX, NULL,
	pdo_oci_deps,
	"PDO_OCI",
	NULL,
	CRX_MINIT(pdo_oci),
	CRX_MSHUTDOWN(pdo_oci),
	CRX_RINIT(pdo_oci),
	NULL,
	CRX_MINFO(pdo_oci),
	CRX_PDO_OCI_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PDO_OCI
CREX_GET_MODULE(pdo_oci)
#endif

const ub4 PDO_OCI_INIT_MODE =
#if 0 && defined(OCI_SHARED)
			/* shared mode is known to be bad for CRX */
			OCI_SHARED
#else
			OCI_DEFAULT
#endif
#ifdef OCI_OBJECT
			|OCI_OBJECT
#endif
#ifdef ZTS
			|OCI_THREADED
#endif
			;

/* true global environment */
OCIEnv *pdo_oci_Env = NULL;

#ifdef ZTS
/* lock for pdo_oci_Env initialization */
static MUTEX_T pdo_oci_env_mutex;
#endif

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(pdo_oci)
{
	REGISTER_PDO_CLASS_CONST_LONG("OCI_ATTR_ACTION", (crex_long)PDO_OCI_ATTR_ACTION);
	REGISTER_PDO_CLASS_CONST_LONG("OCI_ATTR_CLIENT_INFO", (crex_long)PDO_OCI_ATTR_CLIENT_INFO);
	REGISTER_PDO_CLASS_CONST_LONG("OCI_ATTR_CLIENT_IDENTIFIER", (crex_long)PDO_OCI_ATTR_CLIENT_IDENTIFIER);
	REGISTER_PDO_CLASS_CONST_LONG("OCI_ATTR_MODULE", (crex_long)PDO_OCI_ATTR_MODULE);
	REGISTER_PDO_CLASS_CONST_LONG("OCI_ATTR_CALL_TIMEOUT", (crex_long)PDO_OCI_ATTR_CALL_TIMEOUT);

	if (FAILURE == crx_pdo_register_driver(&pdo_oci_driver)) {
		return FAILURE;
	}

	// Defer OCI init to CRX_RINIT_FUNCTION because with crx-fpm,
	// NLS_LANG is not yet available here.

#ifdef ZTS
	pdo_oci_env_mutex = tsrm_mutex_alloc();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_RINIT_FUNCTION */
CRX_RINIT_FUNCTION(pdo_oci)
{
	if (!pdo_oci_Env) {
#ifdef ZTS
		tsrm_mutex_lock(pdo_oci_env_mutex);
		if (!pdo_oci_Env) { // double-checked locking idiom
#endif
#ifdef HAVE_OCIENVCREATE
		OCIEnvCreate(&pdo_oci_Env, PDO_OCI_INIT_MODE, NULL, NULL, NULL, NULL, 0, NULL);
#else
		OCIInitialize(PDO_OCI_INIT_MODE, NULL, NULL, NULL, NULL);
		OCIEnvInit(&pdo_oci_Env, OCI_DEFAULT, 0, NULL);
#endif
#ifdef ZTS
		}
		tsrm_mutex_unlock(pdo_oci_env_mutex);
#endif
	}

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION */
CRX_MSHUTDOWN_FUNCTION(pdo_oci)
{
	crx_pdo_unregister_driver(&pdo_oci_driver);

	if (pdo_oci_Env) {
		OCIHandleFree((dvoid*)pdo_oci_Env, OCI_HTYPE_ENV);
	}

#ifdef ZTS
	tsrm_mutex_free(pdo_oci_env_mutex);
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(pdo_oci)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "PDO Driver for OCI 8 and later", "enabled");
	crx_info_print_table_end();
}
/* }}} */
