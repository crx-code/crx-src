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
   | Authors: Zeev Suraski <zeev@crx.net>                                 |
   |          Jouni Ahto <jouni.ahto@exdec.fi>                            |
   +----------------------------------------------------------------------+
 */

#ifndef CRX_PGSQL_H
#define CRX_PGSQL_H

#ifdef HAVE_PGSQL

#define CRX_PGSQL_API_VERSION 20140217

extern crex_module_entry pgsql_module_entry;
#define pgsql_module_ptr &pgsql_module_entry

#include "crx_version.h"
#define CRX_PGSQL_VERSION CRX_VERSION

#ifdef CRX_PGSQL_PRIVATE
#undef SOCKET_SIZE_TYPE
#include <libpq-fe.h>

#include <libpq/libpq-fs.h>
#ifdef CRX_WIN32
#undef CRX_PGSQL_API
#ifdef PGSQL_EXPORTS
#define CRX_PGSQL_API __declspec(dllexport)
#else
#define CRX_PGSQL_API __declspec(dllimport)
#endif
#else
# if defined(__GNUC__) && __GNUC__ >= 4
#  define CRX_PGSQL_API __attribute__ ((visibility("default")))
# else
#  define CRX_PGSQL_API
# endif
#endif

#ifdef HAVE_PGSQL_WITH_MULTIBYTE_SUPPORT
const char * pg_encoding_to_char(int encoding);
#endif

CRX_MINIT_FUNCTION(pgsql);
CRX_MSHUTDOWN_FUNCTION(pgsql);
CRX_RINIT_FUNCTION(pgsql);
CRX_RSHUTDOWN_FUNCTION(pgsql);
CRX_MINFO_FUNCTION(pgsql);

/* connection options - ToDo: Add async connection option */
#define PGSQL_CONNECT_FORCE_NEW     (1<<1)
#define PGSQL_CONNECT_ASYNC         (1<<2)
/* crx_pgsql_convert options */
#define PGSQL_CONV_IGNORE_DEFAULT   (1<<1)     /* Do not use DEFAULT value by removing field from returned array */
#define PGSQL_CONV_FORCE_NULL       (1<<2)     /* Convert to NULL if string is null string */
#define PGSQL_CONV_IGNORE_NOT_NULL  (1<<3)     /* Ignore NOT NULL constraints */
#define PGSQL_CONV_OPTS             (PGSQL_CONV_IGNORE_DEFAULT|PGSQL_CONV_FORCE_NULL|PGSQL_CONV_IGNORE_NOT_NULL)
/* crx_pgsql_insert/update/select/delete options */
#define PGSQL_DML_NO_CONV           (1<<8)     /* Do not call crx_pgsql_convert() */
#define PGSQL_DML_EXEC              (1<<9)     /* Execute query */
#define PGSQL_DML_ASYNC             (1<<10)    /* Do async query */
#define PGSQL_DML_STRING            (1<<11)    /* Return query string */
#define PGSQL_DML_ESCAPE            (1<<12)    /* No convert, but escape only */

/* exported functions */
CRX_PGSQL_API crex_result crx_pgsql_meta_data(PGconn *pg_link, const crex_string *table_name, zval *meta, bool extended);
CRX_PGSQL_API crex_result crx_pgsql_convert(PGconn *pg_link, const crex_string *table_name, const zval *values, zval *result, crex_ulong opt);
CRX_PGSQL_API crex_result crx_pgsql_insert(PGconn *pg_link, const crex_string *table, zval *values, crex_ulong opt, crex_string **sql);
CRX_PGSQL_API crex_result crx_pgsql_update(PGconn *pg_link, const crex_string *table, zval *values, zval *ids, crex_ulong opt , crex_string **sql);
CRX_PGSQL_API crex_result crx_pgsql_delete(PGconn *pg_link, const crex_string *table, zval *ids, crex_ulong opt, crex_string **sql);
CRX_PGSQL_API crex_result crx_pgsql_select(PGconn *pg_link, const crex_string *table, zval *ids, zval *ret_array, crex_ulong opt, long fetch_option, crex_string **sql );
CRX_PGSQL_API void crx_pgsql_result2array(PGresult *pg_result, zval *ret_array, long fetch_option);

/* internal functions */
static void crx_pgsql_do_connect(INTERNAL_FUNCTION_PARAMETERS,int persistent);
static void crx_pgsql_get_link_info(INTERNAL_FUNCTION_PARAMETERS, int entry_type);
static void crx_pgsql_get_result_info(INTERNAL_FUNCTION_PARAMETERS, int entry_type);
static void crx_pgsql_get_field_info(INTERNAL_FUNCTION_PARAMETERS, int entry_type);
static void crx_pgsql_data_info(INTERNAL_FUNCTION_PARAMETERS, int entry_type, bool nullable_row);
static void crx_pgsql_do_async(INTERNAL_FUNCTION_PARAMETERS,int entry_type);

static ssize_t crx_pgsql_fd_write(crx_stream *stream, const char *buf, size_t count);
static ssize_t crx_pgsql_fd_read(crx_stream *stream, char *buf, size_t count);
static int crx_pgsql_fd_close(crx_stream *stream, int close_handle);
static int crx_pgsql_fd_flush(crx_stream *stream);
static int crx_pgsql_fd_set_option(crx_stream *stream, int option, int value, void *ptrparam);
static int crx_pgsql_fd_cast(crx_stream *stream, int cast_as, void **ret);

typedef enum _crx_pgsql_data_type {
	/* boolean */
	PG_BOOL,
	/* number */
	PG_OID,
	PG_INT2,
	PG_INT4,
	PG_INT8,
	PG_FLOAT4,
	PG_FLOAT8,
	PG_NUMERIC,
	PG_MONEY,
	/* character */
	PG_TEXT,
	PG_CHAR,
	PG_VARCHAR,
	/* time and interval */
	PG_UNIX_TIME,
	PG_UNIX_TIME_INTERVAL,
	PG_DATE,
	PG_TIME,
	PG_TIME_WITH_TIMEZONE,
	PG_TIMESTAMP,
	PG_TIMESTAMP_WITH_TIMEZONE,
	PG_INTERVAL,
	/* binary */
	PG_BYTEA,
	/* network */
	PG_CIDR,
	PG_INET,
	PG_MACADDR,
	/* bit */
	PG_BIT,
	PG_VARBIT,
	/* geometoric */
	PG_LINE,
	PG_LSEG,
	PG_POINT,
	PG_BOX,
	PG_PATH,
	PG_POLYGON,
	PG_CIRCLE,
	/* unknown and system */
	PG_UNKNOWN
} crx_pgsql_data_type;

typedef struct pgsql_link_handle {
	PGconn *conn;
	crex_string *hash;
	HashTable *notices;
	bool persistent;
	crex_object std;
} pgsql_link_handle;

typedef struct pgLofp {
	PGconn *conn;
	int lofd;
	crex_object std;
} pgLofp;

typedef struct _crx_pgsql_result_handle {
	PGconn *conn;
	PGresult *result;
	int row;
	crex_object std;
} pgsql_result_handle;

typedef struct _crx_pgsql_notice {
	char *message;
	size_t len;
} crx_pgsql_notice;

static const crx_stream_ops crx_stream_pgsql_fd_ops = {
	crx_pgsql_fd_write,
	crx_pgsql_fd_read,
	crx_pgsql_fd_close,
	crx_pgsql_fd_flush,
	"PostgreSQL link",
	NULL, /* seek */
	crx_pgsql_fd_cast, /* cast */
	NULL, /* stat */
	crx_pgsql_fd_set_option
};

CREX_BEGIN_MODULE_GLOBALS(pgsql)
	crex_long num_links,num_persistent;
	crex_long max_links,max_persistent;
	crex_long allow_persistent;
	crex_long auto_reset_persistent;
	int ignore_notices,log_notices;
	crex_object *default_link; /* default link when connection is omitted */
	HashTable field_oids;
	HashTable table_oids;
	HashTable connections;
CREX_END_MODULE_GLOBALS(pgsql)

CREX_EXTERN_MODULE_GLOBALS(pgsql)
# define PGG(v) CREX_MODULE_GLOBALS_ACCESSOR(pgsql, v)

#if defined(ZTS) && defined(COMPILE_DL_PGSQL)
CREX_TSRMLS_CACHE_EXTERN()
#endif

#endif

#else

#define pgsql_module_ptr NULL

#endif

#define crxext_pgsql_ptr pgsql_module_ptr

#endif /* CRX_PGSQL_H */
