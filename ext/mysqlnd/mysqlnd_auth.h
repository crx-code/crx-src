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

#ifndef MYSQLND_AUTH_H
#define MYSQLND_AUTH_H
enum_func_status
mysqlnd_auth_handshake(MYSQLND_CONN_DATA * conn,
						const char * const user,
						const char * const passwd,
						const size_t passwd_len,
						const char * const db,
						const size_t db_len,
						const MYSQLND_SESSION_OPTIONS * const session_options,
						const crex_ulong mysql_flags,
						const unsigned int server_charset_no,
						const bool use_full_blown_auth_packet,
						const char * const auth_protocol,
						struct st_mysqlnd_authentication_plugin * auth_plugin,
						const crex_uchar * const orig_auth_plugin_data,
						const size_t orig_auth_plugin_data_len,
						const crex_uchar * const auth_plugin_data,
						const size_t auth_plugin_data_len,
						char ** switch_to_auth_protocol,
						size_t * const switch_to_auth_protocol_len,
						crex_uchar ** switch_to_auth_protocol_data,
						size_t * const switch_to_auth_protocol_data_len
						);

enum_func_status
mysqlnd_auth_change_user(MYSQLND_CONN_DATA * const conn,
								const char * const user,
								const size_t user_len,
								const char * const passwd,
								const size_t passwd_len,
								const char * const db,
								const size_t db_len,
								const bool silent,
								const bool use_full_blown_auth_packet,
								const char * const auth_protocol,
								struct st_mysqlnd_authentication_plugin * auth_plugin,
								const crex_uchar * const orig_auth_plugin_data,
								const size_t orig_auth_plugin_data_len,
								const crex_uchar * auth_plugin_data,
								const size_t auth_plugin_data_len,
								char ** switch_to_auth_protocol,
								size_t * const switch_to_auth_protocol_len,
								crex_uchar ** switch_to_auth_protocol_data,
								size_t * const switch_to_auth_protocol_data_len
								);


enum_func_status
mysqlnd_connect_run_authentication(
			MYSQLND_CONN_DATA * const conn,
			const char * const user,
			const char * const passwd,
			const char * const db,
			const size_t db_len,
			const size_t passwd_len,
			const MYSQLND_STRING authentication_plugin_data,
			const char * const authentication_protocol,
			const unsigned int charset_no,
			const size_t server_capabilities,
			const MYSQLND_SESSION_OPTIONS * const session_options,
			const crex_ulong mysql_flags
			);

enum_func_status
mysqlnd_run_authentication(
			MYSQLND_CONN_DATA * const conn,
			const char * const user,
			const char * const passwd,
			const size_t passwd_len,
			const char * const db,
			const size_t db_len,
			const MYSQLND_STRING auth_plugin_data,
			const char * const auth_protocol,
			const unsigned int charset_no,
			const MYSQLND_SESSION_OPTIONS * const session_options,
			const crex_ulong mysql_flags,
			const bool silent,
			const bool is_change_user
			);

CRXAPI void crx_mysqlnd_scramble(crex_uchar * const buffer, const crex_uchar * const scramble, const crex_uchar * const pass, const size_t pass_len);

#endif /* MYSQLND_AUTH_H */
