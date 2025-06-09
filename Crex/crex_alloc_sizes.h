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

#ifndef CREX_ALLOC_SIZES_H
#define CREX_ALLOC_SIZES_H

#define CREX_MM_CHUNK_SIZE ((size_t) (2 * 1024 * 1024))    /* 2 MB  */
#define CREX_MM_PAGE_SIZE  (4 * 1024)                      /* 4 KB  */
#define CREX_MM_PAGES      (CREX_MM_CHUNK_SIZE / CREX_MM_PAGE_SIZE)  /* 512 */
#define CREX_MM_FIRST_PAGE (1)

#define CREX_MM_MIN_SMALL_SIZE		8
#define CREX_MM_MAX_SMALL_SIZE      3072
#define CREX_MM_MAX_LARGE_SIZE      (CREX_MM_CHUNK_SIZE - (CREX_MM_PAGE_SIZE * CREX_MM_FIRST_PAGE))

/* num, size, count, pages */
#define CREX_MM_BINS_INFO(_, x, y) \
	_( 0,    8,  512, 1, x, y) \
	_( 1,   16,  256, 1, x, y) \
	_( 2,   24,  170, 1, x, y) \
	_( 3,   32,  128, 1, x, y) \
	_( 4,   40,  102, 1, x, y) \
	_( 5,   48,   85, 1, x, y) \
	_( 6,   56,   73, 1, x, y) \
	_( 7,   64,   64, 1, x, y) \
	_( 8,   80,   51, 1, x, y) \
	_( 9,   96,   42, 1, x, y) \
	_(10,  112,   36, 1, x, y) \
	_(11,  128,   32, 1, x, y) \
	_(12,  160,   25, 1, x, y) \
	_(13,  192,   21, 1, x, y) \
	_(14,  224,   18, 1, x, y) \
	_(15,  256,   16, 1, x, y) \
	_(16,  320,   64, 5, x, y) \
	_(17,  384,   32, 3, x, y) \
	_(18,  448,    9, 1, x, y) \
	_(19,  512,    8, 1, x, y) \
	_(20,  640,   32, 5, x, y) \
	_(21,  768,   16, 3, x, y) \
	_(22,  896,    9, 2, x, y) \
	_(23, 1024,    8, 2, x, y) \
	_(24, 1280,   16, 5, x, y) \
	_(25, 1536,    8, 3, x, y) \
	_(26, 1792,   16, 7, x, y) \
	_(27, 2048,    8, 4, x, y) \
	_(28, 2560,    8, 5, x, y) \
	_(29, 3072,    4, 3, x, y)

#endif /* CREX_ALLOC_SIZES_H */
