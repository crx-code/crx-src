CRX_ARG_ENABLE([cli],,
  [AS_HELP_STRING([--disable-cli],
    [Disable building CLI version of CRX (this forces --without-pear)])],
  [yes],
  [no])

AC_CHECK_FUNCS(setproctitle)

AC_CHECK_HEADERS([sys/pstat.h])

AC_CACHE_CHECK([for PS_STRINGS], [cli_cv_var_PS_STRINGS],
[AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <machine/vmparam.h>
#include <sys/exec.h>
]],
[[PS_STRINGS->ps_nargvstr = 1;
PS_STRINGS->ps_argvstr = "foo";]])],
[cli_cv_var_PS_STRINGS=yes],
[cli_cv_var_PS_STRINGS=no])])
if test "$cli_cv_var_PS_STRINGS" = yes ; then
  AC_DEFINE([HAVE_PS_STRINGS], [], [Define to 1 if the PS_STRINGS thing exists.])
fi

AC_MSG_CHECKING(for CLI build)
if test "$CRX_CLI" != "no"; then
  CRX_ADD_MAKEFILE_FRAGMENT($abs_srcdir/sapi/cli/Makefile.frag)

  dnl Set filename.
  SAPI_CLI_PATH=sapi/cli/crx

  dnl Select SAPI.
  CRX_SELECT_SAPI(cli, program, crx_cli.c crx_http_parser.c crx_cli_server.c ps_title.c crx_cli_process_title.c, -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1, '$(SAPI_CLI_PATH)')

  case $host_alias in
  *aix*)
    if test "$crx_sapi_module" = "shared"; then
      BUILD_CLI="echo '\#! .' > crx.sym && echo >>crx.sym && nm -BCpg \`echo \$(CRX_GLOBAL_OBJS) \$(CRX_BINARY_OBJS) \$(CRX_CLI_OBJS) | sed 's/\([A-Za-z0-9_]*\)\.lo/.libs\/\1.o/g'\` | \$(AWK) '{ if (((\$\$2 == \"T\") || (\$\$2 == \"D\") || (\$\$2 == \"B\")) && (substr(\$\$3,1,1) != \".\")) { print \$\$3 } }' | sort -u >> crx.sym && \$(LIBTOOL) --mode=link \$(CC) -export-dynamic \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) -Wl,-brtl -Wl,-bE:crx.sym \$(CRX_RPATHS) \$(CRX_GLOBAL_OBJS) \$(CRX_BINARY_OBJS) \$(CRX_CLI_OBJS) \$(EXTRA_LIBS) \$(CREX_EXTRA_LIBS) -o \$(SAPI_CLI_PATH)"
    else
      BUILD_CLI="echo '\#! .' > crx.sym && echo >>crx.sym && nm -BCpg \`echo \$(CRX_GLOBAL_OBJS) \$(CRX_BINARY_OBJS) \$(CRX_CLI_OBJS) | sed 's/\([A-Za-z0-9_]*\)\.lo/\1.o/g'\` | \$(AWK) '{ if (((\$\$2 == \"T\") || (\$\$2 == \"D\") || (\$\$2 == \"B\")) && (substr(\$\$3,1,1) != \".\")) { print \$\$3 } }' | sort -u >> crx.sym && \$(LIBTOOL) --mode=link \$(CC) -export-dynamic \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) -Wl,-brtl -Wl,-bE:crx.sym \$(CRX_RPATHS) \$(CRX_GLOBAL_OBJS) \$(CRX_BINARY_OBJS) \$(CRX_CLI_OBJS) \$(EXTRA_LIBS) \$(CREX_EXTRA_LIBS) -o \$(SAPI_CLI_PATH)"
    fi
    ;;
  *darwin*)
    BUILD_CLI="\$(CC) \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) \$(NATIVE_RPATHS) \$(CRX_GLOBAL_OBJS:.lo=.o) \$(CRX_BINARY_OBJS:.lo=.o) \$(CRX_CLI_OBJS:.lo=.o) \$(CRX_FRAMEWORKS) \$(EXTRA_LIBS) \$(CREX_EXTRA_LIBS) -o \$(SAPI_CLI_PATH)"
    ;;
  *)
    BUILD_CLI="\$(LIBTOOL) --mode=link \$(CC) -export-dynamic \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) \$(CRX_RPATHS) \$(CRX_GLOBAL_OBJS:.lo=.o) \$(CRX_BINARY_OBJS:.lo=.o) \$(CRX_CLI_OBJS:.lo=.o) \$(EXTRA_LIBS) \$(CREX_EXTRA_LIBS) -o \$(SAPI_CLI_PATH)"
    ;;
  esac

  dnl Set executable for tests.
  CRX_EXECUTABLE="\$(top_builddir)/\$(SAPI_CLI_PATH)"
  CRX_SUBST(CRX_EXECUTABLE)

  dnl Expose to Makefile.
  CRX_SUBST(SAPI_CLI_PATH)
  CRX_SUBST(BUILD_CLI)

  CRX_OUTPUT(sapi/cli/crx.1)

  CRX_INSTALL_HEADERS([sapi/cli/cli.h])
fi
AC_MSG_RESULT($CRX_CLI)
