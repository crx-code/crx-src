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
  | Author: Ilia Alshanetsky <ilia@crx.net>                              |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "crx.h"

#include "libmagic/magic.h"
/*
 * HOWMANY specifies the maximum offset libmagic will look at
 * this is currently hardcoded in the libmagic source but not exported
 */
#ifndef HOWMANY
#define HOWMANY 65536
#endif

#include "crx_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/file.h" /* needed for context stuff */
#include "crx_fileinfo.h"
#include "fileinfo_arginfo.h"
#include "fopen_wrappers.h" /* needed for is_url */
#include "Crex/crex_exceptions.h"

/* {{{ macros and type definitions */
typedef struct _crx_fileinfo {
	crex_long options;
	struct magic_set *magic;
} crx_fileinfo;

static crex_object_handlers finfo_object_handlers;
crex_class_entry *finfo_class_entry;

typedef struct _finfo_object {
	crx_fileinfo *ptr;
	crex_object zo;
} finfo_object;

static inline finfo_object *crx_finfo_fetch_object(crex_object *obj) {
	return (finfo_object *)((char*)(obj) - XtOffsetOf(finfo_object, zo));
}

#define C_FINFO_P(zv) crx_finfo_fetch_object(C_OBJ_P((zv)))

#define FILEINFO_FROM_OBJECT(finfo, object) \
{ \
	finfo_object *obj = C_FINFO_P(object); \
	finfo = obj->ptr; \
	if (!finfo) { \
		crex_throw_error(NULL, "Invalid finfo object"); \
		RETURN_THROWS(); \
	} \
}

/* {{{ finfo_objects_free */
static void finfo_objects_free(crex_object *object)
{
	finfo_object *intern = crx_finfo_fetch_object(object);

	if (intern->ptr) {
		magic_close(intern->ptr->magic);
		efree(intern->ptr);
	}

	crex_object_std_dtor(&intern->zo);
}
/* }}} */

/* {{{ finfo_objects_new */
CRX_FILEINFO_API crex_object *finfo_objects_new(crex_class_entry *class_type)
{
	finfo_object *intern;

	intern = crex_object_alloc(sizeof(finfo_object), class_type);

	crex_object_std_init(&intern->zo, class_type);
	object_properties_init(&intern->zo, class_type);

	return &intern->zo;
}
/* }}} */

#define FINFO_SET_OPTION(magic, options) \
	if (magic_setflags(magic, options) == -1) { \
		crx_error_docref(NULL, E_WARNING, "Failed to set option '" CREX_LONG_FMT "' %d:%s", \
				options, magic_errno(magic), magic_error(magic)); \
		RETURN_FALSE; \
	}
/* }}} */

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(finfo)
{
	finfo_class_entry = register_class_finfo();
	finfo_class_entry->create_object = finfo_objects_new;
	finfo_class_entry->default_object_handlers = &finfo_object_handlers;

	/* copy the standard object handlers to you handler table */
	memcpy(&finfo_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	finfo_object_handlers.offset = XtOffsetOf(finfo_object, zo);
	finfo_object_handlers.free_obj = finfo_objects_free;
	finfo_object_handlers.clone_obj = NULL;

	register_fileinfo_symbols(module_number);

	return SUCCESS;
}
/* }}} */

/* {{{ fileinfo_module_entry */
crex_module_entry fileinfo_module_entry = {
	STANDARD_MODULE_HEADER,
	"fileinfo",
	ext_functions,
	CRX_MINIT(finfo),
	NULL,
	NULL,
	NULL,
	CRX_MINFO(fileinfo),
	CRX_FILEINFO_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_FILEINFO
CREX_GET_MODULE(fileinfo)
#endif

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(fileinfo)
{
	char magic_ver[5];

	(void)snprintf(magic_ver, 4, "%d", magic_version());
	magic_ver[4] = '\0';

	crx_info_print_table_start();
	crx_info_print_table_row(2, "fileinfo support", "enabled");
	crx_info_print_table_row(2, "libmagic", magic_ver);
	crx_info_print_table_end();
}
/* }}} */

/* {{{ Construct a new fileinfo object. */
CRX_FUNCTION(finfo_open)
{
	crex_long options = MAGIC_NONE;
	char *file = NULL;
	size_t file_len = 0;
	crx_fileinfo *finfo;
	zval *object = getThis();
	char resolved_path[MAXPATHLEN];
	crex_error_handling zeh;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|lp!", &options, &file, &file_len) == FAILURE) {
		RETURN_THROWS();
	}

	if (object) {
		finfo_object *finfo_obj = C_FINFO_P(object);

		crex_replace_error_handling(EH_THROW, NULL, &zeh);

		if (finfo_obj->ptr) {
			magic_close(finfo_obj->ptr->magic);
			efree(finfo_obj->ptr);
			finfo_obj->ptr = NULL;
		}
	}

	if (file_len == 0) {
		file = NULL;
	} else if (file && *file) { /* user specified file, perform open_basedir checks */

		if (crx_check_open_basedir(file)) {
			if (object) {
				crex_restore_error_handling(&zeh);
				if (!EG(exception)) {
					crex_throw_exception(NULL, "Constructor failed", 0);
				}
			}
			RETURN_FALSE;
		}
		if (!expand_filepath_with_mode(file, resolved_path, NULL, 0, CWD_EXPAND)) {
			if (object) {
				crex_restore_error_handling(&zeh);
				if (!EG(exception)) {
					crex_throw_exception(NULL, "Constructor failed", 0);
				}
			}
			RETURN_FALSE;
		}
		file = resolved_path;
	}

	finfo = emalloc(sizeof(crx_fileinfo));

	finfo->options = options;
	finfo->magic = magic_open(options);

	if (finfo->magic == NULL) {
		efree(finfo);
		crx_error_docref(NULL, E_WARNING, "Invalid mode '" CREX_LONG_FMT "'.", options);
		if (object) {
			crex_restore_error_handling(&zeh);
			if (!EG(exception)) {
				crex_throw_exception(NULL, "Constructor failed", 0);
			}
		}
		RETURN_FALSE;
	}

	if (magic_load(finfo->magic, file) == -1) {
		crx_error_docref(NULL, E_WARNING, "Failed to load magic database at \"%s\"", file);
		magic_close(finfo->magic);
		efree(finfo);
		if (object) {
			crex_restore_error_handling(&zeh);
			if (!EG(exception)) {
				crex_throw_exception(NULL, "Constructor failed", 0);
			}
		}
		RETURN_FALSE;
	}

	if (object) {
		finfo_object *obj;
		crex_restore_error_handling(&zeh);
		obj = C_FINFO_P(object);
		obj->ptr = finfo;
	} else {
		crex_object *zobj = finfo_objects_new(finfo_class_entry);
		finfo_object *obj = crx_finfo_fetch_object(zobj);
		obj->ptr = finfo;
		RETURN_OBJ(zobj);
	}
}
/* }}} */

/* {{{ Close fileinfo object - a NOP. */
CRX_FUNCTION(finfo_close)
{
	zval *self;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &self, finfo_class_entry) == FAILURE) {
		RETURN_THROWS();
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Set libmagic configuration options. */
CRX_FUNCTION(finfo_set_flags)
{
	crex_long options;
	crx_fileinfo *finfo;
	zval *self;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "Ol", &self, finfo_class_entry, &options) == FAILURE) {
		RETURN_THROWS();
	}
	FILEINFO_FROM_OBJECT(finfo, self);

	FINFO_SET_OPTION(finfo->magic, options)
	finfo->options = options;

	RETURN_TRUE;
}
/* }}} */

#define FILEINFO_MODE_BUFFER 0
#define FILEINFO_MODE_STREAM 1
#define FILEINFO_MODE_FILE 2

static void _crx_finfo_get_type(INTERNAL_FUNCTION_PARAMETERS, int mode, int mimetype_emu) /* {{{ */
{
	crex_long options = 0;
	char *ret_val = NULL, *buffer = NULL;
	size_t buffer_len;
	crx_fileinfo *finfo = NULL;
	zval *zcontext = NULL;
	zval *what;
	char mime_directory[] = "directory";
	struct magic_set *magic = NULL;

	if (mimetype_emu) {

		/* mime_content_type(..) emulation */
		if (crex_parse_parameters(CREX_NUM_ARGS(), "z", &what) == FAILURE) {
			RETURN_THROWS();
		}

		switch (C_TYPE_P(what)) {
			case IS_STRING:
				buffer = C_STRVAL_P(what);
				buffer_len = C_STRLEN_P(what);
				mode = FILEINFO_MODE_FILE;
				break;

			case IS_RESOURCE:
				mode = FILEINFO_MODE_STREAM;
				break;

			default:
				crex_argument_type_error(1, "must be of type resource|string, %s given", crex_zval_value_name(what));
				RETURN_THROWS();
		}

		magic = magic_open(MAGIC_MIME_TYPE);
		if (magic_load(magic, NULL) == -1) {
			crx_error_docref(NULL, E_WARNING, "Failed to load magic database");
			goto common;
		}
	} else {
		zval *self;
		if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "Os|lr!", &self, finfo_class_entry, &buffer, &buffer_len, &options, &zcontext) == FAILURE) {
			RETURN_THROWS();
		}
		FILEINFO_FROM_OBJECT(finfo, self);
		magic = finfo->magic;
	}

	/* Set options for the current file/buffer. */
	if (options) {
		FINFO_SET_OPTION(magic, options)
	}

	switch (mode) {
		case FILEINFO_MODE_BUFFER:
		{
			ret_val = (char *) magic_buffer(magic, buffer, buffer_len);
			break;
		}

		case FILEINFO_MODE_STREAM:
		{
				crx_stream *stream;
				crex_off_t streampos;

				crx_stream_from_zval_no_verify(stream, what);
				if (!stream) {
					goto common;
				}

				streampos = crx_stream_tell(stream); /* remember stream position for restoration */
				crx_stream_seek(stream, 0, SEEK_SET);

				ret_val = (char *) magic_stream(magic, stream);

				crx_stream_seek(stream, streampos, SEEK_SET);
				break;
		}

		case FILEINFO_MODE_FILE:
		{
			/* determine if the file is a local file or remote URL */
			const char *tmp2;
			crx_stream_wrapper *wrap;
			crx_stream_statbuf ssb;

			if (buffer == NULL || buffer_len == 0) {
				crex_argument_value_error(1, "cannot be empty");
				goto clean;
			}
			if (CHECK_NULL_PATH(buffer, buffer_len)) {
				crex_argument_type_error(1, "must not contain any null bytes");
				goto clean;
			}

			wrap = crx_stream_locate_url_wrapper(buffer, &tmp2, 0);

			if (wrap) {
				crx_stream *stream;
				crx_stream_context *context = crx_stream_context_from_zval(zcontext, 0);

#ifdef CRX_WIN32
				if (crx_stream_stat_path_ex(buffer, 0, &ssb, context) == SUCCESS) {
					if (ssb.sb.st_mode & S_IFDIR) {
						ret_val = mime_directory;
						goto common;
					}
				}
#endif

				stream = crx_stream_open_wrapper_ex(buffer, "rb", REPORT_ERRORS, NULL, context);

				if (!stream) {
					RETVAL_FALSE;
					goto clean;
				}

				if (crx_stream_stat(stream, &ssb) == SUCCESS) {
					if (ssb.sb.st_mode & S_IFDIR) {
						ret_val = mime_directory;
					} else {
						ret_val = (char *)magic_stream(magic, stream);
					}
				}

				crx_stream_close(stream);
			}
			break;
		}
		EMPTY_SWITCH_DEFAULT_CASE()
	}

common:
	if (ret_val) {
		RETVAL_STRING(ret_val);
	} else {
		crx_error_docref(NULL, E_WARNING, "Failed identify data %d:%s", magic_errno(magic), magic_error(magic));
		RETVAL_FALSE;
	}

clean:
	if (mimetype_emu) {
		magic_close(magic);
	}

	/* Restore options */
	if (options) {
		FINFO_SET_OPTION(magic, finfo->options)
	}
	return;
}
/* }}} */

/* {{{ Return information about a file. */
CRX_FUNCTION(finfo_file)
{
	_crx_finfo_get_type(INTERNAL_FUNCTION_PARAM_PASSTHRU, FILEINFO_MODE_FILE, 0);
}
/* }}} */

/* {{{ Return information about a string buffer. */
CRX_FUNCTION(finfo_buffer)
{
	_crx_finfo_get_type(INTERNAL_FUNCTION_PARAM_PASSTHRU, FILEINFO_MODE_BUFFER, 0);
}
/* }}} */

/* {{{ Return content-type for file */
CRX_FUNCTION(mime_content_type)
{
	_crx_finfo_get_type(INTERNAL_FUNCTION_PARAM_PASSTHRU, -1, 1);
}
/* }}} */
