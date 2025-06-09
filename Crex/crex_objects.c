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
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#include "crex.h"
#include "crex_globals.h"
#include "crex_variables.h"
#include "crex_API.h"
#include "crex_interfaces.h"
#include "crex_exceptions.h"
#include "crex_weakrefs.h"

static crex_always_inline void _crex_object_std_init(crex_object *object, crex_class_entry *ce)
{
	GC_SET_REFCOUNT(object, 1);
	GC_TYPE_INFO(object) = GC_OBJECT;
	object->ce = ce;
	object->handlers = ce->default_object_handlers;
	object->properties = NULL;
	crex_objects_store_put(object);
	if (UNEXPECTED(ce->ce_flags & CREX_ACC_USE_GUARDS)) {
		zval *guard_value = object->properties_table + object->ce->default_properties_count;
		ZVAL_UNDEF(guard_value);
		C_GUARD_P(guard_value) = 0;
	}
}

CREX_API void CREX_FASTCALL crex_object_std_init(crex_object *object, crex_class_entry *ce)
{
	_crex_object_std_init(object, ce);
}

CREX_API void crex_object_std_dtor(crex_object *object)
{
	zval *p, *end;

	if (object->properties) {
		if (EXPECTED(!(GC_FLAGS(object->properties) & IS_ARRAY_IMMUTABLE))) {
			if (EXPECTED(GC_DELREF(object->properties) == 0)
					&& EXPECTED(GC_TYPE(object->properties) != IS_NULL)) {
				crex_array_destroy(object->properties);
			}
		}
	}
	p = object->properties_table;
	if (EXPECTED(object->ce->default_properties_count)) {
		end = p + object->ce->default_properties_count;
		do {
			if (C_REFCOUNTED_P(p)) {
				if (UNEXPECTED(C_ISREF_P(p)) &&
						(CREX_DEBUG || CREX_REF_HAS_TYPE_SOURCES(C_REF_P(p)))) {
					crex_property_info *prop_info = crex_get_property_info_for_slot(object, p);
					if (CREX_TYPE_IS_SET(prop_info->type)) {
						CREX_REF_DEL_TYPE_SOURCE(C_REF_P(p), prop_info);
					}
				}
				i_zval_ptr_dtor(p);
			}
			p++;
		} while (p != end);
	}

	if (UNEXPECTED(object->ce->ce_flags & CREX_ACC_USE_GUARDS)) {
		if (EXPECTED(C_TYPE_P(p) == IS_STRING)) {
			zval_ptr_dtor_str(p);
		} else if (C_TYPE_P(p) == IS_ARRAY) {
			HashTable *guards;

			guards = C_ARRVAL_P(p);
			CREX_ASSERT(guards != NULL);
			crex_hash_destroy(guards);
			FREE_HASHTABLE(guards);
		}
	}

	if (UNEXPECTED(GC_FLAGS(object) & IS_OBJ_WEAKLY_REFERENCED)) {
		crex_weakrefs_notify(object);
	}
}

CREX_API void crex_objects_destroy_object(crex_object *object)
{
	crex_function *destructor = object->ce->destructor;

	if (destructor) {
		crex_object *old_exception;
		const crex_op *old_opline_before_exception;

		if (destructor->op_array.fn_flags & (CREX_ACC_PRIVATE|CREX_ACC_PROTECTED)) {
			if (destructor->op_array.fn_flags & CREX_ACC_PRIVATE) {
				/* Ensure that if we're calling a private function, we're allowed to do so.
				 */
				if (EG(current_execute_data)) {
					crex_class_entry *scope = crex_get_executed_scope();

					if (object->ce != scope) {
						crex_throw_error(NULL,
							"Call to private %s::__destruct() from %s%s",
							ZSTR_VAL(object->ce->name),
							scope ? "scope " : "global scope",
							scope ? ZSTR_VAL(scope->name) : ""
						);
						return;
					}
				} else {
					crex_error(E_WARNING,
						"Call to private %s::__destruct() from global scope during shutdown ignored",
						ZSTR_VAL(object->ce->name));
					return;
				}
			} else {
				/* Ensure that if we're calling a protected function, we're allowed to do so.
				 */
				if (EG(current_execute_data)) {
					crex_class_entry *scope = crex_get_executed_scope();

					if (!crex_check_protected(crex_get_function_root_class(destructor), scope)) {
						crex_throw_error(NULL,
							"Call to protected %s::__destruct() from %s%s",
							ZSTR_VAL(object->ce->name),
							scope ? "scope " : "global scope",
							scope ? ZSTR_VAL(scope->name) : ""
						);
						return;
					}
				} else {
					crex_error(E_WARNING,
						"Call to protected %s::__destruct() from global scope during shutdown ignored",
						ZSTR_VAL(object->ce->name));
					return;
				}
			}
		}

		GC_ADDREF(object);

		/* Make sure that destructors are protected from previously thrown exceptions.
		 * For example, if an exception was thrown in a function and when the function's
		 * local variable destruction results in a destructor being called.
		 */
		old_exception = NULL;
		if (EG(exception)) {
			if (EG(exception) == object) {
				crex_error_noreturn(E_CORE_ERROR, "Attempt to destruct pending exception");
			} else {
				if (EG(current_execute_data)
				 && EG(current_execute_data)->func
				 && CREX_USER_CODE(EG(current_execute_data)->func->common.type)) {
					crex_rethrow_exception(EG(current_execute_data));
				}
				old_exception = EG(exception);
				old_opline_before_exception = EG(opline_before_exception);
				EG(exception) = NULL;
			}
		}

		crex_call_known_instance_method_with_0_params(destructor, object, NULL);

		if (old_exception) {
			EG(opline_before_exception) = old_opline_before_exception;
			if (EG(exception)) {
				crex_exception_set_previous(EG(exception), old_exception);
			} else {
				EG(exception) = old_exception;
			}
		}
		OBJ_RELEASE(object);
	}
}

CREX_API crex_object* CREX_FASTCALL crex_objects_new(crex_class_entry *ce)
{
	crex_object *object = emalloc(sizeof(crex_object) + crex_object_properties_size(ce));

	_crex_object_std_init(object, ce);
	return object;
}

CREX_API void CREX_FASTCALL crex_objects_clone_members(crex_object *new_object, crex_object *old_object)
{
	bool has_clone_method = old_object->ce->clone != NULL;

	if (old_object->ce->default_properties_count) {
		zval *src = old_object->properties_table;
		zval *dst = new_object->properties_table;
		zval *end = src + old_object->ce->default_properties_count;

		do {
			i_zval_ptr_dtor(dst);
			ZVAL_COPY_VALUE_PROP(dst, src);
			zval_add_ref(dst);
			if (has_clone_method) {
				/* Unconditionally add the IS_PROP_REINITABLE flag to avoid a potential cache miss of property_info */
				C_PROP_FLAG_P(dst) |= IS_PROP_REINITABLE;
			}

			if (UNEXPECTED(C_ISREF_P(dst)) &&
					(CREX_DEBUG || CREX_REF_HAS_TYPE_SOURCES(C_REF_P(dst)))) {
				crex_property_info *prop_info = crex_get_property_info_for_slot(new_object, dst);
				if (CREX_TYPE_IS_SET(prop_info->type)) {
					CREX_REF_ADD_TYPE_SOURCE(C_REF_P(dst), prop_info);
				}
			}
			src++;
			dst++;
		} while (src != end);
	} else if (old_object->properties && !has_clone_method) {
		/* fast copy */
		if (EXPECTED(old_object->handlers == &std_object_handlers)) {
			if (EXPECTED(!(GC_FLAGS(old_object->properties) & IS_ARRAY_IMMUTABLE))) {
				GC_ADDREF(old_object->properties);
			}
			new_object->properties = old_object->properties;
			return;
		}
	}

	if (old_object->properties &&
	    EXPECTED(crex_hash_num_elements(old_object->properties))) {
		zval *prop, new_prop;
		crex_ulong num_key;
		crex_string *key;

		if (!new_object->properties) {
			new_object->properties = crex_new_array(crex_hash_num_elements(old_object->properties));
			crex_hash_real_init_mixed(new_object->properties);
		} else {
			crex_hash_extend(new_object->properties, new_object->properties->nNumUsed + crex_hash_num_elements(old_object->properties), 0);
		}

		HT_FLAGS(new_object->properties) |=
			HT_FLAGS(old_object->properties) & HASH_FLAG_HAS_EMPTY_IND;

		CREX_HASH_MAP_FOREACH_KEY_VAL(old_object->properties, num_key, key, prop) {
			if (C_TYPE_P(prop) == IS_INDIRECT) {
				ZVAL_INDIRECT(&new_prop, new_object->properties_table + (C_INDIRECT_P(prop) - old_object->properties_table));
			} else {
				ZVAL_COPY_VALUE(&new_prop, prop);
				zval_add_ref(&new_prop);
			}
			if (has_clone_method) {
				/* Unconditionally add the IS_PROP_REINITABLE flag to avoid a potential cache miss of property_info */
				C_PROP_FLAG_P(&new_prop) |= IS_PROP_REINITABLE;
			}
			if (EXPECTED(key)) {
				_crex_hash_append(new_object->properties, key, &new_prop);
			} else {
				crex_hash_index_add_new(new_object->properties, num_key, &new_prop);
			}
		} CREX_HASH_FOREACH_END();
	}

	if (has_clone_method) {
		GC_ADDREF(new_object);
		crex_call_known_instance_method_with_0_params(new_object->ce->clone, new_object, NULL);

		if (CREX_CLASS_HAS_READONLY_PROPS(new_object->ce)) {
			for (uint32_t i = 0; i < new_object->ce->default_properties_count; i++) {
				zval* prop = OBJ_PROP_NUM(new_object, i);
				/* Unconditionally remove the IS_PROP_REINITABLE flag to avoid a potential cache miss of property_info */
				C_PROP_FLAG_P(prop) &= ~IS_PROP_REINITABLE;
			}
		}

		OBJ_RELEASE(new_object);
	}
}

CREX_API crex_object *crex_objects_clone_obj(crex_object *old_object)
{
	crex_object *new_object;

	/* assume that create isn't overwritten, so when clone depends on the
	 * overwritten one then it must itself be overwritten */
	new_object = crex_objects_new(old_object->ce);

	/* crex_objects_clone_members() expect the properties to be initialized. */
	if (new_object->ce->default_properties_count) {
		zval *p = new_object->properties_table;
		zval *end = p + new_object->ce->default_properties_count;
		do {
			ZVAL_UNDEF(p);
			p++;
		} while (p != end);
	}

	crex_objects_clone_members(new_object, old_object);

	return new_object;
}
