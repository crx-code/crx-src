CRX_ARG_WITH([sqlite3],
  [whether to enable the SQLite3 extension],
  [AS_HELP_STRING([--without-sqlite3],
    [Do not include SQLite3 support.])],
  [yes])

if test $CRX_SQLITE3 != "no"; then
  PKG_CHECK_MODULES([SQLITE], [sqlite3 >= 3.7.7])

  CRX_EVAL_INCLINE($SQLITE_CFLAGS)
  CRX_EVAL_LIBLINE($SQLITE_LIBS, SQLITE3_SHARED_LIBADD)
  AC_DEFINE(HAVE_SQLITE3, 1, [Define to 1 if you have the sqlite3 extension enabled.])

  CRX_CHECK_LIBRARY(sqlite3, sqlite3_errstr, [
    AC_DEFINE(HAVE_SQLITE3_ERRSTR, 1, [have sqlite3_errstr function])
  ], [], [$SQLITE3_SHARED_LIBADD])

  CRX_CHECK_LIBRARY(sqlite3, sqlite3_expanded_sql, [
    AC_DEFINE(HAVE_SQLITE3_EXPANDED_SQL, 1, [have sqlite3_expanded_sql function])
  ], [], [$SQLITE3_SHARED_LIBADD])

  CRX_CHECK_LIBRARY(sqlite3,sqlite3_load_extension,
    [],
    [AC_DEFINE(SQLITE_OMIT_LOAD_EXTENSION, 1, [have sqlite3 with extension support])],
    [$SQLITE3_SHARED_LIBADD]
  )

  CRX_NEW_EXTENSION(sqlite3, sqlite3.c, $ext_shared,,-DCREX_ENABLE_STATIC_TSRMLS_CACHE=1)
  CRX_SUBST(SQLITE3_SHARED_LIBADD)
fi
