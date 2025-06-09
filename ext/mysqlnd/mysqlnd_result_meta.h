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
  |          Johannes Schlüter <johannes@crx.net>                        |
  |          Ulf Wendel <uw@crx.net>                                     |
  +----------------------------------------------------------------------+
*/

#ifndef MYSQLND_RESULT_META_H
#define MYSQLND_RESULT_META_H

CRXAPI MYSQLND_RES_METADATA * mysqlnd_result_meta_init(MYSQLND_RES * result, unsigned int field_count);
CRXAPI struct st_mysqlnd_res_meta_methods * mysqlnd_result_metadata_get_methods(void);
CRXAPI void ** _mysqlnd_plugin_get_plugin_result_metadata_data(const MYSQLND_RES_METADATA * meta, unsigned int plugin_id);

#endif /* MYSQLND_RESULT_META_H */
