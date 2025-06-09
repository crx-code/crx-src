/*
   +----------------------------------------------------------------------+
   | Crex Engine, DFG - Data Flow Graph                                   |
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

#ifndef CREX_DFG_H
#define CREX_DFG_H

#include "crex_bitset.h"
#include "crex_cfg.h"

typedef struct _crex_dfg {
	int         vars;
	uint32_t    size;
	crex_bitset tmp;
	crex_bitset def;
	crex_bitset use;
	crex_bitset in;
	crex_bitset out;
} crex_dfg;

#define DFG_BITSET(set, set_size, block_num) \
	((set) + ((block_num) * (set_size)))

#define DFG_SET(set, set_size, block_num, var_num) \
	crex_bitset_incl(DFG_BITSET(set, set_size, block_num), (var_num))

#define DFG_ISSET(set, set_size, block_num, var_num) \
	crex_bitset_in(DFG_BITSET(set, set_size, block_num), (var_num))

BEGIN_EXTERN_C()

void crex_build_dfg(const crex_op_array *op_array, const crex_cfg *cfg, crex_dfg *dfg, uint32_t build_flags);
CREX_API void crex_dfg_add_use_def_op(const crex_op_array *op_array, const crex_op *opline, uint32_t build_flags, crex_bitset use, crex_bitset def);

END_EXTERN_C()

#endif /* CREX_DFG_H */
