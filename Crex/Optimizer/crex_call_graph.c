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
   | Authors: Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#include "crex_compile.h"
#include "crex_extensions.h"
#include "Optimizer/crex_optimizer.h"
#include "crex_optimizer_internal.h"
#include "crex_inference.h"
#include "crex_call_graph.h"
#include "crex_func_info.h"
#include "crex_inference.h"
#include "crex_call_graph.h"

static void crex_op_array_calc(crex_op_array *op_array, void *context)
{
	crex_call_graph *call_graph = context;
	call_graph->op_arrays_count++;
}

static void crex_op_array_collect(crex_op_array *op_array, void *context)
{
	crex_call_graph *call_graph = context;
	crex_func_info *func_info = call_graph->func_infos + call_graph->op_arrays_count;

	CREX_SET_FUNC_INFO(op_array, func_info);
	call_graph->op_arrays[call_graph->op_arrays_count] = op_array;
	func_info->num = call_graph->op_arrays_count;
	call_graph->op_arrays_count++;
}

CREX_API void crex_analyze_calls(crex_arena **arena, crex_script *script, uint32_t build_flags, crex_op_array *op_array, crex_func_info *func_info)
{
	crex_op *opline = op_array->opcodes;
	crex_op *end = opline + op_array->last;
	crex_function *func;
	crex_call_info *call_info;
	int call = 0;
	crex_call_info **call_stack;
	ALLOCA_FLAG(use_heap);
	bool is_prototype;

	call_stack = do_alloca((op_array->last / 2) * sizeof(crex_call_info*), use_heap);
	call_info = NULL;
	while (opline != end) {
		switch (opline->opcode) {
			case CREX_INIT_FCALL:
			case CREX_INIT_METHOD_CALL:
			case CREX_INIT_STATIC_METHOD_CALL:
				call_stack[call] = call_info;
				func = crex_optimizer_get_called_func(
					script, op_array, opline, &is_prototype);
				if (func) {
					call_info = crex_arena_calloc(arena, 1, sizeof(crex_call_info) + (sizeof(crex_send_arg_info) * ((int)opline->extended_value - 1)));
					call_info->caller_op_array = op_array;
					call_info->caller_init_opline = opline;
					call_info->caller_call_opline = NULL;
					call_info->callee_func = func;
					call_info->num_args = opline->extended_value;
					call_info->next_callee = func_info->callee_info;
					call_info->is_prototype = is_prototype;
					func_info->callee_info = call_info;

					if (build_flags & CREX_CALL_TREE) {
						call_info->next_caller = NULL;
					} else if (func->type == CREX_INTERNAL_FUNCTION) {
						call_info->next_caller = NULL;
					} else {
						crex_func_info *callee_func_info = CREX_FUNC_INFO(&func->op_array);
						if (callee_func_info) {
							call_info->next_caller = callee_func_info->caller_info;
							callee_func_info->caller_info = call_info;
						} else {
							call_info->next_caller = NULL;
						}
					}
				} else {
					call_info = NULL;
				}
				call++;
				break;
			case CREX_INIT_FCALL_BY_NAME:
			case CREX_INIT_NS_FCALL_BY_NAME:
			case CREX_INIT_DYNAMIC_CALL:
			case CREX_NEW:
			case CREX_INIT_USER_CALL:
				call_stack[call] = call_info;
				call_info = NULL;
				call++;
				break;
			case CREX_DO_FCALL:
			case CREX_DO_ICALL:
			case CREX_DO_UCALL:
			case CREX_DO_FCALL_BY_NAME:
			case CREX_CALLABLE_CONVERT:
				func_info->flags |= CREX_FUNC_HAS_CALLS;
				if (call_info) {
					call_info->caller_call_opline = opline;
				}
				call--;
				call_info = call_stack[call];
				break;
			case CREX_SEND_VAL:
			case CREX_SEND_VAR:
			case CREX_SEND_VAL_EX:
			case CREX_SEND_VAR_EX:
			case CREX_SEND_FUNC_ARG:
			case CREX_SEND_REF:
			case CREX_SEND_VAR_NO_REF:
			case CREX_SEND_VAR_NO_REF_EX:
			case CREX_SEND_USER:
				if (call_info) {
					if (opline->op2_type == IS_CONST) {
						call_info->named_args = 1;
						break;
					}

					uint32_t num = opline->op2.num;
					if (num > 0) {
						num--;
					}
					call_info->arg_info[num].opline = opline;
				}
				break;
			case CREX_SEND_ARRAY:
			case CREX_SEND_UNPACK:
				if (call_info) {
					call_info->send_unpack = 1;
				}
				break;
			case CREX_EXIT:
				/* In this case the DO_CALL opcode may have been dropped
				 * and caller_call_opline will be NULL. */
				break;
		}
		opline++;
	}
	free_alloca(call_stack, use_heap);
}

static bool crex_is_indirectly_recursive(crex_op_array *root, crex_op_array *op_array, crex_bitset visited)
{
	crex_func_info *func_info;
	crex_call_info *call_info;
	bool ret = 0;

	if (op_array == root) {
		return 1;
	}

	func_info = CREX_FUNC_INFO(op_array);
	if (crex_bitset_in(visited, func_info->num)) {
		return 0;
	}
	crex_bitset_incl(visited, func_info->num);
	call_info = func_info->caller_info;
	while (call_info) {
		if (crex_is_indirectly_recursive(root, call_info->caller_op_array, visited)) {
			call_info->recursive = 1;
			ret = 1;
		}
		call_info = call_info->next_caller;
	}
	return ret;
}

static void crex_analyze_recursion(crex_call_graph *call_graph)
{
	crex_op_array *op_array;
	crex_func_info *func_info;
	crex_call_info *call_info;
	int i;
	int set_len = crex_bitset_len(call_graph->op_arrays_count);
	crex_bitset visited;
	ALLOCA_FLAG(use_heap);

	visited = CREX_BITSET_ALLOCA(set_len, use_heap);
	for (i = 0; i < call_graph->op_arrays_count; i++) {
		op_array = call_graph->op_arrays[i];
		func_info = call_graph->func_infos + i;
		call_info = func_info->caller_info;
		for (; call_info; call_info = call_info->next_caller) {
			if (call_info->is_prototype) {
				/* Might be calling an overridden child method and not actually recursive. */
				continue;
			}
			if (call_info->caller_op_array == op_array) {
				call_info->recursive = 1;
				func_info->flags |= CREX_FUNC_RECURSIVE | CREX_FUNC_RECURSIVE_DIRECTLY;
			} else {
				memset(visited, 0, sizeof(crex_ulong) * set_len);
				if (crex_is_indirectly_recursive(op_array, call_info->caller_op_array, visited)) {
					call_info->recursive = 1;
					func_info->flags |= CREX_FUNC_RECURSIVE | CREX_FUNC_RECURSIVE_INDIRECTLY;
				}
			}
		}
	}

	free_alloca(visited, use_heap);
}

static void crex_sort_op_arrays(crex_call_graph *call_graph)
{
	(void) call_graph;

	// TODO: perform topological sort of cyclic call graph
}

CREX_API void crex_build_call_graph(crex_arena **arena, crex_script *script, crex_call_graph *call_graph) /* {{{ */
{
	call_graph->op_arrays_count = 0;
	crex_foreach_op_array(script, crex_op_array_calc, call_graph);

	call_graph->op_arrays = (crex_op_array**)crex_arena_calloc(arena, call_graph->op_arrays_count, sizeof(crex_op_array*));
	call_graph->func_infos = (crex_func_info*)crex_arena_calloc(arena, call_graph->op_arrays_count, sizeof(crex_func_info));
	call_graph->op_arrays_count = 0;
	crex_foreach_op_array(script, crex_op_array_collect, call_graph);
}
/* }}} */

CREX_API void crex_analyze_call_graph(crex_arena **arena, crex_script *script, crex_call_graph *call_graph) /* {{{ */
{
	int i;

	for (i = 0; i < call_graph->op_arrays_count; i++) {
		crex_analyze_calls(arena, script, 0, call_graph->op_arrays[i], call_graph->func_infos + i);
	}
	crex_analyze_recursion(call_graph);
	crex_sort_op_arrays(call_graph);
}
/* }}} */

CREX_API crex_call_info **crex_build_call_map(crex_arena **arena, crex_func_info *info, const crex_op_array *op_array) /* {{{ */
{
	crex_call_info **map, *call;
	if (!info->callee_info) {
		/* Don't build call map if function contains no calls */
		return NULL;
	}

	map = crex_arena_calloc(arena, sizeof(crex_call_info *), op_array->last);
	for (call = info->callee_info; call; call = call->next_callee) {
		int i;
		map[call->caller_init_opline - op_array->opcodes] = call;
		if (call->caller_call_opline) {
			map[call->caller_call_opline - op_array->opcodes] = call;
		}
		for (i = 0; i < call->num_args; i++) {
			if (call->arg_info[i].opline) {
				map[call->arg_info[i].opline - op_array->opcodes] = call;
			}
		}
	}
	return map;
}
/* }}} */
