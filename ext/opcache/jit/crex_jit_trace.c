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

static crex_op_array dummy_op_array;
static crex_jit_trace_info *crex_jit_traces = NULL;
static const void **crex_jit_exit_groups = NULL;

#define CREX_JIT_COUNTER_NUM   crex_jit_traces[0].root
#define CREX_JIT_TRACE_NUM     crex_jit_traces[0].id
#define CREX_JIT_EXIT_NUM      crex_jit_traces[0].exit_count
#define CREX_JIT_EXIT_COUNTERS crex_jit_traces[0].exit_counters

#define CREX_JIT_TRACE_STOP_DESCRIPTION(name, description) \
	description,

static const char * crex_jit_trace_stop_description[] = {
	CREX_JIT_TRACE_STOP(CREX_JIT_TRACE_STOP_DESCRIPTION)
};

static crex_always_inline const char *crex_jit_trace_star_desc(uint8_t trace_flags)
{
	if (trace_flags & CREX_JIT_TRACE_START_LOOP) {
		return "loop";
	} else if (trace_flags & CREX_JIT_TRACE_START_ENTER) {
		return "enter";
	} else if (trace_flags & CREX_JIT_TRACE_START_RETURN) {
		return "return";
	} else {
		CREX_UNREACHABLE();
		return "???";
	}
}

static int crex_jit_trace_startup(bool reattached)
{
	if (!reattached) {
		crex_jit_traces = (crex_jit_trace_info*)crex_shared_alloc(sizeof(crex_jit_trace_info) * JIT_G(max_root_traces));
		if (!crex_jit_traces) {
			return FAILURE;
		}
		crex_jit_exit_groups = (const void**)crex_shared_alloc(sizeof(void*) * (CREX_JIT_TRACE_MAX_EXITS/CREX_JIT_EXIT_POINTS_PER_GROUP));
		if (!crex_jit_exit_groups) {
			return FAILURE;
		}
		CREX_JIT_TRACE_NUM = 1;
		CREX_JIT_COUNTER_NUM = 0;
		CREX_JIT_EXIT_NUM = 0;
		CREX_JIT_EXIT_COUNTERS = 0;
		ZCSG(jit_traces) = crex_jit_traces;
		ZCSG(jit_exit_groups) = crex_jit_exit_groups;
		ZCSG(jit_counters_stopped) = false;
	} else {
		crex_jit_traces = ZCSG(jit_traces);
		if (!crex_jit_traces) {
			return FAILURE;
		}
		crex_jit_exit_groups = ZCSG(jit_exit_groups);
		if (!crex_jit_exit_groups) {
			return FAILURE;
		}
	}

	memset(&dummy_op_array, 0, sizeof(dummy_op_array));
	dummy_op_array.fn_flags = CREX_ACC_DONE_PASS_TWO;

	JIT_G(exit_counters) = calloc(JIT_G(max_exit_counters), 1);
	if (JIT_G(exit_counters) == NULL) {
		return FAILURE;
	}

	return SUCCESS;
}

static const void *crex_jit_trace_allocate_exit_group(uint32_t n)
{
	dasm_State* dasm_state = NULL;
	const void *entry;
	char name[32];

	dasm_init(&dasm_state, DASM_MAXSECTION);
	dasm_setupglobal(&dasm_state, dasm_labels, crex_lb_MAX);
	dasm_setup(&dasm_state, dasm_actions);
	crex_jit_align_stub(&dasm_state);
	crex_jit_trace_exit_group_stub(&dasm_state, n);

	sprintf(name, "jit$$trace_exit_%d", n);
	entry = dasm_link_and_encode(&dasm_state, NULL, NULL, NULL, NULL, name, 0, SP_ADJ_JIT, SP_ADJ_NONE);
	dasm_free(&dasm_state);

#ifdef HAVE_DISASM
	if (JIT_G(debug) & CREX_JIT_DEBUG_ASM) {
		uint32_t i;

		for (i = 0; i < CREX_JIT_EXIT_POINTS_PER_GROUP; i++) {
			sprintf(name, "jit$$trace_exit_%d", n + i);
			crex_jit_disasm_add_symbol(name, (uintptr_t)entry + (i * CREX_JIT_EXIT_POINTS_SPACING), CREX_JIT_EXIT_POINTS_SPACING);
		}
	}
#endif

	return entry;
}

static const void *crex_jit_trace_allocate_exit_point(uint32_t n)
{
	const void *group = NULL;

	if (UNEXPECTED(n >= CREX_JIT_TRACE_MAX_EXITS)) {
		return NULL;
	}
	do {
		group = crex_jit_trace_allocate_exit_group(CREX_JIT_EXIT_NUM);
		if (!group) {
			return NULL;
		}
		crex_jit_exit_groups[CREX_JIT_EXIT_NUM / CREX_JIT_EXIT_POINTS_PER_GROUP] =
			group;
		CREX_JIT_EXIT_NUM += CREX_JIT_EXIT_POINTS_PER_GROUP;
	} while (n >= CREX_JIT_EXIT_NUM);
	return (const void*)
		((const char*)group +
		((n % CREX_JIT_EXIT_POINTS_PER_GROUP) * CREX_JIT_EXIT_POINTS_SPACING));
}

static const void *crex_jit_trace_get_exit_addr(uint32_t n)
{
	if (UNEXPECTED(n >= CREX_JIT_EXIT_NUM)) {
		return crex_jit_trace_allocate_exit_point(n);
	}
	return (const void*)
		((const char*)crex_jit_exit_groups[n / CREX_JIT_EXIT_POINTS_PER_GROUP] +
		((n % CREX_JIT_EXIT_POINTS_PER_GROUP) * CREX_JIT_EXIT_POINTS_SPACING));
}

#if CREX_JIT_TARGET_ARM64
static crex_jit_trace_info *crex_jit_get_current_trace_info(void)
{
	return &crex_jit_traces[CREX_JIT_TRACE_NUM];
}

static uint32_t crex_jit_trace_find_exit_point(const void* addr)
{
	uint32_t n = CREX_JIT_EXIT_NUM / CREX_JIT_EXIT_POINTS_PER_GROUP;
	uint32_t i;

	for (i = 0; i < n; i++) {
		if ((const char*)addr >= (const char*)crex_jit_exit_groups[i]
			&& (const char*)addr < (const char*)crex_jit_exit_groups[i] +
				(CREX_JIT_EXIT_POINTS_PER_GROUP * CREX_JIT_EXIT_POINTS_SPACING)) {
			return (i * CREX_JIT_EXIT_POINTS_PER_GROUP) +
				 ((const char*)addr - (const char*)crex_jit_exit_groups[i]) / CREX_JIT_EXIT_POINTS_SPACING;
		}
	}
	return (uint32_t)-1;
}
#endif

static uint32_t crex_jit_trace_get_exit_point(const crex_op *to_opline, uint32_t flags)
{
	crex_jit_trace_info *t = &crex_jit_traces[CREX_JIT_TRACE_NUM];
	uint32_t exit_point;
	const crex_op_array *op_array;
	uint32_t stack_offset = (uint32_t)-1;
	uint32_t stack_size;
	crex_jit_trace_stack *stack = NULL;

	if (delayed_call_chain) {
		assert(to_opline != NULL); /* CALL and IP share the same register */
		flags |= CREX_JIT_EXIT_RESTORE_CALL;
	}
	if (JIT_G(current_frame)) {
		op_array = &JIT_G(current_frame)->func->op_array;
		stack_size = op_array->last_var + op_array->T;
		if (stack_size) {
			stack = JIT_G(current_frame)->stack;
			do {
				if (STACK_TYPE(stack, stack_size-1) != IS_UNKNOWN
				 || STACK_MEM_TYPE(stack, stack_size-1) != IS_UNKNOWN
				 || STACK_REG(stack, stack_size-1) != ZREG_NONE) {
					break;
				}
				stack_size--;
			} while (stack_size);
		}
	} else {
		op_array = NULL;
		stack_size = 0;
	}

	/* Try to reuse exit points */
	if (to_opline != NULL && t->exit_count > 0) {
		uint32_t i = t->exit_count;

		do {
			i--;
			if (stack_size == 0
			 || (t->exit_info[i].stack_size >= stack_size
			  && memcmp(t->stack_map + t->exit_info[i].stack_offset, stack, stack_size * sizeof(crex_jit_trace_stack)) == 0)) {
				stack_offset = t->exit_info[i].stack_offset;
				if (t->exit_info[i].opline == to_opline
				 && t->exit_info[i].flags == flags
				 && t->exit_info[i].stack_size == stack_size) {
					return i;
				}
			}
		} while (i > 0);
	}

	exit_point = t->exit_count;
	if (exit_point < CREX_JIT_TRACE_MAX_EXITS) {
		if (stack_size != 0 && stack_offset == (uint32_t)-1) {
			stack_offset = t->stack_map_size;
			t->stack_map_size += stack_size;
			// TODO: reduce number of reallocations ???
			t->stack_map = erealloc(t->stack_map, t->stack_map_size * sizeof(crex_jit_trace_stack));
			memcpy(t->stack_map + stack_offset, stack, stack_size * sizeof(crex_jit_trace_stack));
		}
		t->exit_count++;
		t->exit_info[exit_point].opline = to_opline;
		t->exit_info[exit_point].op_array = op_array;
		t->exit_info[exit_point].flags = flags;
		t->exit_info[exit_point].stack_size = stack_size;
		t->exit_info[exit_point].stack_offset = stack_offset;
	}

	return exit_point;
}

static void crex_jit_trace_add_code(const void *start, uint32_t size)
{
	crex_jit_trace_info *t = &crex_jit_traces[CREX_JIT_TRACE_NUM];

	t->code_start = start;
	t->code_size  = size;
}

/**
 * Locate a trace in the #crex_jit_traces array with the specified
 * #code_start address.
 *
 * @return the #crex_jit_traces index or 0 if no such #code_start
 * address was found
 */
static uint32_t crex_jit_find_trace(const void *addr)
{
	uint32_t i;

	for (i = 1; i < CREX_JIT_TRACE_NUM; i++) {
		if (crex_jit_traces[i].code_start == addr) {
			return i;
		}
	}
	return 0;
}

static crex_string *crex_jit_trace_name(const crex_op_array *op_array, uint32_t lineno)
{
	smart_str buf = {0};

	smart_str_appends(&buf, TRACE_PREFIX);
	smart_str_append_long(&buf, (crex_long)CREX_JIT_TRACE_NUM);
	smart_str_appendc(&buf, '$');
	if (op_array->function_name) {
		if (op_array->scope) {
			smart_str_appendl(&buf, ZSTR_VAL(op_array->scope->name), ZSTR_LEN(op_array->scope->name));
			smart_str_appends(&buf, "::");
			smart_str_appendl(&buf, ZSTR_VAL(op_array->function_name), ZSTR_LEN(op_array->function_name));
		} else {
			smart_str_appendl(&buf, ZSTR_VAL(op_array->function_name), ZSTR_LEN(op_array->function_name));
		}
	} else if (op_array->filename) {
		smart_str_appendl(&buf, ZSTR_VAL(op_array->filename), ZSTR_LEN(op_array->filename));
	}
	smart_str_appendc(&buf, '$');
	smart_str_append_long(&buf, (crex_long)lineno);
	smart_str_0(&buf);
	return buf.s;
}

static int crex_jit_trace_may_exit(const crex_op_array *op_array, const crex_op *opline)
{
	switch (opline->opcode) {
		case CREX_IS_IDENTICAL:
		case CREX_IS_NOT_IDENTICAL:
		case CREX_IS_EQUAL:
		case CREX_IS_NOT_EQUAL:
		case CREX_IS_SMALLER:
		case CREX_IS_SMALLER_OR_EQUAL:
		case CREX_CASE:
		case CREX_CASE_STRICT:
		case CREX_ISSET_ISEMPTY_CV:
		case CREX_ISSET_ISEMPTY_VAR:
		case CREX_ISSET_ISEMPTY_DIM_OBJ:
		case CREX_ISSET_ISEMPTY_PROP_OBJ:
		case CREX_ISSET_ISEMPTY_STATIC_PROP:
		case CREX_INSTANCEOF:
		case CREX_TYPE_CHECK:
		case CREX_DEFINED:
		case CREX_IN_ARRAY:
		case CREX_ARRAY_KEY_EXISTS:
			if (opline->result_type & (IS_SMART_BRANCH_JMPNZ | IS_SMART_BRANCH_JMPZ)) {
				/* smart branch */
				return 1;
			}
			break;
		case CREX_JMPZ:
		case CREX_JMPNZ:
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
		case CREX_SWITCH_LONG:
		case CREX_SWITCH_STRING:
		case CREX_MATCH:
		case CREX_BIND_INIT_STATIC_OR_JMP:
			/* branch opcodes */
			return 1;
		case CREX_NEW:
			if (opline->extended_value == 0 && (opline+1)->opcode == CREX_DO_FCALL) {
				/* NEW may skip constructor without arguments */
				return 1;
			}
			break;
		case CREX_CATCH:
		case CREX_FAST_CALL:
		case CREX_FAST_RET:
		case CREX_GENERATOR_CREATE:
		case CREX_GENERATOR_RETURN:
		case CREX_EXIT:
		case CREX_YIELD:
		case CREX_YIELD_FROM:
		case CREX_INCLUDE_OR_EVAL:
		case CREX_MATCH_ERROR:
			/* unsupported */
			return 1;
		case CREX_DO_FCALL:
			/* potentially polymorphic call */
			return 1;
#if 0
		case CREX_DO_UCALL:
		case CREX_DO_FCALL_BY_NAME:
			/* monomorphic call */
			// TODO: recompilation may change target ???
			return 0;
#endif
		case CREX_RETURN_BY_REF:
		case CREX_RETURN:
			/* return */
			return !JIT_G(current_frame) || TRACE_FRAME_IS_UNKNOWN_RETURN(JIT_G(current_frame));
		default:
			break;
	}
	return 0;
}

static crex_always_inline uint32_t crex_jit_trace_type_to_info_ex(uint8_t type, uint32_t info)
{
	if (type == IS_UNKNOWN) {
		return info;
	}
	CREX_ASSERT(info & (1 << type));
	if (type < IS_STRING) {
		return (1 << type);
	} else if (type != IS_ARRAY) {
		return (1 << type) | (info & (MAY_BE_RC1|MAY_BE_RCN));
	} else {
		return MAY_BE_ARRAY | (info & (MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF|MAY_BE_ARRAY_KEY_ANY|MAY_BE_RC1|MAY_BE_RCN));
	}
}

static crex_always_inline uint32_t crex_jit_trace_type_to_info(uint8_t type)
{
	return crex_jit_trace_type_to_info_ex(type, -1);
}

static crex_always_inline crex_ssa_alias_kind crex_jit_var_may_alias(const crex_op_array *op_array, const crex_ssa *ssa, uint32_t var)
{
	if (var >= op_array->last_var) {
		return NO_ALIAS;
	} else if ((!op_array->function_name || (ssa->cfg.flags & CREX_FUNC_INDIRECT_VAR_ACCESS))) {
		return SYMTABLE_ALIAS;
	} else if (ssa->vars) {
		return ssa->vars[var].alias;
	} else if (crex_string_equals_literal(op_array->vars[var], "http_response_header")) {
		return HTTP_RESPONSE_HEADER_ALIAS;
	}
	return NO_ALIAS;
}

static crex_always_inline void crex_jit_trace_add_op_guard(crex_ssa             *tssa,
                                                           int                   ssa_var,
                                                           uint8_t               op_type)
{
	crex_ssa_var_info *info = &tssa->var_info[ssa_var];

	if ((info->type & (MAY_BE_ANY|MAY_BE_UNDEF)) != (1 << op_type)) {
		if (UNEXPECTED(tssa->vars[ssa_var].alias != NO_ALIAS)) {
			info->type |= MAY_BE_GUARD;
		} else {
			info->type = MAY_BE_GUARD | crex_jit_trace_type_to_info_ex(op_type, info->type);
		}
	}
}

#define ADD_OP_GUARD(_ssa_var, _op_type) do { \
		if (_ssa_var >= 0 && _op_type != IS_UNKNOWN) { \
			crex_jit_trace_add_op_guard(tssa, _ssa_var, _op_type); \
		} \
	} while (0)

#define CHECK_OP_TRACE_TYPE(_var, _ssa_var, op_info, op_type) do { \
		if (op_type != IS_UNKNOWN) { \
			if ((op_info & MAY_BE_GUARD) != 0) { \
				if (!crex_jit_type_guard(&dasm_state, opline, _var, op_type)) { \
					goto jit_failure; \
				} \
				if (ssa->vars[_ssa_var].alias != NO_ALIAS) { \
					SET_STACK_TYPE(stack, EX_VAR_TO_NUM(_var), IS_UNKNOWN, 1); \
					op_info = crex_jit_trace_type_to_info(op_type); \
				} else { \
					SET_STACK_TYPE(stack, EX_VAR_TO_NUM(_var), op_type, 1); \
					op_info &= ~MAY_BE_GUARD; \
					ssa->var_info[_ssa_var].type &= op_info; \
				} \
			} \
		} \
	} while (0)

#define ADD_OP1_TRACE_GUARD() \
	ADD_OP_GUARD(tssa->ops[idx].op1_use, op1_type)
#define ADD_OP2_TRACE_GUARD() \
	ADD_OP_GUARD(tssa->ops[idx].op2_use, op2_type)
#define ADD_OP1_DATA_TRACE_GUARD() \
	ADD_OP_GUARD(tssa->ops[idx+1].op1_use, op3_type)

#define CHECK_OP1_TRACE_TYPE() \
	CHECK_OP_TRACE_TYPE(opline->op1.var, ssa_op->op1_use, op1_info, op1_type)
#define CHECK_OP2_TRACE_TYPE() \
	CHECK_OP_TRACE_TYPE(opline->op2.var, ssa_op->op2_use, op2_info, op2_type)
#define CHECK_OP1_DATA_TRACE_TYPE() \
	CHECK_OP_TRACE_TYPE((opline+1)->op1.var, (ssa_op+1)->op1_use, op1_data_info, op3_type)

static crex_always_inline size_t crex_jit_trace_frame_size(const crex_op_array *op_array)
{
	if (op_array && op_array->type == CREX_USER_FUNCTION) {
		return CREX_MM_ALIGNED_SIZE(offsetof(crex_jit_trace_stack_frame, stack) + CREX_MM_ALIGNED_SIZE((op_array->last_var + op_array->T) * sizeof(crex_jit_trace_stack)));
	} else if (op_array) {
		return CREX_MM_ALIGNED_SIZE(offsetof(crex_jit_trace_stack_frame, stack) + CREX_MM_ALIGNED_SIZE(op_array->num_args * sizeof(crex_jit_trace_stack)));
	} else {
		return CREX_MM_ALIGNED_SIZE(offsetof(crex_jit_trace_stack_frame, stack));
	}
}

static crex_jit_trace_stack_frame* crex_jit_trace_call_frame(crex_jit_trace_stack_frame *frame, const crex_op_array *op_array)
{
	return (crex_jit_trace_stack_frame*)((char*)frame + crex_jit_trace_frame_size(op_array));
}

static crex_jit_trace_stack_frame* crex_jit_trace_ret_frame(crex_jit_trace_stack_frame *frame, const crex_op_array *op_array)
{
	return (crex_jit_trace_stack_frame*)((char*)frame - crex_jit_trace_frame_size(op_array));
}

static void crex_jit_trace_send_type(const crex_op *opline, crex_jit_trace_stack_frame *call, uint8_t type)
{
	crex_jit_trace_stack *stack = call->stack;
	const crex_op_array *op_array = &call->func->op_array;
	uint32_t arg_num = opline->op2.num;

	if (arg_num > op_array->num_args) {
		return;
	}
	if (op_array->fn_flags & CREX_ACC_HAS_TYPE_HINTS) {
		crex_arg_info *arg_info;

		CREX_ASSERT(arg_num <= op_array->num_args);
		arg_info = &op_array->arg_info[arg_num-1];

		if (CREX_TYPE_IS_SET(arg_info->type)) {
			if (!(CREX_TYPE_FULL_MASK(arg_info->type) & (1u << type))) {
				return;
			}
		}
	}
	SET_STACK_TYPE(stack, EX_VAR_TO_NUM(opline->result.var), type, 1);
}

static bool crex_jit_needs_arg_dtor(const crex_function *func, uint32_t arg_num, crex_call_info *call_info)
{
	if (func
	 && func->type == CREX_INTERNAL_FUNCTION
	 && (func->internal_function.fn_flags & CREX_ACC_HAS_TYPE_HINTS) != 0
	 && arg_num < func->internal_function.num_args) {
		const crex_internal_arg_info *arg_info = &func->internal_function.arg_info[arg_num];

		if (CREX_ARG_SEND_MODE(arg_info) == CREX_SEND_BY_VAL
		 && CREX_TYPE_IS_SET(arg_info->type)
		 && (CREX_TYPE_FULL_MASK(arg_info->type) & MAY_BE_ANY) != MAY_BE_ANY) {
			if (JIT_G(trigger) == CREX_JIT_ON_HOT_TRACE
			 && JIT_G(current_frame)
			 && JIT_G(current_frame)->call
			 && JIT_G(current_frame)->call->func) {
				uint32_t type = STACK_TYPE(JIT_G(current_frame)->call->stack, arg_num);

				if (type != IS_UNKNOWN
				 && type < IS_STRING
				 && CREX_TYPE_FULL_MASK(arg_info->type) & (1u << type)) {
					return 0;
				}
			}
			if (call_info && arg_num < call_info->num_args && call_info->arg_info[arg_num].opline) {
				const crex_op *opline = call_info->arg_info[arg_num].opline;

				if (opline->opcode == CREX_SEND_VAL && opline->op1_type == IS_CONST) {
					zval *zv = RT_CONSTANT(opline, opline->op1);

					if (!C_REFCOUNTED_P(zv)) {
						uint32_t type = C_TYPE_P(zv);

						// TODO: few functions (e.g. pcntl_exec) modify arrays in-place ???
						if (type != IS_ARRAY
						 && (CREX_TYPE_FULL_MASK(arg_info->type) & (1u << type))) {
							return 0;
						}
					}
				}
			}
		}
	}

	return 1;
}

static crex_ssa *crex_jit_trace_build_ssa(const crex_op_array *op_array, crex_script *script)
{
	crex_jit_op_array_trace_extension *jit_extension;
	crex_ssa *ssa;

	jit_extension =
		(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
	jit_extension->func_info.num = 0;
	jit_extension->func_info.flags &= CREX_FUNC_JIT_ON_FIRST_EXEC
		| CREX_FUNC_JIT_ON_PROF_REQUEST
		| CREX_FUNC_JIT_ON_HOT_COUNTERS
		| CREX_FUNC_JIT_ON_HOT_TRACE;
	memset(&jit_extension->func_info.ssa, 0, sizeof(crex_func_info) - offsetof(crex_func_info, ssa));
	ssa = &jit_extension->func_info.ssa;

	if (JIT_G(opt_level) >= CREX_JIT_LEVEL_OPT_FUNC) {
		do {
			if (crex_jit_op_array_analyze1(op_array, script, ssa) != SUCCESS) {
				break;
			}

			if (JIT_G(opt_level) >= CREX_JIT_LEVEL_OPT_FUNCS) {
				crex_analyze_calls(&CG(arena), script, CREX_CALL_TREE, (crex_op_array*)op_array, &jit_extension->func_info);
				jit_extension->func_info.call_map = crex_build_call_map(&CG(arena), &jit_extension->func_info, op_array);
				if (op_array->fn_flags & CREX_ACC_HAS_RETURN_TYPE) {
					crex_init_func_return_info(op_array, script, &jit_extension->func_info.return_info);
				}
			}

			if (crex_jit_op_array_analyze2(op_array, script, ssa, 0) != SUCCESS) {
				break;
			}

			if (JIT_G(debug) & CREX_JIT_DEBUG_SSA) {
				crex_dump_op_array(op_array, CREX_DUMP_HIDE_UNREACHABLE|CREX_DUMP_RC_INFERENCE|CREX_DUMP_SSA, "JIT", ssa);
			}
			return ssa;
		} while (0);
	}

	memset(ssa, 0, sizeof(crex_ssa));
	ssa->cfg.blocks_count = 1;

	if (JIT_G(opt_level) == CREX_JIT_LEVEL_INLINE) {
		crex_cfg cfg;
		void *checkpoint = crex_arena_checkpoint(CG(arena));

		if (crex_jit_build_cfg(op_array, &cfg) == SUCCESS) {
			ssa->cfg.flags = cfg.flags;
		} else{
			ssa->cfg.flags |= CREX_FUNC_INDIRECT_VAR_ACCESS;
		}

		/* TODO: move this to crex_cfg.c ? */
		if (!op_array->function_name) {
			ssa->cfg.flags |= CREX_FUNC_INDIRECT_VAR_ACCESS;
		}

		crex_arena_release(&CG(arena), checkpoint);
	}

	return ssa;
}

static void crex_jit_dump_trace(crex_jit_trace_rec *trace_buffer, crex_ssa *tssa);
static void crex_jit_dump_exit_info(crex_jit_trace_info *t);

static crex_always_inline int crex_jit_trace_op_len(const crex_op *opline)
{
	int len;

	switch (opline->opcode) {
		case CREX_ASSIGN_DIM:
		case CREX_ASSIGN_OBJ:
		case CREX_ASSIGN_STATIC_PROP:
		case CREX_ASSIGN_DIM_OP:
		case CREX_ASSIGN_OBJ_OP:
		case CREX_ASSIGN_STATIC_PROP_OP:
		case CREX_ASSIGN_OBJ_REF:
		case CREX_ASSIGN_STATIC_PROP_REF:
			return 2; /* OP_DATA */
		case CREX_RECV_INIT:
			len = 1;
			opline++;
			while (opline->opcode == CREX_RECV_INIT) {
				len++;
				opline++;
			}
			return len;
		case CREX_BIND_GLOBAL:
			len = 1;
			opline++;
			while (opline->opcode == CREX_BIND_GLOBAL) {
				len++;
				opline++;
			}
			return len;
//		case CREX_IS_IDENTICAL:
//		case CREX_IS_NOT_IDENTICAL:
//		case CREX_IS_EQUAL:
//		case CREX_IS_NOT_EQUAL:
//		case CREX_IS_SMALLER:
//		case CREX_IS_SMALLER_OR_EQUAL:
//		case CREX_CASE:
//		case CREX_ISSET_ISEMPTY_CV:
//		case CREX_ISSET_ISEMPTY_VAR:
//		case CREX_ISSET_ISEMPTY_DIM_OBJ:
//		case CREX_ISSET_ISEMPTY_PROP_OBJ:
//		case CREX_ISSET_ISEMPTY_STATIC_PROP:
//		case CREX_INSTANCEOF:
//		case CREX_TYPE_CHECK:
//		case CREX_DEFINED:
//		case CREX_IN_ARRAY:
//		case CREX_ARRAY_KEY_EXISTS:
		default:
			if ((opline->result_type & (IS_SMART_BRANCH_JMPZ|IS_SMART_BRANCH_JMPNZ)) != 0) {
				return 2; /* JMPZ/JMPNZ */
			}
			return 1;
	}
}

static int crex_jit_trace_add_phis(crex_jit_trace_rec *trace_buffer, uint32_t ssa_vars_count, crex_ssa *tssa, crex_jit_trace_stack *stack)
{
	const crex_op_array *op_array;
	crex_jit_trace_rec *p;
	int k, vars_count;
	crex_bitset use, def;
	uint32_t build_flags = CREX_SSA_RC_INFERENCE | CREX_SSA_USE_CV_RESULTS;
	uint32_t set_size;
	crex_ssa_phi *prev = NULL;
	int level = 0;
	ALLOCA_FLAG(use_heap);

	op_array = trace_buffer->op_array;
	set_size = crex_bitset_len(op_array->last_var + op_array->T);
	use = CREX_BITSET_ALLOCA(set_size * 2, use_heap);
	memset(use, 0, set_size * 2 * CREX_BITSET_ELM_SIZE);
	def = use + set_size;
	p = trace_buffer + CREX_JIT_TRACE_START_REC_SIZE;
	for (;;p++) {
		if (p->op == CREX_JIT_TRACE_VM && level == 0) {
			const crex_op *opline = p->opline;
			int len;

			crex_dfg_add_use_def_op(op_array, opline, build_flags, use, def);
			len = crex_jit_trace_op_len(opline);
			while (len > 1) {
				opline++;
				if (opline->opcode != CREX_OP_DATA) {
					crex_dfg_add_use_def_op(op_array, opline, build_flags, use, def);
				}
				len--;
			}
		} else if (p->op == CREX_JIT_TRACE_INIT_CALL) {
		} else if (p->op == CREX_JIT_TRACE_DO_ICALL) {
		} else if (p->op == CREX_JIT_TRACE_ENTER) {
			level++;
		} else if (p->op == CREX_JIT_TRACE_BACK) {
			if (level == 0) {
				// Phi for recursive calls and returns are not supported yet ???
				assert(0);
			} else {
				level--;
			}
		} else if (p->op == CREX_JIT_TRACE_END) {
			break;
		}
	}

	crex_bitset_intersection(use, def, set_size);

	if (trace_buffer->start == CREX_JIT_TRACE_START_ENTER) {
		vars_count = op_array->last_var;
	} else {
		vars_count = op_array->last_var + op_array->T;
	}
	for (k = 0; k < vars_count; k++) {
		if (crex_bitset_in(use, k)) {
			crex_ssa_phi *phi = crex_arena_calloc(&CG(arena), 1,
				CREX_MM_ALIGNED_SIZE(sizeof(crex_ssa_phi)) +
				CREX_MM_ALIGNED_SIZE(sizeof(int) * 2) +
				sizeof(void*) * 2);
			phi->sources = (int*)(((char*)phi) + CREX_MM_ALIGNED_SIZE(sizeof(crex_ssa_phi)));
			phi->sources[0] = STACK_VAR(stack, k);
			phi->sources[1] = -1;
			phi->use_chains = (crex_ssa_phi**)(((char*)phi->sources) + CREX_MM_ALIGNED_SIZE(sizeof(int) * 2));
			phi->pi = -1;
			phi->var = k;
			phi->ssa_var = ssa_vars_count;
			SET_STACK_VAR(stack, k, ssa_vars_count);
			ssa_vars_count++;
			phi->block = 1;
			if (prev) {
				prev->next = phi;
			} else {
				tssa->blocks[1].phis = phi;
			}
			prev = phi;
		}
	}

	free_alloca(use, use_heap);

	return ssa_vars_count;
}

static int crex_jit_trace_add_call_phis(crex_jit_trace_rec *trace_buffer, uint32_t ssa_vars_count, crex_ssa *tssa, crex_jit_trace_stack *stack)
{
	crex_ssa_phi *prev = NULL;
	const crex_op_array *op_array = trace_buffer->op_array;
	const crex_op *opline = trace_buffer[1].opline;
	int count = opline - op_array->opcodes;
	int i;

	for(i = 0; i < count; i++) {
		crex_ssa_phi *phi = crex_arena_calloc(&CG(arena), 1,
			CREX_MM_ALIGNED_SIZE(sizeof(crex_ssa_phi)) +
			CREX_MM_ALIGNED_SIZE(sizeof(int) * 2) +
			sizeof(void*) * 2);
		phi->sources = (int*)(((char*)phi) + CREX_MM_ALIGNED_SIZE(sizeof(crex_ssa_phi)));
		phi->sources[0] = STACK_VAR(stack, i);
		phi->sources[1] = -1;
		phi->use_chains = (crex_ssa_phi**)(((char*)phi->sources) + CREX_MM_ALIGNED_SIZE(sizeof(int) * 2));
		phi->pi = -1;
		phi->var = i;
		phi->ssa_var = ssa_vars_count;
		SET_STACK_VAR(stack, i, ssa_vars_count);
		ssa_vars_count++;
		phi->block = 1;
		if (prev) {
			prev->next = phi;
		} else {
			tssa->blocks[1].phis = phi;
		}
		prev = phi;
	}
	return ssa_vars_count;
}

static int crex_jit_trace_add_ret_phis(crex_jit_trace_rec *trace_buffer, uint32_t ssa_vars_count, crex_ssa *tssa, crex_jit_trace_stack *stack)
{
	const crex_op *opline = trace_buffer[1].opline - 1;
	int i;

	if (RETURN_VALUE_USED(opline)) {
		crex_ssa_phi *phi = crex_arena_calloc(&CG(arena), 1,
			CREX_MM_ALIGNED_SIZE(sizeof(crex_ssa_phi)) +
			CREX_MM_ALIGNED_SIZE(sizeof(int) * 2) +
			sizeof(void*) * 2);

		i = EX_VAR_TO_NUM(opline->result.var);
		phi->sources = (int*)(((char*)phi) + CREX_MM_ALIGNED_SIZE(sizeof(crex_ssa_phi)));
		phi->sources[0] = STACK_VAR(stack, i);
		phi->sources[1] = -1;
		phi->use_chains = (crex_ssa_phi**)(((char*)phi->sources) + CREX_MM_ALIGNED_SIZE(sizeof(int) * 2));
		phi->pi = -1;
		phi->var = i;
		phi->ssa_var = ssa_vars_count;
		SET_STACK_VAR(stack, i, ssa_vars_count);
		ssa_vars_count++;
		phi->block = 1;
		tssa->blocks[1].phis = phi;
	}
	return ssa_vars_count;
}

static int crex_jit_trace_copy_ssa_var_info(const crex_op_array *op_array, const crex_ssa *ssa, const crex_op **tssa_opcodes, crex_ssa *tssa, int ssa_var)
{
	int var, use;
	crex_ssa_op *op;
	crex_ssa_var_info *info;
	unsigned int no_val;
	crex_ssa_alias_kind alias;

	if (tssa->vars[ssa_var].phi_use_chain) {
		// TODO: this may be incorrect ???
		var = tssa->vars[ssa_var].phi_use_chain->ssa_var;
	} else {
		var = ssa_var;
	}
	use = tssa->vars[var].use_chain;
	if (use >= 0) {
		CREX_ASSERT((tssa_opcodes[use] - op_array->opcodes) < op_array->last);
		op = ssa->ops + (tssa_opcodes[use] - op_array->opcodes);
		if (tssa->ops[use].op1_use == var) {
			no_val = ssa->vars[op->op1_use].no_val;
			alias = ssa->vars[op->op1_use].alias;
			info = ssa->var_info + op->op1_use;
		} else if (tssa->ops[use].op2_use == var) {
			no_val = ssa->vars[op->op2_use].no_val;
			alias = ssa->vars[op->op2_use].alias;
			info = ssa->var_info + op->op2_use;
		} else if (tssa->ops[use].result_use == var) {
			no_val = ssa->vars[op->result_use].no_val;
			alias = ssa->vars[op->result_use].alias;
			info = ssa->var_info + op->result_use;
		} else {
			assert(0);
			return 0;
		}
		tssa->vars[ssa_var].no_val = no_val;
		tssa->vars[ssa_var].alias = alias;
		memcpy(&tssa->var_info[ssa_var], info, sizeof(crex_ssa_var_info));
		return 1;
	}
	return 0;
}

static void crex_jit_trace_propagate_range(const crex_op_array *op_array, const crex_op **tssa_opcodes, crex_ssa *tssa, int ssa_var)
{
	crex_ssa_range tmp;
	int def = tssa->vars[ssa_var].definition;

	if (tssa->vars[ssa_var].alias == NO_ALIAS
	 && crex_inference_propagate_range(op_array, tssa, tssa_opcodes[def], &tssa->ops[def], ssa_var, &tmp)) {
		tssa->var_info[ssa_var].range.min = tmp.min;
		tssa->var_info[ssa_var].range.max = tmp.max;
		tssa->var_info[ssa_var].range.underflow = tmp.underflow;
		tssa->var_info[ssa_var].range.overflow = tmp.overflow;
		tssa->var_info[ssa_var].has_range = 1;
	}
}

static void crex_jit_trace_copy_ssa_var_range(const crex_op_array *op_array, const crex_ssa *ssa, const crex_op **tssa_opcodes, crex_ssa *tssa, int ssa_var)
{
	int def;
	crex_ssa_op *op;
	crex_ssa_var_info *info;
	unsigned int no_val;
	crex_ssa_alias_kind alias;

	def = tssa->vars[ssa_var].definition;
	if (def >= 0) {
		CREX_ASSERT((tssa_opcodes[def] - op_array->opcodes) < op_array->last);
		op = ssa->ops + (tssa_opcodes[def] - op_array->opcodes);
		if (tssa->ops[def].op1_def == ssa_var) {
			no_val = ssa->vars[op->op1_def].no_val;
			alias = ssa->vars[op->op1_def].alias;
			info = ssa->var_info + op->op1_def;
		} else if (tssa->ops[def].op2_def == ssa_var) {
			no_val = ssa->vars[op->op2_def].no_val;
			alias = ssa->vars[op->op2_def].alias;
			info = ssa->var_info + op->op2_def;
		} else if (tssa->ops[def].result_def == ssa_var) {
			no_val = ssa->vars[op->result_def].no_val;
			alias = ssa->vars[op->result_def].alias;
			info = ssa->var_info + op->result_def;
		} else {
			assert(0);
			return;
		}

		tssa->vars[ssa_var].no_val = no_val;
		tssa->vars[ssa_var].alias = alias;

		if (!(info->type & MAY_BE_REF)) {
			crex_jit_trace_propagate_range(op_array, tssa_opcodes, tssa, ssa_var);
		}

		if (info->has_range) {
			if (tssa->var_info[ssa_var].has_range) {
				tssa->var_info[ssa_var].range.min = MAX(tssa->var_info[ssa_var].range.min, info->range.min);
				tssa->var_info[ssa_var].range.max = MIN(tssa->var_info[ssa_var].range.max, info->range.max);
				tssa->var_info[ssa_var].range.underflow = tssa->var_info[ssa_var].range.underflow && info->range.underflow;
				tssa->var_info[ssa_var].range.overflow = tssa->var_info[ssa_var].range.overflow && info->range.overflow;
			} else {
				tssa->var_info[ssa_var].has_range = 1;
				tssa->var_info[ssa_var].range = info->range;
			}
		}
	}
}

static int crex_jit_trace_restrict_ssa_var_info(const crex_op_array *op_array, const crex_ssa *ssa, const crex_op **tssa_opcodes, crex_ssa *tssa, int ssa_var)
{
	int def;
	crex_ssa_op *op;
	crex_ssa_var_info *info;

	def = tssa->vars[ssa_var].definition;
	if (def >= 0) {
		CREX_ASSERT((tssa_opcodes[def] - op_array->opcodes) < op_array->last);
		op = ssa->ops + (tssa_opcodes[def] - op_array->opcodes);
		if (tssa->ops[def].op1_def == ssa_var) {
			info = ssa->var_info + op->op1_def;
		} else if (tssa->ops[def].op2_def == ssa_var) {
			info = ssa->var_info + op->op2_def;
		} else if (tssa->ops[def].result_def == ssa_var) {
			info = ssa->var_info + op->result_def;
		} else {
			assert(0);
			return 0;
		}
		tssa->var_info[ssa_var].type &= info->type;
		if (info->ce) {
			if (tssa->var_info[ssa_var].ce) {
				if (tssa->var_info[ssa_var].ce != info->ce) {
					if (instanceof_function(tssa->var_info[ssa_var].ce, info->ce)) {
						/* everything fine */
					} else if (instanceof_function(info->ce, tssa->var_info[ssa_var].ce)) {
						// TODO: TSSA may miss Pi() functions and corresponding instanceof() constraints ???
					} else {
						// TODO: classes may implement the same interface ???
						//CREX_UNREACHABLE();
					}
				}
				tssa->var_info[ssa_var].is_instanceof =
					tssa->var_info[ssa_var].is_instanceof && info->is_instanceof;
			} else {
				tssa->var_info[ssa_var].ce = info->ce;
				tssa->var_info[ssa_var].is_instanceof = info->is_instanceof;
			}
		}
		if (info->has_range) {
			if (tssa->var_info[ssa_var].has_range) {
				tssa->var_info[ssa_var].range.min = MAX(tssa->var_info[ssa_var].range.min, info->range.min);
				tssa->var_info[ssa_var].range.max = MIN(tssa->var_info[ssa_var].range.max, info->range.max);
				tssa->var_info[ssa_var].range.underflow = tssa->var_info[ssa_var].range.underflow && info->range.underflow;
				tssa->var_info[ssa_var].range.overflow = tssa->var_info[ssa_var].range.overflow && info->range.overflow;
			} else {
				tssa->var_info[ssa_var].has_range = 1;
				tssa->var_info[ssa_var].range = info->range;
			}
		}
		return 1;
	}
	return 0;
}

static int find_return_ssa_var(crex_jit_trace_rec *p, crex_ssa_op *ssa_op)
{
	while (1) {
		if (p->op == CREX_JIT_TRACE_VM) {
			if (p->opline->opcode == CREX_DO_UCALL
			 || p->opline->opcode == CREX_DO_FCALL_BY_NAME
			 || p->opline->opcode == CREX_DO_FCALL) {
				if (p->opline->result_type != IS_UNUSED) {
					return ssa_op->result_def;
				}
			}
			return -1;
		} else if (p->op >= CREX_JIT_TRACE_OP1_TYPE && p->op <= CREX_JIT_TRACE_VAL_INFO) {
			/*skip */
		} else {
			return -1;
		}
		p--;
	}
}

static const crex_op *crex_jit_trace_find_init_fcall_op(crex_jit_trace_rec *p, const crex_op_array *op_array)
{
	if (!(p->info & CREX_JIT_TRACE_FAKE_INIT_CALL)) {
		p--;
		while (1) {
			if (p->op == CREX_JIT_TRACE_VM) {
				if (p->opline->opcode == CREX_INIT_FCALL
				 || p->opline->opcode == CREX_INIT_FCALL_BY_NAME
				 || p->opline->opcode == CREX_INIT_NS_FCALL_BY_NAME
				 || p->opline->opcode == CREX_INIT_DYNAMIC_CALL
				 || p->opline->opcode == CREX_INIT_USER_CALL
				 || p->opline->opcode == CREX_NEW
				 || p->opline->opcode == CREX_INIT_METHOD_CALL
				 || p->opline->opcode == CREX_INIT_STATIC_METHOD_CALL) {
					return p->opline;
				}
				return NULL;
			} else if (p->op >= CREX_JIT_TRACE_OP1_TYPE && p->op <= CREX_JIT_TRACE_VAL_INFO) {
				/*skip */
			} else {
				return NULL;
			}
			p--;
		}
	} else {
		const crex_op *opline = NULL;
		int call_level = 0;

		p++;
		while (1) {
			if (p->op == CREX_JIT_TRACE_VM) {
				opline = p->opline;
				break;
			} else if (p->op == CREX_JIT_TRACE_INIT_CALL) {
				call_level++;
				/*skip */
			} else {
				return NULL;
			}
			p--;
		}
		if (opline) {
			while (opline > op_array->opcodes) {
				opline--;
				switch (opline->opcode) {
					case CREX_INIT_FCALL:
					case CREX_INIT_FCALL_BY_NAME:
					case CREX_INIT_NS_FCALL_BY_NAME:
					case CREX_INIT_METHOD_CALL:
					case CREX_INIT_DYNAMIC_CALL:
					case CREX_INIT_STATIC_METHOD_CALL:
					case CREX_INIT_USER_CALL:
					case CREX_NEW:
						if (call_level == 0) {
							return opline;
						}
						call_level--;
						break;
					case CREX_DO_FCALL:
					case CREX_DO_ICALL:
					case CREX_DO_UCALL:
					case CREX_DO_FCALL_BY_NAME:
					case CREX_CALLABLE_CONVERT:
						call_level++;
						break;
				}
			}
		}
	}
	return NULL;
}

static int is_checked_guard(const crex_ssa *tssa, const crex_op **ssa_opcodes, uint32_t var, uint32_t phi_var)
{
	if ((tssa->var_info[phi_var].type & MAY_BE_ANY) == MAY_BE_LONG
	 && !(tssa->var_info[var].type & MAY_BE_REF)) {
		int idx = tssa->vars[var].definition;

		if (idx >= 0) {
			if (tssa->ops[idx].op1_def == var) {
				const crex_op *opline = ssa_opcodes[idx];
				if (opline->opcode == CREX_PRE_DEC
				 || opline->opcode == CREX_PRE_INC
				 || opline->opcode == CREX_POST_DEC
				 || opline->opcode == CREX_POST_INC) {
					if (tssa->ops[idx].op1_use >= 0
					 && (tssa->var_info[tssa->ops[idx].op1_use].type & MAY_BE_STRING)) {
						return 0;
					}
					if (!(tssa->var_info[tssa->ops[idx].op1_use].type & (MAY_BE_LONG|MAY_BE_DOUBLE))) {
						return 0;
					}
					return 1;
				} else if (opline->opcode == CREX_ASSIGN_OP
				 && (opline->extended_value == CREX_ADD
				  || opline->extended_value == CREX_SUB
				  || opline->extended_value == CREX_MUL)) {
					if ((opline->op2_type & (IS_VAR|IS_CV))
					  && tssa->ops[idx].op2_use >= 0
					  && (tssa->var_info[tssa->ops[idx].op2_use].type & MAY_BE_REF)) {
						return 0;
					}
					if (!(tssa->var_info[tssa->ops[idx].op1_use].type & (MAY_BE_LONG|MAY_BE_DOUBLE))) {
						return 0;
					}
					if (opline->op2_type == IS_CONST) {
						zval *zv = RT_CONSTANT(opline, opline->op2);
						if (C_TYPE_P(zv) != IS_LONG && C_TYPE_P(zv) != IS_DOUBLE) {
							return 0;
						}
					} else if (!(tssa->var_info[tssa->ops[idx].op2_use].type & (MAY_BE_LONG|MAY_BE_DOUBLE))) {
						return 0;
					}
					return 1;
				}
			}
			if (tssa->ops[idx].result_def == var) {
				const crex_op *opline = ssa_opcodes[idx];
				if (opline->opcode == CREX_ADD
				 || opline->opcode == CREX_SUB
				 || opline->opcode == CREX_MUL) {
					if ((opline->op1_type & (IS_VAR|IS_CV))
					  && tssa->ops[idx].op1_use >= 0
					  && (tssa->var_info[tssa->ops[idx].op1_use].type & MAY_BE_REF)) {
						return 0;
					}
					if ((opline->op2_type & (IS_VAR|IS_CV))
					  && tssa->ops[idx].op2_use >= 0
					  && (tssa->var_info[tssa->ops[idx].op2_use].type & MAY_BE_REF)) {
						return 0;
					}
					if (opline->op1_type == IS_CONST) {
						zval *zv = RT_CONSTANT(opline, opline->op1);
						if (C_TYPE_P(zv) != IS_LONG && C_TYPE_P(zv) != IS_DOUBLE) {
							return 0;
						}
					} else if (!(tssa->var_info[tssa->ops[idx].op1_use].type & (MAY_BE_LONG|MAY_BE_DOUBLE))) {
						return 0;
					}
					if (opline->op2_type == IS_CONST) {
						zval *zv = RT_CONSTANT(opline, opline->op2);
						if (C_TYPE_P(zv) != IS_LONG && C_TYPE_P(zv) != IS_DOUBLE) {
							return 0;
						}
					} else if (!(tssa->var_info[tssa->ops[idx].op2_use].type & (MAY_BE_LONG|MAY_BE_DOUBLE))) {
						return 0;
					}
					return 1;
				} else if (opline->opcode == CREX_PRE_DEC
				 || opline->opcode == CREX_PRE_INC
				 || opline->opcode == CREX_POST_DEC
				 || opline->opcode == CREX_POST_INC) {
					if ((opline->op1_type & (IS_VAR|IS_CV))
					  && tssa->ops[idx].op1_use >= 0
					  && (tssa->var_info[tssa->ops[idx].op1_use].type & MAY_BE_REF)) {
						return 0;
					}
					if (!(tssa->var_info[tssa->ops[idx].op1_use].type & (MAY_BE_LONG|MAY_BE_DOUBLE))) {
						return 0;
					}
					return 1;
				}
			}
		}
	}
	return 0;
}

typedef struct _crex_tssa {
	crex_ssa        ssa;
	const crex_op **tssa_opcodes;
	int             used_stack;
} crex_tssa;

static const crex_op _nop_opcode = {0};

static crex_ssa *crex_jit_trace_build_tssa(crex_jit_trace_rec *trace_buffer, uint32_t parent_trace, uint32_t exit_num, crex_script *script, const crex_op_array **op_arrays, int *num_op_arrays_ptr)
{
	crex_ssa *tssa;
	crex_ssa_op *ssa_ops, *op;
	crex_ssa_var *ssa_vars;
	crex_ssa_var_info *ssa_var_info;
	const crex_op_array *op_array;
	const crex_op *opline;
	const crex_op **ssa_opcodes;
	crex_jit_trace_rec *p;
	int i, v, idx, len, ssa_ops_count, vars_count, ssa_vars_count;
	crex_jit_trace_stack *stack;
	uint32_t build_flags = CREX_SSA_RC_INFERENCE | CREX_SSA_USE_CV_RESULTS;
	uint32_t optimization_level = 0;
	int call_level, level, num_op_arrays, used_stack, max_used_stack;
	size_t frame_size, stack_top, stack_size, stack_bottom;
	crex_jit_op_array_trace_extension *jit_extension;
	crex_ssa *ssa;
	crex_jit_trace_stack_frame *frame, *top, *call;
	crex_ssa_var_info return_value_info;

	/* 1. Count number of TSSA opcodes;
	 *    Count number of activation frames;
	 *    Calculate size of abstract stack;
	 *    Construct regular SSA for involved op_array */
	op_array = trace_buffer->op_array;
	stack_top = stack_size = crex_jit_trace_frame_size(op_array);
	stack_bottom = 0;
	p = trace_buffer + CREX_JIT_TRACE_START_REC_SIZE;
	ssa_ops_count = 0;
	call_level = 0;
	level = 0;
	num_op_arrays = 0;
	/* Remember op_array to cleanup */
	op_arrays[num_op_arrays++] = op_array;
	/* Build SSA */
	ssa = crex_jit_trace_build_ssa(op_array, script);
	for (;;p++) {
		if (p->op == CREX_JIT_TRACE_VM) {
			if (JIT_G(opt_level) < CREX_JIT_LEVEL_OPT_FUNC) {
				const crex_op *opline = p->opline;

				switch (opline->opcode) {
					case CREX_INCLUDE_OR_EVAL:
						ssa->cfg.flags |= CREX_FUNC_INDIRECT_VAR_ACCESS;
						break;
					case CREX_FETCH_R:
					case CREX_FETCH_W:
					case CREX_FETCH_RW:
					case CREX_FETCH_FUNC_ARG:
					case CREX_FETCH_IS:
					case CREX_FETCH_UNSET:
					case CREX_UNSET_VAR:
					case CREX_ISSET_ISEMPTY_VAR:
						if (opline->extended_value & CREX_FETCH_LOCAL) {
							ssa->cfg.flags |= CREX_FUNC_INDIRECT_VAR_ACCESS;
						} else if ((opline->extended_value & (CREX_FETCH_GLOBAL | CREX_FETCH_GLOBAL_LOCK)) &&
						           !op_array->function_name) {
							ssa->cfg.flags |= CREX_FUNC_INDIRECT_VAR_ACCESS;
						}
						break;
				}
			}
			ssa_ops_count += crex_jit_trace_op_len(p->opline);
		} else if (p->op == CREX_JIT_TRACE_INIT_CALL) {
			call_level++;
			stack_top += crex_jit_trace_frame_size(p->op_array);
			if (stack_top > stack_size) {
				stack_size = stack_top;
			}
		} else if (p->op == CREX_JIT_TRACE_DO_ICALL) {
			if (JIT_G(opt_level) < CREX_JIT_LEVEL_OPT_FUNC) {
				if (p->func != (crex_function*)&crex_pass_function
				 && (crex_string_equals_literal(p->func->common.function_name, "extract")
				  || crex_string_equals_literal(p->func->common.function_name, "compact")
				  || crex_string_equals_literal(p->func->common.function_name, "get_defined_vars"))) {
					ssa->cfg.flags |= CREX_FUNC_INDIRECT_VAR_ACCESS;
				}
			}
			frame_size = crex_jit_trace_frame_size(p->op_array);
			if (call_level == 0) {
				if (stack_top + frame_size > stack_size) {
					stack_size = stack_top + frame_size;
				}
			} else {
				call_level--;
				stack_top -= frame_size;
			}
		} else if (p->op == CREX_JIT_TRACE_ENTER) {
			op_array = p->op_array;
			if (call_level == 0) {
				stack_top += crex_jit_trace_frame_size(op_array);
				if (stack_top > stack_size) {
					stack_size = stack_top;
				}
			} else {
				call_level--;
			}
			level++;
			jit_extension =
				(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
			ssa = &jit_extension->func_info.ssa;
			if (ssa->cfg.blocks_count) {
				/* pass */
			} else if (num_op_arrays == CREX_JIT_TRACE_MAX_FUNCS) {
				/* Too many functions in single trace */
				*num_op_arrays_ptr = num_op_arrays;
				return NULL;
			} else {
				/* Remember op_array to cleanup */
				op_arrays[num_op_arrays++] = op_array;
				/* Build SSA */
				ssa = crex_jit_trace_build_ssa(op_array, script);
			}
		} else if (p->op == CREX_JIT_TRACE_BACK) {
			if (level == 0) {
				stack_bottom += crex_jit_trace_frame_size(p->op_array);
				jit_extension =
					(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
				ssa = &jit_extension->func_info.ssa;
				if (ssa->cfg.blocks_count) {
					/* pass */
				} else if (num_op_arrays == CREX_JIT_TRACE_MAX_FUNCS) {
					/* Too many functions in single trace */
					*num_op_arrays_ptr = num_op_arrays;
					return NULL;
				} else {
					/* Remember op_array to cleanup */
					op_arrays[num_op_arrays++] = op_array;
					/* Build SSA */
					ssa = crex_jit_trace_build_ssa(op_array, script);
				}
			} else {
				stack_top -= crex_jit_trace_frame_size(op_array);
				level--;
			}
			op_array = p->op_array;
		} else if (p->op == CREX_JIT_TRACE_END) {
			break;
		}
	}
	*num_op_arrays_ptr = num_op_arrays;

	/* Allocate space for abstract stack */
	JIT_G(current_frame) = frame = (crex_jit_trace_stack_frame*)((char*)crex_arena_alloc(&CG(arena), stack_bottom + stack_size) + stack_bottom);

	/* 2. Construct TSSA */
	tssa = crex_arena_calloc(&CG(arena), 1, sizeof(crex_tssa));
	tssa->cfg.flags = CREX_SSA_TSSA;
	tssa->cfg.blocks = crex_arena_calloc(&CG(arena), 2, sizeof(crex_basic_block));
	tssa->blocks = crex_arena_calloc(&CG(arena), 2, sizeof(crex_ssa_block));
	tssa->cfg.predecessors = crex_arena_calloc(&CG(arena), 2, sizeof(int));

	if (trace_buffer->stop == CREX_JIT_TRACE_STOP_LOOP
	 || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_CALL
	 || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_RET) {
		tssa->cfg.blocks_count = 2;
		tssa->cfg.edges_count = 2;

		tssa->cfg.predecessors[0] = 0;
		tssa->cfg.predecessors[1] = 1;

		tssa->cfg.blocks[0].flags = CREX_BB_START|CREX_BB_REACHABLE;
		tssa->cfg.blocks[0].successors_count = 1;
		tssa->cfg.blocks[0].predecessors_count = 0;
		tssa->cfg.blocks[0].successors = tssa->cfg.blocks[0].successors_storage;
		tssa->cfg.blocks[0].successors[0] = 1;

		tssa->cfg.blocks[0].flags = CREX_BB_FOLLOW|CREX_BB_TARGET|CREX_BB_LOOP_HEADER|CREX_BB_REACHABLE;
		tssa->cfg.blocks[1].successors_count = 1;
		tssa->cfg.blocks[1].predecessors_count = 2;
		tssa->cfg.blocks[1].successors = tssa->cfg.blocks[1].successors_storage;
		tssa->cfg.blocks[1].successors[1] = 1;
	} else {
		tssa->cfg.blocks_count = 1;
		tssa->cfg.edges_count = 0;

		tssa->cfg.blocks[0].flags = CREX_BB_START|CREX_BB_EXIT|CREX_BB_REACHABLE;
		tssa->cfg.blocks[0].successors_count = 0;
		tssa->cfg.blocks[0].predecessors_count = 0;
	}
	((crex_tssa*)tssa)->used_stack = -1;

	if (JIT_G(opt_level) < CREX_JIT_LEVEL_INLINE) {
		return tssa;
	}

	tssa->ops = ssa_ops = crex_arena_alloc(&CG(arena), ssa_ops_count * sizeof(crex_ssa_op));
	memset(ssa_ops, -1, ssa_ops_count * sizeof(crex_ssa_op));
	ssa_opcodes = crex_arena_calloc(&CG(arena), ssa_ops_count + 1, sizeof(crex_op*));
	((crex_tssa*)tssa)->tssa_opcodes = ssa_opcodes;
	ssa_opcodes[ssa_ops_count] = &_nop_opcode;

	op_array = trace_buffer->op_array;
	if (trace_buffer->start == CREX_JIT_TRACE_START_ENTER) {
		ssa_vars_count = op_array->last_var;
	} else {
		ssa_vars_count = op_array->last_var + op_array->T;
	}
	stack = frame->stack;
	for (i = 0; i < ssa_vars_count; i++) {
		SET_STACK_VAR(stack, i, i);
	}

	if (trace_buffer->stop == CREX_JIT_TRACE_STOP_LOOP) {
		// TODO: For tracing, it's possible, to create pseudo Phi functions
		//       at the end of loop, without this additional pass (like LuaJIT) ???
		ssa_vars_count = crex_jit_trace_add_phis(trace_buffer, ssa_vars_count, tssa, stack);
	} else if (trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_CALL) {
		ssa_vars_count = crex_jit_trace_add_call_phis(trace_buffer, ssa_vars_count, tssa, stack);
	} else if (trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_RET) {
		ssa_vars_count = crex_jit_trace_add_ret_phis(trace_buffer, ssa_vars_count, tssa, stack);
	}

	p = trace_buffer + CREX_JIT_TRACE_START_REC_SIZE;
	idx = 0;
	level = 0;
	for (;;p++) {
		if (p->op == CREX_JIT_TRACE_VM) {
			opline = p->opline;
			ssa_opcodes[idx] = opline;
			ssa_vars_count = crex_ssa_rename_op(op_array, opline, idx, build_flags, ssa_vars_count, ssa_ops, (int*)stack);
			idx++;
			len = crex_jit_trace_op_len(p->opline);
			while (len > 1) {
				opline++;
				ssa_opcodes[idx] = opline;
				if (opline->opcode != CREX_OP_DATA) {
					ssa_vars_count = crex_ssa_rename_op(op_array, opline, idx, build_flags, ssa_vars_count, ssa_ops, (int*)stack);
				}
				idx++;
				len--;
			}
		} else if (p->op == CREX_JIT_TRACE_ENTER) {
			frame = crex_jit_trace_call_frame(frame, op_array);
			stack = frame->stack;
			op_array = p->op_array;
			level++;
			if (ssa_vars_count >= CREX_JIT_TRACE_MAX_SSA_VAR) {
				return NULL;
			}
			CREX_JIT_TRACE_SET_FIRST_SSA_VAR(p->info, ssa_vars_count);
			for (i = 0; i < op_array->last_var; i++) {
				SET_STACK_VAR(stack, i, ssa_vars_count++);
			}
		} else if (p->op == CREX_JIT_TRACE_BACK) {
			op_array = p->op_array;
			frame = crex_jit_trace_ret_frame(frame, op_array);
			stack = frame->stack;
			if (level == 0) {
				if (ssa_vars_count >= CREX_JIT_TRACE_MAX_SSA_VAR) {
					return NULL;
				}
				CREX_JIT_TRACE_SET_FIRST_SSA_VAR(p->info, ssa_vars_count);
				for (i = 0; i < op_array->last_var + op_array->T; i++) {
					SET_STACK_VAR(stack, i, ssa_vars_count++);
				}
			} else {
				level--;
			}
		} else if (p->op == CREX_JIT_TRACE_END) {
			break;
		}
	}

	op_array = trace_buffer->op_array;
	tssa->vars_count = ssa_vars_count;
	tssa->vars = ssa_vars = crex_arena_calloc(&CG(arena), tssa->vars_count, sizeof(crex_ssa_var));
	if (trace_buffer->start == CREX_JIT_TRACE_START_ENTER) {
		vars_count = op_array->last_var;
	} else {
		vars_count = op_array->last_var + op_array->T;
	}
	i = 0;
	while (i < vars_count) {
		ssa_vars[i].var = i;
		ssa_vars[i].scc = -1;
		ssa_vars[i].definition = -1;
		ssa_vars[i].use_chain = -1;
		i++;
	}
	while (i < tssa->vars_count) {
		ssa_vars[i].var = -1;
		ssa_vars[i].scc = -1;
		ssa_vars[i].definition = -1;
		ssa_vars[i].use_chain = -1;
		i++;
	}

	if (trace_buffer->stop == CREX_JIT_TRACE_STOP_LOOP
	 || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_CALL
	 || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_RET) {
		/* Update Phi sources */
		crex_ssa_phi *phi = tssa->blocks[1].phis;

		while (phi) {
			phi->sources[1] = STACK_VAR(stack, phi->var);
			ssa_vars[phi->ssa_var].var = phi->var;
			ssa_vars[phi->ssa_var].definition_phi = phi;
			ssa_vars[phi->sources[0]].phi_use_chain = phi;
			ssa_vars[phi->sources[1]].phi_use_chain = phi;
			phi = phi->next;
		}
	}

	/* 3. Compute use-def chains */
	idx = (ssa_ops_count - 1);
	op = ssa_ops + idx;
	while (idx >= 0) {
		opline = ssa_opcodes[idx];
		if (op->op1_use >= 0) {
			op->op1_use_chain = ssa_vars[op->op1_use].use_chain;
			ssa_vars[op->op1_use].use_chain = idx;
		}
		if (op->op2_use >= 0 && op->op2_use != op->op1_use) {
			op->op2_use_chain = ssa_vars[op->op2_use].use_chain;
			ssa_vars[op->op2_use].use_chain = idx;
		}
		if (op->result_use >= 0 && op->result_use != op->op1_use && op->result_use != op->op2_use) {
			op->res_use_chain = ssa_vars[op->result_use].use_chain;
			ssa_vars[op->result_use].use_chain = idx;
		}
		if (op->op1_def >= 0) {
			ssa_vars[op->op1_def].var = EX_VAR_TO_NUM(opline->op1.var);
			ssa_vars[op->op1_def].definition = idx;
		}
		if (op->op2_def >= 0) {
			ssa_vars[op->op2_def].var = EX_VAR_TO_NUM(opline->op2.var);
			ssa_vars[op->op2_def].definition = idx;
		}
		if (op->result_def >= 0) {
			ssa_vars[op->result_def].var = EX_VAR_TO_NUM(opline->result.var);
			ssa_vars[op->result_def].definition = idx;
		}
		op--;
		idx--;
	}

	/* 4. Type inference */
	op_array = trace_buffer->op_array;
	jit_extension =
		(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
	ssa = &jit_extension->func_info.ssa;

	tssa->var_info = ssa_var_info = crex_arena_calloc(&CG(arena), tssa->vars_count, sizeof(crex_ssa_var_info));

	if (trace_buffer->start == CREX_JIT_TRACE_START_ENTER) {
		i = 0;
		while (i < op_array->last_var) {
			if (i < op_array->num_args) {
				if (ssa->var_info
				 && crex_jit_trace_copy_ssa_var_info(op_array, ssa, ssa_opcodes, tssa, i)) {
					/* pass */
				} else {
					if (ssa->vars) {
						ssa_vars[i].no_val = ssa->vars[i].no_val;
						ssa_vars[i].alias = ssa->vars[i].alias;
					} else {
						ssa_vars[i].alias = crex_jit_var_may_alias(op_array, ssa, i);
					}
					if (op_array->arg_info && i < trace_buffer[1].opline - op_array->opcodes) {
						crex_arg_info *arg_info = &op_array->arg_info[i];
						crex_class_entry *ce;
						uint32_t tmp = crex_fetch_arg_info_type(script, arg_info, &ce);

						if (CREX_ARG_SEND_MODE(arg_info)) {
							tmp |= MAY_BE_REF;
						}
						ssa_var_info[i].type = tmp;
						ssa_var_info[i].ce = ce;
						ssa_var_info[i].is_instanceof = 1;
					} else {
						ssa_var_info[i].type = MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_REF | MAY_BE_ANY  | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
					}
				}
			} else {
				if (ssa->vars) {
					ssa_vars[i].no_val = ssa->vars[i].no_val;
					ssa_vars[i].alias = ssa->vars[i].alias;
				} else {
					ssa_vars[i].alias = crex_jit_var_may_alias(op_array, ssa, i);
				}
				if (ssa_vars[i].alias == NO_ALIAS) {
					ssa_var_info[i].type = MAY_BE_UNDEF;
				} else {
					ssa_var_info[i].type = MAY_BE_UNDEF | MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_REF | MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				}
			}
			i++;
		}
	} else {
		int parent_vars_count = 0;
		crex_jit_trace_stack *parent_stack = NULL;

		i = 0;
		if (parent_trace) {
			parent_vars_count = MIN(crex_jit_traces[parent_trace].exit_info[exit_num].stack_size,
				op_array->last_var + op_array->T);
			if (parent_vars_count) {
				parent_stack =
					crex_jit_traces[parent_trace].stack_map +
					crex_jit_traces[parent_trace].exit_info[exit_num].stack_offset;
			}
		}
		while (i < op_array->last_var + op_array->T) {
			if (!ssa->var_info
			 || !crex_jit_trace_copy_ssa_var_info(op_array, ssa, ssa_opcodes, tssa, i)) {
				if (ssa->vars && i < ssa->vars_count) {
					ssa_vars[i].alias = ssa->vars[i].alias;
				} else {
					ssa_vars[i].alias = crex_jit_var_may_alias(op_array, ssa, i);
				}
				if (i < op_array->last_var) {
					ssa_var_info[i].type = MAY_BE_UNDEF | MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_REF | MAY_BE_ANY  | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				} else {
					ssa_var_info[i].type = MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_REF | MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				}
			}
			if (i < parent_vars_count) {
				/* Initialize TSSA variable from parent trace */
				uint8_t op_type = STACK_TYPE(parent_stack, i);

				if (op_type != IS_UNKNOWN) {
					ssa_var_info[i].type &= crex_jit_trace_type_to_info(op_type);
					if (!ssa_var_info[i].type
					 && op_type == IS_UNDEF
					 && i >= op_array->last_var) {
						// TODO: It's better to use NULL instead of UNDEF for temporary variables
						ssa_var_info[i].type |= MAY_BE_UNDEF;
					}
				}
			}
			i++;
		}
	}

	if (trace_buffer->stop == CREX_JIT_TRACE_STOP_LOOP
	 || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_CALL
	 || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_RET) {
		/* Propagate initial value through Phi functions */
		crex_ssa_phi *phi = tssa->blocks[1].phis;

		while (phi) {
			if (!ssa->var_info
			 || !crex_jit_trace_copy_ssa_var_info(op_array, ssa, ssa_opcodes, tssa, phi->ssa_var)) {
				ssa_vars[phi->ssa_var].alias = ssa_vars[phi->sources[0]].alias;
				ssa_var_info[phi->ssa_var].type = ssa_var_info[phi->sources[0]].type;
			}
			phi = phi->next;
		}
	}

	frame = JIT_G(current_frame);
	top = crex_jit_trace_call_frame(frame, op_array);
	TRACE_FRAME_INIT(frame, op_array, 0, 0);
	TRACE_FRAME_SET_RETURN_SSA_VAR(frame, -1);
	frame->used_stack = 0;
	for (i = 0; i < op_array->last_var + op_array->T; i++) {
		SET_STACK_TYPE(frame->stack, i, IS_UNKNOWN, 1);
	}
	memset(&return_value_info, 0, sizeof(return_value_info));

	if (trace_buffer->stop == CREX_JIT_TRACE_STOP_LOOP) {
		max_used_stack = used_stack = 0;
	} else {
		max_used_stack = used_stack = -1;
	}

	p = trace_buffer + CREX_JIT_TRACE_START_REC_SIZE;
	idx = 0;
	level = 0;
	opline = NULL;
	for (;;p++) {
		if (p->op == CREX_JIT_TRACE_VM) {
			uint8_t orig_op1_type, orig_op2_type, op1_type, op2_type, op3_type;
			uint8_t val_type = IS_UNKNOWN;
//			crex_class_entry *op1_ce = NULL;
			crex_class_entry *op2_ce = NULL;

			opline = p->opline;

			op1_type = orig_op1_type = p->op1_type;
			op2_type = orig_op2_type = p->op2_type;
			op3_type = p->op3_type;
			if (op1_type & (IS_TRACE_REFERENCE|IS_TRACE_INDIRECT)) {
				op1_type = IS_UNKNOWN;
			}
			if (op1_type != IS_UNKNOWN) {
				op1_type &= ~IS_TRACE_PACKED;
			}
			if (op2_type & (IS_TRACE_REFERENCE|IS_TRACE_INDIRECT)) {
				op2_type = IS_UNKNOWN;
			}
			if (op3_type & (IS_TRACE_REFERENCE|IS_TRACE_INDIRECT)) {
				op3_type = IS_UNKNOWN;
			}

			if ((p+1)->op == CREX_JIT_TRACE_OP1_TYPE) {
//				op1_ce = (crex_class_entry*)(p+1)->ce;
				p++;
			}
			if ((p+1)->op == CREX_JIT_TRACE_OP2_TYPE) {
				op2_ce = (crex_class_entry*)(p+1)->ce;
				p++;
			}
			if ((p+1)->op == CREX_JIT_TRACE_VAL_INFO) {
				val_type = (p+1)->op1_type;
				p++;
			}

			switch (opline->opcode) {
				case CREX_ASSIGN_OP:
					if (opline->extended_value == CREX_POW
					 || opline->extended_value == CREX_DIV) {
						// TODO: check for division by zero ???
						break;
					}
					if (opline->op1_type != IS_CV || opline->result_type != IS_UNUSED) {
						break;
					}
					ADD_OP1_TRACE_GUARD();
					ADD_OP2_TRACE_GUARD();
					break;
				case CREX_ASSIGN_DIM_OP:
					if (opline->extended_value == CREX_POW
					 || opline->extended_value == CREX_DIV) {
						// TODO: check for division by zero ???
						break;
					}
					if (opline->result_type != IS_UNUSED) {
						break;
					}
					if (op3_type != IS_UNKNOWN
					 && !crex_jit_supported_binary_op(
							opline->extended_value, MAY_BE_ANY, (1<<op3_type))) {
						break;
					}
					CREX_FALLTHROUGH;
				case CREX_ASSIGN_DIM:
					if (opline->op1_type == IS_CV) {
						if ((opline+1)->op1_type == IS_CV
						 && (opline+1)->op1.var == opline->op1.var) {
							/* skip $a[x] = $a; */
							break;
						}
						ADD_OP1_DATA_TRACE_GUARD();
						ADD_OP2_TRACE_GUARD();
						ADD_OP1_TRACE_GUARD();
					} else if (orig_op1_type != IS_UNKNOWN
					        && (orig_op1_type & IS_TRACE_INDIRECT)
					        && opline->result_type == IS_UNUSED) {
						if (opline->opcode == CREX_ASSIGN_DIM_OP) {
							ADD_OP1_DATA_TRACE_GUARD();
						}
						ADD_OP2_TRACE_GUARD();
					}
					if (op1_type == IS_ARRAY
					 && ((opline->op2_type == IS_CONST
					   && C_TYPE_P(RT_CONSTANT(opline, opline->op2)) == IS_LONG)
					  || (opline->op2_type != IS_CONST
					   && op2_type == IS_LONG))) {

						if (!(orig_op1_type & IS_TRACE_PACKED)) {
							crex_ssa_var_info *info = &tssa->var_info[tssa->ops[idx].op1_use];

							if (MAY_BE_PACKED(info->type) && MAY_BE_HASH(info->type)) {
								info->type |= MAY_BE_PACKED_GUARD;
								info->type &= ~MAY_BE_ARRAY_PACKED;
							}
						} else if (opline->opcode == CREX_ASSIGN_DIM_OP
								&& val_type != IS_UNKNOWN
								&& val_type != IS_UNDEF) {
							crex_ssa_var_info *info = &tssa->var_info[tssa->ops[idx].op1_use];

							if (MAY_BE_PACKED(info->type) && MAY_BE_HASH(info->type)) {
								info->type |= MAY_BE_PACKED_GUARD;
								info->type &= ~(MAY_BE_ARRAY_NUMERIC_HASH|MAY_BE_ARRAY_STRING_HASH);
							}
						}
					}
					break;
				case CREX_ASSIGN_OBJ_OP:
					if (opline->extended_value == CREX_POW
					 || opline->extended_value == CREX_DIV) {
						// TODO: check for division by zero ???
						break;
					}
					if (opline->result_type != IS_UNUSED) {
						break;
					}
					CREX_FALLTHROUGH;
				case CREX_ASSIGN_OBJ:
				case CREX_PRE_INC_OBJ:
				case CREX_PRE_DEC_OBJ:
				case CREX_POST_INC_OBJ:
				case CREX_POST_DEC_OBJ:
					if (opline->op2_type != IS_CONST
					 || C_TYPE_P(RT_CONSTANT(opline, opline->op2)) != IS_STRING
					 || C_STRVAL_P(RT_CONSTANT(opline, opline->op2))[0] == '\0') {
						break;
					}
					if (opline->opcode == CREX_ASSIGN_OBJ_OP) {
						if (opline->op1_type == IS_CV
						 && (opline+1)->op1_type == IS_CV
						 && (opline+1)->op1.var == opline->op1.var) {
							/* skip $a->prop += $a; */
							break;
						}
						ADD_OP1_DATA_TRACE_GUARD();
					}
					ADD_OP1_TRACE_GUARD();
					break;
				case CREX_CONCAT:
				case CREX_FAST_CONCAT:
					if ((opline->op1_type == IS_CONST || orig_op1_type == IS_STRING)
					 && (opline->op2_type == IS_CONST || orig_op2_type == IS_STRING)) {
						ADD_OP2_TRACE_GUARD();
						ADD_OP1_TRACE_GUARD();
					}
					break;
				case CREX_IS_EQUAL:
				case CREX_IS_NOT_EQUAL:
				case CREX_IS_SMALLER:
				case CREX_IS_SMALLER_OR_EQUAL:
				case CREX_CASE:
				case CREX_IS_IDENTICAL:
				case CREX_IS_NOT_IDENTICAL:
				case CREX_CASE_STRICT:
				case CREX_BW_OR:
				case CREX_BW_AND:
				case CREX_BW_XOR:
				case CREX_SL:
				case CREX_SR:
				case CREX_MOD:
				case CREX_ADD:
				case CREX_SUB:
				case CREX_MUL:
//				case CREX_DIV: // TODO: check for division by zero ???
					ADD_OP2_TRACE_GUARD();
					CREX_FALLTHROUGH;
				case CREX_ECHO:
				case CREX_STRLEN:
				case CREX_COUNT:
				case CREX_QM_ASSIGN:
				case CREX_FE_RESET_R:
					ADD_OP1_TRACE_GUARD();
					break;
				case CREX_FE_FETCH_R:
					ADD_OP1_TRACE_GUARD();
					if (op1_type == IS_ARRAY && (orig_op1_type & ~IS_TRACE_PACKED) == IS_ARRAY) {

						crex_ssa_var_info *info = &tssa->var_info[tssa->ops[idx].op1_use];

						if (MAY_BE_PACKED(info->type) && MAY_BE_HASH(info->type)) {
							info->type |= MAY_BE_PACKED_GUARD;
							if (orig_op1_type & IS_TRACE_PACKED) {
								info->type &= ~(MAY_BE_ARRAY_NUMERIC_HASH|MAY_BE_ARRAY_STRING_HASH);
							} else {
								info->type &= ~MAY_BE_ARRAY_PACKED;
							}
						}
					}
					break;
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
					ADD_OP1_TRACE_GUARD();
					break;
				case CREX_FETCH_DIM_FUNC_ARG:
					if (!frame
					 || !frame->call
					 || !frame->call->func
					 || !TRACE_FRAME_IS_LAST_SEND_BY_VAL(frame->call)) {
						break;
					}
					ADD_OP2_TRACE_GUARD();
					ADD_OP1_TRACE_GUARD();
					break;
				case CREX_PRE_INC:
				case CREX_PRE_DEC:
				case CREX_POST_INC:
				case CREX_POST_DEC:
					if (opline->op1_type != IS_CV) {
						break;
					}
					ADD_OP1_TRACE_GUARD();
					break;
				case CREX_ASSIGN:
					if (opline->op1_type != IS_CV) {
						break;
					}
					ADD_OP2_TRACE_GUARD();
					if (op1_type != IS_UNKNOWN
					 && (tssa->var_info[tssa->ops[idx].op1_use].type & MAY_BE_REF)) {
						ADD_OP1_TRACE_GUARD();
					}
					break;
				case CREX_CAST:
					if (opline->extended_value != op1_type) {
						break;
					}
					ADD_OP1_TRACE_GUARD();
					break;
				case CREX_JMPZ:
				case CREX_JMPNZ:
				case CREX_JMPC_EX:
				case CREX_JMPNC_EX:
				case CREX_BOOL:
				case CREX_BOOL_NOT:
					ADD_OP1_TRACE_GUARD();
					break;
				case CREX_ISSET_ISEMPTY_CV:
					if ((opline->extended_value & CREX_ISEMPTY)) {
						// TODO: support for empty() ???
						break;
					}
					ADD_OP1_TRACE_GUARD();
					break;
				case CREX_IN_ARRAY:
					if (opline->op1_type == IS_VAR || opline->op1_type == IS_TMP_VAR) {
						break;
					}
					ADD_OP1_TRACE_GUARD();
					break;
				case CREX_ISSET_ISEMPTY_DIM_OBJ:
					if ((opline->extended_value & CREX_ISEMPTY)) {
						// TODO: support for empty() ???
						break;
					}
					CREX_FALLTHROUGH;
				case CREX_FETCH_DIM_R:
				case CREX_FETCH_DIM_IS:
				case CREX_FETCH_LIST_R:
					ADD_OP1_TRACE_GUARD();
					ADD_OP2_TRACE_GUARD();

					if (op1_type == IS_ARRAY
					 && opline->op1_type != IS_CONST
					 && ((opline->op2_type == IS_CONST
					   && C_TYPE_P(RT_CONSTANT(opline, opline->op2)) == IS_LONG)
					  || (opline->op2_type != IS_CONST
					   && op2_type == IS_LONG))) {

						crex_ssa_var_info *info = &tssa->var_info[tssa->ops[idx].op1_use];

						if (MAY_BE_PACKED(info->type) && MAY_BE_HASH(info->type)) {
							info->type |= MAY_BE_PACKED_GUARD;
							if (orig_op1_type & IS_TRACE_PACKED) {
								info->type &= ~(MAY_BE_ARRAY_NUMERIC_HASH|MAY_BE_ARRAY_STRING_HASH);
							} else {
								info->type &= ~MAY_BE_ARRAY_PACKED;
							}
						}
					}
					break;
				case CREX_FETCH_DIM_W:
				case CREX_FETCH_DIM_RW:
//				case CREX_FETCH_DIM_UNSET:
				case CREX_FETCH_LIST_W:
					if (opline->op1_type != IS_CV
					 && (orig_op1_type == IS_UNKNOWN
					  || !(orig_op1_type & IS_TRACE_INDIRECT))) {
						break;
					}
					ADD_OP1_TRACE_GUARD();
					ADD_OP2_TRACE_GUARD();
					if (op1_type == IS_ARRAY
					 && !(orig_op1_type & IS_TRACE_PACKED)
					 && ((opline->op2_type == IS_CONST
					   && C_TYPE_P(RT_CONSTANT(opline, opline->op2)) == IS_LONG)
					  || (opline->op2_type != IS_CONST
					   && op2_type == IS_LONG))) {

						crex_ssa_var_info *info = &tssa->var_info[tssa->ops[idx].op1_use];

						if (MAY_BE_PACKED(info->type) && MAY_BE_HASH(info->type)) {
							info->type |= MAY_BE_PACKED_GUARD;
							info->type &= ~MAY_BE_ARRAY_PACKED;
						}
					}
					break;
				case CREX_SEND_VAL_EX:
				case CREX_SEND_VAR_EX:
				case CREX_SEND_VAR_NO_REF_EX:
					if (opline->op2_type == IS_CONST) {
						/* Named parameters not supported in JIT */
						break;
					}
					if (opline->op2.num > MAX_ARG_FLAG_NUM) {
						goto propagate_arg;
					}
					CREX_FALLTHROUGH;
				case CREX_SEND_VAL:
				case CREX_SEND_VAR:
				case CREX_SEND_VAR_NO_REF:
				case CREX_SEND_FUNC_ARG:
					if (opline->op2_type == IS_CONST) {
						/* Named parameters not supported in JIT */
						break;
					}
					ADD_OP1_TRACE_GUARD();
propagate_arg:
					/* Propagate argument type */
					if (frame->call
					 && frame->call->func
					 && frame->call->func->type == CREX_USER_FUNCTION
					 && opline->op2.num <= frame->call->func->op_array.num_args) {
						uint32_t info;

						if (opline->op1_type == IS_CONST) {
							info = _const_op_type(RT_CONSTANT(opline, opline->op1));
						} else {
							CREX_ASSERT(ssa_ops[idx].op1_use >= 0);
							info = ssa_var_info[ssa_ops[idx].op1_use].type & ~MAY_BE_GUARD;
						}
						if (frame->call->func->op_array.fn_flags & CREX_ACC_HAS_TYPE_HINTS) {
							crex_arg_info *arg_info;

							CREX_ASSERT(frame->call->func->op_array.arg_info);
							arg_info = &frame->call->func->op_array.arg_info[opline->op2.num - 1];
							if (CREX_TYPE_IS_SET(arg_info->type)) {
								crex_class_entry *ce;
								uint32_t tmp = crex_fetch_arg_info_type(script, arg_info, &ce);
								info &= tmp;
								if (!info) {
									break;
								}
							}
						}
						if (opline->op1_type == IS_CV && (info & MAY_BE_RC1)) {
							info |= MAY_BE_RCN;
						}
						if (info & MAY_BE_UNDEF) {
							info |= MAY_BE_NULL;
							info &= ~MAY_BE_UNDEF;
						}
						if (ARG_SHOULD_BE_SENT_BY_REF(frame->call->func, opline->op2.num)) {
							info |= MAY_BE_REF|MAY_BE_RC1|MAY_BE_RCN|MAY_BE_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_KEY_ANY;
						}
						SET_STACK_INFO(frame->call->stack, opline->op2.num - 1, info);
					}
					break;
				case CREX_RETURN:
					ADD_OP1_TRACE_GUARD();
					/* Propagate return value types */
					if (opline->op1_type == IS_UNUSED) {
						return_value_info.type = MAY_BE_NULL;
					} else if (opline->op1_type == IS_CONST) {
						return_value_info.type = _const_op_type(RT_CONSTANT(opline, opline->op1));
					} else {
						CREX_ASSERT(ssa_ops[idx].op1_use >= 0);
						return_value_info = ssa_var_info[ssa_ops[idx].op1_use];
						if (return_value_info.type & MAY_BE_UNDEF) {
							return_value_info.type &= ~MAY_BE_UNDEF;
							return_value_info.type |= MAY_BE_NULL;
						}
						if (return_value_info.type & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)) {
							/* CVs are going to be destructed and the reference-counter
							   of return value may be decremented to 1 */
							return_value_info.type |= MAY_BE_RC1;
						}
						return_value_info.type &= ~MAY_BE_GUARD;
					}
					break;
				case CREX_CHECK_FUNC_ARG:
					if (!frame
					 || !frame->call
					 || !frame->call->func) {
						break;
					}
					if (opline->op2_type == IS_CONST
					 || opline->op2.num > MAX_ARG_FLAG_NUM) {
						/* Named parameters not supported in JIT */
						TRACE_FRAME_SET_LAST_SEND_UNKNOWN(frame->call);
						break;
					}
					if (ARG_SHOULD_BE_SENT_BY_REF(frame->call->func, opline->op2.num)) {
						TRACE_FRAME_SET_LAST_SEND_BY_REF(frame->call);
					} else {
						TRACE_FRAME_SET_LAST_SEND_BY_VAL(frame->call);
					}
					break;
				case CREX_FETCH_OBJ_FUNC_ARG:
					if (!frame
					 || !frame->call
					 || !frame->call->func
					 || !TRACE_FRAME_IS_LAST_SEND_BY_VAL(frame->call)) {
						break;
					}
					CREX_FALLTHROUGH;
				case CREX_FETCH_OBJ_R:
				case CREX_FETCH_OBJ_IS:
				case CREX_FETCH_OBJ_W:
					if (opline->op2_type != IS_CONST
					 || C_TYPE_P(RT_CONSTANT(opline, opline->op2)) != IS_STRING
					 || C_STRVAL_P(RT_CONSTANT(opline, opline->op2))[0] == '\0') {
						break;
					}
					if (opline->op1_type != IS_UNUSED && op1_type == IS_OBJECT) {
						ADD_OP1_TRACE_GUARD();
					}
					break;
				case CREX_INIT_METHOD_CALL:
					if (opline->op2_type != IS_CONST
					 || C_TYPE_P(RT_CONSTANT(opline, opline->op2)) != IS_STRING) {
						break;
					}
					ADD_OP1_TRACE_GUARD();
					break;
				case CREX_INIT_DYNAMIC_CALL:
					if (orig_op2_type == IS_OBJECT && op2_ce == crex_ce_closure) {
						ADD_OP2_TRACE_GUARD();
					}
					break;
				case CREX_SEND_ARRAY:
				case CREX_SEND_UNPACK:
				case CREX_CHECK_UNDEF_ARGS:
				case CREX_INCLUDE_OR_EVAL:
					max_used_stack = used_stack = -1;
					break;
				case CREX_TYPE_CHECK:
					if (opline->extended_value == MAY_BE_RESOURCE) {
						// TODO: support for is_resource() ???
						break;
					}
					if (op1_type != IS_UNKNOWN
					 && (opline->extended_value == (1 << op1_type)
					  || opline->extended_value == MAY_BE_ANY - (1 << op1_type))) {
						/* add guards only for exact checks, to avoid code duplication */
						ADD_OP1_TRACE_GUARD();
					}
					break;
				case CREX_ROPE_INIT:
				case CREX_ROPE_ADD:
				case CREX_ROPE_END:
					if (op2_type == IS_STRING) {
						ADD_OP2_TRACE_GUARD();
					}
					break;
				default:
					break;
			}
			len = crex_jit_trace_op_len(opline);
			if (ssa->var_info) {
				/* Add statically inferred ranges */
				if (ssa_ops[idx].op1_def >= 0) {
					crex_jit_trace_copy_ssa_var_range(op_array, ssa, ssa_opcodes, tssa, ssa_ops[idx].op1_def);
				}
				if (ssa_ops[idx].op2_def >= 0) {
					crex_jit_trace_copy_ssa_var_range(op_array, ssa, ssa_opcodes, tssa, ssa_ops[idx].op2_def);
				}
				if (ssa_ops[idx].result_def >= 0) {
					crex_jit_trace_copy_ssa_var_range(op_array, ssa, ssa_opcodes, tssa, ssa_ops[idx].result_def);
				}
				if (len == 2 && (opline+1)->opcode == CREX_OP_DATA) {
					if (ssa_ops[idx+1].op1_def >= 0) {
						crex_jit_trace_copy_ssa_var_range(op_array, ssa, ssa_opcodes, tssa, ssa_ops[idx+1].op1_def);
					}
					if (ssa_ops[idx+1].op2_def >= 0) {
						crex_jit_trace_copy_ssa_var_range(op_array, ssa, ssa_opcodes, tssa, ssa_ops[idx+1].op2_def);
					}
					if (ssa_ops[idx+1].result_def >= 0) {
						crex_jit_trace_copy_ssa_var_range(op_array, ssa, ssa_opcodes, tssa, ssa_ops[idx+1].result_def);
					}
				}
			} else {
				if (ssa_ops[idx].op1_def >= 0) {
					ssa_vars[ssa_ops[idx].op1_def].alias = crex_jit_var_may_alias(op_array, ssa, EX_VAR_TO_NUM(opline->op1.var));
					if (ssa_ops[idx].op1_use < 0 || !(ssa_var_info[ssa_ops[idx].op1_use].type & MAY_BE_REF)) {
						crex_jit_trace_propagate_range(op_array, ssa_opcodes, tssa, ssa_ops[idx].op1_def);
					}
				}
				if (ssa_ops[idx].op2_def >= 0) {
					ssa_vars[ssa_ops[idx].op2_def].alias = crex_jit_var_may_alias(op_array, ssa, EX_VAR_TO_NUM(opline->op2.var));
					if (ssa_ops[idx].op2_use < 0 || !(ssa_var_info[ssa_ops[idx].op2_use].type & MAY_BE_REF)) {
						crex_jit_trace_propagate_range(op_array, ssa_opcodes, tssa, ssa_ops[idx].op2_def);
					}
				}
				if (ssa_ops[idx].result_def >= 0) {
					ssa_vars[ssa_ops[idx].result_def].alias = crex_jit_var_may_alias(op_array, ssa, EX_VAR_TO_NUM(opline->result.var));
					if (ssa_ops[idx].result_use < 0 || !(ssa_var_info[ssa_ops[idx].result_use].type & MAY_BE_REF)) {
						crex_jit_trace_propagate_range(op_array, ssa_opcodes, tssa, ssa_ops[idx].result_def);
					}
				}
				if (len == 2 && (opline+1)->opcode == CREX_OP_DATA) {
					if (ssa_ops[idx+1].op1_def >= 0) {
						ssa_vars[ssa_ops[idx+1].op1_def].alias = crex_jit_var_may_alias(op_array, ssa, EX_VAR_TO_NUM((opline+1)->op1.var));
						if (ssa_ops[idx+1].op1_use < 0 || !(ssa_var_info[ssa_ops[idx+1].op1_use].type & MAY_BE_REF)) {
							crex_jit_trace_propagate_range(op_array, ssa_opcodes, tssa, ssa_ops[idx+1].op1_def);
						}
					}
					if (ssa_ops[idx+1].op2_def >= 0) {
						ssa_vars[ssa_ops[idx+1].op2_def].alias = crex_jit_var_may_alias(op_array, ssa, EX_VAR_TO_NUM((opline+1)->op2.var));
						if (ssa_ops[idx+1].op2_use < 0 || !(ssa_var_info[ssa_ops[idx+1].op2_use].type & MAY_BE_REF)) {
							crex_jit_trace_propagate_range(op_array, ssa_opcodes, tssa, ssa_ops[idx+1].op2_def);
						}
					}
					if (ssa_ops[idx+1].result_def >= 0) {
						ssa_vars[ssa_ops[idx+1].result_def].alias = crex_jit_var_may_alias(op_array, ssa, EX_VAR_TO_NUM((opline+1)->result.var));
						if (ssa_ops[idx+1].result_use < 0 || !(ssa_var_info[ssa_ops[idx+1].result_use].type & MAY_BE_REF)) {
							crex_jit_trace_propagate_range(op_array, ssa_opcodes, tssa, ssa_ops[idx+1].result_def);
						}
					}
				}
			}
			if (opline->opcode == CREX_RECV_INIT
			 && !(op_array->fn_flags & CREX_ACC_HAS_TYPE_HINTS)) {
				/* RECV_INIT always copy the constant */
				ssa_var_info[ssa_ops[idx].result_def].type = _const_op_type(RT_CONSTANT(opline, opline->op2));
			} else if ((opline->opcode == CREX_FE_FETCH_R || opline->opcode == CREX_FE_FETCH_RW)
			 && ssa_opcodes[idx + 1] == CREX_OFFSET_TO_OPLINE(opline, opline->extended_value)) {
				if (ssa_ops[idx].op2_use >= 0 && ssa_ops[idx].op2_def >= 0) {
					ssa_var_info[ssa_ops[idx].op2_def] = ssa_var_info[ssa_ops[idx].op2_use];
				}
			} else {
				if (crex_update_type_info(op_array, tssa, script, (crex_op*)opline, ssa_ops + idx, ssa_opcodes, optimization_level) == FAILURE) {
					// TODO:
					assert(0);
				}
				if (opline->opcode == CREX_ASSIGN_DIM_OP
				 && ssa_ops[idx].op1_def > 0
				 && op1_type == IS_ARRAY
				 && (orig_op1_type & IS_TRACE_PACKED)
				 && val_type != IS_UNKNOWN
				 && val_type != IS_UNDEF
				 && ((opline->op2_type == IS_CONST
				   && C_TYPE_P(RT_CONSTANT(opline, opline->op2)) == IS_LONG)
				  || (opline->op2_type != IS_CONST
				   && op2_type == IS_LONG))) {
					crex_ssa_var_info *info = &ssa_var_info[ssa_ops[idx].op1_def];

					info->type &= ~(MAY_BE_ARRAY_NUMERIC_HASH|MAY_BE_ARRAY_STRING_HASH);
				}
			}
			if (ssa->var_info) {
				/* Add statically inferred restrictions */
				if (ssa_ops[idx].op1_def >= 0) {
					if (opline->opcode == CREX_SEND_VAR_EX
					 && frame
					 && frame->call
					 && frame->call->func
					 && !ARG_SHOULD_BE_SENT_BY_REF(frame->call->func, opline->op2.num)) {
						ssa_var_info[ssa_ops[idx].op1_def] = ssa_var_info[ssa_ops[idx].op1_use];
						ssa_var_info[ssa_ops[idx].op1_def].type &= ~MAY_BE_GUARD;
						if (ssa_var_info[ssa_ops[idx].op1_def].type & MAY_BE_RC1) {
							ssa_var_info[ssa_ops[idx].op1_def].type |= MAY_BE_RCN;
						}
					} else {
						crex_jit_trace_restrict_ssa_var_info(op_array, ssa, ssa_opcodes, tssa, ssa_ops[idx].op1_def);
					}
				}
				if (ssa_ops[idx].op2_def >= 0) {
					if ((opline->opcode != CREX_FE_FETCH_R && opline->opcode != CREX_FE_FETCH_RW)
					 || ssa_opcodes[idx + 1] != CREX_OFFSET_TO_OPLINE(opline, opline->extended_value)) {
						crex_jit_trace_restrict_ssa_var_info(op_array, ssa, ssa_opcodes, tssa, ssa_ops[idx].op2_def);
					}
				}
				if (ssa_ops[idx].result_def >= 0) {
					crex_jit_trace_restrict_ssa_var_info(op_array, ssa, ssa_opcodes, tssa, ssa_ops[idx].result_def);
				}
			}
			idx++;
			while (len > 1) {
				opline++;
				if (opline->opcode != CREX_OP_DATA) {
					if (ssa->var_info) {
						/* Add statically inferred ranges */
						if (ssa_ops[idx].op1_def >= 0) {
							crex_jit_trace_copy_ssa_var_range(op_array, ssa, ssa_opcodes, tssa, ssa_ops[idx].op1_def);
						}
						if (ssa_ops[idx].op2_def >= 0) {
							crex_jit_trace_copy_ssa_var_range(op_array, ssa, ssa_opcodes, tssa, ssa_ops[idx].op2_def);
						}
						if (ssa_ops[idx].result_def >= 0) {
							crex_jit_trace_copy_ssa_var_range(op_array, ssa, ssa_opcodes, tssa, ssa_ops[idx].result_def);
						}
					} else {
						if (ssa_ops[idx].op1_def >= 0) {
							ssa_vars[ssa_ops[idx].op1_def].alias = crex_jit_var_may_alias(op_array, ssa, EX_VAR_TO_NUM(opline->op1.var));
							if (ssa_ops[idx].op1_use < 0 || !(ssa_var_info[ssa_ops[idx].op1_use].type & MAY_BE_REF)) {
								crex_jit_trace_propagate_range(op_array, ssa_opcodes, tssa, ssa_ops[idx].op1_def);
							}
						}
						if (ssa_ops[idx].op2_def >= 0) {
							ssa_vars[ssa_ops[idx].op2_def].alias = crex_jit_var_may_alias(op_array, ssa, EX_VAR_TO_NUM(opline->op2.var));
							if (ssa_ops[idx].op2_use < 0 || !(ssa_var_info[ssa_ops[idx].op2_use].type & MAY_BE_REF)) {
								crex_jit_trace_propagate_range(op_array, ssa_opcodes, tssa, ssa_ops[idx].op2_def);
							}
						}
						if (ssa_ops[idx].result_def >= 0) {
							ssa_vars[ssa_ops[idx].result_def].alias = crex_jit_var_may_alias(op_array, ssa, EX_VAR_TO_NUM(opline->result.var));
							if (ssa_ops[idx].result_use < 0 || !(ssa_var_info[ssa_ops[idx].result_use].type & MAY_BE_REF)) {
								crex_jit_trace_propagate_range(op_array, ssa_opcodes, tssa, ssa_ops[idx].result_def);
							}
						}
					}
					if (opline->opcode == CREX_RECV_INIT
					 && !(op_array->fn_flags & CREX_ACC_HAS_TYPE_HINTS)) {
						/* RECV_INIT always copy the constant */
						ssa_var_info[ssa_ops[idx].result_def].type = _const_op_type(RT_CONSTANT(opline, opline->op2));
					} else {
						if (crex_update_type_info(op_array, tssa, script, (crex_op*)opline, ssa_ops + idx, ssa_opcodes, optimization_level) == FAILURE) {
							// TODO:
							assert(0);
						}
					}
				}
				if (ssa->var_info) {
					/* Add statically inferred restrictions */
					if (ssa_ops[idx].op1_def >= 0) {
						crex_jit_trace_restrict_ssa_var_info(op_array, ssa, ssa_opcodes, tssa, ssa_ops[idx].op1_def);
					}
					if (ssa_ops[idx].op2_def >= 0) {
						crex_jit_trace_restrict_ssa_var_info(op_array, ssa, ssa_opcodes, tssa, ssa_ops[idx].op2_def);
					}
					if (ssa_ops[idx].result_def >= 0) {
						crex_jit_trace_restrict_ssa_var_info(op_array, ssa, ssa_opcodes, tssa, ssa_ops[idx].result_def);
					}
				}
				idx++;
				len--;
			}

		} else if (p->op == CREX_JIT_TRACE_ENTER) {
			op_array = p->op_array;
			jit_extension =
				(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
			ssa = &jit_extension->func_info.ssa;

			call = frame->call;
			if (!call) {
				/* Trace missed INIT_FCALL opcode */
				call = top;
				TRACE_FRAME_INIT(call, op_array, 0, 0);
				call->used_stack = 0;
				top = crex_jit_trace_call_frame(top, op_array);
				for (i = 0; i < op_array->last_var + op_array->T; i++) {
					SET_STACK_TYPE(call->stack, i, IS_UNKNOWN, 1);
				}
			} else {
				CREX_ASSERT(&call->func->op_array == op_array);
			}
			frame->call = call->prev;
			call->prev = frame;
			TRACE_FRAME_SET_RETURN_SSA_VAR(call, find_return_ssa_var(p - 1, ssa_ops + (idx - 1)));
			frame = call;

			level++;
			i = 0;
			v = CREX_JIT_TRACE_GET_FIRST_SSA_VAR(p->info);
			while (i < op_array->last_var) {
				ssa_vars[v].var = i;
				if (i < op_array->num_args) {
					if (ssa->var_info
					 && crex_jit_trace_copy_ssa_var_info(op_array, ssa, ssa_opcodes, tssa, v)) {
						/* pass */
					} else {
						ssa_vars[v].alias = crex_jit_var_may_alias(op_array, ssa, i);
						if (op_array->arg_info) {
							crex_arg_info *arg_info = &op_array->arg_info[i];
							crex_class_entry *ce;
							uint32_t tmp = crex_fetch_arg_info_type(script, arg_info, &ce);

							if (CREX_ARG_SEND_MODE(arg_info)) {
								tmp |= MAY_BE_REF;
							}
							ssa_var_info[v].type = tmp;
							ssa_var_info[v].ce = ce;
							ssa_var_info[v].is_instanceof = 1;
						} else {
							ssa_var_info[v].type = MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_REF | MAY_BE_ANY  | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
						}
					}
				} else {
					if (ssa->vars) {
						ssa_vars[v].no_val = ssa->vars[i].no_val;
						ssa_vars[v].alias = ssa->vars[i].alias;
					} else {
						ssa_vars[v].alias = crex_jit_var_may_alias(op_array, ssa, i);
					}
					if (ssa_vars[v].alias == NO_ALIAS) {
						ssa_var_info[v].type = MAY_BE_UNDEF;
					} else {
						ssa_var_info[v].type = MAY_BE_UNDEF | MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_REF | MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
					}
				}
				if (!(op_array->fn_flags & CREX_ACC_HAS_TYPE_HINTS)
				 && i < op_array->num_args) {
					/* Propagate argument type */
					ssa_var_info[v].type &= STACK_INFO(frame->stack, i);
				}
				i++;
				v++;
			}
		} else if (p->op == CREX_JIT_TRACE_BACK) {
			op_array = p->op_array;
			jit_extension =
				(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
			ssa = &jit_extension->func_info.ssa;
			if (level == 0) {
				i = 0;
				v = CREX_JIT_TRACE_GET_FIRST_SSA_VAR(p->info);
				while (i < op_array->last_var) {
					ssa_vars[v].var = i;
					if (!ssa->var_info
					 || !crex_jit_trace_copy_ssa_var_info(op_array, ssa, ssa_opcodes, tssa, v)) {
						ssa_var_info[v].type = MAY_BE_UNDEF | MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_REF | MAY_BE_ANY  | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
					}
					i++;
					v++;
				}
				while (i < op_array->last_var + op_array->T) {
					ssa_vars[v].var = i;
					if (!ssa->var_info
					 || !crex_jit_trace_copy_ssa_var_info(op_array, ssa, ssa_opcodes, tssa, v)) {
						ssa_var_info[v].type = MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_REF | MAY_BE_ANY  | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
					}
					i++;
					v++;
				}
				if (return_value_info.type != 0) {
					crex_jit_trace_rec *q = p + 1;
					while (q->op == CREX_JIT_TRACE_INIT_CALL) {
						q++;
					}
					if (q->op == CREX_JIT_TRACE_VM
					 || (q->op == CREX_JIT_TRACE_END
					  && q->stop == CREX_JIT_TRACE_STOP_RECURSIVE_RET)) {
						const crex_op *opline = q->opline - 1;
						if (opline->result_type != IS_UNUSED) {
							ssa_var_info[
								CREX_JIT_TRACE_GET_FIRST_SSA_VAR(p->info) +
								EX_VAR_TO_NUM(opline->result.var)] = return_value_info;
						}
					}
					memset(&return_value_info, 0, sizeof(return_value_info));
				}
			} else {
				level--;
				if (return_value_info.type != 0) {
					if ((p+1)->op == CREX_JIT_TRACE_VM) {
						const crex_op *opline = (p+1)->opline - 1;
						if (opline->result_type != IS_UNUSED) {
							if (TRACE_FRAME_RETURN_SSA_VAR(frame) >= 0) {
								ssa_var_info[TRACE_FRAME_RETURN_SSA_VAR(frame)] = return_value_info;
							}
						}
					}
					memset(&return_value_info, 0, sizeof(return_value_info));
				}
			}

			top = frame;
			if (frame->prev) {
				if (used_stack > 0) {
					used_stack -= frame->used_stack;
				}
				frame = frame->prev;
				CREX_ASSERT(&frame->func->op_array == op_array);
			} else {
				max_used_stack = used_stack = -1;
				frame = crex_jit_trace_ret_frame(frame, op_array);
				TRACE_FRAME_INIT(frame, op_array, 0, 0);
				TRACE_FRAME_SET_RETURN_SSA_VAR(frame, -1);
				frame->used_stack = 0;
				for (i = 0; i < op_array->last_var + op_array->T; i++) {
					SET_STACK_TYPE(frame->stack, i, IS_UNKNOWN, 1);
				}
			}

		} else if (p->op == CREX_JIT_TRACE_INIT_CALL) {
			call = top;
			TRACE_FRAME_INIT(call, p->func, 0, 0);
			call->prev = frame->call;
			call->used_stack = 0;
			frame->call = call;
			top = crex_jit_trace_call_frame(top, p->op_array);
			if (p->func && p->func->type == CREX_USER_FUNCTION) {
				for (i = 0; i < p->op_array->last_var + p->op_array->T; i++) {
					SET_STACK_INFO(call->stack, i, -1);
				}
			}
			if (used_stack >= 0
			 && !(p->info & CREX_JIT_TRACE_FAKE_INIT_CALL)) {
				if (p->func == NULL || (p-1)->op != CREX_JIT_TRACE_VM) {
					max_used_stack = used_stack = -1;
				} else {
					const crex_op *opline = (p-1)->opline;

					switch (opline->opcode) {
						case CREX_INIT_FCALL:
						case CREX_INIT_FCALL_BY_NAME:
						case CREX_INIT_NS_FCALL_BY_NAME:
						case CREX_INIT_METHOD_CALL:
						case CREX_INIT_DYNAMIC_CALL:
						//case CREX_INIT_STATIC_METHOD_CALL:
						//case CREX_INIT_USER_CALL:
						//case CREX_NEW:
							frame->used_stack = crex_vm_calc_used_stack(opline->extended_value, (crex_function*)p->func);
							used_stack += frame->used_stack;
							if (used_stack > max_used_stack) {
								max_used_stack = used_stack;
							}
							break;
						default:
							max_used_stack = used_stack = -1;
					}
				}
			}
		} else if (p->op == CREX_JIT_TRACE_DO_ICALL) {
			call = frame->call;
			if (call) {
				top = call;
				frame->call = call->prev;
			}

			if (idx > 0
			 && ssa_ops[idx-1].result_def >= 0
			 && (p->func->common.fn_flags & CREX_ACC_HAS_RETURN_TYPE)
			 && !(p->func->common.fn_flags & CREX_ACC_RETURN_REFERENCE)) {
				CREX_ASSERT(ssa_opcodes[idx-1] == opline);
				CREX_ASSERT(opline->opcode == CREX_DO_ICALL ||
					opline->opcode == CREX_DO_FCALL ||
					opline->opcode == CREX_DO_FCALL_BY_NAME);

				if (opline->result_type != IS_UNDEF) {
					crex_class_entry *ce;
					const crex_function *func = p->func;
					crex_arg_info *ret_info = func->common.arg_info - 1;
					uint32_t ret_type = crex_fetch_arg_info_type(NULL, ret_info, &ce);

					ssa_var_info[ssa_ops[idx-1].result_def].type &= ret_type;
				}
			}
		} else if (p->op == CREX_JIT_TRACE_END) {
			break;
		}
	}

	((crex_tssa*)tssa)->used_stack = max_used_stack;

	if (trace_buffer->stop == CREX_JIT_TRACE_STOP_LOOP
	 || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_CALL
	 || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_RET) {
		/* Propagate guards through Phi sources */
		crex_ssa_phi *phi = tssa->blocks[1].phis;

		op_array = trace_buffer->op_array;
		jit_extension =
			(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
		ssa = &jit_extension->func_info.ssa;

		while (phi) {
			uint32_t t = ssa_var_info[phi->ssa_var].type;

			if ((t & MAY_BE_GUARD) && tssa->vars[phi->ssa_var].alias == NO_ALIAS) {
				uint32_t t0 = ssa_var_info[phi->sources[0]].type;
				uint32_t t1 = ssa_var_info[phi->sources[1]].type;

				if (((t0 | t1) & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_REF)) == (t & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_REF))) {
					if (!((t0 | t1) & MAY_BE_GUARD)) {
						ssa_var_info[phi->ssa_var].type = t & ~MAY_BE_GUARD;
					}
				} else if ((t1 & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_REF)) == (t & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_REF))) {
					if (!(t1 & MAY_BE_GUARD)
					 || is_checked_guard(tssa, ssa_opcodes, phi->sources[1], phi->ssa_var)) {
						ssa_var_info[phi->ssa_var].type = t & ~MAY_BE_GUARD;
						t0 = (t & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_REF)) |
							(t0 & ~(MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_REF)) |
							MAY_BE_GUARD;
						if (!(t0 & MAY_BE_ARRAY)) {
							t0 &= ~(MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF|MAY_BE_ARRAY_KEY_ANY);
						}
						if (!(t0 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE))) {
							t0 &= ~(MAY_BE_RC1|MAY_BE_RCN);
						}
						ssa_var_info[phi->sources[0]].type = t0;
						ssa_var_info[phi->sources[0]].type = t0;
					}
				} else {
					if ((t0 & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_REF)) != (t & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_REF))) {
						t0 = (t & t0 & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_REF)) |
							(t0 & ~(MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_REF)) |
							MAY_BE_GUARD;
						if (!(t0 & MAY_BE_ARRAY)) {
							t0 &= ~(MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF|MAY_BE_ARRAY_KEY_ANY);
						}
						if (!(t0 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE))) {
							t0 &= ~(MAY_BE_RC1|MAY_BE_RCN);
						}
						ssa_var_info[phi->sources[0]].type = t0;
					}
					if ((t1 & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_REF)) != (t & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_REF))) {
						if (((t & t1) & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_REF)) != 0
						 && is_checked_guard(tssa, ssa_opcodes, phi->sources[1], phi->ssa_var)) {
							t1 = (t & t1 & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_REF)) |
								(t1 & ~(MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_REF)) |
								MAY_BE_GUARD;
							if (!(t1 & MAY_BE_ARRAY)) {
								t1 &= ~(MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF|MAY_BE_ARRAY_KEY_ANY);
							}
							if (!(t1 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE))) {
								t1 &= ~(MAY_BE_RC1|MAY_BE_RCN);
							}
							ssa_var_info[phi->sources[1]].type = t1;
							ssa_var_info[phi->ssa_var].type = t & ~MAY_BE_GUARD;
						}
					}
				}
				t = ssa_var_info[phi->ssa_var].type;
			}

			if ((t & MAY_BE_PACKED_GUARD) && tssa->vars[phi->ssa_var].alias == NO_ALIAS) {
				uint32_t t0 = ssa_var_info[phi->sources[0]].type;
				uint32_t t1 = ssa_var_info[phi->sources[1]].type;

				if (((t0 | t1) & MAY_BE_ARRAY_KEY_ANY) == (t & MAY_BE_ARRAY_KEY_ANY)) {
					if (!((t0 | t1) & MAY_BE_PACKED_GUARD)) {
						ssa_var_info[phi->ssa_var].type = t & ~MAY_BE_PACKED_GUARD;
					}
				} else if ((t1 & MAY_BE_ARRAY_KEY_ANY) == (t & MAY_BE_ARRAY_KEY_ANY)) {
					if (!(t1 & MAY_BE_PACKED_GUARD)) {
						ssa_var_info[phi->ssa_var].type = t & ~MAY_BE_PACKED_GUARD;
						ssa_var_info[phi->sources[0]].type =
							(t0 & ~MAY_BE_ARRAY_KEY_ANY) | (t & MAY_BE_ARRAY_KEY_ANY) | MAY_BE_PACKED_GUARD;
					}
				}
			}
			phi = phi->next;
		}
	}

	if (UNEXPECTED(JIT_G(debug) & CREX_JIT_DEBUG_TRACE_TSSA)) {
		if (parent_trace) {
			fprintf(stderr, "---- TRACE %d TSSA start (side trace %d/%d) %s%s%s() %s:%d\n",
				CREX_JIT_TRACE_NUM,
				parent_trace,
				exit_num,
				trace_buffer->op_array->scope ? ZSTR_VAL(trace_buffer->op_array->scope->name) : "",
				trace_buffer->op_array->scope ? "::" : "",
				trace_buffer->op_array->function_name ?
					ZSTR_VAL(trace_buffer->op_array->function_name) : "$main",
				ZSTR_VAL(trace_buffer->op_array->filename),
				trace_buffer[1].opline->lineno);
		} else {
			fprintf(stderr, "---- TRACE %d TSSA start (%s) %s%s%s() %s:%d\n",
				CREX_JIT_TRACE_NUM,
				crex_jit_trace_star_desc(trace_buffer->start),
				trace_buffer->op_array->scope ? ZSTR_VAL(trace_buffer->op_array->scope->name) : "",
				trace_buffer->op_array->scope ? "::" : "",
				trace_buffer->op_array->function_name ?
					ZSTR_VAL(trace_buffer->op_array->function_name) : "$main",
				ZSTR_VAL(trace_buffer->op_array->filename),
				trace_buffer[1].opline->lineno);
		}
		crex_jit_dump_trace(trace_buffer, tssa);
		if (trace_buffer->stop == CREX_JIT_TRACE_STOP_LINK) {
			uint32_t idx = trace_buffer[1].last;
			uint32_t link_to = crex_jit_find_trace(trace_buffer[idx].opline->handler);
			fprintf(stderr, "---- TRACE %d TSSA stop (link to %d)\n",
				CREX_JIT_TRACE_NUM,
				link_to);
		} else {
			fprintf(stderr, "---- TRACE %d TSSA stop (%s)\n",
				CREX_JIT_TRACE_NUM,
				crex_jit_trace_stop_description[trace_buffer->stop]);
		}
	}

	return tssa;
}

static void crex_jit_close_var(crex_jit_trace_stack *stack, uint32_t n, int *start, int *end, uint8_t *flags, int line)
{
	int32_t var = STACK_VAR(stack, n);

	if (var >= 0 && start[var] >= 0 && !(flags[var] & ZREG_LAST_USE)) {
		// TODO: shrink interval to last side exit ????
		end[var] = line;
	}
}

static void crex_jit_trace_use_var(int line, int var, int def, int use_chain, int *start, int *end, uint8_t *flags, const crex_ssa *ssa, const crex_op **ssa_opcodes, const crex_op_array *op_array, const crex_ssa *op_array_ssa)
{
	CREX_ASSERT(start[var] >= 0);
	CREX_ASSERT(!(flags[var] & ZREG_LAST_USE));
	end[var] = line;
	if (def >= 0) {
		flags[var] |= ZREG_LAST_USE;
	} else if (use_chain < 0 && (flags[var] & (ZREG_LOAD|ZREG_STORE))) {
		flags[var] |= ZREG_LAST_USE;
	} else if (use_chain >= 0 && !crex_ssa_is_no_val_use(ssa_opcodes[use_chain], ssa->ops + use_chain, var)) {
		/* pass */
	} else if (op_array_ssa->vars) {
		uint32_t use = ssa_opcodes[line] - op_array->opcodes;

		if (ssa->ops[line].op1_use == var) {
			if (crex_ssa_is_last_use(op_array, op_array_ssa, op_array_ssa->ops[use].op1_use, use)) {
				flags[var] |= ZREG_LAST_USE;
			}
		} else if (ssa->ops[line].op2_use == var) {
			if (crex_ssa_is_last_use(op_array, op_array_ssa, op_array_ssa->ops[use].op2_use, use)) {
				flags[var] |= ZREG_LAST_USE;
			}
		} else if (ssa->ops[line].result_use == var) {
			if (crex_ssa_is_last_use(op_array, op_array_ssa, op_array_ssa->ops[use].result_use, use)) {
				flags[var] |= ZREG_LAST_USE;
			}
		}
	}
}

static crex_lifetime_interval** crex_jit_trace_allocate_registers(crex_jit_trace_rec *trace_buffer, crex_ssa *ssa, uint32_t parent_trace, uint32_t exit_num)
{
	const crex_op **ssa_opcodes = ((crex_tssa*)ssa)->tssa_opcodes;
	crex_jit_trace_rec *p;
	const crex_op_array *op_array;
	crex_jit_op_array_trace_extension *jit_extension;
	const crex_ssa *op_array_ssa;
	const crex_ssa_op *ssa_op;
	int i, j, idx, count, level;
	int last_idx = -1;
	int *start, *end;
	uint8_t *flags;
	const crex_op_array **vars_op_array;
	crex_lifetime_interval **intervals, *list, *ival;
	void *checkpoint;
	crex_jit_trace_stack_frame *frame;
	crex_jit_trace_stack *stack;
	uint32_t parent_vars_count = parent_trace ?
		crex_jit_traces[parent_trace].exit_info[exit_num].stack_size : 0;
	crex_jit_trace_stack *parent_stack = parent_trace ?
		crex_jit_traces[parent_trace].stack_map +
		crex_jit_traces[parent_trace].exit_info[exit_num].stack_offset : NULL;
	ALLOCA_FLAG(use_heap);

	CREX_ASSERT(ssa->var_info != NULL);

	start = do_alloca(sizeof(int) * ssa->vars_count * 2 +
		CREX_MM_ALIGNED_SIZE(sizeof(uint8_t) * ssa->vars_count),
		use_heap);
	if (!start) {
		return NULL;
	}
	end = start + ssa->vars_count;
	flags = (uint8_t*)(end + ssa->vars_count);
	checkpoint = crex_arena_checkpoint(CG(arena));
	vars_op_array = crex_arena_calloc(&CG(arena), ssa->vars_count, sizeof(crex_op_array*));

	memset(start, -1, sizeof(int) * ssa->vars_count * 2);
	memset(flags, 0, sizeof(uint8_t) * ssa->vars_count);
	memset(CREX_VOIDP(vars_op_array), 0, sizeof(crex_op_array*) * ssa->vars_count);

	op_array = trace_buffer->op_array;
	jit_extension =
		(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
	op_array_ssa = &jit_extension->func_info.ssa;
	frame = JIT_G(current_frame);
	frame->prev = NULL;
	frame->func = (const crex_function*)op_array;
	stack = frame->stack;

	count = 0;

	i = 0;
	j = op_array->last_var;
	if (trace_buffer->start != CREX_JIT_TRACE_START_ENTER) {
		j += op_array->T;
	}
	while (i < j) {
		SET_STACK_VAR(stack, i, i);
		vars_op_array[i] = op_array;
		/* We don't start intervals for variables used in Phi */
		if ((ssa->vars[i].use_chain >= 0 /*|| ssa->vars[i].phi_use_chain*/)
		 && !crex_ssa_is_no_val_use(ssa_opcodes[ssa->vars[i].use_chain], ssa->ops + ssa->vars[i].use_chain, i)
		 && ssa->vars[i].alias == NO_ALIAS
		 && crex_jit_var_supports_reg(ssa, i)) {
			start[i] = 0;
			if (i < parent_vars_count
			 && STACK_REG(parent_stack, i) != ZREG_NONE
			 && STACK_REG(parent_stack, i) < ZREG_NUM) {
				/* We will try to reuse register from parent trace */
				flags[i] = STACK_FLAGS(parent_stack, i);
				count += 2;
			} else {
				flags[i] = ZREG_LOAD;
				count++;
			}
		}
		i++;
	}

	if (trace_buffer->start == CREX_JIT_TRACE_START_ENTER) {
		j = op_array->last_var + op_array->T;
		while (i < j) {
			SET_STACK_VAR(stack, i, -1);
			i++;
		}
	}

	if (trace_buffer->stop == CREX_JIT_TRACE_STOP_LOOP
	 || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_CALL
	 || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_RET) {
		crex_ssa_phi *phi = ssa->blocks[1].phis;

		while (phi) {
			SET_STACK_VAR(stack, phi->var, phi->ssa_var);
			vars_op_array[phi->ssa_var] = op_array;
			if (ssa->vars[phi->ssa_var].use_chain >= 0
			 && ssa->vars[phi->ssa_var].alias == NO_ALIAS
			 && crex_jit_var_supports_reg(ssa, phi->ssa_var)) {
				start[phi->ssa_var] = 0;
				count++;
			}
			phi = phi->next;
		}
	}

	p = trace_buffer + CREX_JIT_TRACE_START_REC_SIZE;
	level = 0;
	ssa_op = ssa->ops;
	idx = 0;
	for (;;p++) {
		if (p->op == CREX_JIT_TRACE_VM) {
			const crex_op *opline = p->opline;
			int len;
			bool support_opline;

			support_opline =
				crex_jit_opline_supports_reg(op_array, ssa, opline, ssa_op, p);

			if (support_opline
			 && opline->opcode == CREX_ASSIGN
			 && opline->op1_type == IS_CV
			 && ssa_op->op1_def >= 0
			 && ssa->vars[ssa_op->op1_def].alias != NO_ALIAS) {
				/* avoid register allocation in case of possibility of indirect modification*/
				support_opline = 0;
			}

			if (ssa_op->op1_use >= 0
			 && start[ssa_op->op1_use] >= 0
			 && !crex_ssa_is_no_val_use(opline, ssa_op, ssa_op->op1_use)) {
				if (support_opline) {
					crex_jit_trace_use_var(idx, ssa_op->op1_use, ssa_op->op1_def, ssa_op->op1_use_chain, start, end, flags, ssa, ssa_opcodes, op_array, op_array_ssa);
					if (opline->op1_type != IS_CV) {
						if (opline->opcode == CREX_CASE
						 || opline->opcode == CREX_CASE_STRICT
						 || opline->opcode == CREX_SWITCH_LONG
						 || opline->opcode == CREX_MATCH
						 || opline->opcode == CREX_FETCH_LIST_R
						 || opline->opcode == CREX_COPY_TMP
						 || opline->opcode == CREX_SWITCH_STRING
						 || opline->opcode == CREX_FE_FETCH_R
						 || opline->opcode == CREX_FE_FETCH_RW
						 || opline->opcode == CREX_FETCH_LIST_W
						 || opline->opcode == CREX_VERIFY_RETURN_TYPE
						 || opline->opcode == CREX_BIND_LEXICAL
						 || opline->opcode == CREX_ROPE_ADD) {
							/* The value is kept alive and may be used outside of the trace */
							flags[ssa_op->op1_use] |= ZREG_STORE;
						} else {
							flags[ssa_op->op1_use] |= ZREG_LAST_USE;
						}
					}
				} else {
					start[ssa_op->op1_use] = -1;
					end[ssa_op->op1_use] = -1;
					count--;
				}
			}
			if (ssa_op->op2_use >= 0
			 && ssa_op->op2_use != ssa_op->op1_use
			 && start[ssa_op->op2_use] >= 0
			 && !crex_ssa_is_no_val_use(opline, ssa_op, ssa_op->op2_use)) {
				if (support_opline) {
					crex_jit_trace_use_var(idx, ssa_op->op2_use, ssa_op->op2_def, ssa_op->op2_use_chain, start, end, flags, ssa, ssa_opcodes, op_array, op_array_ssa);
					if (opline->op2_type != IS_CV) {
						flags[ssa_op->op2_use] |= ZREG_LAST_USE;
					}
				} else {
					start[ssa_op->op2_use] = -1;
					end[ssa_op->op2_use] = -1;
					count--;
				}
			}
			if (ssa_op->result_use >= 0
			 && ssa_op->result_use != ssa_op->op1_use
			 && ssa_op->result_use != ssa_op->op2_use
			 && start[ssa_op->result_use] >= 0
			 && !crex_ssa_is_no_val_use(opline, ssa_op, ssa_op->result_use)) {
				if (support_opline) {
					crex_jit_trace_use_var(idx, ssa_op->result_use, ssa_op->result_def, ssa_op->res_use_chain, start, end, flags, ssa, ssa_opcodes, op_array, op_array_ssa);
				} else {
					start[ssa_op->result_use] = -1;
					end[ssa_op->result_use] = -1;
					count--;
				}
			}

			if (ssa_op->op1_def >= 0) {
				crex_jit_close_var(stack, EX_VAR_TO_NUM(opline->op1.var), start, end, flags, idx);
				SET_STACK_VAR(stack, EX_VAR_TO_NUM(opline->op1.var), ssa_op->op1_def);
			}
			if (ssa_op->op2_def >= 0) {
				crex_jit_close_var(stack, EX_VAR_TO_NUM(opline->op2.var), start, end, flags, idx);
				SET_STACK_VAR(stack, EX_VAR_TO_NUM(opline->op2.var), ssa_op->op2_def);
			}
			if (ssa_op->result_def >= 0) {
				crex_jit_close_var(stack, EX_VAR_TO_NUM(opline->result.var), start, end, flags, idx);
				SET_STACK_VAR(stack, EX_VAR_TO_NUM(opline->result.var), ssa_op->result_def);
			}

			if (support_opline) {
				if (ssa_op->result_def >= 0
				 && (ssa->vars[ssa_op->result_def].use_chain >= 0
			      || ssa->vars[ssa_op->result_def].phi_use_chain)
				 && ssa->vars[ssa_op->result_def].alias == NO_ALIAS
				 && crex_jit_var_supports_reg(ssa, ssa_op->result_def)) {
					if (!(ssa->var_info[ssa_op->result_def].type & MAY_BE_GUARD)
					 || opline->opcode == CREX_PRE_INC
					 || opline->opcode == CREX_PRE_DEC
					 || opline->opcode == CREX_POST_INC
					 || opline->opcode == CREX_POST_DEC
					 || opline->opcode == CREX_ADD
					 || opline->opcode == CREX_SUB
					 || opline->opcode == CREX_MUL
					 || opline->opcode == CREX_FETCH_DIM_R
					 || opline->opcode == CREX_FETCH_CONSTANT) {
						if (!(ssa->var_info[ssa_op->result_def].type & MAY_BE_DOUBLE)
						 || (opline->opcode != CREX_PRE_INC && opline->opcode != CREX_PRE_DEC)) {
							start[ssa_op->result_def] = idx;
							vars_op_array[ssa_op->result_def] = op_array;
							count++;
						}
					}
				}
				if (ssa_op->op1_def >= 0
				 && (ssa->vars[ssa_op->op1_def].use_chain >= 0
			      || ssa->vars[ssa_op->op1_def].phi_use_chain)
				 && ssa->vars[ssa_op->op1_def].alias == NO_ALIAS
				 && crex_jit_var_supports_reg(ssa, ssa_op->op1_def)) {
					start[ssa_op->op1_def] = idx;
					vars_op_array[ssa_op->op1_def] = op_array;
					count++;
				}
				if (ssa_op->op2_def >= 0
				 && (ssa->vars[ssa_op->op2_def].use_chain >= 0
			      || ssa->vars[ssa_op->op2_def].phi_use_chain)
				 && ssa->vars[ssa_op->op2_def].alias == NO_ALIAS
				 && crex_jit_var_supports_reg(ssa, ssa_op->op2_def)) {
					start[ssa_op->op2_def] = idx;
					vars_op_array[ssa_op->op2_def] = op_array;
					count++;
				}
			}

			len = crex_jit_trace_op_len(opline);
			switch (opline->opcode) {
				case CREX_ASSIGN_DIM:
				case CREX_ASSIGN_OBJ:
				case CREX_ASSIGN_STATIC_PROP:
				case CREX_ASSIGN_DIM_OP:
				case CREX_ASSIGN_OBJ_OP:
				case CREX_ASSIGN_STATIC_PROP_OP:
				case CREX_ASSIGN_OBJ_REF:
				case CREX_ASSIGN_STATIC_PROP_REF:
					/* OP_DATA */
					ssa_op++;
					opline++;
					if (ssa_op->op1_use >= 0
					 && start[ssa_op->op1_use] >= 0
					 && !crex_ssa_is_no_val_use(opline, ssa_op, ssa_op->op1_use)) {
						if (support_opline) {
							crex_jit_trace_use_var(idx, ssa_op->op1_use, ssa_op->op1_def, ssa_op->op1_use_chain, start, end, flags, ssa, ssa_opcodes, op_array, op_array_ssa);
							if (opline->op1_type != IS_CV) {
								flags[ssa_op->op1_use] |= ZREG_LAST_USE;
							}
						} else {
							start[ssa_op->op1_use] = -1;
							end[ssa_op->op1_use] = -1;
							count--;
						}
					}
					if (ssa_op->op1_def >= 0) {
						crex_jit_close_var(stack, EX_VAR_TO_NUM(opline->op1.var), start, end, flags, idx);
						SET_STACK_VAR(stack, EX_VAR_TO_NUM(opline->op1.var), ssa_op->op1_def);
						if (support_opline
						 && (ssa->vars[ssa_op->op1_def].use_chain >= 0
					      || ssa->vars[ssa_op->op1_def].phi_use_chain)
						 && ssa->vars[ssa_op->op1_def].alias == NO_ALIAS
						 && crex_jit_var_supports_reg(ssa, ssa_op->op1_def)) {
							start[ssa_op->op1_def] = idx;
							vars_op_array[ssa_op->op1_def] = op_array;
							count++;
						}
					}
					ssa_op++;
					opline++;
					idx+=2;
					break;
				case CREX_RECV_INIT:
				    ssa_op++;
					opline++;
					idx++;
					while (opline->opcode == CREX_RECV_INIT) {
						/* RECV_INIT doesn't support registers */
						if (ssa_op->result_def >= 0) {
							crex_jit_close_var(stack, EX_VAR_TO_NUM(opline->result.var), start, end, flags, idx);
							SET_STACK_VAR(stack, EX_VAR_TO_NUM(opline->result.var), ssa_op->result_def);
						}
						ssa_op++;
						opline++;
						idx++;
					}
					break;
				case CREX_BIND_GLOBAL:
					ssa_op++;
					opline++;
					idx++;
					while (opline->opcode == CREX_BIND_GLOBAL) {
						/* BIND_GLOBAL doesn't support registers */
						if (ssa_op->op1_def >= 0) {
							crex_jit_close_var(stack, EX_VAR_TO_NUM(opline->op1.var), start, end, flags, idx);
							SET_STACK_VAR(stack, EX_VAR_TO_NUM(opline->op1.var), ssa_op->op1_def);
						}
						ssa_op++;
						opline++;
						idx++;
					}
					break;
				default:
					ssa_op += len;
					idx += len;
					break;
			}
		} else if (p->op == CREX_JIT_TRACE_ENTER) {
			/* New call frames */
			crex_jit_trace_stack_frame *prev_frame = frame;

			frame = crex_jit_trace_call_frame(frame, op_array);
			frame->prev = prev_frame;
			frame->func = (const crex_function*)p->op_array;
			stack = frame->stack;
			op_array = p->op_array;
			jit_extension =
				(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
			op_array_ssa = &jit_extension->func_info.ssa;
			j = CREX_JIT_TRACE_GET_FIRST_SSA_VAR(p->info);
			for (i = 0; i < op_array->last_var; i++) {
				SET_STACK_VAR(stack, i, j);
				vars_op_array[j] = op_array;
				if (ssa->vars[j].use_chain >= 0
				 && ssa->vars[j].alias == NO_ALIAS
				 && crex_jit_var_supports_reg(ssa, j)) {
					start[j] = idx;
					flags[j] = ZREG_LOAD;
					count++;
				}
				j++;
			}
			for (i = op_array->last_var; i < op_array->last_var + op_array->T; i++) {
				SET_STACK_VAR(stack, i, -1);
			}
			level++;
		} else if (p->op == CREX_JIT_TRACE_BACK) {
			/* Close exiting call frames */
			for (i = 0; i < op_array->last_var; i++) {
				crex_jit_close_var(stack, i, start, end, flags, idx-1);
			}
			op_array = p->op_array;
			jit_extension =
				(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
			op_array_ssa = &jit_extension->func_info.ssa;
			frame = crex_jit_trace_ret_frame(frame, op_array);
			stack = frame->stack;
			if (level == 0) {
				/* New return frames */
				frame->prev = NULL;
				frame->func = (const crex_function*)op_array;
				j = CREX_JIT_TRACE_GET_FIRST_SSA_VAR(p->info);
				for (i = 0; i < op_array->last_var + op_array->T; i++) {
					SET_STACK_VAR(stack, i, j);
					vars_op_array[j] = op_array;
					if (ssa->vars[j].use_chain >= 0
					 && ssa->vars[j].alias == NO_ALIAS
					 && crex_jit_var_supports_reg(ssa, j)) {
						start[j] = idx;
						flags[j] = ZREG_LOAD;
						count++;
					}
					j++;
				}
			} else {
				level--;
			}
		} else if (p->op == CREX_JIT_TRACE_END) {
			break;
		}
	}

	if (trace_buffer->stop == CREX_JIT_TRACE_STOP_LOOP
	 || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_CALL
	 || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_RET) {
		crex_ssa_phi *phi = ssa->blocks[1].phis;

		while (phi) {
			i = phi->sources[1];
			if (start[i] >= 0 && !ssa->vars[phi->ssa_var].no_val) {
				end[i] = idx;
				flags[i] &= ~ZREG_LAST_USE;
			}
			phi = phi->next;
		}

		if (trace_buffer->stop == CREX_JIT_TRACE_STOP_LOOP) {
			for (i = 0; i < op_array->last_var; i++) {
				if (start[i] >= 0 && !ssa->vars[i].phi_use_chain) {
					end[i] = idx;
					flags[i] &= ~ZREG_LAST_USE;
				} else {
					crex_jit_close_var(stack, i, start, end, flags, idx);
				}
			}
		}
	} else {
		last_idx = idx;
		for (i = 0; i < op_array->last_var; i++) {
			crex_jit_close_var(stack, i, start, end, flags, idx);
		}
		while (frame->prev) {
			frame = frame->prev;
			op_array = &frame->func->op_array;
			stack = frame->stack;
			for (i = 0; i < op_array->last_var; i++) {
				crex_jit_close_var(stack, i, start, end, flags, idx);
			}
		}
	}

	if (!count) {
		free_alloca(start, use_heap);
		crex_arena_release(&CG(arena), checkpoint);
		return NULL;
	}

	intervals = crex_arena_calloc(&CG(arena), ssa->vars_count, sizeof(crex_lifetime_interval));
	memset(intervals, 0, sizeof(crex_lifetime_interval*) * ssa->vars_count);
	list = crex_arena_alloc(&CG(arena), sizeof(crex_lifetime_interval) * count);
	j = 0;
	for (i = 0; i < ssa->vars_count; i++) {
		if (start[i] >= 0 && end[i] >= 0) {
			CREX_ASSERT(j < count);
			if ((flags[i] & ZREG_LOAD) &&
			    (flags[i] & ZREG_LAST_USE) &&
			    end[i] == ssa->vars[i].use_chain) {
				/* skip life range with single use */
				continue;
			}
			intervals[i] = &list[j];
			list[j].ssa_var = i;
			list[j].reg = ZREG_NONE;
			list[j].flags = flags[i];
			list[j].range.start = start[i];
			list[j].range.end = end[i];
			list[j].range.next = NULL;
			list[j].hint = NULL;
			list[j].used_as_hint = NULL;
			list[j].list_next = NULL;
			j++;
		}
	}
	count = j;
	free_alloca(start, use_heap);
	start = end = NULL;

	if (!count) {
		crex_arena_release(&CG(arena), checkpoint);
		return NULL;
	}

	/* Add hints */
	if (parent_vars_count) {
		i = trace_buffer->op_array->last_var;
		if (trace_buffer->start != CREX_JIT_TRACE_START_ENTER) {
			i += trace_buffer->op_array->T;
		}
		if ((uint32_t)i > parent_vars_count) {
			i = parent_vars_count;
		}
		while (i > 0) {
			i--;
			if (intervals[i]
			 && STACK_REG(parent_stack, i) != ZREG_NONE
			 && STACK_REG(parent_stack, i) < ZREG_NUM) {
				list[j].ssa_var = - 1;
				list[j].reg = STACK_REG(parent_stack, i);
				list[j].flags = 0;
				list[j].range.start = -1;
				list[j].range.end = -1;
				list[j].range.next = NULL;
				list[j].hint = NULL;
				list[j].used_as_hint = NULL;
				list[j].list_next = NULL;
				intervals[i]->hint = &list[j];
				j++;
			}
		}
	}

	if (trace_buffer->stop == CREX_JIT_TRACE_STOP_LOOP
	 || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_CALL
	 || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_RET) {
		crex_ssa_phi *phi = ssa->blocks[1].phis;

		while (phi) {
			if (intervals[phi->ssa_var]) {
				if (intervals[phi->sources[1]]
				 && (ssa->var_info[phi->sources[1]].type & MAY_BE_ANY) ==
						(ssa->var_info[phi->ssa_var].type & MAY_BE_ANY)) {
					intervals[phi->sources[1]]->hint = intervals[phi->ssa_var];
				}
			}
			phi = phi->next;
		}
	}

	for (i = 0; i < ssa->vars_count; i++) {
		if (intervals[i] && !intervals[i]->hint) {

			if (ssa->vars[i].definition >= 0) {
				uint32_t line = ssa->vars[i].definition;
				const crex_op *opline = ssa_opcodes[line];

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
						if (opline->op2_type == IS_CONST) {
							/* Named parameters not supported in JIT */
							break;
						}
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

								crex_ssa_phi *phi = ssa->vars[ssa->ops[line].op1_use].definition_phi;
								if (phi &&
								    intervals[phi->sources[1]] &&
								    intervals[phi->sources[1]]->hint == intervals[ssa->ops[line].op1_use]) {
									break;
								}
								crex_jit_add_hint(intervals, i, ssa->ops[line].op1_use);
							} else if (opline->opcode != CREX_SUB &&
							    ssa->ops[line].op2_use >= 0 &&
							    intervals[ssa->ops[line].op2_use] &&
							    ssa->ops[line].op2_use_chain < 0 &&
							    !ssa->vars[ssa->ops[line].op2_use].phi_use_chain &&
							    (ssa->var_info[i].type & MAY_BE_ANY) ==
							        (ssa->var_info[ssa->ops[line].op2_use].type & MAY_BE_ANY)) {

								crex_ssa_phi *phi = ssa->vars[ssa->ops[line].op2_use].definition_phi;
								if (phi &&
								    intervals[phi->sources[1]] &&
								    intervals[phi->sources[1]]->hint == intervals[ssa->ops[line].op2_use]) {
									break;
								}
								crex_jit_add_hint(intervals, i, ssa->ops[line].op2_use);
							}
						}
						break;
				}
			}
		}
	}

	list = crex_jit_sort_intervals(intervals, ssa->vars_count);

	if (list) {
		ival = list;
		while (ival) {
			if (ival->hint) {
				ival->hint->used_as_hint = ival;
			}
			ival = ival->list_next;
		}
	}

	if (list) {
		if (JIT_G(debug) & CREX_JIT_DEBUG_REG_ALLOC) {
			fprintf(stderr, "---- TRACE %d Live Ranges\n", CREX_JIT_TRACE_NUM);
			ival = list;
			while (ival) {
				crex_jit_dump_lifetime_interval(vars_op_array[ival->ssa_var], ssa, ival);
				ival = ival->list_next;
			}
		}
	}

	/* Linear Scan Register Allocation (op_array is not actually used, only fn_flags matters) */
	list = crex_jit_linear_scan(&dummy_op_array, ssa_opcodes, ssa, list);

	if (list) {
		crex_lifetime_interval *ival, *next;

		memset(intervals, 0, ssa->vars_count * sizeof(crex_lifetime_interval*));
		ival = list;
		count = 0;
		while (ival != NULL) {
			CREX_ASSERT(ival->reg != ZREG_NONE);
			count++;
			next = ival->list_next;
			ival->list_next = intervals[ival->ssa_var];
			intervals[ival->ssa_var] = ival;
			ival = next;
		}

		if (!count) {
			crex_arena_release(&CG(arena), checkpoint);
			return NULL;
		}

		/* Add LOAD flag to registers that can't reuse register from parent trace */
		if (parent_vars_count) {
			i = trace_buffer->op_array->last_var;
			if (trace_buffer->start != CREX_JIT_TRACE_START_ENTER) {
				i += trace_buffer->op_array->T;
			}
			if ((uint32_t)i > parent_vars_count) {
				i = parent_vars_count;
			}
			while (i > 0) {
				i--;
				if (intervals[i] && intervals[i]->reg != STACK_REG(parent_stack, i)) {
					intervals[i]->flags |= ZREG_LOAD;
				}
			}
		}

		/* SSA resolution */
		if (trace_buffer->stop == CREX_JIT_TRACE_STOP_LOOP
		 || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_CALL
		 || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_RET) {
			crex_ssa_phi *phi = ssa->blocks[1].phis;

			while (phi) {
				int def = phi->ssa_var;
				int use = phi->sources[1];

				if (intervals[def]) {
					if (!intervals[use]) {
						intervals[def]->flags |= ZREG_LOAD;
						if ((intervals[def]->flags & ZREG_LAST_USE)
						 && ssa->vars[def].use_chain >= 0
						 && ssa->vars[def].use_chain == intervals[def]->range.end) {
							/* remove interval used once */
							intervals[def] = NULL;
							count--;
						}
					} else if (intervals[def]->reg != intervals[use]->reg) {
						intervals[def]->flags |= ZREG_LOAD;
						if (ssa->vars[use].use_chain >= 0) {
							intervals[use]->flags |= ZREG_STORE;
						} else {
							intervals[use] = NULL;
							count--;
						}
					} else {
						use = phi->sources[0];
						CREX_ASSERT(!intervals[use]);
						intervals[use] = crex_arena_alloc(&CG(arena), sizeof(crex_lifetime_interval));
						intervals[use]->ssa_var = phi->sources[0];
						intervals[use]->reg = intervals[def]->reg;
						intervals[use]->flags = ZREG_LOAD;
						intervals[use]->range.start = 0;
						intervals[use]->range.end = 0;
						intervals[use]->range.next = NULL;
						intervals[use]->hint = NULL;
						intervals[use]->used_as_hint = NULL;
						intervals[use]->list_next = NULL;
					}
				} else if (intervals[use]
						&& (!ssa->vars[def].no_val
							|| ssa->var_info[def].type != ssa->var_info[use].type)) {
					if (ssa->vars[use].use_chain >= 0) {
						intervals[use]->flags |= ZREG_STORE;
					} else {
						intervals[use] = NULL;
						count--;
					}
				}
				phi = phi->next;
			}
		} else {
			for (i = 0; i < ssa->vars_count; i++) {
				if (intervals[i]
				 && intervals[i]->range.end == last_idx
				 && !(intervals[i]->flags & (ZREG_LOAD|ZREG_STORE))) {
					intervals[i]->flags |= ZREG_STORE;
				}
			}
		}

		if (!count) {
			crex_arena_release(&CG(arena), checkpoint);
			return NULL;
		}

		if (JIT_G(debug) & CREX_JIT_DEBUG_REG_ALLOC) {
			fprintf(stderr, "---- TRACE %d Allocated Live Ranges\n", CREX_JIT_TRACE_NUM);
			for (i = 0; i < ssa->vars_count; i++) {
				ival = intervals[i];
				while (ival) {
					crex_jit_dump_lifetime_interval(vars_op_array[ival->ssa_var], ssa, ival);
					ival = ival->list_next;
				}
			}
		}

		return intervals;
	}

	crex_arena_release(&CG(arena), checkpoint); //???
	return NULL;
}

static void crex_jit_trace_clenup_stack(crex_jit_trace_stack *stack, const crex_op *opline, const crex_ssa_op *ssa_op, const crex_ssa *ssa, crex_lifetime_interval **ra)
{
	uint32_t line = ssa_op - ssa->ops;

	if (ssa_op->op1_use >= 0
	 && ra[ssa_op->op1_use]
	 && ra[ssa_op->op1_use]->range.end == line) {
		SET_STACK_REG(stack, EX_VAR_TO_NUM(opline->op1.var), ZREG_NONE);
	}
	if (ssa_op->op2_use >= 0
	 && ra[ssa_op->op2_use]
	 && ra[ssa_op->op2_use]->range.end == line) {
		SET_STACK_REG(stack, EX_VAR_TO_NUM(opline->op2.var), ZREG_NONE);
	}
	if (ssa_op->result_use >= 0
	 && ra[ssa_op->result_use]
	 && ra[ssa_op->result_use]->range.end == line) {
		SET_STACK_REG(stack, EX_VAR_TO_NUM(opline->result.var), ZREG_NONE);
	}
}

static void crex_jit_trace_setup_ret_counter(const crex_op *opline, size_t offset)
{
	crex_op *next_opline = (crex_op*)(opline + 1);

	if (JIT_G(hot_return) && !CREX_OP_TRACE_INFO(next_opline, offset)->trace_flags) {
		CREX_ASSERT(crex_jit_ret_trace_counter_handler != NULL);
		if (!CREX_OP_TRACE_INFO(next_opline, offset)->counter) {
			CREX_OP_TRACE_INFO(next_opline, offset)->counter =
				&crex_jit_hot_counters[CREX_JIT_COUNTER_NUM];
			CREX_JIT_COUNTER_NUM = (CREX_JIT_COUNTER_NUM + 1) % CREX_HOT_COUNTERS_COUNT;
		}
		CREX_OP_TRACE_INFO(next_opline, offset)->trace_flags = CREX_JIT_TRACE_START_RETURN;
		next_opline->handler = (const void*)crex_jit_ret_trace_counter_handler;
	}
}

static bool crex_jit_may_delay_fetch_this(const crex_op_array *op_array, crex_ssa *ssa, const crex_op **ssa_opcodes, const crex_ssa_op *ssa_op)
{
	int var = ssa_op->result_def;
	int i;
	int use = ssa->vars[var].use_chain;
	const crex_op *opline;

	if (use < 0
	 || ssa->vars[var].phi_use_chain
	 || ssa->ops[use].op1_use != var
	 || ssa->ops[use].op1_use_chain != -1) {
		return 0;
	}

	opline = ssa_opcodes[use];
	if (opline->opcode == CREX_INIT_METHOD_CALL) {
		return (opline->op2_type == IS_CONST &&
			C_TYPE_P(RT_CONSTANT(opline, opline->op2)) == IS_STRING);
	} else if (opline->opcode == CREX_FETCH_OBJ_FUNC_ARG) {
		if (!JIT_G(current_frame)
		 || !JIT_G(current_frame)->call
		 || !JIT_G(current_frame)->call->func
		 || !TRACE_FRAME_IS_LAST_SEND_BY_VAL(JIT_G(current_frame)->call)) {
			return 0;
		}
	} else if (opline->opcode != CREX_FETCH_OBJ_R
			&& opline->opcode != CREX_FETCH_OBJ_IS
			&& opline->opcode != CREX_FETCH_OBJ_W
			&& opline->opcode != CREX_ASSIGN_OBJ
			&& opline->opcode != CREX_ASSIGN_OBJ_OP
			&& opline->opcode != CREX_PRE_INC_OBJ
			&& opline->opcode != CREX_PRE_DEC_OBJ
			&& opline->opcode != CREX_POST_INC_OBJ
			&& opline->opcode != CREX_POST_DEC_OBJ) {
		return 0;
	}

	if (opline->op2_type != IS_CONST
	 || C_TYPE_P(RT_CONSTANT(opline, opline->op2)) != IS_STRING
	 || C_STRVAL_P(RT_CONSTANT(opline, opline->op2))[0] == '\0') {
		return 0;
	}

	if (opline->opcode == CREX_ASSIGN_OBJ_OP) {
		if (opline->op1_type == IS_CV
		 && (opline+1)->op1_type == IS_CV
		 && (opline+1)->op1.var == opline->op1.var) {
			/* skip $a->prop += $a; */
			return 0;
		}
		if (!crex_jit_supported_binary_op(
				opline->extended_value, MAY_BE_ANY, OP1_DATA_INFO())) {
			return 0;
		}
	}

	for (i = ssa->vars[var].definition; i < use; i++) {
		if (ssa_opcodes[i]->opcode == CREX_DO_UCALL
		 || ssa_opcodes[i]->opcode == CREX_DO_FCALL_BY_NAME
		 || ssa_opcodes[i]->opcode == CREX_DO_FCALL
		 || ssa_opcodes[i]->opcode == CREX_INCLUDE_OR_EVAL) {
			return 0;
		}
	}

	return 1;
}

static int crex_jit_trace_stack_needs_deoptimization(crex_jit_trace_stack *stack, uint32_t stack_size)
{
	uint32_t i;

	for (i = 0; i < stack_size; i++) {
		if (STACK_REG(stack, i) != ZREG_NONE
		 && !(STACK_FLAGS(stack, i) & (ZREG_LOAD|ZREG_STORE))) {
			return 1;
		}
	}
	return 0;
}

static int crex_jit_trace_exit_needs_deoptimization(uint32_t trace_num, uint32_t exit_num)
{
	const crex_op *opline = crex_jit_traces[trace_num].exit_info[exit_num].opline;
	uint32_t flags = crex_jit_traces[trace_num].exit_info[exit_num].flags;
	uint32_t stack_size;
	crex_jit_trace_stack *stack;

	if (opline || (flags & (CREX_JIT_EXIT_RESTORE_CALL|CREX_JIT_EXIT_FREE_OP1|CREX_JIT_EXIT_FREE_OP2))) {
		return 1;
	}

	stack_size = crex_jit_traces[trace_num].exit_info[exit_num].stack_size;
	stack = crex_jit_traces[trace_num].stack_map + crex_jit_traces[trace_num].exit_info[exit_num].stack_offset;
	return crex_jit_trace_stack_needs_deoptimization(stack, stack_size);
}

static int crex_jit_trace_deoptimization(dasm_State             **Dst,
                                         uint32_t                 flags,
                                         const crex_op           *opline,
                                         crex_jit_trace_stack    *parent_stack,
                                         int                      parent_vars_count,
                                         crex_ssa                *ssa,
                                         crex_jit_trace_stack    *stack,
                                         crex_lifetime_interval **ra,
                                         bool                polymorphic_side_trace)
{
	int i;
	bool has_constants = 0;
	bool has_unsaved_vars = 0;

	// TODO: Merge this loop with the following register LOAD loop to implement parallel move ???
	for (i = 0; i < parent_vars_count; i++) {
		int8_t reg = STACK_REG(parent_stack, i);

		if (reg != ZREG_NONE) {
			if (reg < ZREG_NUM) {
				if (ssa && ssa->vars[i].no_val) {
					/* pass */
				} else if (ra && ra[i] && ra[i]->reg == reg) {
					/* register already loaded by parent trace */
					if (stack) {
						SET_STACK_REG_EX(stack, i, reg, STACK_FLAGS(parent_stack, i));
					}
					has_unsaved_vars = 1;
				} else {
					uint8_t type = STACK_TYPE(parent_stack, i);

					if (!(STACK_FLAGS(parent_stack, i) & (ZREG_LOAD|ZREG_STORE))
					 && !crex_jit_store_var(Dst, 1 << type, i, reg,
							STACK_MEM_TYPE(parent_stack, i) != type)) {
						return 0;
					}
					if (stack) {
						SET_STACK_TYPE(stack, i, type, 1);
					}
				}
			} else {
				/* delay custom deoptimization instructions to prevent register clobbering */
				has_constants = 1;
			}
		}
	}

	if (has_unsaved_vars
	 && (has_constants
	  || (flags & (CREX_JIT_EXIT_RESTORE_CALL|CREX_JIT_EXIT_FREE_OP1|CREX_JIT_EXIT_FREE_OP2)))) {
		for (i = 0; i < parent_vars_count; i++) {
			int8_t reg = STACK_REG(parent_stack, i);

			if (reg != ZREG_NONE) {
				if (reg < ZREG_NUM) {
					if (ssa && ssa->vars[i].no_val) {
						/* pass */
					} else if (ra && ra[i] && ra[i]->reg == reg) {
						uint8_t type = STACK_TYPE(parent_stack, i);

					    if (stack) {
							SET_STACK_TYPE(stack, i, type, 1);
						}
						if (!(STACK_FLAGS(parent_stack, i) & (ZREG_LOAD|ZREG_STORE))
						 && !crex_jit_store_var(Dst, 1 << type, i, reg,
								STACK_MEM_TYPE(parent_stack, i) != type)) {
							return 0;
						}
					}
				}
			}
		}
	}

	if (has_constants) {
		for (i = 0; i < parent_vars_count; i++) {
			int8_t reg = STACK_REG(parent_stack, i);

			if (reg != ZREG_NONE) {
				if (reg < ZREG_NUM) {
					/* pass */
				} else if (reg == ZREG_THIS) {
					if (polymorphic_side_trace) {
						ssa->var_info[i].delayed_fetch_this = 1;
						if (stack) {
							SET_STACK_REG(stack, i, ZREG_THIS);
						}
					} else if (!crex_jit_load_this(Dst, EX_NUM_TO_VAR(i))) {
						return 0;
					}
				} else {
					if (reg == ZREG_ZVAL_COPY_GPR0
					 &&!crex_jit_escape_if_undef_r0(Dst, i, flags, opline)) {
						return 0;
					}
					if (!crex_jit_store_const(Dst, i, reg)) {
						return 0;
					}
				}
			}
		}
	}

	if (flags & CREX_JIT_EXIT_RESTORE_CALL) {
		if (!crex_jit_save_call_chain(Dst, -1)) {
			return 0;
		}
	}

	if (flags & CREX_JIT_EXIT_FREE_OP2) {
		const crex_op *op = opline - 1;

		if (!crex_jit_free_op(Dst, op, -1, op->op2.var)) {
			return 0;
		}
	}

	if (flags & CREX_JIT_EXIT_FREE_OP1) {
		const crex_op *op = opline - 1;

		if (!crex_jit_free_op(Dst, op, -1, op->op1.var)) {
			return 0;
		}
	}

	if (flags & (CREX_JIT_EXIT_FREE_OP1|CREX_JIT_EXIT_FREE_OP2)) {
		if (!crex_jit_check_exception(Dst)) {
			return 0;
		}
	}

	if ((flags & CREX_JIT_EXIT_METHOD_CALL) && !polymorphic_side_trace) {
		if (!crex_jit_free_trampoline(Dst)) {
			return 0;
		}
	}

	return 1;
}

static void crex_jit_trace_set_var_range(crex_ssa_var_info *info, crex_long min, crex_long max)
{
	info->has_range = 1;
	info->range.min = min;
	info->range.max = max;
	info->range.underflow = 0;
	info->range.overflow = 0;
}

static void crex_jit_trace_update_condition_ranges(const crex_op *opline, const crex_ssa_op *ssa_op, const crex_op_array *op_array, crex_ssa *ssa, bool exit_if_true)
{
	crex_long op1_min, op1_max, op2_min, op2_max;

	if ((OP1_INFO() & MAY_BE_ANY) != MAY_BE_LONG
	 || (OP1_INFO() & MAY_BE_ANY) != MAY_BE_LONG) {
		return;
	}

	op1_min = OP1_MIN_RANGE();
	op1_max = OP1_MAX_RANGE();
	op2_min = OP2_MIN_RANGE();
	op2_max = OP2_MAX_RANGE();

	switch (opline->opcode) {
		case CREX_IS_EQUAL:
		case CREX_CASE:
		case CREX_IS_IDENTICAL:
		case CREX_CASE_STRICT:
		case CREX_IS_NOT_IDENTICAL:
			if (!exit_if_true) {
				/* op1 == op2 */
				if (ssa_op->op1_use >= 0) {
					crex_jit_trace_set_var_range(
						&ssa->var_info[ssa_op->op1_use],
						MAX(op1_min, op2_min),
						MIN(op1_max, op2_max));
				}
				if (ssa_op->op2_use >= 0) {
					crex_jit_trace_set_var_range(
						&ssa->var_info[ssa_op->op2_use],
						MAX(op2_min, op1_min),
						MIN(op2_max, op1_max));
				}
			}
			break;
		case CREX_IS_NOT_EQUAL:
			if (exit_if_true) {
				/* op1 == op2 */
				if (ssa_op->op1_use >= 0) {
					crex_jit_trace_set_var_range(
						&ssa->var_info[ssa_op->op1_use],
						MAX(op1_min, op2_min),
						MIN(op1_max, op2_max));
				}
				if (ssa_op->op2_use >= 0) {
					crex_jit_trace_set_var_range(
						&ssa->var_info[ssa_op->op2_use],
						MAX(op2_min, op1_min),
						MIN(op2_max, op1_max));
				}
			}
			break;
		case CREX_IS_SMALLER_OR_EQUAL:
			if (!exit_if_true) {
				/* op1 <= op2 */
				if (ssa_op->op1_use >= 0) {
					crex_jit_trace_set_var_range(
						&ssa->var_info[ssa_op->op1_use],
						op1_min,
						MIN(op1_max, op2_max));
				}
				if (ssa_op->op2_use >= 0) {
					crex_jit_trace_set_var_range(
						&ssa->var_info[ssa_op->op2_use],
						MAX(op2_min, op1_min),
						op2_max);
				}
			} else {
				/* op1 > op2 */
				if (ssa_op->op1_use >= 0) {
					crex_jit_trace_set_var_range(
						&ssa->var_info[ssa_op->op1_use],
						op2_min != CREX_LONG_MAX ? MAX(op1_min, op2_min + 1) : op1_min,
						op1_max);
				}
				if (ssa_op->op2_use >= 0) {
					crex_jit_trace_set_var_range(
						&ssa->var_info[ssa_op->op2_use],
						op2_min,
						op2_max != CREX_LONG_MIN ?MIN(op2_max, op1_max - 1) : op1_max);
				}
			}
			break;
		case CREX_IS_SMALLER:
			if (!exit_if_true) {
				/* op1 < op2 */
				if (ssa_op->op1_use >= 0) {
					crex_jit_trace_set_var_range(
						&ssa->var_info[ssa_op->op1_use],
						op1_min,
						op2_max != CREX_LONG_MIN ? MIN(op1_max, op2_max - 1) : op1_max);
				}
				if (ssa_op->op2_use >= 0) {
					crex_jit_trace_set_var_range(
						&ssa->var_info[ssa_op->op2_use],
						op1_min != CREX_LONG_MAX ? MAX(op2_min, op1_min + 1) : op2_min,
						op2_max);
				}
			} else {
				/* op1 >= op2 */
				if (ssa_op->op1_use >= 0) {
					crex_jit_trace_set_var_range(
						&ssa->var_info[ssa_op->op1_use],
						MAX(op1_min, op2_min),
						op1_max);
				}
				if (ssa_op->op2_use >= 0) {
					crex_jit_trace_set_var_range(
						&ssa->var_info[ssa_op->op2_use],
						op2_min,
						MIN(op2_max, op1_max));
				}
			}
			break;
	}
}

static bool crex_jit_may_skip_comparison(const crex_op *opline, const crex_ssa_op *ssa_op, const crex_ssa *ssa, const crex_op **ssa_opcodes, const crex_op_array *op_array)
{
	uint8_t prev_opcode;

	if (opline->op1_type == IS_CONST
	 && C_TYPE_P(RT_CONSTANT(opline, opline->op1)) == IS_LONG
	 && C_LVAL_P(RT_CONSTANT(opline, opline->op1)) == 0) {
		if (ssa_op->op2_use >= 0) {
			if ((ssa_op-1)->op1_def == ssa_op->op2_use) {
				ssa_op--;
				opline = ssa_opcodes[ssa_op - ssa->ops];
				prev_opcode = opline->opcode;
				if (prev_opcode == CREX_PRE_INC
				 || prev_opcode == CREX_PRE_DEC
				 || prev_opcode == CREX_POST_INC
				 || prev_opcode == CREX_POST_DEC) {
					return (OP1_INFO() & ((MAY_BE_UNDEF|MAY_BE_ANY|MAY_BE_REF)-MAY_BE_LONG)) == 0;
				}
			} else if ((ssa_op-1)->result_def == ssa_op->op2_use) {
				ssa_op--;
				opline = ssa_opcodes[ssa_op - ssa->ops];
				prev_opcode = opline->opcode;
				if (prev_opcode == CREX_ADD
				 || prev_opcode == CREX_SUB) {
					return (OP1_INFO() & ((MAY_BE_UNDEF|MAY_BE_ANY|MAY_BE_REF)-MAY_BE_LONG)) == 0 &&
						(OP2_INFO() & ((MAY_BE_UNDEF|MAY_BE_ANY|MAY_BE_REF)-MAY_BE_LONG)) == 0;
				}
			}
		}
	} else if (opline->op2_type == IS_CONST
	 && C_TYPE_P(RT_CONSTANT(opline, opline->op2)) == IS_LONG
	 && C_LVAL_P(RT_CONSTANT(opline, opline->op2)) == 0) {
		if (ssa_op->op1_use >= 0) {
			if ((ssa_op-1)->op1_def == ssa_op->op1_use) {
				ssa_op--;
				opline = ssa_opcodes[ssa_op - ssa->ops];
				prev_opcode = opline->opcode;
				if (prev_opcode == CREX_PRE_INC
				 || prev_opcode == CREX_PRE_DEC
				 || prev_opcode == CREX_POST_INC
				 || prev_opcode == CREX_POST_DEC) {
					return (OP1_INFO() & ((MAY_BE_UNDEF|MAY_BE_ANY|MAY_BE_REF)-MAY_BE_LONG)) == 0;
				}
			} else if ((ssa_op-1)->result_def == ssa_op->op1_use) {
				ssa_op--;
				opline = ssa_opcodes[ssa_op - ssa->ops];
				prev_opcode = opline->opcode;
				if (prev_opcode == CREX_ADD
				 || prev_opcode == CREX_SUB) {
					return (OP1_INFO() & ((MAY_BE_UNDEF|MAY_BE_ANY|MAY_BE_REF)-MAY_BE_LONG)) == 0 &&
						(OP2_INFO() & ((MAY_BE_UNDEF|MAY_BE_ANY|MAY_BE_REF)-MAY_BE_LONG)) == 0;
				}
			}
		}
	} else {
		const crex_ssa_op *prev_ssa_op = ssa_op - 1;
		prev_opcode = ssa_opcodes[prev_ssa_op - ssa->ops]->opcode;

		if ((prev_opcode == CREX_JMPZ || prev_opcode == CREX_JMPNZ)
		 && prev_ssa_op != ssa->ops
		 && prev_ssa_op->op1_use >= 0
		 && prev_ssa_op->op1_use == (prev_ssa_op-1)->result_def) {
			prev_ssa_op--;
			prev_opcode = ssa_opcodes[prev_ssa_op - ssa->ops]->opcode;
		}

		if (ssa_op->op1_use == prev_ssa_op->op1_use
		 && ssa_op->op2_use == prev_ssa_op->op2_use) {
			if (prev_opcode == CREX_IS_EQUAL
			 || prev_opcode == CREX_IS_NOT_EQUAL
			 || prev_opcode == CREX_IS_SMALLER
			 || prev_opcode == CREX_IS_SMALLER_OR_EQUAL
			 || prev_opcode == CREX_CASE
			 || prev_opcode == CREX_IS_IDENTICAL
			 || prev_opcode == CREX_IS_NOT_IDENTICAL
			 || prev_opcode == CREX_CASE_STRICT) {
				if (ssa_op->op1_use < 0) {
					if (RT_CONSTANT(opline, opline->op1) != RT_CONSTANT(&ssa_opcodes[prev_ssa_op - ssa->ops], ssa_opcodes[prev_ssa_op - ssa->ops]->op1)) {
						return 0;
					}
				}
				if (ssa_op->op2_use < 0) {
					if (RT_CONSTANT(opline, opline->op2) != RT_CONSTANT(&ssa_opcodes[prev_ssa_op - ssa->ops], ssa_opcodes[prev_ssa_op - ssa->ops]->op2)) {
						return 0;
					}
				}
				return 1;
			}
		}
	}
	return 0;
}

static bool crex_jit_trace_next_is_send_result(const crex_op              *opline,
                                               crex_jit_trace_rec         *p,
                                               crex_jit_trace_stack_frame *frame)
{
	if (opline->result_type == IS_TMP_VAR
	 && (p+1)->op == CREX_JIT_TRACE_VM
	 && (p+1)->opline == opline + 1
	 && ((opline+1)->opcode == CREX_SEND_VAL
	  || ((opline+1)->opcode == CREX_SEND_VAL_EX
	   && frame
	   && frame->call
	   && frame->call->func
	   && !ARG_MUST_BE_SENT_BY_REF(frame->call->func, (opline+1)->op2.num)))
	 && (opline+1)->op1_type == IS_TMP_VAR
	 && (opline+1)->op2_type != IS_CONST /* Named parameters not supported in JIT */
	 && (opline+1)->op1.var == opline->result.var) {

		if (frame->call && frame->call->func) {
			uint8_t res_type = (p+1)->op1_type;

			if (res_type != IS_UNKNOWN && !(res_type & IS_TRACE_REFERENCE) ) {
				crex_jit_trace_send_type(opline+1, frame->call, res_type);
			}
		}
		return 1;
	}
	return 0;
}

static const void *crex_jit_trace(crex_jit_trace_rec *trace_buffer, uint32_t parent_trace, uint32_t exit_num)
{
	const void *handler = NULL;
	dasm_State* dasm_state = NULL;
	crex_script *script = NULL;
	crex_lifetime_interval **ra = NULL;
	crex_string *name = NULL;
	void *checkpoint;
	const crex_op_array *op_array;
	crex_ssa *ssa, *op_array_ssa;
	const crex_op **ssa_opcodes;
	crex_jit_trace_rec *p;
	crex_jit_op_array_trace_extension *jit_extension;
	int num_op_arrays = 0;
	crex_jit_trace_info *t;
	const crex_op_array *op_arrays[CREX_JIT_TRACE_MAX_FUNCS];
	uint8_t smart_branch_opcode;
	const void *exit_addr;
	uint32_t op1_info, op1_def_info, op2_info, res_info, res_use_info, op1_data_info;
	bool send_result = 0;
	bool skip_comparison;
	crex_jit_addr op1_addr, op1_def_addr, op2_addr, op2_def_addr, res_addr;
	crex_class_entry *ce;
	bool ce_is_instanceof;
	bool on_this = 0;
	bool delayed_fetch_this = 0;
	bool avoid_refcounting = 0;
	bool polymorphic_side_trace =
		parent_trace &&
		(crex_jit_traces[parent_trace].exit_info[exit_num].flags & CREX_JIT_EXIT_METHOD_CALL);
	uint32_t i;
	crex_jit_trace_stack_frame *frame, *top, *call;
	crex_jit_trace_stack *stack;
	uint8_t res_type = IS_UNKNOWN;
	const crex_op *opline, *orig_opline;
	const crex_ssa_op *ssa_op, *orig_ssa_op;
	int checked_stack;
	int peek_checked_stack;
	uint32_t frame_flags = 0;

	JIT_G(current_trace) = trace_buffer;

	checkpoint = crex_arena_checkpoint(CG(arena));

	ssa = crex_jit_trace_build_tssa(trace_buffer, parent_trace, exit_num, script, op_arrays, &num_op_arrays);

	if (!ssa) {
		goto jit_cleanup;
	}

	ssa_opcodes = ((crex_tssa*)ssa)->tssa_opcodes;

	/* Register allocation */
	if ((JIT_G(opt_flags) & (CREX_JIT_REG_ALLOC_LOCAL|CREX_JIT_REG_ALLOC_GLOBAL))
	 && JIT_G(opt_level) >= CREX_JIT_LEVEL_INLINE) {
		ra = crex_jit_trace_allocate_registers(trace_buffer, ssa, parent_trace, exit_num);
	}

	p = trace_buffer;
	CREX_ASSERT(p->op == CREX_JIT_TRACE_START);
	op_array = p->op_array;
	frame = JIT_G(current_frame);
	top = crex_jit_trace_call_frame(frame, op_array);
	TRACE_FRAME_INIT(frame, op_array, TRACE_FRAME_MASK_UNKNOWN_RETURN, -1);
	frame->used_stack = checked_stack = peek_checked_stack = 0;
	stack = frame->stack;
	for (i = 0; i < op_array->last_var + op_array->T; i++) {
		SET_STACK_TYPE(stack, i, IS_UNKNOWN, 1);
	}

	opline = p[1].opline;
	name = crex_jit_trace_name(op_array, opline->lineno);
	p += CREX_JIT_TRACE_START_REC_SIZE;

	dasm_init(&dasm_state, DASM_MAXSECTION);
	dasm_setupglobal(&dasm_state, dasm_labels, crex_lb_MAX);
	dasm_setup(&dasm_state, dasm_actions);

	jit_extension =
		(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
	op_array_ssa = &jit_extension->func_info.ssa;

	dasm_growpc(&dasm_state, 2); /* =>0: loop header */
	                             /* =>1: end of code */

	crex_jit_align_func(&dasm_state);
	if (!parent_trace) {
		crex_jit_prologue(&dasm_state);
	}
	crex_jit_trace_begin(&dasm_state, CREX_JIT_TRACE_NUM,
		parent_trace ? &crex_jit_traces[parent_trace] : NULL, exit_num);

	if (!parent_trace) {
		crex_jit_set_last_valid_opline(opline);
		crex_jit_track_last_valid_opline();
	} else {
		if (crex_jit_traces[parent_trace].exit_info[exit_num].opline == NULL) {
			crex_jit_trace_opline_guard(&dasm_state, opline);
		} else {
			crex_jit_reset_last_valid_opline();
		}
	}

	if (JIT_G(opt_level) >= CREX_JIT_LEVEL_INLINE) {
		int last_var;
		int parent_vars_count = 0;
		crex_jit_trace_stack *parent_stack = NULL;
		int used_stack = ((crex_tssa*)ssa)->used_stack;

		if (used_stack > 0) {
			peek_checked_stack = used_stack;
			if (!crex_jit_stack_check(&dasm_state, opline, used_stack)) {
				goto jit_failure;
			}
		}

		if (parent_trace) {
			parent_vars_count = MIN(crex_jit_traces[parent_trace].exit_info[exit_num].stack_size,
				op_array->last_var + op_array->T);
			if (parent_vars_count) {
				parent_stack =
					crex_jit_traces[parent_trace].stack_map +
					crex_jit_traces[parent_trace].exit_info[exit_num].stack_offset;
			}
		}

		last_var = op_array->last_var;
		if (trace_buffer->start != CREX_JIT_TRACE_START_ENTER) {
			last_var += op_array->T;
		}

		for (i = 0; i < last_var; i++) {
			uint32_t info = ssa->var_info[i].type;

			if (!(info & MAY_BE_GUARD) && has_concrete_type(info)) {
				uint8_t type, mem_type;

				type = concrete_type(info);
				if (i < parent_vars_count
				 && STACK_TYPE(parent_stack, i) == type) {
					mem_type = STACK_MEM_TYPE(parent_stack, i);
					if (mem_type != IS_UNKNOWN) {
						SET_STACK_TYPE(stack, i, mem_type, 1);
					}
					SET_STACK_TYPE(stack, i, type, 0);
				} else {
					SET_STACK_TYPE(stack, i, type, 1);
				}
			} else if (ssa->vars[i].alias != NO_ALIAS) {
				SET_STACK_TYPE(stack, i, IS_UNKNOWN, 1);
			} else if (i < parent_vars_count
			 && STACK_TYPE(parent_stack, i) != IS_UNKNOWN) {
				/* This must be already handled by trace type inference */
				CREX_UNREACHABLE();
				// SET_STACK_TYPE(stack, i, STACK_TYPE(parent_stack, i));
			} else if ((info & MAY_BE_GUARD) != 0
			 && (trace_buffer->stop == CREX_JIT_TRACE_STOP_LOOP
			  || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_CALL
			  || (trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_RET
			   && (opline-1)->result_type == IS_VAR
			   && EX_VAR_TO_NUM((opline-1)->result.var) == i))
			 && (ssa->vars[i].use_chain != -1
			  || (ssa->vars[i].phi_use_chain
			   && !(ssa->var_info[ssa->vars[i].phi_use_chain->ssa_var].type & MAY_BE_GUARD)))) {
				/* Check loop-invariant variable type */
				if (!crex_jit_type_guard(&dasm_state, opline, EX_NUM_TO_VAR(i), concrete_type(info))) {
					goto jit_failure;
				}
				info &= ~MAY_BE_GUARD;
				ssa->var_info[i].type = info;
				SET_STACK_TYPE(stack, i, concrete_type(info), 1);
			} else if (trace_buffer->start == CREX_JIT_TRACE_START_ENTER
			 && op_array->function_name
			 && i >= op_array->num_args) {
				/* This must be already handled by trace type inference */
				CREX_UNREACHABLE();
				// SET_STACK_TYPE(stack, i, IS_UNDEF, 1);
			}

			if ((info & MAY_BE_PACKED_GUARD) != 0
			 && (trace_buffer->stop == CREX_JIT_TRACE_STOP_LOOP
			  || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_CALL
			  || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_RET)
			 && (ssa->vars[i].use_chain != -1
			  || (ssa->vars[i].phi_use_chain
			   && !(ssa->var_info[ssa->vars[i].phi_use_chain->ssa_var].type & MAY_BE_PACKED_GUARD)))) {
				if (!crex_jit_packed_guard(&dasm_state, opline, EX_NUM_TO_VAR(i), info)) {
					goto jit_failure;
				}
				info &= ~MAY_BE_PACKED_GUARD;
				ssa->var_info[i].type = info;
			}
		}

		if (parent_trace) {
			/* Deoptimization */
			if (!crex_jit_trace_deoptimization(&dasm_state,
					crex_jit_traces[parent_trace].exit_info[exit_num].flags,
					crex_jit_traces[parent_trace].exit_info[exit_num].opline,
					parent_stack, parent_vars_count, ssa, stack, ra,
					polymorphic_side_trace)) {
				goto jit_failure;
			}
		}

		if (ra
		 && trace_buffer->stop != CREX_JIT_TRACE_STOP_RECURSIVE_CALL
		 && trace_buffer->stop != CREX_JIT_TRACE_STOP_RECURSIVE_RET) {
			for (i = 0; i < last_var; i++) {
				if (ra[i]
				 && (ra[i]->flags & ZREG_LOAD) != 0
				 && ra[i]->reg != stack[i].reg) {

					if ((ssa->var_info[i].type & MAY_BE_GUARD) != 0) {
						uint8_t op_type;

						ssa->var_info[i].type &= ~MAY_BE_GUARD;
						op_type = concrete_type(ssa->var_info[i].type);
						if (!crex_jit_type_guard(&dasm_state, opline, EX_NUM_TO_VAR(i), op_type)) {
							goto jit_failure;
						}
						SET_STACK_TYPE(stack, i, op_type, 1);
					}

					SET_STACK_REG_EX(stack, i, ra[i]->reg, ZREG_LOAD);
					if (!crex_jit_load_var(&dasm_state, ssa->var_info[i].type, i, ra[i]->reg)) {
						goto jit_failure;
					}
				}
			}
		}
	}

	if (trace_buffer->stop == CREX_JIT_TRACE_STOP_LOOP
	 || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_CALL
	 || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_RET) {

		crex_jit_label(&dasm_state, 0); /* start of of trace loop */

		if (ra) {
			crex_ssa_phi *phi = ssa->blocks[1].phis;

			while (phi) {
				crex_lifetime_interval *ival = ra[phi->ssa_var];

				if (ival) {
					if (ival->flags & ZREG_LOAD) {
						uint32_t info = ssa->var_info[phi->ssa_var].type;
						CREX_ASSERT(ival->reg != ZREG_NONE);

						if (info & MAY_BE_GUARD) {
							if (!crex_jit_type_guard(&dasm_state, opline, EX_NUM_TO_VAR(phi->var), concrete_type(info))) {
								goto jit_failure;
							}
							info &= ~MAY_BE_GUARD;
							ssa->var_info[phi->ssa_var].type = info;
							SET_STACK_TYPE(stack, phi->var, concrete_type(info), 1);
						}
						SET_STACK_REG_EX(stack, phi->var, ival->reg, ZREG_LOAD);
						if (!crex_jit_load_var(&dasm_state, ssa->var_info[phi->ssa_var].type, ssa->vars[phi->ssa_var].var, ival->reg)) {
							goto jit_failure;
						}
					} else if (ival->flags & ZREG_STORE) {
						CREX_ASSERT(ival->reg != ZREG_NONE);

						SET_STACK_REG_EX(stack, phi->var, ival->reg, ZREG_STORE);
						if (!crex_jit_store_var(&dasm_state, ssa->var_info[phi->ssa_var].type, ssa->vars[phi->ssa_var].var, ival->reg,
								STACK_MEM_TYPE(stack, phi->var) != ssa->var_info[phi->ssa_var].type)) {
							goto jit_failure;
						}
					} else {
						/* Register has to be written back on side exit */
						SET_STACK_REG(stack, phi->var, ival->reg);
					}
				}
				phi = phi->next;
			}
		}

//		if (trace_buffer->stop != CREX_JIT_TRACE_STOP_RECURSIVE_RET) {
//			if (ra && crex_jit_trace_stack_needs_deoptimization(stack, op_array->last_var + op_array->T)) {
//				uint32_t exit_point = crex_jit_trace_get_exit_point(opline, CREX_JIT_EXIT_TO_VM);
//
//				timeout_exit_addr = crex_jit_trace_get_exit_addr(exit_point);
//				if (!timeout_exit_addr) {
//					goto jit_failure;
//				}
//			}
//		}

		if (ra && trace_buffer->stop != CREX_JIT_TRACE_STOP_LOOP) {
			int last_var = op_array->last_var;

			if (trace_buffer->start != CREX_JIT_TRACE_START_ENTER) {
				last_var += op_array->T;
			}
			for (i = 0; i < last_var; i++) {
				if (ra && ra[i] && (ra[i]->flags & ZREG_LOAD) != 0) {
					SET_STACK_REG_EX(stack, i, ra[i]->reg, ZREG_LOAD);
					if (!crex_jit_load_var(&dasm_state, ssa->var_info[i].type, i, ra[i]->reg)) {
						goto jit_failure;
					}
				}
			}
		}
	}

	ssa_op = (JIT_G(opt_level) >= CREX_JIT_LEVEL_INLINE) ? ssa->ops : NULL;
	for (;;p++) {
		if (p->op == CREX_JIT_TRACE_VM) {
			uint8_t op1_type = p->op1_type;
			uint8_t op2_type = p->op2_type;
			uint8_t op3_type = p->op3_type;
			uint8_t orig_op1_type = op1_type;
			uint8_t orig_op2_type = op2_type;
			uint8_t val_type = IS_UNKNOWN;
			bool op1_indirect;
			crex_class_entry *op1_ce = NULL;
			crex_class_entry *op2_ce = NULL;
			bool gen_handler = false;

			opline = p->opline;
			if (op1_type & (IS_TRACE_REFERENCE|IS_TRACE_INDIRECT)) {
				op1_type = IS_UNKNOWN;
			}
			if (op1_type != IS_UNKNOWN) {
				op1_type &= ~IS_TRACE_PACKED;
			}
			if (op2_type & (IS_TRACE_REFERENCE|IS_TRACE_INDIRECT)) {
				op2_type = IS_UNKNOWN;
			}
			if (op3_type & (IS_TRACE_REFERENCE|IS_TRACE_INDIRECT)) {
				op3_type = IS_UNKNOWN;
			}

			if ((p+1)->op == CREX_JIT_TRACE_OP1_TYPE) {
				op1_ce = (crex_class_entry*)(p+1)->ce;
				p++;
			}
			if ((p+1)->op == CREX_JIT_TRACE_OP2_TYPE) {
				op2_ce = (crex_class_entry*)(p+1)->ce;
				p++;
			}
			if ((p+1)->op == CREX_JIT_TRACE_VAL_INFO) {
				val_type = (p+1)->op1_type;
				p++;
			}

			frame_flags = 0;

			switch (opline->opcode) {
				case CREX_INIT_FCALL:
				case CREX_INIT_FCALL_BY_NAME:
				case CREX_INIT_NS_FCALL_BY_NAME:
				case CREX_INIT_METHOD_CALL:
				case CREX_INIT_DYNAMIC_CALL:
				case CREX_INIT_STATIC_METHOD_CALL:
				case CREX_INIT_USER_CALL:
				case CREX_NEW:
					frame->call_level++;
			}

			if (JIT_G(opt_level) >= CREX_JIT_LEVEL_INLINE) {
				gen_handler = 0;
				switch (opline->opcode) {
					case CREX_PRE_INC:
					case CREX_PRE_DEC:
					case CREX_POST_INC:
					case CREX_POST_DEC:
						if (opline->op1_type != IS_CV) {
							break;
						}
						op1_info = OP1_INFO();
						CHECK_OP1_TRACE_TYPE();
						if (!(op1_info & MAY_BE_LONG)) {
							break;
						}
						if (opline->result_type != IS_UNUSED) {
							res_use_info = crex_jit_trace_type_to_info(
								STACK_MEM_TYPE(stack, EX_VAR_TO_NUM(opline->result.var)))
									& (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_LONG|MAY_BE_DOUBLE);
							res_info = RES_INFO();
							res_addr = RES_REG_ADDR();
						} else {
							res_use_info = -1;
							res_info = -1;
							res_addr = 0;
						}
						op1_def_info = OP1_DEF_INFO();
						if (op1_def_info & MAY_BE_GUARD
						 && !has_concrete_type(op1_def_info)) {
							op1_def_info &= ~MAY_BE_GUARD;
						}
						if (!crex_jit_inc_dec(&dasm_state, opline,
								op1_info, OP1_REG_ADDR(),
								op1_def_info, OP1_DEF_REG_ADDR(),
								res_use_info, res_info,
								res_addr,
								(op1_def_info & (MAY_BE_DOUBLE|MAY_BE_GUARD)) && crex_may_overflow(opline, ssa_op, op_array, ssa),
								crex_may_throw(opline, ssa_op, op_array, ssa))) {
							goto jit_failure;
						}
						if ((op1_def_info & (MAY_BE_ANY|MAY_BE_GUARD)) == (MAY_BE_LONG|MAY_BE_GUARD)
						 && !(op1_info & MAY_BE_STRING)) {
							ssa->var_info[ssa_op->op1_def].type &= ~MAY_BE_GUARD;
							if (opline->result_type != IS_UNUSED) {
								ssa->var_info[ssa_op->result_def].type &= ~MAY_BE_GUARD;
							}
						}
						if (opline->result_type != IS_UNUSED
						 && (res_info & (MAY_BE_ANY|MAY_BE_GUARD)) == (MAY_BE_LONG|MAY_BE_GUARD)
						 && !(op1_info & MAY_BE_STRING)) {
							ssa->var_info[ssa_op->result_def].type &= ~MAY_BE_GUARD;
						}
						goto done;
					case CREX_BW_OR:
					case CREX_BW_AND:
					case CREX_BW_XOR:
					case CREX_SL:
					case CREX_SR:
					case CREX_MOD:
						op1_info = OP1_INFO();
						CHECK_OP1_TRACE_TYPE();
						op2_info = OP2_INFO();
						CHECK_OP2_TRACE_TYPE();
						if ((op1_info & MAY_BE_UNDEF) || (op2_info & MAY_BE_UNDEF)) {
							break;
						}
						if (!(op1_info & MAY_BE_LONG)
						 || !(op2_info & MAY_BE_LONG)) {
							break;
						}
						res_addr = RES_REG_ADDR();
						if (C_MODE(res_addr) != IS_REG
						 && crex_jit_trace_next_is_send_result(opline, p, frame)) {
							send_result = 1;
							res_use_info = -1;
							res_addr = CREX_ADDR_MEM_ZVAL(ZREG_RX, (opline+1)->result.var);
							if (!crex_jit_reuse_ip(&dasm_state)) {
								goto jit_failure;
							}
						} else {
							res_use_info = crex_jit_trace_type_to_info(
								STACK_MEM_TYPE(stack, EX_VAR_TO_NUM(opline->result.var)))
									& (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_LONG|MAY_BE_DOUBLE);
						}
						res_info = RES_INFO();
						if (!crex_jit_long_math(&dasm_state, opline,
								op1_info, OP1_RANGE(), OP1_REG_ADDR(),
								op2_info, OP2_RANGE(), OP2_REG_ADDR(),
								res_use_info, res_info, res_addr,
								crex_may_throw(opline, ssa_op, op_array, ssa))) {
							goto jit_failure;
						}
						goto done;
					case CREX_ADD:
					case CREX_SUB:
					case CREX_MUL:
//					case CREX_DIV: // TODO: check for division by zero ???
						op1_info = OP1_INFO();
						op1_addr = OP1_REG_ADDR();
						op2_info = OP2_INFO();
						op2_addr = OP2_REG_ADDR();
						if (orig_op1_type != IS_UNKNOWN
						 && (orig_op1_type & IS_TRACE_REFERENCE)
						 && opline->op1_type == IS_CV
						 && (C_MODE(op2_addr) != IS_REG || C_REG(op2_addr) != ZREG_FCARG1)
						 && (orig_op2_type == IS_UNKNOWN || !(orig_op2_type & IS_TRACE_REFERENCE))) {
							if (!crex_jit_fetch_reference(&dasm_state, opline, orig_op1_type, &op1_info, &op1_addr,
									!ssa->var_info[ssa_op->op1_use].guarded_reference, 1)) {
								goto jit_failure;
							}
							if (ssa->vars[ssa_op->op1_use].alias == NO_ALIAS) {
								ssa->var_info[ssa_op->op1_use].guarded_reference = 1;
							}
						} else {
							CHECK_OP1_TRACE_TYPE();
						}
						if (orig_op2_type != IS_UNKNOWN
						 && (orig_op2_type & IS_TRACE_REFERENCE)
						 && opline->op2_type == IS_CV
						 && (C_MODE(op1_addr) != IS_REG || C_REG(op1_addr) != ZREG_FCARG1)
						 && (orig_op1_type == IS_UNKNOWN || !(orig_op1_type & IS_TRACE_REFERENCE))) {
							if (!crex_jit_fetch_reference(&dasm_state, opline, orig_op2_type, &op2_info, &op2_addr,
									!ssa->var_info[ssa_op->op2_use].guarded_reference, 1)) {
								goto jit_failure;
							}
							if (ssa->vars[ssa_op->op2_use].alias == NO_ALIAS) {
								ssa->var_info[ssa_op->op2_use].guarded_reference = 1;
							}
						} else {
							CHECK_OP2_TRACE_TYPE();
						}
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
						 && crex_jit_trace_next_is_send_result(opline, p, frame)) {
							send_result = 1;
							res_use_info = -1;
							res_addr = CREX_ADDR_MEM_ZVAL(ZREG_RX, (opline+1)->result.var);
							if (!crex_jit_reuse_ip(&dasm_state)) {
								goto jit_failure;
							}
						} else {
							res_use_info = crex_jit_trace_type_to_info(
								STACK_MEM_TYPE(stack, EX_VAR_TO_NUM(opline->result.var)))
									& (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_LONG|MAY_BE_DOUBLE);
						}
						res_info = RES_INFO();
						if (opline->opcode == CREX_ADD &&
						    (op1_info & (MAY_BE_ANY|MAY_BE_UNDEF)) == MAY_BE_ARRAY &&
						    (op2_info & (MAY_BE_ANY|MAY_BE_UNDEF)) == MAY_BE_ARRAY) {
							if (!crex_jit_add_arrays(&dasm_state, opline, op1_info, op1_addr, op2_info, op2_addr, res_addr)) {
								goto jit_failure;
							}
						} else {
							if (!crex_jit_math(&dasm_state, opline,
									op1_info, op1_addr,
									op2_info, op2_addr,
									res_use_info, res_info, res_addr,
									(op1_info & MAY_BE_LONG) && (op2_info & MAY_BE_LONG) && (res_info & (MAY_BE_DOUBLE|MAY_BE_GUARD)) && crex_may_overflow(opline, ssa_op, op_array, ssa),
									crex_may_throw(opline, ssa_op, op_array, ssa))) {
								goto jit_failure;
							}
							if (((res_info & (MAY_BE_ANY|MAY_BE_GUARD)) == (MAY_BE_LONG|MAY_BE_GUARD)
							  || (res_info & (MAY_BE_ANY|MAY_BE_GUARD)) == (MAY_BE_DOUBLE|MAY_BE_GUARD))
							 && has_concrete_type(op1_info)
							 && has_concrete_type(op2_info)) {
								ssa->var_info[ssa_op->result_def].type &= ~MAY_BE_GUARD;
							}
						}
						goto done;
					case CREX_CONCAT:
					case CREX_FAST_CONCAT:
						op1_info = OP1_INFO();
						CHECK_OP1_TRACE_TYPE();
						op2_info = OP2_INFO();
						CHECK_OP2_TRACE_TYPE();
						if ((op1_info & MAY_BE_UNDEF) || (op2_info & MAY_BE_UNDEF)) {
							break;
						}
						if (!(op1_info & MAY_BE_STRING) ||
						    !(op2_info & MAY_BE_STRING)) {
							break;
						}
						res_addr = RES_REG_ADDR();
						if (crex_jit_trace_next_is_send_result(opline, p, frame)) {
							send_result = 1;
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
						op1_info = OP1_INFO();
						CHECK_OP1_TRACE_TYPE();
						op2_info = OP2_INFO();
						CHECK_OP2_TRACE_TYPE();
						if (!crex_jit_supported_binary_op(
								opline->extended_value, op1_info, op2_info)) {
							break;
						}
						op1_def_info = OP1_DEF_INFO();
						if (op1_def_info & MAY_BE_GUARD
						 && !has_concrete_type(op1_def_info)) {
							op1_def_info &= ~MAY_BE_GUARD;
						}
						if (!crex_jit_assign_op(&dasm_state, opline,
								op1_info, op1_def_info, OP1_RANGE(),
								op2_info, OP2_RANGE(),
								(op1_info & MAY_BE_LONG) && (op2_info & MAY_BE_LONG) && (op1_def_info & (MAY_BE_DOUBLE|MAY_BE_GUARD)) && crex_may_overflow(opline, ssa_op, op_array, ssa),
								crex_may_throw(opline, ssa_op, op_array, ssa))) {
							goto jit_failure;
						}
						if ((op1_def_info & (MAY_BE_ANY|MAY_BE_GUARD)) == (MAY_BE_LONG|MAY_BE_GUARD)
						 && has_concrete_type(op1_info)
						 && has_concrete_type(op2_info)) {
							ssa->var_info[ssa_op->op1_def].type &= ~MAY_BE_GUARD;
							if (opline->result_type != IS_UNUSED) {
								ssa->var_info[ssa_op->result_def].type &= ~MAY_BE_GUARD;
							}
						}
						goto done;
					case CREX_ASSIGN_DIM_OP:
						if (opline->result_type != IS_UNUSED) {
							break;
						}
						if (!crex_jit_supported_binary_op(
								opline->extended_value, MAY_BE_ANY, OP1_DATA_INFO())) {
							break;
						}
						if (opline->op1_type == IS_CV
						 && (opline+1)->op1_type == IS_CV
						 && (opline+1)->op1.var == opline->op1.var) {
							/* skip $a[x] += $a; */
							break;
						}
						op1_info = OP1_INFO();
						op1_addr = OP1_REG_ADDR();
						if (opline->op1_type == IS_VAR) {
							if (orig_op1_type != IS_UNKNOWN
							 && (orig_op1_type & IS_TRACE_INDIRECT)) {
								if (!crex_jit_fetch_indirect_var(&dasm_state, opline, orig_op1_type,
										&op1_info, &op1_addr, !ssa->var_info[ssa_op->op1_use].indirect_reference)) {
									goto jit_failure;
								}
							} else {
								break;
							}
						}
						if (orig_op1_type != IS_UNKNOWN
						 && (orig_op1_type & IS_TRACE_REFERENCE)) {
							if (!crex_jit_fetch_reference(&dasm_state, opline, orig_op1_type, &op1_info, &op1_addr,
									!ssa->var_info[ssa_op->op1_use].guarded_reference, 1)) {
								goto jit_failure;
							}
							if (opline->op1_type == IS_CV
							 && ssa->vars[ssa_op->op1_def].alias == NO_ALIAS) {
								ssa->var_info[ssa_op->op1_def].guarded_reference = 1;
							}
						} else {
							CHECK_OP1_TRACE_TYPE();
						}
						op2_info = OP2_INFO();
						CHECK_OP2_TRACE_TYPE();
						op1_data_info = OP1_DATA_INFO();
						CHECK_OP1_DATA_TRACE_TYPE();
						op1_def_info = OP1_DEF_INFO();
						if (!crex_jit_assign_dim_op(&dasm_state, opline,
								op1_info, op1_def_info, op1_addr, op2_info,
								op1_data_info, OP1_DATA_RANGE(), val_type,
								crex_may_throw_ex(opline, ssa_op, op_array, ssa, op1_info, op2_info))) {
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
						ce = NULL;
						ce_is_instanceof = 0;
						on_this = delayed_fetch_this = 0;
						op1_indirect = 0;
						if (opline->op1_type == IS_UNUSED) {
							op1_info = MAY_BE_OBJECT|MAY_BE_RC1|MAY_BE_RCN;
							ce = op_array->scope;
							ce_is_instanceof = (ce->ce_flags & CREX_ACC_FINAL) != 0;
							op1_addr = 0;
							on_this = 1;
						} else {
							if (ssa_op->op1_use >= 0) {
								delayed_fetch_this = ssa->var_info[ssa_op->op1_use].delayed_fetch_this;
							}
							op1_info = OP1_INFO();
							if (!(op1_info & MAY_BE_OBJECT)) {
								break;
							}
							op1_addr = OP1_REG_ADDR();
							if (opline->op1_type == IS_VAR) {
								if (orig_op1_type != IS_UNKNOWN
								 && (orig_op1_type & IS_TRACE_INDIRECT)) {
									op1_indirect = 1;
									if (!crex_jit_fetch_indirect_var(&dasm_state, opline, orig_op1_type,
											&op1_info, &op1_addr, !ssa->var_info[ssa_op->op1_use].indirect_reference)) {
										goto jit_failure;
									}
								}
							}
							if (orig_op1_type != IS_UNKNOWN
							 && (orig_op1_type & IS_TRACE_REFERENCE)) {
								if (!crex_jit_fetch_reference(&dasm_state, opline, orig_op1_type, &op1_info, &op1_addr,
										!ssa->var_info[ssa_op->op1_use].guarded_reference, 1)) {
									goto jit_failure;
								}
								if (opline->op1_type == IS_CV
								 && ssa->vars[ssa_op->op1_def].alias == NO_ALIAS) {
									ssa->var_info[ssa_op->op1_def].guarded_reference = 1;
								}
							} else {
								CHECK_OP1_TRACE_TYPE();
							}
							if (!(op1_info & MAY_BE_OBJECT)) {
								break;
							}
							if (ssa->var_info && ssa->ops) {
								if (ssa_op->op1_use >= 0) {
									crex_ssa_var_info *op1_ssa = ssa->var_info + ssa_op->op1_use;
									if (op1_ssa->ce && !op1_ssa->ce->create_object) {
										ce = op1_ssa->ce;
										ce_is_instanceof = op1_ssa->is_instanceof;
									}
								}
							}
							if (delayed_fetch_this) {
								on_this = 1;
							} else if (ssa_op->op1_use >= 0 && ssa->vars[ssa_op->op1_use].definition >= 0) {
								on_this = ssa_opcodes[ssa->vars[ssa_op->op1_use].definition]->opcode == CREX_FETCH_THIS;
							} else if (op_array_ssa->ops
							        && op_array_ssa->vars
									&& op_array_ssa->ops[opline-op_array->opcodes].op1_use >= 0
									&& op_array_ssa->vars[op_array_ssa->ops[opline-op_array->opcodes].op1_use].definition >= 0) {
								on_this = op_array->opcodes[op_array_ssa->vars[op_array_ssa->ops[opline-op_array->opcodes].op1_use].definition].opcode == CREX_FETCH_THIS;
							}
						}
						if (!crex_jit_incdec_obj(&dasm_state, opline, op_array, ssa, ssa_op,
								op1_info, op1_addr,
								op1_indirect, ce, ce_is_instanceof, on_this, delayed_fetch_this, op1_ce,
								val_type)) {
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
						if (opline->op1_type == IS_CV
						 && (opline+1)->op1_type == IS_CV
						 && (opline+1)->op1.var == opline->op1.var) {
							/* skip $a->prop += $a; */
							break;
						}
						if (!crex_jit_supported_binary_op(
								opline->extended_value, MAY_BE_ANY, OP1_DATA_INFO())) {
							break;
						}
						ce = NULL;
						ce_is_instanceof = 0;
						on_this = delayed_fetch_this = 0;
						op1_indirect = 0;
						if (opline->op1_type == IS_UNUSED) {
							op1_info = MAY_BE_OBJECT|MAY_BE_RC1|MAY_BE_RCN;
							ce = op_array->scope;
							ce_is_instanceof = (ce->ce_flags & CREX_ACC_FINAL) != 0;
							op1_addr = 0;
							on_this = 1;
						} else {
							if (ssa_op->op1_use >= 0) {
								delayed_fetch_this = ssa->var_info[ssa_op->op1_use].delayed_fetch_this;
							}
							op1_info = OP1_INFO();
							if (!(op1_info & MAY_BE_OBJECT)) {
								break;
							}
							op1_addr = OP1_REG_ADDR();
							if (opline->op1_type == IS_VAR) {
								if (orig_op1_type != IS_UNKNOWN
								 && (orig_op1_type & IS_TRACE_INDIRECT)) {
									op1_indirect = 1;
									if (!crex_jit_fetch_indirect_var(&dasm_state, opline, orig_op1_type,
											&op1_info, &op1_addr, !ssa->var_info[ssa_op->op1_use].indirect_reference)) {
										goto jit_failure;
									}
								}
							}
							if (orig_op1_type != IS_UNKNOWN
							 && (orig_op1_type & IS_TRACE_REFERENCE)) {
								if (!crex_jit_fetch_reference(&dasm_state, opline, orig_op1_type, &op1_info, &op1_addr,
										!ssa->var_info[ssa_op->op1_use].guarded_reference, 1)) {
									goto jit_failure;
								}
								if (opline->op1_type == IS_CV
								 && ssa->vars[ssa_op->op1_def].alias == NO_ALIAS) {
									ssa->var_info[ssa_op->op1_def].guarded_reference = 1;
								}
							} else {
								CHECK_OP1_TRACE_TYPE();
							}
							if (!(op1_info & MAY_BE_OBJECT)) {
								break;
							}
							if (ssa->var_info && ssa->ops) {
								if (ssa_op->op1_use >= 0) {
									crex_ssa_var_info *op1_ssa = ssa->var_info + ssa_op->op1_use;
									if (op1_ssa->ce && !op1_ssa->ce->create_object) {
										ce = op1_ssa->ce;
										ce_is_instanceof = op1_ssa->is_instanceof;
									}
								}
							}
							if (delayed_fetch_this) {
								on_this = 1;
							} else if (ssa_op->op1_use >= 0 && ssa->vars[ssa_op->op1_use].definition >= 0) {
								on_this = ssa_opcodes[ssa->vars[ssa_op->op1_use].definition]->opcode == CREX_FETCH_THIS;
							} else if (op_array_ssa->ops
							        && op_array_ssa->vars
									&& op_array_ssa->ops[opline-op_array->opcodes].op1_use >= 0
									&& op_array_ssa->vars[op_array_ssa->ops[opline-op_array->opcodes].op1_use].definition >= 0) {
								on_this = op_array->opcodes[op_array_ssa->vars[op_array_ssa->ops[opline-op_array->opcodes].op1_use].definition].opcode == CREX_FETCH_THIS;
							}
						}
						op1_data_info = OP1_DATA_INFO();
						CHECK_OP1_DATA_TRACE_TYPE();
						if (!crex_jit_assign_obj_op(&dasm_state, opline, op_array, ssa, ssa_op,
								op1_info, op1_addr, op1_data_info, OP1_DATA_RANGE(),
								op1_indirect, ce, ce_is_instanceof, on_this, delayed_fetch_this, op1_ce,
								val_type)) {
							goto jit_failure;
						}
						goto done;
					case CREX_ASSIGN_OBJ:
						if (opline->op2_type != IS_CONST
						 || C_TYPE_P(RT_CONSTANT(opline, opline->op2)) != IS_STRING
						 || C_STRVAL_P(RT_CONSTANT(opline, opline->op2))[0] == '\0') {
							break;
						}
						ce = NULL;
						ce_is_instanceof = 0;
						on_this = delayed_fetch_this = 0;
						op1_indirect = 0;
						if (opline->op1_type == IS_UNUSED) {
							op1_info = MAY_BE_OBJECT|MAY_BE_RC1|MAY_BE_RCN;
							ce = op_array->scope;
							ce_is_instanceof = (ce->ce_flags & CREX_ACC_FINAL) != 0;
							op1_addr = 0;
							on_this = 1;
						} else {
							if (ssa_op->op1_use >= 0) {
								delayed_fetch_this = ssa->var_info[ssa_op->op1_use].delayed_fetch_this;
							}
							op1_info = OP1_INFO();
							if (!(op1_info & MAY_BE_OBJECT)) {
								break;
							}
							op1_addr = OP1_REG_ADDR();
							if (opline->op1_type == IS_VAR) {
								if (orig_op1_type != IS_UNKNOWN
								 && (orig_op1_type & IS_TRACE_INDIRECT)) {
									op1_indirect = 1;
									if (!crex_jit_fetch_indirect_var(&dasm_state, opline, orig_op1_type,
											&op1_info, &op1_addr, !ssa->var_info[ssa_op->op1_use].indirect_reference)) {
										goto jit_failure;
									}
								}
							}
							if (orig_op1_type != IS_UNKNOWN
							 && (orig_op1_type & IS_TRACE_REFERENCE)) {
								if (!crex_jit_fetch_reference(&dasm_state, opline, orig_op1_type, &op1_info, &op1_addr,
										!ssa->var_info[ssa_op->op1_use].guarded_reference, 1)) {
									goto jit_failure;
								}
								if (opline->op1_type == IS_CV
								 && ssa->vars[ssa_op->op1_def].alias == NO_ALIAS) {
									ssa->var_info[ssa_op->op1_def].guarded_reference = 1;
								}
							} else {
								CHECK_OP1_TRACE_TYPE();
							}
							if (!(op1_info & MAY_BE_OBJECT)) {
								break;
							}
							if (ssa->var_info && ssa->ops) {
								if (ssa_op->op1_use >= 0) {
									crex_ssa_var_info *op1_ssa = ssa->var_info + ssa_op->op1_use;
									if (op1_ssa->ce && !op1_ssa->ce->create_object) {
										ce = op1_ssa->ce;
										ce_is_instanceof = op1_ssa->is_instanceof;
									}
								}
							}
							if (delayed_fetch_this) {
								on_this = 1;
							} else if (ssa_op->op1_use >= 0 && ssa->vars[ssa_op->op1_use].definition >= 0) {
								on_this = ssa_opcodes[ssa->vars[ssa_op->op1_use].definition]->opcode == CREX_FETCH_THIS;
							} else if (op_array_ssa->ops
							        && op_array_ssa->vars
									&& op_array_ssa->ops[opline-op_array->opcodes].op1_use >= 0
									&& op_array_ssa->vars[op_array_ssa->ops[opline-op_array->opcodes].op1_use].definition >= 0) {
								on_this = op_array->opcodes[op_array_ssa->vars[op_array_ssa->ops[opline-op_array->opcodes].op1_use].definition].opcode == CREX_FETCH_THIS;
							}
						}
						op1_data_info = OP1_DATA_INFO();
						CHECK_OP1_DATA_TRACE_TYPE();
						if (!crex_jit_assign_obj(&dasm_state, opline, op_array, ssa, ssa_op,
								op1_info, op1_addr, op1_data_info,
								op1_indirect, ce, ce_is_instanceof, on_this, delayed_fetch_this, op1_ce,
								val_type,
								crex_may_throw(opline, ssa_op, op_array, ssa))) {
							goto jit_failure;
						}
						if ((opline+1)->op1_type == IS_CV
						 && (ssa_op+1)->op1_def >= 0
						 && ssa->vars[(ssa_op+1)->op1_def].alias == NO_ALIAS) {
							ssa->var_info[(ssa_op+1)->op1_def].guarded_reference = ssa->var_info[(ssa_op+1)->op1_use].guarded_reference;
						}
						goto done;
					case CREX_ASSIGN_DIM:
						op1_info = OP1_INFO();
						op1_addr = OP1_REG_ADDR();
						if (opline->op1_type == IS_CV
						 && (opline+1)->op1_type == IS_CV
						 && (opline+1)->op1.var == opline->op1.var) {
							/* skip $a[x] = $a; */
							break;
						}
						if (opline->op1_type == IS_VAR) {
							if (orig_op1_type != IS_UNKNOWN
							 && (orig_op1_type & IS_TRACE_INDIRECT)
							 && opline->result_type == IS_UNUSED) {
								if (!crex_jit_fetch_indirect_var(&dasm_state, opline, orig_op1_type,
										&op1_info, &op1_addr, !ssa->var_info[ssa_op->op1_use].indirect_reference)) {
									goto jit_failure;
								}
							} else {
								break;
							}
						}
						if (orig_op1_type != IS_UNKNOWN
						 && (orig_op1_type & IS_TRACE_REFERENCE)) {
							if (!crex_jit_fetch_reference(&dasm_state, opline, orig_op1_type, &op1_info, &op1_addr,
									!ssa->var_info[ssa_op->op1_use].guarded_reference, 1)) {
								goto jit_failure;
							}
							if (opline->op1_type == IS_CV
							 && ssa->vars[ssa_op->op1_def].alias == NO_ALIAS) {
								ssa->var_info[ssa_op->op1_def].guarded_reference = 1;
							}
						} else {
							CHECK_OP1_TRACE_TYPE();
						}
						op2_info = OP2_INFO();
						CHECK_OP2_TRACE_TYPE();
						op1_data_info = OP1_DATA_INFO();
						CHECK_OP1_DATA_TRACE_TYPE();
						if (!crex_jit_assign_dim(&dasm_state, opline,
								op1_info, op1_addr, op2_info, op1_data_info, val_type,
								crex_may_throw_ex(opline, ssa_op, op_array, ssa, op1_info, op2_info))) {
							goto jit_failure;
						}
						if ((opline+1)->op1_type == IS_CV
						 && (ssa_op+1)->op1_def >= 0
						 && ssa->vars[(ssa_op+1)->op1_def].alias == NO_ALIAS) {
							ssa->var_info[(ssa_op+1)->op1_def].guarded_reference = ssa->var_info[(ssa_op+1)->op1_use].guarded_reference;
						}
						goto done;
					case CREX_ASSIGN:
						if (opline->op1_type != IS_CV) {
							break;
						}
						op2_addr = OP2_REG_ADDR();
						op2_info = OP2_INFO();

						if (ssa_op->op2_def < 0 || (C_MODE(op2_addr) == IS_REG && ssa->vars[ssa_op->op2_def].no_val)) {
							op2_def_addr = op2_addr;
						} else {
							op2_def_addr = OP2_DEF_REG_ADDR();
						}
						CHECK_OP2_TRACE_TYPE();
						op1_info = OP1_INFO();
						op1_def_info = OP1_DEF_INFO();
						if (op1_type != IS_UNKNOWN && (op1_info & MAY_BE_GUARD)) {
							if (op1_type < IS_STRING
							 && (op1_info & (MAY_BE_ANY|MAY_BE_UNDEF)) != (op1_def_info & (MAY_BE_ANY|MAY_BE_UNDEF))) {
								if (!crex_jit_scalar_type_guard(&dasm_state, opline, opline->op1.var)) {
									goto jit_failure;
								}
								op1_info &= ~(MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE|MAY_BE_REF|MAY_BE_GUARD);
							} else {
								CHECK_OP1_TRACE_TYPE();
							}
						}
						op1_addr = OP1_REG_ADDR();
						op1_def_addr = OP1_DEF_REG_ADDR();
						if (C_MODE(op1_def_addr) != IS_REG &&
								STACK_TYPE(stack, EX_VAR_TO_NUM(opline->op1.var)) !=
								STACK_MEM_TYPE(stack, EX_VAR_TO_NUM(opline->op1.var))) {
							/* type may be not set */
							op1_info |= MAY_BE_NULL;
						}
						if (orig_op1_type != IS_UNKNOWN) {
							if (orig_op1_type & IS_TRACE_REFERENCE) {
								if (!crex_jit_fetch_reference(&dasm_state, opline, orig_op1_type, &op1_info, &op1_addr,
										!ssa->var_info[ssa_op->op1_use].guarded_reference, 0)) {
									goto jit_failure;
								}
								if (opline->op1_type == IS_CV
								 && ssa->vars[ssa_op->op1_def].alias == NO_ALIAS) {
									ssa->var_info[ssa_op->op1_def].guarded_reference = 1;
								}
								if (opline->result_type == IS_UNUSED) {
									res_addr = 0;
								} else {
									res_addr = RES_REG_ADDR();
									if (C_MODE(res_addr) != IS_REG
									 && crex_jit_trace_next_is_send_result(opline, p, frame)) {
										send_result = 1;
										res_addr = CREX_ADDR_MEM_ZVAL(ZREG_RX, (opline+1)->result.var);
										if (!crex_jit_reuse_ip(&dasm_state)) {
											goto jit_failure;
										}
									}
								}
								if (!crex_jit_assign_to_typed_ref(&dasm_state, opline, opline->op2_type, op2_addr, res_addr, 1)) {
									goto jit_failure;
								}
								op1_def_addr = op1_addr;
								op1_def_info &= ~MAY_BE_REF;
							} else if (op1_info & MAY_BE_REF) {
								if (!crex_jit_noref_guard(&dasm_state, opline, op1_addr)) {
									goto jit_failure;
								}
								op1_info &= ~MAY_BE_REF;
								op1_def_info &= ~MAY_BE_REF;
							}
						}
						if (opline->result_type == IS_UNUSED) {
							res_addr = 0;
							res_info = -1;
						} else {
							res_addr = RES_REG_ADDR();
							res_info = RES_INFO();
							if (C_MODE(res_addr) != IS_REG
							 && crex_jit_trace_next_is_send_result(opline, p, frame)) {
								send_result = 1;
								res_addr = CREX_ADDR_MEM_ZVAL(ZREG_RX, (opline+1)->result.var);
								if (!crex_jit_reuse_ip(&dasm_state)) {
									goto jit_failure;
								}
							}
						}
						if (!crex_jit_assign(&dasm_state, opline,
								op1_info, op1_addr,
								op1_def_info, op1_def_addr,
								op2_info, op2_addr, op2_def_addr,
								res_info, res_addr,
								crex_may_throw_ex(opline, ssa_op, op_array, ssa, op1_info, op2_info))) {
							goto jit_failure;
						}
						if (ssa_op->op2_def > 0
						 && C_MODE(op2_addr) == IS_REG
						 && ssa->vars[ssa_op->op2_def].no_val) {
							uint8_t type = (op2_info & MAY_BE_LONG) ? IS_LONG : IS_DOUBLE;
							uint32_t var_num = EX_VAR_TO_NUM(opline->op2.var);

							if (STACK_MEM_TYPE(stack, var_num) != type
							 && ssa->vars[ssa_op->op2_def].use_chain < 0
							 && !ssa->vars[ssa_op->op2_def].phi_use_chain) {
								if (!crex_jit_store_var_type(&dasm_state, var_num, type)) {
									return 0;
								}
								SET_STACK_TYPE(stack, var_num, type, 1);
							}
						}
						if (opline->op2_type == IS_CV
						 && ssa_op->op2_def >= 0
						 && ssa->vars[ssa_op->op2_def].alias == NO_ALIAS) {
							ssa->var_info[ssa_op->op2_def].guarded_reference = ssa->var_info[ssa_op->op2_use].guarded_reference;
						}
						goto done;
					case CREX_CAST:
						if (opline->extended_value != op1_type) {
							break;
						}
						CREX_FALLTHROUGH;
					case CREX_QM_ASSIGN:
						op1_addr = OP1_REG_ADDR();
						if (ssa_op->op1_def < 0 || (C_MODE(op1_addr) == IS_REG && ssa->vars[ssa_op->op1_def].no_val)) {
							op1_def_addr = op1_addr;
						} else {
							op1_def_addr = OP1_DEF_REG_ADDR();
						}
						op1_info = OP1_INFO();
						CHECK_OP1_TRACE_TYPE();
						res_info = RES_INFO();
						res_use_info = crex_jit_trace_type_to_info(
							STACK_MEM_TYPE(stack, EX_VAR_TO_NUM(opline->result.var)))
								& (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_LONG|MAY_BE_DOUBLE);
						res_addr = RES_REG_ADDR();
						if (C_MODE(res_addr) != IS_REG &&
								STACK_TYPE(stack, EX_VAR_TO_NUM(opline->result.var)) !=
								STACK_MEM_TYPE(stack, EX_VAR_TO_NUM(opline->result.var))) {
							/* type may be not set */
							res_use_info |= MAY_BE_NULL;
						}
						if (!crex_jit_qm_assign(&dasm_state, opline,
								op1_info, op1_addr, op1_def_addr,
								res_use_info, res_info, res_addr)) {
							goto jit_failure;
						}
						if (ssa_op->op1_def > 0
						 && C_MODE(op1_addr) == IS_REG
						 && ssa->vars[ssa_op->op1_def].no_val) {
							uint8_t type = (op1_info & MAY_BE_LONG) ? IS_LONG : IS_DOUBLE;
							uint32_t var_num = EX_VAR_TO_NUM(opline->op1.var);

							if (STACK_MEM_TYPE(stack, var_num) != type
							 && ssa->vars[ssa_op->op1_def].use_chain < 0
							 && !ssa->vars[ssa_op->op1_def].phi_use_chain) {
								if (!crex_jit_store_var_type(&dasm_state, var_num, type)) {
									return 0;
								}
								SET_STACK_TYPE(stack, var_num, type, 1);
							}
						}
						if (opline->op1_type == IS_CV
						 && ssa_op->op1_def >= 0
						 && ssa->vars[ssa_op->op1_def].alias == NO_ALIAS) {
							ssa->var_info[ssa_op->op1_def].guarded_reference = ssa->var_info[ssa_op->op1_use].guarded_reference;
						}
						goto done;
					case CREX_INIT_FCALL:
					case CREX_INIT_FCALL_BY_NAME:
					case CREX_INIT_NS_FCALL_BY_NAME:
						frame_flags = TRACE_FRAME_MASK_NESTED;
						if (!crex_jit_init_fcall(&dasm_state, opline, op_array_ssa->cfg.map ? op_array_ssa->cfg.map[opline - op_array->opcodes] : -1, op_array, ssa, ssa_op, frame->call_level, p + 1, peek_checked_stack - checked_stack)) {
							goto jit_failure;
						}
						goto done;
					case CREX_SEND_VAL:
					case CREX_SEND_VAL_EX:
						if (opline->op2_type == IS_CONST) {
							/* Named parameters not supported in JIT */
							break;
						}
						if (opline->opcode == CREX_SEND_VAL_EX
						 && opline->op2.num > MAX_ARG_FLAG_NUM) {
							break;
						}
						op1_info = OP1_INFO();
						CHECK_OP1_TRACE_TYPE();
						if (!crex_jit_send_val(&dasm_state, opline,
								op1_info, OP1_REG_ADDR())) {
							goto jit_failure;
						}
						if (frame->call && frame->call->func) {
							if (opline->op1_type == IS_CONST) {
								crex_jit_trace_send_type(opline, frame->call, C_TYPE_P(RT_CONSTANT(opline, opline->op1)));
							} else if (op1_type != IS_UNKNOWN) {
								if (op1_type == IS_UNDEF) {
									op1_type = IS_NULL;
								}
								crex_jit_trace_send_type(opline, frame->call, op1_type);
							}
						}
						goto done;
					case CREX_SEND_REF:
						if (opline->op2_type == IS_CONST) {
							/* Named parameters not supported in JIT */
							break;
						}
						op1_info = OP1_INFO();
						if (!crex_jit_send_ref(&dasm_state, opline, op_array,
								op1_info, 0)) {
							goto jit_failure;
						}
						if (opline->op1_type == IS_CV
						 && ssa->vars[ssa_op->op1_def].alias == NO_ALIAS) {
							ssa->var_info[ssa_op->op1_def].guarded_reference = 1;
						}
						goto done;
					case CREX_SEND_VAR:
					case CREX_SEND_VAR_EX:
					case CREX_SEND_VAR_NO_REF:
					case CREX_SEND_VAR_NO_REF_EX:
					case CREX_SEND_FUNC_ARG:
						if (opline->op2_type == IS_CONST) {
							/* Named parameters not supported in JIT */
							break;
						}
						if ((opline->opcode == CREX_SEND_VAR_EX
						  || opline->opcode == CREX_SEND_VAR_NO_REF_EX)
						 && opline->op2.num > MAX_ARG_FLAG_NUM) {
							break;
						}
						op1_addr = OP1_REG_ADDR();
						if (ssa_op->op1_def < 0 || (C_MODE(op1_addr) == IS_REG && ssa->vars[ssa_op->op1_def].no_val)) {
							op1_def_addr = op1_addr;
						} else {
							op1_def_addr = OP1_DEF_REG_ADDR();
						}
						op1_info = OP1_INFO();
						CHECK_OP1_TRACE_TYPE();
						if (!crex_jit_send_var(&dasm_state, opline, op_array,
								op1_info, op1_addr, op1_def_addr)) {
							goto jit_failure;
						}
						if (ssa_op->op1_def > 0
						 && C_MODE(op1_addr) == IS_REG
						 && ssa->vars[ssa_op->op1_def].no_val) {
							uint8_t type = (op1_info & MAY_BE_LONG) ? IS_LONG : IS_DOUBLE;
							uint32_t var_num = EX_VAR_TO_NUM(opline->op1.var);

							if (STACK_MEM_TYPE(stack, var_num) != type
							 && ssa->vars[ssa_op->op1_def].use_chain < 0
							 && !ssa->vars[ssa_op->op1_def].phi_use_chain) {
								if (!crex_jit_store_var_type(&dasm_state, var_num, type)) {
									return 0;
								}
								SET_STACK_TYPE(stack, var_num, type, 1);
							}
						}
						if (opline->op1_type == IS_CV
						 && ssa_op->op1_def >= 0
						 && ssa->vars[ssa_op->op1_def].alias == NO_ALIAS) {
							ssa->var_info[ssa_op->op1_def].guarded_reference = ssa->var_info[ssa_op->op1_use].guarded_reference;
						}
						if (frame->call && frame->call->func) {
							if ((opline->opcode == CREX_SEND_VAR_EX
							  || opline->opcode == CREX_SEND_FUNC_ARG)
							 && ARG_SHOULD_BE_SENT_BY_REF(frame->call->func, opline->op2.num)) {
								goto done;
							}
							if (op1_type != IS_UNKNOWN) {
								if (op1_type == IS_UNDEF) {
									op1_type = IS_NULL;
								}
								crex_jit_trace_send_type(opline, frame->call, op1_type);
							}
						}
						goto done;
					case CREX_CHECK_FUNC_ARG:
						if (!JIT_G(current_frame)
						 || !JIT_G(current_frame)->call
						 || !JIT_G(current_frame)->call->func) {
							break;
						}
						if (opline->op2_type == IS_CONST
						 || opline->op2.num > MAX_ARG_FLAG_NUM) {
							/* Named parameters not supported in JIT */
							TRACE_FRAME_SET_LAST_SEND_UNKNOWN(JIT_G(current_frame)->call);
							break;
						}
						if (!crex_jit_check_func_arg(&dasm_state, opline)) {
							goto jit_failure;
						}
						goto done;
					case CREX_CHECK_UNDEF_ARGS:
						if (JIT_G(current_frame)
						 && JIT_G(current_frame)->call) {
							TRACE_FRAME_SET_UNKNOWN_NUM_ARGS(JIT_G(current_frame)->call);
						}
						if (!crex_jit_check_undef_args(&dasm_state, opline)) {
							goto jit_failure;
						}
						goto done;
					case CREX_DO_UCALL:
					case CREX_DO_ICALL:
					case CREX_DO_FCALL_BY_NAME:
					case CREX_DO_FCALL:
						if (!crex_jit_do_fcall(&dasm_state, opline, op_array, op_array_ssa, frame->call_level, -1, p + 1)) {
							goto jit_failure;
						}
						goto done;
					case CREX_IS_EQUAL:
					case CREX_IS_NOT_EQUAL:
					case CREX_IS_SMALLER:
					case CREX_IS_SMALLER_OR_EQUAL:
					case CREX_CASE:
						op1_info = OP1_INFO();
						op2_info = OP2_INFO();
						skip_comparison =
							ssa_op != ssa->ops &&
							(op1_info & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_GUARD)) == MAY_BE_LONG &&
							(op2_info & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_GUARD)) == MAY_BE_LONG &&
							crex_jit_may_skip_comparison(opline, ssa_op, ssa, ssa_opcodes, op_array);
						CHECK_OP1_TRACE_TYPE();
						CHECK_OP2_TRACE_TYPE();
						if ((opline->result_type & (IS_SMART_BRANCH_JMPZ|IS_SMART_BRANCH_JMPNZ)) != 0) {
							bool exit_if_true = 0;
							const crex_op *exit_opline = crex_jit_trace_get_exit_opline(p + 1, opline + 1, &exit_if_true);
							uint32_t exit_point;

							if (ra) {
								crex_jit_trace_clenup_stack(stack, opline, ssa_op, ssa, ra);
							}
							exit_point = crex_jit_trace_get_exit_point(exit_opline, 0);
							exit_addr = crex_jit_trace_get_exit_addr(exit_point);
							if (!exit_addr) {
								goto jit_failure;
							}
							smart_branch_opcode = exit_if_true ? CREX_JMPNZ : CREX_JMPZ;
							if (!crex_jit_cmp(&dasm_state, opline,
									op1_info, OP1_RANGE(), OP1_REG_ADDR(),
									op2_info, OP2_RANGE(), OP2_REG_ADDR(),
									RES_REG_ADDR(),
									crex_may_throw(opline, ssa_op, op_array, ssa),
									smart_branch_opcode, -1, -1, exit_addr, skip_comparison)) {
								goto jit_failure;
							}
							crex_jit_trace_update_condition_ranges(opline, ssa_op, op_array, ssa, exit_if_true);
						} else {
							smart_branch_opcode = 0;
							exit_addr = NULL;
							if (!crex_jit_cmp(&dasm_state, opline,
									op1_info, OP1_RANGE(), OP1_REG_ADDR(),
									op2_info, OP2_RANGE(), OP2_REG_ADDR(),
									RES_REG_ADDR(),
									crex_may_throw(opline, ssa_op, op_array, ssa),
									smart_branch_opcode, -1, -1, exit_addr, skip_comparison)) {
								goto jit_failure;
							}
						}
						goto done;
					case CREX_IS_IDENTICAL:
					case CREX_IS_NOT_IDENTICAL:
					case CREX_CASE_STRICT:
						op1_info = OP1_INFO();
						op2_info = OP2_INFO();
						skip_comparison =
							ssa_op != ssa->ops &&
							(op1_info & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_GUARD)) == MAY_BE_LONG &&
							(op2_info & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_GUARD)) == MAY_BE_LONG &&
							crex_jit_may_skip_comparison(opline, ssa_op, ssa, ssa_opcodes, op_array);
						CHECK_OP1_TRACE_TYPE();
						CHECK_OP2_TRACE_TYPE();
						if ((opline->result_type & (IS_SMART_BRANCH_JMPZ|IS_SMART_BRANCH_JMPNZ)) != 0) {
							bool exit_if_true = 0;
							const crex_op *exit_opline = crex_jit_trace_get_exit_opline(p + 1, opline + 1, &exit_if_true);
							uint32_t exit_point;

							if (ra) {
								crex_jit_trace_clenup_stack(stack, opline, ssa_op, ssa, ra);
							}
							exit_point = crex_jit_trace_get_exit_point(exit_opline, 0);
							exit_addr = crex_jit_trace_get_exit_addr(exit_point);
							if (!exit_addr) {
								goto jit_failure;
							}
							if (opline->opcode == CREX_IS_NOT_IDENTICAL) {
								exit_if_true = !exit_if_true;
							}
							smart_branch_opcode = exit_if_true ? CREX_JMPNZ : CREX_JMPZ;
							if (!crex_jit_identical(&dasm_state, opline,
									op1_info, OP1_RANGE(), OP1_REG_ADDR(),
									op2_info, OP2_RANGE(), OP2_REG_ADDR(),
									RES_REG_ADDR(),
									crex_may_throw(opline, ssa_op, op_array, ssa),
									smart_branch_opcode, -1, -1, exit_addr, skip_comparison)) {
								goto jit_failure;
							}
							crex_jit_trace_update_condition_ranges(opline, ssa_op, op_array, ssa, exit_if_true);
						} else {
							smart_branch_opcode = 0;
							exit_addr = NULL;
							if (!crex_jit_identical(&dasm_state, opline,
									op1_info, OP1_RANGE(), OP1_REG_ADDR(),
									op2_info, OP2_RANGE(), OP2_REG_ADDR(),
									RES_REG_ADDR(),
									crex_may_throw(opline, ssa_op, op_array, ssa),
									smart_branch_opcode, -1, -1, exit_addr, skip_comparison)) {
								goto jit_failure;
							}
						}
						goto done;
					case CREX_DEFINED:
						if ((opline->result_type & (IS_SMART_BRANCH_JMPZ|IS_SMART_BRANCH_JMPNZ)) != 0) {
							bool exit_if_true = 0;
							const crex_op *exit_opline = crex_jit_trace_get_exit_opline(p + 1, opline + 1, &exit_if_true);
							uint32_t exit_point = crex_jit_trace_get_exit_point(exit_opline, 0);

							exit_addr = crex_jit_trace_get_exit_addr(exit_point);
							if (!exit_addr) {
								goto jit_failure;
							}
							smart_branch_opcode = exit_if_true ? CREX_JMPNZ : CREX_JMPZ;
						} else {
							smart_branch_opcode = 0;
							exit_addr = NULL;
						}
						if (!crex_jit_defined(&dasm_state, opline, smart_branch_opcode, -1, -1, exit_addr)) {
							goto jit_failure;
						}
						goto done;
					case CREX_TYPE_CHECK:
						if (opline->extended_value == MAY_BE_RESOURCE) {
							// TODO: support for is_resource() ???
							break;
						}
						op1_info = OP1_INFO();
						CHECK_OP1_TRACE_TYPE();
						if ((opline->result_type & (IS_SMART_BRANCH_JMPZ|IS_SMART_BRANCH_JMPNZ)) != 0) {
							bool exit_if_true = 0;
							const crex_op *exit_opline = crex_jit_trace_get_exit_opline(p + 1, opline + 1, &exit_if_true);
							uint32_t exit_point;

							if (ra) {
								crex_jit_trace_clenup_stack(stack, opline, ssa_op, ssa, ra);
							}
							exit_point = crex_jit_trace_get_exit_point(exit_opline, 0);
							exit_addr = crex_jit_trace_get_exit_addr(exit_point);
							if (!exit_addr) {
								goto jit_failure;
							}
							smart_branch_opcode = exit_if_true ? CREX_JMPNZ : CREX_JMPZ;
						} else {
							smart_branch_opcode = 0;
							exit_addr = NULL;
						}
						if (!crex_jit_type_check(&dasm_state, opline, op1_info, smart_branch_opcode, -1, -1, exit_addr)) {
							goto jit_failure;
						}
						goto done;
					case CREX_RETURN:
						op1_info = OP1_INFO();
						CHECK_OP1_TRACE_TYPE();
						if (opline->op1_type == IS_CONST) {
							res_type = C_TYPE_P(RT_CONSTANT(opline, opline->op1));
						} else if (op1_type != IS_UNKNOWN) {
							res_type = op1_type;
						}
						if (op_array->type == CREX_EVAL_CODE
						 // TODO: support for top-level code
						 || !op_array->function_name
						 // TODO: support for IS_UNDEF ???
						 || (op1_info & MAY_BE_UNDEF)) {
							if (!crex_jit_trace_handler(&dasm_state, op_array, opline, crex_may_throw(opline, ssa_op, op_array, ssa), p + 1)) {
								goto jit_failure;
							}
						} else {
							int j;
							int may_throw = 0;
							bool left_frame = 0;

							if (!crex_jit_return(&dasm_state, opline, op_array,
									op1_info, OP1_REG_ADDR())) {
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
									uint32_t info;
									uint8_t type;

									info = crex_ssa_cv_info(op_array, op_array_ssa, j);
									type = STACK_TYPE(stack, j);
									info = crex_jit_trace_type_to_info_ex(type, info);
									if (opline->op1_type == IS_CV
									 && EX_VAR_TO_NUM(opline->op1.var) == j
									 && !(op1_info & (MAY_BE_REF|MAY_BE_OBJECT))) {
										if (JIT_G(current_frame)
										 && TRACE_FRAME_IS_RETURN_VALUE_USED(JIT_G(current_frame))) {
											continue;
										} else {
											info |= MAY_BE_NULL;
										}
									}
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
										if (info & (MAY_BE_OBJECT|MAY_BE_RESOURCE|MAY_BE_ARRAY_OF_OBJECT|MAY_BE_ARRAY_OF_ARRAY|MAY_BE_ARRAY_OF_RESOURCE)) {
											if (info & MAY_BE_RC1) {
												may_throw = 1;
											}
										}
									}
								}
							}
							if (!crex_jit_leave_func(&dasm_state, op_array, opline, op1_info, left_frame,
									p + 1, &crex_jit_traces[CREX_JIT_TRACE_NUM],
									(op_array_ssa->cfg.flags & CREX_FUNC_INDIRECT_VAR_ACCESS) != 0, may_throw)) {
								goto jit_failure;
							}
						}
						goto done;
					case CREX_BOOL:
					case CREX_BOOL_NOT:
						op1_info = OP1_INFO();
						CHECK_OP1_TRACE_TYPE();
						if (!crex_jit_bool_jmpznz(&dasm_state, opline,
								op1_info, OP1_REG_ADDR(), RES_REG_ADDR(),
								-1, -1,
								crex_may_throw(opline, ssa_op, op_array, ssa),
								opline->opcode, NULL)) {
							goto jit_failure;
						}
						goto done;
					case CREX_JMPZ:
					case CREX_JMPNZ:
					case CREX_JMPC_EX:
					case CREX_JMPNC_EX:
						op1_info = OP1_INFO();
						CHECK_OP1_TRACE_TYPE();
						if ((p+1)->op == CREX_JIT_TRACE_VM || (p+1)->op == CREX_JIT_TRACE_END) {
							const crex_op *exit_opline = NULL;
							uint32_t exit_point;

							if ((p+1)->opline == OP_JMP_ADDR(opline, opline->op2)) {
								/* taken branch */
								if (opline->opcode == CREX_JMPNC_EX) {
									smart_branch_opcode = CREX_JMPC_EX;
								} else if (opline->opcode == CREX_JMPC_EX) {
									smart_branch_opcode = CREX_JMPNC_EX;
								} else if (opline->opcode == CREX_JMPNZ) {
									smart_branch_opcode = CREX_JMPZ;
								} else {
									smart_branch_opcode = CREX_JMPNZ;
								}
								exit_opline = opline + 1;
							} else if ((p+1)->opline == opline + 1) {
								/* not taken branch */
								smart_branch_opcode = opline->opcode;
								exit_opline = OP_JMP_ADDR(opline, opline->op2);
							} else {
								CREX_UNREACHABLE();
							}
							if (ra) {
								crex_jit_trace_clenup_stack(stack, opline, ssa_op, ssa, ra);
							}
							if (!(op1_info & MAY_BE_GUARD)
							 && has_concrete_type(op1_info)
							 && concrete_type(op1_info) <= IS_TRUE) {
								/* unconditional branch */
								exit_addr = NULL;
							} else if (opline->result_type == IS_TMP_VAR) {
								crex_jit_trace_stack *stack = JIT_G(current_frame)->stack;
								uint32_t old_info = STACK_INFO(stack, EX_VAR_TO_NUM(opline->result.var));

								SET_STACK_TYPE(stack, EX_VAR_TO_NUM(opline->result.var), IS_UNKNOWN, 1);
								exit_point = crex_jit_trace_get_exit_point(exit_opline, 0);
								SET_STACK_INFO(stack, EX_VAR_TO_NUM(opline->result.var), old_info);
								exit_addr = crex_jit_trace_get_exit_addr(exit_point);
								if (!exit_addr) {
									goto jit_failure;
								}
							} else {
								exit_point = crex_jit_trace_get_exit_point(exit_opline, 0);
								exit_addr = crex_jit_trace_get_exit_addr(exit_point);
								if (!exit_addr) {
									goto jit_failure;
								}
							}
						} else  {
							CREX_UNREACHABLE();
						}
						if (opline->result_type == IS_UNDEF) {
							res_addr = 0;
						} else {
							res_addr = RES_REG_ADDR();
						}
						if (!crex_jit_bool_jmpznz(&dasm_state, opline,
								op1_info, OP1_REG_ADDR(), res_addr,
								-1, -1,
								crex_may_throw(opline, ssa_op, op_array, ssa),
								smart_branch_opcode, exit_addr)) {
							goto jit_failure;
						}
						goto done;
					case CREX_ISSET_ISEMPTY_CV:
						if ((opline->extended_value & CREX_ISEMPTY)) {
							// TODO: support for empty() ???
							break;
						}
						op1_info = OP1_INFO();
						op1_addr = OP1_REG_ADDR();
						if (orig_op1_type != IS_UNKNOWN
						 && (orig_op1_type & IS_TRACE_REFERENCE)) {
							if (!crex_jit_fetch_reference(&dasm_state, opline, orig_op1_type, &op1_info, &op1_addr,
									!ssa->var_info[ssa_op->op1_use].guarded_reference, 1)) {
								goto jit_failure;
							}
							if (opline->op1_type == IS_CV
							 && ssa->vars[ssa_op->op1_use].alias == NO_ALIAS) {
								ssa->var_info[ssa_op->op1_use].guarded_reference = 1;
							}
						} else {
							CHECK_OP1_TRACE_TYPE();
						}
						if ((opline->result_type & (IS_SMART_BRANCH_JMPZ|IS_SMART_BRANCH_JMPNZ)) != 0) {
							bool exit_if_true = 0;
							const crex_op *exit_opline = crex_jit_trace_get_exit_opline(p + 1, opline + 1, &exit_if_true);
							uint32_t exit_point = crex_jit_trace_get_exit_point(exit_opline, 0);

							exit_addr = crex_jit_trace_get_exit_addr(exit_point);
							if (!exit_addr) {
								goto jit_failure;
							}
							smart_branch_opcode = exit_if_true ? CREX_JMPNZ : CREX_JMPZ;
						} else {
							smart_branch_opcode = 0;
							exit_addr = NULL;
						}
						if (!crex_jit_isset_isempty_cv(&dasm_state, opline,
								op1_info, op1_addr,
								smart_branch_opcode, -1, -1, exit_addr)) {
							goto jit_failure;
						}
						goto done;
					case CREX_IN_ARRAY:
						if (opline->op1_type == IS_VAR || opline->op1_type == IS_TMP_VAR) {
							break;
						}
						op1_info = OP1_INFO();
						op1_addr = OP1_REG_ADDR();
						CHECK_OP1_TRACE_TYPE();
						if ((op1_info & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_REF)) != MAY_BE_STRING) {
							break;
						}
						if ((opline->result_type & (IS_SMART_BRANCH_JMPZ|IS_SMART_BRANCH_JMPNZ)) != 0) {
							bool exit_if_true = 0;
							const crex_op *exit_opline = crex_jit_trace_get_exit_opline(p + 1, opline + 1, &exit_if_true);
							uint32_t exit_point = crex_jit_trace_get_exit_point(exit_opline, 0);

							exit_addr = crex_jit_trace_get_exit_addr(exit_point);
							if (!exit_addr) {
								goto jit_failure;
							}
							smart_branch_opcode = exit_if_true ? CREX_JMPNZ : CREX_JMPZ;
						} else {
							smart_branch_opcode = 0;
							exit_addr = NULL;
						}
						if (!crex_jit_in_array(&dasm_state, opline,
								op1_info, op1_addr,
								smart_branch_opcode, -1, -1, exit_addr)) {
							goto jit_failure;
						}
						goto done;
					case CREX_FETCH_DIM_FUNC_ARG:
						if (!JIT_G(current_frame)
						 || !JIT_G(current_frame)->call
						 || !JIT_G(current_frame)->call->func
						 || !TRACE_FRAME_IS_LAST_SEND_BY_VAL(JIT_G(current_frame)->call)) {
							break;
						}
						CREX_FALLTHROUGH;
					case CREX_FETCH_DIM_R:
					case CREX_FETCH_DIM_IS:
					case CREX_FETCH_LIST_R:
						op1_info = OP1_INFO();
						op1_addr = OP1_REG_ADDR();
						if (orig_op1_type != IS_UNKNOWN
						 && (orig_op1_type & IS_TRACE_REFERENCE)) {
							if (!crex_jit_fetch_reference(&dasm_state, opline, orig_op1_type, &op1_info, &op1_addr,
									!ssa->var_info[ssa_op->op1_use].guarded_reference, 1)) {
								goto jit_failure;
							}
							if (opline->op1_type == IS_CV
							 && ssa->vars[ssa_op->op1_use].alias == NO_ALIAS) {
								ssa->var_info[ssa_op->op1_use].guarded_reference = 1;
								if (ssa_op->op1_def >= 0) {
									ssa->var_info[ssa_op->op1_def].guarded_reference = 1;
								}
							}
						} else {
							CHECK_OP1_TRACE_TYPE();
						}
						op2_info = OP2_INFO();
						CHECK_OP2_TRACE_TYPE();
						res_info = RES_INFO();
						avoid_refcounting =
							ssa_op->op1_use >= 0 &&
							ssa->var_info[ssa_op->op1_use].avoid_refcounting;
						if (op1_info & MAY_BE_PACKED_GUARD) {
							ssa->var_info[ssa_op->op1_use].type &= ~MAY_BE_PACKED_GUARD;
						} else if ((op2_info & (MAY_BE_ANY|MAY_BE_UNDEF)) == MAY_BE_LONG
								&& (op1_info & (MAY_BE_ANY|MAY_BE_UNDEF)) == MAY_BE_ARRAY
								&& MAY_BE_PACKED(op1_info)
								&& MAY_BE_HASH(op1_info)
								&& orig_op1_type != IS_UNKNOWN) {
							op1_info |= MAY_BE_PACKED_GUARD;
							if (orig_op1_type & IS_TRACE_PACKED) {
								op1_info &= ~(MAY_BE_ARRAY_NUMERIC_HASH|MAY_BE_ARRAY_STRING_HASH);
								if (op1_type != IS_UNKNOWN) {
									ssa->var_info[ssa_op->op1_use].type &= ~(MAY_BE_ARRAY_NUMERIC_HASH|MAY_BE_ARRAY_STRING_HASH);
								}
							} else {
								op1_info &= ~MAY_BE_ARRAY_PACKED;
								if (op1_type != IS_UNKNOWN) {
									ssa->var_info[ssa_op->op1_use].type &= ~MAY_BE_ARRAY_PACKED;
								}
							}
						}
						if (!crex_jit_fetch_dim_read(&dasm_state, opline, ssa, ssa_op,
								op1_info, op1_addr, avoid_refcounting,
								op2_info, res_info, RES_REG_ADDR(), val_type)) {
							goto jit_failure;
						}
						if (ssa_op->op1_def >= 0 && op1_type != IS_UNKNOWN) {
							ssa->var_info[ssa_op->op1_def].type = ssa->var_info[ssa_op->op1_use].type;
						}
						goto done;
					case CREX_FETCH_DIM_W:
					case CREX_FETCH_DIM_RW:
//					case CREX_FETCH_DIM_UNSET:
					case CREX_FETCH_LIST_W:
						if (opline->op1_type != IS_CV
						 && (orig_op1_type == IS_UNKNOWN
						  || !(orig_op1_type & IS_TRACE_INDIRECT))) {
							break;
						}
						op1_info = OP1_INFO();
						op1_addr = OP1_REG_ADDR();
						if (opline->op1_type == IS_VAR) {
							if (orig_op1_type != IS_UNKNOWN
							 && (orig_op1_type & IS_TRACE_INDIRECT)) {
								if (!crex_jit_fetch_indirect_var(&dasm_state, opline, orig_op1_type,
										&op1_info, &op1_addr, !ssa->var_info[ssa_op->op1_use].indirect_reference)) {
									goto jit_failure;
								}
							} else {
								break;
							}
						}
						if (orig_op1_type != IS_UNKNOWN
						 && (orig_op1_type & IS_TRACE_REFERENCE)) {
							if (!crex_jit_fetch_reference(&dasm_state, opline, orig_op1_type, &op1_info, &op1_addr,
									!ssa->var_info[ssa_op->op1_use].guarded_reference, 1)) {
								goto jit_failure;
							}
							if (opline->op1_type == IS_CV
							 && ssa->vars[ssa_op->op1_def].alias == NO_ALIAS) {
								ssa->var_info[ssa_op->op1_def].guarded_reference = 1;
							}
						} else {
							CHECK_OP1_TRACE_TYPE();
						}
						op2_info = OP2_INFO();
						CHECK_OP2_TRACE_TYPE();
						op1_def_info = OP1_DEF_INFO();
						if (!crex_jit_fetch_dim(&dasm_state, opline,
								op1_info, op1_addr, op2_info, RES_REG_ADDR(), val_type)) {
							goto jit_failure;
						}
						if (ssa_op->result_def > 0
						 && (opline->opcode == CREX_FETCH_DIM_W || opline->opcode == CREX_FETCH_LIST_W)
						 && !(op1_info & (MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_STRING|MAY_BE_OBJECT|MAY_BE_RESOURCE|MAY_BE_REF))
						 && !(op2_info & (MAY_BE_UNDEF|MAY_BE_RESOURCE|MAY_BE_ARRAY|MAY_BE_OBJECT))) {
							ssa->var_info[ssa_op->result_def].indirect_reference = 1;
						}
						goto done;
					case CREX_ISSET_ISEMPTY_DIM_OBJ:
						if ((opline->extended_value & CREX_ISEMPTY)) {
							// TODO: support for empty() ???
							break;
						}
						op1_info = OP1_INFO();
						op1_addr = OP1_REG_ADDR();
						if (orig_op1_type != IS_UNKNOWN
						 && (orig_op1_type & IS_TRACE_REFERENCE)) {
							if (!crex_jit_fetch_reference(&dasm_state, opline, orig_op1_type, &op1_info, &op1_addr,
									!ssa->var_info[ssa_op->op1_use].guarded_reference, 1)) {
								goto jit_failure;
							}
							if (opline->op1_type == IS_CV
							 && ssa->vars[ssa_op->op1_use].alias == NO_ALIAS) {
								ssa->var_info[ssa_op->op1_use].guarded_reference = 1;
							}
						} else {
							CHECK_OP1_TRACE_TYPE();
						}
						op2_info = OP2_INFO();
						CHECK_OP2_TRACE_TYPE();
						if ((opline->result_type & (IS_SMART_BRANCH_JMPZ|IS_SMART_BRANCH_JMPNZ)) != 0) {
							bool exit_if_true = 0;
							const crex_op *exit_opline = crex_jit_trace_get_exit_opline(p + 1, opline + 1, &exit_if_true);
							uint32_t exit_point;

							if (ra) {
								crex_jit_trace_clenup_stack(stack, opline, ssa_op, ssa, ra);
							}
							if (ssa_op->op1_use >= 0
							 && ssa->var_info[ssa_op->op1_use].avoid_refcounting) {
								/* Temporary reset ZREG_ZVAL_TRY_ADDREF */
								crex_jit_trace_stack *stack = JIT_G(current_frame)->stack;
								uint32_t old_info = STACK_INFO(stack, EX_VAR_TO_NUM(opline->op1.var));

								SET_STACK_REG(stack, EX_VAR_TO_NUM(opline->op1.var), ZREG_NONE);
								exit_point = crex_jit_trace_get_exit_point(exit_opline, 0);
								SET_STACK_INFO(stack, EX_VAR_TO_NUM(opline->op1.var), old_info);
							} else {
								exit_point = crex_jit_trace_get_exit_point(exit_opline, 0);
							}
							exit_addr = crex_jit_trace_get_exit_addr(exit_point);
							if (!exit_addr) {
								goto jit_failure;
							}
							smart_branch_opcode = exit_if_true ? CREX_JMPNZ : CREX_JMPZ;
						} else {
							smart_branch_opcode = 0;
							exit_addr = NULL;
						}
						avoid_refcounting =
							ssa_op->op1_use >= 0 &&
							ssa->var_info[ssa_op->op1_use].avoid_refcounting;
						if (op1_info & MAY_BE_PACKED_GUARD) {
							ssa->var_info[ssa_op->op1_use].type &= ~MAY_BE_PACKED_GUARD;
						} else if ((op2_info & (MAY_BE_ANY|MAY_BE_UNDEF)) == MAY_BE_LONG
								&& (op1_info & (MAY_BE_ANY|MAY_BE_UNDEF)) == MAY_BE_ARRAY
								&& MAY_BE_PACKED(op1_info)
								&& MAY_BE_HASH(op1_info)
								&& orig_op1_type != IS_UNKNOWN) {
							op1_info |= MAY_BE_PACKED_GUARD;
							if (orig_op1_type & IS_TRACE_PACKED) {
								op1_info &= ~(MAY_BE_ARRAY_NUMERIC_HASH|MAY_BE_ARRAY_STRING_HASH);
							} else {
								op1_info &= ~MAY_BE_ARRAY_PACKED;
							}
						}
						if (!crex_jit_isset_isempty_dim(&dasm_state, opline,
								op1_info, op1_addr, avoid_refcounting,
								op2_info, val_type,
								crex_may_throw_ex(opline, ssa_op, op_array, ssa, op1_info, op2_info),
								smart_branch_opcode, -1, -1,
								exit_addr)) {
							goto jit_failure;
						}
						goto done;
					case CREX_FETCH_OBJ_FUNC_ARG:
						if (!JIT_G(current_frame)
						 || !JIT_G(current_frame)->call
						 || !JIT_G(current_frame)->call->func
						 || !TRACE_FRAME_IS_LAST_SEND_BY_VAL(JIT_G(current_frame)->call)) {
							break;
						}
						CREX_FALLTHROUGH;
					case CREX_FETCH_OBJ_R:
					case CREX_FETCH_OBJ_IS:
					case CREX_FETCH_OBJ_W:
						on_this = delayed_fetch_this = 0;
						avoid_refcounting = 0;
						if (opline->op2_type != IS_CONST
						 || C_TYPE_P(RT_CONSTANT(opline, opline->op2)) != IS_STRING
						 || C_STRVAL_P(RT_CONSTANT(opline, opline->op2))[0] == '\0') {
							break;
						}
						ce = NULL;
						ce_is_instanceof = 0;
						op1_indirect = 0;
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
							if (opline->op1_type == IS_VAR
							 && opline->opcode == CREX_FETCH_OBJ_W) {
								if (orig_op1_type != IS_UNKNOWN
								 && (orig_op1_type & IS_TRACE_INDIRECT)) {
									op1_indirect = 1;
									if (!crex_jit_fetch_indirect_var(&dasm_state, opline, orig_op1_type,
											&op1_info, &op1_addr, !ssa->var_info[ssa_op->op1_use].indirect_reference)) {
										goto jit_failure;
									}
								}
							}
							if (orig_op1_type != IS_UNKNOWN
							 && (orig_op1_type & IS_TRACE_REFERENCE)) {
								if (!crex_jit_fetch_reference(&dasm_state, opline, orig_op1_type, &op1_info, &op1_addr,
										!ssa->var_info[ssa_op->op1_use].guarded_reference, 1)) {
									goto jit_failure;
								}
								if (opline->op1_type == IS_CV
								 && ssa->vars[ssa_op->op1_use].alias == NO_ALIAS) {
									ssa->var_info[ssa_op->op1_def >= 0 ? ssa_op->op1_def : ssa_op->op1_use].guarded_reference = 1;
								}
							} else {
								CHECK_OP1_TRACE_TYPE();
							}
							if (!(op1_info & MAY_BE_OBJECT)) {
								break;
							}
							if (ssa->var_info && ssa->ops) {
								if (ssa_op->op1_use >= 0) {
									crex_ssa_var_info *op1_ssa = ssa->var_info + ssa_op->op1_use;
									if (op1_ssa->ce && !op1_ssa->ce->create_object) {
										ce = op1_ssa->ce;
										ce_is_instanceof = op1_ssa->is_instanceof;
									}
								}
							}
							if (ssa_op->op1_use >= 0) {
								delayed_fetch_this = ssa->var_info[ssa_op->op1_use].delayed_fetch_this;
								avoid_refcounting = ssa->var_info[ssa_op->op1_use].avoid_refcounting;
							}
							if (delayed_fetch_this) {
								on_this = 1;
							} else if (ssa_op->op1_use >= 0 && ssa->vars[ssa_op->op1_use].definition >= 0) {
								on_this = ssa_opcodes[ssa->vars[ssa_op->op1_use].definition]->opcode == CREX_FETCH_THIS;
							} else if (op_array_ssa->ops
							        && op_array_ssa->vars
									&& op_array_ssa->ops[opline-op_array->opcodes].op1_use >= 0
									&& op_array_ssa->vars[op_array_ssa->ops[opline-op_array->opcodes].op1_use].definition >= 0) {
								on_this = op_array->opcodes[op_array_ssa->vars[op_array_ssa->ops[opline-op_array->opcodes].op1_use].definition].opcode == CREX_FETCH_THIS;
							}
						}
						if (!crex_jit_fetch_obj(&dasm_state, opline, op_array, ssa, ssa_op,
								op1_info, op1_addr, op1_indirect, ce, ce_is_instanceof,
								on_this, delayed_fetch_this, avoid_refcounting, op1_ce, val_type,
								crex_may_throw_ex(opline, ssa_op, op_array, ssa, op1_info, MAY_BE_STRING))) {
							goto jit_failure;
						}
						goto done;
					case CREX_BIND_GLOBAL:
						orig_opline = opline;
						orig_ssa_op = ssa_op;
						while (1) {
							if (!ssa->ops || !ssa->var_info) {
								op1_info = MAY_BE_ANY|MAY_BE_REF;
							} else {
								op1_info = OP1_INFO();
							}
							if (ssa->vars[ssa_op->op1_def].alias == NO_ALIAS) {
								ssa->var_info[ssa_op->op1_def].guarded_reference = 1;
							}
							if (!crex_jit_bind_global(&dasm_state, opline, op1_info)) {
								goto jit_failure;
							}
							if ((opline+1)->opcode == CREX_BIND_GLOBAL) {
								opline++;
								ssa_op++;
							} else {
								break;
							}
						}
						opline = orig_opline;
						ssa_op = orig_ssa_op;
						goto done;
					case CREX_RECV:
						if (!crex_jit_recv(&dasm_state, opline, op_array)) {
							goto jit_failure;
						}
						goto done;
					case CREX_RECV_INIT:
						orig_opline = opline;
						orig_ssa_op = ssa_op;
						while (1) {
							if (!crex_jit_recv_init(&dasm_state, opline, op_array,
									(opline + 1)->opcode != CREX_RECV_INIT,
									crex_may_throw(opline, ssa_op, op_array, ssa))) {
								goto jit_failure;
							}
							if ((opline+1)->opcode == CREX_RECV_INIT) {
								opline++;
								ssa_op++;
							} else {
								break;
							}
						}
						opline = orig_opline;
						ssa_op = orig_ssa_op;
						goto done;
					case CREX_FREE:
					case CREX_FE_FREE:
						op1_info = OP1_INFO();
						if (!crex_jit_free(&dasm_state, opline, op1_info,
								crex_may_throw(opline, ssa_op, op_array, ssa))) {
							goto jit_failure;
						}
						goto done;
					case CREX_ECHO:
						op1_info = OP1_INFO();
						CHECK_OP1_TRACE_TYPE();
						if ((op1_info & (MAY_BE_UNDEF|MAY_BE_ANY|MAY_BE_REF)) != MAY_BE_STRING) {
							break;
						}
						if (!crex_jit_echo(&dasm_state, opline, op1_info)) {
							goto jit_failure;
						}
						goto done;
					case CREX_STRLEN:
						op1_info = OP1_INFO();
						op1_addr = OP1_REG_ADDR();
						if (orig_op1_type == (IS_TRACE_REFERENCE|IS_STRING)) {
							if (!crex_jit_fetch_reference(&dasm_state, opline, orig_op1_type, &op1_info, &op1_addr,
									!ssa->var_info[ssa_op->op1_use].guarded_reference, 1)) {
								goto jit_failure;
							}
							if (opline->op1_type == IS_CV
							 && ssa->vars[ssa_op->op1_use].alias == NO_ALIAS) {
								ssa->var_info[ssa_op->op1_use].guarded_reference = 1;
							}
						} else {
							CHECK_OP1_TRACE_TYPE();
							if ((op1_info & (MAY_BE_UNDEF|MAY_BE_ANY|MAY_BE_REF)) != MAY_BE_STRING) {
								break;
							}
						}
						if (!crex_jit_strlen(&dasm_state, opline, op1_info, op1_addr, RES_REG_ADDR())) {
							goto jit_failure;
						}
						goto done;
					case CREX_COUNT:
						op1_info = OP1_INFO();
						op1_addr = OP1_REG_ADDR();
						if (orig_op1_type == (IS_TRACE_REFERENCE|IS_ARRAY)) {
							if (!crex_jit_fetch_reference(&dasm_state, opline, orig_op1_type, &op1_info, &op1_addr,
									!ssa->var_info[ssa_op->op1_use].guarded_reference, 1)) {
								goto jit_failure;
							}
							if (opline->op1_type == IS_CV
							 && ssa->vars[ssa_op->op1_use].alias == NO_ALIAS) {
								ssa->var_info[ssa_op->op1_use].guarded_reference = 1;
							}
						} else {
							CHECK_OP1_TRACE_TYPE();
							if ((op1_info & (MAY_BE_UNDEF|MAY_BE_ANY|MAY_BE_REF)) != MAY_BE_ARRAY) {
								break;
							}
						}
						if (!crex_jit_count(&dasm_state, opline, op1_info, op1_addr, RES_REG_ADDR(), crex_may_throw(opline, ssa_op, op_array, ssa))) {
							goto jit_failure;
						}
						goto done;
					case CREX_FETCH_THIS:
						delayed_fetch_this = 0;
						if (ssa_op->result_def >= 0 && opline->result_type != IS_CV) {
							if (crex_jit_may_delay_fetch_this(op_array, ssa, ssa_opcodes, ssa_op)) {
								ssa->var_info[ssa_op->result_def].delayed_fetch_this = 1;
								delayed_fetch_this = 1;
							}
						}
						if (!crex_jit_fetch_this(&dasm_state, opline, op_array, delayed_fetch_this)) {
							goto jit_failure;
						}
						goto done;
					case CREX_SWITCH_LONG:
					case CREX_SWITCH_STRING:
					case CREX_MATCH:
						if (!crex_jit_switch(&dasm_state, opline, op_array, op_array_ssa, p+1, &crex_jit_traces[CREX_JIT_TRACE_NUM])) {
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
						op1_info = OP1_INFO();
						CHECK_OP1_TRACE_TYPE();
						if (op1_info & MAY_BE_REF) {
							/* TODO May need reference unwrapping. */
							break;
						}
						if (!crex_jit_verify_return_type(&dasm_state, opline, op_array, op1_info)) {
							goto jit_failure;
						}
						goto done;
					case CREX_FE_RESET_R:
						op1_info = OP1_INFO();
						CHECK_OP1_TRACE_TYPE();
						if ((op1_info & (MAY_BE_ANY|MAY_BE_REF|MAY_BE_UNDEF)) != MAY_BE_ARRAY) {
							break;
						}
						if (!crex_jit_fe_reset(&dasm_state, opline, op1_info)) {
							goto jit_failure;
						}
						goto done;
					case CREX_FE_FETCH_R:
						op1_info = OP1_INFO();
						CHECK_OP1_TRACE_TYPE();
						if ((op1_info & MAY_BE_ANY) != MAY_BE_ARRAY) {
							break;
						}
						if ((p+1)->op == CREX_JIT_TRACE_VM || (p+1)->op == CREX_JIT_TRACE_END) {
							const crex_op *exit_opline = CREX_OFFSET_TO_OPLINE(opline, opline->extended_value);
							uint32_t exit_point;

							if ((p+1)->opline == exit_opline) {
								/* taken branch (exit from loop) */
								exit_opline = opline;
								smart_branch_opcode = CREX_NOP;
							} else if ((p+1)->opline == opline + 1) {
								/* not taken branch (loop) */
								smart_branch_opcode = CREX_JMP;
							} else {
								CREX_UNREACHABLE();
							}
							exit_point = crex_jit_trace_get_exit_point(exit_opline, 0);
							exit_addr = crex_jit_trace_get_exit_addr(exit_point);
							if (!exit_addr) {
								goto jit_failure;
							}
						} else  {
							CREX_UNREACHABLE();
						}
						if (!crex_jit_fe_fetch(&dasm_state, opline, op1_info, OP2_INFO(),
								-1, smart_branch_opcode, exit_addr)) {
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
						on_this = delayed_fetch_this = 0;
						ce = NULL;
						ce_is_instanceof = 0;
						if (opline->op1_type == IS_UNUSED) {
							op1_info = MAY_BE_OBJECT|MAY_BE_RC1|MAY_BE_RCN;
							ce = op_array->scope;
							ce_is_instanceof = (ce->ce_flags & CREX_ACC_FINAL) != 0;
							op1_addr = 0;
							on_this = 1;
						} else {
							op1_info = OP1_INFO();
							op1_addr = OP1_REG_ADDR();
							if (polymorphic_side_trace) {
								op1_info = MAY_BE_OBJECT;
								op1_addr = 0;
							} else if (orig_op1_type != IS_UNKNOWN
							 && (orig_op1_type & IS_TRACE_REFERENCE)) {
								if (!crex_jit_fetch_reference(&dasm_state, opline, orig_op1_type, &op1_info, &op1_addr,
										!ssa->var_info[ssa_op->op1_use].guarded_reference, 1)) {
									goto jit_failure;
								}
								if (opline->op1_type == IS_CV
								 && ssa->vars[ssa_op->op1_use].alias == NO_ALIAS) {
									ssa->var_info[ssa_op->op1_use].guarded_reference = 1;
								}
							} else {
								CHECK_OP1_TRACE_TYPE();
							}
							if (ssa->var_info && ssa->ops) {
								if (ssa_op->op1_use >= 0) {
									crex_ssa_var_info *op1_ssa = ssa->var_info + ssa_op->op1_use;
									if (op1_ssa->ce && !op1_ssa->ce->create_object) {
										ce = op1_ssa->ce;
										ce_is_instanceof = op1_ssa->is_instanceof;
									}
								}
							}
							if (ssa_op->op1_use >= 0) {
								delayed_fetch_this = ssa->var_info[ssa_op->op1_use].delayed_fetch_this;
							}
							if (delayed_fetch_this) {
								on_this = 1;
							} else if (ssa_op->op1_use >= 0 && ssa->vars[ssa_op->op1_use].definition >= 0) {
								on_this = ssa_opcodes[ssa->vars[ssa_op->op1_use].definition]->opcode == CREX_FETCH_THIS;
							} else if (op_array_ssa->ops
							        && op_array_ssa->vars
									&& op_array_ssa->ops[opline-op_array->opcodes].op1_use >= 0
									&& op_array_ssa->vars[op_array_ssa->ops[opline-op_array->opcodes].op1_use].definition >= 0) {
								on_this = op_array->opcodes[op_array_ssa->vars[op_array_ssa->ops[opline-op_array->opcodes].op1_use].definition].opcode == CREX_FETCH_THIS;
							}
						}
						frame_flags = TRACE_FRAME_MASK_NESTED;
						if (!crex_jit_init_method_call(&dasm_state, opline,
								op_array_ssa->cfg.map ? op_array_ssa->cfg.map[opline - op_array->opcodes] : -1,
								op_array, ssa, ssa_op, frame->call_level,
								op1_info, op1_addr, ce, ce_is_instanceof, on_this, delayed_fetch_this, op1_ce,
								p + 1, peek_checked_stack - checked_stack, polymorphic_side_trace)) {
							goto jit_failure;
						}
						goto done;
					case CREX_INIT_DYNAMIC_CALL:
						if (orig_op2_type != IS_OBJECT || op2_ce != crex_ce_closure) {
							break;
						}
						op2_info = OP2_INFO();
						CHECK_OP2_TRACE_TYPE();
						frame_flags = TRACE_FRAME_MASK_NESTED;
						if (!crex_jit_init_closure_call(&dasm_state, opline, op_array_ssa->cfg.map ? op_array_ssa->cfg.map[opline - op_array->opcodes] : -1, op_array, ssa, ssa_op, frame->call_level, p + 1, peek_checked_stack - checked_stack)) {
							goto jit_failure;
						}
						goto done;
					case CREX_SEND_ARRAY:
					case CREX_SEND_UNPACK:
						if (JIT_G(current_frame)
						 && JIT_G(current_frame)->call) {
							TRACE_FRAME_SET_UNKNOWN_NUM_ARGS(JIT_G(current_frame)->call);
						}
						break;
					case CREX_ROPE_INIT:
					case CREX_ROPE_ADD:
					case CREX_ROPE_END:
						op2_info = OP2_INFO();
						CHECK_OP2_TRACE_TYPE();
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

			if (opline->opcode != CREX_NOP && opline->opcode != CREX_JMP) {
				gen_handler = 1;
				op1_info = OP1_INFO();
				op2_info = OP2_INFO();
				if (op1_info & MAY_BE_GUARD) {
					op1_info = MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_REF | MAY_BE_ANY  | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				}
				if (op2_info & MAY_BE_GUARD) {
					op2_info = MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_REF | MAY_BE_ANY  | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				}
				if (!crex_jit_trace_handler(&dasm_state, op_array, opline,
						crex_may_throw_ex(opline, ssa_op, op_array, ssa, op1_info, op2_info), p + 1)) {
					goto jit_failure;
				}
				if ((p+1)->op == CREX_JIT_TRACE_INIT_CALL && (p+1)->func) {
					if (opline->opcode == CREX_NEW && ssa_op->result_def >= 0) {
						SET_STACK_TYPE(stack, EX_VAR_TO_NUM(opline->result.var), IS_OBJECT, 1);
					}
					if (crex_jit_may_be_polymorphic_call(opline) ||
							crex_jit_may_be_modified((p+1)->func, op_array)) {
						if (!crex_jit_init_fcall_guard(&dasm_state, 0, (p+1)->func, opline+1)) {
							goto jit_failure;
						}
					}
				}
			}

done:
			polymorphic_side_trace = 0;
			switch (opline->opcode) {
				case CREX_DO_FCALL:
				case CREX_DO_ICALL:
				case CREX_DO_UCALL:
				case CREX_DO_FCALL_BY_NAME:
				case CREX_CALLABLE_CONVERT:
					frame->call_level--;
			}

			if (ra) {
				crex_jit_trace_clenup_stack(stack, opline, ssa_op, ssa, ra);
			}

			if ((opline->op1_type & (IS_VAR|IS_TMP_VAR))
			 && STACK_REG(stack, EX_VAR_TO_NUM(opline->op1.var)) > ZREG_NUM) {
				SET_STACK_REG(stack, EX_VAR_TO_NUM(opline->op1.var), ZREG_NONE);
			}

			if (opline->opcode == CREX_ROPE_INIT) {
				/* clear stack slots used by rope */
				uint32_t var = EX_VAR_TO_NUM(opline->result.var);
				uint32_t count =
					((opline->extended_value * sizeof(void*)) + (sizeof(zval)-1)) / sizeof(zval);

				do {
					SET_STACK_TYPE(stack, var, IS_UNKNOWN, 1);
					var++;
					count--;
				} while (count);
			}

			if (ssa_op) {
				crex_ssa_range tmp;

				/* Keep information about known types on abstract stack */
				if (ssa_op->result_def >= 0) {
					uint8_t type = IS_UNKNOWN;

					if ((opline->result_type & (IS_SMART_BRANCH_JMPZ|IS_SMART_BRANCH_JMPNZ)) != 0
					 || send_result) {
						/* we didn't set result variable */
						type = IS_UNKNOWN;
					} else if (!(ssa->var_info[ssa_op->result_def].type & MAY_BE_GUARD)
					 && has_concrete_type(ssa->var_info[ssa_op->result_def].type)) {
						type = concrete_type(ssa->var_info[ssa_op->result_def].type);
					} else if (opline->opcode == CREX_QM_ASSIGN) {
						if (opline->op1_type != IS_CONST) {
							/* copy */
							type = STACK_TYPE(stack, EX_VAR_TO_NUM(opline->op1.var));
						}
					} else if (opline->opcode == CREX_ASSIGN) {
						if (opline->op2_type != IS_CONST
						 && ssa_op->op1_use >= 0
						 /* assignment to typed reference may cause conversion */
						 && (ssa->var_info[ssa_op->op1_use].type & MAY_BE_REF) == 0) {
							/* copy */
							type = STACK_TYPE(stack, EX_VAR_TO_NUM(opline->op2.var));
						}
					} else if (opline->opcode == CREX_POST_INC
			         || opline->opcode == CREX_POST_DEC) {
						/* copy */
						type = STACK_TYPE(stack, EX_VAR_TO_NUM(opline->op1.var));
					}
					if (opline->opcode == CREX_JMP_SET
					  || opline->opcode == CREX_COALESCE
					  || opline->opcode == CREX_JMP_NULL) {
						if ((p+1)->op != CREX_JIT_TRACE_VM) {
							SET_STACK_TYPE(stack, EX_VAR_TO_NUM(opline->result.var), IS_UNKNOWN, 1);
						} else if ((p+1)->opline != (opline + 1)) {
							SET_STACK_TYPE(stack, EX_VAR_TO_NUM(opline->result.var), type, 1);
						}
					} else {
						SET_STACK_TYPE(stack, EX_VAR_TO_NUM(opline->result.var), type,
							(gen_handler || type == IS_UNKNOWN || !ra || !ra[ssa_op->result_def]));
						if (ssa->var_info[ssa_op->result_def].type & MAY_BE_INDIRECT) {
							RESET_STACK_MEM_TYPE(stack, EX_VAR_TO_NUM(opline->result.var));
						}
						if (type != IS_UNKNOWN) {
							ssa->var_info[ssa_op->result_def].type &= ~MAY_BE_GUARD;
							if (opline->opcode == CREX_FETCH_THIS
							 && delayed_fetch_this) {
								SET_STACK_REG(stack, EX_VAR_TO_NUM(opline->result.var), ZREG_THIS);
							} else if (ssa->var_info[ssa_op->result_def].avoid_refcounting) {
								SET_STACK_REG(stack, EX_VAR_TO_NUM(opline->result.var), ZREG_ZVAL_TRY_ADDREF);
							} else if (ra && ra[ssa_op->result_def]) {
								SET_STACK_REG_EX(stack, EX_VAR_TO_NUM(opline->result.var), ra[ssa_op->result_def]->reg,
									ra[ssa_op->result_def]->flags & ZREG_STORE);
							}
						}
					}

					if (type == IS_LONG
					 && crex_inference_propagate_range(op_array, ssa, opline, ssa_op, ssa_op->result_def, &tmp)) {
						ssa->var_info[ssa_op->result_def].range.min = tmp.min;
						ssa->var_info[ssa_op->result_def].range.max = tmp.max;
						ssa->var_info[ssa_op->result_def].range.underflow = 0;
						ssa->var_info[ssa_op->result_def].range.overflow = 0;
						ssa->var_info[ssa_op->result_def].has_range = 1;
					}
				}
				if (ssa_op->op1_def >= 0
				 && ((opline->opcode != CREX_QM_ASSIGN && opline->opcode != CREX_CAST)
				  || opline->result_type != IS_CV
				  || opline->result.var != opline->op1.var)) {
					uint8_t type = IS_UNKNOWN;

					if (!(ssa->var_info[ssa_op->op1_def].type & MAY_BE_GUARD)
					 && has_concrete_type(ssa->var_info[ssa_op->op1_def].type)) {
						type = concrete_type(ssa->var_info[ssa_op->op1_def].type);
					} else if (opline->opcode == CREX_ASSIGN) {
						if (!(OP1_INFO() & MAY_BE_REF)
						 || STACK_TYPE(stack, EX_VAR_TO_NUM(opline->op1.var)) != IS_UNKNOWN) {
							if (opline->op2_type != IS_CONST) {
								/* copy */
								type = STACK_TYPE(stack, EX_VAR_TO_NUM(opline->op2.var));
							}
						}
					} else if (opline->opcode == CREX_SEND_VAR
					 || opline->opcode == CREX_CAST
					 || opline->opcode == CREX_QM_ASSIGN
					 || opline->opcode == CREX_JMP_SET
					 || opline->opcode == CREX_COALESCE
					 || opline->opcode == CREX_JMP_NULL
					 || opline->opcode == CREX_FE_RESET_R) {
						/* keep old value */
						type = STACK_TYPE(stack, EX_VAR_TO_NUM(opline->op1.var));
					}
					SET_STACK_TYPE(stack, EX_VAR_TO_NUM(opline->op1.var), type,
						(gen_handler || type == IS_UNKNOWN || !ra ||
							(!ra[ssa_op->op1_def] &&
								!(ssa->vars[ssa_op->op1_def].no_val &&
									C_MODE(OP1_REG_ADDR()) == IS_REG &&
										(opline->opcode == CREX_QM_ASSIGN ||
											opline->opcode == CREX_SEND_VAR ||
											opline->opcode == CREX_SEND_VAR_EX ||
											opline->opcode == CREX_SEND_VAR_NO_REF ||
											opline->opcode == CREX_SEND_VAR_NO_REF_EX ||
											opline->opcode == CREX_SEND_FUNC_ARG)))));
					if (type != IS_UNKNOWN) {
						ssa->var_info[ssa_op->op1_def].type &= ~MAY_BE_GUARD;
						if (ra && ra[ssa_op->op1_def]) {
							uint8_t flags = ra[ssa_op->op1_def]->flags & ZREG_STORE;

							if (ssa_op->op1_use >= 0) {
								if (opline->opcode == CREX_SEND_VAR
								 || opline->opcode == CREX_CAST
								 || opline->opcode == CREX_QM_ASSIGN
								 || opline->opcode == CREX_JMP_SET
								 || opline->opcode == CREX_COALESCE
								 || opline->opcode == CREX_JMP_NULL
								 || opline->opcode == CREX_FE_RESET_R) {
									if (!ra[ssa_op->op1_use]) {
										flags |= ZREG_LOAD;
									}
								}
							}
							SET_STACK_REG_EX(stack, EX_VAR_TO_NUM(opline->op1.var), ra[ssa_op->op1_def]->reg, flags);
						}
					}
					if (type == IS_LONG
					 && crex_inference_propagate_range(op_array, ssa, opline, ssa_op, ssa_op->op1_def, &tmp)) {
						ssa->var_info[ssa_op->op1_def].range.min = tmp.min;
						ssa->var_info[ssa_op->op1_def].range.max = tmp.max;
						ssa->var_info[ssa_op->op1_def].range.underflow = 0;
						ssa->var_info[ssa_op->op1_def].range.overflow = 0;
						ssa->var_info[ssa_op->op1_def].has_range = 1;
					}
				}
				if (ssa_op->op2_def >= 0
				 && (opline->opcode != CREX_ASSIGN
				  || opline->op1_type != IS_CV
				  || opline->op1.var != opline->op2.var)) {
					uint8_t type = IS_UNKNOWN;

					if (!(ssa->var_info[ssa_op->op2_def].type & MAY_BE_GUARD)
					 && has_concrete_type(ssa->var_info[ssa_op->op2_def].type)) {
						type = concrete_type(ssa->var_info[ssa_op->op2_def].type);
					} else if (opline->opcode == CREX_ASSIGN) {
						/* keep old value */
						type = STACK_TYPE(stack, EX_VAR_TO_NUM(opline->op2.var));
					}
					SET_STACK_TYPE(stack, EX_VAR_TO_NUM(opline->op2.var), type,
						(gen_handler || type == IS_UNKNOWN || !ra ||
							(!ra[ssa_op->op2_def] &&
								!(ssa->vars[ssa_op->op2_def].no_val &&
									C_MODE(OP2_REG_ADDR()) == IS_REG &&
									opline->opcode == CREX_ASSIGN))));
					if (type != IS_UNKNOWN) {
						ssa->var_info[ssa_op->op2_def].type &= ~MAY_BE_GUARD;
						if (ra && ra[ssa_op->op2_def]) {
							uint8_t flags = ra[ssa_op->op2_def]->flags & ZREG_STORE;

							if (ssa_op->op2_use >= 0) {
								if (opline->opcode == CREX_ASSIGN) {
									if (!ra[ssa_op->op2_use]
									 || ra[ssa_op->op2_use]->reg != ra[ssa_op->op2_def]->reg) {
										flags |= ZREG_LOAD;
									}
								}
							}
							SET_STACK_REG_EX(stack, EX_VAR_TO_NUM(opline->op2.var), ra[ssa_op->op2_def]->reg, flags);
						}
					}
					if (type == IS_LONG
					 && crex_inference_propagate_range(op_array, ssa, opline, ssa_op, ssa_op->op2_def, &tmp)) {
						ssa->var_info[ssa_op->op2_def].range.min = tmp.min;
						ssa->var_info[ssa_op->op2_def].range.max = tmp.max;
						ssa->var_info[ssa_op->op2_def].range.underflow = 0;
						ssa->var_info[ssa_op->op2_def].range.overflow = 0;
						ssa->var_info[ssa_op->op2_def].has_range = 1;
					}
				}

				switch (opline->opcode) {
					case CREX_ASSIGN_DIM:
					case CREX_ASSIGN_OBJ:
					case CREX_ASSIGN_STATIC_PROP:
					case CREX_ASSIGN_DIM_OP:
					case CREX_ASSIGN_OBJ_OP:
					case CREX_ASSIGN_STATIC_PROP_OP:
					case CREX_ASSIGN_OBJ_REF:
					case CREX_ASSIGN_STATIC_PROP_REF:
						/* OP_DATA */
						ssa_op++;
						opline++;
						if (ssa_op->op1_def >= 0) {
							uint8_t type = IS_UNKNOWN;

							if (!(ssa->var_info[ssa_op->op1_def].type & MAY_BE_GUARD)
							 && has_concrete_type(ssa->var_info[ssa_op->op1_def].type)) {
								type = concrete_type(ssa->var_info[ssa_op->op1_def].type);
							} else if ((opline-1)->opcode == CREX_ASSIGN_DIM
							 || (opline-1)->opcode == CREX_ASSIGN_OBJ
							 || (opline-1)->opcode == CREX_ASSIGN_STATIC_PROP) {
								/* keep old value */
								type = STACK_TYPE(stack, EX_VAR_TO_NUM(opline->op1.var));
							}
							SET_STACK_TYPE(stack, EX_VAR_TO_NUM(opline->op1.var), type,
								(gen_handler || type == IS_UNKNOWN || !ra || !ra[ssa_op->op1_def]));
							if (type != IS_UNKNOWN) {
								ssa->var_info[ssa_op->op1_def].type &= ~MAY_BE_GUARD;
								if (ra && ra[ssa_op->op1_def]) {
									SET_STACK_REG_EX(stack, EX_VAR_TO_NUM(opline->op1.var), ra[ssa_op->op1_def]->reg,
										ra[ssa_op->op1_def]->flags & ZREG_STORE);
								}
							}
							if (type == IS_LONG
							 && crex_inference_propagate_range(op_array, ssa, opline, ssa_op, ssa_op->op1_def, &tmp)) {
								ssa->var_info[ssa_op->op1_def].range.min = tmp.min;
								ssa->var_info[ssa_op->op1_def].range.max = tmp.max;
								ssa->var_info[ssa_op->op1_def].range.underflow = 0;
								ssa->var_info[ssa_op->op1_def].range.overflow = 0;
								ssa->var_info[ssa_op->op1_def].has_range = 1;
							}
						}
						ssa_op++;
						break;
					case CREX_RECV_INIT:
					    ssa_op++;
						opline++;
						while (opline->opcode == CREX_RECV_INIT) {
							if (ssa_op->result_def >= 0) {
								uint8_t type = IS_UNKNOWN;

								if (!(ssa->var_info[ssa_op->result_def].type & MAY_BE_GUARD)
								 && has_concrete_type(ssa->var_info[ssa_op->result_def].type)) {
									type = concrete_type(ssa->var_info[ssa_op->result_def].type);
								}
								SET_STACK_TYPE(stack, EX_VAR_TO_NUM(opline->result.var), type,
									(gen_handler || !ra || !ra[ssa_op->result_def]));
								if (ra && ra[ssa_op->result_def]) {
									SET_STACK_REG_EX(stack, EX_VAR_TO_NUM(opline->result.var), ra[ssa_op->result_def]->reg,
										ra[ssa_op->result_def]->flags & ZREG_STORE);
								}
							}
							ssa_op++;
							opline++;
						}
						break;
					case CREX_BIND_GLOBAL:
						ssa_op++;
						opline++;
						while (opline->opcode == CREX_BIND_GLOBAL) {
							if (ssa_op->op1_def >= 0) {
								uint8_t type = IS_UNKNOWN;

								if (!(ssa->var_info[ssa_op->op1_def].type & MAY_BE_GUARD)
								 && has_concrete_type(ssa->var_info[ssa_op->op1_def].type)) {
									type = concrete_type(ssa->var_info[ssa_op->op1_def].type);
								}
								SET_STACK_TYPE(stack, EX_VAR_TO_NUM(opline->op1.var), type,
									(gen_handler || !ra || !ra[ssa_op->op1_def]));
								if (ra && ra[ssa_op->op1_def]) {
									SET_STACK_REG_EX(stack, EX_VAR_TO_NUM(opline->op1.var), ra[ssa_op->op1_def]->reg,
										ra[ssa_op->op1_def]->flags & ZREG_STORE);
								}
							}
							ssa_op++;
							opline++;
						}
						break;
					default:
						ssa_op += crex_jit_trace_op_len(opline);
						break;
				}

				if (send_result) {
					ssa_op++;
					p++;
					if ((p+1)->op == CREX_JIT_TRACE_OP1_TYPE) {
						p++;
					}
					send_result = 0;
				}
			}
		} else if (p->op == CREX_JIT_TRACE_ENTER) {
			call = frame->call;
			assert(call && &call->func->op_array == p->op_array);

			if (opline->opcode == CREX_DO_UCALL
			 || opline->opcode == CREX_DO_FCALL_BY_NAME
			 || opline->opcode == CREX_DO_FCALL) {

				frame->call_opline = opline;

				/* Check if SEND_UNPACK/SEND_ARRAY may cause enter at different opline */
				if (opline > op_array->opcodes) {
					const crex_op *prev_opline = opline - 1;

					while (prev_opline->opcode == CREX_EXT_FCALL_BEGIN || prev_opline->opcode == CREX_TICKS) {
						prev_opline--;
					}
					JIT_G(current_frame) = call;
					if ((prev_opline->opcode == CREX_SEND_ARRAY
					  || prev_opline->opcode == CREX_SEND_UNPACK
					  || prev_opline->opcode == CREX_CHECK_UNDEF_ARGS)
					 && p->op_array->num_args
					 && (p->op_array->fn_flags & CREX_ACC_HAS_TYPE_HINTS) == 0
					 && ((p+1)->op == CREX_JIT_TRACE_VM
					  || (p+1)->op == CREX_JIT_TRACE_END)
					 && (TRACE_FRAME_NUM_ARGS(call) < 0
					  || TRACE_FRAME_NUM_ARGS(call) < p->op_array->num_args)
					 && !crex_jit_trace_opline_guard(&dasm_state, (p+1)->opline)) {
						goto jit_failure;
					}
					JIT_G(current_frame) = frame;
				}
			}

			if ((p+1)->op == CREX_JIT_TRACE_END) {
				p++;
				break;
			}
			if (op_array->fn_flags & CREX_ACC_CLOSURE) {
				if (TRACE_FRAME_IS_THIS_CHECKED(frame)) {
					TRACE_FRAME_SET_THIS_CHECKED(call);
				}
			} else if (op_array->scope && !(op_array->fn_flags & CREX_ACC_STATIC)) {
				TRACE_FRAME_SET_THIS_CHECKED(call);
			}
			op_array = (crex_op_array*)p->op_array;
			jit_extension =
				(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
			op_array_ssa = &jit_extension->func_info.ssa;
			frame->call = call->prev;
			call->prev = frame;
			if (p->info & CREX_JIT_TRACE_RETURN_VALUE_USED) {
				TRACE_FRAME_SET_RETURN_VALUE_USED(call);
			} else {
				TRACE_FRAME_SET_RETURN_VALUE_UNUSED(call);
			}
			JIT_G(current_frame) = frame = call;
			stack = frame->stack;
			if (ra) {
				int j = CREX_JIT_TRACE_GET_FIRST_SSA_VAR(p->info);

				for (i = 0; i < op_array->last_var; i++,j++) {
					if (ra[j] && (ra[j]->flags & ZREG_LOAD) != 0) {
						if ((ssa->var_info[j].type & MAY_BE_GUARD) != 0) {
							uint8_t op_type;

							ssa->var_info[j].type &= ~MAY_BE_GUARD;
							op_type = concrete_type(ssa->var_info[j].type);
							if (!crex_jit_type_guard(&dasm_state, opline, EX_NUM_TO_VAR(i), op_type)) {
								goto jit_failure;
							}
							SET_STACK_TYPE(stack, i, op_type, 1);
						}
						SET_STACK_REG_EX(stack, i, ra[j]->reg, ZREG_LOAD);
						if (!crex_jit_load_var(&dasm_state, ssa->var_info[j].type, i, ra[j]->reg)) {
							goto jit_failure;
						}
					}
				}
			}
		} else if (p->op == CREX_JIT_TRACE_BACK) {
			op_array = (crex_op_array*)p->op_array;
			jit_extension =
				(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
			op_array_ssa = &jit_extension->func_info.ssa;
			top = frame;
			if (frame->prev) {
				checked_stack = frame->old_checked_stack;
				peek_checked_stack = frame->old_peek_checked_stack;
				frame = frame->prev;
				stack = frame->stack;
				CREX_ASSERT(&frame->func->op_array == op_array);
			} else {
				frame = crex_jit_trace_ret_frame(frame, op_array);
				TRACE_FRAME_INIT(frame, op_array, TRACE_FRAME_MASK_UNKNOWN_RETURN, -1);
				frame->used_stack = checked_stack = peek_checked_stack = 0;
				stack = frame->stack;
				if (JIT_G(opt_level) >= CREX_JIT_LEVEL_INLINE) {
					uint32_t j = CREX_JIT_TRACE_GET_FIRST_SSA_VAR(p->info);

					for (i = 0; i < op_array->last_var + op_array->T; i++, j++) {
						/* Initialize abstract stack using SSA */
						if (!(ssa->var_info[j].type & MAY_BE_GUARD)
						 && has_concrete_type(ssa->var_info[j].type)) {
							SET_STACK_TYPE(stack, i, concrete_type(ssa->var_info[j].type), 1);
						} else {
							SET_STACK_TYPE(stack, i, IS_UNKNOWN, 1);
						}
					}
					if (ra) {
						j = CREX_JIT_TRACE_GET_FIRST_SSA_VAR(p->info);
						for (i = 0; i < op_array->last_var + op_array->T; i++, j++) {
							if (ra[j] && (ra[j]->flags & ZREG_LOAD) != 0) {
								SET_STACK_REG_EX(stack, i, ra[j]->reg, ZREG_LOAD);
								if (!crex_jit_load_var(&dasm_state, ssa->var_info[j].type, i, ra[j]->reg)) {
									goto jit_failure;
								}
							}
						}
					}
				} else {
					for (i = 0; i < op_array->last_var + op_array->T; i++) {
						SET_STACK_TYPE(stack, i, IS_UNKNOWN, 1);
					}
				}
				opline = NULL;
			}
			JIT_G(current_frame) = frame;
			if (res_type != IS_UNKNOWN
			 && (p+1)->op == CREX_JIT_TRACE_VM) {
				const crex_op *opline = (p+1)->opline - 1;
				if (opline->result_type != IS_UNUSED) {
				    SET_STACK_TYPE(stack, EX_VAR_TO_NUM(opline->result.var), res_type, 1);
				}
			}
			res_type = IS_UNKNOWN;
		} else if (p->op == CREX_JIT_TRACE_END) {
			break;
		} else if (p->op == CREX_JIT_TRACE_INIT_CALL) {
			const crex_op *init_opline = crex_jit_trace_find_init_fcall_op(p, op_array);
			int num_args = -1;

			if (init_opline
			 && init_opline->extended_value <= TRACE_FRAME_MAX_NUM_ARGS) {
				num_args = init_opline->extended_value;
			}

			call = top;
			TRACE_FRAME_INIT(call, p->func, frame_flags, num_args);
			call->prev = frame->call;
			if (!(p->info & CREX_JIT_TRACE_FAKE_INIT_CALL)) {
				TRACE_FRAME_SET_LAST_SEND_BY_VAL(call);
				if (init_opline && init_opline->opcode == CREX_INIT_DYNAMIC_CALL) {
					TRACE_FRAME_SET_CLOSURE_CALL(call);
				}
			}
			if (init_opline) {
				if (init_opline->opcode != CREX_NEW
				 && (init_opline->opcode != CREX_INIT_METHOD_CALL
				  || init_opline->op1_type == IS_UNDEF
				  || (!(p->info & CREX_JIT_TRACE_FAKE_INIT_CALL)
				   && ssa_op
				   && (ssa_op-1)->op1_use >=0
				   && ssa->var_info[(ssa_op-1)->op1_use].delayed_fetch_this))
				 && (init_opline->opcode != CREX_INIT_USER_CALL
				  || (p->func && (!p->func->common.scope || (p->func->common.fn_flags & CREX_ACC_STATIC))))
				 && (init_opline->opcode != CREX_INIT_DYNAMIC_CALL
				  || (p->func && (!p->func->common.scope || (p->func->common.fn_flags & CREX_ACC_STATIC))))
				) {
					TRACE_FRAME_SET_NO_NEED_RELEASE_THIS(call);
				} else if (init_opline->opcode == CREX_NEW
				 || (init_opline->opcode == CREX_INIT_METHOD_CALL
				  && init_opline->op1_type != IS_UNDEF
				  && !(p->info & CREX_JIT_TRACE_FAKE_INIT_CALL)
				  && p->func && p->func->common.scope && !(p->func->common.fn_flags & CREX_ACC_STATIC))) {
					TRACE_FRAME_SET_ALWAYS_RELEASE_THIS(call);
				}
			}
			frame->call = call;
			top = crex_jit_trace_call_frame(top, p->op_array);
			if (p->func) {
				if (p->func->type == CREX_USER_FUNCTION) {
					if (JIT_G(opt_level) >= CREX_JIT_LEVEL_INLINE) {
						crex_jit_op_array_trace_extension *jit_extension =
							(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(p->op_array);

						i = 0;
						while (i < p->op_array->num_args) {
							/* Types of arguments are going to be stored in abstract stack when processing SEV instruction */
							SET_STACK_TYPE(call->stack, i, IS_UNKNOWN, 1);
							i++;
						}
						while (i < p->op_array->last_var) {
							if (jit_extension
							 && crex_jit_var_may_alias(p->op_array, &jit_extension->func_info.ssa, i) != NO_ALIAS) {
								SET_STACK_TYPE(call->stack, i, IS_UNKNOWN, 1);
							} else {
								SET_STACK_TYPE(call->stack, i, IS_UNDEF, 1);
							}
							i++;
						}
						while (i < p->op_array->last_var + p->op_array->T) {
							SET_STACK_TYPE(call->stack, i, IS_UNKNOWN, 1);
							i++;
						}
					} else {
						for (i = 0; i < p->op_array->last_var + p->op_array->T; i++) {
							SET_STACK_TYPE(call->stack, i, IS_UNKNOWN, 1);
						}
					}
				} else {
					CREX_ASSERT(p->func->type == CREX_INTERNAL_FUNCTION);
					for (i = 0; i < p->op_array->num_args; i++) {
						SET_STACK_TYPE(call->stack, i, IS_UNKNOWN, 1);
					}
				}
				if (p->info & CREX_JIT_TRACE_FAKE_INIT_CALL) {
					int skip_guard = 0;

					if (init_opline) {
						crex_call_info *call_info = jit_extension->func_info.callee_info;

						while (call_info) {
							if (call_info->caller_init_opline == init_opline
									&& !call_info->is_prototype) {
								if (op_array->fn_flags & CREX_ACC_TRAIT_CLONE) {
									if (init_opline->opcode == CREX_INIT_STATIC_METHOD_CALL
									 && init_opline->op1_type != IS_CONST) {
										break;
									} else if (init_opline->opcode == CREX_INIT_METHOD_CALL) {
										break;
									}
								}
								skip_guard = 1;
								break;
							}
							call_info = call_info->next_callee;
						}
						if (!skip_guard
						 && !crex_jit_may_be_polymorphic_call(init_opline)
						 && !crex_jit_may_be_modified(p->func, op_array)) {
							skip_guard = 1;
						}
					}

					if (!skip_guard) {
						if (!opline) {
							crex_jit_trace_rec *q = p + 1;
							while (q->op != CREX_JIT_TRACE_VM && q->op != CREX_JIT_TRACE_END) {
								q++;
							}
							opline = q->opline;
							CREX_ASSERT(opline != NULL);
						}
						if (!crex_jit_init_fcall_guard(&dasm_state,
								CREX_JIT_TRACE_FAKE_LEVEL(p->info), p->func, opline)) {
							goto jit_failure;
						}
					}
				}
			}
			call->old_checked_stack = checked_stack;
			call->old_peek_checked_stack = peek_checked_stack;
			if (p->info & CREX_JIT_TRACE_FAKE_INIT_CALL) {
				frame->call_level++;
				call->used_stack = checked_stack = peek_checked_stack = 0;
			} else {
				if (p->func) {
					call->used_stack = crex_vm_calc_used_stack(init_opline->extended_value, (crex_function*)p->func);
				} else {
					call->used_stack = (CREX_CALL_FRAME_SLOT + init_opline->extended_value) * sizeof(zval);
				}
				switch (init_opline->opcode) {
					case CREX_INIT_FCALL:
					case CREX_INIT_FCALL_BY_NAME:
					case CREX_INIT_NS_FCALL_BY_NAME:
					case CREX_INIT_METHOD_CALL:
					case CREX_INIT_DYNAMIC_CALL:
					//case CREX_INIT_STATIC_METHOD_CALL:
					//case CREX_INIT_USER_CALL:
					//case CREX_NEW:
						checked_stack += call->used_stack;
						if (checked_stack > peek_checked_stack) {
							peek_checked_stack = checked_stack;
						}
						break;
					default:
						checked_stack = peek_checked_stack = 0;
				}
			}
		} else if (p->op == CREX_JIT_TRACE_DO_ICALL) {
			call = frame->call;
			if (call) {
				checked_stack = call->old_checked_stack;
				peek_checked_stack = call->old_peek_checked_stack;
				top = call;
				frame->call = call->prev;
			}
		} else {
			CREX_UNREACHABLE();
		}
	}

	CREX_ASSERT(p->op == CREX_JIT_TRACE_END);

	t = &crex_jit_traces[CREX_JIT_TRACE_NUM];

	if (!parent_trace && crex_jit_trace_uses_initial_ip()) {
		t->flags |= CREX_JIT_TRACE_USES_INITIAL_IP;
	}

	if (p->stop == CREX_JIT_TRACE_STOP_LOOP
	 || p->stop == CREX_JIT_TRACE_STOP_RECURSIVE_CALL
	 || p->stop == CREX_JIT_TRACE_STOP_RECURSIVE_RET) {
		if (ra) {
			crex_ssa_phi *phi = ssa->blocks[1].phis;

			while (phi) {
				if (ra[phi->ssa_var]
				 && ra[phi->sources[1]]
				 && STACK_MEM_TYPE(stack, phi->var) != STACK_TYPE(stack, phi->var)
				 && (ra[phi->ssa_var]->flags & (ZREG_LOAD|ZREG_STORE)) == 0
				 && (ra[phi->sources[1]]->flags & (ZREG_LOAD|ZREG_STORE)) == 0) {
					/* Store actual type to memory to avoid deoptimization mistakes */
					/* TODO: Alternatively, we may try to update alredy generated deoptimization info */
					crex_jit_store_var_type(&dasm_state, phi->var, STACK_TYPE(stack, phi->var));
				}
				phi = phi->next;
			}
		}
		if (p->stop != CREX_JIT_TRACE_STOP_RECURSIVE_RET) {
			if ((t->flags & CREX_JIT_TRACE_USES_INITIAL_IP)
			 && !crex_jit_set_ip(&dasm_state, p->opline)) {
				goto jit_failure;
			}
		}
		t->link = CREX_JIT_TRACE_NUM;
		if (p->stop != CREX_JIT_TRACE_STOP_RECURSIVE_RET) {
			t->flags |= CREX_JIT_TRACE_CHECK_INTERRUPT;
		}
		if (!(t->flags & CREX_JIT_TRACE_LOOP)) {
			const void *timeout_exit_addr = NULL;

			t->flags |= CREX_JIT_TRACE_LOOP;

			if (trace_buffer->stop != CREX_JIT_TRACE_STOP_RECURSIVE_RET) {
				if (!(t->flags & CREX_JIT_TRACE_USES_INITIAL_IP)
				 || (ra
				  && crex_jit_trace_stack_needs_deoptimization(stack, op_array->last_var + op_array->T))) {
					/* Deoptimize to the first instruction of the loop */
					uint32_t exit_point = crex_jit_trace_get_exit_point(trace_buffer[1].opline, CREX_JIT_EXIT_TO_VM);

					timeout_exit_addr = crex_jit_trace_get_exit_addr(exit_point);
					if (!timeout_exit_addr) {
						goto jit_failure;
					}
				} else {
					timeout_exit_addr = dasm_labels[crex_lbinterrupt_handler];
				}
			}

			crex_jit_trace_end_loop(&dasm_state, 0, timeout_exit_addr); /* jump back to start of the trace loop */
		}
	} else if (p->stop == CREX_JIT_TRACE_STOP_LINK
	        || p->stop == CREX_JIT_TRACE_STOP_INTERPRETER) {
		if (ra
		 && (p-1)->op != CREX_JIT_TRACE_ENTER
		 && (p-1)->op != CREX_JIT_TRACE_BACK
		 && opline->opcode != CREX_DO_UCALL
		 && opline->opcode != CREX_DO_FCALL
		 && opline->opcode != CREX_DO_FCALL_BY_NAME
		 && opline->opcode != CREX_INCLUDE_OR_EVAL) {
			if (!crex_jit_trace_deoptimization(&dasm_state, 0, NULL,
					stack, op_array->last_var + op_array->T, NULL, NULL, NULL, 0)) {
				goto jit_failure;
			}
			for (i = 0; i < op_array->last_var; i++) {
				int8_t reg = STACK_REG(stack, i);
				uint8_t type = STACK_TYPE(stack, i);

				if (reg == ZREG_NONE
				 && type != IS_UNKNOWN
				 && type != STACK_MEM_TYPE(stack, i)) {
					if (!crex_jit_store_var_type(&dasm_state, i, type)) {
						return 0;
					}
					SET_STACK_TYPE(stack, i, type, 1);
				}
			}
		}
		if (p->stop == CREX_JIT_TRACE_STOP_LINK) {
			const void *timeout_exit_addr = NULL;

			t->link = crex_jit_find_trace(p->opline->handler);
			if (t->link == 0) {
				/* this can happen if CREX_JIT_EXIT_INVALIDATE was handled
				 * by crex_jit_trace_exit() in another thread after this
				 * thread set CREX_JIT_TRACE_STOP_LINK in crex_jit_trace_execute();
				 * CREX_JIT_EXIT_INVALIDATE resets the opline handler to one of
				 * the "_counter_handler" functions, and these are not registered
				 * tracer functions */
				goto jit_failure;
			}
			if ((crex_jit_traces[t->link].flags & CREX_JIT_TRACE_USES_INITIAL_IP)
			 && !crex_jit_set_ip(&dasm_state, p->opline)) {
				goto jit_failure;
			}
			if (!parent_trace && crex_jit_trace_uses_initial_ip()) {
				t->flags |= CREX_JIT_TRACE_USES_INITIAL_IP;
			}
			if (parent_trace
			 && (crex_jit_traces[t->link].flags & CREX_JIT_TRACE_CHECK_INTERRUPT)
			 && crex_jit_traces[parent_trace].root == t->link) {
				if (!(crex_jit_traces[t->link].flags & CREX_JIT_TRACE_USES_INITIAL_IP)) {
					uint32_t exit_point;

					for (i = 0; i < op_array->last_var + op_array->T; i++) {
						SET_STACK_TYPE(stack, i, IS_UNKNOWN, 1);
					}
					exit_point = crex_jit_trace_get_exit_point(opline, CREX_JIT_EXIT_TO_VM);
					timeout_exit_addr = crex_jit_trace_get_exit_addr(exit_point);
					if (!timeout_exit_addr) {
						goto jit_failure;
					}
				} else {
					timeout_exit_addr = dasm_labels[crex_lbinterrupt_handler];
				}
			}
			crex_jit_trace_link_to_root(&dasm_state, &crex_jit_traces[t->link], timeout_exit_addr);
		} else {
			crex_jit_trace_return(&dasm_state, 0, NULL);
		}
	} else if (p->stop == CREX_JIT_TRACE_STOP_RETURN) {
		crex_jit_trace_return(&dasm_state, 0, NULL);
	} else {
		// TODO: not implemented ???
		CREX_ASSERT(0 && p->stop);
	}

	if (CREX_JIT_EXIT_COUNTERS + t->exit_count >= JIT_G(max_exit_counters)) {
		goto jit_failure;
	}

	if (!crex_jit_trace_end(&dasm_state, t)) {
		goto jit_failure;
	}

	handler = dasm_link_and_encode(&dasm_state, NULL, NULL, NULL, NULL, ZSTR_VAL(name), CREX_JIT_TRACE_NUM,
		parent_trace ? SP_ADJ_JIT : ((crex_jit_vm_kind == CREX_VM_KIND_HYBRID) ? SP_ADJ_VM : SP_ADJ_RET),
		parent_trace ? SP_ADJ_NONE : SP_ADJ_JIT);

	if (handler) {
		if (p->stop == CREX_JIT_TRACE_STOP_RECURSIVE_CALL) {
			const crex_op_array *rec_op_array;

			rec_op_array = op_array = trace_buffer->op_array;
			jit_extension =
				(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
			p = trace_buffer + CREX_JIT_TRACE_START_REC_SIZE;
			for (;;p++) {
				if (p->op == CREX_JIT_TRACE_VM) {
					opline = p->opline;
				} else if (p->op == CREX_JIT_TRACE_ENTER) {
					if (p->op_array == rec_op_array) {
						crex_jit_trace_setup_ret_counter(opline, jit_extension->offset);
					}
					op_array = p->op_array;
					jit_extension =
						(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
				} else if (p->op == CREX_JIT_TRACE_BACK) {
					op_array = p->op_array;
					jit_extension =
						(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
				} else if (p->op == CREX_JIT_TRACE_END) {
					break;
				}
			}
		} else if (p->stop == CREX_JIT_TRACE_STOP_LINK
		        || p->stop == CREX_JIT_TRACE_STOP_INTERPRETER) {
			if (opline
			 && (opline->opcode == CREX_DO_UCALL
			  || opline->opcode == CREX_DO_FCALL
			  || opline->opcode == CREX_DO_FCALL_BY_NAME
			  || opline->opcode == CREX_YIELD
			  || opline->opcode == CREX_YIELD_FROM
			  || opline->opcode == CREX_INCLUDE_OR_EVAL)) {
				crex_jit_trace_setup_ret_counter(opline, jit_extension->offset);
			}
			if (JIT_G(current_frame)
			 && JIT_G(current_frame)->prev) {
				frame = JIT_G(current_frame)->prev;
				do {
					if (frame->call_opline) {
						op_array = &frame->func->op_array;
						jit_extension =
							(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
						crex_jit_trace_setup_ret_counter(frame->call_opline, jit_extension->offset);
					}
					frame = frame->prev;
				} while (frame);
			}
		}
	}

jit_failure:
	dasm_free(&dasm_state);

	if (name) {
		crex_string_release(name);
	}

jit_cleanup:
	/* Clean up used op_arrays */
	while (num_op_arrays > 0) {
		op_array = op_arrays[--num_op_arrays];
		jit_extension =
			(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);

	    jit_extension->func_info.num = 0;
		jit_extension->func_info.flags &= CREX_FUNC_JIT_ON_FIRST_EXEC
			| CREX_FUNC_JIT_ON_PROF_REQUEST
			| CREX_FUNC_JIT_ON_HOT_COUNTERS
			| CREX_FUNC_JIT_ON_HOT_TRACE;
		memset(&jit_extension->func_info.ssa, 0, sizeof(crex_func_info) - offsetof(crex_func_info, ssa));
	}

	crex_arena_release(&CG(arena), checkpoint);

	JIT_G(current_frame) = NULL;
	JIT_G(current_trace) = NULL;

	return handler;
}

static const void *crex_jit_trace_exit_to_vm(uint32_t trace_num, uint32_t exit_num)
{
	const void *handler = NULL;
	dasm_State* dasm_state = NULL;
	void *checkpoint;
	char name[32];
	const crex_op *opline;
	uint32_t stack_size;
	crex_jit_trace_stack *stack;
	bool original_handler = 0;

	if (!crex_jit_trace_exit_needs_deoptimization(trace_num, exit_num)) {
		return dasm_labels[crex_lbtrace_escape];
	}

	checkpoint = crex_arena_checkpoint(CG(arena));;

	sprintf(name, "ESCAPE-%d-%d", trace_num, exit_num);

	dasm_init(&dasm_state, DASM_MAXSECTION);
	dasm_setupglobal(&dasm_state, dasm_labels, crex_lb_MAX);
	dasm_setup(&dasm_state, dasm_actions);

	crex_jit_align_func(&dasm_state);

	/* Deoptimization */
	stack_size = crex_jit_traces[trace_num].exit_info[exit_num].stack_size;
	stack = crex_jit_traces[trace_num].stack_map + crex_jit_traces[trace_num].exit_info[exit_num].stack_offset;

	if (!crex_jit_trace_deoptimization(&dasm_state,
			crex_jit_traces[trace_num].exit_info[exit_num].flags,
			crex_jit_traces[trace_num].exit_info[exit_num].opline,
			stack, stack_size, NULL, NULL, NULL, 0)) {
		goto jit_failure;
	}

	opline = crex_jit_traces[trace_num].exit_info[exit_num].opline;
	if (opline) {
		if (opline == crex_jit_traces[crex_jit_traces[trace_num].root].opline) {
			/* prevent endless loop */
			original_handler = 1;
		}
		crex_jit_set_ip_ex(&dasm_state, opline, original_handler);
	}

	crex_jit_trace_return(&dasm_state, original_handler, opline);

	handler = dasm_link_and_encode(&dasm_state, NULL, NULL, NULL, NULL, name, CREX_JIT_TRACE_NUM, SP_ADJ_JIT, SP_ADJ_NONE);

jit_failure:
	dasm_free(&dasm_state);
	crex_arena_release(&CG(arena), checkpoint);
	return handler;
}

static crex_jit_trace_stop crex_jit_compile_root_trace(crex_jit_trace_rec *trace_buffer, const crex_op *opline, size_t offset)
{
	crex_jit_trace_stop ret;
	const void *handler;
	uint8_t orig_trigger;
	crex_jit_trace_info *t = NULL;
	crex_jit_trace_exit_info exit_info[CREX_JIT_TRACE_MAX_EXITS];
	bool do_bailout = 0;

	crex_shared_alloc_lock();

	/* Checks under lock */
	if ((CREX_OP_TRACE_INFO(opline, offset)->trace_flags & CREX_JIT_TRACE_JITED)) {
		ret = CREX_JIT_TRACE_STOP_ALREADY_DONE;
	} else if (CREX_JIT_TRACE_NUM >= JIT_G(max_root_traces)) {
		ret = CREX_JIT_TRACE_STOP_TOO_MANY_TRACES;
	} else {
		crex_try {
			SHM_UNPROTECT();
			crex_jit_unprotect();

			t = &crex_jit_traces[CREX_JIT_TRACE_NUM];

			t->id = CREX_JIT_TRACE_NUM;
			t->root = CREX_JIT_TRACE_NUM;
			t->parent = 0;
			t->link = 0;
			t->exit_count = 0;
			t->child_count = 0;
			t->stack_map_size = 0;
			t->flags = 0;
			t->polymorphism = 0;
			t->jmp_table_size = 0;
			t->op_array = trace_buffer[0].op_array;
			t->opline = trace_buffer[1].opline;
			t->exit_info = exit_info;
			t->stack_map = NULL;

			orig_trigger = JIT_G(trigger);
			JIT_G(trigger) = CREX_JIT_ON_HOT_TRACE;

			handler = crex_jit_trace(trace_buffer, 0, 0);

			JIT_G(trigger) = orig_trigger;

			if (handler) {
				crex_jit_trace_exit_info *shared_exit_info = NULL;

				t->exit_info = NULL;
				if (t->exit_count) {
					/* reallocate exit_info into shared memory */
					shared_exit_info = (crex_jit_trace_exit_info*)crex_shared_alloc(
						sizeof(crex_jit_trace_exit_info) * t->exit_count);

					if (!shared_exit_info) {
					    if (t->stack_map) {
							efree(t->stack_map);
							t->stack_map = NULL;
						}
						ret = CREX_JIT_TRACE_STOP_NO_SHM;
						goto exit;
					}
					memcpy(shared_exit_info, exit_info,
						sizeof(crex_jit_trace_exit_info) * t->exit_count);
					t->exit_info = shared_exit_info;
				}

			    if (t->stack_map_size) {
					crex_jit_trace_stack *shared_stack_map = (crex_jit_trace_stack*)crex_shared_alloc(t->stack_map_size * sizeof(crex_jit_trace_stack));
					if (!shared_stack_map) {
						efree(t->stack_map);
						ret = CREX_JIT_TRACE_STOP_NO_SHM;
						goto exit;
					}
					memcpy(shared_stack_map, t->stack_map, t->stack_map_size * sizeof(crex_jit_trace_stack));
					efree(t->stack_map);
					t->stack_map = shared_stack_map;
			    }

				t->exit_counters = CREX_JIT_EXIT_COUNTERS;
				CREX_JIT_EXIT_COUNTERS += t->exit_count;

				((crex_op*)opline)->handler = handler;

				CREX_JIT_TRACE_NUM++;
				CREX_OP_TRACE_INFO(opline, offset)->trace_flags |= CREX_JIT_TRACE_JITED;

				ret = CREX_JIT_TRACE_STOP_COMPILED;
			} else if (t->exit_count >= CREX_JIT_TRACE_MAX_EXITS ||
			           CREX_JIT_EXIT_COUNTERS + t->exit_count >= JIT_G(max_exit_counters)) {
			    if (t->stack_map) {
					efree(t->stack_map);
					t->stack_map = NULL;
				}
				ret = CREX_JIT_TRACE_STOP_TOO_MANY_EXITS;
			} else {
			    if (t->stack_map) {
					efree(t->stack_map);
					t->stack_map = NULL;
				}
				ret = CREX_JIT_TRACE_STOP_COMPILER_ERROR;
			}

exit:;
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

	if ((JIT_G(debug) & CREX_JIT_DEBUG_TRACE_EXIT_INFO) != 0
	 && ret == CREX_JIT_TRACE_STOP_COMPILED
	 && t->exit_count > 0) {
		crex_jit_dump_exit_info(t);
	}

	return ret;
}

/* Set counting handler back to original VM handler. */
static void crex_jit_stop_hot_trace_counters(crex_op_array *op_array)
{
	crex_jit_op_array_trace_extension *jit_extension;
	uint32_t i;

	jit_extension = (crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
	for (i = 0; i < op_array->last; i++) {
		/* Opline with Jit-ed code handler is skipped. */
		if (jit_extension->trace_info[i].trace_flags &
				(CREX_JIT_TRACE_JITED|CREX_JIT_TRACE_BLACKLISTED)) {
			continue;
		}
		if (jit_extension->trace_info[i].trace_flags &
				(CREX_JIT_TRACE_START_LOOP | CREX_JIT_TRACE_START_ENTER | CREX_JIT_TRACE_START_RETURN)) {
			op_array->opcodes[i].handler = jit_extension->trace_info[i].orig_handler;
		}
	}
}

/* Get the tracing op_array. */
static void crex_jit_stop_persistent_op_array(crex_op_array *op_array)
{
	crex_func_info *func_info = CREX_FUNC_INFO(op_array);
	if (!func_info) {
		return;
	}
	if (func_info->flags & CREX_FUNC_JIT_ON_HOT_TRACE) {
		crex_jit_stop_hot_trace_counters(op_array);
	}
}

/* Get all op_arrays with counter handler. */
static void crex_jit_stop_persistent_script(crex_persistent_script *script)
{
	crex_class_entry *ce;
	crex_op_array *op_array;

	crex_jit_stop_persistent_op_array(&script->script.main_op_array);

	CREX_HASH_FOREACH_PTR(&script->script.function_table, op_array) {
		crex_jit_stop_persistent_op_array(op_array);
	} CREX_HASH_FOREACH_END();

	CREX_HASH_FOREACH_PTR(&script->script.class_table, ce) {
		CREX_HASH_FOREACH_PTR(&ce->function_table, op_array) {
			if (op_array->type == CREX_USER_FUNCTION) {
				crex_jit_stop_persistent_op_array(op_array);
			}
		} CREX_HASH_FOREACH_END();
	} CREX_HASH_FOREACH_END();
}

/* Get all scripts which are accelerated by JIT */
static void crex_jit_stop_counter_handlers(void)
{
	if (ZCSG(jit_counters_stopped)) {
		return;
	}

	crex_shared_alloc_lock();
	/* mprotect has an extreme overhead, avoid calls to it for every function. */
	SHM_UNPROTECT();
	if (!ZCSG(jit_counters_stopped)) {
		ZCSG(jit_counters_stopped) = true;
		for (uint32_t i = 0; i < ZCSG(hash).max_num_entries; i++) {
			crex_accel_hash_entry *cache_entry;
			for (cache_entry = ZCSG(hash).hash_table[i]; cache_entry; cache_entry = cache_entry->next) {
				crex_persistent_script *script;
				if (cache_entry->indirect) continue;
				script = (crex_persistent_script *)cache_entry->data;
				crex_jit_stop_persistent_script(script);
			}
		}
	}
	SHM_PROTECT();
	crex_shared_alloc_unlock();
}

static void crex_jit_blacklist_root_trace(const crex_op *opline, size_t offset)
{
	crex_shared_alloc_lock();

	if (!(CREX_OP_TRACE_INFO(opline, offset)->trace_flags & CREX_JIT_TRACE_BLACKLISTED)) {
		SHM_UNPROTECT();
		crex_jit_unprotect();

		((crex_op*)opline)->handler =
			CREX_OP_TRACE_INFO(opline, offset)->orig_handler;

		CREX_OP_TRACE_INFO(opline, offset)->trace_flags |= CREX_JIT_TRACE_BLACKLISTED;

		crex_jit_protect();
		SHM_PROTECT();
	}

	crex_shared_alloc_unlock();
}

static bool crex_jit_trace_is_bad_root(const crex_op *opline, crex_jit_trace_stop stop, size_t offset)
{
	const crex_op **cache_opline = JIT_G(bad_root_cache_opline);
	uint8_t *cache_count = JIT_G(bad_root_cache_count);
	uint8_t *cache_stop = JIT_G(bad_root_cache_stop);
	uint32_t cache_slot = JIT_G(bad_root_slot);
	uint32_t i;

	for (i = 0; i < CREX_JIT_TRACE_BAD_ROOT_SLOTS; i++) {
		if (cache_opline[i] == opline) {
			if (cache_count[i] >= JIT_G(blacklist_root_trace) - 1) {
				cache_opline[i] = NULL;
				return 1;
			} else {
#if 0
				if (CREX_OP_TRACE_INFO(opline, offset)->counter) {
					*CREX_OP_TRACE_INFO(opline, offset)->counter =
						random() % CREX_JIT_TRACE_COUNTER_MAX;
				}
#endif
				cache_count[i]++;
				cache_stop[i] = stop;
				return 0;
			}
		}
	}
	i = cache_slot;
	cache_opline[i] = opline;
	cache_count[i] = 1;
	cache_stop[i] = stop;
	cache_slot = (i + 1) % CREX_JIT_TRACE_BAD_ROOT_SLOTS;
	JIT_G(bad_root_slot) = cache_slot;
	return 0;
}

static void crex_jit_dump_trace(crex_jit_trace_rec *trace_buffer, crex_ssa *tssa)
{
	crex_jit_trace_rec *p = trace_buffer;
	const crex_op_array *op_array;
	const crex_op *opline;
	uint32_t level = 1 + trace_buffer[0].level;
	int idx, len, i, v, vars_count, call_level;

	CREX_ASSERT(p->op == CREX_JIT_TRACE_START);
	op_array = p->op_array;
	p += CREX_JIT_TRACE_START_REC_SIZE;
	idx = 0;
	call_level = 0;

	if (tssa && tssa->var_info) {
		if (trace_buffer->start == CREX_JIT_TRACE_START_ENTER) {
			vars_count = op_array->last_var;
		} else {
			vars_count = op_array->last_var + op_array->T;
		}
		for (i = 0; i < vars_count; i++) {
			if (tssa->vars[i].use_chain >= 0 || tssa->vars[i].phi_use_chain) {
				fprintf(stderr, "    %*c;", level, ' ');
				crex_dump_ssa_var(op_array, tssa, i, 0, i, CREX_DUMP_RC_INFERENCE);
				fprintf(stderr, "\n");
			}
		}
		if (trace_buffer->stop == CREX_JIT_TRACE_STOP_LOOP
		 || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_CALL
		 || trace_buffer->stop == CREX_JIT_TRACE_STOP_RECURSIVE_RET) {
			crex_ssa_phi *p = tssa->blocks[1].phis;

			fprintf(stderr, "LOOP:\n");

			while (p) {
				fprintf(stderr, "     ;");
				crex_dump_ssa_var(op_array, tssa, p->ssa_var, 0, p->var, CREX_DUMP_RC_INFERENCE);
				fprintf(stderr, " = Phi(");
				crex_dump_ssa_var(op_array, tssa, p->sources[0], 0, p->var, CREX_DUMP_RC_INFERENCE);
				fprintf(stderr, ", ");
				crex_dump_ssa_var(op_array, tssa, p->sources[1], 0, p->var, CREX_DUMP_RC_INFERENCE);
				fprintf(stderr, ")\n");
				p = p->next;
			}
		}
	}

	while (1) {
		if (p->op == CREX_JIT_TRACE_VM) {
			uint8_t op1_type, op2_type, op3_type;

			opline = p->opline;
			fprintf(stderr, "%04d%*c",
				(int)(opline - op_array->opcodes),
				level, ' ');
			crex_dump_op(op_array, NULL, opline, CREX_DUMP_RC_INFERENCE, tssa, (tssa && tssa->ops) ? tssa->ops + idx : NULL);

			op1_type = p->op1_type;
			op2_type = p->op2_type;
			op3_type = p->op3_type;
			if (op1_type != IS_UNKNOWN || op2_type != IS_UNKNOWN || op3_type != IS_UNKNOWN) {
				fprintf(stderr, " ;");
				if (op1_type != IS_UNKNOWN) {
					const char *ref = (op1_type & IS_TRACE_INDIRECT) ?
						((op1_type & IS_TRACE_REFERENCE) ? "*&" : "*") :
						((op1_type & IS_TRACE_REFERENCE) ? "&" : "");
					if ((p+1)->op == CREX_JIT_TRACE_OP1_TYPE) {
						p++;
						fprintf(stderr, " op1(%sobject of class %s)", ref,
							ZSTR_VAL(p->ce->name));
					} else {
						const char *type = ((op1_type & ~IS_TRACE_INDIRECT) == 0) ? "undef" : crex_get_type_by_const(op1_type & ~(IS_TRACE_REFERENCE|IS_TRACE_INDIRECT|IS_TRACE_PACKED));
						fprintf(stderr, " op1(%s%s%s)", ref, (op1_type & IS_TRACE_PACKED) ? "packed " : "", type);
					}
				}
				if (op2_type != IS_UNKNOWN) {
					const char *ref = (op2_type & IS_TRACE_INDIRECT) ?
						((op2_type & IS_TRACE_REFERENCE) ? "*&" : "*") :
						((op2_type & IS_TRACE_REFERENCE) ? "&" : "");
					if ((p+1)->op == CREX_JIT_TRACE_OP2_TYPE) {
						p++;
						fprintf(stderr, " op2(%sobject of class %s)", ref,
							ZSTR_VAL(p->ce->name));
					} else {
						const char *type = ((op2_type & ~IS_TRACE_INDIRECT) == 0) ? "undef" : crex_get_type_by_const(op2_type & ~(IS_TRACE_REFERENCE|IS_TRACE_INDIRECT));
						fprintf(stderr, " op2(%s%s)", ref, type);
					}
				}
				if (op3_type != IS_UNKNOWN) {
					const char *ref = (op3_type & IS_TRACE_INDIRECT) ?
						((op3_type & IS_TRACE_REFERENCE) ? "*&" : "*") :
						((op3_type & IS_TRACE_REFERENCE) ? "&" : "");
					const char *type = ((op3_type & ~IS_TRACE_INDIRECT) == 0) ? "undef" : crex_get_type_by_const(op3_type & ~(IS_TRACE_REFERENCE|IS_TRACE_INDIRECT));
					fprintf(stderr, " op3(%s%s)", ref, type);
				}
			}
			if ((p+1)->op == CREX_JIT_TRACE_VAL_INFO) {
				uint8_t val_type;
				const char *type;

				if (op1_type == IS_UNKNOWN && op2_type == IS_UNKNOWN && op3_type == IS_UNKNOWN) {
					fprintf(stderr, " ;");
				}
				p++;
				val_type = p->op1_type;

				if (val_type == IS_UNDEF) {
					type = "undef";
				} else if (val_type == IS_REFERENCE) {
					type = "ref";
				} else {
					type = crex_get_type_by_const(val_type);
				}
				fprintf(stderr, " val(%s)", type);
			}
			fprintf(stderr, "\n");
			idx++;

			len = crex_jit_trace_op_len(opline);
			while (len > 1) {
				opline++;
				fprintf(stderr, "%04d%*c;",
					(int)(opline - op_array->opcodes),
					level, ' ');
				crex_dump_op(op_array, NULL, opline, CREX_DUMP_RC_INFERENCE, tssa, (tssa && tssa->ops) ? tssa->ops + idx : NULL);
				idx++;
				len--;
				fprintf(stderr, "\n");
			}
		} else if (p->op == CREX_JIT_TRACE_ENTER) {
			op_array = p->op_array;
			fprintf(stderr, "    %*c>enter %s%s%s\n",
				level, ' ',
				op_array->scope ? ZSTR_VAL(op_array->scope->name) : "",
				op_array->scope ? "::" : "",
				op_array->function_name ?
					ZSTR_VAL(op_array->function_name) :
					ZSTR_VAL(op_array->filename));
			level++;
			if (tssa && tssa->var_info) {
				call_level++;
				v = CREX_JIT_TRACE_GET_FIRST_SSA_VAR(p->info);
				vars_count = op_array->last_var;
				for (i = 0; i < vars_count; i++, v++) {
					if (tssa->vars[v].use_chain >= 0 || tssa->vars[v].phi_use_chain) {
						fprintf(stderr, "    %*c;", level, ' ');
						crex_dump_ssa_var(op_array, tssa, v, 0, i, CREX_DUMP_RC_INFERENCE);
						fprintf(stderr, "\n");
					}
				}
			}
		} else if (p->op == CREX_JIT_TRACE_BACK) {
			op_array = p->op_array;
			level--;
			fprintf(stderr, "    %*c<back %s%s%s\n",
				level, ' ',
				op_array->scope ? ZSTR_VAL(op_array->scope->name) : "",
				op_array->scope ? "::" : "",
				op_array->function_name ?
					ZSTR_VAL(op_array->function_name) :
					ZSTR_VAL(op_array->filename));
			if (tssa && tssa->var_info) {
				if (call_level == 0) {
					v = CREX_JIT_TRACE_GET_FIRST_SSA_VAR(p->info);
					vars_count = op_array->last_var + op_array->T;
					for (i = 0; i < vars_count; i++, v++) {
						if (tssa->vars[v].use_chain >= 0 || tssa->vars[v].phi_use_chain) {
							fprintf(stderr, "    %*c;", level, ' ');
							crex_dump_ssa_var(op_array, tssa, v, 0, i, CREX_DUMP_RC_INFERENCE);
							fprintf(stderr, "\n");
						}
					}
				} else {
					call_level--;
				}
			}
		} else if (p->op == CREX_JIT_TRACE_INIT_CALL) {
			if (p->func != (crex_function*)&crex_pass_function) {
				fprintf(stderr, (p->info & CREX_JIT_TRACE_FAKE_INIT_CALL) ? "    %*c>fake_init %s%s%s\n" : "    %*c>init %s%s%s\n",
					level, ' ',
					(p->func && p->func->common.scope) ? ZSTR_VAL(p->func->common.scope->name) : "",
					(p->func && p->func->common.scope) ? "::" : "",
					p->func ? ZSTR_VAL(p->func->common.function_name) : "???");
			} else {
				fprintf(stderr, "    %*c>skip\n",
					level, ' ');
			}
		} else if (p->op == CREX_JIT_TRACE_DO_ICALL) {
			if (p->func != (crex_function*)&crex_pass_function) {
				fprintf(stderr, "    %*c>call %s%s%s\n",
					level, ' ',
					p->func->common.scope ? ZSTR_VAL(p->func->common.scope->name) : "",
					p->func->common.scope ? "::" : "",
					ZSTR_VAL(p->func->common.function_name));
			} else {
				fprintf(stderr, "    %*c>skip\n",
					level, ' ');
			}
		} else if (p->op == CREX_JIT_TRACE_END) {
			break;
		}
		p++;
	}
}

static void crex_jit_dump_exit_info(crex_jit_trace_info *t)
{
	int i, j;

	fprintf(stderr, "---- TRACE %d exit info\n", t->id);
	for (i = 0; i < t->exit_count; i++) {
		const crex_op_array *op_array = t->exit_info[i].op_array;
		uint32_t stack_size = t->exit_info[i].stack_size;
		crex_jit_trace_stack *stack = t->stack_map + t->exit_info[i].stack_offset;

		fprintf(stderr, "     exit_%d:", i);
		if (t->exit_info[i].opline) {
			fprintf(stderr, " %04d/", (int)(t->exit_info[i].opline - op_array->opcodes));
		} else {
			fprintf(stderr, " ----/");
		}
		if (t->exit_info[i].stack_size) {
			fprintf(stderr, "%04d/%d", t->exit_info[i].stack_offset, t->exit_info[i].stack_size);
		} else {
			fprintf(stderr, "----/0");
		}
		if (t->exit_info[i].flags & CREX_JIT_EXIT_TO_VM) {
			fprintf(stderr, "/VM");
		}
		if (t->exit_info[i].flags & CREX_JIT_EXIT_RESTORE_CALL) {
			fprintf(stderr, "/CALL");
		}
		if (t->exit_info[i].flags & (CREX_JIT_EXIT_POLYMORPHISM|CREX_JIT_EXIT_METHOD_CALL|CREX_JIT_EXIT_CLOSURE_CALL)) {
			fprintf(stderr, "/POLY");
		}
		if (t->exit_info[i].flags & CREX_JIT_EXIT_FREE_OP1) {
			fprintf(stderr, "/FREE_OP1");
		}
		if (t->exit_info[i].flags & CREX_JIT_EXIT_FREE_OP2) {
			fprintf(stderr, "/FREE_OP2");
		}
		for (j = 0; j < stack_size; j++) {
			uint8_t type = STACK_TYPE(stack, j);
			if (type != IS_UNKNOWN) {
				fprintf(stderr, " ");
				crex_dump_var(op_array, (j < op_array->last_var) ? IS_CV : 0, j);
				fprintf(stderr, ":");
				if (type == IS_UNDEF) {
					fprintf(stderr, "undef");
				} else {
					fprintf(stderr, "%s", crex_get_type_by_const(type));
				}
				if (STACK_REG(stack, j) != ZREG_NONE) {
					if (STACK_REG(stack, j) < ZREG_NUM) {
						fprintf(stderr, "(%s)", crex_reg_name[STACK_REG(stack, j)]);
					} else if (STACK_REG(stack, j) == ZREG_THIS) {
						fprintf(stderr, "(this)");
					} else if (STACK_REG(stack, j) == ZREG_ZVAL_TRY_ADDREF) {
						fprintf(stderr, "(zval_try_addref)");
					} else {
						fprintf(stderr, "(const_%d)", STACK_REG(stack, j) - ZREG_NUM);
					}
				}
			} else if (STACK_REG(stack, j) == ZREG_ZVAL_TRY_ADDREF) {
				fprintf(stderr, " ");
				crex_dump_var(op_array, (j < op_array->last_var) ? IS_CV : 0, j);
				fprintf(stderr, ":unknown(zval_try_addref)");
			} else if (STACK_REG(stack, j) == ZREG_ZVAL_COPY_GPR0) {
				fprintf(stderr, " ");
				crex_dump_var(op_array, (j < op_array->last_var) ? IS_CV : 0, j);
				fprintf(stderr, ":unknown(zval_copy(%s))", crex_reg_name[ZREG_COPY]);
			}
		}
		fprintf(stderr, "\n");
	}
}

int CREX_FASTCALL crex_jit_trace_hot_root(crex_execute_data *execute_data, const crex_op *opline)
{
	const crex_op *orig_opline;
	crex_jit_trace_stop stop;
	int ret = 0;
	crex_op_array *op_array;
	crex_jit_op_array_trace_extension *jit_extension;
	size_t offset;
	uint32_t trace_num;
	crex_jit_trace_rec trace_buffer[CREX_JIT_TRACE_MAX_LENGTH];

	CREX_ASSERT(EX(func)->type == CREX_USER_FUNCTION);
	CREX_ASSERT(opline >= EX(func)->op_array.opcodes &&
		opline < EX(func)->op_array.opcodes + EX(func)->op_array.last);

repeat:
	trace_num = CREX_JIT_TRACE_NUM;
	orig_opline = opline;
	op_array = &EX(func)->op_array;
	jit_extension = (crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
	offset = jit_extension->offset;

	EX(opline) = opline;

	/* Lock-free check if the root trace was already JIT-ed or blacklist-ed in another process */
	if (CREX_OP_TRACE_INFO(opline, offset)->trace_flags & (CREX_JIT_TRACE_JITED|CREX_JIT_TRACE_BLACKLISTED)) {
		return 0;
	}

	if (JIT_G(tracing)) {
		++(*CREX_OP_TRACE_INFO(opline, offset)->counter);
		return 0;
	}

	if (JIT_G(debug) & CREX_JIT_DEBUG_TRACE_START) {
		fprintf(stderr, "---- TRACE %d start (%s) %s%s%s() %s:%d\n",
			trace_num,
			crex_jit_trace_star_desc(CREX_OP_TRACE_INFO(opline, offset)->trace_flags),
			EX(func)->op_array.scope ? ZSTR_VAL(EX(func)->op_array.scope->name) : "",
			EX(func)->op_array.scope ? "::" : "",
			EX(func)->op_array.function_name ?
				ZSTR_VAL(EX(func)->op_array.function_name) : "$main",
			ZSTR_VAL(EX(func)->op_array.filename),
			opline->lineno);
	}

	if (CREX_JIT_TRACE_NUM >= JIT_G(max_root_traces)) {
		stop = CREX_JIT_TRACE_STOP_TOO_MANY_TRACES;
		crex_jit_stop_counter_handlers();
		goto abort;
	}

	JIT_G(tracing) = 1;
	stop = crex_jit_trace_execute(execute_data, opline, trace_buffer,
		CREX_OP_TRACE_INFO(opline, offset)->trace_flags & CREX_JIT_TRACE_START_MASK, 0);
	JIT_G(tracing) = 0;

	if (stop & CREX_JIT_TRACE_HALT) {
		ret = -1;
	}
	stop &= ~CREX_JIT_TRACE_HALT;

	if (UNEXPECTED(trace_buffer[1].opline != orig_opline)) {
		orig_opline = trace_buffer[1].opline;
		op_array = (crex_op_array*)trace_buffer[0].op_array;
		jit_extension = (crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
		offset = jit_extension->offset;
		if (JIT_G(debug) & CREX_JIT_DEBUG_TRACE_START) {
			const crex_op_array *op_array = trace_buffer[0].op_array;
			const crex_op *opline = trace_buffer[1].opline;
			crex_jit_op_array_trace_extension *jit_extension =
				(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
			size_t offset = jit_extension->offset;

			fprintf(stderr, "---- TRACE %d start (%s) %s%s%s() %s:%d\n",
				trace_num,
				crex_jit_trace_star_desc(CREX_OP_TRACE_INFO(opline, offset)->trace_flags),
				op_array->scope ? ZSTR_VAL(op_array->scope->name) : "",
				op_array->scope ? "::" : "",
				op_array->function_name ?
					ZSTR_VAL(op_array->function_name) : "$main",
				ZSTR_VAL(op_array->filename),
				opline->lineno);
		}
	}

	if (UNEXPECTED(JIT_G(debug) & CREX_JIT_DEBUG_TRACE_BYTECODE)) {
		crex_jit_dump_trace(trace_buffer, NULL);
	}

	if (CREX_JIT_TRACE_STOP_OK(stop)) {
		if (JIT_G(debug) & CREX_JIT_DEBUG_TRACE_STOP) {
			if (stop == CREX_JIT_TRACE_STOP_LINK) {
				uint32_t idx = trace_buffer[1].last;
				uint32_t link_to = crex_jit_find_trace(trace_buffer[idx].opline->handler);
				fprintf(stderr, "---- TRACE %d stop (link to %d)\n",
					trace_num,
					link_to);
			} else {
				fprintf(stderr, "---- TRACE %d stop (%s)\n",
					trace_num,
					crex_jit_trace_stop_description[stop]);
			}
		}
		stop = crex_jit_compile_root_trace(trace_buffer, orig_opline, offset);
		if (EXPECTED(CREX_JIT_TRACE_STOP_DONE(stop))) {
			if (JIT_G(debug) & CREX_JIT_DEBUG_TRACE_COMPILED) {
				fprintf(stderr, "---- TRACE %d %s\n",
					trace_num,
					crex_jit_trace_stop_description[stop]);
			}
		} else {
			goto abort;
		}
	} else {
abort:
		if (JIT_G(debug) & CREX_JIT_DEBUG_TRACE_ABORT) {
			fprintf(stderr, "---- TRACE %d abort (%s)\n",
				trace_num,
				crex_jit_trace_stop_description[stop]);
		}
		if (!CREX_JIT_TRACE_STOP_MAY_RECOVER(stop)
		 || crex_jit_trace_is_bad_root(orig_opline, stop, offset)) {
			if (JIT_G(debug) & CREX_JIT_DEBUG_TRACE_BLACKLIST) {
				fprintf(stderr, "---- TRACE %d blacklisted\n",
					trace_num);
			}
			crex_jit_blacklist_root_trace(orig_opline, offset);
		}
		if (CREX_JIT_TRACE_STOP_REPEAT(stop)) {
			execute_data = EG(current_execute_data);
			opline = EX(opline);
			goto repeat;
		}
	}

	if (JIT_G(debug) & (CREX_JIT_DEBUG_TRACE_STOP|CREX_JIT_DEBUG_TRACE_ABORT|CREX_JIT_DEBUG_TRACE_COMPILED|CREX_JIT_DEBUG_TRACE_BLACKLIST)) {
		fprintf(stderr, "\n");
	}

	return ret;
}

static void crex_jit_blacklist_trace_exit(uint32_t trace_num, uint32_t exit_num)
{
	const void *handler;
	bool do_bailout = 0;

	crex_shared_alloc_lock();

	if (!(crex_jit_traces[trace_num].exit_info[exit_num].flags & (CREX_JIT_EXIT_JITED|CREX_JIT_EXIT_BLACKLISTED))) {
		SHM_UNPROTECT();
		crex_jit_unprotect();

		crex_try {
			handler = crex_jit_trace_exit_to_vm(trace_num, exit_num);

			if (handler) {
				crex_jit_link_side_trace(
					crex_jit_traces[trace_num].code_start,
					crex_jit_traces[trace_num].code_size,
					crex_jit_traces[trace_num].jmp_table_size,
					exit_num,
					handler);
			}
			crex_jit_traces[trace_num].exit_info[exit_num].flags |= CREX_JIT_EXIT_BLACKLISTED;
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
}

static bool crex_jit_trace_exit_is_bad(uint32_t trace_num, uint32_t exit_num)
{
	uint8_t *counter = JIT_G(exit_counters) +
		crex_jit_traces[trace_num].exit_counters + exit_num;

	if (*counter + 1 >= JIT_G(hot_side_exit) + JIT_G(blacklist_side_trace)) {
		return 1;
	}
	(*counter)++;
	return 0;
}

static bool crex_jit_trace_exit_is_hot(uint32_t trace_num, uint32_t exit_num)
{
	uint8_t *counter = JIT_G(exit_counters) +
		crex_jit_traces[trace_num].exit_counters + exit_num;

	if (*counter + 1 >= JIT_G(hot_side_exit)) {
		return 1;
	}
	(*counter)++;
	return 0;
}

static crex_jit_trace_stop crex_jit_compile_side_trace(crex_jit_trace_rec *trace_buffer, uint32_t parent_num, uint32_t exit_num, uint32_t polymorphism)
{
	crex_jit_trace_stop ret;
	const void *handler;
	uint8_t orig_trigger;
	crex_jit_trace_info *t;
	crex_jit_trace_exit_info exit_info[CREX_JIT_TRACE_MAX_EXITS];
	bool do_bailout = 0;

	crex_shared_alloc_lock();

	/* Checks under lock */
	if (crex_jit_traces[parent_num].exit_info[exit_num].flags & (CREX_JIT_EXIT_JITED|CREX_JIT_EXIT_BLACKLISTED)) {
		ret = CREX_JIT_TRACE_STOP_ALREADY_DONE;
	} else if (CREX_JIT_TRACE_NUM >= JIT_G(max_root_traces)) {
		ret = CREX_JIT_TRACE_STOP_TOO_MANY_TRACES;
	} else if (crex_jit_traces[crex_jit_traces[parent_num].root].child_count >= JIT_G(max_side_traces)) {
		ret = CREX_JIT_TRACE_STOP_TOO_MANY_CHILDREN;
	} else {
		SHM_UNPROTECT();
		crex_jit_unprotect();

		crex_try {
			t = &crex_jit_traces[CREX_JIT_TRACE_NUM];

			t->id = CREX_JIT_TRACE_NUM;
			t->root = crex_jit_traces[parent_num].root;
			t->parent = parent_num;
			t->link = 0;
			t->exit_count = 0;
			t->child_count = 0;
			t->stack_map_size = 0;
			t->flags = 0;
			t->polymorphism = polymorphism;
			t->jmp_table_size = 0;
			t->opline = NULL;
			t->exit_info = exit_info;
			t->stack_map = NULL;

			orig_trigger = JIT_G(trigger);
			JIT_G(trigger) = CREX_JIT_ON_HOT_TRACE;

			handler = crex_jit_trace(trace_buffer, parent_num, exit_num);

			JIT_G(trigger) = orig_trigger;

			if (handler) {
				crex_jit_trace_exit_info *shared_exit_info = NULL;

				t->exit_info = NULL;
				if (t->exit_count) {
					/* reallocate exit_info into shared memory */
					shared_exit_info = (crex_jit_trace_exit_info*)crex_shared_alloc(
						sizeof(crex_jit_trace_exit_info) * t->exit_count);

					if (!shared_exit_info) {
						if (t->stack_map) {
							efree(t->stack_map);
							t->stack_map = NULL;
						}
						ret = CREX_JIT_TRACE_STOP_NO_SHM;
						goto exit;
					}
					memcpy(shared_exit_info, exit_info,
						sizeof(crex_jit_trace_exit_info) * t->exit_count);
					t->exit_info = shared_exit_info;
				}

				if (t->stack_map_size) {
					crex_jit_trace_stack *shared_stack_map = (crex_jit_trace_stack*)crex_shared_alloc(t->stack_map_size * sizeof(crex_jit_trace_stack));
					if (!shared_stack_map) {
						efree(t->stack_map);
						ret = CREX_JIT_TRACE_STOP_NO_SHM;
						goto exit;
					}
					memcpy(shared_stack_map, t->stack_map, t->stack_map_size * sizeof(crex_jit_trace_stack));
					efree(t->stack_map);
					t->stack_map = shared_stack_map;
			    }

				crex_jit_link_side_trace(
					crex_jit_traces[parent_num].code_start,
					crex_jit_traces[parent_num].code_size,
					crex_jit_traces[parent_num].jmp_table_size,
					exit_num,
					handler);

				t->exit_counters = CREX_JIT_EXIT_COUNTERS;
				CREX_JIT_EXIT_COUNTERS += t->exit_count;

				crex_jit_traces[crex_jit_traces[parent_num].root].child_count++;
				CREX_JIT_TRACE_NUM++;
				crex_jit_traces[parent_num].exit_info[exit_num].flags |= CREX_JIT_EXIT_JITED;

				ret = CREX_JIT_TRACE_STOP_COMPILED;
			} else if (t->exit_count >= CREX_JIT_TRACE_MAX_EXITS ||
			           CREX_JIT_EXIT_COUNTERS + t->exit_count >= JIT_G(max_exit_counters)) {
			    if (t->stack_map) {
					efree(t->stack_map);
					t->stack_map = NULL;
				}
				ret = CREX_JIT_TRACE_STOP_TOO_MANY_EXITS;
			} else {
				if (t->stack_map) {
					efree(t->stack_map);
					t->stack_map = NULL;
				}
				ret = CREX_JIT_TRACE_STOP_COMPILER_ERROR;
			}

exit:;
		} crex_catch {
			do_bailout = 1;
		}  crex_end_try();

		crex_jit_protect();
		SHM_PROTECT();
	}

	crex_shared_alloc_unlock();

	if (do_bailout) {
		crex_bailout();
	}

	if ((JIT_G(debug) & CREX_JIT_DEBUG_TRACE_EXIT_INFO) != 0
	 && ret == CREX_JIT_TRACE_STOP_COMPILED
	 && t->exit_count > 0) {
		crex_jit_dump_exit_info(t);
	}

	return ret;
}

int CREX_FASTCALL crex_jit_trace_hot_side(crex_execute_data *execute_data, uint32_t parent_num, uint32_t exit_num)
{
	crex_jit_trace_stop stop;
	int ret = 0;
	uint32_t trace_num;
	crex_jit_trace_rec trace_buffer[CREX_JIT_TRACE_MAX_LENGTH];
	uint32_t is_megamorphic = 0;
	uint32_t polymorphism = 0;

	trace_num = CREX_JIT_TRACE_NUM;

	/* Lock-free check if the side trace was already JIT-ed or blacklist-ed in another process */
	if (crex_jit_traces[parent_num].exit_info[exit_num].flags & (CREX_JIT_EXIT_JITED|CREX_JIT_EXIT_BLACKLISTED)) {
		return 0;
	}

	if (JIT_G(debug) & CREX_JIT_DEBUG_TRACE_START) {
		fprintf(stderr, "---- TRACE %d start (side trace %d/%d) %s%s%s() %s:%d\n",
			trace_num, parent_num, exit_num,
			EX(func)->op_array.scope ? ZSTR_VAL(EX(func)->op_array.scope->name) : "",
			EX(func)->op_array.scope ? "::" : "",
			EX(func)->op_array.function_name ?
				ZSTR_VAL(EX(func)->op_array.function_name) : "$main",
			ZSTR_VAL(EX(func)->op_array.filename),
			EX(opline)->lineno);
	}

	if (CREX_JIT_TRACE_NUM >= JIT_G(max_root_traces)) {
		stop = CREX_JIT_TRACE_STOP_TOO_MANY_TRACES;
		goto abort;
	}

	if (crex_jit_traces[crex_jit_traces[parent_num].root].child_count >= JIT_G(max_side_traces)) {
		stop = CREX_JIT_TRACE_STOP_TOO_MANY_CHILDREN;
		goto abort;
	}

	if (JIT_G(max_polymorphic_calls) > 0) {
		if ((crex_jit_traces[parent_num].exit_info[exit_num].flags & (CREX_JIT_EXIT_METHOD_CALL|CREX_JIT_EXIT_CLOSURE_CALL))
		 || ((crex_jit_traces[parent_num].exit_info[exit_num].flags & CREX_JIT_EXIT_POLYMORPHISM)
		  && EX(call))) {
			if (crex_jit_traces[parent_num].polymorphism >= JIT_G(max_polymorphic_calls) - 1) {
				is_megamorphic = crex_jit_traces[parent_num].exit_info[exit_num].flags &
					(CREX_JIT_EXIT_METHOD_CALL | CREX_JIT_EXIT_CLOSURE_CALL | CREX_JIT_EXIT_POLYMORPHISM);
			} else if (!crex_jit_traces[parent_num].polymorphism) {
				polymorphism = 1;
			} else if (exit_num == 0) {
				polymorphism = crex_jit_traces[parent_num].polymorphism + 1;
			}
		}
	}

	JIT_G(tracing) = 1;
	stop = crex_jit_trace_execute(execute_data, EX(opline), trace_buffer, CREX_JIT_TRACE_START_SIDE, is_megamorphic);
	JIT_G(tracing) = 0;

	if (stop & CREX_JIT_TRACE_HALT) {
		ret = -1;
	}
	stop &= ~CREX_JIT_TRACE_HALT;

	if (UNEXPECTED(trace_buffer->start != CREX_JIT_TRACE_START_SIDE)) {
		if (JIT_G(debug) & CREX_JIT_DEBUG_TRACE_START) {
			const crex_op_array *op_array = trace_buffer[0].op_array;
			const crex_op *opline = trace_buffer[1].opline;
			crex_jit_op_array_trace_extension *jit_extension =
				(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
			size_t offset = jit_extension->offset;

			fprintf(stderr, "---- TRACE %d start (%s) %s%s%s() %s:%d\n",
				trace_num,
				crex_jit_trace_star_desc(CREX_OP_TRACE_INFO(opline, offset)->trace_flags),
				op_array->scope ? ZSTR_VAL(op_array->scope->name) : "",
				op_array->scope ? "::" : "",
				op_array->function_name ?
					ZSTR_VAL(op_array->function_name) : "$main",
				ZSTR_VAL(op_array->filename),
				opline->lineno);
		}
	}

	if (UNEXPECTED(JIT_G(debug) & CREX_JIT_DEBUG_TRACE_BYTECODE)) {
		crex_jit_dump_trace(trace_buffer, NULL);
	}

	if (CREX_JIT_TRACE_STOP_OK(stop)) {
		if (JIT_G(debug) & CREX_JIT_DEBUG_TRACE_STOP) {
			if (stop == CREX_JIT_TRACE_STOP_LINK) {
				uint32_t idx = trace_buffer[1].last;
				uint32_t link_to = crex_jit_find_trace(trace_buffer[idx].opline->handler);;
				fprintf(stderr, "---- TRACE %d stop (link to %d)\n",
					trace_num,
					link_to);
			} else {
				fprintf(stderr, "---- TRACE %d stop (%s)\n",
					trace_num,
					crex_jit_trace_stop_description[stop]);
			}
		}
		if (EXPECTED(trace_buffer->start == CREX_JIT_TRACE_START_SIDE)) {
			stop = crex_jit_compile_side_trace(trace_buffer, parent_num, exit_num, polymorphism);
		} else {
			const crex_op_array *op_array = trace_buffer[0].op_array;
			crex_jit_op_array_trace_extension *jit_extension =
				(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
			const crex_op *opline = trace_buffer[1].opline;

			stop = crex_jit_compile_root_trace(trace_buffer, opline, jit_extension->offset);
		}
		if (EXPECTED(CREX_JIT_TRACE_STOP_DONE(stop))) {
			if (JIT_G(debug) & CREX_JIT_DEBUG_TRACE_COMPILED) {
				fprintf(stderr, "---- TRACE %d %s\n",
					trace_num,
					crex_jit_trace_stop_description[stop]);
			}
		} else {
			goto abort;
		}
	} else {
abort:
		if (JIT_G(debug) & CREX_JIT_DEBUG_TRACE_ABORT) {
			fprintf(stderr, "---- TRACE %d abort (%s)\n",
				trace_num,
				crex_jit_trace_stop_description[stop]);
		}
		if (!CREX_JIT_TRACE_STOP_MAY_RECOVER(stop)
		 || crex_jit_trace_exit_is_bad(parent_num, exit_num)) {
			crex_jit_blacklist_trace_exit(parent_num, exit_num);
			if (JIT_G(debug) & CREX_JIT_DEBUG_TRACE_BLACKLIST) {
				fprintf(stderr, "---- EXIT %d/%d blacklisted\n",
					parent_num, exit_num);
			}
		}
		if (CREX_JIT_TRACE_STOP_REPEAT(stop)) {
			execute_data = EG(current_execute_data);
			return crex_jit_trace_hot_root(execute_data, EX(opline));
		}
	}

	if (JIT_G(debug) & (CREX_JIT_DEBUG_TRACE_STOP|CREX_JIT_DEBUG_TRACE_ABORT|CREX_JIT_DEBUG_TRACE_COMPILED|CREX_JIT_DEBUG_TRACE_BLACKLIST)) {
		fprintf(stderr, "\n");
	}

	return ret;
}

int CREX_FASTCALL crex_jit_trace_exit(uint32_t exit_num, crex_jit_registers_buf *regs)
{
	uint32_t trace_num = EG(jit_trace_num);
	crex_execute_data *execute_data = EG(current_execute_data);
	const crex_op *orig_opline = EX(opline);
	const crex_op *opline;
	crex_jit_trace_info *t = &crex_jit_traces[trace_num];
	int repeat_last_opline = 0;

	/* Deoptimization of VM stack state */
	uint32_t i;
	uint32_t stack_size = t->exit_info[exit_num].stack_size;
	crex_jit_trace_stack *stack = t->stack_map + t->exit_info[exit_num].stack_offset;

	if (t->exit_info[exit_num].flags & CREX_JIT_EXIT_RESTORE_CALL) {
		crex_execute_data *call = (crex_execute_data *)regs->gpr[ZREG_RX];
		call->prev_execute_data = EX(call);
		EX(call) = call;
	}

	for (i = 0; i < stack_size; i++) {
		if (STACK_REG(stack, i) != ZREG_NONE) {
			if (STACK_TYPE(stack, i) == IS_LONG) {
				crex_long val;

				if (STACK_REG(stack, i) < ZREG_NUM) {
					val = regs->gpr[STACK_REG(stack, i)];
				} else if (STACK_REG(stack, i) == ZREG_LONG_MIN) {
					val = CREX_LONG_MIN;
				} else if (STACK_REG(stack, i) == ZREG_LONG_MAX) {
					val = CREX_LONG_MAX;
				} else {
					CREX_UNREACHABLE();
				}
				ZVAL_LONG(EX_VAR_NUM(i), val);
			} else if (STACK_TYPE(stack, i) == IS_DOUBLE) {
				double val;

				if (STACK_REG(stack, i) < ZREG_NUM) {
					val = regs->fpr[STACK_REG(stack, i) - ZREG_FIRST_FPR];
				} else if (STACK_REG(stack, i) == ZREG_LONG_MIN_MINUS_1) {
					val = (double)CREX_LONG_MIN - 1.0;
				} else if (STACK_REG(stack, i) == ZREG_LONG_MAX_PLUS_1) {
					val = (double)CREX_LONG_MAX + 1.0;
				} else {
					CREX_UNREACHABLE();
				}
				ZVAL_DOUBLE(EX_VAR_NUM(i), val);
			} else if (STACK_REG(stack, i) == ZREG_THIS) {
				crex_object *obj = C_OBJ(EX(This));

				GC_ADDREF(obj);
				ZVAL_OBJ(EX_VAR_NUM(i), obj);
			} else if (STACK_REG(stack, i) == ZREG_NULL) {
				ZVAL_NULL(EX_VAR_NUM(i));
			} else if (STACK_REG(stack, i) == ZREG_ZVAL_TRY_ADDREF) {
				C_TRY_ADDREF_P(EX_VAR_NUM(i));
			} else if (STACK_REG(stack, i) == ZREG_ZVAL_COPY_GPR0) {
				zval *val = (zval*)regs->gpr[ZREG_COPY];

				if (UNEXPECTED(C_TYPE_P(val) == IS_UNDEF)) {
					/* Undefined array index or property */
					repeat_last_opline = 1;
				} else {
					ZVAL_COPY(EX_VAR_NUM(i), val);
				}
			} else {
				CREX_UNREACHABLE();
			}
		}
	}

	if (repeat_last_opline) {
		EX(opline) = t->exit_info[exit_num].opline - 1;
		if ((EX(opline)->op1_type & (IS_VAR|IS_TMP_VAR))
		 && !(t->exit_info[exit_num].flags & CREX_JIT_EXIT_FREE_OP1)
		 && EX(opline)->opcode != CREX_FETCH_LIST_R) {
			C_TRY_ADDREF_P(EX_VAR(EX(opline)->op1.var));
		}
		return 1;
	}

	opline = t->exit_info[exit_num].opline;

	if (opline) {
		if (t->exit_info[exit_num].flags & CREX_JIT_EXIT_FREE_OP2) {
			CREX_ASSERT((opline-1)->opcode == CREX_FETCH_DIM_R
					|| (opline-1)->opcode == CREX_FETCH_DIM_IS
					|| (opline-1)->opcode == CREX_FETCH_LIST_R
					|| (opline-1)->opcode == CREX_FETCH_DIM_FUNC_ARG);
			EX(opline) = opline-1;
			zval_ptr_dtor_nogc(EX_VAR((opline-1)->op2.var));
		}
		if (t->exit_info[exit_num].flags & CREX_JIT_EXIT_FREE_OP1) {
			CREX_ASSERT((opline-1)->opcode == CREX_FETCH_DIM_R
					|| (opline-1)->opcode == CREX_FETCH_DIM_IS
					|| (opline-1)->opcode == CREX_FETCH_DIM_FUNC_ARG
					|| (opline-1)->opcode == CREX_FETCH_OBJ_R
					|| (opline-1)->opcode == CREX_FETCH_OBJ_IS
					|| (opline-1)->opcode == CREX_FETCH_OBJ_FUNC_ARG);
			EX(opline) = opline-1;
			zval_ptr_dtor_nogc(EX_VAR((opline-1)->op1.var));
		}
		if (t->exit_info[exit_num].flags & (CREX_JIT_EXIT_FREE_OP1|CREX_JIT_EXIT_FREE_OP2)) {
			if (EG(exception)) {
				return 1;
			}
		}
		if (t->exit_info[exit_num].flags & CREX_JIT_EXIT_METHOD_CALL) {
			crex_function *func = (crex_function*)regs->gpr[ZREG_COPY];

			if (UNEXPECTED(func->common.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE)) {
				crex_string_release_ex(func->common.function_name, 0);
				crex_free_trampoline(func);
				EX(opline) = opline;
				return 1;
			}
		}

		/* Set VM opline to continue interpretation */
		EX(opline) = opline;
	}

	if (crex_atomic_bool_load_ex(&EG(vm_interrupt)) || JIT_G(tracing)) {
		return 1;
	/* Lock-free check if the side trace was already JIT-ed or blacklist-ed in another process */
	} else if (t->exit_info[exit_num].flags & (CREX_JIT_EXIT_JITED|CREX_JIT_EXIT_BLACKLISTED)) {
		return 0;
	}

	CREX_ASSERT(EX(func)->type == CREX_USER_FUNCTION);
	CREX_ASSERT(EX(opline) >= EX(func)->op_array.opcodes &&
		EX(opline) < EX(func)->op_array.opcodes + EX(func)->op_array.last);

	if (JIT_G(debug) & CREX_JIT_DEBUG_TRACE_EXIT) {
		fprintf(stderr, "     TRACE %d exit %d %s%s%s() %s:%d\n",
			trace_num,
			exit_num,
			EX(func)->op_array.scope ? ZSTR_VAL(EX(func)->op_array.scope->name) : "",
			EX(func)->op_array.scope ? "::" : "",
			EX(func)->op_array.function_name ?
				ZSTR_VAL(EX(func)->op_array.function_name) : "$main",
			ZSTR_VAL(EX(func)->op_array.filename),
			EX(opline)->lineno);
	}

	if (t->exit_info[exit_num].flags & CREX_JIT_EXIT_INVALIDATE) {
		crex_jit_op_array_trace_extension *jit_extension;
		uint32_t num = trace_num;

		while (t->root != num) {
			num = t->root;
			t = &crex_jit_traces[num];
		}

		crex_shared_alloc_lock();

		jit_extension = (crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(t->op_array);

		/* Checks under lock, just in case something has changed while we were waiting for the lock */
		if (!(CREX_OP_TRACE_INFO(t->opline, jit_extension->offset)->trace_flags & (CREX_JIT_TRACE_JITED|CREX_JIT_TRACE_BLACKLISTED))) {
			/* skip: not JIT-ed nor blacklisted */
		} else if (CREX_JIT_TRACE_NUM >= JIT_G(max_root_traces)) {
			/* skip: too many root traces */
		} else {
			SHM_UNPROTECT();
			crex_jit_unprotect();

			if (CREX_OP_TRACE_INFO(t->opline, jit_extension->offset)->trace_flags & CREX_JIT_TRACE_START_LOOP) {
				((crex_op*)(t->opline))->handler = (const void*)crex_jit_loop_trace_counter_handler;
			} else if (CREX_OP_TRACE_INFO(t->opline, jit_extension->offset)->trace_flags & CREX_JIT_TRACE_START_ENTER) {
				((crex_op*)(t->opline))->handler = (const void*)crex_jit_func_trace_counter_handler;
			} else if (CREX_OP_TRACE_INFO(t->opline, jit_extension->offset)->trace_flags & CREX_JIT_TRACE_START_RETURN) {
				((crex_op*)(t->opline))->handler = (const void*)crex_jit_ret_trace_counter_handler;
			}
			CREX_OP_TRACE_INFO(t->opline, jit_extension->offset)->trace_flags &=
				CREX_JIT_TRACE_START_LOOP|CREX_JIT_TRACE_START_ENTER|CREX_JIT_TRACE_START_RETURN;

			crex_jit_protect();
			SHM_PROTECT();
		}

		crex_shared_alloc_unlock();

		return 0;
	}

	if (t->exit_info[exit_num].flags & CREX_JIT_EXIT_TO_VM) {
		if (crex_jit_trace_exit_is_bad(trace_num, exit_num)) {
			crex_jit_blacklist_trace_exit(trace_num, exit_num);
			if (JIT_G(debug) & CREX_JIT_DEBUG_TRACE_BLACKLIST) {
				fprintf(stderr, "---- EXIT %d/%d blacklisted\n",
					trace_num, exit_num);
			}
			return 0;
		}
	} else if (JIT_G(hot_side_exit) && crex_jit_trace_exit_is_hot(trace_num, exit_num)) {
		return crex_jit_trace_hot_side(execute_data, trace_num, exit_num);
	}

	/* Return 1 to call original handler instead of the same JIT-ed trace */
	return (orig_opline == t->opline && EX(opline) == orig_opline);
}

static crex_always_inline uint8_t crex_jit_trace_supported(const crex_op *opline)
{
	switch (opline->opcode) {
		case CREX_CATCH:
		case CREX_FAST_CALL:
		case CREX_FAST_RET:
			return CREX_JIT_TRACE_UNSUPPORTED;
		default:
			return CREX_JIT_TRACE_SUPPORTED;
	}
}

static int crex_jit_restart_hot_trace_counters(crex_op_array *op_array)
{
	crex_jit_op_array_trace_extension *jit_extension;
	uint32_t i;

	jit_extension = (crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
	for (i = 0; i < op_array->last; i++) {
		jit_extension->trace_info[i].trace_flags &=
			CREX_JIT_TRACE_START_LOOP | CREX_JIT_TRACE_START_ENTER | CREX_JIT_TRACE_UNSUPPORTED;
		if (jit_extension->trace_info[i].trace_flags == CREX_JIT_TRACE_START_LOOP) {
			op_array->opcodes[i].handler = (const void*)crex_jit_loop_trace_counter_handler;
		} else if (jit_extension->trace_info[i].trace_flags == CREX_JIT_TRACE_START_ENTER) {
			op_array->opcodes[i].handler = (const void*)crex_jit_func_trace_counter_handler;
		} else {
			op_array->opcodes[i].handler = jit_extension->trace_info[i].orig_handler;
		}
	}
	return SUCCESS;
}

static int crex_jit_setup_hot_trace_counters(crex_op_array *op_array)
{
	crex_op *opline;
	crex_jit_op_array_trace_extension *jit_extension;
	uint32_t i;

	CREX_ASSERT(sizeof(crex_op_trace_info) == sizeof(crex_op));

	jit_extension = (crex_jit_op_array_trace_extension*)crex_shared_alloc(sizeof(crex_jit_op_array_trace_extension) + (op_array->last - 1) * sizeof(crex_op_trace_info));
	if (!jit_extension) {
		return FAILURE;
	}
	memset(&jit_extension->func_info, 0, sizeof(crex_func_info));
	jit_extension->func_info.flags = CREX_FUNC_JIT_ON_HOT_TRACE;
	jit_extension->op_array = op_array;
	jit_extension->offset = (char*)jit_extension->trace_info - (char*)op_array->opcodes;
	for (i = 0; i < op_array->last; i++) {
		jit_extension->trace_info[i].orig_handler = op_array->opcodes[i].handler;
		jit_extension->trace_info[i].call_handler = crex_get_opcode_handler_func(&op_array->opcodes[i]);
		jit_extension->trace_info[i].counter = NULL;
		jit_extension->trace_info[i].trace_flags =
			crex_jit_trace_supported(&op_array->opcodes[i]);
	}
	CREX_SET_FUNC_INFO(op_array, (void*)jit_extension);

	if (JIT_G(hot_loop)) {
		crex_cfg cfg;

		CREX_ASSERT(crex_jit_loop_trace_counter_handler != NULL);

		if (crex_jit_build_cfg(op_array, &cfg) != SUCCESS) {
			return FAILURE;
		}

		for (i = 0; i < cfg.blocks_count; i++) {
			if (cfg.blocks[i].flags & CREX_BB_REACHABLE) {
				if (cfg.blocks[i].flags & CREX_BB_LOOP_HEADER) {
					/* loop header */
					opline = op_array->opcodes + cfg.blocks[i].start;
					if (!(CREX_OP_TRACE_INFO(opline, jit_extension->offset)->trace_flags & CREX_JIT_TRACE_UNSUPPORTED)) {
						opline->handler = (const void*)crex_jit_loop_trace_counter_handler;
						if (!CREX_OP_TRACE_INFO(opline, jit_extension->offset)->counter) {
							CREX_OP_TRACE_INFO(opline, jit_extension->offset)->counter =
								&crex_jit_hot_counters[CREX_JIT_COUNTER_NUM];
							CREX_JIT_COUNTER_NUM = (CREX_JIT_COUNTER_NUM + 1) % CREX_HOT_COUNTERS_COUNT;
						}
						CREX_OP_TRACE_INFO(opline, jit_extension->offset)->trace_flags |=
							CREX_JIT_TRACE_START_LOOP;
					}
				}
			}
		}
	}

	if (JIT_G(hot_func)) {
		CREX_ASSERT(crex_jit_func_trace_counter_handler != NULL);
		opline = op_array->opcodes;
		if (!(op_array->fn_flags & CREX_ACC_HAS_TYPE_HINTS)) {
			while (opline->opcode == CREX_RECV || opline->opcode == CREX_RECV_INIT) {
				opline++;
			}
		}

		if (!CREX_OP_TRACE_INFO(opline, jit_extension->offset)->trace_flags) {
			/* function entry */
			opline->handler = (const void*)crex_jit_func_trace_counter_handler;
			CREX_OP_TRACE_INFO(opline, jit_extension->offset)->counter =
				&crex_jit_hot_counters[CREX_JIT_COUNTER_NUM];
			CREX_JIT_COUNTER_NUM = (CREX_JIT_COUNTER_NUM + 1) % CREX_HOT_COUNTERS_COUNT;
			CREX_OP_TRACE_INFO(opline, jit_extension->offset)->trace_flags |=
				CREX_JIT_TRACE_START_ENTER;
		}
	}

	crex_shared_alloc_register_xlat_entry(op_array->opcodes, jit_extension);

	return SUCCESS;
}

static void crex_jit_trace_init_caches(void)
{
	memset(CREX_VOIDP(JIT_G(bad_root_cache_opline)), 0, sizeof(JIT_G(bad_root_cache_opline)));
	memset(JIT_G(bad_root_cache_count), 0, sizeof(JIT_G(bad_root_cache_count)));
	memset(JIT_G(bad_root_cache_stop), 0, sizeof(JIT_G(bad_root_cache_count)));
	JIT_G(bad_root_slot) = 0;

	if (JIT_G(exit_counters)) {
		memset(JIT_G(exit_counters), 0, JIT_G(max_exit_counters));
	}
}

static void crex_jit_trace_reset_caches(void)
{
	JIT_G(tracing) = 0;
#ifdef ZTS
	if (!JIT_G(exit_counters)) {
		JIT_G(exit_counters) = calloc(JIT_G(max_exit_counters), 1);
	}
#endif
}

static void crex_jit_trace_free_caches(crex_jit_globals *jit_globals)
{
	if (jit_globals->exit_counters) {
		free(jit_globals->exit_counters);
	}
}

static void crex_jit_trace_restart(void)
{
	CREX_JIT_TRACE_NUM = 1;
	CREX_JIT_COUNTER_NUM = 0;
	CREX_JIT_EXIT_NUM = 0;
	CREX_JIT_EXIT_COUNTERS = 0;
	ZCSG(jit_counters_stopped) = false;

	crex_jit_trace_init_caches();
}
