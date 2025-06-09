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

#ifndef CREX_FIBERS_H
#define CREX_FIBERS_H

#include "crex_API.h"
#include "crex_types.h"

#define CREX_FIBER_GUARD_PAGES 1

#define CREX_FIBER_DEFAULT_C_STACK_SIZE (4096 * (((sizeof(void *)) < 8) ? 256 : 512))
#define CREX_FIBER_VM_STACK_SIZE (1024 * sizeof(zval))

BEGIN_EXTERN_C()

typedef enum {
	CREX_FIBER_STATUS_INIT,
	CREX_FIBER_STATUS_RUNNING,
	CREX_FIBER_STATUS_SUSPENDED,
	CREX_FIBER_STATUS_DEAD,
} crex_fiber_status;

typedef enum {
	CREX_FIBER_FLAG_THREW     = 1 << 0,
	CREX_FIBER_FLAG_BAILOUT   = 1 << 1,
	CREX_FIBER_FLAG_DESTROYED = 1 << 2,
} crex_fiber_flag;

typedef enum {
	CREX_FIBER_TRANSFER_FLAG_ERROR = 1 << 0,
	CREX_FIBER_TRANSFER_FLAG_BAILOUT = 1 << 1
} crex_fiber_transfer_flag;

void crex_register_fiber_ce(void);
void crex_fiber_init(void);
void crex_fiber_shutdown(void);

extern CREX_API crex_class_entry *crex_ce_fiber;

typedef struct _crex_fiber_stack crex_fiber_stack;

/* Encapsulates data needed for a context switch. */
typedef struct _crex_fiber_transfer {
	/* Fiber that will be switched to / has resumed us. */
	crex_fiber_context *context;

	/* Value to that should be send to (or was received from) a fiber. */
	zval value;

	/* Bitmask of flags defined in enum crex_fiber_transfer_flag. */
	uint8_t flags;
} crex_fiber_transfer;

/* Coroutine functions must populate the given transfer with a new context
 * and (optional) data before they return. */
typedef void (*crex_fiber_coroutine)(crex_fiber_transfer *transfer);
typedef void (*crex_fiber_clean)(crex_fiber_context *context);

struct _crex_fiber_context {
	/* Pointer to boost.context or ucontext_t data. */
	void *handle;

	/* Pointer that identifies the fiber type. */
	void *kind;

	/* Entrypoint function of the fiber. */
	crex_fiber_coroutine function;

	/* Cleanup function for fiber. */
	crex_fiber_clean cleanup;

	/* Assigned C stack. */
	crex_fiber_stack *stack;

	/* Fiber status. */
	crex_fiber_status status;

	/* Observer state */
	crex_execute_data *top_observed_frame;

	/* Reserved for extensions */
	void *reserved[CREX_MAX_RESERVED_RESOURCES];
};

struct _crex_fiber {
	/* CRX object handle. */
	crex_object std;

	/* Flags are defined in enum crex_fiber_flag. */
	uint8_t flags;

	/* Native C fiber context. */
	crex_fiber_context context;

	/* Fiber that resumed us. */
	crex_fiber_context *caller;

	/* Fiber that suspended us. */
	crex_fiber_context *previous;

	/* Callback and info / cache to be used when fiber is started. */
	crex_fcall_info fci;
	crex_fcall_info_cache fci_cache;

	/* Current Crex VM execute data being run by the fiber. */
	crex_execute_data *execute_data;

	/* Frame on the bottom of the fiber vm stack. */
	crex_execute_data *stack_bottom;

	/* Active fiber vm stack. */
	crex_vm_stack vm_stack;

	/* Storage for fiber return value. */
	zval result;
};

/* These functions may be used to create custom fiber objects using the bundled fiber switching context. */
CREX_API crex_result crex_fiber_init_context(crex_fiber_context *context, void *kind, crex_fiber_coroutine coroutine, size_t stack_size);
CREX_API void crex_fiber_destroy_context(crex_fiber_context *context);
CREX_API void crex_fiber_switch_context(crex_fiber_transfer *transfer);
#ifdef CREX_CHECK_STACK_LIMIT
CREX_API void* crex_fiber_stack_limit(crex_fiber_stack *stack);
CREX_API void* crex_fiber_stack_base(crex_fiber_stack *stack);
#endif /* CREX_CHECK_STACK_LIMIT */

CREX_API void crex_fiber_switch_block(void);
CREX_API void crex_fiber_switch_unblock(void);
CREX_API bool crex_fiber_switch_blocked(void);

END_EXTERN_C()

static crex_always_inline crex_fiber *crex_fiber_from_context(crex_fiber_context *context)
{
	CREX_ASSERT(context->kind == crex_ce_fiber && "Fiber context does not belong to a Crex fiber");

	return (crex_fiber *)(((char *) context) - XtOffsetOf(crex_fiber, context));
}

static crex_always_inline crex_fiber_context *crex_fiber_get_context(crex_fiber *fiber)
{
	return &fiber->context;
}

#endif
