/*
   +----------------------------------------------------------------------+
   | Crex Engine, Bytecode Visualisation                                  |
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
   | Authors: Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_DUMP_H
#define CREX_DUMP_H

#include "crex_ssa.h"
#include "crex_dfg.h"

#include <stdint.h>

#define CREX_DUMP_HIDE_UNREACHABLE     (1<<0)
#define CREX_DUMP_RC_INFERENCE         (1<<1)
#define CREX_DUMP_CFG                  (1<<2)
#define CREX_DUMP_SSA                  (1<<3)
#define CREX_DUMP_LIVE_RANGES          (1<<4)
#define CREX_DUMP_LINE_NUMBERS         (1<<5)

BEGIN_EXTERN_C()

CREX_API void crex_dump_op_array(const crex_op_array *op_array, uint32_t dump_flags, const char *msg, const void *data);
CREX_API void crex_dump_op(const crex_op_array *op_array, const crex_basic_block *b, const crex_op *opline, uint32_t dump_flags, const crex_ssa *ssa, const crex_ssa_op *ssa_op);
CREX_API void crex_dump_op_line(const crex_op_array *op_array, const crex_basic_block *b, const crex_op *opline, uint32_t dump_flags, const void *data);
void crex_dump_dominators(const crex_op_array *op_array, const crex_cfg *cfg);
void crex_dump_dfg(const crex_op_array *op_array, const crex_cfg *cfg, const crex_dfg *dfg);
void crex_dump_phi_placement(const crex_op_array *op_array, const crex_ssa *ssa);
void crex_dump_variables(const crex_op_array *op_array);
void crex_dump_ssa_variables(const crex_op_array *op_array, const crex_ssa *ssa, uint32_t dump_flags);
CREX_API void crex_dump_ssa_var(const crex_op_array *op_array, const crex_ssa *ssa, int ssa_var_num, uint8_t var_type, int var_num, uint32_t dump_flags);
CREX_API void crex_dump_var(const crex_op_array *op_array, uint8_t var_type, int var_num);
void crex_dump_op_array_name(const crex_op_array *op_array);
void crex_dump_const(const zval *zv);
void crex_dump_ht(HashTable *ht);

END_EXTERN_C()

#endif /* CREX_DUMP_H */
