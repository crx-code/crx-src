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
  | Author: Georg Richter <georg@crx.net>                                |
  +----------------------------------------------------------------------+

*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <signal.h>

#include "crx.h"
#include "crx_ini.h"
#include "ext/standard/info.h"
#include "crx_mysqli_structs.h"
#include "crex_exceptions.h"


/* {{{ property driver_report_read */
static int driver_report_read(mysqli_object *obj, zval *retval, bool quiet)
{
	ZVAL_LONG(retval, MyG(report_mode));
	return SUCCESS;
}
/* }}} */

/* {{{ property driver_report_write */
static int driver_report_write(mysqli_object *obj, zval *value)
{
	CREX_ASSERT(C_TYPE_P(value) == IS_LONG);
	MyG(report_mode) = C_LVAL_P(value);
	return SUCCESS;
}
/* }}} */

/* {{{ property driver_client_version_read */
static int driver_client_version_read(mysqli_object *obj, zval *retval, bool quiet)
{
	ZVAL_LONG(retval, mysql_get_client_version());
	return SUCCESS;
}
/* }}} */

/* {{{ property driver_client_info_read */
static int driver_client_info_read(mysqli_object *obj, zval *retval, bool quiet)
{
	ZVAL_STRING(retval, (char *)mysql_get_client_info());
	return SUCCESS;
}
/* }}} */

/* {{{ property driver_driver_version_read */
static int driver_driver_version_read(mysqli_object *obj, zval *retval, bool quiet)
{
	if (quiet) {
		return FAILURE;
	}
	crex_error(E_DEPRECATED, "The driver_version property is deprecated");
	ZVAL_LONG(retval, MYSQLI_VERSION_ID);
	return SUCCESS;
}
/* }}} */

const mysqli_property_entry mysqli_driver_property_entries[] = {
	{"client_info", sizeof("client_info") - 1, driver_client_info_read, NULL},
	{"client_version", sizeof("client_version") - 1, driver_client_version_read, NULL},
	{"driver_version", sizeof("driver_version") - 1, driver_driver_version_read, NULL},
	{"report_mode", sizeof("report_mode") - 1, driver_report_read, driver_report_write},
	{NULL, 0, NULL, NULL}
};

