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
   | Authors: Levi Morrison <levim@crx.net>                               |
   |          Sammy Kaye Powers <sammyk@crx.net>                          |
   +----------------------------------------------------------------------+
*/

#include "crex_observer.h"

#include "crex_extensions.h"
#include "crex_llist.h"
#include "crex_vm.h"

#define CREX_OBSERVER_DATA(function) \
	CREX_OP_ARRAY_EXTENSION((&(function)->common), crex_observer_fcall_op_array_extension)

#define CREX_OBSERVER_NOT_OBSERVED ((void *) 2)

#define CREX_OBSERVABLE_FN(function) \
	(CREX_MAP_PTR(function->common.run_time_cache) && !(function->common.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE))

crex_llist crex_observers_fcall_list;
crex_llist crex_observer_function_declared_callbacks;
crex_llist crex_observer_class_linked_callbacks;
crex_llist crex_observer_error_callbacks;
crex_llist crex_observer_fiber_init;
crex_llist crex_observer_fiber_switch;
crex_llist crex_observer_fiber_destroy;

int crex_observer_fcall_op_array_extension;
bool crex_observer_errors_observed;
bool crex_observer_function_declared_observed;
bool crex_observer_class_linked_observed;

CREX_TLS crex_execute_data *current_observed_frame;

// Call during minit/startup ONLY
CREX_API void crex_observer_fcall_register(crex_observer_fcall_init init)
{
	crex_llist_add_element(&crex_observers_fcall_list, &init);
}

// Called by engine before MINITs
CREX_API void crex_observer_startup(void)
{
	crex_llist_init(&crex_observers_fcall_list, sizeof(crex_observer_fcall_init), NULL, 1);
	crex_llist_init(&crex_observer_function_declared_callbacks, sizeof(crex_observer_function_declared_cb), NULL, 1);
	crex_llist_init(&crex_observer_class_linked_callbacks, sizeof(crex_observer_class_linked_cb), NULL, 1);
	crex_llist_init(&crex_observer_error_callbacks, sizeof(crex_observer_error_cb), NULL, 1);
	crex_llist_init(&crex_observer_fiber_init, sizeof(crex_observer_fiber_init_handler), NULL, 1);
	crex_llist_init(&crex_observer_fiber_switch, sizeof(crex_observer_fiber_switch_handler), NULL, 1);
	crex_llist_init(&crex_observer_fiber_destroy, sizeof(crex_observer_fiber_destroy_handler), NULL, 1);

	crex_observer_fcall_op_array_extension = -1;
}

CREX_API void crex_observer_post_startup(void)
{
	if (crex_observers_fcall_list.count) {
		/* We don't want to get an extension handle unless an ext installs an observer
		 * Allocate each a begin and an end pointer */
		crex_observer_fcall_op_array_extension =
			crex_get_op_array_extension_handles("Crex Observer", (int) crex_observers_fcall_list.count * 2);

		/* CREX_CALL_TRAMPOLINE has SPEC(OBSERVER) but crex_init_call_trampoline_op()
		 * is called before any extensions have registered as an observer. So we
		 * adjust the offset to the observed handler when we know we need to observe. */
		CREX_VM_SET_OPCODE_HANDLER(&EG(call_trampoline_op));

		/* CREX_HANDLE_EXCEPTION also has SPEC(OBSERVER) and no observer extensions
		 * exist when crex_init_exception_op() is called. */
		CREX_VM_SET_OPCODE_HANDLER(EG(exception_op));
		CREX_VM_SET_OPCODE_HANDLER(EG(exception_op) + 1);
		CREX_VM_SET_OPCODE_HANDLER(EG(exception_op) + 2);

		// Add an observer temporary to store previous observed frames
		crex_internal_function *zif;
		CREX_HASH_FOREACH_PTR(CG(function_table), zif) {
			++zif->T;
		} CREX_HASH_FOREACH_END();
		crex_class_entry *ce;
		CREX_HASH_MAP_FOREACH_PTR(CG(class_table), ce) {
			CREX_HASH_MAP_FOREACH_PTR(&ce->function_table, zif) {
				++zif->T;
			} CREX_HASH_FOREACH_END();
		} CREX_HASH_FOREACH_END();
	}
}

CREX_API void crex_observer_activate(void)
{
	current_observed_frame = NULL;
}

CREX_API void crex_observer_shutdown(void)
{
	crex_llist_destroy(&crex_observers_fcall_list);
	crex_llist_destroy(&crex_observer_function_declared_callbacks);
	crex_llist_destroy(&crex_observer_class_linked_callbacks);
	crex_llist_destroy(&crex_observer_error_callbacks);
	crex_llist_destroy(&crex_observer_fiber_init);
	crex_llist_destroy(&crex_observer_fiber_switch);
	crex_llist_destroy(&crex_observer_fiber_destroy);
}

static void crex_observer_fcall_install(crex_execute_data *execute_data)
{
	crex_llist *list = &crex_observers_fcall_list;
	crex_function *function = execute_data->func;

	CREX_ASSERT(RUN_TIME_CACHE(&function->common));
	crex_observer_fcall_begin_handler *begin_handlers = (crex_observer_fcall_begin_handler *)&CREX_OBSERVER_DATA(function);
	crex_observer_fcall_end_handler *end_handlers = (crex_observer_fcall_end_handler *)begin_handlers + list->count, *end_handlers_start = end_handlers;

	*begin_handlers = CREX_OBSERVER_NOT_OBSERVED;
	*end_handlers = CREX_OBSERVER_NOT_OBSERVED;

	for (crex_llist_element *element = list->head; element; element = element->next) {
		crex_observer_fcall_init init;
		memcpy(&init, element->data, sizeof init);
		crex_observer_fcall_handlers handlers = init(execute_data);
		if (handlers.begin) {
			*(begin_handlers++) = handlers.begin;
		}
		if (handlers.end) {
			*(end_handlers++) = handlers.end;
		}
	}

	// end handlers are executed in reverse order
	for (--end_handlers; end_handlers_start < end_handlers; --end_handlers, ++end_handlers_start) {
		crex_observer_fcall_end_handler tmp = *end_handlers;
		*end_handlers = *end_handlers_start;
		*end_handlers_start = tmp;
	}
}

static bool crex_observer_remove_handler(void **first_handler, void *old_handler) {
	size_t registered_observers = crex_observers_fcall_list.count;

	void **last_handler = first_handler + registered_observers - 1;
	for (void **cur_handler = first_handler; cur_handler <= last_handler; ++cur_handler) {
		if (*cur_handler == old_handler) {
			if (registered_observers == 1 || (cur_handler == first_handler && cur_handler[1] == NULL)) {
				*cur_handler = CREX_OBSERVER_NOT_OBSERVED;
			} else {
				if (cur_handler != last_handler) {
					memmove(cur_handler, cur_handler + 1, sizeof(cur_handler) * (last_handler - cur_handler));
				}
				*last_handler = NULL;
			}
			return true;
		}
	}
	return false;
}

CREX_API void crex_observer_add_begin_handler(crex_function *function, crex_observer_fcall_begin_handler begin) {
	size_t registered_observers = crex_observers_fcall_list.count;
	crex_observer_fcall_begin_handler *first_handler = (void *)&CREX_OBSERVER_DATA(function), *last_handler = first_handler + registered_observers - 1;
	if (*first_handler == CREX_OBSERVER_NOT_OBSERVED) {
		*first_handler = begin;
	} else {
		for (crex_observer_fcall_begin_handler *cur_handler = first_handler + 1; cur_handler <= last_handler; ++cur_handler) {
			if (*cur_handler == NULL) {
				*cur_handler = begin;
				return;
			}
		}
		// there's no space for new handlers, then it's forbidden to call this function
		CREX_UNREACHABLE();
	}
}

CREX_API bool crex_observer_remove_begin_handler(crex_function *function, crex_observer_fcall_begin_handler begin) {
	return crex_observer_remove_handler((void **)&CREX_OBSERVER_DATA(function), begin);
}

CREX_API void crex_observer_add_end_handler(crex_function *function, crex_observer_fcall_end_handler end) {
	size_t registered_observers = crex_observers_fcall_list.count;
	crex_observer_fcall_end_handler *end_handler = (crex_observer_fcall_end_handler *)&CREX_OBSERVER_DATA(function) + registered_observers;
	// to allow to preserve the invariant that end handlers are in reverse order of begin handlers, push the new end handler in front
	if (*end_handler != CREX_OBSERVER_NOT_OBSERVED) {
		// there's no space for new handlers, then it's forbidden to call this function
		CREX_ASSERT(end_handler[registered_observers - 1] == NULL);
		memmove(end_handler + 1, end_handler, sizeof(end_handler) * (registered_observers - 1));
	}
	*end_handler = end;
}

CREX_API bool crex_observer_remove_end_handler(crex_function *function, crex_observer_fcall_end_handler end) {
	size_t registered_observers = crex_observers_fcall_list.count;
	return crex_observer_remove_handler((void **)&CREX_OBSERVER_DATA(function) + registered_observers, end);
}

static inline crex_execute_data **prev_observed_frame(crex_execute_data *execute_data) {
	crex_function *func = EX(func);
	CREX_ASSERT(func);
	return (crex_execute_data **)&C_PTR_P(EX_VAR_NUM((CREX_USER_CODE(func->type) ? func->op_array.last_var : CREX_CALL_NUM_ARGS(execute_data)) + func->common.T - 1));
}

static void CREX_FASTCALL _crex_observe_fcall_begin(crex_execute_data *execute_data)
{
	if (!CREX_OBSERVER_ENABLED) {
		return;
	}

	crex_function *function = execute_data->func;

	if (!CREX_OBSERVABLE_FN(function)) {
		return;
	}

	crex_observer_fcall_begin_handler *handler = (crex_observer_fcall_begin_handler *)&CREX_OBSERVER_DATA(function);
	if (!*handler) {
		crex_observer_fcall_install(execute_data);
	}

	crex_observer_fcall_begin_handler *possible_handlers_end = handler + crex_observers_fcall_list.count;

	crex_observer_fcall_end_handler *end_handler = (crex_observer_fcall_end_handler *)possible_handlers_end;
	if (*end_handler != CREX_OBSERVER_NOT_OBSERVED) {
		*prev_observed_frame(execute_data) = current_observed_frame;
		current_observed_frame = execute_data;
	}

	if (*handler == CREX_OBSERVER_NOT_OBSERVED) {
		return;
	}

	do {
		(*handler)(execute_data);
	} while (++handler != possible_handlers_end && *handler != NULL);
}

CREX_API void CREX_FASTCALL crex_observer_generator_resume(crex_execute_data *execute_data)
{
	_crex_observe_fcall_begin(execute_data);
}

CREX_API void CREX_FASTCALL crex_observer_fcall_begin(crex_execute_data *execute_data)
{
	CREX_ASSUME(execute_data->func);
	if (!(execute_data->func->common.fn_flags & CREX_ACC_GENERATOR)) {
		_crex_observe_fcall_begin(execute_data);
	}
}

static inline void call_end_observers(crex_execute_data *execute_data, zval *return_value) {
	crex_function *func = execute_data->func;
	CREX_ASSERT(func);

	crex_observer_fcall_end_handler *handler = (crex_observer_fcall_end_handler *)&CREX_OBSERVER_DATA(func) + crex_observers_fcall_list.count;
	// TODO: Fix exceptions from generators
	// CREX_ASSERT(fcall_data);
	if (!*handler || *handler == CREX_OBSERVER_NOT_OBSERVED) {
		return;
	}

	crex_observer_fcall_end_handler *possible_handlers_end = handler + crex_observers_fcall_list.count;
	do {
		(*handler)(execute_data, return_value);
	} while (++handler != possible_handlers_end && *handler != NULL);
}

CREX_API void CREX_FASTCALL crex_observer_fcall_end(crex_execute_data *execute_data, zval *return_value)
{
	if (execute_data != current_observed_frame) {
		return;
	}
	call_end_observers(execute_data, return_value);
	current_observed_frame = *prev_observed_frame(execute_data);
}

CREX_API void crex_observer_fcall_end_all(void)
{
	crex_execute_data *execute_data = current_observed_frame, *original_execute_data = EG(current_execute_data);
	current_observed_frame = NULL;
	while (execute_data) {
		EG(current_execute_data) = execute_data;
		call_end_observers(execute_data, NULL);
		execute_data = *prev_observed_frame(execute_data);
	}
	EG(current_execute_data) = original_execute_data;
}

CREX_API void crex_observer_function_declared_register(crex_observer_function_declared_cb cb)
{
	crex_observer_function_declared_observed = true;
	crex_llist_add_element(&crex_observer_function_declared_callbacks, &cb);
}

CREX_API void CREX_FASTCALL _crex_observer_function_declared_notify(crex_op_array *op_array, crex_string *name)
{
	if (CG(compiler_options) & CREX_COMPILE_IGNORE_OBSERVER) {
		return;
	}

	for (crex_llist_element *element = crex_observer_function_declared_callbacks.head; element; element = element->next) {
		crex_observer_function_declared_cb callback = *(crex_observer_function_declared_cb *) (element->data);
		callback(op_array, name);
	}
}

CREX_API void crex_observer_class_linked_register(crex_observer_class_linked_cb cb)
{
	crex_observer_class_linked_observed = true;
	crex_llist_add_element(&crex_observer_class_linked_callbacks, &cb);
}

CREX_API void CREX_FASTCALL _crex_observer_class_linked_notify(crex_class_entry *ce, crex_string *name)
{
	if (CG(compiler_options) & CREX_COMPILE_IGNORE_OBSERVER) {
		return;
	}

	for (crex_llist_element *element = crex_observer_class_linked_callbacks.head; element; element = element->next) {
		crex_observer_class_linked_cb callback = *(crex_observer_class_linked_cb *) (element->data);
		callback(ce, name);
	}
}

CREX_API void crex_observer_error_register(crex_observer_error_cb cb)
{
	crex_observer_errors_observed = true;
	crex_llist_add_element(&crex_observer_error_callbacks, &cb);
}

CREX_API void _crex_observer_error_notify(int type, crex_string *error_filename, uint32_t error_lineno, crex_string *message)
{
	for (crex_llist_element *element = crex_observer_error_callbacks.head; element; element = element->next) {
		crex_observer_error_cb callback = *(crex_observer_error_cb *) (element->data);
		callback(type, error_filename, error_lineno, message);
	}
}

CREX_API void crex_observer_fiber_init_register(crex_observer_fiber_init_handler handler)
{
	crex_llist_add_element(&crex_observer_fiber_init, &handler);
}

CREX_API void crex_observer_fiber_switch_register(crex_observer_fiber_switch_handler handler)
{
	crex_llist_add_element(&crex_observer_fiber_switch, &handler);
}

CREX_API void crex_observer_fiber_destroy_register(crex_observer_fiber_destroy_handler handler)
{
	crex_llist_add_element(&crex_observer_fiber_destroy, &handler);
}

CREX_API void CREX_FASTCALL crex_observer_fiber_init_notify(crex_fiber_context *initializing)
{
	crex_llist_element *element;
	crex_observer_fiber_init_handler callback;

	initializing->top_observed_frame = NULL;

	for (element = crex_observer_fiber_init.head; element; element = element->next) {
		callback = *(crex_observer_fiber_init_handler *) element->data;
		callback(initializing);
	}
}

CREX_API void CREX_FASTCALL crex_observer_fiber_switch_notify(crex_fiber_context *from, crex_fiber_context *to)
{
	crex_llist_element *element;
	crex_observer_fiber_switch_handler callback;

	if (from->status == CREX_FIBER_STATUS_DEAD) {
		crex_observer_fcall_end_all(); // fiber is either finished (call will do nothing) or has bailed out
	}

	for (element = crex_observer_fiber_switch.head; element; element = element->next) {
		callback = *(crex_observer_fiber_switch_handler *) element->data;
		callback(from, to);
	}

	from->top_observed_frame = current_observed_frame;
	current_observed_frame = to->top_observed_frame;
}

CREX_API void CREX_FASTCALL crex_observer_fiber_destroy_notify(crex_fiber_context *destroying)
{
	crex_llist_element *element;
	crex_observer_fiber_destroy_handler callback;

	for (element = crex_observer_fiber_destroy.head; element; element = element->next) {
		callback = *(crex_observer_fiber_destroy_handler *) element->data;
		callback(destroying);
	}
}
