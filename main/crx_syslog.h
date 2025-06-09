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
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_SYSLOG_H
#define CRX_SYSLOG_H

#include "crx.h"

#ifdef CRX_WIN32
#include "win32/syslog.h"
#else
#include <crx_config.h>
#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif
#endif

/* Syslog filters */
#define CRX_SYSLOG_FILTER_ALL		0
#define CRX_SYSLOG_FILTER_NO_CTRL	1
#define CRX_SYSLOG_FILTER_ASCII		2
#define CRX_SYSLOG_FILTER_RAW		3

BEGIN_EXTERN_C()
CRXAPI void crx_syslog_str(int priority, const crex_string* message);
CRXAPI void crx_syslog(int, const char *format, ...);
CRXAPI void crx_openlog(const char *, int, int);
CRXAPI void crx_closelog(void);
END_EXTERN_C()

#endif
