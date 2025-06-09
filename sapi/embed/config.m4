CRX_ARG_ENABLE([embed],,
  [AS_HELP_STRING([[--enable-embed[=TYPE]]],
    [EXPERIMENTAL: Enable building of embedded SAPI library TYPE is either
    'shared' or 'static'. [TYPE=shared]])],
  [no],
  [no])

AC_MSG_CHECKING([for embedded SAPI library support])

if test "$CRX_EMBED" != "no"; then
  case "$CRX_EMBED" in
    yes|shared)
      LIBCRX_CFLAGS="-shared"
      CRX_EMBED_TYPE=shared
      INSTALL_IT="\$(mkinstalldirs) \$(INSTALL_ROOT)\$(prefix)/lib; \$(INSTALL) -m 0755 $SAPI_SHARED \$(INSTALL_ROOT)\$(prefix)/lib"
      ;;
    static)
      LIBCRX_CFLAGS="-static"
      CRX_EMBED_TYPE=static
      INSTALL_IT="\$(mkinstalldirs) \$(INSTALL_ROOT)\$(prefix)/lib; \$(INSTALL) -m 0644 $SAPI_STATIC \$(INSTALL_ROOT)\$(prefix)/lib"
      ;;
    *)
      CRX_EMBED_TYPE=no
      ;;
  esac
  if test "$CRX_EMBED_TYPE" != "no"; then
    CRX_SUBST(LIBCRX_CFLAGS)
    CRX_SELECT_SAPI(embed, $CRX_EMBED_TYPE, crx_embed.c, -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1)
    CRX_INSTALL_HEADERS([sapi/embed/crx_embed.h])
  fi
  AC_MSG_RESULT([$CRX_EMBED_TYPE])
else
  AC_MSG_RESULT(no)
fi
