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
   | Authors: Rasmus Lerdorf <rasmus@crx.net>                             |
   |          Zeev Suraski <zeev@crx.net>                                 |
   |          Sascha Schumann <sascha@schumann.cx>                        |
   |          Pedro Melo <melo@ip.pt>                                     |
   |          Sterling Hughes <sterling@crx.net>                          |
   |          Sammy Kaye Powers <me@sammyk.me>                            |
   |          Go Kudo <zeriyoshi@crx.net>                                 |
   |                                                                      |
   | Based on code from: Richard J. Wagner <rjwagner@writeme.com>         |
   |                     Makoto Matsumoto <matumoto@math.keio.ac.jp>      |
   |                     Takuji Nishimura                                 |
   |                     Shawn Cokus <Cokus@math.washington.edu>          |
   |                     David Blackman                                   |
   |                     Sebastiano Vigna <vigna@acm.org>                 |
   |                     Melissa O'Neill <oneill@pcg-random.org>          |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_RANDOM_H
# define CRX_RANDOM_H

# include "crx.h"

CRXAPI double crx_combined_lcg(void);

/*
 * A bit of tricky math here.  We want to avoid using a modulus because
 * that simply tosses the high-order bits and might skew the distribution
 * of random values over the range.  Instead we map the range directly.
 *
 * We need to map the range from 0...M evenly to the range a...b
 * Let n = the random number and n' = the mapped random number
 *
 * Then we have: n' = a + n(b-a)/M
 *
 * We have a problem here in that only n==M will get mapped to b which
 * means the chances of getting b is much much less than getting any of
 * the other values in the range.  We can fix this by increasing our range
 * artificially and using:
 *
 *               n' = a + n(b-a+1)/M
 *
 * Now we only have a problem if n==M which would cause us to produce a
 * number of b+1 which would be bad.  So we bump M up by one to make sure
 * this will never happen, and the final algorithm looks like this:
 *
 *               n' = a + n(b-a+1)/(M+1)
 *
 * -RL
 */
# define RAND_RANGE_BADSCALING(__n, __min, __max, __tmax) \
	(__n) = (__min) + (crex_long) ((double) ( (double) (__max) - (__min) + 1.0) * ((__n) / ((__tmax) + 1.0)))

# ifdef CRX_WIN32
#  define GENERATE_SEED() (((crex_long) ((crex_ulong) time(NULL) * (crex_ulong) GetCurrentProcessId())) ^ ((crex_long) (1000000.0 * crx_combined_lcg())))
# else
#  define GENERATE_SEED() (((crex_long) ((crex_ulong) time(NULL) * (crex_ulong) getpid())) ^ ((crex_long) (1000000.0 * crx_combined_lcg())))
# endif

# define CRX_MT_RAND_MAX ((crex_long) (0x7FFFFFFF)) /* (1<<31) - 1 */

# define MT_RAND_MT19937 0
# define MT_RAND_CRX 1

# define MT_N (624)

#define CRX_RANDOM_RANGE_ATTEMPTS (50)

CRXAPI void crx_mt_srand(uint32_t seed);
CRXAPI uint32_t crx_mt_rand(void);
CRXAPI crex_long crx_mt_rand_range(crex_long min, crex_long max);
CRXAPI crex_long crx_mt_rand_common(crex_long min, crex_long max);

# ifndef RAND_MAX
#  define RAND_MAX CRX_MT_RAND_MAX
# endif

# define CRX_RAND_MAX CRX_MT_RAND_MAX

CRXAPI void crx_srand(crex_long seed);
CRXAPI crex_long crx_rand(void);

# if !defined(__SIZEOF_INT128__) || defined(CRX_RANDOM_FORCE_EMULATE_128)
typedef struct _crx_random_uint128_t {
	uint64_t hi;
	uint64_t lo;
} crx_random_uint128_t;

static inline uint64_t crx_random_uint128_hi(crx_random_uint128_t num)
{
	return num.hi;
}

static inline uint64_t crx_random_uint128_lo(crx_random_uint128_t num)
{
	return num.lo;
}

static inline crx_random_uint128_t crx_random_uint128_constant(uint64_t hi, uint64_t lo)
{
	crx_random_uint128_t r;

	r.hi = hi;
	r.lo = lo;

	return r;
}

static inline crx_random_uint128_t crx_random_uint128_add(crx_random_uint128_t num1, crx_random_uint128_t num2)
{
	crx_random_uint128_t r;

	r.lo = (num1.lo + num2.lo);
	r.hi = (num1.hi + num2.hi + (r.lo < num1.lo));

	return r;
}

static inline crx_random_uint128_t crx_random_uint128_multiply(crx_random_uint128_t num1, crx_random_uint128_t num2)
{
	crx_random_uint128_t r;
	const uint64_t
		x0 = num1.lo & 0xffffffffULL,
		x1 = num1.lo >> 32,
		y0 = num2.lo & 0xffffffffULL,
		y1 = num2.lo >> 32,
		z0 = (((x1 * y0) + (x0 * y0 >> 32)) & 0xffffffffULL) + x0 * y1;

	r.hi = num1.hi * num2.lo + num1.lo * num2.hi;
	r.lo = num1.lo * num2.lo;
	r.hi += x1 * y1 + ((x1 * y0 + (x0 * y0 >> 32)) >> 32) + (z0 >> 32);

	return r;
}

static inline uint64_t crx_random_pcgoneseq128xslrr64_rotr64(crx_random_uint128_t num)
{
	const uint64_t
		v = (num.hi ^ num.lo),
		s = num.hi >> 58U;

	return (v >> s) | (v << ((-s) & 63));
}
# else
typedef __uint128_t crx_random_uint128_t;

static inline uint64_t crx_random_uint128_hi(crx_random_uint128_t num)
{
	return (uint64_t) (num >> 64);
}

static inline uint64_t crx_random_uint128_lo(crx_random_uint128_t num)
{
	return (uint64_t) num;
}

static inline crx_random_uint128_t crx_random_uint128_constant(uint64_t hi, uint64_t lo)
{
	crx_random_uint128_t r;

	r = ((crx_random_uint128_t) hi << 64) + lo;

	return r;
}

static inline crx_random_uint128_t crx_random_uint128_add(crx_random_uint128_t num1, crx_random_uint128_t num2)
{
	return num1 + num2;
}

static inline crx_random_uint128_t crx_random_uint128_multiply(crx_random_uint128_t num1, crx_random_uint128_t num2)
{
	return num1 * num2;
}

static inline uint64_t crx_random_pcgoneseq128xslrr64_rotr64(crx_random_uint128_t num)
{
	const uint64_t
		v = ((uint64_t) (num >> 64U)) ^ (uint64_t) num,
		s = num >> 122U;

	return (v >> s) | (v << ((-s) & 63));
}
# endif

CRXAPI crex_result crx_random_bytes(void *bytes, size_t size, bool should_throw);
CRXAPI crex_result crx_random_int(crex_long min, crex_long max, crex_long *result, bool should_throw);

static inline crex_result crx_random_bytes_throw(void *bytes, size_t size)
{
	return crx_random_bytes(bytes, size, true);
}

static inline crex_result crx_random_bytes_silent(void *bytes, size_t size)
{
	return crx_random_bytes(bytes, size, false);
}

static inline crex_result crx_random_int_throw(crex_long min, crex_long max, crex_long *result)
{
	return crx_random_int(min, max, result, true);
}

static inline crex_result crx_random_int_silent(crex_long min, crex_long max, crex_long *result)
{
	return crx_random_int(min, max, result, false);
}

typedef struct _crx_random_status_ {
	size_t last_generated_size;
	void *state;
} crx_random_status;

typedef struct _crx_random_status_state_combinedlcg {
	int32_t state[2];
} crx_random_status_state_combinedlcg;

typedef struct _crx_random_status_state_mt19937 {
	uint32_t state[MT_N];
	uint32_t count;
	uint8_t mode;
} crx_random_status_state_mt19937;

typedef struct _crx_random_status_state_pcgoneseq128xslrr64 {
	crx_random_uint128_t state;
} crx_random_status_state_pcgoneseq128xslrr64;

typedef struct _crx_random_status_state_xoshiro256starstar {
	uint64_t state[4];
} crx_random_status_state_xoshiro256starstar;

typedef struct _crx_random_status_state_user {
	crex_object *object;
	crex_function *generate_method;
} crx_random_status_state_user;

typedef struct _crx_random_algo {
	const size_t generate_size;
	const size_t state_size;
	void (*seed)(crx_random_status *status, uint64_t seed);
	uint64_t (*generate)(crx_random_status *status);
	crex_long (*range)(crx_random_status *status, crex_long min, crex_long max);
	bool (*serialize)(crx_random_status *status, HashTable *data);
	bool (*unserialize)(crx_random_status *status, HashTable *data);
} crx_random_algo;

extern CRXAPI const crx_random_algo crx_random_algo_combinedlcg;
extern CRXAPI const crx_random_algo crx_random_algo_mt19937;
extern CRXAPI const crx_random_algo crx_random_algo_pcgoneseq128xslrr64;
extern CRXAPI const crx_random_algo crx_random_algo_xoshiro256starstar;
extern CRXAPI const crx_random_algo crx_random_algo_secure;
extern CRXAPI const crx_random_algo crx_random_algo_user;

typedef struct _crx_random_engine {
	const crx_random_algo *algo;
	crx_random_status *status;
	crex_object std;
} crx_random_engine;

typedef struct _crx_random_randomizer {
	const crx_random_algo *algo;
	crx_random_status *status;
	bool is_userland_algo;
	crex_object std;
} crx_random_randomizer;

extern CRXAPI crex_class_entry *random_ce_Random_Engine;
extern CRXAPI crex_class_entry *random_ce_Random_CryptoSafeEngine;

extern CRXAPI crex_class_entry *random_ce_Random_RandomError;
extern CRXAPI crex_class_entry *random_ce_Random_BrokenRandomEngineError;
extern CRXAPI crex_class_entry *random_ce_Random_RandomException;

extern CRXAPI crex_class_entry *random_ce_Random_Engine_PcgOneseq128XslRr64;
extern CRXAPI crex_class_entry *random_ce_Random_Engine_Mt19937;
extern CRXAPI crex_class_entry *random_ce_Random_Engine_Xoshiro256StarStar;
extern CRXAPI crex_class_entry *random_ce_Random_Engine_Secure;

extern CRXAPI crex_class_entry *random_ce_Random_Randomizer;

extern CRXAPI crex_class_entry *random_ce_Random_IntervalBoundary;

static inline crx_random_engine *crx_random_engine_from_obj(crex_object *object) {
	return (crx_random_engine *)((char *)(object) - XtOffsetOf(crx_random_engine, std));
}

static inline crx_random_randomizer *crx_random_randomizer_from_obj(crex_object *object) {
	return (crx_random_randomizer *)((char *)(object) - XtOffsetOf(crx_random_randomizer, std));
}

# define C_RANDOM_ENGINE_P(zval) crx_random_engine_from_obj(C_OBJ_P(zval))

# define C_RANDOM_RANDOMIZER_P(zval) crx_random_randomizer_from_obj(C_OBJ_P(zval));

CRXAPI crx_random_status *crx_random_status_alloc(const crx_random_algo *algo, const bool persistent);
CRXAPI crx_random_status *crx_random_status_copy(const crx_random_algo *algo, crx_random_status *old_status, crx_random_status *new_status);
CRXAPI void crx_random_status_free(crx_random_status *status, const bool persistent);
CRXAPI crx_random_engine *crx_random_engine_common_init(crex_class_entry *ce, crex_object_handlers *handlers, const crx_random_algo *algo);
CRXAPI void crx_random_engine_common_free_object(crex_object *object);
CRXAPI crex_object *crx_random_engine_common_clone_object(crex_object *object);
CRXAPI uint32_t crx_random_range32(const crx_random_algo *algo, crx_random_status *status, uint32_t umax);
CRXAPI uint64_t crx_random_range64(const crx_random_algo *algo, crx_random_status *status, uint64_t umax);
CRXAPI crex_long crx_random_range(const crx_random_algo *algo, crx_random_status *status, crex_long min, crex_long max);
CRXAPI const crx_random_algo *crx_random_default_algo(void);
CRXAPI crx_random_status *crx_random_default_status(void);

CRXAPI crex_string *crx_random_bin2hex_le(const void *ptr, const size_t len);
CRXAPI bool crx_random_hex2bin_le(crex_string *hexstr, void *dest);

CRXAPI void crx_random_combinedlcg_seed_default(crx_random_status_state_combinedlcg *state);

CRXAPI void crx_random_mt19937_seed_default(crx_random_status_state_mt19937 *state);

CRXAPI void crx_random_pcgoneseq128xslrr64_advance(crx_random_status_state_pcgoneseq128xslrr64 *state, uint64_t advance);

CRXAPI void crx_random_xoshiro256starstar_jump(crx_random_status_state_xoshiro256starstar *state);
CRXAPI void crx_random_xoshiro256starstar_jump_long(crx_random_status_state_xoshiro256starstar *state);

CRXAPI double crx_random_gammasection_closed_open(const crx_random_algo *algo, crx_random_status *status, double min, double max);
CRXAPI double crx_random_gammasection_closed_closed(const crx_random_algo *algo, crx_random_status *status, double min, double max);
CRXAPI double crx_random_gammasection_open_closed(const crx_random_algo *algo, crx_random_status *status, double min, double max);
CRXAPI double crx_random_gammasection_open_open(const crx_random_algo *algo, crx_random_status *status, double min, double max);

extern crex_module_entry random_module_entry;
# define crxext_random_ptr &random_module_entry

CRX_MINIT_FUNCTION(random);
CRX_MSHUTDOWN_FUNCTION(random);
CRX_RINIT_FUNCTION(random);

CREX_BEGIN_MODULE_GLOBALS(random)
	crx_random_status *combined_lcg;
	bool combined_lcg_seeded;
	crx_random_status *mt19937;
	bool mt19937_seeded;
	int random_fd;
CREX_END_MODULE_GLOBALS(random)

CRXAPI CREX_EXTERN_MODULE_GLOBALS(random)

# define RANDOM_G(v)	CREX_MODULE_GLOBALS_ACCESSOR(random, v)

#endif	/* CRX_RANDOM_H */
