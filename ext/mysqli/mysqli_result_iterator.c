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
  | Authors: Georg Richter <georg@crx.net>                               |
  |          Andrey Hristov <andrey@crx.net>                             |
  |          Ulf Wendel <uw@crx.net>                                     |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <signal.h>

#include "crx.h"
#include "crx_ini.h"
#include "crx_mysqli_structs.h"
#include "mysqli_priv.h"
#include "crex_interfaces.h"


extern const crex_object_iterator_funcs crx_mysqli_result_iterator_funcs;

typedef struct {
	crex_object_iterator  intern;
	mysqli_object *result;
	zval current_row;
	my_longlong row_num;
} crx_mysqli_result_iterator;


/* {{{ */
crex_object_iterator *crx_mysqli_result_get_iterator(crex_class_entry *ce, zval *object, int by_ref)
{
	crx_mysqli_result_iterator *iterator;

	if (by_ref) {
		crex_throw_error(NULL, "An iterator cannot be used with foreach by reference");
		return NULL;
	}

	iterator = ecalloc(1, sizeof(crx_mysqli_result_iterator));
	crex_iterator_init(&iterator->intern);

	C_ADDREF_P(object);
	ZVAL_OBJ(&iterator->intern.data, C_OBJ_P(object));
	iterator->intern.funcs = &crx_mysqli_result_iterator_funcs;
	iterator->result = C_MYSQLI_P(object);
	iterator->row_num = -1;

	return &iterator->intern;
}
/* }}} */

/* {{{ */
static void crx_mysqli_result_iterator_dtor(crex_object_iterator *iter)
{
	crx_mysqli_result_iterator *iterator = (crx_mysqli_result_iterator*)iter;

	/* cleanup handled in sxe_object_dtor as we don't always have an iterator wrapper */
	zval_ptr_dtor(&iterator->intern.data);
	zval_ptr_dtor(&iterator->current_row);
}
/* }}} */

/* {{{ */
static int crx_mysqli_result_iterator_valid(crex_object_iterator *iter)
{
	crx_mysqli_result_iterator *iterator = (crx_mysqli_result_iterator*) iter;

	return C_TYPE(iterator->current_row) == IS_ARRAY ? SUCCESS : FAILURE;
}
/* }}} */

/* {{{ */
static zval *crx_mysqli_result_iterator_current_data(crex_object_iterator *iter)
{
	crx_mysqli_result_iterator *iterator = (crx_mysqli_result_iterator*) iter;

	return &iterator->current_row;
}
/* }}} */

/* {{{ */
static void crx_mysqli_result_iterator_move_forward(crex_object_iterator *iter)
{

	crx_mysqli_result_iterator *iterator = (crx_mysqli_result_iterator*) iter;
	mysqli_object *intern = iterator->result;
	MYSQL_RES	*result;

	MYSQLI_FETCH_RESOURCE_BY_OBJ(result, MYSQL_RES *, intern, "mysqli_result", MYSQLI_STATUS_VALID);

	zval_ptr_dtor(&iterator->current_row);
	crx_mysqli_fetch_into_hash_aux(&iterator->current_row, result, MYSQLI_ASSOC);
	if (C_TYPE(iterator->current_row) == IS_ARRAY) {
		iterator->row_num++;
	}
}
/* }}} */

/* {{{ */
static void crx_mysqli_result_iterator_rewind(crex_object_iterator *iter)
{
	crx_mysqli_result_iterator *iterator = (crx_mysqli_result_iterator*) iter;
	mysqli_object *intern = iterator->result;
	MYSQL_RES	*result;

	MYSQLI_FETCH_RESOURCE_BY_OBJ(result, MYSQL_RES *, intern, "mysqli_result", MYSQLI_STATUS_VALID);

	if (mysqli_result_is_unbuffered(result)) {
		if (result->unbuf->eof_reached) {
			crex_error(E_WARNING, "Data fetched with MYSQLI_USE_RESULT can be iterated only once");
			return;
		}
	} else {
		mysql_data_seek(result, 0);
	}
	iterator->row_num = -1;
	crx_mysqli_result_iterator_move_forward(iter);
}
/* }}} */

/* {{{ crx_mysqli_result_iterator_current_key */
static void crx_mysqli_result_iterator_current_key(crex_object_iterator *iter, zval *key)
{
	crx_mysqli_result_iterator *iterator = (crx_mysqli_result_iterator*) iter;

	ZVAL_LONG(key, iterator->row_num);
}
/* }}} */

/* {{{ crx_mysqli_result_iterator_funcs */
const crex_object_iterator_funcs crx_mysqli_result_iterator_funcs = {
	crx_mysqli_result_iterator_dtor,
	crx_mysqli_result_iterator_valid,
	crx_mysqli_result_iterator_current_data,
	crx_mysqli_result_iterator_current_key,
	crx_mysqli_result_iterator_move_forward,
	crx_mysqli_result_iterator_rewind,
	NULL,
	NULL, /* get_gc */
};
/* }}} */
