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

#include "crex.h"
#include "crxdbg.h"
#include "crxdbg_utils.h"
#include "crxdbg_frame.h"
#include "crxdbg_list.h"
#include "crex_smart_str.h"

CREX_EXTERN_MODULE_GLOBALS(crxdbg)

static inline void crxdbg_append_individual_arg(smart_str *s, uint32_t i, crex_function *func, zval *arg) {
	const crex_arg_info *arginfo = func->common.arg_info;
	char *arg_name = NULL;

	if (i) {
		smart_str_appends(s, ", ");
	}
	if (i < func->common.num_args) {
		if (arginfo) {
			if (func->type == CREX_INTERNAL_FUNCTION) {
				arg_name = (char *) ((crex_internal_arg_info *) &arginfo[i])->name;
			} else {
				arg_name = ZSTR_VAL(arginfo[i].name);
			}
		}
		smart_str_appends(s, arg_name ? arg_name : "?");
		smart_str_appendc(s, '=');
	}
	{
		char *arg_print = crxdbg_short_zval_print(arg, 40);
		smart_str_appends(s, arg_print);
		efree(arg_print);
	}
}

crex_string *crxdbg_compile_stackframe(crex_execute_data *ex) {
	smart_str s = {0};
	crex_op_array *op_array = &ex->func->op_array;
	uint32_t i = 0, first_extra_arg = op_array->num_args, num_args = CREX_CALL_NUM_ARGS(ex);
	zval *p = CREX_CALL_ARG(ex, 1);

	if (op_array->scope) {
		smart_str_append(&s, op_array->scope->name);
		smart_str_appends(&s, "::");
	}
	smart_str_append(&s, op_array->function_name);
	smart_str_appendc(&s, '(');
	if (CREX_CALL_NUM_ARGS(ex) > first_extra_arg) {
		while (i < first_extra_arg) {
			crxdbg_append_individual_arg(&s, i, ex->func, p);
			p++;
			i++;
		}
		p = CREX_CALL_VAR_NUM(ex, op_array->last_var + op_array->T);
	}
	while (i < num_args) {
		crxdbg_append_individual_arg(&s, i, ex->func, p);
		p++;
		i++;
	}
	smart_str_appendc(&s, ')');

	if (ex->func->type == CREX_USER_FUNCTION) {
		smart_str_appends(&s, " at ");
		smart_str_append(&s, op_array->filename);
		smart_str_appendc(&s, ':');
		smart_str_append_unsigned(&s, ex->opline->lineno);
	} else {
		smart_str_appends(&s, " [internal function]");
	}

	return s.s;
}

void crxdbg_print_cur_frame_info(void) {
	const char *file_chr = crex_get_executed_filename();
	crex_string *file = crex_string_init(file_chr, strlen(file_chr), 0);

	crxdbg_list_file(file, 3, crex_get_executed_lineno() - 1, crex_get_executed_lineno());
	efree(file);
}

void crxdbg_restore_frame(void) /* {{{ */
{
	if (CRXDBG_FRAME(num) == 0) {
		return;
	}

	if (CRXDBG_FRAME(generator)) {
		if (CRXDBG_FRAME(generator)->execute_data->call) {
			CRXDBG_FRAME(generator)->frozen_call_stack = crex_generator_freeze_call_stack(CRXDBG_FRAME(generator)->execute_data);
		}
		CRXDBG_FRAME(generator) = NULL;
	}

	CRXDBG_FRAME(num) = 0;

	/* move things back */
	EG(current_execute_data) = CRXDBG_FRAME(execute_data);
} /* }}} */

void crxdbg_switch_frame(int frame) /* {{{ */
{
	crex_execute_data *execute_data = CRXDBG_FRAME(num) ? CRXDBG_FRAME(execute_data) : EG(current_execute_data);
	int i = 0;

	if (CRXDBG_FRAME(num) == frame) {
		crxdbg_notice("Already in frame #%d", frame);
		return;
	}

	crxdbg_try_access {
		while (execute_data) {
			if (i++ == frame) {
				break;
			}

			do {
				execute_data = execute_data->prev_execute_data;
			} while (execute_data && execute_data->opline == NULL);
		}
	} crxdbg_catch_access {
		crxdbg_error("Couldn't switch frames, invalid data source");
		return;
	} crxdbg_end_try_access();

	if (execute_data == NULL) {
		crxdbg_error("No frame #%d", frame);
		return;
	}

	crxdbg_restore_frame();

	if (frame > 0) {
		CRXDBG_FRAME(num) = frame;

		/* backup things and jump back */
		CRXDBG_FRAME(execute_data) = EG(current_execute_data);
		EG(current_execute_data) = execute_data;
	}

	crxdbg_try_access {
		crex_string *s = crxdbg_compile_stackframe(EG(current_execute_data));
		crxdbg_notice("Switched to frame #%d: %.*s", frame, (int) ZSTR_LEN(s), ZSTR_VAL(s));
		crex_string_release(s);
	} crxdbg_catch_access {
		crxdbg_notice("Switched to frame #%d", frame);
	} crxdbg_end_try_access();

	crxdbg_print_cur_frame_info();
} /* }}} */

static void crxdbg_dump_prototype(zval *tmp) /* {{{ */
{
	zval *funcname, *class, class_zv, *args, *argstmp;

	funcname = crex_hash_find(C_ARRVAL_P(tmp), ZSTR_KNOWN(CREX_STR_FUNCTION));

	if ((class = crex_hash_find(C_ARRVAL_P(tmp), ZSTR_KNOWN(CREX_STR_OBJECT)))) {
		ZVAL_NEW_STR(&class_zv, C_OBJCE_P(class)->name);
		class = &class_zv;
	} else {
		class = crex_hash_find(C_ARRVAL_P(tmp), ZSTR_KNOWN(CREX_STR_CLASS));
	}

	if (class) {
		zval *type = crex_hash_find(C_ARRVAL_P(tmp), ZSTR_KNOWN(CREX_STR_TYPE));

		crxdbg_out("%s%s%s(", C_STRVAL_P(class), C_STRVAL_P(type), C_STRVAL_P(funcname));
	} else {
		crxdbg_out("%s(", C_STRVAL_P(funcname));
	}

	args = crex_hash_find(C_ARRVAL_P(tmp), ZSTR_KNOWN(CREX_STR_ARGS));

	if (args) {
		const crex_function *func = NULL;
		const crex_arg_info *arginfo = NULL;
		bool is_variadic = 0;
		int j = 0, m;

		crxdbg_try_access {
			/* assuming no autoloader call is necessary, class should have been loaded if it's in backtrace ... */
			if ((func = crxdbg_get_function(C_STRVAL_P(funcname), class ? C_STRVAL_P(class) : NULL))) {
				arginfo = func->common.arg_info;
			}
		} crxdbg_end_try_access();

		m = func ? func->common.num_args : 0;

		CREX_HASH_FOREACH_VAL(C_ARRVAL_P(args), argstmp) {
			if (j) {
				crxdbg_out(", ");
			}
			if (m && j < m) {
				char *arg_name = NULL;

				if (arginfo) {
					if (func->type == CREX_INTERNAL_FUNCTION) {
						arg_name = (char *)((crex_internal_arg_info *)&arginfo[j])->name;
					} else {
						arg_name = ZSTR_VAL(arginfo[j].name);
					}
				}

				if (!is_variadic) {
					is_variadic = arginfo ? CREX_ARG_IS_VARIADIC(&arginfo[j]) : 0;
				}

				crxdbg_out("%s=%s", arg_name ? arg_name : "?", is_variadic ? "[": "");

			}
			++j;

			{
				char *arg_print = crxdbg_short_zval_print(argstmp, 40);
				crx_printf("%s", arg_print);
				efree(arg_print);
			}
		} CREX_HASH_FOREACH_END();

		if (is_variadic) {
			crxdbg_out("]");
		}
	}
	crxdbg_out(")");
}

void crxdbg_dump_backtrace(size_t num) /* {{{ */
{
	HashPosition position;
	zval zbacktrace;
	zval *tmp;
	zval startline, startfile;
	const char *startfilename;
	zval *file = &startfile, *line = &startline;
	int i = 0, limit = num;

	CRXDBG_OUTPUT_BACKUP();

	if (limit < 0) {
		crxdbg_error("Invalid backtrace size %d", limit);

		CRXDBG_OUTPUT_BACKUP_RESTORE();
		return;
	}

	crxdbg_try_access {
		crex_fetch_debug_backtrace(&zbacktrace, 0, 0, limit);
	} crxdbg_catch_access {
		crxdbg_error("Couldn't fetch backtrace, invalid data source");
		return;
	} crxdbg_end_try_access();

	C_LVAL(startline) = crex_get_executed_lineno();
	startfilename = crex_get_executed_filename();
	C_STR(startfile) = crex_string_init(startfilename, strlen(startfilename), 0);

	crex_hash_internal_pointer_reset_ex(C_ARRVAL(zbacktrace), &position);
	tmp = crex_hash_get_current_data_ex(C_ARRVAL(zbacktrace), &position);
	while ((tmp = crex_hash_get_current_data_ex(C_ARRVAL(zbacktrace), &position))) {
		if (file) { /* userland */
			crxdbg_out("frame #%d: ", i);
			crxdbg_dump_prototype(tmp);
			crxdbg_out(" at %s:"CREX_LONG_FMT"\n", C_STRVAL_P(file), C_LVAL_P(line));
			i++;
		} else {
			crxdbg_out(" => ");
			crxdbg_dump_prototype(tmp);
			crxdbg_out(" (internal function)\n");
		}

		file = crex_hash_find(C_ARRVAL_P(tmp), ZSTR_KNOWN(CREX_STR_FILE));
		line = crex_hash_find(C_ARRVAL_P(tmp), ZSTR_KNOWN(CREX_STR_LINE));
		crex_hash_move_forward_ex(C_ARRVAL(zbacktrace), &position);
	}

	crxdbg_writeln("frame #%d: {main} at %s:"CREX_LONG_FMT, i, C_STRVAL_P(file), C_LVAL_P(line));

	zval_ptr_dtor_nogc(&zbacktrace);
	crex_string_release(C_STR(startfile));

	CRXDBG_OUTPUT_BACKUP_RESTORE();
} /* }}} */

void crxdbg_open_generator_frame(crex_generator *gen) {
	crex_string *s;

	if (EG(current_execute_data) == gen->execute_data) {
		return;
	}

	crxdbg_restore_frame();

	CRXDBG_FRAME(num) = -1;
	CRXDBG_FRAME(generator) = gen;

	EG(current_execute_data) = gen->execute_data;
	if (gen->frozen_call_stack) {
		crex_generator_restore_call_stack(gen);
	}
	gen->execute_data->prev_execute_data = NULL;

	s = crxdbg_compile_stackframe(EG(current_execute_data));
	crxdbg_notice("Switched to generator with handle #%d: %.*s", gen->std.handle, (int) ZSTR_LEN(s), ZSTR_VAL(s));
	crex_string_release(s);
	crxdbg_print_cur_frame_info();
}
