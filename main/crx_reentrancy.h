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
   | Author: Sascha Schumann <sascha@schumann.cx>                         |
   +----------------------------------------------------------------------+
 */

#ifndef CRX_REENTRANCY_H
#define CRX_REENTRANCY_H

#include "crx.h"

#include <sys/types.h>
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#include <time.h>

/* currently, CRX does not check for these functions, but assumes
   that they are available on all systems. */

#define HAVE_LOCALTIME 1
#define HAVE_GMTIME 1
#define HAVE_ASCTIME 1
#define HAVE_CTIME 1

#if defined(CRX_IRIX_TIME_R)
#undef HAVE_ASCTIME_R
#undef HAVE_CTIME_R
#endif

#if defined(CRX_HPUX_TIME_R)
#undef HAVE_LOCALTIME_R
#undef HAVE_ASCTIME_R
#undef HAVE_CTIME_R
#undef HAVE_GMTIME_R
#endif

BEGIN_EXTERN_C()

#if !defined(HAVE_LOCALTIME_R) && defined(HAVE_LOCALTIME)
#define CRX_NEED_REENTRANCY 1
CRXAPI struct tm *crx_localtime_r(const time_t *const timep, struct tm *p_tm);
#else
#define crx_localtime_r localtime_r
#ifdef MISSING_LOCALTIME_R_DECL
struct tm *localtime_r(const time_t *const timep, struct tm *p_tm);
#endif
#endif


#if !defined(HAVE_CTIME_R) && defined(HAVE_CTIME)
#define CRX_NEED_REENTRANCY 1
CRXAPI char *crx_ctime_r(const time_t *clock, char *buf);
#else
#define crx_ctime_r ctime_r
#ifdef MISSING_CTIME_R_DECL
char *ctime_r(const time_t *clock, char *buf);
#endif
#endif


#if !defined(HAVE_ASCTIME_R) && defined(HAVE_ASCTIME)
#define CRX_NEED_REENTRANCY 1
CRXAPI char *crx_asctime_r(const struct tm *tm, char *buf);
#else
#define crx_asctime_r asctime_r
#ifdef MISSING_ASCTIME_R_DECL
char *asctime_r(const struct tm *tm, char *buf);
#endif
#endif


#if !defined(HAVE_GMTIME_R) && defined(HAVE_GMTIME)
#define CRX_NEED_REENTRANCY 1
CRXAPI struct tm *crx_gmtime_r(const time_t *const timep, struct tm *p_tm);
#else
#define crx_gmtime_r gmtime_r
#ifdef MISSING_GMTIME_R_DECL
struct tm *crx_gmtime_r(const time_t *const timep, struct tm *p_tm);
#endif
#endif

#if !defined(HAVE_STRTOK_R)
CRXAPI char *crx_strtok_r(char *s, const char *delim, char **last);
#else
#define crx_strtok_r strtok_r
#ifdef MISSING_STRTOK_R_DECL
char *strtok_r(char *s, const char *delim, char **last);
#endif
#endif

END_EXTERN_C()

#if !defined(ZTS)
#undef CRX_NEED_REENTRANCY
#endif

#if defined(CRX_NEED_REENTRANCY)
void reentrancy_startup(void);
void reentrancy_shutdown(void);
#else
#define reentrancy_startup()
#define reentrancy_shutdown()
#endif

#endif
