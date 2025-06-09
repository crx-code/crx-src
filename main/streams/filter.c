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
   | Authors: Wez Furlong <wez@thebrainroom.com>                          |
   +----------------------------------------------------------------------+
 */

#include "crx.h"
#include "crx_globals.h"
#include "crx_network.h"
#include "crx_open_temporary_file.h"
#include "ext/standard/file.h"
#include <stddef.h>
#include <fcntl.h>

#include "crx_streams_int.h"

/* Global filter hash, copied to FG(stream_filters) on registration of volatile filter */
static HashTable stream_filters_hash;

/* Should only be used during core initialization */
CRXAPI HashTable *crx_get_stream_filters_hash_global(void)
{
	return &stream_filters_hash;
}

/* Normal hash selection/retrieval call */
CRXAPI HashTable *_crx_get_stream_filters_hash(void)
{
	return (FG(stream_filters) ? FG(stream_filters) : &stream_filters_hash);
}

/* API for registering GLOBAL filters */
CRXAPI int crx_stream_filter_register_factory(const char *filterpattern, const crx_stream_filter_factory *factory)
{
	int ret;
	crex_string *str = crex_string_init_interned(filterpattern, strlen(filterpattern), 1);
	ret = crex_hash_add_ptr(&stream_filters_hash, str, (void*)factory) ? SUCCESS : FAILURE;
	crex_string_release_ex(str, 1);
	return ret;
}

CRXAPI int crx_stream_filter_unregister_factory(const char *filterpattern)
{
	return crex_hash_str_del(&stream_filters_hash, filterpattern, strlen(filterpattern));
}

/* API for registering VOLATILE wrappers */
CRXAPI int crx_stream_filter_register_factory_volatile(crex_string *filterpattern, const crx_stream_filter_factory *factory)
{
	if (!FG(stream_filters)) {
		ALLOC_HASHTABLE(FG(stream_filters));
		crex_hash_init(FG(stream_filters), crex_hash_num_elements(&stream_filters_hash) + 1, NULL, NULL, 0);
		crex_hash_copy(FG(stream_filters), &stream_filters_hash, NULL);
	}

	return crex_hash_add_ptr(FG(stream_filters), filterpattern, (void*)factory) ? SUCCESS : FAILURE;
}

/* Buckets */

CRXAPI crx_stream_bucket *crx_stream_bucket_new(crx_stream *stream, char *buf, size_t buflen, uint8_t own_buf, uint8_t buf_persistent)
{
	int is_persistent = crx_stream_is_persistent(stream);
	crx_stream_bucket *bucket;

	bucket = (crx_stream_bucket*)pemalloc(sizeof(crx_stream_bucket), is_persistent);
	bucket->next = bucket->prev = NULL;

	if (is_persistent && !buf_persistent) {
		/* all data in a persistent bucket must also be persistent */
		bucket->buf = pemalloc(buflen, 1);
		memcpy(bucket->buf, buf, buflen);
		bucket->buflen = buflen;
		bucket->own_buf = 1;
	} else {
		bucket->buf = buf;
		bucket->buflen = buflen;
		bucket->own_buf = own_buf;
	}
	bucket->is_persistent = is_persistent;
	bucket->refcount = 1;
	bucket->brigade = NULL;

	return bucket;
}

/* Given a bucket, returns a version of that bucket with a writeable buffer.
 * If the original bucket has a refcount of 1 and owns its buffer, then it
 * is returned unchanged.
 * Otherwise, a copy of the buffer is made.
 * In both cases, the original bucket is unlinked from its brigade.
 * If a copy is made, the original bucket is delref'd.
 * */
CRXAPI crx_stream_bucket *crx_stream_bucket_make_writeable(crx_stream_bucket *bucket)
{
	crx_stream_bucket *retval;

	crx_stream_bucket_unlink(bucket);

	if (bucket->refcount == 1 && bucket->own_buf) {
		return bucket;
	}

	retval = (crx_stream_bucket*)pemalloc(sizeof(crx_stream_bucket), bucket->is_persistent);
	memcpy(retval, bucket, sizeof(*retval));

	retval->buf = pemalloc(retval->buflen, retval->is_persistent);
	memcpy(retval->buf, bucket->buf, retval->buflen);

	retval->refcount = 1;
	retval->own_buf = 1;

	crx_stream_bucket_delref(bucket);

	return retval;
}

CRXAPI int crx_stream_bucket_split(crx_stream_bucket *in, crx_stream_bucket **left, crx_stream_bucket **right, size_t length)
{
	*left = (crx_stream_bucket*)pecalloc(1, sizeof(crx_stream_bucket), in->is_persistent);
	*right = (crx_stream_bucket*)pecalloc(1, sizeof(crx_stream_bucket), in->is_persistent);

	(*left)->buf = pemalloc(length, in->is_persistent);
	(*left)->buflen = length;
	memcpy((*left)->buf, in->buf, length);
	(*left)->refcount = 1;
	(*left)->own_buf = 1;
	(*left)->is_persistent = in->is_persistent;

	(*right)->buflen = in->buflen - length;
	(*right)->buf = pemalloc((*right)->buflen, in->is_persistent);
	memcpy((*right)->buf, in->buf + length, (*right)->buflen);
	(*right)->refcount = 1;
	(*right)->own_buf = 1;
	(*right)->is_persistent = in->is_persistent;

	return SUCCESS;
}

CRXAPI void crx_stream_bucket_delref(crx_stream_bucket *bucket)
{
	if (--bucket->refcount == 0) {
		if (bucket->own_buf) {
			pefree(bucket->buf, bucket->is_persistent);
		}
		pefree(bucket, bucket->is_persistent);
	}
}

CRXAPI void crx_stream_bucket_prepend(crx_stream_bucket_brigade *brigade, crx_stream_bucket *bucket)
{
	bucket->next = brigade->head;
	bucket->prev = NULL;

	if (brigade->head) {
		brigade->head->prev = bucket;
	} else {
		brigade->tail = bucket;
	}
	brigade->head = bucket;
	bucket->brigade = brigade;
}

CRXAPI void crx_stream_bucket_append(crx_stream_bucket_brigade *brigade, crx_stream_bucket *bucket)
{
	if (brigade->tail == bucket) {
		return;
	}

	bucket->prev = brigade->tail;
	bucket->next = NULL;

	if (brigade->tail) {
		brigade->tail->next = bucket;
	} else {
		brigade->head = bucket;
	}
	brigade->tail = bucket;
	bucket->brigade = brigade;
}

CRXAPI void crx_stream_bucket_unlink(crx_stream_bucket *bucket)
{
	if (bucket->prev) {
		bucket->prev->next = bucket->next;
	} else if (bucket->brigade) {
		bucket->brigade->head = bucket->next;
	}
	if (bucket->next) {
		bucket->next->prev = bucket->prev;
	} else if (bucket->brigade) {
		bucket->brigade->tail = bucket->prev;
	}
	bucket->brigade = NULL;
	bucket->next = bucket->prev = NULL;
}








/* We allow very simple pattern matching for filter factories:
 * if "convert.charset.utf-8/sjis" is requested, we search first for an exact
 * match. If that fails, we try "convert.charset.*", then "convert.*"
 * This means that we don't need to clog up the hashtable with a zillion
 * charsets (for example) but still be able to provide them all as filters */
CRXAPI crx_stream_filter *crx_stream_filter_create(const char *filtername, zval *filterparams, uint8_t persistent)
{
	HashTable *filter_hash = (FG(stream_filters) ? FG(stream_filters) : &stream_filters_hash);
	const crx_stream_filter_factory *factory = NULL;
	crx_stream_filter *filter = NULL;
	size_t n;
	char *period;

	n = strlen(filtername);

	if (NULL != (factory = crex_hash_str_find_ptr(filter_hash, filtername, n))) {
		filter = factory->create_filter(filtername, filterparams, persistent);
	} else if ((period = strrchr(filtername, '.'))) {
		/* try a wildcard */
		char *wildname;

		wildname = safe_emalloc(1, n, 3);
		memcpy(wildname, filtername, n+1);
		period = wildname + (period - filtername);
		while (period && !filter) {
			CREX_ASSERT(period[0] == '.');
			period[1] = '*';
			period[2] = '\0';
			if (NULL != (factory = crex_hash_str_find_ptr(filter_hash, wildname, strlen(wildname)))) {
				filter = factory->create_filter(filtername, filterparams, persistent);
			}

			*period = '\0';
			period = strrchr(wildname, '.');
		}
		efree(wildname);
	}

	if (filter == NULL) {
		/* TODO: these need correct docrefs */
		if (factory == NULL)
			crx_error_docref(NULL, E_WARNING, "Unable to locate filter \"%s\"", filtername);
		else
			crx_error_docref(NULL, E_WARNING, "Unable to create or locate filter \"%s\"", filtername);
	}

	return filter;
}

CRXAPI crx_stream_filter *_crx_stream_filter_alloc(const crx_stream_filter_ops *fops, void *abstract, uint8_t persistent STREAMS_DC)
{
	crx_stream_filter *filter;

	filter = (crx_stream_filter*) pemalloc_rel_orig(sizeof(crx_stream_filter), persistent);
	memset(filter, 0, sizeof(crx_stream_filter));

	filter->fops = fops;
	C_PTR(filter->abstract) = abstract;
	filter->is_persistent = persistent;

	return filter;
}

CRXAPI void crx_stream_filter_free(crx_stream_filter *filter)
{
	if (filter->fops->dtor)
		filter->fops->dtor(filter);
	pefree(filter, filter->is_persistent);
}

CRXAPI int crx_stream_filter_prepend_ex(crx_stream_filter_chain *chain, crx_stream_filter *filter)
{
	filter->next = chain->head;
	filter->prev = NULL;

	if (chain->head) {
		chain->head->prev = filter;
	} else {
		chain->tail = filter;
	}
	chain->head = filter;
	filter->chain = chain;

	return SUCCESS;
}

CRXAPI void _crx_stream_filter_prepend(crx_stream_filter_chain *chain, crx_stream_filter *filter)
{
	crx_stream_filter_prepend_ex(chain, filter);
}

CRXAPI int crx_stream_filter_append_ex(crx_stream_filter_chain *chain, crx_stream_filter *filter)
{
	crx_stream *stream = chain->stream;

	filter->prev = chain->tail;
	filter->next = NULL;
	if (chain->tail) {
		chain->tail->next = filter;
	} else {
		chain->head = filter;
	}
	chain->tail = filter;
	filter->chain = chain;

	if (&(stream->readfilters) == chain && (stream->writepos - stream->readpos) > 0) {
		/* Let's going ahead and wind anything in the buffer through this filter */
		crx_stream_bucket_brigade brig_in = { NULL, NULL }, brig_out = { NULL, NULL };
		crx_stream_bucket_brigade *brig_inp = &brig_in, *brig_outp = &brig_out;
		crx_stream_filter_status_t status;
		crx_stream_bucket *bucket;
		size_t consumed = 0;

		bucket = crx_stream_bucket_new(stream, (char*) stream->readbuf + stream->readpos, stream->writepos - stream->readpos, 0, 0);
		crx_stream_bucket_append(brig_inp, bucket);
		status = filter->fops->filter(stream, filter, brig_inp, brig_outp, &consumed, PSFS_FLAG_NORMAL);

		if (stream->readpos + consumed > (uint32_t)stream->writepos) {
			/* No behaving filter should cause this. */
			status = PSFS_ERR_FATAL;
		}

		switch (status) {
			case PSFS_ERR_FATAL:
				while (brig_in.head) {
					bucket = brig_in.head;
					crx_stream_bucket_unlink(bucket);
					crx_stream_bucket_delref(bucket);
				}
				while (brig_out.head) {
					bucket = brig_out.head;
					crx_stream_bucket_unlink(bucket);
					crx_stream_bucket_delref(bucket);
				}
				crx_error_docref(NULL, E_WARNING, "Filter failed to process pre-buffered data");
				return FAILURE;
			case PSFS_FEED_ME:
				/* We don't actually need data yet,
				   leave this filter in a feed me state until data is needed.
				   Reset stream's internal read buffer since the filter is "holding" it. */
				stream->readpos = 0;
				stream->writepos = 0;
				break;
			case PSFS_PASS_ON:
				/* If any data is consumed, we cannot rely upon the existing read buffer,
				   as the filtered data must replace the existing data, so invalidate the cache */
				stream->writepos = 0;
				stream->readpos = 0;

				while (brig_outp->head) {
					bucket = brig_outp->head;
					/* Grow buffer to hold this bucket if need be.
					   TODO: See warning in main/stream/streams.c::crx_stream_fill_read_buffer */
					if (stream->readbuflen - stream->writepos < bucket->buflen) {
						stream->readbuflen += bucket->buflen;
						stream->readbuf = perealloc(stream->readbuf, stream->readbuflen, stream->is_persistent);
					}
					memcpy(stream->readbuf + stream->writepos, bucket->buf, bucket->buflen);
					stream->writepos += bucket->buflen;

					crx_stream_bucket_unlink(bucket);
					crx_stream_bucket_delref(bucket);
				}
				break;
		}
	}

	return SUCCESS;
}

CRXAPI void _crx_stream_filter_append(crx_stream_filter_chain *chain, crx_stream_filter *filter)
{
	if (crx_stream_filter_append_ex(chain, filter) != SUCCESS) {
		if (chain->head == filter) {
			chain->head = NULL;
			chain->tail = NULL;
		} else {
			filter->prev->next = NULL;
			chain->tail = filter->prev;
		}
	}
}

CRXAPI int _crx_stream_filter_flush(crx_stream_filter *filter, int finish)
{
	crx_stream_bucket_brigade brig_a = { NULL, NULL }, brig_b = { NULL, NULL }, *inp = &brig_a, *outp = &brig_b, *brig_temp;
	crx_stream_bucket *bucket;
	crx_stream_filter_chain *chain;
	crx_stream_filter *current;
	crx_stream *stream;
	size_t flushed_size = 0;
	long flags = (finish ? PSFS_FLAG_FLUSH_CLOSE : PSFS_FLAG_FLUSH_INC);

	if (!filter->chain || !filter->chain->stream) {
		/* Filter is not attached to a chain, or chain is somehow not part of a stream */
		return FAILURE;
	}

	chain = filter->chain;
	stream = chain->stream;

	for(current = filter; current; current = current->next) {
		crx_stream_filter_status_t status;

		status = current->fops->filter(stream, current, inp, outp, NULL, flags);
		if (status == PSFS_FEED_ME) {
			/* We've flushed the data far enough */
			return SUCCESS;
		}
		if (status == PSFS_ERR_FATAL) {
			return FAILURE;
		}
		/* Otherwise we have data available to PASS_ON
			Swap the brigades and continue */
		brig_temp = inp;
		inp = outp;
		outp = brig_temp;
		outp->head = NULL;
		outp->tail = NULL;

		flags = PSFS_FLAG_NORMAL;
	}

	/* Last filter returned data via PSFS_PASS_ON
		Do something with it */

	for(bucket = inp->head; bucket; bucket = bucket->next) {
		flushed_size += bucket->buflen;
	}

	if (flushed_size == 0) {
		/* Unlikely, but possible */
		return SUCCESS;
	}

	if (chain == &(stream->readfilters)) {
		/* Dump any newly flushed data to the read buffer */
		if (stream->readpos > 0) {
			/* Back the buffer up */
			memcpy(stream->readbuf, stream->readbuf + stream->readpos, stream->writepos - stream->readpos);
			stream->readpos = 0;
			stream->writepos -= stream->readpos;
		}
		if (flushed_size > (stream->readbuflen - stream->writepos)) {
			/* Grow the buffer */
			stream->readbuf = perealloc(stream->readbuf, stream->writepos + flushed_size + stream->chunk_size, stream->is_persistent);
		}
		while ((bucket = inp->head)) {
			memcpy(stream->readbuf + stream->writepos, bucket->buf, bucket->buflen);
			stream->writepos += bucket->buflen;
			crx_stream_bucket_unlink(bucket);
			crx_stream_bucket_delref(bucket);
		}
	} else if (chain == &(stream->writefilters)) {
		/* Send flushed data to the stream */
		while ((bucket = inp->head)) {
			ssize_t count = stream->ops->write(stream, bucket->buf, bucket->buflen);
			if (count > 0) {
				stream->position += count;
			}
			crx_stream_bucket_unlink(bucket);
			crx_stream_bucket_delref(bucket);
		}
	}

	return SUCCESS;
}

CRXAPI crx_stream_filter *crx_stream_filter_remove(crx_stream_filter *filter, int call_dtor)
{
	if (filter->prev) {
		filter->prev->next = filter->next;
	} else {
		filter->chain->head = filter->next;
	}
	if (filter->next) {
		filter->next->prev = filter->prev;
	} else {
		filter->chain->tail = filter->prev;
	}

	if (filter->res) {
		crex_list_delete(filter->res);
	}

	if (call_dtor) {
		crx_stream_filter_free(filter);
		return NULL;
	}
	return filter;
}
