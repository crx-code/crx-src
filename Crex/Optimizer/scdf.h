/*
   +----------------------------------------------------------------------+
   | Crex Engine, Call Graph                                              |
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
   | Authors: Nikita Popov <nikic@crx.net>                                |
   +----------------------------------------------------------------------+
*/

#ifndef _SCDF_H
#define _SCDF_H

#include "crex_bitset.h"

typedef struct _scdf_ctx {
	crex_op_array *op_array;
	crex_ssa *ssa;
	crex_bitset instr_worklist;
	/* Represent phi-instructions through the defining var */
	crex_bitset phi_var_worklist;
	crex_bitset block_worklist;
	crex_bitset executable_blocks;
	/* 1 bit per edge, see scdf_edge(cfg, from, to) */
	crex_bitset feasible_edges;
	uint32_t instr_worklist_len;
	uint32_t phi_var_worklist_len;
	uint32_t block_worklist_len;

	struct {
		void (*visit_instr)(
			struct _scdf_ctx *scdf, crex_op *opline, crex_ssa_op *ssa_op);
		void (*visit_phi)(
			struct _scdf_ctx *scdf, crex_ssa_phi *phi);
		void (*mark_feasible_successors)(
			struct _scdf_ctx *scdf, int block_num, crex_basic_block *block,
			crex_op *opline, crex_ssa_op *ssa_op);
	} handlers;
} scdf_ctx;

void scdf_init(crex_optimizer_ctx *ctx, scdf_ctx *scdf, crex_op_array *op_array, crex_ssa *ssa);
void scdf_solve(scdf_ctx *scdf, const char *name);

uint32_t scdf_remove_unreachable_blocks(scdf_ctx *scdf);

/* Add uses to worklist */
static inline void scdf_add_to_worklist(scdf_ctx *scdf, int var_num) {
	const crex_ssa *ssa = scdf->ssa;
	const crex_ssa_var *var = &ssa->vars[var_num];
	int use;
	crex_ssa_phi *phi;
	FOREACH_USE(var, use) {
		crex_bitset_incl(scdf->instr_worklist, use);
	} FOREACH_USE_END();
	FOREACH_PHI_USE(var, phi) {
		crex_bitset_incl(scdf->phi_var_worklist, phi->ssa_var);
	} FOREACH_PHI_USE_END();
}

/* This should usually not be necessary, however it's used for type narrowing. */
static inline void scdf_add_def_to_worklist(scdf_ctx *scdf, int var_num) {
	const crex_ssa_var *var = &scdf->ssa->vars[var_num];
	if (var->definition >= 0) {
		crex_bitset_incl(scdf->instr_worklist, var->definition);
	} else if (var->definition_phi) {
		crex_bitset_incl(scdf->phi_var_worklist, var_num);
	}
}

static inline uint32_t scdf_edge(const crex_cfg *cfg, int from, int to) {
	const crex_basic_block *to_block = cfg->blocks + to;
	int i;

	for (i = 0; i < to_block->predecessors_count; i++) {
		uint32_t edge = to_block->predecessor_offset + i;

		if (cfg->predecessors[edge] == from) {
			return edge;
		}
	}
	CREX_UNREACHABLE();
}

static inline bool scdf_is_edge_feasible(const scdf_ctx *scdf, int from, int to) {
	uint32_t edge = scdf_edge(&scdf->ssa->cfg, from, to);
	return crex_bitset_in(scdf->feasible_edges, edge);
}

void scdf_mark_edge_feasible(scdf_ctx *scdf, int from, int to);

#endif
