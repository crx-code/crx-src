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

#ifndef MYSQLND_PRIV_H
#define MYSQLND_PRIV_H
CRXAPI extern MYSQLND_CLASS_METHOD_TABLE_NAME_FORWARD(mysqlnd_object_factory);
CRXAPI extern MYSQLND_CLASS_METHOD_TABLE_NAME_FORWARD(mysqlnd_conn);
CRXAPI extern MYSQLND_CLASS_METHOD_TABLE_NAME_FORWARD(mysqlnd_conn_data);
CRXAPI extern MYSQLND_CLASS_METHOD_TABLE_NAME_FORWARD(mysqlnd_res);
CRXAPI extern MYSQLND_CLASS_METHOD_TABLE_NAME_FORWARD(mysqlnd_result_unbuffered);
CRXAPI extern MYSQLND_CLASS_METHOD_TABLE_NAME_FORWARD(mysqlnd_result_buffered);
CRXAPI extern MYSQLND_CLASS_METHOD_TABLE_NAME_FORWARD(mysqlnd_protocol_payload_decoder_factory);
CRXAPI extern MYSQLND_CLASS_METHOD_TABLE_NAME_FORWARD(mysqlnd_protocol_packet_frame_codec);
CRXAPI extern MYSQLND_CLASS_METHOD_TABLE_NAME_FORWARD(mysqlnd_vio);
CRXAPI extern MYSQLND_CLASS_METHOD_TABLE_NAME_FORWARD(mysqlnd_upsert_status);
CRXAPI extern MYSQLND_CLASS_METHOD_TABLE_NAME_FORWARD(mysqlnd_error_info);
CRXAPI extern MYSQLND_CLASS_METHOD_TABLE_NAME_FORWARD(mysqlnd_command);

enum_func_status mysqlnd_handle_local_infile(MYSQLND_CONN_DATA * conn, const char * const filename, bool * is_warning);
#endif	/* MYSQLND_PRIV_H */
