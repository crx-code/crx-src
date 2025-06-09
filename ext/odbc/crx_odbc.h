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
   | Authors: Stig Sæther Bakken <ssb@crx.net>                            |
   |          Andreas Karajannis <Andreas.Karajannis@gmd.de>              |
   |	      Kevin N. Shallow <kshallow@tampabay.rr.com>                 |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_ODBC_H
#define CRX_ODBC_H

#ifdef HAVE_UODBC

#ifdef ZTS
#include "TSRM.h"
#endif

extern crex_module_entry odbc_module_entry;
#define odbc_module_ptr &odbc_module_entry

#include "crx_version.h"
#define CRX_ODBC_VERSION CRX_VERSION

#if defined(HAVE_DBMAKER) || defined(CRX_WIN32) || defined(HAVE_IBMDB2) || defined(HAVE_UNIXODBC) || defined(HAVE_IODBC)
# define CRX_ODBC_HAVE_FETCH_HASH 1
#endif

/* user functions */
CRX_MINIT_FUNCTION(odbc);
CRX_MSHUTDOWN_FUNCTION(odbc);
CRX_RINIT_FUNCTION(odbc);
CRX_RSHUTDOWN_FUNCTION(odbc);
CRX_MINFO_FUNCTION(odbc);

#ifdef CRX_WIN32
# define CRX_ODBC_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
# define CRX_ODBC_API __attribute__ ((visibility("default")))
#else
# define CRX_ODBC_API
#endif

#else

#define odbc_module_ptr NULL

#endif /* HAVE_UODBC */

#define crxext_odbc_ptr odbc_module_ptr

#endif /* CRX_ODBC_H */
