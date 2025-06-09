/*
  +----------------------------------------------------------------------+
  | crxa crx single-file executable CRX extension                        |
  | utility functions                                                    |
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
#include "ext/hash/crx_hash_sha.h"

#ifdef CRXA_HAVE_OPENSSL
/* OpenSSL includes */
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/crypto.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/pkcs12.h>
#else
static int crxa_call_openssl_signverify(int is_sign, crx_stream *fp, crex_off_t end, char *key, size_t key_len, char **signature, size_t *signature_len, crx_uint32 sig_type);
#endif

/* for links to relative location, prepend cwd of the entry */
static char *crxa_get_link_location(crxa_entry_info *entry) /* {{{ */
{
	char *p, *ret = NULL;
	if (!entry->link) {
		return NULL;
	}
	if (entry->link[0] == '/') {
		return estrdup(entry->link + 1);
	}
	p = strrchr(entry->filename, '/');
	if (p) {
		*p = '\0';
		spprintf(&ret, 0, "%s/%s", entry->filename, entry->link);
		return ret;
	}
	return entry->link;
}
/* }}} */

crxa_entry_info *crxa_get_link_source(crxa_entry_info *entry) /* {{{ */
{
	crxa_entry_info *link_entry;
	char *link;

	if (!entry->link) {
		return entry;
	}

	link = crxa_get_link_location(entry);
	if (NULL != (link_entry = crex_hash_str_find_ptr(&(entry->crxa->manifest), entry->link, strlen(entry->link))) ||
		NULL != (link_entry = crex_hash_str_find_ptr(&(entry->crxa->manifest), link, strlen(link)))) {
		if (link != entry->link) {
			efree(link);
		}
		return crxa_get_link_source(link_entry);
	} else {
		if (link != entry->link) {
			efree(link);
		}
		return NULL;
	}
}
/* }}} */

/* retrieve a crxa_entry_info's current file pointer for reading contents */
crx_stream *crxa_get_efp(crxa_entry_info *entry, int follow_links) /* {{{ */
{
	if (follow_links && entry->link) {
		crxa_entry_info *link_entry = crxa_get_link_source(entry);

		if (link_entry && link_entry != entry) {
			return crxa_get_efp(link_entry, 1);
		}
	}

	if (crxa_get_fp_type(entry) == CRXA_FP) {
		if (!crxa_get_entrypfp(entry)) {
			/* re-open just in time for cases where our refcount reached 0 on the crxa archive */
			crxa_open_archive_fp(entry->crxa);
		}
		return crxa_get_entrypfp(entry);
	} else if (crxa_get_fp_type(entry) == CRXA_UFP) {
		return crxa_get_entrypufp(entry);
	} else if (entry->fp_type == CRXA_MOD) {
		return entry->fp;
	} else {
		/* temporary manifest entry */
		if (!entry->fp) {
			entry->fp = crx_stream_open_wrapper(entry->tmp, "rb", STREAM_MUST_SEEK|0, NULL);
		}
		return entry->fp;
	}
}
/* }}} */

int crxa_seek_efp(crxa_entry_info *entry, crex_off_t offset, int whence, crex_off_t position, int follow_links) /* {{{ */
{
	crx_stream *fp = crxa_get_efp(entry, follow_links);
	crex_off_t temp, eoffset;

	if (!fp) {
		return -1;
	}

	if (follow_links) {
		crxa_entry_info *t;
		t = crxa_get_link_source(entry);
		if (t) {
			entry = t;
		}
	}

	if (entry->is_dir) {
		return 0;
	}

	eoffset = crxa_get_fp_offset(entry);

	switch (whence) {
		case SEEK_END:
			temp = eoffset + entry->uncompressed_filesize + offset;
			break;
		case SEEK_CUR:
			temp = eoffset + position + offset;
			break;
		case SEEK_SET:
			temp = eoffset + offset;
			break;
		default:
			temp = 0;
	}

	if (temp > eoffset + (crex_off_t) entry->uncompressed_filesize) {
		return -1;
	}

	if (temp < eoffset) {
		return -1;
	}

	return crx_stream_seek(fp, temp, SEEK_SET);
}
/* }}} */

/* mount an absolute path or uri to a path internal to the crxa archive */
int crxa_mount_entry(crxa_archive_data *crxa, char *filename, size_t filename_len, char *path, size_t path_len) /* {{{ */
{
	crxa_entry_info entry = {0};
	crx_stream_statbuf ssb;
	int is_crxa;
	const char *err;

	if (crxa_path_check(&path, &path_len, &err) > pcr_is_ok) {
		return FAILURE;
	}

	if (path_len >= sizeof(".crxa")-1 && !memcmp(path, ".crxa", sizeof(".crxa")-1)) {
		/* no creating magic crxa files by mounting them */
		return FAILURE;
	}

	is_crxa = (filename_len > 7 && !memcmp(filename, "crxa://", 7));

	entry.crxa = crxa;
	entry.filename = estrndup(path, path_len);
#ifdef CRX_WIN32
	crxa_unixify_path_separators(entry.filename, path_len);
#endif
	entry.filename_len = path_len;
	if (is_crxa) {
		entry.tmp = estrndup(filename, filename_len);
	} else {
		entry.tmp = expand_filepath(filename, NULL);
		if (!entry.tmp) {
			entry.tmp = estrndup(filename, filename_len);
		}
	}
	filename = entry.tmp;

	/* only check openbasedir for files, not for crxa streams */
	if (!is_crxa && crx_check_open_basedir(filename)) {
		efree(entry.tmp);
		efree(entry.filename);
		return FAILURE;
	}

	entry.is_mounted = 1;
	entry.is_crc_checked = 1;
	entry.fp_type = CRXA_TMP;

	if (SUCCESS != crx_stream_stat_path(filename, &ssb)) {
		efree(entry.tmp);
		efree(entry.filename);
		return FAILURE;
	}

	if (ssb.sb.st_mode & S_IFDIR) {
		entry.is_dir = 1;
		if (NULL == crex_hash_str_add_ptr(&crxa->mounted_dirs, entry.filename, path_len, entry.filename)) {
			/* directory already mounted */
			efree(entry.tmp);
			efree(entry.filename);
			return FAILURE;
		}
	} else {
		entry.is_dir = 0;
		entry.uncompressed_filesize = entry.compressed_filesize = ssb.sb.st_size;
	}

	entry.flags = ssb.sb.st_mode;

	if (NULL != crex_hash_str_add_mem(&crxa->manifest, entry.filename, path_len, (void*)&entry, sizeof(crxa_entry_info))) {
		return SUCCESS;
	}

	efree(entry.tmp);
	efree(entry.filename);
	return FAILURE;
}
/* }}} */

crex_string *crxa_find_in_include_path(crex_string *filename, crxa_archive_data **pcrxa) /* {{{ */
{
	crex_string *ret;
	char *path, *arch, *entry, *test;
	size_t arch_len, entry_len;
	crxa_archive_data *crxa;

	if (pcrxa) {
		*pcrxa = NULL;
	} else {
		pcrxa = &crxa;
	}

	if (!crex_is_executing() || !CRXA_G(cwd)) {
		return NULL;
	}

	crex_string *fname = crex_get_executed_filename_ex();
	if (!fname) {
		return NULL;
	}

	bool is_file_a_crxa_wrapper = crex_string_starts_with_literal_ci(fname, "crxa://");
	size_t length_crxa_protocol = strlen("crxa://");

	if (
		CRXA_G(last_crxa)
		&& is_file_a_crxa_wrapper
		&& ZSTR_LEN(fname) - length_crxa_protocol >= CRXA_G(last_crxa_name_len)
		&& !memcmp(ZSTR_VAL(fname) + length_crxa_protocol, CRXA_G(last_crxa_name), CRXA_G(last_crxa_name_len))
	) {
		arch = estrndup(CRXA_G(last_crxa_name), CRXA_G(last_crxa_name_len));
		arch_len = CRXA_G(last_crxa_name_len);
		crxa = CRXA_G(last_crxa);
		goto splitted;
	}

	if (!is_file_a_crxa_wrapper || SUCCESS != crxa_split_fname(ZSTR_VAL(fname), ZSTR_LEN(fname), &arch, &arch_len, &entry, &entry_len, 1, 0)) {
		return NULL;
	}

	efree(entry);

	if (*ZSTR_VAL(filename) == '.') {
		size_t try_len;

		if (FAILURE == crxa_get_archive(&crxa, arch, arch_len, NULL, 0, NULL)) {
			efree(arch);
			return NULL;
		}
splitted:
		if (pcrxa) {
			*pcrxa = crxa;
		}

		try_len = ZSTR_LEN(filename);
		test = crxa_fix_filepath(estrndup(ZSTR_VAL(filename), ZSTR_LEN(filename)), &try_len, 1);

		if (*test == '/') {
			if (crex_hash_str_exists(&(crxa->manifest), test + 1, try_len - 1)) {
				ret = strpprintf(0, "crxa://%s%s", arch, test);
				efree(arch);
				efree(test);
				return ret;
			}
		} else {
			if (crex_hash_str_exists(&(crxa->manifest), test, try_len)) {
				ret = strpprintf(0, "crxa://%s/%s", arch, test);
				efree(arch);
				efree(test);
				return ret;
			}
		}
		efree(test);
	}

	spprintf(&path, MAXPATHLEN + 1 + strlen(PG(include_path)), "crxa://%s/%s%c%s", arch, CRXA_G(cwd), DEFAULT_DIR_SEPARATOR, PG(include_path));
	efree(arch);
	ret = crx_resolve_path(ZSTR_VAL(filename), ZSTR_LEN(filename), path);
	efree(path);

	if (ret && crex_string_starts_with_literal_ci(ret, "crxa://")) {
		/* found crxa:// */
		if (SUCCESS != crxa_split_fname(ZSTR_VAL(ret), ZSTR_LEN(ret), &arch, &arch_len, &entry, &entry_len, 1, 0)) {
			return ret;
		}

		*pcrxa = crex_hash_str_find_ptr(&(CRXA_G(crxa_fname_map)), arch, arch_len);

		if (!*pcrxa && CRXA_G(manifest_cached)) {
			*pcrxa = crex_hash_str_find_ptr(&cached_crxas, arch, arch_len);
		}

		efree(arch);
		efree(entry);
	}

	return ret;
}
/* }}} */

/**
 * Retrieve a copy of the file information on a single file within a crxa, or null.
 * This also transfers the open file pointer, if any, to the entry.
 *
 * If the file does not already exist, this will fail.  Pre-existing files can be
 * appended, truncated, or read.  For read, if the entry is marked unmodified, it is
 * assumed that the file pointer, if present, is opened for reading
 */
int crxa_get_entry_data(crxa_entry_data **ret, char *fname, size_t fname_len, char *path, size_t path_len, const char *mode, char allow_dir, char **error, int security) /* {{{ */
{
	crxa_archive_data *crxa;
	crxa_entry_info *entry;
	int for_write  = mode[0] != 'r' || mode[1] == '+';
	int for_append = mode[0] == 'a';
	int for_create = mode[0] != 'r';
	int for_trunc  = mode[0] == 'w';

	if (!ret) {
		return FAILURE;
	}

	*ret = NULL;

	if (error) {
		*error = NULL;
	}

	if (FAILURE == crxa_get_archive(&crxa, fname, fname_len, NULL, 0, error)) {
		return FAILURE;
	}

	if (for_write && CRXA_G(readonly) && !crxa->is_data) {
		if (error) {
			spprintf(error, 4096, "crxa error: file \"%s\" in crxa \"%s\" cannot be opened for writing, disabled by ini setting", path, fname);
		}
		return FAILURE;
	}

	if (!path_len) {
		if (error) {
			spprintf(error, 4096, "crxa error: file \"\" in crxa \"%s\" cannot be empty", fname);
		}
		return FAILURE;
	}
really_get_entry:
	if (allow_dir) {
		if ((entry = crxa_get_entry_info_dir(crxa, path, path_len, allow_dir, for_create && !CRXA_G(readonly) && !crxa->is_data ? NULL : error, security)) == NULL) {
			if (for_create && (!CRXA_G(readonly) || crxa->is_data)) {
				return SUCCESS;
			}
			return FAILURE;
		}
	} else {
		if ((entry = crxa_get_entry_info(crxa, path, path_len, for_create && !CRXA_G(readonly) && !crxa->is_data ? NULL : error, security)) == NULL) {
			if (for_create && (!CRXA_G(readonly) || crxa->is_data)) {
				return SUCCESS;
			}
			return FAILURE;
		}
	}

	if (for_write && crxa->is_persistent) {
		if (FAILURE == crxa_copy_on_write(&crxa)) {
			if (error) {
				spprintf(error, 4096, "crxa error: file \"%s\" in crxa \"%s\" cannot be opened for writing, could not make cached crxa writeable", path, fname);
			}
			return FAILURE;
		} else {
			goto really_get_entry;
		}
	}

	if (entry->is_modified && !for_write) {
		if (error) {
			spprintf(error, 4096, "crxa error: file \"%s\" in crxa \"%s\" cannot be opened for reading, writable file pointers are open", path, fname);
		}
		return FAILURE;
	}

	if (entry->fp_refcount && for_write) {
		if (error) {
			spprintf(error, 4096, "crxa error: file \"%s\" in crxa \"%s\" cannot be opened for writing, readable file pointers are open", path, fname);
		}
		return FAILURE;
	}

	if (entry->is_deleted) {
		if (!for_create) {
			return FAILURE;
		}
		entry->is_deleted = 0;
	}

	if (entry->is_dir) {
		*ret = (crxa_entry_data *) emalloc(sizeof(crxa_entry_data));
		(*ret)->position = 0;
		(*ret)->fp = NULL;
		(*ret)->crxa = crxa;
		(*ret)->for_write = for_write;
		(*ret)->internal_file = entry;
		(*ret)->is_zip = entry->is_zip;
		(*ret)->is_tar = entry->is_tar;

		if (!crxa->is_persistent) {
			++(entry->crxa->refcount);
			++(entry->fp_refcount);
		}

		return SUCCESS;
	}

	if (entry->fp_type == CRXA_MOD) {
		if (for_trunc) {
			if (FAILURE == crxa_create_writeable_entry(crxa, entry, error)) {
				return FAILURE;
			}
		} else if (for_append) {
			crxa_seek_efp(entry, 0, SEEK_END, 0, 0);
		}
	} else {
		if (for_write) {
			if (entry->link) {
				efree(entry->link);
				entry->link = NULL;
				entry->tar_type = (entry->is_tar ? TAR_FILE : '\0');
			}

			if (for_trunc) {
				if (FAILURE == crxa_create_writeable_entry(crxa, entry, error)) {
					return FAILURE;
				}
			} else {
				if (FAILURE == crxa_separate_entry_fp(entry, error)) {
					return FAILURE;
				}
			}
		} else {
			if (FAILURE == crxa_open_entry_fp(entry, error, 1)) {
				return FAILURE;
			}
		}
	}

	*ret = (crxa_entry_data *) emalloc(sizeof(crxa_entry_data));
	(*ret)->position = 0;
	(*ret)->crxa = crxa;
	(*ret)->for_write = for_write;
	(*ret)->internal_file = entry;
	(*ret)->is_zip = entry->is_zip;
	(*ret)->is_tar = entry->is_tar;
	(*ret)->fp = crxa_get_efp(entry, 1);
	if (entry->link) {
		crxa_entry_info *link = crxa_get_link_source(entry);
		if(!link) {
			efree(*ret);
			return FAILURE;
		}
		(*ret)->zero = crxa_get_fp_offset(link);
	} else {
		(*ret)->zero = crxa_get_fp_offset(entry);
	}

	if (!crxa->is_persistent) {
		++(entry->fp_refcount);
		++(entry->crxa->refcount);
	}

	return SUCCESS;
}
/* }}} */

/**
 * Create a new dummy file slot within a writeable crxa for a newly created file
 */
crxa_entry_data *crxa_get_or_create_entry_data(char *fname, size_t fname_len, char *path, size_t path_len, const char *mode, char allow_dir, char **error, int security) /* {{{ */
{
	crxa_archive_data *crxa;
	crxa_entry_info *entry, etemp;
	crxa_entry_data *ret;
	const char *pcr_error;
	char is_dir;

#ifdef CRX_WIN32
	crxa_unixify_path_separators(path, path_len);
#endif

	is_dir = (path_len && path[path_len - 1] == '/') ? 1 : 0;

	if (FAILURE == crxa_get_archive(&crxa, fname, fname_len, NULL, 0, error)) {
		return NULL;
	}

	if (FAILURE == crxa_get_entry_data(&ret, fname, fname_len, path, path_len, mode, allow_dir, error, security)) {
		return NULL;
	} else if (ret) {
		return ret;
	}

	if (crxa_path_check(&path, &path_len, &pcr_error) > pcr_is_ok) {
		if (error) {
			spprintf(error, 0, "crxa error: invalid path \"%s\" contains %s", path, pcr_error);
		}
		return NULL;
	}

	if (crxa->is_persistent && FAILURE == crxa_copy_on_write(&crxa)) {
		if (error) {
			spprintf(error, 4096, "crxa error: file \"%s\" in crxa \"%s\" cannot be created, could not make cached crxa writeable", path, fname);
		}
		return NULL;
	}

	/* create a new crxa data holder */
	ret = (crxa_entry_data *) emalloc(sizeof(crxa_entry_data));

	/* create an entry, this is a new file */
	memset(&etemp, 0, sizeof(crxa_entry_info));
	etemp.filename_len = path_len;
	etemp.fp_type = CRXA_MOD;
	etemp.fp = crx_stream_fopen_tmpfile();

	if (!etemp.fp) {
		if (error) {
			spprintf(error, 0, "crxa error: unable to create temporary file");
		}
		efree(ret);
		return NULL;
	}

	etemp.fp_refcount = 1;

	if (allow_dir == 2) {
		etemp.is_dir = 1;
		etemp.flags = etemp.old_flags = CRXA_ENT_PERM_DEF_DIR;
	} else {
		etemp.flags = etemp.old_flags = CRXA_ENT_PERM_DEF_FILE;
	}
	if (is_dir && path_len) {
		etemp.filename_len--; /* strip trailing / */
		path_len--;
	}

	crxa_add_virtual_dirs(crxa, path, path_len);
	etemp.is_modified = 1;
	etemp.timestamp = time(0);
	etemp.is_crc_checked = 1;
	etemp.crxa = crxa;
	etemp.filename = estrndup(path, path_len);
	etemp.is_zip = crxa->is_zip;

	if (crxa->is_tar) {
		etemp.is_tar = crxa->is_tar;
		etemp.tar_type = etemp.is_dir ? TAR_DIR : TAR_FILE;
	}

	if (NULL == (entry = crex_hash_str_add_mem(&crxa->manifest, etemp.filename, path_len, (void*)&etemp, sizeof(crxa_entry_info)))) {
		crx_stream_close(etemp.fp);
		if (error) {
			spprintf(error, 0, "crxa error: unable to add new entry \"%s\" to crxa \"%s\"", etemp.filename, crxa->fname);
		}
		efree(ret);
		efree(etemp.filename);
		return NULL;
	}

	if (!entry) {
		crx_stream_close(etemp.fp);
		efree(etemp.filename);
		efree(ret);
		return NULL;
	}

	++(crxa->refcount);
	ret->crxa = crxa;
	ret->fp = entry->fp;
	ret->position = ret->zero = 0;
	ret->for_write = 1;
	ret->is_zip = entry->is_zip;
	ret->is_tar = entry->is_tar;
	ret->internal_file = entry;

	return ret;
}
/* }}} */

/* initialize a crxa_archive_data's read-only fp for existing crxa data */
int crxa_open_archive_fp(crxa_archive_data *crxa) /* {{{ */
{
	if (crxa_get_crxafp(crxa)) {
		return SUCCESS;
	}

	if (crx_check_open_basedir(crxa->fname)) {
		return FAILURE;
	}

	crxa_set_crxafp(crxa, crx_stream_open_wrapper(crxa->fname, "rb", IGNORE_URL|STREAM_MUST_SEEK|0, NULL));

	if (!crxa_get_crxafp(crxa)) {
		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */

/* copy file data from an existing to a new crxa_entry_info that is not in the manifest */
int crxa_copy_entry_fp(crxa_entry_info *source, crxa_entry_info *dest, char **error) /* {{{ */
{
	crxa_entry_info *link;

	if (FAILURE == crxa_open_entry_fp(source, error, 1)) {
		return FAILURE;
	}

	if (dest->link) {
		efree(dest->link);
		dest->link = NULL;
		dest->tar_type = (dest->is_tar ? TAR_FILE : '\0');
	}

	dest->fp_type = CRXA_MOD;
	dest->offset = 0;
	dest->is_modified = 1;
	dest->fp = crx_stream_fopen_tmpfile();
	if (dest->fp == NULL) {
		spprintf(error, 0, "crxa error: unable to create temporary file");
		return EOF;
	}
	crxa_seek_efp(source, 0, SEEK_SET, 0, 1);
	link = crxa_get_link_source(source);

	if (!link) {
		link = source;
	}

	if (SUCCESS != crx_stream_copy_to_stream_ex(crxa_get_efp(link, 0), dest->fp, link->uncompressed_filesize, NULL)) {
		crx_stream_close(dest->fp);
		dest->fp_type = CRXA_FP;
		if (error) {
			spprintf(error, 4096, "crxa error: unable to copy contents of file \"%s\" to \"%s\" in crxa archive \"%s\"", source->filename, dest->filename, source->crxa->fname);
		}
		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */

/* open and decompress a compressed crxa entry
 */
int crxa_open_entry_fp(crxa_entry_info *entry, char **error, int follow_links) /* {{{ */
{
	crx_stream_filter *filter;
	crxa_archive_data *crxa = entry->crxa;
	char *filtername;
	crex_off_t loc;
	crx_stream *ufp;
	crxa_entry_data dummy;

	if (follow_links && entry->link) {
		crxa_entry_info *link_entry = crxa_get_link_source(entry);
		if (link_entry && link_entry != entry) {
			return crxa_open_entry_fp(link_entry, error, 1);
		}
	}

	if (entry->is_modified) {
		return SUCCESS;
	}

	if (entry->fp_type == CRXA_TMP) {
		if (!entry->fp) {
			entry->fp = crx_stream_open_wrapper(entry->tmp, "rb", STREAM_MUST_SEEK|0, NULL);
		}
		return SUCCESS;
	}

	if (entry->fp_type != CRXA_FP) {
		/* either newly created or already modified */
		return SUCCESS;
	}

	if (!crxa_get_crxafp(crxa)) {
		if (FAILURE == crxa_open_archive_fp(crxa)) {
			spprintf(error, 4096, "crxa error: Cannot open crxa archive \"%s\" for reading", crxa->fname);
			return FAILURE;
		}
	}

	if ((entry->old_flags && !(entry->old_flags & CRXA_ENT_COMPRESSION_MASK)) || !(entry->flags & CRXA_ENT_COMPRESSION_MASK)) {
		dummy.internal_file = entry;
		dummy.crxa = crxa;
		dummy.zero = entry->offset;
		dummy.fp = crxa_get_crxafp(crxa);
		if (FAILURE == crxa_postprocess_file(&dummy, entry->crc32, error, 1)) {
			return FAILURE;
		}
		return SUCCESS;
	}

	if (!crxa_get_entrypufp(entry)) {
		crxa_set_entrypufp(entry, crx_stream_fopen_tmpfile());
		if (!crxa_get_entrypufp(entry)) {
			spprintf(error, 4096, "crxa error: Cannot open temporary file for decompressing crxa archive \"%s\" file \"%s\"", crxa->fname, entry->filename);
			return FAILURE;
		}
	}

	dummy.internal_file = entry;
	dummy.crxa = crxa;
	dummy.zero = entry->offset;
	dummy.fp = crxa_get_crxafp(crxa);
	if (FAILURE == crxa_postprocess_file(&dummy, entry->crc32, error, 1)) {
		return FAILURE;
	}

	ufp = crxa_get_entrypufp(entry);

	if ((filtername = crxa_decompress_filter(entry, 0)) != NULL) {
		filter = crx_stream_filter_create(filtername, NULL, 0);
	} else {
		filter = NULL;
	}

	if (!filter) {
		spprintf(error, 4096, "crxa error: unable to read crxa \"%s\" (cannot create %s filter while decompressing file \"%s\")", crxa->fname, crxa_decompress_filter(entry, 1), entry->filename);
		return FAILURE;
	}

	/* now we can safely use proper decompression */
	/* save the new offset location within ufp */
	crx_stream_seek(ufp, 0, SEEK_END);
	loc = crx_stream_tell(ufp);
	crx_stream_filter_append(&ufp->writefilters, filter);
	crx_stream_seek(crxa_get_entrypfp(entry), crxa_get_fp_offset(entry), SEEK_SET);

	if (entry->uncompressed_filesize) {
		if (SUCCESS != crx_stream_copy_to_stream_ex(crxa_get_entrypfp(entry), ufp, entry->compressed_filesize, NULL)) {
			spprintf(error, 4096, "crxa error: internal corruption of crxa \"%s\" (actual filesize mismatch on file \"%s\")", crxa->fname, entry->filename);
			crx_stream_filter_remove(filter, 1);
			return FAILURE;
		}
	}

	crx_stream_filter_flush(filter, 1);
	crx_stream_flush(ufp);
	crx_stream_filter_remove(filter, 1);

	if (crx_stream_tell(ufp) - loc != (crex_off_t) entry->uncompressed_filesize) {
		spprintf(error, 4096, "crxa error: internal corruption of crxa \"%s\" (actual filesize mismatch on file \"%s\")", crxa->fname, entry->filename);
		return FAILURE;
	}

	entry->old_flags = entry->flags;

	/* this is now the new location of the file contents within this fp */
	crxa_set_fp_type(entry, CRXA_UFP, loc);
	dummy.zero = entry->offset;
	dummy.fp = ufp;
	if (FAILURE == crxa_postprocess_file(&dummy, entry->crc32, error, 0)) {
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

int crxa_create_writeable_entry(crxa_archive_data *crxa, crxa_entry_info *entry, char **error) /* {{{ */
{
	if (entry->fp_type == CRXA_MOD) {
		/* already newly created, truncate */
		crx_stream_truncate_set_size(entry->fp, 0);

		entry->old_flags = entry->flags;
		entry->is_modified = 1;
		crxa->is_modified = 1;
		/* reset file size */
		entry->uncompressed_filesize = 0;
		entry->compressed_filesize = 0;
		entry->crc32 = 0;
		entry->flags = CRXA_ENT_PERM_DEF_FILE;
		entry->fp_type = CRXA_MOD;
		entry->offset = 0;
		return SUCCESS;
	}

	if (error) {
		*error = NULL;
	}

	/* open a new temp file for writing */
	if (entry->link) {
		efree(entry->link);
		entry->link = NULL;
		entry->tar_type = (entry->is_tar ? TAR_FILE : '\0');
	}

	entry->fp = crx_stream_fopen_tmpfile();

	if (!entry->fp) {
		if (error) {
			spprintf(error, 0, "crxa error: unable to create temporary file");
		}
		return FAILURE;
	}

	entry->old_flags = entry->flags;
	entry->is_modified = 1;
	crxa->is_modified = 1;
	/* reset file size */
	entry->uncompressed_filesize = 0;
	entry->compressed_filesize = 0;
	entry->crc32 = 0;
	entry->flags = CRXA_ENT_PERM_DEF_FILE;
	entry->fp_type = CRXA_MOD;
	entry->offset = 0;
	return SUCCESS;
}
/* }}} */

int crxa_separate_entry_fp(crxa_entry_info *entry, char **error) /* {{{ */
{
	crx_stream *fp;
	crxa_entry_info *link;

	if (FAILURE == crxa_open_entry_fp(entry, error, 1)) {
		return FAILURE;
	}

	if (entry->fp_type == CRXA_MOD) {
		return SUCCESS;
	}

	fp = crx_stream_fopen_tmpfile();
	if (fp == NULL) {
		spprintf(error, 0, "crxa error: unable to create temporary file");
		return FAILURE;
	}
	crxa_seek_efp(entry, 0, SEEK_SET, 0, 1);
	link = crxa_get_link_source(entry);

	if (!link) {
		link = entry;
	}

	if (SUCCESS != crx_stream_copy_to_stream_ex(crxa_get_efp(link, 0), fp, link->uncompressed_filesize, NULL)) {
		if (error) {
			spprintf(error, 4096, "crxa error: cannot separate entry file \"%s\" contents in crxa archive \"%s\" for write access", entry->filename, entry->crxa->fname);
		}
		return FAILURE;
	}

	if (entry->link) {
		efree(entry->link);
		entry->link = NULL;
		entry->tar_type = (entry->is_tar ? TAR_FILE : '\0');
	}

	entry->offset = 0;
	entry->fp = fp;
	entry->fp_type = CRXA_MOD;
	entry->is_modified = 1;
	return SUCCESS;
}
/* }}} */

/**
 * helper function to open an internal file's fp just-in-time
 */
crxa_entry_info * crxa_open_jit(crxa_archive_data *crxa, crxa_entry_info *entry, char **error) /* {{{ */
{
	if (error) {
		*error = NULL;
	}
	/* seek to start of internal file and read it */
	if (FAILURE == crxa_open_entry_fp(entry, error, 1)) {
		return NULL;
	}
	if (-1 == crxa_seek_efp(entry, 0, SEEK_SET, 0, 1)) {
		spprintf(error, 4096, "crxa error: cannot seek to start of file \"%s\" in crxa \"%s\"", entry->filename, crxa->fname);
		return NULL;
	}
	return entry;
}
/* }}} */

CRX_CRXA_API int crxa_resolve_alias(char *alias, size_t alias_len, char **filename, size_t *filename_len) /* {{{ */ {
	crxa_archive_data *fd_ptr;
	if (HT_IS_INITIALIZED(&CRXA_G(crxa_alias_map))
			&& NULL != (fd_ptr = crex_hash_str_find_ptr(&(CRXA_G(crxa_alias_map)), alias, alias_len))) {
		*filename = fd_ptr->fname;
		*filename_len = fd_ptr->fname_len;
		return SUCCESS;
	}
	return FAILURE;
}
/* }}} */

int crxa_free_alias(crxa_archive_data *crxa, char *alias, size_t alias_len) /* {{{ */
{
	if (crxa->refcount || crxa->is_persistent) {
		return FAILURE;
	}

	/* this archive has no open references, so emit a notice and remove it */
	if (crex_hash_str_del(&(CRXA_G(crxa_fname_map)), crxa->fname, crxa->fname_len) != SUCCESS) {
		return FAILURE;
	}

	/* invalidate crxa cache */
	CRXA_G(last_crxa) = NULL;
	CRXA_G(last_crxa_name) = CRXA_G(last_alias) = NULL;

	return SUCCESS;
}
/* }}} */

/**
 * Looks up a crxa archive in the filename map, connecting it to the alias
 * (if any) or returns null
 */
int crxa_get_archive(crxa_archive_data **archive, char *fname, size_t fname_len, char *alias, size_t alias_len, char **error) /* {{{ */
{
	crxa_archive_data *fd, *fd_ptr;
	char *my_realpath, *save;
	size_t save_len;

	crxa_request_initialize();

	if (error) {
		*error = NULL;
	}

	*archive = NULL;

	if (CRXA_G(last_crxa) && fname_len == CRXA_G(last_crxa_name_len) && !memcmp(fname, CRXA_G(last_crxa_name), fname_len)) {
		*archive = CRXA_G(last_crxa);
		if (alias && alias_len) {

			if (!CRXA_G(last_crxa)->is_temporary_alias && (alias_len != CRXA_G(last_crxa)->alias_len || memcmp(CRXA_G(last_crxa)->alias, alias, alias_len))) {
				if (error) {
					spprintf(error, 0, "alias \"%s\" is already used for archive \"%s\" cannot be overloaded with \"%s\"", alias, CRXA_G(last_crxa)->fname, fname);
				}
				*archive = NULL;
				return FAILURE;
			}

			if (CRXA_G(last_crxa)->alias_len && NULL != (fd_ptr = crex_hash_str_find_ptr(&(CRXA_G(crxa_alias_map)), CRXA_G(last_crxa)->alias, CRXA_G(last_crxa)->alias_len))) {
				crex_hash_str_del(&(CRXA_G(crxa_alias_map)), CRXA_G(last_crxa)->alias, CRXA_G(last_crxa)->alias_len);
			}

			crex_hash_str_add_ptr(&(CRXA_G(crxa_alias_map)), alias, alias_len, *archive);
			CRXA_G(last_alias) = alias;
			CRXA_G(last_alias_len) = alias_len;
		}

		return SUCCESS;
	}

	if (alias && alias_len && CRXA_G(last_crxa) && alias_len == CRXA_G(last_alias_len) && !memcmp(alias, CRXA_G(last_alias), alias_len)) {
		fd = CRXA_G(last_crxa);
		fd_ptr = fd;
		goto alias_success;
	}

	if (alias && alias_len) {
		if (NULL != (fd_ptr = crex_hash_str_find_ptr(&(CRXA_G(crxa_alias_map)), alias, alias_len))) {
alias_success:
			if (fname && (fname_len != fd_ptr->fname_len || strncmp(fname, fd_ptr->fname, fname_len))) {
				if (error) {
					spprintf(error, 0, "alias \"%s\" is already used for archive \"%s\" cannot be overloaded with \"%s\"", alias, fd_ptr->fname, fname);
				}
				if (SUCCESS == crxa_free_alias(fd_ptr, alias, alias_len)) {
					if (error) {
						efree(*error);
						*error = NULL;
					}
				}
				return FAILURE;
			}

			*archive = fd_ptr;
			fd = fd_ptr;
			CRXA_G(last_crxa) = fd;
			CRXA_G(last_crxa_name) = fd->fname;
			CRXA_G(last_crxa_name_len) = fd->fname_len;
			CRXA_G(last_alias) = alias;
			CRXA_G(last_alias_len) = alias_len;

			return SUCCESS;
		}

		if (CRXA_G(manifest_cached) && NULL != (fd_ptr = crex_hash_str_find_ptr(&cached_alias, alias, alias_len))) {
			goto alias_success;
		}
	}

	my_realpath = NULL;
	save = fname;
	save_len = fname_len;

	if (fname && fname_len) {
		if (NULL != (fd_ptr = crex_hash_str_find_ptr(&(CRXA_G(crxa_fname_map)), fname, fname_len))) {
			*archive = fd_ptr;
			fd = fd_ptr;

			if (alias && alias_len) {
				if (!fd->is_temporary_alias && (alias_len != fd->alias_len || memcmp(fd->alias, alias, alias_len))) {
					if (error) {
						spprintf(error, 0, "alias \"%s\" is already used for archive \"%s\" cannot be overloaded with \"%s\"", alias, fd_ptr->fname, fname);
					}
					return FAILURE;
				}

				if (fd->alias_len && NULL != (fd_ptr = crex_hash_str_find_ptr(&(CRXA_G(crxa_alias_map)), fd->alias, fd->alias_len))) {
					crex_hash_str_del(&(CRXA_G(crxa_alias_map)), fd->alias, fd->alias_len);
				}

				crex_hash_str_add_ptr(&(CRXA_G(crxa_alias_map)), alias, alias_len, fd);
			}

			CRXA_G(last_crxa) = fd;
			CRXA_G(last_crxa_name) = fd->fname;
			CRXA_G(last_crxa_name_len) = fd->fname_len;
			CRXA_G(last_alias) = fd->alias;
			CRXA_G(last_alias_len) = fd->alias_len;

			return SUCCESS;
		}

		if (CRXA_G(manifest_cached) && NULL != (fd_ptr = crex_hash_str_find_ptr(&cached_crxas, fname, fname_len))) {
			*archive = fd_ptr;
			fd = fd_ptr;

			/* this could be problematic - alias should never be different from manifest alias
			   for cached crxas */
			if (!fd->is_temporary_alias && alias && alias_len) {
				if (alias_len != fd->alias_len || memcmp(fd->alias, alias, alias_len)) {
					if (error) {
						spprintf(error, 0, "alias \"%s\" is already used for archive \"%s\" cannot be overloaded with \"%s\"", alias, fd_ptr->fname, fname);
					}
					return FAILURE;
				}
			}

			CRXA_G(last_crxa) = fd;
			CRXA_G(last_crxa_name) = fd->fname;
			CRXA_G(last_crxa_name_len) = fd->fname_len;
			CRXA_G(last_alias) = fd->alias;
			CRXA_G(last_alias_len) = fd->alias_len;

			return SUCCESS;
		}

		if (NULL != (fd_ptr = crex_hash_str_find_ptr(&(CRXA_G(crxa_alias_map)), save, save_len))) {
			fd = *archive = fd_ptr;

			CRXA_G(last_crxa) = fd;
			CRXA_G(last_crxa_name) = fd->fname;
			CRXA_G(last_crxa_name_len) = fd->fname_len;
			CRXA_G(last_alias) = fd->alias;
			CRXA_G(last_alias_len) = fd->alias_len;

			return SUCCESS;
		}

		if (CRXA_G(manifest_cached) && NULL != (fd_ptr = crex_hash_str_find_ptr(&cached_alias, save, save_len))) {
			fd = *archive = fd_ptr;

			CRXA_G(last_crxa) = fd;
			CRXA_G(last_crxa_name) = fd->fname;
			CRXA_G(last_crxa_name_len) = fd->fname_len;
			CRXA_G(last_alias) = fd->alias;
			CRXA_G(last_alias_len) = fd->alias_len;

			return SUCCESS;
		}

		/* not found, try converting \ to / */
		my_realpath = expand_filepath(fname, my_realpath);

		if (my_realpath) {
			fname_len = strlen(my_realpath);
			fname = my_realpath;
		} else {
			return FAILURE;
		}
#ifdef CRX_WIN32
		crxa_unixify_path_separators(fname, fname_len);
#endif

		if (NULL != (fd_ptr = crex_hash_str_find_ptr(&(CRXA_G(crxa_fname_map)), fname, fname_len))) {
realpath_success:
			*archive = fd_ptr;
			fd = fd_ptr;

			if (alias && alias_len) {
				crex_hash_str_add_ptr(&(CRXA_G(crxa_alias_map)), alias, alias_len, fd);
			}

			efree(my_realpath);

			CRXA_G(last_crxa) = fd;
			CRXA_G(last_crxa_name) = fd->fname;
			CRXA_G(last_crxa_name_len) = fd->fname_len;
			CRXA_G(last_alias) = fd->alias;
			CRXA_G(last_alias_len) = fd->alias_len;

			return SUCCESS;
		}

		if (CRXA_G(manifest_cached) && NULL != (fd_ptr = crex_hash_str_find_ptr(&cached_crxas, fname, fname_len))) {
			goto realpath_success;
		}

		efree(my_realpath);
	}

	return FAILURE;
}
/* }}} */

/**
 * Determine which stream compression filter (if any) we need to read this file
 */
char * crxa_compress_filter(crxa_entry_info * entry, int return_unknown) /* {{{ */
{
	switch (entry->flags & CRXA_ENT_COMPRESSION_MASK) {
	case CRXA_ENT_COMPRESSED_GZ:
		return "zlib.deflate";
	case CRXA_ENT_COMPRESSED_BZ2:
		return "bzip2.compress";
	default:
		return return_unknown ? "unknown" : NULL;
	}
}
/* }}} */

/**
 * Determine which stream decompression filter (if any) we need to read this file
 */
char * crxa_decompress_filter(crxa_entry_info * entry, int return_unknown) /* {{{ */
{
	uint32_t flags;

	if (entry->is_modified) {
		flags = entry->old_flags;
	} else {
		flags = entry->flags;
	}

	switch (flags & CRXA_ENT_COMPRESSION_MASK) {
		case CRXA_ENT_COMPRESSED_GZ:
			return "zlib.inflate";
		case CRXA_ENT_COMPRESSED_BZ2:
			return "bzip2.decompress";
		default:
			return return_unknown ? "unknown" : NULL;
	}
}
/* }}} */

/**
 * retrieve information on a file contained within a crxa, or null if it ain't there
 */
crxa_entry_info *crxa_get_entry_info(crxa_archive_data *crxa, char *path, size_t path_len, char **error, int security) /* {{{ */
{
	return crxa_get_entry_info_dir(crxa, path, path_len, 0, error, security);
}
/* }}} */
/**
 * retrieve information on a file or directory contained within a crxa, or null if none found
 * allow_dir is 0 for none, 1 for both empty directories in the crxa and temp directories, and 2 for only
 * valid pre-existing empty directory entries
 */
crxa_entry_info *crxa_get_entry_info_dir(crxa_archive_data *crxa, char *path, size_t path_len, char dir, char **error, int security) /* {{{ */
{
	const char *pcr_error;
	crxa_entry_info *entry;
	int is_dir;

#ifdef CRX_WIN32
	crxa_unixify_path_separators(path, path_len);
#endif

	is_dir = (path_len && (path[path_len - 1] == '/')) ? 1 : 0;

	if (error) {
		*error = NULL;
	}

	if (security && path_len >= sizeof(".crxa")-1 && !memcmp(path, ".crxa", sizeof(".crxa")-1)) {
		if (error) {
			spprintf(error, 4096, "crxa error: cannot directly access magic \".crxa\" directory or files within it");
		}
		return NULL;
	}

	if (!path_len && !dir) {
		if (error) {
			spprintf(error, 4096, "crxa error: invalid path \"%s\" must not be empty", path);
		}
		return NULL;
	}

	if (crxa_path_check(&path, &path_len, &pcr_error) > pcr_is_ok) {
		if (error) {
			spprintf(error, 4096, "crxa error: invalid path \"%s\" contains %s", path, pcr_error);
		}
		return NULL;
	}

	if (!HT_IS_INITIALIZED(&crxa->manifest)) {
		return NULL;
	}

	if (is_dir) {
		if (path_len <= 1) {
			return NULL;
		}
		path_len--;
	}

	if (NULL != (entry = crex_hash_str_find_ptr(&crxa->manifest, path, path_len))) {
		if (entry->is_deleted) {
			/* entry is deleted, but has not been flushed to disk yet */
			return NULL;
		}
		if (entry->is_dir && !dir) {
			if (error) {
				spprintf(error, 4096, "crxa error: path \"%s\" is a directory", path);
			}
			return NULL;
		}
		if (!entry->is_dir && dir == 2) {
			/* user requested a directory, we must return one */
			if (error) {
				spprintf(error, 4096, "crxa error: path \"%s\" exists and is a not a directory", path);
			}
			return NULL;
		}
		return entry;
	}

	if (dir) {
		if (crex_hash_str_exists(&crxa->virtual_dirs, path, path_len)) {
			/* a file or directory exists in a sub-directory of this path */
			entry = (crxa_entry_info *) ecalloc(1, sizeof(crxa_entry_info));
			/* this next line tells CrxaFileInfo->__destruct() to efree the filename */
			entry->is_temp_dir = entry->is_dir = 1;
			entry->filename = (char *) estrndup(path, path_len + 1);
			entry->filename_len = path_len;
			entry->crxa = crxa;
			return entry;
		}
	}

	if (HT_IS_INITIALIZED(&crxa->mounted_dirs) && crex_hash_num_elements(&crxa->mounted_dirs)) {
		crex_string *str_key;

		CREX_HASH_MAP_FOREACH_STR_KEY(&crxa->mounted_dirs, str_key) {
			if (ZSTR_LEN(str_key) >= path_len || strncmp(ZSTR_VAL(str_key), path, ZSTR_LEN(str_key))) {
				continue;
			} else {
				char *test;
				size_t test_len;
				crx_stream_statbuf ssb;

				if (NULL == (entry = crex_hash_find_ptr(&crxa->manifest, str_key))) {
					if (error) {
						spprintf(error, 4096, "crxa internal error: mounted path \"%s\" could not be retrieved from manifest", ZSTR_VAL(str_key));
					}
					return NULL;
				}

				if (!entry->tmp || !entry->is_mounted) {
					if (error) {
						spprintf(error, 4096, "crxa internal error: mounted path \"%s\" is not properly initialized as a mounted path", ZSTR_VAL(str_key));
					}
					return NULL;
				}

				test_len = spprintf(&test, MAXPATHLEN, "%s%s", entry->tmp, path + ZSTR_LEN(str_key));

				if (SUCCESS != crx_stream_stat_path(test, &ssb)) {
					efree(test);
					return NULL;
				}

				if ((ssb.sb.st_mode & S_IFDIR) && !dir) {
					efree(test);
					if (error) {
						spprintf(error, 4096, "crxa error: path \"%s\" is a directory", path);
					}
					return NULL;
				}

				if ((ssb.sb.st_mode & S_IFDIR) == 0 && dir) {
					efree(test);
					/* user requested a directory, we must return one */
					if (error) {
						spprintf(error, 4096, "crxa error: path \"%s\" exists and is a not a directory", path);
					}
					return NULL;
				}

				/* mount the file just in time */
				if (SUCCESS != crxa_mount_entry(crxa, test, test_len, path, path_len)) {
					efree(test);
					if (error) {
						spprintf(error, 4096, "crxa error: path \"%s\" exists as file \"%s\" and could not be mounted", path, test);
					}
					return NULL;
				}

				efree(test);

				if (NULL == (entry = crex_hash_str_find_ptr(&crxa->manifest, path, path_len))) {
					if (error) {
						spprintf(error, 4096, "crxa error: path \"%s\" exists as file \"%s\" and could not be retrieved after being mounted", path, test);
					}
					return NULL;
				}
				return entry;
			}
		} CREX_HASH_FOREACH_END();
	}

	return NULL;
}
/* }}} */

static const char hexChars[] = "0123456789ABCDEF";

static int crxa_hex_str(const char *digest, size_t digest_len, char **signature) /* {{{ */
{
	int pos = -1;
	size_t len = 0;

	*signature = (char*)safe_pemalloc(digest_len, 2, 1, CRXA_G(persist));

	for (; len < digest_len; ++len) {
		(*signature)[++pos] = hexChars[((const unsigned char *)digest)[len] >> 4];
		(*signature)[++pos] = hexChars[((const unsigned char *)digest)[len] & 0x0F];
	}
	(*signature)[++pos] = '\0';
	return pos;
}
/* }}} */

#ifndef CRXA_HAVE_OPENSSL
static int crxa_call_openssl_signverify(int is_sign, crx_stream *fp, crex_off_t end, char *key, size_t key_len, char **signature, size_t *signature_len, crx_uint32 sig_type) /* {{{ */
{
	crex_fcall_info fci;
	crex_fcall_info_cache fcc;
	zval retval, zp[4], openssl;
	crex_string *str;

	ZVAL_STRINGL(&openssl, is_sign ? "openssl_sign" : "openssl_verify", is_sign ? sizeof("openssl_sign")-1 : sizeof("openssl_verify")-1);
	if (*signature_len) {
		ZVAL_STRINGL(&zp[1], *signature, *signature_len);
	} else {
		ZVAL_EMPTY_STRING(&zp[1]);
	}
	ZVAL_STRINGL(&zp[2], key, key_len);
	crx_stream_rewind(fp);
	str = crx_stream_copy_to_mem(fp, (size_t) end, 0);
	if (str) {
		ZVAL_STR(&zp[0], str);
	} else {
		ZVAL_EMPTY_STRING(&zp[0]);
	}
	if (sig_type == CRXA_SIG_OPENSSL_SHA512) {
		ZVAL_LONG(&zp[3], 9); /* value from openssl.c #define OPENSSL_ALGO_SHA512 9 */
	} else if (sig_type == CRXA_SIG_OPENSSL_SHA256) {
		ZVAL_LONG(&zp[3], 7); /* value from openssl.c #define OPENSSL_ALGO_SHA256 7 */
	} else {
		/* don't rely on default value which may change in the future */
		ZVAL_LONG(&zp[3], 1); /* value from openssl.c #define OPENSSL_ALGO_SHA1   1 */
	}

	if ((size_t)end != C_STRLEN(zp[0])) {
		zval_ptr_dtor_str(&zp[0]);
		zval_ptr_dtor_str(&zp[1]);
		zval_ptr_dtor_str(&zp[2]);
		zval_ptr_dtor_str(&openssl);
		return FAILURE;
	}

	if (FAILURE == crex_fcall_info_init(&openssl, 0, &fci, &fcc, NULL, NULL)) {
		zval_ptr_dtor_str(&zp[0]);
		zval_ptr_dtor_str(&zp[1]);
		zval_ptr_dtor_str(&zp[2]);
		zval_ptr_dtor_str(&openssl);
		return FAILURE;
	}

	fci.param_count = 4;
	fci.params = zp;
	C_ADDREF(zp[0]);
	if (is_sign) {
		ZVAL_NEW_REF(&zp[1], &zp[1]);
	} else {
		C_ADDREF(zp[1]);
	}
	C_ADDREF(zp[2]);

	fci.retval = &retval;

	if (FAILURE == crex_call_function(&fci, &fcc)) {
		zval_ptr_dtor_str(&zp[0]);
		zval_ptr_dtor(&zp[1]);
		zval_ptr_dtor_str(&zp[2]);
		zval_ptr_dtor_str(&openssl);
		return FAILURE;
	}

	zval_ptr_dtor_str(&openssl);
	C_DELREF(zp[0]);

	if (is_sign) {
		ZVAL_UNREF(&zp[1]);
	} else {
		C_DELREF(zp[1]);
	}
	C_DELREF(zp[2]);

	zval_ptr_dtor_str(&zp[0]);
	zval_ptr_dtor_str(&zp[2]);

	switch (C_TYPE(retval)) {
		default:
		case IS_LONG:
			zval_ptr_dtor(&zp[1]);
			if (1 == C_LVAL(retval)) {
				return SUCCESS;
			}
			return FAILURE;
		case IS_TRUE:
			*signature = estrndup(C_STRVAL(zp[1]), C_STRLEN(zp[1]));
			*signature_len = C_STRLEN(zp[1]);
			zval_ptr_dtor(&zp[1]);
			return SUCCESS;
		case IS_FALSE:
			zval_ptr_dtor(&zp[1]);
			return FAILURE;
	}
}
/* }}} */
#endif /* #ifndef CRXA_HAVE_OPENSSL */

int crxa_verify_signature(crx_stream *fp, size_t end_of_crxa, uint32_t sig_type, char *sig, size_t sig_len, char *fname, char **signature, size_t *signature_len, char **error) /* {{{ */
{
	size_t read_size, len;
	crex_off_t read_len;
	unsigned char buf[1024];

	crx_stream_rewind(fp);

	switch (sig_type) {
		case CRXA_SIG_OPENSSL_SHA512:
		case CRXA_SIG_OPENSSL_SHA256:
		case CRXA_SIG_OPENSSL: {
#ifdef CRXA_HAVE_OPENSSL
			BIO *in;
			EVP_PKEY *key;
			const EVP_MD *mdtype;
			EVP_MD_CTX *md_ctx;

			if (sig_type == CRXA_SIG_OPENSSL_SHA512) {
				mdtype = EVP_sha512();
			} else if (sig_type == CRXA_SIG_OPENSSL_SHA256) {
				mdtype = EVP_sha256();
			} else {
				mdtype = EVP_sha1();
			}
#else
			size_t tempsig;
#endif
			crex_string *pubkey = NULL;
			char *pfile;
			crx_stream *pfp;
#ifndef CRXA_HAVE_OPENSSL
			if (!crex_hash_str_exists(&module_registry, "openssl", sizeof("openssl")-1)) {
				if (error) {
					spprintf(error, 0, "openssl not loaded");
				}
				return FAILURE;
			}
#endif
			/* use __FILE__ . '.pubkey' for public key file */
			spprintf(&pfile, 0, "%s.pubkey", fname);
			pfp = crx_stream_open_wrapper(pfile, "rb", 0, NULL);
			efree(pfile);

			if (!pfp || !(pubkey = crx_stream_copy_to_mem(pfp, CRX_STREAM_COPY_ALL, 0)) || !ZSTR_LEN(pubkey)) {
				if (pfp) {
					crx_stream_close(pfp);
				}
				if (error) {
					spprintf(error, 0, "openssl public key could not be read");
				}
				return FAILURE;
			}

			crx_stream_close(pfp);
#ifndef CRXA_HAVE_OPENSSL
			tempsig = sig_len;

			if (FAILURE == crxa_call_openssl_signverify(0, fp, end_of_crxa, ZSTR_VAL(pubkey), ZSTR_LEN(pubkey), &sig, &tempsig, sig_type)) {
				crex_string_release_ex(pubkey, 0);

				if (error) {
					spprintf(error, 0, "openssl signature could not be verified");
				}

				return FAILURE;
			}

			crex_string_release_ex(pubkey, 0);

			sig_len = tempsig;
#else
			in = BIO_new_mem_buf(ZSTR_VAL(pubkey), ZSTR_LEN(pubkey));

			if (NULL == in) {
				crex_string_release_ex(pubkey, 0);
				if (error) {
					spprintf(error, 0, "openssl signature could not be processed");
				}
				return FAILURE;
			}

			key = PEM_read_bio_PUBKEY(in, NULL, NULL, NULL);
			BIO_free(in);
			crex_string_release_ex(pubkey, 0);

			if (NULL == key) {
				if (error) {
					spprintf(error, 0, "openssl signature could not be processed");
				}
				return FAILURE;
			}

			md_ctx = EVP_MD_CTX_create();
			if (!md_ctx || !EVP_VerifyInit(md_ctx, mdtype)) {
				if (md_ctx) {
					EVP_MD_CTX_destroy(md_ctx);
				}
				if (error) {
					spprintf(error, 0, "openssl signature could not be verified");
				}
				return FAILURE;
			}
			read_len = end_of_crxa;

			if ((size_t)read_len > sizeof(buf)) {
				read_size = sizeof(buf);
			} else {
				read_size = (size_t)read_len;
			}

			crx_stream_seek(fp, 0, SEEK_SET);

			while (read_size && (len = crx_stream_read(fp, (char*)buf, read_size)) > 0) {
				if (UNEXPECTED(EVP_VerifyUpdate (md_ctx, buf, len) == 0)) {
					goto failure;
				}
				read_len -= (crex_off_t)len;

				if (read_len < read_size) {
					read_size = (size_t)read_len;
				}
			}

			if (EVP_VerifyFinal(md_ctx, (unsigned char *)sig, sig_len, key) != 1) {
				failure:
				/* 1: signature verified, 0: signature does not match, -1: failed signature operation */
				EVP_PKEY_free(key);
				EVP_MD_CTX_destroy(md_ctx);

				if (error) {
					spprintf(error, 0, "broken openssl signature");
				}

				return FAILURE;
			}

			EVP_PKEY_free(key);
			EVP_MD_CTX_destroy(md_ctx);
#endif

			*signature_len = crxa_hex_str((const char*)sig, sig_len, signature);
		}
		break;
		case CRXA_SIG_SHA512: {
			unsigned char digest[64];
			CRX_SHA512_CTX context;

			if (sig_len < sizeof(digest)) {
				if (error) {
					spprintf(error, 0, "broken signature");
				}
				return FAILURE;
			}

			CRX_SHA512Init(&context);
			read_len = end_of_crxa;

			if ((size_t)read_len > sizeof(buf)) {
				read_size = sizeof(buf);
			} else {
				read_size = (size_t)read_len;
			}

			while ((len = crx_stream_read(fp, (char*)buf, read_size)) > 0) {
				CRX_SHA512Update(&context, buf, len);
				read_len -= (crex_off_t)len;
				if ((size_t)read_len < read_size) {
					read_size = (size_t)read_len;
				}
			}

			CRX_SHA512Final(digest, &context);

			if (memcmp(digest, sig, sizeof(digest))) {
				if (error) {
					spprintf(error, 0, "broken signature");
				}
				return FAILURE;
			}

			*signature_len = crxa_hex_str((const char*)digest, sizeof(digest), signature);
			break;
		}
		case CRXA_SIG_SHA256: {
			unsigned char digest[32];
			CRX_SHA256_CTX context;

			if (sig_len < sizeof(digest)) {
				if (error) {
					spprintf(error, 0, "broken signature");
				}
				return FAILURE;
			}

			CRX_SHA256Init(&context);
			read_len = end_of_crxa;

			if ((size_t)read_len > sizeof(buf)) {
				read_size = sizeof(buf);
			} else {
				read_size = (size_t)read_len;
			}

			while ((len = crx_stream_read(fp, (char*)buf, read_size)) > 0) {
				CRX_SHA256Update(&context, buf, len);
				read_len -= (crex_off_t)len;
				if ((size_t)read_len < read_size) {
					read_size = (size_t)read_len;
				}
			}

			CRX_SHA256Final(digest, &context);

			if (memcmp(digest, sig, sizeof(digest))) {
				if (error) {
					spprintf(error, 0, "broken signature");
				}
				return FAILURE;
			}

			*signature_len = crxa_hex_str((const char*)digest, sizeof(digest), signature);
			break;
		}
		case CRXA_SIG_SHA1: {
			unsigned char digest[20];
			CRX_SHA1_CTX  context;

			if (sig_len < sizeof(digest)) {
				if (error) {
					spprintf(error, 0, "broken signature");
				}
				return FAILURE;
			}

			CRX_SHA1Init(&context);
			read_len = end_of_crxa;

			if ((size_t)read_len > sizeof(buf)) {
				read_size = sizeof(buf);
			} else {
				read_size = (size_t)read_len;
			}

			while ((len = crx_stream_read(fp, (char*)buf, read_size)) > 0) {
				CRX_SHA1Update(&context, buf, len);
				read_len -= (crex_off_t)len;
				if ((size_t)read_len < read_size) {
					read_size = (size_t)read_len;
				}
			}

			CRX_SHA1Final(digest, &context);

			if (memcmp(digest, sig, sizeof(digest))) {
				if (error) {
					spprintf(error, 0, "broken signature");
				}
				return FAILURE;
			}

			*signature_len = crxa_hex_str((const char*)digest, sizeof(digest), signature);
			break;
		}
		case CRXA_SIG_MD5: {
			unsigned char digest[16];
			CRX_MD5_CTX   context;

			if (sig_len < sizeof(digest)) {
				if (error) {
					spprintf(error, 0, "broken signature");
				}
				return FAILURE;
			}

			CRX_MD5Init(&context);
			read_len = end_of_crxa;

			if ((size_t)read_len > sizeof(buf)) {
				read_size = sizeof(buf);
			} else {
				read_size = (size_t)read_len;
			}

			while ((len = crx_stream_read(fp, (char*)buf, read_size)) > 0) {
				CRX_MD5Update(&context, buf, len);
				read_len -= (crex_off_t)len;
				if ((size_t)read_len < read_size) {
					read_size = (size_t)read_len;
				}
			}

			CRX_MD5Final(digest, &context);

			if (memcmp(digest, sig, sizeof(digest))) {
				if (error) {
					spprintf(error, 0, "broken signature");
				}
				return FAILURE;
			}

			*signature_len = crxa_hex_str((const char*)digest, sizeof(digest), signature);
			break;
		}
		default:
			if (error) {
				spprintf(error, 0, "broken or unsupported signature");
			}
			return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

int crxa_create_signature(crxa_archive_data *crxa, crx_stream *fp, char **signature, size_t *signature_length, char **error) /* {{{ */
{
	unsigned char buf[1024];
	size_t sig_len;

	crx_stream_rewind(fp);

	if (crxa->signature) {
		efree(crxa->signature);
		crxa->signature = NULL;
	}

	switch(crxa->sig_flags) {
		case CRXA_SIG_SHA512: {
			unsigned char digest[64];
			CRX_SHA512_CTX context;

			CRX_SHA512Init(&context);

			while ((sig_len = crx_stream_read(fp, (char*)buf, sizeof(buf))) > 0) {
				CRX_SHA512Update(&context, buf, sig_len);
			}

			CRX_SHA512Final(digest, &context);
			*signature = estrndup((char *) digest, 64);
			*signature_length = 64;
			break;
		}
		default:
			crxa->sig_flags = CRXA_SIG_SHA256;
			CREX_FALLTHROUGH;
		case CRXA_SIG_SHA256: {
			unsigned char digest[32];
			CRX_SHA256_CTX  context;

			CRX_SHA256Init(&context);

			while ((sig_len = crx_stream_read(fp, (char*)buf, sizeof(buf))) > 0) {
				CRX_SHA256Update(&context, buf, sig_len);
			}

			CRX_SHA256Final(digest, &context);
			*signature = estrndup((char *) digest, 32);
			*signature_length = 32;
			break;
		}
		case CRXA_SIG_OPENSSL_SHA512:
		case CRXA_SIG_OPENSSL_SHA256:
		case CRXA_SIG_OPENSSL: {
			unsigned char *sigbuf;
#ifdef CRXA_HAVE_OPENSSL
			unsigned int siglen;
			BIO *in;
			EVP_PKEY *key;
			EVP_MD_CTX *md_ctx;
			const EVP_MD *mdtype;

			if (crxa->sig_flags == CRXA_SIG_OPENSSL_SHA512) {
				mdtype = EVP_sha512();
			} else if (crxa->sig_flags == CRXA_SIG_OPENSSL_SHA256) {
				mdtype = EVP_sha256();
			} else {
				mdtype = EVP_sha1();
			}

			in = BIO_new_mem_buf(CRXA_G(openssl_privatekey), CRXA_G(openssl_privatekey_len));

			if (in == NULL) {
				if (error) {
					spprintf(error, 0, "unable to write to crxa \"%s\" with requested openssl signature", crxa->fname);
				}
				return FAILURE;
			}

			key = PEM_read_bio_PrivateKey(in, NULL,NULL, "");
			BIO_free(in);

			if (!key) {
				if (error) {
					spprintf(error, 0, "unable to process private key");
				}
				return FAILURE;
			}

			md_ctx = EVP_MD_CTX_create();

			siglen = EVP_PKEY_size(key);
			sigbuf = emalloc(siglen + 1);

			if (!EVP_SignInit(md_ctx, mdtype)) {
				EVP_PKEY_free(key);
				efree(sigbuf);
				if (error) {
					spprintf(error, 0, "unable to initialize openssl signature for crxa \"%s\"", crxa->fname);
				}
				return FAILURE;
			}

			while ((sig_len = crx_stream_read(fp, (char*)buf, sizeof(buf))) > 0) {
				if (!EVP_SignUpdate(md_ctx, buf, sig_len)) {
					EVP_PKEY_free(key);
					efree(sigbuf);
					if (error) {
						spprintf(error, 0, "unable to update the openssl signature for crxa \"%s\"", crxa->fname);
					}
					return FAILURE;
				}
			}

			if (!EVP_SignFinal (md_ctx, sigbuf, &siglen, key)) {
				EVP_PKEY_free(key);
				efree(sigbuf);
				if (error) {
					spprintf(error, 0, "unable to write crxa \"%s\" with requested openssl signature", crxa->fname);
				}
				return FAILURE;
			}

			sigbuf[siglen] = '\0';
			EVP_PKEY_free(key);
			EVP_MD_CTX_destroy(md_ctx);
#else
			size_t siglen;
			sigbuf = NULL;
			siglen = 0;
			crx_stream_seek(fp, 0, SEEK_END);

			if (FAILURE == crxa_call_openssl_signverify(1, fp, crx_stream_tell(fp), CRXA_G(openssl_privatekey), CRXA_G(openssl_privatekey_len), (char **)&sigbuf, &siglen, crxa->sig_flags)) {
				if (error) {
					spprintf(error, 0, "unable to write crxa \"%s\" with requested openssl signature", crxa->fname);
				}
				return FAILURE;
			}
#endif
			*signature = (char *) sigbuf;
			*signature_length = siglen;
		}
		break;
		case CRXA_SIG_SHA1: {
			unsigned char digest[20];
			CRX_SHA1_CTX  context;

			CRX_SHA1Init(&context);

			while ((sig_len = crx_stream_read(fp, (char*)buf, sizeof(buf))) > 0) {
				CRX_SHA1Update(&context, buf, sig_len);
			}

			CRX_SHA1Final(digest, &context);
			*signature = estrndup((char *) digest, 20);
			*signature_length = 20;
			break;
		}
		case CRXA_SIG_MD5: {
			unsigned char digest[16];
			CRX_MD5_CTX   context;

			CRX_MD5Init(&context);

			while ((sig_len = crx_stream_read(fp, (char*)buf, sizeof(buf))) > 0) {
				CRX_MD5Update(&context, buf, sig_len);
			}

			CRX_MD5Final(digest, &context);
			*signature = estrndup((char *) digest, 16);
			*signature_length = 16;
			break;
		}
	}

	crxa->sig_len = crxa_hex_str((const char *)*signature, *signature_length, &crxa->signature);
	return SUCCESS;
}
/* }}} */

void crxa_add_virtual_dirs(crxa_archive_data *crxa, char *filename, size_t filename_len) /* {{{ */
{
	const char *s;
	crex_string *str;
	zval *ret;

	while ((s = crex_memrchr(filename, '/', filename_len))) {
		filename_len = s - filename;
		if (!filename_len) {
			break;
		}
		if (GC_FLAGS(&crxa->virtual_dirs) & GC_PERSISTENT) {
			str = crex_string_init_interned(filename, filename_len, 1);
		} else {
			str = crex_string_init(filename, filename_len, 0);
		}
		ret = crex_hash_add_empty_element(&crxa->virtual_dirs, str);
		crex_string_release(str);
		if (ret == NULL) {
			break;
		}
	}
}
/* }}} */

static int crxa_update_cached_entry(zval *data, void *argument) /* {{{ */
{
	crxa_entry_info *entry = (crxa_entry_info *)C_PTR_P(data);

	entry->crxa = (crxa_archive_data *)argument;

	if (entry->link) {
		entry->link = estrdup(entry->link);
	}

	if (entry->tmp) {
		entry->tmp = estrdup(entry->tmp);
	}

	entry->filename = estrndup(entry->filename, entry->filename_len);
	entry->is_persistent = 0;

	/* Replace metadata with non-persistent clones of the metadata. */
	crxa_metadata_tracker_clone(&entry->metadata_tracker);
	return CREX_HASH_APPLY_KEEP;
}
/* }}} */

static void crxa_manifest_copy_ctor(zval *zv) /* {{{ */
{
	crxa_entry_info *info = emalloc(sizeof(crxa_entry_info));
	memcpy(info, C_PTR_P(zv), sizeof(crxa_entry_info));
	C_PTR_P(zv) = info;
}
/* }}} */

static void crxa_copy_cached_crxa(crxa_archive_data **pcrxa) /* {{{ */
{
	crxa_archive_data *crxa;
	HashTable newmanifest;
	char *fname;
	crxa_archive_object *objcrxa;

	crxa = (crxa_archive_data *) emalloc(sizeof(crxa_archive_data));
	*crxa = **pcrxa;
	crxa->is_persistent = 0;
	fname = crxa->fname;
	crxa->fname = estrndup(crxa->fname, crxa->fname_len);
	crxa->ext = crxa->fname + (crxa->ext - fname);

	if (crxa->alias) {
		crxa->alias = estrndup(crxa->alias, crxa->alias_len);
	}

	if (crxa->signature) {
		crxa->signature = estrdup(crxa->signature);
	}

	crxa_metadata_tracker_clone(&crxa->metadata_tracker);

	crex_hash_init(&newmanifest, sizeof(crxa_entry_info),
		crex_get_hash_value, destroy_crxa_manifest_entry, 0);
	crex_hash_copy(&newmanifest, &(*pcrxa)->manifest, crxa_manifest_copy_ctor);
	crex_hash_apply_with_argument(&newmanifest, crxa_update_cached_entry, (void *)crxa);
	crxa->manifest = newmanifest;
	crex_hash_init(&crxa->mounted_dirs, sizeof(char *),
		crex_get_hash_value, NULL, 0);
	crex_hash_init(&crxa->virtual_dirs, sizeof(char *),
		crex_get_hash_value, NULL, 0);
	crex_hash_copy(&crxa->virtual_dirs, &(*pcrxa)->virtual_dirs, NULL);
	*pcrxa = crxa;

	/* now, scan the list of persistent Crxa objects referencing this crxa and update the pointers */
	CREX_HASH_MAP_FOREACH_PTR(&CRXA_G(crxa_persist_map), objcrxa) {
		if (objcrxa->archive->fname_len == crxa->fname_len && !memcmp(objcrxa->archive->fname, crxa->fname, crxa->fname_len)) {
			objcrxa->archive = crxa;
		}
	} CREX_HASH_FOREACH_END();
}
/* }}} */

int crxa_copy_on_write(crxa_archive_data **pcrxa) /* {{{ */
{
	zval zv, *pzv;
	crxa_archive_data *newpcrxa;

	ZVAL_PTR(&zv, *pcrxa);
	if (NULL == (pzv = crex_hash_str_add(&(CRXA_G(crxa_fname_map)), (*pcrxa)->fname, (*pcrxa)->fname_len, &zv))) {
		return FAILURE;
	}

	crxa_copy_cached_crxa((crxa_archive_data **)&C_PTR_P(pzv));
	newpcrxa = C_PTR_P(pzv);
	/* invalidate crxa cache */
	CRXA_G(last_crxa) = NULL;
	CRXA_G(last_crxa_name) = CRXA_G(last_alias) = NULL;

	if (newpcrxa->alias_len && NULL == crex_hash_str_add_ptr(&(CRXA_G(crxa_alias_map)), newpcrxa->alias, newpcrxa->alias_len, newpcrxa)) {
		crex_hash_str_del(&(CRXA_G(crxa_fname_map)), (*pcrxa)->fname, (*pcrxa)->fname_len);
		return FAILURE;
	}

	*pcrxa = newpcrxa;
	return SUCCESS;
}
/* }}} */
