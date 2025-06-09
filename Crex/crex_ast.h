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
   |          Nikita Popov <nikic@crx.net>                                |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_AST_H
#define CREX_AST_H

#include "crex.h"

#ifndef CREX_AST_SPEC
# define CREX_AST_SPEC 1
#endif

#define CREX_AST_SPECIAL_SHIFT      6
#define CREX_AST_IS_LIST_SHIFT      7
#define CREX_AST_NUM_CHILDREN_SHIFT 8

enum _crex_ast_kind {
	/* special nodes */
	CREX_AST_ZVAL = 1 << CREX_AST_SPECIAL_SHIFT,
	CREX_AST_CONSTANT,
	CREX_AST_ZNODE,

	/* declaration nodes */
	CREX_AST_FUNC_DECL,
	CREX_AST_CLOSURE,
	CREX_AST_METHOD,
	CREX_AST_CLASS,
	CREX_AST_ARROW_FUNC,

	/* list nodes */
	CREX_AST_ARG_LIST = 1 << CREX_AST_IS_LIST_SHIFT,
	CREX_AST_ARRAY,
	CREX_AST_ENCAPS_LIST,
	CREX_AST_EXPR_LIST,
	CREX_AST_STMT_LIST,
	CREX_AST_IF,
	CREX_AST_SWITCH_LIST,
	CREX_AST_CATCH_LIST,
	CREX_AST_PARAM_LIST,
	CREX_AST_CLOSURE_USES,
	CREX_AST_PROP_DECL,
	CREX_AST_CONST_DECL,
	CREX_AST_CLASS_CONST_DECL,
	CREX_AST_NAME_LIST,
	CREX_AST_TRAIT_ADAPTATIONS,
	CREX_AST_USE,
	CREX_AST_TYPE_UNION,
	CREX_AST_TYPE_INTERSECTION,
	CREX_AST_ATTRIBUTE_LIST,
	CREX_AST_ATTRIBUTE_GROUP,
	CREX_AST_MATCH_ARM_LIST,
	CREX_AST_MODIFIER_LIST,

	/* 0 child nodes */
	CREX_AST_MAGIC_CONST = 0 << CREX_AST_NUM_CHILDREN_SHIFT,
	CREX_AST_TYPE,
	CREX_AST_CONSTANT_CLASS,
	CREX_AST_CALLABLE_CONVERT,

	/* 1 child node */
	CREX_AST_VAR = 1 << CREX_AST_NUM_CHILDREN_SHIFT,
	CREX_AST_CONST,
	CREX_AST_UNPACK,
	CREX_AST_UNARY_PLUS,
	CREX_AST_UNARY_MINUS,
	CREX_AST_CAST,
	CREX_AST_EMPTY,
	CREX_AST_ISSET,
	CREX_AST_SILENCE,
	CREX_AST_SHELL_EXEC,
	CREX_AST_CLONE,
	CREX_AST_EXIT,
	CREX_AST_PRINT,
	CREX_AST_INCLUDE_OR_EVAL,
	CREX_AST_UNARY_OP,
	CREX_AST_PRE_INC,
	CREX_AST_PRE_DEC,
	CREX_AST_POST_INC,
	CREX_AST_POST_DEC,
	CREX_AST_YIELD_FROM,
	CREX_AST_CLASS_NAME,

	CREX_AST_GLOBAL,
	CREX_AST_UNSET,
	CREX_AST_RETURN,
	CREX_AST_LABEL,
	CREX_AST_REF,
	CREX_AST_HALT_COMPILER,
	CREX_AST_ECHO,
	CREX_AST_THROW,
	CREX_AST_GOTO,
	CREX_AST_BREAK,
	CREX_AST_CONTINUE,

	/* 2 child nodes */
	CREX_AST_DIM = 2 << CREX_AST_NUM_CHILDREN_SHIFT,
	CREX_AST_PROP,
	CREX_AST_NULLSAFE_PROP,
	CREX_AST_STATIC_PROP,
	CREX_AST_CALL,
	CREX_AST_CLASS_CONST,
	CREX_AST_ASSIGN,
	CREX_AST_ASSIGN_REF,
	CREX_AST_ASSIGN_OP,
	CREX_AST_BINARY_OP,
	CREX_AST_GREATER,
	CREX_AST_GREATER_EQUAL,
	CREX_AST_AND,
	CREX_AST_OR,
	CREX_AST_ARRAY_ELEM,
	CREX_AST_NEW,
	CREX_AST_INSTANCEOF,
	CREX_AST_YIELD,
	CREX_AST_COALESCE,
	CREX_AST_ASSIGN_COALESCE,

	CREX_AST_STATIC,
	CREX_AST_WHILE,
	CREX_AST_DO_WHILE,
	CREX_AST_IF_ELEM,
	CREX_AST_SWITCH,
	CREX_AST_SWITCH_CASE,
	CREX_AST_DECLARE,
	CREX_AST_USE_TRAIT,
	CREX_AST_TRAIT_PRECEDENCE,
	CREX_AST_METHOD_REFERENCE,
	CREX_AST_NAMESPACE,
	CREX_AST_USE_ELEM,
	CREX_AST_TRAIT_ALIAS,
	CREX_AST_GROUP_USE,
	CREX_AST_ATTRIBUTE,
	CREX_AST_MATCH,
	CREX_AST_MATCH_ARM,
	CREX_AST_NAMED_ARG,

	/* 3 child nodes */
	CREX_AST_METHOD_CALL = 3 << CREX_AST_NUM_CHILDREN_SHIFT,
	CREX_AST_NULLSAFE_METHOD_CALL,
	CREX_AST_STATIC_CALL,
	CREX_AST_CONDITIONAL,

	CREX_AST_TRY,
	CREX_AST_CATCH,
	CREX_AST_PROP_GROUP,
	CREX_AST_PROP_ELEM,
	CREX_AST_CONST_ELEM,
	CREX_AST_CLASS_CONST_GROUP,

	// Pseudo node for initializing enums
	CREX_AST_CONST_ENUM_INIT,

	/* 4 child nodes */
	CREX_AST_FOR = 4 << CREX_AST_NUM_CHILDREN_SHIFT,
	CREX_AST_FOREACH,
	CREX_AST_ENUM_CASE,

	/* 5 child nodes */
	CREX_AST_PARAM = 5 << CREX_AST_NUM_CHILDREN_SHIFT,
};

typedef uint16_t crex_ast_kind;
typedef uint16_t crex_ast_attr;

struct _crex_ast {
	crex_ast_kind kind; /* Type of the node (CREX_AST_* enum constant) */
	crex_ast_attr attr; /* Additional attribute, use depending on node type */
	uint32_t lineno;    /* Line number */
	crex_ast *child[1]; /* Array of children (using struct hack) */
};

/* Same as crex_ast, but with children count, which is updated dynamically */
typedef struct _crex_ast_list {
	crex_ast_kind kind;
	crex_ast_attr attr;
	uint32_t lineno;
	uint32_t children;
	crex_ast *child[1];
} crex_ast_list;

/* Lineno is stored in val.u2.lineno */
typedef struct _crex_ast_zval {
	crex_ast_kind kind;
	crex_ast_attr attr;
	zval val;
} crex_ast_zval;

/* Separate structure for function and class declaration, as they need extra information. */
typedef struct _crex_ast_decl {
	crex_ast_kind kind;
	crex_ast_attr attr; /* Unused - for structure compatibility */
	uint32_t start_lineno;
	uint32_t end_lineno;
	uint32_t flags;
	crex_string *doc_comment;
	crex_string *name;
	crex_ast *child[5];
} crex_ast_decl;

typedef void (*crex_ast_process_t)(crex_ast *ast);
extern CREX_API crex_ast_process_t crex_ast_process;

CREX_API crex_ast * CREX_FASTCALL crex_ast_create_zval_with_lineno(zval *zv, uint32_t lineno);
CREX_API crex_ast * CREX_FASTCALL crex_ast_create_zval_ex(zval *zv, crex_ast_attr attr);
CREX_API crex_ast * CREX_FASTCALL crex_ast_create_zval(zval *zv);
CREX_API crex_ast * CREX_FASTCALL crex_ast_create_zval_from_str(crex_string *str);
CREX_API crex_ast * CREX_FASTCALL crex_ast_create_zval_from_long(crex_long lval);

CREX_API crex_ast * CREX_FASTCALL crex_ast_create_constant(crex_string *name, crex_ast_attr attr);
CREX_API crex_ast * CREX_FASTCALL crex_ast_create_class_const_or_name(crex_ast *class_name, crex_ast *name);

#if CREX_AST_SPEC
# define CREX_AST_SPEC_CALL(name, ...) \
	CREX_EXPAND_VA(CREX_AST_SPEC_CALL_(name, __VA_ARGS__, _5, _4, _3, _2, _1, _0)(__VA_ARGS__))
# define CREX_AST_SPEC_CALL_(name, _, _5, _4, _3, _2, _1, suffix, ...) \
	name ## suffix
# define CREX_AST_SPEC_CALL_EX(name, ...) \
	CREX_EXPAND_VA(CREX_AST_SPEC_CALL_EX_(name, __VA_ARGS__, _5, _4, _3, _2, _1, _0)(__VA_ARGS__))
# define CREX_AST_SPEC_CALL_EX_(name, _, _6, _5, _4, _3, _2, _1, suffix, ...) \
	name ## suffix

CREX_API crex_ast * CREX_FASTCALL crex_ast_create_0(crex_ast_kind kind);
CREX_API crex_ast * CREX_FASTCALL crex_ast_create_1(crex_ast_kind kind, crex_ast *child);
CREX_API crex_ast * CREX_FASTCALL crex_ast_create_2(crex_ast_kind kind, crex_ast *child1, crex_ast *child2);
CREX_API crex_ast * CREX_FASTCALL crex_ast_create_3(crex_ast_kind kind, crex_ast *child1, crex_ast *child2, crex_ast *child3);
CREX_API crex_ast * CREX_FASTCALL crex_ast_create_4(crex_ast_kind kind, crex_ast *child1, crex_ast *child2, crex_ast *child3, crex_ast *child4);
CREX_API crex_ast * CREX_FASTCALL crex_ast_create_5(crex_ast_kind kind, crex_ast *child1, crex_ast *child2, crex_ast *child3, crex_ast *child4, crex_ast *child5);

static crex_always_inline crex_ast * crex_ast_create_ex_0(crex_ast_kind kind, crex_ast_attr attr) {
	crex_ast *ast = crex_ast_create_0(kind);
	ast->attr = attr;
	return ast;
}
static crex_always_inline crex_ast * crex_ast_create_ex_1(crex_ast_kind kind, crex_ast_attr attr, crex_ast *child) {
	crex_ast *ast = crex_ast_create_1(kind, child);
	ast->attr = attr;
	return ast;
}
static crex_always_inline crex_ast * crex_ast_create_ex_2(crex_ast_kind kind, crex_ast_attr attr, crex_ast *child1, crex_ast *child2) {
	crex_ast *ast = crex_ast_create_2(kind, child1, child2);
	ast->attr = attr;
	return ast;
}
static crex_always_inline crex_ast * crex_ast_create_ex_3(crex_ast_kind kind, crex_ast_attr attr, crex_ast *child1, crex_ast *child2, crex_ast *child3) {
	crex_ast *ast = crex_ast_create_3(kind, child1, child2, child3);
	ast->attr = attr;
	return ast;
}
static crex_always_inline crex_ast * crex_ast_create_ex_4(crex_ast_kind kind, crex_ast_attr attr, crex_ast *child1, crex_ast *child2, crex_ast *child3, crex_ast *child4) {
	crex_ast *ast = crex_ast_create_4(kind, child1, child2, child3, child4);
	ast->attr = attr;
	return ast;
}
static crex_always_inline crex_ast * crex_ast_create_ex_5(crex_ast_kind kind, crex_ast_attr attr, crex_ast *child1, crex_ast *child2, crex_ast *child3, crex_ast *child4, crex_ast *child5) {
	crex_ast *ast = crex_ast_create_5(kind, child1, child2, child3, child4, child5);
	ast->attr = attr;
	return ast;
}

CREX_API crex_ast * CREX_FASTCALL crex_ast_create_list_0(crex_ast_kind kind);
CREX_API crex_ast * CREX_FASTCALL crex_ast_create_list_1(crex_ast_kind kind, crex_ast *child);
CREX_API crex_ast * CREX_FASTCALL crex_ast_create_list_2(crex_ast_kind kind, crex_ast *child1, crex_ast *child2);

# define crex_ast_create(...) \
	CREX_AST_SPEC_CALL(crex_ast_create, __VA_ARGS__)
# define crex_ast_create_ex(...) \
	CREX_AST_SPEC_CALL_EX(crex_ast_create_ex, __VA_ARGS__)
# define crex_ast_create_list(init_children, ...) \
	CREX_AST_SPEC_CALL(crex_ast_create_list, __VA_ARGS__)

#else
CREX_API crex_ast *crex_ast_create(crex_ast_kind kind, ...);
CREX_API crex_ast *crex_ast_create_ex(crex_ast_kind kind, crex_ast_attr attr, ...);
CREX_API crex_ast *crex_ast_create_list(uint32_t init_children, crex_ast_kind kind, ...);
#endif

CREX_API crex_ast * CREX_FASTCALL crex_ast_list_add(crex_ast *list, crex_ast *op);

CREX_API crex_ast *crex_ast_create_decl(
	crex_ast_kind kind, uint32_t flags, uint32_t start_lineno, crex_string *doc_comment,
	crex_string *name, crex_ast *child0, crex_ast *child1, crex_ast *child2, crex_ast *child3, crex_ast *child4
);

typedef struct {
	bool had_side_effects;
} crex_ast_evaluate_ctx;

CREX_API crex_result CREX_FASTCALL crex_ast_evaluate(zval *result, crex_ast *ast, crex_class_entry *scope);
CREX_API crex_result CREX_FASTCALL crex_ast_evaluate_ex(zval *result, crex_ast *ast, crex_class_entry *scope, bool *short_circuited_ptr, crex_ast_evaluate_ctx *ctx);
CREX_API crex_string *crex_ast_export(const char *prefix, crex_ast *ast, const char *suffix);

CREX_API crex_ast_ref * CREX_FASTCALL crex_ast_copy(crex_ast *ast);
CREX_API void CREX_FASTCALL crex_ast_destroy(crex_ast *ast);
CREX_API void CREX_FASTCALL crex_ast_ref_destroy(crex_ast_ref *ast);

typedef void (*crex_ast_apply_func)(crex_ast **ast_ptr, void *context);
CREX_API void crex_ast_apply(crex_ast *ast, crex_ast_apply_func fn, void *context);

static crex_always_inline size_t crex_ast_size(uint32_t children) {
	return XtOffsetOf(crex_ast, child) + (sizeof(crex_ast *) * children);
}

static crex_always_inline bool crex_ast_is_special(crex_ast *ast) {
	return (ast->kind >> CREX_AST_SPECIAL_SHIFT) & 1;
}

static crex_always_inline bool crex_ast_is_list(crex_ast *ast) {
	return (ast->kind >> CREX_AST_IS_LIST_SHIFT) & 1;
}
static crex_always_inline crex_ast_list *crex_ast_get_list(crex_ast *ast) {
	CREX_ASSERT(crex_ast_is_list(ast));
	return (crex_ast_list *) ast;
}

static crex_always_inline zval *crex_ast_get_zval(crex_ast *ast) {
	CREX_ASSERT(ast->kind == CREX_AST_ZVAL);
	return &((crex_ast_zval *) ast)->val;
}
static crex_always_inline crex_string *crex_ast_get_str(crex_ast *ast) {
	zval *zv = crex_ast_get_zval(ast);
	CREX_ASSERT(C_TYPE_P(zv) == IS_STRING);
	return C_STR_P(zv);
}

static crex_always_inline crex_string *crex_ast_get_constant_name(crex_ast *ast) {
	CREX_ASSERT(ast->kind == CREX_AST_CONSTANT);
	CREX_ASSERT(C_TYPE(((crex_ast_zval *) ast)->val) == IS_STRING);
	return C_STR(((crex_ast_zval *) ast)->val);
}

static crex_always_inline uint32_t crex_ast_get_num_children(crex_ast *ast) {
	CREX_ASSERT(!crex_ast_is_list(ast));
	return ast->kind >> CREX_AST_NUM_CHILDREN_SHIFT;
}
static crex_always_inline uint32_t crex_ast_get_lineno(crex_ast *ast) {
	if (ast->kind == CREX_AST_ZVAL) {
		zval *zv = crex_ast_get_zval(ast);
		return C_LINENO_P(zv);
	} else if (ast->kind == CREX_AST_CONSTANT) {
		zval *zv = &((crex_ast_zval *) ast)->val;
		return C_LINENO_P(zv);
	} else {
		return ast->lineno;
	}
}

static crex_always_inline crex_ast *crex_ast_create_binary_op(uint32_t opcode, crex_ast *op0, crex_ast *op1) {
	return crex_ast_create_ex(CREX_AST_BINARY_OP, opcode, op0, op1);
}

crex_ast *crex_ast_create_concat_op(crex_ast *op0, crex_ast *op1);

static crex_always_inline crex_ast *crex_ast_create_assign_op(uint32_t opcode, crex_ast *op0, crex_ast *op1) {
	return crex_ast_create_ex(CREX_AST_ASSIGN_OP, opcode, op0, op1);
}
static crex_always_inline crex_ast *crex_ast_create_cast(uint32_t type, crex_ast *op0) {
	return crex_ast_create_ex(CREX_AST_CAST, type, op0);
}
static crex_always_inline crex_ast *crex_ast_list_rtrim(crex_ast *ast) {
	crex_ast_list *list = crex_ast_get_list(ast);
	if (list->children && list->child[list->children - 1] == NULL) {
		list->children--;
	}
	return ast;
}

crex_ast * CREX_FASTCALL crex_ast_with_attributes(crex_ast *ast, crex_ast *attr);

#endif
