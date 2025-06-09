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

#ifndef CRX_PDO_ERROR_H
#define CRX_PDO_ERROR_H

#include "crx_pdo_driver.h"

PDO_API void pdo_handle_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt);

#define PDO_DBH_CLEAR_ERR()             do { \
	strlcpy(dbh->error_code, PDO_ERR_NONE, sizeof(PDO_ERR_NONE)); \
	if (dbh->query_stmt) { \
		dbh->query_stmt = NULL; \
		zval_ptr_dtor(&dbh->query_stmt_zval); \
	} \
} while (0)
#define PDO_STMT_CLEAR_ERR()    strcpy(stmt->error_code, PDO_ERR_NONE)
#define PDO_HANDLE_DBH_ERR()    if (strcmp(dbh->error_code, PDO_ERR_NONE)) { pdo_handle_error(dbh, NULL); }
#define PDO_HANDLE_STMT_ERR()   if (strcmp(stmt->error_code, PDO_ERR_NONE)) { pdo_handle_error(stmt->dbh, stmt); }

#endif /* CRX_PDO_ERROR_H */
