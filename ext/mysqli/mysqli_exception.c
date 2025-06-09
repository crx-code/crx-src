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
#include "mysqli_priv.h"
#include "crex_exceptions.h"

void crx_mysqli_throw_sql_exception(char *sqlstate, int errorno, char *format, ...)
{
	zval	sql_ex;
	va_list arg;
	char 	*message;

	va_start(arg, format);
	vspprintf(&message, 0, format, arg);
	va_end(arg);

	if (!(MyG(report_mode) & MYSQLI_REPORT_STRICT)) {
	 	crx_error_docref(NULL, E_WARNING, "(%s/%d): %s", sqlstate, errorno, message);
		efree(message);
		return;
	}

	object_init_ex(&sql_ex, mysqli_exception_class_entry);

	if (message) {
		crex_update_property_string(
			mysqli_exception_class_entry, C_OBJ(sql_ex), "message", sizeof("message") - 1, message);
	}

	if (sqlstate) {
		crex_update_property_string(
			mysqli_exception_class_entry, C_OBJ(sql_ex), "sqlstate", sizeof("sqlstate") - 1, sqlstate);
	} else {
		crex_update_property_string(
			mysqli_exception_class_entry, C_OBJ(sql_ex), "sqlstate", sizeof("sqlstate") - 1, "00000");
	}

	efree(message);
	crex_update_property_long(mysqli_exception_class_entry, C_OBJ(sql_ex), "code", sizeof("code") - 1, errorno);

	crex_throw_exception_object(&sql_ex);
}

CRX_METHOD(mysqli_sql_exception, getSqlState)
{
	zval *prop;
	zval rv;

	CREX_PARSE_PARAMETERS_NONE();

	prop = crex_read_property(mysqli_exception_class_entry, C_OBJ_P(CREX_THIS), "sqlstate", sizeof("sqlstate")-1, 1, &rv);
	ZVAL_DEREF(prop);
	crex_string *str = zval_get_string(prop);
	
	RETURN_STR(str);
}
