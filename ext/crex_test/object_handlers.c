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
  | Authors: George Peter Banyard <girgias@crx.net>                      |
  +----------------------------------------------------------------------+
*/

#include "object_handlers.h"
#include "crex_API.h"
#include "object_handlers_arginfo.h"

/* donc refers to DoOperationNoCast */
static crex_class_entry *donc_ce;
static crex_object_handlers donc_object_handlers;

static crex_object* donc_object_create_ex(crex_class_entry* ce, crex_long l) {
	crex_object *obj = crex_objects_new(ce);
	object_properties_init(obj, ce);
	obj->handlers = &donc_object_handlers;
	ZVAL_LONG(OBJ_PROP_NUM(obj, 0), l);
	return obj;
}
static crex_object *donc_object_create(crex_class_entry *ce) /* {{{ */
{
	return donc_object_create_ex(ce, 0);
}
/* }}} */

static inline void donc_create(zval *target, crex_long l) /* {{{ */
{
	ZVAL_OBJ(target, donc_object_create_ex(donc_ce, l));
}

#define IS_DONC(zval) \
	(C_TYPE_P(zval) == IS_OBJECT && instanceof_function(C_OBJCE_P(zval), donc_ce))

static void donc_add(zval *result, zval *op1, zval *op2)
{
	crex_long val_1;
	crex_long val_2;
	if (IS_DONC(op1)) {
		val_1 = C_LVAL_P(OBJ_PROP_NUM(C_OBJ_P(op1), 0));
	} else {
		val_1 = zval_get_long(op1);
	}
	if (IS_DONC(op2)) {
		val_2 = C_LVAL_P(OBJ_PROP_NUM(C_OBJ_P(op2), 0));
	} else {
		val_2 = zval_get_long(op2);
	}

	donc_create(result, val_1 + val_2);
}
static void donc_mul(zval *result, zval *op1, zval *op2)
{
	crex_long val_1;
	crex_long val_2;
	if (IS_DONC(op1)) {
		val_1 = C_LVAL_P(OBJ_PROP_NUM(C_OBJ_P(op1), 0));
	} else {
		val_1 = zval_get_long(op1);
	}
	if (IS_DONC(op2)) {
		val_2 = C_LVAL_P(OBJ_PROP_NUM(C_OBJ_P(op2), 0));
	} else {
		val_2 = zval_get_long(op2);
	}

	donc_create(result, val_1 * val_2);
}

static crex_result donc_do_operation(crex_uchar opcode, zval *result, zval *op1, zval *op2)
{
	zval op1_copy;
	crex_result status;

	if (result == op1) {
		ZVAL_COPY_VALUE(&op1_copy, op1);
		op1 = &op1_copy;
	}

	switch (opcode) {
		case CREX_ADD:
			donc_add(result, op1, op2);
			if (UNEXPECTED(EG(exception))) { status = FAILURE; }
			status = SUCCESS;
			break;
		case CREX_MUL:
			donc_mul(result, op1, op2);
			if (UNEXPECTED(EG(exception))) { status = FAILURE; }
			status = SUCCESS;
			break;
		default:
			status = FAILURE;
			break;
	}

	if (status == SUCCESS && op1 == &op1_copy) {
		zval_ptr_dtor(op1);
	}

	return status;
}

CREX_METHOD(DoOperationNoCast, __main)
{
	crex_long l;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(l)
	CREX_PARSE_PARAMETERS_END();

	ZVAL_LONG(OBJ_PROP_NUM(C_OBJ_P(CREX_THIS), 0), l);
}

static crex_class_entry *long_castable_no_operation_ce;
static crex_object_handlers long_castable_no_operation_object_handlers;

static crex_object* long_castable_no_operation_object_create_ex(crex_class_entry* ce, crex_long l) {
	crex_object *obj = crex_objects_new(ce);
	object_properties_init(obj, ce);
	obj->handlers = &long_castable_no_operation_object_handlers;
	ZVAL_LONG(OBJ_PROP_NUM(obj, 0), l);
	return obj;
}

static crex_object *long_castable_no_operation_object_create(crex_class_entry *ce)
{
	return long_castable_no_operation_object_create_ex(ce, 0);
}

static crex_result long_castable_no_operation_cast_object(crex_object *obj, zval *result, int type)
{
	if (type == IS_LONG) {
		ZVAL_COPY(result, OBJ_PROP_NUM(obj, 0));
		return SUCCESS;
	}
	return FAILURE;
}

CREX_METHOD(LongCastableNoOperations, __main)
{
	crex_long l;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(l)
	CREX_PARSE_PARAMETERS_END();

	ZVAL_LONG(OBJ_PROP_NUM(C_OBJ_P(CREX_THIS), 0), l);
}

static crex_class_entry *float_castable_no_operation_ce;
static crex_object_handlers float_castable_no_operation_object_handlers;

static crex_object* float_castable_no_operation_object_create_ex(crex_class_entry* ce, double d) {
	crex_object *obj = crex_objects_new(ce);
	object_properties_init(obj, ce);
	obj->handlers = &float_castable_no_operation_object_handlers;
	ZVAL_DOUBLE(OBJ_PROP_NUM(obj, 0), d);
	return obj;
}

static crex_object *float_castable_no_operation_object_create(crex_class_entry *ce)
{
	return float_castable_no_operation_object_create_ex(ce, 0.0);
}

static crex_result float_castable_no_operation_cast_object(crex_object *obj, zval *result, int type)
{
	if (type == IS_DOUBLE) {
		ZVAL_COPY(result, OBJ_PROP_NUM(obj, 0));
		return SUCCESS;
	}
	return FAILURE;
}

CREX_METHOD(FloatCastableNoOperations, __main)
{
	double d;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(d)
	CREX_PARSE_PARAMETERS_END();

	ZVAL_DOUBLE(OBJ_PROP_NUM(C_OBJ_P(CREX_THIS), 0), d);
}

static crex_class_entry *numeric_castable_no_operation_ce;
static crex_object_handlers numeric_castable_no_operation_object_handlers;

static crex_object* numeric_castable_no_operation_object_create_ex(crex_class_entry* ce, const zval *n) {
	crex_object *obj = crex_objects_new(ce);
	object_properties_init(obj, ce);
	obj->handlers = &numeric_castable_no_operation_object_handlers;
	ZVAL_COPY(OBJ_PROP_NUM(obj, 0), n);
	return obj;
}

static crex_object *numeric_castable_no_operation_object_create(crex_class_entry *ce)
{
	zval tmp;
	ZVAL_LONG(&tmp, 0);
	return numeric_castable_no_operation_object_create_ex(ce, &tmp);
}

static crex_result numeric_castable_no_operation_cast_object(crex_object *obj, zval *result, int type)
{
	if (type == _IS_NUMBER) {
		ZVAL_COPY(result, OBJ_PROP_NUM(obj, 0));
		return SUCCESS;
	}
	return FAILURE;
}

CREX_METHOD(NumericCastableNoOperations, __main)
{
	zval *n;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_NUMBER(n)
	CREX_PARSE_PARAMETERS_END();

	ZVAL_COPY(OBJ_PROP_NUM(C_OBJ_P(CREX_THIS), 0), n);
}

void crex_test_object_handlers_init(void)
{
	/* DoOperationNoCast class */
	donc_ce = register_class_DoOperationNoCast();
	donc_ce->create_object = donc_object_create;
	memcpy(&donc_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	donc_object_handlers.do_operation = donc_do_operation;

	/* CastableNoOperation classes */
	long_castable_no_operation_ce = register_class_LongCastableNoOperations();
	long_castable_no_operation_ce->create_object = long_castable_no_operation_object_create;
	memcpy(&long_castable_no_operation_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	long_castable_no_operation_object_handlers.cast_object = long_castable_no_operation_cast_object;

	float_castable_no_operation_ce = register_class_FloatCastableNoOperations();
	float_castable_no_operation_ce->create_object = float_castable_no_operation_object_create;
	memcpy(&float_castable_no_operation_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	float_castable_no_operation_object_handlers.cast_object = float_castable_no_operation_cast_object;

	numeric_castable_no_operation_ce = register_class_NumericCastableNoOperations();
	numeric_castable_no_operation_ce->create_object = numeric_castable_no_operation_object_create;
	memcpy(&numeric_castable_no_operation_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	numeric_castable_no_operation_object_handlers.cast_object = numeric_castable_no_operation_cast_object;
}
