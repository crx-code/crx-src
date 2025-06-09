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
   |          Stig Bakken <ssb@crx.net>                                   |
   |          Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   | CRX 4.0 patches by Thies C. Arntzen (thies@thieso.net)               |
   | CRX streams by Wez Furlong (wez@thebrainroom.com)                    |
   +----------------------------------------------------------------------+
*/

/* {{{ includes */

#include "crx.h"
#include "crx_globals.h"
#include "ext/standard/flock_compat.h"
#include "ext/standard/exec.h"
#include "ext/standard/crx_filestat.h"
#include "crx_open_temporary_file.h"
#include "ext/standard/basic_functions.h"
#include "crx_ini.h"
#include "crex_smart_str.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <wchar.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef CRX_WIN32
# include <io.h>
# define O_RDONLY _O_RDONLY
# include "win32/param.h"
# include "win32/winutil.h"
# include "win32/fnmatch.h"
# include "win32/ioutil.h"
#else
# if HAVE_SYS_PARAM_H
#  include <sys/param.h>
# endif
# if HAVE_SYS_SELECT_H
#  include <sys/select.h>
# endif
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
# if HAVE_ARPA_INET_H
#  include <arpa/inet.h>
# endif
#endif

#include "ext/standard/head.h"
#include "crx_string.h"
#include "file.h"

#ifdef HAVE_PWD_H
# ifdef CRX_WIN32
#  include "win32/pwd.h"
# else
#  include <pwd.h>
# endif
#endif

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#include "fsock.h"
#include "fopen_wrappers.h"
#include "streamsfuncs.h"
#include "crx_globals.h"

#ifdef HAVE_SYS_FILE_H
# include <sys/file.h>
#endif

#if MISSING_FCLOSE_DECL
extern int fclose(FILE *);
#endif

#ifdef HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif

#include "scanf.h"
#include "crex_API.h"

#ifdef ZTS
int file_globals_id;
#else
crx_file_globals file_globals;
#endif

#if defined(HAVE_FNMATCH) && !defined(CRX_WIN32)
# ifndef _GNU_SOURCE
#  define _GNU_SOURCE
# endif
# include <fnmatch.h>
#endif

#include "file_arginfo.h"

/* }}} */

#define CRX_STREAM_FROM_ZVAL(stream, arg) \
	CREX_ASSERT(C_TYPE_P(arg) == IS_RESOURCE); \
	crx_stream_from_res(stream, C_RES_P(arg));

/* {{{ ZTS-stuff / Globals / Prototypes */

/* sharing globals is *evil* */
static int le_stream_context = FAILURE;

CRXAPI int crx_le_stream_context(void)
{
	return le_stream_context;
}
/* }}} */

/* {{{ Module-Stuff */
static CREX_RSRC_DTOR_FUNC(file_context_dtor)
{
	crx_stream_context *context = (crx_stream_context*)res->ptr;
	if (C_TYPE(context->options) != IS_UNDEF) {
		zval_ptr_dtor(&context->options);
		ZVAL_UNDEF(&context->options);
	}
	crx_stream_context_free(context);
}

static void file_globals_ctor(crx_file_globals *file_globals_p)
{
	memset(file_globals_p, 0, sizeof(crx_file_globals));
	file_globals_p->def_chunk_size = CRX_SOCK_CHUNK_SIZE;
}

static void file_globals_dtor(crx_file_globals *file_globals_p)
{
#if defined(HAVE_GETHOSTBYNAME_R)
	if (file_globals_p->tmp_host_buf) {
		free(file_globals_p->tmp_host_buf);
	}
#endif
}

static CRX_INI_MH(OnUpdateAutoDetectLineEndings)
{
	if (crex_ini_parse_bool(new_value)) {
		crex_error(E_DEPRECATED, "auto_detect_line_endings is deprecated");
	}
	return OnUpdateBool(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
}

CRX_INI_BEGIN()
	STD_CRX_INI_ENTRY("user_agent", NULL, CRX_INI_ALL, OnUpdateString, user_agent, crx_file_globals, file_globals)
	STD_CRX_INI_ENTRY("from", NULL, CRX_INI_ALL, OnUpdateString, from_address, crx_file_globals, file_globals)
	STD_CRX_INI_ENTRY("default_socket_timeout", "60", CRX_INI_ALL, OnUpdateLong, default_socket_timeout, crx_file_globals, file_globals)
	STD_CRX_INI_BOOLEAN("auto_detect_line_endings", "0", CRX_INI_ALL, OnUpdateAutoDetectLineEndings, auto_detect_line_endings, crx_file_globals, file_globals)
CRX_INI_END()

CRX_MINIT_FUNCTION(file)
{
	le_stream_context = crex_register_list_destructors_ex(file_context_dtor, NULL, "stream-context", module_number);

#ifdef ZTS
	ts_allocate_id(&file_globals_id, sizeof(crx_file_globals), (ts_allocate_ctor) file_globals_ctor, (ts_allocate_dtor) file_globals_dtor);
#else
	file_globals_ctor(&file_globals);
#endif

	REGISTER_INI_ENTRIES();

	register_file_symbols(module_number);

	return SUCCESS;
}
/* }}} */

CRX_MSHUTDOWN_FUNCTION(file) /* {{{ */
{
#ifndef ZTS
	file_globals_dtor(&file_globals);
#endif
	return SUCCESS;
}
/* }}} */

CRXAPI void crx_flock_common(crx_stream *stream, crex_long operation,
	uint32_t operation_arg_num, zval *wouldblock, zval *return_value)
{
	int flock_values[] = { LOCK_SH, LOCK_EX, LOCK_UN };
	int act;

	act = operation & CRX_LOCK_UN;
	if (act < 1 || act > 3) {
		crex_argument_value_error(operation_arg_num, "must be one of LOCK_SH, LOCK_EX, or LOCK_UN");
		RETURN_THROWS();
	}

	if (wouldblock) {
		CREX_TRY_ASSIGN_REF_LONG(wouldblock, 0);
	}

	/* flock_values contains all possible actions if (operation & CRX_LOCK_NB) we won't block on the lock */
	act = flock_values[act - 1] | (operation & CRX_LOCK_NB ? LOCK_NB : 0);
	if (crx_stream_lock(stream, act)) {
		if (operation && errno == EWOULDBLOCK && wouldblock) {
			CREX_TRY_ASSIGN_REF_LONG(wouldblock, 1);
		}
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

/* {{{ Portable file locking */
CRX_FUNCTION(flock)
{
	zval *res, *wouldblock = NULL;
	crx_stream *stream;
	crex_long operation = 0;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_RESOURCE(res)
		C_PARAM_LONG(operation)
		C_PARAM_OPTIONAL
		C_PARAM_ZVAL(wouldblock)
	CREX_PARSE_PARAMETERS_END();

	CRX_STREAM_FROM_ZVAL(stream, res);

	crx_flock_common(stream, operation, 2, wouldblock, return_value);
}
/* }}} */

#define CRX_META_UNSAFE ".\\+*?[^]$() "

/* {{{ Extracts all meta tag content attributes from a file and returns an array */
CRX_FUNCTION(get_meta_tags)
{
	char *filename;
	size_t filename_len;
	bool use_include_path = 0;
	int in_tag = 0, done = 0;
	int looking_for_val = 0, have_name = 0, have_content = 0;
	int saw_name = 0, saw_content = 0;
	char *name = NULL, *value = NULL, *temp = NULL;
	crx_meta_tags_token tok, tok_last;
	crx_meta_tags_data md;

	/* Initialize our structure */
	memset(&md, 0, sizeof(md));

	/* Parse arguments */
	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_PATH(filename, filename_len)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(use_include_path)
	CREX_PARSE_PARAMETERS_END();

	md.stream = crx_stream_open_wrapper(filename, "rb",
			(use_include_path ? USE_PATH : 0) | REPORT_ERRORS,
			NULL);
	if (!md.stream)	{
		RETURN_FALSE;
	}

	array_init(return_value);

	tok_last = TOK_EOF;

	while (!done && (tok = crx_next_meta_token(&md)) != TOK_EOF) {
		if (tok == TOK_ID) {
			if (tok_last == TOK_OPENTAG) {
				md.in_meta = !strcasecmp("meta", md.token_data);
			} else if (tok_last == TOK_SLASH && in_tag) {
				if (strcasecmp("head", md.token_data) == 0) {
					/* We are done here! */
					done = 1;
				}
			} else if (tok_last == TOK_EQUAL && looking_for_val) {
				if (saw_name) {
					if (name) efree(name);
					/* Get the NAME attr (Single word attr, non-quoted) */
					temp = name = estrndup(md.token_data, md.token_len);

					while (temp && *temp) {
						if (strchr(CRX_META_UNSAFE, *temp)) {
							*temp = '_';
						}
						temp++;
					}

					have_name = 1;
				} else if (saw_content) {
					if (value) efree(value);
					value = estrndup(md.token_data, md.token_len);
					have_content = 1;
				}

				looking_for_val = 0;
			} else {
				if (md.in_meta) {
					if (strcasecmp("name", md.token_data) == 0) {
						saw_name = 1;
						saw_content = 0;
						looking_for_val = 1;
					} else if (strcasecmp("content", md.token_data) == 0) {
						saw_name = 0;
						saw_content = 1;
						looking_for_val = 1;
					}
				}
			}
		} else if (tok == TOK_STRING && tok_last == TOK_EQUAL && looking_for_val) {
			if (saw_name) {
				if (name) efree(name);
				/* Get the NAME attr (Quoted single/double) */
				temp = name = estrndup(md.token_data, md.token_len);

				while (temp && *temp) {
					if (strchr(CRX_META_UNSAFE, *temp)) {
						*temp = '_';
					}
					temp++;
				}

				have_name = 1;
			} else if (saw_content) {
				if (value) efree(value);
				value = estrndup(md.token_data, md.token_len);
				have_content = 1;
			}

			looking_for_val = 0;
		} else if (tok == TOK_OPENTAG) {
			if (looking_for_val) {
				looking_for_val = 0;
				have_name = saw_name = 0;
				have_content = saw_content = 0;
			}
			in_tag = 1;
		} else if (tok == TOK_CLOSETAG) {
			if (have_name) {
				/* For BC */
				crex_str_tolower(name, strlen(name));
				if (have_content) {
					add_assoc_string(return_value, name, value);
				} else {
					add_assoc_string(return_value, name, "");
				}

				efree(name);
				if (value) efree(value);
			} else if (have_content) {
				efree(value);
			}

			name = value = NULL;

			/* Reset all of our flags */
			in_tag = looking_for_val = 0;
			have_name = saw_name = 0;
			have_content = saw_content = 0;
			md.in_meta = 0;
		}

		tok_last = tok;

		if (md.token_data)
			efree(md.token_data);

		md.token_data = NULL;
	}

	if (value) efree(value);
	if (name) efree(name);
	crx_stream_close(md.stream);
}
/* }}} */

/* {{{ Read the entire file into a string */
CRX_FUNCTION(file_get_contents)
{
	char *filename;
	size_t filename_len;
	bool use_include_path = 0;
	crx_stream *stream;
	crex_long offset = 0;
	crex_long maxlen;
	bool maxlen_is_null = 1;
	zval *zcontext = NULL;
	crx_stream_context *context = NULL;
	crex_string *contents;

	/* Parse arguments */
	CREX_PARSE_PARAMETERS_START(1, 5)
		C_PARAM_PATH(filename, filename_len)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(use_include_path)
		C_PARAM_RESOURCE_OR_NULL(zcontext)
		C_PARAM_LONG(offset)
		C_PARAM_LONG_OR_NULL(maxlen, maxlen_is_null)
	CREX_PARSE_PARAMETERS_END();

	if (maxlen_is_null) {
		maxlen = (ssize_t) CRX_STREAM_COPY_ALL;
	} else if (maxlen < 0) {
		crex_argument_value_error(5, "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	context = crx_stream_context_from_zval(zcontext, 0);

	stream = crx_stream_open_wrapper_ex(filename, "rb",
				(use_include_path ? USE_PATH : 0) | REPORT_ERRORS,
				NULL, context);
	if (!stream) {
		RETURN_FALSE;
	}

	/* disabling the read buffer allows doing the whole transfer
	   in just one read() system call */
	if (crx_stream_is(stream, CRX_STREAM_IS_STDIO)) {
		crx_stream_set_option(stream, CRX_STREAM_OPTION_READ_BUFFER, CRX_STREAM_BUFFER_NONE, NULL);
	}

	if (offset != 0 && crx_stream_seek(stream, offset, ((offset > 0) ? SEEK_SET : SEEK_END)) < 0) {
		crx_error_docref(NULL, E_WARNING, "Failed to seek to position " CREX_LONG_FMT " in the stream", offset);
		crx_stream_close(stream);
		RETURN_FALSE;
	}

	if ((contents = crx_stream_copy_to_mem(stream, maxlen, 0)) != NULL) {
		RETVAL_STR(contents);
	} else {
		RETVAL_EMPTY_STRING();
	}

	crx_stream_close(stream);
}
/* }}} */

/* {{{ Write/Create a file with contents data and return the number of bytes written */
CRX_FUNCTION(file_put_contents)
{
	crx_stream *stream;
	char *filename;
	size_t filename_len;
	zval *data;
	ssize_t numbytes = 0;
	crex_long flags = 0;
	zval *zcontext = NULL;
	crx_stream_context *context = NULL;
	crx_stream *srcstream = NULL;
	char mode[3] = "wb";

	CREX_PARSE_PARAMETERS_START(2, 4)
		C_PARAM_PATH(filename, filename_len)
		C_PARAM_ZVAL(data)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(flags)
		C_PARAM_RESOURCE_OR_NULL(zcontext)
	CREX_PARSE_PARAMETERS_END();

	if (C_TYPE_P(data) == IS_RESOURCE) {
		crx_stream_from_zval(srcstream, data);
	}

	context = crx_stream_context_from_zval(zcontext, flags & CRX_FILE_NO_DEFAULT_CONTEXT);

	if (flags & CRX_FILE_APPEND) {
		mode[0] = 'a';
	} else if (flags & LOCK_EX) {
		/* check to make sure we are dealing with a regular file */
		if (crx_memnstr(filename, "://", sizeof("://") - 1, filename + filename_len)) {
			if (strncasecmp(filename, "file://", sizeof("file://") - 1)) {
				crx_error_docref(NULL, E_WARNING, "Exclusive locks may only be set for regular files");
				RETURN_FALSE;
			}
		}
		mode[0] = 'c';
	}
	mode[2] = '\0';

	stream = crx_stream_open_wrapper_ex(filename, mode, ((flags & CRX_FILE_USE_INCLUDE_PATH) ? USE_PATH : 0) | REPORT_ERRORS, NULL, context);
	if (stream == NULL) {
		RETURN_FALSE;
	}

	if ((flags & LOCK_EX) && (!crx_stream_supports_lock(stream) || crx_stream_lock(stream, LOCK_EX))) {
		crx_stream_close(stream);
		crx_error_docref(NULL, E_WARNING, "Exclusive locks are not supported for this stream");
		RETURN_FALSE;
	}

	if (mode[0] == 'c') {
		crx_stream_truncate_set_size(stream, 0);
	}

	switch (C_TYPE_P(data)) {
		case IS_RESOURCE: {
			size_t len;
			if (crx_stream_copy_to_stream_ex(srcstream, stream, CRX_STREAM_COPY_ALL, &len) != SUCCESS) {
				numbytes = -1;
			} else {
				if (len > CREX_LONG_MAX) {
					crx_error_docref(NULL, E_WARNING, "content truncated from %zu to " CREX_LONG_FMT " bytes", len, CREX_LONG_MAX);
					len = CREX_LONG_MAX;
				}
				numbytes = len;
			}
			break;
		}
		case IS_NULL:
		case IS_LONG:
		case IS_DOUBLE:
		case IS_FALSE:
		case IS_TRUE:
			convert_to_string(data);
			CREX_FALLTHROUGH;
		case IS_STRING:
			if (C_STRLEN_P(data)) {
				numbytes = crx_stream_write(stream, C_STRVAL_P(data), C_STRLEN_P(data));
				if (numbytes != -1 && numbytes != C_STRLEN_P(data)) {
					crx_error_docref(NULL, E_WARNING, "Only %zd of %zd bytes written, possibly out of free disk space", numbytes, C_STRLEN_P(data));
					numbytes = -1;
				}
			}
			break;

		case IS_ARRAY:
			if (crex_hash_num_elements(C_ARRVAL_P(data))) {
				ssize_t bytes_written;
				zval *tmp;

				CREX_HASH_FOREACH_VAL(C_ARRVAL_P(data), tmp) {
					crex_string *t;
					crex_string *str = zval_get_tmp_string(tmp, &t);
					if (ZSTR_LEN(str)) {
						numbytes += ZSTR_LEN(str);
						bytes_written = crx_stream_write(stream, ZSTR_VAL(str), ZSTR_LEN(str));
						if (bytes_written != ZSTR_LEN(str)) {
							crx_error_docref(NULL, E_WARNING, "Failed to write %zd bytes to %s", ZSTR_LEN(str), filename);
							crex_tmp_string_release(t);
							numbytes = -1;
							break;
						}
					}
					crex_tmp_string_release(t);
				} CREX_HASH_FOREACH_END();
			}
			break;

		case IS_OBJECT:
			if (C_OBJ_HT_P(data) != NULL) {
				zval out;

				if (crex_std_cast_object_tostring(C_OBJ_P(data), &out, IS_STRING) == SUCCESS) {
					numbytes = crx_stream_write(stream, C_STRVAL(out), C_STRLEN(out));
					if (numbytes != -1 && numbytes != C_STRLEN(out)) {
						crx_error_docref(NULL, E_WARNING, "Only %zd of %zd bytes written, possibly out of free disk space", numbytes, C_STRLEN(out));
						numbytes = -1;
					}
					zval_ptr_dtor_str(&out);
					break;
				}
			}
			CREX_FALLTHROUGH;
		default:
			numbytes = -1;
			break;
	}
	crx_stream_close(stream);

	if (numbytes < 0) {
		RETURN_FALSE;
	}

	RETURN_LONG(numbytes);
}
/* }}} */

#define CRX_FILE_BUF_SIZE	80

/* {{{ Read entire file into an array */
CRX_FUNCTION(file)
{
	char *filename;
	size_t filename_len;
	char *p, *s, *e;
	int i = 0;
	char eol_marker = '\n';
	crex_long flags = 0;
	bool use_include_path;
	bool include_new_line;
	bool skip_blank_lines;
	crx_stream *stream;
	zval *zcontext = NULL;
	crx_stream_context *context = NULL;
	crex_string *target_buf;

	/* Parse arguments */
	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_PATH(filename, filename_len)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(flags)
		C_PARAM_RESOURCE_OR_NULL(zcontext)
	CREX_PARSE_PARAMETERS_END();

	if ((flags & ~(CRX_FILE_USE_INCLUDE_PATH | CRX_FILE_IGNORE_NEW_LINES | CRX_FILE_SKIP_EMPTY_LINES | CRX_FILE_NO_DEFAULT_CONTEXT)) != 0) {
		crex_argument_value_error(2, "must be a valid flag value");
		RETURN_THROWS();
	}

	use_include_path = flags & CRX_FILE_USE_INCLUDE_PATH;
	include_new_line = !(flags & CRX_FILE_IGNORE_NEW_LINES);
	skip_blank_lines = flags & CRX_FILE_SKIP_EMPTY_LINES;

	context = crx_stream_context_from_zval(zcontext, flags & CRX_FILE_NO_DEFAULT_CONTEXT);

	stream = crx_stream_open_wrapper_ex(filename, "rb", (use_include_path ? USE_PATH : 0) | REPORT_ERRORS, NULL, context);
	if (!stream) {
		RETURN_FALSE;
	}

	/* Initialize return array */
	array_init(return_value);

	if ((target_buf = crx_stream_copy_to_mem(stream, CRX_STREAM_COPY_ALL, 0)) != NULL) {
		s = ZSTR_VAL(target_buf);
		e = ZSTR_VAL(target_buf) + ZSTR_LEN(target_buf);

		if (!(p = (char*)crx_stream_locate_eol(stream, target_buf))) {
			p = e;
			goto parse_eol;
		}

		if (stream->flags & CRX_STREAM_FLAG_EOL_MAC) {
			eol_marker = '\r';
		}

		/* for performance reasons the code is duplicated, so that the if (include_new_line)
		 * will not need to be done for every single line in the file. */
		if (include_new_line) {
			do {
				p++;
parse_eol:
				add_index_stringl(return_value, i++, s, p-s);
				s = p;
			} while ((p = memchr(p, eol_marker, (e-p))));
		} else {
			do {
				int windows_eol = 0;
				if (p != ZSTR_VAL(target_buf) && eol_marker == '\n' && *(p - 1) == '\r') {
					windows_eol++;
				}
				if (skip_blank_lines && !(p-s-windows_eol)) {
					s = ++p;
					continue;
				}
				add_index_stringl(return_value, i++, s, p-s-windows_eol);
				s = ++p;
			} while ((p = memchr(p, eol_marker, (e-p))));
		}

		/* handle any leftovers of files without new lines */
		if (s != e) {
			p = e;
			goto parse_eol;
		}
	}

	if (target_buf) {
		crex_string_free(target_buf);
	}
	crx_stream_close(stream);
}
/* }}} */

/* {{{ Create a unique filename in a directory */
CRX_FUNCTION(tempnam)
{
	char *dir, *prefix;
	size_t dir_len, prefix_len;
	crex_string *opened_path;
	int fd;
	crex_string *p;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_PATH(dir, dir_len)
		C_PARAM_PATH(prefix, prefix_len)
	CREX_PARSE_PARAMETERS_END();

	p = crx_basename(prefix, prefix_len, NULL, 0);
	if (ZSTR_LEN(p) >= 64) {
		ZSTR_VAL(p)[63] = '\0';
	}

	RETVAL_FALSE;

	if ((fd = crx_open_temporary_fd_ex(dir, ZSTR_VAL(p), &opened_path, CRX_TMP_FILE_OPEN_BASEDIR_CHECK_ALWAYS)) >= 0) {
		close(fd);
		RETVAL_STR(opened_path);
	}
	crex_string_release_ex(p, 0);
}
/* }}} */

/* {{{ Create a temporary file that will be deleted automatically after use */
CRX_FUNCTION(tmpfile)
{
	crx_stream *stream;

	CREX_PARSE_PARAMETERS_NONE();

	stream = crx_stream_fopen_tmpfile();

	if (stream) {
		crx_stream_to_zval(stream, return_value);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Open a file or a URL and return a file pointer */
CRX_FUNCTION(fopen)
{
	char *filename, *mode;
	size_t filename_len, mode_len;
	bool use_include_path = 0;
	zval *zcontext = NULL;
	crx_stream *stream;
	crx_stream_context *context = NULL;

	CREX_PARSE_PARAMETERS_START(2, 4)
		C_PARAM_PATH(filename, filename_len)
		C_PARAM_STRING(mode, mode_len)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(use_include_path)
		C_PARAM_RESOURCE_OR_NULL(zcontext)
	CREX_PARSE_PARAMETERS_END();

	context = crx_stream_context_from_zval(zcontext, 0);

	stream = crx_stream_open_wrapper_ex(filename, mode, (use_include_path ? USE_PATH : 0) | REPORT_ERRORS, NULL, context);

	if (stream == NULL) {
		RETURN_FALSE;
	}

	crx_stream_to_zval(stream, return_value);
}
/* }}} */

/* {{{ Close an open file pointer */
CRXAPI CRX_FUNCTION(fclose)
{
	zval *res;
	crx_stream *stream;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(res)
	CREX_PARSE_PARAMETERS_END();

	CRX_STREAM_FROM_ZVAL(stream, res);

	if ((stream->flags & CRX_STREAM_FLAG_NO_FCLOSE) != 0) {
		crx_error_docref(NULL, E_WARNING, CREX_LONG_FMT " is not a valid stream resource", stream->res->handle);
		RETURN_FALSE;
	}

	crx_stream_free(stream,
		CRX_STREAM_FREE_KEEP_RSRC |
		(stream->is_persistent ? CRX_STREAM_FREE_CLOSE_PERSISTENT : CRX_STREAM_FREE_CLOSE));

	RETURN_TRUE;
}
/* }}} */

/* {{{ Execute a command and open either a read or a write pipe to it */
CRX_FUNCTION(popen)
{
	char *command, *mode;
	size_t command_len, mode_len;
	FILE *fp;
	crx_stream *stream;
	char *posix_mode;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_PATH(command, command_len)
		C_PARAM_STRING(mode, mode_len)
	CREX_PARSE_PARAMETERS_END();

	posix_mode = estrndup(mode, mode_len);
#ifndef CRX_WIN32
	{
		char *z = memchr(posix_mode, 'b', mode_len);
		if (z) {
			memmove(z, z + 1, mode_len - (z - posix_mode));
			mode_len--;
		}
	}
#endif

	/* Musl only partially validates the mode. Manually check it to ensure consistent behavior. */
	if (mode_len > 2 ||
		(mode_len == 1 && (*posix_mode != 'r' && *posix_mode != 'w')) ||
		(mode_len == 2 && (memcmp(posix_mode, "rb", 2) && memcmp(posix_mode, "wb", 2)))
	) {
		crex_argument_value_error(2, "must be one of \"r\", \"rb\", \"w\", or \"wb\"");
		efree(posix_mode);
		RETURN_THROWS();
	}

	fp = VCWD_POPEN(command, posix_mode);
	if (!fp) {
		crx_error_docref2(NULL, command, posix_mode, E_WARNING, "%s", strerror(errno));
		efree(posix_mode);
		RETURN_FALSE;
	}

	stream = crx_stream_fopen_from_pipe(fp, mode);

	if (stream == NULL)	{
		crx_error_docref2(NULL, command, mode, E_WARNING, "%s", strerror(errno));
		RETVAL_FALSE;
	} else {
		crx_stream_to_zval(stream, return_value);
	}

	efree(posix_mode);
}
/* }}} */

/* {{{ Close a file pointer opened by popen() */
CRX_FUNCTION(pclose)
{
	zval *res;
	crx_stream *stream;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(res)
	CREX_PARSE_PARAMETERS_END();

	CRX_STREAM_FROM_ZVAL(stream, res);

	FG(pclose_wait) = 1;
	crex_list_close(stream->res);
	FG(pclose_wait) = 0;
	RETURN_LONG(FG(pclose_ret));
}
/* }}} */

/* {{{ Test for end-of-file on a file pointer */
CRXAPI CRX_FUNCTION(feof)
{
	zval *res;
	crx_stream *stream;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(res)
	CREX_PARSE_PARAMETERS_END();

	CRX_STREAM_FROM_ZVAL(stream, res);

	if (crx_stream_eof(stream)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Get a line from file pointer */
CRXAPI CRX_FUNCTION(fgets)
{
	zval *res;
	crex_long len = 1024;
	bool len_is_null = 1;
	char *buf = NULL;
	size_t line_len = 0;
	crex_string *str;
	crx_stream *stream;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_RESOURCE(res)
		C_PARAM_OPTIONAL
		C_PARAM_LONG_OR_NULL(len, len_is_null)
	CREX_PARSE_PARAMETERS_END();

	CRX_STREAM_FROM_ZVAL(stream, res);

	if (len_is_null) {
		/* ask streams to give us a buffer of an appropriate size */
		buf = crx_stream_get_line(stream, NULL, 0, &line_len);
		if (buf == NULL) {
			RETURN_FALSE;
		}
		// TODO: avoid reallocation ???
		RETVAL_STRINGL(buf, line_len);
		efree(buf);
	} else {
		if (len <= 0) {
			crex_argument_value_error(2, "must be greater than 0");
			RETURN_THROWS();
		}

		str = crex_string_alloc(len, 0);
		if (crx_stream_get_line(stream, ZSTR_VAL(str), len, &line_len) == NULL) {
			crex_string_efree(str);
			RETURN_FALSE;
		}
		/* resize buffer if it's much larger than the result.
		 * Only needed if the user requested a buffer size. */
		if (line_len < (size_t)len / 2) {
			str = crex_string_truncate(str, line_len, 0);
		} else {
			ZSTR_LEN(str) = line_len;
		}
		RETURN_NEW_STR(str);
	}
}
/* }}} */

/* {{{ Get a character from file pointer */
CRXAPI CRX_FUNCTION(fgetc)
{
	zval *res;
	crx_stream *stream;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(res)
	CREX_PARSE_PARAMETERS_END();

	CRX_STREAM_FROM_ZVAL(stream, res);

	int result = crx_stream_getc(stream);

	if (result == EOF) {
		RETVAL_FALSE;
	} else {
		RETURN_CHAR(result);
	}
}
/* }}} */

/* {{{ Implements a mostly ANSI compatible fscanf() */
CRX_FUNCTION(fscanf)
{
	int result, argc = 0;
	size_t format_len;
	zval *args = NULL;
	zval *file_handle;
	char *buf, *format;
	size_t len;
	void *what;

	CREX_PARSE_PARAMETERS_START(2, -1)
		C_PARAM_RESOURCE(file_handle)
		C_PARAM_STRING(format, format_len)
		C_PARAM_VARIADIC('*', args, argc)
	CREX_PARSE_PARAMETERS_END();

	what = crex_fetch_resource2(C_RES_P(file_handle), "File-Handle", crx_file_le_stream(), crx_file_le_pstream());

	/* we can't do a CREX_VERIFY_RESOURCE(what), otherwise we end up
	 * with a leak if we have an invalid filehandle. This needs changing
	 * if the code behind CREX_VERIFY_RESOURCE changed. - cc */
	if (!what) {
		RETURN_THROWS();
	}

	buf = crx_stream_get_line((crx_stream *) what, NULL, 0, &len);
	if (buf == NULL) {
		RETURN_FALSE;
	}

	result = crx_sscanf_internal(buf, format, argc, args, 0, return_value);

	efree(buf);

	if (SCAN_ERROR_WRONG_PARAM_COUNT == result) {
		WRONG_PARAM_COUNT;
	}
}
/* }}} */

/* {{{ Binary-safe file write */
CRXAPI CRX_FUNCTION(fwrite)
{
	zval *res;
	char *input;
	size_t inputlen;
	ssize_t ret;
	size_t num_bytes;
	crex_long maxlen = 0;
	bool maxlen_is_null = 1;
	crx_stream *stream;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_RESOURCE(res)
		C_PARAM_STRING(input, inputlen)
		C_PARAM_OPTIONAL
		C_PARAM_LONG_OR_NULL(maxlen, maxlen_is_null)
	CREX_PARSE_PARAMETERS_END();

	if (maxlen_is_null) {
		num_bytes = inputlen;
	} else if (maxlen <= 0) {
		num_bytes = 0;
	} else {
		num_bytes = MIN((size_t) maxlen, inputlen);
	}

	if (!num_bytes) {
		RETURN_LONG(0);
	}

	CRX_STREAM_FROM_ZVAL(stream, res);

	ret = crx_stream_write(stream, input, num_bytes);
	if (ret < 0) {
		RETURN_FALSE;
	}

	RETURN_LONG(ret);
}
/* }}} */

/* {{{ Flushes output */
CRXAPI CRX_FUNCTION(fflush)
{
	zval *res;
	int ret;
	crx_stream *stream;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(res)
	CREX_PARSE_PARAMETERS_END();

	CRX_STREAM_FROM_ZVAL(stream, res);

	ret = crx_stream_flush(stream);
	if (ret) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Rewind the position of a file pointer */
CRXAPI CRX_FUNCTION(rewind)
{
	zval *res;
	crx_stream *stream;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(res)
	CREX_PARSE_PARAMETERS_END();

	CRX_STREAM_FROM_ZVAL(stream, res);

	if (-1 == crx_stream_rewind(stream)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Get file pointer's read/write position */
CRXAPI CRX_FUNCTION(ftell)
{
	zval *res;
	crex_long ret;
	crx_stream *stream;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(res)
	CREX_PARSE_PARAMETERS_END();

	CRX_STREAM_FROM_ZVAL(stream, res);

	ret = crx_stream_tell(stream);
	if (ret == -1)	{
		RETURN_FALSE;
	}
	RETURN_LONG(ret);
}
/* }}} */

/* {{{ Seek on a file pointer */
CRXAPI CRX_FUNCTION(fseek)
{
	zval *res;
	crex_long offset, whence = SEEK_SET;
	crx_stream *stream;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_RESOURCE(res)
		C_PARAM_LONG(offset)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(whence)
	CREX_PARSE_PARAMETERS_END();

	CRX_STREAM_FROM_ZVAL(stream, res);

	RETURN_LONG(crx_stream_seek(stream, offset, (int) whence));
}
/* }}} */

/* {{{ crx_mkdir */

/* DEPRECATED APIs: Use crx_stream_mkdir() instead */
CRXAPI int crx_mkdir_ex(const char *dir, crex_long mode, int options)
{
	int ret;

	if (crx_check_open_basedir(dir)) {
		return -1;
	}

	if ((ret = VCWD_MKDIR(dir, (mode_t)mode)) < 0 && (options & REPORT_ERRORS)) {
		crx_error_docref(NULL, E_WARNING, "%s", strerror(errno));
	}

	return ret;
}

CRXAPI int crx_mkdir(const char *dir, crex_long mode)
{
	return crx_mkdir_ex(dir, mode, REPORT_ERRORS);
}
/* }}} */

/* {{{ Create a directory */
CRX_FUNCTION(mkdir)
{
	char *dir;
	size_t dir_len;
	zval *zcontext = NULL;
	crex_long mode = 0777;
	bool recursive = 0;
	crx_stream_context *context;

	CREX_PARSE_PARAMETERS_START(1, 4)
		C_PARAM_PATH(dir, dir_len)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(mode)
		C_PARAM_BOOL(recursive)
		C_PARAM_RESOURCE_OR_NULL(zcontext)
	CREX_PARSE_PARAMETERS_END();

	context = crx_stream_context_from_zval(zcontext, 0);

	RETURN_BOOL(crx_stream_mkdir(dir, (int)mode, (recursive ? CRX_STREAM_MKDIR_RECURSIVE : 0) | REPORT_ERRORS, context));
}
/* }}} */

/* {{{ Remove a directory */
CRX_FUNCTION(rmdir)
{
	char *dir;
	size_t dir_len;
	zval *zcontext = NULL;
	crx_stream_context *context;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_PATH(dir, dir_len)
		C_PARAM_OPTIONAL
		C_PARAM_RESOURCE_OR_NULL(zcontext)
	CREX_PARSE_PARAMETERS_END();

	context = crx_stream_context_from_zval(zcontext, 0);

	RETURN_BOOL(crx_stream_rmdir(dir, REPORT_ERRORS, context));
}
/* }}} */

/* {{{ Output a file or a URL */
CRX_FUNCTION(readfile)
{
	char *filename;
	size_t filename_len;
	size_t size = 0;
	bool use_include_path = 0;
	zval *zcontext = NULL;
	crx_stream *stream;
	crx_stream_context *context = NULL;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_PATH(filename, filename_len)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(use_include_path)
		C_PARAM_RESOURCE_OR_NULL(zcontext)
	CREX_PARSE_PARAMETERS_END();

	context = crx_stream_context_from_zval(zcontext, 0);

	stream = crx_stream_open_wrapper_ex(filename, "rb", (use_include_path ? USE_PATH : 0) | REPORT_ERRORS, NULL, context);
	if (stream) {
		size = crx_stream_passthru(stream);
		crx_stream_close(stream);
		RETURN_LONG(size);
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ Return or change the umask */
CRX_FUNCTION(umask)
{
	crex_long mask = 0;
	bool mask_is_null = 1;
	int oldumask;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_LONG_OR_NULL(mask, mask_is_null)
	CREX_PARSE_PARAMETERS_END();

	oldumask = umask(077);

	if (BG(umask) == -1) {
		BG(umask) = oldumask;
	}

	if (mask_is_null) {
		umask(oldumask);
	} else {
		umask((int) mask);
	}

	RETURN_LONG(oldumask);
}
/* }}} */

/* {{{ Output all remaining data from a file pointer */
CRXAPI CRX_FUNCTION(fpassthru)
{
	zval *res;
	size_t size;
	crx_stream *stream;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(res)
	CREX_PARSE_PARAMETERS_END();

	CRX_STREAM_FROM_ZVAL(stream, res);

	size = crx_stream_passthru(stream);
	RETURN_LONG(size);
}
/* }}} */

/* {{{ Rename a file */
CRX_FUNCTION(rename)
{
	char *old_name, *new_name;
	size_t old_name_len, new_name_len;
	zval *zcontext = NULL;
	crx_stream_wrapper *wrapper;
	crx_stream_context *context;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_PATH(old_name, old_name_len)
		C_PARAM_PATH(new_name, new_name_len)
		C_PARAM_OPTIONAL
		C_PARAM_RESOURCE_OR_NULL(zcontext)
	CREX_PARSE_PARAMETERS_END();

	wrapper = crx_stream_locate_url_wrapper(old_name, NULL, 0);

	if (!wrapper || !wrapper->wops) {
		crx_error_docref(NULL, E_WARNING, "Unable to locate stream wrapper");
		RETURN_FALSE;
	}

	if (!wrapper->wops->rename) {
		crx_error_docref(NULL, E_WARNING, "%s wrapper does not support renaming", wrapper->wops->label ? wrapper->wops->label : "Source");
		RETURN_FALSE;
	}

	if (wrapper != crx_stream_locate_url_wrapper(new_name, NULL, 0)) {
		crx_error_docref(NULL, E_WARNING, "Cannot rename a file across wrapper types");
		RETURN_FALSE;
	}

	context = crx_stream_context_from_zval(zcontext, 0);

	RETURN_BOOL(wrapper->wops->rename(wrapper, old_name, new_name, 0, context));
}
/* }}} */

/* {{{ Delete a file */
CRX_FUNCTION(unlink)
{
	char *filename;
	size_t filename_len;
	crx_stream_wrapper *wrapper;
	zval *zcontext = NULL;
	crx_stream_context *context = NULL;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_PATH(filename, filename_len)
		C_PARAM_OPTIONAL
		C_PARAM_RESOURCE_OR_NULL(zcontext)
	CREX_PARSE_PARAMETERS_END();

	context = crx_stream_context_from_zval(zcontext, 0);

	wrapper = crx_stream_locate_url_wrapper(filename, NULL, 0);

	if (!wrapper || !wrapper->wops) {
		crx_error_docref(NULL, E_WARNING, "Unable to locate stream wrapper");
		RETURN_FALSE;
	}

	if (!wrapper->wops->unlink) {
		crx_error_docref(NULL, E_WARNING, "%s does not allow unlinking", wrapper->wops->label ? wrapper->wops->label : "Wrapper");
		RETURN_FALSE;
	}
	RETURN_BOOL(wrapper->wops->unlink(wrapper, filename, REPORT_ERRORS, context));
}
/* }}} */

CRX_FUNCTION(fsync)
{
	zval *res;
	crx_stream *stream;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(res)
	CREX_PARSE_PARAMETERS_END();

	CRX_STREAM_FROM_ZVAL(stream, res);

	if (!crx_stream_sync_supported(stream)) {
		crx_error_docref(NULL, E_WARNING, "Can't fsync this stream!");
		RETURN_FALSE;
	}

	RETURN_BOOL(crx_stream_sync(stream, /* data_only */ 0) == 0);
}

CRX_FUNCTION(fdatasync)
{
	zval *res;
	crx_stream *stream;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(res)
	CREX_PARSE_PARAMETERS_END();

	CRX_STREAM_FROM_ZVAL(stream, res);

	if (!crx_stream_sync_supported(stream)) {
		crx_error_docref(NULL, E_WARNING, "Can't fsync this stream!");
		RETURN_FALSE;
	}

	RETURN_BOOL(crx_stream_sync(stream, /* data_only */ 1) == 0);
}

/* {{{ Truncate file to 'size' length */
CRX_FUNCTION(ftruncate)
{
	zval *fp;
	crex_long size;
	crx_stream *stream;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_RESOURCE(fp)
		C_PARAM_LONG(size)
	CREX_PARSE_PARAMETERS_END();

	if (size < 0) {
		crex_argument_value_error(2, "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	CRX_STREAM_FROM_ZVAL(stream, fp);

	if (!crx_stream_truncate_supported(stream)) {
		crx_error_docref(NULL, E_WARNING, "Can't truncate this stream!");
		RETURN_FALSE;
	}

	RETURN_BOOL(0 == crx_stream_truncate_set_size(stream, size));
}
/* }}} */
CRXAPI void crx_fstat(crx_stream *stream, zval *return_value)
{
	crx_stream_statbuf stat_ssb;
	zval stat_dev, stat_ino, stat_mode, stat_nlink, stat_uid, stat_gid, stat_rdev,
		 stat_size, stat_atime, stat_mtime, stat_ctime, stat_blksize, stat_blocks;
	char *stat_sb_names[13] = {
		"dev", "ino", "mode", "nlink", "uid", "gid", "rdev",
		"size", "atime", "mtime", "ctime", "blksize", "blocks"
	};

	if (crx_stream_stat(stream, &stat_ssb)) {
		RETURN_FALSE;
	}

	array_init(return_value);

	ZVAL_LONG(&stat_dev, stat_ssb.sb.st_dev);
	ZVAL_LONG(&stat_ino, stat_ssb.sb.st_ino);
	ZVAL_LONG(&stat_mode, stat_ssb.sb.st_mode);
	ZVAL_LONG(&stat_nlink, stat_ssb.sb.st_nlink);
	ZVAL_LONG(&stat_uid, stat_ssb.sb.st_uid);
	ZVAL_LONG(&stat_gid, stat_ssb.sb.st_gid);
#ifdef HAVE_STRUCT_STAT_ST_RDEV
	ZVAL_LONG(&stat_rdev, stat_ssb.sb.st_rdev);
#else
	ZVAL_LONG(&stat_rdev, -1);
#endif
	ZVAL_LONG(&stat_size, stat_ssb.sb.st_size);
	ZVAL_LONG(&stat_atime, stat_ssb.sb.st_atime);
	ZVAL_LONG(&stat_mtime, stat_ssb.sb.st_mtime);
	ZVAL_LONG(&stat_ctime, stat_ssb.sb.st_ctime);
#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
	ZVAL_LONG(&stat_blksize, stat_ssb.sb.st_blksize);
#else
	ZVAL_LONG(&stat_blksize,-1);
#endif
#ifdef HAVE_STRUCT_STAT_ST_BLOCKS
	ZVAL_LONG(&stat_blocks, stat_ssb.sb.st_blocks);
#else
	ZVAL_LONG(&stat_blocks,-1);
#endif
	/* Store numeric indexes in proper order */
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &stat_dev);
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &stat_ino);
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &stat_mode);
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &stat_nlink);
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &stat_uid);
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &stat_gid);
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &stat_rdev);
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &stat_size);
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &stat_atime);
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &stat_mtime);
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &stat_ctime);
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &stat_blksize);
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &stat_blocks);

	/* Store string indexes referencing the same zval*/
	crex_hash_str_add_new(C_ARRVAL_P(return_value), stat_sb_names[0], strlen(stat_sb_names[0]), &stat_dev);
	crex_hash_str_add_new(C_ARRVAL_P(return_value), stat_sb_names[1], strlen(stat_sb_names[1]), &stat_ino);
	crex_hash_str_add_new(C_ARRVAL_P(return_value), stat_sb_names[2], strlen(stat_sb_names[2]), &stat_mode);
	crex_hash_str_add_new(C_ARRVAL_P(return_value), stat_sb_names[3], strlen(stat_sb_names[3]), &stat_nlink);
	crex_hash_str_add_new(C_ARRVAL_P(return_value), stat_sb_names[4], strlen(stat_sb_names[4]), &stat_uid);
	crex_hash_str_add_new(C_ARRVAL_P(return_value), stat_sb_names[5], strlen(stat_sb_names[5]), &stat_gid);
	crex_hash_str_add_new(C_ARRVAL_P(return_value), stat_sb_names[6], strlen(stat_sb_names[6]), &stat_rdev);
	crex_hash_str_add_new(C_ARRVAL_P(return_value), stat_sb_names[7], strlen(stat_sb_names[7]), &stat_size);
	crex_hash_str_add_new(C_ARRVAL_P(return_value), stat_sb_names[8], strlen(stat_sb_names[8]), &stat_atime);
	crex_hash_str_add_new(C_ARRVAL_P(return_value), stat_sb_names[9], strlen(stat_sb_names[9]), &stat_mtime);
	crex_hash_str_add_new(C_ARRVAL_P(return_value), stat_sb_names[10], strlen(stat_sb_names[10]), &stat_ctime);
	crex_hash_str_add_new(C_ARRVAL_P(return_value), stat_sb_names[11], strlen(stat_sb_names[11]), &stat_blksize);
	crex_hash_str_add_new(C_ARRVAL_P(return_value), stat_sb_names[12], strlen(stat_sb_names[12]), &stat_blocks);
}

/* {{{ Stat() on a filehandle */
CRX_FUNCTION(fstat)
{
	zval *fp;
	crx_stream *stream;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(fp)
	CREX_PARSE_PARAMETERS_END();

	CRX_STREAM_FROM_ZVAL(stream, fp);

	crx_fstat(stream, return_value);
}
/* }}} */

/* {{{ Copy a file */
CRX_FUNCTION(copy)
{
	char *source, *target;
	size_t source_len, target_len;
	zval *zcontext = NULL;
	crx_stream_context *context;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_PATH(source, source_len)
		C_PARAM_PATH(target, target_len)
		C_PARAM_OPTIONAL
		C_PARAM_RESOURCE_OR_NULL(zcontext)
	CREX_PARSE_PARAMETERS_END();

	if (crx_stream_locate_url_wrapper(source, NULL, 0) == &crx_plain_files_wrapper && crx_check_open_basedir(source)) {
		RETURN_FALSE;
	}

	context = crx_stream_context_from_zval(zcontext, 0);

	if (crx_copy_file_ctx(source, target, 0, context) == SUCCESS) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ crx_copy_file */
CRXAPI int crx_copy_file(const char *src, const char *dest)
{
	return crx_copy_file_ctx(src, dest, 0, NULL);
}
/* }}} */

/* {{{ crx_copy_file_ex */
CRXAPI int crx_copy_file_ex(const char *src, const char *dest, int src_flg)
{
	return crx_copy_file_ctx(src, dest, src_flg, NULL);
}
/* }}} */

/* {{{ crx_copy_file_ctx */
CRXAPI int crx_copy_file_ctx(const char *src, const char *dest, int src_flg, crx_stream_context *ctx)
{
	crx_stream *srcstream = NULL, *deststream = NULL;
	int ret = FAILURE;
	crx_stream_statbuf src_s, dest_s;
	int src_stat_flags = (src_flg & STREAM_DISABLE_OPEN_BASEDIR) ? CRX_STREAM_URL_STAT_IGNORE_OPEN_BASEDIR : 0;

	switch (crx_stream_stat_path_ex(src, src_stat_flags, &src_s, ctx)) {
		case -1:
			/* non-statable stream */
			goto safe_to_copy;
			break;
		case 0:
			break;
		default: /* failed to stat file, does not exist? */
			return ret;
	}
	if (S_ISDIR(src_s.sb.st_mode)) {
		crx_error_docref(NULL, E_WARNING, "The first argument to copy() function cannot be a directory");
		return FAILURE;
	}

	switch (crx_stream_stat_path_ex(dest, CRX_STREAM_URL_STAT_QUIET, &dest_s, ctx)) {
		case -1:
			/* non-statable stream */
			goto safe_to_copy;
			break;
		case 0:
			break;
		default: /* failed to stat file, does not exist? */
			return ret;
	}
	if (S_ISDIR(dest_s.sb.st_mode)) {
		crx_error_docref(NULL, E_WARNING, "The second argument to copy() function cannot be a directory");
		return FAILURE;
	}
	if (!src_s.sb.st_ino || !dest_s.sb.st_ino) {
		goto no_stat;
	}
	if (src_s.sb.st_ino == dest_s.sb.st_ino && src_s.sb.st_dev == dest_s.sb.st_dev) {
		return ret;
	} else {
		goto safe_to_copy;
	}
no_stat:
	{
		char *sp, *dp;
		int res;

		if ((sp = expand_filepath(src, NULL)) == NULL) {
			return ret;
		}
		if ((dp = expand_filepath(dest, NULL)) == NULL) {
			efree(sp);
			goto safe_to_copy;
		}

		res =
#ifndef CRX_WIN32
			!strcmp(sp, dp);
#else
			!strcasecmp(sp, dp);
#endif

		efree(sp);
		efree(dp);
		if (res) {
			return ret;
		}
	}
safe_to_copy:

	srcstream = crx_stream_open_wrapper_ex(src, "rb", src_flg | REPORT_ERRORS, NULL, ctx);

	if (!srcstream) {
		return ret;
	}

	deststream = crx_stream_open_wrapper_ex(dest, "wb", REPORT_ERRORS, NULL, ctx);

	if (srcstream && deststream) {
		ret = crx_stream_copy_to_stream_ex(srcstream, deststream, CRX_STREAM_COPY_ALL, NULL);
	}
	if (srcstream) {
		crx_stream_close(srcstream);
	}
	if (deststream) {
		crx_stream_close(deststream);
	}
	return ret;
}
/* }}} */

/* {{{ Binary-safe file read */
CRXAPI CRX_FUNCTION(fread)
{
	zval *res;
	crex_long len;
	crx_stream *stream;
	crex_string *str;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_RESOURCE(res)
		C_PARAM_LONG(len)
	CREX_PARSE_PARAMETERS_END();

	CRX_STREAM_FROM_ZVAL(stream, res);

	if (len <= 0) {
		crex_argument_value_error(2, "must be greater than 0");
		RETURN_THROWS();
	}

	str = crx_stream_read_to_str(stream, len);
	if (!str) {
		zval_ptr_dtor_str(return_value);
		RETURN_FALSE;
	}

	RETURN_STR(str);
}
/* }}} */

static const char *crx_fgetcsv_lookup_trailing_spaces(const char *ptr, size_t len) /* {{{ */
{
	int inc_len;
	unsigned char last_chars[2] = { 0, 0 };

	while (len > 0) {
		inc_len = (*ptr == '\0' ? 1 : crx_mblen(ptr, len));
		switch (inc_len) {
			case -2:
			case -1:
				inc_len = 1;
				crx_mb_reset();
				break;
			case 0:
				goto quit_loop;
			case 1:
			default:
				last_chars[0] = last_chars[1];
				last_chars[1] = *ptr;
				break;
		}
		ptr += inc_len;
		len -= inc_len;
	}
quit_loop:
	switch (last_chars[1]) {
		case '\n':
			if (last_chars[0] == '\r') {
				return ptr - 2;
			}
			CREX_FALLTHROUGH;
		case '\r':
			return ptr - 1;
	}
	return ptr;
}
/* }}} */

#define FPUTCSV_FLD_CHK(c) memchr(ZSTR_VAL(field_str), c, ZSTR_LEN(field_str))

/* {{{ Format line as CSV and write to file pointer */
CRX_FUNCTION(fputcsv)
{
	char delimiter = ',';					/* allow this to be set as parameter */
	char enclosure = '"';					/* allow this to be set as parameter */
	int escape_char = (unsigned char) '\\';	/* allow this to be set as parameter */
	crx_stream *stream;
	zval *fp = NULL, *fields = NULL;
	ssize_t ret;
	char *delimiter_str = NULL, *enclosure_str = NULL, *escape_str = NULL;
	size_t delimiter_str_len = 0, enclosure_str_len = 0, escape_str_len = 0;
	crex_string *eol_str = NULL;

	CREX_PARSE_PARAMETERS_START(2, 6)
		C_PARAM_RESOURCE(fp)
		C_PARAM_ARRAY(fields)
		C_PARAM_OPTIONAL
		C_PARAM_STRING(delimiter_str, delimiter_str_len)
		C_PARAM_STRING(enclosure_str, enclosure_str_len)
		C_PARAM_STRING(escape_str, escape_str_len)
		C_PARAM_STR_OR_NULL(eol_str)
	CREX_PARSE_PARAMETERS_END();

	if (delimiter_str != NULL) {
		/* Make sure that there is at least one character in string */
		if (delimiter_str_len != 1) {
			crex_argument_value_error(3, "must be a single character");
			RETURN_THROWS();
		}

		/* use first character from string */
		delimiter = *delimiter_str;
	}

	if (enclosure_str != NULL) {
		if (enclosure_str_len != 1) {
			crex_argument_value_error(4, "must be a single character");
			RETURN_THROWS();
		}
		/* use first character from string */
		enclosure = *enclosure_str;
	}

	if (escape_str != NULL) {
		if (escape_str_len > 1) {
			crex_argument_value_error(5, "must be empty or a single character");
			RETURN_THROWS();
		}
		if (escape_str_len < 1) {
			escape_char = CRX_CSV_NO_ESCAPE;
		} else {
			/* use first character from string */
			escape_char = (unsigned char) *escape_str;
		}
	}

	CRX_STREAM_FROM_ZVAL(stream, fp);

	ret = crx_fputcsv(stream, fields, delimiter, enclosure, escape_char, eol_str);
	if (ret < 0) {
		RETURN_FALSE;
	}
	RETURN_LONG(ret);
}
/* }}} */

/* {{{ CRXAPI size_t crx_fputcsv(crx_stream *stream, zval *fields, char delimiter, char enclosure, int escape_char, crex_string *eol_str) */
CRXAPI ssize_t crx_fputcsv(crx_stream *stream, zval *fields, char delimiter, char enclosure, int escape_char, crex_string *eol_str)
{
	uint32_t count, i = 0;
	size_t ret;
	zval *field_tmp;
	smart_str csvline = {0};

	CREX_ASSERT((escape_char >= 0 && escape_char <= UCHAR_MAX) || escape_char == CRX_CSV_NO_ESCAPE);
	count = crex_hash_num_elements(C_ARRVAL_P(fields));
	CREX_HASH_FOREACH_VAL(C_ARRVAL_P(fields), field_tmp) {
		crex_string *tmp_field_str;
		crex_string *field_str = zval_get_tmp_string(field_tmp, &tmp_field_str);

		/* enclose a field that contains a delimiter, an enclosure character, or a newline */
		if (FPUTCSV_FLD_CHK(delimiter) ||
			FPUTCSV_FLD_CHK(enclosure) ||
			(escape_char != CRX_CSV_NO_ESCAPE && FPUTCSV_FLD_CHK(escape_char)) ||
			FPUTCSV_FLD_CHK('\n') ||
			FPUTCSV_FLD_CHK('\r') ||
			FPUTCSV_FLD_CHK('\t') ||
			FPUTCSV_FLD_CHK(' ')
		) {
			char *ch = ZSTR_VAL(field_str);
			char *end = ch + ZSTR_LEN(field_str);
			int escaped = 0;

			smart_str_appendc(&csvline, enclosure);
			while (ch < end) {
				if (escape_char != CRX_CSV_NO_ESCAPE && *ch == escape_char) {
					escaped = 1;
				} else if (!escaped && *ch == enclosure) {
					smart_str_appendc(&csvline, enclosure);
				} else {
					escaped = 0;
				}
				smart_str_appendc(&csvline, *ch);
				ch++;
			}
			smart_str_appendc(&csvline, enclosure);
		} else {
			smart_str_append(&csvline, field_str);
		}

		if (++i != count) {
			smart_str_appendl(&csvline, &delimiter, 1);
		}
		crex_tmp_string_release(tmp_field_str);
	} CREX_HASH_FOREACH_END();

	if (eol_str) {
		smart_str_append(&csvline, eol_str);
	} else {
		smart_str_appendc(&csvline, '\n');
	}
	smart_str_0(&csvline);

	ret = crx_stream_write(stream, ZSTR_VAL(csvline.s), ZSTR_LEN(csvline.s));

	smart_str_free(&csvline);

	return ret;
}
/* }}} */

/* {{{ Get line from file pointer and parse for CSV fields */
CRX_FUNCTION(fgetcsv)
{
	char delimiter = ',';	/* allow this to be set as parameter */
	char enclosure = '"';	/* allow this to be set as parameter */
	int escape = (unsigned char) '\\';

	crex_long len = 0;
	size_t buf_len;
	char *buf;
	crx_stream *stream;

	{
		zval *fd;
		bool len_is_null = 1;
		char *delimiter_str = NULL;
		size_t delimiter_str_len = 0;
		char *enclosure_str = NULL;
		size_t enclosure_str_len = 0;
		char *escape_str = NULL;
		size_t escape_str_len = 0;

		CREX_PARSE_PARAMETERS_START(1, 5)
			C_PARAM_RESOURCE(fd)
			C_PARAM_OPTIONAL
			C_PARAM_LONG_OR_NULL(len, len_is_null)
			C_PARAM_STRING(delimiter_str, delimiter_str_len)
			C_PARAM_STRING(enclosure_str, enclosure_str_len)
			C_PARAM_STRING(escape_str, escape_str_len)
		CREX_PARSE_PARAMETERS_END();

		if (delimiter_str != NULL) {
			/* Make sure that there is at least one character in string */
			if (delimiter_str_len != 1) {
				crex_argument_value_error(3, "must be a single character");
				RETURN_THROWS();
			}

			/* use first character from string */
			delimiter = delimiter_str[0];
		}

		if (enclosure_str != NULL) {
			if (enclosure_str_len != 1) {
				crex_argument_value_error(4, "must be a single character");
				RETURN_THROWS();
			}

			/* use first character from string */
			enclosure = enclosure_str[0];
		}

		if (escape_str != NULL) {
			if (escape_str_len > 1) {
				crex_argument_value_error(5, "must be empty or a single character");
				RETURN_THROWS();
			}

			if (escape_str_len < 1) {
				escape = CRX_CSV_NO_ESCAPE;
			} else {
				escape = (unsigned char) escape_str[0];
			}
		}

		if (len_is_null || len == 0) {
			len = -1;
		} else if (len < 0) {
			crex_argument_value_error(2, "must be a greater than or equal to 0");
			RETURN_THROWS();
		}

		CRX_STREAM_FROM_ZVAL(stream, fd);
	}

	if (len < 0) {
		if ((buf = crx_stream_get_line(stream, NULL, 0, &buf_len)) == NULL) {
			RETURN_FALSE;
		}
	} else {
		buf = emalloc(len + 1);
		if (crx_stream_get_line(stream, buf, len + 1, &buf_len) == NULL) {
			efree(buf);
			RETURN_FALSE;
		}
	}

	HashTable *values = crx_fgetcsv(stream, delimiter, enclosure, escape, buf_len, buf);
	if (values == NULL) {
		values = crx_bc_fgetcsv_empty_line();
	}
	RETURN_ARR(values);
}
/* }}} */

CRXAPI HashTable *crx_bc_fgetcsv_empty_line(void)
{
	HashTable *values = crex_new_array(1);
	zval tmp;
	ZVAL_NULL(&tmp);
	crex_hash_next_index_insert(values, &tmp);
	return values;
}

CRXAPI HashTable *crx_fgetcsv(crx_stream *stream, char delimiter, char enclosure, int escape_char, size_t buf_len, char *buf) /* {{{ */
{
	char *temp, *bptr, *line_end, *limit;
	size_t temp_len, line_end_len;
	int inc_len;
	bool first_field = true;

	CREX_ASSERT((escape_char >= 0 && escape_char <= UCHAR_MAX) || escape_char == CRX_CSV_NO_ESCAPE);

	/* initialize internal state */
	crx_mb_reset();

	/* Now into new section that parses buf for delimiter/enclosure fields */

	/* Strip trailing space from buf, saving end of line in case required for enclosure field */

	bptr = buf;
	line_end = limit = (char *)crx_fgetcsv_lookup_trailing_spaces(buf, buf_len);
	line_end_len = buf_len - (size_t)(limit - buf);

	/* reserve workspace for building each individual field */
	temp_len = buf_len;
	temp = emalloc(temp_len + line_end_len + 1);

	/* Initialize values HashTable */
	HashTable *values = crex_new_array(0);

	/* Main loop to read CSV fields */
	/* NB this routine will return NULL for a blank line */
	do {
		char *comp_end, *hunk_begin;
		char *tptr = temp;

		inc_len = (bptr < limit ? (*bptr == '\0' ? 1 : crx_mblen(bptr, limit - bptr)): 0);
		if (inc_len == 1) {
			char *tmp = bptr;
			while ((*tmp != delimiter) && isspace((int)*(unsigned char *)tmp)) {
				tmp++;
			}
			if (*tmp == enclosure && tmp < limit) {
				bptr = tmp;
			}
		}

		if (first_field && bptr == line_end) {
			crex_array_destroy(values);
			values = NULL;
			break;
		}
		first_field = false;
		/* 2. Read field, leaving bptr pointing at start of next field */
		if (inc_len != 0 && *bptr == enclosure) {
			int state = 0;

			bptr++;	/* move on to first character in field */
			hunk_begin = bptr;

			/* 2A. handle enclosure delimited field */
			for (;;) {
				switch (inc_len) {
					case 0:
						switch (state) {
							case 2:
								memcpy(tptr, hunk_begin, bptr - hunk_begin - 1);
								tptr += (bptr - hunk_begin - 1);
								hunk_begin = bptr;
								goto quit_loop_2;

							case 1:
								memcpy(tptr, hunk_begin, bptr - hunk_begin);
								tptr += (bptr - hunk_begin);
								hunk_begin = bptr;
								CREX_FALLTHROUGH;

							case 0: {
								if (hunk_begin != line_end) {
									memcpy(tptr, hunk_begin, bptr - hunk_begin);
									tptr += (bptr - hunk_begin);
									hunk_begin = bptr;
								}

								/* add the embedded line end to the field */
								memcpy(tptr, line_end, line_end_len);
								tptr += line_end_len;

								/* nothing can be fetched if stream is NULL (e.g. str_getcsv()) */
								if (stream == NULL) {
									/* the enclosure is unterminated */
									if (bptr > limit) {
										/* if the line ends with enclosure, we need to go back by
										 * one character so the \0 character is not copied. */
										if (hunk_begin == bptr) {
											--hunk_begin;
										}
										--bptr;
									}
									goto quit_loop_2;
								}

								size_t new_len;
								char *new_buf = crx_stream_get_line(stream, NULL, 0, &new_len);
								if (!new_buf) {
									/* we've got an unterminated enclosure,
									 * assign all the data from the start of
									 * the enclosure to end of data to the
									 * last element */
									if (bptr > limit) {
										/* if the line ends with enclosure, we need to go back by
										 * one character so the \0 character is not copied. */
										if (hunk_begin == bptr) {
											--hunk_begin;
										}
										--bptr;
									}
									goto quit_loop_2;
								}

								temp_len += new_len;
								char *new_temp = erealloc(temp, temp_len);
								tptr = new_temp + (size_t)(tptr - temp);
								temp = new_temp;

								efree(buf);
								buf_len = new_len;
								bptr = buf = new_buf;
								hunk_begin = buf;

								line_end = limit = (char *)crx_fgetcsv_lookup_trailing_spaces(buf, buf_len);
								line_end_len = buf_len - (size_t)(limit - buf);

								state = 0;
							} break;
						}
						break;

					case -2:
					case -1:
						crx_mb_reset();
						CREX_FALLTHROUGH;
					case 1:
						/* we need to determine if the enclosure is
						 * 'real' or is it escaped */
						switch (state) {
							case 1: /* escaped */
								bptr++;
								state = 0;
								break;
							case 2: /* embedded enclosure ? let's check it */
								if (*bptr != enclosure) {
									/* real enclosure */
									memcpy(tptr, hunk_begin, bptr - hunk_begin - 1);
									tptr += (bptr - hunk_begin - 1);
									hunk_begin = bptr;
									goto quit_loop_2;
								}
								memcpy(tptr, hunk_begin, bptr - hunk_begin);
								tptr += (bptr - hunk_begin);
								bptr++;
								hunk_begin = bptr;
								state = 0;
								break;
							default:
								if (*bptr == enclosure) {
									state = 2;
								} else if (escape_char != CRX_CSV_NO_ESCAPE && *bptr == escape_char) {
									state = 1;
								}
								bptr++;
								break;
						}
						break;

					default:
						switch (state) {
							case 2:
								/* real enclosure */
								memcpy(tptr, hunk_begin, bptr - hunk_begin - 1);
								tptr += (bptr - hunk_begin - 1);
								hunk_begin = bptr;
								goto quit_loop_2;
							case 1:
								bptr += inc_len;
								memcpy(tptr, hunk_begin, bptr - hunk_begin);
								tptr += (bptr - hunk_begin);
								hunk_begin = bptr;
								state = 0;
								break;
							default:
								bptr += inc_len;
								break;
						}
						break;
				}
				inc_len = (bptr < limit ? (*bptr == '\0' ? 1 : crx_mblen(bptr, limit - bptr)): 0);
			}

		quit_loop_2:
			/* look up for a delimiter */
			for (;;) {
				switch (inc_len) {
					case 0:
						goto quit_loop_3;

					case -2:
					case -1:
						inc_len = 1;
						crx_mb_reset();
						CREX_FALLTHROUGH;
					case 1:
						if (*bptr == delimiter) {
							goto quit_loop_3;
						}
						break;
					default:
						break;
				}
				bptr += inc_len;
				inc_len = (bptr < limit ? (*bptr == '\0' ? 1 : crx_mblen(bptr, limit - bptr)): 0);
			}

		quit_loop_3:
			memcpy(tptr, hunk_begin, bptr - hunk_begin);
			tptr += (bptr - hunk_begin);
			bptr += inc_len;
			comp_end = tptr;
		} else {
			/* 2B. Handle non-enclosure field */

			hunk_begin = bptr;

			for (;;) {
				switch (inc_len) {
					case 0:
						goto quit_loop_4;
					case -2:
					case -1:
						inc_len = 1;
						crx_mb_reset();
						CREX_FALLTHROUGH;
					case 1:
						if (*bptr == delimiter) {
							goto quit_loop_4;
						}
						break;
					default:
						break;
				}
				bptr += inc_len;
				inc_len = (bptr < limit ? (*bptr == '\0' ? 1 : crx_mblen(bptr, limit - bptr)): 0);
			}
		quit_loop_4:
			memcpy(tptr, hunk_begin, bptr - hunk_begin);
			tptr += (bptr - hunk_begin);

			comp_end = (char *)crx_fgetcsv_lookup_trailing_spaces(temp, tptr - temp);
			if (*bptr == delimiter) {
				bptr++;
			}
		}

		/* 3. Now pass our field back to crx */
		*comp_end = '\0';

		zval z_tmp;
		ZVAL_STRINGL(&z_tmp, temp, comp_end - temp);
		crex_hash_next_index_insert(values, &z_tmp);
	} while (inc_len > 0);

	efree(temp);
	if (stream) {
		efree(buf);
	}

	return values;
}
/* }}} */

/* {{{ Return the resolved path */
CRX_FUNCTION(realpath)
{
	char *filename;
	size_t filename_len;
	char resolved_path_buff[MAXPATHLEN];

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_PATH(filename, filename_len)
	CREX_PARSE_PARAMETERS_END();

	if (VCWD_REALPATH(filename, resolved_path_buff)) {
		if (crx_check_open_basedir(resolved_path_buff)) {
			RETURN_FALSE;
		}

#ifdef ZTS
		if (VCWD_ACCESS(resolved_path_buff, F_OK)) {
			RETURN_FALSE;
		}
#endif
		RETURN_STRING(resolved_path_buff);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* See http://www.w3.org/TR/html4/intro/sgmltut.html#h-3.2.2 */
#define CRX_META_HTML401_CHARS "-_.:"

/* {{{ crx_next_meta_token
   Tokenizes an HTML file for get_meta_tags */
crx_meta_tags_token crx_next_meta_token(crx_meta_tags_data *md)
{
	int ch = 0, compliment;
	char buff[META_DEF_BUFSIZE + 1];

	memset((void *)buff, 0, META_DEF_BUFSIZE + 1);

	while (md->ulc || (!crx_stream_eof(md->stream) && (ch = crx_stream_getc(md->stream)))) {
		if (crx_stream_eof(md->stream)) {
			break;
		}

		if (md->ulc) {
			ch = md->lc;
			md->ulc = 0;
		}

		switch (ch) {
			case '<':
				return TOK_OPENTAG;
				break;

			case '>':
				return TOK_CLOSETAG;
				break;

			case '=':
				return TOK_EQUAL;
				break;
			case '/':
				return TOK_SLASH;
				break;

			case '\'':
			case '"':
				compliment = ch;
				md->token_len = 0;
				while (!crx_stream_eof(md->stream) && (ch = crx_stream_getc(md->stream)) && ch != compliment && ch != '<' && ch != '>') {
					buff[(md->token_len)++] = ch;

					if (md->token_len == META_DEF_BUFSIZE) {
						break;
					}
				}

				if (ch == '<' || ch == '>') {
					/* Was just an apostrophe */
					md->ulc = 1;
					md->lc = ch;
				}

				/* We don't need to alloc unless we are in a meta tag */
				if (md->in_meta) {
					md->token_data = (char *) emalloc(md->token_len + 1);
					memcpy(md->token_data, buff, md->token_len+1);
				}

				return TOK_STRING;
				break;

			case '\n':
			case '\r':
			case '\t':
				break;

			case ' ':
				return TOK_SPACE;
				break;

			default:
				if (isalnum(ch)) {
					md->token_len = 0;
					buff[(md->token_len)++] = ch;
					while (!crx_stream_eof(md->stream) && (ch = crx_stream_getc(md->stream)) && (isalnum(ch) || strchr(CRX_META_HTML401_CHARS, ch))) {
						buff[(md->token_len)++] = ch;

						if (md->token_len == META_DEF_BUFSIZE) {
							break;
						}
					}

					/* This is ugly, but we have to replace ungetc */
					if (!isalpha(ch) && ch != '-') {
						md->ulc = 1;
						md->lc = ch;
					}

					md->token_data = (char *) emalloc(md->token_len + 1);
					memcpy(md->token_data, buff, md->token_len+1);

					return TOK_ID;
				} else {
					return TOK_OTHER;
				}
				break;
		}
	}

	return TOK_EOF;
}
/* }}} */

#ifdef HAVE_FNMATCH
/* {{{ Match filename against pattern */
CRX_FUNCTION(fnmatch)
{
	char *pattern, *filename;
	size_t pattern_len, filename_len;
	crex_long flags = 0;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_PATH(pattern, pattern_len)
		C_PARAM_PATH(filename, filename_len)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(flags)
	CREX_PARSE_PARAMETERS_END();

	if (filename_len >= MAXPATHLEN) {
		crx_error_docref(NULL, E_WARNING, "Filename exceeds the maximum allowed length of %d characters", MAXPATHLEN);
		RETURN_FALSE;
	}
	if (pattern_len >= MAXPATHLEN) {
		crx_error_docref(NULL, E_WARNING, "Pattern exceeds the maximum allowed length of %d characters", MAXPATHLEN);
		RETURN_FALSE;
	}

	RETURN_BOOL( ! fnmatch( pattern, filename, (int)flags ));
}
/* }}} */
#endif

/* {{{ Returns directory path used for temporary files */
CRX_FUNCTION(sys_get_temp_dir)
{
	CREX_PARSE_PARAMETERS_NONE();

	RETURN_STRING((char *)crx_get_temporary_directory());
}
/* }}} */
