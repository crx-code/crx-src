/*
   +----------------------------------------------------------------------+
   | Crex Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Crex Technologies Ltd. (http://www.crex.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_PORTABILITY_H
#define CREX_PORTABILITY_H

#ifdef __cplusplus
#define BEGIN_EXTERN_C() extern "C" {
#define END_EXTERN_C() }
#else
#define BEGIN_EXTERN_C()
#define END_EXTERN_C()
#endif

/*
 * general definitions
 */

#ifdef CREX_WIN32
# include "crex_config.w32.h"
# define CREX_PATHS_SEPARATOR		';'
#elif defined(__riscos__)
# include <crex_config.h>
# define CREX_PATHS_SEPARATOR		';'
#else
# include <crex_config.h>
# define CREX_PATHS_SEPARATOR		':'
#endif

#include "../TSRM/TSRM.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>

#ifdef HAVE_UNIX_H
# include <unix.h>
#endif

#include <stdarg.h>
#include <stddef.h>

#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#endif

#include <limits.h>

#if defined(CREX_WIN32) && !defined(__clang__)
#include <intrin.h>
#endif

#include "crex_range_check.h"

/* GCC x.y.z supplies __GNUC__ = x and __GNUC_MINOR__ = y */
#ifdef __GNUC__
# define CREX_GCC_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
#else
# define CREX_GCC_VERSION 0
#endif

/* Compatibility with non-clang compilers */
#ifndef __has_attribute
# define __has_attribute(x) 0
#endif
#ifndef __has_builtin
# define __has_builtin(x) 0
#endif
#ifndef __has_feature
# define __has_feature(x) 0
#endif

#if defined(CREX_WIN32) && !defined(__clang__)
# define CREX_ASSUME(c)	__assume(c)
#elif defined(__clang__) && __has_builtin(__builtin_assume)
# pragma clang diagnostic ignored "-Wassume"
# define CREX_ASSUME(c)	__builtin_assume(c)
#elif ((defined(__GNUC__) && CREX_GCC_VERSION >= 4005) || __has_builtin(__builtin_unreachable)) && CRX_HAVE_BUILTIN_EXPECT
# define CREX_ASSUME(c)	do { \
		if (__builtin_expect(!(c), 0)) __builtin_unreachable(); \
	} while (0)
#else
# define CREX_ASSUME(c)
#endif

#if CREX_DEBUG
# define CREX_ASSERT(c)	assert(c)
#else
# define CREX_ASSERT(c) CREX_ASSUME(c)
#endif

#if CREX_DEBUG
# define CREX_UNREACHABLE() do {CREX_ASSERT(0); CREX_ASSUME(0);} while (0)
#else
# define CREX_UNREACHABLE() CREX_ASSUME(0)
#endif

/* pseudo fallthrough keyword; */
#if defined(__GNUC__) && __GNUC__ >= 7
# define CREX_FALLTHROUGH __attribute__((__fallthrough__))
#else
# define CREX_FALLTHROUGH ((void)0)
#endif

/* Only use this macro if you know for sure that all of the switches values
   are covered by its case statements */
#define EMPTY_SWITCH_DEFAULT_CASE() default: CREX_UNREACHABLE(); break;

#if defined(__GNUC__) && __GNUC__ >= 4
# define CREX_IGNORE_VALUE(x) (({ __typeof__ (x) __x = (x); (void) __x; }))
#else
# define CREX_IGNORE_VALUE(x) ((void) (x))
#endif

#define crex_quiet_write(...) CREX_IGNORE_VALUE(write(__VA_ARGS__))

/* all HAVE_XXX test have to be after the include of crex_config above */

#if defined(HAVE_LIBDL) && !defined(CREX_WIN32)

# if __has_feature(address_sanitizer)
#  define __SANITIZE_ADDRESS__
# endif

# ifndef RTLD_LAZY
#  define RTLD_LAZY 1    /* Solaris 1, FreeBSD's (2.1.7.1 and older) */
# endif

# ifndef RTLD_GLOBAL
#  define RTLD_GLOBAL 0
# endif

# ifdef CRX_USE_RTLD_NOW
#  define CRX_RTLD_MODE  RTLD_NOW
# else
#  define CRX_RTLD_MODE  RTLD_LAZY
# endif

# if defined(RTLD_GROUP) && defined(RTLD_WORLD) && defined(RTLD_PARENT)
#  define DL_LOAD(libname)			dlopen(libname, CRX_RTLD_MODE | RTLD_GLOBAL | RTLD_GROUP | RTLD_WORLD | RTLD_PARENT)
# elif defined(RTLD_DEEPBIND) && !defined(__SANITIZE_ADDRESS__) && !__has_feature(memory_sanitizer)
#  define DL_LOAD(libname)			dlopen(libname, CRX_RTLD_MODE | RTLD_GLOBAL | RTLD_DEEPBIND)
# else
#  define DL_LOAD(libname)			dlopen(libname, CRX_RTLD_MODE | RTLD_GLOBAL)
# endif
# define DL_UNLOAD					dlclose
# if defined(DLSYM_NEEDS_UNDERSCORE)
#  define DL_FETCH_SYMBOL(h,s)		dlsym((h), "_" s)
# else
#  define DL_FETCH_SYMBOL			dlsym
# endif
# define DL_ERROR					dlerror
# define DL_HANDLE					void *
# define CREX_EXTENSIONS_SUPPORT	1
#elif defined(CREX_WIN32)
# define DL_LOAD(libname)			LoadLibrary(libname)
# define DL_FETCH_SYMBOL			GetProcAddress
# define DL_UNLOAD					FreeLibrary
# define DL_HANDLE					HMODULE
# define CREX_EXTENSIONS_SUPPORT	1
#else
# define DL_HANDLE					void *
# define CREX_EXTENSIONS_SUPPORT	0
#endif

#if defined(HAVE_ALLOCA_H) && !defined(_ALLOCA_H)
# include <alloca.h>
#endif
/* AIX requires this to be the first thing in the file.  */
#ifndef __GNUC__
# ifndef HAVE_ALLOCA_H
#  ifdef _AIX
#   pragma alloca
#  else
#   ifndef alloca /* predefined by HP cc +Olibcalls */
char *alloca();
#   endif
#  endif
# endif
#endif

#if !CREX_DEBUG && (defined(HAVE_ALLOCA) || (defined (__GNUC__) && __GNUC__ >= 2)) && !(defined(ZTS) && defined(HPUX)) && !defined(DARWIN)
# define CREX_ALLOCA_MAX_SIZE (32 * 1024)
# define ALLOCA_FLAG(name) \
	bool name;
# define SET_ALLOCA_FLAG(name) \
	name = true
# define do_alloca_ex(size, limit, use_heap) \
	((use_heap = (UNEXPECTED((size) > (limit)))) ? emalloc(size) : alloca(size))
# define do_alloca(size, use_heap) \
	do_alloca_ex(size, CREX_ALLOCA_MAX_SIZE, use_heap)
# define free_alloca(p, use_heap) \
	do { if (UNEXPECTED(use_heap)) efree(p); } while (0)
#else
# define ALLOCA_FLAG(name)
# define SET_ALLOCA_FLAG(name)
# define do_alloca(p, use_heap)		emalloc(p)
# define free_alloca(p, use_heap)	efree(p)
#endif

#if CREX_GCC_VERSION >= 2096 || __has_attribute(__malloc__)
# define CREX_ATTRIBUTE_MALLOC __attribute__ ((__malloc__))
#elif defined(CREX_WIN32)
# define CREX_ATTRIBUTE_MALLOC __declspec(allocator) __declspec(restrict)
#else
# define CREX_ATTRIBUTE_MALLOC
#endif

#if CREX_GCC_VERSION >= 4003 || __has_attribute(alloc_size)
# define CREX_ATTRIBUTE_ALLOC_SIZE(X) __attribute__ ((alloc_size(X)))
# define CREX_ATTRIBUTE_ALLOC_SIZE2(X,Y) __attribute__ ((alloc_size(X,Y)))
#else
# define CREX_ATTRIBUTE_ALLOC_SIZE(X)
# define CREX_ATTRIBUTE_ALLOC_SIZE2(X,Y)
#endif

#if CREX_GCC_VERSION >= 3000
# define CREX_ATTRIBUTE_CONST __attribute__((const))
#else
# define CREX_ATTRIBUTE_CONST
#endif

#if CREX_GCC_VERSION >= 2007 || __has_attribute(format)
# define CREX_ATTRIBUTE_FORMAT(type, idx, first) __attribute__ ((format(type, idx, first)))
#else
# define CREX_ATTRIBUTE_FORMAT(type, idx, first)
#endif

#if (CREX_GCC_VERSION >= 3001 && !defined(__INTEL_COMPILER)) || __has_attribute(format)
# define CREX_ATTRIBUTE_PTR_FORMAT(type, idx, first) __attribute__ ((format(type, idx, first)))
#else
# define CREX_ATTRIBUTE_PTR_FORMAT(type, idx, first)
#endif

#if CREX_GCC_VERSION >= 3001 || __has_attribute(deprecated)
# define CREX_ATTRIBUTE_DEPRECATED  __attribute__((deprecated))
#elif defined(CREX_WIN32)
# define CREX_ATTRIBUTE_DEPRECATED  __declspec(deprecated)
#else
# define CREX_ATTRIBUTE_DEPRECATED
#endif

#if CREX_GCC_VERSION >= 4003 || __has_attribute(unused)
# define CREX_ATTRIBUTE_UNUSED __attribute__((unused))
#else
# define CREX_ATTRIBUTE_UNUSED
#endif

#if defined(__GNUC__) && CREX_GCC_VERSION >= 4003
# define CREX_COLD __attribute__((cold))
# ifdef __OPTIMIZE__
#  define CREX_OPT_SIZE  __attribute__((optimize("Os")))
#  define CREX_OPT_SPEED __attribute__((optimize("Ofast")))
# else
#  define CREX_OPT_SIZE
#  define CREX_OPT_SPEED
# endif
#else
# define CREX_COLD
# define CREX_OPT_SIZE
# define CREX_OPT_SPEED
#endif

#if defined(__GNUC__) && CREX_GCC_VERSION >= 5000
# define CREX_ATTRIBUTE_UNUSED_LABEL __attribute__((unused));
# define CREX_ATTRIBUTE_COLD_LABEL __attribute__((cold));
#else
# define CREX_ATTRIBUTE_UNUSED_LABEL
# define CREX_ATTRIBUTE_COLD_LABEL
#endif

#if defined(__GNUC__) && CREX_GCC_VERSION >= 3004 && defined(__i386__)
# define CREX_FASTCALL __attribute__((fastcall))
#elif defined(_MSC_VER) && defined(_M_IX86) && _MSC_VER == 1700
# define CREX_FASTCALL __fastcall
#elif defined(_MSC_VER) && _MSC_VER >= 1800 && !defined(__clang__)
# define CREX_FASTCALL __vectorcall
#else
# define CREX_FASTCALL
#endif

#if (defined(__GNUC__) && __GNUC__ >= 3 && !defined(__INTEL_COMPILER) && !defined(DARWIN) && !defined(__hpux) && !defined(_AIX) && !defined(__osf__)) || __has_attribute(noreturn)
# define HAVE_NORETURN
# define CREX_NORETURN __attribute__((noreturn))
#elif defined(CREX_WIN32)
# define HAVE_NORETURN
# define CREX_NORETURN __declspec(noreturn)
#else
# define CREX_NORETURN
#endif

#if __has_attribute(force_align_arg_pointer)
# define CREX_STACK_ALIGNED __attribute__((force_align_arg_pointer))
#else
# define CREX_STACK_ALIGNED
#endif

#if (defined(__GNUC__) && __GNUC__ >= 3 && !defined(__INTEL_COMPILER) && !defined(DARWIN) && !defined(__hpux) && !defined(_AIX) && !defined(__osf__))
# define HAVE_NORETURN_ALIAS
# define HAVE_ATTRIBUTE_WEAK
#endif

#if CREX_GCC_VERSION >= 3001 || __has_builtin(__builtin_constant_p)
# define HAVE_BUILTIN_CONSTANT_P
#endif

#ifdef HAVE_BUILTIN_CONSTANT_P
# define CREX_CONST_COND(_condition, _default) \
	(__builtin_constant_p(_condition) ? (_condition) : (_default))
#else
# define CREX_CONST_COND(_condition, _default) \
	(_default)
#endif

#if CREX_DEBUG || defined(CREX_WIN32_NEVER_INLINE)
# define crex_always_inline inline
# define crex_never_inline
#else
# if defined(__GNUC__)
#  if __GNUC__ >= 3
#   define crex_always_inline inline __attribute__((always_inline))
#   define crex_never_inline __attribute__((noinline))
#  else
#   define crex_always_inline inline
#   define crex_never_inline
#  endif
# elif defined(_MSC_VER)
#  define crex_always_inline __forceinline
#  define crex_never_inline __declspec(noinline)
# else
#  if __has_attribute(always_inline)
#   define crex_always_inline inline __attribute__((always_inline))
#  else
#   define crex_always_inline inline
#  endif
#  if __has_attribute(noinline)
#   define crex_never_inline __attribute__((noinline))
#  else
#   define crex_never_inline
#  endif
# endif
#endif /* CREX_DEBUG */

#ifdef CRX_HAVE_BUILTIN_EXPECT
# define EXPECTED(condition)   __builtin_expect(!!(condition), 1)
# define UNEXPECTED(condition) __builtin_expect(!!(condition), 0)
#else
# define EXPECTED(condition)   (condition)
# define UNEXPECTED(condition) (condition)
#endif

#ifndef XtOffsetOf
# define XtOffsetOf(s_type, field) offsetof(s_type, field)
#endif

#ifdef HAVE_SIGSETJMP
# define SETJMP(a) sigsetjmp(a, 0)
# define LONGJMP(a,b) siglongjmp(a, b)
# define JMP_BUF sigjmp_buf
#else
# define SETJMP(a) setjmp(a)
# define LONGJMP(a,b) longjmp(a, b)
# define JMP_BUF jmp_buf
#endif

#if CREX_DEBUG
# define CREX_FILE_LINE_D				const char *__crex_filename, const uint32_t __crex_lineno
# define CREX_FILE_LINE_DC				, CREX_FILE_LINE_D
# define CREX_FILE_LINE_ORIG_D			const char *__crex_orig_filename, const uint32_t __crex_orig_lineno
# define CREX_FILE_LINE_ORIG_DC			, CREX_FILE_LINE_ORIG_D
# define CREX_FILE_LINE_RELAY_C			__crex_filename, __crex_lineno
# define CREX_FILE_LINE_RELAY_CC		, CREX_FILE_LINE_RELAY_C
# define CREX_FILE_LINE_C				__FILE__, __LINE__
# define CREX_FILE_LINE_CC				, CREX_FILE_LINE_C
# define CREX_FILE_LINE_EMPTY_C			NULL, 0
# define CREX_FILE_LINE_EMPTY_CC		, CREX_FILE_LINE_EMPTY_C
# define CREX_FILE_LINE_ORIG_RELAY_C	__crex_orig_filename, __crex_orig_lineno
# define CREX_FILE_LINE_ORIG_RELAY_CC	, CREX_FILE_LINE_ORIG_RELAY_C
#else
# define CREX_FILE_LINE_D				void
# define CREX_FILE_LINE_DC
# define CREX_FILE_LINE_ORIG_D			void
# define CREX_FILE_LINE_ORIG_DC
# define CREX_FILE_LINE_RELAY_C
# define CREX_FILE_LINE_RELAY_CC
# define CREX_FILE_LINE_C
# define CREX_FILE_LINE_CC
# define CREX_FILE_LINE_EMPTY_C
# define CREX_FILE_LINE_EMPTY_CC
# define CREX_FILE_LINE_ORIG_RELAY_C
# define CREX_FILE_LINE_ORIG_RELAY_CC
#endif	/* CREX_DEBUG */

#if CREX_DEBUG
# define C_DBG(expr)		(expr)
#else
# define C_DBG(expr)
#endif

#ifdef ZTS
# define ZTS_V 1
#else
# define ZTS_V 0
#endif

#ifndef LONG_MAX
# define LONG_MAX 2147483647L
#endif

#ifndef LONG_MIN
# define LONG_MIN (- LONG_MAX - 1)
#endif

#define MAX_LENGTH_OF_DOUBLE 32

#undef MIN
#undef MAX
#define MAX(a, b)  (((a)>(b))?(a):(b))
#define MIN(a, b)  (((a)<(b))?(a):(b))

#define CREX_BIT_TEST(bits, bit) \
	(((bits)[(bit) / (sizeof((bits)[0])*8)] >> ((bit) & (sizeof((bits)[0])*8-1))) & 1)

#define CREX_INFINITY INFINITY

#define CREX_NAN NAN

#if defined(__cplusplus) && __cplusplus >= 201103L
extern "C++" {
# include <cmath>
}
# define crex_isnan std::isnan
# define crex_isinf std::isinf
# define crex_finite std::isfinite
#else
# include <math.h>
# define crex_isnan(a) isnan(a)
# define crex_isinf(a) isinf(a)
# define crex_finite(a) isfinite(a)
#endif

#define CREX_STRL(str)		(str), (sizeof(str)-1)
#define CREX_STRS(str)		(str), (sizeof(str))
#define CREX_NORMALIZE_BOOL(n)			\
	((n) ? (((n)<0) ? -1 : 1) : 0)
#define CREX_TRUTH(x)		((x) ? 1 : 0)
#define CREX_LOG_XOR(a, b)		(CREX_TRUTH(a) ^ CREX_TRUTH(b))

/**
 * Do a three-way comparison of two integers and returns -1, 0 or 1
 * depending on whether #a is smaller, equal or larger than #b.
 */
#define CREX_THREEWAY_COMPARE(a, b) ((a) == (b) ? 0 : ((a) < (b) ? -1 : 1))

#define CREX_MAX_RESERVED_RESOURCES	6

/* excpt.h on Digital Unix 4.0 defines function_table */
#undef function_table

#ifdef CREX_WIN32
#define CREX_SECURE_ZERO(var, size) RtlSecureZeroMemory((var), (size))
#else
#define CREX_SECURE_ZERO(var, size) explicit_bzero((var), (size))
#endif

/* This check should only be used on network socket, not file descriptors */
#ifdef CREX_WIN32
#define CREX_VALID_SOCKET(sock) (INVALID_SOCKET != (sock))
#else
#define CREX_VALID_SOCKET(sock) ((sock) >= 0)
#endif

/* Intrinsics macros start. */

/* Memory sanitizer is incompatible with ifunc resolvers. Even if the resolver
 * is marked as no_sanitize("memory") it will still be instrumented and crash. */
#if __has_feature(memory_sanitizer) || __has_feature(thread_sanitizer) || \
	__has_feature(dataflow_sanitizer)
# undef HAVE_FUNC_ATTRIBUTE_IFUNC
#endif

/* Only use ifunc resolvers if we have __builtin_cpu_supports() and __builtin_cpu_init(),
 * otherwise the use of crex_cpu_supports() may not be safe inside ifunc resolvers. */
#if defined(HAVE_FUNC_ATTRIBUTE_IFUNC) && defined(HAVE_FUNC_ATTRIBUTE_TARGET) && \
	defined(CRX_HAVE_BUILTIN_CPU_SUPPORTS) && defined(CRX_HAVE_BUILTIN_CPU_INIT)
# define CREX_INTRIN_HAVE_IFUNC_TARGET 1
#endif

#if (defined(__i386__) || defined(__x86_64__))
# if defined(HAVE_TMMINTRIN_H)
#  define CRX_HAVE_SSSE3
# endif

# if defined(HAVE_NMMINTRIN_H)
#  define CRX_HAVE_SSE4_2
# endif

# if defined(HAVE_WMMINTRIN_H)
#  define CRX_HAVE_PCLMUL
# endif

/*
 * AVX2 support was added in gcc 4.7, but AVX2 intrinsics don't work in
 * __attribute__((target("avx2"))) functions until gcc 4.9.
 */
# if defined(HAVE_IMMINTRIN_H) && \
  (defined(__llvm__) || defined(__clang__) || (defined(__GNUC__) && CREX_GCC_VERSION >= 4009))
#  define CRX_HAVE_AVX2
# endif
#endif

#ifdef __SSSE3__
/* Instructions compiled directly. */
# define CREX_INTRIN_SSSE3_NATIVE 1
#elif (defined(HAVE_FUNC_ATTRIBUTE_TARGET) && defined(CRX_HAVE_SSSE3)) || (defined(CREX_WIN32) && (!defined(_M_ARM64)))
/* Function resolved by ifunc or MINIT. */
# define CREX_INTRIN_SSSE3_RESOLVER 1
#endif

/* Do not use for conditional declaration of API functions! */
#if defined(CREX_INTRIN_SSSE3_RESOLVER) && defined(CREX_INTRIN_HAVE_IFUNC_TARGET)
# define CREX_INTRIN_SSSE3_FUNC_PROTO 1
#elif defined(CREX_INTRIN_SSSE3_RESOLVER)
# define CREX_INTRIN_SSSE3_FUNC_PTR 1
#endif

#ifdef CREX_INTRIN_SSSE3_RESOLVER
# ifdef HAVE_FUNC_ATTRIBUTE_TARGET
#  define CREX_INTRIN_SSSE3_FUNC_DECL(func) CREX_API func __attribute__((target("ssse3")))
# else
#  define CREX_INTRIN_SSSE3_FUNC_DECL(func) func
# endif
#else
# define CREX_INTRIN_SSSE3_FUNC_DECL(func)
#endif

#ifdef __SSE4_2__
/* Instructions compiled directly. */
# define CREX_INTRIN_SSE4_2_NATIVE 1
#elif (defined(HAVE_FUNC_ATTRIBUTE_TARGET) && defined(CRX_HAVE_SSE4_2)) || (defined(CREX_WIN32) && (!defined(_M_ARM64)))
/* Function resolved by ifunc or MINIT. */
# define CREX_INTRIN_SSE4_2_RESOLVER 1
#endif

/* Do not use for conditional declaration of API functions! */
#if defined(CREX_INTRIN_SSE4_2_RESOLVER) && defined(CREX_INTRIN_HAVE_IFUNC_TARGET)
# define CREX_INTRIN_SSE4_2_FUNC_PROTO 1
#elif defined(CREX_INTRIN_SSE4_2_RESOLVER)
# define CREX_INTRIN_SSE4_2_FUNC_PTR 1
#endif

#ifdef CREX_INTRIN_SSE4_2_RESOLVER
# ifdef HAVE_FUNC_ATTRIBUTE_TARGET
#  define CREX_INTRIN_SSE4_2_FUNC_DECL(func) CREX_API func __attribute__((target("sse4.2")))
# else
#  define CREX_INTRIN_SSE4_2_FUNC_DECL(func) func
# endif
#else
# define CREX_INTRIN_SSE4_2_FUNC_DECL(func)
#endif

#ifdef __PCLMUL__
/* Instructions compiled directly. */
# define CREX_INTRIN_PCLMUL_NATIVE 1
#elif (defined(HAVE_FUNC_ATTRIBUTE_TARGET) && defined(CRX_HAVE_PCLMUL)) || (defined(CREX_WIN32) && (!defined(_M_ARM64)))
/* Function resolved by ifunc or MINIT. */
# define CREX_INTRIN_PCLMUL_RESOLVER 1
#endif

/* Do not use for conditional declaration of API functions! */
#if defined(CREX_INTRIN_PCLMUL_RESOLVER) && defined(CREX_INTRIN_HAVE_IFUNC_TARGET) && (!defined(__GNUC__) || (CREX_GCC_VERSION >= 9000))
/* __builtin_cpu_supports has pclmul from gcc9 */
# define CREX_INTRIN_PCLMUL_FUNC_PROTO 1
#elif defined(CREX_INTRIN_PCLMUL_RESOLVER)
# define CREX_INTRIN_PCLMUL_FUNC_PTR 1
#endif

#ifdef CREX_INTRIN_PCLMUL_RESOLVER
# ifdef HAVE_FUNC_ATTRIBUTE_TARGET
#  define CREX_INTRIN_PCLMUL_FUNC_DECL(func) CREX_API func __attribute__((target("pclmul")))
# else
#  define CREX_INTRIN_PCLMUL_FUNC_DECL(func) func
# endif
#else
# define CREX_INTRIN_PCLMUL_FUNC_DECL(func)
#endif

#if defined(CREX_INTRIN_SSE4_2_NATIVE) && defined(CREX_INTRIN_PCLMUL_NATIVE)
/* Instructions compiled directly. */
# define CREX_INTRIN_SSE4_2_PCLMUL_NATIVE 1
#elif (defined(HAVE_FUNC_ATTRIBUTE_TARGET) && defined(CRX_HAVE_SSE4_2) && defined(CRX_HAVE_PCLMUL)) || (defined(CREX_WIN32) && (!defined(_M_ARM64)))
/* Function resolved by ifunc or MINIT. */
# define CREX_INTRIN_SSE4_2_PCLMUL_RESOLVER 1
#endif

/* Do not use for conditional declaration of API functions! */
#if defined(CREX_INTRIN_SSE4_2_PCLMUL_RESOLVER) && defined(CREX_INTRIN_HAVE_IFUNC_TARGET) && (!defined(__GNUC__) || (CREX_GCC_VERSION >= 9000))
/* __builtin_cpu_supports has pclmul from gcc9 */
# define CREX_INTRIN_SSE4_2_PCLMUL_FUNC_PROTO 1
#elif defined(CREX_INTRIN_SSE4_2_PCLMUL_RESOLVER)
# define CREX_INTRIN_SSE4_2_PCLMUL_FUNC_PTR 1
#endif

#ifdef CREX_INTRIN_SSE4_2_PCLMUL_RESOLVER
# ifdef HAVE_FUNC_ATTRIBUTE_TARGET
#  define CREX_INTRIN_SSE4_2_PCLMUL_FUNC_DECL(func) CREX_API func __attribute__((target("sse4.2,pclmul")))
# else
#  define CREX_INTRIN_SSE4_2_PCLMUL_FUNC_DECL(func) func
# endif
#else
# define CREX_INTRIN_SSE4_2_PCLMUL_FUNC_DECL(func)
#endif

#ifdef __AVX2__
# define CREX_INTRIN_AVX2_NATIVE 1
#elif (defined(HAVE_FUNC_ATTRIBUTE_TARGET) && defined(CRX_HAVE_AVX2)) || (defined(CREX_WIN32) && (!defined(_M_ARM64)))
# define CREX_INTRIN_AVX2_RESOLVER 1
#endif

/* Do not use for conditional declaration of API functions! */
#if defined(CREX_INTRIN_AVX2_RESOLVER) && defined(CREX_INTRIN_HAVE_IFUNC_TARGET)
# define CREX_INTRIN_AVX2_FUNC_PROTO 1
#elif defined(CREX_INTRIN_AVX2_RESOLVER)
# define CREX_INTRIN_AVX2_FUNC_PTR 1
#endif

#ifdef CREX_INTRIN_AVX2_RESOLVER
# ifdef HAVE_FUNC_ATTRIBUTE_TARGET
#  define CREX_INTRIN_AVX2_FUNC_DECL(func) CREX_API func __attribute__((target("avx2")))
# else
#  define CREX_INTRIN_AVX2_FUNC_DECL(func) func
# endif
#else
# define CREX_INTRIN_AVX2_FUNC_DECL(func)
#endif

#if CRX_HAVE_AVX512_SUPPORTS && defined(HAVE_FUNC_ATTRIBUTE_TARGET) || defined(CREX_WIN32)
#define CREX_INTRIN_AVX512_RESOLVER 1
#endif

#if defined(CREX_INTRIN_AVX512_RESOLVER) && defined(CREX_INTRIN_HAVE_IFUNC_TARGET)
# define CREX_INTRIN_AVX512_FUNC_PROTO 1
#elif defined(CREX_INTRIN_AVX512_RESOLVER)
# define CREX_INTRIN_AVX512_FUNC_PTR 1
#endif

#ifdef CREX_INTRIN_AVX512_RESOLVER
# ifdef HAVE_FUNC_ATTRIBUTE_TARGET
#  define CREX_INTRIN_AVX512_FUNC_DECL(func) CREX_API func __attribute__((target("avx512f,avx512cd,avx512vl,avx512dq,avx512bw")))
# else
#  define CREX_INTRIN_AVX512_FUNC_DECL(func) func
# endif
#else
# define CREX_INTRIN_AVX512_FUNC_DECL(func)
#endif

#if CRX_HAVE_AVX512_VBMI_SUPPORTS && defined(HAVE_FUNC_ATTRIBUTE_TARGET)
#define CREX_INTRIN_AVX512_VBMI_RESOLVER 1
#endif

#if defined(CREX_INTRIN_AVX512_VBMI_RESOLVER) && defined(CREX_INTRIN_HAVE_IFUNC_TARGET)
# define CREX_INTRIN_AVX512_VBMI_FUNC_PROTO 1
#elif defined(CREX_INTRIN_AVX512_VBMI_RESOLVER)
# define CREX_INTRIN_AVX512_VBMI_FUNC_PTR 1
#endif

#ifdef CREX_INTRIN_AVX512_VBMI_RESOLVER
# ifdef HAVE_FUNC_ATTRIBUTE_TARGET
#  define CREX_INTRIN_AVX512_VBMI_FUNC_DECL(func) CREX_API func __attribute__((target("avx512f,avx512cd,avx512vl,avx512dq,avx512bw,avx512vbmi")))
# else
#  define CREX_INTRIN_AVX512_VBMI_FUNC_DECL(func) func
# endif
#else
# define CREX_INTRIN_AVX512_VBMI_FUNC_DECL(func)
#endif

/* Intrinsics macros end. */

#ifdef CREX_WIN32
# define CREX_SET_ALIGNED(alignment, decl) __declspec(align(alignment)) decl
#elif defined(HAVE_ATTRIBUTE_ALIGNED)
# define CREX_SET_ALIGNED(alignment, decl) decl __attribute__ ((__aligned__ (alignment)))
#else
# define CREX_SET_ALIGNED(alignment, decl) decl
#endif

#define CREX_SLIDE_TO_ALIGNED(alignment, ptr) (((uintptr_t)(ptr) + ((alignment)-1)) & ~((alignment)-1))
#define CREX_SLIDE_TO_ALIGNED16(ptr) CREX_SLIDE_TO_ALIGNED(C_UL(16), ptr)

#ifdef CREX_WIN32
# define _CREX_EXPAND_VA(a) a
# define CREX_EXPAND_VA(code) _CREX_EXPAND_VA(code)
#else
# define CREX_EXPAND_VA(code) code
#endif

/* On CPU with few registers, it's cheaper to reload value then use spill slot */
#if defined(__i386__) || (defined(_WIN32) && !defined(_WIN64))
# define CREX_PREFER_RELOAD
#endif

#if defined(CREX_WIN32) && defined(_DEBUG)
# define CREX_IGNORE_LEAKS_BEGIN() _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) & ~_CRTDBG_ALLOC_MEM_DF)
# define CREX_IGNORE_LEAKS_END() _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_ALLOC_MEM_DF)
#else
# define CREX_IGNORE_LEAKS_BEGIN()
# define CREX_IGNORE_LEAKS_END()
#endif

/* MSVC yields C4090 when a (const T **) is passed to a (void *); CREX_VOIDP works around that */
#ifdef _MSC_VER
# define CREX_VOIDP(ptr) ((void *) ptr)
#else
# define CREX_VOIDP(ptr) (ptr)
#endif

#if __has_attribute(__indirect_return__)
# define CREX_INDIRECT_RETURN __attribute__((__indirect_return__))
#else
# define CREX_INDIRECT_RETURN
#endif

#define __CREX_DO_PRAGMA(x) _Pragma(#x)
#define _CREX_DO_PRAGMA(x) __CREX_DO_PRAGMA(x)
#if defined(__clang__)
# define CREX_DIAGNOSTIC_IGNORED_START(warning) \
	_Pragma("clang diagnostic push") \
	_CREX_DO_PRAGMA(clang diagnostic ignored warning)
# define CREX_DIAGNOSTIC_IGNORED_END \
	_Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
# define CREX_DIAGNOSTIC_IGNORED_START(warning) \
	_Pragma("GCC diagnostic push") \
	_CREX_DO_PRAGMA(GCC diagnostic ignored warning)
# define CREX_DIAGNOSTIC_IGNORED_END \
	_Pragma("GCC diagnostic pop")
#else
# define CREX_DIAGNOSTIC_IGNORED_START(warning)
# define CREX_DIAGNOSTIC_IGNORED_END
#endif

/** @deprecated */
#define CREX_CGG_DIAGNOSTIC_IGNORED_START CREX_DIAGNOSTIC_IGNORED_START
/** @deprecated */
#define CREX_CGG_DIAGNOSTIC_IGNORED_END CREX_DIAGNOSTIC_IGNORED_END

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) /* C11 */
# define CREX_STATIC_ASSERT(c, m) _Static_assert((c), m)
#else
# define CREX_STATIC_ASSERT(c, m)
#endif

#endif /* CREX_PORTABILITY_H */
