/*
   +----------------------------------------------------------------------+
   | Crex Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Crex Technologies Ltd. (http://www.crex.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_EXECUTE_H
#define CREX_EXECUTE_H

#include "crex_compile.h"
#include "crex_hash.h"
#include "crex_operators.h"
#include "crex_variables.h"

#include <stdint.h>

BEGIN_EXTERN_C()
struct _crex_fcall_info;
CREX_API extern void (*crex_execute_ex)(crex_execute_data *execute_data);
CREX_API extern void (*crex_execute_internal)(crex_execute_data *execute_data, zval *return_value);

/* The lc_name may be stack allocated! */
CREX_API extern crex_class_entry *(*crex_autoload)(crex_string *name, crex_string *lc_name);

void init_executor(void);
void shutdown_executor(void);
void shutdown_destructors(void);
CREX_API void crex_shutdown_executor_values(bool fast_shutdown);

CREX_API void crex_init_execute_data(crex_execute_data *execute_data, crex_op_array *op_array, zval *return_value);
CREX_API void crex_init_func_execute_data(crex_execute_data *execute_data, crex_op_array *op_array, zval *return_value);
CREX_API void crex_init_code_execute_data(crex_execute_data *execute_data, crex_op_array *op_array, zval *return_value);
CREX_API void crex_execute(crex_op_array *op_array, zval *return_value);
CREX_API void execute_ex(crex_execute_data *execute_data);
CREX_API void execute_internal(crex_execute_data *execute_data, zval *return_value);
CREX_API bool crex_is_valid_class_name(crex_string *name);
CREX_API crex_class_entry *crex_lookup_class(crex_string *name);
CREX_API crex_class_entry *crex_lookup_class_ex(crex_string *name, crex_string *lcname, uint32_t flags);
CREX_API crex_class_entry *crex_get_called_scope(crex_execute_data *ex);
CREX_API crex_object *crex_get_this_object(crex_execute_data *ex);
CREX_API crex_result crex_eval_string(const char *str, zval *retval_ptr, const char *string_name);
CREX_API crex_result crex_eval_stringl(const char *str, size_t str_len, zval *retval_ptr, const char *string_name);
CREX_API crex_result crex_eval_string_ex(const char *str, zval *retval_ptr, const char *string_name, bool handle_exceptions);
CREX_API crex_result crex_eval_stringl_ex(const char *str, size_t str_len, zval *retval_ptr, const char *string_name, bool handle_exceptions);

/* export crex_pass_function to allow comparisons against it */
extern CREX_API const crex_internal_function crex_pass_function;

CREX_API CREX_COLD void CREX_FASTCALL crex_missing_arg_error(crex_execute_data *execute_data);
CREX_API CREX_COLD void CREX_FASTCALL crex_deprecated_function(const crex_function *fbc);
CREX_API CREX_COLD void CREX_FASTCALL crex_false_to_array_deprecated(void);
CREX_COLD void CREX_FASTCALL crex_param_must_be_ref(const crex_function *func, uint32_t arg_num);
CREX_API CREX_COLD void CREX_FASTCALL crex_use_resource_as_offset(const zval *dim);

CREX_API bool CREX_FASTCALL crex_verify_ref_assignable_zval(crex_reference *ref, zval *zv, bool strict);

typedef enum {
	CREX_VERIFY_PROP_ASSIGNABLE_BY_REF_CONTEXT_ASSIGNMENT,
	CREX_VERIFY_PROP_ASSIGNABLE_BY_REF_CONTEXT_MAGIC_GET,
} crex_verify_prop_assignable_by_ref_context;
CREX_API bool CREX_FASTCALL crex_verify_prop_assignable_by_ref_ex(const crex_property_info *prop_info, zval *orig_val, bool strict, crex_verify_prop_assignable_by_ref_context context);
CREX_API bool CREX_FASTCALL crex_verify_prop_assignable_by_ref(const crex_property_info *prop_info, zval *orig_val, bool strict);

CREX_API CREX_COLD void crex_throw_ref_type_error_zval(const crex_property_info *prop, const zval *zv);
CREX_API CREX_COLD void crex_throw_ref_type_error_type(const crex_property_info *prop1, const crex_property_info *prop2, const zval *zv);
CREX_API CREX_COLD zval* CREX_FASTCALL crex_undefined_offset_write(HashTable *ht, crex_long lval);
CREX_API CREX_COLD zval* CREX_FASTCALL crex_undefined_index_write(HashTable *ht, crex_string *offset);
CREX_API CREX_COLD void crex_wrong_string_offset_error(void);

CREX_API CREX_COLD void CREX_FASTCALL crex_readonly_property_modification_error(const crex_property_info *info);
CREX_API CREX_COLD void CREX_FASTCALL crex_readonly_property_indirect_modification_error(const crex_property_info *info);

CREX_API CREX_COLD void CREX_FASTCALL crex_invalid_class_constant_type_error(uint8_t type);

CREX_API CREX_COLD void CREX_FASTCALL crex_object_released_while_assigning_to_property_error(const crex_property_info *info);

CREX_API CREX_COLD void CREX_FASTCALL crex_cannot_add_element(void);

CREX_API bool crex_verify_scalar_type_hint(uint32_t type_mask, zval *arg, bool strict, bool is_internal_arg);
CREX_API CREX_COLD void crex_verify_arg_error(
		const crex_function *zf, const crex_arg_info *arg_info, uint32_t arg_num, zval *value);
CREX_API CREX_COLD void crex_verify_return_error(
		const crex_function *zf, zval *value);
CREX_API CREX_COLD void crex_verify_never_error(
		const crex_function *zf);
CREX_API bool crex_verify_ref_array_assignable(crex_reference *ref);
CREX_API bool crex_check_user_type_slow(
		crex_type *type, zval *arg, crex_reference *ref, void **cache_slot, bool is_return_type);

#if CREX_DEBUG
CREX_API bool crex_internal_call_should_throw(crex_function *fbc, crex_execute_data *call);
CREX_API CREX_COLD void crex_internal_call_arginfo_violation(crex_function *fbc);
CREX_API bool crex_verify_internal_return_type(crex_function *zf, zval *ret);
#endif

#define CREX_REF_TYPE_SOURCES(ref) \
	(ref)->sources

#define CREX_REF_HAS_TYPE_SOURCES(ref) \
	(CREX_REF_TYPE_SOURCES(ref).ptr != NULL)

#define CREX_REF_FIRST_SOURCE(ref) \
	(CREX_PROPERTY_INFO_SOURCE_IS_LIST((ref)->sources.list) \
		? CREX_PROPERTY_INFO_SOURCE_TO_LIST((ref)->sources.list)->ptr[0] \
		: (ref)->sources.ptr)


CREX_API void CREX_FASTCALL crex_ref_add_type_source(crex_property_info_source_list *source_list, crex_property_info *prop);
CREX_API void CREX_FASTCALL crex_ref_del_type_source(crex_property_info_source_list *source_list, const crex_property_info *prop);

CREX_API zval* crex_assign_to_typed_ref(zval *variable_ptr, zval *value, uint8_t value_type, bool strict);
CREX_API zval* crex_assign_to_typed_ref_ex(zval *variable_ptr, zval *value, uint8_t value_type, bool strict, crex_refcounted **garbage_ptr);

static crex_always_inline void crex_copy_to_variable(zval *variable_ptr, zval *value, uint8_t value_type)
{
	crex_refcounted *ref = NULL;

	if (CREX_CONST_COND(value_type & (IS_VAR|IS_CV), 1) && C_ISREF_P(value)) {
		ref = C_COUNTED_P(value);
		value = C_REFVAL_P(value);
	}

	ZVAL_COPY_VALUE(variable_ptr, value);
	if (CREX_CONST_COND(value_type  == IS_CONST, 0)) {
		if (UNEXPECTED(C_OPT_REFCOUNTED_P(variable_ptr))) {
			C_ADDREF_P(variable_ptr);
		}
	} else if (value_type & (IS_CONST|IS_CV)) {
		if (C_OPT_REFCOUNTED_P(variable_ptr)) {
			C_ADDREF_P(variable_ptr);
		}
	} else if (CREX_CONST_COND(value_type == IS_VAR, 1) && UNEXPECTED(ref)) {
		if (UNEXPECTED(GC_DELREF(ref) == 0)) {
			efree_size(ref, sizeof(crex_reference));
		} else if (C_OPT_REFCOUNTED_P(variable_ptr)) {
			C_ADDREF_P(variable_ptr);
		}
	}
}

static crex_always_inline zval* crex_assign_to_variable(zval *variable_ptr, zval *value, uint8_t value_type, bool strict)
{
	do {
		if (UNEXPECTED(C_REFCOUNTED_P(variable_ptr))) {
			crex_refcounted *garbage;

			if (C_ISREF_P(variable_ptr)) {
				if (UNEXPECTED(CREX_REF_HAS_TYPE_SOURCES(C_REF_P(variable_ptr)))) {
					return crex_assign_to_typed_ref(variable_ptr, value, value_type, strict);
				}

				variable_ptr = C_REFVAL_P(variable_ptr);
				if (EXPECTED(!C_REFCOUNTED_P(variable_ptr))) {
					break;
				}
			}
			garbage = C_COUNTED_P(variable_ptr);
			crex_copy_to_variable(variable_ptr, value, value_type);
			GC_DTOR_NO_REF(garbage);
			return variable_ptr;
		}
	} while (0);

	crex_copy_to_variable(variable_ptr, value, value_type);
	return variable_ptr;
}

static crex_always_inline zval* crex_assign_to_variable_ex(zval *variable_ptr, zval *value, crex_uchar value_type, bool strict, crex_refcounted **garbage_ptr)
{
	do {
		if (UNEXPECTED(C_REFCOUNTED_P(variable_ptr))) {
			if (C_ISREF_P(variable_ptr)) {
				if (UNEXPECTED(CREX_REF_HAS_TYPE_SOURCES(C_REF_P(variable_ptr)))) {
					return crex_assign_to_typed_ref_ex(variable_ptr, value, value_type, strict, garbage_ptr);
				}

				variable_ptr = C_REFVAL_P(variable_ptr);
				if (EXPECTED(!C_REFCOUNTED_P(variable_ptr))) {
					break;
				}
			}
			*garbage_ptr = C_COUNTED_P(variable_ptr);
		}
	} while (0);

	crex_copy_to_variable(variable_ptr, value, value_type);
	return variable_ptr;
}

CREX_API crex_result CREX_FASTCALL zval_update_constant(zval *pp);
CREX_API crex_result CREX_FASTCALL zval_update_constant_ex(zval *pp, crex_class_entry *scope);
CREX_API crex_result CREX_FASTCALL zval_update_constant_with_ctx(zval *pp, crex_class_entry *scope, crex_ast_evaluate_ctx *ctx);

/* dedicated Crex executor functions - do not use! */
struct _crex_vm_stack {
	zval *top;
	zval *end;
	crex_vm_stack prev;
};

/* Ensure the correct alignment before slots calculation */
CREX_STATIC_ASSERT(CREX_MM_ALIGNED_SIZE(sizeof(zval)) == sizeof(zval),
                   "zval must be aligned by CREX_MM_ALIGNMENT");
/* A number of call frame slots (zvals) reserved for _crex_vm_stack. */
#define CREX_VM_STACK_HEADER_SLOTS \
	((sizeof(struct _crex_vm_stack) + sizeof(zval) - 1) / sizeof(zval))

#define CREX_VM_STACK_ELEMENTS(stack) \
	(((zval*)(stack)) + CREX_VM_STACK_HEADER_SLOTS)

/*
 * In general in RELEASE build CREX_ASSERT() must be zero-cost, but for some
 * reason, GCC generated worse code, performing CSE on assertion code and the
 * following "slow path" and moving memory read operations from slow path into
 * common header. This made a degradation for the fast path.
 * The following "#if CREX_DEBUG" eliminates it.
 */
#if CREX_DEBUG
# define CREX_ASSERT_VM_STACK(stack) CREX_ASSERT(stack->top > (zval *) stack && stack->end > (zval *) stack && stack->top <= stack->end)
# define CREX_ASSERT_VM_STACK_GLOBAL CREX_ASSERT(EG(vm_stack_top) > (zval *) EG(vm_stack) && EG(vm_stack_end) > (zval *) EG(vm_stack) && EG(vm_stack_top) <= EG(vm_stack_end))
#else
# define CREX_ASSERT_VM_STACK(stack)
# define CREX_ASSERT_VM_STACK_GLOBAL
#endif

CREX_API void crex_vm_stack_init(void);
CREX_API void crex_vm_stack_init_ex(size_t page_size);
CREX_API void crex_vm_stack_destroy(void);
CREX_API void* crex_vm_stack_extend(size_t size);

static crex_always_inline crex_vm_stack crex_vm_stack_new_page(size_t size, crex_vm_stack prev) {
	crex_vm_stack page = (crex_vm_stack)emalloc(size);

	page->top = CREX_VM_STACK_ELEMENTS(page);
	page->end = (zval*)((char*)page + size);
	page->prev = prev;
	return page;
}

static crex_always_inline void crex_vm_init_call_frame(crex_execute_data *call, uint32_t call_info, crex_function *func, uint32_t num_args, void *object_or_called_scope)
{
	CREX_ASSERT(!func->common.scope || object_or_called_scope);
	call->func = func;
	C_PTR(call->This) = object_or_called_scope;
	CREX_CALL_INFO(call) = call_info;
	CREX_CALL_NUM_ARGS(call) = num_args;
}

static crex_always_inline crex_execute_data *crex_vm_stack_push_call_frame_ex(uint32_t used_stack, uint32_t call_info, crex_function *func, uint32_t num_args, void *object_or_called_scope)
{
	crex_execute_data *call = (crex_execute_data*)EG(vm_stack_top);

	CREX_ASSERT_VM_STACK_GLOBAL;

	if (UNEXPECTED(used_stack > (size_t)(((char*)EG(vm_stack_end)) - (char*)call))) {
		call = (crex_execute_data*)crex_vm_stack_extend(used_stack);
		CREX_ASSERT_VM_STACK_GLOBAL;
		crex_vm_init_call_frame(call, call_info | CREX_CALL_ALLOCATED, func, num_args, object_or_called_scope);
		return call;
	} else {
		EG(vm_stack_top) = (zval*)((char*)call + used_stack);
		crex_vm_init_call_frame(call, call_info, func, num_args, object_or_called_scope);
		return call;
	}
}

static crex_always_inline uint32_t crex_vm_calc_used_stack(uint32_t num_args, crex_function *func)
{
	uint32_t used_stack = CREX_CALL_FRAME_SLOT + num_args + func->common.T;

	if (EXPECTED(CREX_USER_CODE(func->type))) {
		used_stack += func->op_array.last_var - MIN(func->op_array.num_args, num_args);
	}
	return used_stack * sizeof(zval);
}

static crex_always_inline crex_execute_data *crex_vm_stack_push_call_frame(uint32_t call_info, crex_function *func, uint32_t num_args, void *object_or_called_scope)
{
	uint32_t used_stack = crex_vm_calc_used_stack(num_args, func);

	return crex_vm_stack_push_call_frame_ex(used_stack, call_info,
		func, num_args, object_or_called_scope);
}

static crex_always_inline void crex_vm_stack_free_extra_args_ex(uint32_t call_info, crex_execute_data *call)
{
	if (UNEXPECTED(call_info & CREX_CALL_FREE_EXTRA_ARGS)) {
		uint32_t count = CREX_CALL_NUM_ARGS(call) - call->func->op_array.num_args;
		zval *p = CREX_CALL_VAR_NUM(call, call->func->op_array.last_var + call->func->op_array.T);
		do {
			i_zval_ptr_dtor(p);
			p++;
		} while (--count);
 	}
}

static crex_always_inline void crex_vm_stack_free_extra_args(crex_execute_data *call)
{
	crex_vm_stack_free_extra_args_ex(CREX_CALL_INFO(call), call);
}

static crex_always_inline void crex_vm_stack_free_args(crex_execute_data *call)
{
	uint32_t num_args = CREX_CALL_NUM_ARGS(call);

	if (EXPECTED(num_args > 0)) {
		zval *p = CREX_CALL_ARG(call, 1);

		do {
			zval_ptr_dtor_nogc(p);
			p++;
		} while (--num_args);
	}
}

static crex_always_inline void crex_vm_stack_free_call_frame_ex(uint32_t call_info, crex_execute_data *call)
{
	CREX_ASSERT_VM_STACK_GLOBAL;

	if (UNEXPECTED(call_info & CREX_CALL_ALLOCATED)) {
		crex_vm_stack p = EG(vm_stack);
		crex_vm_stack prev = p->prev;

		CREX_ASSERT(call == (crex_execute_data*)CREX_VM_STACK_ELEMENTS(EG(vm_stack)));
		EG(vm_stack_top) = prev->top;
		EG(vm_stack_end) = prev->end;
		EG(vm_stack) = prev;
		efree(p);
	} else {
		EG(vm_stack_top) = (zval*)call;
	}

	CREX_ASSERT_VM_STACK_GLOBAL;
}

static crex_always_inline void crex_vm_stack_free_call_frame(crex_execute_data *call)
{
	crex_vm_stack_free_call_frame_ex(CREX_CALL_INFO(call), call);
}

crex_execute_data *crex_vm_stack_copy_call_frame(
	crex_execute_data *call, uint32_t passed_args, uint32_t additional_args);

static crex_always_inline void crex_vm_stack_extend_call_frame(
	crex_execute_data **call, uint32_t passed_args, uint32_t additional_args)
{
	if (EXPECTED((uint32_t)(EG(vm_stack_end) - EG(vm_stack_top)) > additional_args)) {
		EG(vm_stack_top) += additional_args;
	} else {
		*call = crex_vm_stack_copy_call_frame(*call, passed_args, additional_args);
	}
}

CREX_API void CREX_FASTCALL crex_free_extra_named_params(crex_array *extra_named_params);

/* services */
CREX_API const char *get_active_class_name(const char **space);
CREX_API const char *get_active_function_name(void);
CREX_API const char *get_active_function_arg_name(uint32_t arg_num);
CREX_API const char *get_function_arg_name(const crex_function *func, uint32_t arg_num);
CREX_API crex_string *get_active_function_or_method_name(void);
CREX_API crex_string *get_function_or_method_name(const crex_function *func);
CREX_API const char *crex_get_executed_filename(void);
CREX_API crex_string *crex_get_executed_filename_ex(void);
CREX_API uint32_t crex_get_executed_lineno(void);
CREX_API crex_class_entry *crex_get_executed_scope(void);
CREX_API bool crex_is_executing(void);
CREX_API CREX_COLD void CREX_FASTCALL crex_cannot_pass_by_reference(uint32_t arg_num);

CREX_API void crex_set_timeout(crex_long seconds, bool reset_signals);
CREX_API void crex_unset_timeout(void);
CREX_API CREX_NORETURN void CREX_FASTCALL crex_timeout(void);
CREX_API crex_class_entry *crex_fetch_class(crex_string *class_name, uint32_t fetch_type);
CREX_API crex_class_entry *crex_fetch_class_with_scope(crex_string *class_name, uint32_t fetch_type, crex_class_entry *scope);
CREX_API crex_class_entry *crex_fetch_class_by_name(crex_string *class_name, crex_string *lcname, uint32_t fetch_type);

CREX_API crex_function * CREX_FASTCALL crex_fetch_function(crex_string *name);
CREX_API crex_function * CREX_FASTCALL crex_fetch_function_str(const char *name, size_t len);
CREX_API void CREX_FASTCALL crex_init_func_run_time_cache(crex_op_array *op_array);

CREX_API void crex_fetch_dimension_const(zval *result, zval *container, zval *dim, int type);

CREX_API zval* crex_get_compiled_variable_value(const crex_execute_data *execute_data_ptr, uint32_t var);

CREX_API bool crex_gcc_global_regs(void);

#define CREX_USER_OPCODE_CONTINUE   0 /* execute next opcode */
#define CREX_USER_OPCODE_RETURN     1 /* exit from executor (return from function) */
#define CREX_USER_OPCODE_DISPATCH   2 /* call original opcode handler */
#define CREX_USER_OPCODE_ENTER      3 /* enter into new op_array without recursion */
#define CREX_USER_OPCODE_LEAVE      4 /* return to calling op_array within the same executor */

#define CREX_USER_OPCODE_DISPATCH_TO 0x100 /* call original handler of returned opcode */

CREX_API crex_result crex_set_user_opcode_handler(uint8_t opcode, user_opcode_handler_t handler);
CREX_API user_opcode_handler_t crex_get_user_opcode_handler(uint8_t opcode);

CREX_API zval *crex_get_zval_ptr(const crex_op *opline, int op_type, const znode_op *node, const crex_execute_data *execute_data);

CREX_API void crex_clean_and_cache_symbol_table(crex_array *symbol_table);
CREX_API void CREX_FASTCALL crex_free_compiled_variables(crex_execute_data *execute_data);
CREX_API void crex_unfinished_calls_gc(crex_execute_data *execute_data, crex_execute_data *call, uint32_t op_num, crex_get_gc_buffer *buf);
CREX_API void crex_cleanup_unfinished_execution(crex_execute_data *execute_data, uint32_t op_num, uint32_t catch_op_num);
CREX_API CREX_ATTRIBUTE_DEPRECATED HashTable *crex_unfinished_execution_gc(crex_execute_data *execute_data, crex_execute_data *call, crex_get_gc_buffer *gc_buffer);
CREX_API HashTable *crex_unfinished_execution_gc_ex(crex_execute_data *execute_data, crex_execute_data *call, crex_get_gc_buffer *gc_buffer, bool suspended_by_yield);

zval * CREX_FASTCALL crex_handle_named_arg(
		crex_execute_data **call_ptr, crex_string *arg_name,
		uint32_t *arg_num_ptr, void **cache_slot);
CREX_API crex_result CREX_FASTCALL crex_handle_undef_args(crex_execute_data *call);

#define CACHE_ADDR(num) \
	((void**)((char*)EX(run_time_cache) + (num)))

#define CACHED_PTR(num) \
	((void**)((char*)EX(run_time_cache) + (num)))[0]

#define CACHE_PTR(num, ptr) do { \
		((void**)((char*)EX(run_time_cache) + (num)))[0] = (ptr); \
	} while (0)

#define CACHED_POLYMORPHIC_PTR(num, ce) \
	(EXPECTED(((void**)((char*)EX(run_time_cache) + (num)))[0] == (void*)(ce)) ? \
		((void**)((char*)EX(run_time_cache) + (num)))[1] : \
		NULL)

#define CACHE_POLYMORPHIC_PTR(num, ce, ptr) do { \
		void **slot = (void**)((char*)EX(run_time_cache) + (num)); \
		slot[0] = (ce); \
		slot[1] = (ptr); \
	} while (0)

#define CACHED_PTR_EX(slot) \
	(slot)[0]

#define CACHE_PTR_EX(slot, ptr) do { \
		(slot)[0] = (ptr); \
	} while (0)

#define CACHED_POLYMORPHIC_PTR_EX(slot, ce) \
	(EXPECTED((slot)[0] == (ce)) ? (slot)[1] : NULL)

#define CACHE_POLYMORPHIC_PTR_EX(slot, ce, ptr) do { \
		(slot)[0] = (ce); \
		(slot)[1] = (ptr); \
	} while (0)

#define CACHE_SPECIAL (1<<0)

#define IS_SPECIAL_CACHE_VAL(ptr) \
	(((uintptr_t)(ptr)) & CACHE_SPECIAL)

#define ENCODE_SPECIAL_CACHE_NUM(num) \
	((void*)((((uintptr_t)(num)) << 1) | CACHE_SPECIAL))

#define DECODE_SPECIAL_CACHE_NUM(ptr) \
	(((uintptr_t)(ptr)) >> 1)

#define ENCODE_SPECIAL_CACHE_PTR(ptr) \
	((void*)(((uintptr_t)(ptr)) | CACHE_SPECIAL))

#define DECODE_SPECIAL_CACHE_PTR(ptr) \
	((void*)(((uintptr_t)(ptr)) & ~CACHE_SPECIAL))

#define SKIP_EXT_OPLINE(opline) do { \
		while (UNEXPECTED((opline)->opcode >= CREX_EXT_STMT \
			&& (opline)->opcode <= CREX_TICKS)) {     \
			(opline)--;                                  \
		}                                                \
	} while (0)

#define CREX_CLASS_HAS_TYPE_HINTS(ce) ((ce->ce_flags & CREX_ACC_HAS_TYPE_HINTS) == CREX_ACC_HAS_TYPE_HINTS)
#define CREX_CLASS_HAS_READONLY_PROPS(ce) ((ce->ce_flags & CREX_ACC_HAS_READONLY_PROPS) == CREX_ACC_HAS_READONLY_PROPS)


CREX_API bool crex_verify_class_constant_type(crex_class_constant *c, const crex_string *name, zval *constant);
CREX_COLD void crex_verify_class_constant_type_error(const crex_class_constant *c, const crex_string *name, const zval *constant);

CREX_API bool crex_verify_property_type(const crex_property_info *info, zval *property, bool strict);
CREX_COLD void crex_verify_property_type_error(const crex_property_info *info, const zval *property);
CREX_COLD void crex_magic_get_property_type_inconsistency_error(const crex_property_info *info, const zval *property);

#define CREX_REF_ADD_TYPE_SOURCE(ref, source) \
	crex_ref_add_type_source(&CREX_REF_TYPE_SOURCES(ref), source)

#define CREX_REF_DEL_TYPE_SOURCE(ref, source) \
	crex_ref_del_type_source(&CREX_REF_TYPE_SOURCES(ref), source)

#define CREX_REF_FOREACH_TYPE_SOURCES(ref, prop) do { \
		crex_property_info_source_list *_source_list = &CREX_REF_TYPE_SOURCES(ref); \
		crex_property_info **_prop, **_end; \
		crex_property_info_list *_list; \
		if (_source_list->ptr) { \
			if (CREX_PROPERTY_INFO_SOURCE_IS_LIST(_source_list->list)) { \
				_list = CREX_PROPERTY_INFO_SOURCE_TO_LIST(_source_list->list); \
				_prop = _list->ptr; \
				_end = _list->ptr + _list->num; \
			} else { \
				_prop = &_source_list->ptr; \
				_end = _prop + 1; \
			} \
			for (; _prop < _end; _prop++) { \
				prop = *_prop; \

#define CREX_REF_FOREACH_TYPE_SOURCES_END() \
			} \
		} \
	} while (0)

CREX_COLD void crex_match_unhandled_error(const zval *value);

static crex_always_inline void *crex_get_bad_ptr(void)
{
	CREX_UNREACHABLE();
	return NULL;
}

END_EXTERN_C()

#endif /* CREX_EXECUTE_H */
