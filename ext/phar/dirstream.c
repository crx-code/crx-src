/*
  +----------------------------------------------------------------------+
  | crxa:// stream wrapper support                                       |
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

#define CRXA_DIRSTREAM 1
#include "crxa_internal.h"
#include "dirstream.h"

void crxa_dostat(crxa_archive_data *crxa, crxa_entry_info *data, crx_stream_statbuf *ssb, bool is_dir);

const crx_stream_ops crxa_dir_ops = {
	crxa_dir_write, /* write */
	crxa_dir_read,  /* read  */
	crxa_dir_close, /* close */
	crxa_dir_flush, /* flush */
	"crxa dir",
	crxa_dir_seek,  /* seek */
	NULL,           /* cast */
	NULL,           /* stat */
	NULL, /* set option */
};

/**
 * Used for closedir($fp) where $fp is an opendir('crxa://...') directory handle
 */
static int crxa_dir_close(crx_stream *stream, int close_handle)  /* {{{ */
{
	HashTable *data = (HashTable *)stream->abstract;

	if (data) {
		crex_hash_destroy(data);
		FREE_HASHTABLE(data);
		stream->abstract = NULL;
	}

	return 0;
}
/* }}} */

/**
 * Used for seeking on a crxa directory handle
 */
static int crxa_dir_seek(crx_stream *stream, crex_off_t offset, int whence, crex_off_t *newoffset) /* {{{ */
{
	HashTable *data = (HashTable *)stream->abstract;

	if (!data) {
		return -1;
	}

	if (whence == SEEK_END) {
		whence = SEEK_SET;
		offset = crex_hash_num_elements(data) + offset;
	}

	if (whence == SEEK_SET) {
		crex_hash_internal_pointer_reset(data);
	}

	if (offset < 0) {
		return -1;
	} else {
		*newoffset = 0;
		while (*newoffset < offset && crex_hash_move_forward(data) == SUCCESS) {
			++(*newoffset);
		}
		return 0;
	}
}
/* }}} */

/**
 * Used for readdir() on an opendir()ed crxa directory handle
 */
static ssize_t crxa_dir_read(crx_stream *stream, char *buf, size_t count) /* {{{ */
{
	HashTable *data = (HashTable *)stream->abstract;
	crex_string *str_key;
	crex_ulong unused;

	if (count != sizeof(crx_stream_dirent)) {
		return -1;
	}

	if (HASH_KEY_NON_EXISTENT == crex_hash_get_current_key(data, &str_key, &unused)) {
		return 0;
	}

	crex_hash_move_forward(data);

	crx_stream_dirent *dirent = (crx_stream_dirent *) buf;

	if (sizeof(dirent->d_name) <= ZSTR_LEN(str_key)) {
		return 0;
	}

	memset(dirent, 0, sizeof(crx_stream_dirent));
	CRX_STRLCPY(dirent->d_name, ZSTR_VAL(str_key), sizeof(dirent->d_name), ZSTR_LEN(str_key));

	return sizeof(crx_stream_dirent);
}
/* }}} */

/**
 * Dummy: Used for writing to a crxa directory (i.e. not used)
 */
static ssize_t crxa_dir_write(crx_stream *stream, const char *buf, size_t count) /* {{{ */
{
	return -1;
}
/* }}} */

/**
 * Dummy: Used for flushing writes to a crxa directory (i.e. not used)
 */
static int crxa_dir_flush(crx_stream *stream) /* {{{ */
{
	return EOF;
}
/* }}} */

/**
 * add an empty element with a char * key to a hash table, avoiding duplicates
 *
 * This is used to get a unique listing of virtual directories within a crxa,
 * for iterating over opendir()ed crxa directories.
 */
static int crxa_add_empty(HashTable *ht, char *arKey, uint32_t nKeyLength)  /* {{{ */
{
	zval dummy;

	ZVAL_NULL(&dummy);
	crex_hash_str_update(ht, arKey, nKeyLength, &dummy);
	return SUCCESS;
}
/* }}} */

/**
 * Used for sorting directories alphabetically
 */
static int crxa_compare_dir_name(Bucket *f, Bucket *s)  /* {{{ */
{
	int result = crex_binary_strcmp(
		ZSTR_VAL(f->key), ZSTR_LEN(f->key), ZSTR_VAL(s->key), ZSTR_LEN(s->key));
	return CREX_NORMALIZE_BOOL(result);
}
/* }}} */

/**
 * Create a opendir() directory stream handle by iterating over each of the
 * files in a crxa and retrieving its relative path.  From this, construct
 * a list of files/directories that are "in" the directory represented by dir
 */
static crx_stream *crxa_make_dirstream(char *dir, HashTable *manifest) /* {{{ */
{
	HashTable *data;
	size_t dirlen = strlen(dir);
	char *entry, *found, *save;
	crex_string *str_key;
	size_t keylen;
	crex_ulong unused;

	ALLOC_HASHTABLE(data);
	crex_hash_init(data, 64, NULL, NULL, 0);

	if ((*dir == '/' && dirlen == 1 && (manifest->nNumOfElements == 0)) || (dirlen >= sizeof(".crxa")-1 && !memcmp(dir, ".crxa", sizeof(".crxa")-1))) {
		/* make empty root directory for empty crxa */
		/* make empty directory for .crxa magic directory */
		efree(dir);
		return crx_stream_alloc(&crxa_dir_ops, data, NULL, "r");
	}

	crex_hash_internal_pointer_reset(manifest);

	while (FAILURE != crex_hash_has_more_elements(manifest)) {
		if (HASH_KEY_NON_EXISTENT == crex_hash_get_current_key(manifest, &str_key, &unused)) {
			break;
		}

		keylen = ZSTR_LEN(str_key);
		if (keylen <= dirlen) {
			if (keylen == 0 || keylen < dirlen || !strncmp(ZSTR_VAL(str_key), dir, dirlen)) {
				if (SUCCESS != crex_hash_move_forward(manifest)) {
					break;
				}
				continue;
			}
		}

		if (*dir == '/') {
			/* root directory */
			if (keylen >= sizeof(".crxa")-1 && !memcmp(ZSTR_VAL(str_key), ".crxa", sizeof(".crxa")-1)) {
				/* do not add any magic entries to this directory */
				if (SUCCESS != crex_hash_move_forward(manifest)) {
					break;
				}
				continue;
			}

			if (NULL != (found = (char *) memchr(ZSTR_VAL(str_key), '/', keylen))) {
				/* the entry has a path separator and is a subdirectory */
				entry = (char *) safe_emalloc(found - ZSTR_VAL(str_key), 1, 1);
				memcpy(entry, ZSTR_VAL(str_key), found - ZSTR_VAL(str_key));
				keylen = found - ZSTR_VAL(str_key);
				entry[keylen] = '\0';
			} else {
				entry = (char *) safe_emalloc(keylen, 1, 1);
				memcpy(entry, ZSTR_VAL(str_key), keylen);
				entry[keylen] = '\0';
			}

			goto CRXA_ADD_ENTRY;
		} else {
			if (0 != memcmp(ZSTR_VAL(str_key), dir, dirlen)) {
				/* entry in directory not found */
				if (SUCCESS != crex_hash_move_forward(manifest)) {
					break;
				}
				continue;
			} else {
				if (ZSTR_VAL(str_key)[dirlen] != '/') {
					if (SUCCESS != crex_hash_move_forward(manifest)) {
						break;
					}
					continue;
				}
			}
		}

		save = ZSTR_VAL(str_key);
		save += dirlen + 1; /* seek to just past the path separator */

		if (NULL != (found = (char *) memchr(save, '/', keylen - dirlen - 1))) {
			/* is subdirectory */
			save -= dirlen + 1;
			entry = (char *) safe_emalloc(found - save + dirlen, 1, 1);
			memcpy(entry, save + dirlen + 1, found - save - dirlen - 1);
			keylen = found - save - dirlen - 1;
			entry[keylen] = '\0';
		} else {
			/* is file */
			save -= dirlen + 1;
			entry = (char *) safe_emalloc(keylen - dirlen, 1, 1);
			memcpy(entry, save + dirlen + 1, keylen - dirlen - 1);
			entry[keylen - dirlen - 1] = '\0';
			keylen = keylen - dirlen - 1;
		}
CRXA_ADD_ENTRY:
		if (keylen) {
			crxa_add_empty(data, entry, keylen);
		}

		efree(entry);

		if (SUCCESS != crex_hash_move_forward(manifest)) {
			break;
		}
	}

	if (FAILURE != crex_hash_has_more_elements(data)) {
		efree(dir);
		crex_hash_sort(data, crxa_compare_dir_name, 0);
		return crx_stream_alloc(&crxa_dir_ops, data, NULL, "r");
	} else {
		efree(dir);
		return crx_stream_alloc(&crxa_dir_ops, data, NULL, "r");
	}
}
/* }}}*/

/**
 * Open a directory handle within a crxa archive
 */
crx_stream *crxa_wrapper_open_dir(crx_stream_wrapper *wrapper, const char *path, const char *mode, int options, crex_string **opened_path, crx_stream_context *context STREAMS_DC) /* {{{ */
{
	crx_url *resource = NULL;
	crx_stream *ret;
	char *internal_file, *error;
	crex_string *str_key;
	crex_ulong unused;
	crxa_archive_data *crxa;
	crxa_entry_info *entry = NULL;
	uint32_t host_len;

	if ((resource = crxa_parse_url(wrapper, path, mode, options)) == NULL) {
		crx_stream_wrapper_log_error(wrapper, options, "crxa url \"%s\" is unknown", path);
		return NULL;
	}

	/* we must have at the very least crxa://alias.crxa/ */
	if (!resource->scheme || !resource->host || !resource->path) {
		if (resource->host && !resource->path) {
			crx_stream_wrapper_log_error(wrapper, options, "crxa error: no directory in \"%s\", must have at least crxa://%s/ for root directory (always use full path to a new crxa)", path, ZSTR_VAL(resource->host));
			crx_url_free(resource);
			return NULL;
		}
		crx_url_free(resource);
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: invalid url \"%s\", must have at least crxa://%s/", path, path);
		return NULL;
	}

	if (!crex_string_equals_literal_ci(resource->scheme, "crxa")) {
		crx_url_free(resource);
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: not a crxa url \"%s\"", path);
		return NULL;
	}

	host_len = ZSTR_LEN(resource->host);
	crxa_request_initialize();
	internal_file = ZSTR_VAL(resource->path) + 1; /* strip leading "/" */

	if (FAILURE == crxa_get_archive(&crxa, ZSTR_VAL(resource->host), host_len, NULL, 0, &error)) {
		if (error) {
			crx_stream_wrapper_log_error(wrapper, options, "%s", error);
			efree(error);
		} else {
			crx_stream_wrapper_log_error(wrapper, options, "crxa file \"%s\" is unknown", ZSTR_VAL(resource->host));
		}
		crx_url_free(resource);
		return NULL;
	}

	if (error) {
		efree(error);
	}

	if (*internal_file == '\0') {
		/* root directory requested */
		internal_file = estrndup(internal_file - 1, 1);
		ret = crxa_make_dirstream(internal_file, &crxa->manifest);
		crx_url_free(resource);
		return ret;
	}

	if (!HT_IS_INITIALIZED(&crxa->manifest)) {
		crx_url_free(resource);
		return NULL;
	}

	if (NULL != (entry = crex_hash_str_find_ptr(&crxa->manifest, internal_file, strlen(internal_file))) && !entry->is_dir) {
		crx_url_free(resource);
		return NULL;
	} else if (entry && entry->is_dir) {
		if (entry->is_mounted) {
			crx_url_free(resource);
			return crx_stream_opendir(entry->tmp, options, context);
		}
		internal_file = estrdup(internal_file);
		crx_url_free(resource);
		return crxa_make_dirstream(internal_file, &crxa->manifest);
	} else {
		size_t i_len = strlen(internal_file);

		/* search for directory */
		crex_hash_internal_pointer_reset(&crxa->manifest);
		while (FAILURE != crex_hash_has_more_elements(&crxa->manifest)) {
			if (HASH_KEY_NON_EXISTENT !=
					crex_hash_get_current_key(&crxa->manifest, &str_key, &unused)) {
				if (ZSTR_LEN(str_key) > i_len && 0 == memcmp(ZSTR_VAL(str_key), internal_file, i_len)) {
					/* directory found */
					internal_file = estrndup(internal_file,
							i_len);
					crx_url_free(resource);
					return crxa_make_dirstream(internal_file, &crxa->manifest);
				}
			}

			if (SUCCESS != crex_hash_move_forward(&crxa->manifest)) {
				break;
			}
		}
	}

	crx_url_free(resource);
	return NULL;
}
/* }}} */

/**
 * Make a new directory within a crxa archive
 */
int crxa_wrapper_mkdir(crx_stream_wrapper *wrapper, const char *url_from, int mode, int options, crx_stream_context *context) /* {{{ */
{
	crxa_entry_info entry, *e;
	crxa_archive_data *crxa = NULL;
	char *error, *arch, *entry2;
	size_t arch_len, entry_len;
	crx_url *resource = NULL;
	uint32_t host_len;

	/* pre-readonly check, we need to know if this is a data crxa */
	if (FAILURE == crxa_split_fname(url_from, strlen(url_from), &arch, &arch_len, &entry2, &entry_len, 2, 2)) {
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: cannot create directory \"%s\", no crxa archive specified", url_from);
		return 0;
	}

	if (FAILURE == crxa_get_archive(&crxa, arch, arch_len, NULL, 0, NULL)) {
		crxa = NULL;
	}

	efree(arch);
	efree(entry2);

	if (CRXA_G(readonly) && (!crxa || !crxa->is_data)) {
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: cannot create directory \"%s\", write operations disabled", url_from);
		return 0;
	}

	if ((resource = crxa_parse_url(wrapper, url_from, "w", options)) == NULL) {
		return 0;
	}

	/* we must have at the very least crxa://alias.crxa/internalfile.crx */
	if (!resource->scheme || !resource->host || !resource->path) {
		crx_url_free(resource);
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: invalid url \"%s\"", url_from);
		return 0;
	}

	if (!crex_string_equals_literal_ci(resource->scheme, "crxa")) {
		crx_url_free(resource);
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: not a crxa stream url \"%s\"", url_from);
		return 0;
	}

	host_len = ZSTR_LEN(resource->host);

	if (FAILURE == crxa_get_archive(&crxa, ZSTR_VAL(resource->host), host_len, NULL, 0, &error)) {
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: cannot create directory \"%s\" in crxa \"%s\", error retrieving crxa information: %s", ZSTR_VAL(resource->path) + 1, ZSTR_VAL(resource->host), error);
		efree(error);
		crx_url_free(resource);
		return 0;
	}

	if ((e = crxa_get_entry_info_dir(crxa, ZSTR_VAL(resource->path) + 1, ZSTR_LEN(resource->path) - 1, 2, &error, 1))) {
		/* directory exists, or is a subdirectory of an existing file */
		if (e->is_temp_dir) {
			efree(e->filename);
			efree(e);
		}
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: cannot create directory \"%s\" in crxa \"%s\", directory already exists", ZSTR_VAL(resource->path)+1, ZSTR_VAL(resource->host));
		crx_url_free(resource);
		return 0;
	}

	if (error) {
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: cannot create directory \"%s\" in crxa \"%s\", %s", ZSTR_VAL(resource->path)+1, ZSTR_VAL(resource->host), error);
		efree(error);
		crx_url_free(resource);
		return 0;
	}

	if (crxa_get_entry_info_dir(crxa, ZSTR_VAL(resource->path) + 1, ZSTR_LEN(resource->path) - 1, 0, &error, 1)) {
		/* entry exists as a file */
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: cannot create directory \"%s\" in crxa \"%s\", file already exists", ZSTR_VAL(resource->path)+1, ZSTR_VAL(resource->host));
		crx_url_free(resource);
		return 0;
	}

	if (error) {
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: cannot create directory \"%s\" in crxa \"%s\", %s", ZSTR_VAL(resource->path)+1, ZSTR_VAL(resource->host), error);
		efree(error);
		crx_url_free(resource);
		return 0;
	}

	memset((void *) &entry, 0, sizeof(crxa_entry_info));

	/* strip leading "/" */
	if (crxa->is_zip) {
		entry.is_zip = 1;
	}

	entry.filename = estrdup(ZSTR_VAL(resource->path) + 1);

	if (crxa->is_tar) {
		entry.is_tar = 1;
		entry.tar_type = TAR_DIR;
	}

	entry.filename_len = ZSTR_LEN(resource->path) - 1;
	crx_url_free(resource);
	entry.is_dir = 1;
	entry.crxa = crxa;
	entry.is_modified = 1;
	entry.is_crc_checked = 1;
	entry.flags = CRXA_ENT_PERM_DEF_DIR;
	entry.old_flags = CRXA_ENT_PERM_DEF_DIR;

	if (NULL == crex_hash_str_add_mem(&crxa->manifest, entry.filename, entry.filename_len, (void*)&entry, sizeof(crxa_entry_info))) {
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: cannot create directory \"%s\" in crxa \"%s\", adding to manifest failed", entry.filename, crxa->fname);
		efree(error);
		efree(entry.filename);
		return 0;
	}

	crxa_flush(crxa, 0, 0, 0, &error);

	if (error) {
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: cannot create directory \"%s\" in crxa \"%s\", %s", entry.filename, crxa->fname, error);
		crex_hash_str_del(&crxa->manifest, entry.filename, entry.filename_len);
		efree(error);
		return 0;
	}

	crxa_add_virtual_dirs(crxa, entry.filename, entry.filename_len);
	return 1;
}
/* }}} */

/**
 * Remove a directory within a crxa archive
 */
int crxa_wrapper_rmdir(crx_stream_wrapper *wrapper, const char *url, int options, crx_stream_context *context) /* {{{ */
{
	crxa_entry_info *entry;
	crxa_archive_data *crxa = NULL;
	char *error, *arch, *entry2;
	size_t arch_len, entry_len;
	crx_url *resource = NULL;
	uint32_t host_len;
	crex_string *str_key;
	crex_ulong unused;
	uint32_t path_len;

	/* pre-readonly check, we need to know if this is a data crxa */
	if (FAILURE == crxa_split_fname(url, strlen(url), &arch, &arch_len, &entry2, &entry_len, 2, 2)) {
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: cannot remove directory \"%s\", no crxa archive specified, or crxa archive does not exist", url);
		return 0;
	}

	if (FAILURE == crxa_get_archive(&crxa, arch, arch_len, NULL, 0, NULL)) {
		crxa = NULL;
	}

	efree(arch);
	efree(entry2);

	if (CRXA_G(readonly) && (!crxa || !crxa->is_data)) {
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: cannot rmdir directory \"%s\", write operations disabled", url);
		return 0;
	}

	if ((resource = crxa_parse_url(wrapper, url, "w", options)) == NULL) {
		return 0;
	}

	/* we must have at the very least crxa://alias.crxa/internalfile.crx */
	if (!resource->scheme || !resource->host || !resource->path) {
		crx_url_free(resource);
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: invalid url \"%s\"", url);
		return 0;
	}

	if (!crex_string_equals_literal_ci(resource->scheme, "crxa")) {
		crx_url_free(resource);
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: not a crxa stream url \"%s\"", url);
		return 0;
	}

	host_len = ZSTR_LEN(resource->host);

	if (FAILURE == crxa_get_archive(&crxa, ZSTR_VAL(resource->host), host_len, NULL, 0, &error)) {
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: cannot remove directory \"%s\" in crxa \"%s\", error retrieving crxa information: %s", ZSTR_VAL(resource->path)+1, ZSTR_VAL(resource->host), error);
		efree(error);
		crx_url_free(resource);
		return 0;
	}

	path_len = ZSTR_LEN(resource->path) - 1;

	if (!(entry = crxa_get_entry_info_dir(crxa, ZSTR_VAL(resource->path) + 1, path_len, 2, &error, 1))) {
		if (error) {
			crx_stream_wrapper_log_error(wrapper, options, "crxa error: cannot remove directory \"%s\" in crxa \"%s\", %s", ZSTR_VAL(resource->path)+1, ZSTR_VAL(resource->host), error);
			efree(error);
		} else {
			crx_stream_wrapper_log_error(wrapper, options, "crxa error: cannot remove directory \"%s\" in crxa \"%s\", directory does not exist", ZSTR_VAL(resource->path)+1, ZSTR_VAL(resource->host));
		}
		crx_url_free(resource);
		return 0;
	}

	if (!entry->is_deleted) {
		for (crex_hash_internal_pointer_reset(&crxa->manifest);
			HASH_KEY_NON_EXISTENT != crex_hash_get_current_key(&crxa->manifest, &str_key, &unused);
			crex_hash_move_forward(&crxa->manifest)
		) {
			if (ZSTR_LEN(str_key) > path_len &&
				memcmp(ZSTR_VAL(str_key), ZSTR_VAL(resource->path)+1, path_len) == 0 &&
				IS_SLASH(ZSTR_VAL(str_key)[path_len])) {
				crx_stream_wrapper_log_error(wrapper, options, "crxa error: Directory not empty");
				if (entry->is_temp_dir) {
					efree(entry->filename);
					efree(entry);
				}
				crx_url_free(resource);
				return 0;
			}
		}

		for (crex_hash_internal_pointer_reset(&crxa->virtual_dirs);
			HASH_KEY_NON_EXISTENT != crex_hash_get_current_key(&crxa->virtual_dirs, &str_key, &unused);
			crex_hash_move_forward(&crxa->virtual_dirs)) {

			if (ZSTR_LEN(str_key) > path_len &&
				memcmp(ZSTR_VAL(str_key), ZSTR_VAL(resource->path)+1, path_len) == 0 &&
				IS_SLASH(ZSTR_VAL(str_key)[path_len])) {
				crx_stream_wrapper_log_error(wrapper, options, "crxa error: Directory not empty");
				if (entry->is_temp_dir) {
					efree(entry->filename);
					efree(entry);
				}
				crx_url_free(resource);
				return 0;
			}
		}
	}

	if (entry->is_temp_dir) {
		crex_hash_str_del(&crxa->virtual_dirs, ZSTR_VAL(resource->path)+1, path_len);
		efree(entry->filename);
		efree(entry);
	} else {
		entry->is_deleted = 1;
		entry->is_modified = 1;
		crxa_flush(crxa, 0, 0, 0, &error);

		if (error) {
			crx_stream_wrapper_log_error(wrapper, options, "crxa error: cannot remove directory \"%s\" in crxa \"%s\", %s", entry->filename, crxa->fname, error);
			crx_url_free(resource);
			efree(error);
			return 0;
		}
	}

	crx_url_free(resource);
	return 1;
}
/* }}} */
