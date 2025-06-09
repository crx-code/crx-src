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
   | Authors: Stig Sæther Bakken <ssb@crx.net>                            |
   |          Thies C. Arntzen <thies@thieso.net>                         |
   |                                                                      |
   | Collection support by Andy Sautins <asautins@veripost.net>           |
   | Temporary LOB support by David Benson <dbenson@mancala.com>          |
   | ZTS per process OCIPLogon by Harald Radi <harald.radi@nme.at>        |
   |                                                                      |
   | Redesigned by: Antony Dovgal <antony@crex.com>                       |
   |                Andi Gutmans <andi@crx.net>                           |
   |                Wez Furlong <wez@omniti.com>                          |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "ext/standard/info.h"
#include "crx_ini.h"

#ifdef HAVE_OCI8

#include "crx_oci8.h"
#include "crx_oci8_int.h"

#if defined(__x86_64__) || defined(__LP64__) || defined(_LP64) || defined(_WIN64)
typedef ub8 oci_crxsized_int;
#else
typedef ub4 oci_crxsized_int;
#endif

/* {{{ crx_oci_statement_create()
 Create statemend handle and allocate necessary resources */
crx_oci_statement *crx_oci_statement_create(crx_oci_connection *connection, char *query, int query_len)
{
	crx_oci_statement *statement;
	sword errstatus;

	connection->errcode = 0; /* retain backwards compat with OCI8 1.4 */

	statement = ecalloc(1,sizeof(crx_oci_statement));

	if (!query_len) {
		/* do not allocate stmt handle for refcursors, we'll get it from OCIStmtPrepare2() */
		CRX_OCI_CALL(OCIHandleAlloc, (connection->env, (dvoid **)&(statement->stmt), OCI_HTYPE_STMT, 0, NULL));
	}

	CRX_OCI_CALL(OCIHandleAlloc, (connection->env, (dvoid **)&(statement->err), OCI_HTYPE_ERROR, 0, NULL));

	if (query_len > 0) {
		CRX_OCI_CALL_RETURN(errstatus, OCIStmtPrepare2,
				(
				 connection->svc,
				 &(statement->stmt),
				 connection->err,
				 (text *)query,
				 query_len,
				 NULL,
				 0,
				 OCI_NTV_SYNTAX,
				 OCI_DEFAULT
				)
		);
#ifdef HAVE_OCI8_DTRACE
		if (DTRACE_OCI8_SQLTEXT_ENABLED()) {
			DTRACE_OCI8_SQLTEXT(connection, connection->client_id, statement, query);
		}
#endif /* HAVE_OCI8_DTRACE */

		if (errstatus != OCI_SUCCESS) {
			connection->errcode = crx_oci_error(connection->err, errstatus);

			CRX_OCI_CALL(OCIStmtRelease, (statement->stmt, statement->err, NULL, 0, OCI_STRLS_CACHE_DELETE));
			CRX_OCI_CALL(OCIHandleFree,(statement->err, OCI_HTYPE_ERROR));
			CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
			efree(statement);
			return NULL;
		}
	}

	if (query && query_len) {
		statement->last_query = ecalloc(1, query_len + 1);
		memcpy(statement->last_query, query, query_len);
		statement->last_query_len = query_len;
	}
	else {
		statement->last_query = NULL;
		statement->last_query_len = 0;
	}

	statement->connection = connection;
	statement->has_data = 0;
	statement->has_descr = 0;
	statement->parent_stmtid = 0;
	statement->impres_child_stmt = NULL;
	statement->impres_count = 0;
	statement->impres_flag = CRX_OCI_IMPRES_UNKNOWN;  /* may or may not have Implicit Result Set children */
	GC_ADDREF(statement->connection->id);

	if (OCI_G(default_prefetch) >= 0) {
		crx_oci_statement_set_prefetch(statement, (ub4)OCI_G(default_prefetch));
	} else {
		crx_oci_statement_set_prefetch(statement, (ub4)100); /* semi-arbitrary, "sensible default" */
	}

	if (OCI_G(prefetch_lob_size) > 0) {
		statement->prefetch_lob_size = (ub4)OCI_G(prefetch_lob_size);
	} else {
		statement->prefetch_lob_size = 0;
	}		
	
	CRX_OCI_REGISTER_RESOURCE(statement, le_statement);

	OCI_G(num_statements)++;

	return statement;
}
/* }}} */

/* {{{ crx_oci_get_implicit_resultset()
   Fetch implicit result set statement resource */
crx_oci_statement *crx_oci_get_implicit_resultset(crx_oci_statement *statement)
{
#if (OCI_MAJOR_VERSION < 12)
	crx_error_docref(NULL, E_WARNING, "Implicit results are available in Oracle Database 12c onwards");
	return NULL;
#else
	void *result;
	ub4   rtype;
	crx_oci_statement *statement2;  /* implicit result set statement handle */
	sword errstatus;

	CRX_OCI_CALL_RETURN(errstatus, OCIStmtGetNextResult, (statement->stmt, statement->err, &result, &rtype, OCI_DEFAULT));
	if (errstatus == OCI_NO_DATA) {
		return NULL;
	}

	if (rtype != OCI_RESULT_TYPE_SELECT) {
		/* Only OCI_RESULT_TYPE_SELECT is supported by Oracle DB 12cR1 */
		crx_error_docref(NULL, E_WARNING, "Unexpected implicit result type returned from Oracle Database");
		return NULL;
	} else {
		statement2 = ecalloc(1,sizeof(crx_oci_statement));

		CRX_OCI_CALL(OCIHandleAlloc, (statement->connection->env, (dvoid **)&(statement2->err), OCI_HTYPE_ERROR, 0, NULL));
		statement2->stmt = (OCIStmt *)result;
		statement2->parent_stmtid = statement->id;
		statement2->impres_child_stmt = NULL;
		statement2->impres_count = 0;
		statement2->impres_flag = CRX_OCI_IMPRES_IS_CHILD;
		statement2->connection = statement->connection;
		statement2->errcode = 0;
		statement2->last_query = NULL;
		statement2->last_query_len = 0;
		statement2->columns = NULL;
		statement2->binds = NULL;
		statement2->defines = NULL;
		statement2->ncolumns = 0;
		statement2->executed = 0;
		statement2->has_data = 0;
		statement2->has_descr = 0;
		statement2->stmttype = 0;

		GC_ADDREF(statement->id);
		GC_ADDREF(statement2->connection->id);

		crx_oci_statement_set_prefetch(statement2, statement->prefetch_count);
		statement2->prefetch_lob_size = statement->prefetch_lob_size;

		CRX_OCI_REGISTER_RESOURCE(statement2, le_statement);

		OCI_G(num_statements)++;

		return statement2;
	}
#endif /* OCI_MAJOR_VERSION < 12 */
}
/* }}} */

/* {{{ crx_oci_statement_set_prefetch()
 Set prefetch buffer size for the statement */
int crx_oci_statement_set_prefetch(crx_oci_statement *statement, ub4 prefetch)
{
	sword errstatus;

	if (prefetch > 20000) {
		prefetch = 20000;		/* keep it somewhat sane */
	}

	CRX_OCI_CALL_RETURN(errstatus, OCIAttrSet, (statement->stmt, OCI_HTYPE_STMT, &prefetch, 0, OCI_ATTR_PREFETCH_ROWS, statement->err));

	if (errstatus != OCI_SUCCESS) {
		statement->errcode = crx_oci_error(statement->err, errstatus);
		CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
		statement->prefetch_count = 0;
		return 1;
	}
	statement->prefetch_count = prefetch;
	statement->errcode = 0; /* retain backwards compat with OCI8 1.4 */
	return 0;
}
/* }}} */

/* {{{ crx_oci_cleanup_pre_fetch()
   Helper function to cleanup ref-cursors and descriptors from the previous row */
int crx_oci_cleanup_pre_fetch(zval *data)
{
	crx_oci_out_column *outcol = (crx_oci_out_column*) C_PTR_P(data);

	if (!outcol->is_descr && !outcol->is_cursor)
		return CREX_HASH_APPLY_KEEP;

	switch(outcol->data_type) {
		case SQLT_CLOB:
		case SQLT_BLOB:
		case SQLT_RDD:
		case SQLT_BFILE:
			if (outcol->descid) {
				crex_list_delete(outcol->descid);
				outcol->descid = 0;
			}
			break;
		case SQLT_RSET:
			if (outcol->stmtid) {
				crex_list_delete(outcol->stmtid);
				outcol->stmtid = 0;
				outcol->nested_statement = NULL;
			}
			break;
		default:
			break;
	}
	return CREX_HASH_APPLY_KEEP;

}
/* }}} */

/* {{{ crx_oci_statement_fetch()
 Fetch a row from the statement */
int crx_oci_statement_fetch(crx_oci_statement *statement, ub4 nrows)
{
	int i;
	void *handlepp;
	ub4 typep, iterp, idxp;
	ub1 in_outp, piecep;
	bool piecewisecols = 0;
	crx_oci_out_column *column;
	sword errstatus;

	statement->errcode = 0; /* retain backwards compat with OCI8 1.4 */

	if (statement->has_descr && statement->columns) {
		crex_hash_apply(statement->columns, crx_oci_cleanup_pre_fetch);
	}

	CRX_OCI_CALL_RETURN(errstatus, OCIStmtFetch2, (statement->stmt, statement->err, nrows, OCI_FETCH_NEXT, 0, OCI_DEFAULT));

	if (errstatus == OCI_NO_DATA || nrows == 0) {
		if (statement->last_query == NULL) {
			/* reset define-list for refcursors */
			if (statement->columns) {
				crex_hash_destroy(statement->columns);
				efree(statement->columns);
				statement->columns = NULL;
				statement->ncolumns = 0;
			}
			statement->executed = 0;
		}

		statement->has_data = 0;

		if (nrows == 0) {
			/* this is exactly what we requested */
			return 0;
		}
		return 1;
	}

	/* reset length for all piecewise columns */
	for (i = 0; i < statement->ncolumns; i++) {
		column = crx_oci_statement_get_column(statement, i + 1, NULL, 0);
		if (column && column->piecewise) {
			column->retlen4 = 0;
			piecewisecols = 1;
		}
	}

	while (errstatus == OCI_NEED_DATA) {
		if (piecewisecols) {
			CRX_OCI_CALL_RETURN(errstatus,
				OCIStmtGetPieceInfo,
				   (
					statement->stmt,
					statement->err,
					&handlepp,
					&typep,
					&in_outp,
					&iterp,
					&idxp,
					&piecep
				   )
			);

			/* scan through our columns for a piecewise column with a matching handle */
			for (i = 0; i < statement->ncolumns; i++) {
				column = crx_oci_statement_get_column(statement, i + 1, NULL, 0);
				if (column && column->piecewise && handlepp == column->oci_define)   {
					if (!column->data) {
						column->data = (text *) ecalloc(1, CRX_OCI_PIECE_SIZE + 1);
					} else {
						column->data = erealloc(column->data, column->retlen4 + CRX_OCI_PIECE_SIZE + 1);
					}
					column->cb_retlen = CRX_OCI_PIECE_SIZE;

					/* and instruct fetch to fetch waiting piece into our buffer */
					CRX_OCI_CALL(OCIStmtSetPieceInfo,
						   (
							(void *) column->oci_define,
							OCI_HTYPE_DEFINE,
							statement->err,
							((char*)column->data) + column->retlen4,
							&(column->cb_retlen),
							piecep,
							&column->indicator,
							&column->retcode
						   )
					);
				}
			}
		}

		CRX_OCI_CALL_RETURN(errstatus, OCIStmtFetch2, (statement->stmt, statement->err, nrows, OCI_FETCH_NEXT, 0, OCI_DEFAULT));

		if (piecewisecols) {
			for (i = 0; i < statement->ncolumns; i++) {
				column = crx_oci_statement_get_column(statement, i + 1, NULL, 0);
				if (column && column->piecewise && handlepp == column->oci_define)	{
					column->retlen4 += column->cb_retlen;
				}
			}
		}
	}

	if (errstatus == OCI_SUCCESS_WITH_INFO || errstatus == OCI_SUCCESS) {
		statement->has_data = 1;

		/* do the stuff needed for OCIDefineByPos */
		for (i = 0; i < statement->ncolumns; i++) {
			column = crx_oci_statement_get_column(statement, i + 1, NULL, 0);
			if (column == NULL) {
				continue;
			}

			if (!column->define) {
				continue;
			}

			CREX_ASSERT(C_ISREF(column->define->val));
			zval_ptr_dtor(C_REFVAL(column->define->val));
			ZVAL_NULL(C_REFVAL(column->define->val));
			crx_oci_column_to_zval(column, C_REFVAL(column->define->val), 0);
		}

		return 0;
	}

	statement->errcode = crx_oci_error(statement->err, errstatus);
	CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);

	statement->has_data = 0;

	return 1;
}
/* }}} */

/* {{{ crx_oci_statement_get_column()
 Get column from the result set */
crx_oci_out_column *crx_oci_statement_get_column(crx_oci_statement *statement, crex_long column_index, char *column_name, int column_name_len)
{
	crx_oci_out_column *column = NULL;
	int i;

	if (statement->columns == NULL) { /* we release the columns at the end of a fetch */
		return NULL;
	}

	if (column_name) {
		for (i = 0; i < statement->ncolumns; i++) {
			column = crx_oci_statement_get_column(statement, i + 1, NULL, 0);
			if (column == NULL) {
				continue;
			} else if (((int) column->name_len == column_name_len) && (!strncmp(column->name, column_name, column_name_len))) {
				return column;
			}
		}
	} else if (column_index != -1) {
		if ((column = crex_hash_index_find_ptr(statement->columns, column_index)) == NULL) {
			return NULL;
		}
		return column;
	}

	return NULL;
}
/* }}} */

/* {{{ crx_oci_define_callback() */
sb4 crx_oci_define_callback(dvoid *ctx, OCIDefine *define, ub4 iter, dvoid **bufpp, ub4 **alenpp, ub1 *piecep, dvoid **indpp, ub2 **rcpp)
{
	crx_oci_out_column *outcol = (crx_oci_out_column *)ctx;

	if (!outcol) {

		crx_error_docref(NULL, E_WARNING, "Invalid context pointer value");
		return OCI_ERROR;
	}

	switch(outcol->data_type) {
		case SQLT_RSET: {
				crx_oci_statement *nested_stmt;

				nested_stmt = crx_oci_statement_create(outcol->statement->connection, NULL, 0);
				if (!nested_stmt) {
					return OCI_ERROR;
				}
				nested_stmt->parent_stmtid = outcol->statement->id;
				GC_ADDREF(outcol->statement->id);
				outcol->nested_statement = nested_stmt;
				outcol->stmtid = nested_stmt->id;

				*bufpp = nested_stmt->stmt;
				*alenpp = &(outcol->retlen4);
				*piecep = OCI_ONE_PIECE;
				*indpp = &(outcol->indicator);
				*rcpp = &(outcol->retcode);
				return OCI_CONTINUE;
			}
			break;
		case SQLT_RDD:
		case SQLT_BLOB:
		case SQLT_CLOB:
		case SQLT_BFILE: {
				crx_oci_descriptor *descr;
				int dtype;

				if (outcol->data_type == SQLT_BFILE) {
					dtype = OCI_DTYPE_FILE;
				} else if (outcol->data_type == SQLT_RDD ) {
					dtype = OCI_DTYPE_ROWID;
				} else {
					dtype = OCI_DTYPE_LOB;
				}

				descr = crx_oci_lob_create(outcol->statement->connection, dtype);
				if (!descr) {
					return OCI_ERROR;
				}
				outcol->descid = descr->id;
				descr->charset_form = outcol->charset_form;

				*bufpp = descr->descriptor;
				*alenpp = &(outcol->retlen4);
				*piecep = OCI_ONE_PIECE;
				*indpp = &(outcol->indicator);
				*rcpp = &(outcol->retcode);

				return OCI_CONTINUE;
			}
			break;
	}
	return OCI_ERROR;
}
/* }}} */

/* {{{ crx_oci_statement_execute()
 Execute statement */
int crx_oci_statement_execute(crx_oci_statement *statement, ub4 mode)
{
	crx_oci_out_column *outcol;
	OCIParam *param = NULL;
	text *colname;
	ub4 counter;
	ub2 define_type;
	ub4 iters;
	ub4 colcount;
	ub2 dynamic;
	dvoid *buf;
	sword errstatus;

	switch (mode) {
		case OCI_COMMIT_ON_SUCCESS:
		case OCI_DESCRIBE_ONLY:
		case OCI_DEFAULT:
			/* only these are allowed */
#ifdef HAVE_OCI8_DTRACE
			if (DTRACE_OCI8_EXECUTE_MODE_ENABLED()) {
				DTRACE_OCI8_EXECUTE_MODE(statement->connection, statement->connection->client_id, statement, mode);
			}
#endif /* HAVE_OCI8_DTRACE */
			break;
		default:
			crx_error_docref(NULL, E_WARNING, "Invalid execute mode given: %d", mode);
			return 1;
			break;
	}

	if (!statement->stmttype) {
		/* get statement type */
		CRX_OCI_CALL_RETURN(errstatus, OCIAttrGet, ((dvoid *)statement->stmt, OCI_HTYPE_STMT, (ub2 *)&statement->stmttype, (ub4 *)0, OCI_ATTR_STMT_TYPE, statement->err));

		if (errstatus != OCI_SUCCESS) {
			statement->errcode = crx_oci_error(statement->err, errstatus);
			CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
			return 1;
		} else {
			statement->errcode = 0; /* retain backwards compat with OCI8 1.4 */
		}
	}

	if (statement->stmttype == OCI_STMT_SELECT) {
		iters = 0;
	} else {
		iters = 1;
	}

	if (statement->last_query) { /* Don't execute REFCURSORS or Implicit Result Set handles */

		if (statement->binds) {
			int result = 0;
			crex_hash_apply_with_argument(statement->binds, crx_oci_bind_pre_exec, (void *)&result);
			if (result) {
				return 1;
			}
		}

		/* execute statement */
		CRX_OCI_CALL_RETURN(errstatus, OCIStmtExecute, (statement->connection->svc, statement->stmt, statement->err, iters, 0, NULL, NULL, mode));

		if (errstatus != OCI_SUCCESS) {
			statement->errcode = crx_oci_error(statement->err, errstatus);
			CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
			return 1;
		}

		if (statement->binds) {
			crex_hash_apply(statement->binds, crx_oci_bind_post_exec);
		}

		if (mode & OCI_COMMIT_ON_SUCCESS) {
			/* No need to rollback on disconnect */
			statement->connection->rb_on_disconnect = 0;
		} else if (statement->stmttype != OCI_STMT_SELECT) {
			/* Assume some uncommitted DML occurred */
			statement->connection->rb_on_disconnect = 1;
		}
		/* else for SELECT with OCI_NO_AUTO_COMMIT, leave
		 * "rb_on_disconnect" at its previous value.  SELECT can't
		 * initiate uncommitted DML. (An AUTONOMOUS_TRANSACTION in
		 * invoked PL/SQL must explicitly rollback/commit else the
		 * SELECT fails).
		 */

		statement->errcode = 0; /* retain backwards compat with OCI8 1.4 */
	}

	if (statement->stmttype == OCI_STMT_SELECT && statement->executed == 0) {
		/* we only need to do the define step is this very statement is executed the first time! */
		statement->executed = 1;

		ALLOC_HASHTABLE(statement->columns);
		crex_hash_init(statement->columns, 13, NULL, crx_oci_column_hash_dtor, 0);

		counter = 1;

		/* get number of columns */
		CRX_OCI_CALL_RETURN(errstatus, OCIAttrGet, ((dvoid *)statement->stmt, OCI_HTYPE_STMT, (dvoid *)&colcount, (ub4 *)0, OCI_ATTR_PARAM_COUNT, statement->err));

		if (errstatus != OCI_SUCCESS) {
			statement->errcode = crx_oci_error(statement->err, errstatus);
			CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
			return 1;
		}

		statement->ncolumns = colcount;

		for (counter = 1; counter <= colcount; counter++) {
			outcol = (crx_oci_out_column *) ecalloc(1, sizeof(crx_oci_out_column));

			outcol = crex_hash_index_update_ptr(statement->columns, counter, outcol);

			/* get column */
			CRX_OCI_CALL_RETURN(errstatus, OCIParamGet, ((dvoid *)statement->stmt, OCI_HTYPE_STMT, statement->err, (dvoid**)&param, counter));

			if (errstatus != OCI_SUCCESS) {
				statement->errcode = crx_oci_error(statement->err, errstatus);
				CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
				return 1;
			}

			/* get column datatype */
			CRX_OCI_CALL_RETURN(errstatus, OCIAttrGet, ((dvoid *)param, OCI_DTYPE_PARAM, (dvoid *)&outcol->data_type, (ub4 *)0, OCI_ATTR_DATA_TYPE, statement->err));

			if (errstatus != OCI_SUCCESS) {
				CRX_OCI_CALL(OCIDescriptorFree, (param, OCI_DTYPE_PARAM));
				statement->errcode = crx_oci_error(statement->err, errstatus);
				CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
				return 1;
			}

			/* get character set form  */
			CRX_OCI_CALL_RETURN(errstatus, OCIAttrGet, ((dvoid *)param, OCI_DTYPE_PARAM, (dvoid *)&outcol->charset_form, (ub4 *)0, OCI_ATTR_CHARSET_FORM, statement->err));

			if (errstatus != OCI_SUCCESS) {
				CRX_OCI_CALL(OCIDescriptorFree, (param, OCI_DTYPE_PARAM));
				statement->errcode = crx_oci_error(statement->err, errstatus);
				CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
				return 1;
			}

			/* get character set id	 */
			CRX_OCI_CALL_RETURN(errstatus, OCIAttrGet, ((dvoid *)param, OCI_DTYPE_PARAM, (dvoid *)&outcol->charset_id, (ub4 *)0, OCI_ATTR_CHARSET_ID, statement->err));

			if (errstatus != OCI_SUCCESS) {
				CRX_OCI_CALL(OCIDescriptorFree, (param, OCI_DTYPE_PARAM));
				statement->errcode = crx_oci_error(statement->err, errstatus);
				CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
				return 1;
			}

			/* get size of the column */
			CRX_OCI_CALL_RETURN(errstatus, OCIAttrGet, ((dvoid *)param, OCI_DTYPE_PARAM, (dvoid *)&outcol->data_size, (dvoid *)0, OCI_ATTR_DATA_SIZE, statement->err));

			if (errstatus != OCI_SUCCESS) {
				CRX_OCI_CALL(OCIDescriptorFree, (param, OCI_DTYPE_PARAM));
				statement->errcode = crx_oci_error(statement->err, errstatus);
				CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
				return 1;
			}

			outcol->storage_size4 = outcol->data_size;
			outcol->retlen = outcol->data_size;

			/* get scale of the column */
			CRX_OCI_CALL_RETURN(errstatus, OCIAttrGet, ((dvoid *)param, OCI_DTYPE_PARAM, (dvoid *)&outcol->scale, (dvoid *)0, OCI_ATTR_SCALE, statement->err));

			if (errstatus != OCI_SUCCESS) {
				CRX_OCI_CALL(OCIDescriptorFree, (param, OCI_DTYPE_PARAM));
				statement->errcode = crx_oci_error(statement->err, errstatus);
				CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
				return 1;
			}

			/* get precision of the column */
			CRX_OCI_CALL_RETURN(errstatus, OCIAttrGet, ((dvoid *)param, OCI_DTYPE_PARAM, (dvoid *)&outcol->precision, (dvoid *)0, OCI_ATTR_PRECISION, statement->err));

			if (errstatus != OCI_SUCCESS) {
				CRX_OCI_CALL(OCIDescriptorFree, (param, OCI_DTYPE_PARAM));
				statement->errcode = crx_oci_error(statement->err, errstatus);
				CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
				return 1;
			}

			/* get name of the column */
			CRX_OCI_CALL_RETURN(errstatus, OCIAttrGet, ((dvoid *)param, OCI_DTYPE_PARAM, (dvoid **)&colname, (ub4 *)&outcol->name_len, (ub4)OCI_ATTR_NAME, statement->err));

			if (errstatus != OCI_SUCCESS) {
				CRX_OCI_CALL(OCIDescriptorFree, (param, OCI_DTYPE_PARAM));
				statement->errcode = crx_oci_error(statement->err, errstatus);
				CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
				return 1;
			}
			CRX_OCI_CALL(OCIDescriptorFree, (param, OCI_DTYPE_PARAM));

			outcol->name = ecalloc(1, outcol->name_len + 1);
			memcpy(outcol->name, colname, outcol->name_len);

			/* find a user-set define */
			if (statement->defines) {
				if ((outcol->define = crex_hash_str_find_ptr(statement->defines, outcol->name, outcol->name_len)) != NULL) {
					if (outcol->define->type) {
						outcol->data_type = outcol->define->type;
					}
				}
			}

			buf = 0;
			switch (outcol->data_type) {
				case SQLT_RSET:
					outcol->statement = statement; /* parent handle */

					define_type = SQLT_RSET;
					outcol->is_cursor = 1;
					outcol->statement->has_descr = 1;
					outcol->storage_size4 = -1;
					outcol->retlen = -1;
					dynamic = OCI_DYNAMIC_FETCH;
					break;

				case SQLT_RDD:	 /* ROWID */
				case SQLT_BLOB:	 /* binary LOB */
				case SQLT_CLOB:	 /* character LOB */
				case SQLT_BFILE: /* binary file LOB */
					outcol->statement = statement; /* parent handle */

					define_type = outcol->data_type;
					outcol->is_descr = 1;
					outcol->statement->has_descr = 1;
					outcol->storage_size4 = -1;
					outcol->chunk_size = 0;
					dynamic = OCI_DYNAMIC_FETCH;
					break;

				case SQLT_LNG:
				case SQLT_LBI:
					if (outcol->data_type == SQLT_LBI) {
						define_type = SQLT_BIN;
					} else {
						define_type = SQLT_CHR;
					}
					outcol->storage_size4 = CRX_OCI_MAX_DATA_SIZE;
					outcol->piecewise = 1;
					dynamic = OCI_DYNAMIC_FETCH;
					break;

				case SQLT_BIN:
				default:
					define_type = SQLT_CHR;
					if (outcol->data_type == SQLT_BIN) {
						define_type = SQLT_BIN;
					}
					if ((outcol->data_type == SQLT_DAT) || (outcol->data_type == SQLT_NUM)
#ifdef SQLT_TIMESTAMP
						|| (outcol->data_type == SQLT_TIMESTAMP)
#endif
#ifdef SQLT_TIMESTAMP_TZ
						|| (outcol->data_type == SQLT_TIMESTAMP_TZ)
#endif
#ifdef SQLT_TIMESTAMP_LTZ
						|| (outcol->data_type == SQLT_TIMESTAMP_LTZ)
#endif
#ifdef SQLT_INTERVAL_YM
						|| (outcol->data_type == SQLT_INTERVAL_YM)
#endif
#ifdef SQLT_INTERVAL_DS
						|| (outcol->data_type == SQLT_INTERVAL_DS)
#endif
						) {
						outcol->storage_size4 = 512; /* XXX this should fit "most" NLS date-formats and Numbers */
#if defined(SQLT_IBFLOAT) && defined(SQLT_IBDOUBLE)
					} else if (outcol->data_type == SQLT_IBFLOAT || outcol->data_type == SQLT_IBDOUBLE) {
						outcol->storage_size4 = 1024;
#endif
					} else {
						outcol->storage_size4++; /* add one for string terminator */
					}

					outcol->storage_size4 *= 3;

					dynamic = OCI_DEFAULT;
					buf = outcol->data = (text *) safe_emalloc(1, outcol->storage_size4, 0);
					memset(buf, 0, outcol->storage_size4);
					break;
			}

			if (dynamic == OCI_DYNAMIC_FETCH) {
				CRX_OCI_CALL_RETURN(errstatus,
					OCIDefineByPos,
					(
						statement->stmt,							/* IN/OUT handle to the requested SQL query */
						(OCIDefine **)&outcol->oci_define,			/* IN/OUT pointer to a pointer to a define handle */
						statement->err,								/* IN/OUT An error handle  */
						counter,									/* IN	  position in the select list */
						(dvoid *)NULL,								/* IN/OUT pointer to a buffer */
						outcol->storage_size4,						/* IN	  The size of each valuep buffer in bytes */
						define_type,								/* IN	  The data type */
						(dvoid *)&outcol->indicator,				/* IN	  pointer to an indicator variable or arr */
						(ub2 *)NULL,								/* IN/OUT Pointer to array of length of data fetched */
						(ub2 *)NULL,								/* OUT	  Pointer to array of column-level return codes */
						OCI_DYNAMIC_FETCH							/* IN	  mode (OCI_DEFAULT, OCI_DYNAMIC_FETCH) */
					)
				);
			} else {
				CRX_OCI_CALL_RETURN(errstatus,
					OCIDefineByPos,
					(
						statement->stmt,							/* IN/OUT handle to the requested SQL query */
						(OCIDefine **)&outcol->oci_define,			/* IN/OUT pointer to a pointer to a define handle */
						statement->err,								/* IN/OUT An error handle  */
						counter,									/* IN	  position in the select list */
						(dvoid *)buf,								/* IN/OUT pointer to a buffer */
						outcol->storage_size4,						/* IN	  The size of each valuep buffer in bytes */
						define_type,								/* IN	  The data type */
						(dvoid *)&outcol->indicator,				/* IN	  pointer to an indicator variable or arr */
						(ub2 *)&outcol->retlen,						/* IN/OUT Pointer to array of length of data fetched */
						(ub2 *)&outcol->retcode,					/* OUT	  Pointer to array of column-level return codes */
						OCI_DEFAULT									/* IN	  mode (OCI_DEFAULT, OCI_DYNAMIC_FETCH) */
					)
				);

			}

			if (errstatus != OCI_SUCCESS) {
				statement->errcode = crx_oci_error(statement->err, errstatus);
				CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
				return 1;
			}

			/* Enable LOB data prefetching. */
#if (OCI_MAJOR_VERSION > 12 || (OCI_MAJOR_VERSION == 12 && OCI_MINOR_VERSION >= 2))
			if ((outcol->data_type == SQLT_CLOB || outcol->data_type == SQLT_BLOB) && statement->prefetch_lob_size > 0) {
				int get_lob_len = 1;  /* == true */

				CRX_OCI_CALL_RETURN(errstatus, OCIAttrSet, (outcol->oci_define, OCI_HTYPE_DEFINE, &get_lob_len, 0, OCI_ATTR_LOBPREFETCH_LENGTH, statement->err));
				if (errstatus != OCI_SUCCESS) {
					statement->errcode = crx_oci_error(statement->err, errstatus);
					CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
					return 1;
				}

				CRX_OCI_CALL_RETURN(errstatus, OCIAttrSet, (outcol->oci_define, OCI_HTYPE_DEFINE, &(statement->prefetch_lob_size), 0, OCI_ATTR_LOBPREFETCH_SIZE, statement->err));
				if (errstatus != OCI_SUCCESS) {
					statement->errcode = crx_oci_error(statement->err, errstatus);
					CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
					return 1;
				}
			}
#endif

			/* additional define setup */
			switch (outcol->data_type) {
				case SQLT_RSET:
				case SQLT_BLOB:
				case SQLT_CLOB:
				case SQLT_RDD:
				case SQLT_BFILE:
					CRX_OCI_CALL_RETURN(errstatus,
						OCIDefineDynamic,
						(
							outcol->oci_define,
							statement->err,
							(dvoid *)outcol,
							crx_oci_define_callback
						)
					);

					if (errstatus != OCI_SUCCESS) {
						statement->errcode = crx_oci_error(statement->err, errstatus);
						CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
						return 1;
					}
					break;
			}
		}
		statement->errcode = 0; /* retain backwards compat with OCI8 1.4 */
	}

	return 0;
}
/* }}} */

/* {{{ crx_oci_statement_cancel()
 Cancel statement */
int crx_oci_statement_cancel(crx_oci_statement *statement)
{
	return crx_oci_statement_fetch(statement, 0);
}
/* }}} */

/* {{{ crx_oci_statement_free()
 Destroy statement handle and free associated resources */
void crx_oci_statement_free(crx_oci_statement *statement)
{
	if (statement->stmt) {
		if (statement->last_query_len) { /* FIXME: magical */
			CRX_OCI_CALL(OCIStmtRelease, (statement->stmt, statement->err, NULL, 0, statement->errcode ? OCI_STRLS_CACHE_DELETE : OCI_DEFAULT));
		} else if (statement->impres_flag != CRX_OCI_IMPRES_IS_CHILD) {  /* Oracle doc says don't free Implicit Result Set handles */
			CRX_OCI_CALL(OCIHandleFree, (statement->stmt, OCI_HTYPE_STMT));
		}
		statement->stmt = NULL;
	}

	if (statement->err) {
		CRX_OCI_CALL(OCIHandleFree, (statement->err, OCI_HTYPE_ERROR));
		statement->err = NULL;
	}

	if (statement->last_query) {
		efree(statement->last_query);
	}

	if (statement->binds) {
		crex_hash_destroy(statement->binds);
		efree(statement->binds);
	}

	if (statement->defines) {
		crex_hash_destroy(statement->defines);
		efree(statement->defines);
	}

	if (statement->columns) {
		crex_hash_destroy(statement->columns);
		efree(statement->columns);
	}

	if (statement->parent_stmtid) {
		crex_list_delete(statement->parent_stmtid);
	}

	crex_list_delete(statement->connection->id);
	efree(statement);

	OCI_G(num_statements)--;
}
/* }}} */

/* {{{ crx_oci_bind_pre_exec()
 Helper function */
int crx_oci_bind_pre_exec(zval *data, void *result)
{
	crx_oci_bind *bind = (crx_oci_bind *) C_PTR_P(data);
	zval *zv = &bind->val;

	*(int *)result = 0;

	ZVAL_DEREF(zv);
	if (C_TYPE_P(zv) == IS_ARRAY) {
		/* These checks are currently valid for oci_bind_by_name, not
		 * oci_bind_array_by_name.  Also bind->type and
		 * bind->indicator are not used for oci_bind_array_by_name.
		 */
		return 0;
	}
	switch (bind->type) {
		case SQLT_NTY:
		case SQLT_BFILEE:
		case SQLT_CFILEE:
		case SQLT_CLOB:
		case SQLT_BLOB:
		case SQLT_RDD:
			if (C_TYPE_P(zv) != IS_OBJECT) {
				crx_error_docref(NULL, E_WARNING, "Invalid variable used for bind");
				*(int *)result = 1;
			}
			break;

		case SQLT_CHR:
		case SQLT_AFC:
		case SQLT_INT:
		case SQLT_NUM:
#if defined(OCI_MAJOR_VERSION) && OCI_MAJOR_VERSION >= 12
		case SQLT_BOL:
#endif
		case SQLT_LBI:
		case SQLT_BIN:
		case SQLT_LNG:
			if (C_TYPE_P(zv) == IS_RESOURCE || C_TYPE_P(zv) == IS_OBJECT) {
				crx_error_docref(NULL, E_WARNING, "Invalid variable used for bind");
				*(int *)result = 1;
			}
			break;

		case SQLT_RSET:
			if (C_TYPE_P(zv) != IS_RESOURCE) {
				crx_error_docref(NULL, E_WARNING, "Invalid variable used for bind");
				*(int *)result = 1;
			}
			break;
	}

	/* reset all bind stuff to a normal state... */
	bind->indicator = 0;

	return 0;
}
/* }}} */

/* {{{ crx_oci_bind_post_exec()
 Helper function */
int crx_oci_bind_post_exec(zval *data)
{
	crx_oci_bind *bind = (crx_oci_bind *) C_PTR_P(data);
	crx_oci_connection *connection = bind->parent_statement->connection;
	sword errstatus;
	zval *zv = &bind->val;

	ZVAL_DEREF(zv);
	if (bind->indicator == -1) { /* NULL */
		if (C_TYPE_P(zv) == IS_STRING) {
			*C_STRVAL_P(zv) = '\0'; /* XXX avoid warning in debug mode */
		}
		zval_ptr_dtor(zv);
		ZVAL_NULL(zv);
	} else if (C_TYPE_P(zv) == IS_STRING
			   && C_STRLEN_P(zv) > 0
			   && C_STRVAL_P(zv)[ C_STRLEN_P(zv) ] != '\0') {
		/* The post- CRX 5.3 feature for "interned" strings disallows
		 * their reallocation but (i) any IN binds either interned or
		 * not should already be null terminated and (ii) for OUT
		 * binds, crx_oci_bind_out_callback() should have allocated a
		 * new string that we can modify here.
		 */
		ZVAL_NEW_STR(zv, crex_string_extend(C_STR_P(zv), C_STRLEN_P(zv)+1, 0));
		C_STRVAL_P(zv)[ C_STRLEN_P(zv) ] = '\0';
	} else if (C_TYPE_P(zv) == IS_ARRAY) {
		int i;
		zval *entry = NULL;
		HashTable *hash;

		SEPARATE_ARRAY(zv);
		hash = C_ARRVAL_P(zv);
		crex_hash_internal_pointer_reset(hash);

		switch (bind->array.type) {
			case SQLT_NUM:
			case SQLT_INT:
			case SQLT_LNG:
				for (i = 0; i < (int) bind->array.current_length; i++) {
					if ((i < (int) bind->array.old_length) && (entry = crex_hash_get_current_data(hash)) != NULL) {
						zval_ptr_dtor(entry);
						ZVAL_LONG(entry, ((oci_crxsized_int *)(bind->array.elements))[i]);
						crex_hash_move_forward(hash);
					} else {
						add_next_index_long(zv, ((oci_crxsized_int *)(bind->array.elements))[i]);
					}
				}
				break;
			case SQLT_FLT:
				for (i = 0; i < (int) bind->array.current_length; i++) {
					if ((i < (int) bind->array.old_length) && (entry = crex_hash_get_current_data(hash)) != NULL) {
						zval_ptr_dtor(entry);
						ZVAL_DOUBLE(entry, ((double *)(bind->array.elements))[i]);
						crex_hash_move_forward(hash);
					} else {
						add_next_index_double(zv, ((double *)(bind->array.elements))[i]);
					}
				}
				break;
			case SQLT_ODT:
				for (i = 0; i < (int) bind->array.current_length; i++) {
					oratext buff[1024];
					ub4 buff_len = 1024;

					memset((void*)buff,0,sizeof(buff));

					if ((i < (int) bind->array.old_length) && (entry = crex_hash_get_current_data(hash)) != NULL) {
						CRX_OCI_CALL_RETURN(errstatus, OCIDateToText, (connection->err, &(((OCIDate *)(bind->array.elements))[i]), 0, 0, 0, 0, &buff_len, buff));
						zval_ptr_dtor(entry);

						if (errstatus != OCI_SUCCESS) {
							connection->errcode = crx_oci_error(connection->err, errstatus);
							CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
							ZVAL_NULL(entry);
						} else {
							connection->errcode = 0; /* retain backwards compat with OCI8 1.4 */
							ZVAL_STRINGL(entry, (char *)buff, buff_len);
						}
						crex_hash_move_forward(hash);
					} else {
						CRX_OCI_CALL_RETURN(errstatus, OCIDateToText, (connection->err, &(((OCIDate *)(bind->array.elements))[i]), 0, 0, 0, 0, &buff_len, buff));
						if (errstatus != OCI_SUCCESS) {
							connection->errcode = crx_oci_error(connection->err, errstatus);
							CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
							add_next_index_null(zv);
						} else {
							connection->errcode = 0; /* retain backwards compat with OCI8 1.4 */
							add_next_index_stringl(zv, (char *)buff, buff_len);
						}
					}
				}
				break;

			case SQLT_AFC:
			case SQLT_CHR:
			case SQLT_VCS:
			case SQLT_AVC:
			case SQLT_STR:
			case SQLT_LVC:
				for (i = 0; i < (int) bind->array.current_length; i++) {
					/* int curr_element_length = strlen(((text *)bind->array.elements)+i*bind->array.max_length); */
					int curr_element_length = bind->array.element_lengths[i];
					if ((i < (int) bind->array.old_length) && (entry = crex_hash_get_current_data(hash)) != NULL) {
						zval_ptr_dtor(entry);
						ZVAL_STRINGL(entry, (char *)(((text *)bind->array.elements)+i*bind->array.max_length), curr_element_length);
						crex_hash_move_forward(hash);
					} else {
						add_next_index_stringl(zv, (char *)(((text *)bind->array.elements)+i*bind->array.max_length), curr_element_length);
					}
				}
				break;
		}
	} else if ((C_TYPE_P(zv) == IS_TRUE) || (C_TYPE_P(zv) == IS_FALSE)) {
		/* This convetrsion is done on purpose (ext/oci8 uses LVAL as a temporary value) */
		if (C_LVAL_P(zv) == 0)
			ZVAL_FALSE(zv);
		else if (C_LVAL_P(zv) == 1)
			ZVAL_TRUE(zv);
	}

	return 0;
}
/* }}} */

/* {{{ crx_oci_bind_by_name()
 Bind zval to the given placeholder */
int crx_oci_bind_by_name(crx_oci_statement *statement, char *name, size_t name_len, zval *var, crex_long maxlength, ub2 type)
{
	crx_oci_collection *bind_collection = NULL;
	crx_oci_descriptor *bind_descriptor = NULL;
	crx_oci_statement  *bind_statement	= NULL;
	dvoid *oci_desc					= NULL;
	/* dvoid *crx_oci_collection		   = NULL; */
	OCIStmt *oci_stmt				= NULL;
	dvoid *bind_data				= NULL;
	crx_oci_bind *old_bind, *bindp;
	int mode = OCI_DATA_AT_EXEC;
	sb4 value_sz = -1;
	sword errstatus;
	zval *param = NULL;

	CREX_ASSERT(C_ISREF_P(var));
	param = C_REFVAL_P(var);

	switch (type) {
		case SQLT_NTY:
		{
			zval *tmp;

			if (C_TYPE_P(param) != IS_OBJECT || (tmp = crex_hash_str_find(C_OBJPROP_P(param), "collection", sizeof("collection")-1)) == NULL) {
				crx_error_docref(NULL, E_WARNING, "Unable to find collection property");
				return 1;
			}

			CRX_OCI_ZVAL_TO_COLLECTION_EX(tmp, bind_collection);
			value_sz = sizeof(void*);
			mode = OCI_DEFAULT;

			if (!bind_collection->collection) {
				return 1;
			}
		}
			break;
		case SQLT_BFILEE:
		case SQLT_CFILEE:
		case SQLT_CLOB:
		case SQLT_BLOB:
		case SQLT_RDD:
		{
			zval *tmp;

			if (C_TYPE_P(param) != IS_OBJECT || (tmp = crex_hash_str_find(C_OBJPROP_P(param), "descriptor", sizeof("descriptor")-1)) == NULL) {
				crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property");
				return 1;
			}

			CRX_OCI_ZVAL_TO_DESCRIPTOR_EX(tmp, bind_descriptor);

			value_sz = sizeof(void*);

			oci_desc = bind_descriptor->descriptor;

			if (!oci_desc) {
				return 1;
			}
		}
			break;

		case SQLT_INT:
		case SQLT_NUM:
			if (C_TYPE_P(param) == IS_RESOURCE || C_TYPE_P(param) == IS_OBJECT) {
				crx_error_docref(NULL, E_WARNING, "Invalid variable used for bind");
				return 1;
			}
			convert_to_long(param);
			bind_data = (oci_crxsized_int *)&C_LVAL_P(param);
			value_sz = sizeof(oci_crxsized_int);
			mode = OCI_DEFAULT;
			break;

		case SQLT_LBI:
		case SQLT_BIN:
		case SQLT_LNG:
		case SQLT_AFC:
		case SQLT_CHR: /* SQLT_CHR is the default value when type was not specified */
			if (C_TYPE_P(param) == IS_RESOURCE || C_TYPE_P(param) == IS_OBJECT) {
				crx_error_docref(NULL, E_WARNING, "Invalid variable used for bind");
				return 1;
			}
			if (C_TYPE_P(param) != IS_NULL) {
				if (!try_convert_to_string(param)) {
					return 1;
				}
			}
			if ((maxlength == -1) || (maxlength == 0)) {
				if (type == SQLT_LNG) {
					value_sz = SB4MAXVAL;
				} else if (C_TYPE_P(param) == IS_STRING) {
					value_sz = (sb4) C_STRLEN_P(param);
				} else {
					/* Bug-72524: revert value_sz from CRX_OCI_PIECE_SIZE to 0. This restores CRX 5.6 behavior  */
					value_sz = 0;
				}
			} else {
				value_sz = (sb4) maxlength;
			}
			break;

		case SQLT_RSET:
			if (C_TYPE_P(param) != IS_RESOURCE) {
				crx_error_docref(NULL, E_WARNING, "Invalid variable used for bind");
				return 1;
			}
			CRX_OCI_ZVAL_TO_STATEMENT_EX(param, bind_statement);
			value_sz = sizeof(void*);

			oci_stmt = bind_statement->stmt;

			if (!oci_stmt) {
				return 1;
			}
			break;

#if defined(OCI_MAJOR_VERSION) && OCI_MAJOR_VERSION >= 12
		case SQLT_BOL:
			if (C_TYPE_P(param) == IS_RESOURCE || C_TYPE_P(param) == IS_OBJECT) {
				crx_error_docref(NULL, E_WARNING, "Invalid variable used for bind");
				return 1;
			}
			convert_to_boolean(param);
			bind_data = (crex_long *)&C_LVAL_P(param);
			if (C_TYPE_P(param) == IS_TRUE)
				*(crex_long *)bind_data = 1;
			else if (C_TYPE_P(param) == IS_FALSE)
				*(crex_long *)bind_data = 0;
			else {
				crx_error_docref(NULL, E_WARNING, "Invalid variable used for bind");
				return 1;
			}

			value_sz = sizeof(crex_long);

			mode = OCI_DEFAULT;
			break;
#endif

		default:
			crx_error_docref(NULL, E_WARNING, "Unknown or unsupported datatype given: %d", (int)type);
			return 1;
			break;
	}

	if (!statement->binds) {
		ALLOC_HASHTABLE(statement->binds);
		crex_hash_init(statement->binds, 13, NULL, crx_oci_bind_hash_dtor, 0);
	}

	if ((old_bind = crex_hash_str_find_ptr(statement->binds, name, name_len)) != NULL) {
		bindp = old_bind;
		if (!C_ISUNDEF(bindp->val)) {
			zval_ptr_dtor(&bindp->val);
			ZVAL_UNDEF(&bindp->val);
		}
	} else {
		crex_string *zvtmp;
		zvtmp = crex_string_init(name, name_len, 0);
		bindp = (crx_oci_bind *) ecalloc(1, sizeof(crx_oci_bind));
		bindp = crex_hash_update_ptr(statement->binds, zvtmp, bindp);
		crex_string_release_ex(zvtmp, 0);
	}

	/* Make sure the minimum of value_sz is 1 to avoid ORA-3149
	 * when both in/out parameters are bound with empty strings
	 */
	if (value_sz == 0)
		value_sz = 1;

	bindp->descriptor = oci_desc;
	bindp->statement = oci_stmt;
	bindp->parent_statement = statement;
	ZVAL_COPY(&bindp->val, var);
	bindp->type = type;
	/* Storing max length set in OCIBindByName() to check it later in
	 * crx_oci_bind_in_callback() function to avoid ORA-1406 error while
	 * executing OCIStmtExecute()
     */
	bindp->dummy_len = value_sz;

	CRX_OCI_CALL_RETURN(errstatus,
		OCIBindByName,
		(
			statement->stmt,				 /* statement handle */
			(OCIBind **)&bindp->bind,		 /* bind hdl (will alloc) */
			statement->err,				  	 /* error handle */
			(text*) name,					 /* placeholder name */
			(sb4) name_len,					 /* placeholder length */
			(dvoid *)bind_data,				 /* in/out data */
			value_sz, /* CRX_OCI_MAX_DATA_SIZE, */ /* max size of input/output data */
			type,							 /* in/out data type */
			(dvoid *)&bindp->indicator,		 /* indicator (ignored) */
			(ub2 *)0,						 /* size array (ignored) */
			(ub2 *)&bindp->retcode,			 /* return code (ignored) */
			(ub4)0,							 /* maxarr_len (PL/SQL only?) */
			(ub4 *)0,						 /* actual array size (PL/SQL only?) */
			mode							 /* mode */
		)
	);

	if (errstatus != OCI_SUCCESS) {
		statement->errcode = crx_oci_error(statement->err, errstatus);
		CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
		return 1;
	}

	if (mode == OCI_DATA_AT_EXEC) {
		CRX_OCI_CALL_RETURN(errstatus, OCIBindDynamic,
				(
				 bindp->bind,
				 statement->err,
				 (dvoid *)bindp,
				 crx_oci_bind_in_callback,
				 (dvoid *)bindp,
				 crx_oci_bind_out_callback
				)
		);

		if (errstatus != OCI_SUCCESS) {
			statement->errcode = crx_oci_error(statement->err, errstatus);
			CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
			return 1;
		}
	}

	if (type == SQLT_NTY) {
		/* Bind object */
		CRX_OCI_CALL_RETURN(errstatus, OCIBindObject,
				(
				 bindp->bind,
				 statement->err,
				 bind_collection->tdo,
				 (dvoid **) &(bind_collection->collection),
				 (ub4 *) 0,
				 (dvoid **) 0,
				 (ub4 *) 0
				)
		);

		if (errstatus) {
			statement->errcode = crx_oci_error(statement->err, errstatus);
			CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
			return 1;
		}
	}

	statement->errcode = 0; /* retain backwards compat with OCI8 1.4 */
	return 0;
}
/* }}} */

/* {{{ crx_oci_bind_in_callback()
 Callback used when binding LOBs and VARCHARs */
sb4 crx_oci_bind_in_callback(
					dvoid *ictxp,	  /* context pointer */
					OCIBind *bindp,	  /* bind handle */
					ub4 iter,		  /* 0-based execute iteration value */
					ub4 index,		  /* index of current array for PL/SQL or row index for SQL */
					dvoid **bufpp,	  /* pointer to data */
					ub4 *alenp,		  /* size after value/piece has been read */
					ub1 *piecep,	  /* which piece */
					dvoid **indpp)	  /* indicator value */
{
	crx_oci_bind *crxbind;
	zval *val;

	if (!(crxbind=(crx_oci_bind *)ictxp) || C_ISUNDEF(crxbind->val)) {
		crx_error_docref(NULL, E_WARNING, "Invalid crxbind pointer value");
		return OCI_ERROR;
	}

	val = &crxbind->val;
	ZVAL_DEREF(val);

	if (C_ISNULL_P(val)) {
		/* we're going to insert a NULL column */
		crxbind->indicator = -1;
		*bufpp = 0;
		*alenp = -1;
		*indpp = (dvoid *)&crxbind->indicator;
	} else	if ((crxbind->descriptor == 0) && (crxbind->statement == 0)) {
		/* "normal" string bind */
		if (!try_convert_to_string(val)) {
			return OCI_ERROR;
		}

		*bufpp = C_STRVAL_P(val);
		*alenp = (ub4) C_STRLEN_P(val);
		/*
		 * bind_char_1: If max length set in OCIBindByName is less than the
		 * actual length of input string, then we have to overwrite alenp with
		 * max value set in OCIBindByName (dummy_len). Or else it will cause
		 * ORA-1406 error in OCIStmtExecute
		 */
		if ((crxbind->dummy_len > 0) && (crxbind->dummy_len < *alenp))
			*alenp = crxbind->dummy_len;
		*indpp = (dvoid *)&crxbind->indicator;
	} else if (crxbind->statement != 0) {
		/* RSET */
		*bufpp = crxbind->statement;
		*alenp = -1;		/* seems to be alright */
		*indpp = (dvoid *)&crxbind->indicator;
	} else {
		/* descriptor bind */
		*bufpp = crxbind->descriptor;
		*alenp = -1;		/* seems to be alright */
		*indpp = (dvoid *)&crxbind->indicator;
	}

	*piecep = OCI_ONE_PIECE; /* pass all data in one go */

	return OCI_CONTINUE;
}
/* }}} */

/* {{{ crx_oci_bind_out_callback()
 Callback used when binding LOBs and VARCHARs */
sb4 crx_oci_bind_out_callback(
					dvoid *octxp,	   /* context pointer */
					OCIBind *bindp,	   /* bind handle */
					ub4 iter,		   /* 0-based execute iteration value */
					ub4 index,		   /* index of current array for PL/SQL or row index for SQL */
					dvoid **bufpp,	   /* pointer to data */
					ub4 **alenpp,	   /* size after value/piece has been read */
					ub1 *piecep,	   /* which piece */
					dvoid **indpp,	   /* indicator value */
					ub2 **rcodepp)	   /* return code */
{
	crx_oci_bind *crxbind;
	zval *val;
	sb4 retval = OCI_ERROR;

	if (!(crxbind=(crx_oci_bind *)octxp) || C_ISUNDEF(crxbind->val)) {
		crx_error_docref(NULL, E_WARNING, "Invalid crxbind pointer value");
		return retval;
	}

	val = &crxbind->val;
	ZVAL_DEREF(val);

	if (C_TYPE_P(val) == IS_RESOURCE) {
		/* Processing for ref-cursor out binds */
		if (crxbind->statement != NULL) {
			*bufpp = crxbind->statement;
			*alenpp = &crxbind->dummy_len;
			*piecep = OCI_ONE_PIECE;
			*rcodepp = &crxbind->retcode;
			*indpp = &crxbind->indicator;
		}
		retval = OCI_CONTINUE;
	} else if (C_TYPE_P(val) == IS_OBJECT) {
		zval *tmp;
		crx_oci_descriptor *desc;

		if (!crxbind->descriptor) {
			return OCI_ERROR;
		}

		/* Do not use the cached lob size if the descriptor is an
		 * out-bind as the contents would have been changed for in/out
		 * binds (Bug #46994).
		 */
		if ((tmp = crex_hash_str_find(C_OBJPROP_P(val), "descriptor", sizeof("descriptor")-1)) == NULL) {
			crx_error_docref(NULL, E_WARNING, "Unable to find object outbind descriptor property");
			return OCI_ERROR;
		}
		CRX_OCI_ZVAL_TO_DESCRIPTOR_EX(tmp, desc);
		desc->lob_size = -1;	/* force OCI8 to update cached size */

		*alenpp = &crxbind->dummy_len;
		*bufpp = crxbind->descriptor;
		*piecep = OCI_ONE_PIECE;
		*rcodepp = &crxbind->retcode;
		*indpp = &crxbind->indicator;
		retval = OCI_CONTINUE;
	} else {
		zval_ptr_dtor(val);

		{
			char *p = ecalloc(1, CRX_OCI_PIECE_SIZE);
			ZVAL_STRINGL(val, p, CRX_OCI_PIECE_SIZE);
			efree(p);
		}

		/* XXX we assume that crex-zval len has 4 bytes */
		*alenpp = (ub4*) &C_STRLEN_P(val);
		*bufpp = C_STRVAL_P(val);
		*piecep = OCI_ONE_PIECE;
		*rcodepp = &crxbind->retcode;
		*indpp = &crxbind->indicator;
		retval = OCI_CONTINUE;
	}

	return retval;
}
/* }}} */

/* {{{ crx_oci_statement_get_column_helper()
 Helper function to get column by name and index */
crx_oci_out_column *crx_oci_statement_get_column_helper(INTERNAL_FUNCTION_PARAMETERS, int need_data)
{
	zval *z_statement;
	crex_string *column_index_string;
	crex_long column_index_int;

	crx_oci_statement *statement;
	crx_oci_out_column *column;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_RESOURCE(z_statement)
		C_PARAM_STR_OR_LONG(column_index_string, column_index_int)
	CREX_PARSE_PARAMETERS_END_EX(return NULL);

	statement = (crx_oci_statement *) crex_fetch_resource_ex(z_statement, "oci8 statement", le_statement);

	if (!statement) {
		return NULL;
	}

	if (need_data && !statement->has_data) {
		return NULL;
	}

	if (column_index_string != NULL) {
		column = crx_oci_statement_get_column(statement, -1, ZSTR_VAL(column_index_string), (int) ZSTR_LEN(column_index_string));
		if (!column) {
			crx_error_docref(NULL, E_WARNING, "Invalid column name \"%s\"", ZSTR_VAL(column_index_string));
			return NULL;
		}
	} else {
		column = crx_oci_statement_get_column(statement, column_index_int, NULL, 0);

		if (!column) {
			crx_error_docref(NULL, E_WARNING, "Invalid column index \"" CREX_LONG_FMT "\"", column_index_int);
			return NULL;
		}
	}
	return column;
}
/* }}} */

/* {{{ crx_oci_statement_get_type()
 Return type of the statement */
int crx_oci_statement_get_type(crx_oci_statement *statement, ub2 *type)
{
	ub2 statement_type;
	sword errstatus;

	*type = 0;

	CRX_OCI_CALL_RETURN(errstatus, OCIAttrGet, ((dvoid *)statement->stmt, OCI_HTYPE_STMT, (ub2 *)&statement_type, (ub4 *)0, OCI_ATTR_STMT_TYPE, statement->err));

	if (errstatus != OCI_SUCCESS) {
		statement->errcode = crx_oci_error(statement->err, errstatus);
		CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
		return 1;
	}
	statement->errcode = 0; /* retain backwards compat with OCI8 1.4 */
	*type = statement_type;

	return 0;
}
/* }}} */

/* {{{ crx_oci_statement_get_numrows()
 Get the number of rows fetched to the clientside (NOT the number of rows in the result set) */
int crx_oci_statement_get_numrows(crx_oci_statement *statement, ub4 *numrows)
{
	ub4 statement_numrows;
	sword errstatus;

	*numrows = 0;

	CRX_OCI_CALL_RETURN(errstatus, OCIAttrGet, ((dvoid *)statement->stmt, OCI_HTYPE_STMT, (ub4 *)&statement_numrows, (ub4 *)0, OCI_ATTR_ROW_COUNT, statement->err));

	if (errstatus != OCI_SUCCESS) {
		statement->errcode = crx_oci_error(statement->err, errstatus);
		CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
		return 1;
	}
	statement->errcode = 0; /* retain backwards compat with OCI8 1.4 */
	*numrows = statement_numrows;

	return 0;
}
/* }}} */

/* {{{ crx_oci_bind_array_by_name()
 Bind arrays to PL/SQL types */
int crx_oci_bind_array_by_name(crx_oci_statement *statement, char *name, size_t name_len, zval *var, crex_long max_table_length, crex_long maxlength, crex_long type)
{
	crx_oci_bind *bind;
	sword errstatus;
	crex_string *zvtmp;
	zval *val;

	CREX_ASSERT(C_ISREF_P(var));
	val = C_REFVAL_P(var);
	convert_to_array(val);
	SEPARATE_ARRAY(val);

	if (maxlength < -1) {
		crx_error_docref(NULL, E_WARNING, "Invalid max length value (" CREX_LONG_FMT ")", maxlength);
		return 1;
	}

	switch(type) {
		case SQLT_NUM:
		case SQLT_INT:
		case SQLT_LNG:
			bind = crx_oci_bind_array_helper_number(val, max_table_length);
			break;

		case SQLT_FLT:
			bind = crx_oci_bind_array_helper_double(val, max_table_length);
			break;

		case SQLT_AFC:
		case SQLT_CHR:
		case SQLT_VCS:
		case SQLT_AVC:
		case SQLT_STR:
		case SQLT_LVC:
			if (maxlength == -1 && crex_hash_num_elements(C_ARRVAL_P(val)) == 0) {
				crx_error_docref(NULL, E_WARNING, "You must provide max length value for empty arrays");
				return 1;
			}
			bind = crx_oci_bind_array_helper_string(val, max_table_length, maxlength);
			break;
		case SQLT_ODT:
			bind = crx_oci_bind_array_helper_date(val, max_table_length, statement->connection);
			break;
		default:
			crx_error_docref(NULL, E_WARNING, "Unknown or unsupported datatype given: " CREX_LONG_FMT, type);
			return 1;
			break;
	}

	if (bind == NULL) {
		/* failed to generate bind struct */
		return 1;
	}

	bind->descriptor = NULL;
	bind->statement = NULL;
	bind->parent_statement = statement;
	bind->bind = NULL;
	ZVAL_COPY(&bind->val, var);
	bind->array.type = type;
	bind->indicator = 0;  		/* not used for array binds */
	bind->type = 0; 			/* not used for array binds */

	CRX_OCI_CALL_RETURN(errstatus,
							OCIBindByName,
							(
								statement->stmt,
								(OCIBind **)&bind->bind,
								statement->err,
								(text *)name,
								(sb4) name_len,
								(dvoid *) bind->array.elements,
								(sb4) bind->array.max_length,
								(ub2)type,
								(dvoid *)bind->array.indicators,
								(ub2 *)bind->array.element_lengths,
								(ub2 *)0, /* bindp->array.retcodes, */
								(ub4) max_table_length,
								(ub4 *) &(bind->array.current_length),
								(ub4) OCI_DEFAULT
							)
						);


	if (errstatus != OCI_SUCCESS) {
		if (bind->array.elements) {
			efree(bind->array.elements);
		}

		if (bind->array.element_lengths) {
			efree(bind->array.element_lengths);
		}

		if (bind->array.indicators) {
			efree(bind->array.indicators);
		}

		zval_ptr_dtor(&bind->val);

		efree(bind);

		statement->errcode = crx_oci_error(statement->err, errstatus);
		CRX_OCI_HANDLE_ERROR(statement->connection, statement->errcode);
		return 1;
	}

	if (!statement->binds) {
		ALLOC_HASHTABLE(statement->binds);
		crex_hash_init(statement->binds, 13, NULL, crx_oci_bind_hash_dtor, 0);
	}

	zvtmp = crex_string_init(name, name_len, 0);
	crex_hash_update_ptr(statement->binds, zvtmp, bind);
	crex_string_release_ex(zvtmp, 0);

	statement->errcode = 0; /* retain backwards compat with OCI8 1.4 */
	return 0;
}
/* }}} */

/* {{{ crx_oci_bind_array_helper_string()
 Bind arrays to PL/SQL types */
crx_oci_bind *crx_oci_bind_array_helper_string(zval *var, crex_long max_table_length, crex_long maxlength)
{
	crx_oci_bind *bind;
	ub4 i;
	HashTable *hash;
	zval *entry;

	SEPARATE_ARRAY(var); /* TODO: may be use new HashTable iteration and prevent inplace modification */
	hash = C_ARRVAL_P(var);

	if (maxlength == -1) {
		crex_hash_internal_pointer_reset(hash);
		while ((entry = crex_hash_get_current_data(hash)) != NULL) {
			if (!try_convert_to_string(entry)) {
				return NULL;
			}

			if (maxlength == -1 || C_STRLEN_P(entry) > (size_t) maxlength) {
				maxlength = C_STRLEN_P(entry) + 1;
			}

			crex_hash_move_forward(hash);
		}
	}

	bind = emalloc(sizeof(crx_oci_bind));
	ZVAL_UNDEF(&bind->val);
	bind->array.elements		= (text *)safe_emalloc(max_table_length * (maxlength + 1), sizeof(text), 0);
	memset(bind->array.elements, 0, max_table_length * (maxlength + 1) * sizeof(text));
	bind->array.current_length	= crex_hash_num_elements(C_ARRVAL_P(var));
	bind->array.old_length		= bind->array.current_length;
	bind->array.max_length		= (ub4) maxlength;
	bind->array.element_lengths	= safe_emalloc(max_table_length, sizeof(ub2), 0);
	memset(bind->array.element_lengths, 0, max_table_length*sizeof(ub2));
	bind->array.indicators		= safe_emalloc(max_table_length, sizeof(sb2), 0);
	memset(bind->array.indicators, 0, max_table_length*sizeof(sb2));

	crex_hash_internal_pointer_reset(hash);

	for (i = 0; i < bind->array.current_length; i++) {
		if ((entry = crex_hash_get_current_data(hash)) != NULL) {
			if (!try_convert_to_string(entry)) {
				efree(bind->array.elements);
				efree(bind->array.element_lengths);
				efree(bind->array.indicators);
				efree(bind);
				return NULL;
			}

			bind->array.element_lengths[i] = (ub2) C_STRLEN_P(entry);
			if (C_STRLEN_P(entry) == 0) {
				bind->array.indicators[i] = -1;
			}
			crex_hash_move_forward(hash);
		} else {
			break;
		}
	}

	crex_hash_internal_pointer_reset(hash);
	for (i = 0; i < max_table_length; i++) {
		if ((i < bind->array.current_length) && (entry = crex_hash_get_current_data(hash)) != NULL) {
			int element_length;
			if (!try_convert_to_string(entry)) {
				efree(bind->array.elements);
				efree(bind->array.element_lengths);
				efree(bind->array.indicators);
				efree(bind);
				return NULL;
			}

			element_length = ((size_t) maxlength > C_STRLEN_P(entry)) ? (int) C_STRLEN_P(entry) : (int) maxlength;

			memcpy((text *)bind->array.elements + i*maxlength, C_STRVAL_P(entry), element_length);
			((text *)bind->array.elements)[i*maxlength + element_length] = '\0';

			crex_hash_move_forward(hash);
		} else {
			((text *)bind->array.elements)[i*maxlength] = '\0';
		}
	}
	crex_hash_internal_pointer_reset(hash);

	return bind;
}
/* }}} */

/* {{{ crx_oci_bind_array_helper_number()
 Bind arrays to PL/SQL types */
crx_oci_bind *crx_oci_bind_array_helper_number(zval *var, crex_long max_table_length)
{
	crx_oci_bind *bind;
	ub4 i;
	HashTable *hash;
	zval *entry;

	SEPARATE_ARRAY(var); /* TODO: may be use new HashTable iteration and prevent inplace modification */
	hash = C_ARRVAL_P(var);

	bind = emalloc(sizeof(crx_oci_bind));
	ZVAL_UNDEF(&bind->val);
	bind->array.elements		= (oci_crxsized_int *)safe_emalloc(max_table_length, sizeof(oci_crxsized_int), 0);
	bind->array.current_length	= crex_hash_num_elements(C_ARRVAL_P(var));
	bind->array.old_length		= bind->array.current_length;
	bind->array.max_length		= sizeof(oci_crxsized_int);
	bind->array.element_lengths	= safe_emalloc(max_table_length, sizeof(ub2), 0);
	memset(bind->array.element_lengths, 0, max_table_length * sizeof(ub2));
	bind->array.indicators		= NULL;

	crex_hash_internal_pointer_reset(hash);
	for (i = 0; i < max_table_length; i++) {
		if (i < bind->array.current_length) {
			bind->array.element_lengths[i] = sizeof(oci_crxsized_int);
		}
		if ((i < bind->array.current_length) && (entry = crex_hash_get_current_data(hash)) != NULL) {
			convert_to_long(entry);
			((oci_crxsized_int *)bind->array.elements)[i] = (oci_crxsized_int) C_LVAL_P(entry);
			crex_hash_move_forward(hash);
		} else {
			((oci_crxsized_int *)bind->array.elements)[i] = 0;
		}
	}
	crex_hash_internal_pointer_reset(hash);

	return bind;
}
/* }}} */

/* {{{ crx_oci_bind_array_helper_double()
 Bind arrays to PL/SQL types */
crx_oci_bind *crx_oci_bind_array_helper_double(zval *var, crex_long max_table_length)
{
	crx_oci_bind *bind;
	ub4 i;
	HashTable *hash;
	zval *entry;

	SEPARATE_ARRAY(var); /* TODO: may be use new HashTable iteration and prevent inplace modification */
	hash = C_ARRVAL_P(var);

	bind = emalloc(sizeof(crx_oci_bind));
	ZVAL_UNDEF(&bind->val);
	bind->array.elements		= (double *)safe_emalloc(max_table_length, sizeof(double), 0);
	bind->array.current_length	= crex_hash_num_elements(C_ARRVAL_P(var));
	bind->array.old_length		= bind->array.current_length;
	bind->array.max_length		= sizeof(double);
	bind->array.element_lengths	= safe_emalloc(max_table_length, sizeof(ub2), 0);
	memset(bind->array.element_lengths, 0, max_table_length * sizeof(ub2));
	bind->array.indicators		= NULL;

	crex_hash_internal_pointer_reset(hash);
	for (i = 0; i < max_table_length; i++) {
		if (i < bind->array.current_length) {
			bind->array.element_lengths[i] = sizeof(double);
		}
		if ((i < bind->array.current_length) && (entry = crex_hash_get_current_data(hash)) != NULL) {
			convert_to_double(entry);
			((double *)bind->array.elements)[i] = (double) C_DVAL_P(entry);
			crex_hash_move_forward(hash);
		} else {
			((double *)bind->array.elements)[i] = 0;
		}
	}
	crex_hash_internal_pointer_reset(hash);

	return bind;
}
/* }}} */

/* {{{ crx_oci_bind_array_helper_date()
 Bind arrays to PL/SQL types */
crx_oci_bind *crx_oci_bind_array_helper_date(zval *var, crex_long max_table_length, crx_oci_connection *connection)
{
	crx_oci_bind *bind;
	ub4 i;
	HashTable *hash;
	zval *entry;
	sword errstatus;

	SEPARATE_ARRAY(var); /* TODO: may be use new HashTable iteration and prevent inplace modification */
	hash = C_ARRVAL_P(var);

	bind = emalloc(sizeof(crx_oci_bind));
	ZVAL_UNDEF(&bind->val);
	bind->array.elements		= (OCIDate *)safe_emalloc(max_table_length, sizeof(OCIDate), 0);
	bind->array.current_length	= crex_hash_num_elements(C_ARRVAL_P(var));
	bind->array.old_length		= bind->array.current_length;
	bind->array.max_length		= sizeof(OCIDate);
	bind->array.element_lengths	= safe_emalloc(max_table_length, sizeof(ub2), 0);
	memset(bind->array.element_lengths, 0, max_table_length * sizeof(ub2));
	bind->array.indicators		= NULL;

	crex_hash_internal_pointer_reset(hash);
	for (i = 0; i < max_table_length; i++) {
		OCIDate oci_date;
		if (i < bind->array.current_length) {
			bind->array.element_lengths[i] = sizeof(OCIDate);
		}
		if ((i < bind->array.current_length) && (entry = crex_hash_get_current_data(hash)) != NULL) {
			crex_string *entry_str = zval_try_get_string(entry);
			if (UNEXPECTED(!entry_str)) {
				efree(bind->array.element_lengths);
				efree(bind->array.elements);
				efree(bind);
				return NULL;
			}

			CRX_OCI_CALL_RETURN(errstatus, OCIDateFromText, (connection->err, (CONST text *)ZSTR_VAL(entry_str), (ub4) ZSTR_LEN(entry_str), NULL, 0, NULL, 0, &oci_date));
			crex_string_release(entry_str);

			if (errstatus != OCI_SUCCESS) {
				/* failed to convert string to date */
				efree(bind->array.element_lengths);
				efree(bind->array.elements);
				efree(bind);
				connection->errcode = crx_oci_error(connection->err, errstatus);
				CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
				return NULL;
			}

			((OCIDate *)bind->array.elements)[i] = oci_date;
			crex_hash_move_forward(hash);
		} else {
			CRX_OCI_CALL_RETURN(errstatus, OCIDateFromText, (connection->err, (CONST text *)"01-JAN-00", sizeof("01-JAN-00")-1, NULL, 0, NULL, 0, &oci_date));

			if (errstatus != OCI_SUCCESS) {
				/* failed to convert string to date */
				efree(bind->array.element_lengths);
				efree(bind->array.elements);
				efree(bind);
				connection->errcode = crx_oci_error(connection->err, errstatus);
				CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
				return NULL;
			}

			((OCIDate *)bind->array.elements)[i] = oci_date;
		}
		connection->errcode = 0; /* retain backwards compat with OCI8 1.4 */
	}
	crex_hash_internal_pointer_reset(hash);

	return bind;
}
/* }}} */

#endif /* HAVE_OCI8 */
