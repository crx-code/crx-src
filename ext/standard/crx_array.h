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
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   |          Rasmus Lerdorf <rasmus@crx.net>                             |
   |          Andrei Zmievski <andrei@crx.net>                            |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_ARRAY_H
#define CRX_ARRAY_H

# include "ext/random/crx_random.h"

CRX_MINIT_FUNCTION(array);
CRX_MSHUTDOWN_FUNCTION(array);

CRXAPI int crx_array_merge(HashTable *dest, HashTable *src);
CRXAPI int crx_array_merge_recursive(HashTable *dest, HashTable *src);
CRXAPI int crx_array_replace_recursive(HashTable *dest, HashTable *src);
CRXAPI int crx_multisort_compare(const void *a, const void *b);
CRXAPI crex_long crx_count_recursive(HashTable *ht);

CRXAPI bool crx_array_data_shuffle(const crx_random_algo *algo, crx_random_status *status, zval *array);
CRXAPI bool crx_array_pick_keys(const crx_random_algo *algo, crx_random_status *status, zval *input, crex_long num_req, zval *retval, bool silent);

#define CRX_EXTR_OVERWRITE			0
#define CRX_EXTR_SKIP				1
#define CRX_EXTR_PREFIX_SAME		2
#define CRX_EXTR_PREFIX_ALL			3
#define CRX_EXTR_PREFIX_INVALID		4
#define CRX_EXTR_PREFIX_IF_EXISTS	5
#define CRX_EXTR_IF_EXISTS			6

#define CRX_EXTR_REFS				0x100

#define CRX_CASE_LOWER				0
#define CRX_CASE_UPPER				1

#define CRX_SORT_REGULAR            0
#define CRX_SORT_NUMERIC            1
#define CRX_SORT_STRING             2
#define CRX_SORT_DESC               3
#define CRX_SORT_ASC                4
#define CRX_SORT_LOCALE_STRING      5
#define CRX_SORT_NATURAL            6
#define CRX_SORT_FLAG_CASE          8

#define CRX_COUNT_NORMAL      0
#define CRX_COUNT_RECURSIVE   1

#define ARRAY_FILTER_USE_BOTH	1
#define ARRAY_FILTER_USE_KEY	2

CREX_BEGIN_MODULE_GLOBALS(array)
	bucket_compare_func_t *multisort_func;
	bool compare_deprecation_thrown;
CREX_END_MODULE_GLOBALS(array)

#define ARRAYG(v) CREX_MODULE_GLOBALS_ACCESSOR(array, v)

#endif /* CRX_ARRAY_H */
