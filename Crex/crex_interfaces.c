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

#include "crex.h"
#include "crex_API.h"
#include "crex_interfaces.h"
#include "crex_exceptions.h"
#include "crex_interfaces_arginfo.h"

CREX_API crex_class_entry *crex_ce_traversable;
CREX_API crex_class_entry *crex_ce_aggregate;
CREX_API crex_class_entry *crex_ce_iterator;
CREX_API crex_class_entry *crex_ce_arrayaccess;
CREX_API crex_class_entry *crex_ce_serializable;
CREX_API crex_class_entry *crex_ce_countable;
CREX_API crex_class_entry *crex_ce_stringable;
CREX_API crex_class_entry *crex_ce_internal_iterator;

static crex_object_handlers crex_internal_iterator_handlers;

/* {{{ crex_call_method
 Only returns the returned zval if retval_ptr != NULL */
CREX_API zval* crex_call_method(crex_object *object, crex_class_entry *obj_ce, crex_function **fn_proxy, const char *function_name, size_t function_name_len, zval *retval_ptr, uint32_t param_count, zval* arg1, zval* arg2)
{
	crex_function *fn;
	crex_class_entry *called_scope;
	zval params[2];

	if (param_count > 0) {
		ZVAL_COPY_VALUE(&params[0], arg1);
	}
	if (param_count > 1) {
		ZVAL_COPY_VALUE(&params[1], arg2);
	}

	if (!obj_ce) {
		obj_ce = object ? object->ce : NULL;
	}
	if (!fn_proxy || !*fn_proxy) {
		if (EXPECTED(obj_ce)) {
			fn = crex_hash_str_find_ptr_lc(
				&obj_ce->function_table, function_name, function_name_len);
			if (UNEXPECTED(fn == NULL)) {
				/* error at c-level */
				crex_error_noreturn(E_CORE_ERROR, "Couldn't find implementation for method %s::%s", ZSTR_VAL(obj_ce->name), function_name);
			}
		} else {
			fn = crex_fetch_function_str(function_name, function_name_len);
			if (UNEXPECTED(fn == NULL)) {
				/* error at c-level */
				crex_error_noreturn(E_CORE_ERROR, "Couldn't find implementation for function %s", function_name);
			}
		}
		if (fn_proxy) {
			*fn_proxy = fn;
		}
	} else {
		fn = *fn_proxy;
	}

	if (object) {
		called_scope = object->ce;
	} else {
		called_scope = obj_ce;
	}

	crex_call_known_function(fn, object, called_scope, retval_ptr, param_count, params, NULL);
	return retval_ptr;
}
/* }}} */

/* iterator interface, c-level functions used by engine */

/* {{{ crex_user_it_new_iterator */
CREX_API void crex_user_it_new_iterator(crex_class_entry *ce, zval *object, zval *retval)
{
	crex_call_known_instance_method_with_0_params(
		ce->iterator_funcs_ptr->zf_new_iterator, C_OBJ_P(object), retval);
}
/* }}} */

/* {{{ crex_user_it_invalidate_current */
CREX_API void crex_user_it_invalidate_current(crex_object_iterator *_iter)
{
	crex_user_iterator *iter = (crex_user_iterator*)_iter;

	if (!C_ISUNDEF(iter->value)) {
		zval_ptr_dtor(&iter->value);
		ZVAL_UNDEF(&iter->value);
	}
}
/* }}} */

/* {{{ crex_user_it_dtor */
static void crex_user_it_dtor(crex_object_iterator *_iter)
{
	crex_user_iterator *iter = (crex_user_iterator*)_iter;
	zval *object = &iter->it.data;

	crex_user_it_invalidate_current(_iter);
	zval_ptr_dtor(object);
}
/* }}} */

/* {{{ crex_user_it_valid */
CREX_API crex_result crex_user_it_valid(crex_object_iterator *_iter)
{
	if (_iter) {
		crex_user_iterator *iter = (crex_user_iterator*)_iter;
		zval *object = &iter->it.data;
		zval more;

		crex_call_known_instance_method_with_0_params(iter->ce->iterator_funcs_ptr->zf_valid, C_OBJ_P(object), &more);
		bool result = i_crex_is_true(&more);
		zval_ptr_dtor(&more);
		return result ? SUCCESS : FAILURE;
	}
	return FAILURE;
}
/* }}} */

/* {{{ crex_user_it_get_current_data */
CREX_API zval *crex_user_it_get_current_data(crex_object_iterator *_iter)
{
	crex_user_iterator *iter = (crex_user_iterator*)_iter;
	zval *object = &iter->it.data;

	if (C_ISUNDEF(iter->value)) {
		crex_call_known_instance_method_with_0_params(iter->ce->iterator_funcs_ptr->zf_current, C_OBJ_P(object), &iter->value);
	}
	return &iter->value;
}
/* }}} */

/* {{{ crex_user_it_get_current_key */
CREX_API void crex_user_it_get_current_key(crex_object_iterator *_iter, zval *key)
{
	crex_user_iterator *iter = (crex_user_iterator*)_iter;
	zval *object = &iter->it.data;
	crex_call_known_instance_method_with_0_params(iter->ce->iterator_funcs_ptr->zf_key, C_OBJ_P(object), key);
	if (UNEXPECTED(C_ISREF_P(key))) {
		crex_unwrap_reference(key);
	}
}
/* }}} */

/* {{{ crex_user_it_move_forward */
CREX_API void crex_user_it_move_forward(crex_object_iterator *_iter)
{
	crex_user_iterator *iter = (crex_user_iterator*)_iter;
	zval *object = &iter->it.data;

	crex_user_it_invalidate_current(_iter);
	crex_call_known_instance_method_with_0_params(iter->ce->iterator_funcs_ptr->zf_next, C_OBJ_P(object), NULL);
}
/* }}} */

/* {{{ crex_user_it_rewind */
CREX_API void crex_user_it_rewind(crex_object_iterator *_iter)
{
	crex_user_iterator *iter = (crex_user_iterator*)_iter;
	zval *object = &iter->it.data;

	crex_user_it_invalidate_current(_iter);
	crex_call_known_instance_method_with_0_params(iter->ce->iterator_funcs_ptr->zf_rewind, C_OBJ_P(object), NULL);
}
/* }}} */

CREX_API HashTable *crex_user_it_get_gc(crex_object_iterator *_iter, zval **table, int *n)
{
	crex_user_iterator *iter = (crex_user_iterator*)_iter;
	if (C_ISUNDEF(iter->value)) {
		*table = &iter->it.data;
		*n = 1;
	} else {
		crex_get_gc_buffer *gc_buffer = crex_get_gc_buffer_create();
		crex_get_gc_buffer_add_zval(gc_buffer, &iter->it.data);
		crex_get_gc_buffer_add_zval(gc_buffer, &iter->value);
		crex_get_gc_buffer_use(gc_buffer, table, n);
	}
	return NULL;
}

static const crex_object_iterator_funcs crex_interface_iterator_funcs_iterator = {
	crex_user_it_dtor,
	// FIXME: Adjust the actual function prototype in crex_object_iterator_funcs
	(int (*)(crex_object_iterator *)) crex_user_it_valid,
	crex_user_it_get_current_data,
	crex_user_it_get_current_key,
	crex_user_it_move_forward,
	crex_user_it_rewind,
	crex_user_it_invalidate_current,
	crex_user_it_get_gc,
};

/* {{{ crex_user_it_get_iterator */
/* by_ref is int due to Iterator API */
static crex_object_iterator *crex_user_it_get_iterator(crex_class_entry *ce, zval *object, int by_ref)
{
	crex_user_iterator *iterator;

	if (by_ref) {
		crex_throw_error(NULL, "An iterator cannot be used with foreach by reference");
		return NULL;
	}

	iterator = emalloc(sizeof(crex_user_iterator));

	crex_iterator_init((crex_object_iterator*)iterator);

	ZVAL_OBJ_COPY(&iterator->it.data, C_OBJ_P(object));
	iterator->it.funcs = &crex_interface_iterator_funcs_iterator;
	iterator->ce = C_OBJCE_P(object);
	ZVAL_UNDEF(&iterator->value);
	return (crex_object_iterator*)iterator;
}
/* }}} */

/* {{{ crex_user_it_get_new_iterator */
/* by_ref is int due to Iterator API */
CREX_API crex_object_iterator *crex_user_it_get_new_iterator(crex_class_entry *ce, zval *object, int by_ref)
{
	zval iterator;
	crex_object_iterator *new_iterator;
	crex_class_entry *ce_it;

	crex_user_it_new_iterator(ce, object, &iterator);
	ce_it = (C_TYPE(iterator) == IS_OBJECT) ? C_OBJCE(iterator) : NULL;

	if (!ce_it || !ce_it->get_iterator || (ce_it->get_iterator == crex_user_it_get_new_iterator && C_OBJ(iterator) == C_OBJ_P(object))) {
		if (!EG(exception)) {
			crex_throw_exception_ex(NULL, 0, "Objects returned by %s::getIterator() must be traversable or implement interface Iterator", ce ? ZSTR_VAL(ce->name) : ZSTR_VAL(C_OBJCE_P(object)->name));
		}
		zval_ptr_dtor(&iterator);
		return NULL;
	}

	new_iterator = ce_it->get_iterator(ce_it, &iterator, by_ref);
	zval_ptr_dtor(&iterator);
	return new_iterator;
}
/* }}} */

/* {{{ crex_implement_traversable */
static int crex_implement_traversable(crex_class_entry *interface, crex_class_entry *class_type)
{
	/* Abstract class can implement Traversable only, in which case the extending class must
	 * implement Iterator or IteratorAggregate. */
	if (class_type->ce_flags & CREX_ACC_EXPLICIT_ABSTRACT_CLASS) {
		return SUCCESS;
	}

	/* Check that class_type implements at least one of 'IteratorAggregate' or 'Iterator' */
	if (class_type->num_interfaces) {
		CREX_ASSERT(class_type->ce_flags & CREX_ACC_RESOLVED_INTERFACES);
		for (uint32_t i = 0; i < class_type->num_interfaces; i++) {
			if (class_type->interfaces[i] == crex_ce_aggregate || class_type->interfaces[i] == crex_ce_iterator) {
				return SUCCESS;
			}
		}
	}
	crex_error_noreturn(E_CORE_ERROR, "%s %s must implement interface %s as part of either %s or %s",
		crex_get_object_type_uc(class_type),
		ZSTR_VAL(class_type->name),
		ZSTR_VAL(crex_ce_traversable->name),
		ZSTR_VAL(crex_ce_iterator->name),
		ZSTR_VAL(crex_ce_aggregate->name));
	return FAILURE;
}
/* }}} */

/* {{{ crex_implement_aggregate */
static int crex_implement_aggregate(crex_class_entry *interface, crex_class_entry *class_type)
{
	if (crex_class_implements_interface(class_type, crex_ce_iterator)) {
		crex_error_noreturn(E_ERROR,
			"Class %s cannot implement both Iterator and IteratorAggregate at the same time",
			ZSTR_VAL(class_type->name));
	}

	/* Always initialize iterator_funcs_ptr. */
	CREX_ASSERT(!class_type->iterator_funcs_ptr && "Iterator funcs already set?");
	crex_class_iterator_funcs *funcs_ptr = class_type->type == CREX_INTERNAL_CLASS
		? pemalloc(sizeof(crex_class_iterator_funcs), 1)
		: crex_arena_alloc(&CG(arena), sizeof(crex_class_iterator_funcs));
	class_type->iterator_funcs_ptr = funcs_ptr;

	memset(funcs_ptr, 0, sizeof(crex_class_iterator_funcs));
	funcs_ptr->zf_new_iterator = crex_hash_str_find_ptr(
		&class_type->function_table, "getiterator", sizeof("getiterator") - 1);

	if (class_type->get_iterator && class_type->get_iterator != crex_user_it_get_new_iterator) {
		/* get_iterator was explicitly assigned for an internal class. */
		if (!class_type->parent || class_type->parent->get_iterator != class_type->get_iterator) {
			CREX_ASSERT(class_type->type == CREX_INTERNAL_CLASS);
			return SUCCESS;
		}

		/* The getIterator() method has not been overwritten, use inherited get_iterator(). */
		if (funcs_ptr->zf_new_iterator->common.scope != class_type) {
			return SUCCESS;
		}

		/* getIterator() has been overwritten, switch to crex_user_it_get_new_iterator. */
	}

	class_type->get_iterator = crex_user_it_get_new_iterator;
	return SUCCESS;
}
/* }}} */

/* {{{ crex_implement_iterator */
static int crex_implement_iterator(crex_class_entry *interface, crex_class_entry *class_type)
{
	if (crex_class_implements_interface(class_type, crex_ce_aggregate)) {
		crex_error_noreturn(E_ERROR,
			"Class %s cannot implement both Iterator and IteratorAggregate at the same time",
			ZSTR_VAL(class_type->name));
	}

	CREX_ASSERT(!class_type->iterator_funcs_ptr && "Iterator funcs already set?");
	crex_class_iterator_funcs *funcs_ptr = class_type->type == CREX_INTERNAL_CLASS
		? pemalloc(sizeof(crex_class_iterator_funcs), 1)
		: crex_arena_alloc(&CG(arena), sizeof(crex_class_iterator_funcs));
	class_type->iterator_funcs_ptr = funcs_ptr;

	memset(funcs_ptr, 0, sizeof(crex_class_iterator_funcs));
	funcs_ptr->zf_rewind = crex_hash_str_find_ptr(
		&class_type->function_table, "rewind", sizeof("rewind") - 1);
	funcs_ptr->zf_valid = crex_hash_str_find_ptr(
		&class_type->function_table, "valid", sizeof("valid") - 1);
	funcs_ptr->zf_key = crex_hash_find_ptr(
		&class_type->function_table, ZSTR_KNOWN(CREX_STR_KEY));
	funcs_ptr->zf_current = crex_hash_str_find_ptr(
		&class_type->function_table, "current", sizeof("current") - 1);
	funcs_ptr->zf_next = crex_hash_str_find_ptr(
		&class_type->function_table, "next", sizeof("next") - 1);

	if (class_type->get_iterator && class_type->get_iterator != crex_user_it_get_iterator) {
		if (!class_type->parent || class_type->parent->get_iterator != class_type->get_iterator) {
			/* get_iterator was explicitly assigned for an internal class. */
			CREX_ASSERT(class_type->type == CREX_INTERNAL_CLASS);
			return SUCCESS;
		}

		/* None of the Iterator methods have been overwritten, use inherited get_iterator(). */
		if (funcs_ptr->zf_rewind->common.scope != class_type &&
				funcs_ptr->zf_valid->common.scope != class_type &&
				funcs_ptr->zf_key->common.scope != class_type &&
				funcs_ptr->zf_current->common.scope != class_type &&
				funcs_ptr->zf_next->common.scope != class_type) {
			return SUCCESS;
		}

		/* One of the Iterator methods has been overwritten,
		 * switch to crex_user_it_get_iterator. */
	}

	class_type->get_iterator = crex_user_it_get_iterator;
	return SUCCESS;
}
/* }}} */

/* {{{ crex_implement_arrayaccess */
static int crex_implement_arrayaccess(crex_class_entry *interface, crex_class_entry *class_type)
{
	CREX_ASSERT(!class_type->arrayaccess_funcs_ptr && "ArrayAccess funcs already set?");
	crex_class_arrayaccess_funcs *funcs_ptr = class_type->type == CREX_INTERNAL_CLASS
		? pemalloc(sizeof(crex_class_arrayaccess_funcs), 1)
		: crex_arena_alloc(&CG(arena), sizeof(crex_class_arrayaccess_funcs));
	class_type->arrayaccess_funcs_ptr = funcs_ptr;

	funcs_ptr->zf_offsetget = crex_hash_str_find_ptr(
		&class_type->function_table, "offsetget", sizeof("offsetget") - 1);
	funcs_ptr->zf_offsetexists = crex_hash_str_find_ptr(
		&class_type->function_table, "offsetexists", sizeof("offsetexists") - 1);
	funcs_ptr->zf_offsetset = crex_hash_str_find_ptr(
		&class_type->function_table, "offsetset", sizeof("offsetset") - 1);
	funcs_ptr->zf_offsetunset = crex_hash_str_find_ptr(
		&class_type->function_table, "offsetunset", sizeof("offsetunset") - 1);

	return SUCCESS;
}
/* }}} */

/* {{{ crex_user_serialize */
CREX_API int crex_user_serialize(zval *object, unsigned char **buffer, size_t *buf_len, crex_serialize_data *data)
{
	crex_class_entry * ce = C_OBJCE_P(object);
	zval retval;
	crex_result result;

	crex_call_method_with_0_params(
		C_OBJ_P(object), C_OBJCE_P(object), NULL, "serialize", &retval);

	if (C_TYPE(retval) == IS_UNDEF || EG(exception)) {
		result = FAILURE;
	} else {
		switch(C_TYPE(retval)) {
		case IS_NULL:
			/* we could also make this '*buf_len = 0' but this allows to skip variables */
			zval_ptr_dtor(&retval);
			return FAILURE;
		case IS_STRING:
			*buffer = (unsigned char*)estrndup(C_STRVAL(retval), C_STRLEN(retval));
			*buf_len = C_STRLEN(retval);
			result = SUCCESS;
			break;
		default: /* failure */
			result = FAILURE;
			break;
		}
		zval_ptr_dtor(&retval);
	}

	if (result == FAILURE && !EG(exception)) {
		crex_throw_exception_ex(NULL, 0, "%s::serialize() must return a string or NULL", ZSTR_VAL(ce->name));
	}
	return result;
}
/* }}} */

/* {{{ crex_user_unserialize */
CREX_API int crex_user_unserialize(zval *object, crex_class_entry *ce, const unsigned char *buf, size_t buf_len, crex_unserialize_data *data)
{
	zval zdata;

	if (UNEXPECTED(object_init_ex(object, ce) != SUCCESS)) {
		return FAILURE;
	}

	ZVAL_STRINGL(&zdata, (char*)buf, buf_len);
	crex_call_method_with_1_params(
		C_OBJ_P(object), C_OBJCE_P(object), NULL, "unserialize", NULL, &zdata);
	zval_ptr_dtor(&zdata);

	if (EG(exception)) {
		return FAILURE;
	} else {
		return SUCCESS;
	}
}
/* }}} */

/* {{{ crex_implement_serializable */
static int crex_implement_serializable(crex_class_entry *interface, crex_class_entry *class_type)
{
	if (class_type->parent
		&& (class_type->parent->serialize || class_type->parent->unserialize)
		&& !crex_class_implements_interface(class_type->parent, crex_ce_serializable)) {
		return FAILURE;
	}
	if (!class_type->serialize) {
		class_type->serialize = crex_user_serialize;
	}
	if (!class_type->unserialize) {
		class_type->unserialize = crex_user_unserialize;
	}
	if (!(class_type->ce_flags & CREX_ACC_EXPLICIT_ABSTRACT_CLASS)
			&& (!class_type->__serialize || !class_type->__unserialize)) {
		crex_error(E_DEPRECATED, "%s implements the Serializable interface, which is deprecated. Implement __serialize() and __unserialize() instead (or in addition, if support for old CRX versions is necessary)", ZSTR_VAL(class_type->name));
	}
	return SUCCESS;
}
/* }}}*/

typedef struct {
	crex_object std;
	crex_object_iterator *iter;
	bool rewind_called;
} crex_internal_iterator;

static crex_object *crex_internal_iterator_create(crex_class_entry *ce) {
	crex_internal_iterator *intern = emalloc(sizeof(crex_internal_iterator));
	crex_object_std_init(&intern->std, ce);
	intern->iter = NULL;
	intern->rewind_called = 0;
	return &intern->std;
}

CREX_API crex_result crex_create_internal_iterator_zval(zval *return_value, zval *obj) {
	crex_class_entry *scope = EG(current_execute_data)->func->common.scope;
	CREX_ASSERT(scope->get_iterator != crex_user_it_get_new_iterator);
	crex_object_iterator *iter = scope->get_iterator(C_OBJCE_P(obj), obj, /* by_ref */ 0);
	if (!iter) {
		return FAILURE;
	}

	crex_internal_iterator *intern =
		(crex_internal_iterator *) crex_internal_iterator_create(crex_ce_internal_iterator);
	intern->iter = iter;
	intern->iter->index = 0;
	ZVAL_OBJ(return_value, &intern->std);
	return SUCCESS;
}

static void crex_internal_iterator_free(crex_object *obj) {
	crex_internal_iterator *intern = (crex_internal_iterator *) obj;
	if (intern->iter) {
		crex_iterator_dtor(intern->iter);
	}
	crex_object_std_dtor(&intern->std);
}

static crex_internal_iterator *crex_internal_iterator_fetch(zval *This) {
	crex_internal_iterator *intern = (crex_internal_iterator *) C_OBJ_P(This);
	if (!intern->iter) {
		crex_throw_error(NULL, "The InternalIterator object has not been properly initialized");
		return NULL;
	}
	return intern;
}

/* Many iterators will not behave correctly if rewind() is not called, make sure it happens. */
static crex_result crex_internal_iterator_ensure_rewound(crex_internal_iterator *intern) {
	if (!intern->rewind_called) {
		crex_object_iterator *iter = intern->iter;
		intern->rewind_called = 1;
		if (iter->funcs->rewind) {
			iter->funcs->rewind(iter);
			if (UNEXPECTED(EG(exception))) {
				return FAILURE;
			}
		}
	}
	return SUCCESS;
}


CREX_METHOD(InternalIterator, __main) {
	crex_throw_error(NULL, "Cannot manually construct InternalIterator");
}

CREX_METHOD(InternalIterator, current) {
	CREX_PARSE_PARAMETERS_NONE();

	crex_internal_iterator *intern = crex_internal_iterator_fetch(CREX_THIS);
	if (!intern) {
		RETURN_THROWS();
	}

	if (crex_internal_iterator_ensure_rewound(intern) == FAILURE) {
		RETURN_THROWS();
	}

	zval *data = intern->iter->funcs->get_current_data(intern->iter);
	if (data) {
		RETURN_COPY_DEREF(data);
	}
}

CREX_METHOD(InternalIterator, key) {
	CREX_PARSE_PARAMETERS_NONE();

	crex_internal_iterator *intern = crex_internal_iterator_fetch(CREX_THIS);
	if (!intern) {
		RETURN_THROWS();
	}

	if (crex_internal_iterator_ensure_rewound(intern) == FAILURE) {
		RETURN_THROWS();
	}

	if (intern->iter->funcs->get_current_key) {
		intern->iter->funcs->get_current_key(intern->iter, return_value);
	} else {
		RETURN_LONG(intern->iter->index);
	}
}

CREX_METHOD(InternalIterator, next) {
	CREX_PARSE_PARAMETERS_NONE();

	crex_internal_iterator *intern = crex_internal_iterator_fetch(CREX_THIS);
	if (!intern) {
		RETURN_THROWS();
	}

	if (crex_internal_iterator_ensure_rewound(intern) == FAILURE) {
		RETURN_THROWS();
	}

	/* Advance index first to match foreach behavior. */
	intern->iter->index++;
	intern->iter->funcs->move_forward(intern->iter);
}

CREX_METHOD(InternalIterator, valid) {
	CREX_PARSE_PARAMETERS_NONE();

	crex_internal_iterator *intern = crex_internal_iterator_fetch(CREX_THIS);
	if (!intern) {
		RETURN_THROWS();
	}

	if (crex_internal_iterator_ensure_rewound(intern) == FAILURE) {
		RETURN_THROWS();
	}

	RETURN_BOOL(intern->iter->funcs->valid(intern->iter) == SUCCESS);
}

CREX_METHOD(InternalIterator, rewind) {
	CREX_PARSE_PARAMETERS_NONE();

	crex_internal_iterator *intern = crex_internal_iterator_fetch(CREX_THIS);
	if (!intern) {
		RETURN_THROWS();
	}

	intern->rewind_called = 1;
	if (!intern->iter->funcs->rewind) {
		/* Allow calling rewind() if no iteration has happened yet,
		 * even if the iterator does not support rewinding. */
		if (intern->iter->index != 0) {
			crex_throw_error(NULL, "Iterator does not support rewinding");
			RETURN_THROWS();
		}
		intern->iter->index = 0;
		return;
	}

	intern->iter->funcs->rewind(intern->iter);
	intern->iter->index = 0;
}

/* {{{ crex_register_interfaces */
CREX_API void crex_register_interfaces(void)
{
	crex_ce_traversable = register_class_Traversable();
	crex_ce_traversable->interface_gets_implemented = crex_implement_traversable;

	crex_ce_aggregate = register_class_IteratorAggregate(crex_ce_traversable);
	crex_ce_aggregate->interface_gets_implemented = crex_implement_aggregate;

	crex_ce_iterator = register_class_Iterator(crex_ce_traversable);
	crex_ce_iterator->interface_gets_implemented = crex_implement_iterator;

	crex_ce_serializable = register_class_Serializable();
	crex_ce_serializable->interface_gets_implemented = crex_implement_serializable;

	crex_ce_arrayaccess = register_class_ArrayAccess();
	crex_ce_arrayaccess->interface_gets_implemented = crex_implement_arrayaccess;

	crex_ce_countable = register_class_Countable();

	crex_ce_stringable = register_class_Stringable();

	crex_ce_internal_iterator = register_class_InternalIterator(crex_ce_iterator);
	crex_ce_internal_iterator->create_object = crex_internal_iterator_create;
	crex_ce_internal_iterator->default_object_handlers = &crex_internal_iterator_handlers;

	memcpy(&crex_internal_iterator_handlers, crex_get_std_object_handlers(),
		sizeof(crex_object_handlers));
	crex_internal_iterator_handlers.free_obj = crex_internal_iterator_free;
}
/* }}} */
