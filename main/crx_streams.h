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
   | Author: Wez Furlong (wez@thebrainroom.com)                           |
   +----------------------------------------------------------------------+
 */

#ifndef CRX_STREAMS_H
#define CRX_STREAMS_H

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include "crex.h"
#include "crex_stream.h"

BEGIN_EXTERN_C()
CRXAPI int crx_file_le_stream(void);
CRXAPI int crx_file_le_pstream(void);
CRXAPI int crx_file_le_stream_filter(void);
END_EXTERN_C()

/* {{{ Streams memory debugging stuff */

#if CREX_DEBUG
/* these have more of a dependency on the definitions of the crex macros than
 * I would prefer, but doing it this way saves loads of idefs :-/ */
# define STREAMS_D			int __crx_stream_call_depth CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC
# define STREAMS_C			0 CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC
# define STREAMS_REL_C		__crx_stream_call_depth + 1 CREX_FILE_LINE_CC, \
	__crx_stream_call_depth ? __crex_orig_filename : __crex_filename, \
	__crx_stream_call_depth ? __crex_orig_lineno : __crex_lineno

# define STREAMS_DC		, STREAMS_D
# define STREAMS_CC		, STREAMS_C
# define STREAMS_REL_CC	, STREAMS_REL_C

#else
# define STREAMS_D
# define STREAMS_C
# define STREAMS_REL_C
# define STREAMS_DC
# define STREAMS_CC
# define STREAMS_REL_CC
#endif

/* these functions relay the file/line number information. They are depth aware, so they will pass
 * the ultimate ancestor, which is useful, because there can be several layers of calls */
#define crx_stream_alloc_rel(ops, thisptr, persistent, mode) _crx_stream_alloc((ops), (thisptr), (persistent), (mode) STREAMS_REL_CC)

#define crx_stream_copy_to_mem_rel(src, maxlen, persistent) _crx_stream_copy_to_mem((src), (buf), (maxlen), (persistent) STREAMS_REL_CC)

#define crx_stream_fopen_rel(filename, mode, opened, options) _crx_stream_fopen((filename), (mode), (opened), (options) STREAMS_REL_CC)

#define crx_stream_fopen_with_path_rel(filename, mode, path, opened, options) _crx_stream_fopen_with_path((filename), (mode), (path), (opened), (options) STREAMS_REL_CC)

#define crx_stream_fopen_from_fd_rel(fd, mode, persistent_id, zero_position)	 _crx_stream_fopen_from_fd((fd), (mode), (persistent_id), (zero_position) STREAMS_REL_CC)
#define crx_stream_fopen_from_file_rel(file, mode)	 _crx_stream_fopen_from_file((file), (mode) STREAMS_REL_CC)

#define crx_stream_fopen_from_pipe_rel(file, mode)	 _crx_stream_fopen_from_pipe((file), (mode) STREAMS_REL_CC)

#define crx_stream_fopen_tmpfile_rel()	_crx_stream_fopen_tmpfile(0 STREAMS_REL_CC)

#define crx_stream_fopen_temporary_file_rel(dir, pfx, opened_path)	_crx_stream_fopen_temporary_file((dir), (pfx), (opened_path) STREAMS_REL_CC)

#define crx_stream_open_wrapper_rel(path, mode, options, opened) _crx_stream_open_wrapper_ex((path), (mode), (options), (opened), NULL STREAMS_REL_CC)
#define crx_stream_open_wrapper_ex_rel(path, mode, options, opened, context) _crx_stream_open_wrapper_ex((path), (mode), (options), (opened), (context) STREAMS_REL_CC)

#define crx_stream_make_seekable_rel(origstream, newstream, flags) _crx_stream_make_seekable((origstream), (newstream), (flags) STREAMS_REL_CC)

/* }}} */

/* The contents of the crx_stream_ops and crx_stream should only be accessed
 * using the functions/macros in this header.
 * If you need to get at something that doesn't have an API,
 * drop me a line <wez@thebrainroom.com> and we can sort out a way to do
 * it properly.
 *
 * The only exceptions to this rule are that stream implementations can use
 * the crx_stream->abstract pointer to hold their context, and streams
 * opened via stream_open_wrappers can use the zval ptr in
 * crx_stream->wrapperdata to hold metadata for crx scripts to
 * retrieve using file_get_wrapper_data(). */

typedef struct _crx_stream crx_stream;
typedef struct _crx_stream_wrapper crx_stream_wrapper;
typedef struct _crx_stream_context crx_stream_context;
typedef struct _crx_stream_filter crx_stream_filter;

#include "streams/crx_stream_context.h"
#include "streams/crx_stream_filter_api.h"

typedef struct _crx_stream_statbuf {
	crex_stat_t sb; /* regular info */
	/* extended info to go here some day: content-type etc. etc. */
} crx_stream_statbuf;

typedef struct _crx_stream_dirent {
#ifdef NAME_MAX
	char d_name[NAME_MAX + 1];
#else
	char d_name[MAXPATHLEN];
#endif
	unsigned char d_type;
} crx_stream_dirent;

/* operations on streams that are file-handles */
typedef struct _crx_stream_ops  {
	/* stdio like functions - these are mandatory! */
	ssize_t (*write)(crx_stream *stream, const char *buf, size_t count);
	ssize_t (*read)(crx_stream *stream, char *buf, size_t count);
	int    (*close)(crx_stream *stream, int close_handle);
	int    (*flush)(crx_stream *stream);

	const char *label; /* label for this ops structure */

	/* these are optional */
	int (*seek)(crx_stream *stream, crex_off_t offset, int whence, crex_off_t *newoffset);
	int (*cast)(crx_stream *stream, int castas, void **ret);
	int (*stat)(crx_stream *stream, crx_stream_statbuf *ssb);
	int (*set_option)(crx_stream *stream, int option, int value, void *ptrparam);
} crx_stream_ops;

typedef struct _crx_stream_wrapper_ops {
	/* open/create a wrapped stream */
	crx_stream *(*stream_opener)(crx_stream_wrapper *wrapper, const char *filename, const char *mode,
			int options, crex_string **opened_path, crx_stream_context *context STREAMS_DC);
	/* close/destroy a wrapped stream */
	int (*stream_closer)(crx_stream_wrapper *wrapper, crx_stream *stream);
	/* stat a wrapped stream */
	int (*stream_stat)(crx_stream_wrapper *wrapper, crx_stream *stream, crx_stream_statbuf *ssb);
	/* stat a URL */
	int (*url_stat)(crx_stream_wrapper *wrapper, const char *url, int flags, crx_stream_statbuf *ssb, crx_stream_context *context);
	/* open a "directory" stream */
	crx_stream *(*dir_opener)(crx_stream_wrapper *wrapper, const char *filename, const char *mode,
			int options, crex_string **opened_path, crx_stream_context *context STREAMS_DC);

	const char *label;

	/* delete a file */
	int (*unlink)(crx_stream_wrapper *wrapper, const char *url, int options, crx_stream_context *context);

	/* rename a file */
	int (*rename)(crx_stream_wrapper *wrapper, const char *url_from, const char *url_to, int options, crx_stream_context *context);

	/* Create/Remove directory */
	int (*stream_mkdir)(crx_stream_wrapper *wrapper, const char *url, int mode, int options, crx_stream_context *context);
	int (*stream_rmdir)(crx_stream_wrapper *wrapper, const char *url, int options, crx_stream_context *context);
	/* Metadata handling */
	int (*stream_metadata)(crx_stream_wrapper *wrapper, const char *url, int options, void *value, crx_stream_context *context);
} crx_stream_wrapper_ops;

struct _crx_stream_wrapper	{
	const crx_stream_wrapper_ops *wops;	/* operations the wrapper can perform */
	void *abstract;					/* context for the wrapper */
	int is_url;						/* so that PG(allow_url_fopen) can be respected */
};

#define CRX_STREAM_FLAG_NO_SEEK						0x1
#define CRX_STREAM_FLAG_NO_BUFFER					0x2

#define CRX_STREAM_FLAG_EOL_UNIX					0x0 /* also includes DOS */
#define CRX_STREAM_FLAG_DETECT_EOL					0x4
#define CRX_STREAM_FLAG_EOL_MAC						0x8

/* set this when the stream might represent "interactive" data.
 * When set, the read buffer will avoid certain operations that
 * might otherwise cause the read to block for much longer than
 * is strictly required. */
#define CRX_STREAM_FLAG_AVOID_BLOCKING					0x10

#define CRX_STREAM_FLAG_NO_CLOSE					0x20

#define CRX_STREAM_FLAG_IS_DIR						0x40

#define CRX_STREAM_FLAG_NO_FCLOSE					0x80

/* Suppress generation of CRX warnings on stream read/write errors.
 * Currently, for internal use only. */
#define CRX_STREAM_FLAG_SUPPRESS_ERRORS				0x100

/* Do not close handle except it is explicitly closed by user (e.g. fclose) */
#define CRX_STREAM_FLAG_NO_RSCR_DTOR_CLOSE			0x200

#define CRX_STREAM_FLAG_NO_IO						0x400

#define CRX_STREAM_FLAG_WAS_WRITTEN					0x80000000

struct _crx_stream  {
	const crx_stream_ops *ops;
	void *abstract;			/* convenience pointer for abstraction */

	crx_stream_filter_chain readfilters, writefilters;

	crx_stream_wrapper *wrapper; /* which wrapper was used to open the stream */
	void *wrapperthis;		/* convenience pointer for an instance of a wrapper */
	zval wrapperdata;		/* fgetwrapperdata retrieves this */

	uint16_t is_persistent:1;
	uint16_t in_free:2;			/* to prevent recursion during free */
	uint16_t eof:1;
	uint16_t __exposed:1;	/* non-zero if exposed as a zval somewhere */

	/* so we know how to clean it up correctly.  This should be set to
	 * CRX_STREAM_FCLOSE_XXX as appropriate */
	uint16_t fclose_stdiocast:2;


	/* flag to mark whether the stream has buffered data */
	uint16_t has_buffered_data:1;

	/* whether stdio cast flushing is in progress */
	uint16_t fclose_stdiocast_flush_in_progress:1;

	char mode[16];			/* "rwb" etc. ala stdio */

	uint32_t flags;	/* CRX_STREAM_FLAG_XXX */

	crex_resource *res;		/* used for auto-cleanup */
	FILE *stdiocast;    /* cache this, otherwise we might leak! */
	char *orig_path;

	crex_resource *ctx;

	/* buffer */
	crex_off_t position; /* of underlying stream */
	unsigned char *readbuf;
	size_t readbuflen;
	crex_off_t readpos;
	crex_off_t writepos;

	/* how much data to read when filling buffer */
	size_t chunk_size;

#if CREX_DEBUG
	const char *open_filename;
	uint32_t open_lineno;
#endif

	struct _crx_stream *enclosing_stream; /* this is a private stream owned by enclosing_stream */
}; /* crx_stream */

#define CRX_STREAM_CONTEXT(stream) \
	((crx_stream_context*) ((stream)->ctx ? ((stream)->ctx->ptr) : NULL))

/* state definitions when closing down; these are private to streams.c */
#define CRX_STREAM_FCLOSE_NONE 0
#define CRX_STREAM_FCLOSE_FDOPEN	1
#define CRX_STREAM_FCLOSE_FOPENCOOKIE 2

/* allocate a new stream for a particular ops */
BEGIN_EXTERN_C()
CRXAPI crx_stream *_crx_stream_alloc(const crx_stream_ops *ops, void *abstract,
		const char *persistent_id, const char *mode STREAMS_DC);
END_EXTERN_C()
#define crx_stream_alloc(ops, thisptr, persistent_id, mode)	_crx_stream_alloc((ops), (thisptr), (persistent_id), (mode) STREAMS_CC)

#define crx_stream_get_resource_id(stream)		((crx_stream *)(stream))->res->handle
/* use this to tell the stream that it is OK if we don't explicitly close it */
#define crx_stream_auto_cleanup(stream)	{ (stream)->__exposed = 1; }
/* use this to assign the stream to a zval and tell the stream that is
 * has been exported to the engine; it will expect to be closed automatically
 * when the resources are auto-destructed */
#define crx_stream_to_zval(stream, zval)	{ ZVAL_RES(zval, (stream)->res); (stream)->__exposed = 1; }

#define crx_stream_from_zval(xstr, pzval)	do { \
	if (((xstr) = (crx_stream*)crex_fetch_resource2_ex((pzval), \
				"stream", crx_file_le_stream(), crx_file_le_pstream())) == NULL) { \
		return; \
	} \
} while (0)
#define crx_stream_from_res(xstr, res)	do { \
	if (((xstr) = (crx_stream*)crex_fetch_resource2((res), \
			   	"stream", crx_file_le_stream(), crx_file_le_pstream())) == NULL) { \
		return; \
	} \
} while (0)
#define crx_stream_from_res_no_verify(xstr, pzval)	(xstr) = (crx_stream*)crex_fetch_resource2((res), "stream", crx_file_le_stream(), crx_file_le_pstream())
#define crx_stream_from_zval_no_verify(xstr, pzval)	(xstr) = (crx_stream*)crex_fetch_resource2_ex((pzval), "stream", crx_file_le_stream(), crx_file_le_pstream())

BEGIN_EXTERN_C()
CRXAPI crx_stream *crx_stream_encloses(crx_stream *enclosing, crx_stream *enclosed);
#define crx_stream_free_enclosed(stream_enclosed, close_options) _crx_stream_free_enclosed((stream_enclosed), (close_options))
CRXAPI int _crx_stream_free_enclosed(crx_stream *stream_enclosed, int close_options);

CRXAPI int crx_stream_from_persistent_id(const char *persistent_id, crx_stream **stream);
#define CRX_STREAM_PERSISTENT_SUCCESS	0 /* id exists */
#define CRX_STREAM_PERSISTENT_FAILURE	1 /* id exists but is not a stream! */
#define CRX_STREAM_PERSISTENT_NOT_EXIST	2 /* id does not exist */

#define CRX_STREAM_FREE_CALL_DTOR			1 /* call ops->close */
#define CRX_STREAM_FREE_RELEASE_STREAM		2 /* pefree(stream) */
#define CRX_STREAM_FREE_PRESERVE_HANDLE		4 /* tell ops->close to not close its underlying handle */
#define CRX_STREAM_FREE_RSRC_DTOR			8 /* called from the resource list dtor */
#define CRX_STREAM_FREE_PERSISTENT			16 /* manually freeing a persistent connection */
#define CRX_STREAM_FREE_IGNORE_ENCLOSING	32 /* don't close the enclosing stream instead */
#define CRX_STREAM_FREE_KEEP_RSRC			64 /* keep associated crex_resource */
#define CRX_STREAM_FREE_CLOSE				(CRX_STREAM_FREE_CALL_DTOR | CRX_STREAM_FREE_RELEASE_STREAM)
#define CRX_STREAM_FREE_CLOSE_CASTED		(CRX_STREAM_FREE_CLOSE | CRX_STREAM_FREE_PRESERVE_HANDLE)
#define CRX_STREAM_FREE_CLOSE_PERSISTENT	(CRX_STREAM_FREE_CLOSE | CRX_STREAM_FREE_PERSISTENT)

CRXAPI int _crx_stream_free(crx_stream *stream, int close_options);
#define crx_stream_free(stream, close_options)	_crx_stream_free((stream), (close_options))
#define crx_stream_close(stream)	_crx_stream_free((stream), CRX_STREAM_FREE_CLOSE)
#define crx_stream_pclose(stream)	_crx_stream_free((stream), CRX_STREAM_FREE_CLOSE_PERSISTENT)

CRXAPI int _crx_stream_seek(crx_stream *stream, crex_off_t offset, int whence);
#define crx_stream_rewind(stream)	_crx_stream_seek((stream), 0L, SEEK_SET)
#define crx_stream_seek(stream, offset, whence)	_crx_stream_seek((stream), (offset), (whence))

CRXAPI crex_off_t _crx_stream_tell(crx_stream *stream);
#define crx_stream_tell(stream)	_crx_stream_tell((stream))

CRXAPI ssize_t _crx_stream_read(crx_stream *stream, char *buf, size_t count);
#define crx_stream_read(stream, buf, count)		_crx_stream_read((stream), (buf), (count))

CRXAPI crex_string *crx_stream_read_to_str(crx_stream *stream, size_t len);

CRXAPI ssize_t _crx_stream_write(crx_stream *stream, const char *buf, size_t count);
#define crx_stream_write_string(stream, str)	_crx_stream_write(stream, str, strlen(str))
#define crx_stream_write(stream, buf, count)	_crx_stream_write(stream, (buf), (count))

CRXAPI crex_result _crx_stream_fill_read_buffer(crx_stream *stream, size_t size);
#define crx_stream_fill_read_buffer(stream, size)	_crx_stream_fill_read_buffer((stream), (size))

CRXAPI ssize_t _crx_stream_printf(crx_stream *stream, const char *fmt, ...) CRX_ATTRIBUTE_FORMAT(printf, 2, 3);

/* crx_stream_printf macro & function require */
#define crx_stream_printf _crx_stream_printf

CRXAPI bool _crx_stream_eof(crx_stream *stream);
#define crx_stream_eof(stream)	_crx_stream_eof((stream))

CRXAPI int _crx_stream_getc(crx_stream *stream);
#define crx_stream_getc(stream)	_crx_stream_getc((stream))

CRXAPI int _crx_stream_putc(crx_stream *stream, int c);
#define crx_stream_putc(stream, c)	_crx_stream_putc((stream), (c))

CRXAPI int _crx_stream_flush(crx_stream *stream, int closing);
#define crx_stream_flush(stream)	_crx_stream_flush((stream), 0)

CRXAPI int _crx_stream_sync(crx_stream *stream, bool data_only);
#define crx_stream_sync(stream, d)	    _crx_stream_sync((stream), (d))

CRXAPI char *_crx_stream_get_line(crx_stream *stream, char *buf, size_t maxlen, size_t *returned_len);
#define crx_stream_gets(stream, buf, maxlen)	_crx_stream_get_line((stream), (buf), (maxlen), NULL)

#define crx_stream_get_line(stream, buf, maxlen, retlen) _crx_stream_get_line((stream), (buf), (maxlen), (retlen))
CRXAPI crex_string *crx_stream_get_record(crx_stream *stream, size_t maxlen, const char *delim, size_t delim_len);

/* Returns true if buffer has been appended, false on error */
CRXAPI bool _crx_stream_puts(crx_stream *stream, const char *buf);
#define crx_stream_puts(stream, buf)	_crx_stream_puts((stream), (buf))

CRXAPI int _crx_stream_stat(crx_stream *stream, crx_stream_statbuf *ssb);
#define crx_stream_stat(stream, ssb)	_crx_stream_stat((stream), (ssb))

CRXAPI int _crx_stream_stat_path(const char *path, int flags, crx_stream_statbuf *ssb, crx_stream_context *context);
#define crx_stream_stat_path(path, ssb)	_crx_stream_stat_path((path), 0, (ssb), NULL)
#define crx_stream_stat_path_ex(path, flags, ssb, context)	_crx_stream_stat_path((path), (flags), (ssb), (context))

CRXAPI int _crx_stream_mkdir(const char *path, int mode, int options, crx_stream_context *context);
#define crx_stream_mkdir(path, mode, options, context)	_crx_stream_mkdir(path, mode, options, context)

CRXAPI int _crx_stream_rmdir(const char *path, int options, crx_stream_context *context);
#define crx_stream_rmdir(path, options, context)	_crx_stream_rmdir(path, options, context)

CRXAPI crx_stream *_crx_stream_opendir(const char *path, int options, crx_stream_context *context STREAMS_DC);
#define crx_stream_opendir(path, options, context)	_crx_stream_opendir((path), (options), (context) STREAMS_CC)
CRXAPI crx_stream_dirent *_crx_stream_readdir(crx_stream *dirstream, crx_stream_dirent *ent);
#define crx_stream_readdir(dirstream, dirent)	_crx_stream_readdir((dirstream), (dirent))
#define crx_stream_closedir(dirstream)	crx_stream_close((dirstream))
#define crx_stream_rewinddir(dirstream)	crx_stream_rewind((dirstream))

CRXAPI int crx_stream_dirent_alphasort(const crex_string **a, const crex_string **b);
CRXAPI int crx_stream_dirent_alphasortr(const crex_string **a, const crex_string **b);

CRXAPI int _crx_stream_scandir(const char *dirname, crex_string **namelist[], int flags, crx_stream_context *context,
			int (*compare) (const crex_string **a, const crex_string **b));
#define crx_stream_scandir(dirname, namelist, context, compare) _crx_stream_scandir((dirname), (namelist), 0, (context), (compare))

CRXAPI int _crx_stream_set_option(crx_stream *stream, int option, int value, void *ptrparam);
#define crx_stream_set_option(stream, option, value, ptrvalue)	_crx_stream_set_option((stream), (option), (value), (ptrvalue))

#define crx_stream_set_chunk_size(stream, size) _crx_stream_set_option((stream), CRX_STREAM_OPTION_SET_CHUNK_SIZE, (size), NULL)

END_EXTERN_C()


/* Flags for mkdir method in wrapper ops */
#define CRX_STREAM_MKDIR_RECURSIVE	1
/* define REPORT ERRORS 8 (below) */

/* Flags for rmdir method in wrapper ops */
/* define REPORT_ERRORS 8 (below) */

/* Flags for url_stat method in wrapper ops */
#define CRX_STREAM_URL_STAT_LINK	1
#define CRX_STREAM_URL_STAT_QUIET	2
#define CRX_STREAM_URL_STAT_IGNORE_OPEN_BASEDIR	4

/* change the blocking mode of stream: value == 1 => blocking, value == 0 => non-blocking. */
#define CRX_STREAM_OPTION_BLOCKING	1

/* change the buffering mode of stream. value is a CRX_STREAM_BUFFER_XXXX value, ptrparam is a ptr to a size_t holding
 * the required buffer size */
#define CRX_STREAM_OPTION_READ_BUFFER	2
#define CRX_STREAM_OPTION_WRITE_BUFFER	3

#define CRX_STREAM_BUFFER_NONE	0	/* unbuffered */
#define CRX_STREAM_BUFFER_LINE	1	/* line buffered */
#define CRX_STREAM_BUFFER_FULL	2	/* fully buffered */

/* set the timeout duration for reads on the stream. ptrparam is a pointer to a struct timeval * */
#define CRX_STREAM_OPTION_READ_TIMEOUT	4
#define CRX_STREAM_OPTION_SET_CHUNK_SIZE	5

/* set or release lock on a stream */
#define CRX_STREAM_OPTION_LOCKING		6

/* whether or not locking is supported */
#define CRX_STREAM_LOCK_SUPPORTED		1

#define crx_stream_supports_lock(stream)	(_crx_stream_set_option((stream), CRX_STREAM_OPTION_LOCKING, 0, (void *) CRX_STREAM_LOCK_SUPPORTED) == 0 ? 1 : 0)
#define crx_stream_lock(stream, mode)		_crx_stream_set_option((stream), CRX_STREAM_OPTION_LOCKING, (mode), (void *) NULL)

/* option code used by the crx_stream_xport_XXX api */
#define CRX_STREAM_OPTION_XPORT_API			7 /* see crx_stream_transport.h */
#define CRX_STREAM_OPTION_CRYPTO_API		8 /* see crx_stream_transport.h */
#define CRX_STREAM_OPTION_MMAP_API			9 /* see crx_stream_mmap.h */
#define CRX_STREAM_OPTION_TRUNCATE_API		10

#define CRX_STREAM_TRUNCATE_SUPPORTED	0
#define CRX_STREAM_TRUNCATE_SET_SIZE	1	/* ptrparam is a pointer to a size_t */

#define crx_stream_truncate_supported(stream)	(_crx_stream_set_option((stream), CRX_STREAM_OPTION_TRUNCATE_API, CRX_STREAM_TRUNCATE_SUPPORTED, NULL) == CRX_STREAM_OPTION_RETURN_OK ? 1 : 0)

BEGIN_EXTERN_C()
CRXAPI int _crx_stream_truncate_set_size(crx_stream *stream, size_t newsize);
#define crx_stream_truncate_set_size(stream, size)	_crx_stream_truncate_set_size((stream), (size))
END_EXTERN_C()

#define CRX_STREAM_OPTION_META_DATA_API		11 /* ptrparam is a zval* to which to add metadata information */
#define crx_stream_populate_meta_data(stream, zv)	(_crx_stream_set_option((stream), CRX_STREAM_OPTION_META_DATA_API, 0, zv) == CRX_STREAM_OPTION_RETURN_OK ? 1 : 0)

/* Check if the stream is still "live"; for sockets/pipes this means the socket
 * is still connected; for files, this does not really have meaning */
#define CRX_STREAM_OPTION_CHECK_LIVENESS	12 /* no parameters */

/* Enable/disable blocking reads on anonymous pipes on Windows. */
#define CRX_STREAM_OPTION_PIPE_BLOCKING 13

/* Stream can support fsync operation */
#define CRX_STREAM_OPTION_SYNC_API		14
#define CRX_STREAM_SYNC_SUPPORTED	0
#define CRX_STREAM_SYNC_FSYNC 1
#define CRX_STREAM_SYNC_FDSYNC 2

#define crx_stream_sync_supported(stream)	(_crx_stream_set_option((stream), CRX_STREAM_OPTION_SYNC_API, CRX_STREAM_SYNC_SUPPORTED, NULL) == CRX_STREAM_OPTION_RETURN_OK ? 1 : 0)


#define CRX_STREAM_OPTION_RETURN_OK			 0 /* option set OK */
#define CRX_STREAM_OPTION_RETURN_ERR		-1 /* problem setting option */
#define CRX_STREAM_OPTION_RETURN_NOTIMPL	-2 /* underlying stream does not implement; streams can handle it instead */

/* copy up to maxlen bytes from src to dest.  If maxlen is CRX_STREAM_COPY_ALL,
 * copy until eof(src). */
#define CRX_STREAM_COPY_ALL		((size_t)-1)

BEGIN_EXTERN_C()
CREX_ATTRIBUTE_DEPRECATED
CRXAPI size_t _crx_stream_copy_to_stream(crx_stream *src, crx_stream *dest, size_t maxlen STREAMS_DC);
#define crx_stream_copy_to_stream(src, dest, maxlen)	_crx_stream_copy_to_stream((src), (dest), (maxlen) STREAMS_CC)
CRXAPI crex_result _crx_stream_copy_to_stream_ex(crx_stream *src, crx_stream *dest, size_t maxlen, size_t *len STREAMS_DC);
#define crx_stream_copy_to_stream_ex(src, dest, maxlen, len)	_crx_stream_copy_to_stream_ex((src), (dest), (maxlen), (len) STREAMS_CC)


/* read all data from stream and put into a buffer. Caller must free buffer
 * when done. */
CRXAPI crex_string *_crx_stream_copy_to_mem(crx_stream *src, size_t maxlen, int persistent STREAMS_DC);
#define crx_stream_copy_to_mem(src, maxlen, persistent) _crx_stream_copy_to_mem((src), (maxlen), (persistent) STREAMS_CC)

/* output all data from a stream */
CRXAPI ssize_t _crx_stream_passthru(crx_stream * src STREAMS_DC);
#define crx_stream_passthru(stream)	_crx_stream_passthru((stream) STREAMS_CC)
END_EXTERN_C()

#include "streams/crx_stream_transport.h"
#include "streams/crx_stream_plain_wrapper.h"
#include "streams/crx_stream_glob_wrapper.h"
#include "streams/crx_stream_userspace.h"
#include "streams/crx_stream_mmap.h"

/* coerce the stream into some other form */
/* cast as a stdio FILE * */
#define CRX_STREAM_AS_STDIO     0
/* cast as a POSIX fd or socketd */
#define CRX_STREAM_AS_FD		1
/* cast as a socketd */
#define CRX_STREAM_AS_SOCKETD	2
/* cast as fd/socket for select purposes */
#define CRX_STREAM_AS_FD_FOR_SELECT 3

/* try really, really hard to make sure the cast happens (avoid using this flag if possible) */
#define CRX_STREAM_CAST_TRY_HARD	0x80000000
#define CRX_STREAM_CAST_RELEASE		0x40000000	/* stream becomes invalid on success */
#define CRX_STREAM_CAST_INTERNAL	0x20000000	/* stream cast for internal use */
#define CRX_STREAM_CAST_MASK		(CRX_STREAM_CAST_TRY_HARD | CRX_STREAM_CAST_RELEASE | CRX_STREAM_CAST_INTERNAL)
BEGIN_EXTERN_C()
CRXAPI int _crx_stream_cast(crx_stream *stream, int castas, void **ret, int show_err);
END_EXTERN_C()
/* use this to check if a stream can be cast into another form */
#define crx_stream_can_cast(stream, as)	_crx_stream_cast((stream), (as), NULL, 0)
#define crx_stream_cast(stream, as, ret, show_err)	_crx_stream_cast((stream), (as), (ret), (show_err))

/* use this to check if a stream is of a particular type:
 * CRXAPI int crx_stream_is(crx_stream *stream, crx_stream_ops *ops); */
#define crx_stream_is(stream, anops)		((stream)->ops == anops)
#define CRX_STREAM_IS_STDIO &crx_stream_stdio_ops

#define crx_stream_is_persistent(stream)	(stream)->is_persistent

/* Wrappers support */

#define IGNORE_PATH                     0x00000000
#define USE_PATH                        0x00000001
#define IGNORE_URL                      0x00000002
#define REPORT_ERRORS                   0x00000008

/* If you don't need to write to the stream, but really need to
 * be able to seek, use this flag in your options. */
#define STREAM_MUST_SEEK                0x00000010
/* If you are going to end up casting the stream into a FILE* or
 * a socket, pass this flag and the streams/wrappers will not use
 * buffering mechanisms while reading the headers, so that HTTP
 * wrapped streams will work consistently.
 * If you omit this flag, streams will use buffering and should end
 * up working more optimally.
 * */
#define STREAM_WILL_CAST                0x00000020

/* this flag applies to crx_stream_locate_url_wrapper */
#define STREAM_LOCATE_WRAPPERS_ONLY     0x00000040

/* this flag is only used by include/require functions */
#define STREAM_OPEN_FOR_INCLUDE         0x00000080

/* this flag tells streams to ONLY open urls */
#define STREAM_USE_URL                  0x00000100

/* this flag is used when only the headers from HTTP request are to be fetched */
#define STREAM_ONLY_GET_HEADERS         0x00000200

/* don't apply open_basedir checks */
#define STREAM_DISABLE_OPEN_BASEDIR     0x00000400

/* get (or create) a persistent version of the stream */
#define STREAM_OPEN_PERSISTENT          0x00000800

/* use glob stream for directory open in plain files stream */
#define STREAM_USE_GLOB_DIR_OPEN        0x00001000

/* don't check allow_url_fopen and allow_url_include */
#define STREAM_DISABLE_URL_PROTECTION   0x00002000

/* assume the path passed in exists and is fully expanded, avoiding syscalls */
#define STREAM_ASSUME_REALPATH          0x00004000

/* Allow blocking reads on anonymous pipes on Windows. */
#define STREAM_USE_BLOCKING_PIPE        0x00008000

/* this flag is only used by include/require functions */
#define STREAM_OPEN_FOR_CREX_STREAM     0x00010000

int crx_init_stream_wrappers(int module_number);
void crx_shutdown_stream_wrappers(int module_number);
void crx_shutdown_stream_hashes(void);
CRX_RSHUTDOWN_FUNCTION(streams);

BEGIN_EXTERN_C()
CRXAPI crex_result crx_register_url_stream_wrapper(const char *protocol, const crx_stream_wrapper *wrapper);
CRXAPI crex_result crx_unregister_url_stream_wrapper(const char *protocol);
CRXAPI crex_result crx_register_url_stream_wrapper_volatile(crex_string *protocol, crx_stream_wrapper *wrapper);
CRXAPI crex_result crx_unregister_url_stream_wrapper_volatile(crex_string *protocol);
CRXAPI crx_stream *_crx_stream_open_wrapper_ex(const char *path, const char *mode, int options, crex_string **opened_path, crx_stream_context *context STREAMS_DC);
CRXAPI crx_stream_wrapper *crx_stream_locate_url_wrapper(const char *path, const char **path_for_open, int options);
CRXAPI const char *crx_stream_locate_eol(crx_stream *stream, crex_string *buf);

#define crx_stream_open_wrapper(path, mode, options, opened)	_crx_stream_open_wrapper_ex((path), (mode), (options), (opened), NULL STREAMS_CC)
#define crx_stream_open_wrapper_ex(path, mode, options, opened, context)	_crx_stream_open_wrapper_ex((path), (mode), (options), (opened), (context) STREAMS_CC)

/* pushes an error message onto the stack for a wrapper instance */
CRXAPI void crx_stream_wrapper_log_error(const crx_stream_wrapper *wrapper, int options, const char *fmt, ...) CRX_ATTRIBUTE_FORMAT(printf, 3, 4);

#define CRX_STREAM_UNCHANGED	0 /* orig stream was seekable anyway */
#define CRX_STREAM_RELEASED		1 /* newstream should be used; origstream is no longer valid */
#define CRX_STREAM_FAILED		2 /* an error occurred while attempting conversion */
#define CRX_STREAM_CRITICAL		3 /* an error occurred; origstream is in an unknown state; you should close origstream */
#define CRX_STREAM_NO_PREFERENCE	0
#define CRX_STREAM_PREFER_STDIO		1
#define CRX_STREAM_FORCE_CONVERSION	2
/* DO NOT call this on streams that are referenced by resources! */
CRXAPI int _crx_stream_make_seekable(crx_stream *origstream, crx_stream **newstream, int flags STREAMS_DC);
#define crx_stream_make_seekable(origstream, newstream, flags)	_crx_stream_make_seekable((origstream), (newstream), (flags) STREAMS_CC)

/* Give other modules access to the url_stream_wrappers_hash and stream_filters_hash */
CRXAPI HashTable *_crx_stream_get_url_stream_wrappers_hash(void);
#define crx_stream_get_url_stream_wrappers_hash()	_crx_stream_get_url_stream_wrappers_hash()
CRXAPI HashTable *crx_stream_get_url_stream_wrappers_hash_global(void);
CRXAPI HashTable *_crx_get_stream_filters_hash(void);
#define crx_get_stream_filters_hash()	_crx_get_stream_filters_hash()
CRXAPI HashTable *crx_get_stream_filters_hash_global(void);
extern const crx_stream_wrapper_ops *crx_stream_user_wrapper_ops;

static inline bool crx_is_stream_path(const char *filename)
{
	const char *p;

	for (p = filename;
	     (*p >= 'a' && *p <= 'z') ||
	     (*p >= 'A' && *p <= 'Z') ||
	     (*p >= '0' && *p <= '9') ||
	     *p == '+' || *p == '-' || *p == '.';
	     p++);
	return ((p != filename) && (p[0] == ':') && (p[1] == '/') && (p[2] == '/'));
}

END_EXTERN_C()
#endif

/* Definitions for user streams */
#define CRX_STREAM_IS_URL		1

/* Stream metadata definitions */
/* Create if referred resource does not exist */
#define CRX_STREAM_META_TOUCH		1
#define CRX_STREAM_META_OWNER_NAME	2
#define CRX_STREAM_META_OWNER		3
#define CRX_STREAM_META_GROUP_NAME	4
#define CRX_STREAM_META_GROUP		5
#define CRX_STREAM_META_ACCESS		6
