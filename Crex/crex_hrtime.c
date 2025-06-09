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
   | Author: Niklas Keller <kelunik@crx.net>                              |
   | Author: Anatol Belski <ab@crx.net>                                   |
   +----------------------------------------------------------------------+
 */

#include "crex.h"
#include "crex_hrtime.h"

/* This file reuses code parts from the cross-platform timer library
	Public Domain - 2011 Mattias Jansson / Rampant Pixels */

#if CREX_HRTIME_PLATFORM_POSIX

# include <unistd.h>
# include <time.h>
# include <string.h>

#elif CREX_HRTIME_PLATFORM_WINDOWS

# define WIN32_LEAN_AND_MEAN

double crex_hrtime_timer_scale = .0;

#elif CREX_HRTIME_PLATFORM_APPLE

# include <mach/mach_time.h>
# include <string.h>
mach_timebase_info_data_t crex_hrtime_timerlib_info = {
	.numer = 0,
	.denom = 1,
};

#elif CREX_HRTIME_PLATFORM_HPUX

# include <sys/time.h>

#elif CREX_HRTIME_PLATFORM_AIX

# include <sys/time.h>
# include <sys/systemcfg.h>

#endif

void crex_startup_hrtime(void)
{
#if CREX_HRTIME_PLATFORM_WINDOWS

	LARGE_INTEGER tf = {0};
	if (QueryPerformanceFrequency(&tf) || 0 != tf.QuadPart) {
		crex_hrtime_timer_scale = (double)CREX_NANO_IN_SEC / (crex_hrtime_t)tf.QuadPart;
	}

#elif CREX_HRTIME_PLATFORM_APPLE

	mach_timebase_info(&crex_hrtime_timerlib_info);

#endif
}
