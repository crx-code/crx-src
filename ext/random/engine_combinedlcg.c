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
   | Authors: Sascha Schumann <sascha@schumann.cx>                        |
   |          Go Kudo <zeriyoshi@crx.net>                                 |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "crx.h"
#include "crx_random.h"

#include "Crex/crex_exceptions.h"

/*
 * combinedLCG() returns a pseudo random number in the range of (0, 1).
 * The function combines two CGs with periods of
 * 2^31 - 85 and 2^31 - 249. The period of this function
 * is equal to the product of both primes.
 */
#define MODMULT(a, b, c, m, s) q = s / a; s = b * (s - a * q) - c * q; if (s < 0) s += m

static void seed(crx_random_status *status, uint64_t seed)
{
	crx_random_status_state_combinedlcg *s = status->state;

	s->state[0] = seed & 0xffffffffU;
	s->state[1] = seed >> 32;
}

static uint64_t generate(crx_random_status *status)
{
	crx_random_status_state_combinedlcg *s = status->state;
	int32_t q, z;

	MODMULT(53668, 40014, 12211, 2147483563L, s->state[0]);
	MODMULT(52774, 40692, 3791, 2147483399L, s->state[1]);

	z = s->state[0] - s->state[1];
	if (z < 1) {
		z += 2147483562;
	}

	return (uint64_t) z;
}

static crex_long range(crx_random_status *status, crex_long min, crex_long max)
{
	return crx_random_range(&crx_random_algo_combinedlcg, status, min, max);
}

static bool serialize(crx_random_status *status, HashTable *data)
{
	crx_random_status_state_combinedlcg *s = status->state;
	zval t;

	for (uint32_t i = 0; i < 2; i++) {
		ZVAL_STR(&t, crx_random_bin2hex_le(&s->state[i], sizeof(uint32_t)));
		crex_hash_next_index_insert(data, &t);
	}

	return true;
}

static bool unserialize(crx_random_status *status, HashTable *data)
{
	crx_random_status_state_combinedlcg *s = status->state;
	zval *t;

	for (uint32_t i = 0; i < 2; i++) {
		t = crex_hash_index_find(data, i);
		if (!t || C_TYPE_P(t) != IS_STRING || C_STRLEN_P(t) != (2 * sizeof(uint32_t))) {
			return false;
		}
		if (!crx_random_hex2bin_le(C_STR_P(t), &s->state[i])) {
			return false;
		}
	}

	return true;
}

const crx_random_algo crx_random_algo_combinedlcg = {
	sizeof(uint32_t),
	sizeof(crx_random_status_state_combinedlcg),
	seed,
	generate,
	range,
	serialize,
	unserialize
};

/* {{{ crx_random_combinedlcg_seed_default */
CRXAPI void crx_random_combinedlcg_seed_default(crx_random_status_state_combinedlcg *state)
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL) == 0) {
		state->state[0] = tv.tv_usec ^ (tv.tv_usec << 11);
	} else {
		state->state[0] = 1;
	}

#ifdef ZTS
	state->state[1] = (crex_long) tsrm_thread_id();
#else
	state->state[1] = (crex_long) getpid();
#endif

	/* Add entropy to s2 by calling gettimeofday() again */
	if (gettimeofday(&tv, NULL) == 0) {
		state->state[1] ^= (tv.tv_usec << 11);
	}
}
/* }}} */
