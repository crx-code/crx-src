CRX_ARG_ENABLE([crxdbg],
  [for crxdbg support],
  [AS_HELP_STRING([--disable-crxdbg],
    [Disable building of crxdbg])],
  [yes],
  [yes])

CRX_ARG_ENABLE([crxdbg-debug],
  [for crxdbg debug build],
  [AS_HELP_STRING([--enable-crxdbg-debug],
    [Build crxdbg in debug mode])],
  [no],
  [no])

CRX_ARG_ENABLE([crxdbg-readline],
  [for crxdbg readline support],
  [AS_HELP_STRING([--enable-crxdbg-readline],
    [Enable readline support in crxdbg (depends on static ext/readline)])],
  [no],
  [no])

if test "$BUILD_CRXDBG" = "" && test "$CRX_CRXDBG" != "no"; then
  AC_HEADER_TIOCGWINSZ
  AC_DEFINE(HAVE_CRXDBG, 1, [ ])

  if test "$CRX_CRXDBG_DEBUG" != "no"; then
    AC_DEFINE(CRXDBG_DEBUG, 1, [ ])
  else
    AC_DEFINE(CRXDBG_DEBUG, 0, [ ])
  fi

  CRX_CRXDBG_CFLAGS="-D_GNU_SOURCE -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1"
  CRX_CRXDBG_FILES="crxdbg.c crxdbg_parser.c crxdbg_lexer.c crxdbg_prompt.c crxdbg_help.c crxdbg_break.c crxdbg_print.c crxdbg_bp.c crxdbg_list.c crxdbg_utils.c crxdbg_info.c crxdbg_cmd.c crxdbg_set.c crxdbg_frame.c crxdbg_watch.c crxdbg_btree.c crxdbg_sigsafe.c crxdbg_io.c crxdbg_out.c"

  AC_MSG_CHECKING([for crxdbg and readline integration])
  if test "$CRX_CRXDBG_READLINE" = "yes"; then
    if test "$CRX_READLINE" != "no" -o  "$CRX_LIBEDIT" != "no"; then
  	  AC_DEFINE(HAVE_CRXDBG_READLINE, 1, [ ])
  	  CRXDBG_EXTRA_LIBS="$CRX_READLINE_LIBS"
  	  AC_MSG_RESULT([ok])
  	else
  	  AC_MSG_RESULT([readline is not available])
    fi
  else
    AC_MSG_RESULT([disabled])
  fi

  AC_CACHE_CHECK([for userfaultfd faulting on write-protected memory support], ac_cv_crxdbg_userfaultfd_writefault, AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
    #include <linux/userfaultfd.h>
    #ifndef UFFDIO_WRITEPROTECT_MODE_WP
    # error userfaults on write-protected memory not supported
    #endif
  ]])], [ac_cv_crxdbg_userfaultfd_writefault=yes], [ac_cv_crxdbg_userfaultfd_writefault=no]))
  if test "$ac_cv_crxdbg_userfaultfd_writefault" = "yes"; then
    if test "$enable_zts" != "yes"; then
      CFLAGS_SAVE="$CFLAGS"
      LIBS_SAVE="$LIBS"

      PTHREADS_CHECK
      AC_MSG_CHECKING([working pthreads]);

      if test "$pthreads_working" = "yes"; then
      	AC_MSG_RESULT([$ac_cv_pthreads_cflags -l$ac_cv_pthreads_lib]);
      	CRX_CRXDBG_CFLAGS="$CRX_CRXDBG_CFLAGS $ac_cv_pthreads_cflags"
      	CRXDBG_EXTRA_LIBS="$CRXDBG_EXTRA_LIBS -l$ac_cv_pthreads_lib"
      	AC_DEFINE(HAVE_USERFAULTFD_WRITEFAULT, 1, [Whether faulting on write-protected memory support can be compiled for userfaultfd])
      else
        AC_MSG_WARN([pthreads not available])
      fi

      CFLAGS="$CFLAGS_SAVE"
      LIBS="$LIBS_SAVE"
    else
      AC_DEFINE(HAVE_USERFAULTFD_WRITEFAULT, 1, [Whether faulting on write-protected memory support can be compiled for userfaultfd])
    fi
  fi

  CRX_SUBST(CRX_CRXDBG_CFLAGS)
  CRX_SUBST(CRX_CRXDBG_FILES)
  CRX_SUBST(CRXDBG_EXTRA_LIBS)

  CRX_ADD_MAKEFILE_FRAGMENT([$abs_srcdir/sapi/crxdbg/Makefile.frag], [$abs_srcdir/sapi/crxdbg], [$abs_builddir/sapi/crxdbg])
  CRX_SELECT_SAPI(crxdbg, program, $CRX_CRXDBG_FILES, $CRX_CRXDBG_CFLAGS, [$(SAPI_CRXDBG_PATH)])

  BUILD_BINARY="sapi/crxdbg/crxdbg"
  BUILD_SHARED="sapi/crxdbg/libcrxdbg.la"

  BUILD_CRXDBG="\$(LIBTOOL) --mode=link \
        \$(CC) -export-dynamic \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) \$(CRX_RPATHS) \
                \$(CRX_GLOBAL_OBJS:.lo=.o) \
                \$(CRX_BINARY_OBJS:.lo=.o) \
                \$(CRX_CRXDBG_OBJS:.lo=.o) \
                \$(EXTRA_LIBS) \
                \$(CRXDBG_EXTRA_LIBS) \
                \$(CREX_EXTRA_LIBS) \
                \$(CRX_FRAMEWORKS) \
         -o \$(BUILD_BINARY)"

  BUILD_CRXDBG_SHARED="\$(LIBTOOL) --mode=link \
        \$(CC) -shared -Wl,-soname,libcrxdbg.so -export-dynamic \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) \$(CRX_RPATHS) \
                \$(CRX_GLOBAL_OBJS) \
                \$(CRX_BINARY_OBJS) \
                \$(CRX_CRXDBG_OBJS) \
                \$(EXTRA_LIBS) \
                \$(CRXDBG_EXTRA_LIBS) \
                \$(CREX_EXTRA_LIBS) \
                \-DCRXDBG_SHARED \
         -o \$(BUILD_SHARED)"

  CRX_SUBST(BUILD_BINARY)
  CRX_SUBST(BUILD_SHARED)
  CRX_SUBST(BUILD_CRXDBG)
  CRX_SUBST(BUILD_CRXDBG_SHARED)

  CRX_OUTPUT(sapi/crxdbg/crxdbg.1)
fi
