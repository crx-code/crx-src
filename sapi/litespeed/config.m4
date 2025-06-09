AC_MSG_CHECKING(for LiteSpeed support)

CRX_ARG_ENABLE([litespeed],,
  [AS_HELP_STRING([--enable-litespeed],
    [Build CRX as litespeed module])],
  [no])

if test "$CRX_LITESPEED" != "no"; then
  CRX_ADD_MAKEFILE_FRAGMENT($abs_srcdir/sapi/litespeed/Makefile.frag,$abs_srcdir/sapi/litespeed,sapi/litespeed)
  SAPI_LITESPEED_PATH=sapi/litespeed/crx
  CRX_SELECT_SAPI(litespeed, program, lsapi_main.c lsapilib.c, "", '$(SAPI_LITESPEED_PATH)')
  case $host_alias in
  *darwin*)
    BUILD_LITESPEED="\$(CC) \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) \$(NATIVE_RPATHS) \$(CRX_GLOBAL_OBJS:.lo=.o) \$(CRX_BINARY_OBJS:.lo=.o) \$(CRX_LITESPEED_OBJS:.lo=.o) \$(CRX_FRAMEWORKS) \$(EXTRA_LIBS) \$(CREX_EXTRA_LIBS) -o \$(SAPI_LITESPEED_PATH)"
    ;;
  *cygwin*)
    SAPI_LITESPEED_PATH=sapi/litespeed/crx.exe
    BUILD_LITESPEED="\$(LIBTOOL) --mode=link \$(CC) -export-dynamic \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) \$(CRX_RPATHS) \$(CRX_GLOBAL_OBJS) \$(CRX_BINARY_OBJS) \$(CRX_LITESPEED_OBJS) \$(EXTRA_LIBS) \$(CREX_EXTRA_LIBS) -o \$(SAPI_LITESPEED_PATH)"
    ;;
  *)
    BUILD_LITESPEED="\$(LIBTOOL) --mode=link \$(CC) -export-dynamic \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) \$(CRX_RPATHS) \$(CRX_GLOBAL_OBJS:.lo=.o) \$(CRX_BINARY_OBJS:.lo=.o) \$(CRX_LITESPEED_OBJS:.lo=.o) \$(EXTRA_LIBS) \$(CREX_EXTRA_LIBS) -o \$(SAPI_LITESPEED_PATH)"
    ;;
  esac

  CRX_SUBST(SAPI_LITESPEED_PATH)
  CRX_SUBST(BUILD_LITESPEED)
fi

AC_MSG_RESULT($CRX_LITESPEED)
