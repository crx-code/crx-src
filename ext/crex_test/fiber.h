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
  | Authors: Aaron Piotrowski <aaron@trowski.com>                        |
  +----------------------------------------------------------------------+
*/

#ifndef CREX_TEST_FIBER_H
#define CREX_TEST_FIBER_H

#include "crex_fibers.h"

typedef struct _crex_test_fiber crex_test_fiber;

struct _crex_test_fiber {
	crex_object std;
	uint8_t flags;
	crex_fiber_context context;
	crex_fiber_context *caller;
	crex_fiber_context *previous;
	crex_test_fiber *target;
	crex_fcall_info fci;
	crex_fcall_info_cache fci_cache;
	zval result;
};

void crex_test_fiber_init(void);

#endif
