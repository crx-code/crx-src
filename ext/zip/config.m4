CRX_ARG_WITH([zip],
  [for zip archive read/write support],
  [AS_HELP_STRING([--with-zip],
    [Include Zip read/write support])])

if test "$CRX_ZIP" != "no"; then
  PKG_CHECK_MODULES([LIBZIP], [libzip >= 0.11 libzip != 1.3.1 libzip != 1.7.0])

  CRX_EVAL_INCLINE($LIBZIP_CFLAGS)
  CRX_EVAL_LIBLINE($LIBZIP_LIBS, ZIP_SHARED_LIBADD)

  CRX_CHECK_LIBRARY(zip, zip_file_set_mtime,
  [
    AC_DEFINE(HAVE_SET_MTIME, 1, [Libzip >= 1.0.0 with zip_file_set_mtime])
  ], [
    AC_MSG_WARN(Libzip >= 1.0.0 needed for setting mtime)
  ], [
    $LIBZIP_LIBS
  ])

  CRX_CHECK_LIBRARY(zip, zip_file_set_encryption,
  [
    AC_DEFINE(HAVE_ENCRYPTION, 1, [Libzip >= 1.2.0 with encryption support])
  ], [
    AC_MSG_WARN(Libzip >= 1.2.0 needed for encryption support)
  ], [
    $LIBZIP_LIBS
  ])

  CRX_CHECK_LIBRARY(zip, zip_libzip_version,
  [
    AC_DEFINE(HAVE_LIBZIP_VERSION, 1, [Libzip >= 1.3.1 with zip_libzip_version function])
  ], [
  ], [
    $LIBZIP_LIBS
  ])

  CRX_CHECK_LIBRARY(zip, zip_register_progress_callback_with_state,
  [
    AC_DEFINE(HAVE_PROGRESS_CALLBACK, 1, [Libzip >= 1.3.0 with zip_register_progress_callback_with_state function])
  ], [
  ], [
    $LIBZIP_LIBS
  ])

  CRX_CHECK_LIBRARY(zip, zip_register_cancel_callback_with_state,
  [
    AC_DEFINE(HAVE_CANCEL_CALLBACK, 1, [Libzip >= 1.6.0 with zip_register_cancel_callback_with_state function])
  ], [
  ], [
    $LIBZIP_LIBS
  ])

  CRX_CHECK_LIBRARY(zip, zip_compression_method_supported,
  [
    AC_DEFINE(HAVE_METHOD_SUPPORTED, 1, [Libzip >= 1.7.0 with zip_*_method_supported functions])
  ], [
  ], [
    $LIBZIP_LIBS
  ])

  AC_DEFINE(HAVE_ZIP,1,[ ])

  CRX_ZIP_SOURCES="crx_zip.c zip_stream.c"
  CRX_NEW_EXTENSION(zip, $CRX_ZIP_SOURCES, $ext_shared)

  CRX_SUBST(ZIP_SHARED_LIBADD)
fi
