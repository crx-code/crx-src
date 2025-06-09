/*
   +----------------------------------------------------------------------+
   | Crex JIT                                                             |
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

#include "main/crx.h"
#include "main/SAPI.h"
#include "crx_version.h"
#include <CrexAccelerator.h>
#include "crex_shared_alloc.h"
#include "Crex/crex_execute.h"
#include "Crex/crex_vm.h"
#include "Crex/crex_exceptions.h"
#include "Crex/crex_constants.h"
#include "Crex/crex_closures.h"
#include "Crex/crex_ini.h"
#include "Crex/crex_observer.h"
#include "crex_smart_str.h"
#include "jit/crex_jit.h"

#ifdef HAVE_JIT

#include "Optimizer/crex_func_info.h"
#include "Optimizer/crex_ssa.h"
#include "Optimizer/crex_inference.h"
#include "Optimizer/crex_call_graph.h"
#include "Optimizer/crex_dump.h"

#if CREX_JIT_TARGET_X86
# include "jit/crex_jit_x86.h"
#elif CREX_JIT_TARGET_ARM64
# include "jit/crex_jit_arm64.h"
#endif

#include "jit/crex_jit_internal.h"

#ifdef HAVE_PTHREAD_JIT_WRITE_PROTECT_NP
#include <pthread.h>
#endif

#ifdef ZTS
int jit_globals_id;
#else
crex_jit_globals jit_globals;
#endif

//#define CONTEXT_THREADED_JIT
#define CREX_JIT_USE_RC_INFERENCE

#ifdef CREX_JIT_USE_RC_INFERENCE
# define CREX_SSA_RC_INFERENCE_FLAG CREX_SSA_RC_INFERENCE
# define RC_MAY_BE_1(info)          (((info) & (MAY_BE_RC1|MAY_BE_REF)) != 0)
# define RC_MAY_BE_N(info)          (((info) & (MAY_BE_RCN|MAY_BE_REF)) != 0)
#else
# define CREX_SSA_RC_INFERENCE_FLAG 0
# define RC_MAY_BE_1(info)          1
# define RC_MAY_BE_N(info)          1
#endif

#define JIT_PREFIX      "JIT$"
#define JIT_STUB_PREFIX "JIT$$"
#define TRACE_PREFIX    "TRACE-"

#define DASM_M_GROW(ctx, t, p, sz, need) \
  do { \
    size_t _sz = (sz), _need = (need); \
    if (_sz < _need) { \
      if (_sz < 16) _sz = 16; \
      while (_sz < _need) _sz += _sz; \
      (p) = (t *)erealloc((p), _sz); \
      (sz) = _sz; \
    } \
  } while(0)

#define DASM_M_FREE(ctx, p, sz) efree(p)

#if CREX_DEBUG
# define DASM_CHECKS 1
#endif

#include "dynasm/dasm_proto.h"

typedef struct _crex_jit_stub {
	const char *name;
	int (*stub)(dasm_State **Dst);
	uint32_t offset;
	uint32_t adjustment;
} crex_jit_stub;

#define JIT_STUB(name, offset, adjustment) \
	{JIT_STUB_PREFIX #name, crex_jit_ ## name ## _stub, offset, adjustment}

crex_ulong crex_jit_profile_counter = 0;
int crex_jit_profile_counter_rid = -1;

int16_t crex_jit_hot_counters[CREX_HOT_COUNTERS_COUNT];

const crex_op *crex_jit_halt_op = NULL;
static int crex_jit_vm_kind = 0;
#ifdef HAVE_PTHREAD_JIT_WRITE_PROTECT_NP
static int crex_write_protect = 1;
#endif

static void *dasm_buf = NULL;
static void *dasm_end = NULL;
static void **dasm_ptr = NULL;

static size_t dasm_size = 0;

static crex_long jit_bisect_pos = 0;

static const void *crex_jit_runtime_jit_handler = NULL;
static const void *crex_jit_profile_jit_handler = NULL;
static const void *crex_jit_func_hot_counter_handler = NULL;
static const void *crex_jit_loop_hot_counter_handler = NULL;
static const void *crex_jit_func_trace_counter_handler = NULL;
static const void *crex_jit_ret_trace_counter_handler = NULL;
static const void *crex_jit_loop_trace_counter_handler = NULL;

static int CREX_FASTCALL crex_runtime_jit(void);

static int crex_jit_trace_op_len(const crex_op *opline);
static int crex_jit_trace_may_exit(const crex_op_array *op_array, const crex_op *opline);
static uint32_t crex_jit_trace_get_exit_point(const crex_op *to_opline, uint32_t flags);
static const void *crex_jit_trace_get_exit_addr(uint32_t n);
static void crex_jit_trace_add_code(const void *start, uint32_t size);
static bool crex_jit_needs_arg_dtor(const crex_function *func, uint32_t arg_num, crex_call_info *call_info);

#if CREX_JIT_TARGET_ARM64
static crex_jit_trace_info *crex_jit_get_current_trace_info(void);
static uint32_t crex_jit_trace_find_exit_point(const void* addr);
#endif

#if CREX_JIT_TARGET_X86 && defined(__linux__)
# if CRX_HAVE_BUILTIN_CPU_SUPPORTS && defined(__GNUC__) && (CREX_GCC_VERSION >= 11000)
# define CREX_JIT_SUPPORT_CLDEMOTE 1
# else
# define CREX_JIT_SUPPORT_CLDEMOTE 0
# endif
#endif

#if CREX_JIT_SUPPORT_CLDEMOTE
#include <immintrin.h>
#pragma GCC push_options
#pragma GCC target("cldemote")
// check cldemote by CPUID when JIT startup
static int cpu_support_cldemote = 0;
static inline void shared_cacheline_demote(uintptr_t start, size_t size) {
    uintptr_t cache_line_base = start & ~0x3F;
    do {
        _cldemote((void *)cache_line_base);
        // next cacheline start size
        cache_line_base += 64;
    } while (cache_line_base < start + size);
}
#pragma GCC pop_options
#endif

static int crex_jit_assign_to_variable(dasm_State    **Dst,
                                       const crex_op  *opline,
                                       crex_jit_addr   var_use_addr,
                                       crex_jit_addr   var_addr,
                                       uint32_t        var_info,
                                       uint32_t        var_def_info,
                                       uint8_t         val_type,
                                       crex_jit_addr   val_addr,
                                       uint32_t        val_info,
                                       crex_jit_addr   res_addr,
                                       bool       check_exception);

static bool dominates(const crex_basic_block *blocks, int a, int b) {
	while (blocks[b].level > blocks[a].level) {
		b = blocks[b].idom;
	}
	return a == b;
}

static bool crex_ssa_is_last_use(const crex_op_array *op_array, const crex_ssa *ssa, int var, int use)
{
	int next_use;

	if (ssa->vars[var].phi_use_chain) {
		crex_ssa_phi *phi = ssa->vars[var].phi_use_chain;
		do {
			if (!ssa->vars[phi->ssa_var].no_val) {
				return 0;
			}
			phi = crex_ssa_next_use_phi(ssa, var, phi);
		} while (phi);
	}

	if (ssa->cfg.blocks[ssa->cfg.map[use]].loop_header > 0
	 || (ssa->cfg.blocks[ssa->cfg.map[use]].flags & CREX_BB_LOOP_HEADER)) {
		int b = ssa->cfg.map[use];
		int prev_use = ssa->vars[var].use_chain;

		while (prev_use >= 0 && prev_use != use) {
			if (b != ssa->cfg.map[prev_use]
			 && dominates(ssa->cfg.blocks, b, ssa->cfg.map[prev_use])
			 && !crex_ssa_is_no_val_use(op_array->opcodes + prev_use, ssa->ops + prev_use, var)) {
				return 0;
			}
			prev_use = crex_ssa_next_use(ssa->ops, var, prev_use);
		}
	}

	next_use = crex_ssa_next_use(ssa->ops, var, use);
	if (next_use < 0) {
		return 1;
	} else if (crex_ssa_is_no_val_use(op_array->opcodes + next_use, ssa->ops + next_use, var)) {
		return 1;
	}
	return 0;
}

static bool crex_ival_is_last_use(const crex_lifetime_interval *ival, int use)
{
	if (ival->flags & ZREG_LAST_USE) {
		const crex_life_range *range = &ival->range;

		while (range->next) {
			range = range->next;
		}
		return range->end == use;
	}
	return 0;
}

static bool crex_is_commutative(uint8_t opcode)
{
	return
		opcode == CREX_ADD ||
		opcode == CREX_MUL ||
		opcode == CREX_BW_OR ||
		opcode == CREX_BW_AND ||
		opcode == CREX_BW_XOR;
}

static int crex_jit_is_constant_cmp_long_long(const crex_op  *opline,
                                              crex_ssa_range *op1_range,
                                              crex_jit_addr   op1_addr,
                                              crex_ssa_range *op2_range,
                                              crex_jit_addr   op2_addr,
                                              bool           *result)
{
	crex_long op1_min;
	crex_long op1_max;
	crex_long op2_min;
	crex_long op2_max;

	if (op1_range) {
		op1_min = op1_range->min;
		op1_max = op1_range->max;
	} else if (C_MODE(op1_addr) == IS_CONST_ZVAL) {
		CREX_ASSERT(C_TYPE_P(C_ZV(op1_addr)) == IS_LONG);
		op1_min = op1_max = C_LVAL_P(C_ZV(op1_addr));
	} else {
		return 0;
	}

	if (op2_range) {
		op2_min = op2_range->min;
		op2_max = op2_range->max;
	} else if (C_MODE(op2_addr) == IS_CONST_ZVAL) {
		CREX_ASSERT(C_TYPE_P(C_ZV(op2_addr)) == IS_LONG);
		op2_min = op2_max = C_LVAL_P(C_ZV(op2_addr));
	} else {
		return 0;
	}

	switch (opline->opcode) {
		case CREX_IS_EQUAL:
		case CREX_IS_IDENTICAL:
		case CREX_CASE:
		case CREX_CASE_STRICT:
			if (op1_min == op1_max && op2_min == op2_max && op1_min == op2_min) {
				*result = 1;
				return 1;
			} else if (op1_max < op2_min || op1_min > op2_max) {
				*result = 0;
				return 1;
			}
			return 0;
		case CREX_IS_NOT_EQUAL:
		case CREX_IS_NOT_IDENTICAL:
			if (op1_min == op1_max && op2_min == op2_max && op1_min == op2_min) {
				*result = 0;
				return 1;
			} else if (op1_max < op2_min || op1_min > op2_max) {
				*result = 1;
				return 1;
			}
			return 0;
		case CREX_IS_SMALLER:
			if (op1_max < op2_min) {
				*result = 1;
				return 1;
			} else if (op1_min >= op2_max) {
				*result = 0;
				return 1;
			}
			return 0;
		case CREX_IS_SMALLER_OR_EQUAL:
			if (op1_max <= op2_min) {
				*result = 1;
				return 1;
			} else if (op1_min > op2_max) {
				*result = 0;
				return 1;
			}
			return 0;
		default:
			CREX_UNREACHABLE();
	}
	return 0;
}

static int crex_jit_needs_call_chain(crex_call_info *call_info, uint32_t b, const crex_op_array *op_array, crex_ssa *ssa, const crex_ssa_op *ssa_op, const crex_op *opline, int call_level, crex_jit_trace_rec *trace)
{
	int skip;

	if (trace) {
		crex_jit_trace_rec *p = trace;

		ssa_op++;
		while (1) {
			if (p->op == CREX_JIT_TRACE_VM) {
				switch (p->opline->opcode) {
					case CREX_SEND_ARRAY:
					case CREX_SEND_USER:
					case CREX_SEND_UNPACK:
					case CREX_INIT_FCALL:
					case CREX_INIT_METHOD_CALL:
					case CREX_INIT_STATIC_METHOD_CALL:
					case CREX_INIT_FCALL_BY_NAME:
					case CREX_INIT_NS_FCALL_BY_NAME:
					case CREX_INIT_DYNAMIC_CALL:
					case CREX_NEW:
					case CREX_INIT_USER_CALL:
					case CREX_FAST_CALL:
					case CREX_JMP:
					case CREX_JMPZ:
					case CREX_JMPNZ:
					case CREX_JMPC_EX:
					case CREX_JMPNC_EX:
					case CREX_FE_RESET_R:
					case CREX_FE_RESET_RW:
					case CREX_JMP_SET:
					case CREX_COALESCE:
					case CREX_JMP_NULL:
					case CREX_ASSERT_CHECK:
					case CREX_CATCH:
					case CREX_DECLARE_ANON_CLASS:
					case CREX_FE_FETCH_R:
					case CREX_FE_FETCH_RW:
					case CREX_BIND_INIT_STATIC_OR_JMP:
						return 1;
					case CREX_DO_ICALL:
					case CREX_DO_UCALL:
					case CREX_DO_FCALL_BY_NAME:
					case CREX_DO_FCALL:
					case CREX_CALLABLE_CONVERT:
						return 0;
					case CREX_SEND_VAL:
					case CREX_SEND_VAR:
					case CREX_SEND_VAL_EX:
					case CREX_SEND_VAR_EX:
					case CREX_SEND_FUNC_ARG:
					case CREX_SEND_REF:
					case CREX_SEND_VAR_NO_REF:
					case CREX_SEND_VAR_NO_REF_EX:
						/* skip */
						break;
					default:
						if (crex_may_throw(opline, ssa_op, op_array, ssa)) {
							return 1;
						}
				}
				ssa_op += crex_jit_trace_op_len(opline);
			} else if (p->op == CREX_JIT_TRACE_ENTER ||
			           p->op == CREX_JIT_TRACE_BACK ||
			           p->op == CREX_JIT_TRACE_END) {
				return 1;
			}
			p++;
		}
	}

	if (!call_info) {
		const crex_op *end = op_array->opcodes + op_array->last;

		opline++;
		ssa_op++;
		skip = (call_level == 1);
		while (opline != end) {
			if (!skip) {
				if (crex_may_throw(opline, ssa_op, op_array, ssa)) {
					return 1;
				}
			}
			switch (opline->opcode) {
				case CREX_SEND_VAL:
				case CREX_SEND_VAR:
				case CREX_SEND_VAL_EX:
				case CREX_SEND_VAR_EX:
				case CREX_SEND_FUNC_ARG:
				case CREX_SEND_REF:
				case CREX_SEND_VAR_NO_REF:
				case CREX_SEND_VAR_NO_REF_EX:
					skip = 0;
					break;
				case CREX_SEND_ARRAY:
				case CREX_SEND_USER:
				case CREX_SEND_UNPACK:
				case CREX_INIT_FCALL:
				case CREX_INIT_METHOD_CALL:
				case CREX_INIT_STATIC_METHOD_CALL:
				case CREX_INIT_FCALL_BY_NAME:
				case CREX_INIT_NS_FCALL_BY_NAME:
				case CREX_INIT_DYNAMIC_CALL:
				case CREX_NEW:
				case CREX_INIT_USER_CALL:
				case CREX_FAST_CALL:
				case CREX_JMP:
				case CREX_JMPZ:
				case CREX_JMPNZ:
				case CREX_JMPC_EX:
				case CREX_JMPNC_EX:
				case CREX_FE_RESET_R:
				case CREX_FE_RESET_RW:
				case CREX_JMP_SET:
				case CREX_COALESCE:
				case CREX_JMP_NULL:
				case CREX_ASSERT_CHECK:
				case CREX_CATCH:
				case CREX_DECLARE_ANON_CLASS:
				case CREX_FE_FETCH_R:
				case CREX_FE_FETCH_RW:
				case CREX_BIND_INIT_STATIC_OR_JMP:
					return 1;
				case CREX_DO_ICALL:
				case CREX_DO_UCALL:
				case CREX_DO_FCALL_BY_NAME:
				case CREX_DO_FCALL:
				case CREX_CALLABLE_CONVERT:
					end = opline;
					if (end - op_array->opcodes >= ssa->cfg.blocks[b].start + ssa->cfg.blocks[b].len) {
						/* INIT_FCALL and DO_FCALL in different BasicBlocks */
						return 1;
					}
					return 0;
			}
			opline++;
			ssa_op++;
		}

		return 1;
	} else {
		const crex_op *end = call_info->caller_call_opline;

		/* end may be null if an opcode like EXIT is part of the argument list. */
		if (!end || end - op_array->opcodes >= ssa->cfg.blocks[b].start + ssa->cfg.blocks[b].len) {
			/* INIT_FCALL and DO_FCALL in different BasicBlocks */
			return 1;
		}

		opline++;
		ssa_op++;
		skip = (call_level == 1);
		while (opline != end) {
			if (skip) {
				switch (opline->opcode) {
					case CREX_SEND_VAL:
					case CREX_SEND_VAR:
					case CREX_SEND_VAL_EX:
					case CREX_SEND_VAR_EX:
					case CREX_SEND_FUNC_ARG:
					case CREX_SEND_REF:
					case CREX_SEND_VAR_NO_REF:
					case CREX_SEND_VAR_NO_REF_EX:
						skip = 0;
						break;
					case CREX_SEND_ARRAY:
					case CREX_SEND_USER:
					case CREX_SEND_UNPACK:
						return 1;
				}
			} else {
				if (crex_may_throw(opline, ssa_op, op_array, ssa)) {
					return 1;
				}
			}
			opline++;
			ssa_op++;
		}

		return 0;
	}
}

static uint32_t skip_valid_arguments(const crex_op_array *op_array, crex_ssa *ssa, const crex_call_info *call_info)
{
	uint32_t num_args = 0;
	crex_function *func = call_info->callee_func;

	/* It's okay to handle prototypes here, because they can only increase the accepted arguments.
	 * Anything legal for the parent method is also legal for the parent method. */
	while (num_args < call_info->num_args) {
		crex_arg_info *arg_info = func->op_array.arg_info + num_args;

		if (CREX_TYPE_IS_SET(arg_info->type)) {
			if (CREX_TYPE_IS_ONLY_MASK(arg_info->type)) {
				crex_op *opline = call_info->arg_info[num_args].opline;
				crex_ssa_op *ssa_op = &ssa->ops[opline - op_array->opcodes];
				uint32_t type_mask = CREX_TYPE_PURE_MASK(arg_info->type);
				if ((OP1_INFO() & (MAY_BE_ANY|MAY_BE_UNDEF)) & ~type_mask) {
					break;
				}
			} else {
				break;
			}
		}
		num_args++;
	}
	return num_args;
}

static uint32_t crex_ssa_cv_info(const crex_op_array *op_array, crex_ssa *ssa, uint32_t var)
{
	uint32_t j, info;

	if (ssa->vars && ssa->var_info) {
		info = ssa->var_info[var].type;
		for (j = op_array->last_var; j < ssa->vars_count; j++) {
			if (ssa->vars[j].var == var) {
				info |= ssa->var_info[j].type;
			}
		}
	} else {
		info = MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_REF | MAY_BE_ANY | MAY_BE_UNDEF |
			MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
	}

#ifdef CREX_JIT_USE_RC_INFERENCE
	/* Refcount may be increased by RETURN opcode */
	if ((info & MAY_BE_RC1) && !(info & MAY_BE_RCN)) {
		for (j = 0; j < ssa->cfg.blocks_count; j++) {
			if ((ssa->cfg.blocks[j].flags & CREX_BB_REACHABLE) &&
			    ssa->cfg.blocks[j].len > 0) {
				const crex_op *opline = op_array->opcodes + ssa->cfg.blocks[j].start + ssa->cfg.blocks[j].len - 1;

				if (opline->opcode == CREX_RETURN) {
					if (opline->op1_type == IS_CV && opline->op1.var == EX_NUM_TO_VAR(var)) {
						info |= MAY_BE_RCN;
						break;
					}
				}
			}
		}
	}
#endif

	return info;
}

static bool crex_jit_may_avoid_refcounting(const crex_op *opline, uint32_t op1_info)
{
	switch (opline->opcode) {
		case CREX_FETCH_OBJ_FUNC_ARG:
			if (!JIT_G(current_frame) ||
			    !JIT_G(current_frame)->call->func ||
			    !TRACE_FRAME_IS_LAST_SEND_BY_VAL(JIT_G(current_frame)->call)) {
				return 0;
			}
			/* break missing intentionally */
		case CREX_FETCH_OBJ_R:
		case CREX_FETCH_OBJ_IS:
			if ((op1_info & MAY_BE_OBJECT)
			 && opline->op2_type == IS_CONST
			 && C_TYPE_P(RT_CONSTANT(opline, opline->op2)) == IS_STRING
			 && C_STRVAL_P(RT_CONSTANT(opline, opline->op2))[0] != '\0') {
				return 1;
			}
			break;
		case CREX_FETCH_DIM_FUNC_ARG:
			if (!JIT_G(current_frame) ||
			    !JIT_G(current_frame)->call->func ||
			    !TRACE_FRAME_IS_LAST_SEND_BY_VAL(JIT_G(current_frame)->call)) {
				return 0;
			}
			/* break missing intentionally */
		case CREX_FETCH_DIM_R:
		case CREX_FETCH_DIM_IS:
			return 1;
		case CREX_ISSET_ISEMPTY_DIM_OBJ:
			if (!(opline->extended_value & CREX_ISEMPTY)) {
				return 1;
			}
			break;
	}
	return 0;
}

static bool crex_jit_is_persistent_constant(zval *key, uint32_t flags)
{
	zval *zv;
	crex_constant *c = NULL;

	/* null/true/false are resolved during compilation, so don't check for them here. */
	zv = crex_hash_find_known_hash(EG(crex_constants), C_STR_P(key));
	if (zv) {
		c = (crex_constant*)C_PTR_P(zv);
	} else if (flags & IS_CONSTANT_UNQUALIFIED_IN_NAMESPACE) {
		key++;
		zv = crex_hash_find_known_hash(EG(crex_constants), C_STR_P(key));
		if (zv) {
			c = (crex_constant*)C_PTR_P(zv);
		}
	}
	return c && (CREX_CONSTANT_FLAGS(c) & CONST_PERSISTENT);
}

static crex_property_info* crex_get_known_property_info(const crex_op_array *op_array, crex_class_entry *ce, crex_string *member, bool on_this, crex_string *filename)
{
	crex_property_info *info = NULL;

	if ((on_this && (op_array->fn_flags & CREX_ACC_TRAIT_CLONE)) ||
	    !ce ||
	    !(ce->ce_flags & CREX_ACC_LINKED) ||
	    (ce->ce_flags & CREX_ACC_TRAIT) ||
	    ce->create_object) {
		return NULL;
	}

	if (!(ce->ce_flags & CREX_ACC_IMMUTABLE)) {
		if (ce->info.user.filename != filename) {
			/* class declaration might be changed independently */
			return NULL;
		}

		if (ce->parent) {
			crex_class_entry *parent = ce->parent;

			do {
				if (parent->type == CREX_INTERNAL_CLASS) {
					break;
				} else if (parent->info.user.filename != filename) {
					/* some of parents class declarations might be changed independently */
					/* TODO: this check may be not enough, because even
					 * in the same it's possible to conditionally define
					 * few classes with the same name, and "parent" may
					 * change from request to request.
					 */
					return NULL;
				}
				parent = parent->parent;
			} while (parent);
		}
	}

	info = (crex_property_info*)crex_hash_find_ptr(&ce->properties_info, member);
	if (info == NULL ||
	    !IS_VALID_PROPERTY_OFFSET(info->offset) ||
	    (info->flags & CREX_ACC_STATIC)) {
		return NULL;
	}

	if (info->flags & CREX_ACC_PUBLIC) {
		return info;
	} else if (on_this) {
		if (ce == info->ce) {
			if (ce == op_array->scope) {
				return info;
			} else {
				return NULL;
			}
		} else if ((info->flags & CREX_ACC_PROTECTED)
				&& instanceof_function_slow(ce, info->ce)) {
			return info;
		}
	}

	return NULL;
}

static bool crex_may_be_dynamic_property(crex_class_entry *ce, crex_string *member, bool on_this, crex_string *filename)
{
	crex_property_info *info;

	if (!ce || (ce->ce_flags & CREX_ACC_TRAIT)) {
		return 1;
	}

	if (!(ce->ce_flags & CREX_ACC_IMMUTABLE)) {
		if (ce->info.user.filename != filename) {
			/* class declaration might be changed independently */
			return 1;
		}
	}

	info = (crex_property_info*)crex_hash_find_ptr(&ce->properties_info, member);
	if (info == NULL ||
	    !IS_VALID_PROPERTY_OFFSET(info->offset) ||
	    (info->flags & CREX_ACC_STATIC)) {
		return 1;
	}

	if (!(info->flags & CREX_ACC_PUBLIC) &&
	    (!on_this || info->ce != ce)) {
		return 1;
	}

	return 0;
}

#define OP_RANGE(ssa_op, opN) \
	(((opline->opN##_type & (IS_TMP_VAR|IS_VAR|IS_CV)) && \
	  ssa->var_info && \
	  (ssa_op)->opN##_use >= 0 && \
	  ssa->var_info[(ssa_op)->opN##_use].has_range) ? \
	 &ssa->var_info[(ssa_op)->opN##_use].range : NULL)

#define OP1_RANGE()      OP_RANGE(ssa_op, op1)
#define OP2_RANGE()      OP_RANGE(ssa_op, op2)
#define OP1_DATA_RANGE() OP_RANGE(ssa_op + 1, op1)

#if CREX_JIT_TARGET_X86
# include "dynasm/dasm_x86.h"
#elif CREX_JIT_TARGET_ARM64
static int crex_jit_add_veneer(dasm_State *Dst, void *buffer, uint32_t ins, int *b, uint32_t *cp, ptrdiff_t offset);
# define DASM_ADD_VENEER crex_jit_add_veneer
# include "dynasm/dasm_arm64.h"
#endif

#include "jit/crex_jit_helpers.c"
#include "jit/crex_jit_disasm.c"
#ifndef _WIN32
# include "jit/crex_jit_gdb.h"
# include "jit/crex_jit_perf_dump.c"
#endif

#include "Crex/crex_cpuinfo.h"

#ifdef HAVE_VALGRIND
# include <valgrind/valgrind.h>
#endif

#ifdef HAVE_GCC_GLOBAL_REGS
# define GCC_GLOBAL_REGS 1
#else
# define GCC_GLOBAL_REGS 0
#endif

/* By default avoid JITing inline handlers if it does not seem profitable due to lack of
 * type information. Disabling this option allows testing some JIT handlers in the
 * presence of try/catch blocks, which prevent SSA construction. */
#ifndef PROFITABILITY_CHECKS
# define PROFITABILITY_CHECKS 1
#endif

#define BP_JIT_IS 6 /* Used for ISSET_ISEMPTY_DIM_OBJ. see BP_VAR_*defines in Crex/crex_compile.h */

typedef enum _sp_adj_kind {
	SP_ADJ_NONE,
	SP_ADJ_RET,
	SP_ADJ_VM,
	SP_ADJ_JIT,
	SP_ADJ_ASSIGN,
	SP_ADJ_LAST
} sp_adj_kind;

static int sp_adj[SP_ADJ_LAST];

/* The generated code may contain tautological comparisons, ignore them. */
#if defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wtautological-compare"
# pragma clang diagnostic ignored "-Wstring-compare"
#endif

#if CREX_JIT_TARGET_X86
# include "jit/crex_jit_vtune.c"
# include "jit/crex_jit_x86.c"
#elif CREX_JIT_TARGET_ARM64
# include "jit/crex_jit_arm64.c"
#endif

#if defined(__clang__)
# pragma clang diagnostic pop
#endif

#if _WIN32
# include <Windows.h>
#else
# include <sys/mman.h>
# if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#   define MAP_ANONYMOUS MAP_ANON
# endif
#endif

CREX_EXT_API void crex_jit_status(zval *ret)
{
	zval stats;
	array_init(&stats);
	add_assoc_bool(&stats, "enabled", JIT_G(enabled));
	add_assoc_bool(&stats, "on", JIT_G(on));
	add_assoc_long(&stats, "kind", JIT_G(trigger));
	add_assoc_long(&stats, "opt_level", JIT_G(opt_level));
	add_assoc_long(&stats, "opt_flags", JIT_G(opt_flags));
	if (dasm_buf) {
		add_assoc_long(&stats, "buffer_size", (char*)dasm_end - (char*)dasm_buf);
		add_assoc_long(&stats, "buffer_free", (char*)dasm_end - (char*)*dasm_ptr);
	} else {
		add_assoc_long(&stats, "buffer_size", 0);
		add_assoc_long(&stats, "buffer_free", 0);
	}
	add_assoc_zval(ret, "jit", &stats);
}

static crex_string *crex_jit_func_name(const crex_op_array *op_array)
{
	smart_str buf = {0};

	if (op_array->function_name) {
		if (op_array->scope) {
			smart_str_appends(&buf, JIT_PREFIX);
			smart_str_appendl(&buf, ZSTR_VAL(op_array->scope->name), ZSTR_LEN(op_array->scope->name));
			smart_str_appends(&buf, "::");
			smart_str_appendl(&buf, ZSTR_VAL(op_array->function_name), ZSTR_LEN(op_array->function_name));
			smart_str_0(&buf);
			return buf.s;
		} else {
			smart_str_appends(&buf, JIT_PREFIX);
			smart_str_appendl(&buf, ZSTR_VAL(op_array->function_name), ZSTR_LEN(op_array->function_name));
			smart_str_0(&buf);
			return buf.s;
		}
	} else if (op_array->filename) {
		smart_str_appends(&buf, JIT_PREFIX);
		smart_str_appendl(&buf, ZSTR_VAL(op_array->filename), ZSTR_LEN(op_array->filename));
		smart_str_0(&buf);
		return buf.s;
	} else {
		return NULL;
	}
}

#if CREX_DEBUG
static void handle_dasm_error(int ret) {
	switch (ret & 0xff000000u) {
		case DASM_S_NOMEM:
			fprintf(stderr, "DASM_S_NOMEM\n");
			break;
		case DASM_S_PHASE:
			fprintf(stderr, "DASM_S_PHASE\n");
			break;
		case DASM_S_MATCH_SEC:
			fprintf(stderr, "DASM_S_MATCH_SEC\n");
			break;
		case DASM_S_RANGE_I:
			fprintf(stderr, "DASM_S_RANGE_I\n");
			break;
		case DASM_S_RANGE_SEC:
			fprintf(stderr, "DASM_S_RANGE_SEC\n");
			break;
		case DASM_S_RANGE_LG:
			fprintf(stderr, "DASM_S_RANGE_LG\n");
			break;
		case DASM_S_RANGE_PC:
			fprintf(stderr, "DASM_S_RANGE_PC %d\n", ret & 0xffffffu);
			break;
#ifdef DASM_S_RANGE_VREG
		case DASM_S_RANGE_VREG:
			fprintf(stderr, "DASM_S_RANGE_VREG\n");
			break;
#endif
#ifdef DASM_S_UNDEF_L
		case DASM_S_UNDEF_L:
			fprintf(stderr, "DASM_S_UNDEF_L\n");
			break;
#endif
#ifdef DASM_S_UNDEF_LG
		case DASM_S_UNDEF_LG:
			fprintf(stderr, "DASM_S_UNDEF_LG\n");
			break;
#endif
#ifdef DASM_S_RANGE_REL
		case DASM_S_RANGE_REL:
			fprintf(stderr, "DASM_S_RANGE_REL\n");
			break;
#endif
		case DASM_S_UNDEF_PC:
			fprintf(stderr, "DASM_S_UNDEF_PC %d\n", ret & 0xffffffu);
			break;
		default:
			fprintf(stderr, "DASM_S_%0x\n", ret & 0xff000000u);
			break;
	}
	CREX_UNREACHABLE();
}
#endif

static void *dasm_link_and_encode(dasm_State             **dasm_state,
                                  const crex_op_array     *op_array,
                                  crex_ssa                *ssa,
                                  const crex_op           *rt_opline,
                                  crex_lifetime_interval **ra,
                                  const char              *name,
                                  uint32_t                 trace_num,
                                  uint32_t                 sp_offset,
                                  uint32_t                 sp_adjustment)
{
	size_t size;
	int ret;
	void *entry;
#if defined(HAVE_DISASM) || defined(HAVE_GDB) || defined(HAVE_PERFTOOLS) || defined(HAVE_VTUNE)
	crex_string *str = NULL;
#endif

	if (rt_opline && ssa && ssa->cfg.map) {
		/* Create additional entry point, to switch from interpreter to JIT-ed
		 * code at run-time.
		 */
		int b = ssa->cfg.map[rt_opline - op_array->opcodes];

//#ifdef CONTEXT_THREADED_JIT
//		if (!(ssa->cfg.blocks[b].flags & (CREX_BB_START|CREX_BB_RECV_ENTRY))) {
//#else
		if (!(ssa->cfg.blocks[b].flags & (CREX_BB_START|CREX_BB_ENTRY|CREX_BB_RECV_ENTRY))) {
//#endif
			crex_jit_label(dasm_state, ssa->cfg.blocks_count + b);
			crex_jit_prologue(dasm_state);
			if (ra) {
				int i;
				crex_lifetime_interval *ival;
				crex_life_range *range;
				uint32_t pos = rt_opline - op_array->opcodes;

				for (i = 0; i < ssa->vars_count; i++) {
					ival = ra[i];

					if (ival && ival->reg != ZREG_NONE) {
						range = &ival->range;

						if (pos >= range->start && pos <= range->end) {
							if (!crex_jit_load_var(dasm_state, ssa->var_info[i].type, ssa->vars[i].var, ival->reg)) {
								return NULL;
							}
							break;
						}
						range = range->next;
					}
				}
			}
			crex_jit_jmp(dasm_state, b);
		}
	}

	ret = dasm_link(dasm_state, &size);
	if (ret != DASM_S_OK) {
#if CREX_DEBUG
		handle_dasm_error(ret);
#endif
		return NULL;
	}

	if ((void*)((char*)*dasm_ptr + size) > dasm_end) {
		*dasm_ptr = dasm_end; //prevent further try
		// TODO: jit_buffer_size overflow ???
		return NULL;
	}

#if CREX_JIT_TARGET_ARM64
	dasm_venners_size = 0;
#endif

	ret = dasm_encode(dasm_state, *dasm_ptr);
	if (ret != DASM_S_OK) {
#if CREX_DEBUG
		handle_dasm_error(ret);
#endif
		return NULL;
	}

#if CREX_JIT_TARGET_ARM64
	size += dasm_venners_size;
#endif

	entry = *dasm_ptr;
	*dasm_ptr = (void*)((char*)*dasm_ptr + CREX_MM_ALIGNED_SIZE_EX(size, DASM_ALIGNMENT));

	/* flush the hardware I-cache */
	JIT_CACHE_FLUSH(entry, entry + size);
	/* hint to the hardware to push out the cache line that contains the linear address */
#if CREX_JIT_SUPPORT_CLDEMOTE
	if (cpu_support_cldemote && JIT_G(trigger) == CREX_JIT_ON_HOT_TRACE) {
		shared_cacheline_demote((uintptr_t)entry, size);
	}
#endif

	if (trace_num) {
		crex_jit_trace_add_code(entry, dasm_getpclabel(dasm_state, 1));
	}

	if (op_array && ssa) {
		int b;

		for (b = 0; b < ssa->cfg.blocks_count; b++) {
//#ifdef CONTEXT_THREADED_JIT
//			if (ssa->cfg.blocks[b].flags & (CREX_BB_START|CREX_BB_RECV_ENTRY)) {
//#else
			if (ssa->cfg.blocks[b].flags & (CREX_BB_START|CREX_BB_ENTRY|CREX_BB_RECV_ENTRY)) {
//#endif
				crex_op *opline = op_array->opcodes + ssa->cfg.blocks[b].start;
				int offset = dasm_getpclabel(dasm_state, ssa->cfg.blocks_count + b);

				if (offset >= 0) {
					opline->handler = (void*)(((char*)entry) + offset);
				}
			}
		}
	    if (rt_opline && ssa && ssa->cfg.map) {
			int b = ssa->cfg.map[rt_opline - op_array->opcodes];
			crex_op *opline = (crex_op*)rt_opline;
			int offset = dasm_getpclabel(dasm_state, ssa->cfg.blocks_count + b);

			if (offset >= 0) {
				opline->handler = (void*)(((char*)entry) + offset);
			}
		}
	}

#if defined(HAVE_DISASM) || defined(HAVE_GDB) || defined(HAVE_PERFTOOLS) || defined(HAVE_VTUNE)
	if (!name) {
		if (JIT_G(debug) & (CREX_JIT_DEBUG_ASM|CREX_JIT_DEBUG_GDB|CREX_JIT_DEBUG_PERF|CREX_JIT_DEBUG_VTUNE|CREX_JIT_DEBUG_PERF_DUMP)) {
			str = crex_jit_func_name(op_array);
			if (str) {
				name = ZSTR_VAL(str);
			}
		}
#ifdef HAVE_DISASM
	    if (JIT_G(debug) & CREX_JIT_DEBUG_ASM) {
			crex_jit_disasm_add_symbol(name, (uintptr_t)entry, size);
			crex_jit_disasm(
				name,
				(op_array && op_array->filename) ? ZSTR_VAL(op_array->filename) : NULL,
				op_array,
				&ssa->cfg,
				entry,
				size);
		}
	} else {
	    if (JIT_G(debug) & (CREX_JIT_DEBUG_ASM_STUBS|CREX_JIT_DEBUG_ASM)) {
			crex_jit_disasm_add_symbol(name, (uintptr_t)entry, size);
			if ((JIT_G(debug) & (trace_num ? CREX_JIT_DEBUG_ASM : CREX_JIT_DEBUG_ASM_STUBS)) != 0) {
				crex_jit_disasm(
					name,
					(op_array && op_array->filename) ? ZSTR_VAL(op_array->filename) : NULL,
					op_array,
					ssa ? &ssa->cfg : NULL,
					entry,
					size);
			}
		}
# endif
	}
#endif

#ifdef HAVE_GDB
	if (JIT_G(debug) & CREX_JIT_DEBUG_GDB) {
		if (name) {
			crex_jit_gdb_register(
					name,
					op_array,
					entry,
					size,
					sp_adj[sp_offset],
					sp_adj[sp_adjustment]);
		}
	}
#endif


#ifdef HAVE_PERFTOOLS
	if (JIT_G(debug) & (CREX_JIT_DEBUG_PERF|CREX_JIT_DEBUG_PERF_DUMP)) {
		if (name) {
			crex_jit_perf_map_register(
				name,
				entry,
				size);
			if (JIT_G(debug) & CREX_JIT_DEBUG_PERF_DUMP) {
				crex_jit_perf_jitdump_register(
					name,
					entry,
					size);
			}
		}
	}
#endif

#ifdef HAVE_VTUNE
	if (JIT_G(debug) & CREX_JIT_DEBUG_VTUNE) {
		if (name) {
			crex_jit_vtune_register(
				name,
				entry,
				size);
		}
	}
#endif

#if defined(HAVE_DISASM) || defined(HAVE_GDB) || defined(HAVE_PERFTOOLS) || defined(HAVE_VTUNE)
	if (str) {
		crex_string_release(str);
	}
#endif

	return entry;
}

static int crex_may_overflow(const crex_op *opline, const crex_ssa_op *ssa_op, const crex_op_array *op_array, crex_ssa *ssa)
{
	int res;
	crex_long op1_min, op1_max, op2_min, op2_max;

	if (!ssa->ops || !ssa->var_info) {
		return 1;
	}
	switch (opline->opcode) {
		case CREX_PRE_INC:
		case CREX_POST_INC:
			res = ssa_op->op1_def;
			if (res < 0
			 || !ssa->var_info[res].has_range
			 || ssa->var_info[res].range.overflow) {
				if (!OP1_HAS_RANGE()) {
					return 1;
				}
				op1_max = OP1_MAX_RANGE();
				if (op1_max == CREX_LONG_MAX) {
					return 1;
				}
			}
			return 0;
		case CREX_PRE_DEC:
		case CREX_POST_DEC:
			res = ssa_op->op1_def;
			if (res < 0
			 || !ssa->var_info[res].has_range
			 || ssa->var_info[res].range.underflow) {
				if (!OP1_HAS_RANGE()) {
					return 1;
				}
				op1_min = OP1_MIN_RANGE();
				if (op1_min == CREX_LONG_MIN) {
					return 1;
				}
			}
			return 0;
		case CREX_ADD:
			res = ssa_op->result_def;
			if (res < 0
			 || !ssa->var_info[res].has_range
			 || ssa->var_info[res].range.underflow) {
				if (!OP1_HAS_RANGE() || !OP2_HAS_RANGE()) {
					return 1;
				}
				op1_min = OP1_MIN_RANGE();
				op2_min = OP2_MIN_RANGE();
				if (crex_add_will_overflow(op1_min, op2_min)) {
					return 1;
				}
			}
			if (res < 0
			 || !ssa->var_info[res].has_range
			 || ssa->var_info[res].range.overflow) {
				if (!OP1_HAS_RANGE() || !OP2_HAS_RANGE()) {
					return 1;
				}
				op1_max = OP1_MAX_RANGE();
				op2_max = OP2_MAX_RANGE();
				if (crex_add_will_overflow(op1_max, op2_max)) {
					return 1;
				}
			}
			return 0;
		case CREX_SUB:
			res = ssa_op->result_def;
			if (res < 0
			 || !ssa->var_info[res].has_range
			 || ssa->var_info[res].range.underflow) {
				if (!OP1_HAS_RANGE() || !OP2_HAS_RANGE()) {
					return 1;
				}
				op1_min = OP1_MIN_RANGE();
				op2_max = OP2_MAX_RANGE();
				if (crex_sub_will_overflow(op1_min, op2_max)) {
					return 1;
				}
			}
			if (res < 0
			 || !ssa->var_info[res].has_range
			 || ssa->var_info[res].range.overflow) {
				if (!OP1_HAS_RANGE() || !OP2_HAS_RANGE()) {
					return 1;
				}
				op1_max = OP1_MAX_RANGE();
				op2_min = OP2_MIN_RANGE();
				if (crex_sub_will_overflow(op1_max, op2_min)) {
					return 1;
				}
			}
			return 0;
		case CREX_MUL:
			res = ssa_op->result_def;
			return (res < 0 ||
				!ssa->var_info[res].has_range ||
				ssa->var_info[res].range.underflow ||
				ssa->var_info[res].range.overflow);
		case CREX_ASSIGN_OP:
			if (opline->extended_value == CREX_ADD) {
				res = ssa_op->op1_def;
				if (res < 0
				 || !ssa->var_info[res].has_range
				 || ssa->var_info[res].range.underflow) {
					if (!OP1_HAS_RANGE() || !OP2_HAS_RANGE()) {
						return 1;
					}
					op1_min = OP1_MIN_RANGE();
					op2_min = OP2_MIN_RANGE();
					if (crex_add_will_overflow(op1_min, op2_min)) {
						return 1;
					}
				}
				if (res < 0
				 || !ssa->var_info[res].has_range
				 || ssa->var_info[res].range.overflow) {
					if (!OP1_HAS_RANGE() || !OP2_HAS_RANGE()) {
						return 1;
					}
					op1_max = OP1_MAX_RANGE();
					op2_max = OP2_MAX_RANGE();
					if (crex_add_will_overflow(op1_max, op2_max)) {
						return 1;
					}
				}
				return 0;
			} else if (opline->extended_value == CREX_SUB) {
				res = ssa_op->op1_def;
				if (res < 0
				 || !ssa->var_info[res].has_range
				 || ssa->var_info[res].range.underflow) {
					if (!OP1_HAS_RANGE() || !OP2_HAS_RANGE()) {
						return 1;
					}
					op1_min = OP1_MIN_RANGE();
					op2_max = OP2_MAX_RANGE();
					if (crex_sub_will_overflow(op1_min, op2_max)) {
						return 1;
					}
				}
				if (res < 0
				 || !ssa->var_info[res].has_range
				 || ssa->var_info[res].range.overflow) {
					if (!OP1_HAS_RANGE() || !OP2_HAS_RANGE()) {
						return 1;
					}
					op1_max = OP1_MAX_RANGE();
					op2_min = OP2_MIN_RANGE();
					if (crex_sub_will_overflow(op1_max, op2_min)) {
						return 1;
					}
				}
				return 0;
			} else if (opline->extended_value == CREX_MUL) {
				res = ssa_op->op1_def;
				return (res < 0 ||
					!ssa->var_info[res].has_range ||
					ssa->var_info[res].range.underflow ||
					ssa->var_info[res].range.overflow);
			}
			CREX_FALLTHROUGH;
		default:
			return 1;
	}
}

static int crex_jit_build_cfg(const crex_op_array *op_array, crex_cfg *cfg)
{
	uint32_t flags;

	flags = CREX_CFG_STACKLESS | CREX_CFG_NO_ENTRY_PREDECESSORS | CREX_SSA_RC_INFERENCE_FLAG | CREX_SSA_USE_CV_RESULTS | CREX_CFG_RECV_ENTRY;

	crex_build_cfg(&CG(arena), op_array, flags, cfg);

	/* Don't JIT huge functions. Apart from likely being detrimental due to the amount of
	 * generated code, some of our analysis is recursive and will stack overflow with many
	 * blocks. */
	if (cfg->blocks_count > 100000) {
		return FAILURE;
	}

	crex_cfg_build_predecessors(&CG(arena), cfg);

	/* Compute Dominators Tree */
	crex_cfg_compute_dominators_tree(op_array, cfg);

	/* Identify reducible and irreducible loops */
	crex_cfg_identify_loops(op_array, cfg);

	return SUCCESS;
}

static int crex_jit_op_array_analyze1(const crex_op_array *op_array, crex_script *script, crex_ssa *ssa)
{
	if (crex_jit_build_cfg(op_array, &ssa->cfg) != SUCCESS) {
		return FAILURE;
	}

#if 0
	/* TODO: debugger and profiler supports? */
	if ((ssa->cfg.flags & CREX_FUNC_HAS_EXTENDED_INFO)) {
		return FAILURE;
	}
#endif

	/* TODO: move this to crex_cfg.c ? */
	if (!op_array->function_name) {
		ssa->cfg.flags |= CREX_FUNC_INDIRECT_VAR_ACCESS;
	}

	if ((JIT_G(opt_level) >= CREX_JIT_LEVEL_OPT_FUNC)
	 && ssa->cfg.blocks
	 && op_array->last_try_catch == 0
	 && !(op_array->fn_flags & CREX_ACC_GENERATOR)
	 && !(ssa->cfg.flags & CREX_FUNC_INDIRECT_VAR_ACCESS)) {
		if (crex_build_ssa(&CG(arena), script, op_array, CREX_SSA_RC_INFERENCE | CREX_SSA_USE_CV_RESULTS, ssa) != SUCCESS) {
			return FAILURE;
		}

		crex_ssa_compute_use_def_chains(&CG(arena), op_array, ssa);

		crex_ssa_find_false_dependencies(op_array, ssa);

		crex_ssa_find_sccs(op_array, ssa);
	}

	return SUCCESS;
}

static int crex_jit_op_array_analyze2(const crex_op_array *op_array, crex_script *script, crex_ssa *ssa, uint32_t optimization_level)
{
	if ((JIT_G(opt_level) >= CREX_JIT_LEVEL_OPT_FUNC)
	 && ssa->cfg.blocks
	 && op_array->last_try_catch == 0
	 && !(op_array->fn_flags & CREX_ACC_GENERATOR)
	 && !(ssa->cfg.flags & CREX_FUNC_INDIRECT_VAR_ACCESS)) {
		if (crex_ssa_inference(&CG(arena), op_array, script, ssa,
				optimization_level & ~CREX_OPTIMIZER_NARROW_TO_DOUBLE) != SUCCESS) {
			return FAILURE;
		}
	}

	return SUCCESS;
}

static int crex_jit_add_range(crex_lifetime_interval **intervals, int var, uint32_t from, uint32_t to)
{
	crex_lifetime_interval *ival = intervals[var];

	if (!ival) {
		ival = crex_arena_alloc(&CG(arena), sizeof(crex_lifetime_interval));
		if (!ival) {
			return FAILURE;
		}
		ival->ssa_var = var;
		ival->reg = ZREG_NONE;
		ival->flags = 0;
		ival->range.start = from;
		ival->range.end = to;
		ival->range.next = NULL;
		ival->hint = NULL;
		ival->used_as_hint = NULL;
		intervals[var] = ival;
	} else if (ival->range.start > to + 1) {
		crex_life_range *range = crex_arena_alloc(&CG(arena), sizeof(crex_life_range));

		if (!range) {
			return FAILURE;
		}
		range->start = ival->range.start;
		range->end   = ival->range.end;
		range->next  = ival->range.next;
		ival->range.start = from;
		ival->range.end = to;
		ival->range.next = range;
	} else if (ival->range.start == to + 1) {
		ival->range.start = from;
	} else {
		crex_life_range *range = &ival->range;
		crex_life_range *last = NULL;

		do {
			if (range->start > to + 1) {
				break;
			} else if (range->end + 1 >= from) {
				if (range->start > from) {
					range->start = from;
				}
				last = range;
				range = range->next;
				while (range) {
					if (range->start > to + 1) {
						break;
					}
					last->end = range->end;
					range = range->next;
					last->next = range;
				}
				if (to > last->end) {
					last->end = to;
				}
				return SUCCESS;
			}
			last = range;
			range = range->next;
		} while (range);

		range = crex_arena_alloc(&CG(arena), sizeof(crex_life_range));
		if (!range) {
			return FAILURE;
		}
		range->start = from;
		range->end   = to;
		range->next  = last->next;
		last->next = range;
	}

	return SUCCESS;
}

static int crex_jit_begin_range(crex_lifetime_interval **intervals, int var, uint32_t block_start, uint32_t from)
{
	if (block_start != from && intervals[var]) {
		crex_life_range *range = &intervals[var]->range;

		do {
			if (from >= range->start && from <= range->end) {
				if (range->start == block_start) {
					range->start = from;
				} else {
					crex_life_range *r = crex_arena_alloc(&CG(arena), sizeof(crex_life_range));
					if (!r) {
						return FAILURE;
					}
					r->start = from;
					r->end = range->end;
					r->next = range->next;
					range->end = block_start - 1;
					range->next = r;
				}
				return SUCCESS;
			}
			range = range->next;
		} while (range);
	}

	// dead store
	return crex_jit_add_range(intervals, var, from, from);
}

static void crex_jit_insert_interval(crex_lifetime_interval **list, crex_lifetime_interval *ival)
{
	while (1) {
		if (*list == NULL) {
			*list = ival;
			ival->list_next = NULL;
			return;
		} else if (ival->range.start < (*list)->range.start) {
			ival->list_next = *list;
			*list = ival;
			return;
		}
		list = &(*list)->list_next;
	}
}

static int crex_jit_split_interval(crex_lifetime_interval *current, uint32_t pos, crex_lifetime_interval **list, crex_lifetime_interval **free)
{
	crex_lifetime_interval *ival;
	crex_life_range *range = &current->range;
	crex_life_range *prev = NULL;

	if (*free) {
		ival = *free;
		*free = ival->list_next;
	} else {
		ival = crex_arena_alloc(&CG(arena), sizeof(crex_lifetime_interval));

		if (!ival) {
			return FAILURE;
		}
	}

	current->flags |= ZREG_STORE;

	ival->ssa_var = current->ssa_var;
	ival->reg     = ZREG_NONE;
	ival->flags  |= ZREG_SPLIT | ZREG_LOAD;
	ival->flags  &= ~ZREG_STORE;
	ival->hint    = NULL;

	do {
		if (pos >= range->start && pos <= range->end) {
			break;
		}
		prev = range;
		range = range->next;
	} while(range);

	CREX_ASSERT(range != NULL);

	ival->range.start   = pos;
	ival->range.end     = range->end;
	ival->range.next    = range->next;

	if (pos == range->start) {
		CREX_ASSERT(prev != NULL);
		prev->next = NULL;
	} else {
		range->end = pos - 1;
	}

	crex_jit_insert_interval(list, ival);

	return SUCCESS;
}

static crex_lifetime_interval *crex_jit_sort_intervals(crex_lifetime_interval **intervals, int count)
{
	crex_lifetime_interval *list, *last;
	int i;

	list = NULL;
	i = 0;
	while (i < count) {
		list = intervals[i];
		i++;
		if (list) {
			last = list;
			last->list_next = NULL;
			break;
		}
	}

	while (i < count) {
		crex_lifetime_interval *ival = intervals[i];

		i++;
		if (ival) {
			if ((ival->range.start > last->range.start) ||
			    (ival->range.start == last->range.start &&
			     ((!ival->hint && last->hint && last->hint != ival) ||
			      ival->range.end > last->range.end))) {
				last->list_next = ival;
				last = ival;
				ival->list_next = NULL;
			} else {
				crex_lifetime_interval **p = &list;

				while (1) {
					if (*p == NULL) {
						*p = last = ival;
						ival->list_next = NULL;
						break;
					} else if ((ival->range.start < (*p)->range.start) ||
					           (ival->range.start == (*p)->range.start &&
					            ((ival->hint && !(*p)->hint && ival->hint != *p) ||
					             ival->range.end < (*p)->range.end))) {
						ival->list_next = *p;
						*p = ival;
						break;
					}
					p = &(*p)->list_next;
				}
			}
		}
	}

	return list;
}

static CREX_ATTRIBUTE_UNUSED void crex_jit_print_regset(crex_regset regset)
{
	crex_reg reg;
	int first = 1;

	CREX_REGSET_FOREACH(regset, reg) {
		if (first) {
			first = 0;
			fprintf(stderr, "%s", crex_reg_name[reg]);
		} else {
			fprintf(stderr, ", %s", crex_reg_name[reg]);
		}
	} CREX_REGSET_FOREACH_END();
}

static int *crex_jit_compute_block_order_int(crex_ssa *ssa, int n, int *block_order)
{
	crex_basic_block *b = ssa->cfg.blocks + n;

tail_call:
	*block_order = n;
	block_order++;

	n = b->children;
	while (n >= 0) {
		b = ssa->cfg.blocks + n;
		if (b->next_child < 0) {
			goto tail_call;
		}
		block_order = crex_jit_compute_block_order_int(ssa, n, block_order);
		n = b->next_child;
	}

	return block_order;
}

static int crex_jit_compute_block_order(crex_ssa *ssa, int *block_order)
{
	int *end = crex_jit_compute_block_order_int(ssa, 0, block_order);

	return end - block_order;
}

static bool crex_jit_in_loop(crex_ssa *ssa, int header, crex_basic_block *b)
{
	while (b->loop_header >= 0) {
		if (b->loop_header == header) {
			return 1;
		}
		b = ssa->cfg.blocks + b->loop_header;
	}
	return 0;
}

static void crex_jit_compute_loop_body(crex_ssa *ssa, int header, int n, crex_bitset loop_body)
{
	crex_basic_block *b = ssa->cfg.blocks + n;
	uint32_t i;

tail_call:
	if (b->len) {
		for (i = b->start; i < b->start + b->len; i++) {
			crex_bitset_incl(loop_body, i);
		}
	}

	n = b->children;
	while (n >= 0) {
		b = ssa->cfg.blocks + n;
		if (crex_jit_in_loop(ssa, header, b)) {
			if (b->next_child < 0) {
				goto tail_call;
			}
			crex_jit_compute_loop_body(ssa, header, n, loop_body);
		}
		n = b->next_child;
	}
}

static void crex_jit_add_hint(crex_lifetime_interval **intervals, int dst, int src)
{
	if (intervals[dst]->range.start < intervals[src]->range.start) {
		int tmp = src;
		src = dst;
		dst = tmp;
	}
	while (dst != src && intervals[dst]->hint) {
		if (intervals[dst]->hint->range.start < intervals[src]->range.start) {
			int tmp = src;
			src = intervals[dst]->hint->ssa_var;
			dst = tmp;
		} else {
			dst = intervals[dst]->hint->ssa_var;
		}
	}
	if (dst != src) {
		intervals[dst]->hint = intervals[src];
	}
}

/* See "Linear Scan Register Allocation on SSA Form", Christian Wimmer and
   Michael Franz, CGO'10 (2010), Figure 4. */
static int crex_jit_compute_liveness(const crex_op_array *op_array, crex_ssa *ssa, crex_bitset candidates, crex_lifetime_interval **list)
{
	int set_size, i, j, k, l;
	uint32_t n;
	crex_bitset live, live_in, pi_vars, loop_body;
	int *block_order;
	crex_ssa_phi *phi;
	crex_lifetime_interval **intervals;
	size_t mem_size;
	ALLOCA_FLAG(use_heap);

	set_size = crex_bitset_len(ssa->vars_count);
	mem_size =
		CREX_MM_ALIGNED_SIZE(ssa->vars_count * sizeof(crex_lifetime_interval*)) +
		CREX_MM_ALIGNED_SIZE((set_size * ssa->cfg.blocks_count) * CREX_BITSET_ELM_SIZE) +
		CREX_MM_ALIGNED_SIZE(set_size * CREX_BITSET_ELM_SIZE) +
		CREX_MM_ALIGNED_SIZE(set_size * CREX_BITSET_ELM_SIZE) +
		CREX_MM_ALIGNED_SIZE(crex_bitset_len(op_array->last) * CREX_BITSET_ELM_SIZE) +
		CREX_MM_ALIGNED_SIZE(ssa->cfg.blocks_count * sizeof(int));
	intervals = do_alloca(mem_size, use_heap);
	if (!intervals) {
		*list = NULL;
		return FAILURE;
	}

	live_in = (crex_bitset)((char*)intervals + CREX_MM_ALIGNED_SIZE(ssa->vars_count * sizeof(crex_lifetime_interval*)));
	live = (crex_bitset)((char*)live_in + CREX_MM_ALIGNED_SIZE((set_size * ssa->cfg.blocks_count) * CREX_BITSET_ELM_SIZE));
	pi_vars = (crex_bitset)((char*)live + CREX_MM_ALIGNED_SIZE(set_size * CREX_BITSET_ELM_SIZE));
	loop_body = (crex_bitset)((char*)pi_vars + CREX_MM_ALIGNED_SIZE(set_size * CREX_BITSET_ELM_SIZE));
	block_order = (int*)((char*)loop_body + CREX_MM_ALIGNED_SIZE(crex_bitset_len(op_array->last) * CREX_BITSET_ELM_SIZE));

	memset(intervals, 0, ssa->vars_count * sizeof(crex_lifetime_interval*));
	crex_bitset_clear(live_in, set_size * ssa->cfg.blocks_count);

	/* TODO: Provide a linear block order where all dominators of a block
	 * are before this block, and where all blocks belonging to the same loop
	 * are contiguous ???
	 */
	for (l = crex_jit_compute_block_order(ssa, block_order) - 1; l >= 0; l--) {
		crex_basic_block *b;

		i = block_order[l];
		b = ssa->cfg.blocks + i;

		/* live = UNION of successor.liveIn for each successor of b */
		/* live.add(phi.inputOf(b)) for each phi of successors of b */
		crex_bitset_clear(live, set_size);
		for (j = 0; j < b->successors_count; j++) {
			int succ = b->successors[j];

			crex_bitset_union(live, live_in + set_size * succ, set_size);
			crex_bitset_clear(pi_vars, set_size);
			for (phi = ssa->blocks[succ].phis; phi; phi = phi->next) {
				if (ssa->vars[phi->ssa_var].no_val) {
					/* skip */
				} else if (phi->pi >= 0) {
					if (phi->pi == i && phi->sources[0] >= 0) {
						if (crex_bitset_in(candidates, phi->sources[0])) {
							crex_bitset_incl(live, phi->sources[0]);
						}
						crex_bitset_incl(pi_vars, phi->var);
					}
				} else if (!crex_bitset_in(pi_vars, phi->var)) {
					for (k = 0; k < ssa->cfg.blocks[succ].predecessors_count; k++) {
						if (ssa->cfg.predecessors[ssa->cfg.blocks[succ].predecessor_offset + k] == i) {
							if (phi->sources[k] >= 0 && crex_bitset_in(candidates, phi->sources[k])) {
								crex_bitset_incl(live, phi->sources[k]);
							}
							break;
						}
					}
				}
			}
		}

		/* addRange(var, b.from, b.to) for each var in live */
		CREX_BITSET_FOREACH(live, set_size, j) {
			if (crex_bitset_in(candidates, j)) {
				if (crex_jit_add_range(intervals, j, b->start, b->start + b->len - 1) != SUCCESS) {
					goto failure;
				}
			}
		} CREX_BITSET_FOREACH_END();

		/* for each operation op of b in reverse order */
		for (n = b->start + b->len; n > b->start;) {
			crex_ssa_op *op;
			const crex_op *opline;
			uint32_t num;

			n--;
			op = ssa->ops + n;
			opline = op_array->opcodes + n;

			if (UNEXPECTED(opline->opcode == CREX_OP_DATA)) {
				num = n - 1;
			} else {
				num = n;
			}

			/* for each output operand opd of op do */
			/*   setFrom(opd, op)                   */
			/*   live.remove(opd)                   */
			if (op->op1_def >= 0 && crex_bitset_in(candidates, op->op1_def)) {
				if (crex_jit_begin_range(intervals, op->op1_def, b->start, num) != SUCCESS) {
					goto failure;
				}
				crex_bitset_excl(live, op->op1_def);
			}
			if (op->op2_def >= 0 && crex_bitset_in(candidates, op->op2_def)) {
				if (crex_jit_begin_range(intervals, op->op2_def, b->start, num) != SUCCESS) {
					goto failure;
				}
				crex_bitset_excl(live, op->op2_def);
			}
			if (op->result_def >= 0 && crex_bitset_in(candidates, op->result_def)) {
				if (crex_jit_begin_range(intervals, op->result_def, b->start, num) != SUCCESS) {
					goto failure;
				}
				crex_bitset_excl(live, op->result_def);
			}

			/* for each input operand opd of op do */
			/*   live.add(opd)                     */
			/*   addRange(opd, b.from, op)         */
			if (op->op1_use >= 0
			 && crex_bitset_in(candidates, op->op1_use)
			 && !crex_ssa_is_no_val_use(opline, op, op->op1_use)) {
				crex_bitset_incl(live, op->op1_use);
				if (crex_jit_add_range(intervals, op->op1_use, b->start, num) != SUCCESS) {
					goto failure;
				}
			}
			if (op->op2_use >= 0
			 && crex_bitset_in(candidates, op->op2_use)
			 && !crex_ssa_is_no_val_use(opline, op, op->op2_use)) {
				crex_bitset_incl(live, op->op2_use);
				if (crex_jit_add_range(intervals, op->op2_use, b->start, num) != SUCCESS) {
					goto failure;
				}
			}
			if (op->result_use >= 0
			 && crex_bitset_in(candidates, op->result_use)
			 && !crex_ssa_is_no_val_use(opline, op, op->result_use)) {
				crex_bitset_incl(live, op->result_use);
				if (crex_jit_add_range(intervals, op->result_use, b->start, num) != SUCCESS) {
					goto failure;
				}
			}
		}

		/* live.remove(phi.output) for each phi of b */
		for (phi = ssa->blocks[i].phis; phi; phi = phi->next) {
			crex_bitset_excl(live, phi->ssa_var);
		}

		/* b.liveIn = live */
		crex_bitset_copy(live_in + set_size * i, live, set_size);
	}

	for (i = ssa->cfg.blocks_count - 1; i >= 0; i--) {
		crex_basic_block *b = ssa->cfg.blocks + i;

		/* if b is loop header */
		if ((b->flags & CREX_BB_LOOP_HEADER)) {
			live = live_in + set_size * i;

			if (!crex_bitset_empty(live, set_size)) {
				uint32_t set_size2 = crex_bitset_len(op_array->last);

				crex_bitset_clear(loop_body, set_size2);
				crex_jit_compute_loop_body(ssa, i, i, loop_body);
				while (!crex_bitset_empty(loop_body, set_size2)) {
					uint32_t from = crex_bitset_first(loop_body, set_size2);
					uint32_t to = from;

					do {
						crex_bitset_excl(loop_body, to);
						to++;
					} while (crex_bitset_in(loop_body, to));
					to--;

					CREX_BITSET_FOREACH(live, set_size, j) {
						if (crex_jit_add_range(intervals, j, from, to) != SUCCESS) {
							goto failure;
						}
					} CREX_BITSET_FOREACH_END();
				}
			}
		}

	}

	if (JIT_G(opt_flags) & CREX_JIT_REG_ALLOC_GLOBAL) {
		/* Register hinting (a cheap way for register coalescing) */
		for (i = 0; i < ssa->vars_count; i++) {
			if (intervals[i]) {
				int src;

				if (ssa->vars[i].definition_phi) {
					crex_ssa_phi *phi = ssa->vars[i].definition_phi;

					if (phi->pi >= 0) {
						src = phi->sources[0];
						if (intervals[src]) {
							crex_jit_add_hint(intervals, i, src);
						}
					} else {
						for (k = 0; k < ssa->cfg.blocks[phi->block].predecessors_count; k++) {
							src = phi->sources[k];
							if (src >= 0) {
								if (ssa->vars[src].definition_phi
								 && ssa->vars[src].definition_phi->pi >= 0
								 && phi->block == ssa->vars[src].definition_phi->block) {
									/* Skip zero-length interval for Pi variable */
									src = ssa->vars[src].definition_phi->sources[0];
								}
								if (intervals[src]) {
									crex_jit_add_hint(intervals, i, src);
								}
							}
						}
					}
				}
			}
		}
		for (i = 0; i < ssa->vars_count; i++) {
			if (intervals[i] && !intervals[i]->hint) {

				if (ssa->vars[i].definition >= 0) {
					uint32_t line = ssa->vars[i].definition;
					const crex_op *opline = op_array->opcodes + line;

					switch (opline->opcode) {
						case CREX_QM_ASSIGN:
						case CREX_POST_INC:
						case CREX_POST_DEC:
							if (ssa->ops[line].op1_use >= 0 &&
							    intervals[ssa->ops[line].op1_use] &&
							    (i == ssa->ops[line].op1_def ||
							     (i == ssa->ops[line].result_def &&
							      (ssa->ops[line].op1_def < 0 ||
							       !intervals[ssa->ops[line].op1_def])))) {
								crex_jit_add_hint(intervals, i, ssa->ops[line].op1_use);
							}
							break;
						case CREX_SEND_VAR:
						case CREX_PRE_INC:
						case CREX_PRE_DEC:
							if (i == ssa->ops[line].op1_def &&
							    ssa->ops[line].op1_use >= 0 &&
							    intervals[ssa->ops[line].op1_use]) {
								crex_jit_add_hint(intervals, i, ssa->ops[line].op1_use);
							}
							break;
						case CREX_ASSIGN:
							if (ssa->ops[line].op2_use >= 0 &&
							    intervals[ssa->ops[line].op2_use] &&
							    (i == ssa->ops[line].op2_def ||
								 (i == ssa->ops[line].op1_def &&
							      (ssa->ops[line].op2_def < 0 ||
							       !intervals[ssa->ops[line].op2_def])) ||
								 (i == ssa->ops[line].result_def &&
							      (ssa->ops[line].op2_def < 0 ||
							       !intervals[ssa->ops[line].op2_def]) &&
							      (ssa->ops[line].op1_def < 0 ||
							       !intervals[ssa->ops[line].op1_def])))) {
								crex_jit_add_hint(intervals, i, ssa->ops[line].op2_use);
							}
							break;
						case CREX_SUB:
						case CREX_ADD:
						case CREX_MUL:
						case CREX_BW_OR:
						case CREX_BW_AND:
						case CREX_BW_XOR:
							if (i == ssa->ops[line].result_def) {
								if (ssa->ops[line].op1_use >= 0 &&
								    intervals[ssa->ops[line].op1_use] &&
								    ssa->ops[line].op1_use_chain < 0 &&
								    !ssa->vars[ssa->ops[line].op1_use].phi_use_chain &&
								    (ssa->var_info[i].type & MAY_BE_ANY) ==
								        (ssa->var_info[ssa->ops[line].op1_use].type & MAY_BE_ANY)) {
									crex_jit_add_hint(intervals, i, ssa->ops[line].op1_use);
								} else if (opline->opcode != CREX_SUB &&
								    ssa->ops[line].op2_use >= 0 &&
								    intervals[ssa->ops[line].op2_use] &&
								    ssa->ops[line].op2_use_chain < 0 &&
								    !ssa->vars[ssa->ops[line].op2_use].phi_use_chain &&
								    (ssa->var_info[i].type & MAY_BE_ANY) ==
								        (ssa->var_info[ssa->ops[line].op2_use].type & MAY_BE_ANY)) {
									crex_jit_add_hint(intervals, i, ssa->ops[line].op2_use);
								}
							}
							break;
					}
				}
			}
		}
	}

	*list = crex_jit_sort_intervals(intervals, ssa->vars_count);

	if (*list) {
		crex_lifetime_interval *ival = *list;
		while (ival) {
			if (ival->hint) {
				ival->hint->used_as_hint = ival;
			}
			ival = ival->list_next;
		}
	}

	free_alloca(intervals, use_heap);
	return SUCCESS;

failure:
	*list = NULL;
	free_alloca(intervals, use_heap);
	return FAILURE;
}

static uint32_t crex_interval_end(crex_lifetime_interval *ival)
{
	crex_life_range *range = &ival->range;

	while (range->next) {
		range = range->next;
	}
	return range->end;
}

static bool crex_interval_covers(crex_lifetime_interval *ival, uint32_t position)
{
	crex_life_range *range = &ival->range;

	do {
		if (position >= range->start && position <= range->end) {
			return 1;
		}
		range = range->next;
	} while (range);

	return 0;
}

static uint32_t crex_interval_intersection(crex_lifetime_interval *ival1, crex_lifetime_interval *ival2)
{
	crex_life_range *r1 = &ival1->range;
	crex_life_range *r2 = &ival2->range;

	do {
		if (r1->start <= r2->end) {
			if (r2->start <= r1->end) {
				return MAX(r1->start, r2->start);
			} else {
				r2 = r2->next;
			}
		} else {
			r1 = r1->next;
		}
	} while (r1 && r2);

	return 0xffffffff;
}

/* See "Optimized Interval Splitting in a Linear Scan Register Allocator",
   Christian Wimmer VEE'05 (2005), Figure 4. Allocation without spilling */
static int crex_jit_try_allocate_free_reg(const crex_op_array *op_array, const crex_op **ssa_opcodes, crex_ssa *ssa, crex_lifetime_interval *current, crex_regset available, crex_regset *hints, crex_lifetime_interval *active, crex_lifetime_interval *inactive, crex_lifetime_interval **list, crex_lifetime_interval **free)
{
	crex_lifetime_interval *it;
	uint32_t freeUntilPos[ZREG_NUM];
	uint32_t pos, pos2;
	crex_reg i, reg, reg2;
	crex_reg hint = ZREG_NONE;
	crex_regset low_priority_regs;
	crex_life_range *range;

	if ((ssa->var_info[current->ssa_var].type & MAY_BE_ANY) == MAY_BE_DOUBLE) {
		available = CREX_REGSET_INTERSECTION(available, CREX_REGSET_FP);
	} else {
		available = CREX_REGSET_INTERSECTION(available, CREX_REGSET_GP);
	}

	/* TODO: Allow usage of preserved registers ???
	 * Their values have to be stored in prologue and restored in epilogue
	 */
	available = CREX_REGSET_DIFFERENCE(available, CREX_REGSET_PRESERVED);

	/* Set freeUntilPos of all physical registers to maxInt */
	for (i = 0; i < ZREG_NUM; i++) {
		freeUntilPos[i] = 0xffffffff;
	}

	/* for each interval it in active do */
	/*   freeUntilPos[it.reg] = 0        */
	it = active;
	if (ssa->vars[current->ssa_var].definition == current->range.start) {
		while (it) {
			if (current->range.start != crex_interval_end(it)) {
				freeUntilPos[it->reg] = 0;
			} else if (crex_jit_may_reuse_reg(
					ssa_opcodes ? ssa_opcodes[current->range.start] : op_array->opcodes + current->range.start,
					ssa->ops + current->range.start, ssa, current->ssa_var, it->ssa_var)) {
				if (!CREX_REGSET_IN(*hints, it->reg) &&
				    /* TODO: Avoid most often scratch registers. Find a better way ??? */
				    (!current->used_as_hint ||
				     !CREX_REGSET_IN(CREX_REGSET_LOW_PRIORITY, it->reg))) {
					hint = it->reg;
				}
			} else {
				freeUntilPos[it->reg] = 0;
			}
			it = it->list_next;
		}
	} else {
		while (it) {
			freeUntilPos[it->reg] = 0;
			it = it->list_next;
		}
	}
	if (current->hint) {
		hint = current->hint->reg;
		if (hint != ZREG_NONE && current->hint->used_as_hint == current) {
			CREX_REGSET_EXCL(*hints, hint);
		}
	}

	if (hint == ZREG_NONE && CREX_REGSET_IS_EMPTY(available)) {
		return 0;
	}

	/* See "Linear Scan Register Allocation on SSA Form", Christian Wimmer and
	   Michael Franz, CGO'10 (2010), Figure 6. */
	if (current->flags & ZREG_SPLIT) {
		/* for each interval it in inactive intersecting with current do */
		/*   freeUntilPos[it.reg] = next intersection of it with current */
		it = inactive;
		while (it) {
			uint32_t next = crex_interval_intersection(current, it);

			//CREX_ASSERT(next != 0xffffffff && !current->split);
			if (next < freeUntilPos[it->reg]) {
				freeUntilPos[it->reg] = next;
			}
			it = it->list_next;
		}
	}

	/* Handle Scratch Registers */
	/* TODO: Optimize ??? */
	range = &current->range;
	do {
		uint32_t line = range->start;
		uint32_t last_use_line = (uint32_t)-1;
		crex_regset regset;
		crex_reg reg;

		if ((current->flags & ZREG_LAST_USE) && !range->next) {
			last_use_line = range->end;
		}
		if (ssa->ops[line].op1_def == current->ssa_var ||
		    ssa->ops[line].op2_def == current->ssa_var ||
		    ssa->ops[line].result_def == current->ssa_var) {
			regset = crex_jit_get_def_scratch_regset(
				ssa_opcodes ? ssa_opcodes[line] : op_array->opcodes + line,
				ssa->ops + line,
				op_array, ssa, current->ssa_var, line == last_use_line);
			CREX_REGSET_FOREACH(regset, reg) {
				if (line < freeUntilPos[reg]) {
					freeUntilPos[reg] = line;
				}
			} CREX_REGSET_FOREACH_END();
			line++;
		}
		while (line <= range->end) {
			regset = crex_jit_get_scratch_regset(
				ssa_opcodes ? ssa_opcodes[line] : op_array->opcodes + line,
				ssa->ops + line,
				op_array, ssa, current->ssa_var, line == last_use_line);
			CREX_REGSET_FOREACH(regset, reg) {
				if (line < freeUntilPos[reg]) {
					freeUntilPos[reg] = line;
				}
			} CREX_REGSET_FOREACH_END();
			line++;
		}
		range = range->next;
	} while (range);

#if 0
	/* Coalescing */
	if (ssa->vars[current->ssa_var].definition == current->start) {
		crex_op *opline = op_array->opcodes + current->start;
		int hint = -1;

		switch (opline->opcode) {
			case CREX_ASSIGN:
				hint = ssa->ops[current->start].op2_use;
			case CREX_QM_ASSIGN:
				hint = ssa->ops[current->start].op1_use;
				break;
			case CREX_ADD:
			case CREX_SUB:
			case CREX_MUL:
				hint = ssa->ops[current->start].op1_use;
				break;
			case CREX_ASSIGN_OP:
				if (opline->extended_value == CREX_ADD
				 || opline->extended_value == CREX_SUB
				 || opline->extended_value == CREX_MUL) {
					hint = ssa->ops[current->start].op1_use;
				}
				break;
		}
		if (hint >= 0) {
		}
	}
#endif

	if (hint != ZREG_NONE && freeUntilPos[hint] > crex_interval_end(current)) {
		current->reg = hint;
		if (current->used_as_hint) {
			CREX_REGSET_INCL(*hints, hint);
		}
		return 1;
	}

	if (CREX_REGSET_IS_EMPTY(available)) {
		return 0;
	}

	pos = 0; reg = ZREG_NONE;
	pos2 = 0; reg2 = ZREG_NONE;
	low_priority_regs = *hints;
	if (current->used_as_hint) {
		/* TODO: Avoid most often scratch registers. Find a better way ??? */
		low_priority_regs = CREX_REGSET_UNION(low_priority_regs, CREX_REGSET_LOW_PRIORITY);
	}

	CREX_REGSET_FOREACH(available, i) {
		if (CREX_REGSET_IN(low_priority_regs, i)) {
			if (freeUntilPos[i] > pos2) {
				reg2 = i;
				pos2 = freeUntilPos[i];
			}
		} else if (freeUntilPos[i] > pos) {
			reg = i;
			pos = freeUntilPos[i];
		}
	} CREX_REGSET_FOREACH_END();

	if (reg == ZREG_NONE) {
		if (reg2 != ZREG_NONE) {
			reg = reg2;
			pos = pos2;
			reg2 = ZREG_NONE;
		}
	}

	if (reg == ZREG_NONE) {
		/* no register available without spilling */
		return 0;
	} else if (crex_interval_end(current) < pos) {
		/* register available for the whole interval */
		current->reg = reg;
		if (current->used_as_hint) {
			CREX_REGSET_INCL(*hints, reg);
		}
		return 1;
#if 0
	// TODO: allow low priority register usage
	} else if (reg2 != ZREG_NONE && crex_interval_end(current) < pos2) {
		/* register available for the whole interval */
		current->reg = reg2;
		if (current->used_as_hint) {
			CREX_REGSET_INCL(*hints, reg2);
		}
		return 1;
#endif
	} else {
		/* TODO: enable interval splitting ??? */
		/* register available for the first part of the interval */
		if (1 || crex_jit_split_interval(current, pos, list, free) != SUCCESS) {
			return 0;
		}
		current->reg = reg;
		if (current->used_as_hint) {
			CREX_REGSET_INCL(*hints, reg);
		}
		return 1;
	}
}

/* See "Optimized Interval Splitting in a Linear Scan Register Allocator",
   Christian Wimmer VEE'05 (2005), Figure 5. Allocation with spilling.
   and "Linear Scan Register Allocation on SSA Form", Christian Wimmer and
   Michael Franz, CGO'10 (2010), Figure 6. */
static int crex_jit_allocate_blocked_reg(void)
{
	/* TODO: ??? */
	return 0;
}

/* See "Optimized Interval Splitting in a Linear Scan Register Allocator",
   Christian Wimmer VEE'10 (2005), Figure 2. */
static crex_lifetime_interval* crex_jit_linear_scan(const crex_op_array *op_array, const crex_op **ssa_opcodes, crex_ssa *ssa, crex_lifetime_interval *list)
{
	crex_lifetime_interval *unhandled, *active, *inactive, *handled, *free;
	crex_lifetime_interval *current, **p, *q;
	uint32_t position;
	crex_regset available = CREX_REGSET_UNION(CREX_REGSET_GP, CREX_REGSET_FP);
	crex_regset hints = CREX_REGSET_EMPTY;

	unhandled = list;
	/* active = inactive = handled = free = {} */
	active = inactive = handled = free = NULL;
	while (unhandled != NULL) {
		current = unhandled;
		unhandled = unhandled->list_next;
		position = current->range.start;

		p = &active;
		while (*p) {
			uint32_t end = crex_interval_end(*p);

			q = *p;
			if (end < position) {
				/* move ival from active to handled */
				CREX_REGSET_INCL(available, q->reg);
				*p = q->list_next;
				q->list_next = handled;
				handled = q;
			} else if (!crex_interval_covers(q, position)) {
				/* move ival from active to inactive */
				CREX_REGSET_INCL(available, q->reg);
				*p = q->list_next;
				q->list_next = inactive;
				inactive = q;
			} else {
				p = &q->list_next;
			}
		}

		p = &inactive;
		while (*p) {
			uint32_t end = crex_interval_end(*p);

			q = *p;
			if (end < position) {
				/* move ival from inactive to handled */
				*p = q->list_next;
				q->list_next = handled;
				handled = q;
			} else if (crex_interval_covers(q, position)) {
				/* move ival from inactive to active */
				CREX_REGSET_EXCL(available, q->reg);
				*p = q->list_next;
				q->list_next = active;
				active = q;
			} else {
				p = &q->list_next;
			}
		}

		if (crex_jit_try_allocate_free_reg(op_array, ssa_opcodes, ssa, current, available, &hints, active, inactive, &unhandled, &free) ||
		    crex_jit_allocate_blocked_reg()) {
			CREX_REGSET_EXCL(available, current->reg);
			current->list_next = active;
			active = current;
		} else {
			current->list_next = free;
			free = current;
		}
	}

	/* move active to handled */
	while (active) {
		current = active;
		active = active->list_next;
		current->list_next = handled;
		handled = current;
	}

	/* move inactive to handled */
	while (inactive) {
		current = inactive;
		inactive = inactive->list_next;
		current->list_next = handled;
		handled = current;
	}

	return handled;
}

static void crex_jit_dump_lifetime_interval(const crex_op_array *op_array, const crex_ssa *ssa, const crex_lifetime_interval *ival)
{
	crex_life_range *range;
	int var_num = ssa->vars[ival->ssa_var].var;

	fprintf(stderr, "#%d.", ival->ssa_var);
	crex_dump_var(op_array, (var_num < op_array->last_var ? IS_CV : 0), var_num);
	fprintf(stderr, ": %u-%u", ival->range.start, ival->range.end);
	range = ival->range.next;
	while (range) {
		fprintf(stderr, ", %u-%u", range->start, range->end);
		range = range->next;
	}
	if (ival->reg != ZREG_NONE) {
		fprintf(stderr, " (%s)", crex_reg_name[ival->reg]);
	}
	if (ival->flags & ZREG_LAST_USE) {
		fprintf(stderr, " last_use");
	}
	if (ival->flags & ZREG_LOAD) {
		fprintf(stderr, " load");
	}
	if (ival->flags & ZREG_STORE) {
		fprintf(stderr, " store");
	}
	if (ival->hint) {
		fprintf(stderr, " hint");
		if (ival->hint->ssa_var >= 0) {
			var_num = ssa->vars[ival->hint->ssa_var].var;
			fprintf(stderr, "=#%d.", ival->hint->ssa_var);
			crex_dump_var(op_array, (var_num < op_array->last_var ? IS_CV : 0), var_num);
		}
		if (ival->hint->reg != ZREG_NONE) {
			fprintf(stderr, " (%s)", crex_reg_name[ival->hint->reg]);
		}
	}
	fprintf(stderr, "\n");
}

static crex_lifetime_interval** crex_jit_allocate_registers(const crex_op_array *op_array, crex_ssa *ssa)
{
	void *checkpoint;
	int set_size, candidates_count, i;
	crex_bitset candidates = NULL;
	crex_lifetime_interval *list, *ival;
	crex_lifetime_interval **intervals;
	ALLOCA_FLAG(use_heap);

	if (!ssa->var_info) {
		return NULL;
	}

	/* Identify SSA variables suitable for register allocation */
	set_size = crex_bitset_len(ssa->vars_count);
	candidates = CREX_BITSET_ALLOCA(set_size, use_heap);
	if (!candidates) {
		return NULL;
	}
	candidates_count = 0;
	crex_bitset_clear(candidates, set_size);
	for (i = 0; i < ssa->vars_count; i++) {
		if (crex_jit_may_be_in_reg(op_array, ssa, i)) {
			crex_bitset_incl(candidates, i);
			candidates_count++;
		}
	}
	if (!candidates_count) {
		free_alloca(candidates, use_heap);
		return NULL;
	}

	checkpoint = crex_arena_checkpoint(CG(arena));

	/* Find life-time intervals */
	if (crex_jit_compute_liveness(op_array, ssa, candidates, &list) != SUCCESS) {
		goto failure;
	}

	if (list) {
		/* Set ZREG_LAST_USE flags */
		ival = list;
		while (ival) {
			crex_life_range *range = &ival->range;

			while (range->next) {
				range = range->next;
			}
			if (crex_ssa_is_last_use(op_array, ssa, ival->ssa_var, range->end)) {
				ival->flags |= ZREG_LAST_USE;
			}
			ival = ival->list_next;
		}
	}

	if (list) {
		if (JIT_G(debug) & CREX_JIT_DEBUG_REG_ALLOC) {
			fprintf(stderr, "Live Ranges \"%s\"\n", op_array->function_name ? ZSTR_VAL(op_array->function_name) : "[main]");
			ival = list;
			while (ival) {
				crex_jit_dump_lifetime_interval(op_array, ssa, ival);
				ival = ival->list_next;
			}
			fprintf(stderr, "\n");
		}

		/* Linear Scan Register Allocation */
		list = crex_jit_linear_scan(op_array, NULL, ssa, list);

		if (list) {
			intervals = crex_arena_calloc(&CG(arena), ssa->vars_count, sizeof(crex_lifetime_interval*));
			if (!intervals) {
				goto failure;
			}

			ival = list;
			while (ival != NULL) {
				crex_lifetime_interval *next = ival->list_next;

				ival->list_next = intervals[ival->ssa_var];
				intervals[ival->ssa_var] = ival;
				ival = next;
			}

			if (JIT_G(opt_flags) & CREX_JIT_REG_ALLOC_GLOBAL) {
				/* Naive SSA resolution */
				for (i = 0; i < ssa->vars_count; i++) {
					if (ssa->vars[i].definition_phi && !ssa->vars[i].no_val) {
						crex_ssa_phi *phi = ssa->vars[i].definition_phi;
						int k, src;

						if (phi->pi >= 0) {
							if (!ssa->vars[i].phi_use_chain
							 || ssa->vars[i].phi_use_chain->block != phi->block) {
								src = phi->sources[0];
								if (intervals[i]) {
									if (!intervals[src]) {
										intervals[i]->flags |= ZREG_LOAD;
									} else if (intervals[i]->reg != intervals[src]->reg) {
										intervals[i]->flags |= ZREG_LOAD;
										intervals[src]->flags |= ZREG_STORE;
									}
								} else if (intervals[src]) {
									intervals[src]->flags |= ZREG_STORE;
								}
							}
						} else {
							int need_move = 0;

							for (k = 0; k < ssa->cfg.blocks[phi->block].predecessors_count; k++) {
								src = phi->sources[k];
								if (src >= 0) {
									if (ssa->vars[src].definition_phi
									 && ssa->vars[src].definition_phi->pi >= 0
									 && phi->block == ssa->vars[src].definition_phi->block) {
										/* Skip zero-length interval for Pi variable */
										src = ssa->vars[src].definition_phi->sources[0];
									}
									if (intervals[i]) {
										if (!intervals[src]) {
											need_move = 1;
										} else if (intervals[i]->reg != intervals[src]->reg) {
											need_move = 1;
										}
									} else if (intervals[src]) {
										need_move = 1;
									}
								}
							}
							if (need_move) {
								if (intervals[i]) {
									intervals[i]->flags |= ZREG_LOAD;
								}
								for (k = 0; k < ssa->cfg.blocks[phi->block].predecessors_count; k++) {
									src = phi->sources[k];
									if (src >= 0) {
										if (ssa->vars[src].definition_phi
										 && ssa->vars[src].definition_phi->pi >= 0
										 && phi->block == ssa->vars[src].definition_phi->block) {
											/* Skip zero-length interval for Pi variable */
											src = ssa->vars[src].definition_phi->sources[0];
										}
										if (intervals[src]) {
											intervals[src]->flags |= ZREG_STORE;
										}
									}
								}
							}
						}
					}
				}
				/* Remove useless register allocation */
				for (i = 0; i < ssa->vars_count; i++) {
					if (intervals[i] &&
					    ((intervals[i]->flags & ZREG_LOAD) ||
					     ((intervals[i]->flags & ZREG_STORE) && ssa->vars[i].definition >= 0)) &&
					    ssa->vars[i].use_chain < 0) {
					    bool may_remove = 1;
						crex_ssa_phi *phi = ssa->vars[i].phi_use_chain;

						while (phi) {
							if (intervals[phi->ssa_var] &&
							    !(intervals[phi->ssa_var]->flags & ZREG_LOAD)) {
								may_remove = 0;
								break;
							}
							phi = crex_ssa_next_use_phi(ssa, i, phi);
						}
						if (may_remove) {
							intervals[i] = NULL;
						}
					}
				}
				/* Remove intervals used once */
				for (i = 0; i < ssa->vars_count; i++) {
					if (intervals[i] &&
					    (intervals[i]->flags & ZREG_LOAD) &&
					    (intervals[i]->flags & ZREG_STORE) &&
					    (ssa->vars[i].use_chain < 0 ||
					     crex_ssa_next_use(ssa->ops, i, ssa->vars[i].use_chain) < 0)) {
						bool may_remove = 1;
						crex_ssa_phi *phi = ssa->vars[i].phi_use_chain;

						while (phi) {
							if (intervals[phi->ssa_var] &&
							    !(intervals[phi->ssa_var]->flags & ZREG_LOAD)) {
								may_remove = 0;
								break;
							}
							phi = crex_ssa_next_use_phi(ssa, i, phi);
						}
						if (may_remove) {
							intervals[i] = NULL;
						}
					}
				}
			}

			if (JIT_G(debug) & CREX_JIT_DEBUG_REG_ALLOC) {
				fprintf(stderr, "Allocated Live Ranges \"%s\"\n", op_array->function_name ? ZSTR_VAL(op_array->function_name) : "[main]");
				for (i = 0; i < ssa->vars_count; i++) {
					ival = intervals[i];
					while (ival) {
						crex_jit_dump_lifetime_interval(op_array, ssa, ival);
						ival = ival->list_next;
					}
				}
				fprintf(stderr, "\n");
			}

			free_alloca(candidates, use_heap);
			return intervals;
		}
	}

failure:
	crex_arena_release(&CG(arena), checkpoint);
	free_alloca(candidates, use_heap);
	return NULL;
}

static bool crex_jit_next_is_send_result(const crex_op *opline)
{
	if (opline->result_type == IS_TMP_VAR
	 && (opline+1)->opcode == CREX_SEND_VAL
	 && (opline+1)->op1_type == IS_TMP_VAR
	 && (opline+1)->op2_type != IS_CONST
	 && (opline+1)->op1.var == opline->result.var) {
		return 1;
	}
	return 0;
}

static bool crex_jit_supported_binary_op(uint8_t op, uint32_t op1_info, uint32_t op2_info)
{
	if ((op1_info & MAY_BE_UNDEF) || (op2_info & MAY_BE_UNDEF)) {
		return false;
	}
	switch (op) {
		case CREX_POW:
		case CREX_DIV:
			// TODO: check for division by zero ???
			return false;
		case CREX_ADD:
		case CREX_SUB:
		case CREX_MUL:
			return (op1_info & (MAY_BE_LONG|MAY_BE_DOUBLE))
				&& (op2_info & (MAY_BE_LONG|MAY_BE_DOUBLE));
		case CREX_BW_OR:
		case CREX_BW_AND:
		case CREX_BW_XOR:
		case CREX_SL:
		case CREX_SR:
		case CREX_MOD:
			return (op1_info & MAY_BE_LONG) && (op2_info & MAY_BE_LONG);
		case CREX_CONCAT:
			return (op1_info & MAY_BE_STRING) && (op2_info & MAY_BE_STRING);
		EMPTY_SWITCH_DEFAULT_CASE()
	}
}

static int crex_jit(const crex_op_array *op_array, crex_ssa *ssa, const crex_op *rt_opline)
{
	int b, i, end;
	crex_op *opline;
	dasm_State* dasm_state = NULL;
	void *handler;
	int call_level = 0;
	void *checkpoint = NULL;
	crex_lifetime_interval **ra = NULL;
	bool is_terminated = 1; /* previous basic block is terminated by jump */
	bool recv_emitted = 0;   /* emitted at least one RECV opcode */
	uint8_t smart_branch_opcode;
	uint32_t target_label, target_label2;
	uint32_t op1_info, op1_def_info, op2_info, res_info, res_use_info;
	crex_jit_addr op1_addr, op1_def_addr, op2_addr, op2_def_addr, res_addr;
	crex_class_entry *ce;
	bool ce_is_instanceof;
	bool on_this;

	if (JIT_G(bisect_limit)) {
		jit_bisect_pos++;
		if (jit_bisect_pos >= JIT_G(bisect_limit)) {
			if (jit_bisect_pos == JIT_G(bisect_limit)) {
				fprintf(stderr, "Not JITing %s%s%s in %s:%d and after due to jit_bisect_limit\n",
					op_array->scope ? ZSTR_VAL(op_array->scope->name) : "",
					op_array->scope ? "::" : "",
					op_array->function_name ? ZSTR_VAL(op_array->function_name) : "{main}",
					ZSTR_VAL(op_array->filename), op_array->line_start);
			}
			return FAILURE;
		}
	}

	if (JIT_G(opt_flags) & (CREX_JIT_REG_ALLOC_LOCAL|CREX_JIT_REG_ALLOC_GLOBAL)) {
		checkpoint = crex_arena_checkpoint(CG(arena));
		ra = crex_jit_allocate_registers(op_array, ssa);
	}

	/* mark hidden branch targets */
	for (b = 0; b < ssa->cfg.blocks_count; b++) {
		if (ssa->cfg.blocks[b].flags & CREX_BB_REACHABLE &&
		    ssa->cfg.blocks[b].len > 1) {

			opline = op_array->opcodes + ssa->cfg.blocks[b].start + ssa->cfg.blocks[b].len - 1;
			if (opline->opcode == CREX_DO_FCALL &&
			    (opline-1)->opcode == CREX_NEW) {
				ssa->cfg.blocks[ssa->cfg.blocks[b].successors[0]].flags |= CREX_BB_TARGET;
			}
		}
	}

	dasm_init(&dasm_state, DASM_MAXSECTION);
	dasm_setupglobal(&dasm_state, dasm_labels, crex_lb_MAX);
	dasm_setup(&dasm_state, dasm_actions);

	dasm_growpc(&dasm_state, ssa->cfg.blocks_count * 2 + 1);

	crex_jit_align_func(&dasm_state);
	for (b = 0; b < ssa->cfg.blocks_count; b++) {
		if ((ssa->cfg.blocks[b].flags & CREX_BB_REACHABLE) == 0) {
			continue;
		}
//#ifndef CONTEXT_THREADED_JIT
		if (ssa->cfg.blocks[b].flags & CREX_BB_ENTRY) {
			if (ssa->cfg.blocks[b].flags & CREX_BB_TARGET) {
				/* pass */
			} else if (JIT_G(opt_level) < CREX_JIT_LEVEL_INLINE &&
			           ssa->cfg.blocks[b].len == 1 &&
			           (ssa->cfg.blocks[b].flags & CREX_BB_EXIT) &&
			           op_array->opcodes[ssa->cfg.blocks[b].start].opcode != CREX_JMP) {
				/* don't generate code for BB with single opcode */
				continue;
			}
			if (ssa->cfg.blocks[b].flags & CREX_BB_FOLLOW) {
				if (!is_terminated) {
					crex_jit_jmp(&dasm_state, b);
				}
			}
			crex_jit_label(&dasm_state, ssa->cfg.blocks_count + b);
			crex_jit_prologue(&dasm_state);
		} else
//#endif
		if (ssa->cfg.blocks[b].flags & (CREX_BB_START|CREX_BB_RECV_ENTRY)) {
			opline = op_array->opcodes + ssa->cfg.blocks[b].start;
			if (ssa->cfg.flags & CREX_CFG_RECV_ENTRY) {
				if (opline->opcode == CREX_RECV_INIT) {
					if (opline == op_array->opcodes ||
					    (opline-1)->opcode != CREX_RECV_INIT) {
						if (recv_emitted) {
							crex_jit_jmp(&dasm_state, b);
						}
						crex_jit_label(&dasm_state, ssa->cfg.blocks_count + b);
						for (i = 1; (opline+i)->opcode == CREX_RECV_INIT; i++) {
							crex_jit_label(&dasm_state, ssa->cfg.blocks_count + b + i);
						}
						crex_jit_prologue(&dasm_state);
					}
					recv_emitted = 1;
				} else if (opline->opcode == CREX_RECV) {
					if (!(op_array->fn_flags & CREX_ACC_HAS_TYPE_HINTS)) {
						/* skip */
						continue;
					} else if (recv_emitted) {
						crex_jit_jmp(&dasm_state, b);
						crex_jit_label(&dasm_state, ssa->cfg.blocks_count + b);
						crex_jit_prologue(&dasm_state);
					} else {
						crex_arg_info *arg_info;

						if (opline->op1.num <= op_array->num_args) {
							arg_info = &op_array->arg_info[opline->op1.num - 1];
						} else if (op_array->fn_flags & CREX_ACC_VARIADIC) {
							arg_info = &op_array->arg_info[op_array->num_args];
						} else {
							/* skip */
							continue;
						}
						if (!CREX_TYPE_IS_SET(arg_info->type)) {
							/* skip */
							continue;
						}
						crex_jit_label(&dasm_state, ssa->cfg.blocks_count + b);
						crex_jit_prologue(&dasm_state);
						recv_emitted = 1;
					}
				} else {
					if (recv_emitted) {
						crex_jit_jmp(&dasm_state, b);
					} else if (JIT_G(opt_level) < CREX_JIT_LEVEL_INLINE &&
					           ssa->cfg.blocks[b].len == 1 &&
					           (ssa->cfg.blocks[b].flags & CREX_BB_EXIT)) {
						/* don't generate code for BB with single opcode */
						dasm_free(&dasm_state);

						if (JIT_G(opt_flags) & (CREX_JIT_REG_ALLOC_LOCAL|CREX_JIT_REG_ALLOC_GLOBAL)) {
							crex_arena_release(&CG(arena), checkpoint);
						}
						return SUCCESS;
					}
					crex_jit_label(&dasm_state, ssa->cfg.blocks_count + b);
					crex_jit_prologue(&dasm_state);
					recv_emitted = 1;
				}
			} else if (JIT_G(opt_level) < CREX_JIT_LEVEL_INLINE &&
			           ssa->cfg.blocks[b].len == 1 &&
			           (ssa->cfg.blocks[b].flags & CREX_BB_EXIT)) {
				/* don't generate code for BB with single opcode */
				dasm_free(&dasm_state);

				if (JIT_G(opt_flags) & (CREX_JIT_REG_ALLOC_LOCAL|CREX_JIT_REG_ALLOC_GLOBAL)) {
					crex_arena_release(&CG(arena), checkpoint);
				}
				return SUCCESS;
			} else {
				crex_jit_label(&dasm_state, ssa->cfg.blocks_count + b);
				crex_jit_prologue(&dasm_state);
			}
		}

		is_terminated = 0;

		crex_jit_label(&dasm_state, b);
		if (JIT_G(opt_level) < CREX_JIT_LEVEL_INLINE) {
			if ((ssa->cfg.blocks[b].flags & CREX_BB_FOLLOW)
			  && ssa->cfg.blocks[b].start != 0
			  && (op_array->opcodes[ssa->cfg.blocks[b].start - 1].opcode == CREX_NOP
			   || op_array->opcodes[ssa->cfg.blocks[b].start - 1].opcode == CREX_SWITCH_LONG
			   || op_array->opcodes[ssa->cfg.blocks[b].start - 1].opcode == CREX_SWITCH_STRING
			   || op_array->opcodes[ssa->cfg.blocks[b].start - 1].opcode == CREX_MATCH)) {
				crex_jit_reset_last_valid_opline();
				if (!crex_jit_set_ip(&dasm_state, op_array->opcodes + ssa->cfg.blocks[b].start)) {
					goto jit_failure;
				}
			} else {
				crex_jit_set_last_valid_opline(op_array->opcodes + ssa->cfg.blocks[b].start);
			}
		} else if (ssa->cfg.blocks[b].flags & CREX_BB_TARGET) {
			crex_jit_reset_last_valid_opline();
		} else if (ssa->cfg.blocks[b].flags & (CREX_BB_START|CREX_BB_RECV_ENTRY|CREX_BB_ENTRY)) {
			crex_jit_set_last_valid_opline(op_array->opcodes + ssa->cfg.blocks[b].start);
		}
		if (ssa->cfg.blocks[b].flags & CREX_BB_LOOP_HEADER) {
			if (!crex_jit_check_timeout(&dasm_state, op_array->opcodes + ssa->cfg.blocks[b].start, NULL)) {
				goto jit_failure;
			}
		}
		if (!ssa->cfg.blocks[b].len) {
			continue;
		}
		if ((JIT_G(opt_flags) & CREX_JIT_REG_ALLOC_GLOBAL) && ra) {
			crex_ssa_phi *phi = ssa->blocks[b].phis;

			while (phi) {
				crex_lifetime_interval *ival = ra[phi->ssa_var];

				if (ival) {
					if (ival->flags & ZREG_LOAD) {
						CREX_ASSERT(ival->reg != ZREG_NONE);

						if (!crex_jit_load_var(&dasm_state, ssa->var_info[phi->ssa_var].type, ssa->vars[phi->ssa_var].var, ival->reg)) {
							goto jit_failure;
						}
					} else if (ival->flags & ZREG_STORE) {
						CREX_ASSERT(ival->reg != ZREG_NONE);

						if (!crex_jit_store_var(&dasm_state, ssa->var_info[phi->ssa_var].type, ssa->vars[phi->ssa_var].var, ival->reg, 1)) {
							goto jit_failure;
						}
					}
				}
				phi = phi->next;
			}
		}
		end = ssa->cfg.blocks[b].start + ssa->cfg.blocks[b].len - 1;
		for (i = ssa->cfg.blocks[b].start; i <= end; i++) {
			crex_ssa_op *ssa_op = ssa->ops ? &ssa->ops[i] : NULL;
			opline = op_array->opcodes + i;
			switch (opline->opcode) {
				case CREX_INIT_FCALL:
				case CREX_INIT_FCALL_BY_NAME:
				case CREX_INIT_NS_FCALL_BY_NAME:
				case CREX_INIT_METHOD_CALL:
				case CREX_INIT_DYNAMIC_CALL:
				case CREX_INIT_STATIC_METHOD_CALL:
				case CREX_INIT_USER_CALL:
				case CREX_NEW:
					call_level++;
			}

			if (JIT_G(opt_level) >= CREX_JIT_LEVEL_INLINE) {
				switch (opline->opcode) {
					case CREX_PRE_INC:
					case CREX_PRE_DEC:
					case CREX_POST_INC:
					case CREX_POST_DEC:
						if (opline->op1_type != IS_CV) {
							break;
						}
						op1_info = OP1_INFO();
						if (!(op1_info & MAY_BE_LONG)) {
							break;
						}
						if (opline->result_type != IS_UNUSED) {
							res_use_info = -1;

							if (opline->result_type == IS_CV
							 && ssa->vars
							 && ssa_op->result_use >= 0
							 && !ssa->vars[ssa_op->result_use].no_val) {
								crex_jit_addr res_use_addr = RES_USE_REG_ADDR();

								if (C_MODE(res_use_addr) != IS_REG
								 || C_LOAD(res_use_addr)
								 || C_STORE(res_use_addr)) {
									res_use_info = RES_USE_INFO();
								}
							}
							res_info = RES_INFO();
							res_addr = RES_REG_ADDR();
						} else {
							res_use_info = -1;
							res_info = -1;
							res_addr = 0;
						}
						op1_def_info = OP1_DEF_INFO();
						if (!crex_jit_inc_dec(&dasm_state, opline,
								op1_info, OP1_REG_ADDR(),
								op1_def_info, OP1_DEF_REG_ADDR(),
								res_use_info, res_info,
								res_addr,
								(op1_info & MAY_BE_LONG) && (op1_def_info & MAY_BE_DOUBLE) && crex_may_overflow(opline, ssa_op, op_array, ssa),
								crex_may_throw(opline, ssa_op, op_array, ssa))) {
							goto jit_failure;
						}
						goto done;
					case CREX_BW_OR:
					case CREX_BW_AND:
					case CREX_BW_XOR:
					case CREX_SL:
					case CREX_SR:
					case CREX_MOD:
						if (PROFITABILITY_CHECKS && (!ssa->ops || !ssa->var_info)) {
							break;
						}
						op1_info = OP1_INFO();
						op2_info = OP2_INFO();
						if ((op1_info & MAY_BE_UNDEF) || (op2_info & MAY_BE_UNDEF)) {
							break;
						}
						if (!(op1_info & MAY_BE_LONG)
						 || !(op2_info & MAY_BE_LONG)) {
							break;
						}
						res_addr = RES_REG_ADDR();
						if (C_MODE(res_addr) != IS_REG
						 && (i + 1) <= end
						 && crex_jit_next_is_send_result(opline)) {
							i++;
							res_use_info = -1;
							res_addr = CREX_ADDR_MEM_ZVAL(ZREG_RX, (opline+1)->result.var);
							if (!crex_jit_reuse_ip(&dasm_state)) {
								goto jit_failure;
							}
						} else {
							res_use_info = -1;

							if (opline->result_type == IS_CV
							 && ssa->vars
							 && ssa_op->result_use >= 0
							 && !ssa->vars[ssa_op->result_use].no_val) {
								crex_jit_addr res_use_addr = RES_USE_REG_ADDR();

								if (C_MODE(res_use_addr) != IS_REG
								 || C_LOAD(res_use_addr)
								 || C_STORE(res_use_addr)) {
									res_use_info = RES_USE_INFO();
								}
							}
						}
						if (!crex_jit_long_math(&dasm_state, opline,
								op1_info, OP1_RANGE(), OP1_REG_ADDR(),
								op2_info, OP2_RANGE(), OP2_REG_ADDR(),
								res_use_info, RES_INFO(), res_addr,
								crex_may_throw(opline, ssa_op, op_array, ssa))) {
							goto jit_failure;
						}
						goto done;
					case CREX_ADD:
					case CREX_SUB:
					case CREX_MUL:
//					case CREX_DIV: // TODO: check for division by zero ???
						if (PROFITABILITY_CHECKS && (!ssa->ops || !ssa->var_info)) {
							break;
						}
						op1_info = OP1_INFO();
						op2_info = OP2_INFO();
						if ((op1_info & MAY_BE_UNDEF) || (op2_info & MAY_BE_UNDEF)) {
							break;
						}
						if (opline->opcode == CREX_ADD &&
						    (op1_info & (MAY_BE_ANY|MAY_BE_UNDEF)) == MAY_BE_ARRAY &&
						    (op2_info & (MAY_BE_ANY|MAY_BE_UNDEF)) == MAY_BE_ARRAY) {
							/* pass */
						} else if (!(op1_info & (MAY_BE_LONG|MAY_BE_DOUBLE)) ||
						    !(op2_info & (MAY_BE_LONG|MAY_BE_DOUBLE))) {
							break;
						}
						res_addr = RES_REG_ADDR();
						if (C_MODE(res_addr) != IS_REG
						 && (i + 1) <= end
						 && crex_jit_next_is_send_result(opline)) {
							i++;
							res_use_info = -1;
							res_addr = CREX_ADDR_MEM_ZVAL(ZREG_RX, (opline+1)->result.var);
							if (!crex_jit_reuse_ip(&dasm_state)) {
								goto jit_failure;
							}
						} else {
							res_use_info = -1;

							if (opline->result_type == IS_CV
							 && ssa->vars
							 && ssa_op->result_use >= 0
							 && !ssa->vars[ssa_op->result_use].no_val) {
								crex_jit_addr res_use_addr = RES_USE_REG_ADDR();

								if (C_MODE(res_use_addr) != IS_REG
								 || C_LOAD(res_use_addr)
								 || C_STORE(res_use_addr)) {
									res_use_info = RES_USE_INFO();
								}
							}
						}
						res_info = RES_INFO();
						if (opline->opcode == CREX_ADD &&
						    (op1_info & (MAY_BE_ANY|MAY_BE_UNDEF)) == MAY_BE_ARRAY &&
						    (op2_info & (MAY_BE_ANY|MAY_BE_UNDEF)) == MAY_BE_ARRAY) {
							if (!crex_jit_add_arrays(&dasm_state, opline, op1_info, OP1_REG_ADDR(), op2_info, OP2_REG_ADDR(), res_addr)) {
								goto jit_failure;
							}
						} else {
							if (!crex_jit_math(&dasm_state, opline,
									op1_info, OP1_REG_ADDR(),
									op2_info, OP2_REG_ADDR(),
									res_use_info, res_info, res_addr,
									(op1_info & MAY_BE_LONG) && (op2_info & MAY_BE_LONG) && (res_info & MAY_BE_DOUBLE) && crex_may_overflow(opline, ssa_op, op_array, ssa),
									crex_may_throw(opline, ssa_op, op_array, ssa))) {
								goto jit_failure;
							}
						}
						goto done;
					case CREX_CONCAT:
					case CREX_FAST_CONCAT:
						if (PROFITABILITY_CHECKS && (!ssa->ops || !ssa->var_info)) {
							break;
						}
						op1_info = OP1_INFO();
						op2_info = OP2_INFO();
						if ((op1_info & MAY_BE_UNDEF) || (op2_info & MAY_BE_UNDEF)) {
							break;
						}
						if (!(op1_info & MAY_BE_STRING) ||
						    !(op2_info & MAY_BE_STRING)) {
							break;
						}
						res_addr = RES_REG_ADDR();
						if ((i + 1) <= end
						 && crex_jit_next_is_send_result(opline)) {
							i++;
							res_addr = CREX_ADDR_MEM_ZVAL(ZREG_RX, (opline+1)->result.var);
							if (!crex_jit_reuse_ip(&dasm_state)) {
								goto jit_failure;
							}
						}
						if (!crex_jit_concat(&dasm_state, opline,
								op1_info, op2_info, res_addr,
								crex_may_throw(opline, ssa_op, op_array, ssa))) {
							goto jit_failure;
						}
						goto done;
					case CREX_ASSIGN_OP:
						if (opline->op1_type != IS_CV || opline->result_type != IS_UNUSED) {
							break;
						}
						if (PROFITABILITY_CHECKS && (!ssa->ops || !ssa->var_info)) {
							break;
						}
						op1_info = OP1_INFO();
						op2_info = OP2_INFO();
						if (!crex_jit_supported_binary_op(
								opline->extended_value, op1_info, op2_info)) {
							break;
						}
						op1_def_info = OP1_DEF_INFO();
						if (!crex_jit_assign_op(&dasm_state, opline,
								op1_info, op1_def_info, OP1_RANGE(),
								op2_info, OP2_RANGE(),
								(op1_info & MAY_BE_LONG) && (op2_info & MAY_BE_LONG) && (op1_def_info & MAY_BE_DOUBLE) && crex_may_overflow(opline, ssa_op, op_array, ssa),
								crex_may_throw(opline, ssa_op, op_array, ssa))) {
							goto jit_failure;
						}
						goto done;
					case CREX_ASSIGN_DIM_OP:
						if (opline->op1_type != IS_CV || opline->result_type != IS_UNUSED) {
							break;
						}
						if (PROFITABILITY_CHECKS && (!ssa->ops || !ssa->var_info)) {
							break;
						}
						if (!crex_jit_supported_binary_op(
								opline->extended_value, MAY_BE_ANY, OP1_DATA_INFO())) {
							break;
						}
						if (!crex_jit_assign_dim_op(&dasm_state, opline,
								OP1_INFO(), OP1_DEF_INFO(), OP1_REG_ADDR(), OP2_INFO(),
								OP1_DATA_INFO(), OP1_DATA_RANGE(), IS_UNKNOWN,
								crex_may_throw(opline, ssa_op, op_array, ssa))) {
							goto jit_failure;
						}
						goto done;
					case CREX_ASSIGN_DIM:
						if (opline->op1_type != IS_CV) {
							break;
						}
						if (PROFITABILITY_CHECKS && (!ssa->ops || !ssa->var_info)) {
							break;
						}
						if (!crex_jit_assign_dim(&dasm_state, opline,
								OP1_INFO(), OP1_REG_ADDR(), OP2_INFO(), OP1_DATA_INFO(), IS_UNKNOWN,
								crex_may_throw(opline, ssa_op, op_array, ssa))) {
							goto jit_failure;
						}
						goto done;
					case CREX_PRE_INC_OBJ:
					case CREX_PRE_DEC_OBJ:
					case CREX_POST_INC_OBJ:
					case CREX_POST_DEC_OBJ:
						if (opline->op2_type != IS_CONST
						 || C_TYPE_P(RT_CONSTANT(opline, opline->op2)) != IS_STRING
						 || C_STRVAL_P(RT_CONSTANT(opline, opline->op2))[0] == '\0') {
							break;
						}
						if (PROFITABILITY_CHECKS && (!ssa->ops || !ssa->var_info)) {
							break;
						}
						ce = NULL;
						ce_is_instanceof = 0;
						on_this = 0;
						if (opline->op1_type == IS_UNUSED) {
							op1_info = MAY_BE_OBJECT|MAY_BE_RC1|MAY_BE_RCN;
							ce = op_array->scope;
							ce_is_instanceof = (ce->ce_flags & CREX_ACC_FINAL) != 0;
							op1_addr = 0;
							on_this = 1;
						} else {
							op1_info = OP1_INFO();
							if (!(op1_info & MAY_BE_OBJECT)) {
								break;
							}
							op1_addr = OP1_REG_ADDR();
							if (ssa->var_info && ssa->ops) {
								crex_ssa_op *ssa_op = &ssa->ops[opline - op_array->opcodes];
								if (ssa_op->op1_use >= 0) {
									crex_ssa_var_info *op1_ssa = ssa->var_info + ssa_op->op1_use;
									if (op1_ssa->ce && !op1_ssa->ce->create_object) {
										ce = op1_ssa->ce;
										ce_is_instanceof = op1_ssa->is_instanceof;
									}
								}
							}
						}
						if (!crex_jit_incdec_obj(&dasm_state, opline, op_array, ssa, ssa_op,
								op1_info, op1_addr,
								0, ce, ce_is_instanceof, on_this, 0, NULL, IS_UNKNOWN)) {
							goto jit_failure;
						}
						goto done;
					case CREX_ASSIGN_OBJ_OP:
						if (opline->result_type != IS_UNUSED) {
							break;
						}
						if (opline->op2_type != IS_CONST
						 || C_TYPE_P(RT_CONSTANT(opline, opline->op2)) != IS_STRING
						 || C_STRVAL_P(RT_CONSTANT(opline, opline->op2))[0] == '\0') {
							break;
						}
						if (PROFITABILITY_CHECKS && (!ssa->ops || !ssa->var_info)) {
							break;
						}
						if (!crex_jit_supported_binary_op(
								opline->extended_value, MAY_BE_ANY, OP1_DATA_INFO())) {
							break;
						}
						ce = NULL;
						ce_is_instanceof = 0;
						on_this = 0;
						if (opline->op1_type == IS_UNUSED) {
							op1_info = MAY_BE_OBJECT|MAY_BE_RC1|MAY_BE_RCN;
							ce = op_array->scope;
							ce_is_instanceof = (ce->ce_flags & CREX_ACC_FINAL) != 0;
							op1_addr = 0;
							on_this = 1;
						} else {
							op1_info = OP1_INFO();
							if (!(op1_info & MAY_BE_OBJECT)) {
								break;
							}
							op1_addr = OP1_REG_ADDR();
							if (ssa->var_info && ssa->ops) {
								crex_ssa_op *ssa_op = &ssa->ops[opline - op_array->opcodes];
								if (ssa_op->op1_use >= 0) {
									crex_ssa_var_info *op1_ssa = ssa->var_info + ssa_op->op1_use;
									if (op1_ssa->ce && !op1_ssa->ce->create_object) {
										ce = op1_ssa->ce;
										ce_is_instanceof = op1_ssa->is_instanceof;
									}
								}
							}
						}
						if (!crex_jit_assign_obj_op(&dasm_state, opline, op_array, ssa, ssa_op,
								op1_info, op1_addr, OP1_DATA_INFO(), OP1_DATA_RANGE(),
								0, ce, ce_is_instanceof, on_this, 0, NULL, IS_UNKNOWN)) {
							goto jit_failure;
						}
						goto done;
					case CREX_ASSIGN_OBJ:
						if (opline->op2_type != IS_CONST
						 || C_TYPE_P(RT_CONSTANT(opline, opline->op2)) != IS_STRING
						 || C_STRVAL_P(RT_CONSTANT(opline, opline->op2))[0] == '\0') {
							break;
						}
						if (PROFITABILITY_CHECKS && (!ssa->ops || !ssa->var_info)) {
							break;
						}
						ce = NULL;
						ce_is_instanceof = 0;
						on_this = 0;
						if (opline->op1_type == IS_UNUSED) {
							op1_info = MAY_BE_OBJECT|MAY_BE_RC1|MAY_BE_RCN;
							ce = op_array->scope;
							ce_is_instanceof = (ce->ce_flags & CREX_ACC_FINAL) != 0;
							op1_addr = 0;
							on_this = 1;
						} else {
							op1_info = OP1_INFO();
							if (!(op1_info & MAY_BE_OBJECT)) {
								break;
							}
							op1_addr = OP1_REG_ADDR();
							if (ssa->var_info && ssa->ops) {
								crex_ssa_op *ssa_op = &ssa->ops[opline - op_array->opcodes];
								if (ssa_op->op1_use >= 0) {
									crex_ssa_var_info *op1_ssa = ssa->var_info + ssa_op->op1_use;
									if (op1_ssa->ce && !op1_ssa->ce->create_object) {
										ce = op1_ssa->ce;
										ce_is_instanceof = op1_ssa->is_instanceof;
									}
								}
							}
						}
						if (!crex_jit_assign_obj(&dasm_state, opline, op_array, ssa, ssa_op,
								op1_info, op1_addr, OP1_DATA_INFO(),
								0, ce, ce_is_instanceof, on_this, 0, NULL, IS_UNKNOWN,
								crex_may_throw(opline, ssa_op, op_array, ssa))) {
							goto jit_failure;
						}
						goto done;
					case CREX_ASSIGN:
						if (opline->op1_type != IS_CV) {
							break;
						}
						if (PROFITABILITY_CHECKS && (!ssa->ops || !ssa->var_info)) {
							break;
						}
						op2_addr = OP2_REG_ADDR();
						if (ra
						 && ssa->ops[opline - op_array->opcodes].op2_def >= 0
						 && !ssa->vars[ssa->ops[opline - op_array->opcodes].op2_def].no_val) {
							op2_def_addr = OP2_DEF_REG_ADDR();
						} else {
							op2_def_addr = op2_addr;
						}
						op1_info = OP1_INFO();
						if (ra && ssa->vars[ssa_op->op1_use].no_val) {
							op1_info |= MAY_BE_UNDEF; // requres type assignment
						}
						if (opline->result_type == IS_UNUSED) {
							res_addr = 0;
							res_info = -1;
						} else {
							res_addr = RES_REG_ADDR();
							res_info = RES_INFO();
							if (C_MODE(res_addr) != IS_REG
							 && (i + 1) <= end
							 && crex_jit_next_is_send_result(opline)
							 && (!(op1_info & MAY_HAVE_DTOR) || !(op1_info & MAY_BE_RC1))) {
								i++;
								res_addr = CREX_ADDR_MEM_ZVAL(ZREG_RX, (opline+1)->result.var);
								if (!crex_jit_reuse_ip(&dasm_state)) {
									goto jit_failure;
								}
							}
						}
						if (!crex_jit_assign(&dasm_state, opline,
								op1_info, OP1_REG_ADDR(),
								OP1_DEF_INFO(), OP1_DEF_REG_ADDR(),
								OP2_INFO(), op2_addr, op2_def_addr,
								res_info, res_addr,
								crex_may_throw(opline, ssa_op, op_array, ssa))) {
							goto jit_failure;
						}
						goto done;
					case CREX_QM_ASSIGN:
						op1_addr = OP1_REG_ADDR();
						if (ra
						 && ssa->ops[opline - op_array->opcodes].op1_def >= 0
						 && !ssa->vars[ssa->ops[opline - op_array->opcodes].op1_def].no_val) {
							op1_def_addr = OP1_DEF_REG_ADDR();
						} else {
							op1_def_addr = op1_addr;
						}
						if (!crex_jit_qm_assign(&dasm_state, opline,
								OP1_INFO(), op1_addr, op1_def_addr,
								-1, RES_INFO(), RES_REG_ADDR())) {
							goto jit_failure;
						}
						goto done;
					case CREX_INIT_FCALL:
					case CREX_INIT_FCALL_BY_NAME:
					case CREX_INIT_NS_FCALL_BY_NAME:
						if (!crex_jit_init_fcall(&dasm_state, opline, b, op_array, ssa, ssa_op, call_level, NULL, 0)) {
							goto jit_failure;
						}
						goto done;
					case CREX_SEND_VAL:
					case CREX_SEND_VAL_EX:
						if (opline->op2_type == IS_CONST) {
							/* Named parameters not supported in JIT (yet) */
							break;
						}
						if (opline->opcode == CREX_SEND_VAL_EX
						 && opline->op2.num > MAX_ARG_FLAG_NUM) {
							break;
						}
						if (!crex_jit_send_val(&dasm_state, opline,
								OP1_INFO(), OP1_REG_ADDR())) {
							goto jit_failure;
						}
						goto done;
					case CREX_SEND_REF:
						if (opline->op2_type == IS_CONST) {
							/* Named parameters not supported in JIT (yet) */
							break;
						}
						if (!crex_jit_send_ref(&dasm_state, opline, op_array,
								OP1_INFO(), 0)) {
							goto jit_failure;
						}
						goto done;
					case CREX_SEND_VAR:
					case CREX_SEND_VAR_EX:
					case CREX_SEND_VAR_NO_REF:
					case CREX_SEND_VAR_NO_REF_EX:
					case CREX_SEND_FUNC_ARG:
						if (opline->op2_type == IS_CONST) {
							/* Named parameters not supported in JIT (yet) */
							break;
						}
						if ((opline->opcode == CREX_SEND_VAR_EX
						  || opline->opcode == CREX_SEND_VAR_NO_REF_EX)
						 && opline->op2.num > MAX_ARG_FLAG_NUM) {
							break;
						}
						op1_addr = OP1_REG_ADDR();
						if (ra
						 && ssa->ops[opline - op_array->opcodes].op1_def >= 0
						 && !ssa->vars[ssa->ops[opline - op_array->opcodes].op1_def].no_val) {
							op1_def_addr = OP1_DEF_REG_ADDR();
						} else {
							op1_def_addr = op1_addr;
						}
						if (!crex_jit_send_var(&dasm_state, opline, op_array,
								OP1_INFO(), op1_addr, op1_def_addr)) {
							goto jit_failure;
						}
						goto done;
					case CREX_CHECK_FUNC_ARG:
						if (opline->op2_type == IS_CONST) {
							/* Named parameters not supported in JIT (yet) */
							break;
						}
						if (opline->op2.num > MAX_ARG_FLAG_NUM) {
							break;
						}
						if (!crex_jit_check_func_arg(&dasm_state, opline)) {
							goto jit_failure;
						}
						goto done;
					case CREX_CHECK_UNDEF_ARGS:
						if (!crex_jit_check_undef_args(&dasm_state, opline)) {
							goto jit_failure;
						}
						goto done;
					case CREX_DO_UCALL:
						is_terminated = 1;
						CREX_FALLTHROUGH;
					case CREX_DO_ICALL:
					case CREX_DO_FCALL_BY_NAME:
					case CREX_DO_FCALL:
						if (!crex_jit_do_fcall(&dasm_state, opline, op_array, ssa, call_level, b + 1, NULL)) {
							goto jit_failure;
						}
						goto done;
					case CREX_IS_EQUAL:
					case CREX_IS_NOT_EQUAL:
					case CREX_IS_SMALLER:
					case CREX_IS_SMALLER_OR_EQUAL:
					case CREX_CASE: {
						res_addr = RES_REG_ADDR();
						if ((opline->result_type & IS_TMP_VAR)
						 && (i + 1) <= end
						 && ((opline+1)->opcode == CREX_JMPZ
						  || (opline+1)->opcode == CREX_JMPNZ
						  || (opline+1)->opcode == CREX_JMPC_EX
						  || (opline+1)->opcode == CREX_JMPNC_EX)
						 && (opline+1)->op1_type == IS_TMP_VAR
						 && (opline+1)->op1.var == opline->result.var) {
							i++;
							smart_branch_opcode = (opline+1)->opcode;
							target_label = ssa->cfg.blocks[b].successors[0];
							target_label2 = ssa->cfg.blocks[b].successors[1];
							/* For EX variant write into the result of EX opcode. */
							if ((opline+1)->opcode == CREX_JMPC_EX
									|| (opline+1)->opcode == CREX_JMPNC_EX) {
								res_addr = OP_REG_ADDR(opline + 1, result_type, result, result_def);
							}
						} else {
							smart_branch_opcode = 0;
							target_label = target_label2 = (uint32_t)-1;
						}
						if (!crex_jit_cmp(&dasm_state, opline,
								OP1_INFO(), OP1_RANGE(), OP1_REG_ADDR(),
								OP2_INFO(), OP2_RANGE(), OP2_REG_ADDR(),
								res_addr,
								crex_may_throw(opline, ssa_op, op_array, ssa),
								smart_branch_opcode, target_label, target_label2,
								NULL, 0)) {
							goto jit_failure;
						}
						goto done;
					}
					case CREX_IS_IDENTICAL:
					case CREX_IS_NOT_IDENTICAL:
					case CREX_CASE_STRICT:
						if ((opline->result_type & IS_TMP_VAR)
						 && (i + 1) <= end
						 && ((opline+1)->opcode == CREX_JMPZ
						  || (opline+1)->opcode == CREX_JMPNZ)
						 && (opline+1)->op1_type == IS_TMP_VAR
						 && (opline+1)->op1.var == opline->result.var) {
							i++;
							smart_branch_opcode = (opline+1)->opcode;
							target_label = ssa->cfg.blocks[b].successors[0];
							target_label2 = ssa->cfg.blocks[b].successors[1];
						} else {
							smart_branch_opcode = 0;
							target_label = target_label2 = (uint32_t)-1;
						}
						if (!crex_jit_identical(&dasm_state, opline,
								OP1_INFO(), OP1_RANGE(), OP1_REG_ADDR(),
								OP2_INFO(), OP2_RANGE(), OP2_REG_ADDR(),
								RES_REG_ADDR(),
								crex_may_throw(opline, ssa_op, op_array, ssa),
								smart_branch_opcode, target_label, target_label2,
								NULL, 0)) {
							goto jit_failure;
						}
						goto done;
					case CREX_DEFINED:
						if ((opline->result_type & IS_TMP_VAR)
						 && (i + 1) <= end
						 && ((opline+1)->opcode == CREX_JMPZ
						  || (opline+1)->opcode == CREX_JMPNZ)
						 && (opline+1)->op1_type == IS_TMP_VAR
						 && (opline+1)->op1.var == opline->result.var) {
							i++;
							smart_branch_opcode = (opline+1)->opcode;
							target_label = ssa->cfg.blocks[b].successors[0];
							target_label2 = ssa->cfg.blocks[b].successors[1];
						} else {
							smart_branch_opcode = 0;
							target_label = target_label2 = (uint32_t)-1;
						}
						if (!crex_jit_defined(&dasm_state, opline, smart_branch_opcode, target_label, target_label2, NULL)) {
							goto jit_failure;
						}
						goto done;
					case CREX_TYPE_CHECK:
						if (opline->extended_value == MAY_BE_RESOURCE) {
							// TODO: support for is_resource() ???
							break;
						}
						if ((opline->result_type & IS_TMP_VAR)
						 && (i + 1) <= end
						 && ((opline+1)->opcode == CREX_JMPZ
						  || (opline+1)->opcode == CREX_JMPNZ)
						 && (opline+1)->op1_type == IS_TMP_VAR
						 && (opline+1)->op1.var == opline->result.var) {
							i++;
							smart_branch_opcode = (opline+1)->opcode;
							target_label = ssa->cfg.blocks[b].successors[0];
							target_label2 = ssa->cfg.blocks[b].successors[1];
						} else {
							smart_branch_opcode = 0;
							target_label = target_label2 = (uint32_t)-1;
						}
						if (!crex_jit_type_check(&dasm_state, opline, OP1_INFO(), smart_branch_opcode, target_label, target_label2, NULL)) {
							goto jit_failure;
						}
						goto done;
					case CREX_RETURN:
						op1_info = OP1_INFO();
						if ((PROFITABILITY_CHECKS && (!ssa->ops || !ssa->var_info))
						 || op_array->type == CREX_EVAL_CODE
						 // TODO: support for top-level code
						 || !op_array->function_name
						 // TODO: support for IS_UNDEF ???
						 || (op1_info & MAY_BE_UNDEF)) {
							if (!crex_jit_tail_handler(&dasm_state, opline)) {
								goto jit_failure;
							}
						} else {
							int j;
							bool left_frame = 0;

							if (!crex_jit_return(&dasm_state, opline, op_array,
									op1_info, OP1_REG_ADDR())) {
								goto jit_failure;
							}
							if (jit_return_label >= 0) {
								if (!crex_jit_jmp(&dasm_state, jit_return_label)) {
									goto jit_failure;
								}
								goto done;
							}
							jit_return_label = ssa->cfg.blocks_count * 2;
							if (!crex_jit_label(&dasm_state, jit_return_label)) {
								goto jit_failure;
							}
							if (op_array->last_var > 100) {
								/* To many CVs to unroll */
								if (!crex_jit_free_cvs(&dasm_state)) {
									goto jit_failure;
								}
								left_frame = 1;
							}
							if (!left_frame) {
								for (j = 0 ; j < op_array->last_var; j++) {
									uint32_t info = crex_ssa_cv_info(op_array, ssa, j);

									if (info & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE|MAY_BE_REF)) {
										if (!left_frame) {
											left_frame = 1;
										    if (!crex_jit_leave_frame(&dasm_state)) {
												goto jit_failure;
										    }
										}
										if (!crex_jit_free_cv(&dasm_state, info, j)) {
											goto jit_failure;
										}
									}
								}
							}
							if (!crex_jit_leave_func(&dasm_state, op_array, opline, op1_info, left_frame,
									NULL, NULL, (ssa->cfg.flags & CREX_FUNC_INDIRECT_VAR_ACCESS) != 0, 1)) {
								goto jit_failure;
							}
						}
						goto done;
					case CREX_BOOL:
					case CREX_BOOL_NOT:
						if (!crex_jit_bool_jmpznz(&dasm_state, opline,
								OP1_INFO(), OP1_REG_ADDR(), RES_REG_ADDR(),
								-1, -1,
								crex_may_throw(opline, ssa_op, op_array, ssa),
								opline->opcode, NULL)) {
							goto jit_failure;
						}
						goto done;
					case CREX_JMPZ:
					case CREX_JMPNZ:
						if (opline > op_array->opcodes + ssa->cfg.blocks[b].start &&
						    ((opline-1)->result_type & (IS_SMART_BRANCH_JMPZ|IS_SMART_BRANCH_JMPNZ)) != 0) {
							/* smart branch */
							if (!crex_jit_cond_jmp(&dasm_state, opline + 1, ssa->cfg.blocks[b].successors[0])) {
								goto jit_failure;
							}
							goto done;
						}
						CREX_FALLTHROUGH;
					case CREX_JMPC_EX:
					case CREX_JMPNC_EX:
						if (opline->result_type == IS_UNDEF) {
							res_addr = 0;
						} else {
							res_addr = RES_REG_ADDR();
						}
						if (!crex_jit_bool_jmpznz(&dasm_state, opline,
								OP1_INFO(), OP1_REG_ADDR(), res_addr,
								ssa->cfg.blocks[b].successors[0], ssa->cfg.blocks[b].successors[1],
								crex_may_throw(opline, ssa_op, op_array, ssa),
								opline->opcode, NULL)) {
							goto jit_failure;
						}
						goto done;
					case CREX_ISSET_ISEMPTY_CV:
						if ((opline->extended_value & CREX_ISEMPTY)) {
							// TODO: support for empty() ???
							break;
						}
						if ((opline->result_type & IS_TMP_VAR)
						 && (i + 1) <= end
						 && ((opline+1)->opcode == CREX_JMPZ
						  || (opline+1)->opcode == CREX_JMPNZ)
						 && (opline+1)->op1_type == IS_TMP_VAR
						 && (opline+1)->op1.var == opline->result.var) {
							i++;
							smart_branch_opcode = (opline+1)->opcode;
							target_label = ssa->cfg.blocks[b].successors[0];
							target_label2 = ssa->cfg.blocks[b].successors[1];
						} else {
							smart_branch_opcode = 0;
							target_label = target_label2 = (uint32_t)-1;
						}
						if (!crex_jit_isset_isempty_cv(&dasm_state, opline,
								OP1_INFO(), OP1_REG_ADDR(),
								smart_branch_opcode, target_label, target_label2,
								NULL)) {
							goto jit_failure;
						}
						goto done;
					case CREX_IN_ARRAY:
						if (opline->op1_type == IS_VAR || opline->op1_type == IS_TMP_VAR) {
							break;
						}
						op1_info = OP1_INFO();
						if ((op1_info & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_REF)) != MAY_BE_STRING) {
							break;
						}
						if ((opline->result_type & IS_TMP_VAR)
						 && (i + 1) <= end
						 && ((opline+1)->opcode == CREX_JMPZ
						  || (opline+1)->opcode == CREX_JMPNZ)
						 && (opline+1)->op1_type == IS_TMP_VAR
						 && (opline+1)->op1.var == opline->result.var) {
							i++;
							smart_branch_opcode = (opline+1)->opcode;
							target_label = ssa->cfg.blocks[b].successors[0];
							target_label2 = ssa->cfg.blocks[b].successors[1];
						} else {
							smart_branch_opcode = 0;
							target_label = target_label2 = (uint32_t)-1;
						}
						if (!crex_jit_in_array(&dasm_state, opline,
								op1_info, OP1_REG_ADDR(),
								smart_branch_opcode, target_label, target_label2,
								NULL)) {
							goto jit_failure;
						}
						goto done;
					case CREX_FETCH_DIM_R:
					case CREX_FETCH_DIM_IS:
					case CREX_FETCH_LIST_R:
						if (PROFITABILITY_CHECKS && (!ssa->ops || !ssa->var_info)) {
							break;
						}
						if (!crex_jit_fetch_dim_read(&dasm_state, opline, ssa, ssa_op,
								OP1_INFO(), OP1_REG_ADDR(), 0,
								OP2_INFO(), RES_INFO(), RES_REG_ADDR(), IS_UNKNOWN)) {
							goto jit_failure;
						}
						goto done;
					case CREX_FETCH_DIM_W:
					case CREX_FETCH_DIM_RW:
//					case CREX_FETCH_DIM_UNSET:
					case CREX_FETCH_LIST_W:
						if (PROFITABILITY_CHECKS && (!ssa->ops || !ssa->var_info)) {
							break;
						}
						if (opline->op1_type != IS_CV) {
							break;
						}
						if (!crex_jit_fetch_dim(&dasm_state, opline,
								OP1_INFO(), OP1_REG_ADDR(), OP2_INFO(), RES_REG_ADDR(), IS_UNKNOWN)) {
							goto jit_failure;
						}
						goto done;
					case CREX_ISSET_ISEMPTY_DIM_OBJ:
						if ((opline->extended_value & CREX_ISEMPTY)) {
							// TODO: support for empty() ???
							break;
						}
						if (PROFITABILITY_CHECKS && (!ssa->ops || !ssa->var_info)) {
							break;
						}
						if ((opline->result_type & IS_TMP_VAR)
						 && (i + 1) <= end
						 && ((opline+1)->opcode == CREX_JMPZ
						  || (opline+1)->opcode == CREX_JMPNZ)
						 && (opline+1)->op1_type == IS_TMP_VAR
						 && (opline+1)->op1.var == opline->result.var) {
							i++;
							smart_branch_opcode = (opline+1)->opcode;
							target_label = ssa->cfg.blocks[b].successors[0];
							target_label2 = ssa->cfg.blocks[b].successors[1];
						} else {
							smart_branch_opcode = 0;
							target_label = target_label2 = (uint32_t)-1;
						}
						if (!crex_jit_isset_isempty_dim(&dasm_state, opline,
								OP1_INFO(), OP1_REG_ADDR(), 0,
								OP2_INFO(), IS_UNKNOWN,
								crex_may_throw(opline, ssa_op, op_array, ssa),
								smart_branch_opcode, target_label, target_label2,
								NULL)) {
							goto jit_failure;
						}
						goto done;
					case CREX_FETCH_OBJ_R:
					case CREX_FETCH_OBJ_IS:
					case CREX_FETCH_OBJ_W:
						if (opline->op2_type != IS_CONST
						 || C_TYPE_P(RT_CONSTANT(opline, opline->op2)) != IS_STRING
						 || C_STRVAL_P(RT_CONSTANT(opline, opline->op2))[0] == '\0') {
							break;
						}
						ce = NULL;
						ce_is_instanceof = 0;
						on_this = 0;
						if (opline->op1_type == IS_UNUSED) {
							op1_info = MAY_BE_OBJECT|MAY_BE_RC1|MAY_BE_RCN;
							op1_addr = 0;
							ce = op_array->scope;
							ce_is_instanceof = (ce->ce_flags & CREX_ACC_FINAL) != 0;
							on_this = 1;
						} else {
							op1_info = OP1_INFO();
							if (!(op1_info & MAY_BE_OBJECT)) {
								break;
							}
							op1_addr = OP1_REG_ADDR();
							if (ssa->var_info && ssa->ops) {
								crex_ssa_op *ssa_op = &ssa->ops[opline - op_array->opcodes];
								if (ssa_op->op1_use >= 0) {
									crex_ssa_var_info *op1_ssa = ssa->var_info + ssa_op->op1_use;
									if (op1_ssa->ce && !op1_ssa->ce->create_object) {
										ce = op1_ssa->ce;
										ce_is_instanceof = op1_ssa->is_instanceof;
									}
								}
							}
						}
						if (!crex_jit_fetch_obj(&dasm_state, opline, op_array, ssa, ssa_op,
								op1_info, op1_addr, 0, ce, ce_is_instanceof, on_this, 0, 0, NULL,
								IS_UNKNOWN,
								crex_may_throw(opline, ssa_op, op_array, ssa))) {
							goto jit_failure;
						}
						goto done;
					case CREX_BIND_GLOBAL:
						if (!ssa->ops || !ssa->var_info) {
							op1_info = MAY_BE_ANY|MAY_BE_REF;
						} else {
							op1_info = OP1_INFO();
						}
						if (!crex_jit_bind_global(&dasm_state, opline, op1_info)) {
							goto jit_failure;
						}
						goto done;
					case CREX_RECV:
						if (!crex_jit_recv(&dasm_state, opline, op_array)) {
							goto jit_failure;
						}
						goto done;
					case CREX_RECV_INIT:
						if (!crex_jit_recv_init(&dasm_state, opline, op_array,
								(opline + 1)->opcode != CREX_RECV_INIT,
								crex_may_throw(opline, ssa_op, op_array, ssa))) {
							goto jit_failure;
						}
						goto done;
					case CREX_FREE:
					case CREX_FE_FREE:
						if (!crex_jit_free(&dasm_state, opline, OP1_INFO(),
								crex_may_throw(opline, ssa_op, op_array, ssa))) {
							goto jit_failure;
						}
						goto done;
					case CREX_ECHO:
						op1_info = OP1_INFO();
						if ((op1_info & (MAY_BE_UNDEF|MAY_BE_ANY|MAY_BE_REF)) != MAY_BE_STRING) {
							break;
						}
						if (!crex_jit_echo(&dasm_state, opline, op1_info)) {
							goto jit_failure;
						}
						goto done;
					case CREX_STRLEN:
						op1_info = OP1_INFO();
						if ((op1_info & (MAY_BE_UNDEF|MAY_BE_ANY|MAY_BE_REF)) != MAY_BE_STRING) {
							break;
						}
						if (!crex_jit_strlen(&dasm_state, opline, op1_info, OP1_REG_ADDR(), RES_REG_ADDR())) {
							goto jit_failure;
						}
						goto done;
					case CREX_COUNT:
						op1_info = OP1_INFO();
						if ((op1_info & (MAY_BE_UNDEF|MAY_BE_ANY|MAY_BE_REF)) != MAY_BE_ARRAY) {
							break;
						}
						if (!crex_jit_count(&dasm_state, opline, op1_info, OP1_REG_ADDR(), RES_REG_ADDR(), crex_may_throw(opline, ssa_op, op_array, ssa))) {
							goto jit_failure;
						}
						goto done;
					case CREX_FETCH_THIS:
						if (!crex_jit_fetch_this(&dasm_state, opline, op_array, 0)) {
							goto jit_failure;
						}
						goto done;
					case CREX_SWITCH_LONG:
					case CREX_SWITCH_STRING:
					case CREX_MATCH:
						if (!crex_jit_switch(&dasm_state, opline, op_array, ssa, NULL, NULL)) {
							goto jit_failure;
						}
						goto done;
					case CREX_VERIFY_RETURN_TYPE:
						if (opline->op1_type == IS_UNUSED) {
							/* Always throws */
							break;
						}
						if (opline->op1_type == IS_CONST) {
							/* TODO Different instruction format, has return value */
							break;
						}
						if (op_array->fn_flags & CREX_ACC_RETURN_REFERENCE) {
							/* Not worth bothering with */
							break;
						}
						if (OP1_INFO() & MAY_BE_REF) {
							/* TODO May need reference unwrapping. */
							break;
						}
						if (!crex_jit_verify_return_type(&dasm_state, opline, op_array, OP1_INFO())) {
							goto jit_failure;
						}
						goto done;
					case CREX_FE_RESET_R:
						op1_info = OP1_INFO();
						if ((op1_info & (MAY_BE_ANY|MAY_BE_REF|MAY_BE_UNDEF)) != MAY_BE_ARRAY) {
							break;
						}
						if (!crex_jit_fe_reset(&dasm_state, opline, op1_info)) {
							goto jit_failure;
						}
						goto done;
					case CREX_FE_FETCH_R:
						op1_info = OP1_INFO();
						if ((op1_info & MAY_BE_ANY) != MAY_BE_ARRAY) {
							break;
						}
						if (!crex_jit_fe_fetch(&dasm_state, opline, op1_info, OP2_INFO(),
								ssa->cfg.blocks[b].successors[0], opline->opcode, NULL)) {
							goto jit_failure;
						}
						goto done;
					case CREX_FETCH_CONSTANT:
						if (!crex_jit_fetch_constant(&dasm_state, opline, op_array, ssa, ssa_op, RES_REG_ADDR())) {
							goto jit_failure;
						}
						goto done;
					case CREX_INIT_METHOD_CALL:
						if (opline->op2_type != IS_CONST
						 || C_TYPE_P(RT_CONSTANT(opline, opline->op2)) != IS_STRING) {
							break;
						}
						ce = NULL;
						ce_is_instanceof = 0;
						on_this = 0;
						if (opline->op1_type == IS_UNUSED) {
							op1_info = MAY_BE_OBJECT|MAY_BE_RC1|MAY_BE_RCN;
							op1_addr = 0;
							ce = op_array->scope;
							ce_is_instanceof = (ce->ce_flags & CREX_ACC_FINAL) != 0;
							on_this = 1;
						} else {
							op1_info = OP1_INFO();
							if (!(op1_info & MAY_BE_OBJECT)) {
								break;
							}
							op1_addr = OP1_REG_ADDR();
							if (ssa->var_info && ssa->ops) {
								crex_ssa_op *ssa_op = &ssa->ops[opline - op_array->opcodes];
								if (ssa_op->op1_use >= 0) {
									crex_ssa_var_info *op1_ssa = ssa->var_info + ssa_op->op1_use;
									if (op1_ssa->ce && !op1_ssa->ce->create_object) {
										ce = op1_ssa->ce;
										ce_is_instanceof = op1_ssa->is_instanceof;
									}
								}
							}
						}
						if (!crex_jit_init_method_call(&dasm_state, opline, b, op_array, ssa, ssa_op, call_level,
								op1_info, op1_addr, ce, ce_is_instanceof, on_this, 0, NULL,
								NULL, 0, 0)) {
							goto jit_failure;
						}
						goto done;
					case CREX_ROPE_INIT:
					case CREX_ROPE_ADD:
					case CREX_ROPE_END:
						op2_info = OP2_INFO();
						if ((op2_info & (MAY_BE_UNDEF|MAY_BE_ANY|MAY_BE_REF)) != MAY_BE_STRING) {
							break;
						}
						if (!crex_jit_rope(&dasm_state, opline, op2_info)) {
							goto jit_failure;
						}
						goto done;
					default:
						break;
				}
			}

			switch (opline->opcode) {
				case CREX_RECV_INIT:
				case CREX_BIND_GLOBAL:
					if (opline == op_array->opcodes ||
					    opline->opcode != op_array->opcodes[i-1].opcode) {
						/* repeatable opcodes */
						if (!crex_jit_handler(&dasm_state, opline,
								crex_may_throw(opline, ssa_op, op_array, ssa))) {
							goto jit_failure;
						}
					}
					crex_jit_set_last_valid_opline(opline+1);
					break;
				case CREX_NOP:
				case CREX_OP_DATA:
				case CREX_SWITCH_LONG:
				case CREX_SWITCH_STRING:
				case CREX_MATCH:
					break;
				case CREX_JMP:
					if (JIT_G(opt_level) < CREX_JIT_LEVEL_INLINE) {
						const crex_op *target = OP_JMP_ADDR(opline, opline->op1);

						if (!crex_jit_set_ip(&dasm_state, target)) {
							goto jit_failure;
						}
					}
					if (!crex_jit_jmp(&dasm_state, ssa->cfg.blocks[b].successors[0])) {
						goto jit_failure;
					}
					is_terminated = 1;
					break;
				case CREX_CATCH:
				case CREX_FAST_CALL:
				case CREX_FAST_RET:
				case CREX_GENERATOR_CREATE:
				case CREX_GENERATOR_RETURN:
				case CREX_RETURN_BY_REF:
				case CREX_RETURN:
				case CREX_EXIT:
				case CREX_MATCH_ERROR:
				/* switch through trampoline */
				case CREX_YIELD:
				case CREX_YIELD_FROM:
					if (!crex_jit_tail_handler(&dasm_state, opline)) {
						goto jit_failure;
					}
					is_terminated = 1;
					break;
				/* stackless execution */
				case CREX_INCLUDE_OR_EVAL:
				case CREX_DO_FCALL:
				case CREX_DO_UCALL:
				case CREX_DO_FCALL_BY_NAME:
					if (!crex_jit_call(&dasm_state, opline, b + 1)) {
						goto jit_failure;
					}
					is_terminated = 1;
					break;
				case CREX_JMPZ:
				case CREX_JMPNZ:
					if (opline > op_array->opcodes + ssa->cfg.blocks[b].start &&
					    ((opline-1)->result_type & (IS_SMART_BRANCH_JMPZ|IS_SMART_BRANCH_JMPNZ)) != 0) {
						/* smart branch */
						if (!crex_jit_cond_jmp(&dasm_state, opline + 1, ssa->cfg.blocks[b].successors[0])) {
							goto jit_failure;
						}
						goto done;
					}
					CREX_FALLTHROUGH;
				case CREX_JMPC_EX:
				case CREX_JMPNC_EX:
				case CREX_JMP_SET:
				case CREX_COALESCE:
				case CREX_JMP_NULL:
				case CREX_FE_RESET_R:
				case CREX_FE_RESET_RW:
				case CREX_ASSERT_CHECK:
				case CREX_FE_FETCH_R:
				case CREX_FE_FETCH_RW:
				case CREX_BIND_INIT_STATIC_OR_JMP:
					if (!crex_jit_handler(&dasm_state, opline,
							crex_may_throw(opline, ssa_op, op_array, ssa)) ||
					    !crex_jit_cond_jmp(&dasm_state, opline + 1, ssa->cfg.blocks[b].successors[0])) {
						goto jit_failure;
					}
					break;
				case CREX_NEW:
					if (!crex_jit_handler(&dasm_state, opline, 1)) {
						return 0;
					}
					if (opline->extended_value == 0 && (opline+1)->opcode == CREX_DO_FCALL) {
						crex_class_entry *ce = NULL;

						if (JIT_G(opt_level) >= CREX_JIT_LEVEL_OPT_FUNC) {
							if (ssa->ops && ssa->var_info) {
								crex_ssa_var_info *res_ssa = &ssa->var_info[ssa->ops[opline - op_array->opcodes].result_def];
								if (res_ssa->ce && !res_ssa->is_instanceof) {
									ce = res_ssa->ce;
								}
							}
						} else {
							if (opline->op1_type == IS_CONST) {
								zval *zv = RT_CONSTANT(opline, opline->op1);
								if (C_TYPE_P(zv) == IS_STRING) {
									zval *lc = zv + 1;
									ce = (crex_class_entry*)crex_hash_find_ptr(EG(class_table), C_STR_P(lc));
								}
							}
						}

						i++;

						if (!ce || !(ce->ce_flags & CREX_ACC_LINKED) || ce->constructor) {
							const crex_op *next_opline = opline + 1;

							crex_jit_cond_jmp(&dasm_state, next_opline, ssa->cfg.blocks[b].successors[0]);
							if (JIT_G(opt_level) < CREX_JIT_LEVEL_INLINE) {
								crex_jit_call(&dasm_state, next_opline, b + 1);
								is_terminated = 1;
							} else {
								crex_jit_do_fcall(&dasm_state, next_opline, op_array, ssa, call_level, b + 1, NULL);
							}
						}

						/* We skip over the DO_FCALL, so decrement call_level ourselves. */
						call_level--;
					}
					break;
				default:
					if (!crex_jit_handler(&dasm_state, opline,
							crex_may_throw(opline, ssa_op, op_array, ssa))) {
						goto jit_failure;
					}
					if (i == end
					 && (opline->result_type & (IS_SMART_BRANCH_JMPZ|IS_SMART_BRANCH_JMPNZ)) != 0) {
						/* smart branch split across basic blocks */
						if (!crex_jit_cond_jmp(&dasm_state, opline + 2, ssa->cfg.blocks[b+1].successors[0])) {
							goto jit_failure;
						}
						if (!crex_jit_jmp(&dasm_state, ssa->cfg.blocks[b+1].successors[1])) {
							goto jit_failure;
						}
						is_terminated = 1;
					}
			}
done:
			switch (opline->opcode) {
				case CREX_DO_FCALL:
				case CREX_DO_ICALL:
				case CREX_DO_UCALL:
				case CREX_DO_FCALL_BY_NAME:
				case CREX_CALLABLE_CONVERT:
					call_level--;
			}
		}
	}

	handler = dasm_link_and_encode(&dasm_state, op_array, ssa, rt_opline, ra, NULL, 0,
		(crex_jit_vm_kind == CREX_VM_KIND_HYBRID) ? SP_ADJ_VM : SP_ADJ_RET, SP_ADJ_JIT);
	if (!handler) {
		goto jit_failure;
	}
	dasm_free(&dasm_state);

	if (JIT_G(opt_flags) & (CREX_JIT_REG_ALLOC_LOCAL|CREX_JIT_REG_ALLOC_GLOBAL)) {
		crex_arena_release(&CG(arena), checkpoint);
	}
	return SUCCESS;

jit_failure:
	if (dasm_state) {
		dasm_free(&dasm_state);
	}
	if (JIT_G(opt_flags) & (CREX_JIT_REG_ALLOC_LOCAL|CREX_JIT_REG_ALLOC_GLOBAL)) {
		crex_arena_release(&CG(arena), checkpoint);
	}
	return FAILURE;
}

static void crex_jit_collect_calls(crex_op_array *op_array, crex_script *script)
{
	crex_func_info *func_info;

	if (JIT_G(trigger) == CREX_JIT_ON_FIRST_EXEC ||
	    JIT_G(trigger) == CREX_JIT_ON_PROF_REQUEST ||
	    JIT_G(trigger) == CREX_JIT_ON_HOT_COUNTERS) {
	    func_info = CREX_FUNC_INFO(op_array);
	} else {
		func_info = crex_arena_calloc(&CG(arena), 1, sizeof(crex_func_info));
		CREX_SET_FUNC_INFO(op_array, func_info);
	}
	crex_analyze_calls(&CG(arena), script, CREX_CALL_TREE, op_array, func_info);
}

static void crex_jit_cleanup_func_info(crex_op_array *op_array)
{
	crex_func_info *func_info = CREX_FUNC_INFO(op_array);
	crex_call_info *caller_info, *callee_info;

	if (func_info) {
		caller_info = func_info->caller_info;
		callee_info = func_info->callee_info;

		if (JIT_G(trigger) == CREX_JIT_ON_FIRST_EXEC ||
		    JIT_G(trigger) == CREX_JIT_ON_PROF_REQUEST ||
		    JIT_G(trigger) == CREX_JIT_ON_HOT_COUNTERS) {
			func_info->num = 0;
			func_info->flags &= CREX_FUNC_JIT_ON_FIRST_EXEC
				| CREX_FUNC_JIT_ON_PROF_REQUEST
				| CREX_FUNC_JIT_ON_HOT_COUNTERS
				| CREX_FUNC_JIT_ON_HOT_TRACE;
			memset(&func_info->ssa, 0, sizeof(crex_func_info) - offsetof(crex_func_info, ssa));
		} else {
			CREX_SET_FUNC_INFO(op_array, NULL);
		}

		while (caller_info) {
			if (caller_info->caller_op_array) {
				crex_jit_cleanup_func_info(caller_info->caller_op_array);
			}
			caller_info = caller_info->next_caller;
		}
		while (callee_info) {
			if (callee_info->callee_func && callee_info->callee_func->type == CREX_USER_FUNCTION) {
				crex_jit_cleanup_func_info(&callee_info->callee_func->op_array);
			}
			callee_info = callee_info->next_callee;
		}
	}
}

static int crex_real_jit_func(crex_op_array *op_array, crex_script *script, const crex_op *rt_opline)
{
	crex_ssa ssa;
	void *checkpoint;
	crex_func_info *func_info;

	if (*dasm_ptr == dasm_end) {
		return FAILURE;
	}

	checkpoint = crex_arena_checkpoint(CG(arena));

	/* Build SSA */
	memset(&ssa, 0, sizeof(crex_ssa));

	if (crex_jit_op_array_analyze1(op_array, script, &ssa) != SUCCESS) {
		goto jit_failure;
	}

	if (JIT_G(opt_level) >= CREX_JIT_LEVEL_OPT_FUNCS) {
		crex_jit_collect_calls(op_array, script);
		func_info = CREX_FUNC_INFO(op_array);
		func_info->call_map = crex_build_call_map(&CG(arena), func_info, op_array);
		if (op_array->fn_flags & CREX_ACC_HAS_RETURN_TYPE) {
			crex_init_func_return_info(op_array, script, &func_info->return_info);
		}
	}

	if (crex_jit_op_array_analyze2(op_array, script, &ssa, ZCG(accel_directives).optimization_level) != SUCCESS) {
		goto jit_failure;
	}

	if (JIT_G(debug) & CREX_JIT_DEBUG_SSA) {
		crex_dump_op_array(op_array, CREX_DUMP_HIDE_UNREACHABLE|CREX_DUMP_RC_INFERENCE|CREX_DUMP_SSA, "JIT", &ssa);
	}

	if (crex_jit(op_array, &ssa, rt_opline) != SUCCESS) {
		goto jit_failure;
	}

	crex_jit_cleanup_func_info(op_array);
	crex_arena_release(&CG(arena), checkpoint);
	return SUCCESS;

jit_failure:
	crex_jit_cleanup_func_info(op_array);
	crex_arena_release(&CG(arena), checkpoint);
	return FAILURE;
}

/* Run-time JIT handler */
static int CREX_FASTCALL crex_runtime_jit(void)
{
	crex_execute_data *execute_data = EG(current_execute_data);
	crex_op_array *op_array = &EX(func)->op_array;
	crex_op *opline = op_array->opcodes;
	crex_jit_op_array_extension *jit_extension;
	bool do_bailout = 0;

	crex_shared_alloc_lock();

	if (CREX_FUNC_INFO(op_array)) {

		SHM_UNPROTECT();
		crex_jit_unprotect();

		crex_try {
			/* restore original opcode handlers */
			if (!(op_array->fn_flags & CREX_ACC_HAS_TYPE_HINTS)) {
				while (opline->opcode == CREX_RECV || opline->opcode == CREX_RECV_INIT) {
					opline++;
				}
			}
			jit_extension = (crex_jit_op_array_extension*)CREX_FUNC_INFO(op_array);
			opline->handler = jit_extension->orig_handler;

			/* perform real JIT for this function */
			crex_real_jit_func(op_array, NULL, NULL);
		} crex_catch {
			do_bailout = true;
		} crex_end_try();

		crex_jit_protect();
		SHM_PROTECT();
	}

	crex_shared_alloc_unlock();

	if (do_bailout) {
		crex_bailout();
	}

	/* JIT-ed code is going to be called by VM */
	return 0;
}

void crex_jit_check_funcs(HashTable *function_table, bool is_method) {
	crex_op *opline;
	crex_function *func;
	crex_op_array *op_array;
	uintptr_t counter;
	crex_jit_op_array_extension *jit_extension;

	CREX_HASH_MAP_REVERSE_FOREACH_PTR(function_table, func) {
		if (func->type == CREX_INTERNAL_FUNCTION) {
			break;
		}
		op_array = &func->op_array;
		opline = op_array->opcodes;
		if (!(op_array->fn_flags & CREX_ACC_HAS_TYPE_HINTS)) {
			while (opline->opcode == CREX_RECV || opline->opcode == CREX_RECV_INIT) {
				opline++;
			}
		}
		if (opline->handler == crex_jit_profile_jit_handler) {
			if (!RUN_TIME_CACHE(op_array)) {
				continue;
			}
			counter = (uintptr_t)CREX_COUNTER_INFO(op_array);
			CREX_COUNTER_INFO(op_array) = 0;
			jit_extension = (crex_jit_op_array_extension*)CREX_FUNC_INFO(op_array);
			opline->handler = jit_extension->orig_handler;
			if (((double)counter / (double)crex_jit_profile_counter) > JIT_G(prof_threshold)) {
				crex_real_jit_func(op_array, NULL, NULL);
			}
		}
	} CREX_HASH_FOREACH_END();
}

void CREX_FASTCALL crex_jit_hot_func(crex_execute_data *execute_data, const crex_op *opline)
{
	crex_op_array *op_array = &EX(func)->op_array;
	crex_jit_op_array_hot_extension *jit_extension;
	uint32_t i;
	bool do_bailout = 0;

	crex_shared_alloc_lock();
	jit_extension = (crex_jit_op_array_hot_extension*)CREX_FUNC_INFO(op_array);

	if (jit_extension) {
		SHM_UNPROTECT();
		crex_jit_unprotect();

		crex_try {
			for (i = 0; i < op_array->last; i++) {
				op_array->opcodes[i].handler = jit_extension->orig_handlers[i];
			}

			/* perform real JIT for this function */
			crex_real_jit_func(op_array, NULL, opline);
		} crex_catch {
			do_bailout = 1;
		} crex_end_try();

		crex_jit_protect();
		SHM_PROTECT();
	}

	crex_shared_alloc_unlock();

	if (do_bailout) {
		crex_bailout();
	}
	/* JIT-ed code is going to be called by VM */
}

static void crex_jit_setup_hot_counters_ex(crex_op_array *op_array, crex_cfg *cfg)
{
	if (JIT_G(hot_func)) {
		crex_op *opline = op_array->opcodes;

		if (!(op_array->fn_flags & CREX_ACC_HAS_TYPE_HINTS)) {
			while (opline->opcode == CREX_RECV || opline->opcode == CREX_RECV_INIT) {
				opline++;
			}
		}

		opline->handler = (const void*)crex_jit_func_hot_counter_handler;
	}

	if (JIT_G(hot_loop)) {
		uint32_t i;

		for (i = 0; i < cfg->blocks_count; i++) {
			if ((cfg->blocks[i].flags & CREX_BB_REACHABLE) &&
			    (cfg->blocks[i].flags & CREX_BB_LOOP_HEADER)) {
			    op_array->opcodes[cfg->blocks[i].start].handler =
					(const void*)crex_jit_loop_hot_counter_handler;
			}
		}
	}
}

static int crex_jit_restart_hot_counters(crex_op_array *op_array)
{
	crex_jit_op_array_hot_extension *jit_extension;
	crex_cfg cfg;
	uint32_t i;

	jit_extension = (crex_jit_op_array_hot_extension*)CREX_FUNC_INFO(op_array);
	for (i = 0; i < op_array->last; i++) {
		op_array->opcodes[i].handler = jit_extension->orig_handlers[i];
	}

	if (crex_jit_build_cfg(op_array, &cfg) != SUCCESS) {
		return FAILURE;
	}

	crex_jit_setup_hot_counters_ex(op_array, &cfg);

	return SUCCESS;
}

static int crex_jit_setup_hot_counters(crex_op_array *op_array)
{
	crex_jit_op_array_hot_extension *jit_extension;
	crex_cfg cfg;
	uint32_t i;

	CREX_ASSERT(crex_jit_func_hot_counter_handler != NULL);
	CREX_ASSERT(crex_jit_loop_hot_counter_handler != NULL);

	if (crex_jit_build_cfg(op_array, &cfg) != SUCCESS) {
		return FAILURE;
	}

	jit_extension = (crex_jit_op_array_hot_extension*)crex_shared_alloc(sizeof(crex_jit_op_array_hot_extension) + (op_array->last - 1) * sizeof(void*));
	if (!jit_extension) {
		return FAILURE;
	}
	memset(&jit_extension->func_info, 0, sizeof(crex_func_info));
	jit_extension->func_info.flags = CREX_FUNC_JIT_ON_HOT_COUNTERS;
	jit_extension->counter = &crex_jit_hot_counters[crex_jit_op_array_hash(op_array) & (CREX_HOT_COUNTERS_COUNT - 1)];
	for (i = 0; i < op_array->last; i++) {
		jit_extension->orig_handlers[i] = op_array->opcodes[i].handler;
	}
	CREX_SET_FUNC_INFO(op_array, (void*)jit_extension);

	crex_jit_setup_hot_counters_ex(op_array, &cfg);

	crex_shared_alloc_register_xlat_entry(op_array->opcodes, jit_extension);

	return SUCCESS;
}

#include "jit/crex_jit_trace.c"

CREX_EXT_API int crex_jit_op_array(crex_op_array *op_array, crex_script *script)
{
	if (dasm_ptr == NULL) {
		return FAILURE;
	}

	if (JIT_G(trigger) == CREX_JIT_ON_FIRST_EXEC) {
		crex_jit_op_array_extension *jit_extension;
		crex_op *opline = op_array->opcodes;

		if (CG(compiler_options) & CREX_COMPILE_PRELOAD) {
			CREX_SET_FUNC_INFO(op_array, NULL);
			crex_error(E_WARNING, "Preloading is incompatible with first-exec and profile triggered JIT");
			return SUCCESS;
		}

		/* Set run-time JIT handler */
		CREX_ASSERT(crex_jit_runtime_jit_handler != NULL);
		if (!(op_array->fn_flags & CREX_ACC_HAS_TYPE_HINTS)) {
			while (opline->opcode == CREX_RECV || opline->opcode == CREX_RECV_INIT) {
				opline++;
			}
		}
		jit_extension = (crex_jit_op_array_extension*)crex_shared_alloc(sizeof(crex_jit_op_array_extension));
		if (!jit_extension) {
			return FAILURE;
		}
		memset(&jit_extension->func_info, 0, sizeof(crex_func_info));
		jit_extension->func_info.flags = CREX_FUNC_JIT_ON_FIRST_EXEC;
		jit_extension->orig_handler = (void*)opline->handler;
		CREX_SET_FUNC_INFO(op_array, (void*)jit_extension);
		opline->handler = (const void*)crex_jit_runtime_jit_handler;
		crex_shared_alloc_register_xlat_entry(op_array->opcodes, jit_extension);

		return SUCCESS;
	} else if (JIT_G(trigger) == CREX_JIT_ON_PROF_REQUEST) {
		crex_jit_op_array_extension *jit_extension;
		crex_op *opline = op_array->opcodes;

		if (CG(compiler_options) & CREX_COMPILE_PRELOAD) {
			CREX_SET_FUNC_INFO(op_array, NULL);
			crex_error(E_WARNING, "Preloading is incompatible with first-exec and profile triggered JIT");
			return SUCCESS;
		}

		CREX_ASSERT(crex_jit_profile_jit_handler != NULL);
		if (op_array->function_name) {
			if (!(op_array->fn_flags & CREX_ACC_HAS_TYPE_HINTS)) {
				while (opline->opcode == CREX_RECV || opline->opcode == CREX_RECV_INIT) {
					opline++;
				}
			}
			jit_extension = (crex_jit_op_array_extension*)crex_shared_alloc(sizeof(crex_jit_op_array_extension));
			if (!jit_extension) {
				return FAILURE;
			}
			memset(&jit_extension->func_info, 0, sizeof(crex_func_info));
			jit_extension->func_info.flags = CREX_FUNC_JIT_ON_PROF_REQUEST;
			jit_extension->orig_handler = (void*)opline->handler;
			CREX_SET_FUNC_INFO(op_array, (void*)jit_extension);
			opline->handler = (const void*)crex_jit_profile_jit_handler;
			crex_shared_alloc_register_xlat_entry(op_array->opcodes, jit_extension);
		}

		return SUCCESS;
	} else if (JIT_G(trigger) == CREX_JIT_ON_HOT_COUNTERS) {
		return crex_jit_setup_hot_counters(op_array);
	} else if (JIT_G(trigger) == CREX_JIT_ON_HOT_TRACE) {
		return crex_jit_setup_hot_trace_counters(op_array);
	} else if (JIT_G(trigger) == CREX_JIT_ON_SCRIPT_LOAD) {
		return crex_real_jit_func(op_array, script, NULL);
	} else {
		CREX_UNREACHABLE();
	}
}

CREX_EXT_API int crex_jit_script(crex_script *script)
{
	void *checkpoint;
	crex_call_graph call_graph;
	crex_func_info *info;
	int i;

	if (dasm_ptr == NULL || *dasm_ptr == dasm_end) {
		return FAILURE;
	}

	checkpoint = crex_arena_checkpoint(CG(arena));

	call_graph.op_arrays_count = 0;
	crex_build_call_graph(&CG(arena), script, &call_graph);

	crex_analyze_call_graph(&CG(arena), script, &call_graph);

	if (JIT_G(trigger) == CREX_JIT_ON_FIRST_EXEC ||
	    JIT_G(trigger) == CREX_JIT_ON_PROF_REQUEST ||
	    JIT_G(trigger) == CREX_JIT_ON_HOT_COUNTERS ||
	    JIT_G(trigger) == CREX_JIT_ON_HOT_TRACE) {
		for (i = 0; i < call_graph.op_arrays_count; i++) {
			if (crex_jit_op_array(call_graph.op_arrays[i], script) != SUCCESS) {
				goto jit_failure;
			}
		}
	} else if (JIT_G(trigger) == CREX_JIT_ON_SCRIPT_LOAD) {
		for (i = 0; i < call_graph.op_arrays_count; i++) {
			info = CREX_FUNC_INFO(call_graph.op_arrays[i]);
			if (info) {
				if (crex_jit_op_array_analyze1(call_graph.op_arrays[i], script, &info->ssa) != SUCCESS) {
					goto jit_failure;
				}
				info->flags = info->ssa.cfg.flags;
			}
		}

		for (i = 0; i < call_graph.op_arrays_count; i++) {
			info = CREX_FUNC_INFO(call_graph.op_arrays[i]);
			if (info) {
				info->call_map = crex_build_call_map(&CG(arena), info, call_graph.op_arrays[i]);
				if (call_graph.op_arrays[i]->fn_flags & CREX_ACC_HAS_RETURN_TYPE) {
					crex_init_func_return_info(call_graph.op_arrays[i], script, &info->return_info);
				}
			}
		}

		for (i = 0; i < call_graph.op_arrays_count; i++) {
			info = CREX_FUNC_INFO(call_graph.op_arrays[i]);
			if (info) {
				if (crex_jit_op_array_analyze2(call_graph.op_arrays[i], script, &info->ssa, ZCG(accel_directives).optimization_level) != SUCCESS) {
					goto jit_failure;
				}
				info->flags = info->ssa.cfg.flags;
			}
		}

		for (i = 0; i < call_graph.op_arrays_count; i++) {
			info = CREX_FUNC_INFO(call_graph.op_arrays[i]);
			if (info) {
				if (JIT_G(debug) & CREX_JIT_DEBUG_SSA) {
					crex_dump_op_array(call_graph.op_arrays[i], CREX_DUMP_HIDE_UNREACHABLE|CREX_DUMP_RC_INFERENCE|CREX_DUMP_SSA, "JIT", &info->ssa);
				}
				if (crex_jit(call_graph.op_arrays[i], &info->ssa, NULL) != SUCCESS) {
					goto jit_failure;
				}
			}
		}

		for (i = 0; i < call_graph.op_arrays_count; i++) {
			CREX_SET_FUNC_INFO(call_graph.op_arrays[i], NULL);
		}
	} else {
		CREX_UNREACHABLE();
	}

	crex_arena_release(&CG(arena), checkpoint);

	if (JIT_G(trigger) == CREX_JIT_ON_FIRST_EXEC
	 || JIT_G(trigger) == CREX_JIT_ON_PROF_REQUEST
	 || JIT_G(trigger) == CREX_JIT_ON_HOT_COUNTERS
	 || JIT_G(trigger) == CREX_JIT_ON_HOT_TRACE) {
		crex_class_entry *ce;
		crex_op_array *op_array;

		CREX_HASH_MAP_FOREACH_PTR(&script->class_table, ce) {
			CREX_HASH_MAP_FOREACH_PTR(&ce->function_table, op_array) {
				if (!CREX_FUNC_INFO(op_array)) {
					void *jit_extension = crex_shared_alloc_get_xlat_entry(op_array->opcodes);

					if (jit_extension) {
						CREX_SET_FUNC_INFO(op_array, jit_extension);
					}
				}
			} CREX_HASH_FOREACH_END();
		} CREX_HASH_FOREACH_END();
	}

	return SUCCESS;

jit_failure:
	if (JIT_G(trigger) == CREX_JIT_ON_SCRIPT_LOAD) {
		for (i = 0; i < call_graph.op_arrays_count; i++) {
			CREX_SET_FUNC_INFO(call_graph.op_arrays[i], NULL);
		}
	}
	crex_arena_release(&CG(arena), checkpoint);
	return FAILURE;
}

CREX_EXT_API void crex_jit_unprotect(void)
{
#ifdef HAVE_MPROTECT
	if (!(JIT_G(debug) & (CREX_JIT_DEBUG_GDB|CREX_JIT_DEBUG_PERF_DUMP))) {
		int opts = PROT_READ | PROT_WRITE;
#ifdef ZTS
#ifdef HAVE_PTHREAD_JIT_WRITE_PROTECT_NP
		if (crex_write_protect) {
			pthread_jit_write_protect_np(0);
		}
#endif
		opts |= PROT_EXEC;
#endif
		if (mprotect(dasm_buf, dasm_size, opts) != 0) {
			fprintf(stderr, "mprotect() failed [%d] %s\n", errno, strerror(errno));
		}
	}
#elif _WIN32
	if (!(JIT_G(debug) & (CREX_JIT_DEBUG_GDB|CREX_JIT_DEBUG_PERF_DUMP))) {
		DWORD old, new;
#ifdef ZTS
		new = PAGE_EXECUTE_READWRITE;
#else
		new = PAGE_READWRITE;
#endif
		if (!VirtualProtect(dasm_buf, dasm_size, new, &old)) {
			DWORD err = GetLastError();
			char *msg = crx_win32_error_to_msg(err);
			fprintf(stderr, "VirtualProtect() failed [%u] %s\n", err, msg);
			crx_win32_error_msg_free(msg);
		}
	}
#endif
}

CREX_EXT_API void crex_jit_protect(void)
{
#ifdef HAVE_MPROTECT
	if (!(JIT_G(debug) & (CREX_JIT_DEBUG_GDB|CREX_JIT_DEBUG_PERF_DUMP))) {
#ifdef HAVE_PTHREAD_JIT_WRITE_PROTECT_NP
		if (crex_write_protect) {
			pthread_jit_write_protect_np(1);
		}
#endif
		if (mprotect(dasm_buf, dasm_size, PROT_READ | PROT_EXEC) != 0) {
			fprintf(stderr, "mprotect() failed [%d] %s\n", errno, strerror(errno));
		}
	}
#elif _WIN32
	if (!(JIT_G(debug) & (CREX_JIT_DEBUG_GDB|CREX_JIT_DEBUG_PERF_DUMP))) {
		DWORD old;

		if (!VirtualProtect(dasm_buf, dasm_size, PAGE_EXECUTE_READ, &old)) {
			DWORD err = GetLastError();
			char *msg = crx_win32_error_to_msg(err);
			fprintf(stderr, "VirtualProtect() failed [%u] %s\n", err, msg);
			crx_win32_error_msg_free(msg);
		}
	}
#endif
}

static void crex_jit_init_handlers(void)
{
	if (crex_jit_vm_kind == CREX_VM_KIND_HYBRID) {
		crex_jit_runtime_jit_handler = dasm_labels[crex_lbhybrid_runtime_jit];
		crex_jit_profile_jit_handler = dasm_labels[crex_lbhybrid_profile_jit];
		crex_jit_func_hot_counter_handler = dasm_labels[crex_lbhybrid_func_hot_counter];
		crex_jit_loop_hot_counter_handler = dasm_labels[crex_lbhybrid_loop_hot_counter];
		crex_jit_func_trace_counter_handler = dasm_labels[crex_lbhybrid_func_trace_counter];
		crex_jit_ret_trace_counter_handler = dasm_labels[crex_lbhybrid_ret_trace_counter];
		crex_jit_loop_trace_counter_handler = dasm_labels[crex_lbhybrid_loop_trace_counter];
	} else {
		crex_jit_runtime_jit_handler = (const void*)crex_runtime_jit;
		crex_jit_profile_jit_handler = (const void*)crex_jit_profile_helper;
		crex_jit_func_hot_counter_handler = (const void*)crex_jit_func_counter_helper;
		crex_jit_loop_hot_counter_handler = (const void*)crex_jit_loop_counter_helper;
		crex_jit_func_trace_counter_handler = (const void*)crex_jit_func_trace_helper;
		crex_jit_ret_trace_counter_handler = (const void*)crex_jit_ret_trace_helper;
		crex_jit_loop_trace_counter_handler = (const void*)crex_jit_loop_trace_helper;
	}
}

static int crex_jit_make_stubs(void)
{
	dasm_State* dasm_state = NULL;
	uint32_t i;

	dasm_init(&dasm_state, DASM_MAXSECTION);
	dasm_setupglobal(&dasm_state, dasm_labels, crex_lb_MAX);

	for (i = 0; i < sizeof(crex_jit_stubs)/sizeof(crex_jit_stubs[0]); i++) {
		dasm_setup(&dasm_state, dasm_actions);
		crex_jit_align_stub(&dasm_state);
		if (!crex_jit_stubs[i].stub(&dasm_state)) {
			return 0;
		}
		if (!dasm_link_and_encode(&dasm_state, NULL, NULL, NULL, NULL, crex_jit_stubs[i].name, 0,
				crex_jit_stubs[i].offset, crex_jit_stubs[i].adjustment)) {
			return 0;
		}
	}

	crex_jit_init_handlers();

	dasm_free(&dasm_state);
	return 1;
}

static void crex_jit_globals_ctor(crex_jit_globals *jit_globals)
{
	memset(jit_globals, 0, sizeof(crex_jit_globals));
	crex_jit_trace_init_caches();
}

#ifdef ZTS
static void crex_jit_globals_dtor(crex_jit_globals *jit_globals)
{
	crex_jit_trace_free_caches(jit_globals);
}
#endif

static int crex_jit_parse_config_num(crex_long jit)
{
	if (jit == 0) {
		JIT_G(on) = 0;
		return SUCCESS;
	}

	if (jit < 0) return FAILURE;

	if (jit % 10 == 0 || jit % 10 > 5) return FAILURE;
	JIT_G(opt_level) = jit % 10;

	jit /= 10;
	if (jit % 10 > 5) return FAILURE;
	JIT_G(trigger) = jit % 10;

	jit /= 10;
	if (jit % 10 > 2) return FAILURE;
	JIT_G(opt_flags) = jit % 10;

	jit /= 10;
	if (jit % 10 > 1) return FAILURE;
	JIT_G(opt_flags) |= ((jit % 10) ? CREX_JIT_CPU_AVX : 0);

	if (jit / 10 != 0) return FAILURE;

	JIT_G(on) = 1;

	return SUCCESS;
}

CREX_EXT_API int crex_jit_config(crex_string *jit, int stage)
{
	if (stage != CREX_INI_STAGE_STARTUP && !JIT_G(enabled)) {
		if (stage == CREX_INI_STAGE_RUNTIME) {
			crex_error(E_WARNING, "Cannot change opcache.jit setting at run-time (JIT is disabled)");
		}
		return FAILURE;
	}

	if (ZSTR_LEN(jit) == 0
	 || crex_string_equals_literal_ci(jit, "disable")) {
		JIT_G(enabled) = 0;
		JIT_G(on) = 0;
		return SUCCESS;
	} else if (crex_string_equals_literal_ci(jit, "0")
			|| crex_string_equals_literal_ci(jit, "off")
			|| crex_string_equals_literal_ci(jit, "no")
			|| crex_string_equals_literal_ci(jit, "false")) {
		JIT_G(enabled) = 1;
		JIT_G(on) = 0;
		return SUCCESS;
	} else if (crex_string_equals_literal_ci(jit, "1")
			|| crex_string_equals_literal_ci(jit, "on")
			|| crex_string_equals_literal_ci(jit, "yes")
			|| crex_string_equals_literal_ci(jit, "true")
			|| crex_string_equals_literal_ci(jit, "tracing")) {
		JIT_G(enabled) = 1;
		JIT_G(on) = 1;
		JIT_G(opt_level) = CREX_JIT_LEVEL_OPT_FUNCS;
		JIT_G(trigger) = CREX_JIT_ON_HOT_TRACE;
		JIT_G(opt_flags) = CREX_JIT_REG_ALLOC_GLOBAL | CREX_JIT_CPU_AVX;
		return SUCCESS;
	} else if (crex_string_equals_ci(jit, ZSTR_KNOWN(CREX_STR_FUNCTION))) {
		JIT_G(enabled) = 1;
		JIT_G(on) = 1;
		JIT_G(opt_level) = CREX_JIT_LEVEL_OPT_SCRIPT;
		JIT_G(trigger) = CREX_JIT_ON_SCRIPT_LOAD;
		JIT_G(opt_flags) = CREX_JIT_REG_ALLOC_GLOBAL | CREX_JIT_CPU_AVX;
		return SUCCESS;
	} else  {
		char *end;
		crex_long num = CREX_STRTOL(ZSTR_VAL(jit), &end, 10);
		if (end != ZSTR_VAL(jit) + ZSTR_LEN(jit) || crex_jit_parse_config_num(num) != SUCCESS) {
			goto failure;
		}
		JIT_G(enabled) = 1;
		return SUCCESS;
	}

failure:
	crex_error(E_WARNING, "Invalid \"opcache.jit\" setting. Should be \"disable\", \"on\", \"off\", \"tracing\", \"function\" or 4-digit number");
	JIT_G(enabled) = 0;
	JIT_G(on) = 0;
	return FAILURE;
}

CREX_EXT_API int crex_jit_debug_config(crex_long old_val, crex_long new_val, int stage)
{
	if (stage != CREX_INI_STAGE_STARTUP) {
		if (((old_val ^ new_val) & CREX_JIT_DEBUG_PERSISTENT) != 0) {
			if (stage == CREX_INI_STAGE_RUNTIME) {
				crex_error(E_WARNING, "Some opcache.jit_debug bits cannot be changed after startup");
			}
			return FAILURE;
		}
#ifdef HAVE_DISASM
		if (new_val & (CREX_JIT_DEBUG_ASM|CREX_JIT_DEBUG_ASM_STUBS)) {
			if (JIT_G(enabled) && !JIT_G(symbols) && !crex_jit_disasm_init()) {
				// TODO: error reporting and cleanup ???
				return FAILURE;
			}
			// TODO: symbols for JIT-ed code compiled before are missing ???
		}
#endif
	}
	return SUCCESS;
}

CREX_EXT_API void crex_jit_init(void)
{
#ifdef ZTS
	jit_globals_id = ts_allocate_id(&jit_globals_id, sizeof(crex_jit_globals), (ts_allocate_ctor) crex_jit_globals_ctor, (ts_allocate_dtor) crex_jit_globals_dtor);
#else
	crex_jit_globals_ctor(&jit_globals);
#endif
}

CREX_EXT_API int crex_jit_check_support(void)
{
	int i;

	crex_jit_vm_kind = crex_vm_kind();
	if (crex_jit_vm_kind != CREX_VM_KIND_CALL &&
	    crex_jit_vm_kind != CREX_VM_KIND_HYBRID) {
		crex_error(E_WARNING, "JIT is compatible only with CALL and HYBRID VM. JIT disabled.");
		JIT_G(enabled) = 0;
		JIT_G(on) = 0;
		return FAILURE;
	}

	if (crex_execute_ex != execute_ex) {
		if (strcmp(sapi_module.name, "crxdbg") != 0) {
			crex_error(E_WARNING, "JIT is incompatible with third party extensions that override crex_execute_ex(). JIT disabled.");
		}
		JIT_G(enabled) = 0;
		JIT_G(on) = 0;
		return FAILURE;
	}

	for (i = 0; i <= 256; i++) {
		switch (i) {
			/* JIT has no effect on these opcodes */
			case CREX_BEGIN_SILENCE:
			case CREX_END_SILENCE:
			case CREX_EXIT:
				break;
			default:
				if (crex_get_user_opcode_handler(i) != NULL) {
					crex_error(E_WARNING, "JIT is incompatible with third party extensions that setup user opcode handlers. JIT disabled.");
					JIT_G(enabled) = 0;
					JIT_G(on) = 0;
					return FAILURE;
				}
		}
	}

	return SUCCESS;
}

CREX_EXT_API int crex_jit_startup(void *buf, size_t size, bool reattached)
{
	int ret;

	crex_jit_halt_op = crex_get_halt_op();

	if (crex_jit_setup() != SUCCESS) {
		// TODO: error reporting and cleanup ???
		return FAILURE;
	}

	crex_jit_profile_counter_rid = crex_get_op_array_extension_handle(ACCELERATOR_PRODUCT_NAME);

#ifdef HAVE_GDB
	crex_jit_gdb_init();
#endif

#if CREX_JIT_SUPPORT_CLDEMOTE
	cpu_support_cldemote = crex_cpu_supports_cldemote();
#endif

#ifdef HAVE_PTHREAD_JIT_WRITE_PROTECT_NP
	crex_write_protect = pthread_jit_write_protect_supported_np();
#endif

	dasm_buf = buf;
	dasm_size = size;

#ifdef HAVE_MPROTECT
#ifdef HAVE_PTHREAD_JIT_WRITE_PROTECT_NP
	if (crex_write_protect) {
		pthread_jit_write_protect_np(1);
	}
#endif
	if (JIT_G(debug) & (CREX_JIT_DEBUG_GDB|CREX_JIT_DEBUG_PERF_DUMP)) {
		if (mprotect(dasm_buf, dasm_size, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
			fprintf(stderr, "mprotect() failed [%d] %s\n", errno, strerror(errno));
		}
	} else {
		if (mprotect(dasm_buf, dasm_size, PROT_READ | PROT_EXEC) != 0) {
			fprintf(stderr, "mprotect() failed [%d] %s\n", errno, strerror(errno));
		}
	}
#elif _WIN32
	if (JIT_G(debug) & (CREX_JIT_DEBUG_GDB|CREX_JIT_DEBUG_PERF_DUMP)) {
		DWORD old;

		if (!VirtualProtect(dasm_buf, dasm_size, PAGE_EXECUTE_READWRITE, &old)) {
			DWORD err = GetLastError();
			char *msg = crx_win32_error_to_msg(err);
			fprintf(stderr, "VirtualProtect() failed [%u] %s\n", err, msg);
			crx_win32_error_msg_free(msg);
		}
	} else {
		DWORD old;

		if (!VirtualProtect(dasm_buf, dasm_size, PAGE_EXECUTE_READ, &old)) {
			DWORD err = GetLastError();
			char *msg = crx_win32_error_to_msg(err);
			fprintf(stderr, "VirtualProtect() failed [%u] %s\n", err, msg);
			crx_win32_error_msg_free(msg);
		}
	}
#endif

	dasm_ptr = dasm_end = (void*)(((char*)dasm_buf) + size - sizeof(*dasm_ptr) * 2);
	if (!reattached) {
		crex_jit_unprotect();
		*dasm_ptr = dasm_buf;
#if _WIN32
		/* reserve space for global labels */
		*dasm_ptr = (void**)*dasm_ptr + crex_lb_MAX;
#endif
		crex_jit_protect();
	}

#ifdef HAVE_DISASM
	if (JIT_G(debug) & (CREX_JIT_DEBUG_ASM|CREX_JIT_DEBUG_ASM_STUBS)) {
		if (!crex_jit_disasm_init()) {
			// TODO: error reporting and cleanup ???
			return FAILURE;
		}
	}
#endif

#ifdef HAVE_PERFTOOLS
	if (JIT_G(debug) & CREX_JIT_DEBUG_PERF_DUMP) {
		crex_jit_perf_jitdump_open();
	}
#endif

	if (!reattached) {
		crex_jit_unprotect();
		ret = crex_jit_make_stubs();
#if _WIN32
		/* save global labels */
		memcpy(dasm_buf, dasm_labels, sizeof(void*) * crex_lb_MAX);
#endif
		crex_jit_protect();
		if (!ret) {
			// TODO: error reporting and cleanup ???
			return FAILURE;
		}
	} else {
#if _WIN32
		/* restore global labels */
		memcpy(dasm_labels, dasm_buf, sizeof(void*) * crex_lb_MAX);
		crex_jit_init_handlers();
#endif
	}

	if (crex_jit_trace_startup(reattached) != SUCCESS) {
		return FAILURE;
	}

	crex_jit_unprotect();
#if CREX_JIT_TARGET_ARM64
	/* reserve space for global labels veneers */
	dasm_labels_veneers = *dasm_ptr;
	*dasm_ptr = (void**)*dasm_ptr + CREX_MM_ALIGNED_SIZE_EX(crex_lb_MAX, DASM_ALIGNMENT);
	memset(dasm_labels_veneers, 0, sizeof(void*) * CREX_MM_ALIGNED_SIZE_EX(crex_lb_MAX, DASM_ALIGNMENT));
#endif
	/* save JIT buffer pos */
	dasm_ptr[1] = dasm_ptr[0];
	crex_jit_protect();

	return SUCCESS;
}

CREX_EXT_API void crex_jit_shutdown(void)
{
	if (JIT_G(debug) & CREX_JIT_DEBUG_SIZE) {
		fprintf(stderr, "\nJIT memory usage: %td\n", (ptrdiff_t)((char*)*dasm_ptr - (char*)dasm_buf));
	}

#ifdef HAVE_GDB
	if (JIT_G(debug) & CREX_JIT_DEBUG_GDB) {
		crex_jit_gdb_unregister();
	}
#endif

#ifdef HAVE_DISASM
	crex_jit_disasm_shutdown();
#endif

#ifdef HAVE_PERFTOOLS
	if (JIT_G(debug) & CREX_JIT_DEBUG_PERF_DUMP) {
		crex_jit_perf_jitdump_close();
	}
#endif
#ifdef ZTS
	ts_free_id(jit_globals_id);
#else
	crex_jit_trace_free_caches(&jit_globals);
#endif
}

static void crex_jit_reset_counters(void)
{
	int i;

	for (i = 0; i < CREX_HOT_COUNTERS_COUNT; i++) {
		crex_jit_hot_counters[i] = CREX_JIT_COUNTER_INIT;
	}
}

CREX_EXT_API void crex_jit_activate(void)
{
	crex_jit_profile_counter = 0;
	if (JIT_G(on)) {
		if (JIT_G(trigger) == CREX_JIT_ON_HOT_COUNTERS) {
			crex_jit_reset_counters();
		} else if (JIT_G(trigger) == CREX_JIT_ON_HOT_TRACE) {
			crex_jit_reset_counters();
			crex_jit_trace_reset_caches();
		}
	}
}

CREX_EXT_API void crex_jit_deactivate(void)
{
	if (crex_jit_profile_counter && !CG(unclean_shutdown)) {
		crex_class_entry *ce;

		crex_shared_alloc_lock();
		SHM_UNPROTECT();
		crex_jit_unprotect();

		crex_jit_check_funcs(EG(function_table), 0);
		CREX_HASH_MAP_REVERSE_FOREACH_PTR(EG(class_table), ce) {
			if (ce->type == CREX_INTERNAL_CLASS) {
				break;
			}
			crex_jit_check_funcs(&ce->function_table, 1);
		} CREX_HASH_FOREACH_END();

		crex_jit_protect();
		SHM_PROTECT();
		crex_shared_alloc_unlock();
	}

	crex_jit_profile_counter = 0;
}

static void crex_jit_restart_preloaded_op_array(crex_op_array *op_array)
{
	crex_func_info *func_info = CREX_FUNC_INFO(op_array);

	if (!func_info) {
		return;
	}

	if (func_info->flags & CREX_FUNC_JIT_ON_HOT_TRACE) {
		crex_jit_restart_hot_trace_counters(op_array);
	} else if (func_info->flags & CREX_FUNC_JIT_ON_HOT_COUNTERS) {
		crex_jit_restart_hot_counters(op_array);
#if 0
	// TODO: We have to restore handlers for some inner basic-blocks, but we didn't store them ???
	} else if (func_info->flags & (CREX_FUNC_JIT_ON_FIRST_EXEC|CREX_FUNC_JIT_ON_PROF_REQUEST)) {
		crex_op *opline = op_array->opcodes;
		crex_jit_op_array_extension *jit_extension =
			(crex_jit_op_array_extension*)func_info;

		if (!(op_array->fn_flags & CREX_ACC_HAS_TYPE_HINTS)) {
			while (opline->opcode == CREX_RECV || opline->opcode == CREX_RECV_INIT) {
				opline++;
			}
		}
		if (func_info->flags & CREX_FUNC_JIT_ON_FIRST_EXEC) {
			opline->handler = (const void*)crex_jit_runtime_jit_handler;
		} else {
			opline->handler = (const void*)crex_jit_profile_jit_handler;
		}
#endif
	}
	if (op_array->num_dynamic_func_defs) {
		for (uint32_t i = 0; i < op_array->num_dynamic_func_defs; i++) {
			crex_jit_restart_preloaded_op_array(op_array->dynamic_func_defs[i]);
		}
	}
}

static void crex_jit_restart_preloaded_script(crex_persistent_script *script)
{
	crex_class_entry *ce;
	crex_op_array *op_array;

	crex_jit_restart_preloaded_op_array(&script->script.main_op_array);

	CREX_HASH_MAP_FOREACH_PTR(&script->script.function_table, op_array) {
		crex_jit_restart_preloaded_op_array(op_array);
	} CREX_HASH_FOREACH_END();

	CREX_HASH_MAP_FOREACH_PTR(&script->script.class_table, ce) {
		CREX_HASH_MAP_FOREACH_PTR(&ce->function_table, op_array) {
			if (op_array->type == CREX_USER_FUNCTION) {
				crex_jit_restart_preloaded_op_array(op_array);
			}
		} CREX_HASH_FOREACH_END();
	} CREX_HASH_FOREACH_END();
}

CREX_EXT_API void crex_jit_restart(void)
{
	if (dasm_buf) {
		crex_jit_unprotect();

#if CREX_JIT_TARGET_ARM64
		memset(dasm_labels_veneers, 0, sizeof(void*) * CREX_MM_ALIGNED_SIZE_EX(crex_lb_MAX, DASM_ALIGNMENT));
#endif

		/* restore JIT buffer pos */
		dasm_ptr[0] = dasm_ptr[1];

		crex_jit_trace_restart();

		if (ZCSG(preload_script)) {
			crex_jit_restart_preloaded_script(ZCSG(preload_script));
			if (ZCSG(saved_scripts)) {
				crex_persistent_script **p = ZCSG(saved_scripts);

				while (*p) {
					crex_jit_restart_preloaded_script(*p);
					p++;
				}
			}
		}

		crex_jit_protect();

#ifdef HAVE_DISASM
		if (JIT_G(debug) & (CREX_JIT_DEBUG_ASM|CREX_JIT_DEBUG_ASM_STUBS)) {
			crex_jit_disasm_shutdown();
			crex_jit_disasm_init();
		}
#endif
	}
}

#endif /* HAVE_JIT */
