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
   | Author: Edin Kadribasic <edink@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef _CRX_EMBED_H_
#define _CRX_EMBED_H_

#include <main/crx.h>
#include <main/SAPI.h>
#include <main/crx_main.h>
#include <main/crx_variables.h>
#include <main/crx_ini.h>
#include <crex_ini.h>

#define CRX_EMBED_START_BLOCK(x,y) { \
    crx_embed_init(x, y); \
    crex_first_try {

#define CRX_EMBED_END_BLOCK() \
  } crex_catch { \
    /* int exit_status = EG(exit_status); */ \
  } crex_end_try(); \
  crx_embed_shutdown(); \
}

#ifndef CRX_WIN32
    #define EMBED_SAPI_API SAPI_API
#else
    #define EMBED_SAPI_API
#endif

#ifdef ZTS
CREX_TSRMLS_CACHE_EXTERN()
#endif

BEGIN_EXTERN_C()
EMBED_SAPI_API int crx_embed_init(int argc, char **argv);
EMBED_SAPI_API void crx_embed_shutdown(void);
extern EMBED_SAPI_API sapi_module_struct crx_embed_module;
END_EXTERN_C()


#endif /* _CRX_EMBED_H_ */
