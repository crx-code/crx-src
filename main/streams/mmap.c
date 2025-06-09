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

/* Memory Mapping interface for streams */
#include "crx.h"
#include "crx_streams_int.h"

CRXAPI char *_crx_stream_mmap_range(crx_stream *stream, size_t offset, size_t length, crx_stream_mmap_access_t mode, size_t *mapped_len)
{
	crx_stream_mmap_range range;

	range.offset = offset;
	range.length = length;
	range.mode = mode;
	range.mapped = NULL;

	if (CRX_STREAM_OPTION_RETURN_OK == crx_stream_set_option(stream, CRX_STREAM_OPTION_MMAP_API, CRX_STREAM_MMAP_MAP_RANGE, &range)) {
		if (mapped_len) {
			*mapped_len = range.length;
		}
		return range.mapped;
	}
	return NULL;
}

CRXAPI int _crx_stream_mmap_unmap(crx_stream *stream)
{
	return crx_stream_set_option(stream, CRX_STREAM_OPTION_MMAP_API, CRX_STREAM_MMAP_UNMAP, NULL) == CRX_STREAM_OPTION_RETURN_OK;
}

CRXAPI int _crx_stream_mmap_unmap_ex(crx_stream *stream, crex_off_t readden)
{
	int ret = 1;

	if (crx_stream_seek(stream, readden, SEEK_CUR) != 0) {
		ret = 0;
	}
	if (crx_stream_mmap_unmap(stream) == 0) {
		ret = 0;
	}

	return ret;
}
