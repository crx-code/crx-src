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
   | Author: Jani Lehtimäki <jkl@njet.net>                                |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_VAR_H
#define CRX_VAR_H

#include "ext/standard/basic_functions.h"
#include "crex_smart_str_public.h"

CRX_MINIT_FUNCTION(var);

CRXAPI void crx_var_dump(zval *struc, int level);
CRXAPI void crx_var_export(zval *struc, int level);
CRXAPI void crx_var_export_ex(zval *struc, int level, smart_str *buf);

CRXAPI void crx_debug_zval_dump(zval *struc, int level);

typedef struct crx_serialize_data *crx_serialize_data_t;
typedef struct crx_unserialize_data *crx_unserialize_data_t;

CRXAPI void crx_var_serialize(smart_str *buf, zval *struc, crx_serialize_data_t *data);
CRXAPI int crx_var_unserialize(zval *rval, const unsigned char **p, const unsigned char *max, crx_unserialize_data_t *var_hash);
CRXAPI int crx_var_unserialize_ref(zval *rval, const unsigned char **p, const unsigned char *max, crx_unserialize_data_t *var_hash);
CRXAPI int crx_var_unserialize_intern(zval *rval, const unsigned char **p, const unsigned char *max, crx_unserialize_data_t *var_hash);

CRXAPI crx_serialize_data_t crx_var_serialize_init(void);
CRXAPI void crx_var_serialize_destroy(crx_serialize_data_t d);
CRXAPI crx_unserialize_data_t crx_var_unserialize_init(void);
CRXAPI void crx_var_unserialize_destroy(crx_unserialize_data_t d);
CRXAPI HashTable *crx_var_unserialize_get_allowed_classes(crx_unserialize_data_t d);
CRXAPI void crx_var_unserialize_set_allowed_classes(crx_unserialize_data_t d, HashTable *classes);
CRXAPI void crx_var_unserialize_set_max_depth(crx_unserialize_data_t d, crex_long max_depth);
CRXAPI crex_long crx_var_unserialize_get_max_depth(crx_unserialize_data_t d);
CRXAPI void crx_var_unserialize_set_cur_depth(crx_unserialize_data_t d, crex_long cur_depth);
CRXAPI crex_long crx_var_unserialize_get_cur_depth(crx_unserialize_data_t d);

#define CRX_VAR_SERIALIZE_INIT(d) \
	(d) = crx_var_serialize_init()

#define CRX_VAR_SERIALIZE_DESTROY(d) \
	crx_var_serialize_destroy(d)

#define CRX_VAR_UNSERIALIZE_INIT(d) \
	(d) = crx_var_unserialize_init()

#define CRX_VAR_UNSERIALIZE_DESTROY(d) \
	crx_var_unserialize_destroy(d)

CRXAPI void crx_unserialize_with_options(zval *return_value, const char *buf, const size_t buf_len, HashTable *options, const char* function_name);

CRXAPI void var_replace(crx_unserialize_data_t *var_hash, zval *ozval, zval *nzval);
CRXAPI void var_push_dtor(crx_unserialize_data_t *var_hash, zval *val);
CRXAPI zval *var_tmp_var(crx_unserialize_data_t *var_hashx);
CRXAPI void var_destroy(crx_unserialize_data_t *var_hash);

#endif /* CRX_VAR_H */
