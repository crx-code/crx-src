CRX_ARG_WITH([libedit],
  [for libedit readline replacement],
  [AS_HELP_STRING([--with-libedit],
    [Include libedit readline replacement (CLI/CGI only)])])

if test "$CRX_LIBEDIT" = "no"; then
  CRX_ARG_WITH([readline],
    [for readline support],
    [AS_HELP_STRING([[--with-readline[=DIR]]],
      [Include readline support (CLI/CGI only)])])
else
  dnl "register" the --with-readline option to prevent invalid "unknown
  dnl configure option" warning
  crx_with_readline=no
fi

if test "$CRX_READLINE" && test "$CRX_READLINE" != "no"; then
  for i in $CRX_READLINE /usr/local /usr; do
    test -f $i/include/readline/readline.h && READLINE_DIR=$i && break
  done

  if test -z "$READLINE_DIR"; then
    AC_MSG_ERROR(Please reinstall readline - I cannot find readline.h)
  fi

  CRX_ADD_INCLUDE($READLINE_DIR/include)

  CRX_READLINE_LIBS=""
  AC_CHECK_LIB(ncurses, tgetent,
  [
    CRX_ADD_LIBRARY(ncurses,,READLINE_SHARED_LIBADD)
    CRX_READLINE_LIBS="$CRX_READLINE_LIBS -lncurses"
  ],[
    AC_CHECK_LIB(termcap, tgetent,
    [
      CRX_ADD_LIBRARY(termcap,,READLINE_SHARED_LIBADD)
      CRX_READLINE_LIBS="$CRX_READLINE_LIBS -ltermcap"
    ])
  ])

  CRX_CHECK_LIBRARY(readline, readline,
  [
    CRX_ADD_LIBRARY_WITH_PATH(readline, $READLINE_DIR/$CRX_LIBDIR, READLINE_SHARED_LIBADD)
  ], [
    AC_MSG_ERROR(readline library not found)
  ], [
    -L$READLINE_DIR/$CRX_LIBDIR $CRX_READLINE_LIBS
  ])

  CRX_CHECK_LIBRARY(readline, rl_pending_input,
  [], [
    AC_MSG_ERROR([invalid readline installation detected. Try --with-libedit instead.])
  ], [
    -L$READLINE_DIR/$CRX_LIBDIR $CRX_READLINE_LIBS
  ])

  CRX_CHECK_LIBRARY(readline, rl_callback_read_char,
  [
    AC_DEFINE(HAVE_RL_CALLBACK_READ_CHAR, 1, [ ])
  ],[],[
    -L$READLINE_DIR/$CRX_LIBDIR $CRX_READLINE_LIBS
  ])

  CRX_CHECK_LIBRARY(readline, rl_on_new_line,
  [
    AC_DEFINE(HAVE_RL_ON_NEW_LINE, 1, [ ])
  ],[],[
    -L$READLINE_DIR/$CRX_LIBDIR $CRX_READLINE_LIBS
  ])

  CRX_CHECK_LIBRARY(readline, rl_completion_matches,
  [
    AC_DEFINE(HAVE_RL_COMPLETION_MATCHES, 1, [ ])
  ],[],[
    -L$READLINE_DIR/$CRX_LIBDIR $CRX_READLINE_LIBS
  ])

  AC_DEFINE(HAVE_HISTORY_LIST, 1, [ ])
  AC_DEFINE(HAVE_LIBREADLINE, 1, [ ])

elif test "$CRX_LIBEDIT" != "no"; then
  if test "$CRX_LIBEDIT" != "yes"; then
    AC_MSG_WARN([libedit directory ignored, rely on pkg-config])
  fi

  PKG_CHECK_MODULES([EDIT], [libedit])
  CRX_EVAL_LIBLINE($EDIT_LIBS, READLINE_SHARED_LIBADD)
  CRX_EVAL_INCLINE($EDIT_CFLAGS)

  AC_CHECK_LIB(ncurses, tgetent,
  [
    CRX_ADD_LIBRARY(ncurses,,READLINE_SHARED_LIBADD)
  ],[
    AC_CHECK_LIB(termcap, tgetent,
    [
      CRX_ADD_LIBRARY(termcap,,READLINE_SHARED_LIBADD)
    ])
  ])

  CRX_CHECK_LIBRARY(edit, readline,
  [
  ], [
    AC_MSG_ERROR(edit library required by readline not found)
  ], [
    $READLINE_SHARED_LIBADD
  ])

  CRX_CHECK_LIBRARY(edit, rl_callback_read_char,
  [
    AC_DEFINE(HAVE_RL_CALLBACK_READ_CHAR, 1, [ ])
  ],[],[
    $READLINE_SHARED_LIBADD
  ])

  CRX_CHECK_LIBRARY(edit, rl_on_new_line,
  [
    AC_DEFINE(HAVE_RL_ON_NEW_LINE, 1, [ ])
  ],[],[
    $READLINE_SHARED_LIBADD
  ])

  CRX_CHECK_LIBRARY(edit, rl_completion_matches,
  [
    AC_DEFINE(HAVE_RL_COMPLETION_MATCHES, 1, [ ])
  ],[],[
    $READLINE_SHARED_LIBADD
  ])

  CRX_CHECK_LIBRARY(edit, history_list,
  [
    AC_DEFINE(HAVE_HISTORY_LIST, 1, [ ])
  ],[],[
    $READLINE_SHARED_LIBADD
  ])

  AC_DEFINE(HAVE_LIBEDIT, 1, [ ])
fi

if test "$CRX_READLINE" != "no" || test "$CRX_LIBEDIT" != "no"; then
  dnl Add -Wno-strict-prototypes as depends on user libs
  CRX_NEW_EXTENSION(readline, readline.c readline_cli.c, $ext_shared, cli, "-Wno-strict-prototypes")
  CRX_SUBST(READLINE_SHARED_LIBADD)
fi
