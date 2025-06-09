/*
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the CRX license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | https://www.crx.net/license/3_01.txt                                 |
   | If you did not receive a copy of the CRX license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crx.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Gustavo Lopes <cataphract@crx.net>                          |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unicode/brkiter.h>

#include "breakiterator_iterators.h"
#include "../common/common_enum.h"

extern "C" {
#define USE_BREAKITERATOR_POINTER
#include "breakiterator_class.h"
#include "breakiterator_iterators_arginfo.h"
#include "../intl_convert.h"
#include "../locale/locale.h"
#include <crex_exceptions.h>
#include <crex_interfaces.h>
}

static crex_class_entry *IntlPartsIterator_ce_ptr;

/* BreakIterator's iterator */

inline BreakIterator *_breakiter_prolog(crex_object_iterator *iter)
{
	BreakIterator_object *bio;
	bio = C_INTL_BREAKITERATOR_P(&iter->data);
	intl_errors_reset(BREAKITER_ERROR_P(bio));
	if (bio->biter == NULL) {
		intl_errors_set(BREAKITER_ERROR_P(bio), U_INVALID_STATE_ERROR,
			"The BreakIterator object backing the CRX iterator is not "
			"properly constructed", 0);
	}
	return bio->biter;
}

static void _breakiterator_destroy_it(crex_object_iterator *iter)
{
	zval_ptr_dtor(&iter->data);
}

static void _breakiterator_move_forward(crex_object_iterator *iter)
{
	BreakIterator *biter = _breakiter_prolog(iter);
	zoi_with_current *zoi_iter = (zoi_with_current*)iter;

	iter->funcs->invalidate_current(iter);

	if (biter == NULL) {
		return;
	}

	int32_t pos = biter->next();
	if (pos != BreakIterator::DONE) {
		ZVAL_LONG(&zoi_iter->current, (crex_long)pos);
	} //else we've reached the end of the enum, nothing more is required
}

static void _breakiterator_rewind(crex_object_iterator *iter)
{
	BreakIterator *biter = _breakiter_prolog(iter);
	zoi_with_current *zoi_iter = (zoi_with_current*)iter;

	int32_t pos = biter->first();
	ZVAL_LONG(&zoi_iter->current, (crex_long)pos);
}

static const crex_object_iterator_funcs breakiterator_iterator_funcs = {
	zoi_with_current_dtor,
	zoi_with_current_valid,
	zoi_with_current_get_current_data,
	NULL,
	_breakiterator_move_forward,
	_breakiterator_rewind,
	zoi_with_current_invalidate_current,
	NULL, /* get_gc */
};

U_CFUNC crex_object_iterator *_breakiterator_get_iterator(
	crex_class_entry *ce, zval *object, int by_ref)
{
	BreakIterator_object *bio;
	if (by_ref) {
		crex_throw_exception(NULL,
			"Iteration by reference is not supported", 0);
		return NULL;
	}

	bio = C_INTL_BREAKITERATOR_P(object);
	BreakIterator *biter = bio->biter;

	if (biter == NULL) {
		crex_throw_exception(NULL,
			"The BreakIterator is not properly constructed", 0);
		return NULL;
	}

	zoi_with_current *zoi_iter = static_cast<zoi_with_current*>(emalloc(sizeof *zoi_iter));
	crex_iterator_init(&zoi_iter->zoi);
	ZVAL_OBJ_COPY(&zoi_iter->zoi.data, C_OBJ_P(object));
	zoi_iter->zoi.funcs = &breakiterator_iterator_funcs;
	zoi_iter->zoi.index = 0;
	zoi_iter->destroy_it = _breakiterator_destroy_it;
	ZVAL_UNDEF(&zoi_iter->wrapping_obj); /* not used; object is in zoi.data */
	ZVAL_UNDEF(&zoi_iter->current);

	return reinterpret_cast<crex_object_iterator *>(zoi_iter);
}

/* BreakIterator parts iterator */

typedef struct zoi_break_iter_parts {
	zoi_with_current zoi_cur;
	parts_iter_key_type key_type;
	BreakIterator_object *bio; /* so we don't have to fetch it all the time */
	crex_ulong index_right;
} zoi_break_iter_parts;

static void _breakiterator_parts_destroy_it(crex_object_iterator *iter)
{
	zval_ptr_dtor(&iter->data);
}

static void _breakiterator_parts_get_current_key(crex_object_iterator *iter, zval *key)
{
	// The engine resets the iterator index to -1 after rewinding. When using
	// PARTS_ITERATOR_KEY_RIGHT we store it in zoi_break_iter_parts.index_right
	// so it doesn't get lost.
	zoi_break_iter_parts *zoi_bit = (zoi_break_iter_parts*)iter;

	if (zoi_bit->key_type == PARTS_ITERATOR_KEY_RIGHT && iter->index == 0) {
		ZVAL_LONG(key, zoi_bit->index_right);
	} else {
		ZVAL_LONG(key, iter->index);
	}
}

static void _breakiterator_parts_move_forward(crex_object_iterator *iter)
{
	zoi_break_iter_parts *zoi_bit = (zoi_break_iter_parts*)iter;
	BreakIterator_object *bio = zoi_bit->bio;

	iter->funcs->invalidate_current(iter);

	int32_t cur,
			next;

	cur = bio->biter->current();
	if (cur == BreakIterator::DONE) {
		return;
	}
	next = bio->biter->next();
	if (next == BreakIterator::DONE) {
		return;
	}

	if (zoi_bit->key_type == PARTS_ITERATOR_KEY_LEFT) {
		iter->index = cur;
	} else if (zoi_bit->key_type == PARTS_ITERATOR_KEY_RIGHT) {
		iter->index = next;
		zoi_bit->index_right = next;
	}
	/* else zoi_bit->key_type == PARTS_ITERATOR_KEY_SEQUENTIAL
	 * No need to do anything, the engine increments ->index */

	const char	*s = C_STRVAL(bio->text);
	crex_string	*res;

	assert(next <= C_STRLEN(bio->text) && next >= cur);
	res = crex_string_alloc(next - cur, 0);

	memcpy(ZSTR_VAL(res), &s[cur], ZSTR_LEN(res));
	ZSTR_VAL(res)[ZSTR_LEN(res)] = '\0';

	ZVAL_STR(&zoi_bit->zoi_cur.current, res);
}

static void _breakiterator_parts_rewind(crex_object_iterator *iter)
{
	zoi_break_iter_parts *zoi_bit = (zoi_break_iter_parts*)iter;
	BreakIterator_object *bio = zoi_bit->bio;

	if (!C_ISUNDEF(zoi_bit->zoi_cur.current)) {
		iter->funcs->invalidate_current(iter);
	}

	bio->biter->first();

	iter->funcs->move_forward(iter);
}

static const crex_object_iterator_funcs breakiterator_parts_it_funcs = {
	zoi_with_current_dtor,
	zoi_with_current_valid,
	zoi_with_current_get_current_data,
	_breakiterator_parts_get_current_key,
	_breakiterator_parts_move_forward,
	_breakiterator_parts_rewind,
	zoi_with_current_invalidate_current,
	NULL, /* get_gc */
};

void IntlIterator_from_BreakIterator_parts(zval *break_iter_zv,
										   zval *object,
										   parts_iter_key_type key_type)
{
	IntlIterator_object *ii;

	object_init_ex(object, IntlPartsIterator_ce_ptr);
	ii = C_INTL_ITERATOR_P(object);

	ii->iterator = (crex_object_iterator*)emalloc(sizeof(zoi_break_iter_parts));
	crex_iterator_init(ii->iterator);

	ZVAL_COPY(&ii->iterator->data, break_iter_zv);
	ii->iterator->funcs = &breakiterator_parts_it_funcs;
	ii->iterator->index = 0;

	((zoi_with_current*)ii->iterator)->destroy_it = _breakiterator_parts_destroy_it;
	ZVAL_OBJ(&((zoi_with_current*)ii->iterator)->wrapping_obj, C_OBJ_P(object));
	ZVAL_UNDEF(&((zoi_with_current*)ii->iterator)->current);

	((zoi_break_iter_parts*)ii->iterator)->bio = C_INTL_BREAKITERATOR_P(break_iter_zv);

	assert(((zoi_break_iter_parts*)ii->iterator)->bio->biter != NULL);

	((zoi_break_iter_parts*)ii->iterator)->key_type = key_type;
	((zoi_break_iter_parts*)ii->iterator)->index_right = 0;
}

U_CFUNC CRX_METHOD(IntlPartsIterator, getBreakIterator)
{
	INTLITERATOR_METHOD_INIT_VARS;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	INTLITERATOR_METHOD_FETCH_OBJECT;

	RETURN_COPY_DEREF(&ii->iterator->data);
}

U_CFUNC CRX_METHOD(IntlPartsIterator, getRuleStatus)
{
	INTLITERATOR_METHOD_INIT_VARS;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	INTLITERATOR_METHOD_FETCH_OBJECT;

	zval *iter = &ii->iterator->data;
	CREX_ASSERT(C_TYPE_P(iter) == IS_OBJECT);
	crex_call_method_with_0_params(
			C_OBJ_P(iter), C_OBJCE_P(iter), NULL, "getrulestatus", return_value);
}

U_CFUNC void breakiterator_register_IntlPartsIterator_class(void)
{
	/* Create and register 'BreakIterator' class. */
	IntlPartsIterator_ce_ptr = register_class_IntlPartsIterator(IntlIterator_ce_ptr);
}
