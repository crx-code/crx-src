AC_DEFUN([CRX_MBSTRING_ADD_SOURCES], [
  CRX_MBSTRING_SOURCES="$CRX_MBSTRING_SOURCES $1"
])

AC_DEFUN([CRX_MBSTRING_ADD_BASE_SOURCES], [
  CRX_MBSTRING_BASE_SOURCES="$CRX_MBSTRING_BASE_SOURCES $1"
])

AC_DEFUN([CRX_MBSTRING_ADD_BUILD_DIR], [
  CRX_MBSTRING_EXTRA_BUILD_DIRS="$CRX_MBSTRING_EXTRA_BUILD_DIRS $1"
])

AC_DEFUN([CRX_MBSTRING_ADD_INCLUDE], [
  CRX_MBSTRING_EXTRA_INCLUDES="$CRX_MBSTRING_EXTRA_INCLUDES $1"
])

AC_DEFUN([CRX_MBSTRING_ADD_CFLAG], [
  CRX_MBSTRING_CFLAGS="$CRX_MBSTRING_CFLAGS $1"
])

AC_DEFUN([CRX_MBSTRING_ADD_INSTALL_HEADERS], [
  CRX_MBSTRING_INSTALL_HEADERS="$CRX_MBSTRING_INSTALL_HEADERS $1"
])

AC_DEFUN([CRX_MBSTRING_EXTENSION], [
  CRX_NEW_EXTENSION(mbstring, $CRX_MBSTRING_BASE_SOURCES $CRX_MBSTRING_SOURCES, $ext_shared,, $CRX_MBSTRING_CFLAGS -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1)
  CRX_SUBST(MBSTRING_SHARED_LIBADD)

  for dir in $CRX_MBSTRING_EXTRA_BUILD_DIRS; do
    CRX_ADD_BUILD_DIR([$ext_builddir/$dir], 1)
  done

  for dir in $CRX_MBSTRING_EXTRA_INCLUDES; do
    CRX_ADD_INCLUDE([$ext_srcdir/$dir])
    CRX_ADD_INCLUDE([$ext_builddir/$dir])
  done

  out="crx_config.h"

  if test "$ext_shared" != "no" && test -f "$ext_builddir/config.h.in"; then
    out="$abs_builddir/config.h"
  fi

  cat > $ext_builddir/libmbfl/config.h <<EOF
#include "$out"
EOF

  CRX_MBSTRING_ADD_INSTALL_HEADERS([mbstring.h])
  CRX_INSTALL_HEADERS([ext/mbstring], [$CRX_MBSTRING_INSTALL_HEADERS])
])

AC_DEFUN([CRX_MBSTRING_SETUP_MBREGEX], [
  if test "$CRX_MBREGEX" = "yes"; then
    PKG_CHECK_MODULES([ONIG], [oniguruma])
    CRX_EVAL_LIBLINE($ONIG_LIBS, MBSTRING_SHARED_LIBADD)
    CRX_EVAL_INCLINE($ONIG_CFLAGS)

    save_old_LDFLAGS=$LDFLAGS
    CRX_EVAL_LIBLINE([$MBSTRING_SHARED_LIBADD], LDFLAGS)
    AC_MSG_CHECKING([if oniguruma has an invalid entry for KOI8 encoding])
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <oniguruma.h>
    ]], [[
return (int)(ONIG_ENCODING_KOI8 + 1);
    ]])], [
      AC_MSG_RESULT([no])
    ], [
      AC_MSG_RESULT([yes])
      AC_DEFINE([CRX_ONIG_BAD_KOI8_ENTRY], [1], [define to 1 if oniguruma has an invalid entry for KOI8 encoding])
    ])
    LDFLAGS=$save_old_LDFLAGS

    CRX_MBSTRING_ADD_CFLAG([-DONIG_ESCAPE_UCHAR_COLLISION=1])
    CRX_MBSTRING_ADD_CFLAG([-DUChar=OnigUChar])

    AC_DEFINE([HAVE_MBREGEX], 1, [whether to have multibyte regex support])

    CRX_MBSTRING_ADD_BASE_SOURCES([crx_mbregex.c])
    CRX_MBSTRING_ADD_INSTALL_HEADERS([crx_mbregex.h crx_onig_compat.h])
  fi
])

AC_DEFUN([CRX_MBSTRING_SETUP_LIBMBFL], [
  dnl
  dnl Bundled libmbfl is required and can not be disabled
  dnl
  CRX_MBSTRING_ADD_BUILD_DIR([libmbfl])
  CRX_MBSTRING_ADD_BUILD_DIR([libmbfl/mbfl])
  CRX_MBSTRING_ADD_BUILD_DIR([libmbfl/filters])
  CRX_MBSTRING_ADD_BUILD_DIR([libmbfl/nls])
  CRX_MBSTRING_ADD_INCLUDE([libmbfl])
  CRX_MBSTRING_ADD_INCLUDE([libmbfl/mbfl])

  CRX_MBSTRING_ADD_SOURCES([
    libmbfl/filters/html_entities.c
    libmbfl/filters/mbfilter_7bit.c
    libmbfl/filters/mbfilter_base64.c
    libmbfl/filters/mbfilter_cjk.c
    libmbfl/filters/mbfilter_htmlent.c
    libmbfl/filters/mbfilter_qprint.c
    libmbfl/filters/mbfilter_singlebyte.c
    libmbfl/filters/mbfilter_ucs2.c
    libmbfl/filters/mbfilter_ucs4.c
    libmbfl/filters/mbfilter_utf16.c
    libmbfl/filters/mbfilter_utf32.c
    libmbfl/filters/mbfilter_utf7.c
    libmbfl/filters/mbfilter_utf7imap.c
    libmbfl/filters/mbfilter_utf8.c
    libmbfl/filters/mbfilter_utf8_mobile.c
    libmbfl/filters/mbfilter_uuencode.c
    libmbfl/mbfl/mbfilter.c
    libmbfl/mbfl/mbfilter_8bit.c
    libmbfl/mbfl/mbfilter_pass.c
    libmbfl/mbfl/mbfilter_wchar.c
    libmbfl/mbfl/mbfl_convert.c
    libmbfl/mbfl/mbfl_encoding.c
    libmbfl/mbfl/mbfl_filter_output.c
    libmbfl/mbfl/mbfl_language.c
    libmbfl/mbfl/mbfl_memory_device.c
    libmbfl/mbfl/mbfl_string.c
    libmbfl/nls/nls_de.c
    libmbfl/nls/nls_en.c
    libmbfl/nls/nls_ja.c
    libmbfl/nls/nls_kr.c
    libmbfl/nls/nls_neutral.c
    libmbfl/nls/nls_ru.c
    libmbfl/nls/nls_uni.c
    libmbfl/nls/nls_zh.c
    libmbfl/nls/nls_hy.c
    libmbfl/nls/nls_tr.c
    libmbfl/nls/nls_ua.c
  ])
  CRX_MBSTRING_ADD_INSTALL_HEADERS([libmbfl/config.h libmbfl/mbfl/eaw_table.h libmbfl/mbfl/mbfilter.h libmbfl/mbfl/mbfilter_8bit.h libmbfl/mbfl/mbfilter_pass.h libmbfl/mbfl/mbfilter_wchar.h libmbfl/mbfl/mbfl_consts.h libmbfl/mbfl/mbfl_convert.h libmbfl/mbfl/mbfl_defs.h libmbfl/mbfl/mbfl_encoding.h libmbfl/mbfl/mbfl_filter_output.h libmbfl/mbfl/mbfl_language.h libmbfl/mbfl/mbfl_memory_device.h libmbfl/mbfl/mbfl_string.h])
])

dnl
dnl Main config
dnl

CRX_ARG_ENABLE([mbstring],
  [whether to enable multibyte string support],
  [AS_HELP_STRING([--enable-mbstring],
    [Enable multibyte string support])])

CRX_ARG_ENABLE([mbregex],
  [whether to enable multibyte regex support (requires oniguruma)],
  [AS_HELP_STRING([--disable-mbregex],
    [MBSTRING: Disable multibyte regex support])],
  [yes],
  [no])

if test "$CRX_MBSTRING" != "no"; then
  AC_DEFINE([HAVE_MBSTRING],1,[whether to have multibyte string support])

  CRX_MBSTRING_ADD_BASE_SOURCES([mbstring.c crx_unicode.c mb_gpc.c])

  if test "$CRX_MBREGEX" != "no"; then
    CRX_MBSTRING_SETUP_MBREGEX
  fi

  dnl libmbfl is required
  CRX_MBSTRING_SETUP_LIBMBFL
  CRX_MBSTRING_EXTENSION
fi
