/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 0f8a22bff1d123313f37da400500e573baace837 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gd_info, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_imageloadfont, 0, 1, GdFont, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagesetstyle, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, style, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_imagecreatetruecolor, 0, 2, GdImage, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, height, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imageistruecolor, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagetruecolortopalette, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, dither, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, num_colors, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_imagepalettetotruecolor arginfo_imageistruecolor

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagecolormatch, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image1, GdImage, 0)
	CREX_ARG_OBJ_INFO(0, image2, GdImage, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagesetthickness, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, thickness, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagefilledellipse, 0, 6, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, center_x, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, center_y, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, color, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagefilledarc, 0, 9, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, center_x, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, center_y, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, start_angle, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, end_angle, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, color, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, style, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagealphablending, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, enable, _IS_BOOL, 0)
CREX_END_ARG_INFO()

#define arginfo_imagesavealpha arginfo_imagealphablending

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagelayereffect, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, effect, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_imagecolorallocatealpha, 0, 5, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, red, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, green, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, blue, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, alpha, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagecolorresolvealpha, 0, 5, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, red, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, green, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, blue, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, alpha, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_imagecolorclosestalpha arginfo_imagecolorresolvealpha

#define arginfo_imagecolorexactalpha arginfo_imagecolorresolvealpha

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagecopyresampled, 0, 10, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, dst_image, GdImage, 0)
	CREX_ARG_OBJ_INFO(0, src_image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, dst_x, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, dst_y, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, src_x, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, src_y, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, dst_width, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, dst_height, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, src_width, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, src_height, IS_LONG, 0)
CREX_END_ARG_INFO()

#if defined(CRX_WIN32)
CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_imagegrabwindow, 0, 1, GdImage, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, handle, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, client_area, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()
#endif

#if defined(CRX_WIN32)
CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_imagegrabscreen, 0, 0, GdImage, MAY_BE_FALSE)
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_imagerotate, 0, 3, GdImage, MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, angle, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, background_color, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagesettile, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_OBJ_INFO(0, tile, GdImage, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagesetbrush, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_OBJ_INFO(0, brush, GdImage, 0)
CREX_END_ARG_INFO()

#define arginfo_imagecreate arginfo_imagecreatetruecolor

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagetypes, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_imagecreatefromstring, 0, 1, GdImage, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
CREX_END_ARG_INFO()

#if defined(HAVE_GD_AVIF)
CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_imagecreatefromavif, 0, 1, GdImage, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_imagecreatefromgif, 0, 1, GdImage, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()

#if defined(HAVE_GD_JPG)
CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_imagecreatefromjpeg, 0, 1, GdImage, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_GD_PNG)
CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_imagecreatefrompng, 0, 1, GdImage, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_GD_WEBP)
CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_imagecreatefromwebp, 0, 1, GdImage, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#define arginfo_imagecreatefromxbm arginfo_imagecreatefromgif

#if defined(HAVE_GD_XPM)
CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_imagecreatefromxpm, 0, 1, GdImage, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#define arginfo_imagecreatefromwbmp arginfo_imagecreatefromgif

#define arginfo_imagecreatefromgd arginfo_imagecreatefromgif

#define arginfo_imagecreatefromgd2 arginfo_imagecreatefromgif

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_imagecreatefromgd2part, 0, 5, GdImage, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, y, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, height, IS_LONG, 0)
CREX_END_ARG_INFO()

#if defined(HAVE_GD_BMP)
CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_imagecreatefrombmp, 0, 1, GdImage, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_GD_TGA)
CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_imagecreatefromtga, 0, 1, GdImage, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagexbm, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, foreground_color, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

#if defined(HAVE_GD_AVIF)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imageavif, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, file, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, quality, IS_LONG, 0, "-1")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, speed, IS_LONG, 0, "-1")
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagegif, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, file, "null")
CREX_END_ARG_INFO()

#if defined(HAVE_GD_PNG)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagepng, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, file, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, quality, IS_LONG, 0, "-1")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, filters, IS_LONG, 0, "-1")
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_GD_WEBP)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagewebp, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, file, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, quality, IS_LONG, 0, "-1")
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_GD_JPG)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagejpeg, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, file, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, quality, IS_LONG, 0, "-1")
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagewbmp, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, file, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, foreground_color, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagegd, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, file, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagegd2, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, file, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, chunk_size, IS_LONG, 0, "128")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "IMG_GD2_RAW")
CREX_END_ARG_INFO()

#if defined(HAVE_GD_BMP)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagebmp, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, file, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, compressed, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()
#endif

#define arginfo_imagedestroy arginfo_imageistruecolor

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_imagecolorallocate, 0, 4, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, red, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, green, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, blue, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagepalettecopy, 0, 2, IS_VOID, 0)
	CREX_ARG_OBJ_INFO(0, dst, GdImage, 0)
	CREX_ARG_OBJ_INFO(0, src, GdImage, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_imagecolorat, 0, 3, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, y, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagecolorclosest, 0, 4, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, red, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, green, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, blue, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_imagecolorclosesthwb arginfo_imagecolorclosest

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagecolordeallocate, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, color, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_imagecolorresolve arginfo_imagecolorclosest

#define arginfo_imagecolorexact arginfo_imagecolorclosest

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagecolorset, 0, 5, IS_FALSE, 1)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, color, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, red, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, green, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, blue, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, alpha, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagecolorsforindex, 0, 2, IS_ARRAY, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, color, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagegammacorrect, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, input_gamma, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, output_gamma, IS_DOUBLE, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagesetpixel, 0, 4, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, y, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, color, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imageline, 0, 6, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, x1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, y1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, x2, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, y2, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, color, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_imagedashedline arginfo_imageline

#define arginfo_imagerectangle arginfo_imageline

#define arginfo_imagefilledrectangle arginfo_imageline

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagearc, 0, 8, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, center_x, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, center_y, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, start_angle, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, end_angle, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, color, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_imageellipse arginfo_imagefilledellipse

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagefilltoborder, 0, 5, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, y, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, border_color, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, color, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_imagefill arginfo_imagesetpixel

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagecolorstotal, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagecolortransparent, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, color, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imageinterlace, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, enable, _IS_BOOL, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagepolygon, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, points, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, num_points_or_color, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, color, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_imageopenpolygon arginfo_imagepolygon

#define arginfo_imagefilledpolygon arginfo_imagepolygon

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagefontwidth, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, font, GdFont, MAY_BE_LONG, NULL)
CREX_END_ARG_INFO()

#define arginfo_imagefontheight arginfo_imagefontwidth

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagechar, 0, 6, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, font, GdFont, MAY_BE_LONG, NULL)
	CREX_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, y, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, char, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, color, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_imagecharup arginfo_imagechar

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagestring, 0, 6, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, font, GdFont, MAY_BE_LONG, NULL)
	CREX_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, y, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, color, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_imagestringup arginfo_imagestring

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagecopy, 0, 8, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, dst_image, GdImage, 0)
	CREX_ARG_OBJ_INFO(0, src_image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, dst_x, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, dst_y, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, src_x, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, src_y, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, src_width, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, src_height, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagecopymerge, 0, 9, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, dst_image, GdImage, 0)
	CREX_ARG_OBJ_INFO(0, src_image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, dst_x, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, dst_y, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, src_x, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, src_y, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, src_width, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, src_height, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, pct, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_imagecopymergegray arginfo_imagecopymerge

#define arginfo_imagecopyresized arginfo_imagecopyresampled

#define arginfo_imagesx arginfo_imagecolorstotal

#define arginfo_imagesy arginfo_imagecolorstotal

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagesetclip, 0, 5, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, x1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, y1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, x2, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, y2, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagegetclip, 0, 1, IS_ARRAY, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
CREX_END_ARG_INFO()

#if defined(HAVE_GD_FREETYPE)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_imageftbbox, 0, 4, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, size, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, angle, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, font_filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_GD_FREETYPE)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_imagefttext, 0, 8, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, size, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, angle, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, x, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, y, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, color, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, font_filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, text, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_GD_FREETYPE)
#define arginfo_imagettfbbox arginfo_imageftbbox
#endif

#if defined(HAVE_GD_FREETYPE)
#define arginfo_imagettftext arginfo_imagefttext
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagefilter, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, filter, IS_LONG, 0)
	CREX_ARG_VARIADIC_INFO(0, args)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imageconvolution, 0, 4, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, matrix, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, divisor, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_DOUBLE, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imageflip, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_imageantialias arginfo_imagealphablending

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_imagecrop, 0, 2, GdImage, MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, rectangle, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_imagecropauto, 0, 1, GdImage, MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "IMG_CROP_DEFAULT")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, threshold, IS_DOUBLE, 0, "0.5")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, color, IS_LONG, 0, "-1")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_imagescale, 0, 2, GdImage, MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, height, IS_LONG, 0, "-1")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "IMG_BILINEAR_FIXED")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_imageaffine, 0, 2, GdImage, MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO(0, affine, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, clip, IS_ARRAY, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_imageaffinematrixget, 0, 2, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	CREX_ARG_INFO(0, options)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_imageaffinematrixconcat, 0, 2, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, matrix1, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, matrix2, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_imagegetinterpolation arginfo_imagecolorstotal

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_imagesetinterpolation, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, method, IS_LONG, 0, "IMG_BILINEAR_FIXED")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_imageresolution, 0, 1, MAY_BE_ARRAY|MAY_BE_BOOL)
	CREX_ARG_OBJ_INFO(0, image, GdImage, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, resolution_x, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, resolution_y, IS_LONG, 1, "null")
CREX_END_ARG_INFO()


CREX_FUNCTION(gd_info);
CREX_FUNCTION(imageloadfont);
CREX_FUNCTION(imagesetstyle);
CREX_FUNCTION(imagecreatetruecolor);
CREX_FUNCTION(imageistruecolor);
CREX_FUNCTION(imagetruecolortopalette);
CREX_FUNCTION(imagepalettetotruecolor);
CREX_FUNCTION(imagecolormatch);
CREX_FUNCTION(imagesetthickness);
CREX_FUNCTION(imagefilledellipse);
CREX_FUNCTION(imagefilledarc);
CREX_FUNCTION(imagealphablending);
CREX_FUNCTION(imagesavealpha);
CREX_FUNCTION(imagelayereffect);
CREX_FUNCTION(imagecolorallocatealpha);
CREX_FUNCTION(imagecolorresolvealpha);
CREX_FUNCTION(imagecolorclosestalpha);
CREX_FUNCTION(imagecolorexactalpha);
CREX_FUNCTION(imagecopyresampled);
#if defined(CRX_WIN32)
CREX_FUNCTION(imagegrabwindow);
#endif
#if defined(CRX_WIN32)
CREX_FUNCTION(imagegrabscreen);
#endif
CREX_FUNCTION(imagerotate);
CREX_FUNCTION(imagesettile);
CREX_FUNCTION(imagesetbrush);
CREX_FUNCTION(imagecreate);
CREX_FUNCTION(imagetypes);
CREX_FUNCTION(imagecreatefromstring);
#if defined(HAVE_GD_AVIF)
CREX_FUNCTION(imagecreatefromavif);
#endif
CREX_FUNCTION(imagecreatefromgif);
#if defined(HAVE_GD_JPG)
CREX_FUNCTION(imagecreatefromjpeg);
#endif
#if defined(HAVE_GD_PNG)
CREX_FUNCTION(imagecreatefrompng);
#endif
#if defined(HAVE_GD_WEBP)
CREX_FUNCTION(imagecreatefromwebp);
#endif
CREX_FUNCTION(imagecreatefromxbm);
#if defined(HAVE_GD_XPM)
CREX_FUNCTION(imagecreatefromxpm);
#endif
CREX_FUNCTION(imagecreatefromwbmp);
CREX_FUNCTION(imagecreatefromgd);
CREX_FUNCTION(imagecreatefromgd2);
CREX_FUNCTION(imagecreatefromgd2part);
#if defined(HAVE_GD_BMP)
CREX_FUNCTION(imagecreatefrombmp);
#endif
#if defined(HAVE_GD_TGA)
CREX_FUNCTION(imagecreatefromtga);
#endif
CREX_FUNCTION(imagexbm);
#if defined(HAVE_GD_AVIF)
CREX_FUNCTION(imageavif);
#endif
CREX_FUNCTION(imagegif);
#if defined(HAVE_GD_PNG)
CREX_FUNCTION(imagepng);
#endif
#if defined(HAVE_GD_WEBP)
CREX_FUNCTION(imagewebp);
#endif
#if defined(HAVE_GD_JPG)
CREX_FUNCTION(imagejpeg);
#endif
CREX_FUNCTION(imagewbmp);
CREX_FUNCTION(imagegd);
CREX_FUNCTION(imagegd2);
#if defined(HAVE_GD_BMP)
CREX_FUNCTION(imagebmp);
#endif
CREX_FUNCTION(imagedestroy);
CREX_FUNCTION(imagecolorallocate);
CREX_FUNCTION(imagepalettecopy);
CREX_FUNCTION(imagecolorat);
CREX_FUNCTION(imagecolorclosest);
CREX_FUNCTION(imagecolorclosesthwb);
CREX_FUNCTION(imagecolordeallocate);
CREX_FUNCTION(imagecolorresolve);
CREX_FUNCTION(imagecolorexact);
CREX_FUNCTION(imagecolorset);
CREX_FUNCTION(imagecolorsforindex);
CREX_FUNCTION(imagegammacorrect);
CREX_FUNCTION(imagesetpixel);
CREX_FUNCTION(imageline);
CREX_FUNCTION(imagedashedline);
CREX_FUNCTION(imagerectangle);
CREX_FUNCTION(imagefilledrectangle);
CREX_FUNCTION(imagearc);
CREX_FUNCTION(imageellipse);
CREX_FUNCTION(imagefilltoborder);
CREX_FUNCTION(imagefill);
CREX_FUNCTION(imagecolorstotal);
CREX_FUNCTION(imagecolortransparent);
CREX_FUNCTION(imageinterlace);
CREX_FUNCTION(imagepolygon);
CREX_FUNCTION(imageopenpolygon);
CREX_FUNCTION(imagefilledpolygon);
CREX_FUNCTION(imagefontwidth);
CREX_FUNCTION(imagefontheight);
CREX_FUNCTION(imagechar);
CREX_FUNCTION(imagecharup);
CREX_FUNCTION(imagestring);
CREX_FUNCTION(imagestringup);
CREX_FUNCTION(imagecopy);
CREX_FUNCTION(imagecopymerge);
CREX_FUNCTION(imagecopymergegray);
CREX_FUNCTION(imagecopyresized);
CREX_FUNCTION(imagesx);
CREX_FUNCTION(imagesy);
CREX_FUNCTION(imagesetclip);
CREX_FUNCTION(imagegetclip);
#if defined(HAVE_GD_FREETYPE)
CREX_FUNCTION(imageftbbox);
#endif
#if defined(HAVE_GD_FREETYPE)
CREX_FUNCTION(imagefttext);
#endif
CREX_FUNCTION(imagefilter);
CREX_FUNCTION(imageconvolution);
CREX_FUNCTION(imageflip);
CREX_FUNCTION(imageantialias);
CREX_FUNCTION(imagecrop);
CREX_FUNCTION(imagecropauto);
CREX_FUNCTION(imagescale);
CREX_FUNCTION(imageaffine);
CREX_FUNCTION(imageaffinematrixget);
CREX_FUNCTION(imageaffinematrixconcat);
CREX_FUNCTION(imagegetinterpolation);
CREX_FUNCTION(imagesetinterpolation);
CREX_FUNCTION(imageresolution);


static const crex_function_entry ext_functions[] = {
	CREX_FE(gd_info, arginfo_gd_info)
	CREX_FE(imageloadfont, arginfo_imageloadfont)
	CREX_FE(imagesetstyle, arginfo_imagesetstyle)
	CREX_FE(imagecreatetruecolor, arginfo_imagecreatetruecolor)
	CREX_FE(imageistruecolor, arginfo_imageistruecolor)
	CREX_FE(imagetruecolortopalette, arginfo_imagetruecolortopalette)
	CREX_FE(imagepalettetotruecolor, arginfo_imagepalettetotruecolor)
	CREX_FE(imagecolormatch, arginfo_imagecolormatch)
	CREX_FE(imagesetthickness, arginfo_imagesetthickness)
	CREX_FE(imagefilledellipse, arginfo_imagefilledellipse)
	CREX_FE(imagefilledarc, arginfo_imagefilledarc)
	CREX_FE(imagealphablending, arginfo_imagealphablending)
	CREX_FE(imagesavealpha, arginfo_imagesavealpha)
	CREX_FE(imagelayereffect, arginfo_imagelayereffect)
	CREX_FE(imagecolorallocatealpha, arginfo_imagecolorallocatealpha)
	CREX_FE(imagecolorresolvealpha, arginfo_imagecolorresolvealpha)
	CREX_FE(imagecolorclosestalpha, arginfo_imagecolorclosestalpha)
	CREX_FE(imagecolorexactalpha, arginfo_imagecolorexactalpha)
	CREX_FE(imagecopyresampled, arginfo_imagecopyresampled)
#if defined(CRX_WIN32)
	CREX_FE(imagegrabwindow, arginfo_imagegrabwindow)
#endif
#if defined(CRX_WIN32)
	CREX_FE(imagegrabscreen, arginfo_imagegrabscreen)
#endif
	CREX_FE(imagerotate, arginfo_imagerotate)
	CREX_FE(imagesettile, arginfo_imagesettile)
	CREX_FE(imagesetbrush, arginfo_imagesetbrush)
	CREX_FE(imagecreate, arginfo_imagecreate)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(imagetypes, arginfo_imagetypes)
	CREX_FE(imagecreatefromstring, arginfo_imagecreatefromstring)
#if defined(HAVE_GD_AVIF)
	CREX_FE(imagecreatefromavif, arginfo_imagecreatefromavif)
#endif
	CREX_FE(imagecreatefromgif, arginfo_imagecreatefromgif)
#if defined(HAVE_GD_JPG)
	CREX_FE(imagecreatefromjpeg, arginfo_imagecreatefromjpeg)
#endif
#if defined(HAVE_GD_PNG)
	CREX_FE(imagecreatefrompng, arginfo_imagecreatefrompng)
#endif
#if defined(HAVE_GD_WEBP)
	CREX_FE(imagecreatefromwebp, arginfo_imagecreatefromwebp)
#endif
	CREX_FE(imagecreatefromxbm, arginfo_imagecreatefromxbm)
#if defined(HAVE_GD_XPM)
	CREX_FE(imagecreatefromxpm, arginfo_imagecreatefromxpm)
#endif
	CREX_FE(imagecreatefromwbmp, arginfo_imagecreatefromwbmp)
	CREX_FE(imagecreatefromgd, arginfo_imagecreatefromgd)
	CREX_FE(imagecreatefromgd2, arginfo_imagecreatefromgd2)
	CREX_FE(imagecreatefromgd2part, arginfo_imagecreatefromgd2part)
#if defined(HAVE_GD_BMP)
	CREX_FE(imagecreatefrombmp, arginfo_imagecreatefrombmp)
#endif
#if defined(HAVE_GD_TGA)
	CREX_FE(imagecreatefromtga, arginfo_imagecreatefromtga)
#endif
	CREX_FE(imagexbm, arginfo_imagexbm)
#if defined(HAVE_GD_AVIF)
	CREX_FE(imageavif, arginfo_imageavif)
#endif
	CREX_FE(imagegif, arginfo_imagegif)
#if defined(HAVE_GD_PNG)
	CREX_FE(imagepng, arginfo_imagepng)
#endif
#if defined(HAVE_GD_WEBP)
	CREX_FE(imagewebp, arginfo_imagewebp)
#endif
#if defined(HAVE_GD_JPG)
	CREX_FE(imagejpeg, arginfo_imagejpeg)
#endif
	CREX_FE(imagewbmp, arginfo_imagewbmp)
	CREX_FE(imagegd, arginfo_imagegd)
	CREX_FE(imagegd2, arginfo_imagegd2)
#if defined(HAVE_GD_BMP)
	CREX_FE(imagebmp, arginfo_imagebmp)
#endif
	CREX_FE(imagedestroy, arginfo_imagedestroy)
	CREX_FE(imagecolorallocate, arginfo_imagecolorallocate)
	CREX_FE(imagepalettecopy, arginfo_imagepalettecopy)
	CREX_FE(imagecolorat, arginfo_imagecolorat)
	CREX_FE(imagecolorclosest, arginfo_imagecolorclosest)
	CREX_FE(imagecolorclosesthwb, arginfo_imagecolorclosesthwb)
	CREX_FE(imagecolordeallocate, arginfo_imagecolordeallocate)
	CREX_FE(imagecolorresolve, arginfo_imagecolorresolve)
	CREX_FE(imagecolorexact, arginfo_imagecolorexact)
	CREX_FE(imagecolorset, arginfo_imagecolorset)
	CREX_FE(imagecolorsforindex, arginfo_imagecolorsforindex)
	CREX_FE(imagegammacorrect, arginfo_imagegammacorrect)
	CREX_FE(imagesetpixel, arginfo_imagesetpixel)
	CREX_FE(imageline, arginfo_imageline)
	CREX_FE(imagedashedline, arginfo_imagedashedline)
	CREX_FE(imagerectangle, arginfo_imagerectangle)
	CREX_FE(imagefilledrectangle, arginfo_imagefilledrectangle)
	CREX_FE(imagearc, arginfo_imagearc)
	CREX_FE(imageellipse, arginfo_imageellipse)
	CREX_FE(imagefilltoborder, arginfo_imagefilltoborder)
	CREX_FE(imagefill, arginfo_imagefill)
	CREX_FE(imagecolorstotal, arginfo_imagecolorstotal)
	CREX_FE(imagecolortransparent, arginfo_imagecolortransparent)
	CREX_FE(imageinterlace, arginfo_imageinterlace)
	CREX_FE(imagepolygon, arginfo_imagepolygon)
	CREX_FE(imageopenpolygon, arginfo_imageopenpolygon)
	CREX_FE(imagefilledpolygon, arginfo_imagefilledpolygon)
	CREX_FE(imagefontwidth, arginfo_imagefontwidth)
	CREX_FE(imagefontheight, arginfo_imagefontheight)
	CREX_FE(imagechar, arginfo_imagechar)
	CREX_FE(imagecharup, arginfo_imagecharup)
	CREX_FE(imagestring, arginfo_imagestring)
	CREX_FE(imagestringup, arginfo_imagestringup)
	CREX_FE(imagecopy, arginfo_imagecopy)
	CREX_FE(imagecopymerge, arginfo_imagecopymerge)
	CREX_FE(imagecopymergegray, arginfo_imagecopymergegray)
	CREX_FE(imagecopyresized, arginfo_imagecopyresized)
	CREX_FE(imagesx, arginfo_imagesx)
	CREX_FE(imagesy, arginfo_imagesy)
	CREX_FE(imagesetclip, arginfo_imagesetclip)
	CREX_FE(imagegetclip, arginfo_imagegetclip)
#if defined(HAVE_GD_FREETYPE)
	CREX_FE(imageftbbox, arginfo_imageftbbox)
#endif
#if defined(HAVE_GD_FREETYPE)
	CREX_FE(imagefttext, arginfo_imagefttext)
#endif
#if defined(HAVE_GD_FREETYPE)
	CREX_FALIAS(imagettfbbox, imageftbbox, arginfo_imagettfbbox)
#endif
#if defined(HAVE_GD_FREETYPE)
	CREX_FALIAS(imagettftext, imagefttext, arginfo_imagettftext)
#endif
	CREX_FE(imagefilter, arginfo_imagefilter)
	CREX_FE(imageconvolution, arginfo_imageconvolution)
	CREX_FE(imageflip, arginfo_imageflip)
	CREX_FE(imageantialias, arginfo_imageantialias)
	CREX_FE(imagecrop, arginfo_imagecrop)
	CREX_FE(imagecropauto, arginfo_imagecropauto)
	CREX_FE(imagescale, arginfo_imagescale)
	CREX_FE(imageaffine, arginfo_imageaffine)
	CREX_FE(imageaffinematrixget, arginfo_imageaffinematrixget)
	CREX_FE(imageaffinematrixconcat, arginfo_imageaffinematrixconcat)
	CREX_FE(imagegetinterpolation, arginfo_imagegetinterpolation)
	CREX_FE(imagesetinterpolation, arginfo_imagesetinterpolation)
	CREX_FE(imageresolution, arginfo_imageresolution)
	CREX_FE_END
};


static const crex_function_entry class_GdImage_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_GdFont_methods[] = {
	CREX_FE_END
};

static void register_gd_symbols(int module_number)
{
	REGISTER_LONG_CONSTANT("IMG_AVIF", CRX_IMG_AVIF, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_GIF", CRX_IMG_GIF, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_JPG", CRX_IMG_JPG, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_JPEG", CRX_IMG_JPEG, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_PNG", CRX_IMG_PNG, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_WBMP", CRX_IMG_WBMP, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_XPM", CRX_IMG_XPM, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_WEBP", CRX_IMG_WEBP, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_BMP", CRX_IMG_BMP, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_TGA", CRX_IMG_TGA, CONST_PERSISTENT);
#if defined(gdWebpLossless)
	REGISTER_LONG_CONSTANT("IMG_WEBP_LOSSLESS", gdWebpLossless, CONST_PERSISTENT);
#endif
	REGISTER_LONG_CONSTANT("IMG_COLOR_TILED", gdTiled, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_COLOR_STYLED", gdStyled, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_COLOR_BRUSHED", gdBrushed, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_COLOR_STYLEDBRUSHED", gdStyledBrushed, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_COLOR_TRANSPARENT", gdTransparent, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_ARC_ROUNDED", gdArc, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_ARC_PIE", gdPie, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_ARC_CHORD", gdChord, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_ARC_NOFILL", gdNoFill, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_ARC_EDGED", gdEdged, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_GD2_RAW", GD2_FMT_RAW, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_GD2_COMPRESSED", GD2_FMT_COMPRESSED, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FLIP_HORIZONTAL", CRX_GD_FLIP_HORIZONTAL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FLIP_VERTICAL", CRX_GD_FLIP_VERTICAL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FLIP_BOTH", CRX_GD_FLIP_BOTH, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_EFFECT_REPLACE", gdEffectReplace, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_EFFECT_ALPHABLEND", gdEffectAlphaBlend, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_EFFECT_NORMAL", gdEffectNormal, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_EFFECT_OVERLAY", gdEffectOverlay, CONST_PERSISTENT);
#if defined(gdEffectMultiply)
	REGISTER_LONG_CONSTANT("IMG_EFFECT_MULTIPLY", gdEffectMultiply, CONST_PERSISTENT);
#endif
	REGISTER_LONG_CONSTANT("IMG_CROP_DEFAULT", GD_CROP_DEFAULT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_CROP_TRANSPARENT", GD_CROP_TRANSPARENT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_CROP_BLACK", GD_CROP_BLACK, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_CROP_WHITE", GD_CROP_WHITE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_CROP_SIDES", GD_CROP_SIDES, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_CROP_THRESHOLD", GD_CROP_THRESHOLD, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_BELL", GD_BELL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_BESSEL", GD_BESSEL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_BILINEAR_FIXED", GD_BILINEAR_FIXED, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_BICUBIC", GD_BICUBIC, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_BICUBIC_FIXED", GD_BICUBIC_FIXED, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_BLACKMAN", GD_BLACKMAN, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_BOX", GD_BOX, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_BSPLINE", GD_BSPLINE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_CATMULLROM", GD_CATMULLROM, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_GAUSSIAN", GD_GAUSSIAN, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_GENERALIZED_CUBIC", GD_GENERALIZED_CUBIC, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_HERMITE", GD_HERMITE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_HAMMING", GD_HAMMING, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_HANNING", GD_HANNING, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_MITCHELL", GD_MITCHELL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_POWER", GD_POWER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_QUADRATIC", GD_QUADRATIC, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_SINC", GD_SINC, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_NEAREST_NEIGHBOUR", GD_NEAREST_NEIGHBOUR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_WEIGHTED4", GD_WEIGHTED4, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_TRIANGLE", GD_TRIANGLE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_AFFINE_TRANSLATE", GD_AFFINE_TRANSLATE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_AFFINE_SCALE", GD_AFFINE_SCALE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_AFFINE_ROTATE", GD_AFFINE_ROTATE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_AFFINE_SHEAR_HORIZONTAL", GD_AFFINE_SHEAR_HORIZONTAL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_AFFINE_SHEAR_VERTICAL", GD_AFFINE_SHEAR_VERTICAL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GD_BUNDLED", GD_BUNDLED, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_NEGATE", IMAGE_FILTER_NEGATE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_GRAYSCALE", IMAGE_FILTER_GRAYSCALE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_BRIGHTNESS", IMAGE_FILTER_BRIGHTNESS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_CONTRAST", IMAGE_FILTER_CONTRAST, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_COLORIZE", IMAGE_FILTER_COLORIZE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_EDGEDETECT", IMAGE_FILTER_EDGEDETECT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_GAUSSIAN_BLUR", IMAGE_FILTER_GAUSSIAN_BLUR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_SELECTIVE_BLUR", IMAGE_FILTER_SELECTIVE_BLUR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_EMBOSS", IMAGE_FILTER_EMBOSS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_MEAN_REMOVAL", IMAGE_FILTER_MEAN_REMOVAL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_SMOOTH", IMAGE_FILTER_SMOOTH, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_PIXELATE", IMAGE_FILTER_PIXELATE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMG_FILTER_SCATTER", IMAGE_FILTER_SCATTER, CONST_PERSISTENT);
#if defined(GD_VERSION_STRING)
	REGISTER_STRING_CONSTANT("GD_VERSION", GD_VERSION_STRING, CONST_PERSISTENT);
#endif
#if (defined(GD_MAJOR_VERSION) && defined(GD_MINOR_VERSION) && defined(GD_RELEASE_VERSION) && defined(GD_EXTRA_VERSION))
	REGISTER_LONG_CONSTANT("GD_MAJOR_VERSION", GD_MAJOR_VERSION, CONST_PERSISTENT);
#endif
#if (defined(GD_MAJOR_VERSION) && defined(GD_MINOR_VERSION) && defined(GD_RELEASE_VERSION) && defined(GD_EXTRA_VERSION))
	REGISTER_LONG_CONSTANT("GD_MINOR_VERSION", GD_MINOR_VERSION, CONST_PERSISTENT);
#endif
#if (defined(GD_MAJOR_VERSION) && defined(GD_MINOR_VERSION) && defined(GD_RELEASE_VERSION) && defined(GD_EXTRA_VERSION))
	REGISTER_LONG_CONSTANT("GD_RELEASE_VERSION", GD_RELEASE_VERSION, CONST_PERSISTENT);
#endif
#if (defined(GD_MAJOR_VERSION) && defined(GD_MINOR_VERSION) && defined(GD_RELEASE_VERSION) && defined(GD_EXTRA_VERSION))
	REGISTER_STRING_CONSTANT("GD_EXTRA_VERSION", GD_EXTRA_VERSION, CONST_PERSISTENT);
#endif
#if defined(HAVE_GD_PNG)
	REGISTER_LONG_CONSTANT("PNG_NO_FILTER", 0x0, CONST_PERSISTENT);
#endif
#if defined(HAVE_GD_PNG)
	REGISTER_LONG_CONSTANT("PNG_FILTER_NONE", 0x8, CONST_PERSISTENT);
#endif
#if defined(HAVE_GD_PNG)
	REGISTER_LONG_CONSTANT("PNG_FILTER_SUB", 0x10, CONST_PERSISTENT);
#endif
#if defined(HAVE_GD_PNG)
	REGISTER_LONG_CONSTANT("PNG_FILTER_UP", 0x20, CONST_PERSISTENT);
#endif
#if defined(HAVE_GD_PNG)
	REGISTER_LONG_CONSTANT("PNG_FILTER_AVG", 0x40, CONST_PERSISTENT);
#endif
#if defined(HAVE_GD_PNG)
	REGISTER_LONG_CONSTANT("PNG_FILTER_PAETH", 0x80, CONST_PERSISTENT);
#endif
#if defined(HAVE_GD_PNG)
	REGISTER_LONG_CONSTANT("PNG_ALL_FILTERS", 0x8 | 0x10 | 0x20 | 0x40 | 0x80, CONST_PERSISTENT);
#endif
}

static crex_class_entry *register_class_GdImage(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "GdImage", class_GdImage_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES|CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}

static crex_class_entry *register_class_GdFont(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "GdFont", class_GdFont_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES|CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}
