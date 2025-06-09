CRX_ARG_ENABLE([exif],
  [whether to enable EXIF (metadata from images) support],
  [AS_HELP_STRING([--enable-exif],
    [Enable EXIF (metadata from images) support])])

if test "$CRX_EXIF" != "no"; then
  AC_DEFINE(HAVE_EXIF, 1, [Whether you want EXIF (metadata from images) support])
  CRX_NEW_EXTENSION(exif, exif.c, $ext_shared,, -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
