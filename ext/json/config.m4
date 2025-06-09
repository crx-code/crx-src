dnl HAVE_JSON is always 1 as of crx 8.0 and the constant will be removed in the future.
dnl Note that HAVE_JSON was never defined for Windows builds (see config.w32)
AC_DEFINE([HAVE_JSON],1 ,[whether to enable JavaScript Object Serialization support])
CRX_NEW_EXTENSION(json,
	  json.c \
	  json_encoder.c \
	  json_parser.tab.c \
	  json_scanner.c,
	  no,, -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1)
CRX_INSTALL_HEADERS([ext/json], [crx_json.h crx_json_parser.h crx_json_scanner.h])
CRX_ADD_MAKEFILE_FRAGMENT()
