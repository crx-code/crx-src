CRX_ARG_ENABLE([xmlreader],
  [whether to enable XMLReader support],
  [AS_HELP_STRING([--disable-xmlreader],
    [Disable XMLReader support])],
  [yes])

if test "$CRX_XMLREADER" != "no"; then

  if test "$CRX_LIBXML" = "no"; then
    AC_MSG_ERROR([XMLReader extension requires LIBXML extension, add --with-libxml])
  fi

  CRX_SETUP_LIBXML(XMLREADER_SHARED_LIBADD, [
    AC_DEFINE(HAVE_XMLREADER,1,[ ])
    CRX_NEW_EXTENSION(xmlreader, crx_xmlreader.c, $ext_shared)
    CRX_ADD_EXTENSION_DEP(xmlreader, dom, true)
    CRX_SUBST(XMLREADER_SHARED_LIBADD)
  ])
fi
