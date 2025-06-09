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
   | Author: Go Kudo <zeriyoshi@crx.net>                                  |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "crx.h"
#include "crx_random.h"

static uint64_t generate(crx_random_status *status)
{
	crx_random_status_state_user *s = status->state;
	uint64_t result = 0;
	size_t size;
	zval retval;

	crex_call_known_instance_method_with_0_params(s->generate_method, s->object, &retval);

	if (EG(exception)) {
		return 0;
	}

	/* Store generated size in a state */
	size = C_STRLEN(retval);

	/* Guard for over 64-bit results */
	if (size > sizeof(uint64_t)) {
		size = sizeof(uint64_t);
	}
	status->last_generated_size = size;

	if (size > 0) {
		/* Endianness safe copy */
		for (size_t i = 0; i < size; i++) {
			result += ((uint64_t) (unsigned char) C_STRVAL(retval)[i]) << (8 * i);
		}
	} else {
		crex_throw_error(random_ce_Random_BrokenRandomEngineError, "A random engine must return a non-empty string");
		return 0;
	}

	zval_ptr_dtor(&retval);

	return result;
}

static crex_long range(crx_random_status *status, crex_long min, crex_long max)
{
	return crx_random_range(&crx_random_algo_user, status, min, max);
}

const crx_random_algo crx_random_algo_user = {
	0,
	sizeof(crx_random_status_state_user),
	NULL,
	generate,
	range,
	NULL,
	NULL,
};
