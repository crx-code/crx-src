CRX_ARG_ENABLE([soap],
  [whether to enable SOAP support],
  [AS_HELP_STRING([--enable-soap],
    [Enable SOAP support])])

if test "$CRX_SOAP" != "no"; then

  if test "$CRX_LIBXML" = "no"; then
    AC_MSG_ERROR([SOAP extension requires LIBXML extension, add --with-libxml])
  fi

  CRX_SETUP_LIBXML(SOAP_SHARED_LIBADD, [
    AC_DEFINE(HAVE_SOAP,1,[ ])
    CRX_NEW_EXTENSION(soap, soap.c crx_encoding.c crx_http.c crx_packet_soap.c crx_schema.c crx_sdl.c crx_xml.c, $ext_shared,, -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1)
    CRX_SUBST(SOAP_SHARED_LIBADD)
  ])
fi
