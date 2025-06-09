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

#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#include "crx.h"

#include "Crex/crex_enum.h"
#include "Crex/crex_exceptions.h"

#include "crx_random.h"

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef CRX_WIN32
# include "win32/time.h"
# include "win32/winutil.h"
# include <process.h>
#else
# include <sys/time.h>
#endif

#if HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif

#include "random_arginfo.h"

CRXAPI CREX_DECLARE_MODULE_GLOBALS(random)

CRXAPI crex_class_entry *random_ce_Random_Engine;
CRXAPI crex_class_entry *random_ce_Random_CryptoSafeEngine;

CRXAPI crex_class_entry *random_ce_Random_Engine_Mt19937;
CRXAPI crex_class_entry *random_ce_Random_Engine_PcgOneseq128XslRr64;
CRXAPI crex_class_entry *random_ce_Random_Engine_Xoshiro256StarStar;
CRXAPI crex_class_entry *random_ce_Random_Engine_Secure;

CRXAPI crex_class_entry *random_ce_Random_Randomizer;

CRXAPI crex_class_entry *random_ce_Random_IntervalBoundary;

CRXAPI crex_class_entry *random_ce_Random_RandomError;
CRXAPI crex_class_entry *random_ce_Random_BrokenRandomEngineError;
CRXAPI crex_class_entry *random_ce_Random_RandomException;

static crex_object_handlers random_engine_mt19937_object_handlers;
static crex_object_handlers random_engine_pcgoneseq128xslrr64_object_handlers;
static crex_object_handlers random_engine_xoshiro256starstar_object_handlers;
static crex_object_handlers random_engine_secure_object_handlers;
static crex_object_handlers random_randomizer_object_handlers;

CRXAPI uint32_t crx_random_range32(const crx_random_algo *algo, crx_random_status *status, uint32_t umax)
{
	uint32_t result, limit;
	size_t total_size = 0;
	uint32_t count = 0;

	result = 0;
	total_size = 0;
	do {
		uint32_t r = algo->generate(status);
		result = result | (r << (total_size * 8));
		total_size += status->last_generated_size;
		if (EG(exception)) {
			return 0;
		}
	} while (total_size < sizeof(uint32_t));

	/* Special case where no modulus is required */
	if (UNEXPECTED(umax == UINT32_MAX)) {
		return result;
	}

	/* Increment the max so range is inclusive of max */
	umax++;

	/* Powers of two are not biased */
	if ((umax & (umax - 1)) == 0) {
		return result & (umax - 1);
	}

	/* Ceiling under which UINT32_MAX % max == 0 */
	limit = UINT32_MAX - (UINT32_MAX % umax) - 1;

	/* Discard numbers over the limit to avoid modulo bias */
	while (UNEXPECTED(result > limit)) {
		/* If the requirements cannot be met in a cycles, return fail */
		if (++count > CRX_RANDOM_RANGE_ATTEMPTS) {
			crex_throw_error(random_ce_Random_BrokenRandomEngineError, "Failed to generate an acceptable random number in %d attempts", CRX_RANDOM_RANGE_ATTEMPTS);
			return 0;
		}

		result = 0;
		total_size = 0;
		do {
			uint32_t r = algo->generate(status);
			result = result | (r << (total_size * 8));
			total_size += status->last_generated_size;
			if (EG(exception)) {
				return 0;
			}
		} while (total_size < sizeof(uint32_t));
	}

	return result % umax;
}

CRXAPI uint64_t crx_random_range64(const crx_random_algo *algo, crx_random_status *status, uint64_t umax)
{
	uint64_t result, limit;
	size_t total_size = 0;
	uint32_t count = 0;

	result = 0;
	total_size = 0;
	do {
		uint64_t r = algo->generate(status);
		result = result | (r << (total_size * 8));
		total_size += status->last_generated_size;
		if (EG(exception)) {
			return 0;
		}
	} while (total_size < sizeof(uint64_t));

	/* Special case where no modulus is required */
	if (UNEXPECTED(umax == UINT64_MAX)) {
		return result;
	}

	/* Increment the max so range is inclusive of max */
	umax++;

	/* Powers of two are not biased */
	if ((umax & (umax - 1)) == 0) {
		return result & (umax - 1);
	}

	/* Ceiling under which UINT64_MAX % max == 0 */
	limit = UINT64_MAX - (UINT64_MAX % umax) - 1;

	/* Discard numbers over the limit to avoid modulo bias */
	while (UNEXPECTED(result > limit)) {
		/* If the requirements cannot be met in a cycles, return fail */
		if (++count > CRX_RANDOM_RANGE_ATTEMPTS) {
			crex_throw_error(random_ce_Random_BrokenRandomEngineError, "Failed to generate an acceptable random number in %d attempts", CRX_RANDOM_RANGE_ATTEMPTS);
			return 0;
		}

		result = 0;
		total_size = 0;
		do {
			uint64_t r = algo->generate(status);
			result = result | (r << (total_size * 8));
			total_size += status->last_generated_size;
			if (EG(exception)) {
				return 0;
			}
		} while (total_size < sizeof(uint64_t));
	}

	return result % umax;
}

static crex_object *crx_random_engine_mt19937_new(crex_class_entry *ce)
{
	return &crx_random_engine_common_init(ce, &random_engine_mt19937_object_handlers, &crx_random_algo_mt19937)->std;
}

static crex_object *crx_random_engine_pcgoneseq128xslrr64_new(crex_class_entry *ce)
{
	return &crx_random_engine_common_init(ce, &random_engine_pcgoneseq128xslrr64_object_handlers, &crx_random_algo_pcgoneseq128xslrr64)->std;
}

static crex_object *crx_random_engine_xoshiro256starstar_new(crex_class_entry *ce)
{
	return &crx_random_engine_common_init(ce, &random_engine_xoshiro256starstar_object_handlers, &crx_random_algo_xoshiro256starstar)->std;
}

static crex_object *crx_random_engine_secure_new(crex_class_entry *ce)
{
	return &crx_random_engine_common_init(ce, &random_engine_secure_object_handlers, &crx_random_algo_secure)->std;
}

static crex_object *crx_random_randomizer_new(crex_class_entry *ce)
{
	crx_random_randomizer *randomizer = crex_object_alloc(sizeof(crx_random_randomizer), ce);

	crex_object_std_init(&randomizer->std, ce);
	object_properties_init(&randomizer->std, ce);

	return &randomizer->std;
}

static void randomizer_free_obj(crex_object *object) {
	crx_random_randomizer *randomizer = crx_random_randomizer_from_obj(object);

	if (randomizer->is_userland_algo) {
		crx_random_status_free(randomizer->status, false);
	}

	crex_object_std_dtor(&randomizer->std);
}

CRXAPI crx_random_status *crx_random_status_alloc(const crx_random_algo *algo, const bool persistent)
{
	crx_random_status *status = pecalloc(1, sizeof(crx_random_status), persistent);

	status->last_generated_size = algo->generate_size;
	status->state = algo->state_size > 0 ? pecalloc(1, algo->state_size, persistent) : NULL;

	return status;
}

CRXAPI crx_random_status *crx_random_status_copy(const crx_random_algo *algo, crx_random_status *old_status, crx_random_status *new_status)
{
	new_status->last_generated_size = old_status->last_generated_size;
	new_status->state = memcpy(new_status->state, old_status->state, algo->state_size);

	return new_status;
}

CRXAPI void crx_random_status_free(crx_random_status *status, const bool persistent)
{
	if (status != NULL) {
		pefree(status->state, persistent);
	}

	pefree(status, persistent);
}

CRXAPI crx_random_engine *crx_random_engine_common_init(crex_class_entry *ce, crex_object_handlers *handlers, const crx_random_algo *algo)
{
	crx_random_engine *engine = crex_object_alloc(sizeof(crx_random_engine), ce);

	crex_object_std_init(&engine->std, ce);
	object_properties_init(&engine->std, ce);

	engine->algo = algo;
	engine->status = crx_random_status_alloc(engine->algo, false);
	engine->std.handlers = handlers;

	return engine;
}

CRXAPI void crx_random_engine_common_free_object(crex_object *object)
{
	crx_random_engine *engine = crx_random_engine_from_obj(object);

	crx_random_status_free(engine->status, false);
	crex_object_std_dtor(object);
}

CRXAPI crex_object *crx_random_engine_common_clone_object(crex_object *object)
{
	crx_random_engine *old_engine = crx_random_engine_from_obj(object);
	crx_random_engine *new_engine = crx_random_engine_from_obj(old_engine->std.ce->create_object(old_engine->std.ce));

	new_engine->algo = old_engine->algo;
	if (old_engine->status) {
		new_engine->status = crx_random_status_copy(old_engine->algo, old_engine->status, new_engine->status);
	}

	crex_objects_clone_members(&new_engine->std, &old_engine->std);

	return &new_engine->std;
}

/* {{{ crx_random_range */
CRXAPI crex_long crx_random_range(const crx_random_algo *algo, crx_random_status *status, crex_long min, crex_long max)
{
	crex_ulong umax = (crex_ulong) max - (crex_ulong) min;

	if (umax > UINT32_MAX) {
		return (crex_long) (crx_random_range64(algo, status, umax) + min);
	}

	return (crex_long) (crx_random_range32(algo, status, umax) + min);
}
/* }}} */

/* {{{ crx_random_default_algo */
CRXAPI const crx_random_algo *crx_random_default_algo(void)
{
	return &crx_random_algo_mt19937;
}
/* }}} */

/* {{{ crx_random_default_status */
CRXAPI crx_random_status *crx_random_default_status(void)
{
	crx_random_status *status = RANDOM_G(mt19937);

	if (!RANDOM_G(mt19937_seeded)) {
		crx_random_mt19937_seed_default(status->state);
		RANDOM_G(mt19937_seeded) = true;
	}

	return status;
}
/* }}} */

/* this is read-only, so it's ok */
CREX_SET_ALIGNED(16, static const char hexconvtab[]) = "0123456789abcdef";

/* {{{ crx_random_bin2hex_le */
/* stolen from standard/string.c */
CRXAPI crex_string *crx_random_bin2hex_le(const void *ptr, const size_t len)
{
	crex_string *str;
	size_t i;

	str = crex_string_safe_alloc(len, 2 * sizeof(char), 0, 0);

	i = 0;
#ifdef WORDS_BIGENDIAN
	/* force little endian */
	for (crex_long j = (len - 1); 0 <= j; j--) {
#else
	for (crex_long j = 0; j < len; j++) {
#endif
		ZSTR_VAL(str)[i++] = hexconvtab[((unsigned char *) ptr)[j] >> 4];
		ZSTR_VAL(str)[i++] = hexconvtab[((unsigned char *) ptr)[j] & 15];
	}
	ZSTR_VAL(str)[i] = '\0';

	return str;
}
/* }}} */

/* {{{ crx_random_hex2bin_le */
/* stolen from standard/string.c */
CRXAPI bool crx_random_hex2bin_le(crex_string *hexstr, void *dest)
{
	size_t len = hexstr->len >> 1;
	unsigned char *str = (unsigned char *) hexstr->val, c, l, d;
	unsigned char *ptr = (unsigned char *) dest;
	int is_letter, i = 0;

#ifdef WORDS_BIGENDIAN
	/* force little endian */
	for (crex_long j = (len - 1); 0 <= j; j--) {
#else
	for (crex_long j = 0; j < len; j++) {
#endif
		c = str[i++];
		l = c & ~0x20;
		is_letter = ((uint32_t) ((l - 'A') ^ (l - 'F' - 1))) >> (8 * sizeof(uint32_t) - 1);

		/* basically (c >= '0' && c <= '9') || (l >= 'A' && l <= 'F') */
		if (EXPECTED((((c ^ '0') - 10) >> (8 * sizeof(uint32_t) - 1)) | is_letter)) {
			d = (l - 0x10 - 0x27 * is_letter) << 4;
		} else {
			return false;
		}
		c = str[i++];
		l = c & ~0x20;
		is_letter = ((uint32_t) ((l - 'A') ^ (l - 'F' - 1))) >> (8 * sizeof(uint32_t) - 1);
		if (EXPECTED((((c ^ '0') - 10) >> (8 * sizeof(uint32_t) - 1)) | is_letter)) {
			d |= l - 0x10 - 0x27 * is_letter;
		} else {
			return false;
		}
		ptr[j] = d;
	}
	return true;
}
/* }}} */

/* {{{ crx_combined_lcg */
CRXAPI double crx_combined_lcg(void)
{
	crx_random_status *status = RANDOM_G(combined_lcg);

	if (!RANDOM_G(combined_lcg_seeded)) {
		crx_random_combinedlcg_seed_default(status->state);
		RANDOM_G(combined_lcg_seeded) = true;
	}

	return crx_random_algo_combinedlcg.generate(status) * 4.656613e-10;
}
/* }}} */

/* {{{ crx_mt_srand */
CRXAPI void crx_mt_srand(uint32_t seed)
{
	/* Seed the generator with a simple uint32 */
	crx_random_algo_mt19937.seed(crx_random_default_status(), (crex_long) seed);
}
/* }}} */

/* {{{ crx_mt_rand */
CRXAPI uint32_t crx_mt_rand(void)
{
	return (uint32_t) crx_random_algo_mt19937.generate(crx_random_default_status());
}
/* }}} */

/* {{{ crx_mt_rand_range */
CRXAPI crex_long crx_mt_rand_range(crex_long min, crex_long max)
{
	return crx_random_algo_mt19937.range(crx_random_default_status(), min, max);
}
/* }}} */

/* {{{ crx_mt_rand_common
 * rand() allows min > max, mt_rand does not */
CRXAPI crex_long crx_mt_rand_common(crex_long min, crex_long max)
{
	crx_random_status *status = crx_random_default_status();
	crx_random_status_state_mt19937 *s = status->state;

	if (s->mode == MT_RAND_MT19937) {
		return crx_mt_rand_range(min, max);
	}

	uint64_t r = crx_random_algo_mt19937.generate(crx_random_default_status()) >> 1;

	/* This is an inlined version of the RAND_RANGE_BADSCALING macro that does not invoke UB when encountering
	 * (max - min) > CREX_LONG_MAX.
	 */
	crex_ulong offset = (double) ( (double) max - min + 1.0) * (r / (CRX_MT_RAND_MAX + 1.0));

	return (crex_long) (offset + min);
}
/* }}} */

/* {{{ crx_srand */
CRXAPI void crx_srand(crex_long seed)
{
	crx_mt_srand((uint32_t) seed);
}
/* }}} */

/* {{{ crx_rand */
CRXAPI crex_long crx_rand(void)
{
	return crx_mt_rand();
}
/* }}} */

/* {{{ Returns a value from the combined linear congruential generator */
CRX_FUNCTION(lcg_value)
{
	CREX_PARSE_PARAMETERS_NONE();

	RETURN_DOUBLE(crx_combined_lcg());
}
/* }}} */

/* {{{ Seeds Mersenne Twister random number generator */
CRX_FUNCTION(mt_srand)
{
	crex_long seed = 0;
	bool seed_is_null = true;
	crex_long mode = MT_RAND_MT19937;
	crx_random_status *status = RANDOM_G(mt19937);
	crx_random_status_state_mt19937 *state = status->state;

	CREX_PARSE_PARAMETERS_START(0, 2)
		C_PARAM_OPTIONAL
		C_PARAM_LONG_OR_NULL(seed, seed_is_null)
		C_PARAM_LONG(mode)
	CREX_PARSE_PARAMETERS_END();

	state->mode = mode;

	/* Anything that is not MT_RAND_MT19937 was interpreted as MT_RAND_CRX. */
	if (state->mode != MT_RAND_MT19937) {
		crex_error(E_DEPRECATED, "The MT_RAND_CRX variant of Mt19937 is deprecated");
	}

	if (seed_is_null) {
		crx_random_mt19937_seed_default(status->state);
	} else {
		crx_random_algo_mt19937.seed(status, (uint64_t) seed);
	}
	RANDOM_G(mt19937_seeded) = true;
}
/* }}} */

/* {{{ Returns a random number from Mersenne Twister */
CRX_FUNCTION(mt_rand)
{
	crex_long min, max;
	int argc = CREX_NUM_ARGS();

	if (argc == 0) {
		/* genrand_int31 in mt19937ar.c performs a right shift */
		RETURN_LONG(crx_mt_rand() >> 1);
	}

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_LONG(min)
		C_PARAM_LONG(max)
	CREX_PARSE_PARAMETERS_END();

	if (UNEXPECTED(max < min)) {
		crex_argument_value_error(2, "must be greater than or equal to argument #1 ($min)");
		RETURN_THROWS();
	}

	RETURN_LONG(crx_mt_rand_common(min, max));
}
/* }}} */

/* {{{ Returns the maximum value a random number from Mersenne Twister can have */
CRX_FUNCTION(mt_getrandmax)
{
	CREX_PARSE_PARAMETERS_NONE();

	/*
	 * Melo: it could be 2^^32 but we only use 2^^31 to maintain
	 * compatibility with the previous crx_rand
	 */
	RETURN_LONG(CRX_MT_RAND_MAX); /* 2^^31 */
}
/* }}} */

/* {{{ Returns a random number from Mersenne Twister */
CRX_FUNCTION(rand)
{
	crex_long min, max;
	int argc = CREX_NUM_ARGS();

	if (argc == 0) {
		/* genrand_int31 in mt19937ar.c performs a right shift */
		RETURN_LONG(crx_mt_rand() >> 1);
	}

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_LONG(min)
		C_PARAM_LONG(max)
	CREX_PARSE_PARAMETERS_END();

	if (max < min) {
		RETURN_LONG(crx_mt_rand_common(max, min));
	}

	RETURN_LONG(crx_mt_rand_common(min, max));
}
/* }}} */

/* {{{ Return an arbitrary length of pseudo-random bytes as binary string */
CRX_FUNCTION(random_bytes)
{
	crex_long size;
	crex_string *bytes;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(size)
	CREX_PARSE_PARAMETERS_END();

	if (size < 1) {
		crex_argument_value_error(1, "must be greater than 0");
		RETURN_THROWS();
	}

	bytes = crex_string_alloc(size, 0);

	if (crx_random_bytes_throw(ZSTR_VAL(bytes), size) == FAILURE) {
		crex_string_release_ex(bytes, 0);
		RETURN_THROWS();
	}

	ZSTR_VAL(bytes)[size] = '\0';

	RETURN_STR(bytes);
}
/* }}} */

/* {{{ Return an arbitrary pseudo-random integer */
CRX_FUNCTION(random_int)
{
	crex_long min, max, result;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_LONG(min)
		C_PARAM_LONG(max)
	CREX_PARSE_PARAMETERS_END();

	if (min > max) {
		crex_argument_value_error(1, "must be less than or equal to argument #2 ($max)");
		RETURN_THROWS();
	}

	if (crx_random_int_throw(min, max, &result) == FAILURE) {
		RETURN_THROWS();
	}

	RETURN_LONG(result);
}
/* }}} */

/* {{{ CRX_GINIT_FUNCTION */
static CRX_GINIT_FUNCTION(random)
{
	random_globals->random_fd = -1;

	random_globals->combined_lcg = crx_random_status_alloc(&crx_random_algo_combinedlcg, true);
	random_globals->combined_lcg_seeded = false;

	random_globals->mt19937 = crx_random_status_alloc(&crx_random_algo_mt19937, true);
	random_globals->mt19937_seeded = false;
}
/* }}} */

/* {{{ CRX_GSHUTDOWN_FUNCTION */
static CRX_GSHUTDOWN_FUNCTION(random)
{
	if (random_globals->random_fd >= 0) {
		close(random_globals->random_fd);
		random_globals->random_fd = -1;
	}

	crx_random_status_free(random_globals->combined_lcg, true);
	random_globals->combined_lcg = NULL;

	crx_random_status_free(random_globals->mt19937, true);
	random_globals->mt19937 = NULL;
}
/* }}} */

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(random)
{
	/* Random\Engine */
	random_ce_Random_Engine = register_class_Random_Engine();

	/* Random\CryptoSafeEngine */
	random_ce_Random_CryptoSafeEngine = register_class_Random_CryptoSafeEngine(random_ce_Random_Engine);

	/* Random\RandomError */
	random_ce_Random_RandomError = register_class_Random_RandomError(crex_ce_error);

	/* Random\BrokenRandomEngineError */
	random_ce_Random_BrokenRandomEngineError = register_class_Random_BrokenRandomEngineError(random_ce_Random_RandomError);

	/* Random\RandomException */
	random_ce_Random_RandomException = register_class_Random_RandomException(crex_ce_exception);

	/* Random\Engine\Mt19937 */
	random_ce_Random_Engine_Mt19937 = register_class_Random_Engine_Mt19937(random_ce_Random_Engine);
	random_ce_Random_Engine_Mt19937->create_object = crx_random_engine_mt19937_new;
	memcpy(&random_engine_mt19937_object_handlers, crex_get_std_object_handlers(), sizeof(crex_object_handlers));
	random_engine_mt19937_object_handlers.offset = XtOffsetOf(crx_random_engine, std);
	random_engine_mt19937_object_handlers.free_obj = crx_random_engine_common_free_object;
	random_engine_mt19937_object_handlers.clone_obj = crx_random_engine_common_clone_object;

	/* Random\Engine\PcgOnseq128XslRr64 */
	random_ce_Random_Engine_PcgOneseq128XslRr64 = register_class_Random_Engine_PcgOneseq128XslRr64(random_ce_Random_Engine);
	random_ce_Random_Engine_PcgOneseq128XslRr64->create_object = crx_random_engine_pcgoneseq128xslrr64_new;
	memcpy(&random_engine_pcgoneseq128xslrr64_object_handlers, crex_get_std_object_handlers(), sizeof(crex_object_handlers));
	random_engine_pcgoneseq128xslrr64_object_handlers.offset = XtOffsetOf(crx_random_engine, std);
	random_engine_pcgoneseq128xslrr64_object_handlers.free_obj = crx_random_engine_common_free_object;
	random_engine_pcgoneseq128xslrr64_object_handlers.clone_obj = crx_random_engine_common_clone_object;

	/* Random\Engine\Xoshiro256StarStar */
	random_ce_Random_Engine_Xoshiro256StarStar = register_class_Random_Engine_Xoshiro256StarStar(random_ce_Random_Engine);
	random_ce_Random_Engine_Xoshiro256StarStar->create_object = crx_random_engine_xoshiro256starstar_new;
	memcpy(&random_engine_xoshiro256starstar_object_handlers, crex_get_std_object_handlers(), sizeof(crex_object_handlers));
	random_engine_xoshiro256starstar_object_handlers.offset = XtOffsetOf(crx_random_engine, std);
	random_engine_xoshiro256starstar_object_handlers.free_obj = crx_random_engine_common_free_object;
	random_engine_xoshiro256starstar_object_handlers.clone_obj = crx_random_engine_common_clone_object;

	/* Random\Engine\Secure */
	random_ce_Random_Engine_Secure = register_class_Random_Engine_Secure(random_ce_Random_CryptoSafeEngine);
	random_ce_Random_Engine_Secure->create_object = crx_random_engine_secure_new;
	memcpy(&random_engine_secure_object_handlers, crex_get_std_object_handlers(), sizeof(crex_object_handlers));
	random_engine_secure_object_handlers.offset = XtOffsetOf(crx_random_engine, std);
	random_engine_secure_object_handlers.free_obj = crx_random_engine_common_free_object;
	random_engine_secure_object_handlers.clone_obj = NULL;

	/* Random\Randomizer */
	random_ce_Random_Randomizer = register_class_Random_Randomizer();
	random_ce_Random_Randomizer->create_object = crx_random_randomizer_new;
	random_ce_Random_Randomizer->default_object_handlers = &random_randomizer_object_handlers;
	memcpy(&random_randomizer_object_handlers, crex_get_std_object_handlers(), sizeof(crex_object_handlers));
	random_randomizer_object_handlers.offset = XtOffsetOf(crx_random_randomizer, std);
	random_randomizer_object_handlers.free_obj = randomizer_free_obj;
	random_randomizer_object_handlers.clone_obj = NULL;

	/* Random\IntervalBoundary */
	random_ce_Random_IntervalBoundary = register_class_Random_IntervalBoundary();

	register_random_symbols(module_number);

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_RINIT_FUNCTION */
CRX_RINIT_FUNCTION(random)
{
	RANDOM_G(combined_lcg_seeded) = false;
	RANDOM_G(mt19937_seeded) = false;

	return SUCCESS;
}
/* }}} */

/* {{{ random_module_entry */
crex_module_entry random_module_entry = {
	STANDARD_MODULE_HEADER,
	"random",					/* Extension name */
	ext_functions,				/* crex_function_entry */
	CRX_MINIT(random),			/* CRX_MINIT - Module initialization */
	NULL,						/* CRX_MSHUTDOWN - Module shutdown */
	CRX_RINIT(random),			/* CRX_RINIT - Request initialization */
	NULL,						/* CRX_RSHUTDOWN - Request shutdown */
	NULL,						/* CRX_MINFO - Module info */
	CRX_VERSION,				/* Version */
	CRX_MODULE_GLOBALS(random),	/* ZTS Module globals */
	CRX_GINIT(random),			/* CRX_GINIT - Global initialization */
	CRX_GSHUTDOWN(random),		/* CRX_GSHUTDOWN - Global shutdown */
	NULL,						/* Post deactivate */
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */
