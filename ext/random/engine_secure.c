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
   | Authors: Sammy Kaye Powers <me@sammyk.me>                            |
   |          Go Kudo <zeriyoshi@crx.net>                                 |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "crx.h"
#include "crx_random.h"

#include "Crex/crex_exceptions.h"

static uint64_t generate(crx_random_status *status)
{
	crex_ulong r = 0;

	crx_random_bytes_throw(&r, sizeof(crex_ulong));

	return r;
}

static crex_long range(crx_random_status *status, crex_long min, crex_long max)
{
	crex_long result = 0;

	crx_random_int_throw(min, max, &result);

	return result;
}

const crx_random_algo crx_random_algo_secure = {
	sizeof(crex_ulong),
	0,
	NULL,
	generate,
	range,
	NULL,
	NULL
};
