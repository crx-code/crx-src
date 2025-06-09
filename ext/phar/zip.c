/*
  +----------------------------------------------------------------------+
  | ZIP archive support for Crxa                                         |
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

#define CRXA_GET_16(var) ((uint16_t)((((uint16_t)var[0]) & 0xff) | \
	(((uint16_t)var[1]) & 0xff) << 8))
#define CRXA_GET_32(var) ((uint32_t)((((uint32_t)var[0]) & 0xff) | \
	(((uint32_t)var[1]) & 0xff) << 8 | \
	(((uint32_t)var[2]) & 0xff) << 16 | \
	(((uint32_t)var[3]) & 0xff) << 24))
static inline void crxa_write_32(char buffer[4], uint32_t value)
{
	buffer[3] = (unsigned char) ((value & 0xff000000) >> 24);
	buffer[2] = (unsigned char) ((value & 0xff0000) >> 16);
	buffer[1] = (unsigned char) ((value & 0xff00) >> 8);
	buffer[0] = (unsigned char) (value & 0xff);
}
static inline void crxa_write_16(char buffer[2], uint32_t value)
{
	buffer[1] = (unsigned char) ((value & 0xff00) >> 8);
	buffer[0] = (unsigned char) (value & 0xff);
}
# define CRXA_SET_32(var, value) crxa_write_32(var, (uint32_t) (value));
# define CRXA_SET_16(var, value) crxa_write_16(var, (uint16_t) (value));

static int crxa_zip_process_extra(crx_stream *fp, crxa_entry_info *entry, uint16_t len) /* {{{ */
{
	union {
		crxa_zip_extra_field_header header;
		crxa_zip_unix3 unix3;
	} h;
	size_t read;

	do {
		if (sizeof(h.header) != crx_stream_read(fp, (char *) &h.header, sizeof(h.header))) {
			return FAILURE;
		}

		if (h.header.tag[0] != 'n' || h.header.tag[1] != 'u') {
			/* skip to next header */
			crx_stream_seek(fp, CRXA_GET_16(h.header.size), SEEK_CUR);
			len -= CRXA_GET_16(h.header.size) + 4;
			continue;
		}

		/* unix3 header found */
		read = crx_stream_read(fp, (char *) &(h.unix3.crc32), sizeof(h.unix3) - sizeof(h.header));
		len -= read + 4;

		if (sizeof(h.unix3) - sizeof(h.header) != read) {
			return FAILURE;
		}

		if (CRXA_GET_16(h.unix3.size) > sizeof(h.unix3) - 4) {
			/* skip symlink filename - we may add this support in later */
			crx_stream_seek(fp, CRXA_GET_16(h.unix3.size) - sizeof(h.unix3.size), SEEK_CUR);
		}

		/* set permissions */
		entry->flags &= CRXA_ENT_COMPRESSION_MASK;

		if (entry->is_dir) {
			entry->flags |= CRXA_GET_16(h.unix3.perms) & CRXA_ENT_PERM_MASK;
		} else {
			entry->flags |= CRXA_GET_16(h.unix3.perms) & CRXA_ENT_PERM_MASK;
		}

	} while (len);

	return SUCCESS;
}
/* }}} */

/*
  extracted from libzip
  zip_dirent.c -- read directory entry (local or central), clean dirent
  Copyright (C) 1999, 2003, 2004, 2005 Dieter Baron and Thomas Klausner

  This function is part of libzip, a library to manipulate ZIP archives.
  The authors can be contacted at <nih@giga.or.at>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
  3. The names of the authors may not be used to endorse or promote
     products derived from this software without specific prior
     written permission.

  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
static time_t crxa_zip_d2u_time(char *cdtime, char *cddate) /* {{{ */
{
	int dtime = CRXA_GET_16(cdtime), ddate = CRXA_GET_16(cddate);
	struct tm *tm, tmbuf;
	time_t now;

	now = time(NULL);
	tm = crx_localtime_r(&now, &tmbuf);

	tm->tm_year = ((ddate>>9)&127) + 1980 - 1900;
	tm->tm_mon = ((ddate>>5)&15) - 1;
	tm->tm_mday = ddate&31;

	tm->tm_hour = (dtime>>11)&31;
	tm->tm_min = (dtime>>5)&63;
	tm->tm_sec = (dtime<<1)&62;

	return mktime(tm);
}
/* }}} */

static void crxa_zip_u2d_time(time_t time, char *dtime, char *ddate) /* {{{ */
{
	uint16_t ctime, cdate;
	struct tm *tm, tmbuf;

	tm = crx_localtime_r(&time, &tmbuf);
	/* Note: tm_year is the year - 1900 */
	if (tm->tm_year >= 80) {
		cdate = ((tm->tm_year+1900-1980)<<9) + ((tm->tm_mon+1)<<5) + tm->tm_mday;
		ctime = ((tm->tm_hour)<<11) + ((tm->tm_min)<<5) + ((tm->tm_sec)>>1);
	} else {
		/* This is the earliest date/time supported by zip. */
		cdate = (1<<5) + 1; /* 1980-01-01 */
		ctime = 0; /* 00:00:00 */
	}

	CRXA_SET_16(dtime, ctime);
	CRXA_SET_16(ddate, cdate);
}
/* }}} */

static char *crxa_find_eocd(const char *s, size_t n)
{
	const char *end = s + n + sizeof("PK\5\6") - 1 - sizeof(crxa_zip_dir_end);

	/* search backwards for end of central directory signatures */
	do {
		uint16_t comment_len;
		const char *eocd_start = crex_memnrstr(s, "PK\5\6", sizeof("PK\5\6") - 1, end);

		if (eocd_start == NULL) {
			return NULL;
		}
		CREX_ASSERT(eocd_start + sizeof(crxa_zip_dir_end) <= s + n);
		comment_len = CRXA_GET_16(((crxa_zip_dir_end *) eocd_start)->comment_len);
		if (eocd_start + sizeof(crxa_zip_dir_end) + comment_len == s + n) {
			/* we can't be sure, but this looks like the proper EOCD signature */
			return (char *) eocd_start;
		}
		end = eocd_start;
	} while (end > s);
	return NULL;
}

/**
 * Does not check for a previously opened crxa in the cache.
 *
 * Parse a new one and add it to the cache, returning either SUCCESS or
 * FAILURE, and setting pcrxa to the pointer to the manifest entry
 *
 * This is used by crxa_open_from_fp to process a zip-based crxa, but can be called
 * directly.
 */
int crxa_parse_zipfile(crx_stream *fp, char *fname, size_t fname_len, char *alias, size_t alias_len, crxa_archive_data** pcrxa, char **error) /* {{{ */
{
	crxa_zip_dir_end locator;
	char buf[sizeof(locator) + 65536];
	crex_off_t size;
	uint16_t i;
	crxa_archive_data *mydata = NULL;
	crxa_entry_info entry = {0};
	char *p = buf, *ext, *actual_alias = NULL;
	char *metadata = NULL;

	size = crx_stream_tell(fp);

	if (size > sizeof(locator) + 65536) {
		/* seek to max comment length + end of central directory record */
		size = sizeof(locator) + 65536;
		if (FAILURE == crx_stream_seek(fp, -size, SEEK_END)) {
			crx_stream_close(fp);
			if (error) {
				spprintf(error, 4096, "crxa error: unable to search for end of central directory in zip-based crxa \"%s\"", fname);
			}
			return FAILURE;
		}
	} else {
		crx_stream_seek(fp, 0, SEEK_SET);
	}

	if (!crx_stream_read(fp, buf, size)) {
		crx_stream_close(fp);
		if (error) {
			spprintf(error, 4096, "crxa error: unable to read in data to search for end of central directory in zip-based crxa \"%s\"", fname);
		}
		return FAILURE;
	}

	if ((p = crxa_find_eocd(buf, size)) != NULL) {
		memcpy((void *)&locator, (void *) p, sizeof(locator));
		if (CRXA_GET_16(locator.centraldisk) != 0 || CRXA_GET_16(locator.disknumber) != 0) {
			/* split archives not handled */
			crx_stream_close(fp);
			if (error) {
				spprintf(error, 4096, "crxa error: split archives spanning multiple zips cannot be processed in zip-based crxa \"%s\"", fname);
			}
			return FAILURE;
		}

		if (CRXA_GET_16(locator.counthere) != CRXA_GET_16(locator.count)) {
			if (error) {
				spprintf(error, 4096, "crxa error: corrupt zip archive, conflicting file count in end of central directory record in zip-based crxa \"%s\"", fname);
			}
			crx_stream_close(fp);
			return FAILURE;
		}

		mydata = pecalloc(1, sizeof(crxa_archive_data), CRXA_G(persist));
		mydata->is_persistent = CRXA_G(persist);

		/* read in archive comment, if any */
		if (CRXA_GET_16(locator.comment_len)) {

			metadata = p + sizeof(locator);

			if (CRXA_GET_16(locator.comment_len) != size - (metadata - buf)) {
				if (error) {
					spprintf(error, 4096, "crxa error: corrupt zip archive, zip file comment truncated in zip-based crxa \"%s\"", fname);
				}
				crx_stream_close(fp);
				pefree(mydata, mydata->is_persistent);
				return FAILURE;
			}

			crxa_parse_metadata_lazy(metadata, &mydata->metadata_tracker, CRXA_GET_16(locator.comment_len), mydata->is_persistent);
		} else {
			ZVAL_UNDEF(&mydata->metadata_tracker.val);
		}

		goto foundit;
	}

	crx_stream_close(fp);

	if (error) {
		spprintf(error, 4096, "crxa error: end of central directory not found in zip-based crxa \"%s\"", fname);
	}

	return FAILURE;
foundit:
	mydata->fname = pestrndup(fname, fname_len, mydata->is_persistent);
#ifdef CRX_WIN32
	crxa_unixify_path_separators(mydata->fname, fname_len);
#endif
	mydata->is_zip = 1;
	mydata->fname_len = fname_len;
	ext = strrchr(mydata->fname, '/');

	if (ext) {
		mydata->ext = memchr(ext, '.', (mydata->fname + fname_len) - ext);
		if (mydata->ext == ext) {
			mydata->ext = memchr(ext + 1, '.', (mydata->fname + fname_len) - ext - 1);
		}
		if (mydata->ext) {
			mydata->ext_len = (mydata->fname + fname_len) - mydata->ext;
		}
	}

	/* clean up on big-endian systems */
	/* seek to central directory */
	crx_stream_seek(fp, CRXA_GET_32(locator.cdir_offset), SEEK_SET);
	/* read in central directory */
	crex_hash_init(&mydata->manifest, CRXA_GET_16(locator.count),
		crex_get_hash_value, destroy_crxa_manifest_entry, (bool)mydata->is_persistent);
	crex_hash_init(&mydata->mounted_dirs, 5,
		crex_get_hash_value, NULL, (bool)mydata->is_persistent);
	crex_hash_init(&mydata->virtual_dirs, CRXA_GET_16(locator.count) * 2,
		crex_get_hash_value, NULL, (bool)mydata->is_persistent);
	entry.crxa = mydata;
	entry.is_zip = 1;
	entry.fp_type = CRXA_FP;
	entry.is_persistent = mydata->is_persistent;
#define CRXA_ZIP_FAIL_FREE(errmsg, save) \
			crex_hash_destroy(&mydata->manifest); \
			HT_INVALIDATE(&mydata->manifest); \
			crex_hash_destroy(&mydata->mounted_dirs); \
			HT_INVALIDATE(&mydata->mounted_dirs); \
			crex_hash_destroy(&mydata->virtual_dirs); \
			HT_INVALIDATE(&mydata->virtual_dirs); \
			crx_stream_close(fp); \
			crxa_metadata_tracker_free(&mydata->metadata_tracker, mydata->is_persistent); \
			if (mydata->signature) { \
				efree(mydata->signature); \
			} \
			if (error) { \
				spprintf(error, 4096, "crxa error: %s in zip-based crxa \"%s\"", errmsg, mydata->fname); \
			} \
			pefree(mydata->fname, mydata->is_persistent); \
			if (mydata->alias) { \
				pefree(mydata->alias, mydata->is_persistent); \
			} \
			pefree(mydata, mydata->is_persistent); \
			efree(save); \
			return FAILURE;
#define CRXA_ZIP_FAIL(errmsg) \
			crex_hash_destroy(&mydata->manifest); \
			HT_INVALIDATE(&mydata->manifest); \
			crex_hash_destroy(&mydata->mounted_dirs); \
			HT_INVALIDATE(&mydata->mounted_dirs); \
			crex_hash_destroy(&mydata->virtual_dirs); \
			HT_INVALIDATE(&mydata->virtual_dirs); \
			crx_stream_close(fp); \
			crxa_metadata_tracker_free(&mydata->metadata_tracker, mydata->is_persistent); \
			if (mydata->signature) { \
				efree(mydata->signature); \
			} \
			if (error) { \
				spprintf(error, 4096, "crxa error: %s in zip-based crxa \"%s\"", errmsg, mydata->fname); \
			} \
			pefree(mydata->fname, mydata->is_persistent); \
			if (mydata->alias) { \
				pefree(mydata->alias, mydata->is_persistent); \
			} \
			pefree(mydata, mydata->is_persistent); \
			return FAILURE;

	/* add each central directory item to the manifest */
	for (i = 0; i < CRXA_GET_16(locator.count); ++i) {
		crxa_zip_central_dir_file zipentry;
		crex_off_t beforeus = crx_stream_tell(fp);

		ZVAL_UNDEF(&entry.metadata_tracker.val);
		entry.metadata_tracker.str = NULL;

		if (sizeof(zipentry) != crx_stream_read(fp, (char *) &zipentry, sizeof(zipentry))) {
			CRXA_ZIP_FAIL("unable to read central directory entry, truncated");
		}

		/* clean up for bigendian systems */
		if (memcmp("PK\1\2", zipentry.signature, 4)) {
			/* corrupted entry */
			CRXA_ZIP_FAIL("corrupted central directory entry, no magic signature");
		}

		if (entry.is_persistent) {
			entry.manifest_pos = i;
		}

		entry.compressed_filesize = CRXA_GET_32(zipentry.compsize);
		entry.uncompressed_filesize = CRXA_GET_32(zipentry.uncompsize);
		entry.crc32 = CRXA_GET_32(zipentry.crc32);
		/* do not CRXA_GET_16 either on the next line */
		entry.timestamp = crxa_zip_d2u_time(zipentry.timestamp, zipentry.datestamp);
		entry.flags = CRXA_ENT_PERM_DEF_FILE;
		entry.header_offset = CRXA_GET_32(zipentry.offset);
		entry.offset = entry.offset_abs = CRXA_GET_32(zipentry.offset) + sizeof(crxa_zip_file_header) + CRXA_GET_16(zipentry.filename_len) +
			CRXA_GET_16(zipentry.extra_len);

		if (CRXA_GET_16(zipentry.flags) & CRXA_ZIP_FLAG_ENCRYPTED) {
			CRXA_ZIP_FAIL("Cannot process encrypted zip files");
		}

		if (!CRXA_GET_16(zipentry.filename_len)) {
			CRXA_ZIP_FAIL("Cannot process zips created from stdin (zero-length filename)");
		}

		entry.filename_len = CRXA_GET_16(zipentry.filename_len);
		entry.filename = (char *) pemalloc(entry.filename_len + 1, entry.is_persistent);

		if (entry.filename_len != crx_stream_read(fp, entry.filename, entry.filename_len)) {
			pefree(entry.filename, entry.is_persistent);
			CRXA_ZIP_FAIL("unable to read in filename from central directory, truncated");
		}

		entry.filename[entry.filename_len] = '\0';

		if (entry.filename[entry.filename_len - 1] == '/') {
			entry.is_dir = 1;
			if(entry.filename_len > 1) {
				entry.filename_len--;
			}
			entry.flags |= CRXA_ENT_PERM_DEF_DIR;
		} else {
			entry.is_dir = 0;
		}

		if (entry.filename_len == sizeof(".crxa/signature.bin")-1 && !strncmp(entry.filename, ".crxa/signature.bin", sizeof(".crxa/signature.bin")-1)) {
			size_t read;
			crx_stream *sigfile;
			char *sig;
			size_t sig_len;

			pefree(entry.filename, entry.is_persistent);

			if (entry.uncompressed_filesize > 0x10000) {
				CRXA_ZIP_FAIL("signatures larger than 64 KiB are not supported");
			}

			sigfile = crx_stream_fopen_tmpfile();
			if (!sigfile) {
				CRXA_ZIP_FAIL("couldn't open temporary file");
			}

			crx_stream_seek(fp, 0, SEEK_SET);
			/* copy file contents + local headers and zip comment, if any, to be hashed for signature */
			crx_stream_copy_to_stream_ex(fp, sigfile, entry.header_offset, NULL);
			/* seek to central directory */
			crx_stream_seek(fp, CRXA_GET_32(locator.cdir_offset), SEEK_SET);
			/* copy central directory header */
			crx_stream_copy_to_stream_ex(fp, sigfile, beforeus - CRXA_GET_32(locator.cdir_offset), NULL);
			if (metadata) {
				crx_stream_write(sigfile, metadata, CRXA_GET_16(locator.comment_len));
			}
			crx_stream_seek(fp, sizeof(crxa_zip_file_header) + entry.header_offset + entry.filename_len + CRXA_GET_16(zipentry.extra_len), SEEK_SET);
			sig = (char *) emalloc(entry.uncompressed_filesize);
			read = crx_stream_read(fp, sig, entry.uncompressed_filesize);
			if (read != entry.uncompressed_filesize || read <= 8) {
				crx_stream_close(sigfile);
				efree(sig);
				CRXA_ZIP_FAIL("signature cannot be read");
			}
			mydata->sig_flags = CRXA_GET_32(sig);
			if (FAILURE == crxa_verify_signature(sigfile, crx_stream_tell(sigfile), mydata->sig_flags, sig + 8, entry.uncompressed_filesize - 8, fname, &mydata->signature, &sig_len, error)) {
				efree(sig);
				if (error) {
					char *save;
					crx_stream_close(sigfile);
					spprintf(&save, 4096, "signature cannot be verified: %s", *error);
					efree(*error);
					CRXA_ZIP_FAIL_FREE(save, save);
				} else {
					crx_stream_close(sigfile);
					CRXA_ZIP_FAIL("signature cannot be verified");
				}
			}
			mydata->sig_len = sig_len;
			crx_stream_close(sigfile);
			efree(sig);
			/* signature checked out, let's ensure this is the last file in the crxa */
			if (i != CRXA_GET_16(locator.count) - 1) {
				CRXA_ZIP_FAIL("entries exist after signature, invalid crxa");
			}

			continue;
		}

		crxa_add_virtual_dirs(mydata, entry.filename, entry.filename_len);

		if (CRXA_GET_16(zipentry.extra_len)) {
			crex_off_t loc = crx_stream_tell(fp);
			if (FAILURE == crxa_zip_process_extra(fp, &entry, CRXA_GET_16(zipentry.extra_len))) {
				pefree(entry.filename, entry.is_persistent);
				CRXA_ZIP_FAIL("Unable to process extra field header for file in central directory");
			}
			crx_stream_seek(fp, loc + CRXA_GET_16(zipentry.extra_len), SEEK_SET);
		}

		switch (CRXA_GET_16(zipentry.compressed)) {
			case CRXA_ZIP_COMP_NONE :
				/* compression flag already set */
				break;
			case CRXA_ZIP_COMP_DEFLATE :
				entry.flags |= CRXA_ENT_COMPRESSED_GZ;
				if (!CRXA_G(has_zlib)) {
					pefree(entry.filename, entry.is_persistent);
					CRXA_ZIP_FAIL("zlib extension is required");
				}
				break;
			case CRXA_ZIP_COMP_BZIP2 :
				entry.flags |= CRXA_ENT_COMPRESSED_BZ2;
				if (!CRXA_G(has_bz2)) {
					pefree(entry.filename, entry.is_persistent);
					CRXA_ZIP_FAIL("bzip2 extension is required");
				}
				break;
			case 1 :
				pefree(entry.filename, entry.is_persistent);
				CRXA_ZIP_FAIL("unsupported compression method (Shrunk) used in this zip");
			case 2 :
			case 3 :
			case 4 :
			case 5 :
				pefree(entry.filename, entry.is_persistent);
				CRXA_ZIP_FAIL("unsupported compression method (Reduce) used in this zip");
			case 6 :
				pefree(entry.filename, entry.is_persistent);
				CRXA_ZIP_FAIL("unsupported compression method (Implode) used in this zip");
			case 7 :
				pefree(entry.filename, entry.is_persistent);
				CRXA_ZIP_FAIL("unsupported compression method (Tokenize) used in this zip");
			case 9 :
				pefree(entry.filename, entry.is_persistent);
				CRXA_ZIP_FAIL("unsupported compression method (Deflate64) used in this zip");
			case 10 :
				pefree(entry.filename, entry.is_persistent);
				CRXA_ZIP_FAIL("unsupported compression method (PKWare Implode/old IBM TERSE) used in this zip");
			case 14 :
				pefree(entry.filename, entry.is_persistent);
				CRXA_ZIP_FAIL("unsupported compression method (LZMA) used in this zip");
			case 18 :
				pefree(entry.filename, entry.is_persistent);
				CRXA_ZIP_FAIL("unsupported compression method (IBM TERSE) used in this zip");
			case 19 :
				pefree(entry.filename, entry.is_persistent);
				CRXA_ZIP_FAIL("unsupported compression method (IBM LZ77) used in this zip");
			case 97 :
				pefree(entry.filename, entry.is_persistent);
				CRXA_ZIP_FAIL("unsupported compression method (WavPack) used in this zip");
			case 98 :
				pefree(entry.filename, entry.is_persistent);
				CRXA_ZIP_FAIL("unsupported compression method (PPMd) used in this zip");
			default :
				pefree(entry.filename, entry.is_persistent);
				CRXA_ZIP_FAIL("unsupported compression method (unknown) used in this zip");
		}

		/* get file metadata */
		if (CRXA_GET_16(zipentry.comment_len)) {
			if (CRXA_GET_16(zipentry.comment_len) != crx_stream_read(fp, buf, CRXA_GET_16(zipentry.comment_len))) {
				pefree(entry.filename, entry.is_persistent);
				CRXA_ZIP_FAIL("unable to read in file comment, truncated");
			}

			p = buf;
			crxa_parse_metadata_lazy(buf, &entry.metadata_tracker, CRXA_GET_16(zipentry.comment_len), entry.is_persistent);
		} else {
			ZVAL_UNDEF(&entry.metadata_tracker.val);
		}

		if (!actual_alias && entry.filename_len == sizeof(".crxa/alias.txt")-1 && !strncmp(entry.filename, ".crxa/alias.txt", sizeof(".crxa/alias.txt")-1)) {
			crx_stream_filter *filter;
			crex_off_t saveloc;
			/* verify local file header */
			crxa_zip_file_header local;

			/* archive alias found */
			saveloc = crx_stream_tell(fp);
			crx_stream_seek(fp, CRXA_GET_32(zipentry.offset), SEEK_SET);

			if (sizeof(local) != crx_stream_read(fp, (char *) &local, sizeof(local))) {
				pefree(entry.filename, entry.is_persistent);
				CRXA_ZIP_FAIL("crxa error: internal corruption of zip-based crxa (cannot read local file header for alias)");
			}

			/* verify local header */
			if (entry.filename_len != CRXA_GET_16(local.filename_len) || entry.crc32 != CRXA_GET_32(local.crc32) || entry.uncompressed_filesize != CRXA_GET_32(local.uncompsize) || entry.compressed_filesize != CRXA_GET_32(local.compsize)) {
				pefree(entry.filename, entry.is_persistent);
				CRXA_ZIP_FAIL("crxa error: internal corruption of zip-based crxa (local header of alias does not match central directory)");
			}

			/* construct actual offset to file start - local extra_len can be different from central extra_len */
			entry.offset = entry.offset_abs =
				sizeof(local) + entry.header_offset + CRXA_GET_16(local.filename_len) + CRXA_GET_16(local.extra_len);
			crx_stream_seek(fp, entry.offset, SEEK_SET);
			/* these next lines should be for crx < 5.2.6 after 5.3 filters are fixed */
			fp->writepos = 0;
			fp->readpos = 0;
			crx_stream_seek(fp, entry.offset, SEEK_SET);
			fp->writepos = 0;
			fp->readpos = 0;
			/* the above lines should be for crx < 5.2.6 after 5.3 filters are fixed */

			mydata->alias_len = entry.uncompressed_filesize;
			if (entry.flags & CRXA_ENT_COMPRESSED_GZ) {
				filter = crx_stream_filter_create("zlib.inflate", NULL, crx_stream_is_persistent(fp));

				if (!filter) {
					pefree(entry.filename, entry.is_persistent);
					CRXA_ZIP_FAIL("unable to decompress alias, zlib filter creation failed");
				}

				crx_stream_filter_append(&fp->readfilters, filter);

				// TODO: refactor to avoid reallocation ???
//???			entry.uncompressed_filesize = crx_stream_copy_to_mem(fp, &actual_alias, entry.uncompressed_filesize, 0)
				{
					crex_string *str = crx_stream_copy_to_mem(fp, entry.uncompressed_filesize, 0);
					if (str) {
						entry.uncompressed_filesize = ZSTR_LEN(str);
						actual_alias = estrndup(ZSTR_VAL(str), ZSTR_LEN(str));
						crex_string_release_ex(str, 0);
					} else {
						actual_alias = NULL;
						entry.uncompressed_filesize = 0;
					}
				}

				if (!entry.uncompressed_filesize || !actual_alias) {
					pefree(entry.filename, entry.is_persistent);
					CRXA_ZIP_FAIL("unable to read in alias, truncated");
				}

				crx_stream_filter_flush(filter, 1);
				crx_stream_filter_remove(filter, 1);

			} else if (entry.flags & CRXA_ENT_COMPRESSED_BZ2) {
				filter = crx_stream_filter_create("bzip2.decompress", NULL, crx_stream_is_persistent(fp));

				if (!filter) {
					pefree(entry.filename, entry.is_persistent);
					CRXA_ZIP_FAIL("unable to read in alias, bzip2 filter creation failed");
				}

				crx_stream_filter_append(&fp->readfilters, filter);

				// TODO: refactor to avoid reallocation ???
//???			entry.uncompressed_filesize = crx_stream_copy_to_mem(fp, &actual_alias, entry.uncompressed_filesize, 0)
				{
					crex_string *str = crx_stream_copy_to_mem(fp, entry.uncompressed_filesize, 0);
					if (str) {
						entry.uncompressed_filesize = ZSTR_LEN(str);
						actual_alias = estrndup(ZSTR_VAL(str), ZSTR_LEN(str));
						crex_string_release_ex(str, 0);
					} else {
						actual_alias = NULL;
						entry.uncompressed_filesize = 0;
					}
				}

				if (!entry.uncompressed_filesize || !actual_alias) {
					pefree(entry.filename, entry.is_persistent);
					CRXA_ZIP_FAIL("unable to read in alias, truncated");
				}

				crx_stream_filter_flush(filter, 1);
				crx_stream_filter_remove(filter, 1);
			} else {
				// TODO: refactor to avoid reallocation ???
//???			entry.uncompressed_filesize = crx_stream_copy_to_mem(fp, &actual_alias, entry.uncompressed_filesize, 0)
				{
					crex_string *str = crx_stream_copy_to_mem(fp, entry.uncompressed_filesize, 0);
					if (str) {
						entry.uncompressed_filesize = ZSTR_LEN(str);
						actual_alias = estrndup(ZSTR_VAL(str), ZSTR_LEN(str));
						crex_string_release_ex(str, 0);
					} else {
						actual_alias = NULL;
						entry.uncompressed_filesize = 0;
					}
				}

				if (!entry.uncompressed_filesize || !actual_alias) {
					pefree(entry.filename, entry.is_persistent);
					CRXA_ZIP_FAIL("unable to read in alias, truncated");
				}
			}

			/* return to central directory parsing */
			crx_stream_seek(fp, saveloc, SEEK_SET);
		}

		crxa_set_inode(&entry);
		crex_hash_str_add_mem(&mydata->manifest, entry.filename, entry.filename_len, (void *)&entry, sizeof(crxa_entry_info));
	}

	if (crex_hash_str_exists(&(mydata->manifest), ".crxa/stub.crx", sizeof(".crxa/stub.crx")-1)) {
		mydata->is_data = 0;
	} else {
		mydata->is_data = 1;
	}

	/* ensure signature set */
	if (!mydata->is_data && CRXA_G(require_hash) && !mydata->signature) {
		CRXA_ZIP_FAIL("signature is missing");
	}

	mydata->fp = fp;

	crex_hash_str_add_ptr(&(CRXA_G(crxa_fname_map)), mydata->fname, fname_len, mydata);

	if (actual_alias) {
		crxa_archive_data *fd_ptr;

		if (!crxa_validate_alias(actual_alias, mydata->alias_len)) {
			if (error) {
				spprintf(error, 4096, "crxa error: invalid alias \"%s\" in zip-based crxa \"%s\"", actual_alias, fname);
			}
			efree(actual_alias);
			crex_hash_str_del(&(CRXA_G(crxa_fname_map)), mydata->fname, fname_len);
			return FAILURE;
		}

		mydata->is_temporary_alias = 0;

		if (NULL != (fd_ptr = crex_hash_str_find_ptr(&(CRXA_G(crxa_alias_map)), actual_alias, mydata->alias_len))) {
			if (SUCCESS != crxa_free_alias(fd_ptr, actual_alias, mydata->alias_len)) {
				if (error) {
					spprintf(error, 4096, "crxa error: Unable to add zip-based crxa \"%s\" with implicit alias, alias is already in use", fname);
				}
				efree(actual_alias);
				crex_hash_str_del(&(CRXA_G(crxa_fname_map)), mydata->fname, fname_len);
				return FAILURE;
			}
		}

		mydata->alias = entry.is_persistent ? pestrndup(actual_alias, mydata->alias_len, 1) : actual_alias;

		if (entry.is_persistent) {
			efree(actual_alias);
		}

		crex_hash_str_add_ptr(&(CRXA_G(crxa_alias_map)), mydata->alias, mydata->alias_len, mydata);
	} else {
		crxa_archive_data *fd_ptr;

		if (alias_len) {
			if (NULL != (fd_ptr = crex_hash_str_find_ptr(&(CRXA_G(crxa_alias_map)), alias, alias_len))) {
				if (SUCCESS != crxa_free_alias(fd_ptr, alias, alias_len)) {
					if (error) {
						spprintf(error, 4096, "crxa error: Unable to add zip-based crxa \"%s\" with explicit alias, alias is already in use", fname);
					}
					crex_hash_str_del(&(CRXA_G(crxa_fname_map)), mydata->fname, fname_len);
					return FAILURE;
				}
			}

			crex_hash_str_add_ptr(&(CRXA_G(crxa_alias_map)), actual_alias, mydata->alias_len, mydata);
			mydata->alias = pestrndup(alias, alias_len, mydata->is_persistent);
			mydata->alias_len = alias_len;
		} else {
			mydata->alias = pestrndup(mydata->fname, fname_len, mydata->is_persistent);
			mydata->alias_len = fname_len;
		}

		mydata->is_temporary_alias = 1;
	}

	if (pcrxa) {
		*pcrxa = mydata;
	}

	return SUCCESS;
}
/* }}} */

/**
 * Create or open a zip-based crxa for writing
 */
int crxa_open_or_create_zip(char *fname, size_t fname_len, char *alias, size_t alias_len, int is_data, uint32_t options, crxa_archive_data** pcrxa, char **error) /* {{{ */
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

	if (crxa->is_zip) {
		return ret;
	}

	if (crxa->is_brandnew) {
		crxa->internal_file_start = 0;
		crxa->is_zip = 1;
		crxa->is_tar = 0;
		return SUCCESS;
	}

	/* we've reached here - the crxa exists and is a regular crxa */
	if (error) {
		spprintf(error, 4096, "crxa zip error: crxa \"%s\" already exists as a regular crxa and must be deleted from disk prior to creating as a zip-based crxa", fname);
	}

	return FAILURE;
}
/* }}} */

struct _crxa_zip_pass {
	crx_stream *filefp;
	crx_stream *centralfp;
	crx_stream *old;
	int free_fp;
	int free_ufp;
	char **error;
};
/* perform final modification of zip contents for each file in the manifest before saving */
static int crxa_zip_changed_apply_int(crxa_entry_info *entry, void *arg) /* {{{ */
{
	crxa_zip_file_header local;
	crxa_zip_unix3 perms;
	crxa_zip_central_dir_file central;
	struct _crxa_zip_pass *p;
	uint32_t newcrc32;
	crex_off_t offset;
	int not_really_modified = 0;
	p = (struct _crxa_zip_pass*) arg;
	uint16_t general_purpose_flags;

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
	memset(&local, 0, sizeof(local));
	memset(&central, 0, sizeof(central));
	memset(&perms, 0, sizeof(perms));
	memcpy(local.signature, "PK\3\4", 4);
	memcpy(central.signature, "PK\1\2", 4);
	CRXA_SET_16(central.extra_len, sizeof(perms));
	CRXA_SET_16(local.extra_len, sizeof(perms));
	perms.tag[0] = 'n';
	perms.tag[1] = 'u';
	CRXA_SET_16(perms.size, sizeof(perms) - 4);
	CRXA_SET_16(perms.perms, entry->flags & CRXA_ENT_PERM_MASK);
	{
		uint32_t crc = (uint32_t) crx_crc32_bulk_init();
		CRC32(crc, perms.perms[0]);
		CRC32(crc, perms.perms[1]);
		CRXA_SET_32(perms.crc32, crx_crc32_bulk_end(crc));
	}

	if (entry->flags & CRXA_ENT_COMPRESSED_GZ) {
		CRXA_SET_16(central.compressed, CRXA_ZIP_COMP_DEFLATE);
		CRXA_SET_16(local.compressed, CRXA_ZIP_COMP_DEFLATE);
	}

	if (entry->flags & CRXA_ENT_COMPRESSED_BZ2) {
		CRXA_SET_16(central.compressed, CRXA_ZIP_COMP_BZIP2);
		CRXA_SET_16(local.compressed, CRXA_ZIP_COMP_BZIP2);
	}

	/* do not use CRXA_GET_16 on either field of the next line */
	crxa_zip_u2d_time(entry->timestamp, local.timestamp, local.datestamp);
	memcpy(central.timestamp, local.timestamp, sizeof(local.timestamp));
	memcpy(central.datestamp, local.datestamp, sizeof(local.datestamp));
	CRXA_SET_16(central.filename_len, entry->filename_len + (entry->is_dir ? 1 : 0));
	CRXA_SET_16(local.filename_len, entry->filename_len + (entry->is_dir ? 1 : 0));
	// set language encoding flag (all filenames have to be UTF-8 anyway)
	general_purpose_flags = CRXA_GET_16(central.flags);
	CRXA_SET_16(central.flags, general_purpose_flags | (1 << 11));
	general_purpose_flags = CRXA_GET_16(local.flags);
	CRXA_SET_16(local.flags, general_purpose_flags | (1 << 11));
	CRXA_SET_32(central.offset, crx_stream_tell(p->filefp));

	/* do extra field for perms later */
	if (entry->is_modified) {
		crx_stream_filter *filter;
		crx_stream *efp;

		if (entry->is_dir) {
			entry->is_modified = 0;
			if (entry->fp_type == CRXA_MOD && entry->fp != entry->crxa->fp && entry->fp != entry->crxa->ufp) {
				crx_stream_close(entry->fp);
				entry->fp = NULL;
				entry->fp_type = CRXA_FP;
			}
			goto continue_dir;
		}

		if (FAILURE == crxa_open_entry_fp(entry, p->error, 0)) {
			spprintf(p->error, 0, "unable to open file contents of file \"%s\" in zip-based crxa \"%s\"", entry->filename, entry->crxa->fname);
			return CREX_HASH_APPLY_STOP;
		}

		/* we can be modified and already be compressed, such as when chmod() is executed */
		if (entry->flags & CRXA_ENT_COMPRESSION_MASK && (entry->old_flags == entry->flags || !entry->old_flags)) {
			not_really_modified = 1;
			goto is_compressed;
		}

		if (-1 == crxa_seek_efp(entry, 0, SEEK_SET, 0, 0)) {
			spprintf(p->error, 0, "unable to seek to start of file \"%s\" to zip-based crxa \"%s\"", entry->filename, entry->crxa->fname);
			return CREX_HASH_APPLY_STOP;
		}

		efp = crxa_get_efp(entry, 0);
		newcrc32 = crx_crc32_bulk_init();

		crx_crc32_stream_bulk_update(&newcrc32, efp, entry->uncompressed_filesize);

		entry->crc32 = crx_crc32_bulk_end(newcrc32);
		CRXA_SET_32(central.uncompsize, entry->uncompressed_filesize);
		CRXA_SET_32(local.uncompsize, entry->uncompressed_filesize);

		if (!(entry->flags & CRXA_ENT_COMPRESSION_MASK)) {
			/* not compressed */
			entry->compressed_filesize = entry->uncompressed_filesize;
			CRXA_SET_32(central.compsize, entry->uncompressed_filesize);
			CRXA_SET_32(local.compsize, entry->uncompressed_filesize);
			goto not_compressed;
		}

		filter = crx_stream_filter_create(crxa_compress_filter(entry, 0), NULL, 0);

		if (!filter) {
			if (entry->flags & CRXA_ENT_COMPRESSED_GZ) {
				spprintf(p->error, 0, "unable to gzip compress file \"%s\" to zip-based crxa \"%s\"", entry->filename, entry->crxa->fname);
			} else {
				spprintf(p->error, 0, "unable to bzip2 compress file \"%s\" to zip-based crxa \"%s\"", entry->filename, entry->crxa->fname);
			}
			return CREX_HASH_APPLY_STOP;
		}

		/* create new file that holds the compressed version */
		/* work around inability to specify freedom in write and strictness
		in read count */
		entry->cfp = crx_stream_fopen_tmpfile();

		if (!entry->cfp) {
			spprintf(p->error, 0, "unable to create temporary file for file \"%s\" while creating zip-based crxa \"%s\"", entry->filename, entry->crxa->fname);
			return CREX_HASH_APPLY_STOP;
		}

		crx_stream_flush(efp);

		if (-1 == crxa_seek_efp(entry, 0, SEEK_SET, 0, 0)) {
			spprintf(p->error, 0, "unable to seek to start of file \"%s\" to zip-based crxa \"%s\"", entry->filename, entry->crxa->fname);
			return CREX_HASH_APPLY_STOP;
		}

		crx_stream_filter_append((&entry->cfp->writefilters), filter);

		if (SUCCESS != crx_stream_copy_to_stream_ex(efp, entry->cfp, entry->uncompressed_filesize, NULL)) {
			spprintf(p->error, 0, "unable to copy compressed file contents of file \"%s\" while creating new crxa \"%s\"", entry->filename, entry->crxa->fname);
			return CREX_HASH_APPLY_STOP;
		}

		crx_stream_filter_flush(filter, 1);
		crx_stream_flush(entry->cfp);
		crx_stream_filter_remove(filter, 1);
		crx_stream_seek(entry->cfp, 0, SEEK_END);
		entry->compressed_filesize = (uint32_t) crx_stream_tell(entry->cfp);
		CRXA_SET_32(central.compsize, entry->compressed_filesize);
		CRXA_SET_32(local.compsize, entry->compressed_filesize);
		/* generate crc on compressed file */
		crx_stream_rewind(entry->cfp);
		entry->old_flags = entry->flags;
		entry->is_modified = 1;
	} else {
is_compressed:
		CRXA_SET_32(central.uncompsize, entry->uncompressed_filesize);
		CRXA_SET_32(local.uncompsize, entry->uncompressed_filesize);
		CRXA_SET_32(central.compsize, entry->compressed_filesize);
		CRXA_SET_32(local.compsize, entry->compressed_filesize);
		if (p->old) {
			if (-1 == crx_stream_seek(p->old, entry->offset_abs, SEEK_SET)) {
				spprintf(p->error, 0, "unable to seek to start of file \"%s\" while creating zip-based crxa \"%s\"", entry->filename, entry->crxa->fname);
				return CREX_HASH_APPLY_STOP;
			}
		}
	}
not_compressed:
	CRXA_SET_32(central.crc32, entry->crc32);
	CRXA_SET_32(local.crc32, entry->crc32);
continue_dir:
	/* set file metadata */
	if (crxa_metadata_tracker_has_data(&entry->metadata_tracker, entry->is_persistent)) {
		crxa_metadata_tracker_try_ensure_has_serialized_data(&entry->metadata_tracker, entry->is_persistent);
		CRXA_SET_16(central.comment_len, entry->metadata_tracker.str ? ZSTR_LEN(entry->metadata_tracker.str) : 0);
	}

	entry->header_offset = crx_stream_tell(p->filefp);
	offset = entry->header_offset + sizeof(local) + entry->filename_len + (entry->is_dir ? 1 : 0) + sizeof(perms);

	if (sizeof(local) != crx_stream_write(p->filefp, (char *)&local, sizeof(local))) {
		spprintf(p->error, 0, "unable to write local file header of file \"%s\" to zip-based crxa \"%s\"", entry->filename, entry->crxa->fname);
		return CREX_HASH_APPLY_STOP;
	}

	if (sizeof(central) != crx_stream_write(p->centralfp, (char *)&central, sizeof(central))) {
		spprintf(p->error, 0, "unable to write central directory entry for file \"%s\" while creating zip-based crxa \"%s\"", entry->filename, entry->crxa->fname);
		return CREX_HASH_APPLY_STOP;
	}

	if (entry->is_dir) {
		if (entry->filename_len != crx_stream_write(p->filefp, entry->filename, entry->filename_len)) {
			spprintf(p->error, 0, "unable to write filename to local directory entry for directory \"%s\" while creating zip-based crxa \"%s\"", entry->filename, entry->crxa->fname);
			return CREX_HASH_APPLY_STOP;
		}

		if (1 != crx_stream_write(p->filefp, "/", 1)) {
			spprintf(p->error, 0, "unable to write filename to local directory entry for directory \"%s\" while creating zip-based crxa \"%s\"", entry->filename, entry->crxa->fname);
			return CREX_HASH_APPLY_STOP;
		}

		if (entry->filename_len != crx_stream_write(p->centralfp, entry->filename, entry->filename_len)) {
			spprintf(p->error, 0, "unable to write filename to central directory entry for directory \"%s\" while creating zip-based crxa \"%s\"", entry->filename, entry->crxa->fname);
			return CREX_HASH_APPLY_STOP;
		}

		if (1 != crx_stream_write(p->centralfp, "/", 1)) {
			spprintf(p->error, 0, "unable to write filename to central directory entry for directory \"%s\" while creating zip-based crxa \"%s\"", entry->filename, entry->crxa->fname);
			return CREX_HASH_APPLY_STOP;
		}
	} else {
		if (entry->filename_len != crx_stream_write(p->filefp, entry->filename, entry->filename_len)) {
			spprintf(p->error, 0, "unable to write filename to local directory entry for file \"%s\" while creating zip-based crxa \"%s\"", entry->filename, entry->crxa->fname);
			return CREX_HASH_APPLY_STOP;
		}

		if (entry->filename_len != crx_stream_write(p->centralfp, entry->filename, entry->filename_len)) {
			spprintf(p->error, 0, "unable to write filename to central directory entry for file \"%s\" while creating zip-based crxa \"%s\"", entry->filename, entry->crxa->fname);
			return CREX_HASH_APPLY_STOP;
		}
	}

	if (sizeof(perms) != crx_stream_write(p->filefp, (char *)&perms, sizeof(perms))) {
		spprintf(p->error, 0, "unable to write local extra permissions file header of file \"%s\" to zip-based crxa \"%s\"", entry->filename, entry->crxa->fname);
		return CREX_HASH_APPLY_STOP;
	}

	if (sizeof(perms) != crx_stream_write(p->centralfp, (char *)&perms, sizeof(perms))) {
		spprintf(p->error, 0, "unable to write central extra permissions file header of file \"%s\" to zip-based crxa \"%s\"", entry->filename, entry->crxa->fname);
		return CREX_HASH_APPLY_STOP;
	}

	if (!not_really_modified && entry->is_modified) {
		if (entry->cfp) {
			if (SUCCESS != crx_stream_copy_to_stream_ex(entry->cfp, p->filefp, entry->compressed_filesize, NULL)) {
				spprintf(p->error, 0, "unable to write compressed contents of file \"%s\" in zip-based crxa \"%s\"", entry->filename, entry->crxa->fname);
				return CREX_HASH_APPLY_STOP;
			}

			crx_stream_close(entry->cfp);
			entry->cfp = NULL;
		} else {
			if (FAILURE == crxa_open_entry_fp(entry, p->error, 0)) {
				return CREX_HASH_APPLY_STOP;
			}

			crxa_seek_efp(entry, 0, SEEK_SET, 0, 0);

			if (SUCCESS != crx_stream_copy_to_stream_ex(crxa_get_efp(entry, 0), p->filefp, entry->uncompressed_filesize, NULL)) {
				spprintf(p->error, 0, "unable to write contents of file \"%s\" in zip-based crxa \"%s\"", entry->filename, entry->crxa->fname);
				return CREX_HASH_APPLY_STOP;
			}
		}

		if (entry->fp_type == CRXA_MOD && entry->fp != entry->crxa->fp && entry->fp != entry->crxa->ufp && entry->fp_refcount == 0) {
			crx_stream_close(entry->fp);
		}

		entry->is_modified = 0;
	} else {
		entry->is_modified = 0;
		if (entry->fp_refcount) {
			/* open file pointers refer to this fp, do not free the stream */
			switch (entry->fp_type) {
				case CRXA_FP:
					p->free_fp = 0;
					break;
				case CRXA_UFP:
					p->free_ufp = 0;
				default:
					break;
			}
		}

		if (!entry->is_dir && entry->compressed_filesize && SUCCESS != crx_stream_copy_to_stream_ex(p->old, p->filefp, entry->compressed_filesize, NULL)) {
			spprintf(p->error, 0, "unable to copy contents of file \"%s\" while creating zip-based crxa \"%s\"", entry->filename, entry->crxa->fname);
			return CREX_HASH_APPLY_STOP;
		}
	}

	entry->fp = NULL;
	entry->offset = entry->offset_abs = offset;
	entry->fp_type = CRXA_FP;

	if (entry->metadata_tracker.str) {
		if (ZSTR_LEN(entry->metadata_tracker.str) != crx_stream_write(p->centralfp, ZSTR_VAL(entry->metadata_tracker.str), ZSTR_LEN(entry->metadata_tracker.str))) {
			spprintf(p->error, 0, "unable to write metadata as file comment for file \"%s\" while creating zip-based crxa \"%s\"", entry->filename, entry->crxa->fname);
			return CREX_HASH_APPLY_STOP;
		}
	}

	return CREX_HASH_APPLY_KEEP;
}
/* }}} */

static int crxa_zip_changed_apply(zval *zv, void *arg) /* {{{ */
{
	return crxa_zip_changed_apply_int(C_PTR_P(zv), arg);
}
/* }}} */

static int crxa_zip_applysignature(crxa_archive_data *crxa, struct _crxa_zip_pass *pass) /* {{{ */
{
	/* add signature for executable tars or tars explicitly set with setSignatureAlgorithm */
	if (!crxa->is_data || crxa->sig_flags) {
		size_t signature_length;
		char *signature, sigbuf[8];
		crxa_entry_info entry = {0};
		crx_stream *newfile;
		crex_off_t tell;

		newfile = crx_stream_fopen_tmpfile();
		if (newfile == NULL) {
			spprintf(pass->error, 0, "crxa error: unable to create temporary file for the signature file");
			return FAILURE;
		}
		tell = crx_stream_tell(pass->filefp);
		/* copy the local files, central directory, and the zip comment to generate the hash */
		crx_stream_seek(pass->filefp, 0, SEEK_SET);
		crx_stream_copy_to_stream_ex(pass->filefp, newfile, tell, NULL);
		tell = crx_stream_tell(pass->centralfp);
		crx_stream_seek(pass->centralfp, 0, SEEK_SET);
		crx_stream_copy_to_stream_ex(pass->centralfp, newfile, tell, NULL);
		if (crxa->metadata_tracker.str) {
			crx_stream_write(newfile, ZSTR_VAL(crxa->metadata_tracker.str), ZSTR_LEN(crxa->metadata_tracker.str));
		}

		if (FAILURE == crxa_create_signature(crxa, newfile, &signature, &signature_length, pass->error)) {
			if (pass->error) {
				char *save = *(pass->error);
				spprintf(pass->error, 0, "crxa error: unable to write signature to zip-based crxa: %s", save);
				efree(save);
			}

			crx_stream_close(newfile);
			return FAILURE;
		}

		entry.filename = ".crxa/signature.bin";
		entry.filename_len = sizeof(".crxa/signature.bin")-1;
		entry.fp = crx_stream_fopen_tmpfile();
		entry.fp_type = CRXA_MOD;
		entry.is_modified = 1;
		if (entry.fp == NULL) {
			spprintf(pass->error, 0, "crxa error: unable to create temporary file for signature");
			return FAILURE;
		}

		CRXA_SET_32(sigbuf, crxa->sig_flags);
		CRXA_SET_32(sigbuf + 4, signature_length);

		if (C_UL(8) != crx_stream_write(entry.fp, sigbuf, 8) || signature_length != crx_stream_write(entry.fp, signature, signature_length)) {
			efree(signature);
			if (pass->error) {
				spprintf(pass->error, 0, "crxa error: unable to write signature to zip-based crxa %s", crxa->fname);
			}

			crx_stream_close(newfile);
			return FAILURE;
		}

		efree(signature);
		entry.uncompressed_filesize = entry.compressed_filesize = signature_length + 8;
		entry.crxa = crxa;
		/* throw out return value and write the signature */
		crxa_zip_changed_apply_int(&entry, (void *)pass);
		crx_stream_close(newfile);

		if (pass->error && *(pass->error)) {
			/* error is set by writeheaders */
			return FAILURE;
		}
	} /* signature */
	return SUCCESS;
}
/* }}} */

int crxa_zip_flush(crxa_archive_data *crxa, char *user_stub, crex_long len, int defaultstub, char **error) /* {{{ */
{
	char *pos;
	static const char newstub[] = "<?crx // zip-based crxa archive stub file\n__HALT_COMPILER();";
	char halt_stub[] = "__HALT_COMPILER();";

	crx_stream *stubfile, *oldfile;
	int free_user_stub, closeoldfile = 0;
	crxa_entry_info entry = {0};
	char *temperr = NULL;
	struct _crxa_zip_pass pass;
	crxa_zip_dir_end eocd;
	uint32_t cdir_size, cdir_offset;

	pass.error = &temperr;
	entry.flags = CRXA_ENT_PERM_DEF_FILE;
	entry.timestamp = time(NULL);
	entry.is_modified = 1;
	entry.is_zip = 1;
	entry.crxa = crxa;
	entry.fp_type = CRXA_MOD;

	if (crxa->is_persistent) {
		if (error) {
			spprintf(error, 0, "internal error: attempt to flush cached zip-based crxa \"%s\"", crxa->fname);
		}
		return EOF;
	}

	if (crxa->is_data) {
		goto nostub;
	}

	/* set alias */
	if (!crxa->is_temporary_alias && crxa->alias_len) {
		entry.fp = crx_stream_fopen_tmpfile();
		if (entry.fp == NULL) {
			spprintf(error, 0, "crxa error: unable to create temporary file");
			return EOF;
		}
		if (crxa->alias_len != crx_stream_write(entry.fp, crxa->alias, crxa->alias_len)) {
			if (error) {
				spprintf(error, 0, "unable to set alias in zip-based crxa \"%s\"", crxa->fname);
			}
			return EOF;
		}

		entry.uncompressed_filesize = entry.compressed_filesize = crxa->alias_len;
		entry.filename = estrndup(".crxa/alias.txt", sizeof(".crxa/alias.txt")-1);
		entry.filename_len = sizeof(".crxa/alias.txt")-1;

		crex_hash_str_update_mem(&crxa->manifest, entry.filename, entry.filename_len, (void*)&entry, sizeof(crxa_entry_info));
	} else {
		crex_hash_str_del(&crxa->manifest, ".crxa/alias.txt", sizeof(".crxa/alias.txt")-1);
	}

	/* register alias */
	if (crxa->alias_len) {
		if (FAILURE == crxa_get_archive(&crxa, crxa->fname, crxa->fname_len, crxa->alias, crxa->alias_len, error)) {
			return EOF;
		}
	}

	/* set stub */
	if (user_stub && !defaultstub) {
		if (len < 0) {
			/* resource passed in */
			if (!(crx_stream_from_zval_no_verify(stubfile, (zval *)user_stub))) {
				if (error) {
					spprintf(error, 0, "unable to access resource to copy stub to new zip-based crxa \"%s\"", crxa->fname);
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
					spprintf(error, 0, "unable to read resource to copy stub to new zip-based crxa \"%s\"", crxa->fname);
				}
				return EOF;
			}
			free_user_stub = 1;
		} else {
			free_user_stub = 0;
		}

		if ((pos = crx_stristr(user_stub, halt_stub, len, sizeof(halt_stub) - 1)) == NULL) {
			if (error) {
				spprintf(error, 0, "illegal stub for zip-based crxa \"%s\"", crxa->fname);
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
				spprintf(error, 0, "unable to create stub from string in new zip-based crxa \"%s\"", crxa->fname);
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
				spprintf(error, 0, "unable to %s stub in%szip-based crxa \"%s\", failed", user_stub ? "overwrite" : "create", user_stub ? " " : " new ", crxa->fname);
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
						spprintf(error, 0, "unable to create stub in zip-based crxa \"%s\"", crxa->fname);
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

	/* save modified files to the zip */
	pass.old = oldfile;
	pass.filefp = crx_stream_fopen_tmpfile();

	if (!pass.filefp) {
fperror:
		if (closeoldfile) {
			crx_stream_close(oldfile);
		}
		if (error) {
			spprintf(error, 4096, "crxa zip flush of \"%s\" failed: unable to open temporary file", crxa->fname);
		}
		return EOF;
	}

	pass.centralfp = crx_stream_fopen_tmpfile();

	if (!pass.centralfp) {
		goto fperror;
	}

	pass.free_fp = pass.free_ufp = 1;
	memset(&eocd, 0, sizeof(eocd));

	memcpy(eocd.signature, "PK\5\6", 4);
	if (!crxa->is_data && !crxa->sig_flags) {
		crxa->sig_flags = CRXA_SIG_SHA256;
	}
	if (crxa->sig_flags) {
		CRXA_SET_16(eocd.counthere, crex_hash_num_elements(&crxa->manifest) + 1);
		CRXA_SET_16(eocd.count, crex_hash_num_elements(&crxa->manifest) + 1);
	} else {
		CRXA_SET_16(eocd.counthere, crex_hash_num_elements(&crxa->manifest));
		CRXA_SET_16(eocd.count, crex_hash_num_elements(&crxa->manifest));
	}
	crex_hash_apply_with_argument(&crxa->manifest, crxa_zip_changed_apply, (void *) &pass);

	crxa_metadata_tracker_try_ensure_has_serialized_data(&crxa->metadata_tracker, crxa->is_persistent);
	if (temperr) {
		if (error) {
			spprintf(error, 4096, "crxa zip flush of \"%s\" failed: %s", crxa->fname, temperr);
		}
		efree(temperr);
temperror:
		crx_stream_close(pass.centralfp);
nocentralerror:
		crx_stream_close(pass.filefp);
		if (closeoldfile) {
			crx_stream_close(oldfile);
		}
		return EOF;
	}

	if (FAILURE == crxa_zip_applysignature(crxa, &pass)) {
		goto temperror;
	}

	/* save zip */
	cdir_size = crx_stream_tell(pass.centralfp);
	cdir_offset = crx_stream_tell(pass.filefp);
	CRXA_SET_32(eocd.cdir_size, cdir_size);
	CRXA_SET_32(eocd.cdir_offset, cdir_offset);
	crx_stream_seek(pass.centralfp, 0, SEEK_SET);

	{
		size_t clen;
		int ret = crx_stream_copy_to_stream_ex(pass.centralfp, pass.filefp, CRX_STREAM_COPY_ALL, &clen);
		if (SUCCESS != ret || clen != cdir_size) {
			if (error) {
				spprintf(error, 4096, "crxa zip flush of \"%s\" failed: unable to write central-directory", crxa->fname);
			}
			goto temperror;
		}
	}

	crx_stream_close(pass.centralfp);

	crxa_metadata_tracker_try_ensure_has_serialized_data(&crxa->metadata_tracker, crxa->is_persistent);
	if (crxa->metadata_tracker.str) {
		/* set crxa metadata */
		CRXA_SET_16(eocd.comment_len, ZSTR_LEN(crxa->metadata_tracker.str));

		if (sizeof(eocd) != crx_stream_write(pass.filefp, (char *)&eocd, sizeof(eocd))) {
			if (error) {
				spprintf(error, 4096, "crxa zip flush of \"%s\" failed: unable to write end of central-directory", crxa->fname);
			}
			goto nocentralerror;
		}

		if (ZSTR_LEN(crxa->metadata_tracker.str) != crx_stream_write(pass.filefp, ZSTR_VAL(crxa->metadata_tracker.str), ZSTR_LEN(crxa->metadata_tracker.str))) {
			if (error) {
				spprintf(error, 4096, "crxa zip flush of \"%s\" failed: unable to write metadata to zip comment", crxa->fname);
			}
			goto nocentralerror;
		}
	} else {
		if (sizeof(eocd) != crx_stream_write(pass.filefp, (char *)&eocd, sizeof(eocd))) {
			if (error) {
				spprintf(error, 4096, "crxa zip flush of \"%s\" failed: unable to write end of central-directory", crxa->fname);
			}
			goto nocentralerror;
		}
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

	/* re-open */
	crxa->is_brandnew = 0;

	if (crxa->donotflush) {
		/* deferred flush */
		crxa->fp = pass.filefp;
	} else {
		crxa->fp = crx_stream_open_wrapper(crxa->fname, "w+b", IGNORE_URL|STREAM_MUST_SEEK|REPORT_ERRORS, NULL);
		if (!crxa->fp) {
			if (closeoldfile) {
				crx_stream_close(oldfile);
			}
			crxa->fp = pass.filefp;
			if (error) {
				spprintf(error, 4096, "unable to open new crxa \"%s\" for writing", crxa->fname);
			}
			return EOF;
		}
		crx_stream_rewind(pass.filefp);
		crx_stream_copy_to_stream_ex(pass.filefp, crxa->fp, CRX_STREAM_COPY_ALL, NULL);
		/* we could also reopen the file in "rb" mode but there is no need for that */
		crx_stream_close(pass.filefp);
	}

	if (closeoldfile) {
		crx_stream_close(oldfile);
	}
	return EOF;
}
/* }}} */
