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
   | Authors: Stig Sæther Bakken <ssb@crx.net>                            |
   |          Thies C. Arntzen <thies@thieso.net>                         |
   |          Sterling Hughes <sterling@crx.net>                          |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"

#include "crex_variables.h"
#include "ext/standard/info.h"
#include "ext/standard/html.h"

#ifdef HAVE_XML

#include "crx_xml.h"
# include "ext/standard/head.h"
#ifdef LIBXML_EXPAT_COMPAT
#include "ext/libxml/crx_libxml.h"
#endif

#include "xml_arginfo.h"

/* Short-term TODO list:
 * - Implement XML_ExternalEntityParserCreate()
 * - XML_SetCommentHandler
 * - XML_SetCdataSectionHandler
 * - XML_SetParamEntityParsing
 */

/* Long-term TODO list:
 * - Fix the expat library so you can install your own memory manager
 *   functions
 */

/* Known bugs:
 * - Weird things happen with <![CDATA[]]> sections.
 */

CREX_BEGIN_MODULE_GLOBALS(xml)
	XML_Char *default_encoding;
CREX_END_MODULE_GLOBALS(xml)

CREX_DECLARE_MODULE_GLOBALS(xml)

#define XML(v) CREX_MODULE_GLOBALS_ACCESSOR(xml, v)

typedef struct {
	int case_folding;
	XML_Parser parser;
	XML_Char *target_encoding;

	/* Reference to the object itself, for convenience.
	 * It is not owned, do not release it. */
	zval index;

	/* We return a pointer to these zvals in get_gc(), so it's
	 * important that a) they are adjacent b) object is the first
	 * and c) the number of zvals is kept up to date. */
#define XML_PARSER_NUM_ZVALS 12
	zval object;
	zval startElementHandler;
	zval endElementHandler;
	zval characterDataHandler;
	zval processingInstructionHandler;
	zval defaultHandler;
	zval unparsedEntityDeclHandler;
	zval notationDeclHandler;
	zval externalEntityRefHandler;
	zval unknownEncodingHandler;
	zval startNamespaceDeclHandler;
	zval endNamespaceDeclHandler;

	crex_function *startElementPtr;
	crex_function *endElementPtr;
	crex_function *characterDataPtr;
	crex_function *processingInstructionPtr;
	crex_function *defaultPtr;
	crex_function *unparsedEntityDeclPtr;
	crex_function *notationDeclPtr;
	crex_function *externalEntityRefPtr;
	crex_function *unknownEncodingPtr;
	crex_function *startNamespaceDeclPtr;
	crex_function *endNamespaceDeclPtr;

	zval data;
	zval info;
	int level;
	int toffset;
	int curtag;
	zval *ctag;
	char **ltags;
	int lastwasopen;
	int skipwhite;
	int isparsing;

	XML_Char *baseURI;

	crex_object std;
} xml_parser;


typedef struct {
	XML_Char *name;
	char (*decoding_function)(unsigned short);
	unsigned short (*encoding_function)(unsigned char);
} xml_encoding;

/* {{{ dynamically loadable module stuff */
#ifdef COMPILE_DL_XML
#ifdef ZTS
CREX_TSRMLS_CACHE_DEFINE()
#endif
CREX_GET_MODULE(xml)
#endif /* COMPILE_DL_XML */
/* }}} */

#define XML_MAXLEVEL 255 /* XXX this should be dynamic */

#define SKIP_TAGSTART(str) ((str) + (parser->toffset > strlen(str) ? strlen(str) : parser->toffset))

static crex_class_entry *xml_parser_ce;
static crex_object_handlers xml_parser_object_handlers;

/* {{{ function prototypes */
CRX_MINIT_FUNCTION(xml);
CRX_MINFO_FUNCTION(xml);
static CRX_GINIT_FUNCTION(xml);

static crex_object *xml_parser_create_object(crex_class_entry *class_type);
static void xml_parser_free_obj(crex_object *object);
static HashTable *xml_parser_get_gc(crex_object *object, zval **table, int *n);
static crex_function *xml_parser_get_constructor(crex_object *object);

static crex_string *xml_utf8_decode(const XML_Char *, size_t, const XML_Char *);
static void xml_set_handler(zval *, zval *);
inline static unsigned short xml_encode_iso_8859_1(unsigned char);
inline static char xml_decode_iso_8859_1(unsigned short);
inline static unsigned short xml_encode_us_ascii(unsigned char);
inline static char xml_decode_us_ascii(unsigned short);
static void xml_call_handler(xml_parser *, zval *, crex_function *, int, zval *, zval *);
static void _xml_xmlchar_zval(const XML_Char *, int, const XML_Char *, zval *);
static int _xml_xmlcharlen(const XML_Char *);
static void _xml_add_to_info(xml_parser *parser, const char *name);
inline static crex_string *_xml_decode_tag(xml_parser *parser, const XML_Char *tag);

void _xml_startElementHandler(void *, const XML_Char *, const XML_Char **);
void _xml_endElementHandler(void *, const XML_Char *);
void _xml_characterDataHandler(void *, const XML_Char *, int);
void _xml_processingInstructionHandler(void *, const XML_Char *, const XML_Char *);
void _xml_defaultHandler(void *, const XML_Char *, int);
void _xml_unparsedEntityDeclHandler(void *, const XML_Char *, const XML_Char *, const XML_Char *, const XML_Char *, const XML_Char *);
void _xml_notationDeclHandler(void *, const XML_Char *, const XML_Char *, const XML_Char *, const XML_Char *);
int  _xml_externalEntityRefHandler(XML_Parser, const XML_Char *, const XML_Char *, const XML_Char *, const XML_Char *);

void _xml_startNamespaceDeclHandler(void *, const XML_Char *, const XML_Char *);
void _xml_endNamespaceDeclHandler(void *, const XML_Char *);
/* }}} */

#ifdef LIBXML_EXPAT_COMPAT
static const crex_module_dep xml_deps[] = {
	CREX_MOD_REQUIRED("libxml")
	CREX_MOD_END
};
#endif

crex_module_entry xml_module_entry = {
#ifdef LIBXML_EXPAT_COMPAT
	STANDARD_MODULE_HEADER_EX, NULL,
	xml_deps,
#else
	STANDARD_MODULE_HEADER,
#endif
	"xml",                /* extension name */
	ext_functions,        /* extension function list */
	CRX_MINIT(xml),       /* extension-wide startup function */
	NULL,                 /* extension-wide shutdown function */
	NULL,                 /* per-request startup function */
	NULL,                 /* per-request shutdown function */
	CRX_MINFO(xml),       /* information function */
	CRX_XML_VERSION,
	CRX_MODULE_GLOBALS(xml), /* globals descriptor */
	CRX_GINIT(xml),          /* globals ctor */
	NULL,                    /* globals dtor */
	NULL,                    /* post deactivate */
	STANDARD_MODULE_PROPERTIES_EX
};

/* All the encoding functions are set to NULL right now, since all
 * the encoding is currently done internally by expat/xmltok.
 */
const xml_encoding xml_encodings[] = {
	{ (XML_Char *)"ISO-8859-1", xml_decode_iso_8859_1, xml_encode_iso_8859_1 },
	{ (XML_Char *)"US-ASCII",   xml_decode_us_ascii,   xml_encode_us_ascii   },
	{ (XML_Char *)"UTF-8",      NULL,                  NULL                  },
	{ (XML_Char *)NULL,         NULL,                  NULL                  }
};

static XML_Memory_Handling_Suite crx_xml_mem_hdlrs;

/* }}} */

/* {{{ startup, shutdown and info functions */
static CRX_GINIT_FUNCTION(xml)
{
#if defined(COMPILE_DL_XML) && defined(ZTS)
	CREX_TSRMLS_CACHE_UPDATE();
#endif
	xml_globals->default_encoding = (XML_Char*)"UTF-8";
}

static void *crx_xml_malloc_wrapper(size_t sz)
{
	return emalloc(sz);
}

static void *crx_xml_realloc_wrapper(void *ptr, size_t sz)
{
	return erealloc(ptr, sz);
}

static void crx_xml_free_wrapper(void *ptr)
{
	if (ptr != NULL) {
		efree(ptr);
	}
}

CRX_MINIT_FUNCTION(xml)
{
	xml_parser_ce = register_class_XMLParser();
	xml_parser_ce->create_object = xml_parser_create_object;
	xml_parser_ce->default_object_handlers = &xml_parser_object_handlers;

	memcpy(&xml_parser_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	xml_parser_object_handlers.offset = XtOffsetOf(xml_parser, std);
	xml_parser_object_handlers.free_obj = xml_parser_free_obj;
	xml_parser_object_handlers.get_gc = xml_parser_get_gc;
	xml_parser_object_handlers.get_constructor = xml_parser_get_constructor;
	xml_parser_object_handlers.clone_obj = NULL;
	xml_parser_object_handlers.compare = crex_objects_not_comparable;

	register_xml_symbols(module_number);

	/* this object should not be pre-initialised at compile time,
	   as the order of members may vary */

	crx_xml_mem_hdlrs.malloc_fcn = crx_xml_malloc_wrapper;
	crx_xml_mem_hdlrs.realloc_fcn = crx_xml_realloc_wrapper;
	crx_xml_mem_hdlrs.free_fcn = crx_xml_free_wrapper;

	return SUCCESS;
}

CRX_MINFO_FUNCTION(xml)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "XML Support", "active");
	crx_info_print_table_row(2, "XML Namespace Support", "active");
#if defined(LIBXML_DOTTED_VERSION) && defined(LIBXML_EXPAT_COMPAT)
	crx_info_print_table_row(2, "libxml2 Version", LIBXML_DOTTED_VERSION);
#else
	crx_info_print_table_row(2, "EXPAT Version", XML_ExpatVersion());
#endif
	crx_info_print_table_end();
}
/* }}} */

/* {{{ extension-internal functions */

static void _xml_xmlchar_zval(const XML_Char *s, int len, const XML_Char *encoding, zval *ret)
{
	if (s == NULL) {
		ZVAL_FALSE(ret);
		return;
	}
	if (len == 0) {
		len = _xml_xmlcharlen(s);
	}
	ZVAL_STR(ret, xml_utf8_decode(s, len, encoding));
}
/* }}} */

static inline xml_parser *xml_parser_from_obj(crex_object *obj) {
	return (xml_parser *)((char *)(obj) - XtOffsetOf(xml_parser, std));
}

#define C_XMLPARSER_P(zv) xml_parser_from_obj(C_OBJ_P(zv))

static crex_object *xml_parser_create_object(crex_class_entry *class_type) {
	xml_parser *intern = crex_object_alloc(sizeof(xml_parser), class_type);
	memset(intern, 0, sizeof(xml_parser) - sizeof(crex_object));

	crex_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	return &intern->std;
}

static void xml_parser_free_ltags(xml_parser *parser)
{
	if (parser->ltags) {
		int inx;
		for (inx = 0; ((inx < parser->level) && (inx < XML_MAXLEVEL)); inx++)
			efree(parser->ltags[ inx ]);
		efree(parser->ltags);
	}
}

static void xml_parser_free_obj(crex_object *object)
{
	xml_parser *parser = xml_parser_from_obj(object);

	if (parser->parser) {
		XML_ParserFree(parser->parser);
	}
	xml_parser_free_ltags(parser);
	if (!C_ISUNDEF(parser->startElementHandler)) {
		zval_ptr_dtor(&parser->startElementHandler);
	}
	if (!C_ISUNDEF(parser->endElementHandler)) {
		zval_ptr_dtor(&parser->endElementHandler);
	}
	if (!C_ISUNDEF(parser->characterDataHandler)) {
		zval_ptr_dtor(&parser->characterDataHandler);
	}
	if (!C_ISUNDEF(parser->processingInstructionHandler)) {
		zval_ptr_dtor(&parser->processingInstructionHandler);
	}
	if (!C_ISUNDEF(parser->defaultHandler)) {
		zval_ptr_dtor(&parser->defaultHandler);
	}
	if (!C_ISUNDEF(parser->unparsedEntityDeclHandler)) {
		zval_ptr_dtor(&parser->unparsedEntityDeclHandler);
	}
	if (!C_ISUNDEF(parser->notationDeclHandler)) {
		zval_ptr_dtor(&parser->notationDeclHandler);
	}
	if (!C_ISUNDEF(parser->externalEntityRefHandler)) {
		zval_ptr_dtor(&parser->externalEntityRefHandler);
	}
	if (!C_ISUNDEF(parser->unknownEncodingHandler)) {
		zval_ptr_dtor(&parser->unknownEncodingHandler);
	}
	if (!C_ISUNDEF(parser->startNamespaceDeclHandler)) {
		zval_ptr_dtor(&parser->startNamespaceDeclHandler);
	}
	if (!C_ISUNDEF(parser->endNamespaceDeclHandler)) {
		zval_ptr_dtor(&parser->endNamespaceDeclHandler);
	}
	if (parser->baseURI) {
		efree(parser->baseURI);
	}
	if (!C_ISUNDEF(parser->object)) {
		zval_ptr_dtor(&parser->object);
	}

	crex_object_std_dtor(&parser->std);
}

static HashTable *xml_parser_get_gc(crex_object *object, zval **table, int *n)
{
	xml_parser *parser = xml_parser_from_obj(object);
	*table = &parser->object;
	*n = XML_PARSER_NUM_ZVALS;
	return crex_std_get_properties(object);
}

static crex_function *xml_parser_get_constructor(crex_object *object) {
	crex_throw_error(NULL, "Cannot directly construct XMLParser, use xml_parser_create() or xml_parser_create_ns() instead");
	return NULL;
}

/* {{{ xml_set_handler() */
static void xml_set_handler(zval *handler, zval *data)
{
	/* If we have already a handler, release it */
	if (handler) {
		zval_ptr_dtor(handler);
	}

	/* IS_ARRAY might indicate that we're using array($obj, 'method') syntax */
	if (C_TYPE_P(data) != IS_ARRAY && C_TYPE_P(data) != IS_OBJECT) {
		convert_to_string(data);
		if (C_STRLEN_P(data) == 0) {
			ZVAL_UNDEF(handler);
			return;
		}
	}

	ZVAL_COPY(handler, data);
}
/* }}} */

/* {{{ xml_call_handler() */
static void xml_call_handler(xml_parser *parser, zval *handler, crex_function *function_ptr, int argc, zval *argv, zval *retval)
{
	int i;

	ZVAL_UNDEF(retval);
	if (parser && handler && !EG(exception)) {
		int result;
		crex_fcall_info fci;

		fci.size = sizeof(fci);
		ZVAL_COPY_VALUE(&fci.function_name, handler);
		fci.object = C_OBJ(parser->object);
		fci.retval = retval;
		fci.param_count = argc;
		fci.params = argv;
		fci.named_params = NULL;

		result = crex_call_function(&fci, NULL);
		if (result == FAILURE) {
			zval *method;
			zval *obj;

			if (C_TYPE_P(handler) == IS_STRING) {
				crx_error_docref(NULL, E_WARNING, "Unable to call handler %s()", C_STRVAL_P(handler));
			} else if (C_TYPE_P(handler) == IS_ARRAY &&
					   (obj = crex_hash_index_find(C_ARRVAL_P(handler), 0)) != NULL &&
					   (method = crex_hash_index_find(C_ARRVAL_P(handler), 1)) != NULL &&
					   C_TYPE_P(obj) == IS_OBJECT &&
					   C_TYPE_P(method) == IS_STRING) {
				crx_error_docref(NULL, E_WARNING, "Unable to call handler %s::%s()", ZSTR_VAL(C_OBJCE_P(obj)->name), C_STRVAL_P(method));
			} else
				crx_error_docref(NULL, E_WARNING, "Unable to call handler");
		}
	}
	for (i = 0; i < argc; i++) {
		zval_ptr_dtor(&argv[i]);
	}
}
/* }}} */

/* {{{ xml_encode_iso_8859_1() */
inline static unsigned short xml_encode_iso_8859_1(unsigned char c)
{
	return (unsigned short)c;
}
/* }}} */

/* {{{ xml_decode_iso_8859_1() */
inline static char xml_decode_iso_8859_1(unsigned short c)
{
	return (char)(c > 0xff ? '?' : c);
}
/* }}} */

/* {{{ xml_encode_us_ascii() */
inline static unsigned short xml_encode_us_ascii(unsigned char c)
{
	return (unsigned short)c;
}
/* }}} */

/* {{{ xml_decode_us_ascii() */
inline static char xml_decode_us_ascii(unsigned short c)
{
	return (char)(c > 0x7f ? '?' : c);
}
/* }}} */

/* {{{ xml_get_encoding() */
static const xml_encoding *xml_get_encoding(const XML_Char *name)
{
	const xml_encoding *enc = &xml_encodings[0];

	while (enc && enc->name) {
		if (strcasecmp((char *)name, (char *)enc->name) == 0) {
			return enc;
		}
		enc++;
	}
	return NULL;
}
/* }}} */

/* {{{ xml_utf8_decode() */
static crex_string *xml_utf8_decode(const XML_Char *s, size_t len, const XML_Char *encoding)
{
	size_t pos = 0;
	unsigned int c;
	char (*decoder)(unsigned short) = NULL;
	const xml_encoding *enc = xml_get_encoding(encoding);
	crex_string *str;

	if (enc) {
		decoder = enc->decoding_function;
	}

	if (decoder == NULL) {
		/* If the target encoding was unknown, or no decoder function
		 * was specified, return the UTF-8-encoded data as-is.
		 */
		str = crex_string_init((char *)s, len, 0);
		return str;
	}

	str = crex_string_alloc(len, 0);
	ZSTR_LEN(str) = 0;
	while (pos < len) {
		crex_result status = FAILURE;
		c = crx_next_utf8_char((const unsigned char*)s, len, &pos, &status);

		if (status == FAILURE || c > 0xFFU) {
			c = '?';
		}

		ZSTR_VAL(str)[ZSTR_LEN(str)++] = (unsigned int)decoder(c);
	}
	ZSTR_VAL(str)[ZSTR_LEN(str)] = '\0';
	if (ZSTR_LEN(str) < len) {
		str = crex_string_truncate(str, ZSTR_LEN(str), 0);
	}

	return str;
}
/* }}} */

/* {{{ _xml_xmlcharlen() */
static int _xml_xmlcharlen(const XML_Char *s)
{
	int len = 0;

	while (*s) {
		len++;
		s++;
	}
	return len;
}
/* }}} */

/* {{{ _xml_add_to_info() */
static void _xml_add_to_info(xml_parser *parser, const char *name)
{
	zval *element;

	if (C_ISUNDEF(parser->info)) {
		return;
	}

	size_t name_len = strlen(name);
	if ((element = crex_hash_str_find(C_ARRVAL(parser->info), name, name_len)) == NULL) {
		zval values;
		array_init(&values);
		element = crex_hash_str_update(C_ARRVAL(parser->info), name, name_len, &values);
	}

	add_next_index_long(element, parser->curtag);

	parser->curtag++;
}
/* }}} */

/* {{{ _xml_decode_tag() */
static crex_string *_xml_decode_tag(xml_parser *parser, const XML_Char *tag)
{
	crex_string *str;

	str = xml_utf8_decode(tag, _xml_xmlcharlen(tag), parser->target_encoding);

	if (parser->case_folding) {
		crex_str_toupper(ZSTR_VAL(str), ZSTR_LEN(str));
	}

	return str;
}
/* }}} */

/* {{{ _xml_startElementHandler() */
void _xml_startElementHandler(void *userData, const XML_Char *name, const XML_Char **attributes)
{
	xml_parser *parser = (xml_parser *)userData;
	const char **attrs = (const char **) attributes;
	crex_string *att, *tag_name, *val;
	zval retval, args[3];

	if (!parser) {
		return;
	}

	parser->level++;

	tag_name = _xml_decode_tag(parser, name);

	if (!C_ISUNDEF(parser->startElementHandler)) {
		ZVAL_COPY(&args[0], &parser->index);
		ZVAL_STRING(&args[1], SKIP_TAGSTART(ZSTR_VAL(tag_name)));
		array_init(&args[2]);

		while (attributes && *attributes) {
			zval tmp;

			att = _xml_decode_tag(parser, attributes[0]);
			val = xml_utf8_decode(attributes[1], strlen((char *)attributes[1]), parser->target_encoding);

			ZVAL_STR(&tmp, val);
			crex_symtable_update(C_ARRVAL(args[2]), att, &tmp);

			attributes += 2;

			crex_string_release_ex(att, 0);
		}

		xml_call_handler(parser, &parser->startElementHandler, parser->startElementPtr, 3, args, &retval);
		zval_ptr_dtor(&retval);
	}

	if (!C_ISUNDEF(parser->data)) {
		if (parser->level <= XML_MAXLEVEL)  {
			zval tag, atr;
			int atcnt = 0;

			array_init(&tag);
			array_init(&atr);

			_xml_add_to_info(parser, ZSTR_VAL(tag_name) + parser->toffset);

			add_assoc_string(&tag, "tag", SKIP_TAGSTART(ZSTR_VAL(tag_name))); /* cast to avoid gcc-warning */
			add_assoc_string(&tag, "type", "open");
			add_assoc_long(&tag, "level", parser->level);

			parser->ltags[parser->level-1] = estrdup(ZSTR_VAL(tag_name));
			parser->lastwasopen = 1;

			attributes = (const XML_Char **) attrs;

			while (attributes && *attributes) {
				zval tmp;

				att = _xml_decode_tag(parser, attributes[0]);
				val = xml_utf8_decode(attributes[1], strlen((char *)attributes[1]), parser->target_encoding);

				ZVAL_STR(&tmp, val);
				crex_symtable_update(C_ARRVAL(atr), att, &tmp);

				atcnt++;
				attributes += 2;

				crex_string_release_ex(att, 0);
			}

			if (atcnt) {
				crex_hash_str_add(C_ARRVAL(tag), "attributes", sizeof("attributes") - 1, &atr);
			} else {
				zval_ptr_dtor(&atr);
			}

			parser->ctag = crex_hash_next_index_insert(C_ARRVAL(parser->data), &tag);
		} else if (parser->level == (XML_MAXLEVEL + 1)) {
						crx_error_docref(NULL, E_WARNING, "Maximum depth exceeded - Results truncated");
		}
	}

	crex_string_release_ex(tag_name, 0);
}
/* }}} */

/* {{{ _xml_endElementHandler() */
void _xml_endElementHandler(void *userData, const XML_Char *name)
{
	xml_parser *parser = (xml_parser *)userData;

	if (!parser) {
		return;
	}

	zval retval, args[2];

	crex_string *tag_name = _xml_decode_tag(parser, name);

	if (!C_ISUNDEF(parser->endElementHandler)) {
		ZVAL_COPY(&args[0], &parser->index);
		ZVAL_STRING(&args[1], SKIP_TAGSTART(ZSTR_VAL(tag_name)));

		xml_call_handler(parser, &parser->endElementHandler, parser->endElementPtr, 2, args, &retval);
		zval_ptr_dtor(&retval);
	}

	if (!C_ISUNDEF(parser->data)) {
		zval tag;

		if (parser->lastwasopen) {
			add_assoc_string(parser->ctag, "type", "complete");
		} else {
			array_init(&tag);

			_xml_add_to_info(parser, ZSTR_VAL(tag_name) + parser->toffset);

			add_assoc_string(&tag, "tag", SKIP_TAGSTART(ZSTR_VAL(tag_name))); /* cast to avoid gcc-warning */
			add_assoc_string(&tag, "type", "close");
			add_assoc_long(&tag, "level", parser->level);

			crex_hash_next_index_insert(C_ARRVAL(parser->data), &tag);
		}

		parser->lastwasopen = 0;
	}

	crex_string_release_ex(tag_name, 0);

	if ((parser->ltags) && (parser->level <= XML_MAXLEVEL)) {
		efree(parser->ltags[parser->level-1]);
	}

	parser->level--;
}
/* }}} */

/* {{{ _xml_characterDataHandler() */
void _xml_characterDataHandler(void *userData, const XML_Char *s, int len)
{
	xml_parser *parser = (xml_parser *)userData;

	if (!parser) {
		return;
	}

	zval retval, args[2];

	if (!C_ISUNDEF(parser->characterDataHandler)) {
		ZVAL_COPY(&args[0], &parser->index);
		_xml_xmlchar_zval(s, len, parser->target_encoding, &args[1]);
		xml_call_handler(parser, &parser->characterDataHandler, parser->characterDataPtr, 2, args, &retval);
		zval_ptr_dtor(&retval);
	}

	if (C_ISUNDEF(parser->data)) {
		return;
	}

	bool doprint = 0;
	crex_string *decoded_value;
	decoded_value = xml_utf8_decode(s, len, parser->target_encoding);
	if (parser->skipwhite) {
		for (size_t i = 0; i < ZSTR_LEN(decoded_value); i++) {
			switch (ZSTR_VAL(decoded_value)[i]) {
				case ' ':
				case '\t':
				case '\n':
					continue;
				default:
					doprint = 1;
					break;
			}
			if (doprint) {
				break;
			}
		}
	}
	if (parser->lastwasopen) {
		zval *myval;
		/* check if the current tag already has a value - if yes append to that! */
		if ((myval = crex_hash_find(C_ARRVAL_P(parser->ctag), ZSTR_KNOWN(CREX_STR_VALUE)))) {
			size_t newlen = C_STRLEN_P(myval) + ZSTR_LEN(decoded_value);
			C_STR_P(myval) = crex_string_extend(C_STR_P(myval), newlen, 0);
			strncpy(C_STRVAL_P(myval) + C_STRLEN_P(myval) - ZSTR_LEN(decoded_value),
					ZSTR_VAL(decoded_value), ZSTR_LEN(decoded_value) + 1);
			crex_string_release_ex(decoded_value, 0);
		} else {
			if (doprint || (! parser->skipwhite)) {
				add_assoc_str(parser->ctag, "value", decoded_value);
			} else {
				crex_string_release_ex(decoded_value, 0);
			}
		}
	} else {
		zval tag;
		zval *curtag, *mytype, *myval;
		CREX_HASH_REVERSE_FOREACH_VAL(C_ARRVAL(parser->data), curtag) {
			if ((mytype = crex_hash_str_find(C_ARRVAL_P(curtag),"type", sizeof("type") - 1))) {
				if (crex_string_equals_literal(C_STR_P(mytype), "cdata")) {
					if ((myval = crex_hash_find(C_ARRVAL_P(curtag), ZSTR_KNOWN(CREX_STR_VALUE)))) {
						size_t newlen = C_STRLEN_P(myval) + ZSTR_LEN(decoded_value);
						C_STR_P(myval) = crex_string_extend(C_STR_P(myval), newlen, 0);
						strncpy(C_STRVAL_P(myval) + C_STRLEN_P(myval) - ZSTR_LEN(decoded_value),
								ZSTR_VAL(decoded_value), ZSTR_LEN(decoded_value) + 1);
						crex_string_release_ex(decoded_value, 0);
						return;
					}
				}
			}
			break;
		} CREX_HASH_FOREACH_END();
		if (parser->level <= XML_MAXLEVEL && parser->level > 0 && (doprint || (! parser->skipwhite))) {
			array_init(&tag);
			_xml_add_to_info(parser,SKIP_TAGSTART(parser->ltags[parser->level-1]));
			add_assoc_string(&tag, "tag", SKIP_TAGSTART(parser->ltags[parser->level-1]));
			add_assoc_str(&tag, "value", decoded_value);
			add_assoc_string(&tag, "type", "cdata");
			add_assoc_long(&tag, "level", parser->level);
			crex_hash_next_index_insert(C_ARRVAL(parser->data), &tag);
		} else if (parser->level == (XML_MAXLEVEL + 1)) {
								crx_error_docref(NULL, E_WARNING, "Maximum depth exceeded - Results truncated");
		} else {
			crex_string_release_ex(decoded_value, 0);
		}
	}
}
/* }}} */

/* {{{ _xml_processingInstructionHandler() */
void _xml_processingInstructionHandler(void *userData, const XML_Char *target, const XML_Char *data)
{
	xml_parser *parser = (xml_parser *)userData;

	if (!parser || C_ISUNDEF(parser->processingInstructionHandler)) {
		return;
	}

	zval retval, args[3];

	ZVAL_COPY(&args[0], &parser->index);
	_xml_xmlchar_zval(target, 0, parser->target_encoding, &args[1]);
	_xml_xmlchar_zval(data, 0, parser->target_encoding, &args[2]);
	xml_call_handler(parser, &parser->processingInstructionHandler, parser->processingInstructionPtr, 3, args, &retval);
	zval_ptr_dtor(&retval);
}
/* }}} */

/* {{{ _xml_defaultHandler() */
void _xml_defaultHandler(void *userData, const XML_Char *s, int len)
{
	xml_parser *parser = (xml_parser *)userData;

	if (!parser || C_ISUNDEF(parser->defaultHandler)) {
		return;
	}

	zval retval, args[2];

	ZVAL_COPY(&args[0], &parser->index);
	_xml_xmlchar_zval(s, len, parser->target_encoding, &args[1]);
	xml_call_handler(parser, &parser->defaultHandler, parser->defaultPtr, 2, args, &retval);
	zval_ptr_dtor(&retval);
}
/* }}} */

/* {{{ _xml_unparsedEntityDeclHandler() */
void _xml_unparsedEntityDeclHandler(void *userData,
	const XML_Char *entityName, const XML_Char *base, const XML_Char *systemId,
	const XML_Char *publicId, const XML_Char *notationName)
{
	xml_parser *parser = (xml_parser *)userData;

	if (!parser || C_ISUNDEF(parser->unparsedEntityDeclHandler)) {
		return;
	}

	zval retval, args[6];

	ZVAL_COPY(&args[0], &parser->index);
	_xml_xmlchar_zval(entityName, 0, parser->target_encoding, &args[1]);
	_xml_xmlchar_zval(base, 0, parser->target_encoding, &args[2]);
	_xml_xmlchar_zval(systemId, 0, parser->target_encoding, &args[3]);
	_xml_xmlchar_zval(publicId, 0, parser->target_encoding, &args[4]);
	_xml_xmlchar_zval(notationName, 0, parser->target_encoding, &args[5]);
	xml_call_handler(parser, &parser->unparsedEntityDeclHandler, parser->unparsedEntityDeclPtr, 6, args, &retval);
	zval_ptr_dtor(&retval);
}
/* }}} */

/* {{{ _xml_notationDeclHandler() */
void _xml_notationDeclHandler(void *userData, const XML_Char *notationName,
	const XML_Char *base, const XML_Char *systemId, const XML_Char *publicId)
{
	xml_parser *parser = (xml_parser *)userData;

	if (!parser || C_ISUNDEF(parser->notationDeclHandler)) {
		return;
	}

	zval retval, args[5];

	ZVAL_COPY(&args[0], &parser->index);
	_xml_xmlchar_zval(notationName, 0, parser->target_encoding, &args[1]);
	_xml_xmlchar_zval(base, 0, parser->target_encoding, &args[2]);
	_xml_xmlchar_zval(systemId, 0, parser->target_encoding, &args[3]);
	_xml_xmlchar_zval(publicId, 0, parser->target_encoding, &args[4]);
	xml_call_handler(parser, &parser->notationDeclHandler, parser->notationDeclPtr, 5, args, &retval);
	zval_ptr_dtor(&retval);
}
/* }}} */

/* {{{ _xml_externalEntityRefHandler() */
int _xml_externalEntityRefHandler(XML_Parser parserPtr, const XML_Char *openEntityNames,
	const XML_Char *base, const XML_Char *systemId, const XML_Char *publicId)
{
	xml_parser *parser = XML_GetUserData(parserPtr);

	if (!parser || C_ISUNDEF(parser->externalEntityRefHandler)) {
		return 0;
	}

	int ret = 0; /* abort if no handler is set (should be configurable?) */
	zval retval, args[5];

	ZVAL_COPY(&args[0], &parser->index);
	_xml_xmlchar_zval(openEntityNames, 0, parser->target_encoding, &args[1]);
	_xml_xmlchar_zval(base, 0, parser->target_encoding, &args[2]);
	_xml_xmlchar_zval(systemId, 0, parser->target_encoding, &args[3]);
	_xml_xmlchar_zval(publicId, 0, parser->target_encoding, &args[4]);
	xml_call_handler(parser, &parser->externalEntityRefHandler, parser->externalEntityRefPtr, 5, args, &retval);
	if (!C_ISUNDEF(retval)) {
		convert_to_long(&retval);
		ret = C_LVAL(retval);
	} else {
		ret = 0;
	}

	return ret;
}
/* }}} */

/* {{{ _xml_startNamespaceDeclHandler() */
void _xml_startNamespaceDeclHandler(void *userData,const XML_Char *prefix, const XML_Char *uri)
{
	xml_parser *parser = (xml_parser *)userData;

	if (!parser || C_ISUNDEF(parser->startNamespaceDeclHandler)) {
		return;
	}

	zval retval, args[3];

	ZVAL_COPY(&args[0], &parser->index);
	_xml_xmlchar_zval(prefix, 0, parser->target_encoding, &args[1]);
	_xml_xmlchar_zval(uri, 0, parser->target_encoding, &args[2]);
	xml_call_handler(parser, &parser->startNamespaceDeclHandler, parser->startNamespaceDeclPtr, 3, args, &retval);
	zval_ptr_dtor(&retval);
}
/* }}} */

/* {{{ _xml_endNamespaceDeclHandler() */
void _xml_endNamespaceDeclHandler(void *userData, const XML_Char *prefix)
{
	xml_parser *parser = (xml_parser *)userData;

	if (!parser || C_ISUNDEF(parser->endNamespaceDeclHandler)) {
		return;
	}

	zval retval, args[2];

	ZVAL_COPY(&args[0], &parser->index);
	_xml_xmlchar_zval(prefix, 0, parser->target_encoding, &args[1]);
	xml_call_handler(parser, &parser->endNamespaceDeclHandler, parser->endNamespaceDeclPtr, 2, args, &retval);
	zval_ptr_dtor(&retval);
}
/* }}} */

/************************* EXTENSION FUNCTIONS *************************/

static void crx_xml_parser_create_impl(INTERNAL_FUNCTION_PARAMETERS, int ns_support) /* {{{ */
{
	xml_parser *parser;
	int auto_detect = 0;

	crex_string *encoding_param = NULL;

	char *ns_param = NULL;
	size_t ns_param_len = 0;

	XML_Char *encoding;

	if (crex_parse_parameters(CREX_NUM_ARGS(), (ns_support ? "|S!s": "|S!"), &encoding_param, &ns_param, &ns_param_len) == FAILURE) {
		RETURN_THROWS();
	}

	if (encoding_param != NULL) {
		/* The supported encoding types are hardcoded here because
		 * we are limited to the encodings supported by expat/xmltok.
		 */
		if (ZSTR_LEN(encoding_param) == 0) {
			encoding = XML(default_encoding);
			auto_detect = 1;
		} else if (crex_string_equals_literal_ci(encoding_param, "ISO-8859-1")) {
			encoding = (XML_Char*)"ISO-8859-1";
		} else if (crex_string_equals_literal_ci(encoding_param, "UTF-8")) {
			encoding = (XML_Char*)"UTF-8";
		} else if (crex_string_equals_literal_ci(encoding_param, "US-ASCII")) {
			encoding = (XML_Char*)"US-ASCII";
		} else {
			crex_argument_value_error(1, "is not a supported source encoding");
			RETURN_THROWS();
		}
	} else {
		encoding = XML(default_encoding);
	}

	if (ns_support && ns_param == NULL){
		ns_param = ":";
	}

	object_init_ex(return_value, xml_parser_ce);
	parser = C_XMLPARSER_P(return_value);
	parser->parser = XML_ParserCreate_MM((auto_detect ? NULL : encoding),
	                                     &crx_xml_mem_hdlrs, (XML_Char*)ns_param);

	parser->target_encoding = encoding;
	parser->case_folding = 1;
	parser->isparsing = 0;

	XML_SetUserData(parser->parser, parser);
	ZVAL_COPY_VALUE(&parser->index, return_value);
}
/* }}} */

/* {{{ Create an XML parser */
CRX_FUNCTION(xml_parser_create)
{
	crx_xml_parser_create_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ Create an XML parser */
CRX_FUNCTION(xml_parser_create_ns)
{
	crx_xml_parser_create_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ Set up object which should be used for callbacks */
CRX_FUNCTION(xml_set_object)
{
	xml_parser *parser;
	zval *pind, *mythis;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oo", &pind, xml_parser_ce, &mythis) == FAILURE) {
		RETURN_THROWS();
	}

	parser = C_XMLPARSER_P(pind);

	zval_ptr_dtor(&parser->object);
	ZVAL_OBJ_COPY(&parser->object, C_OBJ_P(mythis));

	RETURN_TRUE;
}
/* }}} */

/* {{{ Set up start and end element handlers */
CRX_FUNCTION(xml_set_element_handler)
{
	xml_parser *parser;
	zval *pind, *shdl, *ehdl;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ozz", &pind, xml_parser_ce, &shdl, &ehdl) == FAILURE) {
		RETURN_THROWS();
	}

	parser = C_XMLPARSER_P(pind);
	xml_set_handler(&parser->startElementHandler, shdl);
	xml_set_handler(&parser->endElementHandler, ehdl);
	XML_SetElementHandler(parser->parser, _xml_startElementHandler, _xml_endElementHandler);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Set up character data handler */
CRX_FUNCTION(xml_set_character_data_handler)
{
	xml_parser *parser;
	zval *pind, *hdl;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oz", &pind, xml_parser_ce, &hdl) == FAILURE) {
		RETURN_THROWS();
	}

	parser = C_XMLPARSER_P(pind);
	xml_set_handler(&parser->characterDataHandler, hdl);
	XML_SetCharacterDataHandler(parser->parser, _xml_characterDataHandler);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Set up processing instruction (PI) handler */
CRX_FUNCTION(xml_set_processing_instruction_handler)
{
	xml_parser *parser;
	zval *pind, *hdl;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oz", &pind, xml_parser_ce, &hdl) == FAILURE) {
		RETURN_THROWS();
	}

	parser = C_XMLPARSER_P(pind);
	xml_set_handler(&parser->processingInstructionHandler, hdl);
	XML_SetProcessingInstructionHandler(parser->parser, _xml_processingInstructionHandler);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Set up default handler */
CRX_FUNCTION(xml_set_default_handler)
{
	xml_parser *parser;
	zval *pind, *hdl;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oz", &pind, xml_parser_ce, &hdl) == FAILURE) {
		RETURN_THROWS();
	}

	parser = C_XMLPARSER_P(pind);
	xml_set_handler(&parser->defaultHandler, hdl);
	XML_SetDefaultHandler(parser->parser, _xml_defaultHandler);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Set up unparsed entity declaration handler */
CRX_FUNCTION(xml_set_unparsed_entity_decl_handler)
{
	xml_parser *parser;
	zval *pind, *hdl;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oz", &pind, xml_parser_ce, &hdl) == FAILURE) {
		RETURN_THROWS();
	}

	parser = C_XMLPARSER_P(pind);
	xml_set_handler(&parser->unparsedEntityDeclHandler, hdl);
	XML_SetUnparsedEntityDeclHandler(parser->parser, _xml_unparsedEntityDeclHandler);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Set up notation declaration handler */
CRX_FUNCTION(xml_set_notation_decl_handler)
{
	xml_parser *parser;
	zval *pind, *hdl;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oz", &pind, xml_parser_ce, &hdl) == FAILURE) {
		RETURN_THROWS();
	}

	parser = C_XMLPARSER_P(pind);
	xml_set_handler(&parser->notationDeclHandler, hdl);
	XML_SetNotationDeclHandler(parser->parser, _xml_notationDeclHandler);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Set up external entity reference handler */
CRX_FUNCTION(xml_set_external_entity_ref_handler)
{
	xml_parser *parser;
	zval *pind, *hdl;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oz", &pind, xml_parser_ce, &hdl) == FAILURE) {
		RETURN_THROWS();
	}

	parser = C_XMLPARSER_P(pind);
	xml_set_handler(&parser->externalEntityRefHandler, hdl);
	XML_SetExternalEntityRefHandler(parser->parser, (void *) _xml_externalEntityRefHandler);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Set up character data handler */
CRX_FUNCTION(xml_set_start_namespace_decl_handler)
{
	xml_parser *parser;
	zval *pind, *hdl;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oz", &pind, xml_parser_ce, &hdl) == FAILURE) {
		RETURN_THROWS();
	}

	parser = C_XMLPARSER_P(pind);
	xml_set_handler(&parser->startNamespaceDeclHandler, hdl);
	XML_SetStartNamespaceDeclHandler(parser->parser, _xml_startNamespaceDeclHandler);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Set up character data handler */
CRX_FUNCTION(xml_set_end_namespace_decl_handler)
{
	xml_parser *parser;
	zval *pind, *hdl;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oz", &pind, xml_parser_ce, &hdl) == FAILURE) {
		RETURN_THROWS();
	}

	parser = C_XMLPARSER_P(pind);
	xml_set_handler(&parser->endNamespaceDeclHandler, hdl);
	XML_SetEndNamespaceDeclHandler(parser->parser, _xml_endNamespaceDeclHandler);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Start parsing an XML document */
CRX_FUNCTION(xml_parse)
{
	xml_parser *parser;
	zval *pind;
	char *data;
	size_t data_len;
	int ret;
	bool isFinal = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Os|b", &pind, xml_parser_ce, &data, &data_len, &isFinal) == FAILURE) {
		RETURN_THROWS();
	}

	parser = C_XMLPARSER_P(pind);
	if (parser->isparsing) {
		crex_throw_error(NULL, "Parser must not be called recursively");
		RETURN_THROWS();
	}
	parser->isparsing = 1;
	ret = XML_Parse(parser->parser, (XML_Char*)data, data_len, isFinal);
	parser->isparsing = 0;
	RETVAL_LONG(ret);
}

/* }}} */

/* {{{ Parsing a XML document */
CRX_FUNCTION(xml_parse_into_struct)
{
	xml_parser *parser;
	zval *pind, *xdata, *info = NULL;
	char *data;
	size_t data_len;
	int ret;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Osz|z", &pind, xml_parser_ce, &data, &data_len, &xdata, &info) == FAILURE) {
		RETURN_THROWS();
	}

	parser = C_XMLPARSER_P(pind);

	if (parser->isparsing) {
		crx_error_docref(NULL, E_WARNING, "Parser must not be called recursively");
		RETURN_FALSE;
	}

	if (info) {
		info = crex_try_array_init(info);
		if (!info) {
			RETURN_THROWS();
		}
	}

	xdata = crex_try_array_init(xdata);
	if (!xdata) {
		RETURN_THROWS();
	}

	ZVAL_COPY_VALUE(&parser->data, xdata);

	if (info) {
		ZVAL_COPY_VALUE(&parser->info, info);
	}

	parser->level = 0;
	xml_parser_free_ltags(parser);
	parser->ltags = safe_emalloc(XML_MAXLEVEL, sizeof(char *), 0);

	XML_SetElementHandler(parser->parser, _xml_startElementHandler, _xml_endElementHandler);
	XML_SetCharacterDataHandler(parser->parser, _xml_characterDataHandler);

	parser->isparsing = 1;
	ret = XML_Parse(parser->parser, (XML_Char*)data, data_len, 1);
	parser->isparsing = 0;

	RETVAL_LONG(ret);
}
/* }}} */

/* {{{ Get XML parser error code */
CRX_FUNCTION(xml_get_error_code)
{
	xml_parser *parser;
	zval *pind;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &pind, xml_parser_ce) == FAILURE) {
		RETURN_THROWS();
	}

	parser = C_XMLPARSER_P(pind);
	RETURN_LONG((crex_long)XML_GetErrorCode(parser->parser));
}
/* }}} */

/* {{{ Get XML parser error string */
CRX_FUNCTION(xml_error_string)
{
	crex_long code;
	char *str;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &code) == FAILURE) {
		RETURN_THROWS();
	}

	str = (char *)XML_ErrorString((int)code);
	if (str) {
		RETVAL_STRING(str);
	}
}
/* }}} */

/* {{{ Get current line number for an XML parser */
CRX_FUNCTION(xml_get_current_line_number)
{
	xml_parser *parser;
	zval *pind;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &pind, xml_parser_ce) == FAILURE) {
		RETURN_THROWS();
	}

	parser = C_XMLPARSER_P(pind);
	RETVAL_LONG(XML_GetCurrentLineNumber(parser->parser));
}
/* }}} */

/* {{{ Get current column number for an XML parser */
CRX_FUNCTION(xml_get_current_column_number)
{
	xml_parser *parser;
	zval *pind;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &pind, xml_parser_ce) == FAILURE) {
		RETURN_THROWS();
	}

	parser = C_XMLPARSER_P(pind);
	RETVAL_LONG(XML_GetCurrentColumnNumber(parser->parser));
}
/* }}} */

/* {{{ Get current byte index for an XML parser */
CRX_FUNCTION(xml_get_current_byte_index)
{
	xml_parser *parser;
	zval *pind;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &pind, xml_parser_ce) == FAILURE) {
		RETURN_THROWS();
	}

	parser = C_XMLPARSER_P(pind);
	RETVAL_LONG(XML_GetCurrentByteIndex(parser->parser));
}
/* }}} */

/* {{{ Free an XML parser */
CRX_FUNCTION(xml_parser_free)
{
	zval *pind;
	xml_parser *parser;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &pind, xml_parser_ce) == FAILURE) {
		RETURN_THROWS();
	}

	parser = C_XMLPARSER_P(pind);
	if (parser->isparsing == 1) {
		crx_error_docref(NULL, E_WARNING, "Parser cannot be freed while it is parsing");
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Set options in an XML parser */
CRX_FUNCTION(xml_parser_set_option)
{
	xml_parser *parser;
	zval *pind;
	crex_long opt;
	zval *value;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Olz", &pind, xml_parser_ce, &opt, &value) == FAILURE) {
		RETURN_THROWS();
	}

	if (C_TYPE_P(value) != IS_FALSE && C_TYPE_P(value) != IS_TRUE &&
		C_TYPE_P(value) != IS_LONG && C_TYPE_P(value) != IS_STRING) {
		crx_error_docref(NULL, E_WARNING,
			"Argument #3 ($value) must be of type string|int|bool, %s given", crex_zval_type_name(value));
	}

	parser = C_XMLPARSER_P(pind);
	switch (opt) {
		/* Boolean option */
		case CRX_XML_OPTION_CASE_FOLDING:
			parser->case_folding = crex_is_true(value);
			break;
		/* Boolean option */
		case CRX_XML_OPTION_SKIP_WHITE:
			parser->skipwhite = crex_is_true(value);
			break;
		/* Integer option */
		case CRX_XML_OPTION_SKIP_TAGSTART:
			/* The tag start offset is stored in an int */
			/* TODO Improve handling of values? */
			parser->toffset = zval_get_long(value);
			if (parser->toffset < 0) {
				/* TODO Promote to ValueError in CRX 9.0 */
				crx_error_docref(NULL, E_WARNING, "Argument #3 ($value) must be between 0 and %d"
					" for option XML_OPTION_SKIP_TAGSTART", INT_MAX);
				parser->toffset = 0;
				RETURN_FALSE;
			}
			break;
		/* String option */
		case CRX_XML_OPTION_TARGET_ENCODING: {
			const xml_encoding *enc;
			if (!try_convert_to_string(value)) {
				RETURN_THROWS();
			}

			enc = xml_get_encoding((XML_Char*)C_STRVAL_P(value));
			if (enc == NULL) {
				crex_argument_value_error(3, "is not a supported target encoding");
				RETURN_THROWS();
			}

			parser->target_encoding = enc->name;
			break;
		}
		default:
			crex_argument_value_error(2, "must be a XML_OPTION_* constant");
			RETURN_THROWS();
			break;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Get options from an XML parser */
CRX_FUNCTION(xml_parser_get_option)
{
	xml_parser *parser;
	zval *pind;
	crex_long opt;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ol", &pind, xml_parser_ce, &opt) == FAILURE) {
		RETURN_THROWS();
	}

	parser = C_XMLPARSER_P(pind);
	switch (opt) {
		case CRX_XML_OPTION_CASE_FOLDING:
			RETURN_BOOL(parser->case_folding);
			break;
		case CRX_XML_OPTION_SKIP_TAGSTART:
			RETURN_LONG(parser->toffset);
			break;
		case CRX_XML_OPTION_SKIP_WHITE:
			RETURN_BOOL(parser->skipwhite);
			break;
		case CRX_XML_OPTION_TARGET_ENCODING:
			RETURN_STRING((char *)parser->target_encoding);
			break;
		default:
			crex_argument_value_error(2, "must be a XML_OPTION_* constant");
			RETURN_THROWS();
	}
}
/* }}} */

#endif
