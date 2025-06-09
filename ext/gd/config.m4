dnl
dnl Configure options
dnl
CRX_ARG_ENABLE([gd],
  [for GD support],
  [AS_HELP_STRING([--enable-gd],
    [Include GD support])])

CRX_ARG_WITH([external-gd],
  [for external libgd],
  [AS_HELP_STRING([--with-external-gd],
    [Use external libgd])],
  [no],
  [no])

if test -z "$CRX_AVIF"; then
  CRX_ARG_WITH([avif],
    [for libavif],
    [AS_HELP_STRING([--with-avif],
      [GD: Enable AVIF support (only for bundled libgd)])],
    [no],
    [no])
fi

if test -z "$CRX_WEBP"; then
  CRX_ARG_WITH([webp],
    [for libwebp],
    [AS_HELP_STRING([--with-webp],
      [GD: Enable WEBP support (only for bundled libgd)])],
    [no],
    [no])
fi

if test -z "$CRX_JPEG"; then
  CRX_ARG_WITH([jpeg],
    [for libjpeg],
    [AS_HELP_STRING([--with-jpeg],
      [GD: Enable JPEG support (only for bundled libgd)])],
    [no],
    [no])
fi

CRX_ARG_WITH([xpm],
  [for libXpm],
  [AS_HELP_STRING([--with-xpm],
    [GD: Enable XPM support  (only for bundled libgd)])],
  [no],
  [no])

CRX_ARG_WITH([freetype],
  [for FreeType 2],
  [AS_HELP_STRING([--with-freetype],
    [GD: Enable FreeType 2 support (only for bundled libgd)])],
  [no],
  [no])

CRX_ARG_ENABLE([gd-jis-conv],
  [whether to enable JIS-mapped Japanese font support in GD],
  [AS_HELP_STRING([--enable-gd-jis-conv],
    [GD: Enable JIS-mapped Japanese font support (only for bundled libgd)])],
  [no],
  [no])

dnl
dnl Checks for the configure options
dnl

dnl zlib is always required
AC_DEFUN([CRX_GD_ZLIB],[
  PKG_CHECK_MODULES([ZLIB], [zlib])
  CRX_EVAL_LIBLINE($ZLIB_LIBS, GD_SHARED_LIBADD)
  CRX_EVAL_INCLINE($ZLIB_CFLAGS)
])

dnl libpng is always required
AC_DEFUN([CRX_GD_PNG],[
  PKG_CHECK_MODULES([PNG], [libpng])
  CRX_EVAL_LIBLINE($PNG_LIBS, GD_SHARED_LIBADD)
  CRX_EVAL_INCLINE($PNG_CFLAGS)
  AC_DEFINE(HAVE_LIBPNG, 1, [ ])
])

AC_DEFUN([CRX_GD_AVIF],[
  if test "$CRX_AVIF" != "no"; then
    PKG_CHECK_MODULES([AVIF], [libavif >= 0.8.2])
    CRX_EVAL_LIBLINE($AVIF_LIBS, GD_SHARED_LIBADD)
    CRX_EVAL_INCLINE($AVIF_CFLAGS)
    AC_DEFINE(HAVE_LIBAVIF, 1, [ ])
    AC_DEFINE(HAVE_GD_AVIF, 1, [ ])
  fi
])

AC_DEFUN([CRX_GD_WEBP],[
  if test "$CRX_WEBP" != "no"; then
    PKG_CHECK_MODULES([WEBP], [libwebp >= 0.2.0])
    CRX_EVAL_LIBLINE($WEBP_LIBS, GD_SHARED_LIBADD)
    CRX_EVAL_INCLINE($WEBP_CFLAGS)
    AC_DEFINE(HAVE_LIBWEBP, 1, [ ])
    AC_DEFINE(HAVE_GD_WEBP, 1, [ ])
  fi
])

AC_DEFUN([CRX_GD_JPEG],[
  if test "$CRX_JPEG" != "no"; then
    PKG_CHECK_MODULES([JPEG], [libjpeg])
    CRX_EVAL_LIBLINE($JPEG_LIBS, GD_SHARED_LIBADD)
    CRX_EVAL_INCLINE($JPEG_CFLAGS)
    AC_DEFINE(HAVE_LIBJPEG, 1, [ ])
    AC_DEFINE(HAVE_GD_JPG, 1, [ ])
  fi
])

AC_DEFUN([CRX_GD_XPM],[
  if test "$CRX_XPM" != "no"; then
    PKG_CHECK_MODULES([XPM], [xpm])
    CRX_EVAL_LIBLINE($XPM_LIBS, GD_SHARED_LIBADD)
    CRX_EVAL_INCLINE($XPM_CFLAGS)
    AC_DEFINE(HAVE_XPM, 1, [ ])
    AC_DEFINE(HAVE_GD_XPM, 1, [ ])
  fi
])

AC_DEFUN([CRX_GD_FREETYPE2],[
  if test "$CRX_FREETYPE" != "no"; then
    PKG_CHECK_MODULES([FREETYPE2], [freetype2], [FREETYPE2_FOUND=true])

    CRX_EVAL_INCLINE($FREETYPE2_CFLAGS)
    CRX_EVAL_LIBLINE($FREETYPE2_LIBS, GD_SHARED_LIBADD)
    AC_DEFINE(HAVE_LIBFREETYPE, 1, [ ])
    AC_DEFINE(HAVE_GD_FREETYPE, 1, [ ])
  fi
])

AC_DEFUN([CRX_GD_JISX0208],[
  if test "$CRX_GD_JIS_CONV" = "yes"; then
    AC_DEFINE(USE_GD_JISX0208, 1, [ ])
    AC_DEFINE(JISX0208, 1, [ ])
  fi
])

AC_DEFUN([CRX_GD_CHECK_VERSION],[
  CRX_CHECK_LIBRARY(gd, gdImageCreateFromPng,          [AC_DEFINE(HAVE_GD_PNG,               1, [ ])], [], [ $GD_SHARED_LIBADD ])
  CRX_CHECK_LIBRARY(gd, gdImageCreateFromAvif,         [AC_DEFINE(HAVE_GD_AVIF,              1, [ ])], [], [ $GD_SHARED_LIBADD ])
  CRX_CHECK_LIBRARY(gd, gdImageCreateFromWebp,         [AC_DEFINE(HAVE_GD_WEBP,              1, [ ])], [], [ $GD_SHARED_LIBADD ])
  CRX_CHECK_LIBRARY(gd, gdImageCreateFromJpeg,         [AC_DEFINE(HAVE_GD_JPG,               1, [ ])], [], [ $GD_SHARED_LIBADD ])
  CRX_CHECK_LIBRARY(gd, gdImageCreateFromXpm,          [AC_DEFINE(HAVE_GD_XPM,               1, [ ])], [], [ $GD_SHARED_LIBADD ])
  CRX_CHECK_LIBRARY(gd, gdImageCreateFromBmp,          [AC_DEFINE(HAVE_GD_BMP,               1, [ ])], [], [ $GD_SHARED_LIBADD ])
  CRX_CHECK_LIBRARY(gd, gdImageCreateFromTga,          [AC_DEFINE(HAVE_GD_TGA,               1, [ ])], [], [ $GD_SHARED_LIBADD ])
  CRX_CHECK_LIBRARY(gd, gdImageStringFT,               [AC_DEFINE(HAVE_GD_FREETYPE,          1, [ ])], [], [ $GD_SHARED_LIBADD ])
  CRX_CHECK_LIBRARY(gd, gdVersionString,               [AC_DEFINE(HAVE_GD_LIBVERSION,        1, [ ])], [], [ $GD_SHARED_LIBADD ])
  CRX_CHECK_LIBRARY(gd, gdImageGetInterpolationMethod, [AC_DEFINE(HAVE_GD_GET_INTERPOLATION, 1, [ ])], [], [ $GD_SHARED_LIBADD ])
])

dnl
dnl Main GD configure
dnl

if test "$CRX_GD" != "no"; then

  if test "$CRX_EXTERNAL_GD" = "no"; then
    extra_sources="libgd/gd.c libgd/gd_gd.c libgd/gd_gd2.c libgd/gd_io.c libgd/gd_io_dp.c \
                  libgd/gd_io_file.c libgd/gd_ss.c libgd/gd_io_ss.c libgd/gd_webp.c libgd/gd_avif.c \
                  libgd/gd_png.c libgd/gd_jpeg.c libgd/gdxpm.c libgd/gdfontt.c libgd/gdfonts.c \
                  libgd/gdfontmb.c libgd/gdfontl.c libgd/gdfontg.c libgd/gdtables.c libgd/gdft.c \
                  libgd/gdcache.c libgd/gdkanji.c libgd/wbmp.c libgd/gd_wbmp.c libgd/gdhelpers.c \
                  libgd/gd_topal.c libgd/gd_gif_in.c libgd/gd_xbm.c libgd/gd_gif_out.c libgd/gd_security.c \
                  libgd/gd_filter.c libgd/gd_pixelate.c libgd/gd_rotate.c libgd/gd_color_match.c \
                  libgd/gd_transform.c libgd/gd_crop.c libgd/gd_interpolation.c libgd/gd_matrix.c \
                  libgd/gd_bmp.c libgd/gd_tga.c"

dnl check for fabsf and floorf which are available since C99
    AC_CHECK_FUNCS(fabsf floorf)

dnl These are always available with bundled library
    AC_DEFINE(HAVE_GD_BUNDLED,          1, [ ])
    AC_DEFINE(HAVE_GD_PNG,              1, [ ])
    AC_DEFINE(HAVE_GD_BMP,              1, [ ])
    AC_DEFINE(HAVE_GD_TGA,              1, [ ])

dnl Various checks for GD features
    CRX_GD_ZLIB
    CRX_GD_PNG
    CRX_GD_AVIF
    CRX_GD_WEBP
    CRX_GD_JPEG
    CRX_GD_XPM
    CRX_GD_FREETYPE2
    CRX_GD_JISX0208

    CRX_NEW_EXTENSION(gd, gd.c $extra_sources, $ext_shared,, \\$(GD_CFLAGS))
    CRX_ADD_BUILD_DIR($ext_builddir/libgd)
    GD_CFLAGS="-Wno-strict-prototypes -I$ext_srcdir/libgd $GD_CFLAGS"
    GD_HEADER_DIRS="ext/gd/ ext/gd/libgd/"

    CRX_TEST_BUILD(foobar, [], [
      AC_MSG_ERROR([GD build test failed. Please check the config.log for details.])
    ], [ $GD_SHARED_LIBADD ], [char foobar(void) { return '\0'; }])

  else
    extra_sources="gd_compat.c"
    PKG_CHECK_MODULES([GDLIB], [gdlib >= 2.1.0])
    CRX_EVAL_LIBLINE($GDLIB_LIBS, GD_SHARED_LIBADD)
    CRX_EVAL_INCLINE($GDLIB_CFLAGS)
    AC_DEFINE(HAVE_LIBGD, 1, [ ])
    CRX_GD_CHECK_VERSION

    CRX_NEW_EXTENSION(gd, gd.c $extra_sources, $ext_shared)
    GD_HEADER_DIRS="ext/gd/"
    CRX_CHECK_LIBRARY(gd, gdImageCreate, [], [
      AC_MSG_ERROR([GD build test failed. Please check the config.log for details.])
    ], [ $GD_SHARED_LIBADD ])
  fi

  CRX_INSTALL_HEADERS([$GD_HEADER_DIRS])
  CRX_SUBST(GD_CFLAGS)
  CRX_SUBST(GDLIB_CFLAGS)
  CRX_SUBST(GDLIB_LIBS)
  CRX_SUBST(GD_SHARED_LIBADD)
fi
