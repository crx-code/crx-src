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

#ifndef CRX_FFI_H
#define CRX_FFI_H

extern crex_module_entry ffi_module_entry;
#define crxext_ffi_ptr &ffi_module_entry

typedef enum _crex_ffi_api_restriction {
	CREX_FFI_DISABLED = 0,  /* completely disabled */
	CREX_FFI_ENABLED = 1,   /* enabled everywhere */
	CREX_FFI_PRELOAD = 2,   /* enabled only in preloaded scripts and CLI */
} crex_ffi_api_restriction;

typedef struct _crex_ffi_type  crex_ffi_type;

CREX_BEGIN_MODULE_GLOBALS(ffi)
	crex_ffi_api_restriction restriction;
	bool is_cli;

	/* predefined ffi_types */
	HashTable types;

	/* preloading */
	char *preload;
	HashTable *scopes;           /* list of preloaded scopes */

	/* callbacks */
	HashTable *callbacks;

	/* weak type references */
	HashTable *weak_types;

	/* ffi_parser */
	JMP_BUF	bailout;
	unsigned const char *buf;
	unsigned const char *end;
	unsigned const char *pos;
	unsigned const char *text;
	int line;
	HashTable *symbols;
	HashTable *tags;
	bool allow_vla;
	bool attribute_parsing;
	bool persistent;
	uint32_t  default_type_attr;
CREX_END_MODULE_GLOBALS(ffi)

CREX_EXTERN_MODULE_GLOBALS(ffi)

#ifdef CRX_WIN32
# define CRX_FFI_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
# define CRX_FFI_API __attribute__ ((visibility("default")))
#else
# define CRX_FFI_API
#endif

#define FFI_G(v) CREX_MODULE_GLOBALS_ACCESSOR(ffi, v)

#define CREX_FFI_DCL_VOID            (1<<0)
#define CREX_FFI_DCL_CHAR            (1<<1)
#define CREX_FFI_DCL_SHORT           (1<<2)
#define CREX_FFI_DCL_INT             (1<<3)
#define CREX_FFI_DCL_LONG            (1<<4)
#define CREX_FFI_DCL_LONG_LONG       (1<<5)
#define CREX_FFI_DCL_FLOAT           (1<<6)
#define CREX_FFI_DCL_DOUBLE          (1<<7)
#define CREX_FFI_DCL_SIGNED          (1<<8)
#define CREX_FFI_DCL_UNSIGNED        (1<<9)
#define CREX_FFI_DCL_BOOL            (1<<10)
#define CREX_FFI_DCL_COMPLEX         (1<<11)

#define CREX_FFI_DCL_STRUCT          (1<<12)
#define CREX_FFI_DCL_UNION           (1<<13)
#define CREX_FFI_DCL_ENUM            (1<<14)
#define CREX_FFI_DCL_TYPEDEF_NAME    (1<<15)

#define CREX_FFI_DCL_TYPE_SPECIFIERS \
	(CREX_FFI_DCL_VOID|CREX_FFI_DCL_CHAR|CREX_FFI_DCL_SHORT \
	|CREX_FFI_DCL_INT|CREX_FFI_DCL_LONG|CREX_FFI_DCL_LONG_LONG \
	|CREX_FFI_DCL_FLOAT|CREX_FFI_DCL_DOUBLE|CREX_FFI_DCL_SIGNED \
	|CREX_FFI_DCL_UNSIGNED|CREX_FFI_DCL_BOOL|CREX_FFI_DCL_COMPLEX \
	|CREX_FFI_DCL_STRUCT|CREX_FFI_DCL_UNION|CREX_FFI_DCL_ENUM \
	|CREX_FFI_DCL_TYPEDEF_NAME)

#define CREX_FFI_DCL_TYPEDEF         (1<<16)
#define CREX_FFI_DCL_EXTERN          (1<<17)
#define CREX_FFI_DCL_STATIC          (1<<18)
#define CREX_FFI_DCL_AUTO            (1<<19)
#define CREX_FFI_DCL_REGISTER        (1<<20)

#define CREX_FFI_DCL_STORAGE_CLASS \
	(CREX_FFI_DCL_TYPEDEF|CREX_FFI_DCL_EXTERN|CREX_FFI_DCL_STATIC \
	|CREX_FFI_DCL_AUTO|CREX_FFI_DCL_REGISTER)

#define CREX_FFI_DCL_CONST           (1<<21)
#define CREX_FFI_DCL_RESTRICT        (1<<22)
#define CREX_FFI_DCL_VOLATILE        (1<<23)
#define CREX_FFI_DCL_ATOMIC          (1<<24)

#define CREX_FFI_DCL_TYPE_QUALIFIERS \
	(CREX_FFI_DCL_CONST|CREX_FFI_DCL_RESTRICT|CREX_FFI_DCL_VOLATILE \
	|CREX_FFI_DCL_ATOMIC)

#define CREX_FFI_DCL_INLINE          (1<<25)
#define CREX_FFI_DCL_NO_RETURN       (1<<26)

#define CREX_FFI_ABI_DEFAULT        0

#define CREX_FFI_ABI_CDECL          1  // FFI_DEFAULT_ABI
#define CREX_FFI_ABI_FASTCALL       2  // FFI_FASTCALL
#define CREX_FFI_ABI_THISCALL       3  // FFI_THISCALL
#define CREX_FFI_ABI_STDCALL        4  // FFI_STDCALL
#define	CREX_FFI_ABI_PASCAL         5  // FFI_PASCAL
#define	CREX_FFI_ABI_REGISTER       6  // FFI_REGISTER
#define	CREX_FFI_ABI_MS             7  // FFI_MS_CDECL
#define	CREX_FFI_ABI_SYSV           8  // FFI_SYSV
#define CREX_FFI_ABI_VECTORCALL     9  // FFI_VECTORCALL

#define CREX_FFI_ATTR_CONST             (1<<0)
#define CREX_FFI_ATTR_INCOMPLETE_TAG    (1<<1)
#define CREX_FFI_ATTR_VARIADIC          (1<<2)
#define CREX_FFI_ATTR_INCOMPLETE_ARRAY  (1<<3)
#define CREX_FFI_ATTR_VLA               (1<<4)
#define	CREX_FFI_ATTR_UNION             (1<<5)
#define	CREX_FFI_ATTR_PACKED            (1<<6)
#define	CREX_FFI_ATTR_MS_STRUCT         (1<<7)
#define	CREX_FFI_ATTR_GCC_STRUCT        (1<<8)

#define	CREX_FFI_ATTR_PERSISTENT        (1<<9)
#define	CREX_FFI_ATTR_STORED            (1<<10)

#define CREX_FFI_STRUCT_ATTRS \
	(CREX_FFI_ATTR_UNION|CREX_FFI_ATTR_PACKED|CREX_FFI_ATTR_MS_STRUCT \
	|CREX_FFI_ATTR_GCC_STRUCT)

#define CREX_FFI_ENUM_ATTRS \
	(CREX_FFI_ATTR_PACKED)

#define CREX_FFI_ARRAY_ATTRS \
	(CREX_FFI_ATTR_CONST|CREX_FFI_ATTR_VLA|CREX_FFI_ATTR_INCOMPLETE_ARRAY)

#define CREX_FFI_FUNC_ATTRS \
	(CREX_FFI_ATTR_VARIADIC)

#define CREX_FFI_POINTER_ATTRS \
	(CREX_FFI_ATTR_CONST)

typedef struct _crex_ffi_dcl {
	uint32_t       flags;
	uint32_t       align;
	uint16_t       attr;
	uint16_t       abi;
	crex_ffi_type *type;
} crex_ffi_dcl;

#define CREX_FFI_ATTR_INIT {0, 0, 0, 0, NULL}

typedef enum _crex_ffi_val_kind {
	CREX_FFI_VAL_EMPTY,
	CREX_FFI_VAL_ERROR,
	CREX_FFI_VAL_INT32,
	CREX_FFI_VAL_INT64,
	CREX_FFI_VAL_UINT32,
	CREX_FFI_VAL_UINT64,
	CREX_FFI_VAL_FLOAT,
	CREX_FFI_VAL_DOUBLE,
	CREX_FFI_VAL_LONG_DOUBLE,
	CREX_FFI_VAL_CHAR,
	CREX_FFI_VAL_STRING,
	CREX_FFI_VAL_NAME, /* attribute value */
} crex_ffi_val_kind;

#ifdef HAVE_LONG_DOUBLE
typedef long double crex_ffi_double;
#else
typedef double crex_ffi_double;
#endif

typedef struct _crex_ffi_val {
	crex_ffi_val_kind   kind;
	union {
		uint64_t        u64;
		int64_t         i64;
		crex_ffi_double d;
		signed char     ch;
		struct {
			const char *str;
			size_t      len;
		};
	};
} crex_ffi_val;

crex_result crex_ffi_parse_decl(const char *str, size_t len);
crex_result crex_ffi_parse_type(const char *str, size_t len, crex_ffi_dcl *dcl);

/* parser callbacks */
void CREX_NORETURN crex_ffi_parser_error(const char *msg, ...);
bool crex_ffi_is_typedef_name(const char *name, size_t name_len);
void crex_ffi_resolve_typedef(const char *name, size_t name_len, crex_ffi_dcl *dcl);
void crex_ffi_resolve_const(const char *name, size_t name_len, crex_ffi_val *val);
void crex_ffi_declare_tag(const char *name, size_t name_len, crex_ffi_dcl *dcl, bool incomplete);
void crex_ffi_make_enum_type(crex_ffi_dcl *dcl);
void crex_ffi_add_enum_val(crex_ffi_dcl *enum_dcl, const char *name, size_t name_len, crex_ffi_val *val, int64_t *min, int64_t *max, int64_t *last);
void crex_ffi_make_struct_type(crex_ffi_dcl *dcl);
void crex_ffi_add_field(crex_ffi_dcl *struct_dcl, const char *name, size_t name_len, crex_ffi_dcl *field_dcl);
void crex_ffi_add_anonymous_field(crex_ffi_dcl *struct_dcl, crex_ffi_dcl *field_dcl);
void crex_ffi_add_bit_field(crex_ffi_dcl *struct_dcl, const char *name, size_t name_len, crex_ffi_dcl *field_dcl, crex_ffi_val *bits);
void crex_ffi_adjust_struct_size(crex_ffi_dcl *dcl);
void crex_ffi_make_pointer_type(crex_ffi_dcl *dcl);
void crex_ffi_make_array_type(crex_ffi_dcl *dcl, crex_ffi_val *len);
void crex_ffi_make_func_type(crex_ffi_dcl *dcl, HashTable *args, crex_ffi_dcl *nested_dcl);
void crex_ffi_add_arg(HashTable **args, const char *name, size_t name_len, crex_ffi_dcl *arg_dcl);
void crex_ffi_declare(const char *name, size_t name_len, crex_ffi_dcl *dcl);
void crex_ffi_add_attribute(crex_ffi_dcl *dcl, const char *name, size_t name_len);
void crex_ffi_add_attribute_value(crex_ffi_dcl *dcl, const char *name, size_t name_len, int n, crex_ffi_val *val);
void crex_ffi_add_msvc_attribute_value(crex_ffi_dcl *dcl, const char *name, size_t name_len, crex_ffi_val *val);
void crex_ffi_set_abi(crex_ffi_dcl *dcl, uint16_t abi);
void crex_ffi_nested_declaration(crex_ffi_dcl *dcl, crex_ffi_dcl *nested_dcl);
void crex_ffi_align_as_type(crex_ffi_dcl *dcl, crex_ffi_dcl *align_dcl);
void crex_ffi_align_as_val(crex_ffi_dcl *dcl, crex_ffi_val *align_val);
void crex_ffi_validate_type_name(crex_ffi_dcl *dcl);

void crex_ffi_expr_conditional(crex_ffi_val *val, crex_ffi_val *op2, crex_ffi_val *op3);
void crex_ffi_expr_bool_or(crex_ffi_val *val, crex_ffi_val *op2);
void crex_ffi_expr_bool_and(crex_ffi_val *val, crex_ffi_val *op2);
void crex_ffi_expr_bw_or(crex_ffi_val *val, crex_ffi_val *op2);
void crex_ffi_expr_bw_xor(crex_ffi_val *val, crex_ffi_val *op2);
void crex_ffi_expr_bw_and(crex_ffi_val *val, crex_ffi_val *op2);
void crex_ffi_expr_is_equal(crex_ffi_val *val, crex_ffi_val *op2);
void crex_ffi_expr_is_not_equal(crex_ffi_val *val, crex_ffi_val *op2);
void crex_ffi_expr_is_less(crex_ffi_val *val, crex_ffi_val *op2);
void crex_ffi_expr_is_greater(crex_ffi_val *val, crex_ffi_val *op2);
void crex_ffi_expr_is_less_or_equal(crex_ffi_val *val, crex_ffi_val *op2);
void crex_ffi_expr_is_greater_or_equal(crex_ffi_val *val, crex_ffi_val *op2);
void crex_ffi_expr_shift_left(crex_ffi_val *val, crex_ffi_val *op2);
void crex_ffi_expr_shift_right(crex_ffi_val *val, crex_ffi_val *op2);
void crex_ffi_expr_add(crex_ffi_val *val, crex_ffi_val *op2);
void crex_ffi_expr_sub(crex_ffi_val *val, crex_ffi_val *op2);
void crex_ffi_expr_mul(crex_ffi_val *val, crex_ffi_val *op2);
void crex_ffi_expr_div(crex_ffi_val *val, crex_ffi_val *op2);
void crex_ffi_expr_mod(crex_ffi_val *val, crex_ffi_val *op2);
void crex_ffi_expr_cast(crex_ffi_val *val, crex_ffi_dcl *dcl);
void crex_ffi_expr_plus(crex_ffi_val *val);
void crex_ffi_expr_neg(crex_ffi_val *val);
void crex_ffi_expr_bw_not(crex_ffi_val *val);
void crex_ffi_expr_bool_not(crex_ffi_val *val);
void crex_ffi_expr_sizeof_val(crex_ffi_val *val);
void crex_ffi_expr_sizeof_type(crex_ffi_val *val, crex_ffi_dcl *dcl);
void crex_ffi_expr_alignof_val(crex_ffi_val *val);
void crex_ffi_expr_alignof_type(crex_ffi_val *val, crex_ffi_dcl *dcl);

static crex_always_inline void crex_ffi_val_error(crex_ffi_val *val) /* {{{ */
{
	val->kind = CREX_FFI_VAL_ERROR;
}
/* }}} */

void crex_ffi_val_number(crex_ffi_val *val, int base, const char *str, size_t str_len);
void crex_ffi_val_float_number(crex_ffi_val *val, const char *str, size_t str_len);
void crex_ffi_val_string(crex_ffi_val *val, const char *str, size_t str_len);
void crex_ffi_val_character(crex_ffi_val *val, const char *str, size_t str_len);

#endif	/* CRX_FFI_H */
