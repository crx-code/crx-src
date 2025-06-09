/*
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
  | Authors: Aaron Piotrowski <aaron@trowski.com>                         |
  +----------------------------------------------------------------------+
*/

#include "crx_test.h"
#include "fiber.h"
#include "fiber_arginfo.h"
#include "crex_fibers.h"
#include "crex_exceptions.h"

static crex_class_entry *crex_test_fiber_class;
static crex_object_handlers crex_test_fiber_handlers;

static crex_fiber_transfer crex_test_fiber_switch_to(crex_fiber_context *context, zval *value, bool exception)
{
	crex_fiber_transfer transfer = {
			.context = context,
			.flags = exception ? CREX_FIBER_TRANSFER_FLAG_ERROR : 0,
	};

	if (value) {
		ZVAL_COPY(&transfer.value, value);
	} else {
		ZVAL_NULL(&transfer.value);
	}

	crex_fiber_switch_context(&transfer);

	/* Forward bailout into current fiber. */
	if (UNEXPECTED(transfer.flags & CREX_FIBER_TRANSFER_FLAG_BAILOUT)) {
		crex_bailout();
	}

	return transfer;
}

static crex_fiber_transfer crex_test_fiber_resume(crex_test_fiber *fiber, zval *value, bool exception)
{
	crex_test_fiber *previous = ZT_G(active_fiber);

	fiber->caller = EG(current_fiber_context);
	ZT_G(active_fiber) = fiber;

	crex_fiber_transfer transfer = crex_test_fiber_switch_to(fiber->previous, value, exception);

	ZT_G(active_fiber) = previous;

	return transfer;
}

static crex_fiber_transfer crex_test_fiber_suspend(crex_test_fiber *fiber, zval *value)
{
	CREX_ASSERT(fiber->caller != NULL);

	crex_fiber_context *caller = fiber->caller;
	fiber->previous = EG(current_fiber_context);
	fiber->caller = NULL;

	return crex_test_fiber_switch_to(caller, value, false);
}

static CREX_STACK_ALIGNED void crex_test_fiber_execute(crex_fiber_transfer *transfer)
{
	crex_test_fiber *fiber = ZT_G(active_fiber);
	zval retval;

	crex_execute_data *execute_data;

	EG(vm_stack) = NULL;
	transfer->flags = 0;

	crex_first_try {
		crex_vm_stack stack = crex_vm_stack_new_page(CREX_FIBER_VM_STACK_SIZE, NULL);
		EG(vm_stack) = stack;
		EG(vm_stack_top) = stack->top + CREX_CALL_FRAME_SLOT;
		EG(vm_stack_end) = stack->end;
		EG(vm_stack_page_size) = CREX_FIBER_VM_STACK_SIZE;

		execute_data = (crex_execute_data *) stack->top;

		memset(execute_data, 0, sizeof(crex_execute_data));

		EG(current_execute_data) = execute_data;
		EG(jit_trace_num) = 0;

#ifdef CREX_CHECK_STACK_LIMIT
		EG(stack_base) = crex_fiber_stack_base(fiber->context.stack);
		EG(stack_limit) = crex_fiber_stack_limit(fiber->context.stack);
#endif
		fiber->fci.retval = &retval;

		crex_call_function(&fiber->fci, &fiber->fci_cache);

		zval_ptr_dtor(&fiber->result); // Destroy param from symmetric coroutine.
		zval_ptr_dtor(&fiber->fci.function_name);

		if (EG(exception)) {
			if (!(fiber->flags & CREX_FIBER_FLAG_DESTROYED)
				|| !(crex_is_graceful_exit(EG(exception)) || crex_is_unwind_exit(EG(exception)))
			) {
				fiber->flags |= CREX_FIBER_FLAG_THREW;
				transfer->flags = CREX_FIBER_TRANSFER_FLAG_ERROR;

				ZVAL_OBJ_COPY(&transfer->value, EG(exception));
			}

			crex_clear_exception();
		} else {
			ZVAL_COPY_VALUE(&fiber->result, &retval);
			ZVAL_COPY(&transfer->value, &fiber->result);
		}
	} crex_catch {
		fiber->flags |= CREX_FIBER_FLAG_BAILOUT;
		transfer->flags = CREX_FIBER_TRANSFER_FLAG_BAILOUT;
	} crex_end_try();

	crex_vm_stack_destroy();

	if (fiber->target) {
		crex_fiber_context *target = &fiber->target->context;
		crex_fiber_init_context(target, crex_test_fiber_class, crex_test_fiber_execute, EG(fiber_stack_size));
		transfer->context = target;

		ZVAL_COPY(&fiber->target->result, &fiber->result);
		fiber->target->fci.params = &fiber->target->result;
		fiber->target->fci.param_count = 1;

		fiber->target->caller = fiber->caller;
		ZT_G(active_fiber) = fiber->target;
	} else {
		transfer->context = fiber->caller;
	}

	fiber->caller = NULL;
}

static crex_object *crex_test_fiber_object_create(crex_class_entry *ce)
{
	crex_test_fiber *fiber;

	fiber = emalloc(sizeof(crex_test_fiber));
	memset(fiber, 0, sizeof(crex_test_fiber));

	crex_object_std_init(&fiber->std, ce);

	return &fiber->std;
}

static void crex_test_fiber_object_destroy(crex_object *object)
{
	crex_test_fiber *fiber = (crex_test_fiber *) object;

	if (fiber->context.status != CREX_FIBER_STATUS_SUSPENDED) {
		return;
	}

	crex_object *exception = EG(exception);
	EG(exception) = NULL;

	fiber->flags |= CREX_FIBER_FLAG_DESTROYED;

	crex_fiber_transfer transfer = crex_test_fiber_resume(fiber, NULL, false);

	if (transfer.flags & CREX_FIBER_TRANSFER_FLAG_ERROR) {
		EG(exception) = C_OBJ(transfer.value);

		if (!exception && EG(current_execute_data) && EG(current_execute_data)->func
			&& CREX_USER_CODE(EG(current_execute_data)->func->common.type)) {
			crex_rethrow_exception(EG(current_execute_data));
		}

		crex_exception_set_previous(EG(exception), exception);

		if (!EG(current_execute_data)) {
			crex_exception_error(EG(exception), E_ERROR);
		}
	} else {
		zval_ptr_dtor(&transfer.value);
		EG(exception) = exception;
	}
}

static void crex_test_fiber_object_free(crex_object *object)
{
	crex_test_fiber *fiber = (crex_test_fiber *) object;

	if (fiber->context.status == CREX_FIBER_STATUS_INIT) {
		// Fiber was never started, so we need to release the reference to the callback.
		zval_ptr_dtor(&fiber->fci.function_name);
	}

	if (fiber->target) {
		OBJ_RELEASE(&fiber->target->std);
	}

	zval_ptr_dtor(&fiber->result);

	crex_object_std_dtor(&fiber->std);
}

static crex_always_inline void delegate_transfer_result(
	crex_test_fiber *fiber, crex_fiber_transfer *transfer, INTERNAL_FUNCTION_PARAMETERS
) {
	if (transfer->flags & CREX_FIBER_TRANSFER_FLAG_ERROR) {
		crex_throw_exception_internal(C_OBJ(transfer->value));
		RETURN_THROWS();
	}

	if (fiber->context.status == CREX_FIBER_STATUS_DEAD) {
		zval_ptr_dtor(&transfer->value);
		RETURN_NULL();
	}

	RETURN_COPY_VALUE(&transfer->value);
}

static CREX_METHOD(_CrexTestFiber, __main)
{
	crex_test_fiber *fiber = (crex_test_fiber *) C_OBJ_P(getThis());

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_FUNC(fiber->fci, fiber->fci_cache)
	CREX_PARSE_PARAMETERS_END();

	// Keep a reference to closures or callable objects while the fiber is running.
	C_TRY_ADDREF(fiber->fci.function_name);
}

static CREX_METHOD(_CrexTestFiber, start)
{
	crex_test_fiber *fiber = (crex_test_fiber *) C_OBJ_P(getThis());
	zval *params;
	uint32_t param_count;
	crex_array *named_params;

	CREX_PARSE_PARAMETERS_START(0, -1)
		C_PARAM_VARIADIC_WITH_NAMED(params, param_count, named_params);
	CREX_PARSE_PARAMETERS_END();

	CREX_ASSERT(fiber->context.status == CREX_FIBER_STATUS_INIT);

	if (fiber->previous != NULL) {
		crex_throw_error(NULL, "Cannot start a fiber that is the target of another fiber");
		RETURN_THROWS();
	}

	fiber->fci.params = params;
	fiber->fci.param_count = param_count;
	fiber->fci.named_params = named_params;

	crex_fiber_init_context(&fiber->context, crex_test_fiber_class, crex_test_fiber_execute, EG(fiber_stack_size));

	fiber->previous = &fiber->context;

	crex_fiber_transfer transfer = crex_test_fiber_resume(fiber, NULL, false);

	delegate_transfer_result(fiber, &transfer, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static CREX_METHOD(_CrexTestFiber, suspend)
{
	zval *value = NULL;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_ZVAL(value);
	CREX_PARSE_PARAMETERS_END();

	crex_test_fiber *fiber = ZT_G(active_fiber);

	CREX_ASSERT(fiber);

	crex_fiber_transfer transfer = crex_test_fiber_suspend(fiber, value);

	if (fiber->flags & CREX_FIBER_FLAG_DESTROYED) {
		// This occurs when the test fiber is GC'ed while suspended.
		zval_ptr_dtor(&transfer.value);
		crex_throw_graceful_exit();
		RETURN_THROWS();
	}

	delegate_transfer_result(fiber, &transfer, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static CREX_METHOD(_CrexTestFiber, resume)
{
	crex_test_fiber *fiber;
	zval *value = NULL;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_ZVAL(value);
	CREX_PARSE_PARAMETERS_END();

	fiber = (crex_test_fiber *) C_OBJ_P(getThis());

	if (UNEXPECTED(fiber->context.status != CREX_FIBER_STATUS_SUSPENDED || fiber->caller != NULL)) {
		crex_throw_error(NULL, "Cannot resume a fiber that is not suspended");
		RETURN_THROWS();
	}

	crex_fiber_transfer transfer = crex_test_fiber_resume(fiber, value, false);

	delegate_transfer_result(fiber, &transfer, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static CREX_METHOD(_CrexTestFiber, pipeTo)
{
	crex_fcall_info fci;
	crex_fcall_info_cache fci_cache;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_FUNC(fci, fci_cache)
	CREX_PARSE_PARAMETERS_END();

	crex_test_fiber *fiber = (crex_test_fiber *) C_OBJ_P(getThis());
	crex_test_fiber *target = (crex_test_fiber *) crex_test_fiber_class->create_object(crex_test_fiber_class);

	target->fci = fci;
	target->fci_cache = fci_cache;
	C_TRY_ADDREF(target->fci.function_name);

	target->previous = &fiber->context;

	if (fiber->target) {
		OBJ_RELEASE(&fiber->target->std);
	}

	fiber->target = target;

	RETURN_OBJ_COPY(&target->std);
}

void crex_test_fiber_init(void)
{
	crex_test_fiber_class = register_class__CrexTestFiber();
	crex_test_fiber_class->create_object = crex_test_fiber_object_create;
	crex_test_fiber_class->default_object_handlers = &crex_test_fiber_handlers;

	crex_test_fiber_handlers = std_object_handlers;
	crex_test_fiber_handlers.dtor_obj = crex_test_fiber_object_destroy;
	crex_test_fiber_handlers.free_obj = crex_test_fiber_object_free;
}
