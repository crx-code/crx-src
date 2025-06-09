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

#ifndef CRX_FUNCTIONS_H
#define CRX_FUNCTIONS_H

#include "crx.h"

typedef crex_object* (*create_object_func_t)(crex_class_entry *class_type);

/* sub: whether to allow subclasses/interfaces
   allow = 0: allow all classes and interfaces
   allow > 0: allow all that match and mask ce_flags
   allow < 0: disallow all that match and mask ce_flags
 */
void spl_add_class_name(zval * list, crex_class_entry * pce, int allow, int ce_flags);
void spl_add_interfaces(zval * list, crex_class_entry * pce, int allow, int ce_flags);
void spl_add_traits(zval * list, crex_class_entry * pce, int allow, int ce_flags);
void spl_add_classes(crex_class_entry *pce, zval *list, bool sub, int allow, int ce_flags);

/* caller must efree(return) */
crex_string *spl_gen_private_prop_name(crex_class_entry *ce, char *prop_name, size_t prop_len);

#endif /* CRX_FUNCTIONS_H */
