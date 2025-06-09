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

#ifndef CRXDBG_OUT_H
#define CRXDBG_OUT_H

/**
 * Error/notice/formatting helpers
 */
enum {
	P_ERROR  = 1,
	P_NOTICE,
	P_WRITELN,
	P_WRITE,
	P_STDOUT,
	P_STDERR,
	P_LOG
};

CRXDBG_API int crxdbg_print(int severity, int fd, const char *strfmt, ...) CRX_ATTRIBUTE_FORMAT(printf, 3, 4);
CRXDBG_API int crxdbg_log_internal(int fd, const char *fmt, ...) CRX_ATTRIBUTE_FORMAT(printf, 2, 3);
CRXDBG_API int crxdbg_out_internal(int fd, const char *fmt, ...) CRX_ATTRIBUTE_FORMAT(printf, 2, 3);

#define crxdbg_error(strfmt, ...)              crxdbg_print(P_ERROR  , CRXDBG_G(io)[CRXDBG_STDOUT].fd, strfmt, ##__VA_ARGS__)
#define crxdbg_notice(strfmt, ...)             crxdbg_print(P_NOTICE , CRXDBG_G(io)[CRXDBG_STDOUT].fd, strfmt, ##__VA_ARGS__)
#define crxdbg_writeln(strfmt, ...)            crxdbg_print(P_WRITELN, CRXDBG_G(io)[CRXDBG_STDOUT].fd, strfmt, ##__VA_ARGS__)
#define crxdbg_write(strfmt, ...)              crxdbg_print(P_WRITE  , CRXDBG_G(io)[CRXDBG_STDOUT].fd, strfmt, ##__VA_ARGS__)

#define crxdbg_log(fmt, ...)                   crxdbg_log_internal(CRXDBG_G(io)[CRXDBG_STDOUT].fd, fmt, ##__VA_ARGS__)
#define crxdbg_out(fmt, ...)                   crxdbg_out_internal(CRXDBG_G(io)[CRXDBG_STDOUT].fd, fmt, ##__VA_ARGS__)

#define crxdbg_script(type, strfmt, ...)       crxdbg_print(type,      CRXDBG_G(io)[CRXDBG_STDOUT].fd, strfmt, ##__VA_ARGS__)

#define crxdbg_asprintf(buf, ...) _crxdbg_asprintf(buf, ##__VA_ARGS__)
CRXDBG_API int _crxdbg_asprintf(char **buf, const char *format, ...);

#if CRXDBG_DEBUG
#	define crxdbg_debug(strfmt, ...) crxdbg_log_internal(CRXDBG_G(io)[CRXDBG_STDERR].fd, strfmt, ##__VA_ARGS__)
#else
#	define crxdbg_debug(strfmt, ...)
#endif

CRXDBG_API void crxdbg_free_err_buf(void);
CRXDBG_API void crxdbg_activate_err_buf(bool active);
CRXDBG_API int crxdbg_output_err_buf(const char *strfmt, ...);


/* {{{ For separation */
#define SEPARATE "------------------------------------------------" /* }}} */

#endif /* CRXDBG_OUT_H */
