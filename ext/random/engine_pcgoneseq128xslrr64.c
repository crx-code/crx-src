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
   |                                                                      |
   | Based on code from: Melissa O'Neill <oneill@pcg-random.org>          |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "crx.h"
#include "crx_random.h"

#include "Crex/crex_exceptions.h"

static inline void step(crx_random_status_state_pcgoneseq128xslrr64 *s)
{
	s->state = crx_random_uint128_add(
		crx_random_uint128_multiply(s->state, crx_random_uint128_constant(2549297995355413924ULL,4865540595714422341ULL)),
		crx_random_uint128_constant(6364136223846793005ULL,1442695040888963407ULL)
	);
}

static inline void seed128(crx_random_status *status, crx_random_uint128_t seed)
{
	crx_random_status_state_pcgoneseq128xslrr64 *s = status->state;
	s->state = crx_random_uint128_constant(0ULL, 0ULL);
	step(s);
	s->state = crx_random_uint128_add(s->state, seed);
	step(s);
}

static void seed(crx_random_status *status, uint64_t seed)
{
	seed128(status, crx_random_uint128_constant(0ULL, seed));
}

static uint64_t generate(crx_random_status *status)
{
	crx_random_status_state_pcgoneseq128xslrr64 *s = status->state;

	step(s);
	return crx_random_pcgoneseq128xslrr64_rotr64(s->state);
}

static crex_long range(crx_random_status *status, crex_long min, crex_long max)
{
	return crx_random_range(&crx_random_algo_pcgoneseq128xslrr64, status, min, max);
}

static bool serialize(crx_random_status *status, HashTable *data)
{
	crx_random_status_state_pcgoneseq128xslrr64 *s = status->state;
	uint64_t u;
	zval z;

	u = crx_random_uint128_hi(s->state);
	ZVAL_STR(&z, crx_random_bin2hex_le(&u, sizeof(uint64_t)));
	crex_hash_next_index_insert(data, &z);

	u = crx_random_uint128_lo(s->state);
	ZVAL_STR(&z, crx_random_bin2hex_le(&u, sizeof(uint64_t)));
	crex_hash_next_index_insert(data, &z);

	return true;
}

static bool unserialize(crx_random_status *status, HashTable *data)
{
	crx_random_status_state_pcgoneseq128xslrr64 *s = status->state;
	uint64_t u[2];
	zval *t;

	/* Verify the expected number of elements, this implicitly ensures that no additional elements are present. */
	if (crex_hash_num_elements(data) != 2) {
		return false;
	}

	for (uint32_t i = 0; i < 2; i++) {
		t = crex_hash_index_find(data, i);
		if (!t || C_TYPE_P(t) != IS_STRING || C_STRLEN_P(t) != (2 * sizeof(uint64_t))) {
			return false;
		}
		if (!crx_random_hex2bin_le(C_STR_P(t), &u[i])) {
			return false;
		}
	}
	s->state = crx_random_uint128_constant(u[0], u[1]);

	return true;
}

const crx_random_algo crx_random_algo_pcgoneseq128xslrr64 = {
	sizeof(uint64_t),
	sizeof(crx_random_status_state_pcgoneseq128xslrr64),
	seed,
	generate,
	range,
	serialize,
	unserialize
};

/* {{{ crx_random_pcgoneseq128xslrr64_advance */
CRXAPI void crx_random_pcgoneseq128xslrr64_advance(crx_random_status_state_pcgoneseq128xslrr64 *state, uint64_t advance)
{
	crx_random_uint128_t
		cur_mult = crx_random_uint128_constant(2549297995355413924ULL,4865540595714422341ULL),
		cur_plus = crx_random_uint128_constant(6364136223846793005ULL,1442695040888963407ULL),
		acc_mult = crx_random_uint128_constant(0ULL, 1ULL),
		acc_plus = crx_random_uint128_constant(0ULL, 0ULL);

	while (advance > 0) {
		if (advance & 1) {
			acc_mult = crx_random_uint128_multiply(acc_mult, cur_mult);
			acc_plus = crx_random_uint128_add(crx_random_uint128_multiply(acc_plus, cur_mult), cur_plus);
		}
		cur_plus = crx_random_uint128_multiply(crx_random_uint128_add(cur_mult, crx_random_uint128_constant(0ULL, 1ULL)), cur_plus);
		cur_mult = crx_random_uint128_multiply(cur_mult, cur_mult);
		advance /= 2;
	}

	state->state = crx_random_uint128_add(crx_random_uint128_multiply(acc_mult, state->state), acc_plus);
}
/* }}} */

/* {{{ Random\Engine\PcgOneseq128XslRr64::__main */
CRX_METHOD(Random_Engine_PcgOneseq128XslRr64, __main)
{
	crx_random_engine *engine = C_RANDOM_ENGINE_P(CREX_THIS);
	crx_random_status_state_pcgoneseq128xslrr64 *state = engine->status->state;
	crex_string *str_seed = NULL;
	crex_long int_seed = 0;
	bool seed_is_null = true;
	uint32_t i, j;
	uint64_t t[2];

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL;
		C_PARAM_STR_OR_LONG_OR_NULL(str_seed, int_seed, seed_is_null);
	CREX_PARSE_PARAMETERS_END();

	if (seed_is_null) {
		if (crx_random_bytes_throw(&state->state, sizeof(crx_random_uint128_t)) == FAILURE) {
			crex_throw_exception(random_ce_Random_RandomException, "Failed to generate a random seed", 0);
			RETURN_THROWS();
		}
	} else {
		if (str_seed) {
			/* char (byte: 8 bit) * 16 = 128 bits */
			if (ZSTR_LEN(str_seed) == 16) {
				/* Endianness safe copy */
				for (i = 0; i < 2; i++) {
					t[i] = 0;
					for (j = 0; j < 8; j++) {
						t[i] += ((uint64_t) (unsigned char) ZSTR_VAL(str_seed)[(i * 8) + j]) << (j * 8);
					}
				}
				seed128(engine->status, crx_random_uint128_constant(t[0], t[1]));
			} else {
				crex_argument_value_error(1, "must be a 16 byte (128 bit) string");
				RETURN_THROWS();
			}
		} else {
			engine->algo->seed(engine->status, int_seed);
		}
	}
}
/* }}} */

/* {{{ Random\Engine\PcgOneseq128XslRr64::jump() */
CRX_METHOD(Random_Engine_PcgOneseq128XslRr64, jump)
{
	crx_random_engine *engine = C_RANDOM_ENGINE_P(CREX_THIS);
	crx_random_status_state_pcgoneseq128xslrr64 *state = engine->status->state;
	crex_long advance = 0;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(advance);
	CREX_PARSE_PARAMETERS_END();

	if (UNEXPECTED(advance < 0)) {
		crex_argument_value_error(1, "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	crx_random_pcgoneseq128xslrr64_advance(state, advance);
}
/* }}} */
