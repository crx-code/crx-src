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

#define CRXA_STREAM 1
#include "crxa_internal.h"
#include "stream.h"
#include "dirstream.h"

const crx_stream_ops crxa_ops = {
	crxa_stream_write, /* write */
	crxa_stream_read,  /* read */
	crxa_stream_close, /* close */
	crxa_stream_flush, /* flush */
	"crxa stream",
	crxa_stream_seek,  /* seek */
	NULL,              /* cast */
	crxa_stream_stat,  /* stat */
	NULL, /* set option */
};

const crx_stream_wrapper_ops crxa_stream_wops = {
	crxa_wrapper_open_url,
	NULL,                  /* crxa_wrapper_close */
	NULL,                  /* crxa_wrapper_stat, */
	crxa_wrapper_stat,     /* stat_url */
	crxa_wrapper_open_dir, /* opendir */
	"crxa",
	crxa_wrapper_unlink,   /* unlink */
	crxa_wrapper_rename,   /* rename */
	crxa_wrapper_mkdir,    /* create directory */
	crxa_wrapper_rmdir,    /* remove directory */
	NULL
};

const crx_stream_wrapper crx_stream_crxa_wrapper = {
	&crxa_stream_wops,
	NULL,
	0 /* is_url */
};

/**
 * Open a crxa file for streams API
 */
crx_url* crxa_parse_url(crx_stream_wrapper *wrapper, const char *filename, const char *mode, int options) /* {{{ */
{
	crx_url *resource;
	char *arch = NULL, *entry = NULL, *error;
	size_t arch_len, entry_len;

	if (strlen(filename) < 7 || strncasecmp(filename, "crxa://", 7)) {
		return NULL;
	}
	if (mode[0] == 'a') {
		if (!(options & CRX_STREAM_URL_STAT_QUIET)) {
			crx_stream_wrapper_log_error(wrapper, options, "crxa error: open mode append not supported");
		}
		return NULL;
	}
	if (crxa_split_fname(filename, strlen(filename), &arch, &arch_len, &entry, &entry_len, 2, (mode[0] == 'w' ? 2 : 0)) == FAILURE) {
		if (!(options & CRX_STREAM_URL_STAT_QUIET)) {
			if (arch && !entry) {
				crx_stream_wrapper_log_error(wrapper, options, "crxa error: no directory in \"%s\", must have at least crxa://%s/ for root directory (always use full path to a new crxa)", filename, arch);
				arch = NULL;
			} else {
				crx_stream_wrapper_log_error(wrapper, options, "crxa error: invalid url or non-existent crxa \"%s\"", filename);
			}
		}
		return NULL;
	}
	resource = ecalloc(1, sizeof(crx_url));
	resource->scheme = ZSTR_INIT_LITERAL("crxa", 0);
	resource->host = crex_string_init(arch, arch_len, 0);
	efree(arch);
	resource->path = crex_string_init(entry, entry_len, 0);
	efree(entry);

#ifdef MBO_0
		if (resource) {
			fprintf(stderr, "Alias:     %s\n", alias);
			fprintf(stderr, "Scheme:    %s\n", ZSTR_VAL(resource->scheme));
/*			fprintf(stderr, "User:      %s\n", resource->user);*/
/*			fprintf(stderr, "Pass:      %s\n", resource->pass ? "***" : NULL);*/
			fprintf(stderr, "Host:      %s\n", ZSTR_VAL(resource->host));
/*			fprintf(stderr, "Port:      %d\n", resource->port);*/
			fprintf(stderr, "Path:      %s\n", ZSTR_VAL(resource->path));
/*			fprintf(stderr, "Query:     %s\n", resource->query);*/
/*			fprintf(stderr, "Fragment:  %s\n", resource->fragment);*/
		}
#endif
	if (mode[0] == 'w' || (mode[0] == 'r' && mode[1] == '+')) {
		crxa_archive_data *pcrxa = NULL, *crxa;

		if (CRXA_G(request_init) && HT_IS_INITIALIZED(&CRXA_G(crxa_fname_map)) && NULL == (pcrxa = crex_hash_find_ptr(&(CRXA_G(crxa_fname_map)), resource->host))) {
			pcrxa = NULL;
		}
		if (CRXA_G(readonly) && (!pcrxa || !pcrxa->is_data)) {
			if (!(options & CRX_STREAM_URL_STAT_QUIET)) {
				crx_stream_wrapper_log_error(wrapper, options, "crxa error: write operations disabled by the crx.ini setting crxa.readonly");
			}
			crx_url_free(resource);
			return NULL;
		}
		if (crxa_open_or_create_filename(ZSTR_VAL(resource->host), ZSTR_LEN(resource->host), NULL, 0, 0, options, &crxa, &error) == FAILURE)
		{
			if (error) {
				if (!(options & CRX_STREAM_URL_STAT_QUIET)) {
					crx_stream_wrapper_log_error(wrapper, options, "%s", error);
				}
				efree(error);
			}
			crx_url_free(resource);
			return NULL;
		}
		if (crxa->is_persistent && FAILURE == crxa_copy_on_write(&crxa)) {
			if (error) {
				spprintf(&error, 0, "Cannot open cached crxa '%s' as writeable, copy on write failed", ZSTR_VAL(resource->host));
				if (!(options & CRX_STREAM_URL_STAT_QUIET)) {
					crx_stream_wrapper_log_error(wrapper, options, "%s", error);
				}
				efree(error);
			}
			crx_url_free(resource);
			return NULL;
		}
	} else {
		if (crxa_open_from_filename(ZSTR_VAL(resource->host), ZSTR_LEN(resource->host), NULL, 0, options, NULL, &error) == FAILURE)
		{
			if (error) {
				if (!(options & CRX_STREAM_URL_STAT_QUIET)) {
					crx_stream_wrapper_log_error(wrapper, options, "%s", error);
				}
				efree(error);
			}
			crx_url_free(resource);
			return NULL;
		}
	}
	return resource;
}
/* }}} */

/**
 * used for fopen('crxa://...') and company
 */
static crx_stream * crxa_wrapper_open_url(crx_stream_wrapper *wrapper, const char *path, const char *mode, int options, crex_string **opened_path, crx_stream_context *context STREAMS_DC) /* {{{ */
{
	crxa_archive_data *crxa;
	crxa_entry_data *idata;
	char *internal_file;
	char *error;
	HashTable *crxacontext;
	crx_url *resource = NULL;
	crx_stream *fpf;
	zval *pzoption, *metadata;
	uint32_t host_len;

	if ((resource = crxa_parse_url(wrapper, path, mode, options)) == NULL) {
		return NULL;
	}

	/* we must have at the very least crxa://alias.crxa/internalfile.crx */
	if (!resource->scheme || !resource->host || !resource->path) {
		crx_url_free(resource);
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: invalid url \"%s\"", path);
		return NULL;
	}

	if (!crex_string_equals_literal_ci(resource->scheme, "crxa")) {
		crx_url_free(resource);
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: not a crxa stream url \"%s\"", path);
		return NULL;
	}

	host_len = ZSTR_LEN(resource->host);
	crxa_request_initialize();

	/* strip leading "/" */
	internal_file = estrndup(ZSTR_VAL(resource->path) + 1, ZSTR_LEN(resource->path) - 1);
	if (mode[0] == 'w' || (mode[0] == 'r' && mode[1] == '+')) {
		if (NULL == (idata = crxa_get_or_create_entry_data(ZSTR_VAL(resource->host), host_len, internal_file, strlen(internal_file), mode, 0, &error, 1))) {
			if (error) {
				crx_stream_wrapper_log_error(wrapper, options, "%s", error);
				efree(error);
			} else {
				crx_stream_wrapper_log_error(wrapper, options, "crxa error: file \"%s\" could not be created in crxa \"%s\"", internal_file, ZSTR_VAL(resource->host));
			}
			efree(internal_file);
			crx_url_free(resource);
			return NULL;
		}
		if (error) {
			efree(error);
		}
		fpf = crx_stream_alloc(&crxa_ops, idata, NULL, mode);
		crx_url_free(resource);
		efree(internal_file);

		if (context && C_TYPE(context->options) != IS_UNDEF && (pzoption = crex_hash_str_find(HASH_OF(&context->options), "crxa", sizeof("crxa")-1)) != NULL) {
			crxacontext = HASH_OF(pzoption);
			if (idata->internal_file->uncompressed_filesize == 0
				&& idata->internal_file->compressed_filesize == 0
				&& (pzoption = crex_hash_str_find(crxacontext, "compress", sizeof("compress")-1)) != NULL
				&& C_TYPE_P(pzoption) == IS_LONG
				&& (C_LVAL_P(pzoption) & ~CRXA_ENT_COMPRESSION_MASK) == 0
			) {
				idata->internal_file->flags &= ~CRXA_ENT_COMPRESSION_MASK;
				idata->internal_file->flags |= C_LVAL_P(pzoption);
			}
			if ((pzoption = crex_hash_str_find(crxacontext, "metadata", sizeof("metadata")-1)) != NULL) {
				crxa_metadata_tracker_free(&idata->internal_file->metadata_tracker, idata->internal_file->is_persistent);

				metadata = pzoption;
				ZVAL_COPY_DEREF(&idata->internal_file->metadata_tracker.val, metadata);
				idata->crxa->is_modified = 1;
			}
		}
		if (opened_path) {
			*opened_path = strpprintf(MAXPATHLEN, "crxa://%s/%s", idata->crxa->fname, idata->internal_file->filename);
		}
		return fpf;
	} else {
		if (!*internal_file && (options & STREAM_OPEN_FOR_INCLUDE)) {
			/* retrieve the stub */
			if (FAILURE == crxa_get_archive(&crxa, ZSTR_VAL(resource->host), host_len, NULL, 0, NULL)) {
				crx_stream_wrapper_log_error(wrapper, options, "file %s is not a valid crxa archive", ZSTR_VAL(resource->host));
				efree(internal_file);
				crx_url_free(resource);
				return NULL;
			}
			if (crxa->is_tar || crxa->is_zip) {
				if ((FAILURE == crxa_get_entry_data(&idata, ZSTR_VAL(resource->host), host_len, ".crxa/stub.crx", sizeof(".crxa/stub.crx")-1, "r", 0, &error, 0)) || !idata) {
					goto idata_error;
				}
				efree(internal_file);
				if (opened_path) {
					*opened_path = strpprintf(MAXPATHLEN, "%s", crxa->fname);
				}
				crx_url_free(resource);
				goto crxa_stub;
			} else {
				crxa_entry_info *entry;

				entry = (crxa_entry_info *) ecalloc(1, sizeof(crxa_entry_info));
				entry->is_temp_dir = 1;
				entry->filename = estrndup("", 0);
				entry->filename_len = 0;
				entry->crxa = crxa;
				entry->offset = entry->offset_abs = 0;
				entry->compressed_filesize = entry->uncompressed_filesize = crxa->halt_offset;
				entry->is_crc_checked = 1;

				idata = (crxa_entry_data *) ecalloc(1, sizeof(crxa_entry_data));
				idata->fp = crxa_get_crxafp(crxa);
				idata->crxa = crxa;
				idata->internal_file = entry;
				if (!crxa->is_persistent) {
					++(entry->crxa->refcount);
				}
				++(entry->fp_refcount);
				crx_url_free(resource);
				if (opened_path) {
					*opened_path = strpprintf(MAXPATHLEN, "%s", crxa->fname);
				}
				efree(internal_file);
				goto crxa_stub;
			}
		}
		/* read-only access is allowed to magic files in .crxa directory */
		if ((FAILURE == crxa_get_entry_data(&idata, ZSTR_VAL(resource->host), host_len, internal_file, strlen(internal_file), "r", 0, &error, 0)) || !idata) {
idata_error:
			if (error) {
				crx_stream_wrapper_log_error(wrapper, options, "%s", error);
				efree(error);
			} else {
				crx_stream_wrapper_log_error(wrapper, options, "crxa error: \"%s\" is not a file in crxa \"%s\"", internal_file, ZSTR_VAL(resource->host));
			}
			efree(internal_file);
			crx_url_free(resource);
			return NULL;
		}
	}
	crx_url_free(resource);
#ifdef MBO_0
		fprintf(stderr, "Crxaname:   %s\n", idata->crxa->filename);
		fprintf(stderr, "Filename:   %s\n", internal_file);
		fprintf(stderr, "Entry:      %s\n", idata->internal_file->filename);
		fprintf(stderr, "Size:       %u\n", idata->internal_file->uncompressed_filesize);
		fprintf(stderr, "Compressed: %u\n", idata->internal_file->flags);
		fprintf(stderr, "Offset:     %u\n", idata->internal_file->offset_within_crxa);
		fprintf(stderr, "Cached:     %s\n", idata->internal_file->filedata ? "yes" : "no");
#endif

	/* check length, crc32 */
	if (!idata->internal_file->is_crc_checked && crxa_postprocess_file(idata, idata->internal_file->crc32, &error, 2) != SUCCESS) {
		crx_stream_wrapper_log_error(wrapper, options, "%s", error);
		efree(error);
		crxa_entry_delref(idata);
		efree(internal_file);
		return NULL;
	}

	if (!CRXA_G(cwd_init) && (options & STREAM_OPEN_FOR_INCLUDE)) {
		char *entry = idata->internal_file->filename, *cwd;

		CRXA_G(cwd_init) = 1;
		if ((idata->crxa->is_tar || idata->crxa->is_zip) && idata->internal_file->filename_len == sizeof(".crxa/stub.crx")-1 && !strncmp(idata->internal_file->filename, ".crxa/stub.crx", sizeof(".crxa/stub.crx")-1)) {
			/* we're executing the stub, which doesn't count as a file */
			CRXA_G(cwd_init) = 0;
		} else if ((cwd = strrchr(entry, '/'))) {
			CRXA_G(cwd_len) = cwd - entry;
			CRXA_G(cwd) = estrndup(entry, CRXA_G(cwd_len));
		} else {
			/* root directory */
			CRXA_G(cwd_len) = 0;
			CRXA_G(cwd) = NULL;
		}
	}
	if (opened_path) {
		*opened_path = strpprintf(MAXPATHLEN, "crxa://%s/%s", idata->crxa->fname, idata->internal_file->filename);
	}
	efree(internal_file);
crxa_stub:
	fpf = crx_stream_alloc(&crxa_ops, idata, NULL, mode);
	return fpf;
}
/* }}} */

/**
 * Used for fclose($fp) where $fp is a crxa archive
 */
static int crxa_stream_close(crx_stream *stream, int close_handle) /* {{{ */
{
	/* for some reasons crxa needs to be flushed even if there is no write going on */
	crxa_stream_flush(stream);

	crxa_entry_delref((crxa_entry_data *)stream->abstract);

	return 0;
}
/* }}} */

/**
 * used for fread($fp) and company on a fopen()ed crxa file handle
 */
static ssize_t crxa_stream_read(crx_stream *stream, char *buf, size_t count) /* {{{ */
{
	crxa_entry_data *data = (crxa_entry_data *)stream->abstract;
	size_t got;
	crxa_entry_info *entry;

	if (data->internal_file->link) {
		entry = crxa_get_link_source(data->internal_file);
	} else {
		entry = data->internal_file;
	}

	if (entry->is_deleted) {
		stream->eof = 1;
		return -1;
	}

	/* use our proxy position */
	crx_stream_seek(data->fp, data->position + data->zero, SEEK_SET);

	got = crx_stream_read(data->fp, buf, MIN(count, (size_t)(entry->uncompressed_filesize - data->position)));
	data->position = crx_stream_tell(data->fp) - data->zero;
	stream->eof = (data->position == (crex_off_t) entry->uncompressed_filesize);

	return got;
}
/* }}} */

/**
 * Used for fseek($fp) on a crxa file handle
 */
static int crxa_stream_seek(crx_stream *stream, crex_off_t offset, int whence, crex_off_t *newoffset) /* {{{ */
{
	crxa_entry_data *data = (crxa_entry_data *)stream->abstract;
	crxa_entry_info *entry;
	int res;
	crex_off_t temp;

	if (data->internal_file->link) {
		entry = crxa_get_link_source(data->internal_file);
	} else {
		entry = data->internal_file;
	}

	switch (whence) {
		case SEEK_END :
			temp = data->zero + entry->uncompressed_filesize + offset;
			break;
		case SEEK_CUR :
			temp = data->zero + data->position + offset;
			break;
		case SEEK_SET :
			temp = data->zero + offset;
			break;
		default:
			temp = 0;
	}
	if (temp > data->zero + (crex_off_t) entry->uncompressed_filesize) {
		*newoffset = -1;
		return -1;
	}
	if (temp < data->zero) {
		*newoffset = -1;
		return -1;
	}
	res = crx_stream_seek(data->fp, temp, SEEK_SET);
	*newoffset = crx_stream_tell(data->fp) - data->zero;
	data->position = *newoffset;
	return res;
}
/* }}} */

/**
 * Used for writing to a crxa file
 */
static ssize_t crxa_stream_write(crx_stream *stream, const char *buf, size_t count) /* {{{ */
{
	crxa_entry_data *data = (crxa_entry_data *) stream->abstract;

	crx_stream_seek(data->fp, data->position, SEEK_SET);
	if (count != crx_stream_write(data->fp, buf, count)) {
		crx_stream_wrapper_log_error(stream->wrapper, stream->flags, "crxa error: Could not write %d characters to \"%s\" in crxa \"%s\"", (int) count, data->internal_file->filename, data->crxa->fname);
		return -1;
	}
	data->position = crx_stream_tell(data->fp);
	if (data->position > (crex_off_t)data->internal_file->uncompressed_filesize) {
		data->internal_file->uncompressed_filesize = data->position;
	}
	data->internal_file->compressed_filesize = data->internal_file->uncompressed_filesize;
	data->internal_file->old_flags = data->internal_file->flags;
	data->internal_file->is_modified = 1;
	return count;
}
/* }}} */

/**
 * Used to save work done on a writeable crxa
 */
static int crxa_stream_flush(crx_stream *stream) /* {{{ */
{
	char *error;
	int ret;
	crxa_entry_data *data = (crxa_entry_data *) stream->abstract;

	if (data->internal_file->is_modified) {
		data->internal_file->timestamp = time(0);
		ret = crxa_flush(data->crxa, 0, 0, 0, &error);
		if (error) {
			crx_stream_wrapper_log_error(stream->wrapper, REPORT_ERRORS, "%s", error);
			efree(error);
		}
		return ret;
	} else {
		return EOF;
	}
}
/* }}} */

 /* {{{ crxa_dostat */
/**
 * stat an opened crxa file handle stream, used by crxa_stat()
 */
void crxa_dostat(crxa_archive_data *crxa, crxa_entry_info *data, crx_stream_statbuf *ssb, bool is_temp_dir)
{
	memset(ssb, 0, sizeof(crx_stream_statbuf));

	if (!is_temp_dir && !data->is_dir) {
		ssb->sb.st_size = data->uncompressed_filesize;
		ssb->sb.st_mode = data->flags & CRXA_ENT_PERM_MASK;
		ssb->sb.st_mode |= S_IFREG; /* regular file */
		/* timestamp is just the timestamp when this was added to the crxa */
		ssb->sb.st_mtime = data->timestamp;
		ssb->sb.st_atime = data->timestamp;
		ssb->sb.st_ctime = data->timestamp;
	} else if (!is_temp_dir && data->is_dir) {
		ssb->sb.st_size = 0;
		ssb->sb.st_mode = data->flags & CRXA_ENT_PERM_MASK;
		ssb->sb.st_mode |= S_IFDIR; /* regular directory */
		/* timestamp is just the timestamp when this was added to the crxa */
		ssb->sb.st_mtime = data->timestamp;
		ssb->sb.st_atime = data->timestamp;
		ssb->sb.st_ctime = data->timestamp;
	} else {
		ssb->sb.st_size = 0;
		ssb->sb.st_mode = 0777;
		ssb->sb.st_mode |= S_IFDIR; /* regular directory */
		ssb->sb.st_mtime = crxa->max_timestamp;
		ssb->sb.st_atime = crxa->max_timestamp;
		ssb->sb.st_ctime = crxa->max_timestamp;
	}
	if (!crxa->is_writeable) {
		ssb->sb.st_mode = (ssb->sb.st_mode & 0555) | (ssb->sb.st_mode & ~0777);
	}

	ssb->sb.st_nlink = 1;
	ssb->sb.st_rdev = -1;
	/* this is only for APC, so use /dev/null device - no chance of conflict there! */
	ssb->sb.st_dev = 0xc;
	/* generate unique inode number for alias/filename, so no crxas will conflict */
	if (!is_temp_dir) {
		ssb->sb.st_ino = data->inode;
	}
#ifndef CRX_WIN32
	ssb->sb.st_blksize = -1;
	ssb->sb.st_blocks = -1;
#endif
}
/* }}}*/

/**
 * Stat an opened crxa file handle
 */
static int crxa_stream_stat(crx_stream *stream, crx_stream_statbuf *ssb) /* {{{ */
{
	crxa_entry_data *data = (crxa_entry_data *)stream->abstract;

	/* If ssb is NULL then someone is misbehaving */
	if (!ssb) {
		return -1;
	}

	crxa_dostat(data->crxa, data->internal_file, ssb, 0);
	return 0;
}
/* }}} */

/**
 * Stream wrapper stat implementation of stat()
 */
static int crxa_wrapper_stat(crx_stream_wrapper *wrapper, const char *url, int flags,
				  crx_stream_statbuf *ssb, crx_stream_context *context) /* {{{ */
{
	crx_url *resource = NULL;
	char *internal_file, *error;
	crxa_archive_data *crxa;
	crxa_entry_info *entry;
	uint32_t host_len;
	size_t internal_file_len;

	if ((resource = crxa_parse_url(wrapper, url, "r", flags|CRX_STREAM_URL_STAT_QUIET)) == NULL) {
		return FAILURE;
	}

	/* we must have at the very least crxa://alias.crxa/internalfile.crx */
	if (!resource->scheme || !resource->host || !resource->path) {
		crx_url_free(resource);
		return FAILURE;
	}

	if (!crex_string_equals_literal_ci(resource->scheme, "crxa")) {
		crx_url_free(resource);
		return FAILURE;
	}

	host_len = ZSTR_LEN(resource->host);
	crxa_request_initialize();

	internal_file = ZSTR_VAL(resource->path) + 1; /* strip leading "/" */
	/* find the crxa in our trusty global hash indexed by alias (host of crxa://blah.crxa/file.whatever) */
	if (FAILURE == crxa_get_archive(&crxa, ZSTR_VAL(resource->host), host_len, NULL, 0, &error)) {
		crx_url_free(resource);
		if (error) {
			efree(error);
		}
		return FAILURE;
	}
	if (error) {
		efree(error);
	}
	if (*internal_file == '\0') {
		/* root directory requested */
		crxa_dostat(crxa, NULL, ssb, 1);
		crx_url_free(resource);
		return SUCCESS;
	}
	if (!HT_IS_INITIALIZED(&crxa->manifest)) {
		crx_url_free(resource);
		return FAILURE;
	}
	internal_file_len = strlen(internal_file);
	/* search through the manifest of files, and if we have an exact match, it's a file */
	if (NULL != (entry = crex_hash_str_find_ptr(&crxa->manifest, internal_file, internal_file_len))) {
		crxa_dostat(crxa, entry, ssb, 0);
		crx_url_free(resource);
		return SUCCESS;
	}
	if (crex_hash_str_exists(&(crxa->virtual_dirs), internal_file, internal_file_len)) {
		crxa_dostat(crxa, NULL, ssb, 1);
		crx_url_free(resource);
		return SUCCESS;
	}
	/* check for mounted directories */
	if (HT_IS_INITIALIZED(&crxa->mounted_dirs) && crex_hash_num_elements(&crxa->mounted_dirs)) {
		crex_string *str_key;

		CREX_HASH_MAP_FOREACH_STR_KEY(&crxa->mounted_dirs, str_key) {
			if (ZSTR_LEN(str_key) >= internal_file_len || strncmp(ZSTR_VAL(str_key), internal_file, ZSTR_LEN(str_key))) {
				continue;
			} else {
				char *test;
				size_t test_len;
				crx_stream_statbuf ssbi;

				if (NULL == (entry = crex_hash_find_ptr(&crxa->manifest, str_key))) {
					goto free_resource;
				}
				if (!entry->tmp || !entry->is_mounted) {
					goto free_resource;
				}
				test_len = spprintf(&test, MAXPATHLEN, "%s%s", entry->tmp, internal_file + ZSTR_LEN(str_key));
				if (SUCCESS != crx_stream_stat_path(test, &ssbi)) {
					efree(test);
					continue;
				}
				/* mount the file/directory just in time */
				if (SUCCESS != crxa_mount_entry(crxa, test, test_len, internal_file, internal_file_len)) {
					efree(test);
					goto free_resource;
				}
				efree(test);
				if (NULL == (entry = crex_hash_str_find_ptr(&crxa->manifest, internal_file, internal_file_len))) {
					goto free_resource;
				}
				crxa_dostat(crxa, entry, ssb, 0);
				crx_url_free(resource);
				return SUCCESS;
			}
		} CREX_HASH_FOREACH_END();
	}
free_resource:
	crx_url_free(resource);
	return FAILURE;
}
/* }}} */

/**
 * Unlink a file within a crxa archive
 */
static int crxa_wrapper_unlink(crx_stream_wrapper *wrapper, const char *url, int options, crx_stream_context *context) /* {{{ */
{
	crx_url *resource;
	char *internal_file, *error;
	int internal_file_len;
	crxa_entry_data *idata;
	crxa_archive_data *pcrxa;
	uint32_t host_len;

	if ((resource = crxa_parse_url(wrapper, url, "rb", options)) == NULL) {
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: unlink failed");
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
	crxa_request_initialize();

	pcrxa = crex_hash_find_ptr(&(CRXA_G(crxa_fname_map)), resource->host);
	if (CRXA_G(readonly) && (!pcrxa || !pcrxa->is_data)) {
		crx_url_free(resource);
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: write operations disabled by the crx.ini setting crxa.readonly");
		return 0;
	}

	/* need to copy to strip leading "/", will get touched again */
	internal_file = estrndup(ZSTR_VAL(resource->path) + 1, ZSTR_LEN(resource->path) - 1);
	internal_file_len = ZSTR_LEN(resource->path) - 1;
	if (FAILURE == crxa_get_entry_data(&idata, ZSTR_VAL(resource->host), host_len, internal_file, internal_file_len, "r", 0, &error, 1)) {
		/* constraints of fp refcount were not met */
		if (error) {
			crx_stream_wrapper_log_error(wrapper, options, "unlink of \"%s\" failed: %s", url, error);
			efree(error);
		} else {
			crx_stream_wrapper_log_error(wrapper, options, "unlink of \"%s\" failed, file does not exist", url);
		}
		efree(internal_file);
		crx_url_free(resource);
		return 0;
	}
	if (error) {
		efree(error);
	}
	if (idata->internal_file->fp_refcount > 1) {
		/* more than just our fp resource is open for this file */
		crx_stream_wrapper_log_error(wrapper, options, "crxa error: \"%s\" in crxa \"%s\", has open file pointers, cannot unlink", internal_file, ZSTR_VAL(resource->host));
		efree(internal_file);
		crx_url_free(resource);
		crxa_entry_delref(idata);
		return 0;
	}
	crx_url_free(resource);
	efree(internal_file);
	crxa_entry_remove(idata, &error);
	if (error) {
		crx_stream_wrapper_log_error(wrapper, options, "%s", error);
		efree(error);
	}
	return 1;
}
/* }}} */

static int crxa_wrapper_rename(crx_stream_wrapper *wrapper, const char *url_from, const char *url_to, int options, crx_stream_context *context) /* {{{ */
{
	crx_url *resource_from, *resource_to;
	char *error;
	crxa_archive_data *crxa, *pfrom, *pto;
	crxa_entry_info *entry;
	uint32_t host_len;
	int is_dir = 0;
	int is_modified = 0;

	error = NULL;

	if ((resource_from = crxa_parse_url(wrapper, url_from, "wb", options|CRX_STREAM_URL_STAT_QUIET)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "crxa error: cannot rename \"%s\" to \"%s\": invalid or non-writable url \"%s\"", url_from, url_to, url_from);
		return 0;
	}
	if (SUCCESS != crxa_get_archive(&pfrom, ZSTR_VAL(resource_from->host), ZSTR_LEN(resource_from->host), NULL, 0, &error)) {
		pfrom = NULL;
		if (error) {
			efree(error);
		}
	}
	if (CRXA_G(readonly) && (!pfrom || !pfrom->is_data)) {
		crx_url_free(resource_from);
		crx_error_docref(NULL, E_WARNING, "crxa error: Write operations disabled by the crx.ini setting crxa.readonly");
		return 0;
	}

	if ((resource_to = crxa_parse_url(wrapper, url_to, "wb", options|CRX_STREAM_URL_STAT_QUIET)) == NULL) {
		crx_url_free(resource_from);
		crx_error_docref(NULL, E_WARNING, "crxa error: cannot rename \"%s\" to \"%s\": invalid or non-writable url \"%s\"", url_from, url_to, url_to);
		return 0;
	}
	if (SUCCESS != crxa_get_archive(&pto, ZSTR_VAL(resource_to->host), ZSTR_LEN(resource_to->host), NULL, 0, &error)) {
		if (error) {
			efree(error);
		}
		pto = NULL;
	}
	if (CRXA_G(readonly) && (!pto || !pto->is_data)) {
		crx_url_free(resource_from);
		crx_error_docref(NULL, E_WARNING, "crxa error: Write operations disabled by the crx.ini setting crxa.readonly");
		return 0;
	}

	if (!crex_string_equals(resource_from->host, resource_to->host)) {
		crx_url_free(resource_from);
		crx_url_free(resource_to);
		crx_error_docref(NULL, E_WARNING, "crxa error: cannot rename \"%s\" to \"%s\", not within the same crxa archive", url_from, url_to);
		return 0;
	}

	/* we must have at the very least crxa://alias.crxa/internalfile.crx */
	if (!resource_from->scheme || !resource_from->host || !resource_from->path) {
		crx_url_free(resource_from);
		crx_url_free(resource_to);
		crx_error_docref(NULL, E_WARNING, "crxa error: cannot rename \"%s\" to \"%s\": invalid url \"%s\"", url_from, url_to, url_from);
		return 0;
	}

	if (!resource_to->scheme || !resource_to->host || !resource_to->path) {
		crx_url_free(resource_from);
		crx_url_free(resource_to);
		crx_error_docref(NULL, E_WARNING, "crxa error: cannot rename \"%s\" to \"%s\": invalid url \"%s\"", url_from, url_to, url_to);
		return 0;
	}

	if (!crex_string_equals_literal_ci(resource_from->scheme, "crxa")) {
		crx_url_free(resource_from);
		crx_url_free(resource_to);
		crx_error_docref(NULL, E_WARNING, "crxa error: cannot rename \"%s\" to \"%s\": not a crxa stream url \"%s\"", url_from, url_to, url_from);
		return 0;
	}

	if (!crex_string_equals_literal_ci(resource_to->scheme, "crxa")) {
		crx_url_free(resource_from);
		crx_url_free(resource_to);
		crx_error_docref(NULL, E_WARNING, "crxa error: cannot rename \"%s\" to \"%s\": not a crxa stream url \"%s\"", url_from, url_to, url_to);
		return 0;
	}

	host_len = ZSTR_LEN(resource_from->host);

	if (SUCCESS != crxa_get_archive(&crxa, ZSTR_VAL(resource_from->host), host_len, NULL, 0, &error)) {
		crx_url_free(resource_from);
		crx_url_free(resource_to);
		crx_error_docref(NULL, E_WARNING, "crxa error: cannot rename \"%s\" to \"%s\": %s", url_from, url_to, error);
		efree(error);
		return 0;
	}

	if (crxa->is_persistent && FAILURE == crxa_copy_on_write(&crxa)) {
		crx_url_free(resource_from);
		crx_url_free(resource_to);
		crx_error_docref(NULL, E_WARNING, "crxa error: cannot rename \"%s\" to \"%s\": could not make cached crxa writeable", url_from, url_to);
		return 0;
	}

	if (NULL != (entry = crex_hash_str_find_ptr(&(crxa->manifest), ZSTR_VAL(resource_from->path)+1, ZSTR_LEN(resource_from->path)-1))) {
		crxa_entry_info new, *source;

		/* perform rename magic */
		if (entry->is_deleted) {
			crx_url_free(resource_from);
			crx_url_free(resource_to);
			crx_error_docref(NULL, E_WARNING, "crxa error: cannot rename \"%s\" to \"%s\" from extracted crxa archive, source has been deleted", url_from, url_to);
			return 0;
		}
		/* transfer all data over to the new entry */
		memcpy((void *) &new, (void *) entry, sizeof(crxa_entry_info));
		/* mark the old one for deletion */
		entry->is_deleted = 1;
		entry->fp = NULL;
		ZVAL_UNDEF(&entry->metadata_tracker.val);
		entry->link = entry->tmp = NULL;
		source = entry;

		/* add to the manifest, and then store the pointer to the new guy in entry */
		entry = crex_hash_str_add_mem(&(crxa->manifest), ZSTR_VAL(resource_to->path)+1, ZSTR_LEN(resource_to->path)-1, (void **)&new, sizeof(crxa_entry_info));

		entry->filename = estrndup(ZSTR_VAL(resource_to->path)+1, ZSTR_LEN(resource_to->path)-1);
		if (FAILURE == crxa_copy_entry_fp(source, entry, &error)) {
			crx_url_free(resource_from);
			crx_url_free(resource_to);
			crx_error_docref(NULL, E_WARNING, "crxa error: cannot rename \"%s\" to \"%s\": %s", url_from, url_to, error);
			efree(error);
			crex_hash_str_del(&(crxa->manifest), entry->filename, strlen(entry->filename));
			return 0;
		}
		is_modified = 1;
		entry->is_modified = 1;
		entry->filename_len = strlen(entry->filename);
		is_dir = entry->is_dir;
	} else {
		is_dir = crex_hash_str_exists(&(crxa->virtual_dirs), ZSTR_VAL(resource_from->path)+1, ZSTR_LEN(resource_from->path)-1);
		if (!is_dir) {
			/* file does not exist */
			crx_url_free(resource_from);
			crx_url_free(resource_to);
			crx_error_docref(NULL, E_WARNING, "crxa error: cannot rename \"%s\" to \"%s\" from extracted crxa archive, source does not exist", url_from, url_to);
			return 0;

		}
	}

	/* Rename directory. Update all nested paths */
	if (is_dir) {
		Bucket *b;
		crex_string *str_key;
		crex_string *new_str_key;
		uint32_t from_len = ZSTR_LEN(resource_from->path) - 1;
		uint32_t to_len = ZSTR_LEN(resource_to->path) - 1;

		CREX_HASH_MAP_FOREACH_BUCKET(&crxa->manifest, b) {
			str_key = b->key;
			entry = C_PTR(b->val);
			if (!entry->is_deleted &&
				ZSTR_LEN(str_key) > from_len &&
				memcmp(ZSTR_VAL(str_key), ZSTR_VAL(resource_from->path)+1, from_len) == 0 &&
				IS_SLASH(ZSTR_VAL(str_key)[from_len])) {

				new_str_key = crex_string_alloc(ZSTR_LEN(str_key) + to_len - from_len, 0);
				memcpy(ZSTR_VAL(new_str_key), ZSTR_VAL(resource_to->path) + 1, to_len);
				memcpy(ZSTR_VAL(new_str_key) + to_len, ZSTR_VAL(str_key) + from_len, ZSTR_LEN(str_key) - from_len);
				ZSTR_VAL(new_str_key)[ZSTR_LEN(new_str_key)] = 0;

				is_modified = 1;
				entry->is_modified = 1;
				efree(entry->filename);
				// TODO: avoid reallocation (make entry->filename crex_string*)
				entry->filename = estrndup(ZSTR_VAL(new_str_key), ZSTR_LEN(new_str_key));
				entry->filename_len = ZSTR_LEN(new_str_key);

				crex_string_release_ex(str_key, 0);
				b->h = crex_string_hash_val(new_str_key);
				b->key = new_str_key;
			}
		} CREX_HASH_FOREACH_END();
		crex_hash_rehash(&crxa->manifest);

		CREX_HASH_MAP_FOREACH_BUCKET(&crxa->virtual_dirs, b) {
			str_key = b->key;
			if (crex_string_starts_with_cstr(str_key, ZSTR_VAL(resource_from->path)+1, from_len) &&
				(ZSTR_LEN(str_key) == from_len || IS_SLASH(ZSTR_VAL(str_key)[from_len]))) {

				new_str_key = crex_string_alloc(ZSTR_LEN(str_key) + to_len - from_len, 0);
				memcpy(ZSTR_VAL(new_str_key), ZSTR_VAL(resource_to->path) + 1, to_len);
				memcpy(ZSTR_VAL(new_str_key) + to_len, ZSTR_VAL(str_key) + from_len, ZSTR_LEN(str_key) - from_len);
				ZSTR_VAL(new_str_key)[ZSTR_LEN(new_str_key)] = 0;

				crex_string_release_ex(str_key, 0);
				b->h = crex_string_hash_val(new_str_key);
				b->key = new_str_key;
			}
		} CREX_HASH_FOREACH_END();
		crex_hash_rehash(&crxa->virtual_dirs);

		CREX_HASH_MAP_FOREACH_BUCKET(&crxa->mounted_dirs, b) {
			str_key = b->key;
			if (crex_string_starts_with_cstr(str_key, ZSTR_VAL(resource_from->path)+1, from_len) &&
				(ZSTR_LEN(str_key) == from_len || IS_SLASH(ZSTR_VAL(str_key)[from_len]))) {

				new_str_key = crex_string_alloc(ZSTR_LEN(str_key) + to_len - from_len, 0);
				memcpy(ZSTR_VAL(new_str_key), ZSTR_VAL(resource_to->path) + 1, to_len);
				memcpy(ZSTR_VAL(new_str_key) + to_len, ZSTR_VAL(str_key) + from_len, ZSTR_LEN(str_key) - from_len);
				ZSTR_VAL(new_str_key)[ZSTR_LEN(new_str_key)] = 0;

				crex_string_release_ex(str_key, 0);
				b->h = crex_string_hash_val(new_str_key);
				b->key = new_str_key;
			}
		} CREX_HASH_FOREACH_END();
		crex_hash_rehash(&crxa->mounted_dirs);
	}

	if (is_modified) {
		crxa_flush(crxa, 0, 0, 0, &error);
		if (error) {
			crx_url_free(resource_from);
			crx_url_free(resource_to);
			crx_error_docref(NULL, E_WARNING, "crxa error: cannot rename \"%s\" to \"%s\": %s", url_from, url_to, error);
			efree(error);
			return 0;
		}
	}

	crx_url_free(resource_from);
	crx_url_free(resource_to);

	return 1;
}
/* }}} */
