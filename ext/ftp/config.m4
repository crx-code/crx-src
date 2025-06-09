CRX_ARG_ENABLE([ftp],
  [whether to enable FTP support],
  [AS_HELP_STRING([--enable-ftp],
    [Enable FTP support])])

dnl TODO: Rename this option for master.
CRX_ARG_WITH([openssl-dir],
  [whether to explicitly enable FTP SSL support],
  [AS_HELP_STRING([[--with-openssl-dir]],
    [FTP: Whether to enable FTP SSL support without ext/openssl])],
  [no],
  [no])

if test "$CRX_FTP" = "yes"; then
  AC_DEFINE(HAVE_FTP,1,[Whether you want FTP support])
  CRX_NEW_EXTENSION(ftp, crx_ftp.c ftp.c, $ext_shared)

  dnl Empty variable means 'no'
  test -z "$CRX_OPENSSL" && CRX_OPENSSL=no

  if test "$CRX_OPENSSL" != "no" || test "$CRX_OPENSSL_DIR" != "no"; then
    CRX_SETUP_OPENSSL(FTP_SHARED_LIBADD)
    CRX_SUBST(FTP_SHARED_LIBADD)
    AC_DEFINE(HAVE_FTP_SSL,1,[Whether FTP over SSL is supported])
  fi
fi
