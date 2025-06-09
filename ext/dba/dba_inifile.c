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
   | Author: Marcus Boerger <helly@crx.net>                               |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"

#ifdef DBA_INIFILE
#include "crx_inifile.h"

#include "libinifile/inifile.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

DBA_OPEN_FUNC(inifile)
{
	info->dbf = inifile_alloc(info->fp, info->mode == DBA_READER, info->flags&DBA_PERSISTENT);

	return info->dbf ? SUCCESS : FAILURE;
}

DBA_CLOSE_FUNC(inifile)
{
	inifile *dba = info->dbf;

	inifile_free(dba, info->flags&DBA_PERSISTENT);
}

DBA_FETCH_FUNC(inifile)
{
	inifile *dba = info->dbf;
	val_type ini_val;
	key_type ini_key;
	crex_string *fetched_val = NULL;

	if (!key) {
		crx_error_docref(NULL, E_WARNING, "No key specified");
		return 0;
	}
	ini_key = inifile_key_split(ZSTR_VAL(key)); /* keylen not needed here */

	ini_val = inifile_fetch(dba, &ini_key, skip);
	inifile_key_free(&ini_key);
	if (ini_val.value) {
		fetched_val = crex_string_init(ini_val.value, strlen(ini_val.value), /* persistent */ false);
		inifile_val_free(&ini_val);
	}
	return fetched_val;
}

DBA_UPDATE_FUNC(inifile)
{
	inifile *dba = info->dbf;
	val_type ini_val;
	int res;
	key_type ini_key;

	if (!key) {
		crx_error_docref(NULL, E_WARNING, "No key specified");
		return 0;
	}
	ini_key = inifile_key_split(ZSTR_VAL(key)); /* keylen not needed here */

	ini_val.value = ZSTR_VAL(val);

	if (mode == 1) {
		res = inifile_append(dba, &ini_key, &ini_val);
	} else {
		res = inifile_replace(dba, &ini_key, &ini_val);
	}
	inifile_key_free(&ini_key);
	switch(res) {
	case -1:
		// TODO Check when this happens and confirm this can even happen
		crx_error_docref(NULL, E_WARNING, "Operation not possible");
		return FAILURE;
	default:
	case 0:
		return SUCCESS;
	case 1:
		return FAILURE;
	}
}

DBA_EXISTS_FUNC(inifile)
{
	inifile *dba = info->dbf;
	val_type ini_val;
	key_type ini_key;

	if (!key) {
		crx_error_docref(NULL, E_WARNING, "No key specified");
		return 0;
	}
	ini_key = inifile_key_split(ZSTR_VAL(key)); /* keylen not needed here */

	ini_val = inifile_fetch(dba, &ini_key, 0);
	inifile_key_free(&ini_key);
	if (ini_val.value) {
		inifile_val_free(&ini_val);
		return SUCCESS;
	}
	return FAILURE;
}

DBA_DELETE_FUNC(inifile)
{
	inifile *dba = info->dbf;
	int res;
	bool found = 0;
	key_type ini_key;

	if (!key) {
		crx_error_docref(NULL, E_WARNING, "No key specified");
		return 0;
	}
	ini_key = inifile_key_split(ZSTR_VAL(key)); /* keylen not needed here */

	res =  inifile_delete_ex(dba, &ini_key, &found);

	inifile_key_free(&ini_key);
	return (res == -1 || !found ? FAILURE : SUCCESS);
}

DBA_FIRSTKEY_FUNC(inifile)
{
	inifile *dba = info->dbf;

	if (inifile_firstkey(dba)) {
		char *result = inifile_key_string(&dba->curr.key);
		crex_string *key = crex_string_init(result, strlen(result), /* persistent */ false);
		efree(result);
		return key;
	} else {
		return NULL;
	}
}

DBA_NEXTKEY_FUNC(inifile)
{
	inifile *dba = info->dbf;

	if (!dba->curr.key.group && !dba->curr.key.name) {
		return NULL;
	}

	if (inifile_nextkey(dba)) {
		char *result = inifile_key_string(&dba->curr.key);
		crex_string *key = crex_string_init(result, strlen(result), /* persistent */ false);
		efree(result);
		return key;
	} else {
		return NULL;
	}
}

DBA_OPTIMIZE_FUNC(inifile)
{
	/* dummy */
	return SUCCESS;
}

DBA_SYNC_FUNC(inifile)
{
	/* dummy */
	return SUCCESS;
}

DBA_INFO_FUNC(inifile)
{
	return estrdup(inifile_version());
}

#endif
