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
   |          Marcus Boerger <helly@crx.net>                              |
   |          Sterling Hughes <sterling@crx.net>                          |
   |          Zeev Suraski <zeev@crx.net>                                 |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_EXCEPTIONS_H
#define CREX_EXCEPTIONS_H

BEGIN_EXTERN_C()

extern CREX_API crex_class_entry *crex_ce_throwable;
extern CREX_API crex_class_entry *crex_ce_exception;
extern CREX_API crex_class_entry *crex_ce_error_exception;
extern CREX_API crex_class_entry *crex_ce_error;
extern CREX_API crex_class_entry *crex_ce_compile_error;
extern CREX_API crex_class_entry *crex_ce_parse_error;
extern CREX_API crex_class_entry *crex_ce_type_error;
extern CREX_API crex_class_entry *crex_ce_argument_count_error;
extern CREX_API crex_class_entry *crex_ce_value_error;
extern CREX_API crex_class_entry *crex_ce_arithmetic_error;
extern CREX_API crex_class_entry *crex_ce_division_by_zero_error;
extern CREX_API crex_class_entry *crex_ce_unhandled_match_error;

CREX_API void crex_exception_set_previous(crex_object *exception, crex_object *add_previous);
CREX_API void crex_exception_save(void);
CREX_API void crex_exception_restore(void);

CREX_API CREX_COLD void crex_throw_exception_internal(crex_object *exception);

void crex_register_default_exception(void);

CREX_API crex_class_entry *crex_get_exception_base(crex_object *object);

/* Deprecated - Use crex_ce_exception directly instead */
CREX_API crex_class_entry *crex_exception_get_default(void);

/* Deprecated - Use crex_ce_error_exception directly instead */
CREX_API crex_class_entry *crex_get_error_exception(void);

CREX_API void crex_register_default_classes(void);

/* exception_ce   NULL, crex_ce_exception, crex_ce_error, or a derived class
 * message        NULL or the message of the exception */
CREX_API CREX_COLD crex_object *crex_throw_exception(crex_class_entry *exception_ce, const char *message, crex_long code);
CREX_API CREX_COLD crex_object *crex_throw_exception_ex(crex_class_entry *exception_ce, crex_long code, const char *format, ...) CREX_ATTRIBUTE_FORMAT(printf, 3, 4);
CREX_API CREX_COLD void crex_throw_exception_object(zval *exception);
CREX_API void crex_clear_exception(void);

CREX_API crex_object *crex_throw_error_exception(crex_class_entry *exception_ce, crex_string *message, crex_long code, int severity);

extern CREX_API void (*crex_throw_exception_hook)(crex_object *ex);

/* show an exception using crex_error(severity,...), severity should be E_ERROR */
CREX_API CREX_COLD crex_result crex_exception_error(crex_object *exception, int severity);
CREX_NORETURN void crex_exception_uncaught_error(const char *prefix, ...) CREX_ATTRIBUTE_FORMAT(printf, 1, 2);
CREX_API crex_string *crex_trace_to_string(HashTable *trace, bool include_main);

CREX_API CREX_COLD crex_object *crex_create_unwind_exit(void);
CREX_API CREX_COLD crex_object *crex_create_graceful_exit(void);
CREX_API CREX_COLD void crex_throw_unwind_exit(void);
CREX_API CREX_COLD void crex_throw_graceful_exit(void);
CREX_API bool crex_is_unwind_exit(const crex_object *ex);
CREX_API bool crex_is_graceful_exit(const crex_object *ex);

#include "crex_globals.h"

static crex_always_inline void crex_rethrow_exception(crex_execute_data *execute_data)
{
	if (EX(opline)->opcode != CREX_HANDLE_EXCEPTION) {
		EG(opline_before_exception) = EX(opline);
		EX(opline) = EG(exception_op);
	}
}

END_EXTERN_C()

#endif
