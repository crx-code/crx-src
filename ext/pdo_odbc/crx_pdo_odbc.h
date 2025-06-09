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
  | Author: Wez Furlong <wez@crx.net>                                    |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_PDO_ODBC_H
#define CRX_PDO_ODBC_H

extern crex_module_entry pdo_odbc_module_entry;
#define crxext_pdo_odbc_ptr &pdo_odbc_module_entry

#include "crx_version.h"
#define CRX_PDO_ODBC_VERSION CRX_VERSION

#ifdef ZTS
#include "TSRM.h"
#endif

CRX_MINIT_FUNCTION(pdo_odbc);
CRX_MSHUTDOWN_FUNCTION(pdo_odbc);
CRX_RINIT_FUNCTION(pdo_odbc);
CRX_RSHUTDOWN_FUNCTION(pdo_odbc);
CRX_MINFO_FUNCTION(pdo_odbc);

#endif	/* CRX_PDO_ODBC_H */
