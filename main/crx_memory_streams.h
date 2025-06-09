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
   | Author: Marcus Boerger <helly@crx.net>                               |
   +----------------------------------------------------------------------+
 */

#ifndef CRX_MEMORY_STREAM_H
#define CRX_MEMORY_STREAM_H

#include "crx_streams.h"

#define CRX_STREAM_MAX_MEM	2 * 1024 * 1024

#define TEMP_STREAM_DEFAULT     0x0
#define TEMP_STREAM_READONLY    0x1
#define TEMP_STREAM_TAKE_BUFFER 0x2
#define TEMP_STREAM_APPEND      0x4

#define crx_stream_memory_create(mode) _crx_stream_memory_create((mode) STREAMS_CC)
#define crx_stream_memory_create_rel(mode) _crx_stream_memory_create((mode) STREAMS_REL_CC)
#define crx_stream_memory_open(mode, str) _crx_stream_memory_open((mode), (str) STREAMS_CC)
#define crx_stream_memory_get_buffer(stream) _crx_stream_memory_get_buffer((stream) STREAMS_CC)

#define crx_stream_temp_new() crx_stream_temp_create(TEMP_STREAM_DEFAULT, CRX_STREAM_MAX_MEM)
#define crx_stream_temp_create(mode, max_memory_usage) _crx_stream_temp_create((mode), (max_memory_usage) STREAMS_CC)
#define crx_stream_temp_create_ex(mode, max_memory_usage, tmpdir) _crx_stream_temp_create_ex((mode), (max_memory_usage), (tmpdir) STREAMS_CC)
#define crx_stream_temp_create_rel(mode, max_memory_usage) _crx_stream_temp_create((mode), (max_memory_usage) STREAMS_REL_CC)
#define crx_stream_temp_open(mode, max_memory_usage, buf, length) _crx_stream_temp_open((mode), (max_memory_usage), (buf), (length) STREAMS_CC)

BEGIN_EXTERN_C()

CRXAPI crx_stream *_crx_stream_memory_create(int mode STREAMS_DC);
CRXAPI crx_stream *_crx_stream_memory_open(int mode, crex_string *buf STREAMS_DC);
CRXAPI crex_string *_crx_stream_memory_get_buffer(crx_stream *stream STREAMS_DC);

CRXAPI crx_stream *_crx_stream_temp_create(int mode, size_t max_memory_usage STREAMS_DC);
CRXAPI crx_stream *_crx_stream_temp_create_ex(int mode, size_t max_memory_usage, const char *tmpdir STREAMS_DC);
CRXAPI crx_stream *_crx_stream_temp_open(int mode, size_t max_memory_usage, const char *buf, size_t length STREAMS_DC);

CRXAPI int crx_stream_mode_from_str(const char *mode);
CRXAPI const char *_crx_stream_mode_to_str(int mode);

END_EXTERN_C()

extern CRXAPI const crx_stream_ops crx_stream_memory_ops;
extern CRXAPI const crx_stream_ops crx_stream_temp_ops;
extern CRXAPI const crx_stream_ops crx_stream_rfc2397_ops;
extern CRXAPI const crx_stream_wrapper crx_stream_rfc2397_wrapper;

#define CRX_STREAM_IS_MEMORY &crx_stream_memory_ops
#define CRX_STREAM_IS_TEMP   &crx_stream_temp_ops

#endif
