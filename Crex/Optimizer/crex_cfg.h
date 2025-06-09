/*
   +----------------------------------------------------------------------+
   | Crex Engine, CFG - Control Flow Graph                                |
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

#ifndef CREX_CFG_H
#define CREX_CFG_H

/* crex_basic_block.flags */
#define CREX_BB_START            (1<<0)  /* first block            */
#define CREX_BB_FOLLOW           (1<<1)  /* follows the next block */
#define CREX_BB_TARGET           (1<<2)  /* jump target            */
#define CREX_BB_EXIT             (1<<3)  /* without successors     */
#define CREX_BB_ENTRY            (1<<4)  /* stackless entry        */
#define CREX_BB_TRY              (1<<5)  /* start of try block     */
#define CREX_BB_CATCH            (1<<6)  /* start of catch block   */
#define CREX_BB_FINALLY          (1<<7)  /* start of finally block */
#define CREX_BB_FINALLY_END      (1<<8)  /* end of finally block   */
#define CREX_BB_UNREACHABLE_FREE (1<<11) /* unreachable loop free  */
#define CREX_BB_RECV_ENTRY       (1<<12) /* RECV entry             */

#define CREX_BB_LOOP_HEADER      (1<<16)
#define CREX_BB_IRREDUCIBLE_LOOP (1<<17)

#define CREX_BB_REACHABLE        (1U<<31)

#define CREX_BB_PROTECTED        (CREX_BB_ENTRY|CREX_BB_RECV_ENTRY|CREX_BB_TRY|CREX_BB_CATCH|CREX_BB_FINALLY|CREX_BB_FINALLY_END|CREX_BB_UNREACHABLE_FREE)

typedef struct _crex_basic_block {
	int              *successors;         /* successor block indices     */
	uint32_t          flags;
	uint32_t          start;              /* first opcode number         */
	uint32_t          len;                /* number of opcodes           */
	int               successors_count;   /* number of successors        */
	int               predecessors_count; /* number of predecessors      */
	int               predecessor_offset; /* offset of 1-st predecessor  */
	int               idom;               /* immediate dominator block   */
	int               loop_header;        /* closest loop header, or -1  */
	int               level;              /* steps away from the entry in the dom. tree */
	int               children;           /* list of dominated blocks    */
	int               next_child;         /* next dominated block        */
	int               successors_storage[2]; /* up to 2 successor blocks */
} crex_basic_block;

/*
+------------+---+---+---+---+---+
|            |OP1|OP2|EXT| 0 | 1 |
+------------+---+---+---+---+---+
|JMP         |ADR|   |   |OP1| - |
|JMPZ        |   |ADR|   |OP2|FOL|
|JMPNZ       |   |ADR|   |OP2|FOL|
|JMPC_EX     |   |ADR|   |OP2|FOL|
|JMPNC_EX    |   |ADR|   |OP2|FOL|
|JMP_SET     |   |ADR|   |OP2|FOL|
|COALESCE    |   |ADR|   |OP2|FOL|
|ASSERT_CHK  |   |ADR|   |OP2|FOL|
|NEW         |   |ADR|   |OP2|FOL|
|DCL_ANON*   |ADR|   |   |OP1|FOL|
|FE_RESET_*  |   |ADR|   |OP2|FOL|
|FE_FETCH_*  |   |   |ADR|EXT|FOL|
|CATCH       |   |   |ADR|EXT|FOL|
|FAST_CALL   |ADR|   |   |OP1|FOL|
|FAST_RET    |   |   |   | - | - |
|RETURN*     |   |   |   | - | - |
|EXIT        |   |   |   | - | - |
|THROW       |   |   |   | - | - |
|*           |   |   |   |FOL| - |
+------------+---+---+---+---+---+
*/

typedef struct _crex_cfg {
	int               blocks_count;       /* number of basic blocks      */
	int               edges_count;        /* number of edges             */
	crex_basic_block *blocks;             /* array of basic blocks       */
	int              *predecessors;
	uint32_t         *map;
	uint32_t          flags;
} crex_cfg;

/* Build Flags */
#define CREX_CFG_STACKLESS             (1<<30)
#define CREX_SSA_DEBUG_LIVENESS        (1<<29)
#define CREX_SSA_DEBUG_PHI_PLACEMENT   (1<<28)
#define CREX_SSA_RC_INFERENCE          (1<<27)
#define CREX_CFG_NO_ENTRY_PREDECESSORS (1<<25)
#define CREX_CFG_RECV_ENTRY            (1<<24)
#define CREX_CALL_TREE                 (1<<23)
#define CREX_SSA_USE_CV_RESULTS        (1<<22)

#define CRT_CONSTANT_EX(op_array, opline, node) \
	(((op_array)->fn_flags & CREX_ACC_DONE_PASS_TWO) ? \
		RT_CONSTANT(opline, (node)) \
	: \
		CT_CONSTANT_EX(op_array, (node).constant) \
	)

#define CRT_CONSTANT(node) \
	CRT_CONSTANT_EX(op_array, opline, node)

#define RETURN_VALUE_USED(opline) \
	((opline)->result_type != IS_UNUSED)

BEGIN_EXTERN_C()

CREX_API void crex_build_cfg(crex_arena **arena, const crex_op_array *op_array, uint32_t build_flags, crex_cfg *cfg);
void crex_cfg_remark_reachable_blocks(const crex_op_array *op_array, crex_cfg *cfg);
CREX_API void crex_cfg_build_predecessors(crex_arena **arena, crex_cfg *cfg);
CREX_API void crex_cfg_compute_dominators_tree(const crex_op_array *op_array, crex_cfg *cfg);
CREX_API void crex_cfg_identify_loops(const crex_op_array *op_array, crex_cfg *cfg);

END_EXTERN_C()

#endif /* CREX_CFG_H */
