CRX_ARG_ENABLE([filter],
  [whether to enable input filter support],
  [AS_HELP_STRING([--disable-filter],
    [Disable input filter support])],
  [yes])

if test "$CRX_FILTER" != "no"; then
  CRX_NEW_EXTENSION(filter, filter.c sanitizing_filters.c logical_filters.c callback_filter.c, $ext_shared,, -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1)
  CRX_SUBST(FILTER_SHARED_LIBADD)

  CRX_INSTALL_HEADERS([ext/filter/crx_filter.h])
  CRX_ADD_EXTENSION_DEP(filter, pcre)
fi
