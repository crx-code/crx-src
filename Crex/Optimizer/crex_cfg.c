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

#include "crex_compile.h"
#include "crex_cfg.h"
#include "crex_func_info.h"
#include "crex_worklist.h"
#include "crex_optimizer.h"
#include "crex_optimizer_internal.h"
#include "crex_sort.h"

static void crex_mark_reachable(crex_op *opcodes, crex_cfg *cfg, crex_basic_block *b) /* {{{ */
{
	crex_basic_block *blocks = cfg->blocks;

	while (1) {
		int i;

		b->flags |= CREX_BB_REACHABLE;
		if (b->successors_count == 0) {
			b->flags |= CREX_BB_EXIT;
			return;
		}

		for (i = 0; i < b->successors_count; i++) {
			crex_basic_block *succ = blocks + b->successors[i];

			if (b->len != 0) {
				uint8_t opcode = opcodes[b->start + b->len - 1].opcode;
				if (opcode == CREX_MATCH) {
					succ->flags |= CREX_BB_TARGET;
				} else if (opcode == CREX_SWITCH_LONG || opcode == CREX_SWITCH_STRING) {
					if (i == b->successors_count - 1) {
						succ->flags |= CREX_BB_FOLLOW | CREX_BB_TARGET;
					} else {
						succ->flags |= CREX_BB_TARGET;
					}
				} else if (b->successors_count == 1) {
					if (opcode == CREX_JMP) {
						succ->flags |= CREX_BB_TARGET;
					} else {
						succ->flags |= CREX_BB_FOLLOW;

						if ((cfg->flags & CREX_CFG_STACKLESS)) {
							if (opcode == CREX_INCLUDE_OR_EVAL ||
								opcode == CREX_GENERATOR_CREATE ||
								opcode == CREX_YIELD ||
								opcode == CREX_YIELD_FROM ||
								opcode == CREX_DO_FCALL ||
								opcode == CREX_DO_UCALL ||
								opcode == CREX_DO_FCALL_BY_NAME) {
								succ->flags |= CREX_BB_ENTRY;
							}
						}
						if ((cfg->flags & CREX_CFG_RECV_ENTRY)) {
							if (opcode == CREX_RECV ||
								opcode == CREX_RECV_INIT) {
								succ->flags |= CREX_BB_RECV_ENTRY;
							}
						}
					}
				} else {
					CREX_ASSERT(b->successors_count == 2);
					if (i == 0) {
						succ->flags |= CREX_BB_TARGET;
					} else {
						succ->flags |= CREX_BB_FOLLOW;
					}
				}
			} else {
				succ->flags |= CREX_BB_FOLLOW;
			}

			if (i == b->successors_count - 1) {
				/* Tail call optimization */
				if (succ->flags & CREX_BB_REACHABLE) {
					return;
				}

				b = succ;
				break;
			} else {
				/* Recursively check reachability */
				if (!(succ->flags & CREX_BB_REACHABLE)) {
					crex_mark_reachable(opcodes, cfg, succ);
				}
			}
		}
	}
}
/* }}} */

static void crex_mark_reachable_blocks(const crex_op_array *op_array, crex_cfg *cfg, int start) /* {{{ */
{
	crex_basic_block *blocks = cfg->blocks;

	blocks[start].flags = CREX_BB_START;
	crex_mark_reachable(op_array->opcodes, cfg, blocks + start);

	if (op_array->last_try_catch) {
		crex_basic_block *b;
		int j, changed;
		uint32_t *block_map = cfg->map;

		do {
			changed = 0;

			/* Add exception paths */
			for (j = 0; j < op_array->last_try_catch; j++) {

				/* check for jumps into the middle of try block */
				b = blocks + block_map[op_array->try_catch_array[j].try_op];
				if (!(b->flags & CREX_BB_REACHABLE)) {
					crex_basic_block *end;

					if (op_array->try_catch_array[j].catch_op) {
						end = blocks + block_map[op_array->try_catch_array[j].catch_op];
						while (b != end) {
							if (b->flags & CREX_BB_REACHABLE) {
								op_array->try_catch_array[j].try_op = b->start;
								break;
							}
							b++;
						}
					}
					b = blocks + block_map[op_array->try_catch_array[j].try_op];
					if (!(b->flags & CREX_BB_REACHABLE)) {
						if (op_array->try_catch_array[j].finally_op) {
							end = blocks + block_map[op_array->try_catch_array[j].finally_op];
							while (b != end) {
								if (b->flags & CREX_BB_REACHABLE) {
									op_array->try_catch_array[j].try_op = op_array->try_catch_array[j].catch_op;
									changed = 1;
									crex_mark_reachable(op_array->opcodes, cfg, blocks + block_map[op_array->try_catch_array[j].try_op]);
									break;
								}
								b++;
							}
						}
					}
				}

				b = blocks + block_map[op_array->try_catch_array[j].try_op];
				if (b->flags & CREX_BB_REACHABLE) {
					b->flags |= CREX_BB_TRY;
					if (op_array->try_catch_array[j].catch_op) {
						b = blocks + block_map[op_array->try_catch_array[j].catch_op];
						b->flags |= CREX_BB_CATCH;
						if (!(b->flags & CREX_BB_REACHABLE)) {
							changed = 1;
							crex_mark_reachable(op_array->opcodes, cfg, b);
						}
					}
					if (op_array->try_catch_array[j].finally_op) {
						b = blocks + block_map[op_array->try_catch_array[j].finally_op];
						b->flags |= CREX_BB_FINALLY;
						if (!(b->flags & CREX_BB_REACHABLE)) {
							changed = 1;
							crex_mark_reachable(op_array->opcodes, cfg, b);
						}
					}
					if (op_array->try_catch_array[j].finally_end) {
						b = blocks + block_map[op_array->try_catch_array[j].finally_end];
						b->flags |= CREX_BB_FINALLY_END;
						if (!(b->flags & CREX_BB_REACHABLE)) {
							changed = 1;
							crex_mark_reachable(op_array->opcodes, cfg, b);
						}
					}
				} else {
					if (op_array->try_catch_array[j].catch_op) {
						CREX_ASSERT(!(blocks[block_map[op_array->try_catch_array[j].catch_op]].flags & CREX_BB_REACHABLE));
					}
					if (op_array->try_catch_array[j].finally_op) {
						CREX_ASSERT(!(blocks[block_map[op_array->try_catch_array[j].finally_op]].flags & CREX_BB_REACHABLE));
					}
					if (op_array->try_catch_array[j].finally_end) {
						CREX_ASSERT(!(blocks[block_map[op_array->try_catch_array[j].finally_end]].flags & CREX_BB_REACHABLE));
					}
				}
			}
		} while (changed);
	}

	if (cfg->flags & CREX_FUNC_FREE_LOOP_VAR) {
		crex_basic_block *b;
		int j;
		uint32_t *block_map = cfg->map;

		/* Mark blocks that are unreachable, but free a loop var created in a reachable block. */
		for (b = blocks; b < blocks + cfg->blocks_count; b++) {
			if (b->flags & CREX_BB_REACHABLE) {
				continue;
			}

			for (j = b->start; j < b->start + b->len; j++) {
				crex_op *opline = &op_array->opcodes[j];
				if (crex_optimizer_is_loop_var_free(opline)) {
					crex_op *def_opline = crex_optimizer_get_loop_var_def(op_array, opline);
					if (def_opline) {
						uint32_t def_block = block_map[def_opline - op_array->opcodes];
						if (blocks[def_block].flags & CREX_BB_REACHABLE) {
							b->flags |= CREX_BB_UNREACHABLE_FREE;
							break;
						}
					}
				}
			}
		}
	}
}
/* }}} */

void crex_cfg_remark_reachable_blocks(const crex_op_array *op_array, crex_cfg *cfg) /* {{{ */
{
	crex_basic_block *blocks = cfg->blocks;
	int i;
	int start = 0;

	for (i = 0; i < cfg->blocks_count; i++) {
		if (blocks[i].flags & CREX_BB_REACHABLE) {
			start = i;
			i++;
			break;
		}
	}

	/* clear all flags */
	for (i = 0; i < cfg->blocks_count; i++) {
		blocks[i].flags = 0;
	}

	crex_mark_reachable_blocks(op_array, cfg, start);
}
/* }}} */

static void initialize_block(crex_basic_block *block) {
	block->flags = 0;
	block->successors = block->successors_storage;
	block->successors_count = 0;
	block->predecessors_count = 0;
	block->predecessor_offset = -1;
	block->idom = -1;
	block->loop_header = -1;
	block->level = -1;
	block->children = -1;
	block->next_child = -1;
}

#define BB_START(i) do { \
		if (!block_map[i]) { blocks_count++;} \
		block_map[i]++; \
	} while (0)

CREX_API void crex_build_cfg(crex_arena **arena, const crex_op_array *op_array, uint32_t build_flags, crex_cfg *cfg) /* {{{ */
{
	uint32_t flags = 0;
	uint32_t i;
	int j;
	uint32_t *block_map;
	crex_function *fn;
	int blocks_count = 0;
	crex_basic_block *blocks;
	zval *zv;
	bool extra_entry_block = 0;

	cfg->flags = build_flags & (CREX_CFG_STACKLESS|CREX_CFG_RECV_ENTRY);

	cfg->map = block_map = crex_arena_calloc(arena, op_array->last, sizeof(uint32_t));

	/* Build CFG, Step 1: Find basic blocks starts, calculate number of blocks */
	BB_START(0);
	for (i = 0; i < op_array->last; i++) {
		crex_op *opline = op_array->opcodes + i;
		switch (opline->opcode) {
			case CREX_RECV:
			case CREX_RECV_INIT:
				if (build_flags & CREX_CFG_RECV_ENTRY) {
					BB_START(i + 1);
				}
				break;
			case CREX_RETURN:
			case CREX_RETURN_BY_REF:
			case CREX_GENERATOR_RETURN:
			case CREX_VERIFY_NEVER_TYPE:
				if (i + 1 < op_array->last) {
					BB_START(i + 1);
				}
				break;
			case CREX_MATCH_ERROR:
			case CREX_EXIT:
			case CREX_THROW:
				/* Don't treat THROW as terminator if it's used in expression context,
				 * as we may lose live ranges when eliminating unreachable code. */
				if (opline->extended_value != CREX_THROW_IS_EXPR && i + 1 < op_array->last) {
					BB_START(i + 1);
				}
				break;
			case CREX_INCLUDE_OR_EVAL:
				flags |= CREX_FUNC_INDIRECT_VAR_ACCESS;
				CREX_FALLTHROUGH;
			case CREX_GENERATOR_CREATE:
			case CREX_YIELD:
			case CREX_YIELD_FROM:
				if (build_flags & CREX_CFG_STACKLESS) {
					BB_START(i + 1);
				}
				break;
			case CREX_DO_FCALL:
			case CREX_DO_UCALL:
			case CREX_DO_FCALL_BY_NAME:
				flags |= CREX_FUNC_HAS_CALLS;
				if (build_flags & CREX_CFG_STACKLESS) {
					BB_START(i + 1);
				}
				break;
			case CREX_DO_ICALL:
				flags |= CREX_FUNC_HAS_CALLS;
				break;
			case CREX_INIT_FCALL:
			case CREX_INIT_NS_FCALL_BY_NAME:
				zv = CRT_CONSTANT(opline->op2);
				if (opline->opcode == CREX_INIT_NS_FCALL_BY_NAME) {
					/* The third literal is the lowercased unqualified name */
					zv += 2;
				}
				if ((fn = crex_hash_find_ptr(EG(function_table), C_STR_P(zv))) != NULL) {
					if (fn->type == CREX_INTERNAL_FUNCTION) {
						flags |= crex_optimizer_classify_function(
							C_STR_P(zv), opline->extended_value);
					}
				}
				break;
			case CREX_FAST_CALL:
				BB_START(OP_JMP_ADDR(opline, opline->op1) - op_array->opcodes);
				BB_START(i + 1);
				break;
			case CREX_FAST_RET:
				if (i + 1 < op_array->last) {
					BB_START(i + 1);
				}
				break;
			case CREX_JMP:
				BB_START(OP_JMP_ADDR(opline, opline->op1) - op_array->opcodes);
				if (i + 1 < op_array->last) {
					BB_START(i + 1);
				}
				break;
			case CREX_JMPZ:
			case CREX_JMPNZ:
			case CREX_JMPC_EX:
			case CREX_JMPNC_EX:
			case CREX_JMP_SET:
			case CREX_COALESCE:
			case CREX_ASSERT_CHECK:
			case CREX_JMP_NULL:
			case CREX_BIND_INIT_STATIC_OR_JMP:
				BB_START(OP_JMP_ADDR(opline, opline->op2) - op_array->opcodes);
				BB_START(i + 1);
				break;
			case CREX_CATCH:
				if (!(opline->extended_value & CREX_LAST_CATCH)) {
					BB_START(OP_JMP_ADDR(opline, opline->op2) - op_array->opcodes);
				}
				BB_START(i + 1);
				break;
			case CREX_FE_FETCH_R:
			case CREX_FE_FETCH_RW:
				BB_START(CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, opline->extended_value));
				BB_START(i + 1);
				break;
			case CREX_FE_RESET_R:
			case CREX_FE_RESET_RW:
				BB_START(OP_JMP_ADDR(opline, opline->op2) - op_array->opcodes);
				BB_START(i + 1);
				break;
			case CREX_SWITCH_LONG:
			case CREX_SWITCH_STRING:
			case CREX_MATCH:
			{
				HashTable *jumptable = C_ARRVAL_P(CRT_CONSTANT(opline->op2));
				zval *zv;
				CREX_HASH_FOREACH_VAL(jumptable, zv) {
					BB_START(CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, C_LVAL_P(zv)));
				} CREX_HASH_FOREACH_END();
				BB_START(CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, opline->extended_value));
				BB_START(i + 1);
				break;
			}
			case CREX_FETCH_R:
			case CREX_FETCH_W:
			case CREX_FETCH_RW:
			case CREX_FETCH_FUNC_ARG:
			case CREX_FETCH_IS:
			case CREX_FETCH_UNSET:
			case CREX_UNSET_VAR:
			case CREX_ISSET_ISEMPTY_VAR:
				if (opline->extended_value & CREX_FETCH_LOCAL) {
					flags |= CREX_FUNC_INDIRECT_VAR_ACCESS;
				} else if ((opline->extended_value & (CREX_FETCH_GLOBAL | CREX_FETCH_GLOBAL_LOCK)) &&
				           !op_array->function_name) {
					flags |= CREX_FUNC_INDIRECT_VAR_ACCESS;
				}
				break;
			case CREX_FUNC_GET_ARGS:
				flags |= CREX_FUNC_VARARG;
				break;
			case CREX_EXT_STMT:
				flags |= CREX_FUNC_HAS_EXTENDED_STMT;
				break;
			case CREX_EXT_FCALL_BEGIN:
			case CREX_EXT_FCALL_END:
				flags |= CREX_FUNC_HAS_EXTENDED_FCALL;
				break;
			case CREX_FREE:
			case CREX_FE_FREE:
				if (crex_optimizer_is_loop_var_free(opline)
				 && ((opline-1)->opcode != CREX_MATCH_ERROR
				  || (opline-1)->extended_value != CREX_THROW_IS_EXPR)) {
					BB_START(i);
					flags |= CREX_FUNC_FREE_LOOP_VAR;
				}
				break;
		}
	}

	/* If the entry block has predecessors, we may need to split it */
	if ((build_flags & CREX_CFG_NO_ENTRY_PREDECESSORS)
			&& op_array->last > 0 && block_map[0] > 1) {
		extra_entry_block = 1;
	}

	if (op_array->last_try_catch) {
		for (j = 0; j < op_array->last_try_catch; j++) {
			BB_START(op_array->try_catch_array[j].try_op);
			if (op_array->try_catch_array[j].catch_op) {
				BB_START(op_array->try_catch_array[j].catch_op);
			}
			if (op_array->try_catch_array[j].finally_op) {
				BB_START(op_array->try_catch_array[j].finally_op);
			}
			if (op_array->try_catch_array[j].finally_end) {
				BB_START(op_array->try_catch_array[j].finally_end);
			}
		}
	}

	blocks_count += extra_entry_block;
	cfg->blocks_count = blocks_count;

	/* Build CFG, Step 2: Build Array of Basic Blocks */
	cfg->blocks = blocks = crex_arena_calloc(arena, sizeof(crex_basic_block), blocks_count);

	blocks_count = -1;

	if (extra_entry_block) {
		initialize_block(&blocks[0]);
		blocks[0].start = 0;
		blocks[0].len = 0;
		blocks_count++;
	}

	for (i = 0; i < op_array->last; i++) {
		if (block_map[i]) {
			if (blocks_count >= 0) {
				blocks[blocks_count].len = i - blocks[blocks_count].start;
			}
			blocks_count++;
			initialize_block(&blocks[blocks_count]);
			blocks[blocks_count].start = i;
		}
		block_map[i] = blocks_count;
	}

	blocks[blocks_count].len = i - blocks[blocks_count].start;
	blocks_count++;

	/* Build CFG, Step 3: Calculate successors */
	for (j = 0; j < blocks_count; j++) {
		crex_basic_block *block = &blocks[j];
		crex_op *opline;
		if (block->len == 0) {
			block->successors_count = 1;
			block->successors[0] = j + 1;
			continue;
		}

		opline = op_array->opcodes + block->start + block->len - 1;
		switch (opline->opcode) {
			case CREX_FAST_RET:
			case CREX_RETURN:
			case CREX_RETURN_BY_REF:
			case CREX_GENERATOR_RETURN:
			case CREX_EXIT:
			case CREX_THROW:
			case CREX_MATCH_ERROR:
			case CREX_VERIFY_NEVER_TYPE:
				break;
			case CREX_JMP:
				block->successors_count = 1;
				block->successors[0] = block_map[OP_JMP_ADDR(opline, opline->op1) - op_array->opcodes];
				break;
			case CREX_JMPZ:
			case CREX_JMPNZ:
			case CREX_JMPC_EX:
			case CREX_JMPNC_EX:
			case CREX_JMP_SET:
			case CREX_COALESCE:
			case CREX_ASSERT_CHECK:
			case CREX_JMP_NULL:
			case CREX_BIND_INIT_STATIC_OR_JMP:
				block->successors_count = 2;
				block->successors[0] = block_map[OP_JMP_ADDR(opline, opline->op2) - op_array->opcodes];
				block->successors[1] = j + 1;
				break;
			case CREX_CATCH:
				if (!(opline->extended_value & CREX_LAST_CATCH)) {
					block->successors_count = 2;
					block->successors[0] = block_map[OP_JMP_ADDR(opline, opline->op2) - op_array->opcodes];
					block->successors[1] = j + 1;
				} else {
					block->successors_count = 1;
					block->successors[0] = j + 1;
				}
				break;
			case CREX_FE_FETCH_R:
			case CREX_FE_FETCH_RW:
				block->successors_count = 2;
				block->successors[0] = block_map[CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, opline->extended_value)];
				block->successors[1] = j + 1;
				break;
			case CREX_FE_RESET_R:
			case CREX_FE_RESET_RW:
				block->successors_count = 2;
				block->successors[0] = block_map[OP_JMP_ADDR(opline, opline->op2) - op_array->opcodes];
				block->successors[1] = j + 1;
				break;
			case CREX_FAST_CALL:
				block->successors_count = 2;
				block->successors[0] = block_map[OP_JMP_ADDR(opline, opline->op1) - op_array->opcodes];
				block->successors[1] = j + 1;
				break;
			case CREX_SWITCH_LONG:
			case CREX_SWITCH_STRING:
			case CREX_MATCH:
			{
				HashTable *jumptable = C_ARRVAL_P(CRT_CONSTANT(opline->op2));
				zval *zv;
				uint32_t s = 0;

				block->successors_count = (opline->opcode == CREX_MATCH ? 1 : 2) + crex_hash_num_elements(jumptable);
				block->successors = crex_arena_calloc(arena, block->successors_count, sizeof(int));

				CREX_HASH_FOREACH_VAL(jumptable, zv) {
					block->successors[s++] = block_map[CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, C_LVAL_P(zv))];
				} CREX_HASH_FOREACH_END();

				block->successors[s++] = block_map[CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, opline->extended_value)];
				if (opline->opcode != CREX_MATCH) {
					block->successors[s++] = j + 1;
				}
				break;
			}
			default:
				block->successors_count = 1;
				block->successors[0] = j + 1;
				break;
		}
	}

	/* Build CFG, Step 4, Mark Reachable Basic Blocks */
	cfg->flags |= flags;
	crex_mark_reachable_blocks(op_array, cfg, 0);
}
/* }}} */

CREX_API void crex_cfg_build_predecessors(crex_arena **arena, crex_cfg *cfg) /* {{{ */
{
	int j, s, edges;
	crex_basic_block *b;
	crex_basic_block *blocks = cfg->blocks;
	crex_basic_block *end = blocks + cfg->blocks_count;
	int *predecessors;

	edges = 0;
	for (b = blocks; b < end; b++) {
		b->predecessors_count = 0;
	}
	for (b = blocks; b < end; b++) {
		if (!(b->flags & CREX_BB_REACHABLE)) {
			b->successors_count = 0;
			b->predecessors_count = 0;
		} else {
			for (s = 0; s < b->successors_count; s++) {
				edges++;
				blocks[b->successors[s]].predecessors_count++;
			}
		}
	}

	cfg->edges_count = edges;
	cfg->predecessors = predecessors = (int*)crex_arena_calloc(arena, sizeof(int), edges);

	edges = 0;
	for (b = blocks; b < end; b++) {
		if (b->flags & CREX_BB_REACHABLE) {
			b->predecessor_offset = edges;
			edges += b->predecessors_count;
			b->predecessors_count = 0;
		}
	}

	for (j = 0; j < cfg->blocks_count; j++) {
		if (blocks[j].flags & CREX_BB_REACHABLE) {
			/* SWITCH_STRING/LONG may have few identical successors */
			for (s = 0; s < blocks[j].successors_count; s++) {
				int duplicate = 0;
				int p;

				for (p = 0; p < s; p++) {
					if (blocks[j].successors[p] == blocks[j].successors[s]) {
						duplicate = 1;
						break;
					}
				}
				if (!duplicate) {
					crex_basic_block *b = blocks + blocks[j].successors[s];

					predecessors[b->predecessor_offset + b->predecessors_count] = j;
					b->predecessors_count++;
				}
			}
		}
	}
}
/* }}} */

/* Computes a postorder numbering of the CFG */
static void compute_postnum_recursive(
		int *postnum, int *cur, const crex_cfg *cfg, int block_num) /* {{{ */
{
	int s;
	crex_basic_block *block = &cfg->blocks[block_num];
	if (postnum[block_num] != -1) {
		return;
	}

	postnum[block_num] = -2; /* Marker for "currently visiting" */
	for (s = 0; s < block->successors_count; s++) {
		compute_postnum_recursive(postnum, cur, cfg, block->successors[s]);
	}
	postnum[block_num] = (*cur)++;
}
/* }}} */

/* Computes dominator tree using algorithm from "A Simple, Fast Dominance Algorithm" by
 * Cooper, Harvey and Kennedy. */
CREX_API void crex_cfg_compute_dominators_tree(const crex_op_array *op_array, crex_cfg *cfg) /* {{{ */
{
	crex_basic_block *blocks = cfg->blocks;
	int blocks_count = cfg->blocks_count;
	int j, k, changed;

	if (cfg->blocks_count == 1) {
		blocks[0].level = 0;
		return;
	}

	ALLOCA_FLAG(use_heap)
	int *postnum = do_alloca(sizeof(int) * cfg->blocks_count, use_heap);
	memset(postnum, -1, sizeof(int) * cfg->blocks_count);
	j = 0;
	compute_postnum_recursive(postnum, &j, cfg, 0);

	/* FIXME: move declarations */
	blocks[0].idom = 0;
	do {
		changed = 0;
		/* Iterating in RPO here would converge faster */
		for (j = 1; j < blocks_count; j++) {
			int idom = -1;

			if ((blocks[j].flags & CREX_BB_REACHABLE) == 0) {
				continue;
			}
			for (k = 0; k < blocks[j].predecessors_count; k++) {
				int pred = cfg->predecessors[blocks[j].predecessor_offset + k];

				if (blocks[pred].idom >= 0) {
					if (idom < 0) {
						idom = pred;
					} else {
						while (idom != pred) {
							while (postnum[pred] < postnum[idom]) pred = blocks[pred].idom;
							while (postnum[idom] < postnum[pred]) idom = blocks[idom].idom;
						}
					}
				}
			}

			if (idom >= 0 && blocks[j].idom != idom) {
				blocks[j].idom = idom;
				changed = 1;
			}
		}
	} while (changed);
	blocks[0].idom = -1;

	for (j = 1; j < blocks_count; j++) {
		if ((blocks[j].flags & CREX_BB_REACHABLE) == 0) {
			continue;
		}
		if (blocks[j].idom >= 0) {
			/* Sort by block number to traverse children in pre-order */
			if (blocks[blocks[j].idom].children < 0 ||
			    j < blocks[blocks[j].idom].children) {
				blocks[j].next_child = blocks[blocks[j].idom].children;
				blocks[blocks[j].idom].children = j;
			} else {
				int k = blocks[blocks[j].idom].children;
				while (blocks[k].next_child >=0 && j > blocks[k].next_child) {
					k = blocks[k].next_child;
				}
				blocks[j].next_child = blocks[k].next_child;
				blocks[k].next_child = j;
			}
		}
	}

	for (j = 0; j < blocks_count; j++) {
		int idom = blocks[j].idom, level = 0;
		if ((blocks[j].flags & CREX_BB_REACHABLE) == 0) {
			continue;
		}
		while (idom >= 0) {
			level++;
			if (blocks[idom].level >= 0) {
				level += blocks[idom].level;
				break;
			} else {
				idom = blocks[idom].idom;
			}
		}
		blocks[j].level = level;
	}

	free_alloca(postnum, use_heap);
}
/* }}} */

static bool dominates(crex_basic_block *blocks, int a, int b) /* {{{ */
{
	while (blocks[b].level > blocks[a].level) {
		b = blocks[b].idom;
	}
	return a == b;
}
/* }}} */

CREX_API void crex_cfg_identify_loops(const crex_op_array *op_array, crex_cfg *cfg) /* {{{ */
{
	int i, j, k, n;
	int time;
	crex_basic_block *blocks = cfg->blocks;
	int *entry_times, *exit_times;
	crex_worklist work;
	int flag = CREX_FUNC_NO_LOOPS;
	int *sorted_blocks;
	ALLOCA_FLAG(list_use_heap)
	ALLOCA_FLAG(tree_use_heap)

	if (cfg->blocks_count == 1) {
		cfg->flags |= flag;
		return;
	}

	CREX_WORKLIST_ALLOCA(&work, cfg->blocks_count, list_use_heap);

	/* We don't materialize the DJ spanning tree explicitly, as we are only interested in ancestor
	 * queries. These are implemented by checking entry/exit times of the DFS search. */
	entry_times = do_alloca(3 * sizeof(int) * cfg->blocks_count, tree_use_heap);
	exit_times = entry_times + cfg->blocks_count;
	sorted_blocks = exit_times + cfg->blocks_count;
	memset(entry_times, -1, 2 * sizeof(int) * cfg->blocks_count);

	crex_worklist_push(&work, 0);
	time = 0;
	while (crex_worklist_len(&work)) {
	next:
		i = crex_worklist_peek(&work);
		if (entry_times[i] == -1) {
			entry_times[i] = time++;
		}
		/* Visit blocks immediately dominated by i. */
		for (j = blocks[i].children; j >= 0; j = blocks[j].next_child) {
			if (crex_worklist_push(&work, j)) {
				goto next;
			}
		}
		/* Visit join edges.  */
		for (j = 0; j < blocks[i].successors_count; j++) {
			int succ = blocks[i].successors[j];
			if (blocks[succ].idom == i) {
				continue;
			} else if (crex_worklist_push(&work, succ)) {
				goto next;
			}
		}
		exit_times[i] = time++;
		crex_worklist_pop(&work);
	}

	/* Sort blocks by level, which is the opposite order in which we want to process them */
	sorted_blocks[0] = 0;
	j = 0;
	n = 1;
	while (j != n) {
		i = j;
		j = n;
		for (; i < j; i++) {
			int child;
			for (child = blocks[sorted_blocks[i]].children; child >= 0; child = blocks[child].next_child) {
				sorted_blocks[n++] = child;
			}
		}
	}

	/* Identify loops. See Sreedhar et al, "Identifying Loops Using DJ Graphs". */
	while (n > 0) {
		i = sorted_blocks[--n];

		if (blocks[i].predecessors_count < 2) {
		    /* loop header has at least two input edges */
			continue;
		}

		for (j = 0; j < blocks[i].predecessors_count; j++) {
			int pred = cfg->predecessors[blocks[i].predecessor_offset + j];

			/* A join edge is one for which the predecessor does not
			   immediately dominate the successor. */
			if (blocks[i].idom == pred) {
				continue;
			}

			/* In a loop back-edge (back-join edge), the successor dominates
			   the predecessor.  */
			if (dominates(blocks, i, pred)) {
				blocks[i].flags |= CREX_BB_LOOP_HEADER;
				flag &= ~CREX_FUNC_NO_LOOPS;
				if (!crex_worklist_len(&work)) {
					crex_bitset_clear(work.visited, crex_bitset_len(cfg->blocks_count));
				}
				crex_worklist_push(&work, pred);
			} else {
				/* Otherwise it's a cross-join edge.  See if it's a branch
				   to an ancestor on the DJ spanning tree.  */
				if (entry_times[pred] > entry_times[i] && exit_times[pred] < exit_times[i]) {
					blocks[i].flags |= CREX_BB_IRREDUCIBLE_LOOP;
					flag |= CREX_FUNC_IRREDUCIBLE;
					flag &= ~CREX_FUNC_NO_LOOPS;
				}
			}
		}
		while (crex_worklist_len(&work)) {
			j = crex_worklist_pop(&work);
			while (blocks[j].loop_header >= 0) {
				j = blocks[j].loop_header;
			}
			if (j != i) {
				if (blocks[j].idom < 0 && j != 0) {
					/* Ignore blocks that are unreachable or only abnormally reachable. */
					continue;
				}
				blocks[j].loop_header = i;
				for (k = 0; k < blocks[j].predecessors_count; k++) {
					crex_worklist_push(&work, cfg->predecessors[blocks[j].predecessor_offset + k]);
				}
			}
		}
	}

	free_alloca(entry_times, tree_use_heap);
	CREX_WORKLIST_FREE_ALLOCA(&work, list_use_heap);

	cfg->flags |= flag;
}
/* }}} */
