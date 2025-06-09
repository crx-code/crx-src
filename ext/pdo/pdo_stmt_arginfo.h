/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: adcbda7b6763141981700bec5d8c5b739f8de767 */

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDOStatement_bindColumn, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_MASK(0, column, MAY_BE_STRING|MAY_BE_LONG, NULL)
	CREX_ARG_TYPE_INFO(1, var, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_LONG, 0, "PDO::PARAM_STR")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, maxLength, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, driverOptions, IS_MIXED, 0, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDOStatement_bindParam, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_MASK(0, param, MAY_BE_STRING|MAY_BE_LONG, NULL)
	CREX_ARG_TYPE_INFO(1, var, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_LONG, 0, "PDO::PARAM_STR")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, maxLength, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, driverOptions, IS_MIXED, 0, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDOStatement_bindValue, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_MASK(0, param, MAY_BE_STRING|MAY_BE_LONG, NULL)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_LONG, 0, "PDO::PARAM_STR")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDOStatement_closeCursor, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDOStatement_columnCount, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDOStatement_debugDumpParams, 0, 0, _IS_BOOL, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDOStatement_errorCode, 0, 0, IS_STRING, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDOStatement_errorInfo, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDOStatement_execute, 0, 0, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, params, IS_ARRAY, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDOStatement_fetch, 0, 0, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "PDO::FETCH_DEFAULT")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, cursorOrientation, IS_LONG, 0, "PDO::FETCH_ORI_NEXT")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, cursorOffset, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDOStatement_fetchAll, 0, 0, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "PDO::FETCH_DEFAULT")
	CREX_ARG_VARIADIC_TYPE_INFO(0, args, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDOStatement_fetchColumn, 0, 0, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, column, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_PDOStatement_fetchObject, 0, 0, MAY_BE_OBJECT|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, class, IS_STRING, 1, "\"stdClass\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, constructorArgs, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDOStatement_getAttribute, 0, 1, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_PDOStatement_getColumnMeta, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, column, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_PDOStatement_nextRowset arginfo_class_PDOStatement_closeCursor

#define arginfo_class_PDOStatement_rowCount arginfo_class_PDOStatement_columnCount

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDOStatement_setAttribute, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, attribute, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_PDOStatement_setFetchMode, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(0, args, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_PDOStatement_getIterator, 0, 0, Iterator, 0)
CREX_END_ARG_INFO()


CREX_METHOD(PDOStatement, bindColumn);
CREX_METHOD(PDOStatement, bindParam);
CREX_METHOD(PDOStatement, bindValue);
CREX_METHOD(PDOStatement, closeCursor);
CREX_METHOD(PDOStatement, columnCount);
CREX_METHOD(PDOStatement, debugDumpParams);
CREX_METHOD(PDOStatement, errorCode);
CREX_METHOD(PDOStatement, errorInfo);
CREX_METHOD(PDOStatement, execute);
CREX_METHOD(PDOStatement, fetch);
CREX_METHOD(PDOStatement, fetchAll);
CREX_METHOD(PDOStatement, fetchColumn);
CREX_METHOD(PDOStatement, fetchObject);
CREX_METHOD(PDOStatement, getAttribute);
CREX_METHOD(PDOStatement, getColumnMeta);
CREX_METHOD(PDOStatement, nextRowset);
CREX_METHOD(PDOStatement, rowCount);
CREX_METHOD(PDOStatement, setAttribute);
CREX_METHOD(PDOStatement, setFetchMode);
CREX_METHOD(PDOStatement, getIterator);


static const crex_function_entry class_PDOStatement_methods[] = {
	CREX_ME(PDOStatement, bindColumn, arginfo_class_PDOStatement_bindColumn, CREX_ACC_PUBLIC)
	CREX_ME(PDOStatement, bindParam, arginfo_class_PDOStatement_bindParam, CREX_ACC_PUBLIC)
	CREX_ME(PDOStatement, bindValue, arginfo_class_PDOStatement_bindValue, CREX_ACC_PUBLIC)
	CREX_ME(PDOStatement, closeCursor, arginfo_class_PDOStatement_closeCursor, CREX_ACC_PUBLIC)
	CREX_ME(PDOStatement, columnCount, arginfo_class_PDOStatement_columnCount, CREX_ACC_PUBLIC)
	CREX_ME(PDOStatement, debugDumpParams, arginfo_class_PDOStatement_debugDumpParams, CREX_ACC_PUBLIC)
	CREX_ME(PDOStatement, errorCode, arginfo_class_PDOStatement_errorCode, CREX_ACC_PUBLIC)
	CREX_ME(PDOStatement, errorInfo, arginfo_class_PDOStatement_errorInfo, CREX_ACC_PUBLIC)
	CREX_ME(PDOStatement, execute, arginfo_class_PDOStatement_execute, CREX_ACC_PUBLIC)
	CREX_ME(PDOStatement, fetch, arginfo_class_PDOStatement_fetch, CREX_ACC_PUBLIC)
	CREX_ME(PDOStatement, fetchAll, arginfo_class_PDOStatement_fetchAll, CREX_ACC_PUBLIC)
	CREX_ME(PDOStatement, fetchColumn, arginfo_class_PDOStatement_fetchColumn, CREX_ACC_PUBLIC)
	CREX_ME(PDOStatement, fetchObject, arginfo_class_PDOStatement_fetchObject, CREX_ACC_PUBLIC)
	CREX_ME(PDOStatement, getAttribute, arginfo_class_PDOStatement_getAttribute, CREX_ACC_PUBLIC)
	CREX_ME(PDOStatement, getColumnMeta, arginfo_class_PDOStatement_getColumnMeta, CREX_ACC_PUBLIC)
	CREX_ME(PDOStatement, nextRowset, arginfo_class_PDOStatement_nextRowset, CREX_ACC_PUBLIC)
	CREX_ME(PDOStatement, rowCount, arginfo_class_PDOStatement_rowCount, CREX_ACC_PUBLIC)
	CREX_ME(PDOStatement, setAttribute, arginfo_class_PDOStatement_setAttribute, CREX_ACC_PUBLIC)
	CREX_ME(PDOStatement, setFetchMode, arginfo_class_PDOStatement_setFetchMode, CREX_ACC_PUBLIC)
	CREX_ME(PDOStatement, getIterator, arginfo_class_PDOStatement_getIterator, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_PDORow_methods[] = {
	CREX_FE_END
};

static crex_class_entry *register_class_PDOStatement(crex_class_entry *class_entry_IteratorAggregate)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "PDOStatement", class_PDOStatement_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;
	crex_class_implements(class_entry, 1, class_entry_IteratorAggregate);

	zval property_queryString_default_value;
	ZVAL_UNDEF(&property_queryString_default_value);
	crex_string *property_queryString_name = crex_string_init("queryString", sizeof("queryString") - 1, 1);
	crex_declare_typed_property(class_entry, property_queryString_name, &property_queryString_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_queryString_name);

	return class_entry;
}

static crex_class_entry *register_class_PDORow(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "PDORow", class_PDORow_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NOT_SERIALIZABLE;

	zval property_queryString_default_value;
	ZVAL_UNDEF(&property_queryString_default_value);
	crex_string *property_queryString_name = crex_string_init("queryString", sizeof("queryString") - 1, 1);
	crex_declare_typed_property(class_entry, property_queryString_name, &property_queryString_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_queryString_name);

	return class_entry;
}
