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
   | Authors: Christian Seiler <chris_se@gmx.net>                         |
   +----------------------------------------------------------------------+
*/

#include "crex.h"
#include "crex_compile.h"
#include "crex_float.h"

CREX_API void crex_init_fpu(void) /* {{{ */
{
#if XPFPA_HAVE_CW
	XPFPA_DECLARE

	if (!EG(saved_fpu_cw_ptr)) {
		EG(saved_fpu_cw_ptr) = (void*)&EG(saved_fpu_cw);
	}
	XPFPA_STORE_CW(EG(saved_fpu_cw_ptr));
	XPFPA_SWITCH_DOUBLE();
#else
	EG(saved_fpu_cw_ptr) = NULL;
#endif
}
/* }}} */

CREX_API void crex_shutdown_fpu(void) /* {{{ */
{
#if XPFPA_HAVE_CW
	if (EG(saved_fpu_cw_ptr)) {
		XPFPA_RESTORE_CW(EG(saved_fpu_cw_ptr));
	}
#endif
	EG(saved_fpu_cw_ptr) = NULL;
}
/* }}} */

CREX_API void crex_ensure_fpu_mode(void) /* {{{ */
{
	XPFPA_DECLARE

	XPFPA_SWITCH_DOUBLE();
}
/* }}} */
