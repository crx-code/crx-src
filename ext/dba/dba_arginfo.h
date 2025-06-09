/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 1a02eaf9da45edb40720620e3beef43fd19dd520 */

CREX_BEGIN_ARG_INFO_EX(arginfo_dba_popen, 0, 0, 2)
	CREX_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, mode, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, handler, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, permission, IS_LONG, 0, "0644")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, map_size, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_dba_open arginfo_dba_popen

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dba_close, 0, 1, IS_VOID, 0)
	CREX_ARG_INFO(0, dba)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dba_exists, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_MASK(0, key, MAY_BE_STRING|MAY_BE_ARRAY, NULL)
	CREX_ARG_INFO(0, dba)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_dba_fetch, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_MASK(0, key, MAY_BE_STRING|MAY_BE_ARRAY, NULL)
	CREX_ARG_INFO(0, dba)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, skip, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_dba_key_split, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_MASK(0, key, MAY_BE_STRING|MAY_BE_FALSE|MAY_BE_NULL, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_dba_firstkey, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO(0, dba)
CREX_END_ARG_INFO()

#define arginfo_dba_nextkey arginfo_dba_firstkey

#define arginfo_dba_delete arginfo_dba_exists

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dba_insert, 0, 3, _IS_BOOL, 0)
	CREX_ARG_TYPE_MASK(0, key, MAY_BE_STRING|MAY_BE_ARRAY, NULL)
	CREX_ARG_TYPE_INFO(0, value, IS_STRING, 0)
	CREX_ARG_INFO(0, dba)
CREX_END_ARG_INFO()

#define arginfo_dba_replace arginfo_dba_insert

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dba_optimize, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, dba)
CREX_END_ARG_INFO()

#define arginfo_dba_sync arginfo_dba_optimize

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dba_handlers, 0, 0, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, full_info, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dba_list, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()


CREX_FUNCTION(dba_popen);
CREX_FUNCTION(dba_open);
CREX_FUNCTION(dba_close);
CREX_FUNCTION(dba_exists);
CREX_FUNCTION(dba_fetch);
CREX_FUNCTION(dba_key_split);
CREX_FUNCTION(dba_firstkey);
CREX_FUNCTION(dba_nextkey);
CREX_FUNCTION(dba_delete);
CREX_FUNCTION(dba_insert);
CREX_FUNCTION(dba_replace);
CREX_FUNCTION(dba_optimize);
CREX_FUNCTION(dba_sync);
CREX_FUNCTION(dba_handlers);
CREX_FUNCTION(dba_list);


static const crex_function_entry ext_functions[] = {
	CREX_FE(dba_popen, arginfo_dba_popen)
	CREX_FE(dba_open, arginfo_dba_open)
	CREX_FE(dba_close, arginfo_dba_close)
	CREX_FE(dba_exists, arginfo_dba_exists)
	CREX_FE(dba_fetch, arginfo_dba_fetch)
	CREX_FE(dba_key_split, arginfo_dba_key_split)
	CREX_FE(dba_firstkey, arginfo_dba_firstkey)
	CREX_FE(dba_nextkey, arginfo_dba_nextkey)
	CREX_FE(dba_delete, arginfo_dba_delete)
	CREX_FE(dba_insert, arginfo_dba_insert)
	CREX_FE(dba_replace, arginfo_dba_replace)
	CREX_FE(dba_optimize, arginfo_dba_optimize)
	CREX_FE(dba_sync, arginfo_dba_sync)
	CREX_FE(dba_handlers, arginfo_dba_handlers)
	CREX_FE(dba_list, arginfo_dba_list)
	CREX_FE_END
};

static void register_dba_symbols(int module_number)
{
#if defined(DBA_LMDB)
	REGISTER_LONG_CONSTANT("DBA_LMDB_USE_SUB_DIR", 0, CONST_PERSISTENT);
#endif
#if defined(DBA_LMDB)
	REGISTER_LONG_CONSTANT("DBA_LMDB_NO_SUB_DIR", MDB_NOSUBDIR, CONST_PERSISTENT);
#endif
}
