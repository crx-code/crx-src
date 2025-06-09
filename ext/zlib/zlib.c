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
   | Authors: Rasmus Lerdorf <rasmus@lerdorf.on.ca>                       |
   |          Stefan Röhrich <sr@linux.de>                                |
   |          Zeev Suraski <zeev@crx.net>                                 |
   |          Jade Nicoletti <nicoletti@nns.ch>                           |
   |          Michael Wallner <mike@crx.net>                              |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "SAPI.h"
#include "crx_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/crx_string.h"
#include "crx_zlib.h"
#include "zlib_arginfo.h"

/*
 * zlib include files can define the following preprocessor defines which rename
 * the corresponding CRX functions to gzopen64, gzseek64 and gztell64 and thereby
 * breaking some software, most notably PEAR's Archive_Tar, which halts execution
 * without error message on gzip compressed archives.
 *
 * This only seems to happen on 32bit systems with large file support.
 */
#undef gzopen
#undef gzseek
#undef gztell

CREX_DECLARE_MODULE_GLOBALS(zlib)

/* InflateContext class */

crex_class_entry *inflate_context_ce;
static crex_object_handlers inflate_context_object_handlers;

static inline crx_zlib_context *inflate_context_from_obj(crex_object *obj) {
	return (crx_zlib_context *)((char *)(obj) - XtOffsetOf(crx_zlib_context, std));
}

#define C_INFLATE_CONTEXT_P(zv) inflate_context_from_obj(C_OBJ_P(zv))

static crex_object *inflate_context_create_object(crex_class_entry *class_type) {
	crx_zlib_context *intern = crex_object_alloc(sizeof(crx_zlib_context), class_type);

	crex_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	return &intern->std;
}

static crex_function *inflate_context_get_constructor(crex_object *object) {
	crex_throw_error(NULL, "Cannot directly construct InflateContext, use inflate_init() instead");
	return NULL;
}

static void inflate_context_free_obj(crex_object *object)
{
	crx_zlib_context *intern = inflate_context_from_obj(object);

	if (intern->inflateDict) {
		efree(intern->inflateDict);
	}
	inflateEnd(&intern->Z);

	crex_object_std_dtor(&intern->std);
}
/* }}} */

/* DeflateContext class */

crex_class_entry *deflate_context_ce;
static crex_object_handlers deflate_context_object_handlers;

static inline crx_zlib_context *deflate_context_from_obj(crex_object *obj) {
	return (crx_zlib_context *)((char *)(obj) - XtOffsetOf(crx_zlib_context, std));
}

#define C_DEFLATE_CONTEXT_P(zv) deflate_context_from_obj(C_OBJ_P(zv))

static crex_object *deflate_context_create_object(crex_class_entry *class_type) {
	crx_zlib_context *intern = crex_object_alloc(sizeof(crx_zlib_context), class_type);

	crex_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	return &intern->std;
}

static crex_function *deflate_context_get_constructor(crex_object *object) {
	crex_throw_error(NULL, "Cannot directly construct DeflateContext, use deflate_init() instead");
	return NULL;
}

static void deflate_context_free_obj(crex_object *object)
{
	crx_zlib_context *intern = deflate_context_from_obj(object);

	deflateEnd(&intern->Z);

	crex_object_std_dtor(&intern->std);
}
/* }}} */

/* {{{ Memory management wrappers */

static voidpf crx_zlib_alloc(voidpf opaque, uInt items, uInt size)
{
	return (voidpf)safe_emalloc(items, size, 0);
}

static void crx_zlib_free(voidpf opaque, voidpf address)
{
	efree((void*)address);
}
/* }}} */

/* {{{ crx_zlib_output_conflict_check() */
static int crx_zlib_output_conflict_check(const char *handler_name, size_t handler_name_len)
{
	if (crx_output_get_level() > 0) {
		if (crx_output_handler_conflict(handler_name, handler_name_len, CREX_STRL(CRX_ZLIB_OUTPUT_HANDLER_NAME))
		||	crx_output_handler_conflict(handler_name, handler_name_len, CREX_STRL("ob_gzhandler"))
		||  crx_output_handler_conflict(handler_name, handler_name_len, CREX_STRL("mb_output_handler"))
		||	crx_output_handler_conflict(handler_name, handler_name_len, CREX_STRL("URL-Rewriter"))) {
			return FAILURE;
		}
	}
	return SUCCESS;
}
/* }}} */

/* {{{ crx_zlib_output_encoding() */
static int crx_zlib_output_encoding(void)
{
	zval *enc;

	if (!ZLIBG(compression_coding)) {
		if ((C_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) == IS_ARRAY || crex_is_auto_global(ZSTR_KNOWN(CREX_STR_AUTOGLOBAL_SERVER))) &&
			(enc = crex_hash_str_find(C_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]), "HTTP_ACCEPT_ENCODING", sizeof("HTTP_ACCEPT_ENCODING") - 1))) {
			convert_to_string(enc);
			if (strstr(C_STRVAL_P(enc), "gzip")) {
				ZLIBG(compression_coding) = CRX_ZLIB_ENCODING_GZIP;
			} else if (strstr(C_STRVAL_P(enc), "deflate")) {
				ZLIBG(compression_coding) = CRX_ZLIB_ENCODING_DEFLATE;
			}
		}
	}
	return ZLIBG(compression_coding);
}
/* }}} */

/* {{{ crx_zlib_output_handler_ex() */
static int crx_zlib_output_handler_ex(crx_zlib_context *ctx, crx_output_context *output_context)
{
	int flags = C_SYNC_FLUSH;

	if (output_context->op & CRX_OUTPUT_HANDLER_START) {
		/* start up */
		if (C_OK != deflateInit2(&ctx->Z, ZLIBG(output_compression_level), C_DEFLATED, ZLIBG(compression_coding), MAX_MEM_LEVEL, C_DEFAULT_STRATEGY)) {
			return FAILURE;
		}
	}

	if (output_context->op & CRX_OUTPUT_HANDLER_CLEAN) {
		/* free buffers */
		deflateEnd(&ctx->Z);

		if (output_context->op & CRX_OUTPUT_HANDLER_FINAL) {
			/* discard */
			return SUCCESS;
		} else {
			/* restart */
			if (C_OK != deflateInit2(&ctx->Z, ZLIBG(output_compression_level), C_DEFLATED, ZLIBG(compression_coding), MAX_MEM_LEVEL, C_DEFAULT_STRATEGY)) {
				return FAILURE;
			}
			ctx->buffer.used = 0;
		}
	} else {
		if (output_context->in.used) {
			/* append input */
			if (ctx->buffer.free < output_context->in.used) {
				if (!(ctx->buffer.aptr = erealloc_recoverable(ctx->buffer.data, ctx->buffer.used + ctx->buffer.free + output_context->in.used))) {
					deflateEnd(&ctx->Z);
					return FAILURE;
				}
				ctx->buffer.data = ctx->buffer.aptr;
				ctx->buffer.free += output_context->in.used;
			}
			memcpy(ctx->buffer.data + ctx->buffer.used, output_context->in.data, output_context->in.used);
			ctx->buffer.free -= output_context->in.used;
			ctx->buffer.used += output_context->in.used;
		}
		output_context->out.size = CRX_ZLIB_BUFFER_SIZE_GUESS(output_context->in.used);
		output_context->out.data = emalloc(output_context->out.size);
		output_context->out.free = 1;
		output_context->out.used = 0;

		ctx->Z.avail_in = ctx->buffer.used;
		ctx->Z.next_in = (Bytef *) ctx->buffer.data;
		ctx->Z.avail_out = output_context->out.size;
		ctx->Z.next_out = (Bytef *) output_context->out.data;

		if (output_context->op & CRX_OUTPUT_HANDLER_FINAL) {
			flags = C_FINISH;
		} else if (output_context->op & CRX_OUTPUT_HANDLER_FLUSH) {
			flags = C_FULL_FLUSH;
		}

		switch (deflate(&ctx->Z, flags)) {
			case C_OK:
				if (flags == C_FINISH) {
					deflateEnd(&ctx->Z);
					return FAILURE;
				}
				CREX_FALLTHROUGH;
			case C_STREAM_END:
				if (ctx->Z.avail_in) {
					memmove(ctx->buffer.data, ctx->buffer.data + ctx->buffer.used - ctx->Z.avail_in, ctx->Z.avail_in);
				}
				ctx->buffer.free += ctx->buffer.used - ctx->Z.avail_in;
				ctx->buffer.used = ctx->Z.avail_in;
				output_context->out.used = output_context->out.size - ctx->Z.avail_out;
				break;
			default:
				deflateEnd(&ctx->Z);
				return FAILURE;
		}

		if (output_context->op & CRX_OUTPUT_HANDLER_FINAL) {
			deflateEnd(&ctx->Z);
		}
	}

	return SUCCESS;
}
/* }}} */

/* {{{ crx_zlib_output_handler() */
static int crx_zlib_output_handler(void **handler_context, crx_output_context *output_context)
{
	crx_zlib_context *ctx = *(crx_zlib_context **) handler_context;

	if (!crx_zlib_output_encoding()) {
		/* "Vary: Accept-Encoding" header sent along uncompressed content breaks caching in MSIE,
			so let's just send it with successfully compressed content or unless the complete
			buffer gets discarded, see http://bugs.crx.net/40325;

			Test as follows:
			+Vary: $ HTTP_ACCEPT_ENCODING=gzip ./sapi/cgi/crx <<<'<?crx ob_start("ob_gzhandler"); echo "foo\n";'
			+Vary: $ HTTP_ACCEPT_ENCODING= ./sapi/cgi/crx <<<'<?crx ob_start("ob_gzhandler"); echo "foo\n";'
			-Vary: $ HTTP_ACCEPT_ENCODING=gzip ./sapi/cgi/crx <<<'<?crx ob_start("ob_gzhandler"); echo "foo\n"; ob_end_clean();'
			-Vary: $ HTTP_ACCEPT_ENCODING= ./sapi/cgi/crx <<<'<?crx ob_start("ob_gzhandler"); echo "foo\n"; ob_end_clean();'
		*/
		if ((output_context->op & CRX_OUTPUT_HANDLER_START)
		&&	(output_context->op != (CRX_OUTPUT_HANDLER_START|CRX_OUTPUT_HANDLER_CLEAN|CRX_OUTPUT_HANDLER_FINAL))
		) {
			sapi_add_header_ex(CREX_STRL("Vary: Accept-Encoding"), 1, 0);
		}
		return FAILURE;
	}

	if (SUCCESS != crx_zlib_output_handler_ex(ctx, output_context)) {
		return FAILURE;
	}

	if (!(output_context->op & CRX_OUTPUT_HANDLER_CLEAN) || ((output_context->op & CRX_OUTPUT_HANDLER_START) && !(output_context->op & CRX_OUTPUT_HANDLER_FINAL))) {
		int flags;

		if (SUCCESS == crx_output_handler_hook(CRX_OUTPUT_HANDLER_HOOK_GET_FLAGS, &flags)) {
			/* only run this once */
			if (!(flags & CRX_OUTPUT_HANDLER_STARTED)) {
				if (SG(headers_sent) || !ZLIBG(output_compression)) {
					deflateEnd(&ctx->Z);
					return FAILURE;
				}
				switch (ZLIBG(compression_coding)) {
					case CRX_ZLIB_ENCODING_GZIP:
						sapi_add_header_ex(CREX_STRL("Content-Encoding: gzip"), 1, 1);
						break;
					case CRX_ZLIB_ENCODING_DEFLATE:
						sapi_add_header_ex(CREX_STRL("Content-Encoding: deflate"), 1, 1);
						break;
					default:
						deflateEnd(&ctx->Z);
						return FAILURE;
				}
				sapi_add_header_ex(CREX_STRL("Vary: Accept-Encoding"), 1, 0);
				crx_output_handler_hook(CRX_OUTPUT_HANDLER_HOOK_IMMUTABLE, NULL);
			}
		}
	}

	return SUCCESS;
}
/* }}} */

/* {{{ crx_zlib_output_handler_context_init() */
static crx_zlib_context *crx_zlib_output_handler_context_init(void)
{
	crx_zlib_context *ctx = (crx_zlib_context *) ecalloc(1, sizeof(crx_zlib_context));
	ctx->Z.zalloc = crx_zlib_alloc;
	ctx->Z.zfree = crx_zlib_free;
	return ctx;
}
/* }}} */

/* {{{ crx_zlib_output_handler_context_dtor() */
static void crx_zlib_output_handler_context_dtor(void *opaq)
{
	crx_zlib_context *ctx = (crx_zlib_context *) opaq;

	if (ctx) {
		if (ctx->buffer.data) {
			efree(ctx->buffer.data);
		}
		efree(ctx);
	}
}
/* }}} */

/* {{{ crx_zlib_output_handler_init() */
static crx_output_handler *crx_zlib_output_handler_init(const char *handler_name, size_t handler_name_len, size_t chunk_size, int flags)
{
	crx_output_handler *h = NULL;

	if (!ZLIBG(output_compression)) {
		ZLIBG(output_compression) = chunk_size ? chunk_size : CRX_OUTPUT_HANDLER_DEFAULT_SIZE;
	}

	ZLIBG(handler_registered) = 1;

	if ((h = crx_output_handler_create_internal(handler_name, handler_name_len, crx_zlib_output_handler, chunk_size, flags))) {
		crx_output_handler_set_context(h, crx_zlib_output_handler_context_init(), crx_zlib_output_handler_context_dtor);
	}

	return h;
}
/* }}} */

/* {{{ crx_zlib_output_compression_start() */
static void crx_zlib_output_compression_start(void)
{
	zval zoh;
	crx_output_handler *h;

	switch (ZLIBG(output_compression)) {
		case 0:
			break;
		case 1:
			ZLIBG(output_compression) = CRX_OUTPUT_HANDLER_DEFAULT_SIZE;
			CREX_FALLTHROUGH;
		default:
			if (	crx_zlib_output_encoding() &&
					(h = crx_zlib_output_handler_init(CREX_STRL(CRX_ZLIB_OUTPUT_HANDLER_NAME), ZLIBG(output_compression), CRX_OUTPUT_HANDLER_STDFLAGS)) &&
					(SUCCESS == crx_output_handler_start(h))) {
				if (ZLIBG(output_handler) && *ZLIBG(output_handler)) {
					ZVAL_STRING(&zoh, ZLIBG(output_handler));
					crx_output_start_user(&zoh, ZLIBG(output_compression), CRX_OUTPUT_HANDLER_STDFLAGS);
					zval_ptr_dtor(&zoh);
				}
			}
			break;
	}
}
/* }}} */

/* {{{ crx_zlib_encode() */
static crex_string *crx_zlib_encode(const char *in_buf, size_t in_len, int encoding, int level)
{
	int status;
	z_stream Z;
	crex_string *out;

	memset(&Z, 0, sizeof(z_stream));
	Z.zalloc = crx_zlib_alloc;
	Z.zfree = crx_zlib_free;

	if (C_OK == (status = deflateInit2(&Z, level, C_DEFLATED, encoding, MAX_MEM_LEVEL, C_DEFAULT_STRATEGY))) {
		out = crex_string_alloc(CRX_ZLIB_BUFFER_SIZE_GUESS(in_len), 0);

		Z.next_in = (Bytef *) in_buf;
		Z.next_out = (Bytef *) ZSTR_VAL(out);
		Z.avail_in = in_len;
		Z.avail_out = ZSTR_LEN(out);

		status = deflate(&Z, C_FINISH);
		deflateEnd(&Z);

		if (C_STREAM_END == status) {
			/* size buffer down to actual length */
			out = crex_string_truncate(out, Z.total_out, 0);
			ZSTR_VAL(out)[ZSTR_LEN(out)] = '\0';
			return out;
		} else {
			crex_string_efree(out);
		}
	}

	crx_error_docref(NULL, E_WARNING, "%s", zError(status));
	return NULL;
}
/* }}} */

/* {{{ crx_zlib_inflate_rounds() */
static inline int crx_zlib_inflate_rounds(z_stream *Z, size_t max, char **buf, size_t *len)
{
	int status, round = 0;
	crx_zlib_buffer buffer = {NULL, NULL, 0, 0, 0};

	*buf = NULL;
	*len = 0;

	buffer.size = (max && (max < Z->avail_in)) ? max : Z->avail_in;

	do {
		if ((max && (max <= buffer.used)) || !(buffer.aptr = erealloc_recoverable(buffer.data, buffer.size))) {
			status = C_MEM_ERROR;
		} else {
			buffer.data = buffer.aptr;
			Z->avail_out = buffer.free = buffer.size - buffer.used;
			Z->next_out = (Bytef *) buffer.data + buffer.used;
#if 0
			fprintf(stderr, "\n%3d: %3d PRIOR: size=%7lu,\tfree=%7lu,\tused=%7lu,\tavail_in=%7lu,\tavail_out=%7lu\n", round, status, buffer.size, buffer.free, buffer.used, Z->avail_in, Z->avail_out);
#endif
			status = inflate(Z, C_NO_FLUSH);

			buffer.used += buffer.free - Z->avail_out;
			buffer.free = Z->avail_out;
#if 0
			fprintf(stderr, "%3d: %3d AFTER: size=%7lu,\tfree=%7lu,\tused=%7lu,\tavail_in=%7lu,\tavail_out=%7lu\n", round, status, buffer.size, buffer.free, buffer.used, Z->avail_in, Z->avail_out);
#endif
			buffer.size += (buffer.size >> 3) + 1;
		}
	} while ((C_BUF_ERROR == status || (C_OK == status && Z->avail_in)) && ++round < 100);

	if (status == C_STREAM_END) {
		buffer.data = erealloc(buffer.data, buffer.used + 1);
		buffer.data[buffer.used] = '\0';
		*buf = buffer.data;
		*len = buffer.used;
	} else {
		if (buffer.data) {
			efree(buffer.data);
		}
		/* HACK: See zlib/examples/zpipe.c inf() function for explanation. */
		/* This works as long as this function is not used for streaming. Required to catch very short invalid data. */
		status = (status == C_OK) ? C_DATA_ERROR : status;
	}
	return status;
}
/* }}} */

/* {{{ crx_zlib_decode() */
static int crx_zlib_decode(const char *in_buf, size_t in_len, char **out_buf, size_t *out_len, int encoding, size_t max_len)
{
	int status = C_DATA_ERROR;
	z_stream Z;

	memset(&Z, 0, sizeof(z_stream));
	Z.zalloc = crx_zlib_alloc;
	Z.zfree = crx_zlib_free;

	if (in_len) {
retry_raw_inflate:
		status = inflateInit2(&Z, encoding);
		if (C_OK == status) {
			Z.next_in = (Bytef *) in_buf;
			Z.avail_in = in_len + 1; /* NOTE: data must be zero terminated */

			switch (status = crx_zlib_inflate_rounds(&Z, max_len, out_buf, out_len)) {
				case C_STREAM_END:
					inflateEnd(&Z);
					return SUCCESS;

				case C_DATA_ERROR:
					/* raw deflated data? */
					if (CRX_ZLIB_ENCODING_ANY == encoding) {
						inflateEnd(&Z);
						encoding = CRX_ZLIB_ENCODING_RAW;
						goto retry_raw_inflate;
					}
			}
			inflateEnd(&Z);
		}
	}

	*out_buf = NULL;
	*out_len = 0;

	crx_error_docref(NULL, E_WARNING, "%s", zError(status));
	return FAILURE;
}
/* }}} */

/* {{{ crx_zlib_cleanup_ob_gzhandler_mess() */
static void crx_zlib_cleanup_ob_gzhandler_mess(void)
{
	if (ZLIBG(ob_gzhandler)) {
		deflateEnd(&(ZLIBG(ob_gzhandler)->Z));
		crx_zlib_output_handler_context_dtor(ZLIBG(ob_gzhandler));
		ZLIBG(ob_gzhandler) = NULL;
	}
}
/* }}} */

/* {{{ Legacy hack */
CRX_FUNCTION(ob_gzhandler)
{
	char *in_str;
	size_t in_len;
	crex_long flags = 0;
	crx_output_context ctx = {0};
	int encoding, rv;

	/*
	 * NOTE that the real ob_gzhandler is an alias to "zlib output compression".
	 * This is a really bad hack, because
	 * - we have to initialize a crx_zlib_context on demand
	 * - we have to clean it up in RSHUTDOWN
	 * - OG(running) is not set or set to any other output handler
	 * - we have to mess around with crx_output_context */

	if (SUCCESS != crex_parse_parameters(CREX_NUM_ARGS(), "sl", &in_str, &in_len, &flags)) {
		RETURN_THROWS();
	}

	if (!(encoding = crx_zlib_output_encoding())) {
		RETURN_FALSE;
	}

	if (flags & CRX_OUTPUT_HANDLER_START) {
		switch (encoding) {
			case CRX_ZLIB_ENCODING_GZIP:
				sapi_add_header_ex(CREX_STRL("Content-Encoding: gzip"), 1, 1);
				break;
			case CRX_ZLIB_ENCODING_DEFLATE:
				sapi_add_header_ex(CREX_STRL("Content-Encoding: deflate"), 1, 1);
				break;
		}
		sapi_add_header_ex(CREX_STRL("Vary: Accept-Encoding"), 1, 0);
	}

	if (!ZLIBG(ob_gzhandler)) {
		ZLIBG(ob_gzhandler) = crx_zlib_output_handler_context_init();
	}

	ctx.op = flags;
	ctx.in.data = in_str;
	ctx.in.used = in_len;

	rv = crx_zlib_output_handler_ex(ZLIBG(ob_gzhandler), &ctx);

	if (SUCCESS != rv) {
		if (ctx.out.data && ctx.out.free) {
			efree(ctx.out.data);
		}
		crx_zlib_cleanup_ob_gzhandler_mess();
		RETURN_FALSE;
	}

	if (ctx.out.data) {
		RETVAL_STRINGL(ctx.out.data, ctx.out.used);
		if (ctx.out.free) {
			efree(ctx.out.data);
		}
	} else {
		RETVAL_EMPTY_STRING();
	}
}
/* }}} */

/* {{{ Returns the coding type used for output compression */
CRX_FUNCTION(zlib_get_coding_type)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	switch (ZLIBG(compression_coding)) {
		case CRX_ZLIB_ENCODING_GZIP:
			RETURN_STRINGL("gzip", sizeof("gzip") - 1);
		case CRX_ZLIB_ENCODING_DEFLATE:
			RETURN_STRINGL("deflate", sizeof("deflate") - 1);
		default:
			RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Read and uncompress entire .gz-file into an array */
CRX_FUNCTION(gzfile)
{
	char *filename;
	size_t filename_len;
	int flags = REPORT_ERRORS;
	char buf[8192] = {0};
	int i = 0;
	crex_long use_include_path = 0;
	crx_stream *stream;

	if (SUCCESS != crex_parse_parameters(CREX_NUM_ARGS(), "p|l", &filename, &filename_len, &use_include_path)) {
		RETURN_THROWS();
	}

	if (use_include_path) {
		flags |= USE_PATH;
	}

	/* using a stream here is a bit more efficient (resource wise) than crx_gzopen_wrapper */
	stream = crx_stream_gzopen(NULL, filename, "rb", flags, NULL, NULL STREAMS_CC);

	if (!stream) {
		/* Error reporting is already done by stream code */
		RETURN_FALSE;
	}

	/* Initialize return array */
	array_init(return_value);

	/* Now loop through the file and do the magic quotes thing if needed */
	memset(buf, 0, sizeof(buf));

	while (crx_stream_gets(stream, buf, sizeof(buf) - 1) != NULL) {
		add_index_string(return_value, i++, buf);
	}
	crx_stream_close(stream);
}
/* }}} */

/* {{{ Open a .gz-file and return a .gz-file pointer */
CRX_FUNCTION(gzopen)
{
	char *filename;
	char *mode;
	size_t filename_len, mode_len;
	int flags = REPORT_ERRORS;
	crx_stream *stream;
	crex_long use_include_path = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ps|l", &filename, &filename_len, &mode, &mode_len, &use_include_path) == FAILURE) {
		RETURN_THROWS();
	}

	if (use_include_path) {
		flags |= USE_PATH;
	}

	stream = crx_stream_gzopen(NULL, filename, mode, flags, NULL, NULL STREAMS_CC);

	if (!stream) {
		RETURN_FALSE;
	}
	crx_stream_to_zval(stream, return_value);
}
/* }}} */

/* {{{ Output a .gz-file */
CRX_FUNCTION(readgzfile)
{
	char *filename;
	size_t filename_len;
	int flags = REPORT_ERRORS;
	crx_stream *stream;
	size_t size;
	crex_long use_include_path = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "p|l", &filename, &filename_len, &use_include_path) == FAILURE) {
		RETURN_THROWS();
	}

	if (use_include_path) {
		flags |= USE_PATH;
	}

	stream = crx_stream_gzopen(NULL, filename, "rb", flags, NULL, NULL STREAMS_CC);

	if (!stream) {
		RETURN_FALSE;
	}
	size = crx_stream_passthru(stream);
	crx_stream_close(stream);
	RETURN_LONG(size);
}
/* }}} */

#define CRX_ZLIB_ENCODE_FUNC(name, default_encoding) \
CRX_FUNCTION(name) \
{ \
	crex_string *in, *out; \
	crex_long level = -1; \
	crex_long encoding = default_encoding; \
	if (default_encoding) { \
		if (SUCCESS != crex_parse_parameters(CREX_NUM_ARGS(), "S|ll", &in, &level, &encoding)) { \
			RETURN_THROWS(); \
		} \
	} else { \
		if (SUCCESS != crex_parse_parameters(CREX_NUM_ARGS(), "Sl|l", &in, &encoding, &level)) { \
			RETURN_THROWS(); \
		} \
	} \
	if (level < -1 || level > 9) { \
		crex_argument_value_error(default_encoding ? 2 : 3, "must be between -1 and 9"); \
		RETURN_THROWS(); \
	} \
	switch (encoding) { \
		case CRX_ZLIB_ENCODING_RAW: \
		case CRX_ZLIB_ENCODING_GZIP: \
		case CRX_ZLIB_ENCODING_DEFLATE: \
			break; \
		default: \
			crex_argument_value_error(default_encoding ? 3 : 2, "must be one of ZLIB_ENCODING_RAW, ZLIB_ENCODING_GZIP, or ZLIB_ENCODING_DEFLATE"); \
			RETURN_THROWS(); \
	} \
	if ((out = crx_zlib_encode(ZSTR_VAL(in), ZSTR_LEN(in), encoding, level)) == NULL) { \
		RETURN_FALSE; \
	} \
	RETURN_STR(out); \
}

#define CRX_ZLIB_DECODE_FUNC(name, encoding) \
CRX_FUNCTION(name) \
{ \
	char *in_buf, *out_buf; \
	size_t in_len; \
	size_t out_len; \
	crex_long max_len = 0; \
	if (SUCCESS != crex_parse_parameters(CREX_NUM_ARGS(), "s|l", &in_buf, &in_len, &max_len)) { \
		RETURN_THROWS(); \
	} \
	if (max_len < 0) { \
		crex_argument_value_error(2, "must be greater than or equal to 0"); \
		RETURN_THROWS(); \
	} \
	if (SUCCESS != crx_zlib_decode(in_buf, in_len, &out_buf, &out_len, encoding, max_len)) { \
		RETURN_FALSE; \
	} \
	RETVAL_STRINGL(out_buf, out_len); \
	efree(out_buf); \
}

/* {{{ Compress data with the specified encoding */
CRX_ZLIB_ENCODE_FUNC(zlib_encode, 0);
/* }}} */

/* {{{ Uncompress any raw/gzip/zlib encoded data */
CRX_ZLIB_DECODE_FUNC(zlib_decode, CRX_ZLIB_ENCODING_ANY);
/* }}} */

/* NOTE: The naming of these userland functions was quite unlucky */
/* {{{ Encode data with the raw deflate encoding */
CRX_ZLIB_ENCODE_FUNC(gzdeflate, CRX_ZLIB_ENCODING_RAW);
/* }}} */

/* {{{ Encode data with the gzip encoding */
CRX_ZLIB_ENCODE_FUNC(gzencode, CRX_ZLIB_ENCODING_GZIP);
/* }}} */

/* {{{ Encode data with the zlib encoding */
CRX_ZLIB_ENCODE_FUNC(gzcompress, CRX_ZLIB_ENCODING_DEFLATE);
/* }}} */

/* {{{ Decode raw deflate encoded data */
CRX_ZLIB_DECODE_FUNC(gzinflate, CRX_ZLIB_ENCODING_RAW);
/* }}} */

/* {{{ Decode gzip encoded data */
CRX_ZLIB_DECODE_FUNC(gzdecode, CRX_ZLIB_ENCODING_GZIP);
/* }}} */

/* {{{ Decode zlib encoded data */
CRX_ZLIB_DECODE_FUNC(gzuncompress, CRX_ZLIB_ENCODING_DEFLATE);
/* }}} */

static bool zlib_create_dictionary_string(HashTable *options, char **dict, size_t *dictlen) {
	zval *option_buffer;

	if (options && (option_buffer = crex_hash_str_find(options, CREX_STRL("dictionary"))) != NULL) {
		ZVAL_DEREF(option_buffer);
		switch (C_TYPE_P(option_buffer)) {
			case IS_STRING: {
				crex_string *str = C_STR_P(option_buffer);
				*dict = emalloc(ZSTR_LEN(str));
				memcpy(*dict, ZSTR_VAL(str), ZSTR_LEN(str));
				*dictlen = ZSTR_LEN(str);
			} break;

			case IS_ARRAY: {
				HashTable *dictionary = C_ARR_P(option_buffer);

				if (crex_hash_num_elements(dictionary) > 0) {
					char *dictptr;
					zval *cur;
					crex_string **strings = emalloc(sizeof(crex_string *) * crex_hash_num_elements(dictionary));
					crex_string **end, **ptr = strings - 1;

					CREX_HASH_FOREACH_VAL(dictionary, cur) {
						size_t i;

						*++ptr = zval_get_string(cur);
						if (!*ptr || ZSTR_LEN(*ptr) == 0 || EG(exception)) {
							if (*ptr) {
								efree(*ptr);
							}
							while (--ptr >= strings) {
								efree(ptr);
							}
							efree(strings);
							if (!EG(exception)) {
								crex_argument_value_error(2, "must not contain empty strings");
							}
							return 0;
						}
						for (i = 0; i < ZSTR_LEN(*ptr); i++) {
							if (ZSTR_VAL(*ptr)[i] == 0) {
								do {
									efree(ptr);
								} while (--ptr >= strings);
								efree(strings);
								crex_argument_value_error(2, "must not contain strings with null bytes");
								return 0;
							}
						}

						*dictlen += ZSTR_LEN(*ptr) + 1;
					} CREX_HASH_FOREACH_END();

					dictptr = *dict = emalloc(*dictlen);
					ptr = strings;
					end = strings + crex_hash_num_elements(dictionary);
					do {
						memcpy(dictptr, ZSTR_VAL(*ptr), ZSTR_LEN(*ptr));
						dictptr += ZSTR_LEN(*ptr);
						*dictptr++ = 0;
						crex_string_release_ex(*ptr, 0);
					} while (++ptr != end);
					efree(strings);
				}
			} break;

			default:
				crex_argument_type_error(2, "must be of type zero-terminated string or array, %s given", crex_zval_value_name(option_buffer));
				return 0;
		}
	}

	return 1;
}

/* {{{ Initialize an incremental inflate context with the specified encoding */
CRX_FUNCTION(inflate_init)
{
	crx_zlib_context *ctx;
	crex_long encoding, window = 15;
	char *dict = NULL;
	size_t dictlen = 0;
	HashTable *options = NULL;
	zval *option_buffer;

	if (SUCCESS != crex_parse_parameters(CREX_NUM_ARGS(), "l|H", &encoding, &options)) {
		RETURN_THROWS();
	}

	if (options && (option_buffer = crex_hash_str_find(options, CREX_STRL("window"))) != NULL) {
		window = zval_get_long(option_buffer);
	}
	if (window < 8 || window > 15) {
		crex_value_error("zlib window size (logarithm) (" CREX_LONG_FMT ") must be within 8..15", window);
		RETURN_THROWS();
	}

	if (!zlib_create_dictionary_string(options, &dict, &dictlen)) {
		RETURN_THROWS();
	}

	switch (encoding) {
		case CRX_ZLIB_ENCODING_RAW:
		case CRX_ZLIB_ENCODING_GZIP:
		case CRX_ZLIB_ENCODING_DEFLATE:
			break;
		default:
			crex_value_error("Encoding mode must be ZLIB_ENCODING_RAW, ZLIB_ENCODING_GZIP or ZLIB_ENCODING_DEFLATE");
			RETURN_THROWS();
	}

	object_init_ex(return_value, inflate_context_ce);
	ctx = C_INFLATE_CONTEXT_P(return_value);

	ctx->Z.zalloc = crx_zlib_alloc;
	ctx->Z.zfree = crx_zlib_free;
	ctx->inflateDict = dict;
	ctx->inflateDictlen = dictlen;
	ctx->status = C_OK;

	if (encoding < 0) {
		encoding += 15 - window;
	} else {
		encoding -= 15 - window;
	}

	if (inflateInit2(&ctx->Z, encoding) != C_OK) {
		zval_ptr_dtor(return_value);
		crx_error_docref(NULL, E_WARNING, "Failed allocating zlib.inflate context");
		RETURN_FALSE;
	}

	if (encoding == CRX_ZLIB_ENCODING_RAW && dictlen > 0) {
		switch (inflateSetDictionary(&ctx->Z, (Bytef *) ctx->inflateDict, ctx->inflateDictlen)) {
			case C_OK:
				efree(ctx->inflateDict);
				ctx->inflateDict = NULL;
				break;
			case C_DATA_ERROR:
				crx_error_docref(NULL, E_WARNING, "Dictionary does not match expected dictionary (incorrect adler32 hash)");
				efree(ctx->inflateDict);
				ctx->inflateDict = NULL;
				break;
			EMPTY_SWITCH_DEFAULT_CASE()
		}
	}
}
/* }}} */

/* {{{ Incrementally inflate encoded data in the specified context */
CRX_FUNCTION(inflate_add)
{
	crex_string *out;
	char *in_buf;
	size_t in_len, buffer_used = 0, CHUNK_SIZE = 8192;
	zval *res;
	crx_zlib_context *ctx;
	crex_long flush_type = C_SYNC_FLUSH;
	int status;

	if (SUCCESS != crex_parse_parameters(CREX_NUM_ARGS(), "Os|l", &res, inflate_context_ce, &in_buf, &in_len, &flush_type)) {
		RETURN_THROWS();
	}

	ctx = C_INFLATE_CONTEXT_P(res);

	switch (flush_type) {
		case C_NO_FLUSH:
		case C_PARTIAL_FLUSH:
		case C_SYNC_FLUSH:
		case C_FULL_FLUSH:
		case C_BLOCK:
		case C_FINISH:
			break;

		default:
			crex_argument_value_error(3, "must be one of ZLIB_NO_FLUSH, ZLIB_PARTIAL_FLUSH, ZLIB_SYNC_FLUSH, ZLIB_FULL_FLUSH, ZLIB_BLOCK, or ZLIB_FINISH");
			RETURN_THROWS();
	}

	/* Lazy-resetting the zlib stream so ctx->total_in remains available until the next inflate_add() call. */
	if (ctx->status == C_STREAM_END)
	{
		ctx->status = C_OK;
		inflateReset(&ctx->Z);
	}

	if (in_len <= 0 && flush_type != C_FINISH) {
		RETURN_EMPTY_STRING();
	}

	out = crex_string_alloc((in_len > CHUNK_SIZE) ? in_len : CHUNK_SIZE, 0);
	ctx->Z.next_in = (Bytef *) in_buf;
	ctx->Z.next_out = (Bytef *) ZSTR_VAL(out);
	ctx->Z.avail_in = in_len;
	ctx->Z.avail_out = ZSTR_LEN(out);

	do {
		status = inflate(&ctx->Z, flush_type);
		buffer_used = ZSTR_LEN(out) - ctx->Z.avail_out;

		ctx->status = status; /* Save status for exposing to userspace */

		switch (status) {
			case C_OK:
				if (ctx->Z.avail_out == 0) {
					/* more output buffer space needed; realloc and try again */
					out = crex_string_realloc(out, ZSTR_LEN(out) + CHUNK_SIZE, 0);
					ctx->Z.avail_out = CHUNK_SIZE;
					ctx->Z.next_out = (Bytef *) ZSTR_VAL(out) + buffer_used;
					break;
				} else {
					goto complete;
				}
			case C_STREAM_END:
				goto complete;
			case C_BUF_ERROR:
				if (flush_type == C_FINISH && ctx->Z.avail_out == 0) {
					/* more output buffer space needed; realloc and try again */
					out = crex_string_realloc(out, ZSTR_LEN(out) + CHUNK_SIZE, 0);
					ctx->Z.avail_out = CHUNK_SIZE;
					ctx->Z.next_out = (Bytef *) ZSTR_VAL(out) + buffer_used;
					break;
				} else {
					/* No more input data; we're finished */
					goto complete;
				}
			case C_NEED_DICT:
				if (ctx->inflateDict) {
					switch (inflateSetDictionary(&ctx->Z, (Bytef *) ctx->inflateDict, ctx->inflateDictlen)) {
						case C_OK:
							efree(ctx->inflateDict);
							ctx->inflateDict = NULL;
							break;
						case C_DATA_ERROR:
							efree(ctx->inflateDict);
							ctx->inflateDict = NULL;
							crex_string_release_ex(out, 0);
							crx_error_docref(NULL, E_WARNING, "Dictionary does not match expected dictionary (incorrect adler32 hash)");
							RETURN_FALSE;
						EMPTY_SWITCH_DEFAULT_CASE()
					}
					break;
				} else {
					crx_error_docref(NULL, E_WARNING, "Inflating this data requires a preset dictionary, please specify it in the options array of inflate_init()");
					RETURN_FALSE;
				}
			default:
				crex_string_release_ex(out, 0);
				crx_error_docref(NULL, E_WARNING, "%s", zError(status));
				RETURN_FALSE;
		}
	} while (1);

complete:
	out = crex_string_realloc(out, buffer_used, 0);
	ZSTR_VAL(out)[buffer_used] = 0;
	RETURN_STR(out);
}
/* }}} */

/* {{{ Get decompression status, usually returns either ZLIB_OK or ZLIB_STREAM_END. */
CRX_FUNCTION(inflate_get_status)
{
	zval *res;
	crx_zlib_context *ctx;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &res, inflate_context_ce) != SUCCESS) {
		RETURN_THROWS();
	}

	ctx = C_INFLATE_CONTEXT_P(res);

	RETURN_LONG(ctx->status);
}
/* }}} */

/* {{{ Get number of bytes read so far. */
CRX_FUNCTION(inflate_get_read_len)
{
	zval *res;
	crx_zlib_context *ctx;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &res, inflate_context_ce) != SUCCESS) {
		RETURN_THROWS();
	}

	ctx = C_INFLATE_CONTEXT_P(res);

	RETURN_LONG(ctx->Z.total_in);
}
/* }}} */

/* {{{ Initialize an incremental deflate context using the specified encoding */
CRX_FUNCTION(deflate_init)
{
	crx_zlib_context *ctx;
	crex_long encoding, level = -1, memory = 8, window = 15, strategy = C_DEFAULT_STRATEGY;
	char *dict = NULL;
	size_t dictlen = 0;
	HashTable *options = NULL;
	zval *option_buffer;

	if (SUCCESS != crex_parse_parameters(CREX_NUM_ARGS(), "l|H", &encoding, &options)) {
		RETURN_THROWS();
	}

	if (options && (option_buffer = crex_hash_str_find(options, CREX_STRL("level"))) != NULL) {
		level = zval_get_long(option_buffer);
	}
	if (level < -1 || level > 9) {
		crex_value_error("deflate_init(): \"level\" option must be between -1 and 9");
		RETURN_THROWS();
	}

	if (options && (option_buffer = crex_hash_str_find(options, CREX_STRL("memory"))) != NULL) {
		memory = zval_get_long(option_buffer);
	}
	if (memory < 1 || memory > 9) {
		crex_value_error("deflate_init(): \"memory\" option must be between 1 and 9");
		RETURN_THROWS();
	}

	if (options && (option_buffer = crex_hash_str_find(options, CREX_STRL("window"))) != NULL) {
		window = zval_get_long(option_buffer);
	}
	if (window < 8 || window > 15) {
		crex_value_error("deflate_init(): \"window\" option must be between 8 and 15");
		RETURN_THROWS();
	}

	if (options && (option_buffer = crex_hash_str_find(options, CREX_STRL("strategy"))) != NULL) {
		strategy = zval_get_long(option_buffer);
	}
	switch (strategy) {
		case C_FILTERED:
		case C_HUFFMAN_ONLY:
		case C_RLE:
		case C_FIXED:
		case C_DEFAULT_STRATEGY:
			break;
		default:
			crex_value_error("deflate_init(): \"strategy\" option must be one of ZLIB_FILTERED, ZLIB_HUFFMAN_ONLY, ZLIB_RLE, ZLIB_FIXED, or ZLIB_DEFAULT_STRATEGY");
			RETURN_THROWS();
	}

	if (!zlib_create_dictionary_string(options, &dict, &dictlen)) {
		RETURN_THROWS();
	}

	switch (encoding) {
		case CRX_ZLIB_ENCODING_RAW:
		case CRX_ZLIB_ENCODING_GZIP:
		case CRX_ZLIB_ENCODING_DEFLATE:
			break;
		default:
			crex_argument_value_error(1, "must be one of ZLIB_ENCODING_RAW, ZLIB_ENCODING_GZIP, or ZLIB_ENCODING_DEFLATE");
			RETURN_THROWS();
	}

	object_init_ex(return_value, deflate_context_ce);
	ctx = C_DEFLATE_CONTEXT_P(return_value);

	ctx->Z.zalloc = crx_zlib_alloc;
	ctx->Z.zfree = crx_zlib_free;

	if (encoding < 0) {
		encoding += 15 - window;
	} else {
		encoding -= 15 - window;
	}

	if (deflateInit2(&ctx->Z, level, C_DEFLATED, encoding, memory, strategy) != C_OK) {
		zval_ptr_dtor(return_value);
		crx_error_docref(NULL, E_WARNING, "Failed allocating zlib.deflate context");
		RETURN_FALSE;
	}

	if (dict) {
		int success = deflateSetDictionary(&ctx->Z, (Bytef *) dict, dictlen);
		CREX_ASSERT(success == C_OK);
		efree(dict);
	}
}
/* }}} */

/* {{{ Incrementally deflate data in the specified context */
CRX_FUNCTION(deflate_add)
{
	crex_string *out;
	char *in_buf;
	size_t in_len, out_size, buffer_used;
	zval *res;
	crx_zlib_context *ctx;
	crex_long flush_type = C_SYNC_FLUSH;
	int status;

	if (SUCCESS != crex_parse_parameters(CREX_NUM_ARGS(), "Os|l", &res, deflate_context_ce, &in_buf, &in_len, &flush_type)) {
		RETURN_THROWS();
	}

	ctx = C_DEFLATE_CONTEXT_P(res);

	switch (flush_type) {
		case C_BLOCK:
#if ZLIB_VERNUM < 0x1240L
			crex_throw_error(NULL, "zlib >= 1.2.4 required for BLOCK deflate; current version: %s", ZLIB_VERSION);
			RETURN_THROWS();
#endif
		case C_NO_FLUSH:
		case C_PARTIAL_FLUSH:
		case C_SYNC_FLUSH:
		case C_FULL_FLUSH:
		case C_FINISH:
			break;

		default:
			crex_argument_value_error(3, "must be one of ZLIB_NO_FLUSH, ZLIB_PARTIAL_FLUSH, ZLIB_SYNC_FLUSH, ZLIB_FULL_FLUSH, ZLIB_BLOCK, or ZLIB_FINISH");
			RETURN_THROWS();
	}

	if (in_len <= 0 && flush_type != C_FINISH) {
		RETURN_EMPTY_STRING();
	}

	out_size = CRX_ZLIB_BUFFER_SIZE_GUESS(in_len);
	out_size = (out_size < 64) ? 64 : out_size;
	out = crex_string_alloc(out_size, 0);

	ctx->Z.next_in = (Bytef *) in_buf;
	ctx->Z.next_out = (Bytef *) ZSTR_VAL(out);
	ctx->Z.avail_in = in_len;
	ctx->Z.avail_out = ZSTR_LEN(out);

	buffer_used = 0;

	do {
		if (ctx->Z.avail_out == 0) {
			/* more output buffer space needed; realloc and try again */
			/* adding 64 more bytes solved every issue I have seen    */
			out = crex_string_realloc(out, ZSTR_LEN(out) + 64, 0);
			ctx->Z.avail_out = 64;
			ctx->Z.next_out = (Bytef *) ZSTR_VAL(out) + buffer_used;
		}
		status = deflate(&ctx->Z, flush_type);
		buffer_used = ZSTR_LEN(out) - ctx->Z.avail_out;
	} while (status == C_OK && ctx->Z.avail_out == 0);

	switch (status) {
		case C_OK:
			ZSTR_LEN(out) = (char *) ctx->Z.next_out - ZSTR_VAL(out);
			ZSTR_VAL(out)[ZSTR_LEN(out)] = 0;
			RETURN_STR(out);
			break;
		case C_STREAM_END:
			ZSTR_LEN(out) = (char *) ctx->Z.next_out - ZSTR_VAL(out);
			ZSTR_VAL(out)[ZSTR_LEN(out)] = 0;
			deflateReset(&ctx->Z);
			RETURN_STR(out);
			break;
		default:
			crex_string_release_ex(out, 0);
			crx_error_docref(NULL, E_WARNING, "zlib error (%s)", zError(status));
			RETURN_FALSE;
	}
}
/* }}} */

#ifdef COMPILE_DL_ZLIB
#ifdef ZTS
CREX_TSRMLS_CACHE_DEFINE()
#endif
CREX_GET_MODULE(crx_zlib)
#endif

/* {{{ OnUpdate_zlib_output_compression */
static CRX_INI_MH(OnUpdate_zlib_output_compression)
{
	int int_value;
	char *ini_value;
	if (new_value == NULL) {
		return FAILURE;
	}

	if (crex_string_equals_literal_ci(new_value, "off")) {
		int_value = 0;
	} else if (crex_string_equals_literal_ci(new_value, "on")) {
		int_value = 1;
	} else {
		int_value = (int) crex_ini_parse_quantity_warn(new_value, entry->name);
	}
	ini_value = crex_ini_string("output_handler", sizeof("output_handler") - 1, 0);

	if (ini_value && *ini_value && int_value) {
		crx_error_docref("ref.outcontrol", E_CORE_ERROR, "Cannot use both zlib.output_compression and output_handler together!!");
		return FAILURE;
	}
	if (stage == CRX_INI_STAGE_RUNTIME) {
		int status = crx_output_get_status();
		if (status & CRX_OUTPUT_SENT) {
			crx_error_docref("ref.outcontrol", E_WARNING, "Cannot change zlib.output_compression - headers already sent");
			return FAILURE;
		}
	}

	crex_long *p = (crex_long *) CREX_INI_GET_ADDR();
	*p = int_value;

	ZLIBG(output_compression) = ZLIBG(output_compression_default);
	if (stage == CRX_INI_STAGE_RUNTIME && int_value) {
		if (!crx_output_handler_started(CREX_STRL(CRX_ZLIB_OUTPUT_HANDLER_NAME))) {
			crx_zlib_output_compression_start();
		}
	}

	return SUCCESS;
}
/* }}} */

/* {{{ OnUpdate_zlib_output_handler */
static CRX_INI_MH(OnUpdate_zlib_output_handler)
{
	if (stage == CRX_INI_STAGE_RUNTIME && (crx_output_get_status() & CRX_OUTPUT_SENT)) {
		crx_error_docref("ref.outcontrol", E_WARNING, "Cannot change zlib.output_handler - headers already sent");
		return FAILURE;
	}

	return OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
}
/* }}} */

/* {{{ INI */
CRX_INI_BEGIN()
	STD_CRX_INI_BOOLEAN("zlib.output_compression",      "0", CRX_INI_ALL, OnUpdate_zlib_output_compression,       output_compression_default,       crex_zlib_globals, zlib_globals)
	STD_CRX_INI_ENTRY("zlib.output_compression_level", "-1", CRX_INI_ALL, OnUpdateLong,                           output_compression_level, crex_zlib_globals, zlib_globals)
	STD_CRX_INI_ENTRY("zlib.output_handler",             "", CRX_INI_ALL, OnUpdate_zlib_output_handler,           output_handler,           crex_zlib_globals, zlib_globals)
CRX_INI_END()

/* }}} */

/* {{{ CRX_MINIT_FUNCTION */
static CRX_MINIT_FUNCTION(zlib)
{
	crx_register_url_stream_wrapper("compress.zlib", &crx_stream_gzip_wrapper);
	crx_stream_filter_register_factory("zlib.*", &crx_zlib_filter_factory);

	crx_output_handler_alias_register(CREX_STRL("ob_gzhandler"), crx_zlib_output_handler_init);
	crx_output_handler_conflict_register(CREX_STRL("ob_gzhandler"), crx_zlib_output_conflict_check);
	crx_output_handler_conflict_register(CREX_STRL(CRX_ZLIB_OUTPUT_HANDLER_NAME), crx_zlib_output_conflict_check);

	inflate_context_ce = register_class_InflateContext();
	inflate_context_ce->create_object = inflate_context_create_object;
	inflate_context_ce->default_object_handlers = &inflate_context_object_handlers;

	memcpy(&inflate_context_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	inflate_context_object_handlers.offset = XtOffsetOf(crx_zlib_context, std);
	inflate_context_object_handlers.free_obj = inflate_context_free_obj;
	inflate_context_object_handlers.get_constructor = inflate_context_get_constructor;
	inflate_context_object_handlers.clone_obj = NULL;
	inflate_context_object_handlers.compare = crex_objects_not_comparable;

	deflate_context_ce = register_class_DeflateContext();
	deflate_context_ce->create_object = deflate_context_create_object;
	deflate_context_ce->default_object_handlers = &deflate_context_object_handlers;

	memcpy(&deflate_context_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	deflate_context_object_handlers.offset = XtOffsetOf(crx_zlib_context, std);
	deflate_context_object_handlers.free_obj = deflate_context_free_obj;
	deflate_context_object_handlers.get_constructor = deflate_context_get_constructor;
	deflate_context_object_handlers.clone_obj = NULL;
	deflate_context_object_handlers.compare = crex_objects_not_comparable;

	register_zlib_symbols(module_number);

	REGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION */
static CRX_MSHUTDOWN_FUNCTION(zlib)
{
	crx_unregister_url_stream_wrapper("zlib");
	crx_stream_filter_unregister_factory("zlib.*");

	UNREGISTER_INI_ENTRIES();

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_RINIT_FUNCTION */
static CRX_RINIT_FUNCTION(zlib)
{
	ZLIBG(compression_coding) = 0;
	if (!ZLIBG(handler_registered)) {
		ZLIBG(output_compression) = ZLIBG(output_compression_default);
		crx_zlib_output_compression_start();
	}

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_RSHUTDOWN_FUNCTION */
static CRX_RSHUTDOWN_FUNCTION(zlib)
{
	crx_zlib_cleanup_ob_gzhandler_mess();
	ZLIBG(handler_registered) = 0;

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
static CRX_MINFO_FUNCTION(zlib)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "ZLib Support", "enabled");
	crx_info_print_table_row(2, "Stream Wrapper", "compress.zlib://");
	crx_info_print_table_row(2, "Stream Filter", "zlib.inflate, zlib.deflate");
	crx_info_print_table_row(2, "Compiled Version", ZLIB_VERSION);
	crx_info_print_table_row(2, "Linked Version", (char *) zlibVersion());
	crx_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ CREX_MODULE_GLOBALS_CTOR */
static CRX_GINIT_FUNCTION(zlib)
{
#if defined(COMPILE_DL_ZLIB) && defined(ZTS)
	CREX_TSRMLS_CACHE_UPDATE();
#endif
	zlib_globals->ob_gzhandler = NULL;
	zlib_globals->handler_registered = 0;
}
/* }}} */

/* {{{ crx_zlib_module_entry */
crex_module_entry crx_zlib_module_entry = {
	STANDARD_MODULE_HEADER,
	"zlib",
	ext_functions,
	CRX_MINIT(zlib),
	CRX_MSHUTDOWN(zlib),
	CRX_RINIT(zlib),
	CRX_RSHUTDOWN(zlib),
	CRX_MINFO(zlib),
	CRX_ZLIB_VERSION,
	CRX_MODULE_GLOBALS(zlib),
	CRX_GINIT(zlib),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */
