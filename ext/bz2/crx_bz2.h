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
  | Author: Sterling Hughes <sterling@crx.net>                           |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_BZ2_H
#define CRX_BZ2_H

#ifdef HAVE_BZ2

extern crex_module_entry bz2_module_entry;
#define crxext_bz2_ptr &bz2_module_entry

/* Bzip2 includes */
#include <bzlib.h>

#else
#define crxext_bz2_ptr NULL
#endif

#ifdef CRX_WIN32
#	ifdef CRX_BZ2_EXPORTS
#		define CRX_BZ2_API __declspec(dllexport)
#	elif defined(COMPILE_DL_BZ2)
#		define CRX_BZ2_API __declspec(dllimport)
#	else
#		define CRX_BZ2_API /* nothing special */
#	endif
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define CRX_BZ2_API __attribute__ ((visibility("default")))
#else
#	define CRX_BZ2_API
#endif

#include "crx_version.h"
#define CRX_BZ2_VERSION CRX_VERSION

CRX_BZ2_API crx_stream *_crx_stream_bz2open(crx_stream_wrapper *wrapper, const char *path, const char *mode, int options, crex_string **opened_path, crx_stream_context *context STREAMS_DC);
CRX_BZ2_API crx_stream *_crx_stream_bz2open_from_BZFILE(BZFILE *bz, const char *mode, crx_stream *innerstream STREAMS_DC);

#define crx_stream_bz2open_from_BZFILE(bz, mode, innerstream)	_crx_stream_bz2open_from_BZFILE((bz), (mode), (innerstream) STREAMS_CC)
#define crx_stream_bz2open(wrapper, path, mode, options, opened_path)	_crx_stream_bz2open((wrapper), (path), (mode), (options), (opened_path), NULL STREAMS_CC)

extern const crx_stream_filter_factory crx_bz2_filter_factory;
extern const crx_stream_ops crx_stream_bz2io_ops;
#define CRX_STREAM_IS_BZIP2	&crx_stream_bz2io_ops

/* 400kb */
#define CRX_BZ2_FILTER_DEFAULT_BLOCKSIZE        4

/* BZ2 Internal Default */
#define CRX_BZ2_FILTER_DEFAULT_WORKFACTOR       0

#endif
