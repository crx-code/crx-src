/*
   +----------------------------------------------------------------------+
   | Crex Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Crex Technologies Ltd. (http://www.crex.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_INHERITANCE_H
#define CREX_INHERITANCE_H

#include "crex.h"

BEGIN_EXTERN_C()

CREX_API void crex_do_implement_interface(crex_class_entry *ce, crex_class_entry *iface);
CREX_API void crex_do_inheritance_ex(crex_class_entry *ce, crex_class_entry *parent_ce, bool checked);

static crex_always_inline void crex_do_inheritance(crex_class_entry *ce, crex_class_entry *parent_ce) {
	crex_do_inheritance_ex(ce, parent_ce, 0);
}

CREX_API crex_class_entry *crex_do_link_class(crex_class_entry *ce, crex_string *lc_parent_name, crex_string *key);

void crex_verify_abstract_class(crex_class_entry *ce);
void crex_build_properties_info_table(crex_class_entry *ce);
CREX_API crex_class_entry *crex_try_early_bind(crex_class_entry *ce, crex_class_entry *parent_ce, crex_string *lcname, zval *delayed_early_binding);

void crex_inheritance_check_override(crex_class_entry *ce);

CREX_API extern crex_class_entry* (*crex_inheritance_cache_get)(crex_class_entry *ce, crex_class_entry *parent, crex_class_entry **traits_and_interfaces);
CREX_API extern crex_class_entry* (*crex_inheritance_cache_add)(crex_class_entry *ce, crex_class_entry *proto, crex_class_entry *parent, crex_class_entry **traits_and_interfaces, HashTable *dependencies);

END_EXTERN_C()

#endif
