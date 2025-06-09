CRX_ARG_ENABLE([mysqlnd],
  [whether to enable mysqlnd],
  [AS_HELP_STRING([--enable-mysqlnd],
    [Enable mysqlnd explicitly, will be done implicitly when required by other
    extensions])],
  [no],
  [yes])

CRX_ARG_ENABLE([mysqlnd-compression-support],
  [whether to enable compressed protocol support in mysqlnd],
  [AS_HELP_STRING([--disable-mysqlnd-compression-support],
    [Disable support for the MySQL compressed protocol in mysqlnd])],
  [yes],
  [no])

dnl If some extension uses mysqlnd it will get compiled in CRX core
if test "$CRX_MYSQLND" != "no" || test "$CRX_MYSQLND_ENABLED" = "yes"; then
  mysqlnd_ps_sources="mysqlnd_ps.c mysqlnd_ps_codec.c"
  mysqlnd_base_sources="mysqlnd_connection.c mysqlnd_alloc.c mysqlnd_charset.c mysqlnd_wireprotocol.c \
                   mysqlnd_loaddata.c mysqlnd_reverse_api.c mysqlnd_vio.c mysqlnd_protocol_frame_codec.c \
                   mysqlnd_statistics.c mysqlnd_driver.c mysqlnd_ext_plugin.c mysqlnd_auth.c \
				   mysqlnd_result.c mysqlnd_result_meta.c mysqlnd_debug.c mysqlnd_commands.c \
				   mysqlnd_block_alloc.c mysqlnd_read_buffer.c mysqlnd_plugin.c crx_mysqlnd.c"


  if test "$CRX_MYSQLND_COMPRESSION_SUPPORT" != "no"; then
    PKG_CHECK_MODULES([ZLIB], [zlib])
    CRX_EVAL_LIBLINE($ZLIB_LIBS, MYSQLND_SHARED_LIBADD)
    CRX_EVAL_INCLINE($ZLIB_CFLAGS)
    AC_DEFINE([MYSQLND_COMPRESSION_ENABLED], 1, [Enable compressed protocol support])
  fi

  AC_DEFINE([MYSQLND_SSL_SUPPORTED], 1, [Enable core mysqlnd SSL code])

  test -z "$CRX_OPENSSL" && CRX_OPENSSL=no

  if test "$CRX_OPENSSL" != "no" || test "$CRX_OPENSSL_DIR" != "no"; then
    CRX_SETUP_OPENSSL(MYSQLND_SHARED_LIBADD, [AC_DEFINE(MYSQLND_HAVE_SSL,1,[Enable mysqlnd code that uses OpenSSL directly])])
  fi

  mysqlnd_sources="$mysqlnd_base_sources $mysqlnd_ps_sources"
  CRX_NEW_EXTENSION(mysqlnd, $mysqlnd_sources, $ext_shared,, -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1)
  CRX_ADD_BUILD_DIR([ext/mysqlnd], 1)
  CRX_INSTALL_HEADERS([ext/mysqlnd/])
fi

if test "$CRX_MYSQLND" != "no" || test "$CRX_MYSQLND_ENABLED" = "yes" || test "$CRX_MYSQLI" != "no"; then
  CRX_ADD_BUILD_DIR([ext/mysqlnd], 1)
fi
