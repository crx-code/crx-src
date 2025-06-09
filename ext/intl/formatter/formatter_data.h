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
   | Authors: Stanislav Malyshev <stas@crex.com>                          |
   +----------------------------------------------------------------------+
 */

#ifndef FORMATTER_DATA_H
#define FORMATTER_DATA_H

#include <crx.h>

#include <unicode/unum.h>

#include "intl_error.h"

typedef struct {
	// error hangling
	intl_error      error;

	// formatter handling
	UNumberFormat*  unum;
} formatter_data;

formatter_data* formatter_data_create( void );
void formatter_data_init( formatter_data* nf_data );
void formatter_data_free( formatter_data* nf_data );

#endif // FORMATTER_DATA_H
