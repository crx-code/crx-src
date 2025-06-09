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
   | Authors: Bob Weinand <bwoebi@crx.net>                                |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#include "crex_ast.h"
#include "crex_API.h"
#include "crex_operators.h"
#include "crex_language_parser.h"
#include "crex_smart_str.h"
#include "crex_exceptions.h"
#include "crex_constants.h"
#include "crex_enum.h"

CREX_API crex_ast_process_t crex_ast_process = NULL;

static inline void *crex_ast_alloc(size_t size) {
	return crex_arena_alloc(&CG(ast_arena), size);
}

static inline void *crex_ast_realloc(void *old, size_t old_size, size_t new_size) {
	void *new = crex_ast_alloc(new_size);
	memcpy(new, old, old_size);
	return new;
}

static inline size_t crex_ast_list_size(uint32_t children) {
	return sizeof(crex_ast_list) - sizeof(crex_ast *) + sizeof(crex_ast *) * children;
}

CREX_API crex_ast * CREX_FASTCALL crex_ast_create_znode(znode *node) {
	crex_ast_znode *ast;

	ast = crex_ast_alloc(sizeof(crex_ast_znode));
	ast->kind = CREX_AST_ZNODE;
	ast->attr = 0;
	ast->lineno = CG(crex_lineno);
	ast->node = *node;
	return (crex_ast *) ast;
}

static crex_always_inline crex_ast * crex_ast_create_zval_int(zval *zv, uint32_t attr, uint32_t lineno) {
	crex_ast_zval *ast;

	ast = crex_ast_alloc(sizeof(crex_ast_zval));
	ast->kind = CREX_AST_ZVAL;
	ast->attr = attr;
	ZVAL_COPY_VALUE(&ast->val, zv);
	C_LINENO(ast->val) = lineno;
	return (crex_ast *) ast;
}

CREX_API crex_ast * CREX_FASTCALL crex_ast_create_zval_with_lineno(zval *zv, uint32_t lineno) {
	return crex_ast_create_zval_int(zv, 0, lineno);
}

CREX_API crex_ast * CREX_FASTCALL crex_ast_create_zval_ex(zval *zv, crex_ast_attr attr) {
	return crex_ast_create_zval_int(zv, attr, CG(crex_lineno));
}

CREX_API crex_ast * CREX_FASTCALL crex_ast_create_zval(zval *zv) {
	return crex_ast_create_zval_int(zv, 0, CG(crex_lineno));
}

CREX_API crex_ast * CREX_FASTCALL crex_ast_create_zval_from_str(crex_string *str) {
	zval zv;
	ZVAL_STR(&zv, str);
	return crex_ast_create_zval_int(&zv, 0, CG(crex_lineno));
}

CREX_API crex_ast * CREX_FASTCALL crex_ast_create_zval_from_long(crex_long lval) {
	zval zv;
	ZVAL_LONG(&zv, lval);
	return crex_ast_create_zval_int(&zv, 0, CG(crex_lineno));
}

CREX_API crex_ast * CREX_FASTCALL crex_ast_create_constant(crex_string *name, crex_ast_attr attr) {
	crex_ast_zval *ast;

	ast = crex_ast_alloc(sizeof(crex_ast_zval));
	ast->kind = CREX_AST_CONSTANT;
	ast->attr = attr;
	ZVAL_STR(&ast->val, name);
	C_LINENO(ast->val) = CG(crex_lineno);
	return (crex_ast *) ast;
}

CREX_API crex_ast * CREX_FASTCALL crex_ast_create_class_const_or_name(crex_ast *class_name, crex_ast *name) {
	crex_string *name_str = crex_ast_get_str(name);
	if (crex_string_equals_ci(name_str, ZSTR_KNOWN(CREX_STR_CLASS))) {
		crex_string_release(name_str);
		return crex_ast_create(CREX_AST_CLASS_NAME, class_name);
	} else {
		return crex_ast_create(CREX_AST_CLASS_CONST, class_name, name);
	}
}

CREX_API crex_ast *crex_ast_create_decl(
	crex_ast_kind kind, uint32_t flags, uint32_t start_lineno, crex_string *doc_comment,
	crex_string *name, crex_ast *child0, crex_ast *child1, crex_ast *child2, crex_ast *child3, crex_ast *child4
) {
	crex_ast_decl *ast;

	ast = crex_ast_alloc(sizeof(crex_ast_decl));
	ast->kind = kind;
	ast->attr = 0;
	ast->start_lineno = start_lineno;
	ast->end_lineno = CG(crex_lineno);
	ast->flags = flags;
	ast->doc_comment = doc_comment;
	ast->name = name;
	ast->child[0] = child0;
	ast->child[1] = child1;
	ast->child[2] = child2;
	ast->child[3] = child3;
	ast->child[4] = child4;

	return (crex_ast *) ast;
}

#if CREX_AST_SPEC
CREX_API crex_ast * CREX_FASTCALL crex_ast_create_0(crex_ast_kind kind) {
	crex_ast *ast;

	CREX_ASSERT(kind >> CREX_AST_NUM_CHILDREN_SHIFT == 0);
	ast = crex_ast_alloc(crex_ast_size(0));
	ast->kind = kind;
	ast->attr = 0;
	ast->lineno = CG(crex_lineno);

	return ast;
}

CREX_API crex_ast * CREX_FASTCALL crex_ast_create_1(crex_ast_kind kind, crex_ast *child) {
	crex_ast *ast;
	uint32_t lineno;

	CREX_ASSERT(kind >> CREX_AST_NUM_CHILDREN_SHIFT == 1);
	ast = crex_ast_alloc(crex_ast_size(1));
	ast->kind = kind;
	ast->attr = 0;
	ast->child[0] = child;
	if (child) {
		lineno = crex_ast_get_lineno(child);
	} else {
		lineno = CG(crex_lineno);
	}
	ast->lineno = lineno;

	return ast;
}

CREX_API crex_ast * CREX_FASTCALL crex_ast_create_2(crex_ast_kind kind, crex_ast *child1, crex_ast *child2) {
	crex_ast *ast;
	uint32_t lineno;

	CREX_ASSERT(kind >> CREX_AST_NUM_CHILDREN_SHIFT == 2);
	ast = crex_ast_alloc(crex_ast_size(2));
	ast->kind = kind;
	ast->attr = 0;
	ast->child[0] = child1;
	ast->child[1] = child2;
	if (child1) {
		lineno = crex_ast_get_lineno(child1);
	} else if (child2) {
		lineno = crex_ast_get_lineno(child2);
	} else {
		lineno = CG(crex_lineno);
	}
	ast->lineno = lineno;

	return ast;
}

CREX_API crex_ast * CREX_FASTCALL crex_ast_create_3(crex_ast_kind kind, crex_ast *child1, crex_ast *child2, crex_ast *child3) {
	crex_ast *ast;
	uint32_t lineno;

	CREX_ASSERT(kind >> CREX_AST_NUM_CHILDREN_SHIFT == 3);
	ast = crex_ast_alloc(crex_ast_size(3));
	ast->kind = kind;
	ast->attr = 0;
	ast->child[0] = child1;
	ast->child[1] = child2;
	ast->child[2] = child3;
	if (child1) {
		lineno = crex_ast_get_lineno(child1);
	} else if (child2) {
		lineno = crex_ast_get_lineno(child2);
	} else if (child3) {
		lineno = crex_ast_get_lineno(child3);
	} else {
		lineno = CG(crex_lineno);
	}
	ast->lineno = lineno;

	return ast;
}

CREX_API crex_ast * CREX_FASTCALL crex_ast_create_4(crex_ast_kind kind, crex_ast *child1, crex_ast *child2, crex_ast *child3, crex_ast *child4) {
	crex_ast *ast;
	uint32_t lineno;

	CREX_ASSERT(kind >> CREX_AST_NUM_CHILDREN_SHIFT == 4);
	ast = crex_ast_alloc(crex_ast_size(4));
	ast->kind = kind;
	ast->attr = 0;
	ast->child[0] = child1;
	ast->child[1] = child2;
	ast->child[2] = child3;
	ast->child[3] = child4;
	if (child1) {
		lineno = crex_ast_get_lineno(child1);
	} else if (child2) {
		lineno = crex_ast_get_lineno(child2);
	} else if (child3) {
		lineno = crex_ast_get_lineno(child3);
	} else if (child4) {
		lineno = crex_ast_get_lineno(child4);
	} else {
		lineno = CG(crex_lineno);
	}
	ast->lineno = lineno;

	return ast;
}

CREX_API crex_ast * CREX_FASTCALL crex_ast_create_5(crex_ast_kind kind, crex_ast *child1, crex_ast *child2, crex_ast *child3, crex_ast *child4, crex_ast *child5) {
	crex_ast *ast;
	uint32_t lineno;

	CREX_ASSERT(kind >> CREX_AST_NUM_CHILDREN_SHIFT == 5);
	ast = crex_ast_alloc(crex_ast_size(5));
	ast->kind = kind;
	ast->attr = 0;
	ast->child[0] = child1;
	ast->child[1] = child2;
	ast->child[2] = child3;
	ast->child[3] = child4;
	ast->child[4] = child5;
	if (child1) {
		lineno = crex_ast_get_lineno(child1);
	} else if (child2) {
		lineno = crex_ast_get_lineno(child2);
	} else if (child3) {
		lineno = crex_ast_get_lineno(child3);
	} else if (child4) {
		lineno = crex_ast_get_lineno(child4);
	} else if (child5) {
		lineno = crex_ast_get_lineno(child5);
	} else {
		lineno = CG(crex_lineno);
	}
	ast->lineno = lineno;

	return ast;
}

CREX_API crex_ast * CREX_FASTCALL crex_ast_create_list_0(crex_ast_kind kind) {
	crex_ast *ast;
	crex_ast_list *list;

	ast = crex_ast_alloc(crex_ast_list_size(4));
	list = (crex_ast_list *) ast;
	list->kind = kind;
	list->attr = 0;
	list->lineno = CG(crex_lineno);
	list->children = 0;

	return ast;
}

CREX_API crex_ast * CREX_FASTCALL crex_ast_create_list_1(crex_ast_kind kind, crex_ast *child) {
	crex_ast *ast;
	crex_ast_list *list;
	uint32_t lineno;

	ast = crex_ast_alloc(crex_ast_list_size(4));
	list = (crex_ast_list *) ast;
	list->kind = kind;
	list->attr = 0;
	list->children = 1;
	list->child[0] = child;
	if (child) {
		lineno = crex_ast_get_lineno(child);
		if (lineno > CG(crex_lineno)) {
			lineno = CG(crex_lineno);
		}
	} else {
		lineno = CG(crex_lineno);
	}
	list->lineno = lineno;

	return ast;
}

CREX_API crex_ast * CREX_FASTCALL crex_ast_create_list_2(crex_ast_kind kind, crex_ast *child1, crex_ast *child2) {
	crex_ast *ast;
	crex_ast_list *list;
	uint32_t lineno;

	ast = crex_ast_alloc(crex_ast_list_size(4));
	list = (crex_ast_list *) ast;
	list->kind = kind;
	list->attr = 0;
	list->children = 2;
	list->child[0] = child1;
	list->child[1] = child2;
	if (child1) {
		lineno = crex_ast_get_lineno(child1);
		if (lineno > CG(crex_lineno)) {
			lineno = CG(crex_lineno);
		}
	} else if (child2) {
		lineno = crex_ast_get_lineno(child2);
		if (lineno > CG(crex_lineno)) {
			lineno = CG(crex_lineno);
		}
	} else {
		list->children = 0;
		lineno = CG(crex_lineno);
	}
	list->lineno = lineno;

	return ast;
}
#else
static crex_ast *crex_ast_create_from_va_list(crex_ast_kind kind, crex_ast_attr attr, va_list va) {
	uint32_t i, children = kind >> CREX_AST_NUM_CHILDREN_SHIFT;
	crex_ast *ast;

	ast = crex_ast_alloc(crex_ast_size(children));
	ast->kind = kind;
	ast->attr = attr;
	ast->lineno = (uint32_t) -1;

	for (i = 0; i < children; ++i) {
		ast->child[i] = va_arg(va, crex_ast *);
		if (ast->child[i] != NULL) {
			uint32_t lineno = crex_ast_get_lineno(ast->child[i]);
			if (lineno < ast->lineno) {
				ast->lineno = lineno;
			}
		}
	}

	if (ast->lineno == UINT_MAX) {
		ast->lineno = CG(crex_lineno);
	}

	return ast;
}

CREX_API crex_ast *crex_ast_create_ex(crex_ast_kind kind, crex_ast_attr attr, ...) {
	va_list va;
	crex_ast *ast;

	va_start(va, attr);
	ast = crex_ast_create_from_va_list(kind, attr, va);
	va_end(va);

	return ast;
}

CREX_API crex_ast *crex_ast_create(crex_ast_kind kind, ...) {
	va_list va;
	crex_ast *ast;

	va_start(va, kind);
	ast = crex_ast_create_from_va_list(kind, 0, va);
	va_end(va);

	return ast;
}

CREX_API crex_ast *crex_ast_create_list(uint32_t init_children, crex_ast_kind kind, ...) {
	crex_ast *ast;
	crex_ast_list *list;

	ast = crex_ast_alloc(crex_ast_list_size(4));
	list = (crex_ast_list *) ast;
	list->kind = kind;
	list->attr = 0;
	list->lineno = CG(crex_lineno);
	list->children = 0;

	{
		va_list va;
		uint32_t i;
		va_start(va, kind);
		for (i = 0; i < init_children; ++i) {
			crex_ast *child = va_arg(va, crex_ast *);
			ast = crex_ast_list_add(ast, child);
			if (child != NULL) {
				uint32_t lineno = crex_ast_get_lineno(child);
				if (lineno < ast->lineno) {
					ast->lineno = lineno;
				}
			}
		}
		va_end(va);
	}

	return ast;
}
#endif

crex_ast *crex_ast_create_concat_op(crex_ast *op0, crex_ast *op1) {
	if (op0->kind == CREX_AST_ZVAL && op1->kind == CREX_AST_ZVAL) {
		zval *zv0 = crex_ast_get_zval(op0);
		zval *zv1 = crex_ast_get_zval(op1);
		if (!crex_binary_op_produces_error(CREX_CONCAT, zv0, zv1) &&
				concat_function(zv0, zv0, zv1) == SUCCESS) {
			zval_ptr_dtor_nogc(zv1);
			return crex_ast_create_zval(zv0);
		}
	}
	return crex_ast_create_binary_op(CREX_CONCAT, op0, op1);
}

static inline bool is_power_of_two(uint32_t n) {
	return ((n != 0) && (n == (n & (~n + 1))));
}

CREX_API crex_ast * CREX_FASTCALL crex_ast_list_add(crex_ast *ast, crex_ast *op) {
	crex_ast_list *list = crex_ast_get_list(ast);
	if (list->children >= 4 && is_power_of_two(list->children)) {
			list = crex_ast_realloc(list,
			crex_ast_list_size(list->children), crex_ast_list_size(list->children * 2));
	}
	list->child[list->children++] = op;
	return (crex_ast *) list;
}

static crex_result crex_ast_add_array_element(zval *result, zval *offset, zval *expr)
{
	if (C_TYPE_P(offset) == IS_UNDEF) {
		if (!crex_hash_next_index_insert(C_ARRVAL_P(result), expr)) {
			crex_throw_error(NULL,
				"Cannot add element to the array as the next element is already occupied");
			return FAILURE;
		}
		return SUCCESS;
	}

	if (array_set_zval_key(C_ARRVAL_P(result), offset, expr) == FAILURE) {
		return FAILURE;
	}

	zval_ptr_dtor_nogc(offset);
	zval_ptr_dtor_nogc(expr);
	return SUCCESS;
}

static crex_result crex_ast_add_unpacked_element(zval *result, zval *expr) {
	if (EXPECTED(C_TYPE_P(expr) == IS_ARRAY)) {
		HashTable *ht = C_ARRVAL_P(expr);
		zval *val;
		crex_string *key;

		CREX_HASH_FOREACH_STR_KEY_VAL(ht, key, val) {
			if (key) {
				crex_hash_update(C_ARRVAL_P(result), key, val);
			} else {
				if (!crex_hash_next_index_insert(C_ARRVAL_P(result), val)) {
					crex_throw_error(NULL,
						"Cannot add element to the array as the next element is already occupied");
					return FAILURE;
				}
			}
			C_TRY_ADDREF_P(val);
		} CREX_HASH_FOREACH_END();
		return SUCCESS;
	}

	crex_throw_error(NULL, "Only arrays can be unpacked in constant expression");
	return FAILURE;
}

crex_class_entry *crex_ast_fetch_class(crex_ast *ast, crex_class_entry *scope)
{
	return crex_fetch_class_with_scope(crex_ast_get_str(ast), (ast->attr >> CREX_CONST_EXPR_NEW_FETCH_TYPE_SHIFT) | CREX_FETCH_CLASS_EXCEPTION, scope);
}

CREX_API crex_result CREX_FASTCALL crex_ast_evaluate_inner(
	zval *result,
	crex_ast *ast,
	crex_class_entry *scope,
	bool *short_circuited_ptr,
	crex_ast_evaluate_ctx *ctx
);

CREX_API crex_result CREX_FASTCALL crex_ast_evaluate_ex(
	zval *result,
	crex_ast *ast,
	crex_class_entry *scope,
	bool *short_circuited_ptr,
	crex_ast_evaluate_ctx *ctx
) {
	crex_string *previous_filename;
	crex_long previous_lineno;
	if (scope) {
		previous_filename = EG(filename_override);
		previous_lineno = EG(lineno_override);
		EG(filename_override) = scope->info.user.filename;
		EG(lineno_override) = crex_ast_get_lineno(ast);
	}
	crex_result r = crex_ast_evaluate_inner(result, ast, scope, short_circuited_ptr, ctx);
	if (scope) {
		EG(filename_override) = previous_filename;
		EG(lineno_override) = previous_lineno;
	}
	return r;
}

CREX_API crex_result CREX_FASTCALL crex_ast_evaluate_inner(
	zval *result,
	crex_ast *ast,
	crex_class_entry *scope,
	bool *short_circuited_ptr,
	crex_ast_evaluate_ctx *ctx
) {
	zval op1, op2;
	crex_result ret = SUCCESS;
	bool short_circuited;
	*short_circuited_ptr = false;

	switch (ast->kind) {
		case CREX_AST_BINARY_OP:
			if (UNEXPECTED(crex_ast_evaluate_ex(&op1, ast->child[0], scope, &short_circuited, ctx) != SUCCESS)) {
				ret = FAILURE;
			} else if (UNEXPECTED(crex_ast_evaluate_ex(&op2, ast->child[1], scope, &short_circuited, ctx) != SUCCESS)) {
				zval_ptr_dtor_nogc(&op1);
				ret = FAILURE;
			} else {
				binary_op_type op = get_binary_op(ast->attr);
				ret = op(result, &op1, &op2);
				zval_ptr_dtor_nogc(&op1);
				zval_ptr_dtor_nogc(&op2);
			}
			break;
		case CREX_AST_GREATER:
		case CREX_AST_GREATER_EQUAL:
			if (UNEXPECTED(crex_ast_evaluate_ex(&op1, ast->child[0], scope, &short_circuited, ctx) != SUCCESS)) {
				ret = FAILURE;
			} else if (UNEXPECTED(crex_ast_evaluate_ex(&op2, ast->child[1], scope, &short_circuited, ctx) != SUCCESS)) {
				zval_ptr_dtor_nogc(&op1);
				ret = FAILURE;
			} else {
				/* op1 > op2 is the same as op2 < op1 */
				binary_op_type op = ast->kind == CREX_AST_GREATER
					? is_smaller_function : is_smaller_or_equal_function;
				ret = op(result, &op2, &op1);
				zval_ptr_dtor_nogc(&op1);
				zval_ptr_dtor_nogc(&op2);
			}
			break;
		case CREX_AST_UNARY_OP:
			if (UNEXPECTED(crex_ast_evaluate_ex(&op1, ast->child[0], scope, &short_circuited, ctx) != SUCCESS)) {
				ret = FAILURE;
			} else {
				unary_op_type op = get_unary_op(ast->attr);
				ret = op(result, &op1);
				zval_ptr_dtor_nogc(&op1);
			}
			break;
		case CREX_AST_ZVAL:
		{
			zval *zv = crex_ast_get_zval(ast);

			ZVAL_COPY(result, zv);
			break;
		}
		case CREX_AST_CONSTANT:
		{
			crex_string *name = crex_ast_get_constant_name(ast);
			zval *zv = crex_get_constant_ex(name, scope, ast->attr);

			if (UNEXPECTED(zv == NULL)) {
				ZVAL_UNDEF(result);
				return FAILURE;
			}
			ZVAL_COPY_OR_DUP(result, zv);
			break;
		}
		case CREX_AST_CONSTANT_CLASS:
			if (scope) {
				ZVAL_STR_COPY(result, scope->name);
			} else {
				ZVAL_EMPTY_STRING(result);
			}
			break;
		case CREX_AST_CLASS_NAME:
			if (!scope) {
				crex_throw_error(NULL, "Cannot use \"self\" when no class scope is active");
				return FAILURE;
			}
			if (ast->attr == CREX_FETCH_CLASS_SELF) {
				ZVAL_STR_COPY(result, scope->name);
			} else if (ast->attr == CREX_FETCH_CLASS_PARENT) {
				if (!scope->parent) {
					crex_throw_error(NULL,
						"Cannot use \"parent\" when current class scope has no parent");
					return FAILURE;
				}
				ZVAL_STR_COPY(result, scope->parent->name);
			} else {
				CREX_ASSERT(0 && "Should have errored during compilation");
			}
			break;
		case CREX_AST_AND:
			if (UNEXPECTED(crex_ast_evaluate_ex(&op1, ast->child[0], scope, &short_circuited, ctx) != SUCCESS)) {
				ret = FAILURE;
				break;
			}
			if (crex_is_true(&op1)) {
				if (UNEXPECTED(crex_ast_evaluate_ex(&op2, ast->child[1], scope, &short_circuited, ctx) != SUCCESS)) {
					zval_ptr_dtor_nogc(&op1);
					ret = FAILURE;
					break;
				}
				ZVAL_BOOL(result, crex_is_true(&op2));
				zval_ptr_dtor_nogc(&op2);
			} else {
				ZVAL_FALSE(result);
			}
			zval_ptr_dtor_nogc(&op1);
			break;
		case CREX_AST_OR:
			if (UNEXPECTED(crex_ast_evaluate_ex(&op1, ast->child[0], scope, &short_circuited, ctx) != SUCCESS)) {
				ret = FAILURE;
				break;
			}
			if (crex_is_true(&op1)) {
				ZVAL_TRUE(result);
			} else {
				if (UNEXPECTED(crex_ast_evaluate_ex(&op2, ast->child[1], scope, &short_circuited, ctx) != SUCCESS)) {
					zval_ptr_dtor_nogc(&op1);
					ret = FAILURE;
					break;
				}
				ZVAL_BOOL(result, crex_is_true(&op2));
				zval_ptr_dtor_nogc(&op2);
			}
			zval_ptr_dtor_nogc(&op1);
			break;
		case CREX_AST_CONDITIONAL:
			if (UNEXPECTED(crex_ast_evaluate_ex(&op1, ast->child[0], scope, &short_circuited, ctx) != SUCCESS)) {
				ret = FAILURE;
				break;
			}
			if (crex_is_true(&op1)) {
				if (!ast->child[1]) {
					*result = op1;
				} else {
					if (UNEXPECTED(crex_ast_evaluate_ex(result, ast->child[1], scope, &short_circuited, ctx) != SUCCESS)) {
						zval_ptr_dtor_nogc(&op1);
						ret = FAILURE;
						break;
					}
					zval_ptr_dtor_nogc(&op1);
				}
			} else {
				if (UNEXPECTED(crex_ast_evaluate_ex(result, ast->child[2], scope, &short_circuited, ctx) != SUCCESS)) {
					zval_ptr_dtor_nogc(&op1);
					ret = FAILURE;
					break;
				}
				zval_ptr_dtor_nogc(&op1);
			}
			break;
		case CREX_AST_COALESCE:
			if (UNEXPECTED(crex_ast_evaluate_ex(&op1, ast->child[0], scope, &short_circuited, ctx) != SUCCESS)) {
				ret = FAILURE;
				break;
			}
			if (C_TYPE(op1) > IS_NULL) {
				*result = op1;
			} else {
				if (UNEXPECTED(crex_ast_evaluate_ex(result, ast->child[1], scope, &short_circuited, ctx) != SUCCESS)) {
					zval_ptr_dtor_nogc(&op1);
					ret = FAILURE;
					break;
				}
				zval_ptr_dtor_nogc(&op1);
			}
			break;
		case CREX_AST_UNARY_PLUS:
			if (UNEXPECTED(crex_ast_evaluate_ex(&op2, ast->child[0], scope, &short_circuited, ctx) != SUCCESS)) {
				ret = FAILURE;
			} else {
				ZVAL_LONG(&op1, 0);
				ret = add_function(result, &op1, &op2);
				zval_ptr_dtor_nogc(&op2);
			}
			break;
		case CREX_AST_UNARY_MINUS:
			if (UNEXPECTED(crex_ast_evaluate_ex(&op2, ast->child[0], scope, &short_circuited, ctx) != SUCCESS)) {
				ret = FAILURE;
			} else {
				ZVAL_LONG(&op1, -1);
				ret = mul_function(result, &op1, &op2);
				zval_ptr_dtor_nogc(&op2);
			}
			break;
		case CREX_AST_ARRAY:
			{
				uint32_t i;
				crex_ast_list *list = crex_ast_get_list(ast);

				if (!list->children) {
					ZVAL_EMPTY_ARRAY(result);
					break;
				}
				array_init(result);
				for (i = 0; i < list->children; i++) {
					crex_ast *elem = list->child[i];
					if (elem->kind == CREX_AST_UNPACK) {
						if (UNEXPECTED(crex_ast_evaluate_ex(&op1, elem->child[0], scope, &short_circuited, ctx) != SUCCESS)) {
							zval_ptr_dtor_nogc(result);
							return FAILURE;
						}
						if (UNEXPECTED(crex_ast_add_unpacked_element(result, &op1) != SUCCESS)) {
							zval_ptr_dtor_nogc(&op1);
							zval_ptr_dtor_nogc(result);
							return FAILURE;
						}
						zval_ptr_dtor_nogc(&op1);
						continue;
					}
					if (elem->child[1]) {
						if (UNEXPECTED(crex_ast_evaluate_ex(&op1, elem->child[1], scope, &short_circuited, ctx) != SUCCESS)) {
							zval_ptr_dtor_nogc(result);
							return FAILURE;
						}
					} else {
						ZVAL_UNDEF(&op1);
					}
					if (UNEXPECTED(crex_ast_evaluate_ex(&op2, elem->child[0], scope, &short_circuited, ctx) != SUCCESS)) {
						zval_ptr_dtor_nogc(&op1);
						zval_ptr_dtor_nogc(result);
						return FAILURE;
					}
					if (UNEXPECTED(crex_ast_add_array_element(result, &op1, &op2) != SUCCESS)) {
						zval_ptr_dtor_nogc(&op1);
						zval_ptr_dtor_nogc(&op2);
						zval_ptr_dtor_nogc(result);
						return FAILURE;
					}
				}
			}
			break;
		case CREX_AST_DIM:
			if (ast->child[1] == NULL) {
				crex_error_noreturn(E_COMPILE_ERROR, "Cannot use [] for reading");
			}

			if (UNEXPECTED(crex_ast_evaluate_ex(&op1, ast->child[0], scope, &short_circuited, ctx) != SUCCESS)) {
				ret = FAILURE;
				break;
			}
			if (short_circuited) {
				*short_circuited_ptr = true;
				ZVAL_NULL(result);
				return SUCCESS;
			}

			// DIM on objects is disallowed because it allows executing arbitrary expressions
			if (C_TYPE(op1) == IS_OBJECT) {
				zval_ptr_dtor_nogc(&op1);
				crex_throw_error(NULL, "Cannot use [] on objects in constant expression");
				ret = FAILURE;
				break;
			}

			if (UNEXPECTED(crex_ast_evaluate_ex(&op2, ast->child[1], scope, &short_circuited, ctx) != SUCCESS)) {
				zval_ptr_dtor_nogc(&op1);
				ret = FAILURE;
				break;
			}

			crex_fetch_dimension_const(result, &op1, &op2, (ast->attr & CREX_DIM_IS) ? BP_VAR_IS : BP_VAR_R);

			zval_ptr_dtor_nogc(&op1);
			zval_ptr_dtor_nogc(&op2);
			if (UNEXPECTED(EG(exception))) {
				return FAILURE;
			}

			break;
		case CREX_AST_CONST_ENUM_INIT:
		{
			// Preloading will attempt to resolve constants but objects can't be stored in shm
			// Aborting here to store the const AST instead
			if (CG(in_compilation)) {
				return FAILURE;
			}

			crex_ast *class_name_ast = ast->child[0];
			crex_string *class_name = crex_ast_get_str(class_name_ast);

			crex_ast *case_name_ast = ast->child[1];
			crex_string *case_name = crex_ast_get_str(case_name_ast);

			crex_ast *case_value_ast = ast->child[2];

			zval case_value_zv;
			ZVAL_UNDEF(&case_value_zv);
			if (case_value_ast != NULL) {
				if (UNEXPECTED(crex_ast_evaluate_ex(&case_value_zv, case_value_ast, scope, &short_circuited, ctx) != SUCCESS)) {
					return FAILURE;
				}
			}

			crex_class_entry *ce = crex_lookup_class(class_name);
			crex_enum_new(result, ce, case_name, case_value_ast != NULL ? &case_value_zv : NULL);
			zval_ptr_dtor_nogc(&case_value_zv);
			break;
		}
		case CREX_AST_CLASS_CONST:
		{
			crex_string *class_name = crex_ast_get_str(ast->child[0]);
			if (UNEXPECTED(crex_ast_evaluate_ex(&op2, ast->child[1], scope, &short_circuited, ctx) != SUCCESS)) {
				return FAILURE;
			}
			if (UNEXPECTED(C_TYPE(op2) != IS_STRING)) {
				crex_invalid_class_constant_type_error(C_TYPE(op2));
				zval_ptr_dtor_nogc(&op2);
				return FAILURE;
			}
			crex_string *const_name = C_STR(op2);

			crex_string *previous_filename;
			crex_long previous_lineno;
			if (scope) {
				previous_filename = EG(filename_override);
				previous_lineno = EG(lineno_override);
				EG(filename_override) = scope->info.user.filename;
				EG(lineno_override) = crex_ast_get_lineno(ast);
			}
			zval *zv = crex_get_class_constant_ex(class_name, const_name, scope, ast->attr);
			if (scope) {
				EG(filename_override) = previous_filename;
				EG(lineno_override) = previous_lineno;
			}

			if (UNEXPECTED(zv == NULL)) {
				ZVAL_UNDEF(result);
				zval_ptr_dtor_nogc(&op2);
				return FAILURE;
			}
			ZVAL_COPY_OR_DUP(result, zv);
			zval_ptr_dtor_nogc(&op2);
			break;
		}
		case CREX_AST_NEW:
		{
			crex_class_entry *ce = crex_ast_fetch_class(ast->child[0], scope);
			if (!ce) {
				return FAILURE;
			}

			if (object_init_ex(result, ce) != SUCCESS) {
				return FAILURE;
			}

			/* Even if there is no constructor, the object can have cause side-effects in various ways (__toString(), __get(), __isset(), etc). */
			ctx->had_side_effects = true;

			crex_ast_list *args_ast = crex_ast_get_list(ast->child[1]);
			if (args_ast->attr) {
				/* Has named arguments. */
				HashTable *args = crex_new_array(args_ast->children);
				for (uint32_t i = 0; i < args_ast->children; i++) {
					crex_ast *arg_ast = args_ast->child[i];
					crex_string *name = NULL;
					zval arg;
					if (arg_ast->kind == CREX_AST_NAMED_ARG) {
						name = crex_ast_get_str(arg_ast->child[0]);
						arg_ast = arg_ast->child[1];
					}
					if (crex_ast_evaluate_ex(&arg, arg_ast, scope, &short_circuited, ctx) == FAILURE) {
						crex_array_destroy(args);
						zval_ptr_dtor(result);
						return FAILURE;
					}
					if (name) {
						if (!crex_hash_add(args, name, &arg)) {
							crex_throw_error(NULL,
								"Named parameter $%s overwrites previous argument",
								ZSTR_VAL(name));
							crex_array_destroy(args);
							zval_ptr_dtor(result);
							return FAILURE;
						}
					} else {
						crex_hash_next_index_insert(args, &arg);
					}
				}

				crex_function *ctor = C_OBJ_HT_P(result)->get_constructor(C_OBJ_P(result));
				if (ctor) {
					crex_call_known_function(
						ctor, C_OBJ_P(result), C_OBJCE_P(result), NULL, 0, NULL, args);
				}

				crex_array_destroy(args);
			} else {
				ALLOCA_FLAG(use_heap)
				zval *args = do_alloca(sizeof(zval) * args_ast->children, use_heap);
				for (uint32_t i = 0; i < args_ast->children; i++) {
					if (crex_ast_evaluate_ex(&args[i], args_ast->child[i], scope, &short_circuited, ctx) == FAILURE) {
						for (uint32_t j = 0; j < i; j++) {
							zval_ptr_dtor(&args[j]);
						}
						free_alloca(args, use_heap);
						zval_ptr_dtor(result);
						return FAILURE;
					}
				}

				crex_function *ctor = C_OBJ_HT_P(result)->get_constructor(C_OBJ_P(result));
				if (ctor) {
					crex_call_known_instance_method(
						ctor, C_OBJ_P(result), NULL, args_ast->children, args);
				}

				for (uint32_t i = 0; i < args_ast->children; i++) {
					zval_ptr_dtor(&args[i]);
				}
				free_alloca(args, use_heap);
			}

			if (EG(exception)) {
				crex_object_store_ctor_failed(C_OBJ_P(result));
				zval_ptr_dtor(result);
				return FAILURE;
			}
			return SUCCESS;
		}
		case CREX_AST_PROP:
		case CREX_AST_NULLSAFE_PROP:
		{
			if (UNEXPECTED(crex_ast_evaluate_ex(&op1, ast->child[0], scope, &short_circuited, ctx) != SUCCESS)) {
				return FAILURE;
			}
			if (short_circuited) {
				*short_circuited_ptr = true;
				ZVAL_NULL(result);
				return SUCCESS;
			}
			if (ast->kind == CREX_AST_NULLSAFE_PROP && C_TYPE(op1) == IS_NULL) {
				*short_circuited_ptr = true;
				ZVAL_NULL(result);
				return SUCCESS;
			}

			if (UNEXPECTED(crex_ast_evaluate_ex(&op2, ast->child[1], scope, &short_circuited, ctx) != SUCCESS)) {
				zval_ptr_dtor_nogc(&op1);
				return FAILURE;
			}

			if (!try_convert_to_string(&op2)) {
				zval_ptr_dtor_nogc(&op1);
				zval_ptr_dtor_nogc(&op2);
				return FAILURE;
			}

			if (C_TYPE(op1) != IS_OBJECT) {
				crex_wrong_property_read(&op1, &op2);

				zval_ptr_dtor_nogc(&op1);
				zval_ptr_dtor_nogc(&op2);

				ZVAL_NULL(result);
				return SUCCESS;
			}

			crex_object *zobj = C_OBJ(op1);
			if (!(zobj->ce->ce_flags & CREX_ACC_ENUM)) {
				crex_throw_error(NULL, "Fetching properties on non-enums in constant expressions is not allowed");
				zval_ptr_dtor_nogc(&op1);
				zval_ptr_dtor_nogc(&op2);
				return FAILURE;
			}

			crex_string *name = C_STR(op2);
			zval *property_result = crex_read_property_ex(scope, zobj, name, 0, result);
			if (EG(exception)) {
				zval_ptr_dtor_nogc(&op1);
				zval_ptr_dtor_nogc(&op2);
				return FAILURE;
			}

			if (result != property_result) {
				ZVAL_COPY(result, property_result);
			}
			zval_ptr_dtor_nogc(&op1);
			zval_ptr_dtor_nogc(&op2);
			return SUCCESS;
		}
		default:
			crex_throw_error(NULL, "Unsupported constant expression");
			ret = FAILURE;
	}
	return ret;
}

CREX_API crex_result CREX_FASTCALL crex_ast_evaluate(zval *result, crex_ast *ast, crex_class_entry *scope)
{
	crex_ast_evaluate_ctx ctx = {0};
	bool short_circuited;
	return crex_ast_evaluate_ex(result, ast, scope, &short_circuited, &ctx);
}

static size_t CREX_FASTCALL crex_ast_tree_size(crex_ast *ast)
{
	size_t size;

	if (ast->kind == CREX_AST_ZVAL || ast->kind == CREX_AST_CONSTANT) {
		size = sizeof(crex_ast_zval);
	} else if (crex_ast_is_list(ast)) {
		uint32_t i;
		crex_ast_list *list = crex_ast_get_list(ast);

		size = crex_ast_list_size(list->children);
		for (i = 0; i < list->children; i++) {
			if (list->child[i]) {
				size += crex_ast_tree_size(list->child[i]);
			}
		}
	} else {
		uint32_t i, children = crex_ast_get_num_children(ast);

		size = crex_ast_size(children);
		for (i = 0; i < children; i++) {
			if (ast->child[i]) {
				size += crex_ast_tree_size(ast->child[i]);
			}
		}
	}
	return size;
}

static void* CREX_FASTCALL crex_ast_tree_copy(crex_ast *ast, void *buf)
{
	if (ast->kind == CREX_AST_ZVAL) {
		crex_ast_zval *new = (crex_ast_zval*)buf;
		new->kind = CREX_AST_ZVAL;
		new->attr = ast->attr;
		ZVAL_COPY(&new->val, crex_ast_get_zval(ast));
		C_LINENO(new->val) = crex_ast_get_lineno(ast);
		buf = (void*)((char*)buf + sizeof(crex_ast_zval));
	} else if (ast->kind == CREX_AST_CONSTANT) {
		crex_ast_zval *new = (crex_ast_zval*)buf;
		new->kind = CREX_AST_CONSTANT;
		new->attr = ast->attr;
		ZVAL_STR_COPY(&new->val, crex_ast_get_constant_name(ast));
		C_LINENO(new->val) = crex_ast_get_lineno(ast);
		buf = (void*)((char*)buf + sizeof(crex_ast_zval));
	} else if (crex_ast_is_list(ast)) {
		crex_ast_list *list = crex_ast_get_list(ast);
		crex_ast_list *new = (crex_ast_list*)buf;
		uint32_t i;
		new->kind = list->kind;
		new->attr = list->attr;
		new->children = list->children;
		new->lineno = list->lineno;
		buf = (void*)((char*)buf + crex_ast_list_size(list->children));
		for (i = 0; i < list->children; i++) {
			if (list->child[i]) {
				new->child[i] = (crex_ast*)buf;
				buf = crex_ast_tree_copy(list->child[i], buf);
			} else {
				new->child[i] = NULL;
			}
		}
	} else {
		uint32_t i, children = crex_ast_get_num_children(ast);
		crex_ast *new = (crex_ast*)buf;
		new->kind = ast->kind;
		new->attr = ast->attr;
		new->lineno = ast->lineno;
		buf = (void*)((char*)buf + crex_ast_size(children));
		for (i = 0; i < children; i++) {
			if (ast->child[i]) {
				new->child[i] = (crex_ast*)buf;
				buf = crex_ast_tree_copy(ast->child[i], buf);
			} else {
				new->child[i] = NULL;
			}
		}
	}
	return buf;
}

CREX_API crex_ast_ref * CREX_FASTCALL crex_ast_copy(crex_ast *ast)
{
	size_t tree_size;
	crex_ast_ref *ref;

	CREX_ASSERT(ast != NULL);
	tree_size = crex_ast_tree_size(ast) + sizeof(crex_ast_ref);
	ref = emalloc(tree_size);
	crex_ast_tree_copy(ast, GC_AST(ref));
	GC_SET_REFCOUNT(ref, 1);
	GC_TYPE_INFO(ref) = GC_CONSTANT_AST;
	return ref;
}

CREX_API void CREX_FASTCALL crex_ast_destroy(crex_ast *ast)
{
tail_call:
	if (!ast) {
		return;
	}

	if (EXPECTED(ast->kind >= CREX_AST_VAR)) {
		uint32_t i, children = crex_ast_get_num_children(ast);

		for (i = 1; i < children; i++) {
			crex_ast_destroy(ast->child[i]);
		}
		ast = ast->child[0];
		goto tail_call;
	} else if (EXPECTED(ast->kind == CREX_AST_ZVAL)) {
		zval_ptr_dtor_nogc(crex_ast_get_zval(ast));
	} else if (EXPECTED(crex_ast_is_list(ast))) {
		crex_ast_list *list = crex_ast_get_list(ast);
		if (list->children) {
			uint32_t i;

			for (i = 1; i < list->children; i++) {
				crex_ast_destroy(list->child[i]);
			}
			ast = list->child[0];
			goto tail_call;
		}
	} else if (EXPECTED(ast->kind == CREX_AST_CONSTANT)) {
		crex_string_release_ex(crex_ast_get_constant_name(ast), 0);
	} else if (EXPECTED(ast->kind >= CREX_AST_FUNC_DECL)) {
		crex_ast_decl *decl = (crex_ast_decl *) ast;

		if (decl->name) {
		    crex_string_release_ex(decl->name, 0);
		}
		if (decl->doc_comment) {
			crex_string_release_ex(decl->doc_comment, 0);
		}
		crex_ast_destroy(decl->child[0]);
		crex_ast_destroy(decl->child[1]);
		crex_ast_destroy(decl->child[2]);
		crex_ast_destroy(decl->child[3]);
		ast = decl->child[4];
		goto tail_call;
	}
}

CREX_API void CREX_FASTCALL crex_ast_ref_destroy(crex_ast_ref *ast)
{
	crex_ast_destroy(GC_AST(ast));
	efree(ast);
}

CREX_API void crex_ast_apply(crex_ast *ast, crex_ast_apply_func fn, void *context) {
	if (crex_ast_is_list(ast)) {
		crex_ast_list *list = crex_ast_get_list(ast);
		uint32_t i;
		for (i = 0; i < list->children; ++i) {
			fn(&list->child[i], context);
		}
	} else {
		uint32_t i, children = crex_ast_get_num_children(ast);
		for (i = 0; i < children; ++i) {
			fn(&ast->child[i], context);
		}
	}
}

/*
 * Operator Precedence
 * ====================
 * priority  associativity  operators
 * ----------------------------------
 *   10     left            include, include_once, eval, require, require_once
 *   20     left            ,
 *   30     left            or
 *   40     left            xor
 *   50     left            and
 *   60     right           print
 *   70     right           yield
 *   80     right           =>
 *   85     right           yield from
 *   90     right           = += -= *= /= .= %= &= |= ^= <<= >>= **=
 *  100     left            ? :
 *  110     right           ??
 *  120     left            ||
 *  130     left            &&
 *  140     left            |
 *  150     left            ^
 *  160     left            &
 *  170     non-associative == != === !==
 *  180     non-associative < <= > >= <=>
 *  185     left            .
 *  190     left            << >>
 *  200     left            + -
 *  210     left            * / %
 *  220     right           !
 *  230     non-associative instanceof
 *  240     right           + - ++ -- ~ (type) @
 *  250     right           **
 *  260     left            [
 *  270     non-associative clone new
 */

static CREX_COLD void crex_ast_export_ex(smart_str *str, crex_ast *ast, int priority, int indent);

static CREX_COLD void crex_ast_export_str(smart_str *str, crex_string *s)
{
	size_t i;

	for (i = 0; i < ZSTR_LEN(s); i++) {
		unsigned char c = ZSTR_VAL(s)[i];
		if (c == '\'' || c == '\\') {
			smart_str_appendc(str, '\\');
			smart_str_appendc(str, c);
		} else {
			smart_str_appendc(str, c);
		}
	}
}

static CREX_COLD void crex_ast_export_qstr(smart_str *str, char quote, crex_string *s)
{
	size_t i;

	for (i = 0; i < ZSTR_LEN(s); i++) {
		unsigned char c = ZSTR_VAL(s)[i];
		if (c < ' ') {
			switch (c) {
				case '\n':
					smart_str_appends(str, "\\n");
					break;
				case '\r':
					smart_str_appends(str, "\\r");
					break;
				case '\t':
					smart_str_appends(str, "\\t");
					break;
				case '\f':
					smart_str_appends(str, "\\f");
					break;
				case '\v':
					smart_str_appends(str, "\\v");
					break;
#ifdef CREX_WIN32
				case VK_ESCAPE:
#else
				case '\e':
#endif
					smart_str_appends(str, "\\e");
					break;
				default:
					smart_str_appends(str, "\\0");
					smart_str_appendc(str, '0' + (c / 8));
					smart_str_appendc(str, '0' + (c % 8));
					break;
			}
		} else {
			if (c == quote || c == '$' || c == '\\') {
				smart_str_appendc(str, '\\');
			}
			smart_str_appendc(str, c);
		}
	}
}

static CREX_COLD void crex_ast_export_indent(smart_str *str, int indent)
{
	while (indent > 0) {
		smart_str_appends(str, "    ");
		indent--;
	}
}

static CREX_COLD void crex_ast_export_name(smart_str *str, crex_ast *ast, int priority, int indent)
{
	if (ast->kind == CREX_AST_ZVAL) {
		zval *zv = crex_ast_get_zval(ast);

		if (C_TYPE_P(zv) == IS_STRING) {
			smart_str_append(str, C_STR_P(zv));
			return;
		}
	}
	crex_ast_export_ex(str, ast, priority, indent);
}

static CREX_COLD void crex_ast_export_ns_name(smart_str *str, crex_ast *ast, int priority, int indent)
{
	if (ast->kind == CREX_AST_ZVAL) {
		zval *zv = crex_ast_get_zval(ast);

		if (C_TYPE_P(zv) == IS_STRING) {
		    if (ast->attr == CREX_NAME_FQ) {
				smart_str_appendc(str, '\\');
		    } else if (ast->attr == CREX_NAME_RELATIVE) {
				smart_str_appends(str, "namespace\\");
		    }
			smart_str_append(str, C_STR_P(zv));
			return;
		}
	}
	crex_ast_export_ex(str, ast, priority, indent);
}

static CREX_COLD bool crex_ast_valid_var_char(char ch)
{
	unsigned char c = (unsigned char)ch;

	if (c != '_' && c < 127 &&
	    (c < '0' || c > '9') &&
	    (c < 'A' || c > 'Z') &&
	    (c < 'a' || c > 'z')) {
		return 0;
	}
	return 1;
}

static CREX_COLD bool crex_ast_valid_var_name(const char *s, size_t len)
{
	unsigned char c;
	size_t i;

	if (len == 0) {
		return 0;
	}
	c = (unsigned char)s[0];
	if (c != '_' && c < 127 &&
	    (c < 'A' || c > 'Z') &&
	    (c < 'a' || c > 'z')) {
		return 0;
	}
	for (i = 1; i < len; i++) {
		c = (unsigned char)s[i];
		if (c != '_' && c < 127 &&
		    (c < '0' || c > '9') &&
		    (c < 'A' || c > 'Z') &&
		    (c < 'a' || c > 'z')) {
			return 0;
		}
	}
	return 1;
}

static CREX_COLD bool crex_ast_var_needs_braces(char ch)
{
	return ch == '[' || crex_ast_valid_var_char(ch);
}

static CREX_COLD void crex_ast_export_var(smart_str *str, crex_ast *ast, int priority, int indent)
{
	if (ast->kind == CREX_AST_ZVAL) {
		zval *zv = crex_ast_get_zval(ast);
		if (C_TYPE_P(zv) == IS_STRING &&
		    crex_ast_valid_var_name(C_STRVAL_P(zv), C_STRLEN_P(zv))) {
			smart_str_append(str, C_STR_P(zv));
			return;
		}
	} else if (ast->kind == CREX_AST_VAR) {
		crex_ast_export_ex(str, ast, 0, indent);
		return;
	}
	smart_str_appendc(str, '{');
	crex_ast_export_name(str, ast, 0, indent);
	smart_str_appendc(str, '}');
}

static CREX_COLD void crex_ast_export_list(smart_str *str, crex_ast_list *list, bool separator, int priority, int indent)
{
	uint32_t i = 0;

	while (i < list->children) {
		if (i != 0 && separator) {
			smart_str_appends(str, ", ");
		}
		crex_ast_export_ex(str, list->child[i], priority, indent);
		i++;
	}
}

static CREX_COLD void crex_ast_export_encaps_list(smart_str *str, char quote, crex_ast_list *list, int indent)
{
	uint32_t i = 0;
	crex_ast *ast;

	while (i < list->children) {
		ast = list->child[i];
		if (ast->kind == CREX_AST_ZVAL) {
			zval *zv = crex_ast_get_zval(ast);

			CREX_ASSERT(C_TYPE_P(zv) == IS_STRING);
			crex_ast_export_qstr(str, quote, C_STR_P(zv));
		} else if (ast->kind == CREX_AST_VAR &&
		           ast->child[0]->kind == CREX_AST_ZVAL &&
		           (i + 1 == list->children ||
		            list->child[i + 1]->kind != CREX_AST_ZVAL ||
		            !crex_ast_var_needs_braces(
		                *C_STRVAL_P(
		                    crex_ast_get_zval(list->child[i + 1]))))) {
			crex_ast_export_ex(str, ast, 0, indent);
		} else {
			smart_str_appendc(str, '{');
			crex_ast_export_ex(str, ast, 0, indent);
			smart_str_appendc(str, '}');
		}
		i++;
	}
}

static CREX_COLD void crex_ast_export_name_list_ex(smart_str *str, crex_ast_list *list, int indent, const char *separator)
{
	uint32_t i = 0;

	while (i < list->children) {
		if (i != 0) {
			smart_str_appends(str, separator);
		}
		crex_ast_export_name(str, list->child[i], 0, indent);
		i++;
	}
}

#define crex_ast_export_name_list(s, l, i) crex_ast_export_name_list_ex(s, l, i, ", ")
#define crex_ast_export_catch_name_list(s, l, i) crex_ast_export_name_list_ex(s, l, i, "|")

static CREX_COLD void crex_ast_export_var_list(smart_str *str, crex_ast_list *list, int indent)
{
	uint32_t i = 0;

	while (i < list->children) {
		if (i != 0) {
			smart_str_appends(str, ", ");
		}
		if (list->child[i]->attr & CREX_BIND_REF) {
			smart_str_appendc(str, '&');
		}
		smart_str_appendc(str, '$');
		crex_ast_export_name(str, list->child[i], 20, indent);
		i++;
	}
}

static CREX_COLD void crex_ast_export_stmt(smart_str *str, crex_ast *ast, int indent)
{
	if (!ast) {
		return;
	}

	if (ast->kind == CREX_AST_STMT_LIST ||
	    ast->kind == CREX_AST_TRAIT_ADAPTATIONS) {
		crex_ast_list *list = (crex_ast_list*)ast;
		uint32_t i = 0;

		while (i < list->children) {
			ast = list->child[i];
			crex_ast_export_stmt(str, ast, indent);
			i++;
		}
	} else {
		crex_ast_export_indent(str, indent);
		crex_ast_export_ex(str, ast, 0, indent);
		switch (ast->kind) {
			case CREX_AST_LABEL:
			case CREX_AST_IF:
			case CREX_AST_SWITCH:
			case CREX_AST_WHILE:
			case CREX_AST_TRY:
			case CREX_AST_FOR:
			case CREX_AST_FOREACH:
			case CREX_AST_FUNC_DECL:
			case CREX_AST_METHOD:
			case CREX_AST_CLASS:
			case CREX_AST_USE_TRAIT:
			case CREX_AST_NAMESPACE:
			case CREX_AST_DECLARE:
				break;
			default:
				smart_str_appendc(str, ';');
				break;
		}
		smart_str_appendc(str, '\n');
	}
}

static CREX_COLD void crex_ast_export_if_stmt(smart_str *str, crex_ast_list *list, int indent)
{
	uint32_t i;
	crex_ast *ast;

tail_call:
	i = 0;
	while (i < list->children) {
		ast = list->child[i];
		CREX_ASSERT(ast->kind == CREX_AST_IF_ELEM);
		if (ast->child[0]) {
			if (i == 0) {
				smart_str_appends(str, "if (");
			} else {
				crex_ast_export_indent(str, indent);
				smart_str_appends(str, "} elseif (");
			}
			crex_ast_export_ex(str, ast->child[0], 0, indent);
			smart_str_appends(str, ") {\n");
			crex_ast_export_stmt(str, ast->child[1], indent + 1);
		} else {
			crex_ast_export_indent(str, indent);
			smart_str_appends(str, "} else ");
			if (ast->child[1] && ast->child[1]->kind == CREX_AST_IF) {
				list = (crex_ast_list*)ast->child[1];
				goto tail_call;
			} else {
				smart_str_appends(str, "{\n");
				crex_ast_export_stmt(str, ast->child[1], indent + 1);
			}
		}
		i++;
	}
	crex_ast_export_indent(str, indent);
	smart_str_appendc(str, '}');
}

static CREX_COLD void crex_ast_export_zval(smart_str *str, zval *zv, int priority, int indent)
{
	ZVAL_DEREF(zv);
	switch (C_TYPE_P(zv)) {
		case IS_NULL:
			smart_str_appends(str, "null");
			break;
		case IS_FALSE:
			smart_str_appends(str, "false");
			break;
		case IS_TRUE:
			smart_str_appends(str, "true");
			break;
		case IS_LONG:
			smart_str_append_long(str, C_LVAL_P(zv));
			break;
		case IS_DOUBLE:
			smart_str_append_double(
				str, C_DVAL_P(zv), (int) EG(precision), /* zero_fraction */ false);
			break;
		case IS_STRING:
			smart_str_appendc(str, '\'');
			crex_ast_export_str(str, C_STR_P(zv));
			smart_str_appendc(str, '\'');
			break;
		case IS_ARRAY: {
			crex_long idx;
			crex_string *key;
			zval *val;
			bool first = true;
			smart_str_appendc(str, '[');
			CREX_HASH_FOREACH_KEY_VAL(C_ARRVAL_P(zv), idx, key, val) {
				if (first) {
					first = false;
				} else {
					smart_str_appends(str, ", ");
				}
				if (key) {
					smart_str_appendc(str, '\'');
					crex_ast_export_str(str, key);
					smart_str_appends(str, "' => ");
				} else {
					smart_str_append_long(str, idx);
					smart_str_appends(str, " => ");
				}
				crex_ast_export_zval(str, val, 0, indent);
			} CREX_HASH_FOREACH_END();
			smart_str_appendc(str, ']');
			break;
		}
		case IS_CONSTANT_AST:
			crex_ast_export_ex(str, C_ASTVAL_P(zv), priority, indent);
			break;
		EMPTY_SWITCH_DEFAULT_CASE();
	}
}

static CREX_COLD void crex_ast_export_class_no_header(smart_str *str, crex_ast_decl *decl, int indent) {
	if (decl->child[0]) {
		smart_str_appends(str, " extends ");
		crex_ast_export_ns_name(str, decl->child[0], 0, indent);
	}
	if (decl->child[1]) {
		smart_str_appends(str, " implements ");
		crex_ast_export_ex(str, decl->child[1], 0, indent);
	}
	smart_str_appends(str, " {\n");
	crex_ast_export_stmt(str, decl->child[2], indent + 1);
	crex_ast_export_indent(str, indent);
	smart_str_appends(str, "}");
}

static CREX_COLD void crex_ast_export_attribute_group(smart_str *str, crex_ast *ast, int indent) {
	crex_ast_list *list = crex_ast_get_list(ast);
	for (uint32_t i = 0; i < list->children; i++) {
		crex_ast *attr = list->child[i];

		if (i) {
			smart_str_appends(str, ", ");
		}
		crex_ast_export_ns_name(str, attr->child[0], 0, indent);

		if (attr->child[1]) {
			smart_str_appendc(str, '(');
			crex_ast_export_ex(str, attr->child[1], 0, indent);
			smart_str_appendc(str, ')');
		}
	}
}

static CREX_COLD void crex_ast_export_attributes(smart_str *str, crex_ast *ast, int indent, bool newlines) {
	crex_ast_list *list = crex_ast_get_list(ast);
	uint32_t i;

	for (i = 0; i < list->children; i++) {
		smart_str_appends(str, "#[");
		crex_ast_export_attribute_group(str, list->child[i], indent);
		smart_str_appends(str, "]");

		if (newlines) {
			smart_str_appendc(str, '\n');
			crex_ast_export_indent(str, indent);
		} else {
			smart_str_appendc(str, ' ');
		}
	}
}

static CREX_COLD void crex_ast_export_visibility(smart_str *str, uint32_t flags) {
	if (flags & CREX_ACC_PUBLIC) {
		smart_str_appends(str, "public ");
	} else if (flags & CREX_ACC_PROTECTED) {
		smart_str_appends(str, "protected ");
	} else if (flags & CREX_ACC_PRIVATE) {
		smart_str_appends(str, "private ");
	}
}

static CREX_COLD void crex_ast_export_type(smart_str *str, crex_ast *ast, int indent) {
	if (ast->kind == CREX_AST_TYPE_UNION) {
		crex_ast_list *list = crex_ast_get_list(ast);
		for (uint32_t i = 0; i < list->children; i++) {
			if (i != 0) {
				smart_str_appendc(str, '|');
			}
			crex_ast_export_type(str, list->child[i], indent);
		}
		return;
	}
	if (ast->kind == CREX_AST_TYPE_INTERSECTION) {
		crex_ast_list *list = crex_ast_get_list(ast);
		for (uint32_t i = 0; i < list->children; i++) {
			if (i != 0) {
				smart_str_appendc(str, '&');
			}
			crex_ast_export_type(str, list->child[i], indent);
		}
		return;
	}
	if (ast->attr & CREX_TYPE_NULLABLE) {
		smart_str_appendc(str, '?');
	}
	crex_ast_export_ns_name(str, ast, 0, indent);
}

#define BINARY_OP(_op, _p, _pl, _pr) do { \
		op = _op; \
		p = _p; \
		pl = _pl; \
		pr = _pr; \
		goto binary_op; \
	} while (0)

#define PREFIX_OP(_op, _p, _pl) do { \
		op = _op; \
		p = _p; \
		pl = _pl; \
		goto prefix_op; \
	} while (0)

#define FUNC_OP(_op) do { \
		op = _op; \
		goto func_op; \
	} while (0)

#define POSTFIX_OP(_op, _p, _pl) do { \
		op = _op; \
		p = _p; \
		pl = _pl; \
		goto postfix_op; \
	} while (0)

#define APPEND_NODE_1(_op) do { \
		op = _op; \
		goto append_node_1; \
	} while (0)

#define APPEND_STR(_op) do { \
		op = _op; \
		goto append_str; \
	} while (0)

#define APPEND_DEFAULT_VALUE(n) do { \
		p = n; \
		goto append_default_value; \
	} while (0)

static CREX_COLD void crex_ast_export_ex(smart_str *str, crex_ast *ast, int priority, int indent)
{
	crex_ast_decl *decl;
	int p, pl, pr;
	const char *op;

tail_call:
	if (!ast) {
		return;
	}
	switch (ast->kind) {
		/* special nodes */
		case CREX_AST_ZVAL:
			crex_ast_export_zval(str, crex_ast_get_zval(ast), priority, indent);
			break;
		case CREX_AST_CONSTANT: {
			crex_string *name = crex_ast_get_constant_name(ast);
			smart_str_appendl(str, ZSTR_VAL(name), ZSTR_LEN(name));
			break;
		}
		case CREX_AST_CONSTANT_CLASS:
			smart_str_appendl(str, "__CLASS__", sizeof("__CLASS__")-1);
			break;
		case CREX_AST_ZNODE:
			/* This AST kind is only used for temporary nodes during compilation */
			CREX_UNREACHABLE();
			break;

		/* declaration nodes */
		case CREX_AST_FUNC_DECL:
		case CREX_AST_CLOSURE:
		case CREX_AST_ARROW_FUNC:
		case CREX_AST_METHOD:
			decl = (crex_ast_decl *) ast;
			if (decl->child[4]) {
				bool newlines = !(ast->kind == CREX_AST_CLOSURE || ast->kind == CREX_AST_ARROW_FUNC);
				crex_ast_export_attributes(str, decl->child[4], indent, newlines);
			}

			crex_ast_export_visibility(str, decl->flags);

			if (decl->flags & CREX_ACC_STATIC) {
				smart_str_appends(str, "static ");
			}
			if (decl->flags & CREX_ACC_ABSTRACT) {
				smart_str_appends(str, "abstract ");
			}
			if (decl->flags & CREX_ACC_FINAL) {
				smart_str_appends(str, "final ");
			}
			if (decl->kind == CREX_AST_ARROW_FUNC) {
				smart_str_appends(str, "fn");
			} else {
				smart_str_appends(str, "function ");
			}
			if (decl->flags & CREX_ACC_RETURN_REFERENCE) {
				smart_str_appendc(str, '&');
			}
			if (ast->kind != CREX_AST_CLOSURE && ast->kind != CREX_AST_ARROW_FUNC) {
				smart_str_appendl(str, ZSTR_VAL(decl->name), ZSTR_LEN(decl->name));
			}
			smart_str_appendc(str, '(');
			crex_ast_export_ex(str, decl->child[0], 0, indent);
			smart_str_appendc(str, ')');
			crex_ast_export_ex(str, decl->child[1], 0, indent);
			if (decl->child[3]) {
				smart_str_appends(str, ": ");
				crex_ast_export_type(str, decl->child[3], indent);
			}
			if (decl->child[2]) {
				if (decl->kind == CREX_AST_ARROW_FUNC) {
					crex_ast *body = decl->child[2];
					if (body->kind == CREX_AST_RETURN) {
						body = body->child[0];
					}
					smart_str_appends(str, " => ");
					crex_ast_export_ex(str, body, 0, indent);
					break;
				}

				smart_str_appends(str, " {\n");
				crex_ast_export_stmt(str, decl->child[2], indent + 1);
				crex_ast_export_indent(str, indent);
				smart_str_appendc(str, '}');
				if (ast->kind != CREX_AST_CLOSURE) {
					smart_str_appendc(str, '\n');
				}
			} else {
				smart_str_appends(str, ";\n");
			}
			break;
		case CREX_AST_CLASS:
			decl = (crex_ast_decl *) ast;
			if (decl->child[3]) {
				crex_ast_export_attributes(str, decl->child[3], indent, 1);
			}
			if (decl->flags & CREX_ACC_INTERFACE) {
				smart_str_appends(str, "interface ");
			} else if (decl->flags & CREX_ACC_TRAIT) {
				smart_str_appends(str, "trait ");
			} else if (decl->flags & CREX_ACC_ENUM) {
				smart_str_appends(str, "enum ");
			} else {
				if (decl->flags & CREX_ACC_EXPLICIT_ABSTRACT_CLASS) {
					smart_str_appends(str, "abstract ");
				}
				if (decl->flags & CREX_ACC_FINAL) {
					smart_str_appends(str, "final ");
				}
				if (decl->flags & CREX_ACC_READONLY_CLASS) {
					smart_str_appends(str, "readonly ");
				}
				smart_str_appends(str, "class ");
			}
			smart_str_appendl(str, ZSTR_VAL(decl->name), ZSTR_LEN(decl->name));
			if (decl->flags & CREX_ACC_ENUM && decl->child[4]) {
				smart_str_appends(str, ": ");
				crex_ast_export_type(str, decl->child[4], indent);
			}
			crex_ast_export_class_no_header(str, decl, indent);
			smart_str_appendc(str, '\n');
			break;

		/* list nodes */
		case CREX_AST_ARG_LIST:
		case CREX_AST_EXPR_LIST:
		case CREX_AST_PARAM_LIST:
simple_list:
			crex_ast_export_list(str, (crex_ast_list*)ast, 1, 20, indent);
			break;
		case CREX_AST_ARRAY:
			smart_str_appendc(str, '[');
			crex_ast_export_list(str, (crex_ast_list*)ast, 1, 20, indent);
			smart_str_appendc(str, ']');
			break;
		case CREX_AST_ENCAPS_LIST:
			smart_str_appendc(str, '"');
			crex_ast_export_encaps_list(str, '"', (crex_ast_list*)ast, indent);
			smart_str_appendc(str, '"');
			break;
		case CREX_AST_STMT_LIST:
		case CREX_AST_TRAIT_ADAPTATIONS:
			crex_ast_export_stmt(str, ast, indent);
			break;
		case CREX_AST_IF:
			crex_ast_export_if_stmt(str, (crex_ast_list*)ast, indent);
			break;
		case CREX_AST_SWITCH_LIST:
		case CREX_AST_CATCH_LIST:
		case CREX_AST_MATCH_ARM_LIST:
			crex_ast_export_list(str, (crex_ast_list*)ast, 0, 0, indent);
			break;
		case CREX_AST_CLOSURE_USES:
			smart_str_appends(str, " use(");
			crex_ast_export_var_list(str, (crex_ast_list*)ast, indent);
			smart_str_appendc(str, ')');
			break;
		case CREX_AST_PROP_GROUP: {
			crex_ast *type_ast = ast->child[0];
			crex_ast *prop_ast = ast->child[1];

			if (ast->child[2]) {
				crex_ast_export_attributes(str, ast->child[2], indent, 1);
			}

			crex_ast_export_visibility(str, ast->attr);

			if (ast->attr & CREX_ACC_STATIC) {
				smart_str_appends(str, "static ");
			}
			if (ast->attr & CREX_ACC_READONLY) {
				smart_str_appends(str, "readonly ");
			}

			if (type_ast) {
				crex_ast_export_type(str, type_ast, indent);
				smart_str_appendc(str, ' ');
			}

			ast = prop_ast;
			goto simple_list;
		}

		case CREX_AST_CONST_DECL:
			smart_str_appends(str, "const ");
			goto simple_list;
		case CREX_AST_CLASS_CONST_GROUP:
			if (ast->child[1]) {
				crex_ast_export_attributes(str, ast->child[1], indent, 1);
			}

			crex_ast_export_visibility(str, ast->attr);
			smart_str_appends(str, "const ");
			if (ast->child[2]) {
				crex_ast_export_type(str, ast->child[2], indent);
				smart_str_appendc(str, ' ');
			}

			ast = ast->child[0];

			goto simple_list;
		case CREX_AST_NAME_LIST:
			crex_ast_export_name_list(str, (crex_ast_list*)ast, indent);
			break;
		case CREX_AST_USE:
			smart_str_appends(str, "use ");
			if (ast->attr == T_FUNCTION) {
				smart_str_appends(str, "function ");
			} else if (ast->attr == T_CONST) {
				smart_str_appends(str, "const ");
			}
			goto simple_list;

		/* 0 child nodes */
		case CREX_AST_MAGIC_CONST:
			switch (ast->attr) {
				case T_LINE:     APPEND_STR("__LINE__");
				case T_FILE:     APPEND_STR("__FILE__");
				case T_DIR:      APPEND_STR("__DIR__");
				case T_TRAIT_C:  APPEND_STR("__TRAIT__");
				case T_METHOD_C: APPEND_STR("__METHOD__");
				case T_FUNC_C:   APPEND_STR("__FUNCTION__");
				case T_NS_C:     APPEND_STR("__NAMESPACE__");
				case T_CLASS_C:  APPEND_STR("__CLASS__");
				EMPTY_SWITCH_DEFAULT_CASE();
			}
			break;
		case CREX_AST_TYPE:
			switch (ast->attr & ~CREX_TYPE_NULLABLE) {
				case IS_ARRAY:    APPEND_STR("array");
				case IS_CALLABLE: APPEND_STR("callable");
				case IS_STATIC:   APPEND_STR("static");
				case IS_MIXED:    APPEND_STR("mixed");
				EMPTY_SWITCH_DEFAULT_CASE();
			}
			break;

		/* 1 child node */
		case CREX_AST_VAR:
			smart_str_appendc(str, '$');
			crex_ast_export_var(str, ast->child[0], 0, indent);
			break;
		case CREX_AST_CONST:
			crex_ast_export_ns_name(str, ast->child[0], 0, indent);
			break;
		case CREX_AST_UNPACK:
			smart_str_appends(str, "...");
			ast = ast->child[0];
			goto tail_call;
		case CREX_AST_UNARY_PLUS:  PREFIX_OP("+", 240, 241);
		case CREX_AST_UNARY_MINUS: PREFIX_OP("-", 240, 241);
		case CREX_AST_CAST:
			switch (ast->attr) {
				case IS_NULL:      PREFIX_OP("(unset)",  240, 241);
				case _IS_BOOL:     PREFIX_OP("(bool)",   240, 241);
				case IS_LONG:      PREFIX_OP("(int)",    240, 241);
				case IS_DOUBLE:    PREFIX_OP("(double)", 240, 241);
				case IS_STRING:    PREFIX_OP("(string)", 240, 241);
				case IS_ARRAY:     PREFIX_OP("(array)",  240, 241);
				case IS_OBJECT:    PREFIX_OP("(object)", 240, 241);
				EMPTY_SWITCH_DEFAULT_CASE();
			}
			break;
		case CREX_AST_EMPTY:
			FUNC_OP("empty");
		case CREX_AST_ISSET:
			FUNC_OP("isset");
		case CREX_AST_SILENCE:
			PREFIX_OP("@", 240, 241);
		case CREX_AST_SHELL_EXEC:
			smart_str_appendc(str, '`');
			if (ast->child[0]->kind == CREX_AST_ENCAPS_LIST) {
				crex_ast_export_encaps_list(str, '`', (crex_ast_list*)ast->child[0], indent);
			} else {
				zval *zv;
				CREX_ASSERT(ast->child[0]->kind == CREX_AST_ZVAL);
				zv = crex_ast_get_zval(ast->child[0]);
				CREX_ASSERT(C_TYPE_P(zv) == IS_STRING);
				crex_ast_export_qstr(str, '`', C_STR_P(zv));
			}
			smart_str_appendc(str, '`');
			break;
		case CREX_AST_CLONE:
			PREFIX_OP("clone ", 270, 271);
		case CREX_AST_EXIT:
			if (ast->child[0]) {
				FUNC_OP("exit");
			} else {
				APPEND_STR("exit");
			}
			break;
		case CREX_AST_PRINT:
			PREFIX_OP("print ", 60, 61);
		case CREX_AST_INCLUDE_OR_EVAL:
			switch (ast->attr) {
				case CREX_INCLUDE_ONCE: FUNC_OP("include_once");
				case CREX_INCLUDE:      FUNC_OP("include");
				case CREX_REQUIRE_ONCE: FUNC_OP("require_once");
				case CREX_REQUIRE:      FUNC_OP("require");
				case CREX_EVAL:         FUNC_OP("eval");
				EMPTY_SWITCH_DEFAULT_CASE();
			}
			break;
		case CREX_AST_UNARY_OP:
			switch (ast->attr) {
				case CREX_BW_NOT:   PREFIX_OP("~", 240, 241);
				case CREX_BOOL_NOT: PREFIX_OP("!", 240, 241);
				EMPTY_SWITCH_DEFAULT_CASE();
			}
			break;
		case CREX_AST_PRE_INC:
			PREFIX_OP("++", 240, 241);
		case CREX_AST_PRE_DEC:
			PREFIX_OP("--", 240, 241);
		case CREX_AST_POST_INC:
			POSTFIX_OP("++", 240, 241);
		case CREX_AST_POST_DEC:
			POSTFIX_OP("--", 240, 241);

		case CREX_AST_GLOBAL:
			APPEND_NODE_1("global");
		case CREX_AST_UNSET:
			FUNC_OP("unset");
		case CREX_AST_RETURN:
			APPEND_NODE_1("return");
		case CREX_AST_LABEL:
			crex_ast_export_name(str, ast->child[0], 0, indent);
			smart_str_appendc(str, ':');
			break;
		case CREX_AST_REF:
			smart_str_appendc(str, '&');
			ast = ast->child[0];
			goto tail_call;
		case CREX_AST_HALT_COMPILER:
			APPEND_STR("__HALT_COMPILER()");
		case CREX_AST_ECHO:
			APPEND_NODE_1("echo");
		case CREX_AST_THROW:
			APPEND_NODE_1("throw");
		case CREX_AST_GOTO:
			smart_str_appends(str, "goto ");
			crex_ast_export_name(str, ast->child[0], 0, indent);
			break;
		case CREX_AST_BREAK:
			APPEND_NODE_1("break");
		case CREX_AST_CONTINUE:
			APPEND_NODE_1("continue");

		/* 2 child nodes */
		case CREX_AST_DIM:
			crex_ast_export_ex(str, ast->child[0], 260, indent);
			smart_str_appendc(str, '[');
			if (ast->child[1]) {
				crex_ast_export_ex(str, ast->child[1], 0, indent);
			}
			smart_str_appendc(str, ']');
			break;
		case CREX_AST_PROP:
		case CREX_AST_NULLSAFE_PROP:
			crex_ast_export_ex(str, ast->child[0], 0, indent);
			smart_str_appends(str, ast->kind == CREX_AST_NULLSAFE_PROP ? "?->" : "->");
			crex_ast_export_var(str, ast->child[1], 0, indent);
			break;
		case CREX_AST_STATIC_PROP:
			crex_ast_export_ns_name(str, ast->child[0], 0, indent);
			smart_str_appends(str, "::$");
			crex_ast_export_var(str, ast->child[1], 0, indent);
			break;
		case CREX_AST_CALL:
			crex_ast_export_ns_name(str, ast->child[0], 0, indent);
			smart_str_appendc(str, '(');
			crex_ast_export_ex(str, ast->child[1], 0, indent);
			smart_str_appendc(str, ')');
			break;
		case CREX_AST_CALLABLE_CONVERT:
			smart_str_appends(str, "...");
			break;
		case CREX_AST_CLASS_CONST:
			crex_ast_export_ns_name(str, ast->child[0], 0, indent);
			smart_str_appends(str, "::");
			crex_ast_export_name(str, ast->child[1], 0, indent);
			break;
		case CREX_AST_CLASS_NAME:
			if (ast->child[0] == NULL) {
				/* The const expr representation stores the fetch type instead. */
				switch (ast->attr) {
					case CREX_FETCH_CLASS_SELF:
						smart_str_appends(str, "self");
						break;
					case CREX_FETCH_CLASS_PARENT:
						smart_str_appends(str, "parent");
						break;
					EMPTY_SWITCH_DEFAULT_CASE()
				}
			} else {
				crex_ast_export_ns_name(str, ast->child[0], 0, indent);
			}
			smart_str_appends(str, "::class");
			break;
		case CREX_AST_ASSIGN:            BINARY_OP(" = ",   90, 91, 90);
		case CREX_AST_ASSIGN_REF:        BINARY_OP(" =& ",  90, 91, 90);
		case CREX_AST_ASSIGN_OP:
			switch (ast->attr) {
				case CREX_ADD:    BINARY_OP(" += ",  90, 91, 90);
				case CREX_SUB:    BINARY_OP(" -= ",  90, 91, 90);
				case CREX_MUL:    BINARY_OP(" *= ",  90, 91, 90);
				case CREX_DIV:    BINARY_OP(" /= ",  90, 91, 90);
				case CREX_MOD:    BINARY_OP(" %= ",  90, 91, 90);
				case CREX_SL:     BINARY_OP(" <<= ", 90, 91, 90);
				case CREX_SR:     BINARY_OP(" >>= ", 90, 91, 90);
				case CREX_CONCAT: BINARY_OP(" .= ",  90, 91, 90);
				case CREX_BW_OR:  BINARY_OP(" |= ",  90, 91, 90);
				case CREX_BW_AND: BINARY_OP(" &= ",  90, 91, 90);
				case CREX_BW_XOR: BINARY_OP(" ^= ",  90, 91, 90);
				case CREX_POW:    BINARY_OP(" **= ", 90, 91, 90);
				EMPTY_SWITCH_DEFAULT_CASE();
			}
			break;
		case CREX_AST_ASSIGN_COALESCE: BINARY_OP(" \?\?= ", 90, 91, 90);
		case CREX_AST_BINARY_OP:
			switch (ast->attr) {
				case CREX_ADD:                 BINARY_OP(" + ",   200, 200, 201);
				case CREX_SUB:                 BINARY_OP(" - ",   200, 200, 201);
				case CREX_MUL:                 BINARY_OP(" * ",   210, 210, 211);
				case CREX_DIV:                 BINARY_OP(" / ",   210, 210, 211);
				case CREX_MOD:                 BINARY_OP(" % ",   210, 210, 211);
				case CREX_SL:                  BINARY_OP(" << ",  190, 190, 191);
				case CREX_SR:                  BINARY_OP(" >> ",  190, 190, 191);
				case CREX_CONCAT:              BINARY_OP(" . ",   185, 185, 186);
				case CREX_BW_OR:               BINARY_OP(" | ",   140, 140, 141);
				case CREX_BW_AND:              BINARY_OP(" & ",   160, 160, 161);
				case CREX_BW_XOR:              BINARY_OP(" ^ ",   150, 150, 151);
				case CREX_IS_IDENTICAL:        BINARY_OP(" === ", 170, 171, 171);
				case CREX_IS_NOT_IDENTICAL:    BINARY_OP(" !== ", 170, 171, 171);
				case CREX_IS_EQUAL:            BINARY_OP(" == ",  170, 171, 171);
				case CREX_IS_NOT_EQUAL:        BINARY_OP(" != ",  170, 171, 171);
				case CREX_IS_SMALLER:          BINARY_OP(" < ",   180, 181, 181);
				case CREX_IS_SMALLER_OR_EQUAL: BINARY_OP(" <= ",  180, 181, 181);
				case CREX_POW:                 BINARY_OP(" ** ",  250, 251, 250);
				case CREX_BOOL_XOR:            BINARY_OP(" xor ",  40,  40,  41);
				case CREX_SPACESHIP:           BINARY_OP(" <=> ", 180, 181, 181);
				EMPTY_SWITCH_DEFAULT_CASE();
			}
			break;
		case CREX_AST_GREATER:                 BINARY_OP(" > ",   180, 181, 181);
		case CREX_AST_GREATER_EQUAL:           BINARY_OP(" >= ",  180, 181, 181);
		case CREX_AST_AND:                     BINARY_OP(" && ",  130, 130, 131);
		case CREX_AST_OR:                      BINARY_OP(" || ",  120, 120, 121);
		case CREX_AST_ARRAY_ELEM:
			if (ast->child[1]) {
				crex_ast_export_ex(str, ast->child[1], 80, indent);
				smart_str_appends(str, " => ");
			}
			if (ast->attr)
				smart_str_appendc(str, '&');
			crex_ast_export_ex(str, ast->child[0], 80, indent);
			break;
		case CREX_AST_NEW:
			smart_str_appends(str, "new ");
			if (ast->child[0]->kind == CREX_AST_CLASS) {
				crex_ast_decl *decl = (crex_ast_decl *) ast->child[0];
				if (decl->child[3]) {
					crex_ast_export_attributes(str, decl->child[3], indent, 0);
				}
				smart_str_appends(str, "class");
				if (!crex_ast_is_list(ast->child[1])
						|| crex_ast_get_list(ast->child[1])->children) {
					smart_str_appendc(str, '(');
					crex_ast_export_ex(str, ast->child[1], 0, indent);
					smart_str_appendc(str, ')');
				}
				crex_ast_export_class_no_header(str, decl, indent);
			} else {
				crex_ast_export_ns_name(str, ast->child[0], 0, indent);
				smart_str_appendc(str, '(');
				crex_ast_export_ex(str, ast->child[1], 0, indent);
				smart_str_appendc(str, ')');
			}
			break;
		case CREX_AST_INSTANCEOF:
			crex_ast_export_ex(str, ast->child[0], 0, indent);
			smart_str_appends(str, " instanceof ");
			crex_ast_export_ns_name(str, ast->child[1], 0, indent);
			break;
		case CREX_AST_YIELD:
			if (priority > 70) smart_str_appendc(str, '(');
			smart_str_appends(str, "yield ");
			if (ast->child[0]) {
				if (ast->child[1]) {
					crex_ast_export_ex(str, ast->child[1], 70, indent);
					smart_str_appends(str, " => ");
				}
				crex_ast_export_ex(str, ast->child[0], 70, indent);
			}
			if (priority > 70) smart_str_appendc(str, ')');
			break;
		case CREX_AST_YIELD_FROM:
			PREFIX_OP("yield from ", 85, 86);
		case CREX_AST_COALESCE: BINARY_OP(" ?? ", 110, 111, 110);
		case CREX_AST_STATIC:
			smart_str_appends(str, "static $");
			crex_ast_export_name(str, ast->child[0], 0, indent);
			APPEND_DEFAULT_VALUE(1);
		case CREX_AST_WHILE:
			smart_str_appends(str, "while (");
			crex_ast_export_ex(str, ast->child[0], 0, indent);
			smart_str_appends(str, ") {\n");
			crex_ast_export_stmt(str, ast->child[1], indent + 1);
			crex_ast_export_indent(str, indent);
			smart_str_appendc(str, '}');
			break;
		case CREX_AST_DO_WHILE:
			smart_str_appends(str, "do {\n");
			crex_ast_export_stmt(str, ast->child[0], indent + 1);
			crex_ast_export_indent(str, indent);
			smart_str_appends(str, "} while (");
			crex_ast_export_ex(str, ast->child[1], 0, indent);
			smart_str_appendc(str, ')');
			break;

		case CREX_AST_IF_ELEM:
			if (ast->child[0]) {
				smart_str_appends(str, "if (");
				crex_ast_export_ex(str, ast->child[0], 0, indent);
				smart_str_appends(str, ") {\n");
				crex_ast_export_stmt(str, ast->child[1], indent + 1);
			} else {
				smart_str_appends(str, "else {\n");
				crex_ast_export_stmt(str, ast->child[1], indent + 1);
			}
			crex_ast_export_indent(str, indent);
			smart_str_appendc(str, '}');
			break;
		case CREX_AST_SWITCH:
			smart_str_appends(str, "switch (");
			crex_ast_export_ex(str, ast->child[0], 0, indent);
			smart_str_appends(str, ") {\n");
			crex_ast_export_ex(str, ast->child[1], 0, indent + 1);
			crex_ast_export_indent(str, indent);
			smart_str_appendc(str, '}');
			break;
		case CREX_AST_SWITCH_CASE:
			crex_ast_export_indent(str, indent);
			if (ast->child[0]) {
				smart_str_appends(str, "case ");
				crex_ast_export_ex(str, ast->child[0], 0, indent);
				smart_str_appends(str, ":\n");
			} else {
				smart_str_appends(str, "default:\n");
			}
			crex_ast_export_stmt(str, ast->child[1], indent + 1);
			break;
		case CREX_AST_MATCH:
			smart_str_appends(str, "match (");
			crex_ast_export_ex(str, ast->child[0], 0, indent);
			smart_str_appends(str, ") {\n");
			crex_ast_export_ex(str, ast->child[1], 0, indent + 1);
			crex_ast_export_indent(str, indent);
			smart_str_appendc(str, '}');
			break;
		case CREX_AST_MATCH_ARM:
			crex_ast_export_indent(str, indent);
			if (ast->child[0]) {
				crex_ast_export_list(str, (crex_ast_list*)ast->child[0], 1, 0, indent);
				smart_str_appends(str, " => ");
			} else {
				smart_str_appends(str, "default => ");
			}
			crex_ast_export_ex(str, ast->child[1], 0, 0);
			smart_str_appends(str, ",\n");
			break;
		case CREX_AST_DECLARE:
			smart_str_appends(str, "declare(");
			CREX_ASSERT(ast->child[0]->kind == CREX_AST_CONST_DECL);
			crex_ast_export_list(str, (crex_ast_list*)ast->child[0], 1, 0, indent);
			smart_str_appendc(str, ')');
			if (ast->child[1]) {
				smart_str_appends(str, " {\n");
				crex_ast_export_stmt(str, ast->child[1], indent + 1);
				crex_ast_export_indent(str, indent);
				smart_str_appendc(str, '}');
			} else {
				smart_str_appendc(str, ';');
			}
			break;
		case CREX_AST_PROP_ELEM:
			smart_str_appendc(str, '$');
			CREX_FALLTHROUGH;
		case CREX_AST_CONST_ELEM:
			crex_ast_export_name(str, ast->child[0], 0, indent);
			APPEND_DEFAULT_VALUE(1);
		case CREX_AST_USE_TRAIT:
			smart_str_appends(str, "use ");
			crex_ast_export_ex(str, ast->child[0], 0, indent);
			if (ast->child[1]) {
				smart_str_appends(str, " {\n");
				crex_ast_export_ex(str, ast->child[1], 0, indent + 1);
				crex_ast_export_indent(str, indent);
				smart_str_appends(str, "}");
			} else {
				smart_str_appends(str, ";");
			}
			break;
		case CREX_AST_TRAIT_PRECEDENCE:
			crex_ast_export_ex(str, ast->child[0], 0, indent);
			smart_str_appends(str, " insteadof ");
			crex_ast_export_ex(str, ast->child[1], 0, indent);
			break;
		case CREX_AST_METHOD_REFERENCE:
			if (ast->child[0]) {
				crex_ast_export_name(str, ast->child[0], 0, indent);
				smart_str_appends(str, "::");
			}
			crex_ast_export_name(str, ast->child[1], 0, indent);
			break;
		case CREX_AST_NAMESPACE:
			smart_str_appends(str, "namespace");
			if (ast->child[0]) {
				smart_str_appendc(str, ' ');
				crex_ast_export_name(str, ast->child[0], 0, indent);
			}
			if (ast->child[1]) {
				smart_str_appends(str, " {\n");
				crex_ast_export_stmt(str, ast->child[1], indent + 1);
				crex_ast_export_indent(str, indent);
				smart_str_appends(str, "}\n");
			} else {
				smart_str_appendc(str, ';');
			}
			break;
		case CREX_AST_USE_ELEM:
		case CREX_AST_TRAIT_ALIAS:
			crex_ast_export_name(str, ast->child[0], 0, indent);
			if (ast->attr & CREX_ACC_PUBLIC) {
				smart_str_appends(str, " as public");
			} else if (ast->attr & CREX_ACC_PROTECTED) {
				smart_str_appends(str, " as protected");
			} else if (ast->attr & CREX_ACC_PRIVATE) {
				smart_str_appends(str, " as private");
			} else if (ast->child[1]) {
				smart_str_appends(str, " as");
			}
			if (ast->child[1]) {
				smart_str_appendc(str, ' ');
				crex_ast_export_name(str, ast->child[1], 0, indent);
			}
			break;
		case CREX_AST_NAMED_ARG:
			smart_str_append(str, crex_ast_get_str(ast->child[0]));
			smart_str_appends(str, ": ");
			ast = ast->child[1];
			goto tail_call;

		/* 3 child nodes */
		case CREX_AST_METHOD_CALL:
		case CREX_AST_NULLSAFE_METHOD_CALL:
			crex_ast_export_ex(str, ast->child[0], 0, indent);
			smart_str_appends(str, ast->kind == CREX_AST_NULLSAFE_METHOD_CALL ? "?->" : "->");
			crex_ast_export_var(str, ast->child[1], 0, indent);
			smart_str_appendc(str, '(');
			crex_ast_export_ex(str, ast->child[2], 0, indent);
			smart_str_appendc(str, ')');
			break;
		case CREX_AST_STATIC_CALL:
			crex_ast_export_ns_name(str, ast->child[0], 0, indent);
			smart_str_appends(str, "::");
			crex_ast_export_var(str, ast->child[1], 0, indent);
			smart_str_appendc(str, '(');
			crex_ast_export_ex(str, ast->child[2], 0, indent);
			smart_str_appendc(str, ')');
			break;
		case CREX_AST_CONDITIONAL:
			if (priority > 100) smart_str_appendc(str, '(');
			crex_ast_export_ex(str, ast->child[0], 100, indent);
			if (ast->child[1]) {
				smart_str_appends(str, " ? ");
				crex_ast_export_ex(str, ast->child[1], 101, indent);
				smart_str_appends(str, " : ");
			} else {
				smart_str_appends(str, " ?: ");
			}
			crex_ast_export_ex(str, ast->child[2], 101, indent);
			if (priority > 100) smart_str_appendc(str, ')');
			break;

		case CREX_AST_TRY:
			smart_str_appends(str, "try {\n");
			crex_ast_export_stmt(str, ast->child[0], indent + 1);
			crex_ast_export_indent(str, indent);
			crex_ast_export_ex(str, ast->child[1], 0, indent);
			if (ast->child[2]) {
				smart_str_appends(str, "} finally {\n");
				crex_ast_export_stmt(str, ast->child[2], indent + 1);
				crex_ast_export_indent(str, indent);
			}
			smart_str_appendc(str, '}');
			break;
		case CREX_AST_CATCH:
			smart_str_appends(str, "} catch (");
			crex_ast_export_catch_name_list(str, crex_ast_get_list(ast->child[0]), indent);
			if (ast->child[1]) {
				smart_str_appends(str, " $");
				crex_ast_export_var(str, ast->child[1], 0, indent);
			}
			smart_str_appends(str, ") {\n");
			crex_ast_export_stmt(str, ast->child[2], indent + 1);
			crex_ast_export_indent(str, indent);
			break;
		case CREX_AST_PARAM:
			if (ast->child[3]) {
				crex_ast_export_attributes(str, ast->child[3], indent, 0);
			}
			if (ast->child[0]) {
				crex_ast_export_type(str, ast->child[0], indent);
				smart_str_appendc(str, ' ');
			}
			if (ast->attr & CREX_PARAM_REF) {
				smart_str_appendc(str, '&');
			}
			if (ast->attr & CREX_PARAM_VARIADIC) {
				smart_str_appends(str, "...");
			}
			smart_str_appendc(str, '$');
			crex_ast_export_name(str, ast->child[1], 0, indent);
			APPEND_DEFAULT_VALUE(2);
		case CREX_AST_ENUM_CASE:
			if (ast->child[3]) {
				crex_ast_export_attributes(str, ast->child[3], indent, 1);
			}
			smart_str_appends(str, "case ");
			crex_ast_export_name(str, ast->child[0], 0, indent);
			if (ast->child[1]) {
				smart_str_appends(str, " = ");
				crex_ast_export_ex(str, ast->child[1], 0, indent);
			}
			break;

		/* 4 child nodes */
		case CREX_AST_FOR:
			smart_str_appends(str, "for (");
			crex_ast_export_ex(str, ast->child[0], 0, indent);
			smart_str_appendc(str, ';');
			if (ast->child[1]) {
				smart_str_appendc(str, ' ');
				crex_ast_export_ex(str, ast->child[1], 0, indent);
			}
			smart_str_appendc(str, ';');
			if (ast->child[2]) {
				smart_str_appendc(str, ' ');
				crex_ast_export_ex(str, ast->child[2], 0, indent);
			}
			smart_str_appends(str, ") {\n");
			crex_ast_export_stmt(str, ast->child[3], indent + 1);
			crex_ast_export_indent(str, indent);
			smart_str_appendc(str, '}');
			break;
		case CREX_AST_FOREACH:
			smart_str_appends(str, "foreach (");
			crex_ast_export_ex(str, ast->child[0], 0, indent);
			smart_str_appends(str, " as ");
			if (ast->child[2]) {
				crex_ast_export_ex(str, ast->child[2], 0, indent);
				smart_str_appends(str, " => ");
			}
			crex_ast_export_ex(str, ast->child[1], 0, indent);
			smart_str_appends(str, ") {\n");
			crex_ast_export_stmt(str, ast->child[3], indent + 1);
			crex_ast_export_indent(str, indent);
			smart_str_appendc(str, '}');
			break;
		EMPTY_SWITCH_DEFAULT_CASE();
	}
	return;

binary_op:
	if (priority > p) smart_str_appendc(str, '(');
	crex_ast_export_ex(str, ast->child[0], pl, indent);
	smart_str_appends(str, op);
	crex_ast_export_ex(str, ast->child[1], pr, indent);
	if (priority > p) smart_str_appendc(str, ')');
	return;

prefix_op:
	if (priority > p) smart_str_appendc(str, '(');
	smart_str_appends(str, op);
	crex_ast_export_ex(str, ast->child[0], pl, indent);
	if (priority > p) smart_str_appendc(str, ')');
	return;

postfix_op:
	if (priority > p) smart_str_appendc(str, '(');
	crex_ast_export_ex(str, ast->child[0], pl, indent);
	smart_str_appends(str, op);
	if (priority > p) smart_str_appendc(str, ')');
	return;

func_op:
	smart_str_appends(str, op);
	smart_str_appendc(str, '(');
	crex_ast_export_ex(str, ast->child[0], 0, indent);
	smart_str_appendc(str, ')');
	return;

append_node_1:
	smart_str_appends(str, op);
	if (ast->child[0]) {
		smart_str_appendc(str, ' ');
		ast = ast->child[0];
		goto tail_call;
	}
	return;

append_str:
	smart_str_appends(str, op);
	return;

append_default_value:
	if (ast->child[p]) {
		smart_str_appends(str, " = ");
		ast = ast->child[p];
		goto tail_call;
	}
	return;
}

CREX_API CREX_COLD crex_string *crex_ast_export(const char *prefix, crex_ast *ast, const char *suffix)
{
	smart_str str = {0};

	smart_str_appends(&str, prefix);
	crex_ast_export_ex(&str, ast, 0, 0);
	smart_str_appends(&str, suffix);
	smart_str_0(&str);
	return str.s;
}

crex_ast * CREX_FASTCALL crex_ast_with_attributes(crex_ast *ast, crex_ast *attr)
{
	CREX_ASSERT(attr->kind == CREX_AST_ATTRIBUTE_LIST);

	switch (ast->kind) {
	case CREX_AST_FUNC_DECL:
	case CREX_AST_CLOSURE:
	case CREX_AST_METHOD:
	case CREX_AST_ARROW_FUNC:
		((crex_ast_decl *) ast)->child[4] = attr;
		break;
	case CREX_AST_CLASS:
		((crex_ast_decl *) ast)->child[3] = attr;
		break;
	case CREX_AST_PROP_GROUP:
		ast->child[2] = attr;
		break;
	case CREX_AST_PARAM:
	case CREX_AST_ENUM_CASE:
		ast->child[3] = attr;
		break;
	case CREX_AST_CLASS_CONST_GROUP:
		ast->child[1] = attr;
		break;
	EMPTY_SWITCH_DEFAULT_CASE()
	}

	return ast;
}
