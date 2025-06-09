/*
   +----------------------------------------------------------------------+
   | Crex OPcache                                                         |
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
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   |          Stanislav Malyshev <stas@crex.com>                          |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_PERSIST_H
#define CREX_PERSIST_H

BEGIN_EXTERN_C()

uint32_t crex_accel_script_persist_calc(crex_persistent_script *script, int for_shm);
crex_persistent_script *crex_accel_script_persist(crex_persistent_script *script, int for_shm);

void crex_persist_class_entry_calc(crex_class_entry *ce);
crex_class_entry *crex_persist_class_entry(crex_class_entry *ce);
void crex_update_parent_ce(crex_class_entry *ce);
void crex_persist_warnings_calc(uint32_t num_warnings, crex_error_info **warnings);
crex_error_info **crex_persist_warnings(uint32_t num_warnings, crex_error_info **warnings);

END_EXTERN_C()

#endif /* CREX_PERSIST_H */
