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
   | Author: Stig Sæther Bakken <ssb@crx.net>                             |
   +----------------------------------------------------------------------+
 */

#include "crx.h"

#ifdef HAVE_SYSLOG_H
#include "crx_ini.h"
#include "crex_globals.h"

#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <string.h>
#include <errno.h>

#include <stdio.h>
#include "basic_functions.h"
#include "crx_ext_syslog.h"

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(syslog)
{
	return SUCCESS;
}
/* }}} */

CRX_RSHUTDOWN_FUNCTION(syslog)
{
	crx_closelog();
	if (BG(syslog_device)) {
		free(BG(syslog_device));
		BG(syslog_device) = NULL;
	}
	return SUCCESS;
}


/* {{{ Open connection to system logger */
/*
   ** OpenLog("nettopp", $LOG_PID, $LOG_LOCAL1);
   ** Syslog($LOG_EMERG, "help me!")
   ** CloseLog();
 */
CRX_FUNCTION(openlog)
{
	char *ident;
	crex_long option, facility;
	size_t ident_len;

	CREX_PARSE_PARAMETERS_START(3, 3)
		C_PARAM_STRING(ident, ident_len)
		C_PARAM_LONG(option)
		C_PARAM_LONG(facility)
	CREX_PARSE_PARAMETERS_END();

	if (BG(syslog_device)) {
		free(BG(syslog_device));
	}
	BG(syslog_device) = crex_strndup(ident, ident_len);
	crx_openlog(BG(syslog_device), option, facility);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Close connection to system logger */
CRX_FUNCTION(closelog)
{
	CREX_PARSE_PARAMETERS_NONE();

	crx_closelog();
	if (BG(syslog_device)) {
		free(BG(syslog_device));
		BG(syslog_device)=NULL;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Generate a system log message */
CRX_FUNCTION(syslog)
{
	crex_long priority;
	crex_string *message;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_LONG(priority)
		C_PARAM_STR(message)
	CREX_PARSE_PARAMETERS_END();

	crx_syslog_str(priority, message);
	RETURN_TRUE;
}
/* }}} */

#endif
