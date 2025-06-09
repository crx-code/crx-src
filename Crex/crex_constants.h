/*
   +----------------------------------------------------------------------+
   | Crex Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Crex Technologies Ltd. (http://www.crex.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_CONSTANTS_H
#define CREX_CONSTANTS_H

#include "crex_globals.h"

#define CONST_CS				0					/* No longer used -- always case sensitive */
#define CONST_PERSISTENT		(1<<0)				/* Persistent */
#define CONST_NO_FILE_CACHE		(1<<1)				/* Can't be saved in file cache */
#define CONST_DEPRECATED		(1<<2)				/* Deprecated */
#define CONST_OWNED				(1<<3)				/* constant should be destroyed together with class */

#define	CRX_USER_CONSTANT   0x7fffff /* a constant defined in user space */

typedef struct _crex_constant {
	zval value;
	crex_string *name;
} crex_constant;

#define CREX_CONSTANT_FLAGS(c) \
	(C_CONSTANT_FLAGS((c)->value) & 0xff)

#define CREX_CONSTANT_MODULE_NUMBER(c) \
	(C_CONSTANT_FLAGS((c)->value) >> 8)

#define CREX_CONSTANT_SET_FLAGS(c, _flags, _module_number) do { \
		C_CONSTANT_FLAGS((c)->value) = \
			((_flags) & 0xff) | ((_module_number) << 8); \
	} while (0)

#define REGISTER_NULL_CONSTANT(name, flags)  crex_register_null_constant((name), sizeof(name)-1, (flags), module_number)
#define REGISTER_BOOL_CONSTANT(name, bval, flags)  crex_register_bool_constant((name), sizeof(name)-1, (bval), (flags), module_number)
#define REGISTER_LONG_CONSTANT(name, lval, flags)  crex_register_long_constant((name), sizeof(name)-1, (lval), (flags), module_number)
#define REGISTER_DOUBLE_CONSTANT(name, dval, flags)  crex_register_double_constant((name), sizeof(name)-1, (dval), (flags), module_number)
#define REGISTER_STRING_CONSTANT(name, str, flags)  crex_register_string_constant((name), sizeof(name)-1, (str), (flags), module_number)
#define REGISTER_STRINGL_CONSTANT(name, str, len, flags)  crex_register_stringl_constant((name), sizeof(name)-1, (str), (len), (flags), module_number)

#define REGISTER_NS_NULL_CONSTANT(ns, name, flags)  crex_register_null_constant(CREX_NS_NAME(ns, name), sizeof(CREX_NS_NAME(ns, name))-1, (flags), module_number)
#define REGISTER_NS_BOOL_CONSTANT(ns, name, bval, flags)  crex_register_bool_constant(CREX_NS_NAME(ns, name), sizeof(CREX_NS_NAME(ns, name))-1, (bval), (flags), module_number)
#define REGISTER_NS_LONG_CONSTANT(ns, name, lval, flags)  crex_register_long_constant(CREX_NS_NAME(ns, name), sizeof(CREX_NS_NAME(ns, name))-1, (lval), (flags), module_number)
#define REGISTER_NS_DOUBLE_CONSTANT(ns, name, dval, flags)  crex_register_double_constant(CREX_NS_NAME(ns, name), sizeof(CREX_NS_NAME(ns, name))-1, (dval), (flags), module_number)
#define REGISTER_NS_STRING_CONSTANT(ns, name, str, flags)  crex_register_string_constant(CREX_NS_NAME(ns, name), sizeof(CREX_NS_NAME(ns, name))-1, (str), (flags), module_number)
#define REGISTER_NS_STRINGL_CONSTANT(ns, name, str, len, flags)  crex_register_stringl_constant(CREX_NS_NAME(ns, name), sizeof(CREX_NS_NAME(ns, name))-1, (str), (len), (flags), module_number)

#define REGISTER_MAIN_NULL_CONSTANT(name, flags)  crex_register_null_constant((name), sizeof(name)-1, (flags), 0)
#define REGISTER_MAIN_BOOL_CONSTANT(name, bval, flags)  crex_register_bool_constant((name), sizeof(name)-1, (bval), (flags), 0)
#define REGISTER_MAIN_LONG_CONSTANT(name, lval, flags)  crex_register_long_constant((name), sizeof(name)-1, (lval), (flags), 0)
#define REGISTER_MAIN_DOUBLE_CONSTANT(name, dval, flags)  crex_register_double_constant((name), sizeof(name)-1, (dval), (flags), 0)
#define REGISTER_MAIN_STRING_CONSTANT(name, str, flags)  crex_register_string_constant((name), sizeof(name)-1, (str), (flags), 0)
#define REGISTER_MAIN_STRINGL_CONSTANT(name, str, len, flags)  crex_register_stringl_constant((name), sizeof(name)-1, (str), (len), (flags), 0)

BEGIN_EXTERN_C()
void clean_module_constants(int module_number);
void free_crex_constant(zval *zv);
void crex_startup_constants(void);
void crex_shutdown_constants(void);
void crex_register_standard_constants(void);
CREX_API bool crex_verify_const_access(crex_class_constant *c, crex_class_entry *ce);
CREX_API zval *crex_get_constant(crex_string *name);
CREX_API zval *crex_get_constant_str(const char *name, size_t name_len);
CREX_API zval *crex_get_constant_ex(crex_string *name, crex_class_entry *scope, uint32_t flags);
CREX_API zval *crex_get_class_constant_ex(crex_string *class_name, crex_string *constant_name, crex_class_entry *scope, uint32_t flags);
CREX_API void crex_register_bool_constant(const char *name, size_t name_len, bool bval, int flags, int module_number);
CREX_API void crex_register_null_constant(const char *name, size_t name_len, int flags, int module_number);
CREX_API void crex_register_long_constant(const char *name, size_t name_len, crex_long lval, int flags, int module_number);
CREX_API void crex_register_double_constant(const char *name, size_t name_len, double dval, int flags, int module_number);
CREX_API void crex_register_string_constant(const char *name, size_t name_len, const char *strval, int flags, int module_number);
CREX_API void crex_register_stringl_constant(const char *name, size_t name_len, const char *strval, size_t strlen, int flags, int module_number);
CREX_API crex_result crex_register_constant(crex_constant *c);
#ifdef ZTS
void crex_copy_constants(HashTable *target, HashTable *source);
#endif

CREX_API crex_constant *_crex_get_special_const(const char *name, size_t name_len);

static crex_always_inline crex_constant *crex_get_special_const(
		const char *name, size_t name_len) {
	if (name_len == 4 || name_len == 5) {
		return _crex_get_special_const(name, name_len);
	}
	return NULL;
}

END_EXTERN_C()

#define CREX_CONSTANT_DTOR free_crex_constant

#endif
