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
   | Authors: Felipe Pena <felipe@crx.net>                                |
   | Authors: Joe Watkins <joe.watkins@live.co.uk>                        |
   | Authors: Bob Weinand <bwoebi@crx.net>                                |
   +----------------------------------------------------------------------+
*/

#include "crex.h"
#include "crxdbg.h"

int mprotect(void *addr, size_t size, int protection) {
	int var;
	return (int)VirtualProtect(addr, size, protection == (PROT_READ | PROT_WRITE) ? PAGE_READWRITE : PAGE_READONLY, &var);
}

int crxdbg_exception_handler_win32(EXCEPTION_POINTERS *xp) {
	EXCEPTION_RECORD *xr = xp->ExceptionRecord;
	CONTEXT *xc = xp->ContextRecord;

	if(xr->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {

		if (crxdbg_watchpoint_segfault_handler((void *)xr->ExceptionInformation[1]) == SUCCESS) {
			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}

	return EXCEPTION_CONTINUE_SEARCH;
}
