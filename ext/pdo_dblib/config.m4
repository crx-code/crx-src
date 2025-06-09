CRX_ARG_WITH([pdo-dblib],
  [for PDO_DBLIB support via FreeTDS],
  [AS_HELP_STRING([[--with-pdo-dblib[=DIR]]],
    [PDO: DBLIB-DB support. DIR is the FreeTDS home directory])])

if test "$CRX_PDO_DBLIB" != "no"; then

  if test "$CRX_PDO" = "no" && test "$ext_shared" = "no"; then
    AC_MSG_ERROR([PDO is not enabled! Add --enable-pdo to your configure line.])
  fi

  if test "$CRX_PDO_DBLIB" = "yes"; then
    dnl FreeTDS must be on the default system include/library path.
    dnl Only perform a sanity check that this is really the case.
    CRX_CHECK_LIBRARY(sybdb, dbsqlexec,
    [],[
      AC_MSG_ERROR([Cannot find FreeTDS in known installation directories])
    ])
    CRX_ADD_LIBRARY(sybdb,,PDO_DBLIB_SHARED_LIBADD)
  elif test "$CRX_PDO_DBLIB" != "no"; then

    if test -f $CRX_PDO_DBLIB/include/sybdb.h; then
      PDO_FREETDS_INSTALLATION_DIR=$CRX_PDO_DBLIB
      PDO_FREETDS_INCLUDE_DIR=$CRX_PDO_DBLIB/include
    elif test -f $CRX_PDO_DBLIB/include/freetds/sybdb.h; then
      PDO_FREETDS_INSTALLATION_DIR=$CRX_PDO_DBLIB
      PDO_FREETDS_INCLUDE_DIR=$CRX_PDO_DBLIB/include/freetds
    else
      AC_MSG_ERROR(Directory $CRX_PDO_DBLIB is not a FreeTDS installation directory)
    fi

    if test "x$CRX_LIBDIR" = "x" ; then
      CRX_LIBDIR=lib
    fi

    if test ! -r "$PDO_FREETDS_INSTALLATION_DIR/$CRX_LIBDIR/libsybdb.a" && test ! -r "$PDO_FREETDS_INSTALLATION_DIR/$CRX_LIBDIR/libsybdb.so"; then
       AC_MSG_ERROR(Could not find $PDO_FREETDS_INSTALLATION_DIR/$CRX_LIBDIR/libsybdb.[a|so])
    fi

    CRX_ADD_INCLUDE($PDO_FREETDS_INCLUDE_DIR)
    CRX_ADD_LIBRARY_WITH_PATH(sybdb, $PDO_FREETDS_INSTALLATION_DIR/$CRX_LIBDIR, PDO_DBLIB_SHARED_LIBADD)
  fi

  CRX_CHECK_PDO_INCLUDES

  PDO_DBLIB_DEFS="-DPDO_DBLIB_FLAVOUR=\\\"freetds\\\""
  CRX_NEW_EXTENSION(pdo_dblib, pdo_dblib.c dblib_driver.c dblib_stmt.c, $ext_shared,,-I$pdo_cv_inc_path $PDO_DBLIB_DEFS -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1)
  AC_CHECK_LIB(dnet_stub, dnet_addr, [
    CRX_ADD_LIBRARY_WITH_PATH(dnet_stub,,PDO_DBLIB_SHARED_LIBADD)
  ])
  AC_DEFINE(HAVE_PDO_DBLIB,1,[ ])
  CRX_SUBST(PDO_DBLIB_SHARED_LIBADD)

  CRX_ADD_EXTENSION_DEP(pdo_dblib, pdo)
fi
