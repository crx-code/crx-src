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
   | Authors: Jerome Loyet <jerome@loyet.net>                             |
   +----------------------------------------------------------------------+
*/

#ifndef FPM_EVENTS_KQUEUE_H
#define FPM_EVENTS_KQUEUE_H

#include "../fpm_config.h"
#include "../fpm_events.h"

struct fpm_event_module_s *fpm_event_kqueue_module(void);

#endif /* FPM_EVENTS_KQUEUE_H */
