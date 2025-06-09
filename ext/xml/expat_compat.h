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
   | Authors: Sterling Hughes <sterling@crx.net>                          |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_EXPAT_COMPAT_H
#define CRX_EXPAT_COMPAT_H

#ifdef CRX_WIN32
#include "config.w32.h"
#else
#include <crx_config.h>
#endif

#ifdef CRX_WIN32
# define CRX_XML_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
# define CRX_XML_API __attribute__ ((visibility("default")))
#else
# define CRX_XML_API
#endif

#if !defined(HAVE_LIBEXPAT) && defined(HAVE_LIBXML)
#define LIBXML_EXPAT_COMPAT 1

#include "crx.h"
#include "crx_compat.h"

#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/tree.h>
#include <libxml/hash.h>

/* For compatibility with the misspelled version. */
#define _ns_seperator _ns_separator

typedef xmlChar XML_Char;

typedef void (*XML_StartElementHandler)(void *, const XML_Char *, const XML_Char **);
typedef void (*XML_EndElementHandler)(void *, const XML_Char *);
typedef void (*XML_CharacterDataHandler)(void *, const XML_Char *, int);
typedef void (*XML_ProcessingInstructionHandler)(void *, const XML_Char *, const XML_Char *);
typedef void (*XML_CommentHandler)(void *, const XML_Char *);
typedef void (*XML_DefaultHandler)(void *, const XML_Char *, int);
typedef void (*XML_UnparsedEntityDeclHandler)(void *, const XML_Char *, const XML_Char *, const XML_Char *, const XML_Char *, const XML_Char *);
typedef void (*XML_NotationDeclHandler)(void *, const XML_Char *, const XML_Char *, const XML_Char *, const XML_Char *);
typedef int  (*XML_ExternalEntityRefHandler)(void *, const XML_Char *, const XML_Char *, const XML_Char *, const XML_Char *);
typedef void (*XML_StartNamespaceDeclHandler)(void *, const XML_Char *, const XML_Char *);
typedef void (*XML_EndNamespaceDeclHandler)(void *, const XML_Char *);

typedef struct _XML_Memory_Handling_Suite {
  void *(*malloc_fcn)(size_t size);
  void *(*realloc_fcn)(void *ptr, size_t size);
  void (*free_fcn)(void *ptr);
} XML_Memory_Handling_Suite;

typedef struct _XML_Parser {
	int use_namespace;

	xmlChar *_ns_separator;

	void *user;
	xmlParserCtxtPtr parser;

	XML_StartElementHandler          h_start_element;
	XML_EndElementHandler            h_end_element;
	XML_CharacterDataHandler         h_cdata;
	XML_ProcessingInstructionHandler h_pi;
	XML_CommentHandler               h_comment;
	XML_DefaultHandler               h_default;
	XML_UnparsedEntityDeclHandler    h_unparsed_entity_decl;
	XML_NotationDeclHandler          h_notation_decl;
	XML_ExternalEntityRefHandler     h_external_entity_ref;
	XML_StartNamespaceDeclHandler    h_start_ns;
	XML_EndNamespaceDeclHandler      h_end_ns;
} *XML_Parser;

enum XML_Error {
	XML_ERROR_NONE,
	XML_ERROR_NO_MEMORY,
	XML_ERROR_SYNTAX,
	XML_ERROR_NO_ELEMENTS,
	XML_ERROR_INVALID_TOKEN,
	XML_ERROR_UNCLOSED_TOKEN,
	XML_ERROR_PARTIAL_CHAR,
	XML_ERROR_TAG_MISMATCH,
	XML_ERROR_DUPLICATE_ATTRIBUTE,
	XML_ERROR_JUNK_AFTER_DOC_ELEMENT,
	XML_ERROR_PARAM_ENTITY_REF,
	XML_ERROR_UNDEFINED_ENTITY,
	XML_ERROR_RECURSIVE_ENTITY_REF,
	XML_ERROR_ASYNC_ENTITY,
	XML_ERROR_BAD_CHAR_REF,
	XML_ERROR_BINARY_ENTITY_REF,
	XML_ERROR_ATTRIBUTE_EXTERNAL_ENTITY_REF,
	XML_ERROR_MISPLACED_XML_PI,
	XML_ERROR_UNKNOWN_ENCODING,
	XML_ERROR_INCORRECT_ENCODING,
	XML_ERROR_UNCLOSED_CDATA_SECTION,
	XML_ERROR_EXTERNAL_ENTITY_HANDLING,
	XML_ERROR_NOT_STANDALONE,
	XML_ERROR_UNEXPECTED_STATE,
	XML_ERROR_ENTITY_DECLARED_IN_PE,
	XML_ERROR_FEATURE_REQUIRES_XML_DTD,
	XML_ERROR_CANT_CHANGE_FEATURE_ONCE_PARSING
};

enum XML_Content_Type {
	XML_CTYPE_EMPTY = 1,
	XML_CTYPE_ANY,
	XML_CTYPE_MIXED,
	XML_CTYPE_NAME,
	XML_CTYPE_CHOICE,
	XML_CTYPE_SEQ
};

CRX_XML_API XML_Parser XML_ParserCreate(const XML_Char *);
CRX_XML_API XML_Parser XML_ParserCreateNS(const XML_Char *, const XML_Char);
CRX_XML_API XML_Parser XML_ParserCreate_MM(const XML_Char *, const XML_Memory_Handling_Suite *, const XML_Char *);
CRX_XML_API void XML_SetUserData(XML_Parser, void *);
CRX_XML_API void *XML_GetUserData(XML_Parser);
CRX_XML_API void XML_SetElementHandler(XML_Parser, XML_StartElementHandler, XML_EndElementHandler);
CRX_XML_API void XML_SetCharacterDataHandler(XML_Parser, XML_CharacterDataHandler);
CRX_XML_API void XML_SetProcessingInstructionHandler(XML_Parser, XML_ProcessingInstructionHandler);
CRX_XML_API void XML_SetDefaultHandler(XML_Parser, XML_DefaultHandler);
CRX_XML_API void XML_SetUnparsedEntityDeclHandler(XML_Parser, XML_UnparsedEntityDeclHandler);
CRX_XML_API void XML_SetNotationDeclHandler(XML_Parser, XML_NotationDeclHandler);
CRX_XML_API void XML_SetExternalEntityRefHandler(XML_Parser, XML_ExternalEntityRefHandler);
CRX_XML_API void XML_SetStartNamespaceDeclHandler(XML_Parser, XML_StartNamespaceDeclHandler);
CRX_XML_API void XML_SetEndNamespaceDeclHandler(XML_Parser, XML_EndNamespaceDeclHandler);
CRX_XML_API int  XML_Parse(XML_Parser, const XML_Char *, int data_len, int is_final);
CRX_XML_API int  XML_GetErrorCode(XML_Parser);
CRX_XML_API const XML_Char *XML_ErrorString(int);
CRX_XML_API int  XML_GetCurrentLineNumber(XML_Parser);
CRX_XML_API int  XML_GetCurrentColumnNumber(XML_Parser);
CRX_XML_API int  XML_GetCurrentByteIndex(XML_Parser);
CRX_XML_API int  XML_GetCurrentByteCount(XML_Parser);
CRX_XML_API const XML_Char *XML_ExpatVersion(void);
CRX_XML_API void XML_ParserFree(XML_Parser);

#elif defined(HAVE_LIBEXPAT)
#include "crx.h"
#include <expat.h>
#endif /* HAVE_LIBEXPAT */

#endif /* CRX_EXPAT_COMPAT_H */
