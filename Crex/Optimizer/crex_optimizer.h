/*
   +----------------------------------------------------------------------+
   | Crex OPcache                                                         |
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
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   |          Stanislav Malyshev <stas@crex.com>                          |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_OPTIMIZER_H
#define CREX_OPTIMIZER_H

#include "crex.h"
#include "crex_compile.h"

#define CREX_OPTIMIZER_PASS_1		(1<<0)   /* Simple local optimizations   */
#define CREX_OPTIMIZER_PASS_2		(1<<1)   /*                              */
#define CREX_OPTIMIZER_PASS_3		(1<<2)   /* Jump optimization            */
#define CREX_OPTIMIZER_PASS_4		(1<<3)   /* INIT_FCALL_BY_NAME -> DO_FCALL */
#define CREX_OPTIMIZER_PASS_5		(1<<4)   /* CFG based optimization       */
#define CREX_OPTIMIZER_PASS_6		(1<<5)   /* DFA based optimization       */
#define CREX_OPTIMIZER_PASS_7		(1<<6)   /* CALL GRAPH optimization      */
#define CREX_OPTIMIZER_PASS_8		(1<<7)   /* SCCP (constant propagation)  */
#define CREX_OPTIMIZER_PASS_9		(1<<8)   /* TMP VAR usage                */
#define CREX_OPTIMIZER_PASS_10		(1<<9)   /* NOP removal                 */
#define CREX_OPTIMIZER_PASS_11		(1<<10)  /* Merge equal constants       */
#define CREX_OPTIMIZER_PASS_12		(1<<11)  /* Adjust used stack           */
#define CREX_OPTIMIZER_PASS_13		(1<<12)  /* Remove unused variables     */
#define CREX_OPTIMIZER_PASS_14		(1<<13)  /* DCE (dead code elimination) */
#define CREX_OPTIMIZER_PASS_15		(1<<14)  /* (unsafe) Collect constants */
#define CREX_OPTIMIZER_PASS_16		(1<<15)  /* Inline functions */

#define CREX_OPTIMIZER_IGNORE_OVERLOADING	(1<<16)  /* (unsafe) Ignore possibility of operator overloading */

#define CREX_OPTIMIZER_NARROW_TO_DOUBLE		(1<<17)  /* try to narrow long constant assignments to double */

#define CREX_OPTIMIZER_ALL_PASSES	0x7FFFFFFF

#define DEFAULT_OPTIMIZATION_LEVEL  "0x7FFEBFFF"


#define CREX_DUMP_AFTER_PASS_1		CREX_OPTIMIZER_PASS_1
#define CREX_DUMP_AFTER_PASS_2		CREX_OPTIMIZER_PASS_2
#define CREX_DUMP_AFTER_PASS_3		CREX_OPTIMIZER_PASS_3
#define CREX_DUMP_AFTER_PASS_4		CREX_OPTIMIZER_PASS_4
#define CREX_DUMP_AFTER_PASS_5		CREX_OPTIMIZER_PASS_5
#define CREX_DUMP_AFTER_PASS_6		CREX_OPTIMIZER_PASS_6
#define CREX_DUMP_AFTER_PASS_7		CREX_OPTIMIZER_PASS_7
#define CREX_DUMP_AFTER_PASS_8		CREX_OPTIMIZER_PASS_8
#define CREX_DUMP_AFTER_PASS_9		CREX_OPTIMIZER_PASS_9
#define CREX_DUMP_AFTER_PASS_10		CREX_OPTIMIZER_PASS_10
#define CREX_DUMP_AFTER_PASS_11		CREX_OPTIMIZER_PASS_11
#define CREX_DUMP_AFTER_PASS_12		CREX_OPTIMIZER_PASS_12
#define CREX_DUMP_AFTER_PASS_13		CREX_OPTIMIZER_PASS_13
#define CREX_DUMP_AFTER_PASS_14		CREX_OPTIMIZER_PASS_14

#define CREX_DUMP_BEFORE_OPTIMIZER  (1<<16)
#define CREX_DUMP_AFTER_OPTIMIZER   (1<<17)

#define CREX_DUMP_BEFORE_BLOCK_PASS (1<<18)
#define CREX_DUMP_AFTER_BLOCK_PASS  (1<<19)
#define CREX_DUMP_BLOCK_PASS_VARS   (1<<20)

#define CREX_DUMP_BEFORE_DFA_PASS   (1<<21)
#define CREX_DUMP_AFTER_DFA_PASS    (1<<22)
#define CREX_DUMP_DFA_CFG           (1<<23)
#define CREX_DUMP_DFA_DOMINATORS    (1<<24)
#define CREX_DUMP_DFA_LIVENESS      (1<<25)
#define CREX_DUMP_DFA_PHI           (1<<26)
#define CREX_DUMP_DFA_SSA           (1<<27)
#define CREX_DUMP_DFA_SSA_VARS      (1<<28)
#define CREX_DUMP_SCCP              (1<<29)

typedef struct _crex_script {
	crex_string   *filename;
	crex_op_array  main_op_array;
	HashTable      function_table;
	HashTable      class_table;
} crex_script;

typedef void (*crex_optimizer_pass_t)(crex_script *, void *context);

BEGIN_EXTERN_C()
CREX_API void crex_optimize_script(crex_script *script, crex_long optimization_level, crex_long debug_level);
CREX_API int crex_optimizer_register_pass(crex_optimizer_pass_t pass);
CREX_API void crex_optimizer_unregister_pass(int idx);
crex_result crex_optimizer_startup(void);
crex_result crex_optimizer_shutdown(void);
END_EXTERN_C()

#endif
