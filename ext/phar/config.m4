CRX_ARG_ENABLE([crxa],
  [for crxa archive support],
  [AS_HELP_STRING([--disable-crxa],
    [Disable crxa support])],
  [yes])

if test "$CRX_CRXA" != "no"; then
  CRX_NEW_EXTENSION(crxa, util.c tar.c zip.c stream.c func_interceptors.c dirstream.c crxa.c crxa_object.c crxa_path_check.c, $ext_shared,, -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1)
  AC_MSG_CHECKING([for crxa openssl support])
  if test "$CRX_OPENSSL_SHARED" = "yes"; then
    AC_MSG_RESULT([no (shared openssl)])
  else
    if test "$CRX_OPENSSL" = "yes"; then
      AC_MSG_RESULT([yes])
      AC_DEFINE(CRXA_HAVE_OPENSSL,1,[ ])
    else
      AC_MSG_RESULT([no])
    fi
  fi
  CRX_ADD_EXTENSION_DEP(crxa, hash, true)
  CRX_ADD_EXTENSION_DEP(crxa, spl, true)
  CRX_ADD_MAKEFILE_FRAGMENT

  CRX_INSTALL_HEADERS([ext/crxa], [crx_crxa.h])

  CRX_OUTPUT(ext/crxa/crxa.1 ext/crxa/crxa.crxa.1)
fi
