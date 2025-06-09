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
   | Authors: Marcus Boerger <helly@crx.net>                              |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include "crx.h"
#include "crx_spl.h"

/* {{{ spl_add_class_name */
void spl_add_class_name(zval *list, crex_class_entry *pce, int allow, int ce_flags)
{
	if (!allow || (allow > 0 && (pce->ce_flags & ce_flags)) || (allow < 0 && !(pce->ce_flags & ce_flags))) {
		zval *tmp;

		if ((tmp = crex_hash_find(C_ARRVAL_P(list), pce->name)) == NULL) {
			zval t;
			ZVAL_STR_COPY(&t, pce->name);
			crex_hash_add(C_ARRVAL_P(list), pce->name, &t);
		}
	}
}
/* }}} */

/* {{{ spl_add_interfaces */
void spl_add_interfaces(zval *list, crex_class_entry * pce, int allow, int ce_flags)
{
	if (pce->num_interfaces) {
		CREX_ASSERT(pce->ce_flags & CREX_ACC_LINKED);
		for (uint32_t num_interfaces = 0; num_interfaces < pce->num_interfaces; num_interfaces++) {
			spl_add_class_name(list, pce->interfaces[num_interfaces], allow, ce_flags);
		}
	}
}
/* }}} */

/* {{{ spl_add_traits */
void spl_add_traits(zval *list, crex_class_entry * pce, int allow, int ce_flags)
{
	crex_class_entry *trait;

	for (uint32_t num_traits = 0; num_traits < pce->num_traits; num_traits++) {
		trait = crex_fetch_class_by_name(pce->trait_names[num_traits].name,
			pce->trait_names[num_traits].lc_name, CREX_FETCH_CLASS_TRAIT);
		CREX_ASSERT(trait);
		spl_add_class_name(list, trait, allow, ce_flags);
	}
}
/* }}} */


/* {{{ spl_add_classes */
void spl_add_classes(crex_class_entry *pce, zval *list, bool sub, int allow, int ce_flags)
{
	CREX_ASSERT(pce);
	spl_add_class_name(list, pce, allow, ce_flags);
	if (sub) {
		spl_add_interfaces(list, pce, allow, ce_flags);
		while (pce->parent) {
			pce = pce->parent;
			spl_add_classes(pce, list, sub, allow, ce_flags);
		}
	}
}
/* }}} */

crex_string * spl_gen_private_prop_name(crex_class_entry *ce, char *prop_name, size_t prop_len) /* {{{ */
{
	return crex_mangle_property_name(ZSTR_VAL(ce->name), ZSTR_LEN(ce->name), prop_name, prop_len, 0);
}
/* }}} */
