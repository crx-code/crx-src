CRX_ARG_WITH([zlib],
  [for ZLIB support],
  [AS_HELP_STRING([--with-zlib],
    [Include ZLIB support (requires zlib >= 1.2.0.4)])])

if test "$CRX_ZLIB" != "no"; then
  PKG_CHECK_MODULES([ZLIB], [zlib >= 1.2.0.4])

  CRX_EVAL_LIBLINE($ZLIB_LIBS, ZLIB_SHARED_LIBADD)
  CRX_EVAL_INCLINE($ZLIB_CFLAGS)

  AC_DEFINE(HAVE_ZLIB,1,[ ])

  CRX_NEW_EXTENSION(zlib, zlib.c zlib_fopen_wrapper.c zlib_filter.c, $ext_shared,, -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1)
  CRX_SUBST(ZLIB_SHARED_LIBADD)
fi
