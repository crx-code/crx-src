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
   | Author: Sascha Schumann <sascha@schumann.cx>                         |
   +----------------------------------------------------------------------+
 */

#ifndef CRX_APACHE_H
#define CRX_APACHE_H

#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_log.h"

#include "crx.h"
#include "main/crx_streams.h"

/* Enable per-module logging in Apache 2.4+ */
#ifdef APLOG_USE_MODULE
APLOG_USE_MODULE(crx);
#endif

/* Declare this so we can get to it from outside the sapi_apache2.c file */
extern module AP_MODULE_DECLARE_DATA crx_module;

/* A way to specify the location of the crx.ini dir in an apache directive */
extern char *apache2_crx_ini_path_override;

/* The server_context used by CRX */
typedef struct crx_struct {
	int state;
	request_rec *r;
	apr_bucket_brigade *brigade;
	/* stat structure of the current file */
	crex_stat_t finfo;
	/* Whether or not we've processed CRX in the output filters yet. */
	int request_processed;
	/* final content type */
	char *content_type;
} crx_struct;

void *merge_crx_config(apr_pool_t *p, void *base_conf, void *new_conf);
void *create_crx_config(apr_pool_t *p, char *dummy);
char *get_crx_config(void *conf, char *name, size_t name_len);
void apply_config(void *);
extern const command_rec crx_dir_cmds[];
void crx_ap2_register_hook(apr_pool_t *p);

#define APR_ARRAY_FOREACH_OPEN(arr, key, val) 		\
{													\
	apr_table_entry_t *elts;						\
	int i;											\
	elts = (apr_table_entry_t *) arr->elts;			\
	for (i = 0; i < arr->nelts; i++) {				\
		key = elts[i].key;							\
		val = elts[i].val;

#define APR_ARRAY_FOREACH_CLOSE() }}

typedef struct {
	bool engine;
	bool xbithack;
	bool last_modified;
} crx_apache2_info_struct;

extern crex_module_entry apache2_module_entry;

#ifdef ZTS
extern int crx_apache2_info_id;
#define AP2(v) CREX_TSRMG(crx_apache2_info_id, crx_apache2_info_struct *, v)
CREX_TSRMLS_CACHE_EXTERN()
#else
extern crx_apache2_info_struct crx_apache2_info;
#define AP2(v) (crx_apache2_info.v)
#endif

/* fix for gcc4 visibility patch */
#ifndef CRX_WIN32
# undef AP_MODULE_DECLARE_DATA
# define AP_MODULE_DECLARE_DATA CRXAPI
#endif

#endif /* CRX_APACHE_H */
