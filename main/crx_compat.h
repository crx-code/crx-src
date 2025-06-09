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
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_COMPAT_H
#define CRX_COMPAT_H

#ifdef CRX_WIN32
#include "config.w32.h"
#else
#include <crx_config.h>
#endif

#if defined(HAVE_BUNDLED_PCRE) || !defined(CRX_VERSION)
#define pcre2_jit_callback_8 crx_pcre2_jit_callback
#define pcre2_callout_enumerate_8 crx_pcre2_callout_enumerate
#define pcre2_code_copy_8 crx_pcre2_code_copy
#define pcre2_code_copy_with_tables_8 crx_pcre2_code_copy_with_tables
#define pcre2_code_free_8 crx_pcre2_code_free
#define pcre2_compile_8 crx_pcre2_compile
#define pcre2_compile_context_copy_8 crx_pcre2_compile_context_copy
#define pcre2_compile_context_create_8 crx_pcre2_compile_context_create
#define pcre2_compile_context_free_8 crx_pcre2_compile_context_free
#define pcre2_config_8 crx_pcre2_config
#define pcre2_convert_context_copy_8 crx_pcre2_convert_context_copy
#define pcre2_convert_context_create_8 crx_pcre2_convert_context_create
#define pcre2_convert_context_free_8 crx_pcre2_convert_context_free
#define pcre2_dfa_match_8 crx_pcre2_dfa_match
#define pcre2_general_context_copy_8 crx_pcre2_general_context_copy
#define pcre2_general_context_create_8 crx_pcre2_general_context_create
#define pcre2_general_context_free_8 crx_pcre2_general_context_free
#define pcre2_get_error_message_8 crx_pcre2_get_error_message
#define pcre2_get_mark_8 crx_pcre2_get_mark
#define pcre2_get_ovector_pointer_8 crx_pcre2_get_ovector_pointer
#define pcre2_get_ovector_count_8 crx_pcre2_get_ovector_count
#define pcre2_get_startchar_8 crx_pcre2_get_startchar
#define pcre2_jit_compile_8 crx_pcre2_jit_compile
#define pcre2_jit_match_8 crx_pcre2_jit_match
#define pcre2_jit_free_unused_memory_8 crx_pcre2_jit_free_unused_memory
#define pcre2_jit_stack_assign_8 crx_pcre2_jit_stack_assign
#define pcre2_jit_stack_create_8 crx_pcre2_jit_stack_create
#define pcre2_jit_stack_free_8 crx_pcre2_jit_stack_free
#define pcre2_maketables_8 crx_pcre2_maketables
#define pcre2_match_8 crx_pcre2_match
#define pcre2_match_context_copy_8 crx_pcre2_match_context_copy
#define pcre2_match_context_create_8 crx_pcre2_match_context_create
#define pcre2_match_context_free_8 crx_pcre2_match_context_free
#define pcre2_match_data_create_8 crx_pcre2_match_data_create
#define pcre2_match_data_create_from_pattern_8 crx_pcre2_match_data_create_from_pattern
#define pcre2_match_data_free_8 crx_pcre2_match_data_free
#define pcre2_pattern_info_8 crx_pcre2_pattern_info
#define pcre2_serialize_decode_8 crx_pcre2_serialize_decode
#define pcre2_serialize_encode_8 crx_pcre2_serialize_encode
#define pcre2_serialize_free_8 crx_pcre2_serialize_free
#define pcre2_serialize_get_number_of_codes_8 crx_pcre2_serialize_get_number_of_codes
#define pcre2_set_bsr_8 crx_pcre2_set_bsr
#define pcre2_set_callout_8 crx_pcre2_set_callout
#define pcre2_set_character_tables_8 crx_pcre2_set_character_tables
#define pcre2_set_compile_extra_options_8 crx_pcre2_set_compile_extra_options
#define pcre2_set_compile_recursion_guard_8 crx_pcre2_set_compile_recursion_guard
#define pcre2_set_depth_limit_8 crx_pcre2_set_depth_limit
#define pcre2_set_glob_escape_8 crx_pcre2_set_glob_escape
#define pcre2_set_glob_separator_8 crx_pcre2_set_glob_separator
#define pcre2_set_heap_limit_8 crx_pcre2_set_heap_limit
#define pcre2_set_match_limit_8 crx_pcre2_set_match_limit
#define pcre2_set_max_pattern_length_8 crx_pcre2_set_max_pattern_length
#define pcre2_set_newline_8 crx_pcre2_set_newline
#define pcre2_set_parens_nest_limit_8 crx_pcre2_set_parens_nest_limit
#define pcre2_set_offset_limit_8 crx_pcre2_set_offset_limit
#define pcre2_substitute_8 crx_pcre2_substitute
#define pcre2_substring_copy_byname_8 crx_pcre2_substring_copy_byname
#define pcre2_substring_copy_bynumber_8 crx_pcre2_substring_copy_bynumber
#define pcre2_substring_free_8 crx_pcre2_substring_free
#define pcre2_substring_get_byname_8 crx_pcre2_substring_get_byname
#define pcre2_substring_get_bynumber_8 crx_pcre2_substring_get_bynumber
#define pcre2_substring_length_byname_8 crx_pcre2_substring_length_byname
#define pcre2_substring_length_bynumber_8 crx_pcre2_substring_length_bynumber
#define pcre2_substring_list_get_8 crx_pcre2_substring_list_get
#define pcre2_substring_list_free_8 crx_pcre2_substring_list_free
#define pcre2_substring_nametable_scan_8 crx_pcre2_substring_nametable_scan
#define pcre2_substring_number_from_name_8 crx_pcre2_substring_number_from_name
#define pcre2_set_recursion_limit_8 crx_pcre2_set_recursion_limit
#define pcre2_set_recursion_memory_management_8 crx_pcre2_set_recursion_memory_management
#endif

#define lookup				crx_lookup
#define hashTableInit		crx_hashTableInit
#define hashTableDestroy	crx_hashTableDestroy
#define hashTableIterInit	crx_hashTableIterInit
#define hashTableIterNext	crx_hashTableIterNext

#if defined(HAVE_LIBXML) && (defined(HAVE_XML) || defined(HAVE_XMLRPC)) && !defined(HAVE_LIBEXPAT)
#define XML_DefaultCurrent crx_XML_DefaultCurrent
#define XML_ErrorString crx_XML_ErrorString
#define XML_ExpatVersion crx_XML_ExpatVersion
#define XML_ExpatVersionInfo crx_XML_ExpatVersionInfo
#define XML_ExternalEntityParserCreate crx_XML_ExternalEntityParserCreate
#define XML_GetBase crx_XML_GetBase
#define XML_GetBuffer crx_XML_GetBuffer
#define XML_GetCurrentByteCount crx_XML_GetCurrentByteCount
#define XML_GetCurrentByteIndex crx_XML_GetCurrentByteIndex
#define XML_GetCurrentColumnNumber crx_XML_GetCurrentColumnNumber
#define XML_GetCurrentLineNumber crx_XML_GetCurrentLineNumber
#define XML_GetErrorCode crx_XML_GetErrorCode
#define XML_GetIdAttributeIndex crx_XML_GetIdAttributeIndex
#define XML_GetInputContext crx_XML_GetInputContext
#define XML_GetSpecifiedAttributeCount crx_XML_GetSpecifiedAttributeCount
#define XmlGetUtf16InternalEncodingNS crx_XmlGetUtf16InternalEncodingNS
#define XmlGetUtf16InternalEncoding crx_XmlGetUtf16InternalEncoding
#define XmlGetUtf8InternalEncodingNS crx_XmlGetUtf8InternalEncodingNS
#define XmlGetUtf8InternalEncoding crx_XmlGetUtf8InternalEncoding
#define XmlInitEncoding crx_XmlInitEncoding
#define XmlInitEncodingNS crx_XmlInitEncodingNS
#define XmlInitUnknownEncoding crx_XmlInitUnknownEncoding
#define XmlInitUnknownEncodingNS crx_XmlInitUnknownEncodingNS
#define XML_ParseBuffer crx_XML_ParseBuffer
#define XML_Parse crx_XML_Parse
#define XML_ParserCreate_MM crx_XML_ParserCreate_MM
#define XML_ParserCreateNS crx_XML_ParserCreateNS
#define XML_ParserCreate crx_XML_ParserCreate
#define XML_ParserFree crx_XML_ParserFree
#define XmlParseXmlDecl crx_XmlParseXmlDecl
#define XmlParseXmlDeclNS crx_XmlParseXmlDeclNS
#define XmlPrologStateInitExternalEntity crx_XmlPrologStateInitExternalEntity
#define XmlPrologStateInit crx_XmlPrologStateInit
#define XML_SetAttlistDeclHandler crx_XML_SetAttlistDeclHandler
#define XML_SetBase crx_XML_SetBase
#define XML_SetCdataSectionHandler crx_XML_SetCdataSectionHandler
#define XML_SetCharacterDataHandler crx_XML_SetCharacterDataHandler
#define XML_SetCommentHandler crx_XML_SetCommentHandler
#define XML_SetDefaultHandlerExpand crx_XML_SetDefaultHandlerExpand
#define XML_SetDefaultHandler crx_XML_SetDefaultHandler
#define XML_SetDoctypeDeclHandler crx_XML_SetDoctypeDeclHandler
#define XML_SetElementDeclHandler crx_XML_SetElementDeclHandler
#define XML_SetElementHandler crx_XML_SetElementHandler
#define XML_SetEncoding crx_XML_SetEncoding
#define XML_SetEndCdataSectionHandler crx_XML_SetEndCdataSectionHandler
#define XML_SetEndDoctypeDeclHandler crx_XML_SetEndDoctypeDeclHandler
#define XML_SetEndElementHandler crx_XML_SetEndElementHandler
#define XML_SetEndNamespaceDeclHandler crx_XML_SetEndNamespaceDeclHandler
#define XML_SetEntityDeclHandler crx_XML_SetEntityDeclHandler
#define XML_SetExternalEntityRefHandlerArg crx_XML_SetExternalEntityRefHandlerArg
#define XML_SetExternalEntityRefHandler crx_XML_SetExternalEntityRefHandler
#define XML_SetNamespaceDeclHandler crx_XML_SetNamespaceDeclHandler
#define XML_SetNotationDeclHandler crx_XML_SetNotationDeclHandler
#define XML_SetNotStandaloneHandler crx_XML_SetNotStandaloneHandler
#define XML_SetParamEntityParsing crx_XML_SetParamEntityParsing
#define XML_SetProcessingInstructionHandler crx_XML_SetProcessingInstructionHandler
#define XML_SetReturnNSTriplet crx_XML_SetReturnNSTriplet
#define XML_SetStartCdataSectionHandler crx_XML_SetStartCdataSectionHandler
#define XML_SetStartDoctypeDeclHandler crx_XML_SetStartDoctypeDeclHandler
#define XML_SetStartElementHandler crx_XML_SetStartElementHandler
#define XML_SetStartNamespaceDeclHandler crx_XML_SetStartNamespaceDeclHandler
#define XML_SetUnknownEncodingHandler crx_XML_SetUnknownEncodingHandler
#define XML_SetUnparsedEntityDeclHandler crx_XML_SetUnparsedEntityDeclHandler
#define XML_SetUserData crx_XML_SetUserData
#define XML_SetXmlDeclHandler crx_XML_SetXmlDeclHandler
#define XmlSizeOfUnknownEncoding crx_XmlSizeOfUnknownEncoding
#define XML_UseParserAsHandlerArg crx_XML_UseParserAsHandlerArg
#define XmlUtf16Encode crx_XmlUtf16Encode
#define XmlUtf8Encode crx_XmlUtf8Encode
#define XML_FreeContentModel crx_XML_FreeContentModel
#define XML_MemMalloc crx_XML_MemMalloc
#define XML_MemRealloc crx_XML_MemRealloc
#define XML_MemFree crx_XML_MemFree
#define XML_UseForeignDTD crx_XML_UseForeignDTD
#define XML_GetFeatureList crx_XML_GetFeatureList
#define XML_ParserReset crx_XML_ParserReset

#ifdef HAVE_GD_BUNDLED
#define any2eucjp crx_gd_any2eucjp
#define createwbmp crx_gd_createwbmp
#define empty_output_buffer crx_gd_empty_output_buffer
#define fill_input_buffer crx_gd_fill_input_buffer
#define freewbmp crx_gd_freewbmp
#define gdAlphaBlend crx_gd_gdAlphaBlend
#define gdCompareInt crx_gd_gdCompareInt
#define gdCosT crx_gd_gdCosT
#define gdCtxPrintf crx_gd_gdCtxPrintf
#define gdDPExtractData crx_gd_gdDPExtractData
#define gdFontGetGiant crx_gd_gdFontGetGiant
#define gdFontGetLarge crx_gd_gdFontGetLarge
#define gdFontGetMediumBold crx_gd_gdFontGetMediumBold
#define gdFontGetSmall crx_gd_gdFontGetSmall
#define gdFontGetTiny crx_gd_gdFontGetTiny
#define gdFontGiant crx_gd_gdFontGiant
#define gdFontGiantData crx_gd_gdFontGiantData
#define gdFontGiantRep crx_gd_gdFontGiantRep
#define gdFontLarge crx_gd_gdFontLarge
#define gdFontLargeData crx_gd_gdFontLargeData
#define gdFontLargeRep crx_gd_gdFontLargeRep
#define gdFontMediumBold crx_gd_gdFontMediumBold
#define gdFontMediumBoldData crx_gd_gdFontMediumBoldData
#define gdFontMediumBoldRep crx_gd_gdFontMediumBoldRep
#define gdFontSmall crx_gd_gdFontSmall
#define gdFontSmallData crx_gd_gdFontSmallData
#define gdFontSmallRep crx_gd_gdFontSmallRep
#define gdFontTiny crx_gd_gdFontTiny
#define gdFontTinyData crx_gd_gdFontTinyData
#define gdFontTinyRep crx_gd_gdFontTinyRep
#define gdGetBuf crx_gd_gdGetBuf
#define gdGetByte crx_gd_gdGetByte
#define gdGetC crx_gd_gdGetC
#define _gdGetColors crx_gd__gdGetColors
#define gd_getin crx_gd_gd_getin
#define gdGetInt crx_gd_gdGetInt
#define gdGetWord crx_gd_gdGetWord
#define gdImageAABlend crx_gd_gdImageAABlend
#define gdImageAALine crx_gd_gdImageAALine
#define gdImageAlphaBlending crx_gd_gdImageAlphaBlending
#define gdImageAntialias crx_gd_gdImageAntialias
#define gdImageArc crx_gd_gdImageArc
#define gdImageBrightness crx_gd_gdImageBrightness
#define gdImageChar crx_gd_gdImageChar
#define gdImageCharUp crx_gd_gdImageCharUp
#define gdImageColor crx_gd_gdImageColor
#define gdImageColorAllocate crx_gd_gdImageColorAllocate
#define gdImageColorAllocateAlpha crx_gd_gdImageColorAllocateAlpha
#define gdImageColorClosest crx_gd_gdImageColorClosest
#define gdImageColorClosestAlpha crx_gd_gdImageColorClosestAlpha
#define gdImageColorClosestHWB crx_gd_gdImageColorClosestHWB
#define gdImageColorDeallocate crx_gd_gdImageColorDeallocate
#define gdImageColorExact crx_gd_gdImageColorExact
#define gdImageColorExactAlpha crx_gd_gdImageColorExactAlpha
#define gdImageColorMatch crx_gd_gdImageColorMatch
#define gdImageColorResolve crx_gd_gdImageColorResolve
#define gdImageColorResolveAlpha crx_gd_gdImageColorResolveAlpha
#define gdImageColorTransparent crx_gd_gdImageColorTransparent
#define gdImageCompare crx_gd_gdImageCompare
#define gdImageContrast crx_gd_gdImageContrast
#define gdImageConvolution crx_gd_gdImageConvolution
#define gdImageCopy crx_gd_gdImageCopy
#define gdImageCopyMerge crx_gd_gdImageCopyMerge
#define gdImageCopyMergeGray crx_gd_gdImageCopyMergeGray
#define gdImageCopyResampled crx_gd_gdImageCopyResampled
#define gdImageCopyResized crx_gd_gdImageCopyResized
#define gdImageCreate crx_gd_gdImageCreate
#define gdImageCreateFromGd crx_gd_gdImageCreateFromGd
#define gdImageCreateFromGd2 crx_gd_gdImageCreateFromGd2
#define gdImageCreateFromGd2Ctx crx_gd_gdImageCreateFromGd2Ctx
#define gdImageCreateFromGd2Part crx_gd_gdImageCreateFromGd2Part
#define gdImageCreateFromGd2PartCtx crx_gd_gdImageCreateFromGd2PartCtx
#define gdImageCreateFromGd2PartPtr crx_gd_gdImageCreateFromGd2PartPtr
#define gdImageCreateFromGd2Ptr crx_gd_gdImageCreateFromGd2Ptr
#define gdImageCreateFromGdCtx crx_gd_gdImageCreateFromGdCtx
#define gdImageCreateFromGdPtr crx_gd_gdImageCreateFromGdPtr
#define gdImageCreateFromGif crx_gd_gdImageCreateFromGif
#define gdImageCreateFromGifCtx crx_gd_gdImageCreateFromGifCtx
#define gdImageCreateFromGifSource crx_gd_gdImageCreateFromGifSource
#define gdImageCreateFromJpeg crx_gd_gdImageCreateFromJpeg
#define gdImageCreateFromJpegCtx crx_gd_gdImageCreateFromJpegCtx
#define gdImageCreateFromJpegPtr crx_gd_gdImageCreateFromJpegPtr
#define gdImageCreateFromPng crx_gd_gdImageCreateFromPng
#define gdImageCreateFromPngCtx crx_gd_gdImageCreateFromPngCtx
#define gdImageCreateFromPngPtr crx_gd_gdImageCreateFromPngPtr
#define gdImageCreateFromPngSource crx_gd_gdImageCreateFromPngSource
#define gdImageCreateFromWBMP crx_gd_gdImageCreateFromWBMP
#define gdImageCreateFromWBMPCtx crx_gd_gdImageCreateFromWBMPCtx
#define gdImageCreateFromWBMPPtr crx_gd_gdImageCreateFromWBMPPtr
#define gdImageCreateFromXbm crx_gd_gdImageCreateFromXbm
#define gdImageCreatePaletteFromTrueColor crx_gd_gdImageCreatePaletteFromTrueColor
#define gdImageCreateTrueColor crx_gd_gdImageCreateTrueColor
#define gdImageDashedLine crx_gd_gdImageDashedLine
#define gdImageDestroy crx_gd_gdImageDestroy
#define gdImageEdgeDetectQuick crx_gd_gdImageEdgeDetectQuick
#define gdImageEllipse crx_gd_gdImageEllipse
#define gdImageEmboss crx_gd_gdImageEmboss
#define gdImageFill crx_gd_gdImageFill
#define gdImageFilledArc crx_gd_gdImageFilledArc
#define gdImageFilledEllipse crx_gd_gdImageFilledEllipse
#define gdImageFilledPolygon crx_gd_gdImageFilledPolygon
#define gdImageFilledRectangle crx_gd_gdImageFilledRectangle
#define _gdImageFillTiled crx_gd__gdImageFillTiled
#define gdImageFillToBorder crx_gd_gdImageFillToBorder
#define gdImageGaussianBlur crx_gd_gdImageGaussianBlur
#define gdImageGd crx_gd_gdImageGd
#define gdImageGd2 crx_gd_gdImageGd2
#define gdImageGd2Ptr crx_gd_gdImageGd2Ptr
#define gdImageGdPtr crx_gd_gdImageGdPtr
#define gdImageGetClip crx_gd_gdImageGetClip
#define gdImageGetPixel crx_gd_gdImageGetPixel
#define gdImageGetTrueColorPixel crx_gd_gdImageGetTrueColorPixel
#define gdImageGif crx_gd_gdImageGif
#define gdImageGifCtx crx_gd_gdImageGifCtx
#define gdImageGifPtr crx_gd_gdImageGifPtr
#define gdImageGrayScale crx_gd_gdImageGrayScale
#define gdImageInterlace crx_gd_gdImageInterlace
#define gdImageJpeg crx_gd_gdImageJpeg
#define gdImageJpegCtx crx_gd_gdImageJpegCtx
#define gdImageJpegPtr crx_gd_gdImageJpegPtr
#define gdImageLine crx_gd_gdImageLine
#define gdImageMeanRemoval crx_gd_gdImageMeanRemoval
#define gdImageNegate crx_gd_gdImageNegate
#define gdImagePaletteCopy crx_gd_gdImagePaletteCopy
#define gdImagePng crx_gd_gdImagePng
#define gdImagePngCtx crx_gd_gdImagePngCtx
#define gdImagePngCtxEx crx_gd_gdImagePngCtxEx
#define gdImagePngEx crx_gd_gdImagePngEx
#define gdImagePngPtr crx_gd_gdImagePngPtr
#define gdImagePngPtrEx crx_gd_gdImagePngPtrEx
#define gdImagePngToSink crx_gd_gdImagePngToSink
#define gdImagePolygon crx_gd_gdImagePolygon
#define gdImageRectangle crx_gd_gdImageRectangle
#define gdImageRotate crx_gd_gdImageRotate
#define gdImageRotate180 crx_gd_gdImageRotate180
#define gdImageRotate270 crx_gd_gdImageRotate270
#define gdImageRotate45 crx_gd_gdImageRotate45
#define gdImageRotate90 crx_gd_gdImageRotate90
#define gdImageSaveAlpha crx_gd_gdImageSaveAlpha
#define gdImageSelectiveBlur crx_gd_gdImageSelectiveBlur
#define gdImageSetAntiAliased crx_gd_gdImageSetAntiAliased
#define gdImageSetAntiAliasedDontBlend crx_gd_gdImageSetAntiAliasedDontBlend
#define gdImageSetBrush crx_gd_gdImageSetBrush
#define gdImageSetClip crx_gd_gdImageSetClip
#define gdImageSetPixel crx_gd_gdImageSetPixel
#define gdImageSetStyle crx_gd_gdImageSetStyle
#define gdImageSetThickness crx_gd_gdImageSetThickness
#define gdImageSetTile crx_gd_gdImageSetTile
#define gdImageSkewX crx_gd_gdImageSkewX
#define gdImageSkewY crx_gd_gdImageSkewY
#define gdImageSmooth crx_gd_gdImageSmooth
#define gdImageString crx_gd_gdImageString
#define gdImageString16 crx_gd_gdImageString16
#define gdImageStringFT crx_gd_gdImageStringFT
#define gdImageStringFTEx crx_gd_gdImageStringFTEx
#define gdImageStringTTF crx_gd_gdImageStringTTF
#define gdImageStringUp crx_gd_gdImageStringUp
#define gdImageStringUp16 crx_gd_gdImageStringUp16
#define gdImageTrueColorToPalette crx_gd_gdImageTrueColorToPalette
#define gdImageWBMP crx_gd_gdImageWBMP
#define gdImageWBMPCtx crx_gd_gdImageWBMPCtx
#define gdImageWBMPPtr crx_gd_gdImageWBMPPtr
#define gdImageXbmCtx crx_gd_gdImageXbmCtx
#define gdNewDynamicCtx crx_gd_gdNewDynamicCtx
#define gdNewDynamicCtxEx crx_gd_gdNewDynamicCtxEx
#define gdNewFileCtx crx_gd_gdNewFileCtx
#define gdNewSSCtx crx_gd_gdNewSSCtx
#define gdPutBuf crx_gd_gdPutBuf
#define gdPutC crx_gd_gdPutC
#define _gdPutColors crx_gd__gdPutColors
#define gdPutInt crx_gd_gdPutInt
#define gd_putout crx_gd_gd_putout
#define gdPutWord crx_gd_gdPutWord
#define gdSeek crx_gd_gdSeek
#define gdSinT crx_gd_gdSinT
#define gd_strtok_r crx_gd_gd_strtok_r
#define gdTell crx_gd_gdTell
#define getmbi crx_gd_getmbi
#define init_destination crx_gd_init_destination
#define init_source crx_gd_init_source
#define jpeg_gdIOCtx_dest crx_gd_jpeg_gdIOCtx_dest
#define jpeg_gdIOCtx_src crx_gd_jpeg_gdIOCtx_src
#define lsqrt crx_gd_lsqrt
#define printwbmp crx_gd_printwbmp
#define Putchar crx_gd_Putchar
#define putmbi crx_gd_putmbi
#define Putword crx_gd_Putword
#define readwbmp crx_gd_readwbmp
#define skipheader crx_gd_skipheader
#define skip_input_data crx_gd_skip_input_data
#define term_destination crx_gd_term_destination
#define term_source crx_gd_term_source
#define writewbmp crx_gd_writewbmp
#define ZeroDataBlock crx_gd_ZeroDataBlock
#define gdCacheCreate crx_gd_gdCacheCreate
#define gdCacheDelete crx_gd_gdCacheDelete
#define gdCacheGet crx_gd_gdCacheGet
#define gdFontCacheSetup crx_gd_gdFontCacheSetup
#define gdFontCacheShutdown crx_gd_gdFontCacheShutdown
#define gdFreeFontCache crx_gd_gdFreeFontCache
#endif /* HAVE_GD_BUNDLED */

/* Define to specify how much context to retain around the current parse
   point. */
#define XML_CONTEXT_BYTES 1024

/* Define to make parameter entity parsing functionality available. */
#define XML_DTD 1

/* Define to make XML Namespaces functionality available. */
#define XML_NS 1
#endif

#ifdef CRX_EXPORTS
#define PCRE_STATIC
#endif

#endif
