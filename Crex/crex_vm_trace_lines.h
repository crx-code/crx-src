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
   | Authors: Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#include "crex_sort.h"

#define VM_TRACE(op)     crex_vm_trace(execute_data, opline);
#define VM_TRACE_START() crex_vm_trace_init();
#define VM_TRACE_END()   crex_vm_trace_finish();

static FILE *vm_trace_file;

static void crex_vm_trace(const crex_execute_data *execute_data, const crex_op *opline)
{
	if (EX(func) && EX(func)->op_array.filename) {
		fprintf(vm_trace_file, "%s:%d\n", ZSTR_VAL(EX(func)->op_array.filename), opline->lineno);
	}
}

static void crex_vm_trace_finish(void)
{
	fclose(vm_trace_file);
}

static void crex_vm_trace_init(void)
{
	vm_trace_file = fopen("crex_vm_trace.log", "w+");
}
