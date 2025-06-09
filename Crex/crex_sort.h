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
   | Authors: Xinchen Hui <laruence@crx.net>                              |
   |          Sterling Hughes <sterling@crx.net>                          |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_SORT_H
#define CREX_SORT_H

BEGIN_EXTERN_C()
CREX_API void crex_sort(void *base, size_t nmemb, size_t siz, compare_func_t cmp, swap_func_t swp);
CREX_API void crex_insert_sort(void *base, size_t nmemb, size_t siz, compare_func_t cmp, swap_func_t swp);
END_EXTERN_C()

#endif       /* CREX_SORT_H */
