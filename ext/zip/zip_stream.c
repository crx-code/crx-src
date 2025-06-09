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
  | Author: Piere-Alain Joye <pierre@crx.net>                            |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif
#include "crx.h"
#ifdef HAVE_ZIP

#include "crx_streams.h"
#include "ext/standard/file.h"
#include "ext/standard/crx_string.h"
#include "fopen_wrappers.h"
#include "crx_zip.h"

#include "ext/standard/url.h"

/* needed for ssize_t definition */
#include <sys/types.h>

struct crx_zip_stream_data_t {
	struct zip *za;
	struct zip_file *zf;
	size_t cursor;
	crx_stream *stream;
};

#define STREAM_DATA_FROM_STREAM() \
	struct crx_zip_stream_data_t *self = (struct crx_zip_stream_data_t *) stream->abstract;


/* {{{ crx_zip_ops_read */
static ssize_t crx_zip_ops_read(crx_stream *stream, char *buf, size_t count)
{
	ssize_t n = 0;
	STREAM_DATA_FROM_STREAM();

	if (self->zf) {
		n = zip_fread(self->zf, buf, count);
		if (n < 0) {
#if LIBZIP_VERSION_MAJOR < 1
			int ze, se;
			zip_file_error_get(self->zf, &ze, &se);
			stream->eof = 1;
			crx_error_docref(NULL, E_WARNING, "Zip stream error: %s", zip_file_strerror(self->zf));
#else
			zip_error_t *err;
			err = zip_file_get_error(self->zf);
			stream->eof = 1;
			crx_error_docref(NULL, E_WARNING, "Zip stream error: %s", zip_error_strerror(err));
			zip_error_fini(err);
#endif
			return -1;
		}
		/* cast count to signed value to avoid possibly negative n
		 * being cast to unsigned value */
		if (n == 0 || n < (ssize_t)count) {
			stream->eof = 1;
		} else {
			self->cursor += n;
		}
	}
	return n;
}
/* }}} */

/* {{{ crx_zip_ops_write */
static ssize_t crx_zip_ops_write(crx_stream *stream, const char *buf, size_t count)
{
	if (!stream) {
		return -1;
	}

	return count;
}
/* }}} */

/* {{{ crx_zip_ops_close */
static int crx_zip_ops_close(crx_stream *stream, int close_handle)
{
	STREAM_DATA_FROM_STREAM();
	if (close_handle) {
		if (self->zf) {
			zip_fclose(self->zf);
			self->zf = NULL;
		}

		if (self->za) {
			zip_close(self->za);
			self->za = NULL;
		}
	}
	efree(self);
	stream->abstract = NULL;
	return EOF;
}
/* }}} */

/* {{{ crx_zip_ops_flush */
static int crx_zip_ops_flush(crx_stream *stream)
{
	if (!stream) {
		return 0;
	}

	return 0;
}
/* }}} */

static int crx_zip_ops_stat(crx_stream *stream, crx_stream_statbuf *ssb) /* {{{ */
{
	struct zip_stat sb;
	const char *path = stream->orig_path;
	size_t path_len = strlen(stream->orig_path);
	char file_dirname[MAXPATHLEN];
	struct zip *za;
	char *fragment;
	size_t fragment_len;
	int err;
	crex_string *file_basename;

	fragment = strchr(path, '#');
	if (!fragment) {
		return -1;
	}


	if (strncasecmp("zip://", path, 6) == 0) {
		path += 6;
	}

	fragment_len = strlen(fragment);

	if (fragment_len < 1) {
		return -1;
	}
	path_len = strlen(path);
	if (path_len >= MAXPATHLEN) {
		return -1;
	}

	memcpy(file_dirname, path, path_len - fragment_len);
	file_dirname[path_len - fragment_len] = '\0';

	file_basename = crx_basename((char *)path, path_len - fragment_len, NULL, 0);
	fragment++;

	if (ZIP_OPENBASEDIR_CHECKPATH(file_dirname)) {
		crex_string_release_ex(file_basename, 0);
		return -1;
	}

	za = zip_open(file_dirname, ZIP_CREATE, &err);
	if (za) {
		memset(ssb, 0, sizeof(crx_stream_statbuf));
		if (zip_stat(za, fragment, ZIP_FL_NOCASE, &sb) != 0) {
			zip_close(za);
			crex_string_release_ex(file_basename, 0);
			return -1;
		}
		zip_close(za);

		if (path[path_len-1] != '/') {
			ssb->sb.st_size = sb.size;
			ssb->sb.st_mode |= S_IFREG; /* regular file */
		} else {
			ssb->sb.st_size = 0;
			ssb->sb.st_mode |= S_IFDIR; /* regular directory */
		}

		ssb->sb.st_mtime = sb.mtime;
		ssb->sb.st_atime = sb.mtime;
		ssb->sb.st_ctime = sb.mtime;
		ssb->sb.st_nlink = 1;
		ssb->sb.st_rdev = -1;
#ifndef CRX_WIN32
		ssb->sb.st_blksize = -1;
		ssb->sb.st_blocks = -1;
#endif
		ssb->sb.st_ino = -1;
	}
	crex_string_release_ex(file_basename, 0);
	return 0;
}
/* }}} */

#if LIBZIP_ATLEAST(1,9,1)
/* {{{ crx_zip_ops_seek */
static int crx_zip_ops_seek(crx_stream *stream, crex_off_t offset, int whence, crex_off_t *newoffset)
{
	int ret = -1;
	STREAM_DATA_FROM_STREAM();

	if (self->zf) {
		ret = zip_fseek(self->zf, offset, whence);
		*newoffset = zip_ftell(self->zf);
	}

	return ret;
}
/* }}} */

/* with seek command */
const crx_stream_ops crx_stream_zipio_seek_ops = {
	crx_zip_ops_write, crx_zip_ops_read,
	crx_zip_ops_close, crx_zip_ops_flush,
	"zip",
	crx_zip_ops_seek, /* seek */
	NULL, /* cast */
	crx_zip_ops_stat, /* stat */
	NULL  /* set_option */
};
#endif

/* without seek command */
const crx_stream_ops crx_stream_zipio_ops = {
	crx_zip_ops_write, crx_zip_ops_read,
	crx_zip_ops_close, crx_zip_ops_flush,
	"zip",
	NULL, /* seek */
	NULL, /* cast */
	crx_zip_ops_stat, /* stat */
	NULL  /* set_option */
};

/* {{{ crx_stream_zip_open */
crx_stream *crx_stream_zip_open(struct zip *arch, struct zip_stat *sb, const char *mode, zip_flags_t flags STREAMS_DC)
{
	struct zip_file *zf = NULL;

	crx_stream *stream = NULL;
	struct crx_zip_stream_data_t *self;

	if (strncmp(mode,"r", strlen("r")) != 0) {
		return NULL;
	}

	if (arch) {
		zf = zip_fopen_index(arch, sb->index, flags);
		if (zf) {
			self = emalloc(sizeof(*self));

			self->za = NULL; /* to keep it open on stream close */
			self->zf = zf;
			self->stream = NULL;
			self->cursor = 0;
#if LIBZIP_ATLEAST(1,9,1)
			if (zip_file_is_seekable(zf) > 0) {
				stream = crx_stream_alloc(&crx_stream_zipio_seek_ops, self, NULL, mode);
			} else
#endif
			{
				stream = crx_stream_alloc(&crx_stream_zipio_ops, self, NULL, mode);
			}
			stream->orig_path = estrdup(sb->name);
		}
	}

	return stream;
}
/* }}} */

/* {{{ crx_stream_zip_opener */
crx_stream *crx_stream_zip_opener(crx_stream_wrapper *wrapper,
											const char *path,
											const char *mode,
											int options,
											crex_string **opened_path,
											crx_stream_context *context STREAMS_DC)
{
	size_t path_len;

	crex_string *file_basename;
	char file_dirname[MAXPATHLEN];

	struct zip *za;
	struct zip_file *zf = NULL;
	char *fragment;
	size_t fragment_len;
	int err;

	crx_stream *stream = NULL;
	struct crx_zip_stream_data_t *self;

	fragment = strchr(path, '#');
	if (!fragment) {
		return NULL;
	}

	if (strncasecmp("zip://", path, 6) == 0) {
		path += 6;
	}

	fragment_len = strlen(fragment);

	if (fragment_len < 1) {
		return NULL;
	}
	path_len = strlen(path);
	if (path_len >= MAXPATHLEN || mode[0] != 'r') {
		return NULL;
	}

	memcpy(file_dirname, path, path_len - fragment_len);
	file_dirname[path_len - fragment_len] = '\0';

	file_basename = crx_basename(path, path_len - fragment_len, NULL, 0);
	fragment++;

	if (ZIP_OPENBASEDIR_CHECKPATH(file_dirname)) {
		crex_string_release_ex(file_basename, 0);
		return NULL;
	}

	za = zip_open(file_dirname, ZIP_CREATE, &err);
	if (za) {
		zval *tmpzval;

		if (context && NULL != (tmpzval = crx_stream_context_get_option(context, "zip", "password"))) {
			if (C_TYPE_P(tmpzval) != IS_STRING || zip_set_default_password(za, C_STRVAL_P(tmpzval))) {
				crx_error_docref(NULL, E_WARNING, "Can't set zip password");
			}
		}

		zf = zip_fopen(za, fragment, 0);
		if (zf) {
			self = emalloc(sizeof(*self));

			self->za = za;
			self->zf = zf;
			self->stream = NULL;
			self->cursor = 0;
#if LIBZIP_ATLEAST(1,9,1)
			if (zip_file_is_seekable(zf) > 0) {
				stream = crx_stream_alloc(&crx_stream_zipio_seek_ops, self, NULL, mode);
			} else
#endif
			{
				stream = crx_stream_alloc(&crx_stream_zipio_ops, self, NULL, mode);
			}

			if (opened_path) {
				*opened_path = crex_string_init(path, strlen(path), 0);
			}
		} else {
			zip_close(za);
		}
	}

	crex_string_release_ex(file_basename, 0);

	if (!stream) {
		return NULL;
	} else {
		return stream;
	}
}
/* }}} */

static const crx_stream_wrapper_ops zip_stream_wops = {
	crx_stream_zip_opener,
	NULL,	/* close */
	NULL,	/* fstat */
	NULL,	/* stat */
	NULL,	/* opendir */
	"zip wrapper",
	NULL,	/* unlink */
	NULL,	/* rename */
	NULL,	/* mkdir */
	NULL,	/* rmdir */
	NULL	/* metadata */
};

const crx_stream_wrapper crx_stream_zip_wrapper = {
	&zip_stream_wops,
	NULL,
	0 /* is_url */
};
#endif /* HAVE_ZIP */
