/*
  +----------------------------------------------------------------------+
  | TAR archive support for Crxa                                         |
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
  | Authors: Dmitry Stogov <dmitry@crx.net>                              |
  |          Gregory Beaver <cellog@crx.net>                             |
  +----------------------------------------------------------------------+
*/

#include "crxa_internal.h"

static uint32_t crxa_tar_number(char *buf, size_t len) /* {{{ */
{
	uint32_t num = 0;
	size_t i = 0;

	while (i < len && buf[i] == ' ') {
		++i;
	}

	while (i < len && buf[i] >= '0' && buf[i] <= '7') {
		num = num * 8 + (buf[i] - '0');
		++i;
	}

	return num;
}
/* }}} */

/* adapted from format_octal() in libarchive
 *
 * Copyright (c) 2003-2009 Tim Kientzle
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
static int crxa_tar_octal(char *buf, uint32_t val, int len) /* {{{ */
{
	char *p = buf;
	int s = len;

	p += len;		/* Start at the end and work backwards. */
	while (s-- > 0) {
		*--p = (char)('0' + (val & 7));
		val >>= 3;
	}

	if (val == 0)
		return SUCCESS;

	/* If it overflowed, fill field with max value. */
	while (len-- > 0)
		*p++ = '7';

	return FAILURE;
}
/* }}} */

static uint32_t crxa_tar_checksum(char *buf, size_t len) /* {{{ */
{
	uint32_t sum = 0;
	char *end = buf + len;

	while (buf != end) {
		sum += (unsigned char)*buf;
		++buf;
	}
	return sum;
}
/* }}} */

int crxa_is_tar(char *buf, char *fname) /* {{{ */
{
	tar_header *header = (tar_header *) buf;
	uint32_t checksum = crxa_tar_number(header->checksum, sizeof(header->checksum));
	uint32_t ret;
	char save[sizeof(header->checksum)], *bname;

	/* assume that the first filename in a tar won't begin with <?crx */
	if (!strncmp(buf, "<?crx", sizeof("<?crx")-1)) {
		return 0;
	}

	memcpy(save, header->checksum, sizeof(header->checksum));
	memset(header->checksum, ' ', sizeof(header->checksum));
	ret = (checksum == crxa_tar_checksum(buf, 512));
	memcpy(header->checksum, save, sizeof(header->checksum));
	if ((bname = strrchr(fname, CRX_DIR_SEPARATOR))) {
		fname = bname;
	}
	if (!ret && (bname = strstr(fname, ".tar")) && (bname[4] == '\0' || bname[4] == '.')) {
		/* probably a corrupted tar - so we will pretend it is one */
		return 1;
	}
	return ret;
}
/* }}} */

int crxa_open_or_create_tar(char *fname, size_t fname_len, char *alias, size_t alias_len, int is_data, uint32_t options, crxa_archive_data** pcrxa, char **error) /* {{{ */
{
	crxa_archive_data *crxa;
	int ret = crxa_create_or_parse_filename(fname, fname_len, alias, alias_len, is_data, options, &crxa, error);

	if (FAILURE == ret) {
		return FAILURE;
	}

	if (pcrxa) {
		*pcrxa = crxa;
	}

	crxa->is_data = is_data;

	if (crxa->is_tar) {
		return ret;
	}

	if (crxa->is_brandnew) {
		crxa->is_tar = 1;
		crxa->is_zip = 0;
		crxa->internal_file_start = 0;
		return SUCCESS;
	}

	/* we've reached here - the crxa exists and is a regular crxa */
	if (error) {
		spprintf(error, 4096, "crxa tar error: \"%s\" already exists as a regular crxa and must be deleted from disk prior to creating as a tar-based crxa", fname);
	}
	return FAILURE;
}
/* }}} */

static int crxa_tar_process_metadata(crxa_entry_info *entry, crx_stream *fp) /* {{{ */
{
	char *metadata;
	size_t save = crx_stream_tell(fp), read;
	crxa_entry_info *mentry;

	metadata = (char *) safe_emalloc(1, entry->uncompressed_filesize, 1);

	read = crx_stream_read(fp, metadata, entry->uncompressed_filesize);
	if (read != entry->uncompressed_filesize) {
		efree(metadata);
		crx_stream_seek(fp, save, SEEK_SET);
		return FAILURE;
	}

	crxa_parse_metadata_lazy(metadata, &entry->metadata_tracker, entry->uncompressed_filesize, entry->is_persistent);

	if (entry->filename_len == sizeof(".crxa/.metadata.bin")-1 && !memcmp(entry->filename, ".crxa/.metadata.bin", sizeof(".crxa/.metadata.bin")-1)) {
		if (crxa_metadata_tracker_has_data(&entry->crxa->metadata_tracker, entry->crxa->is_persistent)) {
			efree(metadata);
			return FAILURE;
		}
		entry->crxa->metadata_tracker = entry->metadata_tracker;
		entry->metadata_tracker.str = NULL;
		ZVAL_UNDEF(&entry->metadata_tracker.val);
	} else if (entry->filename_len >= sizeof(".crxa/.metadata/") + sizeof("/.metadata.bin") - 1 && NULL != (mentry = crex_hash_str_find_ptr(&(entry->crxa->manifest), entry->filename + sizeof(".crxa/.metadata/") - 1, entry->filename_len - (sizeof("/.metadata.bin") - 1 + sizeof(".crxa/.metadata/") - 1)))) {
		if (crxa_metadata_tracker_has_data(&mentry->metadata_tracker, mentry->is_persistent)) {
			efree(metadata);
			return FAILURE;
		}
		/* transfer this metadata to the entry it refers */
		mentry->metadata_tracker = entry->metadata_tracker;
		entry->metadata_tracker.str = NULL;
		ZVAL_UNDEF(&entry->metadata_tracker.val);
	}

	efree(metadata);
	crx_stream_seek(fp, save, SEEK_SET);
	return SUCCESS;
}
/* }}} */

int crxa_parse_tarfile(crx_stream* fp, char *fname, size_t fname_len, char *alias, size_t alias_len, crxa_archive_data** pcrxa, int is_data, uint32_t compression, char **error) /* {{{ */
{
	char buf[512], *actual_alias = NULL, *p;
	crxa_entry_info entry = {0};
	size_t pos = 0, read, totalsize;
	tar_header *hdr;
	uint32_t sum1, sum2, size, old;
	crxa_archive_data *mycrxa, *actual;
	int last_was_longlink = 0;
	size_t linkname_len;

	if (error) {
		*error = NULL;
	}

	crx_stream_seek(fp, 0, SEEK_END);
	totalsize = crx_stream_tell(fp);
	crx_stream_seek(fp, 0, SEEK_SET);
	read = crx_stream_read(fp, buf, sizeof(buf));

	if (read != sizeof(buf)) {
		if (error) {
			spprintf(error, 4096, "crxa error: \"%s\" is not a tar file or is truncated", fname);
		}
		crx_stream_close(fp);
		return FAILURE;
	}

	hdr = (tar_header*)buf;
	old = (memcmp(hdr->magic, "ustar", sizeof("ustar")-1) != 0);

	mycrxa = (crxa_archive_data *) pecalloc(1, sizeof(crxa_archive_data), CRXA_G(persist));
	mycrxa->is_persistent = CRXA_G(persist);
	/* estimate number of entries, can't be certain with tar files */
	crex_hash_init(&mycrxa->manifest, 2 + (totalsize >> 12),
		crex_get_hash_value, destroy_crxa_manifest_entry, (bool)mycrxa->is_persistent);
	crex_hash_init(&mycrxa->mounted_dirs, 5,
		crex_get_hash_value, NULL, (bool)mycrxa->is_persistent);
	crex_hash_init(&mycrxa->virtual_dirs, 4 + (totalsize >> 11),
		crex_get_hash_value, NULL, (bool)mycrxa->is_persistent);
	mycrxa->is_tar = 1;
	/* remember whether this entire crxa was compressed with gz/bzip2 */
	mycrxa->flags = compression;

	entry.is_tar = 1;
	entry.is_crc_checked = 1;
	entry.crxa = mycrxa;
	pos += sizeof(buf);

	do {
		crxa_entry_info *newentry;

		pos = crx_stream_tell(fp);
		hdr = (tar_header*) buf;
		sum1 = crxa_tar_number(hdr->checksum, sizeof(hdr->checksum));
		if (sum1 == 0 && crxa_tar_checksum(buf, sizeof(buf)) == 0) {
			break;
		}
		memset(hdr->checksum, ' ', sizeof(hdr->checksum));
		sum2 = crxa_tar_checksum(buf, old?sizeof(old_tar_header):sizeof(tar_header));

		if (old && sum2 != sum1) {
			uint32_t sum3 = crxa_tar_checksum(buf, sizeof(tar_header));
			if (sum3 == sum1) {
				/* apparently a broken tar which is in ustar format w/o setting the ustar marker */
				sum2 = sum3;
				old = 0;
			}
		}

		size = entry.uncompressed_filesize = entry.compressed_filesize =
			crxa_tar_number(hdr->size, sizeof(hdr->size));

		/* skip global/file headers (pax) */
		if (!old && (hdr->typeflag == TAR_GLOBAL_HDR || hdr->typeflag == TAR_FILE_HDR)) {
			size = (size+511)&~511;
			goto next;
		}

		if (((!old && hdr->prefix[0] == 0) || old) && crex_strnlen(hdr->name, 100) == sizeof(".crxa/signature.bin")-1 && !strncmp(hdr->name, ".crxa/signature.bin", sizeof(".crxa/signature.bin")-1)) {
			crex_off_t curloc;
			size_t sig_len;

			if (size > 511) {
				if (error) {
					spprintf(error, 4096, "crxa error: tar-based crxa \"%s\" has signature that is larger than 511 bytes, cannot process", fname);
				}
bail:
				crx_stream_close(fp);
				crxa_destroy_crxa_data(mycrxa);
				return FAILURE;
			}
			curloc = crx_stream_tell(fp);
			read = crx_stream_read(fp, buf, size);
			if (read != size || read <= 8) {
				if (error) {
					spprintf(error, 4096, "crxa error: tar-based crxa \"%s\" signature cannot be read", fname);
				}
				goto bail;
			}
#ifdef WORDS_BIGENDIAN
# define CRXA_GET_32(buffer) \
	(((((unsigned char*)(buffer))[3]) << 24) \
		| ((((unsigned char*)(buffer))[2]) << 16) \
		| ((((unsigned char*)(buffer))[1]) <<  8) \
		| (((unsigned char*)(buffer))[0]))
#else
# define CRXA_GET_32(buffer) (uint32_t) *(buffer)
#endif
			mycrxa->sig_flags = CRXA_GET_32(buf);
			if (FAILURE == crxa_verify_signature(fp, crx_stream_tell(fp) - size - 512, mycrxa->sig_flags, buf + 8, size - 8, fname, &mycrxa->signature, &sig_len, error)) {
				if (error) {
					char *save = *error;
					spprintf(error, 4096, "crxa error: tar-based crxa \"%s\" signature cannot be verified: %s", fname, save);
					efree(save);
				}
				goto bail;
			}
			mycrxa->sig_len = sig_len;
			crx_stream_seek(fp, curloc + 512, SEEK_SET);
			/* signature checked out, let's ensure this is the last file in the crxa */
			if (((hdr->typeflag == '\0') || (hdr->typeflag == TAR_FILE)) && size > 0) {
				/* this is not good enough - seek succeeds even on truncated tars */
				crx_stream_seek(fp, 512, SEEK_CUR);
				if ((uint32_t)crx_stream_tell(fp) > totalsize) {
					if (error) {
						spprintf(error, 4096, "crxa error: \"%s\" is a corrupted tar file (truncated)", fname);
					}
					crx_stream_close(fp);
					crxa_destroy_crxa_data(mycrxa);
					return FAILURE;
				}
			}

			read = crx_stream_read(fp, buf, sizeof(buf));

			if (read != sizeof(buf)) {
				if (error) {
					spprintf(error, 4096, "crxa error: \"%s\" is a corrupted tar file (truncated)", fname);
				}
				crx_stream_close(fp);
				crxa_destroy_crxa_data(mycrxa);
				return FAILURE;
			}

			hdr = (tar_header*) buf;
			sum1 = crxa_tar_number(hdr->checksum, sizeof(hdr->checksum));

			if (sum1 == 0 && crxa_tar_checksum(buf, sizeof(buf)) == 0) {
				break;
			}

			if (error) {
				spprintf(error, 4096, "crxa error: \"%s\" has entries after signature, invalid crxa", fname);
			}

			goto bail;
		}

		if (!last_was_longlink && hdr->typeflag == 'L') {
			last_was_longlink = 1;
			/* support the ././@LongLink system for storing long filenames */
			entry.filename_len = entry.uncompressed_filesize;

			/* Check for overflow - bug 61065 */
			if (entry.filename_len == UINT_MAX || entry.filename_len == 0) {
				if (error) {
					spprintf(error, 4096, "crxa error: \"%s\" is a corrupted tar file (invalid entry size)", fname);
				}
				crx_stream_close(fp);
				crxa_destroy_crxa_data(mycrxa);
				return FAILURE;
			}
			entry.filename = pemalloc(entry.filename_len+1, mycrxa->is_persistent);

			read = crx_stream_read(fp, entry.filename, entry.filename_len);
			if (read != entry.filename_len) {
				efree(entry.filename);
				if (error) {
					spprintf(error, 4096, "crxa error: \"%s\" is a corrupted tar file (truncated)", fname);
				}
				crx_stream_close(fp);
				crxa_destroy_crxa_data(mycrxa);
				return FAILURE;
			}
			entry.filename[entry.filename_len] = '\0';

			/* skip blank stuff */
			size = ((size+511)&~511) - size;

			/* this is not good enough - seek succeeds even on truncated tars */
			crx_stream_seek(fp, size, SEEK_CUR);
			if ((uint32_t)crx_stream_tell(fp) > totalsize) {
				efree(entry.filename);
				if (error) {
					spprintf(error, 4096, "crxa error: \"%s\" is a corrupted tar file (truncated)", fname);
				}
				crx_stream_close(fp);
				crxa_destroy_crxa_data(mycrxa);
				return FAILURE;
			}

			read = crx_stream_read(fp, buf, sizeof(buf));

			if (read != sizeof(buf)) {
				efree(entry.filename);
				if (error) {
					spprintf(error, 4096, "crxa error: \"%s\" is a corrupted tar file (truncated)", fname);
				}
				crx_stream_close(fp);
				crxa_destroy_crxa_data(mycrxa);
				return FAILURE;
			}
			continue;
		} else if (!last_was_longlink && !old && hdr->prefix[0] != 0) {
			char name[256];
			int i, j;

			for (i = 0; i < 155; i++) {
				name[i] = hdr->prefix[i];
				if (name[i] == '\0') {
					break;
				}
			}
			name[i++] = '/';
			for (j = 0; j < 100; j++) {
				name[i+j] = hdr->name[j];
				if (name[i+j] == '\0') {
					break;
				}
			}

			entry.filename_len = i+j;

			if (name[entry.filename_len - 1] == '/') {
				/* some tar programs store directories with trailing slash */
				entry.filename_len--;
			}
			entry.filename = pestrndup(name, entry.filename_len, mycrxa->is_persistent);
		} else if (!last_was_longlink) {
			int i;

			/* calculate strlen, which can be no longer than 100 */
			for (i = 0; i < 100; i++) {
				if (hdr->name[i] == '\0') {
					break;
				}
			}
			entry.filename_len = i;
			entry.filename = pestrndup(hdr->name, i, mycrxa->is_persistent);

			if (i > 0 && entry.filename[entry.filename_len - 1] == '/') {
				/* some tar programs store directories with trailing slash */
				entry.filename[entry.filename_len - 1] = '\0';
				entry.filename_len--;
			}
		}
		last_was_longlink = 0;

		crxa_add_virtual_dirs(mycrxa, entry.filename, entry.filename_len);

		if (sum1 != sum2) {
			if (error) {
				spprintf(error, 4096, "crxa error: \"%s\" is a corrupted tar file (checksum mismatch of file \"%s\")", fname, entry.filename);
			}
			pefree(entry.filename, mycrxa->is_persistent);
			crx_stream_close(fp);
			crxa_destroy_crxa_data(mycrxa);
			return FAILURE;
		}

		uint32_t entry_mode = crxa_tar_number(hdr->mode, sizeof(hdr->mode));
		entry.tar_type = ((old & (hdr->typeflag == '\0')) ? TAR_FILE : hdr->typeflag);
		entry.offset = entry.offset_abs = pos; /* header_offset unused in tar */
		entry.fp_type = CRXA_FP;
		entry.flags = entry_mode & CRXA_ENT_PERM_MASK;
		entry.timestamp = crxa_tar_number(hdr->mtime, sizeof(hdr->mtime));
		entry.is_persistent = mycrxa->is_persistent;

		if (old && entry.tar_type == TAR_FILE && S_ISDIR(entry_mode)) {
			entry.tar_type = TAR_DIR;
		}

		if (entry.tar_type == TAR_DIR) {
			entry.is_dir = 1;
		} else {
			entry.is_dir = 0;
		}

		entry.link = NULL;
		/* link field is null-terminated unless it has 100 non-null chars.
		 * Thus we cannot use strlen. */
		linkname_len = crex_strnlen(hdr->linkname, 100);
		if (entry.tar_type == TAR_LINK) {
			if (!crex_hash_str_exists(&mycrxa->manifest, hdr->linkname, linkname_len)) {
				if (error) {
					spprintf(error, 4096, "crxa error: \"%s\" is a corrupted tar file - hard link to non-existent file \"%.*s\"", fname, (int)linkname_len, hdr->linkname);
				}
				pefree(entry.filename, entry.is_persistent);
				crx_stream_close(fp);
				crxa_destroy_crxa_data(mycrxa);
				return FAILURE;
			}
			entry.link = estrndup(hdr->linkname, linkname_len);
		} else if (entry.tar_type == TAR_SYMLINK) {
			entry.link = estrndup(hdr->linkname, linkname_len);
		}
		crxa_set_inode(&entry);

		newentry = crex_hash_str_update_mem(&mycrxa->manifest, entry.filename, entry.filename_len, (void*)&entry, sizeof(crxa_entry_info));
		CREX_ASSERT(newentry != NULL);

		if (entry.is_persistent) {
			++entry.manifest_pos;
		}

		if (entry.filename_len >= sizeof(".crxa/.metadata")-1 && !memcmp(entry.filename, ".crxa/.metadata", sizeof(".crxa/.metadata")-1)) {
			if (FAILURE == crxa_tar_process_metadata(newentry, fp)) {
				if (error) {
					spprintf(error, 4096, "crxa error: tar-based crxa \"%s\" has invalid metadata in magic file \"%s\"", fname, entry.filename);
				}
				crx_stream_close(fp);
				crxa_destroy_crxa_data(mycrxa);
				return FAILURE;
			}
		}

		if (!actual_alias && entry.filename_len == sizeof(".crxa/alias.txt")-1 && !strncmp(entry.filename, ".crxa/alias.txt", sizeof(".crxa/alias.txt")-1)) {
			/* found explicit alias */
			if (size > 511) {
				if (error) {
					spprintf(error, 4096, "crxa error: tar-based crxa \"%s\" has alias that is larger than 511 bytes, cannot process", fname);
				}
				crx_stream_close(fp);
				crxa_destroy_crxa_data(mycrxa);
				return FAILURE;
			}

			read = crx_stream_read(fp, buf, size);

			if (read == size) {
				buf[size] = '\0';
				if (!crxa_validate_alias(buf, size)) {
					if (size > 50) {
						buf[50] = '.';
						buf[51] = '.';
						buf[52] = '.';
						buf[53] = '\0';
					}

					if (error) {
						spprintf(error, 4096, "crxa error: invalid alias \"%s\" in tar-based crxa \"%s\"", buf, fname);
					}

					crx_stream_close(fp);
					crxa_destroy_crxa_data(mycrxa);
					return FAILURE;
				}

				actual_alias = pestrndup(buf, size, mycrxa->is_persistent);
				mycrxa->alias = actual_alias;
				mycrxa->alias_len = size;
				crx_stream_seek(fp, pos, SEEK_SET);
			} else {
				if (error) {
					spprintf(error, 4096, "crxa error: Unable to read alias from tar-based crxa \"%s\"", fname);
				}

				crx_stream_close(fp);
				crxa_destroy_crxa_data(mycrxa);
				return FAILURE;
			}
		}

		size = (size+511)&~511;

		if (((hdr->typeflag == '\0') || (hdr->typeflag == TAR_FILE)) && size > 0) {
next:
			/* this is not good enough - seek succeeds even on truncated tars */
			crx_stream_seek(fp, size, SEEK_CUR);
			if ((uint32_t)crx_stream_tell(fp) > totalsize) {
				if (error) {
					spprintf(error, 4096, "crxa error: \"%s\" is a corrupted tar file (truncated)", fname);
				}
				crx_stream_close(fp);
				crxa_destroy_crxa_data(mycrxa);
				return FAILURE;
			}
		}

		read = crx_stream_read(fp, buf, sizeof(buf));

		if (read != sizeof(buf)) {
			if (error) {
				spprintf(error, 4096, "crxa error: \"%s\" is a corrupted tar file (truncated)", fname);
			}
			crx_stream_close(fp);
			crxa_destroy_crxa_data(mycrxa);
			return FAILURE;
		}
	} while (!crx_stream_eof(fp));

	if (crex_hash_str_exists(&(mycrxa->manifest), ".crxa/stub.crx", sizeof(".crxa/stub.crx")-1)) {
		mycrxa->is_data = 0;
	} else {
		mycrxa->is_data = 1;
	}

	/* ensure signature set */
	if (!mycrxa->is_data && CRXA_G(require_hash) && !mycrxa->signature) {
		crx_stream_close(fp);
		crxa_destroy_crxa_data(mycrxa);
		if (error) {
			spprintf(error, 0, "tar-based crxa \"%s\" does not have a signature", fname);
		}
		return FAILURE;
	}

	mycrxa->fname = pestrndup(fname, fname_len, mycrxa->is_persistent);
#ifdef CRX_WIN32
	crxa_unixify_path_separators(mycrxa->fname, fname_len);
#endif
	mycrxa->fname_len = fname_len;
	mycrxa->fp = fp;
	p = strrchr(mycrxa->fname, '/');

	if (p) {
		mycrxa->ext = memchr(p, '.', (mycrxa->fname + fname_len) - p);
		if (mycrxa->ext == p) {
			mycrxa->ext = memchr(p + 1, '.', (mycrxa->fname + fname_len) - p - 1);
		}
		if (mycrxa->ext) {
			mycrxa->ext_len = (mycrxa->fname + fname_len) - mycrxa->ext;
		}
	}

	crxa_request_initialize();

	if (NULL == (actual = crex_hash_str_add_ptr(&(CRXA_G(crxa_fname_map)), mycrxa->fname, fname_len, mycrxa))) {
		if (error) {
			spprintf(error, 4096, "crxa error: Unable to add tar-based crxa \"%s\" to crxa registry", fname);
		}
		crx_stream_close(fp);
		crxa_destroy_crxa_data(mycrxa);
		return FAILURE;
	}

	mycrxa = actual;

	if (actual_alias) {
		crxa_archive_data *fd_ptr;

		mycrxa->is_temporary_alias = 0;

		if (NULL != (fd_ptr = crex_hash_str_find_ptr(&(CRXA_G(crxa_alias_map)), actual_alias, mycrxa->alias_len))) {
			if (SUCCESS != crxa_free_alias(fd_ptr, actual_alias, mycrxa->alias_len)) {
				if (error) {
					spprintf(error, 4096, "crxa error: Unable to add tar-based crxa \"%s\", alias is already in use", fname);
				}
				crex_hash_str_del(&(CRXA_G(crxa_fname_map)), mycrxa->fname, fname_len);
				return FAILURE;
			}
		}

		crex_hash_str_add_ptr(&(CRXA_G(crxa_alias_map)), actual_alias, mycrxa->alias_len, mycrxa);
	} else {
		crxa_archive_data *fd_ptr;

		if (alias_len) {
			if (NULL != (fd_ptr = crex_hash_str_find_ptr(&(CRXA_G(crxa_alias_map)), alias, alias_len))) {
				if (SUCCESS != crxa_free_alias(fd_ptr, alias, alias_len)) {
					if (error) {
						spprintf(error, 4096, "crxa error: Unable to add tar-based crxa \"%s\", alias is already in use", fname);
					}
					crex_hash_str_del(&(CRXA_G(crxa_fname_map)), mycrxa->fname, fname_len);
					return FAILURE;
				}
			}
			crex_hash_str_add_ptr(&(CRXA_G(crxa_alias_map)), alias, alias_len, mycrxa);
			mycrxa->alias = pestrndup(alias, alias_len, mycrxa->is_persistent);
			mycrxa->alias_len = alias_len;
		} else {
			mycrxa->alias = pestrndup(mycrxa->fname, fname_len, mycrxa->is_persistent);
			mycrxa->alias_len = fname_len;
		}

		mycrxa->is_temporary_alias = 1;
	}

	if (pcrxa) {
		*pcrxa = mycrxa;
	}

	return SUCCESS;
}
/* }}} */

struct _crxa_pass_tar_info {
	crx_stream *old;
	crx_stream *new;
	int free_fp;
	int free_ufp;
	char **error;
};

static int crxa_tar_writeheaders_int(crxa_entry_info *entry, void *argument) /* {{{ */
{
	tar_header header;
	size_t pos;
	struct _crxa_pass_tar_info *fp = (struct _crxa_pass_tar_info *)argument;
	char padding[512];

	if (entry->is_mounted) {
		return CREX_HASH_APPLY_KEEP;
	}

	if (entry->is_deleted) {
		if (entry->fp_refcount <= 0) {
			return CREX_HASH_APPLY_REMOVE;
		} else {
			/* we can't delete this in-memory until it is closed */
			return CREX_HASH_APPLY_KEEP;
		}
	}

	crxa_add_virtual_dirs(entry->crxa, entry->filename, entry->filename_len);
	memset((char *) &header, 0, sizeof(header));

	if (entry->filename_len > 100) {
		char *boundary;
		if (entry->filename_len > 256) {
			if (fp->error) {
				spprintf(fp->error, 4096, "tar-based crxa \"%s\" cannot be created, filename \"%s\" is too long for tar file format", entry->crxa->fname, entry->filename);
			}
			return CREX_HASH_APPLY_STOP;
		}
		boundary = entry->filename + entry->filename_len - 101;
		while (*boundary && *boundary != '/') {
			++boundary;
		}
		if (!*boundary || ((boundary - entry->filename) > 155)) {
			if (fp->error) {
				spprintf(fp->error, 4096, "tar-based crxa \"%s\" cannot be created, filename \"%s\" is too long for tar file format", entry->crxa->fname, entry->filename);
			}
			return CREX_HASH_APPLY_STOP;
		}
		memcpy(header.prefix, entry->filename, boundary - entry->filename);
		memcpy(header.name, boundary + 1, entry->filename_len - (boundary + 1 - entry->filename));
	} else {
		memcpy(header.name, entry->filename, entry->filename_len);
	}

	crxa_tar_octal(header.mode, entry->flags & CRXA_ENT_PERM_MASK, sizeof(header.mode)-1);

	if (FAILURE == crxa_tar_octal(header.size, entry->uncompressed_filesize, sizeof(header.size)-1)) {
		if (fp->error) {
			spprintf(fp->error, 4096, "tar-based crxa \"%s\" cannot be created, filename \"%s\" is too large for tar file format", entry->crxa->fname, entry->filename);
		}
		return CREX_HASH_APPLY_STOP;
	}

	if (FAILURE == crxa_tar_octal(header.mtime, entry->timestamp, sizeof(header.mtime)-1)) {
		if (fp->error) {
			spprintf(fp->error, 4096, "tar-based crxa \"%s\" cannot be created, file modification time of file \"%s\" is too large for tar file format", entry->crxa->fname, entry->filename);
		}
		return CREX_HASH_APPLY_STOP;
	}

	/* calc checksum */
	header.typeflag = entry->tar_type;

	if (entry->link) {
		if (strlcpy(header.linkname, entry->link, sizeof(header.linkname)) >= sizeof(header.linkname)) {
			if (fp->error) {
				spprintf(fp->error, 4096, "tar-based crxa \"%s\" cannot be created, link \"%s\" is too long for format", entry->crxa->fname, entry->link);
			}
			return CREX_HASH_APPLY_STOP;
		}
	}

	memcpy(header.magic, "ustar", sizeof("ustar")-1);
	memcpy(header.version, "00", sizeof("00")-1);
	memcpy(header.checksum, "        ", sizeof("        ")-1);
	entry->crc32 = crxa_tar_checksum((char *)&header, sizeof(header));

	if (FAILURE == crxa_tar_octal(header.checksum, entry->crc32, sizeof(header.checksum)-1)) {
		if (fp->error) {
			spprintf(fp->error, 4096, "tar-based crxa \"%s\" cannot be created, checksum of file \"%s\" is too large for tar file format", entry->crxa->fname, entry->filename);
		}
		return CREX_HASH_APPLY_STOP;
	}

	/* write header */
	entry->header_offset = crx_stream_tell(fp->new);

	if (sizeof(header) != crx_stream_write(fp->new, (char *) &header, sizeof(header))) {
		if (fp->error) {
			spprintf(fp->error, 4096, "tar-based crxa \"%s\" cannot be created, header for  file \"%s\" could not be written", entry->crxa->fname, entry->filename);
		}
		return CREX_HASH_APPLY_STOP;
	}

	pos = crx_stream_tell(fp->new); /* save start of file within tar */

	/* write contents */
	if (entry->uncompressed_filesize) {
		if (FAILURE == crxa_open_entry_fp(entry, fp->error, 0)) {
			return CREX_HASH_APPLY_STOP;
		}

		if (-1 == crxa_seek_efp(entry, 0, SEEK_SET, 0, 0)) {
			if (fp->error) {
				spprintf(fp->error, 4096, "tar-based crxa \"%s\" cannot be created, contents of file \"%s\" could not be written, seek failed", entry->crxa->fname, entry->filename);
			}
			return CREX_HASH_APPLY_STOP;
		}

		if (SUCCESS != crx_stream_copy_to_stream_ex(crxa_get_efp(entry, 0), fp->new, entry->uncompressed_filesize, NULL)) {
			if (fp->error) {
				spprintf(fp->error, 4096, "tar-based crxa \"%s\" cannot be created, contents of file \"%s\" could not be written", entry->crxa->fname, entry->filename);
			}
			return CREX_HASH_APPLY_STOP;
		}

		memset(padding, 0, 512);
		crx_stream_write(fp->new, padding, ((entry->uncompressed_filesize +511)&~511) - entry->uncompressed_filesize);
	}

	if (!entry->is_modified && entry->fp_refcount) {
		/* open file pointers refer to this fp, do not free the stream */
		switch (entry->fp_type) {
			case CRXA_FP:
				fp->free_fp = 0;
				break;
			case CRXA_UFP:
				fp->free_ufp = 0;
			default:
				break;
		}
	}

	entry->is_modified = 0;

	if (entry->fp_type == CRXA_MOD && entry->fp != entry->crxa->fp && entry->fp != entry->crxa->ufp) {
		if (!entry->fp_refcount) {
			crx_stream_close(entry->fp);
		}
		entry->fp = NULL;
	}

	entry->fp_type = CRXA_FP;

	/* note new location within tar */
	entry->offset = entry->offset_abs = pos;
	return CREX_HASH_APPLY_KEEP;
}
/* }}} */

static int crxa_tar_writeheaders(zval *zv, void *argument) /* {{{ */
{
	return crxa_tar_writeheaders_int(C_PTR_P(zv), argument);
}
/* }}} */

int crxa_tar_setmetadata(const crxa_metadata_tracker *tracker, crxa_entry_info *entry, char **error) /* {{{ */
{
	/* Copy the metadata from tracker to the new entry being written out to temporary files */
	const crex_string *serialized_str;
	crxa_metadata_tracker_copy(&entry->metadata_tracker, tracker, entry->is_persistent);
	crxa_metadata_tracker_try_ensure_has_serialized_data(&entry->metadata_tracker, entry->is_persistent);
	serialized_str = entry->metadata_tracker.str;

	/* If there is no data, this will replace the metadata file (e.g. .crxa/.metadata.bin) with an empty file */
	entry->uncompressed_filesize = entry->compressed_filesize = serialized_str ? ZSTR_LEN(serialized_str) : 0;

	if (entry->fp && entry->fp_type == CRXA_MOD) {
		crx_stream_close(entry->fp);
	}

	entry->fp_type = CRXA_MOD;
	entry->is_modified = 1;
	entry->fp = crx_stream_fopen_tmpfile();
	entry->offset = entry->offset_abs = 0;
	if (entry->fp == NULL) {
		spprintf(error, 0, "crxa error: unable to create temporary file");
		return -1;
	}
	if (serialized_str && ZSTR_LEN(serialized_str) != crx_stream_write(entry->fp, ZSTR_VAL(serialized_str), ZSTR_LEN(serialized_str))) {
		spprintf(error, 0, "crxa tar error: unable to write metadata to magic metadata file \"%s\"", entry->filename);
		crex_hash_str_del(&(entry->crxa->manifest), entry->filename, entry->filename_len);
		return CREX_HASH_APPLY_STOP;
	}

	return CREX_HASH_APPLY_KEEP;
}
/* }}} */

static int crxa_tar_setupmetadata(zval *zv, void *argument) /* {{{ */
{
	int lookfor_len;
	struct _crxa_pass_tar_info *i = (struct _crxa_pass_tar_info *)argument;
	char *lookfor, **error = i->error;
	crxa_entry_info *entry = (crxa_entry_info *)C_PTR_P(zv), *metadata, newentry = {0};

	if (entry->filename_len >= sizeof(".crxa/.metadata") && !memcmp(entry->filename, ".crxa/.metadata", sizeof(".crxa/.metadata")-1)) {
		if (entry->filename_len == sizeof(".crxa/.metadata.bin")-1 && !memcmp(entry->filename, ".crxa/.metadata.bin", sizeof(".crxa/.metadata.bin")-1)) {
			return crxa_tar_setmetadata(&entry->crxa->metadata_tracker, entry, error);
		}
		/* search for the file this metadata entry references */
		if (entry->filename_len >= sizeof(".crxa/.metadata/") + sizeof("/.metadata.bin") - 1 && !crex_hash_str_exists(&(entry->crxa->manifest), entry->filename + sizeof(".crxa/.metadata/") - 1, entry->filename_len - (sizeof("/.metadata.bin") - 1 + sizeof(".crxa/.metadata/") - 1))) {
			/* this is orphaned metadata, erase it */
			return CREX_HASH_APPLY_REMOVE;
		}
		/* we can keep this entry, the file that refers to it exists */
		return CREX_HASH_APPLY_KEEP;
	}

	if (!entry->is_modified) {
		return CREX_HASH_APPLY_KEEP;
	}

	/* now we are dealing with regular files, so look for metadata */
	lookfor_len = spprintf(&lookfor, 0, ".crxa/.metadata/%s/.metadata.bin", entry->filename);

	if (!crxa_metadata_tracker_has_data(&entry->metadata_tracker, entry->is_persistent)) {
		crex_hash_str_del(&(entry->crxa->manifest), lookfor, lookfor_len);
		efree(lookfor);
		return CREX_HASH_APPLY_KEEP;
	}

	if (NULL != (metadata = crex_hash_str_find_ptr(&(entry->crxa->manifest), lookfor, lookfor_len))) {
		int ret;
		ret = crxa_tar_setmetadata(&entry->metadata_tracker, metadata, error);
		efree(lookfor);
		return ret;
	}

	newentry.filename = lookfor;
	newentry.filename_len = lookfor_len;
	newentry.crxa = entry->crxa;
	newentry.tar_type = TAR_FILE;
	newentry.is_tar = 1;

	if (NULL == (metadata = crex_hash_str_add_mem(&(entry->crxa->manifest), lookfor, lookfor_len, (void *)&newentry, sizeof(crxa_entry_info)))) {
		efree(lookfor);
		spprintf(error, 0, "crxa tar error: unable to add magic metadata file to manifest for file \"%s\"", entry->filename);
		return CREX_HASH_APPLY_STOP;
	}

	return crxa_tar_setmetadata(&entry->metadata_tracker, metadata, error);
}
/* }}} */

int crxa_tar_flush(crxa_archive_data *crxa, char *user_stub, crex_long len, int defaultstub, char **error) /* {{{ */
{
	crxa_entry_info entry = {0};
	static const char newstub[] = "<?crx // tar-based crxa archive stub file\n__HALT_COMPILER();";
	crx_stream *oldfile, *newfile, *stubfile;
	int closeoldfile, free_user_stub;
	size_t signature_length;
	struct _crxa_pass_tar_info pass;
	char *buf, *signature, sigbuf[8];
	char halt_stub[] = "__HALT_COMPILER();";

	entry.flags = CRXA_ENT_PERM_DEF_FILE;
	entry.timestamp = time(NULL);
	entry.is_modified = 1;
	entry.is_crc_checked = 1;
	entry.is_tar = 1;
	entry.tar_type = '0';
	entry.crxa = crxa;
	entry.fp_type = CRXA_MOD;
	entry.fp = NULL;
	entry.filename = NULL;

	if (crxa->is_persistent) {
		if (error) {
			spprintf(error, 0, "internal error: attempt to flush cached tar-based crxa \"%s\"", crxa->fname);
		}
		return EOF;
	}

	if (crxa->is_data) {
		goto nostub;
	}

	/* set alias */
	if (!crxa->is_temporary_alias && crxa->alias_len) {
		entry.filename = estrndup(".crxa/alias.txt", sizeof(".crxa/alias.txt")-1);
		entry.filename_len = sizeof(".crxa/alias.txt")-1;
		entry.fp = crx_stream_fopen_tmpfile();
		if (entry.fp == NULL) {
			efree(entry.filename);
			spprintf(error, 0, "crxa error: unable to create temporary file");
			return -1;
		}
		if (crxa->alias_len != crx_stream_write(entry.fp, crxa->alias, crxa->alias_len)) {
			if (error) {
				spprintf(error, 0, "unable to set alias in tar-based crxa \"%s\"", crxa->fname);
			}
			crx_stream_close(entry.fp);
			efree(entry.filename);
			return EOF;
		}

		entry.uncompressed_filesize = crxa->alias_len;

		crex_hash_str_update_mem(&crxa->manifest, entry.filename, entry.filename_len, (void*)&entry, sizeof(crxa_entry_info));
		/* At this point the entry is saved into the manifest. The manifest destroy
			routine will care about any resources to be freed. */
	} else {
		crex_hash_str_del(&crxa->manifest, ".crxa/alias.txt", sizeof(".crxa/alias.txt")-1);
	}

	/* set stub */
	if (user_stub && !defaultstub) {
		char *pos;
		if (len < 0) {
			/* resource passed in */
			if (!(crx_stream_from_zval_no_verify(stubfile, (zval *)user_stub))) {
				if (error) {
					spprintf(error, 0, "unable to access resource to copy stub to new tar-based crxa \"%s\"", crxa->fname);
				}
				return EOF;
			}
			if (len == -1) {
				len = CRX_STREAM_COPY_ALL;
			} else {
				len = -len;
			}
			user_stub = 0;

			// TODO: refactor to avoid reallocation ???
//???		len = crx_stream_copy_to_mem(stubfile, &user_stub, len, 0)
			{
				crex_string *str = crx_stream_copy_to_mem(stubfile, len, 0);
				if (str) {
					len = ZSTR_LEN(str);
					user_stub = estrndup(ZSTR_VAL(str), ZSTR_LEN(str));
					crex_string_release_ex(str, 0);
				} else {
					user_stub = NULL;
					len = 0;
				}
			}

			if (!len || !user_stub) {
				if (error) {
					spprintf(error, 0, "unable to read resource to copy stub to new tar-based crxa \"%s\"", crxa->fname);
				}
				return EOF;
			}
			free_user_stub = 1;
		} else {
			free_user_stub = 0;
		}

		if ((pos = crx_stristr(user_stub, halt_stub, len, sizeof(halt_stub) - 1)) == NULL) {
			if (error) {
				spprintf(error, 0, "illegal stub for tar-based crxa \"%s\"", crxa->fname);
			}
			if (free_user_stub) {
				efree(user_stub);
			}
			return EOF;
		}

		len = pos - user_stub + 18;
		entry.fp = crx_stream_fopen_tmpfile();
		if (entry.fp == NULL) {
			spprintf(error, 0, "crxa error: unable to create temporary file");
			return EOF;
		}
		entry.uncompressed_filesize = len + 5;

		if ((size_t)len != crx_stream_write(entry.fp, user_stub, len)
		||            5 != crx_stream_write(entry.fp, " ?>\r\n", 5)) {
			if (error) {
				spprintf(error, 0, "unable to create stub from string in new tar-based crxa \"%s\"", crxa->fname);
			}
			if (free_user_stub) {
				efree(user_stub);
			}
			crx_stream_close(entry.fp);
			return EOF;
		}

		entry.filename = estrndup(".crxa/stub.crx", sizeof(".crxa/stub.crx")-1);
		entry.filename_len = sizeof(".crxa/stub.crx")-1;
		crex_hash_str_update_mem(&crxa->manifest, entry.filename, entry.filename_len, (void*)&entry, sizeof(crxa_entry_info));

		if (free_user_stub) {
			efree(user_stub);
		}
	} else {
		/* Either this is a brand new crxa (add the stub), or the default stub is required (overwrite the stub) */
		entry.fp = crx_stream_fopen_tmpfile();
		if (entry.fp == NULL) {
			spprintf(error, 0, "crxa error: unable to create temporary file");
			return EOF;
		}
		if (sizeof(newstub)-1 != crx_stream_write(entry.fp, newstub, sizeof(newstub)-1)) {
			crx_stream_close(entry.fp);
			if (error) {
				spprintf(error, 0, "unable to %s stub in%star-based crxa \"%s\", failed", user_stub ? "overwrite" : "create", user_stub ? " " : " new ", crxa->fname);
			}
			return EOF;
		}

		entry.uncompressed_filesize = entry.compressed_filesize = sizeof(newstub) - 1;
		entry.filename = estrndup(".crxa/stub.crx", sizeof(".crxa/stub.crx")-1);
		entry.filename_len = sizeof(".crxa/stub.crx")-1;

		if (!defaultstub) {
			if (!crex_hash_str_exists(&crxa->manifest, ".crxa/stub.crx", sizeof(".crxa/stub.crx")-1)) {
				if (NULL == crex_hash_str_add_mem(&crxa->manifest, entry.filename, entry.filename_len, (void*)&entry, sizeof(crxa_entry_info))) {
					crx_stream_close(entry.fp);
					efree(entry.filename);
					if (error) {
						spprintf(error, 0, "unable to create stub in tar-based crxa \"%s\"", crxa->fname);
					}
					return EOF;
				}
			} else {
				crx_stream_close(entry.fp);
				efree(entry.filename);
			}
		} else {
			crex_hash_str_update_mem(&crxa->manifest, entry.filename, entry.filename_len, (void*)&entry, sizeof(crxa_entry_info));
		}
	}
nostub:
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

	pass.old = oldfile;
	pass.new = newfile;
	pass.error = error;
	pass.free_fp = 1;
	pass.free_ufp = 1;

	if (crxa_metadata_tracker_has_data(&crxa->metadata_tracker, crxa->is_persistent)) {
		crxa_entry_info *mentry;
		if (NULL != (mentry = crex_hash_str_find_ptr(&(crxa->manifest), ".crxa/.metadata.bin", sizeof(".crxa/.metadata.bin")-1))) {
			if (CREX_HASH_APPLY_KEEP != crxa_tar_setmetadata(&crxa->metadata_tracker, mentry, error)) {
				if (closeoldfile) {
					crx_stream_close(oldfile);
				}
				return EOF;
			}
		} else {
			crxa_entry_info newentry = {0};

			newentry.filename = estrndup(".crxa/.metadata.bin", sizeof(".crxa/.metadata.bin")-1);
			newentry.filename_len = sizeof(".crxa/.metadata.bin")-1;
			newentry.crxa = crxa;
			newentry.tar_type = TAR_FILE;
			newentry.is_tar = 1;

			if (NULL == (mentry = crex_hash_str_add_mem(&(crxa->manifest), ".crxa/.metadata.bin", sizeof(".crxa/.metadata.bin")-1, (void *)&newentry, sizeof(crxa_entry_info)))) {
				spprintf(error, 0, "crxa tar error: unable to add magic metadata file to manifest for crxa archive \"%s\"", crxa->fname);
				if (closeoldfile) {
					crx_stream_close(oldfile);
				}
				return EOF;
			}

			if (CREX_HASH_APPLY_KEEP != crxa_tar_setmetadata(&crxa->metadata_tracker, mentry, error)) {
				crex_hash_str_del(&(crxa->manifest), ".crxa/.metadata.bin", sizeof(".crxa/.metadata.bin")-1);
				if (closeoldfile) {
					crx_stream_close(oldfile);
				}
				return EOF;
			}
		}
	}

	crex_hash_apply_with_argument(&crxa->manifest, crxa_tar_setupmetadata, (void *) &pass);

	if (error && *error) {
		if (closeoldfile) {
			crx_stream_close(oldfile);
		}

		/* on error in the hash iterator above, error is set */
		crx_stream_close(newfile);
		return EOF;
	}

	crex_hash_apply_with_argument(&crxa->manifest, crxa_tar_writeheaders, (void *) &pass);

	/* add signature for executable tars or tars explicitly set with setSignatureAlgorithm */
	if (!crxa->is_data || crxa->sig_flags) {
		if (FAILURE == crxa_create_signature(crxa, newfile, &signature, &signature_length, error)) {
			if (error) {
				char *save = *error;
				spprintf(error, 0, "crxa error: unable to write signature to tar-based crxa: %s", save);
				efree(save);
			}

			if (closeoldfile) {
				crx_stream_close(oldfile);
			}

			crx_stream_close(newfile);
			return EOF;
		}

		entry.filename = ".crxa/signature.bin";
		entry.filename_len = sizeof(".crxa/signature.bin")-1;
		entry.fp = crx_stream_fopen_tmpfile();
		if (entry.fp == NULL) {
			spprintf(error, 0, "crxa error: unable to create temporary file");
			return EOF;
		}
#ifdef WORDS_BIGENDIAN
# define CRXA_SET_32(destination, source) do { \
        uint32_t swapped = (((((unsigned char*)&(source))[3]) << 24) \
            | ((((unsigned char*)&(source))[2]) << 16) \
            | ((((unsigned char*)&(source))[1]) << 8) \
            | (((unsigned char*)&(source))[0])); \
        memcpy(destination, &swapped, 4); \
    } while (0);
#else
# define CRXA_SET_32(destination, source) memcpy(destination, &source, 4)
#endif
		CRXA_SET_32(sigbuf, crxa->sig_flags);
		CRXA_SET_32(sigbuf + 4, signature_length);

		if (8 != crx_stream_write(entry.fp, sigbuf, 8) || signature_length != crx_stream_write(entry.fp, signature, signature_length)) {
			efree(signature);
			if (error) {
				spprintf(error, 0, "crxa error: unable to write signature to tar-based crxa %s", crxa->fname);
			}

			if (closeoldfile) {
				crx_stream_close(oldfile);
			}
			crx_stream_close(newfile);
			return EOF;
		}

		efree(signature);
		entry.uncompressed_filesize = entry.compressed_filesize = signature_length + 8;
		/* throw out return value and write the signature */
		entry.filename_len = crxa_tar_writeheaders_int(&entry, (void *)&pass);

		if (error && *error) {
			if (closeoldfile) {
				crx_stream_close(oldfile);
			}
			/* error is set by writeheaders */
			crx_stream_close(newfile);
			return EOF;
		}
	} /* signature */

	/* add final zero blocks */
	buf = (char *) ecalloc(1024, 1);
	crx_stream_write(newfile, buf, 1024);
	efree(buf);

	if (closeoldfile) {
		crx_stream_close(oldfile);
	}

	/* on error in the hash iterator above, error is set */
	if (error && *error) {
		crx_stream_close(newfile);
		return EOF;
	}

	if (crxa->fp && pass.free_fp) {
		crx_stream_close(crxa->fp);
	}

	if (crxa->ufp) {
		if (pass.free_ufp) {
			crx_stream_close(crxa->ufp);
		}
		crxa->ufp = NULL;
	}

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
				spprintf(error, 0, "unable to open new crxa \"%s\" for writing", crxa->fname);
			}
			return EOF;
		}

		if (crxa->flags & CRXA_FILE_COMPRESSED_GZ) {
			crx_stream_filter *filter;
			/* to properly compress, we have to tell zlib to add a zlib header */
			zval filterparams;

			array_init(&filterparams);
/* this is defined in zlib's zconf.h */
#ifndef MAX_WBITS
#define MAX_WBITS 15
#endif
			add_assoc_long(&filterparams, "window", MAX_WBITS + 16);
			filter = crx_stream_filter_create("zlib.deflate", &filterparams, crx_stream_is_persistent(crxa->fp));
			crex_array_destroy(C_ARR(filterparams));

			if (!filter) {
				/* copy contents uncompressed rather than lose them */
				crx_stream_copy_to_stream_ex(newfile, crxa->fp, CRX_STREAM_COPY_ALL, NULL);
				crx_stream_close(newfile);
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
			crx_stream_filter *filter;

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
	return EOF;
}
/* }}} */
