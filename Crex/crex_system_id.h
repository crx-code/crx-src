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
   | Author: Sammy Kaye Powers <sammyk@crx.net>                           |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_SYSTEM_ID_H
#define CREX_SYSTEM_ID_H

BEGIN_EXTERN_C()
/* True global; Write-only during MINIT/startup */
extern CREX_API char crex_system_id[32];

CREX_API CREX_RESULT_CODE crex_add_system_entropy(const char *module_name, const char *hook_name, const void *data, size_t size);
END_EXTERN_C()

void crex_startup_system_id(void);
void crex_finalize_system_id(void);

#endif /* CREX_SYSTEM_ID_H */
