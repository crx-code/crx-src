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
   | Authors: Rasmus Lerdorf <rasmus@crx.net>                             |
   |          Jim Winstead <jimw@crx.net>                                 |
   |          Hartmut Holzgraefe <hholzgra@crx.net>                       |
   +----------------------------------------------------------------------+
 */

#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "crx.h"
#include "crx_globals.h"
#include "crx_standard.h"
#include "crx_memory_streams.h"
#include "crx_fopen_wrappers.h"
#include "SAPI.h"

static ssize_t crx_stream_output_write(crx_stream *stream, const char *buf, size_t count) /* {{{ */
{
	CRXWRITE(buf, count);
	return count;
}
/* }}} */

static ssize_t crx_stream_output_read(crx_stream *stream, char *buf, size_t count) /* {{{ */
{
	stream->eof = 1;
	return -1;
}
/* }}} */

static int crx_stream_output_close(crx_stream *stream, int close_handle) /* {{{ */
{
	return 0;
}
/* }}} */

const crx_stream_ops crx_stream_output_ops = {
	crx_stream_output_write,
	crx_stream_output_read,
	crx_stream_output_close,
	NULL, /* flush */
	"Output",
	NULL, /* seek */
	NULL, /* cast */
	NULL, /* stat */
	NULL  /* set_option */
};

typedef struct crx_stream_input { /* {{{ */
	crx_stream *body;
	crex_off_t position;
} crx_stream_input_t;
/* }}} */

static ssize_t crx_stream_input_write(crx_stream *stream, const char *buf, size_t count) /* {{{ */
{
	return -1;
}
/* }}} */

static ssize_t crx_stream_input_read(crx_stream *stream, char *buf, size_t count) /* {{{ */
{
	crx_stream_input_t *input = stream->abstract;
	ssize_t read;

	if (!SG(post_read) && SG(read_post_bytes) < (int64_t)(input->position + count)) {
		/* read requested data from SAPI */
		size_t read_bytes = sapi_read_post_block(buf, count);

		if (read_bytes > 0) {
			crx_stream_seek(input->body, 0, SEEK_END);
			crx_stream_write(input->body, buf, read_bytes);
		}
	}

	if (!input->body->readfilters.head) {
		/* If the input stream contains filters, it's not really seekable. The
			input->position is likely to be wrong for unfiltered data. */
		crx_stream_seek(input->body, input->position, SEEK_SET);
	}
	read = crx_stream_read(input->body, buf, count);

	if (!read || read == (size_t) -1) {
		stream->eof = 1;
	} else {
		input->position += read;
	}

	return read;
}
/* }}} */

static int crx_stream_input_close(crx_stream *stream, int close_handle) /* {{{ */
{
	efree(stream->abstract);
	stream->abstract = NULL;

	return 0;
}
/* }}} */

static int crx_stream_input_flush(crx_stream *stream) /* {{{ */
{
	return -1;
}
/* }}} */

static int crx_stream_input_seek(crx_stream *stream, crex_off_t offset, int whence, crex_off_t *newoffset) /* {{{ */
{
	crx_stream_input_t *input = stream->abstract;

	if (input->body) {
		int sought = crx_stream_seek(input->body, offset, whence);
		*newoffset = input->position = (input->body)->position;
		return sought;
	}

	return -1;
}
/* }}} */

const crx_stream_ops crx_stream_input_ops = {
	crx_stream_input_write,
	crx_stream_input_read,
	crx_stream_input_close,
	crx_stream_input_flush,
	"Input",
	crx_stream_input_seek,
	NULL, /* cast */
	NULL, /* stat */
	NULL  /* set_option */
};

static void crx_stream_apply_filter_list(crx_stream *stream, char *filterlist, int read_chain, int write_chain) /* {{{ */
{
	char *p, *token = NULL;
	crx_stream_filter *temp_filter;

	p = crx_strtok_r(filterlist, "|", &token);
	while (p) {
		crx_url_decode(p, strlen(p));
		if (read_chain) {
			if ((temp_filter = crx_stream_filter_create(p, NULL, crx_stream_is_persistent(stream)))) {
				crx_stream_filter_append(&stream->readfilters, temp_filter);
			} else {
				crx_error_docref(NULL, E_WARNING, "Unable to create filter (%s)", p);
			}
		}
		if (write_chain) {
			if ((temp_filter = crx_stream_filter_create(p, NULL, crx_stream_is_persistent(stream)))) {
				crx_stream_filter_append(&stream->writefilters, temp_filter);
			} else {
				crx_error_docref(NULL, E_WARNING, "Unable to create filter (%s)", p);
			}
		}
		p = crx_strtok_r(NULL, "|", &token);
	}
}
/* }}} */

crx_stream * crx_stream_url_wrap_crx(crx_stream_wrapper *wrapper, const char *path, const char *mode, int options,
									 crex_string **opened_path, crx_stream_context *context STREAMS_DC) /* {{{ */
{
	int fd = -1;
	int mode_rw = 0;
	crx_stream * stream = NULL;
	char *p, *token = NULL, *pathdup;
	crex_long max_memory;
	FILE *file = NULL;
#ifdef CRX_WIN32
	int pipe_requested = 0;
#endif

	if (!strncasecmp(path, "crx://", 6)) {
		path += 6;
	}

	if (!strncasecmp(path, "temp", 4)) {
		path += 4;
		max_memory = CRX_STREAM_MAX_MEM;
		if (!strncasecmp(path, "/maxmemory:", 11)) {
			path += 11;
			max_memory = CREX_STRTOL(path, NULL, 10);
			if (max_memory < 0) {
				crex_argument_value_error(2, "must be greater than or equal to 0");
				return NULL;
			}
		}
		mode_rw = crx_stream_mode_from_str(mode);
		return crx_stream_temp_create(mode_rw, max_memory);
	}

	if (!strcasecmp(path, "memory")) {
		mode_rw = crx_stream_mode_from_str(mode);
		return crx_stream_memory_create(mode_rw);
	}

	if (!strcasecmp(path, "output")) {
		return crx_stream_alloc(&crx_stream_output_ops, NULL, 0, "wb");
	}

	if (!strcasecmp(path, "input")) {
		crx_stream_input_t *input;

		if ((options & STREAM_OPEN_FOR_INCLUDE) && !PG(allow_url_include) ) {
			if (options & REPORT_ERRORS) {
				crx_error_docref(NULL, E_WARNING, "URL file-access is disabled in the server configuration");
			}
			return NULL;
		}

		input = ecalloc(1, sizeof(*input));
		if ((input->body = SG(request_info).request_body)) {
			crx_stream_rewind(input->body);
		} else {
			input->body = crx_stream_temp_create_ex(TEMP_STREAM_DEFAULT, SAPI_POST_BLOCK_SIZE, PG(upload_tmp_dir));
			SG(request_info).request_body = input->body;
		}

		return crx_stream_alloc(&crx_stream_input_ops, input, 0, "rb");
	}

	if (!strcasecmp(path, "stdin")) {
		if ((options & STREAM_OPEN_FOR_INCLUDE) && !PG(allow_url_include) ) {
			if (options & REPORT_ERRORS) {
				crx_error_docref(NULL, E_WARNING, "URL file-access is disabled in the server configuration");
			}
			return NULL;
		}
		if (!strcmp(sapi_module.name, "cli")) {
			static int cli_in = 0;
			fd = STDIN_FILENO;
			if (cli_in) {
				fd = dup(fd);
			} else {
				cli_in = 1;
				file = stdin;
			}
		} else {
			fd = dup(STDIN_FILENO);
		}
#ifdef CRX_WIN32
		pipe_requested = 1;
#endif
	} else if (!strcasecmp(path, "stdout")) {
		if (!strcmp(sapi_module.name, "cli")) {
			static int cli_out = 0;
			fd = STDOUT_FILENO;
			if (cli_out++) {
				fd = dup(fd);
			} else {
				cli_out = 1;
				file = stdout;
			}
		} else {
			fd = dup(STDOUT_FILENO);
		}
#ifdef CRX_WIN32
		pipe_requested = 1;
#endif
	} else if (!strcasecmp(path, "stderr")) {
		if (!strcmp(sapi_module.name, "cli")) {
			static int cli_err = 0;
			fd = STDERR_FILENO;
			if (cli_err++) {
				fd = dup(fd);
			} else {
				cli_err = 1;
				file = stderr;
			}
		} else {
			fd = dup(STDERR_FILENO);
		}
#ifdef CRX_WIN32
		pipe_requested = 1;
#endif
	} else if (!strncasecmp(path, "fd/", 3)) {
		const char *start;
		char       *end;
		crex_long  fildes_ori;
		int		   dtablesize;

		if (strcmp(sapi_module.name, "cli")) {
			if (options & REPORT_ERRORS) {
				crx_error_docref(NULL, E_WARNING, "Direct access to file descriptors is only available from command-line CRX");
			}
			return NULL;
		}

		if ((options & STREAM_OPEN_FOR_INCLUDE) && !PG(allow_url_include) ) {
			if (options & REPORT_ERRORS) {
				crx_error_docref(NULL, E_WARNING, "URL file-access is disabled in the server configuration");
			}
			return NULL;
		}

		start = &path[3];
		fildes_ori = CREX_STRTOL(start, &end, 10);
		if (end == start || *end != '\0') {
			crx_stream_wrapper_log_error(wrapper, options,
				"crx://fd/ stream must be specified in the form crx://fd/<orig fd>");
			return NULL;
		}

#ifdef HAVE_UNISTD_H
		dtablesize = getdtablesize();
#else
		dtablesize = INT_MAX;
#endif

		if (fildes_ori < 0 || fildes_ori >= dtablesize) {
			crx_stream_wrapper_log_error(wrapper, options,
				"The file descriptors must be non-negative numbers smaller than %d", dtablesize);
			return NULL;
		}

		fd = dup((int)fildes_ori);
		if (fd == -1) {
			crx_stream_wrapper_log_error(wrapper, options,
				"Error duping file descriptor " CREX_LONG_FMT "; possibly it doesn't exist: "
				"[%d]: %s", fildes_ori, errno, strerror(errno));
			return NULL;
		}
	} else if (!strncasecmp(path, "filter/", 7)) {
		/* Save time/memory when chain isn't specified */
		if (strchr(mode, 'r') || strchr(mode, '+')) {
			mode_rw |= CRX_STREAM_FILTER_READ;
		}
		if (strchr(mode, 'w') || strchr(mode, '+') || strchr(mode, 'a')) {
			mode_rw |= CRX_STREAM_FILTER_WRITE;
		}
		pathdup = estrndup(path + 6, strlen(path + 6));
		p = strstr(pathdup, "/resource=");
		if (!p) {
			crex_throw_error(NULL, "No URL resource specified");
			efree(pathdup);
			return NULL;
		}

		if (!(stream = crx_stream_open_wrapper(p + 10, mode, options, opened_path))) {
			efree(pathdup);
			return NULL;
		}

		*p = '\0';

		p = crx_strtok_r(pathdup + 1, "/", &token);
		while (p) {
			if (!strncasecmp(p, "read=", 5)) {
				crx_stream_apply_filter_list(stream, p + 5, 1, 0);
			} else if (!strncasecmp(p, "write=", 6)) {
				crx_stream_apply_filter_list(stream, p + 6, 0, 1);
			} else {
				crx_stream_apply_filter_list(stream, p, mode_rw & CRX_STREAM_FILTER_READ, mode_rw & CRX_STREAM_FILTER_WRITE);
			}
			p = crx_strtok_r(NULL, "/", &token);
		}
		efree(pathdup);

		if (EG(exception)) {
			crx_stream_close(stream);
			return NULL;
		}

		return stream;
	} else {
		/* invalid crx://thingy */
		crx_error_docref(NULL, E_WARNING, "Invalid crx:// URL specified");
		return NULL;
	}

	/* must be stdin, stderr or stdout */
	if (fd == -1)	{
		/* failed to dup */
		return NULL;
	}

#if defined(S_IFSOCK) && !defined(CRX_WIN32)
	do {
		crex_stat_t st = {0};
		memset(&st, 0, sizeof(st));
		if (crex_fstat(fd, &st) == 0 && (st.st_mode & S_IFMT) == S_IFSOCK) {
			stream = crx_stream_sock_open_from_socket(fd, NULL);
			if (stream) {
				stream->ops = &crx_stream_socket_ops;
				return stream;
			}
		}
	} while (0);
#endif

	if (file) {
		stream = crx_stream_fopen_from_file(file, mode);
	} else {
		stream = crx_stream_fopen_from_fd(fd, mode, NULL);
		if (stream == NULL) {
			close(fd);
		}
	}

#ifdef CRX_WIN32
	if (pipe_requested && stream && context) {
		zval *blocking_pipes = crx_stream_context_get_option(context, "pipe", "blocking");
		if (blocking_pipes) {
			crx_stream_set_option(stream, CRX_STREAM_OPTION_PIPE_BLOCKING, zval_get_long(blocking_pipes), NULL);
		}
	}
#endif
	return stream;
}
/* }}} */

static const crx_stream_wrapper_ops crx_stdio_wops = {
	crx_stream_url_wrap_crx,
	NULL, /* close */
	NULL, /* fstat */
	NULL, /* stat */
	NULL, /* opendir */
	"CRX",
	NULL, /* unlink */
	NULL, /* rename */
	NULL, /* mkdir */
	NULL, /* rmdir */
	NULL
};

CRXAPI const crx_stream_wrapper crx_stream_crx_wrapper =	{
	&crx_stdio_wops,
	NULL,
	0, /* is_url */
};
