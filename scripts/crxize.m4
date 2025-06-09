dnl This file becomes configure.ac for self-contained extensions.

dnl Include external macro definitions before the AC_INIT to also remove
dnl comments starting with # and empty newlines from the included files.
m4_include([build/ax_check_compile_flag.m4])
m4_include([build/ax_gcc_func_attribute.m4])
m4_include([build/libtool.m4])
m4_include([build/crx_cxx_compile_stdcxx.m4])
m4_include([build/crx.m4])
m4_include([build/pkg.m4])

AC_PREREQ([2.68])
AC_INIT
AC_CONFIG_SRCDIR([config.m4])
AC_CONFIG_AUX_DIR([build])
AC_PRESERVE_HELP_ORDER

CRX_CONFIG_NICE(config.nice)

AC_DEFUN([CRX_EXT_BUILDDIR],[.])dnl
AC_DEFUN([CRX_EXT_DIR],[""])dnl
AC_DEFUN([CRX_EXT_SRCDIR],[$abs_srcdir])dnl
AC_DEFUN([CRX_ALWAYS_SHARED],[
  ext_output="yes, shared"
  ext_shared=yes
  test "[$]$1" = "no" && $1=yes
])dnl

test -z "$CFLAGS" && auto_cflags=1

abs_srcdir=`(cd $srcdir && pwd)`
abs_builddir=`pwd`

PKG_PROG_PKG_CONFIG
AC_PROG_CC([cc gcc])
CRX_DETECT_ICC
CRX_DETECT_SUNCC

dnl Support systems with system libraries in e.g. /usr/lib64.
CRX_ARG_WITH([libdir],
  [for system library directory],
  [AS_HELP_STRING([--with-libdir=NAME],
    [Look for libraries in .../NAME rather than .../lib])],
  [lib],
  [no])

CRX_RUNPATH_SWITCH
CRX_SHLIB_SUFFIX_NAMES

dnl Find crx-config script.
CRX_ARG_WITH([crx-config],,
  [AS_HELP_STRING([--with-crx-config=PATH],
    [Path to crx-config [crx-config]])],
  [crx-config],
  [no])

dnl For BC.
CRX_CONFIG=$CRX_CRX_CONFIG
prefix=`$CRX_CONFIG --prefix 2>/dev/null`
crxincludedir=`$CRX_CONFIG --include-dir 2>/dev/null`
INCLUDES=`$CRX_CONFIG --includes 2>/dev/null`
EXTENSION_DIR=`$CRX_CONFIG --extension-dir 2>/dev/null`
CRX_EXECUTABLE=`$CRX_CONFIG --crx-binary 2>/dev/null`

if test -z "$prefix"; then
  AC_MSG_ERROR([Cannot find crx-config. Please use --with-crx-config=PATH])
fi

crx_shtool=$srcdir/build/shtool
CRX_INIT_BUILD_SYSTEM

AC_MSG_CHECKING([for CRX prefix])
AC_MSG_RESULT([$prefix])
AC_MSG_CHECKING([for CRX includes])
AC_MSG_RESULT([$INCLUDES])
AC_MSG_CHECKING([for CRX extension directory])
AC_MSG_RESULT([$EXTENSION_DIR])
AC_MSG_CHECKING([for CRX installed headers prefix])
AC_MSG_RESULT([$crxincludedir])

dnl Checks for CRX_DEBUG / CREX_DEBUG / ZTS.
AC_MSG_CHECKING([if debug is enabled])
old_CPPFLAGS=$CPPFLAGS
CPPFLAGS="-I$crxincludedir"
AC_EGREP_CPP(crx_debug_is_enabled,[
#include <main/crx_config.h>
#if CREX_DEBUG
crx_debug_is_enabled
#endif
],[
  CRX_DEBUG=yes
],[
  CRX_DEBUG=no
])
CPPFLAGS=$old_CPPFLAGS
AC_MSG_RESULT([$CRX_DEBUG])

AC_MSG_CHECKING([if zts is enabled])
old_CPPFLAGS=$CPPFLAGS
CPPFLAGS="-I$crxincludedir"
AC_EGREP_CPP(crx_zts_is_enabled,[
#include <main/crx_config.h>
#if ZTS
crx_zts_is_enabled
#endif
],[
  CRX_THREAD_SAFETY=yes
],[
  CRX_THREAD_SAFETY=no
])
CPPFLAGS=$old_CPPFLAGS
AC_MSG_RESULT([$CRX_THREAD_SAFETY])

dnl Discard optimization flags when debugging is enabled.
if test "$CRX_DEBUG" = "yes"; then
  CRX_DEBUG=1
  CREX_DEBUG=yes
  changequote({,})
  CFLAGS=`echo "$CFLAGS" | $SED -e 's/-O[0-9s]*//g'`
  CXXFLAGS=`echo "$CXXFLAGS" | $SED -e 's/-O[0-9s]*//g'`
  changequote([,])
  dnl Add -O0 only if GCC or ICC is used.
  if test "$GCC" = "yes" || test "$ICC" = "yes"; then
    CFLAGS="$CFLAGS -O0"
    CXXFLAGS="$CXXFLAGS -g -O0"
  fi
  if test "$SUNCC" = "yes"; then
    if test -n "$auto_cflags"; then
      CFLAGS="-g"
      CXXFLAGS="-g"
    else
      CFLAGS="$CFLAGS -g"
      CXXFLAGS="$CFLAGS -g"
    fi
  fi
else
  CRX_DEBUG=0
  CREX_DEBUG=no
fi

dnl Always shared.
CRX_BUILD_SHARED

dnl Required programs.
CRX_PROG_AWK

sinclude(config.m4)

enable_static=no
enable_shared=yes

dnl Only allow AC_PROG_CXX and AC_PROG_CXXCPP if they are explicitly called (by
dnl CRX_REQUIRE_CXX). Otherwise AC_PROG_LIBTOOL fails if there is no working C++
dnl compiler.
AC_PROVIDE_IFELSE([CRX_REQUIRE_CXX], [], [
  undefine([AC_PROG_CXX])
  AC_DEFUN([AC_PROG_CXX], [])
  undefine([AC_PROG_CXXCPP])
  AC_DEFUN([AC_PROG_CXXCPP], [crx_prog_cxxcpp=disabled])
])
AC_PROG_LIBTOOL

all_targets='$(CRX_MODULES) $(CRX_CREX_EX)'
install_targets="install-modules install-headers"
crxlibdir="`pwd`/modules"
CPPFLAGS="$CPPFLAGS -DHAVE_CONFIG_H"
CFLAGS_CLEAN='$(CFLAGS) -D_GNU_SOURCE'
CXXFLAGS_CLEAN='$(CXXFLAGS)'

test "$prefix" = "NONE" && prefix="/usr/local"
test "$exec_prefix" = "NONE" && exec_prefix='$(prefix)'

if test "$cross_compiling" = yes ; then
  AC_MSG_CHECKING(for native build C compiler)
  AC_CHECK_PROGS(BUILD_CC, [gcc clang c99 c89 cc cl],none)
  AC_MSG_RESULT($BUILD_CC)
else
  BUILD_CC=$CC
fi

CRX_SUBST(CRX_MODULES)
CRX_SUBST(CRX_CREX_EX)

CRX_SUBST(all_targets)
CRX_SUBST(install_targets)

CRX_SUBST(prefix)
CRX_SUBST(exec_prefix)
CRX_SUBST(libdir)
CRX_SUBST(prefix)
CRX_SUBST(crxlibdir)
CRX_SUBST(crxincludedir)

CRX_SUBST(CC)
CRX_SUBST(CFLAGS)
CRX_SUBST(CFLAGS_CLEAN)
CRX_SUBST(CPP)
CRX_SUBST(CPPFLAGS)
CRX_SUBST(CXX)
CRX_SUBST(CXXFLAGS)
CRX_SUBST(CXXFLAGS_CLEAN)
CRX_SUBST(EXTENSION_DIR)
CRX_SUBST(CRX_EXECUTABLE)
CRX_SUBST(EXTRA_LDFLAGS)
CRX_SUBST(EXTRA_LIBS)
CRX_SUBST(INCLUDES)
CRX_SUBST(LFLAGS)
CRX_SUBST(LDFLAGS)
CRX_SUBST(SHARED_LIBTOOL)
CRX_SUBST(LIBTOOL)
CRX_SUBST(SHELL)
CRX_SUBST(INSTALL_HEADERS)
CRX_SUBST(BUILD_CC)

CRX_GEN_BUILD_DIRS
CRX_GEN_GLOBAL_MAKEFILE

test -d modules || $crx_shtool mkdir modules

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_COMMANDS_PRE([CRX_PATCH_CONFIG_HEADERS([config.h.in])])

AC_OUTPUT
