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
   | Authors: Aaron Piotrowski <aaron@trowski.com>                        |
   |          Martin Schröder <m.schroeder2007@gmail.com>                 |
   +----------------------------------------------------------------------+
*/

#include "crex.h"
#include "crex_API.h"
#include "crex_ini.h"
#include "crex_vm.h"
#include "crex_exceptions.h"
#include "crex_builtin_functions.h"
#include "crex_observer.h"
#include "crex_mmap.h"
#include "crex_compile.h"
#include "crex_closures.h"

#include "crex_fibers.h"
#include "crex_fibers_arginfo.h"

#ifdef HAVE_VALGRIND
# include <valgrind/valgrind.h>
#endif

#ifdef CREX_FIBER_UCONTEXT
# include <ucontext.h>
#endif

#ifndef CREX_WIN32
# include <unistd.h>
# include <sys/mman.h>
# include <limits.h>

# if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#  define MAP_ANONYMOUS MAP_ANON
# endif

/* FreeBSD require a first (i.e. addr) argument of mmap(2) is not NULL
 * if MAP_STACK is passed.
 * http://www.FreeBSD.org/cgi/query-pr.cgi?pr=158755 */
# if !defined(MAP_STACK) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#  undef MAP_STACK
#  define MAP_STACK 0
# endif

# ifndef MAP_FAILED
#  define MAP_FAILED ((void * ) -1)
# endif
#endif

#ifdef __SANITIZE_ADDRESS__
# include <sanitizer/common_interface_defs.h>
#endif

# if defined __CET__
#  include <cet.h>
#  define SHSTK_ENABLED (__CET__ & 0x2)
#  define BOOST_CONTEXT_SHADOW_STACK (SHSTK_ENABLED && SHADOW_STACK_SYSCALL)
#  define __NR_map_shadow_stack 451
# ifndef SHADOW_STACK_SET_TOKEN
#  define SHADOW_STACK_SET_TOKEN 0x1
#endif
#endif

/* Encapsulates the fiber C stack with extension for debugging tools. */
struct _crex_fiber_stack {
	void *pointer;
	size_t size;

#ifdef HAVE_VALGRIND
	unsigned int valgrind_stack_id;
#endif

#ifdef __SANITIZE_ADDRESS__
	const void *asan_pointer;
	size_t asan_size;
#endif

#ifdef CREX_FIBER_UCONTEXT
	/* Embedded ucontext to avoid unnecessary memory allocations. */
	ucontext_t ucontext;
#elif BOOST_CONTEXT_SHADOW_STACK
	/* Shadow stack: base, size */
	void *ss_base;
	size_t ss_size;
#endif
};

/* Crex VM state that needs to be captured / restored during fiber context switch. */
typedef struct _crex_fiber_vm_state {
	crex_vm_stack vm_stack;
	zval *vm_stack_top;
	zval *vm_stack_end;
	size_t vm_stack_page_size;
	crex_execute_data *current_execute_data;
	int error_reporting;
	uint32_t jit_trace_num;
	JMP_BUF *bailout;
	crex_fiber *active_fiber;
#ifdef CREX_CHECK_STACK_LIMIT
	void *stack_base;
	void *stack_limit;
#endif
} crex_fiber_vm_state;

static crex_always_inline void crex_fiber_capture_vm_state(crex_fiber_vm_state *state)
{
	state->vm_stack = EG(vm_stack);
	state->vm_stack_top = EG(vm_stack_top);
	state->vm_stack_end = EG(vm_stack_end);
	state->vm_stack_page_size = EG(vm_stack_page_size);
	state->current_execute_data = EG(current_execute_data);
	state->error_reporting = EG(error_reporting);
	state->jit_trace_num = EG(jit_trace_num);
	state->bailout = EG(bailout);
	state->active_fiber = EG(active_fiber);
#ifdef CREX_CHECK_STACK_LIMIT
	state->stack_base = EG(stack_base);
	state->stack_limit = EG(stack_limit);
#endif
}

static crex_always_inline void crex_fiber_restore_vm_state(crex_fiber_vm_state *state)
{
	EG(vm_stack) = state->vm_stack;
	EG(vm_stack_top) = state->vm_stack_top;
	EG(vm_stack_end) = state->vm_stack_end;
	EG(vm_stack_page_size) = state->vm_stack_page_size;
	EG(current_execute_data) = state->current_execute_data;
	EG(error_reporting) = state->error_reporting;
	EG(jit_trace_num) = state->jit_trace_num;
	EG(bailout) = state->bailout;
	EG(active_fiber) = state->active_fiber;
#ifdef CREX_CHECK_STACK_LIMIT
	EG(stack_base) = state->stack_base;
	EG(stack_limit) = state->stack_limit;
#endif
}

#ifdef CREX_FIBER_UCONTEXT
CREX_TLS crex_fiber_transfer *transfer_data;
#else
/* boost_context_data is our customized definition of struct transfer_t as
 * provided by boost.context in fcontext.hpp:
 *
 * typedef void* fcontext_t;
 *
 * struct transfer_t {
 *     fcontext_t fctx;
 *     void *data;
 * }; */

typedef struct {
	void *handle;
	crex_fiber_transfer *transfer;
} boost_context_data;

/* These functions are defined in assembler files provided by boost.context (located in "Crex/asm"). */
extern void *make_fcontext(void *sp, size_t size, void (*fn)(boost_context_data));
extern CREX_INDIRECT_RETURN boost_context_data jump_fcontext(void *to, crex_fiber_transfer *transfer);
#endif

CREX_API crex_class_entry *crex_ce_fiber;
static crex_class_entry *crex_ce_fiber_error;

static crex_object_handlers crex_fiber_handlers;

static crex_function crex_fiber_function = { CREX_INTERNAL_FUNCTION };

CREX_TLS uint32_t crex_fiber_switch_blocking = 0;

#define CREX_FIBER_DEFAULT_PAGE_SIZE 4096

static size_t crex_fiber_get_page_size(void)
{
	static size_t page_size = 0;

	if (!page_size) {
		page_size = crex_get_page_size();
		if (!page_size || (page_size & (page_size - 1))) {
			/* anyway, we have to return a valid result */
			page_size = CREX_FIBER_DEFAULT_PAGE_SIZE;
		}
	}

	return page_size;
}

static crex_fiber_stack *crex_fiber_stack_allocate(size_t size)
{
	void *pointer;
	const size_t page_size = crex_fiber_get_page_size();
	const size_t minimum_stack_size = page_size + CREX_FIBER_GUARD_PAGES * page_size;

	if (size < minimum_stack_size) {
		crex_throw_exception_ex(NULL, 0, "Fiber stack size is too small, it needs to be at least %zu bytes", minimum_stack_size);
		return NULL;
	}

	const size_t stack_size = (size + page_size - 1) / page_size * page_size;
	const size_t alloc_size = stack_size + CREX_FIBER_GUARD_PAGES * page_size;

#ifdef CREX_WIN32
	pointer = VirtualAlloc(0, alloc_size, MEM_COMMIT, PAGE_READWRITE);

	if (!pointer) {
		DWORD err = GetLastError();
		char *errmsg = crx_win32_error_to_msg(err);
		crex_throw_exception_ex(NULL, 0, "Fiber stack allocate failed: VirtualAlloc failed: [0x%08lx] %s", err, errmsg[0] ? errmsg : "Unknown");
		crx_win32_error_msg_free(errmsg);
		return NULL;
	}

# if CREX_FIBER_GUARD_PAGES
	DWORD protect;

	if (!VirtualProtect(pointer, CREX_FIBER_GUARD_PAGES * page_size, PAGE_READWRITE | PAGE_GUARD, &protect)) {
		DWORD err = GetLastError();
		char *errmsg = crx_win32_error_to_msg(err);
		crex_throw_exception_ex(NULL, 0, "Fiber stack protect failed: VirtualProtect failed: [0x%08lx] %s", err, errmsg[0] ? errmsg : "Unknown");
		crx_win32_error_msg_free(errmsg);
		VirtualFree(pointer, 0, MEM_RELEASE);
		return NULL;
	}
# endif
#else
	pointer = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);

	if (pointer == MAP_FAILED) {
		crex_throw_exception_ex(NULL, 0, "Fiber stack allocate failed: mmap failed: %s (%d)", strerror(errno), errno);
		return NULL;
	}

	crex_mmap_set_name(pointer, alloc_size, "crex_fiber_stack");

# if CREX_FIBER_GUARD_PAGES
	if (mprotect(pointer, CREX_FIBER_GUARD_PAGES * page_size, PROT_NONE) < 0) {
		crex_throw_exception_ex(NULL, 0, "Fiber stack protect failed: mprotect failed: %s (%d)", strerror(errno), errno);
		munmap(pointer, alloc_size);
		return NULL;
	}
# endif
#endif

	crex_fiber_stack *stack = emalloc(sizeof(crex_fiber_stack));

	stack->pointer = (void *) ((uintptr_t) pointer + CREX_FIBER_GUARD_PAGES * page_size);
	stack->size = stack_size;

#if !defined(CREX_FIBER_UCONTEXT) && BOOST_CONTEXT_SHADOW_STACK
	/* shadow stack saves ret address only, need less space */
	stack->ss_size= stack_size >> 5;

	/* align shadow stack to 8 bytes. */
	stack->ss_size = (stack->ss_size + 7) & ~7;

	/* issue syscall to create shadow stack for the new fcontext */
	/* SHADOW_STACK_SET_TOKEN option will put "restore token" on the new shadow stack */
	stack->ss_base = (void *)syscall(__NR_map_shadow_stack, 0, stack->ss_size, SHADOW_STACK_SET_TOKEN);

	if (stack->ss_base == MAP_FAILED) {
		crex_throw_exception_ex(NULL, 0, "Fiber shadow stack allocate failed: mmap failed: %s (%d)", strerror(errno), errno);
		return NULL;
	}
#endif

#ifdef VALGRIND_STACK_REGISTER
	uintptr_t base = (uintptr_t) stack->pointer;
	stack->valgrind_stack_id = VALGRIND_STACK_REGISTER(base, base + stack->size);
#endif

#ifdef __SANITIZE_ADDRESS__
	stack->asan_pointer = stack->pointer;
	stack->asan_size = stack->size;
#endif

	return stack;
}

static void crex_fiber_stack_free(crex_fiber_stack *stack)
{
#ifdef VALGRIND_STACK_DEREGISTER
	VALGRIND_STACK_DEREGISTER(stack->valgrind_stack_id);
#endif

	const size_t page_size = crex_fiber_get_page_size();

	void *pointer = (void *) ((uintptr_t) stack->pointer - CREX_FIBER_GUARD_PAGES * page_size);

#ifdef CREX_WIN32
	VirtualFree(pointer, 0, MEM_RELEASE);
#else
	munmap(pointer, stack->size + CREX_FIBER_GUARD_PAGES * page_size);
#endif

#if !defined(CREX_FIBER_UCONTEXT) && BOOST_CONTEXT_SHADOW_STACK
	munmap(stack->ss_base, stack->ss_size);
#endif

	efree(stack);
}

#ifdef CREX_CHECK_STACK_LIMIT
CREX_API void* crex_fiber_stack_limit(crex_fiber_stack *stack)
{
	crex_ulong reserve = EG(reserved_stack_size);

#ifdef __APPLE__
	/* On Apple Clang, the stack probing function ___chkstk_darwin incorrectly
	 * probes a location that is twice the entered function's stack usage away
	 * from the stack pointer, when using an alternative stack.
	 * https://openradar.appspot.com/radar?id=5497722702397440
	 */
	reserve = reserve * 2;
#endif

	/* stack->pointer is the end of the stack */
	return (int8_t*)stack->pointer + reserve;
}

CREX_API void* crex_fiber_stack_base(crex_fiber_stack *stack)
{
	return (void*)((uintptr_t)stack->pointer + stack->size);
}
#endif

#ifdef CREX_FIBER_UCONTEXT
static CREX_NORETURN void crex_fiber_trampoline(void)
#else
static CREX_NORETURN void crex_fiber_trampoline(boost_context_data data)
#endif
{
	/* Initialize transfer struct with a copy of passed data. */
#ifdef CREX_FIBER_UCONTEXT
	crex_fiber_transfer transfer = *transfer_data;
#else
	crex_fiber_transfer transfer = *data.transfer;
#endif

	crex_fiber_context *from = transfer.context;

#ifdef __SANITIZE_ADDRESS__
	__sanitizer_finish_switch_fiber(NULL, &from->stack->asan_pointer, &from->stack->asan_size);
#endif

#ifndef CREX_FIBER_UCONTEXT
	/* Get the context that resumed us and update its handle to allow for symmetric coroutines. */
	from->handle = data.handle;
#endif

	/* Ensure that previous fiber will be cleaned up (needed by symmetric coroutines). */
	if (from->status == CREX_FIBER_STATUS_DEAD) {
		crex_fiber_destroy_context(from);
	}

	crex_fiber_context *context = EG(current_fiber_context);

	context->function(&transfer);
	context->status = CREX_FIBER_STATUS_DEAD;

	/* Final context switch, the fiber must not be resumed afterwards! */
	crex_fiber_switch_context(&transfer);

	/* Abort here because we are in an inconsistent program state. */
	abort();
}

CREX_API void crex_fiber_switch_block(void)
{
	++crex_fiber_switch_blocking;
}

CREX_API void crex_fiber_switch_unblock(void)
{
	CREX_ASSERT(crex_fiber_switch_blocking && "Fiber switching was not blocked");
	--crex_fiber_switch_blocking;
}

CREX_API bool crex_fiber_switch_blocked(void)
{
	return crex_fiber_switch_blocking;
}

CREX_API crex_result crex_fiber_init_context(crex_fiber_context *context, void *kind, crex_fiber_coroutine coroutine, size_t stack_size)
{
	context->stack = crex_fiber_stack_allocate(stack_size);

	if (UNEXPECTED(!context->stack)) {
		return FAILURE;
	}

#ifdef CREX_FIBER_UCONTEXT
	ucontext_t *handle = &context->stack->ucontext;

	getcontext(handle);

	handle->uc_stack.ss_size = context->stack->size;
	handle->uc_stack.ss_sp = context->stack->pointer;
	handle->uc_stack.ss_flags = 0;
	handle->uc_link = NULL;

	makecontext(handle, (void (*)(void)) crex_fiber_trampoline, 0);

	context->handle = handle;
#else
	// Stack grows down, calculate the top of the stack. make_fcontext then shifts pointer to lower 16-byte boundary.
	void *stack = (void *) ((uintptr_t) context->stack->pointer + context->stack->size);

#if BOOST_CONTEXT_SHADOW_STACK
	// pass the shadow stack pointer to make_fcontext
	// i.e., link the new shadow stack with the new fcontext
	// TODO should be a better way?
	*((unsigned long*) (stack - 8)) = (unsigned long)context->stack->ss_base + context->stack->ss_size;
#endif

	context->handle = make_fcontext(stack, context->stack->size, crex_fiber_trampoline);
	CREX_ASSERT(context->handle != NULL && "make_fcontext() never returns NULL");
#endif

	context->kind = kind;
	context->function = coroutine;

	// Set status in case memory has not been zeroed.
	context->status = CREX_FIBER_STATUS_INIT;

	crex_observer_fiber_init_notify(context);

	return SUCCESS;
}

CREX_API void crex_fiber_destroy_context(crex_fiber_context *context)
{
	crex_observer_fiber_destroy_notify(context);

	if (context->cleanup) {
		context->cleanup(context);
	}

	crex_fiber_stack_free(context->stack);
}

CREX_API void crex_fiber_switch_context(crex_fiber_transfer *transfer)
{
	crex_fiber_context *from = EG(current_fiber_context);
	crex_fiber_context *to = transfer->context;
	crex_fiber_vm_state state;

	CREX_ASSERT(to && to->handle && to->status != CREX_FIBER_STATUS_DEAD && "Invalid fiber context");
	CREX_ASSERT(from && "From fiber context must be present");
	CREX_ASSERT(to != from && "Cannot switch into the running fiber context");

	/* Assert that all error transfers hold a Throwable value. */
	CREX_ASSERT((
		!(transfer->flags & CREX_FIBER_TRANSFER_FLAG_ERROR) ||
		(C_TYPE(transfer->value) == IS_OBJECT && (
			crex_is_unwind_exit(C_OBJ(transfer->value)) ||
			crex_is_graceful_exit(C_OBJ(transfer->value)) ||
			instanceof_function(C_OBJCE(transfer->value), crex_ce_throwable)
		))
	) && "Error transfer requires a throwable value");

	crex_observer_fiber_switch_notify(from, to);

	crex_fiber_capture_vm_state(&state);

	to->status = CREX_FIBER_STATUS_RUNNING;

	if (EXPECTED(from->status == CREX_FIBER_STATUS_RUNNING)) {
		from->status = CREX_FIBER_STATUS_SUSPENDED;
	}

	/* Update transfer context with the current fiber before switching. */
	transfer->context = from;

	EG(current_fiber_context) = to;

#ifdef __SANITIZE_ADDRESS__
	void *fake_stack = NULL;
	__sanitizer_start_switch_fiber(
		from->status != CREX_FIBER_STATUS_DEAD ? &fake_stack : NULL,
		to->stack->asan_pointer,
		to->stack->asan_size);
#endif

#ifdef CREX_FIBER_UCONTEXT
	transfer_data = transfer;

	swapcontext(from->handle, to->handle);

	/* Copy transfer struct because it might live on the other fiber's stack that will eventually be destroyed. */
	*transfer = *transfer_data;
#else
	boost_context_data data = jump_fcontext(to->handle, transfer);

	/* Copy transfer struct because it might live on the other fiber's stack that will eventually be destroyed. */
	*transfer = *data.transfer;
#endif

	to = transfer->context;

#ifndef CREX_FIBER_UCONTEXT
	/* Get the context that resumed us and update its handle to allow for symmetric coroutines. */
	to->handle = data.handle;
#endif

#ifdef __SANITIZE_ADDRESS__
	__sanitizer_finish_switch_fiber(fake_stack, &to->stack->asan_pointer, &to->stack->asan_size);
#endif

	EG(current_fiber_context) = from;

	crex_fiber_restore_vm_state(&state);

	/* Destroy prior context if it has been marked as dead. */
	if (to->status == CREX_FIBER_STATUS_DEAD) {
		crex_fiber_destroy_context(to);
	}
}

static void crex_fiber_cleanup(crex_fiber_context *context)
{
	crex_fiber *fiber = crex_fiber_from_context(context);

	crex_vm_stack current_stack = EG(vm_stack);
	EG(vm_stack) = fiber->vm_stack;
	crex_vm_stack_destroy();
	EG(vm_stack) = current_stack;
	fiber->execute_data = NULL;
	fiber->stack_bottom = NULL;
	fiber->caller = NULL;
}

static CREX_STACK_ALIGNED void crex_fiber_execute(crex_fiber_transfer *transfer)
{
	CREX_ASSERT(C_TYPE(transfer->value) == IS_NULL && "Initial transfer value to fiber context must be NULL");
	CREX_ASSERT(!transfer->flags && "No flags should be set on initial transfer");

	crex_fiber *fiber = EG(active_fiber);

	/* Determine the current error_reporting ini setting. */
	crex_long error_reporting = INI_INT("error_reporting");
	/* If error_reporting is 0 and not explicitly set to 0, INI_STR returns a null pointer. */
	if (!error_reporting && !INI_STR("error_reporting")) {
		error_reporting = E_ALL;
	}

	EG(vm_stack) = NULL;

	crex_first_try {
		crex_vm_stack stack = crex_vm_stack_new_page(CREX_FIBER_VM_STACK_SIZE, NULL);
		EG(vm_stack) = stack;
		EG(vm_stack_top) = stack->top + CREX_CALL_FRAME_SLOT;
		EG(vm_stack_end) = stack->end;
		EG(vm_stack_page_size) = CREX_FIBER_VM_STACK_SIZE;

		fiber->execute_data = (crex_execute_data *) stack->top;
		fiber->stack_bottom = fiber->execute_data;

		memset(fiber->execute_data, 0, sizeof(crex_execute_data));

		fiber->execute_data->func = &crex_fiber_function;
		fiber->stack_bottom->prev_execute_data = EG(current_execute_data);

		EG(current_execute_data) = fiber->execute_data;
		EG(jit_trace_num) = 0;
		EG(error_reporting) = error_reporting;

#ifdef CREX_CHECK_STACK_LIMIT
		EG(stack_base) = crex_fiber_stack_base(fiber->context.stack);
		EG(stack_limit) = crex_fiber_stack_limit(fiber->context.stack);
#endif

		fiber->fci.retval = &fiber->result;

		crex_call_function(&fiber->fci, &fiber->fci_cache);

		/* Cleanup callback and unset field to prevent GC / duplicate dtor issues. */
		zval_ptr_dtor(&fiber->fci.function_name);
		ZVAL_UNDEF(&fiber->fci.function_name);

		if (EG(exception)) {
			if (!(fiber->flags & CREX_FIBER_FLAG_DESTROYED)
				|| !(crex_is_graceful_exit(EG(exception)) || crex_is_unwind_exit(EG(exception)))
			) {
				fiber->flags |= CREX_FIBER_FLAG_THREW;
				transfer->flags = CREX_FIBER_TRANSFER_FLAG_ERROR;

				ZVAL_OBJ_COPY(&transfer->value, EG(exception));
			}

			crex_clear_exception();
		}
	} crex_catch {
		fiber->flags |= CREX_FIBER_FLAG_BAILOUT;
		transfer->flags = CREX_FIBER_TRANSFER_FLAG_BAILOUT;
	} crex_end_try();

	fiber->context.cleanup = &crex_fiber_cleanup;
	fiber->vm_stack = EG(vm_stack);

	transfer->context = fiber->caller;
}

/* Handles forwarding of result / error from a transfer into the running fiber. */
static crex_always_inline void crex_fiber_delegate_transfer_result(
	crex_fiber_transfer *transfer, INTERNAL_FUNCTION_PARAMETERS
) {
	if (transfer->flags & CREX_FIBER_TRANSFER_FLAG_ERROR) {
		/* Use internal throw to skip the Throwable-check that would fail for (graceful) exit. */
		crex_throw_exception_internal(C_OBJ(transfer->value));
		RETURN_THROWS();
	}

	RETURN_COPY_VALUE(&transfer->value);
}

static crex_always_inline crex_fiber_transfer crex_fiber_switch_to(
	crex_fiber_context *context, zval *value, bool exception
) {
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
		EG(active_fiber) = NULL;
		crex_bailout();
	}

	return transfer;
}

static crex_always_inline crex_fiber_transfer crex_fiber_resume(crex_fiber *fiber, zval *value, bool exception)
{
	crex_fiber *previous = EG(active_fiber);

	if (previous) {
		previous->execute_data = EG(current_execute_data);
	}

	fiber->caller = EG(current_fiber_context);
	EG(active_fiber) = fiber;

	crex_fiber_transfer transfer = crex_fiber_switch_to(fiber->previous, value, exception);

	EG(active_fiber) = previous;

	return transfer;
}

static crex_always_inline crex_fiber_transfer crex_fiber_suspend(crex_fiber *fiber, zval *value)
{
	CREX_ASSERT(fiber->caller != NULL);

	crex_fiber_context *caller = fiber->caller;
	fiber->previous = EG(current_fiber_context);
	fiber->caller = NULL;
	fiber->execute_data = EG(current_execute_data);

	return crex_fiber_switch_to(caller, value, false);
}

static crex_object *crex_fiber_object_create(crex_class_entry *ce)
{
	crex_fiber *fiber = emalloc(sizeof(crex_fiber));
	memset(fiber, 0, sizeof(crex_fiber));

	crex_object_std_init(&fiber->std, ce);
	return &fiber->std;
}

static void crex_fiber_object_destroy(crex_object *object)
{
	crex_fiber *fiber = (crex_fiber *) object;

	if (fiber->context.status != CREX_FIBER_STATUS_SUSPENDED) {
		return;
	}

	crex_object *exception = EG(exception);
	EG(exception) = NULL;

	zval graceful_exit;
	ZVAL_OBJ(&graceful_exit, crex_create_graceful_exit());

	fiber->flags |= CREX_FIBER_FLAG_DESTROYED;

	crex_fiber_transfer transfer = crex_fiber_resume(fiber, &graceful_exit, true);

	zval_ptr_dtor(&graceful_exit);

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

static void crex_fiber_object_free(crex_object *object)
{
	crex_fiber *fiber = (crex_fiber *) object;

	zval_ptr_dtor(&fiber->fci.function_name);
	zval_ptr_dtor(&fiber->result);

	crex_object_std_dtor(&fiber->std);
}

static HashTable *crex_fiber_object_gc(crex_object *object, zval **table, int *num)
{
	crex_fiber *fiber = (crex_fiber *) object;
	crex_get_gc_buffer *buf = crex_get_gc_buffer_create();

	crex_get_gc_buffer_add_zval(buf, &fiber->fci.function_name);
	crex_get_gc_buffer_add_zval(buf, &fiber->result);

	if (fiber->context.status != CREX_FIBER_STATUS_SUSPENDED || fiber->caller != NULL) {
		crex_get_gc_buffer_use(buf, table, num);
		return NULL;
	}

	HashTable *lastSymTable = NULL;
	crex_execute_data *ex = fiber->execute_data;
	for (; ex; ex = ex->prev_execute_data) {
		HashTable *symTable = crex_unfinished_execution_gc_ex(ex, ex->func && CREX_USER_CODE(ex->func->type) ? ex->call : NULL, buf, false);
		if (symTable) {
			if (lastSymTable) {
				zval *val;
				CREX_HASH_FOREACH_VAL(lastSymTable, val) {
					if (EXPECTED(C_TYPE_P(val) == IS_INDIRECT)) {
						val = C_INDIRECT_P(val);
					}
					crex_get_gc_buffer_add_zval(buf, val);
				} CREX_HASH_FOREACH_END();
			}
			lastSymTable = symTable;
		}
	}

	crex_get_gc_buffer_use(buf, table, num);

	return lastSymTable;
}

CREX_METHOD(Fiber, __main)
{
	crex_fcall_info fci;
	crex_fcall_info_cache fcc;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_FUNC(fci, fcc)
	CREX_PARSE_PARAMETERS_END();

	crex_fiber *fiber = (crex_fiber *) C_OBJ_P(CREX_THIS);

	if (UNEXPECTED(fiber->context.status != CREX_FIBER_STATUS_INIT || C_TYPE(fiber->fci.function_name) != IS_UNDEF)) {
		crex_throw_error(crex_ce_fiber_error, "Cannot call constructor twice");
		RETURN_THROWS();
	}

	fiber->fci = fci;
	fiber->fci_cache = fcc;

	// Keep a reference to closures or callable objects while the fiber is running.
	C_TRY_ADDREF(fiber->fci.function_name);
}

CREX_METHOD(Fiber, start)
{
	crex_fiber *fiber = (crex_fiber *) C_OBJ_P(CREX_THIS);

	CREX_PARSE_PARAMETERS_START(0, -1)
		C_PARAM_VARIADIC_WITH_NAMED(fiber->fci.params, fiber->fci.param_count, fiber->fci.named_params);
	CREX_PARSE_PARAMETERS_END();

	if (UNEXPECTED(crex_fiber_switch_blocked())) {
		crex_throw_error(crex_ce_fiber_error, "Cannot switch fibers in current execution context");
		RETURN_THROWS();
	}

	if (fiber->context.status != CREX_FIBER_STATUS_INIT) {
		crex_throw_error(crex_ce_fiber_error, "Cannot start a fiber that has already been started");
		RETURN_THROWS();
	}

	if (crex_fiber_init_context(&fiber->context, crex_ce_fiber, crex_fiber_execute, EG(fiber_stack_size)) == FAILURE) {
		RETURN_THROWS();
	}

	fiber->previous = &fiber->context;

	crex_fiber_transfer transfer = crex_fiber_resume(fiber, NULL, false);

	crex_fiber_delegate_transfer_result(&transfer, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

CREX_METHOD(Fiber, suspend)
{
	zval *value = NULL;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_ZVAL(value);
	CREX_PARSE_PARAMETERS_END();

	crex_fiber *fiber = EG(active_fiber);

	if (UNEXPECTED(!fiber)) {
		crex_throw_error(crex_ce_fiber_error, "Cannot suspend outside of a fiber");
		RETURN_THROWS();
	}

	if (UNEXPECTED(fiber->flags & CREX_FIBER_FLAG_DESTROYED)) {
		crex_throw_error(crex_ce_fiber_error, "Cannot suspend in a force-closed fiber");
		RETURN_THROWS();
	}

	if (UNEXPECTED(crex_fiber_switch_blocked())) {
		crex_throw_error(crex_ce_fiber_error, "Cannot switch fibers in current execution context");
		RETURN_THROWS();
	}

	CREX_ASSERT(fiber->context.status == CREX_FIBER_STATUS_RUNNING || fiber->context.status == CREX_FIBER_STATUS_SUSPENDED);

	fiber->stack_bottom->prev_execute_data = NULL;

	crex_fiber_transfer transfer = crex_fiber_suspend(fiber, value);

	crex_fiber_delegate_transfer_result(&transfer, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

CREX_METHOD(Fiber, resume)
{
	crex_fiber *fiber;
	zval *value = NULL;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_ZVAL(value);
	CREX_PARSE_PARAMETERS_END();

	if (UNEXPECTED(crex_fiber_switch_blocked())) {
		crex_throw_error(crex_ce_fiber_error, "Cannot switch fibers in current execution context");
		RETURN_THROWS();
	}

	fiber = (crex_fiber *) C_OBJ_P(CREX_THIS);

	if (UNEXPECTED(fiber->context.status != CREX_FIBER_STATUS_SUSPENDED || fiber->caller != NULL)) {
		crex_throw_error(crex_ce_fiber_error, "Cannot resume a fiber that is not suspended");
		RETURN_THROWS();
	}

	fiber->stack_bottom->prev_execute_data = EG(current_execute_data);

	crex_fiber_transfer transfer = crex_fiber_resume(fiber, value, false);

	crex_fiber_delegate_transfer_result(&transfer, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

CREX_METHOD(Fiber, throw)
{
	crex_fiber *fiber;
	zval *exception;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_OBJECT_OF_CLASS(exception, crex_ce_throwable)
	CREX_PARSE_PARAMETERS_END();

	if (UNEXPECTED(crex_fiber_switch_blocked())) {
		crex_throw_error(crex_ce_fiber_error, "Cannot switch fibers in current execution context");
		RETURN_THROWS();
	}

	fiber = (crex_fiber *) C_OBJ_P(CREX_THIS);

	if (UNEXPECTED(fiber->context.status != CREX_FIBER_STATUS_SUSPENDED || fiber->caller != NULL)) {
		crex_throw_error(crex_ce_fiber_error, "Cannot resume a fiber that is not suspended");
		RETURN_THROWS();
	}

	fiber->stack_bottom->prev_execute_data = EG(current_execute_data);

	crex_fiber_transfer transfer = crex_fiber_resume(fiber, exception, true);

	crex_fiber_delegate_transfer_result(&transfer, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

CREX_METHOD(Fiber, isStarted)
{
	crex_fiber *fiber;

	CREX_PARSE_PARAMETERS_NONE();

	fiber = (crex_fiber *) C_OBJ_P(CREX_THIS);

	RETURN_BOOL(fiber->context.status != CREX_FIBER_STATUS_INIT);
}

CREX_METHOD(Fiber, isSuspended)
{
	crex_fiber *fiber;

	CREX_PARSE_PARAMETERS_NONE();

	fiber = (crex_fiber *) C_OBJ_P(CREX_THIS);

	RETURN_BOOL(fiber->context.status == CREX_FIBER_STATUS_SUSPENDED && fiber->caller == NULL);
}

CREX_METHOD(Fiber, isRunning)
{
	crex_fiber *fiber;

	CREX_PARSE_PARAMETERS_NONE();

	fiber = (crex_fiber *) C_OBJ_P(CREX_THIS);

	RETURN_BOOL(fiber->context.status == CREX_FIBER_STATUS_RUNNING || fiber->caller != NULL);
}

CREX_METHOD(Fiber, isTerminated)
{
	crex_fiber *fiber;

	CREX_PARSE_PARAMETERS_NONE();

	fiber = (crex_fiber *) C_OBJ_P(CREX_THIS);

	RETURN_BOOL(fiber->context.status == CREX_FIBER_STATUS_DEAD);
}

CREX_METHOD(Fiber, getReturn)
{
	crex_fiber *fiber;
	const char *message;

	CREX_PARSE_PARAMETERS_NONE();

	fiber = (crex_fiber *) C_OBJ_P(CREX_THIS);

	if (fiber->context.status == CREX_FIBER_STATUS_DEAD) {
		if (fiber->flags & CREX_FIBER_FLAG_THREW) {
			message = "The fiber threw an exception";
		} else if (fiber->flags & CREX_FIBER_FLAG_BAILOUT) {
			message = "The fiber exited with a fatal error";
		} else {
			RETURN_COPY_DEREF(&fiber->result);
		}
	} else if (fiber->context.status == CREX_FIBER_STATUS_INIT) {
		message = "The fiber has not been started";
	} else {
		message = "The fiber has not returned";
	}

	crex_throw_error(crex_ce_fiber_error, "Cannot get fiber return value: %s", message);
	RETURN_THROWS();
}

CREX_METHOD(Fiber, getCurrent)
{
	CREX_PARSE_PARAMETERS_NONE();

	crex_fiber *fiber = EG(active_fiber);

	if (!fiber) {
		RETURN_NULL();
	}

	RETURN_OBJ_COPY(&fiber->std);
}

CREX_METHOD(FiberError, __main)
{
	crex_throw_error(
		NULL,
		"The \"%s\" class is reserved for internal use and cannot be manually instantiated",
		ZSTR_VAL(C_OBJCE_P(CREX_THIS)->name)
	);
}


void crex_register_fiber_ce(void)
{
	crex_ce_fiber = register_class_Fiber();
	crex_ce_fiber->create_object = crex_fiber_object_create;
	crex_ce_fiber->default_object_handlers = &crex_fiber_handlers;

	crex_fiber_handlers = std_object_handlers;
	crex_fiber_handlers.dtor_obj = crex_fiber_object_destroy;
	crex_fiber_handlers.free_obj = crex_fiber_object_free;
	crex_fiber_handlers.get_gc = crex_fiber_object_gc;
	crex_fiber_handlers.clone_obj = NULL;

	crex_ce_fiber_error = register_class_FiberError(crex_ce_error);
	crex_ce_fiber_error->create_object = crex_ce_error->create_object;
}

void crex_fiber_init(void)
{
	crex_fiber_context *context = ecalloc(1, sizeof(crex_fiber_context));

#if defined(__SANITIZE_ADDRESS__) || defined(CREX_FIBER_UCONTEXT)
	// Main fiber stack is only needed if ASan or ucontext is enabled.
	context->stack = emalloc(sizeof(crex_fiber_stack));

#ifdef CREX_FIBER_UCONTEXT
	context->handle = &context->stack->ucontext;
#endif
#endif

	context->status = CREX_FIBER_STATUS_RUNNING;

	EG(main_fiber_context) = context;
	EG(current_fiber_context) = context;
	EG(active_fiber) = NULL;

	crex_fiber_switch_blocking = 0;
}

void crex_fiber_shutdown(void)
{
#if defined(__SANITIZE_ADDRESS__) || defined(CREX_FIBER_UCONTEXT)
	efree(EG(main_fiber_context)->stack);
#endif

	efree(EG(main_fiber_context));

	crex_fiber_switch_block();
}
