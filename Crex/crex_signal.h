/*
  +----------------------------------------------------------------------+
  | Crex Signal Handling                                                 |
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
  | Authors: Lucas Nealan <lucas@crx.net>                                |
  |          Arnaud Le Blanc <lbarnaud@crx.net>                          |
  +----------------------------------------------------------------------+

 */

#ifndef CREX_SIGNAL_H
#define CREX_SIGNAL_H

#ifdef CREX_SIGNALS

#include <signal.h>

#ifndef NSIG
#define NSIG 65
#endif

#ifndef CREX_SIGNAL_QUEUE_SIZE
#define CREX_SIGNAL_QUEUE_SIZE 64
#endif

/* Signal structs */
typedef struct _crex_signal_entry_t {
	int   flags;          /* sigaction style flags */
	void* handler;      /* signal handler or context */
} crex_signal_entry_t;

typedef struct _crex_signal_t {
	int signo;
	siginfo_t *siginfo;
	void* context;
} crex_signal_t;

typedef struct _crex_signal_queue_t {
	crex_signal_t crex_signal;
	struct _crex_signal_queue_t *next;
} crex_signal_queue_t;

/* Signal Globals */
typedef struct _crex_signal_globals_t {
	int depth;
	int blocked;            /* 1==TRUE, 0==FALSE */
	int running;            /* in signal handler execution */
	int active;             /* internal signal handling is enabled */
	bool check;        /* check for replaced handlers on shutdown */
	bool reset;        /* reset signal handlers on each request */
	crex_signal_entry_t handlers[NSIG];
	crex_signal_queue_t pstorage[CREX_SIGNAL_QUEUE_SIZE], *phead, *ptail, *pavail; /* pending queue */
} crex_signal_globals_t;

# ifdef ZTS
#  define SIGG(v) CREX_TSRMG_FAST(crex_signal_globals_offset, crex_signal_globals_t *, v)
BEGIN_EXTERN_C()
CREX_API extern int crex_signal_globals_id;
CREX_API extern size_t crex_signal_globals_offset;
END_EXTERN_C()
# else
#  define SIGG(v) (crex_signal_globals.v)
BEGIN_EXTERN_C()
CREX_API extern crex_signal_globals_t crex_signal_globals;
END_EXTERN_C()
# endif /* not ZTS */

# ifdef ZTS
#  define CREX_SIGNAL_BLOCK_INTERRUPTIONS() if (EXPECTED(crex_signal_globals_id)) { SIGG(depth)++; }
#  define CREX_SIGNAL_UNBLOCK_INTERRUPTIONS() if (EXPECTED(crex_signal_globals_id) && UNEXPECTED(((SIGG(depth)--) == SIGG(blocked)))) { crex_signal_handler_unblock(); }
# else /* ZTS */
#  define CREX_SIGNAL_BLOCK_INTERRUPTIONS()  SIGG(depth)++;
#  define CREX_SIGNAL_UNBLOCK_INTERRUPTIONS() if (((SIGG(depth)--) == SIGG(blocked))) { crex_signal_handler_unblock(); }
# endif /* not ZTS */

CREX_API void crex_signal_handler_unblock(void);
void crex_signal_activate(void);
void crex_signal_deactivate(void);
BEGIN_EXTERN_C()
CREX_API void crex_signal_startup(void);
END_EXTERN_C()
void crex_signal_init(void);

CREX_API void crex_signal(int signo, void (*handler)(int));
CREX_API void crex_sigaction(int signo, const struct sigaction *act, struct sigaction *oldact);

#else /* CREX_SIGNALS */

# define CREX_SIGNAL_BLOCK_INTERRUPTIONS()
# define CREX_SIGNAL_UNBLOCK_INTERRUPTIONS()

# define crex_signal_activate()
# define crex_signal_deactivate()
# define crex_signal_startup()
# define crex_signal_init()

# define crex_signal(signo, handler)           signal(signo, handler)
# define crex_sigaction(signo, act, oldact)    sigaction(signo, act, oldact)

#endif /* CREX_SIGNALS */

#endif /* CREX_SIGNAL_H */
