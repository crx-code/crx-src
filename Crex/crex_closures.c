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
   | Authors: Christian Seiler <chris_se@gmx.net>                         |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   |          Marcus Boerger <helly@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#include "crex.h"
#include "crex_API.h"
#include "crex_closures.h"
#include "crex_exceptions.h"
#include "crex_interfaces.h"
#include "crex_objects.h"
#include "crex_objects_API.h"
#include "crex_globals.h"
#include "crex_closures_arginfo.h"

typedef struct _crex_closure {
	crex_object       std;
	crex_function     func;
	zval              this_ptr;
	crex_class_entry *called_scope;
	zif_handler       orig_internal_handler;
} crex_closure;

/* non-static since it needs to be referenced */
CREX_API crex_class_entry *crex_ce_closure;
static crex_object_handlers closure_handlers;

CREX_METHOD(Closure, __invoke) /* {{{ */
{
	crex_function *func = EX(func);
	zval *args;
	uint32_t num_args;
	HashTable *named_args;

	CREX_PARSE_PARAMETERS_START(0, -1)
		C_PARAM_VARIADIC_WITH_NAMED(args, num_args, named_args)
	CREX_PARSE_PARAMETERS_END();

	if (call_user_function_named(CG(function_table), NULL, CREX_THIS, return_value, num_args, args, named_args) == FAILURE) {
		RETVAL_FALSE;
	}

	/* destruct the function also, then - we have allocated it in get_method */
	crex_string_release_ex(func->internal_function.function_name, 0);
	efree(func);

	/* Set the func pointer to NULL. Prior to CRX 8.3, this was only done for debug builds,
	 * because debug builds check certain properties after the call and needed to know this
	 * had been freed.
	 * However, extensions can proxy crex_execute_internal, and it's a bit surprising to have
	 * an invalid func pointer sitting on there, so this was changed in CRX 8.3.
	 */
	execute_data->func = NULL;
}
/* }}} */

static bool crex_valid_closure_binding(
		crex_closure *closure, zval *newthis, crex_class_entry *scope) /* {{{ */
{
	crex_function *func = &closure->func;
	bool is_fake_closure = (func->common.fn_flags & CREX_ACC_FAKE_CLOSURE) != 0;
	if (newthis) {
		if (func->common.fn_flags & CREX_ACC_STATIC) {
			crex_error(E_WARNING, "Cannot bind an instance to a static closure");
			return 0;
		}

		if (is_fake_closure && func->common.scope &&
				!instanceof_function(C_OBJCE_P(newthis), func->common.scope)) {
			/* Binding incompatible $this to an internal method is not supported. */
			crex_error(E_WARNING, "Cannot bind method %s::%s() to object of class %s",
					ZSTR_VAL(func->common.scope->name),
					ZSTR_VAL(func->common.function_name),
					ZSTR_VAL(C_OBJCE_P(newthis)->name));
			return 0;
		}
	} else if (is_fake_closure && func->common.scope
			&& !(func->common.fn_flags & CREX_ACC_STATIC)) {
		crex_error(E_WARNING, "Cannot unbind $this of method");
		return 0;
	} else if (!is_fake_closure && !C_ISUNDEF(closure->this_ptr)
			&& (func->common.fn_flags & CREX_ACC_USES_THIS)) {
		crex_error(E_WARNING, "Cannot unbind $this of closure using $this");
		return 0;
	}

	if (scope && scope != func->common.scope && scope->type == CREX_INTERNAL_CLASS) {
		/* rebinding to internal class is not allowed */
		crex_error(E_WARNING, "Cannot bind closure to scope of internal class %s",
				ZSTR_VAL(scope->name));
		return 0;
	}

	if (is_fake_closure && scope != func->common.scope) {
		if (func->common.scope == NULL) {
			crex_error(E_WARNING, "Cannot rebind scope of closure created from function");
		} else {
			crex_error(E_WARNING, "Cannot rebind scope of closure created from method");
		}
		return 0;
	}

	return 1;
}
/* }}} */

/* {{{ Call closure, binding to a given object with its class as the scope */
CREX_METHOD(Closure, call)
{
	zval *newthis, closure_result;
	crex_closure *closure;
	crex_fcall_info fci;
	crex_fcall_info_cache fci_cache;
	crex_object *newobj;
	crex_class_entry *newclass;

	fci.param_count = 0;
	fci.params = NULL;

	CREX_PARSE_PARAMETERS_START(1, -1)
		C_PARAM_OBJECT(newthis)
		C_PARAM_VARIADIC_WITH_NAMED(fci.params, fci.param_count, fci.named_params)
	CREX_PARSE_PARAMETERS_END();

	closure = (crex_closure *) C_OBJ_P(CREX_THIS);

	newobj = C_OBJ_P(newthis);
	newclass = newobj->ce;

	if (!crex_valid_closure_binding(closure, newthis, newclass)) {
		return;
	}

	fci_cache.called_scope = newclass;
	fci_cache.object = fci.object = newobj;

	fci.size = sizeof(fci);
	ZVAL_OBJ(&fci.function_name, &closure->std);
	ZVAL_UNDEF(&closure_result);
	fci.retval = &closure_result;

	if (closure->func.common.fn_flags & CREX_ACC_GENERATOR) {
		zval new_closure;
		crex_create_closure(&new_closure, &closure->func, newclass, closure->called_scope, newthis);
		closure = (crex_closure *) C_OBJ(new_closure);
		fci_cache.function_handler = &closure->func;

		crex_call_function(&fci, &fci_cache);

		/* copied upon generator creation */
		GC_DELREF(&closure->std);
	} else {
		crex_closure *fake_closure;
		crex_function *my_function;

		fake_closure = emalloc(sizeof(crex_closure));
		memset(&fake_closure->std, 0, sizeof(fake_closure->std));
		fake_closure->std.gc.refcount = 1;
		fake_closure->std.gc.u.type_info = GC_NULL;
		ZVAL_UNDEF(&fake_closure->this_ptr);
		fake_closure->called_scope = NULL;
		my_function = &fake_closure->func;
		if (CREX_USER_CODE(closure->func.type)) {
			memcpy(my_function, &closure->func, sizeof(crex_op_array));
		} else {
			memcpy(my_function, &closure->func, sizeof(crex_internal_function));
		}
		/* use scope of passed object */
		my_function->common.scope = newclass;
		if (closure->func.type == CREX_INTERNAL_FUNCTION) {
			my_function->internal_function.handler = closure->orig_internal_handler;
		}
		fci_cache.function_handler = my_function;

		/* Runtime cache relies on bound scope to be immutable, hence we need a separate rt cache in case scope changed */
		if (CREX_USER_CODE(my_function->type)
		 && (closure->func.common.scope != newclass
		  || (closure->func.common.fn_flags & CREX_ACC_HEAP_RT_CACHE))) {
			void *ptr;

			my_function->op_array.fn_flags |= CREX_ACC_HEAP_RT_CACHE;
			ptr = emalloc(my_function->op_array.cache_size);
			CREX_MAP_PTR_INIT(my_function->op_array.run_time_cache, ptr);
			memset(ptr, 0, my_function->op_array.cache_size);
		}

		crex_call_function(&fci, &fci_cache);

		if (CREX_USER_CODE(my_function->type)) {
			if (fci_cache.function_handler->common.fn_flags & CREX_ACC_HEAP_RT_CACHE) {
				efree(CREX_MAP_PTR(my_function->op_array.run_time_cache));
			}
		}
		efree_size(fake_closure, sizeof(crex_closure));
	}

	if (C_TYPE(closure_result) != IS_UNDEF) {
		if (C_ISREF(closure_result)) {
			crex_unwrap_reference(&closure_result);
		}
		ZVAL_COPY_VALUE(return_value, &closure_result);
	}
}
/* }}} */

static void do_closure_bind(zval *return_value, zval *zclosure, zval *newthis, crex_object *scope_obj, crex_string *scope_str)
{
	crex_class_entry *ce, *called_scope;
	crex_closure *closure = (crex_closure *) C_OBJ_P(zclosure);

	if (scope_obj) {
		ce = scope_obj->ce;
	} else if (scope_str) {
		if (crex_string_equals(scope_str, ZSTR_KNOWN(CREX_STR_STATIC))) {
			ce = closure->func.common.scope;
		} else if ((ce = crex_lookup_class(scope_str)) == NULL) {
			crex_error(E_WARNING, "Class \"%s\" not found", ZSTR_VAL(scope_str));
			RETURN_NULL();
		}
	} else {
		ce = NULL;
	}

	if (!crex_valid_closure_binding(closure, newthis, ce)) {
		return;
	}

	if (newthis) {
		called_scope = C_OBJCE_P(newthis);
	} else {
		called_scope = ce;
	}

	crex_create_closure(return_value, &closure->func, ce, called_scope, newthis);
}

/* {{{ Create a closure from another one and bind to another object and scope */
CREX_METHOD(Closure, bind)
{
	zval *zclosure, *newthis;
	crex_object *scope_obj = NULL;
	crex_string *scope_str = ZSTR_KNOWN(CREX_STR_STATIC);

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_OBJECT_OF_CLASS(zclosure, crex_ce_closure)
		C_PARAM_OBJECT_OR_NULL(newthis)
		C_PARAM_OPTIONAL
		C_PARAM_OBJ_OR_STR_OR_NULL(scope_obj, scope_str)
	CREX_PARSE_PARAMETERS_END();

	do_closure_bind(return_value, zclosure, newthis, scope_obj, scope_str);
}

/* {{{ Create a closure from another one and bind to another object and scope */
CREX_METHOD(Closure, bindTo)
{
	zval *newthis;
	crex_object *scope_obj = NULL;
	crex_string *scope_str = ZSTR_KNOWN(CREX_STR_STATIC);

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_OBJECT_OR_NULL(newthis)
		C_PARAM_OPTIONAL
		C_PARAM_OBJ_OR_STR_OR_NULL(scope_obj, scope_str)
	CREX_PARSE_PARAMETERS_END();

	do_closure_bind(return_value, getThis(), newthis, scope_obj, scope_str);
}

static CREX_NAMED_FUNCTION(crex_closure_call_magic) /* {{{ */ {
	crex_fcall_info fci;
	crex_fcall_info_cache fcc;
	zval params[2];

	memset(&fci, 0, sizeof(crex_fcall_info));
	memset(&fcc, 0, sizeof(crex_fcall_info_cache));

	fci.size = sizeof(crex_fcall_info);
	fci.retval = return_value;

	fcc.function_handler = (EX(func)->internal_function.fn_flags & CREX_ACC_STATIC) ?
		EX(func)->internal_function.scope->__callstatic : EX(func)->internal_function.scope->__call;
	fci.named_params = NULL;
	fci.params = params;
	fci.param_count = 2;
	ZVAL_STR(&fci.params[0], EX(func)->common.function_name);
	if (EX_CALL_INFO() & CREX_CALL_HAS_EXTRA_NAMED_PARAMS) {
		crex_string *name;
		zval *named_param_zval;
		array_init_size(&fci.params[1], CREX_NUM_ARGS() + crex_hash_num_elements(EX(extra_named_params)));
		/* Avoid conversion from packed to mixed later. */
		crex_hash_real_init_mixed(C_ARRVAL(fci.params[1]));
		crex_copy_parameters_array(CREX_NUM_ARGS(), &fci.params[1]);
		CREX_HASH_MAP_FOREACH_STR_KEY_VAL(EX(extra_named_params), name, named_param_zval) {
			C_TRY_ADDREF_P(named_param_zval);
			crex_hash_add_new(C_ARRVAL(fci.params[1]), name, named_param_zval);
		} CREX_HASH_FOREACH_END();
	} else if (CREX_NUM_ARGS()) {
		array_init_size(&fci.params[1], CREX_NUM_ARGS());
		crex_copy_parameters_array(CREX_NUM_ARGS(), &fci.params[1]);
	} else {
		ZVAL_EMPTY_ARRAY(&fci.params[1]);
	}

	fcc.object = fci.object = C_OBJ_P(CREX_THIS);
	fcc.called_scope = crex_get_called_scope(EG(current_execute_data));

	crex_call_function(&fci, &fcc);

	zval_ptr_dtor(&fci.params[1]);
}
/* }}} */

static crex_result crex_create_closure_from_callable(zval *return_value, zval *callable, char **error) /* {{{ */ {
	crex_fcall_info_cache fcc;
	crex_function *mptr;
	zval instance;
	crex_internal_function call;

	if (!crex_is_callable_ex(callable, NULL, 0, NULL, &fcc, error)) {
		return FAILURE;
	}

	mptr = fcc.function_handler;
	if (mptr->common.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE) {
		/* For Closure::fromCallable([$closure, "__invoke"]) return $closure. */
		if (fcc.object && fcc.object->ce == crex_ce_closure
				&& crex_string_equals(mptr->common.function_name, ZSTR_KNOWN(CREX_STR_MAGIC_INVOKE))) {
			RETVAL_OBJ_COPY(fcc.object);
			crex_free_trampoline(mptr);
			return SUCCESS;
		}

		if (!mptr->common.scope) {
			return FAILURE;
		}
		if (mptr->common.fn_flags & CREX_ACC_STATIC) {
			if (!mptr->common.scope->__callstatic) {
				return FAILURE;
			}
		} else {
			if (!mptr->common.scope->__call) {
				return FAILURE;
			}
		}

		memset(&call, 0, sizeof(crex_internal_function));
		call.type = CREX_INTERNAL_FUNCTION;
		call.fn_flags = mptr->common.fn_flags & CREX_ACC_STATIC;
		call.handler = crex_closure_call_magic;
		call.function_name = mptr->common.function_name;
		call.scope = mptr->common.scope;

		crex_free_trampoline(mptr);
		mptr = (crex_function *) &call;
	}

	if (fcc.object) {
		ZVAL_OBJ(&instance, fcc.object);
		crex_create_fake_closure(return_value, mptr, mptr->common.scope, fcc.called_scope, &instance);
	} else {
		crex_create_fake_closure(return_value, mptr, mptr->common.scope, fcc.called_scope, NULL);
	}

	if (&mptr->internal_function == &call) {
		crex_string_release(mptr->common.function_name);
	}

	return SUCCESS;
}
/* }}} */

/* {{{ Create a closure from a callable using the current scope. */
CREX_METHOD(Closure, fromCallable)
{
	zval *callable;
	char *error = NULL;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(callable)
	CREX_PARSE_PARAMETERS_END();

	if (C_TYPE_P(callable) == IS_OBJECT && instanceof_function(C_OBJCE_P(callable), crex_ce_closure)) {
		/* It's already a closure */
		RETURN_COPY(callable);
	}

	if (crex_create_closure_from_callable(return_value, callable, &error) == FAILURE) {
		if (error) {
			crex_type_error("Failed to create closure from callable: %s", error);
			efree(error);
		} else {
			crex_type_error("Failed to create closure from callable");
		}
	}
}
/* }}} */

static CREX_COLD crex_function *crex_closure_get_constructor(crex_object *object) /* {{{ */
{
	crex_throw_error(NULL, "Instantiation of class Closure is not allowed");
	return NULL;
}
/* }}} */

/* int return due to Object Handler API */
static int crex_closure_compare(zval *o1, zval *o2) /* {{{ */
{
	CREX_COMPARE_OBJECTS_FALLBACK(o1, o2);

	crex_closure *lhs = (crex_closure*) C_OBJ_P(o1);
	crex_closure *rhs = (crex_closure*) C_OBJ_P(o2);

	if (!((lhs->func.common.fn_flags & CREX_ACC_FAKE_CLOSURE) && (rhs->func.common.fn_flags & CREX_ACC_FAKE_CLOSURE))) {
		return CREX_UNCOMPARABLE;
	}

	if (C_TYPE(lhs->this_ptr) != C_TYPE(rhs->this_ptr)) {
		return CREX_UNCOMPARABLE;
	}

	if (C_TYPE(lhs->this_ptr) == IS_OBJECT && C_OBJ(lhs->this_ptr) != C_OBJ(rhs->this_ptr)) {
		return CREX_UNCOMPARABLE;
	}

	if (lhs->called_scope != rhs->called_scope) {
		return CREX_UNCOMPARABLE;
	}

	if (lhs->func.type != rhs->func.type) {
		return CREX_UNCOMPARABLE;
	}

	if (lhs->func.common.scope != rhs->func.common.scope) {
		return CREX_UNCOMPARABLE;
	}

	if (!crex_string_equals(lhs->func.common.function_name, rhs->func.common.function_name)) {
		return CREX_UNCOMPARABLE;
	}

	return 0;
}
/* }}} */

CREX_API crex_function *crex_get_closure_invoke_method(crex_object *object) /* {{{ */
{
	crex_closure *closure = (crex_closure *)object;
	crex_function *invoke = (crex_function*)emalloc(sizeof(crex_function));
	const uint32_t keep_flags =
		CREX_ACC_RETURN_REFERENCE | CREX_ACC_VARIADIC | CREX_ACC_HAS_RETURN_TYPE;

	invoke->common = closure->func.common;
	/* We return CREX_INTERNAL_FUNCTION, but arg_info representation is the
	 * same as for CREX_USER_FUNCTION (uses crex_string* instead of char*).
	 * This is not a problem, because CREX_ACC_HAS_TYPE_HINTS is never set,
	 * and we won't check arguments on internal function. We also set
	 * CREX_ACC_USER_ARG_INFO flag to prevent invalid usage by Reflection */
	invoke->type = CREX_INTERNAL_FUNCTION;
	invoke->internal_function.fn_flags =
		CREX_ACC_PUBLIC | CREX_ACC_CALL_VIA_HANDLER | (closure->func.common.fn_flags & keep_flags);
	if (closure->func.type != CREX_INTERNAL_FUNCTION || (closure->func.common.fn_flags & CREX_ACC_USER_ARG_INFO)) {
		invoke->internal_function.fn_flags |=
			CREX_ACC_USER_ARG_INFO;
	}
	invoke->internal_function.handler = CREX_MN(Closure___invoke);
	invoke->internal_function.module = 0;
	invoke->internal_function.scope = crex_ce_closure;
	invoke->internal_function.function_name = ZSTR_KNOWN(CREX_STR_MAGIC_INVOKE);
	return invoke;
}
/* }}} */

CREX_API const crex_function *crex_get_closure_method_def(crex_object *obj) /* {{{ */
{
	crex_closure *closure = (crex_closure *) obj;
	return &closure->func;
}
/* }}} */

CREX_API zval* crex_get_closure_this_ptr(zval *obj) /* {{{ */
{
	crex_closure *closure = (crex_closure *)C_OBJ_P(obj);
	return &closure->this_ptr;
}
/* }}} */

static crex_function *crex_closure_get_method(crex_object **object, crex_string *method, const zval *key) /* {{{ */
{
	if (crex_string_equals_literal_ci(method, CREX_INVOKE_FUNC_NAME)) {
		return crex_get_closure_invoke_method(*object);
	}

	return crex_std_get_method(object, method, key);
}
/* }}} */

static void crex_closure_free_storage(crex_object *object) /* {{{ */
{
	crex_closure *closure = (crex_closure *)object;

	crex_object_std_dtor(&closure->std);

	if (closure->func.type == CREX_USER_FUNCTION) {
		/* We don't own the static variables of fake closures. */
		if (!(closure->func.op_array.fn_flags & CREX_ACC_FAKE_CLOSURE)) {
			crex_destroy_static_vars(&closure->func.op_array);
			closure->func.op_array.static_variables = NULL;
		}
		destroy_op_array(&closure->func.op_array);
	} else if (closure->func.type == CREX_INTERNAL_FUNCTION) {
		crex_string_release(closure->func.common.function_name);
	}

	if (C_TYPE(closure->this_ptr) != IS_UNDEF) {
		zval_ptr_dtor(&closure->this_ptr);
	}
}
/* }}} */

static crex_object *crex_closure_new(crex_class_entry *class_type) /* {{{ */
{
	crex_closure *closure;

	closure = emalloc(sizeof(crex_closure));
	memset(closure, 0, sizeof(crex_closure));

	crex_object_std_init(&closure->std, class_type);

	return (crex_object*)closure;
}
/* }}} */

static crex_object *crex_closure_clone(crex_object *zobject) /* {{{ */
{
	crex_closure *closure = (crex_closure *)zobject;
	zval result;

	crex_create_closure(&result, &closure->func,
		closure->func.common.scope, closure->called_scope, &closure->this_ptr);
	return C_OBJ(result);
}
/* }}} */

static crex_result crex_closure_get_closure(crex_object *obj, crex_class_entry **ce_ptr, crex_function **fptr_ptr, crex_object **obj_ptr, bool check_only) /* {{{ */
{
	crex_closure *closure = (crex_closure*)obj;

	*fptr_ptr = &closure->func;
	*ce_ptr = closure->called_scope;

	if (C_TYPE(closure->this_ptr) != IS_UNDEF) {
		*obj_ptr = C_OBJ(closure->this_ptr);
	} else {
		*obj_ptr = NULL;
	}

	return SUCCESS;
}
/* }}} */

/* *is_temp is int due to Object Handler API */
static HashTable *crex_closure_get_debug_info(crex_object *object, int *is_temp) /* {{{ */
{
	crex_closure *closure = (crex_closure *)object;
	zval val;
	struct _crex_arg_info *arg_info = closure->func.common.arg_info;
	HashTable *debug_info;
	bool zstr_args = (closure->func.type == CREX_USER_FUNCTION) || (closure->func.common.fn_flags & CREX_ACC_USER_ARG_INFO);

	*is_temp = 1;

	debug_info = crex_new_array(8);

	if (closure->func.op_array.fn_flags & CREX_ACC_FAKE_CLOSURE) {
		if (closure->func.common.scope) {
			crex_string *class_name = closure->func.common.scope->name;
			crex_string *func_name = closure->func.common.function_name;
			crex_string *combined = crex_string_concat3(
				ZSTR_VAL(class_name), ZSTR_LEN(class_name),
				"::", strlen("::"),
				ZSTR_VAL(func_name), ZSTR_LEN(func_name)
			);
			ZVAL_STR(&val, combined);
		} else {
			ZVAL_STR_COPY(&val, closure->func.common.function_name);
		}
		crex_hash_update(debug_info, ZSTR_KNOWN(CREX_STR_FUNCTION), &val);
	}

	if (closure->func.type == CREX_USER_FUNCTION && closure->func.op_array.static_variables) {
		zval *var;
		crex_string *key;
		HashTable *static_variables = CREX_MAP_PTR_GET(closure->func.op_array.static_variables_ptr);

		array_init(&val);

		CREX_HASH_MAP_FOREACH_STR_KEY_VAL(static_variables, key, var) {
			zval copy;

			if (C_ISREF_P(var) && C_REFCOUNT_P(var) == 1) {
				var = C_REFVAL_P(var);
			}
			ZVAL_COPY(&copy, var);

			crex_hash_add_new(C_ARRVAL(val), key, &copy);
		} CREX_HASH_FOREACH_END();

		if (crex_hash_num_elements(C_ARRVAL(val))) {
			crex_hash_update(debug_info, ZSTR_KNOWN(CREX_STR_STATIC), &val);
		} else {
			zval_ptr_dtor(&val);
		}
	}

	if (C_TYPE(closure->this_ptr) != IS_UNDEF) {
		C_ADDREF(closure->this_ptr);
		crex_hash_update(debug_info, ZSTR_KNOWN(CREX_STR_THIS), &closure->this_ptr);
	}

	if (arg_info &&
		(closure->func.common.num_args ||
		 (closure->func.common.fn_flags & CREX_ACC_VARIADIC))) {
		uint32_t i, num_args, required = closure->func.common.required_num_args;

		array_init(&val);

		num_args = closure->func.common.num_args;
		if (closure->func.common.fn_flags & CREX_ACC_VARIADIC) {
			num_args++;
		}
		for (i = 0; i < num_args; i++) {
			crex_string *name;
			zval info;
			CREX_ASSERT(arg_info->name && "Argument should have name");
			if (zstr_args) {
				name = crex_strpprintf(0, "%s$%s",
						CREX_ARG_SEND_MODE(arg_info) ? "&" : "",
						ZSTR_VAL(arg_info->name));
			} else {
				name = crex_strpprintf(0, "%s$%s",
						CREX_ARG_SEND_MODE(arg_info) ? "&" : "",
						((crex_internal_arg_info*)arg_info)->name);
			}
			ZVAL_NEW_STR(&info, crex_strpprintf(0, "%s", i >= required ? "<optional>" : "<required>"));
			crex_hash_update(C_ARRVAL(val), name, &info);
			crex_string_release_ex(name, 0);
			arg_info++;
		}
		crex_hash_str_update(debug_info, "parameter", sizeof("parameter")-1, &val);
	}

	return debug_info;
}
/* }}} */

static HashTable *crex_closure_get_gc(crex_object *obj, zval **table, int *n) /* {{{ */
{
	crex_closure *closure = (crex_closure *)obj;

	*table = C_TYPE(closure->this_ptr) != IS_NULL ? &closure->this_ptr : NULL;
	*n = C_TYPE(closure->this_ptr) != IS_NULL ? 1 : 0;
	/* Fake closures don't own the static variables they reference. */
	return (closure->func.type == CREX_USER_FUNCTION
			&& !(closure->func.op_array.fn_flags & CREX_ACC_FAKE_CLOSURE)) ?
		CREX_MAP_PTR_GET(closure->func.op_array.static_variables_ptr) : NULL;
}
/* }}} */

/* {{{ Private constructor preventing instantiation */
CREX_COLD CREX_METHOD(Closure, __main)
{
	crex_throw_error(NULL, "Instantiation of class Closure is not allowed");
}
/* }}} */

void crex_register_closure_ce(void) /* {{{ */
{
	crex_ce_closure = register_class_Closure();
	crex_ce_closure->create_object = crex_closure_new;
	crex_ce_closure->default_object_handlers = &closure_handlers;

	memcpy(&closure_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	closure_handlers.free_obj = crex_closure_free_storage;
	closure_handlers.get_constructor = crex_closure_get_constructor;
	closure_handlers.get_method = crex_closure_get_method;
	closure_handlers.compare = crex_closure_compare;
	closure_handlers.clone_obj = crex_closure_clone;
	closure_handlers.get_debug_info = crex_closure_get_debug_info;
	closure_handlers.get_closure = crex_closure_get_closure;
	closure_handlers.get_gc = crex_closure_get_gc;
}
/* }}} */

static CREX_NAMED_FUNCTION(crex_closure_internal_handler) /* {{{ */
{
	crex_closure *closure = (crex_closure*)CREX_CLOSURE_OBJECT(EX(func));
	closure->orig_internal_handler(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	// Assign to EX(this) so that it is released after observer checks etc.
	CREX_ADD_CALL_FLAG(execute_data, CREX_CALL_RELEASE_THIS);
	C_OBJ(EX(This)) = &closure->std;
}
/* }}} */

static void crex_create_closure_ex(zval *res, crex_function *func, crex_class_entry *scope, crex_class_entry *called_scope, zval *this_ptr, bool is_fake) /* {{{ */
{
	crex_closure *closure;
	void *ptr;

	object_init_ex(res, crex_ce_closure);

	closure = (crex_closure *)C_OBJ_P(res);

	if ((scope == NULL) && this_ptr && (C_TYPE_P(this_ptr) != IS_UNDEF)) {
		/* use dummy scope if we're binding an object without specifying a scope */
		/* maybe it would be better to create one for this purpose */
		scope = crex_ce_closure;
	}

	if (func->type == CREX_USER_FUNCTION) {
		memcpy(&closure->func, func, sizeof(crex_op_array));
		closure->func.common.fn_flags |= CREX_ACC_CLOSURE;
		closure->func.common.fn_flags &= ~CREX_ACC_IMMUTABLE;

		crex_string_addref(closure->func.op_array.function_name);
		if (closure->func.op_array.refcount) {
			(*closure->func.op_array.refcount)++;
		}

		/* For fake closures, we want to reuse the static variables of the original function. */
		if (!is_fake) {
			if (closure->func.op_array.static_variables) {
				closure->func.op_array.static_variables =
					crex_array_dup(closure->func.op_array.static_variables);
			}
			CREX_MAP_PTR_INIT(closure->func.op_array.static_variables_ptr,
				closure->func.op_array.static_variables);
		} else if (func->op_array.static_variables) {
			HashTable *ht = CREX_MAP_PTR_GET(func->op_array.static_variables_ptr);

			if (!ht) {
				ht = crex_array_dup(func->op_array.static_variables);
				CREX_MAP_PTR_SET(func->op_array.static_variables_ptr, ht);
			}
			CREX_MAP_PTR_INIT(closure->func.op_array.static_variables_ptr, ht);
		}

		/* Runtime cache is scope-dependent, so we cannot reuse it if the scope changed */
		ptr = CREX_MAP_PTR_GET(func->op_array.run_time_cache);
		if (!ptr
			|| func->common.scope != scope
			|| (func->common.fn_flags & CREX_ACC_HEAP_RT_CACHE)
		) {
			if (!ptr
			 && (func->common.fn_flags & CREX_ACC_CLOSURE)
			 && (func->common.scope == scope ||
			     !(func->common.fn_flags & CREX_ACC_IMMUTABLE))) {
				/* If a real closure is used for the first time, we create a shared runtime cache
				 * and remember which scope it is for. */
				if (func->common.scope != scope) {
					func->common.scope = scope;
				}
				ptr = crex_arena_alloc(&CG(arena), func->op_array.cache_size);
				CREX_MAP_PTR_SET(func->op_array.run_time_cache, ptr);
				closure->func.op_array.fn_flags &= ~CREX_ACC_HEAP_RT_CACHE;
			} else {
				/* Otherwise, we use a non-shared runtime cache */
				ptr = emalloc(func->op_array.cache_size);
				closure->func.op_array.fn_flags |= CREX_ACC_HEAP_RT_CACHE;
			}
			memset(ptr, 0, func->op_array.cache_size);
		}
		CREX_MAP_PTR_INIT(closure->func.op_array.run_time_cache, ptr);
	} else {
		memcpy(&closure->func, func, sizeof(crex_internal_function));
		closure->func.common.fn_flags |= CREX_ACC_CLOSURE;
		/* wrap internal function handler to avoid memory leak */
		if (UNEXPECTED(closure->func.internal_function.handler == crex_closure_internal_handler)) {
			/* avoid infinity recursion, by taking handler from nested closure */
			crex_closure *nested = (crex_closure*)((char*)func - XtOffsetOf(crex_closure, func));
			CREX_ASSERT(nested->std.ce == crex_ce_closure);
			closure->orig_internal_handler = nested->orig_internal_handler;
		} else {
			closure->orig_internal_handler = closure->func.internal_function.handler;
		}
		closure->func.internal_function.handler = crex_closure_internal_handler;
		crex_string_addref(closure->func.op_array.function_name);
		if (!func->common.scope) {
			/* if it's a free function, we won't set scope & this since they're meaningless */
			this_ptr = NULL;
			scope = NULL;
		}
	}

	ZVAL_UNDEF(&closure->this_ptr);
	/* Invariant:
	 * If the closure is unscoped or static, it has no bound object. */
	closure->func.common.scope = scope;
	closure->called_scope = called_scope;
	if (scope) {
		closure->func.common.fn_flags |= CREX_ACC_PUBLIC;
		if (this_ptr && C_TYPE_P(this_ptr) == IS_OBJECT && (closure->func.common.fn_flags & CREX_ACC_STATIC) == 0) {
			ZVAL_OBJ_COPY(&closure->this_ptr, C_OBJ_P(this_ptr));
		}
	}
}
/* }}} */

CREX_API void crex_create_closure(zval *res, crex_function *func, crex_class_entry *scope, crex_class_entry *called_scope, zval *this_ptr)
{
	crex_create_closure_ex(res, func, scope, called_scope, this_ptr,
		/* is_fake */ (func->common.fn_flags & CREX_ACC_FAKE_CLOSURE) != 0);
}

CREX_API void crex_create_fake_closure(zval *res, crex_function *func, crex_class_entry *scope, crex_class_entry *called_scope, zval *this_ptr) /* {{{ */
{
	crex_closure *closure;

	crex_create_closure_ex(res, func, scope, called_scope, this_ptr, /* is_fake */ true);

	closure = (crex_closure *)C_OBJ_P(res);
	closure->func.common.fn_flags |= CREX_ACC_FAKE_CLOSURE;
}
/* }}} */

/* __call and __callStatic name the arguments "$arguments" in the docs. */
static crex_internal_arg_info trampoline_arg_info[] = {CREX_ARG_VARIADIC_TYPE_INFO(false, arguments, IS_MIXED, false)};

void crex_closure_from_frame(zval *return_value, crex_execute_data *call) { /* {{{ */
	zval instance;
	crex_internal_function trampoline;
	crex_function *mptr = call->func;

	if (CREX_CALL_INFO(call) & CREX_CALL_CLOSURE) {
		RETURN_OBJ(CREX_CLOSURE_OBJECT(mptr));
	}

	if (mptr->common.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE) {
		if ((CREX_CALL_INFO(call) & CREX_CALL_HAS_THIS) &&
			(C_OBJCE(call->This) == crex_ce_closure)
			&& crex_string_equals(mptr->common.function_name, ZSTR_KNOWN(CREX_STR_MAGIC_INVOKE))) {
	        crex_free_trampoline(mptr);
	        RETURN_OBJ_COPY(C_OBJ(call->This));
	    }

		memset(&trampoline, 0, sizeof(crex_internal_function));
		trampoline.type = CREX_INTERNAL_FUNCTION;
		trampoline.fn_flags = mptr->common.fn_flags & (CREX_ACC_STATIC | CREX_ACC_VARIADIC);
		trampoline.handler = crex_closure_call_magic;
		trampoline.function_name = mptr->common.function_name;
		trampoline.scope = mptr->common.scope;
		if (trampoline.fn_flags & CREX_ACC_VARIADIC) {
			trampoline.arg_info = trampoline_arg_info;
		}

		crex_free_trampoline(mptr);
		mptr = (crex_function *) &trampoline;
	}

	if (CREX_CALL_INFO(call) & CREX_CALL_HAS_THIS) {
		ZVAL_OBJ(&instance, C_OBJ(call->This));

		crex_create_fake_closure(return_value, mptr, mptr->common.scope, C_OBJCE(instance), &instance);
	} else {
		crex_create_fake_closure(return_value, mptr, mptr->common.scope, C_CE(call->This), NULL);
	}

	if (&mptr->internal_function == &trampoline) {
		crex_string_release(mptr->common.function_name);
	}
} /* }}} */

void crex_closure_bind_var(zval *closure_zv, crex_string *var_name, zval *var) /* {{{ */
{
	crex_closure *closure = (crex_closure *) C_OBJ_P(closure_zv);
	HashTable *static_variables = CREX_MAP_PTR_GET(closure->func.op_array.static_variables_ptr);
	crex_hash_update(static_variables, var_name, var);
}
/* }}} */

void crex_closure_bind_var_ex(zval *closure_zv, uint32_t offset, zval *val) /* {{{ */
{
	crex_closure *closure = (crex_closure *) C_OBJ_P(closure_zv);
	HashTable *static_variables = CREX_MAP_PTR_GET(closure->func.op_array.static_variables_ptr);
	zval *var = (zval*)((char*)static_variables->arData + offset);
	zval_ptr_dtor(var);
	ZVAL_COPY_VALUE(var, val);
}
/* }}} */
