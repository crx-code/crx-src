	/* (c) 2007,2008 Andrei Nigmatulin */

#ifndef FPM_CRX_H
#define FPM_CRX_H 1

#include <TSRM.h>

#include "crx.h"
#include "build-defs.h" /* for CRX_ defines */
#include "fpm/fpm_conf.h"

#define FPM_CRX_INI_TO_EXPAND \
	{ \
		"error_log", \
		"extension_dir", \
		"sendmail_path", \
		"session.cookie_path", \
		"session_pgsql.sem_file_name", \
		"soap.wsdl_cache_dir", \
		"uploadprogress.file.filename_template", \
		"xdebug.output_dir", \
		"xdebug.profiler_output_dir", \
		"xdebug.trace_output_dir", \
		"xmms.path", \
		"axis2.client_home", \
		"blenc.key_file", \
		"coin_acceptor.device", \
		NULL \
	}

struct fpm_worker_pool_s;

int fpm_crx_init_child(struct fpm_worker_pool_s *wp);
char *fpm_crx_script_filename(void);
char *fpm_crx_request_uri(void);
char *fpm_crx_request_method(void);
char *fpm_crx_query_string(void);
char *fpm_crx_auth_user(void);
size_t fpm_crx_content_length(void);
void fpm_crx_soft_quit(void);
int fpm_crx_init_main(void);
int fpm_crx_apply_defines_ex(struct key_value_s *kv, int mode);
int fpm_crx_limit_extensions(char *path);
bool fpm_crx_is_key_in_table(crex_string *table, const char *key, size_t key_len);

#endif
