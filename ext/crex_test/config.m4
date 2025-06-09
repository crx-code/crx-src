CRX_ARG_ENABLE([crex-test],
  [whether to enable crex-test extension],
  [AS_HELP_STRING([--enable-crex-test],
    [Enable crex_test extension])])

if test "$CRX_CREX_TEST" != "no"; then
  CRX_NEW_EXTENSION(crex_test, test.c observer.c fiber.c iterators.c object_handlers.c, $ext_shared,, -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
