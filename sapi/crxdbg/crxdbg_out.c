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
#include "spprintf.h"
#include "crxdbg.h"
#include "crxdbg_io.h"
#include "ext/standard/html.h"

#ifdef _WIN32
#	include "win32/time.h"
#endif

CREX_EXTERN_MODULE_GLOBALS(crxdbg)

CRXDBG_API int _crxdbg_asprintf(char **buf, const char *format, ...) {
	int ret;
	va_list va;

	va_start(va, format);
	ret = vasprintf(buf, format, va);
	va_end(va);

	return ret;
}

static int crxdbg_process_print(int fd, int type, const char *msg, int msglen) {
	char *msgout = NULL;
	int msgoutlen = FAILURE;

	switch (type) {
		case P_ERROR:
			if (!CRXDBG_G(last_was_newline)) {
				crxdbg_mixed_write(fd, CREX_STRL("\n"));
				CRXDBG_G(last_was_newline) = 1;
			}
			if (CRXDBG_G(flags) & CRXDBG_IS_COLOURED) {
				msgoutlen = crxdbg_asprintf(&msgout, "\033[%sm[%.*s]\033[0m\n", CRXDBG_G(colors)[CRXDBG_COLOR_ERROR]->code, msglen, msg);
			} else {
				msgoutlen = crxdbg_asprintf(&msgout, "[%.*s]\n", msglen, msg);
			}
			break;

		case P_NOTICE:
			if (!CRXDBG_G(last_was_newline)) {
				crxdbg_mixed_write(fd, CREX_STRL("\n"));
				CRXDBG_G(last_was_newline) = 1;
			}
			if (CRXDBG_G(flags) & CRXDBG_IS_COLOURED) {
				msgoutlen = crxdbg_asprintf(&msgout, "\033[%sm[%.*s]\033[0m\n", CRXDBG_G(colors)[CRXDBG_COLOR_NOTICE]->code, msglen, msg);
			} else {
				msgoutlen = crxdbg_asprintf(&msgout, "[%.*s]\n", msglen, msg);
			}
			break;

		case P_WRITELN:
			if (msg) {
				msgoutlen = crxdbg_asprintf(&msgout, "%.*s\n", msglen, msg);
			} else {
				msgoutlen = 1;
				msgout = strdup("\n");
			}
			CRXDBG_G(last_was_newline) = 1;
			break;

		case P_WRITE:
			if (msg) {
				msgout = pestrndup(msg, msglen, 1);
				msgoutlen = msglen;
				CRXDBG_G(last_was_newline) = msg[msglen - 1] == '\n';
			} else {
				msgoutlen = 0;
				msgout = strdup("");
			}
			break;

		case P_STDOUT:
		case P_STDERR:
			if (msg) {
				CRXDBG_G(last_was_newline) = msg[msglen - 1] == '\n';
				crxdbg_mixed_write(fd, msg, msglen);
			}
			return msglen;

		/* no formatting on logging output */
		case P_LOG:
			if (msg) {
				struct timeval tp;
				if (gettimeofday(&tp, NULL) == SUCCESS) {
					msgoutlen = crxdbg_asprintf(&msgout, "[%ld %.8F]: %.*s\n", tp.tv_sec, tp.tv_usec / 1000000., msglen, msg);
				} else {
					msgoutlen = FAILURE;
				}
			}
			break;
		EMPTY_SWITCH_DEFAULT_CASE()
	}

	if (msgoutlen != FAILURE) {
		crxdbg_mixed_write(fd, msgout, msgoutlen);

		free(msgout);
	}
	return msgoutlen;
} /* }}} */

CRXDBG_API int crxdbg_vprint(int type, int fd, const char *strfmt, va_list args) {
	char *msg = NULL;
	int msglen = 0;
	int len;
	va_list argcpy;

	if (strfmt != NULL && strlen(strfmt) > 0L) {
		va_copy(argcpy, args);
		msglen = vasprintf(&msg, strfmt, argcpy);
		va_end(argcpy);
	}

	if (CRXDBG_G(err_buf).active && type != P_STDOUT && type != P_STDERR) {
		crxdbg_free_err_buf();

		CRXDBG_G(err_buf).type = type;
		CRXDBG_G(err_buf).fd = fd;
		CRXDBG_G(err_buf).msg = msg;
		CRXDBG_G(err_buf).msglen = msglen;

		return msglen;
	}

	if (UNEXPECTED(msglen == 0)) {
		len = 0;
	} else {
		len = crxdbg_process_print(fd, type, msg, msglen);
	}

	if (msg) {
		free(msg);
	}

	return len;
}

CRXDBG_API void crxdbg_free_err_buf(void) {
	if (CRXDBG_G(err_buf).type == 0) {
		return;
	}

	free(CRXDBG_G(err_buf).msg);

	CRXDBG_G(err_buf).type = 0;
}

CRXDBG_API void crxdbg_activate_err_buf(bool active) {
	CRXDBG_G(err_buf).active = active;
}

CRXDBG_API int crxdbg_output_err_buf(const char *strfmt, ...) {
	int len;
	va_list args;
	int errbuf_active = CRXDBG_G(err_buf).active;

	if (CRXDBG_G(flags) & CRXDBG_DISCARD_OUTPUT) {
		return 0;
	}

	CRXDBG_G(err_buf).active = 0;

	va_start(args, strfmt);
	len = crxdbg_vprint(CRXDBG_G(err_buf).type, CRXDBG_G(err_buf).fd, strfmt, args);
	va_end(args);

	CRXDBG_G(err_buf).active = errbuf_active;
	crxdbg_free_err_buf();

	return len;
}

CRXDBG_API int crxdbg_print(int type, int fd, const char *strfmt, ...) {
	va_list args;
	int len;

	if (CRXDBG_G(flags) & CRXDBG_DISCARD_OUTPUT) {
		return 0;
	}

	va_start(args, strfmt);
	len = crxdbg_vprint(type, fd, strfmt, args);
	va_end(args);

	return len;
}

CRXDBG_API int crxdbg_log_internal(int fd, const char *fmt, ...) {
	va_list args;
	char *buffer;
	int buflen;
	int len = 0;

	va_start(args, fmt);
	buflen = vasprintf(&buffer, fmt, args);
	va_end(args);

	len = crxdbg_mixed_write(fd, buffer, buflen);
	free(buffer);

	return len;
}

CRXDBG_API int crxdbg_out_internal(int fd, const char *fmt, ...) {
	va_list args;
	char *buffer;
	int buflen;
	int len = 0;

	if (CRXDBG_G(flags) & CRXDBG_DISCARD_OUTPUT) {
		return 0;
	}

	va_start(args, fmt);
	buflen = vasprintf(&buffer, fmt, args);
	va_end(args);

	len = crxdbg_mixed_write(fd, buffer, buflen);

	free(buffer);
	return len;
}
