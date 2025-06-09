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

#include "crex.h"
#include "crex_API.h"
#include "crex_builtin_functions.h"
#include "crex_interfaces.h"
#include "crex_exceptions.h"
#include "crex_vm.h"
#include "crex_dtrace.h"
#include "crex_smart_str.h"
#include "crex_exceptions_arginfo.h"
#include "crex_observer.h"

CREX_API crex_class_entry *crex_ce_throwable;
CREX_API crex_class_entry *crex_ce_exception;
CREX_API crex_class_entry *crex_ce_error_exception;
CREX_API crex_class_entry *crex_ce_error;
CREX_API crex_class_entry *crex_ce_compile_error;
CREX_API crex_class_entry *crex_ce_parse_error;
CREX_API crex_class_entry *crex_ce_type_error;
CREX_API crex_class_entry *crex_ce_argument_count_error;
CREX_API crex_class_entry *crex_ce_value_error;
CREX_API crex_class_entry *crex_ce_arithmetic_error;
CREX_API crex_class_entry *crex_ce_division_by_zero_error;
CREX_API crex_class_entry *crex_ce_unhandled_match_error;

/* Internal pseudo-exception that is not exposed to userland. Throwing this exception *does not* execute finally blocks. */
static crex_class_entry crex_ce_unwind_exit;

/* Internal pseudo-exception that is not exposed to userland. Throwing this exception *does* execute finally blocks. */
static crex_class_entry crex_ce_graceful_exit;

CREX_API void (*crex_throw_exception_hook)(crex_object *ex);

static crex_object_handlers default_exception_handlers;

/* {{{ crex_implement_throwable */
static int crex_implement_throwable(crex_class_entry *interface, crex_class_entry *class_type)
{
	/* crex_ce_exception and crex_ce_error may not be initialized yet when this is called (e.g when
	 * implementing Throwable for Exception itself). Perform a manual inheritance check. */
	crex_class_entry *root = class_type;
	while (root->parent) {
		root = root->parent;
	}
	if (crex_string_equals_literal(root->name, "Exception")
			|| crex_string_equals_literal(root->name, "Error")) {
		return SUCCESS;
	}

	bool can_extend = (class_type->ce_flags & CREX_ACC_ENUM) == 0;

	crex_error_noreturn(E_ERROR,
		can_extend
			? "%s %s cannot implement interface %s, extend Exception or Error instead"
			: "%s %s cannot implement interface %s",
		crex_get_object_type_uc(class_type),
		ZSTR_VAL(class_type->name),
		ZSTR_VAL(interface->name));
	return FAILURE;
}
/* }}} */

static inline crex_class_entry *i_get_exception_base(crex_object *object) /* {{{ */
{
	return instanceof_function(object->ce, crex_ce_exception) ? crex_ce_exception : crex_ce_error;
}
/* }}} */

CREX_API crex_class_entry *crex_get_exception_base(crex_object *object) /* {{{ */
{
	return i_get_exception_base(object);
}
/* }}} */

void crex_exception_set_previous(crex_object *exception, crex_object *add_previous) /* {{{ */
{
	zval *previous, *ancestor, *ex;
	zval  pv, zv, rv;
	crex_class_entry *base_ce;

	if (!exception || !add_previous) {
		return;
	}

	if (exception == add_previous || crex_is_unwind_exit(add_previous) || crex_is_graceful_exit(add_previous)) {
		OBJ_RELEASE(add_previous);
		return;
	}

	CREX_ASSERT(instanceof_function(add_previous->ce, crex_ce_throwable)
		&& "Previous exception must implement Throwable");

	ZVAL_OBJ(&pv, add_previous);
	ZVAL_OBJ(&zv, exception);
	ex = &zv;
	do {
		ancestor = crex_read_property_ex(i_get_exception_base(add_previous), add_previous, ZSTR_KNOWN(CREX_STR_PREVIOUS), 1, &rv);
		while (C_TYPE_P(ancestor) == IS_OBJECT) {
			if (C_OBJ_P(ancestor) == C_OBJ_P(ex)) {
				OBJ_RELEASE(add_previous);
				return;
			}
			ancestor = crex_read_property_ex(i_get_exception_base(C_OBJ_P(ancestor)), C_OBJ_P(ancestor), ZSTR_KNOWN(CREX_STR_PREVIOUS), 1, &rv);
		}
		base_ce = i_get_exception_base(C_OBJ_P(ex));
		previous = crex_read_property_ex(base_ce, C_OBJ_P(ex), ZSTR_KNOWN(CREX_STR_PREVIOUS), 1, &rv);
		if (C_TYPE_P(previous) == IS_NULL) {
			crex_update_property_ex(base_ce, C_OBJ_P(ex), ZSTR_KNOWN(CREX_STR_PREVIOUS), &pv);
			GC_DELREF(add_previous);
			return;
		}
		ex = previous;
	} while (C_OBJ_P(ex) != add_previous);
}
/* }}} */

void crex_exception_save(void) /* {{{ */
{
	if (EG(prev_exception)) {
		crex_exception_set_previous(EG(exception), EG(prev_exception));
	}
	if (EG(exception)) {
		EG(prev_exception) = EG(exception);
	}
	EG(exception) = NULL;
}
/* }}} */

void crex_exception_restore(void) /* {{{ */
{
	if (EG(prev_exception)) {
		if (EG(exception)) {
			crex_exception_set_previous(EG(exception), EG(prev_exception));
		} else {
			EG(exception) = EG(prev_exception);
		}
		EG(prev_exception) = NULL;
	}
}
/* }}} */

static crex_always_inline bool is_handle_exception_set(void) {
	crex_execute_data *execute_data = EG(current_execute_data);
	return !execute_data
		|| !execute_data->func
		|| !CREX_USER_CODE(execute_data->func->common.type)
		|| execute_data->opline->opcode == CREX_HANDLE_EXCEPTION;
}

CREX_API CREX_COLD void crex_throw_exception_internal(crex_object *exception) /* {{{ */
{
#ifdef HAVE_DTRACE
	if (DTRACE_EXCEPTION_THROWN_ENABLED()) {
		if (exception != NULL) {
			DTRACE_EXCEPTION_THROWN(ZSTR_VAL(exception->ce->name));
		} else {
			DTRACE_EXCEPTION_THROWN(NULL);
		}
	}
#endif /* HAVE_DTRACE */

	if (exception != NULL) {
		crex_object *previous = EG(exception);
		if (previous && crex_is_unwind_exit(previous)) {
			/* Don't replace unwinding exception with different exception. */
			OBJ_RELEASE(exception);
			return;
		}

		crex_exception_set_previous(exception, EG(exception));
		EG(exception) = exception;
		if (previous) {
			CREX_ASSERT(is_handle_exception_set() && "HANDLE_EXCEPTION not set?");
			return;
		}
	}
	if (!EG(current_execute_data)) {
		if (exception && (exception->ce == crex_ce_parse_error || exception->ce == crex_ce_compile_error)) {
			return;
		}
		if (EG(exception)) {
			if (C_TYPE(EG(user_exception_handler)) != IS_UNDEF
			 && !crex_is_unwind_exit(EG(exception))
			 && !crex_is_graceful_exit(EG(exception))) {
				crex_user_exception_handler();
				if (EG(exception)) {
					crex_exception_error(EG(exception), E_ERROR);
				}
				return;
			} else {
				crex_exception_error(EG(exception), E_ERROR);
			}
			crex_bailout();
		}
		crex_error_noreturn(E_CORE_ERROR, "Exception thrown without a stack frame");
	}

	if (crex_throw_exception_hook) {
		crex_throw_exception_hook(exception);
	}

	if (is_handle_exception_set()) {
		/* no need to rethrow the exception */
		return;
	}
	EG(opline_before_exception) = EG(current_execute_data)->opline;
	EG(current_execute_data)->opline = EG(exception_op);
}
/* }}} */

CREX_API void crex_clear_exception(void) /* {{{ */
{
	crex_object *exception;
	if (EG(prev_exception)) {
		OBJ_RELEASE(EG(prev_exception));
		EG(prev_exception) = NULL;
	}
	if (!EG(exception)) {
		return;
	}
	/* exception may have destructor */
	exception = EG(exception);
	EG(exception) = NULL;
	OBJ_RELEASE(exception);
	if (EG(current_execute_data)) {
		EG(current_execute_data)->opline = EG(opline_before_exception);
	}
#if CREX_DEBUG
	EG(opline_before_exception) = NULL;
#endif
}
/* }}} */

static crex_object *crex_default_exception_new(crex_class_entry *class_type) /* {{{ */
{
	zval tmp;
	zval trace;
	crex_class_entry *base_ce;
	crex_string *filename;

	crex_object *object = crex_objects_new(class_type);
	object_properties_init(object, class_type);

	if (EG(current_execute_data)) {
		crex_fetch_debug_backtrace(&trace,
			0,
			EG(exception_ignore_args) ? DEBUG_BACKTRACE_IGNORE_ARGS : 0, 0);
	} else {
		array_init(&trace);
	}
	C_SET_REFCOUNT(trace, 0);

	base_ce = i_get_exception_base(object);

	if (EXPECTED((class_type != crex_ce_parse_error && class_type != crex_ce_compile_error)
			|| !(filename = crex_get_compiled_filename()))) {
		ZVAL_STRING(&tmp, crex_get_executed_filename());
		crex_update_property_ex(base_ce, object, ZSTR_KNOWN(CREX_STR_FILE), &tmp);
		zval_ptr_dtor(&tmp);
		ZVAL_LONG(&tmp, crex_get_executed_lineno());
		crex_update_property_ex(base_ce, object, ZSTR_KNOWN(CREX_STR_LINE), &tmp);
	} else {
		ZVAL_STR(&tmp, filename);
		crex_update_property_ex(base_ce, object, ZSTR_KNOWN(CREX_STR_FILE), &tmp);
		ZVAL_LONG(&tmp, crex_get_compiled_lineno());
		crex_update_property_ex(base_ce, object, ZSTR_KNOWN(CREX_STR_LINE), &tmp);
	}
	crex_update_property_ex(base_ce, object, ZSTR_KNOWN(CREX_STR_TRACE), &trace);

	return object;
}
/* }}} */

/* {{{ Clone the exception object */
CREX_COLD CREX_METHOD(Exception, __clone)
{
	/* Should never be executable */
	crex_throw_exception(NULL, "Cannot clone object using __clone()", 0);
}
/* }}} */

/* {{{ Exception constructor */
CREX_METHOD(Exception, __main)
{
	crex_string *message = NULL;
	crex_long   code = 0;
	zval  tmp, *object, *previous = NULL;
	crex_class_entry *base_ce;

	object = CREX_THIS;
	base_ce = i_get_exception_base(C_OBJ_P(object));

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|SlO!", &message, &code, &previous, crex_ce_throwable) == FAILURE) {
		RETURN_THROWS();
	}

	if (message) {
		ZVAL_STR(&tmp, message);
		crex_update_property_ex(base_ce, C_OBJ_P(object), ZSTR_KNOWN(CREX_STR_MESSAGE), &tmp);
	}

	if (code) {
		ZVAL_LONG(&tmp, code);
		crex_update_property_ex(base_ce, C_OBJ_P(object), ZSTR_KNOWN(CREX_STR_CODE), &tmp);
	}

	if (previous) {
		crex_update_property_ex(base_ce, C_OBJ_P(object), ZSTR_KNOWN(CREX_STR_PREVIOUS), previous);
	}
}
/* }}} */

/* {{{ Exception unserialize checks */
#define CHECK_EXC_TYPE(id, type) \
	pvalue = crex_read_property_ex(i_get_exception_base(C_OBJ_P(object)), C_OBJ_P(object), ZSTR_KNOWN(id), 1, &value); \
	if (C_TYPE_P(pvalue) != IS_NULL && C_TYPE_P(pvalue) != type) { \
		crex_unset_property(i_get_exception_base(C_OBJ_P(object)), C_OBJ_P(object), ZSTR_VAL(ZSTR_KNOWN(id)), ZSTR_LEN(ZSTR_KNOWN(id))); \
	}

CREX_METHOD(Exception, __wakeup)
{
	CREX_PARSE_PARAMETERS_NONE();

	zval value, *pvalue;
	zval *object = CREX_THIS;
	CHECK_EXC_TYPE(CREX_STR_MESSAGE, IS_STRING);
	CHECK_EXC_TYPE(CREX_STR_CODE,    IS_LONG);
	/* The type of all other properties is enforced through typed properties. */
}
/* }}} */

/* {{{ ErrorException constructor */
CREX_METHOD(ErrorException, __main)
{
	crex_string *message = NULL, *filename = NULL;
	crex_long   code = 0, severity = E_ERROR, lineno;
	bool lineno_is_null = 1;
	zval   tmp, *object, *previous = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|SllS!l!O!", &message, &code, &severity, &filename, &lineno, &lineno_is_null, &previous, crex_ce_throwable) == FAILURE) {
		RETURN_THROWS();
	}

	object = CREX_THIS;

	if (message) {
		ZVAL_STR_COPY(&tmp, message);
		crex_update_property_ex(crex_ce_exception, C_OBJ_P(object), ZSTR_KNOWN(CREX_STR_MESSAGE), &tmp);
		zval_ptr_dtor(&tmp);
	}

	if (code) {
		ZVAL_LONG(&tmp, code);
		crex_update_property_ex(crex_ce_exception, C_OBJ_P(object), ZSTR_KNOWN(CREX_STR_CODE), &tmp);
	}

	if (previous) {
		crex_update_property_ex(crex_ce_exception, C_OBJ_P(object), ZSTR_KNOWN(CREX_STR_PREVIOUS), previous);
	}

	ZVAL_LONG(&tmp, severity);
	crex_update_property_ex(crex_ce_exception, C_OBJ_P(object), ZSTR_KNOWN(CREX_STR_SEVERITY), &tmp);

	if (filename) {
		ZVAL_STR_COPY(&tmp, filename);
		crex_update_property_ex(crex_ce_exception, C_OBJ_P(object), ZSTR_KNOWN(CREX_STR_FILE), &tmp);
		zval_ptr_dtor(&tmp);
	}

	if (!lineno_is_null) {
		ZVAL_LONG(&tmp, lineno);
		crex_update_property_ex(crex_ce_exception, C_OBJ_P(object), ZSTR_KNOWN(CREX_STR_LINE), &tmp);
	} else if (filename) {
		ZVAL_LONG(&tmp, 0);
		crex_update_property_ex(crex_ce_exception, C_OBJ_P(object), ZSTR_KNOWN(CREX_STR_LINE), &tmp);
	}
}
/* }}} */

#define GET_PROPERTY(object, id) \
	crex_read_property_ex(i_get_exception_base(C_OBJ_P(object)), C_OBJ_P(object), ZSTR_KNOWN(id), 0, &rv)
#define GET_PROPERTY_SILENT(object, id) \
	crex_read_property_ex(i_get_exception_base(C_OBJ_P(object)), C_OBJ_P(object), ZSTR_KNOWN(id), 1, &rv)

/* {{{ Get the file in which the exception occurred */
CREX_METHOD(Exception, getFile)
{
	zval *prop, rv;

	CREX_PARSE_PARAMETERS_NONE();

	prop = GET_PROPERTY(CREX_THIS, CREX_STR_FILE);
	RETURN_STR(zval_get_string(prop));
}
/* }}} */

/* {{{ Get the line in which the exception occurred */
CREX_METHOD(Exception, getLine)
{
	zval *prop, rv;

	CREX_PARSE_PARAMETERS_NONE();

	prop = GET_PROPERTY(CREX_THIS, CREX_STR_LINE);
	RETURN_LONG(zval_get_long(prop));
}
/* }}} */

/* {{{ Get the exception message */
CREX_METHOD(Exception, getMessage)
{
	zval *prop, rv;

	CREX_PARSE_PARAMETERS_NONE();

	prop = GET_PROPERTY(CREX_THIS, CREX_STR_MESSAGE);
	RETURN_STR(zval_get_string(prop));
}
/* }}} */

/* {{{ Get the exception code */
CREX_METHOD(Exception, getCode)
{
	zval *prop, rv;

	CREX_PARSE_PARAMETERS_NONE();

	prop = GET_PROPERTY(CREX_THIS, CREX_STR_CODE);
	ZVAL_DEREF(prop);
	ZVAL_COPY(return_value, prop);
}
/* }}} */

/* {{{ Get the stack trace for the location in which the exception occurred */
CREX_METHOD(Exception, getTrace)
{
	zval *prop, rv;

	CREX_PARSE_PARAMETERS_NONE();

	prop = GET_PROPERTY(CREX_THIS, CREX_STR_TRACE);
	ZVAL_DEREF(prop);
	ZVAL_COPY(return_value, prop);
}
/* }}} */

/* {{{ Get the exception severity */
CREX_METHOD(ErrorException, getSeverity)
{
	zval *prop, rv;

	CREX_PARSE_PARAMETERS_NONE();

	prop = GET_PROPERTY(CREX_THIS, CREX_STR_SEVERITY);
	ZVAL_DEREF(prop);
	ZVAL_COPY(return_value, prop);
}
/* }}} */

#define TRACE_APPEND_KEY(key) do {                                          \
		tmp = crex_hash_find(ht, key);                                      \
		if (tmp) {                                                          \
			if (C_TYPE_P(tmp) != IS_STRING) {                               \
				crex_error(E_WARNING, "Value for %s is not a string",       \
					ZSTR_VAL(key));                                         \
				smart_str_appends(str, "[unknown]");                        \
			} else {                                                        \
				smart_str_appends(str, C_STRVAL_P(tmp));                    \
			}                                                               \
		} \
	} while (0)

static void _build_trace_args(zval *arg, smart_str *str) /* {{{ */
{
	/* the trivial way would be to do
	 * convert_to_string(arg);
	 * append it and kill the now tmp arg.
	 * but that could cause some E_NOTICE and also damn long lines.
	 */

	ZVAL_DEREF(arg);

	if (C_TYPE_P(arg) <= IS_STRING) {
		smart_str_append_scalar(str, arg, EG(exception_string_param_max_len));
		smart_str_appends(str, ", ");
	} else {
		switch (C_TYPE_P(arg)) {
			case IS_RESOURCE:
				smart_str_appends(str, "Resource id #");
				smart_str_append_long(str, C_RES_HANDLE_P(arg));
				smart_str_appends(str, ", ");
				break;
			case IS_ARRAY:
				smart_str_appends(str, "Array, ");
				break;
			case IS_OBJECT: {
				crex_string *class_name = C_OBJ_HANDLER_P(arg, get_class_name)(C_OBJ_P(arg));
				smart_str_appends(str, "Object(");
				smart_str_appends(str, ZSTR_VAL(class_name));
				smart_str_appends(str, "), ");
				crex_string_release_ex(class_name, 0);
				break;
			}
		}
	}
}
/* }}} */

static void _build_trace_string(smart_str *str, HashTable *ht, uint32_t num) /* {{{ */
{
	zval *file, *tmp;

	smart_str_appendc(str, '#');
	smart_str_append_long(str, num);
	smart_str_appendc(str, ' ');

	file = crex_hash_find_known_hash(ht, ZSTR_KNOWN(CREX_STR_FILE));
	if (file) {
		if (C_TYPE_P(file) != IS_STRING) {
			crex_error(E_WARNING, "File name is not a string");
			smart_str_appends(str, "[unknown file]: ");
		} else{
			crex_long line = 0;
			tmp = crex_hash_find_known_hash(ht, ZSTR_KNOWN(CREX_STR_LINE));
			if (tmp) {
				if (C_TYPE_P(tmp) == IS_LONG) {
					line = C_LVAL_P(tmp);
				} else {
					crex_error(E_WARNING, "Line is not an int");
				}
			}
			smart_str_append(str, C_STR_P(file));
			smart_str_appendc(str, '(');
			smart_str_append_long(str, line);
			smart_str_appends(str, "): ");
		}
	} else {
		smart_str_appends(str, "[internal function]: ");
	}
	TRACE_APPEND_KEY(ZSTR_KNOWN(CREX_STR_CLASS));
	TRACE_APPEND_KEY(ZSTR_KNOWN(CREX_STR_TYPE));
	TRACE_APPEND_KEY(ZSTR_KNOWN(CREX_STR_FUNCTION));
	smart_str_appendc(str, '(');
	tmp = crex_hash_find_known_hash(ht, ZSTR_KNOWN(CREX_STR_ARGS));
	if (tmp) {
		if (C_TYPE_P(tmp) == IS_ARRAY) {
			size_t last_len = ZSTR_LEN(str->s);
			crex_string *name;
			zval *arg;

			CREX_HASH_FOREACH_STR_KEY_VAL(C_ARRVAL_P(tmp), name, arg) {
				if (name) {
					smart_str_append(str, name);
					smart_str_appends(str, ": ");
				}
				_build_trace_args(arg, str);
			} CREX_HASH_FOREACH_END();

			if (last_len != ZSTR_LEN(str->s)) {
				ZSTR_LEN(str->s) -= 2; /* remove last ', ' */
			}
		} else {
			crex_error(E_WARNING, "args element is not an array");
		}
	}
	smart_str_appends(str, ")\n");
}
/* }}} */

CREX_API crex_string *crex_trace_to_string(HashTable *trace, bool include_main) {
	crex_ulong index;
	zval *frame;
	uint32_t num = 0;
	smart_str str = {0};

	CREX_HASH_FOREACH_NUM_KEY_VAL(trace, index, frame) {
		if (C_TYPE_P(frame) != IS_ARRAY) {
			crex_error(E_WARNING, "Expected array for frame " CREX_ULONG_FMT, index);
			continue;
		}

		_build_trace_string(&str, C_ARRVAL_P(frame), num++);
	} CREX_HASH_FOREACH_END();

	if (include_main) {
		smart_str_appendc(&str, '#');
		smart_str_append_long(&str, num);
		smart_str_appends(&str, " {main}");
	}

	smart_str_0(&str);
	return str.s ? str.s : ZSTR_EMPTY_ALLOC();
}

/* {{{ Obtain the backtrace for the exception as a string (instead of an array) */
CREX_METHOD(Exception, getTraceAsString)
{

	CREX_PARSE_PARAMETERS_NONE();

	zval *object = CREX_THIS;
	crex_class_entry *base_ce = i_get_exception_base(C_OBJ_P(object));
	zval rv;
	zval *trace = crex_read_property_ex(base_ce, C_OBJ_P(object), ZSTR_KNOWN(CREX_STR_TRACE), 1, &rv);
	if (EG(exception)) {
		RETURN_THROWS();
	}

	/* Type should be guaranteed by property type. */
	CREX_ASSERT(C_TYPE_P(trace) == IS_ARRAY);
	RETURN_NEW_STR(crex_trace_to_string(C_ARRVAL_P(trace), /* include_main */ true));
}
/* }}} */

/* {{{ Return previous Throwable or NULL. */
CREX_METHOD(Exception, getPrevious)
{
	zval rv;

	CREX_PARSE_PARAMETERS_NONE();

	ZVAL_COPY(return_value, GET_PROPERTY_SILENT(CREX_THIS, CREX_STR_PREVIOUS));
} /* }}} */

/* {{{ Obtain the string representation of the Exception object */
CREX_METHOD(Exception, __toString)
{
	zval trace, *exception;
	crex_class_entry *base_ce;
	crex_string *str;
	crex_fcall_info fci;
	zval rv, tmp;
	crex_string *fname;

	CREX_PARSE_PARAMETERS_NONE();

	str = ZSTR_EMPTY_ALLOC();

	exception = CREX_THIS;
	fname = ZSTR_INIT_LITERAL("gettraceasstring", 0);

	while (exception && C_TYPE_P(exception) == IS_OBJECT && instanceof_function(C_OBJCE_P(exception), crex_ce_throwable)) {
		crex_string *prev_str = str;
		crex_string *message = zval_get_string(GET_PROPERTY(exception, CREX_STR_MESSAGE));
		crex_string *file = zval_get_string(GET_PROPERTY(exception, CREX_STR_FILE));
		crex_long line = zval_get_long(GET_PROPERTY(exception, CREX_STR_LINE));

		fci.size = sizeof(fci);
		ZVAL_STR(&fci.function_name, fname);
		fci.object = C_OBJ_P(exception);
		fci.retval = &trace;
		fci.param_count = 0;
		fci.params = NULL;
		fci.named_params = NULL;

		crex_call_function(&fci, NULL);

		if (C_TYPE(trace) != IS_STRING) {
			zval_ptr_dtor(&trace);
			ZVAL_UNDEF(&trace);
		}

		if ((C_OBJCE_P(exception) == crex_ce_type_error || C_OBJCE_P(exception) == crex_ce_argument_count_error) && strstr(ZSTR_VAL(message), ", called in ")) {
			zval message_zv;
			ZVAL_STR(&message_zv, message);
			crex_string *real_message = crex_strpprintf_unchecked(0, "%Z and defined", &message_zv);
			crex_string_release_ex(message, 0);
			message = real_message;
		}

		crex_string *tmp_trace = (C_TYPE(trace) == IS_STRING && C_STRLEN(trace))
			? crex_string_copy(C_STR(trace))
			: ZSTR_INIT_LITERAL("#0 {main}\n", false);

		zval name_zv, trace_zv, file_zv, prev_str_zv;
		ZVAL_STR(&name_zv, C_OBJCE_P(exception)->name);
		ZVAL_STR(&trace_zv, tmp_trace);
		ZVAL_STR(&file_zv, file);
		ZVAL_STR(&prev_str_zv, prev_str);

		if (ZSTR_LEN(message) > 0) {
			zval message_zv;
			ZVAL_STR(&message_zv, message);

			str = crex_strpprintf_unchecked(0, "%Z: %Z in %Z:" CREX_LONG_FMT "\nStack trace:\n%Z%s%Z",
				&name_zv, &message_zv, &file_zv, line,
				&trace_zv, ZSTR_LEN(prev_str) ? "\n\nNext " : "", &prev_str_zv);
		} else {
			str = crex_strpprintf_unchecked(0, "%Z in %Z:" CREX_LONG_FMT "\nStack trace:\n%Z%s%Z",
				&name_zv, &file_zv, line,
				&trace_zv, ZSTR_LEN(prev_str) ? "\n\nNext " : "", &prev_str_zv);
		}
		crex_string_release_ex(tmp_trace, false);

		crex_string_release_ex(prev_str, 0);
		crex_string_release_ex(message, 0);
		crex_string_release_ex(file, 0);
		zval_ptr_dtor(&trace);

		C_PROTECT_RECURSION_P(exception);
		exception = GET_PROPERTY(exception, CREX_STR_PREVIOUS);
		if (exception && C_TYPE_P(exception) == IS_OBJECT && C_IS_RECURSIVE_P(exception)) {
			break;
		}
	}
	crex_string_release_ex(fname, 0);

	exception = CREX_THIS;
	/* Reset apply counts */
	while (exception && C_TYPE_P(exception) == IS_OBJECT && (base_ce = i_get_exception_base(C_OBJ_P(exception))) && instanceof_function(C_OBJCE_P(exception), base_ce)) {
		if (C_IS_RECURSIVE_P(exception)) {
			C_UNPROTECT_RECURSION_P(exception);
		} else {
			break;
		}
		exception = GET_PROPERTY(exception, CREX_STR_PREVIOUS);
	}

	exception = CREX_THIS;
	base_ce = i_get_exception_base(C_OBJ_P(exception));

	/* We store the result in the private property string so we can access
	 * the result in uncaught exception handlers without memleaks. */
	ZVAL_STR(&tmp, str);
	crex_update_property_ex(base_ce, C_OBJ_P(exception), ZSTR_KNOWN(CREX_STR_STRING), &tmp);

	RETURN_STR(str);
}
/* }}} */

static void crex_init_exception_class_entry(crex_class_entry *ce) {
	ce->create_object = crex_default_exception_new;
	ce->default_object_handlers = &default_exception_handlers;
}

void crex_register_default_exception(void) /* {{{ */
{
	crex_ce_throwable = register_class_Throwable(crex_ce_stringable);
	crex_ce_throwable->interface_gets_implemented = crex_implement_throwable;

	memcpy(&default_exception_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	default_exception_handlers.clone_obj = NULL;

	crex_ce_exception = register_class_Exception(crex_ce_throwable);
	crex_init_exception_class_entry(crex_ce_exception);

	crex_ce_error_exception = register_class_ErrorException(crex_ce_exception);
	crex_init_exception_class_entry(crex_ce_error_exception);

	crex_ce_error = register_class_Error(crex_ce_throwable);
	crex_init_exception_class_entry(crex_ce_error);

	crex_ce_compile_error = register_class_CompileError(crex_ce_error);
	crex_init_exception_class_entry(crex_ce_compile_error);

	crex_ce_parse_error = register_class_ParseError(crex_ce_compile_error);
	crex_init_exception_class_entry(crex_ce_parse_error);

	crex_ce_type_error = register_class_TypeError(crex_ce_error);
	crex_init_exception_class_entry(crex_ce_type_error);

	crex_ce_argument_count_error = register_class_ArgumentCountError(crex_ce_type_error);
	crex_init_exception_class_entry(crex_ce_argument_count_error);

	crex_ce_value_error = register_class_ValueError(crex_ce_error);
	crex_init_exception_class_entry(crex_ce_value_error);

	crex_ce_arithmetic_error = register_class_ArithmeticError(crex_ce_error);
	crex_init_exception_class_entry(crex_ce_arithmetic_error);

	crex_ce_division_by_zero_error = register_class_DivisionByZeroError(crex_ce_arithmetic_error);
	crex_init_exception_class_entry(crex_ce_division_by_zero_error);

	crex_ce_unhandled_match_error = register_class_UnhandledMatchError(crex_ce_error);
	crex_init_exception_class_entry(crex_ce_unhandled_match_error);

	INIT_CLASS_ENTRY(crex_ce_unwind_exit, "UnwindExit", NULL);

	INIT_CLASS_ENTRY(crex_ce_graceful_exit, "GracefulExit", NULL);
}
/* }}} */

/* {{{ Deprecated - Use crex_ce_exception directly instead */
CREX_API crex_class_entry *crex_exception_get_default(void)
{
	return crex_ce_exception;
}
/* }}} */

/* {{{ Deprecated - Use crex_ce_error_exception directly instead */
CREX_API crex_class_entry *crex_get_error_exception(void)
{
	return crex_ce_error_exception;
}
/* }}} */

static crex_object *crex_throw_exception_zstr(crex_class_entry *exception_ce, crex_string *message, crex_long code) /* {{{ */
{
	zval ex, tmp;

	if (!exception_ce) {
		exception_ce = crex_ce_exception;
	}

	CREX_ASSERT(instanceof_function(exception_ce, crex_ce_throwable)
		&& "Exceptions must implement Throwable");

	object_init_ex(&ex, exception_ce);

	if (message) {
		ZVAL_STR(&tmp, message);
		crex_update_property_ex(exception_ce, C_OBJ(ex), ZSTR_KNOWN(CREX_STR_MESSAGE), &tmp);
	}
	if (code) {
		ZVAL_LONG(&tmp, code);
		crex_update_property_ex(exception_ce, C_OBJ(ex), ZSTR_KNOWN(CREX_STR_CODE), &tmp);
	}

	crex_throw_exception_internal(C_OBJ(ex));

	return C_OBJ(ex);
}
/* }}} */

CREX_API CREX_COLD crex_object *crex_throw_exception(crex_class_entry *exception_ce, const char *message, crex_long code) /* {{{ */
{
	crex_string *msg_str = message ? crex_string_init(message, strlen(message), 0) : NULL;
	crex_object *ex = crex_throw_exception_zstr(exception_ce, msg_str, code);
	if (msg_str) {
		crex_string_release(msg_str);
	}
	return ex;
}
/* }}} */

CREX_API CREX_COLD crex_object *crex_throw_exception_ex(crex_class_entry *exception_ce, crex_long code, const char *format, ...) /* {{{ */
{
	va_list arg;
	char *message;
	crex_object *obj;

	va_start(arg, format);
	crex_vspprintf(&message, 0, format, arg);
	va_end(arg);
	obj = crex_throw_exception(exception_ce, message, code);
	efree(message);
	return obj;
}
/* }}} */

CREX_API CREX_COLD crex_object *crex_throw_error_exception(crex_class_entry *exception_ce, crex_string *message, crex_long code, int severity) /* {{{ */
{
	crex_object *obj = crex_throw_exception_zstr(exception_ce, message, code);
	if (exception_ce && instanceof_function(exception_ce, crex_ce_error_exception)) {
		zval tmp;
		ZVAL_LONG(&tmp, severity);
		crex_update_property_ex(crex_ce_error_exception, obj, ZSTR_KNOWN(CREX_STR_SEVERITY), &tmp);
	}
	return obj;
}
/* }}} */

static void crex_error_va(int type, crex_string *file, uint32_t lineno, const char *format, ...) /* {{{ */
{
	va_list args;
	va_start(args, format);
	crex_string *message = crex_vstrpprintf(0, format, args);
	crex_observer_error_notify(type, file, lineno, message);
	crex_error_cb(type, file, lineno, message);
	crex_string_release(message);
	va_end(args);
}
/* }}} */

/* This function doesn't return if it uses E_ERROR */
CREX_API CREX_COLD crex_result crex_exception_error(crex_object *ex, int severity) /* {{{ */
{
	zval exception, rv;
	crex_class_entry *ce_exception;
	crex_result result = FAILURE;

	ZVAL_OBJ(&exception, ex);
	ce_exception = ex->ce;
	EG(exception) = NULL;
	if (ce_exception == crex_ce_parse_error || ce_exception == crex_ce_compile_error) {
		crex_string *message = zval_get_string(GET_PROPERTY(&exception, CREX_STR_MESSAGE));
		crex_string *file = zval_get_string(GET_PROPERTY_SILENT(&exception, CREX_STR_FILE));
		crex_long line = zval_get_long(GET_PROPERTY_SILENT(&exception, CREX_STR_LINE));
		int type = (ce_exception == crex_ce_parse_error ? E_PARSE : E_COMPILE_ERROR) | E_DONT_BAIL;

		crex_observer_error_notify(type, file, line, message);
		crex_error_cb(type, file, line, message);

		crex_string_release_ex(file, 0);
		crex_string_release_ex(message, 0);
	} else if (instanceof_function(ce_exception, crex_ce_throwable)) {
		zval tmp;
		crex_string *str, *file = NULL;
		crex_long line = 0;

		crex_call_known_instance_method_with_0_params(ex->ce->__tostring, ex, &tmp);
		if (!EG(exception)) {
			if (C_TYPE(tmp) != IS_STRING) {
				crex_error(E_WARNING, "%s::__toString() must return a string", ZSTR_VAL(ce_exception->name));
			} else {
				crex_update_property_ex(i_get_exception_base(ex), ex, ZSTR_KNOWN(CREX_STR_STRING), &tmp);
			}
		}
		zval_ptr_dtor(&tmp);

		if (EG(exception)) {
			zval zv;

			ZVAL_OBJ(&zv, EG(exception));
			/* do the best we can to inform about the inner exception */
			if (instanceof_function(ce_exception, crex_ce_exception) || instanceof_function(ce_exception, crex_ce_error)) {
				file = zval_get_string(GET_PROPERTY_SILENT(&zv, CREX_STR_FILE));
				line = zval_get_long(GET_PROPERTY_SILENT(&zv, CREX_STR_LINE));
			}

			crex_error_va(E_WARNING, (file && ZSTR_LEN(file) > 0) ? file : NULL, line,
				"Uncaught %s in exception handling during call to %s::__toString()",
				ZSTR_VAL(C_OBJCE(zv)->name), ZSTR_VAL(ce_exception->name));

			if (file) {
				crex_string_release_ex(file, 0);
			}
		}

		str = zval_get_string(GET_PROPERTY_SILENT(&exception, CREX_STR_STRING));
		file = zval_get_string(GET_PROPERTY_SILENT(&exception, CREX_STR_FILE));
		line = zval_get_long(GET_PROPERTY_SILENT(&exception, CREX_STR_LINE));

		crex_error_va(severity | E_DONT_BAIL,
			(file && ZSTR_LEN(file) > 0) ? file : NULL, line,
			"Uncaught %s\n  thrown", ZSTR_VAL(str));

		crex_string_release_ex(str, 0);
		crex_string_release_ex(file, 0);
	} else if (ce_exception == &crex_ce_unwind_exit || ce_exception == &crex_ce_graceful_exit) {
		/* We successfully unwound, nothing more to do.
		 * We still return FAILURE in this case, as further execution should still be aborted. */
	} else {
		crex_error(severity, "Uncaught exception %s", ZSTR_VAL(ce_exception->name));
	}

	OBJ_RELEASE(ex);
	return result;
}
/* }}} */

CREX_NORETURN void crex_exception_uncaught_error(const char *format, ...) {
	va_list va;
	va_start(va, format);
	crex_string *prefix = crex_vstrpprintf(0, format, va);
	va_end(va);

	CREX_ASSERT(EG(exception));
	zval exception_zv;
	ZVAL_OBJ_COPY(&exception_zv, EG(exception));
	crex_clear_exception();

	crex_string *exception_str = zval_get_string(&exception_zv);
	crex_error_noreturn(E_ERROR,
		"%s: Uncaught %s", ZSTR_VAL(prefix), ZSTR_VAL(exception_str));
}

CREX_API CREX_COLD void crex_throw_exception_object(zval *exception) /* {{{ */
{
	if (exception == NULL || C_TYPE_P(exception) != IS_OBJECT) {
		crex_error_noreturn(E_CORE_ERROR, "Need to supply an object when throwing an exception");
	}

	crex_class_entry *exception_ce = C_OBJCE_P(exception);

	if (!exception_ce || !instanceof_function(exception_ce, crex_ce_throwable)) {
		crex_throw_error(NULL, "Cannot throw objects that do not implement Throwable");
		zval_ptr_dtor(exception);
		return;
	}

	crex_throw_exception_internal(C_OBJ_P(exception));
}
/* }}} */

CREX_API CREX_COLD crex_object *crex_create_unwind_exit(void)
{
	return crex_objects_new(&crex_ce_unwind_exit);
}

CREX_API CREX_COLD crex_object *crex_create_graceful_exit(void)
{
	return crex_objects_new(&crex_ce_graceful_exit);
}

CREX_API CREX_COLD void crex_throw_unwind_exit(void)
{
	CREX_ASSERT(!EG(exception));
	EG(exception) = crex_create_unwind_exit();
	EG(opline_before_exception) = EG(current_execute_data)->opline;
	EG(current_execute_data)->opline = EG(exception_op);
}

CREX_API CREX_COLD void crex_throw_graceful_exit(void)
{
	CREX_ASSERT(!EG(exception));
	EG(exception) = crex_create_graceful_exit();
	EG(opline_before_exception) = EG(current_execute_data)->opline;
	EG(current_execute_data)->opline = EG(exception_op);
}

CREX_API bool crex_is_unwind_exit(const crex_object *ex)
{
	return ex->ce == &crex_ce_unwind_exit;
}

CREX_API bool crex_is_graceful_exit(const crex_object *ex)
{
	return ex->ce == &crex_ce_graceful_exit;
}
