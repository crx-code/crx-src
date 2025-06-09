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
   |          Xinchen Hui <laruence@crx.net>                              |
   |          Hao Sun <hao.sun@arm.com>                                   |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_JIT_INTERNAL_H
#define CREX_JIT_INTERNAL_H

#include "crex_bitset.h"

/* Register Set */
#define CREX_REGSET_EMPTY 0

#define CREX_REGSET_IS_EMPTY(regset) \
	(regset == CREX_REGSET_EMPTY)

#define CREX_REGSET_IS_SINGLETON(regset) \
	(regset && !(regset & (regset - 1)))

#if (!CREX_REGSET_64BIT)
#define CREX_REGSET(reg) \
	(1u << (reg))
#else
#define CREX_REGSET(reg) \
	(1ull << (reg))
#endif

#if (!CREX_REGSET_64BIT)
#define CREX_REGSET_INTERVAL(reg1, reg2) \
	(((1u << ((reg2) - (reg1) + 1)) - 1) << (reg1))
#else
#define CREX_REGSET_INTERVAL(reg1, reg2) \
	(((1ull << ((reg2) - (reg1) + 1)) - 1) << (reg1))
#endif

#define CREX_REGSET_IN(regset, reg) \
	(((regset) & CREX_REGSET(reg)) != 0)

#define CREX_REGSET_INCL(regset, reg) \
	(regset) |= CREX_REGSET(reg)

#define CREX_REGSET_EXCL(regset, reg) \
	(regset) &= ~CREX_REGSET(reg)

#define CREX_REGSET_UNION(set1, set2) \
	((set1) | (set2))

#define CREX_REGSET_INTERSECTION(set1, set2) \
	((set1) & (set2))

#define CREX_REGSET_DIFFERENCE(set1, set2) \
	((set1) & ~(set2))

#if !defined(_WIN32)
# if (!CREX_REGSET_64BIT)
#  define CREX_REGSET_FIRST(set) ((crex_reg)__builtin_ctz(set))
#  define CREX_REGSET_LAST(set)  ((crex_reg)(__builtin_clz(set)^31))
# else
#  define CREX_REGSET_FIRST(set) ((crex_reg)__builtin_ctzll(set))
#  define CREX_REGSET_LAST(set)  ((crex_reg)(__builtin_clzll(set)^63))
# endif
#else
# include <intrin.h>
uint32_t __inline __crex_jit_ctz(uint32_t value) {
	DWORD trailing_zero = 0;
	if (_BitScanForward(&trailing_zero, value)) {
		return trailing_zero;
	}
	return 32;
}
uint32_t __inline __crex_jit_clz(uint32_t value) {
	DWORD leading_zero = 0;
	if (_BitScanReverse(&leading_zero, value)) {
		return 31 - leading_zero;
	}
	return 32;
}
# define CREX_REGSET_FIRST(set) ((crex_reg)__crex_jit_ctz(set))
# define CREX_REGSET_LAST(set)  ((crex_reg)(__crex_jit_clz(set)^31))
#endif

#define CREX_REGSET_FOREACH(set, reg) \
	do { \
		crex_regset _tmp = (set); \
		while (!CREX_REGSET_IS_EMPTY(_tmp)) { \
			crex_reg _reg = CREX_REGSET_FIRST(_tmp); \
			CREX_REGSET_EXCL(_tmp, _reg); \
			reg = _reg; \

#define CREX_REGSET_FOREACH_END() \
		} \
	} while (0)

/* Register Names */
extern const char *crex_reg_name[];

/* Address Encoding */
typedef uintptr_t crex_jit_addr;

#define IS_CONST_ZVAL                  0
#define IS_MEM_ZVAL                    1
#define IS_REG                         2

#define _CREX_ADDR_MODE_MASK         0x3
#define _CREX_ADDR_REG_SHIFT           2
#define _CREX_ADDR_REG_MASK         0x3f /* no more than 64 registers */
#define _CREX_ADDR_OFFSET_SHIFT        8
#define _CREX_ADDR_REG_STORE_BIT       8
#define _CREX_ADDR_REG_LOAD_BIT        9
#define _CREX_ADDR_REG_LAST_USE_BIT   10

#define CREX_ADDR_CONST_ZVAL(zv) \
	(((crex_jit_addr)(uintptr_t)(zv)) | IS_CONST_ZVAL)
#define CREX_ADDR_MEM_ZVAL(reg, offset) \
	((((crex_jit_addr)(uintptr_t)(offset)) << _CREX_ADDR_OFFSET_SHIFT) | \
	(((crex_jit_addr)(uintptr_t)(reg)) << _CREX_ADDR_REG_SHIFT) | \
	IS_MEM_ZVAL)
#define CREX_ADDR_REG(reg) \
	((((crex_jit_addr)(uintptr_t)(reg)) << _CREX_ADDR_REG_SHIFT) | \
	IS_REG)

#define C_MODE(addr)     (((addr) & _CREX_ADDR_MODE_MASK))
#define C_ZV(addr)       ((zval*)(addr))
#define C_OFFSET(addr)   ((uint32_t)((addr)>>_CREX_ADDR_OFFSET_SHIFT))
#define C_REG(addr)      ((crex_reg)(((addr)>>_CREX_ADDR_REG_SHIFT) & _CREX_ADDR_REG_MASK))
#define C_STORE(addr)    ((crex_reg)(((addr)>>_CREX_ADDR_REG_STORE_BIT) & 1))
#define C_LOAD(addr)     ((crex_reg)(((addr)>>_CREX_ADDR_REG_LOAD_BIT) & 1))
#define C_LAST_USE(addr) ((crex_reg)(((addr)>>_CREX_ADDR_REG_LAST_USE_BIT) & 1))

#define OP_REG_EX(reg, store, load, last_use) \
	((reg) | \
	 ((store) ? (1 << (_CREX_ADDR_REG_STORE_BIT-_CREX_ADDR_REG_SHIFT)) : 0) | \
	 ((load) ? (1 << (_CREX_ADDR_REG_LOAD_BIT-_CREX_ADDR_REG_SHIFT)) : 0) | \
	 ((last_use) ? (1 << (_CREX_ADDR_REG_LAST_USE_BIT-_CREX_ADDR_REG_SHIFT)) : 0) \
	)

#define OP_REG(ssa_op, op) \
	(ra && ssa_op->op >= 0 && ra[ssa_op->op] ? \
		OP_REG_EX(ra[ssa_op->op]->reg, \
			(ra[ssa_op->op]->flags & ZREG_STORE), \
			(ra[ssa_op->op]->flags & ZREG_LOAD), \
			crex_ival_is_last_use(ra[ssa_op->op], ssa_op - ssa->ops) \
		) : ZREG_NONE)

static crex_always_inline crex_jit_addr _crex_jit_decode_op(uint8_t op_type, znode_op op, const crex_op *opline, crex_reg reg)
{
	if (op_type == IS_CONST) {
#if CREX_USE_ABS_CONST_ADDR
		return CREX_ADDR_CONST_ZVAL(op.zv);
#else
		return CREX_ADDR_CONST_ZVAL(RT_CONSTANT(opline, op));
#endif
	} else {
		CREX_ASSERT(op_type & (IS_CV|IS_TMP_VAR|IS_VAR));
		if (reg != ZREG_NONE) {
			return CREX_ADDR_REG(reg);
		} else {
			return CREX_ADDR_MEM_ZVAL(ZREG_FP, op.var);
		}
	}
}

#define OP_ADDR(opline, type, op) \
	_crex_jit_decode_op((opline)->type, (opline)->op, opline, ZREG_NONE)

#define OP1_ADDR() \
	OP_ADDR(opline, op1_type, op1)
#define OP2_ADDR() \
	OP_ADDR(opline, op2_type, op2)
#define RES_ADDR() \
	OP_ADDR(opline, result_type, result)
#define OP1_DATA_ADDR() \
	OP_ADDR(opline + 1, op1_type, op1)

#define OP_REG_ADDR(opline, type, _op, _ssa_op) \
	_crex_jit_decode_op((opline)->type, (opline)->_op, opline, \
		OP_REG(ssa_op, _ssa_op))

#define OP1_REG_ADDR() \
	OP_REG_ADDR(opline, op1_type, op1, op1_use)
#define OP2_REG_ADDR() \
	OP_REG_ADDR(opline, op2_type, op2, op2_use)
#define RES_REG_ADDR() \
	OP_REG_ADDR(opline, result_type, result, result_def)
#define OP1_DATA_REG_ADDR() \
	OP_REG_ADDR(opline + 1, op1_type, op1, op1_use)

#define OP1_DEF_REG_ADDR() \
	OP_REG_ADDR(opline, op1_type, op1, op1_def)
#define OP2_DEF_REG_ADDR() \
	OP_REG_ADDR(opline, op2_type, op2, op2_def)
#define RES_USE_REG_ADDR() \
	OP_REG_ADDR(opline, result_type, result, result_use)
#define OP1_DATA_DEF_REG_ADDR() \
	OP_REG_ADDR(opline + 1, op1_type, op1, op1_def)

static crex_always_inline bool crex_jit_same_addr(crex_jit_addr addr1, crex_jit_addr addr2)
{
	if (addr1 == addr2) {
		return 1;
	} else if (C_MODE(addr1) == IS_REG && C_MODE(addr2) == IS_REG) {
		return C_REG(addr1) == C_REG(addr2);
	}
	return 0;
}

typedef struct _crex_jit_op_array_extension {
	crex_func_info func_info;
	const void *orig_handler;
} crex_jit_op_array_extension;

/* Profiler */
extern crex_ulong crex_jit_profile_counter;
extern int crex_jit_profile_counter_rid;

#define CREX_COUNTER_INFO(op_array) \
	CREX_OP_ARRAY_EXTENSION(op_array, crex_jit_profile_counter_rid)

/* Hot Counters */

#define CREX_HOT_COUNTERS_COUNT 128

extern int16_t crex_jit_hot_counters[CREX_HOT_COUNTERS_COUNT];

static crex_always_inline crex_long crex_jit_hash(const void *ptr)
{
	uintptr_t x;

	x = (uintptr_t)ptr >> 3;
#if SIZEOF_SIZE_T == 4
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = (x >> 16) ^ x;
#elif SIZEOF_SIZE_T == 8
	x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
	x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
	x = x ^ (x >> 31);
#endif
	return x;
}

void CREX_FASTCALL crex_jit_hot_func(crex_execute_data *execute_data, const crex_op *opline);

typedef struct _crex_jit_op_array_hot_extension {
	crex_func_info func_info;
	int16_t    *counter;
	const void *orig_handlers[1];
} crex_jit_op_array_hot_extension;

#define crex_jit_op_array_hash(op_array) \
	crex_jit_hash((op_array)->opcodes)

extern const crex_op *crex_jit_halt_op;

#ifdef HAVE_GCC_GLOBAL_REGS
# define EXECUTE_DATA_D                       void
# define EXECUTE_DATA_C
# define EXECUTE_DATA_DC
# define EXECUTE_DATA_CC
# define OPLINE_D                             void
# define OPLINE_C
# define OPLINE_DC
# define OPLINE_CC
# define CREX_OPCODE_HANDLER_RET              void
# define CREX_OPCODE_HANDLER_ARGS             EXECUTE_DATA_D
# define CREX_OPCODE_HANDLER_ARGS_PASSTHRU
# define CREX_OPCODE_HANDLER_ARGS_DC
# define CREX_OPCODE_HANDLER_ARGS_PASSTHRU_CC
# define CREX_OPCODE_RETURN()                 return
# define CREX_OPCODE_TAIL_CALL(handler)       do { \
		handler(CREX_OPCODE_HANDLER_ARGS_PASSTHRU); \
		return; \
	} while(0)
# define CREX_OPCODE_TAIL_CALL_EX(handler, arg) do { \
		handler(arg CREX_OPCODE_HANDLER_ARGS_PASSTHRU_CC); \
		return; \
	} while(0)
#else
# define EXECUTE_DATA_D                       crex_execute_data* execute_data
# define EXECUTE_DATA_C                       execute_data
# define EXECUTE_DATA_DC                      , EXECUTE_DATA_D
# define EXECUTE_DATA_CC                      , EXECUTE_DATA_C
# define OPLINE_D                             const crex_op* opline
# define OPLINE_C                             opline
# define OPLINE_DC                            , OPLINE_D
# define OPLINE_CC                            , OPLINE_C
# define CREX_OPCODE_HANDLER_RET              int
# define CREX_OPCODE_HANDLER_ARGS             EXECUTE_DATA_D
# define CREX_OPCODE_HANDLER_ARGS_PASSTHRU    EXECUTE_DATA_C
# define CREX_OPCODE_HANDLER_ARGS_DC          EXECUTE_DATA_DC
# define CREX_OPCODE_HANDLER_ARGS_PASSTHRU_CC EXECUTE_DATA_CC
# define CREX_OPCODE_RETURN()                 return 0
# define CREX_OPCODE_TAIL_CALL(handler)       do { \
		return handler(CREX_OPCODE_HANDLER_ARGS_PASSTHRU); \
	} while(0)
# define CREX_OPCODE_TAIL_CALL_EX(handler, arg) do { \
		return handler(arg CREX_OPCODE_HANDLER_ARGS_PASSTHRU_CC); \
	} while(0)
#endif

/* VM handlers */
typedef CREX_OPCODE_HANDLER_RET (CREX_FASTCALL *crex_vm_opcode_handler_t)(CREX_OPCODE_HANDLER_ARGS);

/* VM helpers */
CREX_OPCODE_HANDLER_RET CREX_FASTCALL crex_jit_leave_nested_func_helper(uint32_t call_info EXECUTE_DATA_DC);
CREX_OPCODE_HANDLER_RET CREX_FASTCALL crex_jit_leave_top_func_helper(uint32_t call_info EXECUTE_DATA_DC);
CREX_OPCODE_HANDLER_RET CREX_FASTCALL crex_jit_leave_func_helper(EXECUTE_DATA_D);

CREX_OPCODE_HANDLER_RET CREX_FASTCALL crex_jit_profile_helper(CREX_OPCODE_HANDLER_ARGS);

CREX_OPCODE_HANDLER_RET CREX_FASTCALL crex_jit_func_counter_helper(CREX_OPCODE_HANDLER_ARGS);
CREX_OPCODE_HANDLER_RET CREX_FASTCALL crex_jit_loop_counter_helper(CREX_OPCODE_HANDLER_ARGS);

void CREX_FASTCALL crex_jit_copy_extra_args_helper(EXECUTE_DATA_D);
bool CREX_FASTCALL crex_jit_deprecated_helper(OPLINE_D);

crex_constant* CREX_FASTCALL crex_jit_get_constant(const zval *key, uint32_t flags);
crex_constant* CREX_FASTCALL crex_jit_check_constant(const zval *key);

/* Tracer */
#define crex_jit_opline_hash(opline) \
	crex_jit_hash(opline)

#define CREX_JIT_TRACE_STOP(_) \
	_(LOOP,              "loop") \
	_(RECURSIVE_CALL,    "recursive call") \
	_(RECURSIVE_RET,     "recursive return") \
	_(RETURN,            "return") \
	_(INTERPRETER,       "exit to VM interpreter") \
	_(LINK,              "link to another trace") \
	/* compilation and linking successful */ \
	_(COMPILED,          "compiled") \
	_(ALREADY_DONE,      "already prcessed") \
	/* failures */ \
	_(ERROR,             "error")                          /* not used */ \
	_(NOT_SUPPORTED,     "not supported instructions") \
	_(EXCEPTION,         "exception") \
	_(TOO_LONG,          "trace too long") \
	_(TOO_DEEP,          "trace too deep") \
	_(TOO_DEEP_RET,      "trace too deep return") \
	_(DEEP_RECURSION,    "deep recursion") \
	_(LOOP_UNROLL,       "loop unroll limit reached") \
	_(LOOP_EXIT,         "exit from loop") \
	_(RECURSION_EXIT,    "return from recursive function") \
	_(BLACK_LIST,        "trace blacklisted") \
	_(INNER_LOOP,        "inner loop")                     /* trace it */ \
	_(COMPILED_LOOP,     "compiled loop") \
	_(TRAMPOLINE,        "trampoline call") \
	_(BAD_FUNC,          "bad function call") \
	_(COMPILER_ERROR,    "JIT compilation error") \
	/* no recoverable error (blacklist immediately) */ \
	_(NO_SHM,            "insufficient shared memory") \
	_(TOO_MANY_TRACES,   "too many traces") \
	_(TOO_MANY_CHILDREN, "too many side traces") \
	_(TOO_MANY_EXITS,    "too many side exits") \

#define CREX_JIT_TRACE_STOP_NAME(name, description) \
	CREX_JIT_TRACE_STOP_ ## name,

typedef enum _crex_jit_trace_stop {
	CREX_JIT_TRACE_STOP(CREX_JIT_TRACE_STOP_NAME)
	CREX_JIT_TRACE_HALT = 0x40
} crex_jit_trace_stop;

#define CREX_JIT_TRACE_STOP_OK(ret) \
	(ret < CREX_JIT_TRACE_STOP_COMPILED)

#define CREX_JIT_TRACE_STOP_DONE(ret) \
	(ret < CREX_JIT_TRACE_STOP_ERROR)

#define CREX_JIT_TRACE_STOP_REPEAT(ret) \
	(ret == CREX_JIT_TRACE_STOP_INNER_LOOP)

#define CREX_JIT_TRACE_STOP_MAY_RECOVER(ret) \
	(ret <= CREX_JIT_TRACE_STOP_COMPILER_ERROR)

#define CREX_JIT_TRACE_START_MASK      0xf

#define CREX_JIT_TRACE_START_LOOP   (1<<0)
#define CREX_JIT_TRACE_START_ENTER  (1<<1)
#define CREX_JIT_TRACE_START_RETURN (1<<2)
#define CREX_JIT_TRACE_START_SIDE   (1<<3) /* used for side traces */

#define CREX_JIT_TRACE_JITED        (1<<4)
#define CREX_JIT_TRACE_BLACKLISTED  (1<<5)
#define CREX_JIT_TRACE_UNSUPPORTED  (1<<6)

#define CREX_JIT_TRACE_SUPPORTED    0

#define CREX_JIT_EXIT_JITED         (1<<0)
#define CREX_JIT_EXIT_BLACKLISTED   (1<<1)
#define CREX_JIT_EXIT_TO_VM         (1<<2) /* exit to VM without attempt to create a side trace */
#define CREX_JIT_EXIT_RESTORE_CALL  (1<<3) /* deoptimizer should restore EX(call) chain */
#define CREX_JIT_EXIT_POLYMORPHISM  (1<<4) /* exit because of polymorphic call */
#define CREX_JIT_EXIT_FREE_OP1      (1<<5)
#define CREX_JIT_EXIT_FREE_OP2      (1<<6)
#define CREX_JIT_EXIT_PACKED_GUARD  (1<<7)
#define CREX_JIT_EXIT_CLOSURE_CALL  (1<<8) /* exit because of polymorphic INIT_DYNAMIC_CALL call */
#define CREX_JIT_EXIT_METHOD_CALL   (1<<9) /* exit because of polymorphic INIT_METHOD_CALL call */
#define CREX_JIT_EXIT_INVALIDATE    (1<<10) /* invalidate current trace */

typedef union _crex_op_trace_info {
	crex_op dummy; /* the size of this structure must be the same as crex_op */
	struct {
		const void *orig_handler;
		const void *call_handler;
		int16_t    *counter;
		uint8_t     trace_flags;
	};
} crex_op_trace_info;

typedef struct _crex_jit_op_array_trace_extension {
	crex_func_info func_info;
	const crex_op_array *op_array;
	size_t offset; /* offset from "crex_op" to corresponding "op_info" */
	crex_op_trace_info trace_info[1];
} crex_jit_op_array_trace_extension;

#define CREX_OP_TRACE_INFO(opline, offset) \
	((crex_op_trace_info*)(((char*)opline) + offset))

/* Recorder */
typedef enum _crex_jit_trace_op {
	CREX_JIT_TRACE_VM,
	CREX_JIT_TRACE_OP1_TYPE,
	CREX_JIT_TRACE_OP2_TYPE,
	CREX_JIT_TRACE_VAL_INFO,
	CREX_JIT_TRACE_INIT_CALL,
	CREX_JIT_TRACE_DO_ICALL,
	CREX_JIT_TRACE_ENTER,
	CREX_JIT_TRACE_BACK,
	CREX_JIT_TRACE_END,
	CREX_JIT_TRACE_START,
} crex_jit_trace_op;

#define IS_UNKNOWN 255 /* may be used for crex_jit_trace_rec.op?_type */
#define IS_TRACE_PACKED    (1<<4)
#define IS_TRACE_REFERENCE (1<<5)
#define IS_TRACE_INDIRECT  (1<<6)

#define IS_TRACE_TYPE_MASK 0xf

#define CREX_JIT_TRACE_FAKE_INIT_CALL    0x00000100
#define CREX_JIT_TRACE_RETURN_VALUE_USED 0x00000100

#define CREX_JIT_TRACE_MAX_SSA_VAR       0x7ffffe
#define CREX_JIT_TRACE_SSA_VAR_SHIFT     9

#define CREX_JIT_TRACE_FAKE_LEVEL_MASK   0xffff0000
#define CREX_JIT_TRACE_FAKE_LEVEL_SHIFT  16

#define CREX_JIT_TRACE_FAKE_LEVEL(info) \
	(((info) & CREX_JIT_TRACE_FAKE_LEVEL_MASK) >> CREX_JIT_TRACE_FAKE_LEVEL_SHIFT)

#define CREX_JIT_TRACE_FAKE_INFO(level) \
	(((level) << CREX_JIT_TRACE_FAKE_LEVEL_SHIFT) | CREX_JIT_TRACE_FAKE_INIT_CALL)

#define CREX_JIT_TRACE_SET_FIRST_SSA_VAR(_info, var) do { \
		_info |= (var << CREX_JIT_TRACE_SSA_VAR_SHIFT); \
	} while (0)
#define CREX_JIT_TRACE_GET_FIRST_SSA_VAR(_info) \
	(_info >> CREX_JIT_TRACE_SSA_VAR_SHIFT)

struct _crex_jit_trace_rec {
	union {
		struct { CREX_ENDIAN_LOHI(
			uint8_t   op,    /* crex_jit_trace_op */
			union {
				struct {
					uint8_t op1_type;/* recorded zval op1_type for CREX_JIT_TRACE_VM */
					uint8_t op2_type;/* recorded zval op2_type for CREX_JIT_TRACE_VM */
					uint8_t op3_type;/* recorded zval for op_data.op1_type for CREX_JIT_TRACE_VM */
				};
				struct {
					uint8_t  start;  /* CREX_JIT_TRACE_START_MASK for CREX_JIT_TRACE_START/END */
					uint8_t  stop;   /* crex_jit_trace_stop for CREX_JIT_TRACE_START/END */
					uint8_t  level;  /* recursive return level for CREX_JIT_TRACE_START */
				};
			})
		};
		uint32_t last;
		uint32_t info; /* "first_ssa_var" for CREX_JIT_TRACE_ENTER and CREX_JIT_TRACE_BACK,
		                * "return_value_used" for CREX_JIT_TRACE_ENTER,
		                * "fake" for CREX_JIT_TRACE_INIT_CALL */
	};
	union {
		const void             *ptr;
		const crex_function    *func;
		const crex_op_array    *op_array;
		const crex_op          *opline;
		const crex_class_entry *ce;
	};
};

#define CREX_JIT_TRACE_START_REC_SIZE 2

typedef struct _crex_jit_trace_exit_info {
	const crex_op       *opline;     /* opline where VM should continue execution */
	const crex_op_array *op_array;
	uint32_t             flags;      /* set of CREX_JIT_EXIT_... */
	uint32_t             stack_size;
	uint32_t             stack_offset;
} crex_jit_trace_exit_info;

typedef union _crex_jit_trace_stack {
	int32_t      ssa_var;
	uint32_t     info;
	struct {
		uint8_t type;     /* variable type (for type inference) */
		uint8_t mem_type; /* stack slot type  (for eliminate dead type store) */
		int8_t  reg;
		uint8_t flags;
	};
} crex_jit_trace_stack;

#define STACK_VAR(_stack, _slot) \
	(_stack)[_slot].ssa_var
#define STACK_INFO(_stack, _slot) \
	(_stack)[_slot].info
#define STACK_TYPE(_stack, _slot) \
	(_stack)[_slot].type
#define STACK_MEM_TYPE(_stack, _slot) \
	(_stack)[_slot].mem_type
#define STACK_REG(_stack, _slot) \
	(_stack)[_slot].reg
#define STACK_FLAGS(_stack, _slot) \
	(_stack)[_slot].flags
#define SET_STACK_VAR(_stack, _slot, _ssa_var) do { \
		(_stack)[_slot].ssa_var = _ssa_var; \
	} while (0)
#define SET_STACK_INFO(_stack, _slot, _info) do { \
		(_stack)[_slot].info = _info; \
	} while (0)
#define SET_STACK_TYPE(_stack, _slot, _type, _set_mem_type) do { \
		uint8_t __type = (_type); \
		(_stack)[_slot].type = __type; \
		if (_set_mem_type) { \
			(_stack)[_slot].mem_type = __type; \
		} \
		(_stack)[_slot].reg = ZREG_NONE; \
		(_stack)[_slot].flags = 0; \
	} while (0)
#define SET_STACK_REG(_stack, _slot, _reg) do { \
		(_stack)[_slot].reg = _reg; \
		(_stack)[_slot].flags = 0; \
	} while (0)
#define SET_STACK_REG_EX(_stack, _slot, _reg, _flags) do { \
		(_stack)[_slot].reg = _reg; \
		(_stack)[_slot].flags = _flags; \
	} while (0)
#define RESET_STACK_MEM_TYPE(_stack, _slot) do { \
		(_stack)[_slot].mem_type = IS_UNKNOWN; \
	} while (0)

/* trace info flags */
#define CREX_JIT_TRACE_CHECK_INTERRUPT (1<<0)
#define CREX_JIT_TRACE_LOOP            (1<<1)
#define CREX_JIT_TRACE_USES_INITIAL_IP (1<<2)

typedef struct _crex_jit_trace_info {
	uint32_t                  id;            /* trace id */
	uint32_t                  root;          /* root trace id or self id for root traces */
	uint32_t                  parent;        /* parent trace id or 0 for root traces */
	uint32_t                  link;          /* link trace id or self id for loop) */
	uint32_t                  exit_count;    /* number of side exits */
	uint32_t                  child_count;   /* number of side traces for root traces */
	uint32_t                  code_size;     /* size of native code */
	uint32_t                  exit_counters; /* offset in exit counters array */
	uint32_t                  stack_map_size;
	uint32_t                  flags;         /* See CREX_JIT_TRACE_... defines above */
	uint32_t                  polymorphism;  /* Counter of polymorphic calls */
	uint32_t                  jmp_table_size;/* number of jmp_table slots */
	const crex_op_array      *op_array;      /* function */
	const crex_op            *opline;        /* first opline */
	const void               *code_start;    /* address of native code */
	crex_jit_trace_exit_info *exit_info;     /* info about side exits */
	crex_jit_trace_stack     *stack_map;
	//uint32_t    loop_offset;
} crex_jit_trace_info;

struct _crex_jit_trace_stack_frame {
	crex_jit_trace_stack_frame *call;
	crex_jit_trace_stack_frame *prev;
	const crex_function        *func;
	const crex_op              *call_opline;
	crex_class_entry           *ce;          /* $this */
	uint32_t                    call_level;
	uint32_t                    _info;
	int                         used_stack;
	int                         old_checked_stack;
	int                         old_peek_checked_stack;
	crex_jit_trace_stack        stack[1];
};

#define TRACE_FRAME_SHIFT_NUM_ARGS            16
#define TRACE_FRAME_MAX_NUM_ARGS              32767

#define TRACE_FRAME_MASK_NUM_ARGS             0xffff0000
#define TRACE_FRAME_MASK_NESTED               0x00000001
#define TRACE_FRAME_MASK_LAST_SEND_BY_REF     0x00000002
#define TRACE_FRAME_MASK_LAST_SEND_BY_VAL     0x00000004
#define TRACE_FRAME_MASK_RETURN_VALUE_USED    0x00000008
#define TRACE_FRAME_MASK_RETURN_VALUE_UNUSED  0x00000010
#define TRACE_FRAME_MASK_THIS_CHECKED         0x00000020
#define TRACE_FRAME_MASK_UNKNOWN_RETURN       0x00000040
#define TRACE_FRAME_MASK_NO_NEED_RELEASE_THIS 0x00000080
#define TRACE_FRAME_MASK_THIS_CLASS_CHECKED   0x00000100
#define TRACE_FRAME_MASK_CLOSURE_CALL         0x00000200
#define TRACE_FRAME_MASK_ALWAYS_RELEASE_THIS  0x00000400


#define TRACE_FRAME_INIT(frame, _func, _flags, num_args) do { \
		crex_jit_trace_stack_frame *_frame = (frame); \
		_frame->call = NULL; \
		_frame->prev = NULL; \
		_frame->func = (const crex_function*)_func; \
		_frame->call_opline = NULL; \
		_frame->call_level = 0; \
		_frame->_info = (((uint32_t)(num_args)) << TRACE_FRAME_SHIFT_NUM_ARGS) & TRACE_FRAME_MASK_NUM_ARGS; \
		_frame->_info |= _flags; \
	} while (0)

#define TRACE_FRAME_RETURN_SSA_VAR(frame) \
	((int)(frame)->_info)
#define TRACE_FRAME_NUM_ARGS(frame) \
	((int)((frame)->_info) >> TRACE_FRAME_SHIFT_NUM_ARGS)
#define TRACE_FRAME_IS_NESTED(frame) \
	((frame)->_info & TRACE_FRAME_MASK_NESTED)
#define TRACE_FRAME_IS_LAST_SEND_BY_REF(frame) \
	((frame)->_info & TRACE_FRAME_MASK_LAST_SEND_BY_REF)
#define TRACE_FRAME_IS_LAST_SEND_BY_VAL(frame) \
	((frame)->_info & TRACE_FRAME_MASK_LAST_SEND_BY_VAL)
#define TRACE_FRAME_IS_RETURN_VALUE_USED(frame) \
	((frame)->_info & TRACE_FRAME_MASK_RETURN_VALUE_USED)
#define TRACE_FRAME_IS_RETURN_VALUE_UNUSED(frame) \
	((frame)->_info & TRACE_FRAME_MASK_RETURN_VALUE_UNUSED)
#define TRACE_FRAME_IS_THIS_CHECKED(frame) \
	((frame)->_info & TRACE_FRAME_MASK_THIS_CHECKED)
#define TRACE_FRAME_IS_UNKNOWN_RETURN(frame) \
	((frame)->_info & TRACE_FRAME_MASK_UNKNOWN_RETURN)
#define TRACE_FRAME_NO_NEED_RELEASE_THIS(frame) \
	((frame)->_info & TRACE_FRAME_MASK_NO_NEED_RELEASE_THIS)
#define TRACE_FRAME_IS_THIS_CLASS_CHECKED(frame) \
	((frame)->_info & TRACE_FRAME_MASK_THIS_CLASS_CHECKED)
#define TRACE_FRAME_IS_CLOSURE_CALL(frame) \
	((frame)->_info & TRACE_FRAME_MASK_CLOSURE_CALL)
#define TRACE_FRAME_ALWAYS_RELEASE_THIS(frame) \
	((frame)->_info & TRACE_FRAME_MASK_ALWAYS_RELEASE_THIS)

#define TRACE_FRAME_SET_UNKNOWN_NUM_ARGS(frame) do { \
		(frame)->_info |= (0xffffu << TRACE_FRAME_SHIFT_NUM_ARGS); \
	} while (0)
#define TRACE_FRAME_SET_RETURN_SSA_VAR(frame, var) do { \
		(frame)->_info = var; \
	} while (0)
#define TRACE_FRAME_SET_LAST_SEND_BY_REF(frame) do { \
		(frame)->_info |= TRACE_FRAME_MASK_LAST_SEND_BY_REF; \
		(frame)->_info &= ~TRACE_FRAME_MASK_LAST_SEND_BY_VAL; \
	} while (0)
#define TRACE_FRAME_SET_LAST_SEND_BY_VAL(frame) do { \
		(frame)->_info |= TRACE_FRAME_MASK_LAST_SEND_BY_VAL; \
		(frame)->_info &= ~TRACE_FRAME_MASK_LAST_SEND_BY_REF; \
	} while (0)
#define TRACE_FRAME_SET_LAST_SEND_UNKNOWN(frame) do { \
		(frame)->_info &= ~TRACE_FRAME_MASK_LAST_SEND_BY_VAL; \
		(frame)->_info &= ~TRACE_FRAME_MASK_LAST_SEND_BY_REF; \
	} while (0)
#define TRACE_FRAME_SET_RETURN_VALUE_USED(frame) do { \
		(frame)->_info |= TRACE_FRAME_MASK_RETURN_VALUE_USED; \
		(frame)->_info &= ~TRACE_FRAME_MASK_RETURN_VALUE_UNUSED; \
	} while (0)
#define TRACE_FRAME_SET_RETURN_VALUE_UNUSED(frame) do { \
		(frame)->_info |= TRACE_FRAME_MASK_RETURN_VALUE_UNUSED; \
		(frame)->_info &= ~TRACE_FRAME_MASK_RETURN_VALUE_USED; \
	} while (0)
#define TRACE_FRAME_SET_THIS_CHECKED(frame) do { \
		(frame)->_info |= TRACE_FRAME_MASK_THIS_CHECKED; \
	} while (0)
#define TRACE_FRAME_SET_NO_NEED_RELEASE_THIS(frame) do { \
		(frame)->_info |= TRACE_FRAME_MASK_NO_NEED_RELEASE_THIS; \
	} while (0)
#define TRACE_FRAME_SET_THIS_CLASS_CHECKED(frame) do { \
		(frame)->_info |= TRACE_FRAME_MASK_THIS_CLASS_CHECKED; \
	} while (0)
#define TRACE_FRAME_SET_CLOSURE_CALL(frame) do { \
		(frame)->_info |= TRACE_FRAME_MASK_CLOSURE_CALL; \
	} while (0)
#define TRACE_FRAME_SET_ALWAYS_RELEASE_THIS(frame) do { \
		(frame)->_info |= TRACE_FRAME_MASK_ALWAYS_RELEASE_THIS; \
	} while (0)

CREX_OPCODE_HANDLER_RET CREX_FASTCALL crex_jit_func_trace_helper(CREX_OPCODE_HANDLER_ARGS);
CREX_OPCODE_HANDLER_RET CREX_FASTCALL crex_jit_ret_trace_helper(CREX_OPCODE_HANDLER_ARGS);
CREX_OPCODE_HANDLER_RET CREX_FASTCALL crex_jit_loop_trace_helper(CREX_OPCODE_HANDLER_ARGS);

int CREX_FASTCALL crex_jit_trace_hot_root(crex_execute_data *execute_data, const crex_op *opline);
int CREX_FASTCALL crex_jit_trace_exit(uint32_t exit_num, crex_jit_registers_buf *regs);
crex_jit_trace_stop CREX_FASTCALL crex_jit_trace_execute(crex_execute_data *execute_data, const crex_op *opline, crex_jit_trace_rec *trace_buffer, uint8_t start, uint32_t is_megamorphc);

static crex_always_inline const crex_op* crex_jit_trace_get_exit_opline(crex_jit_trace_rec *trace, const crex_op *opline, bool *exit_if_true)
{
	if (trace->op == CREX_JIT_TRACE_VM || trace->op == CREX_JIT_TRACE_END) {
		if (trace->opline == opline + 1) {
			/* not taken branch */
			*exit_if_true = opline->opcode == CREX_JMPNZ;
			return OP_JMP_ADDR(opline, opline->op2);
		} else if (trace->opline == OP_JMP_ADDR(opline, opline->op2)) {
			/* taken branch */
			*exit_if_true = opline->opcode == CREX_JMPZ;
			return opline + 1;
		} else {
			CREX_UNREACHABLE();
		}
	} else  {
		CREX_UNREACHABLE();
	}
	*exit_if_true = 0;
	return NULL;
}

static inline bool crex_jit_may_be_modified(const crex_function *func, const crex_op_array *called_from)
{
	if (func->type == CREX_INTERNAL_FUNCTION) {
#ifdef _WIN32
		/* ASLR */
		return 1;
#else
		return 0;
#endif
	} else if (func->type == CREX_USER_FUNCTION) {
		if (func->common.fn_flags & CREX_ACC_PRELOADED) {
			return 0;
		}
		if (func->op_array.filename == called_from->filename && !func->op_array.scope) {
			return 0;
		}
	}
	return 1;
}

static crex_always_inline bool crex_jit_may_be_polymorphic_call(const crex_op *opline)
{
	if (opline->opcode == CREX_INIT_FCALL
	 || opline->opcode == CREX_INIT_FCALL_BY_NAME
	 || opline->opcode == CREX_INIT_NS_FCALL_BY_NAME) {
		return 0;
	} else if (opline->opcode == CREX_INIT_METHOD_CALL
     || opline->opcode == CREX_INIT_DYNAMIC_CALL) {
		return 1;
	} else if (opline->opcode == CREX_INIT_STATIC_METHOD_CALL) {
		return (opline->op1_type != IS_CONST || opline->op2_type != IS_CONST);
	} else if (opline->opcode == CREX_INIT_USER_CALL) {
		return (opline->op2_type != IS_CONST);
	} else if (opline->opcode == CREX_NEW) {
		return (opline->op1_type != IS_CONST);
	} else {
		CREX_UNREACHABLE();
		return 0;
	}
}

/* Instruction cache flush */
#ifndef JIT_CACHE_FLUSH
#  if CREX_JIT_TARGET_ARM64
#    if ((defined(__GNUC__) && CREX_GCC_VERSION >= 4003) || __has_builtin(__builtin___clear_cache))
#      define JIT_CACHE_FLUSH(from, to) __builtin___clear_cache((char*)(from), (char*)(to))
#    else
#      error "Missing builtin to flush instruction cache for AArch64"
#    endif
#  else /* Not required to implement on archs with unified caches */
#    define JIT_CACHE_FLUSH(from, to)
#  endif
#endif /* !JIT_CACHE_FLUSH */

/* bit helpers */

static crex_always_inline bool crex_long_is_power_of_two(crex_long x)
{
	return (x > 0) && !(x & (x - 1));
}

static crex_always_inline uint32_t crex_long_floor_log2(crex_long x)
{
	CREX_ASSERT(crex_long_is_power_of_two(x));
	return crex_ulong_ntz(x);
}

/* from http://aggregate.org/MAGIC/ */
static crex_always_inline uint32_t ones32(uint32_t x)
{
	x -= ((x >> 1) & 0x55555555);
	x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
	x = (((x >> 4) + x) & 0x0f0f0f0f);
	x += (x >> 8);
	x += (x >> 16);
	return x & 0x0000003f;
}

static crex_always_inline uint32_t floor_log2(uint32_t x)
{
	CREX_ASSERT(x != 0);
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return ones32(x) - 1;
}

static crex_always_inline bool is_power_of_two(uint32_t x)
{
	return !(x & (x - 1)) && x != 0;
}

static crex_always_inline bool has_concrete_type(uint32_t value_type)
{
	return is_power_of_two (value_type & (MAY_BE_ANY|MAY_BE_UNDEF));
}

static crex_always_inline uint32_t concrete_type(uint32_t value_type)
{
	return floor_log2(value_type & (MAY_BE_ANY|MAY_BE_UNDEF));
}

static crex_always_inline bool is_signed(double d)
{
	return (((unsigned char*)&d)[sizeof(double)-1] & 0x80) != 0;
}

#endif /* CREX_JIT_INTERNAL_H */
