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
   | Authors: Sascha Schumann <sascha@schumann.cx>                        |
   |          Marcus Boerger <helly@crx.net>                              |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"

#ifdef HAVE_DBA

#include "crx_ini.h"
#include <stdio.h>
#include <fcntl.h>
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif

#include "crx_dba.h"
#include "ext/standard/info.h"
#include "ext/standard/crx_string.h"
#include "ext/standard/flock_compat.h"

#include "crx_gdbm.h"
#include "crx_ndbm.h"
#include "crx_dbm.h"
#include "crx_cdb.h"
#include "crx_db1.h"
#include "crx_db2.h"
#include "crx_db3.h"
#include "crx_db4.h"
#include "crx_flatfile.h"
#include "crx_inifile.h"
#include "crx_qdbm.h"
#include "crx_tcadb.h"
#include "crx_lmdb.h"
#include "dba_arginfo.h"

CRX_MINIT_FUNCTION(dba);
CRX_MSHUTDOWN_FUNCTION(dba);
CRX_MINFO_FUNCTION(dba);

CREX_BEGIN_MODULE_GLOBALS(dba)
	const char *default_handler;
	const dba_handler *default_hptr;
CREX_END_MODULE_GLOBALS(dba)

CREX_DECLARE_MODULE_GLOBALS(dba)

#define DBA_G(v) CREX_MODULE_GLOBALS_ACCESSOR(dba, v)

static CRX_GINIT_FUNCTION(dba);

crex_module_entry dba_module_entry = {
	STANDARD_MODULE_HEADER,
	"dba",
	ext_functions,
	CRX_MINIT(dba),
	CRX_MSHUTDOWN(dba),
	NULL,
	NULL,
	CRX_MINFO(dba),
	CRX_DBA_VERSION,
	CRX_MODULE_GLOBALS(dba),
	CRX_GINIT(dba),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_DBA
#ifdef ZTS
CREX_TSRMLS_CACHE_DEFINE()
#endif
CREX_GET_MODULE(dba)
#endif

/* {{{ crx_dba_make_key */
static crex_string* crx_dba_make_key(HashTable *key)
{
	zval *group, *name;
	crex_string *group_str, *name_str;
	HashPosition pos;

	if (crex_hash_num_elements(key) != 2) {
		crex_argument_error(NULL, 1, "must have exactly two elements: \"key\" and \"name\"");
		return NULL;
	}

	// TODO: Use CREX_HASH_FOREACH_VAL() API?
	crex_hash_internal_pointer_reset_ex(key, &pos);
	group = crex_hash_get_current_data_ex(key, &pos);
	group_str = zval_try_get_string(group);
	if (!group_str) {
		return NULL;
	}

	crex_hash_move_forward_ex(key, &pos);
	name = crex_hash_get_current_data_ex(key, &pos);
	name_str = zval_try_get_string(name);
	if (!name_str) {
		crex_string_release_ex(group_str, false);
		return NULL;
	}

	// TODO: Check ZSTR_LEN(name) != 0
	if (ZSTR_LEN(group_str) == 0) {
		crex_string_release_ex(group_str, false);
		return name_str;
	}

	crex_string *key_str = crex_strpprintf(0, "[%s]%s", ZSTR_VAL(group_str), ZSTR_VAL(name_str));
	crex_string_release_ex(group_str, false);
	crex_string_release_ex(name_str, false);
	return key_str;
}
/* }}} */

#define DBA_RELEASE_HT_KEY_CREATION() if (key_ht) {crex_string_release_ex(key_str, false);}

#define DBA_FETCH_RESOURCE(info, id)	\
	if ((info = (dba_info *)crex_fetch_resource2(C_RES_P(id), "DBA identifier", le_db, le_pdb)) == NULL) { \
		RETURN_THROWS(); \
	}

/* check whether the user has write access */
#define DBA_WRITE_CHECK(info) \
	if ((info)->mode != DBA_WRITER && (info)->mode != DBA_TRUNC && (info)->mode != DBA_CREAT) { \
		crx_error_docref(NULL, E_WARNING, "Cannot perform a modification on a readonly database"); \
		RETURN_FALSE; \
	}

/* a DBA handler must have specific routines */

#define DBA_NAMED_HND(alias, name, flags) \
{\
	#alias, flags, dba_open_##name, dba_close_##name, dba_fetch_##name, dba_update_##name, \
	dba_exists_##name, dba_delete_##name, dba_firstkey_##name, dba_nextkey_##name, \
	dba_optimize_##name, dba_sync_##name, dba_info_##name \
},

#define DBA_HND(name, flags) DBA_NAMED_HND(name, name, flags)

/* }}} */

/* {{{ globals */

static const dba_handler handler[] = {
#ifdef DBA_GDBM
	DBA_HND(gdbm, DBA_LOCK_EXT) /* Locking done in library if set */
#endif
#ifdef DBA_DBM
	DBA_HND(dbm, DBA_LOCK_ALL) /* No lock in lib */
#endif
#ifdef DBA_NDBM
	DBA_HND(ndbm, DBA_LOCK_ALL) /* Could be done in library: filemode = 0644 + S_ENFMT */
#endif
#ifdef DBA_CDB
	DBA_HND(cdb, DBA_STREAM_OPEN|DBA_LOCK_ALL) /* No lock in lib */
#endif
#ifdef DBA_CDB_BUILTIN
	DBA_NAMED_HND(cdb_make, cdb, DBA_STREAM_OPEN|DBA_LOCK_ALL) /* No lock in lib */
#endif
#ifdef DBA_DB1
	DBA_HND(db1, DBA_LOCK_ALL) /* No lock in lib */
#endif
#ifdef DBA_DB2
	DBA_HND(db2, DBA_LOCK_ALL) /* No lock in lib */
#endif
#ifdef DBA_DB3
	DBA_HND(db3, DBA_LOCK_ALL) /* No lock in lib */
#endif
#ifdef DBA_DB4
	DBA_HND(db4, DBA_LOCK_ALL) /* No lock in lib */
#endif
#ifdef DBA_INIFILE
	DBA_HND(inifile, DBA_STREAM_OPEN|DBA_LOCK_ALL|DBA_CAST_AS_FD) /* No lock in lib */
#endif
#ifdef DBA_FLATFILE
	DBA_HND(flatfile, DBA_STREAM_OPEN|DBA_LOCK_ALL|DBA_NO_APPEND) /* No lock in lib */
#endif
#ifdef DBA_QDBM
	DBA_HND(qdbm, DBA_LOCK_EXT)
#endif
#ifdef DBA_TCADB
	DBA_HND(tcadb, DBA_LOCK_ALL)
#endif
#ifdef DBA_LMDB
	DBA_HND(lmdb, DBA_LOCK_EXT)
#endif
	{ NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};

#ifdef DBA_FLATFILE
#define DBA_DEFAULT "flatfile"
#elif defined(DBA_DB4)
#define DBA_DEFAULT "db4"
#elif defined(DBA_DB3)
#define DBA_DEFAULT "db3"
#elif defined(DBA_DB2)
#define DBA_DEFAULT "db2"
#elif defined(DBA_DB1)
#define DBA_DEFAULT "db1"
#elif defined(DBA_GDBM)
#define DBA_DEFAULT "gdbm"
#elif defined(DBA_NBBM)
#define DBA_DEFAULT "ndbm"
#elif defined(DBA_DBM)
#define DBA_DEFAULT "dbm"
#elif defined(DBA_QDBM)
#define DBA_DEFAULT "qdbm"
#elif defined(DBA_TCADB)
#define DBA_DEFAULT "tcadb"
#elif defined(DBA_LMDB)
#define DBA_DEFAULT "lmdb"
#else
#define DBA_DEFAULT ""
#endif
/* cdb/cdb_make and ini are no option here */

static int le_db;
static int le_pdb;
/* }}} */

/* {{{ dba_fetch_resource
CRXAPI void dba_fetch_resource(dba_info **pinfo, zval **id)
{
	dba_info *info;
	DBA_ID_FETCH
	*pinfo = info;
}
*/
/* }}} */

/* {{{ dba_get_handler
CRXAPI dba_handler *dba_get_handler(const char* handler_name)
{
	const dba_handler *hptr;
	for (hptr = handler; hptr->name && strcasecmp(hptr->name, handler_name); hptr++);
	return hptr;
}
*/
/* }}} */

/* {{{ dba_close */
static void dba_close(dba_info *info)
{
	if (info->hnd) {
		info->hnd->close(info);
	}
	CREX_ASSERT(info->path);
	crex_string_release_ex(info->path, info->flags&DBA_PERSISTENT);
	info->path = NULL;

	if (info->fp && info->fp != info->lock.fp) {
		if (info->flags & DBA_PERSISTENT) {
			crx_stream_pclose(info->fp);
		} else {
			crx_stream_close(info->fp);
		}
	}
	if (info->lock.fp) {
		if (info->flags & DBA_PERSISTENT) {
			crx_stream_pclose(info->lock.fp);
		} else {
			crx_stream_close(info->lock.fp);
		}
	}
	pefree(info, info->flags&DBA_PERSISTENT);
}
/* }}} */

/* {{{ dba_close_rsrc */
static void dba_close_rsrc(crex_resource *rsrc)
{
	dba_info *info = (dba_info *)rsrc->ptr;

	dba_close(info);
}
/* }}} */

/* {{{ dba_close_pe_rsrc_deleter */
static int dba_close_pe_rsrc_deleter(zval *el, void *pDba)
{
	if (C_RES_P(el)->ptr == pDba) {
		if (C_DELREF_P(el) == 0) {
			return CREX_HASH_APPLY_REMOVE;
		} else {
			return CREX_HASH_APPLY_KEEP | CREX_HASH_APPLY_STOP;
		}
	} else {
		return CREX_HASH_APPLY_KEEP;
	}
}
/* }}} */

/* {{{ dba_close_pe_rsrc */
static void dba_close_pe_rsrc(crex_resource *rsrc)
{
	dba_info *info = (dba_info *)rsrc->ptr;

	/* closes the resource by calling dba_close_rsrc() */
	crex_hash_apply_with_argument(&EG(persistent_list), dba_close_pe_rsrc_deleter, info);
}
/* }}} */

/* {{{ CRX_INI */
static CREX_INI_MH(OnUpdateDefaultHandler)
{
	const dba_handler *hptr;

	if (!ZSTR_LEN(new_value)) {
		DBA_G(default_hptr) = NULL;
		return OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
	}

	for (hptr = handler; hptr->name && strcasecmp(hptr->name, ZSTR_VAL(new_value)); hptr++);

	if (!hptr->name) {
		crx_error_docref(NULL, E_WARNING, "No such handler: %s", ZSTR_VAL(new_value));
		return FAILURE;
	}
	DBA_G(default_hptr) = hptr;
	return OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
}

CRX_INI_BEGIN()
	STD_CRX_INI_ENTRY("dba.default_handler", DBA_DEFAULT, CRX_INI_ALL, OnUpdateDefaultHandler, default_handler,    crex_dba_globals, dba_globals)
CRX_INI_END()
/* }}} */

/* {{{ CRX_GINIT_FUNCTION */
static CRX_GINIT_FUNCTION(dba)
{
#if defined(COMPILE_DL_DBA) && defined(ZTS)
	CREX_TSRMLS_CACHE_UPDATE();
#endif
	dba_globals->default_handler = "";
	dba_globals->default_hptr    = NULL;
}
/* }}} */

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(dba)
{
	REGISTER_INI_ENTRIES();
	le_db = crex_register_list_destructors_ex(dba_close_rsrc, NULL, "dba", module_number);
	le_pdb = crex_register_list_destructors_ex(dba_close_pe_rsrc, dba_close_rsrc, "dba persistent", module_number);
	register_dba_symbols(module_number);
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION */
CRX_MSHUTDOWN_FUNCTION(dba)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

#include "crex_smart_str.h"

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(dba)
{
	const dba_handler *hptr;
	smart_str handlers = {0};

	for(hptr = handler; hptr->name; hptr++) {
		smart_str_appends(&handlers, hptr->name);
		smart_str_appendc(&handlers, ' ');
	}

	crx_info_print_table_start();
	crx_info_print_table_row(2, "DBA support", "enabled");
	if (handlers.s) {
		smart_str_0(&handlers);
		crx_info_print_table_row(2, "Supported handlers", ZSTR_VAL(handlers.s));
		smart_str_free(&handlers);
	} else {
		crx_info_print_table_row(2, "Supported handlers", "none");
	}
	crx_info_print_table_end();
	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ crx_dba_update */
static void crx_dba_update(INTERNAL_FUNCTION_PARAMETERS, int mode)
{
	zval *id;
	dba_info *info = NULL;
	HashTable *key_ht = NULL;
	crex_string *key_str = NULL;
	crex_string *value;

	CREX_PARSE_PARAMETERS_START(3, 3)
		C_PARAM_ARRAY_HT_OR_STR(key_ht, key_str)
		C_PARAM_STR(value)
		C_PARAM_RESOURCE(id);
	CREX_PARSE_PARAMETERS_END();

	DBA_FETCH_RESOURCE(info, id);
	DBA_WRITE_CHECK(info);

	if (key_ht) {
		key_str = crx_dba_make_key(key_ht);
		if (!key_str) {
			// TODO ValueError?
			RETURN_FALSE;
		}
	}

	RETVAL_BOOL(info->hnd->update(info, key_str, value, mode) == SUCCESS);
	DBA_RELEASE_HT_KEY_CREATION();
}
/* }}} */

/* {{{ crx_find_dbm */
static dba_info *crx_dba_find(const crex_string *path)
{
	crex_resource *le;
	dba_info *info;
	crex_long numitems, i;

	numitems = crex_hash_next_free_element(&EG(regular_list));
	for (i=1; i<numitems; i++) {
		if ((le = crex_hash_index_find_ptr(&EG(regular_list), i)) == NULL) {
			continue;
		}
		if (le->type == le_db || le->type == le_pdb) {
			info = (dba_info *)(le->ptr);
			if (crex_string_equals(path, info->path)) {
				return (dba_info *)(le->ptr);
			}
		}
	}

	return NULL;
}
/* }}} */

static crex_always_inline crex_string *crx_dba_crex_string_dup_safe(crex_string *s, bool persistent)
{
	if (ZSTR_IS_INTERNED(s) && !persistent) {
		return s;
	} else {
		crex_string *duplicated_str = crex_string_init(ZSTR_VAL(s), ZSTR_LEN(s), persistent);
		if (persistent) {
			GC_MAKE_PERSISTENT_LOCAL(duplicated_str);
		}
		return duplicated_str;
	}
}


#define FREE_PERSISTENT_RESOURCE_KEY() if (persistent_resource_key) {crex_string_release_ex(persistent_resource_key, false);}

/* {{{ crx_dba_open */
static void crx_dba_open(INTERNAL_FUNCTION_PARAMETERS, bool persistent)
{
	dba_mode_t modenr;
	dba_info *info, *other;
	const dba_handler *hptr;
	const char *error = NULL;
	int lock_mode, lock_flag = 0;
	const char *file_mode;
	const char *lock_file_mode = NULL;
	int persistent_flag = persistent ? STREAM_OPEN_PERSISTENT : 0;
	char *lock_name;
#ifdef CRX_WIN32
	bool restarted = 0;
	bool need_creation = 0;
#endif

	crex_string *path;
	crex_string *mode;
	crex_string *handler_str = NULL;
	crex_long permission = 0644;
	crex_long map_size = 0;
	crex_long driver_flags = DBA_DEFAULT_DRIVER_FLAGS;
	bool is_flags_null = true;
	crex_string *persistent_resource_key = NULL;

	if (FAILURE == crex_parse_parameters(CREX_NUM_ARGS(), "PS|S!lll!", &path, &mode, &handler_str,
			&permission, &map_size, &driver_flags, &is_flags_null)) {
		RETURN_THROWS();
	}

	if (ZSTR_LEN(path) == 0) {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	}
	if (ZSTR_LEN(mode) == 0) {
		crex_argument_value_error(2, "cannot be empty");
		RETURN_THROWS();
	}
	if (handler_str && ZSTR_LEN(handler_str) == 0) {
		crex_argument_value_error(3, "cannot be empty");
		RETURN_THROWS();
	}
	// TODO Check Value for permission
	if (map_size < 0) {
		crex_argument_value_error(5, "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	if (!is_flags_null && driver_flags < 0) {
		crex_argument_value_error(6, "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	if (persistent) {
		crex_resource *le;

		if (handler_str) {
			persistent_resource_key = crex_string_concat3(
				ZSTR_VAL(path), ZSTR_LEN(path),
				ZSTR_VAL(mode), ZSTR_LEN(mode),
				ZSTR_VAL(handler_str), ZSTR_LEN(handler_str)
			);
		} else {
			persistent_resource_key = crex_string_concat2(
				ZSTR_VAL(path), ZSTR_LEN(path),
				ZSTR_VAL(mode), ZSTR_LEN(mode)
			);
		}

		/* try to find if we already have this link in our persistent list */
		if ((le = crex_hash_find_ptr(&EG(persistent_list), persistent_resource_key)) != NULL) {
			FREE_PERSISTENT_RESOURCE_KEY();
			if (le->type != le_pdb) {
				// TODO This should never happen
				RETURN_FALSE;
			}

			info = (dba_info *)le->ptr;

			GC_ADDREF(le);
			RETURN_RES(crex_register_resource(info, le_pdb));
		}
	}

	if (!handler_str) {
		hptr = DBA_G(default_hptr);
		if (!hptr) {
			crx_error_docref(NULL, E_WARNING, "No default handler selected");
			FREE_PERSISTENT_RESOURCE_KEY();
			RETURN_FALSE;
		}
		CREX_ASSERT(hptr->name);
	} else {
		/* Loop through global static var handlers to see if such a handler exists */
		for (hptr = handler; hptr->name && strcasecmp(hptr->name, ZSTR_VAL(handler_str)); hptr++);

		if (!hptr->name) {
			crx_error_docref(NULL, E_WARNING, "Handler \"%s\" is not available", ZSTR_VAL(handler_str));
			FREE_PERSISTENT_RESOURCE_KEY();
			RETURN_FALSE;
		}
	}

	/* Check mode: [rwnc][dl-]?t?
	 * r: Read
	 * w: Write
	 * n: Create/Truncate
	 * c: Create
	 *
	 * d: force lock on database file
	 * l: force lock on lck file
	 * -: ignore locking
	 *
	 * t: test open database, warning if locked
	 */
	bool is_test_lock = false;
	bool is_db_lock = false;
	bool is_lock_ignored = false;
	// bool is_file_lock = false;

	if (ZSTR_LEN(mode) == 0) {
		crex_argument_value_error(2, "cannot be empty");
		FREE_PERSISTENT_RESOURCE_KEY();
		RETURN_THROWS();
	}
	if (ZSTR_LEN(mode) > 3) {
		crex_argument_value_error(2, "must be at most 3 characters");
		FREE_PERSISTENT_RESOURCE_KEY();
		RETURN_THROWS();
	}
	if (ZSTR_LEN(mode) == 3) {
		if (ZSTR_VAL(mode)[2] != 't') {
			crex_argument_value_error(2, "third character must be \"t\"");
			FREE_PERSISTENT_RESOURCE_KEY();
			RETURN_THROWS();
		}
		is_test_lock = true;
	}
	if (ZSTR_LEN(mode) >= 2) {
		switch (ZSTR_VAL(mode)[1]) {
			case 't':
				is_test_lock = true;
				break;
			case '-':
				if ((hptr->flags & DBA_LOCK_ALL) == 0) {
					crx_error_docref(NULL, E_WARNING, "Locking cannot be disabled for handler %s", hptr->name);
					FREE_PERSISTENT_RESOURCE_KEY();
					RETURN_FALSE;
				}
				is_lock_ignored = true;
				lock_flag = 0;
			break;
			case 'd':
				is_db_lock = true;
				if ((hptr->flags & DBA_LOCK_ALL) == 0) {
					lock_flag = (hptr->flags & DBA_LOCK_ALL);
					break;
				}
				CREX_FALLTHROUGH;
			case 'l':
				// is_file_lock = true;
				lock_flag = DBA_LOCK_ALL;
				if ((hptr->flags & DBA_LOCK_ALL) == 0) {
					crx_error_docref(NULL, E_NOTICE, "Handler %s does locking internally", hptr->name);
				}
				break;
			default:
				crex_argument_value_error(2, "second character must be one of \"d\", \"l\", \"-\", or \"t\"");
				FREE_PERSISTENT_RESOURCE_KEY();
				RETURN_THROWS();
		}
	} else {
		lock_flag = (hptr->flags&DBA_LOCK_ALL);
		is_db_lock = true;
	}

	switch (ZSTR_VAL(mode)[0]) {
		case 'r':
			modenr = DBA_READER;
			lock_mode = (lock_flag & DBA_LOCK_READER) ? LOCK_SH : 0;
			file_mode = "r";
			break;
		case 'w':
			modenr = DBA_WRITER;
			lock_mode = (lock_flag & DBA_LOCK_WRITER) ? LOCK_EX : 0;
			file_mode = "r+b";
			break;
		case 'c': {
#ifdef CRX_WIN32
			if (hptr->flags & (DBA_NO_APPEND|DBA_CAST_AS_FD)) {
				crx_stream_statbuf ssb;
				need_creation = (SUCCESS != crx_stream_stat_path(ZSTR_VAL(path), &ssb));
			}
#endif
			modenr = DBA_CREAT;
			lock_mode = (lock_flag & DBA_LOCK_CREAT) ? LOCK_EX : 0;
			if (lock_mode) {
				if (is_db_lock) {
					/* the create/append check will be done on the lock
					 * when the lib opens the file it is already created
					 */
					file_mode = "r+b";       /* read & write, seek 0 */
#ifdef CRX_WIN32
					if (!need_creation) {
						lock_file_mode = "r+b";
					} else
#endif
					lock_file_mode = "a+b";  /* append */
				} else {
#ifdef CRX_WIN32
					if (!need_creation) {
						file_mode = "r+b";
					} else
#endif
					file_mode = "a+b";       /* append */
					lock_file_mode = "w+b";  /* create/truncate */
				}
			} else {
#ifdef CRX_WIN32
				if (!need_creation) {
					file_mode = "r+b";
				} else
#endif
				file_mode = "a+b";
			}
			/* In case of the 'a+b' append mode, the handler is responsible
			 * to handle any rewind problems (see flatfile handler).
			 */
			break;
		}
		case 'n':
			modenr = DBA_TRUNC;
			lock_mode = (lock_flag & DBA_LOCK_TRUNC) ? LOCK_EX : 0;
			file_mode = "w+b";
			break;
		default:
			crex_argument_value_error(2, "first character must be one of \"r\", \"w\", \"c\", or \"n\"");
			FREE_PERSISTENT_RESOURCE_KEY();
			RETURN_THROWS();
	}
	if (!lock_file_mode) {
		lock_file_mode = file_mode;
	}
	if (is_test_lock) {
		if (is_lock_ignored) {
			crex_argument_value_error(2, "cannot combine mode \"-\" (no lock) and \"t\" (test lock)");
			FREE_PERSISTENT_RESOURCE_KEY();
			RETURN_THROWS();
		}
		if (!lock_mode) {
			if ((hptr->flags & DBA_LOCK_ALL) == 0) {
				crx_error_docref(NULL, E_WARNING, "Handler %s uses its own locking which doesn't support mode modifier t (test lock)", hptr->name);
				FREE_PERSISTENT_RESOURCE_KEY();
				RETURN_FALSE;
			} else {
				crx_error_docref(NULL, E_WARNING, "Handler %s doesn't uses locking for this mode which makes modifier t (test lock) obsolete", hptr->name);
				FREE_PERSISTENT_RESOURCE_KEY();
				RETURN_FALSE;
			}
		} else {
			lock_mode |= LOCK_NB; /* test =: non blocking */
		}
	}

	info = pemalloc(sizeof(dba_info), persistent);
	memset(info, 0, sizeof(dba_info));
	info->path = crx_dba_crex_string_dup_safe(path, persistent);
	info->mode = modenr;
	info->file_permission = permission;
	info->map_size = map_size;
	info->driver_flags = driver_flags;
	info->flags = (hptr->flags & ~DBA_LOCK_ALL) | (lock_flag & DBA_LOCK_ALL) | (persistent ? DBA_PERSISTENT : 0);
	info->lock.mode = lock_mode;

	/* if any open call is a locking call:
	 * check if we already have a locking call open that should block this call
	 * the problem is some systems would allow read during write
	 */
	if (hptr->flags & DBA_LOCK_ALL) {
		if ((other = crx_dba_find(info->path)) != NULL) {
			if (   ( (lock_mode&LOCK_EX)        && (other->lock.mode&(LOCK_EX|LOCK_SH)) )
			    || ( (other->lock.mode&LOCK_EX) && (lock_mode&(LOCK_EX|LOCK_SH))        )
			   ) {
				error = "Unable to establish lock (database file already open)"; /* force failure exit */
			}
		}
	}

#ifdef CRX_WIN32
restart:
#endif
	if (!error && lock_mode) {
		if (is_db_lock) {
			lock_name = ZSTR_VAL(path);
		} else {
			spprintf(&lock_name, 0, "%s.lck", ZSTR_VAL(info->path));
			if (!strcmp(file_mode, "r")) {
				crex_string *opened_path = NULL;
				/* when in read only mode try to use existing .lck file first */
				/* do not log errors for .lck file while in read only mode on .lck file */
				lock_file_mode = "rb";
				info->lock.fp = crx_stream_open_wrapper(lock_name, lock_file_mode, STREAM_MUST_SEEK|IGNORE_PATH|persistent_flag, &opened_path);
				if (opened_path) {
					crex_string_release_ex(opened_path, 0);
				}
			}
			if (!info->lock.fp) {
				/* when not in read mode or failed to open .lck file read only. now try again in create(write) mode and log errors */
				lock_file_mode = "a+b";
			}
		}
		if (!info->lock.fp) {
			crex_string *opened_path = NULL;
			info->lock.fp = crx_stream_open_wrapper(lock_name, lock_file_mode, STREAM_MUST_SEEK|REPORT_ERRORS|IGNORE_PATH|persistent_flag, &opened_path);
			if (info->lock.fp) {
				if (is_db_lock) {
					CREX_ASSERT(opened_path);
					/* replace the path info with the real path of the opened file */
					crex_string_release(info->path);
					info->path = crx_dba_crex_string_dup_safe(opened_path, persistent);
				}
			}
			if (opened_path) {
				crex_string_release_ex(opened_path, 0);
			}
		}
		if (!is_db_lock) {
			efree(lock_name);
		}
		if (!info->lock.fp) {
			dba_close(info);
			/* stream operation already wrote an error message */
			FREE_PERSISTENT_RESOURCE_KEY();
			RETURN_FALSE;
		}
		if (!crx_stream_supports_lock(info->lock.fp)) {
			error = "Stream does not support locking";
		}
		if (crx_stream_lock(info->lock.fp, lock_mode)) {
			error = "Unable to establish lock"; /* force failure exit */
		}
	}

	/* centralised open stream for builtin */
	if (!error && (hptr->flags&DBA_STREAM_OPEN)==DBA_STREAM_OPEN) {
		if (info->lock.fp && is_db_lock) {
			info->fp = info->lock.fp; /* use the same stream for locking and database access */
		} else {
			info->fp = crx_stream_open_wrapper(ZSTR_VAL(info->path), file_mode, STREAM_MUST_SEEK|REPORT_ERRORS|IGNORE_PATH|persistent_flag, NULL);
		}
		if (!info->fp) {
			dba_close(info);
			/* stream operation already wrote an error message */
			FREE_PERSISTENT_RESOURCE_KEY();
			RETURN_FALSE;
		}
		if (hptr->flags & (DBA_NO_APPEND|DBA_CAST_AS_FD)) {
			/* Needed because some systems do not allow to write to the original
			 * file contents with O_APPEND being set.
			 */
			if (SUCCESS != crx_stream_cast(info->fp, CRX_STREAM_AS_FD, (void*)&info->fd, 1)) {
				crx_error_docref(NULL, E_WARNING, "Could not cast stream");
				dba_close(info);
				FREE_PERSISTENT_RESOURCE_KEY();
				RETURN_FALSE;
#ifdef F_SETFL
			} else if (modenr == DBA_CREAT) {
				int flags = fcntl(info->fd, F_GETFL);
				fcntl(info->fd, F_SETFL, flags & ~O_APPEND);
#elif defined(CRX_WIN32)
			} else if (modenr == DBA_CREAT && need_creation && !restarted) {
				if (info->lock.fp != NULL) {
					crx_stream_free(info->lock.fp, persistent ? CRX_STREAM_FREE_CLOSE_PERSISTENT : CRX_STREAM_FREE_CLOSE);
				}
				if (info->fp != info->lock.fp) {
					crx_stream_free(info->fp, persistent ? CRX_STREAM_FREE_CLOSE_PERSISTENT : CRX_STREAM_FREE_CLOSE);
				}
				info->fp = NULL;
				info->lock.fp = NULL;
				info->fd = -1;

				lock_file_mode = "r+b";

				restarted = 1;
				goto restart;
#endif
			}
		}
	}

	if (error || hptr->open(info, &error) == FAILURE) {
		dba_close(info);
		if (EXPECTED(!EG(exception))) {
			if (error) {
				crx_error_docref(NULL, E_WARNING, "Driver initialization failed for handler: %s: %s", hptr->name, error);
			} else {
				crx_error_docref(NULL, E_WARNING, "Driver initialization failed for handler: %s", hptr->name);
			}
		}
		FREE_PERSISTENT_RESOURCE_KEY();
		RETURN_FALSE;
	}

	info->hnd = hptr;

	if (persistent) {
		CREX_ASSERT(persistent_resource_key);
		if (crex_register_persistent_resource_ex(persistent_resource_key, info, le_pdb) == NULL) {
			dba_close(info);
			crx_error_docref(NULL, E_WARNING, "Could not register persistent resource");
			FREE_PERSISTENT_RESOURCE_KEY();
			RETURN_FALSE;
		}
		FREE_PERSISTENT_RESOURCE_KEY();
	}

	RETURN_RES(crex_register_resource(info, (persistent ? le_pdb : le_db)));
}
/* }}} */

/* {{{ Opens path using the specified handler in mode persistently */
CRX_FUNCTION(dba_popen)
{
	crx_dba_open(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ Opens path using the specified handler in mode*/
CRX_FUNCTION(dba_open)
{
	crx_dba_open(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ Closes database */
CRX_FUNCTION(dba_close)
{
	zval *id;
	dba_info *info = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "r", &id) == FAILURE) {
		RETURN_THROWS();
	}

	DBA_FETCH_RESOURCE(info, id);

	crex_list_close(C_RES_P(id));
}
/* }}} */

/* {{{ Checks, if the specified key exists */
CRX_FUNCTION(dba_exists)
{
	zval *id;
	dba_info *info = NULL;
	HashTable *key_ht = NULL;
	crex_string *key_str = NULL;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_ARRAY_HT_OR_STR(key_ht, key_str)
		C_PARAM_RESOURCE(id);
	CREX_PARSE_PARAMETERS_END();

	DBA_FETCH_RESOURCE(info, id);

	if (key_ht) {
		key_str = crx_dba_make_key(key_ht);
		if (!key_str) {
			// TODO ValueError?
			RETURN_FALSE;
		}
	}

	RETVAL_BOOL(info->hnd->exists(info, key_str) == SUCCESS);
	DBA_RELEASE_HT_KEY_CREATION();
}
/* }}} */

/* {{{ Fetches the data associated with key */
CRX_FUNCTION(dba_fetch)
{
	zval *id;
	dba_info *info = NULL;
	HashTable *key_ht = NULL;
	crex_string *key_str = NULL;
	crex_long skip = 0;

	/* Check for legacy signature */
	if (CREX_NUM_ARGS() == 3) {
		CREX_PARSE_PARAMETERS_START_EX(CREX_PARSE_PARAMS_QUIET, 3, 3)
			C_PARAM_ARRAY_HT_OR_STR(key_ht, key_str)
			C_PARAM_LONG(skip)
			C_PARAM_RESOURCE(id);
		CREX_PARSE_PARAMETERS_END_EX(goto standard;);

		crex_error(E_DEPRECATED, "Calling dba_fetch() with $dba at the 3rd parameter is deprecated");
		if (UNEXPECTED(EG(exception))) {
			RETURN_THROWS();
		}
	} else {
		standard:
		CREX_PARSE_PARAMETERS_START(2, 3)
			C_PARAM_ARRAY_HT_OR_STR(key_ht, key_str)
			C_PARAM_RESOURCE(id);
			C_PARAM_OPTIONAL
			C_PARAM_LONG(skip)
		CREX_PARSE_PARAMETERS_END();
	}

	DBA_FETCH_RESOURCE(info, id);

	if (key_ht) {
		key_str = crx_dba_make_key(key_ht);
		if (!key_str) {
			// TODO ValueError?
			RETURN_FALSE;
		}
	}

	if (skip != 0) {
		if (!strcmp(info->hnd->name, "cdb")) {
			// TODO ValueError?
			if (skip < 0) {
				crx_error_docref(NULL, E_NOTICE, "Handler %s accepts only skip values greater than or equal to zero, using skip=0", info->hnd->name);
				skip = 0;
			}
		} else if (!strcmp(info->hnd->name, "inifile")) {
			/* "-1" is comparable to 0 but allows a non restrictive
			 * access which is faster. For example 'inifile' uses this
			 * to allow faster access when the key was already found
			 * using firstkey/nextkey. However explicitly setting the
			 * value to 0 ensures the first value.
			 */
			if (skip < -1) {
				// TODO ValueError?
				crx_error_docref(NULL, E_NOTICE, "Handler %s accepts only skip value -1 and greater, using skip=0", info->hnd->name);
				skip = 0;
			}
		} else {
			crx_error_docref(NULL, E_NOTICE, "Handler %s does not support optional skip parameter, the value will be ignored", info->hnd->name);
			skip = 0;
		}
	}

	crex_string *val;
	if ((val = info->hnd->fetch(info, key_str, skip)) == NULL) {
		DBA_RELEASE_HT_KEY_CREATION();
		RETURN_FALSE;
	}
	DBA_RELEASE_HT_KEY_CREATION();
	RETURN_STR(val);
}
/* }}} */

/* {{{ Splits an inifile key into an array of the form array(0=>group,1=>value_name) but returns false if input is false or null */
CRX_FUNCTION(dba_key_split)
{
	zval *zkey;
	char *key, *name;
	size_t key_len;

	if (CREX_NUM_ARGS() != 1) {
		WRONG_PARAM_COUNT;
	}
	if (crex_parse_parameters_ex(CREX_PARSE_PARAMS_QUIET, CREX_NUM_ARGS(), "z", &zkey) == SUCCESS) {
		if (C_TYPE_P(zkey) == IS_NULL || (C_TYPE_P(zkey) == IS_FALSE)) {
			RETURN_FALSE;
		}
	}
	if (crex_parse_parameters(CREX_NUM_ARGS(), "s", &key, &key_len) == FAILURE) {
		RETURN_THROWS();
	}
	array_init(return_value);
	if (key[0] == '[' && (name = strchr(key, ']')) != NULL) {
		add_next_index_stringl(return_value, key+1, name - (key + 1));
		add_next_index_stringl(return_value, name+1, key_len - (name - key + 1));
	} else {
		add_next_index_stringl(return_value, "", 0);
		add_next_index_stringl(return_value, key, key_len);
	}
}
/* }}} */

/* {{{ Resets the internal key pointer and returns the first key */
CRX_FUNCTION(dba_firstkey)
{
	zval *id;
	dba_info *info = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "r", &id) == FAILURE) {
		RETURN_THROWS();
	}

	DBA_FETCH_RESOURCE(info, id);

	crex_string *fkey = info->hnd->firstkey(info);

	if (fkey) {
		RETURN_STR(fkey);
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ Returns the next key */
CRX_FUNCTION(dba_nextkey)
{
	zval *id;
	dba_info *info = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "r", &id) == FAILURE) {
		RETURN_THROWS();
	}

	DBA_FETCH_RESOURCE(info, id);

	crex_string *nkey = info->hnd->nextkey(info);

	if (nkey) {
		RETURN_STR(nkey);
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ Deletes the entry associated with key
   If inifile: remove all other key lines */
CRX_FUNCTION(dba_delete)
{
	zval *id;
	dba_info *info = NULL;
	HashTable *key_ht = NULL;
	crex_string *key_str = NULL;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_ARRAY_HT_OR_STR(key_ht, key_str)
		C_PARAM_RESOURCE(id);
	CREX_PARSE_PARAMETERS_END();

	DBA_FETCH_RESOURCE(info, id);
	DBA_WRITE_CHECK(info);

	if (key_ht) {
		key_str = crx_dba_make_key(key_ht);
		if (!key_str) {
			// TODO ValueError?
			RETURN_FALSE;
		}
	}

	RETVAL_BOOL(info->hnd->delete(info, key_str) == SUCCESS);
	DBA_RELEASE_HT_KEY_CREATION();
}
/* }}} */

/* {{{ If not inifile: Insert value as key, return false, if key exists already
   If inifile: Add vakue as key (next instance of key) */
CRX_FUNCTION(dba_insert)
{
	crx_dba_update(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ Inserts value as key, replaces key, if key exists already
   If inifile: remove all other key lines */
CRX_FUNCTION(dba_replace)
{
	crx_dba_update(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ Optimizes (e.g. clean up, vacuum) database */
CRX_FUNCTION(dba_optimize)
{
	zval *id;
	dba_info *info = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "r", &id) == FAILURE) {
		RETURN_THROWS();
	}

	DBA_FETCH_RESOURCE(info, id);
	DBA_WRITE_CHECK(info);

	if (info->hnd->optimize(info) == SUCCESS) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ Synchronizes database */
CRX_FUNCTION(dba_sync)
{
	zval *id;
	dba_info *info = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "r", &id) == FAILURE) {
		RETURN_THROWS();
	}

	DBA_FETCH_RESOURCE(info, id);

	if (info->hnd->sync(info) == SUCCESS) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ List configured database handlers */
CRX_FUNCTION(dba_handlers)
{
	const dba_handler *hptr;
	bool full_info = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|b", &full_info) == FAILURE) {
		RETURN_THROWS();
	}

	array_init(return_value);

	for(hptr = handler; hptr->name; hptr++) {
		if (full_info) {
			// TODO: avoid reallocation ???
			char *str = hptr->info(hptr, NULL);
			add_assoc_string(return_value, hptr->name, str);
			efree(str);
		} else {
			add_next_index_string(return_value, hptr->name);
		}
	}
}
/* }}} */

/* {{{ List opened databases */
CRX_FUNCTION(dba_list)
{
	crex_ulong numitems, i;
	crex_resource *le;
	dba_info *info;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	array_init(return_value);

	numitems = crex_hash_next_free_element(&EG(regular_list));
	for (i=1; i<numitems; i++) {
		if ((le = crex_hash_index_find_ptr(&EG(regular_list), i)) == NULL) {
			continue;
		}
		if (le->type == le_db || le->type == le_pdb) {
			info = (dba_info *)(le->ptr);
			add_index_str(return_value, i, crex_string_copy(info->path));
		}
	}
}
/* }}} */

#endif /* HAVE_DBA */
