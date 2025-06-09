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

#include "crx.h"
#include "crxdbg.h"
#include "crxdbg_utils.h"
#include "ext/standard/crx_string.h"

/* FASYNC under Solaris */
#ifdef HAVE_SYS_FILE_H
# include <sys/file.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
# include "sys/ioctl.h"
# ifndef GWINSC_IN_SYS_IOCTL
#  include <termios.h>
# endif
#endif

CREX_EXTERN_MODULE_GLOBALS(crxdbg)

/* {{{ color structures */
static const crxdbg_color_t colors[] = {
	CRXDBG_COLOR_D("none",             "0;0"),

	CRXDBG_COLOR_D("white",            "0;64"),
	CRXDBG_COLOR_D("white-bold",       "1;64"),
	CRXDBG_COLOR_D("white-underline",  "4;64"),
	CRXDBG_COLOR_D("red",              "0;31"),
	CRXDBG_COLOR_D("red-bold",         "1;31"),
	CRXDBG_COLOR_D("red-underline",    "4;31"),
	CRXDBG_COLOR_D("green",            "0;32"),
	CRXDBG_COLOR_D("green-bold",       "1;32"),
	CRXDBG_COLOR_D("green-underline",  "4;32"),
	CRXDBG_COLOR_D("yellow",           "0;33"),
	CRXDBG_COLOR_D("yellow-bold",      "1;33"),
	CRXDBG_COLOR_D("yellow-underline", "4;33"),
	CRXDBG_COLOR_D("blue",             "0;34"),
	CRXDBG_COLOR_D("blue-bold",        "1;34"),
	CRXDBG_COLOR_D("blue-underline",   "4;34"),
	CRXDBG_COLOR_D("purple",           "0;35"),
	CRXDBG_COLOR_D("purple-bold",      "1;35"),
	CRXDBG_COLOR_D("purple-underline", "4;35"),
	CRXDBG_COLOR_D("cyan",             "0;36"),
	CRXDBG_COLOR_D("cyan-bold",        "1;36"),
	CRXDBG_COLOR_D("cyan-underline",   "4;36"),
	CRXDBG_COLOR_D("black",            "0;30"),
	CRXDBG_COLOR_D("black-bold",       "1;30"),
	CRXDBG_COLOR_D("black-underline",  "4;30"),
	CRXDBG_COLOR_END
}; /* }}} */

/* {{{ */
static const crxdbg_element_t elements[] = {
	CRXDBG_ELEMENT_D("prompt", CRXDBG_COLOR_PROMPT),
	CRXDBG_ELEMENT_D("error", CRXDBG_COLOR_ERROR),
	CRXDBG_ELEMENT_D("notice", CRXDBG_COLOR_NOTICE),
	CRXDBG_ELEMENT_END
}; /* }}} */

CRXDBG_API int crxdbg_is_numeric(const char *str) /* {{{ */
{
	if (!str)
		return 0;

	for (; *str; str++) {
		if (isspace(*str) || *str == '-') {
			continue;
		}
		return isdigit(*str);
	}
	return 0;
} /* }}} */

CRXDBG_API int crxdbg_is_empty(const char *str) /* {{{ */
{
	if (!str)
		return 1;

	for (; *str; str++) {
		if (isspace(*str)) {
			continue;
		}
		return 0;
	}
	return 1;
} /* }}} */

CRXDBG_API int crxdbg_is_addr(const char *str) /* {{{ */
{
	return str[0] && str[1] && memcmp(str, "0x", 2) == 0;
} /* }}} */

CRXDBG_API int crxdbg_is_class_method(const char *str, size_t len, char **class, char **method) /* {{{ */
{
	char *sep = NULL;

	if (strstr(str, "#") != NULL)
		return 0;

	if (strstr(str, " ") != NULL)
		return 0;

	sep = strstr(str, "::");

	if (!sep || sep == str || sep+2 == str+len-1) {
		return 0;
	}

	if (class != NULL) {

		if (str[0] == '\\') {
			str++;
			len--;
		}

		*class = estrndup(str, sep - str);
		(*class)[sep - str] = 0;
	}

	if (method != NULL) {
		*method = estrndup(sep+2, str + len - (sep + 2));
	}

	return 1;
} /* }}} */

CRXDBG_API char *crxdbg_resolve_path(const char *path) /* {{{ */
{
	char resolved_name[MAXPATHLEN];

	if (expand_filepath(path, resolved_name) == NULL) {
		return NULL;
	}

	return strdup(resolved_name);
} /* }}} */

CRXDBG_API const char *crxdbg_current_file(void) /* {{{ */
{
	const char *file = crex_get_executed_filename();

	if (memcmp(file, "[no active file]", sizeof("[no active file]")) == 0) {
		return CRXDBG_G(exec);
	}

	return file;
} /* }}} */

CRXDBG_API const crex_function *crxdbg_get_function(const char *fname, const char *cname) /* {{{ */
{
	crex_function *func = NULL;
	crex_string *lfname = crex_string_init(fname, strlen(fname), 0);
	crex_string *tmp = crex_string_tolower(lfname);
	crex_string_release(lfname);
	lfname = tmp;

	if (cname) {
		crex_class_entry *ce;
		crex_string *lcname = crex_string_init(cname, strlen(cname), 0);
		tmp = crex_string_tolower(lcname);
		crex_string_release(lcname);
		lcname = tmp;
		ce = crex_lookup_class(lcname);

		crex_string_release(lcname);

		if (ce) {
			func = crex_hash_find_ptr(&ce->function_table, lfname);
		}
	} else {
		func = crex_hash_find_ptr(EG(function_table), lfname);
	}

	crex_string_release(lfname);
	return func;
} /* }}} */

CRXDBG_API char *crxdbg_trim(const char *str, size_t len, size_t *new_len) /* {{{ */
{
	const char *p = str;
	char *new = NULL;

	while (isspace(*p)) {
		++p;
		--len;
	}

	while (*p && isspace(*(p + len -1))) {
		--len;
	}

	if (len == 0) {
		new = estrndup("", sizeof(""));
		*new_len = 0;
	} else {
		new = estrndup(p, len);
		*(new + len) = '\0';

		if (new_len) {
			*new_len = len;
		}
	}

	return new;

} /* }}} */

CRXDBG_API const crxdbg_color_t *crxdbg_get_color(const char *name, size_t name_length) /* {{{ */
{
	const crxdbg_color_t *color = colors;

	while (color && color->name) {
		if (name_length == color->name_length &&
			memcmp(name, color->name, name_length) == SUCCESS) {
			crxdbg_debug("crxdbg_get_color(%s, %lu): %s", name, name_length, color->code);
			return color;
		}
		++color;
	}

	crxdbg_debug("crxdbg_get_color(%s, %lu): failed", name, name_length);

	return NULL;
} /* }}} */

CRXDBG_API void crxdbg_set_color(int element, const crxdbg_color_t *color) /* {{{ */
{
	CRXDBG_G(colors)[element] = color;
} /* }}} */

CRXDBG_API void crxdbg_set_color_ex(int element, const char *name, size_t name_length) /* {{{ */
{
	const crxdbg_color_t *color = crxdbg_get_color(name, name_length);

	if (color) {
		crxdbg_set_color(element, color);
	} else CRXDBG_G(colors)[element] = colors;
} /* }}} */

CRXDBG_API const crxdbg_color_t* crxdbg_get_colors(void) /* {{{ */
{
	return colors;
} /* }}} */

CRXDBG_API int crxdbg_get_element(const char *name, size_t len) {
	const crxdbg_element_t *element = elements;

	while (element && element->name) {
		if (len == element->name_length) {
			if (strncasecmp(name, element->name, len) == SUCCESS) {
				return element->id;
			}
		}
		element++;
	}

	return CRXDBG_COLOR_INVALID;
}

CRXDBG_API void crxdbg_set_prompt(const char *prompt) /* {{{ */
{
	/* free formatted prompt */
	if (CRXDBG_G(prompt)[1]) {
		free(CRXDBG_G(prompt)[1]);
		CRXDBG_G(prompt)[1] = NULL;
	}
	/* free old prompt */
	if (CRXDBG_G(prompt)[0]) {
		free(CRXDBG_G(prompt)[0]);
		CRXDBG_G(prompt)[0] = NULL;
	}

	/* copy new prompt */
	CRXDBG_G(prompt)[0] = strdup(prompt);
} /* }}} */

CRXDBG_API const char *crxdbg_get_prompt(void) /* {{{ */
{
	/* find cached prompt */
	if (CRXDBG_G(prompt)[1]) {
		return CRXDBG_G(prompt)[1];
	}

	uint32_t pos = 0,
			 end = strlen(CRXDBG_G(prompt)[0]);
	bool unicode_warned = false;

	while (pos < end) {
		if (CRXDBG_G(prompt)[0][pos] & 0x80) {
			CRXDBG_G(prompt)[0][pos] = '?';

			if (!unicode_warned) {
				crex_error(E_WARNING,
					"prompt contains unsupported unicode characters");
				unicode_warned = true;
			}
		}
		pos++;
	}

	/* create cached prompt */
#ifndef HAVE_LIBEDIT
	/* TODO: libedit doesn't seems to support coloured prompt */
	if ((CRXDBG_G(flags) & CRXDBG_IS_COLOURED)) {
		CREX_IGNORE_VALUE(asprintf(&CRXDBG_G(prompt)[1], "\033[%sm%s\033[0m ",
			CRXDBG_G(colors)[CRXDBG_COLOR_PROMPT]->code,
			CRXDBG_G(prompt)[0]));
	} else
#endif
	{
		CREX_IGNORE_VALUE(asprintf(&CRXDBG_G(prompt)[1], "%s ", CRXDBG_G(prompt)[0]));
	}

	return CRXDBG_G(prompt)[1];
} /* }}} */

int crxdbg_rebuild_symtable(void) {
	if (!EG(current_execute_data) || !EG(current_execute_data)->func) {
		crxdbg_error("No active op array!");
		return FAILURE;
	}

	if (!crex_rebuild_symbol_table()) {
		crxdbg_error("No active symbol table!");
		return FAILURE;
	}

	return SUCCESS;
}

CRXDBG_API uint32_t crxdbg_get_terminal_width(void) /* {{{ */
{
	uint32_t columns;
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	columns = (uint32_t) csbi.srWindow.Right - csbi.srWindow.Left + 1;
#elif defined(HAVE_SYS_IOCTL_H) && defined(TIOCGWINSZ)
	struct winsize w;

	columns = (uint32_t) ioctl(fileno(stdout), TIOCGWINSZ, &w) == 0 ? w.ws_col : 80;
#else
	columns = 80;
#endif
	return columns;
} /* }}} */

CRXDBG_API uint32_t crxdbg_get_terminal_height(void) /* {{{ */
{
	uint32_t lines;
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
		lines = (uint32_t) csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	} else {
		lines = 40;
	}
#elif defined(HAVE_SYS_IOCTL_H) && defined(TIOCGWINSZ)
	struct winsize w;

	lines = (uint32_t) ioctl(fileno(stdout), TIOCGWINSZ, &w) == 0 ? w.ws_row : 40;
#else
	lines = 40;
#endif
	return lines;
} /* }}} */

CRXDBG_API void crxdbg_set_async_io(int fd) {
#if !defined(_WIN32) && defined(FASYNC)
	int flags;
	fcntl(STDIN_FILENO, F_SETOWN, getpid());
	flags = fcntl(STDIN_FILENO, F_GETFL);
	fcntl(STDIN_FILENO, F_SETFL, flags | FASYNC);
#endif
}

int crxdbg_safe_class_lookup(const char *name, int name_length, crex_class_entry **ce) {
	if (CRXDBG_G(flags) & CRXDBG_IN_SIGNAL_HANDLER) {
		char *lc_name, *lc_free;
		int lc_length;

		if (name == NULL || !name_length) {
			return FAILURE;
		}

		lc_free = lc_name = emalloc(name_length + 1);
		crex_str_tolower_copy(lc_name, name, name_length);
		lc_length = name_length + 1;

		if (lc_name[0] == '\\') {
			lc_name += 1;
			lc_length -= 1;
		}

		crxdbg_try_access {
			*ce = crex_hash_str_find_ptr(EG(class_table), lc_name, lc_length);
		} crxdbg_catch_access {
			crxdbg_error("Could not fetch class %.*s, invalid data source", name_length, name);
		} crxdbg_end_try_access();

		efree(lc_free);
	} else {
		crex_string *str_name = crex_string_init(name, name_length, 0);
		*ce = crex_lookup_class(str_name);
		efree(str_name);
	}

	return *ce ? SUCCESS : FAILURE;
}

char *crxdbg_get_property_key(char *key) {
	if (*key != 0) {
		return key;
	}
	return strchr(key + 1, 0) + 1;
}

static int crxdbg_parse_variable_arg_wrapper(char *name, size_t len, char *keyname, size_t keylen, HashTable *parent, zval *zv, crxdbg_parse_var_func callback) {
	return callback(name, len, keyname, keylen, parent, zv);
}

CRXDBG_API int crxdbg_parse_variable(char *input, size_t len, HashTable *parent, size_t i, crxdbg_parse_var_func callback, bool silent) {
	return crxdbg_parse_variable_with_arg(input, len, parent, i, (crxdbg_parse_var_with_arg_func) crxdbg_parse_variable_arg_wrapper, NULL, silent, callback);
}

CRXDBG_API int crxdbg_parse_variable_with_arg(char *input, size_t len, HashTable *parent, size_t i, crxdbg_parse_var_with_arg_func callback, crxdbg_parse_var_with_arg_func step_cb, bool silent, void *arg) {
	int ret = FAILURE;
	bool new_index = 1;
	char *last_index = NULL;
	size_t index_len = 0;
	zval *zv;

	if (len < 2 || *input != '$') {
		goto error;
	}

	while (i++ < len) {
		if (i == len) {
			new_index = 1;
		} else {
			switch (input[i]) {
				case '[':
					new_index = 1;
					break;
				case ']':
					break;
				case '>':
					if (!last_index) {
						goto error;
					}
					if (last_index[index_len - 1] == '-') {
						new_index = 1;
						index_len--;
					}
					break;

				default:
					if (new_index) {
						last_index = input + i;
						new_index = 0;
					}
					if (input[i - 1] == ']') {
						goto error;
					}
					index_len++;
			}
		}

		if (new_index && index_len == 0) {
			crex_ulong numkey;
			crex_string *strkey;
			CREX_HASH_FOREACH_KEY_VAL_IND(parent, numkey, strkey, zv) {
				if (i == len || (i == len - 1 && input[len - 1] == ']')) {
					char *key, *propkey;
					size_t namelen, keylen;
					char *name;
					char *keyname = estrndup(last_index, index_len);
					if (strkey) {
						key = ZSTR_VAL(strkey);
						keylen = ZSTR_LEN(strkey);
					} else {
						keylen = spprintf(&key, 0, CREX_ULONG_FMT, numkey);
					}
					propkey = crxdbg_get_property_key(key);
					name = emalloc(i + keylen + 2);
					namelen = sprintf(name, "%.*s%.*s%s", (int) i, input, (int) (keylen - (propkey - key)), propkey, input[len - 1] == ']'?"]":"");
					if (!strkey) {
						efree(key);
					}

					ret = callback(name, namelen, keyname, index_len, parent, zv, arg) == SUCCESS || ret == SUCCESS?SUCCESS:FAILURE;
				} else retry_ref: if (C_TYPE_P(zv) == IS_OBJECT) {
					if (step_cb) {
						char *name = estrndup(input, i);
						char *keyname = estrndup(last_index, index_len);

						ret = step_cb(name, i, keyname, index_len, parent, zv, arg) == SUCCESS || ret == SUCCESS?SUCCESS:FAILURE;
					}

					crxdbg_parse_variable_with_arg(input, len, C_OBJPROP_P(zv), i, callback, step_cb, silent, arg);
				} else if (C_TYPE_P(zv) == IS_ARRAY) {
					if (step_cb) {
						char *name = estrndup(input, i);
						char *keyname = estrndup(last_index, index_len);

						ret = step_cb(name, i, keyname, index_len, parent, zv, arg) == SUCCESS || ret == SUCCESS?SUCCESS:FAILURE;
					}

					crxdbg_parse_variable_with_arg(input, len, C_ARRVAL_P(zv), i, callback, step_cb, silent, arg);
				} else if (C_ISREF_P(zv)) {
					if (step_cb) {
						char *name = estrndup(input, i);
						char *keyname = estrndup(last_index, index_len);

						ret = step_cb(name, i, keyname, index_len, parent, zv, arg) == SUCCESS || ret == SUCCESS?SUCCESS:FAILURE;
					}

					ZVAL_DEREF(zv);
					goto retry_ref;
				} else {
					/* Ignore silently */
				}
			} CREX_HASH_FOREACH_END();
			return ret;
		} else if (new_index) {
			char last_chr = last_index[index_len];
			last_index[index_len] = 0;
			if (!(zv = crex_symtable_str_find(parent, last_index, index_len))) {
				if (!silent) {
					crxdbg_error("%.*s is undefined", (int) (input[i] == ']' ? i + 1 : i), input);
				}
				return FAILURE;
			}
			while (C_TYPE_P(zv) == IS_INDIRECT) {
				zv = C_INDIRECT_P(zv);
			}

			last_index[index_len] = last_chr;
			if (i == len) {
				char *name = estrndup(input, i);
				char *keyname = estrndup(last_index, index_len);

				ret = callback(name, i, keyname, index_len, parent, zv, arg) == SUCCESS || ret == SUCCESS?SUCCESS:FAILURE;
			} else retry_ref_end: if (C_TYPE_P(zv) == IS_OBJECT) {
				if (step_cb) {
					char *name = estrndup(input, i);
					char *keyname = estrndup(last_index, index_len);

					ret = step_cb(name, i, keyname, index_len, parent, zv, arg) == SUCCESS || ret == SUCCESS?SUCCESS:FAILURE;
				}

				parent = C_OBJPROP_P(zv);
			} else if (C_TYPE_P(zv) == IS_ARRAY) {
				if (step_cb) {
					char *name = estrndup(input, i);
					char *keyname = estrndup(last_index, index_len);

					ret = step_cb(name, i, keyname, index_len, parent, zv, arg) == SUCCESS || ret == SUCCESS?SUCCESS:FAILURE;
				}

				parent = C_ARRVAL_P(zv);
			} else if (C_ISREF_P(zv)) {
				if (step_cb) {
					char *name = estrndup(input, i);
					char *keyname = estrndup(last_index, index_len);

					ret = step_cb(name, i, keyname, index_len, parent, zv, arg) == SUCCESS || ret == SUCCESS?SUCCESS:FAILURE;
				}

				ZVAL_DEREF(zv);
				goto retry_ref_end;
			} else {
				crxdbg_error("%.*s is nor an array nor an object", (int) (input[i] == '>' ? i - 1 : i), input);
				return FAILURE;
			}
			index_len = 0;
		}
	}

	return ret;
	error:
		crxdbg_error("Malformed input");
		return FAILURE;
}

int crxdbg_is_auto_global(char *name, int len) {
	return crex_is_auto_global_str(name, len);
}

CRXDBG_API bool crxdbg_check_caught_ex(crex_execute_data *execute_data, crex_object *exception) {
	const crex_op *op;
	crex_op *cur;
	uint32_t op_num, i;
	crex_op_array *op_array = &execute_data->func->op_array;

	if (execute_data->opline >= EG(exception_op) && execute_data->opline < EG(exception_op) + 3 && EG(opline_before_exception)) {
		op = EG(opline_before_exception);
	} else {
		op = execute_data->opline;
	}

	op_num = op - op_array->opcodes;

	for (i = 0; i < op_array->last_try_catch && op_array->try_catch_array[i].try_op <= op_num; i++) {
		uint32_t catch = op_array->try_catch_array[i].catch_op, finally = op_array->try_catch_array[i].finally_op;
		if (op_num <= catch || op_num <= finally) {
			if (finally) {
				return 1;
			}

			cur = &op_array->opcodes[catch];
			while (1) {
				crex_class_entry *ce;

				if (!(ce = CACHED_PTR(cur->extended_value & ~CREX_LAST_CATCH))) {
					ce = crex_fetch_class_by_name(C_STR_P(RT_CONSTANT(cur, cur->op1)), C_STR_P(RT_CONSTANT(cur, cur->op1) + 1), CREX_FETCH_CLASS_NO_AUTOLOAD);
					CACHE_PTR(cur->extended_value & ~CREX_LAST_CATCH, ce);
				}

				if (ce == exception->ce || (ce && instanceof_function(exception->ce, ce))) {
					return 1;
				}

				if (cur->extended_value & CREX_LAST_CATCH) {
					return 0;
				}

				cur = OP_JMP_ADDR(cur, cur->op2);
			}

			return 0;
		}
	}

	return op->opcode == CREX_CATCH;
}

char *crxdbg_short_zval_print(zval *zv, int maxlen) /* {{{ */
{
	char *decode = NULL;

	switch (C_TYPE_P(zv)) {
		case IS_UNDEF:
			decode = estrdup("");
			break;
		case IS_NULL:
			decode = estrdup("null");
			break;
		case IS_FALSE:
			decode = estrdup("false");
			break;
		case IS_TRUE:
			decode = estrdup("true");
			break;
		case IS_LONG:
			spprintf(&decode, 0, CREX_LONG_FMT, C_LVAL_P(zv));
			break;
		case IS_DOUBLE:
			spprintf(&decode, 0, "%.*G", 14, C_DVAL_P(zv));

			/* Make sure it looks like a float */
			if (crex_finite(C_DVAL_P(zv)) && !strchr(decode, '.')) {
				size_t len = strlen(decode);
				char *decode2 = emalloc(len + strlen(".0") + 1);
				memcpy(decode2, decode, len);
				decode2[len] = '.';
				decode2[len+1] = '0';
				decode2[len+2] = '\0';
				efree(decode);
				decode = decode2;
			}
			break;
		case IS_STRING: {
			int i;
			crex_string *str = crx_addcslashes(C_STR_P(zv), "\\\"\n\t\0", 5);
			for (i = 0; i < ZSTR_LEN(str); i++) {
				if (ZSTR_VAL(str)[i] < 32) {
					ZSTR_VAL(str)[i] = ' ';
				}
			}
			spprintf(&decode, 0, "\"%.*s\"%c",
				ZSTR_LEN(str) <= maxlen - 2 ? (int) ZSTR_LEN(str) : (maxlen - 3),
				ZSTR_VAL(str), ZSTR_LEN(str) <= maxlen - 2 ? 0 : '+');
			crex_string_release(str);
			} break;
		case IS_RESOURCE:
			spprintf(&decode, 0, "Rsrc #" CREX_LONG_FMT, C_RES_HANDLE_P(zv));
			break;
		case IS_ARRAY:
			spprintf(&decode, 0, "array(%d)", crex_hash_num_elements(C_ARR_P(zv)));
			break;
		case IS_OBJECT: {
			crex_string *str = C_OBJCE_P(zv)->name;
			spprintf(&decode, 0, "%.*s%c",
				ZSTR_LEN(str) <= maxlen ? (int) ZSTR_LEN(str) : maxlen - 1,
				ZSTR_VAL(str), ZSTR_LEN(str) <= maxlen ? 0 : '+');
			break;
		}
		case IS_CONSTANT_AST: {
			crex_ast *ast = C_ASTVAL_P(zv);

			if (ast->kind == CREX_AST_CONSTANT
			 || ast->kind == CREX_AST_CONSTANT_CLASS
			 || ast->kind == CREX_AST_CLASS_CONST) {
				decode = estrdup("<constant>");
			} else {
				decode = estrdup("<ast>");
			}
			break;
		}
		default:
			spprintf(&decode, 0, "unknown type: %d", C_TYPE_P(zv));
			break;
	}

	return decode;
} /* }}} */
