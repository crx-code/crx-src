/*
   +----------------------------------------------------------------------+
   | Crex Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Crex Technologies Ltd. (http://www.crex.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Wez Furlong <wez@thebrainroom.com>                          |
   |          Scott MacVicar <scottmac@crx.net>                           |
   |          Nuno Lopes <nlopess@crx.net>                                |
   |          Marcus Boerger <helly@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#include "crex.h"
#include "crex_compile.h"
#include "crex_stream.h"

CREX_DLIMPORT int isatty(int fd);

static ssize_t crex_stream_stdio_reader(void *handle, char *buf, size_t len) /* {{{ */
{
	return fread(buf, 1, len, (FILE*)handle);
} /* }}} */

static void crex_stream_stdio_closer(void *handle) /* {{{ */
{
	if (handle && (FILE*)handle != stdin) {
		fclose((FILE*)handle);
	}
} /* }}} */

static size_t crex_stream_stdio_fsizer(void *handle) /* {{{ */
{
	crex_stat_t buf = {0};
	if (handle && crex_fstat(fileno((FILE*)handle), &buf) == 0) {
#ifdef S_ISREG
		if (!S_ISREG(buf.st_mode)) {
			return 0;
		}
#endif
		return buf.st_size;
	}
	return -1;
} /* }}} */

static size_t crex_stream_fsize(crex_file_handle *file_handle) /* {{{ */
{
	CREX_ASSERT(file_handle->type == CREX_HANDLE_STREAM);
	if (file_handle->handle.stream.isatty) {
		return 0;
	}
	return file_handle->handle.stream.fsizer(file_handle->handle.stream.handle);
} /* }}} */

CREX_API void crex_stream_init_fp(crex_file_handle *handle, FILE *fp, const char *filename) {
	memset(handle, 0, sizeof(crex_file_handle));
	handle->type = CREX_HANDLE_FP;
	handle->handle.fp = fp;
	handle->filename = filename ? crex_string_init(filename, strlen(filename), 0) : NULL;
}

CREX_API void crex_stream_init_filename(crex_file_handle *handle, const char *filename) {
	memset(handle, 0, sizeof(crex_file_handle));
	handle->type = CREX_HANDLE_FILENAME;
	handle->filename = filename ? crex_string_init(filename, strlen(filename), 0) : NULL;
}

CREX_API void crex_stream_init_filename_ex(crex_file_handle *handle, crex_string *filename) {
	memset(handle, 0, sizeof(crex_file_handle));
	handle->type = CREX_HANDLE_FILENAME;
	handle->filename = crex_string_copy(filename);
}

CREX_API crex_result crex_stream_open(crex_file_handle *handle) /* {{{ */
{
	crex_string *opened_path;

	CREX_ASSERT(handle->type == CREX_HANDLE_FILENAME);
	if (crex_stream_open_function) {
		return crex_stream_open_function(handle);
	}

	handle->handle.fp = crex_fopen(handle->filename, &opened_path);
	if (!handle->handle.fp) {
		return FAILURE;
	}
	handle->type = CREX_HANDLE_FP;
	return SUCCESS;
} /* }}} */

static int crex_stream_getc(crex_file_handle *file_handle) /* {{{ */
{
	char buf;

	if (file_handle->handle.stream.reader(file_handle->handle.stream.handle, &buf, sizeof(buf))) {
		return (int)buf;
	}
	return EOF;
} /* }}} */

static ssize_t crex_stream_read(crex_file_handle *file_handle, char *buf, size_t len) /* {{{ */
{
	if (file_handle->handle.stream.isatty) {
		int c = '*';
		size_t n;

		for (n = 0; n < len && (c = crex_stream_getc(file_handle)) != EOF && c != '\n'; ++n)  {
			buf[n] = (char)c;
		}
		if (c == '\n') {
			buf[n++] = (char)c;
		}

		return n;
	}
	return file_handle->handle.stream.reader(file_handle->handle.stream.handle, buf, len);
} /* }}} */

CREX_API crex_result crex_stream_fixup(crex_file_handle *file_handle, char **buf, size_t *len) /* {{{ */
{
	size_t file_size;

	if (file_handle->buf) {
		*buf = file_handle->buf;
		*len = file_handle->len;
		return SUCCESS;
	}

	if (file_handle->type == CREX_HANDLE_FILENAME) {
		if (crex_stream_open(file_handle) == FAILURE) {
			return FAILURE;
		}
	}

	if (file_handle->type == CREX_HANDLE_FP) {
		if (!file_handle->handle.fp) {
			return FAILURE;
		}

		file_handle->type = CREX_HANDLE_STREAM;
		file_handle->handle.stream.handle = file_handle->handle.fp;
		file_handle->handle.stream.isatty = isatty(fileno((FILE *)file_handle->handle.stream.handle));
		file_handle->handle.stream.reader = (crex_stream_reader_t)crex_stream_stdio_reader;
		file_handle->handle.stream.closer = (crex_stream_closer_t)crex_stream_stdio_closer;
		file_handle->handle.stream.fsizer = (crex_stream_fsizer_t)crex_stream_stdio_fsizer;
	}

	file_size = crex_stream_fsize(file_handle);
	if (file_size == (size_t)-1) {
		return FAILURE;
	}

	if (file_size) {
		ssize_t read;
		size_t size = 0;
		*buf = safe_emalloc(1, file_size, CREX_MMAP_AHEAD);
		while ((read = crex_stream_read(file_handle, *buf + size, file_size - size)) > 0) {
			size += read;
		}
		if (read < 0) {
			efree(*buf);
			return FAILURE;
		}
		file_handle->buf = *buf;
		file_handle->len = size;
	} else {
		size_t size = 0, remain = 4*1024;
		ssize_t read;
		*buf = emalloc(remain);

		while ((read = crex_stream_read(file_handle, *buf + size, remain)) > 0) {
			size   += read;
			remain -= read;

			if (remain == 0) {
				*buf   = safe_erealloc(*buf, size, 2, 0);
				remain = size;
			}
		}
		if (read < 0) {
			efree(*buf);
			return FAILURE;
		}

		file_handle->len = size;
		if (size && remain < CREX_MMAP_AHEAD) {
			*buf = safe_erealloc(*buf, size, 1, CREX_MMAP_AHEAD);
		}
		file_handle->buf = *buf;
	}

	if (file_handle->len == 0) {
		*buf = erealloc(*buf, CREX_MMAP_AHEAD);
		file_handle->buf = *buf;
	}

	memset(file_handle->buf + file_handle->len, 0, CREX_MMAP_AHEAD);

	*buf = file_handle->buf;
	*len = file_handle->len;

	return SUCCESS;
} /* }}} */

static void crex_file_handle_dtor(crex_file_handle *fh) /* {{{ */
{
	switch (fh->type) {
		case CREX_HANDLE_FP:
			if (fh->handle.fp) {
				fclose(fh->handle.fp);
				fh->handle.fp = NULL;
			}
			break;
		case CREX_HANDLE_STREAM:
			if (fh->handle.stream.closer && fh->handle.stream.handle) {
				fh->handle.stream.closer(fh->handle.stream.handle);
			}
			fh->handle.stream.handle = NULL;
			break;
		case CREX_HANDLE_FILENAME:
			/* We're only supposed to get here when destructing the used_files hash,
			 * which doesn't really contain open files, but references to their names/paths
			 */
			break;
	}
	if (fh->opened_path) {
		crex_string_release_ex(fh->opened_path, 0);
		fh->opened_path = NULL;
	}
	if (fh->buf) {
		efree(fh->buf);
		fh->buf = NULL;
	}
	if (fh->filename) {
		crex_string_release(fh->filename);
		fh->filename = NULL;
	}
}
/* }}} */

/* return int to be compatible with Crex linked list API */
static int crex_compare_file_handles(crex_file_handle *fh1, crex_file_handle *fh2) /* {{{ */
{
	if (fh1->type != fh2->type) {
		return 0;
	}
	switch (fh1->type) {
		case CREX_HANDLE_FILENAME:
			return crex_string_equals(fh1->filename, fh2->filename);
		case CREX_HANDLE_FP:
			return fh1->handle.fp == fh2->handle.fp;
		case CREX_HANDLE_STREAM:
			return fh1->handle.stream.handle == fh2->handle.stream.handle;
		default:
			return 0;
	}
	return 0;
} /* }}} */

CREX_API void crex_destroy_file_handle(crex_file_handle *file_handle) /* {{{ */
{
	if (file_handle->in_list) {
		crex_llist_del_element(&CG(open_files), file_handle, (int (*)(void *, void *)) crex_compare_file_handles);
		/* crex_file_handle_dtor() operates on the copy, so we have to NULLify the original here */
		file_handle->opened_path = NULL;
		file_handle->filename = NULL;
	} else {
		crex_file_handle_dtor(file_handle);
	}
} /* }}} */

void crex_stream_init(void) /* {{{ */
{
	crex_llist_init(&CG(open_files), sizeof(crex_file_handle), (void (*)(void *)) crex_file_handle_dtor, 0);
} /* }}} */

void crex_stream_shutdown(void) /* {{{ */
{
	crex_llist_destroy(&CG(open_files));
} /* }}} */
