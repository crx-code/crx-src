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
   | Authors: Zeev Suraski <zeev@crx.net>                                 |
   |          Thies C. Arntzen <thies@thieso.net>                         |
   |          Marcus Boerger <helly@crx.net>                              |
   | New API: Michael Wallner <mike@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_OUTPUT_DEBUG
# define CRX_OUTPUT_DEBUG 0
#endif
#ifndef CRX_OUTPUT_NOINLINE
# define CRX_OUTPUT_NOINLINE 0
#endif

#include "crx.h"
#include "ext/standard/head.h"
#include "ext/standard/url_scanner_ex.h"
#include "SAPI.h"
#include "crex_stack.h"
#include "crx_output.h"

CRXAPI CREX_DECLARE_MODULE_GLOBALS(output)

const char crx_output_default_handler_name[sizeof("default output handler")] = "default output handler";
const char crx_output_devnull_handler_name[sizeof("null output handler")] = "null output handler";

#if CRX_OUTPUT_NOINLINE || CRX_OUTPUT_DEBUG
# undef inline
# define inline
#endif

/* {{{ aliases, conflict and reverse conflict hash tables */
static HashTable crx_output_handler_aliases;
static HashTable crx_output_handler_conflicts;
static HashTable crx_output_handler_reverse_conflicts;
/* }}} */

/* {{{ forward declarations */
static inline int crx_output_lock_error(int op);
static inline void crx_output_op(int op, const char *str, size_t len);

static inline crx_output_handler *crx_output_handler_init(crex_string *name, size_t chunk_size, int flags);
static inline crx_output_handler_status_t crx_output_handler_op(crx_output_handler *handler, crx_output_context *context);
static inline int crx_output_handler_append(crx_output_handler *handler, const crx_output_buffer *buf);
static inline zval *crx_output_handler_status(crx_output_handler *handler, zval *entry);

static inline void crx_output_context_init(crx_output_context *context, int op);
static inline void crx_output_context_reset(crx_output_context *context);
static inline void crx_output_context_swap(crx_output_context *context);
static inline void crx_output_context_dtor(crx_output_context *context);

static int crx_output_stack_pop(int flags);

static int crx_output_stack_apply_op(void *h, void *c);
static int crx_output_stack_apply_clean(void *h, void *c);
static int crx_output_stack_apply_list(void *h, void *z);
static int crx_output_stack_apply_status(void *h, void *z);

static int crx_output_handler_compat_func(void **handler_context, crx_output_context *output_context);
static int crx_output_handler_default_func(void **handler_context, crx_output_context *output_context);
static int crx_output_handler_devnull_func(void **handler_context, crx_output_context *output_context);
/* }}} */

/* {{{ static void crx_output_init_globals(crex_output_globals *G)
 * Initialize the module globals on MINIT */
static inline void crx_output_init_globals(crex_output_globals *G)
{
	memset(G, 0, sizeof(*G));
}
/* }}} */

/* {{{ stderr/stdout writer if not CRX_OUTPUT_ACTIVATED */
static size_t crx_output_stdout(const char *str, size_t str_len)
{
	fwrite(str, 1, str_len, stdout);
	return str_len;
}
static size_t crx_output_stderr(const char *str, size_t str_len)
{
	fwrite(str, 1, str_len, stderr);
/* See http://support.microsoft.com/kb/190351 */
#ifdef CRX_WIN32
	fflush(stderr);
#endif
	return str_len;
}
static size_t (*crx_output_direct)(const char *str, size_t str_len) = crx_output_stderr;
/* }}} */

/* {{{ void crx_output_header(void) */
static void crx_output_header(void)
{
	if (!SG(headers_sent)) {
		if (!OG(output_start_filename)) {
			if (crex_is_compiling()) {
				OG(output_start_filename) = crex_get_compiled_filename();
				OG(output_start_lineno) = crex_get_compiled_lineno();
			} else if (crex_is_executing()) {
				OG(output_start_filename) = crex_get_executed_filename_ex();
				OG(output_start_lineno) = crex_get_executed_lineno();
			}
			if (OG(output_start_filename)) {
				crex_string_addref(OG(output_start_filename));
			}
#if CRX_OUTPUT_DEBUG
			fprintf(stderr, "!!! output started at: %s (%d)\n",
				ZSTR_VAL(OG(output_start_filename)), OG(output_start_lineno));
#endif
		}
		if (!crx_header()) {
			OG(flags) |= CRX_OUTPUT_DISABLED;
		}
	}
}
/* }}} */

static void reverse_conflict_dtor(zval *zv)
{
	HashTable *ht = C_PTR_P(zv);
	crex_hash_destroy(ht);
}

/* {{{ void crx_output_startup(void)
 * Set up module globals and initialize the conflict and reverse conflict hash tables */
CRXAPI void crx_output_startup(void)
{
	CREX_INIT_MODULE_GLOBALS(output, crx_output_init_globals, NULL);
	crex_hash_init(&crx_output_handler_aliases, 8, NULL, NULL, 1);
	crex_hash_init(&crx_output_handler_conflicts, 8, NULL, NULL, 1);
	crex_hash_init(&crx_output_handler_reverse_conflicts, 8, NULL, reverse_conflict_dtor, 1);
	crx_output_direct = crx_output_stdout;
}
/* }}} */

/* {{{ void crx_output_shutdown(void)
 * Destroy module globals and the conflict and reverse conflict hash tables */
CRXAPI void crx_output_shutdown(void)
{
	crx_output_direct = crx_output_stderr;
	crex_hash_destroy(&crx_output_handler_aliases);
	crex_hash_destroy(&crx_output_handler_conflicts);
	crex_hash_destroy(&crx_output_handler_reverse_conflicts);
}
/* }}} */

/* {{{ SUCCESS|FAILURE crx_output_activate(void)
 * Reset output globals and set up the output handler stack */
CRXAPI int crx_output_activate(void)
{
#ifdef ZTS
	memset(TSRMG_BULK_STATIC(output_globals_id, crex_output_globals*), 0, sizeof(crex_output_globals));
#else
	memset(&output_globals, 0, sizeof(crex_output_globals));
#endif

	crex_stack_init(&OG(handlers), sizeof(crx_output_handler *));
	OG(flags) |= CRX_OUTPUT_ACTIVATED;

	return SUCCESS;
}
/* }}} */

/* {{{ void crx_output_deactivate(void)
 * Destroy the output handler stack */
CRXAPI void crx_output_deactivate(void)
{
	crx_output_handler **handler = NULL;

	if ((OG(flags) & CRX_OUTPUT_ACTIVATED)) {
		crx_output_header();

		OG(flags) ^= CRX_OUTPUT_ACTIVATED;
		OG(active) = NULL;
		OG(running) = NULL;

		/* release all output handlers */
		if (OG(handlers).elements) {
			while ((handler = crex_stack_top(&OG(handlers)))) {
				crx_output_handler_free(handler);
				crex_stack_del_top(&OG(handlers));
			}
		}
		crex_stack_destroy(&OG(handlers));
	}

	if (OG(output_start_filename)) {
		crex_string_release(OG(output_start_filename));
		OG(output_start_filename) = NULL;
	}
}
/* }}} */

/* {{{ void crx_output_set_status(int status)
 * Used by SAPIs to disable output */
CRXAPI void crx_output_set_status(int status)
{
	OG(flags) = (OG(flags) & ~0xf) | (status & 0xf);
}
/* }}} */

/* {{{ int crx_output_get_status()
 * Get output control status */
CRXAPI int crx_output_get_status(void)
{
	return (
		OG(flags)
		|	(OG(active) ? CRX_OUTPUT_ACTIVE : 0)
		|	(OG(running)? CRX_OUTPUT_LOCKED : 0)
	) & 0xff;
}
/* }}} */

/* {{{ int crx_output_write_unbuffered(const char *str, size_t len)
 * Unbuffered write */
CRXAPI size_t crx_output_write_unbuffered(const char *str, size_t len)
{
	if (OG(flags) & CRX_OUTPUT_ACTIVATED) {
		return sapi_module.ub_write(str, len);
	}
	return crx_output_direct(str, len);
}
/* }}} */

/* {{{ int crx_output_write(const char *str, size_t len)
 * Buffered write */
CRXAPI size_t crx_output_write(const char *str, size_t len)
{
	if (OG(flags) & CRX_OUTPUT_ACTIVATED) {
		crx_output_op(CRX_OUTPUT_HANDLER_WRITE, str, len);
		return len;
	}
	if (OG(flags) & CRX_OUTPUT_DISABLED) {
		return 0;
	}
	return crx_output_direct(str, len);
}
/* }}} */

/* {{{ SUCCESS|FAILURE crx_output_flush(void)
 * Flush the most recent output handlers buffer */
CRXAPI int crx_output_flush(void)
{
	crx_output_context context;

	if (OG(active) && (OG(active)->flags & CRX_OUTPUT_HANDLER_FLUSHABLE)) {
		crx_output_context_init(&context, CRX_OUTPUT_HANDLER_FLUSH);
		crx_output_handler_op(OG(active), &context);
		if (context.out.data && context.out.used) {
			crex_stack_del_top(&OG(handlers));
			crx_output_write(context.out.data, context.out.used);
			crex_stack_push(&OG(handlers), &OG(active));
		}
		crx_output_context_dtor(&context);
		return SUCCESS;
	}
	return FAILURE;
}
/* }}} */

/* {{{ void crx_output_flush_all()
 * Flush all output buffers subsequently */
CRXAPI void crx_output_flush_all(void)
{
	if (OG(active)) {
		crx_output_op(CRX_OUTPUT_HANDLER_FLUSH, NULL, 0);
	}
}
/* }}} */

/* {{{ SUCCESS|FAILURE crx_output_clean(void)
 * Cleans the most recent output handlers buffer if the handler is cleanable */
CRXAPI int crx_output_clean(void)
{
	crx_output_context context;

	if (OG(active) && (OG(active)->flags & CRX_OUTPUT_HANDLER_CLEANABLE)) {
		crx_output_context_init(&context, CRX_OUTPUT_HANDLER_CLEAN);
		crx_output_handler_op(OG(active), &context);
		crx_output_context_dtor(&context);
		return SUCCESS;
	}
	return FAILURE;
}
/* }}} */

/* {{{ void crx_output_clean_all(void)
 * Cleans all output handler buffers, without regard whether the handler is cleanable */
CRXAPI void crx_output_clean_all(void)
{
	crx_output_context context;

	if (OG(active)) {
		crx_output_context_init(&context, CRX_OUTPUT_HANDLER_CLEAN);
		crex_stack_apply_with_argument(&OG(handlers), CREX_STACK_APPLY_TOPDOWN, crx_output_stack_apply_clean, &context);
	}
}

/* {{{ SUCCESS|FAILURE crx_output_end(void)
 * Finalizes the most recent output handler at pops it off the stack if the handler is removable */
CRXAPI int crx_output_end(void)
{
	if (crx_output_stack_pop(CRX_OUTPUT_POP_TRY)) {
		return SUCCESS;
	}
	return FAILURE;
}
/* }}} */

/* {{{ void crx_output_end_all(void)
 * Finalizes all output handlers and ends output buffering without regard whether a handler is removable */
CRXAPI void crx_output_end_all(void)
{
	while (OG(active) && crx_output_stack_pop(CRX_OUTPUT_POP_FORCE));
}
/* }}} */

/* {{{ SUCCESS|FAILURE crx_output_discard(void)
 * Discards the most recent output handlers buffer and pops it off the stack if the handler is removable */
CRXAPI int crx_output_discard(void)
{
	if (crx_output_stack_pop(CRX_OUTPUT_POP_DISCARD|CRX_OUTPUT_POP_TRY)) {
		return SUCCESS;
	}
	return FAILURE;
}
/* }}} */

/* {{{ void crx_output_discard_all(void)
 * Discard all output handlers and buffers without regard whether a handler is removable */
CRXAPI void crx_output_discard_all(void)
{
	while (OG(active)) {
		crx_output_stack_pop(CRX_OUTPUT_POP_DISCARD|CRX_OUTPUT_POP_FORCE);
	}
}
/* }}} */

/* {{{ int crx_output_get_level(void)
 * Get output buffering level, i.e. how many output handlers the stack contains */
CRXAPI int crx_output_get_level(void)
{
	return OG(active) ? crex_stack_count(&OG(handlers)) : 0;
}
/* }}} */

/* {{{ SUCCESS|FAILURE crx_output_get_contents(zval *z)
 * Get the contents of the active output handlers buffer */
CRXAPI int crx_output_get_contents(zval *p)
{
	if (OG(active)) {
		ZVAL_STRINGL(p, OG(active)->buffer.data, OG(active)->buffer.used);
		return SUCCESS;
	} else {
		ZVAL_NULL(p);
		return FAILURE;
	}
}

/* {{{ SUCCESS|FAILURE crx_output_get_length(zval *z)
 * Get the length of the active output handlers buffer */
CRXAPI int crx_output_get_length(zval *p)
{
	if (OG(active)) {
		ZVAL_LONG(p, OG(active)->buffer.used);
		return SUCCESS;
	} else {
		ZVAL_NULL(p);
		return FAILURE;
	}
}
/* }}} */

/* {{{ crx_output_handler* crx_output_get_active_handler(void)
 * Get active output handler */
CRXAPI crx_output_handler* crx_output_get_active_handler(void)
{
	return OG(active);
}
/* }}} */

/* {{{ SUCCESS|FAILURE crx_output_handler_start_default(void)
 * Start a "default output handler" */
CRXAPI int crx_output_start_default(void)
{
	crx_output_handler *handler;

	handler = crx_output_handler_create_internal(CREX_STRL(crx_output_default_handler_name), crx_output_handler_default_func, 0, CRX_OUTPUT_HANDLER_STDFLAGS);
	if (SUCCESS == crx_output_handler_start(handler)) {
		return SUCCESS;
	}
	crx_output_handler_free(&handler);
	return FAILURE;
}
/* }}} */

/* {{{ SUCCESS|FAILURE crx_output_handler_start_devnull(void)
 * Start a "null output handler" */
CRXAPI int crx_output_start_devnull(void)
{
	crx_output_handler *handler;

	handler = crx_output_handler_create_internal(CREX_STRL(crx_output_devnull_handler_name), crx_output_handler_devnull_func, CRX_OUTPUT_HANDLER_DEFAULT_SIZE, 0);
	if (SUCCESS == crx_output_handler_start(handler)) {
		return SUCCESS;
	}
	crx_output_handler_free(&handler);
	return FAILURE;
}
/* }}} */

/* {{{ SUCCESS|FAILURE crx_output_start_user(zval *handler, size_t chunk_size, int flags)
 * Start a user level output handler */
CRXAPI int crx_output_start_user(zval *output_handler, size_t chunk_size, int flags)
{
	crx_output_handler *handler;

	if (output_handler) {
		handler = crx_output_handler_create_user(output_handler, chunk_size, flags);
	} else {
		handler = crx_output_handler_create_internal(CREX_STRL(crx_output_default_handler_name), crx_output_handler_default_func, chunk_size, flags);
	}
	if (SUCCESS == crx_output_handler_start(handler)) {
		return SUCCESS;
	}
	crx_output_handler_free(&handler);
	return FAILURE;
}
/* }}} */

/* {{{ SUCCESS|FAILURE crx_output_start_internal(zval *name, crx_output_handler_func_t handler, size_t chunk_size, int flags)
 * Start an internal output handler that does not have to maintain a non-global state */
CRXAPI int crx_output_start_internal(const char *name, size_t name_len, crx_output_handler_func_t output_handler, size_t chunk_size, int flags)
{
	crx_output_handler *handler;

	handler = crx_output_handler_create_internal(name, name_len, crx_output_handler_compat_func, chunk_size, flags);
	crx_output_handler_set_context(handler, output_handler, NULL);
	if (SUCCESS == crx_output_handler_start(handler)) {
		return SUCCESS;
	}
	crx_output_handler_free(&handler);
	return FAILURE;
}
/* }}} */

/* {{{ crx_output_handler *crx_output_handler_create_user(zval *handler, size_t chunk_size, int flags)
 * Create a user level output handler */
CRXAPI crx_output_handler *crx_output_handler_create_user(zval *output_handler, size_t chunk_size, int flags)
{
	crex_string *handler_name = NULL;
	char *error = NULL;
	crx_output_handler *handler = NULL;
	crx_output_handler_alias_ctor_t alias = NULL;
	crx_output_handler_user_func_t *user = NULL;

	switch (C_TYPE_P(output_handler)) {
		case IS_NULL:
			handler = crx_output_handler_create_internal(CREX_STRL(crx_output_default_handler_name), crx_output_handler_default_func, chunk_size, flags);
			break;
		case IS_STRING:
			if (C_STRLEN_P(output_handler) && (alias = crx_output_handler_alias(C_STRVAL_P(output_handler), C_STRLEN_P(output_handler)))) {
				handler = alias(C_STRVAL_P(output_handler), C_STRLEN_P(output_handler), chunk_size, flags);
				break;
			}
			CREX_FALLTHROUGH;
		default:
			user = ecalloc(1, sizeof(crx_output_handler_user_func_t));
			if (SUCCESS == crex_fcall_info_init(output_handler, 0, &user->fci, &user->fcc, &handler_name, &error)) {
				handler = crx_output_handler_init(handler_name, chunk_size, (flags & ~0xf) | CRX_OUTPUT_HANDLER_USER);
				ZVAL_COPY(&user->zoh, output_handler);
				handler->func.user = user;
			} else {
				efree(user);
			}
			if (error) {
				crx_error_docref("ref.outcontrol", E_WARNING, "%s", error);
				efree(error);
			}
			if (handler_name) {
				crex_string_release_ex(handler_name, 0);
			}
	}

	return handler;
}
/* }}} */

/* {{{ crx_output_handler *crx_output_handler_create_internal(zval *name, crx_output_handler_context_func_t handler, size_t chunk_size, int flags)
 * Create an internal output handler that can maintain a non-global state */
CRXAPI crx_output_handler *crx_output_handler_create_internal(const char *name, size_t name_len, crx_output_handler_context_func_t output_handler, size_t chunk_size, int flags)
{
	crx_output_handler *handler;
	crex_string *str = crex_string_init(name, name_len, 0);

	handler = crx_output_handler_init(str, chunk_size, (flags & ~0xf) | CRX_OUTPUT_HANDLER_INTERNAL);
	handler->func.internal = output_handler;
	crex_string_release_ex(str, 0);

	return handler;
}
/* }}} */

/* {{{ void crx_output_handler_set_context(crx_output_handler *handler, void *opaq, void (*dtor)(void*))
 * Set the context/state of an output handler. Calls the dtor of the previous context if there is one */
CRXAPI void crx_output_handler_set_context(crx_output_handler *handler, void *opaq, void (*dtor)(void*))
{
	if (handler->dtor && handler->opaq) {
		handler->dtor(handler->opaq);
	}
	handler->dtor = dtor;
	handler->opaq = opaq;
}
/* }}} */

/* {{{ SUCCESS|FAILURE crx_output_handler_start(crx_output_handler *handler)
 * Starts the set up output handler and pushes it on top of the stack. Checks for any conflicts regarding the output handler to start */
CRXAPI int crx_output_handler_start(crx_output_handler *handler)
{
	HashTable *rconflicts;
	crx_output_handler_conflict_check_t conflict;

	if (crx_output_lock_error(CRX_OUTPUT_HANDLER_START) || !handler) {
		return FAILURE;
	}
	if (NULL != (conflict = crex_hash_find_ptr(&crx_output_handler_conflicts, handler->name))) {
		if (SUCCESS != conflict(ZSTR_VAL(handler->name), ZSTR_LEN(handler->name))) {
			return FAILURE;
		}
	}
	if (NULL != (rconflicts = crex_hash_find_ptr(&crx_output_handler_reverse_conflicts, handler->name))) {
		CREX_HASH_PACKED_FOREACH_PTR(rconflicts, conflict) {
			if (SUCCESS != conflict(ZSTR_VAL(handler->name), ZSTR_LEN(handler->name))) {
				return FAILURE;
			}
		} CREX_HASH_FOREACH_END();
	}
	/* crex_stack_push returns stack level */
	handler->level = crex_stack_push(&OG(handlers), &handler);
	OG(active) = handler;
	return SUCCESS;
}
/* }}} */

/* {{{ int crx_output_handler_started(zval *name)
 * Check whether a certain output handler is in use */
CRXAPI int crx_output_handler_started(const char *name, size_t name_len)
{
	crx_output_handler **handlers;
	int i, count = crx_output_get_level();

	if (count) {
		handlers = (crx_output_handler **) crex_stack_base(&OG(handlers));

		for (i = 0; i < count; ++i) {
			if (crex_string_equals_cstr(handlers[i]->name, name, name_len)) {
				return 1;
			}
		}
	}

	return 0;
}
/* }}} */

/* {{{ int crx_output_handler_conflict(zval *handler_new, zval *handler_old)
 * Check whether a certain handler is in use and issue a warning that the new handler would conflict with the already used one */
CRXAPI int crx_output_handler_conflict(const char *handler_new, size_t handler_new_len, const char *handler_set, size_t handler_set_len)
{
	if (crx_output_handler_started(handler_set, handler_set_len)) {
		if (handler_new_len != handler_set_len || memcmp(handler_new, handler_set, handler_set_len)) {
			crx_error_docref("ref.outcontrol", E_WARNING, "Output handler '%s' conflicts with '%s'", handler_new, handler_set);
		} else {
			crx_error_docref("ref.outcontrol", E_WARNING, "Output handler '%s' cannot be used twice", handler_new);
		}
		return 1;
	}
	return 0;
}
/* }}} */

/* {{{ SUCCESS|FAILURE crx_output_handler_conflict_register(zval *name, crx_output_handler_conflict_check_t check_func)
 * Register a conflict checking function on MINIT */
CRXAPI int crx_output_handler_conflict_register(const char *name, size_t name_len, crx_output_handler_conflict_check_t check_func)
{
	crex_string *str;

	if (!EG(current_module)) {
		crex_error(E_ERROR, "Cannot register an output handler conflict outside of MINIT");
		return FAILURE;
	}
	str = crex_string_init_interned(name, name_len, 1);
	crex_hash_update_ptr(&crx_output_handler_conflicts, str, check_func);
	crex_string_release_ex(str, 1);
	return SUCCESS;
}
/* }}} */

/* {{{ SUCCESS|FAILURE crx_output_handler_reverse_conflict_register(zval *name, crx_output_handler_conflict_check_t check_func)
 * Register a reverse conflict checking function on MINIT */
CRXAPI int crx_output_handler_reverse_conflict_register(const char *name, size_t name_len, crx_output_handler_conflict_check_t check_func)
{
	HashTable rev, *rev_ptr = NULL;

	if (!EG(current_module)) {
		crex_error(E_ERROR, "Cannot register a reverse output handler conflict outside of MINIT");
		return FAILURE;
	}

	if (NULL != (rev_ptr = crex_hash_str_find_ptr(&crx_output_handler_reverse_conflicts, name, name_len))) {
		return crex_hash_next_index_insert_ptr(rev_ptr, check_func) ? SUCCESS : FAILURE;
	} else {
		crex_string *str;

		crex_hash_init(&rev, 8, NULL, NULL, 1);
		if (NULL == crex_hash_next_index_insert_ptr(&rev, check_func)) {
			crex_hash_destroy(&rev);
			return FAILURE;
		}
		str = crex_string_init_interned(name, name_len, 1);
		crex_hash_update_mem(&crx_output_handler_reverse_conflicts, str, &rev, sizeof(HashTable));
		crex_string_release_ex(str, 1);
		return SUCCESS;
	}
}
/* }}} */

/* {{{ crx_output_handler_alias_ctor_t crx_output_handler_alias(zval *name)
 * Get an internal output handler for a user handler if it exists */
CRXAPI crx_output_handler_alias_ctor_t crx_output_handler_alias(const char *name, size_t name_len)
{
	return crex_hash_str_find_ptr(&crx_output_handler_aliases, name, name_len);
}
/* }}} */

/* {{{ SUCCESS|FAILURE crx_output_handler_alias_register(zval *name, crx_output_handler_alias_ctor_t func)
 * Registers an internal output handler as alias for a user handler */
CRXAPI int crx_output_handler_alias_register(const char *name, size_t name_len, crx_output_handler_alias_ctor_t func)
{
	crex_string *str;

	if (!EG(current_module)) {
		crex_error(E_ERROR, "Cannot register an output handler alias outside of MINIT");
		return FAILURE;
	}
	str = crex_string_init_interned(name, name_len, 1);
	crex_hash_update_ptr(&crx_output_handler_aliases, str, func);
	crex_string_release_ex(str, 1);
	return SUCCESS;
}
/* }}} */

/* {{{ SUCCESS|FAILURE crx_output_handler_hook(crx_output_handler_hook_t type, void *arg)
 * Output handler hook for output handler functions to check/modify the current handlers abilities */
CRXAPI int crx_output_handler_hook(crx_output_handler_hook_t type, void *arg)
{
	if (OG(running)) {
		switch (type) {
			case CRX_OUTPUT_HANDLER_HOOK_GET_OPAQ:
				*(void ***) arg = &OG(running)->opaq;
				return SUCCESS;
			case CRX_OUTPUT_HANDLER_HOOK_GET_FLAGS:
				*(int *) arg = OG(running)->flags;
				return SUCCESS;
			case CRX_OUTPUT_HANDLER_HOOK_GET_LEVEL:
				*(int *) arg = OG(running)->level;
				return SUCCESS;
			case CRX_OUTPUT_HANDLER_HOOK_IMMUTABLE:
				OG(running)->flags &= ~(CRX_OUTPUT_HANDLER_REMOVABLE|CRX_OUTPUT_HANDLER_CLEANABLE);
				return SUCCESS;
			case CRX_OUTPUT_HANDLER_HOOK_DISABLE:
				OG(running)->flags |= CRX_OUTPUT_HANDLER_DISABLED;
				return SUCCESS;
			default:
				break;
		}
	}
	return FAILURE;
}
/* }}} */

/* {{{ void crx_output_handler_dtor(crx_output_handler *handler)
 * Destroy an output handler */
CRXAPI void crx_output_handler_dtor(crx_output_handler *handler)
{
	if (handler->name) {
		crex_string_release_ex(handler->name, 0);
	}
	if (handler->buffer.data) {
		efree(handler->buffer.data);
	}
	if (handler->flags & CRX_OUTPUT_HANDLER_USER) {
		zval_ptr_dtor(&handler->func.user->zoh);
		efree(handler->func.user);
	}
	if (handler->dtor && handler->opaq) {
		handler->dtor(handler->opaq);
	}
	memset(handler, 0, sizeof(*handler));
}
/* }}} */

/* {{{ void crx_output_handler_free(crx_output_handler **handler)
 * Destroy and free an output handler */
CRXAPI void crx_output_handler_free(crx_output_handler **h)
{
	if (*h) {
		crx_output_handler_dtor(*h);
		efree(*h);
		*h = NULL;
	}
}
/* }}} */

/* void crx_output_set_implicit_flush(int enabled)
 * Enable or disable implicit flush */
CRXAPI void crx_output_set_implicit_flush(int flush)
{
	if (flush) {
		OG(flags) |= CRX_OUTPUT_IMPLICITFLUSH;
	} else {
		OG(flags) &= ~CRX_OUTPUT_IMPLICITFLUSH;
	}
}
/* }}} */

/* {{{ char *crx_output_get_start_filename(void)
 * Get the file name where output has started */
CRXAPI const char *crx_output_get_start_filename(void)
{
	return OG(output_start_filename) ? ZSTR_VAL(OG(output_start_filename)) : NULL;
}
/* }}} */

/* {{{ int crx_output_get_start_lineno(void)
 * Get the line number where output has started */
CRXAPI int crx_output_get_start_lineno(void)
{
	return OG(output_start_lineno);
}
/* }}} */

/* {{{ static int crx_output_lock_error(int op)
 * Checks whether an unallowed operation is attempted from within the output handler and issues a fatal error */
static inline int crx_output_lock_error(int op)
{
	/* if there's no ob active, ob has been stopped */
	if (op && OG(active) && OG(running)) {
		/* fatal error */
		crx_output_deactivate();
		crx_error_docref("ref.outcontrol", E_ERROR, "Cannot use output buffering in output buffering display handlers");
		return 1;
	}
	return 0;
}
/* }}} */

/* {{{ static crx_output_context *crx_output_context_init(crx_output_context *context, int op)
 * Initialize a new output context */
static inline void crx_output_context_init(crx_output_context *context, int op)
{
	memset(context, 0, sizeof(crx_output_context));
	context->op = op;
}
/* }}} */

/* {{{ static void crx_output_context_reset(crx_output_context *context)
 * Reset an output context */
static inline void crx_output_context_reset(crx_output_context *context)
{
	int op = context->op;
	crx_output_context_dtor(context);
	memset(context, 0, sizeof(crx_output_context));
	context->op = op;
}
/* }}} */

/* {{{ static void crx_output_context_feed(crx_output_context *context, char *, size_t, size_t)
 * Feed output contexts input buffer */
static inline void crx_output_context_feed(crx_output_context *context, char *data, size_t size, size_t used, bool free)
{
	if (context->in.free && context->in.data) {
		efree(context->in.data);
	}
	context->in.data = data;
	context->in.used = used;
	context->in.free = free;
	context->in.size = size;
}
/* }}} */

/* {{{ static void crx_output_context_swap(crx_output_context *context)
 * Swap output contexts buffers */
static inline void crx_output_context_swap(crx_output_context *context)
{
	if (context->in.free && context->in.data) {
		efree(context->in.data);
	}
	context->in.data = context->out.data;
	context->in.used = context->out.used;
	context->in.free = context->out.free;
	context->in.size = context->out.size;
	context->out.data = NULL;
	context->out.used = 0;
	context->out.free = 0;
	context->out.size = 0;
}
/* }}} */

/* {{{ static void crx_output_context_pass(crx_output_context *context)
 * Pass input to output buffer */
static inline void crx_output_context_pass(crx_output_context *context)
{
	context->out.data = context->in.data;
	context->out.used = context->in.used;
	context->out.size = context->in.size;
	context->out.free = context->in.free;
	context->in.data = NULL;
	context->in.used = 0;
	context->in.free = 0;
	context->in.size = 0;
}
/* }}} */

/* {{{ static void crx_output_context_dtor(crx_output_context *context)
 * Destroy the contents of an output context */
static inline void crx_output_context_dtor(crx_output_context *context)
{
	if (context->in.free && context->in.data) {
		efree(context->in.data);
		context->in.data = NULL;
	}
	if (context->out.free && context->out.data) {
		efree(context->out.data);
		context->out.data = NULL;
	}
}
/* }}} */

/* {{{ static crx_output_handler *crx_output_handler_init(zval *name, size_t chunk_size, int flags)
 * Allocates and initializes a crx_output_handler structure */
static inline crx_output_handler *crx_output_handler_init(crex_string *name, size_t chunk_size, int flags)
{
	crx_output_handler *handler;

	handler = ecalloc(1, sizeof(crx_output_handler));
	handler->name = crex_string_copy(name);
	handler->size = chunk_size;
	handler->flags = flags;
	handler->buffer.size = CRX_OUTPUT_HANDLER_INITBUF_SIZE(chunk_size);
	handler->buffer.data = emalloc(handler->buffer.size);

	return handler;
}
/* }}} */

/* {{{ static int crx_output_handler_append(crx_output_handler *handler, const crx_output_buffer *buf)
 * Appends input to the output handlers buffer and indicates whether the buffer does not have to be processed by the output handler */
static inline int crx_output_handler_append(crx_output_handler *handler, const crx_output_buffer *buf)
{
	if (buf->used) {
		OG(flags) |= CRX_OUTPUT_WRITTEN;
		/* store it away */
		if ((handler->buffer.size - handler->buffer.used) <= buf->used) {
			size_t grow_int = CRX_OUTPUT_HANDLER_INITBUF_SIZE(handler->size);
			size_t grow_buf = CRX_OUTPUT_HANDLER_INITBUF_SIZE(buf->used - (handler->buffer.size - handler->buffer.used));
			size_t grow_max = MAX(grow_int, grow_buf);

			handler->buffer.data = safe_erealloc(handler->buffer.data, 1, handler->buffer.size, grow_max);
			handler->buffer.size += grow_max;
		}
		memcpy(handler->buffer.data + handler->buffer.used, buf->data, buf->used);
		handler->buffer.used += buf->used;

		/* chunked buffering */
		if (handler->size && (handler->buffer.used >= handler->size)) {
			/* store away errors and/or any intermediate output */
			return OG(running) ? 1 : 0;
		}
	}
	return 1;
}
/* }}} */

/* {{{ static crx_output_handler_status_t crx_output_handler_op(crx_output_handler *handler, crx_output_context *context)
 * Output handler operation dispatcher, applying context op to the crx_output_handler handler */
static inline crx_output_handler_status_t crx_output_handler_op(crx_output_handler *handler, crx_output_context *context)
{
	crx_output_handler_status_t status;
	int original_op = context->op;

#if CRX_OUTPUT_DEBUG
	fprintf(stderr, ">>> op(%d, "
					"handler=%p, "
					"name=%s, "
					"flags=%d, "
					"buffer.data=%s, "
					"buffer.used=%zu, "
					"buffer.size=%zu, "
					"in.data=%s, "
					"in.used=%zu)\n",
			context->op,
			handler,
			handler->name,
			handler->flags,
			handler->buffer.used?handler->buffer.data:"",
			handler->buffer.used,
			handler->buffer.size,
			context->in.used?context->in.data:"",
			context->in.used
	);
#endif

	if (crx_output_lock_error(context->op)) {
		/* fatal error */
		return CRX_OUTPUT_HANDLER_FAILURE;
	}

	/* storable? */
	if (crx_output_handler_append(handler, &context->in) && !context->op) {
		context->op = original_op;
		return CRX_OUTPUT_HANDLER_NO_DATA;
	} else {
		/* need to start? */
		if (!(handler->flags & CRX_OUTPUT_HANDLER_STARTED)) {
			context->op |= CRX_OUTPUT_HANDLER_START;
		}

		OG(running) = handler;
		if (handler->flags & CRX_OUTPUT_HANDLER_USER) {
			zval ob_args[2];
			zval retval;

			/* ob_data */
			ZVAL_STRINGL(&ob_args[0], handler->buffer.data, handler->buffer.used);
			/* ob_mode */
			ZVAL_LONG(&ob_args[1], (crex_long) context->op);

			/* Set FCI info */
			handler->func.user->fci.param_count = 2;
			handler->func.user->fci.params = ob_args;
			handler->func.user->fci.retval = &retval;

#define CRX_OUTPUT_USER_SUCCESS(retval) ((C_TYPE(retval) != IS_UNDEF) && !(C_TYPE(retval) == IS_FALSE))
			if (SUCCESS == crex_call_function(&handler->func.user->fci, &handler->func.user->fcc) && CRX_OUTPUT_USER_SUCCESS(retval)) {
				/* user handler may have returned TRUE */
				status = CRX_OUTPUT_HANDLER_NO_DATA;
				if (C_TYPE(retval) != IS_FALSE && C_TYPE(retval) != IS_TRUE) {
					convert_to_string(&retval);
					if (C_STRLEN(retval)) {
						context->out.data = estrndup(C_STRVAL(retval), C_STRLEN(retval));
						context->out.used = C_STRLEN(retval);
						context->out.free = 1;
						status = CRX_OUTPUT_HANDLER_SUCCESS;
					}
				}
			} else {
				/* call failed, pass internal buffer along */
				status = CRX_OUTPUT_HANDLER_FAILURE;
			}

			/* Free arguments and return value */
			zval_ptr_dtor(&ob_args[0]);
			zval_ptr_dtor(&ob_args[1]);
			zval_ptr_dtor(&retval);

		} else {

			crx_output_context_feed(context, handler->buffer.data, handler->buffer.size, handler->buffer.used, 0);

			if (SUCCESS == handler->func.internal(&handler->opaq, context)) {
				if (context->out.used) {
					status = CRX_OUTPUT_HANDLER_SUCCESS;
				} else {
					status = CRX_OUTPUT_HANDLER_NO_DATA;
				}
			} else {
				status = CRX_OUTPUT_HANDLER_FAILURE;
			}
		}
		handler->flags |= CRX_OUTPUT_HANDLER_STARTED;
		OG(running) = NULL;
	}

	switch (status) {
		case CRX_OUTPUT_HANDLER_FAILURE:
			/* disable this handler */
			handler->flags |= CRX_OUTPUT_HANDLER_DISABLED;
			/* discard any output */
			if (context->out.data && context->out.free) {
				efree(context->out.data);
			}
			/* returns handlers buffer */
			context->out.data = handler->buffer.data;
			context->out.used = handler->buffer.used;
			context->out.free = 1;
			handler->buffer.data = NULL;
			handler->buffer.used = 0;
			handler->buffer.size = 0;
			break;
		case CRX_OUTPUT_HANDLER_NO_DATA:
			/* handler ate all */
			crx_output_context_reset(context);
			CREX_FALLTHROUGH;
		case CRX_OUTPUT_HANDLER_SUCCESS:
			/* no more buffered data */
			handler->buffer.used = 0;
			handler->flags |= CRX_OUTPUT_HANDLER_PROCESSED;
			break;
	}

	context->op = original_op;
	return status;
}
/* }}} */


/* {{{ static void crx_output_op(int op, const char *str, size_t len)
 * Output op dispatcher, passes input and output handlers output through the output handler stack until it gets written to the SAPI */
static inline void crx_output_op(int op, const char *str, size_t len)
{
	crx_output_context context;
	crx_output_handler **active;
	int obh_cnt;

	if (crx_output_lock_error(op)) {
		return;
	}

	crx_output_context_init(&context, op);

	/*
	 * broken up for better performance:
	 *  - apply op to the one active handler; note that OG(active) might be popped off the stack on a flush
	 *  - or apply op to the handler stack
	 */
	if (OG(active) && (obh_cnt = crex_stack_count(&OG(handlers)))) {
		context.in.data = (char *) str;
		context.in.used = len;

		if (obh_cnt > 1) {
			crex_stack_apply_with_argument(&OG(handlers), CREX_STACK_APPLY_TOPDOWN, crx_output_stack_apply_op, &context);
		} else if ((active = crex_stack_top(&OG(handlers))) && (!((*active)->flags & CRX_OUTPUT_HANDLER_DISABLED))) {
			crx_output_handler_op(*active, &context);
		} else {
			crx_output_context_pass(&context);
		}
	} else {
		context.out.data = (char *) str;
		context.out.used = len;
	}

	if (context.out.data && context.out.used) {
		crx_output_header();

		if (!(OG(flags) & CRX_OUTPUT_DISABLED)) {
#if CRX_OUTPUT_DEBUG
			fprintf(stderr, "::: sapi_write('%s', %zu)\n", context.out.data, context.out.used);
#endif
			sapi_module.ub_write(context.out.data, context.out.used);

			if (OG(flags) & CRX_OUTPUT_IMPLICITFLUSH) {
				sapi_flush();
			}

			OG(flags) |= CRX_OUTPUT_SENT;
		}
	}
	crx_output_context_dtor(&context);
}
/* }}} */

/* {{{ static int crx_output_stack_apply_op(void *h, void *c)
 * Operation callback for the stack apply function */
static int crx_output_stack_apply_op(void *h, void *c)
{
	int was_disabled;
	crx_output_handler_status_t status;
	crx_output_handler *handler = *(crx_output_handler **) h;
	crx_output_context *context = (crx_output_context *) c;

	if ((was_disabled = (handler->flags & CRX_OUTPUT_HANDLER_DISABLED))) {
		status = CRX_OUTPUT_HANDLER_FAILURE;
	} else {
		status = crx_output_handler_op(handler, context);
	}

	/*
	 * handler ate all => break
	 * handler returned data or failed resp. is disabled => continue
	 */
	switch (status) {
		case CRX_OUTPUT_HANDLER_NO_DATA:
			return 1;

		case CRX_OUTPUT_HANDLER_SUCCESS:
			/* swap contexts buffers, unless this is the last handler in the stack */
			if (handler->level) {
				crx_output_context_swap(context);
			}
			return 0;

		case CRX_OUTPUT_HANDLER_FAILURE:
		default:
			if (was_disabled) {
				/* pass input along, if it's the last handler in the stack */
				if (!handler->level) {
					crx_output_context_pass(context);
				}
			} else {
				/* swap buffers, unless this is the last handler */
				if (handler->level) {
					crx_output_context_swap(context);
				}
			}
			return 0;
	}
}
/* }}} */

/* {{{ static int crx_output_stack_apply_clean(void *h, void *c)
 * Clean callback for the stack apply function */
static int crx_output_stack_apply_clean(void *h, void *c)
{
	crx_output_handler *handler = *(crx_output_handler **) h;
	crx_output_context *context = (crx_output_context *) c;

	handler->buffer.used = 0;
	crx_output_handler_op(handler, context);
	crx_output_context_reset(context);
	return 0;
}
/* }}} */

/* {{{ static int crx_output_stack_apply_list(void *h, void *z)
 * List callback for the stack apply function */
static int crx_output_stack_apply_list(void *h, void *z)
{
	crx_output_handler *handler = *(crx_output_handler **) h;
	zval *array = (zval *) z;

	add_next_index_str(array, crex_string_copy(handler->name));
	return 0;
}
/* }}} */

/* {{{ static int crx_output_stack_apply_status(void *h, void *z)
 * Status callback for the stack apply function */
static int crx_output_stack_apply_status(void *h, void *z)
{
	crx_output_handler *handler = *(crx_output_handler **) h;
	zval arr, *array = (zval *) z;

	add_next_index_zval(array, crx_output_handler_status(handler, &arr));

	return 0;
}

/* {{{ static zval *crx_output_handler_status(crx_output_handler *handler, zval *entry)
 * Returns an array with the status of the output handler */
static inline zval *crx_output_handler_status(crx_output_handler *handler, zval *entry)
{
	CREX_ASSERT(entry != NULL);

	array_init(entry);
	add_assoc_str(entry, "name", crex_string_copy(handler->name));
	add_assoc_long(entry, "type", (crex_long) (handler->flags & 0xf));
	add_assoc_long(entry, "flags", (crex_long) handler->flags);
	add_assoc_long(entry, "level", (crex_long) handler->level);
	add_assoc_long(entry, "chunk_size", (crex_long) handler->size);
	add_assoc_long(entry, "buffer_size", (crex_long) handler->buffer.size);
	add_assoc_long(entry, "buffer_used", (crex_long) handler->buffer.used);

	return entry;
}
/* }}} */

/* {{{ static int crx_output_stack_pop(int flags)
 * Pops an output handler off the stack */
static int crx_output_stack_pop(int flags)
{
	crx_output_context context;
	crx_output_handler **current, *orphan = OG(active);

	if (!orphan) {
		if (!(flags & CRX_OUTPUT_POP_SILENT)) {
			crx_error_docref("ref.outcontrol", E_NOTICE, "Failed to %s buffer. No buffer to %s", (flags&CRX_OUTPUT_POP_DISCARD)?"discard":"send", (flags&CRX_OUTPUT_POP_DISCARD)?"discard":"send");
		}
		return 0;
	} else if (!(flags & CRX_OUTPUT_POP_FORCE) && !(orphan->flags & CRX_OUTPUT_HANDLER_REMOVABLE)) {
		if (!(flags & CRX_OUTPUT_POP_SILENT)) {
			crx_error_docref("ref.outcontrol", E_NOTICE, "Failed to %s buffer of %s (%d)", (flags&CRX_OUTPUT_POP_DISCARD)?"discard":"send", ZSTR_VAL(orphan->name), orphan->level);
		}
		return 0;
	} else {
		crx_output_context_init(&context, CRX_OUTPUT_HANDLER_FINAL);

		/* don't run the output handler if it's disabled */
		if (!(orphan->flags & CRX_OUTPUT_HANDLER_DISABLED)) {
			/* didn't it start yet? */
			if (!(orphan->flags & CRX_OUTPUT_HANDLER_STARTED)) {
				context.op |= CRX_OUTPUT_HANDLER_START;
			}
			/* signal that we're cleaning up */
			if (flags & CRX_OUTPUT_POP_DISCARD) {
				context.op |= CRX_OUTPUT_HANDLER_CLEAN;
			}
			crx_output_handler_op(orphan, &context);
		}

		/* pop it off the stack */
		crex_stack_del_top(&OG(handlers));
		if ((current = crex_stack_top(&OG(handlers)))) {
			OG(active) = *current;
		} else {
			OG(active) = NULL;
		}

		/* pass output along */
		if (context.out.data && context.out.used && !(flags & CRX_OUTPUT_POP_DISCARD)) {
			crx_output_write(context.out.data, context.out.used);
		}

		/* destroy the handler (after write!) */
		crx_output_handler_free(&orphan);
		crx_output_context_dtor(&context);

		return 1;
	}
}
/* }}} */

/* {{{ static SUCCESS|FAILURE crx_output_handler_compat_func(void *ctx, crx_output_context *)
 * crx_output_handler_context_func_t for crx_output_handler_func_t output handlers */
static int crx_output_handler_compat_func(void **handler_context, crx_output_context *output_context)
{
	crx_output_handler_func_t func = *(crx_output_handler_func_t *) handler_context;

	if (func) {
		char *out_str = NULL;
		size_t out_len = 0;

		func(output_context->in.data, output_context->in.used, &out_str, &out_len, output_context->op);

		if (out_str) {
			output_context->out.data = out_str;
			output_context->out.used = out_len;
			output_context->out.free = 1;
		} else {
			crx_output_context_pass(output_context);
		}

		return SUCCESS;
	}
	return FAILURE;
}
/* }}} */

/* {{{ static SUCCESS|FAILURE crx_output_handler_default_func(void *ctx, crx_output_context *)
 * Default output handler */
static int crx_output_handler_default_func(void **handler_context, crx_output_context *output_context)
{
	crx_output_context_pass(output_context);
	return SUCCESS;
}
/* }}} */

/* {{{ static SUCCESS|FAILURE crx_output_handler_devnull_func(void *ctx, crx_output_context *)
 * Null output handler */
static int crx_output_handler_devnull_func(void **handler_context, crx_output_context *output_context)
{
	return SUCCESS;
}
/* }}} */

/*
 * USERLAND (nearly 1:1 of old output.c)
 */

/* {{{ Turn on Output Buffering (specifying an optional output handler). */
CRX_FUNCTION(ob_start)
{
	zval *output_handler = NULL;
	crex_long chunk_size = 0;
	crex_long flags = CRX_OUTPUT_HANDLER_STDFLAGS;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|zll", &output_handler, &chunk_size, &flags) == FAILURE) {
		RETURN_THROWS();
	}

	if (chunk_size < 0) {
		chunk_size = 0;
	}

	if (crx_output_start_user(output_handler, chunk_size, flags) == FAILURE) {
		crx_error_docref("ref.outcontrol", E_NOTICE, "Failed to create buffer");
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Flush (send) contents of the output buffer. The last buffer content is sent to next buffer */
CRX_FUNCTION(ob_flush)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if (!OG(active)) {
		crx_error_docref("ref.outcontrol", E_NOTICE, "Failed to flush buffer. No buffer to flush");
		RETURN_FALSE;
	}

	if (SUCCESS != crx_output_flush()) {
		crx_error_docref("ref.outcontrol", E_NOTICE, "Failed to flush buffer of %s (%d)", ZSTR_VAL(OG(active)->name), OG(active)->level);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Clean (delete) the current output buffer */
CRX_FUNCTION(ob_clean)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if (!OG(active)) {
		crx_error_docref("ref.outcontrol", E_NOTICE, "Failed to delete buffer. No buffer to delete");
		RETURN_FALSE;
	}

	if (SUCCESS != crx_output_clean()) {
		crx_error_docref("ref.outcontrol", E_NOTICE, "Failed to delete buffer of %s (%d)", ZSTR_VAL(OG(active)->name), OG(active)->level);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Flush (send) the output buffer, and delete current output buffer */
CRX_FUNCTION(ob_end_flush)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if (!OG(active)) {
		crx_error_docref("ref.outcontrol", E_NOTICE, "Failed to delete and flush buffer. No buffer to delete or flush");
		RETURN_FALSE;
	}

	RETURN_BOOL(SUCCESS == crx_output_end());
}
/* }}} */

/* {{{ Clean the output buffer, and delete current output buffer */
CRX_FUNCTION(ob_end_clean)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if (!OG(active)) {
		crx_error_docref("ref.outcontrol", E_NOTICE, "Failed to delete buffer. No buffer to delete");
		RETURN_FALSE;
	}

	RETURN_BOOL(SUCCESS == crx_output_discard());
}
/* }}} */

/* {{{ Get current buffer contents, flush (send) the output buffer, and delete current output buffer */
CRX_FUNCTION(ob_get_flush)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if (crx_output_get_contents(return_value) == FAILURE) {
		crx_error_docref("ref.outcontrol", E_NOTICE, "Failed to delete and flush buffer. No buffer to delete or flush");
		RETURN_FALSE;
	}

	if (SUCCESS != crx_output_end()) {
		crx_error_docref("ref.outcontrol", E_NOTICE, "Failed to delete buffer of %s (%d)", ZSTR_VAL(OG(active)->name), OG(active)->level);
	}
}
/* }}} */

/* {{{ Get current buffer contents and delete current output buffer */
CRX_FUNCTION(ob_get_clean)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if(!OG(active)) {
		RETURN_FALSE;
	}

	if (crx_output_get_contents(return_value) == FAILURE) {
		crx_error_docref("ref.outcontrol", E_NOTICE, "Failed to delete buffer. No buffer to delete");
		RETURN_FALSE;
	}

	if (SUCCESS != crx_output_discard()) {
		crx_error_docref("ref.outcontrol", E_NOTICE, "Failed to delete buffer of %s (%d)", ZSTR_VAL(OG(active)->name), OG(active)->level);
	}
}
/* }}} */

/* {{{ Return the contents of the output buffer */
CRX_FUNCTION(ob_get_contents)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if (crx_output_get_contents(return_value) == FAILURE) {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Return the nesting level of the output buffer */
CRX_FUNCTION(ob_get_level)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	RETURN_LONG(crx_output_get_level());
}
/* }}} */

/* {{{ Return the length of the output buffer */
CRX_FUNCTION(ob_get_length)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if (crx_output_get_length(return_value) == FAILURE) {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ List all output_buffers in an array */
CRX_FUNCTION(ob_list_handlers)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	array_init(return_value);

	if (!OG(active)) {
		return;
	}

	crex_stack_apply_with_argument(&OG(handlers), CREX_STACK_APPLY_BOTTOMUP, crx_output_stack_apply_list, return_value);
}
/* }}} */

/* {{{ Return the status of the active or all output buffers */
CRX_FUNCTION(ob_get_status)
{
	bool full_status = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|b", &full_status) == FAILURE) {
		RETURN_THROWS();
	}

	if (!OG(active)) {
		array_init(return_value);
		return;
	}

	if (full_status) {
		array_init(return_value);
		crex_stack_apply_with_argument(&OG(handlers), CREX_STACK_APPLY_BOTTOMUP, crx_output_stack_apply_status, return_value);
	} else {
		crx_output_handler_status(OG(active), return_value);
	}
}
/* }}} */

/* {{{ Turn implicit flush on/off and is equivalent to calling flush() after every output call */
CRX_FUNCTION(ob_implicit_flush)
{
	crex_long flag = 1;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|b", &flag) == FAILURE) {
		RETURN_THROWS();
	}

	crx_output_set_implicit_flush((int) flag);
}
/* }}} */

/* {{{ Reset(clear) URL rewriter values */
CRX_FUNCTION(output_reset_rewrite_vars)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if (crx_url_scanner_reset_vars() == SUCCESS) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Add URL rewriter values */
CRX_FUNCTION(output_add_rewrite_var)
{
	char *name, *value;
	size_t name_len, value_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss", &name, &name_len, &value, &value_len) == FAILURE) {
		RETURN_THROWS();
	}

	if (crx_url_scanner_add_var(name, name_len, value, value_len, 1) == SUCCESS) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */
