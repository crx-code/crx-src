CRX_ARG_WITH([gmp],
  [for GNU MP support],
  [AS_HELP_STRING([[--with-gmp[=DIR]]],
    [Include GNU MP support])])

if test "$CRX_GMP" != "no"; then
  if test "$CRX_GMP" = "yes"; then
    CRX_CHECK_LIBRARY(gmp, __gmpz_rootrem,
    [],[
      AC_MSG_ERROR([GNU MP Library version 4.2 or greater required.])
    ])

    CRX_ADD_LIBRARY(gmp,,GMP_SHARED_LIBADD)
  else
    if test ! -f $CRX_GMP/include/gmp.h; then
      AC_MSG_ERROR(Unable to locate gmp.h)
    fi

    CRX_CHECK_LIBRARY(gmp, __gmpz_rootrem,
    [],[
      AC_MSG_ERROR([GNU MP Library version 4.2 or greater required.])
    ],[
      -L$CRX_GMP/$CRX_LIBDIR
    ])

    CRX_ADD_LIBRARY_WITH_PATH(gmp, $CRX_GMP/$CRX_LIBDIR, GMP_SHARED_LIBADD)
    CRX_ADD_INCLUDE($CRX_GMP/include)
  fi

  CRX_INSTALL_HEADERS([ext/gmp/crx_gmp_int.h])

  CRX_NEW_EXTENSION(gmp, gmp.c, $ext_shared,, -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1)
  CRX_SUBST(GMP_SHARED_LIBADD)
  AC_DEFINE(HAVE_GMP, 1, [ ])
fi
