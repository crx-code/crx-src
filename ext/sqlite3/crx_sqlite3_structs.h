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
   | Authors: Scott MacVicar <scottmac@crx.net>                           |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_SQLITE_STRUCTS_H
#define CRX_SQLITE_STRUCTS_H

#include <sqlite3.h>

/* for backwards compatibility reasons */
#ifndef SQLITE_OPEN_READONLY
#define SQLITE_OPEN_READONLY 0x00000001
#endif

#ifndef SQLITE_OPEN_READWRITE
#define SQLITE_OPEN_READWRITE 0x00000002
#endif

#ifndef SQLITE_OPEN_CREATE
#define SQLITE_OPEN_CREATE 0x00000004
#endif

/* Structure for SQLite Statement Parameter. */
struct crx_sqlite3_bound_param  {
	crex_long param_number;
	crex_string *name;
	crex_long type;
	zval parameter;
};

/* Structure for SQLite function. */
typedef struct _crx_sqlite3_func {
	struct _crx_sqlite3_func *next;

	const char *func_name;
	int argc;

	crex_fcall_info_cache func;
	crex_fcall_info_cache step;
	crex_fcall_info_cache fini;
} crx_sqlite3_func;

/* Structure for SQLite collation function */
typedef struct _crx_sqlite3_collation {
	struct _crx_sqlite3_collation *next;

	const char *collation_name;
	crex_fcall_info_cache cmp_func;
} crx_sqlite3_collation;

/* Structure for SQLite Database object. */
typedef struct _crx_sqlite3_db_object  {
	int initialised;
	sqlite3 *db;
	crx_sqlite3_func *funcs;
	crx_sqlite3_collation *collations;
	crex_fcall_info_cache authorizer_fcc;

	bool exception;

	crex_llist free_list;
	crex_object zo;
} crx_sqlite3_db_object;

static inline crx_sqlite3_db_object *crx_sqlite3_db_from_obj(crex_object *obj) {
	return (crx_sqlite3_db_object*)((char*)(obj) - XtOffsetOf(crx_sqlite3_db_object, zo));
}

#define C_SQLITE3_DB_P(zv)  crx_sqlite3_db_from_obj(C_OBJ_P((zv)))

/* Structure for SQLite Database object. */
typedef struct _crx_sqlite3_agg_context  {
	zval zval_context;
	crex_long row_count;
} crx_sqlite3_agg_context;

typedef struct _crx_sqlite3_stmt_object crx_sqlite3_stmt;
typedef struct _crx_sqlite3_result_object crx_sqlite3_result;

/* sqlite3 objects to be destroyed */
typedef struct _crx_sqlite3_free_list {
	zval stmt_obj_zval;
	crx_sqlite3_stmt *stmt_obj;
} crx_sqlite3_free_list;

/* Structure for SQLite Result object. */
struct _crx_sqlite3_result_object  {
	crx_sqlite3_db_object *db_obj;
	crx_sqlite3_stmt *stmt_obj;
	zval stmt_obj_zval;

	/* Cache of column names to speed up repeated fetchArray(SQLITE3_ASSOC) calls.
	 * Cache is cleared on reset() and finalize() calls. */
	int column_count;
	crex_string **column_names;

	int is_prepared_statement;
	crex_object zo;
};

static inline crx_sqlite3_result *crx_sqlite3_result_from_obj(crex_object *obj) {
	return (crx_sqlite3_result*)((char*)(obj) - XtOffsetOf(crx_sqlite3_result, zo));
}

#define C_SQLITE3_RESULT_P(zv)  crx_sqlite3_result_from_obj(C_OBJ_P((zv)))

/* Structure for SQLite Statement object. */
struct _crx_sqlite3_stmt_object  {
	sqlite3_stmt *stmt;
	crx_sqlite3_db_object *db_obj;
	zval db_obj_zval;

	int initialised;

	/* Keep track of the zvals for bound parameters */
	HashTable *bound_params;
	crex_object zo;
};

static inline crx_sqlite3_stmt *crx_sqlite3_stmt_from_obj(crex_object *obj) {
	return (crx_sqlite3_stmt*)((char*)(obj) - XtOffsetOf(crx_sqlite3_stmt, zo));
}

#define C_SQLITE3_STMT_P(zv)  crx_sqlite3_stmt_from_obj(C_OBJ_P((zv)))

#endif
