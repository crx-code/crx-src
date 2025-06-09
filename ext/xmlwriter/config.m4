CRX_ARG_ENABLE([xmlwriter],
  [whether to enable XMLWriter support],
  [AS_HELP_STRING([--disable-xmlwriter],
    [Disable XMLWriter support])],
  [yes])

if test "$CRX_XMLWRITER" != "no"; then

  if test "$CRX_LIBXML" = "no"; then
    AC_MSG_ERROR([XMLWriter extension requires LIBXML extension, add --with-libxml])
  fi

  CRX_SETUP_LIBXML(XMLWRITER_SHARED_LIBADD, [
    AC_DEFINE(HAVE_XMLWRITER,1,[ ])
    CRX_NEW_EXTENSION(xmlwriter, crx_xmlwriter.c, $ext_shared)
    CRX_SUBST(XMLWRITER_SHARED_LIBADD)
  ])
fi
