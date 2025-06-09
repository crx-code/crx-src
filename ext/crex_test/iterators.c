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
*/

#include "iterators.h"
#include "crex_API.h"
#include "iterators_arginfo.h"

#include <crex_interfaces.h>
#include "crx.h"

#define DUMP(s) crx_output_write((s), sizeof((s)) - 1)

static crex_class_entry *traversable_test_ce;

// Dummy iterator that yields numbers from 0..4,
// while printing operations to the output buffer
typedef struct {
	crex_object_iterator intern;
	zval current;
} test_traversable_it;

static test_traversable_it *test_traversable_it_fetch(crex_object_iterator *iter) {
	return (test_traversable_it *)iter;
}

static void test_traversable_it_dtor(crex_object_iterator *iter) {
	DUMP("TraversableTest::drop\n");
	test_traversable_it *iterator = test_traversable_it_fetch(iter);
	zval_ptr_dtor(&iterator->intern.data);
}

static void test_traversable_it_rewind(crex_object_iterator *iter) {
	DUMP("TraversableTest::rewind\n");
	test_traversable_it *iterator = test_traversable_it_fetch(iter);
	ZVAL_LONG(&iterator->current, 0);
}

static void test_traversable_it_next(crex_object_iterator *iter) {
	DUMP("TraversableTest::next\n");
	test_traversable_it *iterator = test_traversable_it_fetch(iter);
	ZVAL_LONG(&iterator->current, C_LVAL(iterator->current) + 1);
}

static int test_traversable_it_valid(crex_object_iterator *iter) {
	DUMP("TraversableTest::valid\n");
	test_traversable_it *iterator = test_traversable_it_fetch(iter);
	if (C_LVAL(iterator->current) < 4) {
		return SUCCESS;
	}
	return FAILURE;
}

static void test_traversable_it_key(crex_object_iterator *iter, zval *return_value) {
	DUMP("TraversableTest::key\n");
	test_traversable_it *iterator = test_traversable_it_fetch(iter);
	ZVAL_LONG(return_value, C_LVAL(iterator->current));
}

static zval *test_traversable_it_current(crex_object_iterator *iter) {
	DUMP("TraversableTest::current\n");
	test_traversable_it *iterator = test_traversable_it_fetch(iter);
	return &iterator->current;
}

static const crex_object_iterator_funcs test_traversable_it_vtable = {
	test_traversable_it_dtor,
	test_traversable_it_valid,
	test_traversable_it_current,
	test_traversable_it_key,
	test_traversable_it_next,
	test_traversable_it_rewind,
	NULL, // invalidate_current
	NULL, // get_gc
};

static crex_object_iterator *test_traversable_get_iterator(
	crex_class_entry *ce,
	zval *object,
	int by_ref
) {
	test_traversable_it *iterator;

	if (by_ref) {
		crex_throw_error(NULL, "An iterator cannot be used with foreach by reference");
		return NULL;
	}

	iterator = emalloc(sizeof(test_traversable_it));
	crex_iterator_init((crex_object_iterator*)iterator);

	ZVAL_OBJ_COPY(&iterator->intern.data, C_OBJ_P(object));
	iterator->intern.funcs = &test_traversable_it_vtable;
	ZVAL_LONG(&iterator->current, 0);

	return (crex_object_iterator*)iterator;
}

CREX_METHOD(CrexTest_Iterators_TraversableTest, __main) {
	CREX_PARSE_PARAMETERS_NONE();
}

CREX_METHOD(CrexTest_Iterators_TraversableTest, getIterator) {
	CREX_PARSE_PARAMETERS_NONE();
    crex_create_internal_iterator_zval(return_value, CREX_THIS);
}

void crex_test_iterators_init(void) {
	traversable_test_ce = register_class_CrexTest_Iterators_TraversableTest(crex_ce_aggregate);
	traversable_test_ce->get_iterator = test_traversable_get_iterator;
}
