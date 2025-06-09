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
   | Authors: Paul Panotzki - Bunyip Information Systems                  |
   |          Jim Winstead <jimw@crx.net>                                 |
   |          Sascha Schumann <sascha@schumann.cx>                        |
   +----------------------------------------------------------------------+
*/

#include "crx.h"
#include "crx_globals.h"
#include <stdlib.h>
#include <stddef.h>
#include "crx_network.h"
#include "file.h"

/* {{{ crx_fsockopen() */

static void crx_fsockopen_stream(INTERNAL_FUNCTION_PARAMETERS, int persistent)
{
	char *host;
	size_t host_len;
	crex_long port = -1;
	zval *zerrno = NULL, *zerrstr = NULL;
	double timeout;
	bool timeout_is_null = 1;
#ifndef CRX_WIN32
	time_t conv;
#else
	long conv;
#endif
	struct timeval tv;
	char *hashkey = NULL;
	crx_stream *stream = NULL;
	int err;
	char *hostname = NULL;
	size_t hostname_len;
	crex_string *errstr = NULL;

	CREX_PARSE_PARAMETERS_START(1, 5)
		C_PARAM_STRING(host, host_len)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(port)
		C_PARAM_ZVAL(zerrno)
		C_PARAM_ZVAL(zerrstr)
		C_PARAM_DOUBLE_OR_NULL(timeout, timeout_is_null)
	CREX_PARSE_PARAMETERS_END();

	RETVAL_FALSE;

	if (timeout_is_null) {
		timeout = (double)FG(default_socket_timeout);
	}

	if (persistent) {
		spprintf(&hashkey, 0, "pfsockopen__%s:" CREX_LONG_FMT, host, port);
	}

	if (port > 0) {
		hostname_len = spprintf(&hostname, 0, "%s:" CREX_LONG_FMT, host, port);
	} else {
		hostname_len = host_len;
		hostname = host;
	}

	/* prepare the timeout value for use */
#ifndef CRX_WIN32
	conv = (time_t) (timeout * 1000000.0);
	tv.tv_sec = conv / 1000000;
#else
	conv = (long) (timeout * 1000000.0);
	tv.tv_sec = conv / 1000000;
#endif
	tv.tv_usec = conv % 1000000;

	stream = crx_stream_xport_create(hostname, hostname_len, REPORT_ERRORS,
			STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT, hashkey, &tv, NULL, &errstr, &err);

	if (port > 0) {
		efree(hostname);
	}
	if (stream == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to connect to %s:" CREX_LONG_FMT " (%s)", host, port, errstr == NULL ? "Unknown error" : ZSTR_VAL(errstr));
	}

	if (hashkey) {
		efree(hashkey);
	}

	if (stream == NULL) {
		if (zerrno) {
			CREX_TRY_ASSIGN_REF_LONG(zerrno, err);
		}
		if (errstr) {
			if (zerrstr) {
				CREX_TRY_ASSIGN_REF_STR(zerrstr, errstr);
			} else {
				crex_string_release(errstr);
			}
		}

		RETURN_FALSE;
	}

	if (zerrno) {
		CREX_TRY_ASSIGN_REF_LONG(zerrno, 0);
	}
	if (zerrstr) {
		CREX_TRY_ASSIGN_REF_EMPTY_STRING(zerrstr);
	}

	if (errstr) {
		crex_string_release_ex(errstr, 0);
	}

	crx_stream_to_zval(stream, return_value);
}

/* }}} */

/* {{{ Open Internet or Unix domain socket connection */
CRX_FUNCTION(fsockopen)
{
	crx_fsockopen_stream(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ Open persistent Internet or Unix domain socket connection */
CRX_FUNCTION(pfsockopen)
{
	crx_fsockopen_stream(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */
