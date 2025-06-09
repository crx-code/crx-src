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
   | Author: Paul Panotzki - Bunyip Information Systems                   |
   +----------------------------------------------------------------------+
 */

#include "crx.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef CRX_WIN32
#include "win32/time.h"
#include "win32/getrusage.h"
#else
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "ext/date/crx_date.h"

#define NUL  '\0'
#define MICRO_IN_SEC 1000000.00
#define SEC_IN_MIN 60

#ifdef HAVE_GETTIMEOFDAY
static void _crx_gettimeofday(INTERNAL_FUNCTION_PARAMETERS, int mode)
{
	bool get_as_float = 0;
	struct timeval tp = {0};

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(get_as_float)
	CREX_PARSE_PARAMETERS_END();

	if (gettimeofday(&tp, NULL)) {
		CREX_ASSERT(0 && "gettimeofday() can't fail");
	}

	if (get_as_float) {
		RETURN_DOUBLE((double)(tp.tv_sec + tp.tv_usec / MICRO_IN_SEC));
	}

	if (mode) {
		timelib_time_offset *offset;

		offset = timelib_get_time_zone_info(tp.tv_sec, get_timezone_info());

		array_init(return_value);
		add_assoc_long(return_value, "sec", tp.tv_sec);
		add_assoc_long(return_value, "usec", tp.tv_usec);

		add_assoc_long(return_value, "minuteswest", -offset->offset / SEC_IN_MIN);
		add_assoc_long(return_value, "dsttime", offset->is_dst);

		timelib_time_offset_dtor(offset);
	} else {
		RETURN_NEW_STR(crex_strpprintf(0, "%.8F %ld", tp.tv_usec / MICRO_IN_SEC, (long)tp.tv_sec));
	}
}

/* {{{ Returns either a string or a float containing the current time in seconds and microseconds */
CRX_FUNCTION(microtime)
{
	_crx_gettimeofday(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ Returns the current time as array */
CRX_FUNCTION(gettimeofday)
{
	_crx_gettimeofday(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
#endif
/* }}} */

#ifdef HAVE_GETRUSAGE
/* {{{ Returns an array of usage statistics */
CRX_FUNCTION(getrusage)
{
	struct rusage usg;
	crex_long pwho = 0;
	int who = RUSAGE_SELF;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(pwho)
	CREX_PARSE_PARAMETERS_END();

	if (pwho == 1) {
		who = RUSAGE_CHILDREN;
	}

	memset(&usg, 0, sizeof(struct rusage));

	if (getrusage(who, &usg) == -1) {
		RETURN_FALSE;
	}

	array_init(return_value);

#define CRX_RUSAGE_PARA(a) \
		add_assoc_long(return_value, #a, usg.a)

#ifdef CRX_WIN32 /* Windows only implements a limited amount of fields from the rusage struct */
	CRX_RUSAGE_PARA(ru_majflt);
	CRX_RUSAGE_PARA(ru_maxrss);
#elif !defined(_OSD_POSIX) && !defined(__HAIKU__)
	CRX_RUSAGE_PARA(ru_oublock);
	CRX_RUSAGE_PARA(ru_inblock);
	CRX_RUSAGE_PARA(ru_msgsnd);
	CRX_RUSAGE_PARA(ru_msgrcv);
	CRX_RUSAGE_PARA(ru_maxrss);
	CRX_RUSAGE_PARA(ru_ixrss);
	CRX_RUSAGE_PARA(ru_idrss);
	CRX_RUSAGE_PARA(ru_minflt);
	CRX_RUSAGE_PARA(ru_majflt);
	CRX_RUSAGE_PARA(ru_nsignals);
	CRX_RUSAGE_PARA(ru_nvcsw);
	CRX_RUSAGE_PARA(ru_nivcsw);
	CRX_RUSAGE_PARA(ru_nswap);
#endif /*_OSD_POSIX*/
	CRX_RUSAGE_PARA(ru_utime.tv_usec);
	CRX_RUSAGE_PARA(ru_utime.tv_sec);
	CRX_RUSAGE_PARA(ru_stime.tv_usec);
	CRX_RUSAGE_PARA(ru_stime.tv_sec);

#undef CRX_RUSAGE_PARA
}
#endif /* HAVE_GETRUSAGE */

/* }}} */
