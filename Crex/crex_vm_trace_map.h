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

#include "crex_vm_handlers.h"
#include "crex_sort.h"

#define GEN_MAP(n, name) do { \
		ZVAL_LONG(&tmp, (crex_long)(uintptr_t)crex_opcode_handlers[n]); \
		crex_hash_str_add(&vm_trace_ht, #name, sizeof(#name) - 1, &tmp); \
	} while (0);

#define VM_TRACE_START() do { \
		zval tmp; \
		crex_hash_init(&vm_trace_ht, 0, NULL, NULL, 1); \
		VM_HANDLERS(GEN_MAP) \
		crex_vm_trace_init(); \
	} while (0)

#ifdef _WIN64
# define ADDR_FMT "%016I64x"
#elif SIZEOF_SIZE_T == 4
# define ADDR_FMT "%08zx"
#elif SIZEOF_SIZE_T == 8
# define ADDR_FMT "%016zx"
#else
# error "Unknown SIZEOF_SIZE_T"
#endif

static HashTable vm_trace_ht;

static int crex_vm_trace_compare(const Bucket *p1, const Bucket *p2)
{
	if (C_LVAL(p1->val) > C_LVAL(p2->val)) {
		return 1;
	} else if (C_LVAL(p1->val) < C_LVAL(p2->val)) {
		return -1;
	} else {
		return 0;
	}
}

static void crex_vm_trace_init(void)
{
	FILE *f;
	crex_string *key, *prev_key;
	zval *val;
	crex_long prev_addr;

	f = fopen("crex_vm.map", "w+");
	if (f) {
		crex_hash_sort(&vm_trace_ht, (bucket_compare_func_t)crex_vm_trace_compare, 0);
		prev_key = NULL;
		CREX_HASH_MAP_FOREACH_STR_KEY_VAL(&vm_trace_ht, key, val) {
			if (prev_key) {
				fprintf(f, ADDR_FMT" "ADDR_FMT" t %s\n", prev_addr, C_LVAL_P(val) - prev_addr, ZSTR_VAL(prev_key));
			}
			prev_key  = key;
			prev_addr = C_LVAL_P(val);
		} CREX_HASH_FOREACH_END();
		if (prev_key) {
			fprintf(f, ADDR_FMT" "ADDR_FMT" t %s\n", prev_addr, 0, ZSTR_VAL(prev_key));
		}
		fclose(f);
	}
	crex_hash_destroy(&vm_trace_ht);
}
