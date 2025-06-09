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
   | Authors: Scott MacVicar <scottmac@crx.net>                           |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_SQLITE3_H
#define CRX_SQLITE3_H

#define CRX_SQLITE3_VERSION	CRX_VERSION

extern crex_module_entry sqlite3_module_entry;
#define crxext_sqlite3_ptr &sqlite3_module_entry

CREX_BEGIN_MODULE_GLOBALS(sqlite3)
	char *extension_dir;
	int dbconfig_defensive;
CREX_END_MODULE_GLOBALS(sqlite3)

#if defined(ZTS) && defined(COMPILE_DL_SQLITE3)
CREX_TSRMLS_CACHE_EXTERN()
#endif

CREX_EXTERN_MODULE_GLOBALS(sqlite3)
#define SQLITE3G(v) CREX_MODULE_GLOBALS_ACCESSOR(sqlite3, v)

#define CRX_SQLITE3_ASSOC	1<<0
#define CRX_SQLITE3_NUM		1<<1
#define CRX_SQLITE3_BOTH	(CRX_SQLITE3_ASSOC|CRX_SQLITE3_NUM)

#endif
