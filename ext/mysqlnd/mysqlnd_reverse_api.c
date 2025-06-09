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

#include "crx.h"
#include "mysqlnd.h"
#include "mysqlnd_priv.h"
#include "mysqlnd_debug.h"
#include "mysqlnd_reverse_api.h"


static HashTable mysqlnd_api_ext_ht;


/* {{{ mysqlnd_reverse_api_init */
CRXAPI void
mysqlnd_reverse_api_init(void)
{
	crex_hash_init(&mysqlnd_api_ext_ht, 3, NULL, NULL, 1);
}
/* }}} */


/* {{{ mysqlnd_reverse_api_end */
CRXAPI void
mysqlnd_reverse_api_end(void)
{
	crex_hash_destroy(&mysqlnd_api_ext_ht);
}
/* }}} */


/* {{{ myslqnd_get_api_extensions */
CRXAPI HashTable *
mysqlnd_reverse_api_get_api_list(void)
{
	return &mysqlnd_api_ext_ht;
}
/* }}} */


/* {{{ mysqlnd_reverse_api_register_api */
CRXAPI void
mysqlnd_reverse_api_register_api(const MYSQLND_REVERSE_API * apiext)
{
	crex_hash_str_add_ptr(&mysqlnd_api_ext_ht, apiext->module->name, strlen(apiext->module->name), (void*)apiext);
}
/* }}} */


/* {{{ zval_to_mysqlnd */
CRXAPI MYSQLND *
zval_to_mysqlnd(zval * zv, const unsigned int client_api_capabilities, unsigned int * save_client_api_capabilities)
{
	MYSQLND_REVERSE_API *api;
	CREX_HASH_MAP_FOREACH_PTR(&mysqlnd_api_ext_ht, api) {
		if (api->conversion_cb) {
			MYSQLND *retval = api->conversion_cb(zv);
			if (retval) {
				if (retval->data) {
					*save_client_api_capabilities = retval->data->m->negotiate_client_api_capabilities(retval->data, client_api_capabilities);
				}
				return retval;
			}
		}
	} CREX_HASH_FOREACH_END();
	return NULL;
}
/* }}} */
