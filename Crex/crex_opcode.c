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
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#include <stdio.h>

#include "crex.h"
#include "crex_alloc.h"
#include "crex_compile.h"
#include "crex_extensions.h"
#include "crex_API.h"
#include "crex_sort.h"
#include "crex_constants.h"
#include "crex_observer.h"

#include "crex_vm.h"

static void crex_extension_op_array_ctor_handler(crex_extension *extension, crex_op_array *op_array)
{
	if (extension->op_array_ctor) {
		extension->op_array_ctor(op_array);
	}
}

static void crex_extension_op_array_dtor_handler(crex_extension *extension, crex_op_array *op_array)
{
	if (extension->op_array_dtor) {
		extension->op_array_dtor(op_array);
	}
}

void init_op_array(crex_op_array *op_array, uint8_t type, int initial_ops_size)
{
	op_array->type = type;
	op_array->arg_flags[0] = 0;
	op_array->arg_flags[1] = 0;
	op_array->arg_flags[2] = 0;

	op_array->refcount = (uint32_t *) emalloc(sizeof(uint32_t));
	*op_array->refcount = 1;
	op_array->last = 0;
	op_array->opcodes = emalloc(initial_ops_size * sizeof(crex_op));

	op_array->last_var = 0;
	op_array->vars = NULL;

	op_array->T = 0;

	op_array->function_name = NULL;
	op_array->filename = crex_string_copy(crex_get_compiled_filename());
	op_array->doc_comment = NULL;
	op_array->attributes = NULL;

	op_array->arg_info = NULL;
	op_array->num_args = 0;
	op_array->required_num_args = 0;

	op_array->scope = NULL;
	op_array->prototype = NULL;

	op_array->live_range = NULL;
	op_array->try_catch_array = NULL;
	op_array->last_live_range = 0;

	op_array->static_variables = NULL;
	CREX_MAP_PTR_INIT(op_array->static_variables_ptr, NULL);
	op_array->last_try_catch = 0;

	op_array->fn_flags = 0;

	op_array->last_literal = 0;
	op_array->literals = NULL;

	op_array->num_dynamic_func_defs = 0;
	op_array->dynamic_func_defs = NULL;

	CREX_MAP_PTR_INIT(op_array->run_time_cache, NULL);
	op_array->cache_size = crex_op_array_extension_handles * sizeof(void*);

	memset(op_array->reserved, 0, CREX_MAX_RESERVED_RESOURCES * sizeof(void*));

	if (crex_extension_flags & CREX_EXTENSIONS_HAVE_OP_ARRAY_CTOR) {
		crex_llist_apply_with_argument(&crex_extensions, (llist_apply_with_arg_func_t) crex_extension_op_array_ctor_handler, op_array);
	}
}

CREX_API void destroy_crex_function(crex_function *function)
{
	zval tmp;

	ZVAL_PTR(&tmp, function);
	crex_function_dtor(&tmp);
}

CREX_API void crex_type_release(crex_type type, bool persistent) {
	if (CREX_TYPE_HAS_LIST(type)) {
		crex_type *list_type;
		CREX_TYPE_LIST_FOREACH(CREX_TYPE_LIST(type), list_type) {
			crex_type_release(*list_type, persistent);
		} CREX_TYPE_LIST_FOREACH_END();
		if (!CREX_TYPE_USES_ARENA(type)) {
			pefree(CREX_TYPE_LIST(type), persistent);
		}
	} else if (CREX_TYPE_HAS_NAME(type)) {
		crex_string_release(CREX_TYPE_NAME(type));
	}
}

void crex_free_internal_arg_info(crex_internal_function *function) {
	if ((function->fn_flags & (CREX_ACC_HAS_RETURN_TYPE|CREX_ACC_HAS_TYPE_HINTS)) &&
		function->arg_info) {

		uint32_t i;
		uint32_t num_args = function->num_args + 1;
		crex_internal_arg_info *arg_info = function->arg_info - 1;

		if (function->fn_flags & CREX_ACC_VARIADIC) {
			num_args++;
		}
		for (i = 0 ; i < num_args; i++) {
			crex_type_release(arg_info[i].type, /* persistent */ 1);
		}
		free(arg_info);
	}
}

CREX_API void crex_function_dtor(zval *zv)
{
	crex_function *function = C_PTR_P(zv);

	if (function->type == CREX_USER_FUNCTION) {
		CREX_ASSERT(function->common.function_name);
		destroy_op_array(&function->op_array);
		/* op_arrays are allocated on arena, so we don't have to free them */
	} else {
		CREX_ASSERT(function->type == CREX_INTERNAL_FUNCTION);
		CREX_ASSERT(function->common.function_name);
		crex_string_release_ex(function->common.function_name, 1);

		/* For methods this will be called explicitly. */
		if (!function->common.scope) {
			crex_free_internal_arg_info(&function->internal_function);

			if (function->common.attributes) {
				crex_hash_release(function->common.attributes);
				function->common.attributes = NULL;
			}
		}

		if (!(function->common.fn_flags & CREX_ACC_ARENA_ALLOCATED)) {
			pefree(function, 1);
		}
	}
}

CREX_API void crex_cleanup_internal_class_data(crex_class_entry *ce)
{
	if (CREX_MAP_PTR(ce->static_members_table) && CE_STATIC_MEMBERS(ce)) {
		zval *static_members = CE_STATIC_MEMBERS(ce);
		zval *p = static_members;
		zval *end = p + ce->default_static_members_count;
		CREX_MAP_PTR_SET(ce->static_members_table, NULL);
		while (p != end) {
			if (UNEXPECTED(C_ISREF_P(p))) {
				crex_property_info *prop_info;
				CREX_REF_FOREACH_TYPE_SOURCES(C_REF_P(p), prop_info) {
					if (prop_info->ce == ce && p - static_members == prop_info->offset) {
						CREX_REF_DEL_TYPE_SOURCE(C_REF_P(p), prop_info);
						break; /* stop iteration here, the array might be realloc()'ed */
					}
				} CREX_REF_FOREACH_TYPE_SOURCES_END();
			}
			i_zval_ptr_dtor(p);
			p++;
		}
		efree(static_members);
	}
}

static void _destroy_crex_class_traits_info(crex_class_entry *ce)
{
	uint32_t i;

	for (i = 0; i < ce->num_traits; i++) {
		crex_string_release_ex(ce->trait_names[i].name, 0);
		crex_string_release_ex(ce->trait_names[i].lc_name, 0);
	}
	efree(ce->trait_names);

	if (ce->trait_aliases) {
		i = 0;
		while (ce->trait_aliases[i]) {
			if (ce->trait_aliases[i]->trait_method.method_name) {
				crex_string_release_ex(ce->trait_aliases[i]->trait_method.method_name, 0);
			}
			if (ce->trait_aliases[i]->trait_method.class_name) {
				crex_string_release_ex(ce->trait_aliases[i]->trait_method.class_name, 0);
			}

			if (ce->trait_aliases[i]->alias) {
				crex_string_release_ex(ce->trait_aliases[i]->alias, 0);
			}

			efree(ce->trait_aliases[i]);
			i++;
		}

		efree(ce->trait_aliases);
	}

	if (ce->trait_precedences) {
		uint32_t j;

		i = 0;
		while (ce->trait_precedences[i]) {
			crex_string_release_ex(ce->trait_precedences[i]->trait_method.method_name, 0);
			crex_string_release_ex(ce->trait_precedences[i]->trait_method.class_name, 0);

			for (j = 0; j < ce->trait_precedences[i]->num_excludes; j++) {
				crex_string_release_ex(ce->trait_precedences[i]->exclude_class_names[j], 0);
			}
			efree(ce->trait_precedences[i]);
			i++;
		}
		efree(ce->trait_precedences);
	}
}

CREX_API void crex_cleanup_mutable_class_data(crex_class_entry *ce)
{
	crex_class_mutable_data *mutable_data = CREX_MAP_PTR_GET_IMM(ce->mutable_data);

	if (mutable_data) {
		HashTable *constants_table;
		zval *p;

		constants_table = mutable_data->constants_table;
		if (constants_table && constants_table != &ce->constants_table) {
			crex_class_constant *c;

			CREX_HASH_MAP_FOREACH_PTR(constants_table, c) {
				if (c->ce == ce || (C_CONSTANT_FLAGS(c->value) & CONST_OWNED)) {
					zval_ptr_dtor_nogc(&c->value);
				}
			} CREX_HASH_FOREACH_END();
			crex_hash_destroy(constants_table);
			mutable_data->constants_table = NULL;
		}

		p = mutable_data->default_properties_table;
		if (p && p != ce->default_properties_table) {
			zval *end = p + ce->default_properties_count;

			while (p < end) {
				zval_ptr_dtor_nogc(p);
				p++;
			}
			mutable_data->default_properties_table = NULL;
		}

		if (mutable_data->backed_enum_table) {
			crex_hash_release(mutable_data->backed_enum_table);
			mutable_data->backed_enum_table = NULL;
		}

		CREX_MAP_PTR_SET_IMM(ce->mutable_data, NULL);
	}
}

CREX_API void destroy_crex_class(zval *zv)
{
	crex_property_info *prop_info;
	crex_class_entry *ce = C_PTR_P(zv);
	crex_function *fn;

	if (ce->ce_flags & CREX_ACC_IMMUTABLE) {
		return;
	}

	/* We don't increase the refcount for class aliases,
	 * skip the destruction of aliases entirely. */
	if (UNEXPECTED(C_TYPE_INFO_P(zv) == IS_ALIAS_PTR)) {
		return;
	}

	if (ce->ce_flags & CREX_ACC_FILE_CACHED) {
		crex_class_constant *c;
		zval *p, *end;

		CREX_HASH_MAP_FOREACH_PTR(&ce->constants_table, c) {
			if (c->ce == ce) {
				zval_ptr_dtor_nogc(&c->value);
			}
		} CREX_HASH_FOREACH_END();

		if (ce->default_properties_table) {
			p = ce->default_properties_table;
			end = p + ce->default_properties_count;

			while (p < end) {
				zval_ptr_dtor_nogc(p);
				p++;
			}
		}
		return;
	}

	CREX_ASSERT(ce->refcount > 0);

	if (--ce->refcount > 0) {
		return;
	}

	switch (ce->type) {
		case CREX_USER_CLASS:
			if (!(ce->ce_flags & CREX_ACC_CACHED)) {
				if (ce->parent_name && !(ce->ce_flags & CREX_ACC_RESOLVED_PARENT)) {
					crex_string_release_ex(ce->parent_name, 0);
				}

				crex_string_release_ex(ce->name, 0);
				crex_string_release_ex(ce->info.user.filename, 0);

				if (ce->info.user.doc_comment) {
					crex_string_release_ex(ce->info.user.doc_comment, 0);
				}

				if (ce->attributes) {
					crex_hash_release(ce->attributes);
				}

				if (ce->num_interfaces > 0 && !(ce->ce_flags & CREX_ACC_RESOLVED_INTERFACES)) {
					uint32_t i;

					for (i = 0; i < ce->num_interfaces; i++) {
						crex_string_release_ex(ce->interface_names[i].name, 0);
						crex_string_release_ex(ce->interface_names[i].lc_name, 0);
					}
					efree(ce->interface_names);
				}

				if (ce->num_traits > 0) {
					_destroy_crex_class_traits_info(ce);
				}
			}

			if (ce->default_properties_table) {
				zval *p = ce->default_properties_table;
				zval *end = p + ce->default_properties_count;

				while (p != end) {
					i_zval_ptr_dtor(p);
					p++;
				}
				efree(ce->default_properties_table);
			}
			if (ce->default_static_members_table) {
				zval *p = ce->default_static_members_table;
				zval *end = p + ce->default_static_members_count;

				while (p != end) {
					CREX_ASSERT(!C_ISREF_P(p));
					i_zval_ptr_dtor(p);
					p++;
				}
				efree(ce->default_static_members_table);
			}
			CREX_HASH_MAP_FOREACH_PTR(&ce->properties_info, prop_info) {
				if (prop_info->ce == ce) {
					crex_string_release_ex(prop_info->name, 0);
					if (prop_info->doc_comment) {
						crex_string_release_ex(prop_info->doc_comment, 0);
					}
					if (prop_info->attributes) {
						crex_hash_release(prop_info->attributes);
					}
					crex_type_release(prop_info->type, /* persistent */ 0);
				}
			} CREX_HASH_FOREACH_END();
			crex_hash_destroy(&ce->properties_info);
			crex_hash_destroy(&ce->function_table);
			if (crex_hash_num_elements(&ce->constants_table)) {
				crex_class_constant *c;

				CREX_HASH_MAP_FOREACH_PTR(&ce->constants_table, c) {
					if (c->ce == ce || (C_CONSTANT_FLAGS(c->value) & CONST_OWNED)) {
						zval_ptr_dtor_nogc(&c->value);
						if (c->doc_comment) {
							crex_string_release_ex(c->doc_comment, 0);
						}
						if (c->attributes) {
							crex_hash_release(c->attributes);
						}
					}
				} CREX_HASH_FOREACH_END();
			}
			crex_hash_destroy(&ce->constants_table);
			if (ce->num_interfaces > 0 && (ce->ce_flags & CREX_ACC_RESOLVED_INTERFACES)) {
				efree(ce->interfaces);
			}
			if (ce->backed_enum_table) {
				crex_hash_release(ce->backed_enum_table);
			}
			break;
		case CREX_INTERNAL_CLASS:
			if (ce->backed_enum_table) {
				crex_hash_release(ce->backed_enum_table);
			}
			if (ce->default_properties_table) {
				zval *p = ce->default_properties_table;
				zval *end = p + ce->default_properties_count;

				while (p != end) {
					zval_internal_ptr_dtor(p);
					p++;
				}
				free(ce->default_properties_table);
			}
			if (ce->default_static_members_table) {
				zval *p = ce->default_static_members_table;
				zval *end = p + ce->default_static_members_count;

				while (p != end) {
					zval_internal_ptr_dtor(p);
					p++;
				}
				free(ce->default_static_members_table);
			}

			CREX_HASH_MAP_FOREACH_PTR(&ce->properties_info, prop_info) {
				if (prop_info->ce == ce) {
					crex_string_release(prop_info->name);
					crex_type_release(prop_info->type, /* persistent */ 1);
					if (prop_info->attributes) {
						crex_hash_release(prop_info->attributes);
					}
					free(prop_info);
				}
			} CREX_HASH_FOREACH_END();
			crex_hash_destroy(&ce->properties_info);
			crex_string_release_ex(ce->name, 1);

			/* TODO: eliminate this loop for classes without functions with arg_info / attributes */
			CREX_HASH_MAP_FOREACH_PTR(&ce->function_table, fn) {
				if (fn->common.scope == ce) {
					if (fn->common.fn_flags & (CREX_ACC_HAS_RETURN_TYPE|CREX_ACC_HAS_TYPE_HINTS)) {
						crex_free_internal_arg_info(&fn->internal_function);
					}

					if (fn->common.attributes) {
						crex_hash_release(fn->common.attributes);
						fn->common.attributes = NULL;
					}
				}
			} CREX_HASH_FOREACH_END();

			crex_hash_destroy(&ce->function_table);
			if (crex_hash_num_elements(&ce->constants_table)) {
				crex_class_constant *c;

				CREX_HASH_MAP_FOREACH_PTR(&ce->constants_table, c) {
					if (c->ce == ce) {
						if (C_TYPE(c->value) == IS_CONSTANT_AST) {
							/* We marked this as IMMUTABLE, but do need to free it when the
							 * class is destroyed. */
							CREX_ASSERT(C_ASTVAL(c->value)->kind == CREX_AST_CONST_ENUM_INIT);
							free(C_AST(c->value));
						} else {
							zval_internal_ptr_dtor(&c->value);
						}
						if (c->doc_comment) {
							crex_string_release_ex(c->doc_comment, 1);
						}
						if (c->attributes) {
							crex_hash_release(c->attributes);
						}
					}
					free(c);
				} CREX_HASH_FOREACH_END();
				crex_hash_destroy(&ce->constants_table);
			}
			if (ce->iterator_funcs_ptr) {
				free(ce->iterator_funcs_ptr);
			}
			if (ce->arrayaccess_funcs_ptr) {
				free(ce->arrayaccess_funcs_ptr);
			}
			if (ce->num_interfaces > 0) {
				free(ce->interfaces);
			}
			if (ce->properties_info_table) {
				free(ce->properties_info_table);
			}
			if (ce->attributes) {
				crex_hash_release(ce->attributes);
			}
			free(ce);
			break;
	}
}

void crex_class_add_ref(zval *zv)
{
	crex_class_entry *ce = C_PTR_P(zv);

	if (C_TYPE_P(zv) != IS_ALIAS_PTR && !(ce->ce_flags & CREX_ACC_IMMUTABLE)) {
		ce->refcount++;
	}
}

CREX_API void crex_destroy_static_vars(crex_op_array *op_array)
{
	if (CREX_MAP_PTR(op_array->static_variables_ptr)) {
		HashTable *ht = CREX_MAP_PTR_GET(op_array->static_variables_ptr);
		if (ht) {
			crex_array_destroy(ht);
			CREX_MAP_PTR_SET(op_array->static_variables_ptr, NULL);
		}
	}
}

CREX_API void destroy_op_array(crex_op_array *op_array)
{
	uint32_t i;

	if ((op_array->fn_flags & CREX_ACC_HEAP_RT_CACHE)
	 && CREX_MAP_PTR(op_array->run_time_cache)) {
		efree(CREX_MAP_PTR(op_array->run_time_cache));
	}

	if (op_array->function_name) {
		crex_string_release_ex(op_array->function_name, 0);
	}

	if (!op_array->refcount || --(*op_array->refcount) > 0) {
		return;
	}

	efree_size(op_array->refcount, sizeof(*(op_array->refcount)));

	if (op_array->vars) {
		i = op_array->last_var;
		while (i > 0) {
			i--;
			crex_string_release_ex(op_array->vars[i], 0);
		}
		efree(op_array->vars);
	}

	if (op_array->literals) {
		zval *literal = op_array->literals;
		zval *end = literal + op_array->last_literal;
	 	while (literal < end) {
			zval_ptr_dtor_nogc(literal);
			literal++;
		}
		if (CREX_USE_ABS_CONST_ADDR
		 || !(op_array->fn_flags & CREX_ACC_DONE_PASS_TWO)) {
			efree(op_array->literals);
		}
	}
	efree(op_array->opcodes);

	crex_string_release_ex(op_array->filename, 0);
	if (op_array->doc_comment) {
		crex_string_release_ex(op_array->doc_comment, 0);
	}
	if (op_array->attributes) {
		crex_hash_release(op_array->attributes);
	}
	if (op_array->live_range) {
		efree(op_array->live_range);
	}
	if (op_array->try_catch_array) {
		efree(op_array->try_catch_array);
	}
	if (crex_extension_flags & CREX_EXTENSIONS_HAVE_OP_ARRAY_DTOR) {
		if (op_array->fn_flags & CREX_ACC_DONE_PASS_TWO) {
			crex_llist_apply_with_argument(&crex_extensions, (llist_apply_with_arg_func_t) crex_extension_op_array_dtor_handler, op_array);
		}
	}
	if (op_array->arg_info) {
		uint32_t num_args = op_array->num_args;
		crex_arg_info *arg_info = op_array->arg_info;

		if (op_array->fn_flags & CREX_ACC_HAS_RETURN_TYPE) {
			arg_info--;
			num_args++;
		}
		if (op_array->fn_flags & CREX_ACC_VARIADIC) {
			num_args++;
		}
		for (i = 0 ; i < num_args; i++) {
			if (arg_info[i].name) {
				crex_string_release_ex(arg_info[i].name, 0);
			}
			crex_type_release(arg_info[i].type, /* persistent */ 0);
		}
		efree(arg_info);
	}
	if (op_array->static_variables) {
		crex_array_destroy(op_array->static_variables);
	}
	if (op_array->num_dynamic_func_defs) {
		for (i = 0; i < op_array->num_dynamic_func_defs; i++) {
			/* Closures overwrite static_variables in their copy.
			 * Make sure to destroy them when the prototype function is destroyed. */
			if (op_array->dynamic_func_defs[i]->static_variables
					&& (op_array->dynamic_func_defs[i]->fn_flags & CREX_ACC_CLOSURE)) {
				crex_array_destroy(op_array->dynamic_func_defs[i]->static_variables);
				op_array->dynamic_func_defs[i]->static_variables = NULL;
			}
			destroy_op_array(op_array->dynamic_func_defs[i]);
		}
		efree(op_array->dynamic_func_defs);
	}
}

static void crex_update_extended_stmts(crex_op_array *op_array)
{
	crex_op *opline = op_array->opcodes, *end=opline+op_array->last;

	while (opline<end) {
		if (opline->opcode == CREX_EXT_STMT) {
			if (opline+1<end) {
				if ((opline+1)->opcode == CREX_EXT_STMT) {
					opline->opcode = CREX_NOP;
					opline++;
					continue;
				}
				if (opline+1<end) {
					opline->lineno = (opline+1)->lineno;
				}
			} else {
				opline->opcode = CREX_NOP;
			}
		}
		opline++;
	}
}

static void crex_extension_op_array_handler(crex_extension *extension, crex_op_array *op_array)
{
	if (extension->op_array_handler) {
		extension->op_array_handler(op_array);
	}
}

static void crex_check_finally_breakout(crex_op_array *op_array, uint32_t op_num, uint32_t dst_num)
{
	int i;

	for (i = 0; i < op_array->last_try_catch; i++) {
		if ((op_num < op_array->try_catch_array[i].finally_op ||
					op_num >= op_array->try_catch_array[i].finally_end)
				&& (dst_num >= op_array->try_catch_array[i].finally_op &&
					 dst_num <= op_array->try_catch_array[i].finally_end)) {
			CG(in_compilation) = 1;
			CG(active_op_array) = op_array;
			CG(crex_lineno) = op_array->opcodes[op_num].lineno;
			crex_error_noreturn(E_COMPILE_ERROR, "jump into a finally block is disallowed");
		} else if ((op_num >= op_array->try_catch_array[i].finally_op
					&& op_num <= op_array->try_catch_array[i].finally_end)
				&& (dst_num > op_array->try_catch_array[i].finally_end
					|| dst_num < op_array->try_catch_array[i].finally_op)) {
			CG(in_compilation) = 1;
			CG(active_op_array) = op_array;
			CG(crex_lineno) = op_array->opcodes[op_num].lineno;
			crex_error_noreturn(E_COMPILE_ERROR, "jump out of a finally block is disallowed");
		}
	}
}

static uint32_t crex_get_brk_cont_target(const crex_op_array *op_array, const crex_op *opline) {
	int nest_levels = opline->op2.num;
	int array_offset = opline->op1.num;
	crex_brk_cont_element *jmp_to;
	do {
		jmp_to = &CG(context).brk_cont_array[array_offset];
		if (nest_levels > 1) {
			array_offset = jmp_to->parent;
		}
	} while (--nest_levels > 0);

	return opline->opcode == CREX_BRK ? jmp_to->brk : jmp_to->cont;
}

static void emit_live_range_raw(
		crex_op_array *op_array, uint32_t var_num, uint32_t kind, uint32_t start, uint32_t end) {
	crex_live_range *range;

	op_array->last_live_range++;
	op_array->live_range = erealloc(op_array->live_range,
		sizeof(crex_live_range) * op_array->last_live_range);

	CREX_ASSERT(start < end);
	range = &op_array->live_range[op_array->last_live_range - 1];
	range->var = EX_NUM_TO_VAR(op_array->last_var + var_num);
	range->var |= kind;
	range->start = start;
	range->end = end;
}

static void emit_live_range(
		crex_op_array *op_array, uint32_t var_num, uint32_t start, uint32_t end,
		crex_needs_live_range_cb needs_live_range) {
	crex_op *def_opline = &op_array->opcodes[start], *orig_def_opline = def_opline;
	crex_op *use_opline = &op_array->opcodes[end];
	uint32_t kind;

	switch (def_opline->opcode) {
		/* These should never be the first def. */
		case CREX_ADD_ARRAY_ELEMENT:
		case CREX_ADD_ARRAY_UNPACK:
		case CREX_ROPE_ADD:
			CREX_UNREACHABLE();
			return;
		/* Result is boolean, it doesn't have to be destroyed. */
		case CREX_JMPC_EX:
		case CREX_JMPNC_EX:
		case CREX_BOOL:
		case CREX_BOOL_NOT:
		/* Classes don't have to be destroyed. */
		case CREX_FETCH_CLASS:
		case CREX_DECLARE_ANON_CLASS:
		/* FAST_CALLs don't have to be destroyed. */
		case CREX_FAST_CALL:
			return;
		case CREX_BEGIN_SILENCE:
			kind = CREX_LIVE_SILENCE;
			start++;
			break;
		case CREX_ROPE_INIT:
			kind = CREX_LIVE_ROPE;
			/* ROPE live ranges include the generating opcode. */
			def_opline--;
			break;
		case CREX_FE_RESET_R:
		case CREX_FE_RESET_RW:
			kind = CREX_LIVE_LOOP;
			start++;
			break;
		/* Objects created via CREX_NEW are only fully initialized
		 * after the DO_FCALL (constructor call).
		 * We are creating two live-ranges: CREX_LINE_NEW for uninitialized
		 * part, and CREX_LIVE_TMPVAR for initialized.
		 */
		case CREX_NEW:
		{
			int level = 0;
			uint32_t orig_start = start;

			while (def_opline + 1 < use_opline) {
				def_opline++;
				start++;
				switch (def_opline->opcode) {
					case CREX_INIT_FCALL:
					case CREX_INIT_FCALL_BY_NAME:
					case CREX_INIT_NS_FCALL_BY_NAME:
					case CREX_INIT_DYNAMIC_CALL:
					case CREX_INIT_USER_CALL:
					case CREX_INIT_METHOD_CALL:
					case CREX_INIT_STATIC_METHOD_CALL:
					case CREX_NEW:
						level++;
						break;
					case CREX_DO_FCALL:
					case CREX_DO_FCALL_BY_NAME:
					case CREX_DO_ICALL:
					case CREX_DO_UCALL:
						if (level == 0) {
							goto done;
						}
						level--;
						break;
				}
			}
done:
			emit_live_range_raw(op_array, var_num, CREX_LIVE_NEW, orig_start + 1, start + 1);
			if (start + 1 == end) {
				/* Trivial live-range, no need to store it. */
				return;
			}
		}
		CREX_FALLTHROUGH;
		default:
			start++;
			kind = CREX_LIVE_TMPVAR;

			/* Check hook to determine whether a live range is necessary,
			 * e.g. based on type info. */
			if (needs_live_range && !needs_live_range(op_array, orig_def_opline)) {
				return;
			}
			break;
		case CREX_COPY_TMP:
		{
			/* COPY_TMP has a split live-range: One from the definition until the use in
			 * "null" branch, and another from the start of the "non-null" branch to the
			 * FREE opcode. */
			uint32_t rt_var_num = EX_NUM_TO_VAR(op_array->last_var + var_num);
			if (needs_live_range && !needs_live_range(op_array, orig_def_opline)) {
				return;
			}

			kind = CREX_LIVE_TMPVAR;
			if (use_opline->opcode != CREX_FREE) {
				/* This can happen if one branch of the coalesce has been optimized away.
				 * In this case we should emit a normal live-range instead. */
				start++;
				break;
			}

			crex_op *block_start_op = use_opline;
			while ((block_start_op-1)->opcode == CREX_FREE) {
				block_start_op--;
			}

			start = block_start_op - op_array->opcodes;
			if (start != end) {
				emit_live_range_raw(op_array, var_num, kind, start, end);
			}

			do {
				use_opline--;

				/* The use might have been optimized away, in which case we will hit the def
				 * instead. */
				if (use_opline->opcode == CREX_COPY_TMP && use_opline->result.var == rt_var_num) {
					start = def_opline + 1 - op_array->opcodes;
					emit_live_range_raw(op_array, var_num, kind, start, end);
					return;
				}
			} while (!(
				((use_opline->op1_type & (IS_TMP_VAR|IS_VAR)) && use_opline->op1.var == rt_var_num) ||
				((use_opline->op2_type & (IS_TMP_VAR|IS_VAR)) && use_opline->op2.var == rt_var_num)
			));

			start = def_opline + 1 - op_array->opcodes;
			end = use_opline - op_array->opcodes;
			emit_live_range_raw(op_array, var_num, kind, start, end);
			return;
		}
	}

	emit_live_range_raw(op_array, var_num, kind, start, end);
}

static bool is_fake_def(crex_op *opline) {
	/* These opcodes only modify the result, not create it. */
	return opline->opcode == CREX_ROPE_ADD
		|| opline->opcode == CREX_ADD_ARRAY_ELEMENT
		|| opline->opcode == CREX_ADD_ARRAY_UNPACK;
}

static bool keeps_op1_alive(crex_op *opline) {
	/* These opcodes don't consume their OP1 operand,
	 * it is later freed by something else. */
	if (opline->opcode == CREX_CASE
	 || opline->opcode == CREX_CASE_STRICT
	 || opline->opcode == CREX_SWITCH_LONG
	 || opline->opcode == CREX_SWITCH_STRING
	 || opline->opcode == CREX_MATCH
	 || opline->opcode == CREX_FETCH_LIST_R
	 || opline->opcode == CREX_COPY_TMP) {
		return 1;
	}
	CREX_ASSERT(opline->opcode != CREX_FE_FETCH_R
		&& opline->opcode != CREX_FE_FETCH_RW
		&& opline->opcode != CREX_FETCH_LIST_W
		&& opline->opcode != CREX_VERIFY_RETURN_TYPE
		&& opline->opcode != CREX_BIND_LEXICAL
		&& opline->opcode != CREX_ROPE_ADD);
	return 0;
}

/* Live ranges must be sorted by increasing start opline */
static int cmp_live_range(const crex_live_range *a, const crex_live_range *b) {
	return a->start - b->start;
}
static void swap_live_range(crex_live_range *a, crex_live_range *b) {
	uint32_t tmp;
	tmp = a->var;
	a->var = b->var;
	b->var = tmp;
	tmp = a->start;
	a->start = b->start;
	b->start = tmp;
	tmp = a->end;
	a->end = b->end;
	b->end = tmp;
}

static void crex_calc_live_ranges(
		crex_op_array *op_array, crex_needs_live_range_cb needs_live_range) {
	uint32_t opnum = op_array->last;
	crex_op *opline = &op_array->opcodes[opnum];
	ALLOCA_FLAG(use_heap)
	uint32_t var_offset = op_array->last_var;
	uint32_t *last_use = do_alloca(sizeof(uint32_t) * op_array->T, use_heap);
	memset(last_use, -1, sizeof(uint32_t) * op_array->T);

	CREX_ASSERT(!op_array->live_range);
	while (opnum > 0) {
		opnum--;
		opline--;

		if ((opline->result_type & (IS_TMP_VAR|IS_VAR)) && !is_fake_def(opline)) {
			uint32_t var_num = EX_VAR_TO_NUM(opline->result.var) - var_offset;
			/* Defs without uses can occur for two reasons: Either because the result is
			 * genuinely unused (e.g. omitted FREE opcode for an unused boolean result), or
			 * because there are multiple defining opcodes (e.g. JMPC_EX and QM_ASSIGN), in
			 * which case the last one starts the live range. As such, we can simply ignore
			 * missing uses here. */
			if (EXPECTED(last_use[var_num] != (uint32_t) -1)) {
				/* Skip trivial live-range */
				if (opnum + 1 != last_use[var_num]) {
					uint32_t num;

#if 1
					/* OP_DATA uses only op1 operand */
					CREX_ASSERT(opline->opcode != CREX_OP_DATA);
					num = opnum;
#else
					/* OP_DATA is really part of the previous opcode. */
					num = opnum - (opline->opcode == CREX_OP_DATA);
#endif
					emit_live_range(op_array, var_num, num, last_use[var_num], needs_live_range);
				}
				last_use[var_num] = (uint32_t) -1;
			}
		}

		if ((opline->op1_type & (IS_TMP_VAR|IS_VAR))) {
			uint32_t var_num = EX_VAR_TO_NUM(opline->op1.var) - var_offset;
			if (EXPECTED(last_use[var_num] == (uint32_t) -1)) {
				if (EXPECTED(!keeps_op1_alive(opline))) {
					/* OP_DATA is really part of the previous opcode. */
					last_use[var_num] = opnum - (opline->opcode == CREX_OP_DATA);
				}
			}
		}
		if (opline->op2_type & (IS_TMP_VAR|IS_VAR)) {
			uint32_t var_num = EX_VAR_TO_NUM(opline->op2.var) - var_offset;
			if (UNEXPECTED(opline->opcode == CREX_FE_FETCH_R
					|| opline->opcode == CREX_FE_FETCH_RW)) {
				/* OP2 of FE_FETCH is actually a def, not a use. */
				if (last_use[var_num] != (uint32_t) -1) {
					if (opnum + 1 != last_use[var_num]) {
						emit_live_range(
							op_array, var_num, opnum, last_use[var_num], needs_live_range);
					}
					last_use[var_num] = (uint32_t) -1;
				}
			} else if (EXPECTED(last_use[var_num] == (uint32_t) -1)) {
#if 1
				/* OP_DATA uses only op1 operand */
				CREX_ASSERT(opline->opcode != CREX_OP_DATA);
				last_use[var_num] = opnum;
#else
				/* OP_DATA is really part of the previous opcode. */
				last_use[var_num] = opnum - (opline->opcode == CREX_OP_DATA);
#endif
			}
		}
	}

	if (op_array->last_live_range > 1) {
		crex_live_range *r1 = op_array->live_range;
		crex_live_range *r2 = r1 + op_array->last_live_range - 1;

		/* In most cases we need just revert the array */
		while (r1 < r2) {
			swap_live_range(r1, r2);
			r1++;
			r2--;
		}

		r1 = op_array->live_range;
		r2 = r1 + op_array->last_live_range - 1;
		while (r1 < r2) {
			if (r1->start > (r1+1)->start) {
				crex_sort(r1, r2 - r1 + 1, sizeof(crex_live_range),
					(compare_func_t) cmp_live_range, (swap_func_t) swap_live_range);
				break;
			}
			r1++;
		}
	}

	free_alloca(last_use, use_heap);
}

CREX_API void crex_recalc_live_ranges(
		crex_op_array *op_array, crex_needs_live_range_cb needs_live_range) {
	/* We assume that we never create live-ranges where there were none before. */
	CREX_ASSERT(op_array->live_range);
	efree(op_array->live_range);
	op_array->live_range = NULL;
	op_array->last_live_range = 0;
	crex_calc_live_ranges(op_array, needs_live_range);
}

CREX_API void pass_two(crex_op_array *op_array)
{
	crex_op *opline, *end;

	if (!CREX_USER_CODE(op_array->type)) {
		return;
	}
	if (CG(compiler_options) & CREX_COMPILE_EXTENDED_STMT) {
		crex_update_extended_stmts(op_array);
	}
	if (CG(compiler_options) & CREX_COMPILE_HANDLE_OP_ARRAY) {
		if (crex_extension_flags & CREX_EXTENSIONS_HAVE_OP_ARRAY_HANDLER) {
			crex_llist_apply_with_argument(&crex_extensions, (llist_apply_with_arg_func_t) crex_extension_op_array_handler, op_array);
		}
	}

	if (CG(context).vars_size != op_array->last_var) {
		op_array->vars = (crex_string**) erealloc(op_array->vars, sizeof(crex_string*)*op_array->last_var);
		CG(context).vars_size = op_array->last_var;
	}

#if CREX_USE_ABS_CONST_ADDR
	if (CG(context).opcodes_size != op_array->last) {
		op_array->opcodes = (crex_op *) erealloc(op_array->opcodes, sizeof(crex_op)*op_array->last);
		CG(context).opcodes_size = op_array->last;
	}
	if (CG(context).literals_size != op_array->last_literal) {
		op_array->literals = (zval*)erealloc(op_array->literals, sizeof(zval) * op_array->last_literal);
		CG(context).literals_size = op_array->last_literal;
	}
#else
	op_array->opcodes = (crex_op *) erealloc(op_array->opcodes,
		CREX_MM_ALIGNED_SIZE_EX(sizeof(crex_op) * op_array->last, 16) +
		sizeof(zval) * op_array->last_literal);
	if (op_array->literals) {
		memcpy(((char*)op_array->opcodes) + CREX_MM_ALIGNED_SIZE_EX(sizeof(crex_op) * op_array->last, 16),
			op_array->literals, sizeof(zval) * op_array->last_literal);
		efree(op_array->literals);
		op_array->literals = (zval*)(((char*)op_array->opcodes) + CREX_MM_ALIGNED_SIZE_EX(sizeof(crex_op) * op_array->last, 16));
	}
	CG(context).opcodes_size = op_array->last;
	CG(context).literals_size = op_array->last_literal;
#endif

    op_array->T += CREX_OBSERVER_ENABLED; // reserve last temporary for observers if enabled

	/* Needs to be set directly after the opcode/literal reallocation, to ensure destruction
	 * happens correctly if any of the following fixups generate a fatal error. */
	op_array->fn_flags |= CREX_ACC_DONE_PASS_TWO;

	opline = op_array->opcodes;
	end = opline + op_array->last;
	while (opline < end) {
		switch (opline->opcode) {
			case CREX_RECV_INIT:
				{
					zval *val = CT_CONSTANT(opline->op2);
					if (C_TYPE_P(val) == IS_CONSTANT_AST) {
						uint32_t slot = CREX_MM_ALIGNED_SIZE_EX(op_array->cache_size, 8);
						C_CACHE_SLOT_P(val) = slot;
						op_array->cache_size += sizeof(zval);
					}
				}
				break;
			case CREX_FAST_CALL:
				opline->op1.opline_num = op_array->try_catch_array[opline->op1.num].finally_op;
				CREX_PASS_TWO_UPDATE_JMP_TARGET(op_array, opline, opline->op1);
				break;
			case CREX_BRK:
			case CREX_CONT:
				{
					uint32_t jmp_target = crex_get_brk_cont_target(op_array, opline);

					if (op_array->fn_flags & CREX_ACC_HAS_FINALLY_BLOCK) {
						crex_check_finally_breakout(op_array, opline - op_array->opcodes, jmp_target);
					}
					opline->opcode = CREX_JMP;
					opline->op1.opline_num = jmp_target;
					opline->op2.num = 0;
					CREX_PASS_TWO_UPDATE_JMP_TARGET(op_array, opline, opline->op1);
				}
				break;
			case CREX_GOTO:
				crex_resolve_goto_label(op_array, opline);
				if (op_array->fn_flags & CREX_ACC_HAS_FINALLY_BLOCK) {
					crex_check_finally_breakout(op_array, opline - op_array->opcodes, opline->op1.opline_num);
				}
				CREX_FALLTHROUGH;
			case CREX_JMP:
				CREX_PASS_TWO_UPDATE_JMP_TARGET(op_array, opline, opline->op1);
				break;
			case CREX_JMPZ:
			case CREX_JMPNZ:
			case CREX_JMPC_EX:
			case CREX_JMPNC_EX:
			case CREX_JMP_SET:
			case CREX_COALESCE:
			case CREX_FE_RESET_R:
			case CREX_FE_RESET_RW:
			case CREX_JMP_NULL:
			case CREX_BIND_INIT_STATIC_OR_JMP:
				CREX_PASS_TWO_UPDATE_JMP_TARGET(op_array, opline, opline->op2);
				break;
			case CREX_ASSERT_CHECK:
			{
				/* If result of assert is unused, result of check is unused as well */
				crex_op *call = &op_array->opcodes[opline->op2.opline_num - 1];
				if (call->opcode == CREX_EXT_FCALL_END) {
					call--;
				}
				if (call->result_type == IS_UNUSED) {
					opline->result_type = IS_UNUSED;
				}
				CREX_PASS_TWO_UPDATE_JMP_TARGET(op_array, opline, opline->op2);
				break;
			}
			case CREX_FE_FETCH_R:
			case CREX_FE_FETCH_RW:
				/* absolute index to relative offset */
				opline->extended_value = CREX_OPLINE_NUM_TO_OFFSET(op_array, opline, opline->extended_value);
				break;
			case CREX_CATCH:
				if (!(opline->extended_value & CREX_LAST_CATCH)) {
					CREX_PASS_TWO_UPDATE_JMP_TARGET(op_array, opline, opline->op2);
				}
				break;
			case CREX_RETURN:
			case CREX_RETURN_BY_REF:
				if (op_array->fn_flags & CREX_ACC_GENERATOR) {
					opline->opcode = CREX_GENERATOR_RETURN;
				}
				break;
			case CREX_SWITCH_LONG:
			case CREX_SWITCH_STRING:
			case CREX_MATCH:
			{
				/* absolute indexes to relative offsets */
				HashTable *jumptable = C_ARRVAL_P(CT_CONSTANT(opline->op2));
				zval *zv;
				CREX_HASH_FOREACH_VAL(jumptable, zv) {
					C_LVAL_P(zv) = CREX_OPLINE_NUM_TO_OFFSET(op_array, opline, C_LVAL_P(zv));
				} CREX_HASH_FOREACH_END();

				opline->extended_value = CREX_OPLINE_NUM_TO_OFFSET(op_array, opline, opline->extended_value);
				break;
			}
		}
		if (opline->op1_type == IS_CONST) {
			CREX_PASS_TWO_UPDATE_CONSTANT(op_array, opline, opline->op1);
		} else if (opline->op1_type & (IS_VAR|IS_TMP_VAR)) {
			opline->op1.var = EX_NUM_TO_VAR(op_array->last_var + opline->op1.var);
		}
		if (opline->op2_type == IS_CONST) {
			CREX_PASS_TWO_UPDATE_CONSTANT(op_array, opline, opline->op2);
		} else if (opline->op2_type & (IS_VAR|IS_TMP_VAR)) {
			opline->op2.var = EX_NUM_TO_VAR(op_array->last_var + opline->op2.var);
		}
		if (opline->result_type & (IS_VAR|IS_TMP_VAR)) {
			opline->result.var = EX_NUM_TO_VAR(op_array->last_var + opline->result.var);
		}
		CREX_VM_SET_OPCODE_HANDLER(opline);
		opline++;
	}

	crex_calc_live_ranges(op_array, NULL);

	return;
}

CREX_API unary_op_type get_unary_op(int opcode)
{
	switch (opcode) {
		case CREX_BW_NOT:
			return (unary_op_type) bitwise_not_function;
		case CREX_BOOL_NOT:
			return (unary_op_type) boolean_not_function;
		default:
			return (unary_op_type) NULL;
	}
}

CREX_API binary_op_type get_binary_op(int opcode)
{
	switch (opcode) {
		case CREX_ADD:
			return (binary_op_type) add_function;
		case CREX_SUB:
			return (binary_op_type) sub_function;
		case CREX_MUL:
			return (binary_op_type) mul_function;
		case CREX_POW:
			return (binary_op_type) pow_function;
		case CREX_DIV:
			return (binary_op_type) div_function;
		case CREX_MOD:
			return (binary_op_type) mod_function;
		case CREX_SL:
			return (binary_op_type) shift_left_function;
		case CREX_SR:
			return (binary_op_type) shift_right_function;
		case CREX_FAST_CONCAT:
		case CREX_CONCAT:
			return (binary_op_type) concat_function;
		case CREX_IS_IDENTICAL:
		case CREX_CASE_STRICT:
			return (binary_op_type) is_identical_function;
		case CREX_IS_NOT_IDENTICAL:
			return (binary_op_type) is_not_identical_function;
		case CREX_IS_EQUAL:
		case CREX_CASE:
			return (binary_op_type) is_equal_function;
		case CREX_IS_NOT_EQUAL:
			return (binary_op_type) is_not_equal_function;
		case CREX_IS_SMALLER:
			return (binary_op_type) is_smaller_function;
		case CREX_IS_SMALLER_OR_EQUAL:
			return (binary_op_type) is_smaller_or_equal_function;
		case CREX_SPACESHIP:
			return (binary_op_type) compare_function;
		case CREX_BW_OR:
			return (binary_op_type) bitwise_or_function;
		case CREX_BW_AND:
			return (binary_op_type) bitwise_and_function;
		case CREX_BW_XOR:
			return (binary_op_type) bitwise_xor_function;
		case CREX_BOOL_XOR:
			return (binary_op_type) boolean_xor_function;
		default:
			CREX_UNREACHABLE();
			return (binary_op_type) NULL;
	}
}
