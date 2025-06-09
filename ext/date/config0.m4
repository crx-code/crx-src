dnl Check for headers needed by timelib
AC_CHECK_HEADERS([io.h])

dnl Check for strtoll, atoll
AC_CHECK_FUNCS(strtoll atoll)

CRX_DATE_CFLAGS="-Wno-implicit-fallthrough -I@ext_builddir@/lib -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1 -DHAVE_TIMELIB_CONFIG_H=1"
timelib_sources="lib/astro.c lib/dow.c lib/parse_date.c lib/parse_tz.c lib/parse_posix.c
                 lib/timelib.c lib/tm2unixtime.c lib/unixtime2tm.c lib/parse_iso_intervals.c lib/interval.c"

CRX_NEW_EXTENSION(date, crx_date.c $timelib_sources, no,, $CRX_DATE_CFLAGS)

CRX_ADD_BUILD_DIR([$ext_builddir/lib], 1)
CRX_ADD_INCLUDE([$ext_builddir/lib])
CRX_ADD_INCLUDE([$ext_srcdir/lib])

CRX_INSTALL_HEADERS([ext/date], [crx_date.h lib/timelib.h lib/timelib_config.h])
AC_DEFINE([HAVE_TIMELIB_CONFIG_H], [1], [Have timelib_config.h])

cat > $ext_builddir/lib/timelib_config.h <<EOF
#ifdef CRX_WIN32
# include "config.w32.h"
#else
# include <crx_config.h>
#endif
#include <inttypes.h>
#include <stdint.h>

#include "crex.h"

#define timelib_malloc  emalloc
#define timelib_realloc erealloc
#define timelib_calloc  ecalloc
#define timelib_strdup  estrdup
#define timelib_strndup estrndup
#define timelib_free    efree
EOF
