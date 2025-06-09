/*
  +----------------------------------------------------------------------+
  | crxa crx single-file executable CRX extension                        |
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
  | Authors: Gregory Beaver <cellog@crx.net>                             |
  |          Marcus Boerger <helly@crx.net>                              |
  +----------------------------------------------------------------------+
*/

BEGIN_EXTERN_C()
int crxa_wrapper_mkdir(crx_stream_wrapper *wrapper, const char *url_from, int mode, int options, crx_stream_context *context);
int crxa_wrapper_rmdir(crx_stream_wrapper *wrapper, const char *url, int options, crx_stream_context *context);

#ifdef CRXA_DIRSTREAM
crx_url* crxa_parse_url(crx_stream_wrapper *wrapper, const char *filename, const char *mode, int options);

/* directory handlers */
static ssize_t crxa_dir_write(crx_stream *stream, const char *buf, size_t count);
static ssize_t crxa_dir_read( crx_stream *stream, char *buf, size_t count);
static int    crxa_dir_close(crx_stream *stream, int close_handle);
static int    crxa_dir_flush(crx_stream *stream);
static int    crxa_dir_seek( crx_stream *stream, crex_off_t offset, int whence, crex_off_t *newoffset);
#else
crx_stream* crxa_wrapper_open_dir(crx_stream_wrapper *wrapper, const char *path, const char *mode, int options, crex_string **opened_path, crx_stream_context *context STREAMS_DC);
#endif
END_EXTERN_C()
