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
#include <sys/stat.h>
#ifndef _WIN32
#	include <sys/mman.h>
#	include <unistd.h>
#endif
#include <fcntl.h>
#include "crxdbg.h"
#include "crxdbg_list.h"
#include "crxdbg_utils.h"
#include "crxdbg_prompt.h"
#include "crx_streams.h"
#include "crex_exceptions.h"

CREX_EXTERN_MODULE_GLOBALS(crxdbg)

#define CRXDBG_LIST_COMMAND_D(f, h, a, m, l, s, flags) \
	CRXDBG_COMMAND_D_EXP(f, h, a, m, l, s, &crxdbg_prompt_commands[12], flags)

const crxdbg_command_t crxdbg_list_commands[] = {
	CRXDBG_LIST_COMMAND_D(lines,     "lists the specified lines",    'l', list_lines,  NULL, "l", CRXDBG_ASYNC_SAFE),
	CRXDBG_LIST_COMMAND_D(class,     "lists the specified class",    'c', list_class,  NULL, "s", CRXDBG_ASYNC_SAFE),
	CRXDBG_LIST_COMMAND_D(method,    "lists the specified method",   'm', list_method, NULL, "m", CRXDBG_ASYNC_SAFE),
	CRXDBG_LIST_COMMAND_D(func,      "lists the specified function", 'f', list_func,   NULL, "s", CRXDBG_ASYNC_SAFE),
	CRXDBG_END_COMMAND
};

CRXDBG_LIST(lines) /* {{{ */
{
	if (!CRXDBG_G(exec) && !crex_is_executing()) {
		crxdbg_error("Not executing, and execution context not set");
		return SUCCESS;
	}

	switch (param->type) {
		case NUMERIC_PARAM: {
			const char *char_file = crxdbg_current_file();
			crex_string *file = crex_string_init(char_file, strlen(char_file), 0);
			crxdbg_list_file(file, param->num < 0 ? 1 - param->num : param->num, (param->num < 0 ? param->num : 0) + crex_get_executed_lineno(), 0);
			efree(file);
		} break;

		case FILE_PARAM: {
			crex_string *file;
			char resolved_path_buf[MAXPATHLEN];
			const char *abspath = param->file.name;
			if (VCWD_REALPATH(abspath, resolved_path_buf)) {
				abspath = resolved_path_buf;
			}
			file = crex_string_init(abspath, strlen(abspath), 0);
			crxdbg_list_file(file, param->file.line, 0, 0);
			crex_string_release(file);
		} break;

		crxdbg_default_switch_case();
	}

	return SUCCESS;
} /* }}} */

CRXDBG_LIST(func) /* {{{ */
{
	crxdbg_list_function_byname(param->str, param->len);

	return SUCCESS;
} /* }}} */

CRXDBG_LIST(method) /* {{{ */
{
	crex_class_entry *ce;

	if (crxdbg_safe_class_lookup(param->method.class, strlen(param->method.class), &ce) == SUCCESS) {
		crex_function *function;
		char *lcname = crex_str_tolower_dup(param->method.name, strlen(param->method.name));

		if ((function = crex_hash_str_find_ptr(&ce->function_table, lcname, strlen(lcname)))) {
			crxdbg_list_function(function);
		} else {
			crxdbg_error("Could not find %s::%s", param->method.class, param->method.name);
		}

		efree(lcname);
	} else {
		crxdbg_error("Could not find the class %s", param->method.class);
	}

	return SUCCESS;
} /* }}} */

CRXDBG_LIST(class) /* {{{ */
{
	crex_class_entry *ce;

	if (crxdbg_safe_class_lookup(param->str, param->len, &ce) == SUCCESS) {
		if (ce->type == CREX_USER_CLASS) {
			if (ce->info.user.filename) {
				crxdbg_list_file(ce->info.user.filename, ce->info.user.line_end - ce->info.user.line_start + 1, ce->info.user.line_start, 0);
			} else {
				crxdbg_error("The source of the requested class (%s) cannot be found", ZSTR_VAL(ce->name));
			}
		} else {
			crxdbg_error("The class requested (%s) is not user defined", ZSTR_VAL(ce->name));
		}
	} else {
		crxdbg_error("The requested class (%s) could not be found", param->str);
	}

	return SUCCESS;
} /* }}} */

void crxdbg_list_file(crex_string *filename, uint32_t count, int offset, uint32_t highlight) /* {{{ */
{
	uint32_t line, lastline;
	crxdbg_file_source *data;

	if (!(data = crex_hash_find_ptr(&CRXDBG_G(file_sources), filename))) {
		crxdbg_error("Could not find information about included file...");
		return;
	}

	if (offset < 0) {
		count += offset;
		offset = 0;
	}

	lastline = offset + count;

	if (lastline > data->lines) {
		lastline = data->lines;
	}

	for (line = offset; line < lastline;) {
		uint32_t linestart = data->line[line++];
		uint32_t linelen = data->line[line] - linestart;
		char *buffer = data->buf + linestart;

		if (!highlight) {
			crxdbg_write(" %05u: %.*s", line, linelen, buffer);
		} else {
			if (highlight != line) {
				crxdbg_write(" %05u: %.*s", line, linelen, buffer);
			} else {
				crxdbg_write(">%05u: %.*s", line, linelen, buffer);
			}
		}

		if (*(buffer + linelen - 1) != '\n' || !linelen) {
			crxdbg_out("\n");
		}
	}
} /* }}} */

void crxdbg_list_function(const crex_function *fbc) /* {{{ */
{
	const crex_op_array *ops;

	if (fbc->type != CREX_USER_FUNCTION) {
		crxdbg_error("The function requested (%s) is not user defined", ZSTR_VAL(fbc->common.function_name));
		return;
	}

	ops = (crex_op_array *) fbc;

	crxdbg_list_file(ops->filename, ops->line_end - ops->line_start + 1, ops->line_start, 0);
} /* }}} */

void crxdbg_list_function_byname(const char *str, size_t len) /* {{{ */
{
	HashTable *func_table = EG(function_table);
	crex_function* fbc;
	char *func_name = (char*) str;
	size_t func_name_len = len;

	/* search active scope if begins with period */
	if (func_name[0] == '.') {
		crex_class_entry *scope = crex_get_executed_scope();
		if (scope) {
			func_name++;
			func_name_len--;

			func_table = &scope->function_table;
		} else {
			crxdbg_error("No active class");
			return;
		}
	} else if (!EG(function_table)) {
		crxdbg_error("No function table loaded");
		return;
	} else {
		func_table = EG(function_table);
	}

	/* use lowercase names, case insensitive */
	func_name = crex_str_tolower_dup(func_name, func_name_len);

	crxdbg_try_access {
		if ((fbc = crex_hash_str_find_ptr(func_table, func_name, func_name_len))) {
			crxdbg_list_function(fbc);
		} else {
			crxdbg_error("Function %s not found", func_name);
		}
	} crxdbg_catch_access {
		crxdbg_error("Could not list function %s, invalid data source", func_name);
	} crxdbg_end_try_access();

	efree(func_name);
} /* }}} */

/* Note: do not free the original file handler, let original compile_file() or caller do that. Caller may rely on its value to check success */
crex_op_array *crxdbg_compile_file(crex_file_handle *file, int type) {
	crxdbg_file_source data, *dataptr;
	crex_op_array *ret;
	uint32_t line;
	char *bufptr, *endptr;
	size_t len;

	/* Copy file contents before calling original compile_file,
	 * as it may invalidate the file handle. */
	if (crex_stream_fixup(file, &bufptr, &len) == FAILURE) {
		if (type == CREX_REQUIRE) {
			crex_message_dispatcher(ZMSG_FAILED_REQUIRE_FOPEN, ZSTR_VAL(file->filename));
		} else {
			crex_message_dispatcher(ZMSG_FAILED_INCLUDE_FOPEN, ZSTR_VAL(file->filename));
		}
		return NULL;
	}

	data.buf = estrndup(bufptr, len);
	data.len = len;

	ret = CRXDBG_G(compile_file)(file, type);
	if (ret == NULL) {
		efree(data.buf);
		return ret;
	}

	data.buf[data.len] = '\0';
	data.line[0] = 0;
	*(dataptr = emalloc(sizeof(crxdbg_file_source) + sizeof(uint32_t) * data.len)) = data;

	for (line = 0, bufptr = data.buf - 1, endptr = data.buf + data.len; ++bufptr < endptr;) {
		if (*bufptr == '\n') {
			dataptr->line[++line] = (uint32_t)(bufptr - data.buf) + 1;
		}
	}

	dataptr->lines = ++line;
	dataptr = erealloc(dataptr, sizeof(crxdbg_file_source) + sizeof(uint32_t) * line);
	dataptr->line[line] = endptr - data.buf;

	crex_hash_del(&CRXDBG_G(file_sources), ret->filename);
	crex_hash_add_ptr(&CRXDBG_G(file_sources), ret->filename, dataptr);
	crxdbg_resolve_pending_file_break(ZSTR_VAL(ret->filename));

	return ret;
}

crex_op_array *crxdbg_init_compile_file(crex_file_handle *file, int type) {
	crex_string *filename = file->opened_path ? file->opened_path : file->filename;
	char resolved_path_buf[MAXPATHLEN];
	crex_op_array *op_array;
	crxdbg_file_source *dataptr;

	if (VCWD_REALPATH(ZSTR_VAL(filename), resolved_path_buf)) {
		filename = crex_string_init(resolved_path_buf, strlen(resolved_path_buf), 0);

		if (file->opened_path) {
			crex_string_release(file->opened_path);
			file->opened_path = filename;
		} else {
			crex_string_release(file->filename);
			file->filename = filename;
		}
	}

	op_array = CRXDBG_G(init_compile_file)(file, type);

	if (op_array == NULL) {
		return NULL;
	}

	dataptr = crex_hash_find_ptr(&CRXDBG_G(file_sources), op_array->filename);
	CREX_ASSERT(dataptr != NULL);

	dataptr->op_array = *op_array;
	if (dataptr->op_array.refcount) {
		++*dataptr->op_array.refcount;
	}

	return op_array;
}

crex_op_array *crxdbg_compile_string(crex_string *source_string, const char *filename, crex_compile_position position) {
	crex_string *fake_name;
	crex_op_array *op_array;
	crxdbg_file_source *dataptr;
	uint32_t line;
	char *bufptr, *endptr;

	if (CRXDBG_G(flags) & CRXDBG_IN_EVAL) {
		return CRXDBG_G(compile_string)(source_string, filename, position);
	}

	dataptr = emalloc(sizeof(crxdbg_file_source) + sizeof(uint32_t) * ZSTR_LEN(source_string));
	dataptr->buf = estrndup(ZSTR_VAL(source_string), ZSTR_LEN(source_string));
	dataptr->len = ZSTR_LEN(source_string);
	dataptr->line[0] = 0;
	for (line = 0, bufptr = dataptr->buf - 1, endptr = dataptr->buf + dataptr->len; ++bufptr < endptr;) {
		if (*bufptr == '\n') {
			dataptr->line[++line] = (uint32_t)(bufptr - dataptr->buf) + 1;
		}
	}
	dataptr->lines = ++line;
	dataptr->line[line] = endptr - dataptr->buf;

	op_array = CRXDBG_G(compile_string)(source_string, filename, position);

	if (op_array == NULL) {
		efree(dataptr->buf);
		efree(dataptr);
		return NULL;
	}

	fake_name = strpprintf(0, "%s%c%p", filename, 0, op_array->opcodes);

	dataptr = erealloc(dataptr, sizeof(crxdbg_file_source) + sizeof(uint32_t) * line);
	crex_hash_add_ptr(&CRXDBG_G(file_sources), fake_name, dataptr);

	crex_string_release(fake_name);

	dataptr->op_array = *op_array;
	if (dataptr->op_array.refcount) {
		++*dataptr->op_array.refcount;
	}

	return op_array;
}

void crxdbg_init_list(void) {
	CRXDBG_G(compile_file) = crex_compile_file;
	CRXDBG_G(compile_string) = crex_compile_string;
	crex_compile_file = crxdbg_compile_file;
	crex_compile_string = crxdbg_compile_string;
}

void crxdbg_list_update(void) {
	CRXDBG_G(init_compile_file) = crex_compile_file;
	crex_compile_file = crxdbg_init_compile_file;
}
