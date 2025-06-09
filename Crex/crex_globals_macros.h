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
   +----------------------------------------------------------------------+
*/

#ifndef CREX_GLOBALS_MACROS_H
#define CREX_GLOBALS_MACROS_H

typedef struct _crex_compiler_globals crex_compiler_globals;
typedef struct _crex_executor_globals crex_executor_globals;
typedef struct _crex_crx_scanner_globals crex_crx_scanner_globals;
typedef struct _crex_ini_scanner_globals crex_ini_scanner_globals;

BEGIN_EXTERN_C()

/* Compiler */
#ifdef ZTS
# define CG(v) CREX_TSRMG_FAST(compiler_globals_offset, crex_compiler_globals *, v)
#else
# define CG(v) (compiler_globals.v)
extern CREX_API struct _crex_compiler_globals compiler_globals;
#endif
CREX_API int crexparse(void);


/* Executor */
#ifdef ZTS
# define EG(v) CREX_TSRMG_FAST(executor_globals_offset, crex_executor_globals *, v)
#else
# define EG(v) (executor_globals.v)
extern CREX_API crex_executor_globals executor_globals;
#endif

/* Language Scanner */
#ifdef ZTS
# define LANG_SCNG(v) CREX_TSRMG_FAST(language_scanner_globals_offset, crex_crx_scanner_globals *, v)
extern CREX_API ts_rsrc_id language_scanner_globals_id;
extern CREX_API size_t language_scanner_globals_offset;
#else
# define LANG_SCNG(v) (language_scanner_globals.v)
extern CREX_API crex_crx_scanner_globals language_scanner_globals;
#endif


/* INI Scanner */
#ifdef ZTS
# define INI_SCNG(v) CREX_TSRMG_FAST(ini_scanner_globals_offset, crex_ini_scanner_globals *, v)
extern CREX_API ts_rsrc_id ini_scanner_globals_id;
extern CREX_API size_t ini_scanner_globals_offset;
#else
# define INI_SCNG(v) (ini_scanner_globals.v)
extern CREX_API crex_ini_scanner_globals ini_scanner_globals;
#endif

END_EXTERN_C()

#endif /* CREX_GLOBALS_MACROS_H */
