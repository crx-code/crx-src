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
   | Author: Dmitry Stogov <dmitry@crex.com>                              |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "crx.h"
#include "crx_ffi.h"
#include "ext/standard/info.h"
#include "crx_scandir.h"
#include "crex_exceptions.h"
#include "crex_closures.h"
#include "crex_weakrefs.h"
#include "main/SAPI.h"

#include <ffi.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef HAVE_LIBDL
#ifdef CRX_WIN32
#include "win32/param.h"
#include "win32/winutil.h"
#define GET_DL_ERROR()  crx_win_err()
#else
#include <sys/param.h>
#define GET_DL_ERROR()  DL_ERROR()
#endif
#endif

#ifdef HAVE_GLOB
#ifdef CRX_WIN32
#include "win32/glob.h"
#else
#include <glob.h>
#endif
#endif

#ifndef __BIGGEST_ALIGNMENT__
/* XXX need something better, perhaps with regard to SIMD, etc. */
# define __BIGGEST_ALIGNMENT__ sizeof(size_t)
#endif

CREX_DECLARE_MODULE_GLOBALS(ffi)

typedef enum _crex_ffi_tag_kind {
	CREX_FFI_TAG_ENUM,
	CREX_FFI_TAG_STRUCT,
	CREX_FFI_TAG_UNION
} crex_ffi_tag_kind;

static const char *crex_ffi_tag_kind_name[3] = {"enum", "struct", "union"};


typedef struct _crex_ffi_tag {
	crex_ffi_tag_kind      kind;
	crex_ffi_type         *type;
} crex_ffi_tag;

typedef enum _crex_ffi_type_kind {
	CREX_FFI_TYPE_VOID,
	CREX_FFI_TYPE_FLOAT,
	CREX_FFI_TYPE_DOUBLE,
#ifdef HAVE_LONG_DOUBLE
	CREX_FFI_TYPE_LONGDOUBLE,
#endif
	CREX_FFI_TYPE_UINT8,
	CREX_FFI_TYPE_SINT8,
	CREX_FFI_TYPE_UINT16,
	CREX_FFI_TYPE_SINT16,
	CREX_FFI_TYPE_UINT32,
	CREX_FFI_TYPE_SINT32,
	CREX_FFI_TYPE_UINT64,
	CREX_FFI_TYPE_SINT64,
	CREX_FFI_TYPE_ENUM,
	CREX_FFI_TYPE_BOOL,
	CREX_FFI_TYPE_CHAR,
	CREX_FFI_TYPE_POINTER,
	CREX_FFI_TYPE_FUNC,
	CREX_FFI_TYPE_ARRAY,
	CREX_FFI_TYPE_STRUCT,
} crex_ffi_type_kind;

#include "ffi_arginfo.h"

typedef enum _crex_ffi_flags {
	CREX_FFI_FLAG_CONST      = (1 << 0),
	CREX_FFI_FLAG_OWNED      = (1 << 1),
	CREX_FFI_FLAG_PERSISTENT = (1 << 2),
} crex_ffi_flags;

struct _crex_ffi_type {
	crex_ffi_type_kind     kind;
	size_t                 size;
	uint32_t               align;
	uint32_t               attr;
	union {
		struct {
			crex_string        *tag_name;
			crex_ffi_type_kind  kind;
		} enumeration;
		struct {
			crex_ffi_type *type;
			crex_long      length;
		} array;
		struct {
			crex_ffi_type *type;
		} pointer;
		struct {
			crex_string   *tag_name;
			HashTable      fields;
		} record;
		struct {
			crex_ffi_type *ret_type;
			HashTable     *args;
			ffi_abi        abi;
		} func;
	};
};

typedef struct _crex_ffi_field {
	size_t                 offset;
	bool              is_const;
	bool              is_nested; /* part of nested anonymous struct */
	uint8_t                first_bit;
	uint8_t                bits;
	crex_ffi_type         *type;
} crex_ffi_field;

typedef enum _crex_ffi_symbol_kind {
	CREX_FFI_SYM_TYPE,
	CREX_FFI_SYM_CONST,
	CREX_FFI_SYM_VAR,
	CREX_FFI_SYM_FUNC
} crex_ffi_symbol_kind;

typedef struct _crex_ffi_symbol {
	crex_ffi_symbol_kind   kind;
	bool              is_const;
	crex_ffi_type         *type;
	union {
		void *addr;
		int64_t value;
	};
} crex_ffi_symbol;

typedef struct _crex_ffi_scope {
	HashTable             *symbols;
	HashTable             *tags;
} crex_ffi_scope;

typedef struct _crex_ffi {
	crex_object            std;
	DL_HANDLE              lib;
	HashTable             *symbols;
	HashTable             *tags;
	bool              persistent;
} crex_ffi;

#define CREX_FFI_TYPE_OWNED        (1<<0)

#define CREX_FFI_TYPE(t) \
	((crex_ffi_type*)(((uintptr_t)(t)) & ~CREX_FFI_TYPE_OWNED))

#define CREX_FFI_TYPE_IS_OWNED(t) \
	(((uintptr_t)(t)) & CREX_FFI_TYPE_OWNED)

#define CREX_FFI_TYPE_MAKE_OWNED(t) \
	((crex_ffi_type*)(((uintptr_t)(t)) | CREX_FFI_TYPE_OWNED))

#define CREX_FFI_SIZEOF_ARG \
	MAX(FFI_SIZEOF_ARG, sizeof(double))

typedef struct _crex_ffi_cdata {
	crex_object            std;
	crex_ffi_type         *type;
	void                  *ptr;
	void                  *ptr_holder;
	crex_ffi_flags         flags;
} crex_ffi_cdata;

typedef struct _crex_ffi_ctype {
	crex_object            std;
	crex_ffi_type         *type;
} crex_ffi_ctype;

static crex_class_entry *crex_ffi_exception_ce;
static crex_class_entry *crex_ffi_parser_exception_ce;
static crex_class_entry *crex_ffi_ce;
static crex_class_entry *crex_ffi_cdata_ce;
static crex_class_entry *crex_ffi_ctype_ce;

static crex_object_handlers crex_ffi_handlers;
static crex_object_handlers crex_ffi_cdata_handlers;
static crex_object_handlers crex_ffi_cdata_value_handlers;
static crex_object_handlers crex_ffi_cdata_free_handlers;
static crex_object_handlers crex_ffi_ctype_handlers;

static crex_internal_function crex_ffi_new_fn;
static crex_internal_function crex_ffi_cast_fn;
static crex_internal_function crex_ffi_type_fn;

/* forward declarations */
static void _crex_ffi_type_dtor(crex_ffi_type *type);
static void crex_ffi_finalize_type(crex_ffi_dcl *dcl);
static bool crex_ffi_is_same_type(crex_ffi_type *type1, crex_ffi_type *type2);
static crex_ffi_type *crex_ffi_remember_type(crex_ffi_type *type);
static char *crex_ffi_parse_directives(const char *filename, char *code_pos, char **scope_name, char **lib, bool preload);
static CREX_FUNCTION(ffi_trampoline);
static CREX_COLD void crex_ffi_return_unsupported(crex_ffi_type *type);
static CREX_COLD void crex_ffi_pass_unsupported(crex_ffi_type *type);
static CREX_COLD void crex_ffi_assign_incompatible(zval *arg, crex_ffi_type *type);
static bool crex_ffi_is_compatible_type(crex_ffi_type *dst_type, crex_ffi_type *src_type);

#if FFI_CLOSURES
static void *crex_ffi_create_callback(crex_ffi_type *type, zval *value);
#endif

static crex_always_inline void crex_ffi_type_dtor(crex_ffi_type *type) /* {{{ */
{
	if (UNEXPECTED(CREX_FFI_TYPE_IS_OWNED(type))) {
		_crex_ffi_type_dtor(type);
		return;
	}
}
/* }}} */

static crex_always_inline void crex_ffi_object_init(crex_object *object, crex_class_entry *ce) /* {{{ */
{
	GC_SET_REFCOUNT(object, 1);
	GC_TYPE_INFO(object) = GC_OBJECT | (IS_OBJ_DESTRUCTOR_CALLED << GC_FLAGS_SHIFT);
	object->ce = ce;
	object->handlers = ce->default_object_handlers;
	object->properties = NULL;
	crex_objects_store_put(object);
}
/* }}} */

static crex_object *crex_ffi_cdata_new(crex_class_entry *class_type) /* {{{ */
{
	crex_ffi_cdata *cdata;

	cdata = emalloc(sizeof(crex_ffi_cdata));

	crex_ffi_object_init(&cdata->std, class_type);

	cdata->type = NULL;
	cdata->ptr = NULL;
	cdata->flags = 0;

	return &cdata->std;
}
/* }}} */

static bool crex_ffi_func_ptr_are_compatible(crex_ffi_type *dst_type, crex_ffi_type *src_type) /* {{{ */
{
	uint32_t dst_argc, src_argc, i;
	crex_ffi_type *dst_arg, *src_arg;

	CREX_ASSERT(dst_type->kind == CREX_FFI_TYPE_FUNC);
	CREX_ASSERT(src_type->kind == CREX_FFI_TYPE_FUNC);

	/* Ensure calling convention matches */
	if (dst_type->func.abi != src_type->func.abi) {
		return 0;
	}

	/* Ensure variadic attr matches */
	if ((dst_type->attr & CREX_FFI_ATTR_VARIADIC) != (src_type->attr & CREX_FFI_ATTR_VARIADIC)) {
		return 0;
	}

	/* Ensure same arg count */
	dst_argc = dst_type->func.args ? crex_hash_num_elements(dst_type->func.args) : 0;
	src_argc = src_type->func.args ? crex_hash_num_elements(src_type->func.args) : 0;
	if (dst_argc != src_argc) {
		return 0;
	}

	/* Ensure compatible ret_type */
	if (!crex_ffi_is_compatible_type(dst_type->func.ret_type, src_type->func.ret_type)) {
		return 0;
	}

	/* Ensure compatible args */
	for (i = 0; i < dst_argc; i++) {
		dst_arg = crex_hash_index_find_ptr(dst_type->func.args, i);
		src_arg = crex_hash_index_find_ptr(src_type->func.args, i);
		if (!crex_ffi_is_compatible_type(CREX_FFI_TYPE(dst_arg), CREX_FFI_TYPE(src_arg))) {
			return 0;
		}
	}

	return 1;
}
/* }}} */

static bool crex_ffi_is_compatible_type(crex_ffi_type *dst_type, crex_ffi_type *src_type) /* {{{ */
{
	while (1) {
		if (dst_type == src_type) {
			return 1;
		} else if (dst_type->kind == src_type->kind) {
			if (dst_type->kind < CREX_FFI_TYPE_POINTER) {
				return 1;
			} else if (dst_type->kind == CREX_FFI_TYPE_POINTER) {
				dst_type = CREX_FFI_TYPE(dst_type->pointer.type);
				src_type = CREX_FFI_TYPE(src_type->pointer.type);
				if (dst_type->kind == CREX_FFI_TYPE_VOID ||
				    src_type->kind == CREX_FFI_TYPE_VOID) {
				    return 1;
				} else if (dst_type->kind == CREX_FFI_TYPE_FUNC &&
				           src_type->kind == CREX_FFI_TYPE_FUNC) {
				    return crex_ffi_func_ptr_are_compatible(dst_type, src_type);
				}
			} else if (dst_type->kind == CREX_FFI_TYPE_ARRAY &&
			           (dst_type->array.length == src_type->array.length ||
			            dst_type->array.length == 0)) {
				dst_type = CREX_FFI_TYPE(dst_type->array.type);
				src_type = CREX_FFI_TYPE(src_type->array.type);
			} else {
				break;
			}
		} else if (dst_type->kind == CREX_FFI_TYPE_POINTER &&
		           src_type->kind == CREX_FFI_TYPE_ARRAY) {
			dst_type = CREX_FFI_TYPE(dst_type->pointer.type);
			src_type = CREX_FFI_TYPE(src_type->array.type);
			if (dst_type->kind == CREX_FFI_TYPE_VOID) {
			    return 1;
			}
		} else {
			break;
		}
	}
	return 0;
}
/* }}} */

static ffi_type* crex_ffi_face_struct_add_fields(ffi_type* t, crex_ffi_type *type, int *i, size_t size)
{
	crex_ffi_field *field;

	CREX_HASH_MAP_FOREACH_PTR(&type->record.fields, field) {
		switch (CREX_FFI_TYPE(field->type)->kind) {
			case CREX_FFI_TYPE_FLOAT:
				t->elements[(*i)++] = &ffi_type_float;
				break;
			case CREX_FFI_TYPE_DOUBLE:
				t->elements[(*i)++] = &ffi_type_double;
				break;
#ifdef HAVE_LONG_DOUBLE
			case CREX_FFI_TYPE_LONGDOUBLE:
				t->elements[(*i)++] = &ffi_type_longdouble;
				break;
#endif
			case CREX_FFI_TYPE_SINT8:
			case CREX_FFI_TYPE_UINT8:
			case CREX_FFI_TYPE_BOOL:
			case CREX_FFI_TYPE_CHAR:
				t->elements[(*i)++] = &ffi_type_uint8;
				break;
			case CREX_FFI_TYPE_SINT16:
			case CREX_FFI_TYPE_UINT16:
				t->elements[(*i)++] = &ffi_type_uint16;
				break;
			case CREX_FFI_TYPE_SINT32:
			case CREX_FFI_TYPE_UINT32:
				t->elements[(*i)++] = &ffi_type_uint32;
				break;
			case CREX_FFI_TYPE_SINT64:
			case CREX_FFI_TYPE_UINT64:
				t->elements[(*i)++] = &ffi_type_uint64;
				break;
			case CREX_FFI_TYPE_POINTER:
				t->elements[(*i)++] = &ffi_type_pointer;
				break;
			case CREX_FFI_TYPE_STRUCT: {
				crex_ffi_type *field_type = CREX_FFI_TYPE(field->type);
				/* for unions we use only the first field */
				uint32_t num_fields = !(field_type->attr & CREX_FFI_ATTR_UNION) ?
					crex_hash_num_elements(&field_type->record.fields) : 1;

				if (num_fields > 1) {
					size += sizeof(ffi_type*) * (num_fields - 1);
					t = erealloc(t, size);
					t->elements = (ffi_type**)(t + 1);
				}
				t = crex_ffi_face_struct_add_fields(t, field_type, i, size);
				break;
			}
			default:
				t->elements[(*i)++] = &ffi_type_void;
				break;
		}
		if (type->attr & CREX_FFI_ATTR_UNION) {
			/* for unions we use only the first field */
			break;
		}
	} CREX_HASH_FOREACH_END();
	return t;
}

static ffi_type *crex_ffi_make_fake_struct_type(crex_ffi_type *type) /* {{{ */
{
	/* for unions we use only the first field */
	uint32_t num_fields = !(type->attr & CREX_FFI_ATTR_UNION) ?
		crex_hash_num_elements(&type->record.fields) : 1;
	size_t size = sizeof(ffi_type) + sizeof(ffi_type*) * (num_fields + 1);
	ffi_type *t = emalloc(size);
	int i;

	t->size = type->size;
	t->alignment = type->align;
	t->type = FFI_TYPE_STRUCT;
	t->elements = (ffi_type**)(t + 1);
	i = 0;
	t = crex_ffi_face_struct_add_fields(t, type, &i, size);
	t->elements[i] = NULL;
	return t;
}
/* }}} */

static ffi_type *crex_ffi_get_type(crex_ffi_type *type) /* {{{ */
{
	crex_ffi_type_kind kind = type->kind;

again:
	switch (kind) {
		case CREX_FFI_TYPE_FLOAT:
			return &ffi_type_float;
		case CREX_FFI_TYPE_DOUBLE:
			return &ffi_type_double;
#ifdef HAVE_LONG_DOUBLE
		case CREX_FFI_TYPE_LONGDOUBLE:
			return &ffi_type_longdouble;
#endif
		case CREX_FFI_TYPE_UINT8:
			return &ffi_type_uint8;
		case CREX_FFI_TYPE_SINT8:
			return &ffi_type_sint8;
		case CREX_FFI_TYPE_UINT16:
			return &ffi_type_uint16;
		case CREX_FFI_TYPE_SINT16:
			return &ffi_type_sint16;
		case CREX_FFI_TYPE_UINT32:
			return &ffi_type_uint32;
		case CREX_FFI_TYPE_SINT32:
			return &ffi_type_sint32;
		case CREX_FFI_TYPE_UINT64:
			return &ffi_type_uint64;
		case CREX_FFI_TYPE_SINT64:
			return &ffi_type_sint64;
		case CREX_FFI_TYPE_POINTER:
			return &ffi_type_pointer;
		case CREX_FFI_TYPE_VOID:
			return &ffi_type_void;
		case CREX_FFI_TYPE_BOOL:
			return &ffi_type_uint8;
		case CREX_FFI_TYPE_CHAR:
			return &ffi_type_sint8;
		case CREX_FFI_TYPE_ENUM:
			kind = type->enumeration.kind;
			goto again;
		case CREX_FFI_TYPE_STRUCT:
			return crex_ffi_make_fake_struct_type(type);
		default:
			break;
	}
	return NULL;
}
/* }}} */

static crex_never_inline crex_ffi_cdata *crex_ffi_cdata_to_zval_slow(void *ptr, crex_ffi_type *type, crex_ffi_flags flags) /* {{{ */
{
	crex_ffi_cdata *cdata = emalloc(sizeof(crex_ffi_cdata));

	crex_ffi_object_init(&cdata->std, crex_ffi_cdata_ce);
	cdata->std.handlers =
		(type->kind < CREX_FFI_TYPE_POINTER) ?
		&crex_ffi_cdata_value_handlers :
		&crex_ffi_cdata_handlers;
	cdata->type = type;
	cdata->flags = flags;
	cdata->ptr = ptr;
	return cdata;
}
/* }}} */

static crex_never_inline crex_ffi_cdata *crex_ffi_cdata_to_zval_slow_ptr(void *ptr, crex_ffi_type *type, crex_ffi_flags flags) /* {{{ */
{
	crex_ffi_cdata *cdata = emalloc(sizeof(crex_ffi_cdata));

	crex_ffi_object_init(&cdata->std, crex_ffi_cdata_ce);
	cdata->type = type;
	cdata->flags = flags;
	cdata->ptr = (void*)&cdata->ptr_holder;
	*(void**)cdata->ptr = *(void**)ptr;
	return cdata;
}
/* }}} */

static crex_never_inline crex_ffi_cdata *crex_ffi_cdata_to_zval_slow_ret(void *ptr, crex_ffi_type *type, crex_ffi_flags flags) /* {{{ */
{
	crex_ffi_cdata *cdata = emalloc(sizeof(crex_ffi_cdata));

	crex_ffi_object_init(&cdata->std, crex_ffi_cdata_ce);
	cdata->std.handlers =
		(type->kind < CREX_FFI_TYPE_POINTER) ?
		&crex_ffi_cdata_value_handlers :
		&crex_ffi_cdata_handlers;
	cdata->type = type;
	cdata->flags = flags;
	if (type->kind == CREX_FFI_TYPE_POINTER) {
		cdata->ptr = (void*)&cdata->ptr_holder;
		*(void**)cdata->ptr = *(void**)ptr;
	} else if (type->kind == CREX_FFI_TYPE_STRUCT) {
		cdata->ptr = emalloc(type->size);
		cdata->flags |= CREX_FFI_FLAG_OWNED;
		memcpy(cdata->ptr, ptr, type->size);
	} else {
		cdata->ptr = ptr;
	}
	return cdata;
}
/* }}} */

static crex_always_inline void crex_ffi_cdata_to_zval(crex_ffi_cdata *cdata, void *ptr, crex_ffi_type *type, int read_type, zval *rv, crex_ffi_flags flags, bool is_ret, bool debug_union) /* {{{ */
{
	if (read_type == BP_VAR_R) {
		crex_ffi_type_kind kind = type->kind;

again:
	    switch (kind) {
			case CREX_FFI_TYPE_FLOAT:
				ZVAL_DOUBLE(rv, *(float*)ptr);
				return;
			case CREX_FFI_TYPE_DOUBLE:
				ZVAL_DOUBLE(rv, *(double*)ptr);
				return;
#ifdef HAVE_LONG_DOUBLE
			case CREX_FFI_TYPE_LONGDOUBLE:
				ZVAL_DOUBLE(rv, *(long double*)ptr);
				return;
#endif
			case CREX_FFI_TYPE_UINT8:
				ZVAL_LONG(rv, *(uint8_t*)ptr);
				return;
			case CREX_FFI_TYPE_SINT8:
				ZVAL_LONG(rv, *(int8_t*)ptr);
				return;
			case CREX_FFI_TYPE_UINT16:
				ZVAL_LONG(rv, *(uint16_t*)ptr);
				return;
			case CREX_FFI_TYPE_SINT16:
				ZVAL_LONG(rv, *(int16_t*)ptr);
				return;
			case CREX_FFI_TYPE_UINT32:
				ZVAL_LONG(rv, *(uint32_t*)ptr);
				return;
			case CREX_FFI_TYPE_SINT32:
				ZVAL_LONG(rv, *(int32_t*)ptr);
				return;
			case CREX_FFI_TYPE_UINT64:
				ZVAL_LONG(rv, *(uint64_t*)ptr);
				return;
			case CREX_FFI_TYPE_SINT64:
				ZVAL_LONG(rv, *(int64_t*)ptr);
				return;
			case CREX_FFI_TYPE_BOOL:
				ZVAL_BOOL(rv, *(uint8_t*)ptr);
				return;
			case CREX_FFI_TYPE_CHAR:
				ZVAL_CHAR(rv, *(char*)ptr);
				return;
			case CREX_FFI_TYPE_ENUM:
				kind = type->enumeration.kind;
				goto again;
			case CREX_FFI_TYPE_POINTER:
				if (*(void**)ptr == NULL) {
					ZVAL_NULL(rv);
					return;
				} else if (debug_union) {
					ZVAL_STR(rv, crex_strpprintf(0, "%p", *(void**)ptr));
					return;
				} else if ((type->attr & CREX_FFI_ATTR_CONST) && CREX_FFI_TYPE(type->pointer.type)->kind == CREX_FFI_TYPE_CHAR) {
					ZVAL_STRING(rv, *(char**)ptr);
					return;
				}
				if (!cdata) {
					if (is_ret) {
						cdata = crex_ffi_cdata_to_zval_slow_ret(ptr, type, flags);
					} else {
						cdata = crex_ffi_cdata_to_zval_slow_ptr(ptr, type, flags);
					}
				} else {
					GC_ADDREF(&cdata->std);
				}
				ZVAL_OBJ(rv, &cdata->std);
				return;
			default:
				break;
		}
	}

	if (!cdata) {
		if (is_ret) {
			cdata = crex_ffi_cdata_to_zval_slow_ret(ptr, type, flags);
		} else {
			cdata = crex_ffi_cdata_to_zval_slow(ptr, type, flags);
		}
	} else {
		GC_ADDREF(&cdata->std);
	}
	ZVAL_OBJ(rv, &cdata->std);
}
/* }}} */

static uint64_t crex_ffi_bit_field_read(void *ptr, crex_ffi_field *field) /* {{{ */
{
	size_t bit = field->first_bit;
	size_t last_bit = bit + field->bits - 1;
	uint8_t *p = (uint8_t *) ptr + bit / 8;
	uint8_t *last_p = (uint8_t *) ptr + last_bit / 8;
	size_t pos = bit % 8;
	size_t insert_pos = 0;
	uint8_t mask;
	uint64_t val = 0;

	/* Bitfield fits into a single byte */
	if (p == last_p) {
		mask = (1U << field->bits) - 1U;
		return (*p >> pos) & mask;
	}

	/* Read partial prefix byte */
	if (pos != 0) {
		size_t num_bits = 8 - pos;
		mask = (1U << num_bits) - 1U;
		val = (*p++ >> pos) & mask;
		insert_pos += num_bits;
	}

	/* Read full bytes */
	while (p < last_p) {
		val |= (uint64_t) *p++ << insert_pos;
		insert_pos += 8;
	}

	/* Read partial suffix byte */
	if (p == last_p) {
		size_t num_bits = last_bit % 8 + 1;
		mask = (1U << num_bits) - 1U;
		val |= (uint64_t) (*p & mask) << insert_pos;
	}

	return val;
}
/* }}} */

static void crex_ffi_bit_field_to_zval(void *ptr, crex_ffi_field *field, zval *rv) /* {{{ */
{
	uint64_t val = crex_ffi_bit_field_read(ptr, field);
	if (CREX_FFI_TYPE(field->type)->kind == CREX_FFI_TYPE_CHAR
	 || CREX_FFI_TYPE(field->type)->kind == CREX_FFI_TYPE_SINT8
	 || CREX_FFI_TYPE(field->type)->kind == CREX_FFI_TYPE_SINT16
	 || CREX_FFI_TYPE(field->type)->kind == CREX_FFI_TYPE_SINT32
	 || CREX_FFI_TYPE(field->type)->kind == CREX_FFI_TYPE_SINT64) {
		/* Sign extend */
		uint64_t shift = 64 - (field->bits % 64);
		if (shift != 0) {
			val = (int64_t)(val << shift) >> shift;
		}
	}
	ZVAL_LONG(rv, val);
}
/* }}} */

static void crex_ffi_zval_to_bit_field(void *ptr, crex_ffi_field *field, zval *value) /* {{{ */
{
	uint64_t val = zval_get_long(value);
	size_t bit = field->first_bit;
	size_t last_bit = bit + field->bits - 1;
	uint8_t *p = (uint8_t *) ptr + bit / 8;
	uint8_t *last_p = (uint8_t *) ptr + last_bit / 8;
	size_t pos = bit % 8;
	uint8_t mask;

	/* Bitfield fits into a single byte */
	if (p == last_p) {
		mask = ((1U << field->bits) - 1U) << pos;
		*p = (*p & ~mask) | ((val << pos) & mask);
		return;
	}

	/* Write partial prefix byte */
	if (pos != 0) {
		size_t num_bits = 8 - pos;
		mask = ((1U << num_bits) - 1U) << pos;
		*p = (*p & ~mask) | ((val << pos) & mask);
		p++;
		val >>= num_bits;
	}

	/* Write full bytes */
	while (p < last_p) {
		*p++ = val;
		val >>= 8;
	}

	/* Write partial suffix byte */
	if (p == last_p) {
		size_t num_bits = last_bit % 8 + 1;
		mask = (1U << num_bits) - 1U;
		*p = (*p & ~mask) | (val & mask);
	}
}
/* }}} */

static crex_always_inline crex_result crex_ffi_zval_to_cdata(void *ptr, crex_ffi_type *type, zval *value) /* {{{ */
{
	crex_long lval;
	double dval;
	crex_string *tmp_str;
	crex_string *str;
	crex_ffi_type_kind kind = type->kind;

	/* Pointer type has special handling of CData */
	if (kind != CREX_FFI_TYPE_POINTER && C_TYPE_P(value) == IS_OBJECT && C_OBJCE_P(value) == crex_ffi_cdata_ce) {
		crex_ffi_cdata *cdata = (crex_ffi_cdata*)C_OBJ_P(value);
		if (crex_ffi_is_compatible_type(type, CREX_FFI_TYPE(cdata->type)) &&
			type->size == CREX_FFI_TYPE(cdata->type)->size) {
			memcpy(ptr, cdata->ptr, type->size);
			return SUCCESS;
		}
	}

again:
	switch (kind) {
		case CREX_FFI_TYPE_FLOAT:
			dval = zval_get_double(value);
			*(float*)ptr = dval;
			break;
		case CREX_FFI_TYPE_DOUBLE:
			dval = zval_get_double(value);
			*(double*)ptr = dval;
			break;
#ifdef HAVE_LONG_DOUBLE
		case CREX_FFI_TYPE_LONGDOUBLE:
			dval = zval_get_double(value);
			*(long double*)ptr = dval;
			break;
#endif
		case CREX_FFI_TYPE_UINT8:
			lval = zval_get_long(value);
			*(uint8_t*)ptr = lval;
			break;
		case CREX_FFI_TYPE_SINT8:
			lval = zval_get_long(value);
			*(int8_t*)ptr = lval;
			break;
		case CREX_FFI_TYPE_UINT16:
			lval = zval_get_long(value);
			*(uint16_t*)ptr = lval;
			break;
		case CREX_FFI_TYPE_SINT16:
			lval = zval_get_long(value);
			*(int16_t*)ptr = lval;
			break;
		case CREX_FFI_TYPE_UINT32:
			lval = zval_get_long(value);
			*(uint32_t*)ptr = lval;
			break;
		case CREX_FFI_TYPE_SINT32:
			lval = zval_get_long(value);
			*(int32_t*)ptr = lval;
			break;
		case CREX_FFI_TYPE_UINT64:
			lval = zval_get_long(value);
			*(uint64_t*)ptr = lval;
			break;
		case CREX_FFI_TYPE_SINT64:
			lval = zval_get_long(value);
			*(int64_t*)ptr = lval;
			break;
		case CREX_FFI_TYPE_BOOL:
			*(uint8_t*)ptr = crex_is_true(value);
			break;
		case CREX_FFI_TYPE_CHAR:
			str = zval_get_tmp_string(value, &tmp_str);
			if (ZSTR_LEN(str) == 1) {
				*(char*)ptr = ZSTR_VAL(str)[0];
			} else {
				crex_ffi_assign_incompatible(value, type);
				return FAILURE;
			}
			crex_tmp_string_release(tmp_str);
			break;
		case CREX_FFI_TYPE_ENUM:
			kind = type->enumeration.kind;
			goto again;
		case CREX_FFI_TYPE_POINTER:
			if (C_TYPE_P(value) == IS_NULL) {
				*(void**)ptr = NULL;
				break;
			} else if (C_TYPE_P(value) == IS_OBJECT && C_OBJCE_P(value) == crex_ffi_cdata_ce) {
				crex_ffi_cdata *cdata = (crex_ffi_cdata*)C_OBJ_P(value);

				if (crex_ffi_is_compatible_type(type, CREX_FFI_TYPE(cdata->type))) {
					if (CREX_FFI_TYPE(cdata->type)->kind == CREX_FFI_TYPE_POINTER) {
						*(void**)ptr = *(void**)cdata->ptr;
					} else {
						if (cdata->flags & CREX_FFI_FLAG_OWNED) {
							crex_throw_error(crex_ffi_exception_ce, "Attempt to perform assign of owned C pointer");
							return FAILURE;
						}
						*(void**)ptr = cdata->ptr;
					}
					return SUCCESS;
				/* Allow transparent assignment of not-owned CData to compatible pointers */
				} else if (CREX_FFI_TYPE(cdata->type)->kind != CREX_FFI_TYPE_POINTER
				 && crex_ffi_is_compatible_type(CREX_FFI_TYPE(type->pointer.type), CREX_FFI_TYPE(cdata->type))) {
					if (cdata->flags & CREX_FFI_FLAG_OWNED) {
						crex_throw_error(crex_ffi_exception_ce, "Attempt to perform assign pointer to owned C data");
						return FAILURE;
					}
					*(void**)ptr = cdata->ptr;
					return SUCCESS;
				}
#if FFI_CLOSURES
			} else if (CREX_FFI_TYPE(type->pointer.type)->kind == CREX_FFI_TYPE_FUNC) {
				void *callback = crex_ffi_create_callback(CREX_FFI_TYPE(type->pointer.type), value);

				if (callback) {
					*(void**)ptr = callback;
					break;
				} else {
					return FAILURE;
				}
#endif
			}
			crex_ffi_assign_incompatible(value, type);
			return FAILURE;
		case CREX_FFI_TYPE_STRUCT:
		case CREX_FFI_TYPE_ARRAY:
		default:
			/* Incompatible types, because otherwise the CData check at the entry point would've succeeded. */
			crex_ffi_assign_incompatible(value, type);
			return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

#if defined(CREX_WIN32) && (defined(HAVE_FFI_FASTCALL) || defined(HAVE_FFI_STDCALL) || defined(HAVE_FFI_VECTORCALL_PARTIAL))
static size_t crex_ffi_arg_size(crex_ffi_type *type) /* {{{ */
{
	crex_ffi_type *arg_type;
	size_t arg_size = 0;

	CREX_HASH_PACKED_FOREACH_PTR(type->func.args, arg_type) {
		arg_size += MAX(CREX_FFI_TYPE(arg_type)->size, sizeof(size_t));
	} CREX_HASH_FOREACH_END();
	return arg_size;
}
/* }}} */
#endif

static crex_always_inline crex_string *crex_ffi_mangled_func_name(crex_string *name, crex_ffi_type *type) /* {{{ */
{
#ifdef CREX_WIN32
	switch (type->func.abi) {
# ifdef HAVE_FFI_FASTCALL
		case FFI_FASTCALL:
			return strpprintf(0, "@%s@%zu", ZSTR_VAL(name), crex_ffi_arg_size(type));
# endif
# ifdef HAVE_FFI_STDCALL
		case FFI_STDCALL:
			return strpprintf(0, "_%s@%zu", ZSTR_VAL(name), crex_ffi_arg_size(type));
# endif
# ifdef HAVE_FFI_VECTORCALL_PARTIAL
		case FFI_VECTORCALL_PARTIAL:
			return strpprintf(0, "%s@@%zu", ZSTR_VAL(name), crex_ffi_arg_size(type));
# endif
	}
#endif
	return crex_string_copy(name);
}
/* }}} */

#if FFI_CLOSURES
typedef struct _crex_ffi_callback_data {
	crex_fcall_info_cache  fcc;
	crex_ffi_type         *type;
	void                  *code;
	void                  *callback;
	ffi_cif                cif;
	uint32_t               arg_count;
	ffi_type              *ret_type;
	ffi_type              *arg_types[0];
} crex_ffi_callback_data;

static void crex_ffi_callback_hash_dtor(zval *zv) /* {{{ */
{
	crex_ffi_callback_data *callback_data = C_PTR_P(zv);

	ffi_closure_free(callback_data->callback);
	if (callback_data->fcc.function_handler->common.fn_flags & CREX_ACC_CLOSURE) {
		OBJ_RELEASE(CREX_CLOSURE_OBJECT(callback_data->fcc.function_handler));
	}
	for (int i = 0; i < callback_data->arg_count; ++i) {
		if (callback_data->arg_types[i]->type == FFI_TYPE_STRUCT) {
			efree(callback_data->arg_types[i]);
		}
	}
	if (callback_data->ret_type->type == FFI_TYPE_STRUCT) {
		efree(callback_data->ret_type);
	}
	efree(callback_data);
}
/* }}} */

static void crex_ffi_callback_trampoline(ffi_cif* cif, void* ret, void** args, void* data) /* {{{ */
{
	crex_ffi_callback_data *callback_data = (crex_ffi_callback_data*)data;
	crex_fcall_info fci;
	crex_ffi_type *ret_type;
	zval retval;
	ALLOCA_FLAG(use_heap)

	fci.size = sizeof(crex_fcall_info);
	ZVAL_UNDEF(&fci.function_name);
	fci.retval = &retval;
	fci.params = do_alloca(sizeof(zval) *callback_data->arg_count, use_heap);
	fci.object = NULL;
	fci.param_count = callback_data->arg_count;
	fci.named_params = NULL;

	if (callback_data->type->func.args) {
		int n = 0;
		crex_ffi_type *arg_type;

		CREX_HASH_PACKED_FOREACH_PTR(callback_data->type->func.args, arg_type) {
			arg_type = CREX_FFI_TYPE(arg_type);
			crex_ffi_cdata_to_zval(NULL, args[n], arg_type, BP_VAR_R, &fci.params[n], (crex_ffi_flags)(arg_type->attr & CREX_FFI_ATTR_CONST), 0, 0);
			n++;
		} CREX_HASH_FOREACH_END();
	}

	ZVAL_UNDEF(&retval);
	if (crex_call_function(&fci, &callback_data->fcc) != SUCCESS) {
		crex_throw_error(crex_ffi_exception_ce, "Cannot call callback");
	}

	if (callback_data->arg_count) {
		int n = 0;

		for (n = 0; n < callback_data->arg_count; n++) {
			zval_ptr_dtor(&fci.params[n]);
		}
	}
	free_alloca(fci.params, use_heap);

	if (EG(exception)) {
		crex_error(E_ERROR, "Throwing from FFI callbacks is not allowed");
	}

	ret_type = CREX_FFI_TYPE(callback_data->type->func.ret_type);
	if (ret_type->kind != CREX_FFI_TYPE_VOID) {
		crex_ffi_zval_to_cdata(ret, ret_type, &retval);
	}

	zval_ptr_dtor(&retval);
}
/* }}} */

static void *crex_ffi_create_callback(crex_ffi_type *type, zval *value) /* {{{ */
{
	crex_fcall_info_cache fcc;
	char *error = NULL;
	uint32_t arg_count;
	void *code;
	void *callback;
	crex_ffi_callback_data *callback_data;

	if (type->attr & CREX_FFI_ATTR_VARIADIC) {
		crex_throw_error(crex_ffi_exception_ce, "Variadic function closures are not supported");
		return NULL;
	}

	if (!crex_is_callable_ex(value, NULL, 0, NULL, &fcc, &error)) {
		crex_throw_error(crex_ffi_exception_ce, "Attempt to assign an invalid callback, %s", error);
		return NULL;
	}

	arg_count = type->func.args ? crex_hash_num_elements(type->func.args) : 0;
	if (arg_count < fcc.function_handler->common.required_num_args) {
		crex_throw_error(crex_ffi_exception_ce, "Attempt to assign an invalid callback, insufficient number of arguments");
		return NULL;
	}

	callback = ffi_closure_alloc(sizeof(ffi_closure), &code);
	if (!callback) {
		crex_throw_error(crex_ffi_exception_ce, "Cannot allocate callback");
		return NULL;
	}

	callback_data = emalloc(sizeof(crex_ffi_callback_data) + sizeof(ffi_type*) * arg_count);
	memcpy(&callback_data->fcc, &fcc, sizeof(crex_fcall_info_cache));
	callback_data->type = type;
	callback_data->callback = callback;
	callback_data->code = code;
	callback_data->arg_count = arg_count;

	if (type->func.args) {
		int n = 0;
		crex_ffi_type *arg_type;

		CREX_HASH_PACKED_FOREACH_PTR(type->func.args, arg_type) {
			arg_type = CREX_FFI_TYPE(arg_type);
			callback_data->arg_types[n] = crex_ffi_get_type(arg_type);
			if (!callback_data->arg_types[n]) {
				crex_ffi_pass_unsupported(arg_type);
				for (int i = 0; i < n; ++i) {
					if (callback_data->arg_types[i]->type == FFI_TYPE_STRUCT) {
						efree(callback_data->arg_types[i]);
					}
				}
				efree(callback_data);
				ffi_closure_free(callback);
				return NULL;
			}
			n++;
		} CREX_HASH_FOREACH_END();
	}
	callback_data->ret_type = crex_ffi_get_type(CREX_FFI_TYPE(type->func.ret_type));
	if (!callback_data->ret_type) {
		crex_ffi_return_unsupported(type->func.ret_type);
		for (int i = 0; i < callback_data->arg_count; ++i) {
			if (callback_data->arg_types[i]->type == FFI_TYPE_STRUCT) {
				efree(callback_data->arg_types[i]);
			}
		}
		efree(callback_data);
		ffi_closure_free(callback);
		return NULL;
	}

	if (ffi_prep_cif(&callback_data->cif, type->func.abi, callback_data->arg_count, callback_data->ret_type, callback_data->arg_types) != FFI_OK) {
		crex_throw_error(crex_ffi_exception_ce, "Cannot prepare callback CIF");
		goto free_on_failure;
	}

	if (ffi_prep_closure_loc(callback, &callback_data->cif, crex_ffi_callback_trampoline, callback_data, code) != FFI_OK) {
		crex_throw_error(crex_ffi_exception_ce, "Cannot prepare callback");
free_on_failure: ;
		for (int i = 0; i < callback_data->arg_count; ++i) {
			if (callback_data->arg_types[i]->type == FFI_TYPE_STRUCT) {
				efree(callback_data->arg_types[i]);
			}
		}
		if (callback_data->ret_type->type == FFI_TYPE_STRUCT) {
			efree(callback_data->ret_type);
		}
		efree(callback_data);
		ffi_closure_free(callback);
		return NULL;
	}

	if (!FFI_G(callbacks)) {
		FFI_G(callbacks) = emalloc(sizeof(HashTable));
		crex_hash_init(FFI_G(callbacks), 0, NULL, crex_ffi_callback_hash_dtor, 0);
	}
	crex_hash_next_index_insert_ptr(FFI_G(callbacks), callback_data);

	if (fcc.function_handler->common.fn_flags & CREX_ACC_CLOSURE) {
		GC_ADDREF(CREX_CLOSURE_OBJECT(fcc.function_handler));
	}

	return code;
}
/* }}} */
#endif

static zval *crex_ffi_cdata_get(crex_object *obj, crex_string *member, int read_type, void **cache_slot, zval *rv) /* {{{ */
{
	crex_ffi_cdata *cdata = (crex_ffi_cdata*)obj;
	crex_ffi_type  *type = CREX_FFI_TYPE(cdata->type);

#if 0
	if (UNEXPECTED(!cdata->ptr)) {
		crex_throw_error(crex_ffi_exception_ce, "NULL pointer dereference");
		return &EG(uninitialized_zval);
	}
#endif

	if (UNEXPECTED(!crex_string_equals_literal(member, "cdata"))) {
		crex_throw_error(crex_ffi_exception_ce, "Only 'cdata' property may be read");
		return &EG(uninitialized_zval);
	}

	crex_ffi_cdata_to_zval(cdata, cdata->ptr, type, BP_VAR_R, rv, 0, 0, 0);
	return rv;
}
/* }}} */

static zval *crex_ffi_cdata_set(crex_object *obj, crex_string *member, zval *value, void **cache_slot) /* {{{ */
{
	crex_ffi_cdata *cdata = (crex_ffi_cdata*)obj;
	crex_ffi_type  *type = CREX_FFI_TYPE(cdata->type);

#if 0
	if (UNEXPECTED(!cdata->ptr)) {
		crex_throw_error(crex_ffi_exception_ce, "NULL pointer dereference");
		return &EG(uninitialized_zval);
	}
#endif

	if (UNEXPECTED(!crex_string_equals_literal(member, "cdata"))) {
		crex_throw_error(crex_ffi_exception_ce, "Only 'cdata' property may be set");
		return &EG(uninitialized_zval);
	}

	crex_ffi_zval_to_cdata(cdata->ptr, type, value);

	return value;
}
/* }}} */

static crex_result crex_ffi_cdata_cast_object(crex_object *readobj, zval *writeobj, int type) /* {{{ */
{
	if (type == IS_STRING) {
		crex_ffi_cdata *cdata = (crex_ffi_cdata*)readobj;
		crex_ffi_type  *ctype = CREX_FFI_TYPE(cdata->type);
		void           *ptr = cdata->ptr;
		crex_ffi_type_kind kind = ctype->kind;

again:
	    switch (kind) {
			case CREX_FFI_TYPE_FLOAT:
				ZVAL_DOUBLE(writeobj, *(float*)ptr);
				break;
			case CREX_FFI_TYPE_DOUBLE:
				ZVAL_DOUBLE(writeobj, *(double*)ptr);
				break;
#ifdef HAVE_LONG_DOUBLE
			case CREX_FFI_TYPE_LONGDOUBLE:
				ZVAL_DOUBLE(writeobj, *(long double*)ptr);
				break;
#endif
			case CREX_FFI_TYPE_UINT8:
				ZVAL_LONG(writeobj, *(uint8_t*)ptr);
				break;
			case CREX_FFI_TYPE_SINT8:
				ZVAL_LONG(writeobj, *(int8_t*)ptr);
				break;
			case CREX_FFI_TYPE_UINT16:
				ZVAL_LONG(writeobj, *(uint16_t*)ptr);
				break;
			case CREX_FFI_TYPE_SINT16:
				ZVAL_LONG(writeobj, *(int16_t*)ptr);
				break;
			case CREX_FFI_TYPE_UINT32:
				ZVAL_LONG(writeobj, *(uint32_t*)ptr);
				break;
			case CREX_FFI_TYPE_SINT32:
				ZVAL_LONG(writeobj, *(int32_t*)ptr);
				break;
			case CREX_FFI_TYPE_UINT64:
				ZVAL_LONG(writeobj, *(uint64_t*)ptr);
				break;
			case CREX_FFI_TYPE_SINT64:
				ZVAL_LONG(writeobj, *(int64_t*)ptr);
				break;
			case CREX_FFI_TYPE_BOOL:
				ZVAL_BOOL(writeobj, *(uint8_t*)ptr);
				break;
			case CREX_FFI_TYPE_CHAR:
				ZVAL_CHAR(writeobj, *(char*)ptr);
				return SUCCESS;
			case CREX_FFI_TYPE_ENUM:
				kind = ctype->enumeration.kind;
				goto again;
			case CREX_FFI_TYPE_POINTER:
				if (*(void**)ptr == NULL) {
					ZVAL_NULL(writeobj);
					break;
				} else if ((ctype->attr & CREX_FFI_ATTR_CONST) && CREX_FFI_TYPE(ctype->pointer.type)->kind == CREX_FFI_TYPE_CHAR) {
					ZVAL_STRING(writeobj, *(char**)ptr);
					return SUCCESS;
				}
				return FAILURE;
			default:
				return FAILURE;
		}
		convert_to_string(writeobj);
		return SUCCESS;
	} else if (type == _IS_BOOL) {
		ZVAL_TRUE(writeobj);
		return SUCCESS;
	}

	return FAILURE;
}
/* }}} */

static zval *crex_ffi_cdata_read_field(crex_object *obj, crex_string *field_name, int read_type, void **cache_slot, zval *rv) /* {{{ */
{
	crex_ffi_cdata *cdata = (crex_ffi_cdata*)obj;
	crex_ffi_type  *type = CREX_FFI_TYPE(cdata->type);
	void           *ptr = cdata->ptr;
	crex_ffi_field *field;

	if (cache_slot && *cache_slot == type) {
		field = *(cache_slot + 1);
	} else {
		if (type->kind == CREX_FFI_TYPE_POINTER) {
			type = CREX_FFI_TYPE(type->pointer.type);
		}
		if (UNEXPECTED(type->kind != CREX_FFI_TYPE_STRUCT)) {
			if (UNEXPECTED(type->kind != CREX_FFI_TYPE_STRUCT)) {
				crex_throw_error(crex_ffi_exception_ce, "Attempt to read field '%s' of non C struct/union", ZSTR_VAL(field_name));
				return &EG(uninitialized_zval);
			}
		}

		field = crex_hash_find_ptr(&type->record.fields, field_name);
		if (UNEXPECTED(!field)) {
			crex_throw_error(crex_ffi_exception_ce, "Attempt to read undefined field '%s' of C struct/union", ZSTR_VAL(field_name));
			return &EG(uninitialized_zval);
		}

		if (cache_slot) {
			*cache_slot = type;
			*(cache_slot + 1) = field;
		}
	}

	if (CREX_FFI_TYPE(cdata->type)->kind == CREX_FFI_TYPE_POINTER) {
		/* transparently dereference the pointer */
		if (UNEXPECTED(!ptr)) {
			crex_throw_error(crex_ffi_exception_ce, "NULL pointer dereference");
			return &EG(uninitialized_zval);
		}
		ptr = (void*)(*(char**)ptr);
		if (UNEXPECTED(!ptr)) {
			crex_throw_error(crex_ffi_exception_ce, "NULL pointer dereference");
			return &EG(uninitialized_zval);
		}
		type = CREX_FFI_TYPE(type->pointer.type);
	}

#if 0
	if (UNEXPECTED(!ptr)) {
		crex_throw_error(crex_ffi_exception_ce, "NULL pointer dereference");
		return &EG(uninitialized_zval);
	}
#endif

	if (EXPECTED(!field->bits)) {
		crex_ffi_type *field_type = field->type;

		if (CREX_FFI_TYPE_IS_OWNED(field_type)) {
			field_type = CREX_FFI_TYPE(field_type);
			if (!(field_type->attr & CREX_FFI_ATTR_STORED)
			 && field_type->kind == CREX_FFI_TYPE_POINTER) {
				field->type = field_type = crex_ffi_remember_type(field_type);
			}
		}
		ptr = (void*)(((char*)ptr) + field->offset);
		crex_ffi_cdata_to_zval(NULL, ptr, field_type, read_type, rv, (cdata->flags & CREX_FFI_FLAG_CONST) | (crex_ffi_flags)field->is_const, 0, 0);
	} else {
		crex_ffi_bit_field_to_zval(ptr, field, rv);
	}

	return rv;
}
/* }}} */

static zval *crex_ffi_cdata_write_field(crex_object *obj, crex_string *field_name, zval *value, void **cache_slot) /* {{{ */
{
	crex_ffi_cdata *cdata = (crex_ffi_cdata*)obj;
	crex_ffi_type  *type = CREX_FFI_TYPE(cdata->type);
	void           *ptr = cdata->ptr;
	crex_ffi_field *field;

	if (cache_slot && *cache_slot == type) {
		field = *(cache_slot + 1);
	} else {
		if (type->kind == CREX_FFI_TYPE_POINTER) {
			type = CREX_FFI_TYPE(type->pointer.type);
		}
		if (UNEXPECTED(type->kind != CREX_FFI_TYPE_STRUCT)) {
			if (UNEXPECTED(type->kind != CREX_FFI_TYPE_STRUCT)) {
				crex_throw_error(crex_ffi_exception_ce, "Attempt to assign field '%s' of non C struct/union", ZSTR_VAL(field_name));
				return value;
			}
		}

		field = crex_hash_find_ptr(&type->record.fields, field_name);
		if (UNEXPECTED(!field)) {
			crex_throw_error(crex_ffi_exception_ce, "Attempt to assign undefined field '%s' of C struct/union", ZSTR_VAL(field_name));
			return value;
		}

		if (cache_slot) {
			*cache_slot = type;
			*(cache_slot + 1) = field;
		}
	}

	if (CREX_FFI_TYPE(cdata->type)->kind == CREX_FFI_TYPE_POINTER) {
		/* transparently dereference the pointer */
		if (UNEXPECTED(!ptr)) {
			crex_throw_error(crex_ffi_exception_ce, "NULL pointer dereference");
			return value;
		}
		ptr = (void*)(*(char**)ptr);
		if (UNEXPECTED(!ptr)) {
			crex_throw_error(crex_ffi_exception_ce, "NULL pointer dereference");
			return value;
		}
	}

#if 0
	if (UNEXPECTED(!ptr)) {
		crex_throw_error(crex_ffi_exception_ce, "NULL pointer dereference");
		return value;
	}
#endif

	if (UNEXPECTED(cdata->flags & CREX_FFI_FLAG_CONST)) {
		crex_throw_error(crex_ffi_exception_ce, "Attempt to assign read-only location");
		return value;
	} else if (UNEXPECTED(field->is_const)) {
		crex_throw_error(crex_ffi_exception_ce, "Attempt to assign read-only field '%s'", ZSTR_VAL(field_name));
		return value;
	}

	if (EXPECTED(!field->bits)) {
		ptr = (void*)(((char*)ptr) + field->offset);
		crex_ffi_zval_to_cdata(ptr, CREX_FFI_TYPE(field->type), value);
	} else {
		crex_ffi_zval_to_bit_field(ptr, field, value);
	}
	return value;
}
/* }}} */

static zval *crex_ffi_cdata_read_dim(crex_object *obj, zval *offset, int read_type, zval *rv) /* {{{ */
{
	crex_ffi_cdata *cdata = (crex_ffi_cdata*)obj;
	crex_ffi_type  *type = CREX_FFI_TYPE(cdata->type);
	crex_long       dim = zval_get_long(offset);
	crex_ffi_type  *dim_type;
	void           *ptr;
	crex_ffi_flags  is_const;

	if (EXPECTED(type->kind == CREX_FFI_TYPE_ARRAY)) {
		if (UNEXPECTED((crex_ulong)(dim) >= (crex_ulong)type->array.length)
		 && (UNEXPECTED(dim < 0) || UNEXPECTED(type->array.length != 0))) {
			crex_throw_error(crex_ffi_exception_ce, "C array index out of bounds");
			return &EG(uninitialized_zval);
		}

		is_const = (cdata->flags & CREX_FFI_FLAG_CONST) | (crex_ffi_flags)(type->attr & CREX_FFI_ATTR_CONST);

		dim_type = type->array.type;
		if (CREX_FFI_TYPE_IS_OWNED(dim_type)) {
			dim_type = CREX_FFI_TYPE(dim_type);
			if (!(dim_type->attr & CREX_FFI_ATTR_STORED)
			 && dim_type->kind == CREX_FFI_TYPE_POINTER) {
				type->array.type = dim_type = crex_ffi_remember_type(dim_type);
			}
		}
#if 0
		if (UNEXPECTED(!cdata->ptr)) {
			crex_throw_error(crex_ffi_exception_ce, "NULL pointer dereference");
			return &EG(uninitialized_zval);
		}
#endif
		ptr = (void*)(((char*)cdata->ptr) + dim_type->size * dim);
	} else if (EXPECTED(type->kind == CREX_FFI_TYPE_POINTER)) {
		is_const = (cdata->flags & CREX_FFI_FLAG_CONST) | (crex_ffi_flags)(type->attr & CREX_FFI_ATTR_CONST);
		dim_type = type->pointer.type;
		if (CREX_FFI_TYPE_IS_OWNED(dim_type)) {
			dim_type = CREX_FFI_TYPE(dim_type);
			if (!(dim_type->attr & CREX_FFI_ATTR_STORED)
			 && dim_type->kind == CREX_FFI_TYPE_POINTER) {
				type->pointer.type = dim_type = crex_ffi_remember_type(dim_type);
			}
		}
		if (UNEXPECTED(!cdata->ptr)) {
			crex_throw_error(crex_ffi_exception_ce, "NULL pointer dereference");
			return &EG(uninitialized_zval);
		}
		ptr = (void*)((*(char**)cdata->ptr) + dim_type->size * dim);
	} else {
		crex_throw_error(crex_ffi_exception_ce, "Attempt to read element of non C array");
		return &EG(uninitialized_zval);
	}

	crex_ffi_cdata_to_zval(NULL, ptr, dim_type, read_type, rv, is_const, 0, 0);
	return rv;
}
/* }}} */

static void crex_ffi_cdata_write_dim(crex_object *obj, zval *offset, zval *value) /* {{{ */
{
	crex_ffi_cdata *cdata = (crex_ffi_cdata*)obj;
	crex_ffi_type  *type = CREX_FFI_TYPE(cdata->type);
	crex_long       dim;
	void           *ptr;
	crex_ffi_flags  is_const;

	if (offset == NULL) {
		crex_throw_error(crex_ffi_exception_ce, "Cannot add next element to object of type FFI\\CData");
		return;
	}

	dim = zval_get_long(offset);
	if (EXPECTED(type->kind == CREX_FFI_TYPE_ARRAY)) {
		if (UNEXPECTED((crex_ulong)(dim) >= (crex_ulong)type->array.length)
		 && (UNEXPECTED(dim < 0) || UNEXPECTED(type->array.length != 0))) {
			crex_throw_error(crex_ffi_exception_ce, "C array index out of bounds");
			return;
		}

		is_const = (cdata->flags & CREX_FFI_FLAG_CONST) | (crex_ffi_flags)(type->attr & CREX_FFI_ATTR_CONST);
		type = CREX_FFI_TYPE(type->array.type);
#if 0
		if (UNEXPECTED(!cdata->ptr)) {
			crex_throw_error(crex_ffi_exception_ce, "NULL pointer dereference");
			return;
		}
#endif
		ptr = (void*)(((char*)cdata->ptr) + type->size * dim);
	} else if (EXPECTED(type->kind == CREX_FFI_TYPE_POINTER)) {
		is_const = (cdata->flags & CREX_FFI_FLAG_CONST) | (crex_ffi_flags)(type->attr & CREX_FFI_ATTR_CONST);
		type = CREX_FFI_TYPE(type->pointer.type);
		if (UNEXPECTED(!cdata->ptr)) {
			crex_throw_error(crex_ffi_exception_ce, "NULL pointer dereference");
			return;
		}
		ptr = (void*)((*(char**)cdata->ptr) + type->size * dim);
	} else {
		crex_throw_error(crex_ffi_exception_ce, "Attempt to assign element of non C array");
		return;
	}

	if (UNEXPECTED(is_const)) {
		crex_throw_error(crex_ffi_exception_ce, "Attempt to assign read-only location");
		return;
	}

	crex_ffi_zval_to_cdata(ptr, type, value);
}
/* }}} */

#define MAX_TYPE_NAME_LEN 256

typedef struct _crex_ffi_ctype_name_buf {
	char *start;
	char *end;
	char buf[MAX_TYPE_NAME_LEN];
} crex_ffi_ctype_name_buf;

static bool crex_ffi_ctype_name_prepend(crex_ffi_ctype_name_buf *buf, const char *str, size_t len) /* {{{ */
{
	buf->start -= len;
	if (buf->start < buf->buf) {
		return 0;
	}
	memcpy(buf->start, str, len);
	return 1;
}
/* }}} */

static bool crex_ffi_ctype_name_append(crex_ffi_ctype_name_buf *buf, const char *str, size_t len) /* {{{ */
{
	if (buf->end + len > buf->buf + MAX_TYPE_NAME_LEN) {
		return 0;
	}
	memcpy(buf->end, str, len);
	buf->end += len;
	return 1;
}
/* }}} */

static bool crex_ffi_ctype_name(crex_ffi_ctype_name_buf *buf, const crex_ffi_type *type) /* {{{ */
{
	const char *name = NULL;
	bool is_ptr = 0;

	while (1) {
		switch (type->kind) {
			case CREX_FFI_TYPE_VOID:
				name = "void";
				break;
			case CREX_FFI_TYPE_FLOAT:
				name = "float";
				break;
			case CREX_FFI_TYPE_DOUBLE:
				name = "double";
				break;
#ifdef HAVE_LONG_DOUBLE
			case CREX_FFI_TYPE_LONGDOUBLE:
				name = "long double";
				break;
#endif
			case CREX_FFI_TYPE_UINT8:
				name = "uint8_t";
				break;
			case CREX_FFI_TYPE_SINT8:
				name = "int8_t";
				break;
			case CREX_FFI_TYPE_UINT16:
				name = "uint16_t";
				break;
			case CREX_FFI_TYPE_SINT16:
				name = "int16_t";
				break;
			case CREX_FFI_TYPE_UINT32:
				name = "uint32_t";
				break;
			case CREX_FFI_TYPE_SINT32:
				name = "int32_t";
				break;
			case CREX_FFI_TYPE_UINT64:
				name = "uint64_t";
				break;
			case CREX_FFI_TYPE_SINT64:
				name = "int64_t";
				break;
			case CREX_FFI_TYPE_ENUM:
				if (type->enumeration.tag_name) {
					crex_ffi_ctype_name_prepend(buf, ZSTR_VAL(type->enumeration.tag_name), ZSTR_LEN(type->enumeration.tag_name));
				} else {
					crex_ffi_ctype_name_prepend(buf, "<anonymous>", sizeof("<anonymous>")-1);
				}
				name = "enum ";
				break;
			case CREX_FFI_TYPE_BOOL:
				name = "bool";
				break;
			case CREX_FFI_TYPE_CHAR:
				name = "char";
				break;
			case CREX_FFI_TYPE_POINTER:
				if (!crex_ffi_ctype_name_prepend(buf, "*", 1)) {
					return 0;
				}
				is_ptr = 1;
				type = CREX_FFI_TYPE(type->pointer.type);
				break;
			case CREX_FFI_TYPE_FUNC:
				if (is_ptr) {
					is_ptr = 0;
					if (!crex_ffi_ctype_name_prepend(buf, "(", 1)
					 || !crex_ffi_ctype_name_append(buf, ")", 1)) {
						return 0;
					}
				}
				if (!crex_ffi_ctype_name_append(buf, "(", 1)
				 || !crex_ffi_ctype_name_append(buf, ")", 1)) {
					return 0;
				}
				type = CREX_FFI_TYPE(type->func.ret_type);
				break;
			case CREX_FFI_TYPE_ARRAY:
				if (is_ptr) {
					is_ptr = 0;
					if (!crex_ffi_ctype_name_prepend(buf, "(", 1)
					 || !crex_ffi_ctype_name_append(buf, ")", 1)) {
						return 0;
					}
				}
				if (!crex_ffi_ctype_name_append(buf, "[", 1)) {
					return 0;
				}
				if (type->attr & CREX_FFI_ATTR_VLA) {
					if (!crex_ffi_ctype_name_append(buf, "*", 1)) {
						return 0;
					}
				} else if (!(type->attr & CREX_FFI_ATTR_INCOMPLETE_ARRAY)) {
					char str[MAX_LENGTH_OF_LONG + 1];
					char *s = crex_print_long_to_buf(str + sizeof(str) - 1, type->array.length);

					if (!crex_ffi_ctype_name_append(buf, s, strlen(s))) {
						return 0;
					}
				}
				if (!crex_ffi_ctype_name_append(buf, "]", 1)) {
					return 0;
				}
				type = CREX_FFI_TYPE(type->array.type);
				break;
			case CREX_FFI_TYPE_STRUCT:
				if (type->attr & CREX_FFI_ATTR_UNION) {
					if (type->record.tag_name) {
						crex_ffi_ctype_name_prepend(buf, ZSTR_VAL(type->record.tag_name), ZSTR_LEN(type->record.tag_name));
					} else {
						crex_ffi_ctype_name_prepend(buf, "<anonymous>", sizeof("<anonymous>")-1);
					}
					name = "union ";
				} else {
					if (type->record.tag_name) {
						crex_ffi_ctype_name_prepend(buf, ZSTR_VAL(type->record.tag_name), ZSTR_LEN(type->record.tag_name));
					} else {
						crex_ffi_ctype_name_prepend(buf, "<anonymous>", sizeof("<anonymous>")-1);
					}
					name = "struct ";
				}
				break;
			default:
				CREX_UNREACHABLE();
		}
		if (name) {
			break;
		}
	}

//	if (buf->start != buf->end && *buf->start != '[') {
//		if (!crex_ffi_ctype_name_prepend(buf, " ", 1)) {
//			return 0;
//		}
//	}
	return crex_ffi_ctype_name_prepend(buf, name, strlen(name));
}
/* }}} */

static CREX_COLD void crex_ffi_return_unsupported(crex_ffi_type *type) /* {{{ */
{
	type = CREX_FFI_TYPE(type);
	if (type->kind == CREX_FFI_TYPE_STRUCT) {
		crex_throw_error(crex_ffi_exception_ce, "FFI return struct/union is not implemented");
	} else if (type->kind == CREX_FFI_TYPE_ARRAY) {
		crex_throw_error(crex_ffi_exception_ce, "FFI return array is not implemented");
	} else {
		crex_throw_error(crex_ffi_exception_ce, "FFI internal error. Unsupported return type");
	}
}
/* }}} */

static CREX_COLD void crex_ffi_pass_unsupported(crex_ffi_type *type) /* {{{ */
{
	type = CREX_FFI_TYPE(type);
	if (type->kind == CREX_FFI_TYPE_STRUCT) {
		crex_throw_error(crex_ffi_exception_ce, "FFI passing struct/union is not implemented");
	} else if (type->kind == CREX_FFI_TYPE_ARRAY) {
		crex_throw_error(crex_ffi_exception_ce, "FFI passing array is not implemented");
	} else {
		crex_throw_error(crex_ffi_exception_ce, "FFI internal error. Unsupported parameter type");
	}
}
/* }}} */

static CREX_COLD void crex_ffi_pass_incompatible(zval *arg, crex_ffi_type *type, uint32_t n, crex_execute_data *execute_data) /* {{{ */
{
	crex_ffi_ctype_name_buf buf1, buf2;

	buf1.start = buf1.end = buf1.buf + ((MAX_TYPE_NAME_LEN * 3) / 4);
	if (!crex_ffi_ctype_name(&buf1, type)) {
		crex_throw_error(crex_ffi_exception_ce, "Passing incompatible argument %d of C function '%s'", n + 1, ZSTR_VAL(EX(func)->internal_function.function_name));
	} else {
		*buf1.end = 0;
		if (C_TYPE_P(arg) == IS_OBJECT && C_OBJCE_P(arg) == crex_ffi_cdata_ce) {
			crex_ffi_cdata *cdata = (crex_ffi_cdata*)C_OBJ_P(arg);

			type = CREX_FFI_TYPE(cdata->type);
			buf2.start = buf2.end = buf2.buf + ((MAX_TYPE_NAME_LEN * 3) / 4);
			if (!crex_ffi_ctype_name(&buf2, type)) {
				crex_throw_error(crex_ffi_exception_ce, "Passing incompatible argument %d of C function '%s', expecting '%s'", n + 1, ZSTR_VAL(EX(func)->internal_function.function_name), buf1.start);
			} else {
				*buf2.end = 0;
				crex_throw_error(crex_ffi_exception_ce, "Passing incompatible argument %d of C function '%s', expecting '%s', found '%s'", n + 1, ZSTR_VAL(EX(func)->internal_function.function_name), buf1.start, buf2.start);
			}
		} else {
			crex_throw_error(crex_ffi_exception_ce, "Passing incompatible argument %d of C function '%s', expecting '%s', found CRX '%s'", n + 1, ZSTR_VAL(EX(func)->internal_function.function_name), buf1.start, crex_zval_value_name(arg));
		}
	}
}
/* }}} */

static CREX_COLD void crex_ffi_assign_incompatible(zval *arg, crex_ffi_type *type) /* {{{ */
{
	crex_ffi_ctype_name_buf buf1, buf2;

	buf1.start = buf1.end = buf1.buf + ((MAX_TYPE_NAME_LEN * 3) / 4);
	if (!crex_ffi_ctype_name(&buf1, type)) {
		crex_throw_error(crex_ffi_exception_ce, "Incompatible types when assigning");
	} else {
		*buf1.end = 0;
		if (C_TYPE_P(arg) == IS_OBJECT && C_OBJCE_P(arg) == crex_ffi_cdata_ce) {
			crex_ffi_cdata *cdata = (crex_ffi_cdata*)C_OBJ_P(arg);

			type = CREX_FFI_TYPE(cdata->type);
			buf2.start = buf2.end = buf2.buf + ((MAX_TYPE_NAME_LEN * 3) / 4);
			if (!crex_ffi_ctype_name(&buf2, type)) {
				crex_throw_error(crex_ffi_exception_ce, "Incompatible types when assigning to type '%s'", buf1.start);
			} else {
				*buf2.end = 0;
				crex_throw_error(crex_ffi_exception_ce, "Incompatible types when assigning to type '%s' from type '%s'", buf1.start, buf2.start);
			}
		} else {
			crex_throw_error(crex_ffi_exception_ce, "Incompatible types when assigning to type '%s' from CRX '%s'", buf1.start, crex_zval_value_name(arg));
		}
	}
}
/* }}} */

static crex_string *crex_ffi_get_class_name(crex_string *prefix, const crex_ffi_type *type) /* {{{ */
{
	crex_ffi_ctype_name_buf buf;

	buf.start = buf.end = buf.buf + ((MAX_TYPE_NAME_LEN * 3) / 4);
	if (!crex_ffi_ctype_name(&buf, type)) {
		return crex_string_copy(prefix);
	} else {
		return crex_string_concat3(
			ZSTR_VAL(prefix), ZSTR_LEN(prefix), ":", 1, buf.start, buf.end - buf.start);
	}
}
/* }}} */

static crex_string *crex_ffi_cdata_get_class_name(const crex_object *zobj) /* {{{ */
{
	crex_ffi_cdata *cdata = (crex_ffi_cdata*)zobj;

	return crex_ffi_get_class_name(zobj->ce->name, CREX_FFI_TYPE(cdata->type));
}
/* }}} */

static int crex_ffi_cdata_compare_objects(zval *o1, zval *o2) /* {{{ */
{
	if (C_TYPE_P(o1) == IS_OBJECT && C_OBJCE_P(o1) == crex_ffi_cdata_ce &&
	    C_TYPE_P(o2) == IS_OBJECT && C_OBJCE_P(o2) == crex_ffi_cdata_ce) {
		crex_ffi_cdata *cdata1 = (crex_ffi_cdata*)C_OBJ_P(o1);
		crex_ffi_cdata *cdata2 = (crex_ffi_cdata*)C_OBJ_P(o2);
		crex_ffi_type *type1 = CREX_FFI_TYPE(cdata1->type);
		crex_ffi_type *type2 = CREX_FFI_TYPE(cdata2->type);

		if (type1->kind == CREX_FFI_TYPE_POINTER && type2->kind == CREX_FFI_TYPE_POINTER) {
			void *ptr1 = *(void**)cdata1->ptr;
			void *ptr2 = *(void**)cdata2->ptr;

			if (!ptr1 || !ptr2) {
				crex_throw_error(crex_ffi_exception_ce, "NULL pointer dereference");
				return 0;
			}
			return ptr1 == ptr2 ? 0 : (ptr1 < ptr2 ? -1 : 1);
		}
	}
	crex_throw_error(crex_ffi_exception_ce, "Comparison of incompatible C types");
	return 0;
}
/* }}} */

static crex_result crex_ffi_cdata_count_elements(crex_object *obj, crex_long *count) /* {{{ */
{
	crex_ffi_cdata *cdata = (crex_ffi_cdata*)obj;
	crex_ffi_type  *type = CREX_FFI_TYPE(cdata->type);

	if (type->kind != CREX_FFI_TYPE_ARRAY) {
		crex_throw_error(crex_ffi_exception_ce, "Attempt to count() on non C array");
		return FAILURE;
	} else {
		*count = type->array.length;
		return SUCCESS;
	}
}
/* }}} */

static crex_object* crex_ffi_add(crex_ffi_cdata *base_cdata, crex_ffi_type *base_type, crex_long offset) /* {{{ */
{
	char *ptr;
	crex_ffi_type *ptr_type;
	crex_ffi_cdata *cdata =
		(crex_ffi_cdata*)crex_ffi_cdata_new(crex_ffi_cdata_ce);

	if (base_type->kind == CREX_FFI_TYPE_POINTER) {
		if (CREX_FFI_TYPE_IS_OWNED(base_cdata->type)) {
			if (!(base_type->attr & CREX_FFI_ATTR_STORED)) {
				if (GC_REFCOUNT(&base_cdata->std) == 1) {
					/* transfer type ownership */
					base_cdata->type = base_type;
					base_type = CREX_FFI_TYPE_MAKE_OWNED(base_type);
				} else {
					base_cdata->type = base_type = crex_ffi_remember_type(base_type);
				}
			}
		}
		cdata->type = base_type;
		ptr = (char*)(*(void**)base_cdata->ptr);
		ptr_type = CREX_FFI_TYPE(base_type)->pointer.type;
	} else {
		crex_ffi_type *new_type = emalloc(sizeof(crex_ffi_type));

		new_type->kind = CREX_FFI_TYPE_POINTER;
		new_type->attr = 0;
		new_type->size = sizeof(void*);
		new_type->align = _Alignof(void*);

		ptr_type = base_type->array.type;
		if (CREX_FFI_TYPE_IS_OWNED(ptr_type)) {
			ptr_type = CREX_FFI_TYPE(ptr_type);
			if (!(ptr_type->attr & CREX_FFI_ATTR_STORED)) {
				if (GC_REFCOUNT(&base_cdata->std) == 1) {
					/* transfer type ownership */
					base_type->array.type = ptr_type;
					ptr_type = CREX_FFI_TYPE_MAKE_OWNED(ptr_type);
				} else {
					base_type->array.type = ptr_type = crex_ffi_remember_type(ptr_type);
				}
			}
		}
		new_type->pointer.type = ptr_type;

		cdata->type = CREX_FFI_TYPE_MAKE_OWNED(new_type);
		ptr = (char*)base_cdata->ptr;
	}
	cdata->ptr = &cdata->ptr_holder;
	cdata->ptr_holder = ptr +
		(ptrdiff_t) (offset * CREX_FFI_TYPE(ptr_type)->size);
	cdata->flags = base_cdata->flags & CREX_FFI_FLAG_CONST;
	return &cdata->std;
}
/* }}} */

static crex_result crex_ffi_cdata_do_operation(uint8_t opcode, zval *result, zval *op1, zval *op2) /* {{{ */
{
	crex_long offset;

	ZVAL_DEREF(op1);
	ZVAL_DEREF(op2);
	if (C_TYPE_P(op1) == IS_OBJECT && C_OBJCE_P(op1) == crex_ffi_cdata_ce) {
		crex_ffi_cdata *cdata1 = (crex_ffi_cdata*)C_OBJ_P(op1);
		crex_ffi_type *type1 = CREX_FFI_TYPE(cdata1->type);

		if (type1->kind == CREX_FFI_TYPE_POINTER || type1->kind == CREX_FFI_TYPE_ARRAY) {
			if (opcode == CREX_ADD) {
				offset = zval_get_long(op2);
				ZVAL_OBJ(result, crex_ffi_add(cdata1, type1, offset));
				if (result == op1) {
					OBJ_RELEASE(&cdata1->std);
				}
				return SUCCESS;
			} else if (opcode == CREX_SUB) {
				if (C_TYPE_P(op2) == IS_OBJECT && C_OBJCE_P(op2) == crex_ffi_cdata_ce) {
					crex_ffi_cdata *cdata2 = (crex_ffi_cdata*)C_OBJ_P(op2);
					crex_ffi_type *type2 = CREX_FFI_TYPE(cdata2->type);

					if (type2->kind == CREX_FFI_TYPE_POINTER || type2->kind == CREX_FFI_TYPE_ARRAY) {
						crex_ffi_type *t1, *t2;
						char *p1, *p2;

						if (type1->kind == CREX_FFI_TYPE_POINTER) {
							t1 = CREX_FFI_TYPE(type1->pointer.type);
							p1 = (char*)(*(void**)cdata1->ptr);
						} else {
							t1 = CREX_FFI_TYPE(type1->array.type);
							p1 = cdata1->ptr;
						}
						if (type2->kind == CREX_FFI_TYPE_POINTER) {
							t2 = CREX_FFI_TYPE(type2->pointer.type);
							p2 = (char*)(*(void**)cdata2->ptr);
						} else {
							t2 = CREX_FFI_TYPE(type2->array.type);
							p2 = cdata2->ptr;
						}
						if (crex_ffi_is_same_type(t1, t2)) {
							ZVAL_LONG(result,
								(crex_long)(p1 - p2) / (crex_long)t1->size);
							return SUCCESS;
						}
					}
				}
				offset = zval_get_long(op2);
				ZVAL_OBJ(result, crex_ffi_add(cdata1, type1, -offset));
				if (result == op1) {
					OBJ_RELEASE(&cdata1->std);
				}
				return SUCCESS;
			}
		}
	} else if (C_TYPE_P(op2) == IS_OBJECT && C_OBJCE_P(op2) == crex_ffi_cdata_ce) {
		crex_ffi_cdata *cdata2 = (crex_ffi_cdata*)C_OBJ_P(op2);
		crex_ffi_type *type2 = CREX_FFI_TYPE(cdata2->type);

		if (type2->kind == CREX_FFI_TYPE_POINTER || type2->kind == CREX_FFI_TYPE_ARRAY) {
			if (opcode == CREX_ADD) {
				offset = zval_get_long(op1);
				ZVAL_OBJ(result, crex_ffi_add(cdata2, type2, offset));
				return SUCCESS;
			}
		}
	}

	return FAILURE;
}
/* }}} */

typedef struct _crex_ffi_cdata_iterator {
	crex_object_iterator it;
	crex_long key;
	zval value;
	bool by_ref;
} crex_ffi_cdata_iterator;

static void crex_ffi_cdata_it_dtor(crex_object_iterator *iter) /* {{{ */
{
	zval_ptr_dtor(&((crex_ffi_cdata_iterator*)iter)->value);
	zval_ptr_dtor(&iter->data);
}
/* }}} */

static int crex_ffi_cdata_it_valid(crex_object_iterator *it) /* {{{ */
{
	crex_ffi_cdata_iterator *iter = (crex_ffi_cdata_iterator*)it;
	crex_ffi_cdata *cdata = (crex_ffi_cdata*)C_OBJ(iter->it.data);
	crex_ffi_type  *type = CREX_FFI_TYPE(cdata->type);

	return (iter->key >= 0 && iter->key < type->array.length) ? SUCCESS : FAILURE;
}
/* }}} */

static zval *crex_ffi_cdata_it_get_current_data(crex_object_iterator *it) /* {{{ */
{
	crex_ffi_cdata_iterator *iter = (crex_ffi_cdata_iterator*)it;
	crex_ffi_cdata *cdata = (crex_ffi_cdata*)C_OBJ(iter->it.data);
	crex_ffi_type  *type = CREX_FFI_TYPE(cdata->type);
	crex_ffi_type  *dim_type;
	void *ptr;

	if (!cdata->ptr) {
		crex_throw_error(crex_ffi_exception_ce, "NULL pointer dereference");
		return &EG(uninitialized_zval);
	}
	dim_type = type->array.type;
	if (CREX_FFI_TYPE_IS_OWNED(dim_type)) {
		dim_type = CREX_FFI_TYPE(dim_type);
		if (!(dim_type->attr & CREX_FFI_ATTR_STORED)
		 && dim_type->kind == CREX_FFI_TYPE_POINTER) {
			type->array.type = dim_type = crex_ffi_remember_type(dim_type);
		}
	}
	ptr = (void*)((char*)cdata->ptr + dim_type->size * iter->it.index);

	zval_ptr_dtor(&iter->value);
	crex_ffi_cdata_to_zval(NULL, ptr, dim_type, iter->by_ref ? BP_VAR_RW : BP_VAR_R, &iter->value, (cdata->flags & CREX_FFI_FLAG_CONST) | (crex_ffi_flags)(type->attr & CREX_FFI_ATTR_CONST), 0, 0);
	return &iter->value;
}
/* }}} */

static void crex_ffi_cdata_it_get_current_key(crex_object_iterator *it, zval *key) /* {{{ */
{
	crex_ffi_cdata_iterator *iter = (crex_ffi_cdata_iterator*)it;
	ZVAL_LONG(key, iter->key);
}
/* }}} */

static void crex_ffi_cdata_it_move_forward(crex_object_iterator *it) /* {{{ */
{
	crex_ffi_cdata_iterator *iter = (crex_ffi_cdata_iterator*)it;
	iter->key++;
}
/* }}} */

static void crex_ffi_cdata_it_rewind(crex_object_iterator *it) /* {{{ */
{
	crex_ffi_cdata_iterator *iter = (crex_ffi_cdata_iterator*)it;
	iter->key = 0;
}
/* }}} */

static const crex_object_iterator_funcs crex_ffi_cdata_it_funcs = {
	crex_ffi_cdata_it_dtor,
	crex_ffi_cdata_it_valid,
	crex_ffi_cdata_it_get_current_data,
	crex_ffi_cdata_it_get_current_key,
	crex_ffi_cdata_it_move_forward,
	crex_ffi_cdata_it_rewind,
	NULL,
	NULL, /* get_gc */
};

static crex_object_iterator *crex_ffi_cdata_get_iterator(crex_class_entry *ce, zval *object, int by_ref) /* {{{ */
{
	crex_ffi_cdata *cdata = (crex_ffi_cdata*)C_OBJ_P(object);
	crex_ffi_type  *type = CREX_FFI_TYPE(cdata->type);
	crex_ffi_cdata_iterator *iter;

	if (type->kind != CREX_FFI_TYPE_ARRAY) {
		crex_throw_error(crex_ffi_exception_ce, "Attempt to iterate on non C array");
		return NULL;
	}

	iter = emalloc(sizeof(crex_ffi_cdata_iterator));

	crex_iterator_init(&iter->it);

	C_ADDREF_P(object);
	ZVAL_OBJ(&iter->it.data, C_OBJ_P(object));
	iter->it.funcs = &crex_ffi_cdata_it_funcs;
	iter->key = 0;
	iter->by_ref = by_ref;
	ZVAL_UNDEF(&iter->value);

	return &iter->it;
}
/* }}} */

static HashTable *crex_ffi_cdata_get_debug_info(crex_object *obj, int *is_temp) /* {{{ */
{
	crex_ffi_cdata *cdata = (crex_ffi_cdata*)obj;
	crex_ffi_type  *type = CREX_FFI_TYPE(cdata->type);
	void           *ptr = cdata->ptr;
	HashTable      *ht = NULL;
	crex_string    *key;
	crex_ffi_field *f;
	crex_long       n;
	zval            tmp;

	if (!cdata->ptr) {
		crex_throw_error(crex_ffi_exception_ce, "NULL pointer dereference");
		return NULL;
	}

	switch (type->kind) {
		case CREX_FFI_TYPE_VOID:
			return NULL;
		case CREX_FFI_TYPE_BOOL:
		case CREX_FFI_TYPE_CHAR:
		case CREX_FFI_TYPE_ENUM:
		case CREX_FFI_TYPE_FLOAT:
		case CREX_FFI_TYPE_DOUBLE:
#ifdef HAVE_LONG_DOUBLE
		case CREX_FFI_TYPE_LONGDOUBLE:
#endif
		case CREX_FFI_TYPE_UINT8:
		case CREX_FFI_TYPE_SINT8:
		case CREX_FFI_TYPE_UINT16:
		case CREX_FFI_TYPE_SINT16:
		case CREX_FFI_TYPE_UINT32:
		case CREX_FFI_TYPE_SINT32:
		case CREX_FFI_TYPE_UINT64:
		case CREX_FFI_TYPE_SINT64:
			crex_ffi_cdata_to_zval(cdata, ptr, type, BP_VAR_R, &tmp, CREX_FFI_FLAG_CONST, 0, 0);
			ht = crex_new_array(1);
			crex_hash_str_add(ht, "cdata", sizeof("cdata")-1, &tmp);
			*is_temp = 1;
			return ht;
			break;
		case CREX_FFI_TYPE_POINTER:
			if (*(void**)ptr == NULL) {
				ZVAL_NULL(&tmp);
				ht = crex_new_array(1);
				crex_hash_index_add_new(ht, 0, &tmp);
				*is_temp = 1;
				return ht;
			} else if (CREX_FFI_TYPE(type->pointer.type)->kind == CREX_FFI_TYPE_VOID) {
				ZVAL_LONG(&tmp, (uintptr_t)*(void**)ptr);
				ht = crex_new_array(1);
				crex_hash_index_add_new(ht, 0, &tmp);
				*is_temp = 1;
				return ht;
			} else {
				crex_ffi_cdata_to_zval(NULL, *(void**)ptr, CREX_FFI_TYPE(type->pointer.type), BP_VAR_R, &tmp, CREX_FFI_FLAG_CONST, 0, 0);
				ht = crex_new_array(1);
				crex_hash_index_add_new(ht, 0, &tmp);
				*is_temp = 1;
				return ht;
			}
			break;
		case CREX_FFI_TYPE_STRUCT:
			ht = crex_new_array(crex_hash_num_elements(&type->record.fields));
			CREX_HASH_MAP_FOREACH_STR_KEY_PTR(&type->record.fields, key, f) {
				if (key) {
					if (!f->bits) {
						void *f_ptr = (void*)(((char*)ptr) + f->offset);
						crex_ffi_cdata_to_zval(NULL, f_ptr, CREX_FFI_TYPE(f->type), BP_VAR_R, &tmp, CREX_FFI_FLAG_CONST, 0, type->attr & CREX_FFI_ATTR_UNION);
						crex_hash_add(ht, key, &tmp);
					} else {
						crex_ffi_bit_field_to_zval(ptr, f, &tmp);
						crex_hash_add(ht, key, &tmp);
					}
				}
			} CREX_HASH_FOREACH_END();
			*is_temp = 1;
			return ht;
		case CREX_FFI_TYPE_ARRAY:
			ht = crex_new_array(type->array.length);
			for (n = 0; n < type->array.length; n++) {
				crex_ffi_cdata_to_zval(NULL, ptr, CREX_FFI_TYPE(type->array.type), BP_VAR_R, &tmp, CREX_FFI_FLAG_CONST, 0, 0);
				crex_hash_index_add(ht, n, &tmp);
				ptr = (void*)(((char*)ptr) + CREX_FFI_TYPE(type->array.type)->size);
			}
			*is_temp = 1;
			return ht;
		case CREX_FFI_TYPE_FUNC:
			ht = crex_new_array(0);
			// TODO: function name ???
			*is_temp = 1;
			return ht;
			break;
		default:
			CREX_UNREACHABLE();
			break;
	}
	return NULL;
}
/* }}} */

static crex_result crex_ffi_cdata_get_closure(crex_object *obj, crex_class_entry **ce_ptr, crex_function **fptr_ptr, crex_object **obj_ptr, bool check_only) /* {{{ */
{
	crex_ffi_cdata *cdata = (crex_ffi_cdata*)obj;
	crex_ffi_type  *type = CREX_FFI_TYPE(cdata->type);
	crex_function  *func;

	if (type->kind != CREX_FFI_TYPE_POINTER) {
		if (!check_only) {
			crex_throw_error(crex_ffi_exception_ce, "Attempt to call non C function pointer");
		}
		return FAILURE;
	}
	type = CREX_FFI_TYPE(type->pointer.type);
	if (type->kind != CREX_FFI_TYPE_FUNC) {
		if (!check_only) {
			crex_throw_error(crex_ffi_exception_ce, "Attempt to call non C function pointer");
		}
		return FAILURE;
	}
	if (!cdata->ptr) {
		if (!check_only) {
			crex_throw_error(crex_ffi_exception_ce, "NULL pointer dereference");
		}
		return FAILURE;
	}

	if (EXPECTED(EG(trampoline).common.function_name == NULL)) {
		func = &EG(trampoline);
	} else {
		func = ecalloc(sizeof(crex_internal_function), 1);
	}
	func->type = CREX_INTERNAL_FUNCTION;
	func->common.arg_flags[0] = 0;
	func->common.arg_flags[1] = 0;
	func->common.arg_flags[2] = 0;
	func->common.fn_flags = CREX_ACC_CALL_VIA_TRAMPOLINE;
	func->common.function_name = ZSTR_KNOWN(CREX_STR_MAGIC_INVOKE);
	/* set to 0 to avoid arg_info[] allocation, because all values are passed by value anyway */
	func->common.num_args = 0;
	func->common.required_num_args = type->func.args ? crex_hash_num_elements(type->func.args) : 0;
	func->common.scope = NULL;
	func->common.prototype = NULL;
	func->common.arg_info = NULL;
	func->internal_function.handler = CREX_FN(ffi_trampoline);
	func->internal_function.module = NULL;

	func->internal_function.reserved[0] = type;
	func->internal_function.reserved[1] = *(void**)cdata->ptr;

	*ce_ptr = NULL;
	*fptr_ptr= func;
	*obj_ptr = NULL;

	return SUCCESS;
}
/* }}} */

static crex_object *crex_ffi_ctype_new(crex_class_entry *class_type) /* {{{ */
{
	crex_ffi_ctype *ctype;

	ctype = emalloc(sizeof(crex_ffi_ctype));

	crex_ffi_object_init(&ctype->std, class_type);

	ctype->type = NULL;

	return &ctype->std;
}
/* }}} */

static void crex_ffi_ctype_free_obj(crex_object *object) /* {{{ */
{
	crex_ffi_ctype *ctype = (crex_ffi_ctype*)object;

	crex_ffi_type_dtor(ctype->type);

    if (UNEXPECTED(GC_FLAGS(object) & IS_OBJ_WEAKLY_REFERENCED)) {
        crex_weakrefs_notify(object);
    }
}
/* }}} */

static bool crex_ffi_is_same_type(crex_ffi_type *type1, crex_ffi_type *type2) /* {{{ */
{
	while (1) {
		if (type1 == type2) {
			return 1;
		} else if (type1->kind == type2->kind) {
			if (type1->kind < CREX_FFI_TYPE_POINTER) {
				return 1;
			} else if (type1->kind == CREX_FFI_TYPE_POINTER) {
				type1 = CREX_FFI_TYPE(type1->pointer.type);
				type2 = CREX_FFI_TYPE(type2->pointer.type);
				if (type1->kind == CREX_FFI_TYPE_VOID ||
				    type2->kind == CREX_FFI_TYPE_VOID) {
				    return 1;
				}
			} else if (type1->kind == CREX_FFI_TYPE_ARRAY &&
			           type1->array.length == type2->array.length) {
				type1 = CREX_FFI_TYPE(type1->array.type);
				type2 = CREX_FFI_TYPE(type2->array.type);
			} else {
				break;
			}
		} else {
			break;
		}
	}
	return 0;
}
/* }}} */

static crex_string *crex_ffi_ctype_get_class_name(const crex_object *zobj) /* {{{ */
{
	crex_ffi_ctype *ctype = (crex_ffi_ctype*)zobj;

	return crex_ffi_get_class_name(zobj->ce->name, CREX_FFI_TYPE(ctype->type));
}
/* }}} */

static int crex_ffi_ctype_compare_objects(zval *o1, zval *o2) /* {{{ */
{
	if (C_TYPE_P(o1) == IS_OBJECT && C_OBJCE_P(o1) == crex_ffi_ctype_ce &&
	    C_TYPE_P(o2) == IS_OBJECT && C_OBJCE_P(o2) == crex_ffi_ctype_ce) {
		crex_ffi_ctype *ctype1 = (crex_ffi_ctype*)C_OBJ_P(o1);
		crex_ffi_ctype *ctype2 = (crex_ffi_ctype*)C_OBJ_P(o2);
		crex_ffi_type *type1 = CREX_FFI_TYPE(ctype1->type);
		crex_ffi_type *type2 = CREX_FFI_TYPE(ctype2->type);

		if (crex_ffi_is_same_type(type1, type2)) {
			return 0;
		} else {
			return 1;
		}
	}
	crex_throw_error(crex_ffi_exception_ce, "Comparison of incompatible C types");
	return 0;
}
/* }}} */

static HashTable *crex_ffi_ctype_get_debug_info(crex_object *obj, int *is_temp) /* {{{ */
{
	return NULL;
}
/* }}} */

static crex_object *crex_ffi_new(crex_class_entry *class_type) /* {{{ */
{
	crex_ffi *ffi;

	ffi = emalloc(sizeof(crex_ffi));

	crex_ffi_object_init(&ffi->std, class_type);

	ffi->lib = NULL;
	ffi->symbols = NULL;
	ffi->tags = NULL;
	ffi->persistent = 0;

	return &ffi->std;
}
/* }}} */

static void _crex_ffi_type_dtor(crex_ffi_type *type) /* {{{ */
{
	type = CREX_FFI_TYPE(type);

	switch (type->kind) {
		case CREX_FFI_TYPE_ENUM:
			if (type->enumeration.tag_name) {
				crex_string_release(type->enumeration.tag_name);
			}
			break;
		case CREX_FFI_TYPE_STRUCT:
			if (type->record.tag_name) {
				crex_string_release(type->record.tag_name);
			}
			crex_hash_destroy(&type->record.fields);
			break;
		case CREX_FFI_TYPE_POINTER:
			crex_ffi_type_dtor(type->pointer.type);
			break;
		case CREX_FFI_TYPE_ARRAY:
			crex_ffi_type_dtor(type->array.type);
			break;
		case CREX_FFI_TYPE_FUNC:
			if (type->func.args) {
				crex_hash_destroy(type->func.args);
				pefree(type->func.args, type->attr & CREX_FFI_ATTR_PERSISTENT);
			}
			crex_ffi_type_dtor(type->func.ret_type);
			break;
		default:
			break;
	}
	pefree(type, type->attr & CREX_FFI_ATTR_PERSISTENT);
}
/* }}} */

static void crex_ffi_type_hash_dtor(zval *zv) /* {{{ */
{
	crex_ffi_type *type = C_PTR_P(zv);
	crex_ffi_type_dtor(type);
}
/* }}} */

static void crex_ffi_field_hash_dtor(zval *zv) /* {{{ */
{
	crex_ffi_field *field = C_PTR_P(zv);
	crex_ffi_type_dtor(field->type);
	efree(field);
}
/* }}} */

static void crex_ffi_field_hash_persistent_dtor(zval *zv) /* {{{ */
{
	crex_ffi_field *field = C_PTR_P(zv);
	crex_ffi_type_dtor(field->type);
	free(field);
}
/* }}} */

static void crex_ffi_symbol_hash_dtor(zval *zv) /* {{{ */
{
	crex_ffi_symbol *sym = C_PTR_P(zv);
	crex_ffi_type_dtor(sym->type);
	efree(sym);
}
/* }}} */

static void crex_ffi_symbol_hash_persistent_dtor(zval *zv) /* {{{ */
{
	crex_ffi_symbol *sym = C_PTR_P(zv);
	crex_ffi_type_dtor(sym->type);
	free(sym);
}
/* }}} */

static void crex_ffi_tag_hash_dtor(zval *zv) /* {{{ */
{
	crex_ffi_tag *tag = C_PTR_P(zv);
	crex_ffi_type_dtor(tag->type);
	efree(tag);
}
/* }}} */

static void crex_ffi_tag_hash_persistent_dtor(zval *zv) /* {{{ */
{
	crex_ffi_tag *tag = C_PTR_P(zv);
	crex_ffi_type_dtor(tag->type);
	free(tag);
}
/* }}} */

static void crex_ffi_cdata_dtor(crex_ffi_cdata *cdata) /* {{{ */
{
	crex_ffi_type_dtor(cdata->type);
	if (cdata->flags & CREX_FFI_FLAG_OWNED) {
		if (cdata->ptr != (void*)&cdata->ptr_holder) {
			pefree(cdata->ptr, cdata->flags & CREX_FFI_FLAG_PERSISTENT);
		} else {
			pefree(cdata->ptr_holder, cdata->flags & CREX_FFI_FLAG_PERSISTENT);
		}
	}
}
/* }}} */

static void crex_ffi_scope_hash_dtor(zval *zv) /* {{{ */
{
	crex_ffi_scope *scope = C_PTR_P(zv);
	if (scope->symbols) {
		crex_hash_destroy(scope->symbols);
		free(scope->symbols);
	}
	if (scope->tags) {
		crex_hash_destroy(scope->tags);
		free(scope->tags);
	}
	free(scope);
}
/* }}} */

static void crex_ffi_free_obj(crex_object *object) /* {{{ */
{
	crex_ffi *ffi = (crex_ffi*)object;

	if (ffi->persistent) {
		return;
	}

	if (ffi->lib) {
		DL_UNLOAD(ffi->lib);
		ffi->lib = NULL;
	}

	if (ffi->symbols) {
		crex_hash_destroy(ffi->symbols);
		efree(ffi->symbols);
	}

	if (ffi->tags) {
		crex_hash_destroy(ffi->tags);
		efree(ffi->tags);
	}

    if (UNEXPECTED(GC_FLAGS(object) & IS_OBJ_WEAKLY_REFERENCED)) {
        crex_weakrefs_notify(object);
    }
}
/* }}} */

static void crex_ffi_cdata_free_obj(crex_object *object) /* {{{ */
{
	crex_ffi_cdata *cdata = (crex_ffi_cdata*)object;

	crex_ffi_cdata_dtor(cdata);

    if (UNEXPECTED(GC_FLAGS(object) & IS_OBJ_WEAKLY_REFERENCED)) {
        crex_weakrefs_notify(object);
    }
}
/* }}} */

static crex_object *crex_ffi_cdata_clone_obj(crex_object *obj) /* {{{ */
{
	crex_ffi_cdata *old_cdata = (crex_ffi_cdata*)obj;
	crex_ffi_type *type = CREX_FFI_TYPE(old_cdata->type);
	crex_ffi_cdata *new_cdata;

	new_cdata = (crex_ffi_cdata*)crex_ffi_cdata_new(crex_ffi_cdata_ce);
	if (type->kind < CREX_FFI_TYPE_POINTER) {
		new_cdata->std.handlers = &crex_ffi_cdata_value_handlers;
	}
	new_cdata->type = type;
	new_cdata->ptr = emalloc(type->size);
	memcpy(new_cdata->ptr, old_cdata->ptr, type->size);
	new_cdata->flags |= CREX_FFI_FLAG_OWNED;

	return &new_cdata->std;
}
/* }}} */

static zval *crex_ffi_read_var(crex_object *obj, crex_string *var_name, int read_type, void **cache_slot, zval *rv) /* {{{ */
{
	crex_ffi        *ffi = (crex_ffi*)obj;
	crex_ffi_symbol *sym = NULL;

	if (ffi->symbols) {
		sym = crex_hash_find_ptr(ffi->symbols, var_name);
		if (sym && sym->kind != CREX_FFI_SYM_VAR && sym->kind != CREX_FFI_SYM_CONST && sym->kind != CREX_FFI_SYM_FUNC) {
			sym = NULL;
		}
	}
	if (!sym) {
		crex_throw_error(crex_ffi_exception_ce, "Attempt to read undefined C variable '%s'", ZSTR_VAL(var_name));
		return &EG(uninitialized_zval);
	}

	if (sym->kind == CREX_FFI_SYM_VAR) {
		crex_ffi_cdata_to_zval(NULL, sym->addr, CREX_FFI_TYPE(sym->type), read_type, rv, (crex_ffi_flags)sym->is_const, 0, 0);
	} else if (sym->kind == CREX_FFI_SYM_FUNC) {
		crex_ffi_cdata *cdata;
		crex_ffi_type *new_type = emalloc(sizeof(crex_ffi_type));

		new_type->kind = CREX_FFI_TYPE_POINTER;
		new_type->attr = 0;
		new_type->size = sizeof(void*);
		new_type->align = _Alignof(void*);
		new_type->pointer.type = CREX_FFI_TYPE(sym->type);

		cdata = emalloc(sizeof(crex_ffi_cdata));
		crex_ffi_object_init(&cdata->std, crex_ffi_cdata_ce);
		cdata->type = CREX_FFI_TYPE_MAKE_OWNED(new_type);
		cdata->flags = CREX_FFI_FLAG_CONST;
		cdata->ptr_holder = sym->addr;
		cdata->ptr = &cdata->ptr_holder;
		ZVAL_OBJ(rv, &cdata->std);
	} else {
		ZVAL_LONG(rv, sym->value);
	}

	return rv;
}
/* }}} */

static zval *crex_ffi_write_var(crex_object *obj, crex_string *var_name, zval *value, void **cache_slot) /* {{{ */
{
	crex_ffi        *ffi = (crex_ffi*)obj;
	crex_ffi_symbol *sym = NULL;

	if (ffi->symbols) {
		sym = crex_hash_find_ptr(ffi->symbols, var_name);
		if (sym && sym->kind != CREX_FFI_SYM_VAR) {
			sym = NULL;
		}
	}
	if (!sym) {
		crex_throw_error(crex_ffi_exception_ce, "Attempt to assign undefined C variable '%s'", ZSTR_VAL(var_name));
		return value;
	}

	if (sym->is_const) {
		crex_throw_error(crex_ffi_exception_ce, "Attempt to assign read-only C variable '%s'", ZSTR_VAL(var_name));
		return value;
	}

	crex_ffi_zval_to_cdata(sym->addr, CREX_FFI_TYPE(sym->type), value);
	return value;
}
/* }}} */

static crex_result crex_ffi_pass_arg(zval *arg, crex_ffi_type *type, ffi_type **pass_type, void **arg_values, uint32_t n, crex_execute_data *execute_data) /* {{{ */
{
	crex_long lval;
	double dval;
	crex_string *str, *tmp_str;
	crex_ffi_type_kind kind = type->kind;

	ZVAL_DEREF(arg);

again:
	switch (kind) {
		case CREX_FFI_TYPE_FLOAT:
			dval = zval_get_double(arg);
			*pass_type = &ffi_type_float;
			*(float*)arg_values[n] = (float)dval;
			break;
		case CREX_FFI_TYPE_DOUBLE:
			dval = zval_get_double(arg);
			*pass_type = &ffi_type_double;
			*(double*)arg_values[n] = dval;
			break;
#ifdef HAVE_LONG_DOUBLE
		case CREX_FFI_TYPE_LONGDOUBLE:
			dval = zval_get_double(arg);
			*pass_type = &ffi_type_double;
			*(long double*)arg_values[n] = (long double)dval;
			break;
#endif
		case CREX_FFI_TYPE_UINT8:
			lval = zval_get_long(arg);
			*pass_type = &ffi_type_uint8;
			*(uint8_t*)arg_values[n] = (uint8_t)lval;
			break;
		case CREX_FFI_TYPE_SINT8:
			lval = zval_get_long(arg);
			*pass_type = &ffi_type_sint8;
			*(int8_t*)arg_values[n] = (int8_t)lval;
			break;
		case CREX_FFI_TYPE_UINT16:
			lval = zval_get_long(arg);
			*pass_type = &ffi_type_uint16;
			*(uint16_t*)arg_values[n] = (uint16_t)lval;
			break;
		case CREX_FFI_TYPE_SINT16:
			lval = zval_get_long(arg);
			*pass_type = &ffi_type_sint16;
			*(int16_t*)arg_values[n] = (int16_t)lval;
			break;
		case CREX_FFI_TYPE_UINT32:
			lval = zval_get_long(arg);
			*pass_type = &ffi_type_uint32;
			*(uint32_t*)arg_values[n] = (uint32_t)lval;
			break;
		case CREX_FFI_TYPE_SINT32:
			lval = zval_get_long(arg);
			*pass_type = &ffi_type_sint32;
			*(int32_t*)arg_values[n] = (int32_t)lval;
			break;
		case CREX_FFI_TYPE_UINT64:
			lval = zval_get_long(arg);
			*pass_type = &ffi_type_uint64;
			*(uint64_t*)arg_values[n] = (uint64_t)lval;
			break;
		case CREX_FFI_TYPE_SINT64:
			lval = zval_get_long(arg);
			*pass_type = &ffi_type_sint64;
			*(int64_t*)arg_values[n] = (int64_t)lval;
			break;
		case CREX_FFI_TYPE_POINTER:
			*pass_type = &ffi_type_pointer;
			if (C_TYPE_P(arg) == IS_NULL) {
				*(void**)arg_values[n] = NULL;
				return SUCCESS;
			} else if (C_TYPE_P(arg) == IS_STRING
			        && ((CREX_FFI_TYPE(type->pointer.type)->kind == CREX_FFI_TYPE_CHAR)
			         || (CREX_FFI_TYPE(type->pointer.type)->kind == CREX_FFI_TYPE_VOID))) {
				*(void**)arg_values[n] = C_STRVAL_P(arg);
				return SUCCESS;
			} else if (C_TYPE_P(arg) == IS_OBJECT && C_OBJCE_P(arg) == crex_ffi_cdata_ce) {
				crex_ffi_cdata *cdata = (crex_ffi_cdata*)C_OBJ_P(arg);

				if (crex_ffi_is_compatible_type(type, CREX_FFI_TYPE(cdata->type))) {
					if (CREX_FFI_TYPE(cdata->type)->kind == CREX_FFI_TYPE_POINTER) {
						if (!cdata->ptr) {
							crex_throw_error(crex_ffi_exception_ce, "NULL pointer dereference");
							return FAILURE;
						}
						*(void**)arg_values[n] = *(void**)cdata->ptr;
					} else {
						*(void**)arg_values[n] = cdata->ptr;
					}
					return SUCCESS;
				}
#if FFI_CLOSURES
			} else if (CREX_FFI_TYPE(type->pointer.type)->kind == CREX_FFI_TYPE_FUNC) {
				void *callback = crex_ffi_create_callback(CREX_FFI_TYPE(type->pointer.type), arg);

				if (callback) {
					*(void**)arg_values[n] = callback;
					break;
				} else {
					return FAILURE;
				}
#endif
			}
			crex_ffi_pass_incompatible(arg, type, n, execute_data);
			return FAILURE;
		case CREX_FFI_TYPE_BOOL:
			*pass_type = &ffi_type_uint8;
			*(uint8_t*)arg_values[n] = crex_is_true(arg);
			break;
		case CREX_FFI_TYPE_CHAR:
			str = zval_get_tmp_string(arg, &tmp_str);
			*pass_type = &ffi_type_sint8;
			*(char*)arg_values[n] = ZSTR_VAL(str)[0];
			if (ZSTR_LEN(str) != 1) {
				crex_ffi_pass_incompatible(arg, type, n, execute_data);
			}
			crex_tmp_string_release(tmp_str);
			break;
		case CREX_FFI_TYPE_ENUM:
			kind = type->enumeration.kind;
			goto again;
		case CREX_FFI_TYPE_STRUCT:
			if (C_TYPE_P(arg) == IS_OBJECT && C_OBJCE_P(arg) == crex_ffi_cdata_ce) {
				crex_ffi_cdata *cdata = (crex_ffi_cdata*)C_OBJ_P(arg);

				if (crex_ffi_is_compatible_type(type, CREX_FFI_TYPE(cdata->type))) {
					*pass_type = crex_ffi_make_fake_struct_type(type);
					arg_values[n] = cdata->ptr;
					break;
				}
			}
			crex_ffi_pass_incompatible(arg, type, n, execute_data);
			return FAILURE;
		default:
			crex_ffi_pass_unsupported(type);
			return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

static crex_result crex_ffi_pass_var_arg(zval *arg, ffi_type **pass_type, void **arg_values, uint32_t n, crex_execute_data *execute_data) /* {{{ */
{
	ZVAL_DEREF(arg);
	switch (C_TYPE_P(arg)) {
		case IS_NULL:
			*pass_type = &ffi_type_pointer;
			*(void**)arg_values[n] = NULL;
			break;
		case IS_FALSE:
			*pass_type = &ffi_type_uint8;
			*(uint8_t*)arg_values[n] = 0;
			break;
		case IS_TRUE:
			*pass_type = &ffi_type_uint8;
			*(uint8_t*)arg_values[n] = 1;
			break;
		case IS_LONG:
			if (sizeof(crex_long) == 4) {
				*pass_type = &ffi_type_sint32;
				*(int32_t*)arg_values[n] = C_LVAL_P(arg);
			} else {
				*pass_type = &ffi_type_sint64;
				*(int64_t*)arg_values[n] = C_LVAL_P(arg);
			}
			break;
		case IS_DOUBLE:
			*pass_type = &ffi_type_double;
			*(double*)arg_values[n] = C_DVAL_P(arg);
			break;
		case IS_STRING:
			*pass_type = &ffi_type_pointer;
			*(char**)arg_values[n] = C_STRVAL_P(arg);
			break;
		case IS_OBJECT:
			if (C_OBJCE_P(arg) == crex_ffi_cdata_ce) {
				crex_ffi_cdata *cdata = (crex_ffi_cdata*)C_OBJ_P(arg);
				crex_ffi_type *type = CREX_FFI_TYPE(cdata->type);

				return crex_ffi_pass_arg(arg, type, pass_type, arg_values, n, execute_data);
			}
			CREX_FALLTHROUGH;
		default:
			crex_throw_error(crex_ffi_exception_ce, "Unsupported argument type");
			return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

static CREX_FUNCTION(ffi_trampoline) /* {{{ */
{
	crex_ffi_type *type = EX(func)->internal_function.reserved[0];
	void *addr = EX(func)->internal_function.reserved[1];
	ffi_cif cif;
	ffi_type *ret_type = NULL;
	ffi_type **arg_types = NULL;
	void **arg_values = NULL;
	uint32_t n, arg_count;
	void *ret;
	crex_ffi_type *arg_type;
	ALLOCA_FLAG(arg_types_use_heap = 0)
	ALLOCA_FLAG(arg_values_use_heap = 0)
	ALLOCA_FLAG(ret_use_heap = 0)

	CREX_ASSERT(type->kind == CREX_FFI_TYPE_FUNC);
	arg_count = type->func.args ? crex_hash_num_elements(type->func.args) : 0;
	if (type->attr & CREX_FFI_ATTR_VARIADIC) {
		if (arg_count > EX_NUM_ARGS()) {
			crex_throw_error(crex_ffi_exception_ce, "Incorrect number of arguments for C function '%s', expecting at least %d parameter%s", ZSTR_VAL(EX(func)->internal_function.function_name), arg_count, (arg_count != 1) ? "s" : "");
			goto exit;
		}
		if (EX_NUM_ARGS()) {
			arg_types = do_alloca(
				sizeof(ffi_type*) * EX_NUM_ARGS(), arg_types_use_heap);
			arg_values = do_alloca(
				(sizeof(void*) + CREX_FFI_SIZEOF_ARG) * EX_NUM_ARGS(), arg_values_use_heap);
			n = 0;
			if (type->func.args) {
				CREX_HASH_PACKED_FOREACH_PTR(type->func.args, arg_type) {
					arg_type = CREX_FFI_TYPE(arg_type);
					arg_values[n] = ((char*)arg_values) + (sizeof(void*) * EX_NUM_ARGS()) + (CREX_FFI_SIZEOF_ARG * n);
					if (crex_ffi_pass_arg(EX_VAR_NUM(n), arg_type, &arg_types[n], arg_values, n, execute_data) == FAILURE) {
						free_alloca(arg_types, arg_types_use_heap);
						free_alloca(arg_values, arg_values_use_heap);
						goto exit;
					}
					n++;
				} CREX_HASH_FOREACH_END();
			}
			for (; n < EX_NUM_ARGS(); n++) {
				arg_values[n] = ((char*)arg_values) + (sizeof(void*) * EX_NUM_ARGS()) + (CREX_FFI_SIZEOF_ARG * n);
				if (crex_ffi_pass_var_arg(EX_VAR_NUM(n), &arg_types[n], arg_values, n, execute_data) == FAILURE) {
					free_alloca(arg_types, arg_types_use_heap);
					free_alloca(arg_values, arg_values_use_heap);
					goto exit;
				}
			}
		}
		ret_type = crex_ffi_get_type(CREX_FFI_TYPE(type->func.ret_type));
		if (!ret_type) {
			crex_ffi_return_unsupported(type->func.ret_type);
			free_alloca(arg_types, arg_types_use_heap);
			free_alloca(arg_values, arg_values_use_heap);
			goto exit;
		}
		if (ffi_prep_cif_var(&cif, type->func.abi, arg_count, EX_NUM_ARGS(), ret_type, arg_types) != FFI_OK) {
			crex_throw_error(crex_ffi_exception_ce, "Cannot prepare callback CIF");
			free_alloca(arg_types, arg_types_use_heap);
			free_alloca(arg_values, arg_values_use_heap);
			goto exit;
		}
	} else {
		if (arg_count != EX_NUM_ARGS()) {
			crex_throw_error(crex_ffi_exception_ce, "Incorrect number of arguments for C function '%s', expecting exactly %d parameter%s", ZSTR_VAL(EX(func)->internal_function.function_name), arg_count, (arg_count != 1) ? "s" : "");
			goto exit;
		}
		if (EX_NUM_ARGS()) {
			arg_types = do_alloca(
				(sizeof(ffi_type*) + sizeof(ffi_type)) * EX_NUM_ARGS(), arg_types_use_heap);
			arg_values = do_alloca(
				(sizeof(void*) + CREX_FFI_SIZEOF_ARG) * EX_NUM_ARGS(), arg_values_use_heap);
			n = 0;
			if (type->func.args) {
				CREX_HASH_PACKED_FOREACH_PTR(type->func.args, arg_type) {
					arg_type = CREX_FFI_TYPE(arg_type);
					arg_values[n] = ((char*)arg_values) + (sizeof(void*) * EX_NUM_ARGS()) + (CREX_FFI_SIZEOF_ARG * n);
					if (crex_ffi_pass_arg(EX_VAR_NUM(n), arg_type, &arg_types[n], arg_values, n, execute_data) == FAILURE) {
						free_alloca(arg_types, arg_types_use_heap);
						free_alloca(arg_values, arg_values_use_heap);
						goto exit;
					}
					n++;
				} CREX_HASH_FOREACH_END();
			}
		}
		ret_type = crex_ffi_get_type(CREX_FFI_TYPE(type->func.ret_type));
		if (!ret_type) {
			crex_ffi_return_unsupported(type->func.ret_type);
			free_alloca(arg_types, arg_types_use_heap);
			free_alloca(arg_values, arg_values_use_heap);
			goto exit;
		}
		if (ffi_prep_cif(&cif, type->func.abi, arg_count, ret_type, arg_types) != FFI_OK) {
			crex_throw_error(crex_ffi_exception_ce, "Cannot prepare callback CIF");
			free_alloca(arg_types, arg_types_use_heap);
			free_alloca(arg_values, arg_values_use_heap);
			goto exit;
		}
	}

	ret = do_alloca(MAX(ret_type->size, sizeof(ffi_arg)), ret_use_heap);
	ffi_call(&cif, addr, ret, arg_values);

	for (n = 0; n < arg_count; n++) {
		if (arg_types[n]->type == FFI_TYPE_STRUCT) {
			efree(arg_types[n]);
		}
	}
	if (ret_type->type == FFI_TYPE_STRUCT) {
		efree(ret_type);
	}

	if (EX_NUM_ARGS()) {
		free_alloca(arg_types, arg_types_use_heap);
		free_alloca(arg_values, arg_values_use_heap);
	}

	if (CREX_FFI_TYPE(type->func.ret_type)->kind != CREX_FFI_TYPE_VOID) {
		crex_ffi_cdata_to_zval(NULL, ret, CREX_FFI_TYPE(type->func.ret_type), BP_VAR_R, return_value, 0, 1, 0);
	} else {
		ZVAL_NULL(return_value);
	}
	free_alloca(ret, ret_use_heap);

exit:
	crex_string_release(EX(func)->common.function_name);
	if (EX(func)->common.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE) {
		crex_free_trampoline(EX(func));
		EX(func) = NULL;
	}
}
/* }}} */

static crex_function *crex_ffi_get_func(crex_object **obj, crex_string *name, const zval *key) /* {{{ */
{
	crex_ffi        *ffi = (crex_ffi*)*obj;
	crex_ffi_symbol *sym = NULL;
	crex_function   *func;
	crex_ffi_type   *type;

	if (ZSTR_LEN(name) == sizeof("new") -1
	 && (ZSTR_VAL(name)[0] == 'n' || ZSTR_VAL(name)[0] == 'N')
	 && (ZSTR_VAL(name)[1] == 'e' || ZSTR_VAL(name)[1] == 'E')
	 && (ZSTR_VAL(name)[2] == 'w' || ZSTR_VAL(name)[2] == 'W')) {
		return (crex_function*)&crex_ffi_new_fn;
	} else if (ZSTR_LEN(name) == sizeof("cast") -1
	 && (ZSTR_VAL(name)[0] == 'c' || ZSTR_VAL(name)[0] == 'C')
	 && (ZSTR_VAL(name)[1] == 'a' || ZSTR_VAL(name)[1] == 'A')
	 && (ZSTR_VAL(name)[2] == 's' || ZSTR_VAL(name)[2] == 'S')
	 && (ZSTR_VAL(name)[3] == 't' || ZSTR_VAL(name)[3] == 'T')) {
		return (crex_function*)&crex_ffi_cast_fn;
	} else if (ZSTR_LEN(name) == sizeof("type") -1
	 && (ZSTR_VAL(name)[0] == 't' || ZSTR_VAL(name)[0] == 'T')
	 && (ZSTR_VAL(name)[1] == 'y' || ZSTR_VAL(name)[1] == 'Y')
	 && (ZSTR_VAL(name)[2] == 'p' || ZSTR_VAL(name)[2] == 'P')
	 && (ZSTR_VAL(name)[3] == 'e' || ZSTR_VAL(name)[3] == 'E')) {
		return (crex_function*)&crex_ffi_type_fn;
	}

	if (ffi->symbols) {
		sym = crex_hash_find_ptr(ffi->symbols, name);
		if (sym && sym->kind != CREX_FFI_SYM_FUNC) {
			sym = NULL;
		}
	}
	if (!sym) {
		crex_throw_error(crex_ffi_exception_ce, "Attempt to call undefined C function '%s'", ZSTR_VAL(name));
		return NULL;
	}

	type = CREX_FFI_TYPE(sym->type);
	CREX_ASSERT(type->kind == CREX_FFI_TYPE_FUNC);

	if (EXPECTED(EG(trampoline).common.function_name == NULL)) {
		func = &EG(trampoline);
	} else {
		func = ecalloc(sizeof(crex_internal_function), 1);
	}
	func->common.type = CREX_INTERNAL_FUNCTION;
	func->common.arg_flags[0] = 0;
	func->common.arg_flags[1] = 0;
	func->common.arg_flags[2] = 0;
	func->common.fn_flags = CREX_ACC_CALL_VIA_TRAMPOLINE;
	func->common.function_name = crex_string_copy(name);
	/* set to 0 to avoid arg_info[] allocation, because all values are passed by value anyway */
	func->common.num_args = 0;
	func->common.required_num_args = type->func.args ? crex_hash_num_elements(type->func.args) : 0;
	func->common.scope = NULL;
	func->common.prototype = NULL;
	func->common.arg_info = NULL;
	func->internal_function.handler = CREX_FN(ffi_trampoline);
	func->internal_function.module = NULL;

	func->internal_function.reserved[0] = type;
	func->internal_function.reserved[1] = sym->addr;

	return func;
}
/* }}} */

static crex_never_inline int crex_ffi_disabled(void) /* {{{ */
{
	crex_throw_error(crex_ffi_exception_ce, "FFI API is restricted by \"ffi.enable\" configuration directive");
	return 0;
}
/* }}} */

static crex_always_inline bool crex_ffi_validate_api_restriction(crex_execute_data *execute_data) /* {{{ */
{
	if (EXPECTED(FFI_G(restriction) > CREX_FFI_ENABLED)) {
		CREX_ASSERT(FFI_G(restriction) == CREX_FFI_PRELOAD);
		if (FFI_G(is_cli)
		 || (execute_data->prev_execute_data
		  && (execute_data->prev_execute_data->func->common.fn_flags & CREX_ACC_PRELOADED))
		 || (CG(compiler_options) & CREX_COMPILE_PRELOAD)) {
			return 1;
		}
	} else if (EXPECTED(FFI_G(restriction) == CREX_FFI_ENABLED)) {
		return 1;
	}
	return crex_ffi_disabled();
}
/* }}} */

#define CREX_FFI_VALIDATE_API_RESTRICTION() do { \
		if (UNEXPECTED(!crex_ffi_validate_api_restriction(execute_data))) { \
			RETURN_THROWS(); \
		} \
	} while (0)

CREX_METHOD(FFI, cdef) /* {{{ */
{
	crex_string *code = NULL;
	crex_string *lib = NULL;
	crex_ffi *ffi = NULL;
	DL_HANDLE handle = NULL;
	char *err;
	void *addr;

	CREX_FFI_VALIDATE_API_RESTRICTION();
	CREX_PARSE_PARAMETERS_START(0, 2)
		C_PARAM_OPTIONAL
		C_PARAM_STR(code)
		C_PARAM_STR_OR_NULL(lib)
	CREX_PARSE_PARAMETERS_END();

	if (lib) {
		handle = DL_LOAD(ZSTR_VAL(lib));
		if (!handle) {
			err = GET_DL_ERROR();
#ifdef CRX_WIN32
			if (err && err[0]) {
				crex_throw_error(crex_ffi_exception_ce, "Failed loading '%s' (%s)", ZSTR_VAL(lib), err);
				crx_win32_error_msg_free(err);
			} else {
				crex_throw_error(crex_ffi_exception_ce, "Failed loading '%s' (Unknown reason)", ZSTR_VAL(lib));
			}
#else
			crex_throw_error(crex_ffi_exception_ce, "Failed loading '%s' (%s)", ZSTR_VAL(lib), err);
			GET_DL_ERROR(); /* free the buffer storing the error */
#endif
			RETURN_THROWS();
		}

#ifdef RTLD_DEFAULT
	} else if (1) {
		// TODO: this might need to be disabled or protected ???
		handle = RTLD_DEFAULT;
#endif
	}

	FFI_G(symbols) = NULL;
	FFI_G(tags) = NULL;

	if (code && ZSTR_LEN(code)) {
		/* Parse C definitions */
		FFI_G(default_type_attr) = CREX_FFI_ATTR_STORED;

		if (crex_ffi_parse_decl(ZSTR_VAL(code), ZSTR_LEN(code)) == FAILURE) {
			if (FFI_G(symbols)) {
				crex_hash_destroy(FFI_G(symbols));
				efree(FFI_G(symbols));
				FFI_G(symbols) = NULL;
			}
			if (FFI_G(tags)) {
				crex_hash_destroy(FFI_G(tags));
				efree(FFI_G(tags));
				FFI_G(tags) = NULL;
			}
			RETURN_THROWS();
		}

		if (FFI_G(symbols)) {
			crex_string *name;
			crex_ffi_symbol *sym;

			CREX_HASH_MAP_FOREACH_STR_KEY_PTR(FFI_G(symbols), name, sym) {
				if (sym->kind == CREX_FFI_SYM_VAR) {
					addr = DL_FETCH_SYMBOL(handle, ZSTR_VAL(name));
					if (!addr) {
						crex_throw_error(crex_ffi_exception_ce, "Failed resolving C variable '%s'", ZSTR_VAL(name));
						RETURN_THROWS();
					}
					sym->addr = addr;
				} else if (sym->kind == CREX_FFI_SYM_FUNC) {
					crex_string *mangled_name = crex_ffi_mangled_func_name(name, CREX_FFI_TYPE(sym->type));

					addr = DL_FETCH_SYMBOL(handle, ZSTR_VAL(mangled_name));
					crex_string_release(mangled_name);
					if (!addr) {
						crex_throw_error(crex_ffi_exception_ce, "Failed resolving C function '%s'", ZSTR_VAL(name));
						RETURN_THROWS();
					}
					sym->addr = addr;
				}
			} CREX_HASH_FOREACH_END();
		}
	}

	ffi = (crex_ffi*)crex_ffi_new(crex_ffi_ce);
	ffi->lib = handle;
	ffi->symbols = FFI_G(symbols);
	ffi->tags = FFI_G(tags);

	FFI_G(symbols) = NULL;
	FFI_G(tags) = NULL;

	RETURN_OBJ(&ffi->std);
}
/* }}} */

static bool crex_ffi_same_types(crex_ffi_type *old, crex_ffi_type *type) /* {{{ */
{
	if (old == type) {
		return 1;
	}

	if (old->kind != type->kind
	 || old->size != type->size
	 || old->align != type->align
	 || old->attr != type->attr) {
		return 0;
	}

	switch (old->kind) {
		case CREX_FFI_TYPE_ENUM:
			return old->enumeration.kind == type->enumeration.kind;
		case CREX_FFI_TYPE_ARRAY:
			return old->array.length == type->array.length
			 &&	crex_ffi_same_types(CREX_FFI_TYPE(old->array.type), CREX_FFI_TYPE(type->array.type));
		case CREX_FFI_TYPE_POINTER:
			return crex_ffi_same_types(CREX_FFI_TYPE(old->pointer.type), CREX_FFI_TYPE(type->pointer.type));
		case CREX_FFI_TYPE_STRUCT:
			if (crex_hash_num_elements(&old->record.fields) != crex_hash_num_elements(&type->record.fields)) {
				return 0;
			} else {
				crex_ffi_field *old_field, *field;
				crex_string *key;
				Bucket *b = type->record.fields.arData;

				CREX_HASH_MAP_FOREACH_STR_KEY_PTR(&old->record.fields, key, old_field) {
					while (C_TYPE(b->val) == IS_UNDEF) {
						b++;
					}
					if (key) {
						if (!b->key
						 || !crex_string_equals(key, b->key)) {
							return 0;
						}
					} else if (b->key) {
						return 0;
					}
					field = C_PTR(b->val);
					if (old_field->offset != field->offset
					 || old_field->is_const != field->is_const
					 || old_field->is_nested != field->is_nested
					 || old_field->first_bit != field->first_bit
					 || old_field->bits != field->bits
					 || !crex_ffi_same_types(CREX_FFI_TYPE(old_field->type), CREX_FFI_TYPE(field->type))) {
						return 0;
					}
					b++;
				} CREX_HASH_FOREACH_END();
			}
			break;
		case CREX_FFI_TYPE_FUNC:
			if (old->func.abi != type->func.abi
			 || ((old->func.args ? crex_hash_num_elements(old->func.args) : 0) != (type->func.args ? crex_hash_num_elements(type->func.args) : 0))
			 || !crex_ffi_same_types(CREX_FFI_TYPE(old->func.ret_type), CREX_FFI_TYPE(type->func.ret_type))) {
				return 0;
			} else if (old->func.args) {
				crex_ffi_type *arg_type;
				zval *zv = type->func.args->arPacked;

				CREX_HASH_PACKED_FOREACH_PTR(old->func.args, arg_type) {
					while (C_TYPE_P(zv) == IS_UNDEF) {
						zv++;
					}
					if (!crex_ffi_same_types(CREX_FFI_TYPE(arg_type), CREX_FFI_TYPE(C_PTR_P(zv)))) {
						return 0;
					}
					zv++;
				} CREX_HASH_FOREACH_END();
			}
			break;
		default:
			break;
	}

	return 1;
}
/* }}} */

static bool crex_ffi_same_symbols(crex_ffi_symbol *old, crex_ffi_symbol *sym) /* {{{ */
{
	if (old->kind != sym->kind || old->is_const != sym->is_const) {
		return 0;
	}

	if (old->kind == CREX_FFI_SYM_CONST) {
		if (old->value != sym->value) {
			return 0;
		}
	}

	return crex_ffi_same_types(CREX_FFI_TYPE(old->type), CREX_FFI_TYPE(sym->type));
}
/* }}} */

static bool crex_ffi_same_tags(crex_ffi_tag *old, crex_ffi_tag *tag) /* {{{ */
{
	if (old->kind != tag->kind) {
		return 0;
	}

	return crex_ffi_same_types(CREX_FFI_TYPE(old->type), CREX_FFI_TYPE(tag->type));
}
/* }}} */

static bool crex_ffi_subst_old_type(crex_ffi_type **dcl, crex_ffi_type *old, crex_ffi_type *type) /* {{{ */
{
	crex_ffi_type *dcl_type;
	crex_ffi_field *field;

	if (CREX_FFI_TYPE(*dcl) == type) {
		*dcl = old;
		return 1;
	}
	dcl_type = *dcl;
	switch (dcl_type->kind) {
		case CREX_FFI_TYPE_POINTER:
			return crex_ffi_subst_old_type(&dcl_type->pointer.type, old, type);
		case CREX_FFI_TYPE_ARRAY:
			return crex_ffi_subst_old_type(&dcl_type->array.type, old, type);
		case CREX_FFI_TYPE_FUNC:
			if (crex_ffi_subst_old_type(&dcl_type->func.ret_type, old, type)) {
				return 1;
			}
			if (dcl_type->func.args) {
				zval *zv;

				CREX_HASH_PACKED_FOREACH_VAL(dcl_type->func.args, zv) {
					if (crex_ffi_subst_old_type((crex_ffi_type**)&C_PTR_P(zv), old, type)) {
						return 1;
					}
				} CREX_HASH_FOREACH_END();
			}
			break;
		case CREX_FFI_TYPE_STRUCT:
			CREX_HASH_MAP_FOREACH_PTR(&dcl_type->record.fields, field) {
				if (crex_ffi_subst_old_type(&field->type, old, type)) {
					return 1;
				}
			} CREX_HASH_FOREACH_END();
			break;
		default:
			break;
	}
	return 0;
} /* }}} */

static void crex_ffi_cleanup_type(crex_ffi_type *old, crex_ffi_type *type) /* {{{ */
{
	crex_ffi_symbol *sym;
	crex_ffi_tag *tag;

	if (FFI_G(symbols)) {
		CREX_HASH_MAP_FOREACH_PTR(FFI_G(symbols), sym) {
			crex_ffi_subst_old_type(&sym->type, old, type);
		} CREX_HASH_FOREACH_END();
	}
	if (FFI_G(tags)) {
		CREX_HASH_MAP_FOREACH_PTR(FFI_G(tags), tag) {
			crex_ffi_subst_old_type(&tag->type, old, type);
		} CREX_HASH_FOREACH_END();
	}
}
/* }}} */

static crex_ffi_type *crex_ffi_remember_type(crex_ffi_type *type) /* {{{ */
{
	if (!FFI_G(weak_types)) {
		FFI_G(weak_types) = emalloc(sizeof(HashTable));
		crex_hash_init(FFI_G(weak_types), 0, NULL, crex_ffi_type_hash_dtor, 0);
	}
	// TODO: avoid dups ???
	type->attr |= CREX_FFI_ATTR_STORED;
	crex_hash_next_index_insert_ptr(FFI_G(weak_types), CREX_FFI_TYPE_MAKE_OWNED(type));
	return type;
}
/* }}} */

static crex_ffi *crex_ffi_load(const char *filename, bool preload) /* {{{ */
{
	struct stat buf;
	int fd;
	char *code, *code_pos, *scope_name, *lib, *err;
	size_t code_size, scope_name_len;
	crex_ffi *ffi;
	DL_HANDLE handle = NULL;
	crex_ffi_scope *scope = NULL;
	crex_string *name;
	crex_ffi_symbol *sym;
	crex_ffi_tag *tag;
	void *addr;

	if (stat(filename, &buf) != 0) {
		if (preload) {
			crex_error(E_WARNING, "FFI: failed pre-loading '%s', file doesn't exist", filename);
		} else {
			crex_throw_error(crex_ffi_exception_ce, "Failed loading '%s', file doesn't exist", filename);
		}
		return NULL;
	}

	if ((buf.st_mode & S_IFMT) != S_IFREG) {
		if (preload) {
			crex_error(E_WARNING, "FFI: failed pre-loading '%s', not a regular file", filename);
		} else {
			crex_throw_error(crex_ffi_exception_ce, "Failed loading '%s', not a regular file", filename);
		}
		return NULL;
	}

	code_size = buf.st_size;
	code = emalloc(code_size + 1);
	fd = open(filename, O_RDONLY, 0);
	if (fd < 0 || read(fd, code, code_size) != code_size) {
		if (preload) {
			crex_error(E_WARNING, "FFI: Failed pre-loading '%s', cannot read_file", filename);
		} else {
			crex_throw_error(crex_ffi_exception_ce, "Failed loading '%s', cannot read_file", filename);
		}
		efree(code);
		close(fd);
		return NULL;
	}
	close(fd);
	code[code_size] = 0;

	FFI_G(symbols) = NULL;
	FFI_G(tags) = NULL;
	FFI_G(persistent) = preload;
	FFI_G(default_type_attr) = preload ?
		CREX_FFI_ATTR_STORED | CREX_FFI_ATTR_PERSISTENT :
		CREX_FFI_ATTR_STORED;

	scope_name = NULL;
	scope_name_len = 0;
	lib = NULL;
	code_pos = crex_ffi_parse_directives(filename, code, &scope_name, &lib, preload);
	if (!code_pos) {
		efree(code);
		FFI_G(persistent) = 0;
		return NULL;
	}
	code_size -= code_pos - code;

	if (crex_ffi_parse_decl(code_pos, code_size) == FAILURE) {
		if (preload) {
			crex_error(E_WARNING, "FFI: failed pre-loading '%s'", filename);
		} else {
			crex_throw_error(crex_ffi_exception_ce, "Failed loading '%s'", filename);
		}
		goto cleanup;
	}

	if (lib) {
		handle = DL_LOAD(lib);
		if (!handle) {
			if (preload) {
				crex_error(E_WARNING, "FFI: Failed pre-loading '%s'", lib);
			} else {
				err = GET_DL_ERROR();
#ifdef CRX_WIN32
				if (err && err[0]) {
					crex_throw_error(crex_ffi_exception_ce, "Failed loading '%s' (%s)", lib, err);
					crx_win32_error_msg_free(err);
				} else {
					crex_throw_error(crex_ffi_exception_ce, "Failed loading '%s' (Unknown reason)", lib);
				}
#else
				crex_throw_error(crex_ffi_exception_ce, "Failed loading '%s' (%s)", lib, err);
				GET_DL_ERROR(); /* free the buffer storing the error */
#endif
			}
			goto cleanup;
		}
#ifdef RTLD_DEFAULT
	} else if (1) {
		// TODO: this might need to be disabled or protected ???
		handle = RTLD_DEFAULT;
#endif
	}

	if (preload) {
		if (!scope_name) {
			scope_name = "C";
		}
		scope_name_len = strlen(scope_name);
		if (FFI_G(scopes)) {
			scope = crex_hash_str_find_ptr(FFI_G(scopes), scope_name, scope_name_len);
		}
	}

	if (FFI_G(symbols)) {
		CREX_HASH_MAP_FOREACH_STR_KEY_PTR(FFI_G(symbols), name, sym) {
			if (sym->kind == CREX_FFI_SYM_VAR) {
				addr = DL_FETCH_SYMBOL(handle, ZSTR_VAL(name));
				if (!addr) {
					if (preload) {
						crex_error(E_WARNING, "FFI: failed pre-loading '%s', cannot resolve C variable '%s'", filename, ZSTR_VAL(name));
					} else {
						crex_throw_error(crex_ffi_exception_ce, "Failed resolving C variable '%s'", ZSTR_VAL(name));
					}
					if (lib) {
						DL_UNLOAD(handle);
					}
					goto cleanup;
				}
				sym->addr = addr;
			} else if (sym->kind == CREX_FFI_SYM_FUNC) {
				crex_string *mangled_name = crex_ffi_mangled_func_name(name, CREX_FFI_TYPE(sym->type));

				addr = DL_FETCH_SYMBOL(handle, ZSTR_VAL(mangled_name));
				crex_string_release(mangled_name);
				if (!addr) {
					if (preload) {
						crex_error(E_WARNING, "failed pre-loading '%s', cannot resolve C function '%s'", filename, ZSTR_VAL(name));
					} else {
						crex_throw_error(crex_ffi_exception_ce, "Failed resolving C function '%s'", ZSTR_VAL(name));
					}
					if (lib) {
						DL_UNLOAD(handle);
					}
					goto cleanup;
				}
				sym->addr = addr;
			}
			if (scope && scope->symbols) {
				crex_ffi_symbol *old_sym = crex_hash_find_ptr(scope->symbols, name);

				if (old_sym) {
					if (crex_ffi_same_symbols(old_sym, sym)) {
						if (CREX_FFI_TYPE_IS_OWNED(sym->type)
						 && CREX_FFI_TYPE(old_sym->type) != CREX_FFI_TYPE(sym->type)) {
							crex_ffi_type *type = CREX_FFI_TYPE(sym->type);
							crex_ffi_cleanup_type(CREX_FFI_TYPE(old_sym->type), CREX_FFI_TYPE(type));
							crex_ffi_type_dtor(type);
						}
					} else {
						crex_error(E_WARNING, "FFI: failed pre-loading '%s', redefinition of '%s'", filename, ZSTR_VAL(name));
						if (lib) {
							DL_UNLOAD(handle);
						}
						goto cleanup;
					}
				}
			}
		} CREX_HASH_FOREACH_END();
	}

	if (preload) {
		if (scope && scope->tags && FFI_G(tags)) {
			CREX_HASH_MAP_FOREACH_STR_KEY_PTR(FFI_G(tags), name, tag) {
				crex_ffi_tag *old_tag = crex_hash_find_ptr(scope->tags, name);

				if (old_tag) {
					if (crex_ffi_same_tags(old_tag, tag)) {
						if (CREX_FFI_TYPE_IS_OWNED(tag->type)
						 && CREX_FFI_TYPE(old_tag->type) != CREX_FFI_TYPE(tag->type)) {
							crex_ffi_type *type = CREX_FFI_TYPE(tag->type);
							crex_ffi_cleanup_type(CREX_FFI_TYPE(old_tag->type), CREX_FFI_TYPE(type));
							crex_ffi_type_dtor(type);
						}
					} else {
						crex_error(E_WARNING, "FFI: failed pre-loading '%s', redefinition of '%s %s'", filename, crex_ffi_tag_kind_name[tag->kind], ZSTR_VAL(name));
						if (lib) {
							DL_UNLOAD(handle);
						}
						goto cleanup;
					}
				}
			} CREX_HASH_FOREACH_END();
		}

		if (!scope) {
			scope = malloc(sizeof(crex_ffi_scope));
			scope->symbols = FFI_G(symbols);
			scope->tags = FFI_G(tags);

			if (!FFI_G(scopes)) {
				FFI_G(scopes) = malloc(sizeof(HashTable));
				crex_hash_init(FFI_G(scopes), 0, NULL, crex_ffi_scope_hash_dtor, 1);
			}

			crex_hash_str_add_ptr(FFI_G(scopes), scope_name, scope_name_len, scope);
		} else {
			if (FFI_G(symbols)) {
				if (!scope->symbols) {
					scope->symbols = FFI_G(symbols);
					FFI_G(symbols) = NULL;
				} else {
					CREX_HASH_MAP_FOREACH_STR_KEY_PTR(FFI_G(symbols), name, sym) {
						if (!crex_hash_add_ptr(scope->symbols, name, sym)) {
							crex_ffi_type_dtor(sym->type);
							free(sym);
						}
					} CREX_HASH_FOREACH_END();
					FFI_G(symbols)->pDestructor = NULL;
					crex_hash_destroy(FFI_G(symbols));
				}
			}
			if (FFI_G(tags)) {
				if (!scope->tags) {
					scope->tags = FFI_G(tags);
					FFI_G(tags) = NULL;
				} else {
					CREX_HASH_MAP_FOREACH_STR_KEY_PTR(FFI_G(tags), name, tag) {
						if (!crex_hash_add_ptr(scope->tags, name, tag)) {
							crex_ffi_type_dtor(tag->type);
							free(tag);
						}
					} CREX_HASH_FOREACH_END();
					FFI_G(tags)->pDestructor = NULL;
					crex_hash_destroy(FFI_G(tags));
				}
			}
		}

		if (EG(objects_store).object_buckets) {
			ffi = (crex_ffi*)crex_ffi_new(crex_ffi_ce);
		} else {
			ffi = ecalloc(1, sizeof(crex_ffi));
		}
		ffi->symbols = scope->symbols;
		ffi->tags = scope->tags;
		ffi->persistent = 1;
	} else {
		ffi = (crex_ffi*)crex_ffi_new(crex_ffi_ce);
		ffi->lib = handle;
		ffi->symbols = FFI_G(symbols);
		ffi->tags = FFI_G(tags);
	}

	efree(code);
	FFI_G(symbols) = NULL;
	FFI_G(tags) = NULL;
	FFI_G(persistent) = 0;

	return ffi;

cleanup:
	efree(code);
	if (FFI_G(symbols)) {
		crex_hash_destroy(FFI_G(symbols));
		pefree(FFI_G(symbols), preload);
		FFI_G(symbols) = NULL;
	}
	if (FFI_G(tags)) {
		crex_hash_destroy(FFI_G(tags));
		pefree(FFI_G(tags), preload);
		FFI_G(tags) = NULL;
	}
	FFI_G(persistent) = 0;
	return NULL;
}
/* }}} */

CREX_METHOD(FFI, load) /* {{{ */
{
	crex_string *fn;
	crex_ffi *ffi;

	CREX_FFI_VALIDATE_API_RESTRICTION();
	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(fn)
	CREX_PARSE_PARAMETERS_END();

	if (CG(compiler_options) & CREX_COMPILE_PRELOAD_IN_CHILD) {
		crex_throw_error(crex_ffi_exception_ce, "FFI::load() doesn't work in conjunction with \"opcache.preload_user\". Use \"ffi.preload\" instead.");
		RETURN_THROWS();
	}

	ffi = crex_ffi_load(ZSTR_VAL(fn), (CG(compiler_options) & CREX_COMPILE_PRELOAD) != 0);

	if (ffi) {
		RETURN_OBJ(&ffi->std);
	}
}
/* }}} */

CREX_METHOD(FFI, scope) /* {{{ */
{
	crex_string *scope_name;
	crex_ffi_scope *scope = NULL;
	crex_ffi *ffi;

	CREX_FFI_VALIDATE_API_RESTRICTION();
	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(scope_name)
	CREX_PARSE_PARAMETERS_END();

	if (FFI_G(scopes)) {
		scope = crex_hash_find_ptr(FFI_G(scopes), scope_name);
	}

	if (!scope) {
		crex_throw_error(crex_ffi_exception_ce, "Failed loading scope '%s'", ZSTR_VAL(scope_name));
		RETURN_THROWS();
	}

	ffi = (crex_ffi*)crex_ffi_new(crex_ffi_ce);

	ffi->symbols = scope->symbols;
	ffi->tags = scope->tags;
	ffi->persistent = 1;

	RETURN_OBJ(&ffi->std);
}
/* }}} */

static void crex_ffi_cleanup_dcl(crex_ffi_dcl *dcl) /* {{{ */
{
	if (dcl) {
		crex_ffi_type_dtor(dcl->type);
		dcl->type = NULL;
	}
}
/* }}} */

static void crex_ffi_throw_parser_error(const char *format, ...) /* {{{ */
{
	va_list va;
	char *message = NULL;

	va_start(va, format);
	crex_vspprintf(&message, 0, format, va);

	if (EG(current_execute_data)) {
		crex_throw_exception(crex_ffi_parser_exception_ce, message, 0);
	} else {
		crex_error(E_WARNING, "FFI Parser: %s", message);
	}

	efree(message);
	va_end(va);
}
/* }}} */

static crex_result crex_ffi_validate_vla(crex_ffi_type *type) /* {{{ */
{
	if (!FFI_G(allow_vla) && (type->attr & CREX_FFI_ATTR_VLA)) {
		crex_ffi_throw_parser_error("\"[*]\" is not allowed in other than function prototype scope at line %d", FFI_G(line));
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

static crex_result crex_ffi_validate_incomplete_type(crex_ffi_type *type, bool allow_incomplete_tag, bool allow_incomplete_array) /* {{{ */
{
	if (!allow_incomplete_tag && (type->attr & CREX_FFI_ATTR_INCOMPLETE_TAG)) {
		if (FFI_G(tags)) {
			crex_string *key;
			crex_ffi_tag *tag;

			CREX_HASH_MAP_FOREACH_STR_KEY_PTR(FFI_G(tags), key, tag) {
				if (CREX_FFI_TYPE(tag->type) == type) {
					if (type->kind == CREX_FFI_TYPE_ENUM) {
						crex_ffi_throw_parser_error("Incomplete enum \"%s\" at line %d", ZSTR_VAL(key), FFI_G(line));
					} else if (type->attr & CREX_FFI_ATTR_UNION) {
						crex_ffi_throw_parser_error("Incomplete union \"%s\" at line %d", ZSTR_VAL(key), FFI_G(line));
					} else {
						crex_ffi_throw_parser_error("Incomplete struct \"%s\" at line %d", ZSTR_VAL(key), FFI_G(line));
					}
					return FAILURE;
				}
			} CREX_HASH_FOREACH_END();
		}
		if (FFI_G(symbols)) {
			crex_string *key;
			crex_ffi_symbol *sym;

			CREX_HASH_MAP_FOREACH_STR_KEY_PTR(FFI_G(symbols), key, sym) {
				if (type == CREX_FFI_TYPE(sym->type)) {
					crex_ffi_throw_parser_error("Incomplete C type %s at line %d", ZSTR_VAL(key), FFI_G(line));
					return FAILURE;
				}
			} CREX_HASH_FOREACH_END();
		}
		crex_ffi_throw_parser_error("Incomplete type at line %d", FFI_G(line));
		return FAILURE;
	} else if (!allow_incomplete_array && (type->attr & CREX_FFI_ATTR_INCOMPLETE_ARRAY)) {
		crex_ffi_throw_parser_error("\"[]\" is not allowed at line %d", FFI_G(line));
		return FAILURE;
	} else if (!FFI_G(allow_vla) && (type->attr & CREX_FFI_ATTR_VLA)) {
		crex_ffi_throw_parser_error("\"[*]\" is not allowed in other than function prototype scope at line %d", FFI_G(line));
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

static crex_result crex_ffi_validate_type(crex_ffi_type *type, bool allow_incomplete_tag, bool allow_incomplete_array) /* {{{ */
{
	if (type->kind == CREX_FFI_TYPE_VOID) {
		crex_ffi_throw_parser_error("void type is not allowed at line %d", FFI_G(line));
		return FAILURE;
	}
	return crex_ffi_validate_incomplete_type(type, allow_incomplete_tag, allow_incomplete_array);
}
/* }}} */

static crex_result crex_ffi_validate_var_type(crex_ffi_type *type, bool allow_incomplete_array) /* {{{ */
{
	if (type->kind == CREX_FFI_TYPE_FUNC) {
		crex_ffi_throw_parser_error("function type is not allowed at line %d", FFI_G(line));
		return FAILURE;
	}
	return crex_ffi_validate_type(type, 0, allow_incomplete_array);
}
/* }}} */

void crex_ffi_validate_type_name(crex_ffi_dcl *dcl) /* {{{ */
{
	crex_ffi_finalize_type(dcl);
	if (crex_ffi_validate_var_type(CREX_FFI_TYPE(dcl->type), 0) == FAILURE) {
		crex_ffi_cleanup_dcl(dcl);
		LONGJMP(FFI_G(bailout), FAILURE);
	}
}
/* }}} */

static bool crex_ffi_subst_type(crex_ffi_type **dcl, crex_ffi_type *type) /* {{{ */
{
	crex_ffi_type *dcl_type;
	crex_ffi_field *field;

	if (*dcl == type) {
		*dcl = CREX_FFI_TYPE_MAKE_OWNED(type);
		return 1;
	}
	dcl_type = *dcl;
	switch (dcl_type->kind) {
		case CREX_FFI_TYPE_POINTER:
			return crex_ffi_subst_type(&dcl_type->pointer.type, type);
		case CREX_FFI_TYPE_ARRAY:
			return crex_ffi_subst_type(&dcl_type->array.type, type);
		case CREX_FFI_TYPE_FUNC:
			if (crex_ffi_subst_type(&dcl_type->func.ret_type, type)) {
				return 1;
			}
			if (dcl_type->func.args) {
				zval *zv;

				CREX_HASH_PACKED_FOREACH_VAL(dcl_type->func.args, zv) {
					if (crex_ffi_subst_type((crex_ffi_type**)&C_PTR_P(zv), type)) {
						return 1;
					}
				} CREX_HASH_FOREACH_END();
			}
			break;
		case CREX_FFI_TYPE_STRUCT:
			CREX_HASH_MAP_FOREACH_PTR(&dcl_type->record.fields, field) {
				if (crex_ffi_subst_type(&field->type, type)) {
					return 1;
				}
			} CREX_HASH_FOREACH_END();
			break;
		default:
			break;
	}
	return 0;
} /* }}} */

static void crex_ffi_tags_cleanup(crex_ffi_dcl *dcl) /* {{{ */
{
	crex_ffi_tag *tag;
	CREX_HASH_MAP_FOREACH_PTR(FFI_G(tags), tag) {
		if (CREX_FFI_TYPE_IS_OWNED(tag->type)) {
			crex_ffi_type *type = CREX_FFI_TYPE(tag->type);
			crex_ffi_subst_type(&dcl->type, type);
			tag->type = type;
		}
	} CREX_HASH_FOREACH_END();
	crex_hash_destroy(FFI_G(tags));
	efree(FFI_G(tags));
}
/* }}} */

CREX_METHOD(FFI, new) /* {{{ */
{
	crex_string *type_def = NULL;
	crex_object *type_obj = NULL;
	crex_ffi_type *type, *type_ptr;
	crex_ffi_cdata *cdata;
	void *ptr;
	bool owned = 1;
	bool persistent = 0;
	bool is_const = 0;
	bool is_static_call = C_TYPE(EX(This)) != IS_OBJECT;
	crex_ffi_flags flags = CREX_FFI_FLAG_OWNED;

	CREX_FFI_VALIDATE_API_RESTRICTION();
	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_OBJ_OF_CLASS_OR_STR(type_obj, crex_ffi_ctype_ce, type_def)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(owned)
		C_PARAM_BOOL(persistent)
	CREX_PARSE_PARAMETERS_END();

	if (is_static_call) {
		crex_error(E_DEPRECATED, "Calling FFI::new() statically is deprecated");
		if (EG(exception)) {
			RETURN_THROWS();
		}
	}

	if (!owned) {
		flags &= ~CREX_FFI_FLAG_OWNED;
	}

	if (persistent) {
		flags |= CREX_FFI_FLAG_PERSISTENT;
	}

	if (type_def) {
		crex_ffi_dcl dcl = CREX_FFI_ATTR_INIT;

		if (!is_static_call) {
			crex_ffi *ffi = (crex_ffi*)C_OBJ(EX(This));
			FFI_G(symbols) = ffi->symbols;
			FFI_G(tags) = ffi->tags;
		} else {
			FFI_G(symbols) = NULL;
			FFI_G(tags) = NULL;
		}
		bool clean_symbols = FFI_G(symbols) == NULL;
		bool clean_tags = FFI_G(tags) == NULL;

		FFI_G(default_type_attr) = 0;

		if (crex_ffi_parse_type(ZSTR_VAL(type_def), ZSTR_LEN(type_def), &dcl) == FAILURE) {
			crex_ffi_type_dtor(dcl.type);
			if (clean_tags && FFI_G(tags)) {
				crex_hash_destroy(FFI_G(tags));
				efree(FFI_G(tags));
				FFI_G(tags) = NULL;
			}
			if (clean_symbols && FFI_G(symbols)) {
				crex_hash_destroy(FFI_G(symbols));
				efree(FFI_G(symbols));
				FFI_G(symbols) = NULL;
			}
			return;
		}

		type = CREX_FFI_TYPE(dcl.type);
		if (dcl.attr & CREX_FFI_ATTR_CONST) {
			is_const = 1;
		}

		if (clean_tags && FFI_G(tags)) {
			crex_ffi_tags_cleanup(&dcl);
		}
		if (clean_symbols && FFI_G(symbols)) {
			crex_hash_destroy(FFI_G(symbols));
			efree(FFI_G(symbols));
			FFI_G(symbols) = NULL;
		}
		FFI_G(symbols) = NULL;
		FFI_G(tags) = NULL;

		type_ptr = dcl.type;
	} else {
		crex_ffi_ctype *ctype = (crex_ffi_ctype*) type_obj;

		type_ptr = type = ctype->type;
		if (CREX_FFI_TYPE_IS_OWNED(type)) {
			type = CREX_FFI_TYPE(type);
			if (!(type->attr & CREX_FFI_ATTR_STORED)) {
				if (GC_REFCOUNT(&ctype->std) == 1) {
					/* transfer type ownership */
					ctype->type = type;
				} else {
					ctype->type = type_ptr = type = crex_ffi_remember_type(type);
				}
			}
		}
	}

	if (type->size == 0) {
		crex_throw_error(crex_ffi_exception_ce, "Cannot instantiate FFI\\CData of zero size");
		crex_ffi_type_dtor(type_ptr);
		return;
	}

	ptr = pemalloc(type->size, flags & CREX_FFI_FLAG_PERSISTENT);
	memset(ptr, 0, type->size);

	cdata = (crex_ffi_cdata*)crex_ffi_cdata_new(crex_ffi_cdata_ce);
	if (type->kind < CREX_FFI_TYPE_POINTER) {
		cdata->std.handlers = &crex_ffi_cdata_value_handlers;
	}
	cdata->type = type_ptr;
	cdata->ptr = ptr;
	cdata->flags = flags;
	if (is_const) {
		cdata->flags |= CREX_FFI_FLAG_CONST;
	}

	RETURN_OBJ(&cdata->std);
}
/* }}} */

CREX_METHOD(FFI, free) /* {{{ */
{
	zval *zv;
	crex_ffi_cdata *cdata;

	CREX_FFI_VALIDATE_API_RESTRICTION();
	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_OBJECT_OF_CLASS_EX(zv, crex_ffi_cdata_ce, 0, 1);
	CREX_PARSE_PARAMETERS_END();

	cdata = (crex_ffi_cdata*)C_OBJ_P(zv);

	if (CREX_FFI_TYPE(cdata->type)->kind == CREX_FFI_TYPE_POINTER) {
		if (!cdata->ptr) {
			crex_throw_error(crex_ffi_exception_ce, "NULL pointer dereference");
			RETURN_THROWS();
		}
		if (cdata->ptr != (void*)&cdata->ptr_holder) {
			pefree(*(void**)cdata->ptr, cdata->flags & CREX_FFI_FLAG_PERSISTENT);
		} else {
			pefree(cdata->ptr_holder, (cdata->flags & CREX_FFI_FLAG_PERSISTENT) || !is_crex_ptr(cdata->ptr_holder));
		}
		*(void**)cdata->ptr = NULL;
	} else if (!(cdata->flags & CREX_FFI_FLAG_OWNED)) {
		pefree(cdata->ptr, cdata->flags & CREX_FFI_FLAG_PERSISTENT);
		cdata->ptr = NULL;
		cdata->flags &= ~(CREX_FFI_FLAG_OWNED|CREX_FFI_FLAG_PERSISTENT);
		cdata->std.handlers = &crex_ffi_cdata_free_handlers;
	} else {
		crex_throw_error(crex_ffi_exception_ce, "free() non a C pointer");
	}
}
/* }}} */

CREX_METHOD(FFI, cast) /* {{{ */
{
	crex_string *type_def = NULL;
	crex_object *ztype = NULL;
	crex_ffi_type *old_type, *type, *type_ptr;
	crex_ffi_cdata *old_cdata, *cdata;
	bool is_const = 0;
	bool is_static_call = C_TYPE(EX(This)) != IS_OBJECT;
	zval *zv, *arg;
	void *ptr;

	CREX_FFI_VALIDATE_API_RESTRICTION();
	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_OBJ_OF_CLASS_OR_STR(ztype, crex_ffi_ctype_ce, type_def)
		C_PARAM_ZVAL(zv)
	CREX_PARSE_PARAMETERS_END();

	if (is_static_call) {
		crex_error(E_DEPRECATED, "Calling FFI::cast() statically is deprecated");
		if (EG(exception)) {
			RETURN_THROWS();
		}
	}

	arg = zv;
	ZVAL_DEREF(zv);

	if (type_def) {
		crex_ffi_dcl dcl = CREX_FFI_ATTR_INIT;

		if (!is_static_call) {
			crex_ffi *ffi = (crex_ffi*)C_OBJ(EX(This));
			FFI_G(symbols) = ffi->symbols;
			FFI_G(tags) = ffi->tags;
		} else {
			FFI_G(symbols) = NULL;
			FFI_G(tags) = NULL;
		}
		bool clean_symbols = FFI_G(symbols) == NULL;
		bool clean_tags = FFI_G(tags) == NULL;

		FFI_G(default_type_attr) = 0;

		if (crex_ffi_parse_type(ZSTR_VAL(type_def), ZSTR_LEN(type_def), &dcl) == FAILURE) {
			crex_ffi_type_dtor(dcl.type);
			if (clean_tags && FFI_G(tags)) {
				crex_hash_destroy(FFI_G(tags));
				efree(FFI_G(tags));
				FFI_G(tags) = NULL;
			}
			if (clean_symbols && FFI_G(symbols)) {
				crex_hash_destroy(FFI_G(symbols));
				efree(FFI_G(symbols));
				FFI_G(symbols) = NULL;
			}
			return;
		}

		type = CREX_FFI_TYPE(dcl.type);
		if (dcl.attr & CREX_FFI_ATTR_CONST) {
			is_const = 1;
		}

		if (clean_tags && FFI_G(tags)) {
			crex_ffi_tags_cleanup(&dcl);
		}
		if (clean_symbols && FFI_G(symbols)) {
			crex_hash_destroy(FFI_G(symbols));
			efree(FFI_G(symbols));
			FFI_G(symbols) = NULL;
		}
		FFI_G(symbols) = NULL;
		FFI_G(tags) = NULL;

		type_ptr = dcl.type;
	} else {
		crex_ffi_ctype *ctype = (crex_ffi_ctype*) ztype;

		type_ptr = type = ctype->type;
		if (CREX_FFI_TYPE_IS_OWNED(type)) {
			type = CREX_FFI_TYPE(type);
			if (!(type->attr & CREX_FFI_ATTR_STORED)) {
				if (GC_REFCOUNT(&ctype->std) == 1) {
					/* transfer type ownership */
					ctype->type = type;
				} else {
					ctype->type = type_ptr = type = crex_ffi_remember_type(type);
				}
			}
		}
	}

	if (C_TYPE_P(zv) != IS_OBJECT || C_OBJCE_P(zv) != crex_ffi_cdata_ce) {
		if (type->kind < CREX_FFI_TYPE_POINTER && C_TYPE_P(zv) < IS_STRING) {
			/* numeric conversion */
			cdata = (crex_ffi_cdata*)crex_ffi_cdata_new(crex_ffi_cdata_ce);
			cdata->std.handlers = &crex_ffi_cdata_value_handlers;
			cdata->type = type_ptr;
			cdata->ptr = emalloc(type->size);
			crex_ffi_zval_to_cdata(cdata->ptr, type, zv);
			cdata->flags = CREX_FFI_FLAG_OWNED;
			if (is_const) {
				cdata->flags |= CREX_FFI_FLAG_CONST;
			}
			RETURN_OBJ(&cdata->std);
		} else if (type->kind == CREX_FFI_TYPE_POINTER && C_TYPE_P(zv) == IS_LONG) {
			/* number to pointer conversion */
			cdata = (crex_ffi_cdata*)crex_ffi_cdata_new(crex_ffi_cdata_ce);
			cdata->type = type_ptr;
			cdata->ptr = &cdata->ptr_holder;
			cdata->ptr_holder = (void*)(intptr_t)C_LVAL_P(zv);
			if (is_const) {
				cdata->flags |= CREX_FFI_FLAG_CONST;
			}
			RETURN_OBJ(&cdata->std);
		} else if (type->kind == CREX_FFI_TYPE_POINTER && C_TYPE_P(zv) == IS_NULL) {
			/* null -> pointer */
			cdata = (crex_ffi_cdata*)crex_ffi_cdata_new(crex_ffi_cdata_ce);
			cdata->type = type_ptr;
			cdata->ptr = &cdata->ptr_holder;
			cdata->ptr_holder = NULL;
			if (is_const) {
				cdata->flags |= CREX_FFI_FLAG_CONST;
			}
			RETURN_OBJ(&cdata->std);
		} else {
			crex_wrong_parameter_class_error(2, "FFI\\CData", zv);
			RETURN_THROWS();
		}
	}

	old_cdata = (crex_ffi_cdata*)C_OBJ_P(zv);
	old_type = CREX_FFI_TYPE(old_cdata->type);
	ptr = old_cdata->ptr;

	cdata = (crex_ffi_cdata*)crex_ffi_cdata_new(crex_ffi_cdata_ce);
	if (type->kind < CREX_FFI_TYPE_POINTER) {
		cdata->std.handlers = &crex_ffi_cdata_value_handlers;
	}
	cdata->type = type_ptr;

	if (old_type->kind == CREX_FFI_TYPE_POINTER
	 && type->kind != CREX_FFI_TYPE_POINTER
	 && CREX_FFI_TYPE(old_type->pointer.type)->kind == CREX_FFI_TYPE_VOID) {
		/* automatically dereference void* pointers ??? */
		cdata->ptr = *(void**)ptr;
	} else if (old_type->kind == CREX_FFI_TYPE_ARRAY
	 && type->kind == CREX_FFI_TYPE_POINTER
	 && crex_ffi_is_compatible_type(CREX_FFI_TYPE(old_type->array.type), CREX_FFI_TYPE(type->pointer.type))) {		cdata->ptr = &cdata->ptr_holder;
 		cdata->ptr = &cdata->ptr_holder;
 		cdata->ptr_holder = old_cdata->ptr;
	} else if (old_type->kind == CREX_FFI_TYPE_POINTER
	 && type->kind == CREX_FFI_TYPE_ARRAY
	 && crex_ffi_is_compatible_type(CREX_FFI_TYPE(old_type->pointer.type), CREX_FFI_TYPE(type->array.type))) {
		cdata->ptr = old_cdata->ptr_holder;
	} else if (type->size > old_type->size) {
		crex_object_release(&cdata->std);
		crex_throw_error(crex_ffi_exception_ce, "attempt to cast to larger type");
		RETURN_THROWS();
	} else if (ptr != &old_cdata->ptr_holder) {
		cdata->ptr = ptr;
	} else {
		cdata->ptr = &cdata->ptr_holder;
		cdata->ptr_holder = old_cdata->ptr_holder;
	}
	if (is_const) {
		cdata->flags |= CREX_FFI_FLAG_CONST;
	}

	if (old_cdata->flags & CREX_FFI_FLAG_OWNED) {
		if (GC_REFCOUNT(&old_cdata->std) == 1 && C_REFCOUNT_P(arg) == 1) {
			/* transfer ownership */
			old_cdata->flags &= ~CREX_FFI_FLAG_OWNED;
			cdata->flags |= CREX_FFI_FLAG_OWNED;
		} else {
			//???crex_throw_error(crex_ffi_exception_ce, "Attempt to cast owned C pointer");
		}
	}

	RETURN_OBJ(&cdata->std);
}
/* }}} */

CREX_METHOD(FFI, type) /* {{{ */
{
	crex_ffi_ctype *ctype;
	crex_ffi_dcl dcl = CREX_FFI_ATTR_INIT;
	crex_string *type_def;
	bool is_static_call = C_TYPE(EX(This)) != IS_OBJECT;

	CREX_FFI_VALIDATE_API_RESTRICTION();
	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(type_def);
	CREX_PARSE_PARAMETERS_END();

	if (is_static_call) {
		crex_error(E_DEPRECATED, "Calling FFI::type() statically is deprecated");
		if (EG(exception)) {
			RETURN_THROWS();
		}
	}

	if (!is_static_call) {
		crex_ffi *ffi = (crex_ffi*)C_OBJ(EX(This));
		FFI_G(symbols) = ffi->symbols;
		FFI_G(tags) = ffi->tags;
	} else {
		FFI_G(symbols) = NULL;
		FFI_G(tags) = NULL;
	}
	bool clean_symbols = FFI_G(symbols) == NULL;
	bool clean_tags = FFI_G(tags) == NULL;

	FFI_G(default_type_attr) = 0;

	if (crex_ffi_parse_type(ZSTR_VAL(type_def), ZSTR_LEN(type_def), &dcl) == FAILURE) {
		crex_ffi_type_dtor(dcl.type);
		if (clean_tags && FFI_G(tags)) {
			crex_hash_destroy(FFI_G(tags));
			efree(FFI_G(tags));
			FFI_G(tags) = NULL;
		}
		if (clean_symbols && FFI_G(symbols)) {
			crex_hash_destroy(FFI_G(symbols));
			efree(FFI_G(symbols));
			FFI_G(symbols) = NULL;
		}
		return;
	}

	if (clean_tags && FFI_G(tags)) {
		crex_ffi_tags_cleanup(&dcl);
	}
	if (clean_symbols && FFI_G(symbols)) {
		crex_hash_destroy(FFI_G(symbols));
		efree(FFI_G(symbols));
		FFI_G(symbols) = NULL;
	}
	FFI_G(symbols) = NULL;
	FFI_G(tags) = NULL;

	ctype = (crex_ffi_ctype*)crex_ffi_ctype_new(crex_ffi_ctype_ce);
	ctype->type = dcl.type;

	RETURN_OBJ(&ctype->std);
}
/* }}} */

CREX_METHOD(FFI, typeof) /* {{{ */
{
	zval *zv, *arg;
	crex_ffi_ctype *ctype;
	crex_ffi_type *type;

	CREX_FFI_VALIDATE_API_RESTRICTION();
	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(zv);
	CREX_PARSE_PARAMETERS_END();

	arg = zv;
	ZVAL_DEREF(zv);
	if (C_TYPE_P(zv) == IS_OBJECT && C_OBJCE_P(zv) == crex_ffi_cdata_ce) {
		crex_ffi_cdata *cdata = (crex_ffi_cdata*)C_OBJ_P(zv);

		type = cdata->type;
		if (CREX_FFI_TYPE_IS_OWNED(type)) {
			type = CREX_FFI_TYPE(type);
			if (!(type->attr & CREX_FFI_ATTR_STORED)) {
				if (GC_REFCOUNT(&cdata->std) == 1 && C_REFCOUNT_P(arg) == 1) {
					/* transfer type ownership */
					cdata->type = type;
					type = CREX_FFI_TYPE_MAKE_OWNED(type);
				} else {
					cdata->type = type = crex_ffi_remember_type(type);
				}
			}
		}
	} else {
		crex_wrong_parameter_class_error(1, "FFI\\CData", zv);
		RETURN_THROWS();
	}

	ctype = (crex_ffi_ctype*)crex_ffi_ctype_new(crex_ffi_ctype_ce);
	ctype->type = type;

	RETURN_OBJ(&ctype->std);
}
/* }}} */

CREX_METHOD(FFI, arrayType) /* {{{ */
{
	zval *ztype;
	crex_ffi_ctype *ctype;
	crex_ffi_type *type;
	HashTable *dims;
	zval *val;

	CREX_FFI_VALIDATE_API_RESTRICTION();
	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_OBJECT_OF_CLASS(ztype, crex_ffi_ctype_ce)
		C_PARAM_ARRAY_HT(dims)
	CREX_PARSE_PARAMETERS_END();

	ctype = (crex_ffi_ctype*)C_OBJ_P(ztype);
	type = CREX_FFI_TYPE(ctype->type);

	if (type->kind == CREX_FFI_TYPE_FUNC) {
		crex_throw_error(crex_ffi_exception_ce, "Array of functions is not allowed");
		RETURN_THROWS();
	} else if (type->kind == CREX_FFI_TYPE_ARRAY && (type->attr & CREX_FFI_ATTR_INCOMPLETE_ARRAY)) {
		crex_throw_error(crex_ffi_exception_ce, "Only the leftmost array can be undimensioned");
		RETURN_THROWS();
	} else if (type->kind == CREX_FFI_TYPE_VOID) {
		crex_throw_error(crex_ffi_exception_ce, "Array of void type is not allowed");
		RETURN_THROWS();
	} else if (type->attr & CREX_FFI_ATTR_INCOMPLETE_TAG) {
		crex_throw_error(crex_ffi_exception_ce, "Array of incomplete type is not allowed");
		RETURN_THROWS();
	}

	if (CREX_FFI_TYPE_IS_OWNED(ctype->type)) {
		if (!(type->attr & CREX_FFI_ATTR_STORED)) {
			if (GC_REFCOUNT(&ctype->std) == 1) {
				/* transfer type ownership */
				ctype->type = type;
				type = CREX_FFI_TYPE_MAKE_OWNED(type);
			} else {
				ctype->type = type = crex_ffi_remember_type(type);
			}
		}
	}

	CREX_HASH_REVERSE_FOREACH_VAL(dims, val) {
		crex_long n = zval_get_long(val);
		crex_ffi_type *new_type;

		if (n < 0) {
			crex_throw_error(crex_ffi_exception_ce, "negative array index");
			crex_ffi_type_dtor(type);
			RETURN_THROWS();
		} else if (CREX_FFI_TYPE(type)->kind == CREX_FFI_TYPE_ARRAY && (CREX_FFI_TYPE(type)->attr & CREX_FFI_ATTR_INCOMPLETE_ARRAY)) {
			crex_throw_error(crex_ffi_exception_ce, "only the leftmost array can be undimensioned");
			crex_ffi_type_dtor(type);
			RETURN_THROWS();
		}

		new_type = emalloc(sizeof(crex_ffi_type));
		new_type->kind = CREX_FFI_TYPE_ARRAY;
		new_type->attr = 0;
		new_type->size = n * CREX_FFI_TYPE(type)->size;
		new_type->align = CREX_FFI_TYPE(type)->align;
		new_type->array.type = type;
		new_type->array.length = n;

		if (n == 0) {
			new_type->attr |= CREX_FFI_ATTR_INCOMPLETE_ARRAY;
		}

		type = CREX_FFI_TYPE_MAKE_OWNED(new_type);
	} CREX_HASH_FOREACH_END();

	ctype = (crex_ffi_ctype*)crex_ffi_ctype_new(crex_ffi_ctype_ce);
	ctype->type = type;

	RETURN_OBJ(&ctype->std);
}
/* }}} */

CREX_METHOD(FFI, addr) /* {{{ */
{
	crex_ffi_type *type, *new_type;
	crex_ffi_cdata *cdata, *new_cdata;
	zval *zv, *arg;

	CREX_FFI_VALIDATE_API_RESTRICTION();
	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(zv)
	CREX_PARSE_PARAMETERS_END();

	arg = zv;
	ZVAL_DEREF(zv);
	if (C_TYPE_P(zv) != IS_OBJECT || C_OBJCE_P(zv) != crex_ffi_cdata_ce) {
		crex_wrong_parameter_class_error(1, "FFI\\CData", zv);
		RETURN_THROWS();
	}

	cdata = (crex_ffi_cdata*)C_OBJ_P(zv);
	type = CREX_FFI_TYPE(cdata->type);

	if (GC_REFCOUNT(&cdata->std) == 1 && C_REFCOUNT_P(arg) == 1 && type->kind == CREX_FFI_TYPE_POINTER) {
		crex_throw_error(crex_ffi_exception_ce, "FFI::addr() cannot create a reference to a temporary pointer");
		RETURN_THROWS();
	}

	new_type = emalloc(sizeof(crex_ffi_type));
	new_type->kind = CREX_FFI_TYPE_POINTER;
	new_type->attr = 0;
	new_type->size = sizeof(void*);
	new_type->align = _Alignof(void*);
	/* life-time (source must relive the resulting pointer) ??? */
	new_type->pointer.type = type;

	new_cdata = (crex_ffi_cdata*)crex_ffi_cdata_new(crex_ffi_cdata_ce);
	new_cdata->type = CREX_FFI_TYPE_MAKE_OWNED(new_type);
	new_cdata->ptr_holder = cdata->ptr;
	new_cdata->ptr = &new_cdata->ptr_holder;

	if (GC_REFCOUNT(&cdata->std) == 1 && C_REFCOUNT_P(arg) == 1) {
		if (CREX_FFI_TYPE_IS_OWNED(cdata->type)) {
			/* transfer type ownership */
			cdata->type = type;
			new_type->pointer.type = CREX_FFI_TYPE_MAKE_OWNED(type);
		}
		if (cdata->flags & CREX_FFI_FLAG_OWNED) {
			/* transfer ownership */
			cdata->flags &= ~CREX_FFI_FLAG_OWNED;
			new_cdata->flags |= CREX_FFI_FLAG_OWNED;
		}
	}

	RETURN_OBJ(&new_cdata->std);
}
/* }}} */

CREX_METHOD(FFI, sizeof) /* {{{ */
{
	zval *zv;
	crex_ffi_type *type;

	CREX_FFI_VALIDATE_API_RESTRICTION();
	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(zv);
	CREX_PARSE_PARAMETERS_END();

	ZVAL_DEREF(zv);
	if (C_TYPE_P(zv) == IS_OBJECT && C_OBJCE_P(zv) == crex_ffi_cdata_ce) {
		crex_ffi_cdata *cdata = (crex_ffi_cdata*)C_OBJ_P(zv);
		type = CREX_FFI_TYPE(cdata->type);
	} else if (C_TYPE_P(zv) == IS_OBJECT && C_OBJCE_P(zv) == crex_ffi_ctype_ce) {
		crex_ffi_ctype *ctype = (crex_ffi_ctype*)C_OBJ_P(zv);
		type = CREX_FFI_TYPE(ctype->type);
	} else {
		crex_wrong_parameter_class_error(1, "FFI\\CData or FFI\\CType", zv);
		RETURN_THROWS();
	}

	RETURN_LONG(type->size);
}
/* }}} */

CREX_METHOD(FFI, alignof) /* {{{ */
{
	zval *zv;
	crex_ffi_type *type;

	CREX_FFI_VALIDATE_API_RESTRICTION();
	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(zv);
	CREX_PARSE_PARAMETERS_END();

	ZVAL_DEREF(zv);
	if (C_TYPE_P(zv) == IS_OBJECT && C_OBJCE_P(zv) == crex_ffi_cdata_ce) {
		crex_ffi_cdata *cdata = (crex_ffi_cdata*)C_OBJ_P(zv);
		type = CREX_FFI_TYPE(cdata->type);
	} else if (C_TYPE_P(zv) == IS_OBJECT && C_OBJCE_P(zv) == crex_ffi_ctype_ce) {
		crex_ffi_ctype *ctype = (crex_ffi_ctype*)C_OBJ_P(zv);
		type = CREX_FFI_TYPE(ctype->type);
	} else {
		crex_wrong_parameter_class_error(1, "FFI\\CData or FFI\\CType", zv);
		RETURN_THROWS();
	}

	RETURN_LONG(type->align);
}
/* }}} */

CREX_METHOD(FFI, memcpy) /* {{{ */
{
	zval *zv1, *zv2;
	crex_ffi_cdata *cdata1, *cdata2;
	crex_ffi_type *type1, *type2;
	void *ptr1, *ptr2;
	crex_long size;

	CREX_FFI_VALIDATE_API_RESTRICTION();
	CREX_PARSE_PARAMETERS_START(3, 3)
		C_PARAM_OBJECT_OF_CLASS_EX(zv1, crex_ffi_cdata_ce, 0, 1);
		C_PARAM_ZVAL(zv2)
		C_PARAM_LONG(size)
	CREX_PARSE_PARAMETERS_END();

	cdata1 = (crex_ffi_cdata*)C_OBJ_P(zv1);
	type1 = CREX_FFI_TYPE(cdata1->type);
	if (type1->kind == CREX_FFI_TYPE_POINTER) {
		ptr1 = *(void**)cdata1->ptr;
	} else {
		ptr1 = cdata1->ptr;
		if (type1->kind != CREX_FFI_TYPE_POINTER && size > type1->size) {
			crex_throw_error(crex_ffi_exception_ce, "Attempt to write over data boundary");
			RETURN_THROWS();
		}
	}

	ZVAL_DEREF(zv2);
	if (C_TYPE_P(zv2) == IS_STRING) {
		ptr2 = C_STRVAL_P(zv2);
		if (size > C_STRLEN_P(zv2)) {
			crex_throw_error(crex_ffi_exception_ce, "Attempt to read over string boundary");
			RETURN_THROWS();
		}
	} else if (C_TYPE_P(zv2) == IS_OBJECT && C_OBJCE_P(zv2) == crex_ffi_cdata_ce) {
		cdata2 = (crex_ffi_cdata*)C_OBJ_P(zv2);
		type2 = CREX_FFI_TYPE(cdata2->type);
		if (type2->kind == CREX_FFI_TYPE_POINTER) {
			ptr2 = *(void**)cdata2->ptr;
		} else {
			ptr2 = cdata2->ptr;
			if (type2->kind != CREX_FFI_TYPE_POINTER && size > type2->size) {
				crex_throw_error(crex_ffi_exception_ce, "Attempt to read over data boundary");
				RETURN_THROWS();
			}
		}
	} else {
		crex_wrong_parameter_class_error(2, "FFI\\CData or string", zv2);
		RETURN_THROWS();
	}

	memcpy(ptr1, ptr2, size);
}
/* }}} */

CREX_METHOD(FFI, memcmp) /* {{{ */
{
	zval *zv1, *zv2;
	crex_ffi_cdata *cdata1, *cdata2;
	crex_ffi_type *type1, *type2;
	void *ptr1, *ptr2;
	crex_long size;
	int ret;

	CREX_FFI_VALIDATE_API_RESTRICTION();
	CREX_PARSE_PARAMETERS_START(3, 3)
		C_PARAM_ZVAL(zv1);
		C_PARAM_ZVAL(zv2);
		C_PARAM_LONG(size)
	CREX_PARSE_PARAMETERS_END();

	ZVAL_DEREF(zv1);
	if (C_TYPE_P(zv1) == IS_STRING) {
		ptr1 = C_STRVAL_P(zv1);
		if (size > C_STRLEN_P(zv1)) {
			crex_throw_error(crex_ffi_exception_ce, "attempt to read over string boundary");
			RETURN_THROWS();
		}
	} else if (C_TYPE_P(zv1) == IS_OBJECT && C_OBJCE_P(zv1) == crex_ffi_cdata_ce) {
		cdata1 = (crex_ffi_cdata*)C_OBJ_P(zv1);
		type1 = CREX_FFI_TYPE(cdata1->type);
		if (type1->kind == CREX_FFI_TYPE_POINTER) {
			ptr1 = *(void**)cdata1->ptr;
		} else {
			ptr1 = cdata1->ptr;
			if (type1->kind != CREX_FFI_TYPE_POINTER && size > type1->size) {
				crex_throw_error(crex_ffi_exception_ce, "attempt to read over data boundary");
				RETURN_THROWS();
			}
		}
	} else {
		crex_wrong_parameter_class_error(1, "FFI\\CData or string", zv1);
		RETURN_THROWS();
	}

	ZVAL_DEREF(zv2);
	if (C_TYPE_P(zv2) == IS_STRING) {
		ptr2 = C_STRVAL_P(zv2);
		if (size > C_STRLEN_P(zv2)) {
			crex_throw_error(crex_ffi_exception_ce, "Attempt to read over string boundary");
			RETURN_THROWS();
		}
	} else if (C_TYPE_P(zv2) == IS_OBJECT && C_OBJCE_P(zv2) == crex_ffi_cdata_ce) {
		cdata2 = (crex_ffi_cdata*)C_OBJ_P(zv2);
		type2 = CREX_FFI_TYPE(cdata2->type);
		if (type2->kind == CREX_FFI_TYPE_POINTER) {
			ptr2 = *(void**)cdata2->ptr;
		} else {
			ptr2 = cdata2->ptr;
			if (type2->kind != CREX_FFI_TYPE_POINTER && size > type2->size) {
				crex_throw_error(crex_ffi_exception_ce, "Attempt to read over data boundary");
				RETURN_THROWS();
			}
		}
	} else {
		crex_wrong_parameter_class_error(2, "FFI\\CData or string", zv2);
		RETURN_THROWS();
	}

	ret = memcmp(ptr1, ptr2, size);
	if (ret == 0) {
		RETVAL_LONG(0);
	} else if (ret < 0) {
		RETVAL_LONG(-1);
	} else {
		RETVAL_LONG(1);
	}
}
/* }}} */

CREX_METHOD(FFI, memset) /* {{{ */
{
	zval *zv;
	crex_ffi_cdata *cdata;
	crex_ffi_type *type;
	void *ptr;
	crex_long ch, size;

	CREX_FFI_VALIDATE_API_RESTRICTION();
	CREX_PARSE_PARAMETERS_START(3, 3)
		C_PARAM_OBJECT_OF_CLASS_EX(zv, crex_ffi_cdata_ce, 0, 1);
		C_PARAM_LONG(ch)
		C_PARAM_LONG(size)
	CREX_PARSE_PARAMETERS_END();

	cdata = (crex_ffi_cdata*)C_OBJ_P(zv);
	type = CREX_FFI_TYPE(cdata->type);
	if (type->kind == CREX_FFI_TYPE_POINTER) {
		ptr = *(void**)cdata->ptr;
	} else {
		ptr = cdata->ptr;
		if (type->kind != CREX_FFI_TYPE_POINTER && size > type->size) {
			crex_throw_error(crex_ffi_exception_ce, "attempt to write over data boundary");
			RETURN_THROWS();
		}
	}

	memset(ptr, ch, size);
}
/* }}} */

CREX_METHOD(FFI, string) /* {{{ */
{
	zval *zv;
	crex_ffi_cdata *cdata;
	crex_ffi_type *type;
	void *ptr;
	crex_long size;
	bool size_is_null = 1;

	CREX_FFI_VALIDATE_API_RESTRICTION();
	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_OBJECT_OF_CLASS_EX(zv, crex_ffi_cdata_ce, 0, 1);
		C_PARAM_OPTIONAL
		C_PARAM_LONG_OR_NULL(size, size_is_null)
	CREX_PARSE_PARAMETERS_END();

	cdata = (crex_ffi_cdata*)C_OBJ_P(zv);
	type = CREX_FFI_TYPE(cdata->type);
	if (!size_is_null) {
		if (type->kind == CREX_FFI_TYPE_POINTER) {
			ptr = *(void**)cdata->ptr;
		} else {
			ptr = cdata->ptr;
			if (type->kind != CREX_FFI_TYPE_POINTER && size > type->size) {
				crex_throw_error(crex_ffi_exception_ce, "attempt to read over data boundary");
				RETURN_THROWS();
			}
		}
		RETURN_STRINGL((char*)ptr, size);
	} else {
		if (type->kind == CREX_FFI_TYPE_POINTER && CREX_FFI_TYPE(type->pointer.type)->kind == CREX_FFI_TYPE_CHAR) {
			ptr = *(void**)cdata->ptr;
		} else if (type->kind == CREX_FFI_TYPE_ARRAY && CREX_FFI_TYPE(type->array.type)->kind == CREX_FFI_TYPE_CHAR) {
			ptr = cdata->ptr;
		} else {
			crex_throw_error(crex_ffi_exception_ce, "FFI\\Cdata is not a C string");
			RETURN_THROWS();
		}
		RETURN_STRING((char*)ptr);
	}
}
/* }}} */

CREX_METHOD(FFI, isNull) /* {{{ */
{
	zval *zv;
	crex_ffi_cdata *cdata;
	crex_ffi_type *type;

	CREX_FFI_VALIDATE_API_RESTRICTION();
	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(zv);
	CREX_PARSE_PARAMETERS_END();

	ZVAL_DEREF(zv);
	if (C_TYPE_P(zv) != IS_OBJECT || C_OBJCE_P(zv) != crex_ffi_cdata_ce) {
		crex_wrong_parameter_class_error(1, "FFI\\CData", zv);
		RETURN_THROWS();
	}

	cdata = (crex_ffi_cdata*)C_OBJ_P(zv);
	type = CREX_FFI_TYPE(cdata->type);

	if (type->kind != CREX_FFI_TYPE_POINTER){
		crex_throw_error(crex_ffi_exception_ce, "FFI\\Cdata is not a pointer");
		RETURN_THROWS();
	}

	RETURN_BOOL(*(void**)cdata->ptr == NULL);
}
/* }}} */


CREX_METHOD(FFI_CType, getName) /* {{{ */
{
	crex_ffi_ctype *ctype = (crex_ffi_ctype*)(C_OBJ_P(CREX_THIS));
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	crex_ffi_ctype_name_buf buf;

	buf.start = buf.end = buf.buf + ((MAX_TYPE_NAME_LEN * 3) / 4);
	if (!crex_ffi_ctype_name(&buf, CREX_FFI_TYPE(ctype->type))) {
		RETURN_STR_COPY(C_OBJ_P(CREX_THIS)->ce->name);
	} else {
		size_t len = buf.end - buf.start;
		crex_string *res = crex_string_init(buf.start, len, 0);
		RETURN_STR(res);
	}
}
/* }}} */

CREX_METHOD(FFI_CType, getKind) /* {{{ */
{
	crex_ffi_ctype *ctype = (crex_ffi_ctype*)(C_OBJ_P(CREX_THIS));
	crex_ffi_type *type;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	type = CREX_FFI_TYPE(ctype->type);
	RETURN_LONG(type->kind);
}
/* }}} */

CREX_METHOD(FFI_CType, getSize) /* {{{ */
{
	crex_ffi_ctype *ctype = (crex_ffi_ctype*)(C_OBJ_P(CREX_THIS));
	crex_ffi_type *type;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	type = CREX_FFI_TYPE(ctype->type);
	RETURN_LONG(type->size);
}
/* }}} */

CREX_METHOD(FFI_CType, getAlignment) /* {{{ */
{
	crex_ffi_ctype *ctype = (crex_ffi_ctype*)(C_OBJ_P(CREX_THIS));
	crex_ffi_type *type;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	type = CREX_FFI_TYPE(ctype->type);
	RETURN_LONG(type->align);
}
/* }}} */

CREX_METHOD(FFI_CType, getAttributes) /* {{{ */
{
	crex_ffi_ctype *ctype = (crex_ffi_ctype*)(C_OBJ_P(CREX_THIS));
	crex_ffi_type *type;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	type = CREX_FFI_TYPE(ctype->type);
	RETURN_LONG(type->attr);
}
/* }}} */

CREX_METHOD(FFI_CType, getEnumKind) /* {{{ */
{
	crex_ffi_ctype *ctype = (crex_ffi_ctype*)(C_OBJ_P(CREX_THIS));
	crex_ffi_type *type;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	type = CREX_FFI_TYPE(ctype->type);
	if (type->kind != CREX_FFI_TYPE_ENUM) {
		crex_throw_error(crex_ffi_exception_ce, "FFI\\CType is not an enumeration");
		RETURN_THROWS();
	}
	RETURN_LONG(type->enumeration.kind);
}
/* }}} */

CREX_METHOD(FFI_CType, getArrayElementType) /* {{{ */
{
	crex_ffi_ctype *ctype = (crex_ffi_ctype*)(C_OBJ_P(CREX_THIS));
	crex_ffi_type *type;
	crex_ffi_ctype *ret;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	type = CREX_FFI_TYPE(ctype->type);
	if (type->kind != CREX_FFI_TYPE_ARRAY) {
		crex_throw_error(crex_ffi_exception_ce, "FFI\\CType is not an array");
		RETURN_THROWS();
	}

	ret = (crex_ffi_ctype*)crex_ffi_ctype_new(crex_ffi_ctype_ce);
	ret->type = CREX_FFI_TYPE(type->array.type);
	RETURN_OBJ(&ret->std);
}
/* }}} */

CREX_METHOD(FFI_CType, getArrayLength) /* {{{ */
{
	crex_ffi_ctype *ctype = (crex_ffi_ctype*)(C_OBJ_P(CREX_THIS));
	crex_ffi_type *type;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	type = CREX_FFI_TYPE(ctype->type);
	if (type->kind != CREX_FFI_TYPE_ARRAY) {
		crex_throw_error(crex_ffi_exception_ce, "FFI\\CType is not an array");
		RETURN_THROWS();
	}
	RETURN_LONG(type->array.length);
}
/* }}} */

CREX_METHOD(FFI_CType, getPointerType) /* {{{ */
{
	crex_ffi_ctype *ctype = (crex_ffi_ctype*)(C_OBJ_P(CREX_THIS));
	crex_ffi_ctype *ret;
	crex_ffi_type *type;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	type = CREX_FFI_TYPE(ctype->type);
	if (type->kind != CREX_FFI_TYPE_POINTER) {
		crex_throw_error(crex_ffi_exception_ce, "FFI\\CType is not a pointer");
		RETURN_THROWS();
	}

	ret = (crex_ffi_ctype*)crex_ffi_ctype_new(crex_ffi_ctype_ce);
	ret->type = CREX_FFI_TYPE(type->pointer.type);
	RETURN_OBJ(&ret->std);
}
/* }}} */

CREX_METHOD(FFI_CType, getStructFieldNames) /* {{{ */
{
	crex_ffi_ctype *ctype = (crex_ffi_ctype*)(C_OBJ_P(CREX_THIS));
	crex_ffi_type *type;
	HashTable *ht;
	crex_string* name;
	zval zv;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	type = CREX_FFI_TYPE(ctype->type);
	if (type->kind != CREX_FFI_TYPE_STRUCT) {
		crex_throw_error(crex_ffi_exception_ce, "FFI\\CType is not a structure");
		RETURN_THROWS();
	}

	ht = crex_new_array(crex_hash_num_elements(&type->record.fields));
	RETVAL_ARR(ht);
	CREX_HASH_MAP_FOREACH_STR_KEY(&type->record.fields, name) {
		ZVAL_STR_COPY(&zv, name);
		crex_hash_next_index_insert_new(ht, &zv);
	} CREX_HASH_FOREACH_END();
}
/* }}} */

CREX_METHOD(FFI_CType, getStructFieldOffset) /* {{{ */
{
	crex_ffi_ctype *ctype = (crex_ffi_ctype*)(C_OBJ_P(CREX_THIS));
	crex_ffi_type *type;
	crex_string *name;
	crex_ffi_field *ptr;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(name)
	CREX_PARSE_PARAMETERS_END();

	type = CREX_FFI_TYPE(ctype->type);
	if (type->kind != CREX_FFI_TYPE_STRUCT) {
		crex_throw_error(crex_ffi_exception_ce, "FFI\\CType is not a structure");
		RETURN_THROWS();
	}

	ptr = crex_hash_find_ptr(&type->record.fields, name);
	if (!ptr) {
		crex_throw_error(crex_ffi_exception_ce, "Wrong field name");
		RETURN_THROWS();
	}
	RETURN_LONG(ptr->offset);
}
/* }}} */

CREX_METHOD(FFI_CType, getStructFieldType) /* {{{ */
{
	crex_ffi_ctype *ctype = (crex_ffi_ctype*)(C_OBJ_P(CREX_THIS));
	crex_ffi_type *type;
	crex_string *name;
	crex_ffi_field *ptr;
	crex_ffi_ctype *ret;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(name)
	CREX_PARSE_PARAMETERS_END();

	type = CREX_FFI_TYPE(ctype->type);
	if (type->kind != CREX_FFI_TYPE_STRUCT) {
		crex_throw_error(crex_ffi_exception_ce, "FFI\\CType is not a structure");
		RETURN_THROWS();
	}

	ptr = crex_hash_find_ptr(&type->record.fields, name);
	if (!ptr) {
		crex_throw_error(crex_ffi_exception_ce, "Wrong field name");
		RETURN_THROWS();
	}

	ret = (crex_ffi_ctype*)crex_ffi_ctype_new(crex_ffi_ctype_ce);
	ret->type = CREX_FFI_TYPE(ptr->type);
	RETURN_OBJ(&ret->std);
}
/* }}} */

CREX_METHOD(FFI_CType, getFuncABI) /* {{{ */
{
	crex_ffi_ctype *ctype = (crex_ffi_ctype*)(C_OBJ_P(CREX_THIS));
	crex_ffi_type *type;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	type = CREX_FFI_TYPE(ctype->type);
	if (type->kind != CREX_FFI_TYPE_FUNC) {
		crex_throw_error(crex_ffi_exception_ce, "FFI\\CType is not a function");
		RETURN_THROWS();
	}
	RETURN_LONG(type->func.abi);
}
/* }}} */

CREX_METHOD(FFI_CType, getFuncReturnType) /* {{{ */
{
	crex_ffi_ctype *ctype = (crex_ffi_ctype*)(C_OBJ_P(CREX_THIS));
	crex_ffi_ctype *ret;
	crex_ffi_type *type;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	type = CREX_FFI_TYPE(ctype->type);
	if (type->kind != CREX_FFI_TYPE_FUNC) {
		crex_throw_error(crex_ffi_exception_ce, "FFI\\CType is not a function");
		RETURN_THROWS();
	}

	ret = (crex_ffi_ctype*)crex_ffi_ctype_new(crex_ffi_ctype_ce);
	ret->type = CREX_FFI_TYPE(type->func.ret_type);
	RETURN_OBJ(&ret->std);
}
/* }}} */

CREX_METHOD(FFI_CType, getFuncParameterCount) /* {{{ */
{
	crex_ffi_ctype *ctype = (crex_ffi_ctype*)(C_OBJ_P(CREX_THIS));
	crex_ffi_type *type;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	type = CREX_FFI_TYPE(ctype->type);
	if (type->kind != CREX_FFI_TYPE_FUNC) {
		crex_throw_error(crex_ffi_exception_ce, "FFI\\CType is not a function");
		RETURN_THROWS();
	}
	RETURN_LONG(type->func.args ? crex_hash_num_elements(type->func.args) : 0);
}
/* }}} */

CREX_METHOD(FFI_CType, getFuncParameterType) /* {{{ */
{
	crex_ffi_ctype *ctype = (crex_ffi_ctype*)(C_OBJ_P(CREX_THIS));
	crex_ffi_type *type, *ptr;
	crex_long n;
	crex_ffi_ctype *ret;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(n)
	CREX_PARSE_PARAMETERS_END();

	type = CREX_FFI_TYPE(ctype->type);
	if (type->kind != CREX_FFI_TYPE_FUNC) {
		crex_throw_error(crex_ffi_exception_ce, "FFI\\CType is not a function");
		RETURN_THROWS();
	}

	if (!type->func.args) {
		crex_throw_error(crex_ffi_exception_ce, "Wrong argument number");
		RETURN_THROWS();
	}

	ptr = crex_hash_index_find_ptr(type->func.args, n);
	if (!ptr) {
		crex_throw_error(crex_ffi_exception_ce, "Wrong argument number");
		RETURN_THROWS();
	}

	ret = (crex_ffi_ctype*)crex_ffi_ctype_new(crex_ffi_ctype_ce);
	ret->type = CREX_FFI_TYPE(ptr);
	RETURN_OBJ(&ret->std);
}
/* }}} */

static char *crex_ffi_parse_directives(const char *filename, char *code_pos, char **scope_name, char **lib, bool preload) /* {{{ */
{
	char *p;

	*scope_name = NULL;
	*lib = NULL;
	while (*code_pos == '#') {
		if (strncmp(code_pos, "#define FFI_SCOPE", sizeof("#define FFI_SCOPE") - 1) == 0
		 && (code_pos[sizeof("#define FFI_SCOPE") - 1] == ' '
		  || code_pos[sizeof("#define FFI_SCOPE") - 1] == '\t')) {
			p = code_pos + sizeof("#define FFI_SCOPE");
			while (*p == ' ' || *p == '\t') {
				p++;
			}
			if (*p != '"') {
				if (preload) {
					crex_error(E_WARNING, "FFI: failed pre-loading '%s', bad FFI_SCOPE define", filename);
				} else {
					crex_throw_error(crex_ffi_exception_ce, "Failed loading '%s', bad FFI_SCOPE define", filename);
				}
				return NULL;
			}
			p++;
			if (*scope_name) {
				if (preload) {
					crex_error(E_WARNING, "FFI: failed pre-loading '%s', FFI_SCOPE defined twice", filename);
				} else {
					crex_throw_error(crex_ffi_exception_ce, "Failed loading '%s', FFI_SCOPE defined twice", filename);
				}
				return NULL;
			}
			*scope_name = p;
			while (1) {
				if (*p == '\"') {
					*p = 0;
					p++;
					break;
				} else if (*p <= ' ') {
					if (preload) {
						crex_error(E_WARNING, "FFI: failed pre-loading '%s', bad FFI_SCOPE define", filename);
					} else {
						crex_throw_error(crex_ffi_exception_ce, "Failed loading '%s', bad FFI_SCOPE define", filename);
					}
					return NULL;
				}
				p++;
			}
			while (*p == ' ' || *p == '\t') {
				p++;
			}
			while (*p == '\r' || *p == '\n') {
				p++;
			}
			code_pos = p;
		} else if (strncmp(code_pos, "#define FFI_LIB", sizeof("#define FFI_LIB") - 1) == 0
		 && (code_pos[sizeof("#define FFI_LIB") - 1] == ' '
		  || code_pos[sizeof("#define FFI_LIB") - 1] == '\t')) {
			p = code_pos + sizeof("#define FFI_LIB");
			while (*p == ' ' || *p == '\t') {
				p++;
			}
			if (*p != '"') {
				if (preload) {
					crex_error(E_WARNING, "FFI: failed pre-loading '%s', bad FFI_LIB define", filename);
				} else {
					crex_throw_error(crex_ffi_exception_ce, "Failed loading '%s', bad FFI_LIB define", filename);
				}
				return NULL;
			}
			p++;
			if (*lib) {
				if (preload) {
					crex_error(E_WARNING, "FFI: failed pre-loading '%s', FFI_LIB defined twice", filename);
				} else {
					crex_throw_error(crex_ffi_exception_ce, "Failed loading '%s', FFI_LIB defined twice", filename);
				}
				return NULL;
			}
			*lib = p;
			while (1) {
				if (*p == '\"') {
					*p = 0;
					p++;
					break;
				} else if (*p <= ' ') {
					if (preload) {
						crex_error(E_WARNING, "FFI: failed pre-loading '%s', bad FFI_LIB define", filename);
					} else {
						crex_throw_error(crex_ffi_exception_ce, "Failed loading '%s', bad FFI_LIB define", filename);
					}
					return NULL;
				}
				p++;
			}
			while (*p == ' ' || *p == '\t') {
				p++;
			}
			while (*p == '\r' || *p == '\n') {
				p++;
			}
			code_pos = p;
		} else {
			break;
		}
	}
	return code_pos;
}
/* }}} */

static CREX_COLD crex_function *crex_fake_get_constructor(crex_object *object) /* {{{ */
{
	crex_throw_error(NULL, "Instantiation of %s is not allowed", ZSTR_VAL(object->ce->name));
	return NULL;
}
/* }}} */

static CREX_COLD crex_never_inline void crex_bad_array_access(crex_class_entry *ce) /* {{{ */
{
	crex_throw_error(NULL, "Cannot use object of type %s as array", ZSTR_VAL(ce->name));
}
/* }}} */

static CREX_COLD zval *crex_fake_read_dimension(crex_object *obj, zval *offset, int type, zval *rv) /* {{{ */
{
	crex_bad_array_access(obj->ce);
	return NULL;
}
/* }}} */

static CREX_COLD void crex_fake_write_dimension(crex_object *obj, zval *offset, zval *value) /* {{{ */
{
	crex_bad_array_access(obj->ce);
}
/* }}} */

static CREX_COLD int crex_fake_has_dimension(crex_object *obj, zval *offset, int check_empty) /* {{{ */
{
	crex_bad_array_access(obj->ce);
	return 0;
}
/* }}} */

static CREX_COLD void crex_fake_unset_dimension(crex_object *obj, zval *offset) /* {{{ */
{
	crex_bad_array_access(obj->ce);
}
/* }}} */

static CREX_COLD crex_never_inline void crex_bad_property_access(crex_class_entry *ce) /* {{{ */
{
	crex_throw_error(NULL, "Cannot access property of object of type %s", ZSTR_VAL(ce->name));
}
/* }}} */

static CREX_COLD zval *crex_fake_read_property(crex_object *obj, crex_string *member, int type, void **cache_slot, zval *rv) /* {{{ */
{
	crex_bad_property_access(obj->ce);
	return &EG(uninitialized_zval);
}
/* }}} */

static CREX_COLD zval *crex_fake_write_property(crex_object *obj, crex_string *member, zval *value, void **cache_slot) /* {{{ */
{
	crex_bad_array_access(obj->ce);
	return value;
}
/* }}} */

static CREX_COLD int crex_fake_has_property(crex_object *obj, crex_string *member, int has_set_exists, void **cache_slot) /* {{{ */
{
	crex_bad_array_access(obj->ce);
	return 0;
}
/* }}} */

static CREX_COLD void crex_fake_unset_property(crex_object *obj, crex_string *member, void **cache_slot) /* {{{ */
{
	crex_bad_array_access(obj->ce);
}
/* }}} */

static zval *crex_fake_get_property_ptr_ptr(crex_object *obj, crex_string *member, int type, void **cache_slot) /* {{{ */
{
	return NULL;
}
/* }}} */

static CREX_COLD crex_function *crex_fake_get_method(crex_object **obj_ptr, crex_string *method_name, const zval *key) /* {{{ */
{
	crex_class_entry *ce = (*obj_ptr)->ce;
	crex_throw_error(NULL, "Object of type %s does not support method calls", ZSTR_VAL(ce->name));
	return NULL;
}
/* }}} */

static HashTable *crex_fake_get_properties(crex_object *obj) /* {{{ */
{
	return (HashTable*)&crex_empty_array;
}
/* }}} */

static HashTable *crex_fake_get_gc(crex_object *ob, zval **table, int *n) /* {{{ */
{
	*table = NULL;
	*n = 0;
	return NULL;
}
/* }}} */

static crex_result crex_fake_cast_object(crex_object *obj, zval *result, int type)
{
	switch (type) {
		case _IS_BOOL:
			ZVAL_TRUE(result);
			return SUCCESS;
		default:
			return FAILURE;
	}
}

static CREX_COLD crex_never_inline void crex_ffi_use_after_free(void) /* {{{ */
{
	crex_throw_error(crex_ffi_exception_ce, "Use after free()");
}
/* }}} */

static crex_object *crex_ffi_free_clone_obj(crex_object *obj) /* {{{ */
{
	crex_ffi_use_after_free();
	return NULL;
}
/* }}} */

static CREX_COLD zval *crex_ffi_free_read_dimension(crex_object *obj, zval *offset, int type, zval *rv) /* {{{ */
{
	crex_ffi_use_after_free();
	return NULL;
}
/* }}} */

static CREX_COLD void crex_ffi_free_write_dimension(crex_object *obj, zval *offset, zval *value) /* {{{ */
{
	crex_ffi_use_after_free();
}
/* }}} */

static CREX_COLD int crex_ffi_free_has_dimension(crex_object *obj, zval *offset, int check_empty) /* {{{ */
{
	crex_ffi_use_after_free();
	return 0;
}
/* }}} */

static CREX_COLD void crex_ffi_free_unset_dimension(crex_object *obj, zval *offset) /* {{{ */
{
	crex_ffi_use_after_free();
}
/* }}} */

static CREX_COLD zval *crex_ffi_free_read_property(crex_object *obj, crex_string *member, int type, void **cache_slot, zval *rv) /* {{{ */
{
	crex_ffi_use_after_free();
	return &EG(uninitialized_zval);
}
/* }}} */

static CREX_COLD zval *crex_ffi_free_write_property(crex_object *obj, crex_string *member, zval *value, void **cache_slot) /* {{{ */
{
	crex_ffi_use_after_free();
	return value;
}
/* }}} */

static CREX_COLD int crex_ffi_free_has_property(crex_object *obj, crex_string *member, int has_set_exists, void **cache_slot) /* {{{ */
{
	crex_ffi_use_after_free();
	return 0;
}
/* }}} */

static CREX_COLD void crex_ffi_free_unset_property(crex_object *obj, crex_string *member, void **cache_slot) /* {{{ */
{
	crex_ffi_use_after_free();
}
/* }}} */

static HashTable *crex_ffi_free_get_debug_info(crex_object *obj, int *is_temp) /* {{{ */
{
	crex_ffi_use_after_free();
	return NULL;
}
/* }}} */

static CREX_INI_MH(OnUpdateFFIEnable) /* {{{ */
{
	if (crex_string_equals_literal_ci(new_value, "preload")) {
		FFI_G(restriction) = CREX_FFI_PRELOAD;
	} else {
		FFI_G(restriction) = (crex_ffi_api_restriction)crex_ini_parse_bool(new_value);
	}
	return SUCCESS;
}
/* }}} */

static CREX_INI_DISP(crex_ffi_enable_displayer_cb) /* {{{ */
{
	if (FFI_G(restriction) == CREX_FFI_PRELOAD) {
		CREX_PUTS("preload");
	} else if (FFI_G(restriction) == CREX_FFI_ENABLED) {
		CREX_PUTS("On");
	} else {
		CREX_PUTS("Off");
	}
}
/* }}} */

CREX_INI_BEGIN()
	CREX_INI_ENTRY_EX("ffi.enable", "preload", CREX_INI_SYSTEM, OnUpdateFFIEnable, crex_ffi_enable_displayer_cb)
	STD_CREX_INI_ENTRY("ffi.preload", NULL, CREX_INI_SYSTEM, OnUpdateString, preload, crex_ffi_globals, ffi_globals)
CREX_INI_END()

static crex_result crex_ffi_preload_glob(const char *filename) /* {{{ */
{
#ifdef HAVE_GLOB
	glob_t globbuf;
	int    ret;
	unsigned int i;

	memset(&globbuf, 0, sizeof(glob_t));

	ret = glob(filename, 0, NULL, &globbuf);
#ifdef GLOB_NOMATCH
	if (ret == GLOB_NOMATCH || !globbuf.gl_pathc) {
#else
	if (!globbuf.gl_pathc) {
#endif
		/* pass */
	} else {
		for(i=0 ; i<globbuf.gl_pathc; i++) {
			crex_ffi *ffi = crex_ffi_load(globbuf.gl_pathv[i], 1);
			if (!ffi) {
				globfree(&globbuf);
				return FAILURE;
			}
			efree(ffi);
		}
		globfree(&globbuf);
	}
#else
	crex_ffi *ffi = crex_ffi_load(filename, 1);
	if (!ffi) {
		return FAILURE;
	}
	efree(ffi);
#endif

	return SUCCESS;
}
/* }}} */

static crex_result crex_ffi_preload(char *preload) /* {{{ */
{
	crex_ffi *ffi;
	char *s = NULL, *e, *filename;
	bool is_glob = 0;

	e = preload;
	while (*e) {
		switch (*e) {
			case CREX_PATHS_SEPARATOR:
				if (s) {
					filename = estrndup(s, e-s);
					s = NULL;
					if (!is_glob) {
						ffi = crex_ffi_load(filename, 1);
						efree(filename);
						if (!ffi) {
							return FAILURE;
						}
						efree(ffi);
					} else {
						crex_result ret = crex_ffi_preload_glob(filename);

						efree(filename);
						if (ret == FAILURE) {
							return FAILURE;
						}
						is_glob = 0;
					}
				}
				break;
			case '*':
			case '?':
			case '[':
				is_glob = 1;
				break;
			default:
				if (!s) {
					s = e;
				}
				break;
		}
		e++;
	}
	if (s) {
		filename = estrndup(s, e-s);
		if (!is_glob) {
			ffi = crex_ffi_load(filename, 1);
			efree(filename);
			if (!ffi) {
				return FAILURE;
			}
			efree(ffi);
		} else {
			crex_result ret = crex_ffi_preload_glob(filename);
			efree(filename);
			if (ret == FAILURE) {
				return FAILURE;
			}
		}
	}

	return SUCCESS;
}
/* }}} */

/* {{{ CREX_MINIT_FUNCTION */
CREX_MINIT_FUNCTION(ffi)
{
	REGISTER_INI_ENTRIES();

	FFI_G(is_cli) = strcmp(sapi_module.name, "cli") == 0;

	crex_ffi_exception_ce = register_class_FFI_Exception(crex_ce_error);

	crex_ffi_parser_exception_ce = register_class_FFI_ParserException(crex_ffi_exception_ce);

	crex_ffi_ce = register_class_FFI();
	crex_ffi_ce->create_object = crex_ffi_new;
	crex_ffi_ce->default_object_handlers = &crex_ffi_handlers;

	memcpy(&crex_ffi_new_fn, crex_hash_str_find_ptr(&crex_ffi_ce->function_table, "new", sizeof("new")-1), sizeof(crex_internal_function));
	crex_ffi_new_fn.fn_flags &= ~CREX_ACC_STATIC;
	memcpy(&crex_ffi_cast_fn, crex_hash_str_find_ptr(&crex_ffi_ce->function_table, "cast", sizeof("cast")-1), sizeof(crex_internal_function));
	crex_ffi_cast_fn.fn_flags &= ~CREX_ACC_STATIC;
	memcpy(&crex_ffi_type_fn, crex_hash_str_find_ptr(&crex_ffi_ce->function_table, "type", sizeof("type")-1), sizeof(crex_internal_function));
	crex_ffi_type_fn.fn_flags &= ~CREX_ACC_STATIC;

	memcpy(&crex_ffi_handlers, crex_get_std_object_handlers(), sizeof(crex_object_handlers));
	crex_ffi_handlers.get_constructor      = crex_fake_get_constructor;
	crex_ffi_handlers.free_obj             = crex_ffi_free_obj;
	crex_ffi_handlers.clone_obj            = NULL;
	crex_ffi_handlers.read_property        = crex_ffi_read_var;
	crex_ffi_handlers.write_property       = crex_ffi_write_var;
	crex_ffi_handlers.read_dimension       = crex_fake_read_dimension;
	crex_ffi_handlers.write_dimension      = crex_fake_write_dimension;
	crex_ffi_handlers.get_property_ptr_ptr = crex_fake_get_property_ptr_ptr;
	crex_ffi_handlers.has_property         = crex_fake_has_property;
	crex_ffi_handlers.unset_property       = crex_fake_unset_property;
	crex_ffi_handlers.has_dimension        = crex_fake_has_dimension;
	crex_ffi_handlers.unset_dimension      = crex_fake_unset_dimension;
	crex_ffi_handlers.get_method           = crex_ffi_get_func;
	crex_ffi_handlers.compare              = NULL;
	crex_ffi_handlers.cast_object          = crex_fake_cast_object;
	crex_ffi_handlers.get_debug_info       = NULL;
	crex_ffi_handlers.get_closure          = NULL;
	crex_ffi_handlers.get_properties       = crex_fake_get_properties;
	crex_ffi_handlers.get_gc               = crex_fake_get_gc;

	crex_ffi_cdata_ce = register_class_FFI_CData();
	crex_ffi_cdata_ce->create_object = crex_ffi_cdata_new;
	crex_ffi_cdata_ce->default_object_handlers = &crex_ffi_cdata_handlers;
	crex_ffi_cdata_ce->get_iterator = crex_ffi_cdata_get_iterator;

	memcpy(&crex_ffi_cdata_handlers, crex_get_std_object_handlers(), sizeof(crex_object_handlers));
	crex_ffi_cdata_handlers.get_constructor      = crex_fake_get_constructor;
	crex_ffi_cdata_handlers.free_obj             = crex_ffi_cdata_free_obj;
	crex_ffi_cdata_handlers.clone_obj            = crex_ffi_cdata_clone_obj;
	crex_ffi_cdata_handlers.read_property        = crex_ffi_cdata_read_field;
	crex_ffi_cdata_handlers.write_property       = crex_ffi_cdata_write_field;
	crex_ffi_cdata_handlers.read_dimension       = crex_ffi_cdata_read_dim;
	crex_ffi_cdata_handlers.write_dimension      = crex_ffi_cdata_write_dim;
	crex_ffi_cdata_handlers.get_property_ptr_ptr = crex_fake_get_property_ptr_ptr;
	crex_ffi_cdata_handlers.has_property         = crex_fake_has_property;
	crex_ffi_cdata_handlers.unset_property       = crex_fake_unset_property;
	crex_ffi_cdata_handlers.has_dimension        = crex_fake_has_dimension;
	crex_ffi_cdata_handlers.unset_dimension      = crex_fake_unset_dimension;
	crex_ffi_cdata_handlers.get_method           = crex_fake_get_method;
	crex_ffi_cdata_handlers.get_class_name       = crex_ffi_cdata_get_class_name;
	crex_ffi_cdata_handlers.do_operation         = crex_ffi_cdata_do_operation;
	crex_ffi_cdata_handlers.compare              = crex_ffi_cdata_compare_objects;
	crex_ffi_cdata_handlers.cast_object          = crex_ffi_cdata_cast_object;
	crex_ffi_cdata_handlers.count_elements       = crex_ffi_cdata_count_elements;
	crex_ffi_cdata_handlers.get_debug_info       = crex_ffi_cdata_get_debug_info;
	crex_ffi_cdata_handlers.get_closure          = crex_ffi_cdata_get_closure;
	crex_ffi_cdata_handlers.get_properties       = crex_fake_get_properties;
	crex_ffi_cdata_handlers.get_gc               = crex_fake_get_gc;

	memcpy(&crex_ffi_cdata_value_handlers, crex_get_std_object_handlers(), sizeof(crex_object_handlers));
	crex_ffi_cdata_value_handlers.get_constructor      = crex_fake_get_constructor;
	crex_ffi_cdata_value_handlers.free_obj             = crex_ffi_cdata_free_obj;
	crex_ffi_cdata_value_handlers.clone_obj            = crex_ffi_cdata_clone_obj;
	crex_ffi_cdata_value_handlers.read_property        = crex_ffi_cdata_get;
	crex_ffi_cdata_value_handlers.write_property       = crex_ffi_cdata_set;
	crex_ffi_cdata_value_handlers.read_dimension       = crex_fake_read_dimension;
	crex_ffi_cdata_value_handlers.write_dimension      = crex_fake_write_dimension;
	crex_ffi_cdata_value_handlers.get_property_ptr_ptr = crex_fake_get_property_ptr_ptr;
	crex_ffi_cdata_value_handlers.has_property         = crex_fake_has_property;
	crex_ffi_cdata_value_handlers.unset_property       = crex_fake_unset_property;
	crex_ffi_cdata_value_handlers.has_dimension        = crex_fake_has_dimension;
	crex_ffi_cdata_value_handlers.unset_dimension      = crex_fake_unset_dimension;
	crex_ffi_cdata_value_handlers.get_method           = crex_fake_get_method;
	crex_ffi_cdata_value_handlers.get_class_name       = crex_ffi_cdata_get_class_name;
	crex_ffi_cdata_value_handlers.compare              = crex_ffi_cdata_compare_objects;
	crex_ffi_cdata_value_handlers.cast_object          = crex_ffi_cdata_cast_object;
	crex_ffi_cdata_value_handlers.count_elements       = NULL;
	crex_ffi_cdata_value_handlers.get_debug_info       = crex_ffi_cdata_get_debug_info;
	crex_ffi_cdata_value_handlers.get_closure          = NULL;
	crex_ffi_cdata_value_handlers.get_properties       = crex_fake_get_properties;
	crex_ffi_cdata_value_handlers.get_gc               = crex_fake_get_gc;

	memcpy(&crex_ffi_cdata_free_handlers, crex_get_std_object_handlers(), sizeof(crex_object_handlers));
	crex_ffi_cdata_free_handlers.get_constructor      = crex_fake_get_constructor;
	crex_ffi_cdata_free_handlers.free_obj             = crex_ffi_cdata_free_obj;
	crex_ffi_cdata_free_handlers.clone_obj            = crex_ffi_free_clone_obj;
	crex_ffi_cdata_free_handlers.read_property        = crex_ffi_free_read_property;
	crex_ffi_cdata_free_handlers.write_property       = crex_ffi_free_write_property;
	crex_ffi_cdata_free_handlers.read_dimension       = crex_ffi_free_read_dimension;
	crex_ffi_cdata_free_handlers.write_dimension      = crex_ffi_free_write_dimension;
	crex_ffi_cdata_free_handlers.get_property_ptr_ptr = crex_fake_get_property_ptr_ptr;
	crex_ffi_cdata_free_handlers.has_property         = crex_ffi_free_has_property;
	crex_ffi_cdata_free_handlers.unset_property       = crex_ffi_free_unset_property;
	crex_ffi_cdata_free_handlers.has_dimension        = crex_ffi_free_has_dimension;
	crex_ffi_cdata_free_handlers.unset_dimension      = crex_ffi_free_unset_dimension;
	crex_ffi_cdata_free_handlers.get_method           = crex_fake_get_method;
	crex_ffi_cdata_free_handlers.get_class_name       = crex_ffi_cdata_get_class_name;
	crex_ffi_cdata_free_handlers.compare              = crex_ffi_cdata_compare_objects;
	crex_ffi_cdata_free_handlers.cast_object          = crex_fake_cast_object;
	crex_ffi_cdata_free_handlers.count_elements       = NULL;
	crex_ffi_cdata_free_handlers.get_debug_info       = crex_ffi_free_get_debug_info;
	crex_ffi_cdata_free_handlers.get_closure          = NULL;
	crex_ffi_cdata_free_handlers.get_properties       = crex_fake_get_properties;
	crex_ffi_cdata_free_handlers.get_gc               = crex_fake_get_gc;

	crex_ffi_ctype_ce = register_class_FFI_CType();
	crex_ffi_ctype_ce->create_object = crex_ffi_ctype_new;
	crex_ffi_ctype_ce->default_object_handlers = &crex_ffi_ctype_handlers;

	memcpy(&crex_ffi_ctype_handlers, crex_get_std_object_handlers(), sizeof(crex_object_handlers));
	crex_ffi_ctype_handlers.get_constructor      = crex_fake_get_constructor;
	crex_ffi_ctype_handlers.free_obj             = crex_ffi_ctype_free_obj;
	crex_ffi_ctype_handlers.clone_obj            = NULL;
	crex_ffi_ctype_handlers.read_property        = crex_fake_read_property;
	crex_ffi_ctype_handlers.write_property       = crex_fake_write_property;
	crex_ffi_ctype_handlers.read_dimension       = crex_fake_read_dimension;
	crex_ffi_ctype_handlers.write_dimension      = crex_fake_write_dimension;
	crex_ffi_ctype_handlers.get_property_ptr_ptr = crex_fake_get_property_ptr_ptr;
	crex_ffi_ctype_handlers.has_property         = crex_fake_has_property;
	crex_ffi_ctype_handlers.unset_property       = crex_fake_unset_property;
	crex_ffi_ctype_handlers.has_dimension        = crex_fake_has_dimension;
	crex_ffi_ctype_handlers.unset_dimension      = crex_fake_unset_dimension;
	//crex_ffi_ctype_handlers.get_method           = crex_fake_get_method;
	crex_ffi_ctype_handlers.get_class_name       = crex_ffi_ctype_get_class_name;
	crex_ffi_ctype_handlers.compare              = crex_ffi_ctype_compare_objects;
	crex_ffi_ctype_handlers.cast_object          = crex_fake_cast_object;
	crex_ffi_ctype_handlers.count_elements       = NULL;
	crex_ffi_ctype_handlers.get_debug_info       = crex_ffi_ctype_get_debug_info;
	crex_ffi_ctype_handlers.get_closure          = NULL;
	crex_ffi_ctype_handlers.get_properties       = crex_fake_get_properties;
	crex_ffi_ctype_handlers.get_gc               = crex_fake_get_gc;

	if (FFI_G(preload)) {
		return crex_ffi_preload(FFI_G(preload));
	}

	return SUCCESS;
}
/* }}} */

/* {{{ CREX_RSHUTDOWN_FUNCTION */
CREX_RSHUTDOWN_FUNCTION(ffi)
{
	if (FFI_G(callbacks)) {
		crex_hash_destroy(FFI_G(callbacks));
		efree(FFI_G(callbacks));
		FFI_G(callbacks) = NULL;
	}
	if (FFI_G(weak_types)) {
#if 0
		fprintf(stderr, "WeakTypes: %d\n", crex_hash_num_elements(FFI_G(weak_types)));
#endif
		crex_hash_destroy(FFI_G(weak_types));
		efree(FFI_G(weak_types));
		FFI_G(weak_types) = NULL;
	}
	return SUCCESS;
}
/* }}} */

/* {{{ CREX_MINFO_FUNCTION */
CREX_MINFO_FUNCTION(ffi)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "FFI support", "enabled");
	crx_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

static const crex_ffi_type crex_ffi_type_void = {.kind=CREX_FFI_TYPE_VOID, .size=1, .align=1};
static const crex_ffi_type crex_ffi_type_char = {.kind=CREX_FFI_TYPE_CHAR, .size=1, .align=_Alignof(char)};
static const crex_ffi_type crex_ffi_type_bool = {.kind=CREX_FFI_TYPE_BOOL, .size=1, .align=_Alignof(uint8_t)};
static const crex_ffi_type crex_ffi_type_sint8 = {.kind=CREX_FFI_TYPE_SINT8, .size=1, .align=_Alignof(int8_t)};
static const crex_ffi_type crex_ffi_type_uint8 = {.kind=CREX_FFI_TYPE_UINT8, .size=1, .align=_Alignof(uint8_t)};
static const crex_ffi_type crex_ffi_type_sint16 = {.kind=CREX_FFI_TYPE_SINT16, .size=2, .align=_Alignof(int16_t)};
static const crex_ffi_type crex_ffi_type_uint16 = {.kind=CREX_FFI_TYPE_UINT16, .size=2, .align=_Alignof(uint16_t)};
static const crex_ffi_type crex_ffi_type_sint32 = {.kind=CREX_FFI_TYPE_SINT32, .size=4, .align=_Alignof(int32_t)};
static const crex_ffi_type crex_ffi_type_uint32 = {.kind=CREX_FFI_TYPE_UINT32, .size=4, .align=_Alignof(uint32_t)};
static const crex_ffi_type crex_ffi_type_sint64 = {.kind=CREX_FFI_TYPE_SINT64, .size=8, .align=_Alignof(int64_t)};
static const crex_ffi_type crex_ffi_type_uint64 = {.kind=CREX_FFI_TYPE_UINT64, .size=8, .align=_Alignof(uint64_t)};
static const crex_ffi_type crex_ffi_type_float = {.kind=CREX_FFI_TYPE_FLOAT, .size=sizeof(float), .align=_Alignof(float)};
static const crex_ffi_type crex_ffi_type_double = {.kind=CREX_FFI_TYPE_DOUBLE, .size=sizeof(double), .align=_Alignof(double)};

#ifdef HAVE_LONG_DOUBLE
static const crex_ffi_type crex_ffi_type_long_double = {.kind=CREX_FFI_TYPE_LONGDOUBLE, .size=sizeof(long double), .align=_Alignof(long double)};
#endif

static const crex_ffi_type crex_ffi_type_ptr = {.kind=CREX_FFI_TYPE_POINTER, .size=sizeof(void*), .align=_Alignof(void*), .pointer.type = (crex_ffi_type*)&crex_ffi_type_void};

static const struct {
	const char *name;
	const crex_ffi_type *type;
} crex_ffi_types[] = {
	{"void",        &crex_ffi_type_void},
	{"char",        &crex_ffi_type_char},
	{"bool",        &crex_ffi_type_bool},
	{"int8_t",      &crex_ffi_type_sint8},
	{"uint8_t",     &crex_ffi_type_uint8},
	{"int16_t",     &crex_ffi_type_sint16},
	{"uint16_t",    &crex_ffi_type_uint16},
	{"int32_t",     &crex_ffi_type_sint32},
	{"uint32_t",    &crex_ffi_type_uint32},
	{"int64_t",     &crex_ffi_type_sint64},
	{"uint64_t",    &crex_ffi_type_uint64},
	{"float",       &crex_ffi_type_float},
	{"double",      &crex_ffi_type_double},
#ifdef HAVE_LONG_DOUBLE
	{"long double", &crex_ffi_type_long_double},
#endif
#if SIZEOF_SIZE_T == 4
	{"uintptr_t",  &crex_ffi_type_uint32},
	{"intptr_t",   &crex_ffi_type_sint32},
	{"size_t",     &crex_ffi_type_uint32},
	{"ssize_t",    &crex_ffi_type_sint32},
	{"ptrdiff_t",  &crex_ffi_type_sint32},
#else
	{"uintptr_t",  &crex_ffi_type_uint64},
	{"intptr_t",   &crex_ffi_type_sint64},
	{"size_t",     &crex_ffi_type_uint64},
	{"ssize_t",    &crex_ffi_type_sint64},
	{"ptrdiff_t",  &crex_ffi_type_sint64},
#endif
#if SIZEOF_OFF_T == 4
	{"off_t",      &crex_ffi_type_sint32},
#else
	{"off_t",      &crex_ffi_type_sint64},
#endif

	{"va_list",           &crex_ffi_type_ptr},
	{"__builtin_va_list", &crex_ffi_type_ptr},
	{"__gnuc_va_list",    &crex_ffi_type_ptr},
};

/* {{{ CREX_GINIT_FUNCTION */
static CREX_GINIT_FUNCTION(ffi)
{
	size_t i;

#if defined(COMPILE_DL_FFI) && defined(ZTS)
	CREX_TSRMLS_CACHE_UPDATE();
#endif
	memset(ffi_globals, 0, sizeof(*ffi_globals));
	crex_hash_init(&ffi_globals->types, 0, NULL, NULL, 1);
	for (i = 0; i < sizeof(crex_ffi_types)/sizeof(crex_ffi_types[0]); i++) {
		crex_hash_str_add_new_ptr(&ffi_globals->types, crex_ffi_types[i].name, strlen(crex_ffi_types[i].name), (void*)crex_ffi_types[i].type);
	}
}
/* }}} */

/* {{{ CREX_GINIT_FUNCTION */
static CREX_GSHUTDOWN_FUNCTION(ffi)
{
	if (ffi_globals->scopes) {
		crex_hash_destroy(ffi_globals->scopes);
		free(ffi_globals->scopes);
	}
	crex_hash_destroy(&ffi_globals->types);
}
/* }}} */

/* {{{ ffi_module_entry */
crex_module_entry ffi_module_entry = {
	STANDARD_MODULE_HEADER,
	"FFI",					/* Extension name */
	NULL,					/* crex_function_entry */
	CREX_MINIT(ffi),		/* CREX_MINIT - Module initialization */
	NULL,					/* CREX_MSHUTDOWN - Module shutdown */
	NULL,					/* CREX_RINIT - Request initialization */
	CREX_RSHUTDOWN(ffi),	/* CREX_RSHUTDOWN - Request shutdown */
	CREX_MINFO(ffi),		/* CREX_MINFO - Module info */
	CRX_VERSION,			/* Version */
	CREX_MODULE_GLOBALS(ffi),
	CREX_GINIT(ffi),
	CREX_GSHUTDOWN(ffi),
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_FFI
# ifdef ZTS
CREX_TSRMLS_CACHE_DEFINE()
# endif
CREX_GET_MODULE(ffi)
#endif

/* parser callbacks */
void crex_ffi_parser_error(const char *format, ...) /* {{{ */
{
	va_list va;
	char *message = NULL;

	va_start(va, format);
	crex_vspprintf(&message, 0, format, va);

	if (EG(current_execute_data)) {
		crex_throw_exception(crex_ffi_parser_exception_ce, message, 0);
	} else {
		crex_error(E_WARNING, "FFI Parser: %s", message);
	}

	efree(message);
	va_end(va);

	LONGJMP(FFI_G(bailout), FAILURE);
}
/* }}} */

static void crex_ffi_finalize_type(crex_ffi_dcl *dcl) /* {{{ */
{
	if (!dcl->type) {
		switch (dcl->flags & CREX_FFI_DCL_TYPE_SPECIFIERS) {
			case CREX_FFI_DCL_VOID:
				dcl->type = (crex_ffi_type*)&crex_ffi_type_void;
				break;
			case CREX_FFI_DCL_CHAR:
				dcl->type = (crex_ffi_type*)&crex_ffi_type_char;
				break;
			case CREX_FFI_DCL_CHAR|CREX_FFI_DCL_SIGNED:
				dcl->type = (crex_ffi_type*)&crex_ffi_type_sint8;
				break;
			case CREX_FFI_DCL_CHAR|CREX_FFI_DCL_UNSIGNED:
			case CREX_FFI_DCL_BOOL:
				dcl->type = (crex_ffi_type*)&crex_ffi_type_uint8;
				break;
			case CREX_FFI_DCL_SHORT:
			case CREX_FFI_DCL_SHORT|CREX_FFI_DCL_SIGNED:
			case CREX_FFI_DCL_SHORT|CREX_FFI_DCL_INT:
			case CREX_FFI_DCL_SHORT|CREX_FFI_DCL_SIGNED|CREX_FFI_DCL_INT:
				dcl->type = (crex_ffi_type*)&crex_ffi_type_sint16;
				break;
			case CREX_FFI_DCL_SHORT|CREX_FFI_DCL_UNSIGNED:
			case CREX_FFI_DCL_SHORT|CREX_FFI_DCL_UNSIGNED|CREX_FFI_DCL_INT:
				dcl->type = (crex_ffi_type*)&crex_ffi_type_uint16;
				break;
			case CREX_FFI_DCL_INT:
			case CREX_FFI_DCL_SIGNED:
			case CREX_FFI_DCL_SIGNED|CREX_FFI_DCL_INT:
				dcl->type = (crex_ffi_type*)&crex_ffi_type_sint32;
				break;
			case CREX_FFI_DCL_UNSIGNED:
			case CREX_FFI_DCL_UNSIGNED|CREX_FFI_DCL_INT:
				dcl->type = (crex_ffi_type*)&crex_ffi_type_uint32;
				break;
			case CREX_FFI_DCL_LONG:
			case CREX_FFI_DCL_LONG|CREX_FFI_DCL_SIGNED:
			case CREX_FFI_DCL_LONG|CREX_FFI_DCL_INT:
			case CREX_FFI_DCL_LONG|CREX_FFI_DCL_SIGNED|CREX_FFI_DCL_INT:
				if (sizeof(long) == 4) {
					dcl->type = (crex_ffi_type*)&crex_ffi_type_sint32;
				} else {
					dcl->type = (crex_ffi_type*)&crex_ffi_type_sint64;
				}
				break;
			case CREX_FFI_DCL_LONG|CREX_FFI_DCL_UNSIGNED:
			case CREX_FFI_DCL_LONG|CREX_FFI_DCL_UNSIGNED|CREX_FFI_DCL_INT:
				if (sizeof(long) == 4) {
					dcl->type = (crex_ffi_type*)&crex_ffi_type_uint32;
				} else {
					dcl->type = (crex_ffi_type*)&crex_ffi_type_uint64;
				}
				break;
			case CREX_FFI_DCL_LONG_LONG|CREX_FFI_DCL_LONG:
			case CREX_FFI_DCL_LONG_LONG|CREX_FFI_DCL_LONG|CREX_FFI_DCL_SIGNED:
			case CREX_FFI_DCL_LONG_LONG|CREX_FFI_DCL_LONG|CREX_FFI_DCL_INT:
			case CREX_FFI_DCL_LONG_LONG|CREX_FFI_DCL_LONG|CREX_FFI_DCL_SIGNED|CREX_FFI_DCL_INT:
				dcl->type = (crex_ffi_type*)&crex_ffi_type_sint64;
				break;
			case CREX_FFI_DCL_LONG_LONG|CREX_FFI_DCL_LONG|CREX_FFI_DCL_UNSIGNED:
			case CREX_FFI_DCL_LONG_LONG|CREX_FFI_DCL_LONG|CREX_FFI_DCL_UNSIGNED|CREX_FFI_DCL_INT:
				dcl->type = (crex_ffi_type*)&crex_ffi_type_uint64;
				break;
			case CREX_FFI_DCL_FLOAT:
				dcl->type = (crex_ffi_type*)&crex_ffi_type_float;
				break;
			case CREX_FFI_DCL_DOUBLE:
				dcl->type = (crex_ffi_type*)&crex_ffi_type_double;
				break;
			case CREX_FFI_DCL_LONG|CREX_FFI_DCL_DOUBLE:
#ifdef _WIN32
				dcl->type = (crex_ffi_type*)&crex_ffi_type_double;
#else
				dcl->type = (crex_ffi_type*)&crex_ffi_type_long_double;
#endif
				break;
			case CREX_FFI_DCL_FLOAT|CREX_FFI_DCL_COMPLEX:
			case CREX_FFI_DCL_DOUBLE|CREX_FFI_DCL_COMPLEX:
			case CREX_FFI_DCL_DOUBLE|CREX_FFI_DCL_LONG|CREX_FFI_DCL_COMPLEX:
				crex_ffi_parser_error("Unsupported type _Complex at line %d", FFI_G(line));
				break;
			default:
				crex_ffi_parser_error("Unsupported type specifier combination at line %d", FFI_G(line));
				break;
		}
		dcl->flags &= ~CREX_FFI_DCL_TYPE_SPECIFIERS;
		dcl->flags |= CREX_FFI_DCL_TYPEDEF_NAME;
	}
}
/* }}} */

bool crex_ffi_is_typedef_name(const char *name, size_t name_len) /* {{{ */
{
	crex_ffi_symbol *sym;
	crex_ffi_type *type;

	if (FFI_G(symbols)) {
		sym = crex_hash_str_find_ptr(FFI_G(symbols), name, name_len);
		if (sym) {
			return (sym->kind == CREX_FFI_SYM_TYPE);
		}
	}
	type = crex_hash_str_find_ptr(&FFI_G(types), name, name_len);
	if (type) {
		return 1;
	}
	return 0;
}
/* }}} */

void crex_ffi_resolve_typedef(const char *name, size_t name_len, crex_ffi_dcl *dcl) /* {{{ */
{
	crex_ffi_symbol *sym;
	crex_ffi_type *type;

	if (FFI_G(symbols)) {
		sym = crex_hash_str_find_ptr(FFI_G(symbols), name, name_len);
		if (sym && sym->kind == CREX_FFI_SYM_TYPE) {
			dcl->type = CREX_FFI_TYPE(sym->type);
			if (sym->is_const) {
				dcl->attr |= CREX_FFI_ATTR_CONST;
			}
			return;
		}
	}
	type = crex_hash_str_find_ptr(&FFI_G(types), name, name_len);
	if (type) {
		dcl->type = type;
		return;
	}
	crex_ffi_parser_error("Undefined C type \"%.*s\" at line %d", name_len, name, FFI_G(line));
}
/* }}} */

void crex_ffi_resolve_const(const char *name, size_t name_len, crex_ffi_val *val) /* {{{ */
{
	crex_ffi_symbol *sym;

	if (UNEXPECTED(FFI_G(attribute_parsing))) {
		val->kind = CREX_FFI_VAL_NAME;
		val->str = name;
		val->len = name_len;
		return;
	} else if (FFI_G(symbols)) {
		sym = crex_hash_str_find_ptr(FFI_G(symbols), name, name_len);
		if (sym && sym->kind == CREX_FFI_SYM_CONST) {
			val->i64 = sym->value;
			switch (sym->type->kind) {
				case CREX_FFI_TYPE_SINT8:
				case CREX_FFI_TYPE_SINT16:
				case CREX_FFI_TYPE_SINT32:
					val->kind = CREX_FFI_VAL_INT32;
					break;
				case CREX_FFI_TYPE_SINT64:
					val->kind = CREX_FFI_VAL_INT64;
					break;
				case CREX_FFI_TYPE_UINT8:
				case CREX_FFI_TYPE_UINT16:
				case CREX_FFI_TYPE_UINT32:
					val->kind = CREX_FFI_VAL_UINT32;
					break;
				case CREX_FFI_TYPE_UINT64:
					val->kind = CREX_FFI_VAL_UINT64;
					break;
				default:
					CREX_UNREACHABLE();
			}
			return;
		}
	}
	val->kind = CREX_FFI_VAL_ERROR;
}
/* }}} */

void crex_ffi_make_enum_type(crex_ffi_dcl *dcl) /* {{{ */
{
	crex_ffi_type *type = pemalloc(sizeof(crex_ffi_type), FFI_G(persistent));
	type->kind = CREX_FFI_TYPE_ENUM;
	type->attr = FFI_G(default_type_attr) | (dcl->attr & CREX_FFI_ENUM_ATTRS);
	type->enumeration.tag_name = NULL;
	if (type->attr & CREX_FFI_ATTR_PACKED) {
		type->size = crex_ffi_type_uint8.size;
		type->align = crex_ffi_type_uint8.align;
		type->enumeration.kind = CREX_FFI_TYPE_UINT8;
	} else {
		type->size = crex_ffi_type_uint32.size;
		type->align = crex_ffi_type_uint32.align;
		type->enumeration.kind = CREX_FFI_TYPE_UINT32;
	}
	dcl->type = CREX_FFI_TYPE_MAKE_OWNED(type);
	dcl->attr &= ~CREX_FFI_ENUM_ATTRS;
}
/* }}} */

void crex_ffi_add_enum_val(crex_ffi_dcl *enum_dcl, const char *name, size_t name_len, crex_ffi_val *val, int64_t *min, int64_t *max, int64_t *last) /* {{{ */
{
	crex_ffi_symbol *sym;
	const crex_ffi_type *sym_type;
	int64_t value;
	crex_ffi_type *enum_type = CREX_FFI_TYPE(enum_dcl->type);
	bool overflow = 0;
	bool is_signed =
		(enum_type->enumeration.kind == CREX_FFI_TYPE_SINT8 ||
		 enum_type->enumeration.kind == CREX_FFI_TYPE_SINT16 ||
		 enum_type->enumeration.kind == CREX_FFI_TYPE_SINT32 ||
		 enum_type->enumeration.kind == CREX_FFI_TYPE_SINT64);

	CREX_ASSERT(enum_type && enum_type->kind == CREX_FFI_TYPE_ENUM);
	if (val->kind == CREX_FFI_VAL_EMPTY) {
		if (is_signed) {
			if (*last == 0x7FFFFFFFFFFFFFFFLL) {
				overflow = 1;
			}
		} else {
			if ((*min != 0 || *max != 0)
			 && (uint64_t)*last == 0xFFFFFFFFFFFFFFFFULL) {
				overflow = 1;
			}
		}
		value = *last + 1;
	} else if (val->kind == CREX_FFI_VAL_CHAR) {
		if (!is_signed && val->ch < 0) {
			if ((uint64_t)*max > 0x7FFFFFFFFFFFFFFFULL) {
				overflow = 1;
			} else {
				is_signed = 1;
			}
		}
		value = val->ch;
	} else if (val->kind == CREX_FFI_VAL_INT32 || val->kind == CREX_FFI_VAL_INT64) {
		if (!is_signed && val->i64 < 0) {
			if ((uint64_t)*max > 0x7FFFFFFFFFFFFFFFULL) {
				overflow = 1;
			} else {
				is_signed = 1;
			}
		}
		value = val->i64;
	} else if (val->kind == CREX_FFI_VAL_UINT32 || val->kind == CREX_FFI_VAL_UINT64) {
		if (is_signed && val->u64 > 0x7FFFFFFFFFFFFFFFULL) {
			overflow = 1;
		}
		value = val->u64;
	} else {
		crex_ffi_parser_error("Enumerator value \"%.*s\" must be an integer at line %d", name_len, name, FFI_G(line));
		return;
	}

	if (overflow) {
		crex_ffi_parser_error("Overflow in enumeration values \"%.*s\" at line %d", name_len, name, FFI_G(line));
		return;
	}

	if (is_signed) {
		*min = MIN(*min, value);
		*max = MAX(*max, value);
		if ((enum_type->attr & CREX_FFI_ATTR_PACKED)
		 && *min >= -0x7FLL-1 && *max <= 0x7FLL) {
			sym_type = &crex_ffi_type_sint8;
		} else if ((enum_type->attr & CREX_FFI_ATTR_PACKED)
		 && *min >= -0x7FFFLL-1 && *max <= 0x7FFFLL) {
			sym_type = &crex_ffi_type_sint16;
		} else if (*min >= -0x7FFFFFFFLL-1 && *max <= 0x7FFFFFFFLL) {
			sym_type = &crex_ffi_type_sint32;
		} else {
			sym_type = &crex_ffi_type_sint64;
		}
	} else {
		*min = MIN((uint64_t)*min, (uint64_t)value);
		*max = MAX((uint64_t)*max, (uint64_t)value);
		if ((enum_type->attr & CREX_FFI_ATTR_PACKED)
		 && (uint64_t)*max <= 0xFFULL) {
			sym_type = &crex_ffi_type_uint8;
		} else if ((enum_type->attr & CREX_FFI_ATTR_PACKED)
		 && (uint64_t)*max <= 0xFFFFULL) {
			sym_type = &crex_ffi_type_uint16;
		} else if ((uint64_t)*max <= 0xFFFFFFFFULL) {
			sym_type = &crex_ffi_type_uint32;
		} else {
			sym_type = &crex_ffi_type_uint64;
		}
	}
	enum_type->enumeration.kind = sym_type->kind;
	enum_type->size = sym_type->size;
	enum_type->align = sym_type->align;
	*last = value;

	if (!FFI_G(symbols)) {
		FFI_G(symbols) = pemalloc(sizeof(HashTable), FFI_G(persistent));
		crex_hash_init(FFI_G(symbols), 0, NULL, FFI_G(persistent) ? crex_ffi_symbol_hash_persistent_dtor : crex_ffi_symbol_hash_dtor, FFI_G(persistent));
	}
	sym = crex_hash_str_find_ptr(FFI_G(symbols), name, name_len);
	if (sym) {
		crex_ffi_parser_error("Redeclaration of \"%.*s\" at line %d", name_len, name, FFI_G(line));
	} else {
		sym = pemalloc(sizeof(crex_ffi_symbol), FFI_G(persistent));
		sym->kind  = CREX_FFI_SYM_CONST;
		sym->type  = (crex_ffi_type*)sym_type;
		sym->value = value;
		crex_hash_str_add_new_ptr(FFI_G(symbols), name, name_len, sym);
	}
}
/* }}} */

void crex_ffi_make_struct_type(crex_ffi_dcl *dcl) /* {{{ */
{
	crex_ffi_type *type = pemalloc(sizeof(crex_ffi_type), FFI_G(persistent));
	type->kind = CREX_FFI_TYPE_STRUCT;
	type->attr = FFI_G(default_type_attr) | (dcl->attr & CREX_FFI_STRUCT_ATTRS);
	type->size = 0;
	type->align = dcl->align > 1 ? dcl->align : 1;
	if (dcl->flags & CREX_FFI_DCL_UNION) {
		type->attr |= CREX_FFI_ATTR_UNION;
	}
	dcl->type = CREX_FFI_TYPE_MAKE_OWNED(type);
	type->record.tag_name = NULL;
	crex_hash_init(&type->record.fields, 0, NULL, FFI_G(persistent) ? crex_ffi_field_hash_persistent_dtor :crex_ffi_field_hash_dtor, FFI_G(persistent));
	dcl->attr &= ~CREX_FFI_STRUCT_ATTRS;
	dcl->align = 0;
}
/* }}} */

static crex_result crex_ffi_validate_prev_field_type(crex_ffi_type *struct_type) /* {{{ */
{
	if (crex_hash_num_elements(&struct_type->record.fields) > 0) {
		crex_ffi_field *field = NULL;

		CREX_HASH_MAP_REVERSE_FOREACH_PTR(&struct_type->record.fields, field) {
			break;
		} CREX_HASH_FOREACH_END();
		if (CREX_FFI_TYPE(field->type)->attr & CREX_FFI_ATTR_INCOMPLETE_ARRAY) {
			crex_ffi_throw_parser_error("Flexible array member not at end of struct at line %d", FFI_G(line));
			return FAILURE;
		}
	}
	return SUCCESS;
}
/* }}} */

static crex_result crex_ffi_validate_field_type(crex_ffi_type *type, crex_ffi_type *struct_type) /* {{{ */
{
	if (type == struct_type) {
		crex_ffi_throw_parser_error("Struct/union can't contain an instance of itself at line %d", FFI_G(line));
		return FAILURE;
	} else if (crex_ffi_validate_var_type(type, 1) == FAILURE) {
		return FAILURE;
	} else if (struct_type->attr & CREX_FFI_ATTR_UNION) {
		if (type->attr & CREX_FFI_ATTR_INCOMPLETE_ARRAY) {
			crex_ffi_throw_parser_error("Flexible array member in union at line %d", FFI_G(line));
			return FAILURE;
		}
	}
	return crex_ffi_validate_prev_field_type(struct_type);
}
/* }}} */

void crex_ffi_add_field(crex_ffi_dcl *struct_dcl, const char *name, size_t name_len, crex_ffi_dcl *field_dcl) /* {{{ */
{
	crex_ffi_field *field;
	crex_ffi_type *struct_type = CREX_FFI_TYPE(struct_dcl->type);
	crex_ffi_type *field_type;

	CREX_ASSERT(struct_type && struct_type->kind == CREX_FFI_TYPE_STRUCT);
	crex_ffi_finalize_type(field_dcl);
	field_type = CREX_FFI_TYPE(field_dcl->type);
	if (crex_ffi_validate_field_type(field_type, struct_type) == FAILURE) {
		crex_ffi_cleanup_dcl(field_dcl);
		LONGJMP(FFI_G(bailout), FAILURE);
	}

	field = pemalloc(sizeof(crex_ffi_field), FFI_G(persistent));
	if (!(struct_type->attr & CREX_FFI_ATTR_PACKED) && !(field_dcl->attr & CREX_FFI_ATTR_PACKED)) {
		struct_type->align = MAX(struct_type->align, MAX(field_type->align, field_dcl->align));
	}
	if (struct_type->attr & CREX_FFI_ATTR_UNION) {
		field->offset = 0;
		struct_type->size = MAX(struct_type->size, field_type->size);
	} else {
		if (!(struct_type->attr & CREX_FFI_ATTR_PACKED) && !(field_dcl->attr & CREX_FFI_ATTR_PACKED)) {
			uint32_t field_align = MAX(field_type->align, field_dcl->align);
			struct_type->size = ((struct_type->size + (field_align - 1)) / field_align) * field_align;
		}
		field->offset = struct_type->size;
		struct_type->size += field_type->size;
	}
	field->type = field_dcl->type;
	field->is_const = (bool)(field_dcl->attr & CREX_FFI_ATTR_CONST);
	field->is_nested = 0;
	field->first_bit = 0;
	field->bits = 0;
	field_dcl->type = field_type; /* reset "owned" flag */

	if (!crex_hash_str_add_ptr(&struct_type->record.fields, name, name_len, field)) {
		crex_ffi_type_dtor(field->type);
		pefree(field, FFI_G(persistent));
		crex_ffi_parser_error("Duplicate field name \"%.*s\" at line %d", name_len, name, FFI_G(line));
	}
}
/* }}} */

void crex_ffi_add_anonymous_field(crex_ffi_dcl *struct_dcl, crex_ffi_dcl *field_dcl) /* {{{ */
{
	crex_ffi_type *struct_type = CREX_FFI_TYPE(struct_dcl->type);
	crex_ffi_type *field_type;
	crex_ffi_field *field;
	crex_string *key;

	CREX_ASSERT(struct_type && struct_type->kind == CREX_FFI_TYPE_STRUCT);
	crex_ffi_finalize_type(field_dcl);
	field_type = CREX_FFI_TYPE(field_dcl->type);
	if (field_type->kind != CREX_FFI_TYPE_STRUCT) {
		crex_ffi_cleanup_dcl(field_dcl);
		crex_ffi_parser_error("Declaration does not declare anything at line %d", FFI_G(line));
		return;
	}

	if (!(struct_type->attr & CREX_FFI_ATTR_PACKED) && !(field_dcl->attr & CREX_FFI_ATTR_PACKED)) {
		struct_type->align = MAX(struct_type->align, MAX(field_type->align, field_dcl->align));
	}
	if (!(struct_type->attr & CREX_FFI_ATTR_UNION)) {
		if (crex_ffi_validate_prev_field_type(struct_type) == FAILURE) {
			crex_ffi_cleanup_dcl(field_dcl);
			LONGJMP(FFI_G(bailout), FAILURE);
		}
		if (!(struct_type->attr & CREX_FFI_ATTR_PACKED) && !(field_dcl->attr & CREX_FFI_ATTR_PACKED)) {
			uint32_t field_align = MAX(field_type->align, field_dcl->align);
			struct_type->size = ((struct_type->size + (field_align - 1)) / field_align) * field_align;
		}
	}

	CREX_HASH_MAP_FOREACH_STR_KEY_PTR(&field_type->record.fields, key, field) {
		crex_ffi_field *new_field = pemalloc(sizeof(crex_ffi_field), FFI_G(persistent));

		if (struct_type->attr & CREX_FFI_ATTR_UNION) {
			new_field->offset = field->offset;
		} else {
			new_field->offset = struct_type->size + field->offset;
		}
		new_field->type = field->type;
		new_field->is_const = field->is_const;
		new_field->is_nested = 1;
		new_field->first_bit = field->first_bit;
		new_field->bits = field->bits;
		field->type = CREX_FFI_TYPE(field->type); /* reset "owned" flag */

		if (key) {
			if (!crex_hash_add_ptr(&struct_type->record.fields, key, new_field)) {
				crex_ffi_type_dtor(new_field->type);
				pefree(new_field, FFI_G(persistent));
				crex_ffi_parser_error("Duplicate field name \"%s\" at line %d", ZSTR_VAL(key), FFI_G(line));
				return;
			}
		} else {
			crex_hash_next_index_insert_ptr(&struct_type->record.fields, field);
		}
	} CREX_HASH_FOREACH_END();

	if (struct_type->attr & CREX_FFI_ATTR_UNION) {
		struct_type->size = MAX(struct_type->size, field_type->size);
	} else {
		struct_type->size += field_type->size;
	}

	crex_ffi_type_dtor(field_dcl->type);
	field_dcl->type = NULL;
}
/* }}} */

void crex_ffi_add_bit_field(crex_ffi_dcl *struct_dcl, const char *name, size_t name_len, crex_ffi_dcl *field_dcl, crex_ffi_val *bits) /* {{{ */
{
	crex_ffi_type *struct_type = CREX_FFI_TYPE(struct_dcl->type);
	crex_ffi_type *field_type;
	crex_ffi_field *field;

	CREX_ASSERT(struct_type && struct_type->kind == CREX_FFI_TYPE_STRUCT);
	crex_ffi_finalize_type(field_dcl);
	field_type = CREX_FFI_TYPE(field_dcl->type);
	if (crex_ffi_validate_field_type(field_type, struct_type) == FAILURE) {
		crex_ffi_cleanup_dcl(field_dcl);
		LONGJMP(FFI_G(bailout), FAILURE);
	}

	if (field_type->kind < CREX_FFI_TYPE_UINT8 || field_type->kind > CREX_FFI_TYPE_BOOL) {
		crex_ffi_cleanup_dcl(field_dcl);
		crex_ffi_parser_error("Wrong type of bit field \"%.*s\" at line %d", name ? name_len : sizeof("<anonymous>")-1, name ? name : "<anonymous>", FFI_G(line));
	}

	if (bits->kind == CREX_FFI_VAL_INT32 || bits->kind == CREX_FFI_VAL_INT64) {
		if (bits->i64 < 0) {
			crex_ffi_cleanup_dcl(field_dcl);
			crex_ffi_parser_error("Negative width in bit-field \"%.*s\" at line %d", name ? name_len : sizeof("<anonymous>")-1, name ? name : "<anonymous>", FFI_G(line));
		} else if (bits->i64 == 0) {
			crex_ffi_cleanup_dcl(field_dcl);
			if (name) {
				crex_ffi_parser_error("Zero width in bit-field \"%.*s\" at line %d", name ? name_len : sizeof("<anonymous>")-1, name ? name : "<anonymous>", FFI_G(line));
			}
			return;
		} else if (bits->i64 > field_type->size * 8) {
			crex_ffi_cleanup_dcl(field_dcl);
			crex_ffi_parser_error("Width of \"%.*s\" exceeds its type at line %d", name ? name_len : sizeof("<anonymous>")-1, name ? name : "<anonymous>", FFI_G(line));
		}
	} else if (bits->kind == CREX_FFI_VAL_UINT32 || bits->kind == CREX_FFI_VAL_UINT64) {
		if (bits->u64 == 0) {
			crex_ffi_cleanup_dcl(field_dcl);
			if (name) {
				crex_ffi_parser_error("Zero width in bit-field \"%.*s\" at line %d", name ? name_len : sizeof("<anonymous>")-1, name ? name : "<anonymous>", FFI_G(line));
			}
			return;
		} else if (bits->u64 > field_type->size * 8) {
			crex_ffi_cleanup_dcl(field_dcl);
			crex_ffi_parser_error("Width of \"%.*s\" exceeds its type at line %d", name ? name_len : sizeof("<anonymous>")-1, name ? name : "<anonymous>", FFI_G(line));
		}
	} else {
		crex_ffi_cleanup_dcl(field_dcl);
		crex_ffi_parser_error("Bit field \"%.*s\" width not an integer constant at line %d", name ? name_len : sizeof("<anonymous>")-1, name ? name : "<anonymous>", FFI_G(line));
	}

	field = pemalloc(sizeof(crex_ffi_field), FFI_G(persistent));
	if (!(struct_type->attr & CREX_FFI_ATTR_PACKED)) {
		struct_type->align = MAX(struct_type->align, sizeof(uint32_t));
	}
	if (struct_type->attr & CREX_FFI_ATTR_UNION) {
		field->offset = 0;
		field->first_bit = 0;
		field->bits = bits->u64;
		if (struct_type->attr & CREX_FFI_ATTR_PACKED) {
			struct_type->size = MAX(struct_type->size, (bits->u64 + 7) / 8);
		} else {
			struct_type->size = MAX(struct_type->size, ((bits->u64 + 31) / 32) * 4);
		}
	} else {
		crex_ffi_field *prev_field = NULL;

		if (crex_hash_num_elements(&struct_type->record.fields) > 0) {
			CREX_HASH_MAP_REVERSE_FOREACH_PTR(&struct_type->record.fields, prev_field) {
				break;
			} CREX_HASH_FOREACH_END();
		}
		if (prev_field && prev_field->bits) {
			field->offset = prev_field->offset;
			field->first_bit = prev_field->first_bit + prev_field->bits;
			field->bits = bits->u64;
		} else {
			field->offset = struct_type->size;
			field->first_bit = 0;
			field->bits = bits->u64;
		}
		if (struct_type->attr & CREX_FFI_ATTR_PACKED) {
			struct_type->size = field->offset + ((field->first_bit + field->bits) + 7) / 8;
		} else {
			struct_type->size = field->offset + (((field->first_bit + field->bits) + 31) / 32) * 4;
		}
	}
	field->type = field_dcl->type;
	field->is_const = (bool)(field_dcl->attr & CREX_FFI_ATTR_CONST);
	field->is_nested = 0;
	field_dcl->type = field_type; /* reset "owned" flag */

	if (name) {
		if (!crex_hash_str_add_ptr(&struct_type->record.fields, name, name_len, field)) {
			crex_ffi_type_dtor(field->type);
			pefree(field, FFI_G(persistent));
			crex_ffi_parser_error("Duplicate field name \"%.*s\" at line %d", name_len, name, FFI_G(line));
		}
	} else {
		crex_hash_next_index_insert_ptr(&struct_type->record.fields, field);
	}
}
/* }}} */

void crex_ffi_adjust_struct_size(crex_ffi_dcl *dcl) /* {{{ */
{
	crex_ffi_type *struct_type = CREX_FFI_TYPE(dcl->type);

	CREX_ASSERT(struct_type->kind == CREX_FFI_TYPE_STRUCT);
	if (dcl->align > struct_type->align) {
		struct_type->align = dcl->align;
	}
	if (!(struct_type->attr & CREX_FFI_ATTR_PACKED)) {
		struct_type->size = ((struct_type->size + (struct_type->align - 1)) / struct_type->align) * struct_type->align;
	}
	dcl->align = 0;
}
/* }}} */

void crex_ffi_make_pointer_type(crex_ffi_dcl *dcl) /* {{{ */
{
	crex_ffi_type *type = pemalloc(sizeof(crex_ffi_type), FFI_G(persistent));
	type->kind = CREX_FFI_TYPE_POINTER;
	type->attr = FFI_G(default_type_attr) | (dcl->attr & CREX_FFI_POINTER_ATTRS);
	type->size = sizeof(void*);
	type->align = _Alignof(void*);
	crex_ffi_finalize_type(dcl);
	if (crex_ffi_validate_vla(CREX_FFI_TYPE(dcl->type)) == FAILURE) {
		crex_ffi_cleanup_dcl(dcl);
		LONGJMP(FFI_G(bailout), FAILURE);
	}
	type->pointer.type = dcl->type;
	dcl->type = CREX_FFI_TYPE_MAKE_OWNED(type);
	dcl->flags &= ~CREX_FFI_DCL_TYPE_QUALIFIERS;
	dcl->attr &= ~CREX_FFI_POINTER_ATTRS;
	dcl->align = 0;
}
/* }}} */

static crex_result crex_ffi_validate_array_element_type(crex_ffi_type *type) /* {{{ */
{
	if (type->kind == CREX_FFI_TYPE_FUNC) {
		crex_ffi_throw_parser_error("Array of functions is not allowed at line %d", FFI_G(line));
		return FAILURE;
	} else if (type->kind == CREX_FFI_TYPE_ARRAY && (type->attr & CREX_FFI_ATTR_INCOMPLETE_ARRAY)) {
		crex_ffi_throw_parser_error("Only the leftmost array can be undimensioned at line %d", FFI_G(line));
		return FAILURE;
	}
	return crex_ffi_validate_type(type, 0, 1);
}
/* }}} */

void crex_ffi_make_array_type(crex_ffi_dcl *dcl, crex_ffi_val *len) /* {{{ */
{
	int length = 0;
	crex_ffi_type *element_type;
	crex_ffi_type *type;

	crex_ffi_finalize_type(dcl);
	element_type = CREX_FFI_TYPE(dcl->type);

	if (len->kind == CREX_FFI_VAL_EMPTY) {
		length = 0;
	} else if (len->kind == CREX_FFI_VAL_UINT32 || len->kind == CREX_FFI_VAL_UINT64) {
		length = len->u64;
	} else if (len->kind == CREX_FFI_VAL_INT32 || len->kind == CREX_FFI_VAL_INT64) {
		length = len->i64;
	} else if (len->kind == CREX_FFI_VAL_CHAR) {
		length = len->ch;
	} else {
		crex_ffi_cleanup_dcl(dcl);
		crex_ffi_parser_error("Unsupported array index type at line %d", FFI_G(line));
		return;
	}
	if (length < 0) {
		crex_ffi_cleanup_dcl(dcl);
		crex_ffi_parser_error("Negative array index at line %d", FFI_G(line));
		return;
	}

	if (crex_ffi_validate_array_element_type(element_type) == FAILURE) {
		crex_ffi_cleanup_dcl(dcl);
		LONGJMP(FFI_G(bailout), FAILURE);
	}

	type = pemalloc(sizeof(crex_ffi_type), FFI_G(persistent));
	type->kind = CREX_FFI_TYPE_ARRAY;
	type->attr = FFI_G(default_type_attr) | (dcl->attr & CREX_FFI_ARRAY_ATTRS);
	type->size = length * element_type->size;
	type->align = element_type->align;
	type->array.type = dcl->type;
	type->array.length = length;
	dcl->type = CREX_FFI_TYPE_MAKE_OWNED(type);
	dcl->flags &= ~CREX_FFI_DCL_TYPE_QUALIFIERS;
	dcl->attr &= ~CREX_FFI_ARRAY_ATTRS;
	dcl->align = 0;
}
/* }}} */

static crex_result crex_ffi_validate_func_ret_type(crex_ffi_type *type) /* {{{ */
{
	if (type->kind == CREX_FFI_TYPE_FUNC) {
		crex_ffi_throw_parser_error("Function returning function is not allowed at line %d", FFI_G(line));
		return FAILURE;
	 } else if (type->kind == CREX_FFI_TYPE_ARRAY) {
		crex_ffi_throw_parser_error("Function returning array is not allowed at line %d", FFI_G(line));
		return FAILURE;
	}
	return crex_ffi_validate_incomplete_type(type, 1, 0);
}
/* }}} */

void crex_ffi_make_func_type(crex_ffi_dcl *dcl, HashTable *args, crex_ffi_dcl *nested_dcl) /* {{{ */
{
	crex_ffi_type *type;
	crex_ffi_type *ret_type;

	crex_ffi_finalize_type(dcl);
	ret_type = CREX_FFI_TYPE(dcl->type);

	if (args) {
		int no_args = 0;
		crex_ffi_type *arg_type;

		CREX_HASH_PACKED_FOREACH_PTR(args, arg_type) {
			arg_type = CREX_FFI_TYPE(arg_type);
			if (arg_type->kind == CREX_FFI_TYPE_VOID) {
				if (crex_hash_num_elements(args) != 1) {
					crex_ffi_cleanup_dcl(nested_dcl);
					crex_ffi_cleanup_dcl(dcl);
					crex_hash_destroy(args);
					pefree(args, FFI_G(persistent));
					crex_ffi_parser_error("void type is not allowed at line %d", FFI_G(line));
					return;
				} else {
					no_args = 1;
				}
			}
		} CREX_HASH_FOREACH_END();
		if (no_args) {
			crex_hash_destroy(args);
			pefree(args, FFI_G(persistent));
			args = NULL;
		}
	}

#ifdef HAVE_FFI_VECTORCALL_PARTIAL
	if (dcl->abi == CREX_FFI_ABI_VECTORCALL && args) {
		crex_ulong i;
		crex_ffi_type *arg_type;

		CREX_HASH_PACKED_FOREACH_KEY_PTR(args, i, arg_type) {
			arg_type = CREX_FFI_TYPE(arg_type);
# ifdef _WIN64
			if (i >= 4 && i <= 5 && (arg_type->kind == CREX_FFI_TYPE_FLOAT || arg_type->kind == CREX_FFI_TYPE_DOUBLE)) {
# else
			if (i < 6 && (arg_type->kind == CREX_FFI_TYPE_FLOAT || arg_type->kind == CREX_FFI_TYPE_DOUBLE)) {
# endif
				crex_ffi_cleanup_dcl(nested_dcl);
				crex_ffi_cleanup_dcl(dcl);
				crex_hash_destroy(args);
				pefree(args, FFI_G(persistent));
				crex_ffi_parser_error("Type float/double is not allowed at position " CREX_ULONG_FMT " with __vectorcall at line %d", i+1, FFI_G(line));
				return;
			}
		} CREX_HASH_FOREACH_END();
	}
#endif

	if (crex_ffi_validate_func_ret_type(ret_type) == FAILURE) {
		crex_ffi_cleanup_dcl(nested_dcl);
		crex_ffi_cleanup_dcl(dcl);
		if (args) {
			crex_hash_destroy(args);
			pefree(args, FFI_G(persistent));
		}
		LONGJMP(FFI_G(bailout), FAILURE);
	}

	type = pemalloc(sizeof(crex_ffi_type), FFI_G(persistent));
	type->kind = CREX_FFI_TYPE_FUNC;
	type->attr = FFI_G(default_type_attr) | (dcl->attr & CREX_FFI_FUNC_ATTRS);
	type->size = sizeof(void*);
	type->align = 1;
	type->func.ret_type = dcl->type;
	switch (dcl->abi) {
		case CREX_FFI_ABI_DEFAULT:
		case CREX_FFI_ABI_CDECL:
			type->func.abi = FFI_DEFAULT_ABI;
			break;
#ifdef HAVE_FFI_FASTCALL
		case CREX_FFI_ABI_FASTCALL:
			type->func.abi = FFI_FASTCALL;
			break;
#endif
#ifdef HAVE_FFI_THISCALL
		case CREX_FFI_ABI_THISCALL:
			type->func.abi = FFI_THISCALL;
			break;
#endif
#ifdef HAVE_FFI_STDCALL
		case CREX_FFI_ABI_STDCALL:
			type->func.abi = FFI_STDCALL;
			break;
#endif
#ifdef HAVE_FFI_PASCAL
		case CREX_FFI_ABI_PASCAL:
			type->func.abi = FFI_PASCAL;
			break;
#endif
#ifdef HAVE_FFI_REGISTER
		case CREX_FFI_ABI_REGISTER:
			type->func.abi = FFI_REGISTER;
			break;
#endif
#ifdef HAVE_FFI_MS_CDECL
		case CREX_FFI_ABI_MS:
			type->func.abi = FFI_MS_CDECL;
			break;
#endif
#ifdef HAVE_FFI_SYSV
		case CREX_FFI_ABI_SYSV:
			type->func.abi = FFI_SYSV;
			break;
#endif
#ifdef HAVE_FFI_VECTORCALL_PARTIAL
		case CREX_FFI_ABI_VECTORCALL:
			type->func.abi = FFI_VECTORCALL_PARTIAL;
			break;
#endif
		default:
			type->func.abi = FFI_DEFAULT_ABI;
			crex_ffi_cleanup_dcl(nested_dcl);
			if (args) {
				crex_hash_destroy(args);
				pefree(args, FFI_G(persistent));
			}
			type->func.args = NULL;
			_crex_ffi_type_dtor(type);
			crex_ffi_parser_error("Unsupported calling convention line %d", FFI_G(line));
			break;
	}
	type->func.args = args;
	dcl->type = CREX_FFI_TYPE_MAKE_OWNED(type);
	dcl->attr &= ~CREX_FFI_FUNC_ATTRS;
	dcl->align = 0;
	dcl->abi = 0;
}
/* }}} */

void crex_ffi_add_arg(HashTable **args, const char *name, size_t name_len, crex_ffi_dcl *arg_dcl) /* {{{ */
{
	crex_ffi_type *type;

	if (!*args) {
		*args = pemalloc(sizeof(HashTable), FFI_G(persistent));
		crex_hash_init(*args, 0, NULL, crex_ffi_type_hash_dtor, FFI_G(persistent));
	}
	crex_ffi_finalize_type(arg_dcl);
	type = CREX_FFI_TYPE(arg_dcl->type);
	if (type->kind == CREX_FFI_TYPE_ARRAY) {
		if (CREX_FFI_TYPE_IS_OWNED(arg_dcl->type)) {
			type->kind = CREX_FFI_TYPE_POINTER;
			type->size = sizeof(void*);
		} else {
			crex_ffi_type *new_type = pemalloc(sizeof(crex_ffi_type), FFI_G(persistent));
			new_type->kind = CREX_FFI_TYPE_POINTER;
			new_type->attr = FFI_G(default_type_attr) | (type->attr & CREX_FFI_POINTER_ATTRS);
			new_type->size = sizeof(void*);
			new_type->align = _Alignof(void*);
			new_type->pointer.type = CREX_FFI_TYPE(type->array.type);
			arg_dcl->type = CREX_FFI_TYPE_MAKE_OWNED(new_type);
		}
	} else if (type->kind == CREX_FFI_TYPE_FUNC) {
		crex_ffi_type *new_type = pemalloc(sizeof(crex_ffi_type), FFI_G(persistent));
		new_type->kind = CREX_FFI_TYPE_POINTER;
		new_type->attr = FFI_G(default_type_attr);
		new_type->size = sizeof(void*);
		new_type->align = _Alignof(void*);
		new_type->pointer.type = arg_dcl->type;
		arg_dcl->type = CREX_FFI_TYPE_MAKE_OWNED(new_type);
	}
	if (crex_ffi_validate_incomplete_type(type, 1, 1) == FAILURE) {
		crex_ffi_cleanup_dcl(arg_dcl);
		crex_hash_destroy(*args);
		pefree(*args, FFI_G(persistent));
		*args = NULL;
		LONGJMP(FFI_G(bailout), FAILURE);
	}
	crex_hash_next_index_insert_ptr(*args, (void*)arg_dcl->type);
}
/* }}} */

void crex_ffi_declare(const char *name, size_t name_len, crex_ffi_dcl *dcl) /* {{{ */
{
	crex_ffi_symbol *sym;

	if (!FFI_G(symbols)) {
		FFI_G(symbols) = pemalloc(sizeof(HashTable), FFI_G(persistent));
		crex_hash_init(FFI_G(symbols), 0, NULL, FFI_G(persistent) ? crex_ffi_symbol_hash_persistent_dtor : crex_ffi_symbol_hash_dtor, FFI_G(persistent));
	}
	crex_ffi_finalize_type(dcl);
	sym = crex_hash_str_find_ptr(FFI_G(symbols), name, name_len);
	if (sym) {
		if ((dcl->flags & CREX_FFI_DCL_STORAGE_CLASS) == CREX_FFI_DCL_TYPEDEF
		 && sym->kind == CREX_FFI_SYM_TYPE
		 && crex_ffi_is_same_type(CREX_FFI_TYPE(sym->type), CREX_FFI_TYPE(dcl->type))
		 && sym->is_const == (bool)(dcl->attr & CREX_FFI_ATTR_CONST)) {
			/* allowed redeclaration */
			crex_ffi_type_dtor(dcl->type);
			return;
		} else if ((dcl->flags & CREX_FFI_DCL_STORAGE_CLASS) == 0
		 || (dcl->flags & CREX_FFI_DCL_STORAGE_CLASS) == CREX_FFI_DCL_EXTERN) {
			crex_ffi_type *type = CREX_FFI_TYPE(dcl->type);

			if (type->kind == CREX_FFI_TYPE_FUNC) {
				if (sym->kind == CREX_FFI_SYM_FUNC
				 && crex_ffi_same_types(CREX_FFI_TYPE(sym->type), type)) {
					/* allowed redeclaration */
					crex_ffi_type_dtor(dcl->type);
					return;
				}
			} else {
				if (sym->kind == CREX_FFI_SYM_VAR
				 && crex_ffi_is_same_type(CREX_FFI_TYPE(sym->type), type)
				 && sym->is_const == (bool)(dcl->attr & CREX_FFI_ATTR_CONST)) {
					/* allowed redeclaration */
					crex_ffi_type_dtor(dcl->type);
					return;
				}
			}
		}
		crex_ffi_parser_error("Redeclaration of \"%.*s\" at line %d", name_len, name, FFI_G(line));
	} else {
		if ((dcl->flags & CREX_FFI_DCL_STORAGE_CLASS) == CREX_FFI_DCL_TYPEDEF) {
			if (crex_ffi_validate_vla(CREX_FFI_TYPE(dcl->type)) == FAILURE) {
				crex_ffi_cleanup_dcl(dcl);
				LONGJMP(FFI_G(bailout), FAILURE);
			}
			if (dcl->align && dcl->align > CREX_FFI_TYPE(dcl->type)->align) {
				if (CREX_FFI_TYPE_IS_OWNED(dcl->type)) {
					CREX_FFI_TYPE(dcl->type)->align = dcl->align;
				} else {
					crex_ffi_type *type = pemalloc(sizeof(crex_ffi_type), FFI_G(persistent));

					memcpy(type, CREX_FFI_TYPE(dcl->type), sizeof(crex_ffi_type));
					type->attr |= FFI_G(default_type_attr);
					type->align = dcl->align;
					dcl->type = CREX_FFI_TYPE_MAKE_OWNED(type);
				}
			}
			sym = pemalloc(sizeof(crex_ffi_symbol), FFI_G(persistent));
			sym->kind = CREX_FFI_SYM_TYPE;
			sym->type = dcl->type;
			sym->is_const = (bool)(dcl->attr & CREX_FFI_ATTR_CONST);
			dcl->type = CREX_FFI_TYPE(dcl->type); /* reset "owned" flag */
			crex_hash_str_add_new_ptr(FFI_G(symbols), name, name_len, sym);
		} else {
			crex_ffi_type *type;

			type = CREX_FFI_TYPE(dcl->type);
			if (crex_ffi_validate_type(type, (dcl->flags & CREX_FFI_DCL_STORAGE_CLASS) == CREX_FFI_DCL_EXTERN, 1) == FAILURE) {
				crex_ffi_cleanup_dcl(dcl);
				LONGJMP(FFI_G(bailout), FAILURE);
			}
			if ((dcl->flags & CREX_FFI_DCL_STORAGE_CLASS) == 0 ||
			    (dcl->flags & CREX_FFI_DCL_STORAGE_CLASS) == CREX_FFI_DCL_EXTERN) {
				sym = pemalloc(sizeof(crex_ffi_symbol), FFI_G(persistent));
				sym->kind = (type->kind == CREX_FFI_TYPE_FUNC) ? CREX_FFI_SYM_FUNC : CREX_FFI_SYM_VAR;
				sym->type = dcl->type;
				sym->is_const = (bool)(dcl->attr & CREX_FFI_ATTR_CONST);
				dcl->type = type; /* reset "owned" flag */
				crex_hash_str_add_new_ptr(FFI_G(symbols), name, name_len, sym);
			} else {
				/* useless declaration */
				crex_ffi_type_dtor(dcl->type);
			}
		}
	}
}
/* }}} */

void crex_ffi_declare_tag(const char *name, size_t name_len, crex_ffi_dcl *dcl, bool incomplete) /* {{{ */
{
	crex_ffi_tag *tag;
	crex_ffi_type *type;

	if (!FFI_G(tags)) {
		FFI_G(tags) = pemalloc(sizeof(HashTable), FFI_G(persistent));
		crex_hash_init(FFI_G(tags), 0, NULL, FFI_G(persistent) ? crex_ffi_tag_hash_persistent_dtor : crex_ffi_tag_hash_dtor, FFI_G(persistent));
	}
	tag = crex_hash_str_find_ptr(FFI_G(tags), name, name_len);
	if (tag) {
		crex_ffi_type *type = CREX_FFI_TYPE(tag->type);

		if (dcl->flags & CREX_FFI_DCL_STRUCT) {
			if (tag->kind != CREX_FFI_TAG_STRUCT) {
				crex_ffi_parser_error("\"%.*s\" defined as wrong kind of tag at line %d", name_len, name, FFI_G(line));
				return;
			} else if (!incomplete && !(type->attr & CREX_FFI_ATTR_INCOMPLETE_TAG)) {
				crex_ffi_parser_error("Redefinition of \"struct %.*s\" at line %d", name_len, name, FFI_G(line));
				return;
			}
		} else if (dcl->flags & CREX_FFI_DCL_UNION) {
			if (tag->kind != CREX_FFI_TAG_UNION) {
				crex_ffi_parser_error("\"%.*s\" defined as wrong kind of tag at line %d", name_len, name, FFI_G(line));
				return;
			} else if (!incomplete && !(type->attr & CREX_FFI_ATTR_INCOMPLETE_TAG)) {
				crex_ffi_parser_error("Redefinition of \"union %.*s\" at line %d", name_len, name, FFI_G(line));
				return;
			}
		} else if (dcl->flags & CREX_FFI_DCL_ENUM) {
			if (tag->kind != CREX_FFI_TAG_ENUM) {
				crex_ffi_parser_error("\"%.*s\" defined as wrong kind of tag at line %d", name_len, name, FFI_G(line));
				return;
			} else if (!incomplete && !(type->attr & CREX_FFI_ATTR_INCOMPLETE_TAG)) {
				crex_ffi_parser_error("Redefinition of \"enum %.*s\" at line %d", name_len, name, FFI_G(line));
				return;
			}
		} else {
			CREX_UNREACHABLE();
			return;
		}
		dcl->type = type;
		if (!incomplete) {
			type->attr &= ~CREX_FFI_ATTR_INCOMPLETE_TAG;
		}
	} else {
		crex_ffi_tag *tag = pemalloc(sizeof(crex_ffi_tag), FFI_G(persistent));
		crex_string *tag_name = crex_string_init(name, name_len, FFI_G(persistent));

		if (dcl->flags & CREX_FFI_DCL_STRUCT) {
			tag->kind = CREX_FFI_TAG_STRUCT;
			crex_ffi_make_struct_type(dcl);
			type = CREX_FFI_TYPE(dcl->type);
			type->record.tag_name = crex_string_copy(tag_name);
		} else if (dcl->flags & CREX_FFI_DCL_UNION) {
			tag->kind = CREX_FFI_TAG_UNION;
			crex_ffi_make_struct_type(dcl);
			type = CREX_FFI_TYPE(dcl->type);
			type->record.tag_name = crex_string_copy(tag_name);
		} else if (dcl->flags & CREX_FFI_DCL_ENUM) {
			tag->kind = CREX_FFI_TAG_ENUM;
			crex_ffi_make_enum_type(dcl);
			type = CREX_FFI_TYPE(dcl->type);
			type->enumeration.tag_name = crex_string_copy(tag_name);
		} else {
			CREX_UNREACHABLE();
		}
		tag->type = CREX_FFI_TYPE_MAKE_OWNED(dcl->type);
		dcl->type = CREX_FFI_TYPE(dcl->type);
		if (incomplete) {
			dcl->type->attr |= CREX_FFI_ATTR_INCOMPLETE_TAG;
		}
		crex_hash_add_new_ptr(FFI_G(tags), tag_name, tag);
		crex_string_release(tag_name);
	}
}
/* }}} */

void crex_ffi_set_abi(crex_ffi_dcl *dcl, uint16_t abi) /* {{{ */
{
	if (dcl->abi != CREX_FFI_ABI_DEFAULT) {
		crex_ffi_parser_error("Multiple calling convention specifiers at line %d", FFI_G(line));
	} else {
		dcl->abi = abi;
	}
}
/* }}} */

#define SIMPLE_ATTRIBUTES(_) \
	_(cdecl) \
	_(fastcall) \
	_(thiscall) \
	_(stdcall) \
	_(ms_abi) \
	_(sysv_abi) \
	_(vectorcall) \
	_(aligned) \
	_(packed) \
	_(ms_struct) \
	_(gcc_struct) \
	_(const) \
	_(malloc) \
	_(deprecated) \
	_(nothrow) \
	_(leaf) \
	_(pure) \
	_(noreturn) \
	_(warn_unused_result)

#define ATTR_ID(name)   attr_ ## name,
#define ATTR_NAME(name) {sizeof(#name)-1, #name},

void crex_ffi_add_attribute(crex_ffi_dcl *dcl, const char *name, size_t name_len) /* {{{ */
{
	enum {
		SIMPLE_ATTRIBUTES(ATTR_ID)
		attr_unsupported
	};
	static const struct {
		size_t len;
		const char * const name;
	} names[] = {
		SIMPLE_ATTRIBUTES(ATTR_NAME)
		{0, NULL}
	};
	int id;

	if (name_len > 4
	 && name[0] == '_'
	 && name[1] == '_'
	 && name[name_len-2] == '_'
	 && name[name_len-1] == '_') {
		name += 2;
		name_len -= 4;
	}
	for (id = 0; names[id].len != 0; id++) {
		if (name_len == names[id].len) {
			if (memcmp(name, names[id].name, name_len) == 0) {
				break;
			}
		}
	}
	switch (id) {
		case attr_cdecl:
			crex_ffi_set_abi(dcl, CREX_FFI_ABI_CDECL);
			break;
		case attr_fastcall:
			crex_ffi_set_abi(dcl, CREX_FFI_ABI_FASTCALL);
			break;
		case attr_thiscall:
			crex_ffi_set_abi(dcl, CREX_FFI_ABI_THISCALL);
			break;
		case attr_stdcall:
			crex_ffi_set_abi(dcl, CREX_FFI_ABI_STDCALL);
			break;
		case attr_ms_abi:
			crex_ffi_set_abi(dcl, CREX_FFI_ABI_MS);
			break;
		case attr_sysv_abi:
			crex_ffi_set_abi(dcl, CREX_FFI_ABI_SYSV);
			break;
		case attr_vectorcall:
			crex_ffi_set_abi(dcl, CREX_FFI_ABI_VECTORCALL);
			break;
		case attr_aligned:
			dcl->align = __BIGGEST_ALIGNMENT__;
			break;
		case attr_packed:
			dcl->attr |= CREX_FFI_ATTR_PACKED;
			break;
		case attr_ms_struct:
			dcl->attr |= CREX_FFI_ATTR_MS_STRUCT;
			break;
		case attr_gcc_struct:
			dcl->attr |= CREX_FFI_ATTR_GCC_STRUCT;
			break;
		case attr_unsupported:
			crex_ffi_parser_error("Unsupported attribute \"%.*s\" at line %d", name_len, name, FFI_G(line));
			break;
		default:
			/* ignore */
			break;
	}
}
/* }}} */

#define VALUE_ATTRIBUTES(_) \
	_(regparam) \
	_(aligned) \
	_(mode) \
	_(nonnull) \
	_(alloc_size) \
	_(format) \
	_(deprecated)

void crex_ffi_add_attribute_value(crex_ffi_dcl *dcl, const char *name, size_t name_len, int n, crex_ffi_val *val) /* {{{ */
{
	enum {
		VALUE_ATTRIBUTES(ATTR_ID)
		attr_unsupported
	};
	static const struct {
		size_t len;
		const char * const name;
	} names[] = {
		VALUE_ATTRIBUTES(ATTR_NAME)
		{0, NULL}
	};
	int id;

	if (name_len > 4
	 && name[0] == '_'
	 && name[1] == '_'
	 && name[name_len-2] == '_'
	 && name[name_len-1] == '_') {
		name += 2;
		name_len -= 4;
	}
	for (id = 0; names[id].len != 0; id++) {
		if (name_len == names[id].len) {
			if (memcmp(name, names[id].name, name_len) == 0) {
				break;
			}
		}
	}
	switch (id) {
		case attr_regparam:
			if (n == 0
			 && (val->kind == CREX_FFI_VAL_INT32 || val->kind == CREX_FFI_VAL_UINT32 || val->kind == CREX_FFI_VAL_INT64 || val->kind == CREX_FFI_VAL_UINT64)
			 && val->i64 == 3) {
				crex_ffi_set_abi(dcl, CREX_FFI_ABI_REGISTER);
			} else {
				crex_ffi_parser_error("Incorrect \"regparam\" value at line %d", FFI_G(line));
			}
			break;
		case attr_aligned:
			if (n == 0
			 && (val->kind == CREX_FFI_VAL_INT32 || val->kind == CREX_FFI_VAL_UINT32 || val->kind == CREX_FFI_VAL_INT64 || val->kind == CREX_FFI_VAL_UINT64)
			 && val->i64 > 0 && val->i64 <= 0x80000000 && (val->i64 & (val->i64 - 1)) == 0) {
				dcl->align = val->i64;
			} else {
				crex_ffi_parser_error("Incorrect \"alignment\" value at line %d", FFI_G(line));
			}
			break;
		case attr_mode:
			if (n == 0
			 && (val->kind == CREX_FFI_VAL_NAME)) {
				const char *str = val->str;
				size_t len = val->len;
				if (len > 4
				 && str[0] == '_'
				 && str[1] == '_'
				 && str[len-2] == '_'
				 && str[len-1] == '_') {
					str += 2;
					len -= 4;
				}
				// TODO: Add support for vector type 'VnXX' ???
				if (len == 2) {
					if (str[1] == 'I') {
						if (dcl->flags & (CREX_FFI_DCL_TYPE_SPECIFIERS-(CREX_FFI_DCL_CHAR|CREX_FFI_DCL_SHORT|CREX_FFI_DCL_INT|CREX_FFI_DCL_LONG|CREX_FFI_DCL_LONG_LONG|CREX_FFI_DCL_SIGNED|CREX_FFI_DCL_UNSIGNED))) {
							/* inappropriate type */
						} else if (str[0] == 'Q') {
							dcl->flags &= ~(CREX_FFI_DCL_CHAR|CREX_FFI_DCL_SHORT|CREX_FFI_DCL_INT|CREX_FFI_DCL_LONG|CREX_FFI_DCL_LONG_LONG);
							dcl->flags |= CREX_FFI_DCL_CHAR;
							break;
						} else if (str[0] == 'H') {
							dcl->flags &= ~(CREX_FFI_DCL_CHAR|CREX_FFI_DCL_SHORT|CREX_FFI_DCL_INT|CREX_FFI_DCL_LONG|CREX_FFI_DCL_LONG_LONG);
							dcl->flags |= CREX_FFI_DCL_SHORT;
							break;
						} else if (str[0] == 'S') {
							dcl->flags &= ~(CREX_FFI_DCL_CHAR|CREX_FFI_DCL_SHORT|CREX_FFI_DCL_INT|CREX_FFI_DCL_LONG|CREX_FFI_DCL_LONG_LONG);
							dcl->flags |= CREX_FFI_DCL_INT;
							break;
						} else if (str[0] == 'D') {
							dcl->flags &= ~(CREX_FFI_DCL_CHAR|CREX_FFI_DCL_SHORT|CREX_FFI_DCL_INT|CREX_FFI_DCL_LONG|CREX_FFI_DCL_LONG_LONG);
							if (sizeof(long) == 8) {
								dcl->flags |= CREX_FFI_DCL_LONG;
							} else {
								dcl->flags |= CREX_FFI_DCL_LONG|CREX_FFI_DCL_LONG_LONG;
							}
							break;
						}
					} else if (str[1] == 'F') {
						if (dcl->flags & (CREX_FFI_DCL_TYPE_SPECIFIERS-(CREX_FFI_DCL_LONG|CREX_FFI_DCL_FLOAT|CREX_FFI_DCL_DOUBLE))) {
							/* inappropriate type */
						} else if (str[0] == 'S') {
							dcl->flags &= ~(CREX_FFI_DCL_LONG|CREX_FFI_DCL_FLOAT|CREX_FFI_DCL_DOUBLE);
							dcl->flags |= CREX_FFI_DCL_FLOAT;
							break;
						} else if (str[0] == 'D') {
							dcl->flags &= ~(CREX_FFI_DCL_LONG|CREX_FFI_DCL_FLOAT|CREX_FFI_DCL_DOUBLE);
							dcl->flags |= CREX_FFI_DCL_DOUBLE;
							break;
						}
					}
				}
			}
			crex_ffi_parser_error("Unsupported \"mode\" value at line %d", FFI_G(line));
			// TODO: ???
		case attr_unsupported:
			crex_ffi_parser_error("Unsupported attribute \"%.*s\" at line %d", name_len, name, FFI_G(line));
			break;
		default:
			/* ignore */
			break;
	}
}
/* }}} */

void crex_ffi_add_msvc_attribute_value(crex_ffi_dcl *dcl, const char *name, size_t name_len, crex_ffi_val *val) /* {{{ */
{
	if (name_len == sizeof("align")-1 && memcmp(name, "align", sizeof("align")-1) == 0) {
		if ((val->kind == CREX_FFI_VAL_INT32 || val->kind == CREX_FFI_VAL_UINT32 || val->kind == CREX_FFI_VAL_INT64 || val->kind == CREX_FFI_VAL_UINT64)
		 && val->i64 > 0 && val->i64 <= 0x80000000 && (val->i64 & (val->i64 - 1)) == 0) {
			dcl->align = val->i64;
		} else {
			crex_ffi_parser_error("Incorrect \"alignment\" value at line %d", FFI_G(line));
		}
	} else {
		/* ignore */
	}
}
/* }}} */

static crex_result crex_ffi_nested_type(crex_ffi_type *type, crex_ffi_type *nested_type) /* {{{ */
{
	nested_type = CREX_FFI_TYPE(nested_type);
	switch (nested_type->kind) {
		case CREX_FFI_TYPE_POINTER:
			/* "char" is used as a terminator of nested declaration */
			if (nested_type->pointer.type == &crex_ffi_type_char) {
				nested_type->pointer.type = type;
				return crex_ffi_validate_vla(CREX_FFI_TYPE(type));
			} else {
				return crex_ffi_nested_type(type, nested_type->pointer.type);
			}
			break;
		case CREX_FFI_TYPE_ARRAY:
			/* "char" is used as a terminator of nested declaration */
			if (nested_type->array.type == &crex_ffi_type_char) {
				nested_type->array.type = type;
				if (crex_ffi_validate_array_element_type(CREX_FFI_TYPE(type)) == FAILURE) {
					return FAILURE;
				}
			} else {
				if (crex_ffi_nested_type(type, nested_type->array.type) != SUCCESS) {
					return FAILURE;
				}
			}
			nested_type->size = nested_type->array.length * CREX_FFI_TYPE(nested_type->array.type)->size;
			nested_type->align = CREX_FFI_TYPE(nested_type->array.type)->align;
			return SUCCESS;
			break;
		case CREX_FFI_TYPE_FUNC:
			/* "char" is used as a terminator of nested declaration */
			if (nested_type->func.ret_type == &crex_ffi_type_char) {
				nested_type->func.ret_type = type;
				return crex_ffi_validate_func_ret_type(CREX_FFI_TYPE(type));
			} else {
				return crex_ffi_nested_type(type, nested_type->func.ret_type);
			}
			break;
		default:
			CREX_UNREACHABLE();
	}
}
/* }}} */

void crex_ffi_nested_declaration(crex_ffi_dcl *dcl, crex_ffi_dcl *nested_dcl) /* {{{ */
{
	/* "char" is used as a terminator of nested declaration */
	crex_ffi_finalize_type(dcl);
	if (!nested_dcl->type || nested_dcl->type == &crex_ffi_type_char) {
		nested_dcl->type = dcl->type;
	} else {
		if (crex_ffi_nested_type(dcl->type, nested_dcl->type) == FAILURE) {
			crex_ffi_cleanup_dcl(nested_dcl);
			LONGJMP(FFI_G(bailout), FAILURE);
		}
	}
	dcl->type = nested_dcl->type;
}
/* }}} */

void crex_ffi_align_as_type(crex_ffi_dcl *dcl, crex_ffi_dcl *align_dcl) /* {{{ */
{
	crex_ffi_finalize_type(align_dcl);
	dcl->align = MAX(align_dcl->align, CREX_FFI_TYPE(align_dcl->type)->align);
}
/* }}} */

void crex_ffi_align_as_val(crex_ffi_dcl *dcl, crex_ffi_val *align_val) /* {{{ */
{
	switch (align_val->kind) {
		case CREX_FFI_VAL_INT32:
		case CREX_FFI_VAL_UINT32:
			dcl->align = crex_ffi_type_uint32.align;
			break;
		case CREX_FFI_VAL_INT64:
		case CREX_FFI_VAL_UINT64:
			dcl->align = crex_ffi_type_uint64.align;
			break;
		case CREX_FFI_VAL_FLOAT:
			dcl->align = crex_ffi_type_float.align;
			break;
		case CREX_FFI_VAL_DOUBLE:
			dcl->align = crex_ffi_type_double.align;
			break;
#ifdef HAVE_LONG_DOUBLE
		case CREX_FFI_VAL_LONG_DOUBLE:
			dcl->align = crex_ffi_type_long_double.align;
			break;
#endif
		case CREX_FFI_VAL_CHAR:
		case CREX_FFI_VAL_STRING:
			dcl->align = crex_ffi_type_char.align;
			break;
		default:
			break;
	}
}
/* }}} */

#define crex_ffi_expr_bool(val) do { \
	if (val->kind == CREX_FFI_VAL_UINT32 || val->kind == CREX_FFI_VAL_UINT64) { \
		val->kind = CREX_FFI_VAL_INT32; \
		val->i64 = !!val->u64; \
	} else if (val->kind == CREX_FFI_VAL_INT32 || val->kind == CREX_FFI_VAL_INT64) { \
		val->kind = CREX_FFI_VAL_INT32; \
		val->i64 = !!val->i64; \
	} else if (val->kind == CREX_FFI_VAL_FLOAT || val->kind == CREX_FFI_VAL_DOUBLE || val->kind == CREX_FFI_VAL_LONG_DOUBLE) { \
		val->kind = CREX_FFI_VAL_INT32; \
		val->i64 = !!val->d; \
	} else if (val->kind == CREX_FFI_VAL_CHAR) { \
		val->kind = CREX_FFI_VAL_INT32; \
		val->i64 = !!val->ch; \
	} else { \
		val->kind = CREX_FFI_VAL_ERROR; \
	} \
} while (0)

#define crex_ffi_expr_math(val, op2, OP) do { \
	if (val->kind == CREX_FFI_VAL_UINT32 || val->kind == CREX_FFI_VAL_UINT64) { \
		if (op2->kind == CREX_FFI_VAL_UINT32 || op2->kind == CREX_FFI_VAL_UINT64) { \
			val->kind = MAX(val->kind, op2->kind); \
			val->u64 = val->u64 OP op2->u64; \
		} else if (op2->kind == CREX_FFI_VAL_INT32) { \
			val->u64 = val->u64 OP op2->i64; \
		} else if (op2->kind == CREX_FFI_VAL_INT64) { \
			val->u64 = val->u64 OP op2->i64; \
		} else if (op2->kind == CREX_FFI_VAL_FLOAT || op2->kind == CREX_FFI_VAL_DOUBLE || op2->kind == CREX_FFI_VAL_LONG_DOUBLE) { \
			val->kind = op2->kind; \
			val->d = (crex_ffi_double)val->u64 OP op2->d; \
		} else if (op2->kind == CREX_FFI_VAL_CHAR) { \
			val->u64 = val->u64 OP op2->ch; \
		} else { \
			val->kind = CREX_FFI_VAL_ERROR; \
		} \
	} else if (val->kind == CREX_FFI_VAL_INT32 || val->kind == CREX_FFI_VAL_INT64) { \
		if (op2->kind == CREX_FFI_VAL_UINT32) { \
			val->i64 = val->i64 OP op2->u64; \
		} else if (op2->kind == CREX_FFI_VAL_UINT64) { \
			val->i64 = val->i64 OP op2->u64; \
		} else if (op2->kind == CREX_FFI_VAL_INT32 || op2->kind == CREX_FFI_VAL_INT64) { \
			val->kind = MAX(val->kind, op2->kind); \
			val->i64 = val->i64 OP op2->i64; \
		} else if (op2->kind == CREX_FFI_VAL_FLOAT || op2->kind == CREX_FFI_VAL_DOUBLE || op2->kind == CREX_FFI_VAL_LONG_DOUBLE) { \
			val->kind = op2->kind; \
			val->d = (crex_ffi_double)val->i64 OP op2->d; \
		} else if (op2->kind == CREX_FFI_VAL_CHAR) { \
			val->i64 = val->i64 OP op2->ch; \
		} else { \
			val->kind = CREX_FFI_VAL_ERROR; \
		} \
	} else if (val->kind == CREX_FFI_VAL_FLOAT || val->kind == CREX_FFI_VAL_DOUBLE || val->kind == CREX_FFI_VAL_LONG_DOUBLE) { \
		if (op2->kind == CREX_FFI_VAL_UINT32 || op2->kind == CREX_FFI_VAL_UINT64) { \
			val->d = val->d OP (crex_ffi_double)op2->u64; \
		} else if (op2->kind == CREX_FFI_VAL_INT32 ||op2->kind == CREX_FFI_VAL_INT64) { \
			val->d = val->d OP (crex_ffi_double)op2->i64; \
		} else if (op2->kind == CREX_FFI_VAL_FLOAT || op2->kind == CREX_FFI_VAL_DOUBLE || op2->kind == CREX_FFI_VAL_LONG_DOUBLE) { \
			val->kind = MAX(val->kind, op2->kind); \
			val->d = val->d OP op2->d; \
		} else if (op2->kind == CREX_FFI_VAL_CHAR) { \
			val->d = val->d OP (crex_ffi_double)op2->ch; \
		} else { \
			val->kind = CREX_FFI_VAL_ERROR; \
		} \
	} else if (val->kind == CREX_FFI_VAL_CHAR) { \
		if (op2->kind == CREX_FFI_VAL_UINT32 || op2->kind == CREX_FFI_VAL_UINT64) { \
			val->kind = op2->kind; \
			val->u64 = val->ch OP op2->u64; \
		} else if (op2->kind == CREX_FFI_VAL_INT32 || op2->kind == CREX_FFI_VAL_INT64) { \
			val->kind = CREX_FFI_VAL_INT64; \
			val->i64 = val->ch OP op2->i64; \
		} else if (op2->kind == CREX_FFI_VAL_FLOAT || op2->kind == CREX_FFI_VAL_DOUBLE || op2->kind == CREX_FFI_VAL_LONG_DOUBLE) { \
			val->kind = op2->kind; \
			val->d = (crex_ffi_double)val->ch OP op2->d; \
		} else if (op2->kind == CREX_FFI_VAL_CHAR) { \
			val->ch = val->ch OP op2->ch; \
		} else { \
			val->kind = CREX_FFI_VAL_ERROR; \
		} \
	} else { \
		val->kind = CREX_FFI_VAL_ERROR; \
	} \
} while (0)

#define crex_ffi_expr_int_math(val, op2, OP) do { \
	if (val->kind == CREX_FFI_VAL_UINT32 || val->kind == CREX_FFI_VAL_UINT64) { \
		if (op2->kind == CREX_FFI_VAL_UINT32 || op2->kind == CREX_FFI_VAL_UINT64) { \
			val->kind = MAX(val->kind, op2->kind); \
			val->u64 = val->u64 OP op2->u64; \
		} else if (op2->kind == CREX_FFI_VAL_INT32) { \
			val->u64 = val->u64 OP op2->i64; \
		} else if (op2->kind == CREX_FFI_VAL_INT64) { \
			val->u64 = val->u64 OP op2->i64; \
		} else if (op2->kind == CREX_FFI_VAL_FLOAT || op2->kind == CREX_FFI_VAL_DOUBLE || op2->kind == CREX_FFI_VAL_LONG_DOUBLE) { \
			val->u64 = val->u64 OP (uint64_t)op2->d; \
		} else if (op2->kind == CREX_FFI_VAL_CHAR) { \
			val->u64 = val->u64 OP op2->ch; \
		} else { \
			val->kind = CREX_FFI_VAL_ERROR; \
		} \
	} else if (val->kind == CREX_FFI_VAL_INT32 || val->kind == CREX_FFI_VAL_INT64) { \
		if (op2->kind == CREX_FFI_VAL_UINT32) { \
			val->i64 = val->i64 OP op2->u64; \
		} else if (op2->kind == CREX_FFI_VAL_UINT64) { \
			val->i64 = val->i64 OP op2->u64; \
		} else if (op2->kind == CREX_FFI_VAL_INT32 || op2->kind == CREX_FFI_VAL_INT64) { \
			val->kind = MAX(val->kind, op2->kind); \
			val->i64 = val->i64 OP op2->i64; \
		} else if (op2->kind == CREX_FFI_VAL_FLOAT || op2->kind == CREX_FFI_VAL_DOUBLE || op2->kind == CREX_FFI_VAL_LONG_DOUBLE) { \
			val->u64 = val->u64 OP (int64_t)op2->d; \
		} else if (op2->kind == CREX_FFI_VAL_CHAR) { \
			val->i64 = val->i64 OP op2->ch; \
		} else { \
			val->kind = CREX_FFI_VAL_ERROR; \
		} \
	} else if (val->kind == CREX_FFI_VAL_FLOAT || val->kind == CREX_FFI_VAL_DOUBLE || val->kind == CREX_FFI_VAL_LONG_DOUBLE) { \
		if (op2->kind == CREX_FFI_VAL_UINT32 || op2->kind == CREX_FFI_VAL_UINT64) { \
			val->kind = op2->kind; \
			val->u64 = (uint64_t)val->d OP op2->u64; \
		} else if (op2->kind == CREX_FFI_VAL_INT32 || op2->kind == CREX_FFI_VAL_INT64) { \
			val->kind = op2->kind; \
			val->i64 = (int64_t)val->d OP op2->i64; \
		} else { \
			val->kind = CREX_FFI_VAL_ERROR; \
		} \
	} else if (val->kind == CREX_FFI_VAL_CHAR) { \
		if (op2->kind == CREX_FFI_VAL_UINT32 || op2->kind == CREX_FFI_VAL_UINT64) { \
			val->kind = op2->kind; \
			val->u64 = (uint64_t)val->ch OP op2->u64; \
		} else if (op2->kind == CREX_FFI_VAL_INT32 || op2->kind == CREX_FFI_VAL_INT64) { \
			val->kind = op2->kind; \
			val->i64 = (int64_t)val->ch OP op2->u64; \
		} else if (op2->kind == CREX_FFI_VAL_CHAR) { \
			val->ch = val->ch OP op2->ch; \
		} else { \
			val->kind = CREX_FFI_VAL_ERROR; \
		} \
	} else { \
		val->kind = CREX_FFI_VAL_ERROR; \
	} \
} while (0)

#define crex_ffi_expr_cmp(val, op2, OP) do { \
	if (val->kind == CREX_FFI_VAL_UINT32 || val->kind == CREX_FFI_VAL_UINT64) { \
		if (op2->kind == CREX_FFI_VAL_UINT32 || op2->kind == CREX_FFI_VAL_UINT64) { \
			val->kind = CREX_FFI_VAL_INT32; \
			val->i64 = val->u64 OP op2->u64; \
		} else if (op2->kind == CREX_FFI_VAL_INT32 || op2->kind == CREX_FFI_VAL_INT64) { \
			val->kind = CREX_FFI_VAL_INT32; \
			val->i64 = val->u64 OP op2->u64; /*signed/unsigned */ \
		} else if (op2->kind == CREX_FFI_VAL_FLOAT || op2->kind == CREX_FFI_VAL_DOUBLE || op2->kind == CREX_FFI_VAL_LONG_DOUBLE) { \
			val->kind = CREX_FFI_VAL_INT32; \
			val->i64 = (crex_ffi_double)val->u64 OP op2->d; \
		} else if (op2->kind == CREX_FFI_VAL_CHAR) { \
			val->kind = CREX_FFI_VAL_INT32; \
			val->i64 = val->u64 OP op2->d; \
		} else { \
			val->kind = CREX_FFI_VAL_ERROR; \
		} \
	} else if (val->kind == CREX_FFI_VAL_INT32 || val->kind == CREX_FFI_VAL_INT64) { \
		if (op2->kind == CREX_FFI_VAL_UINT32 || op2->kind == CREX_FFI_VAL_UINT64) { \
			val->kind = CREX_FFI_VAL_INT32; \
			val->i64 = val->i64 OP op2->i64; /* signed/unsigned */ \
		} else if (op2->kind == CREX_FFI_VAL_INT32 || op2->kind == CREX_FFI_VAL_INT64) { \
			val->kind = CREX_FFI_VAL_INT32; \
			val->i64 = val->i64 OP op2->i64; \
		} else if (op2->kind == CREX_FFI_VAL_FLOAT || op2->kind == CREX_FFI_VAL_DOUBLE || op2->kind == CREX_FFI_VAL_LONG_DOUBLE) { \
			val->kind = CREX_FFI_VAL_INT32; \
			val->i64 = (crex_ffi_double)val->i64 OP op2->d; \
		} else if (op2->kind == CREX_FFI_VAL_CHAR) { \
			val->kind = CREX_FFI_VAL_INT32; \
			val->i64 = val->i64 OP op2->ch; \
		} else { \
			val->kind = CREX_FFI_VAL_ERROR; \
		} \
	} else if (val->kind == CREX_FFI_VAL_FLOAT || val->kind == CREX_FFI_VAL_DOUBLE || val->kind == CREX_FFI_VAL_LONG_DOUBLE) { \
		if (op2->kind == CREX_FFI_VAL_UINT32 || op2->kind == CREX_FFI_VAL_UINT64) { \
			val->kind = CREX_FFI_VAL_INT32; \
			val->i64 = val->d OP (crex_ffi_double)op2->u64; \
		} else if (op2->kind == CREX_FFI_VAL_INT32 ||op2->kind == CREX_FFI_VAL_INT64) { \
			val->kind = CREX_FFI_VAL_INT32; \
			val->i64 = val->d OP (crex_ffi_double)op2->i64; \
		} else if (op2->kind == CREX_FFI_VAL_FLOAT || op2->kind == CREX_FFI_VAL_DOUBLE || op2->kind == CREX_FFI_VAL_LONG_DOUBLE) { \
			val->kind = CREX_FFI_VAL_INT32; \
			val->i64 = val->d OP op2->d; \
		} else if (op2->kind == CREX_FFI_VAL_CHAR) { \
			val->kind = CREX_FFI_VAL_INT32; \
			val->i64 = val->d OP (crex_ffi_double)op2->ch; \
		} else { \
			val->kind = CREX_FFI_VAL_ERROR; \
		} \
	} else if (val->kind == CREX_FFI_VAL_CHAR) { \
		if (op2->kind == CREX_FFI_VAL_UINT32 || op2->kind == CREX_FFI_VAL_UINT64) { \
			val->kind = CREX_FFI_VAL_INT32; \
			val->i64 = val->ch OP op2->i64; /* signed/unsigned */ \
		} else if (op2->kind == CREX_FFI_VAL_INT32 || op2->kind == CREX_FFI_VAL_INT64) { \
			val->kind = CREX_FFI_VAL_INT32; \
			val->i64 = val->ch OP op2->i64; \
		} else if (op2->kind == CREX_FFI_VAL_FLOAT || op2->kind == CREX_FFI_VAL_DOUBLE || op2->kind == CREX_FFI_VAL_LONG_DOUBLE) { \
			val->kind = CREX_FFI_VAL_INT32; \
			val->i64 = (crex_ffi_double)val->ch OP op2->d; \
		} else if (op2->kind == CREX_FFI_VAL_CHAR) { \
			val->kind = CREX_FFI_VAL_INT32; \
			val->i64 = val->ch OP op2->ch; \
		} else { \
			val->kind = CREX_FFI_VAL_ERROR; \
		} \
	} else { \
		val->kind = CREX_FFI_VAL_ERROR; \
	} \
} while (0)

void crex_ffi_expr_conditional(crex_ffi_val *val, crex_ffi_val *op2, crex_ffi_val *op3) /* {{{ */
{
	crex_ffi_expr_bool(val);
	if (val->kind == CREX_FFI_VAL_INT32) {
		if (val->i64) {
			*val = *op2;
		} else {
			*val = *op3;
		}
	}
}
/* }}} */

void crex_ffi_expr_bool_or(crex_ffi_val *val, crex_ffi_val *op2) /* {{{ */
{
	crex_ffi_expr_bool(val);
	crex_ffi_expr_bool(op2);
	if (val->kind == CREX_FFI_VAL_INT32 && op2->kind == CREX_FFI_VAL_INT32) {
		val->i64 = val->i64 || op2->i64;
	} else {
		val->kind = CREX_FFI_VAL_ERROR;
	}
}
/* }}} */

void crex_ffi_expr_bool_and(crex_ffi_val *val, crex_ffi_val *op2) /* {{{ */
{
	crex_ffi_expr_bool(val);
	crex_ffi_expr_bool(op2);
	if (val->kind == CREX_FFI_VAL_INT32 && op2->kind == CREX_FFI_VAL_INT32) {
		val->i64 = val->i64 && op2->i64;
	} else {
		val->kind = CREX_FFI_VAL_ERROR;
	}
}
/* }}} */

void crex_ffi_expr_bw_or(crex_ffi_val *val, crex_ffi_val *op2) /* {{{ */
{
	crex_ffi_expr_int_math(val, op2, |);
}
/* }}} */

void crex_ffi_expr_bw_xor(crex_ffi_val *val, crex_ffi_val *op2) /* {{{ */
{
	crex_ffi_expr_int_math(val, op2, ^);
}
/* }}} */

void crex_ffi_expr_bw_and(crex_ffi_val *val, crex_ffi_val *op2) /* {{{ */
{
	crex_ffi_expr_int_math(val, op2, &);
}
/* }}} */

void crex_ffi_expr_is_equal(crex_ffi_val *val, crex_ffi_val *op2) /* {{{ */
{
	crex_ffi_expr_cmp(val, op2, ==);
}
/* }}} */

void crex_ffi_expr_is_not_equal(crex_ffi_val *val, crex_ffi_val *op2) /* {{{ */
{
	crex_ffi_expr_cmp(val, op2, !=);
}
/* }}} */

void crex_ffi_expr_is_less(crex_ffi_val *val, crex_ffi_val *op2) /* {{{ */
{
	crex_ffi_expr_cmp(val, op2, <);
}
/* }}} */

void crex_ffi_expr_is_greater(crex_ffi_val *val, crex_ffi_val *op2) /* {{{ */
{
	crex_ffi_expr_cmp(val, op2, >);
}
/* }}} */

void crex_ffi_expr_is_less_or_equal(crex_ffi_val *val, crex_ffi_val *op2) /* {{{ */
{
	crex_ffi_expr_cmp(val, op2, <=);
}
/* }}} */

void crex_ffi_expr_is_greater_or_equal(crex_ffi_val *val, crex_ffi_val *op2) /* {{{ */
{
	crex_ffi_expr_cmp(val, op2, >=);
}
/* }}} */

void crex_ffi_expr_shift_left(crex_ffi_val *val, crex_ffi_val *op2) /* {{{ */
{
	crex_ffi_expr_int_math(val, op2, <<);
}
/* }}} */

void crex_ffi_expr_shift_right(crex_ffi_val *val, crex_ffi_val *op2) /* {{{ */
{
	crex_ffi_expr_int_math(val, op2, >>);
}
/* }}} */

void crex_ffi_expr_add(crex_ffi_val *val, crex_ffi_val *op2) /* {{{ */
{
	crex_ffi_expr_math(val, op2, +);
}
/* }}} */

void crex_ffi_expr_sub(crex_ffi_val *val, crex_ffi_val *op2) /* {{{ */
{
	crex_ffi_expr_math(val, op2, -);
}
/* }}} */

void crex_ffi_expr_mul(crex_ffi_val *val, crex_ffi_val *op2) /* {{{ */
{
	crex_ffi_expr_math(val, op2, *);
}
/* }}} */

void crex_ffi_expr_div(crex_ffi_val *val, crex_ffi_val *op2) /* {{{ */
{
	crex_ffi_expr_math(val, op2, /);
}
/* }}} */

void crex_ffi_expr_mod(crex_ffi_val *val, crex_ffi_val *op2) /* {{{ */
{
	crex_ffi_expr_int_math(val, op2, %); // ???
}
/* }}} */

void crex_ffi_expr_cast(crex_ffi_val *val, crex_ffi_dcl *dcl) /* {{{ */
{
	crex_ffi_finalize_type(dcl);
	switch (CREX_FFI_TYPE(dcl->type)->kind) {
		case CREX_FFI_TYPE_FLOAT:
			if (val->kind == CREX_FFI_VAL_UINT32 || val->kind == CREX_FFI_VAL_UINT64) {
				val->kind = CREX_FFI_VAL_FLOAT;
				val->d = val->u64;
			} else if (val->kind == CREX_FFI_VAL_INT32 || val->kind == CREX_FFI_VAL_INT64) {
				val->kind = CREX_FFI_VAL_FLOAT;
				val->d = val->i64;
			} else if (val->kind == CREX_FFI_VAL_FLOAT || val->kind == CREX_FFI_VAL_DOUBLE || val->kind == CREX_FFI_VAL_LONG_DOUBLE) {
				val->kind = CREX_FFI_VAL_FLOAT;
			} else if (val->kind == CREX_FFI_VAL_CHAR) {
				val->kind = CREX_FFI_VAL_FLOAT;
				val->d = val->ch;
			} else {
				val->kind = CREX_FFI_VAL_ERROR;
			}
			break;
		case CREX_FFI_TYPE_DOUBLE:
			if (val->kind == CREX_FFI_VAL_UINT32 || val->kind == CREX_FFI_VAL_UINT64) {
				val->kind = CREX_FFI_VAL_DOUBLE;
				val->d = val->u64;
			} else if (val->kind == CREX_FFI_VAL_INT32 || val->kind == CREX_FFI_VAL_INT64) {
				val->kind = CREX_FFI_VAL_DOUBLE;
				val->d = val->i64;
			} else if (val->kind == CREX_FFI_VAL_FLOAT || val->kind == CREX_FFI_VAL_DOUBLE || val->kind == CREX_FFI_VAL_LONG_DOUBLE) {
				val->kind = CREX_FFI_VAL_DOUBLE;
			} else if (val->kind == CREX_FFI_VAL_CHAR) {
				val->kind = CREX_FFI_VAL_DOUBLE;
				val->d = val->ch;
			} else {
				val->kind = CREX_FFI_VAL_ERROR;
			}
			break;
#ifdef HAVE_LONG_DOUBLE
		case CREX_FFI_TYPE_LONGDOUBLE:
			if (val->kind == CREX_FFI_VAL_UINT32 || val->kind == CREX_FFI_VAL_UINT64) {
				val->kind = CREX_FFI_VAL_LONG_DOUBLE;
				val->d = val->u64;
			} else if (val->kind == CREX_FFI_VAL_INT32 || val->kind == CREX_FFI_VAL_INT64) {
				val->kind = CREX_FFI_VAL_LONG_DOUBLE;
				val->d = val->i64;
			} else if (val->kind == CREX_FFI_VAL_FLOAT || val->kind == CREX_FFI_VAL_DOUBLE || val->kind == CREX_FFI_VAL_LONG_DOUBLE) {
				val->kind = CREX_FFI_VAL_LONG_DOUBLE;
			} else if (val->kind == CREX_FFI_VAL_CHAR) {
				val->kind = CREX_FFI_VAL_LONG_DOUBLE;
				val->d = val->ch;
			} else {
				val->kind = CREX_FFI_VAL_ERROR;
			}
			break;
#endif
		case CREX_FFI_TYPE_UINT8:
		case CREX_FFI_TYPE_UINT16:
		case CREX_FFI_TYPE_UINT32:
		case CREX_FFI_TYPE_BOOL:
			if (val->kind == CREX_FFI_VAL_UINT32 || val->kind == CREX_FFI_VAL_UINT64 || val->kind == CREX_FFI_VAL_INT32 || val->kind == CREX_FFI_VAL_INT64) {
				val->kind = CREX_FFI_VAL_UINT32;
			} else if (val->kind == CREX_FFI_VAL_FLOAT || val->kind == CREX_FFI_VAL_DOUBLE || val->kind == CREX_FFI_VAL_LONG_DOUBLE) {
				val->kind = CREX_FFI_VAL_UINT32;
				val->u64 = val->d;
			} else if (val->kind == CREX_FFI_VAL_CHAR) {
				val->kind = CREX_FFI_VAL_UINT32;
				val->u64 = val->ch;
			} else {
				val->kind = CREX_FFI_VAL_ERROR;
			}
			break;
		case CREX_FFI_TYPE_SINT8:
		case CREX_FFI_TYPE_SINT16:
		case CREX_FFI_TYPE_SINT32:
			if (val->kind == CREX_FFI_VAL_UINT32 || val->kind == CREX_FFI_VAL_UINT64 || val->kind == CREX_FFI_VAL_INT32 || val->kind == CREX_FFI_VAL_INT64) {
				val->kind = CREX_FFI_VAL_INT32;
			} else if (val->kind == CREX_FFI_VAL_FLOAT || val->kind == CREX_FFI_VAL_DOUBLE || val->kind == CREX_FFI_VAL_LONG_DOUBLE) {
				val->kind = CREX_FFI_VAL_INT32;
				val->i64 = val->d;
			} else if (val->kind == CREX_FFI_VAL_CHAR) {
				val->kind = CREX_FFI_VAL_INT32;
				val->i64 = val->ch;
			} else {
				val->kind = CREX_FFI_VAL_ERROR;
			}
			break;
		case CREX_FFI_TYPE_UINT64:
			if (val->kind == CREX_FFI_VAL_UINT32 || val->kind == CREX_FFI_VAL_UINT64 || val->kind == CREX_FFI_VAL_INT32 || val->kind == CREX_FFI_VAL_INT64) {
				val->kind = CREX_FFI_VAL_UINT64;
			} else if (val->kind == CREX_FFI_VAL_FLOAT || val->kind == CREX_FFI_VAL_DOUBLE || val->kind == CREX_FFI_VAL_LONG_DOUBLE) {
				val->kind = CREX_FFI_VAL_UINT64;
				val->u64 = val->d;
			} else if (val->kind == CREX_FFI_VAL_CHAR) {
				val->kind = CREX_FFI_VAL_UINT64;
				val->u64 = val->ch;
			} else {
				val->kind = CREX_FFI_VAL_ERROR;
			}
			break;
		case CREX_FFI_TYPE_SINT64:
			if (val->kind == CREX_FFI_VAL_UINT32 || val->kind == CREX_FFI_VAL_UINT64) {
				val->kind = CREX_FFI_VAL_CHAR;
				val->ch = val->u64;
			} else if (val->kind == CREX_FFI_VAL_INT32 || val->kind == CREX_FFI_VAL_INT64) {
				val->kind = CREX_FFI_VAL_CHAR;
				val->ch = val->i64;
			} else if (val->kind == CREX_FFI_VAL_FLOAT || val->kind == CREX_FFI_VAL_DOUBLE || val->kind == CREX_FFI_VAL_LONG_DOUBLE) {
				val->kind = CREX_FFI_VAL_CHAR;
				val->ch = val->d;
			} else if (val->kind == CREX_FFI_VAL_CHAR) {
			} else {
				val->kind = CREX_FFI_VAL_ERROR;
			}
			break;
		case CREX_FFI_TYPE_CHAR:
			if (val->kind == CREX_FFI_VAL_UINT32 || val->kind == CREX_FFI_VAL_UINT64 || val->kind == CREX_FFI_VAL_INT32 || val->kind == CREX_FFI_VAL_INT64) {
				val->kind = CREX_FFI_VAL_UINT32;
			} else if (val->kind == CREX_FFI_VAL_FLOAT || val->kind == CREX_FFI_VAL_DOUBLE || val->kind == CREX_FFI_VAL_LONG_DOUBLE) {
				val->kind = CREX_FFI_VAL_UINT32;
				val->u64 = val->d;
			} else if (val->kind == CREX_FFI_VAL_CHAR) {
				val->kind = CREX_FFI_VAL_UINT32;
				val->u64 = val->ch;
			} else {
				val->kind = CREX_FFI_VAL_ERROR;
			}
			break;
		default:
			val->kind = CREX_FFI_VAL_ERROR;
			break;
	}
	crex_ffi_type_dtor(dcl->type);
}
/* }}} */

void crex_ffi_expr_plus(crex_ffi_val *val) /* {{{ */
{
	if (val->kind == CREX_FFI_VAL_UINT32 || val->kind == CREX_FFI_VAL_UINT64) {
	} else if (val->kind == CREX_FFI_VAL_INT32 || val->kind == CREX_FFI_VAL_INT64) {
	} else if (val->kind == CREX_FFI_VAL_FLOAT || val->kind == CREX_FFI_VAL_DOUBLE || val->kind == CREX_FFI_VAL_LONG_DOUBLE) {
	} else if (val->kind == CREX_FFI_VAL_CHAR) {
	} else {
		val->kind = CREX_FFI_VAL_ERROR;
	}
}
/* }}} */

void crex_ffi_expr_neg(crex_ffi_val *val) /* {{{ */
{
	if (val->kind == CREX_FFI_VAL_UINT32 || val->kind == CREX_FFI_VAL_UINT64) {
		val->u64 = -val->u64;
	} else if (val->kind == CREX_FFI_VAL_INT32 || val->kind == CREX_FFI_VAL_INT64) {
		val->i64 = -val->i64;
	} else if (val->kind == CREX_FFI_VAL_FLOAT || val->kind == CREX_FFI_VAL_DOUBLE || val->kind == CREX_FFI_VAL_LONG_DOUBLE) {
		val->d = -val->d;
	} else if (val->kind == CREX_FFI_VAL_CHAR) {
		val->ch = -val->ch;
	} else {
		val->kind = CREX_FFI_VAL_ERROR;
	}
}
/* }}} */

void crex_ffi_expr_bw_not(crex_ffi_val *val) /* {{{ */
{
	if (val->kind == CREX_FFI_VAL_UINT32 || val->kind == CREX_FFI_VAL_UINT64) {
		val->u64 = ~val->u64;
	} else if (val->kind == CREX_FFI_VAL_INT32 || val->kind == CREX_FFI_VAL_INT64) {
		val->i64 = ~val->i64;
	} else if (val->kind == CREX_FFI_VAL_CHAR) {
		val->ch = ~val->ch;
	} else {
		val->kind = CREX_FFI_VAL_ERROR;
	}
}
/* }}} */

void crex_ffi_expr_bool_not(crex_ffi_val *val) /* {{{ */
{
	crex_ffi_expr_bool(val);
	if (val->kind == CREX_FFI_VAL_INT32) {
		val->i64 = !val->i64;
	}
}
/* }}} */

void crex_ffi_expr_sizeof_val(crex_ffi_val *val) /* {{{ */
{
	if (val->kind == CREX_FFI_VAL_UINT32 || val->kind == CREX_FFI_VAL_INT32) {
		val->kind = CREX_FFI_VAL_UINT32;
		val->u64 = crex_ffi_type_uint32.size;
	} else if (val->kind == CREX_FFI_VAL_UINT64 || val->kind == CREX_FFI_VAL_INT64) {
		val->kind = CREX_FFI_VAL_UINT32;
		val->u64 = crex_ffi_type_uint64.size;
	} else if (val->kind == CREX_FFI_VAL_FLOAT) {
		val->kind = CREX_FFI_VAL_UINT32;
		val->u64 = crex_ffi_type_float.size;
	} else if (val->kind == CREX_FFI_VAL_DOUBLE) {
		val->kind = CREX_FFI_VAL_UINT32;
		val->u64 = crex_ffi_type_double.size;
	} else if (val->kind == CREX_FFI_VAL_LONG_DOUBLE) {
		val->kind = CREX_FFI_VAL_UINT32;
#ifdef _WIN32
		val->u64 = crex_ffi_type_double.size;
#else
		val->u64 = crex_ffi_type_long_double.size;
#endif
	} else if (val->kind == CREX_FFI_VAL_CHAR) {
		val->kind = CREX_FFI_VAL_UINT32;
		val->u64 = crex_ffi_type_char.size;
	} else if (val->kind == CREX_FFI_VAL_STRING) {
		if (memchr(val->str, '\\', val->len)) {
			// TODO: support for escape sequences ???
			val->kind = CREX_FFI_VAL_ERROR;
		} else {
			val->kind = CREX_FFI_VAL_UINT32;
			val->u64 = val->len + 1;
		}
	} else {
		val->kind = CREX_FFI_VAL_ERROR;
	}
}
/* }}} */

void crex_ffi_expr_sizeof_type(crex_ffi_val *val, crex_ffi_dcl *dcl) /* {{{ */
{
	crex_ffi_type *type;

	crex_ffi_finalize_type(dcl);
	type = CREX_FFI_TYPE(dcl->type);
	val->kind = (type->size > 0xffffffff) ? CREX_FFI_VAL_UINT64 : CREX_FFI_VAL_UINT32;
	val->u64 = type->size;
	crex_ffi_type_dtor(dcl->type);
}
/* }}} */

void crex_ffi_expr_alignof_val(crex_ffi_val *val) /* {{{ */
{
	if (val->kind == CREX_FFI_VAL_UINT32 || val->kind == CREX_FFI_VAL_INT32) {
		val->kind = CREX_FFI_VAL_UINT32;
		val->u64 = crex_ffi_type_uint32.align;
	} else if (val->kind == CREX_FFI_VAL_UINT64 || val->kind == CREX_FFI_VAL_INT64) {
		val->kind = CREX_FFI_VAL_UINT32;
		val->u64 = crex_ffi_type_uint64.align;
	} else if (val->kind == CREX_FFI_VAL_FLOAT) {
		val->kind = CREX_FFI_VAL_UINT32;
		val->u64 = crex_ffi_type_float.align;
	} else if (val->kind == CREX_FFI_VAL_DOUBLE) {
		val->kind = CREX_FFI_VAL_UINT32;
		val->u64 = crex_ffi_type_double.align;
#ifdef HAVE_LONG_DOUBLE
	} else if (val->kind == CREX_FFI_VAL_LONG_DOUBLE) {
		val->kind = CREX_FFI_VAL_UINT32;
		val->u64 = crex_ffi_type_long_double.align;
#endif
	} else if (val->kind == CREX_FFI_VAL_CHAR) {
		val->kind = CREX_FFI_VAL_UINT32;
		val->u64 = crex_ffi_type_char.size;
	} else if (val->kind == CREX_FFI_VAL_STRING) {
		val->kind = CREX_FFI_VAL_UINT32;
		val->u64 = _Alignof(char*);
	} else {
		val->kind = CREX_FFI_VAL_ERROR;
	}
}
/* }}} */

void crex_ffi_expr_alignof_type(crex_ffi_val *val, crex_ffi_dcl *dcl) /* {{{ */
{
	crex_ffi_finalize_type(dcl);
	val->kind = CREX_FFI_VAL_UINT32;
	val->u64 = CREX_FFI_TYPE(dcl->type)->align;
	crex_ffi_type_dtor(dcl->type);
}
/* }}} */

void crex_ffi_val_number(crex_ffi_val *val, int base, const char *str, size_t str_len) /* {{{ */
{
	int u = 0;
	int l = 0;

	if (str[str_len-1] == 'u' || str[str_len-1] == 'U') {
		u = 1;
		if (str[str_len-2] == 'l' || str[str_len-2] == 'L') {
			l = 1;
			if (str[str_len-3] == 'l' || str[str_len-3] == 'L') {
				l = 2;
			}
		}
	} else if (str[str_len-1] == 'l' || str[str_len-1] == 'L') {
		l = 1;
		if (str[str_len-2] == 'l' || str[str_len-2] == 'L') {
			l = 2;
			if (str[str_len-3] == 'u' || str[str_len-3] == 'U') {
				u = 1;
			}
		} else if (str[str_len-2] == 'u' || str[str_len-2] == 'U') {
			u = 1;
		}
	}
	if (u) {
		val->u64 = strtoull(str, NULL, base);
		if (l == 0) {
			val->kind = CREX_FFI_VAL_UINT32;
		} else if (l == 1) {
			val->kind = (sizeof(long) == 4) ? CREX_FFI_VAL_UINT32 : CREX_FFI_VAL_UINT64;
		} else if (l == 2) {
			val->kind = CREX_FFI_VAL_UINT64;
		}
	} else {
		val->i64 = strtoll(str, NULL, base);
		if (l == 0) {
			val->kind = CREX_FFI_VAL_INT32;
		} else if (l == 1) {
			val->kind = (sizeof(long) == 4) ? CREX_FFI_VAL_INT32 : CREX_FFI_VAL_INT64;
		} else if (l == 2) {
			val->kind = CREX_FFI_VAL_INT64;
		}
	}
}
/* }}} */

void crex_ffi_val_float_number(crex_ffi_val *val, const char *str, size_t str_len) /* {{{ */
{
	val->d = strtold(str, NULL);
	if (str[str_len-1] == 'f' || str[str_len-1] == 'F') {
		val->kind = CREX_FFI_VAL_FLOAT;
	} else if (str[str_len-1] == 'l' || str[str_len-1] == 'L') {
		val->kind = CREX_FFI_VAL_LONG_DOUBLE;
	} else {
		val->kind = CREX_FFI_VAL_DOUBLE;
	}
}
/* }}} */

void crex_ffi_val_string(crex_ffi_val *val, const char *str, size_t str_len) /* {{{ */
{
	if (str[0] != '\"') {
		val->kind = CREX_FFI_VAL_ERROR;
	} else {
		val->kind = CREX_FFI_VAL_STRING;
		val->str = str + 1;
		val->len = str_len - 2;
	}
}
/* }}} */

void crex_ffi_val_character(crex_ffi_val *val, const char *str, size_t str_len) /* {{{ */
{
	int n;

	if (str[0] != '\'') {
		val->kind = CREX_FFI_VAL_ERROR;
	} else {
		val->kind = CREX_FFI_VAL_CHAR;
		if (str_len == 3) {
			val->ch = str[1];
		} else if (str[1] == '\\') {
			if (str[2] == 'a') {
			} else if (str[2] == 'b' && str_len == 4) {
				val->ch = '\b';
			} else if (str[2] == 'f' && str_len == 4) {
				val->ch = '\f';
			} else if (str[2] == 'n' && str_len == 4) {
				val->ch = '\n';
			} else if (str[2] == 'r' && str_len == 4) {
				val->ch = '\r';
			} else if (str[2] == 't' && str_len == 4) {
				val->ch = '\t';
			} else if (str[2] == 'v' && str_len == 4) {
				val->ch = '\v';
			} else if (str[2] >= '0' && str[2] <= '7') {
				n = str[2] - '0';
				if (str[3] >= '0' && str[3] <= '7') {
					n = n * 8 + (str[3] - '0');
					if ((str[4] >= '0' && str[4] <= '7') && str_len == 6) {
						n = n * 8 + (str[4] - '0');
					} else if (str_len != 5) {
						val->kind = CREX_FFI_VAL_ERROR;
					}
				} else if (str_len != 4) {
					val->kind = CREX_FFI_VAL_ERROR;
				}
				if (n <= 0xff) {
					val->ch = n;
				} else {
					val->kind = CREX_FFI_VAL_ERROR;
				}
			} else if (str[2] == 'x') {
				if (str[3] >= '0' && str[3] <= '9') {
					n = str[3] - '0';
				} else if (str[3] >= 'A' && str[3] <= 'F') {
					n = str[3] - 'A';
				} else if (str[3] >= 'a' && str[3] <= 'f') {
					n = str[3] - 'a';
				} else {
					val->kind = CREX_FFI_VAL_ERROR;
					return;
				}
				if ((str[4] >= '0' && str[4] <= '9') && str_len == 6) {
					n = n * 16 + (str[4] - '0');
				} else if ((str[4] >= 'A' && str[4] <= 'F') && str_len == 6) {
					n = n * 16 + (str[4] - 'A');
				} else if ((str[4] >= 'a' && str[4] <= 'f') && str_len == 6) {
					n = n * 16 + (str[4] - 'a');
				} else if (str_len != 5) {
					val->kind = CREX_FFI_VAL_ERROR;
					return;
				}
				val->ch = n;
			} else if (str_len == 4) {
				val->ch = str[2];
			} else {
				val->kind = CREX_FFI_VAL_ERROR;
			}
		} else {
			val->kind = CREX_FFI_VAL_ERROR;
		}
	}
}
/* }}} */
