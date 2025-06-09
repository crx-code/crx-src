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

#ifndef CREX_OBSERVER_H
#define CREX_OBSERVER_H

#include "crex.h"
#include "crex_compile.h"
#include "crex_fibers.h"

BEGIN_EXTERN_C()

extern CREX_API int crex_observer_fcall_op_array_extension;
extern CREX_API bool crex_observer_errors_observed;
extern CREX_API bool crex_observer_function_declared_observed;
extern CREX_API bool crex_observer_class_linked_observed;

#define CREX_OBSERVER_ENABLED (crex_observer_fcall_op_array_extension != -1)

#define CREX_OBSERVER_FCALL_BEGIN(execute_data) do { \
		if (CREX_OBSERVER_ENABLED) { \
			crex_observer_fcall_begin(execute_data); \
		} \
	} while (0)

#define CREX_OBSERVER_FCALL_END(execute_data, return_value) do { \
		if (CREX_OBSERVER_ENABLED) { \
			crex_observer_fcall_end(execute_data, return_value); \
		} \
	} while (0)

typedef void (*crex_observer_fcall_begin_handler)(crex_execute_data *execute_data);
typedef void (*crex_observer_fcall_end_handler)(crex_execute_data *execute_data, zval *retval);

typedef struct _crex_observer_fcall_handlers {
	crex_observer_fcall_begin_handler begin;
	crex_observer_fcall_end_handler end;
} crex_observer_fcall_handlers;

/* If the fn should not be observed then return {NULL, NULL} */
typedef crex_observer_fcall_handlers (*crex_observer_fcall_init)(crex_execute_data *execute_data);

// Call during minit/startup ONLY
CREX_API void crex_observer_fcall_register(crex_observer_fcall_init);

// Call during runtime, but only if you have used crex_observer_fcall_register.
// You must not have more than one begin and one end handler active at the same time. Remove the old one first, if there is an existing one.
CREX_API void crex_observer_add_begin_handler(crex_function *function, crex_observer_fcall_begin_handler begin);
CREX_API bool crex_observer_remove_begin_handler(crex_function *function, crex_observer_fcall_begin_handler begin);
CREX_API void crex_observer_add_end_handler(crex_function *function, crex_observer_fcall_end_handler end);
CREX_API bool crex_observer_remove_end_handler(crex_function *function, crex_observer_fcall_end_handler end);

CREX_API void crex_observer_startup(void); // Called by engine before MINITs
CREX_API void crex_observer_post_startup(void); // Called by engine after MINITs
CREX_API void crex_observer_activate(void);
CREX_API void crex_observer_shutdown(void);

CREX_API void CREX_FASTCALL crex_observer_fcall_begin(
	crex_execute_data *execute_data);

CREX_API void CREX_FASTCALL crex_observer_generator_resume(
	crex_execute_data *execute_data);

CREX_API void CREX_FASTCALL crex_observer_fcall_end(
	crex_execute_data *execute_data,
	zval *return_value);

CREX_API void crex_observer_fcall_end_all(void);

typedef void (*crex_observer_function_declared_cb)(crex_op_array *op_array, crex_string *name);

CREX_API void crex_observer_function_declared_register(crex_observer_function_declared_cb cb);
CREX_API void CREX_FASTCALL _crex_observer_function_declared_notify(crex_op_array *op_array, crex_string *name);
static inline void crex_observer_function_declared_notify(crex_op_array *op_array, crex_string *name) {
    if (UNEXPECTED(crex_observer_function_declared_observed)) {
		_crex_observer_function_declared_notify(op_array, name);
	}
}

typedef void (*crex_observer_class_linked_cb)(crex_class_entry *ce, crex_string *name);

CREX_API void crex_observer_class_linked_register(crex_observer_class_linked_cb cb);
CREX_API void CREX_FASTCALL _crex_observer_class_linked_notify(crex_class_entry *ce, crex_string *name);
static inline void crex_observer_class_linked_notify(crex_class_entry *ce, crex_string *name) {
	if (UNEXPECTED(crex_observer_class_linked_observed)) {
		_crex_observer_class_linked_notify(ce, name);
	}
}

typedef void (*crex_observer_error_cb)(int type, crex_string *error_filename, uint32_t error_lineno, crex_string *message);

CREX_API void crex_observer_error_register(crex_observer_error_cb callback);
CREX_API void _crex_observer_error_notify(int type, crex_string *error_filename, uint32_t error_lineno, crex_string *message);
static inline void crex_observer_error_notify(int type, crex_string *error_filename, uint32_t error_lineno, crex_string *message) {
	if (UNEXPECTED(crex_observer_errors_observed)) {
		_crex_observer_error_notify(type, error_filename, error_lineno, message);
	}
}

typedef void (*crex_observer_fiber_init_handler)(crex_fiber_context *initializing);
typedef void (*crex_observer_fiber_switch_handler)(crex_fiber_context *from, crex_fiber_context *to);
typedef void (*crex_observer_fiber_destroy_handler)(crex_fiber_context *destroying);

CREX_API void crex_observer_fiber_init_register(crex_observer_fiber_init_handler handler);
CREX_API void crex_observer_fiber_switch_register(crex_observer_fiber_switch_handler handler);
CREX_API void crex_observer_fiber_destroy_register(crex_observer_fiber_destroy_handler handler);

CREX_API void CREX_FASTCALL crex_observer_fiber_init_notify(crex_fiber_context *initializing);
CREX_API void CREX_FASTCALL crex_observer_fiber_switch_notify(crex_fiber_context *from, crex_fiber_context *to);
CREX_API void CREX_FASTCALL crex_observer_fiber_destroy_notify(crex_fiber_context *destroying);

END_EXTERN_C()

#endif /* CREX_OBSERVER_H */
