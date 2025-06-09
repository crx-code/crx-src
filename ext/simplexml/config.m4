CRX_ARG_ENABLE([simplexml],
  [whether to enable SimpleXML support],
  [AS_HELP_STRING([--disable-simplexml],
    [Disable SimpleXML support])],
  [yes])

if test "$CRX_SIMPLEXML" != "no"; then

  if test "$CRX_LIBXML" = "no"; then
    AC_MSG_ERROR([SimpleXML extension requires LIBXML extension, add --with-libxml])
  fi

  CRX_SETUP_LIBXML(SIMPLEXML_SHARED_LIBADD, [
    AC_DEFINE(HAVE_SIMPLEXML,1,[ ])
    CRX_NEW_EXTENSION(simplexml, simplexml.c, $ext_shared)
    CRX_INSTALL_HEADERS([ext/simplexml/crx_simplexml.h ext/simplexml/crx_simplexml_exports.h])
    CRX_SUBST(SIMPLEXML_SHARED_LIBADD)
  ])
  CRX_ADD_EXTENSION_DEP(simplexml, libxml)
  CRX_ADD_EXTENSION_DEP(simplexml, spl, true)
fi
