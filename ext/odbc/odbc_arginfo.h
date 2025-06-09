/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 0417b68a519527b0ee916bad75116ffe4a3ad304 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_odbc_close_all, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_odbc_binmode, 0, 2, _IS_BOOL, 0)
	CREX_ARG_INFO(0, statement)
	CREX_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_odbc_longreadlen, 0, 2, _IS_BOOL, 0)
	CREX_ARG_INFO(0, statement)
	CREX_ARG_TYPE_INFO(0, length, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_odbc_prepare, 0, 0, 2)
	CREX_ARG_INFO(0, odbc)
	CREX_ARG_TYPE_INFO(0, query, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_odbc_execute, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, statement)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, params, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_odbc_cursor, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO(0, statement)
CREX_END_ARG_INFO()

#if defined(HAVE_SQLDATASOURCES)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_odbc_data_source, 0, 2, MAY_BE_ARRAY|MAY_BE_NULL|MAY_BE_FALSE)
	CREX_ARG_INFO(0, odbc)
	CREX_ARG_TYPE_INFO(0, fetch_type, IS_LONG, 0)
CREX_END_ARG_INFO()
#endif

#define arginfo_odbc_exec arginfo_odbc_prepare

#define arginfo_odbc_do arginfo_odbc_prepare

#if defined(CRX_ODBC_HAVE_FETCH_HASH)
CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_odbc_fetch_object, 0, 1, stdClass, MAY_BE_FALSE)
	CREX_ARG_INFO(0, statement)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, row, IS_LONG, 0, "-1")
CREX_END_ARG_INFO()
#endif

#if defined(CRX_ODBC_HAVE_FETCH_HASH)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_odbc_fetch_array, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_INFO(0, statement)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, row, IS_LONG, 0, "-1")
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_odbc_fetch_into, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_INFO(0, statement)
	CREX_ARG_INFO(1, array)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, row, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_odbc_fetch_row, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, statement)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, row, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_odbc_result, 0, 2, MAY_BE_STRING|MAY_BE_BOOL|MAY_BE_NULL)
	CREX_ARG_INFO(0, statement)
	CREX_ARG_TYPE_MASK(0, field, MAY_BE_STRING|MAY_BE_LONG, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_odbc_result_all, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_INFO(0, statement)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, format, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_odbc_free_result, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, statement)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_odbc_connect, 0, 0, 3)
	CREX_ARG_TYPE_INFO(0, dsn, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, user, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, password, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, cursor_option, IS_LONG, 0, "SQL_CUR_USE_DRIVER")
CREX_END_ARG_INFO()

#define arginfo_odbc_pconnect arginfo_odbc_connect

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_odbc_close, 0, 1, IS_VOID, 0)
	CREX_ARG_INFO(0, odbc)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_odbc_num_rows, 0, 1, IS_LONG, 0)
	CREX_ARG_INFO(0, statement)
CREX_END_ARG_INFO()

#if !defined(HAVE_SOLID) && !defined(HAVE_SOLID_30)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_odbc_next_result, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, statement)
CREX_END_ARG_INFO()
#endif

#define arginfo_odbc_num_fields arginfo_odbc_num_rows

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_odbc_field_name, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO(0, statement)
	CREX_ARG_TYPE_INFO(0, field, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_odbc_field_type arginfo_odbc_field_name

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_odbc_field_len, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_INFO(0, statement)
	CREX_ARG_TYPE_INFO(0, field, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_odbc_field_precision arginfo_odbc_field_len

#define arginfo_odbc_field_scale arginfo_odbc_field_len

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_odbc_field_num, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_INFO(0, statement)
	CREX_ARG_TYPE_INFO(0, field, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_odbc_autocommit, 0, 1, MAY_BE_LONG|MAY_BE_BOOL)
	CREX_ARG_INFO(0, odbc)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, enable, _IS_BOOL, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_odbc_commit, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, odbc)
CREX_END_ARG_INFO()

#define arginfo_odbc_rollback arginfo_odbc_commit

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_odbc_error, 0, 0, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, odbc, "null")
CREX_END_ARG_INFO()

#define arginfo_odbc_errormsg arginfo_odbc_error

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_odbc_setoption, 0, 4, _IS_BOOL, 0)
	CREX_ARG_INFO(0, odbc)
	CREX_ARG_TYPE_INFO(0, which, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, option, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_odbc_tables, 0, 0, 1)
	CREX_ARG_INFO(0, odbc)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, catalog, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, schema, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, table, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, types, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_odbc_columns, 0, 0, 1)
	CREX_ARG_INFO(0, odbc)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, catalog, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, schema, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, table, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, column, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_odbc_gettypeinfo, 0, 0, 1)
	CREX_ARG_INFO(0, odbc)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data_type, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_odbc_primarykeys, 0, 0, 4)
	CREX_ARG_INFO(0, odbc)
	CREX_ARG_TYPE_INFO(0, catalog, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, schema, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, table, IS_STRING, 0)
CREX_END_ARG_INFO()

#if !defined(HAVE_SOLID) && !defined(HAVE_SOLID_30) && !defined(HAVE_SOLID_35)
CREX_BEGIN_ARG_INFO_EX(arginfo_odbc_procedurecolumns, 0, 0, 1)
	CREX_ARG_INFO(0, odbc)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, catalog, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, schema, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, procedure, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, column, IS_STRING, 1, "null")
CREX_END_ARG_INFO()
#endif

#if !defined(HAVE_SOLID) && !defined(HAVE_SOLID_30) && !defined(HAVE_SOLID_35)
CREX_BEGIN_ARG_INFO_EX(arginfo_odbc_procedures, 0, 0, 1)
	CREX_ARG_INFO(0, odbc)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, catalog, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, schema, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, procedure, IS_STRING, 1, "null")
CREX_END_ARG_INFO()
#endif

#if !defined(HAVE_SOLID) && !defined(HAVE_SOLID_30) && !defined(HAVE_SOLID_35)
CREX_BEGIN_ARG_INFO_EX(arginfo_odbc_foreignkeys, 0, 0, 7)
	CREX_ARG_INFO(0, odbc)
	CREX_ARG_TYPE_INFO(0, pk_catalog, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, pk_schema, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, pk_table, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, fk_catalog, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, fk_schema, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, fk_table, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_INFO_EX(arginfo_odbc_specialcolumns, 0, 0, 7)
	CREX_ARG_INFO(0, odbc)
	CREX_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, catalog, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, schema, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, table, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, scope, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, nullable, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_odbc_statistics, 0, 0, 6)
	CREX_ARG_INFO(0, odbc)
	CREX_ARG_TYPE_INFO(0, catalog, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, schema, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, table, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, unique, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, accuracy, IS_LONG, 0)
CREX_END_ARG_INFO()

#if !defined(HAVE_DBMAKER) && !defined(HAVE_SOLID) && !defined(HAVE_SOLID_30) &&!defined(HAVE_SOLID_35)
CREX_BEGIN_ARG_INFO_EX(arginfo_odbc_tableprivileges, 0, 0, 4)
	CREX_ARG_INFO(0, odbc)
	CREX_ARG_TYPE_INFO(0, catalog, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, schema, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, table, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#if !defined(HAVE_DBMAKER) && !defined(HAVE_SOLID) && !defined(HAVE_SOLID_30) &&!defined(HAVE_SOLID_35)
CREX_BEGIN_ARG_INFO_EX(arginfo_odbc_columnprivileges, 0, 0, 5)
	CREX_ARG_INFO(0, odbc)
	CREX_ARG_TYPE_INFO(0, catalog, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, schema, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, table, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, column, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_odbc_connection_string_is_quoted, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, str, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_odbc_connection_string_should_quote arginfo_odbc_connection_string_is_quoted

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_odbc_connection_string_quote, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, str, IS_STRING, 0)
CREX_END_ARG_INFO()


CREX_FUNCTION(odbc_close_all);
CREX_FUNCTION(odbc_binmode);
CREX_FUNCTION(odbc_longreadlen);
CREX_FUNCTION(odbc_prepare);
CREX_FUNCTION(odbc_execute);
CREX_FUNCTION(odbc_cursor);
#if defined(HAVE_SQLDATASOURCES)
CREX_FUNCTION(odbc_data_source);
#endif
CREX_FUNCTION(odbc_exec);
#if defined(CRX_ODBC_HAVE_FETCH_HASH)
CREX_FUNCTION(odbc_fetch_object);
#endif
#if defined(CRX_ODBC_HAVE_FETCH_HASH)
CREX_FUNCTION(odbc_fetch_array);
#endif
CREX_FUNCTION(odbc_fetch_into);
CREX_FUNCTION(odbc_fetch_row);
CREX_FUNCTION(odbc_result);
CREX_FUNCTION(odbc_result_all);
CREX_FUNCTION(odbc_free_result);
CREX_FUNCTION(odbc_connect);
CREX_FUNCTION(odbc_pconnect);
CREX_FUNCTION(odbc_close);
CREX_FUNCTION(odbc_num_rows);
#if !defined(HAVE_SOLID) && !defined(HAVE_SOLID_30)
CREX_FUNCTION(odbc_next_result);
#endif
CREX_FUNCTION(odbc_num_fields);
CREX_FUNCTION(odbc_field_name);
CREX_FUNCTION(odbc_field_type);
CREX_FUNCTION(odbc_field_len);
CREX_FUNCTION(odbc_field_scale);
CREX_FUNCTION(odbc_field_num);
CREX_FUNCTION(odbc_autocommit);
CREX_FUNCTION(odbc_commit);
CREX_FUNCTION(odbc_rollback);
CREX_FUNCTION(odbc_error);
CREX_FUNCTION(odbc_errormsg);
CREX_FUNCTION(odbc_setoption);
CREX_FUNCTION(odbc_tables);
CREX_FUNCTION(odbc_columns);
CREX_FUNCTION(odbc_gettypeinfo);
CREX_FUNCTION(odbc_primarykeys);
#if !defined(HAVE_SOLID) && !defined(HAVE_SOLID_30) && !defined(HAVE_SOLID_35)
CREX_FUNCTION(odbc_procedurecolumns);
#endif
#if !defined(HAVE_SOLID) && !defined(HAVE_SOLID_30) && !defined(HAVE_SOLID_35)
CREX_FUNCTION(odbc_procedures);
#endif
#if !defined(HAVE_SOLID) && !defined(HAVE_SOLID_30) && !defined(HAVE_SOLID_35)
CREX_FUNCTION(odbc_foreignkeys);
#endif
CREX_FUNCTION(odbc_specialcolumns);
CREX_FUNCTION(odbc_statistics);
#if !defined(HAVE_DBMAKER) && !defined(HAVE_SOLID) && !defined(HAVE_SOLID_30) &&!defined(HAVE_SOLID_35)
CREX_FUNCTION(odbc_tableprivileges);
#endif
#if !defined(HAVE_DBMAKER) && !defined(HAVE_SOLID) && !defined(HAVE_SOLID_30) &&!defined(HAVE_SOLID_35)
CREX_FUNCTION(odbc_columnprivileges);
#endif
CREX_FUNCTION(odbc_connection_string_is_quoted);
CREX_FUNCTION(odbc_connection_string_should_quote);
CREX_FUNCTION(odbc_connection_string_quote);


static const crex_function_entry ext_functions[] = {
	CREX_FE(odbc_close_all, arginfo_odbc_close_all)
	CREX_FE(odbc_binmode, arginfo_odbc_binmode)
	CREX_FE(odbc_longreadlen, arginfo_odbc_longreadlen)
	CREX_FE(odbc_prepare, arginfo_odbc_prepare)
	CREX_FE(odbc_execute, arginfo_odbc_execute)
	CREX_FE(odbc_cursor, arginfo_odbc_cursor)
#if defined(HAVE_SQLDATASOURCES)
	CREX_FE(odbc_data_source, arginfo_odbc_data_source)
#endif
	CREX_FE(odbc_exec, arginfo_odbc_exec)
	CREX_FALIAS(odbc_do, odbc_exec, arginfo_odbc_do)
#if defined(CRX_ODBC_HAVE_FETCH_HASH)
	CREX_FE(odbc_fetch_object, arginfo_odbc_fetch_object)
#endif
#if defined(CRX_ODBC_HAVE_FETCH_HASH)
	CREX_FE(odbc_fetch_array, arginfo_odbc_fetch_array)
#endif
	CREX_FE(odbc_fetch_into, arginfo_odbc_fetch_into)
	CREX_FE(odbc_fetch_row, arginfo_odbc_fetch_row)
	CREX_FE(odbc_result, arginfo_odbc_result)
	CREX_DEP_FE(odbc_result_all, arginfo_odbc_result_all)
	CREX_FE(odbc_free_result, arginfo_odbc_free_result)
	CREX_FE(odbc_connect, arginfo_odbc_connect)
	CREX_FE(odbc_pconnect, arginfo_odbc_pconnect)
	CREX_FE(odbc_close, arginfo_odbc_close)
	CREX_FE(odbc_num_rows, arginfo_odbc_num_rows)
#if !defined(HAVE_SOLID) && !defined(HAVE_SOLID_30)
	CREX_FE(odbc_next_result, arginfo_odbc_next_result)
#endif
	CREX_FE(odbc_num_fields, arginfo_odbc_num_fields)
	CREX_FE(odbc_field_name, arginfo_odbc_field_name)
	CREX_FE(odbc_field_type, arginfo_odbc_field_type)
	CREX_FE(odbc_field_len, arginfo_odbc_field_len)
	CREX_FALIAS(odbc_field_precision, odbc_field_len, arginfo_odbc_field_precision)
	CREX_FE(odbc_field_scale, arginfo_odbc_field_scale)
	CREX_FE(odbc_field_num, arginfo_odbc_field_num)
	CREX_FE(odbc_autocommit, arginfo_odbc_autocommit)
	CREX_FE(odbc_commit, arginfo_odbc_commit)
	CREX_FE(odbc_rollback, arginfo_odbc_rollback)
	CREX_FE(odbc_error, arginfo_odbc_error)
	CREX_FE(odbc_errormsg, arginfo_odbc_errormsg)
	CREX_FE(odbc_setoption, arginfo_odbc_setoption)
	CREX_FE(odbc_tables, arginfo_odbc_tables)
	CREX_FE(odbc_columns, arginfo_odbc_columns)
	CREX_FE(odbc_gettypeinfo, arginfo_odbc_gettypeinfo)
	CREX_FE(odbc_primarykeys, arginfo_odbc_primarykeys)
#if !defined(HAVE_SOLID) && !defined(HAVE_SOLID_30) && !defined(HAVE_SOLID_35)
	CREX_FE(odbc_procedurecolumns, arginfo_odbc_procedurecolumns)
#endif
#if !defined(HAVE_SOLID) && !defined(HAVE_SOLID_30) && !defined(HAVE_SOLID_35)
	CREX_FE(odbc_procedures, arginfo_odbc_procedures)
#endif
#if !defined(HAVE_SOLID) && !defined(HAVE_SOLID_30) && !defined(HAVE_SOLID_35)
	CREX_FE(odbc_foreignkeys, arginfo_odbc_foreignkeys)
#endif
	CREX_FE(odbc_specialcolumns, arginfo_odbc_specialcolumns)
	CREX_FE(odbc_statistics, arginfo_odbc_statistics)
#if !defined(HAVE_DBMAKER) && !defined(HAVE_SOLID) && !defined(HAVE_SOLID_30) &&!defined(HAVE_SOLID_35)
	CREX_FE(odbc_tableprivileges, arginfo_odbc_tableprivileges)
#endif
#if !defined(HAVE_DBMAKER) && !defined(HAVE_SOLID) && !defined(HAVE_SOLID_30) &&!defined(HAVE_SOLID_35)
	CREX_FE(odbc_columnprivileges, arginfo_odbc_columnprivileges)
#endif
	CREX_FE(odbc_connection_string_is_quoted, arginfo_odbc_connection_string_is_quoted)
	CREX_FE(odbc_connection_string_should_quote, arginfo_odbc_connection_string_should_quote)
	CREX_FE(odbc_connection_string_quote, arginfo_odbc_connection_string_quote)
	CREX_FE_END
};

static void register_odbc_symbols(int module_number)
{
	REGISTER_STRING_CONSTANT("ODBC_TYPE", CRX_ODBC_TYPE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ODBC_BINMODE_PASSTHRU", CRX_ODBC_BINMODE_PASSTHRU, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ODBC_BINMODE_RETURN", CRX_ODBC_BINMODE_RETURN, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ODBC_BINMODE_CONVERT", CRX_ODBC_BINMODE_CONVERT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_ODBC_CURSORS", SQL_ODBC_CURSORS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_CUR_USE_DRIVER", SQL_CUR_USE_DRIVER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_CUR_USE_IF_NEEDED", SQL_CUR_USE_IF_NEEDED, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_CUR_USE_ODBC", SQL_CUR_USE_ODBC, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_CONCURRENCY", SQL_CONCURRENCY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_CONCUR_READ_ONLY", SQL_CONCUR_READ_ONLY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_CONCUR_LOCK", SQL_CONCUR_LOCK, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_CONCUR_ROWVER", SQL_CONCUR_ROWVER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_CONCUR_VALUES", SQL_CONCUR_VALUES, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_CURSOR_TYPE", SQL_CURSOR_TYPE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_CURSOR_FORWARD_ONLY", SQL_CURSOR_FORWARD_ONLY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_CURSOR_KEYSET_DRIVEN", SQL_CURSOR_KEYSET_DRIVEN, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_CURSOR_DYNAMIC", SQL_CURSOR_DYNAMIC, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_CURSOR_STATIC", SQL_CURSOR_STATIC, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_KEYSET_SIZE", SQL_KEYSET_SIZE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_FETCH_FIRST", SQL_FETCH_FIRST, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_FETCH_NEXT", SQL_FETCH_NEXT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_CHAR", SQL_CHAR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_VARCHAR", SQL_VARCHAR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_LONGVARCHAR", SQL_LONGVARCHAR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_DECIMAL", SQL_DECIMAL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_NUMERIC", SQL_NUMERIC, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_BIT", SQL_BIT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_TINYINT", SQL_TINYINT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_SMALLINT", SQL_SMALLINT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_INTEGER", SQL_INTEGER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_BIGINT", SQL_BIGINT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_REAL", SQL_REAL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_FLOAT", SQL_FLOAT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_DOUBLE", SQL_DOUBLE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_BINARY", SQL_BINARY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_VARBINARY", SQL_VARBINARY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_LONGVARBINARY", SQL_LONGVARBINARY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_DATE", SQL_DATE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_TIME", SQL_TIME, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SQL_TIMESTAMP", SQL_TIMESTAMP, CONST_PERSISTENT);
#if (defined(ODBCVER) && (ODBCVER >= 0x0300))
	REGISTER_LONG_CONSTANT("SQL_TYPE_DATE", SQL_TYPE_DATE, CONST_PERSISTENT);
#endif
#if (defined(ODBCVER) && (ODBCVER >= 0x0300))
	REGISTER_LONG_CONSTANT("SQL_TYPE_TIME", SQL_TYPE_TIME, CONST_PERSISTENT);
#endif
#if (defined(ODBCVER) && (ODBCVER >= 0x0300))
	REGISTER_LONG_CONSTANT("SQL_TYPE_TIMESTAMP", SQL_TYPE_TIMESTAMP, CONST_PERSISTENT);
#endif
#if (defined(ODBCVER) && (ODBCVER >= 0x0300))
	REGISTER_LONG_CONSTANT("SQL_WCHAR", SQL_WCHAR, CONST_PERSISTENT);
#endif
#if (defined(ODBCVER) && (ODBCVER >= 0x0300))
	REGISTER_LONG_CONSTANT("SQL_WVARCHAR", SQL_WVARCHAR, CONST_PERSISTENT);
#endif
#if (defined(ODBCVER) && (ODBCVER >= 0x0300))
	REGISTER_LONG_CONSTANT("SQL_WLONGVARCHAR", SQL_WLONGVARCHAR, CONST_PERSISTENT);
#endif
#if (defined(ODBCVER) && (ODBCVER >= 0x0300))
	REGISTER_LONG_CONSTANT("SQL_BEST_ROWID", SQL_BEST_ROWID, CONST_PERSISTENT);
#endif
#if (defined(ODBCVER) && (ODBCVER >= 0x0300))
	REGISTER_LONG_CONSTANT("SQL_ROWVER", SQL_ROWVER, CONST_PERSISTENT);
#endif
#if (defined(ODBCVER) && (ODBCVER >= 0x0300))
	REGISTER_LONG_CONSTANT("SQL_SCOPE_CURROW", SQL_SCOPE_CURROW, CONST_PERSISTENT);
#endif
#if (defined(ODBCVER) && (ODBCVER >= 0x0300))
	REGISTER_LONG_CONSTANT("SQL_SCOPE_TRANSACTION", SQL_SCOPE_TRANSACTION, CONST_PERSISTENT);
#endif
#if (defined(ODBCVER) && (ODBCVER >= 0x0300))
	REGISTER_LONG_CONSTANT("SQL_SCOPE_SESSION", SQL_SCOPE_SESSION, CONST_PERSISTENT);
#endif
#if (defined(ODBCVER) && (ODBCVER >= 0x0300))
	REGISTER_LONG_CONSTANT("SQL_NO_NULLS", SQL_NO_NULLS, CONST_PERSISTENT);
#endif
#if (defined(ODBCVER) && (ODBCVER >= 0x0300))
	REGISTER_LONG_CONSTANT("SQL_NULLABLE", SQL_NULLABLE, CONST_PERSISTENT);
#endif
#if (defined(ODBCVER) && (ODBCVER >= 0x0300))
	REGISTER_LONG_CONSTANT("SQL_INDEX_UNIQUE", SQL_INDEX_UNIQUE, CONST_PERSISTENT);
#endif
#if (defined(ODBCVER) && (ODBCVER >= 0x0300))
	REGISTER_LONG_CONSTANT("SQL_INDEX_ALL", SQL_INDEX_ALL, CONST_PERSISTENT);
#endif
#if (defined(ODBCVER) && (ODBCVER >= 0x0300))
	REGISTER_LONG_CONSTANT("SQL_ENSURE", SQL_ENSURE, CONST_PERSISTENT);
#endif
#if (defined(ODBCVER) && (ODBCVER >= 0x0300))
	REGISTER_LONG_CONSTANT("SQL_QUICK", SQL_QUICK, CONST_PERSISTENT);
#endif


	crex_add_parameter_attribute(crex_hash_str_find_ptr(CG(function_table), "odbc_connect", sizeof("odbc_connect") - 1), 2, ZSTR_KNOWN(CREX_STR_SENSITIVEPARAMETER), 0);

	crex_add_parameter_attribute(crex_hash_str_find_ptr(CG(function_table), "odbc_pconnect", sizeof("odbc_pconnect") - 1), 2, ZSTR_KNOWN(CREX_STR_SENSITIVEPARAMETER), 0);
}
