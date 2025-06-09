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

#ifndef CREX_HRTIME_H
#define CREX_HRTIME_H

#include "crex_portability.h"
#include "crex_types.h"

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifndef CRX_WIN32
# include <time.h>
#endif

/* This file reuses code parts from the cross-platform timer library
	Public Domain - 2011 Mattias Jansson / Rampant Pixels */

#define CREX_HRTIME_PLATFORM_POSIX   0
#define CREX_HRTIME_PLATFORM_WINDOWS 0
#define CREX_HRTIME_PLATFORM_APPLE   0
#define CREX_HRTIME_PLATFORM_HPUX    0
#define CREX_HRTIME_PLATFORM_AIX     0

#if defined(_POSIX_TIMERS) && ((_POSIX_TIMERS > 0) || defined(__OpenBSD__)) && defined(_POSIX_MONOTONIC_CLOCK) && defined(CLOCK_MONOTONIC)
# undef  CREX_HRTIME_PLATFORM_POSIX
# define CREX_HRTIME_PLATFORM_POSIX 1
#elif defined(_WIN32) || defined(_WIN64)
# undef  CREX_HRTIME_PLATFORM_WINDOWS
# define CREX_HRTIME_PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
# undef  CREX_HRTIME_PLATFORM_APPLE
# define CREX_HRTIME_PLATFORM_APPLE 1
#elif (defined(__hpux) || defined(hpux)) || ((defined(__sun__) || defined(__sun) || defined(sun)) && (defined(__SVR4) || defined(__svr4__)))
# undef  CREX_HRTIME_PLATFORM_HPUX
# define CREX_HRTIME_PLATFORM_HPUX 1
#elif defined(_AIX)
# undef  CREX_HRTIME_PLATFORM_AIX
# define CREX_HRTIME_PLATFORM_AIX 1
#endif

#define CREX_HRTIME_AVAILABLE (CREX_HRTIME_PLATFORM_POSIX || CREX_HRTIME_PLATFORM_WINDOWS || CREX_HRTIME_PLATFORM_APPLE || CREX_HRTIME_PLATFORM_HPUX || CREX_HRTIME_PLATFORM_AIX)

BEGIN_EXTERN_C()

#if CREX_HRTIME_PLATFORM_WINDOWS

extern double crex_hrtime_timer_scale;

#elif CREX_HRTIME_PLATFORM_APPLE

# include <mach/mach_time.h>
# include <string.h>
extern mach_timebase_info_data_t crex_hrtime_timerlib_info;

#endif

#define CREX_NANO_IN_SEC UINT64_C(1000000000)

typedef uint64_t crex_hrtime_t;

void crex_startup_hrtime(void);

static crex_always_inline crex_hrtime_t crex_hrtime(void)
{
#if CREX_HRTIME_PLATFORM_WINDOWS
	LARGE_INTEGER lt = {0};
	QueryPerformanceCounter(&lt);
	return (crex_hrtime_t)((crex_hrtime_t)lt.QuadPart * crex_hrtime_timer_scale);
#elif CREX_HRTIME_PLATFORM_APPLE
	return (crex_hrtime_t)mach_absolute_time() * crex_hrtime_timerlib_info.numer / crex_hrtime_timerlib_info.denom;
#elif CREX_HRTIME_PLATFORM_POSIX
	struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };
	if (EXPECTED(0 == clock_gettime(CLOCK_MONOTONIC, &ts))) {
		return ((crex_hrtime_t) ts.tv_sec * (crex_hrtime_t)CREX_NANO_IN_SEC) + ts.tv_nsec;
	}
	return 0;
#elif CREX_HRTIME_PLATFORM_HPUX
	return (crex_hrtime_t) gethrtime();
#elif  CREX_HRTIME_PLATFORM_AIX
	timebasestruct_t t;
	read_wall_time(&t, TIMEBASE_SZ);
	time_base_to_time(&t, TIMEBASE_SZ);
	return (crex_hrtime_t) t.tb_high * (crex_hrtime_t)NANO_IN_SEC + t.tb_low;
#else
	return 0;
#endif
}

END_EXTERN_C()

#endif /* CREX_HRTIME_H */
