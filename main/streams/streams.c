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
   | Borrowed code from:                                                  |
   |          Rasmus Lerdorf <rasmus@lerdorf.on.ca>                       |
   |          Jim Winstead <jimw@crx.net>                                 |
   +----------------------------------------------------------------------+
 */

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include "crx.h"
#include "crx_globals.h"
#include "crx_memory_streams.h"
#include "crx_network.h"
#include "crx_open_temporary_file.h"
#include "ext/standard/file.h"
#include "ext/standard/basic_functions.h" /* for BG(CurrentStatFile) */
#include "ext/standard/crx_string.h" /* for crx_memnstr, used by crx_stream_get_record() */
#include <stddef.h>
#include <fcntl.h>
#include "crx_streams_int.h"

/* {{{ resource and registration code */
/* Global wrapper hash, copied to FG(stream_wrappers) on registration of volatile wrapper */
static HashTable url_stream_wrappers_hash;
static int le_stream = FAILURE; /* true global */
static int le_pstream = FAILURE; /* true global */
static int le_stream_filter = FAILURE; /* true global */

CRXAPI int crx_file_le_stream(void)
{
	return le_stream;
}

CRXAPI int crx_file_le_pstream(void)
{
	return le_pstream;
}

CRXAPI int crx_file_le_stream_filter(void)
{
	return le_stream_filter;
}

CRXAPI HashTable *_crx_stream_get_url_stream_wrappers_hash(void)
{
	return (FG(stream_wrappers) ? FG(stream_wrappers) : &url_stream_wrappers_hash);
}

CRXAPI HashTable *crx_stream_get_url_stream_wrappers_hash_global(void)
{
	return &url_stream_wrappers_hash;
}

static int forget_persistent_resource_id_numbers(zval *el)
{
	crx_stream *stream;
	crex_resource *rsrc = C_RES_P(el);

	if (rsrc->type != le_pstream) {
		return 0;
	}

	stream = (crx_stream*)rsrc->ptr;

#if STREAM_DEBUG
fprintf(stderr, "forget_persistent: %s:%p\n", stream->ops->label, stream);
#endif

	stream->res = NULL;

	if (stream->ctx) {
		crex_list_delete(stream->ctx);
		stream->ctx = NULL;
	}

	return 0;
}

CRX_RSHUTDOWN_FUNCTION(streams)
{
	zval *el;

	CREX_HASH_FOREACH_VAL(&EG(persistent_list), el) {
		forget_persistent_resource_id_numbers(el);
	} CREX_HASH_FOREACH_END();
	return SUCCESS;
}

CRXAPI crx_stream *crx_stream_encloses(crx_stream *enclosing, crx_stream *enclosed)
{
	crx_stream *orig = enclosed->enclosing_stream;

	crx_stream_auto_cleanup(enclosed);
	enclosed->enclosing_stream = enclosing;
	return orig;
}

CRXAPI int crx_stream_from_persistent_id(const char *persistent_id, crx_stream **stream)
{
	crex_resource *le;

	if ((le = crex_hash_str_find_ptr(&EG(persistent_list), persistent_id, strlen(persistent_id))) != NULL) {
		if (le->type == le_pstream) {
			if (stream) {
				crex_resource *regentry = NULL;

				/* see if this persistent resource already has been loaded to the
				 * regular list; allowing the same resource in several entries in the
				 * regular list causes trouble (see bug #54623) */
				*stream = (crx_stream*)le->ptr;
				CREX_HASH_FOREACH_PTR(&EG(regular_list), regentry) {
					if (regentry->ptr == le->ptr) {
						GC_ADDREF(regentry);
						(*stream)->res = regentry;
						return CRX_STREAM_PERSISTENT_SUCCESS;
					}
				} CREX_HASH_FOREACH_END();
				GC_ADDREF(le);
				(*stream)->res = crex_register_resource(*stream, le_pstream);
			}
			return CRX_STREAM_PERSISTENT_SUCCESS;
		}
		return CRX_STREAM_PERSISTENT_FAILURE;
	}
	return CRX_STREAM_PERSISTENT_NOT_EXIST;
}

/* }}} */

static crex_llist *crx_get_wrapper_errors_list(crx_stream_wrapper *wrapper)
{
	if (!FG(wrapper_errors)) {
		return NULL;
	} else {
		return (crex_llist*) crex_hash_str_find_ptr(FG(wrapper_errors), (const char*)&wrapper, sizeof(wrapper));
	}
}

/* {{{ wrapper error reporting */
static void crx_stream_display_wrapper_errors(crx_stream_wrapper *wrapper, const char *path, const char *caption)
{
	char *tmp;
	char *msg;
	int free_msg = 0;

	if (EG(exception)) {
		/* Don't emit additional warnings if an exception has already been thrown. */
		return;
	}

	tmp = estrdup(path);
	if (wrapper) {
		crex_llist *err_list = crx_get_wrapper_errors_list(wrapper);
		if (err_list) {
			size_t l = 0;
			int brlen;
			int i;
			int count = (int)crex_llist_count(err_list);
			const char *br;
			const char **err_buf_p;
			crex_llist_position pos;

			if (PG(html_errors)) {
				brlen = 7;
				br = "<br />\n";
			} else {
				brlen = 1;
				br = "\n";
			}

			for (err_buf_p = crex_llist_get_first_ex(err_list, &pos), i = 0;
					err_buf_p;
					err_buf_p = crex_llist_get_next_ex(err_list, &pos), i++) {
				l += strlen(*err_buf_p);
				if (i < count - 1) {
					l += brlen;
				}
			}
			msg = emalloc(l + 1);
			msg[0] = '\0';
			for (err_buf_p = crex_llist_get_first_ex(err_list, &pos), i = 0;
					err_buf_p;
					err_buf_p = crex_llist_get_next_ex(err_list, &pos), i++) {
				strcat(msg, *err_buf_p);
				if (i < count - 1) {
					strcat(msg, br);
				}
			}

			free_msg = 1;
		} else {
			if (wrapper == &crx_plain_files_wrapper) {
				msg = strerror(errno); /* TODO: not ts on linux */
			} else {
				msg = "operation failed";
			}
		}
	} else {
		msg = "no suitable wrapper could be found";
	}

	crx_strip_url_passwd(tmp);
	crx_error_docref1(NULL, tmp, E_WARNING, "%s: %s", caption, msg);
	efree(tmp);
	if (free_msg) {
		efree(msg);
	}
}

static void crx_stream_tidy_wrapper_error_log(crx_stream_wrapper *wrapper)
{
	if (wrapper && FG(wrapper_errors)) {
		crex_hash_str_del(FG(wrapper_errors), (const char*)&wrapper, sizeof(wrapper));
	}
}

static void wrapper_error_dtor(void *error)
{
	efree(*(char**)error);
}

static void wrapper_list_dtor(zval *item) {
	crex_llist *list = (crex_llist*)C_PTR_P(item);
	crex_llist_destroy(list);
	efree(list);
}

CRXAPI void crx_stream_wrapper_log_error(const crx_stream_wrapper *wrapper, int options, const char *fmt, ...)
{
	va_list args;
	char *buffer = NULL;

	va_start(args, fmt);
	vspprintf(&buffer, 0, fmt, args);
	va_end(args);

	if ((options & REPORT_ERRORS) || wrapper == NULL) {
		crx_error_docref(NULL, E_WARNING, "%s", buffer);
		efree(buffer);
	} else {
		crex_llist *list = NULL;
		if (!FG(wrapper_errors)) {
			ALLOC_HASHTABLE(FG(wrapper_errors));
			crex_hash_init(FG(wrapper_errors), 8, NULL, wrapper_list_dtor, 0);
		} else {
			list = crex_hash_str_find_ptr(FG(wrapper_errors), (const char*)&wrapper, sizeof(wrapper));
		}

		if (!list) {
			crex_llist new_list;
			crex_llist_init(&new_list, sizeof(buffer), wrapper_error_dtor, 0);
			list = crex_hash_str_update_mem(FG(wrapper_errors), (const char*)&wrapper,
					sizeof(wrapper), &new_list, sizeof(new_list));
		}

		/* append to linked list */
		crex_llist_add_element(list, &buffer);
	}
}


/* }}} */

/* allocate a new stream for a particular ops */
CRXAPI crx_stream *_crx_stream_alloc(const crx_stream_ops *ops, void *abstract, const char *persistent_id, const char *mode STREAMS_DC) /* {{{ */
{
	crx_stream *ret;

	ret = (crx_stream*) pemalloc_rel_orig(sizeof(crx_stream), persistent_id ? 1 : 0);

	memset(ret, 0, sizeof(crx_stream));

	ret->readfilters.stream = ret;
	ret->writefilters.stream = ret;

#if STREAM_DEBUG
fprintf(stderr, "stream_alloc: %s:%p persistent=%s\n", ops->label, ret, persistent_id);
#endif

	ret->ops = ops;
	ret->abstract = abstract;
	ret->is_persistent = persistent_id ? 1 : 0;
	ret->chunk_size = FG(def_chunk_size);

#if CREX_DEBUG
	ret->open_filename = __crex_orig_filename ? __crex_orig_filename : __crex_filename;
	ret->open_lineno = __crex_orig_lineno ? __crex_orig_lineno : __crex_lineno;
#endif

	if (FG(auto_detect_line_endings)) {
		ret->flags |= CRX_STREAM_FLAG_DETECT_EOL;
	}

	if (persistent_id) {
		if (NULL == crex_register_persistent_resource(persistent_id, strlen(persistent_id), ret, le_pstream)) {
			pefree(ret, 1);
			return NULL;
		}
	}

	ret->res = crex_register_resource(ret, persistent_id ? le_pstream : le_stream);
	strlcpy(ret->mode, mode, sizeof(ret->mode));

	ret->wrapper          = NULL;
	ret->wrapperthis      = NULL;
	ZVAL_UNDEF(&ret->wrapperdata);
	ret->stdiocast        = NULL;
	ret->orig_path        = NULL;
	ret->ctx              = NULL;
	ret->readbuf          = NULL;
	ret->enclosing_stream = NULL;

	return ret;
}
/* }}} */

CRXAPI int _crx_stream_free_enclosed(crx_stream *stream_enclosed, int close_options) /* {{{ */
{
	return crx_stream_free(stream_enclosed,
		close_options | CRX_STREAM_FREE_IGNORE_ENCLOSING);
}
/* }}} */

#if STREAM_DEBUG
static const char *_crx_stream_pretty_free_options(int close_options, char *out)
{
	if (close_options & CRX_STREAM_FREE_CALL_DTOR)
		strcat(out, "CALL_DTOR, ");
	if (close_options & CRX_STREAM_FREE_RELEASE_STREAM)
		strcat(out, "RELEASE_STREAM, ");
	if (close_options & CRX_STREAM_FREE_PRESERVE_HANDLE)
		strcat(out, "PRESERVE_HANDLE, ");
	if (close_options & CRX_STREAM_FREE_RSRC_DTOR)
		strcat(out, "RSRC_DTOR, ");
	if (close_options & CRX_STREAM_FREE_PERSISTENT)
		strcat(out, "PERSISTENT, ");
	if (close_options & CRX_STREAM_FREE_IGNORE_ENCLOSING)
		strcat(out, "IGNORE_ENCLOSING, ");
	if (out[0] != '\0')
		out[strlen(out) - 2] = '\0';
	return out;
}
#endif

static int _crx_stream_free_persistent(zval *zv, void *pStream)
{
	crex_resource *le = C_RES_P(zv);
	return le->ptr == pStream;
}


CRXAPI int _crx_stream_free(crx_stream *stream, int close_options) /* {{{ */
{
	int ret = 1;
	int preserve_handle = close_options & CRX_STREAM_FREE_PRESERVE_HANDLE ? 1 : 0;
	int release_cast = 1;
	crx_stream_context *context;

	/* During shutdown resources may be released before other resources still holding them.
	 * When only resources are referenced this is not a problem, because they are refcounted
	 * and will only be fully freed once the refcount drops to zero. However, if crx_stream*
	 * is held directly, we don't have this guarantee. To avoid use-after-free we ignore all
	 * stream free operations in shutdown unless they come from the resource list destruction,
	 * or by freeing an enclosed stream (in which case resource list destruction will not have
	 * freed it). */
	if ((EG(flags) & EG_FLAGS_IN_RESOURCE_SHUTDOWN) &&
			!(close_options & (CRX_STREAM_FREE_RSRC_DTOR|CRX_STREAM_FREE_IGNORE_ENCLOSING))) {
		return 1;
	}

	context = CRX_STREAM_CONTEXT(stream);

	if ((stream->flags & CRX_STREAM_FLAG_NO_CLOSE) ||
			((stream->flags & CRX_STREAM_FLAG_NO_RSCR_DTOR_CLOSE) && (close_options & CRX_STREAM_FREE_RSRC_DTOR))) {
		preserve_handle = 1;
	}

#if STREAM_DEBUG
	{
		char out[200] = "";
		fprintf(stderr, "stream_free: %s:%p[%s] in_free=%d opts=%s\n",
			stream->ops->label, stream, stream->orig_path, stream->in_free, _crx_stream_pretty_free_options(close_options, out));
	}

#endif

	if (stream->in_free) {
		/* hopefully called recursively from the enclosing stream; the pointer was NULLed below */
		if ((stream->in_free == 1) && (close_options & CRX_STREAM_FREE_IGNORE_ENCLOSING) && (stream->enclosing_stream == NULL)) {
			close_options |= CRX_STREAM_FREE_RSRC_DTOR; /* restore flag */
		} else {
			return 1; /* recursion protection */
		}
	}

	stream->in_free++;

	/* force correct order on enclosing/enclosed stream destruction (only from resource
	 * destructor as in when reverse destroying the resource list) */
	if ((close_options & CRX_STREAM_FREE_RSRC_DTOR) &&
			!(close_options & CRX_STREAM_FREE_IGNORE_ENCLOSING) &&
			(close_options & (CRX_STREAM_FREE_CALL_DTOR | CRX_STREAM_FREE_RELEASE_STREAM)) && /* always? */
			(stream->enclosing_stream != NULL)) {
		crx_stream *enclosing_stream = stream->enclosing_stream;
		stream->enclosing_stream = NULL;
		/* we force CRX_STREAM_CALL_DTOR because that's from where the
		 * enclosing stream can free this stream. */
		return crx_stream_free(enclosing_stream,
			(close_options | CRX_STREAM_FREE_CALL_DTOR | CRX_STREAM_FREE_KEEP_RSRC) & ~CRX_STREAM_FREE_RSRC_DTOR);
	}

	/* if we are releasing the stream only (and preserving the underlying handle),
	 * we need to do things a little differently.
	 * We are only ever called like this when the stream is cast to a FILE*
	 * for include (or other similar) purposes.
	 * */
	if (preserve_handle) {
		if (stream->fclose_stdiocast == CRX_STREAM_FCLOSE_FOPENCOOKIE) {
			/* If the stream was fopencookied, we must NOT touch anything
			 * here, as the cookied stream relies on it all.
			 * Instead, mark the stream as OK to auto-clean */
			crx_stream_auto_cleanup(stream);
			stream->in_free--;
			return 0;
		}
		/* otherwise, make sure that we don't close the FILE* from a cast */
		release_cast = 0;
	}

#if STREAM_DEBUG
fprintf(stderr, "stream_free: %s:%p[%s] preserve_handle=%d release_cast=%d remove_rsrc=%d\n",
		stream->ops->label, stream, stream->orig_path, preserve_handle, release_cast,
		(close_options & CRX_STREAM_FREE_RSRC_DTOR) == 0);
#endif

	if (stream->flags & CRX_STREAM_FLAG_WAS_WRITTEN || stream->writefilters.head) {
		/* make sure everything is saved */
		_crx_stream_flush(stream, 1);
	}

	/* If not called from the resource dtor, remove the stream from the resource list. */
	if ((close_options & CRX_STREAM_FREE_RSRC_DTOR) == 0 && stream->res) {
		/* Close resource, but keep it in resource list */
		crex_list_close(stream->res);
		if ((close_options & CRX_STREAM_FREE_KEEP_RSRC) == 0) {
			/* Completely delete crex_resource, if not referenced */
			crex_list_delete(stream->res);
			stream->res = NULL;
		}
	}

	if (close_options & CRX_STREAM_FREE_CALL_DTOR) {
		if (release_cast && stream->fclose_stdiocast == CRX_STREAM_FCLOSE_FOPENCOOKIE) {
			/* calling fclose on an fopencookied stream will ultimately
				call this very same function.  If we were called via fclose,
				the cookie_closer unsets the fclose_stdiocast flags, so
				we can be sure that we only reach here when CRX code calls
				crx_stream_free.
				Let's let the cookie code clean it all up.
			 */
			stream->in_free = 0;
			return fclose(stream->stdiocast);
		}

		ret = stream->ops->close(stream, preserve_handle ? 0 : 1);
		stream->abstract = NULL;

		/* tidy up any FILE* that might have been fdopened */
		if (release_cast && stream->fclose_stdiocast == CRX_STREAM_FCLOSE_FDOPEN && stream->stdiocast) {
			fclose(stream->stdiocast);
			stream->stdiocast = NULL;
			stream->fclose_stdiocast = CRX_STREAM_FCLOSE_NONE;
		}
	}

	if (close_options & CRX_STREAM_FREE_RELEASE_STREAM) {
		while (stream->readfilters.head) {
			if (stream->readfilters.head->res != NULL) {
				crex_list_close(stream->readfilters.head->res);
			}
			crx_stream_filter_remove(stream->readfilters.head, 1);
		}
		while (stream->writefilters.head) {
			if (stream->writefilters.head->res != NULL) {
				crex_list_close(stream->writefilters.head->res);
			}
			crx_stream_filter_remove(stream->writefilters.head, 1);
		}

		if (stream->wrapper && stream->wrapper->wops && stream->wrapper->wops->stream_closer) {
			stream->wrapper->wops->stream_closer(stream->wrapper, stream);
			stream->wrapper = NULL;
		}

		if (C_TYPE(stream->wrapperdata) != IS_UNDEF) {
			zval_ptr_dtor(&stream->wrapperdata);
			ZVAL_UNDEF(&stream->wrapperdata);
		}

		if (stream->readbuf) {
			pefree(stream->readbuf, stream->is_persistent);
			stream->readbuf = NULL;
		}

		if (stream->is_persistent && (close_options & CRX_STREAM_FREE_PERSISTENT)) {
			/* we don't work with *stream but need its value for comparison */
			crex_hash_apply_with_argument(&EG(persistent_list), _crx_stream_free_persistent, stream);
		}

		if (stream->orig_path) {
			pefree(stream->orig_path, stream->is_persistent);
			stream->orig_path = NULL;
		}

		pefree(stream, stream->is_persistent);
	}

	if (context) {
		crex_list_delete(context->res);
	}

	return ret;
}
/* }}} */

/* {{{ generic stream operations */

CRXAPI crex_result _crx_stream_fill_read_buffer(crx_stream *stream, size_t size)
{
	/* allocate/fill the buffer */

	crex_result retval;
	bool old_eof = stream->eof;

	if (stream->readfilters.head) {
		size_t to_read_now = MIN(size, stream->chunk_size);
		char *chunk_buf;
		crx_stream_bucket_brigade brig_in = { NULL, NULL }, brig_out = { NULL, NULL };
		crx_stream_bucket_brigade *brig_inp = &brig_in, *brig_outp = &brig_out, *brig_swap;

		/* allocate a buffer for reading chunks */
		chunk_buf = emalloc(stream->chunk_size);

		while (!stream->eof && (stream->writepos - stream->readpos < (crex_off_t)to_read_now)) {
			ssize_t justread = 0;
			int flags;
			crx_stream_bucket *bucket;
			crx_stream_filter_status_t status = PSFS_ERR_FATAL;
			crx_stream_filter *filter;

			/* read a chunk into a bucket */
			justread = stream->ops->read(stream, chunk_buf, stream->chunk_size);
			if (justread < 0 && stream->writepos == stream->readpos) {
				efree(chunk_buf);
				retval = FAILURE;
				goto out_check_eof;
			} else if (justread > 0) {
				bucket = crx_stream_bucket_new(stream, chunk_buf, justread, 0, 0);

				/* after this call, bucket is owned by the brigade */
				crx_stream_bucket_append(brig_inp, bucket);

				flags = stream->eof ? PSFS_FLAG_FLUSH_CLOSE : PSFS_FLAG_NORMAL;
			} else {
				flags = stream->eof ? PSFS_FLAG_FLUSH_CLOSE : PSFS_FLAG_FLUSH_INC;
			}

			/* wind the handle... */
			for (filter = stream->readfilters.head; filter; filter = filter->next) {
				status = filter->fops->filter(stream, filter, brig_inp, brig_outp, NULL, flags);

				if (status != PSFS_PASS_ON) {
					break;
				}

				/* brig_out becomes brig_in.
				 * brig_in will always be empty here, as the filter MUST attach any un-consumed buckets
				 * to its own brigade */
				brig_swap = brig_inp;
				brig_inp = brig_outp;
				brig_outp = brig_swap;
				memset(brig_outp, 0, sizeof(*brig_outp));
			}

			switch (status) {
				case PSFS_PASS_ON:
					/* we get here when the last filter in the chain has data to pass on.
					 * in this situation, we are passing the brig_in brigade into the
					 * stream read buffer */
					while (brig_inp->head) {
						bucket = brig_inp->head;
						/* reduce buffer memory consumption if possible, to avoid a realloc */
						if (stream->readbuf && stream->readbuflen - stream->writepos < bucket->buflen) {
							if (stream->writepos > stream->readpos) {
								memmove(stream->readbuf, stream->readbuf + stream->readpos, stream->writepos - stream->readpos);
							}
							stream->writepos -= stream->readpos;
							stream->readpos = 0;
						}
						/* grow buffer to hold this bucket */
						if (stream->readbuflen - stream->writepos < bucket->buflen) {
							stream->readbuflen += bucket->buflen;
							stream->readbuf = perealloc(stream->readbuf, stream->readbuflen,
									stream->is_persistent);
						}
						if (bucket->buflen) {
							memcpy(stream->readbuf + stream->writepos, bucket->buf, bucket->buflen);
						}
						stream->writepos += bucket->buflen;

						crx_stream_bucket_unlink(bucket);
						crx_stream_bucket_delref(bucket);
					}
					break;

				case PSFS_FEED_ME:
					/* when a filter needs feeding, there is no brig_out to deal with.
					 * we simply continue the loop; if the caller needs more data,
					 * we will read again, otherwise out job is done here */
					break;

				case PSFS_ERR_FATAL:
					/* some fatal error. Theoretically, the stream is borked, so all
					 * further reads should fail. */
					stream->eof = 1;
					efree(chunk_buf);
					retval = FAILURE;
					goto out_is_eof;
			}

			if (justread <= 0) {
				break;
			}
		}

		efree(chunk_buf);
		return SUCCESS;
	} else {
		/* is there enough data in the buffer ? */
		if (stream->writepos - stream->readpos < (crex_off_t)size) {
			ssize_t justread = 0;

			/* reduce buffer memory consumption if possible, to avoid a realloc */
			if (stream->readbuf && stream->readbuflen - stream->writepos < stream->chunk_size) {
				if (stream->writepos > stream->readpos) {
					memmove(stream->readbuf, stream->readbuf + stream->readpos, stream->writepos - stream->readpos);
				}
				stream->writepos -= stream->readpos;
				stream->readpos = 0;
			}

			/* grow the buffer if required
			 * TODO: this can fail for persistent streams */
			if (stream->readbuflen - stream->writepos < stream->chunk_size) {
				stream->readbuflen += stream->chunk_size;
				stream->readbuf = perealloc(stream->readbuf, stream->readbuflen,
						stream->is_persistent);
			}

			justread = stream->ops->read(stream, (char*)stream->readbuf + stream->writepos,
					stream->readbuflen - stream->writepos
					);
			if (justread < 0) {
				retval = FAILURE;
				goto out_check_eof;
			}
			stream->writepos += justread;
			retval = SUCCESS;
			goto out_check_eof;
		}
		return SUCCESS;
	}

out_check_eof:
	if (old_eof != stream->eof) {
out_is_eof:
		crx_stream_notify_completed(CRX_STREAM_CONTEXT(stream));
	}
	return retval;
}

CRXAPI ssize_t _crx_stream_read(crx_stream *stream, char *buf, size_t size)
{
	ssize_t toread = 0, didread = 0;

	while (size > 0) {

		/* take from the read buffer first.
		 * It is possible that a buffered stream was switched to non-buffered, so we
		 * drain the remainder of the buffer before using the "raw" read mode for
		 * the excess */
		if (stream->writepos > stream->readpos) {

			toread = stream->writepos - stream->readpos;
			if (toread > size) {
				toread = size;
			}

			memcpy(buf, stream->readbuf + stream->readpos, toread);
			stream->readpos += toread;
			size -= toread;
			buf += toread;
			didread += toread;
			stream->has_buffered_data = 1;
		}

		/* ignore eof here; the underlying state might have changed */
		if (size == 0) {
			break;
		}

		if (!stream->readfilters.head && ((stream->flags & CRX_STREAM_FLAG_NO_BUFFER) || stream->chunk_size == 1)) {
			toread = stream->ops->read(stream, buf, size);
			if (toread < 0) {
				/* Report an error if the read failed and we did not read any data
				 * before that. Otherwise return the data we did read. */
				if (didread == 0) {
					return toread;
				}
				break;
			}
		} else {
			if (crx_stream_fill_read_buffer(stream, size) != SUCCESS) {
				if (didread == 0) {
					return -1;
				}
				break;
			}

			toread = stream->writepos - stream->readpos;
			if ((size_t) toread > size) {
				toread = size;
			}

			if (toread > 0) {
				memcpy(buf, stream->readbuf + stream->readpos, toread);
				stream->readpos += toread;
			}
		}
		if (toread > 0) {
			didread += toread;
			buf += toread;
			size -= toread;
			stream->has_buffered_data = 1;
		} else {
			/* EOF, or temporary end of data (for non-blocking mode). */
			break;
		}

		/* just break anyway, to avoid greedy read for file://, crx://memory, and crx://temp */
		if ((stream->wrapper != &crx_plain_files_wrapper) &&
			(stream->ops != &crx_stream_memory_ops) &&
			(stream->ops != &crx_stream_temp_ops)) {
			break;
		}
	}

	if (didread > 0) {
		stream->position += didread;
		stream->has_buffered_data = 0;
	}

	return didread;
}

/* Like crx_stream_read(), but reading into a crex_string buffer. This has some similarity
 * to the copy_to_mem() operation, but only performs a single direct read. */
CRXAPI crex_string *crx_stream_read_to_str(crx_stream *stream, size_t len)
{
	crex_string *str = crex_string_alloc(len, 0);
	ssize_t read = crx_stream_read(stream, ZSTR_VAL(str), len);
	if (read < 0) {
		crex_string_efree(str);
		return NULL;
	}

	ZSTR_LEN(str) = read;
	ZSTR_VAL(str)[read] = 0;

	if ((size_t) read < len / 2) {
		return crex_string_truncate(str, read, 0);
	}
	return str;
}

CRXAPI bool _crx_stream_eof(crx_stream *stream)
{
	/* if there is data in the buffer, it's not EOF */
	if (stream->writepos - stream->readpos > 0) {
		return 0;
	}

	/* use the configured timeout when checking eof */
	if (!stream->eof && CRX_STREAM_OPTION_RETURN_ERR ==
		   	crx_stream_set_option(stream, CRX_STREAM_OPTION_CHECK_LIVENESS,
		   	0, NULL)) {
		stream->eof = 1;
	}

	return stream->eof;
}

CRXAPI int _crx_stream_putc(crx_stream *stream, int c)
{
	unsigned char buf = c;

	if (crx_stream_write(stream, (char*)&buf, 1) > 0) {
		return 1;
	}
	return EOF;
}

CRXAPI int _crx_stream_getc(crx_stream *stream)
{
	char buf;

	if (crx_stream_read(stream, &buf, 1) > 0) {
		return buf & 0xff;
	}
	return EOF;
}

CRXAPI bool _crx_stream_puts(crx_stream *stream, const char *buf)
{
	size_t len;
	char newline[2] = "\n"; /* is this OK for Win? */
	len = strlen(buf);

	if (len > 0 && crx_stream_write(stream, buf, len) > 0 && crx_stream_write(stream, newline, 1) > 0) {
		return 1;
	}
	return 0;
}

CRXAPI int _crx_stream_stat(crx_stream *stream, crx_stream_statbuf *ssb)
{
	memset(ssb, 0, sizeof(*ssb));

	/* if the stream was wrapped, allow the wrapper to stat it */
	if (stream->wrapper && stream->wrapper->wops->stream_stat != NULL) {
		return stream->wrapper->wops->stream_stat(stream->wrapper, stream, ssb);
	}

	/* if the stream doesn't directly support stat-ing, return with failure.
	 * We could try and emulate this by casting to an FD and fstat-ing it,
	 * but since the fd might not represent the actual underlying content
	 * this would give bogus results. */
	if (stream->ops->stat == NULL) {
		return -1;
	}

	return (stream->ops->stat)(stream, ssb);
}

CRXAPI const char *crx_stream_locate_eol(crx_stream *stream, crex_string *buf)
{
	size_t avail;
	const char *cr, *lf, *eol = NULL;
	const char *readptr;

	if (!buf) {
		readptr = (char*)stream->readbuf + stream->readpos;
		avail = stream->writepos - stream->readpos;
	} else {
		readptr = ZSTR_VAL(buf);
		avail = ZSTR_LEN(buf);
	}

	/* Look for EOL */
	if (stream->flags & CRX_STREAM_FLAG_DETECT_EOL) {
		cr = memchr(readptr, '\r', avail);
		lf = memchr(readptr, '\n', avail);

		if (cr && lf != cr + 1 && !(lf && lf < cr)) {
			/* mac */
			stream->flags ^= CRX_STREAM_FLAG_DETECT_EOL;
			stream->flags |= CRX_STREAM_FLAG_EOL_MAC;
			eol = cr;
		} else if ((cr && lf && cr == lf - 1) || (lf)) {
			/* dos or unix endings */
			stream->flags ^= CRX_STREAM_FLAG_DETECT_EOL;
			eol = lf;
		}
	} else if (stream->flags & CRX_STREAM_FLAG_EOL_MAC) {
		eol = memchr(readptr, '\r', avail);
	} else {
		/* unix (and dos) line endings */
		eol = memchr(readptr, '\n', avail);
	}

	return eol;
}

/* If buf == NULL, the buffer will be allocated automatically and will be of an
 * appropriate length to hold the line, regardless of the line length, memory
 * permitting */
CRXAPI char *_crx_stream_get_line(crx_stream *stream, char *buf, size_t maxlen,
		size_t *returned_len)
{
	size_t avail = 0;
	size_t current_buf_size = 0;
	size_t total_copied = 0;
	int grow_mode = 0;
	char *bufstart = buf;

	if (buf == NULL) {
		grow_mode = 1;
	} else if (maxlen == 0) {
		return NULL;
	}

	/*
	 * If the underlying stream operations block when no new data is readable,
	 * we need to take extra precautions.
	 *
	 * If there is buffered data available, we check for a EOL. If it exists,
	 * we pass the data immediately back to the caller. This saves a call
	 * to the read implementation and will not block where blocking
	 * is not necessary at all.
	 *
	 * If the stream buffer contains more data than the caller requested,
	 * we can also avoid that costly step and simply return that data.
	 */

	for (;;) {
		avail = stream->writepos - stream->readpos;

		if (avail > 0) {
			size_t cpysz = 0;
			char *readptr;
			const char *eol;
			int done = 0;

			readptr = (char*)stream->readbuf + stream->readpos;
			eol = crx_stream_locate_eol(stream, NULL);

			if (eol) {
				cpysz = eol - readptr + 1;
				done = 1;
			} else {
				cpysz = avail;
			}

			if (grow_mode) {
				/* allow room for a NUL. If this realloc is really a realloc
				 * (ie: second time around), we get an extra byte. In most
				 * cases, with the default chunk size of 8K, we will only
				 * incur that overhead once.  When people have lines longer
				 * than 8K, we waste 1 byte per additional 8K or so.
				 * That seems acceptable to me, to avoid making this code
				 * hard to follow */
				bufstart = erealloc(bufstart, current_buf_size + cpysz + 1);
				current_buf_size += cpysz + 1;
				buf = bufstart + total_copied;
			} else {
				if (cpysz >= maxlen - 1) {
					cpysz = maxlen - 1;
					done = 1;
				}
			}

			memcpy(buf, readptr, cpysz);

			stream->position += cpysz;
			stream->readpos += cpysz;
			buf += cpysz;
			maxlen -= cpysz;
			total_copied += cpysz;

			if (done) {
				break;
			}
		} else if (stream->eof) {
			break;
		} else {
			/* XXX: Should be fine to always read chunk_size */
			size_t toread;

			if (grow_mode) {
				toread = stream->chunk_size;
			} else {
				toread = maxlen - 1;
				if (toread > stream->chunk_size) {
					toread = stream->chunk_size;
				}
			}

			crx_stream_fill_read_buffer(stream, toread);

			if (stream->writepos - stream->readpos == 0) {
				break;
			}
		}
	}

	if (total_copied == 0) {
		if (grow_mode) {
			assert(bufstart == NULL);
		}
		return NULL;
	}

	buf[0] = '\0';
	if (returned_len) {
		*returned_len = total_copied;
	}

	return bufstart;
}

#define STREAM_BUFFERED_AMOUNT(stream) \
	((size_t)(((stream)->writepos) - (stream)->readpos))

static const char *_crx_stream_search_delim(crx_stream *stream,
											size_t maxlen,
											size_t skiplen,
											const char *delim, /* non-empty! */
											size_t delim_len)
{
	size_t	seek_len;

	/* set the maximum number of bytes we're allowed to read from buffer */
	seek_len = MIN(STREAM_BUFFERED_AMOUNT(stream), maxlen);
	if (seek_len <= skiplen) {
		return NULL;
	}

	if (delim_len == 1) {
		return memchr(&stream->readbuf[stream->readpos + skiplen],
			delim[0], seek_len - skiplen);
	} else {
		return crx_memnstr((char*)&stream->readbuf[stream->readpos + skiplen],
				delim, delim_len,
				(char*)&stream->readbuf[stream->readpos + seek_len]);
	}
}

CRXAPI crex_string *crx_stream_get_record(crx_stream *stream, size_t maxlen, const char *delim, size_t delim_len)
{
	crex_string	*ret_buf;				/* returned buffer */
	const char *found_delim = NULL;
	size_t	buffered_len,
			tent_ret_len;			/* tentative returned length */
	bool	has_delim = delim_len > 0;

	if (maxlen == 0) {
		return NULL;
	}

	if (has_delim) {
		found_delim = _crx_stream_search_delim(
			stream, maxlen, 0, delim, delim_len);
	}

	buffered_len = STREAM_BUFFERED_AMOUNT(stream);
	/* try to read up to maxlen length bytes while we don't find the delim */
	while (!found_delim && buffered_len < maxlen) {
		size_t	just_read,
				to_read_now;

		to_read_now = MIN(maxlen - buffered_len, stream->chunk_size);

		crx_stream_fill_read_buffer(stream, buffered_len + to_read_now);

		just_read = STREAM_BUFFERED_AMOUNT(stream) - buffered_len;

		/* Assume the stream is temporarily or permanently out of data */
		if (just_read == 0) {
			break;
		}

		if (has_delim) {
			/* search for delimiter, but skip buffered_len (the number of bytes
			 * buffered before this loop iteration), as they have already been
			 * searched for the delimiter.
			 * The left part of the delimiter may still remain in the buffer,
			 * so subtract up to <delim_len - 1> from buffered_len, which is
			 * the amount of data we skip on this search  as an optimization
			 */
			found_delim = _crx_stream_search_delim(
				stream, maxlen,
				buffered_len >= (delim_len - 1)
						? buffered_len - (delim_len - 1)
						: 0,
				delim, delim_len);
			if (found_delim) {
				break;
			}
		}
		buffered_len += just_read;
	}

	if (has_delim && found_delim) {
		tent_ret_len = found_delim - (char*)&stream->readbuf[stream->readpos];
	} else if (!has_delim && STREAM_BUFFERED_AMOUNT(stream) >= maxlen) {
		tent_ret_len = maxlen;
	} else {
		/* return with error if the delimiter string (if any) was not found, we
		 * could not completely fill the read buffer with maxlen bytes and we
		 * don't know we've reached end of file. Added with non-blocking streams
		 * in mind, where this situation is frequent */
		if (STREAM_BUFFERED_AMOUNT(stream) < maxlen && !stream->eof) {
			return NULL;
		} else if (STREAM_BUFFERED_AMOUNT(stream) == 0 && stream->eof) {
			/* refuse to return an empty string just because by accident
			 * we knew of EOF in a read that returned no data */
			return NULL;
		} else {
			tent_ret_len = MIN(STREAM_BUFFERED_AMOUNT(stream), maxlen);
		}
	}

	ret_buf = crex_string_alloc(tent_ret_len, 0);
	/* crx_stream_read will not call ops->read here because the necessary
	 * data is guaranteed to be buffered */
	ZSTR_LEN(ret_buf) = crx_stream_read(stream, ZSTR_VAL(ret_buf), tent_ret_len);

	if (found_delim) {
		stream->readpos += delim_len;
		stream->position += delim_len;
	}
	ZSTR_VAL(ret_buf)[ZSTR_LEN(ret_buf)] = '\0';
	return ret_buf;
}

/* Writes a buffer directly to a stream, using multiple of the chunk size */
static ssize_t _crx_stream_write_buffer(crx_stream *stream, const char *buf, size_t count)
{
	ssize_t didwrite = 0;
	ssize_t retval;

	/* if we have a seekable stream we need to ensure that data is written at the
	 * current stream->position. This means invalidating the read buffer and then
	 * performing a low-level seek */
	if (stream->ops->seek && (stream->flags & CRX_STREAM_FLAG_NO_SEEK) == 0 && stream->readpos != stream->writepos) {
		stream->readpos = stream->writepos = 0;

		stream->ops->seek(stream, stream->position, SEEK_SET, &stream->position);
	}

	bool old_eof = stream->eof;

	while (count > 0) {
		ssize_t justwrote = stream->ops->write(stream, buf, count);
		if (justwrote <= 0) {
			/* If we already successfully wrote some bytes and a write error occurred
			 * later, report the successfully written bytes. */
			if (didwrite == 0) {
				retval = justwrote;
				goto out;
			}
			retval = didwrite;
			goto out;
		}

		buf += justwrote;
		count -= justwrote;
		didwrite += justwrote;
		stream->position += justwrote;
	}

	retval = didwrite;

out:
	if (old_eof != stream->eof) {
		crx_stream_notify_completed(CRX_STREAM_CONTEXT(stream));
	}
	return retval;
}

/* push some data through the write filter chain.
 * buf may be NULL, if flags are set to indicate a flush.
 * This may trigger a real write to the stream.
 * Returns the number of bytes consumed from buf by the first filter in the chain.
 * */
static ssize_t _crx_stream_write_filtered(crx_stream *stream, const char *buf, size_t count, int flags)
{
	size_t consumed = 0;
	crx_stream_bucket *bucket;
	crx_stream_bucket_brigade brig_in = { NULL, NULL }, brig_out = { NULL, NULL };
	crx_stream_bucket_brigade *brig_inp = &brig_in, *brig_outp = &brig_out, *brig_swap;
	crx_stream_filter_status_t status = PSFS_ERR_FATAL;
	crx_stream_filter *filter;

	if (buf) {
		bucket = crx_stream_bucket_new(stream, (char *)buf, count, 0, 0);
		crx_stream_bucket_append(&brig_in, bucket);
	}

	for (filter = stream->writefilters.head; filter; filter = filter->next) {
		/* for our return value, we are interested in the number of bytes consumed from
		 * the first filter in the chain */
		status = filter->fops->filter(stream, filter, brig_inp, brig_outp,
				filter == stream->writefilters.head ? &consumed : NULL, flags);

		if (status != PSFS_PASS_ON) {
			break;
		}
		/* brig_out becomes brig_in.
		 * brig_in will always be empty here, as the filter MUST attach any un-consumed buckets
		 * to its own brigade */
		brig_swap = brig_inp;
		brig_inp = brig_outp;
		brig_outp = brig_swap;
		memset(brig_outp, 0, sizeof(*brig_outp));
	}

	switch (status) {
		case PSFS_PASS_ON:
			/* filter chain generated some output; push it through to the
			 * underlying stream */
			while (brig_inp->head) {
				bucket = brig_inp->head;
				if (_crx_stream_write_buffer(stream, bucket->buf, bucket->buflen) < 0) {
					consumed = (ssize_t) -1;
				}

				/* Potential error situation - eg: no space on device. Perhaps we should keep this brigade
				 * hanging around and try to write it later.
				 * At the moment, we just drop it on the floor
				 * */

				crx_stream_bucket_unlink(bucket);
				crx_stream_bucket_delref(bucket);
			}
			break;
		case PSFS_FEED_ME:
			/* need more data before we can push data through to the stream */
			break;

		case PSFS_ERR_FATAL:
			/* some fatal error.  Theoretically, the stream is borked, so all
			 * further writes should fail. */
			return (ssize_t) -1;
	}

	return consumed;
}

CRXAPI int _crx_stream_flush(crx_stream *stream, int closing)
{
	int ret = 0;

	if (stream->writefilters.head) {
		_crx_stream_write_filtered(stream, NULL, 0, closing ? PSFS_FLAG_FLUSH_CLOSE : PSFS_FLAG_FLUSH_INC );
	}

	stream->flags &= ~CRX_STREAM_FLAG_WAS_WRITTEN;

	if (stream->ops->flush) {
		ret = stream->ops->flush(stream);
	}

	return ret;
}

CRXAPI ssize_t _crx_stream_write(crx_stream *stream, const char *buf, size_t count)
{
	ssize_t bytes;

	if (count == 0) {
		return 0;
	}

	CREX_ASSERT(buf != NULL);
	if (stream->ops->write == NULL) {
		crx_error_docref(NULL, E_NOTICE, "Stream is not writable");
		return (ssize_t) -1;
	}

	if (stream->writefilters.head) {
		bytes = _crx_stream_write_filtered(stream, buf, count, PSFS_FLAG_NORMAL);
	} else {
		bytes = _crx_stream_write_buffer(stream, buf, count);
	}

	if (bytes) {
		stream->flags |= CRX_STREAM_FLAG_WAS_WRITTEN;
	}

	return bytes;
}

CRXAPI ssize_t _crx_stream_printf(crx_stream *stream, const char *fmt, ...)
{
	ssize_t count;
	char *buf;
	va_list ap;

	va_start(ap, fmt);
	count = vspprintf(&buf, 0, fmt, ap);
	va_end(ap);

	if (!buf) {
		return -1; /* error condition */
	}

	count = crx_stream_write(stream, buf, count);
	efree(buf);

	return count;
}

CRXAPI crex_off_t _crx_stream_tell(crx_stream *stream)
{
	return stream->position;
}

CRXAPI int _crx_stream_seek(crx_stream *stream, crex_off_t offset, int whence)
{
	if (stream->fclose_stdiocast == CRX_STREAM_FCLOSE_FOPENCOOKIE) {
		/* flush can call seek internally so we need to prevent an infinite loop */
		if (!stream->fclose_stdiocast_flush_in_progress) {
			stream->fclose_stdiocast_flush_in_progress = 1;
			/* flush to commit data written to the fopencookie FILE* */
			fflush(stream->stdiocast);
			stream->fclose_stdiocast_flush_in_progress = 0;
		}
	}

	/* handle the case where we are in the buffer */
	if ((stream->flags & CRX_STREAM_FLAG_NO_BUFFER) == 0) {
		switch(whence) {
			case SEEK_CUR:
				if (offset > 0 && offset <= stream->writepos - stream->readpos) {
					stream->readpos += offset; /* if offset = ..., then readpos = writepos */
					stream->position += offset;
					stream->eof = 0;
					return 0;
				}
				break;
			case SEEK_SET:
				if (offset > stream->position &&
						offset <= stream->position + stream->writepos - stream->readpos) {
					stream->readpos += offset - stream->position;
					stream->position = offset;
					stream->eof = 0;
					return 0;
				}
				break;
		}
	}


	if (stream->ops->seek && (stream->flags & CRX_STREAM_FLAG_NO_SEEK) == 0) {
		int ret;

		if (stream->writefilters.head) {
			_crx_stream_flush(stream, 0);
		}

		switch(whence) {
			case SEEK_CUR:
				offset = stream->position + offset;
				whence = SEEK_SET;
				break;
		}
		ret = stream->ops->seek(stream, offset, whence, &stream->position);

		if (((stream->flags & CRX_STREAM_FLAG_NO_SEEK) == 0) || ret == 0) {
			if (ret == 0) {
				stream->eof = 0;
			}

			/* invalidate the buffer contents */
			stream->readpos = stream->writepos = 0;

			return ret;
		}
		/* else the stream has decided that it can't support seeking after all;
		 * fall through to attempt emulation */
	}

	/* emulate forward moving seeks with reads */
	if (whence == SEEK_CUR && offset >= 0) {
		char tmp[1024];
		ssize_t didread;
		while (offset > 0) {
			if ((didread = crx_stream_read(stream, tmp, MIN(offset, sizeof(tmp)))) <= 0) {
				return -1;
			}
			offset -= didread;
		}
		stream->eof = 0;
		return 0;
	}

	crx_error_docref(NULL, E_WARNING, "Stream does not support seeking");

	return -1;
}

CRXAPI int _crx_stream_set_option(crx_stream *stream, int option, int value, void *ptrparam)
{
	int ret = CRX_STREAM_OPTION_RETURN_NOTIMPL;

	if (stream->ops->set_option) {
		ret = stream->ops->set_option(stream, option, value, ptrparam);
	}

	if (ret == CRX_STREAM_OPTION_RETURN_NOTIMPL) {
		switch(option) {
			case CRX_STREAM_OPTION_SET_CHUNK_SIZE:
				/* XXX chunk size itself is of size_t, that might be ok or not for a particular case*/
				ret = stream->chunk_size > INT_MAX ? INT_MAX : (int)stream->chunk_size;
				stream->chunk_size = value;
				return ret;

			case CRX_STREAM_OPTION_READ_BUFFER:
				/* try to match the buffer mode as best we can */
				if (value == CRX_STREAM_BUFFER_NONE) {
					stream->flags |= CRX_STREAM_FLAG_NO_BUFFER;
				} else if (stream->flags & CRX_STREAM_FLAG_NO_BUFFER) {
					stream->flags ^= CRX_STREAM_FLAG_NO_BUFFER;
				}
				ret = CRX_STREAM_OPTION_RETURN_OK;
				break;

			default:
				;
		}
	}

	return ret;
}

CRXAPI int _crx_stream_sync(crx_stream *stream, bool data_only)
{
	int op = CRX_STREAM_SYNC_FSYNC;
	if (data_only) {
		op = CRX_STREAM_SYNC_FDSYNC;
	}
	return crx_stream_set_option(stream, CRX_STREAM_OPTION_SYNC_API, op, NULL);
}

CRXAPI int _crx_stream_truncate_set_size(crx_stream *stream, size_t newsize)
{
	return crx_stream_set_option(stream, CRX_STREAM_OPTION_TRUNCATE_API, CRX_STREAM_TRUNCATE_SET_SIZE, &newsize);
}

CRXAPI ssize_t _crx_stream_passthru(crx_stream * stream STREAMS_DC)
{
	size_t bcount = 0;
	char buf[8192];
	ssize_t b;

	if (crx_stream_mmap_possible(stream)) {
		char *p;
		size_t mapped;

		p = crx_stream_mmap_range(stream, crx_stream_tell(stream), CRX_STREAM_MMAP_ALL, CRX_STREAM_MAP_MODE_SHARED_READONLY, &mapped);

		if (p) {
			do {
				/* output functions return int, so pass in int max */
				if (0 < (b = CRXWRITE(p + bcount, MIN(mapped - bcount, INT_MAX)))) {
					bcount += b;
				}
			} while (b > 0 && mapped > bcount);

			crx_stream_mmap_unmap_ex(stream, mapped);

			return bcount;
		}
	}

	while ((b = crx_stream_read(stream, buf, sizeof(buf))) > 0) {
		CRXWRITE(buf, b);
		bcount += b;
	}

	if (b < 0 && bcount == 0) {
		return b;
	}

	return bcount;
}


CRXAPI crex_string *_crx_stream_copy_to_mem(crx_stream *src, size_t maxlen, int persistent STREAMS_DC)
{
	ssize_t ret = 0;
	char *ptr;
	size_t len = 0, buflen;
	int step = CHUNK_SIZE;
	int min_room = CHUNK_SIZE / 4;
	crx_stream_statbuf ssbuf;
	crex_string *result;

	if (maxlen == 0) {
		return ZSTR_EMPTY_ALLOC();
	}

	if (maxlen == CRX_STREAM_COPY_ALL) {
		maxlen = 0;
	}

	if (maxlen > 0 && maxlen < 4 * CHUNK_SIZE) {
		result = crex_string_alloc(maxlen, persistent);
		ptr = ZSTR_VAL(result);
		while ((len < maxlen) && !crx_stream_eof(src)) {
			ret = crx_stream_read(src, ptr, maxlen - len);
			if (ret <= 0) {
				// TODO: Propagate error?
				break;
			}
			len += ret;
			ptr += ret;
		}
		if (len) {
			ZSTR_LEN(result) = len;
			ZSTR_VAL(result)[len] = '\0';

			/* Only truncate if the savings are large enough */
			if (len < maxlen / 2) {
				result = crex_string_truncate(result, len, persistent);
			}
		} else {
			crex_string_free(result);
			result = NULL;
		}
		return result;
	}

	/* avoid many reallocs by allocating a good-sized chunk to begin with, if
	 * we can.  Note that the stream may be filtered, in which case the stat
	 * result may be inaccurate, as the filter may inflate or deflate the
	 * number of bytes that we can read.  In order to avoid an upsize followed
	 * by a downsize of the buffer, overestimate by the step size (which is
	 * 8K).  */
	if (crx_stream_stat(src, &ssbuf) == 0 && ssbuf.sb.st_size > 0) {
		buflen = MAX(ssbuf.sb.st_size - src->position, 0) + step;
		if (maxlen > 0 && buflen > maxlen) {
			buflen = maxlen;
		}
	} else {
		buflen = step;
	}

	result = crex_string_alloc(buflen, persistent);
	ptr = ZSTR_VAL(result);

	// TODO: Propagate error?
	while ((ret = crx_stream_read(src, ptr, buflen - len)) > 0) {
		len += ret;
		if (len + min_room >= buflen) {
			if (maxlen == len) {
				break;
			}
			if (maxlen > 0 && buflen + step > maxlen) {
				buflen = maxlen;
			} else {
				buflen += step;
			}
			result = crex_string_extend(result, buflen, persistent);
			ptr = ZSTR_VAL(result) + len;
		} else {
			ptr += ret;
		}
	}
	if (len) {
		result = crex_string_truncate(result, len, persistent);
		ZSTR_VAL(result)[len] = '\0';
	} else {
		crex_string_free(result);
		result = NULL;
	}

	return result;
}

/* Returns SUCCESS/FAILURE and sets *len to the number of bytes moved */
CRXAPI crex_result _crx_stream_copy_to_stream_ex(crx_stream *src, crx_stream *dest, size_t maxlen, size_t *len STREAMS_DC)
{
	char buf[CHUNK_SIZE];
	size_t haveread = 0;
	size_t towrite;
	size_t dummy;

	if (!len) {
		len = &dummy;
	}

	if (maxlen == 0) {
		*len = 0;
		return SUCCESS;
	}

#ifdef HAVE_COPY_FILE_RANGE
	if (crx_stream_is(src, CRX_STREAM_IS_STDIO) &&
			crx_stream_is(dest, CRX_STREAM_IS_STDIO) &&
			src->writepos == src->readpos) {
		/* both crx_stream instances are backed by a file descriptor, are not filtered and the
		 * read buffer is empty: we can use copy_file_range() */
		int src_fd, dest_fd, dest_open_flags = 0;

		/* copy_file_range does not work with O_APPEND */
		if (crx_stream_cast(src, CRX_STREAM_AS_FD, (void*)&src_fd, 0) == SUCCESS &&
				crx_stream_cast(dest, CRX_STREAM_AS_FD, (void*)&dest_fd, 0) == SUCCESS &&
				/* get dest open flags to check if the stream is open in append mode */
				crx_stream_parse_fopen_modes(dest->mode, &dest_open_flags) == SUCCESS &&
				!(dest_open_flags & O_APPEND)) {

			/* clamp to INT_MAX to avoid EOVERFLOW */
			const size_t cfr_max = MIN(maxlen, (size_t)SSIZE_MAX);

			/* copy_file_range() is a Linux-specific system call which allows efficient copying
			 * between two file descriptors, eliminating the need to transfer data from the kernel
			 * to userspace and back. For networking file systems like NFS and Ceph, it even
			 * eliminates copying data to the client, and local filesystems like Btrfs and XFS can
			 * create shared extents. */
			ssize_t result = copy_file_range(src_fd, NULL, dest_fd, NULL, cfr_max, 0);
			if (result > 0) {
				size_t nbytes = (size_t)result;
				haveread += nbytes;

				src->position += nbytes;
				dest->position += nbytes;

				if ((maxlen != CRX_STREAM_COPY_ALL && nbytes == maxlen) || crx_stream_eof(src)) {
					/* the whole request was satisfied or end-of-file reached - done */
					*len = haveread;
					return SUCCESS;
				}

				/* there may be more data; continue copying using the fallback code below */
			} else if (result == 0) {
				/* end of file */
				*len = haveread;
				return SUCCESS;
			} else if (result < 0) {
				switch (errno) {
					case EINVAL:
						/* some formal error, e.g. overlapping file ranges */
						break;

					case EXDEV:
						/* pre Linux 5.3 error */
						break;

					case ENOSYS:
						/* not implemented by this Linux kernel */
						break;

					case EIO:
						/* Some filesystems will cause failures if the max length is greater than the file length
						 * in certain circumstances and configuration. In those cases the errno is EIO and we will
						 * fall back to other methods. We cannot use stat to determine the file length upfront because
						 * that is prone to races and outdated caching. */
						break;

					default:
						/* unexpected I/O error - give up, no fallback */
						*len = haveread;
						return FAILURE;
				}

				/* fall back to classic copying */
			}
		}
	}
#endif // HAVE_COPY_FILE_RANGE

	if (maxlen == CRX_STREAM_COPY_ALL) {
		maxlen = 0;
	}

	if (crx_stream_mmap_possible(src)) {
		char *p;

		do {
			/* We must not modify maxlen here, because otherwise the file copy fallback below can fail */
			size_t chunk_size, must_read, mapped;
			if (maxlen == 0) {
				/* Unlimited read */
				must_read = chunk_size = CRX_STREAM_MMAP_MAX;
			} else {
				must_read = maxlen - haveread;
				if (must_read >= CRX_STREAM_MMAP_MAX) {
					chunk_size = CRX_STREAM_MMAP_MAX;
				} else {
					/* In case the length we still have to read from the file could be smaller than the file size,
					 * chunk_size must not get bigger the size we're trying to read. */
					chunk_size = must_read;
				}
			}

			p = crx_stream_mmap_range(src, crx_stream_tell(src), chunk_size, CRX_STREAM_MAP_MODE_SHARED_READONLY, &mapped);

			if (p) {
				ssize_t didwrite;

				if (crx_stream_seek(src, mapped, SEEK_CUR) != 0) {
					crx_stream_mmap_unmap(src);
					break;
				}

				didwrite = crx_stream_write(dest, p, mapped);
				if (didwrite < 0) {
					*len = haveread;
					crx_stream_mmap_unmap(src);
					return FAILURE;
				}

				crx_stream_mmap_unmap(src);

				*len = haveread += didwrite;

				/* we've got at least 1 byte to read
				 * less than 1 is an error
				 * AND read bytes match written */
				if (mapped == 0 || mapped != didwrite) {
					return FAILURE;
				}
				if (mapped < chunk_size) {
					return SUCCESS;
				}
				/* If we're not reading as much as possible, so a bounded read */
				if (maxlen != 0) {
					must_read -= mapped;
					if (must_read == 0) {
						return SUCCESS;
					}
				}
			}
		} while (p);
	}

	while(1) {
		size_t readchunk = sizeof(buf);
		ssize_t didread;
		char *writeptr;

		if (maxlen && (maxlen - haveread) < readchunk) {
			readchunk = maxlen - haveread;
		}

		didread = crx_stream_read(src, buf, readchunk);
		if (didread <= 0) {
			*len = haveread;
			return didread < 0 ? FAILURE : SUCCESS;
		}

		towrite = didread;
		writeptr = buf;
		haveread += didread;

		while (towrite) {
			ssize_t didwrite = crx_stream_write(dest, writeptr, towrite);
			if (didwrite <= 0) {
				*len = haveread - (didread - towrite);
				return FAILURE;
			}

			towrite -= didwrite;
			writeptr += didwrite;
		}

		if (maxlen && maxlen == haveread) {
			break;
		}
	}

	*len = haveread;
	return SUCCESS;
}

/* Returns the number of bytes moved.
 * Returns 1 when source len is 0.
 * Deprecated in favor of crx_stream_copy_to_stream_ex() */
CREX_ATTRIBUTE_DEPRECATED
CRXAPI size_t _crx_stream_copy_to_stream(crx_stream *src, crx_stream *dest, size_t maxlen STREAMS_DC)
{
	size_t len;
	crex_result ret = _crx_stream_copy_to_stream_ex(src, dest, maxlen, &len STREAMS_REL_CC);
	if (ret == SUCCESS && len == 0 && maxlen != 0) {
		return 1;
	}
	return len;
}
/* }}} */

/* {{{ wrapper init and registration */

static void stream_resource_regular_dtor(crex_resource *rsrc)
{
	crx_stream *stream = (crx_stream*)rsrc->ptr;
	/* set the return value for pclose */
	FG(pclose_ret) = crx_stream_free(stream, CRX_STREAM_FREE_CLOSE | CRX_STREAM_FREE_RSRC_DTOR);
}

static void stream_resource_persistent_dtor(crex_resource *rsrc)
{
	crx_stream *stream = (crx_stream*)rsrc->ptr;
	FG(pclose_ret) = crx_stream_free(stream, CRX_STREAM_FREE_CLOSE | CRX_STREAM_FREE_RSRC_DTOR);
}

void crx_shutdown_stream_hashes(void)
{
	FG(user_stream_current_filename) = NULL;
	if (FG(stream_wrappers)) {
		crex_hash_destroy(FG(stream_wrappers));
		efree(FG(stream_wrappers));
		FG(stream_wrappers) = NULL;
	}

	if (FG(stream_filters)) {
		crex_hash_destroy(FG(stream_filters));
		efree(FG(stream_filters));
		FG(stream_filters) = NULL;
	}

	if (FG(wrapper_errors)) {
		crex_hash_destroy(FG(wrapper_errors));
		efree(FG(wrapper_errors));
		FG(wrapper_errors) = NULL;
	}
}

int crx_init_stream_wrappers(int module_number)
{
	le_stream = crex_register_list_destructors_ex(stream_resource_regular_dtor, NULL, "stream", module_number);
	le_pstream = crex_register_list_destructors_ex(NULL, stream_resource_persistent_dtor, "persistent stream", module_number);

	/* Filters are cleaned up by the streams they're attached to */
	le_stream_filter = crex_register_list_destructors_ex(NULL, NULL, "stream filter", module_number);

	crex_hash_init(&url_stream_wrappers_hash, 8, NULL, NULL, 1);
	crex_hash_init(crx_get_stream_filters_hash_global(), 8, NULL, NULL, 1);
	crex_hash_init(crx_stream_xport_get_hash(), 8, NULL, NULL, 1);

	return (crx_stream_xport_register("tcp", crx_stream_generic_socket_factory) == SUCCESS
			&&
			crx_stream_xport_register("udp", crx_stream_generic_socket_factory) == SUCCESS
#if defined(AF_UNIX) && !(defined(CRX_WIN32) || defined(__riscos__))
			&&
			crx_stream_xport_register("unix", crx_stream_generic_socket_factory) == SUCCESS
			&&
			crx_stream_xport_register("udg", crx_stream_generic_socket_factory) == SUCCESS
#endif
		) ? SUCCESS : FAILURE;
}

void crx_shutdown_stream_wrappers(int module_number)
{
	crex_hash_destroy(&url_stream_wrappers_hash);
	crex_hash_destroy(crx_get_stream_filters_hash_global());
	crex_hash_destroy(crx_stream_xport_get_hash());
}

/* Validate protocol scheme names during registration
 * Must conform to /^[a-zA-Z0-9+.-]+$/
 */
static inline crex_result crx_stream_wrapper_scheme_validate(const char *protocol, unsigned int protocol_len)
{
	unsigned int i;

	for(i = 0; i < protocol_len; i++) {
		if (!isalnum((int)protocol[i]) &&
			protocol[i] != '+' &&
			protocol[i] != '-' &&
			protocol[i] != '.') {
			return FAILURE;
		}
	}

	return SUCCESS;
}

/* API for registering GLOBAL wrappers */
CRXAPI crex_result crx_register_url_stream_wrapper(const char *protocol, const crx_stream_wrapper *wrapper)
{
	size_t protocol_len = strlen(protocol);
	crex_result ret;
	crex_string *str;

	if (crx_stream_wrapper_scheme_validate(protocol, protocol_len) == FAILURE) {
		return FAILURE;
	}

	str = crex_string_init_interned(protocol, protocol_len, 1);
	ret = crex_hash_add_ptr(&url_stream_wrappers_hash, str, (void*)wrapper) ? SUCCESS : FAILURE;
	crex_string_release_ex(str, 1);
	return ret;
}

CRXAPI crex_result crx_unregister_url_stream_wrapper(const char *protocol)
{
	return crex_hash_str_del(&url_stream_wrappers_hash, protocol, strlen(protocol));
}

static void clone_wrapper_hash(void)
{
	ALLOC_HASHTABLE(FG(stream_wrappers));
	crex_hash_init(FG(stream_wrappers), crex_hash_num_elements(&url_stream_wrappers_hash), NULL, NULL, 0);
	crex_hash_copy(FG(stream_wrappers), &url_stream_wrappers_hash, NULL);
}

/* API for registering VOLATILE wrappers */
CRXAPI crex_result crx_register_url_stream_wrapper_volatile(crex_string *protocol, crx_stream_wrapper *wrapper)
{
	if (crx_stream_wrapper_scheme_validate(ZSTR_VAL(protocol), ZSTR_LEN(protocol)) == FAILURE) {
		return FAILURE;
	}

	if (!FG(stream_wrappers)) {
		clone_wrapper_hash();
	}

	return crex_hash_add_ptr(FG(stream_wrappers), protocol, wrapper) ? SUCCESS : FAILURE;
}

CRXAPI crex_result crx_unregister_url_stream_wrapper_volatile(crex_string *protocol)
{
	if (!FG(stream_wrappers)) {
		clone_wrapper_hash();
	}

	return crex_hash_del(FG(stream_wrappers), protocol);
}
/* }}} */

/* {{{ crx_stream_locate_url_wrapper */
CRXAPI crx_stream_wrapper *crx_stream_locate_url_wrapper(const char *path, const char **path_for_open, int options)
{
	HashTable *wrapper_hash = (FG(stream_wrappers) ? FG(stream_wrappers) : &url_stream_wrappers_hash);
	crx_stream_wrapper *wrapper = NULL;
	const char *p, *protocol = NULL;
	size_t n = 0;

	if (path_for_open) {
		*path_for_open = (char*)path;
	}

	if (options & IGNORE_URL) {
		return (crx_stream_wrapper*)((options & STREAM_LOCATE_WRAPPERS_ONLY) ? NULL : &crx_plain_files_wrapper);
	}

	for (p = path; isalnum((int)*p) || *p == '+' || *p == '-' || *p == '.'; p++) {
		n++;
	}

	if ((*p == ':') && (n > 1) && (!strncmp("//", p+1, 2) || (n == 4 && !memcmp("data:", path, 5)))) {
		protocol = path;
	}

	if (protocol) {
		if (NULL == (wrapper = crex_hash_str_find_ptr(wrapper_hash, protocol, n))) {
			char *tmp = estrndup(protocol, n);

			crex_str_tolower(tmp, n);
			if (NULL == (wrapper = crex_hash_str_find_ptr(wrapper_hash, tmp, n))) {
				char wrapper_name[32];

				if (n >= sizeof(wrapper_name)) {
					n = sizeof(wrapper_name) - 1;
				}
				CRX_STRLCPY(wrapper_name, protocol, sizeof(wrapper_name), n);

				crx_error_docref(NULL, E_WARNING, "Unable to find the wrapper \"%s\" - did you forget to enable it when you configured CRX?", wrapper_name);

				wrapper = NULL;
				protocol = NULL;
			}
			efree(tmp);
		}
	}
	/* TODO: curl based streams probably support file:// properly */
	if (!protocol || !strncasecmp(protocol, "file", n))	{
		/* fall back on regular file access */
		crx_stream_wrapper *plain_files_wrapper = (crx_stream_wrapper*)&crx_plain_files_wrapper;

		if (protocol) {
			int localhost = 0;

			if (!strncasecmp(path, "file://localhost/", 17)) {
				localhost = 1;
			}

#ifdef CRX_WIN32
			if (localhost == 0 && path[n+3] != '\0' && path[n+3] != '/' && path[n+4] != ':')	{
#else
			if (localhost == 0 && path[n+3] != '\0' && path[n+3] != '/') {
#endif
				if (options & REPORT_ERRORS) {
					crx_error_docref(NULL, E_WARNING, "Remote host file access not supported, %s", path);
				}
				return NULL;
			}

			if (path_for_open) {
				/* skip past protocol and :/, but handle windows correctly */
				*path_for_open = (char*)path + n + 1;
				if (localhost == 1) {
					(*path_for_open) += 11;
				}
				while (*(++*path_for_open)=='/') {
					/* intentionally empty */
				}
#ifdef CRX_WIN32
				if (*(*path_for_open + 1) != ':')
#endif
					(*path_for_open)--;
			}
		}

		if (options & STREAM_LOCATE_WRAPPERS_ONLY) {
			return NULL;
		}

		if (FG(stream_wrappers)) {
		/* The file:// wrapper may have been disabled/overridden */

			if (wrapper) {
				/* It was found so go ahead and provide it */
				return wrapper;
			}

			/* Check again, the original check might have not known the protocol name */
			if ((wrapper = crex_hash_find_ex_ptr(wrapper_hash, ZSTR_KNOWN(CREX_STR_FILE), 1)) != NULL) {
				return wrapper;
			}

			if (options & REPORT_ERRORS) {
				crx_error_docref(NULL, E_WARNING, "file:// wrapper is disabled in the server configuration");
			}
			return NULL;
		}

		return plain_files_wrapper;
	}

	if (wrapper && wrapper->is_url &&
	    (options & STREAM_DISABLE_URL_PROTECTION) == 0 &&
	    (!PG(allow_url_fopen) ||
	     (((options & STREAM_OPEN_FOR_INCLUDE) ||
	       PG(in_user_include)) && !PG(allow_url_include)))) {
		if (options & REPORT_ERRORS) {
			/* protocol[n] probably isn't '\0' */
			if (!PG(allow_url_fopen)) {
				crx_error_docref(NULL, E_WARNING, "%.*s:// wrapper is disabled in the server configuration by allow_url_fopen=0", (int)n, protocol);
			} else {
				crx_error_docref(NULL, E_WARNING, "%.*s:// wrapper is disabled in the server configuration by allow_url_include=0", (int)n, protocol);
			}
		}
		return NULL;
	}

	return wrapper;
}
/* }}} */

/* {{{ _crx_stream_mkdir */
CRXAPI int _crx_stream_mkdir(const char *path, int mode, int options, crx_stream_context *context)
{
	crx_stream_wrapper *wrapper = NULL;

	wrapper = crx_stream_locate_url_wrapper(path, NULL, 0);
	if (!wrapper || !wrapper->wops || !wrapper->wops->stream_mkdir) {
		return 0;
	}

	return wrapper->wops->stream_mkdir(wrapper, path, mode, options, context);
}
/* }}} */

/* {{{ _crx_stream_rmdir */
CRXAPI int _crx_stream_rmdir(const char *path, int options, crx_stream_context *context)
{
	crx_stream_wrapper *wrapper = NULL;

	wrapper = crx_stream_locate_url_wrapper(path, NULL, 0);
	if (!wrapper || !wrapper->wops || !wrapper->wops->stream_rmdir) {
		return 0;
	}

	return wrapper->wops->stream_rmdir(wrapper, path, options, context);
}
/* }}} */

/* {{{ _crx_stream_stat_path */
CRXAPI int _crx_stream_stat_path(const char *path, int flags, crx_stream_statbuf *ssb, crx_stream_context *context)
{
	crx_stream_wrapper *wrapper = NULL;
	const char *path_to_open = path;

	memset(ssb, 0, sizeof(*ssb));

	wrapper = crx_stream_locate_url_wrapper(path, &path_to_open, 0);
	if (wrapper && wrapper->wops->url_stat) {
		return wrapper->wops->url_stat(wrapper, path_to_open, flags, ssb, context);
	}
	return -1;
}
/* }}} */

/* {{{ crx_stream_opendir */
CRXAPI crx_stream *_crx_stream_opendir(const char *path, int options,
		crx_stream_context *context STREAMS_DC)
{
	crx_stream *stream = NULL;
	crx_stream_wrapper *wrapper = NULL;
	const char *path_to_open;

	if (!path || !*path) {
		return NULL;
	}

	path_to_open = path;

	wrapper = crx_stream_locate_url_wrapper(path, &path_to_open, options);

	if (wrapper && wrapper->wops->dir_opener) {
		stream = wrapper->wops->dir_opener(wrapper,
				path_to_open, "r", options & ~REPORT_ERRORS, NULL,
				context STREAMS_REL_CC);

		if (stream) {
			stream->wrapper = wrapper;
			stream->flags |= CRX_STREAM_FLAG_NO_BUFFER | CRX_STREAM_FLAG_IS_DIR;
		}
	} else if (wrapper) {
		crx_stream_wrapper_log_error(wrapper, options & ~REPORT_ERRORS, "not implemented");
	}
	if (stream == NULL && (options & REPORT_ERRORS)) {
		crx_stream_display_wrapper_errors(wrapper, path, "Failed to open directory");
	}
	crx_stream_tidy_wrapper_error_log(wrapper);

	return stream;
}
/* }}} */

/* {{{ _crx_stream_readdir */
CRXAPI crx_stream_dirent *_crx_stream_readdir(crx_stream *dirstream, crx_stream_dirent *ent)
{

	if (sizeof(crx_stream_dirent) == crx_stream_read(dirstream, (char*)ent, sizeof(crx_stream_dirent))) {
		return ent;
	}

	return NULL;
}
/* }}} */

/* {{{ crx_stream_open_wrapper_ex */
CRXAPI crx_stream *_crx_stream_open_wrapper_ex(const char *path, const char *mode, int options,
		crex_string **opened_path, crx_stream_context *context STREAMS_DC)
{
	crx_stream *stream = NULL;
	crx_stream_wrapper *wrapper = NULL;
	const char *path_to_open;
	int persistent = options & STREAM_OPEN_PERSISTENT;
	crex_string *path_str = NULL;
	crex_string *resolved_path = NULL;
	char *copy_of_path = NULL;

	if (opened_path) {
		if (options & STREAM_OPEN_FOR_CREX_STREAM) {
			path_str = *opened_path;
		}
		*opened_path = NULL;
	}

	if (!path || !*path) {
		crex_value_error("Path cannot be empty");
		return NULL;
	}

	if (options & USE_PATH) {
		if (path_str) {
			resolved_path = crex_resolve_path(path_str);
		} else {
			resolved_path = crx_resolve_path(path, strlen(path), PG(include_path));
		}
		if (resolved_path) {
			path = ZSTR_VAL(resolved_path);
			/* we've found this file, don't re-check include_path or run realpath */
			options |= STREAM_ASSUME_REALPATH;
			options &= ~USE_PATH;
		}
		if (EG(exception)) {
			return NULL;
		}
	}

	path_to_open = path;

	wrapper = crx_stream_locate_url_wrapper(path, &path_to_open, options);
	if ((options & STREAM_USE_URL) && (!wrapper || !wrapper->is_url)) {
		crx_error_docref(NULL, E_WARNING, "This function may only be used against URLs");
		if (resolved_path) {
			crex_string_release_ex(resolved_path, 0);
		}
		return NULL;
	}

	if (wrapper) {
		if (!wrapper->wops->stream_opener) {
			crx_stream_wrapper_log_error(wrapper, options & ~REPORT_ERRORS,
					"wrapper does not support stream open");
		} else {
			stream = wrapper->wops->stream_opener(wrapper,
				path_to_open, mode, options & ~REPORT_ERRORS,
				opened_path, context STREAMS_REL_CC);
		}

		/* if the caller asked for a persistent stream but the wrapper did not
		 * return one, force an error here */
		if (stream && (options & STREAM_OPEN_PERSISTENT) && !stream->is_persistent) {
			crx_stream_wrapper_log_error(wrapper, options & ~REPORT_ERRORS,
					"wrapper does not support persistent streams");
			crx_stream_close(stream);
			stream = NULL;
		}

		if (stream) {
			stream->wrapper = wrapper;
		}
	}

	if (stream) {
		if (opened_path && !*opened_path && resolved_path) {
			*opened_path = resolved_path;
			resolved_path = NULL;
		}
		if (stream->orig_path) {
			pefree(stream->orig_path, persistent);
		}
		copy_of_path = pestrdup(path, persistent);
		stream->orig_path = copy_of_path;
#if CREX_DEBUG
		stream->open_filename = __crex_orig_filename ? __crex_orig_filename : __crex_filename;
		stream->open_lineno = __crex_orig_lineno ? __crex_orig_lineno : __crex_lineno;
#endif
	}

	if (stream != NULL && (options & STREAM_MUST_SEEK)) {
		crx_stream *newstream;

		switch(crx_stream_make_seekable_rel(stream, &newstream,
					(options & STREAM_WILL_CAST)
						? CRX_STREAM_PREFER_STDIO : CRX_STREAM_NO_PREFERENCE)) {
			case CRX_STREAM_UNCHANGED:
				if (resolved_path) {
					crex_string_release_ex(resolved_path, 0);
				}
				return stream;
			case CRX_STREAM_RELEASED:
				if (newstream->orig_path) {
					pefree(newstream->orig_path, persistent);
				}
				newstream->orig_path = pestrdup(path, persistent);
				if (resolved_path) {
					crex_string_release_ex(resolved_path, 0);
				}
				return newstream;
			default:
				crx_stream_close(stream);
				stream = NULL;
				if (options & REPORT_ERRORS) {
					char *tmp = estrdup(path);
					crx_strip_url_passwd(tmp);
					crx_error_docref1(NULL, tmp, E_WARNING, "could not make seekable - %s",
							tmp);
					efree(tmp);

					options &= ~REPORT_ERRORS;
				}
		}
	}

	if (stream && stream->ops->seek && (stream->flags & CRX_STREAM_FLAG_NO_SEEK) == 0 && strchr(mode, 'a') && stream->position == 0) {
		crex_off_t newpos = 0;

		/* if opened for append, we need to revise our idea of the initial file position */
		if (0 == stream->ops->seek(stream, 0, SEEK_CUR, &newpos)) {
			stream->position = newpos;
		}
	}

	if (stream == NULL && (options & REPORT_ERRORS)) {
		crx_stream_display_wrapper_errors(wrapper, path, "Failed to open stream");
		if (opened_path && *opened_path) {
			crex_string_release_ex(*opened_path, 0);
			*opened_path = NULL;
		}
	}
	crx_stream_tidy_wrapper_error_log(wrapper);
#if CREX_DEBUG
	if (stream == NULL && copy_of_path != NULL) {
		pefree(copy_of_path, persistent);
	}
#endif
	if (resolved_path) {
		crex_string_release_ex(resolved_path, 0);
	}
	return stream;
}
/* }}} */

/* {{{ context API */
CRXAPI crx_stream_context *crx_stream_context_set(crx_stream *stream, crx_stream_context *context)
{
	crx_stream_context *oldcontext = CRX_STREAM_CONTEXT(stream);

	if (context) {
		stream->ctx = context->res;
		GC_ADDREF(context->res);
	} else {
		stream->ctx = NULL;
	}
	if (oldcontext) {
		crex_list_delete(oldcontext->res);
	}

	return oldcontext;
}

CRXAPI void crx_stream_notification_notify(crx_stream_context *context, int notifycode, int severity,
		char *xmsg, int xcode, size_t bytes_sofar, size_t bytes_max, void * ptr)
{
	if (context && context->notifier)
		context->notifier->func(context, notifycode, severity, xmsg, xcode, bytes_sofar, bytes_max, ptr);
}

CRXAPI void crx_stream_context_free(crx_stream_context *context)
{
	if (C_TYPE(context->options) != IS_UNDEF) {
		zval_ptr_dtor(&context->options);
		ZVAL_UNDEF(&context->options);
	}
	if (context->notifier) {
		crx_stream_notification_free(context->notifier);
		context->notifier = NULL;
	}
	efree(context);
}

CRXAPI crx_stream_context *crx_stream_context_alloc(void)
{
	crx_stream_context *context;

	context = ecalloc(1, sizeof(crx_stream_context));
	array_init(&context->options);

	context->res = crex_register_resource(context, crx_le_stream_context());
	return context;
}

CRXAPI crx_stream_notifier *crx_stream_notification_alloc(void)
{
	return ecalloc(1, sizeof(crx_stream_notifier));
}

CRXAPI void crx_stream_notification_free(crx_stream_notifier *notifier)
{
	if (notifier->dtor) {
		notifier->dtor(notifier);
	}
	efree(notifier);
}

CRXAPI zval *crx_stream_context_get_option(crx_stream_context *context,
		const char *wrappername, const char *optionname)
{
	zval *wrapperhash;

	if (NULL == (wrapperhash = crex_hash_str_find(C_ARRVAL(context->options), wrappername, strlen(wrappername)))) {
		return NULL;
	}
	return crex_hash_str_find(C_ARRVAL_P(wrapperhash), optionname, strlen(optionname));
}

CRXAPI void crx_stream_context_set_option(crx_stream_context *context,
		const char *wrappername, const char *optionname, zval *optionvalue)
{
	zval *wrapperhash;
	zval category;

	SEPARATE_ARRAY(&context->options);
	wrapperhash = crex_hash_str_find(C_ARRVAL(context->options), wrappername, strlen(wrappername));
	if (NULL == wrapperhash) {
		array_init(&category);
		wrapperhash = crex_hash_str_update(C_ARRVAL(context->options), (char*)wrappername, strlen(wrappername), &category);
	}
	ZVAL_DEREF(optionvalue);
	C_TRY_ADDREF_P(optionvalue);
	SEPARATE_ARRAY(wrapperhash);
	crex_hash_str_update(C_ARRVAL_P(wrapperhash), optionname, strlen(optionname), optionvalue);
}
/* }}} */

/* {{{ crx_stream_dirent_alphasort */
CRXAPI int crx_stream_dirent_alphasort(const crex_string **a, const crex_string **b)
{
	return strcoll(ZSTR_VAL(*a), ZSTR_VAL(*b));
}
/* }}} */

/* {{{ crx_stream_dirent_alphasortr */
CRXAPI int crx_stream_dirent_alphasortr(const crex_string **a, const crex_string **b)
{
	return strcoll(ZSTR_VAL(*b), ZSTR_VAL(*a));
}
/* }}} */

/* {{{ crx_stream_scandir */
CRXAPI int _crx_stream_scandir(const char *dirname, crex_string **namelist[], int flags, crx_stream_context *context,
			  int (*compare) (const crex_string **a, const crex_string **b))
{
	crx_stream *stream;
	crx_stream_dirent sdp;
	crex_string **vector = NULL;
	unsigned int vector_size = 0;
	unsigned int nfiles = 0;

	if (!namelist) {
		return -1;
	}

	stream = crx_stream_opendir(dirname, REPORT_ERRORS, context);
	if (!stream) {
		return -1;
	}

	while (crx_stream_readdir(stream, &sdp)) {
		if (nfiles == vector_size) {
			if (vector_size == 0) {
				vector_size = 10;
			} else {
				if(vector_size*2 < vector_size) {
					/* overflow */
					crx_stream_closedir(stream);
					efree(vector);
					return -1;
				}
				vector_size *= 2;
			}
			vector = (crex_string **) safe_erealloc(vector, vector_size, sizeof(char *), 0);
		}

		vector[nfiles] = crex_string_init(sdp.d_name, strlen(sdp.d_name), 0);

		nfiles++;
		if(vector_size < 10 || nfiles == 0) {
			/* overflow */
			crx_stream_closedir(stream);
			efree(vector);
			return -1;
		}
	}
	crx_stream_closedir(stream);

	*namelist = vector;

	if (nfiles > 0 && compare) {
		qsort(*namelist, nfiles, sizeof(crex_string *), (int(*)(const void *, const void *))compare);
	}
	return nfiles;
}
/* }}} */
