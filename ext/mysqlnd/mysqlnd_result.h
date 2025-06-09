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
  | Authors: Andrey Hristov <andrey@crx.net>                             |
  |          Ulf Wendel <uw@crx.net>                                     |
  +----------------------------------------------------------------------+
*/

#ifndef MYSQLND_RESULT_H
#define MYSQLND_RESULT_H

CRXAPI MYSQLND_RES * mysqlnd_result_init(const unsigned int field_count);
CRXAPI MYSQLND_RES_UNBUFFERED * mysqlnd_result_unbuffered_init(MYSQLND_RES * result, const unsigned int field_count, MYSQLND_STMT_DATA *stmt);
CRXAPI MYSQLND_RES_BUFFERED * mysqlnd_result_buffered_init(MYSQLND_RES * result, const unsigned int field_count, MYSQLND_STMT_DATA *stmt);

enum_func_status mysqlnd_query_read_result_set_header(MYSQLND_CONN_DATA * conn, MYSQLND_STMT * stmt);

#endif /* MYSQLND_RESULT_H */
