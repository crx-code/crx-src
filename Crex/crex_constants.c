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

#include "crex.h"
#include "crex_constants.h"
#include "crex_exceptions.h"
#include "crex_execute.h"
#include "crex_variables.h"
#include "crex_operators.h"
#include "crex_globals.h"
#include "crex_API.h"
#include "crex_constants_arginfo.h"

/* Protection from recursive self-referencing class constants */
#define IS_CONSTANT_VISITED_MARK    0x80

#define IS_CONSTANT_VISITED(zv)     (C_CONSTANT_FLAGS_P(zv) & IS_CONSTANT_VISITED_MARK)
#define MARK_CONSTANT_VISITED(zv)   C_CONSTANT_FLAGS_P(zv) |= IS_CONSTANT_VISITED_MARK
#define RESET_CONSTANT_VISITED(zv)  C_CONSTANT_FLAGS_P(zv) &= ~IS_CONSTANT_VISITED_MARK

/* Use for special null/true/false constants. */
static crex_constant *null_const, *true_const, *false_const;

void free_crex_constant(zval *zv)
{
	crex_constant *c = C_PTR_P(zv);

	if (!(CREX_CONSTANT_FLAGS(c) & CONST_PERSISTENT)) {
		zval_ptr_dtor_nogc(&c->value);
		if (c->name) {
			crex_string_release_ex(c->name, 0);
		}
		efree(c);
	} else {
		zval_internal_ptr_dtor(&c->value);
		if (c->name) {
			crex_string_release_ex(c->name, 1);
		}
		free(c);
	}
}


#ifdef ZTS
static void copy_crex_constant(zval *zv)
{
	crex_constant *c = C_PTR_P(zv);

	CREX_ASSERT(CREX_CONSTANT_FLAGS(c) & CONST_PERSISTENT);
	C_PTR_P(zv) = pemalloc(sizeof(crex_constant), 1);
	memcpy(C_PTR_P(zv), c, sizeof(crex_constant));

	c = C_PTR_P(zv);
	c->name = crex_string_copy(c->name);
	if (C_TYPE(c->value) == IS_STRING) {
		C_STR(c->value) = crex_string_dup(C_STR(c->value), 1);
	}
}


void crex_copy_constants(HashTable *target, HashTable *source)
{
	crex_hash_copy(target, source, copy_crex_constant);
}
#endif


static int clean_module_constant(zval *el, void *arg)
{
	crex_constant *c = (crex_constant *)C_PTR_P(el);
	int module_number = *(int *)arg;

	if (CREX_CONSTANT_MODULE_NUMBER(c) == module_number) {
		return CREX_HASH_APPLY_REMOVE;
	} else {
		return CREX_HASH_APPLY_KEEP;
	}
}


void clean_module_constants(int module_number)
{
	crex_hash_apply_with_argument(EG(crex_constants), clean_module_constant, (void *) &module_number);
}

void crex_startup_constants(void)
{
	EG(crex_constants) = (HashTable *) malloc(sizeof(HashTable));
	crex_hash_init(EG(crex_constants), 128, NULL, CREX_CONSTANT_DTOR, 1);
}



void crex_register_standard_constants(void)
{
	register_crex_constants_symbols(0);

	true_const = crex_hash_str_find_ptr(EG(crex_constants), "TRUE", sizeof("TRUE")-1);
	false_const = crex_hash_str_find_ptr(EG(crex_constants), "FALSE", sizeof("FALSE")-1);
	null_const = crex_hash_str_find_ptr(EG(crex_constants), "NULL", sizeof("NULL")-1);
}


void crex_shutdown_constants(void)
{
	crex_hash_destroy(EG(crex_constants));
	free(EG(crex_constants));
}

CREX_API void crex_register_null_constant(const char *name, size_t name_len, int flags, int module_number)
{
	crex_constant c;

	ZVAL_NULL(&c.value);
	CREX_CONSTANT_SET_FLAGS(&c, flags, module_number);
	c.name = crex_string_init_interned(name, name_len, flags & CONST_PERSISTENT);
	crex_register_constant(&c);
}

CREX_API void crex_register_bool_constant(const char *name, size_t name_len, bool bval, int flags, int module_number)
{
	crex_constant c;

	ZVAL_BOOL(&c.value, bval);
	CREX_CONSTANT_SET_FLAGS(&c, flags, module_number);
	c.name = crex_string_init_interned(name, name_len, flags & CONST_PERSISTENT);
	crex_register_constant(&c);
}

CREX_API void crex_register_long_constant(const char *name, size_t name_len, crex_long lval, int flags, int module_number)
{
	crex_constant c;

	ZVAL_LONG(&c.value, lval);
	CREX_CONSTANT_SET_FLAGS(&c, flags, module_number);
	c.name = crex_string_init_interned(name, name_len, flags & CONST_PERSISTENT);
	crex_register_constant(&c);
}


CREX_API void crex_register_double_constant(const char *name, size_t name_len, double dval, int flags, int module_number)
{
	crex_constant c;

	ZVAL_DOUBLE(&c.value, dval);
	CREX_CONSTANT_SET_FLAGS(&c, flags, module_number);
	c.name = crex_string_init_interned(name, name_len, flags & CONST_PERSISTENT);
	crex_register_constant(&c);
}


CREX_API void crex_register_stringl_constant(const char *name, size_t name_len, const char *strval, size_t strlen, int flags, int module_number)
{
	crex_constant c;

	ZVAL_STR(&c.value, crex_string_init_interned(strval, strlen, flags & CONST_PERSISTENT));
	CREX_CONSTANT_SET_FLAGS(&c, flags, module_number);
	c.name = crex_string_init_interned(name, name_len, flags & CONST_PERSISTENT);
	crex_register_constant(&c);
}


CREX_API void crex_register_string_constant(const char *name, size_t name_len, const char *strval, int flags, int module_number)
{
	crex_register_stringl_constant(name, name_len, strval, strlen(strval), flags, module_number);
}

static crex_constant *crex_get_halt_offset_constant(const char *name, size_t name_len)
{
	crex_constant *c;
	static const char haltoff[] = "__COMPILER_HALT_OFFSET__";

	if (!EG(current_execute_data)) {
		return NULL;
	} else if (name_len == sizeof("__COMPILER_HALT_OFFSET__")-1 &&
	          !memcmp(name, "__COMPILER_HALT_OFFSET__", sizeof("__COMPILER_HALT_OFFSET__")-1)) {
		const char *cfilename;
		crex_string *haltname;
		size_t clen;

		cfilename = crex_get_executed_filename();
		clen = strlen(cfilename);
		/* check for __COMPILER_HALT_OFFSET__ */
		haltname = crex_mangle_property_name(haltoff,
			sizeof("__COMPILER_HALT_OFFSET__") - 1, cfilename, clen, 0);
		c = crex_hash_find_ptr(EG(crex_constants), haltname);
		crex_string_efree(haltname);
		return c;
	} else {
		return NULL;
	}
}

CREX_API crex_constant *_crex_get_special_const(const char *name, size_t len) /* {{{ */
{
	if (len == 4) {
		if ((name[0] == 'n' || name[0] == 'N') &&
			(name[1] == 'u' || name[1] == 'U') &&
			(name[2] == 'l' || name[2] == 'L') &&
			(name[3] == 'l' || name[3] == 'L')
		) {
			return null_const;
		}
		if ((name[0] == 't' || name[0] == 'T') &&
			(name[1] == 'r' || name[1] == 'R') &&
			(name[2] == 'u' || name[2] == 'U') &&
			(name[3] == 'e' || name[3] == 'E')
		) {
			return true_const;
		}
	} else {
		if ((name[0] == 'f' || name[0] == 'F') &&
			(name[1] == 'a' || name[1] == 'A') &&
			(name[2] == 'l' || name[2] == 'L') &&
			(name[3] == 's' || name[3] == 'S') &&
			(name[4] == 'e' || name[4] == 'E')
		) {
			return false_const;
		}
	}
	return NULL;
}
/* }}} */

CREX_API bool crex_verify_const_access(crex_class_constant *c, crex_class_entry *scope) /* {{{ */
{
	if (CREX_CLASS_CONST_FLAGS(c) & CREX_ACC_PUBLIC) {
		return 1;
	} else if (CREX_CLASS_CONST_FLAGS(c) & CREX_ACC_PRIVATE) {
		return (c->ce == scope);
	} else {
		CREX_ASSERT(CREX_CLASS_CONST_FLAGS(c) & CREX_ACC_PROTECTED);
		return crex_check_protected(c->ce, scope);
	}
}
/* }}} */

static crex_constant *crex_get_constant_str_impl(const char *name, size_t name_len)
{
	crex_constant *c = crex_hash_str_find_ptr(EG(crex_constants), name, name_len);
	if (c) {
		return c;
	}

	c = crex_get_halt_offset_constant(name, name_len);
	if (c) {
		return c;
	}

	return crex_get_special_const(name, name_len);
}

CREX_API zval *crex_get_constant_str(const char *name, size_t name_len)
{
	crex_constant *c = crex_get_constant_str_impl(name, name_len);
	if (c) {
		return &c->value;
	}
	return NULL;
}

static crex_constant *crex_get_constant_impl(crex_string *name)
{
	crex_constant *c = crex_hash_find_ptr(EG(crex_constants), name);
	if (c) {
		return c;
	}

	c = crex_get_halt_offset_constant(ZSTR_VAL(name), ZSTR_LEN(name));
	if (c) {
		return c;
	}

	return crex_get_special_const(ZSTR_VAL(name), ZSTR_LEN(name));
}

CREX_API zval *crex_get_constant(crex_string *name)
{
	crex_constant *c = crex_get_constant_impl(name);
	if (c) {
		return &c->value;
	}
	return NULL;
}

CREX_API zval *crex_get_class_constant_ex(crex_string *class_name, crex_string *constant_name, crex_class_entry *scope, uint32_t flags)
{
	crex_class_entry *ce = NULL;
	crex_class_constant *c = NULL;
	zval *ret_constant = NULL;

	if (ZSTR_HAS_CE_CACHE(class_name)) {
		ce = ZSTR_GET_CE_CACHE(class_name);
		if (!ce) {
			ce = crex_fetch_class(class_name, flags);
		}
	} else if (crex_string_equals_literal_ci(class_name, "self")) {
		if (UNEXPECTED(!scope)) {
			crex_throw_error(NULL, "Cannot access \"self\" when no class scope is active");
			goto failure;
		}
		ce = scope;
	} else if (crex_string_equals_literal_ci(class_name, "parent")) {
		if (UNEXPECTED(!scope)) {
			crex_throw_error(NULL, "Cannot access \"parent\" when no class scope is active");
			goto failure;
		} else if (UNEXPECTED(!scope->parent)) {
			crex_throw_error(NULL, "Cannot access \"parent\" when current class scope has no parent");
			goto failure;
		} else {
			ce = scope->parent;
		}
	} else if (crex_string_equals_ci(class_name, ZSTR_KNOWN(CREX_STR_STATIC))) {
		ce = crex_get_called_scope(EG(current_execute_data));
		if (UNEXPECTED(!ce)) {
			crex_throw_error(NULL, "Cannot access \"static\" when no class scope is active");
			goto failure;
		}
	} else {
		ce = crex_fetch_class(class_name, flags);
	}
	if (ce) {
		c = crex_hash_find_ptr(CE_CONSTANTS_TABLE(ce), constant_name);
		if (c == NULL) {
			if ((flags & CREX_FETCH_CLASS_SILENT) == 0) {
				crex_throw_error(NULL, "Undefined constant %s::%s", ZSTR_VAL(class_name), ZSTR_VAL(constant_name));
				goto failure;
			}
			ret_constant = NULL;
		} else {
			if (!crex_verify_const_access(c, scope)) {
				if ((flags & CREX_FETCH_CLASS_SILENT) == 0) {
					crex_throw_error(NULL, "Cannot access %s constant %s::%s", crex_visibility_string(CREX_CLASS_CONST_FLAGS(c)), ZSTR_VAL(class_name), ZSTR_VAL(constant_name));
				}
				goto failure;
			}

			if (UNEXPECTED(ce->ce_flags & CREX_ACC_TRAIT)) {
				/** Prevent accessing trait constants directly on cases like \defined() or \constant(), etc. */
				if ((flags & CREX_FETCH_CLASS_SILENT) == 0) {
					crex_throw_error(NULL, "Cannot access trait constant %s::%s directly", ZSTR_VAL(class_name), ZSTR_VAL(constant_name));
				}
				goto failure;
			}

			if (UNEXPECTED(CREX_CLASS_CONST_FLAGS(c) & CREX_ACC_DEPRECATED)) {
				if ((flags & CREX_FETCH_CLASS_SILENT) == 0) {
					crex_error(E_DEPRECATED, "Constant %s::%s is deprecated", ZSTR_VAL(class_name), ZSTR_VAL(constant_name));
					if (EG(exception)) {
						goto failure;
					}
				}
			}
			ret_constant = &c->value;
		}
	}

	if (ret_constant && C_TYPE_P(ret_constant) == IS_CONSTANT_AST) {
		crex_result ret;

		if (IS_CONSTANT_VISITED(ret_constant)) {
			crex_throw_error(NULL, "Cannot declare self-referencing constant %s::%s", ZSTR_VAL(class_name), ZSTR_VAL(constant_name));
			ret_constant = NULL;
			goto failure;
		}

		MARK_CONSTANT_VISITED(ret_constant);
		ret = crex_update_class_constant(c, constant_name, c->ce);
		RESET_CONSTANT_VISITED(ret_constant);

		if (UNEXPECTED(ret != SUCCESS)) {
			ret_constant = NULL;
			goto failure;
		}
	}
failure:
	return ret_constant;
}

CREX_API zval *crex_get_constant_ex(crex_string *cname, crex_class_entry *scope, uint32_t flags)
{
	crex_constant *c;
	const char *colon;
	const char *name = ZSTR_VAL(cname);
	size_t name_len = ZSTR_LEN(cname);

	/* Skip leading \\ */
	if (name[0] == '\\') {
		name += 1;
		name_len -= 1;
		cname = NULL;
	}

	if ((colon = crex_memrchr(name, ':', name_len)) &&
	    colon > name && (*(colon - 1) == ':')) {
		int class_name_len = colon - name - 1;
		size_t const_name_len = name_len - class_name_len - 2;
		crex_string *constant_name = crex_string_init(colon + 1, const_name_len, 0);
		crex_string *class_name = crex_string_init_interned(name, class_name_len, 0);
		zval *ret_constant = crex_get_class_constant_ex(class_name, constant_name, scope, flags);

		crex_string_release_ex(class_name, 0);
		crex_string_efree(constant_name);
		return ret_constant;
/*
		crex_class_entry *ce = NULL;
		crex_class_constant *c = NULL;
		zval *ret_constant = NULL;

		if (crex_string_equals_literal_ci(class_name, "self")) {
			if (UNEXPECTED(!scope)) {
				crex_throw_error(NULL, "Cannot access \"self\" when no class scope is active");
				goto failure;
			}
			ce = scope;
		} else if (crex_string_equals_literal_ci(class_name, "parent")) {
			if (UNEXPECTED(!scope)) {
				crex_throw_error(NULL, "Cannot access \"parent\" when no class scope is active");
				goto failure;
			} else if (UNEXPECTED(!scope->parent)) {
				crex_throw_error(NULL, "Cannot access \"parent\" when current class scope has no parent");
				goto failure;
			} else {
				ce = scope->parent;
			}
		} else if (crex_string_equals_ci(class_name, ZSTR_KNOWN(CREX_STR_STATIC))) {
			ce = crex_get_called_scope(EG(current_execute_data));
			if (UNEXPECTED(!ce)) {
				crex_throw_error(NULL, "Cannot access \"static\" when no class scope is active");
				goto failure;
			}
		} else {
			ce = crex_fetch_class(class_name, flags);
		}
		if (ce) {
			c = crex_hash_find_ptr(CE_CONSTANTS_TABLE(ce), constant_name);
			if (c == NULL) {
				if ((flags & CREX_FETCH_CLASS_SILENT) == 0) {
					crex_throw_error(NULL, "Undefined constant %s::%s", ZSTR_VAL(class_name), ZSTR_VAL(constant_name));
					goto failure;
				}
				ret_constant = NULL;
			} else {
				if (!crex_verify_const_access(c, scope)) {
					if ((flags & CREX_FETCH_CLASS_SILENT) == 0) {
						crex_throw_error(NULL, "Cannot access %s constant %s::%s", crex_visibility_string(CREX_CLASS_CONST_FLAGS(c)), ZSTR_VAL(class_name), ZSTR_VAL(constant_name));
					}
					goto failure;
				}
				ret_constant = &c->value;
			}
		}

		if (ret_constant && C_TYPE_P(ret_constant) == IS_CONSTANT_AST) {
			crex_result ret;

			if (IS_CONSTANT_VISITED(ret_constant)) {
				crex_throw_error(NULL, "Cannot declare self-referencing constant %s::%s", ZSTR_VAL(class_name), ZSTR_VAL(constant_name));
				ret_constant = NULL;
				goto failure;
			}

			MARK_CONSTANT_VISITED(ret_constant);
			ret = zval_update_constant_ex(ret_constant, c->ce);
			RESET_CONSTANT_VISITED(ret_constant);

			if (UNEXPECTED(ret != SUCCESS)) {
				ret_constant = NULL;
				goto failure;
			}
		}
failure:
		crex_string_release_ex(class_name, 0);
		crex_string_efree(constant_name);
		return ret_constant;
*/
	}

	/* non-class constant */
	if ((colon = crex_memrchr(name, '\\', name_len)) != NULL) {
		/* compound constant name */
		int prefix_len = colon - name;
		size_t const_name_len = name_len - prefix_len - 1;
		const char *constant_name = colon + 1;
		char *lcname;
		size_t lcname_len;
		ALLOCA_FLAG(use_heap)

		/* Lowercase the namespace portion */
		lcname_len = prefix_len + 1 + const_name_len;
		lcname = do_alloca(lcname_len + 1, use_heap);
		crex_str_tolower_copy(lcname, name, prefix_len);

		lcname[prefix_len] = '\\';
		memcpy(lcname + prefix_len + 1, constant_name, const_name_len + 1);

		c = crex_hash_str_find_ptr(EG(crex_constants), lcname, lcname_len);
		free_alloca(lcname, use_heap);

		if (!c) {
			if (flags & IS_CONSTANT_UNQUALIFIED_IN_NAMESPACE) {
				/* name requires runtime resolution, need to check non-namespaced name */
				c = crex_get_constant_str_impl(constant_name, const_name_len);
			}
		}
	} else {
		if (cname) {
			c = crex_get_constant_impl(cname);
		} else {
			c = crex_get_constant_str_impl(name, name_len);
		}
	}

	if (!c) {
		if (!(flags & CREX_FETCH_CLASS_SILENT)) {
			crex_throw_error(NULL, "Undefined constant \"%s\"", name);
		}
		return NULL;
	}

	if (!(flags & CREX_FETCH_CLASS_SILENT) && (CREX_CONSTANT_FLAGS(c) & CONST_DEPRECATED)) {
		crex_error(E_DEPRECATED, "Constant %s is deprecated", name);
	}
	return &c->value;
}

static void* crex_hash_add_constant(HashTable *ht, crex_string *key, crex_constant *c)
{
	void *ret;
	crex_constant *copy = pemalloc(sizeof(crex_constant), CREX_CONSTANT_FLAGS(c) & CONST_PERSISTENT);

	memcpy(copy, c, sizeof(crex_constant));
	ret = crex_hash_add_ptr(ht, key, copy);
	if (!ret) {
		pefree(copy, CREX_CONSTANT_FLAGS(c) & CONST_PERSISTENT);
	}
	return ret;
}

CREX_API crex_result crex_register_constant(crex_constant *c)
{
	crex_string *lowercase_name = NULL;
	crex_string *name;
	crex_result ret = SUCCESS;
	bool persistent = (CREX_CONSTANT_FLAGS(c) & CONST_PERSISTENT) != 0;

#if 0
	printf("Registering constant for module %d\n", c->module_number);
#endif

	const char *slash = strrchr(ZSTR_VAL(c->name), '\\');
	if (slash) {
		lowercase_name = crex_string_init(ZSTR_VAL(c->name), ZSTR_LEN(c->name), persistent);
		crex_str_tolower(ZSTR_VAL(lowercase_name), slash - ZSTR_VAL(c->name));
		lowercase_name = crex_new_interned_string(lowercase_name);
		name = lowercase_name;
	} else {
		name = c->name;
	}

	/* Check if the user is trying to define any special constant */
	if (crex_string_equals_literal(name, "__COMPILER_HALT_OFFSET__")
		|| (!persistent && crex_get_special_const(ZSTR_VAL(name), ZSTR_LEN(name)))
		|| crex_hash_add_constant(EG(crex_constants), name, c) == NULL
	) {
		crex_error(E_WARNING, "Constant %s already defined", ZSTR_VAL(name));
		crex_string_release(c->name);
		if (!persistent) {
			zval_ptr_dtor_nogc(&c->value);
		}
		ret = FAILURE;
	}
	if (lowercase_name) {
		crex_string_release(lowercase_name);
	}
	return ret;
}
