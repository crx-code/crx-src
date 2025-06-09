CRX_ARG_WITH([pdo-sqlite],
  [for sqlite 3 support for PDO],
  [AS_HELP_STRING([--without-pdo-sqlite],
    [PDO: sqlite 3 support.])],
  [$CRX_PDO])

if test "$CRX_PDO_SQLITE" != "no"; then

  if test "$CRX_PDO" = "no" && test "$ext_shared" = "no"; then
    AC_MSG_ERROR([PDO is not enabled! Add --enable-pdo to your configure line.])
  fi

  CRX_CHECK_PDO_INCLUDES

  PKG_CHECK_MODULES([SQLITE], [sqlite3 >= 3.7.7])

  CRX_EVAL_INCLINE($SQLITE_CFLAGS)
  CRX_EVAL_LIBLINE($SQLITE_LIBS, PDO_SQLITE_SHARED_LIBADD)
  AC_DEFINE(HAVE_PDO_SQLITELIB, 1, [Define to 1 if you have the pdo_sqlite extension enabled.])

  CRX_CHECK_LIBRARY(sqlite3, sqlite3_close_v2, [
    AC_DEFINE(HAVE_SQLITE3_CLOSE_V2, 1, [have sqlite3_close_v2])
  ], [], [$PDO_SQLITE_SHARED_LIBADD])

  CRX_CHECK_LIBRARY(sqlite3, sqlite3_column_table_name, [
    AC_DEFINE(HAVE_SQLITE3_COLUMN_TABLE_NAME, 1, [have sqlite3_column_table_name])
  ], [], [$PDO_SQLITE_SHARED_LIBADD])

  CRX_SUBST(PDO_SQLITE_SHARED_LIBADD)
  CRX_NEW_EXTENSION(pdo_sqlite, pdo_sqlite.c sqlite_driver.c sqlite_statement.c,
    $ext_shared,,-I$pdo_cv_inc_path)

  CRX_ADD_EXTENSION_DEP(pdo_sqlite, pdo)
fi
