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

#include "crx.h"
#include "crxdbg.h"
#include "crxdbg_utils.h"
#include "crxdbg_info.h"
#include "crxdbg_bp.h"
#include "crxdbg_prompt.h"

CREX_EXTERN_MODULE_GLOBALS(crxdbg)

#define CRXDBG_INFO_COMMAND_D(f, h, a, m, l, s, flags) \
	CRXDBG_COMMAND_D_EXP(f, h, a, m, l, s, &crxdbg_prompt_commands[13], flags)

const crxdbg_command_t crxdbg_info_commands[] = {
	CRXDBG_INFO_COMMAND_D(break,     "show breakpoints",              'b', info_break,     NULL, 0, CRXDBG_ASYNC_SAFE),
	CRXDBG_INFO_COMMAND_D(files,     "show included files",           'F', info_files,     NULL, 0, CRXDBG_ASYNC_SAFE),
	CRXDBG_INFO_COMMAND_D(classes,   "show loaded classes",           'c', info_classes,   NULL, 0, CRXDBG_ASYNC_SAFE),
	CRXDBG_INFO_COMMAND_D(funcs,     "show loaded classes",           'f', info_funcs,     NULL, 0, CRXDBG_ASYNC_SAFE),
	CRXDBG_INFO_COMMAND_D(error,     "show last error",               'e', info_error,     NULL, 0, CRXDBG_ASYNC_SAFE),
	CRXDBG_INFO_COMMAND_D(constants, "show user defined constants",   'd', info_constants, NULL, 0, CRXDBG_ASYNC_SAFE),
	CRXDBG_INFO_COMMAND_D(vars,      "show active variables",         'v', info_vars,      NULL, 0, CRXDBG_ASYNC_SAFE),
	CRXDBG_INFO_COMMAND_D(globals,   "show superglobals",             'g', info_globals,   NULL, 0, CRXDBG_ASYNC_SAFE),
	CRXDBG_INFO_COMMAND_D(literal,   "show active literal constants", 'l', info_literal,   NULL, 0, CRXDBG_ASYNC_SAFE),
	CRXDBG_INFO_COMMAND_D(memory,    "show memory manager stats",     'm', info_memory,    NULL, 0, CRXDBG_ASYNC_SAFE),
	CRXDBG_END_COMMAND
};

CRXDBG_INFO(break) /* {{{ */
{
	crxdbg_print_breakpoints(CRXDBG_BREAK_FILE);
	crxdbg_print_breakpoints(CRXDBG_BREAK_SYM);
	crxdbg_print_breakpoints(CRXDBG_BREAK_METHOD);
	crxdbg_print_breakpoints(CRXDBG_BREAK_OPLINE);
	crxdbg_print_breakpoints(CRXDBG_BREAK_FILE_OPLINE);
	crxdbg_print_breakpoints(CRXDBG_BREAK_FUNCTION_OPLINE);
	crxdbg_print_breakpoints(CRXDBG_BREAK_METHOD_OPLINE);
	crxdbg_print_breakpoints(CRXDBG_BREAK_COND);
	crxdbg_print_breakpoints(CRXDBG_BREAK_OPCODE);

	return SUCCESS;
} /* }}} */

CRXDBG_INFO(files) /* {{{ */
{
	crex_string *fname;

	crxdbg_try_access {
		crxdbg_notice("Included files: %d", crex_hash_num_elements(&EG(included_files)));
	} crxdbg_catch_access {
		crxdbg_error("Could not fetch included file count, invalid data source");
		return SUCCESS;
	} crxdbg_end_try_access();

	crxdbg_try_access {
		CREX_HASH_MAP_FOREACH_STR_KEY(&EG(included_files), fname) {
			crxdbg_writeln("File: %s", ZSTR_VAL(fname));
		} CREX_HASH_FOREACH_END();
	} crxdbg_catch_access {
		crxdbg_error("Could not fetch file name, invalid data source, aborting included file listing");
	} crxdbg_end_try_access();

	return SUCCESS;
} /* }}} */

CRXDBG_INFO(error) /* {{{ */
{
	if (PG(last_error_message)) {
		crxdbg_try_access {
			crxdbg_writeln("Last error: %s at %s line %d",
			    ZSTR_VAL(PG(last_error_message)),
			    ZSTR_VAL(PG(last_error_file)),
			    PG(last_error_lineno));
		} crxdbg_catch_access {
			crxdbg_notice("No error found!");
		} crxdbg_end_try_access();
	} else {
		crxdbg_notice("No error found!");
	}
	return SUCCESS;
} /* }}} */

CRXDBG_INFO(constants) /* {{{ */
{
	HashTable consts;
	crex_constant *data;

	crex_hash_init(&consts, 8, NULL, NULL, 0);

	if (EG(crex_constants)) {
		crxdbg_try_access {
			CREX_HASH_MAP_FOREACH_PTR(EG(crex_constants), data) {
				if (CREX_CONSTANT_MODULE_NUMBER(data) == CRX_USER_CONSTANT) {
					crex_hash_update_ptr(&consts, data->name, data);
				}
			} CREX_HASH_FOREACH_END();
		} crxdbg_catch_access {
			crxdbg_error("Cannot fetch all the constants, invalid data source");
		} crxdbg_end_try_access();
	}

	crxdbg_notice("User-defined constants (%d)", crex_hash_num_elements(&consts));

	if (crex_hash_num_elements(&consts)) {
		crxdbg_out("Address            Refs    Type      Constant\n");
		CREX_HASH_MAP_FOREACH_PTR(&consts, data) {

#define VARIABLEINFO(msg, ...) \
	crxdbg_writeln( \
		"%-18p %-7d %-9s %.*s" msg, &data->value, \
		C_REFCOUNTED(data->value) ? C_REFCOUNT(data->value) : 1, \
		crex_get_type_by_const(C_TYPE(data->value)), \
		(int) ZSTR_LEN(data->name), ZSTR_VAL(data->name), ##__VA_ARGS__)

			switch (C_TYPE(data->value)) {
				case IS_STRING:
					crxdbg_try_access {
						VARIABLEINFO("\nstring (%zd) \"%.*s%s\"", C_STRLEN(data->value), C_STRLEN(data->value) < 255 ? (int) C_STRLEN(data->value) : 255, C_STRVAL(data->value), C_STRLEN(data->value) > 255 ? "..." : "");
					} crxdbg_catch_access {
						VARIABLEINFO("");
					} crxdbg_end_try_access();
					break;
				case IS_TRUE:
					VARIABLEINFO("\nbool (true)");
					break;
				case IS_FALSE:
					VARIABLEINFO("\nbool (false)");
					break;
				case IS_LONG:
					VARIABLEINFO("\nint ("CREX_LONG_FMT")", C_LVAL(data->value));
					break;
				case IS_DOUBLE:
					VARIABLEINFO("\ndouble (%lf)", C_DVAL(data->value));
					break;
				default:
					VARIABLEINFO("");

#undef VARIABLEINFO
			}
		} CREX_HASH_FOREACH_END();
	}

	return SUCCESS;
} /* }}} */

static int crxdbg_arm_auto_global(zval *ptrzv) {
	crex_auto_global *auto_global = C_PTR_P(ptrzv);

	if (auto_global->armed) {
		if (CRXDBG_G(flags) & CRXDBG_IN_SIGNAL_HANDLER) {
			crxdbg_notice("Cannot show information about superglobal variable %.*s", (int) ZSTR_LEN(auto_global->name), ZSTR_VAL(auto_global->name));
		} else {
			auto_global->armed = auto_global->auto_global_callback(auto_global->name);
		}
	}

	return 0;
}

static int crxdbg_print_symbols(bool show_globals) {
	HashTable vars;
	crex_array *symtable;
	crex_string *var;
	zval *data;

	if (!EG(current_execute_data) || !EG(current_execute_data)->func) {
		crxdbg_error("No active op array!");
		return SUCCESS;
	}

	if (show_globals) {
		/* that array should only be manipulated during init, so safe for async access during execution */
		crex_hash_apply(CG(auto_globals), (apply_func_t) crxdbg_arm_auto_global);
		symtable = &EG(symbol_table);
	} else if (!(symtable = crex_rebuild_symbol_table())) {
		crxdbg_error("No active symbol table!");
		return SUCCESS;
	}

	crex_hash_init(&vars, 8, NULL, NULL, 0);

	crxdbg_try_access {
		CREX_HASH_MAP_FOREACH_STR_KEY_VAL(symtable, var, data) {
			if (crex_is_auto_global(var) ^ !show_globals) {
				crex_hash_update(&vars, var, data);
			}
		} CREX_HASH_FOREACH_END();
	} crxdbg_catch_access {
		crxdbg_error("Cannot fetch all data from the symbol table, invalid data source");
	} crxdbg_end_try_access();

	if (show_globals) {
		crxdbg_notice("Superglobal variables (%d)", crex_hash_num_elements(&vars));
	} else {
		crex_op_array *ops = &EG(current_execute_data)->func->op_array;

		if (ops->function_name) {
			if (ops->scope) {
				crxdbg_notice("Variables in %s::%s() (%d)", ops->scope->name->val, ops->function_name->val, crex_hash_num_elements(&vars));
			} else {
				crxdbg_notice("Variables in %s() (%d)", ZSTR_VAL(ops->function_name), crex_hash_num_elements(&vars));
			}
		} else {
			if (ops->filename) {
				crxdbg_notice("Variables in %s (%d)", ZSTR_VAL(ops->filename), crex_hash_num_elements(&vars));
			} else {
				crxdbg_notice("Variables @ %p (%d)", ops, crex_hash_num_elements(&vars));
			}
		}
	}

	if (crex_hash_num_elements(&vars)) {
		crxdbg_out("Address            Refs    Type      Variable\n");
		CREX_HASH_MAP_FOREACH_STR_KEY_VAL(&vars, var, data) {
			crxdbg_try_access {
				const char *isref = "";
#define VARIABLEINFO(msg, ...) \
	crxdbg_writeln( \
		"%-18p %-7d %-9s %s$%.*s" msg, data, C_REFCOUNTED_P(data) ? C_REFCOUNT_P(data) : 1, crex_get_type_by_const(C_TYPE_P(data)), isref, (int) ZSTR_LEN(var), ZSTR_VAL(var), ##__VA_ARGS__)
retry_switch:
				switch (C_TYPE_P(data)) {
					case IS_RESOURCE:
						crxdbg_try_access {
							const char *type = crex_rsrc_list_get_rsrc_type(C_RES_P(data));
							VARIABLEINFO("\n|-------(typeof)------> (%s)\n", type ? type : "unknown");
						} crxdbg_catch_access {
							VARIABLEINFO("\n|-------(typeof)------> (unknown)\n");
						} crxdbg_end_try_access();
						break;
					case IS_OBJECT:
						crxdbg_try_access {
							VARIABLEINFO("\n|-----(instanceof)----> (%s)\n", ZSTR_VAL(C_OBJCE_P(data)->name));
						} crxdbg_catch_access {
							VARIABLEINFO("\n|-----(instanceof)----> (unknown)\n");
						} crxdbg_end_try_access();
						break;
					case IS_STRING:
						crxdbg_try_access {
							VARIABLEINFO("\nstring (%zd) \"%.*s%s\"", C_STRLEN_P(data), C_STRLEN_P(data) < 255 ? (int) C_STRLEN_P(data) : 255, C_STRVAL_P(data), C_STRLEN_P(data) > 255 ? "..." : "");
						} crxdbg_catch_access {
							VARIABLEINFO("");
						} crxdbg_end_try_access();
						break;
					case IS_TRUE:
						VARIABLEINFO("\nbool (true)");
						break;
					case IS_FALSE:
						VARIABLEINFO("\nbool (false)");
						break;
					case IS_LONG:
						VARIABLEINFO("\nint ("CREX_LONG_FMT")", C_LVAL_P(data));
						break;
					case IS_DOUBLE:
						VARIABLEINFO("\ndouble (%lf)", C_DVAL_P(data));
						break;
					case IS_REFERENCE:
						isref = "&";
						data = C_REFVAL_P(data);
						goto retry_switch;
					case IS_INDIRECT:
						data = C_INDIRECT_P(data);
						goto retry_switch;
					default:
						VARIABLEINFO("");
				}
#undef VARIABLEINFO
			} crxdbg_catch_access {
				crxdbg_writeln("%p\tn/a\tn/a\t$%s", data, ZSTR_VAL(var));
			} crxdbg_end_try_access();
		} CREX_HASH_FOREACH_END();
	}

	crex_hash_destroy(&vars);

	return SUCCESS;
} /* }}} */

CRXDBG_INFO(vars) /* {{{ */
{
	return crxdbg_print_symbols(0);
}

CRXDBG_INFO(globals) /* {{{ */
{
	return crxdbg_print_symbols(1);
}

CRXDBG_INFO(literal) /* {{{ */
{
	/* literals are assumed to not be manipulated during executing of their op_array and as such async safe */
	bool in_executor = CRXDBG_G(in_execution) && EG(current_execute_data) && EG(current_execute_data)->func;
	if (in_executor || CRXDBG_G(ops)) {
		crex_op_array *ops = in_executor ? &EG(current_execute_data)->func->op_array : CRXDBG_G(ops);
		int literal = 0, count = ops->last_literal - 1;

		if (ops->function_name) {
			if (ops->scope) {
				crxdbg_notice("Literal Constants in %s::%s() (%d)", ops->scope->name->val, ops->function_name->val, count);
			} else {
				crxdbg_notice("Literal Constants in %s() (%d)", ops->function_name->val, count);
			}
		} else {
			if (ops->filename) {
				crxdbg_notice("Literal Constants in %s (%d)", ZSTR_VAL(ops->filename), count);
			} else {
				crxdbg_notice("Literal Constants @ %p (%d)", ops, count);
			}
		}

		while (literal < ops->last_literal) {
			if (C_TYPE(ops->literals[literal]) != IS_NULL) {
				crxdbg_write("|-------- C%u -------> [", literal);
				crex_print_zval(&ops->literals[literal], 0);
				crxdbg_out("]\n");
			}
			literal++;
		}
	} else {
		crxdbg_error("Not executing!");
	}

	return SUCCESS;
} /* }}} */

CRXDBG_INFO(memory) /* {{{ */
{
	size_t used, real, peak_used, peak_real;
	crex_mm_heap *orig_heap = NULL;
	bool is_mm;

	if (CRXDBG_G(flags) & CRXDBG_IN_SIGNAL_HANDLER) {
		orig_heap = crex_mm_set_heap(crxdbg_original_heap_sigsafe_mem());
	}
	if ((is_mm = is_crex_mm())) {
		used = crex_memory_usage(0);
		real = crex_memory_usage(1);
		peak_used = crex_memory_peak_usage(0);
		peak_real = crex_memory_peak_usage(1);
	}
	if (orig_heap) {
		crex_mm_set_heap(orig_heap);
	}

	if (is_mm) {
		crxdbg_notice("Memory Manager Information");
		crxdbg_notice("Current");
		crxdbg_writeln( "|-------> Used:\t%.3f kB", (float) (used / 1024));
		crxdbg_writeln("|-------> Real:\t%.3f kB", (float) (real / 1024));
		crxdbg_notice("Peak");
		crxdbg_writeln("|-------> Used:\t%.3f kB", (float) (peak_used / 1024));
		crxdbg_writeln("|-------> Real:\t%.3f kB", (float) (peak_real / 1024));
	} else {
		crxdbg_error("Memory Manager Disabled!");
	}
	return SUCCESS;
} /* }}} */

static inline void crxdbg_print_class_name(crex_class_entry *ce) /* {{{ */
{
	const char *visibility = ce->type == CREX_USER_CLASS ? "User" : "Internal";
	const char *type = (ce->ce_flags & CREX_ACC_INTERFACE) ? "Interface" : (ce->ce_flags & CREX_ACC_ABSTRACT) ? "Abstract Class" : "Class";

	crxdbg_writeln("%s %s %.*s (%d)", visibility, type, (int) ZSTR_LEN(ce->name), ZSTR_VAL(ce->name), crex_hash_num_elements(&ce->function_table));
} /* }}} */

CRXDBG_INFO(classes) /* {{{ */
{
	crex_class_entry *ce;
	HashTable classes;

	crex_hash_init(&classes, 8, NULL, NULL, 0);

	crxdbg_try_access {
		CREX_HASH_MAP_FOREACH_PTR(EG(class_table), ce) {
			if (ce->type == CREX_USER_CLASS) {
				crex_hash_next_index_insert_ptr(&classes, ce);
			}
		} CREX_HASH_FOREACH_END();
	} crxdbg_catch_access {
		crxdbg_notice("Not all classes could be fetched, possibly invalid data source");
	} crxdbg_end_try_access();

	crxdbg_notice("User Classes (%d)", crex_hash_num_elements(&classes));

	/* once added, assume that classes are stable... until shutdown. */
	CREX_HASH_PACKED_FOREACH_PTR(&classes, ce) {
		crxdbg_print_class_name(ce);

		if (ce->parent) {
			crex_class_entry *pce;
			pce = ce->parent;
			do {
				crxdbg_out("|-------- ");
				crxdbg_print_class_name(pce);
			} while ((pce = pce->parent));
		}

		if (ce->info.user.filename) {
			crxdbg_writeln("|---- in %s on line %u", ZSTR_VAL(ce->info.user.filename), ce->info.user.line_start);
		} else {
			crxdbg_writeln("|---- no source code");
		}
	} CREX_HASH_FOREACH_END();

	crex_hash_destroy(&classes);

	return SUCCESS;
} /* }}} */

CRXDBG_INFO(funcs) /* {{{ */
{
	crex_function *zf;
	HashTable functions;

	crex_hash_init(&functions, 8, NULL, NULL, 0);

	crxdbg_try_access {
		CREX_HASH_MAP_FOREACH_PTR(EG(function_table), zf) {
			if (zf->type == CREX_USER_FUNCTION) {
				crex_hash_next_index_insert_ptr(&functions, zf);
			}
		} CREX_HASH_FOREACH_END();
	} crxdbg_catch_access {
		crxdbg_notice("Not all functions could be fetched, possibly invalid data source");
	} crxdbg_end_try_access();

	crxdbg_notice("User Functions (%d)", crex_hash_num_elements(&functions));

	CREX_HASH_PACKED_FOREACH_PTR(&functions, zf) {
		crex_op_array *op_array = &zf->op_array;

		crxdbg_write("|-------- %s", op_array->function_name ? ZSTR_VAL(op_array->function_name) : "{main}");

		if (op_array->filename) {
			crxdbg_writeln(" in %s on line %d", ZSTR_VAL(op_array->filename), op_array->line_start);
		} else {
			crxdbg_writeln(" (no source code)");
		}
	} CREX_HASH_FOREACH_END();

	crex_hash_destroy(&functions);

	return SUCCESS;
} /* }}} */
