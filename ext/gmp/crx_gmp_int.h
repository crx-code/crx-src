#ifndef incl_CRX_GMP_INT_H
#define incl_CRX_GMP_INT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include <gmp.h>

#ifdef CRX_WIN32
# define CRX_GMP_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
# define CRX_GMP_API __attribute__ ((visibility("default")))
#else
# define CRX_GMP_API
#endif

typedef struct _gmp_object {
	mpz_t num;
	crex_object std;
} gmp_object;

static inline gmp_object *crx_gmp_object_from_crex_object(crex_object *zobj) {
	return (gmp_object *)( ((char *)zobj) - XtOffsetOf(gmp_object, std) );
}

CRX_GMP_API crex_class_entry *crx_gmp_class_entry(void);

/* GMP and MPIR use different datatypes on different platforms */
#ifdef _WIN64
typedef crex_long gmp_long;
typedef crex_ulong gmp_ulong;
#else
typedef long gmp_long;
typedef unsigned long gmp_ulong;
#endif

#endif
