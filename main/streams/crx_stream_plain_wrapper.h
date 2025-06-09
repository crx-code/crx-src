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
   | Author: Wez Furlong <wez@thebrainroom.com>                           |
   +----------------------------------------------------------------------+
 */

/* definitions for the plain files wrapper */

/* operations for a plain file; use the crx_stream_fopen_XXX funcs below */
CRXAPI extern crx_stream_ops crx_stream_stdio_ops;
CRXAPI extern /*const*/ crx_stream_wrapper crx_plain_files_wrapper;

BEGIN_EXTERN_C()

/* like fopen, but returns a stream */
CRXAPI crx_stream *_crx_stream_fopen(const char *filename, const char *mode, crex_string **opened_path, int options STREAMS_DC);
#define crx_stream_fopen(filename, mode, opened)	_crx_stream_fopen((filename), (mode), (opened), 0 STREAMS_CC)

CRXAPI crx_stream *_crx_stream_fopen_with_path(const char *filename, const char *mode, const char *path, crex_string **opened_path, int options STREAMS_DC);
#define crx_stream_fopen_with_path(filename, mode, path, opened)	_crx_stream_fopen_with_path((filename), (mode), (path), (opened), 0 STREAMS_CC)

CRXAPI crx_stream *_crx_stream_fopen_from_file(FILE *file, const char *mode STREAMS_DC);
#define crx_stream_fopen_from_file(file, mode)	_crx_stream_fopen_from_file((file), (mode) STREAMS_CC)

CRXAPI crx_stream *_crx_stream_fopen_from_fd(int fd, const char *mode, const char *persistent_id, bool zero_position STREAMS_DC);
#define crx_stream_fopen_from_fd(fd, mode, persistent_id)	_crx_stream_fopen_from_fd((fd), (mode), (persistent_id), false STREAMS_CC)

CRXAPI crx_stream *_crx_stream_fopen_from_pipe(FILE *file, const char *mode STREAMS_DC);
#define crx_stream_fopen_from_pipe(file, mode)	_crx_stream_fopen_from_pipe((file), (mode) STREAMS_CC)

CRXAPI crx_stream *_crx_stream_fopen_tmpfile(int dummy STREAMS_DC);
#define crx_stream_fopen_tmpfile()	_crx_stream_fopen_tmpfile(0 STREAMS_CC)

CRXAPI crx_stream *_crx_stream_fopen_temporary_file(const char *dir, const char *pfx, crex_string **opened_path STREAMS_DC);
#define crx_stream_fopen_temporary_file(dir, pfx, opened_path)	_crx_stream_fopen_temporary_file((dir), (pfx), (opened_path) STREAMS_CC)

/* This is a utility API for extensions that are opening a stream, converting it
 * to a FILE* and then closing it again.  Be warned that fileno() on the result
 * will most likely fail on systems with fopencookie. */
CRXAPI FILE * _crx_stream_open_wrapper_as_file(char * path, char * mode, int options, crex_string **opened_path STREAMS_DC);
#define crx_stream_open_wrapper_as_file(path, mode, options, opened_path) _crx_stream_open_wrapper_as_file((path), (mode), (options), (opened_path) STREAMS_CC)

/* parse standard "fopen" modes into open() flags */
CRXAPI int crx_stream_parse_fopen_modes(const char *mode, int *open_flags);

END_EXTERN_C()
