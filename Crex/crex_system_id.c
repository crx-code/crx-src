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
   | Authors: Sammy Kaye Powers <sammyk@crx.net>                          |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
 */

#include "crx.h"
#include "crex_system_id.h"
#include "crex_extensions.h"
#include "ext/standard/md5.h"
#include "ext/hash/crx_hash.h"

CREX_API char crex_system_id[32];

static CRX_MD5_CTX context;
static int finalized = 0;

CREX_API CREX_RESULT_CODE crex_add_system_entropy(const char *module_name, const char *hook_name, const void *data, size_t size)
{
	if (finalized == 0) {
		CRX_MD5Update(&context, module_name, strlen(module_name));
		CRX_MD5Update(&context, hook_name, strlen(hook_name));
		if (size) {
			CRX_MD5Update(&context, data, size);
		}
		return SUCCESS;
	}
	return FAILURE;
}

#define CREX_BIN_ID "BIN_" CREX_TOSTR(SIZEOF_INT) CREX_TOSTR(SIZEOF_LONG) CREX_TOSTR(SIZEOF_SIZE_T) CREX_TOSTR(SIZEOF_CREX_LONG) CREX_TOSTR(CREX_MM_ALIGNMENT)

void crex_startup_system_id(void)
{
	CRX_MD5Init(&context);
	CRX_MD5Update(&context, CRX_VERSION, sizeof(CRX_VERSION)-1);
	CRX_MD5Update(&context, CREX_EXTENSION_BUILD_ID, sizeof(CREX_EXTENSION_BUILD_ID)-1);
	CRX_MD5Update(&context, CREX_BIN_ID, sizeof(CREX_BIN_ID)-1);
	if (strstr(CRX_VERSION, "-dev") != 0) {
		/* Development versions may be changed from build to build */
		CRX_MD5Update(&context, __DATE__, sizeof(__DATE__)-1);
		CRX_MD5Update(&context, __TIME__, sizeof(__TIME__)-1);
	}
	crex_system_id[0] = '\0';
}

#define CREX_HOOK_AST_PROCESS      (1 << 0)
#define CREX_HOOK_COMPILE_FILE     (1 << 1)
#define CREX_HOOK_EXECUTE_EX       (1 << 2)
#define CREX_HOOK_EXECUTE_INTERNAL (1 << 3)

void crex_finalize_system_id(void)
{
	unsigned char digest[16];
	uint8_t hooks = 0;

	if (crex_ast_process) {
		hooks |= CREX_HOOK_AST_PROCESS;
	}
	if (crex_compile_file != compile_file) {
		hooks |= CREX_HOOK_COMPILE_FILE;
	}
	if (crex_execute_ex != execute_ex) {
		hooks |= CREX_HOOK_EXECUTE_EX;
	}
	if (crex_execute_internal) {
		hooks |= CREX_HOOK_EXECUTE_INTERNAL;
	}
	CRX_MD5Update(&context, &hooks, sizeof hooks);

	for (int16_t i = 0; i < 256; i++) {
		if (crex_get_user_opcode_handler((uint8_t) i) != NULL) {
			CRX_MD5Update(&context, &i, sizeof i);
		}
	}

	CRX_MD5Final(digest, &context);
	crx_hash_bin2hex(crex_system_id, digest, sizeof digest);
	finalized = 1;
}
