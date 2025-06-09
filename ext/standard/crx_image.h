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
   |          Marcus Boerger <helly@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_IMAGE_H
#define CRX_IMAGE_H

/* {{{ enum image_filetype
   This enum is used to have ext/standard/image.c and ext/exif/exif.c use
   the same constants for file types.
*/
typedef enum
{ IMAGE_FILETYPE_UNKNOWN=0,
  IMAGE_FILETYPE_GIF=1,
  IMAGE_FILETYPE_JPEG,
  IMAGE_FILETYPE_PNG,
  IMAGE_FILETYPE_SWF,
  IMAGE_FILETYPE_PSD,
  IMAGE_FILETYPE_BMP,
  IMAGE_FILETYPE_TIFF_II, /* intel */
  IMAGE_FILETYPE_TIFF_MM, /* motorola */
  IMAGE_FILETYPE_JPC,
  IMAGE_FILETYPE_JP2,
  IMAGE_FILETYPE_JPX,
  IMAGE_FILETYPE_JB2,
  IMAGE_FILETYPE_SWC,
  IMAGE_FILETYPE_IFF,
  IMAGE_FILETYPE_WBMP,
  /* IMAGE_FILETYPE_JPEG2000 is a userland alias for IMAGE_FILETYPE_JPC */
  IMAGE_FILETYPE_XBM,
  IMAGE_FILETYPE_ICO,
  IMAGE_FILETYPE_WEBP,
  IMAGE_FILETYPE_AVIF,
/* WHEN EXTENDING: PLEASE ALSO REGISTER IN basic_function.stub.crx */
  IMAGE_FILETYPE_COUNT
} image_filetype;
/* }}} */

CRXAPI int crx_getimagetype(crx_stream *stream, const char *input, char *filetype);

CRXAPI char * crx_image_type_to_mime_type(int image_type);

CRXAPI bool crx_is_image_avif(crx_stream *stream);

#endif /* CRX_IMAGE_H */
