	/* (c) 2007,2008 Andrei Nigmatulin */

#include "fpm_config.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "crx.h"
#include "crx_main.h"
#include "crx_ini.h"
#include "ext/standard/dl.h"

#include "fastcgi.h"

#include "fpm.h"
#include "fpm_crx.h"
#include "fpm_cleanup.h"
#include "fpm_worker_pool.h"
#include "zlog.h"

static char **limit_extensions = NULL;

static int fpm_crx_crex_ini_alter_master(char *name, int name_length, char *new_value, int new_value_length, int mode, int stage) /* {{{ */
{
	crex_ini_entry *ini_entry;
	crex_string *duplicate;

	if ((ini_entry = crex_hash_str_find_ptr(EG(ini_directives), name, name_length)) == NULL) {
		return FAILURE;
	}

	duplicate = crex_string_init(new_value, new_value_length, 1);

	if (!ini_entry->on_modify
			|| ini_entry->on_modify(ini_entry, duplicate,
				ini_entry->mh_arg1, ini_entry->mh_arg2, ini_entry->mh_arg3, stage) == SUCCESS) {
		ini_entry->value = duplicate;
		/* when mode == CREX_INI_USER keep unchanged to allow CREX_INI_PERDIR (.user.ini) */
		if (mode == CREX_INI_SYSTEM) {
			ini_entry->modifiable = mode;
		}
	} else {
		crex_string_release_ex(duplicate, 1);
	}

	return SUCCESS;
}
/* }}} */

static void fpm_crx_disable(char *value, int (*crex_disable)(const char *, size_t)) /* {{{ */
{
	char *s = 0, *e = value;

	while (*e) {
		switch (*e) {
			case ' ':
			case ',':
				if (s) {
					*e = '\0';
					crex_disable(s, e - s);
					s = 0;
				}
				break;
			default:
				if (!s) {
					s = e;
				}
				break;
		}
		e++;
	}

	if (s) {
		crex_disable(s, e - s);
	}
}
/* }}} */

#define FPM_CRX_INI_ALTERING_ERROR   -1
#define FPM_CRX_INI_APPLIED          1
#define FPM_CRX_INI_EXTENSION_FAILED 0
#define FPM_CRX_INI_EXTENSION_LOADED 2

int fpm_crx_apply_defines_ex(struct key_value_s *kv, int mode) /* {{{ */
{

	char *name = kv->key;
	char *value = kv->value;
	int name_len = strlen(name);
	int value_len = strlen(value);

	if (!strcmp(name, "extension") && *value) {
		zval zv;
		crex_interned_strings_switch_storage(0);
		crx_dl(value, MODULE_PERSISTENT, &zv, 1);
		crex_interned_strings_switch_storage(1);
		return C_TYPE(zv) == IS_TRUE ? FPM_CRX_INI_EXTENSION_LOADED : FPM_CRX_INI_EXTENSION_FAILED;
	}

	if (fpm_crx_crex_ini_alter_master(name, name_len, value, value_len, mode, CRX_INI_STAGE_ACTIVATE) == FAILURE) {
		return FPM_CRX_INI_ALTERING_ERROR;
	}

	if (!strcmp(name, "disable_functions") && *value) {
		crex_disable_functions(value);
		return FPM_CRX_INI_APPLIED;
	}

	if (!strcmp(name, "disable_classes") && *value) {
		char *v = strdup(value);
		PG(disable_classes) = v;
		fpm_crx_disable(v, crex_disable_class);
		return FPM_CRX_INI_APPLIED;
	}

	return FPM_CRX_INI_APPLIED;
}
/* }}} */

static int fpm_crx_apply_defines(struct fpm_worker_pool_s *wp) /* {{{ */
{
	struct key_value_s *kv;
	int apply_result;
	bool extension_loaded = false;

	for (kv = wp->config->crx_values; kv; kv = kv->next) {
		apply_result = fpm_crx_apply_defines_ex(kv, CREX_INI_USER);
		if (apply_result == FPM_CRX_INI_ALTERING_ERROR) {
			zlog(ZLOG_ERROR, "Unable to set crx_value '%s'", kv->key);
		} else if (apply_result == FPM_CRX_INI_EXTENSION_LOADED) {
			extension_loaded = true;
		}
	}

	for (kv = wp->config->crx_admin_values; kv; kv = kv->next) {
		apply_result = fpm_crx_apply_defines_ex(kv, CREX_INI_SYSTEM);
		if (apply_result == FPM_CRX_INI_ALTERING_ERROR) {
			zlog(ZLOG_ERROR, "Unable to set crx_admin_value '%s'", kv->key);
		} else if (apply_result == FPM_CRX_INI_EXTENSION_LOADED) {
			extension_loaded = true;
		}
	}

	if (extension_loaded) {
		crex_collect_module_handlers();
	}

	return 0;
}
/* }}} */

static int fpm_crx_set_allowed_clients(struct fpm_worker_pool_s *wp) /* {{{ */
{
	if (wp->listen_address_domain == FPM_AF_INET) {
		fcgi_set_allowed_clients(wp->config->listen_allowed_clients);
	}
	return 0;
}
/* }}} */

#if 0 /* Comment out this non used function. It could be used later. */
static int fpm_crx_set_fcgi_mgmt_vars(struct fpm_worker_pool_s *wp) /* {{{ */
{
	char max_workers[10 + 1]; /* 4294967295 */
	int len;

	len = sprintf(max_workers, "%u", (unsigned int) wp->config->pm_max_children);

	fcgi_set_mgmt_var("FCGI_MAX_CONNS", sizeof("FCGI_MAX_CONNS")-1, max_workers, len);
	fcgi_set_mgmt_var("FCGI_MAX_REQS",  sizeof("FCGI_MAX_REQS")-1,  max_workers, len);
	return 0;
}
/* }}} */
#endif

char *fpm_crx_script_filename(void)
{
	return SG(request_info).path_translated;
}

char *fpm_crx_request_uri(void)
{
	return (char *) SG(request_info).request_uri;
}

char *fpm_crx_request_method(void)
{
	return (char *) SG(request_info).request_method;
}

char *fpm_crx_query_string(void)
{
	return SG(request_info).query_string;
}

char *fpm_crx_auth_user(void)
{
	return SG(request_info).auth_user;
}

size_t fpm_crx_content_length(void)
{
	return SG(request_info).content_length;
}

static void fpm_crx_cleanup(int which, void *arg) /* {{{ */
{
	crx_module_shutdown();
	sapi_shutdown();
	if (limit_extensions) {
		fpm_worker_pool_free_limit_extensions(limit_extensions);
	}
}
/* }}} */

void fpm_crx_soft_quit(void)
{
	fcgi_terminate();
}

int fpm_crx_init_main(void)
{
	if (0 > fpm_cleanup_add(FPM_CLEANUP_PARENT, fpm_crx_cleanup, 0)) {
		return -1;
	}
	return 0;
}

int fpm_crx_init_child(struct fpm_worker_pool_s *wp) /* {{{ */
{
	if (0 > fpm_crx_apply_defines(wp) ||
		0 > fpm_crx_set_allowed_clients(wp)) {
		return -1;
	}

	if (wp->limit_extensions) {
		/* Take ownership of limit_extensions. */
		limit_extensions = wp->limit_extensions;
		wp->limit_extensions = NULL;
	}
	return 0;
}
/* }}} */

int fpm_crx_limit_extensions(char *path) /* {{{ */
{
	char **p;
	size_t path_len;

	if (!path || !limit_extensions) {
		return 0; /* allowed by default */
	}

	p = limit_extensions;
	path_len = strlen(path);
	while (p && *p) {
		size_t ext_len = strlen(*p);
		if (path_len > ext_len) {
			char *path_ext = path + path_len - ext_len;
			if (strcmp(*p, path_ext) == 0) {
				return 0; /* allow as the extension has been found */
			}
		}
		p++;
	}


	zlog(ZLOG_NOTICE, "Access to the script '%s' has been denied (see security.limit_extensions)", path);
	return 1; /* extension not found: not allowed  */
}
/* }}} */

bool fpm_crx_is_key_in_table(crex_string *table, const char *key, size_t key_len) /* {{{ */
{
	zval *data;
	crex_string *str;

	CREX_ASSERT(table);
	CREX_ASSERT(key);

	/* inspired from ext/standard/info.c */

	crex_is_auto_global(table);

	/* find the table and ensure it's an array */
	data = crex_hash_find(&EG(symbol_table), table);
	if (!data || C_TYPE_P(data) != IS_ARRAY) {
		return NULL;
	}

	CREX_HASH_FOREACH_STR_KEY(C_ARRVAL_P(data), str) {
		if (str && crex_string_equals_cstr(str, key, key_len)) {
			return true;
		}
	} CREX_HASH_FOREACH_END();

	return false;
}
/* }}} */
