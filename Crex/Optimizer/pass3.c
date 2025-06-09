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

/* pass 3: (Jump optimization)
 * - optimize series of JMPs
 */

#include "Optimizer/crex_optimizer.h"
#include "Optimizer/crex_optimizer_internal.h"
#include "crex_API.h"
#include "crex_constants.h"
#include "crex_execute.h"
#include "crex_vm.h"

/* we use "jmp_hitlist" to avoid infinity loops during jmp optimization */
static crex_always_inline bool in_hitlist(crex_op *target, crex_op **jmp_hitlist, int jmp_hitlist_count)
{
	int i;

	for (i = 0; i < jmp_hitlist_count; i++) {
		if (jmp_hitlist[i] == target) {
			return 1;
		}
	}
	return 0;
}

#define CHECK_LOOP(target) \
	if (EXPECTED(!in_hitlist(target, jmp_hitlist, jmp_hitlist_count))) { \
		jmp_hitlist[jmp_hitlist_count++] = target;	\
	} else { \
		break; \
	}

void crex_optimizer_pass3(crex_op_array *op_array, crex_optimizer_ctx *ctx)
{
	crex_op *opline;
	crex_op *end;
	crex_op *target;
	crex_op **jmp_hitlist;
	int jmp_hitlist_count;
	ALLOCA_FLAG(use_heap);

	jmp_hitlist = (crex_op**)do_alloca(sizeof(crex_op*)*op_array->last, use_heap);
	opline = op_array->opcodes;
	end =  opline + op_array->last;

	while (opline < end) {

		switch (opline->opcode) {
			case CREX_JMP:
				jmp_hitlist_count = 0;

				target = CREX_OP1_JMP_ADDR(opline);
				while (1) {
					if (target->opcode == CREX_JMP) {
						/* convert JMP L1 ... L1: JMP L2 to JMP L2 .. L1: JMP L2 */
						target = CREX_OP1_JMP_ADDR(target);
						CHECK_LOOP(target);
					} else if (target->opcode == CREX_NOP) {
						target = target + 1;
					} else {
						break;
					}
					CREX_SET_OP_JMP_ADDR(opline, opline->op1, target);
				}

				if (target == opline + 1) {
					/* convert L: JMP L+1 to NOP */
					MAKE_NOP(opline);
				} else if ((target->opcode == CREX_RETURN ||
				            target->opcode == CREX_RETURN_BY_REF ||
				            target->opcode == CREX_GENERATOR_RETURN ||
				            target->opcode == CREX_EXIT) &&
				           !(op_array->fn_flags & CREX_ACC_HAS_FINALLY_BLOCK)) {
					/* JMP L, L: RETURN to immediate RETURN */
					*opline = *target;
					if (opline->op1_type == IS_CONST) {
						zval zv;
						ZVAL_COPY(&zv, &CREX_OP1_LITERAL(opline));
						opline->op1.constant = crex_optimizer_add_literal(op_array, &zv);
					}
				} else if (opline > op_array->opcodes &&
				           ((opline-1)->opcode == CREX_JMPZ ||
				            (opline-1)->opcode == CREX_JMPNZ)) {
				    if (CREX_OP2_JMP_ADDR(opline-1) == target) {
						/* JMPZ(X,L1), JMP(L1) -> NOP, JMP(L1) */
						crex_optimizer_convert_to_free_op1(op_array, opline - 1);
				    }
				}
				break;

			case CREX_JMP_SET:
			case CREX_COALESCE:
				jmp_hitlist_count = 0;

				target = CREX_OP2_JMP_ADDR(opline);
				while (1) {
					if (target->opcode == CREX_JMP) {
						target = CREX_OP1_JMP_ADDR(target);
						CHECK_LOOP(target);
					} else if (target->opcode == CREX_NOP) {
						target = target + 1;
					} else {
						break;
					}
					CREX_SET_OP_JMP_ADDR(opline, opline->op2, target);
				}
				break;

			case CREX_JMPZ:
			case CREX_JMPNZ:
				jmp_hitlist_count = 0;

				target = CREX_OP2_JMP_ADDR(opline);
				while (1) {
					if (target->opcode == CREX_JMP) {
						/* plain JMP */
						/* JMPZ(X,L1), L1: JMP(L2) => JMPZ(X,L2), L1: JMP(L2) */
						target = CREX_OP1_JMP_ADDR(target);
						CHECK_LOOP(target);
					} else if (target->opcode == opline->opcode &&
					           SAME_VAR(opline->op1, target->op1)) {
						/* same opcode and same var as this opcode */
						/* JMPZ(X,L1), L1: JMPZ(X,L2) => JMPZ(X,L2), L1: JMPZ(X,L2) */
						target = CREX_OP2_JMP_ADDR(target);
						CHECK_LOOP(target);
					} else if (target->opcode == INV_COND(opline->opcode) &&
					           SAME_VAR(opline->op1, target->op1)) {
						/* convert JMPZ(X,L1), L1: JMPNZ(X,L2) to
						   JMPZ(X,L1+1) */
						target = target + 1;
					} else if (target->opcode == CREX_NOP) {
						target = target + 1;
					} else {
						break;
					}
					CREX_SET_OP_JMP_ADDR(opline, opline->op2, target);
				}

				/* convert L: JMPZ L+1 to NOP */
				if (target == opline + 1) {
					crex_optimizer_convert_to_free_op1(op_array, opline);
				}
				break;

			case CREX_JMPC_EX:
			case CREX_JMPNC_EX:
				jmp_hitlist_count = 0;

				target = CREX_OP2_JMP_ADDR(opline);
				while (1) {
					if (target->opcode == CREX_JMP) {
						/* plain JMP */
						/* JMPC_EX(X,L1), L1: JMP(L2) => JMPC_EX(X,L2), L1: JMP(L2) */
						target = CREX_OP1_JMP_ADDR(target);
						CHECK_LOOP(target);
					} else if (target->opcode == opline->opcode-3 &&
					           (SAME_VAR(target->op1, opline->result) ||
					            SAME_VAR(target->op1, opline->op1))) {
						/* convert T=JMPC_EX(X,L1), L1: JMPZ(T,L2) to
						   JMPC_EX(X,L2) */
						target = CREX_OP2_JMP_ADDR(target);
						CHECK_LOOP(target);
					} else if (target->opcode == opline->opcode &&
					           target->result.var == opline->result.var &&
					           (SAME_VAR(target->op1, opline->result) ||
					            SAME_VAR(target->op1, opline->op1))) {
						/* convert T=JMPC_EX(X,L1), L1: T=JMPC_EX(T,L2) to
						   JMPC_EX(X,L2) */
						target = CREX_OP2_JMP_ADDR(target);
						CHECK_LOOP(target);
					} else if (target->opcode == INV_EX_COND(opline->opcode) &&
					           (SAME_VAR(target->op1, opline->result) ||
					            SAME_VAR(target->op1, opline->op1))) {
					   /* convert T=JMPC_EX(X,L1), L1: JMPNZ(T,L2) to
						  JMPC_EX(X,L1+1) */
						target = target + 1;
					} else if (target->opcode == INV_EX_COND_EX(opline->opcode) &&
					           target->result.var == opline->result.var &&
					           (SAME_VAR(target->op1, opline->result) ||
					            SAME_VAR(target->op1, opline->op1))) {
					   /* convert T=JMPC_EX(X,L1), L1: T=JMPNC_EX(T,L2) to
						  JMPC_EX(X,L1+1) */
						target = target + 1;
					} else if (target->opcode == CREX_BOOL &&
					           (SAME_VAR(target->op1, opline->result) ||
					            SAME_VAR(target->op1, opline->op1))) {
						/* convert Y = JMPC_EX(X,L1), L1: Z = BOOL(Y) to
						   Z = JMPC_EX(X,L1+1) */

						/* NOTE: This optimization pattern is not safe, but works, */
						/*       because result of JMPC_EX instruction             */
						/*       is not used on the following path and             */
						/*       should be used once on the branch path.           */
						/*                                                         */
						/*       The pattern works well only if jumps processed in */
						/*       direct order, otherwise it breaks JMPC_EX         */
						/*       sequences too early.                              */
						opline->result.var = target->result.var;
						target = target + 1;
						CHECK_LOOP(target);
					} else if (target->opcode == CREX_NOP) {
						target = target + 1;
					} else {
						break;
					}
					CREX_SET_OP_JMP_ADDR(opline, opline->op2, target);
				}

				/* convert L: T = JMPC_EX X,L+1 to T = BOOL(X) */
				if (target == opline + 1) {
					opline->opcode = CREX_BOOL;
					opline->op2.num = 0;
				}
				break;
		}
		opline++;
	}
	free_alloca(jmp_hitlist, use_heap);
}
