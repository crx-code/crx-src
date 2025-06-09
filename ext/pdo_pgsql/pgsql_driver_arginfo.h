/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: b30fa6327876dc1090ee5397253c935e4566a8fe */

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDO_PGSql_Ext_pgsqlCopyFromArray, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, tableName, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, rows, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, separator, IS_STRING, 0, "\"\\t\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, nullAs, IS_STRING, 0, "\"\\\\\\\\N\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, fields, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDO_PGSql_Ext_pgsqlCopyFromFile, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, tableName, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, separator, IS_STRING, 0, "\"\\t\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, nullAs, IS_STRING, 0, "\"\\\\\\\\N\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, fields, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_PDO_PGSql_Ext_pgsqlCopyToArray, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, tableName, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, separator, IS_STRING, 0, "\"\\t\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, nullAs, IS_STRING, 0, "\"\\\\\\\\N\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, fields, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_class_PDO_PGSql_Ext_pgsqlCopyToFile arginfo_class_PDO_PGSql_Ext_pgsqlCopyFromFile

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_PDO_PGSql_Ext_pgsqlLOBCreate, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_PDO_PGSql_Ext_pgsqlLOBOpen, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, oid, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_STRING, 0, "\"rb\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDO_PGSql_Ext_pgsqlLOBUnlink, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, oid, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_PDO_PGSql_Ext_pgsqlGetNotify, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, fetchMode, IS_LONG, 0, "PDO::FETCH_USE_DEFAULT")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeoutMilliseconds, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDO_PGSql_Ext_pgsqlGetPid, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()


CREX_METHOD(PDO_PGSql_Ext, pgsqlCopyFromArray);
CREX_METHOD(PDO_PGSql_Ext, pgsqlCopyFromFile);
CREX_METHOD(PDO_PGSql_Ext, pgsqlCopyToArray);
CREX_METHOD(PDO_PGSql_Ext, pgsqlCopyToFile);
CREX_METHOD(PDO_PGSql_Ext, pgsqlLOBCreate);
CREX_METHOD(PDO_PGSql_Ext, pgsqlLOBOpen);
CREX_METHOD(PDO_PGSql_Ext, pgsqlLOBUnlink);
CREX_METHOD(PDO_PGSql_Ext, pgsqlGetNotify);
CREX_METHOD(PDO_PGSql_Ext, pgsqlGetPid);


static const crex_function_entry class_PDO_PGSql_Ext_methods[] = {
	CREX_ME(PDO_PGSql_Ext, pgsqlCopyFromArray, arginfo_class_PDO_PGSql_Ext_pgsqlCopyFromArray, CREX_ACC_PUBLIC)
	CREX_ME(PDO_PGSql_Ext, pgsqlCopyFromFile, arginfo_class_PDO_PGSql_Ext_pgsqlCopyFromFile, CREX_ACC_PUBLIC)
	CREX_ME(PDO_PGSql_Ext, pgsqlCopyToArray, arginfo_class_PDO_PGSql_Ext_pgsqlCopyToArray, CREX_ACC_PUBLIC)
	CREX_ME(PDO_PGSql_Ext, pgsqlCopyToFile, arginfo_class_PDO_PGSql_Ext_pgsqlCopyToFile, CREX_ACC_PUBLIC)
	CREX_ME(PDO_PGSql_Ext, pgsqlLOBCreate, arginfo_class_PDO_PGSql_Ext_pgsqlLOBCreate, CREX_ACC_PUBLIC)
	CREX_ME(PDO_PGSql_Ext, pgsqlLOBOpen, arginfo_class_PDO_PGSql_Ext_pgsqlLOBOpen, CREX_ACC_PUBLIC)
	CREX_ME(PDO_PGSql_Ext, pgsqlLOBUnlink, arginfo_class_PDO_PGSql_Ext_pgsqlLOBUnlink, CREX_ACC_PUBLIC)
	CREX_ME(PDO_PGSql_Ext, pgsqlGetNotify, arginfo_class_PDO_PGSql_Ext_pgsqlGetNotify, CREX_ACC_PUBLIC)
	CREX_ME(PDO_PGSql_Ext, pgsqlGetPid, arginfo_class_PDO_PGSql_Ext_pgsqlGetPid, CREX_ACC_PUBLIC)
	CREX_FE_END
};
