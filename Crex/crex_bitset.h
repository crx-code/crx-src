/*
   +----------------------------------------------------------------------+
   | Crex OPcache JIT                                                     |
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
   | Authors: Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef _CREX_BITSET_H_
#define _CREX_BITSET_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "crex_portability.h"
#include "crex_long.h"

typedef crex_ulong *crex_bitset;

#define CREX_BITSET_ELM_SIZE sizeof(crex_ulong)

#if SIZEOF_CREX_LONG == 4
# define CREX_BITSET_ELM_NUM(n)		((n) >> 5)
# define CREX_BITSET_BIT_NUM(n)		((crex_ulong)(n) & C_UL(0x1f))
#elif SIZEOF_CREX_LONG == 8
# define CREX_BITSET_ELM_NUM(n)		((n) >> 6)
# define CREX_BITSET_BIT_NUM(n)		((crex_ulong)(n) & C_UL(0x3f))
#else
# define CREX_BITSET_ELM_NUM(n)		((n) / (sizeof(crex_long) * 8))
# define CREX_BITSET_BIT_NUM(n)		((n) % (sizeof(crex_long) * 8))
#endif

#define CREX_BITSET_ALLOCA(n, use_heap) \
	(crex_bitset)do_alloca((n) * CREX_BITSET_ELM_SIZE, use_heap)

/* Number of trailing zero bits (0x01 -> 0; 0x40 -> 6; 0x00 -> LEN) */
static crex_always_inline int crex_ulong_ntz(crex_ulong num)
{
#if (defined(__GNUC__) || __has_builtin(__builtin_ctzl)) \
	&& SIZEOF_CREX_LONG == SIZEOF_LONG && defined(CRX_HAVE_BUILTIN_CTZL)
	return __builtin_ctzl(num);
#elif (defined(__GNUC__) || __has_builtin(__builtin_ctzll)) && defined(CRX_HAVE_BUILTIN_CTZLL)
	return __builtin_ctzll(num);
#elif defined(_WIN32)
	unsigned long index;

#if defined(_WIN64)
	if (!BitScanForward64(&index, num)) {
#else
	if (!BitScanForward(&index, num)) {
#endif
		/* undefined behavior */
		return SIZEOF_CREX_LONG * 8;
	}

	return (int) index;
#else
	int n;

	if (num == C_UL(0)) return SIZEOF_CREX_LONG * 8;

	n = 1;
#if SIZEOF_CREX_LONG == 8
	if ((num & 0xffffffff) == 0) {n += 32; num = num >> C_UL(32);}
#endif
	if ((num & 0x0000ffff) == 0) {n += 16; num = num >> 16;}
	if ((num & 0x000000ff) == 0) {n +=  8; num = num >>  8;}
	if ((num & 0x0000000f) == 0) {n +=  4; num = num >>  4;}
	if ((num & 0x00000003) == 0) {n +=  2; num = num >>  2;}
	return n - (num & 1);
#endif
}

/* Number of leading zero bits (Undefined for zero) */
static crex_always_inline int crex_ulong_nlz(crex_ulong num)
{
#if (defined(__GNUC__) || __has_builtin(__builtin_clzl)) \
	&& SIZEOF_CREX_LONG == SIZEOF_LONG && defined(CRX_HAVE_BUILTIN_CLZL)
	return __builtin_clzl(num);
#elif (defined(__GNUC__) || __has_builtin(__builtin_clzll)) && defined(CRX_HAVE_BUILTIN_CLZLL)
	return __builtin_clzll(num);
#elif defined(_WIN32)
	unsigned long index;

#if defined(_WIN64)
	if (!BitScanReverse64(&index, num)) {
#else
	if (!BitScanReverse(&index, num)) {
#endif
		/* undefined behavior */
		return SIZEOF_CREX_LONG * 8;
	}

	return (int) (SIZEOF_CREX_LONG * 8 - 1)- index;
#else
	crex_ulong x;
	int n;

#if SIZEOF_CREX_LONG == 8
	n = 64;
	x = num >> 32; if (x != 0) {n -= 32; num = x;}
#else
	n = 32;
#endif
	x = num >> 16; if (x != 0) {n -= 16; num = x;}
	x = num >> 8;  if (x != 0) {n -=  8; num = x;}
	x = num >> 4;  if (x != 0) {n -=  4; num = x;}
	x = num >> 2;  if (x != 0) {n -=  2; num = x;}
	x = num >> 1;  if (x != 0) return n - 2;
	return n - num;
#endif
}

/* Returns the number of crex_ulong words needed to store a bitset that is N
   bits long.  */
static inline uint32_t crex_bitset_len(uint32_t n)
{
	return (n + ((sizeof(crex_long) * 8) - 1)) / (sizeof(crex_long) * 8);
}

static inline bool crex_bitset_in(crex_bitset set, uint32_t n)
{
	return CREX_BIT_TEST(set, n);
}

static inline void crex_bitset_incl(crex_bitset set, uint32_t n)
{
	set[CREX_BITSET_ELM_NUM(n)] |= C_UL(1) << CREX_BITSET_BIT_NUM(n);
}

static inline void crex_bitset_excl(crex_bitset set, uint32_t n)
{
	set[CREX_BITSET_ELM_NUM(n)] &= ~(C_UL(1) << CREX_BITSET_BIT_NUM(n));
}

static inline void crex_bitset_clear(crex_bitset set, uint32_t len)
{
	memset(set, 0, len * CREX_BITSET_ELM_SIZE);
}

static inline bool crex_bitset_empty(crex_bitset set, uint32_t len)
{
	uint32_t i;
	for (i = 0; i < len; i++) {
		if (set[i]) {
			return 0;
		}
	}
	return 1;
}

static inline void crex_bitset_fill(crex_bitset set, uint32_t len)
{
	memset(set, 0xff, len * CREX_BITSET_ELM_SIZE);
}

static inline bool crex_bitset_equal(crex_bitset set1, crex_bitset set2, uint32_t len)
{
    return memcmp(set1, set2, len * CREX_BITSET_ELM_SIZE) == 0;
}

static inline void crex_bitset_copy(crex_bitset set1, crex_bitset set2, uint32_t len)
{
    memcpy(set1, set2, len * CREX_BITSET_ELM_SIZE);
}

static inline void crex_bitset_intersection(crex_bitset set1, crex_bitset set2, uint32_t len)
{
    uint32_t i;

    for (i = 0; i < len; i++) {
		set1[i] &= set2[i];
	}
}

static inline void crex_bitset_union(crex_bitset set1, crex_bitset set2, uint32_t len)
{
	uint32_t i;

	for (i = 0; i < len; i++) {
		set1[i] |= set2[i];
	}
}

static inline void crex_bitset_difference(crex_bitset set1, crex_bitset set2, uint32_t len)
{
	uint32_t i;

	for (i = 0; i < len; i++) {
		set1[i] = set1[i] & ~set2[i];
	}
}

static inline void crex_bitset_union_with_intersection(crex_bitset set1, crex_bitset set2, crex_bitset set3, crex_bitset set4, uint32_t len)
{
	uint32_t i;

	for (i = 0; i < len; i++) {
		set1[i] = set2[i] | (set3[i] & set4[i]);
	}
}

static inline void crex_bitset_union_with_difference(crex_bitset set1, crex_bitset set2, crex_bitset set3, crex_bitset set4, uint32_t len)
{
	uint32_t i;

	for (i = 0; i < len; i++) {
		set1[i] = set2[i] | (set3[i] & ~set4[i]);
	}
}

static inline bool crex_bitset_subset(crex_bitset set1, crex_bitset set2, uint32_t len)
{
	uint32_t i;

	for (i = 0; i < len; i++) {
		if (set1[i] & ~set2[i]) {
			return 0;
		}
	}
	return 1;
}

static inline int crex_bitset_first(crex_bitset set, uint32_t len)
{
	uint32_t i;

	for (i = 0; i < len; i++) {
		if (set[i]) {
			return CREX_BITSET_ELM_SIZE * 8 * i + crex_ulong_ntz(set[i]);
		}
	}
	return -1; /* empty set */
}

static inline int crex_bitset_last(crex_bitset set, uint32_t len)
{
	uint32_t i = len;

	while (i > 0) {
		i--;
		if (set[i]) {
			int j = CREX_BITSET_ELM_SIZE * 8 * i - 1;
			crex_ulong x = set[i];
			while (x != C_UL(0)) {
				x = x >> C_UL(1);
				j++;
			}
			return j;
		}
	}
	return -1; /* empty set */
}

#define CREX_BITSET_FOREACH(set, len, bit) do { \
	crex_bitset _set = (set); \
	uint32_t _i, _len = (len); \
	for (_i = 0; _i < _len; _i++) { \
		crex_ulong _x = _set[_i]; \
		if (_x) { \
			(bit) = CREX_BITSET_ELM_SIZE * 8 * _i; \
			for (; _x != 0; _x >>= C_UL(1), (bit)++) { \
				if (!(_x & C_UL(1))) continue;

#define CREX_BITSET_REVERSE_FOREACH(set, len, bit) do { \
	crex_bitset _set = (set); \
	uint32_t _i = (len); \
	crex_ulong _test = C_UL(1) << (CREX_BITSET_ELM_SIZE * 8 - 1); \
	while (_i-- > 0) { \
		crex_ulong _x = _set[_i]; \
		if (_x) { \
			(bit) = CREX_BITSET_ELM_SIZE * 8 * (_i + 1) - 1; \
			for (; _x != 0; _x <<= C_UL(1), (bit)--) { \
				if (!(_x & _test)) continue; \

#define CREX_BITSET_FOREACH_END() \
			} \
		} \
	} \
} while (0)

static inline int crex_bitset_pop_first(crex_bitset set, uint32_t len) {
	int i = crex_bitset_first(set, len);
	if (i >= 0) {
		crex_bitset_excl(set, i);
	}
	return i;
}

#endif /* _CREX_BITSET_H_ */
