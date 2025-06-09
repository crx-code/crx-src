CRX_ARG_ENABLE([cgi],,
  [AS_HELP_STRING([--disable-cgi],
    [Disable building CGI version of CRX])],
  [yes],
  [no])

dnl CGI setup.
AC_MSG_CHECKING(for CGI build)
if test "$CRX_CGI" != "no"; then
    AC_MSG_RESULT(yes)

    AC_MSG_CHECKING([for sun_len in sys/un.h])
    AC_EGREP_HEADER([sun_len], [sys/un.h],
      [AC_MSG_RESULT([yes])
       AC_DEFINE([HAVE_SOCKADDR_UN_SUN_LEN], [1],
        [Define if sockaddr_un in sys/un.h contains a sun_len component])],
      AC_MSG_RESULT([no]))

    AC_MSG_CHECKING([whether cross-process locking is required by accept()])
    case "`uname -sr`" in
      IRIX\ 5.* | SunOS\ 5.* | UNIX_System_V\ 4.0)
        AC_MSG_RESULT([yes])
        AC_DEFINE([USE_LOCKING], [1],
          [Define if cross-process locking is required by accept()])
      ;;
      *)
        AC_MSG_RESULT([no])
      ;;
    esac

    CRX_ADD_MAKEFILE_FRAGMENT($abs_srcdir/sapi/cgi/Makefile.frag)

    dnl Set filename.
    case $host_alias in
      *cygwin* )
        SAPI_CGI_PATH=sapi/cgi/crx-cgi.exe
        ;;
      * )
        SAPI_CGI_PATH=sapi/cgi/crx-cgi
        ;;
    esac

    dnl Select SAPI.
    CRX_SELECT_SAPI(cgi, program, cgi_main.c, -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1, '$(SAPI_CGI_PATH)')

    case $host_alias in
      *aix*)
        if test "$crx_sapi_module" = "shared"; then
          BUILD_CGI="echo '\#! .' > crx.sym && echo >>crx.sym && nm -BCpg \`echo \$(CRX_GLOBAL_OBJS) \$(CRX_BINARY_OBJS) \$(CRX_CGI_OBJS) | sed 's/\([A-Za-z0-9_]*\)\.lo/.libs\/\1.o/g'\` | \$(AWK) '{ if (((\$\$2 == \"T\") || (\$\$2 == \"D\") || (\$\$2 == \"B\")) && (substr(\$\$3,1,1) != \".\")) { print \$\$3 } }' | sort -u >> crx.sym && \$(LIBTOOL) --mode=link \$(CC) -export-dynamic \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) -Wl,-brtl -Wl,-bE:crx.sym \$(CRX_RPATHS) \$(CRX_GLOBAL_OBJS) \$(CRX_BINARY_OBJS) \$(CRX_FASTCGI_OBJS) \$(CRX_CGI_OBJS) \$(EXTRA_LIBS) \$(CREX_EXTRA_LIBS) -o \$(SAPI_CGI_PATH)"
        else
          BUILD_CGI="echo '\#! .' > crx.sym && echo >>crx.sym && nm -BCpg \`echo \$(CRX_GLOBAL_OBJS) \$(CRX_BINARY_OBJS) \$(CRX_CGI_OBJS) | sed 's/\([A-Za-z0-9_]*\)\.lo/\1.o/g'\` | \$(AWK) '{ if (((\$\$2 == \"T\") || (\$\$2 == \"D\") || (\$\$2 == \"B\")) && (substr(\$\$3,1,1) != \".\")) { print \$\$3 } }' | sort -u >> crx.sym && \$(LIBTOOL) --mode=link \$(CC) -export-dynamic \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) -Wl,-brtl -Wl,-bE:crx.sym \$(CRX_RPATHS) \$(CRX_GLOBAL_OBJS) \$(CRX_BINARY_OBJS) \$(CRX_FASTCGI_OBJS) \$(CRX_CGI_OBJS) \$(EXTRA_LIBS) \$(CREX_EXTRA_LIBS) -o \$(SAPI_CGI_PATH)"
        fi
        ;;
      *darwin*)
        BUILD_CGI="\$(CC) \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) \$(NATIVE_RPATHS) \$(CRX_GLOBAL_OBJS:.lo=.o) \$(CRX_BINARY_OBJS:.lo=.o) \$(CRX_FASTCGI_OBJS:.lo=.o) \$(CRX_CGI_OBJS:.lo=.o) \$(CRX_FRAMEWORKS) \$(EXTRA_LIBS) \$(CREX_EXTRA_LIBS) -o \$(SAPI_CGI_PATH)"
      ;;
      *)
        BUILD_CGI="\$(LIBTOOL) --mode=link \$(CC) -export-dynamic \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) \$(CRX_RPATHS) \$(CRX_GLOBAL_OBJS:.lo=.o) \$(CRX_BINARY_OBJS:.lo=.o) \$(CRX_FASTCGI_OBJS:.lo=.o) \$(CRX_CGI_OBJS:.lo=.o) \$(EXTRA_LIBS) \$(CREX_EXTRA_LIBS) -o \$(SAPI_CGI_PATH)"
      ;;
    esac

    dnl Expose to Makefile.
    CRX_SUBST(SAPI_CGI_PATH)
    CRX_SUBST(BUILD_CGI)

    CRX_OUTPUT(sapi/cgi/crx-cgi.1)
else
  AC_MSG_RESULT(no)
fi
