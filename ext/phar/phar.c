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

#define CRXA_MAIN 1
#include "crxa_internal.h"
#include "SAPI.h"
#include "func_interceptors.h"
#include "ext/standard/crx_var.h"

static void destroy_crxa_data(zval *zv);

CREX_DECLARE_MODULE_GLOBALS(crxa)
static crex_string *(*crxa_save_resolve_path)(crex_string *filename);

/**
 * set's crxa->is_writeable based on the current INI value
 */
static int crxa_set_writeable_bit(zval *zv, void *argument) /* {{{ */
{
	bool keep = *(bool *)argument;
	crxa_archive_data *crxa = (crxa_archive_data *)C_PTR_P(zv);

	if (!crxa->is_data) {
		crxa->is_writeable = !keep;
	}

	return CREX_HASH_APPLY_KEEP;
}
/* }}} */

/* if the original value is 0 (disabled), then allow setting/unsetting at will. Otherwise only allow 1 (enabled), and error on disabling */
CREX_INI_MH(crxa_ini_modify_handler) /* {{{ */
{
	bool old, ini;

	if (ZSTR_LEN(entry->name) == sizeof("crxa.readonly")-1) {
		old = CRXA_G(readonly_orig);
	} else {
		old = CRXA_G(require_hash_orig);
	}

	ini = crex_ini_parse_bool(new_value);

	/* do not allow unsetting in runtime */
	if (stage == CREX_INI_STAGE_STARTUP) {
		if (ZSTR_LEN(entry->name) == sizeof("crxa.readonly")-1) {
			CRXA_G(readonly_orig) = ini;
		} else {
			CRXA_G(require_hash_orig) = ini;
		}
	} else if (old && !ini) {
		return FAILURE;
	}

	if (ZSTR_LEN(entry->name) == sizeof("crxa.readonly")-1) {
		CRXA_G(readonly) = ini;
		if (CRXA_G(request_init) && HT_IS_INITIALIZED(&CRXA_G(crxa_fname_map))) {
			crex_hash_apply_with_argument(&(CRXA_G(crxa_fname_map)), crxa_set_writeable_bit, (void *)&ini);
		}
	} else {
		CRXA_G(require_hash) = ini;
	}

	return SUCCESS;
}
/* }}}*/

/* this global stores the global cached pre-parsed manifests */
HashTable cached_crxas;
HashTable cached_alias;

static void crxa_split_cache_list(void) /* {{{ */
{
	char *tmp;
	char *key, *lasts, *end;
	char ds[2];
	crxa_archive_data *crxa;
	uint32_t i = 0;

	if (!CRXA_G(cache_list) || !(CRXA_G(cache_list)[0])) {
		return;
	}

	ds[0] = DEFAULT_DIR_SEPARATOR;
	ds[1] = '\0';
	tmp = estrdup(CRXA_G(cache_list));

	/* fake request startup */
	CRXA_G(request_init) = 1;
	crex_init_rsrc_list();
	EG(regular_list).nNextFreeElement=1;	/* we don't want resource id 0 */

	CRXA_G(has_bz2) = crex_hash_str_exists(&module_registry, "bz2", sizeof("bz2")-1);
	CRXA_G(has_zlib) = crex_hash_str_exists(&module_registry, "zlib", sizeof("zlib")-1);
	/* these two are dummies and will be destroyed later */
	crex_hash_init(&cached_crxas, sizeof(crxa_archive_data*), crex_get_hash_value, destroy_crxa_data,  1);
	crex_hash_init(&cached_alias, sizeof(crxa_archive_data*), crex_get_hash_value, NULL, 1);
	/* these two are real and will be copied over cached_crxas/cached_alias later */
	crex_hash_init(&(CRXA_G(crxa_fname_map)), sizeof(crxa_archive_data*), crex_get_hash_value, destroy_crxa_data,  1);
	crex_hash_init(&(CRXA_G(crxa_alias_map)), sizeof(crxa_archive_data*), crex_get_hash_value, NULL, 1);
	CRXA_G(manifest_cached) = 1;
	CRXA_G(persist) = 1;

	for (key = crx_strtok_r(tmp, ds, &lasts);
			key;
			key = crx_strtok_r(NULL, ds, &lasts)) {
		size_t len;
		end = strchr(key, DEFAULT_DIR_SEPARATOR);
		if (end) {
			len = end - key;
		} else {
			len = strlen(key);
		}

		if (SUCCESS == crxa_open_from_filename(key, len, NULL, 0, 0, &crxa, NULL)) {
			crxa->crxa_pos = i++;
			crx_stream_close(crxa->fp);
			crxa->fp = NULL;
		} else {
			CRXA_G(persist) = 0;
			CRXA_G(manifest_cached) = 0;
			efree(tmp);
			crex_hash_destroy(&(CRXA_G(crxa_fname_map)));
			HT_INVALIDATE(&CRXA_G(crxa_fname_map));
			crex_hash_destroy(&(CRXA_G(crxa_alias_map)));
			HT_INVALIDATE(&CRXA_G(crxa_alias_map));
			crex_hash_destroy(&cached_crxas);
			crex_hash_destroy(&cached_alias);
			crex_hash_graceful_reverse_destroy(&EG(regular_list));
			memset(&EG(regular_list), 0, sizeof(HashTable));
			/* free cached manifests */
			CRXA_G(request_init) = 0;
			return;
		}
	}

	CRXA_G(persist) = 0;
	CRXA_G(request_init) = 0;
	/* destroy dummy values from before */
	crex_hash_destroy(&cached_crxas);
	crex_hash_destroy(&cached_alias);
	cached_crxas = CRXA_G(crxa_fname_map);
	cached_alias = CRXA_G(crxa_alias_map);
	HT_INVALIDATE(&CRXA_G(crxa_fname_map));
	HT_INVALIDATE(&CRXA_G(crxa_alias_map));
	crex_hash_graceful_reverse_destroy(&EG(regular_list));
	memset(&EG(regular_list), 0, sizeof(HashTable));
	efree(tmp);
}
/* }}} */

CREX_INI_MH(crxa_ini_cache_list) /* {{{ */
{
	CRXA_G(cache_list) = ZSTR_VAL(new_value);

	if (stage == CREX_INI_STAGE_STARTUP) {
		crxa_split_cache_list();
	}

	return SUCCESS;
}
/* }}} */

CRX_INI_BEGIN()
	STD_CRX_INI_BOOLEAN("crxa.readonly", "1", CRX_INI_ALL, crxa_ini_modify_handler, readonly, crex_crxa_globals, crxa_globals)
	STD_CRX_INI_BOOLEAN("crxa.require_hash", "1", CRX_INI_ALL, crxa_ini_modify_handler, require_hash, crex_crxa_globals, crxa_globals)
	STD_CRX_INI_ENTRY("crxa.cache_list", "", CRX_INI_SYSTEM, crxa_ini_cache_list, cache_list, crex_crxa_globals, crxa_globals)
CRX_INI_END()

/**
 * When all uses of a crxa have been concluded, this frees the manifest
 * and the crxa slot
 */
void crxa_destroy_crxa_data(crxa_archive_data *crxa) /* {{{ */
{
	if (crxa->alias && crxa->alias != crxa->fname) {
		pefree(crxa->alias, crxa->is_persistent);
		crxa->alias = NULL;
	}

	if (crxa->fname) {
		pefree(crxa->fname, crxa->is_persistent);
		crxa->fname = NULL;
	}

	if (crxa->signature) {
		pefree(crxa->signature, crxa->is_persistent);
		crxa->signature = NULL;
	}

	if (HT_IS_INITIALIZED(&crxa->manifest)) {
		crex_hash_destroy(&crxa->manifest);
		HT_INVALIDATE(&crxa->manifest);
	}

	if (HT_IS_INITIALIZED(&crxa->mounted_dirs)) {
		crex_hash_destroy(&crxa->mounted_dirs);
		HT_INVALIDATE(&crxa->mounted_dirs);
	}

	if (HT_IS_INITIALIZED(&crxa->virtual_dirs)) {
		crex_hash_destroy(&crxa->virtual_dirs);
		HT_INVALIDATE(&crxa->virtual_dirs);
	}

	crxa_metadata_tracker_free(&crxa->metadata_tracker, crxa->is_persistent);

	if (crxa->fp) {
		crx_stream_close(crxa->fp);
		crxa->fp = 0;
	}

	if (crxa->ufp) {
		crx_stream_close(crxa->ufp);
		crxa->ufp = 0;
	}

	pefree(crxa, crxa->is_persistent);
}
/* }}}*/

/**
 * Delete refcount and destruct if needed. On destruct return 1 else 0.
 */
int crxa_archive_delref(crxa_archive_data *crxa) /* {{{ */
{
	if (crxa->is_persistent) {
		return 0;
	}

	if (--crxa->refcount < 0) {
		if (CRXA_G(request_done)
		|| crex_hash_str_del(&(CRXA_G(crxa_fname_map)), crxa->fname, crxa->fname_len) != SUCCESS) {
			crxa_destroy_crxa_data(crxa);
		}
		return 1;
	} else if (!crxa->refcount) {
		/* invalidate crxa cache */
		CRXA_G(last_crxa) = NULL;
		CRXA_G(last_crxa_name) = CRXA_G(last_alias) = NULL;

		if (crxa->fp && (!(crxa->flags & CRXA_FILE_COMPRESSION_MASK) || !crxa->alias)) {
			/* close open file handle - allows removal or rename of
			the file on windows, which has greedy locking
			only close if the archive was not already compressed.  If it
			was compressed, then the fp does not refer to the original file.
			We're also closing compressed files to save resources,
			but only if the archive isn't aliased. */
			crx_stream_close(crxa->fp);
			crxa->fp = NULL;
		}

		if (!crex_hash_num_elements(&crxa->manifest)) {
			/* this is a new crxa that has perhaps had an alias/metadata set, but has never
			been flushed */
			if (crex_hash_str_del(&(CRXA_G(crxa_fname_map)), crxa->fname, crxa->fname_len) != SUCCESS) {
				crxa_destroy_crxa_data(crxa);
			}
			return 1;
		}
	}
	return 0;
}
/* }}}*/

/**
 * Destroy crxa's in shutdown, here we don't care about aliases
 */
static void destroy_crxa_data_only(zval *zv) /* {{{ */
{
	crxa_archive_data *crxa_data = (crxa_archive_data *) C_PTR_P(zv);

	if (EG(exception) || --crxa_data->refcount < 0) {
		crxa_destroy_crxa_data(crxa_data);
	}
}
/* }}}*/

/**
 * Delete aliases to crxa's that got kicked out of the global table
 */
static int crxa_unalias_apply(zval *zv, void *argument) /* {{{ */
{
	return C_PTR_P(zv) == argument ? CREX_HASH_APPLY_REMOVE : CREX_HASH_APPLY_KEEP;
}
/* }}} */

/**
 * Delete aliases to crxa's that got kicked out of the global table
 */
static int crxa_tmpclose_apply(zval *zv) /* {{{ */
{
	crxa_entry_info *entry = (crxa_entry_info *) C_PTR_P(zv);

	if (entry->fp_type != CRXA_TMP) {
		return CREX_HASH_APPLY_KEEP;
	}

	if (entry->fp && !entry->fp_refcount) {
		crx_stream_close(entry->fp);
		entry->fp = NULL;
	}

	return CREX_HASH_APPLY_KEEP;
}
/* }}} */

/**
 * Filename map destructor
 */
static void destroy_crxa_data(zval *zv) /* {{{ */
{
	crxa_archive_data *crxa_data = (crxa_archive_data *)C_PTR_P(zv);

	if (CRXA_G(request_ends)) {
		/* first, iterate over the manifest and close all CRXA_TMP entry fp handles,
		this prevents unnecessary unfreed stream resources */
		crex_hash_apply(&(crxa_data->manifest), crxa_tmpclose_apply);
		destroy_crxa_data_only(zv);
		return;
	}

	crex_hash_apply_with_argument(&(CRXA_G(crxa_alias_map)), crxa_unalias_apply, crxa_data);

	if (--crxa_data->refcount < 0) {
		crxa_destroy_crxa_data(crxa_data);
	}
}
/* }}}*/

/**
 * destructor for the manifest hash, frees each file's entry
 */
void destroy_crxa_manifest_entry_int(crxa_entry_info *entry) /* {{{ */
{

	if (entry->cfp) {
		crx_stream_close(entry->cfp);
		entry->cfp = 0;
	}

	if (entry->fp) {
		crx_stream_close(entry->fp);
		entry->fp = 0;
	}

	crxa_metadata_tracker_free(&entry->metadata_tracker, entry->is_persistent);

	pefree(entry->filename, entry->is_persistent);

	if (entry->link) {
		pefree(entry->link, entry->is_persistent);
		entry->link = 0;
	}

	if (entry->tmp) {
		pefree(entry->tmp, entry->is_persistent);
		entry->tmp = 0;
	}
}
/* }}} */

void destroy_crxa_manifest_entry(zval *zv) /* {{{ */
{
	crxa_entry_info *entry = C_PTR_P(zv);
	destroy_crxa_manifest_entry_int(entry);
	pefree(entry, entry->is_persistent);
}
/* }}} */

int crxa_entry_delref(crxa_entry_data *idata) /* {{{ */
{
	int ret = 0;

	if (idata->internal_file && !idata->internal_file->is_persistent) {
		if (--idata->internal_file->fp_refcount < 0) {
			idata->internal_file->fp_refcount = 0;
		}

		if (idata->fp && idata->fp != idata->crxa->fp && idata->fp != idata->crxa->ufp && idata->fp != idata->internal_file->fp) {
			crx_stream_close(idata->fp);
		}
		/* if crxa_get_or_create_entry_data returns a sub-directory, we have to free it */
		if (idata->internal_file->is_temp_dir) {
			destroy_crxa_manifest_entry_int(idata->internal_file);
			efree(idata->internal_file);
		}
	}

	crxa_archive_delref(idata->crxa);
	efree(idata);
	return ret;
}
/* }}} */

/**
 * Removes an entry, either by actually removing it or by marking it.
 */
void crxa_entry_remove(crxa_entry_data *idata, char **error) /* {{{ */
{
	crxa_archive_data *crxa;

	crxa = idata->crxa;

	if (idata->internal_file->fp_refcount < 2) {
		if (idata->fp && idata->fp != idata->crxa->fp && idata->fp != idata->crxa->ufp && idata->fp != idata->internal_file->fp) {
			crx_stream_close(idata->fp);
		}
		crex_hash_str_del(&idata->crxa->manifest, idata->internal_file->filename, idata->internal_file->filename_len);
		idata->crxa->refcount--;
		efree(idata);
	} else {
		idata->internal_file->is_deleted = 1;
		crxa_entry_delref(idata);
	}

	if (!crxa->donotflush) {
		crxa_flush(crxa, 0, 0, 0, error);
	}
}
/* }}} */

#define MAPCRXA_ALLOC_FAIL(msg) \
	if (fp) {\
		crx_stream_close(fp);\
	}\
	if (error) {\
		spprintf(error, 0, msg, fname);\
	}\
	return FAILURE;

#define MAPCRXA_FAIL(msg) \
	efree(savebuf);\
	if (mydata) {\
		crxa_destroy_crxa_data(mydata);\
	}\
	if (signature) {\
		pefree(signature, CRXA_G(persist));\
	}\
	MAPCRXA_ALLOC_FAIL(msg)

#ifdef WORDS_BIGENDIAN
# define CRXA_GET_32(buffer, var) \
	var = ((((unsigned char*)(buffer))[3]) << 24) \
		| ((((unsigned char*)(buffer))[2]) << 16) \
		| ((((unsigned char*)(buffer))[1]) <<  8) \
		| (((unsigned char*)(buffer))[0]); \
	(buffer) += 4
# define CRXA_GET_16(buffer, var) \
	var = ((((unsigned char*)(buffer))[1]) <<  8) \
		| (((unsigned char*)(buffer))[0]); \
	(buffer) += 2
#else
# define CRXA_GET_32(buffer, var) \
	memcpy(&var, buffer, sizeof(var)); \
	buffer += 4
# define CRXA_GET_16(buffer, var) \
	var = *(uint16_t*)(buffer); \
	buffer += 2
#endif
#define CRXA_ZIP_16(var) ((uint16_t)((((uint16_t)var[0]) & 0xff) | \
	(((uint16_t)var[1]) & 0xff) << 8))
#define CRXA_ZIP_32(var) ((uint32_t)((((uint32_t)var[0]) & 0xff) | \
	(((uint32_t)var[1]) & 0xff) << 8 | \
	(((uint32_t)var[2]) & 0xff) << 16 | \
	(((uint32_t)var[3]) & 0xff) << 24))

/**
 * Open an already loaded crxa
 */
int crxa_open_parsed_crxa(char *fname, size_t fname_len, char *alias, size_t alias_len, bool is_data, uint32_t options, crxa_archive_data** pcrxa, char **error) /* {{{ */
{
	crxa_archive_data *crxa;
#ifdef CRX_WIN32
	char *save_fname;
	ALLOCA_FLAG(fname_use_heap)
#endif

	if (error) {
		*error = NULL;
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
	if (SUCCESS == crxa_get_archive(&crxa, fname, fname_len, alias, alias_len, error)
		&& ((alias && fname_len == crxa->fname_len
		&& !strncmp(fname, crxa->fname, fname_len)) || !alias)
	) {
		crxa_entry_info *stub;
#ifdef CRX_WIN32
		if (fname != save_fname) {
			free_alloca(fname, fname_use_heap);
			fname = save_fname;
		}
#endif
		/* logic above is as follows:
		   If an explicit alias was requested, ensure the filename passed in
		   matches the crxa's filename.
		   If no alias was passed in, then it can match either and be valid
		 */

		if (!is_data) {
			/* prevent any ".crxa" without a stub getting through */
			if (!crxa->halt_offset && !crxa->is_brandnew && (crxa->is_tar || crxa->is_zip)) {
				if (CRXA_G(readonly) && NULL == (stub = crex_hash_str_find_ptr(&(crxa->manifest), ".crxa/stub.crx", sizeof(".crxa/stub.crx")-1))) {
					if (error) {
						spprintf(error, 0, "'%s' is not a crxa archive. Use CrxaData::__main() for a standard zip or tar archive", fname);
					}
					return FAILURE;
				}
			}
		}

		if (pcrxa) {
			*pcrxa = crxa;
		}

		return SUCCESS;
	} else {
#ifdef CRX_WIN32
		if (fname != save_fname) {
			free_alloca(fname, fname_use_heap);
			fname = save_fname;
		}
#endif
		if (pcrxa) {
			*pcrxa = NULL;
		}

		if (crxa && error && !(options & REPORT_ERRORS)) {
			efree(error);
		}

		return FAILURE;
	}
}
/* }}}*/

/**
 * Attempt to serialize the data.
 * Callers are responsible for handling EG(exception) if one occurs.
 */
void crxa_metadata_tracker_try_ensure_has_serialized_data(crxa_metadata_tracker *tracker, int persistent) /* {{{ */
{
	crx_serialize_data_t metadata_hash;
	smart_str metadata_str = {0};
	if (tracker->str || C_ISUNDEF(tracker->val)) {
		/* Already has serialized the value or there is no value */
		return;
	}
	/* Assert it should not be possible to create raw zvals in a persistent crxa (i.e. from cache_list) */
	CREX_ASSERT(!persistent);

	CRX_VAR_SERIALIZE_INIT(metadata_hash);
	crx_var_serialize(&metadata_str, &tracker->val, &metadata_hash);
	CRX_VAR_SERIALIZE_DESTROY(metadata_hash);
	if (!metadata_str.s) {
		return;
	}
	tracker->str = metadata_str.s;
}
/* }}} */

/**
 * Parse out metadata when crxa_metadata_tracker_has_data is true.
 *
 * Precondition: crxa_metadata_tracker_has_data is true
 */
int crxa_metadata_tracker_unserialize_or_copy(crxa_metadata_tracker *tracker, zval *metadata, int persistent, HashTable *unserialize_options, const char* method_name) /* {{{ */
{
	const bool has_unserialize_options = unserialize_options != NULL && crex_hash_num_elements(unserialize_options) > 0;
	/* It should be impossible to create a zval in a persistent crxa/entry. */
	CREX_ASSERT(!persistent || C_ISUNDEF(tracker->val));

	if (C_ISUNDEF(tracker->val) || has_unserialize_options) {
		if (EG(exception)) {
			/* Because other parts of the crxa code haven't been updated to check for exceptions after doing something that may throw,
			 * check for exceptions before potentially serializing/unserializing instead. */
			return FAILURE;
		}
		/* Persistent crxas should always be unserialized. */
		const char *start;
		/* Assert it should not be possible to create raw data in a persistent crxa (i.e. from cache_list) */

		/* Precondition: This has serialized data, either from setMetadata or the crxa file. */
		CREX_ASSERT(tracker->str != NULL);
		ZVAL_NULL(metadata);
		start = ZSTR_VAL(tracker->str);

		crx_unserialize_with_options(metadata, start, ZSTR_LEN(tracker->str), unserialize_options, method_name);
		if (EG(exception)) {
			zval_ptr_dtor(metadata);
			ZVAL_UNDEF(metadata);
			return FAILURE;
		}
		return SUCCESS;
	} else {
		/* TODO: what is the current/expected behavior when fetching an object set with setMetadata then getting it
		 * with getMetadata() and modifying a property? Previously, it was underdefined, and probably unimportant to support. */
		ZVAL_COPY(metadata, &tracker->val);
	}

	return SUCCESS;
}
/* }}}*/

/**
 * Check if this has any data, serialized or as a raw value.
 */
bool crxa_metadata_tracker_has_data(const crxa_metadata_tracker *tracker, int persistent) /* {{{ */
{
	CREX_ASSERT(!persistent || C_ISUNDEF(tracker->val));
	return !C_ISUNDEF(tracker->val) || tracker->str != NULL;
}
/* }}} */

/**
 * Free memory used to track the metadata and set all fields to be null/undef.
 */
void crxa_metadata_tracker_free(crxa_metadata_tracker *tracker, int persistent) /* {{{ */
{
	/* Free the string before the zval in case the zval's destructor modifies the metadata */
	if (tracker->str) {
		crex_string_release(tracker->str);
		tracker->str = NULL;
	}
	if (!C_ISUNDEF(tracker->val)) {
		/* Here, copy the original zval to a different pointer without incrementing the refcount in case something uses the original while it's being freed. */
		zval zval_copy;

		CREX_ASSERT(!persistent);
		ZVAL_COPY_VALUE(&zval_copy, &tracker->val);
		ZVAL_UNDEF(&tracker->val);
		zval_ptr_dtor(&zval_copy);
	}
}
/* }}} */

/**
 * Free memory used to track the metadata and set all fields to be null/undef.
 */
void crxa_metadata_tracker_copy(crxa_metadata_tracker *dest, const crxa_metadata_tracker *source, int persistent) /* {{{ */
{
	CREX_ASSERT(dest != source);
	crxa_metadata_tracker_free(dest, persistent);

	if (!C_ISUNDEF(source->val)) {
		CREX_ASSERT(!persistent);
		ZVAL_COPY(&dest->val, &source->val);
	}
	if (source->str) {
		dest->str = crex_string_copy(source->str);
	}
}
/* }}} */

/**
 * Copy constructor for a non-persistent clone.
 */
void crxa_metadata_tracker_clone(crxa_metadata_tracker *tracker) /* {{{ */
{
	C_TRY_ADDREF_P(&tracker->val);
	if (tracker->str) {
		/* Duplicate the string, as the original may have been persistent. */
		tracker->str = crex_string_dup(tracker->str, false);
	}
}
/* }}} */

/**
 * Parse out metadata from the manifest for a single file, saving it into a string.
 *
 * Meta-data is in this format:
 * [len32][data...]
 *
 * data is the serialized zval
 */
void crxa_parse_metadata_lazy(const char *buffer, crxa_metadata_tracker *tracker, uint32_t zip_metadata_len, int persistent) /* {{{ */
{
	crxa_metadata_tracker_free(tracker, persistent);
	if (zip_metadata_len) {
		/* lazy init metadata */
		tracker->str = crex_string_init(buffer, zip_metadata_len, persistent);
	}
}
/* }}}*/

/**
 * Size of fixed fields in the manifest.
 * See: http://crx.net/manual/en/crxa.fileformat.crxa.crx
 */
#define MANIFEST_FIXED_LEN	18

#define SAFE_CRXA_GET_32(buffer, endbuffer, var) \
	if (UNEXPECTED(buffer + 4 > endbuffer)) { \
		MAPCRXA_FAIL("internal corruption of crxa \"%s\" (truncated manifest header)"); \
	} \
	CRXA_GET_32(buffer, var);

/**
 * Does not check for a previously opened crxa in the cache.
 *
 * Parse a new one and add it to the cache, returning either SUCCESS or
 * FAILURE, and setting pcrxa to the pointer to the manifest entry
 *
 * This is used by crxa_open_from_filename to process the manifest, but can be called
 * directly.
 */
static int crxa_parse_crxafile(crx_stream *fp, char *fname, size_t fname_len, char *alias, size_t alias_len, crex_long halt_offset, crxa_archive_data** pcrxa, uint32_t compression, char **error) /* {{{ */
{
	char b32[4], *buffer, *endbuffer, *savebuf;
	crxa_archive_data *mydata = NULL;
	crxa_entry_info entry;
	uint32_t manifest_len, manifest_count, manifest_flags, manifest_index, tmp_len, sig_flags;
	uint16_t manifest_ver;
	uint32_t len;
	crex_long offset;
	size_t sig_len;
	int register_alias = 0, temp_alias = 0;
	char *signature = NULL;
	crex_string *str;

	if (pcrxa) {
		*pcrxa = NULL;
	}

	if (error) {
		*error = NULL;
	}

	/* check for ?>\n and increment accordingly */
	if (-1 == crx_stream_seek(fp, halt_offset, SEEK_SET)) {
		MAPCRXA_ALLOC_FAIL("cannot seek to __HALT_COMPILER(); location in crxa \"%s\"")
	}

	buffer = b32;

	if (3 != crx_stream_read(fp, buffer, 3)) {
		MAPCRXA_ALLOC_FAIL("internal corruption of crxa \"%s\" (truncated manifest at stub end)")
	}

	if ((*buffer == ' ' || *buffer == '\n') && *(buffer + 1) == '?' && *(buffer + 2) == '>') {
		int nextchar;
		halt_offset += 3;
		if (EOF == (nextchar = crx_stream_getc(fp))) {
			MAPCRXA_ALLOC_FAIL("internal corruption of crxa \"%s\" (truncated manifest at stub end)")
		}

		if ((char) nextchar == '\r') {
			/* if we have an \r we require an \n as well */
			if (EOF == (nextchar = crx_stream_getc(fp)) || (char)nextchar != '\n') {
				MAPCRXA_ALLOC_FAIL("internal corruption of crxa \"%s\" (truncated manifest at stub end)")
			}
			++halt_offset;
		}

		if ((char) nextchar == '\n') {
			++halt_offset;
		}
	}

	/* make sure we are at the right location to read the manifest */
	if (-1 == crx_stream_seek(fp, halt_offset, SEEK_SET)) {
		MAPCRXA_ALLOC_FAIL("cannot seek to __HALT_COMPILER(); location in crxa \"%s\"")
	}

	/* read in manifest */
	buffer = b32;

	if (4 != crx_stream_read(fp, buffer, 4)) {
		MAPCRXA_ALLOC_FAIL("internal corruption of crxa \"%s\" (truncated manifest at manifest length)")
	}

	CRXA_GET_32(buffer, manifest_len);

	if (manifest_len > 1048576 * 100) {
		/* prevent serious memory issues by limiting manifest to at most 100 MB in length */
		MAPCRXA_ALLOC_FAIL("manifest cannot be larger than 100 MB in crxa \"%s\"")
	}

	buffer = (char *)emalloc(manifest_len);
	savebuf = buffer;
	endbuffer = buffer + manifest_len;

	if (manifest_len < MANIFEST_FIXED_LEN || manifest_len != crx_stream_read(fp, buffer, manifest_len)) {
		MAPCRXA_FAIL("internal corruption of crxa \"%s\" (truncated manifest header)")
	}

	/* extract the number of entries */
	SAFE_CRXA_GET_32(buffer, endbuffer, manifest_count);

	if (manifest_count == 0) {
		MAPCRXA_FAIL("in crxa \"%s\", manifest claims to have zero entries.  Crxas must have at least 1 entry");
	}

	/* extract API version, lowest nibble currently unused */
	manifest_ver = (((unsigned char)buffer[0]) << 8)
				 + ((unsigned char)buffer[1]);
	buffer += 2;

	if ((manifest_ver & CRXA_API_VER_MASK) < CRXA_API_MIN_READ) {
		efree(savebuf);
		crx_stream_close(fp);
		if (error) {
			spprintf(error, 0, "crxa \"%s\" is API version %1.u.%1.u.%1.u, and cannot be processed", fname, manifest_ver >> 12, (manifest_ver >> 8) & 0xF, (manifest_ver >> 4) & 0x0F);
		}
		return FAILURE;
	}

	SAFE_CRXA_GET_32(buffer, endbuffer, manifest_flags);

	manifest_flags &= ~CRXA_HDR_COMPRESSION_MASK;
	manifest_flags &= ~CRXA_FILE_COMPRESSION_MASK;
	/* remember whether this entire crxa was compressed with gz/bzip2 */
	manifest_flags |= compression;

	/* The lowest nibble contains the crxa wide flags. The compression flags can */
	/* be ignored on reading because it is being generated anyways. */
	if (manifest_flags & CRXA_HDR_SIGNATURE) {
		char sig_buf[8], *sig_ptr = sig_buf;
		crex_off_t read_len;
		size_t end_of_crxa;

		if (-1 == crx_stream_seek(fp, -8, SEEK_END)
		|| (read_len = crx_stream_tell(fp)) < 20
		|| 8 != crx_stream_read(fp, sig_buf, 8)
		|| memcmp(sig_buf+4, "GBMB", 4)) {
			efree(savebuf);
			crx_stream_close(fp);
			if (error) {
				spprintf(error, 0, "crxa \"%s\" has a broken signature", fname);
			}
			return FAILURE;
		}

		CRXA_GET_32(sig_ptr, sig_flags);

		switch(sig_flags) {
			case CRXA_SIG_OPENSSL_SHA512:
			case CRXA_SIG_OPENSSL_SHA256:
			case CRXA_SIG_OPENSSL: {
				uint32_t signature_len;
				char *sig;
				crex_off_t whence;

				/* we store the signature followed by the signature length */
				if (-1 == crx_stream_seek(fp, -12, SEEK_CUR)
				|| 4 != crx_stream_read(fp, sig_buf, 4)) {
					efree(savebuf);
					crx_stream_close(fp);
					if (error) {
						spprintf(error, 0, "crxa \"%s\" openssl signature length could not be read", fname);
					}
					return FAILURE;
				}

				sig_ptr = sig_buf;
				CRXA_GET_32(sig_ptr, signature_len);
				sig = (char *) emalloc(signature_len);
				whence = signature_len + 4;
				whence = -whence;

				if (-1 == crx_stream_seek(fp, whence, SEEK_CUR)
				|| !(end_of_crxa = crx_stream_tell(fp))
				|| signature_len != crx_stream_read(fp, sig, signature_len)) {
					efree(savebuf);
					efree(sig);
					crx_stream_close(fp);
					if (error) {
						spprintf(error, 0, "crxa \"%s\" openssl signature could not be read", fname);
					}
					return FAILURE;
				}

				if (FAILURE == crxa_verify_signature(fp, end_of_crxa, sig_flags, sig, signature_len, fname, &signature, &sig_len, error)) {
					efree(savebuf);
					efree(sig);
					crx_stream_close(fp);
					if (error) {
						char *save = *error;
						spprintf(error, 0, "crxa \"%s\" openssl signature could not be verified: %s", fname, *error);
						efree(save);
					}
					return FAILURE;
				}
				efree(sig);
			}
			break;
			case CRXA_SIG_SHA512: {
				unsigned char digest[64];

				crx_stream_seek(fp, -(8 + 64), SEEK_END);
				read_len = crx_stream_tell(fp);

				if (crx_stream_read(fp, (char*)digest, sizeof(digest)) != sizeof(digest)) {
					efree(savebuf);
					crx_stream_close(fp);
					if (error) {
						spprintf(error, 0, "crxa \"%s\" has a broken signature", fname);
					}
					return FAILURE;
				}

				if (FAILURE == crxa_verify_signature(fp, read_len, CRXA_SIG_SHA512, (char *)digest, 64, fname, &signature, &sig_len, error)) {
					efree(savebuf);
					crx_stream_close(fp);
					if (error) {
						char *save = *error;
						spprintf(error, 0, "crxa \"%s\" SHA512 signature could not be verified: %s", fname, *error);
						efree(save);
					}
					return FAILURE;
				}
				break;
			}
			case CRXA_SIG_SHA256: {
				unsigned char digest[32];

				crx_stream_seek(fp, -(8 + 32), SEEK_END);
				read_len = crx_stream_tell(fp);

				if (crx_stream_read(fp, (char*)digest, sizeof(digest)) != sizeof(digest)) {
					efree(savebuf);
					crx_stream_close(fp);
					if (error) {
						spprintf(error, 0, "crxa \"%s\" has a broken signature", fname);
					}
					return FAILURE;
				}

				if (FAILURE == crxa_verify_signature(fp, read_len, CRXA_SIG_SHA256, (char *)digest, 32, fname, &signature, &sig_len, error)) {
					efree(savebuf);
					crx_stream_close(fp);
					if (error) {
						char *save = *error;
						spprintf(error, 0, "crxa \"%s\" SHA256 signature could not be verified: %s", fname, *error);
						efree(save);
					}
					return FAILURE;
				}
				break;
			}
			case CRXA_SIG_SHA1: {
				unsigned char digest[20];

				crx_stream_seek(fp, -(8 + 20), SEEK_END);
				read_len = crx_stream_tell(fp);

				if (crx_stream_read(fp, (char*)digest, sizeof(digest)) != sizeof(digest)) {
					efree(savebuf);
					crx_stream_close(fp);
					if (error) {
						spprintf(error, 0, "crxa \"%s\" has a broken signature", fname);
					}
					return FAILURE;
				}

				if (FAILURE == crxa_verify_signature(fp, read_len, CRXA_SIG_SHA1, (char *)digest, 20, fname, &signature, &sig_len, error)) {
					efree(savebuf);
					crx_stream_close(fp);
					if (error) {
						char *save = *error;
						spprintf(error, 0, "crxa \"%s\" SHA1 signature could not be verified: %s", fname, *error);
						efree(save);
					}
					return FAILURE;
				}
				break;
			}
			case CRXA_SIG_MD5: {
				unsigned char digest[16];

				crx_stream_seek(fp, -(8 + 16), SEEK_END);
				read_len = crx_stream_tell(fp);

				if (crx_stream_read(fp, (char*)digest, sizeof(digest)) != sizeof(digest)) {
					efree(savebuf);
					crx_stream_close(fp);
					if (error) {
						spprintf(error, 0, "crxa \"%s\" has a broken signature", fname);
					}
					return FAILURE;
				}

				if (FAILURE == crxa_verify_signature(fp, read_len, CRXA_SIG_MD5, (char *)digest, 16, fname, &signature, &sig_len, error)) {
					efree(savebuf);
					crx_stream_close(fp);
					if (error) {
						char *save = *error;
						spprintf(error, 0, "crxa \"%s\" MD5 signature could not be verified: %s", fname, *error);
						efree(save);
					}
					return FAILURE;
				}
				break;
			}
			default:
				efree(savebuf);
				crx_stream_close(fp);

				if (error) {
					spprintf(error, 0, "crxa \"%s\" has a broken or unsupported signature", fname);
				}
				return FAILURE;
		}
	} else if (CRXA_G(require_hash)) {
		efree(savebuf);
		crx_stream_close(fp);

		if (error) {
			spprintf(error, 0, "crxa \"%s\" does not have a signature", fname);
		}
		return FAILURE;
	} else {
		sig_flags = 0;
		sig_len = 0;
	}

	/* extract alias */
	SAFE_CRXA_GET_32(buffer, endbuffer, tmp_len);

	if (buffer + tmp_len > endbuffer) {
		MAPCRXA_FAIL("internal corruption of crxa \"%s\" (buffer overrun)");
	}

	if (manifest_len < MANIFEST_FIXED_LEN + tmp_len) {
		MAPCRXA_FAIL("internal corruption of crxa \"%s\" (truncated manifest header)")
	}

	/* tmp_len = 0 says alias length is 0, which means the alias is not stored in the crxa */
	if (tmp_len) {
		/* if the alias is stored we enforce it (implicit overrides explicit) */
		if (alias && alias_len && (alias_len != tmp_len || strncmp(alias, buffer, tmp_len)))
		{
			crx_stream_close(fp);

			if (signature) {
				efree(signature);
			}

			if (error) {
				spprintf(error, 0, "cannot load crxa \"%s\" with implicit alias \"%.*s\" under different alias \"%s\"", fname, tmp_len, buffer, alias);
			}

			efree(savebuf);
			return FAILURE;
		}

		alias_len = tmp_len;
		alias = buffer;
		buffer += tmp_len;
		register_alias = 1;
	} else if (!alias_len || !alias) {
		/* if we neither have an explicit nor an implicit alias, we use the filename */
		alias = NULL;
		alias_len = 0;
		register_alias = 0;
	} else if (alias_len) {
		register_alias = 1;
		temp_alias = 1;
	}

	/* we have 5 32-bit items plus 1 byte at least */
	if (manifest_count > ((manifest_len - MANIFEST_FIXED_LEN - tmp_len) / (5 * 4 + 1))) {
		/* prevent serious memory issues */
		MAPCRXA_FAIL("internal corruption of crxa \"%s\" (too many manifest entries for size of manifest)")
	}

	mydata = pecalloc(1, sizeof(crxa_archive_data), CRXA_G(persist));
	mydata->is_persistent = CRXA_G(persist);

	/* check whether we have meta data, zero check works regardless of byte order */
	SAFE_CRXA_GET_32(buffer, endbuffer, len);
	if (mydata->is_persistent) {
		if (!len) {
			/* FIXME: not sure why this is needed but removing it breaks tests */
			SAFE_CRXA_GET_32(buffer, endbuffer, len);
		}
	}
	if(len > (size_t)(endbuffer - buffer)) {
		MAPCRXA_FAIL("internal corruption of crxa \"%s\" (trying to read past buffer end)");
	}
	/* Don't implicitly call unserialize() on potentially untrusted input unless getMetadata() is called directly. */
	crxa_parse_metadata_lazy(buffer, &mydata->metadata_tracker, len, mydata->is_persistent);
	buffer += len;

	/* set up our manifest */
	crex_hash_init(&mydata->manifest, manifest_count,
		crex_get_hash_value, destroy_crxa_manifest_entry, (bool)mydata->is_persistent);
	crex_hash_init(&mydata->mounted_dirs, 5,
		crex_get_hash_value, NULL, (bool)mydata->is_persistent);
	crex_hash_init(&mydata->virtual_dirs, manifest_count * 2,
		crex_get_hash_value, NULL, (bool)mydata->is_persistent);
	mydata->fname = pestrndup(fname, fname_len, mydata->is_persistent);
#ifdef CRX_WIN32
	crxa_unixify_path_separators(mydata->fname, fname_len);
#endif
	mydata->fname_len = fname_len;
	offset = halt_offset + manifest_len + 4;
	memset(&entry, 0, sizeof(crxa_entry_info));
	entry.crxa = mydata;
	entry.fp_type = CRXA_FP;
	entry.is_persistent = mydata->is_persistent;

	for (manifest_index = 0; manifest_index < manifest_count; ++manifest_index) {
		if (buffer + 28 > endbuffer) {
			MAPCRXA_FAIL("internal corruption of crxa \"%s\" (truncated manifest entry)")
		}

		CRXA_GET_32(buffer, entry.filename_len);

		if (entry.filename_len == 0) {
			MAPCRXA_FAIL("zero-length filename encountered in crxa \"%s\"");
		}

		if (entry.is_persistent) {
			entry.manifest_pos = manifest_index;
		}

		if (entry.filename_len > (size_t)(endbuffer - buffer - 24)) {
			MAPCRXA_FAIL("internal corruption of crxa \"%s\" (truncated manifest entry)");
		}

		if ((manifest_ver & CRXA_API_VER_MASK) >= CRXA_API_MIN_DIR && buffer[entry.filename_len - 1] == '/') {
			entry.is_dir = 1;
		} else {
			entry.is_dir = 0;
		}

		crxa_add_virtual_dirs(mydata, buffer, entry.filename_len);
		entry.filename = pestrndup(buffer, entry.filename_len, entry.is_persistent);
		buffer += entry.filename_len;
		CRXA_GET_32(buffer, entry.uncompressed_filesize);
		CRXA_GET_32(buffer, entry.timestamp);

		if (offset == halt_offset + manifest_len + 4) {
			mydata->min_timestamp = entry.timestamp;
			mydata->max_timestamp = entry.timestamp;
		} else {
			if (mydata->min_timestamp > entry.timestamp) {
				mydata->min_timestamp = entry.timestamp;
			} else if (mydata->max_timestamp < entry.timestamp) {
				mydata->max_timestamp = entry.timestamp;
			}
		}

		CRXA_GET_32(buffer, entry.compressed_filesize);
		CRXA_GET_32(buffer, entry.crc32);
		CRXA_GET_32(buffer, entry.flags);

		if (entry.is_dir) {
			entry.filename_len--;
			entry.flags |= CRXA_ENT_PERM_DEF_DIR;
		}

		CRXA_GET_32(buffer, len);
		if (len > (size_t)(endbuffer - buffer)) {
			pefree(entry.filename, entry.is_persistent);
			MAPCRXA_FAIL("internal corruption of crxa \"%s\" (truncated manifest entry)");
		}
		/* Don't implicitly call unserialize() on potentially untrusted input unless getMetadata() is called directly. */
		/* The same local variable entry is reused in a loop, so reset the state before reading data. */
		ZVAL_UNDEF(&entry.metadata_tracker.val);
		entry.metadata_tracker.str = NULL;
		crxa_parse_metadata_lazy(buffer, &entry.metadata_tracker, len, entry.is_persistent);
		buffer += len;

		entry.offset = entry.offset_abs = offset;
		offset += entry.compressed_filesize;

		switch (entry.flags & CRXA_ENT_COMPRESSION_MASK) {
			case CRXA_ENT_COMPRESSED_GZ:
				if (!CRXA_G(has_zlib)) {
					crxa_metadata_tracker_free(&entry.metadata_tracker, entry.is_persistent);
					pefree(entry.filename, entry.is_persistent);
					MAPCRXA_FAIL("zlib extension is required for gz compressed .crxa file \"%s\"");
				}
				break;
			case CRXA_ENT_COMPRESSED_BZ2:
				if (!CRXA_G(has_bz2)) {
					crxa_metadata_tracker_free(&entry.metadata_tracker, entry.is_persistent);
					pefree(entry.filename, entry.is_persistent);
					MAPCRXA_FAIL("bz2 extension is required for bzip2 compressed .crxa file \"%s\"");
				}
				break;
			default:
				if (entry.uncompressed_filesize != entry.compressed_filesize) {
					crxa_metadata_tracker_free(&entry.metadata_tracker, entry.is_persistent);
					pefree(entry.filename, entry.is_persistent);
					MAPCRXA_FAIL("internal corruption of crxa \"%s\" (compressed and uncompressed size does not match for uncompressed entry)");
				}
				break;
		}

		manifest_flags |= (entry.flags & CRXA_ENT_COMPRESSION_MASK);
		/* if signature matched, no need to check CRC32 for each file */
		entry.is_crc_checked = (manifest_flags & CRXA_HDR_SIGNATURE ? 1 : 0);
		crxa_set_inode(&entry);
		if (mydata->is_persistent) {
			str = crex_string_init_interned(entry.filename, entry.filename_len, 1);
		} else {
			str = crex_string_init(entry.filename, entry.filename_len, 0);
		}
		crex_hash_add_mem(&mydata->manifest, str, (void*)&entry, sizeof(crxa_entry_info));
		crex_string_release(str);
	}

	snprintf(mydata->version, sizeof(mydata->version), "%u.%u.%u", manifest_ver >> 12, (manifest_ver >> 8) & 0xF, (manifest_ver >> 4) & 0xF);
	mydata->internal_file_start = halt_offset + manifest_len + 4;
	mydata->halt_offset = halt_offset;
	mydata->flags = manifest_flags;
	endbuffer = strrchr(mydata->fname, '/');

	if (endbuffer) {
		mydata->ext = memchr(endbuffer, '.', (mydata->fname + fname_len) - endbuffer);
		if (mydata->ext == endbuffer) {
			mydata->ext = memchr(endbuffer + 1, '.', (mydata->fname + fname_len) - endbuffer - 1);
		}
		if (mydata->ext) {
			mydata->ext_len = (mydata->fname + mydata->fname_len) - mydata->ext;
		}
	}

	mydata->alias = alias ?
		pestrndup(alias, alias_len, mydata->is_persistent) :
		pestrndup(mydata->fname, fname_len, mydata->is_persistent);
	mydata->alias_len = alias ? alias_len : fname_len;
	mydata->sig_flags = sig_flags;
	mydata->fp = fp;
	mydata->sig_len = sig_len;
	mydata->signature = signature;
	crxa_request_initialize();

	if (register_alias) {
		crxa_archive_data *fd_ptr;

		mydata->is_temporary_alias = temp_alias;

		if (!crxa_validate_alias(mydata->alias, mydata->alias_len)) {
			signature = NULL;
			fp = NULL;
			MAPCRXA_FAIL("Cannot open archive \"%s\", invalid alias");
		}

		if (NULL != (fd_ptr = crex_hash_str_find_ptr(&(CRXA_G(crxa_alias_map)), alias, alias_len))) {
			if (SUCCESS != crxa_free_alias(fd_ptr, alias, alias_len)) {
				signature = NULL;
				fp = NULL;
				MAPCRXA_FAIL("Cannot open archive \"%s\", alias is already in use by existing archive");
			}
		}

		if (mydata->is_persistent) {
			str = crex_string_init_interned(alias, alias_len, 1);
		} else {
			str = crex_string_init(alias, alias_len, 0);
		}
		crex_hash_add_ptr(&(CRXA_G(crxa_alias_map)), str, mydata);
		crex_string_release(str);
	} else {
		mydata->is_temporary_alias = 1;
	}

	if (mydata->is_persistent) {
		str = crex_string_init_interned(mydata->fname, fname_len, 1);
	} else {
		str = crex_string_init(mydata->fname, fname_len, 0);
	}
	crex_hash_add_ptr(&(CRXA_G(crxa_fname_map)), str, mydata);
	crex_string_release(str);
	efree(savebuf);

	if (pcrxa) {
		*pcrxa = mydata;
	}

	return SUCCESS;
}
/* }}} */

/**
 * Create or open a crxa for writing
 */
int crxa_open_or_create_filename(char *fname, size_t fname_len, char *alias, size_t alias_len, bool is_data, uint32_t options, crxa_archive_data** pcrxa, char **error) /* {{{ */
{
	const char *ext_str, *z;
	char *my_error;
	size_t ext_len;
	crxa_archive_data **test, *unused = NULL;

	test = &unused;

	if (error) {
		*error = NULL;
	}

	/* first try to open an existing file */
	if (crxa_detect_crxa_fname_ext(fname, fname_len, &ext_str, &ext_len, !is_data, 0, 1) == SUCCESS) {
		goto check_file;
	}

	/* next try to create a new file */
	if (FAILURE == crxa_detect_crxa_fname_ext(fname, fname_len, &ext_str, &ext_len, !is_data, 1, 1)) {
		if (error) {
			if (ext_len == -2) {
				spprintf(error, 0, "Cannot create a crxa archive from a URL like \"%s\". Crxa objects can only be created from local files", fname);
			} else {
				spprintf(error, 0, "Cannot create crxa '%s', file extension (or combination) not recognised or the directory does not exist", fname);
			}
		}
		return FAILURE;
	}
check_file:
	if (crxa_open_parsed_crxa(fname, fname_len, alias, alias_len, is_data, options, test, &my_error) == SUCCESS) {
		if (pcrxa) {
			*pcrxa = *test;
		}

		if ((*test)->is_data && !(*test)->is_tar && !(*test)->is_zip) {
			if (error) {
				spprintf(error, 0, "Cannot open '%s' as a CrxaData object. Use Crxa::__main() for executable archives", fname);
			}
			return FAILURE;
		}

		if (CRXA_G(readonly) && !(*test)->is_data && ((*test)->is_tar || (*test)->is_zip)) {
			crxa_entry_info *stub;
			if (NULL == (stub = crex_hash_str_find_ptr(&((*test)->manifest), ".crxa/stub.crx", sizeof(".crxa/stub.crx")-1))) {
				spprintf(error, 0, "'%s' is not a crxa archive. Use CrxaData::__main() for a standard zip or tar archive", fname);
				return FAILURE;
			}
		}

		if (!CRXA_G(readonly) || (*test)->is_data) {
			(*test)->is_writeable = 1;
		}
		return SUCCESS;
	} else if (my_error) {
		if (error) {
			*error = my_error;
		} else {
			efree(my_error);
		}
		return FAILURE;
	}

	if (ext_len > 3 && (z = memchr(ext_str, 'z', ext_len)) && ((ext_str + ext_len) - z >= 2) && !memcmp(z + 1, "ip", 2)) {
		/* assume zip-based crxa */
		return crxa_open_or_create_zip(fname, fname_len, alias, alias_len, is_data, options, pcrxa, error);
	}

	if (ext_len > 3 && (z = memchr(ext_str, 't', ext_len)) && ((ext_str + ext_len) - z >= 2) && !memcmp(z + 1, "ar", 2)) {
		/* assume tar-based crxa */
		return crxa_open_or_create_tar(fname, fname_len, alias, alias_len, is_data, options, pcrxa, error);
	}

	return crxa_create_or_parse_filename(fname, fname_len, alias, alias_len, is_data, options, pcrxa, error);
}
/* }}} */

int crxa_create_or_parse_filename(char *fname, size_t fname_len, char *alias, size_t alias_len, bool is_data, uint32_t options, crxa_archive_data** pcrxa, char **error) /* {{{ */
{
	crxa_archive_data *mydata;
	crx_stream *fp;
	crex_string *actual = NULL;
	char *p;

	if (!pcrxa) {
		pcrxa = &mydata;
	}
	if (crx_check_open_basedir(fname)) {
		return FAILURE;
	}

	/* first open readonly so it won't be created if not present */
	fp = crx_stream_open_wrapper(fname, "rb", IGNORE_URL|STREAM_MUST_SEEK|0, &actual);

	if (actual) {
		fname = ZSTR_VAL(actual);
		fname_len = ZSTR_LEN(actual);
	}

	if (fp) {
		if (crxa_open_from_fp(fp, fname, fname_len, alias, alias_len, options, pcrxa, is_data, error) == SUCCESS) {
			if ((*pcrxa)->is_data || !CRXA_G(readonly)) {
				(*pcrxa)->is_writeable = 1;
			}
			if (actual) {
				crex_string_release_ex(actual, 0);
			}
			return SUCCESS;
		} else {
			/* file exists, but is either corrupt or not a crxa archive */
			if (actual) {
				crex_string_release_ex(actual, 0);
			}
			return FAILURE;
		}
	}

	if (actual) {
		crex_string_release_ex(actual, 0);
	}

	if (CRXA_G(readonly) && !is_data) {
		if (options & REPORT_ERRORS) {
			if (error) {
				spprintf(error, 0, "creating archive \"%s\" disabled by the crx.ini setting crxa.readonly", fname);
			}
		}
		return FAILURE;
	}

	/* set up our manifest */
	mydata = ecalloc(1, sizeof(crxa_archive_data));
	mydata->fname = expand_filepath(fname, NULL);
	if (mydata->fname == NULL) {
		efree(mydata);
		return FAILURE;
	}
	fname_len = strlen(mydata->fname);
#ifdef CRX_WIN32
	crxa_unixify_path_separators(mydata->fname, fname_len);
#endif
	p = strrchr(mydata->fname, '/');

	if (p) {
		mydata->ext = memchr(p, '.', (mydata->fname + fname_len) - p);
		if (mydata->ext == p) {
			mydata->ext = memchr(p + 1, '.', (mydata->fname + fname_len) - p - 1);
		}
		if (mydata->ext) {
			mydata->ext_len = (mydata->fname + fname_len) - mydata->ext;
		}
	}

	if (pcrxa) {
		*pcrxa = mydata;
	}

	crex_hash_init(&mydata->manifest, sizeof(crxa_entry_info),
		crex_get_hash_value, destroy_crxa_manifest_entry, 0);
	crex_hash_init(&mydata->mounted_dirs, sizeof(char *),
		crex_get_hash_value, NULL, 0);
	crex_hash_init(&mydata->virtual_dirs, sizeof(char *),
		crex_get_hash_value, NULL, (bool)mydata->is_persistent);
	mydata->fname_len = fname_len;
	snprintf(mydata->version, sizeof(mydata->version), "%s", CRX_CRXA_API_VERSION);
	mydata->is_temporary_alias = alias ? 0 : 1;
	mydata->internal_file_start = -1;
	mydata->fp = NULL;
	mydata->is_writeable = 1;
	mydata->is_brandnew = 1;
	crxa_request_initialize();
	crex_hash_str_add_ptr(&(CRXA_G(crxa_fname_map)), mydata->fname, fname_len, mydata);

	if (is_data) {
		alias = NULL;
		alias_len = 0;
		mydata->is_data = 1;
		/* assume tar format, CrxaData can specify other */
		mydata->is_tar = 1;
	} else {
		crxa_archive_data *fd_ptr;

		if (alias && NULL != (fd_ptr = crex_hash_str_find_ptr(&(CRXA_G(crxa_alias_map)), alias, alias_len))) {
			if (SUCCESS != crxa_free_alias(fd_ptr, alias, alias_len)) {
				if (error) {
					spprintf(error, 4096, "crxa error: crxa \"%s\" cannot set alias \"%s\", already in use by another crxa archive", mydata->fname, alias);
				}

				crex_hash_str_del(&(CRXA_G(crxa_fname_map)), mydata->fname, fname_len);

				if (pcrxa) {
					*pcrxa = NULL;
				}

				return FAILURE;
			}
		}

		mydata->alias = alias ? estrndup(alias, alias_len) : estrndup(mydata->fname, fname_len);
		mydata->alias_len = alias ? alias_len : fname_len;
	}

	if (alias_len && alias) {
		if (NULL == crex_hash_str_add_ptr(&(CRXA_G(crxa_alias_map)), alias, alias_len, mydata)) {
			if (options & REPORT_ERRORS) {
				if (error) {
					spprintf(error, 0, "archive \"%s\" cannot be associated with alias \"%s\", already in use", fname, alias);
				}
			}

			crex_hash_str_del(&(CRXA_G(crxa_fname_map)), mydata->fname, fname_len);

			if (pcrxa) {
				*pcrxa = NULL;
			}

			return FAILURE;
		}
	}

	return SUCCESS;
}
/* }}}*/

/**
 * Return an already opened filename.
 *
 * Or scan a crxa file for the required __HALT_COMPILER(); ?> token and verify
 * that the manifest is proper, then pass it to crxa_parse_crxafile().  SUCCESS
 * or FAILURE is returned and pcrxa is set to a pointer to the crxa's manifest
 */
int crxa_open_from_filename(char *fname, size_t fname_len, char *alias, size_t alias_len, uint32_t options, crxa_archive_data** pcrxa, char **error) /* {{{ */
{
	crx_stream *fp;
	crex_string *actual;
	int ret, is_data = 0;

	if (error) {
		*error = NULL;
	}

	if (!strstr(fname, ".crxa")) {
		is_data = 1;
	}

	if (crxa_open_parsed_crxa(fname, fname_len, alias, alias_len, is_data, options, pcrxa, error) == SUCCESS) {
		return SUCCESS;
	} else if (error && *error) {
		return FAILURE;
	}
	if (crx_check_open_basedir(fname)) {
		return FAILURE;
	}

	fp = crx_stream_open_wrapper(fname, "rb", IGNORE_URL|STREAM_MUST_SEEK, &actual);

	if (!fp) {
		if (options & REPORT_ERRORS) {
			if (error) {
				spprintf(error, 0, "unable to open crxa for reading \"%s\"", fname);
			}
		}
		if (actual) {
			crex_string_release_ex(actual, 0);
		}
		return FAILURE;
	}

	if (actual) {
		fname = ZSTR_VAL(actual);
		fname_len = ZSTR_LEN(actual);
	}

	ret =  crxa_open_from_fp(fp, fname, fname_len, alias, alias_len, options, pcrxa, is_data, error);

	if (actual) {
		crex_string_release_ex(actual, 0);
	}

	return ret;
}
/* }}}*/

static inline char *crxa_strnstr(const char *buf, int buf_len, const char *search, int search_len) /* {{{ */
{
	const char *c;
	ptrdiff_t so_far = 0;

	if (buf_len < search_len) {
		return NULL;
	}

	c = buf - 1;

	do {
		if (!(c = memchr(c + 1, search[0], buf_len - search_len - so_far))) {
			return (char *) NULL;
		}

		so_far = c - buf;

		if (so_far >= (buf_len - search_len)) {
			return (char *) NULL;
		}

		if (!memcmp(c, search, search_len)) {
			return (char *) c;
		}
	} while (1);
}
/* }}} */

/**
 * Scan an open fp for the required __HALT_COMPILER(); ?> token and verify
 * that the manifest is proper, then pass it to crxa_parse_crxafile().  SUCCESS
 * or FAILURE is returned and pcrxa is set to a pointer to the crxa's manifest
 */
static int crxa_open_from_fp(crx_stream* fp, char *fname, size_t fname_len, char *alias, size_t alias_len, uint32_t options, crxa_archive_data** pcrxa, int is_data, char **error) /* {{{ */
{
	static const char token[] = "__HALT_COMPILER();";
	static const char zip_magic[] = "PK\x03\x04";
	static const char gz_magic[] = "\x1f\x8b\x08";
	static const char bz_magic[] = "BZh";
	char *pos, test = '\0';
	int recursion_count = 3; // arbitrary limit to avoid too deep or even infinite recursion
	const int window_size = 1024;
	char buffer[1024 + sizeof(token)]; /* a 1024 byte window + the size of the halt_compiler token (moving window) */
	const crex_long readsize = sizeof(buffer) - sizeof(token);
	const crex_long tokenlen = sizeof(token) - 1;
	crex_long halt_offset;
	size_t got;
	uint32_t compression = CRXA_FILE_COMPRESSED_NONE;

	if (error) {
		*error = NULL;
	}

	if (-1 == crx_stream_rewind(fp)) {
		MAPCRXA_ALLOC_FAIL("cannot rewind crxa \"%s\"")
	}

	buffer[sizeof(buffer)-1] = '\0';
	memset(buffer, 32, sizeof(token));
	halt_offset = 0;

	/* Maybe it's better to compile the file instead of just searching,  */
	/* but we only want the offset. So we want a .re scanner to find it. */
	while(!crx_stream_eof(fp)) {
		if ((got = crx_stream_read(fp, buffer+tokenlen, readsize)) < (size_t) tokenlen) {
			MAPCRXA_ALLOC_FAIL("internal corruption of crxa \"%s\" (truncated entry)")
		}

		if (!test && recursion_count) {
			test = '\1';
			pos = buffer+tokenlen;
			if (!memcmp(pos, gz_magic, 3)) {
				char err = 0;
				crx_stream_filter *filter;
				crx_stream *temp;
				/* to properly decompress, we have to tell zlib to look for a zlib or gzip header */
				zval filterparams;

				if (!CRXA_G(has_zlib)) {
					MAPCRXA_ALLOC_FAIL("unable to decompress gzipped crxa archive \"%s\" to temporary file, enable zlib extension in crx.ini")
				}
				array_init(&filterparams);
/* this is defined in zlib's zconf.h */
#ifndef MAX_WBITS
#define MAX_WBITS 15
#endif
				add_assoc_long_ex(&filterparams, "window", sizeof("window") - 1, MAX_WBITS + 32);

				/* entire file is gzip-compressed, uncompress to temporary file */
				if (!(temp = crx_stream_fopen_tmpfile())) {
					MAPCRXA_ALLOC_FAIL("unable to create temporary file for decompression of gzipped crxa archive \"%s\"")
				}

				crx_stream_rewind(fp);
				filter = crx_stream_filter_create("zlib.inflate", &filterparams, crx_stream_is_persistent(fp));

				if (!filter) {
					err = 1;
					add_assoc_long_ex(&filterparams, "window", sizeof("window") - 1, MAX_WBITS);
					filter = crx_stream_filter_create("zlib.inflate", &filterparams, crx_stream_is_persistent(fp));
					crex_array_destroy(C_ARR(filterparams));

					if (!filter) {
						crx_stream_close(temp);
						MAPCRXA_ALLOC_FAIL("unable to decompress gzipped crxa archive \"%s\", ext/zlib is buggy in CRX versions older than 5.2.6")
					}
				} else {
					crex_array_destroy(C_ARR(filterparams));
				}

				crx_stream_filter_append(&temp->writefilters, filter);

				if (SUCCESS != crx_stream_copy_to_stream_ex(fp, temp, CRX_STREAM_COPY_ALL, NULL)) {
					if (err) {
						crx_stream_close(temp);
						MAPCRXA_ALLOC_FAIL("unable to decompress gzipped crxa archive \"%s\", ext/zlib is buggy in CRX versions older than 5.2.6")
					}
					crx_stream_close(temp);
					MAPCRXA_ALLOC_FAIL("unable to decompress gzipped crxa archive \"%s\" to temporary file")
				}

				crx_stream_filter_flush(filter, 1);
				crx_stream_filter_remove(filter, 1);
				crx_stream_close(fp);
				fp = temp;
				crx_stream_rewind(fp);
				compression = CRXA_FILE_COMPRESSED_GZ;

				/* now, start over */
				test = '\0';
				if (!--recursion_count) {
					MAPCRXA_ALLOC_FAIL("unable to decompress gzipped crxa archive \"%s\"");
					break;
				}
				continue;
			} else if (!memcmp(pos, bz_magic, 3)) {
				crx_stream_filter *filter;
				crx_stream *temp;

				if (!CRXA_G(has_bz2)) {
					MAPCRXA_ALLOC_FAIL("unable to decompress bzipped crxa archive \"%s\" to temporary file, enable bz2 extension in crx.ini")
				}

				/* entire file is bzip-compressed, uncompress to temporary file */
				if (!(temp = crx_stream_fopen_tmpfile())) {
					MAPCRXA_ALLOC_FAIL("unable to create temporary file for decompression of bzipped crxa archive \"%s\"")
				}

				crx_stream_rewind(fp);
				filter = crx_stream_filter_create("bzip2.decompress", NULL, crx_stream_is_persistent(fp));

				if (!filter) {
					crx_stream_close(temp);
					MAPCRXA_ALLOC_FAIL("unable to decompress bzipped crxa archive \"%s\", filter creation failed")
				}

				crx_stream_filter_append(&temp->writefilters, filter);

				if (SUCCESS != crx_stream_copy_to_stream_ex(fp, temp, CRX_STREAM_COPY_ALL, NULL)) {
					crx_stream_close(temp);
					MAPCRXA_ALLOC_FAIL("unable to decompress bzipped crxa archive \"%s\" to temporary file")
				}

				crx_stream_filter_flush(filter, 1);
				crx_stream_filter_remove(filter, 1);
				crx_stream_close(fp);
				fp = temp;
				crx_stream_rewind(fp);
				compression = CRXA_FILE_COMPRESSED_BZ2;

				/* now, start over */
				test = '\0';
				if (!--recursion_count) {
					MAPCRXA_ALLOC_FAIL("unable to decompress bzipped crxa archive \"%s\"");
					break;
				}
				continue;
			}

			if (!memcmp(pos, zip_magic, 4)) {
				crx_stream_seek(fp, 0, SEEK_END);
				return crxa_parse_zipfile(fp, fname, fname_len, alias, alias_len, pcrxa, error);
			}

			if (got > 512) {
				if (crxa_is_tar(pos, fname)) {
					crx_stream_rewind(fp);
					return crxa_parse_tarfile(fp, fname, fname_len, alias, alias_len, pcrxa, is_data, compression, error);
				}
			}
		}

		if (got > 0 && (pos = crxa_strnstr(buffer, got + sizeof(token), token, sizeof(token)-1)) != NULL) {
			halt_offset += (pos - buffer); /* no -tokenlen+tokenlen here */
			return crxa_parse_crxafile(fp, fname, fname_len, alias, alias_len, halt_offset, pcrxa, compression, error);
		}

		halt_offset += got;
		memmove(buffer, buffer + window_size, tokenlen); /* move the memory buffer by the size of the window */
	}

	MAPCRXA_ALLOC_FAIL("internal corruption of crxa \"%s\" (__HALT_COMPILER(); not found)")
}
/* }}} */

/*
 * given the location of the file extension and the start of the file path,
 * determine the end of the portion of the path (i.e. /path/to/file.ext/blah
 * grabs "/path/to/file.ext" as does the straight /path/to/file.ext),
 * stat it to determine if it exists.
 * if so, check to see if it is a directory and fail if so
 * if not, check to see if its dirname() exists (i.e. "/path/to") and is a directory
 * succeed if we are creating the file, otherwise fail.
 */
static int crxa_analyze_path(const char *fname, const char *ext, size_t ext_len, int for_create) /* {{{ */
{
	crx_stream_statbuf ssb;
	char *realpath;
	char *filename = estrndup(fname, (ext - fname) + ext_len);

	if ((realpath = expand_filepath(filename, NULL))) {
#ifdef CRX_WIN32
		crxa_unixify_path_separators(realpath, strlen(realpath));
#endif
		if (crex_hash_str_exists(&(CRXA_G(crxa_fname_map)), realpath, strlen(realpath))) {
			efree(realpath);
			efree(filename);
			return SUCCESS;
		}

		if (CRXA_G(manifest_cached) && crex_hash_str_exists(&cached_crxas, realpath, strlen(realpath))) {
			efree(realpath);
			efree(filename);
			return SUCCESS;
		}
		efree(realpath);
	}

	if (SUCCESS == crx_stream_stat_path((char *) filename, &ssb)) {

		efree(filename);

		if (ssb.sb.st_mode & S_IFDIR) {
			return FAILURE;
		}

		if (for_create == 1) {
			return FAILURE;
		}

		return SUCCESS;
	} else {
		char *slash;

		if (!for_create) {
			efree(filename);
			return FAILURE;
		}

		slash = (char *) strrchr(filename, '/');

		if (slash) {
			*slash = '\0';
		}

		if (SUCCESS != crx_stream_stat_path((char *) filename, &ssb)) {
			if (!slash) {
				if (!(realpath = expand_filepath(filename, NULL))) {
					efree(filename);
					return FAILURE;
				}
#ifdef CRX_WIN32
				crxa_unixify_path_separators(realpath, strlen(realpath));
#endif
				slash = strstr(realpath, filename);
				if (slash) {
					slash += ((ext - fname) + ext_len);
					*slash = '\0';
				}
				slash = strrchr(realpath, '/');

				if (slash) {
					*slash = '\0';
				} else {
					efree(realpath);
					efree(filename);
					return FAILURE;
				}

				if (SUCCESS != crx_stream_stat_path(realpath, &ssb)) {
					efree(realpath);
					efree(filename);
					return FAILURE;
				}

				efree(realpath);

				if (ssb.sb.st_mode & S_IFDIR) {
					efree(filename);
					return SUCCESS;
				}
			}

			efree(filename);
			return FAILURE;
		}

		efree(filename);

		if (ssb.sb.st_mode & S_IFDIR) {
			return SUCCESS;
		}

		return FAILURE;
	}
}
/* }}} */

/* check for ".crxa" in extension */
static int crxa_check_str(const char *fname, const char *ext_str, size_t ext_len, int executable, int for_create) /* {{{ */
{
	const char *pos;

	if (ext_len >= 50) {
		return FAILURE;
	}
	if (executable == 1) {
		/* executable crxas must contain ".crxa" as a valid extension (crxa://.crxamy/oops is invalid) */
		/* (crxa://hi/there/.crxa/oops is also invalid) */
		pos = strstr(ext_str, ".crxa");

		if (!pos
			|| (pos != ext_str && (*(pos - 1) == '/'))
			|| (ext_len - (pos - ext_str)) < 5
			|| !(pos += 5)
			|| !(*pos == '\0' || *pos == '/' || *pos == '.')) {
			return FAILURE;
		}
		return crxa_analyze_path(fname, ext_str, ext_len, for_create);
	}

	/* data crxas need only contain a single non-"." to be valid */
	if (!executable) {
		pos = strstr(ext_str, ".crxa");
		if (!(pos && (*(pos - 1) != '/')
					&& (pos += 5) && (*pos == '\0' || *pos == '/' || *pos == '.')) && *(ext_str + 1) != '.' && *(ext_str + 1) != '/' && *(ext_str + 1) != '\0') {
			return crxa_analyze_path(fname, ext_str, ext_len, for_create);
		}
	} else {
		if (*(ext_str + 1) != '.' && *(ext_str + 1) != '/' && *(ext_str + 1) != '\0') {
			return crxa_analyze_path(fname, ext_str, ext_len, for_create);
		}
	}

	return FAILURE;
}
/* }}} */

/*
 * if executable is 1, only returns SUCCESS if the extension is one of the tar/zip .crxa extensions
 * if executable is 0, it returns SUCCESS only if the filename does *not* contain ".crxa" anywhere, and treats
 * the first extension as the filename extension
 *
 * if an extension is found, it sets ext_str to the location of the file extension in filename,
 * and ext_len to the length of the extension.
 * for urls like "crxa://alias/oops" it instead sets ext_len to -1 and returns FAILURE, which tells
 * the calling function to use "alias" as the crxa alias
 *
 * the last parameter should be set to tell the thing to assume that filename is the full path, and only to check the
 * extension rules, not to iterate.
 */
int crxa_detect_crxa_fname_ext(const char *filename, size_t filename_len, const char **ext_str, size_t *ext_len, int executable, int for_create, int is_complete) /* {{{ */
{
	const char *pos, *slash;

	*ext_str = NULL;
	*ext_len = 0;

	if (filename_len <= 1) {
		return FAILURE;
	}

	crxa_request_initialize();
	/* first check for alias in first segment */
	pos = memchr(filename, '/', filename_len);

	if (pos && pos != filename) {
		/* check for url like http:// or crxa:// */
		if (*(pos - 1) == ':' && (size_t)(pos - filename) < filename_len - 1 && *(pos + 1) == '/') {
			*ext_len = -2;
			*ext_str = NULL;
			return FAILURE;
		}
		if (crex_hash_str_exists(&(CRXA_G(crxa_alias_map)), (char *) filename, pos - filename)) {
			*ext_str = pos;
			*ext_len = -1;
			return FAILURE;
		}

		if (CRXA_G(manifest_cached) && crex_hash_str_exists(&cached_alias, (char *) filename, pos - filename)) {
			*ext_str = pos;
			*ext_len = -1;
			return FAILURE;
		}
	}

	if (crex_hash_num_elements(&(CRXA_G(crxa_fname_map))) || CRXA_G(manifest_cached)) {
		crxa_archive_data *pcrxa;

		if (is_complete) {
			if (NULL != (pcrxa = crex_hash_str_find_ptr(&(CRXA_G(crxa_fname_map)), (char *) filename, filename_len))) {
				*ext_str = filename + (filename_len - pcrxa->ext_len);
woohoo:
				*ext_len = pcrxa->ext_len;

				if (executable == 2) {
					return SUCCESS;
				}

				if (executable == 1 && !pcrxa->is_data) {
					return SUCCESS;
				}

				if (!executable && pcrxa->is_data) {
					return SUCCESS;
				}

				return FAILURE;
			}

			if (CRXA_G(manifest_cached) && NULL != (pcrxa = crex_hash_str_find_ptr(&cached_crxas, (char *) filename, filename_len))) {
				*ext_str = filename + (filename_len - pcrxa->ext_len);
				goto woohoo;
			}
		} else {
			crex_string *str_key;

			CREX_HASH_MAP_FOREACH_STR_KEY_PTR(&CRXA_G(crxa_fname_map), str_key, pcrxa) {
				if (ZSTR_LEN(str_key) > (uint32_t) filename_len) {
					continue;
				}

				if (!memcmp(filename, ZSTR_VAL(str_key), ZSTR_LEN(str_key)) && ((uint32_t)filename_len == ZSTR_LEN(str_key)
					|| filename[ZSTR_LEN(str_key)] == '/' || filename[ZSTR_LEN(str_key)] == '\0')) {
					*ext_str = filename + (ZSTR_LEN(str_key) - pcrxa->ext_len);
					goto woohoo;
				}
			} CREX_HASH_FOREACH_END();

			if (CRXA_G(manifest_cached)) {
				CREX_HASH_MAP_FOREACH_STR_KEY_PTR(&cached_crxas, str_key, pcrxa) {
					if (ZSTR_LEN(str_key) > (uint32_t) filename_len) {
						continue;
					}

					if (!memcmp(filename, ZSTR_VAL(str_key), ZSTR_LEN(str_key)) && ((uint32_t)filename_len == ZSTR_LEN(str_key)
						|| filename[ZSTR_LEN(str_key)] == '/' || filename[ZSTR_LEN(str_key)] == '\0')) {
						*ext_str = filename + (ZSTR_LEN(str_key) - pcrxa->ext_len);
						goto woohoo;
					}
				} CREX_HASH_FOREACH_END();
			}
		}
	}

	pos = memchr(filename + 1, '.', filename_len);
next_extension:
	if (!pos) {
		return FAILURE;
	}

	while (pos != filename && (*(pos - 1) == '/' || *(pos - 1) == '\0')) {
		pos = memchr(pos + 1, '.', filename_len - (pos - filename) - 1);
		if (!pos) {
			return FAILURE;
		}
	}

	slash = memchr(pos, '/', filename_len - (pos - filename));

	if (!slash) {
		/* this is a url like "crxa://blah.crxa" with no directory */
		*ext_str = pos;
		*ext_len = strlen(pos);

		/* file extension must contain "crxa" */
		switch (crxa_check_str(filename, *ext_str, *ext_len, executable, for_create)) {
			case SUCCESS:
				return SUCCESS;
			case FAILURE:
				/* we are at the end of the string, so we fail */
				return FAILURE;
		}
	}

	/* we've found an extension that ends at a directory separator */
	*ext_str = pos;
	*ext_len = slash - pos;

	switch (crxa_check_str(filename, *ext_str, *ext_len, executable, for_create)) {
		case SUCCESS:
			return SUCCESS;
		case FAILURE:
			/* look for more extensions */
			pos = strchr(pos + 1, '.');
			if (pos) {
				*ext_str = NULL;
				*ext_len = 0;
			}
			goto next_extension;
	}

	return FAILURE;
}
/* }}} */

static int crx_check_dots(const char *element, size_t n) /* {{{ */
{
	for(n-- ; n != SIZE_MAX; --n) {
		if (element[n] != '.') {
			return 1;
		}
	}
	return 0;
}
/* }}} */

#define IS_DIRECTORY_UP(element, len) \
	(len >= 2 && !crx_check_dots(element, len))

#define IS_DIRECTORY_CURRENT(element, len) \
	(len == 1 && element[0] == '.')

#define IS_BACKSLASH(c) ((c) == '/')

/**
 * Remove .. and . references within a crxa filename
 */
char *crxa_fix_filepath(char *path, size_t *new_len, int use_cwd) /* {{{ */
{
	char *newpath;
	size_t newpath_len;
	char *ptr;
	char *tok;
	size_t ptr_length, path_length = *new_len;

	if (CRXA_G(cwd_len) && use_cwd && path_length > 2 && path[0] == '.' && path[1] == '/') {
		newpath_len = CRXA_G(cwd_len);
		newpath = emalloc(strlen(path) + newpath_len + 1);
		memcpy(newpath, CRXA_G(cwd), newpath_len);
	} else {
		newpath = emalloc(strlen(path) + 2);
		newpath[0] = '/';
		newpath_len = 1;
	}

	ptr = path;

	if (*ptr == '/') {
		++ptr;
	}

	tok = ptr;

	do {
		ptr = memchr(ptr, '/', path_length - (ptr - path));
	} while (ptr && ptr - tok == 0 && *ptr == '/' && ++ptr && ++tok);

	if (!ptr && (path_length - (tok - path))) {
		switch (path_length - (tok - path)) {
			case 1:
				if (*tok == '.') {
					efree(path);
					*new_len = 1;
					efree(newpath);
					return estrndup("/", 1);
				}
				break;
			case 2:
				if (tok[0] == '.' && tok[1] == '.') {
					efree(path);
					*new_len = 1;
					efree(newpath);
					return estrndup("/", 1);
				}
		}
		efree(newpath);
		return path;
	}

	while (ptr) {
		ptr_length = ptr - tok;
last_time:
		if (IS_DIRECTORY_UP(tok, ptr_length)) {
#define PREVIOUS newpath[newpath_len - 1]

			while (newpath_len > 1 && !IS_BACKSLASH(PREVIOUS)) {
				newpath_len--;
			}

			if (newpath[0] != '/') {
				newpath[newpath_len] = '\0';
			} else if (newpath_len > 1) {
				--newpath_len;
			}
		} else if (!IS_DIRECTORY_CURRENT(tok, ptr_length)) {
			if (newpath_len > 1) {
				newpath[newpath_len++] = '/';
				memcpy(newpath + newpath_len, tok, ptr_length+1);
			} else {
				memcpy(newpath + newpath_len, tok, ptr_length+1);
			}

			newpath_len += ptr_length;
		}

		if (ptr == path + path_length) {
			break;
		}

		tok = ++ptr;

		do {
			ptr = memchr(ptr, '/', path_length - (ptr - path));
		} while (ptr && ptr - tok == 0 && *ptr == '/' && ++ptr && ++tok);

		if (!ptr && (path_length - (tok - path))) {
			ptr_length = path_length - (tok - path);
			ptr = path + path_length;
			goto last_time;
		}
	}

	efree(path);
	*new_len = newpath_len;
	newpath[newpath_len] = '\0';
	return erealloc(newpath, newpath_len + 1);
}
/* }}} */

/**
 * Process a crxa stream name, ensuring we can handle any of:
 *
 * - whatever.crxa
 * - whatever.crxa.gz
 * - whatever.crxa.bz2
 * - whatever.crxa.crx
 *
 * Optionally the name might start with 'crxa://'
 *
 * This is used by crxa_parse_url()
 */
int crxa_split_fname(const char *filename, size_t filename_len, char **arch, size_t *arch_len, char **entry, size_t *entry_len, int executable, int for_create) /* {{{ */
{
	const char *ext_str;
#ifdef CRX_WIN32
	char *save;
#endif
	size_t ext_len;

	if (CHECK_NULL_PATH(filename, filename_len)) {
		return FAILURE;
	}

	if (!strncasecmp(filename, "crxa://", 7)) {
		filename += 7;
		filename_len -= 7;
	}

	ext_len = 0;
#ifdef CRX_WIN32
	save = (char *)filename;
	if (memchr(filename, '\\', filename_len)) {
		filename = estrndup(filename, filename_len);
		crxa_unixify_path_separators((char *)filename, filename_len);
	}
#endif
	if (crxa_detect_crxa_fname_ext(filename, filename_len, &ext_str, &ext_len, executable, for_create, 0) == FAILURE) {
		if (ext_len != -1) {
			if (!ext_str) {
				/* no / detected, restore arch for error message */
#ifdef CRX_WIN32
				*arch = save;
#else
				*arch = (char*)filename;
#endif
			}

#ifdef CRX_WIN32
			if (filename != save) {
				efree((char *)filename);
			}
#endif
			return FAILURE;
		}

		ext_len = 0;
		/* no extension detected - instead we are dealing with an alias */
	}

	*arch_len = ext_str - filename + ext_len;
	*arch = estrndup(filename, *arch_len);

	if (ext_str[ext_len]) {
		*entry_len = filename_len - *arch_len;
		*entry = estrndup(ext_str+ext_len, *entry_len);
#ifdef CRX_WIN32
		crxa_unixify_path_separators(*entry, *entry_len);
#endif
		*entry = crxa_fix_filepath(*entry, entry_len, 0);
	} else {
		*entry_len = 1;
		*entry = estrndup("/", 1);
	}

#ifdef CRX_WIN32
	if (filename != save) {
		efree((char *)filename);
	}
#endif

	return SUCCESS;
}
/* }}} */

/**
 * Invoked when a user calls Crxa::mapCrxa() from within an executing .crxa
 * to set up its manifest directly
 */
int crxa_open_executed_filename(char *alias, size_t alias_len, char **error) /* {{{ */
{
	if (error) {
		*error = NULL;
	}

	crex_string *fname = crex_get_executed_filename_ex();

	if (!fname) {
		if (error) {
			spprintf(error, 0, "cannot initialize a crxa outside of CRX execution");
		}
		return FAILURE;
	}

	if (crxa_open_parsed_crxa(ZSTR_VAL(fname), ZSTR_LEN(fname), alias, alias_len, 0, REPORT_ERRORS, NULL, 0) == SUCCESS) {
		return SUCCESS;
	}

	if (0 == crex_get_constant_str("__COMPILER_HALT_OFFSET__", sizeof("__COMPILER_HALT_OFFSET__")-1)) {
		if (error) {
			spprintf(error, 0, "__HALT_COMPILER(); must be declared in a crxa");
		}
		return FAILURE;
	}

	if (crx_check_open_basedir(ZSTR_VAL(fname))) {
		return FAILURE;
	}

	crex_string *actual = NULL;
	crx_stream *fp;
	fp = crx_stream_open_wrapper(ZSTR_VAL(fname), "rb", IGNORE_URL|STREAM_MUST_SEEK|REPORT_ERRORS, &actual);

	if (!fp) {
		if (error) {
			spprintf(error, 0, "unable to open crxa for reading \"%s\"", ZSTR_VAL(fname));
		}
		if (actual) {
			crex_string_release_ex(actual, 0);
		}
		return FAILURE;
	}

	if (actual) {
		fname = actual;
	}

	int ret = crxa_open_from_fp(fp, ZSTR_VAL(fname), ZSTR_LEN(fname), alias, alias_len, REPORT_ERRORS, NULL, 0, error);

	if (actual) {
		crex_string_release_ex(actual, 0);
	}

	return ret;
}
/* }}} */

/**
 * Validate the CRC32 of a file opened from within the crxa
 */
int crxa_postprocess_file(crxa_entry_data *idata, uint32_t crc32, char **error, int process_zip) /* {{{ */
{
	uint32_t crc = crx_crc32_bulk_init();
	int len = idata->internal_file->uncompressed_filesize, ret;
	crx_stream *fp = idata->fp;
	crxa_entry_info *entry = idata->internal_file;

	if (error) {
		*error = NULL;
	}

	if (entry->is_zip && process_zip > 0) {
		/* verify local file header */
		crxa_zip_file_header local;
		crxa_zip_data_desc desc;

		if (SUCCESS != crxa_open_archive_fp(idata->crxa)) {
			spprintf(error, 0, "crxa error: unable to open zip-based crxa archive \"%s\" to verify local file header for file \"%s\"", idata->crxa->fname, entry->filename);
			return FAILURE;
		}
		crx_stream_seek(crxa_get_entrypfp(idata->internal_file), entry->header_offset, SEEK_SET);

		if (sizeof(local) != crx_stream_read(crxa_get_entrypfp(idata->internal_file), (char *) &local, sizeof(local))) {

			spprintf(error, 0, "crxa error: internal corruption of zip-based crxa \"%s\" (cannot read local file header for file \"%s\")", idata->crxa->fname, entry->filename);
			return FAILURE;
		}

		/* check for data descriptor */
		if (((CRXA_ZIP_16(local.flags)) & 0x8) == 0x8) {
			crx_stream_seek(crxa_get_entrypfp(idata->internal_file),
					entry->header_offset + sizeof(local) +
					CRXA_ZIP_16(local.filename_len) +
					CRXA_ZIP_16(local.extra_len) +
					entry->compressed_filesize, SEEK_SET);
			if (sizeof(desc) != crx_stream_read(crxa_get_entrypfp(idata->internal_file),
							    (char *) &desc, sizeof(desc))) {
				spprintf(error, 0, "crxa error: internal corruption of zip-based crxa \"%s\" (cannot read local data descriptor for file \"%s\")", idata->crxa->fname, entry->filename);
				return FAILURE;
			}
			if (desc.signature[0] == 'P' && desc.signature[1] == 'K') {
				memcpy(&(local.crc32), &(desc.crc32), 12);
			} else {
				/* old data descriptors have no signature */
				memcpy(&(local.crc32), &desc, 12);
			}
		}
		/* verify local header */
		if (entry->filename_len != CRXA_ZIP_16(local.filename_len) || entry->crc32 != CRXA_ZIP_32(local.crc32) || entry->uncompressed_filesize != CRXA_ZIP_32(local.uncompsize) || entry->compressed_filesize != CRXA_ZIP_32(local.compsize)) {
			spprintf(error, 0, "crxa error: internal corruption of zip-based crxa \"%s\" (local header of file \"%s\" does not match central directory)", idata->crxa->fname, entry->filename);
			return FAILURE;
		}

		/* construct actual offset to file start - local extra_len can be different from central extra_len */
		entry->offset = entry->offset_abs =
			sizeof(local) + entry->header_offset + CRXA_ZIP_16(local.filename_len) + CRXA_ZIP_16(local.extra_len);

		if (idata->zero && idata->zero != entry->offset_abs) {
			idata->zero = entry->offset_abs;
		}
	}

	if (process_zip == 1) {
		return SUCCESS;
	}

	crx_stream_seek(fp, idata->zero, SEEK_SET);

	ret = crx_crc32_stream_bulk_update(&crc, fp, len);

	crx_stream_seek(fp, idata->zero, SEEK_SET);

	if (SUCCESS == ret && crx_crc32_bulk_end(crc) == crc32) {
		entry->is_crc_checked = 1;
		return SUCCESS;
	} else {
		spprintf(error, 0, "crxa error: internal corruption of crxa \"%s\" (crc32 mismatch on file \"%s\")", idata->crxa->fname, entry->filename);
		return FAILURE;
	}
}
/* }}} */

static inline void crxa_set_32(char *buffer, uint32_t var) /* {{{ */
{
#ifdef WORDS_BIGENDIAN
	*((buffer) + 3) = (unsigned char) (((var) >> 24) & 0xFF);
	*((buffer) + 2) = (unsigned char) (((var) >> 16) & 0xFF);
	*((buffer) + 1) = (unsigned char) (((var) >> 8) & 0xFF);
	*((buffer) + 0) = (unsigned char) ((var) & 0xFF);
#else
	 memcpy(buffer, &var, sizeof(var));
#endif
} /* }}} */

static int crxa_flush_clean_deleted_apply(zval *zv) /* {{{ */
{
	crxa_entry_info *entry = (crxa_entry_info *)C_PTR_P(zv);

	if (entry->fp_refcount <= 0 && entry->is_deleted) {
		return CREX_HASH_APPLY_REMOVE;
	} else {
		return CREX_HASH_APPLY_KEEP;
	}
}
/* }}} */

#include "stub.h"

crex_string *crxa_create_default_stub(const char *index_crx, const char *web_index, char **error) /* {{{ */
{
	size_t index_len, web_len;

	if (error) {
		*error = NULL;
	}

	if (!index_crx) {
		index_crx = "index.crx";
	}

	if (!web_index) {
		web_index = "index.crx";
	}

	index_len = strlen(index_crx);
	web_len = strlen(web_index);

	if (index_len > 400) {
		/* ridiculous size not allowed for index.crx startup filename */
		if (error) {
			spprintf(error, 0, "Illegal filename passed in for stub creation, was %zd characters long, and only 400 or less is allowed", index_len);
			return NULL;
		}
	}

	if (web_len > 400) {
		/* ridiculous size not allowed for index.crx startup filename */
		if (error) {
			spprintf(error, 0, "Illegal web filename passed in for stub creation, was %zd characters long, and only 400 or less is allowed", web_len);
			return NULL;
		}
	}

	return crxa_get_stub(index_crx, web_index, index_len+1, web_len+1);
}
/* }}} */

/**
 * Save crxa contents to disk
 *
 * user_stub contains either a string, or a resource pointer, if len is a negative length.
 * user_stub and len should be both 0 if the default or existing stub should be used
 */
int crxa_flush(crxa_archive_data *crxa, char *user_stub, crex_long len, int convert, char **error) /* {{{ */
{
	char halt_stub[] = "__HALT_COMPILER();";
	crex_string *newstub;
	crxa_entry_info *entry, *newentry;
	size_t halt_offset;
	int restore_alias_len, global_flags = 0, closeoldfile;
	char *pos, has_dirs = 0;
	char manifest[18], entry_buffer[24];
	crex_off_t manifest_ftell;
	crex_long offset;
	size_t wrote;
	uint32_t manifest_len, mytime, new_manifest_count;
	uint32_t newcrc32;
	crx_stream *file, *oldfile, *newfile, *stubfile;
	crx_stream_filter *filter;
	crx_serialize_data_t metadata_hash;
	smart_str main_metadata_str = {0};
	int free_user_stub, free_fp = 1, free_ufp = 1;
	int manifest_hack = 0;
	crx_stream *shared_cfp = NULL;

	if (crxa->is_persistent) {
		if (error) {
			spprintf(error, 0, "internal error: attempt to flush cached zip-based crxa \"%s\"", crxa->fname);
		}
		return EOF;
	}

	if (error) {
		*error = NULL;
	}

	if (!crex_hash_num_elements(&crxa->manifest) && !user_stub) {
		return EOF;
	}

	crex_hash_clean(&crxa->virtual_dirs);

	if (crxa->is_zip) {
		return crxa_zip_flush(crxa, user_stub, len, convert, error);
	}

	if (crxa->is_tar) {
		return crxa_tar_flush(crxa, user_stub, len, convert, error);
	}

	if (CRXA_G(readonly)) {
		return EOF;
	}

	if (crxa->fp && !crxa->is_brandnew) {
		oldfile = crxa->fp;
		closeoldfile = 0;
		crx_stream_rewind(oldfile);
	} else {
		oldfile = crx_stream_open_wrapper(crxa->fname, "rb", 0, NULL);
		closeoldfile = oldfile != NULL;
	}
	newfile = crx_stream_fopen_tmpfile();
	if (!newfile) {
		if (error) {
			spprintf(error, 0, "unable to create temporary file");
		}
		if (closeoldfile) {
			crx_stream_close(oldfile);
		}
		return EOF;
	}

	if (user_stub) {
		crex_string *suser_stub;
		if (len < 0) {
			/* resource passed in */
			if (!(crx_stream_from_zval_no_verify(stubfile, (zval *)user_stub))) {
				if (closeoldfile) {
					crx_stream_close(oldfile);
				}
				crx_stream_close(newfile);
				if (error) {
					spprintf(error, 0, "unable to access resource to copy stub to new crxa \"%s\"", crxa->fname);
				}
				return EOF;
			}
			if (len == -1) {
				len = CRX_STREAM_COPY_ALL;
			} else {
				len = -len;
			}
			user_stub = 0;

			if (!(suser_stub = crx_stream_copy_to_mem(stubfile, len, 0))) {
				if (closeoldfile) {
					crx_stream_close(oldfile);
				}
				crx_stream_close(newfile);
				if (error) {
					spprintf(error, 0, "unable to read resource to copy stub to new crxa \"%s\"", crxa->fname);
				}
				return EOF;
			}
			free_user_stub = 1;
			user_stub = ZSTR_VAL(suser_stub);
			len = ZSTR_LEN(suser_stub);
		} else {
			free_user_stub = 0;
		}
		if ((pos = crx_stristr(user_stub, halt_stub, len, sizeof(halt_stub) - 1)) == NULL) {
			if (closeoldfile) {
				crx_stream_close(oldfile);
			}
			crx_stream_close(newfile);
			if (error) {
				spprintf(error, 0, "illegal stub for crxa \"%s\" (__HALT_COMPILER(); is missing)", crxa->fname);
			}
			if (free_user_stub) {
				crex_string_free(suser_stub);
			}
			return EOF;
		}
		len = pos - user_stub + 18;
		if ((size_t)len != crx_stream_write(newfile, user_stub, len)
		||			  5 != crx_stream_write(newfile, " ?>\r\n", 5)) {
			if (closeoldfile) {
				crx_stream_close(oldfile);
			}
			crx_stream_close(newfile);
			if (error) {
				spprintf(error, 0, "unable to create stub from string in new crxa \"%s\"", crxa->fname);
			}
			if (free_user_stub) {
				crex_string_free(suser_stub);
			}
			return EOF;
		}
		crxa->halt_offset = len + 5;
		if (free_user_stub) {
			crex_string_free(suser_stub);
		}
	} else {
		size_t written;

		if (!user_stub && crxa->halt_offset && oldfile && !crxa->is_brandnew) {
			crx_stream_copy_to_stream_ex(oldfile, newfile, crxa->halt_offset, &written);
			newstub = NULL;
		} else {
			/* this is either a brand new crxa or a default stub overwrite */
			newstub = crxa_create_default_stub(NULL, NULL, NULL);
			crxa->halt_offset = ZSTR_LEN(newstub);
			written = crx_stream_write(newfile, ZSTR_VAL(newstub), crxa->halt_offset);
		}
		if (crxa->halt_offset != written) {
			if (closeoldfile) {
				crx_stream_close(oldfile);
			}
			crx_stream_close(newfile);
			if (error) {
				if (newstub) {
					spprintf(error, 0, "unable to create stub in new crxa \"%s\"", crxa->fname);
				} else {
					spprintf(error, 0, "unable to copy stub of old crxa to new crxa \"%s\"", crxa->fname);
				}
			}
			if (newstub) {
				crex_string_free(newstub);
			}
			return EOF;
		}
		if (newstub) {
			crex_string_free(newstub);
		}
	}
	manifest_ftell = crx_stream_tell(newfile);
	halt_offset = manifest_ftell;

	/* Check whether we can get rid of some of the deleted entries which are
	 * unused. However some might still be in use so even after this clean-up
	 * we need to skip entries marked is_deleted. */
	crex_hash_apply(&crxa->manifest, crxa_flush_clean_deleted_apply);

	/* compress as necessary, calculate crcs, serialize meta-data, manifest size, and file sizes */
	main_metadata_str.s = NULL;
	if (crxa->metadata_tracker.str) {
		smart_str_appendl(&main_metadata_str, ZSTR_VAL(crxa->metadata_tracker.str), ZSTR_LEN(crxa->metadata_tracker.str));
	} else if (!C_ISUNDEF(crxa->metadata_tracker.val)) {
		CRX_VAR_SERIALIZE_INIT(metadata_hash);
		crx_var_serialize(&main_metadata_str, &crxa->metadata_tracker.val, &metadata_hash);
		CRX_VAR_SERIALIZE_DESTROY(metadata_hash);
	}
	new_manifest_count = 0;
	offset = 0;
	CREX_HASH_MAP_FOREACH_PTR(&crxa->manifest, entry) {
		if (entry->cfp) {
			/* did we forget to get rid of cfp last time? */
			crx_stream_close(entry->cfp);
			entry->cfp = 0;
		}
		if (entry->is_deleted || entry->is_mounted) {
			/* remove this from the new crxa */
			continue;
		}
		if (!entry->is_modified && entry->fp_refcount) {
			/* open file pointers refer to this fp, do not free the stream */
			switch (entry->fp_type) {
				case CRXA_FP:
					free_fp = 0;
					break;
				case CRXA_UFP:
					free_ufp = 0;
				default:
					break;
			}
		}
		/* after excluding deleted files, calculate manifest size in bytes and number of entries */
		++new_manifest_count;
		crxa_add_virtual_dirs(crxa, entry->filename, entry->filename_len);

		if (entry->is_dir) {
			/* we use this to calculate API version, 1.1.1 is used for crxas with directories */
			has_dirs = 1;
		}
		if (!C_ISUNDEF(entry->metadata_tracker.val) && !entry->metadata_tracker.str) {
			CREX_ASSERT(!entry->is_persistent);
			/* Assume serialization will succeed. TODO: Set error and throw if EG(exception) != NULL */
			smart_str buf = {0};
			CRX_VAR_SERIALIZE_INIT(metadata_hash);
			crx_var_serialize(&buf, &entry->metadata_tracker.val, &metadata_hash);
			CRX_VAR_SERIALIZE_DESTROY(metadata_hash);
			entry->metadata_tracker.str = buf.s;
		}

		/* 32 bits for filename length, length of filename, manifest + metadata, and add 1 for trailing / if a directory */
		offset += 4 + entry->filename_len + sizeof(entry_buffer) + (entry->metadata_tracker.str ? ZSTR_LEN(entry->metadata_tracker.str) : 0) + (entry->is_dir ? 1 : 0);

		/* compress and rehash as necessary */
		if ((oldfile && !entry->is_modified) || entry->is_dir) {
			if (entry->fp_type == CRXA_UFP) {
				/* reset so we can copy the compressed data over */
				entry->fp_type = CRXA_FP;
			}
			continue;
		}
		if (!crxa_get_efp(entry, 0)) {
			/* re-open internal file pointer just-in-time */
			newentry = crxa_open_jit(crxa, entry, error);
			if (!newentry) {
				/* major problem re-opening, so we ignore this file and the error */
				efree(*error);
				*error = NULL;
				continue;
			}
			entry = newentry;
		}
		file = crxa_get_efp(entry, 0);
		if (-1 == crxa_seek_efp(entry, 0, SEEK_SET, 0, 1)) {
			if (closeoldfile) {
				crx_stream_close(oldfile);
			}
			crx_stream_close(newfile);
			if (error) {
				spprintf(error, 0, "unable to seek to start of file \"%s\" while creating new crxa \"%s\"", entry->filename, crxa->fname);
			}
			return EOF;
		}
		newcrc32 = crx_crc32_bulk_init();
		crx_crc32_stream_bulk_update(&newcrc32, file, entry->uncompressed_filesize);
		entry->crc32 = crx_crc32_bulk_end(newcrc32);
		entry->is_crc_checked = 1;
		if (!(entry->flags & CRXA_ENT_COMPRESSION_MASK)) {
			/* not compressed */
			entry->compressed_filesize = entry->uncompressed_filesize;
			continue;
		}
		filter = crx_stream_filter_create(crxa_compress_filter(entry, 0), NULL, 0);
		if (!filter) {
			if (closeoldfile) {
				crx_stream_close(oldfile);
			}
			crx_stream_close(newfile);
			if (entry->flags & CRXA_ENT_COMPRESSED_GZ) {
				if (error) {
					spprintf(error, 0, "unable to gzip compress file \"%s\" to new crxa \"%s\"", entry->filename, crxa->fname);
				}
			} else {
				if (error) {
					spprintf(error, 0, "unable to bzip2 compress file \"%s\" to new crxa \"%s\"", entry->filename, crxa->fname);
				}
			}
			return EOF;
		}

		/* create new file that holds the compressed versions */
		/* work around inability to specify freedom in write and strictness
		in read count */
		if (shared_cfp == NULL) {
			shared_cfp = crx_stream_fopen_tmpfile();
		}
		entry->cfp = shared_cfp;
		if (!entry->cfp) {
			if (error) {
				spprintf(error, 0, "unable to create temporary file");
			}
			if (closeoldfile) {
				crx_stream_close(oldfile);
			}
			crx_stream_close(newfile);
			goto cleanup;
		}
		/* for real crxas, header_offset is unused; we misuse it here to store the offset in the temp file */
		CREX_ASSERT(entry->header_offset == 0);
		entry->header_offset = crx_stream_tell(entry->cfp);
		crx_stream_flush(file);
		if (-1 == crxa_seek_efp(entry, 0, SEEK_SET, 0, 0)) {
			if (closeoldfile) {
				crx_stream_close(oldfile);
			}
			crx_stream_close(newfile);
			if (error) {
				spprintf(error, 0, "unable to seek to start of file \"%s\" while creating new crxa \"%s\"", entry->filename, crxa->fname);
			}
			goto cleanup;
		}
		crx_stream_filter_append((&entry->cfp->writefilters), filter);
		if (SUCCESS != crx_stream_copy_to_stream_ex(file, entry->cfp, entry->uncompressed_filesize, NULL)) {
			if (closeoldfile) {
				crx_stream_close(oldfile);
			}
			crx_stream_close(newfile);
			if (error) {
				spprintf(error, 0, "unable to copy compressed file contents of file \"%s\" while creating new crxa \"%s\"", entry->filename, crxa->fname);
			}
			goto cleanup;
		}
		crx_stream_filter_flush(filter, 1);
		crx_stream_flush(entry->cfp);
		crx_stream_filter_remove(filter, 1);
		crx_stream_seek(entry->cfp, 0, SEEK_END);
		entry->compressed_filesize = ((uint32_t) crx_stream_tell(entry->cfp)) - entry->header_offset;
		/* generate crc on compressed file */
		entry->old_flags = entry->flags;
		entry->is_modified = 1;
		global_flags |= (entry->flags & CRXA_ENT_COMPRESSION_MASK);
	} CREX_HASH_FOREACH_END();
	global_flags |= CRXA_HDR_SIGNATURE;

	/* write out manifest pre-header */
	/*  4: manifest length
	 *  4: manifest entry count
	 *  2: crxa version
	 *  4: crxa global flags
	 *  4: alias length
	 *  ?: the alias itself
	 *  4: crxa metadata length
	 *  ?: crxa metadata
	 */
	restore_alias_len = crxa->alias_len;
	if (crxa->is_temporary_alias) {
		crxa->alias_len = 0;
	}

	manifest_len = offset + crxa->alias_len + sizeof(manifest) + (main_metadata_str.s ? ZSTR_LEN(main_metadata_str.s) : 0);
	crxa_set_32(manifest, manifest_len);
	/* Hack - see bug #65028, add padding byte to the end of the manifest */
	if(manifest[0] == '\r' || manifest[0] == '\n') {
		manifest_len++;
		crxa_set_32(manifest, manifest_len);
		manifest_hack = 1;
	}
	crxa_set_32(manifest+4, new_manifest_count);
	if (has_dirs) {
		*(manifest + 8) = (unsigned char) (((CRXA_API_VERSION) >> 8) & 0xFF);
		*(manifest + 9) = (unsigned char) (((CRXA_API_VERSION) & 0xF0));
	} else {
		*(manifest + 8) = (unsigned char) (((CRXA_API_VERSION_NODIR) >> 8) & 0xFF);
		*(manifest + 9) = (unsigned char) (((CRXA_API_VERSION_NODIR) & 0xF0));
	}
	crxa_set_32(manifest+10, global_flags);
	crxa_set_32(manifest+14, crxa->alias_len);

	/* write the manifest header */
	if (sizeof(manifest) != crx_stream_write(newfile, manifest, sizeof(manifest))
	|| (size_t)crxa->alias_len != crx_stream_write(newfile, crxa->alias, crxa->alias_len)) {

		if (closeoldfile) {
			crx_stream_close(oldfile);
		}

		crx_stream_close(newfile);
		crxa->alias_len = restore_alias_len;

		if (error) {
			spprintf(error, 0, "unable to write manifest header of new crxa \"%s\"", crxa->fname);
		}

		goto cleanup;
	}

	crxa->alias_len = restore_alias_len;

	crxa_set_32(manifest, main_metadata_str.s ? ZSTR_LEN(main_metadata_str.s) : 0);
	if (4 != crx_stream_write(newfile, manifest, 4) || ((main_metadata_str.s ? ZSTR_LEN(main_metadata_str.s) : 0)
	&& ZSTR_LEN(main_metadata_str.s) != crx_stream_write(newfile, ZSTR_VAL(main_metadata_str.s), ZSTR_LEN(main_metadata_str.s)))) {
		smart_str_free(&main_metadata_str);

		if (closeoldfile) {
			crx_stream_close(oldfile);
		}

		crx_stream_close(newfile);
		crxa->alias_len = restore_alias_len;

		if (error) {
			spprintf(error, 0, "unable to write manifest meta-data of new crxa \"%s\"", crxa->fname);
		}

		goto cleanup;
	}
	smart_str_free(&main_metadata_str);

	/* re-calculate the manifest location to simplify later code */
	manifest_ftell = crx_stream_tell(newfile);

	/* now write the manifest */
	CREX_HASH_MAP_FOREACH_PTR(&crxa->manifest, entry) {
		const crex_string *metadata_str;
		if (entry->is_deleted || entry->is_mounted) {
			/* remove this from the new crxa if deleted, ignore if mounted */
			continue;
		}

		if (entry->is_dir) {
			/* add 1 for trailing slash */
			crxa_set_32(entry_buffer, entry->filename_len + 1);
		} else {
			crxa_set_32(entry_buffer, entry->filename_len);
		}

		if (4 != crx_stream_write(newfile, entry_buffer, 4)
		|| entry->filename_len != crx_stream_write(newfile, entry->filename, entry->filename_len)
		|| (entry->is_dir && 1 != crx_stream_write(newfile, "/", 1))) {
			if (closeoldfile) {
				crx_stream_close(oldfile);
			}
			crx_stream_close(newfile);
			if (error) {
				if (entry->is_dir) {
					spprintf(error, 0, "unable to write filename of directory \"%s\" to manifest of new crxa \"%s\"", entry->filename, crxa->fname);
				} else {
					spprintf(error, 0, "unable to write filename of file \"%s\" to manifest of new crxa \"%s\"", entry->filename, crxa->fname);
				}
			}
			goto cleanup;
		}

		/* set the manifest meta-data:
			4: uncompressed filesize
			4: creation timestamp
			4: compressed filesize
			4: crc32
			4: flags
			4: metadata-len
			+: metadata
		*/
		mytime = time(NULL);
		crxa_set_32(entry_buffer, entry->uncompressed_filesize);
		crxa_set_32(entry_buffer+4, mytime);
		crxa_set_32(entry_buffer+8, entry->compressed_filesize);
		crxa_set_32(entry_buffer+12, entry->crc32);
		crxa_set_32(entry_buffer+16, entry->flags);
		metadata_str = entry->metadata_tracker.str;
		crxa_set_32(entry_buffer+20, metadata_str ? ZSTR_LEN(metadata_str) : 0);

		if (sizeof(entry_buffer) != crx_stream_write(newfile, entry_buffer, sizeof(entry_buffer))
		|| (metadata_str &&
		    ZSTR_LEN(metadata_str) != crx_stream_write(newfile, ZSTR_VAL(metadata_str), ZSTR_LEN(metadata_str)))) {
			if (closeoldfile) {
				crx_stream_close(oldfile);
			}

			crx_stream_close(newfile);

			if (error) {
				spprintf(error, 0, "unable to write temporary manifest of file \"%s\" to manifest of new crxa \"%s\"", entry->filename, crxa->fname);
			}

			goto cleanup;
		}
	} CREX_HASH_FOREACH_END();
	/* Hack - see bug #65028, add padding byte to the end of the manifest */
	if(manifest_hack) {
		if(1 != crx_stream_write(newfile, manifest, 1)) {
			if (closeoldfile) {
				crx_stream_close(oldfile);
			}

			crx_stream_close(newfile);

			if (error) {
				spprintf(error, 0, "unable to write manifest padding byte");
			}

			goto cleanup;
		}
	}

	/* now copy the actual file data to the new crxa */
	offset = crx_stream_tell(newfile);
	CREX_HASH_MAP_FOREACH_PTR(&crxa->manifest, entry) {
		if (entry->is_deleted || entry->is_dir || entry->is_mounted) {
			continue;
		}

		if (entry->cfp) {
			file = entry->cfp;
			crx_stream_seek(file, entry->header_offset, SEEK_SET);
		} else {
			file = crxa_get_efp(entry, 0);
			if (-1 == crxa_seek_efp(entry, 0, SEEK_SET, 0, 0)) {
				if (closeoldfile) {
					crx_stream_close(oldfile);
				}
				crx_stream_close(newfile);
				if (error) {
					spprintf(error, 0, "unable to seek to start of file \"%s\" while creating new crxa \"%s\"", entry->filename, crxa->fname);
				}
				goto cleanup;
			}
		}

		if (!file) {
			if (closeoldfile) {
				crx_stream_close(oldfile);
			}
			crx_stream_close(newfile);
			if (error) {
				spprintf(error, 0, "unable to seek to start of file \"%s\" while creating new crxa \"%s\"", entry->filename, crxa->fname);
			}
			goto cleanup;
		}

		/* this will have changed for all files that have either changed compression or been modified */
		entry->offset = entry->offset_abs = offset;
		offset += entry->compressed_filesize;
		if (crx_stream_copy_to_stream_ex(file, newfile, entry->compressed_filesize, &wrote) == FAILURE) {
			if (closeoldfile) {
				crx_stream_close(oldfile);
			}

			crx_stream_close(newfile);

			if (error) {
				spprintf(error, 0, "unable to write contents of file \"%s\" to new crxa \"%s\"", entry->filename, crxa->fname);
			}

			goto cleanup;
		}

		entry->is_modified = 0;

		if (entry->cfp) {
			entry->cfp = NULL;
			entry->header_offset = 0;
		}

		if (entry->fp_type == CRXA_MOD) {
			/* this fp is in use by a crxa_entry_data returned by crxa_get_entry_data, it will be closed when the crxa_entry_data is crxa_entry_delref'ed */
			if (entry->fp_refcount == 0 && entry->fp != crxa->fp && entry->fp != crxa->ufp) {
				crx_stream_close(entry->fp);
			}

			entry->fp = NULL;
			entry->fp_type = CRXA_FP;
		} else if (entry->fp_type == CRXA_UFP) {
			entry->fp_type = CRXA_FP;
		}
	} CREX_HASH_FOREACH_END();

	if (shared_cfp != NULL) {
		crx_stream_close(shared_cfp);
		shared_cfp = NULL;
	}

	/* append signature */
	if (global_flags & CRXA_HDR_SIGNATURE) {
		char sig_buf[4];

		crx_stream_rewind(newfile);

		if (crxa->signature) {
			efree(crxa->signature);
			crxa->signature = NULL;
		}

		switch(crxa->sig_flags) {
			default: {
				char *digest = NULL;
				size_t digest_len;

				if (FAILURE == crxa_create_signature(crxa, newfile, &digest, &digest_len, error)) {
					if (error) {
						char *save = *error;
						spprintf(error, 0, "crxa error: unable to write signature: %s", save);
						efree(save);
					}
					if (digest) {
						efree(digest);
					}
					if (closeoldfile) {
						crx_stream_close(oldfile);
					}
					crx_stream_close(newfile);
					return EOF;
				}

				crx_stream_write(newfile, digest, digest_len);
				efree(digest);
				if (crxa->sig_flags == CRXA_SIG_OPENSSL ||
					crxa->sig_flags == CRXA_SIG_OPENSSL_SHA256 ||
					crxa->sig_flags == CRXA_SIG_OPENSSL_SHA512) {
					crxa_set_32(sig_buf, digest_len);
					crx_stream_write(newfile, sig_buf, 4);
				}
				break;
			}
		}
		crxa_set_32(sig_buf, crxa->sig_flags);
		crx_stream_write(newfile, sig_buf, 4);
		crx_stream_write(newfile, "GBMB", 4);
	}

	/* finally, close the temp file, rename the original crxa,
	   move the temp to the old crxa, unlink the old crxa, and reload it into memory
	*/
	if (crxa->fp && free_fp) {
		crx_stream_close(crxa->fp);
	}

	if (crxa->ufp) {
		if (free_ufp) {
			crx_stream_close(crxa->ufp);
		}
		crxa->ufp = NULL;
	}

	if (closeoldfile) {
		crx_stream_close(oldfile);
	}

	crxa->internal_file_start = halt_offset + manifest_len + 4;
	crxa->halt_offset = halt_offset;
	crxa->is_brandnew = 0;

	crx_stream_rewind(newfile);

	if (crxa->donotflush) {
		/* deferred flush */
		crxa->fp = newfile;
	} else {
		crxa->fp = crx_stream_open_wrapper(crxa->fname, "w+b", IGNORE_URL|STREAM_MUST_SEEK|REPORT_ERRORS, NULL);
		if (!crxa->fp) {
			crxa->fp = newfile;
			if (error) {
				spprintf(error, 4096, "unable to open new crxa \"%s\" for writing", crxa->fname);
			}
			return EOF;
		}

		if (crxa->flags & CRXA_FILE_COMPRESSED_GZ) {
			/* to properly compress, we have to tell zlib to add a zlib header */
			zval filterparams;

			array_init(&filterparams);
			add_assoc_long(&filterparams, "window", MAX_WBITS+16);
			filter = crx_stream_filter_create("zlib.deflate", &filterparams, crx_stream_is_persistent(crxa->fp));
			crex_array_destroy(C_ARR(filterparams));

			if (!filter) {
				if (error) {
					spprintf(error, 4096, "unable to compress all contents of crxa \"%s\" using zlib, CRX versions older than 5.2.6 have a buggy zlib", crxa->fname);
				}
				return EOF;
			}

			crx_stream_filter_append(&crxa->fp->writefilters, filter);
			crx_stream_copy_to_stream_ex(newfile, crxa->fp, CRX_STREAM_COPY_ALL, NULL);
			crx_stream_filter_flush(filter, 1);
			crx_stream_filter_remove(filter, 1);
			crx_stream_close(crxa->fp);
			/* use the temp stream as our base */
			crxa->fp = newfile;
		} else if (crxa->flags & CRXA_FILE_COMPRESSED_BZ2) {
			filter = crx_stream_filter_create("bzip2.compress", NULL, crx_stream_is_persistent(crxa->fp));
			crx_stream_filter_append(&crxa->fp->writefilters, filter);
			crx_stream_copy_to_stream_ex(newfile, crxa->fp, CRX_STREAM_COPY_ALL, NULL);
			crx_stream_filter_flush(filter, 1);
			crx_stream_filter_remove(filter, 1);
			crx_stream_close(crxa->fp);
			/* use the temp stream as our base */
			crxa->fp = newfile;
		} else {
			crx_stream_copy_to_stream_ex(newfile, crxa->fp, CRX_STREAM_COPY_ALL, NULL);
			/* we could also reopen the file in "rb" mode but there is no need for that */
			crx_stream_close(newfile);
		}
	}

	if (-1 == crx_stream_seek(crxa->fp, crxa->halt_offset, SEEK_SET)) {
		if (error) {
			spprintf(error, 0, "unable to seek to __HALT_COMPILER(); in new crxa \"%s\"", crxa->fname);
		}
		return EOF;
	}

	return EOF;

cleanup:
	if (shared_cfp != NULL) {
		crx_stream_close(shared_cfp);
	}
	CREX_HASH_MAP_FOREACH_PTR(&crxa->manifest, entry) {
		if (entry->cfp) {
			entry->cfp = NULL;
			entry->header_offset = 0;
		}
	} CREX_HASH_FOREACH_END();

	return EOF;
}
/* }}} */

#ifdef COMPILE_DL_CRXA
#ifdef ZTS
CREX_TSRMLS_CACHE_DEFINE()
#endif
CREX_GET_MODULE(crxa)
#endif

static ssize_t crxa_crex_stream_reader(void *handle, char *buf, size_t len) /* {{{ */
{
	return crx_stream_read(crxa_get_crxafp((crxa_archive_data*)handle), buf, len);
}
/* }}} */

static size_t crxa_crex_stream_fsizer(void *handle) /* {{{ */
{
	return ((crxa_archive_data*)handle)->halt_offset + 32;
} /* }}} */

crex_op_array *(*crxa_orig_compile_file)(crex_file_handle *file_handle, int type);

static crex_string *crxa_resolve_path(crex_string *filename)
{
	crex_string *ret = crxa_find_in_include_path(filename, NULL);
	if (!ret) {
		ret = crxa_save_resolve_path(filename);
	}
	return ret;
}

static crex_op_array *crxa_compile_file(crex_file_handle *file_handle, int type) /* {{{ */
{
	crex_op_array *res;
	crex_string *name = NULL;
	int failed;
	crxa_archive_data *crxa;

	if (!file_handle || !file_handle->filename) {
		return crxa_orig_compile_file(file_handle, type);
	}
	if (strstr(ZSTR_VAL(file_handle->filename), ".crxa") && !strstr(ZSTR_VAL(file_handle->filename), "://")) {
		if (SUCCESS == crxa_open_from_filename(ZSTR_VAL(file_handle->filename), ZSTR_LEN(file_handle->filename), NULL, 0, 0, &crxa, NULL)) {
			if (crxa->is_zip || crxa->is_tar) {
				crex_file_handle f;

				/* zip or tar-based crxa */
				name = crex_strpprintf(4096, "crxa://%s/%s", ZSTR_VAL(file_handle->filename), ".crxa/stub.crx");
				crex_stream_init_filename_ex(&f, name);
				if (SUCCESS == crex_stream_open_function(&f)) {
					crex_string_release(f.filename);
					f.filename = file_handle->filename;
					if (f.opened_path) {
						crex_string_release(f.opened_path);
					}
					f.opened_path = file_handle->opened_path;

					switch (file_handle->type) {
						case CREX_HANDLE_STREAM:
							if (file_handle->handle.stream.closer && file_handle->handle.stream.handle) {
								file_handle->handle.stream.closer(file_handle->handle.stream.handle);
							}
							file_handle->handle.stream.handle = NULL;
							break;
						default:
							break;
					}
					*file_handle = f;
				}
			} else if (crxa->flags & CRXA_FILE_COMPRESSION_MASK) {
				/* compressed crxa */
				file_handle->type = CREX_HANDLE_STREAM;
				/* we do our own reading directly from the crxa, don't change the next line */
				file_handle->handle.stream.handle  = crxa;
				file_handle->handle.stream.reader  = crxa_crex_stream_reader;
				file_handle->handle.stream.closer  = NULL;
				file_handle->handle.stream.fsizer  = crxa_crex_stream_fsizer;
				file_handle->handle.stream.isatty  = 0;
				crxa->is_persistent ?
					crx_stream_rewind(CRXA_G(cached_fp)[crxa->crxa_pos].fp) :
					crx_stream_rewind(crxa->fp);
			}
		}
	}

	crex_try {
		failed = 0;
		CG(crex_lineno) = 0;
		res = crxa_orig_compile_file(file_handle, type);
	} crex_catch {
		failed = 1;
		res = NULL;
	} crex_end_try();

	if (name) {
		crex_string_release(name);
	}

	if (failed) {
		crex_bailout();
	}

	return res;
}
/* }}} */

typedef crex_op_array* (crex_compile_t)(crex_file_handle*, int);
typedef crex_compile_t* (compile_hook)(crex_compile_t *ptr);

static void mime_type_dtor(zval *zv)
{
	free(C_PTR_P(zv));
}

CRX_GINIT_FUNCTION(crxa) /* {{{ */
{
#if defined(COMPILE_DL_CRXA) && defined(ZTS)
	CREX_TSRMLS_CACHE_UPDATE();
#endif
	crxa_mime_type mime;

	memset(crxa_globals, 0, sizeof(crex_crxa_globals));
	HT_INVALIDATE(&crxa_globals->crxa_persist_map);
	HT_INVALIDATE(&crxa_globals->crxa_fname_map);
	HT_INVALIDATE(&crxa_globals->crxa_alias_map);
	crxa_globals->readonly = 1;

	crex_hash_init(&crxa_globals->mime_types, 0, NULL, mime_type_dtor, 1);

#define CRXA_SET_MIME(mimetype, ret, fileext) \
		mime.mime = mimetype; \
		mime.len = sizeof((mimetype))+1; \
		mime.type = ret; \
		crex_hash_str_add_mem(&crxa_globals->mime_types, fileext, sizeof(fileext)-1, (void *)&mime, sizeof(crxa_mime_type)); \

	CRXA_SET_MIME("text/html", CRXA_MIME_CRXS, "crxs")
	CRXA_SET_MIME("text/plain", CRXA_MIME_OTHER, "c")
	CRXA_SET_MIME("text/plain", CRXA_MIME_OTHER, "cc")
	CRXA_SET_MIME("text/plain", CRXA_MIME_OTHER, "cpp")
	CRXA_SET_MIME("text/plain", CRXA_MIME_OTHER, "c++")
	CRXA_SET_MIME("text/plain", CRXA_MIME_OTHER, "dtd")
	CRXA_SET_MIME("text/plain", CRXA_MIME_OTHER, "h")
	CRXA_SET_MIME("text/plain", CRXA_MIME_OTHER, "log")
	CRXA_SET_MIME("text/plain", CRXA_MIME_OTHER, "rng")
	CRXA_SET_MIME("text/plain", CRXA_MIME_OTHER, "txt")
	CRXA_SET_MIME("text/plain", CRXA_MIME_OTHER, "xsd")
	CRXA_SET_MIME("", CRXA_MIME_CRX, "crx")
	CRXA_SET_MIME("", CRXA_MIME_CRX, "inc")
	CRXA_SET_MIME("video/avi", CRXA_MIME_OTHER, "avi")
	CRXA_SET_MIME("image/bmp", CRXA_MIME_OTHER, "bmp")
	CRXA_SET_MIME("text/css", CRXA_MIME_OTHER, "css")
	CRXA_SET_MIME("image/gif", CRXA_MIME_OTHER, "gif")
	CRXA_SET_MIME("text/html", CRXA_MIME_OTHER, "htm")
	CRXA_SET_MIME("text/html", CRXA_MIME_OTHER, "html")
	CRXA_SET_MIME("text/html", CRXA_MIME_OTHER, "htmls")
	CRXA_SET_MIME("image/x-ico", CRXA_MIME_OTHER, "ico")
	CRXA_SET_MIME("image/jpeg", CRXA_MIME_OTHER, "jpe")
	CRXA_SET_MIME("image/jpeg", CRXA_MIME_OTHER, "jpg")
	CRXA_SET_MIME("image/jpeg", CRXA_MIME_OTHER, "jpeg")
	CRXA_SET_MIME("application/x-javascript", CRXA_MIME_OTHER, "js")
	CRXA_SET_MIME("audio/midi", CRXA_MIME_OTHER, "midi")
	CRXA_SET_MIME("audio/midi", CRXA_MIME_OTHER, "mid")
	CRXA_SET_MIME("audio/mod", CRXA_MIME_OTHER, "mod")
	CRXA_SET_MIME("movie/quicktime", CRXA_MIME_OTHER, "mov")
	CRXA_SET_MIME("audio/mp3", CRXA_MIME_OTHER, "mp3")
	CRXA_SET_MIME("video/mpeg", CRXA_MIME_OTHER, "mpg")
	CRXA_SET_MIME("video/mpeg", CRXA_MIME_OTHER, "mpeg")
	CRXA_SET_MIME("application/pdf", CRXA_MIME_OTHER, "pdf")
	CRXA_SET_MIME("image/png", CRXA_MIME_OTHER, "png")
	CRXA_SET_MIME("application/shockwave-flash", CRXA_MIME_OTHER, "swf")
	CRXA_SET_MIME("image/tiff", CRXA_MIME_OTHER, "tif")
	CRXA_SET_MIME("image/tiff", CRXA_MIME_OTHER, "tiff")
	CRXA_SET_MIME("audio/wav", CRXA_MIME_OTHER, "wav")
	CRXA_SET_MIME("image/xbm", CRXA_MIME_OTHER, "xbm")
	CRXA_SET_MIME("text/xml", CRXA_MIME_OTHER, "xml")

	crxa_restore_orig_functions();
}
/* }}} */

CRX_GSHUTDOWN_FUNCTION(crxa) /* {{{ */
{
	crex_hash_destroy(&crxa_globals->mime_types);
}
/* }}} */

CRX_MINIT_FUNCTION(crxa) /* {{{ */
{
	REGISTER_INI_ENTRIES();

	crxa_orig_compile_file = crex_compile_file;
	crex_compile_file = crxa_compile_file;

	crxa_save_resolve_path = crex_resolve_path;
	crex_resolve_path = crxa_resolve_path;

	crxa_object_init();

	crxa_intercept_functions_init();
	crxa_save_orig_functions();

	return crx_register_url_stream_wrapper("crxa", &crx_stream_crxa_wrapper);
}
/* }}} */

CRX_MSHUTDOWN_FUNCTION(crxa) /* {{{ */
{
	crx_unregister_url_stream_wrapper("crxa");

	crxa_intercept_functions_shutdown();

	if (crex_compile_file == crxa_compile_file) {
		crex_compile_file = crxa_orig_compile_file;
	}

	if (CRXA_G(manifest_cached)) {
		crex_hash_destroy(&(cached_crxas));
		crex_hash_destroy(&(cached_alias));
	}

	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

void crxa_request_initialize(void) /* {{{ */
{
	if (!CRXA_G(request_init))
	{
		CRXA_G(last_crxa) = NULL;
		CRXA_G(last_crxa_name) = CRXA_G(last_alias) = NULL;
		CRXA_G(has_bz2) = crex_hash_str_exists(&module_registry, "bz2", sizeof("bz2")-1);
		CRXA_G(has_zlib) = crex_hash_str_exists(&module_registry, "zlib", sizeof("zlib")-1);
		CRXA_G(request_init) = 1;
		CRXA_G(request_ends) = 0;
		CRXA_G(request_done) = 0;
		crex_hash_init(&(CRXA_G(crxa_fname_map)), 5, crex_get_hash_value, destroy_crxa_data,  0);
		crex_hash_init(&(CRXA_G(crxa_persist_map)), 5, crex_get_hash_value, NULL,  0);
		crex_hash_init(&(CRXA_G(crxa_alias_map)), 5, crex_get_hash_value, NULL, 0);

		if (CRXA_G(manifest_cached)) {
			crxa_archive_data *pcrxa;
			crxa_entry_fp *stuff = (crxa_entry_fp *) ecalloc(crex_hash_num_elements(&cached_crxas), sizeof(crxa_entry_fp));

			CREX_HASH_MAP_FOREACH_PTR(&cached_crxas, pcrxa) {
				stuff[pcrxa->crxa_pos].manifest = (crxa_entry_fp_info *) ecalloc( crex_hash_num_elements(&(pcrxa->manifest)), sizeof(crxa_entry_fp_info));
			} CREX_HASH_FOREACH_END();

			CRXA_G(cached_fp) = stuff;
		}

		CRXA_G(crxa_SERVER_mung_list) = 0;
		CRXA_G(cwd) = NULL;
		CRXA_G(cwd_len) = 0;
		CRXA_G(cwd_init) = 0;
	}
}
/* }}} */

CRX_RSHUTDOWN_FUNCTION(crxa) /* {{{ */
{
	uint32_t i;

	CRXA_G(request_ends) = 1;

	if (CRXA_G(request_init))
	{
		crxa_release_functions();
		crex_hash_destroy(&(CRXA_G(crxa_alias_map)));
		HT_INVALIDATE(&CRXA_G(crxa_alias_map));
		crex_hash_destroy(&(CRXA_G(crxa_fname_map)));
		HT_INVALIDATE(&CRXA_G(crxa_fname_map));
		crex_hash_destroy(&(CRXA_G(crxa_persist_map)));
		HT_INVALIDATE(&CRXA_G(crxa_persist_map));
		CRXA_G(crxa_SERVER_mung_list) = 0;

		if (CRXA_G(cached_fp)) {
			for (i = 0; i < crex_hash_num_elements(&cached_crxas); ++i) {
				if (CRXA_G(cached_fp)[i].fp) {
					crx_stream_close(CRXA_G(cached_fp)[i].fp);
				}
				if (CRXA_G(cached_fp)[i].ufp) {
					crx_stream_close(CRXA_G(cached_fp)[i].ufp);
				}
				efree(CRXA_G(cached_fp)[i].manifest);
			}
			efree(CRXA_G(cached_fp));
			CRXA_G(cached_fp) = 0;
		}

		CRXA_G(request_init) = 0;

		if (CRXA_G(cwd)) {
			efree(CRXA_G(cwd));
		}

		CRXA_G(cwd) = NULL;
		CRXA_G(cwd_len) = 0;
		CRXA_G(cwd_init) = 0;
	}

	CRXA_G(request_done) = 1;
	return SUCCESS;
}
/* }}} */

CRX_MINFO_FUNCTION(crxa) /* {{{ */
{
	crxa_request_initialize();
	crx_info_print_table_start();
	crx_info_print_table_row(2, "Crxa: CRX Archive support", "enabled");
	crx_info_print_table_row(2, "Crxa API version", CRX_CRXA_API_VERSION);
	crx_info_print_table_row(2, "Crxa-based crxa archives", "enabled");
	crx_info_print_table_row(2, "Tar-based crxa archives", "enabled");
	crx_info_print_table_row(2, "ZIP-based crxa archives", "enabled");

	if (CRXA_G(has_zlib)) {
		crx_info_print_table_row(2, "gzip compression", "enabled");
	} else {
		crx_info_print_table_row(2, "gzip compression", "disabled (install ext/zlib)");
	}

	if (CRXA_G(has_bz2)) {
		crx_info_print_table_row(2, "bzip2 compression", "enabled");
	} else {
		crx_info_print_table_row(2, "bzip2 compression", "disabled (install ext/bz2)");
	}
#ifdef CRXA_HAVE_OPENSSL
	crx_info_print_table_row(2, "Native OpenSSL support", "enabled");
#else
	if (crex_hash_str_exists(&module_registry, "openssl", sizeof("openssl")-1)) {
		crx_info_print_table_row(2, "OpenSSL support", "enabled");
	} else {
		crx_info_print_table_row(2, "OpenSSL support", "disabled (install ext/openssl)");
	}
#endif
	crx_info_print_table_end();

	crx_info_print_box_start(0);
	PUTS("Crxa based on pear/CRX_Archive, original concept by Davey Shafik.");
	PUTS(!sapi_module.crxinfo_as_text?"<br />":"\n");
	PUTS("Crxa fully realized by Gregory Beaver and Marcus Boerger.");
	PUTS(!sapi_module.crxinfo_as_text?"<br />":"\n");
	PUTS("Portions of tar implementation Copyright (c) 2003-2009 Tim Kientzle.");
	crx_info_print_box_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ crxa_module_entry */
static const crex_module_dep crxa_deps[] = {
	CREX_MOD_OPTIONAL("apc")
	CREX_MOD_OPTIONAL("bz2")
	CREX_MOD_OPTIONAL("openssl")
	CREX_MOD_OPTIONAL("zlib")
	CREX_MOD_OPTIONAL("standard")
	CREX_MOD_REQUIRED("hash")
	CREX_MOD_REQUIRED("spl")
	CREX_MOD_END
};

crex_module_entry crxa_module_entry = {
	STANDARD_MODULE_HEADER_EX, NULL,
	crxa_deps,
	"Crxa",
	NULL,
	CRX_MINIT(crxa),
	CRX_MSHUTDOWN(crxa),
	NULL,
	CRX_RSHUTDOWN(crxa),
	CRX_MINFO(crxa),
	CRX_CRXA_VERSION,
	CRX_MODULE_GLOBALS(crxa),   /* globals descriptor */
	CRX_GINIT(crxa),            /* globals ctor */
	CRX_GSHUTDOWN(crxa),        /* globals dtor */
	NULL,                       /* post deactivate */
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */
