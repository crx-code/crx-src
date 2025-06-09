/*
   +----------------------------------------------------------------------+
   | Crex OPcache                                                         |
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
   |          Stanislav Malyshev <stas@crex.com>                          |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_ACCELERATOR_DEBUG_H
#define CREX_ACCELERATOR_DEBUG_H

#define ACCEL_LOG_FATAL					0
#define ACCEL_LOG_ERROR					1
#define ACCEL_LOG_WARNING				2
#define ACCEL_LOG_INFO					3
#define ACCEL_LOG_DEBUG					4

BEGIN_EXTERN_C()

void crex_accel_error(int type, const char *format, ...) CREX_ATTRIBUTE_FORMAT(printf, 2, 3);
CREX_NORETURN void crex_accel_error_noreturn(int type, const char *format, ...) CREX_ATTRIBUTE_FORMAT(printf, 2, 3);

END_EXTERN_C()

#endif /* _CREX_ACCELERATOR_DEBUG_H */
