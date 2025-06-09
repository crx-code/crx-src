CRX_ARG_ENABLE([session],
  [whether to enable CRX sessions],
  [AS_HELP_STRING([--disable-session],
    [Disable session support])],
  [yes])

CRX_ARG_WITH([mm],
  [for mm support],
  [AS_HELP_STRING([[--with-mm[=DIR]]],
    [SESSION: Include mm support for session storage])],
  [no],
  [no])

if test "$CRX_SESSION" != "no"; then
  CRX_PWRITE_TEST
  CRX_PREAD_TEST
  CRX_NEW_EXTENSION(session, mod_user_class.c session.c mod_files.c mod_mm.c mod_user.c, $ext_shared,, -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1)
  CRX_ADD_EXTENSION_DEP(session, hash, true)
  CRX_ADD_EXTENSION_DEP(session, spl)
  CRX_SUBST(SESSION_SHARED_LIBADD)
  CRX_INSTALL_HEADERS(ext/session, [crx_session.h mod_files.h mod_user.h])
  AC_DEFINE(HAVE_CRX_SESSION,1,[ ])
fi

if test "$CRX_MM" != "no"; then
  for i in $CRX_MM /usr/local /usr; do
    test -f "$i/include/mm.h" && MM_DIR=$i && break
  done

  if test -z "$MM_DIR" ; then
    AC_MSG_ERROR(cannot find mm library)
  fi

  if test "$CRX_THREAD_SAFETY" = "yes"; then
    dnl The mm library is not thread-safe, and mod_mm.c refuses to compile.
    AC_MSG_ERROR(--with-mm cannot be combined with --enable-zts)
  fi

  CRX_ADD_LIBRARY_WITH_PATH(mm, $MM_DIR/$CRX_LIBDIR, SESSION_SHARED_LIBADD)
  CRX_ADD_INCLUDE($MM_DIR/include)
  CRX_INSTALL_HEADERS([ext/session/mod_mm.h])
  AC_DEFINE(HAVE_LIBMM, 1, [Whether you have libmm])
fi
