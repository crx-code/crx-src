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
   | Author: Rasmus Lerdorf <rasmus@crx.net>                              |
   |         Ilia Alshanetsky <iliaa@crx.net>                             |
   +----------------------------------------------------------------------+
 */

#include <stdio.h>
#include "crx.h"
#include <ctype.h>
#include "crx_string.h"
#include "ext/standard/head.h"
#include "ext/standard/file.h"
#include "basic_functions.h"
#include "exec.h"
#include "crx_globals.h"
#include "SAPI.h"

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include <signal.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <limits.h>

#ifdef CRX_WIN32
# include "win32/nice.h"
#endif

static size_t cmd_max_len;

/* {{{ CRX_MINIT_FUNCTION(exec) */
CRX_MINIT_FUNCTION(exec)
{
#ifdef _SC_ARG_MAX
	cmd_max_len = sysconf(_SC_ARG_MAX);
	if ((size_t)-1 == cmd_max_len) {
#ifdef _POSIX_ARG_MAX
		cmd_max_len = _POSIX_ARG_MAX;
#else
		cmd_max_len = 4096;
#endif
	}
#elif defined(ARG_MAX)
	cmd_max_len = ARG_MAX;
#elif defined(CRX_WIN32)
	/* Executed commands will run through cmd.exe. As long as it's the case,
		it's just the constant limit.*/
	cmd_max_len = 8192;
#else
	/* This is just an arbitrary value for the fallback case. */
	cmd_max_len = 4096;
#endif

	return SUCCESS;
}
/* }}} */

static size_t strip_trailing_whitespace(char *buf, size_t bufl) {
	size_t l = bufl;
	while (l-- > 0 && isspace(((unsigned char *)buf)[l]));
	if (l != (bufl - 1)) {
		bufl = l + 1;
		buf[bufl] = '\0';
	}
	return bufl;
}

static size_t handle_line(int type, zval *array, char *buf, size_t bufl) {
	if (type == 1) {
		CRXWRITE(buf, bufl);
		if (crx_output_get_level() < 1) {
			sapi_flush();
		}
	} else if (type == 2) {
		bufl = strip_trailing_whitespace(buf, bufl);
		add_next_index_stringl(array, buf, bufl);
	}
	return bufl;
}

/* {{{ crx_exec
 * If type==0, only last line of output is returned (exec)
 * If type==1, all lines will be printed and last lined returned (system)
 * If type==2, all lines will be saved to given array (exec with &$array)
 * If type==3, output will be printed binary, no lines will be saved or returned (passthru)
 *
 */
CRXAPI int crx_exec(int type, const char *cmd, zval *array, zval *return_value)
{
	FILE *fp;
	char *buf;
	int pclose_return;
	char *b, *d=NULL;
	crx_stream *stream;
	size_t buflen, bufl = 0;
#if CRX_SIGCHILD
	void (*sig_handler)() = NULL;
#endif

#if CRX_SIGCHILD
	sig_handler = signal (SIGCHLD, SIG_DFL);
#endif

#ifdef CRX_WIN32
	fp = VCWD_POPEN(cmd, "rb");
#else
	fp = VCWD_POPEN(cmd, "r");
#endif
	if (!fp) {
		crx_error_docref(NULL, E_WARNING, "Unable to fork [%s]", cmd);
		goto err;
	}

	stream = crx_stream_fopen_from_pipe(fp, "rb");

	buf = (char *) emalloc(EXEC_INPUT_BUF);
	buflen = EXEC_INPUT_BUF;

	if (type != 3) {
		b = buf;

		while (crx_stream_get_line(stream, b, EXEC_INPUT_BUF, &bufl)) {
			/* no new line found, let's read some more */
			if (b[bufl - 1] != '\n' && !crx_stream_eof(stream)) {
				if (buflen < (bufl + (b - buf) + EXEC_INPUT_BUF)) {
					bufl += b - buf;
					buflen = bufl + EXEC_INPUT_BUF;
					buf = erealloc(buf, buflen);
					b = buf + bufl;
				} else {
					b += bufl;
				}
				continue;
			} else if (b != buf) {
				bufl += b - buf;
			}

			bufl = handle_line(type, array, buf, bufl);
			b = buf;
		}
		if (bufl) {
			if (buf != b) {
				/* Process remaining output */
				bufl = handle_line(type, array, buf, bufl);
			}

			/* Return last line from the shell command */
			bufl = strip_trailing_whitespace(buf, bufl);
			RETVAL_STRINGL(buf, bufl);
		} else { /* should return NULL, but for BC we return "" */
			RETVAL_EMPTY_STRING();
		}
	} else {
		ssize_t read;
		while ((read = crx_stream_read(stream, buf, EXEC_INPUT_BUF)) > 0) {
			CRXWRITE(buf, read);
		}
	}

	pclose_return = crx_stream_close(stream);
	efree(buf);

done:
#if CRX_SIGCHILD
	if (sig_handler) {
		signal(SIGCHLD, sig_handler);
	}
#endif
	if (d) {
		efree(d);
	}
	return pclose_return;
err:
	pclose_return = -1;
	RETVAL_FALSE;
	goto done;
}
/* }}} */

static void crx_exec_ex(INTERNAL_FUNCTION_PARAMETERS, int mode) /* {{{ */
{
	char *cmd;
	size_t cmd_len;
	zval *ret_code=NULL, *ret_array=NULL;
	int ret;

	CREX_PARSE_PARAMETERS_START(1, (mode ? 2 : 3))
		C_PARAM_STRING(cmd, cmd_len)
		C_PARAM_OPTIONAL
		if (!mode) {
			C_PARAM_ZVAL(ret_array)
		}
		C_PARAM_ZVAL(ret_code)
	CREX_PARSE_PARAMETERS_END();

	if (!cmd_len) {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	}
	if (strlen(cmd) != cmd_len) {
		crex_argument_value_error(1, "must not contain any null bytes");
		RETURN_THROWS();
	}

	if (!ret_array) {
		ret = crx_exec(mode, cmd, NULL, return_value);
	} else {
		if (C_TYPE_P(C_REFVAL_P(ret_array)) == IS_ARRAY) {
			ZVAL_DEREF(ret_array);
			SEPARATE_ARRAY(ret_array);
		} else {
			ret_array = crex_try_array_init(ret_array);
			if (!ret_array) {
				RETURN_THROWS();
			}
		}

		ret = crx_exec(2, cmd, ret_array, return_value);
	}
	if (ret_code) {
		CREX_TRY_ASSIGN_REF_LONG(ret_code, ret);
	}
}
/* }}} */

/* {{{ Execute an external program */
CRX_FUNCTION(exec)
{
	crx_exec_ex(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ Execute an external program and display output */
CRX_FUNCTION(system)
{
	crx_exec_ex(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ Execute an external program and display raw output */
CRX_FUNCTION(passthru)
{
	crx_exec_ex(INTERNAL_FUNCTION_PARAM_PASSTHRU, 3);
}
/* }}} */

/* {{{ crx_escape_shell_cmd
   Escape all chars that could possibly be used to
   break out of a shell command

   This function emalloc's a string and returns the pointer.
   Remember to efree it when done with it.

   *NOT* safe for binary strings
*/
CRXAPI crex_string *crx_escape_shell_cmd(const char *str)
{
	size_t x, y;
	size_t l = strlen(str);
	uint64_t estimate = (2 * (uint64_t)l) + 1;
	crex_string *cmd;
#ifndef CRX_WIN32
	char *p = NULL;
#endif

	/* max command line length - two single quotes - \0 byte length */
	if (l > cmd_max_len - 2 - 1) {
		crx_error_docref(NULL, E_ERROR, "Command exceeds the allowed length of %zu bytes", cmd_max_len);
		return ZSTR_EMPTY_ALLOC();
	}

	cmd = crex_string_safe_alloc(2, l, 0, 0);

	for (x = 0, y = 0; x < l; x++) {
		int mb_len = crx_mblen(str + x, (l - x));

		/* skip non-valid multibyte characters */
		if (mb_len < 0) {
			continue;
		} else if (mb_len > 1) {
			memcpy(ZSTR_VAL(cmd) + y, str + x, mb_len);
			y += mb_len;
			x += mb_len - 1;
			continue;
		}

		switch (str[x]) {
#ifndef CRX_WIN32
			case '"':
			case '\'':
				if (!p && (p = memchr(str + x + 1, str[x], l - x - 1))) {
					/* noop */
				} else if (p && *p == str[x]) {
					p = NULL;
				} else {
					ZSTR_VAL(cmd)[y++] = '\\';
				}
				ZSTR_VAL(cmd)[y++] = str[x];
				break;
#else
			/* % is Windows specific for environmental variables, ^%PATH% will
				output PATH while ^%PATH^% will not. escapeshellcmd->val will escape all % and !.
			*/
			case '%':
			case '!':
			case '"':
			case '\'':
#endif
			case '#': /* This is character-set independent */
			case '&':
			case ';':
			case '`':
			case '|':
			case '*':
			case '?':
			case '~':
			case '<':
			case '>':
			case '^':
			case '(':
			case ')':
			case '[':
			case ']':
			case '{':
			case '}':
			case '$':
			case '\\':
			case '\x0A': /* excluding these two */
			case '\xFF':
#ifdef CRX_WIN32
				ZSTR_VAL(cmd)[y++] = '^';
#else
				ZSTR_VAL(cmd)[y++] = '\\';
#endif
				CREX_FALLTHROUGH;
			default:
				ZSTR_VAL(cmd)[y++] = str[x];

		}
	}
	ZSTR_VAL(cmd)[y] = '\0';

	if (y > cmd_max_len + 1) {
		crx_error_docref(NULL, E_ERROR, "Escaped command exceeds the allowed length of %zu bytes", cmd_max_len);
		crex_string_release_ex(cmd, 0);
		return ZSTR_EMPTY_ALLOC();
	}

	if ((estimate - y) > 4096) {
		/* realloc if the estimate was way overill
		 * Arbitrary cutoff point of 4096 */
		cmd = crex_string_truncate(cmd, y, 0);
	}

	ZSTR_LEN(cmd) = y;

	return cmd;
}
/* }}} */

/* {{{ crx_escape_shell_arg */
CRXAPI crex_string *crx_escape_shell_arg(const char *str)
{
	size_t x, y = 0;
	size_t l = strlen(str);
	crex_string *cmd;
	uint64_t estimate = (4 * (uint64_t)l) + 3;

	/* max command line length - two single quotes - \0 byte length */
	if (l > cmd_max_len - 2 - 1) {
		crx_error_docref(NULL, E_ERROR, "Argument exceeds the allowed length of %zu bytes", cmd_max_len);
		return ZSTR_EMPTY_ALLOC();
	}

	cmd = crex_string_safe_alloc(4, l, 2, 0); /* worst case */

#ifdef CRX_WIN32
	ZSTR_VAL(cmd)[y++] = '"';
#else
	ZSTR_VAL(cmd)[y++] = '\'';
#endif

	for (x = 0; x < l; x++) {
		int mb_len = crx_mblen(str + x, (l - x));

		/* skip non-valid multibyte characters */
		if (mb_len < 0) {
			continue;
		} else if (mb_len > 1) {
			memcpy(ZSTR_VAL(cmd) + y, str + x, mb_len);
			y += mb_len;
			x += mb_len - 1;
			continue;
		}

		switch (str[x]) {
#ifdef CRX_WIN32
		case '"':
		case '%':
		case '!':
			ZSTR_VAL(cmd)[y++] = ' ';
			break;
#else
		case '\'':
			ZSTR_VAL(cmd)[y++] = '\'';
			ZSTR_VAL(cmd)[y++] = '\\';
			ZSTR_VAL(cmd)[y++] = '\'';
#endif
			CREX_FALLTHROUGH;
		default:
			ZSTR_VAL(cmd)[y++] = str[x];
		}
	}
#ifdef CRX_WIN32
	if (y > 0 && '\\' == ZSTR_VAL(cmd)[y - 1]) {
		int k = 0, n = y - 1;
		for (; n >= 0 && '\\' == ZSTR_VAL(cmd)[n]; n--, k++);
		if (k % 2) {
			ZSTR_VAL(cmd)[y++] = '\\';
		}
	}

	ZSTR_VAL(cmd)[y++] = '"';
#else
	ZSTR_VAL(cmd)[y++] = '\'';
#endif
	ZSTR_VAL(cmd)[y] = '\0';

	if (y > cmd_max_len + 1) {
		crx_error_docref(NULL, E_ERROR, "Escaped argument exceeds the allowed length of %zu bytes", cmd_max_len);
		crex_string_release_ex(cmd, 0);
		return ZSTR_EMPTY_ALLOC();
	}

	if ((estimate - y) > 4096) {
		/* realloc if the estimate was way overill
		 * Arbitrary cutoff point of 4096 */
		cmd = crex_string_truncate(cmd, y, 0);
	}
	ZSTR_LEN(cmd) = y;
	return cmd;
}
/* }}} */

/* {{{ Escape shell metacharacters */
CRX_FUNCTION(escapeshellcmd)
{
	char *command;
	size_t command_len;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STRING(command, command_len)
	CREX_PARSE_PARAMETERS_END();

	if (command_len) {
		if (command_len != strlen(command)) {
			crex_argument_value_error(1, "must not contain any null bytes");
			RETURN_THROWS();
		}
		RETVAL_STR(crx_escape_shell_cmd(command));
	} else {
		RETVAL_EMPTY_STRING();
	}
}
/* }}} */

/* {{{ Quote and escape an argument for use in a shell command */
CRX_FUNCTION(escapeshellarg)
{
	char *argument;
	size_t argument_len;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STRING(argument, argument_len)
	CREX_PARSE_PARAMETERS_END();

	if (argument_len != strlen(argument)) {
		crex_argument_value_error(1, "must not contain any null bytes");
		RETURN_THROWS();
	}

	RETVAL_STR(crx_escape_shell_arg(argument));
}
/* }}} */

/* {{{ Execute command via shell and return complete output as string */
CRX_FUNCTION(shell_exec)
{
	FILE *in;
	char *command;
	size_t command_len;
	crex_string *ret;
	crx_stream *stream;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STRING(command, command_len)
	CREX_PARSE_PARAMETERS_END();

	if (!command_len) {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	}
	if (strlen(command) != command_len) {
		crex_argument_value_error(1, "must not contain any null bytes");
		RETURN_THROWS();
	}

#ifdef CRX_WIN32
	if ((in=VCWD_POPEN(command, "rt"))==NULL) {
#else
	if ((in=VCWD_POPEN(command, "r"))==NULL) {
#endif
		crx_error_docref(NULL, E_WARNING, "Unable to execute '%s'", command);
		RETURN_FALSE;
	}

	stream = crx_stream_fopen_from_pipe(in, "rb");
	ret = crx_stream_copy_to_mem(stream, CRX_STREAM_COPY_ALL, 0);
	crx_stream_close(stream);

	if (ret && ZSTR_LEN(ret) > 0) {
		RETVAL_STR(ret);
	}
}
/* }}} */

#ifdef HAVE_NICE
/* {{{ Change the priority of the current process */
CRX_FUNCTION(proc_nice)
{
	crex_long pri;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(pri)
	CREX_PARSE_PARAMETERS_END();

	errno = 0;
	crx_ignore_value(nice(pri));
	if (errno) {
#ifdef CRX_WIN32
		char *err = crx_win_err();
		crx_error_docref(NULL, E_WARNING, "%s", err);
		crx_win_err_free(err);
#else
		crx_error_docref(NULL, E_WARNING, "Only a super user may attempt to increase the priority of a process");
#endif
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */
#endif
