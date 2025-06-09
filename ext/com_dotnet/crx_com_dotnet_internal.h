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
   | Author: Wez Furlong <wez@thebrainroom.com>                           |
   +----------------------------------------------------------------------+
 */

#ifndef CRX_COM_DOTNET_INTERNAL_H
#define CRX_COM_DOTNET_INTERNAL_H

#define _WIN32_DCOM
#define COBJMACROS
#include <ocidl.h>
#include <oleauto.h>
#include <unknwn.h>
#include <dispex.h>
#include "win32/winutil.h"

typedef struct _crx_com_dotnet_object {
	crex_object zo;

	VARIANT v;
	bool modified;

	int code_page;

	ITypeInfo *typeinfo;

	crex_class_entry *ce;

   	/* associated event sink */
	IDispatch *sink_dispatch;
	GUID sink_id;
	DWORD sink_cookie;

	/* cache for method signatures */
	HashTable *method_cache;
	/* cache for name -> DISPID */
	HashTable *id_of_name_cache;
} crx_com_dotnet_object;

static inline bool crx_com_is_valid_object(zval *zv)
{
	crex_class_entry *ce = C_OBJCE_P(zv);
	return crex_string_equals_literal(ce->name, "com") ||
		crex_string_equals_literal(ce->name, "dotnet") ||
		crex_string_equals_literal(ce->name, "variant");
}

#define CDNO_FETCH(zv)			(crx_com_dotnet_object*)C_OBJ_P(zv)
#define CDNO_FETCH_VERIFY(obj, zv)	do { \
	if (!crx_com_is_valid_object(zv)) { \
		crx_com_throw_exception(E_UNEXPECTED, "expected a variant object"); \
		return; \
	} \
	obj = (crx_com_dotnet_object*)C_OBJ_P(zv); \
} while(0)

/* com_extension.c */
crex_class_entry *crx_com_variant_class_entry, *crx_com_exception_class_entry, *crx_com_saproxy_class_entry;

/* com_handlers.c */
crex_object* crx_com_object_new(crex_class_entry *ce);
crex_object* crx_com_object_clone(crex_object *object);
void crx_com_object_free_storage(crex_object *object);
extern crex_object_handlers crx_com_object_handlers;
void crx_com_object_enable_event_sink(crx_com_dotnet_object *obj, bool enable);

/* com_saproxy.c */
crex_object_iterator *crx_com_saproxy_iter_get(crex_class_entry *ce, zval *object, int by_ref);
void crx_com_saproxy_create(crex_object *com_object, zval *proxy_out, zval *index);
extern crex_object_handlers crx_com_saproxy_handlers;

/* com_olechar.c */
CRX_COM_DOTNET_API crex_string *crx_com_olestring_to_string(OLECHAR *olestring, int codepage);
CRX_COM_DOTNET_API OLECHAR *crx_com_string_to_olestring(const char *string,
		size_t string_len, int codepage);
BSTR crx_com_string_to_bstr(crex_string *string, int codepage);
crex_string *crx_com_bstr_to_string(BSTR bstr, int codepage);


/* com_com.c */
CRX_METHOD(com, __main);

HRESULT crx_com_invoke_helper(crx_com_dotnet_object *obj, DISPID id_member,
		WORD flags, DISPPARAMS *disp_params, VARIANT *v, bool silent, bool allow_noarg);
HRESULT crx_com_get_id_of_name(crx_com_dotnet_object *obj, crex_string *name,
		DISPID *dispid);
crex_result crx_com_do_invoke_by_id(crx_com_dotnet_object *obj, DISPID dispid,
		WORD flags,	VARIANT *v, int nargs, zval *args, bool silent, bool allow_noarg);
crex_result crx_com_do_invoke(crx_com_dotnet_object *obj, crex_string *name,
		WORD flags,	VARIANT *v, int nargs, zval *args, bool allow_noarg);
crex_result crx_com_do_invoke_byref(crx_com_dotnet_object *obj, crex_internal_function *f,
		WORD flags,	VARIANT *v, int nargs, zval *args);

/* com_wrapper.c */
int crx_com_wrapper_minit(INIT_FUNC_ARGS);
CRX_COM_DOTNET_API IDispatch *crx_com_wrapper_export_as_sink(zval *val, GUID *sinkid, HashTable *id_to_name);
CRX_COM_DOTNET_API IDispatch *crx_com_wrapper_export(zval *val);

/* com_persist.c */
void crx_com_persist_minit(INIT_FUNC_ARGS);

/* com_variant.c */
CRX_METHOD(variant, __main);

CRX_COM_DOTNET_API void crx_com_variant_from_zval_with_type(VARIANT *v, zval *z, VARTYPE type, int codepage);
CRX_COM_DOTNET_API void crx_com_variant_from_zval(VARIANT *v, zval *z, int codepage);
CRX_COM_DOTNET_API crex_result crx_com_zval_from_variant(zval *z, VARIANT *v, int codepage);
CRX_COM_DOTNET_API crex_result crx_com_copy_variant(VARIANT *dst, VARIANT *src);

/* com_dotnet.c */
CRX_METHOD(dotnet, __main);
void crx_com_dotnet_rshutdown(void);
void crx_com_dotnet_mshutdown(void);

/* com_misc.c */
void crx_com_throw_exception(HRESULT code, char *message);
CRX_COM_DOTNET_API void crx_com_wrap_dispatch(zval *z, IDispatch *disp,
		int codepage);
CRX_COM_DOTNET_API void crx_com_wrap_variant(zval *z, VARIANT *v,
		int codepage);
CRX_COM_DOTNET_API bool crx_com_safearray_get_elem(VARIANT *array, VARIANT *dest, LONG dim1);

/* com_typeinfo.c */
CRX_COM_DOTNET_API ITypeLib *crx_com_load_typelib_via_cache(const char *search_string, int codepage);
CRX_COM_DOTNET_API ITypeLib *crx_com_load_typelib(char *search_string, int codepage);
CRX_COM_DOTNET_API crex_result crx_com_import_typelib(ITypeLib *TL, int mode, int codepage);
void crx_com_typelibrary_dtor(zval *pDest);
ITypeInfo *crx_com_locate_typeinfo(crex_string *type_lib_name, crx_com_dotnet_object *obj,
		crex_string *dispatch_name, bool sink);
bool crx_com_process_typeinfo(ITypeInfo *typeinfo, HashTable *id_to_name, bool printdef, GUID *guid, int codepage);
ITypeLib *crx_com_cache_typelib(ITypeLib* TL, char *cache_key, crex_long cache_key_len);
CRX_MINIT_FUNCTION(com_typeinfo);
CRX_MSHUTDOWN_FUNCTION(com_typeinfo);

/* com_iterator.c */
crex_object_iterator *crx_com_iter_get(crex_class_entry *ce, zval *object, int by_ref);


#endif
