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
   +----------------------------------------------------------------------+
*/

#include "Crex/crex_execute.h"
#include "Crex/crex_exceptions.h"
#include "Crex/crex_vm.h"
#include "Crex/crex_closures.h"
#include "Crex/crex_constants.h"
#include "Crex/crex_API.h"

#include <CrexAccelerator.h>
#include "Optimizer/crex_func_info.h"
#include "Optimizer/crex_call_graph.h"
#include "crex_jit.h"
#if CREX_JIT_TARGET_X86
# include "crex_jit_x86.h"
#elif CREX_JIT_TARGET_ARM64
# include "crex_jit_arm64.h"
#endif

#include "crex_jit_internal.h"

#ifdef HAVE_GCC_GLOBAL_REGS
# pragma GCC diagnostic ignored "-Wvolatile-register-var"
# if defined(__x86_64__)
register crex_execute_data* volatile execute_data __asm__("%r14");
register const crex_op* volatile opline __asm__("%r15");
# elif defined(i386)
register crex_execute_data* volatile execute_data __asm__("%esi");
register const crex_op* volatile opline __asm__("%edi");
# elif defined(__aarch64__)
register crex_execute_data* volatile execute_data __asm__("x27");
register const crex_op* volatile opline __asm__("x28");
# endif
# pragma GCC diagnostic warning "-Wvolatile-register-var"
#endif

CREX_OPCODE_HANDLER_RET CREX_FASTCALL crex_jit_leave_nested_func_helper(uint32_t call_info EXECUTE_DATA_DC)
{
	crex_execute_data *old_execute_data;

	if (UNEXPECTED(call_info & CREX_CALL_HAS_SYMBOL_TABLE)) {
		crex_clean_and_cache_symbol_table(EX(symbol_table));
	}

	crex_vm_stack_free_extra_args_ex(call_info, execute_data);
	if (UNEXPECTED(call_info & CREX_CALL_RELEASE_THIS)) {
		OBJ_RELEASE(C_OBJ(execute_data->This));
	} else if (UNEXPECTED(call_info & CREX_CALL_CLOSURE)) {
		OBJ_RELEASE(CREX_CLOSURE_OBJECT(EX(func)));
	}
	if (UNEXPECTED(call_info & CREX_CALL_HAS_EXTRA_NAMED_PARAMS)) {
		crex_free_extra_named_params(EX(extra_named_params));
	}

	old_execute_data = execute_data;
	execute_data = EX(prev_execute_data);
	crex_vm_stack_free_call_frame_ex(call_info, old_execute_data);

	if (UNEXPECTED(EG(exception) != NULL)) {
		const crex_op *old_opline = EX(opline);
		crex_throw_exception_internal(NULL);
		if (old_opline->result_type != IS_UNDEF) {
			zval_ptr_dtor(EX_VAR(old_opline->result.var));
		}
#ifndef HAVE_GCC_GLOBAL_REGS
		return 2; // CREX_VM_LEAVE
#endif
	} else {
		EX(opline)++;
#ifdef HAVE_GCC_GLOBAL_REGS
		opline = EX(opline);
#else
		return 2; // CREX_VM_LEAVE
#endif
	}
}

CREX_OPCODE_HANDLER_RET CREX_FASTCALL crex_jit_leave_top_func_helper(uint32_t call_info EXECUTE_DATA_DC)
{
	if (UNEXPECTED(call_info & (CREX_CALL_HAS_SYMBOL_TABLE|CREX_CALL_FREE_EXTRA_ARGS))) {
		if (UNEXPECTED(call_info & CREX_CALL_HAS_SYMBOL_TABLE)) {
			crex_clean_and_cache_symbol_table(EX(symbol_table));
		}
		crex_vm_stack_free_extra_args_ex(call_info, execute_data);
	}
	if (UNEXPECTED(call_info & CREX_CALL_HAS_EXTRA_NAMED_PARAMS)) {
		crex_free_extra_named_params(EX(extra_named_params));
	}
	if (UNEXPECTED(call_info & CREX_CALL_CLOSURE)) {
		OBJ_RELEASE(CREX_CLOSURE_OBJECT(EX(func)));
	}
	execute_data = EG(current_execute_data);
#ifdef HAVE_GCC_GLOBAL_REGS
	opline = crex_jit_halt_op;
#else
	return -1; // CREX_VM_RETURN
#endif
}

CREX_OPCODE_HANDLER_RET CREX_FASTCALL crex_jit_leave_func_helper(EXECUTE_DATA_D)
{
	uint32_t call_info = EX_CALL_INFO();

	if (call_info & CREX_CALL_TOP) {
		CREX_OPCODE_TAIL_CALL_EX(crex_jit_leave_top_func_helper, call_info);
	} else {
		CREX_OPCODE_TAIL_CALL_EX(crex_jit_leave_nested_func_helper, call_info);
	}
}

void CREX_FASTCALL crex_jit_copy_extra_args_helper(EXECUTE_DATA_D)
{
	crex_op_array *op_array = &EX(func)->op_array;

	if (EXPECTED(!(op_array->fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE))) {
		uint32_t first_extra_arg = op_array->num_args;
		uint32_t num_args = EX_NUM_ARGS();
		zval *end, *src, *dst;
		uint32_t type_flags = 0;

		if (EXPECTED((op_array->fn_flags & CREX_ACC_HAS_TYPE_HINTS) == 0)) {
			/* Skip useless CREX_RECV and CREX_RECV_INIT opcodes */
#ifdef HAVE_GCC_GLOBAL_REGS
			opline += first_extra_arg;
#else
			EX(opline) += first_extra_arg;
#endif
		}

		/* move extra args into separate array after all CV and TMP vars */
		end = EX_VAR_NUM(first_extra_arg - 1);
		src = end + (num_args - first_extra_arg);
		dst = src + (op_array->last_var + op_array->T - first_extra_arg);
		if (EXPECTED(src != dst)) {
			do {
				type_flags |= C_TYPE_INFO_P(src);
				ZVAL_COPY_VALUE(dst, src);
				ZVAL_UNDEF(src);
				src--;
				dst--;
			} while (src != end);
			if (type_flags & (IS_TYPE_REFCOUNTED << C_TYPE_FLAGS_SHIFT)) {
				CREX_ADD_CALL_FLAG(execute_data, CREX_CALL_FREE_EXTRA_ARGS);
			}
		} else {
			do {
				if (C_REFCOUNTED_P(src)) {
					CREX_ADD_CALL_FLAG(execute_data, CREX_CALL_FREE_EXTRA_ARGS);
					break;
				}
				src--;
			} while (src != end);
		}
	}
}

bool CREX_FASTCALL crex_jit_deprecated_helper(OPLINE_D)
{
	crex_execute_data *call = (crex_execute_data *) opline;
	crex_function *fbc = call->func;

	crex_deprecated_function(fbc);

	if (EG(exception)) {
#ifndef HAVE_GCC_GLOBAL_REGS
		crex_execute_data *execute_data = EG(current_execute_data);
#endif
		const crex_op *opline = EG(opline_before_exception);
		if (opline && RETURN_VALUE_USED(opline)) {
			ZVAL_UNDEF(EX_VAR(opline->result.var));
		}

		crex_vm_stack_free_args(call);

		if (UNEXPECTED(CREX_CALL_INFO(call) & CREX_CALL_RELEASE_THIS)) {
			OBJ_RELEASE(C_OBJ(call->This));
		}

		crex_vm_stack_free_call_frame(call);
		return 0;
	}
	return 1;
}

CREX_OPCODE_HANDLER_RET CREX_FASTCALL crex_jit_profile_helper(CREX_OPCODE_HANDLER_ARGS)
{
	crex_op_array *op_array = (crex_op_array*)EX(func);
	crex_jit_op_array_extension *jit_extension = (crex_jit_op_array_extension*)CREX_FUNC_INFO(op_array);
	crex_vm_opcode_handler_t handler = (crex_vm_opcode_handler_t) jit_extension->orig_handler;
	++*(uintptr_t*)(EX(run_time_cache) + crex_jit_profile_counter_rid);
	++crex_jit_profile_counter;
	CREX_OPCODE_TAIL_CALL(handler);
}

CREX_OPCODE_HANDLER_RET CREX_FASTCALL crex_jit_func_counter_helper(CREX_OPCODE_HANDLER_ARGS)
{
	crex_jit_op_array_hot_extension *jit_extension =
		(crex_jit_op_array_hot_extension*)CREX_FUNC_INFO(&EX(func)->op_array);
#ifndef HAVE_GCC_GLOBAL_REGS
	const crex_op *opline = EX(opline);
#endif

	*(jit_extension->counter) -= ((CREX_JIT_COUNTER_INIT + JIT_G(hot_func) - 1) / JIT_G(hot_func));

	if (UNEXPECTED(*(jit_extension->counter) <= 0)) {
		*(jit_extension->counter) = CREX_JIT_COUNTER_INIT;
		crex_jit_hot_func(execute_data, opline);
		CREX_OPCODE_RETURN();
	} else {
		crex_vm_opcode_handler_t handler = (crex_vm_opcode_handler_t)jit_extension->orig_handlers[opline - EX(func)->op_array.opcodes];
		CREX_OPCODE_TAIL_CALL(handler);
	}
}

CREX_OPCODE_HANDLER_RET CREX_FASTCALL crex_jit_loop_counter_helper(CREX_OPCODE_HANDLER_ARGS)
{
	crex_jit_op_array_hot_extension *jit_extension =
		(crex_jit_op_array_hot_extension*)CREX_FUNC_INFO(&EX(func)->op_array);
#ifndef HAVE_GCC_GLOBAL_REGS
	const crex_op *opline = EX(opline);
#endif

	*(jit_extension->counter) -= ((CREX_JIT_COUNTER_INIT + JIT_G(hot_loop) - 1) / JIT_G(hot_loop));

	if (UNEXPECTED(*(jit_extension->counter) <= 0)) {
		*(jit_extension->counter) = CREX_JIT_COUNTER_INIT;
		crex_jit_hot_func(execute_data, opline);
		CREX_OPCODE_RETURN();
	} else {
		crex_vm_opcode_handler_t handler = (crex_vm_opcode_handler_t)jit_extension->orig_handlers[opline - EX(func)->op_array.opcodes];
		CREX_OPCODE_TAIL_CALL(handler);
	}
}

static crex_always_inline crex_constant* _crex_quick_get_constant(
		const zval *key, uint32_t flags, int check_defined_only)
{
#ifndef HAVE_GCC_GLOBAL_REGS
	crex_execute_data *execute_data = EG(current_execute_data);
#endif
	const crex_op *opline = EX(opline);
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

	if (!c) {
		if (!check_defined_only) {
			crex_throw_error(NULL, "Undefined constant \"%s\"", C_STRVAL_P(RT_CONSTANT(opline, opline->op2)));
			ZVAL_UNDEF(EX_VAR(opline->result.var));
		}
		CACHE_PTR(opline->extended_value, ENCODE_SPECIAL_CACHE_NUM(crex_hash_num_elements(EG(crex_constants))));
		return NULL;
	}

	if (!check_defined_only) {
		if (CREX_CONSTANT_FLAGS(c) & CONST_DEPRECATED) {
			crex_error(E_DEPRECATED, "Constant %s is deprecated", ZSTR_VAL(c->name));
			if (EG(exception)) {
				return NULL;
			}
			return c;
		}
	}

	CACHE_PTR(opline->extended_value, c);
	return c;
}

crex_constant* CREX_FASTCALL crex_jit_get_constant(const zval *key, uint32_t flags)
{
	return _crex_quick_get_constant(key, flags, 0);
}

crex_constant* CREX_FASTCALL crex_jit_check_constant(const zval *key)
{
	return _crex_quick_get_constant(key, 0, 1);
}

static crex_always_inline CREX_OPCODE_HANDLER_RET CREX_FASTCALL crex_jit_trace_counter_helper(uint32_t cost CREX_OPCODE_HANDLER_ARGS_DC)
{
	crex_jit_op_array_trace_extension *jit_extension =
		(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(&EX(func)->op_array);
	size_t offset = jit_extension->offset;
#ifndef HAVE_GCC_GLOBAL_REGS
	const crex_op *opline = EX(opline);
#endif

	*(CREX_OP_TRACE_INFO(opline, offset)->counter) -= cost;

	if (UNEXPECTED(*(CREX_OP_TRACE_INFO(opline, offset)->counter) <= 0)) {
		*(CREX_OP_TRACE_INFO(opline, offset)->counter) = CREX_JIT_COUNTER_INIT;
		if (UNEXPECTED(crex_jit_trace_hot_root(execute_data, opline) < 0)) {
#ifdef HAVE_GCC_GLOBAL_REGS
			opline = NULL;
			return;
#else
			return -1;
#endif
		}
#ifdef HAVE_GCC_GLOBAL_REGS
		execute_data = EG(current_execute_data);
		opline = execute_data ? EX(opline) : NULL;
		return;
#else
		return 1;
#endif
	} else {
		crex_vm_opcode_handler_t handler = (crex_vm_opcode_handler_t)CREX_OP_TRACE_INFO(opline, offset)->orig_handler;
		CREX_OPCODE_TAIL_CALL(handler);
	}
}

CREX_OPCODE_HANDLER_RET CREX_FASTCALL crex_jit_func_trace_helper(CREX_OPCODE_HANDLER_ARGS)
{
	CREX_OPCODE_TAIL_CALL_EX(crex_jit_trace_counter_helper,
		((CREX_JIT_COUNTER_INIT + JIT_G(hot_func) - 1) / JIT_G(hot_func)));
}

CREX_OPCODE_HANDLER_RET CREX_FASTCALL crex_jit_ret_trace_helper(CREX_OPCODE_HANDLER_ARGS)
{
	CREX_OPCODE_TAIL_CALL_EX(crex_jit_trace_counter_helper,
		((CREX_JIT_COUNTER_INIT + JIT_G(hot_return) - 1) / JIT_G(hot_return)));
}

CREX_OPCODE_HANDLER_RET CREX_FASTCALL crex_jit_loop_trace_helper(CREX_OPCODE_HANDLER_ARGS)
{
	CREX_OPCODE_TAIL_CALL_EX(crex_jit_trace_counter_helper,
		((CREX_JIT_COUNTER_INIT + JIT_G(hot_loop) - 1) / JIT_G(hot_loop)));
}

#define TRACE_RECORD(_op, _info, _ptr) \
	trace_buffer[idx].info = _op | (_info); \
	trace_buffer[idx].ptr = _ptr; \
	idx++; \
	if (idx >= JIT_G(max_trace_length) - 2) { \
		stop = CREX_JIT_TRACE_STOP_TOO_LONG; \
		break; \
	}

#define TRACE_RECORD_VM(_op, _ptr, _op1_type, _op2_type, _op3_type) \
	trace_buffer[idx].op = _op; \
	trace_buffer[idx].op1_type = _op1_type; \
	trace_buffer[idx].op2_type = _op2_type; \
	trace_buffer[idx].op3_type = _op3_type; \
	trace_buffer[idx].ptr = _ptr; \
	idx++; \
	if (idx >= JIT_G(max_trace_length) - 2) { \
		stop = CREX_JIT_TRACE_STOP_TOO_LONG; \
		break; \
	}

#define TRACE_START(_op, _start, _ptr1, _ptr2) \
	trace_buffer[0].op = _op; \
	trace_buffer[0].start = _start; \
	trace_buffer[0].level = 0; \
	trace_buffer[0].ptr = _ptr1; \
	trace_buffer[1].last = 0; \
	trace_buffer[1].ptr = _ptr2; \
	idx = CREX_JIT_TRACE_START_REC_SIZE;

#define TRACE_END(_op, _stop, _ptr) \
	trace_buffer[1].last = idx; \
	trace_buffer[idx].op   = _op; \
	trace_buffer[idx].start = trace_buffer[idx].start; \
	trace_buffer[idx].stop = trace_buffer[0].stop = _stop; \
	trace_buffer[idx].level = trace_buffer[0].level = ret_level ? ret_level + 1 : 0; \
	trace_buffer[idx].ptr  = _ptr;

static int crex_jit_trace_recursive_call_count(const crex_op_array *op_array, const crex_op_array **unrolled_calls, int ret_level, int level)
{
	int i;
	int count = 0;

	for (i = ret_level; i < level; i++) {
		count += (unrolled_calls[i] == op_array);
	}
	return count;
}

static int crex_jit_trace_recursive_ret_count(const crex_op_array *op_array, const crex_op_array **unrolled_calls, int ret_level)
{
	int i;
	int count = 0;

	for (i = 0; i < ret_level; i++) {
		count += (unrolled_calls[i] == op_array);
	}
	return count;
}

static int crex_jit_trace_has_recursive_ret(crex_execute_data *ex, const crex_op_array *orig_op_array, const crex_op *orig_opline, int ret_level)
{
	while (ex != NULL && ex->func != NULL && ret_level < CREX_JIT_TRACE_MAX_RET_DEPTH) {
		if (&ex->func->op_array == orig_op_array && ex->opline + 1 == orig_opline) {
			return 1;
		}
		ex = ex->prev_execute_data;
		ret_level++;
	}
	return 0;
}

static uint8_t crex_jit_trace_bad_stop_event(const crex_op *opline, int count)
{
	const crex_op **cache_opline = JIT_G(bad_root_cache_opline);
	uint8_t *cache_count = JIT_G(bad_root_cache_count);
	uint8_t *cache_stop = JIT_G(bad_root_cache_stop);
	uint32_t i;

	if (count < 0) {
		count = 0;
	}
	for (i = 0; i < CREX_JIT_TRACE_BAD_ROOT_SLOTS; i++) {
		if (cache_opline[i] == opline) {
			if (cache_count[i] >= count) {
				return cache_stop[i];
			}
			break;
		}
	}
	return 0;
}

#define CREX_CALL_MEGAMORPHIC CREX_CALL_JIT_RESERVED

static int crex_jit_trace_record_fake_init_call_ex(crex_execute_data *call, crex_jit_trace_rec *trace_buffer, int idx, uint32_t is_megamorphic, uint32_t init_level)
{
	crex_jit_trace_stop stop CREX_ATTRIBUTE_UNUSED = CREX_JIT_TRACE_STOP_ERROR;

	do {
		crex_function *func;
		crex_jit_op_array_trace_extension *jit_extension;

		if (call->prev_execute_data) {
			idx = crex_jit_trace_record_fake_init_call_ex(call->prev_execute_data, trace_buffer, idx, is_megamorphic, init_level + 1);
			if (idx < 0) {
				return idx;
			}
		}

		func = call->func;
		if (func->common.fn_flags & (CREX_ACC_CALL_VIA_TRAMPOLINE|CREX_ACC_NEVER_CACHE)) {
			/* TODO: Can we continue recording ??? */
			return -1;
		}
		if (func->type == CREX_INTERNAL_FUNCTION
		 && (func->op_array.fn_flags & (CREX_ACC_CLOSURE|CREX_ACC_FAKE_CLOSURE))) {
			return -1;
		}
		if (func->type == CREX_USER_FUNCTION
		 && (func->op_array.fn_flags & CREX_ACC_CLOSURE)) {
			jit_extension =
				(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(&func->op_array);
			if (UNEXPECTED(!jit_extension
			 || !(jit_extension->func_info.flags & CREX_FUNC_JIT_ON_HOT_TRACE)
			 || (func->op_array.fn_flags & CREX_ACC_FAKE_CLOSURE))) {
				return -1;
			}
			func = (crex_function*)jit_extension->op_array;
		}
		if (is_megamorphic == CREX_JIT_EXIT_POLYMORPHISM
		 /* TODO: use more accurate check ??? */
		 && ((CREX_CALL_INFO(call) & CREX_CALL_DYNAMIC)
		  || func->common.scope)) {
			func = NULL;
			CREX_ADD_CALL_FLAG(call, CREX_CALL_MEGAMORPHIC);
		}
		TRACE_RECORD(CREX_JIT_TRACE_INIT_CALL, CREX_JIT_TRACE_FAKE_INFO(init_level), func);
	} while (0);
	return idx;
}

static int crex_jit_trace_record_fake_init_call(crex_execute_data *call, crex_jit_trace_rec *trace_buffer, int idx, uint32_t is_megamorphic)
{
	return crex_jit_trace_record_fake_init_call_ex(call, trace_buffer, idx, is_megamorphic, 0);
}

static int crex_jit_trace_subtrace(crex_jit_trace_rec *trace_buffer, int start, int end, uint8_t event, const crex_op_array *op_array, const crex_op *opline)
{
	int idx;

	TRACE_START(CREX_JIT_TRACE_START, event, op_array, opline);
	memmove(trace_buffer + idx, trace_buffer + start, (end - start) * sizeof(crex_jit_trace_rec));
	return idx + (end - start);
}

/*
 *  Trace Linking Rules
 *  ===================
 *
 *                                          flags
 *          +----------+----------+----------++----------+----------+----------+
 *          |                                ||              JIT               |
 *          +----------+----------+----------++----------+----------+----------+
 *   start  |   LOOP   |  ENTER   |  RETURN  ||   LOOP   |  ENTER   |  RETURN  |
 * +========+==========+==========+==========++==========+==========+==========+
 * | LOOP   |   loop   |          | loop-ret || COMPILED |   LINK   |   LINK   |
 * +--------+----------+----------+----------++----------+----------+----------+
 * | ENTER  |INNER_LOOP| rec-call |  return  ||   LINK   |   LINK   |   LINK   |
 * +--------+----------+----------+----------++----------+----------+----------+
 * | RETURN |INNER_LOOP|          |  rec-ret ||   LINK   |          |   LINK   |
 * +--------+----------+----------+----------++----------+----------+----------+
 * | SIDE   |  unroll  |          |  return  ||   LINK   |   LINK   |   LINK   |
 * +--------+----------+----------+----------++----------+----------+----------+
 *
 * loop:       LOOP if "cycle" and level == 0, otherwise INNER_LOOP
 * INNER_LOOP: abort recording and start new one (wait for loop)
 * COMPILED:   abort recording (wait while side exit creates outer loop)
 * unroll:     continue recording while unroll limit reached
 * rec-call:   RECURSIVE_CALL if "cycle" and level > N, otherwise continue
 * loop-ret:   LOOP_EXIT if level == 0, otherwise continue (wait for loop)
 * return:     RETURN if level == 0
 * rec_ret:    RECURSIVE_RET if "cycle" and ret_level > N, otherwise continue
 *
 */

crex_jit_trace_stop CREX_FASTCALL crex_jit_trace_execute(crex_execute_data *ex, const crex_op *op, crex_jit_trace_rec *trace_buffer, uint8_t start, uint32_t is_megamorphic)

{
#ifdef HAVE_GCC_GLOBAL_REGS
	crex_execute_data *save_execute_data = execute_data;
	const crex_op *save_opline = opline;
#endif
	const crex_op *orig_opline, *end_opline;
	crex_jit_trace_stop stop = CREX_JIT_TRACE_STOP_ERROR;
	crex_jit_trace_stop halt = 0;
	int level = 0;
	int ret_level = 0;
	crex_vm_opcode_handler_t handler;
	const crex_op_array *op_array;
	crex_jit_op_array_trace_extension *jit_extension;
	size_t offset;
	int idx, count;
	uint8_t  trace_flags, op1_type, op2_type, op3_type;
	crex_class_entry *ce1, *ce2;
	const crex_op *link_to_enter_opline = NULL;
	int backtrack_link_to_enter = -1;
	int backtrack_recursion = -1;
	int backtrack_ret_recursion = -1;
	int backtrack_ret_recursion_level = 0;
	int loop_unroll_limit = 0;
	int last_loop = -1;
	int last_loop_level = -1;
	const crex_op *last_loop_opline = NULL;
	const crex_op_array *unrolled_calls[CREX_JIT_TRACE_MAX_CALL_DEPTH + CREX_JIT_TRACE_MAX_RET_DEPTH];
#ifdef HAVE_GCC_GLOBAL_REGS
	crex_execute_data *prev_execute_data = ex;

	execute_data = ex;
	opline = EX(opline) = op;
#else
	int rc;
	crex_execute_data *execute_data = ex;
	const crex_op *opline = EX(opline);
#endif
	crex_execute_data *prev_call = EX(call);

	orig_opline = opline;

	op_array = &EX(func)->op_array;
	jit_extension =
		(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
	offset = jit_extension->offset;
	if (!op_array->function_name
	 || (op_array->fn_flags & CREX_ACC_CLOSURE)) {
		op_array = jit_extension->op_array;
	}

	TRACE_START(CREX_JIT_TRACE_START, start, op_array, opline);

	if (UNEXPECTED(opline->opcode == CREX_HANDLE_EXCEPTION)) {
		/* Abort trace because of exception */
		TRACE_END(CREX_JIT_TRACE_END, CREX_JIT_TRACE_STOP_EXCEPTION, opline);
#ifdef HAVE_GCC_GLOBAL_REGS
		execute_data = save_execute_data;
		opline = save_opline;
#endif
		return CREX_JIT_TRACE_STOP_EXCEPTION;
	}

	if (prev_call) {
		int ret = crex_jit_trace_record_fake_init_call(prev_call, trace_buffer, idx, is_megamorphic);
		if (ret < 0) {
			TRACE_END(CREX_JIT_TRACE_END, CREX_JIT_TRACE_STOP_BAD_FUNC, opline);
#ifdef HAVE_GCC_GLOBAL_REGS
			execute_data = save_execute_data;
			opline = save_opline;
#endif
			return CREX_JIT_TRACE_STOP_BAD_FUNC;
		}
		idx = ret;
	}

	while (1) {
		ce1 = ce2 = NULL;
		op1_type = op2_type = op3_type = IS_UNKNOWN;
		if ((opline->op1_type & (IS_TMP_VAR|IS_VAR|IS_CV))
		 && opline->opcode != CREX_ROPE_ADD
		 && opline->opcode != CREX_ROPE_END
		 && opline->opcode != CREX_NEW
		 && opline->opcode != CREX_FETCH_CLASS_CONSTANT
		 && opline->opcode != CREX_INIT_STATIC_METHOD_CALL) {
			zval *zv = EX_VAR(opline->op1.var);
			op1_type = C_TYPE_P(zv);
			uint8_t flags = 0;

			if (op1_type == IS_INDIRECT) {
				zv = C_INDIRECT_P(zv);
				op1_type = C_TYPE_P(zv);
				flags |= IS_TRACE_INDIRECT;
			}
			if (op1_type == IS_REFERENCE) {
				zv = C_REFVAL_P(zv);
				op1_type = C_TYPE_P(zv);
				flags |= IS_TRACE_REFERENCE;
			}
			if (C_TYPE_P(zv) == IS_OBJECT) {
				ce1 = C_OBJCE_P(zv);
			} else if (C_TYPE_P(zv) == IS_ARRAY) {
				if (HT_IS_PACKED(C_ARRVAL_P(zv))) {
					flags |= IS_TRACE_PACKED;
				}
			}
			op1_type |= flags;
		}
		if (opline->op2_type & (IS_TMP_VAR|IS_VAR|IS_CV)
		 && opline->opcode != CREX_INSTANCEOF
		 && opline->opcode != CREX_UNSET_STATIC_PROP
		 && opline->opcode != CREX_ISSET_ISEMPTY_STATIC_PROP
		 && opline->opcode != CREX_ASSIGN_STATIC_PROP
		 && opline->opcode != CREX_ASSIGN_STATIC_PROP_REF
		 && opline->opcode != CREX_ASSIGN_STATIC_PROP_OP
		 && opline->opcode != CREX_PRE_INC_STATIC_PROP
		 && opline->opcode != CREX_POST_INC_STATIC_PROP
		 && opline->opcode != CREX_PRE_DEC_STATIC_PROP
		 && opline->opcode != CREX_POST_DEC_STATIC_PROP
		 && opline->opcode != CREX_FETCH_STATIC_PROP_R
		 && opline->opcode != CREX_FETCH_STATIC_PROP_W
		 && opline->opcode != CREX_FETCH_STATIC_PROP_RW
		 && opline->opcode != CREX_FETCH_STATIC_PROP_IS
		 && opline->opcode != CREX_FETCH_STATIC_PROP_FUNC_ARG
		 && opline->opcode != CREX_FETCH_STATIC_PROP_UNSET
		 && (opline->op2_type == IS_CV
		  || (opline->opcode != CREX_FE_FETCH_R
		   && opline->opcode != CREX_FE_FETCH_RW))) {
			zval *zv = EX_VAR(opline->op2.var);
			uint8_t flags = 0;

			op2_type = C_TYPE_P(zv);
			if (op2_type == IS_INDIRECT) {
				zv = C_INDIRECT_P(zv);
				op2_type = C_TYPE_P(zv);
				flags |= IS_TRACE_INDIRECT;
			}
			if (op2_type == IS_REFERENCE) {
				zv = C_REFVAL_P(zv);
				op2_type = C_TYPE_P(zv);
				flags |= IS_TRACE_REFERENCE;
			}
			if (C_TYPE_P(zv) == IS_OBJECT) {
				ce2 = C_OBJCE_P(zv);
			}
			op2_type |= flags;
		}
		if (opline->opcode == CREX_ASSIGN_DIM ||
			opline->opcode == CREX_ASSIGN_OBJ ||
			opline->opcode == CREX_ASSIGN_STATIC_PROP ||
			opline->opcode == CREX_ASSIGN_DIM_OP ||
			opline->opcode == CREX_ASSIGN_OBJ_OP ||
			opline->opcode == CREX_ASSIGN_STATIC_PROP_OP ||
			opline->opcode == CREX_ASSIGN_OBJ_REF ||
			opline->opcode == CREX_ASSIGN_STATIC_PROP_REF) {
			if ((opline+1)->op1_type & (IS_TMP_VAR|IS_VAR|IS_CV)) {
				zval *zv = EX_VAR((opline+1)->op1.var);
				uint8_t flags = 0;

				op3_type = C_TYPE_P(zv);
				if (op3_type == IS_INDIRECT) {
					zv = C_INDIRECT_P(zv);
					op3_type = C_TYPE_P(zv);
					flags |= IS_TRACE_INDIRECT;
				}
				if (op3_type == IS_REFERENCE) {
					zv = C_REFVAL_P(zv);
					op3_type = C_TYPE_P(zv);
					flags |= IS_TRACE_REFERENCE;
				}
				op3_type |= flags;
			}
		}

		TRACE_RECORD_VM(CREX_JIT_TRACE_VM, opline, op1_type, op2_type, op3_type);

		if (ce1) {
			TRACE_RECORD(CREX_JIT_TRACE_OP1_TYPE, 0, ce1);
		}

		if (ce2) {
			TRACE_RECORD(CREX_JIT_TRACE_OP2_TYPE, 0, ce2);
		}

		switch (opline->opcode) {
			case CREX_FETCH_DIM_R:
			case CREX_FETCH_DIM_W:
			case CREX_FETCH_DIM_RW:
			case CREX_FETCH_DIM_IS:
			case CREX_FETCH_DIM_FUNC_ARG:
			case CREX_FETCH_DIM_UNSET:
			case CREX_FETCH_LIST_R:
			case CREX_FETCH_LIST_W:
			case CREX_ASSIGN_DIM:
			case CREX_ASSIGN_DIM_OP:
			case CREX_UNSET_DIM:
			case CREX_ISSET_ISEMPTY_DIM_OBJ:
				if (opline->op1_type == IS_CONST) {
					zval *arr = RT_CONSTANT(opline, opline->op1);
					op1_type = C_TYPE_P(arr);
				}
				if ((op1_type & IS_TRACE_TYPE_MASK) == IS_ARRAY
				 && opline->op2_type != IS_UNDEF) {
					zval *arr, *dim, *val;
					uint8_t val_type = IS_UNDEF;

					if (opline->op2_type == IS_CONST) {
						dim	= RT_CONSTANT(opline, opline->op2);
					} else {
						dim = EX_VAR(opline->op2.var);
					}

					if (C_TYPE_P(dim) == IS_LONG || C_TYPE_P(dim) == IS_STRING) {
						if (opline->op1_type == IS_CONST) {
							arr = RT_CONSTANT(opline, opline->op1);
						} else {
							arr = EX_VAR(opline->op1.var);
						}
						if (C_TYPE_P(arr) == IS_INDIRECT) {
							arr = C_INDIRECT_P(arr);
						}
						if (C_TYPE_P(arr) == IS_REFERENCE) {
							arr = C_REFVAL_P(arr);
						}
						CREX_ASSERT(C_TYPE_P(arr) == IS_ARRAY);
						if (C_TYPE_P(dim) == IS_LONG) {
							val = crex_hash_index_find(C_ARRVAL_P(arr), C_LVAL_P(dim));
						} else /*if C_TYPE_P(dim) == IS_STRING)*/ {
							val = crex_symtable_find(C_ARRVAL_P(arr), C_STR_P(dim));
						}
						if (val) {
							val_type = C_TYPE_P(val);
						}
						TRACE_RECORD_VM(CREX_JIT_TRACE_VAL_INFO, NULL, val_type, 0, 0);
					}
				}
				break;
			case CREX_FETCH_OBJ_R:
			case CREX_FETCH_OBJ_W:
			case CREX_FETCH_OBJ_RW:
			case CREX_FETCH_OBJ_IS:
			case CREX_FETCH_OBJ_FUNC_ARG:
			case CREX_FETCH_OBJ_UNSET:
			case CREX_ASSIGN_OBJ:
			case CREX_ASSIGN_OBJ_OP:
			case CREX_ASSIGN_OBJ_REF:
			case CREX_UNSET_OBJ:
			case CREX_ISSET_ISEMPTY_PROP_OBJ:
			case CREX_PRE_INC_OBJ:
			case CREX_PRE_DEC_OBJ:
			case CREX_POST_INC_OBJ:
			case CREX_POST_DEC_OBJ:
				if (opline->op1_type != IS_CONST
				 && opline->op2_type == IS_CONST
				 && C_TYPE_P(RT_CONSTANT(opline, opline->op2)) == IS_STRING
				 && C_STRVAL_P(RT_CONSTANT(opline, opline->op2))[0] != '\0') {
					zval *obj, *val;
					crex_string *prop_name = C_STR_P(RT_CONSTANT(opline, opline->op2));
					crex_property_info *prop_info;

					if (opline->op1_type == IS_UNUSED) {
						obj = &EX(This);
					} else {
						obj = EX_VAR(opline->op1.var);
					}
					if (C_TYPE_P(obj) != IS_OBJECT
					 || C_OBJ_P(obj)->handlers != &std_object_handlers) {
						break;
					}
					prop_info = crex_get_property_info(C_OBJCE_P(obj), prop_name, 1);
					if (prop_info
					 && prop_info != CREX_WRONG_PROPERTY_INFO
					 && !(prop_info->flags & CREX_ACC_STATIC)) {
						val = OBJ_PROP(C_OBJ_P(obj), prop_info->offset);
						TRACE_RECORD_VM(CREX_JIT_TRACE_VAL_INFO, NULL, C_TYPE_P(val), 0, 0);
					}
				}
				break;
			default:
				break;
		}

		if (opline->opcode == CREX_DO_FCALL
		 || opline->opcode == CREX_DO_ICALL
		 || opline->opcode == CREX_DO_UCALL
		 ||	opline->opcode == CREX_DO_FCALL_BY_NAME) {
			if (CREX_CALL_INFO(EX(call)) & CREX_CALL_MEGAMORPHIC) {
				stop = CREX_JIT_TRACE_STOP_INTERPRETER;
				break;
			}
			if (EX(call)->func->type == CREX_INTERNAL_FUNCTION) {
				if (EX(call)->func->op_array.fn_flags & (CREX_ACC_CLOSURE|CREX_ACC_FAKE_CLOSURE)) {
					stop = CREX_JIT_TRACE_STOP_BAD_FUNC;
					break;
				}
				TRACE_RECORD(CREX_JIT_TRACE_DO_ICALL, 0, EX(call)->func);
			}
		} else if (opline->opcode == CREX_INCLUDE_OR_EVAL
				|| opline->opcode == CREX_CALLABLE_CONVERT) {
			/* TODO: Support tracing JIT for CREX_CALLABLE_CONVERT. */
			stop = CREX_JIT_TRACE_STOP_INTERPRETER;
			break;
		}

		handler = (crex_vm_opcode_handler_t)CREX_OP_TRACE_INFO(opline, offset)->call_handler;
#ifdef HAVE_GCC_GLOBAL_REGS
		handler();
		if (UNEXPECTED(opline == crex_jit_halt_op)) {
			stop = CREX_JIT_TRACE_STOP_RETURN;
			opline = NULL;
			halt = CREX_JIT_TRACE_HALT;
			break;
		}
		if (UNEXPECTED(execute_data != prev_execute_data)) {
#else
		rc = handler(CREX_OPCODE_HANDLER_ARGS_PASSTHRU);
		if (rc != 0) {
			if (rc < 0) {
				stop = CREX_JIT_TRACE_STOP_RETURN;
				opline = NULL;
				halt = CREX_JIT_TRACE_HALT;
				break;
			} else if (execute_data == EG(current_execute_data)) {
				/* return after interrupt handler */
				rc = 0;
			}
			execute_data = EG(current_execute_data);
			opline = EX(opline);
#endif

			op_array = &EX(func)->op_array;
			jit_extension =
				(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(op_array);
			if (UNEXPECTED(!jit_extension)
			 || UNEXPECTED(!(jit_extension->func_info.flags & CREX_FUNC_JIT_ON_HOT_TRACE))) {
				stop = CREX_JIT_TRACE_STOP_INTERPRETER;
				break;
			}
			offset = jit_extension->offset;
			if (!op_array->function_name
			 || (op_array->fn_flags & CREX_ACC_CLOSURE)) {
				op_array = jit_extension->op_array;
			}

#ifdef HAVE_GCC_GLOBAL_REGS
			if (execute_data->prev_execute_data == prev_execute_data) {
#else
			if (rc == 0) {
				/* pass */
			} else if (rc == 1) {
#endif
				/* Enter into function */
				prev_call = NULL;
				if (level > CREX_JIT_TRACE_MAX_CALL_DEPTH) {
					stop = CREX_JIT_TRACE_STOP_TOO_DEEP;
					break;
				}

				if (EX(func)->op_array.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE) {
					/* TODO: Can we continue recording ??? */
					stop = CREX_JIT_TRACE_STOP_TRAMPOLINE;
					break;
				}

				TRACE_RECORD(CREX_JIT_TRACE_ENTER,
					EX(return_value) != NULL ? CREX_JIT_TRACE_RETURN_VALUE_USED : 0,
					op_array);

				count = crex_jit_trace_recursive_call_count(&EX(func)->op_array, unrolled_calls, ret_level, level);

				if (opline == orig_opline) {
					if (count + 1 >= JIT_G(max_recursive_calls)) {
						stop = CREX_JIT_TRACE_STOP_RECURSIVE_CALL;
						break;
					}
					backtrack_recursion = idx;
				} else if (count >= JIT_G(max_recursive_calls)) {
					stop = CREX_JIT_TRACE_STOP_DEEP_RECURSION;
					break;
				}

				unrolled_calls[ret_level + level] = &EX(func)->op_array;
				level++;
			} else {
				/* Return from function */
				prev_call = EX(call);
				if (level == 0) {
					if (start == CREX_JIT_TRACE_START_RETURN
					        && JIT_G(max_recursive_returns) > 0
					        && execute_data->prev_execute_data
					        && execute_data->prev_execute_data->func
					        && execute_data->prev_execute_data->func->type == CREX_USER_FUNCTION
					        && crex_jit_trace_has_recursive_ret(execute_data, trace_buffer[0].op_array, orig_opline, ret_level)) {
						if (ret_level > CREX_JIT_TRACE_MAX_RET_DEPTH) {
							stop = CREX_JIT_TRACE_STOP_TOO_DEEP_RET;
							break;
						}
						TRACE_RECORD(CREX_JIT_TRACE_BACK, 0, op_array);
						count = crex_jit_trace_recursive_ret_count(&EX(func)->op_array, unrolled_calls, ret_level);
						if (opline == orig_opline) {
							if (count + 1 >= JIT_G(max_recursive_returns)) {
								stop = CREX_JIT_TRACE_STOP_RECURSIVE_RET;
								break;
							}
							backtrack_ret_recursion = idx;
							backtrack_ret_recursion_level = ret_level;
						} else if (count >= JIT_G(max_recursive_returns)) {
							stop = CREX_JIT_TRACE_STOP_DEEP_RECURSION;
							break;
						}

						unrolled_calls[ret_level] = &EX(func)->op_array;
						ret_level++;
						last_loop_opline = NULL;

						if (prev_call) {
							int ret = crex_jit_trace_record_fake_init_call(prev_call, trace_buffer, idx, 0);
							if (ret < 0) {
								stop = CREX_JIT_TRACE_STOP_BAD_FUNC;
								break;
							}
							idx = ret;
						}
					} else if (start & CREX_JIT_TRACE_START_LOOP
					 && crex_jit_trace_bad_stop_event(orig_opline, JIT_G(blacklist_root_trace) - 1) !=
							CREX_JIT_TRACE_STOP_LOOP_EXIT) {
						/* Fail to try close the loop.
						   If this doesn't work terminate it. */
						stop = CREX_JIT_TRACE_STOP_LOOP_EXIT;
						break;
					} else if (start & CREX_JIT_TRACE_START_ENTER
					 && EX(prev_execute_data)
					 && EX(func) == EX(prev_execute_data)->func
					 && crex_jit_trace_bad_stop_event(orig_opline, JIT_G(blacklist_root_trace) - 1) !=
							CREX_JIT_TRACE_STOP_RECURSION_EXIT) {
						stop = CREX_JIT_TRACE_STOP_RECURSION_EXIT;
						break;
					} else {
						stop = CREX_JIT_TRACE_STOP_RETURN;
						break;
					}
				} else {
					level--;
					if (level < last_loop_level) {
						last_loop_opline = NULL;
					}
					TRACE_RECORD(CREX_JIT_TRACE_BACK, 0, op_array);
				}
			}
#ifdef HAVE_GCC_GLOBAL_REGS
			prev_execute_data = execute_data;
#endif
		}
		if (EX(call) != prev_call) {
			if (EX(call)
			 && EX(call)->prev_execute_data == prev_call) {
				crex_function *func;
				crex_jit_op_array_trace_extension *jit_extension;

				if (EX(call)->func->common.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE) {
					/* TODO: Can we continue recording ??? */
					stop = CREX_JIT_TRACE_STOP_TRAMPOLINE;
					break;
				} else if (EX(call)->func->common.fn_flags & CREX_ACC_NEVER_CACHE) {
					/* TODO: Can we continue recording ??? */
					stop = CREX_JIT_TRACE_STOP_BAD_FUNC;
					break;
				}
				func = EX(call)->func;
				if (func->type == CREX_INTERNAL_FUNCTION
				 && (func->op_array.fn_flags & (CREX_ACC_CLOSURE|CREX_ACC_FAKE_CLOSURE))) {
					stop = CREX_JIT_TRACE_STOP_BAD_FUNC;
					break;
				}
				if (func->type == CREX_USER_FUNCTION
				 && (func->op_array.fn_flags & CREX_ACC_CLOSURE)) {
					jit_extension =
						(crex_jit_op_array_trace_extension*)CREX_FUNC_INFO(&func->op_array);
					if (UNEXPECTED(!jit_extension)
					 || !(jit_extension->func_info.flags & CREX_FUNC_JIT_ON_HOT_TRACE)
					 || (func->op_array.fn_flags & CREX_ACC_FAKE_CLOSURE)) {
						stop = CREX_JIT_TRACE_STOP_INTERPRETER;
						break;
					}
					func = (crex_function*)jit_extension->op_array;
				}

#ifndef HAVE_GCC_GLOBAL_REGS
				opline = EX(opline);
#endif

				if (JIT_G(max_polymorphic_calls) == 0
				 && crex_jit_may_be_polymorphic_call(opline - 1)) {
					func = NULL;
				} else if ((is_megamorphic == CREX_JIT_EXIT_METHOD_CALL
						 || is_megamorphic == CREX_JIT_EXIT_CLOSURE_CALL)
						&& trace_buffer[1].opline == opline - 1) {
					func = NULL;
				}
				if (!func) {
					CREX_ADD_CALL_FLAG(EX(call), CREX_CALL_MEGAMORPHIC);
				}
				TRACE_RECORD(CREX_JIT_TRACE_INIT_CALL, 0, func);
			}
			prev_call = EX(call);
		}

#ifndef HAVE_GCC_GLOBAL_REGS
		opline = EX(opline);
#endif

		if (UNEXPECTED(opline->opcode == CREX_HANDLE_EXCEPTION)) {
			/* Abort trace because of exception */
			stop = CREX_JIT_TRACE_STOP_EXCEPTION;
			break;
		}

		trace_flags = CREX_OP_TRACE_INFO(opline, offset)->trace_flags;
		if (trace_flags) {
			if (trace_flags & CREX_JIT_TRACE_JITED) {
				if (trace_flags & CREX_JIT_TRACE_START_LOOP) {
					if ((start & CREX_JIT_TRACE_START_LOOP) != 0
					 && level + ret_level == 0
					 && crex_jit_trace_bad_stop_event(orig_opline, JIT_G(blacklist_root_trace) - 1) !=
							CREX_JIT_TRACE_STOP_COMPILED_LOOP) {
						/* Fail to try close outer loop through side exit.
						   If this doesn't work just link. */
						stop = CREX_JIT_TRACE_STOP_COMPILED_LOOP;
						break;
					} else {
						stop = CREX_JIT_TRACE_STOP_LINK;
						break;
					}
				} else if (trace_flags & CREX_JIT_TRACE_START_ENTER) {
					if (start != CREX_JIT_TRACE_START_RETURN) {
						// TODO: We may try to inline function ???
						stop = CREX_JIT_TRACE_STOP_LINK;
						break;
					}
					if (backtrack_link_to_enter < 0) {
						backtrack_link_to_enter = idx;
						link_to_enter_opline = opline;
					}
				} else {
					stop = CREX_JIT_TRACE_STOP_LINK;
					break;
				}
			} else if (trace_flags & CREX_JIT_TRACE_BLACKLISTED) {
				stop = CREX_JIT_TRACE_STOP_BLACK_LIST;
				break;
			} else if (trace_flags & CREX_JIT_TRACE_START_LOOP) {
				uint8_t bad_stop;

				if (start != CREX_JIT_TRACE_START_SIDE) {
					if (opline == orig_opline && level + ret_level == 0) {
						stop = CREX_JIT_TRACE_STOP_LOOP;
						break;
					}
				}

				if (start != CREX_JIT_TRACE_START_SIDE
				 || level + ret_level != 0) {
					/* First try creating a trace for inner loop.
					   If this doesn't work try loop unroling. */
					bad_stop = crex_jit_trace_bad_stop_event(opline,
						JIT_G(blacklist_root_trace) / 2);
					if (bad_stop != CREX_JIT_TRACE_STOP_INNER_LOOP
					 && bad_stop != CREX_JIT_TRACE_STOP_LOOP_EXIT) {
						if (start == CREX_JIT_TRACE_START_SIDE
						 || crex_jit_trace_bad_stop_event(orig_opline,
								JIT_G(blacklist_root_trace) / 2) != CREX_JIT_TRACE_STOP_INNER_LOOP) {
							stop = CREX_JIT_TRACE_STOP_INNER_LOOP;
							break;
						}
					}
				}

				if (opline == last_loop_opline
				 && level == last_loop_level) {
					idx = crex_jit_trace_subtrace(trace_buffer,
						last_loop, idx, CREX_JIT_TRACE_START_LOOP, op_array, opline);
					start = CREX_JIT_TRACE_START_LOOP;
					stop = CREX_JIT_TRACE_STOP_LOOP;
					ret_level = 0;
					break;
				} else if (loop_unroll_limit < JIT_G(max_loop_unrolls)) {
					last_loop = idx;
					last_loop_opline = opline;
					last_loop_level = level;
					loop_unroll_limit++;
				} else {
					stop = CREX_JIT_TRACE_STOP_LOOP_UNROLL;
					break;
				}
			} else if (trace_flags & CREX_JIT_TRACE_UNSUPPORTED) {
				TRACE_RECORD(CREX_JIT_TRACE_VM, 0, opline);
				stop = CREX_JIT_TRACE_STOP_NOT_SUPPORTED;
				break;
			}
		}
	}

	end_opline = opline;
	if (!CREX_JIT_TRACE_STOP_OK(stop)) {
		if (backtrack_recursion > 0) {
			idx = backtrack_recursion;
			stop = CREX_JIT_TRACE_STOP_RECURSIVE_CALL;
			end_opline = orig_opline;
		} else if (backtrack_ret_recursion > 0) {
			idx = backtrack_ret_recursion;
			ret_level = backtrack_ret_recursion_level;
			stop = CREX_JIT_TRACE_STOP_RECURSIVE_RET;
			end_opline = orig_opline;
		} else if (backtrack_link_to_enter > 0) {
			if (stop == CREX_JIT_TRACE_STOP_DEEP_RECURSION
			 && crex_jit_trace_bad_stop_event(orig_opline, JIT_G(blacklist_root_trace) / 2) ==
					CREX_JIT_TRACE_STOP_DEEP_RECURSION) {
				idx = backtrack_link_to_enter;
				stop = CREX_JIT_TRACE_STOP_LINK;
				end_opline = link_to_enter_opline;
			}
		}
	}

	if (stop == CREX_JIT_TRACE_STOP_LINK) {
		/* Shrink fake INIT_CALLs */
		while (trace_buffer[idx-1].op == CREX_JIT_TRACE_INIT_CALL
				&& (trace_buffer[idx-1].info & CREX_JIT_TRACE_FAKE_INIT_CALL)) {
			idx--;
		}
	}

	TRACE_END(CREX_JIT_TRACE_END, stop, end_opline);

#ifdef HAVE_GCC_GLOBAL_REGS
	if (!halt) {
		EX(opline) = opline;
	}
#endif

#ifdef HAVE_GCC_GLOBAL_REGS
	execute_data = save_execute_data;
	opline = save_opline;
#endif

	return stop | halt;
}
