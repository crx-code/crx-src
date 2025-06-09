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
   | With suggestions from:                                               |
   |      Moriyoshi Koizumi <moriyoshi@at.wakwak.com>                     |
   |      Sara Golemon      <pollita@crx.net>                             |
   +----------------------------------------------------------------------+
 */

/* The filter API works on the principle of "Bucket-Brigades".  This is
 * partially inspired by the Apache 2 method of doing things, although
 * it is intentionally a light-weight implementation.
 *
 * Each stream can have a chain of filters for reading and another for writing.
 *
 * When data is written to the stream, it is placed into a bucket and placed at
 * the start of the input brigade.
 *
 * The first filter in the chain is invoked on the brigade and (depending on
 * it's return value), the next filter is invoked and so on.
 * */

#define CRX_STREAM_FILTER_READ	0x0001
#define CRX_STREAM_FILTER_WRITE	0x0002
#define CRX_STREAM_FILTER_ALL	(CRX_STREAM_FILTER_READ | CRX_STREAM_FILTER_WRITE)

typedef struct _crx_stream_bucket			crx_stream_bucket;
typedef struct _crx_stream_bucket_brigade	crx_stream_bucket_brigade;

struct _crx_stream_bucket {
	crx_stream_bucket *next, *prev;
	crx_stream_bucket_brigade *brigade;

	char *buf;
	size_t buflen;
	/* if non-zero, buf should be pefreed when the bucket is destroyed */
	uint8_t own_buf;
	uint8_t is_persistent;

	/* destroy this struct when refcount falls to zero */
	int refcount;
};

struct _crx_stream_bucket_brigade {
	crx_stream_bucket *head, *tail;
};

typedef enum {
	PSFS_ERR_FATAL,	/* error in data stream */
	PSFS_FEED_ME,	/* filter needs more data; stop processing chain until more is available */
	PSFS_PASS_ON	/* filter generated output buckets; pass them on to next in chain */
} crx_stream_filter_status_t;

/* Buckets API. */
BEGIN_EXTERN_C()
CRXAPI crx_stream_bucket *crx_stream_bucket_new(crx_stream *stream, char *buf, size_t buflen, uint8_t own_buf, uint8_t buf_persistent);
CRXAPI int crx_stream_bucket_split(crx_stream_bucket *in, crx_stream_bucket **left, crx_stream_bucket **right, size_t length);
CRXAPI void crx_stream_bucket_delref(crx_stream_bucket *bucket);
#define crx_stream_bucket_addref(bucket)	(bucket)->refcount++
CRXAPI void crx_stream_bucket_prepend(crx_stream_bucket_brigade *brigade, crx_stream_bucket *bucket);
CRXAPI void crx_stream_bucket_append(crx_stream_bucket_brigade *brigade, crx_stream_bucket *bucket);
CRXAPI void crx_stream_bucket_unlink(crx_stream_bucket *bucket);
CRXAPI crx_stream_bucket *crx_stream_bucket_make_writeable(crx_stream_bucket *bucket);
END_EXTERN_C()

#define PSFS_FLAG_NORMAL		0	/* regular read/write */
#define PSFS_FLAG_FLUSH_INC		1	/* an incremental flush */
#define PSFS_FLAG_FLUSH_CLOSE	2	/* final flush prior to closing */

typedef struct _crx_stream_filter_ops {

	crx_stream_filter_status_t (*filter)(
			crx_stream *stream,
			crx_stream_filter *thisfilter,
			crx_stream_bucket_brigade *buckets_in,
			crx_stream_bucket_brigade *buckets_out,
			size_t *bytes_consumed,
			int flags
			);

	void (*dtor)(crx_stream_filter *thisfilter);

	const char *label;

} crx_stream_filter_ops;

typedef struct _crx_stream_filter_chain {
	crx_stream_filter *head, *tail;

	/* Owning stream */
	crx_stream *stream;
} crx_stream_filter_chain;

struct _crx_stream_filter {
	const crx_stream_filter_ops *fops;
	zval abstract; /* for use by filter implementation */
	crx_stream_filter *next;
	crx_stream_filter *prev;
	int is_persistent;

	/* link into stream and chain */
	crx_stream_filter_chain *chain;

	/* buffered buckets */
	crx_stream_bucket_brigade buffer;

	/* filters are auto_registered when they're applied */
	crex_resource *res;
};

/* stack filter onto a stream */
BEGIN_EXTERN_C()
CRXAPI void _crx_stream_filter_prepend(crx_stream_filter_chain *chain, crx_stream_filter *filter);
CRXAPI int crx_stream_filter_prepend_ex(crx_stream_filter_chain *chain, crx_stream_filter *filter);
CRXAPI void _crx_stream_filter_append(crx_stream_filter_chain *chain, crx_stream_filter *filter);
CRXAPI int crx_stream_filter_append_ex(crx_stream_filter_chain *chain, crx_stream_filter *filter);
CRXAPI int _crx_stream_filter_flush(crx_stream_filter *filter, int finish);
CRXAPI crx_stream_filter *crx_stream_filter_remove(crx_stream_filter *filter, int call_dtor);
CRXAPI void crx_stream_filter_free(crx_stream_filter *filter);
CRXAPI crx_stream_filter *_crx_stream_filter_alloc(const crx_stream_filter_ops *fops, void *abstract, uint8_t persistent STREAMS_DC);
END_EXTERN_C()
#define crx_stream_filter_alloc(fops, thisptr, persistent) _crx_stream_filter_alloc((fops), (thisptr), (persistent) STREAMS_CC)
#define crx_stream_filter_alloc_rel(fops, thisptr, persistent) _crx_stream_filter_alloc((fops), (thisptr), (persistent) STREAMS_REL_CC)
#define crx_stream_filter_prepend(chain, filter) _crx_stream_filter_prepend((chain), (filter))
#define crx_stream_filter_append(chain, filter) _crx_stream_filter_append((chain), (filter))
#define crx_stream_filter_flush(filter, finish) _crx_stream_filter_flush((filter), (finish))

#define crx_stream_is_filtered(stream)	((stream)->readfilters.head || (stream)->writefilters.head)

typedef struct _crx_stream_filter_factory {
	crx_stream_filter *(*create_filter)(const char *filtername, zval *filterparams, uint8_t persistent);
} crx_stream_filter_factory;

BEGIN_EXTERN_C()
CRXAPI int crx_stream_filter_register_factory(const char *filterpattern, const crx_stream_filter_factory *factory);
CRXAPI int crx_stream_filter_unregister_factory(const char *filterpattern);
CRXAPI int crx_stream_filter_register_factory_volatile(crex_string *filterpattern, const crx_stream_filter_factory *factory);
CRXAPI crx_stream_filter *crx_stream_filter_create(const char *filtername, zval *filterparams, uint8_t persistent);
END_EXTERN_C()
