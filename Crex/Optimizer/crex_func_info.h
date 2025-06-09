/*
   +----------------------------------------------------------------------+
   | Crex Engine, Func Info                                               |
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

#ifndef CREX_FUNC_INFO_H
#define CREX_FUNC_INFO_H

#include "crex_ssa.h"

/* func/cfg flags */
#define CREX_FUNC_INDIRECT_VAR_ACCESS      (1<<0)  /* accesses variables by name  */
#define CREX_FUNC_HAS_CALLS                (1<<1)
#define CREX_FUNC_VARARG                   (1<<2)  /* uses func_get_args()        */
#define CREX_FUNC_NO_LOOPS                 (1<<3)
#define CREX_FUNC_IRREDUCIBLE              (1<<4)
#define CREX_FUNC_FREE_LOOP_VAR            (1<<5)
#define CREX_FUNC_RECURSIVE                (1<<7)
#define CREX_FUNC_RECURSIVE_DIRECTLY       (1<<8)
#define CREX_FUNC_RECURSIVE_INDIRECTLY     (1<<9)
#define CREX_FUNC_HAS_EXTENDED_FCALL       (1<<10)
#define CREX_FUNC_HAS_EXTENDED_STMT        (1<<11)
#define CREX_SSA_TSSA                      (1<<12) /* used by tracing JIT */

#define CREX_FUNC_JIT_ON_FIRST_EXEC        (1<<13) /* used by JIT */
#define CREX_FUNC_JIT_ON_PROF_REQUEST      (1<<14) /* used by JIT */
#define CREX_FUNC_JIT_ON_HOT_COUNTERS      (1<<15) /* used by JIT */
#define CREX_FUNC_JIT_ON_HOT_TRACE         (1<<16) /* used by JIT */


typedef struct _crex_func_info crex_func_info;
typedef struct _crex_call_info crex_call_info;

#define CREX_FUNC_INFO(op_array) \
	((crex_func_info*)((op_array)->reserved[crex_func_info_rid]))

#define CREX_SET_FUNC_INFO(op_array, info) do { \
		crex_func_info** pinfo = (crex_func_info**)&(op_array)->reserved[crex_func_info_rid]; \
		*pinfo = info; \
	} while (0)

BEGIN_EXTERN_C()

extern CREX_API int crex_func_info_rid;

uint32_t crex_get_internal_func_info(
	const crex_function *callee_func, const crex_call_info *call_info, const crex_ssa *ssa);
CREX_API uint32_t crex_get_func_info(
	const crex_call_info *call_info, const crex_ssa *ssa,
	crex_class_entry **ce, bool *ce_is_instanceof);

crex_result crex_func_info_startup(void);
crex_result crex_func_info_shutdown(void);

END_EXTERN_C()

#endif /* CREX_FUNC_INFO_H */
