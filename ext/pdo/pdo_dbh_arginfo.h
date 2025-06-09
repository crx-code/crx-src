/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 7dcba884671fd90b891fab7e3f0d4cc9a4ac76a1 */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_PDO___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, dsn, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, username, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, password, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDO_beginTransaction, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

#define arginfo_class_PDO_commit arginfo_class_PDO_beginTransaction

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDO_errorCode, 0, 0, IS_STRING, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDO_errorInfo, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_PDO_exec, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, statement, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDO_getAttribute, 0, 1, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, attribute, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_PDO_getAvailableDrivers arginfo_class_PDO_errorInfo

#define arginfo_class_PDO_inTransaction arginfo_class_PDO_beginTransaction

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_PDO_lastInsertId, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, name, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_PDO_prepare, 0, 1, PDOStatement, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, query, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_PDO_query, 0, 1, PDOStatement, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, query, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, fetchMode, IS_LONG, 1, "null")
	CREX_ARG_VARIADIC_TYPE_INFO(0, fetchModeArgs, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_PDO_quote, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_LONG, 0, "PDO::PARAM_STR")
CREX_END_ARG_INFO()

#define arginfo_class_PDO_rollBack arginfo_class_PDO_beginTransaction

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_PDO_setAttribute, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, attribute, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()


CREX_METHOD(PDO, __main);
CREX_METHOD(PDO, beginTransaction);
CREX_METHOD(PDO, commit);
CREX_METHOD(PDO, errorCode);
CREX_METHOD(PDO, errorInfo);
CREX_METHOD(PDO, exec);
CREX_METHOD(PDO, getAttribute);
CREX_METHOD(PDO, getAvailableDrivers);
CREX_METHOD(PDO, inTransaction);
CREX_METHOD(PDO, lastInsertId);
CREX_METHOD(PDO, prepare);
CREX_METHOD(PDO, query);
CREX_METHOD(PDO, quote);
CREX_METHOD(PDO, rollBack);
CREX_METHOD(PDO, setAttribute);


static const crex_function_entry class_PDO_methods[] = {
	CREX_ME(PDO, __main, arginfo_class_PDO___main, CREX_ACC_PUBLIC)
	CREX_ME(PDO, beginTransaction, arginfo_class_PDO_beginTransaction, CREX_ACC_PUBLIC)
	CREX_ME(PDO, commit, arginfo_class_PDO_commit, CREX_ACC_PUBLIC)
	CREX_ME(PDO, errorCode, arginfo_class_PDO_errorCode, CREX_ACC_PUBLIC)
	CREX_ME(PDO, errorInfo, arginfo_class_PDO_errorInfo, CREX_ACC_PUBLIC)
	CREX_ME(PDO, exec, arginfo_class_PDO_exec, CREX_ACC_PUBLIC)
	CREX_ME(PDO, getAttribute, arginfo_class_PDO_getAttribute, CREX_ACC_PUBLIC)
	CREX_ME(PDO, getAvailableDrivers, arginfo_class_PDO_getAvailableDrivers, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(PDO, inTransaction, arginfo_class_PDO_inTransaction, CREX_ACC_PUBLIC)
	CREX_ME(PDO, lastInsertId, arginfo_class_PDO_lastInsertId, CREX_ACC_PUBLIC)
	CREX_ME(PDO, prepare, arginfo_class_PDO_prepare, CREX_ACC_PUBLIC)
	CREX_ME(PDO, query, arginfo_class_PDO_query, CREX_ACC_PUBLIC)
	CREX_ME(PDO, quote, arginfo_class_PDO_quote, CREX_ACC_PUBLIC)
	CREX_ME(PDO, rollBack, arginfo_class_PDO_rollBack, CREX_ACC_PUBLIC)
	CREX_ME(PDO, setAttribute, arginfo_class_PDO_setAttribute, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_PDO(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "PDO", class_PDO_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;

	zval const_PARAM_NULL_value;
	ZVAL_LONG(&const_PARAM_NULL_value, LONG_CONST(PDO_PARAM_NULL));
	crex_string *const_PARAM_NULL_name = crex_string_init_interned("PARAM_NULL", sizeof("PARAM_NULL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PARAM_NULL_name, &const_PARAM_NULL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PARAM_NULL_name);
	CREX_ASSERT(LONG_CONST(PDO_PARAM_NULL) == 0);

	zval const_PARAM_BOOL_value;
	ZVAL_LONG(&const_PARAM_BOOL_value, LONG_CONST(PDO_PARAM_BOOL));
	crex_string *const_PARAM_BOOL_name = crex_string_init_interned("PARAM_BOOL", sizeof("PARAM_BOOL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PARAM_BOOL_name, &const_PARAM_BOOL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PARAM_BOOL_name);
	CREX_ASSERT(LONG_CONST(PDO_PARAM_BOOL) == 5);

	zval const_PARAM_INT_value;
	ZVAL_LONG(&const_PARAM_INT_value, LONG_CONST(PDO_PARAM_INT));
	crex_string *const_PARAM_INT_name = crex_string_init_interned("PARAM_INT", sizeof("PARAM_INT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PARAM_INT_name, &const_PARAM_INT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PARAM_INT_name);
	CREX_ASSERT(LONG_CONST(PDO_PARAM_INT) == 1);

	zval const_PARAM_STR_value;
	ZVAL_LONG(&const_PARAM_STR_value, LONG_CONST(PDO_PARAM_STR));
	crex_string *const_PARAM_STR_name = crex_string_init_interned("PARAM_STR", sizeof("PARAM_STR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PARAM_STR_name, &const_PARAM_STR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PARAM_STR_name);
	CREX_ASSERT(LONG_CONST(PDO_PARAM_STR) == 2);

	zval const_PARAM_LOB_value;
	ZVAL_LONG(&const_PARAM_LOB_value, LONG_CONST(PDO_PARAM_LOB));
	crex_string *const_PARAM_LOB_name = crex_string_init_interned("PARAM_LOB", sizeof("PARAM_LOB") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PARAM_LOB_name, &const_PARAM_LOB_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PARAM_LOB_name);
	CREX_ASSERT(LONG_CONST(PDO_PARAM_LOB) == 3);

	zval const_PARAM_STMT_value;
	ZVAL_LONG(&const_PARAM_STMT_value, LONG_CONST(PDO_PARAM_STMT));
	crex_string *const_PARAM_STMT_name = crex_string_init_interned("PARAM_STMT", sizeof("PARAM_STMT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PARAM_STMT_name, &const_PARAM_STMT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PARAM_STMT_name);
	CREX_ASSERT(LONG_CONST(PDO_PARAM_STMT) == 4);

	zval const_PARAM_INPUT_OUTPUT_value;
	ZVAL_LONG(&const_PARAM_INPUT_OUTPUT_value, LONG_CONST(PDO_PARAM_INPUT_OUTPUT));
	crex_string *const_PARAM_INPUT_OUTPUT_name = crex_string_init_interned("PARAM_INPUT_OUTPUT", sizeof("PARAM_INPUT_OUTPUT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PARAM_INPUT_OUTPUT_name, &const_PARAM_INPUT_OUTPUT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PARAM_INPUT_OUTPUT_name);

	zval const_PARAM_STR_NATL_value;
	ZVAL_LONG(&const_PARAM_STR_NATL_value, LONG_CONST(PDO_PARAM_STR_NATL));
	crex_string *const_PARAM_STR_NATL_name = crex_string_init_interned("PARAM_STR_NATL", sizeof("PARAM_STR_NATL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PARAM_STR_NATL_name, &const_PARAM_STR_NATL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PARAM_STR_NATL_name);

	zval const_PARAM_STR_CHAR_value;
	ZVAL_LONG(&const_PARAM_STR_CHAR_value, LONG_CONST(PDO_PARAM_STR_CHAR));
	crex_string *const_PARAM_STR_CHAR_name = crex_string_init_interned("PARAM_STR_CHAR", sizeof("PARAM_STR_CHAR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PARAM_STR_CHAR_name, &const_PARAM_STR_CHAR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PARAM_STR_CHAR_name);

	zval const_PARAM_EVT_ALLOC_value;
	ZVAL_LONG(&const_PARAM_EVT_ALLOC_value, LONG_CONST(PDO_PARAM_EVT_ALLOC));
	crex_string *const_PARAM_EVT_ALLOC_name = crex_string_init_interned("PARAM_EVT_ALLOC", sizeof("PARAM_EVT_ALLOC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PARAM_EVT_ALLOC_name, &const_PARAM_EVT_ALLOC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PARAM_EVT_ALLOC_name);

	zval const_PARAM_EVT_FREE_value;
	ZVAL_LONG(&const_PARAM_EVT_FREE_value, LONG_CONST(PDO_PARAM_EVT_FREE));
	crex_string *const_PARAM_EVT_FREE_name = crex_string_init_interned("PARAM_EVT_FREE", sizeof("PARAM_EVT_FREE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PARAM_EVT_FREE_name, &const_PARAM_EVT_FREE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PARAM_EVT_FREE_name);

	zval const_PARAM_EVT_EXEC_PRE_value;
	ZVAL_LONG(&const_PARAM_EVT_EXEC_PRE_value, LONG_CONST(PDO_PARAM_EVT_EXEC_PRE));
	crex_string *const_PARAM_EVT_EXEC_PRE_name = crex_string_init_interned("PARAM_EVT_EXEC_PRE", sizeof("PARAM_EVT_EXEC_PRE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PARAM_EVT_EXEC_PRE_name, &const_PARAM_EVT_EXEC_PRE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PARAM_EVT_EXEC_PRE_name);

	zval const_PARAM_EVT_EXEC_POST_value;
	ZVAL_LONG(&const_PARAM_EVT_EXEC_POST_value, LONG_CONST(PDO_PARAM_EVT_EXEC_POST));
	crex_string *const_PARAM_EVT_EXEC_POST_name = crex_string_init_interned("PARAM_EVT_EXEC_POST", sizeof("PARAM_EVT_EXEC_POST") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PARAM_EVT_EXEC_POST_name, &const_PARAM_EVT_EXEC_POST_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PARAM_EVT_EXEC_POST_name);

	zval const_PARAM_EVT_FETCH_PRE_value;
	ZVAL_LONG(&const_PARAM_EVT_FETCH_PRE_value, LONG_CONST(PDO_PARAM_EVT_FETCH_PRE));
	crex_string *const_PARAM_EVT_FETCH_PRE_name = crex_string_init_interned("PARAM_EVT_FETCH_PRE", sizeof("PARAM_EVT_FETCH_PRE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PARAM_EVT_FETCH_PRE_name, &const_PARAM_EVT_FETCH_PRE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PARAM_EVT_FETCH_PRE_name);

	zval const_PARAM_EVT_FETCH_POST_value;
	ZVAL_LONG(&const_PARAM_EVT_FETCH_POST_value, LONG_CONST(PDO_PARAM_EVT_FETCH_POST));
	crex_string *const_PARAM_EVT_FETCH_POST_name = crex_string_init_interned("PARAM_EVT_FETCH_POST", sizeof("PARAM_EVT_FETCH_POST") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PARAM_EVT_FETCH_POST_name, &const_PARAM_EVT_FETCH_POST_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PARAM_EVT_FETCH_POST_name);

	zval const_PARAM_EVT_NORMALIZE_value;
	ZVAL_LONG(&const_PARAM_EVT_NORMALIZE_value, LONG_CONST(PDO_PARAM_EVT_NORMALIZE));
	crex_string *const_PARAM_EVT_NORMALIZE_name = crex_string_init_interned("PARAM_EVT_NORMALIZE", sizeof("PARAM_EVT_NORMALIZE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PARAM_EVT_NORMALIZE_name, &const_PARAM_EVT_NORMALIZE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PARAM_EVT_NORMALIZE_name);

	zval const_FETCH_DEFAULT_value;
	ZVAL_LONG(&const_FETCH_DEFAULT_value, LONG_CONST(PDO_FETCH_USE_DEFAULT));
	crex_string *const_FETCH_DEFAULT_name = crex_string_init_interned("FETCH_DEFAULT", sizeof("FETCH_DEFAULT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_DEFAULT_name, &const_FETCH_DEFAULT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_DEFAULT_name);

	zval const_FETCH_LAZY_value;
	ZVAL_LONG(&const_FETCH_LAZY_value, LONG_CONST(PDO_FETCH_LAZY));
	crex_string *const_FETCH_LAZY_name = crex_string_init_interned("FETCH_LAZY", sizeof("FETCH_LAZY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_LAZY_name, &const_FETCH_LAZY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_LAZY_name);

	zval const_FETCH_ASSOC_value;
	ZVAL_LONG(&const_FETCH_ASSOC_value, LONG_CONST(PDO_FETCH_ASSOC));
	crex_string *const_FETCH_ASSOC_name = crex_string_init_interned("FETCH_ASSOC", sizeof("FETCH_ASSOC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_ASSOC_name, &const_FETCH_ASSOC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_ASSOC_name);

	zval const_FETCH_NUM_value;
	ZVAL_LONG(&const_FETCH_NUM_value, LONG_CONST(PDO_FETCH_NUM));
	crex_string *const_FETCH_NUM_name = crex_string_init_interned("FETCH_NUM", sizeof("FETCH_NUM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_NUM_name, &const_FETCH_NUM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_NUM_name);

	zval const_FETCH_BOTH_value;
	ZVAL_LONG(&const_FETCH_BOTH_value, LONG_CONST(PDO_FETCH_BOTH));
	crex_string *const_FETCH_BOTH_name = crex_string_init_interned("FETCH_BOTH", sizeof("FETCH_BOTH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_BOTH_name, &const_FETCH_BOTH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_BOTH_name);

	zval const_FETCH_OBJ_value;
	ZVAL_LONG(&const_FETCH_OBJ_value, LONG_CONST(PDO_FETCH_OBJ));
	crex_string *const_FETCH_OBJ_name = crex_string_init_interned("FETCH_OBJ", sizeof("FETCH_OBJ") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_OBJ_name, &const_FETCH_OBJ_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_OBJ_name);

	zval const_FETCH_BOUND_value;
	ZVAL_LONG(&const_FETCH_BOUND_value, LONG_CONST(PDO_FETCH_BOUND));
	crex_string *const_FETCH_BOUND_name = crex_string_init_interned("FETCH_BOUND", sizeof("FETCH_BOUND") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_BOUND_name, &const_FETCH_BOUND_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_BOUND_name);

	zval const_FETCH_COLUMN_value;
	ZVAL_LONG(&const_FETCH_COLUMN_value, LONG_CONST(PDO_FETCH_COLUMN));
	crex_string *const_FETCH_COLUMN_name = crex_string_init_interned("FETCH_COLUMN", sizeof("FETCH_COLUMN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_COLUMN_name, &const_FETCH_COLUMN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_COLUMN_name);

	zval const_FETCH_CLASS_value;
	ZVAL_LONG(&const_FETCH_CLASS_value, LONG_CONST(PDO_FETCH_CLASS));
	crex_string *const_FETCH_CLASS_name = crex_string_init_interned("FETCH_CLASS", sizeof("FETCH_CLASS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_CLASS_name, &const_FETCH_CLASS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_CLASS_name);

	zval const_FETCH_INTO_value;
	ZVAL_LONG(&const_FETCH_INTO_value, LONG_CONST(PDO_FETCH_INTO));
	crex_string *const_FETCH_INTO_name = crex_string_init_interned("FETCH_INTO", sizeof("FETCH_INTO") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_INTO_name, &const_FETCH_INTO_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_INTO_name);

	zval const_FETCH_FUNC_value;
	ZVAL_LONG(&const_FETCH_FUNC_value, LONG_CONST(PDO_FETCH_FUNC));
	crex_string *const_FETCH_FUNC_name = crex_string_init_interned("FETCH_FUNC", sizeof("FETCH_FUNC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_FUNC_name, &const_FETCH_FUNC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_FUNC_name);

	zval const_FETCH_GROUP_value;
	ZVAL_LONG(&const_FETCH_GROUP_value, LONG_CONST(PDO_FETCH_GROUP));
	crex_string *const_FETCH_GROUP_name = crex_string_init_interned("FETCH_GROUP", sizeof("FETCH_GROUP") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_GROUP_name, &const_FETCH_GROUP_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_GROUP_name);

	zval const_FETCH_UNIQUE_value;
	ZVAL_LONG(&const_FETCH_UNIQUE_value, LONG_CONST(PDO_FETCH_UNIQUE));
	crex_string *const_FETCH_UNIQUE_name = crex_string_init_interned("FETCH_UNIQUE", sizeof("FETCH_UNIQUE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_UNIQUE_name, &const_FETCH_UNIQUE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_UNIQUE_name);

	zval const_FETCH_KEY_PAIR_value;
	ZVAL_LONG(&const_FETCH_KEY_PAIR_value, LONG_CONST(PDO_FETCH_KEY_PAIR));
	crex_string *const_FETCH_KEY_PAIR_name = crex_string_init_interned("FETCH_KEY_PAIR", sizeof("FETCH_KEY_PAIR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_KEY_PAIR_name, &const_FETCH_KEY_PAIR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_KEY_PAIR_name);

	zval const_FETCH_CLASSTYPE_value;
	ZVAL_LONG(&const_FETCH_CLASSTYPE_value, LONG_CONST(PDO_FETCH_CLASSTYPE));
	crex_string *const_FETCH_CLASSTYPE_name = crex_string_init_interned("FETCH_CLASSTYPE", sizeof("FETCH_CLASSTYPE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_CLASSTYPE_name, &const_FETCH_CLASSTYPE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_CLASSTYPE_name);

	zval const_FETCH_SERIALIZE_value;
	ZVAL_LONG(&const_FETCH_SERIALIZE_value, LONG_CONST(PDO_FETCH_SERIALIZE));
	crex_string *const_FETCH_SERIALIZE_name = crex_string_init_interned("FETCH_SERIALIZE", sizeof("FETCH_SERIALIZE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_SERIALIZE_name, &const_FETCH_SERIALIZE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_SERIALIZE_name);

	zval const_FETCH_PROPS_LATE_value;
	ZVAL_LONG(&const_FETCH_PROPS_LATE_value, LONG_CONST(PDO_FETCH_PROPS_LATE));
	crex_string *const_FETCH_PROPS_LATE_name = crex_string_init_interned("FETCH_PROPS_LATE", sizeof("FETCH_PROPS_LATE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_PROPS_LATE_name, &const_FETCH_PROPS_LATE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_PROPS_LATE_name);

	zval const_FETCH_NAMED_value;
	ZVAL_LONG(&const_FETCH_NAMED_value, LONG_CONST(PDO_FETCH_NAMED));
	crex_string *const_FETCH_NAMED_name = crex_string_init_interned("FETCH_NAMED", sizeof("FETCH_NAMED") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_NAMED_name, &const_FETCH_NAMED_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_NAMED_name);

	zval const_ATTR_AUTOCOMMIT_value;
	ZVAL_LONG(&const_ATTR_AUTOCOMMIT_value, LONG_CONST(PDO_ATTR_AUTOCOMMIT));
	crex_string *const_ATTR_AUTOCOMMIT_name = crex_string_init_interned("ATTR_AUTOCOMMIT", sizeof("ATTR_AUTOCOMMIT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_AUTOCOMMIT_name, &const_ATTR_AUTOCOMMIT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_AUTOCOMMIT_name);

	zval const_ATTR_PREFETCH_value;
	ZVAL_LONG(&const_ATTR_PREFETCH_value, LONG_CONST(PDO_ATTR_PREFETCH));
	crex_string *const_ATTR_PREFETCH_name = crex_string_init_interned("ATTR_PREFETCH", sizeof("ATTR_PREFETCH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_PREFETCH_name, &const_ATTR_PREFETCH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_PREFETCH_name);

	zval const_ATTR_TIMEOUT_value;
	ZVAL_LONG(&const_ATTR_TIMEOUT_value, LONG_CONST(PDO_ATTR_TIMEOUT));
	crex_string *const_ATTR_TIMEOUT_name = crex_string_init_interned("ATTR_TIMEOUT", sizeof("ATTR_TIMEOUT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_TIMEOUT_name, &const_ATTR_TIMEOUT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_TIMEOUT_name);

	zval const_ATTR_ERRMODE_value;
	ZVAL_LONG(&const_ATTR_ERRMODE_value, LONG_CONST(PDO_ATTR_ERRMODE));
	crex_string *const_ATTR_ERRMODE_name = crex_string_init_interned("ATTR_ERRMODE", sizeof("ATTR_ERRMODE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_ERRMODE_name, &const_ATTR_ERRMODE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_ERRMODE_name);

	zval const_ATTR_SERVER_VERSION_value;
	ZVAL_LONG(&const_ATTR_SERVER_VERSION_value, LONG_CONST(PDO_ATTR_SERVER_VERSION));
	crex_string *const_ATTR_SERVER_VERSION_name = crex_string_init_interned("ATTR_SERVER_VERSION", sizeof("ATTR_SERVER_VERSION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_SERVER_VERSION_name, &const_ATTR_SERVER_VERSION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_SERVER_VERSION_name);

	zval const_ATTR_CLIENT_VERSION_value;
	ZVAL_LONG(&const_ATTR_CLIENT_VERSION_value, LONG_CONST(PDO_ATTR_CLIENT_VERSION));
	crex_string *const_ATTR_CLIENT_VERSION_name = crex_string_init_interned("ATTR_CLIENT_VERSION", sizeof("ATTR_CLIENT_VERSION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_CLIENT_VERSION_name, &const_ATTR_CLIENT_VERSION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_CLIENT_VERSION_name);

	zval const_ATTR_SERVER_INFO_value;
	ZVAL_LONG(&const_ATTR_SERVER_INFO_value, LONG_CONST(PDO_ATTR_SERVER_INFO));
	crex_string *const_ATTR_SERVER_INFO_name = crex_string_init_interned("ATTR_SERVER_INFO", sizeof("ATTR_SERVER_INFO") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_SERVER_INFO_name, &const_ATTR_SERVER_INFO_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_SERVER_INFO_name);

	zval const_ATTR_CONNECTION_STATUS_value;
	ZVAL_LONG(&const_ATTR_CONNECTION_STATUS_value, LONG_CONST(PDO_ATTR_CONNECTION_STATUS));
	crex_string *const_ATTR_CONNECTION_STATUS_name = crex_string_init_interned("ATTR_CONNECTION_STATUS", sizeof("ATTR_CONNECTION_STATUS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_CONNECTION_STATUS_name, &const_ATTR_CONNECTION_STATUS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_CONNECTION_STATUS_name);

	zval const_ATTR_CASE_value;
	ZVAL_LONG(&const_ATTR_CASE_value, LONG_CONST(PDO_ATTR_CASE));
	crex_string *const_ATTR_CASE_name = crex_string_init_interned("ATTR_CASE", sizeof("ATTR_CASE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_CASE_name, &const_ATTR_CASE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_CASE_name);

	zval const_ATTR_CURSOR_NAME_value;
	ZVAL_LONG(&const_ATTR_CURSOR_NAME_value, LONG_CONST(PDO_ATTR_CURSOR_NAME));
	crex_string *const_ATTR_CURSOR_NAME_name = crex_string_init_interned("ATTR_CURSOR_NAME", sizeof("ATTR_CURSOR_NAME") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_CURSOR_NAME_name, &const_ATTR_CURSOR_NAME_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_CURSOR_NAME_name);

	zval const_ATTR_CURSOR_value;
	ZVAL_LONG(&const_ATTR_CURSOR_value, LONG_CONST(PDO_ATTR_CURSOR));
	crex_string *const_ATTR_CURSOR_name = crex_string_init_interned("ATTR_CURSOR", sizeof("ATTR_CURSOR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_CURSOR_name, &const_ATTR_CURSOR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_CURSOR_name);

	zval const_ATTR_ORACLE_NULLS_value;
	ZVAL_LONG(&const_ATTR_ORACLE_NULLS_value, LONG_CONST(PDO_ATTR_ORACLE_NULLS));
	crex_string *const_ATTR_ORACLE_NULLS_name = crex_string_init_interned("ATTR_ORACLE_NULLS", sizeof("ATTR_ORACLE_NULLS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_ORACLE_NULLS_name, &const_ATTR_ORACLE_NULLS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_ORACLE_NULLS_name);

	zval const_ATTR_PERSISTENT_value;
	ZVAL_LONG(&const_ATTR_PERSISTENT_value, LONG_CONST(PDO_ATTR_PERSISTENT));
	crex_string *const_ATTR_PERSISTENT_name = crex_string_init_interned("ATTR_PERSISTENT", sizeof("ATTR_PERSISTENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_PERSISTENT_name, &const_ATTR_PERSISTENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_PERSISTENT_name);

	zval const_ATTR_STATEMENT_CLASS_value;
	ZVAL_LONG(&const_ATTR_STATEMENT_CLASS_value, LONG_CONST(PDO_ATTR_STATEMENT_CLASS));
	crex_string *const_ATTR_STATEMENT_CLASS_name = crex_string_init_interned("ATTR_STATEMENT_CLASS", sizeof("ATTR_STATEMENT_CLASS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_STATEMENT_CLASS_name, &const_ATTR_STATEMENT_CLASS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_STATEMENT_CLASS_name);

	zval const_ATTR_FETCH_TABLE_NAMES_value;
	ZVAL_LONG(&const_ATTR_FETCH_TABLE_NAMES_value, LONG_CONST(PDO_ATTR_FETCH_TABLE_NAMES));
	crex_string *const_ATTR_FETCH_TABLE_NAMES_name = crex_string_init_interned("ATTR_FETCH_TABLE_NAMES", sizeof("ATTR_FETCH_TABLE_NAMES") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_FETCH_TABLE_NAMES_name, &const_ATTR_FETCH_TABLE_NAMES_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_FETCH_TABLE_NAMES_name);

	zval const_ATTR_FETCH_CATALOG_NAMES_value;
	ZVAL_LONG(&const_ATTR_FETCH_CATALOG_NAMES_value, LONG_CONST(PDO_ATTR_FETCH_CATALOG_NAMES));
	crex_string *const_ATTR_FETCH_CATALOG_NAMES_name = crex_string_init_interned("ATTR_FETCH_CATALOG_NAMES", sizeof("ATTR_FETCH_CATALOG_NAMES") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_FETCH_CATALOG_NAMES_name, &const_ATTR_FETCH_CATALOG_NAMES_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_FETCH_CATALOG_NAMES_name);

	zval const_ATTR_DRIVER_NAME_value;
	ZVAL_LONG(&const_ATTR_DRIVER_NAME_value, LONG_CONST(PDO_ATTR_DRIVER_NAME));
	crex_string *const_ATTR_DRIVER_NAME_name = crex_string_init_interned("ATTR_DRIVER_NAME", sizeof("ATTR_DRIVER_NAME") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_DRIVER_NAME_name, &const_ATTR_DRIVER_NAME_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_DRIVER_NAME_name);

	zval const_ATTR_STRINGIFY_FETCHES_value;
	ZVAL_LONG(&const_ATTR_STRINGIFY_FETCHES_value, LONG_CONST(PDO_ATTR_STRINGIFY_FETCHES));
	crex_string *const_ATTR_STRINGIFY_FETCHES_name = crex_string_init_interned("ATTR_STRINGIFY_FETCHES", sizeof("ATTR_STRINGIFY_FETCHES") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_STRINGIFY_FETCHES_name, &const_ATTR_STRINGIFY_FETCHES_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_STRINGIFY_FETCHES_name);

	zval const_ATTR_MAX_COLUMN_LEN_value;
	ZVAL_LONG(&const_ATTR_MAX_COLUMN_LEN_value, LONG_CONST(PDO_ATTR_MAX_COLUMN_LEN));
	crex_string *const_ATTR_MAX_COLUMN_LEN_name = crex_string_init_interned("ATTR_MAX_COLUMN_LEN", sizeof("ATTR_MAX_COLUMN_LEN") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_MAX_COLUMN_LEN_name, &const_ATTR_MAX_COLUMN_LEN_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_MAX_COLUMN_LEN_name);

	zval const_ATTR_EMULATE_PREPARES_value;
	ZVAL_LONG(&const_ATTR_EMULATE_PREPARES_value, LONG_CONST(PDO_ATTR_EMULATE_PREPARES));
	crex_string *const_ATTR_EMULATE_PREPARES_name = crex_string_init_interned("ATTR_EMULATE_PREPARES", sizeof("ATTR_EMULATE_PREPARES") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_EMULATE_PREPARES_name, &const_ATTR_EMULATE_PREPARES_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_EMULATE_PREPARES_name);

	zval const_ATTR_DEFAULT_FETCH_MODE_value;
	ZVAL_LONG(&const_ATTR_DEFAULT_FETCH_MODE_value, LONG_CONST(PDO_ATTR_DEFAULT_FETCH_MODE));
	crex_string *const_ATTR_DEFAULT_FETCH_MODE_name = crex_string_init_interned("ATTR_DEFAULT_FETCH_MODE", sizeof("ATTR_DEFAULT_FETCH_MODE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_DEFAULT_FETCH_MODE_name, &const_ATTR_DEFAULT_FETCH_MODE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_DEFAULT_FETCH_MODE_name);

	zval const_ATTR_DEFAULT_STR_PARAM_value;
	ZVAL_LONG(&const_ATTR_DEFAULT_STR_PARAM_value, LONG_CONST(PDO_ATTR_DEFAULT_STR_PARAM));
	crex_string *const_ATTR_DEFAULT_STR_PARAM_name = crex_string_init_interned("ATTR_DEFAULT_STR_PARAM", sizeof("ATTR_DEFAULT_STR_PARAM") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ATTR_DEFAULT_STR_PARAM_name, &const_ATTR_DEFAULT_STR_PARAM_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ATTR_DEFAULT_STR_PARAM_name);

	zval const_ERRMODE_SILENT_value;
	ZVAL_LONG(&const_ERRMODE_SILENT_value, LONG_CONST(PDO_ERRMODE_SILENT));
	crex_string *const_ERRMODE_SILENT_name = crex_string_init_interned("ERRMODE_SILENT", sizeof("ERRMODE_SILENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ERRMODE_SILENT_name, &const_ERRMODE_SILENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ERRMODE_SILENT_name);

	zval const_ERRMODE_WARNING_value;
	ZVAL_LONG(&const_ERRMODE_WARNING_value, LONG_CONST(PDO_ERRMODE_WARNING));
	crex_string *const_ERRMODE_WARNING_name = crex_string_init_interned("ERRMODE_WARNING", sizeof("ERRMODE_WARNING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ERRMODE_WARNING_name, &const_ERRMODE_WARNING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ERRMODE_WARNING_name);

	zval const_ERRMODE_EXCEPTION_value;
	ZVAL_LONG(&const_ERRMODE_EXCEPTION_value, LONG_CONST(PDO_ERRMODE_EXCEPTION));
	crex_string *const_ERRMODE_EXCEPTION_name = crex_string_init_interned("ERRMODE_EXCEPTION", sizeof("ERRMODE_EXCEPTION") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ERRMODE_EXCEPTION_name, &const_ERRMODE_EXCEPTION_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ERRMODE_EXCEPTION_name);

	zval const_CASE_NATURAL_value;
	ZVAL_LONG(&const_CASE_NATURAL_value, LONG_CONST(PDO_CASE_NATURAL));
	crex_string *const_CASE_NATURAL_name = crex_string_init_interned("CASE_NATURAL", sizeof("CASE_NATURAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CASE_NATURAL_name, &const_CASE_NATURAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CASE_NATURAL_name);

	zval const_CASE_LOWER_value;
	ZVAL_LONG(&const_CASE_LOWER_value, LONG_CONST(PDO_CASE_LOWER));
	crex_string *const_CASE_LOWER_name = crex_string_init_interned("CASE_LOWER", sizeof("CASE_LOWER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CASE_LOWER_name, &const_CASE_LOWER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CASE_LOWER_name);

	zval const_CASE_UPPER_value;
	ZVAL_LONG(&const_CASE_UPPER_value, LONG_CONST(PDO_CASE_UPPER));
	crex_string *const_CASE_UPPER_name = crex_string_init_interned("CASE_UPPER", sizeof("CASE_UPPER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CASE_UPPER_name, &const_CASE_UPPER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CASE_UPPER_name);

	zval const_NULL_NATURAL_value;
	ZVAL_LONG(&const_NULL_NATURAL_value, LONG_CONST(PDO_NULL_NATURAL));
	crex_string *const_NULL_NATURAL_name = crex_string_init_interned("NULL_NATURAL", sizeof("NULL_NATURAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NULL_NATURAL_name, &const_NULL_NATURAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NULL_NATURAL_name);

	zval const_NULL_EMPTY_STRING_value;
	ZVAL_LONG(&const_NULL_EMPTY_STRING_value, LONG_CONST(PDO_NULL_EMPTY_STRING));
	crex_string *const_NULL_EMPTY_STRING_name = crex_string_init_interned("NULL_EMPTY_STRING", sizeof("NULL_EMPTY_STRING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NULL_EMPTY_STRING_name, &const_NULL_EMPTY_STRING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NULL_EMPTY_STRING_name);

	zval const_NULL_TO_STRING_value;
	ZVAL_LONG(&const_NULL_TO_STRING_value, LONG_CONST(PDO_NULL_TO_STRING));
	crex_string *const_NULL_TO_STRING_name = crex_string_init_interned("NULL_TO_STRING", sizeof("NULL_TO_STRING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NULL_TO_STRING_name, &const_NULL_TO_STRING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NULL_TO_STRING_name);

	zval const_ERR_NONE_value;
	crex_string *const_ERR_NONE_value_str = crex_string_init(PDO_ERR_NONE, strlen(PDO_ERR_NONE), 1);
	ZVAL_STR(&const_ERR_NONE_value, const_ERR_NONE_value_str);
	crex_string *const_ERR_NONE_name = crex_string_init_interned("ERR_NONE", sizeof("ERR_NONE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ERR_NONE_name, &const_ERR_NONE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ERR_NONE_name);

	zval const_FETCH_ORI_NEXT_value;
	ZVAL_LONG(&const_FETCH_ORI_NEXT_value, LONG_CONST(PDO_FETCH_ORI_NEXT));
	crex_string *const_FETCH_ORI_NEXT_name = crex_string_init_interned("FETCH_ORI_NEXT", sizeof("FETCH_ORI_NEXT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_ORI_NEXT_name, &const_FETCH_ORI_NEXT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_ORI_NEXT_name);

	zval const_FETCH_ORI_PRIOR_value;
	ZVAL_LONG(&const_FETCH_ORI_PRIOR_value, LONG_CONST(PDO_FETCH_ORI_PRIOR));
	crex_string *const_FETCH_ORI_PRIOR_name = crex_string_init_interned("FETCH_ORI_PRIOR", sizeof("FETCH_ORI_PRIOR") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_ORI_PRIOR_name, &const_FETCH_ORI_PRIOR_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_ORI_PRIOR_name);

	zval const_FETCH_ORI_FIRST_value;
	ZVAL_LONG(&const_FETCH_ORI_FIRST_value, LONG_CONST(PDO_FETCH_ORI_FIRST));
	crex_string *const_FETCH_ORI_FIRST_name = crex_string_init_interned("FETCH_ORI_FIRST", sizeof("FETCH_ORI_FIRST") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_ORI_FIRST_name, &const_FETCH_ORI_FIRST_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_ORI_FIRST_name);

	zval const_FETCH_ORI_LAST_value;
	ZVAL_LONG(&const_FETCH_ORI_LAST_value, LONG_CONST(PDO_FETCH_ORI_LAST));
	crex_string *const_FETCH_ORI_LAST_name = crex_string_init_interned("FETCH_ORI_LAST", sizeof("FETCH_ORI_LAST") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_ORI_LAST_name, &const_FETCH_ORI_LAST_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_ORI_LAST_name);

	zval const_FETCH_ORI_ABS_value;
	ZVAL_LONG(&const_FETCH_ORI_ABS_value, LONG_CONST(PDO_FETCH_ORI_ABS));
	crex_string *const_FETCH_ORI_ABS_name = crex_string_init_interned("FETCH_ORI_ABS", sizeof("FETCH_ORI_ABS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_ORI_ABS_name, &const_FETCH_ORI_ABS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_ORI_ABS_name);

	zval const_FETCH_ORI_REL_value;
	ZVAL_LONG(&const_FETCH_ORI_REL_value, LONG_CONST(PDO_FETCH_ORI_REL));
	crex_string *const_FETCH_ORI_REL_name = crex_string_init_interned("FETCH_ORI_REL", sizeof("FETCH_ORI_REL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FETCH_ORI_REL_name, &const_FETCH_ORI_REL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FETCH_ORI_REL_name);

	zval const_CURSOR_FWDONLY_value;
	ZVAL_LONG(&const_CURSOR_FWDONLY_value, LONG_CONST(PDO_CURSOR_FWDONLY));
	crex_string *const_CURSOR_FWDONLY_name = crex_string_init_interned("CURSOR_FWDONLY", sizeof("CURSOR_FWDONLY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CURSOR_FWDONLY_name, &const_CURSOR_FWDONLY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CURSOR_FWDONLY_name);

	zval const_CURSOR_SCROLL_value;
	ZVAL_LONG(&const_CURSOR_SCROLL_value, LONG_CONST(PDO_CURSOR_SCROLL));
	crex_string *const_CURSOR_SCROLL_name = crex_string_init_interned("CURSOR_SCROLL", sizeof("CURSOR_SCROLL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CURSOR_SCROLL_name, &const_CURSOR_SCROLL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CURSOR_SCROLL_name);


	crex_add_parameter_attribute(crex_hash_str_find_ptr(&class_entry->function_table, "__main", sizeof("__main") - 1), 2, ZSTR_KNOWN(CREX_STR_SENSITIVEPARAMETER), 0);

	return class_entry;
}
