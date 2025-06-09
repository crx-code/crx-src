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
   | Author: Andrei Zmievski <andrei@crx.net>                             |
   +----------------------------------------------------------------------+
 */

#ifndef CRX_TOKENIZER_H
#define CRX_TOKENIZER_H

extern crex_module_entry tokenizer_module_entry;
#define crxext_tokenizer_ptr &tokenizer_module_entry

#include "crx_version.h"
#define CRX_TOKENIZER_VERSION CRX_VERSION

#define TOKEN_PARSE (1 << 0)

#ifdef ZTS
#include "TSRM.h"
#endif

char *get_token_type_name(int token_type);


CRX_MINIT_FUNCTION(tokenizer);
CRX_MINFO_FUNCTION(tokenizer);

#endif	/* CRX_TOKENIZER_H */
