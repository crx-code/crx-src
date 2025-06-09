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
   | Authors: Derick Rethans <derick@derickrethans.nl>                    |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_DATE_H
#define CRX_DATE_H

#include "lib/timelib.h"
#include "Crex/crex_hash.h"

#include "crx_version.h"
#define CRX_DATE_VERSION CRX_VERSION

extern crex_module_entry date_module_entry;
#define crxext_date_ptr &date_module_entry

CRX_RINIT_FUNCTION(date);
CRX_RSHUTDOWN_FUNCTION(date);
CRX_MINIT_FUNCTION(date);
CRX_MSHUTDOWN_FUNCTION(date);
CRX_MINFO_FUNCTION(date);
CREX_MODULE_POST_CREX_DEACTIVATE_D(date);

typedef struct _crx_date_obj crx_date_obj;
typedef struct _crx_timezone_obj crx_timezone_obj;
typedef struct _crx_interval_obj crx_interval_obj;
typedef struct _crx_period_obj crx_period_obj;

struct _crx_date_obj {
	timelib_time *time;
	crex_object   std;
};

static inline crx_date_obj *crx_date_obj_from_obj(crex_object *obj) {
	return (crx_date_obj*)((char*)(obj) - XtOffsetOf(crx_date_obj, std));
}

#define C_CRXDATE_P(zv)  crx_date_obj_from_obj(C_OBJ_P((zv)))

struct _crx_timezone_obj {
	bool            initialized;
	int             type;
	union {
		timelib_tzinfo   *tz;         /* TIMELIB_ZONETYPE_ID */
		timelib_sll       utc_offset; /* TIMELIB_ZONETYPE_OFFSET */
		timelib_abbr_info z;          /* TIMELIB_ZONETYPE_ABBR */
	} tzi;
	crex_object std;
};

static inline crx_timezone_obj *crx_timezone_obj_from_obj(crex_object *obj) {
	return (crx_timezone_obj*)((char*)(obj) - XtOffsetOf(crx_timezone_obj, std));
}

#define C_CRXTIMEZONE_P(zv)  crx_timezone_obj_from_obj(C_OBJ_P((zv)))

#define CRX_DATE_CIVIL   1
#define CRX_DATE_WALL    2

struct _crx_interval_obj {
	timelib_rel_time *diff;
	int               civil_or_wall;
	bool              from_string;
	crex_string      *date_string;
	bool              initialized;
	crex_object       std;
};

static inline crx_interval_obj *crx_interval_obj_from_obj(crex_object *obj) {
	return (crx_interval_obj*)((char*)(obj) - XtOffsetOf(crx_interval_obj, std));
}

#define C_CRXINTERVAL_P(zv)  crx_interval_obj_from_obj(C_OBJ_P((zv)))

struct _crx_period_obj {
	timelib_time     *start;
	crex_class_entry *start_ce;
	timelib_time     *current;
	timelib_time     *end;
	timelib_rel_time *interval;
	int               recurrences;
	bool              initialized;
	bool              include_start_date;
	bool              include_end_date;
	crex_object       std;
};

static inline crx_period_obj *crx_period_obj_from_obj(crex_object *obj) {
	return (crx_period_obj*)((char*)(obj) - XtOffsetOf(crx_period_obj, std));
}

#define C_CRXPERIOD_P(zv)  crx_period_obj_from_obj(C_OBJ_P((zv)))

CREX_BEGIN_MODULE_GLOBALS(date)
	char                    *default_timezone;
	char                    *timezone;
	HashTable               *tzcache;
	timelib_error_container *last_errors;
CREX_END_MODULE_GLOBALS(date)

#define DATEG(v) CREX_MODULE_GLOBALS_ACCESSOR(date, v)

CRXAPI time_t crx_time(void);

/* Backwards compatibility wrapper */
CRXAPI crex_long crx_parse_date(const char *string, crex_long *now);
CRXAPI void crx_mktime(INTERNAL_FUNCTION_PARAMETERS, bool gmt);
CRXAPI int crx_idate(char format, time_t ts, bool localtime);

#define _crx_strftime crx_strftime

CRXAPI void crx_strftime(INTERNAL_FUNCTION_PARAMETERS, bool gm);
CRXAPI crex_string *crx_format_date(const char *format, size_t format_len, time_t ts, bool localtime);

/* Mechanism to set new TZ database */
CRXAPI void crx_date_set_tzdb(timelib_tzdb *tzdb);
CRXAPI timelib_tzinfo *get_timezone_info(void);

/* Grabbing CE's so that other exts can use the date objects too */
CRXAPI crex_class_entry *crx_date_get_date_ce(void);
CRXAPI crex_class_entry *crx_date_get_immutable_ce(void);
CRXAPI crex_class_entry *crx_date_get_interface_ce(void);
CRXAPI crex_class_entry *crx_date_get_timezone_ce(void);
CRXAPI crex_class_entry *crx_date_get_interval_ce(void);
CRXAPI crex_class_entry *crx_date_get_period_ce(void);

/* Functions for creating DateTime objects, and initializing them from a string */
#define CRX_DATE_INIT_CTOR   0x01
#define CRX_DATE_INIT_FORMAT 0x02

CRXAPI zval *crx_date_instantiate(crex_class_entry *pce, zval *object);
CRXAPI bool crx_date_initialize(crx_date_obj *dateobj, const char *time_str, size_t time_str_len, const char *format, zval *timezone_object, int flags);


#endif /* CRX_DATE_H */
