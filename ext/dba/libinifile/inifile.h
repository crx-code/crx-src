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

#ifndef CRX_LIB_INIFILE_H
#define CRX_LIB_INIFILE_H

typedef struct {
	char *group;
	char *name;
} key_type;

typedef struct {
	char *value;
} val_type;

typedef struct {
	key_type key;
	val_type val;
	size_t pos;
} line_type;

typedef struct {
	char *lockfn;
	int lockfd;
	crx_stream *fp;
	int readonly;
	line_type curr;
	line_type next;
} inifile;

val_type inifile_fetch(inifile *dba, const key_type *key, int skip);
int inifile_firstkey(inifile *dba);
int inifile_nextkey(inifile *dba);
int inifile_delete(inifile *dba, const key_type *key);
int inifile_delete_ex(inifile *dba, const key_type *key, bool *found);
int inifile_replace(inifile *dba, const key_type *key, const val_type *val);
int inifile_replace_ex(inifile *dba, const key_type *key, const val_type *val, bool *found);
int inifile_append(inifile *dba, const key_type *key, const val_type *val);
const char *inifile_version(void);

key_type inifile_key_split(const char *group_name);
char * inifile_key_string(const key_type *key);

void inifile_key_free(key_type *key);
void inifile_val_free(val_type *val);
void inifile_line_free(line_type *ln);

inifile * inifile_alloc(crx_stream *fp, int readonly, int persistent);
void inifile_free(inifile *dba, int persistent);

#endif
