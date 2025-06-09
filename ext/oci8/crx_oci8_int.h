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

#ifdef HAVE_OCI8
# ifndef CRX_OCI8_INT_H
#  define CRX_OCI8_INT_H

/* {{{ misc defines */
# if (defined(__osf__) && defined(__alpha))
#  ifndef A_OSF
#	define A_OSF
#  endif
#  ifndef OSF1
#	define OSF1
#  endif
#  ifndef _INTRINSICS
#	define _INTRINSICS
#  endif
# endif /* osf alpha */

#ifdef HAVE_OCI8_DTRACE
#include "oci8_dtrace_gen.h"
#endif

#if defined(min)
#undef min
#endif
#if defined(max)
#undef max
#endif
/* }}} */

#include "ext/standard/crx_string.h"
CREX_DIAGNOSTIC_IGNORED_START("-Wstrict-prototypes")
#include <oci.h>
CREX_DIAGNOSTIC_IGNORED_END

#if !defined(OCI_MAJOR_VERSION) || OCI_MAJOR_VERSION < 11 || ((OCI_MAJOR_VERSION == 11) && (OCI_MINOR_VERSION < 2))
#error This version of CRX OCI8 requires Oracle Client libraries from 11.2 or later.
#endif

extern int le_connection;
extern int le_pconnection;
extern int le_statement;
extern int le_descriptor;
extern int le_collection;
extern int le_server;
extern int le_session;

extern crex_class_entry *oci_lob_class_entry_ptr;
extern crex_class_entry *oci_coll_class_entry_ptr;

/* {{{ constants */
#define CRX_OCI_SEEK_SET 0
#define CRX_OCI_SEEK_CUR 1
#define CRX_OCI_SEEK_END 2

#define CRX_OCI_MAX_NAME_LEN  64
#define CRX_OCI_MAX_DATA_SIZE INT_MAX
#define CRX_OCI_PIECE_SIZE	  ((64*1024)-1)
#define CRX_OCI_LOB_BUFFER_SIZE 1048576l  /* 1Mb seems to be the most reasonable buffer size for LOB reading */

#define CRX_OCI_ASSOC				(1<<0)
#define CRX_OCI_NUM					(1<<1)
#define CRX_OCI_BOTH				(CRX_OCI_ASSOC|CRX_OCI_NUM)

#define CRX_OCI_RETURN_NULLS		(1<<2)
#define CRX_OCI_RETURN_LOBS			(1<<3)

#define CRX_OCI_FETCHSTATEMENT_BY_COLUMN	(1<<4)
#define CRX_OCI_FETCHSTATEMENT_BY_ROW		(1<<5)
#define CRX_OCI_FETCHSTATEMENT_BY			(CRX_OCI_FETCHSTATEMENT_BY_COLUMN | CRX_OCI_FETCHSTATEMENT_BY_ROW)

#define CRX_OCI_LOB_BUFFER_DISABLED 0
#define CRX_OCI_LOB_BUFFER_ENABLED  1
#define CRX_OCI_LOB_BUFFER_USED     2

#ifdef OCI_ERROR_MAXMSG_SIZE2
/* Bigger size is defined from 11.2.0.3 onwards */
#define CRX_OCI_ERRBUF_LEN OCI_ERROR_MAXMSG_SIZE2
#else
#define CRX_OCI_ERRBUF_LEN OCI_ERROR_MAXMSG_SIZE
#endif

/* The mode parameter for oci_connect() is overloaded and accepts both
 * privilege and external authentication flags OR'd together.
 * CRX_OCI_CRED_EXT must be distinct from the OCI_xxx privilege
 * values.
 */
#define CRX_OCI_CRED_EXT                    (1u<<31)
#if ((CRX_OCI_CRED_EXT == OCI_DEFAULT) || (CRX_OCI_CRED_EXT & (OCI_SYSOPER | OCI_SYSDBA)))
#error Invalid value for CRX_OCI_CRED_EXT
#endif

#define CRX_OCI_IMPRES_UNKNOWN			0
#define CRX_OCI_IMPRES_NO_CHILDREN		1
#define CRX_OCI_IMPRES_HAS_CHILDREN		2
#define CRX_OCI_IMPRES_IS_CHILD			3

/*
 * Name passed to Oracle for tracing.  Note some DB views only show
 * the first nine characters of the driver name.
 */
#define CRX_OCI8_DRIVER_NAME     "CRX OCI8 : " CRX_OCI8_VERSION

/* }}} */

/* {{{ crx_oci_spool */
typedef struct {
	crex_resource *id;					/* resource id */
	OCIEnv		 *env;					/* env of this session pool */
	OCIError	 *err;					/* pool's error handle	*/
	OCISPool	 *poolh;				/* pool handle */
	void		 *poolname;				/* session pool name */
	unsigned int  poolname_len;			/* length of session pool name */
	crex_string	 *spool_hash_key;		/* Hash key for session pool in plist */
} crx_oci_spool;
/* }}} */

/* {{{ crx_oci_connection */
typedef struct {
	crex_resource  *id;							/* resource ID */
	OCIEnv		   *env;						/* private env handle */
	ub2				charset;					/* charset ID */
	OCIServer	   *server;						/* private server handle */
	OCISvcCtx	   *svc;						/* private service context handle */
	OCISession	   *session;					/* private session handle */
	OCIAuthInfo	   *authinfo;					/* Cached authinfo handle for OCISessionGet */
	OCIError	   *err;						/* private error handle */
	crx_oci_spool  *private_spool;				/* private session pool (for persistent) */
	sb4				errcode;					/* last ORA- error number */

	HashTable	   *descriptors;				/* descriptors hash, used to flush all the LOBs using this connection on commit */
	crex_ulong		descriptor_count;			/* used to index the descriptors hash table.  Not an accurate count */
	unsigned		is_open:1;					/* hels to determine if the connection is dead or not */
	unsigned		is_attached:1;				/* hels to determine if we should detach from the server when closing/freeing the connection */
	unsigned		is_persistent:1;			/* self-descriptive */
	unsigned		used_this_request:1;		/* helps to determine if we should reset connection's next ping time and check its timeout */
	unsigned		rb_on_disconnect:1;			/* helps to determine if we should rollback this connection on close/shutdown */
	unsigned		passwd_changed:1;			/* helps determine if a persistent connection hash should be invalidated after a password change */
	unsigned		is_stub:1;					/* flag to keep track whether the connection structure has a real OCI connection associated */
	unsigned		using_spool:1;				/* Is this connection from session pool? */
	time_t			idle_expiry;				/* time when the connection will be considered as expired */
	time_t		   *next_pingp;					/* (pointer to) time of the next ping */
	crex_string	   *hash_key;					/* hashed details of the connection */
#ifdef HAVE_OCI8_DTRACE
	char		   *client_id;					/* The oci_set_client_identifier() value */
#endif

	zval		    taf_callback;				/* The Oracle TAF callback function in the userspace */
} crx_oci_connection;
/* }}} */

/* {{{ crx_oci_descriptor */
typedef struct {
	crex_resource		*id;
	crex_ulong				 index;		            /* descriptors hash table index */
	crx_oci_connection	*connection;			/* parent connection handle */
	dvoid				*descriptor;			/* OCI descriptor handle */
	ub4					 type;					/* descriptor type (FILE/LOB) */
	ub4					 lob_current_position;	/* LOB internal pointer */
	int					 lob_size;				/* cached LOB size. -1 = Lob wasn't initialized yet */
	int					 buffering;				/* cached buffering flag. 0 - off, 1 - on, 2 - on and buffer was used */
	ub4					 chunk_size;			/* chunk size of the LOB. 0 - unknown */
	ub1					 charset_form;			/* charset form, required for NCLOBs */
	ub2					 charset_id;			/* charset ID */
	unsigned			 is_open:1;				/* helps to determine if lob is open or not */
} crx_oci_descriptor;
/* }}} */

/* {{{ crx_oci_lob_ctx */
typedef struct {
	char			   **lob_data;				/* address of pointer to LOB data */
	ub4					*lob_len;				/* address of LOB length variable (bytes) */
	ub4					 alloc_len;
} crx_oci_lob_ctx;
/* }}} */

/* {{{ crx_oci_collection */
typedef struct {
	crex_resource		*id;
	crx_oci_connection	*connection;			/* parent connection handle */
	OCIType				*tdo;					/* collection's type handle */
	OCITypeCode			 coll_typecode;			/* collection's typecode handle */
	OCIRef				*elem_ref;				/* element's reference handle */
	OCIType				*element_type;			/* element's type handle */
	OCITypeCode			 element_typecode;		/* element's typecode handle */
	OCIColl				*collection;			/* collection handle */
} crx_oci_collection;
/* }}} */

/* {{{ crx_oci_define */
typedef struct {
	zval		 val;			/* zval used in define */
	text		*name;			/* placeholder's name */
	ub4			 name_len;		/* placeholder's name length */
	ub4			 type;			/* define type */
} crx_oci_define;
/* }}} */

/* {{{ crx_oci_statement */
typedef struct {
	crex_resource		*id;
	crex_resource	 	*parent_stmtid;			/* parent statement id */
	struct crx_oci_statement *impres_child_stmt;/* child of current Implicit Result Set statement handle */
	ub4                  impres_count;          /* count of remaining Implicit Result children on parent statement handle */
	crx_oci_connection	*connection;			/* parent connection handle */
	sb4					 errcode;				/* last ORA- error number */
	OCIError			*err;					/* private error handle */
	OCIStmt				*stmt;					/* statement handle */
	char				*last_query;			/* last query issued. also used to determine if this is a statement or a refcursor received from Oracle */
	char                 impres_flag;           /* CRX_OCI_IMPRES_*_ */
	crex_long			 last_query_len;		/* last query length */
	HashTable			*columns;				/* hash containing all the result columns */
	HashTable			*binds;					/* binds hash */
	HashTable			*defines;				/* defines hash */
	int					 ncolumns;				/* number of columns in the result */
	unsigned			 executed:1;			/* statement executed flag */
	unsigned			 has_data:1;			/* statement has more data flag */
	unsigned			 has_descr:1;			/* statement has at least one descriptor or cursor column */
	ub2					 stmttype;				/* statement type */
	ub4                  prefetch_count;        /* row prefetch count */
	ub4                  prefetch_lob_size;     /* LOB prefetch size */
} crx_oci_statement;
/* }}} */

/* {{{ crx_oci_bind */
typedef struct {
	OCIBind				*bind;					/* bind handle */
	zval				val;					/* value */
	dvoid				*descriptor;			/* used for binding of LOBS etc */
	OCIStmt				*statement;				/* used for binding REFCURSORs */
	crx_oci_statement	*parent_statement;		/* pointer to the parent statement */
	ub2 type;						/* bind type */
	struct {
		void	*elements;
		sb2		*indicators;
		ub2		*element_lengths;
		ub4		 current_length;
		ub4		 old_length;
		ub4		 max_length;
		crex_long	 type;
	} array;
	sb2					 indicator;				/* -1 means NULL */
	ub2					 retcode;
	ub4					 dummy_len;				/* a dummy var to store alenpp value in bind OUT callback */
} crx_oci_bind;
/* }}} */

/* {{{ crx_oci_out_column */
typedef struct {
	crx_oci_statement	*statement;				/* statement handle. used when fetching REFCURSORS */
	crx_oci_statement	*nested_statement;		/* statement handle. used when fetching REFCURSORS */
	OCIDefine			*oci_define;			/* define handle */
	char				*name;					/* column name */
	ub4					 name_len;				/* column name length */
	ub2					 data_type;				/* column data type */
	ub2					 data_size;				/* data size */
	ub4					 storage_size4;			/* size used when allocating buffers */
	sb2					 indicator;
	ub2					 retcode;				/* code returned when fetching this particular column */
	ub2					 retlen;
	ub4					 retlen4;
	ub2					 is_descr;				/* column contains a descriptor */
	ub2					 is_cursor;				/* column contains a cursor */
	crex_resource		*stmtid;				/* statement id for cursors */
	crex_resource		*descid;				/* descriptor id for descriptors */
	void				*data;
	crx_oci_define		*define;				/* define handle */
	int					 piecewise;				/* column is fetched piece-by-piece */
	ub4					 cb_retlen;
	sb1					 scale;					/* column scale */
	sb2					 precision;				/* column precision */
	ub1					 charset_form;			/* charset form, required for NCLOBs */
	ub2					 charset_id;			/* charset ID */
	ub4					 chunk_size;			/* LOB chunk size */
} crx_oci_out_column;
/* }}} */

/* {{{ macros */

#define CRX_OCI_CALL(func, params)								\
	do {																\
		OCI_G(in_call) = 1;												\
		func params;													\
		OCI_G(in_call) = 0;												\
	} while (0)

#define CRX_OCI_CALL_RETURN(__retval, func, params)			\
	do {																\
		OCI_G(in_call) = 1;												\
		__retval = func params;											\
		OCI_G(in_call) = 0;												\
	} while (0)

/* Check for errors that indicate the connection to the DB is no
 * longer valid.  If it isn't, then the CRX connection is marked to be
 * reopened by the next CRX OCI8 connect command.  This is most useful
 * for persistent connections.	The error number list is not
 * exclusive.  The error number comparisons and the
 * OCI_ATTR_SERVER_STATUS check are done for maximum cross-version
 * compatibility. In the far future, only the attribute check will be
 * needed.
 */
#define CRX_OCI_HANDLE_ERROR(connection, errcode) \
	do {										  \
		ub4 serverStatus = OCI_SERVER_NORMAL;	  \
		switch (errcode) {						  \
			case  1013:							  \
				crex_bailout();					  \
				break;							  \
			case	22:							  \
			case	28:							  \
			case   378:							  \
			case   602:							  \
			case   603:							  \
			case   604:							  \
			case   609:							  \
			case  1012:							  \
			case  1033:							  \
			case  1041:							  \
			case  1043:							  \
			case  1089:							  \
			case  1090:							  \
			case  1092:							  \
			case  3113:							  \
			case  3114:							  \
			case  3122:							  \
			case  3135:							  \
			case  3136:							  \
			case 12153:							  \
			case 12161:							  \
			case 27146:							  \
			case 28511:							  \
				(connection)->is_open = 0;		  \
				break;							  \
			default:										\
			{												\
				CRX_OCI_CALL(OCIAttrGet, ((dvoid *)(connection)->server, OCI_HTYPE_SERVER, (dvoid *)&serverStatus, \
										  (ub4 *)0, OCI_ATTR_SERVER_STATUS, (connection)->err)); \
				if (serverStatus != OCI_SERVER_NORMAL) {	\
					(connection)->is_open = 0;				\
				}											\
			}												\
			break;											\
		}													\
		crx_oci_dtrace_check_connection(connection, errcode, serverStatus); \
	} while (0)

#define CRX_OCI_REGISTER_RESOURCE(resource, le_resource) \
	do { \
		resource->id = crex_register_resource(resource, le_resource); \
	} while (0)

#define CRX_OCI_ZVAL_TO_CONNECTION(zval, connection) \
	if ((connection = (crx_oci_connection *)crex_fetch_resource2(C_RES_P(zval), "oci8 connection", le_connection, le_pconnection)) == NULL) { \
		RETURN_THROWS(); \
	}

#define CRX_OCI_ZVAL_TO_STATEMENT(zval, statement) \
	if ((statement = (crx_oci_statement *)crex_fetch_resource(C_RES_P(zval), "oci8 statement", le_statement)) == NULL) { \
		RETURN_THROWS(); \
	}

#define CRX_OCI_ZVAL_TO_DESCRIPTOR(zval, descriptor) \
	if ((descriptor = (crx_oci_descriptor *)crex_fetch_resource(C_RES_P(zval), "oci8 descriptor", le_descriptor)) == NULL) { \
		RETURN_THROWS(); \
	}

#define CRX_OCI_ZVAL_TO_COLLECTION(zval, collection) \
	if ((collection = (crx_oci_collection *)crex_fetch_resource(C_RES_P(zval), "oci8 collection", le_collection)) == NULL) { \
		RETURN_THROWS(); \
	}

#define CRX_OCI_FETCH_RESOURCE_EX(zval, var, type, name, resource_type)						 \
	do { \
		var = (type) crex_fetch_resource(C_RES_P(zval), name, resource_type);                \
		if (!var) {																			 \
			return 1;																		 \
		} \
	} while (0)

#define CRX_OCI_ZVAL_TO_CONNECTION_EX(zval, connection) \
	CRX_OCI_FETCH_RESOURCE_EX(zval, connection, crx_oci_connection *, "oci8 connection", le_connection)

#define CRX_OCI_ZVAL_TO_STATEMENT_EX(zval, statement) \
	CRX_OCI_FETCH_RESOURCE_EX(zval, statement, crx_oci_statement *, "oci8 statement", le_statement)

#define CRX_OCI_ZVAL_TO_DESCRIPTOR_EX(zval, descriptor) \
	CRX_OCI_FETCH_RESOURCE_EX(zval, descriptor, crx_oci_descriptor *, "oci8 descriptor", le_descriptor)

#define CRX_OCI_ZVAL_TO_COLLECTION_EX(zval, collection) \
	CRX_OCI_FETCH_RESOURCE_EX(zval, collection, crx_oci_collection *, "oci8 collection", le_collection)

/* }}} */

/* PROTOS */

/* {{{ main prototypes */

void crx_oci_column_hash_dtor(zval *data);
void crx_oci_define_hash_dtor(zval *data);
void crx_oci_bind_hash_dtor(zval *data);
void crx_oci_descriptor_flush_hash_dtor(zval *data);
void crx_oci_connection_descriptors_free(crx_oci_connection *connection);
sb4 crx_oci_error(OCIError *err_p, sword status);
sb4 crx_oci_fetch_errmsg(OCIError *error_handle, text *error_buf, size_t error_buf_size);
int crx_oci_fetch_sqltext_offset(crx_oci_statement *statement, text **sqltext, ub2 *error_offset);
void crx_oci_do_connect(INTERNAL_FUNCTION_PARAMETERS, int persistent, int exclusive);
crx_oci_connection *crx_oci_do_connect_ex(char *username, int username_len, char *password, int password_len, char *new_password, int new_password_len, char *dbname, int dbname_len, char *charset, crex_long session_mode, int persistent, int exclusive);
int crx_oci_connection_rollback(crx_oci_connection *connection);
int crx_oci_connection_commit(crx_oci_connection *connection);
int crx_oci_connection_release(crx_oci_connection *connection);
int crx_oci_password_change(crx_oci_connection *connection, char *user, int user_len, char *pass_old, int pass_old_len, char *pass_new, int pass_new_len);
void crx_oci_client_get_version(char *version, size_t version_size);
int crx_oci_server_get_version(crx_oci_connection *connection, char *version, size_t version_size);
void crx_oci_fetch_row(INTERNAL_FUNCTION_PARAMETERS, int mode, int expected_args);
int crx_oci_column_to_zval(crx_oci_out_column *column, zval *value, int mode);
void crx_oci_dtrace_check_connection(crx_oci_connection *connection, sb4 errcode, ub4 serverStatus);

/* }}} */

/* {{{ lob related prototypes */

crx_oci_descriptor *crx_oci_lob_create(crx_oci_connection *connection, crex_long type);
int crx_oci_lob_get_length(crx_oci_descriptor *descriptor, ub4 *length);
int crx_oci_lob_read(crx_oci_descriptor *descriptor, crex_long read_length, crex_long inital_offset, char **data, ub4 *data_len);
int crx_oci_lob_write(crx_oci_descriptor *descriptor, ub4 offset, char *data, int data_len, ub4 *bytes_written);
int crx_oci_lob_flush(crx_oci_descriptor *descriptor, crex_long flush_flag);
int crx_oci_lob_set_buffering(crx_oci_descriptor *descriptor, int on_off);
int crx_oci_lob_get_buffering(crx_oci_descriptor *descriptor);
int crx_oci_lob_copy(crx_oci_descriptor *descriptor, crx_oci_descriptor *descriptor_from, crex_long length);
int crx_oci_lob_close(crx_oci_descriptor *descriptor);
int crx_oci_temp_lob_close(crx_oci_descriptor *descriptor);
int crx_oci_lob_write_tmp(crx_oci_descriptor *descriptor, crex_long type, char *data, int data_len);
void crx_oci_lob_free(crx_oci_descriptor *descriptor);
int crx_oci_lob_import(crx_oci_descriptor *descriptor, char *filename);
int crx_oci_lob_append(crx_oci_descriptor *descriptor_dest, crx_oci_descriptor *descriptor_from);
int crx_oci_lob_truncate(crx_oci_descriptor *descriptor, crex_long new_lob_length);
int crx_oci_lob_erase(crx_oci_descriptor *descriptor, crex_long offset, ub4 length, ub4 *bytes_erased);
int crx_oci_lob_is_equal(crx_oci_descriptor *descriptor_first, crx_oci_descriptor *descriptor_second, boolean *result);
sb4 crx_oci_lob_callback(dvoid *ctxp, CONST dvoid *bufxp, oraub8 len, ub1 piece, dvoid **changed_bufpp, oraub8 *changed_lenp);
/* }}} */

/* {{{ collection related prototypes */

crx_oci_collection *crx_oci_collection_create(crx_oci_connection *connection, char *tdo, int tdo_len, char *schema, int schema_len);
int crx_oci_collection_size(crx_oci_collection *collection, sb4 *size);
int crx_oci_collection_max(crx_oci_collection *collection, crex_long *max);
int crx_oci_collection_trim(crx_oci_collection *collection, crex_long trim_size);
int crx_oci_collection_append(crx_oci_collection *collection, char *element, int element_len);
int crx_oci_collection_element_get(crx_oci_collection *collection, crex_long index, zval *result_element);
int crx_oci_collection_element_set(crx_oci_collection *collection, crex_long index, char *value, int value_len);
int crx_oci_collection_element_set_null(crx_oci_collection *collection, crex_long index);
int crx_oci_collection_element_set_date(crx_oci_collection *collection, crex_long index, char *date, int date_len);
int crx_oci_collection_element_set_number(crx_oci_collection *collection, crex_long index, char *number, int number_len);
int crx_oci_collection_element_set_string(crx_oci_collection *collection, crex_long index, char *element, int element_len);
int crx_oci_collection_assign(crx_oci_collection *collection_dest, crx_oci_collection *collection_from);
void crx_oci_collection_close(crx_oci_collection *collection);
int crx_oci_collection_append_null(crx_oci_collection *collection);
int crx_oci_collection_append_date(crx_oci_collection *collection, char *date, int date_len);
int crx_oci_collection_append_number(crx_oci_collection *collection, char *number, int number_len);
int crx_oci_collection_append_string(crx_oci_collection *collection, char *element, int element_len);


/* }}} */

/* {{{ statement related prototypes */

crx_oci_statement *crx_oci_statement_create(crx_oci_connection *connection, char *query, int query_len);
crx_oci_statement *crx_oci_get_implicit_resultset(crx_oci_statement *statement);
int crx_oci_statement_set_prefetch(crx_oci_statement *statement, ub4 prefetch);
int crx_oci_statement_fetch(crx_oci_statement *statement, ub4 nrows);
crx_oci_out_column *crx_oci_statement_get_column(crx_oci_statement *statement, crex_long column_index, char *column_name, int column_name_len);
int crx_oci_statement_execute(crx_oci_statement *statement, ub4 mode);
int crx_oci_statement_cancel(crx_oci_statement *statement);
void crx_oci_statement_free(crx_oci_statement *statement);
int crx_oci_bind_pre_exec(zval *data, void *result);
int crx_oci_bind_post_exec(zval *data);
int crx_oci_bind_by_name(crx_oci_statement *statement, char *name, size_t name_len, zval *var, crex_long maxlength, ub2 type);
sb4 crx_oci_bind_in_callback(dvoid *ictxp, OCIBind *bindp, ub4 iter, ub4 index, dvoid **bufpp, ub4 *alenp, ub1 *piecep, dvoid **indpp);
sb4 crx_oci_bind_out_callback(dvoid *octxp, OCIBind *bindp, ub4 iter, ub4 index, dvoid **bufpp, ub4 **alenpp, ub1 *piecep, dvoid **indpp, ub2 **rcodepp);
crx_oci_out_column *crx_oci_statement_get_column_helper(INTERNAL_FUNCTION_PARAMETERS, int need_data);
int crx_oci_cleanup_pre_fetch(zval *data);
int crx_oci_statement_get_type(crx_oci_statement *statement, ub2 *type);
int crx_oci_statement_get_numrows(crx_oci_statement *statement, ub4 *numrows);
int crx_oci_bind_array_by_name(crx_oci_statement *statement, char *name, size_t name_len, zval *var, crex_long max_table_length, crex_long maxlength, crex_long type);
crx_oci_bind *crx_oci_bind_array_helper_number(zval *var, crex_long max_table_length);
crx_oci_bind *crx_oci_bind_array_helper_double(zval *var, crex_long max_table_length);
crx_oci_bind *crx_oci_bind_array_helper_string(zval *var, crex_long max_table_length, crex_long maxlength);
crx_oci_bind *crx_oci_bind_array_helper_date(zval *var, crex_long max_table_length, crx_oci_connection *connection);

/* }}} */

CREX_BEGIN_MODULE_GLOBALS(oci) /* {{{ Module globals */
	sb4			 errcode;						/* global last ORA- error number. Used when connect fails, for example */
	OCIError	*err;							/* global error handle */

	crex_long		 max_persistent;				/* maximum number of persistent connections per process */
	crex_long		 num_persistent;				/* number of existing persistent connections */
	crex_long		 num_links;						/* non-persistent + persistent connections */
	crex_long		 num_statements;				/* number of statements open */
	crex_long		 ping_interval;					/* time interval between pings */
	crex_long		 persistent_timeout;			/* time period after which idle persistent connection is considered expired */
	crex_long		 statement_cache_size;			/* statement cache size. used with 9i+ clients only*/
	crex_long		 default_prefetch;				/* default prefetch setting */
	crex_long	 	 prefetch_lob_size;				/* amount of LOB data to read when initially getting a LOB locator */
	bool	 privileged_connect;			/* privileged connect flag (On/Off) */
	bool	 old_oci_close_semantics;		/* old_oci_close_semantics flag (to determine the way oci_close() should behave) */
	int			 shutdown;						/* in shutdown flag */

	OCIEnv		*env;							/* global environment handle */

	bool	 in_call;
	char		*connection_class;
	bool	 events;
	char		*edition;
CREX_END_MODULE_GLOBALS(oci) /* }}} */

/* {{{ transparent failover related prototypes */

int crx_oci_register_taf_callback(crx_oci_connection *connection, zval *callback);
int crx_oci_unregister_taf_callback(crx_oci_connection *connection);

/* }}} */

#ifdef ZTS
#define OCI_G(v) TSRMG(oci_globals_id, crex_oci_globals *, v)
#else
#define OCI_G(v) (oci_globals.v)
#endif

/* Allow install from PECL on CRX < 7.3 */
#ifndef GC_ADDREF
# define GC_ADDREF(p) (++GC_REFCOUNT(p))
#endif
#ifndef GC_DELREF
# define GC_DELREF(p) (GC_REFCOUNT(p)--)
#endif

CREX_EXTERN_MODULE_GLOBALS(oci)

# endif /* !CRX_OCI8_INT_H */
#else /* !HAVE_OCI8 */

# define oci8_module_ptr NULL

#endif /* HAVE_OCI8 */
