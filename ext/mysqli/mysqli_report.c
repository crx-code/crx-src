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

#include "crx.h"
#include "crx_ini.h"
#include "ext/standard/info.h"
#include "crx_mysqli_structs.h"

extern void crx_mysqli_throw_sql_exception(char *sqlstate, int errorno, char *format, ...);

/* {{{ sets report level */
CRX_FUNCTION(mysqli_report)
{
	crex_long flags;


	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &flags) == FAILURE) {
		RETURN_THROWS();
	}

	MyG(report_mode) = flags;

	RETURN_TRUE;
}
/* }}} */

/* {{{ void crx_mysqli_report_error(char *sqlstate, int errorno, char *error) */
void crx_mysqli_report_error(const char *sqlstate, int errorno, const char *error)
{
	crx_mysqli_throw_sql_exception((char *)sqlstate, errorno, "%s", error);
}
/* }}} */

/* {{{ void crx_mysqli_report_index() */
void crx_mysqli_report_index(const char *query, unsigned int status) {
	char index[15];

	if (status & SERVER_QUERY_NO_GOOD_INDEX_USED) {
		strcpy(index, "Bad index");
	} else if (status & SERVER_QUERY_NO_INDEX_USED) {
		strcpy(index, "No index");
	} else {
		return;
	}
	crx_mysqli_throw_sql_exception("00000", 0, "%s used in query/prepared statement %s", index, query);
}
/* }}} */
