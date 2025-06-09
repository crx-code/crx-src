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
  |         Frank M. Kromann <frank@kromann.info>                        |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_PDO_DBLIB_H
#define CRX_PDO_DBLIB_H

#ifdef PDO_DBLIB_IS_MSSQL
extern crex_module_entry pdo_mssql_module_entry;
#define crxext_pdo_mssql_ptr &pdo_mssql_module_entry
#else
extern crex_module_entry pdo_dblib_module_entry;
#define crxext_pdo_dblib_ptr &pdo_dblib_module_entry
#endif

#include "crx_version.h"
#define CRX_PDO_DBLIB_VERSION CRX_VERSION

#ifdef ZTS
# include "TSRM.h"
#endif

CRX_MINIT_FUNCTION(pdo_dblib);
CRX_MSHUTDOWN_FUNCTION(pdo_dblib);
CRX_MINFO_FUNCTION(pdo_dblib);
CRX_RSHUTDOWN_FUNCTION(pdo_dblib);

#endif
