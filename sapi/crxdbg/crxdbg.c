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
   | Authors: Felipe Pena <felipe@crx.net>                                |
   | Authors: Joe Watkins <joe.watkins@live.co.uk>                        |
   | Authors: Bob Weinand <bwoebi@crx.net>                                |
   +----------------------------------------------------------------------+
*/

#include "crxdbg.h"
#include "crxdbg_prompt.h"
#include "crxdbg_bp.h"
#include "crxdbg_break.h"
#include "crxdbg_list.h"
#include "crxdbg_utils.h"
#include "crxdbg_set.h"
#include "crxdbg_io.h"
#include "crex_alloc.h"
#include "crxdbg_print.h"
#include "crxdbg_help.h"
#include "crxdbg_arginfo.h"
#include "crex_vm.h"
#include "crx_ini_builder.h"

#include "ext/standard/basic_functions.h"

#if defined(CRX_WIN32) && defined(HAVE_OPENSSL)
# include "openssl/applink.c"
#endif

#if defined(CRX_WIN32) && defined(ZTS)
CREX_TSRMLS_CACHE_DEFINE()
#endif

CREX_DECLARE_MODULE_GLOBALS(crxdbg)
int crxdbg_startup_run = 0;

static bool crxdbg_booted = 0;
static bool crxdbg_fully_started = 0;
bool use_mm_wrappers = 1;

static void crx_crxdbg_destroy_bp_file(zval *brake) /* {{{ */
{
	crex_hash_destroy(C_ARRVAL_P(brake));
	efree(C_ARRVAL_P(brake));
} /* }}} */

static void crx_crxdbg_destroy_bp_symbol(zval *brake) /* {{{ */
{
	efree((char *) ((crxdbg_breaksymbol_t *) C_PTR_P(brake))->symbol);
	efree(C_PTR_P(brake));
} /* }}} */

static void crx_crxdbg_destroy_bp_opcode(zval *brake) /* {{{ */
{
	efree((char *) ((crxdbg_breakop_t *) C_PTR_P(brake))->name);
	efree(C_PTR_P(brake));
} /* }}} */

static void crx_crxdbg_destroy_bp_opline(zval *brake) /* {{{ */
{
	efree(C_PTR_P(brake));
} /* }}} */

static void crx_crxdbg_destroy_bp_methods(zval *brake) /* {{{ */
{
	crex_hash_destroy(C_ARRVAL_P(brake));
	efree(C_ARRVAL_P(brake));
} /* }}} */

static void crx_crxdbg_destroy_bp_condition(zval *data) /* {{{ */
{
	crxdbg_breakcond_t *brake = (crxdbg_breakcond_t *) C_PTR_P(data);

	if (brake->ops) {
		destroy_op_array(brake->ops);
		efree(brake->ops);
	}
	efree((char*) brake->code);
	efree(brake);
} /* }}} */

static void crx_crxdbg_destroy_registered(zval *data) /* {{{ */
{
	crex_function_dtor(data);
} /* }}} */

static void crx_crxdbg_destroy_file_source(zval *data) /* {{{ */
{
	crxdbg_file_source *source = (crxdbg_file_source *) C_PTR_P(data);
	destroy_op_array(&source->op_array);
	if (source->buf) {
		efree(source->buf);
	}
	efree(source);
} /* }}} */

static inline void crx_crxdbg_globals_ctor(crex_crxdbg_globals *pg) /* {{{ */
{
	pg->prompt[0] = NULL;
	pg->prompt[1] = NULL;

	pg->colors[0] = NULL;
	pg->colors[1] = NULL;
	pg->colors[2] = NULL;

	pg->lines = crxdbg_get_terminal_height();
	pg->exec = NULL;
	pg->exec_len = 0;
	pg->buffer = NULL;
	pg->last_was_newline = 1;
	pg->ops = NULL;
	pg->vmret = 0;
	pg->in_execution = 0;
	pg->bp_count = 0;
	pg->flags = CRXDBG_DEFAULT_FLAGS;
	memset(pg->io, 0, sizeof(pg->io));
	pg->frame.num = 0;
	pg->sapi_name_ptr = NULL;
	pg->unclean_eval = 0;

	pg->req_id = 0;
	pg->err_buf.active = 0;
	pg->err_buf.type = 0;

	pg->input_buflen = 0;
	pg->sigsafe_mem.mem = NULL;
	pg->sigsegv_bailout = NULL;

	pg->oplog_list = NULL;
	pg->stdin_file = NULL;

	pg->cur_command = NULL;
	pg->last_line = 0;

#ifdef HAVE_USERFAULTFD_WRITEFAULT
	pg->watch_userfaultfd = 0;
	pg->watch_userfault_thread = 0;
#endif
} /* }}} */

static CRX_MINIT_FUNCTION(crxdbg) /* {{{ */
{
	crex_hash_init(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE], 8, NULL, crx_crxdbg_destroy_bp_file, 0);
	crex_hash_init(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE_PENDING], 8, NULL, crx_crxdbg_destroy_bp_file, 0);
	crex_hash_init(&CRXDBG_G(bp)[CRXDBG_BREAK_SYM], 8, NULL, crx_crxdbg_destroy_bp_symbol, 0);
	crex_hash_init(&CRXDBG_G(bp)[CRXDBG_BREAK_FUNCTION_OPLINE], 8, NULL, crx_crxdbg_destroy_bp_methods, 0);
	crex_hash_init(&CRXDBG_G(bp)[CRXDBG_BREAK_METHOD_OPLINE], 8, NULL, crx_crxdbg_destroy_bp_methods, 0);
	crex_hash_init(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE_OPLINE], 8, NULL, crx_crxdbg_destroy_bp_methods, 0);
	crex_hash_init(&CRXDBG_G(bp)[CRXDBG_BREAK_OPLINE], 8, NULL, crx_crxdbg_destroy_bp_opline, 0);
	crex_hash_init(&CRXDBG_G(bp)[CRXDBG_BREAK_OPCODE], 8, NULL, crx_crxdbg_destroy_bp_opcode, 0);
	crex_hash_init(&CRXDBG_G(bp)[CRXDBG_BREAK_METHOD], 8, NULL, crx_crxdbg_destroy_bp_methods, 0);
	crex_hash_init(&CRXDBG_G(bp)[CRXDBG_BREAK_COND], 8, NULL, crx_crxdbg_destroy_bp_condition, 0);
	crex_hash_init(&CRXDBG_G(bp)[CRXDBG_BREAK_MAP], 8, NULL, NULL, 0);

	crex_hash_init(&CRXDBG_G(seek), 8, NULL, NULL, 0);
	crex_hash_init(&CRXDBG_G(registered), 8, NULL, crx_crxdbg_destroy_registered, 0);

	crex_hash_init(&CRXDBG_G(file_sources), 0, NULL, crx_crxdbg_destroy_file_source, 0);
	crxdbg_setup_watchpoints();

	crex_execute_ex = crxdbg_execute_ex;

	register_crxdbg_symbols(module_number);

	return SUCCESS;
} /* }}} */

static CRX_MSHUTDOWN_FUNCTION(crxdbg) /* {{{ */
{
	crex_hash_destroy(&CRXDBG_G(registered));
	crxdbg_destroy_watchpoints();

	if (!(CRXDBG_G(flags) & CRXDBG_IS_QUITTING)) {
		crxdbg_notice("Script ended normally");
	}

	/* hack to restore mm_heap->use_custom_heap in order to receive memory leak info */
	if (use_mm_wrappers) {
		/* ASSUMING that mm_heap->use_custom_heap is the first element of the struct ... */
		*(int *) crex_mm_get_heap() = 0;
	}

	if (CRXDBG_G(buffer)) {
		free(CRXDBG_G(buffer));
		CRXDBG_G(buffer) = NULL;
	}

	if (CRXDBG_G(exec)) {
		free(CRXDBG_G(exec));
		CRXDBG_G(exec) = NULL;
	}

	if (CRXDBG_G(oplog_list)) {
		crxdbg_oplog_list *cur = CRXDBG_G(oplog_list);
		do {
			crxdbg_oplog_list *prev = cur->prev;
			efree(cur);
			cur = prev;
		} while (cur != NULL);

		crex_arena_destroy(CRXDBG_G(oplog_arena));
		CRXDBG_G(oplog_list) = NULL;
	}

	fflush(stdout);
	if (SG(request_info).argv0) {
		free(SG(request_info).argv0);
		SG(request_info).argv0 = NULL;
	}

	return SUCCESS;
}
/* }}} */

static CRX_RINIT_FUNCTION(crxdbg) /* {{{ */
{
	/* deactivate symbol table caching to have these properly destroyed upon stack leaving (especially important for watchpoints) */
	EG(symtable_cache_limit) = EG(symtable_cache);

	if (crex_vm_kind() != CREX_VM_KIND_HYBRID) {
		/* crxdbg cannot work JIT-ed code */
		crex_string *key = crex_string_init(CREX_STRL("opcache.jit"), 1);
		crex_string *value = crex_string_init(CREX_STRL("off"), 1);

		crex_alter_ini_entry(key, value, CREX_INI_SYSTEM, CREX_INI_STAGE_STARTUP);

		crex_string_release(key);
		crex_string_release(value);
	}

	return SUCCESS;
} /* }}} */

static CRX_RSHUTDOWN_FUNCTION(crxdbg) /* {{{ */
{
	if (CRXDBG_G(stdin_file)) {
		fclose(CRXDBG_G(stdin_file));
		CRXDBG_G(stdin_file) = NULL;
	}

	return SUCCESS;
} /* }}} */

/* {{{ Attempt to set the execution context for crxdbg
	If the execution context was set previously it is returned
	If the execution context was not set previously boolean true is returned
	If the request to set the context fails, boolean false is returned, and an E_WARNING raised */
CRX_FUNCTION(crxdbg_exec)
{
	crex_string *exec;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "S", &exec) == FAILURE) {
		RETURN_THROWS();
	}

	{
		crex_stat_t sb = {0};
		bool result = 1;

		if (VCWD_STAT(ZSTR_VAL(exec), &sb) != FAILURE) {
			if (sb.st_mode & (S_IFREG|S_IFLNK)) {
				if (CRXDBG_G(exec)) {
					ZVAL_STRINGL(return_value, CRXDBG_G(exec), CRXDBG_G(exec_len));
					free(CRXDBG_G(exec));
					result = 0;
				}

				CRXDBG_G(exec) = crex_strndup(ZSTR_VAL(exec), ZSTR_LEN(exec));
				CRXDBG_G(exec_len) = ZSTR_LEN(exec);

				if (result) {
					ZVAL_TRUE(return_value);
				}
			} else {
				crex_error(E_WARNING, "Failed to set execution context (%s), not a regular file or symlink", ZSTR_VAL(exec));
				ZVAL_FALSE(return_value);
			}
		} else {
			crex_error(E_WARNING, "Failed to set execution context (%s) the file does not exist", ZSTR_VAL(exec));

			ZVAL_FALSE(return_value);
		}
	}
} /* }}} */

/* {{{ instructs crxdbg to insert a breakpoint at the next opcode */
CRX_FUNCTION(crxdbg_break_next)
{
	crex_execute_data *ex;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	ex = EG(current_execute_data);
	while (ex && ex->func && !CREX_USER_CODE(ex->func->type)) {
		ex = ex->prev_execute_data;
	}

	if (!ex) {
		return;
	}

	crxdbg_set_breakpoint_opline_ex((crxdbg_opline_ptr_t) ex->opline + 1);
} /* }}} */

/* {{{ */
CRX_FUNCTION(crxdbg_break_file)
{
	char *file;
	size_t flen;
	crex_long line;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sl", &file, &flen, &line) == FAILURE) {
		RETURN_THROWS();
	}

	crxdbg_set_breakpoint_file(file, 0, line);
} /* }}} */

/* {{{ */
CRX_FUNCTION(crxdbg_break_method)
{
	char *class, *method;
	size_t clen, mlen;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss", &class, &clen, &method, &mlen) == FAILURE) {
		RETURN_THROWS();
	}

	crxdbg_set_breakpoint_method(class, method);
} /* }}} */

/* {{{ */
CRX_FUNCTION(crxdbg_break_function)
{
	char    *function;
	size_t   function_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s", &function, &function_len) == FAILURE) {
		RETURN_THROWS();
	}

	crxdbg_set_breakpoint_symbol(function, function_len);
} /* }}} */

/* {{{ instructs crxdbg to clear breakpoints */
CRX_FUNCTION(crxdbg_clear)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	crex_hash_clean(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE]);
	crex_hash_clean(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE_PENDING]);
	crex_hash_clean(&CRXDBG_G(bp)[CRXDBG_BREAK_SYM]);
	crex_hash_clean(&CRXDBG_G(bp)[CRXDBG_BREAK_FUNCTION_OPLINE]);
	crex_hash_clean(&CRXDBG_G(bp)[CRXDBG_BREAK_METHOD_OPLINE]);
	crex_hash_clean(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE_OPLINE]);
	crex_hash_clean(&CRXDBG_G(bp)[CRXDBG_BREAK_OPLINE]);
	crex_hash_clean(&CRXDBG_G(bp)[CRXDBG_BREAK_METHOD]);
	crex_hash_clean(&CRXDBG_G(bp)[CRXDBG_BREAK_COND]);
} /* }}} */

/* {{{ */
CRX_FUNCTION(crxdbg_color)
{
	crex_long element;
	char *color;
	size_t color_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ls", &element, &color, &color_len) == FAILURE) {
		RETURN_THROWS();
	}

	switch (element) {
		case CRXDBG_COLOR_NOTICE:
		case CRXDBG_COLOR_ERROR:
		case CRXDBG_COLOR_PROMPT:
			crxdbg_set_color_ex(element, color, color_len);
		break;

		default:
			crex_argument_value_error(1, "must be one of CRXDBG_COLOR_PROMPT, CRXDBG_COLOR_NOTICE, or CRXDBG_COLOR_ERROR");
	}
} /* }}} */

/* {{{ */
CRX_FUNCTION(crxdbg_prompt)
{
	char *prompt = NULL;
	size_t prompt_len = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s", &prompt, &prompt_len) == FAILURE) {
		RETURN_THROWS();
	}

	crxdbg_set_prompt(prompt);
} /* }}} */

/* {{{ */
CRX_FUNCTION(crxdbg_start_oplog)
{
	crxdbg_oplog_list *prev;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	prev = CRXDBG_G(oplog_list);

	if (!prev) {
		CRXDBG_G(oplog_arena) = crex_arena_create(64 * 1024);
	}

	CRXDBG_G(oplog_list) = emalloc(sizeof(crxdbg_oplog_list));
	CRXDBG_G(oplog_list)->prev = prev;
	CRXDBG_G(oplog_cur) = &CRXDBG_G(oplog_list)->start;
	CRXDBG_G(oplog_cur)->next = NULL;
}

static crex_always_inline bool crxdbg_is_ignored_opcode(uint8_t opcode) {
	return
	    opcode == CREX_NOP || opcode == CREX_OP_DATA || opcode == CREX_FE_FREE || opcode == CREX_FREE || opcode == CREX_ASSERT_CHECK || opcode == CREX_VERIFY_RETURN_TYPE
	 || opcode == CREX_DECLARE_CONST || opcode == CREX_DECLARE_CLASS || opcode == CREX_DECLARE_FUNCTION
	 || opcode == CREX_DECLARE_CLASS_DELAYED
	 || opcode == CREX_DECLARE_ANON_CLASS || opcode == CREX_FAST_RET || opcode == CREX_TICKS
	 || opcode == CREX_EXT_STMT || opcode == CREX_EXT_FCALL_BEGIN || opcode == CREX_EXT_FCALL_END
	 || opcode == CREX_BIND_GLOBAL || opcode == CREX_BIND_INIT_STATIC_OR_JMP
	;
}

static void crxdbg_oplog_fill_executable(crex_op_array *op_array, HashTable *insert_ht, bool by_opcode) {
	/* ignore RECV_* opcodes */
	crex_op *cur = op_array->opcodes + op_array->num_args + !!(op_array->fn_flags & CREX_ACC_VARIADIC);
	crex_op *end = op_array->opcodes + op_array->last;

	crex_long insert_idx;
	zval zero;
	ZVAL_LONG(&zero, 0);

	/* ignore autogenerated return (well, not too precise with finally branches, but that's okay) */
	if (op_array->last >= 1 && (((end - 1)->opcode == CREX_RETURN || (end - 1)->opcode == CREX_RETURN_BY_REF || (end - 1)->opcode == CREX_GENERATOR_RETURN)
	 && ((op_array->last > 1 && ((end - 2)->opcode == CREX_RETURN || (end - 2)->opcode == CREX_RETURN_BY_REF || (end - 2)->opcode == CREX_GENERATOR_RETURN || (end - 2)->opcode == CREX_THROW))
	  || op_array->function_name == NULL || (end - 1)->extended_value == -1))) {
		end--;
	}

	for (; cur < end; cur++) {
		uint8_t opcode = cur->opcode;
		if (crxdbg_is_ignored_opcode(opcode)) {
			continue;
		}

		if (by_opcode) {
			insert_idx = cur - op_array->opcodes;
		} else {
			insert_idx = cur->lineno;
		}

		if (opcode == CREX_NEW && cur[1].opcode == CREX_DO_FCALL) {
			cur++;
		}

		crex_hash_index_update(insert_ht, insert_idx, &zero);
	}
}

static inline HashTable* crxdbg_add_empty_array(HashTable *ht, crex_string *name) {
	zval *ht_zv = crex_hash_find(ht, name);
	if (!ht_zv) {
		zval zv;
		array_init(&zv);
		ht_zv = crex_hash_add_new(ht, name, &zv);
	}
	return C_ARR_P(ht_zv);
}

/* {{{ */
CRX_FUNCTION(crxdbg_get_executable)
{
	HashTable *options = NULL;
	zval *option_buffer;
	bool by_function = 0;
	bool by_opcode = 0;
	HashTable *insert_ht;

	crex_function *func;
	crex_class_entry *ce;
	crex_string *name;
	HashTable *files = &CRXDBG_G(file_sources);
	HashTable files_tmp;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|H", &options) == FAILURE) {
		RETURN_THROWS();
	}

	if (options && (option_buffer = crex_hash_str_find(options, CREX_STRL("functions")))) {
		by_function = crex_is_true(option_buffer);
	}

	if (options && (option_buffer = crex_hash_str_find(options, CREX_STRL("opcodes")))) {
		if (by_function) {
			by_opcode = crex_is_true(option_buffer);
		}
	}

	if (options && (option_buffer = crex_hash_str_find(options, CREX_STRL("files")))) {
		ZVAL_DEREF(option_buffer);
		if (C_TYPE_P(option_buffer) == IS_ARRAY && crex_hash_num_elements(C_ARR_P(option_buffer)) > 0) {
			zval *filename;

			files = &files_tmp;
			crex_hash_init(files, 0, NULL, NULL, 0);

			CREX_HASH_FOREACH_VAL(C_ARR_P(option_buffer), filename) {
				crex_hash_add_empty_element(files, zval_get_string(filename));
			} CREX_HASH_FOREACH_END();
		} else {
			GC_ADDREF(files);
		}
	} else {
		GC_ADDREF(files);
	}

	array_init(return_value);

	CREX_HASH_MAP_FOREACH_STR_KEY_PTR(EG(function_table), name, func) {
		if (func->type == CREX_USER_FUNCTION) {
			if (crex_hash_exists(files, func->op_array.filename)) {
				insert_ht = crxdbg_add_empty_array(C_ARR_P(return_value), func->op_array.filename);

				if (by_function) {
					insert_ht = crxdbg_add_empty_array(insert_ht, name);
				}

				crxdbg_oplog_fill_executable(&func->op_array, insert_ht, by_opcode);
			}
		}
	} CREX_HASH_FOREACH_END();

	CREX_HASH_MAP_FOREACH_STR_KEY_PTR(EG(class_table), name, ce) {
		if (ce->type == CREX_USER_CLASS) {
			if (crex_hash_exists(files, ce->info.user.filename)) {
				CREX_HASH_MAP_FOREACH_PTR(&ce->function_table, func) {
					if (func->type == CREX_USER_FUNCTION && crex_hash_exists(files, func->op_array.filename)) {
						insert_ht = crxdbg_add_empty_array(C_ARR_P(return_value), func->op_array.filename);

						if (by_function) {
							crex_string *fn_name = strpprintf(ZSTR_LEN(name) + ZSTR_LEN(func->op_array.function_name) + 2, "%.*s::%.*s", (int) ZSTR_LEN(name), ZSTR_VAL(name), (int) ZSTR_LEN(func->op_array.function_name), ZSTR_VAL(func->op_array.function_name));
							insert_ht = crxdbg_add_empty_array(insert_ht, fn_name);
							crex_string_release(fn_name);
						}

						crxdbg_oplog_fill_executable(&func->op_array, insert_ht, by_opcode);
					}
				} CREX_HASH_FOREACH_END();
			}
		}
	} CREX_HASH_FOREACH_END();

	CREX_HASH_MAP_FOREACH_STR_KEY(files, name) {
		crxdbg_file_source *source = crex_hash_find_ptr(&CRXDBG_G(file_sources), name);
		if (source) {
			crxdbg_oplog_fill_executable(
				&source->op_array,
				crxdbg_add_empty_array(C_ARR_P(return_value), source->op_array.filename),
				by_opcode);
		}
	} CREX_HASH_FOREACH_END();

	if (!GC_DELREF(files)) {
		crex_hash_destroy(files);
	}
}

/* {{{ */
CRX_FUNCTION(crxdbg_end_oplog)
{
	crxdbg_oplog_entry *cur;
	crxdbg_oplog_list *prev;

	HashTable *options = NULL;
	zval *option_buffer;
	bool by_function = 0;
	bool by_opcode = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|H", &options) == FAILURE) {
		RETURN_THROWS();
	}

	if (!CRXDBG_G(oplog_list)) {
		crex_error(E_WARNING, "Cannot end an oplog without starting it");
		return;
	}

	cur = CRXDBG_G(oplog_list)->start.next;
	prev = CRXDBG_G(oplog_list)->prev;

	efree(CRXDBG_G(oplog_list));
	CRXDBG_G(oplog_list) = prev;

	if (options && (option_buffer = crex_hash_str_find(options, CREX_STRL("functions")))) {
		by_function = crex_is_true(option_buffer);
	}

	if (options && (option_buffer = crex_hash_str_find(options, CREX_STRL("opcodes")))) {
		if (by_function) {
			by_opcode = crex_is_true(option_buffer);
		}
	}

	array_init(return_value);

	{
		crex_string *last_file = NULL;
		HashTable *file_ht = NULL;
		crex_string *last_function = (void *)~(uintptr_t)0;
		crex_class_entry *last_scope = NULL;

		HashTable *insert_ht = NULL;
		crex_long insert_idx;

		do {
			zval zero;
			ZVAL_LONG(&zero, 0);

			if (cur->filename != last_file) {
				last_file = cur->filename;
				file_ht = insert_ht = crxdbg_add_empty_array(C_ARR_P(return_value), last_file);
			}

			if (by_function) {
				if (cur->function_name == NULL) {
					if (last_function != NULL) {
						insert_ht = file_ht;
					}
					last_function = NULL;
				} else if (cur->function_name != last_function || cur->scope != last_scope) {
					crex_string *fn_name;
					last_function = cur->function_name;
					last_scope = cur->scope;
					if (last_scope == NULL) {
						fn_name = crex_string_copy(last_function);
					} else {
						fn_name = strpprintf(ZSTR_LEN(last_function) + ZSTR_LEN(last_scope->name) + 2, "%.*s::%.*s", (int) ZSTR_LEN(last_scope->name), ZSTR_VAL(last_scope->name), (int) ZSTR_LEN(last_function), ZSTR_VAL(last_function));
					}
					insert_ht = crxdbg_add_empty_array(C_ARR_P(return_value), fn_name);
					crex_string_release(fn_name);
				}
			}

			if (by_opcode) {
				insert_idx = cur->op - cur->opcodes;
			} else {
				if (crxdbg_is_ignored_opcode(cur->op->opcode)) {
					continue;
				}

				insert_idx = cur->op->lineno;
			}

			CREX_ASSERT(insert_ht && file_ht);
			{
				zval *num = crex_hash_index_find(insert_ht, insert_idx);
				if (!num) {
					num = crex_hash_index_add_new(insert_ht, insert_idx, &zero);
				}
				C_LVAL_P(num)++;
			}

		} while ((cur = cur->next));
	}

	if (!prev) {
		crex_arena_destroy(CRXDBG_G(oplog_arena));
	}
}

static crex_module_entry sapi_crxdbg_module_entry = {
	STANDARD_MODULE_HEADER,
	CRXDBG_NAME,
	ext_functions,
	CRX_MINIT(crxdbg),
	CRX_MSHUTDOWN(crxdbg),
	CRX_RINIT(crxdbg),
	CRX_RSHUTDOWN(crxdbg),
	NULL,
	CRXDBG_VERSION,
	STANDARD_MODULE_PROPERTIES
};

static inline int crx_sapi_crxdbg_module_startup(sapi_module_struct *module) /* {{{ */
{
	if (crx_module_startup(module, &sapi_crxdbg_module_entry) == FAILURE) {
		return FAILURE;
	}

	crxdbg_booted = 1;

	return SUCCESS;
} /* }}} */

static char* crx_sapi_crxdbg_read_cookies(void) /* {{{ */
{
	return NULL;
} /* }}} */

static int crx_sapi_crxdbg_header_handler(sapi_header_struct *h, sapi_header_op_enum op, sapi_headers_struct *s) /* {{{ */
{
	return 0;
}
/* }}} */

static int crx_sapi_crxdbg_send_headers(sapi_headers_struct *sapi_headers) /* {{{ */
{
	/* We do nothing here, this function is needed to prevent that the fallback
	 * header handling is called. */
	return SAPI_HEADER_SENT_SUCCESSFULLY;
}
/* }}} */

static void crx_sapi_crxdbg_send_header(sapi_header_struct *sapi_header, void *server_context) /* {{{ */
{
}
/* }}} */

static void crx_sapi_crxdbg_log_message(const char *message, int syslog_type_int) /* {{{ */
{
	/*
	* We must not request TSRM before being booted
	*/
	if (crxdbg_booted) {
		if (CRXDBG_G(flags) & CRXDBG_IN_EVAL) {
			crxdbg_error("%s", message);
			return;
		}

		crxdbg_error("%s", message);

		if (CRXDBG_G(flags) & CRXDBG_PREVENT_INTERACTIVE) {
			return;
		}

		if (PG(last_error_type) & E_FATAL_ERRORS) {
			const char *file_char = crex_get_executed_filename();
			crex_string *file = crex_string_init(file_char, strlen(file_char), 0);
			crxdbg_list_file(file, 3, crex_get_executed_lineno() - 1, crex_get_executed_lineno());
			crex_string_release(file);

			if (!crxdbg_fully_started) {
				return;
			}

			do {
				switch (crxdbg_interactive(1, NULL)) {
					case CRXDBG_LEAVE:
					case CRXDBG_FINISH:
					case CRXDBG_UNTIL:
					case CRXDBG_NEXT:
						return;
				}
			} while (!(CRXDBG_G(flags) & CRXDBG_IS_STOPPING));
		}
	} else {
		fprintf(stdout, "%s\n", message);
	}
}
/* }}} */

static int crx_sapi_crxdbg_activate(void) /* {{{ */
{
	return SUCCESS;
}

static int crx_sapi_crxdbg_deactivate(void) /* {{{ */
{
	/* Everything using ZMM should be freed here... */
	crex_hash_destroy(&CRXDBG_G(file_sources));
	crex_hash_destroy(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE]);
	crex_hash_destroy(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE_PENDING]);
	crex_hash_destroy(&CRXDBG_G(bp)[CRXDBG_BREAK_SYM]);
	crex_hash_destroy(&CRXDBG_G(bp)[CRXDBG_BREAK_FUNCTION_OPLINE]);
	crex_hash_destroy(&CRXDBG_G(bp)[CRXDBG_BREAK_METHOD_OPLINE]);
	crex_hash_destroy(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE_OPLINE]);
	crex_hash_destroy(&CRXDBG_G(bp)[CRXDBG_BREAK_OPLINE]);
	crex_hash_destroy(&CRXDBG_G(bp)[CRXDBG_BREAK_OPCODE]);
	crex_hash_destroy(&CRXDBG_G(bp)[CRXDBG_BREAK_METHOD]);
	crex_hash_destroy(&CRXDBG_G(bp)[CRXDBG_BREAK_COND]);
	crex_hash_destroy(&CRXDBG_G(bp)[CRXDBG_BREAK_MAP]);
	crex_hash_destroy(&CRXDBG_G(seek));

	if (CRXDBG_G(ops)) {
		destroy_op_array(CRXDBG_G(ops));
		efree(CRXDBG_G(ops));
		CRXDBG_G(ops) = NULL;
	}

	return SUCCESS;
}

static void crx_sapi_crxdbg_register_vars(zval *track_vars_array) /* {{{ */
{
	size_t len;
	char  *docroot = "";

	/* In crxdbg mode, we consider the environment to be a part of the server variables
	*/
	crx_import_environment_variables(track_vars_array);

	if (CRXDBG_G(exec)) {
		len = CRXDBG_G(exec_len);
		if (sapi_module.input_filter(PARSE_SERVER, "CRX_SELF", &CRXDBG_G(exec), CRXDBG_G(exec_len), &len)) {
			crx_register_variable("CRX_SELF", CRXDBG_G(exec), track_vars_array);
		}
		if (sapi_module.input_filter(PARSE_SERVER, "SCRIPT_NAME", &CRXDBG_G(exec), CRXDBG_G(exec_len), &len)) {
			crx_register_variable("SCRIPT_NAME", CRXDBG_G(exec), track_vars_array);
		}

		if (sapi_module.input_filter(PARSE_SERVER, "SCRIPT_FILENAME", &CRXDBG_G(exec), CRXDBG_G(exec_len), &len)) {
			crx_register_variable("SCRIPT_FILENAME", CRXDBG_G(exec), track_vars_array);
		}
		if (sapi_module.input_filter(PARSE_SERVER, "PATH_TRANSLATED", &CRXDBG_G(exec), CRXDBG_G(exec_len), &len)) {
			crx_register_variable("PATH_TRANSLATED", CRXDBG_G(exec), track_vars_array);
		}
	}

	/* any old docroot will do */
	len = 0;
	if (sapi_module.input_filter(PARSE_SERVER, "DOCUMENT_ROOT", &docroot, len, &len)) {
		crx_register_variable("DOCUMENT_ROOT", docroot, track_vars_array);
	}
}
/* }}} */

static inline size_t crx_sapi_crxdbg_ub_write(const char *message, size_t length) /* {{{ */
{
	return crxdbg_script(P_STDOUT, "%.*s", (int) length, message);
} /* }}} */

/* beginning of struct, see main/streams/plain_wrapper.c line 111 */
typedef struct {
	FILE *file;
	int fd;
} crx_stdio_stream_data;

static ssize_t crxdbg_stdiop_write(crx_stream *stream, const char *buf, size_t count) {
	crx_stdio_stream_data *data = (crx_stdio_stream_data*)stream->abstract;

	while (data->fd >= 0) {
		struct stat stat[3];
		memset(stat, 0, sizeof(stat));
		int stat_stderr = fstat(fileno(stderr), &stat[2]);
		int stat_stdout = fstat(fileno(stdout), &stat[0]);
		int stat_datafd = fstat(data->fd, &stat[1]);
		if ((stat_stderr < 0 && stat_stdout < 0) || stat_datafd < 0) {
			break;
		}

		if (stat[0].st_dev == stat[1].st_dev && stat[0].st_ino == stat[1].st_ino) {
			crxdbg_script(P_STDOUT, "%.*s", (int) count, buf);
			return count;
		}
		if (stat[2].st_dev == stat[1].st_dev && stat[2].st_ino == stat[1].st_ino) {
			crxdbg_script(P_STDERR, "%.*s", (int) count, buf);
			return count;
		}
		break;
	}

	return CRXDBG_G(crx_stdiop_write)(stream, buf, count);
}

/* copied from sapi/cli/crx_cli.c cli_register_file_handles */
void crxdbg_register_file_handles(void) /* {{{ */
{
	zval zin, zout, zerr;
	crx_stream *s_in, *s_out, *s_err;
	crx_stream_context *sc_in=NULL, *sc_out=NULL, *sc_err=NULL;
	crex_constant ic, oc, ec;

	s_in  = crx_stream_open_wrapper_ex("crx://stdin",  "rb", 0, NULL, sc_in);
	s_out = crx_stream_open_wrapper_ex("crx://stdout", "wb", 0, NULL, sc_out);
	s_err = crx_stream_open_wrapper_ex("crx://stderr", "wb", 0, NULL, sc_err);

	if (s_in==NULL || s_out==NULL || s_err==NULL) {
		if (s_in) crx_stream_close(s_in);
		if (s_out) crx_stream_close(s_out);
		if (s_err) crx_stream_close(s_err);
		return;
	}

#if CRX_DEBUG
	/* do not close stdout and stderr */
	s_out->flags |= CRX_STREAM_FLAG_NO_CLOSE;
	s_err->flags |= CRX_STREAM_FLAG_NO_CLOSE;
#endif

	crx_stream_to_zval(s_in,  &zin);
	crx_stream_to_zval(s_out, &zout);
	crx_stream_to_zval(s_err, &zerr);

	ic.value = zin;
	C_CONSTANT_FLAGS(ic.value) = 0;
	ic.name = crex_string_init(CREX_STRL("STDIN"), 0);
	crex_hash_del(EG(crex_constants), ic.name);
	crex_register_constant(&ic);

	oc.value = zout;
	C_CONSTANT_FLAGS(oc.value) = 0;
	oc.name = crex_string_init(CREX_STRL("STDOUT"), 0);
	crex_hash_del(EG(crex_constants), oc.name);
	crex_register_constant(&oc);

	ec.value = zerr;
	C_CONSTANT_FLAGS(ec.value) = 0;
	ec.name = crex_string_init(CREX_STRL("STDERR"), 0);
	crex_hash_del(EG(crex_constants), ec.name);
	crex_register_constant(&ec);
}
/* }}} */

/* {{{ sapi_module_struct crxdbg_sapi_module */
static sapi_module_struct crxdbg_sapi_module = {
	"crxdbg",                       /* name */
	"crxdbg",                       /* pretty name */

	crx_sapi_crxdbg_module_startup, /* startup */
	crx_module_shutdown_wrapper,    /* shutdown */

	crx_sapi_crxdbg_activate,       /* activate */
	crx_sapi_crxdbg_deactivate,     /* deactivate */

	crx_sapi_crxdbg_ub_write,       /* unbuffered write */
	NULL,                           /* flush */
	NULL,                           /* get uid */
	NULL,                           /* getenv */

	crx_error,                      /* error handler */

	crx_sapi_crxdbg_header_handler, /* header handler */
	crx_sapi_crxdbg_send_headers,   /* send headers handler */
	crx_sapi_crxdbg_send_header,    /* send header handler */

	NULL,                           /* read POST data */
	crx_sapi_crxdbg_read_cookies,   /* read Cookies */

	crx_sapi_crxdbg_register_vars,  /* register server variables */
	crx_sapi_crxdbg_log_message,    /* Log message */
	NULL,                           /* Get request time */
	NULL,                           /* Child terminate */
	STANDARD_SAPI_MODULE_PROPERTIES
};
/* }}} */

static const opt_struct OPTIONS[] = { /* {{{ */
	{'c', 1, "ini path override"},
	{'d', 1, "define ini entry on command line"},
	{'n', 0, "no crx.ini"},
	{'z', 1, "load crex_extension"},
	/* crxdbg options */
	{'q', 0, "no banner"},
	{'v', 0, "disable quietness"},
	{'b', 0, "boring colours"},
	{'i', 1, "specify init"},
	{'I', 0, "ignore init"},
	{'O', 1, "opline log"},
	{'r', 0, "run"},
	{'e', 0, "generate ext_stmt opcodes"},
	{'E', 0, "step-through-eval"},
	{'s', 1, "script from stdin"},
	{'S', 1, "sapi-name"},
	{'p', 2, "show opcodes"},
	{'h', 0, "help"},
	{'V', 0, "version"},
	{'-', 0, NULL}
}; /* }}} */

const char crxdbg_ini_hardcoded[] =
"html_errors=Off\n"
"register_argc_argv=On\n"
"implicit_flush=On\n"
"display_errors=Off\n"
"log_errors=On\n"
"max_execution_time=0\n"
"max_input_time=-1\n"
"error_log=\n"
"output_buffering=off\n";

static void crxdbg_welcome(bool cleaning) /* {{{ */
{
	/* print blurb */
	if (!cleaning) {
		crxdbg_notice("Welcome to crxdbg, the interactive CRX debugger, v%s", CRXDBG_VERSION);
		crxdbg_writeln("To get help using crxdbg type \"help\" and press enter");
		crxdbg_notice("Please report bugs to <%s>", CRXDBG_ISSUES);
	} else if (crxdbg_startup_run == 0) {
		crxdbg_write(
			"Classes              %d\n"
			"Functions            %d\n"
			"Constants            %d\n"
			"Includes             %d\n",
			crex_hash_num_elements(EG(class_table)),
			crex_hash_num_elements(EG(function_table)),
			crex_hash_num_elements(EG(crex_constants)),
			crex_hash_num_elements(&EG(included_files)));
	}
} /* }}} */

static inline void crxdbg_sigint_handler(int signo) /* {{{ */
{
	if (!(CRXDBG_G(flags) & CRXDBG_IS_INTERACTIVE)) {
		/* set signalled only when not interactive */
		if (CRXDBG_G(flags) & CRXDBG_IS_SIGNALED) {
			char mem[CRXDBG_SIGSAFE_MEM_SIZE + 1];

			crxdbg_set_sigsafe_mem(mem);
			crex_try {
				crxdbg_force_interruption();
			} crex_end_try()
			crxdbg_clear_sigsafe_mem();

			CRXDBG_G(flags) &= ~CRXDBG_IS_SIGNALED;

			if (CRXDBG_G(flags) & CRXDBG_IS_STOPPING) {
				crex_bailout();
			}
		} else {
			CRXDBG_G(flags) |= CRXDBG_IS_SIGNALED;
			if (CRXDBG_G(flags) & CRXDBG_PREVENT_INTERACTIVE) {
				CRXDBG_G(flags) |= CRXDBG_HAS_PAGINATION;
				CRXDBG_G(flags) &= ~CRXDBG_PREVENT_INTERACTIVE;
			}
		}
	}
} /* }}} */

#ifndef _WIN32
void crxdbg_signal_handler(int sig, siginfo_t *info, void *context) /* {{{ */
{
	int is_handled = FAILURE;

	switch (sig) {
		case SIGBUS:
		case SIGSEGV:
			is_handled = crxdbg_watchpoint_segfault_handler(info, context);
			if (is_handled == FAILURE) {
				if (CRXDBG_G(sigsegv_bailout)) {
					LONGJMP(*CRXDBG_G(sigsegv_bailout), FAILURE);
				}
				crex_sigaction(sig, &CRXDBG_G(old_sigsegv_signal), NULL);
			}
			break;
	}

} /* }}} */


void crxdbg_sighup_handler(int sig) /* {{{ */
{
	exit(0);
} /* }}} */
#endif

CREX_ATTRIBUTE_MALLOC CREX_ATTRIBUTE_ALLOC_SIZE(1) void *crxdbg_malloc_wrapper(size_t size CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC) /* {{{ */
{
	return _crex_mm_alloc(crex_mm_get_heap(), size CREX_FILE_LINE_RELAY_CC CREX_FILE_LINE_ORIG_RELAY_CC);
} /* }}} */

void crxdbg_free_wrapper(void *p CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC) /* {{{ */
{
	crex_mm_heap *heap = crex_mm_get_heap();
	if (UNEXPECTED(heap == p)) {
		/* TODO: heap maybe allocated by mmap(crex_mm_init) or malloc(USE_CREX_ALLOC=0)
		 * let's prevent it from segfault for now
		 */
	} else {
		crxdbg_watch_efree(p);
		_crex_mm_free(heap, p CREX_FILE_LINE_RELAY_CC CREX_FILE_LINE_ORIG_RELAY_CC);
	}
} /* }}} */

void *crxdbg_realloc_wrapper(void *ptr, size_t size CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC) /* {{{ */
{
	return _crex_mm_realloc(crex_mm_get_heap(), ptr, size CREX_FILE_LINE_RELAY_CC CREX_FILE_LINE_ORIG_RELAY_CC);
} /* }}} */

crx_stream *crxdbg_stream_url_wrap_crx(crx_stream_wrapper *wrapper, const char *path, const char *mode, int options, crex_string **opened_path, crx_stream_context *context STREAMS_DC) /* {{{ */
{
	if (!strncasecmp(path, "crx://", 6)) {
		path += 6;
	}

	if (!strncasecmp(path, "stdin", 6) && CRXDBG_G(stdin_file)) {
		crx_stream *stream = crx_stream_fopen_from_fd(dup(fileno(CRXDBG_G(stdin_file))), "r", NULL);
#ifdef CRX_WIN32
		if (context != NULL) {
			zval *blocking_pipes = crx_stream_context_get_option(context, "pipe", "blocking");
			if (blocking_pipes) {
				convert_to_long(blocking_pipes);
				crx_stream_set_option(stream, CRX_STREAM_OPTION_PIPE_BLOCKING, C_LVAL_P(blocking_pipes), NULL);
			}
		}
#endif
		return stream;
	}

	return CRXDBG_G(orig_url_wrap_crx)->wops->stream_opener(wrapper, path, mode, options, opened_path, context STREAMS_CC);
} /* }}} */

int main(int argc, char **argv) /* {{{ */
{
	sapi_module_struct *crxdbg = &crxdbg_sapi_module;
	char *sapi_name;
	struct crx_ini_builder ini_builder;
	char **crex_extensions = NULL;
	crex_ulong crex_extensions_len = 0L;
	bool ini_ignore;
	char *ini_override;
	char *exec = NULL;
	char *first_command = NULL;
	char *init_file;
	size_t init_file_len;
	bool init_file_default;
	uint64_t flags;
	char *crx_optarg;
	int crx_optind, opt, show_banner = 1;
	long cleaning = -1;
	volatile bool quit_immediately = 0; /* somehow some gcc release builds will play a bit around with order in combination with setjmp..., hence volatile */
	crex_crxdbg_globals *settings = NULL;
	char *bp_tmp = NULL;
	char *print_opline_func;
	bool ext_stmt = 0;
	bool is_exit;
	int exit_status;
	char *read_from_stdin = NULL;
	crex_string *backup_crxdbg_compile = NULL;
	bool show_help = 0, show_version = 0;
	void* (*_malloc)(size_t);
	void (*_free)(void*);
	void* (*_realloc)(void*, size_t);
	crx_stream_wrapper wrapper;
	crx_stream_wrapper_ops wops;

#ifdef CRX_WIN32
	_fmode = _O_BINARY;                 /* sets default for file streams to binary */
	setmode(_fileno(stdin), O_BINARY);  /* make the stdio mode be binary */
	setmode(_fileno(stdout), O_BINARY); /* make the stdio mode be binary */
	setmode(_fileno(stderr), O_BINARY); /* make the stdio mode be binary */
#else
	struct sigaction signal_struct;
	signal_struct.sa_sigaction = crxdbg_signal_handler;
	signal_struct.sa_flags = SA_SIGINFO | SA_NODEFER;
#endif

crxdbg_main:
#ifdef ZTS
	crx_tsrm_startup();
# ifdef CRX_WIN32
	CREX_TSRMLS_CACHE_UPDATE();
# endif
#endif

	crex_signal_startup();

	crx_ini_builder_init(&ini_builder);
	ini_ignore = 0;
	ini_override = NULL;
	crex_extensions = NULL;
	crex_extensions_len = 0L;
	init_file = NULL;
	init_file_len = 0;
	init_file_default = 1;
	flags = CRXDBG_DEFAULT_FLAGS;
	is_exit = 0;
	crx_optarg = NULL;
	crx_optind = 1;
	opt = 0;
	sapi_name = NULL;
	exit_status = 0;
	if (settings) {
		exec = settings->exec;
	}

	while ((opt = crx_getopt(argc, argv, OPTIONS, &crx_optarg, &crx_optind, 0, 2)) != -1) {
		switch (opt) {
			case 'r':
				if (settings == NULL) {
					crxdbg_startup_run++;
				}
				break;
			case 'n':
				ini_ignore = 1;
				break;
			case 'c':
				if (ini_override) {
					free(ini_override);
				}
				ini_override = strdup(crx_optarg);
				break;
			case 'd':
				/* define ini entries on command line */
				crx_ini_builder_define(&ini_builder, crx_optarg);
				break;

			case 'z':
				crex_extensions_len++;
				if (crex_extensions) {
					crex_extensions = realloc(crex_extensions, sizeof(char*) * crex_extensions_len);
				} else crex_extensions = malloc(sizeof(char*) * crex_extensions_len);
				crex_extensions[crex_extensions_len-1] = strdup(crx_optarg);
			break;

			/* begin crxdbg options */

			case 's': { /* read script from stdin */
				if (settings == NULL) {
					read_from_stdin = strdup(crx_optarg);
				}
			} break;

			case 'S': { /* set SAPI name */
				sapi_name = strdup(crx_optarg);
			} break;

			case 'I': { /* ignore .crxdbginit */
				init_file_default = 0;
			} break;

			case 'i': { /* set init file */
				if (init_file) {
					free(init_file);
					init_file = NULL;
				}

				init_file_len = strlen(crx_optarg);
				if (init_file_len) {
					init_file = strdup(crx_optarg);
				}
			} break;

			case 'v': /* set quietness off */
				flags &= ~CRXDBG_IS_QUIET;
			break;

			case 'e':
				ext_stmt = 1;
			break;

			case 'E': /* stepping through eval on */
				flags |= CRXDBG_IS_STEPONEVAL;
			break;

			case 'b': /* set colours off */
				flags &= ~CRXDBG_IS_COLOURED;
			break;

			case 'q': /* hide banner */
				show_banner = 0;
			break;

			case 'p': {
				print_opline_func = crx_optarg;
				show_banner = 0;
				settings = (void *) 0x1;
			} break;

			case 'h': {
				show_help = 1;
			} break;

			case 'V': {
				show_version = 1;
			} break;
		}

		crx_optarg = NULL;
	}

	quit_immediately = crxdbg_startup_run > 1;

	/* set exec if present on command line */
	if (!read_from_stdin && argc > crx_optind) {
		if (!exec && strlen(argv[crx_optind])) {
			exec = strdup(argv[crx_optind]);
		}
		crx_optind++;
	}

	if (sapi_name) {
		crxdbg->name = sapi_name;
	}

	crxdbg->ini_defaults = NULL;
	crxdbg->crxinfo_as_text = 1;
	crxdbg->crx_ini_ignore_cwd = 1;

	sapi_startup(crxdbg);

	crxdbg->executable_location = argv[0];
	crxdbg->crxinfo_as_text = 1;
	crxdbg->crx_ini_ignore = ini_ignore;
	crxdbg->crx_ini_path_override = ini_override;

	crx_ini_builder_prepend_literal(&ini_builder, crxdbg_ini_hardcoded);

	if (crex_extensions_len) {
		crex_ulong crex_extension = 0L;

		while (crex_extension < crex_extensions_len) {
			const char *ze = crex_extensions[crex_extension];
			size_t ze_len = strlen(ze);

			crx_ini_builder_unquoted(&ini_builder, "crex_extension", strlen("crex_extension"), ze, ze_len);

			free(crex_extensions[crex_extension]);
			crex_extension++;
		}

		free(crex_extensions);
	}

	crxdbg->ini_entries = crx_ini_builder_finish(&ini_builder);

	CREX_INIT_MODULE_GLOBALS(crxdbg, crx_crxdbg_globals_ctor, NULL);

	/* set default colors */
	crxdbg_set_color_ex(CRXDBG_COLOR_PROMPT,  CRXDBG_STRL("white-bold"));
	crxdbg_set_color_ex(CRXDBG_COLOR_ERROR,   CRXDBG_STRL("red-bold"));
	crxdbg_set_color_ex(CRXDBG_COLOR_NOTICE,  CRXDBG_STRL("green"));

	if (settings > (crex_crxdbg_globals *) 0x2) {
#ifdef ZTS
		crex_crxdbg_globals *ptr = TSRMG_BULK_STATIC(crxdbg_globals_id, crex_crxdbg_globals *);
		*ptr = *settings;
#else
		crxdbg_globals = *settings;
#endif
		free(settings);
	} else {
		/* set default prompt */
		crxdbg_set_prompt(CRXDBG_DEFAULT_PROMPT);
	}

	/* set flags from command line */
	CRXDBG_G(flags) = flags;

	if (crxdbg->startup(crxdbg) == SUCCESS) {
		crex_mm_heap *mm_heap;
#ifdef _WIN32
	EXCEPTION_POINTERS *xp;
	__try {
#endif

		if (show_version || show_help) {
			/* It ain't gonna proceed to real execution anyway,
				but the correct descriptor is needed already. */
			CRXDBG_G(io)[CRXDBG_STDOUT].fd = fileno(stdout);
			if (show_help) {
				crxdbg_do_help_cmd(exec);
			} else if (show_version) {
				crxdbg_out(
					"crxdbg %s (built: %s %s)\nCRX %s, Copyright (c) The CRX Group\n%s",
					CRXDBG_VERSION,
					__DATE__,
					__TIME__,
					CRX_VERSION,
					get_crex_version()
				);
			}
			CRXDBG_G(flags) |= CRXDBG_IS_QUITTING;
			crx_module_shutdown();
			sapi_deactivate();
			sapi_shutdown();
			crx_ini_builder_deinit(&ini_builder);
			if (ini_override) {
				free(ini_override);
			}
			if (exec) {
				free(exec);
			}
			if (init_file) {
				free(init_file);
			}
			goto free_and_return;
		}

		crex_try {
			crex_signal_activate();
		} crex_end_try();

#ifndef _WIN32
		crex_signal(SIGHUP, crxdbg_sighup_handler);
#endif

		mm_heap = crex_mm_get_heap();
		crex_mm_get_custom_handlers(mm_heap, &_malloc, &_free, &_realloc);

		use_mm_wrappers = !_malloc && !_realloc && !_free;

		CRXDBG_G(original_free_function) = _free;
		_free = crxdbg_watch_efree;

		if (use_mm_wrappers) {
#if CREX_DEBUG
			crex_mm_set_custom_debug_handlers(mm_heap, crxdbg_malloc_wrapper, crxdbg_free_wrapper, crxdbg_realloc_wrapper);
#else
			crex_mm_set_custom_handlers(mm_heap, crxdbg_malloc_wrapper, crxdbg_free_wrapper, crxdbg_realloc_wrapper);
#endif
		} else {
			crex_mm_set_custom_handlers(mm_heap, _malloc, _free, _realloc);
		}

		_free = CRXDBG_G(original_free_function);


		crxdbg_init_list();

		CRXDBG_G(sapi_name_ptr) = sapi_name;

		if (exec) { /* set execution context */
			CRXDBG_G(exec) = crxdbg_resolve_path(exec);
			CRXDBG_G(exec_len) = CRXDBG_G(exec) ? strlen(CRXDBG_G(exec)) : 0;

			free(exec);
			exec = NULL;
		}

		crx_output_activate();
		crx_output_deactivate();

		if (SG(sapi_headers).mimetype) {
			efree(SG(sapi_headers).mimetype);
			SG(sapi_headers).mimetype = NULL;
		}

		crx_output_activate();

		{
			int i;

			SG(request_info).argc = argc - crx_optind + 1;
			SG(request_info).argv = emalloc(SG(request_info).argc * sizeof(char *));
			for (i = SG(request_info).argc; --i;) {
				SG(request_info).argv[i] = estrdup(argv[crx_optind - 1 + i]);
			}
			SG(request_info).argv[0] = CRXDBG_G(exec) ? estrdup(CRXDBG_G(exec)) : estrdup("");
		}

		if (crx_request_startup() == FAILURE) {
			PUTS("Could not startup");
			return 1;
		}

#ifndef _WIN32
#ifdef HAVE_USERFAULTFD_WRITEFAULT
		if (!CRXDBG_G(watch_userfaultfd))
#endif
		{
			crex_try { crex_sigaction(SIGSEGV, &signal_struct, &CRXDBG_G(old_sigsegv_signal)); } crex_end_try();
			crex_try { crex_sigaction(SIGBUS, &signal_struct, &CRXDBG_G(old_sigsegv_signal)); } crex_end_try();
		}
#endif
		crex_try { crex_signal(SIGINT, crxdbg_sigint_handler); } crex_end_try();


		CRXDBG_G(io)[CRXDBG_STDIN].fd = fileno(stdin);
		CRXDBG_G(io)[CRXDBG_STDOUT].fd = fileno(stdout);
		CRXDBG_G(io)[CRXDBG_STDERR].fd = fileno(stderr);

#ifndef _WIN32
		CRXDBG_G(crx_stdiop_write) = crx_stream_stdio_ops.write;
		crx_stream_stdio_ops.write = crxdbg_stdiop_write;
#endif

		{
			zval *zv = crex_hash_str_find(crx_stream_get_url_stream_wrappers_hash(), CREX_STRL("crx"));
			crx_stream_wrapper *tmp_wrapper = C_PTR_P(zv);
			CRXDBG_G(orig_url_wrap_crx) = tmp_wrapper;
			memcpy(&wrapper, tmp_wrapper, sizeof(wrapper));
			memcpy(&wops, tmp_wrapper->wops, sizeof(wops));
			wops.stream_opener = crxdbg_stream_url_wrap_crx;
			wrapper.wops = (const crx_stream_wrapper_ops*)&wops;
			C_PTR_P(zv) = &wrapper;
		}

		/* Make stdin, stdout and stderr accessible from CRX scripts */
		crxdbg_register_file_handles();

		crxdbg_list_update();

		if (show_banner && cleaning < 2) {
			/* print blurb */
			crxdbg_welcome(cleaning == 1);
		}

		cleaning = -1;

		if (ext_stmt) {
			CG(compiler_options) |= CREX_COMPILE_EXTENDED_INFO;
		}

		/* initialize from file */
		CRXDBG_G(flags) |= CRXDBG_IS_INITIALIZING;
		crex_try {
			crxdbg_init(init_file, init_file_len, init_file_default);
		} crex_end_try();
		CRXDBG_G(flags) &= ~CRXDBG_IS_INITIALIZING;

		/* quit if init says so */
		if (CRXDBG_G(flags) & CRXDBG_IS_QUITTING) {
			goto crxdbg_out;
		}

		/* auto compile */
		if (read_from_stdin) {
			if (!read_from_stdin[0]) {
				if (!quit_immediately) {
					crxdbg_error("Impossible to not specify a stdin delimiter without -rr");
					CRXDBG_G(flags) |= CRXDBG_IS_QUITTING;
					goto crxdbg_out;
				}
			}
			if (show_banner || read_from_stdin[0]) {
				crxdbg_notice("Reading input from stdin; put '%s' followed by a newline on an own line after code to end input", read_from_stdin);
			}

			if (crxdbg_startup_run > 0) {
				CRXDBG_G(flags) |= CRXDBG_DISCARD_OUTPUT;
			}

			crex_try {
				crxdbg_param_t cmd;
				cmd.str = read_from_stdin;
				cmd.len = strlen(read_from_stdin);
				CRXDBG_COMMAND_HANDLER(stdin)(&cmd);
			} crex_end_try();

			CRXDBG_G(flags) &= ~CRXDBG_DISCARD_OUTPUT;
		} else if (CRXDBG_G(exec)) {
			if (settings || crxdbg_startup_run > 0) {
				CRXDBG_G(flags) |= CRXDBG_DISCARD_OUTPUT;
			}

			crex_try {
				if (backup_crxdbg_compile) {
					crxdbg_compile_stdin(backup_crxdbg_compile);
				} else {
					crxdbg_compile();
				}
			} crex_end_try();
			backup_crxdbg_compile = NULL;

			CRXDBG_G(flags) &= ~CRXDBG_DISCARD_OUTPUT;
		}

		if (bp_tmp) {
			CRXDBG_G(flags) |= CRXDBG_DISCARD_OUTPUT | CRXDBG_IS_INITIALIZING;
			crxdbg_string_init(bp_tmp);
			free(bp_tmp);
			bp_tmp = NULL;
			CRXDBG_G(flags) &= ~CRXDBG_DISCARD_OUTPUT & ~CRXDBG_IS_INITIALIZING;
		}

		if (settings == (void *) 0x1) {
			if (CRXDBG_G(ops)) {
				crxdbg_print_opcodes(print_opline_func);
			} else {
				crex_quiet_write(CRXDBG_G(io)[CRXDBG_STDERR].fd, CREX_STRL("No opcodes could be compiled | No file specified or compilation failed?\n"));
			}
			goto crxdbg_out;
		}

		PG(during_request_startup) = 0;

		crxdbg_fully_started = 1;

		/* crxdbg main() */
		do {
			crex_try {
				if (crxdbg_startup_run) {
					crxdbg_startup_run = 0;
					if (quit_immediately) {
						CRXDBG_G(flags) = (CRXDBG_G(flags) & ~CRXDBG_HAS_PAGINATION) | CRXDBG_IS_INTERACTIVE | CRXDBG_PREVENT_INTERACTIVE;
					} else {
						CRXDBG_G(flags) |= CRXDBG_IS_INTERACTIVE;
					}
					crex_try {
						if (first_command) {
							crxdbg_interactive(1, estrdup(first_command));
						} else {
							CRXDBG_COMMAND_HANDLER(run)(NULL);
						}
					} crex_end_try();
					if (quit_immediately) {
						/* if -r is on the command line more than once just quit */
						EG(bailout) = __orig_bailout; /* reset crex_try */
						exit_status = EG(exit_status);
						break;
					}
				}

				CG(unclean_shutdown) = 0;
				crxdbg_interactive(1, NULL);
			} crex_catch {
				if ((CRXDBG_G(flags) & CRXDBG_IS_CLEANING)) {
					char *bp_tmp_str;
					CRXDBG_G(flags) |= CRXDBG_DISCARD_OUTPUT;
					crxdbg_export_breakpoints_to_string(&bp_tmp_str);
					CRXDBG_G(flags) &= ~CRXDBG_DISCARD_OUTPUT;
					if (bp_tmp_str) {
						bp_tmp = strdup(bp_tmp_str);
						free(bp_tmp_str);
					}
					cleaning = 1;
				} else {
					cleaning = 0;
				}
			} crex_end_try();
		} while (!(CRXDBG_G(flags) & CRXDBG_IS_STOPPING));

#ifdef _WIN32
	} __except(crxdbg_exception_handler_win32(xp = GetExceptionInformation())) {
		crxdbg_error("Access violation (Segmentation fault) encountered\ntrying to abort cleanly...");
	}
#endif
crxdbg_out:

		crxdbg_purge_watchpoint_tree();

		if (first_command) {
			free(first_command);
			first_command = NULL;
		}

		if (cleaning <= 0) {
			CRXDBG_G(flags) &= ~CRXDBG_IS_CLEANING;
			cleaning = -1;
		}

		{
			int i;
			/* free argv */
			for (i = SG(request_info).argc; i--;) {
				efree(SG(request_info).argv[i]);
			}
			efree(SG(request_info).argv);
		}

		crx_ini_builder_deinit(&ini_builder);

		if (ini_override) {
			free(ini_override);
		}

		/* In case we aborted during script execution, we may not reset CG(unclean_shutdown) */
		if (!(CRXDBG_G(flags) & CRXDBG_IS_RUNNING)) {
			is_exit = !CRXDBG_G(in_execution);
			CG(unclean_shutdown) = is_exit || CRXDBG_G(unclean_eval);
		}

		if ((CRXDBG_G(flags) & (CRXDBG_IS_CLEANING | CRXDBG_IS_RUNNING)) == CRXDBG_IS_CLEANING) {
			crx_free_shutdown_functions();
			crex_objects_store_mark_destructed(&EG(objects_store));
		}

		if (CRXDBG_G(exec) && strcmp("Standard input code", CRXDBG_G(exec)) == SUCCESS) { /* i.e. execution context has been read from stdin - back it up */
			crxdbg_file_source *data = crex_hash_str_find_ptr(&CRXDBG_G(file_sources), CRXDBG_G(exec), CRXDBG_G(exec_len));
			backup_crxdbg_compile = crex_string_alloc(data->len + 2, 1);
			GC_MAKE_PERSISTENT_LOCAL(backup_crxdbg_compile);
			sprintf(ZSTR_VAL(backup_crxdbg_compile), "?>%.*s", (int) data->len, data->buf);
		}

		crex_try {
			crx_request_shutdown(NULL);
		} crex_end_try();

		/* backup globals when cleaning */
		if ((cleaning > 0) && !quit_immediately) {
			settings = calloc(1, sizeof(crex_crxdbg_globals));

			crx_crxdbg_globals_ctor(settings);

			if (CRXDBG_G(exec)) {
				settings->exec = crex_strndup(CRXDBG_G(exec), CRXDBG_G(exec_len));
				settings->exec_len = CRXDBG_G(exec_len);
			}
			settings->prompt[0] = CRXDBG_G(prompt)[0];
			settings->prompt[1] = CRXDBG_G(prompt)[1];
			memcpy(CREX_VOIDP(settings->colors), CRXDBG_G(colors), sizeof(settings->colors));
			settings->input_buflen = CRXDBG_G(input_buflen);
			memcpy(settings->input_buffer, CRXDBG_G(input_buffer), settings->input_buflen);
			settings->flags = CRXDBG_G(flags) & CRXDBG_PRESERVE_FLAGS_MASK;
			first_command = CRXDBG_G(cur_command);
		} else {
			if (CRXDBG_G(prompt)[0]) {
				free(CRXDBG_G(prompt)[0]);
			}
			if (CRXDBG_G(prompt)[1]) {
				free(CRXDBG_G(prompt)[1]);
			}
			if (CRXDBG_G(cur_command)) {
				free(CRXDBG_G(cur_command));
			}
		}

		if (exit_status == 0) {
			exit_status = EG(exit_status);
		}

		crx_output_deactivate();

		if (!(CRXDBG_G(flags) & CRXDBG_IS_QUITTING)) {
			CRXDBG_G(flags) |= CRXDBG_IS_QUITTING;
			if (CRXDBG_G(in_execution) || is_exit) {
				if (!quit_immediately && !crxdbg_startup_run) {
					CRXDBG_G(flags) -= CRXDBG_IS_QUITTING;
					cleaning++;
				}
			}
		}

		{
			zval *zv = crex_hash_str_find(crx_stream_get_url_stream_wrappers_hash(), CREX_STRL("crx"));
			C_PTR_P(zv) = (void*)CRXDBG_G(orig_url_wrap_crx);
		}

#ifndef _WIN32
		/* force override (no crex_signals) to prevent crashes due to signal recursion in SIGSEGV/SIGBUS handlers */
		signal(SIGSEGV, SIG_DFL);
		signal(SIGBUS, SIG_DFL);

		/* reset it... else we risk a stack overflow upon next run (when clean'ing) */
		crx_stream_stdio_ops.write = CRXDBG_G(crx_stdiop_write);
#endif
	}

	crx_module_shutdown();

	sapi_shutdown();

	if (sapi_name) {
		free(sapi_name);
	}

free_and_return:
	if (read_from_stdin) {
		free(read_from_stdin);
		read_from_stdin = NULL;
	}

#ifdef ZTS
	/* reset to original handlers - otherwise CRXDBG_G() in crxdbg_watch_efree will be segfaulty (with e.g. USE_CREX_ALLOC=0) */
	if (!use_mm_wrappers) {
		crex_mm_set_custom_handlers(crex_mm_get_heap(), _malloc, _free, _realloc);
	}

	ts_free_id(crxdbg_globals_id);

	tsrm_shutdown();
#endif

	if ((cleaning > 0) && !quit_immediately) {
		/* reset internal crx_getopt state */
		crx_getopt(-1, argv, OPTIONS, NULL, &crx_optind, 0, 0);

		goto crxdbg_main;
	}

	if (backup_crxdbg_compile) {
		crex_string_free(backup_crxdbg_compile);
	}

	/* usually 0; just for -rr */
	return exit_status;
} /* }}} */
