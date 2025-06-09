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
  | Authors: Rasmus Lerdorf <rasmus@crx.net>                             |
  |          Mike Jackson <mhjack@tscnet.com>                            |
  |          Steven Lawrance <slawrance@technologist.com>                |
  |          Harrie Hazewinkel <harrie@lisanza.net>                      |
  |          Johann Hanne <jonny@nurfuerspam.de>                         |
  |          Boris Lytockin <lytboris@gmail.com>                         |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_SNMP_H
#define CRX_SNMP_H

#define CRX_SNMP_VERSION CRX_VERSION

#ifdef HAVE_SNMP

#ifndef DLEXPORT
#define DLEXPORT
#endif

extern crex_module_entry snmp_module_entry;
#define snmp_module_ptr &snmp_module_entry

#ifdef ZTS
#include "TSRM.h"
#endif

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

CRX_MINIT_FUNCTION(snmp);
CRX_MSHUTDOWN_FUNCTION(snmp);
CRX_MINFO_FUNCTION(snmp);

typedef struct _crx_snmp_object {
	struct snmp_session *session;
	int max_oids;
	int valueretrieval;
	int quick_print;
	int enum_print;
	int oid_output_format;
	int snmp_errno;
	int oid_increasing_check;
	int exceptions_enabled;
	char snmp_errstr[256];
	crex_object zo;
} crx_snmp_object;

static inline crx_snmp_object *crx_snmp_fetch_object(crex_object *obj) {
	return (crx_snmp_object *)((char*)(obj) - XtOffsetOf(crx_snmp_object, zo));
}

#define C_SNMP_P(zv) crx_snmp_fetch_object(C_OBJ_P((zv)))

typedef int (*crx_snmp_read_t)(crx_snmp_object *snmp_object, zval *retval);
typedef int (*crx_snmp_write_t)(crx_snmp_object *snmp_object, zval *newval);

typedef struct _ptp_snmp_prop_handler {
	const char *name;
	size_t name_length;
	crx_snmp_read_t read_func;
	crx_snmp_write_t write_func;
} crx_snmp_prop_handler;

typedef struct _snmpobjarg {
	char *oid;
	char type;
	char *value;
	oid  name[MAX_OID_LEN];
	size_t name_length;
} snmpobjarg;

CREX_BEGIN_MODULE_GLOBALS(snmp)
	int valueretrieval;
CREX_END_MODULE_GLOBALS(snmp)

#ifdef ZTS
#define SNMP_G(v) TSRMG(snmp_globals_id, crex_snmp_globals *, v)
#else
#define SNMP_G(v) (snmp_globals.v)
#endif

#define SNMP_VALUE_LIBRARY	(0 << 0)
#define SNMP_VALUE_PLAIN	(1 << 0)
#define SNMP_VALUE_OBJECT	(1 << 1)

#define CRX_SNMP_ERRNO_NOERROR			0
#define CRX_SNMP_ERRNO_GENERIC			(1 << 1)
#define CRX_SNMP_ERRNO_TIMEOUT			(1 << 2)
#define CRX_SNMP_ERRNO_ERROR_IN_REPLY		(1 << 3)
#define CRX_SNMP_ERRNO_OID_NOT_INCREASING	(1 << 4)
#define CRX_SNMP_ERRNO_OID_PARSING_ERROR	(1 << 5)
#define CRX_SNMP_ERRNO_MULTIPLE_SET_QUERIES	(1 << 6)
#define CRX_SNMP_ERRNO_ANY	( \
		CRX_SNMP_ERRNO_GENERIC | \
		CRX_SNMP_ERRNO_TIMEOUT | \
		CRX_SNMP_ERRNO_ERROR_IN_REPLY | \
		CRX_SNMP_ERRNO_OID_NOT_INCREASING | \
		CRX_SNMP_ERRNO_OID_PARSING_ERROR | \
		CRX_SNMP_ERRNO_MULTIPLE_SET_QUERIES | \
		CRX_SNMP_ERRNO_NOERROR \
	)

#else

#define snmp_module_ptr NULL

#endif /* HAVE_SNMP */

#define crxext_snmp_ptr snmp_module_ptr

#endif  /* CRX_SNMP_H */
