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
  | Author: Wez Furlong <wez@crx.net>                                    |
  |         Marcus Boerger <helly@crx.net>                               |
  |         Sterling Hughes <sterling@crx.net>                           |
  +----------------------------------------------------------------------+
*/

/* The PDO Statement Handle Class */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "crx_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/crx_var.h"
#include "crx_pdo.h"
#include "crx_pdo_driver.h"
#include "crx_pdo_int.h"
#include "crex_exceptions.h"
#include "crex_interfaces.h"
#include "crx_memory_streams.h"
#include "pdo_stmt_arginfo.h"

#define CRX_STMT_GET_OBJ \
	pdo_stmt_t *stmt = C_PDO_STMT_P(CREX_THIS); \
	if (!stmt->dbh) { \
		crex_throw_error(NULL, "PDO object is uninitialized"); \
		RETURN_THROWS(); \
	} \

static inline bool rewrite_name_to_position(pdo_stmt_t *stmt, struct pdo_bound_param_data *param) /* {{{ */
{
	if (stmt->bound_param_map) {
		/* rewriting :name to ? style.
		 * We need to fixup the parameter numbers on the parameters.
		 * If we find that a given named parameter has been used twice,
		 * we will raise an error, as we can't be sure that it is safe
		 * to bind multiple parameters onto the same zval in the underlying
		 * driver */
		crex_string *name;
		int position = 0;

		if (stmt->named_rewrite_template) {
			/* this is not an error here */
			return 1;
		}
		if (!param->name) {
			/* do the reverse; map the parameter number to the name */
			if ((name = crex_hash_index_find_ptr(stmt->bound_param_map, param->paramno)) != NULL) {
				param->name = crex_string_copy(name);
				return 1;
			}
			/* TODO Error? */
			pdo_raise_impl_error(stmt->dbh, stmt, "HY093", "parameter was not defined");
			return 0;
		}

		CREX_HASH_FOREACH_PTR(stmt->bound_param_map, name) {
			if (!crex_string_equals(name, param->name)) {
				position++;
				continue;
			}
			if (param->paramno >= 0) {
				/* TODO Error? */
				pdo_raise_impl_error(stmt->dbh, stmt, "IM001", "PDO refuses to handle repeating the same :named parameter for multiple positions with this driver, as it might be unsafe to do so.  Consider using a separate name for each parameter instead");
				return -1;
			}
			param->paramno = position;
			return 1;
		} CREX_HASH_FOREACH_END();
		/* TODO Error? */
		pdo_raise_impl_error(stmt->dbh, stmt, "HY093", "parameter was not defined");
		return 0;
	}
	return 1;
}
/* }}} */

/* trigger callback hook for parameters */
static bool dispatch_param_event(pdo_stmt_t *stmt, enum pdo_param_event event_type) /* {{{ */
{
	bool ret = 1, is_param = 1;
	struct pdo_bound_param_data *param;
	HashTable *ht;

	if (stmt->dbh->skip_param_evt & (1 << event_type)) {
		return 1;
	}

	if (!stmt->methods->param_hook) {
		return 1;
	}

	ht = stmt->bound_params;

iterate:
	if (ht) {
		CREX_HASH_FOREACH_PTR(ht, param) {
			if (!stmt->methods->param_hook(stmt, param, event_type)) {
				ret = 0;
				break;
			}
		} CREX_HASH_FOREACH_END();
	}
	if (ret && is_param) {
		ht = stmt->bound_columns;
		is_param = 0;
		goto iterate;
	}

	return ret;
}
/* }}} */

bool pdo_stmt_describe_columns(pdo_stmt_t *stmt) /* {{{ */
{
	int col;

	stmt->columns = ecalloc(stmt->column_count, sizeof(struct pdo_column_data));

	for (col = 0; col < stmt->column_count; col++) {
		if (!stmt->methods->describer(stmt, col)) {
			return false;
		}

		/* if we are applying case conversions on column names, do so now */
		if (stmt->dbh->native_case != stmt->dbh->desired_case && stmt->dbh->desired_case != PDO_CASE_NATURAL) {
			crex_string *orig_name = stmt->columns[col].name;
			switch (stmt->dbh->desired_case) {
				case PDO_CASE_LOWER:
					stmt->columns[col].name = crex_string_tolower(orig_name);
					crex_string_release(orig_name);
					break;
				case PDO_CASE_UPPER: {
					stmt->columns[col].name = crex_string_separate(orig_name, 0);
					char *s = ZSTR_VAL(stmt->columns[col].name);
					while (*s != '\0') {
						*s = toupper(*s);
						s++;
					}
					break;
				}
				EMPTY_SWITCH_DEFAULT_CASE()
			}
		}

		/* update the column index on named bound parameters */
		if (stmt->bound_columns) {
			struct pdo_bound_param_data *param;

			if ((param = crex_hash_find_ptr(stmt->bound_columns,
					stmt->columns[col].name)) != NULL) {
				param->paramno = col;
			}
		}

	}
	return true;
}
/* }}} */

static void pdo_stmt_reset_columns(pdo_stmt_t *stmt) {
	if (stmt->columns) {
		int i;
		struct pdo_column_data *cols = stmt->columns;

		for (i = 0; i < stmt->column_count; i++) {
			if (cols[i].name) {
				crex_string_release_ex(cols[i].name, 0);
			}
		}
		efree(stmt->columns);
	}
	stmt->columns = NULL;
	stmt->column_count = 0;
}

/**
 * Change the column count on the statement. If it differs from the previous one,
 * discard existing columns information.
 */
PDO_API void crx_pdo_stmt_set_column_count(pdo_stmt_t *stmt, int new_count)
{
	/* Columns not yet "described". */
	if (!stmt->columns) {
		stmt->column_count = new_count;
		return;
	}

	/* The column count has not changed: No need to reload columns description.
	 * Note: Do not handle attribute name change, without column count change. */
	if (new_count == stmt->column_count) {
		return;
	}

	/* Free previous columns to force reload description. */
	pdo_stmt_reset_columns(stmt);
	stmt->column_count = new_count;
}

static void get_lazy_object(pdo_stmt_t *stmt, zval *return_value) /* {{{ */
{
	if (C_ISUNDEF(stmt->lazy_object_ref)) {
		pdo_row_t *row = ecalloc(1, sizeof(pdo_row_t));
		row->stmt = stmt;
		crex_object_std_init(&row->std, pdo_row_ce);
		ZVAL_OBJ(&stmt->lazy_object_ref, &row->std);
		GC_ADDREF(&stmt->std);
		GC_DELREF(&row->std);
	}
	ZVAL_COPY(return_value, &stmt->lazy_object_ref);
}
/* }}} */

static void param_dtor(zval *el) /* {{{ */
{
	struct pdo_bound_param_data *param = (struct pdo_bound_param_data *)C_PTR_P(el);

	/* tell the driver that it is going away */
	if (param->stmt->methods->param_hook) {
		param->stmt->methods->param_hook(param->stmt, param, PDO_PARAM_EVT_FREE);
	}

	if (param->name) {
		crex_string_release_ex(param->name, 0);
	}

	if (!C_ISUNDEF(param->parameter)) {
		zval_ptr_dtor(&param->parameter);
		ZVAL_UNDEF(&param->parameter);
	}
	if (!C_ISUNDEF(param->driver_params)) {
		zval_ptr_dtor(&param->driver_params);
	}
	efree(param);
}
/* }}} */

static bool really_register_bound_param(struct pdo_bound_param_data *param, pdo_stmt_t *stmt, bool is_param) /* {{{ */
{
	HashTable *hash;
	zval *parameter;
	struct pdo_bound_param_data *pparam = NULL;

	hash = is_param ? stmt->bound_params : stmt->bound_columns;

	if (!hash) {
		ALLOC_HASHTABLE(hash);
		crex_hash_init(hash, 13, NULL, param_dtor, 0);

		if (is_param) {
			stmt->bound_params = hash;
		} else {
			stmt->bound_columns = hash;
		}
	}

	if (!C_ISREF(param->parameter)) {
		parameter = &param->parameter;
	} else {
		parameter = C_REFVAL(param->parameter);
	}

	if (PDO_PARAM_TYPE(param->param_type) == PDO_PARAM_STR && param->max_value_len <= 0 && !C_ISNULL_P(parameter)) {
		if (!try_convert_to_string(parameter)) {
			return 0;
		}
	} else if (PDO_PARAM_TYPE(param->param_type) == PDO_PARAM_INT && (C_TYPE_P(parameter) == IS_FALSE || C_TYPE_P(parameter) == IS_TRUE)) {
		convert_to_long(parameter);
	} else if (PDO_PARAM_TYPE(param->param_type) == PDO_PARAM_BOOL && C_TYPE_P(parameter) == IS_LONG) {
		convert_to_boolean(parameter);
	}

	param->stmt = stmt;
	param->is_param = is_param;

	if (C_REFCOUNTED(param->driver_params)) {
		C_ADDREF(param->driver_params);
	}

	if (!is_param && param->name && stmt->columns) {
		/* try to map the name to the column */
		int i;

		for (i = 0; i < stmt->column_count; i++) {
			if (crex_string_equals(stmt->columns[i].name, param->name)) {
				param->paramno = i;
				break;
			}
		}

		/* if you prepare and then execute passing an array of params keyed by names,
		 * then this will trigger, and we don't want that */
		if (param->paramno == -1) {
			/* Should this always be an Error? */
			char *tmp;
			/* TODO Error? */
			spprintf(&tmp, 0, "Did not find column name '%s' in the defined columns; it will not be bound", ZSTR_VAL(param->name));
			pdo_raise_impl_error(stmt->dbh, stmt, "HY000", tmp);
			efree(tmp);
		}
	}

	if (param->name) {
		if (is_param && ZSTR_VAL(param->name)[0] != ':') {
			crex_string *temp = crex_string_alloc(ZSTR_LEN(param->name) + 1, 0);
			ZSTR_VAL(temp)[0] = ':';
			memmove(ZSTR_VAL(temp) + 1, ZSTR_VAL(param->name), ZSTR_LEN(param->name) + 1);
			param->name = temp;
		} else {
			param->name = crex_string_init(ZSTR_VAL(param->name), ZSTR_LEN(param->name), 0);
		}
	}

	if (is_param && !rewrite_name_to_position(stmt, param)) {
		if (param->name) {
			crex_string_release_ex(param->name, 0);
			param->name = NULL;
		}
		return 0;
	}

	/* ask the driver to perform any normalization it needs on the
	 * parameter name.  Note that it is illegal for the driver to take
	 * a reference to param, as it resides in transient storage only
	 * at this time. */
	if (stmt->methods->param_hook) {
		if (!stmt->methods->param_hook(stmt, param, PDO_PARAM_EVT_NORMALIZE)) {
			PDO_HANDLE_STMT_ERR();
			if (param->name) {
				crex_string_release_ex(param->name, 0);
				param->name = NULL;
			}
			return 0;
		}
	}

	/* delete any other parameter registered with this number.
	 * If the parameter is named, it will be removed and correctly
	 * disposed of by the hash_update call that follows */
	if (param->paramno >= 0) {
		crex_hash_index_del(hash, param->paramno);
	}

	/* allocate storage for the parameter, keyed by its "canonical" name */
	if (param->name) {
		pparam = crex_hash_update_mem(hash, param->name, param, sizeof(struct pdo_bound_param_data));
	} else {
		pparam = crex_hash_index_update_mem(hash, param->paramno, param, sizeof(struct pdo_bound_param_data));
	}

	/* tell the driver we just created a parameter */
	if (stmt->methods->param_hook) {
		if (!stmt->methods->param_hook(stmt, pparam, PDO_PARAM_EVT_ALLOC)) {
			PDO_HANDLE_STMT_ERR();
			/* undo storage allocation; the hash will free the parameter
			 * name if required */
			if (pparam->name) {
				crex_hash_del(hash, pparam->name);
			} else {
				crex_hash_index_del(hash, pparam->paramno);
			}
			/* param->parameter is freed by hash dtor */
			ZVAL_UNDEF(&param->parameter);
			return 0;
		}
	}
	return 1;
}
/* }}} */

/* {{{ Execute a prepared statement, optionally binding parameters */
CRX_METHOD(PDOStatement, execute)
{
	zval *input_params = NULL;
	int ret = 1;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_OR_NULL(input_params)
	CREX_PARSE_PARAMETERS_END();

	CRX_STMT_GET_OBJ;
	PDO_STMT_CLEAR_ERR();

	if (input_params) {
		struct pdo_bound_param_data param;
		zval *tmp;
		crex_string *key = NULL;
		crex_ulong num_index;

		if (stmt->bound_params) {
			crex_hash_destroy(stmt->bound_params);
			FREE_HASHTABLE(stmt->bound_params);
			stmt->bound_params = NULL;
		}

		CREX_HASH_FOREACH_KEY_VAL(C_ARRVAL_P(input_params), num_index, key, tmp) {
			memset(&param, 0, sizeof(param));

			if (key) {
				/* yes this is correct.  we don't want to count the null byte.  ask wez */
				param.name = key;
				param.paramno = -1;
			} else {
				/* we're okay to be zero based here */
				/* num_index is unsignend */
				param.paramno = num_index;
			}

			param.param_type = PDO_PARAM_STR;
			ZVAL_COPY(&param.parameter, tmp);

			if (!really_register_bound_param(&param, stmt, 1)) {
				if (!C_ISUNDEF(param.parameter)) {
					zval_ptr_dtor(&param.parameter);
				}
				RETURN_FALSE;
			}
		} CREX_HASH_FOREACH_END();
	}

	if (PDO_PLACEHOLDER_NONE == stmt->supports_placeholders) {
		/* handle the emulated parameter binding,
		 * stmt->active_query_string holds the query with binds expanded and
		 * quoted.
		 */

		/* string is leftover from previous calls so PDOStatement::debugDumpParams() can access */
		if (stmt->active_query_string) {
			crex_string_release(stmt->active_query_string);
			stmt->active_query_string = NULL;
		}

		ret = pdo_parse_params(stmt, stmt->query_string, &stmt->active_query_string);

		if (ret == 0) {
			/* no changes were made */
			stmt->active_query_string = crex_string_copy(stmt->query_string);
			ret = 1;
		} else if (ret == -1) {
			/* something broke */
			RETURN_FALSE;
		}
	} else if (!dispatch_param_event(stmt, PDO_PARAM_EVT_EXEC_PRE)) {
		PDO_HANDLE_STMT_ERR();
		RETURN_FALSE;
	}
	if (stmt->methods->executer(stmt)) {
		if (!stmt->executed) {
			/* this is the first execute */

			if (stmt->dbh->alloc_own_columns && !stmt->columns) {
				/* for "big boy" drivers, we need to allocate memory to fetch
				 * the results into, so lets do that now */
				ret = pdo_stmt_describe_columns(stmt);
			}

			stmt->executed = 1;
		}

		if (ret && !dispatch_param_event(stmt, PDO_PARAM_EVT_EXEC_POST)) {
			PDO_HANDLE_STMT_ERR();
			RETURN_FALSE;
		}

		RETURN_BOOL(ret);
	}
	PDO_HANDLE_STMT_ERR();
	RETURN_FALSE;
}
/* }}} */

static inline void fetch_value(pdo_stmt_t *stmt, zval *dest, int colno, enum pdo_param_type *type_override) /* {{{ */
{
	if (colno < 0) {
		crex_value_error("Column index must be greater than or equal to 0");
		ZVAL_NULL(dest);
		return;
	}

	if (colno >= stmt->column_count) {
		crex_value_error("Invalid column index");
		ZVAL_NULL(dest);
		return;
	}

	ZVAL_NULL(dest);
	stmt->methods->get_col(stmt, colno, dest, type_override);

	if (C_TYPE_P(dest) == IS_STRING && C_STRLEN_P(dest) == 0
			&& stmt->dbh->oracle_nulls == PDO_NULL_EMPTY_STRING) {
		zval_ptr_dtor_str(dest);
		ZVAL_NULL(dest);
	}

	/* If stringification is requested, override with PDO_PARAM_STR. */
	enum pdo_param_type pdo_param_str = PDO_PARAM_STR;
	if (stmt->dbh->stringify) {
		type_override = &pdo_param_str;
	}

	if (type_override && C_TYPE_P(dest) != IS_NULL) {
		switch (*type_override) {
			case PDO_PARAM_INT:
				convert_to_long(dest);
				break;
			case PDO_PARAM_BOOL:
				convert_to_boolean(dest);
				break;
			case PDO_PARAM_STR:
				if (C_TYPE_P(dest) == IS_FALSE) {
					/* Return "0" rather than "", because this is what database drivers that
					 * don't have a dedicated boolean type would return. */
					zval_ptr_dtor_nogc(dest);
					ZVAL_INTERNED_STR(dest, ZSTR_CHAR('0'));
				} else if (C_TYPE_P(dest) == IS_RESOURCE) {
					/* Convert LOB stream to string */
					crx_stream *stream;
					crx_stream_from_zval_no_verify(stream, dest);
					crex_string *str = crx_stream_copy_to_mem(stream, CRX_STREAM_COPY_ALL, 0);
					zval_ptr_dtor_nogc(dest);
					if (str == NULL) {
						ZVAL_EMPTY_STRING(dest);
					} else {
						ZVAL_STR(dest, str);
					}
				} else {
					convert_to_string(dest);
				}
				break;
			case PDO_PARAM_NULL:
				convert_to_null(dest);
				break;
			case PDO_PARAM_LOB:
				if (C_TYPE_P(dest) == IS_STRING) {
					crx_stream *stream =
						crx_stream_memory_open(TEMP_STREAM_READONLY, C_STR_P(dest));
					zval_ptr_dtor_str(dest);
					crx_stream_to_zval(stream, dest);
				}
				break;
			default:
				break;
		}
	}

	if (C_TYPE_P(dest) == IS_NULL && stmt->dbh->oracle_nulls == PDO_NULL_TO_STRING) {
		ZVAL_EMPTY_STRING(dest);
	}
}
/* }}} */

static bool do_fetch_common(pdo_stmt_t *stmt, enum pdo_fetch_orientation ori, crex_long offset) /* {{{ */
{
	if (!stmt->executed) {
		return 0;
	}

	if (!dispatch_param_event(stmt, PDO_PARAM_EVT_FETCH_PRE)) {
		return 0;
	}

	if (!stmt->methods->fetcher(stmt, ori, offset)) {
		return 0;
	}

	/* some drivers might need to describe the columns now */
	if (!stmt->columns && !pdo_stmt_describe_columns(stmt)) {
		return 0;
	}

	if (!dispatch_param_event(stmt, PDO_PARAM_EVT_FETCH_POST)) {
		return 0;
	}

	if (stmt->bound_columns) {
		/* update those bound column variables now */
		struct pdo_bound_param_data *param;

		CREX_HASH_FOREACH_PTR(stmt->bound_columns, param) {
			if (param->paramno >= 0) {
				if (!C_ISREF(param->parameter)) {
					continue;
				}

				/* delete old value */
				zval_ptr_dtor(C_REFVAL(param->parameter));

				/* set new value */
				fetch_value(stmt, C_REFVAL(param->parameter), param->paramno, &param->param_type);

				/* TODO: some smart thing that avoids duplicating the value in the
				 * general loop below.  For now, if you're binding output columns,
				 * it's better to use LAZY or BOUND fetches if you want to shave
				 * off those cycles */
			}
		} CREX_HASH_FOREACH_END();
	}

	return 1;
}
/* }}} */

static bool do_fetch_class_prepare(pdo_stmt_t *stmt) /* {{{ */
{
	crex_class_entry *ce = stmt->fetch.cls.ce;
	crex_fcall_info *fci = &stmt->fetch.cls.fci;
	crex_fcall_info_cache *fcc = &stmt->fetch.cls.fcc;

	fci->size = sizeof(crex_fcall_info);

	if (!ce) {
		stmt->fetch.cls.ce = CREX_STANDARD_CLASS_DEF_PTR;
		ce = CREX_STANDARD_CLASS_DEF_PTR;
	}

	if (ce->constructor) {
		ZVAL_UNDEF(&fci->function_name);
		fci->retval = &stmt->fetch.cls.retval;
		fci->param_count = 0;
		fci->params = NULL;

		crex_fcall_info_args_ex(fci, ce->constructor, &stmt->fetch.cls.ctor_args);

		fcc->function_handler = ce->constructor;
		fcc->called_scope = ce;
		return 1;
	} else if (!C_ISUNDEF(stmt->fetch.cls.ctor_args)) {
		crex_throw_error(NULL, "User-supplied statement does not accept constructor arguments");
		return 0;
	} else {
		return 1; /* no ctor no args is also ok */
	}
}
/* }}} */

static bool make_callable_ex(pdo_stmt_t *stmt, zval *callable, crex_fcall_info * fci, crex_fcall_info_cache * fcc, int num_args) /* {{{ */
{
	char *is_callable_error = NULL;

	if (crex_fcall_info_init(callable, 0, fci, fcc, NULL, &is_callable_error) == FAILURE) {
		if (is_callable_error) {
			crex_type_error("%s", is_callable_error);
			efree(is_callable_error);
		} else {
			crex_type_error("User-supplied function must be a valid callback");
		}
		return false;
	}
	if (is_callable_error) {
		/* Possible error message */
		efree(is_callable_error);
	}

	fci->param_count = num_args; /* probably less */
	fci->params = safe_emalloc(sizeof(zval), num_args, 0);

	return true;
}
/* }}} */

static bool do_fetch_func_prepare(pdo_stmt_t *stmt) /* {{{ */
{
	crex_fcall_info *fci = &stmt->fetch.cls.fci;
	crex_fcall_info_cache *fcc = &stmt->fetch.cls.fcc;

	if (!make_callable_ex(stmt, &stmt->fetch.func.function, fci, fcc, stmt->column_count)) {
		return false;
	} else {
		stmt->fetch.func.values = safe_emalloc(sizeof(zval), stmt->column_count, 0);
		return true;
	}
}
/* }}} */

static void do_fetch_opt_finish(pdo_stmt_t *stmt, int free_ctor_agrs) /* {{{ */
{
	/* fci.size is used to check if it is valid */
	if (stmt->fetch.cls.fci.size && stmt->fetch.cls.fci.params) {
		if (!C_ISUNDEF(stmt->fetch.cls.ctor_args)) {
			/* Added to free constructor arguments */
			crex_fcall_info_args_clear(&stmt->fetch.cls.fci, 1);
		} else {
			efree(stmt->fetch.cls.fci.params);
		}
		stmt->fetch.cls.fci.params = NULL;
	}

	stmt->fetch.cls.fci.size = 0;
	if (!C_ISUNDEF(stmt->fetch.cls.ctor_args) && free_ctor_agrs) {
		zval_ptr_dtor(&stmt->fetch.cls.ctor_args);
		ZVAL_UNDEF(&stmt->fetch.cls.ctor_args);
		stmt->fetch.cls.fci.param_count = 0;
	}
	if (stmt->fetch.func.values) {
		efree(stmt->fetch.func.values);
		stmt->fetch.func.values = NULL;
	}
}
/* }}} */

/* perform a fetch.
 * If return_value is not null, store values into it according to HOW. */
static bool do_fetch(pdo_stmt_t *stmt, zval *return_value, enum pdo_fetch_type how, enum pdo_fetch_orientation ori, crex_long offset, zval *return_all) /* {{{ */
{
	int flags, idx, old_arg_count = 0;
	crex_class_entry *ce = NULL, *old_ce = NULL;
	zval grp_val, *pgrp, retval, old_ctor_args = {{0}, {0}, {0}};
	int colno;
	int i = 0;

	if (how == PDO_FETCH_USE_DEFAULT) {
		how = stmt->default_fetch_type;
	}
	flags = how & PDO_FETCH_FLAGS;
	how = how & ~PDO_FETCH_FLAGS;

	if (!do_fetch_common(stmt, ori, offset)) {
		return 0;
	}

	if (how == PDO_FETCH_BOUND) {
		RETVAL_TRUE;
		return 1;
	}

	if ((flags & PDO_FETCH_GROUP) && stmt->fetch.column == -1) {
		colno = 1;
	} else {
		colno = stmt->fetch.column;
	}

	/* If no return value we are done */
	if (!return_value) {
		return true;
	}

	if (how == PDO_FETCH_LAZY) {
		get_lazy_object(stmt, return_value);
		return 1;
	}

	RETVAL_FALSE;

	switch (how) {
		case PDO_FETCH_USE_DEFAULT:
		case PDO_FETCH_ASSOC:
		case PDO_FETCH_BOTH:
		case PDO_FETCH_NUM:
		case PDO_FETCH_NAMED:
			if (!return_all) {
				array_init_size(return_value, stmt->column_count);
			} else {
				array_init(return_value);
			}
			break;

		case PDO_FETCH_KEY_PAIR:
			if (stmt->column_count != 2) {
				/* TODO: Error? */
				pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "PDO::FETCH_KEY_PAIR fetch mode requires the result set to contain exactly 2 columns.");
				return 0;
			}
			if (!return_all) {
				array_init(return_value);
			}
			break;

		case PDO_FETCH_COLUMN:
			if (colno < 0 ) {
				crex_value_error("Column index must be greater than or equal to 0");
				return false;
			}

			if (colno >= stmt->column_count) {
				crex_value_error("Invalid column index");
				return false;
			}

			if (flags == PDO_FETCH_GROUP && stmt->fetch.column == -1) {
				fetch_value(stmt, return_value, 1, NULL);
			} else if (flags == PDO_FETCH_GROUP && colno) {
				fetch_value(stmt, return_value, 0, NULL);
			} else {
				fetch_value(stmt, return_value, colno, NULL);
			}
			if (!return_all) {
				return 1;
			}
			break;

		case PDO_FETCH_OBJ:
			object_init_ex(return_value, CREX_STANDARD_CLASS_DEF_PTR);
			break;

		case PDO_FETCH_CLASS:
			if (flags & PDO_FETCH_CLASSTYPE) {
				zval val;
				crex_class_entry *cep;

				old_ce = stmt->fetch.cls.ce;
				ZVAL_COPY_VALUE(&old_ctor_args, &stmt->fetch.cls.ctor_args);
				old_arg_count = stmt->fetch.cls.fci.param_count;
				do_fetch_opt_finish(stmt, 0);

				fetch_value(stmt, &val, i++, NULL);
				if (C_TYPE(val) != IS_NULL) {
					if (!try_convert_to_string(&val)) {
						return 0;
					}
					if ((cep = crex_lookup_class(C_STR(val))) == NULL) {
						stmt->fetch.cls.ce = CREX_STANDARD_CLASS_DEF_PTR;
					} else {
						stmt->fetch.cls.ce = cep;
					}
				}

				do_fetch_class_prepare(stmt);
				zval_ptr_dtor_str(&val);
			}
			ce = stmt->fetch.cls.ce;
			/* TODO: Make this an assertion and ensure this is true higher up? */
			if (!ce) {
				/* TODO Error? */
				pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "No fetch class specified");
				return 0;
			}
			if ((flags & PDO_FETCH_SERIALIZE) == 0) {
				if (UNEXPECTED(object_init_ex(return_value, ce) != SUCCESS)) {
					return 0;
				}
				if (!stmt->fetch.cls.fci.size) {
					if (!do_fetch_class_prepare(stmt)) {
						zval_ptr_dtor(return_value);
						return 0;
					}
				}
				if (ce->constructor && (flags & PDO_FETCH_PROPS_LATE)) {
					stmt->fetch.cls.fci.object = C_OBJ_P(return_value);
					stmt->fetch.cls.fcc.object = C_OBJ_P(return_value);
					if (crex_call_function(&stmt->fetch.cls.fci, &stmt->fetch.cls.fcc) == FAILURE) {
						/* TODO Error? */
						pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "could not call class constructor");
						return 0;
					} else {
						if (!C_ISUNDEF(stmt->fetch.cls.retval)) {
							zval_ptr_dtor(&stmt->fetch.cls.retval);
							ZVAL_UNDEF(&stmt->fetch.cls.retval);
						}
					}
				}
			}
			break;

		case PDO_FETCH_INTO:
			/* TODO: Make this an assertion and ensure this is true higher up? */
			if (C_ISUNDEF(stmt->fetch.into)) {
				/* TODO ArgumentCountError? */
				pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "No fetch-into object specified.");
				return 0;
				break;
			}

			ZVAL_COPY(return_value, &stmt->fetch.into);

			if (C_OBJ_P(return_value)->ce == CREX_STANDARD_CLASS_DEF_PTR) {
				how = PDO_FETCH_OBJ;
			}
			break;

		case PDO_FETCH_FUNC:
			/* TODO: Make this an assertion and ensure this is true higher up? */
			if (C_ISUNDEF(stmt->fetch.func.function)) {
				/* TODO ArgumentCountError? */
				pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "No fetch function specified");
				return 0;
			}
			if (!stmt->fetch.func.fci.size) {
				if (!do_fetch_func_prepare(stmt))
				{
					return 0;
				}
			}
			break;
		EMPTY_SWITCH_DEFAULT_CASE();
	}

	if (return_all && how != PDO_FETCH_KEY_PAIR) {
		if (flags == PDO_FETCH_GROUP && how == PDO_FETCH_COLUMN && stmt->fetch.column > 0) {
			fetch_value(stmt, &grp_val, colno, NULL);
		} else {
			fetch_value(stmt, &grp_val, i, NULL);
		}
		convert_to_string(&grp_val);
		if (how == PDO_FETCH_COLUMN) {
			i = stmt->column_count; /* no more data to fetch */
		} else {
			i++;
		}
	}

	for (idx = 0; i < stmt->column_count; i++, idx++) {
		zval val;
		fetch_value(stmt, &val, i, NULL);

		switch (how) {
			case PDO_FETCH_ASSOC:
				crex_symtable_update(C_ARRVAL_P(return_value), stmt->columns[i].name, &val);
				break;

			case PDO_FETCH_KEY_PAIR:
				{
					zval tmp;
					fetch_value(stmt, &tmp, ++i, NULL);

					if (C_TYPE(val) == IS_LONG) {
						crex_hash_index_update((return_all ? C_ARRVAL_P(return_all) : C_ARRVAL_P(return_value)), C_LVAL(val), &tmp);
					} else {
						convert_to_string(&val);
						crex_symtable_update((return_all ? C_ARRVAL_P(return_all) : C_ARRVAL_P(return_value)), C_STR(val), &tmp);
					}
					zval_ptr_dtor(&val);
					return 1;
				}
				break;

			case PDO_FETCH_USE_DEFAULT:
			case PDO_FETCH_BOTH:
				crex_symtable_update(C_ARRVAL_P(return_value), stmt->columns[i].name, &val);
				if (crex_hash_index_add(C_ARRVAL_P(return_value), i, &val) != NULL) {
					C_TRY_ADDREF(val);
				}
				break;

			case PDO_FETCH_NAMED:
				/* already have an item with this name? */
				{
					zval *curr_val;
					if ((curr_val = crex_hash_find(C_ARRVAL_P(return_value), stmt->columns[i].name))) {
						zval arr;
						if (C_TYPE_P(curr_val) != IS_ARRAY) {
							/* a little bit of black magic here:
							 * we're creating a new array and swapping it for the
							 * zval that's already stored in the hash under the name
							 * we want.  We then add that zval to the array.
							 * This is effectively the same thing as:
							 * if (!is_array($hash[$name])) {
							 *   $hash[$name] = array($hash[$name]);
							 * }
							 * */
							zval cur;

							array_init(&arr);

							ZVAL_COPY_VALUE(&cur, curr_val);
							ZVAL_COPY_VALUE(curr_val, &arr);

							crex_hash_next_index_insert_new(C_ARRVAL(arr), &cur);
						} else {
							ZVAL_COPY_VALUE(&arr, curr_val);
						}
						crex_hash_next_index_insert_new(C_ARRVAL(arr), &val);
					} else {
						crex_hash_update(C_ARRVAL_P(return_value), stmt->columns[i].name, &val);
					}
				}
				break;

			case PDO_FETCH_NUM:
				crex_hash_next_index_insert_new(C_ARRVAL_P(return_value), &val);
				break;

			case PDO_FETCH_OBJ:
			case PDO_FETCH_INTO:
				crex_update_property_ex(NULL, C_OBJ_P(return_value),
					stmt->columns[i].name,
					&val);
				zval_ptr_dtor(&val);
				break;

			case PDO_FETCH_CLASS:
				if ((flags & PDO_FETCH_SERIALIZE) == 0 || idx) {
					crex_update_property_ex(ce, C_OBJ_P(return_value),
						stmt->columns[i].name,
						&val);
					zval_ptr_dtor(&val);
				} else {
					if (!ce->unserialize) {
						zval_ptr_dtor(&val);
						pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "cannot unserialize class");
						return 0;
					} else if (ce->unserialize(return_value, ce, (unsigned char *)(C_TYPE(val) == IS_STRING ? C_STRVAL(val) : ""), C_TYPE(val) == IS_STRING ? C_STRLEN(val) : 0, NULL) == FAILURE) {
						zval_ptr_dtor(&val);
						pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "cannot unserialize class");
						zval_ptr_dtor(return_value);
						ZVAL_NULL(return_value);
						return 0;
					} else {
						zval_ptr_dtor(&val);
					}
				}
				break;

			case PDO_FETCH_FUNC:
				ZVAL_COPY_VALUE(&stmt->fetch.func.values[idx], &val);
				ZVAL_COPY_VALUE(&stmt->fetch.cls.fci.params[idx], &stmt->fetch.func.values[idx]);
				break;

			default:
				zval_ptr_dtor(&val);
				crex_value_error("Fetch mode must be a bitmask of PDO::FETCH_* constants");
				return 0;
		}
	}

	switch (how) {
		case PDO_FETCH_CLASS:
			if (ce->constructor && !(flags & (PDO_FETCH_PROPS_LATE | PDO_FETCH_SERIALIZE))) {
				stmt->fetch.cls.fci.object = C_OBJ_P(return_value);
				stmt->fetch.cls.fcc.object = C_OBJ_P(return_value);
				if (crex_call_function(&stmt->fetch.cls.fci, &stmt->fetch.cls.fcc) == FAILURE) {
					/* TODO Error? */
					pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "could not call class constructor");
					return 0;
				} else {
					if (!C_ISUNDEF(stmt->fetch.cls.retval)) {
						zval_ptr_dtor(&stmt->fetch.cls.retval);
					}
				}
			}
			if (flags & PDO_FETCH_CLASSTYPE) {
				do_fetch_opt_finish(stmt, 0);
				stmt->fetch.cls.ce = old_ce;
				ZVAL_COPY_VALUE(&stmt->fetch.cls.ctor_args, &old_ctor_args);
				stmt->fetch.cls.fci.param_count = old_arg_count;
			}
			break;

		case PDO_FETCH_FUNC:
			stmt->fetch.func.fci.param_count = idx;
			stmt->fetch.func.fci.retval = &retval;
			if (crex_call_function(&stmt->fetch.func.fci, &stmt->fetch.func.fcc) == FAILURE) {
				/* TODO Error? */
				pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "could not call user-supplied function");
				return 0;
			} else {
				if (return_all) {
					zval_ptr_dtor(return_value); /* we don't need that */
					ZVAL_COPY_VALUE(return_value, &retval);
				} else if (!C_ISUNDEF(retval)) {
					ZVAL_COPY_VALUE(return_value, &retval);
				}
			}
			while (idx--) {
				zval_ptr_dtor(&stmt->fetch.func.values[idx]);
			}
			break;

		default:
			break;
	}

	if (return_all) {
		if ((flags & PDO_FETCH_UNIQUE) == PDO_FETCH_UNIQUE) {
			crex_symtable_update(C_ARRVAL_P(return_all), C_STR(grp_val), return_value);
		} else {
			zval grp;
			if ((pgrp = crex_symtable_find(C_ARRVAL_P(return_all), C_STR(grp_val))) == NULL) {
				array_init(&grp);
				crex_symtable_update(C_ARRVAL_P(return_all), C_STR(grp_val), &grp);
			} else {
				ZVAL_COPY_VALUE(&grp, pgrp);
			}
			crex_hash_next_index_insert(C_ARRVAL(grp), return_value);
		}
		zval_ptr_dtor_str(&grp_val);
	}

	return 1;
}
/* }}} */

static bool pdo_stmt_verify_mode(pdo_stmt_t *stmt, crex_long mode, uint32_t mode_arg_num, bool fetch_all) /* {{{ */
{
	int flags = mode & PDO_FETCH_FLAGS;

	mode = mode & ~PDO_FETCH_FLAGS;

	if (mode < 0 || mode > PDO_FETCH__MAX) {
		crex_argument_value_error(mode_arg_num, "must be a bitmask of PDO::FETCH_* constants");
		return 0;
	}

	if (mode == PDO_FETCH_USE_DEFAULT) {
		flags = stmt->default_fetch_type & PDO_FETCH_FLAGS;
		mode = stmt->default_fetch_type & ~PDO_FETCH_FLAGS;
	}

	switch(mode) {
		case PDO_FETCH_FUNC:
			if (!fetch_all) {
				crex_value_error("Can only use PDO::FETCH_FUNC in PDOStatement::fetchAll()");
				return 0;
			}
			return 1;

		case PDO_FETCH_LAZY:
			if (fetch_all) {
				crex_argument_value_error(mode_arg_num, "cannot be PDO::FETCH_LAZY in PDOStatement::fetchAll()");
				return 0;
			}
			CREX_FALLTHROUGH;
		default:
			if ((flags & PDO_FETCH_SERIALIZE) == PDO_FETCH_SERIALIZE) {
				crex_argument_value_error(mode_arg_num, "must use PDO::FETCH_SERIALIZE with PDO::FETCH_CLASS");
				return 0;
			}
			if ((flags & PDO_FETCH_CLASSTYPE) == PDO_FETCH_CLASSTYPE) {
				crex_argument_value_error(mode_arg_num, "must use PDO::FETCH_CLASSTYPE with PDO::FETCH_CLASS");
				return 0;
			}
			if (mode >= PDO_FETCH__MAX) {
				crex_argument_value_error(mode_arg_num, "must be a bitmask of PDO::FETCH_* constants");
				return 0;
			}
			CREX_FALLTHROUGH;

		case PDO_FETCH_CLASS:
			if (flags & PDO_FETCH_SERIALIZE) {
				crx_error_docref(NULL, E_DEPRECATED, "The PDO::FETCH_SERIALIZE mode is deprecated");
			}
			return 1;
	}
}
/* }}} */

/* {{{ Fetches the next row and returns it, or false if there are no more rows */
CRX_METHOD(PDOStatement, fetch)
{
	crex_long how = PDO_FETCH_USE_DEFAULT;
	crex_long ori = PDO_FETCH_ORI_NEXT;
	crex_long off = 0;

	CREX_PARSE_PARAMETERS_START(0, 3)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(how)
		C_PARAM_LONG(ori)
		C_PARAM_LONG(off)
	CREX_PARSE_PARAMETERS_END();

	CRX_STMT_GET_OBJ;
	PDO_STMT_CLEAR_ERR();

	if (!pdo_stmt_verify_mode(stmt, how, 1, false)) {
		RETURN_THROWS();
	}

	if (!do_fetch(stmt, return_value, how, ori, off, NULL)) {
		PDO_HANDLE_STMT_ERR();
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Fetches the next row and returns it as an object. */
CRX_METHOD(PDOStatement, fetchObject)
{
	crex_class_entry *ce = NULL;
	crex_class_entry *old_ce;
	zval old_ctor_args, *ctor_args = NULL;
	int old_arg_count;

	CREX_PARSE_PARAMETERS_START(0, 2)
		C_PARAM_OPTIONAL
		C_PARAM_CLASS_OR_NULL(ce)
		C_PARAM_ARRAY(ctor_args)
	CREX_PARSE_PARAMETERS_END();

	CRX_STMT_GET_OBJ;
	PDO_STMT_CLEAR_ERR();

	old_ce = stmt->fetch.cls.ce;
	ZVAL_COPY_VALUE(&old_ctor_args, &stmt->fetch.cls.ctor_args);
	old_arg_count = stmt->fetch.cls.fci.param_count;

	do_fetch_opt_finish(stmt, 0);

	if (ctor_args && crex_hash_num_elements(C_ARRVAL_P(ctor_args))) {
		ZVAL_ARR(&stmt->fetch.cls.ctor_args, crex_array_dup(C_ARRVAL_P(ctor_args)));
	} else {
		ZVAL_UNDEF(&stmt->fetch.cls.ctor_args);
	}
	if (ce) {
		stmt->fetch.cls.ce = ce;
	} else {
		stmt->fetch.cls.ce = crex_standard_class_def;
	}

	if (!do_fetch(stmt, return_value, PDO_FETCH_CLASS, PDO_FETCH_ORI_NEXT, /* offset */ 0, NULL)) {
		PDO_HANDLE_STMT_ERR();
		RETVAL_FALSE;
	}
	do_fetch_opt_finish(stmt, 1);

	stmt->fetch.cls.ce = old_ce;
	ZVAL_COPY_VALUE(&stmt->fetch.cls.ctor_args, &old_ctor_args);
	stmt->fetch.cls.fci.param_count = old_arg_count;
}
/* }}} */

/* {{{ Returns a data of the specified column in the result set. */
CRX_METHOD(PDOStatement, fetchColumn)
{
	crex_long col_n = 0;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(col_n)
	CREX_PARSE_PARAMETERS_END();

	CRX_STMT_GET_OBJ;
	PDO_STMT_CLEAR_ERR();

	if (!do_fetch_common(stmt, PDO_FETCH_ORI_NEXT, 0)) {
		PDO_HANDLE_STMT_ERR();
		RETURN_FALSE;
	}

	fetch_value(stmt, return_value, col_n, NULL);
}
/* }}} */

/* {{{ Returns an array of all of the results. */
CRX_METHOD(PDOStatement, fetchAll)
{
	crex_long how = PDO_FETCH_USE_DEFAULT;
	zval data, *return_all = NULL;
	zval *arg2 = NULL;
	crex_class_entry *old_ce;
	zval old_ctor_args, *ctor_args = NULL;
	bool error = false;
	int flags, old_arg_count;

	CREX_PARSE_PARAMETERS_START(0, 3)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(how)
		C_PARAM_ZVAL_OR_NULL(arg2)
		C_PARAM_ARRAY_OR_NULL(ctor_args)
	CREX_PARSE_PARAMETERS_END();

	CRX_STMT_GET_OBJ;
	if (!pdo_stmt_verify_mode(stmt, how, 1, true)) {
		RETURN_THROWS();
	}

	old_ce = stmt->fetch.cls.ce;
	ZVAL_COPY_VALUE(&old_ctor_args, &stmt->fetch.cls.ctor_args);
	old_arg_count = stmt->fetch.cls.fci.param_count;

	do_fetch_opt_finish(stmt, 0);

	/* TODO Would be good to reuse part of pdo_stmt_setup_fetch_mode() in some way */

	switch (how & ~PDO_FETCH_FLAGS) {
		case PDO_FETCH_CLASS:
			/* Figure out correct class */
			if (arg2) {
				if (C_TYPE_P(arg2) != IS_STRING) {
					crex_argument_type_error(2, "must be of type string, %s given", crex_zval_value_name(arg2));
					RETURN_THROWS();
				}
				stmt->fetch.cls.ce = crex_fetch_class(C_STR_P(arg2), CREX_FETCH_CLASS_AUTO);
				if (!stmt->fetch.cls.ce) {
					crex_argument_type_error(2, "must be a valid class");
					RETURN_THROWS();
				}
			} else {
				stmt->fetch.cls.ce = crex_standard_class_def;
			}

			if (ctor_args && crex_hash_num_elements(C_ARRVAL_P(ctor_args)) > 0) {
				ZVAL_COPY_VALUE(&stmt->fetch.cls.ctor_args, ctor_args); /* we're not going to free these */
			} else {
				ZVAL_UNDEF(&stmt->fetch.cls.ctor_args);
			}

			do_fetch_class_prepare(stmt);
			break;

		case PDO_FETCH_FUNC: /* Cannot be a default fetch mode */
			if (CREX_NUM_ARGS() != 2) {
				crex_string *func = get_active_function_or_method_name();
				crex_argument_count_error("%s() expects exactly 2 argument for PDO::FETCH_FUNC, %d given",
					ZSTR_VAL(func), CREX_NUM_ARGS());
				crex_string_release(func);
				RETURN_THROWS();
			}
			if (arg2 == NULL) {
				/* TODO use "must be of type callable" format? */
				crex_argument_type_error(2, "must be a callable, null given");
				RETURN_THROWS();
			}
			/* TODO Check it is a callable? */
			ZVAL_COPY_VALUE(&stmt->fetch.func.function, arg2);
			if (do_fetch_func_prepare(stmt) == false) {
				RETURN_THROWS();
			}
			break;

		case PDO_FETCH_COLUMN:
			if (CREX_NUM_ARGS() > 2) {
				crex_string *func = get_active_function_or_method_name();
				crex_argument_count_error("%s() expects at most 2 argument for the fetch mode provided, %d given",
					ZSTR_VAL(func), CREX_NUM_ARGS());
				crex_string_release(func);
				RETURN_THROWS();
			}
			/* Is column index passed? */
			if (arg2) {
				// Reuse convert_to_long(arg2); ?
				if (C_TYPE_P(arg2) != IS_LONG) {
					crex_argument_type_error(2, "must be of type int, %s given", crex_zval_value_name(arg2));
					RETURN_THROWS();
				}
				if (C_LVAL_P(arg2) < 0) {
					crex_argument_value_error(2, "must be greater than or equal to 0");
					RETURN_THROWS();
				}
				stmt->fetch.column = C_LVAL_P(arg2);
			} else {
				stmt->fetch.column = how & PDO_FETCH_GROUP ? -1 : 0;
			}
			break;

		default:
			/* No support for PDO_FETCH_INTO which takes 2 args??? */
			if (CREX_NUM_ARGS() > 1) {
				crex_string *func = get_active_function_or_method_name();
				crex_argument_count_error("%s() expects exactly 1 argument for the fetch mode provided, %d given",
				ZSTR_VAL(func), CREX_NUM_ARGS());
				crex_string_release(func);
				RETURN_THROWS();
			}
	}

	flags = how & PDO_FETCH_FLAGS;

	if ((how & ~PDO_FETCH_FLAGS) == PDO_FETCH_USE_DEFAULT) {
		flags |= stmt->default_fetch_type & PDO_FETCH_FLAGS;
		how |= stmt->default_fetch_type & ~PDO_FETCH_FLAGS;
	}

	PDO_STMT_CLEAR_ERR();
	if ((how & PDO_FETCH_GROUP) || how == PDO_FETCH_KEY_PAIR ||
		(how == PDO_FETCH_USE_DEFAULT && stmt->default_fetch_type == PDO_FETCH_KEY_PAIR)
	) {
		array_init(return_value);
		return_all = return_value;
	}
	if (!do_fetch(stmt, &data, how | flags, PDO_FETCH_ORI_NEXT, /* offset */ 0, return_all)) {
		error = true;
	}

	if (!error) {
		if ((how & PDO_FETCH_GROUP) || how == PDO_FETCH_KEY_PAIR ||
			(how == PDO_FETCH_USE_DEFAULT && stmt->default_fetch_type == PDO_FETCH_KEY_PAIR)
		) {
			while (do_fetch(stmt, &data, how | flags, PDO_FETCH_ORI_NEXT, /* offset */ 0, return_all));
		} else {
			array_init(return_value);
			do {
				crex_hash_next_index_insert_new(C_ARRVAL_P(return_value), &data);
			} while (do_fetch(stmt, &data, how | flags, PDO_FETCH_ORI_NEXT, /* offset */ 0, NULL));
		}
	}

	do_fetch_opt_finish(stmt, 0);

	/* Restore defaults which were changed by PDO_FETCH_CLASS mode */
	stmt->fetch.cls.ce = old_ce;
	ZVAL_COPY_VALUE(&stmt->fetch.cls.ctor_args, &old_ctor_args);
	stmt->fetch.cls.fci.param_count = old_arg_count;

	/* on no results, return an empty array */
	if (error) {
		PDO_HANDLE_STMT_ERR();
		if (C_TYPE_P(return_value) != IS_ARRAY) {
			array_init(return_value);
		}
	}
}
/* }}} */

static void register_bound_param(INTERNAL_FUNCTION_PARAMETERS, int is_param) /* {{{ */
{
	struct pdo_bound_param_data param;
	crex_long param_type = PDO_PARAM_STR;
	zval *parameter, *driver_params = NULL;

	memset(&param, 0, sizeof(param));

	CREX_PARSE_PARAMETERS_START(2, 5)
		C_PARAM_STR_OR_LONG(param.name, param.paramno)
		C_PARAM_ZVAL(parameter)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(param_type)
		C_PARAM_LONG(param.max_value_len)
		C_PARAM_ZVAL_OR_NULL(driver_params)
	CREX_PARSE_PARAMETERS_END();

	CRX_STMT_GET_OBJ;

	param.param_type = (int) param_type;

	if (param.name) {
		if (ZSTR_LEN(param.name) == 0) {
			crex_argument_value_error(1, "cannot be empty");
			RETURN_THROWS();
		}
		param.paramno = -1;
	} else if (param.paramno > 0) {
		--param.paramno; /* make it zero-based internally */
	} else {
		crex_argument_value_error(1, "must be greater than or equal to 1");
		RETURN_THROWS();
	}

	if (driver_params) {
		ZVAL_COPY(&param.driver_params, driver_params);
	}

	ZVAL_COPY(&param.parameter, parameter);
	if (!really_register_bound_param(&param, stmt, is_param)) {
		if (!C_ISUNDEF(param.parameter)) {
			zval_ptr_dtor(&(param.parameter));
		}

		RETURN_FALSE;
	}

	RETURN_TRUE;
} /* }}} */

/* {{{ bind an input parameter to the value of a CRX variable.  $paramno is the 1-based position of the placeholder in the SQL statement (but can be the parameter name for drivers that support named placeholders).  It should be called prior to execute(). */
CRX_METHOD(PDOStatement, bindValue)
{
	struct pdo_bound_param_data param;
	crex_long param_type = PDO_PARAM_STR;
	zval *parameter;

	memset(&param, 0, sizeof(param));

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR_OR_LONG(param.name, param.paramno)
		C_PARAM_ZVAL(parameter)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(param_type)
	CREX_PARSE_PARAMETERS_END();

	CRX_STMT_GET_OBJ;
	param.param_type = (int) param_type;

	if (param.name) {
		if (ZSTR_LEN(param.name) == 0) {
			crex_argument_value_error(1, "cannot be empty");
			RETURN_THROWS();
		}
		param.paramno = -1;
	} else if (param.paramno > 0) {
		--param.paramno; /* make it zero-based internally */
	} else {
		crex_argument_value_error(1, "must be greater than or equal to 1");
		RETURN_THROWS();
	}

	ZVAL_COPY(&param.parameter, parameter);
	if (!really_register_bound_param(&param, stmt, TRUE)) {
		if (!C_ISUNDEF(param.parameter)) {
			zval_ptr_dtor(&(param.parameter));
			ZVAL_UNDEF(&param.parameter);
		}
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ bind a parameter to a CRX variable.  $paramno is the 1-based position of the placeholder in the SQL statement (but can be the parameter name for drivers that support named placeholders).  This isn't supported by all drivers.  It should be called prior to execute(). */
CRX_METHOD(PDOStatement, bindParam)
{
	register_bound_param(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ bind a column to a CRX variable.  On each row fetch $param will contain the value of the corresponding column.  $column is the 1-based offset of the column, or the column name.  For portability, don't call this before execute(). */
CRX_METHOD(PDOStatement, bindColumn)
{
	register_bound_param(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ Returns the number of rows in a result set, or the number of rows affected by the last execute().  It is not always meaningful. */
CRX_METHOD(PDOStatement, rowCount)
{
	CREX_PARSE_PARAMETERS_NONE();

	CRX_STMT_GET_OBJ;
	RETURN_LONG(stmt->row_count);
}
/* }}} */

/* {{{ Fetch the error code associated with the last operation on the statement handle */
CRX_METHOD(PDOStatement, errorCode)
{
	CREX_PARSE_PARAMETERS_NONE();

	CRX_STMT_GET_OBJ;
	if (stmt->error_code[0] == '\0') {
		RETURN_NULL();
	}

	RETURN_STRING(stmt->error_code);
}
/* }}} */

/* {{{ Fetch extended error information associated with the last operation on the statement handle */
CRX_METHOD(PDOStatement, errorInfo)
{
	int error_count;
	int error_count_diff = 0;
	int error_expected_count = 3;

	CREX_PARSE_PARAMETERS_NONE();

	CRX_STMT_GET_OBJ;
	array_init(return_value);
	add_next_index_string(return_value, stmt->error_code);

	if (strncmp(stmt->error_code, PDO_ERR_NONE, sizeof(PDO_ERR_NONE))) {
		if (stmt->dbh->methods->fetch_err) {
			stmt->dbh->methods->fetch_err(stmt->dbh, stmt, return_value);
		}
	}

	error_count = crex_hash_num_elements(C_ARRVAL_P(return_value));

	if (error_expected_count > error_count) {
		int current_index;

		error_count_diff = error_expected_count - error_count;
		for (current_index = 0; current_index < error_count_diff; current_index++) {
			add_next_index_null(return_value);
		}
	}
}
/* }}} */

/* {{{ Set an attribute */
CRX_METHOD(PDOStatement, setAttribute)
{
	crex_long attr;
	zval *value = NULL;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_LONG(attr)
		C_PARAM_ZVAL_OR_NULL(value)
	CREX_PARSE_PARAMETERS_END();

	CRX_STMT_GET_OBJ;

	/* Driver hasn't registered a function for setting attributes */
	if (!stmt->methods->set_attribute) {
		pdo_raise_impl_error(stmt->dbh, stmt, "IM001", "This driver doesn't support setting attributes");
		RETURN_FALSE;
	}

	PDO_STMT_CLEAR_ERR();
	if (stmt->methods->set_attribute(stmt, attr, value)) {
		RETURN_TRUE;
	}

	/* Error while setting attribute */
	PDO_HANDLE_STMT_ERR();
	RETURN_FALSE;
}
/* }}} */

/* {{{ Get an attribute */

static bool generic_stmt_attr_get(pdo_stmt_t *stmt, zval *return_value, crex_long attr)
{
	switch (attr) {
		case PDO_ATTR_EMULATE_PREPARES:
			RETVAL_BOOL(stmt->supports_placeholders == PDO_PLACEHOLDER_NONE);
			return 1;
	}
	return 0;
}

CRX_METHOD(PDOStatement, getAttribute)
{
	crex_long attr;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(attr)
	CREX_PARSE_PARAMETERS_END();

	CRX_STMT_GET_OBJ;
	if (!stmt->methods->get_attribute) {
		if (!generic_stmt_attr_get(stmt, return_value, attr)) {
			pdo_raise_impl_error(stmt->dbh, stmt, "IM001",
				"This driver doesn't support getting attributes");
			RETURN_FALSE;
		}
		return;
	}

	PDO_STMT_CLEAR_ERR();
	switch (stmt->methods->get_attribute(stmt, attr, return_value)) {
		case -1:
			PDO_HANDLE_STMT_ERR();
			RETURN_FALSE;

		case 0:
			if (!generic_stmt_attr_get(stmt, return_value, attr)) {
				/* XXX: should do something better here */
				pdo_raise_impl_error(stmt->dbh, stmt, "IM001",
					"driver doesn't support getting that attribute");
				RETURN_FALSE;
			}
			return;

		default:
			return;
	}
}
/* }}} */

/* {{{ Returns the number of columns in the result set */
CRX_METHOD(PDOStatement, columnCount)
{
	CREX_PARSE_PARAMETERS_NONE();

	CRX_STMT_GET_OBJ;
	RETURN_LONG(stmt->column_count);
}
/* }}} */

/* {{{ Returns meta data for a numbered column */
CRX_METHOD(PDOStatement, getColumnMeta)
{
	crex_long colno;
	struct pdo_column_data *col;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(colno)
	CREX_PARSE_PARAMETERS_END();

	CRX_STMT_GET_OBJ;
	if (colno < 0) {
		crex_argument_value_error(1, "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	if (!stmt->methods->get_column_meta) {
		pdo_raise_impl_error(stmt->dbh, stmt, "IM001", "driver doesn't support meta data");
		RETURN_FALSE;
	}

	PDO_STMT_CLEAR_ERR();
	if (FAILURE == stmt->methods->get_column_meta(stmt, colno, return_value)) {
		PDO_HANDLE_STMT_ERR();
		RETURN_FALSE;
	}

	/* add stock items */
	col = &stmt->columns[colno];
	add_assoc_str(return_value, "name", crex_string_copy(col->name));
	add_assoc_long(return_value, "len", col->maxlen); /* FIXME: unsigned ? */
	add_assoc_long(return_value, "precision", col->precision);
}
/* }}} */

/* {{{ Changes the default fetch mode for subsequent fetches (params have different meaning for different fetch modes) */

bool pdo_stmt_setup_fetch_mode(pdo_stmt_t *stmt, crex_long mode, uint32_t mode_arg_num,
	zval *args, uint32_t variadic_num_args)
{
	int flags = 0;
	uint32_t arg1_arg_num = mode_arg_num + 1;
	uint32_t constructor_arg_num = mode_arg_num + 2;
	uint32_t total_num_args = mode_arg_num + variadic_num_args;

	switch (stmt->default_fetch_type) {
		case PDO_FETCH_INTO:
			if (!C_ISUNDEF(stmt->fetch.into)) {
				zval_ptr_dtor(&stmt->fetch.into);
				ZVAL_UNDEF(&stmt->fetch.into);
			}
			break;
		default:
			;
	}

	stmt->default_fetch_type = PDO_FETCH_BOTH;

	flags = mode & PDO_FETCH_FLAGS;

	if (!pdo_stmt_verify_mode(stmt, mode, mode_arg_num, false)) {
		return false;
	}

	switch (mode & ~PDO_FETCH_FLAGS) {
		case PDO_FETCH_USE_DEFAULT:
		case PDO_FETCH_LAZY:
		case PDO_FETCH_ASSOC:
		case PDO_FETCH_NUM:
		case PDO_FETCH_BOTH:
		case PDO_FETCH_OBJ:
		case PDO_FETCH_BOUND:
		case PDO_FETCH_NAMED:
		case PDO_FETCH_KEY_PAIR:
			if (variadic_num_args != 0) {
				crex_string *func = get_active_function_or_method_name();
				crex_argument_count_error("%s() expects exactly %d arguments for the fetch mode provided, %d given",
					ZSTR_VAL(func), mode_arg_num, total_num_args);
				crex_string_release(func);
				return false;
			}
			break;

		case PDO_FETCH_COLUMN:
			if (variadic_num_args != 1) {
				crex_string *func = get_active_function_or_method_name();
				crex_argument_count_error("%s() expects exactly %d arguments for the fetch mode provided, %d given",
					ZSTR_VAL(func), arg1_arg_num, total_num_args);
				crex_string_release(func);
				return false;
			}
			if (C_TYPE(args[0]) != IS_LONG) {
				crex_argument_type_error(arg1_arg_num, "must be of type int, %s given", crex_zval_value_name(&args[0]));
				return false;
			}
			if (C_LVAL(args[0]) < 0) {
				crex_argument_value_error(arg1_arg_num, "must be greater than or equal to 0");
				return false;
			}
			stmt->fetch.column = C_LVAL(args[0]);
			break;

		case PDO_FETCH_CLASS: {
			HashTable *constructor_args = NULL;
			/* Undef constructor arguments */
			ZVAL_UNDEF(&stmt->fetch.cls.ctor_args);
			/* Gets its class name from 1st column */
			if ((flags & PDO_FETCH_CLASSTYPE) == PDO_FETCH_CLASSTYPE) {
				if (variadic_num_args != 0) {
					crex_string *func = get_active_function_or_method_name();
					crex_argument_count_error("%s() expects exactly %d arguments for the fetch mode provided, %d given",
						ZSTR_VAL(func), mode_arg_num, total_num_args);
					crex_string_release(func);
					return false;
				}
				stmt->fetch.cls.ce = NULL;
			} else {
				crex_class_entry *cep;
				if (variadic_num_args == 0) {
					crex_string *func = get_active_function_or_method_name();
					crex_argument_count_error("%s() expects at least %d arguments for the fetch mode provided, %d given",
						ZSTR_VAL(func), arg1_arg_num, total_num_args);
					crex_string_release(func);
					return false;
				}
				/* constructor_arguments can be null/not passed */
				if (variadic_num_args > 2) {
					crex_string *func = get_active_function_or_method_name();
					crex_argument_count_error("%s() expects at most %d arguments for the fetch mode provided, %d given",
						ZSTR_VAL(func), constructor_arg_num, total_num_args);
					crex_string_release(func);
					return false;
				}
				if (C_TYPE(args[0]) != IS_STRING) {
					crex_argument_type_error(arg1_arg_num, "must be of type string, %s given", crex_zval_value_name(&args[0]));
					return false;
				}
				cep = crex_lookup_class(C_STR(args[0]));
				if (!cep) {
					crex_argument_type_error(arg1_arg_num, "must be a valid class");
					return false;
				}
				/* Verify constructor_args (args[1]) is ?array */
				/* TODO: Improve logic? */
				if (variadic_num_args == 2) {
					if (C_TYPE(args[1]) != IS_NULL && C_TYPE(args[1]) != IS_ARRAY) {
						crex_argument_type_error(constructor_arg_num, "must be of type ?array, %s given",
							crex_zval_value_name(&args[1]));
						return false;
					}
					if (C_TYPE(args[1]) == IS_ARRAY && crex_hash_num_elements(C_ARRVAL(args[1]))) {
						constructor_args = C_ARRVAL(args[1]);
					}
				}
				stmt->fetch.cls.ce = cep;

				/* If constructor arguments are present and not empty */
				if (constructor_args) {
					ZVAL_ARR(&stmt->fetch.cls.ctor_args, crex_array_dup(constructor_args));
				}
			}

			do_fetch_class_prepare(stmt);
			break;
		}
		case PDO_FETCH_INTO:
			if (total_num_args != arg1_arg_num) {
				crex_string *func = get_active_function_or_method_name();
				crex_argument_count_error("%s() expects exactly %d arguments for the fetch mode provided, %d given",
					ZSTR_VAL(func), arg1_arg_num, total_num_args);
				crex_string_release(func);
				return false;
			}
			if (C_TYPE(args[0]) != IS_OBJECT) {
				crex_argument_type_error(arg1_arg_num, "must be of type object, %s given", crex_zval_value_name(&args[0]));
				return false;
			}

			ZVAL_COPY(&stmt->fetch.into, &args[0]);
			break;
		default:
			crex_argument_value_error(mode_arg_num, "must be one of the PDO::FETCH_* constants");
			return false;
	}

	stmt->default_fetch_type = mode;

	return true;
}

CRX_METHOD(PDOStatement, setFetchMode)
{
	crex_long fetch_mode;
	zval *args = NULL;
	uint32_t num_args = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l*", &fetch_mode, &args, &num_args) == FAILURE) {
		RETURN_THROWS();
	}

	CRX_STMT_GET_OBJ;

	do_fetch_opt_finish(stmt, 1);

	if (!pdo_stmt_setup_fetch_mode(stmt, fetch_mode, 1, args, num_args)) {
		RETURN_THROWS();
	}

	// TODO Void return?
	RETURN_TRUE;
}
/* }}} */

/* {{{ Advances to the next rowset in a multi-rowset statement handle. Returns true if it succeeded, false otherwise */

static bool pdo_stmt_do_next_rowset(pdo_stmt_t *stmt)
{
	pdo_stmt_reset_columns(stmt);

	if (!stmt->methods->next_rowset(stmt)) {
		/* Set the executed flag to 0 to reallocate columns on next execute */
		stmt->executed = 0;
		return 0;
	}

	pdo_stmt_describe_columns(stmt);

	return 1;
}

CRX_METHOD(PDOStatement, nextRowset)
{
	CREX_PARSE_PARAMETERS_NONE();

	CRX_STMT_GET_OBJ;
	if (!stmt->methods->next_rowset) {
		pdo_raise_impl_error(stmt->dbh, stmt, "IM001", "driver does not support multiple rowsets");
		RETURN_FALSE;
	}

	PDO_STMT_CLEAR_ERR();

	if (!pdo_stmt_do_next_rowset(stmt)) {
		PDO_HANDLE_STMT_ERR();
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Closes the cursor, leaving the statement ready for re-execution. */
CRX_METHOD(PDOStatement, closeCursor)
{
	CREX_PARSE_PARAMETERS_NONE();

	CRX_STMT_GET_OBJ;
	if (!stmt->methods->cursor_closer) {
		/* emulate it by fetching and discarding rows */
		do {
			while (stmt->methods->fetcher(stmt, PDO_FETCH_ORI_NEXT, 0))
				;
			if (!stmt->methods->next_rowset) {
				break;
			}

			if (!pdo_stmt_do_next_rowset(stmt)) {
				break;
			}

		} while (1);
		stmt->executed = 0;
		RETURN_TRUE;
	}

	PDO_STMT_CLEAR_ERR();

	if (!stmt->methods->cursor_closer(stmt)) {
		PDO_HANDLE_STMT_ERR();
		RETURN_FALSE;
	}
	stmt->executed = 0;
	RETURN_TRUE;
}
/* }}} */

/* {{{ A utility for internals hackers to debug parameter internals */
CRX_METHOD(PDOStatement, debugDumpParams)
{
	CREX_PARSE_PARAMETERS_NONE();

	crx_stream *out = crx_stream_open_wrapper("crx://output", "w", 0, NULL);
	struct pdo_bound_param_data *param;

	CREX_PARSE_PARAMETERS_NONE();

	CRX_STMT_GET_OBJ;

	if (out == NULL) {
		RETURN_FALSE;
	}

	/* break into multiple operations so query string won't be truncated at FORMAT_CONV_MAX_PRECISION */
	crx_stream_printf(out, "SQL: [%zd] ", ZSTR_LEN(stmt->query_string));
	crx_stream_write(out, ZSTR_VAL(stmt->query_string), ZSTR_LEN(stmt->query_string));
	crx_stream_write(out, "\n", 1);

	/* show parsed SQL if emulated prepares enabled */
	/* pointers will be equal if PDO::query() was invoked */
	if (stmt->active_query_string != NULL && stmt->active_query_string != stmt->query_string) {
		/* break into multiple operations so query string won't be truncated at FORMAT_CONV_MAX_PRECISION */
		crx_stream_printf(out, "Sent SQL: [%zd] ", ZSTR_LEN(stmt->active_query_string));
		crx_stream_write(out, ZSTR_VAL(stmt->active_query_string), ZSTR_LEN(stmt->active_query_string));
		crx_stream_write(out, "\n", 1);
	}

	crx_stream_printf(out, "Params:  %d\n",
		stmt->bound_params ? crex_hash_num_elements(stmt->bound_params) : 0);

	if (stmt->bound_params) {
		crex_ulong num;
		crex_string *key = NULL;
		CREX_HASH_FOREACH_KEY_PTR(stmt->bound_params, num, key, param) {
			if (key) {
				crx_stream_printf(out, "Key: Name: [%zd] %.*s\n",
					ZSTR_LEN(key), (int) ZSTR_LEN(key), ZSTR_VAL(key));
			} else {
				crx_stream_printf(out, "Key: Position #" CREX_ULONG_FMT ":\n", num);
			}

			crx_stream_printf(out,
				"paramno=" CREX_LONG_FMT "\n"
				"name=[%zd] \"%.*s\"\n"
				"is_param=%d\n"
				"param_type=%d\n",
				param->paramno, param->name ? ZSTR_LEN(param->name) : 0, param->name ? (int) ZSTR_LEN(param->name) : 0,
				param->name ? ZSTR_VAL(param->name) : "",
				param->is_param,
				param->param_type);

		} CREX_HASH_FOREACH_END();
	}

	crx_stream_close(out);
}
/* }}} */

CRX_METHOD(PDOStatement, getIterator)
{
	if (crex_parse_parameters_none() == FAILURE) {
		return;
	}

	crex_create_internal_iterator_zval(return_value, CREX_THIS);
}

/* {{{ overloaded handlers for PDOStatement class */
static zval *dbstmt_prop_write(crex_object *object, crex_string *name, zval *value, void **cache_slot)
{
	if (crex_string_equals_literal(name, "queryString")) {
		zval *query_string = OBJ_PROP_NUM(object, 0);
		if (!C_ISUNDEF_P(query_string)) {
			crex_throw_error(NULL, "Property queryString is read only");
			return value;
		}
	}
	return crex_std_write_property(object, name, value, cache_slot);
}

static void dbstmt_prop_delete(crex_object *object, crex_string *name, void **cache_slot)
{
	if (crex_string_equals_literal(name, "queryString")) {
		crex_throw_error(NULL, "Property queryString is read only");
	} else {
		crex_std_unset_property(object, name, cache_slot);
	}
}

static crex_function *dbstmt_method_get(crex_object **object_pp, crex_string *method_name, const zval *key)
{
	crex_function *fbc = NULL;
	crex_string *lc_method_name;
	crex_object *object = *object_pp;

	lc_method_name = crex_string_tolower(method_name);

	if ((fbc = crex_hash_find_ptr(&object->ce->function_table, lc_method_name)) == NULL) {
		pdo_stmt_t *stmt = crx_pdo_stmt_fetch_object(object);
		/* instance not created by PDO object */
		if (!stmt->dbh) {
			goto out;
		}
		/* not a pre-defined method, nor a user-defined method; check
		 * the driver specific methods */
		if (!stmt->dbh->cls_methods[PDO_DBH_DRIVER_METHOD_KIND_STMT]) {
			if (!pdo_hash_methods(C_PDO_OBJECT_P(&stmt->database_object_handle),
				PDO_DBH_DRIVER_METHOD_KIND_STMT)
				|| !stmt->dbh->cls_methods[PDO_DBH_DRIVER_METHOD_KIND_STMT]) {
				goto out;
			}
		}

		if ((fbc = crex_hash_find_ptr(stmt->dbh->cls_methods[PDO_DBH_DRIVER_METHOD_KIND_STMT], lc_method_name)) == NULL) {
			goto out;
		}
		/* got it */
	}

out:
	crex_string_release_ex(lc_method_name, 0);
	if (!fbc) {
		fbc = crex_std_get_method(object_pp, method_name, key);
	}
	return fbc;
}

crex_object_handlers pdo_dbstmt_object_handlers;
crex_object_handlers pdo_row_object_handlers;

PDO_API void crx_pdo_free_statement(pdo_stmt_t *stmt)
{
	if (stmt->bound_params) {
		crex_hash_destroy(stmt->bound_params);
		FREE_HASHTABLE(stmt->bound_params);
		stmt->bound_params = NULL;
	}
	if (stmt->bound_param_map) {
		crex_hash_destroy(stmt->bound_param_map);
		FREE_HASHTABLE(stmt->bound_param_map);
		stmt->bound_param_map = NULL;
	}
	if (stmt->bound_columns) {
		crex_hash_destroy(stmt->bound_columns);
		FREE_HASHTABLE(stmt->bound_columns);
		stmt->bound_columns = NULL;
	}

	if (stmt->methods && stmt->methods->dtor) {
		stmt->methods->dtor(stmt);
	}
	if (stmt->active_query_string) {
		crex_string_release(stmt->active_query_string);
	}
	if (stmt->query_string) {
		crex_string_release(stmt->query_string);
	}

	pdo_stmt_reset_columns(stmt);

	if (!C_ISUNDEF(stmt->fetch.into) && stmt->default_fetch_type == PDO_FETCH_INTO) {
		zval_ptr_dtor(&stmt->fetch.into);
		ZVAL_UNDEF(&stmt->fetch.into);
	}

	do_fetch_opt_finish(stmt, 1);

	if (!C_ISUNDEF(stmt->database_object_handle)) {
		zval_ptr_dtor(&stmt->database_object_handle);
	}
	crex_object_std_dtor(&stmt->std);
}

void pdo_dbstmt_free_storage(crex_object *std)
{
	pdo_stmt_t *stmt = crx_pdo_stmt_fetch_object(std);
	crx_pdo_free_statement(stmt);
}

crex_object *pdo_dbstmt_new(crex_class_entry *ce)
{
	pdo_stmt_t *stmt;

	stmt = crex_object_alloc(sizeof(pdo_stmt_t), ce);
	crex_object_std_init(&stmt->std, ce);
	object_properties_init(&stmt->std, ce);

	return &stmt->std;
}
/* }}} */

/* {{{ statement iterator */

struct crx_pdo_iterator {
	crex_object_iterator iter;
	crex_ulong key;
	zval fetch_ahead;
};

static void pdo_stmt_iter_dtor(crex_object_iterator *iter)
{
	struct crx_pdo_iterator *I = (struct crx_pdo_iterator*)iter;

	zval_ptr_dtor(&I->iter.data);

	if (!C_ISUNDEF(I->fetch_ahead)) {
		zval_ptr_dtor(&I->fetch_ahead);
	}
}

static int pdo_stmt_iter_valid(crex_object_iterator *iter)
{
	struct crx_pdo_iterator *I = (struct crx_pdo_iterator*)iter;

	return C_ISUNDEF(I->fetch_ahead) ? FAILURE : SUCCESS;
}

static zval *pdo_stmt_iter_get_data(crex_object_iterator *iter)
{
	struct crx_pdo_iterator *I = (struct crx_pdo_iterator*)iter;

	/* sanity */
	if (C_ISUNDEF(I->fetch_ahead)) {
		return NULL;
	}

	return &I->fetch_ahead;
}

static void pdo_stmt_iter_get_key(crex_object_iterator *iter, zval *key)
{
	struct crx_pdo_iterator *I = (struct crx_pdo_iterator*)iter;

	if (I->key == (crex_ulong)-1) {
		ZVAL_NULL(key);
	} else {
		ZVAL_LONG(key, I->key);
	}
}

static void pdo_stmt_iter_move_forwards(crex_object_iterator *iter)
{
	struct crx_pdo_iterator *I = (struct crx_pdo_iterator*)iter;
	pdo_stmt_t *stmt = C_PDO_STMT_P(&I->iter.data); /* for PDO_HANDLE_STMT_ERR() */

	if (!C_ISUNDEF(I->fetch_ahead)) {
		zval_ptr_dtor(&I->fetch_ahead);
	}

	if (!do_fetch(stmt, &I->fetch_ahead, PDO_FETCH_USE_DEFAULT,
			PDO_FETCH_ORI_NEXT, /* offset */ 0, NULL)) {

		PDO_HANDLE_STMT_ERR();
		I->key = (crex_ulong)-1;
		ZVAL_UNDEF(&I->fetch_ahead);

		return;
	}

	I->key++;
}

static const crex_object_iterator_funcs pdo_stmt_iter_funcs = {
	pdo_stmt_iter_dtor,
	pdo_stmt_iter_valid,
	pdo_stmt_iter_get_data,
	pdo_stmt_iter_get_key,
	pdo_stmt_iter_move_forwards,
	NULL,
	NULL,
	NULL, /* get_gc */
};

crex_object_iterator *pdo_stmt_iter_get(crex_class_entry *ce, zval *object, int by_ref)
{
	if (by_ref) {
		crex_throw_error(NULL, "An iterator cannot be used with foreach by reference");
		return NULL;
	}

	pdo_stmt_t *stmt = C_PDO_STMT_P(object);
	if (!stmt->dbh) {
		crex_throw_error(NULL, "PDO object is uninitialized");
		return NULL;
	}

	struct crx_pdo_iterator *I = ecalloc(1, sizeof(struct crx_pdo_iterator));
	crex_iterator_init(&I->iter);
	I->iter.funcs = &pdo_stmt_iter_funcs;
	C_ADDREF_P(object);
	ZVAL_OBJ(&I->iter.data, C_OBJ_P(object));

	if (!do_fetch(stmt, &I->fetch_ahead, PDO_FETCH_USE_DEFAULT,
			PDO_FETCH_ORI_NEXT, /* offset */ 0, NULL)) {
		PDO_HANDLE_STMT_ERR();
		I->key = (crex_ulong)-1;
		ZVAL_UNDEF(&I->fetch_ahead);
	}

	return &I->iter;
}

/* }}} */

/* {{{ overloaded handlers for PDORow class (used by PDO_FETCH_LAZY) */

static zval *row_prop_read(crex_object *object, crex_string *name, int type, void **cache_slot, zval *rv)
{
	pdo_row_t *row = (pdo_row_t *)object;
	pdo_stmt_t *stmt = row->stmt;
	int colno = -1;
	crex_long lval;
	CREX_ASSERT(stmt);

	ZVAL_NULL(rv);
	if (crex_string_equals_literal(name, "queryString")) {
		return crex_std_read_property(&stmt->std, name, type, cache_slot, rv);
	} else if (is_numeric_string(ZSTR_VAL(name), ZSTR_LEN(name), &lval, NULL, 0) == IS_LONG) {
		if (lval >= 0 && lval < stmt->column_count) {
			fetch_value(stmt, rv, lval, NULL);
		}
	} else {
		/* TODO: replace this with a hash of available column names to column
		 * numbers */
		for (colno = 0; colno < stmt->column_count; colno++) {
			if (crex_string_equals(stmt->columns[colno].name, name)) {
				fetch_value(stmt, rv, colno, NULL);
				return rv;
			}
		}
	}

	return rv;
}

static zval *row_dim_read(crex_object *object, zval *member, int type, zval *rv)
{
	pdo_row_t *row = (pdo_row_t *)object;
	pdo_stmt_t *stmt = row->stmt;
	int colno = -1;
	crex_long lval;
	CREX_ASSERT(stmt);

	ZVAL_NULL(rv);
	if (C_TYPE_P(member) == IS_LONG) {
		if (C_LVAL_P(member) >= 0 && C_LVAL_P(member) < stmt->column_count) {
			fetch_value(stmt, rv, C_LVAL_P(member), NULL);
		}
	} else if (C_TYPE_P(member) == IS_STRING
		   && is_numeric_string(C_STRVAL_P(member), C_STRLEN_P(member), &lval, NULL, 0) == IS_LONG)	{
		if (lval >= 0 && lval < stmt->column_count) {
			fetch_value(stmt, rv, lval, NULL);
		}
	} else {
		if (!try_convert_to_string(member)) {
			return &EG(uninitialized_zval);
		}

		if (crex_string_equals_literal(C_STR_P(member), "queryString")) {
			return crex_std_read_property(&stmt->std, C_STR_P(member), type, NULL, rv);
		}

		/* TODO: replace this with a hash of available column names to column
		 * numbers */
		for (colno = 0; colno < stmt->column_count; colno++) {
			if (crex_string_equals(stmt->columns[colno].name, C_STR_P(member))) {
				fetch_value(stmt, rv, colno, NULL);
				return rv;
			}
		}
	}

	return rv;
}

static zval *row_prop_write(crex_object *object, crex_string *name, zval *value, void **cache_slot)
{
	crex_throw_error(NULL, "Cannot write to PDORow property");
	return value;
}

static void row_dim_write(crex_object *object, zval *member, zval *value)
{
	crex_throw_error(NULL, "Cannot write to PDORow offset");
}

static int row_prop_exists(crex_object *object, crex_string *name, int check_empty, void **cache_slot)
{
	pdo_row_t *row = (pdo_row_t *)object;
	pdo_stmt_t *stmt = row->stmt;
	int colno = -1;
	crex_long lval;
	CREX_ASSERT(stmt);

	if (is_numeric_string(ZSTR_VAL(name), ZSTR_LEN(name), &lval, NULL, 0) == IS_LONG)	{
		return lval >=0 && lval < stmt->column_count;
	}

	/* TODO: replace this with a hash of available column names to column
	 * numbers */
	for (colno = 0; colno < stmt->column_count; colno++) {
		if (crex_string_equals(stmt->columns[colno].name, name)) {
			int res;
			zval val;

			fetch_value(stmt, &val, colno, NULL);
			res = check_empty ? i_crex_is_true(&val) : C_TYPE(val) != IS_NULL;
			zval_ptr_dtor_nogc(&val);

			return res;
		}
	}

	return 0;
}

static int row_dim_exists(crex_object *object, zval *member, int check_empty)
{
	pdo_row_t *row = (pdo_row_t *)object;
	pdo_stmt_t *stmt = row->stmt;
	int colno = -1;
	crex_long lval;
	CREX_ASSERT(stmt);

	if (C_TYPE_P(member) == IS_LONG) {
		return C_LVAL_P(member) >= 0 && C_LVAL_P(member) < stmt->column_count;
	} else if (C_TYPE_P(member) == IS_STRING) {
		if (is_numeric_string(C_STRVAL_P(member), C_STRLEN_P(member), &lval, NULL, 0) == IS_LONG)	{
			return lval >=0 && lval < stmt->column_count;
		}
	} else {
		if (!try_convert_to_string(member)) {
			return 0;
		}
	}

	/* TODO: replace this with a hash of available column names to column
	 * numbers */
	for (colno = 0; colno < stmt->column_count; colno++) {
		if (crex_string_equals(stmt->columns[colno].name, C_STR_P(member))) {
			int res;
			zval val;

			fetch_value(stmt, &val, colno, NULL);
			res = check_empty ? i_crex_is_true(&val) : C_TYPE(val) != IS_NULL;
			zval_ptr_dtor_nogc(&val);

			return res;
		}
	}

	return 0;
}

static void row_prop_delete(crex_object *object, crex_string *offset, void **cache_slot)
{
	crex_throw_error(NULL, "Cannot unset PDORow property");
}

static void row_dim_delete(crex_object *object, zval *offset)
{
	crex_throw_error(NULL, "Cannot unset PDORow offset");
}

static HashTable *row_get_properties_for(crex_object *object, crex_prop_purpose purpose)
{
	pdo_row_t *row = (pdo_row_t *)object;
	pdo_stmt_t *stmt = row->stmt;
	HashTable *props;
	int i;
	CREX_ASSERT(stmt);

	if (purpose != CREX_PROP_PURPOSE_DEBUG) {
		return crex_std_get_properties_for(object, purpose);
	}

	if (!stmt->std.properties) {
		rebuild_object_properties(&stmt->std);
	}
	props = crex_array_dup(stmt->std.properties);
	for (i = 0; i < stmt->column_count; i++) {
		if (crex_string_equals_literal(stmt->columns[i].name, "queryString")) {
			continue;
		}

		zval val;
		fetch_value(stmt, &val, i, NULL);

		crex_hash_update(props, stmt->columns[i].name, &val);
	}
	return props;
}

static crex_function *row_get_ctor(crex_object *object)
{
	crex_throw_exception_ex(crx_pdo_get_exception(), 0, "You may not create a PDORow manually");
	return NULL;
}

void pdo_row_free_storage(crex_object *std)
{
	pdo_row_t *row = (pdo_row_t *)std;
	if (row->stmt) {
		ZVAL_UNDEF(&row->stmt->lazy_object_ref);
		OBJ_RELEASE(&row->stmt->std);
	}
}

crex_object *pdo_row_new(crex_class_entry *ce)
{
	pdo_row_t *row = ecalloc(1, sizeof(pdo_row_t));
	crex_object_std_init(&row->std, ce);

	return &row->std;
}

void pdo_stmt_init(void)
{
	pdo_dbstmt_ce = register_class_PDOStatement(crex_ce_aggregate);
	pdo_dbstmt_ce->get_iterator = pdo_stmt_iter_get;
	pdo_dbstmt_ce->create_object = pdo_dbstmt_new;
	pdo_dbstmt_ce->default_object_handlers = &pdo_dbstmt_object_handlers;

	memcpy(&pdo_dbstmt_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	pdo_dbstmt_object_handlers.offset = XtOffsetOf(pdo_stmt_t, std);
	pdo_dbstmt_object_handlers.free_obj = pdo_dbstmt_free_storage;
	pdo_dbstmt_object_handlers.write_property = dbstmt_prop_write;
	pdo_dbstmt_object_handlers.unset_property = dbstmt_prop_delete;
	pdo_dbstmt_object_handlers.get_method = dbstmt_method_get;
	pdo_dbstmt_object_handlers.compare = crex_objects_not_comparable;
	pdo_dbstmt_object_handlers.clone_obj = NULL;

	pdo_row_ce = register_class_PDORow();
	pdo_row_ce->create_object = pdo_row_new;
	pdo_row_ce->default_object_handlers = &pdo_row_object_handlers;

	memcpy(&pdo_row_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	pdo_row_object_handlers.free_obj = pdo_row_free_storage;
	pdo_row_object_handlers.clone_obj = NULL;
	pdo_row_object_handlers.get_property_ptr_ptr = NULL;
	pdo_row_object_handlers.read_property = row_prop_read;
	pdo_row_object_handlers.write_property = row_prop_write;
	pdo_row_object_handlers.has_property = row_prop_exists;
	pdo_row_object_handlers.unset_property = row_prop_delete;
	pdo_row_object_handlers.read_dimension = row_dim_read;
	pdo_row_object_handlers.write_dimension = row_dim_write;
	pdo_row_object_handlers.has_dimension = row_dim_exists;
	pdo_row_object_handlers.unset_dimension = row_dim_delete;
	pdo_row_object_handlers.get_properties_for = row_get_properties_for;
	pdo_row_object_handlers.get_constructor = row_get_ctor;
	pdo_row_object_handlers.compare = crex_objects_not_comparable;
}
