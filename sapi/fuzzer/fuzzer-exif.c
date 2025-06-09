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
#include "ext/standard/crx_var.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "fuzzer-sapi.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
#if HAVE_EXIF
	crx_stream *stream;
	zval stream_zv;

	if (Size > 256 * 1024) {
		/* Large inputs have a large impact on fuzzer performance,
		 * but are unlikely to be necessary to reach new codepaths. */
		return 0;
	}

	if (fuzzer_request_startup() == FAILURE) {
		return 0;
	}

	stream = crx_stream_memory_create(TEMP_STREAM_DEFAULT);
	crx_stream_write(stream, (const char *) Data, Size);
	crx_stream_to_zval(stream, &stream_zv);

	fuzzer_call_crx_func_zval("exif_read_data", 1, &stream_zv);

	zval_ptr_dtor(&stream_zv);

	/* cleanup */
	crx_request_shutdown(NULL);

	return 0;
#else
	fprintf(stderr, "\n\nERROR:\nCRX built without EXIF, recompile with --enable-exif to use this fuzzer\n");
	exit(1);
#endif
}

int LLVMFuzzerInitialize(int *argc, char ***argv) {
	fuzzer_init_crx(NULL);

	/* fuzzer_shutdown_crx(); */
	return 0;
}

