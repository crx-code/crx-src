/*
   +----------------------------------------------------------------------+
   | Crex OPcache                                                         |
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
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   |          Stanislav Malyshev <stas@crex.com>                          |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#ifdef CREX_WIN32
# include <process.h>
#endif
#include "CrexAccelerator.h"

static void crex_accel_error_va_args(int type, const char *format, va_list args)
{
	time_t timestamp;
	char *time_string;
	FILE * fLog = NULL;

	if (type <= ZCG(accel_directives).log_verbosity_level) {

	timestamp = time(NULL);
	time_string = asctime(localtime(&timestamp));
	time_string[24] = 0;

	if (!ZCG(accel_directives).error_log ||
		!*ZCG(accel_directives).error_log ||
		strcmp(ZCG(accel_directives).error_log, "stderr") == 0) {

		fLog = stderr;
	} else {
		fLog = fopen(ZCG(accel_directives).error_log, "a");
		if (!fLog) {
			fLog = stderr;
		}
	}

#ifdef ZTS
		fprintf(fLog, "%s (" CREX_ULONG_FMT "): ", time_string, (crex_ulong)tsrm_thread_id());
#else
		fprintf(fLog, "%s (%d): ", time_string, getpid());
#endif

		switch (type) {
			case ACCEL_LOG_FATAL:
				fprintf(fLog, "Fatal Error ");
				break;
			case ACCEL_LOG_ERROR:
				fprintf(fLog, "Error ");
				break;
			case ACCEL_LOG_WARNING:
				fprintf(fLog, "Warning ");
				break;
			case ACCEL_LOG_INFO:
				fprintf(fLog, "Message ");
				break;
			case ACCEL_LOG_DEBUG:
				fprintf(fLog, "Debug ");
				break;
		}

		vfprintf(fLog, format, args);
		fprintf(fLog, "\n");

		fflush(fLog);
		if (fLog != stderr) {
			fclose(fLog);
		}
	}
	/* perform error handling even without logging the error */
	switch (type) {
		case ACCEL_LOG_ERROR:
			crex_bailout();
			break;
		case ACCEL_LOG_FATAL:
			exit(-2);
			break;
	}

}

void crex_accel_error(int type, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	crex_accel_error_va_args(type, format, args);
	va_end(args);
}

CREX_NORETURN void crex_accel_error_noreturn(int type, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	CREX_ASSERT(type == ACCEL_LOG_FATAL || type == ACCEL_LOG_ERROR);
	crex_accel_error_va_args(type, format, args);
	va_end(args);
	/* Should never reach this. */
	abort();
}
