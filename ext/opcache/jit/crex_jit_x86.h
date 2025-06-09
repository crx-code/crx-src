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

#ifndef HAVE_JIT_X86_H
#define HAVE_JIT_X86_H

typedef enum _crex_reg {
	ZREG_NONE = -1,

	ZREG_R0,
	ZREG_R1,
	ZREG_R2,
	ZREG_R3,
	ZREG_R4,
	ZREG_R5,
	ZREG_R6,
	ZREG_R7,

#if defined(__x86_64__) || defined(_WIN64)
	ZREG_R8,
	ZREG_R9,
	ZREG_R10,
	ZREG_R11,
	ZREG_R12,
	ZREG_R13,
	ZREG_R14,
	ZREG_R15,
#endif

	ZREG_XMM0,
	ZREG_XMM1,
	ZREG_XMM2,
	ZREG_XMM3,
	ZREG_XMM4,
	ZREG_XMM5,
	ZREG_XMM6,
	ZREG_XMM7,

#if defined(__x86_64__) || defined(_WIN64)
	ZREG_XMM8,
	ZREG_XMM9,
	ZREG_XMM10,
	ZREG_XMM11,
	ZREG_XMM12,
	ZREG_XMM13,
	ZREG_XMM14,
	ZREG_XMM15,
#endif

	ZREG_NUM,

	ZREG_THIS, /* used for delayed FETCH_THIS deoptimization */

	/* pseudo constants used by deoptimizer */
	ZREG_LONG_MIN_MINUS_1,
	ZREG_LONG_MIN,
	ZREG_LONG_MAX,
	ZREG_LONG_MAX_PLUS_1,
	ZREG_NULL,

	ZREG_ZVAL_TRY_ADDREF,
	ZREG_ZVAL_COPY_GPR0,
} crex_reg;

typedef struct _crex_jit_registers_buf {
#if defined(__x86_64__) || defined(_WIN64)
	uint64_t gpr[16]; /* general purpose integer register */
	double   fpr[16]; /* floating point registers */
#else
	uint32_t gpr[8]; /* general purpose integer register */
	double   fpr[8]; /* floating point registers */
#endif
} crex_jit_registers_buf;

#define ZREG_FIRST_FPR ZREG_XMM0
#define ZREG_COPY      ZREG_R0

#define ZREG_RAX ZREG_R0
#define ZREG_RCX ZREG_R1
#define ZREG_RDX ZREG_R2
#define ZREG_RBX ZREG_R3
#define ZREG_RSP ZREG_R4
#define ZREG_RBP ZREG_R5
#define ZREG_RSI ZREG_R6
#define ZREG_RDI ZREG_R7

#ifdef _WIN64
# define ZREG_FP ZREG_R14
# define ZREG_IP ZREG_R15
#elif defined(__x86_64__)
# define ZREG_FP ZREG_R14
# define ZREG_IP ZREG_R15
#else
# define ZREG_FP ZREG_RSI
# define ZREG_IP ZREG_RDI
#endif

#define ZREG_RX  ZREG_IP

typedef uint32_t crex_regset;

#define CREX_REGSET_64BIT 0

#ifdef _WIN64
# define CREX_REGSET_FIXED \
	(CREX_REGSET(ZREG_RSP) | CREX_REGSET(ZREG_R14) | CREX_REGSET(ZREG_R15))
# define CREX_REGSET_GP \
	CREX_REGSET_DIFFERENCE(CREX_REGSET_INTERVAL(ZREG_R0, ZREG_R15), CREX_REGSET_FIXED)
# define CREX_REGSET_FP \
	CREX_REGSET_DIFFERENCE(CREX_REGSET_INTERVAL(ZREG_XMM0, ZREG_XMM15), CREX_REGSET_FIXED)
# define CREX_REGSET_SCRATCH \
	(CREX_REGSET(ZREG_RAX) | CREX_REGSET(ZREG_RDX) | CREX_REGSET(ZREG_RCX) | CREX_REGSET_INTERVAL(ZREG_R8, ZREG_R11) | CREX_REGSET_FP)
# define CREX_REGSET_PRESERVED \
	(CREX_REGSET(ZREG_RBX) | CREX_REGSET(ZREG_RBP) | CREX_REGSET(ZREG_R12) | CREX_REGSET(ZREG_R13) | CREX_REGSET(ZREG_RDI) | CREX_REGSET(ZREG_RSI))
#elif defined(__x86_64__)
# define CREX_REGSET_FIXED \
	(CREX_REGSET(ZREG_RSP) | CREX_REGSET(ZREG_R14) | CREX_REGSET(ZREG_R15))
# define CREX_REGSET_GP \
	CREX_REGSET_DIFFERENCE(CREX_REGSET_INTERVAL(ZREG_R0, ZREG_R15), CREX_REGSET_FIXED)
# define CREX_REGSET_FP \
	CREX_REGSET_DIFFERENCE(CREX_REGSET_INTERVAL(ZREG_XMM0, ZREG_XMM15), CREX_REGSET_FIXED)
# define CREX_REGSET_SCRATCH \
	(CREX_REGSET(ZREG_RAX) | CREX_REGSET(ZREG_RDI) | CREX_REGSET(ZREG_RSI) | CREX_REGSET(ZREG_RDX) | CREX_REGSET(ZREG_RCX) | CREX_REGSET_INTERVAL(ZREG_R8, ZREG_R11) | CREX_REGSET_FP)
# define CREX_REGSET_PRESERVED \
	(CREX_REGSET(ZREG_RBX) | CREX_REGSET(ZREG_RBP) | CREX_REGSET(ZREG_R12) | CREX_REGSET(ZREG_R13))
#else
# define CREX_REGSET_FIXED \
	(CREX_REGSET(ZREG_RSP) | CREX_REGSET(ZREG_RSI) | CREX_REGSET(ZREG_RDI))
# define CREX_REGSET_GP \
	CREX_REGSET_DIFFERENCE(CREX_REGSET_INTERVAL(ZREG_R0, ZREG_R7), CREX_REGSET_FIXED)
# define CREX_REGSET_FP \
	CREX_REGSET_DIFFERENCE(CREX_REGSET_INTERVAL(ZREG_XMM0, ZREG_XMM7), CREX_REGSET_FIXED)
# define CREX_REGSET_SCRATCH \
	(CREX_REGSET(ZREG_RAX) | CREX_REGSET(ZREG_RCX) | CREX_REGSET(ZREG_RDX) | CREX_REGSET_FP)
# define CREX_REGSET_PRESERVED \
	(CREX_REGSET(ZREG_RBX) | CREX_REGSET(ZREG_RBP))
#endif

#define CREX_REGSET_LOW_PRIORITY \
	(CREX_REGSET(ZREG_R0) | CREX_REGSET(ZREG_R1) | CREX_REGSET(ZREG_XMM0) | CREX_REGSET(ZREG_XMM1))

#endif /* CREX_JIT_X86_H */
