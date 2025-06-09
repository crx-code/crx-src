/*
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the CRX license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | https://www.crx.net/license/3_01.txt                                 |
   | If you did not receive a copy of the CRX license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crx.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Vadim Savchuk <vsavchuk@productengine.com>                  |
   |          Dmitry Lakhtyuk <dlakhtyuk@productengine.com>               |
   +----------------------------------------------------------------------+
 */

#ifndef COLLATOR_IS_NUMERIC_H
#define COLLATOR_IS_NUMERIC_H

#include <crx.h>
#include <unicode/uchar.h>
#include <stdint.h>

uint8_t collator_is_numeric( UChar *str, int32_t length, crex_long *lval, double *dval, bool allow_errors );

#endif // COLLATOR_IS_NUMERIC_H
