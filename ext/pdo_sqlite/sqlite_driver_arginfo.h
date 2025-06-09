/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: dc901bd60d17c1a2cdb40a118e2c6cd6eb0396e3 */

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDO_SQLite_Ext_sqliteCreateFunction, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, numArgs, IS_LONG, 0, "-1")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDO_SQLite_Ext_sqliteCreateAggregate, 0, 3, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, step, IS_CALLABLE, 0)
	CREX_ARG_TYPE_INFO(0, finalize, IS_CALLABLE, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, numArgs, IS_LONG, 0, "-1")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDO_SQLite_Ext_sqliteCreateCollation, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
CREX_END_ARG_INFO()


CREX_METHOD(PDO_SQLite_Ext, sqliteCreateFunction);
CREX_METHOD(PDO_SQLite_Ext, sqliteCreateAggregate);
CREX_METHOD(PDO_SQLite_Ext, sqliteCreateCollation);


static const crex_function_entry class_PDO_SQLite_Ext_methods[] = {
	CREX_ME(PDO_SQLite_Ext, sqliteCreateFunction, arginfo_class_PDO_SQLite_Ext_sqliteCreateFunction, CREX_ACC_PUBLIC)
	CREX_ME(PDO_SQLite_Ext, sqliteCreateAggregate, arginfo_class_PDO_SQLite_Ext_sqliteCreateAggregate, CREX_ACC_PUBLIC)
	CREX_ME(PDO_SQLite_Ext, sqliteCreateCollation, arginfo_class_PDO_SQLite_Ext_sqliteCreateCollation, CREX_ACC_PUBLIC)
	CREX_FE_END
};
