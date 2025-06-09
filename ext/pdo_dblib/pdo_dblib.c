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
  |         Frank M. Kromann <frank@kromann.info>                        |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "crx.h"
#include "crx_ini.h"
#include "ext/standard/info.h"
#include "pdo/crx_pdo.h"
#include "pdo/crx_pdo_driver.h"
#include "crx_pdo_dblib.h"
#include "crx_pdo_dblib_int.h"
#include "crex_exceptions.h"

CREX_DECLARE_MODULE_GLOBALS(dblib)
static CRX_GINIT_FUNCTION(dblib);

static const crex_module_dep pdo_dblib_deps[] = {
	CREX_MOD_REQUIRED("pdo")
	CREX_MOD_END
};

#ifdef PDO_DBLIB_IS_MSSQL
crex_module_entry pdo_mssql_module_entry = {
#else
crex_module_entry pdo_dblib_module_entry = {
#endif
	STANDARD_MODULE_HEADER_EX, NULL,
	pdo_dblib_deps,
#ifdef PDO_DBLIB_IS_MSSQL
	"pdo_mssql",
#elif defined(CRX_WIN32)
	"pdo_sybase",
#else
	"pdo_dblib",
#endif
	NULL,
	CRX_MINIT(pdo_dblib),
	CRX_MSHUTDOWN(pdo_dblib),
	NULL,
	CRX_RSHUTDOWN(pdo_dblib),
	CRX_MINFO(pdo_dblib),
	CRX_PDO_DBLIB_VERSION,
	CRX_MODULE_GLOBALS(dblib),
	CRX_GINIT(dblib),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

#if defined(COMPILE_DL_PDO_DBLIB) || defined(COMPILE_DL_PDO_MSSQL)
#ifdef ZTS
CREX_TSRMLS_CACHE_DEFINE()
#endif
#ifdef PDO_DBLIB_IS_MSSQL
CREX_GET_MODULE(pdo_mssql)
#else
CREX_GET_MODULE(pdo_dblib)
#endif
#endif

int pdo_dblib_error_handler(DBPROCESS *dbproc, int severity, int dberr,
	int oserr, char *dberrstr, char *oserrstr)
{
	pdo_dblib_err *einfo;
	char *state = "HY000";

	if(dbproc) {
		einfo = (pdo_dblib_err*)dbgetuserdata(dbproc);
		if (!einfo) einfo = &DBLIB_G(err);
	} else {
		einfo = &DBLIB_G(err);
	}

	einfo->severity = severity;
	einfo->oserr = oserr;
	einfo->dberr = dberr;

	if (einfo->oserrstr) {
		efree(einfo->oserrstr);
	}
	if (einfo->dberrstr) {
		efree(einfo->dberrstr);
	}
	if (oserrstr) {
		einfo->oserrstr = estrdup(oserrstr);
	} else {
		einfo->oserrstr = NULL;
	}
	if (dberrstr) {
		einfo->dberrstr = estrdup(dberrstr);
	} else {
		einfo->dberrstr = NULL;
	}

	switch (dberr) {
		case SYBESEOF:
		case SYBEFCON:	state = "01002"; break;
		case SYBEMEM:	state = "HY001"; break;
		case SYBEPWD:	state = "28000"; break;
	}
	strcpy(einfo->sqlstate, state);

	return INT_CANCEL;
}

int pdo_dblib_msg_handler(DBPROCESS *dbproc, DBINT msgno, int msgstate,
	int severity, char *msgtext, char *srvname, char *procname, int line)
{
	pdo_dblib_err *einfo;

	if (severity) {
		einfo = (pdo_dblib_err*)dbgetuserdata(dbproc);
		if (!einfo) {
			einfo = &DBLIB_G(err);
		}

		if (einfo->lastmsg) {
			efree(einfo->lastmsg);
		}

		einfo->lastmsg = estrdup(msgtext);
	}

	return 0;
}

void pdo_dblib_err_dtor(pdo_dblib_err *err)
{
	if (!err) {
		return;
	}

	if (err->dberrstr) {
		efree(err->dberrstr);
		err->dberrstr = NULL;
	}
	if (err->lastmsg) {
		efree(err->lastmsg);
		err->lastmsg = NULL;
	}
	if (err->oserrstr) {
		efree(err->oserrstr);
		err->oserrstr = NULL;
	}
}

static CRX_GINIT_FUNCTION(dblib)
{
#if defined(ZTS) && (defined(COMPILE_DL_PDO_DBLIB) || defined(COMPILE_DL_PDO_MSSQL))
	CREX_TSRMLS_CACHE_UPDATE();
#endif
	memset(dblib_globals, 0, sizeof(*dblib_globals));
	dblib_globals->err.sqlstate = dblib_globals->sqlstate;
}

CRX_RSHUTDOWN_FUNCTION(pdo_dblib)
{
	if (DBLIB_G(err).oserrstr) {
		efree(DBLIB_G(err).oserrstr);
		DBLIB_G(err).oserrstr = NULL;
	}
	if (DBLIB_G(err).dberrstr) {
		efree(DBLIB_G(err).dberrstr);
		DBLIB_G(err).dberrstr = NULL;
	}
	if (DBLIB_G(err).lastmsg) {
		efree(DBLIB_G(err).lastmsg);
		DBLIB_G(err).lastmsg = NULL;
	}
	return SUCCESS;
}

CRX_MINIT_FUNCTION(pdo_dblib)
{
	REGISTER_PDO_CLASS_CONST_LONG("DBLIB_ATTR_CONNECTION_TIMEOUT", (long) PDO_DBLIB_ATTR_CONNECTION_TIMEOUT);
	REGISTER_PDO_CLASS_CONST_LONG("DBLIB_ATTR_QUERY_TIMEOUT", (long) PDO_DBLIB_ATTR_QUERY_TIMEOUT);
	REGISTER_PDO_CLASS_CONST_LONG("DBLIB_ATTR_STRINGIFY_UNIQUEIDENTIFIER", (long) PDO_DBLIB_ATTR_STRINGIFY_UNIQUEIDENTIFIER);
	REGISTER_PDO_CLASS_CONST_LONG("DBLIB_ATTR_VERSION", (long) PDO_DBLIB_ATTR_VERSION);
	REGISTER_PDO_CLASS_CONST_LONG("DBLIB_ATTR_TDS_VERSION", (long) PDO_DBLIB_ATTR_TDS_VERSION);
	REGISTER_PDO_CLASS_CONST_LONG("DBLIB_ATTR_SKIP_EMPTY_ROWSETS", (long) PDO_DBLIB_ATTR_SKIP_EMPTY_ROWSETS);
	REGISTER_PDO_CLASS_CONST_LONG("DBLIB_ATTR_DATETIME_CONVERT", (long) PDO_DBLIB_ATTR_DATETIME_CONVERT);

	if (FAIL == dbinit()) {
		return FAILURE;
	}

	if (FAILURE == crx_pdo_register_driver(&pdo_dblib_driver)) {
		return FAILURE;
	}

#ifndef CRX_DBLIB_IS_MSSQL
	dberrhandle((EHANDLEFUNC) pdo_dblib_error_handler);
	dbmsghandle((MHANDLEFUNC) pdo_dblib_msg_handler);
#endif

	return SUCCESS;
}

CRX_MSHUTDOWN_FUNCTION(pdo_dblib)
{
	crx_pdo_unregister_driver(&pdo_dblib_driver);
	dbexit();
	return SUCCESS;
}

CRX_MINFO_FUNCTION(pdo_dblib)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "PDO Driver for "
#ifdef PDO_DBLIB_IS_MSSQL
		"MSSQL"
#elif defined(CRX_WIN32)
		"FreeTDS/Sybase/MSSQL"
#else
		"FreeTDS/Sybase"
#endif
		" DB-lib", "enabled");
	crx_info_print_table_row(2, "Flavour", PDO_DBLIB_FLAVOUR);
	crx_info_print_table_end();
}
