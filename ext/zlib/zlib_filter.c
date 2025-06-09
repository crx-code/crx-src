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
   | Authors: Sara Golemon (pollita@crx.net)                              |
   +----------------------------------------------------------------------+
*/

#include "crx.h"
#include "crx_zlib.h"

/* {{{ data structure */

/* Passed as opaque in malloc callbacks */
typedef struct _crx_zlib_filter_data {
	z_stream strm;
	unsigned char *inbuf;
	size_t inbuf_len;
	unsigned char *outbuf;
	size_t outbuf_len;
	int persistent;
	bool finished; /* for zlib.deflate: signals that no flush is pending */
} crx_zlib_filter_data;

/* }}} */

/* {{{ Memory management wrappers */

static voidpf crx_zlib_alloc(voidpf opaque, uInt items, uInt size)
{
	return (voidpf)safe_pemalloc(items, size, 0, ((crx_zlib_filter_data*)opaque)->persistent);
}

static void crx_zlib_free(voidpf opaque, voidpf address)
{
	pefree((void*)address, ((crx_zlib_filter_data*)opaque)->persistent);
}
/* }}} */

/* {{{ zlib.inflate filter implementation */

static crx_stream_filter_status_t crx_zlib_inflate_filter(
	crx_stream *stream,
	crx_stream_filter *thisfilter,
	crx_stream_bucket_brigade *buckets_in,
	crx_stream_bucket_brigade *buckets_out,
	size_t *bytes_consumed,
	int flags
	)
{
	crx_zlib_filter_data *data;
	crx_stream_bucket *bucket;
	size_t consumed = 0;
	int status;
	crx_stream_filter_status_t exit_status = PSFS_FEED_ME;

	if (!thisfilter || !C_PTR(thisfilter->abstract)) {
		/* Should never happen */
		return PSFS_ERR_FATAL;
	}

	data = (crx_zlib_filter_data *)(C_PTR(thisfilter->abstract));

	while (buckets_in->head) {
		size_t bin = 0, desired;

		bucket = crx_stream_bucket_make_writeable(buckets_in->head);

		while (bin < (unsigned int) bucket->buflen && !data->finished) {

			desired = bucket->buflen - bin;
			if (desired > data->inbuf_len) {
				desired = data->inbuf_len;
			}
			memcpy(data->strm.next_in, bucket->buf + bin, desired);
			data->strm.avail_in = desired;

			status = inflate(&(data->strm), flags & PSFS_FLAG_FLUSH_CLOSE ? C_FINISH : C_SYNC_FLUSH);
			if (status == C_STREAM_END) {
				inflateEnd(&(data->strm));
				data->finished = '\1';
				exit_status = PSFS_PASS_ON;
			} else if (status != C_OK && status != C_BUF_ERROR) {
				/* Something bad happened */
				crx_error_docref(NULL, E_NOTICE, "zlib: %s", zError(status));
				crx_stream_bucket_delref(bucket);
				/* reset these because despite the error the filter may be used again */
				data->strm.next_in = data->inbuf;
				data->strm.avail_in = 0;
				return PSFS_ERR_FATAL;
			}
			desired -= data->strm.avail_in; /* desired becomes what we consumed this round through */
			data->strm.next_in = data->inbuf;
			data->strm.avail_in = 0;
			bin += desired;

			if (data->strm.avail_out < data->outbuf_len) {
				crx_stream_bucket *out_bucket;
				size_t bucketlen = data->outbuf_len - data->strm.avail_out;
				out_bucket = crx_stream_bucket_new(
					stream, estrndup((char *) data->outbuf, bucketlen), bucketlen, 1, 0);
				crx_stream_bucket_append(buckets_out, out_bucket);
				data->strm.avail_out = data->outbuf_len;
				data->strm.next_out = data->outbuf;
				exit_status = PSFS_PASS_ON;
			}

		}
		consumed += bucket->buflen;
		crx_stream_bucket_delref(bucket);
	}

	if (!data->finished && flags & PSFS_FLAG_FLUSH_CLOSE) {
		/* Spit it out! */
		status = C_OK;
		while (status == C_OK) {
			status = inflate(&(data->strm), C_FINISH);
			if (data->strm.avail_out < data->outbuf_len) {
				size_t bucketlen = data->outbuf_len - data->strm.avail_out;

				bucket = crx_stream_bucket_new(
					stream, estrndup((char *) data->outbuf, bucketlen), bucketlen, 1, 0);
				crx_stream_bucket_append(buckets_out, bucket);
				data->strm.avail_out = data->outbuf_len;
				data->strm.next_out = data->outbuf;
				exit_status = PSFS_PASS_ON;
			}
		}
	}

	if (bytes_consumed) {
		*bytes_consumed = consumed;
	}

	return exit_status;
}

static void crx_zlib_inflate_dtor(crx_stream_filter *thisfilter)
{
	if (thisfilter && C_PTR(thisfilter->abstract)) {
		crx_zlib_filter_data *data = C_PTR(thisfilter->abstract);
		if (!data->finished) {
			inflateEnd(&(data->strm));
		}
		pefree(data->inbuf, data->persistent);
		pefree(data->outbuf, data->persistent);
		pefree(data, data->persistent);
	}
}

static const crx_stream_filter_ops crx_zlib_inflate_ops = {
	crx_zlib_inflate_filter,
	crx_zlib_inflate_dtor,
	"zlib.inflate"
};
/* }}} */

/* {{{ zlib.deflate filter implementation */

static crx_stream_filter_status_t crx_zlib_deflate_filter(
	crx_stream *stream,
	crx_stream_filter *thisfilter,
	crx_stream_bucket_brigade *buckets_in,
	crx_stream_bucket_brigade *buckets_out,
	size_t *bytes_consumed,
	int flags
	)
{
	crx_zlib_filter_data *data;
	crx_stream_bucket *bucket;
	size_t consumed = 0;
	int status;
	crx_stream_filter_status_t exit_status = PSFS_FEED_ME;

	if (!thisfilter || !C_PTR(thisfilter->abstract)) {
		/* Should never happen */
		return PSFS_ERR_FATAL;
	}

	data = (crx_zlib_filter_data *)(C_PTR(thisfilter->abstract));

	while (buckets_in->head) {
		size_t bin = 0, desired;

		bucket = buckets_in->head;

		bucket = crx_stream_bucket_make_writeable(bucket);

		while (bin < (unsigned int) bucket->buflen) {
			int flush_mode;

			desired = bucket->buflen - bin;
			if (desired > data->inbuf_len) {
				desired = data->inbuf_len;
			}
			memcpy(data->strm.next_in, bucket->buf + bin, desired);
			data->strm.avail_in = desired;

			flush_mode = flags & PSFS_FLAG_FLUSH_CLOSE ? C_FULL_FLUSH : (flags & PSFS_FLAG_FLUSH_INC ? C_SYNC_FLUSH : C_NO_FLUSH);
			data->finished = flush_mode != C_NO_FLUSH;
			status = deflate(&(data->strm), flush_mode);
			if (status != C_OK) {
				/* Something bad happened */
				crx_stream_bucket_delref(bucket);
				return PSFS_ERR_FATAL;
			}
			desired -= data->strm.avail_in; /* desired becomes what we consumed this round through */
			data->strm.next_in = data->inbuf;
			data->strm.avail_in = 0;
			bin += desired;

			if (data->strm.avail_out < data->outbuf_len) {
				crx_stream_bucket *out_bucket;
				size_t bucketlen = data->outbuf_len - data->strm.avail_out;

				out_bucket = crx_stream_bucket_new(
					stream, estrndup((char *) data->outbuf, bucketlen), bucketlen, 1, 0);
				crx_stream_bucket_append(buckets_out, out_bucket);
				data->strm.avail_out = data->outbuf_len;
				data->strm.next_out = data->outbuf;
				exit_status = PSFS_PASS_ON;
			}
		}
		consumed += bucket->buflen;
		crx_stream_bucket_delref(bucket);
	}

	if (flags & PSFS_FLAG_FLUSH_CLOSE || ((flags & PSFS_FLAG_FLUSH_INC) && !data->finished)) {
		/* Spit it out! */
		status = C_OK;
		while (status == C_OK) {
			status = deflate(&(data->strm), (flags & PSFS_FLAG_FLUSH_CLOSE ? C_FINISH : C_SYNC_FLUSH));
			data->finished = 1;
			if (data->strm.avail_out < data->outbuf_len) {
				size_t bucketlen = data->outbuf_len - data->strm.avail_out;

				bucket = crx_stream_bucket_new(
					stream, estrndup((char *) data->outbuf, bucketlen), bucketlen, 1, 0);
				crx_stream_bucket_append(buckets_out, bucket);
				data->strm.avail_out = data->outbuf_len;
				data->strm.next_out = data->outbuf;
				exit_status = PSFS_PASS_ON;
			}
		}
	}

	if (bytes_consumed) {
		*bytes_consumed = consumed;
	}

	return exit_status;
}

static void crx_zlib_deflate_dtor(crx_stream_filter *thisfilter)
{
	if (thisfilter && C_PTR(thisfilter->abstract)) {
		crx_zlib_filter_data *data = C_PTR(thisfilter->abstract);
		deflateEnd(&(data->strm));
		pefree(data->inbuf, data->persistent);
		pefree(data->outbuf, data->persistent);
		pefree(data, data->persistent);
	}
}

static const crx_stream_filter_ops crx_zlib_deflate_ops = {
	crx_zlib_deflate_filter,
	crx_zlib_deflate_dtor,
	"zlib.deflate"
};

/* }}} */

/* {{{ zlib.* common factory */

static crx_stream_filter *crx_zlib_filter_create(const char *filtername, zval *filterparams, uint8_t persistent)
{
	const crx_stream_filter_ops *fops = NULL;
	crx_zlib_filter_data *data;
	int status;

	/* Create this filter */
	data = pecalloc(1, sizeof(crx_zlib_filter_data), persistent);
	if (!data) {
		crx_error_docref(NULL, E_WARNING, "Failed allocating %zd bytes", sizeof(crx_zlib_filter_data));
		return NULL;
	}

	/* Circular reference */
	data->strm.opaque = (voidpf) data;

	data->strm.zalloc = (alloc_func) crx_zlib_alloc;
	data->strm.zfree = (free_func) crx_zlib_free;
	data->strm.avail_out = data->outbuf_len = data->inbuf_len = 0x8000;
	data->strm.next_in = data->inbuf = (Bytef *) pemalloc(data->inbuf_len, persistent);
	if (!data->inbuf) {
		crx_error_docref(NULL, E_WARNING, "Failed allocating %zd bytes", data->inbuf_len);
		pefree(data, persistent);
		return NULL;
	}
	data->strm.avail_in = 0;
	data->strm.next_out = data->outbuf = (Bytef *) pemalloc(data->outbuf_len, persistent);
	if (!data->outbuf) {
		crx_error_docref(NULL, E_WARNING, "Failed allocating %zd bytes", data->outbuf_len);
		pefree(data->inbuf, persistent);
		pefree(data, persistent);
		return NULL;
	}

	data->strm.data_type = C_ASCII;

	if (strcasecmp(filtername, "zlib.inflate") == 0) {
		int windowBits = -MAX_WBITS;

		if (filterparams) {
			zval *tmpzval;

			if ((C_TYPE_P(filterparams) == IS_ARRAY || C_TYPE_P(filterparams) == IS_OBJECT) &&
				(tmpzval = crex_hash_str_find(HASH_OF(filterparams), "window", sizeof("window") - 1))) {
				/* log-2 base of history window (9 - 15) */
				crex_long tmp = zval_get_long(tmpzval);
				if (tmp < -MAX_WBITS || tmp > MAX_WBITS + 32) {
					crx_error_docref(NULL, E_WARNING, "Invalid parameter given for window size (" CREX_LONG_FMT ")", tmp);
				} else {
					windowBits = tmp;
				}
			}
		}

		/* RFC 1951 Inflate */
		data->finished = '\0';
		status = inflateInit2(&(data->strm), windowBits);
		fops = &crx_zlib_inflate_ops;
	} else if (strcasecmp(filtername, "zlib.deflate") == 0) {
		/* RFC 1951 Deflate */
		int level = C_DEFAULT_COMPRESSION;
		int windowBits = -MAX_WBITS;
		int memLevel = MAX_MEM_LEVEL;


		if (filterparams) {
			zval *tmpzval;
			crex_long tmp;

			/* filterparams can either be a scalar value to indicate compression level (shortcut method)
               Or can be a hash containing one or more of 'window', 'memory', and/or 'level' members. */

			switch (C_TYPE_P(filterparams)) {
				case IS_ARRAY:
				case IS_OBJECT:
					if ((tmpzval = crex_hash_str_find(HASH_OF(filterparams), "memory", sizeof("memory") -1))) {
						/* Memory Level (1 - 9) */
						tmp = zval_get_long(tmpzval);
						if (tmp < 1 || tmp > MAX_MEM_LEVEL) {
							crx_error_docref(NULL, E_WARNING, "Invalid parameter given for memory level (" CREX_LONG_FMT ")", tmp);
						} else {
							memLevel = tmp;
						}
					}

					if ((tmpzval = crex_hash_str_find(HASH_OF(filterparams), "window", sizeof("window") - 1))) {
						/* log-2 base of history window (9 - 15) */
						tmp = zval_get_long(tmpzval);
						if (tmp < -MAX_WBITS || tmp > MAX_WBITS + 16) {
							crx_error_docref(NULL, E_WARNING, "Invalid parameter given for window size (" CREX_LONG_FMT ")", tmp);
						} else {
							windowBits = tmp;
						}
					}

					if ((tmpzval = crex_hash_str_find(HASH_OF(filterparams), "level", sizeof("level") - 1))) {
						tmp = zval_get_long(tmpzval);

						/* Pseudo pass through to catch level validating code */
						goto factory_setlevel;
					}
					break;
				case IS_STRING:
				case IS_DOUBLE:
				case IS_LONG:
					tmp = zval_get_long(filterparams);
factory_setlevel:
					/* Set compression level within reason (-1 == default, 0 == none, 1-9 == least to most compression */
					if (tmp < -1 || tmp > 9) {
						crx_error_docref(NULL, E_WARNING, "Invalid compression level specified. (" CREX_LONG_FMT ")", tmp);
					} else {
						level = tmp;
					}
					break;
				default:
					crx_error_docref(NULL, E_WARNING, "Invalid filter parameter, ignored");
			}
		}
		status = deflateInit2(&(data->strm), level, C_DEFLATED, windowBits, memLevel, 0);
		data->finished = 1;
		fops = &crx_zlib_deflate_ops;
	} else {
		status = C_DATA_ERROR;
	}

	if (status != C_OK) {
		/* Unspecified (probably strm) error, let stream-filter error do its own whining */
		pefree(data->strm.next_in, persistent);
		pefree(data->strm.next_out, persistent);
		pefree(data, persistent);
		return NULL;
	}

	return crx_stream_filter_alloc(fops, data, persistent);
}

const crx_stream_filter_factory crx_zlib_filter_factory = {
	crx_zlib_filter_create
};
/* }}} */
