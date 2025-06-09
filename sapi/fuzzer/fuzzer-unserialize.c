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
   | Authors: Johannes Schlüter <johanes@crx.net>                         |
   +----------------------------------------------------------------------+
 */


#include "fuzzer.h"

#include "Crex/crex.h"
#include "main/crx_config.h"
#include "main/crx_main.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "fuzzer-sapi.h"

#include "ext/standard/crx_var.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
	unsigned char *orig_data = malloc(Size+1);
	memcpy(orig_data, Data, Size);
	orig_data[Size] = '\0';

	if (fuzzer_request_startup() == FAILURE) {
		return 0;
	}

	fuzzer_setup_dummy_frame();

	{
		const unsigned char *data = orig_data;
		zval result;
		ZVAL_UNDEF(&result);

		crx_unserialize_data_t var_hash;
		CRX_VAR_UNSERIALIZE_INIT(var_hash);
		crx_var_unserialize(&result, (const unsigned char **) &data, data + Size, &var_hash);
		CRX_VAR_UNSERIALIZE_DESTROY(var_hash);

		zval_ptr_dtor(&result);
	}

	free(orig_data);

	fuzzer_request_shutdown();
	return 0;
}

int LLVMFuzzerInitialize(int *argc, char ***argv) {
	fuzzer_init_crx(NULL);

	/* fuzzer_shutdown_crx(); */
	return 0;
}
