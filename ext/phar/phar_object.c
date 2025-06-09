/*
  +----------------------------------------------------------------------+
  | crxa crx single-file executable CRX extension                        |
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
  | Authors: Gregory Beaver <cellog@crx.net>                             |
  |          Marcus Boerger <helly@crx.net>                              |
  +----------------------------------------------------------------------+
*/

#include "crxa_internal.h"
#include "func_interceptors.h"
#include "crxa_object_arginfo.h"

static crex_class_entry *crxa_ce_archive;
static crex_class_entry *crxa_ce_data;
static crex_class_entry *crxa_ce_CrxaException;
static crex_class_entry *crxa_ce_entry;

static int crxa_file_type(HashTable *mimes, char *file, char **mime_type) /* {{{ */
{
	char *ext;
	crxa_mime_type *mime;
	ext = strrchr(file, '.');
	if (!ext) {
		*mime_type = "text/plain";
		/* no file extension = assume text/plain */
		return CRXA_MIME_OTHER;
	}
	++ext;
	if (NULL == (mime = crex_hash_str_find_ptr(mimes, ext, strlen(ext)))) {
		*mime_type = "application/octet-stream";
		return CRXA_MIME_OTHER;
	}
	*mime_type = mime->mime;
	return mime->type;
}
/* }}} */

static void crxa_mung_server_vars(char *fname, char *entry, size_t entry_len, char *basename, size_t request_uri_len) /* {{{ */
{
	HashTable *_SERVER;
	zval *stuff;
	char *path_info;
	size_t basename_len = strlen(basename);
	size_t code;
	zval temp;

	/* "tweak" $_SERVER variables requested in earlier call to Crxa::mungServer() */
	if (C_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) == IS_UNDEF) {
		return;
	}

	_SERVER = C_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]);

	/* PATH_INFO and PATH_TRANSLATED should always be munged */
	if (NULL != (stuff = crex_hash_str_find(_SERVER, "PATH_INFO", sizeof("PATH_INFO")-1))) {
		path_info = C_STRVAL_P(stuff);
		code = C_STRLEN_P(stuff);
		if (code > (size_t)entry_len && !memcmp(path_info, entry, entry_len)) {
			ZVAL_STR(&temp, C_STR_P(stuff));
			ZVAL_STRINGL(stuff, path_info + entry_len, request_uri_len);
			crex_hash_str_update(_SERVER, "CRXA_PATH_INFO", sizeof("CRXA_PATH_INFO")-1, &temp);
		}
	}

	if (NULL != (stuff = crex_hash_str_find(_SERVER, "PATH_TRANSLATED", sizeof("PATH_TRANSLATED")-1))) {
		crex_string *str = strpprintf(4096, "crxa://%s%s", fname, entry);

		ZVAL_STR(&temp, C_STR_P(stuff));
		ZVAL_NEW_STR(stuff, str);

		crex_hash_str_update(_SERVER, "CRXA_PATH_TRANSLATED", sizeof("CRXA_PATH_TRANSLATED")-1, &temp);
	}

	if (!CRXA_G(crxa_SERVER_mung_list)) {
		return;
	}

	if (CRXA_G(crxa_SERVER_mung_list) & CRXA_MUNG_REQUEST_URI) {
		if (NULL != (stuff = crex_hash_str_find(_SERVER, "REQUEST_URI", sizeof("REQUEST_URI")-1))) {
			path_info = C_STRVAL_P(stuff);
			code = C_STRLEN_P(stuff);
			if (code > basename_len && !memcmp(path_info, basename, basename_len)) {
				ZVAL_STR(&temp, C_STR_P(stuff));
				ZVAL_STRINGL(stuff, path_info + basename_len, code - basename_len);
				crex_hash_str_update(_SERVER, "CRXA_REQUEST_URI", sizeof("CRXA_REQUEST_URI")-1, &temp);
			}
		}
	}

	if (CRXA_G(crxa_SERVER_mung_list) & CRXA_MUNG_CRX_SELF) {
		if (NULL != (stuff = crex_hash_str_find(_SERVER, "CRX_SELF", sizeof("CRX_SELF")-1))) {
			path_info = C_STRVAL_P(stuff);
			code = C_STRLEN_P(stuff);

			if (code > basename_len && !memcmp(path_info, basename, basename_len)) {
				ZVAL_STR(&temp, C_STR_P(stuff));
				ZVAL_STRINGL(stuff, path_info + basename_len, code - basename_len);
				crex_hash_str_update(_SERVER, "CRXA_CRX_SELF", sizeof("CRXA_CRX_SELF")-1, &temp);
			}
		}
	}

	if (CRXA_G(crxa_SERVER_mung_list) & CRXA_MUNG_SCRIPT_NAME) {
		if (NULL != (stuff = crex_hash_str_find(_SERVER, "SCRIPT_NAME", sizeof("SCRIPT_NAME")-1))) {
			ZVAL_STR(&temp, C_STR_P(stuff));
			ZVAL_STRINGL(stuff, entry, entry_len);
			crex_hash_str_update(_SERVER, "CRXA_SCRIPT_NAME", sizeof("CRXA_SCRIPT_NAME")-1, &temp);
		}
	}

	if (CRXA_G(crxa_SERVER_mung_list) & CRXA_MUNG_SCRIPT_FILENAME) {
		if (NULL != (stuff = crex_hash_str_find(_SERVER, "SCRIPT_FILENAME", sizeof("SCRIPT_FILENAME")-1))) {
			crex_string *str = strpprintf(4096, "crxa://%s%s", fname, entry);

			ZVAL_STR(&temp, C_STR_P(stuff));
			ZVAL_NEW_STR(stuff, str);

			crex_hash_str_update(_SERVER, "CRXA_SCRIPT_FILENAME", sizeof("CRXA_SCRIPT_FILENAME")-1, &temp);
		}
	}
}
/* }}} */

static int crxa_file_action(crxa_archive_data *crxa, crxa_entry_info *info, char *mime_type, int code, char *entry, size_t entry_len, char *arch, char *basename, char *ru, size_t ru_len) /* {{{ */
{
	char *name = NULL, buf[8192];
	const char *cwd;
	crex_syntax_highlighter_ini syntax_highlighter_ini;
	sapi_header_line ctr = {0};
	size_t got;
	zval dummy;
	size_t name_len;
	crex_file_handle file_handle;
	crex_op_array *new_op_array;
	zval result;
	crx_stream *fp;
	crex_off_t position;

	switch (code) {
		case CRXA_MIME_CRXS:
			efree(basename);
			/* highlight source */
			if (entry[0] == '/') {
				spprintf(&name, 4096, "crxa://%s%s", arch, entry);
			} else {
				spprintf(&name, 4096, "crxa://%s/%s", arch, entry);
			}
			crx_get_highlight_struct(&syntax_highlighter_ini);

			highlight_file(name, &syntax_highlighter_ini);

			efree(name);
#ifdef CRX_WIN32
			efree(arch);
#endif
			crex_bailout();
		case CRXA_MIME_OTHER:
			/* send headers, output file contents */
			efree(basename);
			ctr.line_len = spprintf((char **) &(ctr.line), 0, "Content-type: %s", mime_type);
			sapi_header_op(SAPI_HEADER_REPLACE, &ctr);
			efree((void *) ctr.line);
			ctr.line_len = spprintf((char **) &(ctr.line), 0, "Content-length: %u", info->uncompressed_filesize);
			sapi_header_op(SAPI_HEADER_REPLACE, &ctr);
			efree((void *) ctr.line);

			if (FAILURE == sapi_send_headers()) {
				crex_bailout();
			}

			/* prepare to output  */
			fp = crxa_get_efp(info, 1);

			if (!fp) {
				char *error;
				if (!crxa_open_jit(crxa, info, &error)) {
					if (error) {
						crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
						efree(error);
					}
					return -1;
				}
				fp = crxa_get_efp(info, 1);
			}
			position = 0;
			crxa_seek_efp(info, 0, SEEK_SET, 0, 1);

			do {
				got = crx_stream_read(fp, buf, MIN(8192, info->uncompressed_filesize - position));
				if (got > 0) {
					CRXWRITE(buf, got);
					position += got;
					if (position == (crex_off_t) info->uncompressed_filesize) {
						break;
					}
				}
			} while (1);

			crex_bailout();
		case CRXA_MIME_CRX:
			if (basename) {
				crxa_mung_server_vars(arch, entry, entry_len, basename, ru_len);
				efree(basename);
			}

			if (entry[0] == '/') {
				name_len = spprintf(&name, 4096, "crxa://%s%s", arch, entry);
			} else {
				name_len = spprintf(&name, 4096, "crxa://%s/%s", arch, entry);
			}

			crex_stream_init_filename(&file_handle, name);

			CRXA_G(cwd) = NULL;
			CRXA_G(cwd_len) = 0;

			ZVAL_NULL(&dummy);
			if (crex_hash_str_add(&EG(included_files), name, name_len, &dummy) != NULL) {
				if ((cwd = crex_memrchr(entry, '/', entry_len))) {
					CRXA_G(cwd_init) = 1;
					if (entry == cwd) {
						/* root directory */
						CRXA_G(cwd_len) = 0;
						CRXA_G(cwd) = NULL;
					} else if (entry[0] == '/') {
						CRXA_G(cwd_len) = (cwd - (entry + 1));
						CRXA_G(cwd) = estrndup(entry + 1, CRXA_G(cwd_len));
					} else {
						CRXA_G(cwd_len) = (cwd - entry);
						CRXA_G(cwd) = estrndup(entry, CRXA_G(cwd_len));
					}
				}

				new_op_array = crex_compile_file(&file_handle, CREX_REQUIRE);

				if (!new_op_array) {
					crex_hash_str_del(&EG(included_files), name, name_len);
				}
			} else {
				efree(name);
				new_op_array = NULL;
			}

			crex_destroy_file_handle(&file_handle);
#ifdef CRX_WIN32
			efree(arch);
#endif
			if (new_op_array) {
				ZVAL_UNDEF(&result);

				crex_try {
					crex_execute(new_op_array, &result);
					if (CRXA_G(cwd)) {
						efree(CRXA_G(cwd));
						CRXA_G(cwd) = NULL;
						CRXA_G(cwd_len) = 0;
					}

					CRXA_G(cwd_init) = 0;
					efree(name);
					destroy_op_array(new_op_array);
					efree(new_op_array);
					zval_ptr_dtor(&result);
				} crex_catch {
					if (CRXA_G(cwd)) {
						efree(CRXA_G(cwd));
						CRXA_G(cwd) = NULL;
						CRXA_G(cwd_len) = 0;
					}

					CRXA_G(cwd_init) = 0;
					efree(name);
				} crex_end_try();

				crex_bailout();
			}

			return CRXA_MIME_CRX;
	}
	return -1;
}
/* }}} */

static void crxa_do_403(char *entry, size_t entry_len) /* {{{ */
{
	sapi_header_line ctr = {0};

	ctr.response_code = 403;
	ctr.line_len = sizeof("HTTP/1.0 403 Access Denied")-1;
	ctr.line = "HTTP/1.0 403 Access Denied";
	sapi_header_op(SAPI_HEADER_REPLACE, &ctr);
	sapi_send_headers();
	CRXWRITE("<html>\n <head>\n  <title>Access Denied</title>\n </head>\n <body>\n  <h1>403 - File ", sizeof("<html>\n <head>\n  <title>Access Denied</title>\n </head>\n <body>\n  <h1>403 - File ") - 1);
	CRXWRITE("Access Denied</h1>\n </body>\n</html>", sizeof("Access Denied</h1>\n </body>\n</html>") - 1);
}
/* }}} */

static void crxa_do_404(crxa_archive_data *crxa, char *fname, size_t fname_len, char *f404, size_t f404_len, char *entry, size_t entry_len) /* {{{ */
{
	sapi_header_line ctr = {0};
	crxa_entry_info	*info;

	if (crxa && f404_len) {
		info = crxa_get_entry_info(crxa, f404, f404_len, NULL, 1);

		if (info) {
			crxa_file_action(crxa, info, "text/html", CRXA_MIME_CRX, f404, f404_len, fname, NULL, NULL, 0);
			return;
		}
	}

	ctr.response_code = 404;
	ctr.line_len = sizeof("HTTP/1.0 404 Not Found")-1;
	ctr.line = "HTTP/1.0 404 Not Found";
	sapi_header_op(SAPI_HEADER_REPLACE, &ctr);
	sapi_send_headers();
	CRXWRITE("<html>\n <head>\n  <title>File Not Found</title>\n </head>\n <body>\n  <h1>404 - File ", sizeof("<html>\n <head>\n  <title>File Not Found</title>\n </head>\n <body>\n  <h1>404 - File ") - 1);
	CRXWRITE("Not Found</h1>\n </body>\n</html>",  sizeof("Not Found</h1>\n </body>\n</html>") - 1);
}
/* }}} */

/* post-process REQUEST_URI and retrieve the actual request URI.  This is for
   cases like http://localhost/blah.crxa/path/to/file.crx/extra/stuff
   which calls "blah.crxa" file "path/to/file.crx" with PATH_INFO "/extra/stuff" */
static void crxa_postprocess_ru_web(char *fname, size_t fname_len, char **entry, size_t *entry_len, char **ru, size_t *ru_len) /* {{{ */
{
	char *e = *entry + 1, *u = NULL, *u1 = NULL, *saveu = NULL;
	size_t e_len = *entry_len - 1, u_len = 0;
	crxa_archive_data *pcrxa;

	/* we already know we can retrieve the crxa if we reach here */
	pcrxa = crex_hash_str_find_ptr(&(CRXA_G(crxa_fname_map)), fname, fname_len);

	if (!pcrxa && CRXA_G(manifest_cached)) {
		pcrxa = crex_hash_str_find_ptr(&cached_crxas, fname, fname_len);
	}

	do {
		if (crex_hash_str_exists(&(pcrxa->manifest), e, e_len)) {
			if (u) {
				u[0] = '/';
				*ru = estrndup(u, u_len+1);
				++u_len;
				u[0] = '\0';
			} else {
				*ru = NULL;
			}
			*ru_len = u_len;
			*entry_len = e_len + 1;
			return;
		}

		if (u) {
			u1 = strrchr(e, '/');
			u[0] = '/';
			saveu = u;
			e_len += u_len + 1;
			u = u1;
			if (!u) {
				return;
			}
		} else {
			u = strrchr(e, '/');
			if (!u) {
				if (saveu) {
					saveu[0] = '/';
				}
				return;
			}
		}

		u[0] = '\0';
		u_len = strlen(u + 1);
		e_len -= u_len + 1;
	} while (1);
}
/* }}} */

/* {{{ return the name of the currently running crxa archive.  If the optional parameter
 * is set to true, return the crxa:// URL to the currently running crxa
 */
CRX_METHOD(Crxa, running)
{
	crex_string *fname;
	char *arch, *entry;
	size_t arch_len, entry_len;
	bool retcrxa = 1;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|b", &retcrxa) == FAILURE) {
		RETURN_THROWS();
	}

	fname = crex_get_executed_filename_ex();
	if (!fname) {
		RETURN_EMPTY_STRING();
	}

	if (
		crex_string_starts_with_literal_ci(fname, "crxa://")
		&& SUCCESS == crxa_split_fname(ZSTR_VAL(fname), ZSTR_LEN(fname), &arch, &arch_len, &entry, &entry_len, 2, 0)
	) {
		efree(entry);
		if (retcrxa) {
			RETVAL_STRINGL(ZSTR_VAL(fname), arch_len + 7);
			efree(arch);
			return;
		} else {
			// TODO: avoid reallocation ???
			RETVAL_STRINGL(arch, arch_len);
			efree(arch);
			return;
		}
	}

	RETURN_EMPTY_STRING();
}
/* }}} */

/* {{{ mount an external file or path to a location within the crxa.  This maps
 * an external file or directory to a location within the crxa archive, allowing
 * reference to an external location as if it were within the crxa archive.  This
 * is useful for writable temp files like databases
 */
CRX_METHOD(Crxa, mount)
{
	char *fname, *arch = NULL, *entry = NULL, *path, *actual;
	size_t fname_len, arch_len, entry_len;
	size_t path_len, actual_len;
	crxa_archive_data *pcrxa;
#ifdef CRX_WIN32
	char *save_fname;
	ALLOCA_FLAG(fname_use_heap)
#endif

	if (crex_parse_parameters(CREX_NUM_ARGS(), "pp", &path, &path_len, &actual, &actual_len) == FAILURE) {
		RETURN_THROWS();
	}

	crex_string *crex_file_name = crex_get_executed_filename_ex();
	if (UNEXPECTED(!crex_file_name)) {
		fname = "";
		fname_len = 0;
	} else {
		fname = ZSTR_VAL(crex_file_name);
		fname_len = ZSTR_LEN(crex_file_name);
	}

#ifdef CRX_WIN32
	save_fname = fname;
	if (memchr(fname, '\\', fname_len)) {
		fname = do_alloca(fname_len + 1, fname_use_heap);
		memcpy(fname, save_fname, fname_len);
		fname[fname_len] = '\0';
		crxa_unixify_path_separators(fname, fname_len);
	}
#endif

	if (fname_len > 7 && !memcmp(fname, "crxa://", 7) && SUCCESS == crxa_split_fname(fname, fname_len, &arch, &arch_len, &entry, &entry_len, 2, 0)) {
		efree(entry);
		entry = NULL;

		if (path_len > 7 && !memcmp(path, "crxa://", 7)) {
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "Can only mount internal paths within a crxa archive, use a relative path instead of \"%s\"", path);
			efree(arch);
			goto finish;
		}
carry_on2:
		if (NULL == (pcrxa = crex_hash_str_find_ptr(&(CRXA_G(crxa_fname_map)), arch, arch_len))) {
			if (CRXA_G(manifest_cached) && NULL != (pcrxa = crex_hash_str_find_ptr(&cached_crxas, arch, arch_len))) {
				if (SUCCESS == crxa_copy_on_write(&pcrxa)) {
					goto carry_on;
				}
			}

			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s is not a crxa archive, cannot mount", arch);

			if (arch) {
				efree(arch);
			}

			goto finish;
		}
carry_on:
		if (SUCCESS != crxa_mount_entry(pcrxa, actual, actual_len, path, path_len)) {
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "Mounting of %s to %s within crxa %s failed", path, actual, arch);
		}

		if (entry && path == entry) {
			efree(entry);
		}

		if (arch) {
			efree(arch);
		}

		goto finish;
	} else if (HT_IS_INITIALIZED(&CRXA_G(crxa_fname_map)) && NULL != (pcrxa = crex_hash_str_find_ptr(&(CRXA_G(crxa_fname_map)), fname, fname_len))) {
		goto carry_on;
	} else if (CRXA_G(manifest_cached) && NULL != (pcrxa = crex_hash_str_find_ptr(&cached_crxas, fname, fname_len))) {
		if (SUCCESS == crxa_copy_on_write(&pcrxa)) {
			goto carry_on;
		}

		goto carry_on;
	} else if (SUCCESS == crxa_split_fname(path, path_len, &arch, &arch_len, &entry, &entry_len, 2, 0)) {
		path = entry;
		path_len = entry_len;
		goto carry_on2;
	}

	crex_throw_exception_ex(crxa_ce_CrxaException, 0, "Mounting of %s to %s failed", path, actual);

finish: ;
#ifdef CRX_WIN32
	if (fname != save_fname) {
		free_alloca(fname, fname_use_heap);
		fname = save_fname;
	}
#endif
}
/* }}} */

/* {{{ mapCrxa for web-based crxas. Reads the currently executed file (a crxa)
 * and registers its manifest. When executed in the CLI or CGI command-line sapi,
 * this works exactly like mapCrxa().  When executed by a web-based sapi, this
 * reads $_SERVER['REQUEST_URI'] (the actual original value) and parses out the
 * intended internal file.
 */
CRX_METHOD(Crxa, webCrxa)
{
	zval *mimeoverride = NULL;
	crex_fcall_info rewrite_fci = {0};
	crex_fcall_info_cache rewrite_fcc;
	char *alias = NULL, *error, *index_crx = NULL, *f404 = NULL, *ru = NULL;
	size_t alias_len = 0, f404_len = 0, free_pathinfo = 0;
	size_t ru_len = 0;
	char *fname, *path_info, *mime_type = NULL, *entry, *pt;
	const char *basename;
	size_t fname_len, index_crx_len = 0;
	size_t entry_len;
	int code, not_cgi;
	crxa_archive_data *crxa = NULL;
	crxa_entry_info *info = NULL;
	size_t sapi_mod_name_len = strlen(sapi_module.name);

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|s!s!s!af!", &alias, &alias_len, &index_crx, &index_crx_len, &f404, &f404_len, &mimeoverride, &rewrite_fci, &rewrite_fcc) == FAILURE) {
		RETURN_THROWS();
	}

	crxa_request_initialize();

	if (crxa_open_executed_filename(alias, alias_len, &error) != SUCCESS) {
		if (error) {
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
			efree(error);
		}
		return;
	}

	/* retrieve requested file within crxa */
	if (!(SG(request_info).request_method
	      && SG(request_info).request_uri
	      && (!strcmp(SG(request_info).request_method, "GET")
	       || !strcmp(SG(request_info).request_method, "POST")
	       || !strcmp(SG(request_info).request_method, "DELETE")
	       || !strcmp(SG(request_info).request_method, "HEAD")
	       || !strcmp(SG(request_info).request_method, "OPTIONS")
	       || !strcmp(SG(request_info).request_method, "PATCH")
	       || !strcmp(SG(request_info).request_method, "PUT")
	      )
	     )
	   ) {
		return;
	}

	crex_string *crex_file_name = crex_get_executed_filename_ex();
	if (UNEXPECTED(!crex_file_name)) {
		return;
	}

	fname = ZSTR_VAL(crex_file_name);
	fname_len = ZSTR_LEN(crex_file_name);

#ifdef CRX_WIN32
	if (memchr(fname, '\\', fname_len)) {
		fname = estrndup(fname, fname_len);
		crxa_unixify_path_separators(fname, fname_len);
	}
#endif
	basename = crex_memrchr(fname, '/', fname_len);

	if (!basename) {
		basename = fname;
	} else {
		++basename;
	}

	if ((sapi_mod_name_len == sizeof("cgi-fcgi") - 1 && !strncmp(sapi_module.name, "cgi-fcgi", sizeof("cgi-fcgi") - 1))
		|| (sapi_mod_name_len == sizeof("fpm-fcgi") - 1 && !strncmp(sapi_module.name, "fpm-fcgi", sizeof("fpm-fcgi") - 1))
		|| (sapi_mod_name_len == sizeof("cgi") - 1 && !strncmp(sapi_module.name, "cgi", sizeof("cgi") - 1))) {

		if (C_TYPE(PG(http_globals)[TRACK_VARS_SERVER]) != IS_UNDEF) {
			HashTable *_server = C_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]);
			zval *z_script_name, *z_path_info;

			if (NULL == (z_script_name = crex_hash_str_find(_server, "SCRIPT_NAME", sizeof("SCRIPT_NAME")-1)) ||
				IS_STRING != C_TYPE_P(z_script_name) ||
				!strstr(C_STRVAL_P(z_script_name), basename)) {
				goto finish;
			}

			if (NULL != (z_path_info = crex_hash_str_find(_server, "PATH_INFO", sizeof("PATH_INFO")-1)) &&
				IS_STRING == C_TYPE_P(z_path_info)) {
				entry_len = C_STRLEN_P(z_path_info);
				entry = estrndup(C_STRVAL_P(z_path_info), entry_len);
				path_info = emalloc(C_STRLEN_P(z_script_name) + entry_len + 1);
				memcpy(path_info, C_STRVAL_P(z_script_name), C_STRLEN_P(z_script_name));
				memcpy(path_info + C_STRLEN_P(z_script_name), entry, entry_len + 1);
				free_pathinfo = 1;
			} else {
				entry_len = 0;
				entry = estrndup("", 0);
				path_info = C_STRVAL_P(z_script_name);
			}

			pt = estrndup(C_STRVAL_P(z_script_name), C_STRLEN_P(z_script_name));

		} else {
			char *testit;

			testit = sapi_getenv("SCRIPT_NAME", sizeof("SCRIPT_NAME")-1);
			if (!(pt = strstr(testit, basename))) {
				efree(testit);
				goto finish;
			}

			path_info = sapi_getenv("PATH_INFO", sizeof("PATH_INFO")-1);

			if (path_info) {
				entry = path_info;
				entry_len = strlen(entry);
				spprintf(&path_info, 0, "%s%s", testit, path_info);
				free_pathinfo = 1;
			} else {
				path_info = testit;
				free_pathinfo = 1;
				entry = estrndup("", 0);
				entry_len = 0;
			}

			pt = estrndup(testit, (pt - testit) + (fname_len - (basename - fname)));
		}
		not_cgi = 0;
	} else {
		path_info = SG(request_info).request_uri;

		if (!(pt = strstr(path_info, basename))) {
			/* this can happen with rewrite rules - and we have no idea what to do then, so return */
			goto finish;
		}

		entry_len = strlen(path_info);
		entry_len -= (pt - path_info) + (fname_len - (basename - fname));
		entry = estrndup(pt + (fname_len - (basename - fname)), entry_len);

		pt = estrndup(path_info, (pt - path_info) + (fname_len - (basename - fname)));
		not_cgi = 1;
	}

	if (CREX_FCI_INITIALIZED(rewrite_fci)) {
		zval params, retval;

		ZVAL_STRINGL(&params, entry, entry_len);

		rewrite_fci.param_count = 1;
		rewrite_fci.params = &params;
		rewrite_fci.retval = &retval;

		if (FAILURE == crex_call_function(&rewrite_fci, &rewrite_fcc)) {
			if (!EG(exception)) {
				crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa error: failed to call rewrite callback");
			}
			goto cleanup_fail;
		}

		if (C_TYPE_P(rewrite_fci.retval) == IS_UNDEF || C_TYPE(retval) == IS_UNDEF) {
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa error: rewrite callback must return a string or false");
			goto cleanup_fail;
		}

		switch (C_TYPE(retval)) {
			case IS_STRING:
				efree(entry);
				entry = estrndup(C_STRVAL_P(rewrite_fci.retval), C_STRLEN_P(rewrite_fci.retval));
				entry_len = C_STRLEN_P(rewrite_fci.retval);
				break;
			case IS_TRUE:
			case IS_FALSE:
				crxa_do_403(entry, entry_len);

				if (free_pathinfo) {
					efree(path_info);
				}
				efree(pt);

				crex_bailout();
			default:
				crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa error: rewrite callback must return a string or false");

cleanup_fail:
				zval_ptr_dtor(&params);
				if (free_pathinfo) {
					efree(path_info);
				}
				efree(entry);
				efree(pt);
#ifdef CRX_WIN32
				efree(fname);
#endif
				RETURN_THROWS();
		}
	}

	if (entry_len) {
		crxa_postprocess_ru_web(fname, fname_len, &entry, &entry_len, &ru, &ru_len);
	}

	if (!entry_len || (entry_len == 1 && entry[0] == '/')) {
		efree(entry);
		/* direct request */
		if (index_crx_len) {
			entry = index_crx;
			entry_len = index_crx_len;
			if (entry[0] != '/') {
				spprintf(&entry, 0, "/%s", index_crx);
				++entry_len;
			}
		} else {
			/* assume "index.crx" is starting point */
			entry = estrndup("/index.crx", sizeof("/index.crx"));
			entry_len = sizeof("/index.crx")-1;
		}

		if (FAILURE == crxa_get_archive(&crxa, fname, fname_len, NULL, 0, NULL) ||
			(info = crxa_get_entry_info(crxa, entry, entry_len, NULL, 0)) == NULL) {
			crxa_do_404(crxa, fname, fname_len, f404, f404_len, entry, entry_len);

			if (free_pathinfo) {
				efree(path_info);
			}

			crex_bailout();
		} else {
			char *tmp = NULL, sa = '\0';
			sapi_header_line ctr = {0};
			ctr.response_code = 301;
			ctr.line_len = sizeof("HTTP/1.1 301 Moved Permanently")-1;
			ctr.line = "HTTP/1.1 301 Moved Permanently";
			sapi_header_op(SAPI_HEADER_REPLACE, &ctr);

			if (not_cgi) {
				tmp = strstr(path_info, basename) + fname_len;
				sa = *tmp;
				*tmp = '\0';
			}

			ctr.response_code = 0;

			if (path_info[strlen(path_info)-1] == '/') {
				ctr.line_len = spprintf((char **) &(ctr.line), 4096, "Location: %s%s", path_info, entry + 1);
			} else {
				ctr.line_len = spprintf((char **) &(ctr.line), 4096, "Location: %s%s", path_info, entry);
			}

			if (not_cgi) {
				*tmp = sa;
			}

			if (free_pathinfo) {
				efree(path_info);
			}

			sapi_header_op(SAPI_HEADER_REPLACE, &ctr);
			sapi_send_headers();
			efree((void *) ctr.line);
			crex_bailout();
		}
	}

	if (FAILURE == crxa_get_archive(&crxa, fname, fname_len, NULL, 0, NULL) ||
		(info = crxa_get_entry_info(crxa, entry, entry_len, NULL, 0)) == NULL) {
		crxa_do_404(crxa, fname, fname_len, f404, f404_len, entry, entry_len);
		crex_bailout();
	}

	if (mimeoverride && crex_hash_num_elements(C_ARRVAL_P(mimeoverride))) {
		const char *ext = crex_memrchr(entry, '.', entry_len);
		zval *val;

		if (ext) {
			++ext;

			if (NULL != (val = crex_hash_str_find(C_ARRVAL_P(mimeoverride), ext, strlen(ext)))) {
				switch (C_TYPE_P(val)) {
					case IS_LONG:
						if (C_LVAL_P(val) == CRXA_MIME_CRX || C_LVAL_P(val) == CRXA_MIME_CRXS) {
							mime_type = "";
							code = C_LVAL_P(val);
						} else {
							crex_throw_exception_ex(crxa_ce_CrxaException, 0, "Unknown mime type specifier used, only Crxa::CRX, Crxa::CRXS and a mime type string are allowed");
							if (free_pathinfo) {
								efree(path_info);
							}
							efree(pt);
							efree(entry);
#ifdef CRX_WIN32
							efree(fname);
#endif
							RETURN_THROWS();
						}
						break;
					case IS_STRING:
						mime_type = C_STRVAL_P(val);
						code = CRXA_MIME_OTHER;
						break;
					default:
						crex_throw_exception_ex(crxa_ce_CrxaException, 0, "Unknown mime type specifier used (not a string or int), only Crxa::CRX, Crxa::CRXS and a mime type string are allowed");
						if (free_pathinfo) {
							efree(path_info);
						}
						efree(pt);
						efree(entry);
#ifdef CRX_WIN32
						efree(fname);
#endif
						RETURN_THROWS();
				}
			}
		}
	}

	if (!mime_type) {
		code = crxa_file_type(&CRXA_G(mime_types), entry, &mime_type);
	}
	crxa_file_action(crxa, info, mime_type, code, entry, entry_len, fname, pt, ru, ru_len);

finish: ;
#ifdef CRX_WIN32
	efree(fname);
#endif
}
/* }}} */

/* {{{ Defines a list of up to 4 $_SERVER variables that should be modified for execution
 * to mask the presence of the crxa archive.  This should be used in conjunction with
 * Crxa::webCrxa(), and has no effect otherwise
 * SCRIPT_NAME, CRX_SELF, REQUEST_URI and SCRIPT_FILENAME
 */
CRX_METHOD(Crxa, mungServer)
{
	zval *mungvalues, *data;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "a", &mungvalues) == FAILURE) {
		RETURN_THROWS();
	}

	if (!crex_hash_num_elements(C_ARRVAL_P(mungvalues))) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "No values passed to Crxa::mungServer(), expecting an array of any of these strings: CRX_SELF, REQUEST_URI, SCRIPT_FILENAME, SCRIPT_NAME");
		RETURN_THROWS();
	}

	if (crex_hash_num_elements(C_ARRVAL_P(mungvalues)) > 4) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "Too many values passed to Crxa::mungServer(), expecting an array of any of these strings: CRX_SELF, REQUEST_URI, SCRIPT_FILENAME, SCRIPT_NAME");
		RETURN_THROWS();
	}

	crxa_request_initialize();

	CREX_HASH_FOREACH_VAL(C_ARRVAL_P(mungvalues), data) {

		if (C_TYPE_P(data) != IS_STRING) {
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "Non-string value passed to Crxa::mungServer(), expecting an array of any of these strings: CRX_SELF, REQUEST_URI, SCRIPT_FILENAME, SCRIPT_NAME");
			RETURN_THROWS();
		}

		if (crex_string_equals_literal(C_STR_P(data), "CRX_SELF")) {
			CRXA_G(crxa_SERVER_mung_list) |= CRXA_MUNG_CRX_SELF;
		} else if (crex_string_equals_literal(C_STR_P(data), "REQUEST_URI")) {
			CRXA_G(crxa_SERVER_mung_list) |= CRXA_MUNG_REQUEST_URI;
		} else if (crex_string_equals_literal(C_STR_P(data), "SCRIPT_NAME")) {
			CRXA_G(crxa_SERVER_mung_list) |= CRXA_MUNG_SCRIPT_NAME;
		} else if (crex_string_equals_literal(C_STR_P(data), "SCRIPT_FILENAME")) {
			CRXA_G(crxa_SERVER_mung_list) |= CRXA_MUNG_SCRIPT_FILENAME;
		}
		// TODO Warning for invalid value?
	} CREX_HASH_FOREACH_END();
}
/* }}} */

/* {{{ instructs crxa to intercept fopen, file_get_contents, opendir, and all of the stat-related functions
 * and return stat on files within the crxa for relative paths
 *
 * Once called, this cannot be reversed, and continue until the end of the request.
 *
 * This allows legacy scripts to be crxared unmodified
 */
CRX_METHOD(Crxa, interceptFileFuncs)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	crxa_intercept_functions();
}
/* }}} */

/* {{{ Return a stub that can be used to run a crxa-based archive without the crxa extension
 * indexfile is the CLI startup filename, which defaults to "index.crx", webindexfile
 * is the web startup filename, and also defaults to "index.crx"
 */
CRX_METHOD(Crxa, createDefaultStub)
{
	char *index = NULL, *webindex = NULL, *error;
	crex_string *stub;
	size_t index_len = 0, webindex_len = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|p!p!", &index, &index_len, &webindex, &webindex_len) == FAILURE) {
		RETURN_THROWS();
	}

	stub = crxa_create_default_stub(index, webindex, &error);

	if (error) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
		efree(error);
		RETURN_THROWS();
	}
	RETURN_NEW_STR(stub);
}
/* }}} */

/* {{{ Reads the currently executed file (a crxa) and registers its manifest */
CRX_METHOD(Crxa, mapCrxa)
{
	char *alias = NULL, *error;
	size_t alias_len = 0;
	crex_long dataoffset = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|s!l", &alias, &alias_len, &dataoffset) == FAILURE) {
		RETURN_THROWS();
	}

	crxa_request_initialize();

	RETVAL_BOOL(crxa_open_executed_filename(alias, alias_len, &error) == SUCCESS);

	if (error) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
		efree(error);
	}
} /* }}} */

/* {{{ Loads any crxa archive with an alias */
CRX_METHOD(Crxa, loadCrxa)
{
	char *fname, *alias = NULL, *error;
	size_t fname_len, alias_len = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "p|s!", &fname, &fname_len, &alias, &alias_len) == FAILURE) {
		RETURN_THROWS();
	}

	crxa_request_initialize();

	RETVAL_BOOL(crxa_open_from_filename(fname, fname_len, alias, alias_len, REPORT_ERRORS, NULL, &error) == SUCCESS);

	if (error) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
		efree(error);
	}
} /* }}} */

/* {{{ Returns the api version */
CRX_METHOD(Crxa, apiVersion)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	RETURN_STRINGL(CRX_CRXA_API_VERSION, sizeof(CRX_CRXA_API_VERSION)-1);
}
/* }}}*/

/* {{{ Returns whether crxa extension supports compression using zlib/bzip2 */
CRX_METHOD(Crxa, canCompress)
{
	crex_long method = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|l", &method) == FAILURE) {
		RETURN_THROWS();
	}

	crxa_request_initialize();
	switch (method) {
	case CRXA_ENT_COMPRESSED_GZ:
		if (CRXA_G(has_zlib)) {
			RETURN_TRUE;
		} else {
			RETURN_FALSE;
		}
	case CRXA_ENT_COMPRESSED_BZ2:
		if (CRXA_G(has_bz2)) {
			RETURN_TRUE;
		} else {
			RETURN_FALSE;
		}
	default:
		if (CRXA_G(has_zlib) || CRXA_G(has_bz2)) {
			RETURN_TRUE;
		} else {
			RETURN_FALSE;
		}
	}
}
/* }}} */

/* {{{ Returns whether crxa extension supports writing and creating crxas */
CRX_METHOD(Crxa, canWrite)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	RETURN_BOOL(!CRXA_G(readonly));
}
/* }}} */

/* {{{ Returns whether the given filename is a valid crxa filename */
CRX_METHOD(Crxa, isValidCrxaFilename)
{
	char *fname;
	const char *ext_str;
	size_t fname_len;
	size_t ext_len;
	int is_executable;
	bool executable = 1;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "p|b", &fname, &fname_len, &executable) == FAILURE) {
		RETURN_THROWS();
	}

	is_executable = executable;
	RETVAL_BOOL(crxa_detect_crxa_fname_ext(fname, fname_len, &ext_str, &ext_len, is_executable, 2, 1) == SUCCESS);
}
/* }}} */

/**
 * from spl_directory
 */
static void crxa_spl_foreign_dtor(spl_filesystem_object *object) /* {{{ */
{
	crxa_archive_data *crxa = (crxa_archive_data *) object->oth;

	if (!crxa->is_persistent) {
		crxa_archive_delref(crxa);
	}

	object->oth = NULL;
}
/* }}} */

/**
 * from spl_directory
 */
static void crxa_spl_foreign_clone(spl_filesystem_object *src, spl_filesystem_object *dst) /* {{{ */
{
	crxa_archive_data *crxa_data = (crxa_archive_data *) dst->oth;

	if (!crxa_data->is_persistent) {
		++(crxa_data->refcount);
	}
}
/* }}} */

static const spl_other_handler crxa_spl_foreign_handler = {
	crxa_spl_foreign_dtor,
	crxa_spl_foreign_clone
};

/* {{{ Construct a Crxa archive object
 *
 * proto CrxaData::__main(string fname [[, int flags [, string alias]], int file format = Crxa::TAR])
 * Construct a CrxaData archive object
 *
 * This function is used as the constructor for both the Crxa and CrxaData
 * classes, hence the two prototypes above.
 */
CRX_METHOD(Crxa, __main)
{
	char *fname, *alias = NULL, *error, *arch = NULL, *entry = NULL, *save_fname;
	size_t fname_len, alias_len = 0;
	size_t arch_len, entry_len;
	bool is_data;
	crex_long flags = SPL_FILE_DIR_SKIPDOTS|SPL_FILE_DIR_UNIXPATHS;
	crex_long format = 0;
	crxa_archive_object *crxa_obj;
	crxa_archive_data   *crxa_data;
	zval *zobj = CREX_THIS, arg1, arg2;

	crxa_obj = (crxa_archive_object*)((char*)C_OBJ_P(zobj) - C_OBJ_P(zobj)->handlers->offset);

	is_data = instanceof_function(C_OBJCE_P(zobj), crxa_ce_data);

	if (is_data) {
		if (crex_parse_parameters(CREX_NUM_ARGS(), "p|ls!l", &fname, &fname_len, &flags, &alias, &alias_len, &format) == FAILURE) {
			RETURN_THROWS();
		}
	} else {
		if (crex_parse_parameters(CREX_NUM_ARGS(), "p|ls!", &fname, &fname_len, &flags, &alias, &alias_len) == FAILURE) {
			RETURN_THROWS();
		}
	}

	if (crxa_obj->archive) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Cannot call constructor twice");
		RETURN_THROWS();
	}

	save_fname = fname;
	if (SUCCESS == crxa_split_fname(fname, fname_len, &arch, &arch_len, &entry, &entry_len, !is_data, 2)) {
		/* use arch (the basename for the archive) for fname instead of fname */
		/* this allows support for RecursiveDirectoryIterator of subdirectories */
#ifdef CRX_WIN32
		crxa_unixify_path_separators(arch, arch_len);
#endif
		fname = arch;
		fname_len = arch_len;
#ifdef CRX_WIN32
	} else {
		arch = estrndup(fname, fname_len);
		arch_len = fname_len;
		fname = arch;
		crxa_unixify_path_separators(arch, arch_len);
#endif
	}

	if (crxa_open_or_create_filename(fname, fname_len, alias, alias_len, is_data, REPORT_ERRORS, &crxa_data, &error) == FAILURE) {

		if (fname == arch && fname != save_fname) {
			efree(arch);
			fname = save_fname;
		}

		if (entry) {
			efree(entry);
		}

		if (error) {
			crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
				"%s", error);
			efree(error);
		} else {
			crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
				"Crxa creation or opening failed");
		}

		RETURN_THROWS();
	}

	if (is_data && crxa_data->is_tar && crxa_data->is_brandnew && format == CRXA_FORMAT_ZIP) {
		crxa_data->is_zip = 1;
		crxa_data->is_tar = 0;
	}

	if (fname == arch) {
		efree(arch);
		fname = save_fname;
	}

	if ((is_data && !crxa_data->is_data) || (!is_data && crxa_data->is_data)) {
		if (is_data) {
			crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
				"CrxaData class can only be used for non-executable tar and zip archives");
		} else {
			crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
				"Crxa class can only be used for executable tar and zip archives");
		}
		efree(entry);
		RETURN_THROWS();
	}

	is_data = crxa_data->is_data;

	if (!crxa_data->is_persistent) {
		++(crxa_data->refcount);
	}

	crxa_obj->archive = crxa_data;
	crxa_obj->spl.oth_handler = &crxa_spl_foreign_handler;

	if (entry) {
		fname_len = spprintf(&fname, 0, "crxa://%s%s", crxa_data->fname, entry);
		efree(entry);
	} else {
		fname_len = spprintf(&fname, 0, "crxa://%s", crxa_data->fname);
	}

	ZVAL_STRINGL(&arg1, fname, fname_len);
	ZVAL_LONG(&arg2, flags);

	crex_call_known_instance_method_with_2_params(spl_ce_RecursiveDirectoryIterator->constructor,
		C_OBJ_P(zobj), NULL, &arg1, &arg2);

	zval_ptr_dtor(&arg1);

	if (!crxa_data->is_persistent) {
		crxa_obj->archive->is_data = is_data;
	} else if (!EG(exception)) {
		/* register this guy so we can modify if necessary */
		crex_hash_str_add_ptr(&CRXA_G(crxa_persist_map), (const char *) crxa_obj->archive, sizeof(crxa_obj->archive), crxa_obj);
	}

	crxa_obj->spl.info_class = crxa_ce_entry;
	efree(fname);
}
/* }}} */

/* {{{ Return array of supported signature types */
CRX_METHOD(Crxa, getSupportedSignatures)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	array_init(return_value);

	add_next_index_stringl(return_value, "MD5", 3);
	add_next_index_stringl(return_value, "SHA-1", 5);
	add_next_index_stringl(return_value, "SHA-256", 7);
	add_next_index_stringl(return_value, "SHA-512", 7);
#ifdef CRXA_HAVE_OPENSSL
	add_next_index_stringl(return_value, "OpenSSL", 7);
	add_next_index_stringl(return_value, "OpenSSL_SHA256", 14);
	add_next_index_stringl(return_value, "OpenSSL_SHA512", 14);
#else
	if (crex_hash_str_exists(&module_registry, "openssl", sizeof("openssl")-1)) {
		add_next_index_stringl(return_value, "OpenSSL", 7);
		add_next_index_stringl(return_value, "OpenSSL_SHA256", 14);
		add_next_index_stringl(return_value, "OpenSSL_SHA512", 14);
	}
#endif
}
/* }}} */

/* {{{ Return array of supported comparession algorithms */
CRX_METHOD(Crxa, getSupportedCompression)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	array_init(return_value);
	crxa_request_initialize();

	if (CRXA_G(has_zlib)) {
		add_next_index_stringl(return_value, "GZ", 2);
	}

	if (CRXA_G(has_bz2)) {
		add_next_index_stringl(return_value, "BZIP2", 5);
	}
}
/* }}} */

/* {{{ Completely remove a crxa archive from memory and disk */
CRX_METHOD(Crxa, unlinkArchive)
{
	char *fname, *error, *arch, *entry;
	size_t fname_len;
	size_t arch_len, entry_len;
	crxa_archive_data *crxa;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "p", &fname, &fname_len) == FAILURE) {
		RETURN_THROWS();
	}

	if (!fname_len) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "Unknown crxa archive \"\"");
		RETURN_THROWS();
	}

	if (FAILURE == crxa_open_from_filename(fname, fname_len, NULL, 0, REPORT_ERRORS, &crxa, &error)) {
		if (error) {
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "Unknown crxa archive \"%s\": %s", fname, error);
			efree(error);
		} else {
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "Unknown crxa archive \"%s\"", fname);
		}
		RETURN_THROWS();
	}

	crex_string *crex_file_name = crex_get_executed_filename_ex();

	if (
		crex_file_name
		&& crex_string_starts_with_literal_ci(crex_file_name, "crxa://")
		&& SUCCESS == crxa_split_fname(ZSTR_VAL(crex_file_name), ZSTR_LEN(crex_file_name), &arch, &arch_len, &entry, &entry_len, 2, 0)
	) {
		if (arch_len == fname_len && !memcmp(arch, fname, arch_len)) {
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa archive \"%s\" cannot be unlinked from within itself", fname);
			efree(arch);
			efree(entry);
			RETURN_THROWS();
		}
		efree(arch);
		efree(entry);
	}

	if (crxa->is_persistent) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa archive \"%s\" is in crxa.cache_list, cannot unlinkArchive()", fname);
		RETURN_THROWS();
	}

	if (crxa->refcount) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa archive \"%s\" has open file handles or objects.  fclose() all file handles, and unset() all objects prior to calling unlinkArchive()", fname);
		RETURN_THROWS();
	}

	fname = estrndup(crxa->fname, crxa->fname_len);

	/* invalidate crxa cache */
	CRXA_G(last_crxa) = NULL;
	CRXA_G(last_crxa_name) = CRXA_G(last_alias) = NULL;

	crxa_archive_delref(crxa);
	unlink(fname);
	efree(fname);
	RETURN_TRUE;
}
/* }}} */

#define CRXA_ARCHIVE_OBJECT() \
	zval *zobj = CREX_THIS; \
	crxa_archive_object *crxa_obj = (crxa_archive_object*)((char*)C_OBJ_P(zobj) - C_OBJ_P(zobj)->handlers->offset); \
	if (!crxa_obj->archive) { \
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, \
			"Cannot call method on an uninitialized Crxa object"); \
		RETURN_THROWS(); \
	}

/* {{{ if persistent, remove from the cache */
CRX_METHOD(Crxa, __destruct)
{
	zval *zobj = CREX_THIS;
	crxa_archive_object *crxa_obj = (crxa_archive_object*)((char*)C_OBJ_P(zobj) - C_OBJ_P(zobj)->handlers->offset);

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if (crxa_obj->archive && crxa_obj->archive->is_persistent) {
		crex_hash_str_del(&CRXA_G(crxa_persist_map), (const char *) crxa_obj->archive, sizeof(crxa_obj->archive));
	}
}
/* }}} */

struct _crxa_t {
	crxa_archive_object *p;
	crex_class_entry *c;
	crex_string *base;
	zval *ret;
	crx_stream *fp;
	int count;
};

static int crxa_build(crex_object_iterator *iter, void *puser) /* {{{ */
{
	zval *value;
	bool close_fp = 1;
	struct _crxa_t *p_obj = (struct _crxa_t*) puser;
	size_t str_key_len, base_len = ZSTR_LEN(p_obj->base);
	crxa_entry_data *data;
	crx_stream *fp;
	size_t fname_len;
	size_t contents_len;
	char *fname, *error = NULL, *base = ZSTR_VAL(p_obj->base), *save = NULL, *temp = NULL;
	crex_string *opened;
	char *str_key;
	crex_class_entry *ce = p_obj->c;
	crxa_archive_object *crxa_obj = p_obj->p;
	crx_stream_statbuf ssb;
	char ch;

	value = iter->funcs->get_current_data(iter);

	if (EG(exception)) {
		return CREX_HASH_APPLY_STOP;
	}

	if (!value) {
		/* failure in get_current_data */
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Iterator %s returned no value", ZSTR_VAL(ce->name));
		return CREX_HASH_APPLY_STOP;
	}

	switch (C_TYPE_P(value)) {
		case IS_STRING:
			break;
		case IS_RESOURCE:
			crx_stream_from_zval_no_verify(fp, value);

			if (!fp) {
				crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Iterator %s returned an invalid stream handle", ZSTR_VAL(ce->name));
				return CREX_HASH_APPLY_STOP;
			}

			if (iter->funcs->get_current_key) {
				zval key;
				iter->funcs->get_current_key(iter, &key);

				if (EG(exception)) {
					return CREX_HASH_APPLY_STOP;
				}

				if (C_TYPE(key) != IS_STRING) {
					zval_ptr_dtor(&key);
					crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Iterator %s returned an invalid key (must return a string)", ZSTR_VAL(ce->name));
					return CREX_HASH_APPLY_STOP;
				}

				str_key_len = C_STRLEN(key);
				str_key = estrndup(C_STRVAL(key), str_key_len);

				save = str_key;
				zval_ptr_dtor_str(&key);
			} else {
				crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Iterator %s returned an invalid key (must return a string)", ZSTR_VAL(ce->name));
				return CREX_HASH_APPLY_STOP;
			}

			close_fp = 0;
			opened = ZSTR_INIT_LITERAL("[stream]", 0);
			goto after_open_fp;
		case IS_OBJECT:
			if (instanceof_function(C_OBJCE_P(value), spl_ce_SplFileInfo)) {
				char *test = NULL;
				spl_filesystem_object *intern = (spl_filesystem_object*)((char*)C_OBJ_P(value) - C_OBJ_P(value)->handlers->offset);

				if (!base_len) {
					crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Iterator %s returns an SplFileInfo object, so base directory must be specified", ZSTR_VAL(ce->name));
					return CREX_HASH_APPLY_STOP;
				}

				switch (intern->type) {
					case SPL_FS_DIR: {
						crex_string *test_str = spl_filesystem_object_get_path(intern);
						fname_len = spprintf(&fname, 0, "%s%c%s", ZSTR_VAL(test_str), DEFAULT_SLASH, intern->u.dir.entry.d_name);
						crex_string_release_ex(test_str, /* persistent */ false);
						if (crx_stream_stat_path(fname, &ssb) == 0 && S_ISDIR(ssb.sb.st_mode)) {
							/* ignore directories */
							efree(fname);
							return CREX_HASH_APPLY_KEEP;
						}

						test = expand_filepath(fname, NULL);
						efree(fname);

						if (test) {
							fname = test;
							fname_len = strlen(fname);
						} else {
							crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Could not resolve file path");
							return CREX_HASH_APPLY_STOP;
						}

						save = fname;
						goto crxa_spl_fileinfo;
					}
					case SPL_FS_INFO:
					case SPL_FS_FILE:
						fname = expand_filepath(ZSTR_VAL(intern->file_name), NULL);
						if (!fname) {
							crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Could not resolve file path");
							return CREX_HASH_APPLY_STOP;
						}

						fname_len = strlen(fname);
						save = fname;
						goto crxa_spl_fileinfo;
				}
			}
			CREX_FALLTHROUGH;
		default:
			crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Iterator %s returned an invalid value (must return a string)", ZSTR_VAL(ce->name));
			return CREX_HASH_APPLY_STOP;
	}

	fname = C_STRVAL_P(value);
	fname_len = C_STRLEN_P(value);

crxa_spl_fileinfo:
	if (base_len) {
		temp = expand_filepath(base, NULL);
		if (!temp) {
			crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Could not resolve file path");
			if (save) {
				efree(save);
			}
			return CREX_HASH_APPLY_STOP;
		}

		base = temp;
		base_len = strlen(base);

		if (fname_len >= base_len && strncmp(fname, base, base_len) == 0 && ((ch = fname[base_len - IS_SLASH(base[base_len - 1])]) == '\0' || IS_SLASH(ch))) {
			str_key_len = fname_len - base_len;

			if (str_key_len <= 0) {
				if (save) {
					efree(save);
					efree(temp);
				}
				return CREX_HASH_APPLY_KEEP;
			}

			str_key = fname + base_len;

			if (*str_key == '/' || *str_key == '\\') {
				str_key++;
				str_key_len--;
			}

		} else {
			crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Iterator %s returned a path \"%s\" that is not in the base directory \"%s\"", ZSTR_VAL(ce->name), fname, base);

			if (save) {
				efree(save);
				efree(temp);
			}

			return CREX_HASH_APPLY_STOP;
		}
	} else {
		if (iter->funcs->get_current_key) {
			zval key;
			iter->funcs->get_current_key(iter, &key);

			if (EG(exception)) {
				return CREX_HASH_APPLY_STOP;
			}

			if (C_TYPE(key) != IS_STRING) {
				zval_ptr_dtor(&key);
				crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Iterator %s returned an invalid key (must return a string)", ZSTR_VAL(ce->name));
				return CREX_HASH_APPLY_STOP;
			}

			str_key_len = C_STRLEN(key);
			str_key = estrndup(C_STRVAL(key), str_key_len);

			save = str_key;
			zval_ptr_dtor_str(&key);
		} else {
			crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Iterator %s returned an invalid key (must return a string)", ZSTR_VAL(ce->name));
			return CREX_HASH_APPLY_STOP;
		}
	}

	if (crx_check_open_basedir(fname)) {
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Iterator %s returned a path \"%s\" that open_basedir prevents opening", ZSTR_VAL(ce->name), fname);

		if (save) {
			efree(save);
		}

		if (temp) {
			efree(temp);
		}

		return CREX_HASH_APPLY_STOP;
	}

	/* try to open source file, then create internal crxa file and copy contents */
	fp = crx_stream_open_wrapper(fname, "rb", STREAM_MUST_SEEK|0, &opened);

	if (!fp) {
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Iterator %s returned a file that could not be opened \"%s\"", ZSTR_VAL(ce->name), fname);

		if (save) {
			efree(save);
		}

		if (temp) {
			efree(temp);
		}

		return CREX_HASH_APPLY_STOP;
	}
after_open_fp:
	if (str_key_len >= sizeof(".crxa")-1 && !memcmp(str_key, ".crxa", sizeof(".crxa")-1)) {
		/* silently skip any files that would be added to the magic .crxa directory */
		if (save) {
			efree(save);
		}

		if (temp) {
			efree(temp);
		}

		if (opened) {
			crex_string_release_ex(opened, 0);
		}

		if (close_fp) {
			crx_stream_close(fp);
		}

		return CREX_HASH_APPLY_KEEP;
	}

	if (!(data = crxa_get_or_create_entry_data(crxa_obj->archive->fname, crxa_obj->archive->fname_len, str_key, str_key_len, "w+b", 0, &error, 1))) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Entry %s cannot be created: %s", str_key, error);
		efree(error);

		if (save) {
			efree(save);
		}

		if (opened) {
			crex_string_release_ex(opened, 0);
		}

		if (temp) {
			efree(temp);
		}

		if (close_fp) {
			crx_stream_close(fp);
		}

		return CREX_HASH_APPLY_STOP;

	} else {
		if (error) {
			efree(error);
		}
		/* convert to CRXA_UFP */
		if (data->internal_file->fp_type == CRXA_MOD) {
			crx_stream_close(data->internal_file->fp);
		}

		data->internal_file->fp = NULL;
		data->internal_file->fp_type = CRXA_UFP;
		data->internal_file->offset_abs = data->internal_file->offset = crx_stream_tell(p_obj->fp);
		data->fp = NULL;
		crx_stream_copy_to_stream_ex(fp, p_obj->fp, CRX_STREAM_COPY_ALL, &contents_len);
		data->internal_file->uncompressed_filesize = data->internal_file->compressed_filesize =
			crx_stream_tell(p_obj->fp) - data->internal_file->offset;
		if (crx_stream_stat(fp, &ssb) != -1) {
			data->internal_file->flags = ssb.sb.st_mode & CRXA_ENT_PERM_MASK ;
		} else {
#ifndef _WIN32
			mode_t mask;
			mask = umask(0);
			umask(mask);
			data->internal_file->flags &= ~mask;
#endif
		}
	}

	if (close_fp) {
		crx_stream_close(fp);
	}

	add_assoc_str(p_obj->ret, str_key, opened);

	if (save) {
		efree(save);
	}

	if (temp) {
		efree(temp);
	}

	data->internal_file->compressed_filesize = data->internal_file->uncompressed_filesize = contents_len;
	crxa_entry_delref(data);

	return CREX_HASH_APPLY_KEEP;
}
/* }}} */

/* {{{ Construct a crxa archive from an existing directory, recursively.
 * Optional second parameter is a regular expression for filtering directory contents.
 *
 * Return value is an array mapping crxa index to actual files added.
 */
CRX_METHOD(Crxa, buildFromDirectory)
{
	char *error;
	bool apply_reg = 0;
	zval arg, arg2, iter, iteriter, regexiter;
	struct _crxa_t pass;
	crex_string *dir, *regex = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "P|S", &dir, &regex) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (CRXA_G(readonly) && !crxa_obj->archive->is_data) {
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
			"Cannot write to archive - write operations restricted by INI setting");
		RETURN_THROWS();
	}

	if (SUCCESS != object_init_ex(&iter, spl_ce_RecursiveDirectoryIterator)) {
		zval_ptr_dtor(&iter);
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Unable to instantiate directory iterator for %s", crxa_obj->archive->fname);
		RETURN_THROWS();
	}

	ZVAL_STR(&arg, dir);
	ZVAL_LONG(&arg2, SPL_FILE_DIR_SKIPDOTS|SPL_FILE_DIR_UNIXPATHS);

	crex_call_known_instance_method_with_2_params(spl_ce_RecursiveDirectoryIterator->constructor,
		C_OBJ(iter), NULL, &arg, &arg2);

	if (EG(exception)) {
		zval_ptr_dtor(&iter);
		RETURN_THROWS();
	}

	if (SUCCESS != object_init_ex(&iteriter, spl_ce_RecursiveIteratorIterator)) {
		zval_ptr_dtor(&iter);
		zval_ptr_dtor(&iteriter);
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Unable to instantiate directory iterator for %s", crxa_obj->archive->fname);
		RETURN_THROWS();
	}

	crex_call_known_instance_method_with_1_params(spl_ce_RecursiveIteratorIterator->constructor,
		C_OBJ(iteriter), NULL, &iter);

	if (EG(exception)) {
		zval_ptr_dtor(&iter);
		zval_ptr_dtor(&iteriter);
		RETURN_THROWS();
	}

	zval_ptr_dtor(&iter);

	if (regex && ZSTR_LEN(regex) > 0) {
		apply_reg = 1;

		if (SUCCESS != object_init_ex(&regexiter, spl_ce_RegexIterator)) {
			zval_ptr_dtor(&iteriter);
			zval_ptr_dtor(&regexiter);
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Unable to instantiate regex iterator for %s", crxa_obj->archive->fname);
			RETURN_THROWS();
		}

		ZVAL_STR(&arg2, regex);
		crex_call_known_instance_method_with_2_params(spl_ce_RegexIterator->constructor,
			C_OBJ(regexiter), NULL, &iteriter, &arg2);
	}

	array_init(return_value);

	pass.c = apply_reg ? C_OBJCE(regexiter) : C_OBJCE(iteriter);
	pass.p = crxa_obj;
	pass.base = dir;
	pass.count = 0;
	pass.ret = return_value;
	pass.fp = crx_stream_fopen_tmpfile();
	if (pass.fp == NULL) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa \"%s\" unable to create temporary file", crxa_obj->archive->fname);
		RETURN_THROWS();
	}

	if (crxa_obj->archive->is_persistent && FAILURE == crxa_copy_on_write(&(crxa_obj->archive))) {
		zval_ptr_dtor(&iteriter);
		if (apply_reg) {
			zval_ptr_dtor(&regexiter);
		}
		crx_stream_close(pass.fp);
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa \"%s\" is persistent, unable to copy on write", crxa_obj->archive->fname);
		RETURN_THROWS();
	}

	if (SUCCESS == spl_iterator_apply((apply_reg ? &regexiter : &iteriter), (spl_iterator_apply_func_t) crxa_build, (void *) &pass)) {
		zval_ptr_dtor(&iteriter);

		if (apply_reg) {
			zval_ptr_dtor(&regexiter);
		}

		crxa_obj->archive->ufp = pass.fp;
		crxa_flush(crxa_obj->archive, 0, 0, 0, &error);

		if (error) {
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
			efree(error);
		}

	} else {
		zval_ptr_dtor(&iteriter);
		if (apply_reg) {
			zval_ptr_dtor(&regexiter);
		}
		crx_stream_close(pass.fp);
	}
}
/* }}} */

/* {{{ Construct a crxa archive from an iterator.  The iterator must return a series of strings
 * that are full paths to files that should be added to the crxa.  The iterator key should
 * be the path that the file will have within the crxa archive.
 *
 * If base directory is specified, then the key will be ignored, and instead the portion of
 * the current value minus the base directory will be used
 *
 * Returned is an array mapping crxa index to actual file added
 */
CRX_METHOD(Crxa, buildFromIterator)
{
	zval *obj;
	char *error;
	crex_string *base = ZSTR_EMPTY_ALLOC();
	struct _crxa_t pass;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O|S!", &obj, crex_ce_traversable, &base) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (CRXA_G(readonly) && !crxa_obj->archive->is_data) {
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
			"Cannot write out crxa archive, crxa is read-only");
		RETURN_THROWS();
	}

	if (crxa_obj->archive->is_persistent && FAILURE == crxa_copy_on_write(&(crxa_obj->archive))) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa \"%s\" is persistent, unable to copy on write", crxa_obj->archive->fname);
		RETURN_THROWS();
	}

	array_init(return_value);

	pass.c = C_OBJCE_P(obj);
	pass.p = crxa_obj;
	pass.base = base;
	pass.ret = return_value;
	pass.count = 0;
	pass.fp = crx_stream_fopen_tmpfile();
	if (pass.fp == NULL) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa \"%s\": unable to create temporary file", crxa_obj->archive->fname);
		RETURN_THROWS();
	}

	if (SUCCESS == spl_iterator_apply(obj, (spl_iterator_apply_func_t) crxa_build, (void *) &pass)) {
		crxa_obj->archive->ufp = pass.fp;
		crxa_flush(crxa_obj->archive, 0, 0, 0, &error);
		if (error) {
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
			efree(error);
		}
	} else {
		crx_stream_close(pass.fp);
	}
}
/* }}} */

/* {{{ Returns the number of entries in the Crxa archive */
CRX_METHOD(Crxa, count)
{
	/* mode can be ignored, maximum depth is 1 */
	crex_long mode;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|l", &mode) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	RETURN_LONG(crex_hash_num_elements(&crxa_obj->archive->manifest));
}
/* }}} */

/* {{{ Returns true if the crxa archive is based on the tar/zip/crxa file format depending
 * on whether Crxa::TAR, Crxa::ZIP or Crxa::CRXA was passed in
 */
CRX_METHOD(Crxa, isFileFormat)
{
	crex_long type;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &type) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	switch (type) {
		case CRXA_FORMAT_TAR:
			RETURN_BOOL(crxa_obj->archive->is_tar);
		case CRXA_FORMAT_ZIP:
			RETURN_BOOL(crxa_obj->archive->is_zip);
		case CRXA_FORMAT_CRXA:
			RETURN_BOOL(!crxa_obj->archive->is_tar && !crxa_obj->archive->is_zip);
		default:
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "Unknown file format specified");
	}
}
/* }}} */

static int crxa_copy_file_contents(crxa_entry_info *entry, crx_stream *fp) /* {{{ */
{
	char *error;
	crex_off_t offset;
	crxa_entry_info *link;

	if (FAILURE == crxa_open_entry_fp(entry, &error, 1)) {
		if (error) {
			crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
				"Cannot convert crxa archive \"%s\", unable to open entry \"%s\" contents: %s", entry->crxa->fname, entry->filename, error);
			efree(error);
		} else {
			crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
				"Cannot convert crxa archive \"%s\", unable to open entry \"%s\" contents", entry->crxa->fname, entry->filename);
		}
		return FAILURE;
	}

	/* copy old contents in entirety */
	crxa_seek_efp(entry, 0, SEEK_SET, 0, 1);
	offset = crx_stream_tell(fp);
	link = crxa_get_link_source(entry);

	if (!link) {
		link = entry;
	}

	if (SUCCESS != crx_stream_copy_to_stream_ex(crxa_get_efp(link, 0), fp, link->uncompressed_filesize, NULL)) {
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
			"Cannot convert crxa archive \"%s\", unable to copy entry \"%s\" contents", entry->crxa->fname, entry->filename);
		return FAILURE;
	}

	if (entry->fp_type == CRXA_MOD) {
		/* save for potential restore on error */
		entry->cfp = entry->fp;
		entry->fp = NULL;
	}

	/* set new location of file contents */
	entry->fp_type = CRXA_FP;
	entry->offset = offset;
	return SUCCESS;
}
/* }}} */

static crex_object *crxa_rename_archive(crxa_archive_data **scrxa, char *ext) /* {{{ */
{
	const char *oldname = NULL;
	crxa_archive_data *crxa = *scrxa;
	char *oldpath = NULL;
	char *basename = NULL, *basepath = NULL;
	char *newname = NULL, *newpath = NULL;
	zval ret, arg1;
	crex_class_entry *ce;
	char *error = NULL;
	const char *pcr_error;
	size_t ext_len = ext ? strlen(ext) : 0;
	size_t new_len, oldname_len, crxa_ext_len;
	crxa_archive_data *pcrxa = NULL;
	crx_stream_statbuf ssb;

	int crxa_ext_list_len, i = 0;
	char *ext_pos = NULL;
	/* Array of CRXA extensions, Must be in order, starting with longest
	 * ending with the shortest. */
	static const char *const crxa_ext_list[] = {
		".crxa.tar.bz2",
		".crxa.tar.gz",
		".crxa.crx",
		".crxa.bz2",
		".crxa.zip",
		".crxa.tar",
		".crxa.gz",
		".tar.bz2",
		".tar.gz",
		".crxa",
		".tar",
		".zip"
	};

	if (!ext) {
		if (crxa->is_zip) {

			if (crxa->is_data) {
				ext = "zip";
			} else {
				ext = "crxa.zip";
			}

		} else if (crxa->is_tar) {

			switch (crxa->flags) {
				case CRXA_FILE_COMPRESSED_GZ:
					if (crxa->is_data) {
						ext = "tar.gz";
					} else {
						ext = "crxa.tar.gz";
					}
					break;
				case CRXA_FILE_COMPRESSED_BZ2:
					if (crxa->is_data) {
						ext = "tar.bz2";
					} else {
						ext = "crxa.tar.bz2";
					}
					break;
				default:
					if (crxa->is_data) {
						ext = "tar";
					} else {
						ext = "crxa.tar";
					}
			}
		} else {

			switch (crxa->flags) {
				case CRXA_FILE_COMPRESSED_GZ:
					ext = "crxa.gz";
					break;
				case CRXA_FILE_COMPRESSED_BZ2:
					ext = "crxa.bz2";
					break;
				default:
					ext = "crxa";
			}
		}
	} else if (crxa_path_check(&ext, &ext_len, &pcr_error) > pcr_is_ok) {

		if (crxa->is_data) {
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "data crxa converted from \"%s\" has invalid extension %s", crxa->fname, ext);
		} else {
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "crxa converted from \"%s\" has invalid extension %s", crxa->fname, ext);
		}
		return NULL;
	}


	oldpath = estrndup(crxa->fname, crxa->fname_len);
	if ((oldname = crex_memrchr(crxa->fname, '/', crxa->fname_len))) {
		++oldname;
	} else {
		oldname = crxa->fname;
	}

	oldname_len = strlen(oldname);
	/* Copy the old name to create base for the new name */
	basename = estrndup(oldname, oldname_len);

	crxa_ext_list_len = sizeof(crxa_ext_list)/sizeof(crxa_ext_list[0]);
	/* Remove possible CRXA extensions */
	/* crxa_ext_list must be in order of longest extension to shortest */
	for (i=0; i < crxa_ext_list_len; i++) {
		crxa_ext_len = strlen(crxa_ext_list[i]);
		if (crxa_ext_len && oldname_len > crxa_ext_len) {
			/* Check if the basename strings ends with the extension */
			if (memcmp(crxa_ext_list[i], basename + (oldname_len - crxa_ext_len), crxa_ext_len) == 0) {
				ext_pos = basename + (oldname_len - crxa_ext_len);
				ext_pos[0] = '\0';
				break;
			}
		}
		ext_pos = NULL;
	}

	/* If no default CRXA extension found remove the last extension */
	if (!ext_pos) {
		ext_pos = strrchr(basename, '.');
		if (ext_pos) {
			ext_pos[0] = '\0';
		}
	}
	ext_pos = NULL;

	if (ext[0] == '.') {
		++ext;
	}
	/* Append extension to the basename */
	spprintf(&newname, 0, "%s.%s", basename, ext);
	efree(basename);

	basepath = estrndup(oldpath, (strlen(oldpath) - oldname_len));
	new_len = spprintf(&newpath, 0, "%s%s", basepath, newname);
	crxa->fname_len = new_len;
	crxa->fname = newpath;
	crxa->ext = newpath + crxa->fname_len - strlen(ext) - 1;
	efree(basepath);
	efree(newname);

	if (CRXA_G(manifest_cached) && NULL != (pcrxa = crex_hash_str_find_ptr(&cached_crxas, newpath, crxa->fname_len))) {
		efree(oldpath);
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Unable to add newly converted crxa \"%s\" to the list of crxas, new crxa name is in crxa.cache_list", crxa->fname);
		return NULL;
	}

	if (NULL != (pcrxa = crex_hash_str_find_ptr(&(CRXA_G(crxa_fname_map)), newpath, crxa->fname_len))) {
		if (pcrxa->fname_len == crxa->fname_len && !memcmp(pcrxa->fname, crxa->fname, crxa->fname_len)) {
			if (!crex_hash_num_elements(&crxa->manifest)) {
				pcrxa->is_tar = crxa->is_tar;
				pcrxa->is_zip = crxa->is_zip;
				pcrxa->is_data = crxa->is_data;
				pcrxa->flags = crxa->flags;
				pcrxa->fp = crxa->fp;
				crxa->fp = NULL;
				/* FIX: GH-10755 Double-free issue caught by ASAN check */
				pcrxa->alias = crxa->alias; /* Transfer alias to pcrxa to */
				crxa->alias = NULL;         /* avoid being free'd twice   */
				crxa_destroy_crxa_data(crxa);
				*scrxa = NULL;
				crxa = pcrxa;
				newpath = oldpath;
				goto its_ok;
			}
		}

		efree(oldpath);
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Unable to add newly converted crxa \"%s\" to the list of crxas, a crxa with that name already exists", crxa->fname);
		return NULL;
	}
its_ok:
	if (SUCCESS == crx_stream_stat_path(newpath, &ssb)) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "crxa \"%s\" exists and must be unlinked prior to conversion", newpath);
		efree(oldpath);
		return NULL;
	}
	if (!crxa->is_data) {
		if (SUCCESS != crxa_detect_crxa_fname_ext(newpath, crxa->fname_len, (const char **) &(crxa->ext), &ext_len, 1, 1, 1)) {
			efree(oldpath);
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "crxa \"%s\" has invalid extension %s", crxa->fname, ext);
			return NULL;
		}
		crxa->ext_len = ext_len;

		if (crxa->alias) {
			if (crxa->is_temporary_alias) {
				crxa->alias = NULL;
				crxa->alias_len = 0;
			} else {
				crxa->alias = estrndup(newpath, strlen(newpath));
				crxa->alias_len = strlen(newpath);
				crxa->is_temporary_alias = 1;
				crex_hash_str_update_ptr(&(CRXA_G(crxa_alias_map)), newpath, crxa->fname_len, crxa);
			}
		}

	} else {

		if (SUCCESS != crxa_detect_crxa_fname_ext(newpath, crxa->fname_len, (const char **) &(crxa->ext), &ext_len, 0, 1, 1)) {
			efree(oldpath);
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "data crxa \"%s\" has invalid extension %s", crxa->fname, ext);
			return NULL;
		}
		crxa->ext_len = ext_len;

		crxa->alias = NULL;
		crxa->alias_len = 0;
	}

	if ((!pcrxa || crxa == pcrxa) && NULL == crex_hash_str_update_ptr(&(CRXA_G(crxa_fname_map)), newpath, crxa->fname_len, crxa)) {
		efree(oldpath);
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Unable to add newly converted crxa \"%s\" to the list of crxas", crxa->fname);
		return NULL;
	}

	crxa_flush(crxa, 0, 0, 1, &error);

	if (error) {
		crex_hash_str_del(&(CRXA_G(crxa_fname_map)), newpath, crxa->fname_len);
		*scrxa = NULL;
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "%s", error);
		efree(error);
		efree(oldpath);
		return NULL;
	}

	efree(oldpath);

	if (crxa->is_data) {
		ce = crxa_ce_data;
	} else {
		ce = crxa_ce_archive;
	}

	ZVAL_NULL(&ret);
	if (SUCCESS != object_init_ex(&ret, ce)) {
		zval_ptr_dtor(&ret);
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Unable to instantiate crxa object when converting archive \"%s\"", crxa->fname);
		return NULL;
	}

	ZVAL_STRINGL(&arg1, crxa->fname, crxa->fname_len);

	crex_call_known_instance_method_with_1_params(ce->constructor, C_OBJ(ret), NULL, &arg1);
	zval_ptr_dtor(&arg1);
	return C_OBJ(ret);
}
/* }}} */

static crex_object *crxa_convert_to_other(crxa_archive_data *source, int convert, char *ext, uint32_t flags) /* {{{ */
{
	crxa_archive_data *crxa;
	crxa_entry_info *entry, newentry;
	crex_object *ret;

	/* invalidate crxa cache */
	CRXA_G(last_crxa) = NULL;
	CRXA_G(last_crxa_name) = CRXA_G(last_alias) = NULL;

	crxa = (crxa_archive_data *) ecalloc(1, sizeof(crxa_archive_data));
	/* set whole-archive compression and type from parameter */
	crxa->flags = flags;
	crxa->is_data = source->is_data;

	switch (convert) {
		case CRXA_FORMAT_TAR:
			crxa->is_tar = 1;
			break;
		case CRXA_FORMAT_ZIP:
			crxa->is_zip = 1;
			break;
		default:
			crxa->is_data = 0;
			break;
	}

	crex_hash_init(&(crxa->manifest), sizeof(crxa_entry_info),
		crex_get_hash_value, destroy_crxa_manifest_entry, 0);
	crex_hash_init(&crxa->mounted_dirs, sizeof(char *),
		crex_get_hash_value, NULL, 0);
	crex_hash_init(&crxa->virtual_dirs, sizeof(char *),
		crex_get_hash_value, NULL, 0);

	crxa->fp = crx_stream_fopen_tmpfile();
	if (crxa->fp == NULL) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "unable to create temporary file");
		return NULL;
	}
	crxa->fname = source->fname;
	crxa->fname_len = source->fname_len;
	crxa->is_temporary_alias = source->is_temporary_alias;
	crxa->alias = source->alias;

	crxa_metadata_tracker_copy(&crxa->metadata_tracker, &source->metadata_tracker, crxa->is_persistent);

	/* first copy each file's uncompressed contents to a temporary file and set per-file flags */
	CREX_HASH_MAP_FOREACH_PTR(&source->manifest, entry) {

		newentry = *entry;

		if (newentry.link) {
			newentry.link = estrdup(newentry.link);
			goto no_copy;
		}

		if (newentry.tmp) {
			newentry.tmp = estrdup(newentry.tmp);
			goto no_copy;
		}

		if (FAILURE == crxa_copy_file_contents(&newentry, crxa->fp)) {
			crex_hash_destroy(&(crxa->manifest));
			crx_stream_close(crxa->fp);
			efree(crxa);
			/* exception already thrown */
			return NULL;
		}
no_copy:
		newentry.filename = estrndup(newentry.filename, newentry.filename_len);

		crxa_metadata_tracker_clone(&newentry.metadata_tracker);

		newentry.is_zip = crxa->is_zip;
		newentry.is_tar = crxa->is_tar;

		if (newentry.is_tar) {
			newentry.tar_type = (entry->is_dir ? TAR_DIR : TAR_FILE);
		}

		newentry.is_modified = 1;
		newentry.crxa = crxa;
		newentry.old_flags = newentry.flags & ~CRXA_ENT_COMPRESSION_MASK; /* remove compression from old_flags */
		crxa_set_inode(&newentry);
		crex_hash_str_add_mem(&(crxa->manifest), newentry.filename, newentry.filename_len, (void*)&newentry, sizeof(crxa_entry_info));
		crxa_add_virtual_dirs(crxa, newentry.filename, newentry.filename_len);
	} CREX_HASH_FOREACH_END();

	if ((ret = crxa_rename_archive(&crxa, ext))) {
		return ret;
	} else {
		if(crxa != NULL) {
			crex_hash_destroy(&(crxa->manifest));
			crex_hash_destroy(&(crxa->mounted_dirs));
			crex_hash_destroy(&(crxa->virtual_dirs));
			if (crxa->fp) {
				crx_stream_close(crxa->fp);
			}
			efree(crxa->fname);
			efree(crxa);
		}
		return NULL;
	}
}
/* }}} */

/* {{{ Convert a crxa.tar or crxa.zip archive to the crxa file format. The
 * optional parameter allows the user to determine the new
 * filename extension (default is crxa).
 */
CRX_METHOD(Crxa, convertToExecutable)
{
	char *ext = NULL;
	int is_data;
	size_t ext_len = 0;
	uint32_t flags;
	crex_object *ret;
	crex_long format, method;
	bool format_is_null = 1, method_is_null = 1;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|l!l!s!", &format, &format_is_null, &method, &method_is_null, &ext, &ext_len) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (CRXA_G(readonly)) {
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
			"Cannot write out executable crxa archive, crxa is read-only");
		RETURN_THROWS();
	}

	if (format_is_null) {
		format = CRXA_FORMAT_SAME;
	}
	switch (format) {
		case 9021976: /* Retained for BC */
		case CRXA_FORMAT_SAME:
			/* by default, use the existing format */
			if (crxa_obj->archive->is_tar) {
				format = CRXA_FORMAT_TAR;
			} else if (crxa_obj->archive->is_zip) {
				format = CRXA_FORMAT_ZIP;
			} else {
				format = CRXA_FORMAT_CRXA;
			}
			break;
		case CRXA_FORMAT_CRXA:
		case CRXA_FORMAT_TAR:
		case CRXA_FORMAT_ZIP:
			break;
		default:
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
				"Unknown file format specified, please pass one of Crxa::CRXA, Crxa::TAR or Crxa::ZIP");
			RETURN_THROWS();
	}

	if (method_is_null) {
		flags = crxa_obj->archive->flags & CRXA_FILE_COMPRESSION_MASK;
	} else {
		switch (method) {
		case 9021976: /* Retained for BC */
			flags = crxa_obj->archive->flags & CRXA_FILE_COMPRESSION_MASK;
			break;
		case 0:
			flags = CRXA_FILE_COMPRESSED_NONE;
			break;
		case CRXA_ENT_COMPRESSED_GZ:
			if (format == CRXA_FORMAT_ZIP) {
				crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
					"Cannot compress entire archive with gzip, zip archives do not support whole-archive compression");
				RETURN_THROWS();
			}

			if (!CRXA_G(has_zlib)) {
				crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
					"Cannot compress entire archive with gzip, enable ext/zlib in crx.ini");
				RETURN_THROWS();
			}

			flags = CRXA_FILE_COMPRESSED_GZ;
			break;
		case CRXA_ENT_COMPRESSED_BZ2:
			if (format == CRXA_FORMAT_ZIP) {
				crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
					"Cannot compress entire archive with bz2, zip archives do not support whole-archive compression");
				RETURN_THROWS();
			}

			if (!CRXA_G(has_bz2)) {
				crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
					"Cannot compress entire archive with bz2, enable ext/bz2 in crx.ini");
				RETURN_THROWS();
			}

			flags = CRXA_FILE_COMPRESSED_BZ2;
			break;
		default:
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
				"Unknown compression specified, please pass one of Crxa::GZ or Crxa::BZ2");
			RETURN_THROWS();
		}
	}

	is_data = crxa_obj->archive->is_data;
	crxa_obj->archive->is_data = 0;
	ret = crxa_convert_to_other(crxa_obj->archive, format, ext, flags);
	crxa_obj->archive->is_data = is_data;

	if (ret) {
		RETURN_OBJ(ret);
	} else {
		RETURN_NULL();
	}
}
/* }}} */

/* {{{ Convert an archive to a non-executable .tar or .zip.
 * The optional parameter allows the user to determine the new
 * filename extension (default is .zip or .tar).
 */
CRX_METHOD(Crxa, convertToData)
{
	char *ext = NULL;
	int is_data;
	size_t ext_len = 0;
	uint32_t flags;
	crex_object *ret;
	crex_long format, method;
	bool format_is_null = 1, method_is_null = 1;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|l!l!s!", &format, &format_is_null, &method, &method_is_null, &ext, &ext_len) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (format_is_null) {
		format = CRXA_FORMAT_SAME;
	}
	switch (format) {
		case 9021976: /* Retained for BC */
		case CRXA_FORMAT_SAME:
			/* by default, use the existing format */
			if (crxa_obj->archive->is_tar) {
				format = CRXA_FORMAT_TAR;
			} else if (crxa_obj->archive->is_zip) {
				format = CRXA_FORMAT_ZIP;
			} else {
				crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
					"Cannot write out data crxa archive, use Crxa::TAR or Crxa::ZIP");
				RETURN_THROWS();
			}
			break;
		case CRXA_FORMAT_CRXA:
			crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
				"Cannot write out data crxa archive, use Crxa::TAR or Crxa::ZIP");
			RETURN_THROWS();
		case CRXA_FORMAT_TAR:
		case CRXA_FORMAT_ZIP:
			break;
		default:
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
				"Unknown file format specified, please pass one of Crxa::TAR or Crxa::ZIP");
			RETURN_THROWS();
	}

	if (method_is_null) {
		flags = crxa_obj->archive->flags & CRXA_FILE_COMPRESSION_MASK;
	} else  {
		switch (method) {
		case 9021976: /* Retained for BC */
			flags = crxa_obj->archive->flags & CRXA_FILE_COMPRESSION_MASK;
			break;
		case 0:
			flags = CRXA_FILE_COMPRESSED_NONE;
			break;
		case CRXA_ENT_COMPRESSED_GZ:
			if (format == CRXA_FORMAT_ZIP) {
				crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
					"Cannot compress entire archive with gzip, zip archives do not support whole-archive compression");
				RETURN_THROWS();
			}

			if (!CRXA_G(has_zlib)) {
				crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
					"Cannot compress entire archive with gzip, enable ext/zlib in crx.ini");
				RETURN_THROWS();
			}

			flags = CRXA_FILE_COMPRESSED_GZ;
			break;
		case CRXA_ENT_COMPRESSED_BZ2:
			if (format == CRXA_FORMAT_ZIP) {
				crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
					"Cannot compress entire archive with bz2, zip archives do not support whole-archive compression");
				RETURN_THROWS();
			}

			if (!CRXA_G(has_bz2)) {
				crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
					"Cannot compress entire archive with bz2, enable ext/bz2 in crx.ini");
				RETURN_THROWS();
			}

			flags = CRXA_FILE_COMPRESSED_BZ2;
			break;
		default:
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
				"Unknown compression specified, please pass one of Crxa::GZ or Crxa::BZ2");
			RETURN_THROWS();
		}
	}

	is_data = crxa_obj->archive->is_data;
	crxa_obj->archive->is_data = 1;
	ret = crxa_convert_to_other(crxa_obj->archive, (int)format, ext, flags);
	crxa_obj->archive->is_data = is_data;

	if (ret) {
		RETURN_OBJ(ret);
	} else {
		RETURN_NULL();
	}
}
/* }}} */

/* {{{ Returns Crxa::GZ or CRXA::BZ2 if the entire archive is compressed
 * (.tar.gz/tar.bz2 and so on), or FALSE otherwise.
 */
CRX_METHOD(Crxa, isCompressed)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (crxa_obj->archive->flags & CRXA_FILE_COMPRESSED_GZ) {
		RETURN_LONG(CRXA_ENT_COMPRESSED_GZ);
	}

	if (crxa_obj->archive->flags & CRXA_FILE_COMPRESSED_BZ2) {
		RETURN_LONG(CRXA_ENT_COMPRESSED_BZ2);
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ Returns true if crxa.readonly=0 or crxa is a CrxaData AND the actual file is writable. */
CRX_METHOD(Crxa, isWritable)
{
	crx_stream_statbuf ssb;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (!crxa_obj->archive->is_writeable) {
		RETURN_FALSE;
	}

	if (SUCCESS != crx_stream_stat_path(crxa_obj->archive->fname, &ssb)) {
		if (crxa_obj->archive->is_brandnew) {
			/* assume it works if the file doesn't exist yet */
			RETURN_TRUE;
		}
		RETURN_FALSE;
	}

	RETURN_BOOL((ssb.sb.st_mode & (S_IWOTH | S_IWGRP | S_IWUSR)) != 0);
}
/* }}} */

/* {{{ Deletes a named file within the archive. */
CRX_METHOD(Crxa, delete)
{
	char *fname;
	size_t fname_len;
	char *error;
	crxa_entry_info *entry;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "p", &fname, &fname_len) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (CRXA_G(readonly) && !crxa_obj->archive->is_data) {
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
			"Cannot write out crxa archive, crxa is read-only");
		RETURN_THROWS();
	}

	if (crxa_obj->archive->is_persistent && FAILURE == crxa_copy_on_write(&(crxa_obj->archive))) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa \"%s\" is persistent, unable to copy on write", crxa_obj->archive->fname);
		RETURN_THROWS();
	}
	if (NULL != (entry = crex_hash_str_find_ptr(&crxa_obj->archive->manifest, fname, (uint32_t) fname_len))) {
		if (entry->is_deleted) {
			/* entry is deleted, but has not been flushed to disk yet */
			RETURN_TRUE;
		} else {
			entry->is_deleted = 1;
			entry->is_modified = 1;
			crxa_obj->archive->is_modified = 1;
		}
	} else {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Entry %s does not exist and cannot be deleted", fname);
		RETURN_THROWS();
	}

	crxa_flush(crxa_obj->archive, NULL, 0, 0, &error);
	if (error) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
		efree(error);
		RETURN_THROWS();
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Returns the alias for the Crxa or NULL. */
CRX_METHOD(Crxa, getAlias)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (crxa_obj->archive->alias && crxa_obj->archive->alias != crxa_obj->archive->fname) {
		RETURN_STRINGL(crxa_obj->archive->alias, crxa_obj->archive->alias_len);
	}
}
/* }}} */

/* {{{ Returns the real path to the crxa archive on disk */
CRX_METHOD(Crxa, getPath)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	RETURN_STRINGL(crxa_obj->archive->fname, crxa_obj->archive->fname_len);
}
/* }}} */

/* {{{ Sets the alias for a Crxa archive. The default value is the full path
 * to the archive.
 */
CRX_METHOD(Crxa, setAlias)
{
	char *alias, *error, *oldalias;
	crxa_archive_data *fd_ptr;
	size_t alias_len, oldalias_len;
	int old_temp, readd = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s", &alias, &alias_len) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (CRXA_G(readonly) && !crxa_obj->archive->is_data) {
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
			"Cannot write out crxa archive, crxa is read-only");
		RETURN_THROWS();
	}

	/* invalidate crxa cache */
	CRXA_G(last_crxa) = NULL;
	CRXA_G(last_crxa_name) = CRXA_G(last_alias) = NULL;

	if (crxa_obj->archive->is_data) {
		if (crxa_obj->archive->is_tar) {
			crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
				"A Crxa alias cannot be set in a plain tar archive");
		} else {
			crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
				"A Crxa alias cannot be set in a plain zip archive");
		}
		RETURN_THROWS();
	}

	if (alias_len == crxa_obj->archive->alias_len && memcmp(crxa_obj->archive->alias, alias, alias_len) == 0) {
		RETURN_TRUE;
	}
	if (alias_len && NULL != (fd_ptr = crex_hash_str_find_ptr(&(CRXA_G(crxa_alias_map)), alias, alias_len))) {
		spprintf(&error, 0, "alias \"%s\" is already used for archive \"%s\" and cannot be used for other archives", alias, fd_ptr->fname);
		if (SUCCESS == crxa_free_alias(fd_ptr, alias, alias_len)) {
			efree(error);
			goto valid_alias;
		}
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
		efree(error);
		RETURN_THROWS();
	}
	if (!crxa_validate_alias(alias, alias_len)) {
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
			"Invalid alias \"%s\" specified for crxa \"%s\"", alias, crxa_obj->archive->fname);
		RETURN_THROWS();
	}
valid_alias:
	if (crxa_obj->archive->is_persistent && FAILURE == crxa_copy_on_write(&(crxa_obj->archive))) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa \"%s\" is persistent, unable to copy on write", crxa_obj->archive->fname);
		RETURN_THROWS();
	}
	if (crxa_obj->archive->alias_len && NULL != (fd_ptr = crex_hash_str_find_ptr(&(CRXA_G(crxa_alias_map)), crxa_obj->archive->alias, crxa_obj->archive->alias_len))) {
		crex_hash_str_del(&(CRXA_G(crxa_alias_map)), crxa_obj->archive->alias, crxa_obj->archive->alias_len);
		readd = 1;
	}

	oldalias = crxa_obj->archive->alias;
	oldalias_len = crxa_obj->archive->alias_len;
	old_temp = crxa_obj->archive->is_temporary_alias;

	if (alias_len) {
		crxa_obj->archive->alias = estrndup(alias, alias_len);
	} else {
		crxa_obj->archive->alias = NULL;
	}

	crxa_obj->archive->alias_len = alias_len;
	crxa_obj->archive->is_temporary_alias = 0;
	crxa_flush(crxa_obj->archive, NULL, 0, 0, &error);

	if (error) {
		crxa_obj->archive->alias = oldalias;
		crxa_obj->archive->alias_len = oldalias_len;
		crxa_obj->archive->is_temporary_alias = old_temp;
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
		if (readd) {
			crex_hash_str_add_ptr(&(CRXA_G(crxa_alias_map)), oldalias, oldalias_len, crxa_obj->archive);
		}
		efree(error);
		RETURN_THROWS();
	}

	crex_hash_str_add_ptr(&(CRXA_G(crxa_alias_map)), alias, alias_len, crxa_obj->archive);

	if (oldalias) {
		efree(oldalias);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Return version info of Crxa archive */
CRX_METHOD(Crxa, getVersion)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	RETURN_STRING(crxa_obj->archive->version);
}
/* }}} */

/* {{{ Do not flush a writeable crxa (save its contents) until explicitly requested */
CRX_METHOD(Crxa, startBuffering)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	crxa_obj->archive->donotflush = 1;
}
/* }}} */

/* {{{ Returns whether write operations are flushing to disk immediately. */
CRX_METHOD(Crxa, isBuffering)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	RETURN_BOOL(crxa_obj->archive->donotflush);
}
/* }}} */

/* {{{ Saves the contents of a modified archive to disk. */
CRX_METHOD(Crxa, stopBuffering)
{
	char *error;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (CRXA_G(readonly) && !crxa_obj->archive->is_data) {
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
			"Cannot write out crxa archive, crxa is read-only");
		RETURN_THROWS();
	}

	crxa_obj->archive->donotflush = 0;
	crxa_flush(crxa_obj->archive, 0, 0, 0, &error);

	if (error) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
		efree(error);
	}
}
/* }}} */

/* {{{ Change the stub in a crxa, crxa.tar or crxa.zip archive to something other
 * than the default. The stub *must* end with a call to __HALT_COMPILER().
 */
CRX_METHOD(Crxa, setStub)
{
	zval *zstub;
	char *stub, *error;
	size_t stub_len;
	crex_long len = -1;
	crx_stream *stream;

	CRXA_ARCHIVE_OBJECT();

	if (CRXA_G(readonly) && !crxa_obj->archive->is_data) {
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
			"Cannot change stub, crxa is read-only");
		RETURN_THROWS();
	}

	if (crxa_obj->archive->is_data) {
		if (crxa_obj->archive->is_tar) {
			crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
				"A Crxa stub cannot be set in a plain tar archive");
		} else {
			crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
				"A Crxa stub cannot be set in a plain zip archive");
		}
		RETURN_THROWS();
	}

	if (crex_parse_parameters_ex(CREX_PARSE_PARAMS_QUIET, CREX_NUM_ARGS(), "r|l", &zstub, &len) == SUCCESS) {
		crex_string *method_name = get_active_function_or_method_name();
		crex_error(E_DEPRECATED, "Calling %s(resource $stub, int $length) is deprecated", ZSTR_VAL(method_name));
		crex_string_release(method_name);
		if (UNEXPECTED(EG(exception))) {
			RETURN_THROWS();
		}

		if ((crx_stream_from_zval_no_verify(stream, zstub)) != NULL) {
			if (len > 0) {
				len = -len;
			} else {
				len = -1;
			}
			if (crxa_obj->archive->is_persistent && FAILURE == crxa_copy_on_write(&(crxa_obj->archive))) {
				crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa \"%s\" is persistent, unable to copy on write", crxa_obj->archive->fname);
				RETURN_THROWS();
			}
			crxa_flush(crxa_obj->archive, (char *) zstub, len, 0, &error);
			if (error) {
				crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
				efree(error);
			}
			RETURN_TRUE;
		} else {
			crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
				"Cannot change stub, unable to read from input stream");
		}
	} else if (crex_parse_parameters(CREX_NUM_ARGS(), "s", &stub, &stub_len) == SUCCESS) {
		if (crxa_obj->archive->is_persistent && FAILURE == crxa_copy_on_write(&(crxa_obj->archive))) {
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa \"%s\" is persistent, unable to copy on write", crxa_obj->archive->fname);
			RETURN_THROWS();
		}
		crxa_flush(crxa_obj->archive, stub, stub_len, 0, &error);

		if (error) {
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
			efree(error);
			RETURN_THROWS();
		}

		RETURN_TRUE;
	}

	RETURN_THROWS();
}
/* }}} */

/* {{{ In a pure crxa archive, sets a stub that can be used to run the archive
 * regardless of whether the crxa extension is available. The first parameter
 * is the CLI startup filename, which defaults to "index.crx". The second
 * parameter is the web startup filename and also defaults to "index.crx"
 * (falling back to CLI behaviour).
 * Both parameters are optional.
 * In a crxa.zip or crxa.tar archive, the default stub is used only to
 * identify the archive to the extension as a Crxa object. This allows the
 * extension to treat crxa.zip and crxa.tar types as honorary crxas. Since
 * files cannot be loaded via this kind of stub, no parameters are accepted
 * when the Crxa object is zip- or tar-based.
 */
CRX_METHOD(Crxa, setDefaultStub)
{
	char *index = NULL, *webindex = NULL, *error = NULL;
	crex_string *stub = NULL;
	size_t index_len = 0, webindex_len = 0;
	int created_stub = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|s!s!", &index, &index_len, &webindex, &webindex_len) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (crxa_obj->archive->is_data) {
		if (crxa_obj->archive->is_tar) {
			crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
				"A Crxa stub cannot be set in a plain tar archive");
		} else {
			crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
				"A Crxa stub cannot be set in a plain zip archive");
		}
		RETURN_THROWS();
	}

	if ((index || webindex) && (crxa_obj->archive->is_tar || crxa_obj->archive->is_zip)) {
		crex_argument_value_error(index ? 1 : 2, "must be null for a tar- or zip-based crxa stub, string given");
		RETURN_THROWS();
	}

	if (CRXA_G(readonly)) {
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
			"Cannot change stub: crxa.readonly=1");
		RETURN_THROWS();
	}

	if (!crxa_obj->archive->is_tar && !crxa_obj->archive->is_zip) {
		stub = crxa_create_default_stub(index, webindex, &error);

		if (error) {
			crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "%s", error);
			efree(error);
			if (stub) {
				crex_string_free(stub);
			}
			RETURN_THROWS();
		}

		created_stub = 1;
	}

	if (crxa_obj->archive->is_persistent && FAILURE == crxa_copy_on_write(&(crxa_obj->archive))) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa \"%s\" is persistent, unable to copy on write", crxa_obj->archive->fname);
		RETURN_THROWS();
	}
	crxa_flush(crxa_obj->archive, stub ? ZSTR_VAL(stub) : 0, stub ? ZSTR_LEN(stub) : 0, 1, &error);

	if (created_stub) {
		crex_string_free(stub);
	}

	if (error) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
		efree(error);
		RETURN_THROWS();
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Sets the signature algorithm for a crxa and applies it. The signature
 * algorithm must be one of Crxa::MD5, Crxa::SHA1, Crxa::SHA256,
 * Crxa::SHA512, or Crxa::OPENSSL. Note that zip- based crxa archives
 * cannot support signatures.
 */
CRX_METHOD(Crxa, setSignatureAlgorithm)
{
	crex_long algo;
	char *error, *key = NULL;
	size_t key_len = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l|s!", &algo, &key, &key_len) != SUCCESS) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (CRXA_G(readonly) && !crxa_obj->archive->is_data) {
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
			"Cannot set signature algorithm, crxa is read-only");
		RETURN_THROWS();
	}

	switch (algo) {
		case CRXA_SIG_SHA256:
		case CRXA_SIG_SHA512:
		case CRXA_SIG_MD5:
		case CRXA_SIG_SHA1:
		case CRXA_SIG_OPENSSL:
		case CRXA_SIG_OPENSSL_SHA256:
		case CRXA_SIG_OPENSSL_SHA512:
			if (crxa_obj->archive->is_persistent && FAILURE == crxa_copy_on_write(&(crxa_obj->archive))) {
				crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa \"%s\" is persistent, unable to copy on write", crxa_obj->archive->fname);
				RETURN_THROWS();
			}
			crxa_obj->archive->sig_flags = (crx_uint32)algo;
			crxa_obj->archive->is_modified = 1;
			CRXA_G(openssl_privatekey) = key;
			CRXA_G(openssl_privatekey_len) = key_len;

			crxa_flush(crxa_obj->archive, 0, 0, 0, &error);
			if (error) {
				crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
				efree(error);
			}
			break;
		default:
			crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
				"Unknown signature algorithm specified");
	}
}
/* }}} */

/* {{{ Returns a hash signature, or FALSE if the archive is unsigned. */
CRX_METHOD(Crxa, getSignature)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (crxa_obj->archive->signature) {
		crex_string *unknown;

		array_init(return_value);
		add_assoc_stringl(return_value, "hash", crxa_obj->archive->signature, crxa_obj->archive->sig_len);
		switch(crxa_obj->archive->sig_flags) {
			case CRXA_SIG_MD5:
				add_assoc_string(return_value, "hash_type", "MD5");
				break;
			case CRXA_SIG_SHA1:
				add_assoc_string(return_value, "hash_type", "SHA-1");
				break;
			case CRXA_SIG_SHA256:
				add_assoc_string(return_value, "hash_type", "SHA-256");
				break;
			case CRXA_SIG_SHA512:
				add_assoc_string(return_value, "hash_type", "SHA-512");
				break;
			case CRXA_SIG_OPENSSL:
				add_assoc_string(return_value, "hash_type", "OpenSSL");
				break;
			case CRXA_SIG_OPENSSL_SHA256:
				add_assoc_string(return_value, "hash_type", "OpenSSL_SHA256");
				break;
			case CRXA_SIG_OPENSSL_SHA512:
				add_assoc_string(return_value, "hash_type", "OpenSSL_SHA512");
				break;
			default:
				unknown = strpprintf(0, "Unknown (%u)", crxa_obj->archive->sig_flags);
				add_assoc_str(return_value, "hash_type", unknown);
				break;
		}
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Return whether crxa was modified */
CRX_METHOD(Crxa, getModified)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	RETURN_BOOL(crxa_obj->archive->is_modified);
}
/* }}} */

static int crxa_set_compression(zval *zv, void *argument) /* {{{ */
{
	crxa_entry_info *entry = (crxa_entry_info *)C_PTR_P(zv);
	uint32_t compress = *(uint32_t *)argument;

	if (entry->is_deleted) {
		return CREX_HASH_APPLY_KEEP;
	}

	entry->old_flags = entry->flags;
	entry->flags &= ~CRXA_ENT_COMPRESSION_MASK;
	entry->flags |= compress;
	entry->is_modified = 1;
	return CREX_HASH_APPLY_KEEP;
}
/* }}} */

static int crxa_test_compression(zval *zv, void *argument) /* {{{ */
{
	crxa_entry_info *entry = (crxa_entry_info *)C_PTR_P(zv);

	if (entry->is_deleted) {
		return CREX_HASH_APPLY_KEEP;
	}

	if (!CRXA_G(has_bz2)) {
		if (entry->flags & CRXA_ENT_COMPRESSED_BZ2) {
			*(int *) argument = 0;
		}
	}

	if (!CRXA_G(has_zlib)) {
		if (entry->flags & CRXA_ENT_COMPRESSED_GZ) {
			*(int *) argument = 0;
		}
	}

	return CREX_HASH_APPLY_KEEP;
}
/* }}} */

static void crxaobj_set_compression(HashTable *manifest, uint32_t compress) /* {{{ */
{
	crex_hash_apply_with_argument(manifest, crxa_set_compression, &compress);
}
/* }}} */

static int crxaobj_cancompress(HashTable *manifest) /* {{{ */
{
	int test;

	test = 1;
	crex_hash_apply_with_argument(manifest, crxa_test_compression, &test);
	return test;
}
/* }}} */

/* {{{ Compress a .tar, or .crxa.tar with whole-file compression
 * The parameter can be one of Crxa::GZ or Crxa::BZ2 to specify
 * the kind of compression desired
 */
CRX_METHOD(Crxa, compress)
{
	crex_long method;
	char *ext = NULL;
	size_t ext_len = 0;
	uint32_t flags;
	crex_object *ret;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l|s!", &method, &ext, &ext_len) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (CRXA_G(readonly) && !crxa_obj->archive->is_data) {
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
			"Cannot compress crxa archive, crxa is read-only");
		RETURN_THROWS();
	}

	if (crxa_obj->archive->is_zip) {
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
			"Cannot compress zip-based archives with whole-archive compression");
		RETURN_THROWS();
	}

	switch (method) {
		case 0:
			flags = CRXA_FILE_COMPRESSED_NONE;
			break;
		case CRXA_ENT_COMPRESSED_GZ:
			if (!CRXA_G(has_zlib)) {
				crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
					"Cannot compress entire archive with gzip, enable ext/zlib in crx.ini");
				RETURN_THROWS();
			}
			flags = CRXA_FILE_COMPRESSED_GZ;
			break;

		case CRXA_ENT_COMPRESSED_BZ2:
			if (!CRXA_G(has_bz2)) {
				crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
					"Cannot compress entire archive with bz2, enable ext/bz2 in crx.ini");
				return;
			}
			flags = CRXA_FILE_COMPRESSED_BZ2;
			break;
		default:
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
				"Unknown compression specified, please pass one of Crxa::GZ or Crxa::BZ2");
			RETURN_THROWS();
	}

	if (crxa_obj->archive->is_tar) {
		ret = crxa_convert_to_other(crxa_obj->archive, CRXA_FORMAT_TAR, ext, flags);
	} else {
		ret = crxa_convert_to_other(crxa_obj->archive, CRXA_FORMAT_CRXA, ext, flags);
	}

	if (ret) {
		RETURN_OBJ(ret);
	} else {
		RETURN_NULL();
	}
}
/* }}} */

/* {{{ Decompress a .tar, or .crxa.tar with whole-file compression */
CRX_METHOD(Crxa, decompress)
{
	char *ext = NULL;
	size_t ext_len = 0;
	crex_object *ret;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|s!", &ext, &ext_len) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (CRXA_G(readonly) && !crxa_obj->archive->is_data) {
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
			"Cannot decompress crxa archive, crxa is read-only");
		RETURN_THROWS();
	}

	if (crxa_obj->archive->is_zip) {
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
			"Cannot decompress zip-based archives with whole-archive compression");
		RETURN_THROWS();
	}

	if (crxa_obj->archive->is_tar) {
		ret = crxa_convert_to_other(crxa_obj->archive, CRXA_FORMAT_TAR, ext, CRXA_FILE_COMPRESSED_NONE);
	} else {
		ret = crxa_convert_to_other(crxa_obj->archive, CRXA_FORMAT_CRXA, ext, CRXA_FILE_COMPRESSED_NONE);
	}

	if (ret) {
		RETURN_OBJ(ret);
	} else {
		RETURN_NULL();
	}
}
/* }}} */

/* {{{ Compress all files within a crxa or zip archive using the specified compression
 * The parameter can be one of Crxa::GZ or Crxa::BZ2 to specify
 * the kind of compression desired
 */
CRX_METHOD(Crxa, compressFiles)
{
	char *error;
	uint32_t flags;
	crex_long method;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &method) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (CRXA_G(readonly) && !crxa_obj->archive->is_data) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
			"Crxa is readonly, cannot change compression");
		RETURN_THROWS();
	}

	switch (method) {
		case CRXA_ENT_COMPRESSED_GZ:
			if (!CRXA_G(has_zlib)) {
				crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
					"Cannot compress files within archive with gzip, enable ext/zlib in crx.ini");
				RETURN_THROWS();
			}
			flags = CRXA_ENT_COMPRESSED_GZ;
			break;

		case CRXA_ENT_COMPRESSED_BZ2:
			if (!CRXA_G(has_bz2)) {
				crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
					"Cannot compress files within archive with bz2, enable ext/bz2 in crx.ini");
				RETURN_THROWS();
			}
			flags = CRXA_ENT_COMPRESSED_BZ2;
			break;
		default:
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
				"Unknown compression specified, please pass one of Crxa::GZ or Crxa::BZ2");
			RETURN_THROWS();
	}

	if (crxa_obj->archive->is_tar) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
			"Cannot compress with Gzip compression, tar archives cannot compress individual files, use compress() to compress the whole archive");
		RETURN_THROWS();
	}

	if (!crxaobj_cancompress(&crxa_obj->archive->manifest)) {
		if (flags == CRXA_ENT_COMPRESSED_GZ) {
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
				"Cannot compress all files as Gzip, some are compressed as bzip2 and cannot be decompressed");
		} else {
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
				"Cannot compress all files as Bzip2, some are compressed as gzip and cannot be decompressed");
		}
		RETURN_THROWS();
	}

	if (crxa_obj->archive->is_persistent && FAILURE == crxa_copy_on_write(&(crxa_obj->archive))) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa \"%s\" is persistent, unable to copy on write", crxa_obj->archive->fname);
		RETURN_THROWS();
	}
	crxaobj_set_compression(&crxa_obj->archive->manifest, flags);
	crxa_obj->archive->is_modified = 1;
	crxa_flush(crxa_obj->archive, 0, 0, 0, &error);

	if (error) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "%s", error);
		efree(error);
	}
}
/* }}} */

/* {{{ decompress every file */
CRX_METHOD(Crxa, decompressFiles)
{
	char *error;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();

	}

	CRXA_ARCHIVE_OBJECT();

	if (CRXA_G(readonly) && !crxa_obj->archive->is_data) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
			"Crxa is readonly, cannot change compression");
		RETURN_THROWS();
	}

	if (!crxaobj_cancompress(&crxa_obj->archive->manifest)) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
			"Cannot decompress all files, some are compressed as bzip2 or gzip and cannot be decompressed");
		RETURN_THROWS();
	}

	if (crxa_obj->archive->is_tar) {
		RETURN_TRUE;
	} else {
		if (crxa_obj->archive->is_persistent && FAILURE == crxa_copy_on_write(&(crxa_obj->archive))) {
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa \"%s\" is persistent, unable to copy on write", crxa_obj->archive->fname);
			RETURN_THROWS();
		}
		crxaobj_set_compression(&crxa_obj->archive->manifest, CRXA_ENT_COMPRESSED_NONE);
	}

	crxa_obj->archive->is_modified = 1;
	crxa_flush(crxa_obj->archive, 0, 0, 0, &error);

	if (error) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "%s", error);
		efree(error);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ copy a file internal to the crxa archive to another new file within the crxa */
CRX_METHOD(Crxa, copy)
{
	char *oldfile, *newfile, *error;
	const char *pcr_error;
	size_t oldfile_len, newfile_len;
	crxa_entry_info *oldentry, newentry = {0}, *temp;
	size_t tmp_len = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "pp", &oldfile, &oldfile_len, &newfile, &newfile_len) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (CRXA_G(readonly) && !crxa_obj->archive->is_data) {
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
			"Cannot copy \"%s\" to \"%s\", crxa is read-only", oldfile, newfile);
		RETURN_THROWS();
	}

	if (oldfile_len >= sizeof(".crxa")-1 && !memcmp(oldfile, ".crxa", sizeof(".crxa")-1)) {
		/* can't copy a meta file */
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
			"file \"%s\" cannot be copied to file \"%s\", cannot copy Crxa meta-file in %s", oldfile, newfile, crxa_obj->archive->fname);
		RETURN_THROWS();
	}

	if (newfile_len >= sizeof(".crxa")-1 && !memcmp(newfile, ".crxa", sizeof(".crxa")-1)) {
		/* can't copy a meta file */
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
			"file \"%s\" cannot be copied to file \"%s\", cannot copy to Crxa meta-file in %s", oldfile, newfile, crxa_obj->archive->fname);
		RETURN_THROWS();
	}

	if (NULL == (oldentry = crex_hash_str_find_ptr(&crxa_obj->archive->manifest, oldfile, (uint32_t) oldfile_len)) || oldentry->is_deleted) {
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
			"file \"%s\" cannot be copied to file \"%s\", file does not exist in %s", oldfile, newfile, crxa_obj->archive->fname);
		RETURN_THROWS();
	}

	if (NULL != (temp = crex_hash_str_find_ptr(&crxa_obj->archive->manifest, newfile, (uint32_t) newfile_len)) && !temp->is_deleted) {
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
			"file \"%s\" cannot be copied to file \"%s\", file must not already exist in crxa %s", oldfile, newfile, crxa_obj->archive->fname);
		RETURN_THROWS();
	}

	tmp_len = newfile_len;
	if (crxa_path_check(&newfile, &tmp_len, &pcr_error) > pcr_is_ok) {
		crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0,
				"file \"%s\" contains invalid characters %s, cannot be copied from \"%s\" in crxa %s", newfile, pcr_error, oldfile, crxa_obj->archive->fname);
		RETURN_THROWS();
	}
	newfile_len = tmp_len;

	if (crxa_obj->archive->is_persistent) {
		if (FAILURE == crxa_copy_on_write(&(crxa_obj->archive))) {
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa \"%s\" is persistent, unable to copy on write", crxa_obj->archive->fname);
			RETURN_THROWS();
		}
		/* re-populate with copied-on-write entry */
		oldentry = crex_hash_str_find_ptr(&crxa_obj->archive->manifest, oldfile, (uint32_t) oldfile_len);
	}

	memcpy((void *) &newentry, oldentry, sizeof(crxa_entry_info));

	crxa_metadata_tracker_clone(&newentry.metadata_tracker);

	newentry.filename = estrndup(newfile, newfile_len);
	newentry.filename_len = newfile_len;
	newentry.fp_refcount = 0;

	if (oldentry->fp_type != CRXA_FP) {
		if (FAILURE == crxa_copy_entry_fp(oldentry, &newentry, &error)) {
			efree(newentry.filename);
			crx_stream_close(newentry.fp);
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
			efree(error);
			RETURN_THROWS();
		}
	}

	crex_hash_str_add_mem(&oldentry->crxa->manifest, newfile, newfile_len, &newentry, sizeof(crxa_entry_info));
	crxa_obj->archive->is_modified = 1;
	crxa_flush(crxa_obj->archive, 0, 0, 0, &error);

	if (error) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
		efree(error);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ determines whether a file exists in the crxa */
CRX_METHOD(Crxa, offsetExists)
{
	char *fname;
	size_t fname_len;
	crxa_entry_info *entry;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "p", &fname, &fname_len) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (crex_hash_str_exists(&crxa_obj->archive->manifest, fname, (uint32_t) fname_len)) {
		if (NULL != (entry = crex_hash_str_find_ptr(&crxa_obj->archive->manifest, fname, (uint32_t) fname_len))) {
			if (entry->is_deleted) {
				/* entry is deleted, but has not been flushed to disk yet */
				RETURN_FALSE;
			}
		}

		if (fname_len >= sizeof(".crxa")-1 && !memcmp(fname, ".crxa", sizeof(".crxa")-1)) {
			/* none of these are real files, so they don't exist */
			RETURN_FALSE;
		}
		RETURN_TRUE;
	} else {
		if (crex_hash_str_exists(&crxa_obj->archive->virtual_dirs, fname, (uint32_t) fname_len)) {
			RETURN_TRUE;
		}
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ get a CrxaFileInfo object for a specific file */
CRX_METHOD(Crxa, offsetGet)
{
	char *fname, *error;
	size_t fname_len;
	zval zfname;
	crxa_entry_info *entry;
	crex_string *sfname;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "p", &fname, &fname_len) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	/* security is 0 here so that we can get a better error message than "entry doesn't exist" */
	if (!(entry = crxa_get_entry_info_dir(crxa_obj->archive, fname, fname_len, 1, &error, 0))) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Entry %s does not exist%s%s", fname, error?", ":"", error?error:"");
	} else {
		if (fname_len == sizeof(".crxa/stub.crx")-1 && !memcmp(fname, ".crxa/stub.crx", sizeof(".crxa/stub.crx")-1)) {
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Cannot get stub \".crxa/stub.crx\" directly in crxa \"%s\", use getStub", crxa_obj->archive->fname);
			RETURN_THROWS();
		}

		if (fname_len == sizeof(".crxa/alias.txt")-1 && !memcmp(fname, ".crxa/alias.txt", sizeof(".crxa/alias.txt")-1)) {
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Cannot get alias \".crxa/alias.txt\" directly in crxa \"%s\", use getAlias", crxa_obj->archive->fname);
			RETURN_THROWS();
		}

		if (fname_len >= sizeof(".crxa")-1 && !memcmp(fname, ".crxa", sizeof(".crxa")-1)) {
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Cannot directly get any files or directories in magic \".crxa\" directory");
			RETURN_THROWS();
		}

		if (entry->is_temp_dir) {
			efree(entry->filename);
			efree(entry);
		}

		sfname = strpprintf(0, "crxa://%s/%s", crxa_obj->archive->fname, fname);
		ZVAL_NEW_STR(&zfname, sfname);
		spl_instantiate_arg_ex1(crxa_obj->spl.info_class, return_value, &zfname);
		zval_ptr_dtor(&zfname);
	}
}
/* }}} */

/* {{{ add a file within the crxa archive from a string or resource */
static void crxa_add_file(crxa_archive_data **pcrxa, char *filename, size_t filename_len, char *cont_str, size_t cont_len, zval *zresource)
{
	size_t start_pos=0;
	char *error;
	size_t contents_len;
	crxa_entry_data *data;
	crx_stream *contents_file = NULL;
	crx_stream_statbuf ssb;
#ifdef CRX_WIN32
	char *save_filename;
	ALLOCA_FLAG(filename_use_heap)
#endif

	if (filename_len >= sizeof(".crxa")-1) {
		start_pos = '/' == filename[0]; /* account for any leading slash: multiple-leads handled elsewhere */
		if (!memcmp(&filename[start_pos], ".crxa", sizeof(".crxa")-1) && (filename[start_pos+5] == '/' || filename[start_pos+5] == '\\' || filename[start_pos+5] == '\0')) {
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Cannot create any files in magic \".crxa\" directory");
			return;
		}
	}

#ifdef CRX_WIN32
	save_filename = filename;
	if (memchr(filename, '\\', filename_len)) {
		filename = do_alloca(filename_len + 1, filename_use_heap);
		memcpy(filename, save_filename, filename_len);
		filename[filename_len] = '\0';
	}
#endif

	if (!(data = crxa_get_or_create_entry_data((*pcrxa)->fname, (*pcrxa)->fname_len, filename, filename_len, "w+b", 0, &error, 1))) {
		if (error) {
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Entry %s does not exist and cannot be created: %s", filename, error);
			efree(error);
		} else {
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Entry %s does not exist and cannot be created", filename);
		}
		goto finish;
	} else {
		if (error) {
			efree(error);
		}

		if (!data->internal_file->is_dir) {
			if (cont_str) {
				contents_len = crx_stream_write(data->fp, cont_str, cont_len);
				if (contents_len != cont_len) {
					crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Entry %s could not be written to", filename);
					goto finish;
				}
			} else {
				if (!(crx_stream_from_zval_no_verify(contents_file, zresource))) {
					crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Entry %s could not be written to", filename);
					goto finish;
				}
				crx_stream_copy_to_stream_ex(contents_file, data->fp, CRX_STREAM_COPY_ALL, &contents_len);
			}
			data->internal_file->compressed_filesize = data->internal_file->uncompressed_filesize = contents_len;
		}

		if (contents_file != NULL && crx_stream_stat(contents_file, &ssb) != -1) {
			data->internal_file->flags = ssb.sb.st_mode & CRXA_ENT_PERM_MASK ;
		} else {
#ifndef _WIN32
			mode_t mask;
			mask = umask(0);
			umask(mask);
			data->internal_file->flags &= ~mask;
#endif
		}

		/* check for copy-on-write */
		if (pcrxa[0] != data->crxa) {
			*pcrxa = data->crxa;
		}
		crxa_entry_delref(data);
		crxa_flush(*pcrxa, 0, 0, 0, &error);

		if (error) {
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
			efree(error);
		}
	}

finish: ;
#ifdef CRX_WIN32
	if (filename != save_filename) {
		free_alloca(filename, filename_use_heap);
		filename = save_filename;
	}
#endif
}
/* }}} */

/* {{{ create a directory within the crxa archive */
static void crxa_mkdir(crxa_archive_data **pcrxa, char *dirname, size_t dirname_len)
{
	char *error;
	crxa_entry_data *data;

	if (!(data = crxa_get_or_create_entry_data((*pcrxa)->fname, (*pcrxa)->fname_len, dirname, dirname_len, "w+b", 2, &error, 1))) {
		if (error) {
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Directory %s does not exist and cannot be created: %s", dirname, error);
			efree(error);
		} else {
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Directory %s does not exist and cannot be created", dirname);
		}

		return;
	} else {
		if (error) {
			efree(error);
		}

		/* check for copy on write */
		if (data->crxa != *pcrxa) {
			*pcrxa = data->crxa;
		}
		crxa_entry_delref(data);
		crxa_flush(*pcrxa, 0, 0, 0, &error);

		if (error) {
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
			efree(error);
		}
	}
}
/* }}} */

/* {{{ set the contents of an internal file to those of an external file */
CRX_METHOD(Crxa, offsetSet)
{
	char *fname, *cont_str = NULL;
	size_t fname_len, cont_len;
	zval *zresource = NULL;

	if (crex_parse_parameters_ex(CREX_PARSE_PARAMS_QUIET, CREX_NUM_ARGS(), "pr", &fname, &fname_len, &zresource) == FAILURE
	&& crex_parse_parameters(CREX_NUM_ARGS(), "ps", &fname, &fname_len, &cont_str, &cont_len) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (CRXA_G(readonly) && !crxa_obj->archive->is_data) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Write operations disabled by the crx.ini setting crxa.readonly");
		RETURN_THROWS();
	}

	if (fname_len == sizeof(".crxa/stub.crx")-1 && !memcmp(fname, ".crxa/stub.crx", sizeof(".crxa/stub.crx")-1)) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Cannot set stub \".crxa/stub.crx\" directly in crxa \"%s\", use setStub", crxa_obj->archive->fname);
		RETURN_THROWS();
	}

	if (fname_len == sizeof(".crxa/alias.txt")-1 && !memcmp(fname, ".crxa/alias.txt", sizeof(".crxa/alias.txt")-1)) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Cannot set alias \".crxa/alias.txt\" directly in crxa \"%s\", use setAlias", crxa_obj->archive->fname);
		RETURN_THROWS();
	}

	if (fname_len >= sizeof(".crxa")-1 && !memcmp(fname, ".crxa", sizeof(".crxa")-1)) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Cannot set any files or directories in magic \".crxa\" directory");
		RETURN_THROWS();
	}

	crxa_add_file(&(crxa_obj->archive), fname, fname_len, cont_str, cont_len, zresource);
}
/* }}} */

/* {{{ remove a file from a crxa */
CRX_METHOD(Crxa, offsetUnset)
{
	char *fname, *error;
	size_t fname_len;
	crxa_entry_info *entry;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "p", &fname, &fname_len) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (CRXA_G(readonly) && !crxa_obj->archive->is_data) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Write operations disabled by the crx.ini setting crxa.readonly");
		RETURN_THROWS();
	}

	if (crex_hash_str_exists(&crxa_obj->archive->manifest, fname, (uint32_t) fname_len)) {
		if (NULL != (entry = crex_hash_str_find_ptr(&crxa_obj->archive->manifest, fname, (uint32_t) fname_len))) {
			if (entry->is_deleted) {
				/* entry is deleted, but has not been flushed to disk yet */
				return;
			}

			if (crxa_obj->archive->is_persistent) {
				if (FAILURE == crxa_copy_on_write(&(crxa_obj->archive))) {
					crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa \"%s\" is persistent, unable to copy on write", crxa_obj->archive->fname);
					RETURN_THROWS();
				}
				/* re-populate entry after copy on write */
				entry = crex_hash_str_find_ptr(&crxa_obj->archive->manifest, fname, (uint32_t) fname_len);
			}
			entry->is_modified = 0;
			entry->is_deleted = 1;
			/* we need to "flush" the stream to save the newly deleted file on disk */
			crxa_flush(crxa_obj->archive, 0, 0, 0, &error);

			if (error) {
				crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
				efree(error);
			}
		}
	}
}
/* }}} */

/* {{{ Adds an empty directory to the crxa archive */
CRX_METHOD(Crxa, addEmptyDir)
{
	char *dirname;
	size_t dirname_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "p", &dirname, &dirname_len) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (dirname_len >= sizeof(".crxa")-1 && !memcmp(dirname, ".crxa", sizeof(".crxa")-1)) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Cannot create a directory in magic \".crxa\" directory");
		RETURN_THROWS();
	}

	crxa_mkdir(&crxa_obj->archive, dirname, dirname_len);
}
/* }}} */

/* {{{ Adds a file to the archive using the filename, or the second parameter as the name within the archive */
CRX_METHOD(Crxa, addFile)
{
	char *fname, *localname = NULL;
	size_t fname_len, localname_len = 0;
	crx_stream *resource;
	zval zresource;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "p|s!", &fname, &fname_len, &localname, &localname_len) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (!strstr(fname, "://") && crx_check_open_basedir(fname)) {
		crex_throw_exception_ex(spl_ce_RuntimeException, 0, "crxa error: unable to open file \"%s\" to add to crxa archive, open_basedir restrictions prevent this", fname);
		RETURN_THROWS();
	}

	if (!(resource = crx_stream_open_wrapper(fname, "rb", 0, NULL))) {
		crex_throw_exception_ex(spl_ce_RuntimeException, 0, "crxa error: unable to open file \"%s\" to add to crxa archive", fname);
		RETURN_THROWS();
	}

	if (localname) {
		fname = localname;
		fname_len = localname_len;
	}

	crx_stream_to_zval(resource, &zresource);
	crxa_add_file(&(crxa_obj->archive), fname, fname_len, NULL, 0, &zresource);
	zval_ptr_dtor(&zresource);
}
/* }}} */

/* {{{ Adds a file to the archive using its contents as a string */
CRX_METHOD(Crxa, addFromString)
{
	char *localname, *cont_str;
	size_t localname_len, cont_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ps", &localname, &localname_len, &cont_str, &cont_len) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	crxa_add_file(&(crxa_obj->archive), localname, localname_len, cont_str, cont_len, NULL);
}
/* }}} */

/* {{{ Returns the stub at the head of a crxa archive as a string. */
CRX_METHOD(Crxa, getStub)
{
	size_t len;
	crex_string *buf;
	crx_stream *fp;
	crx_stream_filter *filter = NULL;
	crxa_entry_info *stub;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (crxa_obj->archive->is_tar || crxa_obj->archive->is_zip) {

		if (NULL != (stub = crex_hash_str_find_ptr(&(crxa_obj->archive->manifest), ".crxa/stub.crx", sizeof(".crxa/stub.crx")-1))) {
			if (crxa_obj->archive->fp && !crxa_obj->archive->is_brandnew && !(stub->flags & CRXA_ENT_COMPRESSION_MASK)) {
				fp = crxa_obj->archive->fp;
			} else {
				if (!(fp = crx_stream_open_wrapper(crxa_obj->archive->fname, "rb", 0, NULL))) {
					crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "crxa error: unable to open crxa \"%s\"", crxa_obj->archive->fname);
					RETURN_THROWS();
				}
				if (stub->flags & CRXA_ENT_COMPRESSION_MASK) {
					char *filter_name;

					if ((filter_name = crxa_decompress_filter(stub, 0)) != NULL) {
						filter = crx_stream_filter_create(filter_name, NULL, crx_stream_is_persistent(fp));
					} else {
						filter = NULL;
					}
					if (!filter) {
						crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "crxa error: unable to read stub of crxa \"%s\" (cannot create %s filter)", crxa_obj->archive->fname, crxa_decompress_filter(stub, 1));
						RETURN_THROWS();
					}
					crx_stream_filter_append(&fp->readfilters, filter);
				}
			}

			if (!fp)  {
				crex_throw_exception_ex(spl_ce_RuntimeException, 0,
					"Unable to read stub");
				RETURN_THROWS();
			}

			crx_stream_seek(fp, stub->offset_abs, SEEK_SET);
			len = stub->uncompressed_filesize;
			goto carry_on;
		} else {
			RETURN_EMPTY_STRING();
		}
	}
	len = crxa_obj->archive->halt_offset;

	if (crxa_obj->archive->fp && !crxa_obj->archive->is_brandnew) {
		fp = crxa_obj->archive->fp;
	} else {
		fp = crx_stream_open_wrapper(crxa_obj->archive->fname, "rb", 0, NULL);
	}

	if (!fp)  {
		crex_throw_exception_ex(spl_ce_RuntimeException, 0,
			"Unable to read stub");
		RETURN_THROWS();
	}

	crx_stream_rewind(fp);
carry_on:
	buf = crex_string_alloc(len, 0);

	if (len != crx_stream_read(fp, ZSTR_VAL(buf), len)) {
		if (fp != crxa_obj->archive->fp) {
			crx_stream_close(fp);
		}
		crex_throw_exception_ex(spl_ce_RuntimeException, 0,
			"Unable to read stub");
		crex_string_release_ex(buf, 0);
		RETURN_THROWS();
	}

	if (filter) {
		crx_stream_filter_flush(filter, 1);
		crx_stream_filter_remove(filter, 1);
	}

	if (fp != crxa_obj->archive->fp) {
		crx_stream_close(fp);
	}

	ZSTR_VAL(buf)[len] = '\0';
	ZSTR_LEN(buf) = len;
	RETVAL_STR(buf);
}
/* }}}*/

/* {{{ Returns TRUE if the crxa has global metadata, FALSE otherwise. */
CRX_METHOD(Crxa, hasMetadata)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	RETURN_BOOL(crxa_metadata_tracker_has_data(&crxa_obj->archive->metadata_tracker, crxa_obj->archive->is_persistent));
}
/* }}} */

/* {{{ Returns the global metadata of the crxa */
CRX_METHOD(Crxa, getMetadata)
{
	HashTable *unserialize_options = NULL;
	crxa_metadata_tracker *tracker;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT(unserialize_options)
	CREX_PARSE_PARAMETERS_END();

	CRXA_ARCHIVE_OBJECT();

	tracker = &crxa_obj->archive->metadata_tracker;
	if (crxa_metadata_tracker_has_data(tracker, crxa_obj->archive->is_persistent)) {
		crxa_metadata_tracker_unserialize_or_copy(tracker, return_value, crxa_obj->archive->is_persistent, unserialize_options, "Crxa::getMetadata");
	}
}
/* }}} */

/* {{{ Modifies the crxa metadata or throws */
static int serialize_metadata_or_throw(crxa_metadata_tracker *tracker, int persistent, zval *metadata)
{
	crx_serialize_data_t metadata_hash;
	smart_str main_metadata_str = {0};

	CRX_VAR_SERIALIZE_INIT(metadata_hash);
	crx_var_serialize(&main_metadata_str, metadata, &metadata_hash);
	CRX_VAR_SERIALIZE_DESTROY(metadata_hash);
	if (EG(exception)) {
		/* Serialization can throw. Don't overwrite the original value or original string. */
		return FAILURE;
	}

	crxa_metadata_tracker_free(tracker, persistent);
	if (EG(exception)) {
		/* Destructor can throw. */
		crex_string_release(main_metadata_str.s);
		return FAILURE;
	}

	if (tracker->str) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "Metadata unexpectedly changed during setMetadata()");
		crex_string_release(main_metadata_str.s);
		return FAILURE;
	}
	ZVAL_COPY(&tracker->val, metadata);
	tracker->str = main_metadata_str.s;
	return SUCCESS;
}

/* {{{ Sets the global metadata of the crxa */
CRX_METHOD(Crxa, setMetadata)
{
	char *error;
	zval *metadata;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "z", &metadata) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (CRXA_G(readonly) && !crxa_obj->archive->is_data) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Write operations disabled by the crx.ini setting crxa.readonly");
		RETURN_THROWS();
	}

	if (crxa_obj->archive->is_persistent && FAILURE == crxa_copy_on_write(&(crxa_obj->archive))) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa \"%s\" is persistent, unable to copy on write", crxa_obj->archive->fname);
		RETURN_THROWS();
	}

	CREX_ASSERT(!crxa_obj->archive->is_persistent); /* Should no longer be persistent */
	if (serialize_metadata_or_throw(&crxa_obj->archive->metadata_tracker, crxa_obj->archive->is_persistent, metadata) != SUCCESS) {
		RETURN_THROWS();
	}

	crxa_obj->archive->is_modified = 1;
	crxa_flush(crxa_obj->archive, 0, 0, 0, &error);

	if (error) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
		efree(error);
	}
}
/* }}} */

/* {{{ Deletes the global metadata of the crxa */
CRX_METHOD(Crxa, delMetadata)
{
	char *error;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ARCHIVE_OBJECT();

	if (CRXA_G(readonly) && !crxa_obj->archive->is_data) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Write operations disabled by the crx.ini setting crxa.readonly");
		RETURN_THROWS();
	}

	if (!crxa_metadata_tracker_has_data(&crxa_obj->archive->metadata_tracker, crxa_obj->archive->is_persistent)) {
		RETURN_TRUE;
	}

	crxa_metadata_tracker_free(&crxa_obj->archive->metadata_tracker, crxa_obj->archive->is_persistent);
	crxa_obj->archive->is_modified = 1;
	crxa_flush(crxa_obj->archive, 0, 0, 0, &error);

	if (error) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
		efree(error);
		RETURN_THROWS();
	} else {
		RETURN_TRUE;
	}
}
/* }}} */

static int crxa_extract_file(bool overwrite, crxa_entry_info *entry, char *dest, size_t dest_len, char **error) /* {{{ */
{
	crx_stream_statbuf ssb;
	size_t len;
	crx_stream *fp;
	char *fullpath;
	const char *slash;
	mode_t mode;
	cwd_state new_state;
	char *filename;
	size_t filename_len;

	if (entry->is_mounted) {
		/* silently ignore mounted entries */
		return SUCCESS;
	}

	if (entry->filename_len >= sizeof(".crxa")-1 && !memcmp(entry->filename, ".crxa", sizeof(".crxa")-1)) {
		return SUCCESS;
	}
	/* strip .. from path and restrict it to be under dest directory */
	new_state.cwd = (char*)emalloc(2);
	new_state.cwd[0] = DEFAULT_SLASH;
	new_state.cwd[1] = '\0';
	new_state.cwd_length = 1;
	if (virtual_file_ex(&new_state, entry->filename, NULL, CWD_EXPAND) != 0 ||
			new_state.cwd_length <= 1) {
		if (EINVAL == errno && entry->filename_len > 50) {
			char *tmp = estrndup(entry->filename, 50);
			spprintf(error, 4096, "Cannot extract \"%s...\" to \"%s...\", extracted filename is too long for filesystem", tmp, dest);
			efree(tmp);
		} else {
			spprintf(error, 4096, "Cannot extract \"%s\", internal error", entry->filename);
		}
		efree(new_state.cwd);
		return FAILURE;
	}
	filename = new_state.cwd + 1;
	filename_len = new_state.cwd_length - 1;
#ifdef CRX_WIN32
	/* unixify the path back, otherwise non zip formats might be broken */
	{
		size_t cnt = 0;

		do {
			if ('\\' == filename[cnt]) {
				filename[cnt] = '/';
			}
		} while (cnt++ < filename_len);
	}
#endif

	len = spprintf(&fullpath, 0, "%s/%s", dest, filename);

	if (len >= MAXPATHLEN) {
		char *tmp;
		/* truncate for error message */
		fullpath[50] = '\0';
		if (entry->filename_len > 50) {
			tmp = estrndup(entry->filename, 50);
			spprintf(error, 4096, "Cannot extract \"%s...\" to \"%s...\", extracted filename is too long for filesystem", tmp, fullpath);
			efree(tmp);
		} else {
			spprintf(error, 4096, "Cannot extract \"%s\" to \"%s...\", extracted filename is too long for filesystem", entry->filename, fullpath);
		}
		efree(fullpath);
		efree(new_state.cwd);
		return FAILURE;
	}

	if (!len) {
		spprintf(error, 4096, "Cannot extract \"%s\", internal error", entry->filename);
		efree(fullpath);
		efree(new_state.cwd);
		return FAILURE;
	}

	if (crx_check_open_basedir(fullpath)) {
		spprintf(error, 4096, "Cannot extract \"%s\" to \"%s\", openbasedir/safe mode restrictions in effect", entry->filename, fullpath);
		efree(fullpath);
		efree(new_state.cwd);
		return FAILURE;
	}

	/* let see if the path already exists */
	if (!overwrite && SUCCESS == crx_stream_stat_path(fullpath, &ssb)) {
		spprintf(error, 4096, "Cannot extract \"%s\" to \"%s\", path already exists", entry->filename, fullpath);
		efree(fullpath);
		efree(new_state.cwd);
		return FAILURE;
	}

	/* perform dirname */
	slash = crex_memrchr(filename, '/', filename_len);

	if (slash) {
		fullpath[dest_len + (slash - filename) + 1] = '\0';
	} else {
		fullpath[dest_len] = '\0';
	}

	if (FAILURE == crx_stream_stat_path(fullpath, &ssb)) {
		if (entry->is_dir) {
			if (!crx_stream_mkdir(fullpath, entry->flags & CRXA_ENT_PERM_MASK,  CRX_STREAM_MKDIR_RECURSIVE, NULL)) {
				spprintf(error, 4096, "Cannot extract \"%s\", could not create directory \"%s\"", entry->filename, fullpath);
				efree(fullpath);
				efree(new_state.cwd);
				return FAILURE;
			}
		} else {
			if (!crx_stream_mkdir(fullpath, 0777,  CRX_STREAM_MKDIR_RECURSIVE, NULL)) {
				spprintf(error, 4096, "Cannot extract \"%s\", could not create directory \"%s\"", entry->filename, fullpath);
				efree(fullpath);
				efree(new_state.cwd);
				return FAILURE;
			}
		}
	}

	if (slash) {
		fullpath[dest_len + (slash - filename) + 1] = '/';
	} else {
		fullpath[dest_len] = '/';
	}

	filename = NULL;
	efree(new_state.cwd);
	/* it is a standalone directory, job done */
	if (entry->is_dir) {
		efree(fullpath);
		return SUCCESS;
	}

	fp = crx_stream_open_wrapper(fullpath, "w+b", REPORT_ERRORS, NULL);

	if (!fp) {
		spprintf(error, 4096, "Cannot extract \"%s\", could not open for writing \"%s\"", entry->filename, fullpath);
		efree(fullpath);
		return FAILURE;
	}

	if ((crxa_get_fp_type(entry) == CRXA_FP && (entry->flags & CRXA_ENT_COMPRESSION_MASK)) || !crxa_get_efp(entry, 0)) {
		if (FAILURE == crxa_open_entry_fp(entry, error, 1)) {
			if (error) {
				spprintf(error, 4096, "Cannot extract \"%s\" to \"%s\", unable to open internal file pointer: %s", entry->filename, fullpath, *error);
			} else {
				spprintf(error, 4096, "Cannot extract \"%s\" to \"%s\", unable to open internal file pointer", entry->filename, fullpath);
			}
			efree(fullpath);
			crx_stream_close(fp);
			return FAILURE;
		}
	}

	if (FAILURE == crxa_seek_efp(entry, 0, SEEK_SET, 0, 0)) {
		spprintf(error, 4096, "Cannot extract \"%s\" to \"%s\", unable to seek internal file pointer", entry->filename, fullpath);
		efree(fullpath);
		crx_stream_close(fp);
		return FAILURE;
	}

	if (SUCCESS != crx_stream_copy_to_stream_ex(crxa_get_efp(entry, 0), fp, entry->uncompressed_filesize, NULL)) {
		spprintf(error, 4096, "Cannot extract \"%s\" to \"%s\", copying contents failed", entry->filename, fullpath);
		efree(fullpath);
		crx_stream_close(fp);
		return FAILURE;
	}

	crx_stream_close(fp);
	mode = (mode_t) entry->flags & CRXA_ENT_PERM_MASK;

	if (FAILURE == VCWD_CHMOD(fullpath, mode)) {
		spprintf(error, 4096, "Cannot extract \"%s\" to \"%s\", setting file permissions failed", entry->filename, fullpath);
		efree(fullpath);
		return FAILURE;
	}

	efree(fullpath);
	return SUCCESS;
}
/* }}} */

static int extract_helper(crxa_archive_data *archive, crex_string *search, char *pathto, size_t pathto_len, bool overwrite, char **error) { /* {{{ */
	int extracted = 0;
	crxa_entry_info *entry;

	if (!search) {
		/* nothing to match -- extract all files */
		CREX_HASH_MAP_FOREACH_PTR(&archive->manifest, entry) {
			if (FAILURE == crxa_extract_file(overwrite, entry, pathto, pathto_len, error)) return -1;
			extracted++;
		} CREX_HASH_FOREACH_END();
	} else if ('/' == ZSTR_VAL(search)[ZSTR_LEN(search) - 1]) {
		/* ends in "/" -- extract all entries having that prefix */
		CREX_HASH_MAP_FOREACH_PTR(&archive->manifest, entry) {
			if (0 != strncmp(ZSTR_VAL(search), entry->filename, ZSTR_LEN(search))) continue;
			if (FAILURE == crxa_extract_file(overwrite, entry, pathto, pathto_len, error)) return -1;
			extracted++;
		} CREX_HASH_FOREACH_END();
	} else {
		/* otherwise, looking for an exact match */
		entry = crex_hash_find_ptr(&archive->manifest, search);
		if (NULL == entry) return 0;
		if (FAILURE == crxa_extract_file(overwrite, entry, pathto, pathto_len, error)) return -1;
		return 1;
	}

	return extracted;
}
/* }}} */

/* {{{ Extract one or more file from a crxa archive, optionally overwriting existing files */
CRX_METHOD(Crxa, extractTo)
{
	crx_stream *fp;
	crx_stream_statbuf ssb;
	char *pathto;
	crex_string *filename = NULL;
	size_t pathto_len;
	int ret;
	zval *zval_file;
	HashTable *files_ht = NULL;
	bool overwrite = 0;
	char *error = NULL;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_PATH(pathto, pathto_len)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT_OR_STR_OR_NULL(files_ht, filename)
		C_PARAM_BOOL(overwrite)
	CREX_PARSE_PARAMETERS_END();

	CRXA_ARCHIVE_OBJECT();

	fp = crx_stream_open_wrapper(crxa_obj->archive->fname, "rb", IGNORE_URL|STREAM_MUST_SEEK, NULL);

	if (!fp) {
		crex_throw_exception_ex(spl_ce_InvalidArgumentException, 0,
			"Invalid argument, %s cannot be found", crxa_obj->archive->fname);
		RETURN_THROWS();
	}

	crx_stream_close(fp);

	if (pathto_len < 1) {
		crex_throw_exception_ex(spl_ce_InvalidArgumentException, 0,
			"Invalid argument, extraction path must be non-zero length");
		RETURN_THROWS();
	}

	if (pathto_len >= MAXPATHLEN) {
		char *tmp = estrndup(pathto, 50);
		/* truncate for error message */
		crex_throw_exception_ex(spl_ce_InvalidArgumentException, 0, "Cannot extract to \"%s...\", destination directory is too long for filesystem", tmp);
		efree(tmp);
		RETURN_THROWS();
	}

	if (crx_stream_stat_path(pathto, &ssb) < 0) {
		ret = crx_stream_mkdir(pathto, 0777,  CRX_STREAM_MKDIR_RECURSIVE, NULL);
		if (!ret) {
			crex_throw_exception_ex(spl_ce_RuntimeException, 0,
				"Unable to create path \"%s\" for extraction", pathto);
			RETURN_THROWS();
		}
	} else if (!(ssb.sb.st_mode & S_IFDIR)) {
		crex_throw_exception_ex(spl_ce_RuntimeException, 0,
			"Unable to use path \"%s\" for extraction, it is a file, must be a directory", pathto);
		RETURN_THROWS();
	}

	if (files_ht) {
		if (crex_hash_num_elements(files_ht) == 0) {
			RETURN_FALSE;
		}

		CREX_HASH_FOREACH_VAL(files_ht, zval_file) {
			ZVAL_DEREF(zval_file);
			if (IS_STRING != C_TYPE_P(zval_file)) {
				crex_throw_exception_ex(spl_ce_InvalidArgumentException, 0,
					"Invalid argument, array of filenames to extract contains non-string value");
				RETURN_THROWS();
			}
			switch (extract_helper(crxa_obj->archive, C_STR_P(zval_file), pathto, pathto_len, overwrite, &error)) {
				case -1:
					crex_throw_exception_ex(crxa_ce_CrxaException, 0, "Extraction from crxa \"%s\" failed: %s",
						crxa_obj->archive->fname, error);
					efree(error);
					RETURN_THROWS();
				case 0:
					crex_throw_exception_ex(crxa_ce_CrxaException, 0,
						"crxa error: attempted to extract non-existent file or directory \"%s\" from crxa \"%s\"",
						ZSTR_VAL(C_STR_P(zval_file)), crxa_obj->archive->fname);
					RETURN_THROWS();
			}
		} CREX_HASH_FOREACH_END();
		RETURN_TRUE;
	}

	ret = extract_helper(crxa_obj->archive, filename, pathto, pathto_len, overwrite, &error);
	if (-1 == ret) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "Extraction from crxa \"%s\" failed: %s",
			crxa_obj->archive->fname, error);
		efree(error);
	} else if (0 == ret && NULL != filename) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0,
			"crxa error: attempted to extract non-existent file or directory \"%s\" from crxa \"%s\"",
			ZSTR_VAL(filename), crxa_obj->archive->fname);
	} else {
		RETURN_TRUE;
	}
}
/* }}} */


/* {{{ Construct a Crxa entry object */
CRX_METHOD(CrxaFileInfo, __main)
{
	char *fname, *arch, *entry, *error;
	size_t fname_len;
	size_t arch_len, entry_len;
	crxa_entry_object *entry_obj;
	crxa_entry_info *entry_info;
	crxa_archive_data *crxa_data;
	zval *zobj = CREX_THIS, arg1;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "p", &fname, &fname_len) == FAILURE) {
		RETURN_THROWS();
	}

	entry_obj = (crxa_entry_object*)((char*)C_OBJ_P(zobj) - C_OBJ_P(zobj)->handlers->offset);

	if (entry_obj->entry) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Cannot call constructor twice");
		RETURN_THROWS();
	}

	if (fname_len < 7 || memcmp(fname, "crxa://", 7) || crxa_split_fname(fname, fname_len, &arch, &arch_len, &entry, &entry_len, 2, 0) == FAILURE) {
		crex_throw_exception_ex(spl_ce_RuntimeException, 0,
			"'%s' is not a valid crxa archive URL (must have at least crxa://filename.crxa)", fname);
		RETURN_THROWS();
	}

	if (crxa_open_from_filename(arch, arch_len, NULL, 0, REPORT_ERRORS, &crxa_data, &error) == FAILURE) {
		efree(arch);
		efree(entry);
		if (error) {
			crex_throw_exception_ex(spl_ce_RuntimeException, 0,
				"Cannot open crxa file '%s': %s", fname, error);
			efree(error);
		} else {
			crex_throw_exception_ex(spl_ce_RuntimeException, 0,
				"Cannot open crxa file '%s'", fname);
		}
		RETURN_THROWS();
	}

	if ((entry_info = crxa_get_entry_info_dir(crxa_data, entry, entry_len, 1, &error, 1)) == NULL) {
		crex_throw_exception_ex(spl_ce_RuntimeException, 0,
			"Cannot access crxa file entry '%s' in archive '%s'%s%s", entry, arch, error ? ", " : "", error ? error : "");
		efree(arch);
		efree(entry);
		RETURN_THROWS();
	}

	efree(arch);
	efree(entry);

	entry_obj->entry = entry_info;

	ZVAL_STRINGL(&arg1, fname, fname_len);

	crex_call_known_instance_method_with_1_params(spl_ce_SplFileInfo->constructor,
		C_OBJ_P(zobj), NULL, &arg1);

	zval_ptr_dtor(&arg1);
}
/* }}} */

#define CRXA_ENTRY_OBJECT() \
	zval *zobj = CREX_THIS; \
	crxa_entry_object *entry_obj = (crxa_entry_object*)((char*)C_OBJ_P(zobj) - C_OBJ_P(zobj)->handlers->offset); \
	if (!entry_obj->entry) { \
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, \
			"Cannot call method on an uninitialized CrxaFileInfo object"); \
		RETURN_THROWS(); \
	}

/* {{{ clean up directory-based entry objects */
CRX_METHOD(CrxaFileInfo, __destruct)
{
	zval *zobj = CREX_THIS;
	crxa_entry_object *entry_obj = (crxa_entry_object*)((char*)C_OBJ_P(zobj) - C_OBJ_P(zobj)->handlers->offset);

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if (entry_obj->entry && entry_obj->entry->is_temp_dir) {
		if (entry_obj->entry->filename) {
			efree(entry_obj->entry->filename);
			entry_obj->entry->filename = NULL;
		}

		efree(entry_obj->entry);
		entry_obj->entry = NULL;
	}
}
/* }}} */

/* {{{ Returns the compressed size */
CRX_METHOD(CrxaFileInfo, getCompressedSize)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ENTRY_OBJECT();

	RETURN_LONG(entry_obj->entry->compressed_filesize);
}
/* }}} */

/* {{{ Returns whether the entry is compressed, and whether it is compressed with Crxa::GZ or Crxa::BZ2 if specified */
CRX_METHOD(CrxaFileInfo, isCompressed)
{
	crex_long method;
	bool method_is_null = 1;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|l!", &method, &method_is_null) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ENTRY_OBJECT();

	if (method_is_null) {
		RETURN_BOOL(entry_obj->entry->flags & CRXA_ENT_COMPRESSION_MASK);
	}

	switch (method) {
		case 9021976: /* Retained for BC */
			RETURN_BOOL(entry_obj->entry->flags & CRXA_ENT_COMPRESSION_MASK);
		case CRXA_ENT_COMPRESSED_GZ:
			RETURN_BOOL(entry_obj->entry->flags & CRXA_ENT_COMPRESSED_GZ);
		case CRXA_ENT_COMPRESSED_BZ2:
			RETURN_BOOL(entry_obj->entry->flags & CRXA_ENT_COMPRESSED_BZ2);
		default:
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Unknown compression type specified");
			RETURN_THROWS();
	}
}
/* }}} */

/* {{{ Returns CRC32 code or throws an exception if not CRC checked */
CRX_METHOD(CrxaFileInfo, getCRC32)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ENTRY_OBJECT();

	if (entry_obj->entry->is_dir) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, \
			"Crxa entry is a directory, does not have a CRC"); \
		RETURN_THROWS();
	}

	if (entry_obj->entry->is_crc_checked) {
		RETURN_LONG(entry_obj->entry->crc32);
	} else {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Crxa entry was not CRC checked");
		RETURN_THROWS();
	}
}
/* }}} */

/* {{{ Returns whether file entry is CRC checked */
CRX_METHOD(CrxaFileInfo, isCRCChecked)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ENTRY_OBJECT();

	RETURN_BOOL(entry_obj->entry->is_crc_checked);
}
/* }}} */

/* {{{ Returns the Crxa file entry flags */
CRX_METHOD(CrxaFileInfo, getCrxaFlags)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ENTRY_OBJECT();

	RETURN_LONG(entry_obj->entry->flags & ~(CRXA_ENT_PERM_MASK|CRXA_ENT_COMPRESSION_MASK));
}
/* }}} */

/* {{{ set the file permissions for the Crxa.  This only allows setting execution bit, read/write */
CRX_METHOD(CrxaFileInfo, chmod)
{
	char *error;
	crex_long perms;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &perms) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ENTRY_OBJECT();

	if (entry_obj->entry->is_temp_dir) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, \
			"Crxa entry \"%s\" is a temporary directory (not an actual entry in the archive), cannot chmod", entry_obj->entry->filename); \
		RETURN_THROWS();
	}

	if (CRXA_G(readonly) && !entry_obj->entry->crxa->is_data) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "Cannot modify permissions for file \"%s\" in crxa \"%s\", write operations are prohibited", entry_obj->entry->filename, entry_obj->entry->crxa->fname);
		RETURN_THROWS();
	}

	if (entry_obj->entry->is_persistent) {
		crxa_archive_data *crxa = entry_obj->entry->crxa;

		if (FAILURE == crxa_copy_on_write(&crxa)) {
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa \"%s\" is persistent, unable to copy on write", crxa->fname);
			RETURN_THROWS();
		}
		/* re-populate after copy-on-write */
		entry_obj->entry = crex_hash_str_find_ptr(&crxa->manifest, entry_obj->entry->filename, entry_obj->entry->filename_len);
	}
	/* clear permissions */
	entry_obj->entry->flags &= ~CRXA_ENT_PERM_MASK;
	perms &= 0777;
	entry_obj->entry->flags |= perms;
	entry_obj->entry->old_flags = entry_obj->entry->flags;
	entry_obj->entry->crxa->is_modified = 1;
	entry_obj->entry->is_modified = 1;

	/* hackish cache in crx_stat needs to be cleared */
	/* if this code fails to work, check main/streams/streams.c, _crx_stream_stat_path */
	if (BG(CurrentLStatFile)) {
		crex_string_release(BG(CurrentLStatFile));
	}

	if (BG(CurrentStatFile)) {
		crex_string_release(BG(CurrentStatFile));
	}

	BG(CurrentLStatFile) = NULL;
	BG(CurrentStatFile) = NULL;
	crxa_flush(entry_obj->entry->crxa, 0, 0, 0, &error);

	if (error) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
		efree(error);
	}
}
/* }}} */

/* {{{ Returns the metadata of the entry */
CRX_METHOD(CrxaFileInfo, hasMetadata)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ENTRY_OBJECT();

	RETURN_BOOL(crxa_metadata_tracker_has_data(&entry_obj->entry->metadata_tracker, entry_obj->entry->is_persistent));
}
/* }}} */

/* {{{ Returns the metadata of the entry */
CRX_METHOD(CrxaFileInfo, getMetadata)
{
	HashTable *unserialize_options = NULL;
	crxa_metadata_tracker *tracker;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT(unserialize_options)
	CREX_PARSE_PARAMETERS_END();

	CRXA_ENTRY_OBJECT();

	tracker = &entry_obj->entry->metadata_tracker;
	if (crxa_metadata_tracker_has_data(tracker, entry_obj->entry->is_persistent)) {
		crxa_metadata_tracker_unserialize_or_copy(tracker, return_value, entry_obj->entry->is_persistent, unserialize_options, "CrxaFileInfo::getMetadata");
	}
}
/* }}} */

/* {{{ Sets the metadata of the entry */
CRX_METHOD(CrxaFileInfo, setMetadata)
{
	char *error;
	zval *metadata;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "z", &metadata) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ENTRY_OBJECT();

	if (CRXA_G(readonly) && !entry_obj->entry->crxa->is_data) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Write operations disabled by the crx.ini setting crxa.readonly");
		RETURN_THROWS();
	}

	if (entry_obj->entry->is_temp_dir) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, \
			"Crxa entry is a temporary directory (not an actual entry in the archive), cannot set metadata"); \
		RETURN_THROWS();
	}

	if (entry_obj->entry->is_persistent) {
		crxa_archive_data *crxa = entry_obj->entry->crxa;

		if (FAILURE == crxa_copy_on_write(&crxa)) {
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa \"%s\" is persistent, unable to copy on write", crxa->fname);
			RETURN_THROWS();
		}
		/* re-populate after copy-on-write */
		entry_obj->entry = crex_hash_str_find_ptr(&crxa->manifest, entry_obj->entry->filename, entry_obj->entry->filename_len);
		CREX_ASSERT(!entry_obj->entry->is_persistent); /* Should no longer be persistent */
	}

	if (serialize_metadata_or_throw(&entry_obj->entry->metadata_tracker, entry_obj->entry->is_persistent, metadata) != SUCCESS) {
		RETURN_THROWS();
	}

	entry_obj->entry->is_modified = 1;
	entry_obj->entry->crxa->is_modified = 1;
	crxa_flush(entry_obj->entry->crxa, 0, 0, 0, &error);

	if (error) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
		efree(error);
	}
}
/* }}} */

/* {{{ Deletes the metadata of the entry */
CRX_METHOD(CrxaFileInfo, delMetadata)
{
	char *error;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ENTRY_OBJECT();

	if (CRXA_G(readonly) && !entry_obj->entry->crxa->is_data) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Write operations disabled by the crx.ini setting crxa.readonly");
		RETURN_THROWS();
	}

	if (entry_obj->entry->is_temp_dir) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, \
			"Crxa entry is a temporary directory (not an actual entry in the archive), cannot delete metadata"); \
		RETURN_THROWS();
	}

	if (crxa_metadata_tracker_has_data(&entry_obj->entry->metadata_tracker, entry_obj->entry->is_persistent)) {
		if (entry_obj->entry->is_persistent) {
			crxa_archive_data *crxa = entry_obj->entry->crxa;

			if (FAILURE == crxa_copy_on_write(&crxa)) {
				crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa \"%s\" is persistent, unable to copy on write", crxa->fname);
				RETURN_THROWS();
			}
			/* re-populate after copy-on-write */
			entry_obj->entry = crex_hash_str_find_ptr(&crxa->manifest, entry_obj->entry->filename, entry_obj->entry->filename_len);
		}
		/* multiple values may reference the metadata */
		crxa_metadata_tracker_free(&entry_obj->entry->metadata_tracker, entry_obj->entry->is_persistent);
		entry_obj->entry->is_modified = 1;
		entry_obj->entry->crxa->is_modified = 1;

		crxa_flush(entry_obj->entry->crxa, 0, 0, 0, &error);

		if (error) {
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
			efree(error);
			RETURN_THROWS();
		} else {
			RETURN_TRUE;
		}

	} else {
		RETURN_TRUE;
	}
}
/* }}} */

/* {{{ return the complete file contents of the entry (like file_get_contents) */
CRX_METHOD(CrxaFileInfo, getContent)
{
	char *error;
	crx_stream *fp;
	crxa_entry_info *link;
	crex_string *str;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ENTRY_OBJECT();

	if (entry_obj->entry->is_dir) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
			"crxa error: Cannot retrieve contents, \"%s\" in crxa \"%s\" is a directory", entry_obj->entry->filename, entry_obj->entry->crxa->fname);
		RETURN_THROWS();
	}

	link = crxa_get_link_source(entry_obj->entry);

	if (!link) {
		link = entry_obj->entry;
	}

	if (SUCCESS != crxa_open_entry_fp(link, &error, 0)) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
			"crxa error: Cannot retrieve contents, \"%s\" in crxa \"%s\": %s", entry_obj->entry->filename, entry_obj->entry->crxa->fname, error);
		efree(error);
		RETURN_THROWS();
	}

	if (!(fp = crxa_get_efp(link, 0))) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
			"crxa error: Cannot retrieve contents of \"%s\" in crxa \"%s\"", entry_obj->entry->filename, entry_obj->entry->crxa->fname);
		RETURN_THROWS();
	}

	crxa_seek_efp(link, 0, SEEK_SET, 0, 0);
	str = crx_stream_copy_to_mem(fp, link->uncompressed_filesize, 0);
	if (str) {
		RETURN_STR(str);
	} else {
		RETURN_EMPTY_STRING();
	}
}
/* }}} */

/* {{{ Instructs the Crxa class to compress the current file using zlib or bzip2 compression */
CRX_METHOD(CrxaFileInfo, compress)
{
	crex_long method;
	char *error;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &method) == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ENTRY_OBJECT();

	if (entry_obj->entry->is_tar) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
			"Cannot compress with Gzip compression, not possible with tar-based crxa archives");
		RETURN_THROWS();
	}

	if (entry_obj->entry->is_dir) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, \
			"Crxa entry is a directory, cannot set compression"); \
		RETURN_THROWS();
	}

	if (CRXA_G(readonly) && !entry_obj->entry->crxa->is_data) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
			"Crxa is readonly, cannot change compression");
		RETURN_THROWS();
	}

	if (entry_obj->entry->is_deleted) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
			"Cannot compress deleted file");
		RETURN_THROWS();
	}

	if (entry_obj->entry->is_persistent) {
		crxa_archive_data *crxa = entry_obj->entry->crxa;

		if (FAILURE == crxa_copy_on_write(&crxa)) {
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa \"%s\" is persistent, unable to copy on write", crxa->fname);
			RETURN_THROWS();
		}
		/* re-populate after copy-on-write */
		entry_obj->entry = crex_hash_str_find_ptr(&crxa->manifest, entry_obj->entry->filename, entry_obj->entry->filename_len);
	}
	switch (method) {
		case CRXA_ENT_COMPRESSED_GZ:
			if (entry_obj->entry->flags & CRXA_ENT_COMPRESSED_GZ) {
				RETURN_TRUE;
			}

			if ((entry_obj->entry->flags & CRXA_ENT_COMPRESSED_BZ2) != 0) {
				if (!CRXA_G(has_bz2)) {
					crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
						"Cannot compress with gzip compression, file is already compressed with bzip2 compression and bz2 extension is not enabled, cannot decompress");
					RETURN_THROWS();
				}

				/* decompress this file indirectly */
				if (SUCCESS != crxa_open_entry_fp(entry_obj->entry, &error, 1)) {
					crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
						"crxa error: Cannot decompress bzip2-compressed file \"%s\" in crxa \"%s\" in order to compress with gzip: %s", entry_obj->entry->filename, entry_obj->entry->crxa->fname, error);
					efree(error);
					RETURN_THROWS();
				}
			}

			if (!CRXA_G(has_zlib)) {
				crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
					"Cannot compress with gzip compression, zlib extension is not enabled");
				RETURN_THROWS();
			}

			entry_obj->entry->old_flags = entry_obj->entry->flags;
			entry_obj->entry->flags &= ~CRXA_ENT_COMPRESSION_MASK;
			entry_obj->entry->flags |= CRXA_ENT_COMPRESSED_GZ;
			break;
		case CRXA_ENT_COMPRESSED_BZ2:
			if (entry_obj->entry->flags & CRXA_ENT_COMPRESSED_BZ2) {
				RETURN_TRUE;
			}

			if ((entry_obj->entry->flags & CRXA_ENT_COMPRESSED_GZ) != 0) {
				if (!CRXA_G(has_zlib)) {
					crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
						"Cannot compress with bzip2 compression, file is already compressed with gzip compression and zlib extension is not enabled, cannot decompress");
					RETURN_THROWS();
				}

				/* decompress this file indirectly */
				if (SUCCESS != crxa_open_entry_fp(entry_obj->entry, &error, 1)) {
					crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
						"crxa error: Cannot decompress gzip-compressed file \"%s\" in crxa \"%s\" in order to compress with bzip2: %s", entry_obj->entry->filename, entry_obj->entry->crxa->fname, error);
					efree(error);
					RETURN_THROWS();
				}
			}

			if (!CRXA_G(has_bz2)) {
				crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
					"Cannot compress with bzip2 compression, bz2 extension is not enabled");
				RETURN_THROWS();
			}
			entry_obj->entry->old_flags = entry_obj->entry->flags;
			entry_obj->entry->flags &= ~CRXA_ENT_COMPRESSION_MASK;
			entry_obj->entry->flags |= CRXA_ENT_COMPRESSED_BZ2;
			break;
		default:
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Unknown compression type specified");
			RETURN_THROWS();
	}

	entry_obj->entry->crxa->is_modified = 1;
	entry_obj->entry->is_modified = 1;
	crxa_flush(entry_obj->entry->crxa, 0, 0, 0, &error);

	if (error) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
		efree(error);
		RETURN_THROWS();
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Instructs the Crxa class to decompress the current file */
CRX_METHOD(CrxaFileInfo, decompress)
{
	char *error;
	char *compression_type;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRXA_ENTRY_OBJECT();

	if (entry_obj->entry->is_dir) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0, \
			"Crxa entry is a directory, cannot set compression"); \
		RETURN_THROWS();
	}

	if ((entry_obj->entry->flags & CRXA_ENT_COMPRESSION_MASK) == 0) {
		RETURN_TRUE;
	}

	if (CRXA_G(readonly) && !entry_obj->entry->crxa->is_data) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
			"Crxa is readonly, cannot decompress");
		RETURN_THROWS();
	}

	if (entry_obj->entry->is_deleted) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
			"Cannot compress deleted file");
		RETURN_THROWS();
	}

	if ((entry_obj->entry->flags & CRXA_ENT_COMPRESSED_GZ) != 0 && !CRXA_G(has_zlib)) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
			"Cannot decompress Gzip-compressed file, zlib extension is not enabled");
		RETURN_THROWS();
	}

	if ((entry_obj->entry->flags & CRXA_ENT_COMPRESSED_BZ2) != 0 && !CRXA_G(has_bz2)) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
			"Cannot decompress Bzip2-compressed file, bz2 extension is not enabled");
		RETURN_THROWS();
	}

	if (entry_obj->entry->is_persistent) {
		crxa_archive_data *crxa = entry_obj->entry->crxa;

		if (FAILURE == crxa_copy_on_write(&crxa)) {
			crex_throw_exception_ex(crxa_ce_CrxaException, 0, "crxa \"%s\" is persistent, unable to copy on write", crxa->fname);
			RETURN_THROWS();
		}
		/* re-populate after copy-on-write */
		entry_obj->entry = crex_hash_str_find_ptr(&crxa->manifest, entry_obj->entry->filename, entry_obj->entry->filename_len);
	}
	switch (entry_obj->entry->flags & CRXA_ENT_COMPRESSION_MASK) {
		case CRXA_ENT_COMPRESSED_GZ:
			compression_type = "gzip";
			break;
		case CRXA_ENT_COMPRESSED_BZ2:
			compression_type = "bz2";
			break;
		default:
			crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
				"Cannot decompress file compressed with unknown compression type");
			RETURN_THROWS();
	}
	/* decompress this file indirectly */
	if (SUCCESS != crxa_open_entry_fp(entry_obj->entry, &error, 1)) {
		crex_throw_exception_ex(spl_ce_BadMethodCallException, 0,
			"Crxa error: Cannot decompress %s-compressed file \"%s\" in crxa \"%s\": %s", compression_type, entry_obj->entry->filename, entry_obj->entry->crxa->fname, error);
		efree(error);
		RETURN_THROWS();
	}

	entry_obj->entry->old_flags = entry_obj->entry->flags;
	entry_obj->entry->flags &= ~CRXA_ENT_COMPRESSION_MASK;
	entry_obj->entry->crxa->is_modified = 1;
	entry_obj->entry->is_modified = 1;
	crxa_flush(entry_obj->entry->crxa, 0, 0, 0, &error);

	if (error) {
		crex_throw_exception_ex(crxa_ce_CrxaException, 0, "%s", error);
		efree(error);
		RETURN_THROWS();
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ crxa methods */

void crxa_object_init(void) /* {{{ */
{
	crxa_ce_CrxaException = register_class_CrxaException(crex_ce_exception);

	crxa_ce_archive = register_class_Crxa(spl_ce_RecursiveDirectoryIterator, crex_ce_countable, crex_ce_arrayaccess);

	crxa_ce_data = register_class_CrxaData(spl_ce_RecursiveDirectoryIterator, crex_ce_countable, crex_ce_arrayaccess);

	crxa_ce_entry = register_class_CrxaFileInfo(spl_ce_SplFileInfo);
}
/* }}} */
