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

/* Memory Mapping interface for streams.
 * The intention is to provide a uniform interface over the most common
 * operations that are used within CRX itself, rather than a complete
 * API for all memory mapping needs.
 *
 * ATM, we support only mmap(), but win32 memory mapping support will
 * follow soon.
 * */

typedef enum {
	/* Does the stream support mmap ? */
	CRX_STREAM_MMAP_SUPPORTED,
	/* Request a range and offset to be mapped;
	 * while mapped, you MUST NOT use any read/write functions
	 * on the stream (win9x compatibility) */
	CRX_STREAM_MMAP_MAP_RANGE,
	/* Unmap the last range that was mapped for the stream */
	CRX_STREAM_MMAP_UNMAP
} crx_stream_mmap_operation_t;

typedef enum {
	CRX_STREAM_MAP_MODE_READONLY,
	CRX_STREAM_MAP_MODE_READWRITE,
	CRX_STREAM_MAP_MODE_SHARED_READONLY,
	CRX_STREAM_MAP_MODE_SHARED_READWRITE
} crx_stream_mmap_access_t;

typedef struct {
	/* requested offset and length.
	 * If length is 0, the whole file is mapped */
	size_t offset;
	size_t length;

	crx_stream_mmap_access_t mode;

	/* returned mapped address */
	char *mapped;

} crx_stream_mmap_range;

#define CRX_STREAM_MMAP_ALL 0

#define CRX_STREAM_MMAP_MAX (512 * 1024 * 1024)

#define crx_stream_mmap_supported(stream)	(_crx_stream_set_option((stream), CRX_STREAM_OPTION_MMAP_API, CRX_STREAM_MMAP_SUPPORTED, NULL) == 0 ? 1 : 0)

/* Returns 1 if the stream in its current state can be memory mapped,
 * 0 otherwise */
#define crx_stream_mmap_possible(stream)			(!crx_stream_is_filtered((stream)) && crx_stream_mmap_supported((stream)))

BEGIN_EXTERN_C()
CRXAPI char *_crx_stream_mmap_range(crx_stream *stream, size_t offset, size_t length, crx_stream_mmap_access_t mode, size_t *mapped_len);
#define crx_stream_mmap_range(stream, offset, length, mode, mapped_len)	_crx_stream_mmap_range((stream), (offset), (length), (mode), (mapped_len))

/* un-maps the last mapped range */
CRXAPI int _crx_stream_mmap_unmap(crx_stream *stream);
#define crx_stream_mmap_unmap(stream)				_crx_stream_mmap_unmap((stream))

CRXAPI int _crx_stream_mmap_unmap_ex(crx_stream *stream, crex_off_t readden);
#define crx_stream_mmap_unmap_ex(stream, readden)			_crx_stream_mmap_unmap_ex((stream), (readden))
END_EXTERN_C()
