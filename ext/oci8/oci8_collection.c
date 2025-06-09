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

/* {{{ crx_oci_collection_create()
 Create and return connection handle */
crx_oci_collection *crx_oci_collection_create(crx_oci_connection *connection, char *tdo, int tdo_len, char *schema, int schema_len)
{
	dvoid *dschp1 = NULL;
	dvoid *parmp1;
	dvoid *parmp2;
	crx_oci_collection *collection;
	sword errstatus;

	collection = emalloc(sizeof(crx_oci_collection));

	collection->connection = connection;
	collection->collection = NULL;
	GC_ADDREF(collection->connection->id);

	/* get type handle by name */
	CRX_OCI_CALL_RETURN(errstatus, OCITypeByName,
			(
			 connection->env,
			 connection->err,
			 connection->svc,
			 (text *) schema,
			 (ub4) schema_len,
			 (text *) tdo,
			 (ub4) tdo_len,
			 (CONST text *) 0,
			 (ub4) 0,
			 OCI_DURATION_SESSION,
			 OCI_TYPEGET_ALL,
			 &(collection->tdo)
			)
	);

	if (errstatus != OCI_SUCCESS) {
		goto CLEANUP;
	}

	/* allocate describe handle */
	CRX_OCI_CALL_RETURN(errstatus, OCIHandleAlloc, (connection->env, (dvoid **) &dschp1, (ub4) OCI_HTYPE_DESCRIBE, (size_t) 0, (dvoid **) 0));

	if (errstatus != OCI_SUCCESS) {
		goto CLEANUP;
	}

	/* describe TDO */
	CRX_OCI_CALL_RETURN(errstatus, OCIDescribeAny,
			(
			 connection->svc,
			 connection->err,
			 (dvoid *) collection->tdo,
			 (ub4) 0,
			 OCI_OTYPE_PTR,
			 (ub1) OCI_DEFAULT,
			 (ub1) OCI_PTYPE_TYPE,
			 dschp1
			)
	);

	if (errstatus != OCI_SUCCESS) {
		goto CLEANUP;
	}

	/* get first parameter handle */
	CRX_OCI_CALL_RETURN(errstatus, OCIAttrGet, ((dvoid *) dschp1, (ub4) OCI_HTYPE_DESCRIBE, (dvoid *)&parmp1, (ub4 *)0, (ub4)OCI_ATTR_PARAM, connection->err));

	if (errstatus != OCI_SUCCESS) {
		goto CLEANUP;
	}

	/* get the collection type code of the attribute */
	CRX_OCI_CALL_RETURN(errstatus, OCIAttrGet,
			(
			 (dvoid*) parmp1,
			 (ub4) OCI_DTYPE_PARAM,
			 (dvoid*) &(collection->coll_typecode),
			 (ub4 *) 0,
			 (ub4) OCI_ATTR_COLLECTION_TYPECODE,
			 connection->err
			)
	);

	if (errstatus != OCI_SUCCESS) {
		goto CLEANUP;
	}

	switch(collection->coll_typecode) {
		case OCI_TYPECODE_TABLE:
		case OCI_TYPECODE_VARRAY:
			/* get collection element handle */
			CRX_OCI_CALL_RETURN(errstatus, OCIAttrGet,
					(
					 (dvoid*) parmp1,
					 (ub4) OCI_DTYPE_PARAM,
					 (dvoid*) &parmp2,
					 (ub4 *) 0,
					 (ub4) OCI_ATTR_COLLECTION_ELEMENT,
					 connection->err
					)
			);

			if (errstatus != OCI_SUCCESS) {
				goto CLEANUP;
			}

			/* get REF of the TDO for the type */
			CRX_OCI_CALL_RETURN(errstatus, OCIAttrGet,
					(
					 (dvoid*) parmp2,
					 (ub4) OCI_DTYPE_PARAM,
					 (dvoid*) &(collection->elem_ref),
					 (ub4 *) 0,
					 (ub4) OCI_ATTR_REF_TDO,
					 connection->err
					)
			);

			if (errstatus != OCI_SUCCESS) {
				goto CLEANUP;
			}

			/* get the TDO (only header) */
			CRX_OCI_CALL_RETURN(errstatus, OCITypeByRef,
					(
					 connection->env,
					 connection->err,
					 collection->elem_ref,
					 OCI_DURATION_SESSION,
					 OCI_TYPEGET_HEADER,
					 &(collection->element_type)
					)
			);

			if (errstatus != OCI_SUCCESS) {
				goto CLEANUP;
			}

			/* get typecode */
			CRX_OCI_CALL_RETURN(errstatus, OCIAttrGet,
					(
					 (dvoid*) parmp2,
					 (ub4) OCI_DTYPE_PARAM,
					 (dvoid*) &(collection->element_typecode),
					 (ub4 *) 0,
					 (ub4) OCI_ATTR_TYPECODE,
					 connection->err
					)
			);

			if (errstatus != OCI_SUCCESS) {
				goto CLEANUP;
			}
			break;
			/* we only support VARRAYs and TABLEs */
		default:
			crx_error_docref(NULL, E_WARNING, "Unknown collection type %d", collection->coll_typecode);
			break;
	}

	/* Create object to hold return table */
	CRX_OCI_CALL_RETURN(errstatus, OCIObjectNew,
		(
			connection->env,
			connection->err,
			connection->svc,
			OCI_TYPECODE_TABLE,
			collection->tdo,
			(dvoid *)0,
			OCI_DURATION_DEFAULT,
			TRUE,
			(dvoid **) &(collection->collection)
		)
	);

	if (errstatus != OCI_SUCCESS) {
		goto CLEANUP;
	}

	/* free the describe handle (Bug #44113) */
	CRX_OCI_CALL(OCIHandleFree, ((dvoid *) dschp1, OCI_HTYPE_DESCRIBE));
	CRX_OCI_REGISTER_RESOURCE(collection, le_collection);
	connection->errcode = 0; /* retain backwards compat with OCI8 1.4 */
	return collection;

CLEANUP:

	if (dschp1) {
		/* free the describe handle (Bug #44113) */
		CRX_OCI_CALL(OCIHandleFree, ((dvoid *) dschp1, OCI_HTYPE_DESCRIBE));
	}
	connection->errcode = crx_oci_error(connection->err, errstatus);
	CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
	crx_oci_collection_close(collection);
	return NULL;
}
/* }}} */

/* {{{ crx_oci_collection_size()
 Return size of the collection */
int crx_oci_collection_size(crx_oci_collection *collection, sb4 *size)
{
	crx_oci_connection *connection = collection->connection;
	sword errstatus;

	CRX_OCI_CALL_RETURN(errstatus, OCICollSize, (connection->env, connection->err, collection->collection, (sb4 *)size));

	if (errstatus != OCI_SUCCESS) {
		connection->errcode = crx_oci_error(connection->err, errstatus);
		CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
		return 1;
	}
	connection->errcode = 0; /* retain backwards compat with OCI8 1.4 */
	return 0;
}
/* }}} */

/* {{{ crx_oci_collection_max()
 Return max number of elements in the collection */
int crx_oci_collection_max(crx_oci_collection *collection, crex_long *max)
{
	crx_oci_connection *connection = collection->connection;

	CRX_OCI_CALL_RETURN(*max, OCICollMax, (connection->env, collection->collection));

	/* error handling is not necessary here? */
	return 0;
}
/* }}} */

/* {{{ crx_oci_collection_trim()
 Trim collection to the given number of elements */
int crx_oci_collection_trim(crx_oci_collection *collection, crex_long trim_size)
{
	crx_oci_connection *connection = collection->connection;
	sword errstatus;

	CRX_OCI_CALL_RETURN(errstatus, OCICollTrim, (connection->env, connection->err, (sb4) trim_size, collection->collection));

	if (errstatus != OCI_SUCCESS) {
		errstatus = crx_oci_error(connection->err, errstatus);
		CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
		return 1;
	}
	connection->errcode = 0; /* retain backwards compat with OCI8 1.4 */
	return 0;
}
/* }}} */

/* {{{ crx_oci_collection_append_null()
 Append NULL element to the end of the collection */
int crx_oci_collection_append_null(crx_oci_collection *collection)
{
	OCIInd null_index = OCI_IND_NULL;
	crx_oci_connection *connection = collection->connection;
	sword errstatus;

	/* append NULL element */
	CRX_OCI_CALL_RETURN(errstatus, OCICollAppend, (connection->env, connection->err, (dvoid *)0, &null_index, collection->collection));

	if (errstatus != OCI_SUCCESS) {
		errstatus = crx_oci_error(connection->err, errstatus);
		CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
		return 1;
	}
	connection->errcode = 0; /* retain backwards compat with OCI8 1.4 */
	return 0;
}
/* }}} */

/* {{{ crx_oci_collection_append_date()
 Append DATE element to the end of the collection (use "DD-MON-YY" format) */
int crx_oci_collection_append_date(crx_oci_collection *collection, char *date, int date_len)
{
	OCIInd new_index = OCI_IND_NOTNULL;
	OCIDate oci_date;
	crx_oci_connection *connection = collection->connection;
	sword errstatus;

	/* format and language are NULLs, so format is "DD-MON-YY" and language is the default language of the session */
	CRX_OCI_CALL_RETURN(errstatus, OCIDateFromText, (connection->err, (CONST text *)date, date_len, NULL, 0, NULL, 0, &oci_date));

	if (errstatus != OCI_SUCCESS) {
		/* failed to convert string to date */
		connection->errcode = crx_oci_error(connection->err, errstatus);
		CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
		return 1;
	}

	CRX_OCI_CALL_RETURN(errstatus, OCICollAppend,
			(
			 connection->env,
			 connection->err,
			 (dvoid *) &oci_date,
			 (dvoid *) &new_index,
			 (OCIColl *) collection->collection
			)
	);

	if (errstatus != OCI_SUCCESS) {
		connection->errcode = crx_oci_error(connection->err, errstatus);
		CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
		return 1;
	}

	connection->errcode = 0; /* retain backwards compat with OCI8 1.4 */
	return 0;
}
/* }}} */

/* {{{ crx_oci_collection_append_number()
 Append NUMBER to the end of the collection */
int crx_oci_collection_append_number(crx_oci_collection *collection, char *number, int number_len)
{
	OCIInd new_index = OCI_IND_NOTNULL;
	double element_double;
	OCINumber oci_number;
	crx_oci_connection *connection = collection->connection;
	sword errstatus;

	element_double = crex_strtod(number, NULL);

	CRX_OCI_CALL_RETURN(errstatus, OCINumberFromReal, (connection->err, &element_double, sizeof(double), &oci_number));

	if (errstatus != OCI_SUCCESS) {
		connection->errcode = crx_oci_error(connection->err, errstatus);
		CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
		return 1;
	}

	CRX_OCI_CALL_RETURN(errstatus, OCICollAppend,
			(
			 connection->env,
			 connection->err,
			 (dvoid *) &oci_number,
			 (dvoid *) &new_index,
			 (OCIColl *) collection->collection
			)
	);

	if (errstatus != OCI_SUCCESS) {
		connection->errcode = crx_oci_error(connection->err, errstatus);
		CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
		return 1;
	}

	connection->errcode = 0; /* retain backwards compat with OCI8 1.4 */
	return 0;
}
/* }}} */

/* {{{ crx_oci_collection_append_string()
 Append STRING to the end of the collection */
int crx_oci_collection_append_string(crx_oci_collection *collection, char *element, int element_len)
{
	OCIInd new_index = OCI_IND_NOTNULL;
	OCIString *ocistr = (OCIString *)0;
	crx_oci_connection *connection = collection->connection;
	sword errstatus;

	CRX_OCI_CALL_RETURN(errstatus, OCIStringAssignText, (connection->env, connection->err, (CONST oratext *)element, element_len, &ocistr));

	if (errstatus != OCI_SUCCESS) {
		connection->errcode = crx_oci_error(connection->err, errstatus);
		CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
		return 1;
	}

	CRX_OCI_CALL_RETURN(errstatus, OCICollAppend,
			(
			 connection->env,
			 connection->err,
			 (dvoid *) ocistr,
			 (dvoid *) &new_index,
			 (OCIColl *) collection->collection
			)
	);

	if (errstatus != OCI_SUCCESS) {
		connection->errcode = crx_oci_error(connection->err, errstatus);
		CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
		return 1;
	}

	connection->errcode = 0; /* retain backwards compat with OCI8 1.4 */
	return 0;
}
/* }}} */

/* {{{ crx_oci_collection_append()
 Append wrapper. Appends any supported element to the end of the collection */
int crx_oci_collection_append(crx_oci_collection *collection, char *element, int element_len)
{
	if (element_len == 0) {
		return crx_oci_collection_append_null(collection);
	}

	switch(collection->element_typecode) {
		case OCI_TYPECODE_DATE:
			return crx_oci_collection_append_date(collection, element, element_len);
			break;

		case OCI_TYPECODE_VARCHAR2 :
			return crx_oci_collection_append_string(collection, element, element_len);
			break;

		case OCI_TYPECODE_UNSIGNED16 :						 /* UNSIGNED SHORT	*/
		case OCI_TYPECODE_UNSIGNED32 :						  /* UNSIGNED LONG	*/
		case OCI_TYPECODE_REAL :									 /* REAL	*/
		case OCI_TYPECODE_DOUBLE :									 /* DOUBLE	*/
		case OCI_TYPECODE_INTEGER :										/* INT	*/
		case OCI_TYPECODE_SIGNED16 :								  /* SHORT	*/
		case OCI_TYPECODE_SIGNED32 :								   /* LONG	*/
		case OCI_TYPECODE_DECIMAL :									/* DECIMAL	*/
		case OCI_TYPECODE_FLOAT :									/* FLOAT	*/
		case OCI_TYPECODE_NUMBER :									/* NUMBER	*/
		case OCI_TYPECODE_SMALLINT :								/* SMALLINT */
			return crx_oci_collection_append_number(collection, element, element_len);
			break;

		default:
			crx_error_docref(NULL, E_NOTICE, "Unknown or unsupported type of element: %d", collection->element_typecode);
			return 1;
			break;
	}
	/* never reached */
	return 1;
}
/* }}} */

/* {{{ crx_oci_collection_element_get()
 Get the element with the given index */
int crx_oci_collection_element_get(crx_oci_collection *collection, crex_long index, zval *result_element)
{
	crx_oci_connection *connection = collection->connection;
	dvoid *element;
	OCIInd *element_index;
	boolean exists;
	oratext buff[1024];
	ub4 buff_len = 1024;
	sword errstatus;

	ZVAL_NULL(result_element);

	connection->errcode = 0; /* retain backwards compat with OCI8 1.4 */

	CRX_OCI_CALL_RETURN(errstatus, OCICollGetElem,
			(
			 connection->env,
			 connection->err,
			 collection->collection,
			 (ub4)index,
			 &exists,
			 &element,
			 (dvoid **)&element_index
			)
	);

	if (errstatus != OCI_SUCCESS) {
		connection->errcode = crx_oci_error(connection->err, errstatus);
		CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
		return 1;
	}

	if (exists == 0) {
		/* element doesn't exist */
		return 1;
	}

	if (*element_index == OCI_IND_NULL) {
		/* this is not an error, we're returning NULL here */
		return 0;
	}

	switch (collection->element_typecode) {
		case OCI_TYPECODE_DATE:
			CRX_OCI_CALL_RETURN(errstatus, OCIDateToText, (connection->err, element, 0, 0, 0, 0, &buff_len, buff));

			if (errstatus != OCI_SUCCESS) {
				connection->errcode = crx_oci_error(connection->err, errstatus);
				CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
				return 1;
			}

			ZVAL_STRINGL(result_element, (char *)buff, buff_len);
			C_STRVAL_P(result_element)[buff_len] = '\0';

			return 0;
			break;

		case OCI_TYPECODE_VARCHAR2:
		{
			OCIString *oci_string = *(OCIString **)element;
			text *str;

			CRX_OCI_CALL_RETURN(str, OCIStringPtr, (connection->env, oci_string));

			if (str) {
				ZVAL_STRING(result_element, (char *)str);
			}
			return 0;
		}
			break;

		case OCI_TYPECODE_UNSIGNED16:						/* UNSIGNED SHORT  */
		case OCI_TYPECODE_UNSIGNED32:						/* UNSIGNED LONG  */
		case OCI_TYPECODE_REAL:								/* REAL	   */
		case OCI_TYPECODE_DOUBLE:							/* DOUBLE  */
		case OCI_TYPECODE_INTEGER:							/* INT	*/
		case OCI_TYPECODE_SIGNED16:							/* SHORT  */
		case OCI_TYPECODE_SIGNED32:							/* LONG	 */
		case OCI_TYPECODE_DECIMAL:							/* DECIMAL	*/
		case OCI_TYPECODE_FLOAT:							/* FLOAT	*/
		case OCI_TYPECODE_NUMBER:							/* NUMBER	*/
		case OCI_TYPECODE_SMALLINT:							/* SMALLINT */
		{
			double double_number;

			CRX_OCI_CALL_RETURN(errstatus, OCINumberToReal, (connection->err, (CONST OCINumber *) element, (uword) sizeof(double), (dvoid *) &double_number));

			if (errstatus != OCI_SUCCESS) {
				connection->errcode = crx_oci_error(connection->err, errstatus);
				CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
				return 1;
			}

			ZVAL_DOUBLE(result_element, double_number);

			return 0;
		}
			break;
		default:
			crx_error_docref(NULL, E_NOTICE, "Unknown or unsupported type of element: %d", collection->element_typecode);
			return 1;
			break;
	}
	/* never reached */
	return 1;
}
/* }}} */

/* {{{ crx_oci_collection_element_set_null()
 Set the element with the given index to NULL */
int crx_oci_collection_element_set_null(crx_oci_collection *collection, crex_long index)
{
	OCIInd null_index = OCI_IND_NULL;
	crx_oci_connection *connection = collection->connection;
	sword errstatus;

	/* set NULL element */
	CRX_OCI_CALL_RETURN(errstatus, OCICollAssignElem, (connection->env, connection->err, (ub4) index, (dvoid *)"", &null_index, collection->collection));

	if (errstatus != OCI_SUCCESS) {
		connection->errcode = crx_oci_error(connection->err, errstatus);
		CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
		return 1;
	}
	connection->errcode = 0; /* retain backwards compat with OCI8 1.4 */
	return 0;
}
/* }}} */

/* {{{ crx_oci_collection_element_set_date()
 Change element's value to the given DATE */
int crx_oci_collection_element_set_date(crx_oci_collection *collection, crex_long index, char *date, int date_len)
{
	OCIInd new_index = OCI_IND_NOTNULL;
	OCIDate oci_date;
	crx_oci_connection *connection = collection->connection;
	sword errstatus;

	/* format and language are NULLs, so format is "DD-MON-YY" and language is the default language of the session */
	CRX_OCI_CALL_RETURN(errstatus, OCIDateFromText, (connection->err, (CONST text *)date, date_len, NULL, 0, NULL, 0, &oci_date));

	if (errstatus != OCI_SUCCESS) {
		/* failed to convert string to date */
		connection->errcode = crx_oci_error(connection->err, errstatus);
		CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
		return 1;
	}

	CRX_OCI_CALL_RETURN(errstatus, OCICollAssignElem,
			(
			 connection->env,
			 connection->err,
			 (ub4)index,
			 (dvoid *) &oci_date,
			 (dvoid *) &new_index,
			 (OCIColl *) collection->collection
			 )
	);

	if (errstatus != OCI_SUCCESS) {
		connection->errcode = crx_oci_error(connection->err, errstatus);
		CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
		return 1;
	}

	connection->errcode = 0; /* retain backwards compat with OCI8 1.4 */
	return 0;
}
/* }}} */

/* {{{ crx_oci_collection_element_set_number()
 Change element's value to the given NUMBER */
int crx_oci_collection_element_set_number(crx_oci_collection *collection, crex_long index, char *number, int number_len)
{
	OCIInd new_index = OCI_IND_NOTNULL;
	double element_double;
	OCINumber oci_number;
	crx_oci_connection *connection = collection->connection;
	sword errstatus;

	element_double = crex_strtod(number, NULL);

	CRX_OCI_CALL_RETURN(errstatus, OCINumberFromReal, (connection->err, &element_double, sizeof(double), &oci_number));

	if (errstatus != OCI_SUCCESS) {
		connection->errcode = crx_oci_error(connection->err, errstatus);
		CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
		return 1;
	}

	CRX_OCI_CALL_RETURN(errstatus, OCICollAssignElem,
			(
			 connection->env,
			 connection->err,
			 (ub4) index,
			 (dvoid *) &oci_number,
			 (dvoid *) &new_index,
			 (OCIColl *) collection->collection
			 )
	);

	if (errstatus != OCI_SUCCESS) {
		connection->errcode = crx_oci_error(connection->err, errstatus);
		CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
		return 1;
	}

	connection->errcode = 0; /* retain backwards compat with OCI8 1.4 */
	return 0;
}
/* }}} */

/* {{{ crx_oci_collection_element_set_string()
 Change element's value to the given string */
int crx_oci_collection_element_set_string(crx_oci_collection *collection, crex_long index, char *element, int element_len)
{
	OCIInd new_index = OCI_IND_NOTNULL;
	OCIString *ocistr = (OCIString *)0;
	crx_oci_connection *connection = collection->connection;
	sword errstatus;

	CRX_OCI_CALL_RETURN(errstatus, OCIStringAssignText, (connection->env, connection->err, (CONST oratext *)element, element_len, &ocistr));

	if (errstatus != OCI_SUCCESS) {
		connection->errcode = crx_oci_error(connection->err, errstatus);
		CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
		return 1;
	}

	CRX_OCI_CALL_RETURN(errstatus, OCICollAssignElem,
			(
			 connection->env,
			 connection->err,
			 (ub4)index,
			 (dvoid *) ocistr,
			 (dvoid *) &new_index,
			 (OCIColl *) collection->collection
			 )
	);

	if (errstatus != OCI_SUCCESS) {
		connection->errcode = crx_oci_error(connection->err, errstatus);
		CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
		return 1;
	}

	connection->errcode = 0; /* retain backwards compat with OCI8 1.4 */
	return 0;
}
/* }}} */

/* {{{ crx_oci_collection_element_set()
 Collection element setter */
int crx_oci_collection_element_set(crx_oci_collection *collection, crex_long index, char *value, int value_len)
{
	if (value_len == 0) {
		return crx_oci_collection_element_set_null(collection, index);
	}

	switch(collection->element_typecode) {
		case OCI_TYPECODE_DATE:
			return crx_oci_collection_element_set_date(collection, index, value, value_len);
			break;

		case OCI_TYPECODE_VARCHAR2 :
			return crx_oci_collection_element_set_string(collection, index, value, value_len);
			break;

		case OCI_TYPECODE_UNSIGNED16 :						 /* UNSIGNED SHORT	*/
		case OCI_TYPECODE_UNSIGNED32 :						  /* UNSIGNED LONG	*/
		case OCI_TYPECODE_REAL :									 /* REAL	*/
		case OCI_TYPECODE_DOUBLE :									 /* DOUBLE	*/
		case OCI_TYPECODE_INTEGER :										/* INT	*/
		case OCI_TYPECODE_SIGNED16 :								  /* SHORT	*/
		case OCI_TYPECODE_SIGNED32 :								   /* LONG	*/
		case OCI_TYPECODE_DECIMAL :									/* DECIMAL	*/
		case OCI_TYPECODE_FLOAT :									/* FLOAT	*/
		case OCI_TYPECODE_NUMBER :									/* NUMBER	*/
		case OCI_TYPECODE_SMALLINT :								/* SMALLINT */
			return crx_oci_collection_element_set_number(collection, index, value, value_len);
			break;

		default:
			crx_error_docref(NULL, E_NOTICE, "Unknown or unsupported type of element: %d", collection->element_typecode);
			return 1;
			break;
	}
	/* never reached */
	return 1;
}
/* }}} */

/* {{{ crx_oci_collection_assign()
 Assigns a value to the collection from another collection */
int crx_oci_collection_assign(crx_oci_collection *collection_dest, crx_oci_collection *collection_from)
{
	crx_oci_connection *connection = collection_dest->connection;
	sword errstatus;

	CRX_OCI_CALL_RETURN(errstatus, OCICollAssign, (connection->env, connection->err, collection_from->collection, collection_dest->collection));

	if (errstatus != OCI_SUCCESS) {
		connection->errcode = crx_oci_error(connection->err, errstatus);
		CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
		return 1;
	}
	connection->errcode = 0; /* retain backwards compat with OCI8 1.4 */
	return 0;
}
/* }}} */

/* {{{ crx_oci_collection_close()
 Destroy collection and all associated resources */
void crx_oci_collection_close(crx_oci_collection *collection)
{
	crx_oci_connection *connection = collection->connection;
	sword errstatus;

	if (collection->collection) {
		CRX_OCI_CALL_RETURN(errstatus, OCIObjectFree, (connection->env, connection->err, (dvoid *)collection->collection, (ub2)OCI_OBJECTFREE_FORCE));

		if (errstatus != OCI_SUCCESS) {
			connection->errcode = crx_oci_error(connection->err, errstatus);
			CRX_OCI_HANDLE_ERROR(connection, connection->errcode);
		} else {
			connection->errcode = 0; /* retain backwards compat with OCI8 1.4 */
		}
	}

	crex_list_delete(collection->connection->id);
	efree(collection);
	return;
}
/* }}} */

#endif /* HAVE_OCI8 */
