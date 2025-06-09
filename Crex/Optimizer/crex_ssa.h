/*
   +----------------------------------------------------------------------+
   | Crex Engine, SSA - Static Single Assignment Form                     |
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

#ifndef CREX_SSA_H
#define CREX_SSA_H

#include "crex_optimizer.h"
#include "crex_cfg.h"

typedef struct _crex_ssa_range {
	crex_long              min;
	crex_long              max;
	bool              underflow;
	bool              overflow;
} crex_ssa_range;

typedef enum _crex_ssa_negative_lat {
	NEG_NONE      = 0,
	NEG_INIT      = 1,
	NEG_INVARIANT = 2,
	NEG_USE_LT    = 3,
	NEG_USE_GT    = 4,
	NEG_UNKNOWN   = 5
} crex_ssa_negative_lat;

/* Special kind of SSA Phi function used in eSSA */
typedef struct _crex_ssa_range_constraint {
	crex_ssa_range         range;       /* simple range constraint */
	int                    min_var;
	int                    max_var;
	int                    min_ssa_var; /* ((min_var>0) ? MIN(ssa_var) : 0) + range.min */
	int                    max_ssa_var; /* ((max_var>0) ? MAX(ssa_var) : 0) + range.max */
	crex_ssa_negative_lat  negative;
} crex_ssa_range_constraint;

typedef struct _crex_ssa_type_constraint {
	uint32_t               type_mask;   /* Type mask to intersect with */
	crex_class_entry      *ce;          /* Class entry for instanceof constraints */
} crex_ssa_type_constraint;

typedef union _crex_ssa_pi_constraint {
	crex_ssa_range_constraint range;
	crex_ssa_type_constraint type;
} crex_ssa_pi_constraint;

/* SSA Phi - ssa_var = Phi(source0, source1, ...sourceN) */
typedef struct _crex_ssa_phi crex_ssa_phi;
struct _crex_ssa_phi {
	crex_ssa_phi          *next;          /* next Phi in the same BB */
	int                    pi;            /* if >= 0 this is actually a e-SSA Pi */
	crex_ssa_pi_constraint constraint;    /* e-SSA Pi constraint */
	int                    var;           /* Original CV, VAR or TMP variable index */
	int                    ssa_var;       /* SSA variable index */
	int                    block;         /* current BB index */
	bool                   has_range_constraint : 1;
	crex_ssa_phi         **use_chains;
	crex_ssa_phi          *sym_use_chain;
	int                   *sources;       /* Array of SSA IDs that produce this var.
									         As many as this block has
									         predecessors.  */
};

typedef struct _crex_ssa_block {
	crex_ssa_phi          *phis;
} crex_ssa_block;

typedef struct _crex_ssa_op {
	int                    op1_use;
	int                    op2_use;
	int                    result_use;
	int                    op1_def;
	int                    op2_def;
	int                    result_def;
	int                    op1_use_chain;
	int                    op2_use_chain;
	int                    res_use_chain;
} crex_ssa_op;

typedef enum _crex_ssa_alias_kind {
	NO_ALIAS,
	SYMTABLE_ALIAS,
	HTTP_RESPONSE_HEADER_ALIAS
} crex_ssa_alias_kind;

typedef enum _crex_ssa_escape_state {
	ESCAPE_STATE_UNKNOWN,
	ESCAPE_STATE_NO_ESCAPE,
	ESCAPE_STATE_FUNCTION_ESCAPE,
	ESCAPE_STATE_GLOBAL_ESCAPE
} crex_ssa_escape_state;

typedef struct _crex_ssa_var {
	int                    var;            /* original var number; op.var for CVs and following numbers for VARs and TMP_VARs */
	int                    scc;            /* strongly connected component */
	int                    definition;     /* opcode that defines this value */
	int                    use_chain;      /* uses of this value, linked through opN_use_chain */
	crex_ssa_phi          *definition_phi; /* phi that defines this value */
	crex_ssa_phi          *phi_use_chain;  /* uses of this value in Phi, linked through use_chain */
	crex_ssa_phi          *sym_use_chain;  /* uses of this value in Pi constraints */
	unsigned int           no_val : 1;     /* value doesn't matter (used as op1 in CREX_ASSIGN) */
	unsigned int           scc_entry : 1;
	unsigned int           alias : 2;  /* value may be changed indirectly */
	unsigned int           escape_state : 2;
} crex_ssa_var;

typedef struct _crex_ssa_var_info {
	uint32_t               type; /* inferred type (see crex_inference.h) */
	bool                   has_range : 1;
	bool                   is_instanceof : 1; /* 0 - class == "ce", 1 - may be child of "ce" */
	bool                   recursive : 1;
	bool                   use_as_double : 1;
	bool                   delayed_fetch_this : 1;
	bool                   avoid_refcounting : 1;
	bool                   guarded_reference : 1;
	bool                   indirect_reference : 1; /* IS_INDIRECT returned by FETCH_DIM_W/FETCH_OBJ_W */
	crex_ssa_range         range;
	crex_class_entry      *ce;
} crex_ssa_var_info;

typedef struct _crex_ssa {
	crex_cfg               cfg;            /* control flow graph             */
	int                    vars_count;     /* number of SSA variables        */
	int                    sccs;           /* number of SCCs                 */
	crex_ssa_block        *blocks;         /* array of SSA blocks            */
	crex_ssa_op           *ops;            /* array of SSA instructions      */
	crex_ssa_var          *vars;           /* use/def chain of SSA variables */
	crex_ssa_var_info     *var_info;
} crex_ssa;

BEGIN_EXTERN_C()

CREX_API crex_result crex_build_ssa(crex_arena **arena, const crex_script *script, const crex_op_array *op_array, uint32_t build_flags, crex_ssa *ssa);
CREX_API void crex_ssa_compute_use_def_chains(crex_arena **arena, const crex_op_array *op_array, crex_ssa *ssa);
CREX_API int crex_ssa_rename_op(const crex_op_array *op_array, const crex_op *opline, uint32_t k, uint32_t build_flags, int ssa_vars_count, crex_ssa_op *ssa_ops, int *var);
void crex_ssa_unlink_use_chain(crex_ssa *ssa, int op, int var);
void crex_ssa_replace_use_chain(crex_ssa *ssa, int op, int new_op, int var);

void crex_ssa_remove_predecessor(crex_ssa *ssa, int from, int to);
void crex_ssa_remove_defs_of_instr(crex_ssa *ssa, crex_ssa_op *ssa_op);
void crex_ssa_remove_instr(crex_ssa *ssa, crex_op *opline, crex_ssa_op *ssa_op);
void crex_ssa_remove_phi(crex_ssa *ssa, crex_ssa_phi *phi);
void crex_ssa_remove_uses_of_var(crex_ssa *ssa, int var_num);
void crex_ssa_remove_block(crex_op_array *op_array, crex_ssa *ssa, int b);
void crex_ssa_rename_var_uses(crex_ssa *ssa, int old_var, int new_var, bool update_types);
void crex_ssa_remove_block_from_cfg(crex_ssa *ssa, int b);

static crex_always_inline void _crex_ssa_remove_def(crex_ssa_var *var)
{
	CREX_ASSERT(var->definition >= 0);
	CREX_ASSERT(var->use_chain < 0);
	CREX_ASSERT(!var->phi_use_chain);
	var->definition = -1;
}

static crex_always_inline void crex_ssa_remove_result_def(crex_ssa *ssa, crex_ssa_op *ssa_op)
{
	crex_ssa_var *var = &ssa->vars[ssa_op->result_def];
	_crex_ssa_remove_def(var);
	ssa_op->result_def = -1;
}

static crex_always_inline void crex_ssa_remove_op1_def(crex_ssa *ssa, crex_ssa_op *ssa_op)
{
	crex_ssa_var *var = &ssa->vars[ssa_op->op1_def];
	_crex_ssa_remove_def(var);
	ssa_op->op1_def = -1;
}

static crex_always_inline void crex_ssa_remove_op2_def(crex_ssa *ssa, crex_ssa_op *ssa_op)
{
	crex_ssa_var *var = &ssa->vars[ssa_op->op2_def];
	_crex_ssa_remove_def(var);
	ssa_op->op2_def = -1;
}

END_EXTERN_C()

static crex_always_inline int crex_ssa_next_use(const crex_ssa_op *ssa_op, int var, int use)
{
	ssa_op += use;
	if (ssa_op->op1_use == var) {
		return ssa_op->op1_use_chain;
	} else if (ssa_op->op2_use == var) {
		return ssa_op->op2_use_chain;
	} else {
		return ssa_op->res_use_chain;
	}
}

static crex_always_inline crex_ssa_phi* crex_ssa_next_use_phi(const crex_ssa *ssa, int var, const crex_ssa_phi *p)
{
	if (p->pi >= 0) {
		return p->use_chains[0];
	} else {
		int j;
		for (j = 0; j < ssa->cfg.blocks[p->block].predecessors_count; j++) {
			if (p->sources[j] == var) {
				return p->use_chains[j];
			}
		}
	}
	return NULL;
}

static crex_always_inline bool crex_ssa_is_no_val_use(const crex_op *opline, const crex_ssa_op *ssa_op, int var)
{
	if (opline->opcode == CREX_ASSIGN
			 || opline->opcode == CREX_UNSET_CV
			 || opline->opcode == CREX_BIND_GLOBAL
			 || opline->opcode == CREX_BIND_STATIC) {
		return ssa_op->op1_use == var && ssa_op->op2_use != var;
	}
	if (opline->opcode == CREX_FE_FETCH_R || opline->opcode == CREX_FE_FETCH_RW) {
		return ssa_op->op2_use == var && ssa_op->op1_use != var;
	}
	if (ssa_op->result_use == var
			&& opline->opcode != CREX_ADD_ARRAY_ELEMENT
			&& opline->opcode != CREX_ADD_ARRAY_UNPACK) {
		return ssa_op->op1_use != var && ssa_op->op2_use != var;
	}
	return 0;
}

static crex_always_inline void crex_ssa_rename_defs_of_instr(crex_ssa *ssa, crex_ssa_op *ssa_op) {
	/* Rename def to use if possible. Mark variable as not defined otherwise. */
	if (ssa_op->op1_def >= 0) {
		if (ssa_op->op1_use >= 0) {
			crex_ssa_rename_var_uses(ssa, ssa_op->op1_def, ssa_op->op1_use, 1);
		}
		ssa->vars[ssa_op->op1_def].definition = -1;
		ssa_op->op1_def = -1;
	}
	if (ssa_op->op2_def >= 0) {
		if (ssa_op->op2_use >= 0) {
			crex_ssa_rename_var_uses(ssa, ssa_op->op2_def, ssa_op->op2_use, 1);
		}
		ssa->vars[ssa_op->op2_def].definition = -1;
		ssa_op->op2_def = -1;
	}
	if (ssa_op->result_def >= 0) {
		if (ssa_op->result_use >= 0) {
			crex_ssa_rename_var_uses(ssa, ssa_op->result_def, ssa_op->result_use, 1);
		}
		ssa->vars[ssa_op->result_def].definition = -1;
		ssa_op->result_def = -1;
	}
}

#define NUM_PHI_SOURCES(phi) \
	((phi)->pi >= 0 ? 1 : (ssa->cfg.blocks[(phi)->block].predecessors_count))

/* FOREACH_USE and FOREACH_PHI_USE explicitly support "continue"
 * and changing the use chain of the current element */
#define FOREACH_USE(var, use) do { \
	int _var_num = (var) - ssa->vars, next; \
	for (use = (var)->use_chain; use >= 0; use = next) { \
		next = crex_ssa_next_use(ssa->ops, _var_num, use);
#define FOREACH_USE_END() \
	} \
} while (0)

#define FOREACH_PHI_USE(var, phi) do { \
	int _var_num = (var) - ssa->vars; \
	crex_ssa_phi *next_phi; \
	for (phi = (var)->phi_use_chain; phi; phi = next_phi) { \
		next_phi = crex_ssa_next_use_phi(ssa, _var_num, phi);
#define FOREACH_PHI_USE_END() \
	} \
} while (0)

#define FOREACH_PHI_SOURCE(phi, source) do { \
	crex_ssa_phi *_phi = (phi); \
	int _i, _end = NUM_PHI_SOURCES(phi); \
	for (_i = 0; _i < _end; _i++) { \
		CREX_ASSERT(_phi->sources[_i] >= 0); \
		source = _phi->sources[_i];
#define FOREACH_PHI_SOURCE_END() \
	} \
} while (0)

#define FOREACH_PHI(phi) do { \
	int _i; \
	for (_i = 0; _i < ssa->cfg.blocks_count; _i++) { \
		phi = ssa->blocks[_i].phis; \
		for (; phi; phi = phi->next) {
#define FOREACH_PHI_END() \
		} \
	} \
} while (0)

#define FOREACH_BLOCK(block) do { \
	int _i; \
	for (_i = 0; _i < ssa->cfg.blocks_count; _i++) { \
		(block) = &ssa->cfg.blocks[_i]; \
		if (!((block)->flags & CREX_BB_REACHABLE)) { \
			continue; \
		}
#define FOREACH_BLOCK_END() \
	} \
} while (0)

/* Does not support "break" */
#define FOREACH_INSTR_NUM(i) do { \
	crex_basic_block *_block; \
	FOREACH_BLOCK(_block) { \
		uint32_t _end = _block->start + _block->len; \
		for ((i) = _block->start; (i) < _end; (i)++) {
#define FOREACH_INSTR_NUM_END() \
		} \
	} FOREACH_BLOCK_END(); \
} while (0)

#endif /* CREX_SSA_H */
