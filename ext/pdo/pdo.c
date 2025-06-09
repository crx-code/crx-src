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
  | Author: Wez Furlong <wez@crx.net>                                    |
  |         Marcus Boerger <helly@crx.net>                               |
  |         Sterling Hughes <sterling@crx.net>                           |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include "crx.h"
#include "crx_ini.h"
#include "ext/standard/info.h"
#include "crx_pdo.h"
#include "crx_pdo_driver.h"
#include "crx_pdo_int.h"
#include "crex_exceptions.h"
#include "ext/spl/spl_exceptions.h"
#include "pdo_arginfo.h"

crex_class_entry *pdo_dbh_ce, *pdo_dbstmt_ce, *pdo_row_ce;

/* for exceptional circumstances */
crex_class_entry *pdo_exception_ce;

/* True global resources - no need for thread safety here */

/* the registry of PDO drivers */
HashTable pdo_driver_hash;

/* we use persistent resources for the driver connection stuff */
static int le_ppdo;

int crx_pdo_list_entry(void) /* {{{ */
{
	return le_ppdo;
}
/* }}} */

PDO_API crex_class_entry *crx_pdo_get_dbh_ce(void) /* {{{ */
{
	return pdo_dbh_ce;
}
/* }}} */

PDO_API crex_class_entry *crx_pdo_get_exception(void) /* {{{ */
{
	return pdo_exception_ce;
}
/* }}} */

/* {{{ Return array of available PDO drivers */
CRX_FUNCTION(pdo_drivers)
{
	pdo_driver_t *pdriver;

	CREX_PARSE_PARAMETERS_NONE();

	array_init(return_value);

	CREX_HASH_MAP_FOREACH_PTR(&pdo_driver_hash, pdriver) {
		add_next_index_stringl(return_value, pdriver->driver_name, pdriver->driver_name_len);
	} CREX_HASH_FOREACH_END();
}
/* }}} */

/* {{{ pdo_functions[] */
static const crex_module_dep pdo_deps[] = {
	CREX_MOD_REQUIRED("spl")
	CREX_MOD_END
};
/* }}} */

/* {{{ pdo_module_entry */
crex_module_entry pdo_module_entry = {
	STANDARD_MODULE_HEADER_EX, NULL,
	pdo_deps,
	"PDO",
	ext_functions,
	CRX_MINIT(pdo),
	CRX_MSHUTDOWN(pdo),
	NULL,
	NULL,
	CRX_MINFO(pdo),
	CRX_PDO_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

/* TODO: visit persistent handles: for each persistent statement handle,
 * remove bound parameter associations */

#ifdef COMPILE_DL_PDO
CREX_GET_MODULE(pdo)
#endif

PDO_API crex_result crx_pdo_register_driver(const pdo_driver_t *driver) /* {{{ */
{
	if (driver->api_version != PDO_DRIVER_API) {
		crex_error(E_ERROR, "PDO: driver %s requires PDO API version " CREX_ULONG_FMT "; this is PDO version %d",
			driver->driver_name, driver->api_version, PDO_DRIVER_API);
		return FAILURE;
	}
	if (!crex_hash_str_exists(&module_registry, "pdo", sizeof("pdo") - 1)) {
		crex_error(E_ERROR, "You MUST load PDO before loading any PDO drivers");
		return FAILURE;	/* NOTREACHED */
	}

	return crex_hash_str_add_ptr(&pdo_driver_hash, driver->driver_name, driver->driver_name_len, (void*)driver) != NULL ? SUCCESS : FAILURE;
}
/* }}} */

PDO_API void crx_pdo_unregister_driver(const pdo_driver_t *driver) /* {{{ */
{
	if (!crex_hash_str_exists(&module_registry, "pdo", sizeof("pdo") - 1)) {
		return;
	}

	crex_hash_str_del(&pdo_driver_hash, driver->driver_name, driver->driver_name_len);
}
/* }}} */

pdo_driver_t *pdo_find_driver(const char *name, int namelen) /* {{{ */
{
	return crex_hash_str_find_ptr(&pdo_driver_hash, name, namelen);
}
/* }}} */

PDO_API int crx_pdo_parse_data_source(const char *data_source, crex_ulong data_source_len, struct pdo_data_src_parser *parsed, int nparams) /* {{{ */
{
	crex_ulong i;
	int j;
	int valstart = -1;
	int semi = -1;
	int optstart = 0;
	int nlen;
	int n_matches = 0;
	int n_semicolumns = 0;

	i = 0;
	while (i < data_source_len) {
		/* looking for NAME= */

		if (data_source[i] == '\0') {
			break;
		}

		if (data_source[i] != '=') {
			++i;
			continue;
		}

		valstart = ++i;

		/* now we're looking for VALUE; or just VALUE<NUL> */
		semi = -1;
		n_semicolumns = 0;
		while (i < data_source_len) {
			if (data_source[i] == '\0') {
				semi = i++;
				break;
			}
			if (data_source[i] == ';') {
				if ((i + 1 >= data_source_len) || data_source[i+1] != ';') {
					semi = i++;
					break;
				} else {
					n_semicolumns++;
					i += 2;
					continue;
				}
			}
			++i;
		}

		if (semi == -1) {
			semi = i;
		}

		/* find the entry in the array */
		nlen = valstart - optstart - 1;
		for (j = 0; j < nparams; j++) {
			if (0 == strncmp(data_source + optstart, parsed[j].optname, nlen) && parsed[j].optname[nlen] == '\0') {
				/* got a match */
				if (parsed[j].freeme) {
					efree(parsed[j].optval);
				}

				if (n_semicolumns == 0) {
					parsed[j].optval = estrndup(data_source + valstart, semi - valstart - n_semicolumns);
				} else {
					int vlen = semi - valstart;
					const char *orig_val = data_source + valstart;
					char *new_val  = (char *) emalloc(vlen - n_semicolumns + 1);

					parsed[j].optval = new_val;

					while (vlen && *orig_val) {
						*new_val = *orig_val;
						new_val++;

						if (*orig_val == ';') {
							orig_val+=2;
							vlen-=2;
						} else {
							orig_val++;
							vlen--;
						}
					}
					*new_val = '\0';
				}

				parsed[j].freeme = 1;
				++n_matches;
				break;
			}
		}

		while (i < data_source_len && isspace(data_source[i])) {
			i++;
		}

		optstart = i;
	}

	return n_matches;
}
/* }}} */

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(pdo)
{
	pdo_sqlstate_init_error_table();

	crex_hash_init(&pdo_driver_hash, 0, NULL, NULL, 1);

	le_ppdo = crex_register_list_destructors_ex(NULL, crx_pdo_pdbh_dtor,
		"PDO persistent database", module_number);

	pdo_exception_ce = register_class_PDOException(spl_ce_RuntimeException);

	pdo_dbh_init(module_number);
	pdo_stmt_init();

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION */
CRX_MSHUTDOWN_FUNCTION(pdo)
{
	crex_hash_destroy(&pdo_driver_hash);
	pdo_sqlstate_fini_error_table();
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(pdo)
{
	char *drivers = NULL, *ldrivers = estrdup("");
	pdo_driver_t *pdriver;

	crx_info_print_table_start();
	crx_info_print_table_row(2, "PDO support", "enabled");

	CREX_HASH_MAP_FOREACH_PTR(&pdo_driver_hash, pdriver) {
		spprintf(&drivers, 0, "%s, %s", ldrivers, pdriver->driver_name);
		efree(ldrivers);
		ldrivers = drivers;
	} CREX_HASH_FOREACH_END();

	crx_info_print_table_row(2, "PDO drivers", drivers ? drivers + 2 : "");

	if (drivers) {
		efree(drivers);
	} else {
		efree(ldrivers);
	}

	crx_info_print_table_end();

}
/* }}} */
