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
   | Authors: Ilija Tovilo <ilutov@crx.net>                               |
   +----------------------------------------------------------------------+
*/

#include "crex.h"
#include "crex_API.h"
#include "crex_compile.h"
#include "crex_enum_arginfo.h"
#include "crex_interfaces.h"
#include "crex_enum.h"
#include "crex_extensions.h"
#include "crex_observer.h"

#define CREX_ENUM_DISALLOW_MAGIC_METHOD(propertyName, methodName) \
	do { \
		if (ce->propertyName) { \
			crex_error_noreturn(E_COMPILE_ERROR, "Enum %s cannot include magic method %s", ZSTR_VAL(ce->name), methodName); \
		} \
	} while (0);

CREX_API crex_class_entry *crex_ce_unit_enum;
CREX_API crex_class_entry *crex_ce_backed_enum;
CREX_API crex_object_handlers crex_enum_object_handlers;

crex_object *crex_enum_new(zval *result, crex_class_entry *ce, crex_string *case_name, zval *backing_value_zv)
{
	crex_object *zobj = crex_objects_new(ce);
	ZVAL_OBJ(result, zobj);

	zval *zname = OBJ_PROP_NUM(zobj, 0);
	ZVAL_STR_COPY(zname, case_name);
	/* ZVAL_COPY does not set C_PROP_FLAG, this needs to be cleared to avoid leaving IS_PROP_REINITABLE set */
	C_PROP_FLAG_P(zname) = 0;

	if (backing_value_zv != NULL) {
		zval *prop = OBJ_PROP_NUM(zobj, 1);

		ZVAL_COPY(prop, backing_value_zv);
		/* ZVAL_COPY does not set C_PROP_FLAG, this needs to be cleared to avoid leaving IS_PROP_REINITABLE set */
		C_PROP_FLAG_P(prop) = 0;
	}

	return zobj;
}

static void crex_verify_enum_properties(crex_class_entry *ce)
{
	crex_property_info *property_info;

	CREX_HASH_MAP_FOREACH_PTR(&ce->properties_info, property_info) {
		if (crex_string_equals(property_info->name, ZSTR_KNOWN(CREX_STR_NAME))) {
			continue;
		}
		if (
			ce->enum_backing_type != IS_UNDEF
			&& crex_string_equals(property_info->name, ZSTR_KNOWN(CREX_STR_VALUE))
		) {
			continue;
		}
		// FIXME: File/line number for traits?
		crex_error_noreturn(E_COMPILE_ERROR, "Enum %s cannot include properties",
			ZSTR_VAL(ce->name));
	} CREX_HASH_FOREACH_END();
}

static void crex_verify_enum_magic_methods(crex_class_entry *ce)
{
	// Only __get, __call and __invoke are allowed

	CREX_ENUM_DISALLOW_MAGIC_METHOD(constructor, "__main");
	CREX_ENUM_DISALLOW_MAGIC_METHOD(destructor, "__destruct");
	CREX_ENUM_DISALLOW_MAGIC_METHOD(clone, "__clone");
	CREX_ENUM_DISALLOW_MAGIC_METHOD(__get, "__get");
	CREX_ENUM_DISALLOW_MAGIC_METHOD(__set, "__set");
	CREX_ENUM_DISALLOW_MAGIC_METHOD(__unset, "__unset");
	CREX_ENUM_DISALLOW_MAGIC_METHOD(__isset, "__isset");
	CREX_ENUM_DISALLOW_MAGIC_METHOD(__tostring, "__toString");
	CREX_ENUM_DISALLOW_MAGIC_METHOD(__debugInfo, "__debugInfo");
	CREX_ENUM_DISALLOW_MAGIC_METHOD(__serialize, "__serialize");
	CREX_ENUM_DISALLOW_MAGIC_METHOD(__unserialize, "__unserialize");

	static const char *const forbidden_methods[] = {
		"__sleep",
		"__wakeup",
		"__set_state",
	};

	uint32_t forbidden_methods_length = sizeof(forbidden_methods) / sizeof(forbidden_methods[0]);
	for (uint32_t i = 0; i < forbidden_methods_length; ++i) {
		const char *forbidden_method = forbidden_methods[i];

		if (crex_hash_str_exists(&ce->function_table, forbidden_method, strlen(forbidden_method))) {
			crex_error_noreturn(E_COMPILE_ERROR, "Enum %s cannot include magic method %s", ZSTR_VAL(ce->name), forbidden_method);
		}
	}
}

static void crex_verify_enum_interfaces(crex_class_entry *ce)
{
	if (crex_class_implements_interface(ce, crex_ce_serializable)) {
		crex_error_noreturn(E_COMPILE_ERROR,
			"Enum %s cannot implement the Serializable interface", ZSTR_VAL(ce->name));
	}
}

void crex_verify_enum(crex_class_entry *ce)
{
	crex_verify_enum_properties(ce);
	crex_verify_enum_magic_methods(ce);
	crex_verify_enum_interfaces(ce);
}

static int crex_implement_unit_enum(crex_class_entry *interface, crex_class_entry *class_type)
{
	if (class_type->ce_flags & CREX_ACC_ENUM) {
		return SUCCESS;
	}

	crex_error_noreturn(E_ERROR, "Non-enum class %s cannot implement interface %s",
		ZSTR_VAL(class_type->name),
		ZSTR_VAL(interface->name));

	return FAILURE;
}

static int crex_implement_backed_enum(crex_class_entry *interface, crex_class_entry *class_type)
{
	if (!(class_type->ce_flags & CREX_ACC_ENUM)) {
		crex_error_noreturn(E_ERROR, "Non-enum class %s cannot implement interface %s",
			ZSTR_VAL(class_type->name),
			ZSTR_VAL(interface->name));
		return FAILURE;
	}

	if (class_type->enum_backing_type == IS_UNDEF) {
		crex_error_noreturn(E_ERROR, "Non-backed enum %s cannot implement interface %s",
			ZSTR_VAL(class_type->name),
			ZSTR_VAL(interface->name));
		return FAILURE;
	}

	return SUCCESS;
}

void crex_register_enum_ce(void)
{
	crex_ce_unit_enum = register_class_UnitEnum();
	crex_ce_unit_enum->interface_gets_implemented = crex_implement_unit_enum;

	crex_ce_backed_enum = register_class_BackedEnum(crex_ce_unit_enum);
	crex_ce_backed_enum->interface_gets_implemented = crex_implement_backed_enum;

	memcpy(&crex_enum_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	crex_enum_object_handlers.clone_obj = NULL;
	crex_enum_object_handlers.compare = crex_objects_not_comparable;
}

void crex_enum_add_interfaces(crex_class_entry *ce)
{
	uint32_t num_interfaces_before = ce->num_interfaces;

	ce->num_interfaces++;
	if (ce->enum_backing_type != IS_UNDEF) {
		ce->num_interfaces++;
	}

	CREX_ASSERT(!(ce->ce_flags & CREX_ACC_RESOLVED_INTERFACES));

	ce->interface_names = erealloc(ce->interface_names, sizeof(crex_class_name) * ce->num_interfaces);

	ce->interface_names[num_interfaces_before].name = crex_string_copy(crex_ce_unit_enum->name);
	ce->interface_names[num_interfaces_before].lc_name = ZSTR_INIT_LITERAL("unitenum", 0);

	if (ce->enum_backing_type != IS_UNDEF) {
		ce->interface_names[num_interfaces_before + 1].name = crex_string_copy(crex_ce_backed_enum->name);
		ce->interface_names[num_interfaces_before + 1].lc_name = ZSTR_INIT_LITERAL("backedenum", 0);
	}

	ce->default_object_handlers = &crex_enum_object_handlers;
}

crex_result crex_enum_build_backed_enum_table(crex_class_entry *ce)
{
	CREX_ASSERT(ce->ce_flags & CREX_ACC_ENUM);
	CREX_ASSERT(ce->type == CREX_USER_CLASS);

	uint32_t backing_type = ce->enum_backing_type;
	CREX_ASSERT(backing_type != IS_UNDEF);

	HashTable *backed_enum_table = emalloc(sizeof(HashTable));
	crex_hash_init(backed_enum_table, 0, NULL, ZVAL_PTR_DTOR, 0);
	crex_class_set_backed_enum_table(ce, backed_enum_table);

	crex_string *enum_class_name = ce->name;

	crex_string *name;
	zval *val;
	CREX_HASH_MAP_FOREACH_STR_KEY_VAL(CE_CONSTANTS_TABLE(ce), name, val) {
		crex_class_constant *c = C_PTR_P(val);
		if ((CREX_CLASS_CONST_FLAGS(c) & CREX_CLASS_CONST_IS_CASE) == 0) {
			continue;
		}

		zval *c_value = &c->value;
		zval *case_name = crex_enum_fetch_case_name(C_OBJ_P(c_value));
		zval *case_value = crex_enum_fetch_case_value(C_OBJ_P(c_value));

		if (ce->enum_backing_type != C_TYPE_P(case_value)) {
			crex_type_error("Enum case type %s does not match enum backing type %s",
				crex_get_type_by_const(C_TYPE_P(case_value)),
				crex_get_type_by_const(ce->enum_backing_type));
			goto failure;
		}

		if (ce->enum_backing_type == IS_LONG) {
			crex_long long_key = C_LVAL_P(case_value);
			zval *existing_case_name = crex_hash_index_find(backed_enum_table, long_key);
			if (existing_case_name) {
				crex_throw_error(NULL, "Duplicate value in enum %s for cases %s and %s",
					ZSTR_VAL(enum_class_name),
					C_STRVAL_P(existing_case_name),
					ZSTR_VAL(name));
				goto failure;
			}
			C_TRY_ADDREF_P(case_name);
			crex_hash_index_add_new(backed_enum_table, long_key, case_name);
		} else {
			CREX_ASSERT(ce->enum_backing_type == IS_STRING);
			crex_string *string_key = C_STR_P(case_value);
			zval *existing_case_name = crex_hash_find(backed_enum_table, string_key);
			if (existing_case_name != NULL) {
				crex_throw_error(NULL, "Duplicate value in enum %s for cases %s and %s",
					ZSTR_VAL(enum_class_name),
					C_STRVAL_P(existing_case_name),
					ZSTR_VAL(name));
				goto failure;
			}
			C_TRY_ADDREF_P(case_name);
			crex_hash_add_new(backed_enum_table, string_key, case_name);
		}
	} CREX_HASH_FOREACH_END();

	return SUCCESS;

failure:
	crex_hash_release(backed_enum_table);
	crex_class_set_backed_enum_table(ce, NULL);
	return FAILURE;
}

static CREX_NAMED_FUNCTION(crex_enum_cases_func)
{
	crex_class_entry *ce = execute_data->func->common.scope;
	crex_class_constant *c;

	CREX_PARSE_PARAMETERS_NONE();

	array_init(return_value);

	CREX_HASH_MAP_FOREACH_PTR(CE_CONSTANTS_TABLE(ce), c) {
		if (!(CREX_CLASS_CONST_FLAGS(c) & CREX_CLASS_CONST_IS_CASE)) {
			continue;
		}
		zval *zv = &c->value;
		if (C_TYPE_P(zv) == IS_CONSTANT_AST) {
			if (zval_update_constant_ex(zv, c->ce) == FAILURE) {
				RETURN_THROWS();
			}
		}
		C_ADDREF_P(zv);
		crex_hash_next_index_insert_new(C_ARRVAL_P(return_value), zv);
	} CREX_HASH_FOREACH_END();
}

CREX_API crex_result crex_enum_get_case_by_value(crex_object **result, crex_class_entry *ce, crex_long long_key, crex_string *string_key, bool try)
{
	if (ce->type == CREX_USER_CLASS && !(ce->ce_flags & CREX_ACC_CONSTANTS_UPDATED)) {
		if (crex_update_class_constants(ce) == FAILURE) {
			return FAILURE;
		}
	}

	HashTable *backed_enum_table = CE_BACKED_ENUM_TABLE(ce);
	if (!backed_enum_table) {
		goto not_found;
	}

	zval *case_name_zv;
	if (ce->enum_backing_type == IS_LONG) {
		case_name_zv = crex_hash_index_find(backed_enum_table, long_key);
	} else {
		CREX_ASSERT(ce->enum_backing_type == IS_STRING);
		CREX_ASSERT(string_key != NULL);
		case_name_zv = crex_hash_find(backed_enum_table, string_key);
	}

	if (case_name_zv == NULL) {
not_found:
		if (try) {
			*result = NULL;
			return SUCCESS;
		}

		if (ce->enum_backing_type == IS_LONG) {
			crex_value_error(CREX_LONG_FMT " is not a valid backing value for enum %s", long_key, ZSTR_VAL(ce->name));
		} else {
			CREX_ASSERT(ce->enum_backing_type == IS_STRING);
			crex_value_error("\"%s\" is not a valid backing value for enum %s", ZSTR_VAL(string_key), ZSTR_VAL(ce->name));
		}
		return FAILURE;
	}

	// TODO: We might want to store pointers to constants in backed_enum_table instead of names,
	// to make this lookup more efficient.
	CREX_ASSERT(C_TYPE_P(case_name_zv) == IS_STRING);
	crex_class_constant *c = crex_hash_find_ptr(CE_CONSTANTS_TABLE(ce), C_STR_P(case_name_zv));
	CREX_ASSERT(c != NULL);
	zval *case_zv = &c->value;
	if (C_TYPE_P(case_zv) == IS_CONSTANT_AST) {
		if (zval_update_constant_ex(case_zv, c->ce) == FAILURE) {
			return FAILURE;
		}
	}

	*result = C_OBJ_P(case_zv);
	return SUCCESS;
}

static void crex_enum_from_base(INTERNAL_FUNCTION_PARAMETERS, bool try)
{
	crex_class_entry *ce = execute_data->func->common.scope;
	bool release_string = false;
	crex_string *string_key = NULL;
	crex_long long_key = 0;

	if (ce->enum_backing_type == IS_LONG) {
		CREX_PARSE_PARAMETERS_START(1, 1)
			C_PARAM_LONG(long_key)
		CREX_PARSE_PARAMETERS_END();
	} else {
		CREX_ASSERT(ce->enum_backing_type == IS_STRING);

		if (CREX_ARG_USES_STRICT_TYPES()) {
			CREX_PARSE_PARAMETERS_START(1, 1)
				C_PARAM_STR(string_key)
			CREX_PARSE_PARAMETERS_END();
		} else {
			// We allow long keys so that coercion to string doesn't happen implicitly. The JIT
			// skips deallocation of params that don't require it. In the case of from/tryFrom
			// passing int to from(int|string) looks like no coercion will happen, so the JIT
			// won't emit a dtor call. Thus we allocate/free the string manually.
			CREX_PARSE_PARAMETERS_START(1, 1)
				C_PARAM_STR_OR_LONG(string_key, long_key)
			CREX_PARSE_PARAMETERS_END();

			if (string_key == NULL) {
				release_string = true;
				string_key = crex_long_to_str(long_key);
			}
		}
	}

	crex_object *case_obj;
	if (crex_enum_get_case_by_value(&case_obj, ce, long_key, string_key, try) == FAILURE) {
		goto throw;
	}

	if (case_obj == NULL) {
		CREX_ASSERT(try);
		goto return_null;
	}

	if (release_string) {
		crex_string_release(string_key);
	}
	RETURN_OBJ_COPY(case_obj);

throw:
	if (release_string) {
		crex_string_release(string_key);
	}
	RETURN_THROWS();

return_null:
	if (release_string) {
		crex_string_release(string_key);
	}
	RETURN_NULL();
}

static CREX_NAMED_FUNCTION(crex_enum_from_func)
{
	crex_enum_from_base(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}

static CREX_NAMED_FUNCTION(crex_enum_try_from_func)
{
	crex_enum_from_base(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}

static void crex_enum_register_func(crex_class_entry *ce, crex_known_string_id name_id, crex_internal_function *zif) {
	crex_string *name = ZSTR_KNOWN(name_id);
	zif->type = CREX_INTERNAL_FUNCTION;
	zif->module = EG(current_module);
	zif->scope = ce;
	zif->T = CREX_OBSERVER_ENABLED;
    if (EG(active)) { // at run-time
		CREX_MAP_PTR_INIT(zif->run_time_cache, crex_arena_calloc(&CG(arena), 1, crex_internal_run_time_cache_reserved_size()));
	} else {
		CREX_MAP_PTR_NEW(zif->run_time_cache);
	}

	if (!crex_hash_add_ptr(&ce->function_table, name, zif)) {
		crex_error_noreturn(E_COMPILE_ERROR, "Cannot redeclare %s::%s()", ZSTR_VAL(ce->name), ZSTR_VAL(name));
	}
}

void crex_enum_register_funcs(crex_class_entry *ce)
{
	const uint32_t fn_flags =
		CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_HAS_RETURN_TYPE|CREX_ACC_ARENA_ALLOCATED;
	crex_internal_function *cases_function = crex_arena_calloc(&CG(arena), sizeof(crex_internal_function), 1);
	cases_function->handler = crex_enum_cases_func;
	cases_function->function_name = ZSTR_KNOWN(CREX_STR_CASES);
	cases_function->fn_flags = fn_flags;
	cases_function->arg_info = (crex_internal_arg_info *) (arginfo_class_UnitEnum_cases + 1);
	crex_enum_register_func(ce, CREX_STR_CASES, cases_function);

	if (ce->enum_backing_type != IS_UNDEF) {
		crex_internal_function *from_function = crex_arena_calloc(&CG(arena), sizeof(crex_internal_function), 1);
		from_function->handler = crex_enum_from_func;
		from_function->function_name = ZSTR_KNOWN(CREX_STR_FROM);
		from_function->fn_flags = fn_flags;
		from_function->num_args = 1;
		from_function->required_num_args = 1;
		from_function->arg_info = (crex_internal_arg_info *) (arginfo_class_BackedEnum_from + 1);
		crex_enum_register_func(ce, CREX_STR_FROM, from_function);

		crex_internal_function *try_from_function = crex_arena_calloc(&CG(arena), sizeof(crex_internal_function), 1);
		try_from_function->handler = crex_enum_try_from_func;
		try_from_function->function_name = ZSTR_KNOWN(CREX_STR_TRYFROM);
		try_from_function->fn_flags = fn_flags;
		try_from_function->num_args = 1;
		try_from_function->required_num_args = 1;
		try_from_function->arg_info = (crex_internal_arg_info *) (arginfo_class_BackedEnum_tryFrom + 1);
		crex_enum_register_func(ce, CREX_STR_TRYFROM_LOWERCASE, try_from_function);
	}
}

void crex_enum_register_props(crex_class_entry *ce)
{
	ce->ce_flags |= CREX_ACC_NO_DYNAMIC_PROPERTIES;

	zval name_default_value;
	ZVAL_UNDEF(&name_default_value);
	crex_type name_type = CREX_TYPE_INIT_CODE(IS_STRING, 0, 0);
	crex_declare_typed_property(ce, ZSTR_KNOWN(CREX_STR_NAME), &name_default_value, CREX_ACC_PUBLIC | CREX_ACC_READONLY, NULL, name_type);

	if (ce->enum_backing_type != IS_UNDEF) {
		zval value_default_value;
		ZVAL_UNDEF(&value_default_value);
		crex_type value_type = CREX_TYPE_INIT_CODE(ce->enum_backing_type, 0, 0);
		crex_declare_typed_property(ce, ZSTR_KNOWN(CREX_STR_VALUE), &value_default_value, CREX_ACC_PUBLIC | CREX_ACC_READONLY, NULL, value_type);
	}
}

static const crex_function_entry unit_enum_methods[] = {
	CREX_NAMED_ME(cases, crex_enum_cases_func, arginfo_class_UnitEnum_cases, CREX_ACC_PUBLIC | CREX_ACC_STATIC)
	CREX_FE_END
};

static const crex_function_entry backed_enum_methods[] = {
	CREX_NAMED_ME(cases, crex_enum_cases_func, arginfo_class_UnitEnum_cases, CREX_ACC_PUBLIC | CREX_ACC_STATIC)
	CREX_NAMED_ME(from, crex_enum_from_func, arginfo_class_BackedEnum_from, CREX_ACC_PUBLIC | CREX_ACC_STATIC)
	CREX_NAMED_ME(tryFrom, crex_enum_try_from_func, arginfo_class_BackedEnum_tryFrom, CREX_ACC_PUBLIC | CREX_ACC_STATIC)
	CREX_FE_END
};

CREX_API crex_class_entry *crex_register_internal_enum(
	const char *name, uint8_t type, const crex_function_entry *functions)
{
	CREX_ASSERT(type == IS_UNDEF || type == IS_LONG || type == IS_STRING);

	crex_class_entry tmp_ce;
	INIT_CLASS_ENTRY_EX(tmp_ce, name, strlen(name), functions);

	crex_class_entry *ce = crex_register_internal_class(&tmp_ce);
	ce->ce_flags |= CREX_ACC_ENUM;
	ce->enum_backing_type = type;
	if (type != IS_UNDEF) {
		HashTable *backed_enum_table = pemalloc(sizeof(HashTable), 1);
		crex_hash_init(backed_enum_table, 0, NULL, ZVAL_PTR_DTOR, 1);
		crex_class_set_backed_enum_table(ce, backed_enum_table);
	}

	crex_enum_register_props(ce);
	if (type == IS_UNDEF) {
		crex_register_functions(
			ce, unit_enum_methods, &ce->function_table, EG(current_module)->type);
		crex_class_implements(ce, 1, crex_ce_unit_enum);
	} else {
		crex_register_functions(
			ce, backed_enum_methods, &ce->function_table, EG(current_module)->type);
		crex_class_implements(ce, 1, crex_ce_backed_enum);
	}

	return ce;
}

static crex_ast_ref *create_enum_case_ast(
		crex_string *class_name, crex_string *case_name, zval *value) {
	// TODO: Use custom node type for enum cases?
	size_t size = sizeof(crex_ast_ref) + crex_ast_size(3)
		+ (value ? 3 : 2) * sizeof(crex_ast_zval);
	char *p = pemalloc(size, 1);
	crex_ast_ref *ref = (crex_ast_ref *) p; p += sizeof(crex_ast_ref);
	GC_SET_REFCOUNT(ref, 1);
	GC_TYPE_INFO(ref) = GC_CONSTANT_AST | GC_PERSISTENT | GC_IMMUTABLE;

	crex_ast *ast = (crex_ast *) p; p += crex_ast_size(3);
	ast->kind = CREX_AST_CONST_ENUM_INIT;
	ast->attr = 0;
	ast->lineno = 0;

	ast->child[0] = (crex_ast *) p; p += sizeof(crex_ast_zval);
	ast->child[0]->kind = CREX_AST_ZVAL;
	ast->child[0]->attr = 0;
	CREX_ASSERT(ZSTR_IS_INTERNED(class_name));
	ZVAL_STR(crex_ast_get_zval(ast->child[0]), class_name);

	ast->child[1] = (crex_ast *) p; p += sizeof(crex_ast_zval);
	ast->child[1]->kind = CREX_AST_ZVAL;
	ast->child[1]->attr = 0;
	CREX_ASSERT(ZSTR_IS_INTERNED(case_name));
	ZVAL_STR(crex_ast_get_zval(ast->child[1]), case_name);

	if (value) {
		ast->child[2] = (crex_ast *) p; p += sizeof(crex_ast_zval);
		ast->child[2]->kind = CREX_AST_ZVAL;
		ast->child[2]->attr = 0;
		CREX_ASSERT(!C_REFCOUNTED_P(value));
		ZVAL_COPY_VALUE(crex_ast_get_zval(ast->child[2]), value);
	} else {
		ast->child[2] = NULL;
	}

	return ref;
}

CREX_API void crex_enum_add_case(crex_class_entry *ce, crex_string *case_name, zval *value)
{
	if (value) {
		CREX_ASSERT(ce->enum_backing_type == C_TYPE_P(value));
		if (C_TYPE_P(value) == IS_STRING && !ZSTR_IS_INTERNED(C_STR_P(value))) {
			zval_make_interned_string(value);
		}

		HashTable *backed_enum_table = CE_BACKED_ENUM_TABLE(ce);

		zval case_name_zv;
		ZVAL_STR(&case_name_zv, case_name);
		if (C_TYPE_P(value) == IS_LONG) {
			crex_hash_index_add_new(backed_enum_table, C_LVAL_P(value), &case_name_zv);
		} else {
			crex_hash_add_new(backed_enum_table, C_STR_P(value), &case_name_zv);
		}
	} else {
		CREX_ASSERT(ce->enum_backing_type == IS_UNDEF);
	}

	zval ast_zv;
	C_TYPE_INFO(ast_zv) = IS_CONSTANT_AST;
	C_AST(ast_zv) = create_enum_case_ast(ce->name, case_name, value);
	crex_class_constant *c = crex_declare_class_constant_ex(
		ce, case_name, &ast_zv, CREX_ACC_PUBLIC, NULL);
	CREX_CLASS_CONST_FLAGS(c) |= CREX_CLASS_CONST_IS_CASE;
}

CREX_API void crex_enum_add_case_cstr(crex_class_entry *ce, const char *name, zval *value)
{
	crex_string *name_str = crex_string_init_interned(name, strlen(name), 1);
	crex_enum_add_case(ce, name_str, value);
	crex_string_release(name_str);
}

CREX_API crex_object *crex_enum_get_case(crex_class_entry *ce, crex_string *name) {
	crex_class_constant *c = crex_hash_find_ptr(CE_CONSTANTS_TABLE(ce), name);
	CREX_ASSERT(c && "Must be a valid enum case");
	CREX_ASSERT(CREX_CLASS_CONST_FLAGS(c) & CREX_CLASS_CONST_IS_CASE);

	if (C_TYPE(c->value) == IS_CONSTANT_AST) {
		if (zval_update_constant_ex(&c->value, c->ce) == FAILURE) {
			CREX_UNREACHABLE();
		}
	}
	CREX_ASSERT(C_TYPE(c->value) == IS_OBJECT);
	return C_OBJ(c->value);
}

CREX_API crex_object *crex_enum_get_case_cstr(crex_class_entry *ce, const char *name) {
	crex_string *name_str = crex_string_init(name, strlen(name), 0);
	crex_object *result = crex_enum_get_case(ce, name_str);
	crex_string_release(name_str);
	return result;
}
