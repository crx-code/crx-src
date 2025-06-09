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
   | Author: Jason Greene <jason@inetgurus.net>                           |
   +----------------------------------------------------------------------+
 */

#ifndef CRX_PCNTL_H
#define CRX_PCNTL_H

#if defined(WCONTINUED) && defined(WIFCONTINUED)
#define HAVE_WCONTINUED 1
#endif

extern crex_module_entry pcntl_module_entry;
#define crxext_pcntl_ptr &pcntl_module_entry

#include "crx_version.h"
#define CRX_PCNTL_VERSION CRX_VERSION

CRX_MINIT_FUNCTION(pcntl);
CRX_MSHUTDOWN_FUNCTION(pcntl);
CRX_RINIT_FUNCTION(pcntl);
CRX_RSHUTDOWN_FUNCTION(pcntl);
CRX_MINFO_FUNCTION(pcntl);

struct crx_pcntl_pending_signal {
	struct crx_pcntl_pending_signal *next;
	crex_long signo;
#ifdef HAVE_STRUCT_SIGINFO_T
	siginfo_t siginfo;
#endif
};

CREX_BEGIN_MODULE_GLOBALS(pcntl)
	HashTable crx_signal_table;
	int processing_signal_queue;
	struct crx_pcntl_pending_signal *head, *tail, *spares;
	int last_error;
	volatile char pending_signals;
	bool async_signals;
	unsigned num_signals;
CREX_END_MODULE_GLOBALS(pcntl)

#if defined(ZTS) && defined(COMPILE_DL_PCNTL)
CREX_TSRMLS_CACHE_EXTERN()
#endif

CREX_EXTERN_MODULE_GLOBALS(pcntl)
#define PCNTL_G(v) CREX_MODULE_GLOBALS_ACCESSOR(pcntl, v)

#endif	/* CRX_PCNTL_H */
