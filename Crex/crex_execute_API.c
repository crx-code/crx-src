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

#include <stdio.h>
#include <signal.h>

#include "crex.h"
#include "crex_compile.h"
#include "crex_execute.h"
#include "crex_API.h"
#include "crex_stack.h"
#include "crex_constants.h"
#include "crex_extensions.h"
#include "crex_exceptions.h"
#include "crex_closures.h"
#include "crex_generators.h"
#include "crex_vm.h"
#include "crex_float.h"
#include "crex_fibers.h"
#include "crex_weakrefs.h"
#include "crex_inheritance.h"
#include "crex_observer.h"
#include "crex_call_stack.h"
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef CREX_MAX_EXECUTION_TIMERS
#include <sys/syscall.h>
#endif

CREX_API void (*crex_execute_ex)(crex_execute_data *execute_data);
CREX_API void (*crex_execute_internal)(crex_execute_data *execute_data, zval *return_value);
CREX_API crex_class_entry *(*crex_autoload)(crex_string *name, crex_string *lc_name);

/* true globals */
CREX_API const crex_fcall_info empty_fcall_info = {0};
CREX_API const crex_fcall_info_cache empty_fcall_info_cache = {0};

#ifdef CREX_WIN32
CREX_TLS HANDLE tq_timer = NULL;
#endif

#if 0&&CREX_DEBUG
static void (*original_sigsegv_handler)(int);
static void crex_handle_sigsegv(void) /* {{{ */
{
	fflush(stdout);
	fflush(stderr);
	if (original_sigsegv_handler == crex_handle_sigsegv) {
		signal(SIGSEGV, original_sigsegv_handler);
	} else {
		signal(SIGSEGV, SIG_DFL);
	}
	{

		fprintf(stderr, "SIGSEGV caught on opcode %d on opline %d of %s() at %s:%d\n\n",
				active_opline->opcode,
				active_opline-EG(active_op_array)->opcodes,
				get_active_function_name(),
				crex_get_executed_filename(),
				crex_get_executed_lineno());
/* See http://support.microsoft.com/kb/190351 */
#ifdef CREX_WIN32
		fflush(stderr);
#endif
	}
	if (original_sigsegv_handler!=crex_handle_sigsegv) {
		original_sigsegv_handler(dummy);
	}
}
/* }}} */
#endif

static void crex_extension_activator(crex_extension *extension) /* {{{ */
{
	if (extension->activate) {
		extension->activate();
	}
}
/* }}} */

static void crex_extension_deactivator(crex_extension *extension) /* {{{ */
{
	if (extension->deactivate) {
		extension->deactivate();
	}
}
/* }}} */

static int clean_non_persistent_constant_full(zval *zv) /* {{{ */
{
	crex_constant *c = C_PTR_P(zv);
	return (CREX_CONSTANT_FLAGS(c) & CONST_PERSISTENT) ? CREX_HASH_APPLY_KEEP : CREX_HASH_APPLY_REMOVE;
}
/* }}} */

static int clean_non_persistent_function_full(zval *zv) /* {{{ */
{
	crex_function *function = C_PTR_P(zv);
	return (function->type == CREX_INTERNAL_FUNCTION) ? CREX_HASH_APPLY_KEEP : CREX_HASH_APPLY_REMOVE;
}
/* }}} */

static int clean_non_persistent_class_full(zval *zv) /* {{{ */
{
	crex_class_entry *ce = C_PTR_P(zv);
	return (ce->type == CREX_INTERNAL_CLASS) ? CREX_HASH_APPLY_KEEP : CREX_HASH_APPLY_REMOVE;
}
/* }}} */

void init_executor(void) /* {{{ */
{
	crex_init_fpu();

	ZVAL_NULL(&EG(uninitialized_zval));
	ZVAL_ERROR(&EG(error_zval));
/* destroys stack frame, therefore makes core dumps worthless */
#if 0&&CREX_DEBUG
	original_sigsegv_handler = signal(SIGSEGV, crex_handle_sigsegv);
#endif

	EG(symtable_cache_ptr) = EG(symtable_cache);
	EG(symtable_cache_limit) = EG(symtable_cache) + SYMTABLE_CACHE_SIZE;
	EG(no_extensions) = 0;

	EG(function_table) = CG(function_table);
	EG(class_table) = CG(class_table);

	EG(in_autoload) = NULL;
	EG(error_handling) = EH_NORMAL;
	EG(flags) = EG_FLAGS_INITIAL;

	crex_vm_stack_init();

	crex_hash_init(&EG(symbol_table), 64, NULL, ZVAL_PTR_DTOR, 0);

	crex_llist_apply(&crex_extensions, (llist_apply_func_t) crex_extension_activator);

	crex_hash_init(&EG(included_files), 8, NULL, NULL, 0);

	EG(ticks_count) = 0;

	ZVAL_UNDEF(&EG(user_error_handler));
	ZVAL_UNDEF(&EG(user_exception_handler));

	EG(current_execute_data) = NULL;

	crex_stack_init(&EG(user_error_handlers_error_reporting), sizeof(int));
	crex_stack_init(&EG(user_error_handlers), sizeof(zval));
	crex_stack_init(&EG(user_exception_handlers), sizeof(zval));

	crex_objects_store_init(&EG(objects_store), 1024);

	EG(full_tables_cleanup) = 0;
	CREX_ATOMIC_BOOL_INIT(&EG(vm_interrupt), false);
	CREX_ATOMIC_BOOL_INIT(&EG(timed_out), false);

	EG(exception) = NULL;
	EG(prev_exception) = NULL;

	EG(fake_scope) = NULL;
	EG(trampoline).common.function_name = NULL;

	EG(ht_iterators_count) = sizeof(EG(ht_iterators_slots)) / sizeof(HashTableIterator);
	EG(ht_iterators_used) = 0;
	EG(ht_iterators) = EG(ht_iterators_slots);
	memset(EG(ht_iterators), 0, sizeof(EG(ht_iterators_slots)));

	EG(persistent_constants_count) = EG(crex_constants)->nNumUsed;
	EG(persistent_functions_count) = EG(function_table)->nNumUsed;
	EG(persistent_classes_count)   = EG(class_table)->nNumUsed;

	EG(get_gc_buffer).start = EG(get_gc_buffer).end = EG(get_gc_buffer).cur = NULL;

	EG(record_errors) = false;
	EG(num_errors) = 0;
	EG(errors) = NULL;

	EG(filename_override) = NULL;
	EG(lineno_override) = -1;

	crex_max_execution_timer_init();
	crex_fiber_init();
	crex_weakrefs_init();

	EG(active) = 1;
}
/* }}} */

static int zval_call_destructor(zval *zv) /* {{{ */
{
	if (C_TYPE_P(zv) == IS_INDIRECT) {
		zv = C_INDIRECT_P(zv);
	}
	if (C_TYPE_P(zv) == IS_OBJECT && C_REFCOUNT_P(zv) == 1) {
		return CREX_HASH_APPLY_REMOVE;
	} else {
		return CREX_HASH_APPLY_KEEP;
	}
}
/* }}} */

static void crex_unclean_zval_ptr_dtor(zval *zv) /* {{{ */
{
	if (C_TYPE_P(zv) == IS_INDIRECT) {
		zv = C_INDIRECT_P(zv);
	}
	i_zval_ptr_dtor(zv);
}
/* }}} */

static CREX_COLD void crex_throw_or_error(int fetch_type, crex_class_entry *exception_ce, const char *format, ...) /* {{{ */
{
	va_list va;
	char *message = NULL;

	va_start(va, format);
	crex_vspprintf(&message, 0, format, va);

	if (fetch_type & CREX_FETCH_CLASS_EXCEPTION) {
		crex_throw_error(exception_ce, "%s", message);
	} else {
		crex_error(E_ERROR, "%s", message);
	}

	efree(message);
	va_end(va);
}
/* }}} */

void shutdown_destructors(void) /* {{{ */
{
	if (CG(unclean_shutdown)) {
		EG(symbol_table).pDestructor = crex_unclean_zval_ptr_dtor;
	}
	crex_try {
		uint32_t symbols;
		do {
			symbols = crex_hash_num_elements(&EG(symbol_table));
			crex_hash_reverse_apply(&EG(symbol_table), (apply_func_t) zval_call_destructor);
		} while (symbols != crex_hash_num_elements(&EG(symbol_table)));
		crex_objects_store_call_destructors(&EG(objects_store));
	} crex_catch {
		/* if we couldn't destruct cleanly, mark all objects as destructed anyway */
		crex_objects_store_mark_destructed(&EG(objects_store));
	} crex_end_try();
}
/* }}} */

/* Free values held by the executor. */
CREX_API void crex_shutdown_executor_values(bool fast_shutdown)
{
	crex_string *key;
	zval *zv;

	EG(flags) |= EG_FLAGS_IN_RESOURCE_SHUTDOWN;
	crex_try {
		crex_close_rsrc_list(&EG(regular_list));
	} crex_end_try();

	/* No CRX callback functions should be called after this point. */
	EG(active) = 0;

	if (!fast_shutdown) {
		crex_hash_graceful_reverse_destroy(&EG(symbol_table));

		/* Constants may contain objects, destroy them before the object store. */
		if (EG(full_tables_cleanup)) {
			crex_hash_reverse_apply(EG(crex_constants), clean_non_persistent_constant_full);
		} else {
			CREX_HASH_MAP_REVERSE_FOREACH_STR_KEY_VAL(EG(crex_constants), key, zv) {
				crex_constant *c = C_PTR_P(zv);
				if (_idx == EG(persistent_constants_count)) {
					break;
				}
				zval_ptr_dtor_nogc(&c->value);
				if (c->name) {
					crex_string_release_ex(c->name, 0);
				}
				efree(c);
				crex_string_release_ex(key, 0);
			} CREX_HASH_MAP_FOREACH_END_DEL();
		}

		/* Release static properties and static variables prior to the final GC run,
		 * as they may hold GC roots. */
		CREX_HASH_MAP_REVERSE_FOREACH_VAL(EG(function_table), zv) {
			crex_op_array *op_array = C_PTR_P(zv);
			if (op_array->type == CREX_INTERNAL_FUNCTION) {
				break;
			}
			if (CREX_MAP_PTR(op_array->static_variables_ptr)) {
				HashTable *ht = CREX_MAP_PTR_GET(op_array->static_variables_ptr);
				if (ht) {
					crex_array_destroy(ht);
					CREX_MAP_PTR_SET(op_array->static_variables_ptr, NULL);
				}
			}
		} CREX_HASH_FOREACH_END();
		CREX_HASH_MAP_REVERSE_FOREACH_VAL(EG(class_table), zv) {
			crex_class_entry *ce = C_PTR_P(zv);

			if (ce->default_static_members_count) {
				crex_cleanup_internal_class_data(ce);
			}

			if (CREX_MAP_PTR(ce->mutable_data)) {
				if (CREX_MAP_PTR_GET_IMM(ce->mutable_data)) {
					crex_cleanup_mutable_class_data(ce);
				}
			} else if (ce->type == CREX_USER_CLASS && !(ce->ce_flags & CREX_ACC_IMMUTABLE)) {
				/* Constants may contain objects, destroy the values before the object store. */
				crex_class_constant *c;
				CREX_HASH_MAP_FOREACH_PTR(&ce->constants_table, c) {
					if (c->ce == ce) {
						zval_ptr_dtor_nogc(&c->value);
						ZVAL_UNDEF(&c->value);
					}
				} CREX_HASH_FOREACH_END();

				/* properties may contain objects as well */
				if (ce->default_properties_table) {
					zval *p = ce->default_properties_table;
					zval *end = p + ce->default_properties_count;

					while (p != end) {
						i_zval_ptr_dtor(p);
						ZVAL_UNDEF(p);
						p++;
					}
				}
			}

			if (ce->type == CREX_USER_CLASS && ce->backed_enum_table) {
				CREX_ASSERT(!(ce->ce_flags & CREX_ACC_IMMUTABLE));
				crex_hash_release(ce->backed_enum_table);
				ce->backed_enum_table = NULL;
			}

			if (ce->ce_flags & CREX_HAS_STATIC_IN_METHODS) {
				crex_op_array *op_array;
				CREX_HASH_MAP_FOREACH_PTR(&ce->function_table, op_array) {
					if (op_array->type == CREX_USER_FUNCTION) {
						if (CREX_MAP_PTR(op_array->static_variables_ptr)) {
							HashTable *ht = CREX_MAP_PTR_GET(op_array->static_variables_ptr);
							if (ht) {
								crex_array_destroy(ht);
								CREX_MAP_PTR_SET(op_array->static_variables_ptr, NULL);
							}
						}
					}
				} CREX_HASH_FOREACH_END();
			}
		} CREX_HASH_FOREACH_END();

		/* Also release error and exception handlers, which may hold objects. */
		if (C_TYPE(EG(user_error_handler)) != IS_UNDEF) {
			zval_ptr_dtor(&EG(user_error_handler));
			ZVAL_UNDEF(&EG(user_error_handler));
		}

		if (C_TYPE(EG(user_exception_handler)) != IS_UNDEF) {
			zval_ptr_dtor(&EG(user_exception_handler));
			ZVAL_UNDEF(&EG(user_exception_handler));
		}

		crex_stack_clean(&EG(user_error_handlers_error_reporting), NULL, 1);
		crex_stack_clean(&EG(user_error_handlers), (void (*)(void *))ZVAL_PTR_DTOR, 1);
		crex_stack_clean(&EG(user_exception_handlers), (void (*)(void *))ZVAL_PTR_DTOR, 1);

#if CREX_DEBUG
		if (!CG(unclean_shutdown)) {
			gc_collect_cycles();
		}
#endif
	} else {
		crex_hash_discard(EG(crex_constants), EG(persistent_constants_count));
	}

	crex_objects_store_free_object_storage(&EG(objects_store), fast_shutdown);
}

void shutdown_executor(void) /* {{{ */
{
	crex_string *key;
	zval *zv;
#if CREX_DEBUG
	bool fast_shutdown = 0;
#else
	bool fast_shutdown = is_crex_mm() && !EG(full_tables_cleanup);
#endif

	crex_try {
		crex_stream_shutdown();
	} crex_end_try();

	crex_shutdown_executor_values(fast_shutdown);

	crex_weakrefs_shutdown();
	crex_max_execution_timer_shutdown();
	crex_fiber_shutdown();

	crex_try {
		crex_llist_apply(&crex_extensions, (llist_apply_func_t) crex_extension_deactivator);
	} crex_end_try();

	if (fast_shutdown) {
		/* Fast Request Shutdown
		 * =====================
		 * Crex Memory Manager frees memory by its own. We don't have to free
		 * each allocated block separately.
		 */
		crex_hash_discard(EG(function_table), EG(persistent_functions_count));
		crex_hash_discard(EG(class_table), EG(persistent_classes_count));
	} else {
		crex_vm_stack_destroy();

		if (EG(full_tables_cleanup)) {
			crex_hash_reverse_apply(EG(function_table), clean_non_persistent_function_full);
			crex_hash_reverse_apply(EG(class_table), clean_non_persistent_class_full);
		} else {
			CREX_HASH_MAP_REVERSE_FOREACH_STR_KEY_VAL(EG(function_table), key, zv) {
				crex_function *func = C_PTR_P(zv);
				if (_idx == EG(persistent_functions_count)) {
					break;
				}
				destroy_op_array(&func->op_array);
				crex_string_release_ex(key, 0);
			} CREX_HASH_MAP_FOREACH_END_DEL();

			CREX_HASH_MAP_REVERSE_FOREACH_STR_KEY_VAL(EG(class_table), key, zv) {
				if (_idx == EG(persistent_classes_count)) {
					break;
				}
				destroy_crex_class(zv);
				crex_string_release_ex(key, 0);
			} CREX_HASH_MAP_FOREACH_END_DEL();
		}

		while (EG(symtable_cache_ptr) > EG(symtable_cache)) {
			EG(symtable_cache_ptr)--;
			crex_hash_destroy(*EG(symtable_cache_ptr));
			FREE_HASHTABLE(*EG(symtable_cache_ptr));
		}

		crex_hash_destroy(&EG(included_files));

		crex_stack_destroy(&EG(user_error_handlers_error_reporting));
		crex_stack_destroy(&EG(user_error_handlers));
		crex_stack_destroy(&EG(user_exception_handlers));
		crex_objects_store_destroy(&EG(objects_store));
		if (EG(in_autoload)) {
			crex_hash_destroy(EG(in_autoload));
			FREE_HASHTABLE(EG(in_autoload));
		}

		if (EG(ht_iterators) != EG(ht_iterators_slots)) {
			efree(EG(ht_iterators));
		}
	}

#if CREX_DEBUG
	if (EG(ht_iterators_used) && !CG(unclean_shutdown)) {
		crex_error(E_WARNING, "Leaked %" PRIu32 " hashtable iterators", EG(ht_iterators_used));
	}
#endif

	/* Check whether anyone is hogging the trampoline. */
	CREX_ASSERT(EG(trampoline).common.function_name == NULL || CG(unclean_shutdown));

	EG(ht_iterators_used) = 0;

	crex_shutdown_fpu();
}
/* }}} */

/* return class name and "::" or "". */
CREX_API const char *get_active_class_name(const char **space) /* {{{ */
{
	crex_function *func;

	if (!crex_is_executing()) {
		if (space) {
			*space = "";
		}
		return "";
	}

	func = EG(current_execute_data)->func;

	switch (func->type) {
		case CREX_USER_FUNCTION:
		case CREX_INTERNAL_FUNCTION:
		{
			crex_class_entry *ce = func->common.scope;

			if (space) {
				*space = ce ? "::" : "";
			}
			return ce ? ZSTR_VAL(ce->name) : "";
		}
		default:
			if (space) {
				*space = "";
			}
			return "";
	}
}
/* }}} */

CREX_API const char *get_active_function_name(void) /* {{{ */
{
	crex_function *func;

	if (!crex_is_executing()) {
		return NULL;
	}

	func = EG(current_execute_data)->func;

	switch (func->type) {
		case CREX_USER_FUNCTION: {
				crex_string *function_name = func->common.function_name;

				if (function_name) {
					return ZSTR_VAL(function_name);
				} else {
					return "main";
				}
			}
			break;
		case CREX_INTERNAL_FUNCTION:
			return ZSTR_VAL(func->common.function_name);
			break;
		default:
			return NULL;
	}
}
/* }}} */

CREX_API crex_string *get_active_function_or_method_name(void) /* {{{ */
{
	CREX_ASSERT(crex_is_executing());

	return get_function_or_method_name(EG(current_execute_data)->func);
}
/* }}} */

CREX_API crex_string *get_function_or_method_name(const crex_function *func) /* {{{ */
{
	if (func->common.scope && func->common.function_name) {
		return crex_create_member_string(func->common.scope->name, func->common.function_name);
	}

	return func->common.function_name ? crex_string_copy(func->common.function_name) : ZSTR_INIT_LITERAL("main", 0);
}
/* }}} */

CREX_API const char *get_active_function_arg_name(uint32_t arg_num) /* {{{ */
{
	crex_function *func;

	if (!crex_is_executing()) {
		return NULL;
	}

	func = EG(current_execute_data)->func;

	return get_function_arg_name(func, arg_num);
}
/* }}} */

CREX_API const char *get_function_arg_name(const crex_function *func, uint32_t arg_num) /* {{{ */
{
	if (!func || arg_num == 0 || func->common.num_args < arg_num) {
		return NULL;
	}

	if (func->type == CREX_USER_FUNCTION || (func->common.fn_flags & CREX_ACC_USER_ARG_INFO)) {
		return ZSTR_VAL(func->common.arg_info[arg_num - 1].name);
	} else {
		return ((crex_internal_arg_info*) func->common.arg_info)[arg_num - 1].name;
	}
}
/* }}} */

CREX_API const char *crex_get_executed_filename(void) /* {{{ */
{
	crex_string *filename = crex_get_executed_filename_ex();
	return filename != NULL ? ZSTR_VAL(filename) : "[no active file]";
}
/* }}} */

CREX_API crex_string *crex_get_executed_filename_ex(void) /* {{{ */
{
	crex_string *filename_override = EG(filename_override);
	if (filename_override != NULL) {
		return filename_override;
	}

	crex_execute_data *ex = EG(current_execute_data);

	while (ex && (!ex->func || !CREX_USER_CODE(ex->func->type))) {
		ex = ex->prev_execute_data;
	}
	if (ex) {
		return ex->func->op_array.filename;
	} else {
		return NULL;
	}
}
/* }}} */

CREX_API uint32_t crex_get_executed_lineno(void) /* {{{ */
{
	crex_long lineno_override = EG(lineno_override);
	if (lineno_override != -1) {
		return lineno_override;
	}

	crex_execute_data *ex = EG(current_execute_data);

	while (ex && (!ex->func || !CREX_USER_CODE(ex->func->type))) {
		ex = ex->prev_execute_data;
	}
	if (ex) {
		if (!ex->opline) {
			/* Missing SAVE_OPLINE()? Falling back to first line of function */
			return ex->func->op_array.opcodes[0].lineno;
		}
		if (EG(exception) && ex->opline->opcode == CREX_HANDLE_EXCEPTION &&
		    ex->opline->lineno == 0 && EG(opline_before_exception)) {
			return EG(opline_before_exception)->lineno;
		}
		return ex->opline->lineno;
	} else {
		return 0;
	}
}
/* }}} */

CREX_API crex_class_entry *crex_get_executed_scope(void) /* {{{ */
{
	crex_execute_data *ex = EG(current_execute_data);

	while (1) {
		if (!ex) {
			return NULL;
		} else if (ex->func && (CREX_USER_CODE(ex->func->type) || ex->func->common.scope)) {
			return ex->func->common.scope;
		}
		ex = ex->prev_execute_data;
	}
}
/* }}} */

CREX_API bool crex_is_executing(void) /* {{{ */
{
	return EG(current_execute_data) != 0;
}
/* }}} */

CREX_API crex_result CREX_FASTCALL zval_update_constant_with_ctx(zval *p, crex_class_entry *scope, crex_ast_evaluate_ctx *ctx)
{
	if (C_TYPE_P(p) == IS_CONSTANT_AST) {
		crex_ast *ast = C_ASTVAL_P(p);

		if (ast->kind == CREX_AST_CONSTANT) {
			crex_string *name = crex_ast_get_constant_name(ast);
			zval *zv = crex_get_constant_ex(name, scope, ast->attr);
			if (UNEXPECTED(zv == NULL)) {
				return FAILURE;
			}

			zval_ptr_dtor_nogc(p);
			ZVAL_COPY_OR_DUP(p, zv);
		} else {
			zval tmp;
			bool short_circuited;

			// Increase the refcount during crex_ast_evaluate to avoid releasing the ast too early
			// on nested calls to zval_update_constant_ex which can happen when retriggering ast
			// evaluation during autoloading.
			crex_ast_ref *ast_ref = C_AST_P(p);
			bool ast_is_refcounted = !(GC_FLAGS(ast_ref) & GC_IMMUTABLE);
			if (ast_is_refcounted) {
				GC_ADDREF(ast_ref);
			}
			crex_result result = crex_ast_evaluate_ex(&tmp, ast, scope, &short_circuited, ctx) != SUCCESS;
			if (ast_is_refcounted && !GC_DELREF(ast_ref)) {
				rc_dtor_func((crex_refcounted *)ast_ref);
			}
			if (UNEXPECTED(result != SUCCESS)) {
				return FAILURE;
			}
			zval_ptr_dtor_nogc(p);
			ZVAL_COPY_VALUE(p, &tmp);
		}
	}
	return SUCCESS;
}
/* }}} */

CREX_API crex_result CREX_FASTCALL zval_update_constant_ex(zval *p, crex_class_entry *scope)
{
	crex_ast_evaluate_ctx ctx = {0};
	return zval_update_constant_with_ctx(p, scope, &ctx);
}

CREX_API crex_result CREX_FASTCALL zval_update_constant(zval *pp) /* {{{ */
{
	return zval_update_constant_ex(pp, EG(current_execute_data) ? crex_get_executed_scope() : CG(active_class_entry));
}
/* }}} */

crex_result _call_user_function_impl(zval *object, zval *function_name, zval *retval_ptr, uint32_t param_count, zval params[], HashTable *named_params) /* {{{ */
{
	crex_fcall_info fci;

	fci.size = sizeof(fci);
	if (object) {
		CREX_ASSERT(C_TYPE_P(object) == IS_OBJECT);
		fci.object = C_OBJ_P(object);
	} else {
		fci.object = NULL;
	}
	ZVAL_COPY_VALUE(&fci.function_name, function_name);
	fci.retval = retval_ptr;
	fci.param_count = param_count;
	fci.params = params;
	fci.named_params = named_params;

	return crex_call_function(&fci, NULL);
}
/* }}} */

crex_result crex_call_function(crex_fcall_info *fci, crex_fcall_info_cache *fci_cache) /* {{{ */
{
	uint32_t i;
	crex_execute_data *call;
	crex_fcall_info_cache fci_cache_local;
	crex_function *func;
	uint32_t call_info;
	void *object_or_called_scope;
	crex_class_entry *orig_fake_scope;

	ZVAL_UNDEF(fci->retval);

	if (!EG(active)) {
		return FAILURE; /* executor is already inactive */
	}

	if (EG(exception)) {
		if (fci_cache) {
			crex_release_fcall_info_cache(fci_cache);
		}
		return SUCCESS; /* we would result in an instable executor otherwise */
	}

	CREX_ASSERT(fci->size == sizeof(crex_fcall_info));

	if (!fci_cache || !fci_cache->function_handler) {
		char *error = NULL;

		if (!fci_cache) {
			fci_cache = &fci_cache_local;
		}

		if (!crex_is_callable_ex(&fci->function_name, fci->object, 0, NULL, fci_cache, &error)) {
			CREX_ASSERT(error && "Should have error if not callable");
			crex_string *callable_name
				= crex_get_callable_name_ex(&fci->function_name, fci->object);
			crex_throw_error(NULL, "Invalid callback %s, %s", ZSTR_VAL(callable_name), error);
			efree(error);
			crex_string_release_ex(callable_name, 0);
			return SUCCESS;
		}

		CREX_ASSERT(!error);
	}

	func = fci_cache->function_handler;
	if ((func->common.fn_flags & CREX_ACC_STATIC) || !fci_cache->object) {
		object_or_called_scope = fci_cache->called_scope;
		call_info = CREX_CALL_TOP_FUNCTION | CREX_CALL_DYNAMIC;
	} else {
		object_or_called_scope = fci_cache->object;
		call_info = CREX_CALL_TOP_FUNCTION | CREX_CALL_DYNAMIC | CREX_CALL_HAS_THIS;
	}

	call = crex_vm_stack_push_call_frame(call_info,
		func, fci->param_count, object_or_called_scope);

	if (UNEXPECTED(func->common.fn_flags & CREX_ACC_DEPRECATED)) {
		crex_deprecated_function(func);

		if (UNEXPECTED(EG(exception))) {
			crex_vm_stack_free_call_frame(call);
			return SUCCESS;
		}
	}

	for (i=0; i<fci->param_count; i++) {
		zval *param = CREX_CALL_ARG(call, i+1);
		zval *arg = &fci->params[i];
		bool must_wrap = 0;
		if (UNEXPECTED(C_ISUNDEF_P(arg))) {
			/* Allow forwarding undef slots. This is only used by Closure::__invoke(). */
			ZVAL_UNDEF(param);
			CREX_ADD_CALL_FLAG(call, CREX_CALL_MAY_HAVE_UNDEF);
			continue;
		}

		if (ARG_SHOULD_BE_SENT_BY_REF(func, i + 1)) {
			if (UNEXPECTED(!C_ISREF_P(arg))) {
				if (!ARG_MAY_BE_SENT_BY_REF(func, i + 1)) {
					/* By-value send is not allowed -- emit a warning,
					 * and perform the call with the value wrapped in a reference. */
					crex_param_must_be_ref(func, i + 1);
					must_wrap = 1;
					if (UNEXPECTED(EG(exception))) {
						CREX_CALL_NUM_ARGS(call) = i;
cleanup_args:
						crex_vm_stack_free_args(call);
						crex_vm_stack_free_call_frame(call);
						return SUCCESS;
					}
				}
			}
		} else {
			if (C_ISREF_P(arg) &&
			    !(func->common.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE)) {
				/* don't separate references for __call */
				arg = C_REFVAL_P(arg);
			}
		}

		if (EXPECTED(!must_wrap)) {
			ZVAL_COPY(param, arg);
		} else {
			C_TRY_ADDREF_P(arg);
			ZVAL_NEW_REF(param, arg);
		}
	}

	if (fci->named_params) {
		crex_string *name;
		zval *arg;
		uint32_t arg_num = CREX_CALL_NUM_ARGS(call) + 1;
		bool have_named_params = 0;
		CREX_HASH_FOREACH_STR_KEY_VAL(fci->named_params, name, arg) {
			bool must_wrap = 0;
			zval *target;
			if (name) {
				void *cache_slot[2] = {NULL, NULL};
				have_named_params = 1;
				target = crex_handle_named_arg(&call, name, &arg_num, cache_slot);
				if (!target) {
					goto cleanup_args;
				}
			} else {
				if (have_named_params) {
					crex_throw_error(NULL,
						"Cannot use positional argument after named argument");
					goto cleanup_args;
				}

				crex_vm_stack_extend_call_frame(&call, arg_num - 1, 1);
				target = CREX_CALL_ARG(call, arg_num);
			}

			if (ARG_SHOULD_BE_SENT_BY_REF(func, arg_num)) {
				if (UNEXPECTED(!C_ISREF_P(arg))) {
					if (!ARG_MAY_BE_SENT_BY_REF(func, arg_num)) {
						/* By-value send is not allowed -- emit a warning,
						 * and perform the call with the value wrapped in a reference. */
						crex_param_must_be_ref(func, arg_num);
						must_wrap = 1;
						if (UNEXPECTED(EG(exception))) {
							goto cleanup_args;
						}
					}
				}
			} else {
				if (C_ISREF_P(arg) &&
					!(func->common.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE)) {
					/* don't separate references for __call */
					arg = C_REFVAL_P(arg);
				}
			}

			if (EXPECTED(!must_wrap)) {
				ZVAL_COPY(target, arg);
			} else {
				C_TRY_ADDREF_P(arg);
				ZVAL_NEW_REF(target, arg);
			}
			if (!name) {
				CREX_CALL_NUM_ARGS(call)++;
				arg_num++;
			}
		} CREX_HASH_FOREACH_END();
	}

	if (UNEXPECTED(CREX_CALL_INFO(call) & CREX_CALL_MAY_HAVE_UNDEF)) {
		/* crex_handle_undef_args assumes prev_execute_data is initialized. */
		call->prev_execute_data = NULL;
		if (crex_handle_undef_args(call) == FAILURE) {
			crex_vm_stack_free_args(call);
			crex_vm_stack_free_call_frame(call);
			return SUCCESS;
		}
	}

	if (UNEXPECTED(func->op_array.fn_flags & CREX_ACC_CLOSURE)) {
		uint32_t call_info;

		GC_ADDREF(CREX_CLOSURE_OBJECT(func));
		call_info = CREX_CALL_CLOSURE;
		if (func->common.fn_flags & CREX_ACC_FAKE_CLOSURE) {
			call_info |= CREX_CALL_FAKE_CLOSURE;
		}
		CREX_ADD_CALL_FLAG(call, call_info);
	}

	if (func->common.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE) {
		fci_cache->function_handler = NULL;
	}

	orig_fake_scope = EG(fake_scope);
	EG(fake_scope) = NULL;
	if (func->type == CREX_USER_FUNCTION) {
		uint32_t orig_jit_trace_num = EG(jit_trace_num);

		crex_init_func_execute_data(call, &func->op_array, fci->retval);
		CREX_OBSERVER_FCALL_BEGIN(call);
		crex_execute_ex(call);
		EG(jit_trace_num) = orig_jit_trace_num;
	} else {
		CREX_ASSERT(func->type == CREX_INTERNAL_FUNCTION);
		ZVAL_NULL(fci->retval);
		call->prev_execute_data = EG(current_execute_data);
		EG(current_execute_data) = call;
#if CREX_DEBUG
		bool should_throw = crex_internal_call_should_throw(func, call);
#endif
		CREX_OBSERVER_FCALL_BEGIN(call);
		if (EXPECTED(crex_execute_internal == NULL)) {
			/* saves one function call if crex_execute_internal is not used */
			func->internal_function.handler(call, fci->retval);
		} else {
			crex_execute_internal(call, fci->retval);
		}

#if CREX_DEBUG
		if (!EG(exception) && call->func) {
			if (should_throw) {
				crex_internal_call_arginfo_violation(call->func);
			}
			CREX_ASSERT(!(call->func->common.fn_flags & CREX_ACC_HAS_RETURN_TYPE) ||
				crex_verify_internal_return_type(call->func, fci->retval));
			CREX_ASSERT((call->func->common.fn_flags & CREX_ACC_RETURN_REFERENCE)
				? C_ISREF_P(fci->retval) : !C_ISREF_P(fci->retval));
		}
#endif
		CREX_OBSERVER_FCALL_END(call, fci->retval);
		EG(current_execute_data) = call->prev_execute_data;
		crex_vm_stack_free_args(call);
		if (UNEXPECTED(CREX_CALL_INFO(call) & CREX_CALL_HAS_EXTRA_NAMED_PARAMS)) {
			crex_array_release(call->extra_named_params);
		}

		if (EG(exception)) {
			zval_ptr_dtor(fci->retval);
			ZVAL_UNDEF(fci->retval);
		}

		/* This flag is regularly checked while running user functions, but not internal
		 * So see whether interrupt flag was set while the function was running... */
		if (crex_atomic_bool_exchange_ex(&EG(vm_interrupt), false)) {
			if (crex_atomic_bool_load_ex(&EG(timed_out))) {
				crex_timeout();
			} else if (crex_interrupt_function) {
				crex_interrupt_function(EG(current_execute_data));
			}
		}

		if (UNEXPECTED(CREX_CALL_INFO(call) & CREX_CALL_RELEASE_THIS)) {
			OBJ_RELEASE(C_OBJ(call->This));
		}
	}
	EG(fake_scope) = orig_fake_scope;

	crex_vm_stack_free_call_frame(call);

	if (UNEXPECTED(EG(exception))) {
		if (UNEXPECTED(!EG(current_execute_data))) {
			crex_throw_exception_internal(NULL);
		} else if (EG(current_execute_data)->func &&
		           CREX_USER_CODE(EG(current_execute_data)->func->common.type)) {
			crex_rethrow_exception(EG(current_execute_data));
		}
	}

	return SUCCESS;
}
/* }}} */

CREX_API void crex_call_known_function(
		crex_function *fn, crex_object *object, crex_class_entry *called_scope, zval *retval_ptr,
		uint32_t param_count, zval *params, HashTable *named_params)
{
	zval retval;
	crex_fcall_info fci;
	crex_fcall_info_cache fcic;

	CREX_ASSERT(fn && "crex_function must be passed!");

	fci.size = sizeof(fci);
	fci.object = object;
	fci.retval = retval_ptr ? retval_ptr : &retval;
	fci.param_count = param_count;
	fci.params = params;
	fci.named_params = named_params;
	ZVAL_UNDEF(&fci.function_name); /* Unused */

	fcic.function_handler = fn;
	fcic.object = object;
	fcic.called_scope = called_scope;

	crex_result result = crex_call_function(&fci, &fcic);
	if (UNEXPECTED(result == FAILURE)) {
		if (!EG(exception)) {
			crex_error_noreturn(E_CORE_ERROR, "Couldn't execute method %s%s%s",
				fn->common.scope ? ZSTR_VAL(fn->common.scope->name) : "",
				fn->common.scope ? "::" : "", ZSTR_VAL(fn->common.function_name));
		}
	}

	if (!retval_ptr) {
		zval_ptr_dtor(&retval);
	}
}

CREX_API void crex_call_known_instance_method_with_2_params(
		crex_function *fn, crex_object *object, zval *retval_ptr, zval *param1, zval *param2)
{
	zval params[2];
	ZVAL_COPY_VALUE(&params[0], param1);
	ZVAL_COPY_VALUE(&params[1], param2);
	crex_call_known_instance_method(fn, object, retval_ptr, 2, params);
}

CREX_API crex_result crex_call_method_if_exists(
		crex_object *object, crex_string *method_name, zval *retval,
		uint32_t param_count, zval *params)
{
	crex_fcall_info fci;
	fci.size = sizeof(crex_fcall_info);
	fci.object = object;
	ZVAL_STR(&fci.function_name, method_name);
	fci.retval = retval;
	fci.param_count = param_count;
	fci.params = params;
	fci.named_params = NULL;

	crex_fcall_info_cache fcc;
	if (!crex_is_callable_ex(&fci.function_name, fci.object, IS_CALLABLE_SUPPRESS_DEPRECATIONS, NULL, &fcc, NULL)) {
		ZVAL_UNDEF(retval);
		return FAILURE;
	}

	return crex_call_function(&fci, &fcc);
}

/* 0-9 a-z A-Z _ \ 0x80-0xff */
static const uint32_t valid_chars[8] = {
	0x00000000,
	0x03ff0000,
	0x97fffffe,
	0x07fffffe,
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0xffffffff,
};

CREX_API bool crex_is_valid_class_name(crex_string *name) {
	for (size_t i = 0; i < ZSTR_LEN(name); i++) {
		unsigned char c = ZSTR_VAL(name)[i];
		if (!CREX_BIT_TEST(valid_chars, c)) {
			return 0;
		}
	}
	return 1;
}

CREX_API crex_class_entry *crex_lookup_class_ex(crex_string *name, crex_string *key, uint32_t flags) /* {{{ */
{
	crex_class_entry *ce = NULL;
	zval *zv;
	crex_string *lc_name;
	crex_string *autoload_name;
	uint32_t ce_cache = 0;

	if (ZSTR_HAS_CE_CACHE(name) && ZSTR_VALID_CE_CACHE(name)) {
		ce_cache = GC_REFCOUNT(name);
		ce = GET_CE_CACHE(ce_cache);
		if (EXPECTED(ce)) {
			return ce;
		}
	}

	if (key) {
		lc_name = key;
	} else {
		if (!ZSTR_LEN(name)) {
			return NULL;
		}

		if (ZSTR_VAL(name)[0] == '\\') {
			lc_name = crex_string_alloc(ZSTR_LEN(name) - 1, 0);
			crex_str_tolower_copy(ZSTR_VAL(lc_name), ZSTR_VAL(name) + 1, ZSTR_LEN(name) - 1);
		} else {
			lc_name = crex_string_tolower(name);
		}
	}

	zv = crex_hash_find(EG(class_table), lc_name);
	if (zv) {
		if (!key) {
			crex_string_release_ex(lc_name, 0);
		}
		ce = (crex_class_entry*)C_PTR_P(zv);
		if (UNEXPECTED(!(ce->ce_flags & CREX_ACC_LINKED))) {
			if ((flags & CREX_FETCH_CLASS_ALLOW_UNLINKED) ||
				((flags & CREX_FETCH_CLASS_ALLOW_NEARLY_LINKED) &&
					(ce->ce_flags & CREX_ACC_NEARLY_LINKED))) {
				if (!CG(unlinked_uses)) {
					ALLOC_HASHTABLE(CG(unlinked_uses));
					crex_hash_init(CG(unlinked_uses), 0, NULL, NULL, 0);
				}
				crex_hash_index_add_empty_element(CG(unlinked_uses), (crex_long)(uintptr_t)ce);
				return ce;
			}
			return NULL;
		}
		/* Don't populate CE_CACHE for mutable classes during compilation.
		 * The class may be freed while persisting. */
		if (ce_cache &&
				(!CG(in_compilation) || (ce->ce_flags & CREX_ACC_IMMUTABLE))) {
			SET_CE_CACHE(ce_cache, ce);
		}
		return ce;
	}

	/* The compiler is not-reentrant. Make sure we autoload only during run-time. */
	if ((flags & CREX_FETCH_CLASS_NO_AUTOLOAD) || crex_is_compiling()) {
		if (!key) {
			crex_string_release_ex(lc_name, 0);
		}
		return NULL;
	}

	if (!crex_autoload) {
		if (!key) {
			crex_string_release_ex(lc_name, 0);
		}
		return NULL;
	}

	/* Verify class name before passing it to the autoloader. */
	if (!key && !ZSTR_HAS_CE_CACHE(name) && !crex_is_valid_class_name(name)) {
		crex_string_release_ex(lc_name, 0);
		return NULL;
	}

	if (EG(in_autoload) == NULL) {
		ALLOC_HASHTABLE(EG(in_autoload));
		crex_hash_init(EG(in_autoload), 8, NULL, NULL, 0);
	}

	if (crex_hash_add_empty_element(EG(in_autoload), lc_name) == NULL) {
		if (!key) {
			crex_string_release_ex(lc_name, 0);
		}
		return NULL;
	}

	if (ZSTR_VAL(name)[0] == '\\') {
		autoload_name = crex_string_init(ZSTR_VAL(name) + 1, ZSTR_LEN(name) - 1, 0);
	} else {
		autoload_name = crex_string_copy(name);
	}

	crex_exception_save();
	ce = crex_autoload(autoload_name, lc_name);
	crex_exception_restore();

	crex_string_release_ex(autoload_name, 0);
	crex_hash_del(EG(in_autoload), lc_name);

	if (!key) {
		crex_string_release_ex(lc_name, 0);
	}
	if (ce) {
		CREX_ASSERT(!CG(in_compilation));
		if (ce_cache) {
			SET_CE_CACHE(ce_cache, ce);
		}
	}
	return ce;
}
/* }}} */

CREX_API crex_class_entry *crex_lookup_class(crex_string *name) /* {{{ */
{
	return crex_lookup_class_ex(name, NULL, 0);
}
/* }}} */

CREX_API crex_class_entry *crex_get_called_scope(crex_execute_data *ex) /* {{{ */
{
	while (ex) {
		if (C_TYPE(ex->This) == IS_OBJECT) {
			return C_OBJCE(ex->This);
		} else if (C_CE(ex->This)) {
			return C_CE(ex->This);
		} else if (ex->func) {
			if (ex->func->type != CREX_INTERNAL_FUNCTION || ex->func->common.scope) {
				return NULL;
			}
		}
		ex = ex->prev_execute_data;
	}
	return NULL;
}
/* }}} */

CREX_API crex_object *crex_get_this_object(crex_execute_data *ex) /* {{{ */
{
	while (ex) {
		if (C_TYPE(ex->This) == IS_OBJECT) {
			return C_OBJ(ex->This);
		} else if (ex->func) {
			if (ex->func->type != CREX_INTERNAL_FUNCTION || ex->func->common.scope) {
				return NULL;
			}
		}
		ex = ex->prev_execute_data;
	}
	return NULL;
}
/* }}} */

CREX_API crex_result crex_eval_stringl(const char *str, size_t str_len, zval *retval_ptr, const char *string_name) /* {{{ */
{
	crex_op_array *new_op_array;
	uint32_t original_compiler_options;
	crex_result retval;
	crex_string *code_str;

	if (retval_ptr) {
		code_str = crex_string_concat3(
			"return ", sizeof("return ")-1, str, str_len, ";", sizeof(";")-1);
	} else {
		code_str = crex_string_init(str, str_len, 0);
	}

	/*printf("Evaluating '%s'\n", pv.value.str.val);*/

	original_compiler_options = CG(compiler_options);
	CG(compiler_options) = CREX_COMPILE_DEFAULT_FOR_EVAL;
	new_op_array = crex_compile_string(code_str, string_name, CREX_COMPILE_POSITION_AFTER_OPEN_TAG);
	CG(compiler_options) = original_compiler_options;

	if (new_op_array) {
		zval local_retval;

		EG(no_extensions)=1;

		new_op_array->scope = crex_get_executed_scope();

		crex_try {
			ZVAL_UNDEF(&local_retval);
			crex_execute(new_op_array, &local_retval);
		} crex_catch {
			destroy_op_array(new_op_array);
			efree_size(new_op_array, sizeof(crex_op_array));
			crex_bailout();
		} crex_end_try();

		if (C_TYPE(local_retval) != IS_UNDEF) {
			if (retval_ptr) {
				ZVAL_COPY_VALUE(retval_ptr, &local_retval);
			} else {
				zval_ptr_dtor(&local_retval);
			}
		} else {
			if (retval_ptr) {
				ZVAL_NULL(retval_ptr);
			}
		}

		EG(no_extensions)=0;
		crex_destroy_static_vars(new_op_array);
		destroy_op_array(new_op_array);
		efree_size(new_op_array, sizeof(crex_op_array));
		retval = SUCCESS;
	} else {
		retval = FAILURE;
	}
	crex_string_release(code_str);
	return retval;
}
/* }}} */

CREX_API crex_result crex_eval_string(const char *str, zval *retval_ptr, const char *string_name) /* {{{ */
{
	return crex_eval_stringl(str, strlen(str), retval_ptr, string_name);
}
/* }}} */

CREX_API crex_result crex_eval_stringl_ex(const char *str, size_t str_len, zval *retval_ptr, const char *string_name, bool handle_exceptions) /* {{{ */
{
	crex_result result;

	result = crex_eval_stringl(str, str_len, retval_ptr, string_name);
	if (handle_exceptions && EG(exception)) {
		result = crex_exception_error(EG(exception), E_ERROR);
	}
	return result;
}
/* }}} */

CREX_API crex_result crex_eval_string_ex(const char *str, zval *retval_ptr, const char *string_name, bool handle_exceptions) /* {{{ */
{
	return crex_eval_stringl_ex(str, strlen(str), retval_ptr, string_name, handle_exceptions);
}
/* }}} */

static void crex_set_timeout_ex(crex_long seconds, bool reset_signals);

CREX_API CREX_NORETURN void CREX_FASTCALL crex_timeout(void) /* {{{ */
{
#if defined(CRX_WIN32)
# ifndef ZTS
	/* No action is needed if we're timed out because zero seconds are
	   just ignored. Also, the hard timeout needs to be respected. If the
	   timer is not restarted properly, it could hang in the shutdown
	   function. */
	if (EG(hard_timeout) > 0) {
		crex_atomic_bool_store_ex(&EG(timed_out), false);
		crex_set_timeout_ex(EG(hard_timeout), 1);
		/* XXX Abused, introduce an additional flag if the value needs to be kept. */
		EG(hard_timeout) = 0;
	}
# endif
#else
	crex_atomic_bool_store_ex(&EG(timed_out), false);
	crex_set_timeout_ex(0, 1);
#endif

	crex_error_noreturn(E_ERROR, "Maximum execution time of " CREX_LONG_FMT " second%s exceeded", EG(timeout_seconds), EG(timeout_seconds) == 1 ? "" : "s");
}
/* }}} */

#ifndef CREX_WIN32
# ifdef CREX_MAX_EXECUTION_TIMERS
static void crex_timeout_handler(int dummy, siginfo_t *si, void *uc) /* {{{ */
{
#ifdef ZTS
	if (!tsrm_is_managed_thread()) {
		fprintf(stderr, "crex_timeout_handler() called in a thread not managed by CRX. The expected signal handler will not be called. This is probably a bug.\n");

		return;
	}
#endif

	if (si->si_value.sival_ptr != &EG(max_execution_timer_timer)) {
#ifdef MAX_EXECUTION_TIMERS_DEBUG
		fprintf(stderr, "Executing previous handler (if set) for unexpected signal SIGRTMIN received on thread %d\n", (pid_t) syscall(SYS_gettid));
#endif

		if (EG(oldact).sa_sigaction) {
			EG(oldact).sa_sigaction(dummy, si, uc);

			return;
		}
		if (EG(oldact).sa_handler) EG(oldact).sa_handler(dummy);

		return;
	}
# else
static void crex_timeout_handler(int dummy) /* {{{ */
{
# endif
#ifdef ZTS
	if (!tsrm_is_managed_thread()) {
		fprintf(stderr, "crex_timeout_handler() called in a thread not managed by CRX. The expected signal handler will not be called. This is probably a bug.\n");

		return;
	}
#else
	if (crex_atomic_bool_load_ex(&EG(timed_out))) {
		/* Die on hard timeout */
		const char *error_filename = NULL;
		uint32_t error_lineno = 0;
		char log_buffer[2048];
		int output_len = 0;

		if (crex_is_compiling()) {
			error_filename = ZSTR_VAL(crex_get_compiled_filename());
			error_lineno = crex_get_compiled_lineno();
		} else if (crex_is_executing()) {
			error_filename = crex_get_executed_filename();
			if (error_filename[0] == '[') { /* [no active file] */
				error_filename = NULL;
				error_lineno = 0;
			} else {
				error_lineno = crex_get_executed_lineno();
			}
		}
		if (!error_filename) {
			error_filename = "Unknown";
		}

		output_len = snprintf(log_buffer, sizeof(log_buffer), "\nFatal error: Maximum execution time of " CREX_LONG_FMT "+" CREX_LONG_FMT " seconds exceeded (terminated) in %s on line %d\n", EG(timeout_seconds), EG(hard_timeout), error_filename, error_lineno);
		if (output_len > 0) {
			crex_quiet_write(2, log_buffer, MIN(output_len, sizeof(log_buffer)));
		}
		_exit(124);
	}
#endif

	if (crex_on_timeout) {
		crex_on_timeout(EG(timeout_seconds));
	}

	crex_atomic_bool_store_ex(&EG(timed_out), true);
	crex_atomic_bool_store_ex(&EG(vm_interrupt), true);

#ifndef ZTS
	if (EG(hard_timeout) > 0) {
		/* Set hard timeout */
		crex_set_timeout_ex(EG(hard_timeout), 1);
	}
#endif
}
/* }}} */
#endif

#ifdef CREX_WIN32
VOID CALLBACK tq_timer_cb(PVOID arg, BOOLEAN timed_out)
{
	crex_executor_globals *eg;

	/* The doc states it'll be always true, however it theoretically
		could be FALSE when the thread was signaled. */
	if (!timed_out) {
		return;
	}

	eg = (crex_executor_globals *)arg;
	crex_atomic_bool_store_ex(&eg->timed_out, true);
	crex_atomic_bool_store_ex(&eg->vm_interrupt, true);
}
#endif

/* This one doesn't exists on QNX */
#ifndef SIGPROF
#define SIGPROF 27
#endif

static void crex_set_timeout_ex(crex_long seconds, bool reset_signals) /* {{{ */
{
#ifdef CREX_WIN32
	crex_executor_globals *eg;

	if (!seconds) {
		return;
	}

	/* Don't use ChangeTimerQueueTimer() as it will not restart an expired
	 * timer, so we could end up with just an ignored timeout. Instead
	 * delete and recreate. */
	if (NULL != tq_timer) {
		if (!DeleteTimerQueueTimer(NULL, tq_timer, INVALID_HANDLE_VALUE)) {
			tq_timer = NULL;
			crex_error_noreturn(E_ERROR, "Could not delete queued timer");
			return;
		}
		tq_timer = NULL;
	}

	/* XXX passing NULL means the default timer queue provided by the system is used */
	eg = CREX_MODULE_GLOBALS_BULK(executor);
	if (!CreateTimerQueueTimer(&tq_timer, NULL, (WAITORTIMERCALLBACK)tq_timer_cb, (VOID*)eg, seconds*1000, 0, WT_EXECUTEONLYONCE)) {
		tq_timer = NULL;
		crex_error_noreturn(E_ERROR, "Could not queue new timer");
		return;
	}
#elif defined(CREX_MAX_EXECUTION_TIMERS)
	crex_max_execution_timer_settime(seconds);

	if (reset_signals) {
		sigset_t sigset;
		struct sigaction act;

		act.sa_sigaction = crex_timeout_handler;
		sigemptyset(&act.sa_mask);
		act.sa_flags = SA_ONSTACK | SA_SIGINFO;
		sigaction(SIGRTMIN, &act, NULL);
		sigemptyset(&sigset);
		sigaddset(&sigset, SIGRTMIN);
		sigprocmask(SIG_UNBLOCK, &sigset, NULL);
	}
#elif defined(HAVE_SETITIMER)
	{
		struct itimerval t_r;		/* timeout requested */
		int signo;

		if(seconds) {
			t_r.it_value.tv_sec = seconds;
			t_r.it_value.tv_usec = t_r.it_interval.tv_sec = t_r.it_interval.tv_usec = 0;

# if defined(__CYGWIN__) || defined(__PASE__)
			setitimer(ITIMER_REAL, &t_r, NULL);
		}
		signo = SIGALRM;
# else
			setitimer(ITIMER_PROF, &t_r, NULL);
		}
		signo = SIGPROF;
# endif

		if (reset_signals) {
# ifdef CREX_SIGNALS
			crex_signal(signo, crex_timeout_handler);
# else
			sigset_t sigset;
#  ifdef HAVE_SIGACTION
			struct sigaction act;

			act.sa_handler = crex_timeout_handler;
			sigemptyset(&act.sa_mask);
			act.sa_flags = SA_ONSTACK | SA_RESETHAND | SA_NODEFER;
			sigaction(signo, &act, NULL);
#  else
			signal(signo, crex_timeout_handler);
#  endif /* HAVE_SIGACTION */
			sigemptyset(&sigset);
			sigaddset(&sigset, signo);
			sigprocmask(SIG_UNBLOCK, &sigset, NULL);
# endif /* CREX_SIGNALS */
		}
	}
#endif /* HAVE_SETITIMER */
}
/* }}} */

void crex_set_timeout(crex_long seconds, bool reset_signals) /* {{{ */
{

	EG(timeout_seconds) = seconds;
	crex_set_timeout_ex(seconds, reset_signals);
	crex_atomic_bool_store_ex(&EG(timed_out), false);
}
/* }}} */

void crex_unset_timeout(void) /* {{{ */
{
#ifdef CREX_WIN32
	if (NULL != tq_timer) {
		if (!DeleteTimerQueueTimer(NULL, tq_timer, INVALID_HANDLE_VALUE)) {
			crex_atomic_bool_store_ex(&EG(timed_out), false);
			tq_timer = NULL;
			crex_error_noreturn(E_ERROR, "Could not delete queued timer");
			return;
		}
		tq_timer = NULL;
	}
#elif CREX_MAX_EXECUTION_TIMERS
	crex_max_execution_timer_settime(0);
#elif defined(HAVE_SETITIMER)
	if (EG(timeout_seconds)) {
		struct itimerval no_timeout;

		no_timeout.it_value.tv_sec = no_timeout.it_value.tv_usec = no_timeout.it_interval.tv_sec = no_timeout.it_interval.tv_usec = 0;

# if defined(__CYGWIN__) || defined(__PASE__)
		setitimer(ITIMER_REAL, &no_timeout, NULL);
# else
		setitimer(ITIMER_PROF, &no_timeout, NULL);
# endif
	}
#endif
	crex_atomic_bool_store_ex(&EG(timed_out), false);
}
/* }}} */

static CREX_COLD void report_class_fetch_error(crex_string *class_name, uint32_t fetch_type)
{
	if (fetch_type & CREX_FETCH_CLASS_SILENT) {
		return;
	}

	if (EG(exception)) {
		if (!(fetch_type & CREX_FETCH_CLASS_EXCEPTION)) {
			crex_exception_uncaught_error("During class fetch");
		}
		return;
	}

	if ((fetch_type & CREX_FETCH_CLASS_MASK) == CREX_FETCH_CLASS_INTERFACE) {
		crex_throw_or_error(fetch_type, NULL, "Interface \"%s\" not found", ZSTR_VAL(class_name));
	} else if ((fetch_type & CREX_FETCH_CLASS_MASK) == CREX_FETCH_CLASS_TRAIT) {
		crex_throw_or_error(fetch_type, NULL, "Trait \"%s\" not found", ZSTR_VAL(class_name));
	} else {
		crex_throw_or_error(fetch_type, NULL, "Class \"%s\" not found", ZSTR_VAL(class_name));
	}
}

crex_class_entry *crex_fetch_class(crex_string *class_name, uint32_t fetch_type) /* {{{ */
{
	crex_class_entry *ce, *scope;
	uint32_t fetch_sub_type = fetch_type & CREX_FETCH_CLASS_MASK;

check_fetch_type:
	switch (fetch_sub_type) {
		case CREX_FETCH_CLASS_SELF:
			scope = crex_get_executed_scope();
			if (UNEXPECTED(!scope)) {
				crex_throw_or_error(fetch_type, NULL, "Cannot access \"self\" when no class scope is active");
			}
			return scope;
		case CREX_FETCH_CLASS_PARENT:
			scope = crex_get_executed_scope();
			if (UNEXPECTED(!scope)) {
				crex_throw_or_error(fetch_type, NULL, "Cannot access \"parent\" when no class scope is active");
				return NULL;
			}
			if (UNEXPECTED(!scope->parent)) {
				crex_throw_or_error(fetch_type, NULL, "Cannot access \"parent\" when current class scope has no parent");
			}
			return scope->parent;
		case CREX_FETCH_CLASS_STATIC:
			ce = crex_get_called_scope(EG(current_execute_data));
			if (UNEXPECTED(!ce)) {
				crex_throw_or_error(fetch_type, NULL, "Cannot access \"static\" when no class scope is active");
				return NULL;
			}
			return ce;
		case CREX_FETCH_CLASS_AUTO: {
				fetch_sub_type = crex_get_class_fetch_type(class_name);
				if (UNEXPECTED(fetch_sub_type != CREX_FETCH_CLASS_DEFAULT)) {
					goto check_fetch_type;
				}
			}
			break;
	}

	ce = crex_lookup_class_ex(class_name, NULL, fetch_type);
	if (!ce) {
		report_class_fetch_error(class_name, fetch_type);
		return NULL;
	}
	return ce;
}
/* }}} */

crex_class_entry *crex_fetch_class_with_scope(
		crex_string *class_name, uint32_t fetch_type, crex_class_entry *scope)
{
	crex_class_entry *ce;
	switch (fetch_type & CREX_FETCH_CLASS_MASK) {
		case CREX_FETCH_CLASS_SELF:
			if (UNEXPECTED(!scope)) {
				crex_throw_or_error(fetch_type, NULL, "Cannot access \"self\" when no class scope is active");
			}
			return scope;
		case CREX_FETCH_CLASS_PARENT:
			if (UNEXPECTED(!scope)) {
				crex_throw_or_error(fetch_type, NULL, "Cannot access \"parent\" when no class scope is active");
				return NULL;
			}
			if (UNEXPECTED(!scope->parent)) {
				crex_throw_or_error(fetch_type, NULL, "Cannot access \"parent\" when current class scope has no parent");
			}
			return scope->parent;
		case 0:
			break;
		/* Other fetch types are not supported by this function. */
		EMPTY_SWITCH_DEFAULT_CASE()
	}

	ce = crex_lookup_class_ex(class_name, NULL, fetch_type);
	if (!ce) {
		report_class_fetch_error(class_name, fetch_type);
		return NULL;
	}
	return ce;
}

crex_class_entry *crex_fetch_class_by_name(crex_string *class_name, crex_string *key, uint32_t fetch_type) /* {{{ */
{
	crex_class_entry *ce = crex_lookup_class_ex(class_name, key, fetch_type);
	if (!ce) {
		report_class_fetch_error(class_name, fetch_type);
		return NULL;
	}
	return ce;
}
/* }}} */

CREX_API crex_result crex_delete_global_variable(crex_string *name) /* {{{ */
{
	return crex_hash_del_ind(&EG(symbol_table), name);
}
/* }}} */

CREX_API crex_array *crex_rebuild_symbol_table(void) /* {{{ */
{
	crex_execute_data *ex;
	crex_array *symbol_table;

	/* Search for last called user function */
	ex = EG(current_execute_data);
	while (ex && (!ex->func || !CREX_USER_CODE(ex->func->common.type))) {
		ex = ex->prev_execute_data;
	}
	if (!ex) {
		return NULL;
	}
	if (CREX_CALL_INFO(ex) & CREX_CALL_HAS_SYMBOL_TABLE) {
		return ex->symbol_table;
	}

	CREX_ADD_CALL_FLAG(ex, CREX_CALL_HAS_SYMBOL_TABLE);
	if (EG(symtable_cache_ptr) > EG(symtable_cache)) {
		symbol_table = ex->symbol_table = *(--EG(symtable_cache_ptr));
		if (!ex->func->op_array.last_var) {
			return symbol_table;
		}
		crex_hash_extend(symbol_table, ex->func->op_array.last_var, 0);
	} else {
		symbol_table = ex->symbol_table = crex_new_array(ex->func->op_array.last_var);
		if (!ex->func->op_array.last_var) {
			return symbol_table;
		}
		crex_hash_real_init_mixed(symbol_table);
		/*printf("Cache miss!  Initialized %x\n", EG(active_symbol_table));*/
	}
	if (EXPECTED(ex->func->op_array.last_var)) {
		crex_string **str = ex->func->op_array.vars;
		crex_string **end = str + ex->func->op_array.last_var;
		zval *var = CREX_CALL_VAR_NUM(ex, 0);

		do {
			_crex_hash_append_ind(symbol_table, *str, var);
			str++;
			var++;
		} while (str != end);
	}
	return symbol_table;
}
/* }}} */

CREX_API void crex_attach_symbol_table(crex_execute_data *execute_data) /* {{{ */
{
	crex_op_array *op_array = &execute_data->func->op_array;
	HashTable *ht = execute_data->symbol_table;

	/* copy real values from symbol table into CV slots and create
	   INDIRECT references to CV in symbol table  */
	if (EXPECTED(op_array->last_var)) {
		crex_string **str = op_array->vars;
		crex_string **end = str + op_array->last_var;
		zval *var = EX_VAR_NUM(0);

		do {
			zval *zv = crex_hash_find_known_hash(ht, *str);

			if (zv) {
				if (C_TYPE_P(zv) == IS_INDIRECT) {
					zval *val = C_INDIRECT_P(zv);

					ZVAL_COPY_VALUE(var, val);
				} else {
					ZVAL_COPY_VALUE(var, zv);
				}
			} else {
				ZVAL_UNDEF(var);
				zv = crex_hash_add_new(ht, *str, var);
			}
			ZVAL_INDIRECT(zv, var);
			str++;
			var++;
		} while (str != end);
	}
}
/* }}} */

CREX_API void crex_detach_symbol_table(crex_execute_data *execute_data) /* {{{ */
{
	crex_op_array *op_array = &execute_data->func->op_array;
	HashTable *ht = execute_data->symbol_table;

	/* copy real values from CV slots into symbol table */
	if (EXPECTED(op_array->last_var)) {
		crex_string **str = op_array->vars;
		crex_string **end = str + op_array->last_var;
		zval *var = EX_VAR_NUM(0);

		do {
			if (C_TYPE_P(var) == IS_UNDEF) {
				crex_hash_del(ht, *str);
			} else {
				crex_hash_update(ht, *str, var);
				ZVAL_UNDEF(var);
			}
			str++;
			var++;
		} while (str != end);
	}
}
/* }}} */

CREX_API crex_result crex_set_local_var(crex_string *name, zval *value, bool force) /* {{{ */
{
	crex_execute_data *execute_data = EG(current_execute_data);

	while (execute_data && (!execute_data->func || !CREX_USER_CODE(execute_data->func->common.type))) {
		execute_data = execute_data->prev_execute_data;
	}

	if (execute_data) {
		if (!(EX_CALL_INFO() & CREX_CALL_HAS_SYMBOL_TABLE)) {
			crex_ulong h = crex_string_hash_val(name);
			crex_op_array *op_array = &execute_data->func->op_array;

			if (EXPECTED(op_array->last_var)) {
				crex_string **str = op_array->vars;
				crex_string **end = str + op_array->last_var;

				do {
					if (ZSTR_H(*str) == h &&
					    crex_string_equal_content(*str, name)) {
						zval *var = EX_VAR_NUM(str - op_array->vars);
						ZVAL_COPY_VALUE(var, value);
						return SUCCESS;
					}
					str++;
				} while (str != end);
			}
			if (force) {
				crex_array *symbol_table = crex_rebuild_symbol_table();
				if (symbol_table) {
					crex_hash_update(symbol_table, name, value);
					return SUCCESS;
				}
			}
		} else {
			crex_hash_update_ind(execute_data->symbol_table, name, value);
			return SUCCESS;
		}
	}
	return FAILURE;
}
/* }}} */

CREX_API crex_result crex_set_local_var_str(const char *name, size_t len, zval *value, bool force) /* {{{ */
{
	crex_execute_data *execute_data = EG(current_execute_data);

	while (execute_data && (!execute_data->func || !CREX_USER_CODE(execute_data->func->common.type))) {
		execute_data = execute_data->prev_execute_data;
	}

	if (execute_data) {
		if (!(EX_CALL_INFO() & CREX_CALL_HAS_SYMBOL_TABLE)) {
			crex_ulong h = crex_hash_func(name, len);
			crex_op_array *op_array = &execute_data->func->op_array;
			if (EXPECTED(op_array->last_var)) {
				crex_string **str = op_array->vars;
				crex_string **end = str + op_array->last_var;

				do {
					if (ZSTR_H(*str) == h &&
					    crex_string_equals_cstr(*str, name, len)) {
						zval *var = EX_VAR_NUM(str - op_array->vars);
						zval_ptr_dtor(var);
						ZVAL_COPY_VALUE(var, value);
						return SUCCESS;
					}
					str++;
				} while (str != end);
			}
			if (force) {
				crex_array *symbol_table = crex_rebuild_symbol_table();
				if (symbol_table) {
					crex_hash_str_update(symbol_table, name, len, value);
					return SUCCESS;
				}
			}
		} else {
			crex_hash_str_update_ind(execute_data->symbol_table, name, len, value);
			return SUCCESS;
		}
	}
	return FAILURE;
}
/* }}} */
