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
   | Author: Wez Furlong <wez@thebrainroom.com>, based on work by:        |
   |         Hartmut Holzgraefe <hholzgra@crx.net>                        |
   +----------------------------------------------------------------------+
 */

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include "crx.h"
#include "crx_zlib.h"
#include "fopen_wrappers.h"

#include "main/crx_network.h"

struct crx_gz_stream_data_t	{
	gzFile gz_file;
	crx_stream *stream;
};

static ssize_t crx_gziop_read(crx_stream *stream, char *buf, size_t count)
{
	struct crx_gz_stream_data_t *self = (struct crx_gz_stream_data_t *) stream->abstract;
	int read;

	/* XXX this needs to be looped for the case count > UINT_MAX */
	read = gzread(self->gz_file, buf, count);

	if (gzeof(self->gz_file)) {
		stream->eof = 1;
	}

	return read;
}

static ssize_t crx_gziop_write(crx_stream *stream, const char *buf, size_t count)
{
	struct crx_gz_stream_data_t *self = (struct crx_gz_stream_data_t *) stream->abstract;

	/* XXX this needs to be looped for the case count > UINT_MAX */
	return gzwrite(self->gz_file, (char *) buf, count);
}

static int crx_gziop_seek(crx_stream *stream, crex_off_t offset, int whence, crex_off_t *newoffs)
{
	struct crx_gz_stream_data_t *self = (struct crx_gz_stream_data_t *) stream->abstract;

	assert(self != NULL);

	if (whence == SEEK_END) {
		crx_error_docref(NULL, E_WARNING, "SEEK_END is not supported");
		return -1;
	}
	*newoffs = gzseek(self->gz_file, offset, whence);

	return (*newoffs < 0) ? -1 : 0;
}

static int crx_gziop_close(crx_stream *stream, int close_handle)
{
	struct crx_gz_stream_data_t *self = (struct crx_gz_stream_data_t *) stream->abstract;
	int ret = EOF;

	if (close_handle) {
		if (self->gz_file) {
			ret = gzclose(self->gz_file);
			self->gz_file = NULL;
		}
		if (self->stream) {
			crx_stream_close(self->stream);
			self->stream = NULL;
		}
	}
	efree(self);

	return ret;
}

static int crx_gziop_flush(crx_stream *stream)
{
	struct crx_gz_stream_data_t *self = (struct crx_gz_stream_data_t *) stream->abstract;

	return gzflush(self->gz_file, C_SYNC_FLUSH);
}

const crx_stream_ops crx_stream_gzio_ops = {
	crx_gziop_write, crx_gziop_read,
	crx_gziop_close, crx_gziop_flush,
	"ZLIB",
	crx_gziop_seek,
	NULL, /* cast */
	NULL, /* stat */
	NULL  /* set_option */
};

crx_stream *crx_stream_gzopen(crx_stream_wrapper *wrapper, const char *path, const char *mode, int options,
							  crex_string **opened_path, crx_stream_context *context STREAMS_DC)
{
	struct crx_gz_stream_data_t *self;
	crx_stream *stream = NULL, *innerstream = NULL;

	/* sanity check the stream: it can be either read-only or write-only */
	if (strchr(mode, '+')) {
		if (options & REPORT_ERRORS) {
			crx_error_docref(NULL, E_WARNING, "Cannot open a zlib stream for reading and writing at the same time!");
		}
		return NULL;
	}

	if (strncasecmp("compress.zlib://", path, 16) == 0) {
		path += 16;
	} else if (strncasecmp("zlib:", path, 5) == 0) {
		path += 5;
	}

	innerstream = crx_stream_open_wrapper_ex(path, mode, STREAM_MUST_SEEK | options | STREAM_WILL_CAST, opened_path, context);

	if (innerstream) {
		crx_socket_t fd;

		if (SUCCESS == crx_stream_cast(innerstream, CRX_STREAM_AS_FD, (void **) &fd, REPORT_ERRORS)) {
			self = emalloc(sizeof(*self));
			self->stream = innerstream;
			self->gz_file = gzdopen(dup(fd), mode);

			if (self->gz_file) {
				zval *zlevel = context ? crx_stream_context_get_option(context, "zlib", "level") : NULL;
				if (zlevel && (C_OK != gzsetparams(self->gz_file, zval_get_long(zlevel), C_DEFAULT_STRATEGY))) {
					crx_error(E_WARNING, "failed setting compression level");
				}

				stream = crx_stream_alloc_rel(&crx_stream_gzio_ops, self, 0, mode);
				if (stream) {
					stream->flags |= CRX_STREAM_FLAG_NO_BUFFER;
					return stream;
				}

				gzclose(self->gz_file);
			}

			efree(self);
			if (options & REPORT_ERRORS) {
				crx_error_docref(NULL, E_WARNING, "gzopen failed");
			}
		}

		crx_stream_close(innerstream);
	}

	return NULL;
}

static const crx_stream_wrapper_ops gzip_stream_wops = {
	crx_stream_gzopen,
	NULL, /* close */
	NULL, /* stat */
	NULL, /* stat_url */
	NULL, /* opendir */
	"ZLIB",
	NULL, /* unlink */
	NULL, /* rename */
	NULL, /* mkdir */
	NULL, /* rmdir */
	NULL
};

const crx_stream_wrapper crx_stream_gzip_wrapper =	{
	&gzip_stream_wops,
	NULL,
	0, /* is_url */
};
