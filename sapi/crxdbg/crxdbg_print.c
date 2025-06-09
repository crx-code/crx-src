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
#include "crxdbg_print.h"
#include "crxdbg_utils.h"
#include "crxdbg_prompt.h"

#include "Optimizer/crex_dump.h"

CREX_EXTERN_MODULE_GLOBALS(crxdbg)

#define CRXDBG_PRINT_COMMAND_D(f, h, a, m, l, s, flags) \
	CRXDBG_COMMAND_D_EXP(f, h, a, m, l, s, &crxdbg_prompt_commands[8], flags)

const crxdbg_command_t crxdbg_print_commands[] = {
	CRXDBG_PRINT_COMMAND_D(exec,       "print out the instructions in the main execution context", 'e', print_exec,   NULL, 0, CRXDBG_ASYNC_SAFE),
	CRXDBG_PRINT_COMMAND_D(opline,     "print out the instruction in the current opline",          'o', print_opline, NULL, 0, CRXDBG_ASYNC_SAFE),
	CRXDBG_PRINT_COMMAND_D(class,      "print out the instructions in the specified class",        'c', print_class,  NULL, "s", CRXDBG_ASYNC_SAFE),
	CRXDBG_PRINT_COMMAND_D(method,     "print out the instructions in the specified method",       'm', print_method, NULL, "m", CRXDBG_ASYNC_SAFE),
	CRXDBG_PRINT_COMMAND_D(func,       "print out the instructions in the specified function",     'f', print_func,   NULL, "s", CRXDBG_ASYNC_SAFE),
	CRXDBG_PRINT_COMMAND_D(stack,      "print out the instructions in the current stack",          's', print_stack,  NULL, 0, CRXDBG_ASYNC_SAFE),
	CRXDBG_END_COMMAND
};

CRXDBG_PRINT(opline) /* {{{ */
{
	if (CRXDBG_G(in_execution) && EG(current_execute_data)) {
		crxdbg_print_opline(crxdbg_user_execute_data(EG(current_execute_data)), 1);
	} else {
		crxdbg_error("Not Executing!");
	}

	return SUCCESS;
} /* }}} */

static inline void crxdbg_print_function_helper(crex_function *method) /* {{{ */
{
	switch (method->type) {
		case CREX_USER_FUNCTION: {
			crex_op_array* op_array = &(method->op_array);

			if (op_array) {
				crex_dump_op_array(op_array, CREX_DUMP_LINE_NUMBERS, NULL, NULL);

				for (uint32_t i = 0; i < op_array->num_dynamic_func_defs; i++) {
					crex_op_array *def = op_array->dynamic_func_defs[i];
					crxdbg_out("\ndynamic def: %i, function name: %.*s\n",
						i, (int) ZSTR_LEN(def->function_name), ZSTR_VAL(def->function_name));
					crex_dump_op_array(def, CREX_DUMP_LINE_NUMBERS, NULL, NULL);
				}
			}
		} break;

		default: {
			if (method->common.scope) {
				crxdbg_writeln("\tInternal %s::%s()", ZSTR_VAL(method->common.scope->name), ZSTR_VAL(method->common.function_name));
			} else {
				crxdbg_writeln("\tInternal %s()", ZSTR_VAL(method->common.function_name));
			}
		}
	}
} /* }}} */

CRXDBG_PRINT(exec) /* {{{ */
{
	if (CRXDBG_G(exec)) {
		if (!CRXDBG_G(ops) && !(CRXDBG_G(flags) & CRXDBG_IN_SIGNAL_HANDLER)) {
			crxdbg_compile();
		}

		if (CRXDBG_G(ops)) {
			crxdbg_notice("Context %s (%d ops)", CRXDBG_G(exec), CRXDBG_G(ops)->last);

			crxdbg_print_function_helper((crex_function*) CRXDBG_G(ops));
		}
	} else {
		crxdbg_error("No execution context set");
	}

	return SUCCESS;
} /* }}} */

CRXDBG_PRINT(stack) /* {{{ */
{
	if (CRXDBG_G(in_execution) && EG(current_execute_data)) {
		crex_op_array *ops = &crxdbg_user_execute_data(EG(current_execute_data))->func->op_array;
		if (ops->function_name) {
			if (ops->scope) {
				crxdbg_notice("Stack in %s::%s() (%d ops)", ZSTR_VAL(ops->scope->name), ZSTR_VAL(ops->function_name), ops->last);
			} else {
				crxdbg_notice("Stack in %s() (%d ops)", ZSTR_VAL(ops->function_name), ops->last);
			}
		} else {
			if (ops->filename) {
				crxdbg_notice("Stack in %s (%d ops)", ZSTR_VAL(ops->filename), ops->last);
			} else {
				crxdbg_notice("Stack @ %p (%d ops)", ops, ops->last);
			}
		}
		crxdbg_print_function_helper((crex_function*) ops);
	} else {
		crxdbg_error("Not Executing!");
	}

	return SUCCESS;
} /* }}} */

CRXDBG_PRINT(class) /* {{{ */
{
	crex_class_entry *ce;

	if (crxdbg_safe_class_lookup(param->str, param->len, &ce) == SUCCESS) {
		crxdbg_notice("%s %s: %s (%d methods)",
			(ce->type == CREX_USER_CLASS) ?
				"User" : "Internal",
			(ce->ce_flags & CREX_ACC_INTERFACE) ?
				"Interface" :
				(ce->ce_flags & CREX_ACC_ABSTRACT) ?
					"Abstract Class" :
					"Class",
			ZSTR_VAL(ce->name),
			crex_hash_num_elements(&ce->function_table));

		if (crex_hash_num_elements(&ce->function_table)) {
			crex_function *method;

			CREX_HASH_MAP_FOREACH_PTR(&ce->function_table, method) {
				crxdbg_print_function_helper(method);
			} CREX_HASH_FOREACH_END();
		}
	} else {
		crxdbg_error("The class %s could not be found", param->str);
	}

	return SUCCESS;
} /* }}} */

CRXDBG_PRINT(method) /* {{{ */
{
	crex_class_entry *ce;

	if (crxdbg_safe_class_lookup(param->method.class, strlen(param->method.class), &ce) == SUCCESS) {
		crex_function *fbc;
		crex_string *lcname = crex_string_alloc(strlen(param->method.name), 0);
		crex_str_tolower_copy(ZSTR_VAL(lcname), param->method.name, ZSTR_LEN(lcname));

		if ((fbc = crex_hash_find_ptr(&ce->function_table, lcname))) {
			crxdbg_notice("%s Method %s (%d ops)",
				(fbc->type == CREX_USER_FUNCTION) ? "User" : "Internal",
				ZSTR_VAL(fbc->common.function_name),
				(fbc->type == CREX_USER_FUNCTION) ? fbc->op_array.last : 0);

			crxdbg_print_function_helper(fbc);
		} else {
			crxdbg_error("The method %s::%s could not be found", param->method.class, param->method.name);
		}

		crex_string_release(lcname);
	} else {
		crxdbg_error("The class %s could not be found", param->method.class);
	}

	return SUCCESS;
} /* }}} */

CRXDBG_PRINT(func) /* {{{ */
{
	HashTable *func_table = EG(function_table);
	crex_function* fbc;
	const char *func_name = param->str;
	size_t func_name_len = param->len;
	crex_string *lcname;
	/* search active scope if begins with period */
	if (func_name[0] == '.') {
		crex_class_entry *scope = crex_get_executed_scope();

		if (scope) {
			func_name++;
			func_name_len--;

			func_table = &scope->function_table;
		} else {
			crxdbg_error("No active class");
			return SUCCESS;
		}
	} else if (!EG(function_table)) {
		crxdbg_error("No function table loaded");
		return SUCCESS;
	} else {
		func_table = EG(function_table);
	}

	lcname = crex_string_alloc(func_name_len, 0);
	crex_str_tolower_copy(ZSTR_VAL(lcname), func_name, ZSTR_LEN(lcname));

	crxdbg_try_access {
		if ((fbc = crex_hash_find_ptr(func_table, lcname))) {
			crxdbg_notice("%s %s %s (%d ops)",
				(fbc->type == CREX_USER_FUNCTION) ? "User" : "Internal",
				(fbc->common.scope) ? "Method" : "Function",
				ZSTR_VAL(fbc->common.function_name),
				(fbc->type == CREX_USER_FUNCTION) ? fbc->op_array.last : 0);

			crxdbg_print_function_helper(fbc);
		} else {
			crxdbg_error("The function %s could not be found", func_name);
		}
	} crxdbg_catch_access {
		crxdbg_error("Couldn't fetch function %.*s, invalid data source", (int) func_name_len, func_name);
	} crxdbg_end_try_access();

	efree(lcname);

	return SUCCESS;
} /* }}} */

void crxdbg_print_opcodes_main(void) {
	crxdbg_print_function_helper((crex_function *) CRXDBG_G(ops));
}

void crxdbg_print_opcodes_function(const char *function, size_t len) {
	crex_function *func = crex_hash_str_find_ptr(EG(function_table), function, len);

	if (!func) {
		crxdbg_error("The function %s could not be found", function);
		return;
	}

	crxdbg_print_function_helper(func);
}

static void crxdbg_print_opcodes_method_ce(crex_class_entry *ce, const char *function) {
	crex_function *func;

	if (ce->type != CREX_USER_CLASS) {
		crxdbg_out("function name: %s::%s (internal)\n", ce->name->val, function);
		return;
	}

	if (!(func = crex_hash_str_find_ptr(&ce->function_table, function, strlen(function)))) {
		crxdbg_error("The method %s::%s could not be found", ZSTR_VAL(ce->name), function);
		return;
	}

	crxdbg_print_function_helper(func);
}

void crxdbg_print_opcodes_method(const char *class, const char *function) {
	crex_class_entry *ce;

	if (crxdbg_safe_class_lookup(class, strlen(class), &ce) != SUCCESS) {
		crxdbg_error("The class %s could not be found", class);
		return;
	}

	crxdbg_print_opcodes_method_ce(ce, function);
}

static void crxdbg_print_opcodes_ce(crex_class_entry *ce) {
	crex_function *method;
	bool first = 1;

	crxdbg_out("%s %s: %s\n",
		(ce->type == CREX_USER_CLASS) ?
			"user" : "internal",
		(ce->ce_flags & CREX_ACC_INTERFACE) ?
			"interface" :
			(ce->ce_flags & CREX_ACC_ABSTRACT) ?
				"abstract Class" :
				"class",
		ZSTR_VAL(ce->name));

	if (ce->type != CREX_USER_CLASS) {
		return;
	}

	crxdbg_out("%d methods: ", crex_hash_num_elements(&ce->function_table));
	CREX_HASH_MAP_FOREACH_PTR(&ce->function_table, method) {
		if (first) {
			first = 0;
		} else {
			crxdbg_out(", ");
		}
		crxdbg_out("%s", ZSTR_VAL(method->common.function_name));
	} CREX_HASH_FOREACH_END();
	if (first) {
		crxdbg_out("-");
	}
	crxdbg_out("\n");

	CREX_HASH_MAP_FOREACH_PTR(&ce->function_table, method) {
		crxdbg_print_function_helper(method);
	} CREX_HASH_FOREACH_END();
}

void crxdbg_print_opcodes_class(const char *class) {
	crex_class_entry *ce;

	if (crxdbg_safe_class_lookup(class, strlen(class), &ce) != SUCCESS) {
		crxdbg_error("The class %s could not be found", class);
		return;
	}

	crxdbg_print_opcodes_ce(ce);
}

void crxdbg_print_opcodes(const char *function)
{
	if (function == NULL) {
		crxdbg_print_opcodes_main();
	} else if (function[0] == '*' && function[1] == 0) {
		/* all */
		crex_string *name;
		crex_function *func;
		crex_class_entry *ce;

		crxdbg_print_opcodes_main();

		CREX_HASH_MAP_FOREACH_STR_KEY_PTR(EG(function_table), name, func) {
			if (func->type == CREX_USER_FUNCTION) {
				crxdbg_print_opcodes_function(ZSTR_VAL(name), ZSTR_LEN(name));
			}
		} CREX_HASH_FOREACH_END();

		CREX_HASH_MAP_FOREACH_PTR(EG(class_table), ce) {
			if (ce->type == CREX_USER_CLASS) {
				crxdbg_out("\n");
				crxdbg_print_opcodes_ce(ce);
			}
		} CREX_HASH_FOREACH_END();
	} else {
		char *function_lowercase = crex_str_tolower_dup(function, strlen(function));

		if (strstr(function_lowercase, "::") == NULL) {
			crxdbg_print_opcodes_function(function_lowercase, strlen(function_lowercase));
		} else {
			char *method_name, *class_name = strtok(function_lowercase, "::");
			if ((method_name = strtok(NULL, "::")) == NULL) {
				crxdbg_print_opcodes_class(class_name);
			} else {
				crxdbg_print_opcodes_method(class_name, method_name);
			}
		}

		efree(function_lowercase);
	}
}

void crxdbg_print_opline(crex_execute_data *execute_data, bool ignore_flags) /* {{{ */
{
	if (ignore_flags || (!(CRXDBG_G(flags) & CRXDBG_IS_QUIET) && (CRXDBG_G(flags) & CRXDBG_IS_STEPPING))) {
		crex_dump_op_line(&EX(func)->op_array, NULL, EX(opline), CREX_DUMP_LINE_NUMBERS, NULL);
	}

	if (CRXDBG_G(oplog_list)) {
		crxdbg_oplog_entry *cur = crex_arena_alloc(&CRXDBG_G(oplog_arena), sizeof(crxdbg_oplog_entry));
		crex_op_array *op_array = &EX(func)->op_array;
		cur->op = (crex_op *) EX(opline);
		cur->opcodes = op_array->opcodes;
		cur->filename = op_array->filename;
		cur->scope = op_array->scope;
		cur->function_name = op_array->function_name;
		cur->next = NULL;
		CRXDBG_G(oplog_cur)->next = cur;
		CRXDBG_G(oplog_cur) = cur;
	}
} /* }}} */
