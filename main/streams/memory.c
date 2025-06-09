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

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include "crx.h"
#include "ext/standard/base64.h"

CRXAPI size_t crx_url_decode(char *str, size_t len);

/* Memory streams use a dynamic memory buffer to emulate a stream.
 * You can use crx_stream_memory_open to create a readonly stream
 * from an existing memory buffer.
 */

/* Temp streams are streams that uses memory streams as long their
 * size is less than a given memory amount. When a write operation
 * exceeds that limit the content is written to a temporary file.
 */

/* {{{ ------- MEMORY stream implementation -------*/

typedef struct {
	crex_string *data;
	size_t      fpos;
	int			mode;
} crx_stream_memory_data;


/* {{{ */
static ssize_t crx_stream_memory_write(crx_stream *stream, const char *buf, size_t count)
{
	crx_stream_memory_data *ms = (crx_stream_memory_data*)stream->abstract;
	assert(ms != NULL);

	if (ms->mode & TEMP_STREAM_READONLY) {
		return (ssize_t) -1;
	}
	size_t data_len = ZSTR_LEN(ms->data);
	if (ms->mode & TEMP_STREAM_APPEND) {
		ms->fpos = data_len;
	}
	if (ms->fpos + count > data_len) {
		ms->data = crex_string_realloc(ms->data, ms->fpos + count, 0);
		if (ms->fpos > data_len) {
			/* zero the bytes added due to seek past end position */
			memset(ZSTR_VAL(ms->data) + data_len, 0, ms->fpos - data_len);
		}
	} else {
		ms->data = crex_string_separate(ms->data, 0);
	}
	if (count) {
		CREX_ASSERT(buf != NULL);
		memcpy(ZSTR_VAL(ms->data) + ms->fpos, (char*) buf, count);
		ms->fpos += count;
	}
	return count;
}
/* }}} */


/* {{{ */
static ssize_t crx_stream_memory_read(crx_stream *stream, char *buf, size_t count)
{
	crx_stream_memory_data *ms = (crx_stream_memory_data*)stream->abstract;
	assert(ms != NULL);

	if (ms->fpos >= ZSTR_LEN(ms->data)) {
		stream->eof = 1;
		count = 0;
	} else {
		if (ms->fpos + count > ZSTR_LEN(ms->data)) {
			count = ZSTR_LEN(ms->data) - ms->fpos;
		}
		if (count) {
			CREX_ASSERT(buf != NULL);
			memcpy(buf, ZSTR_VAL(ms->data) + ms->fpos, count);
			ms->fpos += count;
		}
	}
	return count;
}
/* }}} */


/* {{{ */
static int crx_stream_memory_close(crx_stream *stream, int close_handle)
{
	crx_stream_memory_data *ms = (crx_stream_memory_data*)stream->abstract;
	CREX_ASSERT(ms != NULL);
	crex_string_release(ms->data);
	efree(ms);
	return 0;
}
/* }}} */


/* {{{ */
static int crx_stream_memory_flush(crx_stream *stream)
{
	/* nothing to do here */
	return 0;
}
/* }}} */


/* {{{ */
static int crx_stream_memory_seek(crx_stream *stream, crex_off_t offset, int whence, crex_off_t *newoffs)
{
	crx_stream_memory_data *ms = (crx_stream_memory_data*)stream->abstract;
	assert(ms != NULL);

	switch(whence) {
		case SEEK_CUR:
			if (offset < 0) {
				if (ms->fpos < (size_t)(-offset)) {
					ms->fpos = 0;
					*newoffs = -1;
					return -1;
				} else {
					ms->fpos = ms->fpos + offset;
					*newoffs = ms->fpos;
					stream->eof = 0;
					return 0;
				}
			} else {
				stream->eof = 0;
				ms->fpos = ms->fpos + offset;
				*newoffs = ms->fpos;
				return 0;
			}
		case SEEK_SET:
			if (offset < 0) {
				ms->fpos = 0;
				*newoffs = -1;
				return -1;
			} else {
				ms->fpos = offset;
				*newoffs = ms->fpos;
				stream->eof = 0;
				return 0;
			}
		case SEEK_END:
			if (offset > 0) {
				ms->fpos = ZSTR_LEN(ms->data) + offset;
				*newoffs = ms->fpos;
				stream->eof = 0;
				return 0;
			} else if (ZSTR_LEN(ms->data) < (size_t)(-offset)) {
				ms->fpos = 0;
				*newoffs = -1;
				return -1;
			} else {
				ms->fpos = ZSTR_LEN(ms->data) + offset;
				*newoffs = ms->fpos;
				stream->eof = 0;
				return 0;
			}
		default:
			*newoffs = ms->fpos;
			return -1;
	}
}
/* }}} */

/* {{{ */
static int crx_stream_memory_cast(crx_stream *stream, int castas, void **ret)
{
	return FAILURE;
}
/* }}} */

static int crx_stream_memory_stat(crx_stream *stream, crx_stream_statbuf *ssb) /* {{{ */
{
	time_t timestamp = 0;
	crx_stream_memory_data *ms = (crx_stream_memory_data*)stream->abstract;
	assert(ms != NULL);

	memset(ssb, 0, sizeof(crx_stream_statbuf));
	/* read-only across the board */

	ssb->sb.st_mode = ms->mode & TEMP_STREAM_READONLY ? 0444 : 0666;

	ssb->sb.st_size = ZSTR_LEN(ms->data);
	ssb->sb.st_mode |= S_IFREG; /* regular file */
	ssb->sb.st_mtime = timestamp;
	ssb->sb.st_atime = timestamp;
	ssb->sb.st_ctime = timestamp;
	ssb->sb.st_nlink = 1;
	ssb->sb.st_rdev = -1;
	/* this is only for APC, so use /dev/null device - no chance of conflict there! */
	ssb->sb.st_dev = 0xC;
	/* generate unique inode number for alias/filename, so no crxas will conflict */
	ssb->sb.st_ino = 0;

#ifndef CRX_WIN32
	ssb->sb.st_blksize = -1;
	ssb->sb.st_blocks = -1;
#endif

	return 0;
}
/* }}} */

static int crx_stream_memory_set_option(crx_stream *stream, int option, int value, void *ptrparam) /* {{{ */
{
	crx_stream_memory_data *ms = (crx_stream_memory_data*)stream->abstract;
	size_t newsize;

	switch(option) {
		case CRX_STREAM_OPTION_TRUNCATE_API:
			switch (value) {
				case CRX_STREAM_TRUNCATE_SUPPORTED:
					return CRX_STREAM_OPTION_RETURN_OK;

				case CRX_STREAM_TRUNCATE_SET_SIZE:
					if (ms->mode & TEMP_STREAM_READONLY) {
						return CRX_STREAM_OPTION_RETURN_ERR;
					}
					newsize = *(size_t*)ptrparam;
					if (newsize <= ZSTR_LEN(ms->data)) {
						ms->data = crex_string_truncate(ms->data, newsize, 0);
						if (newsize < ms->fpos) {
							ms->fpos = newsize;
						}
					} else {
						size_t old_size = ZSTR_LEN(ms->data);
						ms->data = crex_string_realloc(ms->data, newsize, 0);
						memset(ZSTR_VAL(ms->data) + old_size, 0, newsize - old_size);
					}
					return CRX_STREAM_OPTION_RETURN_OK;
			}
	}

	return CRX_STREAM_OPTION_RETURN_NOTIMPL;
}
/* }}} */

CRXAPI const crx_stream_ops	crx_stream_memory_ops = {
	crx_stream_memory_write, crx_stream_memory_read,
	crx_stream_memory_close, crx_stream_memory_flush,
	"MEMORY",
	crx_stream_memory_seek,
	crx_stream_memory_cast,
	crx_stream_memory_stat,
	crx_stream_memory_set_option
};

/* {{{ */
CRXAPI int crx_stream_mode_from_str(const char *mode)
{
	if (strpbrk(mode, "a")) {
		return TEMP_STREAM_APPEND;
	} else if (strpbrk(mode, "w+")) {
		return TEMP_STREAM_DEFAULT;
	}
	return TEMP_STREAM_READONLY;
}
/* }}} */

/* {{{ */
CRXAPI const char *_crx_stream_mode_to_str(int mode)
{
	if (mode == TEMP_STREAM_READONLY) {
		return "rb";
	} else if (mode == TEMP_STREAM_APPEND) {
		return "a+b";
	}
	return "w+b";
}
/* }}} */

/* {{{ */
CRXAPI crx_stream *_crx_stream_memory_create(int mode STREAMS_DC)
{
	crx_stream_memory_data *self;
	crx_stream *stream;

	self = emalloc(sizeof(*self));
	self->data = ZSTR_EMPTY_ALLOC();
	self->fpos = 0;
	self->mode = mode;

	stream = crx_stream_alloc_rel(&crx_stream_memory_ops, self, 0, _crx_stream_mode_to_str(mode));
	stream->flags |= CRX_STREAM_FLAG_NO_BUFFER;
	return stream;
}
/* }}} */


/* {{{ */
CRXAPI crx_stream *_crx_stream_memory_open(int mode, crex_string *buf STREAMS_DC)
{
	crx_stream *stream;
	crx_stream_memory_data *ms;

	if ((stream = crx_stream_memory_create_rel(mode)) != NULL) {
		ms = (crx_stream_memory_data*)stream->abstract;
		ms->data = crex_string_copy(buf);
	}
	return stream;
}
/* }}} */


/* {{{ */
CRXAPI crex_string *_crx_stream_memory_get_buffer(crx_stream *stream STREAMS_DC)
{
	crx_stream_memory_data *ms = (crx_stream_memory_data*)stream->abstract;
	CREX_ASSERT(ms != NULL);
	return ms->data;
}
/* }}} */

/* }}} */

/* {{{ ------- TEMP stream implementation -------*/

typedef struct {
	crx_stream  *innerstream;
	size_t      smax;
	int			mode;
	zval        meta;
	char*		tmpdir;
} crx_stream_temp_data;


/* {{{ */
static ssize_t crx_stream_temp_write(crx_stream *stream, const char *buf, size_t count)
{
	crx_stream_temp_data *ts = (crx_stream_temp_data*)stream->abstract;
	assert(ts != NULL);

	if (!ts->innerstream) {
		return -1;
	}
	if (crx_stream_is(ts->innerstream, CRX_STREAM_IS_MEMORY)) {
		crex_off_t pos = crx_stream_tell(ts->innerstream);

		if (pos + count >= ts->smax) {
			crex_string *membuf = crx_stream_memory_get_buffer(ts->innerstream);
			crx_stream *file = crx_stream_fopen_temporary_file(ts->tmpdir, "crx", NULL);
			if (file == NULL) {
				crx_error_docref(NULL, E_WARNING, "Unable to create temporary file, Check permissions in temporary files directory.");
				return 0;
			}
			crx_stream_write(file, ZSTR_VAL(membuf), ZSTR_LEN(membuf));
			crx_stream_free_enclosed(ts->innerstream, CRX_STREAM_FREE_CLOSE);
			ts->innerstream = file;
			crx_stream_encloses(stream, ts->innerstream);
			crx_stream_seek(ts->innerstream, pos, SEEK_SET);
		}
	}
	return crx_stream_write(ts->innerstream, buf, count);
}
/* }}} */


/* {{{ */
static ssize_t crx_stream_temp_read(crx_stream *stream, char *buf, size_t count)
{
	crx_stream_temp_data *ts = (crx_stream_temp_data*)stream->abstract;
	size_t got;

	assert(ts != NULL);

	if (!ts->innerstream) {
		return -1;
	}

	got = crx_stream_read(ts->innerstream, buf, count);

	stream->eof = ts->innerstream->eof;

	return got;
}
/* }}} */


/* {{{ */
static int crx_stream_temp_close(crx_stream *stream, int close_handle)
{
	crx_stream_temp_data *ts = (crx_stream_temp_data*)stream->abstract;
	int ret;

	assert(ts != NULL);

	if (ts->innerstream) {
		ret = crx_stream_free_enclosed(ts->innerstream, CRX_STREAM_FREE_CLOSE | (close_handle ? 0 : CRX_STREAM_FREE_PRESERVE_HANDLE));
	} else {
		ret = 0;
	}

	zval_ptr_dtor(&ts->meta);

	if (ts->tmpdir) {
		efree(ts->tmpdir);
	}

	efree(ts);

	return ret;
}
/* }}} */


/* {{{ */
static int crx_stream_temp_flush(crx_stream *stream)
{
	crx_stream_temp_data *ts = (crx_stream_temp_data*)stream->abstract;
	assert(ts != NULL);

	return ts->innerstream ? crx_stream_flush(ts->innerstream) : -1;
}
/* }}} */


/* {{{ */
static int crx_stream_temp_seek(crx_stream *stream, crex_off_t offset, int whence, crex_off_t *newoffs)
{
	crx_stream_temp_data *ts = (crx_stream_temp_data*)stream->abstract;
	int ret;

	assert(ts != NULL);

	if (!ts->innerstream) {
		*newoffs = -1;
		return -1;
	}
	ret = crx_stream_seek(ts->innerstream, offset, whence);
	*newoffs = crx_stream_tell(ts->innerstream);
	stream->eof = ts->innerstream->eof;

	return ret;
}
/* }}} */

/* {{{ */
static int crx_stream_temp_cast(crx_stream *stream, int castas, void **ret)
{
	crx_stream_temp_data *ts = (crx_stream_temp_data*)stream->abstract;
	crx_stream *file;
	crex_string *membuf;
	crex_off_t pos;

	assert(ts != NULL);

	if (!ts->innerstream) {
		return FAILURE;
	}
	if (crx_stream_is(ts->innerstream, CRX_STREAM_IS_STDIO)) {
		return crx_stream_cast(ts->innerstream, castas, ret, 0);
	}

	/* we are still using a memory based backing. If they are if we can be
	 * a FILE*, say yes because we can perform the conversion.
	 * If they actually want to perform the conversion, we need to switch
	 * the memory stream to a tmpfile stream */

	if (ret == NULL && castas == CRX_STREAM_AS_STDIO) {
		return SUCCESS;
	}

	/* say "no" to other stream forms */
	if (ret == NULL) {
		return FAILURE;
	}

	file = crx_stream_fopen_tmpfile();
	if (file == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to create temporary file.");
		return FAILURE;
	}

	/* perform the conversion and then pass the request on to the innerstream */
	membuf = crx_stream_memory_get_buffer(ts->innerstream);
	crx_stream_write(file, ZSTR_VAL(membuf), ZSTR_LEN(membuf));
	pos = crx_stream_tell(ts->innerstream);

	crx_stream_free_enclosed(ts->innerstream, CRX_STREAM_FREE_CLOSE);
	ts->innerstream = file;
	crx_stream_encloses(stream, ts->innerstream);
	crx_stream_seek(ts->innerstream, pos, SEEK_SET);

	return crx_stream_cast(ts->innerstream, castas, ret, 1);
}
/* }}} */

static int crx_stream_temp_stat(crx_stream *stream, crx_stream_statbuf *ssb) /* {{{ */
{
	crx_stream_temp_data *ts = (crx_stream_temp_data*)stream->abstract;

	if (!ts || !ts->innerstream) {
		return -1;
	}
	return crx_stream_stat(ts->innerstream, ssb);
}
/* }}} */

static int crx_stream_temp_set_option(crx_stream *stream, int option, int value, void *ptrparam) /* {{{ */
{
	crx_stream_temp_data *ts = (crx_stream_temp_data*)stream->abstract;

	switch(option) {
		case CRX_STREAM_OPTION_META_DATA_API:
			if (C_TYPE(ts->meta) != IS_UNDEF) {
				crex_hash_copy(C_ARRVAL_P((zval*)ptrparam), C_ARRVAL(ts->meta), zval_add_ref);
			}
			return CRX_STREAM_OPTION_RETURN_OK;
		default:
			if (ts->innerstream) {
				return crx_stream_set_option(ts->innerstream, option, value, ptrparam);
			}
			return CRX_STREAM_OPTION_RETURN_NOTIMPL;
	}
}
/* }}} */

CRXAPI const crx_stream_ops	crx_stream_temp_ops = {
	crx_stream_temp_write, crx_stream_temp_read,
	crx_stream_temp_close, crx_stream_temp_flush,
	"TEMP",
	crx_stream_temp_seek,
	crx_stream_temp_cast,
	crx_stream_temp_stat,
	crx_stream_temp_set_option
};

/* }}} */

/* {{{ _crx_stream_temp_create_ex */
CRXAPI crx_stream *_crx_stream_temp_create_ex(int mode, size_t max_memory_usage, const char *tmpdir STREAMS_DC)
{
	crx_stream_temp_data *self;
	crx_stream *stream;

	self = ecalloc(1, sizeof(*self));
	self->smax = max_memory_usage;
	self->mode = mode;
	ZVAL_UNDEF(&self->meta);
	if (tmpdir) {
		self->tmpdir = estrdup(tmpdir);
	}
	stream = crx_stream_alloc_rel(&crx_stream_temp_ops, self, 0, _crx_stream_mode_to_str(mode));
	stream->flags |= CRX_STREAM_FLAG_NO_BUFFER;
	self->innerstream = crx_stream_memory_create_rel(mode);
	crx_stream_encloses(stream, self->innerstream);

	return stream;
}
/* }}} */

/* {{{ _crx_stream_temp_create */
CRXAPI crx_stream *_crx_stream_temp_create(int mode, size_t max_memory_usage STREAMS_DC)
{
	return crx_stream_temp_create_ex(mode, max_memory_usage, NULL);
}
/* }}} */

/* {{{ _crx_stream_temp_open */
CRXAPI crx_stream *_crx_stream_temp_open(int mode, size_t max_memory_usage, const char *buf, size_t length STREAMS_DC)
{
	crx_stream *stream;
	crx_stream_temp_data *ts;
	crex_off_t newoffs;

	if ((stream = crx_stream_temp_create_rel(mode, max_memory_usage)) != NULL) {
		if (length) {
			assert(buf != NULL);
			crx_stream_temp_write(stream, buf, length);
			crx_stream_temp_seek(stream, 0, SEEK_SET, &newoffs);
		}
		ts = (crx_stream_temp_data*)stream->abstract;
		assert(ts != NULL);
		ts->mode = mode;
	}
	return stream;
}
/* }}} */

CRXAPI const crx_stream_ops crx_stream_rfc2397_ops = {
	NULL, crx_stream_temp_read,
	crx_stream_temp_close, crx_stream_temp_flush,
	"RFC2397",
	crx_stream_temp_seek,
	crx_stream_temp_cast,
	crx_stream_temp_stat,
	crx_stream_temp_set_option
};

static crx_stream * crx_stream_url_wrap_rfc2397(crx_stream_wrapper *wrapper, const char *path,
												const char *mode, int options, crex_string **opened_path,
												crx_stream_context *context STREAMS_DC) /* {{{ */
{
	crx_stream *stream;
	crx_stream_temp_data *ts;
	char *comma, *semi, *sep;
	size_t mlen, dlen, plen, vlen, ilen;
	crex_off_t newoffs;
	zval meta;
	int base64 = 0;
	crex_string *base64_comma = NULL;

	CREX_ASSERT(mode);

	ZVAL_NULL(&meta);
	if (memcmp(path, "data:", 5)) {
		return NULL;
	}

	path += 5;
	dlen = strlen(path);

	if (dlen >= 2 && path[0] == '/' && path[1] == '/') {
		dlen -= 2;
		path += 2;
	}

	if ((comma = memchr(path, ',', dlen)) == NULL) {
		crx_stream_wrapper_log_error(wrapper, options, "rfc2397: no comma in URL");
		return NULL;
	}

	if (comma != path) {
		/* meta info */
		mlen = comma - path;
		dlen -= mlen;
		semi = memchr(path, ';', mlen);
		sep = memchr(path, '/', mlen);

		if (!semi && !sep) {
			crx_stream_wrapper_log_error(wrapper, options, "rfc2397: illegal media type");
			return NULL;
		}

		array_init(&meta);
		if (!semi) { /* there is only a mime type */
			add_assoc_stringl(&meta, "mediatype", (char *) path, mlen);
			mlen = 0;
		} else if (sep && sep < semi) { /* there is a mime type */
			plen = semi - path;
			add_assoc_stringl(&meta, "mediatype", (char *) path, plen);
			mlen -= plen;
			path += plen;
		} else if (semi != path || mlen != sizeof(";base64")-1 || memcmp(path, ";base64", sizeof(";base64")-1)) { /* must be error since parameters are only allowed after mediatype */
			zval_ptr_dtor(&meta);
			crx_stream_wrapper_log_error(wrapper, options, "rfc2397: illegal media type");
			return NULL;
		}
		/* get parameters and potentially ';base64' */
		while(semi && (semi == path)) {
			path++;
			mlen--;
			sep = memchr(path, '=', mlen);
			semi = memchr(path, ';', mlen);
			if (!sep || (semi && semi < sep)) { /* must be ';base64' or failure */
				if (mlen != sizeof("base64")-1 || memcmp(path, "base64", sizeof("base64")-1)) {
					/* must be error since parameters are only allowed after mediatype and we have no '=' sign */
					zval_ptr_dtor(&meta);
					crx_stream_wrapper_log_error(wrapper, options, "rfc2397: illegal parameter");
					return NULL;
				}
				base64 = 1;
				mlen -= sizeof("base64") - 1;
				path += sizeof("base64") - 1;
				break;
			}
			/* found parameter ... the heart of cs ppl lies in +1/-1 or was it +2 this time? */
			plen = sep - path;
			vlen = (semi ? (size_t)(semi - sep) : (mlen - plen)) - 1 /* '=' */;
			if (plen != sizeof("mediatype")-1 || memcmp(path, "mediatype", sizeof("mediatype")-1)) {
				add_assoc_stringl_ex(&meta, path, plen, sep + 1, vlen);
			}
			plen += vlen + 1;
			mlen -= plen;
			path += plen;
		}
		if (mlen) {
			zval_ptr_dtor(&meta);
			crx_stream_wrapper_log_error(wrapper, options, "rfc2397: illegal URL");
			return NULL;
		}
	} else {
		array_init(&meta);
	}
	add_assoc_bool(&meta, "base64", base64);

	/* skip ',' */
	comma++;
	dlen--;

	if (base64) {
		base64_comma = crx_base64_decode_ex((const unsigned char *)comma, dlen, 1);
		if (!base64_comma) {
			zval_ptr_dtor(&meta);
			crx_stream_wrapper_log_error(wrapper, options, "rfc2397: unable to decode");
			return NULL;
		}
		comma = ZSTR_VAL(base64_comma);
		ilen = ZSTR_LEN(base64_comma);
	} else {
		comma = estrndup(comma, dlen);
		dlen = crx_url_decode(comma, dlen);
		ilen = dlen;
	}

	if ((stream = crx_stream_temp_create_rel(0, ~0u)) != NULL) {
		/* store data */
		crx_stream_temp_write(stream, comma, ilen);
		crx_stream_temp_seek(stream, 0, SEEK_SET, &newoffs);
		/* set special stream stuff (enforce exact mode) */
		vlen = strlen(mode);
		if (vlen >= sizeof(stream->mode)) {
			vlen = sizeof(stream->mode) - 1;
		}
		memcpy(stream->mode, mode, vlen);
		stream->mode[vlen] = '\0';
		stream->ops = &crx_stream_rfc2397_ops;
		ts = (crx_stream_temp_data*)stream->abstract;
		assert(ts != NULL);
		ts->mode = mode[0] == 'r' && mode[1] != '+' ? TEMP_STREAM_READONLY : 0;
		ZVAL_COPY_VALUE(&ts->meta, &meta);
	}
	if (base64_comma) {
		crex_string_free(base64_comma);
	} else {
		efree(comma);
	}

	return stream;
}

CRXAPI const crx_stream_wrapper_ops crx_stream_rfc2397_wops = {
	crx_stream_url_wrap_rfc2397,
	NULL, /* close */
	NULL, /* fstat */
	NULL, /* stat */
	NULL, /* opendir */
	"RFC2397",
	NULL, /* unlink */
	NULL, /* rename */
	NULL, /* mkdir */
	NULL, /* rmdir */
	NULL, /* stream_metadata */
};

CRXAPI const crx_stream_wrapper crx_stream_rfc2397_wrapper =	{
	&crx_stream_rfc2397_wops,
	NULL,
	1, /* is_url */
};
