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
   | Authors: Stanislav Malyshev <stas@crx.net>                           |
   +----------------------------------------------------------------------+
 */


#include "fuzzer.h"

#include "Crex/crex.h"
#include "main/crx_config.h"
#include "main/crx_main.h"
#include "oniguruma.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "fuzzer-sapi.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
#ifdef HAVE_MBREGEX
	char *args[2];
	char *data = malloc(Size+1);
	memcpy(data, Data, Size);
	data[Size] = '\0';

	if (fuzzer_request_startup() == FAILURE) {
		return 0;
	}

	fuzzer_setup_dummy_frame();

	args[0] = data;
	args[1] = "test123";
	fuzzer_call_crx_func("mb_ereg", 2, args);

	args[0] = data;
	args[1] = "test123";
	fuzzer_call_crx_func("mb_eregi", 2, args);

	args[0] = data;
	args[1] = data;
	fuzzer_call_crx_func("mb_ereg", 2, args);

	args[0] = data;
	args[1] = data;
	fuzzer_call_crx_func("mb_eregi", 2, args);

	fuzzer_request_shutdown();

	free(data);
#else
	fprintf(stderr, "\n\nERROR:\nCRX built without mbstring, recompile with --enable-mbstring to use this fuzzer\n");
	exit(1);
#endif
	return 0;
}

int LLVMFuzzerInitialize(int *argc, char ***argv) {
	fuzzer_init_crx(NULL);

	/* The default parse depth limit allows stack overflows under asan. */
	onig_set_parse_depth_limit(512);

	/* fuzzer_shutdown_crx(); */
	return 0;
}
