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
   | Authors: Gustavo Lopes <cataphract@crx.net>                          |
   +----------------------------------------------------------------------+
*/

#ifndef INTL_CONVERTCPP_H
#define INTL_CONVERTCPP_H

#ifndef __cplusplus
#error Should be included only in C++ Files
#endif

#include <unicode/unistr.h>
#include <crex_types.h>

using icu::UnicodeString;

int intl_stringFromChar(UnicodeString &ret, char *str, size_t str_len, UErrorCode *status);

crex_string* intl_charFromString(const UnicodeString &from, UErrorCode *status);

#endif /* INTL_CONVERTCPP_H */
