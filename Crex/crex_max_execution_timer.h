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
   | Author: Kévin Dunglas <kevin@dunglas.dev>                            |
   +----------------------------------------------------------------------+
 */

#ifndef CREX_MAX_EXECUTION_TIMER_H
#define CREX_MAX_EXECUTION_TIMER_H

# ifdef CREX_MAX_EXECUTION_TIMERS

#include "crex_long.h"

/* Must be called after calls to fork() */
CREX_API void crex_max_execution_timer_init(void);
void crex_max_execution_timer_settime(crex_long seconds);
void crex_max_execution_timer_shutdown(void);

# else

#define crex_max_execution_timer_init()
#define crex_max_execution_timer_settime(seconds)
#define crex_max_execution_timer_shutdown()

# endif
#endif
