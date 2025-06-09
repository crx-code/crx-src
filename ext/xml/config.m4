CRX_ARG_ENABLE([xml],
  [whether to enable XML support],
  [AS_HELP_STRING([--disable-xml],
    [Disable XML support])],
  [yes])

CRX_ARG_WITH([expat],
  [whether to build with expat support],
  [AS_HELP_STRING([--with-expat],
    [XML: use expat instead of libxml2])],
  [no],
  [no])

if test "$CRX_XML" != "no"; then

  dnl
  dnl Default to libxml2 if --with-expat is not specified.
  dnl
  if test "$CRX_EXPAT" = "no"; then

    if test "$CRX_LIBXML" = "no"; then
      AC_MSG_ERROR([XML extension requires LIBXML extension, add --with-libxml])
    fi

    CRX_SETUP_LIBXML(XML_SHARED_LIBADD, [
      xml_extra_sources="compat.c"
      CRX_ADD_EXTENSION_DEP(xml, libxml)
    ])
  else
    CRX_SETUP_EXPAT([XML_SHARED_LIBADD])
  fi

  CRX_NEW_EXTENSION(xml, xml.c $xml_extra_sources, $ext_shared,, -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1)
  CRX_SUBST(XML_SHARED_LIBADD)
  CRX_INSTALL_HEADERS([ext/xml/])
  AC_DEFINE(HAVE_XML, 1, [ ])
fi
