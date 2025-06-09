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
  +----------------------------------------------------------------------+
*/

#include "crxa_internal.h"

#define CRXA_FUNC(name) \
	static CRX_NAMED_FUNCTION(name)

CRXA_FUNC(crxa_opendir) /* {{{ */
{
	char *filename;
	size_t filename_len;
	zval *zcontext = NULL;

	if (!CRXA_G(intercepted)) {
		goto skip_crxa;
	}

	if ((HT_IS_INITIALIZED(&CRXA_G(crxa_fname_map)) && !crex_hash_num_elements(&(CRXA_G(crxa_fname_map))))
		&& !HT_IS_INITIALIZED(&cached_crxas)) {
		goto skip_crxa;
	}

	if (crex_parse_parameters(CREX_NUM_ARGS(), "p|r!", &filename, &filename_len, &zcontext) == FAILURE) {
		RETURN_THROWS();
	}

	if (!IS_ABSOLUTE_PATH(filename, filename_len) && !strstr(filename, "://")) {
		char *arch, *entry;
		size_t arch_len, entry_len;
		crex_string *fname = crex_get_executed_filename_ex();

		/* we are checking for existence of a file within the relative path.  Chances are good that this is
		   retrieving something from within the crxa archive */
		if (!fname || !crex_string_starts_with_literal_ci(fname, "crxa://")) {
			goto skip_crxa;
		}

		if (SUCCESS == crxa_split_fname(ZSTR_VAL(fname), ZSTR_LEN(fname), &arch, &arch_len, &entry, &entry_len, 2, 0)) {
			crx_stream_context *context = NULL;
			crx_stream *stream;
			char *name;

			efree(entry);
			entry = estrndup(filename, filename_len);
			/* fopen within crxa, if :// is not in the url, then prepend crxa://<archive>/ */
			entry_len = filename_len;
			/* retrieving a file within the current directory, so use this if possible */
			entry = crxa_fix_filepath(entry, &entry_len, 1);

			if (entry[0] == '/') {
				spprintf(&name, 4096, "crxa://%s%s", arch, entry);
			} else {
				spprintf(&name, 4096, "crxa://%s/%s", arch, entry);
			}
			efree(entry);
			efree(arch);
			if (zcontext) {
				context = crx_stream_context_from_zval(zcontext, 0);
			}
			stream = crx_stream_opendir(name, REPORT_ERRORS, context);
			efree(name);
			if (!stream) {
				RETURN_FALSE;
			}
			crx_stream_to_zval(stream, return_value);
			return;
		}
	}
skip_crxa:
	CRXA_G(orig_opendir)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	return;
}
/* }}} */

static crex_string* crxa_get_name_for_relative_paths(crex_string *filename, bool using_include_path)
{
	char *arch, *entry;
	size_t arch_len, entry_len;
	crex_string *fname = crex_get_executed_filename_ex();

	/* we are checking for existence of a file within the relative path.  Chances are good that this is
	   retrieving something from within the crxa archive */
	if (!fname || !crex_string_starts_with_literal_ci(fname, "crxa://")) {
		return NULL;
	}

	if (FAILURE == crxa_split_fname(ZSTR_VAL(fname), ZSTR_LEN(fname), &arch, &arch_len, &entry, &entry_len, 2, 0)) {
		return NULL;
	}

	efree(entry);
	entry = NULL;
	entry_len = 0;
	/* fopen within crxa, if :// is not in the url, then prepend crxa://<archive>/ */
	/* retrieving a file defaults to within the current directory, so use this if possible */
	crxa_archive_data *crxa;
	if (FAILURE == crxa_get_archive(&crxa, arch, arch_len, NULL, 0, NULL)) {
		efree(arch);
		return NULL;
	}

	crex_string *name = NULL;
	if (using_include_path) {
		if (!(name = crxa_find_in_include_path(filename, NULL))) {
			/* this file is not in the crxa, use the original path */
			efree(arch);
			return NULL;
		}
	} else {
		entry_len = ZSTR_LEN(filename);
		entry = crxa_fix_filepath(estrndup(ZSTR_VAL(filename), ZSTR_LEN(filename)), &entry_len, 1);
		if (entry[0] == '/') {
			if (!crex_hash_str_exists(&(crxa->manifest), entry + 1, entry_len - 1)) {
				/* this file is not in the crxa, use the original path */
notfound:
				efree(entry);
				efree(arch);
				return NULL;
			}
		} else {
			if (!crex_hash_str_exists(&(crxa->manifest), entry, entry_len)) {
				goto notfound;
			}
		}
		/* auto-convert to crxa:// */
		if (entry[0] == '/') {
			CREX_ASSERT(strlen("crxa://") + arch_len + entry_len < 4096);
			name = crex_string_concat3(
				"crxa://", strlen("crxa://"),
				arch, arch_len,
				entry, entry_len
			);
		} else {
			name = strpprintf(4096, "crxa://%s/%s", arch, entry);
		}
		efree(entry);
	}

	efree(arch);
	return name;
}

CRXA_FUNC(crxa_file_get_contents) /* {{{ */
{
	crex_string *filename;
	crex_string *contents;
	bool use_include_path = 0;
	crex_long offset = -1;
	crex_long maxlen;
	bool maxlen_is_null = 1;
	zval *zcontext = NULL;

	if (!CRXA_G(intercepted)) {
		goto skip_crxa;
	}

	if ((HT_IS_INITIALIZED(&CRXA_G(crxa_fname_map)) && !crex_hash_num_elements(&(CRXA_G(crxa_fname_map))))
		&& !HT_IS_INITIALIZED(&cached_crxas)) {
		goto skip_crxa;
	}

	/* Parse arguments */
	if (crex_parse_parameters_ex(CREX_PARSE_PARAMS_QUIET, CREX_NUM_ARGS(), "P|br!ll!", &filename, &use_include_path, &zcontext, &offset, &maxlen, &maxlen_is_null) == FAILURE) {
		goto skip_crxa;
	}

	if (maxlen_is_null) {
		maxlen = (ssize_t) CRX_STREAM_COPY_ALL;
	} else if (maxlen < 0) {
		crex_argument_value_error(5, "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	if (use_include_path || (!IS_ABSOLUTE_PATH(ZSTR_VAL(filename), ZSTR_LEN(filename)) && !strstr(ZSTR_VAL(filename), "://"))) {
		crex_string *name = crxa_get_name_for_relative_paths(filename, use_include_path);
		if (!name) {
			goto skip_crxa;
		}

		crx_stream_context *context = NULL;
		crx_stream *stream;

		if (zcontext) {
			context = crx_stream_context_from_zval(zcontext, 0);
		}
		stream = crx_stream_open_wrapper_ex(ZSTR_VAL(name), "rb", 0 | REPORT_ERRORS, NULL, context);

		crex_string_release_ex(name, false);

		if (!stream) {
			RETURN_FALSE;
		}

		if (offset > 0 && crx_stream_seek(stream, offset, SEEK_SET) < 0) {
			crx_error_docref(NULL, E_WARNING, "Failed to seek to position " CREX_LONG_FMT " in the stream", offset);
			crx_stream_close(stream);
			RETURN_FALSE;
		}

		/* uses mmap if possible */
		contents = crx_stream_copy_to_mem(stream, maxlen, 0);
		if (contents && ZSTR_LEN(contents) > 0) {
			RETVAL_STR(contents);
		} else if (contents) {
			crex_string_release_ex(contents, 0);
			RETVAL_EMPTY_STRING();
		} else {
			RETVAL_FALSE;
		}

		crx_stream_close(stream);
		return;
	}
skip_crxa:
	CRXA_G(orig_file_get_contents)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	return;
}
/* }}} */

CRXA_FUNC(crxa_readfile) /* {{{ */
{
	crex_string *filename;
	bool use_include_path = 0;
	zval *zcontext = NULL;

	if (!CRXA_G(intercepted)) {
		goto skip_crxa;
	}

	if ((HT_IS_INITIALIZED(&CRXA_G(crxa_fname_map)) && !crex_hash_num_elements(&(CRXA_G(crxa_fname_map))))
		&& !HT_IS_INITIALIZED(&cached_crxas)) {
		goto skip_crxa;
	}
	if (crex_parse_parameters_ex(CREX_PARSE_PARAMS_QUIET, CREX_NUM_ARGS(), "P|br!", &filename, &use_include_path, &zcontext) == FAILURE) {
		goto skip_crxa;
	}
	if (use_include_path || (!IS_ABSOLUTE_PATH(ZSTR_VAL(filename), ZSTR_LEN(filename)) && !strstr(ZSTR_VAL(filename), "://"))) {
		crex_string *name = crxa_get_name_for_relative_paths(filename, use_include_path);
		if (!name) {
			goto skip_crxa;
		}

		crx_stream *stream;
		crx_stream_context *context = crx_stream_context_from_zval(zcontext, 0);

		stream = crx_stream_open_wrapper_ex(ZSTR_VAL(name), "rb", 0 | REPORT_ERRORS, NULL, context);

		crex_string_release_ex(name, false);
		if (stream == NULL) {
			RETURN_FALSE;
		}
		ssize_t size = crx_stream_passthru(stream);
		crx_stream_close(stream);
		RETURN_LONG(size);
	}

skip_crxa:
	CRXA_G(orig_readfile)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	return;

}
/* }}} */

CRXA_FUNC(crxa_fopen) /* {{{ */
{
	crex_string *filename;
	char *mode;
	size_t mode_len;
	bool use_include_path = 0;
	zval *zcontext = NULL;

	if (!CRXA_G(intercepted)) {
		goto skip_crxa;
	}

	if ((HT_IS_INITIALIZED(&CRXA_G(crxa_fname_map)) && !crex_hash_num_elements(&(CRXA_G(crxa_fname_map))))
		&& !HT_IS_INITIALIZED(&cached_crxas)) {
		/* no need to check, include_path not even specified in fopen/ no active crxas */
		goto skip_crxa;
	}
	if (crex_parse_parameters_ex(CREX_PARSE_PARAMS_QUIET, CREX_NUM_ARGS(), "Ps|br!", &filename, &mode, &mode_len, &use_include_path, &zcontext) == FAILURE) {
		goto skip_crxa;
	}
	if (use_include_path || (!IS_ABSOLUTE_PATH(ZSTR_VAL(filename), ZSTR_LEN(filename)) && !strstr(ZSTR_VAL(filename), "://"))) {
		crex_string *name = crxa_get_name_for_relative_paths(filename, use_include_path);
		if (!name) {
			goto skip_crxa;
		}

		crx_stream *stream;
		crx_stream_context *context = crx_stream_context_from_zval(zcontext, 0);

		stream = crx_stream_open_wrapper_ex(ZSTR_VAL(name), mode, 0 | REPORT_ERRORS, NULL, context);

		crex_string_release_ex(name, false);
		if (stream == NULL) {
			RETURN_FALSE;
		}
		crx_stream_to_zval(stream, return_value);
		if (zcontext) {
			C_ADDREF_P(zcontext);
		}
		return;
	}
skip_crxa:
	CRXA_G(orig_fopen)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	return;
}
/* }}} */

#define IS_LINK_OPERATION(__t) ((__t) == FS_TYPE || (__t) == FS_IS_LINK || (__t) == FS_LSTAT)
#define IS_EXISTS_CHECK(__t) ((__t) == FS_EXISTS  || (__t) == FS_IS_W || (__t) == FS_IS_R || (__t) == FS_IS_X || (__t) == FS_IS_FILE || (__t) == FS_IS_DIR || (__t) == FS_IS_LINK)
#define IS_ABLE_CHECK(__t) ((__t) == FS_IS_R || (__t) == FS_IS_W || (__t) == FS_IS_X)
#define IS_ACCESS_CHECK(__t) (IS_ABLE_CHECK(type) || (__t) == FS_EXISTS)

/* {{{ crx_stat */
static void crxa_fancy_stat(crex_stat_t *stat_sb, int type, zval *return_value)
{
	zval stat_dev, stat_ino, stat_mode, stat_nlink, stat_uid, stat_gid, stat_rdev,
		 stat_size, stat_atime, stat_mtime, stat_ctime, stat_blksize, stat_blocks;
	int rmask=S_IROTH, wmask=S_IWOTH, xmask=S_IXOTH; /* access rights defaults to other */
	char *stat_sb_names[13] = {
		"dev", "ino", "mode", "nlink", "uid", "gid", "rdev",
		"size", "atime", "mtime", "ctime", "blksize", "blocks"
	};

	if (type >= FS_IS_W && type <= FS_IS_X) {
		if(stat_sb->st_uid==getuid()) {
			rmask=S_IRUSR;
			wmask=S_IWUSR;
			xmask=S_IXUSR;
		} else if(stat_sb->st_gid==getgid()) {
			rmask=S_IRGRP;
			wmask=S_IWGRP;
			xmask=S_IXGRP;
		} else {
			int   groups, n, i;
			gid_t *gids;

			groups = getgroups(0, NULL);
			if(groups > 0) {
				gids=(gid_t *)safe_emalloc(groups, sizeof(gid_t), 0);
				n=getgroups(groups, gids);
				for(i=0;i<n;++i){
					if(stat_sb->st_gid==gids[i]) {
						rmask=S_IRGRP;
						wmask=S_IWGRP;
						xmask=S_IXGRP;
						break;
					}
				}
				efree(gids);
			}
		}
	}

	switch (type) {
	case FS_PERMS:
		RETURN_LONG((crex_long)stat_sb->st_mode);
	case FS_INODE:
		RETURN_LONG((crex_long)stat_sb->st_ino);
	case FS_SIZE:
		RETURN_LONG((crex_long)stat_sb->st_size);
	case FS_OWNER:
		RETURN_LONG((crex_long)stat_sb->st_uid);
	case FS_GROUP:
		RETURN_LONG((crex_long)stat_sb->st_gid);
	case FS_ATIME:
		RETURN_LONG((crex_long)stat_sb->st_atime);
	case FS_MTIME:
		RETURN_LONG((crex_long)stat_sb->st_mtime);
	case FS_CTIME:
		RETURN_LONG((crex_long)stat_sb->st_ctime);
	case FS_TYPE:
		if (S_ISLNK(stat_sb->st_mode)) {
			RETURN_STRING("link");
		}
		switch(stat_sb->st_mode & S_IFMT) {
		case S_IFDIR: RETURN_STRING("dir");
		case S_IFREG: RETURN_STRING("file");
		}
		crx_error_docref(NULL, E_NOTICE, "Unknown file type (%u)", stat_sb->st_mode & S_IFMT);
		RETURN_STRING("unknown");
	case FS_IS_W:
		RETURN_BOOL((stat_sb->st_mode & wmask) != 0);
	case FS_IS_R:
		RETURN_BOOL((stat_sb->st_mode&rmask)!=0);
	case FS_IS_X:
		RETURN_BOOL((stat_sb->st_mode&xmask)!=0 && !S_ISDIR(stat_sb->st_mode));
	case FS_IS_FILE:
		RETURN_BOOL(S_ISREG(stat_sb->st_mode));
	case FS_IS_DIR:
		RETURN_BOOL(S_ISDIR(stat_sb->st_mode));
	case FS_IS_LINK:
		RETURN_BOOL(S_ISLNK(stat_sb->st_mode));
	case FS_EXISTS:
		RETURN_TRUE; /* the false case was done earlier */
	case FS_LSTAT:
		/* FALLTHROUGH */
	case FS_STAT:
		array_init(return_value);

		ZVAL_LONG(&stat_dev, stat_sb->st_dev);
		ZVAL_LONG(&stat_ino, stat_sb->st_ino);
		ZVAL_LONG(&stat_mode, stat_sb->st_mode);
		ZVAL_LONG(&stat_nlink, stat_sb->st_nlink);
		ZVAL_LONG(&stat_uid, stat_sb->st_uid);
		ZVAL_LONG(&stat_gid, stat_sb->st_gid);
#ifdef HAVE_STRUCT_STAT_ST_RDEV
		ZVAL_LONG(&stat_rdev, stat_sb->st_rdev);
#else
		ZVAL_LONG(&stat_rdev, -1);
#endif
		ZVAL_LONG(&stat_size, stat_sb->st_size);
		ZVAL_LONG(&stat_atime, stat_sb->st_atime);
		ZVAL_LONG(&stat_mtime, stat_sb->st_mtime);
		ZVAL_LONG(&stat_ctime, stat_sb->st_ctime);
#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
		ZVAL_LONG(&stat_blksize, stat_sb->st_blksize);
#else
		ZVAL_LONG(&stat_blksize,-1);
#endif
#ifdef HAVE_STRUCT_STAT_ST_BLOCKS
		ZVAL_LONG(&stat_blocks, stat_sb->st_blocks);
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
		crex_hash_str_update(C_ARRVAL_P(return_value), stat_sb_names[0], strlen(stat_sb_names[0]), &stat_dev);
		crex_hash_str_update(C_ARRVAL_P(return_value), stat_sb_names[1], strlen(stat_sb_names[1]), &stat_ino);
		crex_hash_str_update(C_ARRVAL_P(return_value), stat_sb_names[2], strlen(stat_sb_names[2]), &stat_mode);
		crex_hash_str_update(C_ARRVAL_P(return_value), stat_sb_names[3], strlen(stat_sb_names[3]), &stat_nlink);
		crex_hash_str_update(C_ARRVAL_P(return_value), stat_sb_names[4], strlen(stat_sb_names[4]), &stat_uid);
		crex_hash_str_update(C_ARRVAL_P(return_value), stat_sb_names[5], strlen(stat_sb_names[5]), &stat_gid);
		crex_hash_str_update(C_ARRVAL_P(return_value), stat_sb_names[6], strlen(stat_sb_names[6]), &stat_rdev);
		crex_hash_str_update(C_ARRVAL_P(return_value), stat_sb_names[7], strlen(stat_sb_names[7]), &stat_size);
		crex_hash_str_update(C_ARRVAL_P(return_value), stat_sb_names[8], strlen(stat_sb_names[8]), &stat_atime);
		crex_hash_str_update(C_ARRVAL_P(return_value), stat_sb_names[9], strlen(stat_sb_names[9]), &stat_mtime);
		crex_hash_str_update(C_ARRVAL_P(return_value), stat_sb_names[10], strlen(stat_sb_names[10]), &stat_ctime);
		crex_hash_str_update(C_ARRVAL_P(return_value), stat_sb_names[11], strlen(stat_sb_names[11]), &stat_blksize);
		crex_hash_str_update(C_ARRVAL_P(return_value), stat_sb_names[12], strlen(stat_sb_names[12]), &stat_blocks);

		return;
	}
	crx_error_docref(NULL, E_WARNING, "Didn't understand stat call");
	RETURN_FALSE;
}
/* }}} */

static void crxa_file_stat(const char *filename, size_t filename_length, int type, zif_handler orig_stat_func, INTERNAL_FUNCTION_PARAMETERS) /* {{{ */
{
	if (!filename_length) {
		RETURN_FALSE;
	}

	if (!IS_ABSOLUTE_PATH(filename, filename_length) && !strstr(filename, "://")) {
		char *arch, *entry;
		size_t arch_len, entry_len;
		crex_string *fname;
		crex_stat_t sb = {0};
		crxa_entry_info *data = NULL;
		crxa_archive_data *crxa;

		fname = crex_get_executed_filename_ex();

		/* we are checking for existence of a file within the relative path.  Chances are good that this is
		   retrieving something from within the crxa archive */
		if (!fname || !crex_string_starts_with_literal_ci(fname, "crxa://")) {
			goto skip_crxa;
		}

		if (CRXA_G(last_crxa) && ZSTR_LEN(fname) - 7 >= CRXA_G(last_crxa_name_len) && !memcmp(ZSTR_VAL(fname) + 7, CRXA_G(last_crxa_name), CRXA_G(last_crxa_name_len))) {
			arch = estrndup(CRXA_G(last_crxa_name), CRXA_G(last_crxa_name_len));
			arch_len = CRXA_G(last_crxa_name_len);
			entry = estrndup(filename, filename_length);
			/* fopen within crxa, if :// is not in the url, then prepend crxa://<archive>/ */
			entry_len = filename_length;
			crxa = CRXA_G(last_crxa);
			goto splitted;
		}
		if (SUCCESS == crxa_split_fname(ZSTR_VAL(fname), ZSTR_LEN(fname), &arch, &arch_len, &entry, &entry_len, 2, 0)) {

			efree(entry);
			entry = estrndup(filename, filename_length);
			/* fopen within crxa, if :// is not in the url, then prepend crxa://<archive>/ */
			entry_len = filename_length;
			if (FAILURE == crxa_get_archive(&crxa, arch, arch_len, NULL, 0, NULL)) {
				efree(arch);
				efree(entry);
				goto skip_crxa;
			}
splitted:
			entry = crxa_fix_filepath(entry, &entry_len, 1);
			if (entry[0] == '/') {
				if (NULL != (data = crex_hash_str_find_ptr(&(crxa->manifest), entry + 1, entry_len - 1))) {
					efree(entry);
					goto stat_entry;
				}
				goto notfound;
			}
			if (NULL != (data = crex_hash_str_find_ptr(&(crxa->manifest), entry, entry_len))) {
				efree(entry);
				goto stat_entry;
			}
			if (crex_hash_str_exists(&(crxa->virtual_dirs), entry, entry_len)) {
				efree(entry);
				efree(arch);
				if (IS_EXISTS_CHECK(type)) {
					RETURN_TRUE;
				}
				sb.st_size = 0;
				sb.st_mode = 0777;
				sb.st_mode |= S_IFDIR; /* regular directory */
				sb.st_mtime = crxa->max_timestamp;
				sb.st_atime = crxa->max_timestamp;
				sb.st_ctime = crxa->max_timestamp;
				goto statme_baby;
			} else {
				char *save;
				size_t save_len;

notfound:
				efree(entry);
				save = CRXA_G(cwd);
				save_len = CRXA_G(cwd_len);
				/* this file is not in the current directory, use the original path */
				entry = estrndup(filename, filename_length);
				entry_len = filename_length;
				CRXA_G(cwd) = "/";
				CRXA_G(cwd_len) = 0;
				/* clean path without cwd */
				entry = crxa_fix_filepath(entry, &entry_len, 1);
				if (NULL != (data = crex_hash_str_find_ptr(&(crxa->manifest), entry + 1, entry_len - 1))) {
					CRXA_G(cwd) = save;
					CRXA_G(cwd_len) = save_len;
					efree(entry);
					if (IS_EXISTS_CHECK(type)) {
						efree(arch);
						RETURN_TRUE;
					}
					goto stat_entry;
				}
				if (crex_hash_str_exists(&(crxa->virtual_dirs), entry + 1, entry_len - 1)) {
					CRXA_G(cwd) = save;
					CRXA_G(cwd_len) = save_len;
					efree(entry);
					efree(arch);
					if (IS_EXISTS_CHECK(type)) {
						RETURN_TRUE;
					}
					sb.st_size = 0;
					sb.st_mode = 0777;
					sb.st_mode |= S_IFDIR; /* regular directory */
					sb.st_mtime = crxa->max_timestamp;
					sb.st_atime = crxa->max_timestamp;
					sb.st_ctime = crxa->max_timestamp;
					goto statme_baby;
				}
				CRXA_G(cwd) = save;
				CRXA_G(cwd_len) = save_len;
				efree(entry);
				efree(arch);
				/* Error Occurred */
				if (!IS_EXISTS_CHECK(type)) {
					crx_error_docref(NULL, E_WARNING, "%sstat failed for %s", IS_LINK_OPERATION(type) ? "L" : "", filename);
				}
				RETURN_FALSE;
			}
stat_entry:
			efree(arch);
			if (!data->is_dir) {
				sb.st_size = data->uncompressed_filesize;
				sb.st_mode = data->flags & CRXA_ENT_PERM_MASK;
				if (data->link) {
					sb.st_mode |= S_IFREG|S_IFLNK; /* regular file */
				} else {
					sb.st_mode |= S_IFREG; /* regular file */
				}
				/* timestamp is just the timestamp when this was added to the crxa */
				sb.st_mtime = data->timestamp;
				sb.st_atime = data->timestamp;
				sb.st_ctime = data->timestamp;
			} else {
				sb.st_size = 0;
				sb.st_mode = data->flags & CRXA_ENT_PERM_MASK;
				sb.st_mode |= S_IFDIR; /* regular directory */
				if (data->link) {
					sb.st_mode |= S_IFLNK;
				}
				/* timestamp is just the timestamp when this was added to the crxa */
				sb.st_mtime = data->timestamp;
				sb.st_atime = data->timestamp;
				sb.st_ctime = data->timestamp;
			}

statme_baby:
			if (!crxa->is_writeable) {
				sb.st_mode = (sb.st_mode & 0555) | (sb.st_mode & ~0777);
			}

			sb.st_nlink = 1;
			sb.st_rdev = -1;
			/* this is only for APC, so use /dev/null device - no chance of conflict there! */
			sb.st_dev = 0xc;
			/* generate unique inode number for alias/filename, so no crxas will conflict */
			if (data) {
				sb.st_ino = data->inode;
			}
#ifndef CRX_WIN32
			sb.st_blksize = -1;
			sb.st_blocks = -1;
#endif
			crxa_fancy_stat(&sb, type, return_value);
			return;
		}
	}
skip_crxa:
	orig_stat_func(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	return;
}
/* }}} */

#define CrxaFileFunction(fname, funcnum, orig) \
CREX_NAMED_FUNCTION(fname) { \
	if (!CRXA_G(intercepted)) { \
		CRXA_G(orig)(INTERNAL_FUNCTION_PARAM_PASSTHRU); \
	} else { \
		char *filename; \
		size_t filename_len; \
		\
		if (crex_parse_parameters(CREX_NUM_ARGS(), "p", &filename, &filename_len) == FAILURE) { \
			RETURN_THROWS(); \
		} \
		\
		crxa_file_stat(filename, filename_len, funcnum, CRXA_G(orig), INTERNAL_FUNCTION_PARAM_PASSTHRU); \
	} \
}
/* }}} */

/* {{{ Get file permissions */
CrxaFileFunction(crxa_fileperms, FS_PERMS, orig_fileperms)
/* }}} */

/* {{{ Get file inode */
CrxaFileFunction(crxa_fileinode, FS_INODE, orig_fileinode)
/* }}} */

/* {{{ Get file size */
CrxaFileFunction(crxa_filesize, FS_SIZE, orig_filesize)
/* }}} */

/* {{{ Get file owner */
CrxaFileFunction(crxa_fileowner, FS_OWNER, orig_fileowner)
/* }}} */

/* {{{ Get file group */
CrxaFileFunction(crxa_filegroup, FS_GROUP, orig_filegroup)
/* }}} */

/* {{{ Get last access time of file */
CrxaFileFunction(crxa_fileatime, FS_ATIME, orig_fileatime)
/* }}} */

/* {{{ Get last modification time of file */
CrxaFileFunction(crxa_filemtime, FS_MTIME, orig_filemtime)
/* }}} */

/* {{{ Get inode modification time of file */
CrxaFileFunction(crxa_filectime, FS_CTIME, orig_filectime)
/* }}} */

/* {{{ Get file type */
CrxaFileFunction(crxa_filetype, FS_TYPE, orig_filetype)
/* }}} */

/* {{{ Returns true if file can be written */
CrxaFileFunction(crxa_is_writable, FS_IS_W, orig_is_writable)
/* }}} */

/* {{{ Returns true if file can be read */
CrxaFileFunction(crxa_is_readable, FS_IS_R, orig_is_readable)
/* }}} */

/* {{{ Returns true if file is executable */
CrxaFileFunction(crxa_is_executable, FS_IS_X, orig_is_executable)
/* }}} */

/* {{{ Returns true if filename exists */
CrxaFileFunction(crxa_file_exists, FS_EXISTS, orig_file_exists)
/* }}} */

/* {{{ Returns true if file is directory */
CrxaFileFunction(crxa_is_dir, FS_IS_DIR, orig_is_dir)
/* }}} */

CRXA_FUNC(crxa_is_file) /* {{{ */
{
	char *filename;
	size_t filename_len;

	if (!CRXA_G(intercepted)) {
		goto skip_crxa;
	}

	if ((HT_IS_INITIALIZED(&CRXA_G(crxa_fname_map)) && !crex_hash_num_elements(&(CRXA_G(crxa_fname_map))))
		&& !HT_IS_INITIALIZED(&cached_crxas)) {
		goto skip_crxa;
	}
	if (crex_parse_parameters_ex(CREX_PARSE_PARAMS_QUIET, CREX_NUM_ARGS(), "p", &filename, &filename_len) == FAILURE) {
		goto skip_crxa;
	}
	if (!IS_ABSOLUTE_PATH(filename, filename_len) && !strstr(filename, "://")) {
		char *arch, *entry;
		size_t arch_len, entry_len;
		crex_string *fname = crex_get_executed_filename_ex();

		/* we are checking for existence of a file within the relative path.  Chances are good that this is
		   retrieving something from within the crxa archive */
		if (!fname || !crex_string_starts_with_literal_ci(fname, "crxa://")) {
			goto skip_crxa;
		}

		if (SUCCESS == crxa_split_fname(ZSTR_VAL(fname), ZSTR_LEN(fname), &arch, &arch_len, &entry, &entry_len, 2, 0)) {
			crxa_archive_data *crxa;

			efree(entry);
			entry = filename;
			/* fopen within crxa, if :// is not in the url, then prepend crxa://<archive>/ */
			entry_len = filename_len;
			/* retrieving a file within the current directory, so use this if possible */
			if (SUCCESS == crxa_get_archive(&crxa, arch, arch_len, NULL, 0, NULL)) {
				crxa_entry_info *etemp;

				entry = crxa_fix_filepath(estrndup(entry, entry_len), &entry_len, 1);
				if (entry[0] == '/') {
					if (NULL != (etemp = crex_hash_str_find_ptr(&(crxa->manifest), entry + 1, entry_len - 1))) {
						/* this file is not in the current directory, use the original path */
found_it:
						efree(entry);
						efree(arch);
						RETURN_BOOL(!etemp->is_dir);
					}
				} else {
					if (NULL != (etemp = crex_hash_str_find_ptr(&(crxa->manifest), entry, entry_len))) {
						goto found_it;
					}
				}
			}
			if (entry != filename) {
				efree(entry);
			}
			efree(arch);
			RETURN_FALSE;
		}
	}
skip_crxa:
	CRXA_G(orig_is_file)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	return;
}
/* }}} */

CRXA_FUNC(crxa_is_link) /* {{{ */
{
	char *filename;
	size_t filename_len;

	if (!CRXA_G(intercepted)) {
		goto skip_crxa;
	}

	if ((HT_IS_INITIALIZED(&CRXA_G(crxa_fname_map)) && !crex_hash_num_elements(&(CRXA_G(crxa_fname_map))))
		&& !HT_IS_INITIALIZED(&cached_crxas)) {
		goto skip_crxa;
	}
	if (crex_parse_parameters_ex(CREX_PARSE_PARAMS_QUIET, CREX_NUM_ARGS(), "p", &filename, &filename_len) == FAILURE) {
		goto skip_crxa;
	}
	if (!IS_ABSOLUTE_PATH(filename, filename_len) && !strstr(filename, "://")) {
		char *arch, *entry;
		size_t arch_len, entry_len;
		crex_string *fname = crex_get_executed_filename_ex();

		/* we are checking for existence of a file within the relative path.  Chances are good that this is
		   retrieving something from within the crxa archive */
		if (!fname || !crex_string_starts_with_literal_ci(fname, "crxa://")) {
			goto skip_crxa;
		}

		if (SUCCESS == crxa_split_fname(ZSTR_VAL(fname), ZSTR_LEN(fname), &arch, &arch_len, &entry, &entry_len, 2, 0)) {
			crxa_archive_data *crxa;

			efree(entry);
			entry = filename;
			/* fopen within crxa, if :// is not in the url, then prepend crxa://<archive>/ */
			entry_len = filename_len;
			/* retrieving a file within the current directory, so use this if possible */
			if (SUCCESS == crxa_get_archive(&crxa, arch, arch_len, NULL, 0, NULL)) {
				crxa_entry_info *etemp;

				entry = crxa_fix_filepath(estrndup(entry, entry_len), &entry_len, 1);
				if (entry[0] == '/') {
					if (NULL != (etemp = crex_hash_str_find_ptr(&(crxa->manifest), entry + 1, entry_len - 1))) {
						/* this file is not in the current directory, use the original path */
found_it:
						efree(entry);
						efree(arch);
						RETURN_BOOL(etemp->link);
					}
				} else {
					if (NULL != (etemp = crex_hash_str_find_ptr(&(crxa->manifest), entry, entry_len))) {
						goto found_it;
					}
				}
			}
			efree(entry);
			efree(arch);
			RETURN_FALSE;
		}
	}
skip_crxa:
	CRXA_G(orig_is_link)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	return;
}
/* }}} */

/* {{{ Give information about a file or symbolic link */
CrxaFileFunction(crxa_lstat, FS_LSTAT, orig_lstat)
/* }}} */

/* {{{ Give information about a file */
CrxaFileFunction(crxa_stat, FS_STAT, orig_stat)
/* }}} */

/* {{{ void crxa_intercept_functions(void) */
void crxa_intercept_functions(void)
{
	if (!CRXA_G(request_init)) {
		CRXA_G(cwd) = NULL;
		CRXA_G(cwd_len) = 0;
	}
	CRXA_G(intercepted) = 1;
}
/* }}} */

/* {{{ void crxa_release_functions(void) */
void crxa_release_functions(void)
{
	CRXA_G(intercepted) = 0;
}
/* }}} */

/* {{{ void crxa_intercept_functions_init(void) */
#define CRXA_INTERCEPT(func) \
	CRXA_G(orig_##func) = NULL; \
	if (NULL != (orig = crex_hash_str_find_ptr(CG(function_table), #func, sizeof(#func)-1))) { \
		CRXA_G(orig_##func) = orig->internal_function.handler; \
		orig->internal_function.handler = crxa_##func; \
	}

void crxa_intercept_functions_init(void)
{
	crex_function *orig;

	CRXA_INTERCEPT(fopen);
	CRXA_INTERCEPT(file_get_contents);
	CRXA_INTERCEPT(is_file);
	CRXA_INTERCEPT(is_link);
	CRXA_INTERCEPT(is_dir);
	CRXA_INTERCEPT(opendir);
	CRXA_INTERCEPT(file_exists);
	CRXA_INTERCEPT(fileperms);
	CRXA_INTERCEPT(fileinode);
	CRXA_INTERCEPT(filesize);
	CRXA_INTERCEPT(fileowner);
	CRXA_INTERCEPT(filegroup);
	CRXA_INTERCEPT(fileatime);
	CRXA_INTERCEPT(filemtime);
	CRXA_INTERCEPT(filectime);
	CRXA_INTERCEPT(filetype);
	CRXA_INTERCEPT(is_writable);
	CRXA_INTERCEPT(is_readable);
	CRXA_INTERCEPT(is_executable);
	CRXA_INTERCEPT(lstat);
	CRXA_INTERCEPT(stat);
	CRXA_INTERCEPT(readfile);
	CRXA_G(intercepted) = 0;
}
/* }}} */

/* {{{ void crxa_intercept_functions_shutdown(void) */
#define CRXA_RELEASE(func) \
	if (CRXA_G(orig_##func) && NULL != (orig = crex_hash_str_find_ptr(CG(function_table), #func, sizeof(#func)-1))) { \
		orig->internal_function.handler = CRXA_G(orig_##func); \
	} \
	CRXA_G(orig_##func) = NULL;

void crxa_intercept_functions_shutdown(void)
{
	crex_function *orig;

	CRXA_RELEASE(fopen);
	CRXA_RELEASE(file_get_contents);
	CRXA_RELEASE(is_file);
	CRXA_RELEASE(is_dir);
	CRXA_RELEASE(opendir);
	CRXA_RELEASE(file_exists);
	CRXA_RELEASE(fileperms);
	CRXA_RELEASE(fileinode);
	CRXA_RELEASE(filesize);
	CRXA_RELEASE(fileowner);
	CRXA_RELEASE(filegroup);
	CRXA_RELEASE(fileatime);
	CRXA_RELEASE(filemtime);
	CRXA_RELEASE(filectime);
	CRXA_RELEASE(filetype);
	CRXA_RELEASE(is_writable);
	CRXA_RELEASE(is_readable);
	CRXA_RELEASE(is_executable);
	CRXA_RELEASE(lstat);
	CRXA_RELEASE(stat);
	CRXA_RELEASE(readfile);
	CRXA_G(intercepted) = 0;
}
/* }}} */

static struct _crxa_orig_functions {
	zif_handler orig_fopen;
	zif_handler orig_file_get_contents;
	zif_handler orig_is_file;
	zif_handler orig_is_link;
	zif_handler orig_is_dir;
	zif_handler orig_opendir;
	zif_handler orig_file_exists;
	zif_handler orig_fileperms;
	zif_handler orig_fileinode;
	zif_handler orig_filesize;
	zif_handler orig_fileowner;
	zif_handler orig_filegroup;
	zif_handler orig_fileatime;
	zif_handler orig_filemtime;
	zif_handler orig_filectime;
	zif_handler orig_filetype;
	zif_handler orig_is_writable;
	zif_handler orig_is_readable;
	zif_handler orig_is_executable;
	zif_handler orig_lstat;
	zif_handler orig_readfile;
	zif_handler orig_stat;
} crxa_orig_functions = {0};

void crxa_save_orig_functions(void) /* {{{ */
{
	crxa_orig_functions.orig_fopen             = CRXA_G(orig_fopen);
	crxa_orig_functions.orig_file_get_contents = CRXA_G(orig_file_get_contents);
	crxa_orig_functions.orig_is_file           = CRXA_G(orig_is_file);
	crxa_orig_functions.orig_is_link           = CRXA_G(orig_is_link);
	crxa_orig_functions.orig_is_dir            = CRXA_G(orig_is_dir);
	crxa_orig_functions.orig_opendir           = CRXA_G(orig_opendir);
	crxa_orig_functions.orig_file_exists       = CRXA_G(orig_file_exists);
	crxa_orig_functions.orig_fileperms         = CRXA_G(orig_fileperms);
	crxa_orig_functions.orig_fileinode         = CRXA_G(orig_fileinode);
	crxa_orig_functions.orig_filesize          = CRXA_G(orig_filesize);
	crxa_orig_functions.orig_fileowner         = CRXA_G(orig_fileowner);
	crxa_orig_functions.orig_filegroup         = CRXA_G(orig_filegroup);
	crxa_orig_functions.orig_fileatime         = CRXA_G(orig_fileatime);
	crxa_orig_functions.orig_filemtime         = CRXA_G(orig_filemtime);
	crxa_orig_functions.orig_filectime         = CRXA_G(orig_filectime);
	crxa_orig_functions.orig_filetype          = CRXA_G(orig_filetype);
	crxa_orig_functions.orig_is_writable       = CRXA_G(orig_is_writable);
	crxa_orig_functions.orig_is_readable       = CRXA_G(orig_is_readable);
	crxa_orig_functions.orig_is_executable     = CRXA_G(orig_is_executable);
	crxa_orig_functions.orig_lstat             = CRXA_G(orig_lstat);
	crxa_orig_functions.orig_readfile          = CRXA_G(orig_readfile);
	crxa_orig_functions.orig_stat              = CRXA_G(orig_stat);
}
/* }}} */

void crxa_restore_orig_functions(void) /* {{{ */
{
	CRXA_G(orig_fopen)             = crxa_orig_functions.orig_fopen;
	CRXA_G(orig_file_get_contents) = crxa_orig_functions.orig_file_get_contents;
	CRXA_G(orig_is_file)           = crxa_orig_functions.orig_is_file;
	CRXA_G(orig_is_link)           = crxa_orig_functions.orig_is_link;
	CRXA_G(orig_is_dir)            = crxa_orig_functions.orig_is_dir;
	CRXA_G(orig_opendir)           = crxa_orig_functions.orig_opendir;
	CRXA_G(orig_file_exists)       = crxa_orig_functions.orig_file_exists;
	CRXA_G(orig_fileperms)         = crxa_orig_functions.orig_fileperms;
	CRXA_G(orig_fileinode)         = crxa_orig_functions.orig_fileinode;
	CRXA_G(orig_filesize)          = crxa_orig_functions.orig_filesize;
	CRXA_G(orig_fileowner)         = crxa_orig_functions.orig_fileowner;
	CRXA_G(orig_filegroup)         = crxa_orig_functions.orig_filegroup;
	CRXA_G(orig_fileatime)         = crxa_orig_functions.orig_fileatime;
	CRXA_G(orig_filemtime)         = crxa_orig_functions.orig_filemtime;
	CRXA_G(orig_filectime)         = crxa_orig_functions.orig_filectime;
	CRXA_G(orig_filetype)          = crxa_orig_functions.orig_filetype;
	CRXA_G(orig_is_writable)       = crxa_orig_functions.orig_is_writable;
	CRXA_G(orig_is_readable)       = crxa_orig_functions.orig_is_readable;
	CRXA_G(orig_is_executable)     = crxa_orig_functions.orig_is_executable;
	CRXA_G(orig_lstat)             = crxa_orig_functions.orig_lstat;
	CRXA_G(orig_readfile)          = crxa_orig_functions.orig_readfile;
	CRXA_G(orig_stat)              = crxa_orig_functions.orig_stat;
}
/* }}} */
