CRX_ARG_ENABLE([pdo],
  [whether to enable PDO support],
  [AS_HELP_STRING([--disable-pdo],
    [Disable CRX Data Objects support])],
  [yes])

if test "$CRX_PDO" != "no"; then

  dnl Make sure $CRX_PDO is 'yes' when it's not 'no' :)
  CRX_PDO=yes

  CRX_NEW_EXTENSION(pdo, pdo.c pdo_dbh.c pdo_stmt.c pdo_sql_parser.c pdo_sqlstate.c, $ext_shared)
  CRX_ADD_EXTENSION_DEP(pdo, spl, true)
  CRX_INSTALL_HEADERS(ext/pdo, [crx_pdo.h crx_pdo_driver.h crx_pdo_error.h])

  dnl so we always include the known-good working hack.
  CRX_ADD_MAKEFILE_FRAGMENT
fi
