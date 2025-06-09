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
  | Author: George Schlossnagle <george@omniti.com>                      |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_PDO_MYSQL_H
#define CRX_PDO_MYSQL_H

extern crex_module_entry pdo_mysql_module_entry;
#define crxext_pdo_mysql_ptr &pdo_mysql_module_entry

#include "crx_version.h"
#define CRX_PDO_MYSQL_VERSION CRX_VERSION

#ifdef CRX_WIN32
#define CRX_PDO_MYSQL_API __declspec(dllexport)
#else
#define CRX_PDO_MYSQL_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif


#endif	/* CRX_PDO_MYSQL_H */
