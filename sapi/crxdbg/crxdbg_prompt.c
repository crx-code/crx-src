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

#include <stdio.h>
#include <string.h>
#include "crex.h"
#include "crex_compile.h"
#include "crex_exceptions.h"
#include "crex_vm.h"
#include "crex_generators.h"
#include "crex_interfaces.h"
#include "crex_smart_str.h"
#include "crxdbg.h"
#include "crxdbg_io.h"

#include "crxdbg_help.h"
#include "crxdbg_print.h"
#include "crxdbg_info.h"
#include "crxdbg_break.h"
#include "crxdbg_list.h"
#include "crxdbg_utils.h"
#include "crxdbg_prompt.h"
#include "crxdbg_cmd.h"
#include "crxdbg_set.h"
#include "crxdbg_frame.h"
#include "crxdbg_lexer.h"
#include "crxdbg_parser.h"

#if CREX_VM_KIND != CREX_VM_KIND_CALL && CREX_VM_KIND != CREX_VM_KIND_HYBRID
#error "crxdbg can only be built with CALL crex vm kind"
#endif

CREX_EXTERN_MODULE_GLOBALS(crxdbg)
extern int crxdbg_startup_run;

#ifdef HAVE_LIBDL
#ifdef CRX_WIN32
#include "win32/param.h"
#include "win32/winutil.h"
#define GET_DL_ERROR()  crx_win_err()
#else
#include <sys/param.h>
#define GET_DL_ERROR()  DL_ERROR()
#endif
#endif

/* {{{ command declarations */
const crxdbg_command_t crxdbg_prompt_commands[] = {
	CRXDBG_COMMAND_D(exec,      "set execution context",                    'e', NULL, "s", 0),
	CRXDBG_COMMAND_D(stdin,     "read script from stdin",                    0 , NULL, "s", 0),
	CRXDBG_COMMAND_D(step,      "step through execution",                   's', NULL, 0, CRXDBG_ASYNC_SAFE),
	CRXDBG_COMMAND_D(continue,  "continue execution",                       'c', NULL, 0, CRXDBG_ASYNC_SAFE),
	CRXDBG_COMMAND_D(run,       "attempt execution",                        'r', NULL, "|s", 0),
	CRXDBG_COMMAND_D(ev,        "evaluate some code",                        0 , NULL, "i", CRXDBG_ASYNC_SAFE), /* restricted ASYNC_SAFE */
	CRXDBG_COMMAND_D(until,     "continue past the current line",           'u', NULL, 0, 0),
	CRXDBG_COMMAND_D(finish,    "continue past the end of the stack",       'F', NULL, 0, 0),
	CRXDBG_COMMAND_D(leave,     "continue until the end of the stack",      'L', NULL, 0, 0),
	CRXDBG_COMMAND_D(generator, "inspect or switch to a generator",         'g', NULL, "|n", 0),
	CRXDBG_COMMAND_D(print,     "print something",                          'p', crxdbg_print_commands, "|*c", 0),
	CRXDBG_COMMAND_D(break,     "set breakpoint",                           'b', crxdbg_break_commands, "|*c", 0),
	CRXDBG_COMMAND_D(back,      "show trace",                               't', NULL, "|n", CRXDBG_ASYNC_SAFE),
	CRXDBG_COMMAND_D(frame,     "switch to a frame",                        'f', NULL, "|n", CRXDBG_ASYNC_SAFE),
	CRXDBG_COMMAND_D(list,      "lists some code",                          'l', crxdbg_list_commands,  "*", CRXDBG_ASYNC_SAFE),
	CRXDBG_COMMAND_D(info,      "displays some information",               'i', crxdbg_info_commands, "|s", CRXDBG_ASYNC_SAFE),
	CRXDBG_COMMAND_D(clean,     "clean the execution environment",          'X', NULL, 0, 0),
	CRXDBG_COMMAND_D(clear,     "clear breakpoints",                        'C', NULL, 0, 0),
	CRXDBG_COMMAND_D(help,      "show help menu",                           'h', crxdbg_help_commands, "|s", CRXDBG_ASYNC_SAFE),
	CRXDBG_COMMAND_D(set,       "set crxdbg configuration",                 'S', crxdbg_set_commands,   "s", CRXDBG_ASYNC_SAFE),
	CRXDBG_COMMAND_D(register,  "register a function",                      'R', NULL, "s", 0),
	CRXDBG_COMMAND_D(source,    "execute a crxdbginit",                     '<', NULL, "s", 0),
	CRXDBG_COMMAND_D(export,    "export breaks to a .crxdbginit script",    '>', NULL, "s", CRXDBG_ASYNC_SAFE),
	CRXDBG_COMMAND_D(sh,   	    "shell a command",                           0 , NULL, "i", 0),
	CRXDBG_COMMAND_D(quit,      "exit crxdbg",                              'q', NULL, 0, CRXDBG_ASYNC_SAFE),
	CRXDBG_COMMAND_D(watch,     "set watchpoint",                           'w', crxdbg_watch_commands, "|ss", 0),
	CRXDBG_COMMAND_D(next,      "step over next line",                      'n', NULL, 0, CRXDBG_ASYNC_SAFE),
	CRXDBG_END_COMMAND
}; /* }}} */

static inline int crxdbg_call_register(crxdbg_param_t *stack) /* {{{ */
{
	crxdbg_param_t *name = NULL;

	if (stack->type == STACK_PARAM) {
		char *lc_name;

		name = stack->next;

		if (!name || name->type != STR_PARAM) {
			return FAILURE;
		}

		lc_name = crex_str_tolower_dup(name->str, name->len);

		if (crex_hash_str_exists(&CRXDBG_G(registered), lc_name, name->len)) {
			zval fretval;
			crex_fcall_info fci;

			memset(&fci, 0, sizeof(crex_fcall_info));

			ZVAL_STRINGL(&fci.function_name, lc_name, name->len);
			fci.size = sizeof(crex_fcall_info);
			fci.object = NULL;
			fci.retval = &fretval;
			fci.param_count = 0;
			fci.params = NULL;
			fci.named_params = NULL;

			zval params;
			if (name->next) {
				crxdbg_param_t *next = name->next;

				array_init(&params);

				while (next) {
					char *buffered = NULL;

					switch (next->type) {
						case OP_PARAM:
						case COND_PARAM:
						case STR_PARAM:
							add_next_index_stringl(&params, next->str, next->len);
						break;

						case NUMERIC_PARAM:
							add_next_index_long(&params, next->num);
						break;

						case METHOD_PARAM:
							spprintf(&buffered, 0, "%s::%s", next->method.class, next->method.name);
							add_next_index_string(&params, buffered);
						break;

						case NUMERIC_METHOD_PARAM:
							spprintf(&buffered, 0, "%s::%s#"CREX_LONG_FMT, next->method.class, next->method.name, next->num);
							add_next_index_string(&params, buffered);
						break;

						case NUMERIC_FUNCTION_PARAM:
							spprintf(&buffered, 0, "%s#"CREX_LONG_FMT, next->str, next->num);
							add_next_index_string(&params, buffered);
						break;

						case FILE_PARAM:
							spprintf(&buffered, 0, "%s:"CREX_ULONG_FMT, next->file.name, next->file.line);
							add_next_index_string(&params, buffered);
						break;

						case NUMERIC_FILE_PARAM:
							spprintf(&buffered, 0, "%s:#"CREX_ULONG_FMT, next->file.name, next->file.line);
							add_next_index_string(&params, buffered);
						break;

						default: {
							/* not yet */
						}
					}

					next = next->next;
				}
				/* Add positional arguments */
				fci.named_params = C_ARRVAL(params);
			}

			crxdbg_activate_err_buf(0);
			crxdbg_free_err_buf();

			crxdbg_debug("created %d params from arguments", fci.param_count);

			if (crex_call_function(&fci, NULL) == SUCCESS) {
				crex_print_zval_r(&fretval, 0);
				crxdbg_out("\n");
				zval_ptr_dtor(&fretval);
			}

			zval_ptr_dtor_str(&fci.function_name);
			efree(lc_name);

			return SUCCESS;
		}

		efree(lc_name);
	}

	return FAILURE;
} /* }}} */

struct crxdbg_init_state {
	int line;
	bool in_code;
	char *code;
	size_t code_len;
	const char *init_file;
};

static void crxdbg_line_init(char *cmd, struct crxdbg_init_state *state) {
	size_t cmd_len = strlen(cmd);

	state->line++;

	while (cmd_len > 0L && isspace(cmd[cmd_len-1])) {
		cmd_len--;
	}

	cmd[cmd_len] = '\0';

	if (*cmd && cmd_len > 0L && cmd[0] != '#') {
		if (cmd_len == 2) {
			if (memcmp(cmd, "<:", sizeof("<:")-1) == SUCCESS) {
				state->in_code = 1;
				return;
			} else {
				if (memcmp(cmd, ":>", sizeof(":>")-1) == SUCCESS) {
					state->in_code = 0;
					state->code[state->code_len] = '\0';
					crex_eval_stringl(state->code, state->code_len, NULL, "crxdbginit code");
					free(state->code);
					state->code = NULL;
					return;
				}
			}
		}

		if (state->in_code) {
			if (state->code == NULL) {
				state->code = malloc(cmd_len + 1);
			} else {
				state->code = realloc(state->code, state->code_len + cmd_len + 1);
			}

			if (state->code) {
				memcpy(&state->code[state->code_len], cmd, cmd_len);
				state->code_len += cmd_len;
			}

			return;
		}

		crex_try {
			char *input = crxdbg_read_input(cmd);
			crxdbg_param_t stack;

			crxdbg_init_param(&stack, STACK_PARAM);

			crxdbg_activate_err_buf(1);

			if (crxdbg_do_parse(&stack, input) <= 0) {
				switch (crxdbg_stack_execute(&stack, 1 /* allow_async_unsafe == 1 */)) {
					case FAILURE:
						crxdbg_activate_err_buf(0);
						if (crxdbg_call_register(&stack) == FAILURE) {
							if (state->init_file) {
								crxdbg_output_err_buf("Unrecognized command in %s:%d: %s, %s!", state->init_file, state->line, input, CRXDBG_G(err_buf).msg);
							} else {
								crxdbg_output_err_buf("Unrecognized command on line %d: %s, %s!", state->line, input, CRXDBG_G(err_buf).msg);
							}
						}
					break;
				}
			}

			crxdbg_activate_err_buf(0);
			crxdbg_free_err_buf();

			crxdbg_stack_free(&stack);
			crxdbg_destroy_input(&input);
		} crex_catch {
			CRXDBG_G(flags) &= ~(CRXDBG_IS_RUNNING | CRXDBG_IS_CLEANING);
			if (CRXDBG_G(flags) & CRXDBG_IS_QUITTING) {
				crex_bailout();
			}
		} crex_end_try();
	}

}

void crxdbg_string_init(char *buffer) {
	struct crxdbg_init_state state = {0};
	char *str = strtok(buffer, "\n");

	while (str) {
		crxdbg_line_init(str, &state);

		str = strtok(NULL, "\n");
	}

	if (state.code) {
		free(state.code);
	}
}

void crxdbg_try_file_init(char *init_file, size_t init_file_len, bool free_init) /* {{{ */
{
	crex_stat_t sb = {0};

	if (init_file && VCWD_STAT(init_file, &sb) != -1) {
		FILE *fp = fopen(init_file, "r");
		if (fp) {
			char cmd[CRXDBG_MAX_CMD];
			struct crxdbg_init_state state = {0};

			state.init_file = init_file;

			while (fgets(cmd, CRXDBG_MAX_CMD, fp) != NULL) {
				crxdbg_line_init(cmd, &state);
			}

			if (state.code) {
				free(state.code);
			}

			fclose(fp);
		} else {
			crxdbg_error("Failed to open %s for initialization", init_file);
		}

		if (free_init) {
			free(init_file);
		}
	}
} /* }}} */

void crxdbg_init(char *init_file, size_t init_file_len, bool use_default) /* {{{ */
{
	if (init_file) {
		crxdbg_try_file_init(init_file, init_file_len, 1);
	} else if (use_default) {
		char *scan_dir = getenv("CRX_INI_SCAN_DIR");
		char *sys_ini;
		int i;

		CREX_IGNORE_VALUE(asprintf(&sys_ini, "%s/" CRXDBG_INIT_FILENAME, CRX_CONFIG_FILE_PATH));
		crxdbg_try_file_init(sys_ini, strlen(sys_ini), 0);
		free(sys_ini);

		if (!scan_dir) {
			scan_dir = CRX_CONFIG_FILE_SCAN_DIR;
		}
		while (*scan_dir != 0) {
			i = 0;
			while (scan_dir[i] != ':') {
				if (scan_dir[i++] == 0) {
					i = -1;
					break;
				}
			}
			if (i != -1) {
				scan_dir[i] = 0;
			}

			CREX_IGNORE_VALUE(asprintf(&init_file, "%s/%s", scan_dir, CRXDBG_INIT_FILENAME));
			crxdbg_try_file_init(init_file, strlen(init_file), 1);
			free(init_file);
			if (i == -1) {
				break;
			}
			scan_dir += i + 1;
		}

		crxdbg_try_file_init(CRXDBG_STRL(CRXDBG_INIT_FILENAME), 0);
	}
}
/* }}} */

void crxdbg_clean(bool full, bool resubmit) /* {{{ */
{
	/* this is implicitly required */
	if (CRXDBG_G(ops)) {
		destroy_op_array(CRXDBG_G(ops));
		efree(CRXDBG_G(ops));
		CRXDBG_G(ops) = NULL;
	}

	if (!resubmit && CRXDBG_G(cur_command)) {
		free(CRXDBG_G(cur_command));
		CRXDBG_G(cur_command) = NULL;
	}

	if (full) {
		CRXDBG_G(flags) |= CRXDBG_IS_CLEANING;
	}
} /* }}} */

CRXDBG_COMMAND(exec) /* {{{ */
{
	crex_stat_t sb = {0};

	if (VCWD_STAT(param->str, &sb) != FAILURE) {
		if (sb.st_mode & (S_IFREG|S_IFLNK)) {
			char *res = crxdbg_resolve_path(param->str);
			size_t res_len = strlen(res);

			if ((res_len != CRXDBG_G(exec_len)) || (memcmp(res, CRXDBG_G(exec), res_len) != SUCCESS)) {
				if (CRXDBG_G(in_execution)) {
					if (crxdbg_ask_user_permission("Do you really want to stop execution to set a new execution context?") == FAILURE) {
						free(res);
						return FAILURE;
					}
				}

				if (CRXDBG_G(exec)) {
					crxdbg_notice("Unsetting old execution context: %s", CRXDBG_G(exec));
					free(CRXDBG_G(exec));
					CRXDBG_G(exec) = NULL;
					CRXDBG_G(exec_len) = 0L;
				}

				if (CRXDBG_G(ops)) {
					crxdbg_notice("Destroying compiled opcodes");
					crxdbg_clean(0, 0);
				}

				CRXDBG_G(exec) = res;
				CRXDBG_G(exec_len) = res_len;

				VCWD_CHDIR_FILE(res);

				*SG(request_info).argv = estrndup(CRXDBG_G(exec), CRXDBG_G(exec_len));
				crx_build_argv(NULL, &PG(http_globals)[TRACK_VARS_SERVER]);

				crxdbg_notice("Set execution context: %s", CRXDBG_G(exec));

				if (CRXDBG_G(in_execution)) {
					crxdbg_clean(1, 0);
					return SUCCESS;
				}

				crxdbg_compile();
			} else {
				free(res);
				crxdbg_notice("Execution context not changed");
			}
		} else {
			crxdbg_error("Cannot use %s as execution context, not a valid file or symlink", param->str);
		}
	} else {
		crxdbg_error("Cannot stat %s, ensure the file exists", param->str);
	}
	return SUCCESS;
} /* }}} */

CRXDBG_COMMAND(stdin)
{
	smart_str code = {0};
	char *buf;
	char *sep = param->str;
	int seplen = param->len;
	int bytes = 0;

	smart_str_appends(&code, "?>");

	do {
		CRXDBG_G(input_buflen) += bytes;
		if (CRXDBG_G(input_buflen) <= 0) {
			continue;
		}

		if (sep && seplen) {
			char *nl = buf = CRXDBG_G(input_buffer);
			do {
				if (buf == nl + seplen) {
					if (!memcmp(sep, nl, seplen) && (*buf == '\n' || (*buf == '\r' && buf[1] == '\n'))) {
						smart_str_appendl(&code, CRXDBG_G(input_buffer), nl - CRXDBG_G(input_buffer));
						memmove(CRXDBG_G(input_buffer), ++buf, --CRXDBG_G(input_buflen));
						goto exec_code;
					}
				}
				if (*buf == '\n') {
					nl = buf + 1;
				}
				buf++;
			} while (--CRXDBG_G(input_buflen));
			if (buf != nl && buf <= nl + seplen) {
				smart_str_appendl(&code, CRXDBG_G(input_buffer), nl - CRXDBG_G(input_buffer));
				CRXDBG_G(input_buflen) = buf - nl;
				memmove(CRXDBG_G(input_buffer), nl, CRXDBG_G(input_buflen));
			} else {
				CRXDBG_G(input_buflen) = 0;
				smart_str_appendl(&code, CRXDBG_G(input_buffer), buf - CRXDBG_G(input_buffer));
			}
		} else {
			smart_str_appendl(&code, CRXDBG_G(input_buffer), CRXDBG_G(input_buflen));
			CRXDBG_G(input_buflen) = 0;
		}
	} while ((bytes = crxdbg_mixed_read(CRXDBG_G(io)[CRXDBG_STDIN].fd, CRXDBG_G(input_buffer) + CRXDBG_G(input_buflen), CRXDBG_MAX_CMD - CRXDBG_G(input_buflen), -1)) > 0);

	if (bytes < 0) {
		CRXDBG_G(flags) |= CRXDBG_IS_QUITTING;
		crex_bailout();
	}

exec_code:
	smart_str_0(&code);

	if (crxdbg_compile_stdin(code.s) == FAILURE) {
		crex_exception_error(EG(exception), E_ERROR);
		crex_bailout();
	}

	return SUCCESS;
} /* }}} */

int crxdbg_compile_stdin(crex_string *code) {
	CRXDBG_G(ops) = crex_compile_string(code, "Standard input code", CREX_COMPILE_POSITION_AFTER_OPEN_TAG);
	crex_string_release(code);

	if (EG(exception)) {
		return FAILURE;
	}

	if (CRXDBG_G(exec)) {
		free(CRXDBG_G(exec));
	}
	CRXDBG_G(exec) = strdup("Standard input code");
	CRXDBG_G(exec_len) = sizeof("Standard input code") - 1;
	{ /* remove leading ?> from source */
		int i;
		/* remove trailing data after zero byte, used for avoiding conflicts in eval()'ed code snippets */
		crex_string *source_path = strpprintf(0, "Standard input code%c%p", 0, CRXDBG_G(ops)->opcodes);
		crxdbg_file_source *data = crex_hash_find_ptr(&CRXDBG_G(file_sources), source_path);
		dtor_func_t dtor = CRXDBG_G(file_sources).pDestructor;
		CRXDBG_G(file_sources).pDestructor = NULL;
		crex_hash_del(&CRXDBG_G(file_sources), source_path);
		CRXDBG_G(file_sources).pDestructor = dtor;
		crex_hash_str_update_ptr(&CRXDBG_G(file_sources), "Standard input code", sizeof("Standard input code")-1, data);
		crex_string_release(source_path);

		for (i = 1; i <= data->lines; i++) {
			data->line[i] -= 2;
		}
		data->len -= 2;
		memmove(data->buf, data->buf + 2, data->len);
	}

	crxdbg_notice("Successful compilation of stdin input");

	return SUCCESS;
}

int crxdbg_compile(void) /* {{{ */
{
	crex_file_handle fh;
	char *buf;
	size_t len;

	if (!CRXDBG_G(exec)) {
		crxdbg_error("No execution context");
		return FAILURE;
	}

	crex_stream_init_filename(&fh, CRXDBG_G(exec));
	if (crx_stream_open_for_crex_ex(&fh, USE_PATH|STREAM_OPEN_FOR_INCLUDE) == SUCCESS && crex_stream_fixup(&fh, &buf, &len) == SUCCESS) {
		CG(skip_shebang) = 1;
		CRXDBG_G(ops) = crex_compile_file(&fh, CREX_INCLUDE);
		crex_destroy_file_handle(&fh);
		if (EG(exception)) {
			crex_exception_error(EG(exception), E_ERROR);
			crex_bailout();
		}

		crxdbg_notice("Successful compilation of %s", CRXDBG_G(exec));

		return SUCCESS;
	} else {
		crxdbg_error("Could not open file %s", CRXDBG_G(exec));
	}
	crex_destroy_file_handle(&fh);
	return FAILURE;
} /* }}} */

CRXDBG_COMMAND(step) /* {{{ */
{
	if (CRXDBG_G(in_execution)) {
		CRXDBG_G(flags) |= CRXDBG_IS_STEPPING;
	}

	return CRXDBG_NEXT;
} /* }}} */

CRXDBG_COMMAND(continue) /* {{{ */
{
	return CRXDBG_NEXT;
} /* }}} */

int crxdbg_skip_line_helper(void) /* {{{ */ {
	crex_execute_data *ex = crxdbg_user_execute_data(EG(current_execute_data));
	const crex_op_array *op_array = &ex->func->op_array;
	const crex_op *opline = op_array->opcodes;

	CRXDBG_G(flags) |= CRXDBG_IN_UNTIL;
	CRXDBG_G(seek_ex) = ex;
	do {
		if (opline->lineno != ex->opline->lineno
		 || opline->opcode == CREX_RETURN
		 || opline->opcode == CREX_FAST_RET
		 || opline->opcode == CREX_GENERATOR_RETURN
		 || opline->opcode == CREX_EXIT
		 || opline->opcode == CREX_YIELD
		 || opline->opcode == CREX_YIELD_FROM
		) {
			crex_hash_index_update_ptr(&CRXDBG_G(seek), (crex_ulong) opline, (void *) opline);
		}
	} while (++opline < op_array->opcodes + op_array->last);

	return CRXDBG_UNTIL;
}
/* }}} */

CRXDBG_COMMAND(until) /* {{{ */
{
	if (!CRXDBG_G(in_execution)) {
		crxdbg_error("Not executing");
		return SUCCESS;
	}

	return crxdbg_skip_line_helper();
} /* }}} */

CRXDBG_COMMAND(next) /* {{{ */
{
	if (!CRXDBG_G(in_execution)) {
		crxdbg_error("Not executing");
		return SUCCESS;
	}

	CRXDBG_G(flags) |= CRXDBG_IS_STEPPING;
	return crxdbg_skip_line_helper();
} /* }}} */

static void crxdbg_seek_to_end(void) /* {{{ */ {
	crex_execute_data *ex = crxdbg_user_execute_data(EG(current_execute_data));
	const crex_op_array *op_array = &ex->func->op_array;
	const crex_op *opline = op_array->opcodes;

	CRXDBG_G(seek_ex) = ex;
	do {
		switch (opline->opcode) {
			case CREX_RETURN:
			case CREX_FAST_RET:
			case CREX_GENERATOR_RETURN:
			case CREX_EXIT:
			case CREX_YIELD:
			case CREX_YIELD_FROM:
				crex_hash_index_update_ptr(&CRXDBG_G(seek), (crex_ulong) opline, (void *) opline);
		}
	} while (++opline < op_array->opcodes + op_array->last);
}
/* }}} */

CRXDBG_COMMAND(finish) /* {{{ */
{
	if (!CRXDBG_G(in_execution)) {
		crxdbg_error("Not executing");
		return SUCCESS;
	}

	crxdbg_seek_to_end();
	if (crex_hash_index_exists(&CRXDBG_G(seek), (crex_ulong) crxdbg_user_execute_data(EG(current_execute_data))->opline)) {
		crex_hash_clean(&CRXDBG_G(seek));
	} else {
		CRXDBG_G(flags) |= CRXDBG_IN_FINISH;
	}

	return CRXDBG_FINISH;
} /* }}} */

CRXDBG_COMMAND(leave) /* {{{ */
{
	if (!CRXDBG_G(in_execution)) {
		crxdbg_error("Not executing");
		return SUCCESS;
	}

	crxdbg_seek_to_end();
	if (crex_hash_index_exists(&CRXDBG_G(seek), (crex_ulong) crxdbg_user_execute_data(EG(current_execute_data))->opline)) {
		crex_hash_clean(&CRXDBG_G(seek));
		crxdbg_notice("Already at the end of the function");
		return SUCCESS;
	} else {
		CRXDBG_G(flags) |= CRXDBG_IN_LEAVE;
		return CRXDBG_LEAVE;
	}
} /* }}} */

CRXDBG_COMMAND(frame) /* {{{ */
{
	if (!param) {
		crxdbg_notice("Currently in frame #%d", CRXDBG_G(frame).num);
	} else {
		crxdbg_switch_frame(param->num);
	}

	return SUCCESS;
} /* }}} */

static inline void crxdbg_handle_exception(void) /* {{{ */
{
	crex_object *ex = EG(exception);
	crex_string *msg, *file;
	crex_long line;
	zval rv, tmp;

	EG(exception) = NULL;

	crex_call_known_instance_method_with_0_params(ex->ce->__tostring, ex, &tmp);
	file = zval_get_string(crex_read_property_ex(crex_get_exception_base(ex), ex, ZSTR_KNOWN(CREX_STR_FILE), /* silent */ true, &rv));
	line = zval_get_long(crex_read_property_ex(crex_get_exception_base(ex), ex, ZSTR_KNOWN(CREX_STR_LINE), /* silent */ true, &rv));

	if (EG(exception)) {
		EG(exception) = NULL;
		msg = ZSTR_EMPTY_ALLOC();
	} else {
		crex_update_property_string(crex_get_exception_base(ex), ex, CREX_STRL("string"), C_STRVAL(tmp));
		zval_ptr_dtor(&tmp);
		msg = zval_get_string(crex_read_property_ex(crex_get_exception_base(ex), ex, ZSTR_KNOWN(CREX_STR_STRING), /* silent */ true, &rv));
	}

	crxdbg_error("Uncaught %s in %s on line " CREX_LONG_FMT, ZSTR_VAL(ex->ce->name), ZSTR_VAL(file), line);
	crex_string_release(file);
	crxdbg_writeln("%s", ZSTR_VAL(msg));
	crex_string_release(msg);

	if (EG(prev_exception)) {
		OBJ_RELEASE(EG(prev_exception));
		EG(prev_exception) = 0;
	}
	OBJ_RELEASE(ex);
	EG(opline_before_exception) = NULL;

	EG(exit_status) = 255;
} /* }}} */

CRXDBG_COMMAND(run) /* {{{ */
{
	if (CRXDBG_G(ops) || CRXDBG_G(exec)) {
		crex_execute_data *ex = EG(current_execute_data);
		bool restore = 1;

		if (CRXDBG_G(in_execution)) {
			if (crxdbg_ask_user_permission("Do you really want to restart execution?") == SUCCESS) {
				crxdbg_startup_run++;
				crxdbg_clean(1, 1);
			}
			return SUCCESS;
		}

		if (!CRXDBG_G(ops)) {
			if (crxdbg_compile() == FAILURE) {
				crxdbg_error("Failed to compile %s, cannot run", CRXDBG_G(exec));
				EG(exit_status) = FAILURE;
				goto out;
			}
		}

		if (param && param->type != EMPTY_PARAM && param->len != 0) {
			char **argv = emalloc(5 * sizeof(char *));
			char *end = param->str + param->len, *p = param->str;
			char last_byte;
			int argc = 0;
			int i;

			while (*end == '\r' || *end == '\n') *(end--) = 0;
			last_byte = end[1];
			end[1] = 0;

			while (*p == ' ') p++;
			while (*p) {
				char sep = ' ';
				char *buf = emalloc(end - p + 2), *q = buf;

				if (*p == '<') {
					/* use as STDIN */
					do p++; while (*p == ' ');

					if (*p == '\'' || *p == '"') {
						sep = *(p++);
					}
					while (*p && *p != sep) {
						if (*p == '\\' && (p[1] == sep || p[1] == '\\')) {
							p++;
						}
						*(q++) = *(p++);
					}
					*(q++) = 0;
					if (*p) {
						do p++; while (*p == ' ');
					}

					if (*p) {
						crxdbg_error("Invalid run command, cannot put further arguments after stdin");
						goto free_cmd;
					}

					CRXDBG_G(stdin_file) = fopen(buf, "r");
					if (CRXDBG_G(stdin_file) == NULL) {
						crxdbg_error("Could not open '%s' for reading from stdin", buf);
						goto free_cmd;
					}
					efree(buf);
					crxdbg_register_file_handles();
					break;
				}

				if (argc >= 4 && argc == (argc & -argc)) {
					argv = erealloc(argv, (argc * 2 + 1) * sizeof(char *));
				}

				if (*p == '\'' || *p == '"') {
					sep = *(p++);
				}
				if (*p == '\\' && (p[1] == '<' || p[1] == '\'' || p[1] == '"')) {
					p++;
				}
				while (*p && *p != sep) {
					if (*p == '\\' && (p[1] == sep || p[1] == '\\' || (p[1] == '#' && sep == ' '))) {
						p++;
					}
					*(q++) = *(p++);
				}
				if (!*p && sep != ' ') {
					crxdbg_error("Invalid run command, unterminated escape sequence");
free_cmd:
					efree(buf);
					for (i = 0; i < argc; i++) {
						efree(argv[i]);
					}
					efree(argv);
					end[1] = last_byte;
					return SUCCESS;
				}

				*(q++) = 0;
				argv[++argc] = erealloc(buf, q - buf);

				if (*p) {
					do p++; while (*p == ' ');
				}
			}
			end[1] = last_byte;

			argv[0] = SG(request_info).argv[0];
			for (i = SG(request_info).argc; --i;) {
				efree(SG(request_info).argv[i]);
			}
			efree(SG(request_info).argv);
			SG(request_info).argv = erealloc(argv, ++argc * sizeof(char *));
			SG(request_info).argc = argc;

			crx_build_argv(NULL, &PG(http_globals)[TRACK_VARS_SERVER]);
		}

		/* clean up from last execution */
		if (ex && (CREX_CALL_INFO(ex) & CREX_CALL_HAS_SYMBOL_TABLE)) {
			crex_hash_clean(ex->symbol_table);
		} else {
			crex_rebuild_symbol_table();
		}
		CRXDBG_G(handled_exception) = NULL;

		/* clean seek state */
		CRXDBG_G(flags) &= ~CRXDBG_SEEK_MASK;
		crex_hash_clean(&CRXDBG_G(seek));

		/* reset hit counters */
		crxdbg_reset_breakpoints();

		crex_try {
			CRXDBG_G(flags) ^= CRXDBG_IS_INTERACTIVE;
			CRXDBG_G(flags) |= CRXDBG_IS_RUNNING;
			crex_execute(CRXDBG_G(ops), &CRXDBG_G(retval));
			CRXDBG_G(flags) ^= CRXDBG_IS_INTERACTIVE;
		} crex_catch {
			CRXDBG_G(in_execution) = 0;

			if (!(CRXDBG_G(flags) & CRXDBG_IS_STOPPING)) {
				restore = 0;
			} else {
				crex_bailout();
			}
		} crex_end_try();

		if (restore) {
			crex_exception_restore();
			crex_try {
				crex_try_exception_handler();
				CRXDBG_G(in_execution) = 1;
			} crex_catch {
				CRXDBG_G(in_execution) = 0;

				if (CRXDBG_G(flags) & CRXDBG_IS_STOPPING) {
					crex_bailout();
				}
			} crex_end_try();

			if (EG(exception)) {
				crxdbg_handle_exception();
			}
		}

		CRXDBG_G(flags) &= ~CRXDBG_IS_RUNNING;

		crxdbg_clean(1, 0);
	} else {
		crxdbg_error("Nothing to execute!");
	}

out:
	CRXDBG_FRAME(num) = 0;
	return SUCCESS;
} /* }}} */

int crxdbg_output_ev_variable(char *name, size_t len, char *keyname, size_t keylen, HashTable *parent, zval *zv) /* {{{ */ {
	crxdbg_notice("Printing variable %.*s", (int) len, name);

	crex_print_zval_r(zv, 0);

	crxdbg_out("\n");

	efree(name);
	efree(keyname);

	return SUCCESS;
}
/* }}} */

CRXDBG_COMMAND(ev) /* {{{ */
{
	bool stepping = ((CRXDBG_G(flags) & CRXDBG_IS_STEPPING) == CRXDBG_IS_STEPPING);
	zval retval;

	crex_execute_data *original_execute_data = EG(current_execute_data);
	crex_vm_stack original_stack = EG(vm_stack);
	crex_object *ex = NULL;

	CRXDBG_OUTPUT_BACKUP();

	original_stack->top = EG(vm_stack_top);

	if (CRXDBG_G(flags) & CRXDBG_IN_SIGNAL_HANDLER) {
		crxdbg_try_access {
			crxdbg_parse_variable(param->str, param->len, &EG(symbol_table), 0, crxdbg_output_ev_variable, 0);
		} crxdbg_catch_access {
			crxdbg_error("Could not fetch data, invalid data source");
		} crxdbg_end_try_access();

		CRXDBG_OUTPUT_BACKUP_RESTORE();
		return SUCCESS;
	}

	if (!(CRXDBG_G(flags) & CRXDBG_IS_STEPONEVAL)) {
		CRXDBG_G(flags) &= ~CRXDBG_IS_STEPPING;
	}

	/* disable stepping while eval() in progress */
	CRXDBG_G(flags) |= CRXDBG_IN_EVAL;
	crex_try {
		if (crex_eval_stringl(param->str, param->len, &retval, "eval()'d code") == SUCCESS) {
			if (EG(exception)) {
				ex = EG(exception);
				crex_exception_error(EG(exception), E_ERROR);
			} else {
				crex_print_zval_r(&retval, 0);
				crxdbg_out("\n");
				zval_ptr_dtor(&retval);
			}
		}
	} crex_catch {
		CRXDBG_G(unclean_eval) = 1;
		if (ex) {
			OBJ_RELEASE(ex);
		}
		EG(current_execute_data) = original_execute_data;
		EG(vm_stack_top) = original_stack->top;
		EG(vm_stack_end) = original_stack->end;
		EG(vm_stack) = original_stack;
		EG(exit_status) = 0;
	} crex_end_try();

	CRXDBG_G(flags) &= ~CRXDBG_IN_EVAL;

	/* switch stepping back on */
	if (stepping && !(CRXDBG_G(flags) & CRXDBG_IS_STEPONEVAL)) {
		CRXDBG_G(flags) |= CRXDBG_IS_STEPPING;
	}

	CG(unclean_shutdown) = 0;

	CRXDBG_OUTPUT_BACKUP_RESTORE();

	return SUCCESS;
} /* }}} */

CRXDBG_COMMAND(back) /* {{{ */
{
	if (!CRXDBG_G(in_execution)) {
		crxdbg_error("Not executing!");
		return SUCCESS;
	}

	if (!param) {
		crxdbg_dump_backtrace(0);
	} else {
		crxdbg_dump_backtrace(param->num);
	}

	return SUCCESS;
} /* }}} */

CRXDBG_COMMAND(generator) /* {{{ */
{
	int i;

	if (!CRXDBG_G(in_execution)) {
		crxdbg_error("Not executing!");
		return SUCCESS;
	}

	if (param) {
		i = param->num;
		crex_object **obj = EG(objects_store).object_buckets + i;
		if (i < EG(objects_store).top && *obj && IS_OBJ_VALID(*obj) && (*obj)->ce == crex_ce_generator) {
			crex_generator *gen = (crex_generator *) *obj;
			if (gen->execute_data) {
				if (crex_generator_get_current(gen)->flags & CREX_GENERATOR_CURRENTLY_RUNNING) {
					crxdbg_error("Generator currently running");
				} else {
					crxdbg_open_generator_frame(gen);
				}
			} else {
				crxdbg_error("Generator already closed");
			}
		} else {
			crxdbg_error("Invalid object handle");
		}
	} else {
		for (i = 0; i < EG(objects_store).top; i++) {
			crex_object *obj = EG(objects_store).object_buckets[i];
			if (obj && IS_OBJ_VALID(obj) && obj->ce == crex_ce_generator) {
				crex_generator *gen = (crex_generator *) obj, *current = crex_generator_get_current(gen);
				if (gen->execute_data) {
					crex_string *s = crxdbg_compile_stackframe(gen->execute_data);
					crxdbg_out("#%d: %.*s", i, (int) ZSTR_LEN(s), ZSTR_VAL(s));
					crex_string_release(s);
					if (gen != current) {
						if (gen->node.parent != current) {
							crxdbg_out(" with direct parent #%d and", gen->node.parent->std.handle);
						}
						crxdbg_out(" executing #%d currently", current->std.handle);
					}
					crxdbg_out("\n");
				}
			}
		}
	}

	return SUCCESS;
} /* }}} */

CRXDBG_COMMAND(print) /* {{{ */
{
	if (!param || param->type == EMPTY_PARAM) {
		return crxdbg_do_print_stack(param);
	} else switch (param->type) {
		case STR_PARAM:
			return crxdbg_do_print_func(param);
		case METHOD_PARAM:
			return crxdbg_do_print_method(param);
		default:
			crxdbg_error("Invalid arguments to print, expected nothing, function name or method name");
			return SUCCESS;
	}
} /* }}} */

CRXDBG_COMMAND(info) /* {{{ */
{
	crxdbg_out("Execution Context Information\n\n");
#ifdef HAVE_CRXDBG_READLINE
# ifdef HAVE_LIBREADLINE
	 crxdbg_writeln( "Readline   yes");
# else
	 crxdbg_writeln("Readline   no");
# endif
# ifdef HAVE_LIBEDIT
	 crxdbg_writeln("Libedit    yes");
# else
	 crxdbg_writeln("Libedit    no");
# endif
#else
	crxdbg_writeln("Readline   unavailable");
#endif

	crxdbg_writeln("Exec       %s", CRXDBG_G(exec) ? CRXDBG_G(exec) : "none");
	crxdbg_writeln("Compiled   %s", CRXDBG_G(ops) ? "yes" : "no");
	crxdbg_writeln("Stepping   %s", (CRXDBG_G(flags) & CRXDBG_IS_STEPPING) ? "on" : "off");
	crxdbg_writeln("Quietness  %s", (CRXDBG_G(flags) & CRXDBG_IS_QUIET) ? "on" : "off");

	if (CRXDBG_G(ops)) {
		crxdbg_writeln("Opcodes    %d", CRXDBG_G(ops)->last);
		crxdbg_writeln("Variables  %d", CRXDBG_G(ops)->last_var ? CRXDBG_G(ops)->last_var - 1 : 0);
	}

	crxdbg_writeln("Executing  %s", CRXDBG_G(in_execution) ? "yes" : "no");
	if (CRXDBG_G(in_execution)) {
		crxdbg_writeln("VM Return  %d", CRXDBG_G(vmret));
	}

	crxdbg_writeln("Classes    %d", crex_hash_num_elements(EG(class_table)));
	crxdbg_writeln("Functions  %d", crex_hash_num_elements(EG(function_table)));
	crxdbg_writeln("Constants  %d", crex_hash_num_elements(EG(crex_constants)));
	crxdbg_writeln("Included   %d", crex_hash_num_elements(&EG(included_files)));

	return SUCCESS;
} /* }}} */

CRXDBG_COMMAND(set) /* {{{ */
{
	crxdbg_error("No set command selected!");

	return SUCCESS;
} /* }}} */

CRXDBG_COMMAND(break) /* {{{ */
{
	if (!param) {
		if (CRXDBG_G(exec)) {
			crxdbg_set_breakpoint_file(
				crex_get_executed_filename(),
				strlen(crex_get_executed_filename()),
				crex_get_executed_lineno());
		} else {
			crxdbg_error("Execution context not set!");
		}
	} else switch (param->type) {
		case ADDR_PARAM:
			crxdbg_set_breakpoint_opline(param->addr);
			break;
		case NUMERIC_PARAM:
			if (CRXDBG_G(exec)) {
				crxdbg_set_breakpoint_file(crxdbg_current_file(), strlen(crxdbg_current_file()), param->num);
			} else {
				crxdbg_error("Execution context not set!");
			}
			break;
		case METHOD_PARAM:
			crxdbg_set_breakpoint_method(param->method.class, param->method.name);
			break;
		case NUMERIC_METHOD_PARAM:
			crxdbg_set_breakpoint_method_opline(param->method.class, param->method.name, param->num);
			break;
		case NUMERIC_FUNCTION_PARAM:
			crxdbg_set_breakpoint_function_opline(param->str, param->num);
			break;
		case FILE_PARAM:
			crxdbg_set_breakpoint_file(param->file.name, 0, param->file.line);
			break;
		case NUMERIC_FILE_PARAM:
			crxdbg_set_breakpoint_file_opline(param->file.name, param->file.line);
			break;
		case COND_PARAM:
			crxdbg_set_breakpoint_expression(param->str, param->len);
			break;
		case STR_PARAM:
			crxdbg_set_breakpoint_symbol(param->str, param->len);
			break;
		case OP_PARAM:
			crxdbg_set_breakpoint_opcode(param->str, param->len);
			break;

		crxdbg_default_switch_case();
	}

	return SUCCESS;
} /* }}} */

CRXDBG_COMMAND(sh) /* {{{ */
{
	FILE *fd = NULL;
	if ((fd=VCWD_POPEN((char*)param->str, "w"))) {
		/* TODO: do something perhaps ?? do we want input ?? */
		pclose(fd);
	} else {
		crxdbg_error("Failed to execute %s", param->str);
	}

	return SUCCESS;
} /* }}} */

static int add_module_info(crex_module_entry *module) /* {{{ */ {
	crxdbg_write("%s\n", module->name);
	return 0;
}
/* }}} */

static void add_crexext_info(crex_extension *ext) /* {{{ */ {
	crxdbg_write("%s\n", ext->name);
}
/* }}} */

#ifdef HAVE_LIBDL
CRXDBG_API const char *crxdbg_load_module_or_extension(char **path, const char **name) /* {{{ */ {
	DL_HANDLE handle;
	char *extension_dir;

	extension_dir = INI_STR("extension_dir");

	if (strchr(*path, '/') != NULL || strchr(*path, DEFAULT_SLASH) != NULL) {
		/* path is fine */
	} else if (extension_dir && extension_dir[0]) {
		char *libpath;
		int extension_dir_len = strlen(extension_dir);
		if (IS_SLASH(extension_dir[extension_dir_len-1])) {
			spprintf(&libpath, 0, "%s%s", extension_dir, *path); /* SAFE */
		} else {
			spprintf(&libpath, 0, "%s%c%s", extension_dir, DEFAULT_SLASH, *path); /* SAFE */
		}
		efree(*path);
		*path = libpath;
	} else {
		crxdbg_error("Not a full path given or extension_dir ini setting is not set");

		return NULL;
	}

	handle = DL_LOAD(*path);

	if (!handle) {
#ifdef CRX_WIN32
		char *err = GET_DL_ERROR();
		if (err && err[0]) {
			crxdbg_error("%s", err);
			crx_win32_error_msg_free(err);
		} else {
			crxdbg_error("Unknown reason");
		}
#else
		crxdbg_error("%s", GET_DL_ERROR());
#endif
		return NULL;
	}

#if CREX_EXTENSIONS_SUPPORT
	do {
		crex_extension *new_extension;

		const crex_extension_version_info *extension_version_info = (const crex_extension_version_info *) DL_FETCH_SYMBOL(handle, "extension_version_info");
		if (!extension_version_info) {
			extension_version_info = (const crex_extension_version_info *) DL_FETCH_SYMBOL(handle, "_extension_version_info");
		}
		new_extension = (crex_extension *) DL_FETCH_SYMBOL(handle, "crex_extension_entry");
		if (!new_extension) {
			new_extension = (crex_extension *) DL_FETCH_SYMBOL(handle, "_crex_extension_entry");
		}
		if (!extension_version_info || !new_extension) {
			break;
		}
		if (extension_version_info->crex_extension_api_no != CREX_EXTENSION_API_NO &&(!new_extension->api_no_check || new_extension->api_no_check(CREX_EXTENSION_API_NO) != SUCCESS)) {
			crxdbg_error("%s requires Crex Engine API version %d, which does not match the installed Crex Engine API version %d", new_extension->name, extension_version_info->crex_extension_api_no, CREX_EXTENSION_API_NO);

			goto quit;
		} else if (strcmp(CREX_EXTENSION_BUILD_ID, extension_version_info->build_id) && (!new_extension->build_id_check || new_extension->build_id_check(CREX_EXTENSION_BUILD_ID) != SUCCESS)) {
			crxdbg_error("%s was built with configuration %s, whereas running engine is %s", new_extension->name, extension_version_info->build_id, CREX_EXTENSION_BUILD_ID);

			goto quit;
		}

		*name = new_extension->name;

		crex_register_extension(new_extension, handle);

		if (new_extension->startup) {
			if (new_extension->startup(new_extension) != SUCCESS) {
				crxdbg_error("Unable to startup Crex extension %s", new_extension->name);

				goto quit;
			}
			crex_append_version_info(new_extension);
		}

		return "Crex extension";
	} while (0);
#endif

	do {
		crex_module_entry *module_entry;
		crex_module_entry *(*get_module)(void);

		get_module = (crex_module_entry *(*)(void)) DL_FETCH_SYMBOL(handle, "get_module");
		if (!get_module) {
			get_module = (crex_module_entry *(*)(void)) DL_FETCH_SYMBOL(handle, "_get_module");
		}

		if (!get_module) {
			break;
		}

		module_entry = get_module();
		*name = module_entry->name;

		if (strcmp(CREX_EXTENSION_BUILD_ID, module_entry->build_id)) {
			crxdbg_error("%s was built with configuration %s, whereas running engine is %s", module_entry->name, module_entry->build_id, CREX_EXTENSION_BUILD_ID);

			goto quit;
		}

		module_entry->type = MODULE_PERSISTENT;
		module_entry->module_number = crex_next_free_module();
		module_entry->handle = handle;

		if ((module_entry = crex_register_module_ex(module_entry)) == NULL) {
			crxdbg_error("Unable to register module %s", *name);

			goto quit;
		}

		if (crex_startup_module_ex(module_entry) == FAILURE) {
			crxdbg_error("Unable to startup module %s", module_entry->name);

			goto quit;
		}

		if (module_entry->request_startup_func) {
			if (module_entry->request_startup_func(MODULE_PERSISTENT, module_entry->module_number) == FAILURE) {
				crxdbg_error("Unable to initialize module %s", module_entry->name);

				goto quit;
			}
		}

		return "module";
	} while (0);

	crxdbg_error("This shared object is nor a Crex extension nor a module");

quit:
	DL_UNLOAD(handle);
	return NULL;
}
/* }}} */
#endif

CRXDBG_COMMAND(dl) /* {{{ */
{
	const char *type, *name;
	char *path;

	if (!param || param->type == EMPTY_PARAM) {
		crxdbg_notice("Crex extensions");
		crex_llist_apply(&crex_extensions, (llist_apply_func_t) add_crexext_info);
		crxdbg_out("\n");
		crxdbg_notice("Modules");
		crex_hash_apply(&module_registry, (apply_func_t) add_module_info);
	} else switch (param->type) {
		case STR_PARAM:
#ifdef HAVE_LIBDL
			path = estrndup(param->str, param->len);

			crxdbg_activate_err_buf(1);
			if ((type = crxdbg_load_module_or_extension(&path, &name)) == NULL) {
				crxdbg_error("Could not load %s, not found or invalid crex extension / module: %s", path, CRXDBG_G(err_buf).msg);
			} else {
				crxdbg_notice("Successfully loaded the %s %s at path %s", type, name, path);
			}
			crxdbg_activate_err_buf(0);
			crxdbg_free_err_buf();
			efree(path);
#else
			crxdbg_error("Cannot dynamically load %.*s - dynamic modules are not supported", (int) param->len, param->str);
#endif
			break;

		crxdbg_default_switch_case();
	}

	return SUCCESS;
} /* }}} */

CRXDBG_COMMAND(source) /* {{{ */
{
	crex_stat_t sb = {0};

	if (VCWD_STAT(param->str, &sb) != -1) {
		crxdbg_try_file_init(param->str, param->len, 0);
	} else {
		crxdbg_error("Failed to stat %s, file does not exist", param->str);
	}

	return SUCCESS;
} /* }}} */

CRXDBG_COMMAND(export) /* {{{ */
{
	FILE *handle = VCWD_FOPEN(param->str, "w+");

	if (handle) {
		crxdbg_export_breakpoints(handle);
		fclose(handle);
	} else {
		crxdbg_error("Failed to open or create %s, check path and permissions", param->str);
	}

	return SUCCESS;
} /* }}} */

CRXDBG_COMMAND(register) /* {{{ */
{
	crex_function *function;
	char *lcname = crex_str_tolower_dup(param->str, param->len);
	size_t lcname_len = strlen(lcname);

	if (!crex_hash_str_exists(&CRXDBG_G(registered), lcname, lcname_len)) {
		if ((function = crex_hash_str_find_ptr(EG(function_table), lcname, lcname_len))) {
			crex_hash_str_update_ptr(&CRXDBG_G(registered), lcname, lcname_len, function);
			function_add_ref(function);

			crxdbg_notice("Registered %s", lcname);
		} else {
			crxdbg_error("The requested function (%s) could not be found", param->str);
		}
	} else {
		crxdbg_error("The requested name (%s) is already in use", lcname);
	}

	efree(lcname);
	return SUCCESS;
} /* }}} */

CRXDBG_COMMAND(quit) /* {{{ */
{
	CRXDBG_G(flags) |= CRXDBG_IS_QUITTING;
	CRXDBG_G(flags) &= ~CRXDBG_IS_CLEANING;

	return SUCCESS;
} /* }}} */

CRXDBG_COMMAND(clean) /* {{{ */
{
	if (CRXDBG_G(in_execution)) {
		if (crxdbg_ask_user_permission("Do you really want to clean your current environment?") == FAILURE) {
			return SUCCESS;
		}
	}

	crxdbg_out("Cleaning Execution Environment\n");

	crxdbg_writeln("Classes    %d", crex_hash_num_elements(EG(class_table)));
	crxdbg_writeln("Functions  %d", crex_hash_num_elements(EG(function_table)));
	crxdbg_writeln("Constants  %d", crex_hash_num_elements(EG(crex_constants)));
	crxdbg_writeln("Includes   %d", crex_hash_num_elements(&EG(included_files)));

	crxdbg_clean(1, 0);

	return SUCCESS;
} /* }}} */

CRXDBG_COMMAND(clear) /* {{{ */
{
	crxdbg_out("Clearing Breakpoints\n");

	crxdbg_writeln("File              %d", crex_hash_num_elements(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE]));
	crxdbg_writeln("Functions         %d", crex_hash_num_elements(&CRXDBG_G(bp)[CRXDBG_BREAK_SYM]));
	crxdbg_writeln("Methods           %d", crex_hash_num_elements(&CRXDBG_G(bp)[CRXDBG_BREAK_METHOD]));
	crxdbg_writeln("Oplines           %d", crex_hash_num_elements(&CRXDBG_G(bp)[CRXDBG_BREAK_OPLINE]));
	crxdbg_writeln("File oplines      %d", crex_hash_num_elements(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE_OPLINE]));
	crxdbg_writeln("Function oplines  %d", crex_hash_num_elements(&CRXDBG_G(bp)[CRXDBG_BREAK_FUNCTION_OPLINE]));
	crxdbg_writeln("Method oplines    %d", crex_hash_num_elements(&CRXDBG_G(bp)[CRXDBG_BREAK_METHOD_OPLINE]));
	crxdbg_writeln("Conditionals      %d", crex_hash_num_elements(&CRXDBG_G(bp)[CRXDBG_BREAK_COND]));

	crxdbg_clear_breakpoints();

	return SUCCESS;
} /* }}} */

CRXDBG_COMMAND(list) /* {{{ */
{
	if (!param) {
		return CRXDBG_LIST_HANDLER(lines)(CRXDBG_COMMAND_ARGS);
	} else switch (param->type) {
		case NUMERIC_PARAM:
			return CRXDBG_LIST_HANDLER(lines)(CRXDBG_COMMAND_ARGS);

		case FILE_PARAM:
			return CRXDBG_LIST_HANDLER(lines)(CRXDBG_COMMAND_ARGS);

		case STR_PARAM:
			crxdbg_list_function_byname(param->str, param->len);
			break;

		case METHOD_PARAM:
			return CRXDBG_LIST_HANDLER(method)(CRXDBG_COMMAND_ARGS);

		crxdbg_default_switch_case();
	}

	return SUCCESS;
} /* }}} */

CRXDBG_COMMAND(watch) /* {{{ */
{
	if (!param || param->type == EMPTY_PARAM) {
		crxdbg_list_watchpoints();
	} else switch (param->type) {
		case STR_PARAM:
			crxdbg_create_var_watchpoint(param->str, param->len);
			break;

		crxdbg_default_switch_case();
	}

	return SUCCESS;
} /* }}} */

int crxdbg_interactive(bool allow_async_unsafe, char *input) /* {{{ */
{
	int ret = SUCCESS;
	crxdbg_param_t stack;

	CRXDBG_G(flags) |= CRXDBG_IS_INTERACTIVE;

	while (ret == SUCCESS || ret == FAILURE) {
		if (CRXDBG_G(flags) & CRXDBG_IS_STOPPING) {
			crex_bailout();
		}

		if (!input && !(input = crxdbg_read_input(NULL))) {
			break;
		}


		crxdbg_init_param(&stack, STACK_PARAM);

		if (crxdbg_do_parse(&stack, input) <= 0) {
			crxdbg_activate_err_buf(1);

			crex_try {
				ret = crxdbg_stack_execute(&stack, allow_async_unsafe);
			} crex_catch {
				crxdbg_stack_free(&stack);
				crex_bailout();
			} crex_end_try();

			switch (ret) {
				case FAILURE:
					if (!(CRXDBG_G(flags) & CRXDBG_IS_STOPPING)) {
						if (!allow_async_unsafe || crxdbg_call_register(&stack) == FAILURE) {
							if (CRXDBG_G(err_buf).active) {
							    crxdbg_output_err_buf("%s", CRXDBG_G(err_buf).msg);
							}
						}
					}
				break;

				case CRXDBG_LEAVE:
				case CRXDBG_FINISH:
				case CRXDBG_UNTIL:
				case CRXDBG_NEXT: {
					crxdbg_activate_err_buf(0);
					crxdbg_free_err_buf();
					if (!CRXDBG_G(in_execution) && !(CRXDBG_G(flags) & CRXDBG_IS_STOPPING)) {
						crxdbg_error("Not running");
					}
					break;
				}
			}

			crxdbg_activate_err_buf(0);
			crxdbg_free_err_buf();
		}

		crxdbg_stack_free(&stack);
		crxdbg_destroy_input(&input);
		CRXDBG_G(req_id) = 0;
		input = NULL;
	}

	if (input) {
		crxdbg_stack_free(&stack);
		crxdbg_destroy_input(&input);
		CRXDBG_G(req_id) = 0;
	}

	if (CRXDBG_G(in_execution)) {
		crxdbg_restore_frame();
	}

	CRXDBG_G(flags) &= ~CRXDBG_IS_INTERACTIVE;

	crxdbg_print_changed_zvals();

	return ret;
} /* }}} */

static inline void list_code(void) {
	if (!(CRXDBG_G(flags) & CRXDBG_IN_EVAL)) {
		const char *file_char = crex_get_executed_filename();
		crex_string *file = crex_string_init(file_char, strlen(file_char), 0);
		crxdbg_list_file(file, 3, crex_get_executed_lineno()-1, crex_get_executed_lineno());
		efree(file);
	}
}

/* code may behave weirdly if EG(exception) is set; thus backup it */
#define DO_INTERACTIVE(allow_async_unsafe) do { \
	if (exception) { \
		const crex_op *before_ex = EG(opline_before_exception); \
		const crex_op *backup_opline = NULL; \
		if (EG(current_execute_data) && EG(current_execute_data)->func && CREX_USER_CODE(EG(current_execute_data)->func->common.type)) { \
			backup_opline = EG(current_execute_data)->opline; \
		} \
		GC_ADDREF(exception); \
		crex_clear_exception(); \
		list_code(); \
		switch (crxdbg_interactive(allow_async_unsafe, NULL)) { \
			case CRXDBG_LEAVE: \
			case CRXDBG_FINISH: \
			case CRXDBG_UNTIL: \
			case CRXDBG_NEXT: \
				if (backup_opline \
				 && (backup_opline->opcode == CREX_HANDLE_EXCEPTION || backup_opline->opcode == CREX_CATCH)) { \
					EG(current_execute_data)->opline = backup_opline; \
					EG(exception) = exception; \
				} else { \
					crex_throw_exception_internal(exception); \
				} \
				EG(opline_before_exception) = before_ex; \
		} \
	} else { \
		list_code(); \
		crxdbg_interactive(allow_async_unsafe, NULL); \
	} \
	goto next; \
} while (0)

void crxdbg_execute_ex(crex_execute_data *execute_data) /* {{{ */
{
	bool original_in_execution = CRXDBG_G(in_execution);

	if ((CRXDBG_G(flags) & CRXDBG_IS_STOPPING) && !(CRXDBG_G(flags) & CRXDBG_IS_RUNNING)) {
		crex_bailout();
	}

	CRXDBG_G(in_execution) = 1;

	while (1) {
		crex_object *exception = EG(exception);

		if ((CRXDBG_G(flags) & CRXDBG_BP_RESOLVE_MASK)) {
			/* resolve nth opline breakpoints */
			crxdbg_resolve_op_array_breaks(&execute_data->func->op_array);
		}

#ifdef CREX_WIN32
		if (crex_atomic_bool_load_ex(&EG(timed_out))) {
			crex_timeout();
		}
#endif

		if (exception && crex_is_unwind_exit(exception)) {
			/* Restore bailout based exit. */
			crex_bailout();
		}

		if (CRXDBG_G(flags) & CRXDBG_PREVENT_INTERACTIVE) {
			crxdbg_print_opline(execute_data, 0);
			goto next;
		}

		/* check for uncaught exceptions */
		if (exception && CRXDBG_G(handled_exception) != exception && !(CRXDBG_G(flags) & CRXDBG_IN_EVAL)) {
			crex_execute_data *prev_ex = execute_data;

			do {
				prev_ex = crex_generator_check_placeholder_frame(prev_ex);
				/* assuming that no internal functions will silently swallow exceptions ... */
				if (!prev_ex->func || !CREX_USER_CODE(prev_ex->func->common.type)) {
					continue;
				}

				if (crxdbg_check_caught_ex(prev_ex, exception)) {
					goto ex_is_caught;
				}
			} while ((prev_ex = prev_ex->prev_execute_data));

			CRXDBG_G(handled_exception) = exception;

			zval rv;
			crex_string *file = zval_get_string(crex_read_property_ex(crex_get_exception_base(exception), exception, ZSTR_KNOWN(CREX_STR_FILE), /* silent */ true, &rv));
			crex_long line = zval_get_long(crex_read_property_ex(crex_get_exception_base(exception), exception, ZSTR_KNOWN(CREX_STR_LINE), /* silent */ true, &rv));
			crex_string *msg = zval_get_string(crex_read_property_ex(crex_get_exception_base(exception), exception, ZSTR_KNOWN(CREX_STR_MESSAGE), /* silent */ true, &rv));

			crxdbg_error("Uncaught %s in %s on line " CREX_LONG_FMT ": %.*s",
				ZSTR_VAL(exception->ce->name), ZSTR_VAL(file), line,
				ZSTR_LEN(msg) < 80 ? (int) ZSTR_LEN(msg) : 80, ZSTR_VAL(msg));
			crex_string_release(msg);
			crex_string_release(file);

			DO_INTERACTIVE(1);
		}
ex_is_caught:

		/* allow conditional breakpoints and initialization to access the vm uninterrupted */
		if (CRXDBG_G(flags) & (CRXDBG_IN_COND_BP | CRXDBG_IS_INITIALIZING)) {
			/* skip possible breakpoints */
			goto next;
		}

		/* not while in conditionals */
		crxdbg_print_opline(execute_data, 0);

		/* perform seek operation */
		if ((CRXDBG_G(flags) & CRXDBG_SEEK_MASK) && !(CRXDBG_G(flags) & CRXDBG_IN_EVAL)) {
			/* current address */
			crex_ulong address = (crex_ulong) execute_data->opline;

			if (CRXDBG_G(seek_ex) != execute_data) {
				if (CRXDBG_G(flags) & CRXDBG_IS_STEPPING) {
					goto stepping;
				}
				goto next;
			}

#define INDEX_EXISTS_CHECK (crex_hash_index_exists(&CRXDBG_G(seek), address) || (exception && crxdbg_check_caught_ex(execute_data, exception) == 0))

			/* run to next line */
			if (CRXDBG_G(flags) & CRXDBG_IN_UNTIL) {
				if (INDEX_EXISTS_CHECK) {
					CRXDBG_G(flags) &= ~CRXDBG_IN_UNTIL;
					crex_hash_clean(&CRXDBG_G(seek));
				} else {
					/* skip possible breakpoints */
					goto next;
				}
			}

			/* run to finish */
			if (CRXDBG_G(flags) & CRXDBG_IN_FINISH) {
				if (INDEX_EXISTS_CHECK) {
					CRXDBG_G(flags) &= ~CRXDBG_IN_FINISH;
					crex_hash_clean(&CRXDBG_G(seek));
				}
				/* skip possible breakpoints */
				goto next;
			}

			/* break for leave */
			if (CRXDBG_G(flags) & CRXDBG_IN_LEAVE) {
				if (INDEX_EXISTS_CHECK) {
					CRXDBG_G(flags) &= ~CRXDBG_IN_LEAVE;
					crex_hash_clean(&CRXDBG_G(seek));
					crxdbg_notice("Breaking for leave at %s:%u",
						crex_get_executed_filename(),
						crex_get_executed_lineno()
					);
					DO_INTERACTIVE(1);
				} else {
					/* skip possible breakpoints */
					goto next;
				}
			}
		}

		if (CRXDBG_G(flags) & CRXDBG_IS_STEPPING && (CRXDBG_G(flags) & CRXDBG_STEP_OPCODE || execute_data->opline->lineno != CRXDBG_G(last_line))) {
stepping:
			CRXDBG_G(flags) &= ~CRXDBG_IS_STEPPING;
			DO_INTERACTIVE(1);
		}

		/* check if some watchpoint was hit */
		{
			if (crxdbg_print_changed_zvals() == SUCCESS) {
				DO_INTERACTIVE(1);
			}
		}

		/* search for breakpoints */
		{
			crxdbg_breakbase_t *brake;

			if ((CRXDBG_G(flags) & CRXDBG_BP_MASK)
			    && (brake = crxdbg_find_breakpoint(execute_data))
			    && (brake->type != CRXDBG_BREAK_FILE || execute_data->opline->lineno != CRXDBG_G(last_line))) {
				crxdbg_hit_breakpoint(brake, 1);
				DO_INTERACTIVE(1);
			}
		}

		if (CRXDBG_G(flags) & CRXDBG_IS_SIGNALED) {
			CRXDBG_G(flags) &= ~CRXDBG_IS_SIGNALED;

			crxdbg_out("\n");
			crxdbg_notice("Program received signal SIGINT");
			DO_INTERACTIVE(1);
		}

next:

		CRXDBG_G(last_line) = execute_data->opline->lineno;

		/* stupid hack to make crex_do_fcall_common_helper return CREX_VM_ENTER() instead of recursively calling crex_execute() and eventually segfaulting */
		if ((execute_data->opline->opcode == CREX_DO_FCALL ||
		     execute_data->opline->opcode == CREX_DO_UCALL ||
		     execute_data->opline->opcode == CREX_DO_FCALL_BY_NAME) &&
		     execute_data->call->func->type == CREX_USER_FUNCTION) {
			crex_execute_ex = execute_ex;
		}
		CRXDBG_G(vmret) = crex_vm_call_opcode_handler(execute_data);
		crex_execute_ex = crxdbg_execute_ex;

		if (CRXDBG_G(vmret) != 0) {
			if (CRXDBG_G(vmret) < 0) {
				CRXDBG_G(in_execution) = original_in_execution;
				return;
			} else {
				execute_data = EG(current_execute_data);
			}
		}
	}
	crex_error_noreturn(E_ERROR, "Arrived at end of main loop which shouldn't happen");
} /* }}} */

/* only if *not* interactive and while executing */
void crxdbg_force_interruption(void) /* {{{ */ {
	crex_object *exception = EG(exception);
	crex_execute_data *data = EG(current_execute_data); /* should be always readable if not NULL */

	CRXDBG_G(flags) |= CRXDBG_IN_SIGNAL_HANDLER;

	if (data) {
		if (data->func) {
			if (CREX_USER_CODE(data->func->type)) {
				crxdbg_notice("Current opline: %p (op #%u) in %s:%u",
				    data->opline,
				    (uint32_t) (data->opline - data->func->op_array.opcodes),
				    data->func->op_array.filename->val,
				    data->opline->lineno);
			} else if (data->func->internal_function.function_name) {
				crxdbg_notice("Current opline: in internal function %s",
				    data->func->internal_function.function_name->val);
			} else {
				crxdbg_notice("Current opline: executing internal code");
			}
		} else {
			crxdbg_notice("Current opline: %p (op_array information unavailable)",
			    data->opline);
		}
	} else {
		crxdbg_notice("No information available about executing context");
	}

	DO_INTERACTIVE(0);

next:
	CRXDBG_G(flags) &= ~CRXDBG_IN_SIGNAL_HANDLER;

	if (CRXDBG_G(flags) & CRXDBG_IS_STOPPING) {
		crex_bailout();
	}
}
/* }}} */
