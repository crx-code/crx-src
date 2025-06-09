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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "crx_ini.h"
#include "ext/standard/info.h"
#include "crx_sqlite3.h"
#include "crx_sqlite3_structs.h"
#include "main/SAPI.h"

#include <sqlite3.h>

#include "crex_exceptions.h"
#include "SAPI.h"
#include "sqlite3_arginfo.h"

CREX_DECLARE_MODULE_GLOBALS(sqlite3)

static CRX_GINIT_FUNCTION(sqlite3);
static int crx_sqlite3_authorizer(void *autharg, int action, const char *arg1, const char *arg2, const char *arg3, const char *arg4);
static void sqlite3_param_dtor(zval *data);
static int crx_sqlite3_compare_stmt_zval_free(crx_sqlite3_free_list **free_list, zval *statement);

#define SQLITE3_CHECK_INITIALIZED(db_obj, member, class_name) \
	if (!(db_obj) || !(member)) { \
		crex_throw_error(NULL, "The " #class_name " object has not been correctly initialised or is already closed"); \
		RETURN_THROWS(); \
	}

#define SQLITE3_CHECK_INITIALIZED_STMT(member, class_name) \
	if (!(member)) { \
		crex_throw_error(NULL, "The " #class_name " object has not been correctly initialised or is already closed"); \
		RETURN_THROWS(); \
	}

/* {{{ CRX_INI */
CRX_INI_BEGIN()
	STD_CRX_INI_ENTRY("sqlite3.extension_dir",  NULL, CRX_INI_SYSTEM, OnUpdateString, extension_dir, crex_sqlite3_globals, sqlite3_globals)
#if SQLITE_VERSION_NUMBER >= 3026000
	STD_CRX_INI_BOOLEAN("sqlite3.defensive",  "1", CRX_INI_USER, OnUpdateBool, dbconfig_defensive, crex_sqlite3_globals, sqlite3_globals)
#endif
CRX_INI_END()
/* }}} */

/* Handlers */
static crex_object_handlers sqlite3_object_handlers;
static crex_object_handlers sqlite3_stmt_object_handlers;
static crex_object_handlers sqlite3_result_object_handlers;

/* Class entries */
crex_class_entry *crx_sqlite3_exception_ce;
crex_class_entry *crx_sqlite3_sc_entry;
crex_class_entry *crx_sqlite3_stmt_entry;
crex_class_entry *crx_sqlite3_result_entry;

/* {{{ Error Handler */
static void crx_sqlite3_error(crx_sqlite3_db_object *db_obj, int errcode, const char *format, ...)
{
	va_list arg;
	char 	*message;

	va_start(arg, format);
	vspprintf(&message, 0, format, arg);
	va_end(arg);

	if (db_obj && db_obj->exception) {
		crex_throw_exception(crx_sqlite3_exception_ce, message, errcode);
	} else {
		crx_error_docref(NULL, E_WARNING, "%s", message);
	}

	if (message) {
		efree(message);
	}
}
/* }}} */

/* {{{ Opens a SQLite 3 Database, if the build includes encryption then it will attempt to use the key. */
CRX_METHOD(SQLite3, open)
{
	crx_sqlite3_db_object *db_obj;
	zval *object = CREX_THIS;
	char *filename, *encryption_key, *fullpath;
	size_t filename_len, encryption_key_len = 0;
	crex_long flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
	int rc;

	db_obj = C_SQLITE3_DB_P(object);

	if (FAILURE == crex_parse_parameters(CREX_NUM_ARGS(), "p|ls", &filename, &filename_len, &flags, &encryption_key, &encryption_key_len)) {
		RETURN_THROWS();
	}

	if (db_obj->initialised) {
		crex_throw_exception(crex_ce_exception, "Already initialised DB Object", 0);
		RETURN_THROWS();
	}

	if (filename_len != 0 && (filename_len != sizeof(":memory:")-1 ||
			memcmp(filename, ":memory:", sizeof(":memory:")-1) != 0)) {
		if (!(fullpath = expand_filepath(filename, NULL))) {
			crex_throw_exception(crex_ce_exception, "Unable to expand filepath", 0);
			RETURN_THROWS();
		}

		if (crx_check_open_basedir(fullpath)) {
			crex_throw_exception_ex(crex_ce_exception, 0, "open_basedir prohibits opening %s", fullpath);
			efree(fullpath);
			RETURN_THROWS();
		}
	} else {
		/* filename equals "" or ":memory:" */
		fullpath = filename;
	}

	rc = sqlite3_open_v2(fullpath, &(db_obj->db), flags, NULL);
	if (rc != SQLITE_OK) {
		crex_throw_exception_ex(crex_ce_exception, 0, "Unable to open database: %s",
#ifdef HAVE_SQLITE3_ERRSTR
				db_obj->db ? sqlite3_errmsg(db_obj->db) : sqlite3_errstr(rc));
#else
				db_obj->db ? sqlite3_errmsg(db_obj->db) : "");
#endif
		sqlite3_close(db_obj->db);
		if (fullpath != filename) {
			efree(fullpath);
		}
		return;
	}

#ifdef SQLITE_HAS_CODEC
	if (encryption_key_len > 0) {
		if (sqlite3_key(db_obj->db, encryption_key, encryption_key_len) != SQLITE_OK) {
			crex_throw_exception_ex(crex_ce_exception, 0, "Unable to open database: %s", sqlite3_errmsg(db_obj->db));
			sqlite3_close(db_obj->db);
			RETURN_THROWS();
		}
	}
#endif

	db_obj->initialised = 1;
	db_obj->authorizer_fcc = empty_fcall_info_cache;

	sqlite3_set_authorizer(db_obj->db, crx_sqlite3_authorizer, db_obj);

#if SQLITE_VERSION_NUMBER >= 3026000
	if (SQLITE3G(dbconfig_defensive)) {
		sqlite3_db_config(db_obj->db, SQLITE_DBCONFIG_DEFENSIVE, 1, NULL);
	}
#endif

	if (fullpath != filename) {
		efree(fullpath);
	}
}
/* }}} */

/* {{{ Close a SQLite 3 Database. */
CRX_METHOD(SQLite3, close)
{
	crx_sqlite3_db_object *db_obj;
	zval *object = CREX_THIS;
	int errcode;
	db_obj = C_SQLITE3_DB_P(object);

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if (db_obj->initialised) {
		crex_llist_clean(&(db_obj->free_list));
		if(db_obj->db) {
			errcode = sqlite3_close(db_obj->db);
			if (errcode != SQLITE_OK) {
				crx_sqlite3_error(db_obj, errcode, "Unable to close database: %s", sqlite3_errmsg(db_obj->db));
				RETURN_FALSE;
			}
		}
		db_obj->initialised = 0;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Executes a result-less query against a given database. */
CRX_METHOD(SQLite3, exec)
{
	crx_sqlite3_db_object *db_obj;
	zval *object = CREX_THIS;
	int errcode;
	crex_string *sql;
	char *errtext = NULL;
	db_obj = C_SQLITE3_DB_P(object);

	if (FAILURE == crex_parse_parameters(CREX_NUM_ARGS(), "S", &sql)) {
		RETURN_THROWS();
	}

	SQLITE3_CHECK_INITIALIZED(db_obj, db_obj->initialised, SQLite3)

	errcode = sqlite3_exec(db_obj->db, ZSTR_VAL(sql), NULL, NULL, &errtext);

	if (errcode != SQLITE_OK) {
		crx_sqlite3_error(db_obj, errcode, "%s", errtext);
		sqlite3_free(errtext);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Returns the SQLite3 Library version as a string constant and as a number. */
CRX_METHOD(SQLite3, version)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	array_init(return_value);

	add_assoc_string(return_value, "versionString", (char*)sqlite3_libversion());
	add_assoc_long(return_value, "versionNumber", sqlite3_libversion_number());

	return;
}
/* }}} */

/* {{{ Returns the rowid of the most recent INSERT into the database from the database connection. */
CRX_METHOD(SQLite3, lastInsertRowID)
{
	crx_sqlite3_db_object *db_obj;
	zval *object = CREX_THIS;
	db_obj = C_SQLITE3_DB_P(object);

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	SQLITE3_CHECK_INITIALIZED(db_obj, db_obj->initialised, SQLite3)

	RETURN_LONG((crex_long) sqlite3_last_insert_rowid(db_obj->db));
}
/* }}} */

/* {{{ Returns the numeric result code of the most recent failed sqlite API call for the database connection. */
CRX_METHOD(SQLite3, lastErrorCode)
{
	crx_sqlite3_db_object *db_obj;
	zval *object = CREX_THIS;
	db_obj = C_SQLITE3_DB_P(object);

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	SQLITE3_CHECK_INITIALIZED(db_obj, db_obj->db, SQLite3)

	if (db_obj->initialised) {
		RETURN_LONG(sqlite3_errcode(db_obj->db));
	} else {
		RETURN_LONG(0);
	}
}
/* }}} */

/* {{{ Returns the numeric extended result code of the most recent failed sqlite API call for the database connection. */
CRX_METHOD(SQLite3, lastExtendedErrorCode)
{
	crx_sqlite3_db_object *db_obj;
	zval *object = CREX_THIS;
	db_obj = C_SQLITE3_DB_P(object);

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	SQLITE3_CHECK_INITIALIZED(db_obj, db_obj->db, SQLite3)

	if (db_obj->initialised) {
		RETURN_LONG(sqlite3_extended_errcode(db_obj->db));
	} else {
		RETURN_LONG(0);
	}
}
/* }}} */

/* {{{ Turns on or off the extended result codes feature of SQLite. */
CRX_METHOD(SQLite3, enableExtendedResultCodes)
{
	crx_sqlite3_db_object *db_obj;
	zval *object = CREX_THIS;
	bool enable = 1;
	db_obj = C_SQLITE3_DB_P(object);
	int ret;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|b", &enable) == FAILURE) {
		RETURN_THROWS();
	}

	SQLITE3_CHECK_INITIALIZED(db_obj, db_obj->db, SQLite3)

	if (db_obj->initialised) {
		ret = sqlite3_extended_result_codes(db_obj->db, enable ? 1 : 0);
		if (ret == SQLITE_OK)
		{
			RETURN_TRUE;
		}
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ Returns english text describing the most recent failed sqlite API call for the database connection. */
CRX_METHOD(SQLite3, lastErrorMsg)
{
	crx_sqlite3_db_object *db_obj;
	zval *object = CREX_THIS;
	db_obj = C_SQLITE3_DB_P(object);

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	SQLITE3_CHECK_INITIALIZED(db_obj, db_obj->db, SQLite3)

	if (db_obj->initialised) {
		RETURN_STRING((char *)sqlite3_errmsg(db_obj->db));
	} else {
		RETURN_EMPTY_STRING();
	}
}
/* }}} */

/* {{{ Sets a busy handler that will sleep until database is not locked or timeout is reached. Passing a value less than or equal to zero turns off all busy handlers. */
CRX_METHOD(SQLite3, busyTimeout)
{
	crx_sqlite3_db_object *db_obj;
	zval *object = CREX_THIS;
	crex_long ms;
#ifdef SQLITE_ENABLE_API_ARMOR
	int return_code;
#endif
	db_obj = C_SQLITE3_DB_P(object);

	if (FAILURE == crex_parse_parameters(CREX_NUM_ARGS(), "l", &ms)) {
		RETURN_THROWS();
	}

	SQLITE3_CHECK_INITIALIZED(db_obj, db_obj->initialised, SQLite3)

#ifdef SQLITE_ENABLE_API_ARMOR
	return_code = sqlite3_busy_timeout(db_obj->db, ms);
	if (return_code != SQLITE_OK) {
		crx_sqlite3_error(db_obj, sqlite3_errcode(db_obj->db), "Unable to set busy timeout: %s", sqlite3_errmsg(db_obj->db));
		RETURN_FALSE;
	}
#else
	crx_ignore_value(sqlite3_busy_timeout(db_obj->db, ms));
#endif

	RETURN_TRUE;
}
/* }}} */


#ifndef SQLITE_OMIT_LOAD_EXTENSION
/* {{{ Attempts to load an SQLite extension library. */
CRX_METHOD(SQLite3, loadExtension)
{
	crx_sqlite3_db_object *db_obj;
	zval *object = CREX_THIS;
	char *extension, *lib_path, *extension_dir, *errtext = NULL;
	char fullpath[MAXPATHLEN];
	size_t extension_len, extension_dir_len;
	db_obj = C_SQLITE3_DB_P(object);

	if (FAILURE == crex_parse_parameters(CREX_NUM_ARGS(), "s", &extension, &extension_len)) {
		RETURN_THROWS();
	}

	SQLITE3_CHECK_INITIALIZED(db_obj, db_obj->initialised, SQLite3)

#ifdef ZTS
	if ((strncmp(sapi_module.name, "cgi", 3) != 0) &&
		(strcmp(sapi_module.name, "cli") != 0) &&
		(strncmp(sapi_module.name, "embed", 5) != 0)
	) {		crx_sqlite3_error(db_obj, 0, "Not supported in multithreaded Web servers");
		RETURN_FALSE;
	}
#endif

	if (!SQLITE3G(extension_dir)) {
		crx_sqlite3_error(db_obj, 0, "SQLite Extensions are disabled");
		RETURN_FALSE;
	}

	if (extension_len == 0) {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_FALSE;
	}

	extension_dir = SQLITE3G(extension_dir);
	extension_dir_len = strlen(SQLITE3G(extension_dir));

	if (IS_SLASH(extension_dir[extension_dir_len-1])) {
		spprintf(&lib_path, 0, "%s%s", extension_dir, extension);
	} else {
		spprintf(&lib_path, 0, "%s%c%s", extension_dir, DEFAULT_SLASH, extension);
	}

	if (!VCWD_REALPATH(lib_path, fullpath)) {
		crx_sqlite3_error(db_obj, 0, "Unable to load extension at '%s'", lib_path);
		efree(lib_path);
		RETURN_FALSE;
	}

	efree(lib_path);

	if (strncmp(fullpath, extension_dir, extension_dir_len) != 0) {
		crx_sqlite3_error(db_obj, 0, "Unable to open extensions outside the defined directory");
		RETURN_FALSE;
	}

	/* Extension loading should only be enabled for when we attempt to load */
	sqlite3_enable_load_extension(db_obj->db, 1);
	if (sqlite3_load_extension(db_obj->db, fullpath, 0, &errtext) != SQLITE_OK) {
		crx_sqlite3_error(db_obj, 0, "%s", errtext);
		sqlite3_free(errtext);
		sqlite3_enable_load_extension(db_obj->db, 0);
		RETURN_FALSE;
	}
	sqlite3_enable_load_extension(db_obj->db, 0);

	RETURN_TRUE;
}
/* }}} */
#endif

/* {{{ Returns the number of database rows that were changed (or inserted or deleted) by the most recent SQL statement. */
CRX_METHOD(SQLite3, changes)
{
	crx_sqlite3_db_object *db_obj;
	zval *object = CREX_THIS;
	db_obj = C_SQLITE3_DB_P(object);

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	SQLITE3_CHECK_INITIALIZED(db_obj, db_obj->initialised, SQLite3)

	RETURN_LONG(sqlite3_changes(db_obj->db));
}
/* }}} */

/* {{{ Returns a string that has been properly escaped. */
CRX_METHOD(SQLite3, escapeString)
{
	crex_string *sql;
	char *ret;

	if (FAILURE == crex_parse_parameters(CREX_NUM_ARGS(), "S", &sql)) {
		RETURN_THROWS();
	}

	if (ZSTR_LEN(sql)) {
		ret = sqlite3_mprintf("%q", ZSTR_VAL(sql));
		if (ret) {
			RETVAL_STRING(ret);
			sqlite3_free(ret);
		}
	} else {
		RETURN_EMPTY_STRING();
	}
}
/* }}} */

/* {{{ Returns a prepared SQL statement for execution. */
CRX_METHOD(SQLite3, prepare)
{
	crx_sqlite3_db_object *db_obj;
	crx_sqlite3_stmt *stmt_obj;
	zval *object = CREX_THIS;
	crex_string *sql;
	int errcode;
	crx_sqlite3_free_list *free_item;

	db_obj = C_SQLITE3_DB_P(object);

	if (FAILURE == crex_parse_parameters(CREX_NUM_ARGS(), "S", &sql)) {
		RETURN_THROWS();
	}

	SQLITE3_CHECK_INITIALIZED(db_obj, db_obj->initialised, SQLite3)

	if (!ZSTR_LEN(sql)) {
		RETURN_FALSE;
	}

	object_init_ex(return_value, crx_sqlite3_stmt_entry);
	stmt_obj = C_SQLITE3_STMT_P(return_value);
	stmt_obj->db_obj = db_obj;
	ZVAL_OBJ_COPY(&stmt_obj->db_obj_zval, C_OBJ_P(object));

	errcode = sqlite3_prepare_v2(db_obj->db, ZSTR_VAL(sql), ZSTR_LEN(sql), &(stmt_obj->stmt), NULL);
	if (errcode != SQLITE_OK) {
		crx_sqlite3_error(db_obj, errcode, "Unable to prepare statement: %s", sqlite3_errmsg(db_obj->db));
		zval_ptr_dtor(return_value);
		RETURN_FALSE;
	}

	stmt_obj->initialised = 1;

	free_item = emalloc(sizeof(crx_sqlite3_free_list));
	free_item->stmt_obj = stmt_obj;
	ZVAL_OBJ(&free_item->stmt_obj_zval, C_OBJ_P(return_value));

	crex_llist_add_element(&(db_obj->free_list), &free_item);
}
/* }}} */

/* {{{ Returns true or false, for queries that return data it will return a SQLite3Result object. */
CRX_METHOD(SQLite3, query)
{
	crx_sqlite3_db_object *db_obj;
	crx_sqlite3_result *result;
	crx_sqlite3_stmt *stmt_obj;
	zval *object = CREX_THIS;
	zval stmt;
	crex_string *sql;
	char *errtext = NULL;
	int return_code;
	db_obj = C_SQLITE3_DB_P(object);

	if (FAILURE == crex_parse_parameters(CREX_NUM_ARGS(), "S", &sql)) {
		RETURN_THROWS();
	}

	SQLITE3_CHECK_INITIALIZED(db_obj, db_obj->initialised, SQLite3)

	if (!ZSTR_LEN(sql)) {
		RETURN_FALSE;
	}

	/* If there was no return value then just execute the query */
	if (!USED_RET()) {
		int errcode;
		errcode = sqlite3_exec(db_obj->db, ZSTR_VAL(sql), NULL, NULL, &errtext);
		if (errcode != SQLITE_OK) {
			crx_sqlite3_error(db_obj, errcode, "%s", errtext);
			sqlite3_free(errtext);
		}
		RETURN_FALSE;
	}

	object_init_ex(&stmt, crx_sqlite3_stmt_entry);
	stmt_obj = C_SQLITE3_STMT_P(&stmt);
	stmt_obj->db_obj = db_obj;
	ZVAL_OBJ_COPY(&stmt_obj->db_obj_zval, C_OBJ_P(object));

	return_code = sqlite3_prepare_v2(db_obj->db, ZSTR_VAL(sql), ZSTR_LEN(sql), &(stmt_obj->stmt), NULL);
	if (return_code != SQLITE_OK) {
		crx_sqlite3_error(db_obj, return_code, "Unable to prepare statement: %s", sqlite3_errmsg(db_obj->db));
		zval_ptr_dtor(&stmt);
		RETURN_FALSE;
	}

	stmt_obj->initialised = 1;

	object_init_ex(return_value, crx_sqlite3_result_entry);
	result = C_SQLITE3_RESULT_P(return_value);
	result->db_obj = db_obj;
	result->stmt_obj = stmt_obj;
	result->column_names = NULL;
	result->column_count = -1;
	ZVAL_OBJ(&result->stmt_obj_zval, C_OBJ(stmt));

	return_code = sqlite3_step(result->stmt_obj->stmt);

	switch (return_code) {
		case SQLITE_ROW: /* Valid Row */
		case SQLITE_DONE: /* Valid but no results */
		{
			crx_sqlite3_free_list *free_item;
			free_item = emalloc(sizeof(crx_sqlite3_free_list));
			free_item->stmt_obj = stmt_obj;
			free_item->stmt_obj_zval = stmt;
			crex_llist_add_element(&(db_obj->free_list), &free_item);
			sqlite3_reset(result->stmt_obj->stmt);
			break;
		}
		default:
			if (!EG(exception)) {
				crx_sqlite3_error(db_obj, sqlite3_errcode(db_obj->db), "Unable to execute statement: %s", sqlite3_errmsg(db_obj->db));
			}
			sqlite3_finalize(stmt_obj->stmt);
			stmt_obj->initialised = 0;
			zval_ptr_dtor(return_value);
			RETURN_FALSE;
	}
}
/* }}} */

static void sqlite_value_to_zval(sqlite3_stmt *stmt, int column, zval *data) /* {{{ */
{
	sqlite3_int64 val;

	switch (sqlite3_column_type(stmt, column)) {
		case SQLITE_INTEGER:
			val = sqlite3_column_int64(stmt, column);
#if LONG_MAX <= 2147483647
			if (val > CREX_LONG_MAX || val < CREX_LONG_MIN) {
				ZVAL_STRINGL(data, (char *)sqlite3_column_text(stmt, column), sqlite3_column_bytes(stmt, column));
			} else {
#endif
				ZVAL_LONG(data, (crex_long) val);
#if LONG_MAX <= 2147483647
			}
#endif
			break;

		case SQLITE_FLOAT:
			ZVAL_DOUBLE(data, sqlite3_column_double(stmt, column));
			break;

		case SQLITE_NULL:
			ZVAL_NULL(data);
			break;

		case SQLITE3_TEXT:
			ZVAL_STRING(data, (char*)sqlite3_column_text(stmt, column));
			break;

		case SQLITE_BLOB:
		default:
			ZVAL_STRINGL(data, (char*)sqlite3_column_blob(stmt, column), sqlite3_column_bytes(stmt, column));
	}
}
/* }}} */

/* {{{ Returns a string of the first column, or an array of the entire row. */
CRX_METHOD(SQLite3, querySingle)
{
	crx_sqlite3_db_object *db_obj;
	zval *object = CREX_THIS;
	crex_string *sql;
	char *errtext = NULL;
	int return_code;
	bool entire_row = 0;
	sqlite3_stmt *stmt;
	db_obj = C_SQLITE3_DB_P(object);

	if (FAILURE == crex_parse_parameters(CREX_NUM_ARGS(), "S|b", &sql, &entire_row)) {
		RETURN_THROWS();
	}

	SQLITE3_CHECK_INITIALIZED(db_obj, db_obj->initialised, SQLite3)

	if (!ZSTR_LEN(sql)) {
		RETURN_FALSE;
	}

	/* If there was no return value then just execute the query */
	if (!USED_RET()) {
		int errcode;
		errcode = sqlite3_exec(db_obj->db, ZSTR_VAL(sql), NULL, NULL, &errtext);
		if (errcode != SQLITE_OK) {
			crx_sqlite3_error(db_obj, errcode, "%s", errtext);
			sqlite3_free(errtext);
		}
		RETURN_FALSE;
	}

	return_code = sqlite3_prepare_v2(db_obj->db, ZSTR_VAL(sql), ZSTR_LEN(sql), &stmt, NULL);
	if (return_code != SQLITE_OK) {
		crx_sqlite3_error(db_obj, return_code, "Unable to prepare statement: %s", sqlite3_errmsg(db_obj->db));
		RETURN_FALSE;
	}

	return_code = sqlite3_step(stmt);

	switch (return_code) {
		case SQLITE_ROW: /* Valid Row */
		{
			if (!entire_row) {
				sqlite_value_to_zval(stmt, 0, return_value);
			} else {
				int i = 0;
				array_init(return_value);
				for (i = 0; i < sqlite3_data_count(stmt); i++) {
					zval data;
					sqlite_value_to_zval(stmt, i, &data);
					add_assoc_zval(return_value, (char*)sqlite3_column_name(stmt, i), &data);
				}
			}
			break;
		}
		case SQLITE_DONE: /* Valid but no results */
		{
			if (!entire_row) {
				RETVAL_NULL();
			} else {
				RETVAL_EMPTY_ARRAY();
			}
			break;
		}
		default:
		if (!EG(exception)) {
			crx_sqlite3_error(db_obj, sqlite3_errcode(db_obj->db), "Unable to execute statement: %s", sqlite3_errmsg(db_obj->db));
		}
		RETVAL_FALSE;
	}
	sqlite3_finalize(stmt);
}
/* }}} */

static int sqlite3_do_callback(crex_fcall_info_cache *fcc, uint32_t argc, sqlite3_value **argv, sqlite3_context *context, int is_agg) /* {{{ */
{
	zval *zargs = NULL;
	zval retval;
	uint32_t i;
	uint32_t fake_argc;
	crex_result ret = SUCCESS;
	crx_sqlite3_agg_context *agg_context = NULL;

	if (is_agg) {
		is_agg = 2;
	}

	fake_argc = argc + is_agg;

	/* build up the params */
	if (fake_argc) {
		zargs = (zval *)safe_emalloc(fake_argc, sizeof(zval), 0);
	}

	if (is_agg) {
		/* summon the aggregation context */
		agg_context = (crx_sqlite3_agg_context *)sqlite3_aggregate_context(context, sizeof(crx_sqlite3_agg_context));

		if (C_ISUNDEF(agg_context->zval_context)) {
			ZVAL_NULL(&agg_context->zval_context);
		}
		ZVAL_COPY(&zargs[0], &agg_context->zval_context);
		ZVAL_LONG(&zargs[1], agg_context->row_count);
	}

	for (i = 0; i < argc; i++) {
		switch (sqlite3_value_type(argv[i])) {
			case SQLITE_INTEGER:
#if CREX_LONG_MAX > 2147483647
				ZVAL_LONG(&zargs[i + is_agg], sqlite3_value_int64(argv[i]));
#else
				ZVAL_LONG(&zargs[i + is_agg], sqlite3_value_int(argv[i]));
#endif
				break;

			case SQLITE_FLOAT:
				ZVAL_DOUBLE(&zargs[i + is_agg], sqlite3_value_double(argv[i]));
				break;

			case SQLITE_NULL:
				ZVAL_NULL(&zargs[i + is_agg]);
				break;

			case SQLITE_BLOB:
			case SQLITE3_TEXT:
			default:
				ZVAL_STRINGL(&zargs[i + is_agg], (char*)sqlite3_value_text(argv[i]), sqlite3_value_bytes(argv[i]));
				break;
		}
	}

	crex_call_known_fcc(fcc, &retval, fake_argc, zargs, /* named_params */ NULL);

	/* clean up the params */
	if (is_agg) {
		zval_ptr_dtor(&zargs[0]);
		zval_ptr_dtor(&zargs[1]);
	}
	if (fake_argc) {
		for (i = is_agg; i < argc + is_agg; i++) {
			zval_ptr_dtor(&zargs[i]);
		}
		efree(zargs);
	}

	if (!is_agg || !argv) {
		/* only set the sqlite return value if we are a scalar function,
		 * or if we are finalizing an aggregate */
		if (!C_ISUNDEF(retval)) {
			switch (C_TYPE(retval)) {
				case IS_LONG:
#if CREX_LONG_MAX > 2147483647
					sqlite3_result_int64(context, C_LVAL(retval));
#else
					sqlite3_result_int(context, C_LVAL(retval));
#endif
					break;

				case IS_NULL:
					sqlite3_result_null(context);
					break;

				case IS_DOUBLE:
					sqlite3_result_double(context, C_DVAL(retval));
					break;

				default: {
					crex_string *str = zval_try_get_string(&retval);
					if (UNEXPECTED(!str)) {
						ret = FAILURE;
						break;
					}
					sqlite3_result_text(context, ZSTR_VAL(str), ZSTR_LEN(str), SQLITE_TRANSIENT);
					crex_string_release(str);
					break;
				}
			}
		} else {
			sqlite3_result_error(context, "failed to invoke callback", 0);
		}

		if (agg_context && !C_ISUNDEF(agg_context->zval_context)) {
			zval_ptr_dtor(&agg_context->zval_context);
		}
	} else {
		/* we're stepping in an aggregate; the return value goes into
		 * the context */
		if (agg_context && !C_ISUNDEF(agg_context->zval_context)) {
			zval_ptr_dtor(&agg_context->zval_context);
		}
		ZVAL_COPY_VALUE(&agg_context->zval_context, &retval);
		ZVAL_UNDEF(&retval);
	}

	if (!C_ISUNDEF(retval)) {
		zval_ptr_dtor(&retval);
	}
	return ret;
}
/* }}}*/

static void crx_sqlite3_callback_func(sqlite3_context *context, int argc, sqlite3_value **argv) /* {{{ */
{
	crx_sqlite3_func *func = (crx_sqlite3_func *)sqlite3_user_data(context);

	sqlite3_do_callback(&func->func, argc, argv, context, 0);
}
/* }}}*/

static void crx_sqlite3_callback_step(sqlite3_context *context, int argc, sqlite3_value **argv) /* {{{ */
{
	crx_sqlite3_func *func = (crx_sqlite3_func *)sqlite3_user_data(context);
	crx_sqlite3_agg_context *agg_context = (crx_sqlite3_agg_context *)sqlite3_aggregate_context(context, sizeof(crx_sqlite3_agg_context));

	agg_context->row_count++;

	sqlite3_do_callback(&func->step, argc, argv, context, 1);
}
/* }}} */

static void crx_sqlite3_callback_final(sqlite3_context *context) /* {{{ */
{
	crx_sqlite3_func *func = (crx_sqlite3_func *)sqlite3_user_data(context);
	crx_sqlite3_agg_context *agg_context = (crx_sqlite3_agg_context *)sqlite3_aggregate_context(context, sizeof(crx_sqlite3_agg_context));

	agg_context->row_count = 0;

	sqlite3_do_callback(&func->fini, 0, NULL, context, 1);
}
/* }}} */

static int crx_sqlite3_callback_compare(void *coll, int a_len, const void *a, int b_len, const void* b) /* {{{ */
{
	crx_sqlite3_collation *collation = (crx_sqlite3_collation*)coll;
	zval zargs[2];
	zval retval;
	int ret = 0;

	// Exception occurred on previous callback. Don't attempt to call function.
	if (EG(exception)) {
		return 0;
	}

	ZVAL_STRINGL(&zargs[0], a, a_len);
	ZVAL_STRINGL(&zargs[1], b, b_len);

	crex_call_known_fcc(&collation->cmp_func, &retval, /* argc */ 2, zargs, /* named_params */ NULL);

	zval_ptr_dtor(&zargs[0]);
	zval_ptr_dtor(&zargs[1]);

	if (EG(exception)) {
		ret = 0;
	} else if (C_TYPE(retval) != IS_LONG){
		//retval ought to contain a ZVAL_LONG by now
		// (the result of a comparison, i.e. most likely -1, 0, or 1)
		//I suppose we could accept any scalar return type, though.
		crx_sqlite3_error(NULL, 0, "An error occurred while invoking the compare callback (invalid return type).  Collation behaviour is undefined.");
	} else {
		ret = C_LVAL(retval);
	}

	zval_ptr_dtor(&retval);

	return ret;
}
/* }}} */

/* {{{ Allows registration of a CRX function as a SQLite UDF that can be called within SQL statements. */
CRX_METHOD(SQLite3, createFunction)
{
	crx_sqlite3_db_object *db_obj;
	zval *object = CREX_THIS;
	crx_sqlite3_func *func;
	char *sql_func;
	size_t sql_func_len;
	crex_fcall_info fci;
	crex_fcall_info_cache fcc;
	crex_long sql_func_num_args = -1;
	crex_long flags = 0;
	db_obj = C_SQLITE3_DB_P(object);

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sf|ll", &sql_func, &sql_func_len, &fci, &fcc, &sql_func_num_args, &flags) == FAILURE) {
		RETURN_THROWS();
	}

	SQLITE3_CHECK_INITIALIZED(db_obj, db_obj->initialised, SQLite3)

	if (!sql_func_len) {
		RETURN_FALSE;
	}

	func = (crx_sqlite3_func *)ecalloc(1, sizeof(*func));

	if (sqlite3_create_function(db_obj->db, sql_func, sql_func_num_args, flags | SQLITE_UTF8, func, crx_sqlite3_callback_func, NULL, NULL) == SQLITE_OK) {
		func->func_name = estrdup(sql_func);

		if (!CREX_FCC_INITIALIZED(fcc)) {
			crex_is_callable_ex(&fci.function_name, NULL, IS_CALLABLE_SUPPRESS_DEPRECATIONS, NULL, &fcc, NULL);
			/* Call trampoline has been cleared by zpp. Refetch it, because we want to deal
			 * with it outselves. It is important that it is not refetched on every call,
			 * because calls may occur from different scopes. */
		}
		crex_fcc_dup(&func->func, &fcc);

		func->argc = sql_func_num_args;
		func->next = db_obj->funcs;
		db_obj->funcs = func;

		RETURN_TRUE;
	}
	efree(func);

	RETURN_FALSE;
}
/* }}} */

/* {{{ Allows registration of a CRX function for use as an aggregate. */
CRX_METHOD(SQLite3, createAggregate)
{
	crx_sqlite3_db_object *db_obj;
	zval *object = CREX_THIS;
	crx_sqlite3_func *func;
	char *sql_func;
	size_t sql_func_len;
	crex_fcall_info step_fci, fini_fci;
	crex_fcall_info_cache step_fcc, fini_fcc;
	crex_long sql_func_num_args = -1;
	db_obj = C_SQLITE3_DB_P(object);

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sff|l", &sql_func, &sql_func_len, &step_fci, &step_fcc, &fini_fci, &fini_fcc, &sql_func_num_args) == FAILURE) {
		RETURN_THROWS();
	}

	SQLITE3_CHECK_INITIALIZED(db_obj, db_obj->initialised, SQLite3)

	if (!sql_func_len) {
		RETURN_FALSE;
	}

	func = (crx_sqlite3_func *)ecalloc(1, sizeof(*func));

	if (sqlite3_create_function(db_obj->db, sql_func, sql_func_num_args, SQLITE_UTF8, func, NULL, crx_sqlite3_callback_step, crx_sqlite3_callback_final) == SQLITE_OK) {
		func->func_name = estrdup(sql_func);

		if (!CREX_FCC_INITIALIZED(step_fcc)) {
			/* Call trampoline has been cleared by zpp. Refetch it, because we want to deal
			 * with it outselves. It is important that it is not refetched on every call,
			 * because calls may occur from different scopes. */
			crex_is_callable_ex(&step_fci.function_name, NULL, IS_CALLABLE_SUPPRESS_DEPRECATIONS, NULL, &step_fcc, NULL);
		}
		crex_fcc_dup(&func->step, &step_fcc);
		if (!CREX_FCC_INITIALIZED(fini_fcc)) {
			/* Call trampoline has been cleared by zpp. Refetch it, because we want to deal
			 * with it outselves. It is important that it is not refetched on every call,
			 * because calls may occur from different scopes. */
			crex_is_callable_ex(&fini_fci.function_name, NULL, IS_CALLABLE_SUPPRESS_DEPRECATIONS, NULL, &fini_fcc, NULL);
		}
		crex_fcc_dup(&func->fini, &fini_fcc);

		func->argc = sql_func_num_args;
		func->next = db_obj->funcs;
		db_obj->funcs = func;

		RETURN_TRUE;
	}
	efree(func);

	RETURN_FALSE;
}
/* }}} */

/* {{{ Registers a CRX function as a comparator that can be used with the SQL COLLATE operator. Callback must accept two strings and return an integer (as strcmp()). */
CRX_METHOD(SQLite3, createCollation)
{
	crx_sqlite3_db_object *db_obj;
	zval *object = CREX_THIS;
	crx_sqlite3_collation *collation;
	char *collation_name;
	size_t collation_name_len;
	crex_fcall_info fci;
	crex_fcall_info_cache fcc;
	db_obj = C_SQLITE3_DB_P(object);

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sf", &collation_name, &collation_name_len, &fci, &fcc) == FAILURE) {
		RETURN_THROWS();
	}

	SQLITE3_CHECK_INITIALIZED(db_obj, db_obj->initialised, SQLite3)

	if (!collation_name_len) {
		RETURN_FALSE;
	}

	collation = (crx_sqlite3_collation *)ecalloc(1, sizeof(*collation));
	if (sqlite3_create_collation(db_obj->db, collation_name, SQLITE_UTF8, collation, crx_sqlite3_callback_compare) == SQLITE_OK) {
		collation->collation_name = estrdup(collation_name);

		if (!CREX_FCC_INITIALIZED(fcc)) {
			/* Call trampoline has been cleared by zpp. Refetch it, because we want to deal
			 * with it outselves. It is important that it is not refetched on every call,
			 * because calls may occur from different scopes. */
			crex_is_callable_ex(&fci.function_name, NULL, IS_CALLABLE_SUPPRESS_DEPRECATIONS, NULL, &fcc, NULL);
		}
		crex_fcc_dup(&collation->cmp_func, &fcc);

		collation->next = db_obj->collations;
		db_obj->collations = collation;

		RETURN_TRUE;
	}
	efree(collation);

	RETURN_FALSE;
}
/* }}} */

typedef struct {
	sqlite3_blob *blob;
	size_t		 position;
	size_t       size;
	int          flags;
} crx_stream_sqlite3_data;

static ssize_t crx_sqlite3_stream_write(crx_stream *stream, const char *buf, size_t count)
{
	crx_stream_sqlite3_data *sqlite3_stream = (crx_stream_sqlite3_data *) stream->abstract;

	if (sqlite3_stream->flags & SQLITE_OPEN_READONLY) {
		crx_sqlite3_error(NULL, 0, "Can't write to blob stream: is open as read only");
		return -1;
	}

	if (sqlite3_stream->position + count > sqlite3_stream->size) {
		crx_sqlite3_error(NULL, 0, "It is not possible to increase the size of a BLOB");
		return -1;
	}

	if (sqlite3_blob_write(sqlite3_stream->blob, buf, count, sqlite3_stream->position) != SQLITE_OK) {
		return -1;
	}

	if (sqlite3_stream->position + count >= sqlite3_stream->size) {
		stream->eof = 1;
		sqlite3_stream->position = sqlite3_stream->size;
	}
	else {
		sqlite3_stream->position += count;
	}

	return count;
}

static ssize_t crx_sqlite3_stream_read(crx_stream *stream, char *buf, size_t count)
{
	crx_stream_sqlite3_data *sqlite3_stream = (crx_stream_sqlite3_data *) stream->abstract;

	if (sqlite3_stream->position + count >= sqlite3_stream->size) {
		count = sqlite3_stream->size - sqlite3_stream->position;
		stream->eof = 1;
	}
	if (count) {
		if (sqlite3_blob_read(sqlite3_stream->blob, buf, count, sqlite3_stream->position) != SQLITE_OK) {
			return -1;
		}
		sqlite3_stream->position += count;
	}
	return count;
}

static int crx_sqlite3_stream_close(crx_stream *stream, int close_handle)
{
	crx_stream_sqlite3_data *sqlite3_stream = (crx_stream_sqlite3_data *) stream->abstract;

	if (sqlite3_blob_close(sqlite3_stream->blob) != SQLITE_OK) {
		/* Error occurred, but it still closed */
	}

	efree(sqlite3_stream);

	return 0;
}

static int crx_sqlite3_stream_flush(crx_stream *stream)
{
	/* do nothing */
	return 0;
}

/* {{{ */
static int crx_sqlite3_stream_seek(crx_stream *stream, crex_off_t offset, int whence, crex_off_t *newoffs)
{
	crx_stream_sqlite3_data *sqlite3_stream = (crx_stream_sqlite3_data *) stream->abstract;

	switch(whence) {
		case SEEK_CUR:
			if (offset < 0) {
				if (sqlite3_stream->position < (size_t)(-offset)) {
					sqlite3_stream->position = 0;
					*newoffs = -1;
					return -1;
				} else {
					sqlite3_stream->position = sqlite3_stream->position + offset;
					*newoffs = sqlite3_stream->position;
					stream->eof = 0;
					return 0;
				}
			} else {
				if (sqlite3_stream->position + (size_t)(offset) > sqlite3_stream->size) {
					sqlite3_stream->position = sqlite3_stream->size;
					*newoffs = -1;
					return -1;
				} else {
					sqlite3_stream->position = sqlite3_stream->position + offset;
					*newoffs = sqlite3_stream->position;
					stream->eof = 0;
					return 0;
				}
			}
		case SEEK_SET:
			if (sqlite3_stream->size < (size_t)(offset)) {
				sqlite3_stream->position = sqlite3_stream->size;
				*newoffs = -1;
				return -1;
			} else {
				sqlite3_stream->position = offset;
				*newoffs = sqlite3_stream->position;
				stream->eof = 0;
				return 0;
			}
		case SEEK_END:
			if (offset > 0) {
				sqlite3_stream->position = sqlite3_stream->size;
				*newoffs = -1;
				return -1;
			} else if (sqlite3_stream->size < (size_t)(-offset)) {
				sqlite3_stream->position = 0;
				*newoffs = -1;
				return -1;
			} else {
				sqlite3_stream->position = sqlite3_stream->size + offset;
				*newoffs = sqlite3_stream->position;
				stream->eof = 0;
				return 0;
			}
		default:
			*newoffs = sqlite3_stream->position;
			return -1;
	}
}
/* }}} */


static int crx_sqlite3_stream_cast(crx_stream *stream, int castas, void **ret)
{
	return FAILURE;
}

static int crx_sqlite3_stream_stat(crx_stream *stream, crx_stream_statbuf *ssb)
{
	crx_stream_sqlite3_data *sqlite3_stream = (crx_stream_sqlite3_data *) stream->abstract;
	ssb->sb.st_size = sqlite3_stream->size;
	return 0;
}

static const crx_stream_ops crx_stream_sqlite3_ops = {
	crx_sqlite3_stream_write,
	crx_sqlite3_stream_read,
	crx_sqlite3_stream_close,
	crx_sqlite3_stream_flush,
	"SQLite3",
	crx_sqlite3_stream_seek,
	crx_sqlite3_stream_cast,
	crx_sqlite3_stream_stat,
	NULL
};

/* {{{ Open a blob as a stream which we can read / write to. */
CRX_METHOD(SQLite3, openBlob)
{
	crx_sqlite3_db_object *db_obj;
	zval *object = CREX_THIS;
	const char *table, *column, *dbname = "main", *mode = "rb";
	size_t table_len, column_len, dbname_len;
	crex_long rowid, flags = SQLITE_OPEN_READONLY, sqlite_flags = 0;
	sqlite3_blob *blob = NULL;
	crx_stream_sqlite3_data *sqlite3_stream;
	crx_stream *stream;

	db_obj = C_SQLITE3_DB_P(object);

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ssl|pl", &table, &table_len, &column, &column_len, &rowid, &dbname, &dbname_len, &flags) == FAILURE) {
		RETURN_THROWS();
	}

	SQLITE3_CHECK_INITIALIZED(db_obj, db_obj->initialised, SQLite3)

	sqlite_flags = (flags & SQLITE_OPEN_READWRITE) ? 1 : 0;

	if (sqlite3_blob_open(db_obj->db, dbname, table, column, rowid, sqlite_flags, &blob) != SQLITE_OK) {
		crx_sqlite3_error(db_obj, sqlite3_errcode(db_obj->db), "Unable to open blob: %s", sqlite3_errmsg(db_obj->db));
		RETURN_FALSE;
	}

	sqlite3_stream = emalloc(sizeof(crx_stream_sqlite3_data));
	sqlite3_stream->blob = blob;
	sqlite3_stream->flags = flags;
	sqlite3_stream->position = 0;
	sqlite3_stream->size = sqlite3_blob_bytes(blob);

	if (sqlite_flags != 0) {
		mode = "r+b";
	}

	stream = crx_stream_alloc(&crx_stream_sqlite3_ops, sqlite3_stream, 0, mode);

	if (stream) {
		crx_stream_to_zval(stream, return_value);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Enables an exception error mode. */
CRX_METHOD(SQLite3, enableExceptions)
{
	crx_sqlite3_db_object *db_obj;
	zval *object = CREX_THIS;
	bool enableExceptions = 0;

	db_obj = C_SQLITE3_DB_P(object);

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|b", &enableExceptions) == FAILURE) {
		RETURN_THROWS();
	}

	RETVAL_BOOL(db_obj->exception);

	if (!enableExceptions) {
		crx_error_docref("ref.sqlite3", E_DEPRECATED, "Use of warnings for SQLite3 is deprecated");
	}

	db_obj->exception = enableExceptions;
}
/* }}} */

/* {{{ Register a callback function to be used as an authorizer by SQLite. The callback should return SQLite3::OK, SQLite3::IGNORE or SQLite3::DENY. */
CRX_METHOD(SQLite3, setAuthorizer)
{
	crx_sqlite3_db_object *db_obj;
	zval *object = CREX_THIS;
	db_obj = C_SQLITE3_DB_P(object);
	crex_fcall_info			fci;
	crex_fcall_info_cache	fcc;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_FUNC_OR_NULL(fci, fcc)
	CREX_PARSE_PARAMETERS_END();

	SQLITE3_CHECK_INITIALIZED(db_obj, db_obj->initialised, SQLite3)

	/* Clear previously set callback */
	if (CREX_FCC_INITIALIZED(db_obj->authorizer_fcc)) {
		crex_fcc_dtor(&db_obj->authorizer_fcc);
	}

	/* Only enable userland authorizer if argument is not NULL */
	if (CREX_FCI_INITIALIZED(fci)) {
		if (!CREX_FCC_INITIALIZED(fcc)) {
			crex_is_callable_ex(&fci.function_name, NULL, IS_CALLABLE_SUPPRESS_DEPRECATIONS, NULL, &fcc, NULL);
			/* Call trampoline has been cleared by zpp. Refetch it, because we want to deal
			 * with it outselves. It is important that it is not refetched on every call,
			 * because calls may occur from different scopes. */
		}
		db_obj->authorizer_fcc = fcc;
		crex_fcc_addref(&db_obj->authorizer_fcc);
	}

	RETURN_TRUE;
}
/* }}} */


#if SQLITE_VERSION_NUMBER >= 3006011
/* {{{ Backups the current database to another one. */
CRX_METHOD(SQLite3, backup)
{
	crx_sqlite3_db_object *source_obj;
	crx_sqlite3_db_object *destination_obj;
	const char *source_dbname = "main", *destination_dbname = "main";
	size_t source_dbname_length, destination_dbname_length;
	zval *source_zval = CREX_THIS;
	zval *destination_zval;
	sqlite3_backup *dbBackup;
	int rc; // Return code

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O|pp", &destination_zval, crx_sqlite3_sc_entry, &source_dbname, &source_dbname_length, &destination_dbname, &destination_dbname_length) == FAILURE) {
		RETURN_THROWS();
	}

	source_obj = C_SQLITE3_DB_P(source_zval);
	SQLITE3_CHECK_INITIALIZED(source_obj, source_obj->initialised, SQLite3)

	destination_obj = C_SQLITE3_DB_P(destination_zval);

	SQLITE3_CHECK_INITIALIZED(destination_obj, destination_obj->initialised, SQLite3)

	dbBackup = sqlite3_backup_init(destination_obj->db, destination_dbname, source_obj->db, source_dbname);

	if (dbBackup) {
		do {
			rc = sqlite3_backup_step(dbBackup, -1);
		} while (rc == SQLITE_OK);

		/* Release resources allocated by backup_init(). */
		rc = sqlite3_backup_finish(dbBackup);
	}
	else {
		rc = sqlite3_errcode(source_obj->db);
	}

	if (rc != SQLITE_OK) {
		if (rc == SQLITE_BUSY) {
			crx_sqlite3_error(source_obj, rc, "Backup failed: source database is busy");
		}
		else if (rc == SQLITE_LOCKED) {
			crx_sqlite3_error(source_obj, rc, "Backup failed: source database is locked");
		}
		else {
			crx_sqlite3_error(source_obj, rc, "Backup failed: %s", sqlite3_errmsg(source_obj->db));
		}
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */
#endif

/* {{{ Returns the number of parameters within the prepared statement. */
CRX_METHOD(SQLite3Stmt, paramCount)
{
	crx_sqlite3_stmt *stmt_obj;
	zval *object = CREX_THIS;
	stmt_obj = C_SQLITE3_STMT_P(object);

	CREX_PARSE_PARAMETERS_NONE();

	SQLITE3_CHECK_INITIALIZED(stmt_obj->db_obj, stmt_obj->initialised, SQLite3);
	SQLITE3_CHECK_INITIALIZED_STMT(stmt_obj->stmt, SQLite3Stmt);

	RETURN_LONG(sqlite3_bind_parameter_count(stmt_obj->stmt));
}
/* }}} */

/* {{{ Closes the prepared statement. */
CRX_METHOD(SQLite3Stmt, close)
{
	crx_sqlite3_stmt *stmt_obj;
	zval *object = CREX_THIS;
	stmt_obj = C_SQLITE3_STMT_P(object);

	CREX_PARSE_PARAMETERS_NONE();

	SQLITE3_CHECK_INITIALIZED(stmt_obj->db_obj, stmt_obj->initialised, SQLite3);

	if(stmt_obj->db_obj) {
		crex_llist_del_element(&(stmt_obj->db_obj->free_list), object, (int (*)(void *, void *)) crx_sqlite3_compare_stmt_zval_free);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Reset the prepared statement to the state before it was executed, bindings still remain. */
CRX_METHOD(SQLite3Stmt, reset)
{
	crx_sqlite3_stmt *stmt_obj;
	zval *object = CREX_THIS;
	stmt_obj = C_SQLITE3_STMT_P(object);

	CREX_PARSE_PARAMETERS_NONE();

	SQLITE3_CHECK_INITIALIZED(stmt_obj->db_obj, stmt_obj->initialised, SQLite3);
	SQLITE3_CHECK_INITIALIZED_STMT(stmt_obj->stmt, SQLite3Stmt);

	if (sqlite3_reset(stmt_obj->stmt) != SQLITE_OK) {
		crx_sqlite3_error(stmt_obj->db_obj, sqlite3_errcode(sqlite3_db_handle(stmt_obj->stmt)), "Unable to reset statement: %s", sqlite3_errmsg(sqlite3_db_handle(stmt_obj->stmt)));
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Clear all current bound parameters. */
CRX_METHOD(SQLite3Stmt, clear)
{
	crx_sqlite3_stmt *stmt_obj;
	zval *object = CREX_THIS;
	stmt_obj = C_SQLITE3_STMT_P(object);

	CREX_PARSE_PARAMETERS_NONE();

	SQLITE3_CHECK_INITIALIZED(stmt_obj->db_obj, stmt_obj->initialised, SQLite3);
	SQLITE3_CHECK_INITIALIZED_STMT(stmt_obj->stmt, SQLite3Stmt);

	if (sqlite3_clear_bindings(stmt_obj->stmt) != SQLITE_OK) {
		crx_sqlite3_error(stmt_obj->db_obj, sqlite3_errcode(sqlite3_db_handle(stmt_obj->stmt)), "Unable to clear statement: %s", sqlite3_errmsg(sqlite3_db_handle(stmt_obj->stmt)));
		RETURN_FALSE;
	}

	if (stmt_obj->bound_params) {
		crex_hash_destroy(stmt_obj->bound_params);
		FREE_HASHTABLE(stmt_obj->bound_params);
		stmt_obj->bound_params = NULL;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Returns true if a statement is definitely read only */
CRX_METHOD(SQLite3Stmt, readOnly)
{
	crx_sqlite3_stmt *stmt_obj;
	zval *object = CREX_THIS;
	stmt_obj = C_SQLITE3_STMT_P(object);

	CREX_PARSE_PARAMETERS_NONE();

	SQLITE3_CHECK_INITIALIZED(stmt_obj->db_obj, stmt_obj->initialised, SQLite3);
	SQLITE3_CHECK_INITIALIZED_STMT(stmt_obj->stmt, SQLite3Stmt);

	if (sqlite3_stmt_readonly(stmt_obj->stmt)) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
/* }}} */

/* bind parameters to a statement before execution */
static int crx_sqlite3_bind_params(crx_sqlite3_stmt *stmt_obj) /* {{{ */
{
	struct crx_sqlite3_bound_param *param;
	int return_code;

	if (stmt_obj->bound_params) {
		CREX_HASH_FOREACH_PTR(stmt_obj->bound_params, param) {
			zval *parameter;
			/* parameter must be a reference? */
			if (C_ISREF(param->parameter)) {
				parameter = C_REFVAL(param->parameter);
			} else {
				parameter = &param->parameter;
			}

			/* If the ZVAL is null then it should be bound as that */
			if (C_TYPE_P(parameter) == IS_NULL) {
				return_code = sqlite3_bind_null(stmt_obj->stmt, param->param_number);
				if (return_code != SQLITE_OK) {
					crx_sqlite3_error(stmt_obj->db_obj, return_code, "Unable to bind parameter number " CREX_LONG_FMT, param->param_number);
				}
				continue;
			}

			switch (param->type) {
				case SQLITE_INTEGER:
					convert_to_long(parameter);
#if CREX_LONG_MAX > 2147483647
					return_code = sqlite3_bind_int64(stmt_obj->stmt, param->param_number, C_LVAL_P(parameter));
#else
					return_code = sqlite3_bind_int(stmt_obj->stmt, param->param_number, C_LVAL_P(parameter));
#endif
					if (return_code != SQLITE_OK) {
						crx_sqlite3_error(stmt_obj->db_obj, return_code, "Unable to bind parameter number " CREX_LONG_FMT, param->param_number);
					}
					break;

				case SQLITE_FLOAT:
					convert_to_double(parameter);
					return_code = sqlite3_bind_double(stmt_obj->stmt, param->param_number, C_DVAL_P(parameter));
					if (return_code != SQLITE_OK) {
						crx_sqlite3_error(stmt_obj->db_obj, return_code, "Unable to bind parameter number " CREX_LONG_FMT, param->param_number);
					}
					break;

				case SQLITE_BLOB:
				{
					crx_stream *stream = NULL;
					crex_string *buffer = NULL;
					if (C_TYPE_P(parameter) == IS_RESOURCE) {
						crx_stream_from_zval_no_verify(stream, parameter);
						if (stream == NULL) {
							crx_sqlite3_error(stmt_obj->db_obj, 0, "Unable to read stream for parameter %ld", param->param_number);
							return FAILURE;
						}
						buffer = crx_stream_copy_to_mem(stream, CRX_STREAM_COPY_ALL, 0);
					} else {
						buffer = zval_get_string(parameter);
					}

					if (buffer) {
						return_code = sqlite3_bind_blob(stmt_obj->stmt, param->param_number, ZSTR_VAL(buffer), ZSTR_LEN(buffer), SQLITE_TRANSIENT);
						crex_string_release_ex(buffer, 0);
						if (return_code != SQLITE_OK) {
							crx_sqlite3_error(stmt_obj->db_obj, return_code, "Unable to bind parameter number " CREX_LONG_FMT, param->param_number);
						}
					} else {
						return_code = sqlite3_bind_null(stmt_obj->stmt, param->param_number);
						if (return_code != SQLITE_OK) {
							crx_sqlite3_error(stmt_obj->db_obj, return_code, "Unable to bind parameter number " CREX_LONG_FMT, param->param_number);
						}
					}
					break;
				}

				case SQLITE3_TEXT: {
					crex_string *str = zval_try_get_string(parameter);
					if (UNEXPECTED(!str)) {
						return FAILURE;
					}
					return_code = sqlite3_bind_text(stmt_obj->stmt, param->param_number, ZSTR_VAL(str), ZSTR_LEN(str), SQLITE_TRANSIENT);
					if (return_code != SQLITE_OK) {
						crx_sqlite3_error(stmt_obj->db_obj, return_code, "Unable to bind parameter number " CREX_LONG_FMT, param->param_number);
					}
					crex_string_release(str);
					break;
				}

				case SQLITE_NULL:
					return_code = sqlite3_bind_null(stmt_obj->stmt, param->param_number);
					if (return_code != SQLITE_OK) {
						crx_sqlite3_error(stmt_obj->db_obj, return_code, "Unable to bind parameter number " CREX_LONG_FMT, param->param_number);
					}
					break;

				default:
					crx_sqlite3_error(stmt_obj->db_obj, 0, "Unknown parameter type: %pd for parameter %pd", param->type, param->param_number);
					return FAILURE;
			}
		} CREX_HASH_FOREACH_END();
	}

	return SUCCESS;
}
/* }}} */


/* {{{ Returns the SQL statement used to prepare the query. If expanded is true, binded parameters and values will be expanded. */
CRX_METHOD(SQLite3Stmt, getSQL)
{
	crx_sqlite3_stmt *stmt_obj;
	bool expanded = 0;
	zval *object = getThis();
	stmt_obj = C_SQLITE3_STMT_P(object);
	int bind_rc;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(expanded)
	CREX_PARSE_PARAMETERS_END();

	SQLITE3_CHECK_INITIALIZED(stmt_obj->db_obj, stmt_obj->initialised, SQLite3);
	SQLITE3_CHECK_INITIALIZED_STMT(stmt_obj->stmt, SQLite3Stmt);

	bind_rc = crx_sqlite3_bind_params(stmt_obj);

	if (bind_rc == FAILURE || EG(exception)) {
		RETURN_FALSE;
	}

	if (expanded) {
#ifdef HAVE_SQLITE3_EXPANDED_SQL
		char *sql = sqlite3_expanded_sql(stmt_obj->stmt);
		RETVAL_STRING(sql);
		sqlite3_free(sql);
#else
		crx_sqlite3_error(stmt_obj->db_obj, 0, "The expanded parameter requires SQLite3 >= 3.14 and %s is installed", sqlite3_libversion());
		RETURN_FALSE;
#endif
	} else {
		const char *sql = sqlite3_sql(stmt_obj->stmt);
		RETVAL_STRING(sql);
	}
}
/* }}} */


static int register_bound_parameter_to_sqlite(struct crx_sqlite3_bound_param *param, crx_sqlite3_stmt *stmt) /* {{{ */
{
	HashTable *hash;
	hash = stmt->bound_params;

	if (!hash) {
		ALLOC_HASHTABLE(hash);
		crex_hash_init(hash, 13, NULL, sqlite3_param_dtor, 0);
		stmt->bound_params = hash;
	}

	/* We need a : prefix to resolve a name to a parameter number */
	if (param->name) {
		if (ZSTR_VAL(param->name)[0] != ':' && ZSTR_VAL(param->name)[0] != '@') {
			/* pre-increment for character + 1 for null */
			crex_string *temp = crex_string_alloc(ZSTR_LEN(param->name) + 1, 0);
			ZSTR_VAL(temp)[0] = ':';
			memmove(ZSTR_VAL(temp) + 1, ZSTR_VAL(param->name), ZSTR_LEN(param->name) + 1);
			param->name = temp;
		} else {
			param->name = crex_string_copy(param->name);
		}
		/* do lookup*/
		param->param_number = sqlite3_bind_parameter_index(stmt->stmt, ZSTR_VAL(param->name));
	}

	if (param->param_number < 1) {
		if (param->name) {
			crex_string_release_ex(param->name, 0);
		}
		return 0;
	}

	if (param->param_number >= 1) {
		crex_hash_index_del(hash, param->param_number);
	}

	if (param->name) {
		crex_hash_update_mem(hash, param->name, param, sizeof(struct crx_sqlite3_bound_param));
	} else {
		crex_hash_index_update_mem(hash, param->param_number, param, sizeof(struct crx_sqlite3_bound_param));
	}

	return 1;
}
/* }}} */

/* {{{ Best try to map between CRX and SQLite. Default is still text. */
#define CRX_SQLITE3_SET_TYPE(z, p) \
	switch (C_TYPE_P(z)) { \
		default: \
			(p).type = SQLITE_TEXT; \
			break; \
		case IS_LONG: \
		case IS_TRUE: \
		case IS_FALSE: \
			(p).type = SQLITE_INTEGER; \
			break; \
		case IS_DOUBLE: \
			(p).type = SQLITE_FLOAT; \
			break; \
		case IS_NULL: \
			(p).type = SQLITE_NULL; \
			break; \
	}
/* }}} */

/* {{{ Common implementation of ::bindParam() and ::bindValue */
static void sqlite3stmt_bind(INTERNAL_FUNCTION_PARAMETERS)
{
	crx_sqlite3_stmt *stmt_obj;
	zval *object = CREX_THIS;
	struct crx_sqlite3_bound_param param = {0};
	zval *parameter;
	stmt_obj = C_SQLITE3_STMT_P(object);

	param.param_number = -1;
	param.type = SQLITE3_TEXT;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR_OR_LONG(param.name, param.param_number)
		C_PARAM_ZVAL(parameter)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(param.type)
	CREX_PARSE_PARAMETERS_END();

	SQLITE3_CHECK_INITIALIZED(stmt_obj->db_obj, stmt_obj->initialised, SQLite3);
	SQLITE3_CHECK_INITIALIZED_STMT(stmt_obj->stmt, SQLite3Stmt);

	ZVAL_COPY(&param.parameter, parameter);

	if (CREX_NUM_ARGS() < 3) {
		CRX_SQLITE3_SET_TYPE(parameter, param);
	}

	if (!register_bound_parameter_to_sqlite(&param, stmt_obj)) {
		if (!C_ISUNDEF(param.parameter)) {
			zval_ptr_dtor(&(param.parameter));
			ZVAL_UNDEF(&param.parameter);
		}
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Bind Parameter to a stmt variable. */
CRX_METHOD(SQLite3Stmt, bindParam)
{
	sqlite3stmt_bind(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ Bind Value of a parameter to a stmt variable. */
CRX_METHOD(SQLite3Stmt, bindValue)
{
	sqlite3stmt_bind(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

#undef CRX_SQLITE3_SET_TYPE

/* {{{ Executes a prepared statement and returns a result set object. */
CRX_METHOD(SQLite3Stmt, execute)
{
	crx_sqlite3_stmt *stmt_obj;
	crx_sqlite3_result *result;
	zval *object = CREX_THIS;
	int return_code = 0;
	int bind_rc = 0;

	stmt_obj = C_SQLITE3_STMT_P(object);

	CREX_PARSE_PARAMETERS_NONE();

	SQLITE3_CHECK_INITIALIZED(stmt_obj->db_obj, stmt_obj->initialised, SQLite3);

	/* Always reset statement before execution, see bug #77051 */
	sqlite3_reset(stmt_obj->stmt);

	/* Bind parameters to the statement */
	bind_rc = crx_sqlite3_bind_params(stmt_obj);

	if (bind_rc == FAILURE || EG(exception)) {
		RETURN_FALSE;
	}

	return_code = sqlite3_step(stmt_obj->stmt);

	switch (return_code) {
		case SQLITE_ROW: /* Valid Row */
		case SQLITE_DONE: /* Valid but no results */
		{
			sqlite3_reset(stmt_obj->stmt);
			object_init_ex(return_value, crx_sqlite3_result_entry);
			result = C_SQLITE3_RESULT_P(return_value);

			result->is_prepared_statement = 1;
			result->db_obj = stmt_obj->db_obj;
			result->stmt_obj = stmt_obj;
			result->column_names = NULL;
			result->column_count = -1;
			ZVAL_OBJ_COPY(&result->stmt_obj_zval, C_OBJ_P(object));

			break;
		}
		case SQLITE_ERROR:
			sqlite3_reset(stmt_obj->stmt);
			CREX_FALLTHROUGH;
		default:
			if (!EG(exception)) {
				crx_sqlite3_error(stmt_obj->db_obj, sqlite3_errcode(sqlite3_db_handle(stmt_obj->stmt)), "Unable to execute statement: %s", sqlite3_errmsg(sqlite3_db_handle(stmt_obj->stmt)));
			}
			zval_ptr_dtor(return_value);
			RETURN_FALSE;
	}

	return;
}
/* }}} */

/* {{{ __mainor for SQLite3Stmt. */
CRX_METHOD(SQLite3Stmt, __main)
{
	crx_sqlite3_stmt *stmt_obj;
	crx_sqlite3_db_object *db_obj;
	zval *object = CREX_THIS;
	zval *db_zval;
	crex_string *sql;
	int errcode;
	crx_sqlite3_free_list *free_item;

	stmt_obj = C_SQLITE3_STMT_P(object);

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OS", &db_zval, crx_sqlite3_sc_entry, &sql) == FAILURE) {
		RETURN_THROWS();
	}

	db_obj = C_SQLITE3_DB_P(db_zval);

	SQLITE3_CHECK_INITIALIZED(db_obj, db_obj->initialised, SQLite3)

	if (!ZSTR_LEN(sql)) {
		RETURN_FALSE;
	}

	stmt_obj->db_obj = db_obj;
	ZVAL_OBJ_COPY(&stmt_obj->db_obj_zval, C_OBJ_P(db_zval));

	errcode = sqlite3_prepare_v2(db_obj->db, ZSTR_VAL(sql), ZSTR_LEN(sql), &(stmt_obj->stmt), NULL);
	if (errcode != SQLITE_OK) {
		crx_sqlite3_error(db_obj, errcode, "Unable to prepare statement: %s", sqlite3_errmsg(db_obj->db));
		zval_ptr_dtor(return_value);
		RETURN_FALSE;
	}
	stmt_obj->initialised = 1;

	free_item = emalloc(sizeof(crx_sqlite3_free_list));
	free_item->stmt_obj = stmt_obj;
	//??  free_item->stmt_obj_zval = CREX_THIS;
	ZVAL_OBJ(&free_item->stmt_obj_zval, C_OBJ_P(object));

	crex_llist_add_element(&(db_obj->free_list), &free_item);
}
/* }}} */

/* {{{ Number of columns in the result set. */
CRX_METHOD(SQLite3Result, numColumns)
{
	crx_sqlite3_result *result_obj;
	zval *object = CREX_THIS;
	result_obj = C_SQLITE3_RESULT_P(object);

	CREX_PARSE_PARAMETERS_NONE();

	SQLITE3_CHECK_INITIALIZED(result_obj->db_obj, result_obj->stmt_obj->initialised, SQLite3Result)

	RETURN_LONG(sqlite3_column_count(result_obj->stmt_obj->stmt));
}
/* }}} */

/* {{{ Returns the name of the nth column. */
CRX_METHOD(SQLite3Result, columnName)
{
	crx_sqlite3_result *result_obj;
	zval *object = CREX_THIS;
	crex_long column = 0;
	char *column_name;
	result_obj = C_SQLITE3_RESULT_P(object);

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(column)
	CREX_PARSE_PARAMETERS_END();

	SQLITE3_CHECK_INITIALIZED(result_obj->db_obj, result_obj->stmt_obj->initialised, SQLite3Result)

	column_name = (char*) sqlite3_column_name(result_obj->stmt_obj->stmt, column);

	if (column_name == NULL) {
		RETURN_FALSE;
	}

	RETVAL_STRING(column_name);
}
/* }}} */

/* {{{ Returns the type of the nth column. */
CRX_METHOD(SQLite3Result, columnType)
{
	crx_sqlite3_result *result_obj;
	zval *object = CREX_THIS;
	crex_long column = 0;
	result_obj = C_SQLITE3_RESULT_P(object);

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(column)
	CREX_PARSE_PARAMETERS_END();

	SQLITE3_CHECK_INITIALIZED(result_obj->db_obj, result_obj->stmt_obj->initialised, SQLite3Result)

	if (!sqlite3_data_count(result_obj->stmt_obj->stmt)) {
		RETURN_FALSE;
	}

	RETURN_LONG(sqlite3_column_type(result_obj->stmt_obj->stmt, column));
}
/* }}} */

/* {{{ Fetch a result row as both an associative or numerically indexed array or both. */
CRX_METHOD(SQLite3Result, fetchArray)
{
	crx_sqlite3_result *result_obj;
	zval *object = CREX_THIS;
	int i, ret;
	crex_long mode = CRX_SQLITE3_BOTH;
	result_obj = C_SQLITE3_RESULT_P(object);

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(mode)
	CREX_PARSE_PARAMETERS_END();

	SQLITE3_CHECK_INITIALIZED(result_obj->db_obj, result_obj->stmt_obj->initialised, SQLite3Result)

	ret = sqlite3_step(result_obj->stmt_obj->stmt);
	switch (ret) {
		case SQLITE_ROW:
			/* If there was no return value then just skip fetching */
			if (!USED_RET()) {
				RETURN_FALSE;
			}

			if (result_obj->column_count == -1) {
				result_obj->column_count = sqlite3_column_count(result_obj->stmt_obj->stmt);
			}

			int n_cols = result_obj->column_count;

			/* Cache column names to speed up repeated fetchArray calls. */
			if (mode & CRX_SQLITE3_ASSOC && !result_obj->column_names) {
				result_obj->column_names = emalloc(n_cols * sizeof(crex_string*));

				for (int i = 0; i < n_cols; i++) {
					const char *column = sqlite3_column_name(result_obj->stmt_obj->stmt, i);
					result_obj->column_names[i] = crex_string_init(column, strlen(column), 0);
				}
			}

			array_init(return_value);

			for (i = 0; i < n_cols; i++) {
				zval data;

				sqlite_value_to_zval(result_obj->stmt_obj->stmt, i, &data);

				if (mode & CRX_SQLITE3_NUM) {
					add_index_zval(return_value, i, &data);
				}

				if (mode & CRX_SQLITE3_ASSOC) {
					if (mode & CRX_SQLITE3_NUM) {
						if (C_REFCOUNTED(data)) {
							C_ADDREF(data);
						}
					}
					/* Note: we can't use the "add_new" variant here instead of "update" because
					 * when the same column name is encountered, the last result should be taken. */
					crex_symtable_update(C_ARR_P(return_value), result_obj->column_names[i], &data);
				}
			}
			break;

		case SQLITE_DONE:
			RETURN_FALSE;
			break;

		default:
			crx_sqlite3_error(result_obj->db_obj, sqlite3_errcode(sqlite3_db_handle(result_obj->stmt_obj->stmt)), "Unable to execute statement: %s", sqlite3_errmsg(sqlite3_db_handle(result_obj->stmt_obj->stmt)));
	}
}
/* }}} */

static void sqlite3result_clear_column_names_cache(crx_sqlite3_result *result) {
	if (result->column_names) {
		for (int i = 0; i < result->column_count; i++) {
			crex_string_release(result->column_names[i]);
		}
		efree(result->column_names);
	}
	result->column_names = NULL;
	result->column_count = -1;
}

/* {{{ Resets the result set back to the first row. */
CRX_METHOD(SQLite3Result, reset)
{
	crx_sqlite3_result *result_obj;
	zval *object = CREX_THIS;
	result_obj = C_SQLITE3_RESULT_P(object);

	CREX_PARSE_PARAMETERS_NONE();

	SQLITE3_CHECK_INITIALIZED(result_obj->db_obj, result_obj->stmt_obj->initialised, SQLite3Result)

	sqlite3result_clear_column_names_cache(result_obj);

	if (sqlite3_reset(result_obj->stmt_obj->stmt) != SQLITE_OK) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Closes the result set. */
CRX_METHOD(SQLite3Result, finalize)
{
	crx_sqlite3_result *result_obj;
	zval *object = CREX_THIS;
	result_obj = C_SQLITE3_RESULT_P(object);

	CREX_PARSE_PARAMETERS_NONE();

	SQLITE3_CHECK_INITIALIZED(result_obj->db_obj, result_obj->stmt_obj->initialised, SQLite3Result)

	sqlite3result_clear_column_names_cache(result_obj);

	/* We need to finalize an internal statement */
	if (result_obj->is_prepared_statement == 0) {
		crex_llist_del_element(&(result_obj->db_obj->free_list), &result_obj->stmt_obj_zval,
			(int (*)(void *, void *)) crx_sqlite3_compare_stmt_zval_free);
	} else {
		sqlite3_reset(result_obj->stmt_obj->stmt);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ __mainor for SQLite3Result. */
CRX_METHOD(SQLite3Result, __main)
{
	crex_throw_exception(crex_ce_exception, "SQLite3Result cannot be directly instantiated", 0);
}
/* }}} */

/* {{{ Authorization Callback */
static int crx_sqlite3_authorizer(void *autharg, int action, const char *arg1, const char *arg2, const char *arg3, const char *arg4)
{
	/* Check open_basedir restrictions first */
	if (PG(open_basedir) && *PG(open_basedir)) {
		if (action == SQLITE_ATTACH) {
			if (!arg1) {
				return SQLITE_DENY;
			}
			if (memcmp(arg1, ":memory:", sizeof(":memory:")) && *arg1) {
				if (strncmp(arg1, "file:", 5) == 0) {
					/* starts with "file:" */
					return SQLITE_DENY;
				} else if (crx_check_open_basedir(arg1)) {
					return SQLITE_DENY;
				}
			}
		}
	}

	crx_sqlite3_db_object *db_obj = (crx_sqlite3_db_object *)autharg;

	/* fallback to access allowed if authorizer callback is not defined */
	if (!CREX_FCC_INITIALIZED(db_obj->authorizer_fcc)) {
		return SQLITE_OK;
	}

	/* call userland authorizer callback, if set */
	zval retval;
	zval argv[5];

	ZVAL_LONG(&argv[0], action);

	if (NULL == arg1) {
		ZVAL_NULL(&argv[1]);
	} else {
		ZVAL_STRING(&argv[1], arg1);
	}

	if (NULL == arg2) {
		ZVAL_NULL(&argv[2]);
	} else {
		ZVAL_STRING(&argv[2], arg2);
	}

	if (NULL == arg3) {
		ZVAL_NULL(&argv[3]);
	} else {
		ZVAL_STRING(&argv[3], arg3);
	}

	if (NULL == arg4) {
		ZVAL_NULL(&argv[4]);
	} else {
		ZVAL_STRING(&argv[4], arg4);
	}

	int authreturn = SQLITE_DENY;

	crex_call_known_fcc(&db_obj->authorizer_fcc, &retval, /* argc */ 5, argv, /* named_params */ NULL);
	if (C_ISUNDEF(retval)) {
		crx_sqlite3_error(db_obj, 0, "An error occurred while invoking the authorizer callback");
	} else {
		if (C_TYPE(retval) != IS_LONG) {
			crx_sqlite3_error(db_obj, 0, "The authorizer callback returned an invalid type: expected int");
		} else {
			authreturn = C_LVAL(retval);

			if (authreturn != SQLITE_OK && authreturn != SQLITE_IGNORE && authreturn != SQLITE_DENY) {
				crx_sqlite3_error(db_obj, 0, "The authorizer callback returned an invalid value: %d", authreturn);
				authreturn = SQLITE_DENY;
			}
		}
	}

	/* Free local return and argument values */
	zval_ptr_dtor(&retval);
	zval_ptr_dtor(&argv[0]);
	zval_ptr_dtor(&argv[1]);
	zval_ptr_dtor(&argv[2]);
	zval_ptr_dtor(&argv[3]);
	zval_ptr_dtor(&argv[4]);

	return authreturn;
}
/* }}} */

/* {{{ crx_sqlite3_free_list_dtor */
static void crx_sqlite3_free_list_dtor(void **item)
{
	crx_sqlite3_free_list *free_item = (crx_sqlite3_free_list *)*item;

	if (free_item->stmt_obj && free_item->stmt_obj->initialised) {
		sqlite3_finalize(free_item->stmt_obj->stmt);
		free_item->stmt_obj->initialised = 0;
	}
	efree(*item);
}
/* }}} */

static int crx_sqlite3_compare_stmt_zval_free(crx_sqlite3_free_list **free_list, zval *statement ) /* {{{ */
{
	return  ((*free_list)->stmt_obj->initialised && C_PTR_P(statement) == C_PTR((*free_list)->stmt_obj_zval));
}
/* }}} */

static int crx_sqlite3_compare_stmt_free( crx_sqlite3_free_list **free_list, sqlite3_stmt *statement ) /* {{{ */
{
	return ((*free_list)->stmt_obj->initialised && statement == (*free_list)->stmt_obj->stmt);
}
/* }}} */

static void crx_sqlite3_object_free_storage(crex_object *object) /* {{{ */
{
	crx_sqlite3_db_object *intern = crx_sqlite3_db_from_obj(object);
	crx_sqlite3_func *func;
	crx_sqlite3_collation *collation;

	if (!intern) {
		return;
	}

	/* Release function_name from authorizer */
	if (CREX_FCC_INITIALIZED(intern->authorizer_fcc)) {
		crex_fcc_dtor(&intern->authorizer_fcc);
	}

	while (intern->funcs) {
		func = intern->funcs;
		intern->funcs = func->next;
		if (intern->initialised && intern->db) {
			sqlite3_create_function(intern->db, func->func_name, func->argc, SQLITE_UTF8, func, NULL, NULL, NULL);
		}

		efree((char*)func->func_name);

		if (CREX_FCC_INITIALIZED(func->func)) {
			crex_fcc_dtor(&func->func);
		}
		if (CREX_FCC_INITIALIZED(func->step)) {
			crex_fcc_dtor(&func->step);
		}
		if (CREX_FCC_INITIALIZED(func->fini)) {
			crex_fcc_dtor(&func->fini);
		}
		efree(func);
	}

	while (intern->collations){
		collation = intern->collations;
		intern->collations = collation->next;
		if (intern->initialised && intern->db){
			sqlite3_create_collation(intern->db, collation->collation_name, SQLITE_UTF8, NULL, NULL);
		}
		efree((char*)collation->collation_name);
		if (CREX_FCC_INITIALIZED(collation->cmp_func)) {
			crex_fcc_dtor(&collation->cmp_func);
		}
		efree(collation);
	}

	if (intern->initialised && intern->db) {
		sqlite3_close(intern->db);
		intern->initialised = 0;
	}

	crex_object_std_dtor(&intern->zo);
}
/* }}} */

static void crx_sqlite3_gc_buffer_add_fcc(crex_get_gc_buffer *gc_buffer, crex_fcall_info_cache *fcc)
{
	if (CREX_FCC_INITIALIZED(*fcc)) {
		crex_get_gc_buffer_add_fcc(gc_buffer, fcc);
	}
}

static HashTable *crx_sqlite3_get_gc(crex_object *object, zval **table, int *n)
{
	crx_sqlite3_db_object *intern = crx_sqlite3_db_from_obj(object);

	if (intern->funcs == NULL && intern->collations == NULL) {
		/* Fast path without allocations */
		*table = NULL;
		*n = 0;
		return crex_std_get_gc(object, table, n);
	} else {
		crex_get_gc_buffer *gc_buffer = crex_get_gc_buffer_create();

		crx_sqlite3_func *func = intern->funcs;
		while (func != NULL) {
			crx_sqlite3_gc_buffer_add_fcc(gc_buffer, &func->func);
			crx_sqlite3_gc_buffer_add_fcc(gc_buffer, &func->step);
			crx_sqlite3_gc_buffer_add_fcc(gc_buffer, &func->fini);
			func = func->next;
		}

		crx_sqlite3_collation *collation = intern->collations;
		while (collation != NULL) {
			crx_sqlite3_gc_buffer_add_fcc(gc_buffer, &collation->cmp_func);
			collation = collation->next;
		}

		crex_get_gc_buffer_use(gc_buffer, table, n);

		if (object->properties == NULL && object->ce->default_properties_count == 0) {
			return NULL;
		} else {
			return crex_std_get_properties(object);
		}
	}
}

static void crx_sqlite3_stmt_object_free_storage(crex_object *object) /* {{{ */
{
	crx_sqlite3_stmt *intern = crx_sqlite3_stmt_from_obj(object);

	if (!intern) {
		return;
	}

	if (intern->bound_params) {
		crex_hash_destroy(intern->bound_params);
		FREE_HASHTABLE(intern->bound_params);
		intern->bound_params = NULL;
	}

	if (intern->initialised) {
		crex_llist_del_element(&(intern->db_obj->free_list), intern->stmt,
			(int (*)(void *, void *)) crx_sqlite3_compare_stmt_free);
	}

	if (!C_ISUNDEF(intern->db_obj_zval)) {
		zval_ptr_dtor(&intern->db_obj_zval);
	}

	crex_object_std_dtor(&intern->zo);
}
/* }}} */

static void crx_sqlite3_result_object_free_storage(crex_object *object) /* {{{ */
{
	crx_sqlite3_result *intern = crx_sqlite3_result_from_obj(object);

	if (!intern) {
		return;
	}

	sqlite3result_clear_column_names_cache(intern);

	if (!C_ISNULL(intern->stmt_obj_zval)) {
		if (intern->stmt_obj && intern->stmt_obj->initialised) {
			sqlite3_reset(intern->stmt_obj->stmt);
		}

		zval_ptr_dtor(&intern->stmt_obj_zval);
	}

	crex_object_std_dtor(&intern->zo);
}
/* }}} */

static crex_object *crx_sqlite3_object_new(crex_class_entry *class_type) /* {{{ */
{
	crx_sqlite3_db_object *intern;

	/* Allocate memory for it */
	intern = crex_object_alloc(sizeof(crx_sqlite3_db_object), class_type);

	/* Need to keep track of things to free */
	crex_llist_init(&(intern->free_list),  sizeof(crx_sqlite3_free_list *), (llist_dtor_func_t)crx_sqlite3_free_list_dtor, 0);

	crex_object_std_init(&intern->zo, class_type);
	object_properties_init(&intern->zo, class_type);

	return &intern->zo;
}
/* }}} */

static crex_object *crx_sqlite3_stmt_object_new(crex_class_entry *class_type) /* {{{ */
{
	crx_sqlite3_stmt *intern;

	/* Allocate memory for it */
	intern = crex_object_alloc(sizeof(crx_sqlite3_stmt), class_type);

	crex_object_std_init(&intern->zo, class_type);
	object_properties_init(&intern->zo, class_type);

	return &intern->zo;
}
/* }}} */

static crex_object *crx_sqlite3_result_object_new(crex_class_entry *class_type) /* {{{ */
{
	crx_sqlite3_result *intern;

	/* Allocate memory for it */
	intern = crex_object_alloc(sizeof(crx_sqlite3_result), class_type);

	crex_object_std_init(&intern->zo, class_type);
	object_properties_init(&intern->zo, class_type);

	return &intern->zo;
}
/* }}} */

static void sqlite3_param_dtor(zval *data) /* {{{ */
{
	struct crx_sqlite3_bound_param *param = (struct crx_sqlite3_bound_param*)C_PTR_P(data);

	if (param->name) {
		crex_string_release_ex(param->name, 0);
	}

	if (!C_ISNULL(param->parameter)) {
		zval_ptr_dtor(&(param->parameter));
		ZVAL_UNDEF(&param->parameter);
	}
	efree(param);
}
/* }}} */

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(sqlite3)
{
	/* Register SQLite3Exception class */
	crx_sqlite3_exception_ce = register_class_SQLite3Exception(crex_ce_exception);

#ifdef ZTS
	/* Refuse to load if this wasn't a threasafe library loaded */
	if (!sqlite3_threadsafe()) {
		crx_error_docref(NULL, E_WARNING, "A thread safe version of SQLite is required when using a thread safe version of CRX.");
		return FAILURE;
	}
#endif

	memcpy(&sqlite3_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	memcpy(&sqlite3_stmt_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	memcpy(&sqlite3_result_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));

	/* Register SQLite 3 Class */
	sqlite3_object_handlers.offset = XtOffsetOf(crx_sqlite3_db_object, zo);
	sqlite3_object_handlers.clone_obj = NULL;
	sqlite3_object_handlers.free_obj = crx_sqlite3_object_free_storage;
	sqlite3_object_handlers.get_gc = crx_sqlite3_get_gc;
	crx_sqlite3_sc_entry = register_class_SQLite3();
	crx_sqlite3_sc_entry->create_object = crx_sqlite3_object_new;
	crx_sqlite3_sc_entry->default_object_handlers = &sqlite3_object_handlers;

	/* Register SQLite 3 Prepared Statement Class */
	sqlite3_stmt_object_handlers.offset = XtOffsetOf(crx_sqlite3_stmt, zo);
	sqlite3_stmt_object_handlers.clone_obj = NULL;
	sqlite3_stmt_object_handlers.free_obj = crx_sqlite3_stmt_object_free_storage;
	crx_sqlite3_stmt_entry = register_class_SQLite3Stmt();
	crx_sqlite3_stmt_entry->create_object = crx_sqlite3_stmt_object_new;
	crx_sqlite3_stmt_entry->default_object_handlers = &sqlite3_stmt_object_handlers;

	/* Register SQLite 3 Result Class */
	sqlite3_result_object_handlers.offset = XtOffsetOf(crx_sqlite3_result, zo);
	sqlite3_result_object_handlers.clone_obj = NULL;
	sqlite3_result_object_handlers.free_obj = crx_sqlite3_result_object_free_storage;
	crx_sqlite3_result_entry = register_class_SQLite3Result();
	crx_sqlite3_result_entry->create_object = crx_sqlite3_result_object_new;
	crx_sqlite3_result_entry->default_object_handlers = &sqlite3_result_object_handlers;

	REGISTER_INI_ENTRIES();

	register_sqlite3_symbols(module_number);

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION */
CRX_MSHUTDOWN_FUNCTION(sqlite3)
{
	UNREGISTER_INI_ENTRIES();

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(sqlite3)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "SQLite3 support", "enabled");
	crx_info_print_table_row(2, "SQLite Library", sqlite3_libversion());
	crx_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ CRX_GINIT_FUNCTION */
static CRX_GINIT_FUNCTION(sqlite3)
{
#if defined(COMPILE_DL_SQLITE3) && defined(ZTS)
	CREX_TSRMLS_CACHE_UPDATE();
#endif
	memset(sqlite3_globals, 0, sizeof(*sqlite3_globals));
}
/* }}} */

/* {{{ sqlite3_module_entry */
crex_module_entry sqlite3_module_entry = {
	STANDARD_MODULE_HEADER,
	"sqlite3",
	NULL,
	CRX_MINIT(sqlite3),
	CRX_MSHUTDOWN(sqlite3),
	NULL,
	NULL,
	CRX_MINFO(sqlite3),
	CRX_SQLITE3_VERSION,
	CRX_MODULE_GLOBALS(sqlite3),
	CRX_GINIT(sqlite3),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_SQLITE3
#ifdef ZTS
CREX_TSRMLS_CACHE_DEFINE()
#endif
CREX_GET_MODULE(sqlite3)
#endif
