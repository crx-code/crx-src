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

#define CREX_INCLUDE_FULL_WINDOWS_HEADERS

#include "crx.h"
#ifdef strcasecmp
# undef strcasecmp
#endif
#ifdef strncasecmp
# undef strncasecmp
#endif
#include "crx_ini.h"
#include "crx_apache.h"

#include "apr_strings.h"
#include "ap_config.h"
#include "util_filter.h"
#include "httpd.h"
#include "http_config.h"
#include "http_request.h"
#include "http_core.h"
#include "http_protocol.h"
#include "http_log.h"
#include "http_main.h"
#include "util_script.h"
#include "http_core.h"

#ifdef CRX_AP_DEBUG
#define crxapdebug(a) fprintf a
#else
#define crxapdebug(a)
#endif

typedef struct {
	HashTable config;
} crx_conf_rec;

typedef struct {
	char *value;
	size_t value_len;
	char status;
	char htaccess;
} crx_dir_entry;

static const char *real_value_hnd(cmd_parms *cmd, void *dummy, const char *name, const char *value, int status)
{
	crx_conf_rec *d = dummy;
	crx_dir_entry e;

	crxapdebug((stderr, "Getting %s=%s for %p (%d)\n", name, value, dummy, crex_hash_num_elements(&d->config)));

	if (!strncasecmp(value, "none", sizeof("none"))) {
		value = "";
	}

	e.value = apr_pstrdup(cmd->pool, value);
	e.value_len = strlen(value);
	e.status = status;
	e.htaccess = ((cmd->override & (RSRC_CONF|ACCESS_CONF)) == 0);

	crex_hash_str_update_mem(&d->config, (char *) name, strlen(name), &e, sizeof(e));
	return NULL;
}

static const char *crx_apache_value_handler(cmd_parms *cmd, void *dummy, const char *name, const char *value)
{
	return real_value_hnd(cmd, dummy, name, value, CRX_INI_PERDIR);
}

static const char *crx_apache_admin_value_handler(cmd_parms *cmd, void *dummy, const char *name, const char *value)
{
	return real_value_hnd(cmd, dummy, name, value, CRX_INI_SYSTEM);
}

static const char *real_flag_hnd(cmd_parms *cmd, void *dummy, const char *arg1, const char *arg2, int status)
{
	char bool_val[2];

	if (!strcasecmp(arg2, "On") || (arg2[0] == '1' && arg2[1] == '\0')) {
		bool_val[0] = '1';
	} else {
		bool_val[0] = '0';
	}
	bool_val[1] = 0;

	return real_value_hnd(cmd, dummy, arg1, bool_val, status);
}

static const char *crx_apache_flag_handler(cmd_parms *cmd, void *dummy, const char *name, const char *value)
{
	return real_flag_hnd(cmd, dummy, name, value, CRX_INI_PERDIR);
}

static const char *crx_apache_admin_flag_handler(cmd_parms *cmd, void *dummy, const char *name, const char *value)
{
	return real_flag_hnd(cmd, dummy, name, value, CRX_INI_SYSTEM);
}

static const char *crx_apache_crxini_set(cmd_parms *cmd, void *mconfig, const char *arg)
{
	if (apache2_crx_ini_path_override) {
		return "Only first CRXINIDir directive honored per configuration tree - subsequent ones ignored";
	}
	apache2_crx_ini_path_override = ap_server_root_relative(cmd->pool, arg);
	return NULL;
}

static bool should_overwrite_per_dir_entry(HashTable *target_ht, zval *zv, crex_hash_key *hash_key, void *pData)
{
	crx_dir_entry *new_per_dir_entry = C_PTR_P(zv);
	crx_dir_entry *orig_per_dir_entry;

	if ((orig_per_dir_entry = crex_hash_find_ptr(target_ht, hash_key->key)) == NULL) {
		return 1; /* does not exist in dest, copy from source */
	}

	if (new_per_dir_entry->status >= orig_per_dir_entry->status) {
		/* use new entry */
		crxapdebug((stderr, "ADDING/OVERWRITING %s (%d vs. %d)\n", ZSTR_VAL(hash_key->key), new_per_dir_entry->status, orig_per_dir_entry->status));
		return 1;
	} else {
		return 0;
	}
}

void config_entry_ctor(zval *zv)
{
	crx_dir_entry *pe = (crx_dir_entry*)C_PTR_P(zv);
	crx_dir_entry *npe = malloc(sizeof(crx_dir_entry));

	memcpy(npe, pe, sizeof(crx_dir_entry));
	ZVAL_PTR(zv, npe);
}

void *merge_crx_config(apr_pool_t *p, void *base_conf, void *new_conf)
{
	crx_conf_rec *d = base_conf, *e = new_conf, *n = NULL;
#ifdef ZTS
	crex_string *str;
	zval *data;
#endif

	n = create_crx_config(p, "merge_crx_config");
	/* copy old config */
#ifdef ZTS
	CREX_HASH_MAP_FOREACH_STR_KEY_VAL(&d->config, str, data) {
		crex_string *key;
		zval *new_entry;

		/* Avoid sharing the non interned string among threads. */
		key = crex_string_dup(str, 1);

		new_entry = crex_hash_add(&n->config, key, data);

		config_entry_ctor(new_entry);
	} CREX_HASH_FOREACH_END();
#else
	crex_hash_copy(&n->config, &d->config, config_entry_ctor);
#endif
	/* merge new config */
	crxapdebug((stderr, "Merge dir (%p)+(%p)=(%p)\n", base_conf, new_conf, n));
	crex_hash_merge_ex(&n->config, &e->config, config_entry_ctor, should_overwrite_per_dir_entry, NULL);
	return n;
}

char *get_crx_config(void *conf, char *name, size_t name_len)
{
	crx_conf_rec *d = conf;
	crx_dir_entry *pe;

	if ((pe = crex_hash_str_find_ptr(&d->config, name, name_len)) != NULL) {
		return pe->value;
	}

	return "";
}

void apply_config(void *dummy)
{
	crx_conf_rec *d = dummy;
	crex_string *str;
	crx_dir_entry *data;

	CREX_HASH_MAP_FOREACH_STR_KEY_PTR(&d->config, str, data) {
		crxapdebug((stderr, "APPLYING (%s)(%s)\n", ZSTR_VAL(str), data->value));
		if (crex_alter_ini_entry_chars(str, data->value, data->value_len, data->status, data->htaccess?CRX_INI_STAGE_HTACCESS:CRX_INI_STAGE_ACTIVATE) == FAILURE) {
			crxapdebug((stderr, "..FAILED\n"));
		}
	} CREX_HASH_FOREACH_END();
}

const command_rec crx_dir_cmds[] =
{
	AP_INIT_TAKE2("crx_value", crx_apache_value_handler, NULL, OR_OPTIONS, "CRX Value Modifier"),
	AP_INIT_TAKE2("crx_flag", crx_apache_flag_handler, NULL, OR_OPTIONS, "CRX Flag Modifier"),
	AP_INIT_TAKE2("crx_admin_value", crx_apache_admin_value_handler, NULL, ACCESS_CONF|RSRC_CONF, "CRX Value Modifier (Admin)"),
	AP_INIT_TAKE2("crx_admin_flag", crx_apache_admin_flag_handler, NULL, ACCESS_CONF|RSRC_CONF, "CRX Flag Modifier (Admin)"),
	AP_INIT_TAKE1("CRXINIDir", crx_apache_crxini_set, NULL, RSRC_CONF, "Directory containing the crx.ini file"),
	{NULL}
};

static apr_status_t destroy_crx_config(void *data)
{
	crx_conf_rec *d = data;

	crxapdebug((stderr, "Destroying config %p\n", data));
	crex_hash_destroy(&d->config);

	return APR_SUCCESS;
}

static void config_entry_dtor(zval *zv)
{
	free((crx_dir_entry*)C_PTR_P(zv));
}

void *create_crx_config(apr_pool_t *p, char *dummy)
{
	crx_conf_rec *newx = (crx_conf_rec *) apr_pcalloc(p, sizeof(*newx));

	crxapdebug((stderr, "Creating new config (%p) for %s\n", newx, dummy));
	crex_hash_init(&newx->config, 0, NULL, config_entry_dtor, 1);
	apr_pool_cleanup_register(p, newx, destroy_crx_config, apr_pool_cleanup_null);
	return (void *) newx;
}
