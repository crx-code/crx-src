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

#ifndef CREX_COMPILE_H
#define CREX_COMPILE_H

#include "crex.h"
#include "crex_ast.h"

#include <stdarg.h>
#include <stdint.h>

#include "crex_llist.h"

#define SET_UNUSED(op) do { \
	op ## _type = IS_UNUSED; \
	op.num = (uint32_t) -1; \
} while (0)

#define MAKE_NOP(opline) do { \
	(opline)->opcode = CREX_NOP; \
	SET_UNUSED((opline)->op1); \
	SET_UNUSED((opline)->op2); \
	SET_UNUSED((opline)->result); \
} while (0)

#define RESET_DOC_COMMENT() do { \
	if (CG(doc_comment)) { \
		crex_string_release_ex(CG(doc_comment), 0); \
		CG(doc_comment) = NULL; \
	} \
} while (0)

typedef struct _crex_op_array crex_op_array;
typedef struct _crex_op crex_op;

/* On 64-bit systems less optimal, but more compact VM code leads to better
 * performance. So on 32-bit systems we use absolute addresses for jump
 * targets and constants, but on 64-bit systems relative 32-bit offsets */
#if SIZEOF_SIZE_T == 4
# define CREX_USE_ABS_JMP_ADDR      1
# define CREX_USE_ABS_CONST_ADDR    1
#else
# define CREX_USE_ABS_JMP_ADDR      0
# define CREX_USE_ABS_CONST_ADDR    0
#endif

typedef union _znode_op {
	uint32_t      constant;
	uint32_t      var;
	uint32_t      num;
	uint32_t      opline_num; /*  Needs to be signed */
#if CREX_USE_ABS_JMP_ADDR
	crex_op       *jmp_addr;
#else
	uint32_t      jmp_offset;
#endif
#if CREX_USE_ABS_CONST_ADDR
	zval          *zv;
#endif
} znode_op;

typedef struct _znode { /* used only during compilation */
	uint8_t op_type;
	uint8_t flag;
	union {
		znode_op op;
		zval constant; /* replaced by literal/zv */
	} u;
} znode;

/* Temporarily defined here, to avoid header ordering issues */
typedef struct _crex_ast_znode {
	crex_ast_kind kind;
	crex_ast_attr attr;
	uint32_t lineno;
	znode node;
} crex_ast_znode;

CREX_API crex_ast * CREX_FASTCALL crex_ast_create_znode(znode *node);

static crex_always_inline znode *crex_ast_get_znode(crex_ast *ast) {
	return &((crex_ast_znode *) ast)->node;
}

typedef struct _crex_declarables {
	crex_long ticks;
} crex_declarables;

/* Compilation context that is different for each file, but shared between op arrays. */
typedef struct _crex_file_context {
	crex_declarables declarables;

	crex_string *current_namespace;
	bool in_namespace;
	bool has_bracketed_namespaces;

	HashTable *imports;
	HashTable *imports_function;
	HashTable *imports_const;

	HashTable seen_symbols;
} crex_file_context;

typedef union _crex_parser_stack_elem {
	crex_ast *ast;
	crex_string *str;
	crex_ulong num;
	unsigned char *ptr;
	unsigned char *ident;
} crex_parser_stack_elem;

void crex_compile_top_stmt(crex_ast *ast);
void crex_const_expr_to_zval(zval *result, crex_ast **ast_ptr, bool allow_dynamic);

typedef int (*user_opcode_handler_t) (crex_execute_data *execute_data);

struct _crex_op {
	const void *handler;
	znode_op op1;
	znode_op op2;
	znode_op result;
	uint32_t extended_value;
	uint32_t lineno;
	uint8_t opcode;       /* Opcodes defined in Crex/crex_vm_opcodes.h */
	uint8_t op1_type;     /* IS_UNUSED, IS_CONST, IS_TMP_VAR, IS_VAR, IS_CV */
	uint8_t op2_type;     /* IS_UNUSED, IS_CONST, IS_TMP_VAR, IS_VAR, IS_CV */
	uint8_t result_type;  /* IS_UNUSED, IS_CONST, IS_TMP_VAR, IS_VAR, IS_CV */
};


typedef struct _crex_brk_cont_element {
	int start;
	int cont;
	int brk;
	int parent;
	bool is_switch;
} crex_brk_cont_element;

typedef struct _crex_label {
	int brk_cont;
	uint32_t opline_num;
} crex_label;

typedef struct _crex_try_catch_element {
	uint32_t try_op;
	uint32_t catch_op;  /* ketchup! */
	uint32_t finally_op;
	uint32_t finally_end;
} crex_try_catch_element;

#define CREX_LIVE_TMPVAR  0
#define CREX_LIVE_LOOP    1
#define CREX_LIVE_SILENCE 2
#define CREX_LIVE_ROPE    3
#define CREX_LIVE_NEW     4
#define CREX_LIVE_MASK    7

typedef struct _crex_live_range {
	uint32_t var; /* low bits are used for variable type (CREX_LIVE_* macros) */
	uint32_t start;
	uint32_t end;
} crex_live_range;

/* Compilation context that is different for each op array. */
typedef struct _crex_oparray_context {
	uint32_t   opcodes_size;
	int        vars_size;
	int        literals_size;
	uint32_t   fast_call_var;
	uint32_t   try_catch_offset;
	int        current_brk_cont;
	int        last_brk_cont;
	crex_brk_cont_element *brk_cont_array;
	HashTable *labels;
} crex_oparray_context;

/* Class, property and method flags                  class|meth.|prop.|const*/
/*                                                        |     |     |     */
/* Common flags                                           |     |     |     */
/* ============                                           |     |     |     */
/*                                                        |     |     |     */
/* Visibility flags (public < protected < private)        |     |     |     */
#define CREX_ACC_PUBLIC                  (1 <<  0) /*     |  X  |  X  |  X  */
#define CREX_ACC_PROTECTED               (1 <<  1) /*     |  X  |  X  |  X  */
#define CREX_ACC_PRIVATE                 (1 <<  2) /*     |  X  |  X  |  X  */
/*                                                        |     |     |     */
/* Property or method overrides private one               |     |     |     */
#define CREX_ACC_CHANGED                 (1 <<  3) /*     |  X  |  X  |     */
/*                                                        |     |     |     */
/* Static method or property                              |     |     |     */
#define CREX_ACC_STATIC                  (1 <<  4) /*     |  X  |  X  |     */
/*                                                        |     |     |     */
/* Promoted property / parameter                          |     |     |     */
#define CREX_ACC_PROMOTED                (1 <<  5) /*     |     |  X  |  X  */
/*                                                        |     |     |     */
/* Final class or method                                  |     |     |     */
#define CREX_ACC_FINAL                   (1 <<  5) /*  X  |  X  |     |     */
/*                                                        |     |     |     */
/* Abstract method                                        |     |     |     */
#define CREX_ACC_ABSTRACT                (1 <<  6) /*  X  |  X  |     |     */
#define CREX_ACC_EXPLICIT_ABSTRACT_CLASS (1 <<  6) /*  X  |     |     |     */
/*                                                        |     |     |     */
/* Readonly property                                      |     |     |     */
#define CREX_ACC_READONLY                (1 <<  7) /*     |     |  X  |     */
/*                                                        |     |     |     */
/* Immutable op_array and class_entries                   |     |     |     */
/* (implemented only for lazy loading of op_arrays)       |     |     |     */
#define CREX_ACC_IMMUTABLE               (1 <<  7) /*  X  |  X  |     |     */
/*                                                        |     |     |     */
/* Function has typed arguments / class has typed props   |     |     |     */
#define CREX_ACC_HAS_TYPE_HINTS          (1 <<  8) /*  X  |  X  |     |     */
/*                                                        |     |     |     */
/* Top-level class or function declaration                |     |     |     */
#define CREX_ACC_TOP_LEVEL               (1 <<  9) /*  X  |  X  |     |     */
/*                                                        |     |     |     */
/* op_array or class is preloaded                         |     |     |     */
#define CREX_ACC_PRELOADED               (1 << 10) /*  X  |  X  |     |     */
/*                                                        |     |     |     */
/* Flag to differentiate cases from constants.            |     |     |     */
/* Must not conflict with CREX_ACC_ visibility flags      |     |     |     */
/* or IS_CONSTANT_VISITED_MARK                            |     |     |     */
#define CREX_CLASS_CONST_IS_CASE         (1 << 6)  /*     |     |     |  X  */
/*                                                        |     |     |     */
/* Class Flags (unused: 30,31)                            |     |     |     */
/* ===========                                            |     |     |     */
/*                                                        |     |     |     */
/* Special class types                                    |     |     |     */
#define CREX_ACC_INTERFACE               (1 <<  0) /*  X  |     |     |     */
#define CREX_ACC_TRAIT                   (1 <<  1) /*  X  |     |     |     */
#define CREX_ACC_ANON_CLASS              (1 <<  2) /*  X  |     |     |     */
#define CREX_ACC_ENUM                    (1 << 28) /*  X  |     |     |     */
/*                                                        |     |     |     */
/* Class linked with parent, interfaces and traits        |     |     |     */
#define CREX_ACC_LINKED                  (1 <<  3) /*  X  |     |     |     */
/*                                                        |     |     |     */
/* Class is abstract, since it is set by any              |     |     |     */
/* abstract method                                        |     |     |     */
#define CREX_ACC_IMPLICIT_ABSTRACT_CLASS (1 <<  4) /*  X  |     |     |     */
/*                                                        |     |     |     */
/* Class has magic methods __get/__set/__unset/           |     |     |     */
/* __isset that use guards                                |     |     |     */
#define CREX_ACC_USE_GUARDS              (1 << 11) /*  X  |     |     |     */
/*                                                        |     |     |     */
/* Class constants updated                                |     |     |     */
#define CREX_ACC_CONSTANTS_UPDATED       (1 << 12) /*  X  |     |     |     */
/*                                                        |     |     |     */
/* Objects of this class may not have dynamic properties  |     |     |     */
#define CREX_ACC_NO_DYNAMIC_PROPERTIES   (1 << 13) /*  X  |     |     |     */
/*                                                        |     |     |     */
/* User class has methods with static variables           |     |     |     */
#define CREX_HAS_STATIC_IN_METHODS       (1 << 14) /*  X  |     |     |     */
/*                                                        |     |     |     */
/* Objects of this class may have dynamic properties      |     |     |     */
/* without triggering a deprecation warning               |     |     |     */
#define CREX_ACC_ALLOW_DYNAMIC_PROPERTIES (1 << 15) /* X  |     |     |     */
/*                                                        |     |     |     */
/* Readonly class                                         |     |     |     */
#define CREX_ACC_READONLY_CLASS          (1 << 16) /*  X  |     |     |     */
/*                                                        |     |     |     */
/* Parent class is resolved (CE).                         |     |     |     */
#define CREX_ACC_RESOLVED_PARENT         (1 << 17) /*  X  |     |     |     */
/*                                                        |     |     |     */
/* Interfaces are resolved (CEs).                         |     |     |     */
#define CREX_ACC_RESOLVED_INTERFACES     (1 << 18) /*  X  |     |     |     */
/*                                                        |     |     |     */
/* Class has unresolved variance obligations.             |     |     |     */
#define CREX_ACC_UNRESOLVED_VARIANCE     (1 << 19) /*  X  |     |     |     */
/*                                                        |     |     |     */
/* Class is linked apart from variance obligations.       |     |     |     */
#define CREX_ACC_NEARLY_LINKED           (1 << 20) /*  X  |     |     |     */
/* Class has readonly props                               |     |     |     */
#define CREX_ACC_HAS_READONLY_PROPS      (1 << 21) /*  X  |     |     |     */
/*                                                        |     |     |     */
/* stored in opcache (may be partially)                   |     |     |     */
#define CREX_ACC_CACHED                  (1 << 22) /*  X  |     |     |     */
/*                                                        |     |     |     */
/* temporary flag used during delayed variance checks     |     |     |     */
#define CREX_ACC_CACHEABLE               (1 << 23) /*  X  |     |     |     */
/*                                                        |     |     |     */
#define CREX_ACC_HAS_AST_CONSTANTS       (1 << 24) /*  X  |     |     |     */
#define CREX_ACC_HAS_AST_PROPERTIES      (1 << 25) /*  X  |     |     |     */
#define CREX_ACC_HAS_AST_STATICS         (1 << 26) /*  X  |     |     |     */
/*                                                        |     |     |     */
/* loaded from file cache to process memory               |     |     |     */
#define CREX_ACC_FILE_CACHED             (1 << 27) /*  X  |     |     |     */
/*                                                        |     |     |     */
/* Class cannot be serialized or unserialized             |     |     |     */
#define CREX_ACC_NOT_SERIALIZABLE        (1 << 29) /*  X  |     |     |     */
/*                                                        |     |     |     */
/* Function Flags (unused: 29-30)                         |     |     |     */
/* ==============                                         |     |     |     */
/*                                                        |     |     |     */
/* deprecation flag                                       |     |     |     */
#define CREX_ACC_DEPRECATED              (1 << 11) /*     |  X  |     |     */
/*                                                        |     |     |     */
/* Function returning by reference                        |     |     |     */
#define CREX_ACC_RETURN_REFERENCE        (1 << 12) /*     |  X  |     |     */
/*                                                        |     |     |     */
/* Function has a return type                             |     |     |     */
#define CREX_ACC_HAS_RETURN_TYPE         (1 << 13) /*     |  X  |     |     */
/*                                                        |     |     |     */
/* Function with variable number of arguments             |     |     |     */
#define CREX_ACC_VARIADIC                (1 << 14) /*     |  X  |     |     */
/*                                                        |     |     |     */
/* op_array has finally blocks (user only)                |     |     |     */
#define CREX_ACC_HAS_FINALLY_BLOCK       (1 << 15) /*     |  X  |     |     */
/*                                                        |     |     |     */
/* "main" op_array with                                   |     |     |     */
/* CREX_DECLARE_CLASS_DELAYED opcodes                     |     |     |     */
#define CREX_ACC_EARLY_BINDING           (1 << 16) /*     |  X  |     |     */
/*                                                        |     |     |     */
/* closure uses $this                                     |     |     |     */
#define CREX_ACC_USES_THIS               (1 << 17) /*     |  X  |     |     */
/*                                                        |     |     |     */
/* call through user function trampoline. e.g.            |     |     |     */
/* __call, __callstatic                                   |     |     |     */
#define CREX_ACC_CALL_VIA_TRAMPOLINE     (1 << 18) /*     |  X  |     |     */
/*                                                        |     |     |     */
/* disable inline caching                                 |     |     |     */
#define CREX_ACC_NEVER_CACHE             (1 << 19) /*     |  X  |     |     */
/*                                                        |     |     |     */
/* op_array is a clone of trait method                    |     |     |     */
#define CREX_ACC_TRAIT_CLONE             (1 << 20) /*     |  X  |     |     */
/*                                                        |     |     |     */
/* functions is a constructor                             |     |     |     */
#define CREX_ACC_CTOR                    (1 << 21) /*     |  X  |     |     */
/*                                                        |     |     |     */
/* Closure related                                        |     |     |     */
#define CREX_ACC_CLOSURE                 (1 << 22) /*     |  X  |     |     */
#define CREX_ACC_FAKE_CLOSURE            (1 << 23) /*     |  X  |     |     */ /* Same as CREX_CALL_FAKE_CLOSURE */
/*                                                        |     |     |     */
#define CREX_ACC_GENERATOR               (1 << 24) /*     |  X  |     |     */
/*                                                        |     |     |     */
/* function was processed by pass two (user only)         |     |     |     */
#define CREX_ACC_DONE_PASS_TWO           (1 << 25) /*     |  X  |     |     */
/*                                                        |     |     |     */
/* internal function is allocated at arena (int only)     |     |     |     */
#define CREX_ACC_ARENA_ALLOCATED         (1 << 25) /*     |  X  |     |     */
/*                                                        |     |     |     */
/* run_time_cache allocated on heap (user only)           |     |     |     */
#define CREX_ACC_HEAP_RT_CACHE           (1 << 26) /*     |  X  |     |     */
/*                                                        |     |     |     */
/* method flag used by Closure::__invoke() (int only)     |     |     |     */
#define CREX_ACC_USER_ARG_INFO           (1 << 26) /*     |  X  |     |     */
/*                                                        |     |     |     */
/* supports opcache compile-time evaluation (funcs)       |     |     |     */
#define CREX_ACC_COMPILE_TIME_EVAL       (1 << 27) /*     |  X  |     |     */
/*                                                        |     |     |     */
/* has #[\Override] attribute                             |     |     |     */
#define CREX_ACC_OVERRIDE                (1 << 28) /*     |  X  |     |     */
/*                                                        |     |     |     */
/* op_array uses strict mode types                        |     |     |     */
#define CREX_ACC_STRICT_TYPES            (1U << 31) /*    |  X  |     |     */


#define CREX_ACC_PPP_MASK  (CREX_ACC_PUBLIC | CREX_ACC_PROTECTED | CREX_ACC_PRIVATE)

/* call through internal function handler. e.g. Closure::invoke() */
#define CREX_ACC_CALL_VIA_HANDLER     CREX_ACC_CALL_VIA_TRAMPOLINE

#define CREX_SHORT_CIRCUITING_CHAIN_MASK 0x3
#define CREX_SHORT_CIRCUITING_CHAIN_EXPR 0
#define CREX_SHORT_CIRCUITING_CHAIN_ISSET 1
#define CREX_SHORT_CIRCUITING_CHAIN_EMPTY 2

// Must not clash with CREX_SHORT_CIRCUITING_CHAIN_MASK
#define CREX_JMP_NULL_BP_VAR_IS 4

char *crex_visibility_string(uint32_t fn_flags);

typedef struct _crex_property_info {
	uint32_t offset; /* property offset for object properties or
	                      property index for static properties */
	uint32_t flags;
	crex_string *name;
	crex_string *doc_comment;
	HashTable *attributes;
	crex_class_entry *ce;
	crex_type type;
} crex_property_info;

#define OBJ_PROP(obj, offset) \
	((zval*)((char*)(obj) + offset))
#define OBJ_PROP_NUM(obj, num) \
	(&(obj)->properties_table[(num)])
#define OBJ_PROP_TO_OFFSET(num) \
	((uint32_t)(XtOffsetOf(crex_object, properties_table) + sizeof(zval) * (num)))
#define OBJ_PROP_TO_NUM(offset) \
	((offset - OBJ_PROP_TO_OFFSET(0)) / sizeof(zval))

typedef struct _crex_class_constant {
	zval value; /* flags are stored in u2 */
	crex_string *doc_comment;
	HashTable *attributes;
	crex_class_entry *ce;
	crex_type type;
} crex_class_constant;

#define CREX_CLASS_CONST_FLAGS(c) C_CONSTANT_FLAGS((c)->value)

/* arg_info for internal functions */
typedef struct _crex_internal_arg_info {
	const char *name;
	crex_type type;
	const char *default_value;
} crex_internal_arg_info;

/* arg_info for user functions */
typedef struct _crex_arg_info {
	crex_string *name;
	crex_type type;
	crex_string *default_value;
} crex_arg_info;

/* the following structure repeats the layout of crex_internal_arg_info,
 * but its fields have different meaning. It's used as the first element of
 * arg_info array to define properties of internal functions.
 * It's also used for the return type.
 */
typedef struct _crex_internal_function_info {
	uintptr_t required_num_args;
	crex_type type;
	const char *default_value;
} crex_internal_function_info;

struct _crex_op_array {
	/* Common elements */
	uint8_t type;
	uint8_t arg_flags[3]; /* bitset of arg_info.pass_by_reference */
	uint32_t fn_flags;
	crex_string *function_name;
	crex_class_entry *scope;
	crex_function *prototype;
	uint32_t num_args;
	uint32_t required_num_args;
	crex_arg_info *arg_info;
	HashTable *attributes;
	CREX_MAP_PTR_DEF(void **, run_time_cache);
	uint32_t T;         /* number of temporary variables */
	/* END of common elements */

	int cache_size;     /* number of run_time_cache_slots * sizeof(void*) */
	int last_var;       /* number of CV variables */
	uint32_t last;      /* number of opcodes */

	crex_op *opcodes;
	CREX_MAP_PTR_DEF(HashTable *, static_variables_ptr);
	HashTable *static_variables;
	crex_string **vars; /* names of CV variables */

	uint32_t *refcount;

	int last_live_range;
	int last_try_catch;
	crex_live_range *live_range;
	crex_try_catch_element *try_catch_array;

	crex_string *filename;
	uint32_t line_start;
	uint32_t line_end;
	crex_string *doc_comment;

	int last_literal;
	uint32_t num_dynamic_func_defs;
	zval *literals;

	/* Functions that are declared dynamically are stored here and
	 * referenced by index from opcodes. */
	crex_op_array **dynamic_func_defs;

	void *reserved[CREX_MAX_RESERVED_RESOURCES];
};


#define CREX_RETURN_VALUE				0
#define CREX_RETURN_REFERENCE			1

/* crex_internal_function_handler */
typedef void (CREX_FASTCALL *zif_handler)(INTERNAL_FUNCTION_PARAMETERS);

typedef struct _crex_internal_function {
	/* Common elements */
	uint8_t type;
	uint8_t arg_flags[3]; /* bitset of arg_info.pass_by_reference */
	uint32_t fn_flags;
	crex_string* function_name;
	crex_class_entry *scope;
	crex_function *prototype;
	uint32_t num_args;
	uint32_t required_num_args;
	crex_internal_arg_info *arg_info;
	HashTable *attributes;
	CREX_MAP_PTR_DEF(void **, run_time_cache);
	uint32_t T;         /* number of temporary variables */
	/* END of common elements */

	zif_handler handler;
	struct _crex_module_entry *module;
	void *reserved[CREX_MAX_RESERVED_RESOURCES];
} crex_internal_function;

#define CREX_FN_SCOPE_NAME(function)  ((function) && (function)->common.scope ? ZSTR_VAL((function)->common.scope->name) : "")

union _crex_function {
	uint8_t type;	/* MUST be the first element of this struct! */
	uint32_t   quick_arg_flags;

	struct {
		uint8_t type;  /* never used */
		uint8_t arg_flags[3]; /* bitset of arg_info.pass_by_reference */
		uint32_t fn_flags;
		crex_string *function_name;
		crex_class_entry *scope;
		crex_function *prototype;
		uint32_t num_args;
		uint32_t required_num_args;
		crex_arg_info *arg_info;  /* index -1 represents the return value info, if any */
		HashTable   *attributes;
		CREX_MAP_PTR_DEF(void **, run_time_cache);
		uint32_t T;         /* number of temporary variables */
	} common;

	crex_op_array op_array;
	crex_internal_function internal_function;
};

struct _crex_execute_data {
	const crex_op       *opline;           /* executed opline                */
	crex_execute_data   *call;             /* current call                   */
	zval                *return_value;
	crex_function       *func;             /* executed function              */
	zval                 This;             /* this + call_info + num_args    */
	crex_execute_data   *prev_execute_data;
	crex_array          *symbol_table;
	void               **run_time_cache;   /* cache op_array->run_time_cache */
	crex_array          *extra_named_params;
};

#define CREX_CALL_HAS_THIS           IS_OBJECT_EX

/* Top 16 bits of C_TYPE_INFO(EX(This)) are used as call_info flags */
#define CREX_CALL_FUNCTION           (0 << 16)
#define CREX_CALL_CODE               (1 << 16)
#define CREX_CALL_NESTED             (0 << 17)
#define CREX_CALL_TOP                (1 << 17)
#define CREX_CALL_ALLOCATED          (1 << 18)
#define CREX_CALL_FREE_EXTRA_ARGS    (1 << 19)
#define CREX_CALL_HAS_SYMBOL_TABLE   (1 << 20)
#define CREX_CALL_RELEASE_THIS       (1 << 21)
#define CREX_CALL_CLOSURE            (1 << 22)
#define CREX_CALL_FAKE_CLOSURE       (1 << 23) /* Same as CREX_ACC_FAKE_CLOSURE */
#define CREX_CALL_GENERATOR          (1 << 24)
#define CREX_CALL_DYNAMIC            (1 << 25)
#define CREX_CALL_MAY_HAVE_UNDEF     (1 << 26)
#define CREX_CALL_HAS_EXTRA_NAMED_PARAMS (1 << 27)
#define CREX_CALL_OBSERVED           (1 << 28) /* "fcall_begin" observer handler may set this flag */
                                               /* to prevent optimization in RETURN handler and    */
                                               /* keep all local variables for "fcall_end" handler */
#define CREX_CALL_JIT_RESERVED       (1 << 29) /* reserved for tracing JIT */
#define CREX_CALL_NEEDS_REATTACH     (1 << 30)
#define CREX_CALL_SEND_ARG_BY_REF    (1u << 31)

#define CREX_CALL_NESTED_FUNCTION    (CREX_CALL_FUNCTION | CREX_CALL_NESTED)
#define CREX_CALL_NESTED_CODE        (CREX_CALL_CODE | CREX_CALL_NESTED)
#define CREX_CALL_TOP_FUNCTION       (CREX_CALL_TOP | CREX_CALL_FUNCTION)
#define CREX_CALL_TOP_CODE           (CREX_CALL_CODE | CREX_CALL_TOP)

#define CREX_CALL_INFO(call) \
	C_TYPE_INFO((call)->This)

#define CREX_CALL_KIND_EX(call_info) \
	(call_info & (CREX_CALL_CODE | CREX_CALL_TOP))

#define CREX_CALL_KIND(call) \
	CREX_CALL_KIND_EX(CREX_CALL_INFO(call))

#define CREX_ADD_CALL_FLAG_EX(call_info, flag) do { \
		call_info |= (flag); \
	} while (0)

#define CREX_DEL_CALL_FLAG_EX(call_info, flag) do { \
		call_info &= ~(flag); \
	} while (0)

#define CREX_ADD_CALL_FLAG(call, flag) do { \
		CREX_ADD_CALL_FLAG_EX(C_TYPE_INFO((call)->This), flag); \
	} while (0)

#define CREX_DEL_CALL_FLAG(call, flag) do { \
		CREX_DEL_CALL_FLAG_EX(C_TYPE_INFO((call)->This), flag); \
	} while (0)

#define CREX_CALL_NUM_ARGS(call) \
	(call)->This.u2.num_args

/* Ensure the correct alignment before slots calculation */
CREX_STATIC_ASSERT(CREX_MM_ALIGNED_SIZE(sizeof(zval)) == sizeof(zval),
                   "zval must be aligned by CREX_MM_ALIGNMENT");
/* A number of call frame slots (zvals) reserved for crex_execute_data. */
#define CREX_CALL_FRAME_SLOT \
	((int)((sizeof(crex_execute_data) + sizeof(zval) - 1) / sizeof(zval)))

#define CREX_CALL_VAR(call, n) \
	((zval*)(((char*)(call)) + ((int)(n))))

#define CREX_CALL_VAR_NUM(call, n) \
	(((zval*)(call)) + (CREX_CALL_FRAME_SLOT + ((int)(n))))

#define CREX_CALL_ARG(call, n) \
	CREX_CALL_VAR_NUM(call, ((int)(n)) - 1)

#define EX(element) 			((execute_data)->element)

#define EX_CALL_INFO()			CREX_CALL_INFO(execute_data)
#define EX_CALL_KIND()			CREX_CALL_KIND(execute_data)
#define EX_NUM_ARGS()			CREX_CALL_NUM_ARGS(execute_data)

#define CREX_CALL_USES_STRICT_TYPES(call) \
	(((call)->func->common.fn_flags & CREX_ACC_STRICT_TYPES) != 0)

#define EX_USES_STRICT_TYPES() \
	CREX_CALL_USES_STRICT_TYPES(execute_data)

#define CREX_ARG_USES_STRICT_TYPES() \
	(EG(current_execute_data)->prev_execute_data && \
	 EG(current_execute_data)->prev_execute_data->func && \
	 CREX_CALL_USES_STRICT_TYPES(EG(current_execute_data)->prev_execute_data))

#define CREX_RET_USES_STRICT_TYPES() \
	CREX_CALL_USES_STRICT_TYPES(EG(current_execute_data))

#define EX_VAR(n)				CREX_CALL_VAR(execute_data, n)
#define EX_VAR_NUM(n)			CREX_CALL_VAR_NUM(execute_data, n)

#define EX_VAR_TO_NUM(n)		((uint32_t)((n) / sizeof(zval) - CREX_CALL_FRAME_SLOT))
#define EX_NUM_TO_VAR(n)		((uint32_t)(((n) + CREX_CALL_FRAME_SLOT) * sizeof(zval)))

#define CREX_OPLINE_TO_OFFSET(opline, target) \
	((char*)(target) - (char*)(opline))

#define CREX_OPLINE_NUM_TO_OFFSET(op_array, opline, opline_num) \
	((char*)&(op_array)->opcodes[opline_num] - (char*)(opline))

#define CREX_OFFSET_TO_OPLINE(base, offset) \
	((crex_op*)(((char*)(base)) + (int)offset))

#define CREX_OFFSET_TO_OPLINE_NUM(op_array, base, offset) \
	(CREX_OFFSET_TO_OPLINE(base, offset) - op_array->opcodes)

#if CREX_USE_ABS_JMP_ADDR

/* run-time jump target */
# define OP_JMP_ADDR(opline, node) \
	(node).jmp_addr

# define CREX_SET_OP_JMP_ADDR(opline, node, val) do { \
		(node).jmp_addr = (val); \
	} while (0)

/* convert jump target from compile-time to run-time */
# define CREX_PASS_TWO_UPDATE_JMP_TARGET(op_array, opline, node) do { \
		(node).jmp_addr = (op_array)->opcodes + (node).opline_num; \
	} while (0)

/* convert jump target back from run-time to compile-time */
# define CREX_PASS_TWO_UNDO_JMP_TARGET(op_array, opline, node) do { \
		(node).opline_num = (node).jmp_addr - (op_array)->opcodes; \
	} while (0)

#else

/* run-time jump target */
# define OP_JMP_ADDR(opline, node) \
	CREX_OFFSET_TO_OPLINE(opline, (node).jmp_offset)

# define CREX_SET_OP_JMP_ADDR(opline, node, val) do { \
		(node).jmp_offset = CREX_OPLINE_TO_OFFSET(opline, val); \
	} while (0)

/* convert jump target from compile-time to run-time */
# define CREX_PASS_TWO_UPDATE_JMP_TARGET(op_array, opline, node) do { \
		(node).jmp_offset = CREX_OPLINE_NUM_TO_OFFSET(op_array, opline, (node).opline_num); \
	} while (0)

/* convert jump target back from run-time to compile-time */
# define CREX_PASS_TWO_UNDO_JMP_TARGET(op_array, opline, node) do { \
		(node).opline_num = CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, (node).jmp_offset); \
	} while (0)

#endif

/* constant-time constant */
# define CT_CONSTANT_EX(op_array, num) \
	((op_array)->literals + (num))

# define CT_CONSTANT(node) \
	CT_CONSTANT_EX(CG(active_op_array), (node).constant)

#if CREX_USE_ABS_CONST_ADDR

/* run-time constant */
# define RT_CONSTANT(opline, node) \
	(node).zv

/* convert constant from compile-time to run-time */
# define CREX_PASS_TWO_UPDATE_CONSTANT(op_array, opline, node) do { \
		(node).zv = CT_CONSTANT_EX(op_array, (node).constant); \
	} while (0)

#else

/* At run-time, constants are allocated together with op_array->opcodes
 * and addressed relatively to current opline.
 */

/* run-time constant */
# define RT_CONSTANT(opline, node) \
	((zval*)(((char*)(opline)) + (int32_t)(node).constant))

/* convert constant from compile-time to run-time */
# define CREX_PASS_TWO_UPDATE_CONSTANT(op_array, opline, node) do { \
		(node).constant = \
			(((char*)CT_CONSTANT_EX(op_array, (node).constant)) - \
			((char*)opline)); \
	} while (0)

#endif

/* convert constant back from run-time to compile-time */
#define CREX_PASS_TWO_UNDO_CONSTANT(op_array, opline, node) do { \
		(node).constant = RT_CONSTANT(opline, node) - (op_array)->literals; \
	} while (0)

#define RUN_TIME_CACHE(op_array) \
	CREX_MAP_PTR_GET((op_array)->run_time_cache)

#define CREX_OP_ARRAY_EXTENSION(op_array, handle) \
	((void**)RUN_TIME_CACHE(op_array))[handle]

#define IS_UNUSED	0		/* Unused operand */
#define IS_CONST	(1<<0)
#define IS_TMP_VAR	(1<<1)
#define IS_VAR		(1<<2)
#define IS_CV		(1<<3)	/* Compiled variable */

/* Used for result.type of smart branch instructions */
#define IS_SMART_BRANCH_JMPZ  (1<<4)
#define IS_SMART_BRANCH_JMPNZ (1<<5)

#define CREX_EXTRA_VALUE 1

#include "crex_globals.h"

typedef enum _crex_compile_position {
	CREX_COMPILE_POSITION_AT_SHEBANG = 0,
	CREX_COMPILE_POSITION_AT_OPEN_TAG,
	CREX_COMPILE_POSITION_AFTER_OPEN_TAG
} crex_compile_position;

BEGIN_EXTERN_C()

void init_compiler(void);
void shutdown_compiler(void);
void crex_init_compiler_data_structures(void);

void crex_oparray_context_begin(crex_oparray_context *prev_context);
void crex_oparray_context_end(crex_oparray_context *prev_context);
void crex_file_context_begin(crex_file_context *prev_context);
void crex_file_context_end(crex_file_context *prev_context);

extern CREX_API crex_op_array *(*crex_compile_file)(crex_file_handle *file_handle, int type);
extern CREX_API crex_op_array *(*crex_compile_string)(crex_string *source_string, const char *filename, crex_compile_position position);

CREX_API int CREX_FASTCALL lex_scan(zval *crexlval, crex_parser_stack_elem *elem);
void startup_scanner(void);
void shutdown_scanner(void);

CREX_API crex_string *crex_set_compiled_filename(crex_string *new_compiled_filename);
CREX_API void crex_restore_compiled_filename(crex_string *original_compiled_filename);
CREX_API crex_string *crex_get_compiled_filename(void);
CREX_API int crex_get_compiled_lineno(void);
CREX_API size_t crex_get_scanned_file_offset(void);

CREX_API crex_string *crex_get_compiled_variable_name(const crex_op_array *op_array, uint32_t var);

#ifdef ZTS
const char *crex_get_crextext(void);
int crex_get_crexleng(void);
#endif

typedef crex_result (CREX_FASTCALL *unary_op_type)(zval *, zval *);
typedef crex_result (CREX_FASTCALL *binary_op_type)(zval *, zval *, zval *);

CREX_API unary_op_type get_unary_op(int opcode);
CREX_API binary_op_type get_binary_op(int opcode);

void crex_stop_lexing(void);
void crex_emit_final_return(bool return_one);

typedef enum {
	CREX_MODIFIER_TARGET_PROPERTY = 0,
	CREX_MODIFIER_TARGET_METHOD,
	CREX_MODIFIER_TARGET_CONSTANT,
	CREX_MODIFIER_TARGET_CPP,
} crex_modifier_target;

/* Used during AST construction */
crex_ast *crex_ast_append_str(crex_ast *left, crex_ast *right);
crex_ast *crex_negate_num_string(crex_ast *ast);
uint32_t crex_add_class_modifier(uint32_t flags, uint32_t new_flag);
uint32_t crex_add_anonymous_class_modifier(uint32_t flags, uint32_t new_flag);
uint32_t crex_add_member_modifier(uint32_t flags, uint32_t new_flag, crex_modifier_target target);

uint32_t crex_modifier_token_to_flag(crex_modifier_target target, uint32_t flags);
uint32_t crex_modifier_list_to_flags(crex_modifier_target target, crex_ast *modifiers);

bool crex_handle_encoding_declaration(crex_ast *ast);

CREX_API crex_class_entry *crex_bind_class_in_slot(
		zval *class_table_slot, zval *lcname, crex_string *lc_parent_name);
CREX_API crex_result do_bind_function(crex_function *func, zval *lcname);
CREX_API crex_result do_bind_class(zval *lcname, crex_string *lc_parent_name);

void crex_resolve_goto_label(crex_op_array *op_array, crex_op *opline);

CREX_API void function_add_ref(crex_function *function);
crex_string *zval_make_interned_string(zval *zv);

#define INITIAL_OP_ARRAY_SIZE 64


/* helper functions in crex_language_scanner.l */
struct _crex_arena;

CREX_API crex_op_array *compile_file(crex_file_handle *file_handle, int type);
CREX_API crex_op_array *compile_string(crex_string *source_string, const char *filename, crex_compile_position position);
CREX_API crex_op_array *compile_filename(int type, crex_string *filename);
CREX_API crex_ast *crex_compile_string_to_ast(
		crex_string *code, struct _crex_arena **ast_arena, crex_string *filename);
CREX_API crex_result crex_execute_scripts(int type, zval *retval, int file_count, ...);
CREX_API crex_result open_file_for_scanning(crex_file_handle *file_handle);
CREX_API void init_op_array(crex_op_array *op_array, uint8_t type, int initial_ops_size);
CREX_API void destroy_op_array(crex_op_array *op_array);
CREX_API void crex_destroy_static_vars(crex_op_array *op_array);
CREX_API void crex_destroy_file_handle(crex_file_handle *file_handle);
CREX_API void crex_cleanup_mutable_class_data(crex_class_entry *ce);
CREX_API void crex_cleanup_internal_class_data(crex_class_entry *ce);
CREX_API void crex_type_release(crex_type type, bool persistent);
CREX_API crex_string *crex_create_member_string(crex_string *class_name, crex_string *member_name);


CREX_API CREX_COLD void crex_user_exception_handler(void);

#define crex_try_exception_handler() do { \
		if (UNEXPECTED(EG(exception))) { \
			if (C_TYPE(EG(user_exception_handler)) != IS_UNDEF) { \
				crex_user_exception_handler(); \
			} \
		} \
	} while (0)

void crex_free_internal_arg_info(crex_internal_function *function);
CREX_API void destroy_crex_function(crex_function *function);
CREX_API void crex_function_dtor(zval *zv);
CREX_API void destroy_crex_class(zval *zv);
void crex_class_add_ref(zval *zv);

CREX_API crex_string *crex_mangle_property_name(const char *src1, size_t src1_length, const char *src2, size_t src2_length, bool internal);
#define crex_unmangle_property_name(mangled_property, class_name, prop_name) \
        crex_unmangle_property_name_ex(mangled_property, class_name, prop_name, NULL)
CREX_API crex_result crex_unmangle_property_name_ex(const crex_string *name, const char **class_name, const char **prop_name, size_t *prop_len);

static crex_always_inline const char *crex_get_unmangled_property_name(const crex_string *mangled_prop) {
	const char *class_name, *prop_name;
	crex_unmangle_property_name(mangled_prop, &class_name, &prop_name);
	return prop_name;
}

#define CREX_FUNCTION_DTOR crex_function_dtor
#define CREX_CLASS_DTOR destroy_crex_class

typedef bool (*crex_needs_live_range_cb)(crex_op_array *op_array, crex_op *opline);
CREX_API void crex_recalc_live_ranges(
	crex_op_array *op_array, crex_needs_live_range_cb needs_live_range);

CREX_API void pass_two(crex_op_array *op_array);
CREX_API bool crex_is_compiling(void);
CREX_API char *crex_make_compiled_string_description(const char *name);
CREX_API void crex_initialize_class_data(crex_class_entry *ce, bool nullify_handlers);
uint32_t crex_get_class_fetch_type(const crex_string *name);
CREX_API uint8_t crex_get_call_op(const crex_op *init_op, crex_function *fbc);
CREX_API bool crex_is_smart_branch(const crex_op *opline);

typedef bool (*crex_auto_global_callback)(crex_string *name);
typedef struct _crex_auto_global {
	crex_string *name;
	crex_auto_global_callback auto_global_callback;
	bool jit;
	bool armed;
} crex_auto_global;

CREX_API crex_result crex_register_auto_global(crex_string *name, bool jit, crex_auto_global_callback auto_global_callback);
CREX_API void crex_activate_auto_globals(void);
CREX_API bool crex_is_auto_global(crex_string *name);
CREX_API bool crex_is_auto_global_str(const char *name, size_t len);
CREX_API size_t crex_dirname(char *path, size_t len);
CREX_API void crex_set_function_arg_flags(crex_function *func);

int CREX_FASTCALL crexlex(crex_parser_stack_elem *elem);

void crex_assert_valid_class_name(const crex_string *const_name);

crex_string *crex_type_to_string_resolved(crex_type type, crex_class_entry *scope);
CREX_API crex_string *crex_type_to_string(crex_type type);

/* BEGIN: OPCODES */

#include "crex_vm_opcodes.h"

/* END: OPCODES */

/* class fetches */
#define CREX_FETCH_CLASS_DEFAULT	0
#define CREX_FETCH_CLASS_SELF		1
#define CREX_FETCH_CLASS_PARENT		2
#define CREX_FETCH_CLASS_STATIC		3
#define CREX_FETCH_CLASS_AUTO		4
#define CREX_FETCH_CLASS_INTERFACE	5
#define CREX_FETCH_CLASS_TRAIT		6
#define CREX_FETCH_CLASS_MASK        0x0f
#define CREX_FETCH_CLASS_NO_AUTOLOAD 0x80
#define CREX_FETCH_CLASS_SILENT      0x0100
#define CREX_FETCH_CLASS_EXCEPTION   0x0200
#define CREX_FETCH_CLASS_ALLOW_UNLINKED 0x0400
#define CREX_FETCH_CLASS_ALLOW_NEARLY_LINKED 0x0800

/* These should not clash with CREX_ACC_(PUBLIC|PROTECTED|PRIVATE) */
#define CREX_PARAM_REF      (1<<3)
#define CREX_PARAM_VARIADIC (1<<4)

#define CREX_NAME_FQ       0
#define CREX_NAME_NOT_FQ   1
#define CREX_NAME_RELATIVE 2

/* CREX_FETCH_ flags in class name AST of new const expression must not clash with CREX_NAME_ flags */
#define CREX_CONST_EXPR_NEW_FETCH_TYPE_SHIFT 2

#define CREX_TYPE_NULLABLE (1<<8)

#define CREX_ARRAY_SYNTAX_LIST 1  /* list() */
#define CREX_ARRAY_SYNTAX_LONG 2  /* array() */
#define CREX_ARRAY_SYNTAX_SHORT 3 /* [] */

/* var status for backpatching */
#define BP_VAR_R			0
#define BP_VAR_W			1
#define BP_VAR_RW			2
#define BP_VAR_IS			3
#define BP_VAR_FUNC_ARG		4
#define BP_VAR_UNSET		5

#define CREX_INTERNAL_FUNCTION		1
#define CREX_USER_FUNCTION			2
#define CREX_EVAL_CODE				4

#define CREX_USER_CODE(type)		((type) != CREX_INTERNAL_FUNCTION)

#define CREX_INTERNAL_CLASS         1
#define CREX_USER_CLASS             2

#define CREX_EVAL				(1<<0)
#define CREX_INCLUDE			(1<<1)
#define CREX_INCLUDE_ONCE		(1<<2)
#define CREX_REQUIRE			(1<<3)
#define CREX_REQUIRE_ONCE		(1<<4)

/* global/local fetches */
#define CREX_FETCH_GLOBAL		(1<<1)
#define CREX_FETCH_LOCAL		(1<<2)
#define CREX_FETCH_GLOBAL_LOCK	(1<<3)

#define CREX_FETCH_TYPE_MASK	0xe

/* Only one of these can ever be in use */
#define CREX_FETCH_REF			1
#define CREX_FETCH_DIM_WRITE	2
#define CREX_FETCH_OBJ_FLAGS	3

/* Used to mark what kind of operation a writing FETCH_DIM is used in,
 * to produce a more precise error on incorrect string offset use. */
#define CREX_FETCH_DIM_REF 1
#define CREX_FETCH_DIM_DIM 2
#define CREX_FETCH_DIM_OBJ 3
#define CREX_FETCH_DIM_INCDEC 4

#define CREX_ISEMPTY			(1<<0)

#define CREX_LAST_CATCH			(1<<0)

#define CREX_FREE_ON_RETURN     (1<<0)
#define CREX_FREE_SWITCH        (1<<1)

#define CREX_SEND_BY_VAL     0u
#define CREX_SEND_BY_REF     1u
#define CREX_SEND_PREFER_REF 2u

#define CREX_THROW_IS_EXPR 1u

#define CREX_FCALL_MAY_HAVE_EXTRA_NAMED_PARAMS 1

/* The send mode, the is_variadic, the is_promoted, and the is_tentative flags are stored as part of crex_type */
#define _CREX_SEND_MODE_SHIFT _CREX_TYPE_EXTRA_FLAGS_SHIFT
#define _CREX_IS_VARIADIC_BIT (1 << (_CREX_TYPE_EXTRA_FLAGS_SHIFT + 2))
#define _CREX_IS_PROMOTED_BIT (1 << (_CREX_TYPE_EXTRA_FLAGS_SHIFT + 3))
#define _CREX_IS_TENTATIVE_BIT (1 << (_CREX_TYPE_EXTRA_FLAGS_SHIFT + 4))
#define CREX_ARG_SEND_MODE(arg_info) \
	((CREX_TYPE_FULL_MASK((arg_info)->type) >> _CREX_SEND_MODE_SHIFT) & 3)
#define CREX_ARG_IS_VARIADIC(arg_info) \
	((CREX_TYPE_FULL_MASK((arg_info)->type) & _CREX_IS_VARIADIC_BIT) != 0)
#define CREX_ARG_IS_PROMOTED(arg_info) \
	((CREX_TYPE_FULL_MASK((arg_info)->type) & _CREX_IS_PROMOTED_BIT) != 0)
#define CREX_ARG_TYPE_IS_TENTATIVE(arg_info) \
	((CREX_TYPE_FULL_MASK((arg_info)->type) & _CREX_IS_TENTATIVE_BIT) != 0)

#define CREX_DIM_IS					(1 << 0) /* isset fetch needed for null coalesce. Set in crex_compile.c for CREX_AST_DIM nested within CREX_AST_COALESCE. */
#define CREX_DIM_ALTERNATIVE_SYNTAX	(1 << 1) /* deprecated curly brace usage */

/* Attributes for ${} encaps var in strings (CREX_AST_DIM or CREX_AST_VAR node) */
/* CREX_AST_VAR nodes can have any of the CREX_ENCAPS_VAR_* flags */
/* CREX_AST_DIM flags can have CREX_DIM_ALTERNATIVE_SYNTAX or CREX_ENCAPS_VAR_DOLLAR_CURLY during the parse phase (CREX_DIM_ALTERNATIVE_SYNTAX is a thrown fatal error). */
#define CREX_ENCAPS_VAR_DOLLAR_CURLY (1 << 0)
#define CREX_ENCAPS_VAR_DOLLAR_CURLY_VAR_VAR (1 << 1)

/* Make sure these don't clash with CREX_FETCH_CLASS_* flags. */
#define IS_CONSTANT_CLASS                    0x400 /* __CLASS__ in trait */
#define IS_CONSTANT_UNQUALIFIED_IN_NAMESPACE 0x800

static crex_always_inline bool crex_check_arg_send_type(const crex_function *zf, uint32_t arg_num, uint32_t mask)
{
	arg_num--;
	if (UNEXPECTED(arg_num >= zf->common.num_args)) {
		if (EXPECTED((zf->common.fn_flags & CREX_ACC_VARIADIC) == 0)) {
			return 0;
		}
		arg_num = zf->common.num_args;
	}
	return UNEXPECTED((CREX_ARG_SEND_MODE(&zf->common.arg_info[arg_num]) & mask) != 0);
}

#define ARG_MUST_BE_SENT_BY_REF(zf, arg_num) \
	crex_check_arg_send_type(zf, arg_num, CREX_SEND_BY_REF)

#define ARG_SHOULD_BE_SENT_BY_REF(zf, arg_num) \
	crex_check_arg_send_type(zf, arg_num, CREX_SEND_BY_REF|CREX_SEND_PREFER_REF)

#define ARG_MAY_BE_SENT_BY_REF(zf, arg_num) \
	crex_check_arg_send_type(zf, arg_num, CREX_SEND_PREFER_REF)

/* Quick API to check first 12 arguments */
#define MAX_ARG_FLAG_NUM 12

#ifdef WORDS_BIGENDIAN
# define CREX_SET_ARG_FLAG(zf, arg_num, mask) do { \
		(zf)->quick_arg_flags |= ((mask) << ((arg_num) - 1) * 2); \
	} while (0)
# define CREX_CHECK_ARG_FLAG(zf, arg_num, mask) \
	(((zf)->quick_arg_flags >> (((arg_num) - 1) * 2)) & (mask))
#else
# define CREX_SET_ARG_FLAG(zf, arg_num, mask) do { \
		(zf)->quick_arg_flags |= (((mask) << 6) << (arg_num) * 2); \
	} while (0)
# define CREX_CHECK_ARG_FLAG(zf, arg_num, mask) \
	(((zf)->quick_arg_flags >> (((arg_num) + 3) * 2)) & (mask))
#endif

#define QUICK_ARG_MUST_BE_SENT_BY_REF(zf, arg_num) \
	CREX_CHECK_ARG_FLAG(zf, arg_num, CREX_SEND_BY_REF)

#define QUICK_ARG_SHOULD_BE_SENT_BY_REF(zf, arg_num) \
	CREX_CHECK_ARG_FLAG(zf, arg_num, CREX_SEND_BY_REF|CREX_SEND_PREFER_REF)

#define QUICK_ARG_MAY_BE_SENT_BY_REF(zf, arg_num) \
	CREX_CHECK_ARG_FLAG(zf, arg_num, CREX_SEND_PREFER_REF)

#define CREX_RETURN_VAL 0
#define CREX_RETURN_REF 1

#define CREX_BIND_VAL      0
#define CREX_BIND_REF      1
#define CREX_BIND_IMPLICIT 2
#define CREX_BIND_EXPLICIT 4

#define CREX_RETURNS_FUNCTION (1<<0)
#define CREX_RETURNS_VALUE    (1<<1)

#define CREX_ARRAY_ELEMENT_REF		(1<<0)
#define CREX_ARRAY_NOT_PACKED		(1<<1)
#define CREX_ARRAY_SIZE_SHIFT		2

/* Attribute for ternary inside parentheses */
#define CREX_PARENTHESIZED_CONDITIONAL 1

/* For "use" AST nodes and the seen symbol table */
#define CREX_SYMBOL_CLASS    (1<<0)
#define CREX_SYMBOL_FUNCTION (1<<1)
#define CREX_SYMBOL_CONST    (1<<2)

/* All increment opcodes are even (decrement are odd) */
#define CREX_IS_INCREMENT(opcode) (((opcode) & 1) == 0)

#define CREX_IS_BINARY_ASSIGN_OP_OPCODE(opcode) \
	(((opcode) >= CREX_ADD) && ((opcode) <= CREX_POW))

/* Pseudo-opcodes that are used only temporarily during compilation */
#define CREX_GOTO  253
#define CREX_BRK   254
#define CREX_CONT  255


END_EXTERN_C()

#define CREX_CLONE_FUNC_NAME		"__clone"
#define CREX_CONSTRUCTOR_FUNC_NAME	"__main"
#define CREX_DESTRUCTOR_FUNC_NAME	"__destruct"
#define CREX_GET_FUNC_NAME          "__get"
#define CREX_SET_FUNC_NAME          "__set"
#define CREX_UNSET_FUNC_NAME        "__unset"
#define CREX_ISSET_FUNC_NAME        "__isset"
#define CREX_CALL_FUNC_NAME         "__call"
#define CREX_CALLSTATIC_FUNC_NAME   "__callstatic"
#define CREX_TOSTRING_FUNC_NAME     "__tostring"
#define CREX_INVOKE_FUNC_NAME       "__invoke"
#define CREX_DEBUGINFO_FUNC_NAME    "__debuginfo"

/* The following constants may be combined in CG(compiler_options)
 * to change the default compiler behavior */

/* generate extended debug information */
#define CREX_COMPILE_EXTENDED_STMT              (1<<0)
#define CREX_COMPILE_EXTENDED_FCALL             (1<<1)
#define CREX_COMPILE_EXTENDED_INFO              (CREX_COMPILE_EXTENDED_STMT|CREX_COMPILE_EXTENDED_FCALL)

/* call op_array handler of extensions */
#define CREX_COMPILE_HANDLE_OP_ARRAY            (1<<2)

/* generate CREX_INIT_FCALL_BY_NAME for internal functions instead of CREX_INIT_FCALL */
#define CREX_COMPILE_IGNORE_INTERNAL_FUNCTIONS  (1<<3)

/* don't perform early binding for classes inherited form internal ones;
 * in namespaces assume that internal class that doesn't exist at compile-time
 * may appear in run-time */
#define CREX_COMPILE_IGNORE_INTERNAL_CLASSES    (1<<4)

/* generate CREX_DECLARE_CLASS_DELAYED opcode to delay early binding */
#define CREX_COMPILE_DELAYED_BINDING            (1<<5)

/* disable constant substitution at compile-time */
#define CREX_COMPILE_NO_CONSTANT_SUBSTITUTION   (1<<6)

/* disable substitution of persistent constants at compile-time */
#define CREX_COMPILE_NO_PERSISTENT_CONSTANT_SUBSTITUTION	(1<<8)

/* generate CREX_INIT_FCALL_BY_NAME for userland functions instead of CREX_INIT_FCALL */
#define CREX_COMPILE_IGNORE_USER_FUNCTIONS      (1<<9)

/* force CREX_ACC_USE_GUARDS for all classes */
#define CREX_COMPILE_GUARDS						(1<<10)

/* disable builtin special case function calls */
#define CREX_COMPILE_NO_BUILTINS				(1<<11)

/* result of compilation may be stored in file cache */
#define CREX_COMPILE_WITH_FILE_CACHE			(1<<12)

/* ignore functions and classes declared in other files */
#define CREX_COMPILE_IGNORE_OTHER_FILES			(1<<13)

/* this flag is set when compiler invoked by opcache_compile_file() */
#define CREX_COMPILE_WITHOUT_EXECUTION          (1<<14)

/* this flag is set when compiler invoked during preloading */
#define CREX_COMPILE_PRELOAD                    (1<<15)

/* disable jumptable optimization for switch statements */
#define CREX_COMPILE_NO_JUMPTABLES				(1<<16)

/* this flag is set when compiler invoked during preloading in separate process */
#define CREX_COMPILE_PRELOAD_IN_CHILD           (1<<17)

/* ignore observer notifications, e.g. to manually notify afterwards in a post-processing step after compilation */
#define CREX_COMPILE_IGNORE_OBSERVER			(1<<18)

/* The default value for CG(compiler_options) */
#define CREX_COMPILE_DEFAULT					CREX_COMPILE_HANDLE_OP_ARRAY

/* The default value for CG(compiler_options) during eval() */
#define CREX_COMPILE_DEFAULT_FOR_EVAL			0

CREX_API bool crex_is_op_long_compatible(const zval *op);
CREX_API bool crex_binary_op_produces_error(uint32_t opcode, const zval *op1, const zval *op2);
CREX_API bool crex_unary_op_produces_error(uint32_t opcode, const zval *op);

#endif /* CREX_COMPILE_H */
