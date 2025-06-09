CRX_ARG_WITH([libxml],
  [whether to build with LIBXML support],
  [AS_HELP_STRING([--without-libxml],
    [Build without LIBXML support])],
  [yes])

if test "$CRX_LIBXML" != "no"; then

  dnl This extension can not be build as shared
  ext_shared=no

  CRX_SETUP_LIBXML(LIBXML_SHARED_LIBADD, [
    AC_DEFINE(HAVE_LIBXML,1,[ ])
    CRX_NEW_EXTENSION(libxml, [libxml.c], $ext_shared,, -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1)
    CRX_INSTALL_HEADERS([ext/libxml/crx_libxml.h])
  ])
fi
