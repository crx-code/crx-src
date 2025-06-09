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

#ifndef CREX_ACCELERATOR_UTIL_FUNCS_H
#define CREX_ACCELERATOR_UTIL_FUNCS_H

#include "crex.h"
#include "CrexAccelerator.h"

BEGIN_EXTERN_C()

crex_persistent_script* create_persistent_script(void);
void free_persistent_script(crex_persistent_script *persistent_script, int destroy_elements);

void crex_accel_move_user_functions(HashTable *str, uint32_t count, crex_script *script);
void crex_accel_move_user_classes(HashTable *str, uint32_t count, crex_script *script);
void crex_accel_build_delayed_early_binding_list(crex_persistent_script *persistent_script);
void crex_accel_finalize_delayed_early_binding_list(crex_persistent_script *persistent_script);
void crex_accel_free_delayed_early_binding_list(crex_persistent_script *persistent_script);

crex_op_array* crex_accel_load_script(crex_persistent_script *persistent_script, int from_shared_memory);

#define ADLER32_INIT 1     /* initial Adler-32 value */

unsigned int crex_adler32(unsigned int checksum, unsigned char *buf, uint32_t len);

unsigned int crex_accel_script_checksum(crex_persistent_script *persistent_script);

END_EXTERN_C()

#endif /* CREX_ACCELERATOR_UTIL_FUNCS_H */
