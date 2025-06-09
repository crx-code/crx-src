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
   | Authors: Marcus Boerger <helly@crx.net>                              |
   | based on ext/db/db.c by:                                             |
   |          Rasmus Lerdorf <rasmus@crx.net>                             |
   |          Jim Winstead <jimw@crx.net>                                 |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "crx_globals.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "flatfile.h"

#define FLATFILE_BLOCK_SIZE 1024

/*
 * ret = -1 means that database was opened for read-only
 * ret = 0  success
 * ret = 1  key already exists - nothing done
 */

/* {{{ flatfile_store */
int flatfile_store(flatfile *dba, datum key_datum, datum value_datum, int mode) {
	if (mode == FLATFILE_INSERT) {
		if (flatfile_findkey(dba, key_datum)) {
			return 1;
		}
		crx_stream_seek(dba->fp, 0L, SEEK_END);
		crx_stream_printf(dba->fp, "%zu\n", key_datum.dsize);
		crx_stream_flush(dba->fp);
		if (crx_stream_write(dba->fp, key_datum.dptr, key_datum.dsize) < key_datum.dsize) {
			return -1;
		}
		crx_stream_printf(dba->fp, "%zu\n", value_datum.dsize);
		crx_stream_flush(dba->fp);
		if (crx_stream_write(dba->fp, value_datum.dptr, value_datum.dsize) < value_datum.dsize) {
			return -1;
		}
	} else { /* FLATFILE_REPLACE */
		flatfile_delete(dba, key_datum);
		crx_stream_printf(dba->fp, "%zu\n", key_datum.dsize);
		crx_stream_flush(dba->fp);
		if (crx_stream_write(dba->fp, key_datum.dptr, key_datum.dsize) < key_datum.dsize) {
			return -1;
		}
		crx_stream_printf(dba->fp, "%zu\n", value_datum.dsize);
		if (crx_stream_write(dba->fp, value_datum.dptr, value_datum.dsize) < value_datum.dsize) {
			return -1;
		}
	}

	crx_stream_flush(dba->fp);
	return 0;
}
/* }}} */

/* {{{ flatfile_fetch */
datum flatfile_fetch(flatfile *dba, datum key_datum) {
	datum value_datum = {NULL, 0};
	char buf[16];

	if (flatfile_findkey(dba, key_datum)) {
		if (crx_stream_gets(dba->fp, buf, sizeof(buf))) {
			value_datum.dsize = atoi(buf);
			value_datum.dptr = safe_emalloc(value_datum.dsize, 1, 1);
			value_datum.dsize = crx_stream_read(dba->fp, value_datum.dptr, value_datum.dsize);
		} else {
			value_datum.dptr = NULL;
			value_datum.dsize = 0;
		}
	}
	return value_datum;
}
/* }}} */

/* {{{ flatfile_delete */
int flatfile_delete(flatfile *dba, datum key_datum) {
	char *key = key_datum.dptr;
	size_t size = key_datum.dsize;
	size_t buf_size = FLATFILE_BLOCK_SIZE;
	char *buf = emalloc(buf_size);
	size_t num;
	size_t pos;

	crx_stream_rewind(dba->fp);
	while(!crx_stream_eof(dba->fp)) {
		/* read in the length of the key name */
		if (!crx_stream_gets(dba->fp, buf, 15)) {
			break;
		}
		num = atoi(buf);
		if (num >= buf_size) {
			buf_size = num + FLATFILE_BLOCK_SIZE;
			buf = erealloc(buf, buf_size);
		}
		pos = crx_stream_tell(dba->fp);

		/* read in the key name */
		num = crx_stream_read(dba->fp, buf, num);

		if (size == num && !memcmp(buf, key, size)) {
			crx_stream_seek(dba->fp, pos, SEEK_SET);
			crx_stream_putc(dba->fp, 0);
			crx_stream_flush(dba->fp);
			crx_stream_seek(dba->fp, 0L, SEEK_END);
			efree(buf);
			return SUCCESS;
		}

		/* read in the length of the value */
		if (!crx_stream_gets(dba->fp, buf, 15)) {
			break;
		}
		num = atoi(buf);
		if (num >= buf_size) {
			buf_size = num + FLATFILE_BLOCK_SIZE;
			buf = erealloc(buf, buf_size);
		}
		/* read in the value */
		num = crx_stream_read(dba->fp, buf, num);
	}
	efree(buf);
	return FAILURE;
}
/* }}} */

/* {{{ flatfile_findkey */
int flatfile_findkey(flatfile *dba, datum key_datum) {
	size_t buf_size = FLATFILE_BLOCK_SIZE;
	char *buf = emalloc(buf_size);
	size_t num;
	int ret=0;
	void *key = key_datum.dptr;
	size_t size = key_datum.dsize;

	crx_stream_rewind(dba->fp);
	while (!crx_stream_eof(dba->fp)) {
		if (!crx_stream_gets(dba->fp, buf, 15)) {
			break;
		}
		num = atoi(buf);
		if (num >= buf_size) {
			buf_size = num + FLATFILE_BLOCK_SIZE;
			buf = erealloc(buf, buf_size);
		}
		num = crx_stream_read(dba->fp, buf, num);

		if (size == num) {
			if (!memcmp(buf, key, size)) {
				ret = 1;
				break;
			}
		}
		if (!crx_stream_gets(dba->fp, buf, 15)) {
			break;
		}
		num = atoi(buf);
		if (num >= buf_size) {
			buf_size = num + FLATFILE_BLOCK_SIZE;
			buf = erealloc(buf, buf_size);
		}
		num = crx_stream_read(dba->fp, buf, num);
	}
	efree(buf);
	return ret;
}
/* }}} */

/* {{{ flatfile_firstkey */
datum flatfile_firstkey(flatfile *dba) {
	datum res;
	size_t num;
	size_t buf_size = FLATFILE_BLOCK_SIZE;
	char *buf = emalloc(buf_size);

	crx_stream_rewind(dba->fp);
	while(!crx_stream_eof(dba->fp)) {
		if (!crx_stream_gets(dba->fp, buf, 15)) {
			break;
		}
		num = atoi(buf);
		if (num >= buf_size) {
			buf_size = num + FLATFILE_BLOCK_SIZE;
			buf = erealloc(buf, buf_size);
		}
		num = crx_stream_read(dba->fp, buf, num);

		if (*(buf) != 0) {
			dba->CurrentFlatFilePos = crx_stream_tell(dba->fp);
			res.dptr = buf;
			res.dsize = num;
			return res;
		}
		if (!crx_stream_gets(dba->fp, buf, 15)) {
			break;
		}
		num = atoi(buf);
		if (num >= buf_size) {
			buf_size = num + FLATFILE_BLOCK_SIZE;
			buf = erealloc(buf, buf_size);
		}
		num = crx_stream_read(dba->fp, buf, num);
	}
	efree(buf);
	res.dptr = NULL;
	res.dsize = 0;
	return res;
}
/* }}} */

/* {{{ flatfile_nextkey */
datum flatfile_nextkey(flatfile *dba) {
	datum res;
	size_t num;
	size_t buf_size = FLATFILE_BLOCK_SIZE;
	char *buf = emalloc(buf_size);

	crx_stream_seek(dba->fp, dba->CurrentFlatFilePos, SEEK_SET);
	while(!crx_stream_eof(dba->fp)) {
		if (!crx_stream_gets(dba->fp, buf, 15)) {
			break;
		}
		num = atoi(buf);
		if (num >= buf_size) {
			buf_size = num + FLATFILE_BLOCK_SIZE;
			buf = erealloc(buf, buf_size);
		}
		num = crx_stream_read(dba->fp, buf, num);

		if (!crx_stream_gets(dba->fp, buf, 15)) {
			break;
		}
		num = atoi(buf);
		if (num >= buf_size) {
			buf_size = num + FLATFILE_BLOCK_SIZE;
			buf = erealloc(buf, buf_size);
		}
		num = crx_stream_read(dba->fp, buf, num);

		if (*(buf)!=0) {
			dba->CurrentFlatFilePos = crx_stream_tell(dba->fp);
			res.dptr = buf;
			res.dsize = num;
			return res;
		}
	}
	efree(buf);
	res.dptr = NULL;
	res.dsize = 0;
	return res;
}
/* }}} */

/* {{{ flatfile_version */
const char *flatfile_version(void)
{
	return "1.0, $Id$";
}
/* }}} */
