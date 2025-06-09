/*
   +----------------------------------------------------------------------+
   | Crex Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Crex Technologies Ltd. (http://www.crex.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Marcus Boerger <helly@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_INTERFACES_H
#define CREX_INTERFACES_H

#include "crex.h"
#include "crex_API.h"

BEGIN_EXTERN_C()

extern CREX_API crex_class_entry *crex_ce_traversable;
extern CREX_API crex_class_entry *crex_ce_aggregate;
extern CREX_API crex_class_entry *crex_ce_iterator;
extern CREX_API crex_class_entry *crex_ce_arrayaccess;
extern CREX_API crex_class_entry *crex_ce_serializable;
extern CREX_API crex_class_entry *crex_ce_countable;
extern CREX_API crex_class_entry *crex_ce_stringable;

typedef struct _crex_user_iterator {
	crex_object_iterator     it;
	crex_class_entry         *ce;
	zval                     value;
} crex_user_iterator;

CREX_API zval* crex_call_method(crex_object *object, crex_class_entry *obj_ce, crex_function **fn_proxy, const char *function_name, size_t function_name_len, zval *retval, uint32_t param_count, zval* arg1, zval* arg2);

static crex_always_inline zval* crex_call_method_with_0_params(crex_object *object, crex_class_entry *obj_ce,
		crex_function **fn_proxy, const char *function_name, zval *retval)
{
	return crex_call_method(object, obj_ce, fn_proxy, function_name, strlen(function_name), retval, 0, NULL, NULL);
}

static crex_always_inline zval* crex_call_method_with_1_params(crex_object *object, crex_class_entry *obj_ce,
		crex_function **fn_proxy, const char *function_name, zval *retval, zval* arg1)
{
	return crex_call_method(object, obj_ce, fn_proxy, function_name, strlen(function_name), retval, 1, arg1, NULL);
}

static crex_always_inline zval* crex_call_method_with_2_params(crex_object *object, crex_class_entry *obj_ce,
		crex_function **fn_proxy, const char *function_name, zval *retval, zval* arg1, zval* arg2)
{
	return crex_call_method(object, obj_ce, fn_proxy, function_name, strlen(function_name), retval, 2, arg1, arg2);
}

CREX_API void crex_user_it_rewind(crex_object_iterator *_iter);
CREX_API crex_result crex_user_it_valid(crex_object_iterator *_iter);
CREX_API void crex_user_it_get_current_key(crex_object_iterator *_iter, zval *key);
CREX_API zval *crex_user_it_get_current_data(crex_object_iterator *_iter);
CREX_API void crex_user_it_move_forward(crex_object_iterator *_iter);
CREX_API void crex_user_it_invalidate_current(crex_object_iterator *_iter);
CREX_API HashTable *crex_user_it_get_gc(crex_object_iterator *_iter, zval **table, int *n);

CREX_API void crex_user_it_new_iterator(crex_class_entry *ce, zval *object, zval *iterator);
CREX_API crex_object_iterator *crex_user_it_get_new_iterator(crex_class_entry *ce, zval *object, int by_ref);

CREX_API void crex_register_interfaces(void);

CREX_API int crex_user_serialize(zval *object, unsigned char **buffer, size_t *buf_len, crex_serialize_data *data);
CREX_API int crex_user_unserialize(zval *object, crex_class_entry *ce, const unsigned char *buf, size_t buf_len, crex_unserialize_data *data);

CREX_API crex_result crex_create_internal_iterator_zval(zval *return_value, zval *obj);

END_EXTERN_C()

#endif /* CREX_INTERFACES_H */
