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

#ifndef CREX_CALL_GRAPH_H
#define CREX_CALL_GRAPH_H

#include "crex_ssa.h"
#include "crex_func_info.h"
#include "crex_optimizer.h"

typedef struct _crex_send_arg_info {
	crex_op                *opline;
} crex_send_arg_info;

struct _crex_call_info {
	crex_op_array          *caller_op_array;
	crex_op                *caller_init_opline;
	crex_op                *caller_call_opline;
	crex_function          *callee_func;
	crex_call_info         *next_caller;
	crex_call_info         *next_callee;
	bool               recursive;
	bool               send_unpack;  /* Parameters passed by SEND_UNPACK or SEND_ARRAY */
	bool               named_args;   /* Function has named arguments */
	bool               is_prototype; /* An overridden child method may be called */
	int                     num_args;	/* Number of arguments, excluding named and variadic arguments */
	crex_send_arg_info      arg_info[1];
};

struct _crex_func_info {
	int                     num;
	uint32_t                flags;
	crex_ssa                ssa;          /* Static Single Assignment Form  */
	crex_call_info         *caller_info;  /* where this function is called from */
	crex_call_info         *callee_info;  /* which functions are called from this one */
	crex_call_info        **call_map;     /* Call info associated with init/call/send opnum */
	crex_ssa_var_info       return_info;
};

typedef struct _crex_call_graph {
	int                     op_arrays_count;
	crex_op_array         **op_arrays;
	crex_func_info         *func_infos;
} crex_call_graph;

BEGIN_EXTERN_C()

CREX_API void crex_build_call_graph(crex_arena **arena, crex_script *script, crex_call_graph *call_graph);
CREX_API void crex_analyze_call_graph(crex_arena **arena, crex_script *script, crex_call_graph *call_graph);
CREX_API crex_call_info **crex_build_call_map(crex_arena **arena, crex_func_info *info, const crex_op_array *op_array);
CREX_API void crex_analyze_calls(crex_arena **arena, crex_script *script, uint32_t build_flags, crex_op_array *op_array, crex_func_info *func_info);

END_EXTERN_C()

#endif /* CREX_CALL_GRAPH_H */
