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
   | Authors: Shane Caraveo <shane@crx.net>                               |
   |          Wez Furlong <wez@thebrainroom.com>                          |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_LIBXML_H
#define CRX_LIBXML_H

#ifdef HAVE_LIBXML
extern crex_module_entry libxml_module_entry;
#define libxml_module_ptr &libxml_module_entry

#include "crx_version.h"
#define CRX_LIBXML_VERSION CRX_VERSION

#ifdef CRX_WIN32
#	define CRX_LIBXML_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define CRX_LIBXML_API __attribute__ ((visibility("default")))
#else
#	define CRX_LIBXML_API
#endif

#include "crex_smart_str.h"
#include <libxml/tree.h>
#include <libxml/parser.h>

#define LIBXML_SAVE_NOEMPTYTAG 1<<2

CREX_BEGIN_MODULE_GLOBALS(libxml)
	zval stream_context;
	smart_str error_buffer;
	crex_llist *error_list;
	crex_fcall_info_cache entity_loader_callback;
	bool entity_loader_disabled;
CREX_END_MODULE_GLOBALS(libxml)

typedef struct _libxml_doc_props {
	HashTable *classmap;
	bool formatoutput;
	bool validateonparse;
	bool resolveexternals;
	bool preservewhitespace;
	bool substituteentities;
	bool stricterror;
	bool recover;
} libxml_doc_props;

typedef struct {
	size_t modification_nr;
} crx_libxml_cache_tag;

typedef struct _crx_libxml_ref_obj {
	void *ptr;
	int   refcount;
	libxml_doc_props *doc_props;
	crx_libxml_cache_tag cache_tag;
} crx_libxml_ref_obj;

typedef struct _crx_libxml_node_ptr {
	xmlNodePtr node;
	int	refcount;
	void *_private;
} crx_libxml_node_ptr;

typedef struct _crx_libxml_node_object {
	crx_libxml_node_ptr *node;
	crx_libxml_ref_obj *document;
	HashTable *properties;
	crex_object  std;
} crx_libxml_node_object;


static inline crx_libxml_node_object *crx_libxml_node_fetch_object(crex_object *obj) {
	return (crx_libxml_node_object *)((char*)(obj) - obj->handlers->offset);
}

static crex_always_inline void crx_libxml_invalidate_node_list_cache(crx_libxml_ref_obj *doc_ptr)
{
	if (!doc_ptr) {
		return;
	}
#if SIZEOF_SIZE_T == 8
	/* If one operation happens every nanosecond, then it would still require 584 years to overflow
	 * the counter. So we'll just assume this never happens. */
	doc_ptr->cache_tag.modification_nr++;
#else
	size_t new_modification_nr = doc_ptr->cache_tag.modification_nr + 1;
	if (EXPECTED(new_modification_nr > 0)) { /* unsigned overflow; checking after addition results in one less instruction */
		doc_ptr->cache_tag.modification_nr = new_modification_nr;
	}
#endif
}

static crex_always_inline void crx_libxml_invalidate_node_list_cache_from_doc(xmlDocPtr docp)
{
	if (docp && docp->_private) { /* docp is NULL for detached nodes */
		crx_libxml_node_ptr *node_private = (crx_libxml_node_ptr *) docp->_private;
		crx_libxml_node_object *object_private = (crx_libxml_node_object *) node_private->_private;
		if (object_private) {
			crx_libxml_invalidate_node_list_cache(object_private->document);
		}
	}
}

#define C_LIBXML_NODE_P(zv) crx_libxml_node_fetch_object(C_OBJ_P((zv)))

typedef void * (*crx_libxml_export_node) (zval *object);

CRX_LIBXML_API int crx_libxml_increment_node_ptr(crx_libxml_node_object *object, xmlNodePtr node, void *private_data);
CRX_LIBXML_API int crx_libxml_decrement_node_ptr(crx_libxml_node_object *object);
CRX_LIBXML_API int crx_libxml_increment_doc_ref(crx_libxml_node_object *object, xmlDocPtr docp);
CRX_LIBXML_API int crx_libxml_decrement_doc_ref(crx_libxml_node_object *object);
CRX_LIBXML_API xmlNodePtr crx_libxml_import_node(zval *object);
CRX_LIBXML_API zval *crx_libxml_register_export(crex_class_entry *ce, crx_libxml_export_node export_function);
/* When an explicit freeing of node and children is required */
CRX_LIBXML_API void crx_libxml_node_free_list(xmlNodePtr node);
CRX_LIBXML_API void crx_libxml_node_free_resource(xmlNodePtr node);
/* When object dtor is called as node may still be referenced */
CRX_LIBXML_API void crx_libxml_node_decrement_resource(crx_libxml_node_object *object);
CRX_LIBXML_API void crx_libxml_error_handler(void *ctx, const char *msg, ...);
CRX_LIBXML_API void crx_libxml_ctx_warning(void *ctx, const char *msg, ...);
CRX_LIBXML_API void crx_libxml_ctx_error(void *ctx, const char *msg, ...);
CRX_LIBXML_API int crx_libxml_xmlCheckUTF8(const unsigned char *s);
CRX_LIBXML_API void crx_libxml_switch_context(zval *context, zval *oldcontext);
CRX_LIBXML_API void crx_libxml_issue_error(int level, const char *msg);
CRX_LIBXML_API bool crx_libxml_disable_entity_loader(bool disable);
CRX_LIBXML_API void crx_libxml_set_old_ns(xmlDocPtr doc, xmlNsPtr ns);

/* Init/shutdown functions*/
CRX_LIBXML_API void crx_libxml_initialize(void);
CRX_LIBXML_API void crx_libxml_shutdown(void);

#define LIBXML(v) CREX_MODULE_GLOBALS_ACCESSOR(libxml, v)

#if defined(ZTS) && defined(COMPILE_DL_LIBXML)
CREX_TSRMLS_CACHE_EXTERN()
#endif

/* Other extension may override the global state options, these global options
 * are copied initially to ctxt->options. Set the options to a known good value.
 * See libxml2 globals.c and parserInternals.c.
 * The unique_name argument allows multiple sanitizes and restores within the
 * same function, even nested is necessary. */
#define CRX_LIBXML_SANITIZE_GLOBALS(unique_name) \
	int xml_old_loadsubset_##unique_name = xmlLoadExtDtdDefaultValue; \
	xmlLoadExtDtdDefaultValue = 0; \
	int xml_old_validate_##unique_name = xmlDoValidityCheckingDefaultValue; \
	xmlDoValidityCheckingDefaultValue = 0; \
	int xml_old_pedantic_##unique_name = xmlPedanticParserDefault(0); \
	int xml_old_substitute_##unique_name = xmlSubstituteEntitiesDefault(0); \
	int xml_old_linenrs_##unique_name = xmlLineNumbersDefault(0); \
	int xml_old_blanks_##unique_name = xmlKeepBlanksDefault(1);

#define CRX_LIBXML_RESTORE_GLOBALS(unique_name) \
	xmlLoadExtDtdDefaultValue = xml_old_loadsubset_##unique_name; \
	xmlDoValidityCheckingDefaultValue = xml_old_validate_##unique_name; \
	(void) xmlPedanticParserDefault(xml_old_pedantic_##unique_name); \
	(void) xmlSubstituteEntitiesDefault(xml_old_substitute_##unique_name); \
	(void) xmlLineNumbersDefault(xml_old_linenrs_##unique_name); \
	(void) xmlKeepBlanksDefault(xml_old_blanks_##unique_name);

/* Alternative for above, working directly on the context and not setting globals.
 * Generally faster because no locking is involved, and this has the advantage that it sets the options to a known good value. */
static crex_always_inline void crx_libxml_sanitize_parse_ctxt_options(xmlParserCtxtPtr ctxt)
{
	ctxt->loadsubset = 0;
	ctxt->validate = 0;
	ctxt->pedantic = 0;
	ctxt->replaceEntities = 0;
	ctxt->linenumbers = 0;
	ctxt->keepBlanks = 1;
	ctxt->options = 0;
}

#else /* HAVE_LIBXML */
#define libxml_module_ptr NULL
#endif

#define crxext_libxml_ptr libxml_module_ptr

#endif /* CRX_LIBXML_H */
