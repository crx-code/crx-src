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
   | Authors: Rasmus Lerdorf <rasmus@lerdorf.on.ca>                       |
   |          Stefan R�hrich <sr@linux.de>                                |
   |          Michael Wallner <mike@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_ZLIB_H
#define CRX_ZLIB_H

#include "crx_version.h"
#define CRX_ZLIB_VERSION CRX_VERSION

#include <zlib.h>

#define CRX_ZLIB_ENCODING_RAW		-0xf
#define CRX_ZLIB_ENCODING_GZIP		0x1f
#define CRX_ZLIB_ENCODING_DEFLATE	0x0f

#define CRX_ZLIB_ENCODING_ANY		0x2f

#define CRX_ZLIB_OUTPUT_HANDLER_NAME "zlib output compression"
#define CRX_ZLIB_BUFFER_SIZE_GUESS(in_len) (((size_t) ((double) in_len * (double) 1.015)) + 10 + 8 + 4 + 1)

typedef struct _crx_zlib_buffer {
	char *data;
	char *aptr;
	size_t used;
	size_t free;
	size_t size;
} crx_zlib_buffer;

typedef struct _crx_zlib_context {
	z_stream Z;
	char *inflateDict;
	int status;
	size_t inflateDictlen;
	crx_zlib_buffer buffer;
	crex_object std;
} crx_zlib_context;

CREX_BEGIN_MODULE_GLOBALS(zlib)
	/* variables for transparent gzip encoding */
	crex_long output_compression;
	crex_long output_compression_level;
	char *output_handler;
	crx_zlib_context *ob_gzhandler;
	crex_long output_compression_default;
	bool handler_registered;
	int compression_coding;
CREX_END_MODULE_GLOBALS(zlib);

#define ZLIBG(v) CREX_MODULE_GLOBALS_ACCESSOR(zlib, v)

crx_stream *crx_stream_gzopen(crx_stream_wrapper *wrapper, const char *path, const char *mode, int options, crex_string **opened_path, crx_stream_context *context STREAMS_DC);
extern const crx_stream_ops crx_stream_gzio_ops;
extern const crx_stream_wrapper crx_stream_gzip_wrapper;
extern const crx_stream_filter_factory crx_zlib_filter_factory;
extern crex_module_entry crx_zlib_module_entry;
#define zlib_module_ptr &crx_zlib_module_entry
#define crxext_zlib_ptr zlib_module_ptr

#endif /* CRX_ZLIB_H */
