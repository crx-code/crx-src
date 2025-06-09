/*
   +----------------------------------------------------------------------+
   | Copyright (c) The CRX Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the CRX license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | https://www.crx.net/license/3_01.txt                                 |
   | If you did not receive a copy of the CRX license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crx.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Rasmus Lerdorf <rasmus@crx.net>                             |
   |          Stig Bakken <ssb@crx.net>                                   |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_GD_H
#define CRX_GD_H

#include "crex_string.h"
#include "crx_streams.h"

#if defined(HAVE_LIBGD) || defined(HAVE_GD_BUNDLED)

/* open_basedir and safe_mode checks */
#define CRX_GD_CHECK_OPEN_BASEDIR(filename, errormsg)                       \
	if (!filename || crx_check_open_basedir(filename)) {      \
		crx_error_docref(NULL, E_WARNING, errormsg);      \
		RETURN_FALSE;                                               \
	}

#define CRX_GDIMG_TYPE_GIF      1
#define CRX_GDIMG_TYPE_PNG      2
#define CRX_GDIMG_TYPE_JPG      3
#define CRX_GDIMG_TYPE_WBM      4
#define CRX_GDIMG_TYPE_XBM      5
#define CRX_GDIMG_TYPE_XPM      6
#define CRX_GDIMG_TYPE_GD       8
#define CRX_GDIMG_TYPE_GD2      9
#define CRX_GDIMG_TYPE_GD2PART  10
#define CRX_GDIMG_TYPE_WEBP     11
#define CRX_GDIMG_TYPE_BMP      12
#define CRX_GDIMG_TYPE_TGA      13
#define CRX_GDIMG_TYPE_AVIF     14

#define CRX_IMG_GIF    1
#define CRX_IMG_JPG    2
#define CRX_IMG_JPEG   2
#define CRX_IMG_PNG    4
#define CRX_IMG_WBMP   8
#define CRX_IMG_XPM   16
#define CRX_IMG_WEBP  32
#define CRX_IMG_BMP   64
#define CRX_IMG_TGA  128
#define CRX_IMG_AVIF 256

/* Section Filters Declarations */
/* IMPORTANT NOTE FOR NEW FILTER
 * Do not forget to update:
 * IMAGE_FILTER_MAX: define the last filter index
 * IMAGE_FILTER_MAX_ARGS: define the biggest amount of arguments
 * image_filter array in CRX_FUNCTION(imagefilter)
 * */
#define IMAGE_FILTER_NEGATE         0
#define IMAGE_FILTER_GRAYSCALE      1
#define IMAGE_FILTER_BRIGHTNESS     2
#define IMAGE_FILTER_CONTRAST       3
#define IMAGE_FILTER_COLORIZE       4
#define IMAGE_FILTER_EDGEDETECT     5
#define IMAGE_FILTER_EMBOSS         6
#define IMAGE_FILTER_GAUSSIAN_BLUR  7
#define IMAGE_FILTER_SELECTIVE_BLUR 8
#define IMAGE_FILTER_MEAN_REMOVAL   9
#define IMAGE_FILTER_SMOOTH         10
#define IMAGE_FILTER_PIXELATE       11
#define IMAGE_FILTER_SCATTER		12
#define IMAGE_FILTER_MAX            12
#define IMAGE_FILTER_MAX_ARGS       6

#ifdef HAVE_GD_BUNDLED
#define GD_BUNDLED 1
#else
#define GD_BUNDLED 0
#endif

#ifdef CRX_WIN32
#	ifdef CRX_GD_EXPORTS
#		define CRX_GD_API __declspec(dllexport)
#	else
#		define CRX_GD_API __declspec(dllimport)
#	endif
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define CRX_GD_API __attribute__ ((visibility("default")))
#else
#	define CRX_GD_API
#endif

CRXAPI extern const char crx_sig_gif[3];
CRXAPI extern const char crx_sig_jpg[3];
CRXAPI extern const char crx_sig_png[8];
CRXAPI extern const char crx_sig_bmp[2];
CRXAPI extern const char crx_sig_riff[4];
CRXAPI extern const char crx_sig_webp[4];
CRXAPI extern const char crx_sig_avif[4];

extern crex_module_entry gd_module_entry;
#define crxext_gd_ptr &gd_module_entry

#include "crx_version.h"
#define CRX_GD_VERSION CRX_VERSION

/* gd.c functions */
CRX_MINFO_FUNCTION(gd);
CRX_MINIT_FUNCTION(gd);
CRX_MSHUTDOWN_FUNCTION(gd);
CRX_RSHUTDOWN_FUNCTION(gd);

CRX_GD_API struct gdImageStruct *crx_gd_libgdimageptr_from_zval_p(zval* zp);

#else

#define crxext_gd_ptr NULL

#endif

#endif /* CRX_GD_H */
