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

CRXAPI extern const crx_stream_wrapper crx_glob_stream_wrapper;
CRXAPI extern const crx_stream_ops     crx_glob_stream_ops;

BEGIN_EXTERN_C()

CRXAPI char* _crx_glob_stream_get_path(crx_stream *stream, size_t *plen STREAMS_DC);
#define crx_glob_stream_get_path(stream, plen)	_crx_glob_stream_get_path((stream), (plen) STREAMS_CC)

CRXAPI char* _crx_glob_stream_get_pattern(crx_stream *stream, size_t *plen STREAMS_DC);
#define crx_glob_stream_get_pattern(stream, plen)	_crx_glob_stream_get_pattern((stream), (plen) STREAMS_CC)

CRXAPI int   _crx_glob_stream_get_count(crx_stream *stream, int *pflags STREAMS_DC);
#define crx_glob_stream_get_count(stream, pflags)	_crx_glob_stream_get_count((stream), (pflags) STREAMS_CC)

END_EXTERN_C()
