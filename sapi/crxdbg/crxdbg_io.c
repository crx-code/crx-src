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
   | Authors: Anatol Belski <ab@crx.net>                                  |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crxdbg_io.h"

CREX_EXTERN_MODULE_GLOBALS(crxdbg)

/* is easy to generalize ... but not needed for now */
CRXDBG_API int crxdbg_consume_stdin_line(char *buf) {
	int bytes = CRXDBG_G(input_buflen), len = 0;

	if (CRXDBG_G(input_buflen)) {
		memcpy(buf, CRXDBG_G(input_buffer), bytes);
	}

	CRXDBG_G(last_was_newline) = 1;

	do {
		int i;
		if (bytes <= 0) {
			continue;
		}

		for (i = len; i < len + bytes; i++) {
			if (buf[i] == '\x03') {
				if (i != len + bytes - 1) {
					memmove(buf + i, buf + i + 1, len + bytes - i - 1);
				}
				len--;
				i--;
				continue;
			}
			if (buf[i] == '\n') {
				CRXDBG_G(input_buflen) = len + bytes - 1 - i;
				if (CRXDBG_G(input_buflen)) {
					memcpy(CRXDBG_G(input_buffer), buf + i + 1, CRXDBG_G(input_buflen));
				}
				if (i != CRXDBG_MAX_CMD - 1) {
					buf[i + 1] = 0;
				}
				return i;
			}
		}

		len += bytes;
	} while ((bytes = crxdbg_mixed_read(CRXDBG_G(io)[CRXDBG_STDIN].fd, buf + len, CRXDBG_MAX_CMD - len, -1)) > 0);

	if (bytes <= 0) {
		CRXDBG_G(flags) |= CRXDBG_IS_QUITTING;
		crex_bailout();
	}

	return bytes;
}

CRXDBG_API int crxdbg_mixed_read(int fd, char *ptr, int len, int tmo) {
	int ret;

	do {
		ret = read(fd, ptr, len);
	} while (ret == -1 && errno == EINTR);

	return ret;
}

static int crxdbg_output_pager(int fd, const char *ptr, int len) {
	int count = 0, bytes = 0;
	const char *p = ptr, *endp = ptr + len;

	while ((p = memchr(p, '\n', endp - p))) {
		count++;
		p++;

		if (count % CRXDBG_G(lines) == 0) {
			bytes += write(fd, ptr + bytes, (p - ptr) - bytes);

			if (memchr(p, '\n', endp - p)) {
				char buf[CRXDBG_MAX_CMD];
				crex_quiet_write(fd, CREX_STRL("\r---Type <return> to continue or q <return> to quit---"));
				crxdbg_consume_stdin_line(buf);
				if (*buf == 'q') {
					break;
				}
				crex_quiet_write(fd, "\r", 1);
			} else break;
		}
	}
	if (bytes && count % CRXDBG_G(lines) != 0) {
		bytes += write(fd, ptr + bytes, len - bytes);
	} else if (!bytes) {
		bytes += write(fd, ptr, len);
	}
	return bytes;
}

CRXDBG_API int crxdbg_mixed_write(int fd, const char *ptr, int len) {
	if ((CRXDBG_G(flags) & CRXDBG_HAS_PAGINATION)
	 && CRXDBG_G(io)[CRXDBG_STDOUT].fd == fd
	 && CRXDBG_G(lines) > 0) {
		return crxdbg_output_pager(fd, ptr, len);
	}

	return write(fd, ptr, len);
}
