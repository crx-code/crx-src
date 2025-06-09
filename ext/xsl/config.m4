CRX_ARG_WITH([xsl],
  [whether to build with XSL support],
  [AS_HELP_STRING([--with-xsl],
    [Build with XSL support])])

if test "$CRX_XSL" != "no"; then

  if test "$CRX_LIBXML" = "no"; then
    AC_MSG_ERROR([XSL extension requires LIBXML extension, add --with-libxml])
  fi

  if test "$CRX_DOM" = "no"; then
    AC_MSG_ERROR([XSL extension requires DOM extension, add --enable-dom])
  fi

  PKG_CHECK_MODULES([XSL], [libxslt >= 1.1.0])

  CRX_EVAL_INCLINE($XSL_CFLAGS)
  CRX_EVAL_LIBLINE($XSL_LIBS, XSL_SHARED_LIBADD)

  PKG_CHECK_MODULES([EXSLT], [libexslt],
  [
    CRX_EVAL_INCLINE($EXSLT_CFLAGS)
    CRX_EVAL_LIBLINE($EXSLT_LIBS, XSL_SHARED_LIBADD)
    AC_DEFINE(HAVE_XSL_EXSLT, 1, [ ])
  ], [ ])

  AC_DEFINE(HAVE_XSL,1,[ ])
  CRX_NEW_EXTENSION(xsl, crx_xsl.c xsltprocessor.c, $ext_shared)
  CRX_SUBST(XSL_SHARED_LIBADD)
  CRX_ADD_EXTENSION_DEP(xsl, libxml)
fi
