CRX_ARG_ENABLE([dl-test],
  [whether to enable dl-test extension],
  [AS_HELP_STRING([--enable-dl-test],
    [Enable dl_test extension])])

if test "$CRX_DL_TEST" != "no"; then
  CRX_NEW_EXTENSION(dl_test, dl_test.c, [shared],, -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
