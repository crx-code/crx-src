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
   |          Jim Winstead <jimw@crx.net>                                 |
   +----------------------------------------------------------------------+
 */

/* gd 1.2 is copyright 1994, 1995, Quest Protein Database Center,
   Cold Spring Harbor Labs. */

/* Note that there is no code from the gd package in this file */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "crx_ini.h"
#include "ext/standard/head.h"
#include <math.h>
#include "SAPI.h"
#include "crx_gd.h"
#include "ext/standard/crx_image.h"
#include "ext/standard/info.h"
#include "crx_open_temporary_file.h"
#include "crx_memory_streams.h"
#include "crex_object_handlers.h"

#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef CRX_WIN32
# include <io.h>
# include <fcntl.h>
# include <windows.h>
# include <Winuser.h>
# include <Wingdi.h>
#endif

#if defined(HAVE_GD_XPM) && defined(HAVE_GD_BUNDLED)
# include <X11/xpm.h>
#endif

#include "gd_compat.h"

#ifdef HAVE_GD_BUNDLED
# include "libgd/gd.h"
# include "libgd/gd_errors.h"
# include "libgd/gdfontt.h"  /* 1 Tiny font */
# include "libgd/gdfonts.h"  /* 2 Small font */
# include "libgd/gdfontmb.h" /* 3 Medium bold font */
# include "libgd/gdfontl.h"  /* 4 Large font */
# include "libgd/gdfontg.h"  /* 5 Giant font */
#else
# include <gd.h>
# include <gd_errors.h>
# include <gdfontt.h>  /* 1 Tiny font */
# include <gdfonts.h>  /* 2 Small font */
# include <gdfontmb.h> /* 3 Medium bold font */
# include <gdfontl.h>  /* 4 Large font */
# include <gdfontg.h>  /* 5 Giant font */
#endif

#if defined(HAVE_GD_FREETYPE) && defined(HAVE_GD_BUNDLED)
# include <ft2build.h>
# include FT_FREETYPE_H
#endif

#if defined(HAVE_GD_XPM) && defined(HAVE_GD_BUNDLED)
# include "X11/xpm.h"
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* don't used libgd constants, not used, so going to be removed */
#define CRX_GD_FLIP_HORIZONTAL 1
#define CRX_GD_FLIP_VERTICAL   2
#define CRX_GD_FLIP_BOTH       3

#ifdef HAVE_GD_FREETYPE
static void crx_imagettftext_common(INTERNAL_FUNCTION_PARAMETERS, int);
#endif

#include "gd_arginfo.h"

/* as it is not really public, duplicate declaration here to avoid
   pointless warnings */
int overflow2(int a, int b);

static void crx_image_filter_negate(INTERNAL_FUNCTION_PARAMETERS);
static void crx_image_filter_grayscale(INTERNAL_FUNCTION_PARAMETERS);
static void crx_image_filter_brightness(INTERNAL_FUNCTION_PARAMETERS);
static void crx_image_filter_contrast(INTERNAL_FUNCTION_PARAMETERS);
static void crx_image_filter_colorize(INTERNAL_FUNCTION_PARAMETERS);
static void crx_image_filter_edgedetect(INTERNAL_FUNCTION_PARAMETERS);
static void crx_image_filter_emboss(INTERNAL_FUNCTION_PARAMETERS);
static void crx_image_filter_gaussian_blur(INTERNAL_FUNCTION_PARAMETERS);
static void crx_image_filter_selective_blur(INTERNAL_FUNCTION_PARAMETERS);
static void crx_image_filter_mean_removal(INTERNAL_FUNCTION_PARAMETERS);
static void crx_image_filter_smooth(INTERNAL_FUNCTION_PARAMETERS);
static void crx_image_filter_pixelate(INTERNAL_FUNCTION_PARAMETERS);
static void crx_image_filter_scatter(INTERNAL_FUNCTION_PARAMETERS);

/* End Section filters declarations */
static gdImagePtr _crx_image_create_from_string(crex_string *Data, char *tn, gdImagePtr (*ioctx_func_p)(gdIOCtxPtr));
static void _crx_image_create_from(INTERNAL_FUNCTION_PARAMETERS, int image_type, char *tn, gdImagePtr (*func_p)(FILE *), gdImagePtr (*ioctx_func_p)(gdIOCtxPtr));
static void _crx_image_output(INTERNAL_FUNCTION_PARAMETERS, int image_type, char *tn);
static gdIOCtx *create_stream_context_from_zval(zval *to_zval);
static gdIOCtx *create_stream_context(crx_stream *stream, int close_stream);
static gdIOCtx *create_output_context(void);
static int _crx_image_type(crex_string *data);

/* output streaming (formerly gd_ctx.c) */
static void _crx_image_output_ctx(INTERNAL_FUNCTION_PARAMETERS, int image_type, char *tn);

/*********************************************************
 *
 * GD Object Representation
 *
 ********************************************************/

crex_class_entry *gd_image_ce;

typedef struct _gd_ext_image_object {
	gdImagePtr image;
	crex_object std;
} crx_gd_image_object;

static crex_object_handlers crx_gd_image_object_handlers;

static crex_function *crx_gd_image_object_get_constructor(crex_object *object)
{
	crex_throw_error(NULL, "You cannot initialize a GdImage object except through helper functions");
	return NULL;
}

/**
 * Returns the underlying crx_gd_image_object from a crex_object
 */

static crex_always_inline crx_gd_image_object* crx_gd_exgdimage_from_zobj_p(crex_object* obj)
{
	return (crx_gd_image_object *) ((char *) (obj) - XtOffsetOf(crx_gd_image_object, std));
}

/**
 * Converts an extension GdImage instance contained within a zval into the gdImagePtr
 * for use with library APIs
 */
CRX_GD_API gdImagePtr crx_gd_libgdimageptr_from_zval_p(zval* zp)
{
	return crx_gd_exgdimage_from_zobj_p(C_OBJ_P(zp))->image;
}


crex_object *crx_gd_image_object_create(crex_class_entry *class_type)
{
	size_t block_len = sizeof(crx_gd_image_object) + crex_object_properties_size(class_type);
	crx_gd_image_object *intern = emalloc(block_len);
	memset(intern, 0, block_len);

	crex_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	return &intern->std;
}

static void crx_gd_image_object_free(crex_object *intern)
{
	crx_gd_image_object *img_obj_ptr = crx_gd_exgdimage_from_zobj_p(intern);
	if (img_obj_ptr->image) {
		gdImageDestroy(img_obj_ptr->image);
	}
	crex_object_std_dtor(intern);
}

/**
 * Creates a new GdImage object wrapping the gdImagePtr and attaches it
 * to the zval (usually return_value).
 *
 * This function must only be called once per valid gdImagePtr
 */
void crx_gd_assign_libgdimageptr_as_extgdimage(zval *val, gdImagePtr image)
{
	object_init_ex(val, gd_image_ce);
	crx_gd_exgdimage_from_zobj_p(C_OBJ_P(val))->image = image;
}

static void crx_gd_object_minit_helper(void)
{
	gd_image_ce = register_class_GdImage();
	gd_image_ce->create_object = crx_gd_image_object_create;
	gd_image_ce->default_object_handlers = &crx_gd_image_object_handlers;

	/* setting up the object handlers for the GdImage class */
	memcpy(&crx_gd_image_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	crx_gd_image_object_handlers.clone_obj = NULL;
	crx_gd_image_object_handlers.free_obj = crx_gd_image_object_free;
	crx_gd_image_object_handlers.get_constructor = crx_gd_image_object_get_constructor;
	crx_gd_image_object_handlers.compare = crex_objects_not_comparable;
	crx_gd_image_object_handlers.offset = XtOffsetOf(crx_gd_image_object, std);
}

static crex_class_entry *gd_font_ce = NULL;
static crex_object_handlers crx_gd_font_object_handlers;

typedef struct _crx_gd_font_object {
	gdFontPtr font;
	crex_object std;
} crx_gd_font_object;

static crx_gd_font_object *crx_gd_font_object_from_crex_object(crex_object *zobj)
{
	return ((crx_gd_font_object*)(zobj + 1)) - 1;
}

static crex_object *crx_gd_font_object_to_crex_object(crx_gd_font_object *obj)
{
	return ((crex_object*)(obj + 1)) - 1;
}

static crex_object *crx_gd_font_object_create(crex_class_entry *ce)
{
	crx_gd_font_object *obj = crex_object_alloc(sizeof(crx_gd_font_object), ce);
	crex_object *zobj = crx_gd_font_object_to_crex_object(obj);

	obj->font = NULL;
	crex_object_std_init(zobj, ce);
	object_properties_init(zobj, ce);
	zobj->handlers = &crx_gd_font_object_handlers;

	return zobj;
}

static void crx_gd_font_object_free(crex_object *zobj)
{
	crx_gd_font_object *obj = crx_gd_font_object_from_crex_object(zobj);

	if (obj->font) {
		if (obj->font->data) {
			efree(obj->font->data);
		}
		efree(obj->font);
		obj->font = NULL;
	}

	crex_object_std_dtor(zobj);
}

static crex_function *crx_gd_font_object_get_constructor(crex_object *object)
{
	crex_throw_error(NULL, "You cannot initialize a GdFont object except through helper functions");
	return NULL;
}

static void crx_gd_font_minit_helper(void)
{
	gd_font_ce = register_class_GdFont();
	gd_font_ce->create_object = crx_gd_font_object_create;

	/* setting up the object handlers for the GdFont class */
	memcpy(&crx_gd_font_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	crx_gd_font_object_handlers.clone_obj = NULL;
	crx_gd_font_object_handlers.free_obj = crx_gd_font_object_free;
	crx_gd_font_object_handlers.get_constructor = crx_gd_font_object_get_constructor;
	crx_gd_font_object_handlers.offset = XtOffsetOf(crx_gd_font_object, std);
}

/*********************************************************
 *
 * Extension Implementation
 *
 ********************************************************/

crex_module_entry gd_module_entry = {
	STANDARD_MODULE_HEADER,
	"gd",
	ext_functions,
	CRX_MINIT(gd),
	CRX_MSHUTDOWN(gd),
	NULL,
	CRX_RSHUTDOWN(gd),
	CRX_MINFO(gd),
	CRX_GD_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_GD
CREX_GET_MODULE(gd)
#endif

/* {{{ CRX_INI_BEGIN */
CRX_INI_BEGIN()
	CRX_INI_ENTRY_EX("gd.jpeg_ignore_warning", "1", CRX_INI_ALL, NULL, crex_ini_boolean_displayer_cb)
CRX_INI_END()
/* }}} */

/* {{{ crx_gd_error_method */
void crx_gd_error_method(int type, const char *format, va_list args)
{
	switch (type) {
#ifndef CRX_WIN32
		case GD_DEBUG:
		case GD_INFO:
#endif
		case GD_NOTICE:
			type = E_NOTICE;
			break;
		case GD_WARNING:
			type = E_WARNING;
			break;
		default:
			type = E_ERROR;
	}
	crx_verror(NULL, "", type, format, args);
}
/* }}} */

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(gd)
{
	crx_gd_object_minit_helper();
	crx_gd_font_minit_helper();

#if defined(HAVE_GD_FREETYPE) && defined(HAVE_GD_BUNDLED)
	gdFontCacheMutexSetup();
#endif
	gdSetErrorMethod(crx_gd_error_method);

	REGISTER_INI_ENTRIES();

	register_gd_symbols(module_number);

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION */
CRX_MSHUTDOWN_FUNCTION(gd)
{
#if defined(HAVE_GD_FREETYPE) && defined(HAVE_GD_BUNDLED)
	gdFontCacheMutexShutdown();
#endif
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_RSHUTDOWN_FUNCTION */
CRX_RSHUTDOWN_FUNCTION(gd)
{
#ifdef HAVE_GD_FREETYPE
	gdFontCacheShutdown();
#endif
	return SUCCESS;
}
/* }}} */

#ifdef HAVE_GD_BUNDLED
#define CRX_GD_VERSION_STRING "bundled (2.1.0 compatible)"
#else
# define CRX_GD_VERSION_STRING GD_VERSION_STRING
#endif

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(gd)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "GD Support", "enabled");

	/* need to use a CRXAPI function here because it is external module in windows */

#ifdef HAVE_GD_BUNDLED
	crx_info_print_table_row(2, "GD Version", CRX_GD_VERSION_STRING);
#else
	crx_info_print_table_row(2, "GD headers Version", CRX_GD_VERSION_STRING);
#ifdef HAVE_GD_LIBVERSION
	crx_info_print_table_row(2, "GD library Version", gdVersionString());
#endif
#endif

#ifdef HAVE_GD_FREETYPE
	crx_info_print_table_row(2, "FreeType Support", "enabled");
	crx_info_print_table_row(2, "FreeType Linkage", "with freetype");
#ifdef HAVE_GD_BUNDLED
	{
		char tmp[256];

#ifdef FREETYPE_PATCH
		snprintf(tmp, sizeof(tmp), "%d.%d.%d", FREETYPE_MAJOR, FREETYPE_MINOR, FREETYPE_PATCH);
#elif defined(FREETYPE_MAJOR)
		snprintf(tmp, sizeof(tmp), "%d.%d", FREETYPE_MAJOR, FREETYPE_MINOR);
#else
		snprintf(tmp, sizeof(tmp), "1.x");
#endif
		crx_info_print_table_row(2, "FreeType Version", tmp);
	}
#endif
#endif

	crx_info_print_table_row(2, "GIF Read Support", "enabled");
	crx_info_print_table_row(2, "GIF Create Support", "enabled");

#ifdef HAVE_GD_JPG
	{
		crx_info_print_table_row(2, "JPEG Support", "enabled");
#ifdef HAVE_GD_BUNDLED
		crx_info_print_table_row(2, "libJPEG Version", gdJpegGetVersionString());
#endif
	}
#endif

#ifdef HAVE_GD_PNG
	crx_info_print_table_row(2, "PNG Support", "enabled");
#ifdef HAVE_GD_BUNDLED
	crx_info_print_table_row(2, "libPNG Version", gdPngGetVersionString());
#endif
#endif
	crx_info_print_table_row(2, "WBMP Support", "enabled");
#ifdef HAVE_GD_XPM
	crx_info_print_table_row(2, "XPM Support", "enabled");
#ifdef HAVE_GD_BUNDLED
	{
		char tmp[12];
		snprintf(tmp, sizeof(tmp), "%d", XpmLibraryVersion());
		crx_info_print_table_row(2, "libXpm Version", tmp);
	}
#endif
#endif
	crx_info_print_table_row(2, "XBM Support", "enabled");
#ifdef USE_GD_JISX0208
	crx_info_print_table_row(2, "JIS-mapped Japanese Font Support", "enabled");
#endif
#ifdef HAVE_GD_WEBP
	crx_info_print_table_row(2, "WebP Support", "enabled");
#endif
#ifdef HAVE_GD_BMP
	crx_info_print_table_row(2, "BMP Support", "enabled");
#endif
#ifdef HAVE_GD_AVIF
	crx_info_print_table_row(2, "AVIF Support", "enabled");
#endif
#ifdef HAVE_GD_TGA
	crx_info_print_table_row(2, "TGA Read Support", "enabled");
#endif
	crx_info_print_table_end();
	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ */
CRX_FUNCTION(gd_info)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	array_init(return_value);

	add_assoc_string(return_value, "GD Version", CRX_GD_VERSION_STRING);

#ifdef HAVE_GD_FREETYPE
	add_assoc_bool(return_value, "FreeType Support", 1);
	add_assoc_string(return_value, "FreeType Linkage", "with freetype");
#else
	add_assoc_bool(return_value, "FreeType Support", 0);
#endif
	add_assoc_bool(return_value, "GIF Read Support", 1);
	add_assoc_bool(return_value, "GIF Create Support", 1);
#ifdef HAVE_GD_JPG
	add_assoc_bool(return_value, "JPEG Support", 1);
#else
	add_assoc_bool(return_value, "JPEG Support", 0);
#endif
#ifdef HAVE_GD_PNG
	add_assoc_bool(return_value, "PNG Support", 1);
#else
	add_assoc_bool(return_value, "PNG Support", 0);
#endif
	add_assoc_bool(return_value, "WBMP Support", 1);
#ifdef HAVE_GD_XPM
	add_assoc_bool(return_value, "XPM Support", 1);
#else
	add_assoc_bool(return_value, "XPM Support", 0);
#endif
	add_assoc_bool(return_value, "XBM Support", 1);
#ifdef HAVE_GD_WEBP
	add_assoc_bool(return_value, "WebP Support", 1);
#else
	add_assoc_bool(return_value, "WebP Support", 0);
#endif
#ifdef HAVE_GD_BMP
	add_assoc_bool(return_value, "BMP Support", 1);
#else
	add_assoc_bool(return_value, "BMP Support", 0);
#endif
#ifdef HAVE_GD_AVIF
	add_assoc_bool(return_value, "AVIF Support", 1);
#else
	add_assoc_bool(return_value, "AVIF Support", 0);
#endif
#ifdef HAVE_GD_TGA
	add_assoc_bool(return_value, "TGA Read Support", 1);
#else
	add_assoc_bool(return_value, "TGA Read Support", 0);
#endif
#ifdef USE_GD_JISX0208
	add_assoc_bool(return_value, "JIS-mapped Japanese Font Support", 1);
#else
	add_assoc_bool(return_value, "JIS-mapped Japanese Font Support", 0);
#endif
}
/* }}} */

#define FLIPWORD(a) (((a & 0xff000000) >> 24) | ((a & 0x00ff0000) >> 8) | ((a & 0x0000ff00) << 8) | ((a & 0x000000ff) << 24))

/* {{{ Load a new font */
CRX_FUNCTION(imageloadfont)
{
	crex_string *file;
	int hdr_size = sizeof(gdFont) - sizeof(char *);
	int body_size, n = 0, b, i, body_size_check;
	gdFontPtr font;
	crx_stream *stream;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "P", &file) == FAILURE) {
		RETURN_THROWS();
	}

	stream = crx_stream_open_wrapper(ZSTR_VAL(file), "rb", IGNORE_PATH | REPORT_ERRORS, NULL);
	if (stream == NULL) {
		RETURN_FALSE;
	}

	/* Only supports a architecture-dependent binary dump format
	 * at the moment.
	 * The file format is like this on machines with 32-byte integers:
	 *
	 * byte 0-3:   (int) number of characters in the font
	 * byte 4-7:   (int) value of first character in the font (often 32, space)
	 * byte 8-11:  (int) pixel width of each character
	 * byte 12-15: (int) pixel height of each character
	 * bytes 16-:  (char) array with character data, one byte per pixel
	 *                    in each character, for a total of
	 *                    (nchars*width*height) bytes.
	 */
	font = (gdFontPtr) emalloc(sizeof(gdFont));
	b = 0;
	while (b < hdr_size && (n = crx_stream_read(stream, (char*)&font[b], hdr_size - b)) > 0) {
		b += n;
	}

	if (n <= 0) {
		efree(font);
		if (crx_stream_eof(stream)) {
			crx_error_docref(NULL, E_WARNING, "End of file while reading header");
		} else {
			crx_error_docref(NULL, E_WARNING, "Error while reading header");
		}
		crx_stream_close(stream);
		RETURN_FALSE;
	}
	i = crx_stream_tell(stream);
	crx_stream_seek(stream, 0, SEEK_END);
	body_size_check = crx_stream_tell(stream) - hdr_size;
	crx_stream_seek(stream, i, SEEK_SET);

	if (overflow2(font->nchars, font->h) || overflow2(font->nchars * font->h, font->w )) {
		crx_error_docref(NULL, E_WARNING, "Error reading font, invalid font header");
		efree(font);
		crx_stream_close(stream);
		RETURN_FALSE;
	}

	body_size = font->w * font->h * font->nchars;
	if (body_size != body_size_check) {
		font->w = FLIPWORD(font->w);
		font->h = FLIPWORD(font->h);
		font->nchars = FLIPWORD(font->nchars);
		if (overflow2(font->nchars, font->h) || overflow2(font->nchars * font->h, font->w )) {
			crx_error_docref(NULL, E_WARNING, "Error reading font, invalid font header");
			efree(font);
			crx_stream_close(stream);
			RETURN_FALSE;
		}
		body_size = font->w * font->h * font->nchars;
	}

	if (body_size != body_size_check) {
		crx_error_docref(NULL, E_WARNING, "Error reading font");
		efree(font);
		crx_stream_close(stream);
		RETURN_FALSE;
	}

	CREX_ASSERT(body_size > 0);
	font->data = emalloc(body_size);
	b = 0;
	while (b < body_size && (n = crx_stream_read(stream, &font->data[b], body_size - b)) > 0) {
		b += n;
	}

	if (n <= 0) {
		efree(font->data);
		efree(font);
		if (crx_stream_eof(stream)) {
			crx_error_docref(NULL, E_WARNING, "End of file while reading body");
		} else {
			crx_error_docref(NULL, E_WARNING, "Error while reading body");
		}
		crx_stream_close(stream);
		RETURN_FALSE;
	}
	crx_stream_close(stream);

	object_init_ex(return_value, gd_font_ce);
	crx_gd_font_object_from_crex_object(C_OBJ_P(return_value))->font = font;
}
/* }}} */

/* {{{ Set the line drawing styles for use with imageline and IMG_COLOR_STYLED. */
CRX_FUNCTION(imagesetstyle)
{
	zval *IM, *styles, *item;
	gdImagePtr im;
	int *stylearr;
	int index = 0;
	uint32_t num_styles;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oa", &IM, gd_image_ce, &styles) == FAILURE)  {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	num_styles = crex_hash_num_elements(C_ARRVAL_P(styles));
	if (num_styles == 0) {
		crex_argument_value_error(2, "cannot be empty");
		RETURN_THROWS();
	}

	/* copy the style values in the stylearr */
	stylearr = safe_emalloc(sizeof(int), num_styles, 0);

	CREX_HASH_FOREACH_VAL(C_ARRVAL_P(styles), item) {
		stylearr[index++] = zval_get_long(item);
	} CREX_HASH_FOREACH_END();

	gdImageSetStyle(im, stylearr, index);

	efree(stylearr);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Create a new true color image */
CRX_FUNCTION(imagecreatetruecolor)
{
	crex_long x_size, y_size;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ll", &x_size, &y_size) == FAILURE) {
		RETURN_THROWS();
	}

	if (x_size <= 0 || x_size >= INT_MAX) {
		crex_argument_value_error(1, "must be greater than 0");
		RETURN_THROWS();
	}

	if (y_size <= 0 || y_size >= INT_MAX) {
		crex_argument_value_error(2, "must be greater than 0");
		RETURN_THROWS();
	}

	im = gdImageCreateTrueColor(x_size, y_size);

	if (!im) {
		RETURN_FALSE;
	}

	crx_gd_assign_libgdimageptr_as_extgdimage(return_value, im);
}
/* }}} */

/* {{{ return true if the image uses truecolor */
CRX_FUNCTION(imageistruecolor)
{
	zval *IM;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &IM, gd_image_ce) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	RETURN_BOOL(im->trueColor);
}
/* }}} */

/* {{{ Convert a true color image to a palette based image with a number of colors, optionally using dithering. */
CRX_FUNCTION(imagetruecolortopalette)
{
	zval *IM;
	bool dither;
	crex_long ncolors;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Obl", &IM, gd_image_ce, &dither, &ncolors) == FAILURE)  {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	if (ncolors <= 0 || CREX_LONG_INT_OVFL(ncolors)) {
		crex_argument_value_error(3, "must be greater than 0 and less than %d", INT_MAX);
		RETURN_THROWS();
	}

	if (gdImageTrueColorToPalette(im, dither, (int)ncolors)) {
		RETURN_TRUE;
	} else {
		crx_error_docref(NULL, E_WARNING, "Couldn't convert to palette");
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Convert a palette based image to a true color image. */
CRX_FUNCTION(imagepalettetotruecolor)
{
	zval *IM;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &IM, gd_image_ce) == FAILURE)  {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	if (gdImagePaletteToTrueColor(im) == 0) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Makes the colors of the palette version of an image more closely match the true color version */
CRX_FUNCTION(imagecolormatch)
{
	zval *IM1, *IM2;
	gdImagePtr im1, im2;
	int result;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OO", &IM1, gd_image_ce, &IM2, gd_image_ce) == FAILURE) {
		RETURN_THROWS();
	}

	im1 = crx_gd_libgdimageptr_from_zval_p(IM1);
	im2 = crx_gd_libgdimageptr_from_zval_p(IM2);

	result = gdImageColorMatch(im1, im2);
	switch (result) {
		case -1:
			crex_argument_value_error(1, "must be TrueColor");
			RETURN_THROWS();
			break;
		case -2:
			crex_argument_value_error(2, "must be Palette");
			RETURN_THROWS();
			break;
		case -3:
			crex_argument_value_error(2, "must be the same size as argument #1 ($im1)");
			RETURN_THROWS();
			break;
		case -4:
			crex_argument_value_error(2, "must have at least one color");
			RETURN_THROWS();
			break;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Set line thickness for drawing lines, ellipses, rectangles, polygons etc. */
CRX_FUNCTION(imagesetthickness)
{
	zval *IM;
	crex_long thick;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ol", &IM, gd_image_ce, &thick) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	gdImageSetThickness(im, thick);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Draw an ellipse */
CRX_FUNCTION(imagefilledellipse)
{
	zval *IM;
	crex_long cx, cy, w, h, color;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Olllll", &IM, gd_image_ce, &cx, &cy, &w, &h, &color) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	gdImageFilledEllipse(im, cx, cy, w, h, color);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Draw a filled partial ellipse */
CRX_FUNCTION(imagefilledarc)
{
	zval *IM;
	crex_long cx, cy, w, h, ST, E, col, style;
	gdImagePtr im;
	int e, st;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ollllllll", &IM, gd_image_ce, &cx, &cy, &w, &h, &ST, &E, &col, &style) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	e = E;
	if (e < 0) {
		e %= 360;
	}

	st = ST;
	if (st < 0) {
		st %= 360;
	}

	gdImageFilledArc(im, cx, cy, w, h, st, e, col, style);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Turn alpha blending mode on or off for the given image */
CRX_FUNCTION(imagealphablending)
{
	zval *IM;
	bool blend;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ob", &IM, gd_image_ce, &blend) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	gdImageAlphaBlending(im, blend);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Include alpha channel to a saved image */
CRX_FUNCTION(imagesavealpha)
{
	zval *IM;
	bool save;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ob", &IM, gd_image_ce, &save) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	gdImageSaveAlpha(im, save);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Set the alpha blending flag to use the bundled libgd layering effects */
CRX_FUNCTION(imagelayereffect)
{
	zval *IM;
	crex_long effect;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ol", &IM, gd_image_ce, &effect) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	gdImageAlphaBlending(im, effect);

	RETURN_TRUE;
}
/* }}} */

#define CHECK_RGBA_RANGE(component, name, argument_number) \
	if (component < 0 || component > gd##name##Max) { \
		crex_argument_value_error(argument_number, "must be between 0 and %d (inclusive)", gd##name##Max); \
		RETURN_THROWS(); \
	}

/* {{{ Allocate a color with an alpha level.  Works for true color and palette based images */
CRX_FUNCTION(imagecolorallocatealpha)
{
	zval *IM;
	crex_long red, green, blue, alpha;
	gdImagePtr im;
	int ct = (-1);

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ollll", &IM, gd_image_ce, &red, &green, &blue, &alpha) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	CHECK_RGBA_RANGE(red, Red, 2);
	CHECK_RGBA_RANGE(green, Green, 3);
	CHECK_RGBA_RANGE(blue, Blue, 4);
	CHECK_RGBA_RANGE(alpha, Alpha, 5);

	ct = gdImageColorAllocateAlpha(im, red, green, blue, alpha);
	if (ct < 0) {
		RETURN_FALSE;
	}
	RETURN_LONG((crex_long)ct);
}
/* }}} */

/* {{{ Resolve/Allocate a colour with an alpha level.  Works for true colour and palette based images */
CRX_FUNCTION(imagecolorresolvealpha)
{
	zval *IM;
	crex_long red, green, blue, alpha;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ollll", &IM, gd_image_ce, &red, &green, &blue, &alpha) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	CHECK_RGBA_RANGE(red, Red, 2);
	CHECK_RGBA_RANGE(green, Green, 3);
	CHECK_RGBA_RANGE(blue, Blue, 4);
	CHECK_RGBA_RANGE(alpha, Alpha, 5);

	RETURN_LONG(gdImageColorResolveAlpha(im, red, green, blue, alpha));
}
/* }}} */

/* {{{ Find the closest matching colour with alpha transparency */
CRX_FUNCTION(imagecolorclosestalpha)
{
	zval *IM;
	crex_long red, green, blue, alpha;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ollll", &IM, gd_image_ce, &red, &green, &blue, &alpha) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	CHECK_RGBA_RANGE(red, Red, 2);
	CHECK_RGBA_RANGE(green, Green, 3);
	CHECK_RGBA_RANGE(blue, Blue, 4);
	CHECK_RGBA_RANGE(alpha, Alpha, 5);

	RETURN_LONG(gdImageColorClosestAlpha(im, red, green, blue, alpha));
}
/* }}} */

/* {{{ Find exact match for colour with transparency */
CRX_FUNCTION(imagecolorexactalpha)
{
	zval *IM;
	crex_long red, green, blue, alpha;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ollll", &IM, gd_image_ce, &red, &green, &blue, &alpha) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	CHECK_RGBA_RANGE(red, Red, 2);
	CHECK_RGBA_RANGE(green, Green, 3);
	CHECK_RGBA_RANGE(blue, Blue, 4);
	CHECK_RGBA_RANGE(alpha, Alpha, 5);

	RETURN_LONG(gdImageColorExactAlpha(im, red, green, blue, alpha));
}
/* }}} */

/* {{{ Copy and resize part of an image using resampling to help ensure clarity */
CRX_FUNCTION(imagecopyresampled)
{
	zval *SIM, *DIM;
	crex_long SX, SY, SW, SH, DX, DY, DW, DH;
	gdImagePtr im_dst, im_src;
	int srcH, srcW, dstH, dstW, srcY, srcX, dstY, dstX;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OOllllllll", &DIM, gd_image_ce, &SIM, gd_image_ce, &DX, &DY, &SX, &SY, &DW, &DH, &SW, &SH) == FAILURE) {
		RETURN_THROWS();
	}

	im_src = crx_gd_libgdimageptr_from_zval_p(SIM);
	im_dst = crx_gd_libgdimageptr_from_zval_p(DIM);

	srcX = SX;
	srcY = SY;
	srcH = SH;
	srcW = SW;
	dstX = DX;
	dstY = DY;
	dstH = DH;
	dstW = DW;

	gdImageCopyResampled(im_dst, im_src, dstX, dstY, srcX, srcY, dstW, dstH, srcW, srcH);

	RETURN_TRUE;
}
/* }}} */

#ifdef CRX_WIN32
/* {{{ Grab a window or its client area using a windows handle (HWND property in COM instance) */
CRX_FUNCTION(imagegrabwindow)
{
	HWND window;
	bool client_area = 0;
	RECT rc = {0};
	int Width, Height;
	HDC		hdc;
	HDC memDC;
	HBITMAP memBM;
	HBITMAP hOld;
	crex_long lwindow_handle;
	gdImagePtr im = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l|b", &lwindow_handle, &client_area) == FAILURE) {
		RETURN_THROWS();
	}

	window = (HWND) lwindow_handle;

	if (!IsWindow(window)) {
		crx_error_docref(NULL, E_NOTICE, "Invalid window handle");
		RETURN_FALSE;
	}

	hdc		= GetDC(0);

	if (client_area) {
		GetClientRect(window, &rc);
		Width = rc.right;
		Height = rc.bottom;
	} else {
		GetWindowRect(window, &rc);
		Width	= rc.right - rc.left;
		Height	= rc.bottom - rc.top;
	}

	Width		= (Width/4)*4;

	memDC	= CreateCompatibleDC(hdc);
	memBM	= CreateCompatibleBitmap(hdc, Width, Height);
	hOld	= (HBITMAP) SelectObject (memDC, memBM);

	PrintWindow(window, memDC, (UINT) client_area);

	im = gdImageCreateTrueColor(Width, Height);
	if (im) {
		int x,y;
		for (y=0; y <= Height; y++) {
			for (x=0; x <= Width; x++) {
				int c = GetPixel(memDC, x,y);
				gdImageSetPixel(im, x, y, gdTrueColor(GetRValue(c), GetGValue(c), GetBValue(c)));
			}
		}
	}

	SelectObject(memDC,hOld);
	DeleteObject(memBM);
	DeleteDC(memDC);
	ReleaseDC( 0, hdc );

	if (!im) {
		RETURN_FALSE;
	}

	crx_gd_assign_libgdimageptr_as_extgdimage(return_value, im);
}
/* }}} */

/* {{{ Grab a screenshot */
CRX_FUNCTION(imagegrabscreen)
{
	HWND window = GetDesktopWindow();
	RECT rc = {0};
	int Width, Height;
	HDC		hdc;
	HDC memDC;
	HBITMAP memBM;
	HBITMAP hOld;
	gdImagePtr im;
	hdc		= GetDC(0);

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if (!hdc) {
		RETURN_FALSE;
	}

	GetWindowRect(window, &rc);
	Width	= rc.right - rc.left;
	Height	= rc.bottom - rc.top;

	Width		= (Width/4)*4;

	memDC	= CreateCompatibleDC(hdc);
	memBM	= CreateCompatibleBitmap(hdc, Width, Height);
	hOld	= (HBITMAP) SelectObject (memDC, memBM);
	BitBlt( memDC, 0, 0, Width, Height , hdc, rc.left, rc.top , SRCCOPY );

	im = gdImageCreateTrueColor(Width, Height);
	if (im) {
		int x,y;
		for (y=0; y <= Height; y++) {
			for (x=0; x <= Width; x++) {
				int c = GetPixel(memDC, x,y);
				gdImageSetPixel(im, x, y, gdTrueColor(GetRValue(c), GetGValue(c), GetBValue(c)));
			}
		}
	}

	SelectObject(memDC,hOld);
	DeleteObject(memBM);
	DeleteDC(memDC);
	ReleaseDC( 0, hdc );

	if (!im) {
		RETURN_FALSE;
	}

	crx_gd_assign_libgdimageptr_as_extgdimage(return_value, im);
}
/* }}} */
#endif /* CRX_WIN32 */

/* {{{ Rotate an image using a custom angle */
CRX_FUNCTION(imagerotate)
{
	zval *SIM;
	gdImagePtr im_dst, im_src;
	double degrees;
	crex_long color;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Odl", &SIM, gd_image_ce,  &degrees, &color) == FAILURE) {
		RETURN_THROWS();
	}

	im_src = crx_gd_libgdimageptr_from_zval_p(SIM);
	im_dst = gdImageRotateInterpolated(im_src, (const float)degrees, color);

	if (im_dst == NULL) {
		RETURN_FALSE;
	}

	crx_gd_assign_libgdimageptr_as_extgdimage(return_value, im_dst);
}
/* }}} */

/* {{{ Set the tile image to $tile when filling $image with the "IMG_COLOR_TILED" color */
CRX_FUNCTION(imagesettile)
{
	zval *IM, *TILE;
	gdImagePtr im, tile;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OO", &IM, gd_image_ce, &TILE, gd_image_ce) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);
	tile = crx_gd_libgdimageptr_from_zval_p(TILE);

	gdImageSetTile(im, tile);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Set the brush image to $brush when filling $image with the "IMG_COLOR_BRUSHED" color */
CRX_FUNCTION(imagesetbrush)
{
	zval *IM, *TILE;
	gdImagePtr im, tile;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OO", &IM, gd_image_ce, &TILE, gd_image_ce) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);
	tile = crx_gd_libgdimageptr_from_zval_p(TILE);

	gdImageSetBrush(im, tile);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Create a new image */
CRX_FUNCTION(imagecreate)
{
	crex_long x_size, y_size;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ll", &x_size, &y_size) == FAILURE) {
		RETURN_THROWS();
	}

	if (x_size <= 0 || x_size >= INT_MAX) {
		crex_argument_value_error(1, "must be greater than 0");
		RETURN_THROWS();
	}

	if (y_size <= 0 || y_size >= INT_MAX) {
		crex_argument_value_error(2, "must be greater than 0");
		RETURN_THROWS();
	}

	im = gdImageCreate(x_size, y_size);

	if (!im) {
		RETURN_FALSE;
	}

	crx_gd_assign_libgdimageptr_as_extgdimage(return_value, im);
}
/* }}} */

/* {{{ Return the types of images supported in a bitfield - 1=GIF, 2=JPEG, 4=PNG, 8=WBMP, 16=XPM, etc */
CRX_FUNCTION(imagetypes)
{
	int ret = 0;
	ret = CRX_IMG_GIF;
#ifdef HAVE_GD_JPG
	ret |= CRX_IMG_JPG;
#endif
#ifdef HAVE_GD_PNG
	ret |= CRX_IMG_PNG;
#endif
	ret |= CRX_IMG_WBMP;
#ifdef HAVE_GD_XPM
	ret |= CRX_IMG_XPM;
#endif
#ifdef HAVE_GD_WEBP
	ret |= CRX_IMG_WEBP;
#endif
#ifdef HAVE_GD_BMP
	ret |= CRX_IMG_BMP;
#endif
#ifdef HAVE_GD_TGA
	ret |= CRX_IMG_TGA;
#endif
#ifdef HAVE_GD_AVIF
	ret |= CRX_IMG_AVIF;
#endif

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	RETURN_LONG(ret);
}
/* }}} */

/* {{{ _crx_ctx_getmbi */

static int _crx_ctx_getmbi(gdIOCtx *ctx)
{
	int i, mbi = 0;

	do {
		i = (ctx->getC)(ctx);
		if (i < 0) {
			return -1;
		}
		mbi = (mbi << 7) | (i & 0x7f);
	} while (i & 0x80);

	return mbi;
}
/* }}} */

/* {{{ _crx_image_type
 * Based on ext/standard/image.c
 */
static const char crx_sig_gd2[3] = {'g', 'd', '2'};

static int _crx_image_type(crex_string *data)
{
	if (ZSTR_LEN(data) < 12) {
		/* Handle this the same way as an unknown image type. */
		return -1;
	}

	if (!memcmp(ZSTR_VAL(data), crx_sig_gd2, sizeof(crx_sig_gd2))) {
		return CRX_GDIMG_TYPE_GD2;
	} else if (!memcmp(ZSTR_VAL(data), crx_sig_jpg, sizeof(crx_sig_jpg))) {
		return CRX_GDIMG_TYPE_JPG;
	} else if (!memcmp(ZSTR_VAL(data), crx_sig_png, sizeof(crx_sig_png))) {
		return CRX_GDIMG_TYPE_PNG;
	} else if (!memcmp(ZSTR_VAL(data), crx_sig_gif, sizeof(crx_sig_gif))) {
		return CRX_GDIMG_TYPE_GIF;
	} else if (!memcmp(ZSTR_VAL(data), crx_sig_bmp, sizeof(crx_sig_bmp))) {
		return CRX_GDIMG_TYPE_BMP;
	} else if(!memcmp(ZSTR_VAL(data), crx_sig_riff, sizeof(crx_sig_riff)) && !memcmp(ZSTR_VAL(data) + sizeof(crx_sig_riff) + sizeof(uint32_t), crx_sig_webp, sizeof(crx_sig_webp))) {
		return CRX_GDIMG_TYPE_WEBP;
	}

	crx_stream *image_stream = crx_stream_memory_open(TEMP_STREAM_READONLY, data);

	if (image_stream != NULL) {
		bool is_avif = crx_is_image_avif(image_stream);
		crx_stream_close(image_stream);

		if (is_avif) {
			return CRX_GDIMG_TYPE_AVIF;
		}
	}

	gdIOCtx *io_ctx;
	io_ctx = gdNewDynamicCtxEx(8, ZSTR_VAL(data), 0);
	if (io_ctx) {
		if (_crx_ctx_getmbi(io_ctx) == 0 && _crx_ctx_getmbi(io_ctx) >= 0) {
			io_ctx->gd_free(io_ctx);
			return CRX_GDIMG_TYPE_WBM;
		} else {
			io_ctx->gd_free(io_ctx);
		}
	}

	return -1;
}
/* }}} */

/* {{{ _crx_image_create_from_string */
gdImagePtr _crx_image_create_from_string(crex_string *data, char *tn, gdImagePtr (*ioctx_func_p)(gdIOCtxPtr))
{
	gdImagePtr im;
	gdIOCtx *io_ctx;

	io_ctx = gdNewDynamicCtxEx(ZSTR_LEN(data), ZSTR_VAL(data), 0);

	if (!io_ctx) {
		return NULL;
	}

	im = (*ioctx_func_p)(io_ctx);
	if (!im) {
		crx_error_docref(NULL, E_WARNING, "Passed data is not in \"%s\" format", tn);
		io_ctx->gd_free(io_ctx);
		return NULL;
	}

	io_ctx->gd_free(io_ctx);

	return im;
}
/* }}} */

/* {{{ Create a new image from the image stream in the string */
CRX_FUNCTION(imagecreatefromstring)
{
	crex_string *data;
	gdImagePtr im;
	int imtype;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "S", &data) == FAILURE) {
		RETURN_THROWS();
	}

	imtype = _crx_image_type(data);

	switch (imtype) {
		case CRX_GDIMG_TYPE_JPG:
#ifdef HAVE_GD_JPG
			im = _crx_image_create_from_string(data, "JPEG", gdImageCreateFromJpegCtx);
#else
			crx_error_docref(NULL, E_WARNING, "No JPEG support in this CRX build");
			RETURN_FALSE;
#endif
			break;

		case CRX_GDIMG_TYPE_PNG:
#ifdef HAVE_GD_PNG
			im = _crx_image_create_from_string(data, "PNG", gdImageCreateFromPngCtx);
#else
			crx_error_docref(NULL, E_WARNING, "No PNG support in this CRX build");
			RETURN_FALSE;
#endif
			break;

		case CRX_GDIMG_TYPE_GIF:
			im = _crx_image_create_from_string(data, "GIF", gdImageCreateFromGifCtx);
			break;

		case CRX_GDIMG_TYPE_WBM:
			im = _crx_image_create_from_string(data, "WBMP", gdImageCreateFromWBMPCtx);
			break;

		case CRX_GDIMG_TYPE_GD2:
			im = _crx_image_create_from_string(data, "GD2", gdImageCreateFromGd2Ctx);
			break;

		case CRX_GDIMG_TYPE_BMP:
			im = _crx_image_create_from_string(data, "BMP", gdImageCreateFromBmpCtx);
			break;

		case CRX_GDIMG_TYPE_WEBP:
#ifdef HAVE_GD_WEBP
			im = _crx_image_create_from_string(data, "WEBP", gdImageCreateFromWebpCtx);
			break;
#else
			crx_error_docref(NULL, E_WARNING, "No WEBP support in this CRX build");
			RETURN_FALSE;
#endif

		case CRX_GDIMG_TYPE_AVIF:
#ifdef HAVE_GD_AVIF
			im = _crx_image_create_from_string(data, "AVIF", gdImageCreateFromAvifCtx);
			break;
#else
			crx_error_docref(NULL, E_WARNING, "No AVIF support in this CRX build");
			RETURN_FALSE;
#endif

		default:
			crx_error_docref(NULL, E_WARNING, "Data is not in a recognized format");
			RETURN_FALSE;
	}

	if (!im) {
		crx_error_docref(NULL, E_WARNING, "Couldn't create GD Image Stream out of Data");
		RETURN_FALSE;
	}

	crx_gd_assign_libgdimageptr_as_extgdimage(return_value, im);
}
/* }}} */

/* {{{ _crx_image_create_from */
static void _crx_image_create_from(INTERNAL_FUNCTION_PARAMETERS, int image_type, char *tn, gdImagePtr (*func_p)(FILE *), gdImagePtr (*ioctx_func_p)(gdIOCtxPtr))
{
	char *file;
	size_t file_len;
	crex_long srcx, srcy, width, height;
	gdImagePtr im = NULL;
	crx_stream *stream;
	FILE * fp = NULL;
#ifdef HAVE_GD_JPG
	long ignore_warning;
#endif

	if (image_type == CRX_GDIMG_TYPE_GD2PART) {
		if (crex_parse_parameters(CREX_NUM_ARGS(), "pllll", &file, &file_len, &srcx, &srcy, &width, &height) == FAILURE) {
			RETURN_THROWS();
		}

		if (width < 1) {
			crex_argument_value_error(4, "must be greater than or equal to 1");
			RETURN_THROWS();
		}

		if (height < 1) {
			crex_argument_value_error(5, "must be greater than or equal to 1");
			RETURN_THROWS();
		}

	} else {
		if (crex_parse_parameters(CREX_NUM_ARGS(), "p", &file, &file_len) == FAILURE) {
			RETURN_THROWS();
		}
	}


	stream = crx_stream_open_wrapper(file, "rb", REPORT_ERRORS|IGNORE_PATH, NULL);
	if (stream == NULL)	{
		RETURN_FALSE;
	}

	/* try and avoid allocating a FILE* if the stream is not naturally a FILE* */
	if (crx_stream_is(stream, CRX_STREAM_IS_STDIO))	{
		if (FAILURE == crx_stream_cast(stream, CRX_STREAM_AS_STDIO, (void**)&fp, REPORT_ERRORS)) {
			goto out_err;
		}
	} else if (ioctx_func_p || image_type == CRX_GDIMG_TYPE_GD2PART) {
		/* we can create an io context */
		gdIOCtx* io_ctx;
		crex_string *buff;
		char *pstr;

		buff = crx_stream_copy_to_mem(stream, CRX_STREAM_COPY_ALL, 0);

		if (!buff) {
			crx_error_docref(NULL, E_WARNING,"Cannot read image data");
			goto out_err;
		}

		/* needs to be malloc (persistent) - GD will free() it later */
		pstr = pestrndup(ZSTR_VAL(buff), ZSTR_LEN(buff), 1);
		io_ctx = gdNewDynamicCtxEx(ZSTR_LEN(buff), pstr, 0);
		if (!io_ctx) {
			pefree(pstr, 1);
			crex_string_release_ex(buff, 0);
			crx_error_docref(NULL, E_WARNING,"Cannot allocate GD IO context");
			goto out_err;
		}

		if (image_type == CRX_GDIMG_TYPE_GD2PART) {
			im = gdImageCreateFromGd2PartCtx(io_ctx, srcx, srcy, width, height);
		} else {
			im = (*ioctx_func_p)(io_ctx);
		}
		io_ctx->gd_free(io_ctx);
		pefree(pstr, 1);
		crex_string_release_ex(buff, 0);
	}
	else if (crx_stream_can_cast(stream, CRX_STREAM_AS_STDIO)) {
		/* try and force the stream to be FILE* */
		if (FAILURE == crx_stream_cast(stream, CRX_STREAM_AS_STDIO | CRX_STREAM_CAST_TRY_HARD, (void **) &fp, REPORT_ERRORS)) {
			goto out_err;
		}
	}

	if (!im && fp) {
		switch (image_type) {
			case CRX_GDIMG_TYPE_GD2PART:
				im = gdImageCreateFromGd2Part(fp, srcx, srcy, width, height);
				break;
#ifdef HAVE_GD_XPM
			case CRX_GDIMG_TYPE_XPM:
				im = gdImageCreateFromXpm(file);
				break;
#endif

#ifdef HAVE_GD_JPG
			case CRX_GDIMG_TYPE_JPG:
				ignore_warning = INI_INT("gd.jpeg_ignore_warning");
				im = gdImageCreateFromJpegEx(fp, ignore_warning);
			break;
#endif

			default:
				im = (*func_p)(fp);
				break;
		}

		fflush(fp);
	}

/* register_im: */
	if (im) {
		crx_stream_close(stream);
		crx_gd_assign_libgdimageptr_as_extgdimage(return_value, im);
		return;
	}

	crx_error_docref(NULL, E_WARNING, "\"%s\" is not a valid %s file", file, tn);
out_err:
	crx_stream_close(stream);
	RETURN_FALSE;

}
/* }}} */

/* {{{ Create a new image from GIF file or URL */
CRX_FUNCTION(imagecreatefromgif)
{
	_crx_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_GDIMG_TYPE_GIF, "GIF", gdImageCreateFromGif, gdImageCreateFromGifCtx);
}
/* }}} */

#ifdef HAVE_GD_JPG
/* {{{ Create a new image from JPEG file or URL */
CRX_FUNCTION(imagecreatefromjpeg)
{
	_crx_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_GDIMG_TYPE_JPG, "JPEG", gdImageCreateFromJpeg, gdImageCreateFromJpegCtx);
}
/* }}} */
#endif /* HAVE_GD_JPG */

#ifdef HAVE_GD_PNG
/* {{{ Create a new image from PNG file or URL */
CRX_FUNCTION(imagecreatefrompng)
{
	_crx_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_GDIMG_TYPE_PNG, "PNG", gdImageCreateFromPng, gdImageCreateFromPngCtx);
}
/* }}} */
#endif /* HAVE_GD_PNG */

#ifdef HAVE_GD_WEBP
/* {{{ Create a new image from WEBP file or URL */
CRX_FUNCTION(imagecreatefromwebp)
{
	_crx_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_GDIMG_TYPE_WEBP, "WEBP", gdImageCreateFromWebp, gdImageCreateFromWebpCtx);
}
/* }}} */
#endif /* HAVE_GD_WEBP */

/* {{{ Create a new image from XBM file or URL */
CRX_FUNCTION(imagecreatefromxbm)
{
	_crx_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_GDIMG_TYPE_XBM, "XBM", gdImageCreateFromXbm, NULL);
}
/* }}} */

#ifdef HAVE_GD_AVIF
/* {{{ Create a new image from AVIF file or URL */
CRX_FUNCTION(imagecreatefromavif)
{
	_crx_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_GDIMG_TYPE_AVIF, "AVIF", gdImageCreateFromAvif, gdImageCreateFromAvifCtx);
}
/* }}} */
#endif /* HAVE_GD_AVIF */

#ifdef HAVE_GD_XPM
/* {{{ Create a new image from XPM file or URL */
CRX_FUNCTION(imagecreatefromxpm)
{
	_crx_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_GDIMG_TYPE_XPM, "XPM", NULL, NULL);
}
/* }}} */
#endif

/* {{{ Create a new image from WBMP file or URL */
CRX_FUNCTION(imagecreatefromwbmp)
{
	_crx_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_GDIMG_TYPE_WBM, "WBMP", gdImageCreateFromWBMP, gdImageCreateFromWBMPCtx);
}
/* }}} */

/* {{{ Create a new image from GD file or URL */
CRX_FUNCTION(imagecreatefromgd)
{
	_crx_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_GDIMG_TYPE_GD, "GD", gdImageCreateFromGd, gdImageCreateFromGdCtx);
}
/* }}} */

/* {{{ Create a new image from GD2 file or URL */
CRX_FUNCTION(imagecreatefromgd2)
{
	_crx_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_GDIMG_TYPE_GD2, "GD2", gdImageCreateFromGd2, gdImageCreateFromGd2Ctx);
}
/* }}} */

/* {{{ Create a new image from a given part of GD2 file or URL */
CRX_FUNCTION(imagecreatefromgd2part)
{
	_crx_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_GDIMG_TYPE_GD2PART, "GD2", NULL, NULL);
}
/* }}} */

#ifdef HAVE_GD_BMP
/* {{{ Create a new image from BMP file or URL */
CRX_FUNCTION(imagecreatefrombmp)
{
	_crx_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_GDIMG_TYPE_BMP, "BMP", gdImageCreateFromBmp, gdImageCreateFromBmpCtx);
}
/* }}} */
#endif

#ifdef HAVE_GD_TGA
/* {{{ Create a new image from TGA file or URL */
CRX_FUNCTION(imagecreatefromtga)
{
	_crx_image_create_from(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_GDIMG_TYPE_TGA, "TGA", gdImageCreateFromTga, gdImageCreateFromTgaCtx);
}
/* }}} */
#endif

/* {{{ _crx_image_output */
static void _crx_image_output(INTERNAL_FUNCTION_PARAMETERS, int image_type, char *tn)
{
	zval *imgind;
	char *file = NULL;
	crex_long quality = 128, type = 1;
	gdImagePtr im;
	FILE *fp;
	size_t file_len = 0;

	/* The quality parameter for gd2 stands for chunk size */

	switch (image_type) {
		case CRX_GDIMG_TYPE_GD:
			if (crex_parse_parameters(CREX_NUM_ARGS(), "O|p!", &imgind, gd_image_ce, &file, &file_len) == FAILURE) {
				RETURN_THROWS();
			}
			break;
		case CRX_GDIMG_TYPE_GD2:
			if (crex_parse_parameters(CREX_NUM_ARGS(), "O|p!ll", &imgind, gd_image_ce, &file, &file_len, &quality, &type) == FAILURE) {
				RETURN_THROWS();
			}
			break;
		EMPTY_SWITCH_DEFAULT_CASE()
	}

	/* quality must fit in an int */
	if (quality < INT_MIN || quality > INT_MAX) {
		crx_error_docref(NULL, E_WARNING, "Argument #3 ($chunk_size) must be between %d and %d", INT_MIN, INT_MAX);
		RETURN_FALSE;
	}

	im = crx_gd_libgdimageptr_from_zval_p(imgind);

	if (file_len) {
		CRX_GD_CHECK_OPEN_BASEDIR(file, "Invalid filename");

		fp = VCWD_FOPEN(file, "wb");
		if (!fp) {
			crx_error_docref(NULL, E_WARNING, "Unable to open \"%s\" for writing", file);
			RETURN_FALSE;
		}

		switch (image_type) {
			case CRX_GDIMG_TYPE_GD:
				gdImageGd(im, fp);
				break;
			case CRX_GDIMG_TYPE_GD2:
				if (quality == -1) {
					quality = 128;
				}
				gdImageGd2(im, fp, quality, type);
				break;
			EMPTY_SWITCH_DEFAULT_CASE()
		}
		fflush(fp);
		fclose(fp);
	} else {
		int   b;
		FILE *tmp;
		char  buf[4096];
		crex_string *path;

		tmp = crx_open_temporary_file(NULL, NULL, &path);
		if (tmp == NULL) {
			crx_error_docref(NULL, E_WARNING, "Unable to open temporary file");
			RETURN_FALSE;
		}

		switch (image_type) {
			case CRX_GDIMG_TYPE_GD:
				gdImageGd(im, tmp);
				break;
			case CRX_GDIMG_TYPE_GD2:
				if (quality == -1) {
					quality = 128;
				}
				gdImageGd2(im, tmp, quality, type);
				break;
			EMPTY_SWITCH_DEFAULT_CASE()
		}

		fseek(tmp, 0, SEEK_SET);

		while ((b = fread(buf, 1, sizeof(buf), tmp)) > 0) {
			crx_write(buf, b);
		}

		fclose(tmp);
		VCWD_UNLINK((const char *)ZSTR_VAL(path)); /* make sure that the temporary file is removed */
		crex_string_release_ex(path, 0);
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Output XBM image to browser or file */
CRX_FUNCTION(imagexbm)
{
	zval *imgind;
	char *file = NULL;
	size_t file_len = 0;
	crex_long foreground_color;
	bool foreground_color_is_null = 1;
	gdImagePtr im;
	int i;
	gdIOCtx *ctx = NULL;
	crx_stream *stream;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Op!|l!", &imgind, gd_image_ce, &file, &file_len, &foreground_color, &foreground_color_is_null) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(imgind);

	if (file != NULL) {
		stream = crx_stream_open_wrapper(file, "wb", REPORT_ERRORS|IGNORE_PATH, NULL);
		if (stream == NULL) {
			RETURN_FALSE;
		}

		ctx = create_stream_context(stream, 1);
	} else {
		ctx = create_output_context();
	}

	if (foreground_color_is_null) {
		for (i=0; i < gdImageColorsTotal(im); i++) {
			if (!gdImageRed(im, i) && !gdImageGreen(im, i) && !gdImageBlue(im, i)) {
				break;
			}
		}

		foreground_color = i;
	}

	gdImageXbmCtx(im, file ? file : "", (int) foreground_color, ctx);

	ctx->gd_free(ctx);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Output GIF image to browser or file */
CRX_FUNCTION(imagegif)
{
	_crx_image_output_ctx(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_GDIMG_TYPE_GIF, "GIF");
}
/* }}} */

#ifdef HAVE_GD_PNG
/* {{{ Output PNG image to browser or file */
CRX_FUNCTION(imagepng)
{
	_crx_image_output_ctx(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_GDIMG_TYPE_PNG, "PNG");
}
/* }}} */
#endif /* HAVE_GD_PNG */

#ifdef HAVE_GD_WEBP
/* {{{ Output WEBP image to browser or file */
CRX_FUNCTION(imagewebp)
{
	_crx_image_output_ctx(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_GDIMG_TYPE_WEBP, "WEBP");
}
/* }}} */
#endif /* HAVE_GD_WEBP */

#ifdef HAVE_GD_AVIF
/* {{{ Output AVIF image to browser or file */
CRX_FUNCTION(imageavif)
{
	_crx_image_output_ctx(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_GDIMG_TYPE_AVIF, "AVIF");
}
/* }}} */
#endif /* HAVE_GD_AVIF */

#ifdef HAVE_GD_JPG
/* {{{ Output JPEG image to browser or file */
CRX_FUNCTION(imagejpeg)
{
	_crx_image_output_ctx(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_GDIMG_TYPE_JPG, "JPEG");
}
/* }}} */
#endif /* HAVE_GD_JPG */

/* {{{ Output WBMP image to browser or file */
CRX_FUNCTION(imagewbmp)
{
	zval *imgind;
	crex_long foreground_color;
	crex_long foreground_color_is_null = 1;
	gdImagePtr im;
	int i;
	gdIOCtx *ctx = NULL;
	zval *to_zval = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O|z!l!", &imgind, gd_image_ce, &to_zval, &foreground_color, &foreground_color_is_null) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(imgind);

	if (to_zval != NULL) {
		ctx = create_stream_context_from_zval(to_zval);
		if (!ctx) {
			RETURN_FALSE;
		}
	} else {
		ctx = create_output_context();
	}

	if (foreground_color_is_null) {
		for (i=0; i < gdImageColorsTotal(im); i++) {
			if (!gdImageRed(im, i) && !gdImageGreen(im, i) && !gdImageBlue(im, i)) {
				break;
			}
		}

		foreground_color = i;
	}

	gdImageWBMPCtx(im, foreground_color, ctx);

	ctx->gd_free(ctx);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Output GD image to browser or file */
CRX_FUNCTION(imagegd)
{
	_crx_image_output(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_GDIMG_TYPE_GD, "GD");
}
/* }}} */

/* {{{ Output GD2 image to browser or file */
CRX_FUNCTION(imagegd2)
{
	_crx_image_output(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_GDIMG_TYPE_GD2, "GD2");
}
/* }}} */

#ifdef HAVE_GD_BMP
/* {{{ Output BMP image to browser or file */
CRX_FUNCTION(imagebmp)
{
	zval *imgind;
	bool compressed = 1;
	gdImagePtr im;
	gdIOCtx *ctx = NULL;
	zval *to_zval = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O|z!b", &imgind, gd_image_ce, &to_zval, &compressed) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(imgind);

	if (to_zval != NULL) {
		ctx = create_stream_context_from_zval(to_zval);
		if (!ctx) {
			RETURN_FALSE;
		}
	} else {
		ctx = create_output_context();
	}

	gdImageBmpCtx(im, ctx, (int) compressed);

	ctx->gd_free(ctx);

	RETURN_TRUE;
}
/* }}} */
#endif

/* {{{ Destroy an image - No effect as of CRX 8.0 */
CRX_FUNCTION(imagedestroy)
{
	/* This function used to free the resource, as resources are no longer used, it does nothing */
	zval *IM;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &IM, gd_image_ce) == FAILURE) {
		RETURN_THROWS();
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Allocate a color for an image */
CRX_FUNCTION(imagecolorallocate)
{
	zval *IM;
	crex_long red, green, blue;
	gdImagePtr im;
	int ct = (-1);

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Olll", &IM, gd_image_ce, &red, &green, &blue) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	CHECK_RGBA_RANGE(red, Red, 2);
	CHECK_RGBA_RANGE(green, Green, 3);
	CHECK_RGBA_RANGE(blue, Blue, 4);

	ct = gdImageColorAllocate(im, red, green, blue);
	if (ct < 0) {
		RETURN_FALSE;
	}
	RETURN_LONG(ct);
}
/* }}} */

/* {{{ Copy the palette from the src image onto the dst image */
CRX_FUNCTION(imagepalettecopy)
{
	zval *dstim, *srcim;
	gdImagePtr dst, src;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OO", &dstim, gd_image_ce, &srcim, gd_image_ce) == FAILURE) {
		RETURN_THROWS();
	}

	src = crx_gd_libgdimageptr_from_zval_p(srcim);
	dst = crx_gd_libgdimageptr_from_zval_p(dstim);

	gdImagePaletteCopy(dst, src);
}
/* }}} */

/* {{{ Get the index of the color of a pixel */
CRX_FUNCTION(imagecolorat)
{
	zval *IM;
	crex_long x, y;
	gdImagePtr im;

	CREX_PARSE_PARAMETERS_START(3, 3)
		C_PARAM_OBJECT_OF_CLASS(IM, gd_image_ce)
		C_PARAM_LONG(x)
		C_PARAM_LONG(y)
	CREX_PARSE_PARAMETERS_END();

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	if (gdImageTrueColor(im)) {
		if (im->tpixels && gdImageBoundsSafe(im, x, y)) {
			RETURN_LONG(gdImageTrueColorPixel(im, x, y));
		} else {
			crx_error_docref(NULL, E_NOTICE, "" CREX_LONG_FMT "," CREX_LONG_FMT " is out of bounds", x, y);
			RETURN_FALSE;
		}
	} else {
		if (im->pixels && gdImageBoundsSafe(im, x, y)) {
			RETURN_LONG(im->pixels[y][x]);
		} else {
			crx_error_docref(NULL, E_NOTICE, "" CREX_LONG_FMT "," CREX_LONG_FMT " is out of bounds", x, y);
			RETURN_FALSE;
		}
	}
}
/* }}} */

/* {{{ Get the index of the closest color to the specified color */
CRX_FUNCTION(imagecolorclosest)
{
	zval *IM;
	crex_long red, green, blue;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Olll", &IM, gd_image_ce, &red, &green, &blue) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	CHECK_RGBA_RANGE(red, Red, 2);
	CHECK_RGBA_RANGE(green, Green, 3);
	CHECK_RGBA_RANGE(blue, Blue, 4);

	RETURN_LONG(gdImageColorClosest(im, red, green, blue));
}
/* }}} */

/* {{{ Get the index of the color which has the hue, white and blackness nearest to the given color */
CRX_FUNCTION(imagecolorclosesthwb)
{
	zval *IM;
	crex_long red, green, blue;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Olll", &IM, gd_image_ce, &red, &green, &blue) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	CHECK_RGBA_RANGE(red, Red, 2);
	CHECK_RGBA_RANGE(green, Green, 3);
	CHECK_RGBA_RANGE(blue, Blue, 4);

	RETURN_LONG(gdImageColorClosestHWB(im, red, green, blue));
}
/* }}} */

/* {{{ De-allocate a color for an image */
CRX_FUNCTION(imagecolordeallocate)
{
	zval *IM;
	crex_long index;
	int col;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ol", &IM, gd_image_ce, &index) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	/* We can return right away for a truecolor image as deallocating colours is meaningless here */
	if (gdImageTrueColor(im)) {
		RETURN_TRUE;
	}

	col = index;

	if (col >= 0 && col < gdImageColorsTotal(im)) {
		gdImageColorDeallocate(im, col);
		RETURN_TRUE;
	} else {
		crex_argument_value_error(2, "must be between 0 and %d", gdImageColorsTotal(im));
		RETURN_THROWS();
	}
}
/* }}} */

/* {{{ Get the index of the specified color or its closest possible alternative */
CRX_FUNCTION(imagecolorresolve)
{
	zval *IM;
	crex_long red, green, blue;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Olll", &IM, gd_image_ce, &red, &green, &blue) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	CHECK_RGBA_RANGE(red, Red, 2);
	CHECK_RGBA_RANGE(green, Green, 3);
	CHECK_RGBA_RANGE(blue, Blue, 4);

	RETURN_LONG(gdImageColorResolve(im, red, green, blue));
}
/* }}} */

/* {{{ Get the index of the specified color */
CRX_FUNCTION(imagecolorexact)
{
	zval *IM;
	crex_long red, green, blue;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Olll", &IM, gd_image_ce, &red, &green, &blue) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	CHECK_RGBA_RANGE(red, Red, 2);
	CHECK_RGBA_RANGE(green, Green, 3);
	CHECK_RGBA_RANGE(blue, Blue, 4);

	RETURN_LONG(gdImageColorExact(im, red, green, blue));
}
/* }}} */

/* {{{ Set the color for the specified palette index */
CRX_FUNCTION(imagecolorset)
{
	zval *IM;
	crex_long color, red, green, blue, alpha = 0;
	int col;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ollll|l", &IM, gd_image_ce, &color, &red, &green, &blue, &alpha) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	CHECK_RGBA_RANGE(red, Red, 2);
	CHECK_RGBA_RANGE(green, Green, 3);
	CHECK_RGBA_RANGE(blue, Blue, 4);

	col = color;

	if (col >= 0 && col < gdImageColorsTotal(im)) {
		im->red[col]   = red;
		im->green[col] = green;
		im->blue[col]  = blue;
		im->alpha[col]  = alpha;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Get the colors for an index */
CRX_FUNCTION(imagecolorsforindex)
{
	zval *IM;
	crex_long index;
	int col;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ol", &IM, gd_image_ce, &index) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	col = index;

	if ((col >= 0 && gdImageTrueColor(im)) || (!gdImageTrueColor(im) && col >= 0 && col < gdImageColorsTotal(im))) {
		array_init(return_value);

		add_assoc_long(return_value,"red",  gdImageRed(im,col));
		add_assoc_long(return_value,"green", gdImageGreen(im,col));
		add_assoc_long(return_value,"blue", gdImageBlue(im,col));
		add_assoc_long(return_value,"alpha", gdImageAlpha(im,col));
	} else {
		crex_argument_value_error(2, "is out of range");
		RETURN_THROWS();
	}
}
/* }}} */

/* {{{ Apply a gamma correction to a GD image */
CRX_FUNCTION(imagegammacorrect)
{
	zval *IM;
	gdImagePtr im;
	int i;
	double input, output, gamma;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Odd", &IM, gd_image_ce, &input, &output) == FAILURE) {
		RETURN_THROWS();
	}

	if (input <= 0.0) {
		crex_argument_value_error(2, "must be greater than 0");
		RETURN_THROWS();
	}

	if (output <= 0.0) {
		crex_argument_value_error(3, "must be greater than 0");
		RETURN_THROWS();
	}

	gamma = input / output;

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	if (gdImageTrueColor(im))	{
		int x, y, c;

		for (y = 0; y < gdImageSY(im); y++)	{
			for (x = 0; x < gdImageSX(im); x++)	{
				c = gdImageGetPixel(im, x, y);
				gdImageSetPixel(im, x, y,
					gdTrueColorAlpha(
						(int) ((pow((gdTrueColorGetRed(c)   / 255.0), gamma) * 255) + .5),
						(int) ((pow((gdTrueColorGetGreen(c) / 255.0), gamma) * 255) + .5),
						(int) ((pow((gdTrueColorGetBlue(c)  / 255.0), gamma) * 255) + .5),
						gdTrueColorGetAlpha(c)
					)
				);
			}
		}
		RETURN_TRUE;
	}

	for (i = 0; i < gdImageColorsTotal(im); i++) {
		im->red[i]   = (int)((pow((im->red[i]   / 255.0), gamma) * 255) + .5);
		im->green[i] = (int)((pow((im->green[i] / 255.0), gamma) * 255) + .5);
		im->blue[i]  = (int)((pow((im->blue[i]  / 255.0), gamma) * 255) + .5);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Set a single pixel */
CRX_FUNCTION(imagesetpixel)
{
	zval *IM;
	crex_long x, y, col;
	gdImagePtr im;

	CREX_PARSE_PARAMETERS_START(4, 4)
		C_PARAM_OBJECT_OF_CLASS(IM, gd_image_ce)
		C_PARAM_LONG(x)
		C_PARAM_LONG(y)
		C_PARAM_LONG(col)
	CREX_PARSE_PARAMETERS_END();

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	gdImageSetPixel(im, x, y, col);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Draw a line */
CRX_FUNCTION(imageline)
{
	zval *IM;
	crex_long x1, y1, x2, y2, col;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Olllll", &IM, gd_image_ce,  &x1, &y1, &x2, &y2, &col) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	if (im->AA) {
		gdImageSetAntiAliased(im, col);
		col = gdAntiAliased;
	}
	gdImageLine(im, x1, y1, x2, y2, col);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Draw a dashed line */
CRX_FUNCTION(imagedashedline)
{
	zval *IM;
	crex_long x1, y1, x2, y2, col;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Olllll", &IM, gd_image_ce, &x1, &y1, &x2, &y2, &col) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	gdImageDashedLine(im, x1, y1, x2, y2, col);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Draw a rectangle */
CRX_FUNCTION(imagerectangle)
{
	zval *IM;
	crex_long x1, y1, x2, y2, col;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Olllll", &IM, gd_image_ce, &x1, &y1, &x2, &y2, &col) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	gdImageRectangle(im, x1, y1, x2, y2, col);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Draw a filled rectangle */
CRX_FUNCTION(imagefilledrectangle)
{
	zval *IM;
	crex_long x1, y1, x2, y2, col;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Olllll", &IM, gd_image_ce, &x1, &y1, &x2, &y2, &col) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);
	gdImageFilledRectangle(im, x1, y1, x2, y2, col);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Draw a partial ellipse */
CRX_FUNCTION(imagearc)
{
	zval *IM;
	crex_long cx, cy, w, h, ST, E, col;
	gdImagePtr im;
	int e, st;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Olllllll", &IM, gd_image_ce, &cx, &cy, &w, &h, &ST, &E, &col) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	e = E;
	if (e < 0) {
		e %= 360;
	}

	st = ST;
	if (st < 0) {
		st %= 360;
	}

	gdImageArc(im, cx, cy, w, h, st, e, col);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Draw an ellipse */
CRX_FUNCTION(imageellipse)
{
	zval *IM;
	crex_long cx, cy, w, h, color;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Olllll", &IM, gd_image_ce,  &cx, &cy, &w, &h, &color) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	gdImageEllipse(im, cx, cy, w, h, color);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Flood fill to specific color */
CRX_FUNCTION(imagefilltoborder)
{
	zval *IM;
	crex_long x, y, border, col;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ollll", &IM, gd_image_ce,  &x, &y, &border, &col) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	gdImageFillToBorder(im, x, y, border, col);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Flood fill */
CRX_FUNCTION(imagefill)
{
	zval *IM;
	crex_long x, y, col;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Olll", &IM, gd_image_ce, &x, &y, &col) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	gdImageFill(im, x, y, col);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Find out the number of colors in an image's palette */
CRX_FUNCTION(imagecolorstotal)
{
	zval *IM;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &IM, gd_image_ce) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	RETURN_LONG(gdImageColorsTotal(im));
}
/* }}} */

/* {{{ Define a color as transparent */
CRX_FUNCTION(imagecolortransparent)
{
	zval *IM;
	crex_long COL = 0;
	bool COL_IS_NULL = 1;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O|l!", &IM, gd_image_ce, &COL, &COL_IS_NULL) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	if (!COL_IS_NULL) {
		gdImageColorTransparent(im, COL);
	}

	RETURN_LONG(gdImageGetTransparent(im));
}
/* }}} */

/* {{{ Enable or disable interlace */
CRX_FUNCTION(imageinterlace)
{
	zval *IM;
	bool INT = 0;
	bool INT_IS_NULL = 1;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O|b!", &IM, gd_image_ce, &INT, &INT_IS_NULL) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	if (!INT_IS_NULL) {
		gdImageInterlace(im, INT);
	}

	RETURN_BOOL(gdImageGetInterlaced(im));
}
/* }}} */

/* {{{ crx_imagepolygon
   arg = -1 open polygon
   arg = 0  normal polygon
   arg = 1  filled polygon */
/* im, points, num_points, col */
static void crx_imagepolygon(INTERNAL_FUNCTION_PARAMETERS, int filled)
{
	zval *IM, *POINTS;
	crex_long NPOINTS, COL;
	bool COL_IS_NULL = 1;
	zval *var = NULL;
	gdImagePtr im;
	gdPointPtr points;
	int npoints, col, nelem, i;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oal|l!", &IM, gd_image_ce, &POINTS, &NPOINTS, &COL, &COL_IS_NULL) == FAILURE) {
		RETURN_THROWS();
	}
	if (COL_IS_NULL) {
		COL = NPOINTS;
		NPOINTS = crex_hash_num_elements(C_ARRVAL_P(POINTS));
		if (NPOINTS % 2 != 0) {
			crex_argument_value_error(2, "must have an even number of elements");
			RETURN_THROWS();
		}
		NPOINTS /= 2;
	} else {
		crx_error_docref(NULL, E_DEPRECATED, "Using the $num_points parameter is deprecated");
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	npoints = NPOINTS;
	col = COL;

	nelem = crex_hash_num_elements(C_ARRVAL_P(POINTS));
	if (npoints < 3) {
		crex_argument_value_error(3, "must be greater than or equal to 3");
		RETURN_THROWS();
	}

	if (nelem < npoints * 2) {
		crex_value_error("Trying to use %d points in array with only %d points", npoints, nelem/2);
		RETURN_THROWS();
	}

	points = (gdPointPtr) safe_emalloc(npoints, sizeof(gdPoint), 0);

	for (i = 0; i < npoints; i++) {
		if ((var = crex_hash_index_find(C_ARRVAL_P(POINTS), (i * 2))) != NULL) {
			points[i].x = zval_get_long(var);
		}
		if ((var = crex_hash_index_find(C_ARRVAL_P(POINTS), (i * 2) + 1)) != NULL) {
			points[i].y = zval_get_long(var);
		}
	}

	if (im->AA) {
		gdImageSetAntiAliased(im, col);
		col = gdAntiAliased;
	}
	switch (filled) {
		case -1:
			gdImageOpenPolygon(im, points, npoints, col);
			break;
		case 0:
			gdImagePolygon(im, points, npoints, col);
			break;
		case 1:
			gdImageFilledPolygon(im, points, npoints, col);
			break;
	}

	efree(points);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Draw a polygon */
CRX_FUNCTION(imagepolygon)
{
	crx_imagepolygon(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ Draw a polygon */
CRX_FUNCTION(imageopenpolygon)
{
	crx_imagepolygon(INTERNAL_FUNCTION_PARAM_PASSTHRU, -1);
}
/* }}} */

/* {{{ Draw a filled polygon */
CRX_FUNCTION(imagefilledpolygon)
{
	crx_imagepolygon(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ crx_find_gd_font */
static gdFontPtr crx_find_gd_font(crex_object *font_obj, crex_long font_int)
{
	if (font_obj) {
		return crx_gd_font_object_from_crex_object(font_obj)->font;
	}

	switch (font_int) {
		case 1: return gdFontTiny;
		case 2: return gdFontSmall;
		case 3: return gdFontMediumBold;
		case 4: return gdFontLarge;
		case 5: return gdFontGiant;
	}

	return font_int < 1 ? gdFontTiny : gdFontGiant;
}
/* }}} */

/* {{{ crx_imagefontsize
 * arg = 0  ImageFontWidth
 * arg = 1  ImageFontHeight
 */
static void crx_imagefontsize(INTERNAL_FUNCTION_PARAMETERS, int arg)
{
	crex_object *font_obj;
	crex_long font_int;
	gdFontPtr font;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_OBJ_OF_CLASS_OR_LONG(font_obj, gd_font_ce, font_int)
	CREX_PARSE_PARAMETERS_END();

	font = crx_find_gd_font(font_obj, font_int);
	RETURN_LONG(arg ? font->h : font->w);
}
/* }}} */

/* {{{ Get font width */
CRX_FUNCTION(imagefontwidth)
{
	crx_imagefontsize(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ Get font height */
CRX_FUNCTION(imagefontheight)
{
	crx_imagefontsize(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ crx_gdimagecharup
 * workaround for a bug in gd 1.2 */
static void crx_gdimagecharup(gdImagePtr im, gdFontPtr f, int x, int y, int c, int color)
{
	int cx, cy, px, py, fline;
	cx = 0;
	cy = 0;

	if ((c < f->offset) || (c >= (f->offset + f->nchars))) {
		return;
	}

	fline = (c - f->offset) * f->h * f->w;
	for (py = y; (py > (y - f->w)); py--) {
		for (px = x; (px < (x + f->h)); px++) {
			if (f->data[fline + cy * f->w + cx]) {
				gdImageSetPixel(im, px, py, color);
			}
			cy++;
		}
		cy = 0;
		cx++;
	}
}
/* }}} */

/* {{{ crx_imagechar
 * arg = 0  ImageChar
 * arg = 1  ImageCharUp
 * arg = 2  ImageString
 * arg = 3  ImageStringUp
 */
static void crx_imagechar(INTERNAL_FUNCTION_PARAMETERS, int mode)
{
	zval *IM;
	crex_long X, Y, COL;
	char *C;
	size_t C_len;
	gdImagePtr im;
	int ch = 0, col, x, y, i, l = 0;
	unsigned char *str = NULL;
	crex_object *font_obj;
	crex_long font_int;
	gdFontPtr font;

	CREX_PARSE_PARAMETERS_START(6, 6)
		C_PARAM_OBJECT_OF_CLASS(IM, gd_image_ce)
		C_PARAM_OBJ_OF_CLASS_OR_LONG(font_obj, gd_font_ce, font_int)
		C_PARAM_LONG(X)
		C_PARAM_LONG(Y)
		C_PARAM_STRING(C, C_len)
		C_PARAM_LONG(COL)
	CREX_PARSE_PARAMETERS_END();

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	col = COL;

	if (mode < 2) {
		ch = (int)((unsigned char)*C);
	} else {
		str = (unsigned char *) estrndup(C, C_len);
		l = strlen((char *)str);
	}

	y = Y;
	x = X;

	font = crx_find_gd_font(font_obj, font_int);

	switch (mode) {
		case 0:
			gdImageChar(im, font, x, y, ch, col);
			break;
		case 1:
			crx_gdimagecharup(im, font, x, y, ch, col);
			break;
		case 2:
			for (i = 0; (i < l); i++) {
				gdImageChar(im, font, x, y, (int) ((unsigned char) str[i]), col);
				x += font->w;
			}
			break;
		case 3: {
			for (i = 0; (i < l); i++) {
				/* crx_gdimagecharup(im, font, x, y, (int) str[i], col); */
				gdImageCharUp(im, font, x, y, (int) str[i], col);
				y -= font->w;
			}
			break;
		}
	}
	if (str) {
		efree(str);
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Draw a character */
CRX_FUNCTION(imagechar)
{
	crx_imagechar(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ Draw a character rotated 90 degrees counter-clockwise */
CRX_FUNCTION(imagecharup)
{
	crx_imagechar(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ Draw a string horizontally */
CRX_FUNCTION(imagestring)
{
	crx_imagechar(INTERNAL_FUNCTION_PARAM_PASSTHRU, 2);
}
/* }}} */

/* {{{ Draw a string vertically - rotated 90 degrees counter-clockwise */
CRX_FUNCTION(imagestringup)
{
	crx_imagechar(INTERNAL_FUNCTION_PARAM_PASSTHRU, 3);
}
/* }}} */

/* {{{ Copy part of an image */
CRX_FUNCTION(imagecopy)
{
	zval *SIM, *DIM;
	crex_long SX, SY, SW, SH, DX, DY;
	gdImagePtr im_dst, im_src;
	int srcH, srcW, srcY, srcX, dstY, dstX;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OOllllll", &DIM, gd_image_ce, &SIM, gd_image_ce, &DX, &DY, &SX, &SY, &SW, &SH) == FAILURE) {
		RETURN_THROWS();
	}

	im_dst = crx_gd_libgdimageptr_from_zval_p(DIM);
	im_src = crx_gd_libgdimageptr_from_zval_p(SIM);

	srcX = SX;
	srcY = SY;
	srcH = SH;
	srcW = SW;
	dstX = DX;
	dstY = DY;

	gdImageCopy(im_dst, im_src, dstX, dstY, srcX, srcY, srcW, srcH);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Merge one part of an image with another */
CRX_FUNCTION(imagecopymerge)
{
	zval *SIM, *DIM;
	crex_long SX, SY, SW, SH, DX, DY, PCT;
	gdImagePtr im_dst, im_src;
	int srcH, srcW, srcY, srcX, dstY, dstX, pct;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OOlllllll", &DIM, gd_image_ce, &SIM, gd_image_ce, &DX, &DY, &SX, &SY, &SW, &SH, &PCT) == FAILURE) {
		RETURN_THROWS();
	}

	im_src = crx_gd_libgdimageptr_from_zval_p(SIM);
	im_dst = crx_gd_libgdimageptr_from_zval_p(DIM);

	srcX = SX;
	srcY = SY;
	srcH = SH;
	srcW = SW;
	dstX = DX;
	dstY = DY;
	pct  = PCT;

	gdImageCopyMerge(im_dst, im_src, dstX, dstY, srcX, srcY, srcW, srcH, pct);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Merge one part of an image with another */
CRX_FUNCTION(imagecopymergegray)
{
	zval *SIM, *DIM;
	crex_long SX, SY, SW, SH, DX, DY, PCT;
	gdImagePtr im_dst, im_src;
	int srcH, srcW, srcY, srcX, dstY, dstX, pct;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OOlllllll", &DIM, gd_image_ce, &SIM, gd_image_ce, &DX, &DY, &SX, &SY, &SW, &SH, &PCT) == FAILURE) {
		RETURN_THROWS();
	}

	im_src = crx_gd_libgdimageptr_from_zval_p(SIM);
	im_dst = crx_gd_libgdimageptr_from_zval_p(DIM);

	srcX = SX;
	srcY = SY;
	srcH = SH;
	srcW = SW;
	dstX = DX;
	dstY = DY;
	pct  = PCT;

	gdImageCopyMergeGray(im_dst, im_src, dstX, dstY, srcX, srcY, srcW, srcH, pct);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Copy and resize part of an image */
CRX_FUNCTION(imagecopyresized)
{
	zval *SIM, *DIM;
	crex_long SX, SY, SW, SH, DX, DY, DW, DH;
	gdImagePtr im_dst, im_src;
	int srcH, srcW, dstH, dstW, srcY, srcX, dstY, dstX;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OOllllllll", &DIM, gd_image_ce, &SIM, gd_image_ce, &DX, &DY, &SX, &SY, &DW, &DH, &SW, &SH) == FAILURE) {
		RETURN_THROWS();
	}

	im_src = crx_gd_libgdimageptr_from_zval_p(SIM);
	im_dst = crx_gd_libgdimageptr_from_zval_p(DIM);

	srcX = SX;
	srcY = SY;
	srcH = SH;
	srcW = SW;
	dstX = DX;
	dstY = DY;
	dstH = DH;
	dstW = DW;

	if (dstW <= 0) {
		crex_argument_value_error(7, "must be greater than 0");
		RETURN_THROWS();
	}

	if (dstH <= 0) {
		crex_argument_value_error(8, "must be greater than 0");
		RETURN_THROWS();
	}

	if (srcW <= 0) {
		crex_argument_value_error(9, "must be greater than 0");
		RETURN_THROWS();
	}

	if (srcH <= 0) {
		crex_argument_value_error(10, "must be greater than 0");
		RETURN_THROWS();
	}

	gdImageCopyResized(im_dst, im_src, dstX, dstY, srcX, srcY, dstW, dstH, srcW, srcH);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Get image width */
CRX_FUNCTION(imagesx)
{
	zval *IM;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &IM, gd_image_ce) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	RETURN_LONG(gdImageSX(im));
}
/* }}} */

/* {{{ Get image height */
CRX_FUNCTION(imagesy)
{
	zval *IM;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &IM, gd_image_ce) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	RETURN_LONG(gdImageSY(im));
}
/* }}} */

/* {{{ Set the clipping rectangle. */
CRX_FUNCTION(imagesetclip)
{
	zval *im_zval;
	gdImagePtr im;
	crex_long x1, y1, x2, y2;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ollll", &im_zval, gd_image_ce, &x1, &y1, &x2, &y2) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(im_zval);

	gdImageSetClip(im, x1, y1, x2, y2);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Get the clipping rectangle. */
CRX_FUNCTION(imagegetclip)
{
	zval *im_zval;
	gdImagePtr im;
	int x1, y1, x2, y2;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &im_zval, gd_image_ce) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(im_zval);

	gdImageGetClip(im, &x1, &y1, &x2, &y2);

	array_init(return_value);
	add_next_index_long(return_value, x1);
	add_next_index_long(return_value, y1);
	add_next_index_long(return_value, x2);
	add_next_index_long(return_value, y2);
}
/* }}} */

#define TTFTEXT_DRAW 0
#define TTFTEXT_BBOX 1

#ifdef HAVE_GD_FREETYPE
/* {{{ Give the bounding box of a text using fonts via freetype2 */
CRX_FUNCTION(imageftbbox)
{
	crx_imagettftext_common(INTERNAL_FUNCTION_PARAM_PASSTHRU, TTFTEXT_BBOX);
}
/* }}} */

/* {{{ Write text to the image using fonts via freetype2 */
CRX_FUNCTION(imagefttext)
{
	crx_imagettftext_common(INTERNAL_FUNCTION_PARAM_PASSTHRU, TTFTEXT_DRAW);
}
/* }}} */

/* {{{ crx_imagettftext_common */
static void crx_imagettftext_common(INTERNAL_FUNCTION_PARAMETERS, int mode)
{
	zval *IM, *EXT = NULL;
	gdImagePtr im=NULL;
	crex_long col = -1, x = 0, y = 0;
	size_t str_len, fontname_len;
	int i, brect[8];
	double ptsize, angle;
	char *str = NULL, *fontname = NULL;
	char *error = NULL;
	gdFTStringExtra strex = {0};

	if (mode == TTFTEXT_BBOX) {
		if (crex_parse_parameters(CREX_NUM_ARGS(), "ddss|a", &ptsize, &angle, &fontname, &fontname_len, &str, &str_len, &EXT) == FAILURE) {
			RETURN_THROWS();
		}
	} else {
		if (crex_parse_parameters(CREX_NUM_ARGS(), "Oddlllss|a", &IM, gd_image_ce, &ptsize, &angle, &x, &y, &col, &fontname, &fontname_len, &str, &str_len, &EXT) == FAILURE) {
			RETURN_THROWS();
		}
		im = crx_gd_libgdimageptr_from_zval_p(IM);
	}

	/* convert angle to radians */
	angle = angle * (M_PI/180);

	if (EXT) {	/* parse extended info */
		zval *item;
		crex_string *key;

		/* walk the assoc array */
		if (!HT_IS_PACKED(C_ARRVAL_P(EXT))) {
			CREX_HASH_MAP_FOREACH_STR_KEY_VAL(C_ARRVAL_P(EXT), key, item) {
				if (key == NULL) {
					continue;
				}
				if (crex_string_equals_literal(key, "linespacing")) {
					strex.flags |= gdFTEX_LINESPACE;
					strex.linespacing = zval_get_double(item);
				}
			} CREX_HASH_FOREACH_END();
		}
	}

#ifdef VIRTUAL_DIR
	{
		char tmp_font_path[MAXPATHLEN];

		if (!VCWD_REALPATH(fontname, tmp_font_path)) {
			fontname = NULL;
		}
	}
#endif /* VIRTUAL_DIR */

	CRX_GD_CHECK_OPEN_BASEDIR(fontname, "Invalid font filename");

	if (EXT) {
		error = gdImageStringFTEx(im, brect, col, fontname, ptsize, angle, x, y, str, &strex);
	} else {
		error = gdImageStringFT(im, brect, col, fontname, ptsize, angle, x, y, str);
	}

	if (error) {
		crx_error_docref(NULL, E_WARNING, "%s", error);
		RETURN_FALSE;
	}

	array_init(return_value);

	/* return array with the text's bounding box */
	for (i = 0; i < 8; i++) {
		add_next_index_long(return_value, brect[i]);
	}
}
/* }}} */
#endif /* HAVE_GD_FREETYPE */

/* Section Filters */
#define CRX_GD_SINGLE_RES	\
	zval *SIM;	\
	gdImagePtr im_src;	\
	if (crex_parse_parameters(1, "O", &SIM, gd_image_ce) == FAILURE) {	\
		RETURN_THROWS();	\
	}	\
	im_src = crx_gd_libgdimageptr_from_zval_p(SIM);

static void crx_image_filter_negate(INTERNAL_FUNCTION_PARAMETERS)
{
	CRX_GD_SINGLE_RES

	if (gdImageNegate(im_src) == 1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void crx_image_filter_grayscale(INTERNAL_FUNCTION_PARAMETERS)
{
	CRX_GD_SINGLE_RES

	if (gdImageGrayScale(im_src) == 1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void crx_image_filter_brightness(INTERNAL_FUNCTION_PARAMETERS)
{
	zval *SIM;
	gdImagePtr im_src;
	crex_long brightness, tmp;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oll", &SIM, gd_image_ce, &tmp, &brightness) == FAILURE) {
		RETURN_THROWS();
	}

	im_src = crx_gd_libgdimageptr_from_zval_p(SIM);

	if (gdImageBrightness(im_src, (int)brightness) == 1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void crx_image_filter_contrast(INTERNAL_FUNCTION_PARAMETERS)
{
	zval *SIM;
	gdImagePtr im_src;
	crex_long contrast, tmp;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oll", &SIM, gd_image_ce, &tmp, &contrast) == FAILURE) {
		RETURN_THROWS();
	}

	im_src = crx_gd_libgdimageptr_from_zval_p(SIM);

	if (gdImageContrast(im_src, (int)contrast) == 1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void crx_image_filter_colorize(INTERNAL_FUNCTION_PARAMETERS)
{
	zval *SIM;
	gdImagePtr im_src;
	crex_long r,g,b,tmp;
	crex_long a = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ollll|l", &SIM, gd_image_ce, &tmp, &r, &g, &b, &a) == FAILURE) {
		RETURN_THROWS();
	}

	im_src = crx_gd_libgdimageptr_from_zval_p(SIM);

	if (gdImageColor(im_src, (int) r, (int) g, (int) b, (int) a) == 1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void crx_image_filter_edgedetect(INTERNAL_FUNCTION_PARAMETERS)
{
	CRX_GD_SINGLE_RES

	if (gdImageEdgeDetectQuick(im_src) == 1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void crx_image_filter_emboss(INTERNAL_FUNCTION_PARAMETERS)
{
	CRX_GD_SINGLE_RES

	if (gdImageEmboss(im_src) == 1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void crx_image_filter_gaussian_blur(INTERNAL_FUNCTION_PARAMETERS)
{
	CRX_GD_SINGLE_RES

	if (gdImageGaussianBlur(im_src) == 1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void crx_image_filter_selective_blur(INTERNAL_FUNCTION_PARAMETERS)
{
	CRX_GD_SINGLE_RES

	if (gdImageSelectiveBlur(im_src) == 1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void crx_image_filter_mean_removal(INTERNAL_FUNCTION_PARAMETERS)
{
	CRX_GD_SINGLE_RES

	if (gdImageMeanRemoval(im_src) == 1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void crx_image_filter_smooth(INTERNAL_FUNCTION_PARAMETERS)
{
	zval *SIM;
	crex_long tmp;
	gdImagePtr im_src;
	double weight;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Old", &SIM, gd_image_ce, &tmp, &weight) == FAILURE) {
		RETURN_THROWS();
	}

	im_src = crx_gd_libgdimageptr_from_zval_p(SIM);

	if (gdImageSmooth(im_src, (float)weight)==1) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void crx_image_filter_pixelate(INTERNAL_FUNCTION_PARAMETERS)
{
	zval *IM;
	gdImagePtr im;
	crex_long tmp, blocksize;
	bool mode = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oll|b", &IM, gd_image_ce, &tmp, &blocksize, &mode) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	if (gdImagePixelate(im, (int) blocksize, (const unsigned int) mode)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

static void crx_image_filter_scatter(INTERNAL_FUNCTION_PARAMETERS)
{
	zval *IM;
	zval *hash_colors = NULL;
	gdImagePtr im;
	crex_long tmp;
	crex_long scatter_sub, scatter_plus;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Olll|a", &IM, gd_image_ce, &tmp, &scatter_sub, &scatter_plus, &hash_colors) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	if (hash_colors) {
		uint32_t i = 0;
		uint32_t num_colors = crex_hash_num_elements(C_ARRVAL_P(hash_colors));
		zval *color;
		int *colors;

		if (num_colors == 0) {
			RETURN_BOOL(gdImageScatter(im, (int)scatter_sub, (int)scatter_plus));
		}

		colors = emalloc(num_colors * sizeof(int));

		CREX_HASH_FOREACH_VAL(C_ARRVAL_P(hash_colors), color) {
			*(colors + i++) = (int) zval_get_long(color);
		} CREX_HASH_FOREACH_END();

		RETVAL_BOOL(gdImageScatterColor(im, (int)scatter_sub, (int)scatter_plus, colors, num_colors));

		efree(colors);
	} else {
		RETURN_BOOL(gdImageScatter(im, (int) scatter_sub, (int) scatter_plus));
	}
}

/* {{{ Applies Filter an image using a custom angle */
CRX_FUNCTION(imagefilter)
{
	zval *tmp;

	typedef void (*image_filter)(INTERNAL_FUNCTION_PARAMETERS);
	crex_long filtertype;
	image_filter filters[] =
	{
		crx_image_filter_negate ,
		crx_image_filter_grayscale,
		crx_image_filter_brightness,
		crx_image_filter_contrast,
		crx_image_filter_colorize,
		crx_image_filter_edgedetect,
		crx_image_filter_emboss,
		crx_image_filter_gaussian_blur,
		crx_image_filter_selective_blur,
		crx_image_filter_mean_removal,
		crx_image_filter_smooth,
		crx_image_filter_pixelate,
		crx_image_filter_scatter
	};

	if (CREX_NUM_ARGS() < 2 || CREX_NUM_ARGS() > IMAGE_FILTER_MAX_ARGS) {
		WRONG_PARAM_COUNT;
	} else if (crex_parse_parameters(2, "Ol", &tmp, gd_image_ce, &filtertype) == FAILURE) {
		RETURN_THROWS();
	}

	if (filtertype >= 0 && filtertype <= IMAGE_FILTER_MAX) {
		filters[filtertype](INTERNAL_FUNCTION_PARAM_PASSTHRU);
	}
}
/* }}} */

/* {{{ Apply a 3x3 convolution matrix, using coefficient div and offset */
CRX_FUNCTION(imageconvolution)
{
	zval *SIM, *hash_matrix;
	zval *var = NULL, *var2 = NULL;
	gdImagePtr im_src = NULL;
	double div, offset;
	int nelem, i, j, res;
	float matrix[3][3] = {{0,0,0}, {0,0,0}, {0,0,0}};

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oadd", &SIM, gd_image_ce, &hash_matrix, &div, &offset) == FAILURE) {
		RETURN_THROWS();
	}

	im_src = crx_gd_libgdimageptr_from_zval_p(SIM);

	nelem = crex_hash_num_elements(C_ARRVAL_P(hash_matrix));
	if (nelem != 3) {
		crex_argument_value_error(2, "must be a 3x3 array");
		RETURN_THROWS();
	}

	for (i=0; i<3; i++) {
		if ((var = crex_hash_index_find(C_ARRVAL_P(hash_matrix), (i))) != NULL && C_TYPE_P(var) == IS_ARRAY) {
			if (crex_hash_num_elements(C_ARRVAL_P(var)) != 3 ) {
				crex_argument_value_error(2, "must be a 3x3 array, matrix[%d] only has %d elements", i, crex_hash_num_elements(C_ARRVAL_P(var)));
				RETURN_THROWS();
			}

			for (j=0; j<3; j++) {
				if ((var2 = crex_hash_index_find(C_ARRVAL_P(var), j)) != NULL) {
					matrix[i][j] = (float) zval_get_double(var2);
				} else {
					crex_argument_value_error(2, "must be a 3x3 array, matrix[%d][%d] cannot be found (missing integer key)", i, j);
					RETURN_THROWS();
				}
			}
		}
	}
	res = gdImageConvolution(im_src, matrix, (float)div, (float)offset);

	if (res) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */
/* End section: Filters */

/* {{{ Flip an image (in place) horizontally, vertically or both directions. */
CRX_FUNCTION(imageflip)
{
	zval *IM;
	crex_long mode;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ol", &IM, gd_image_ce, &mode) == FAILURE)  {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	switch (mode) {
		case CRX_GD_FLIP_VERTICAL:
			gdImageFlipVertical(im);
			break;

		case CRX_GD_FLIP_HORIZONTAL:
			gdImageFlipHorizontal(im);
			break;

		case CRX_GD_FLIP_BOTH:
			gdImageFlipBoth(im);
			break;

		default:
			crex_argument_value_error(2, "must be one of IMG_FLIP_VERTICAL, IMG_FLIP_HORIZONTAL, or IMG_FLIP_BOTH");
			RETURN_THROWS();
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Should antialiased functions used or not*/
CRX_FUNCTION(imageantialias)
{
	zval *IM;
	bool alias;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ob", &IM, gd_image_ce, &alias) == FAILURE) {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);
	if (im->trueColor) {
		im->AA = alias;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Crop an image using the given coordinates and size, x, y, width and height. */
CRX_FUNCTION(imagecrop)
{
	zval *IM;
	gdImagePtr im;
	gdImagePtr im_crop;
	gdRect rect;
	zval *z_rect;
	zval *tmp;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oa", &IM, gd_image_ce, &z_rect) == FAILURE)  {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	if ((tmp = crex_hash_str_find(C_ARRVAL_P(z_rect), "x", sizeof("x") -1)) != NULL) {
		rect.x = zval_get_long(tmp);
	} else {
		crex_argument_value_error(2, "must have an \"x\" key");
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_ARRVAL_P(z_rect), "y", sizeof("y") - 1)) != NULL) {
		rect.y = zval_get_long(tmp);
	} else {
		crex_argument_value_error(2, "must have a \"y\" key");
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_ARRVAL_P(z_rect), "width", sizeof("width") - 1)) != NULL) {
		rect.width = zval_get_long(tmp);
	} else {
		crex_argument_value_error(2, "must have a \"width\" key");
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_ARRVAL_P(z_rect), "height", sizeof("height") - 1)) != NULL) {
		rect.height = zval_get_long(tmp);
	} else {
		crex_argument_value_error(2, "must have a \"height\" key");
		RETURN_THROWS();
	}

	im_crop = gdImageCrop(im, &rect);

	if (im_crop == NULL) {
		RETURN_FALSE;
	}

	crx_gd_assign_libgdimageptr_as_extgdimage(return_value, im_crop);
}
/* }}} */

/* {{{ Crop an image automatically using one of the available modes. */
CRX_FUNCTION(imagecropauto)
{
	zval *IM;
	crex_long mode = GD_CROP_DEFAULT;
	crex_long color = -1;
	double threshold = 0.5f;
	gdImagePtr im;
	gdImagePtr im_crop;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O|ldl", &IM, gd_image_ce, &mode, &threshold, &color) == FAILURE)  {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	switch (mode) {
		case GD_CROP_DEFAULT:
		case GD_CROP_TRANSPARENT:
		case GD_CROP_BLACK:
		case GD_CROP_WHITE:
		case GD_CROP_SIDES:
			im_crop = gdImageCropAuto(im, mode);
			break;

		case GD_CROP_THRESHOLD:
			if (color < 0 || (!gdImageTrueColor(im) && color >= gdImageColorsTotal(im))) {
				crex_argument_value_error(4, "must be greater than or equal to 0 when using the threshold mode");
				RETURN_THROWS();
			}
			im_crop = gdImageCropThreshold(im, color, (float) threshold);
			break;

		default:
			crex_argument_value_error(2, "must be a valid mode");
			RETURN_THROWS();
	}

	if (im_crop == NULL) {
		RETURN_FALSE;
	}

	crx_gd_assign_libgdimageptr_as_extgdimage(return_value, im_crop);
}
/* }}} */

/* {{{ Scale an image using the given new width and height. */
CRX_FUNCTION(imagescale)
{
	zval *IM;
	gdImagePtr im;
	gdImagePtr im_scaled = NULL;
	int new_width, new_height;
	crex_long tmp_w, tmp_h=-1, tmp_m = GD_BILINEAR_FIXED;
	gdInterpolationMethod method, old_method;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ol|ll", &IM, gd_image_ce, &tmp_w, &tmp_h, &tmp_m) == FAILURE)  {
		RETURN_THROWS();
	}
	method = tmp_m;

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	if (tmp_h < 0 || tmp_w < 0) {
		/* preserve ratio */
		long src_x, src_y;

		src_x = gdImageSX(im);
		src_y = gdImageSY(im);

		if (src_x && tmp_h < 0) {
			tmp_h = tmp_w * src_y / src_x;
		}
		if (src_y && tmp_w < 0) {
			tmp_w = tmp_h * src_x / src_y;
		}
	}

	if (tmp_h <= 0 || tmp_h > INT_MAX || tmp_w <= 0 || tmp_w > INT_MAX) {
		RETURN_FALSE;
	}

	new_width = tmp_w;
	new_height = tmp_h;

	/* gdImageGetInterpolationMethod() is only available as of GD 2.1.1 */
	old_method = im->interpolation_id;
	if (gdImageSetInterpolationMethod(im, method)) {
		im_scaled = gdImageScale(im, new_width, new_height);
	}
	gdImageSetInterpolationMethod(im, old_method);

	if (im_scaled == NULL) {
		RETURN_FALSE;
	}

	crx_gd_assign_libgdimageptr_as_extgdimage(return_value, im_scaled);
}
/* }}} */

/* {{{ Return an image containing the affine tramsformed src image, using an optional clipping area */
CRX_FUNCTION(imageaffine)
{
	zval *IM;
	gdImagePtr src;
	gdImagePtr dst;
	gdRect rect;
	gdRectPtr pRect = NULL;
	zval *z_rect = NULL;
	zval *z_affine;
	zval *tmp;
	double affine[6];
	int i, nelems;
	zval *zval_affine_elem = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oa|a!", &IM, gd_image_ce, &z_affine, &z_rect) == FAILURE)  {
		RETURN_THROWS();
	}

	src = crx_gd_libgdimageptr_from_zval_p(IM);

	if ((nelems = crex_hash_num_elements(C_ARRVAL_P(z_affine))) != 6) {
		crex_argument_value_error(2, "must have 6 elements");
		RETURN_THROWS();
	}

	for (i = 0; i < nelems; i++) {
		if ((zval_affine_elem = crex_hash_index_find(C_ARRVAL_P(z_affine), i)) != NULL) {
			switch (C_TYPE_P(zval_affine_elem)) {
				case IS_LONG:
					affine[i]  = C_LVAL_P(zval_affine_elem);
					break;
				case IS_DOUBLE:
					affine[i] = C_DVAL_P(zval_affine_elem);
					break;
				case IS_STRING:
					affine[i] = zval_get_double(zval_affine_elem);
					break;
				default:
					crex_argument_type_error(3, "contains invalid type for element %i", i);
					RETURN_THROWS();
			}
		}
	}

	if (z_rect != NULL) {
		if ((tmp = crex_hash_str_find(C_ARRVAL_P(z_rect), "x", sizeof("x") - 1)) != NULL) {
			rect.x = zval_get_long(tmp);
		} else {
			crex_argument_value_error(3, "must have an \"x\" key");
			RETURN_THROWS();
		}

		if ((tmp = crex_hash_str_find(C_ARRVAL_P(z_rect), "y", sizeof("y") - 1)) != NULL) {
			rect.y = zval_get_long(tmp);
		} else {
			crex_argument_value_error(3, "must have a \"y\" key");
			RETURN_THROWS();
		}

		if ((tmp = crex_hash_str_find(C_ARRVAL_P(z_rect), "width", sizeof("width") - 1)) != NULL) {
			rect.width = zval_get_long(tmp);
		} else {
			crex_argument_value_error(3, "must have a \"width\" key");
			RETURN_THROWS();
		}

		if ((tmp = crex_hash_str_find(C_ARRVAL_P(z_rect), "height", sizeof("height") - 1)) != NULL) {
			rect.height = zval_get_long(tmp);
		} else {
			crex_argument_value_error(3, "must have a \"height\" key");
			RETURN_THROWS();
		}
		pRect = &rect;
	}

	if (gdTransformAffineGetImage(&dst, src, pRect, affine) != GD_TRUE) {
		RETURN_FALSE;
	}

	if (dst == NULL) {
		RETURN_FALSE;
	}

	crx_gd_assign_libgdimageptr_as_extgdimage(return_value, dst);
}
/* }}} */

/* {{{ Return an image containing the affine tramsformed src image, using an optional clipping area */
CRX_FUNCTION(imageaffinematrixget)
{
	double affine[6];
	crex_long type;
	zval *options = NULL;
	zval *tmp;
	int res = GD_FALSE, i;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "lz", &type, &options) == FAILURE)  {
		RETURN_THROWS();
	}

	switch((gdAffineStandardMatrix)type) {
		case GD_AFFINE_TRANSLATE:
		case GD_AFFINE_SCALE: {
			double x, y;
			if (C_TYPE_P(options) != IS_ARRAY) {
				crex_argument_type_error(1, "must be of type array when using translate or scale");
				RETURN_THROWS();
			}

			if ((tmp = crex_hash_str_find(C_ARRVAL_P(options), "x", sizeof("x") - 1)) != NULL) {
				x = zval_get_double(tmp);
			} else {
				crex_argument_value_error(2, "must have an \"x\" key");
				RETURN_THROWS();
			}

			if ((tmp = crex_hash_str_find(C_ARRVAL_P(options), "y", sizeof("y") - 1)) != NULL) {
				y = zval_get_double(tmp);
			} else {
				crex_argument_value_error(2, "must have a \"y\" key");
				RETURN_THROWS();
			}

			if (type == GD_AFFINE_TRANSLATE) {
				res = gdAffineTranslate(affine, x, y);
			} else {
				res = gdAffineScale(affine, x, y);
			}
			break;
		}

		case GD_AFFINE_ROTATE:
		case GD_AFFINE_SHEAR_HORIZONTAL:
		case GD_AFFINE_SHEAR_VERTICAL: {
			double angle;

			angle = zval_get_double(options);

			if (type == GD_AFFINE_SHEAR_HORIZONTAL) {
				res = gdAffineShearHorizontal(affine, angle);
			} else if (type == GD_AFFINE_SHEAR_VERTICAL) {
				res = gdAffineShearVertical(affine, angle);
			} else {
				res = gdAffineRotate(affine, angle);
			}
			break;
		}

		default:
			crex_argument_value_error(1, "must be a valid element type");
			RETURN_THROWS();
	}

	if (res == GD_FALSE) {
		RETURN_FALSE;
	} else {
		array_init(return_value);
		for (i = 0; i < 6; i++) {
			add_index_double(return_value, i, affine[i]);
		}
	}
} /* }}} */

/* {{{ Concat two matrices (as in doing many ops in one go) */
CRX_FUNCTION(imageaffinematrixconcat)
{
	double m1[6];
	double m2[6];
	double mr[6];

	zval *tmp;
	zval *z_m1;
	zval *z_m2;
	int i;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "aa", &z_m1, &z_m2) == FAILURE)  {
		RETURN_THROWS();
	}

	if (crex_hash_num_elements(C_ARRVAL_P(z_m1)) != 6) {
		crex_argument_value_error(1, "must have 6 elements");
		RETURN_THROWS();
	}

	if (crex_hash_num_elements(C_ARRVAL_P(z_m2)) != 6) {
		crex_argument_value_error(1, "must have 6 elements");
		RETURN_THROWS();
	}

	for (i = 0; i < 6; i++) {
		if ((tmp = crex_hash_index_find(C_ARRVAL_P(z_m1), i)) != NULL) {
			switch (C_TYPE_P(tmp)) {
				case IS_LONG:
					m1[i]  = C_LVAL_P(tmp);
					break;
				case IS_DOUBLE:
					m1[i] = C_DVAL_P(tmp);
					break;
				case IS_STRING:
					m1[i] = zval_get_double(tmp);
					break;
				default:
					crex_argument_type_error(1, "contains invalid type for element %i", i);
					RETURN_THROWS();
			}
		}

		if ((tmp = crex_hash_index_find(C_ARRVAL_P(z_m2), i)) != NULL) {
			switch (C_TYPE_P(tmp)) {
				case IS_LONG:
					m2[i]  = C_LVAL_P(tmp);
					break;
				case IS_DOUBLE:
					m2[i] = C_DVAL_P(tmp);
					break;
				case IS_STRING:
					m2[i] = zval_get_double(tmp);
					break;
				default:
					crex_argument_type_error(2, "contains invalid type for element %i", i);
					RETURN_THROWS();
			}
		}
	}

	if (gdAffineConcat (mr, m1, m2) != GD_TRUE) {
		RETURN_FALSE;
	}

	array_init(return_value);
	for (i = 0; i < 6; i++) {
		add_index_double(return_value, i, mr[i]);
	}
} /* }}} */

/* {{{ Get the default interpolation method. */
CRX_FUNCTION(imagegetinterpolation)
{
	zval *IM;
	gdImagePtr im;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &IM, gd_image_ce) == FAILURE)  {
		RETURN_THROWS();
	}
	im = crx_gd_libgdimageptr_from_zval_p(IM);

#ifdef HAVE_GD_GET_INTERPOLATION
	RETURN_LONG(gdImageGetInterpolationMethod(im));
#else
	RETURN_LONG(im->interpolation_id);
#endif
}
/* }}} */

/* {{{ Set the default interpolation method, passing -1 or 0 sets it to the libgd default (bilinear). */
CRX_FUNCTION(imagesetinterpolation)
{
	zval *IM;
	gdImagePtr im;
	crex_long method = GD_BILINEAR_FIXED;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O|l", &IM, gd_image_ce, &method) == FAILURE)  {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	if (method == -1) {
		 method = GD_BILINEAR_FIXED;
	}
	RETURN_BOOL(gdImageSetInterpolationMethod(im, (gdInterpolationMethod) method));
}
/* }}} */

/* {{{ Get or set the resolution of the image in DPI. */
CRX_FUNCTION(imageresolution)
{
	zval *IM;
	gdImagePtr im;
	crex_long res_x, res_y;
	bool res_x_is_null = 1, res_y_is_null = 1;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O|l!l!", &IM, gd_image_ce, &res_x, &res_x_is_null, &res_y, &res_y_is_null) == FAILURE)  {
		RETURN_THROWS();
	}

	im = crx_gd_libgdimageptr_from_zval_p(IM);

	if (!res_x_is_null && !res_y_is_null) {
		gdImageSetResolution(im, res_x, res_y);
		RETURN_TRUE;
	} else if (!res_x_is_null && res_y_is_null) {
		gdImageSetResolution(im, res_x, res_x);
		RETURN_TRUE;
	} else if (res_x_is_null && !res_y_is_null) {
		gdImageSetResolution(im, res_y, res_y);
		RETURN_TRUE;
	}

	array_init(return_value);
	add_next_index_long(return_value, gdImageResolutionX(im));
	add_next_index_long(return_value, gdImageResolutionY(im));
}
/* }}} */


/*********************************************************
 *
 * Stream Handling
 * Formerly contained within ext/gd/gd_ctx.c and included
 * at the the top of this file
 *
 ********************************************************/

#define CTX_PUTC(c,ctx) ctx->putC(ctx, c)

static void _crx_image_output_putc(struct gdIOCtx *ctx, int c) /* {{{ */
{
	/* without the following downcast, the write will fail
	 * (i.e., will write a zero byte) for all
	 * big endian architectures:
	 */
	unsigned char ch = (unsigned char) c;
	crx_write(&ch, 1);
} /* }}} */

static int _crx_image_output_putbuf(struct gdIOCtx *ctx, const void* buf, int l) /* {{{ */
{
	return crx_write((void *)buf, l);
} /* }}} */

static void _crx_image_output_ctxfree(struct gdIOCtx *ctx) /* {{{ */
{
	efree(ctx);
} /* }}} */

static void _crx_image_stream_putc(struct gdIOCtx *ctx, int c) /* {{{ */ {
	char ch = (char) c;
	crx_stream * stream = (crx_stream *)ctx->data;
	crx_stream_write(stream, &ch, 1);
} /* }}} */

static int _crx_image_stream_putbuf(struct gdIOCtx *ctx, const void* buf, int l) /* {{{ */
{
	crx_stream * stream = (crx_stream *)ctx->data;
	return crx_stream_write(stream, (void *)buf, l);
} /* }}} */

static void _crx_image_stream_ctxfree(struct gdIOCtx *ctx) /* {{{ */
{
	if(ctx->data) {
		ctx->data = NULL;
	}
	efree(ctx);
} /* }}} */

static void _crx_image_stream_ctxfreeandclose(struct gdIOCtx *ctx) /* {{{ */
{
	if(ctx->data) {
		crx_stream_close((crx_stream *) ctx->data);
		ctx->data = NULL;
	}
	efree(ctx);
} /* }}} */

static gdIOCtx *create_stream_context_from_zval(zval *to_zval) {
	crx_stream *stream;
	int close_stream = 1;

	if (C_TYPE_P(to_zval) == IS_RESOURCE) {
		crx_stream_from_zval_no_verify(stream, to_zval);
		if (stream == NULL) {
			return NULL;
		}
		close_stream = 0;
	} else if (C_TYPE_P(to_zval) == IS_STRING) {
		if (CHECK_ZVAL_NULL_PATH(to_zval)) {
			crex_argument_type_error(2, "must not contain null bytes");
			return NULL;
		}

		stream = crx_stream_open_wrapper(C_STRVAL_P(to_zval), "wb", REPORT_ERRORS|IGNORE_PATH, NULL);
		if (stream == NULL) {
			return NULL;
		}
	} else {
		crex_argument_type_error(2, "must be a file name or a stream resource, %s given", crex_zval_value_name(to_zval));
		return NULL;
	}

	return create_stream_context(stream, close_stream);
}

static gdIOCtx *create_stream_context(crx_stream *stream, int close_stream) {
	gdIOCtx *ctx = ecalloc(1, sizeof(gdIOCtx));

	ctx->putC = _crx_image_stream_putc;
	ctx->putBuf = _crx_image_stream_putbuf;
	if (close_stream) {
		ctx->gd_free = _crx_image_stream_ctxfreeandclose;
	} else {
		ctx->gd_free = _crx_image_stream_ctxfree;
	}
	ctx->data = (void *)stream;

	return ctx;
}

static gdIOCtx *create_output_context() {
	gdIOCtx *ctx = ecalloc(1, sizeof(gdIOCtx));

	ctx->putC = _crx_image_output_putc;
	ctx->putBuf = _crx_image_output_putbuf;
	ctx->gd_free = _crx_image_output_ctxfree;

	return ctx;
}

static void _crx_image_output_ctx(INTERNAL_FUNCTION_PARAMETERS, int image_type, char *tn)
{
	zval *imgind;
	crex_long quality = -1, basefilter = -1, speed = -1;
	gdImagePtr im;
	gdIOCtx *ctx = NULL;
	zval *to_zval = NULL;

	if (image_type == CRX_GDIMG_TYPE_GIF) {
		if (crex_parse_parameters(CREX_NUM_ARGS(), "O|z!", &imgind, gd_image_ce, &to_zval) == FAILURE) {
			RETURN_THROWS();
		}
	} else if (image_type == CRX_GDIMG_TYPE_PNG) {
		if (crex_parse_parameters(CREX_NUM_ARGS(), "O|z!ll", &imgind, gd_image_ce, &to_zval, &quality, &basefilter) == FAILURE) {
			RETURN_THROWS();
		}
	} else if (image_type == CRX_GDIMG_TYPE_AVIF) {
		if (crex_parse_parameters(CREX_NUM_ARGS(), "O|z!ll", &imgind, gd_image_ce, &to_zval, &quality, &speed) == FAILURE) {
			RETURN_THROWS();
		}
	} else {
		if (crex_parse_parameters(CREX_NUM_ARGS(), "O|z!l", &imgind, gd_image_ce, &to_zval, &quality) == FAILURE) {
			RETURN_THROWS();
		}
	}

	im = crx_gd_libgdimageptr_from_zval_p(imgind);

	if (to_zval != NULL) {
		ctx = create_stream_context_from_zval(to_zval);
		if (!ctx) {
			RETURN_FALSE;
		}
	} else {
		ctx = create_output_context();
	}

	switch (image_type) {
#ifdef HAVE_GD_JPG
		case CRX_GDIMG_TYPE_JPG:
			gdImageJpegCtx(im, ctx, (int) quality);
			break;
#endif
#ifdef HAVE_GD_WEBP
		case CRX_GDIMG_TYPE_WEBP:
			if (quality == -1) {
				quality = 80;
			}
			gdImageWebpCtx(im, ctx, (int) quality);
			break;
#endif
#ifdef HAVE_GD_AVIF
		case CRX_GDIMG_TYPE_AVIF:
			if (speed == -1) {
				speed = 6;
			}
			gdImageAvifCtx(im, ctx, (int) quality, (int) speed);
			break;
#endif
#ifdef HAVE_GD_PNG
		case CRX_GDIMG_TYPE_PNG:
#ifdef HAVE_GD_BUNDLED
			gdImagePngCtxEx(im, ctx, (int) quality, (int) basefilter);
#else
			gdImagePngCtxEx(im, ctx, (int) quality);
#endif
			break;
#endif
		case CRX_GDIMG_TYPE_GIF:
			gdImageGifCtx(im, ctx);
			break;
		 EMPTY_SWITCH_DEFAULT_CASE()
	}

	ctx->gd_free(ctx);

	RETURN_TRUE;
}

/* }}} */
