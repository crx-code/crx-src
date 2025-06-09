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
   |          Nikita Popov <nikic@crx.net>                                |
   +----------------------------------------------------------------------+
*/

#include <crex_language_parser.h>
#include "crex.h"
#include "crex_attributes.h"
#include "crex_compile.h"
#include "crex_constants.h"
#include "crex_llist.h"
#include "crex_API.h"
#include "crex_exceptions.h"
#include "crex_interfaces.h"
#include "crex_virtual_cwd.h"
#include "crex_multibyte.h"
#include "crex_language_scanner.h"
#include "crex_inheritance.h"
#include "crex_vm.h"
#include "crex_enum.h"
#include "crex_observer.h"
#include "crex_call_stack.h"

#define SET_NODE(target, src) do { \
		target ## _type = (src)->op_type; \
		if ((src)->op_type == IS_CONST) { \
			target.constant = crex_add_literal(&(src)->u.constant); \
		} else { \
			target = (src)->u.op; \
		} \
	} while (0)

#define GET_NODE(target, src) do { \
		(target)->op_type = src ## _type; \
		if ((target)->op_type == IS_CONST) { \
			ZVAL_COPY_VALUE(&(target)->u.constant, CT_CONSTANT(src)); \
		} else { \
			(target)->u.op = src; \
		} \
	} while (0)

#define FC(member) (CG(file_context).member)

typedef struct _crex_loop_var {
	uint8_t opcode;
	uint8_t var_type;
	uint32_t   var_num;
	uint32_t   try_catch_offset;
} crex_loop_var;

static inline uint32_t crex_alloc_cache_slots(unsigned count) {
	if (count == 0) {
		/* Even if no cache slots are desired, the VM handler may still want to acquire
		 * CACHE_ADDR() unconditionally. Returning zero makes sure that the address
		 * calculation is still legal and ubsan does not complain. */
		return 0;
	}

	crex_op_array *op_array = CG(active_op_array);
	uint32_t ret = op_array->cache_size;
	op_array->cache_size += count * sizeof(void*);
	return ret;
}

static inline uint32_t crex_alloc_cache_slot(void) {
	return crex_alloc_cache_slots(1);
}

CREX_API crex_op_array *(*crex_compile_file)(crex_file_handle *file_handle, int type);
CREX_API crex_op_array *(*crex_compile_string)(crex_string *source_string, const char *filename, crex_compile_position position);

#ifndef ZTS
CREX_API crex_compiler_globals compiler_globals;
CREX_API crex_executor_globals executor_globals;
#endif

static crex_op *crex_emit_op(znode *result, uint8_t opcode, znode *op1, znode *op2);
static bool crex_try_ct_eval_array(zval *result, crex_ast *ast);
static void crex_eval_const_expr(crex_ast **ast_ptr);

static crex_op *crex_compile_var(znode *result, crex_ast *ast, uint32_t type, bool by_ref);
static crex_op *crex_delayed_compile_var(znode *result, crex_ast *ast, uint32_t type, bool by_ref);
static void crex_compile_expr(znode *result, crex_ast *ast);
static void crex_compile_stmt(crex_ast *ast);
static void crex_compile_assign(znode *result, crex_ast *ast);

#ifdef CREX_CHECK_STACK_LIMIT
crex_never_inline static void crex_stack_limit_error(void)
{
	crex_error_noreturn(E_COMPILE_ERROR,
		"Maximum call stack size of %zu bytes reached during compilation. Try splitting expression",
		(size_t) ((uintptr_t) EG(stack_base) - (uintptr_t) EG(stack_limit)));
}

static void crex_check_stack_limit(void)
{
	if (UNEXPECTED(crex_call_stack_overflowed(EG(stack_limit)))) {
		crex_stack_limit_error();
	}
}
#else /* CREX_CHECK_STACK_LIMIT */
static void crex_check_stack_limit(void)
{
}
#endif /* CREX_CHECK_STACK_LIMIT */

static void init_op(crex_op *op)
{
	MAKE_NOP(op);
	op->extended_value = 0;
	op->lineno = CG(crex_lineno);
}

static crex_always_inline uint32_t get_next_op_number(void)
{
	return CG(active_op_array)->last;
}

static crex_op *get_next_op(void)
{
	crex_op_array *op_array = CG(active_op_array);
	uint32_t next_op_num = op_array->last++;
	crex_op *next_op;

	if (UNEXPECTED(next_op_num >= CG(context).opcodes_size)) {
		CG(context).opcodes_size *= 4;
		op_array->opcodes = erealloc(op_array->opcodes, CG(context).opcodes_size * sizeof(crex_op));
	}

	next_op = &(op_array->opcodes[next_op_num]);

	init_op(next_op);

	return next_op;
}

static crex_brk_cont_element *get_next_brk_cont_element(void)
{
	CG(context).last_brk_cont++;
	CG(context).brk_cont_array = erealloc(CG(context).brk_cont_array, sizeof(crex_brk_cont_element) * CG(context).last_brk_cont);
	return &CG(context).brk_cont_array[CG(context).last_brk_cont-1];
}

static crex_string *crex_build_runtime_definition_key(crex_string *name, uint32_t start_lineno) /* {{{ */
{
	crex_string *filename = CG(active_op_array)->filename;
	crex_string *result = crex_strpprintf(0, "%c%s%s:%" PRIu32 "$%" PRIx32,
		'\0', ZSTR_VAL(name), ZSTR_VAL(filename), start_lineno, CG(rtd_key_counter)++);
	return crex_new_interned_string(result);
}
/* }}} */

static bool crex_get_unqualified_name(const crex_string *name, const char **result, size_t *result_len) /* {{{ */
{
	const char *ns_separator = crex_memrchr(ZSTR_VAL(name), '\\', ZSTR_LEN(name));
	if (ns_separator != NULL) {
		*result = ns_separator + 1;
		*result_len = ZSTR_VAL(name) + ZSTR_LEN(name) - *result;
		return 1;
	}

	return 0;
}
/* }}} */

struct reserved_class_name {
	const char *name;
	size_t len;
};
static const struct reserved_class_name reserved_class_names[] = {
	{CREX_STRL("bool")},
	{CREX_STRL("false")},
	{CREX_STRL("float")},
	{CREX_STRL("int")},
	{CREX_STRL("null")},
	{CREX_STRL("parent")},
	{CREX_STRL("self")},
	{CREX_STRL("static")},
	{CREX_STRL("string")},
	{CREX_STRL("true")},
	{CREX_STRL("void")},
	{CREX_STRL("never")},
	{CREX_STRL("iterable")},
	{CREX_STRL("object")},
	{CREX_STRL("mixed")},
	{NULL, 0}
};

static bool crex_is_reserved_class_name(const crex_string *name) /* {{{ */
{
	const struct reserved_class_name *reserved = reserved_class_names;

	const char *uqname = ZSTR_VAL(name);
	size_t uqname_len = ZSTR_LEN(name);
	crex_get_unqualified_name(name, &uqname, &uqname_len);

	for (; reserved->name; ++reserved) {
		if (uqname_len == reserved->len
			&& crex_binary_strcasecmp(uqname, uqname_len, reserved->name, reserved->len) == 0
		) {
			return 1;
		}
	}

	return 0;
}
/* }}} */

void crex_assert_valid_class_name(const crex_string *name) /* {{{ */
{
	if (crex_is_reserved_class_name(name)) {
		crex_error_noreturn(E_COMPILE_ERROR,
			"Cannot use '%s' as class name as it is reserved", ZSTR_VAL(name));
	}
}
/* }}} */

typedef struct _builtin_type_info {
	const char* name;
	const size_t name_len;
	const uint8_t type;
} builtin_type_info;

static const builtin_type_info builtin_types[] = {
	{CREX_STRL("null"), IS_NULL},
	{CREX_STRL("true"), IS_TRUE},
	{CREX_STRL("false"), IS_FALSE},
	{CREX_STRL("int"), IS_LONG},
	{CREX_STRL("float"), IS_DOUBLE},
	{CREX_STRL("string"), IS_STRING},
	{CREX_STRL("bool"), _IS_BOOL},
	{CREX_STRL("void"), IS_VOID},
	{CREX_STRL("never"), IS_NEVER},
	{CREX_STRL("iterable"), IS_ITERABLE},
	{CREX_STRL("object"), IS_OBJECT},
	{CREX_STRL("mixed"), IS_MIXED},
	{NULL, 0, IS_UNDEF}
};

typedef struct {
	const char *name;
	size_t name_len;
	const char *correct_name;
} confusable_type_info;

static const confusable_type_info confusable_types[] = {
	{CREX_STRL("boolean"), "bool"},
	{CREX_STRL("integer"), "int"},
	{CREX_STRL("double"), "float"},
	{CREX_STRL("resource"), NULL},
	{NULL, 0, NULL},
};

static crex_always_inline uint8_t crex_lookup_builtin_type_by_name(const crex_string *name) /* {{{ */
{
	const builtin_type_info *info = &builtin_types[0];

	for (; info->name; ++info) {
		if (ZSTR_LEN(name) == info->name_len
			&& crex_binary_strcasecmp(ZSTR_VAL(name), ZSTR_LEN(name), info->name, info->name_len) == 0
		) {
			return info->type;
		}
	}

	return 0;
}
/* }}} */

static crex_always_inline bool crex_is_confusable_type(const crex_string *name, const char **correct_name) /* {{{ */
{
	const confusable_type_info *info = confusable_types;

	/* Intentionally using case-sensitive comparison here, because "integer" is likely intended
	 * as a scalar type, while "Integer" is likely a class type. */
	for (; info->name; ++info) {
		if (crex_string_equals_cstr(name, info->name, info->name_len)) {
			*correct_name = info->correct_name;
			return 1;
		}
	}

	return 0;
}
/* }}} */

static bool crex_is_not_imported(crex_string *name) {
	/* Assuming "name" is unqualified here. */
	return !FC(imports) || crex_hash_find_ptr_lc(FC(imports), name) == NULL;
}

void crex_oparray_context_begin(crex_oparray_context *prev_context) /* {{{ */
{
	*prev_context = CG(context);
	CG(context).opcodes_size = INITIAL_OP_ARRAY_SIZE;
	CG(context).vars_size = 0;
	CG(context).literals_size = 0;
	CG(context).fast_call_var = -1;
	CG(context).try_catch_offset = -1;
	CG(context).current_brk_cont = -1;
	CG(context).last_brk_cont = 0;
	CG(context).brk_cont_array = NULL;
	CG(context).labels = NULL;
}
/* }}} */

void crex_oparray_context_end(crex_oparray_context *prev_context) /* {{{ */
{
	if (CG(context).brk_cont_array) {
		efree(CG(context).brk_cont_array);
		CG(context).brk_cont_array = NULL;
	}
	if (CG(context).labels) {
		crex_hash_destroy(CG(context).labels);
		FREE_HASHTABLE(CG(context).labels);
		CG(context).labels = NULL;
	}
	CG(context) = *prev_context;
}
/* }}} */

static void crex_reset_import_tables(void) /* {{{ */
{
	if (FC(imports)) {
		crex_hash_destroy(FC(imports));
		efree(FC(imports));
		FC(imports) = NULL;
	}

	if (FC(imports_function)) {
		crex_hash_destroy(FC(imports_function));
		efree(FC(imports_function));
		FC(imports_function) = NULL;
	}

	if (FC(imports_const)) {
		crex_hash_destroy(FC(imports_const));
		efree(FC(imports_const));
		FC(imports_const) = NULL;
	}
}
/* }}} */

static void crex_end_namespace(void) /* {{{ */ {
	FC(in_namespace) = 0;
	crex_reset_import_tables();
	if (FC(current_namespace)) {
		crex_string_release_ex(FC(current_namespace), 0);
		FC(current_namespace) = NULL;
	}
}
/* }}} */

void crex_file_context_begin(crex_file_context *prev_context) /* {{{ */
{
	*prev_context = CG(file_context);
	FC(imports) = NULL;
	FC(imports_function) = NULL;
	FC(imports_const) = NULL;
	FC(current_namespace) = NULL;
	FC(in_namespace) = 0;
	FC(has_bracketed_namespaces) = 0;
	FC(declarables).ticks = 0;
	crex_hash_init(&FC(seen_symbols), 8, NULL, NULL, 0);
}
/* }}} */

void crex_file_context_end(crex_file_context *prev_context) /* {{{ */
{
	crex_end_namespace();
	crex_hash_destroy(&FC(seen_symbols));
	CG(file_context) = *prev_context;
}
/* }}} */

void crex_init_compiler_data_structures(void) /* {{{ */
{
	crex_stack_init(&CG(loop_var_stack), sizeof(crex_loop_var));
	crex_stack_init(&CG(delayed_oplines_stack), sizeof(crex_op));
	crex_stack_init(&CG(short_circuiting_opnums), sizeof(uint32_t));
	CG(active_class_entry) = NULL;
	CG(in_compilation) = 0;
	CG(skip_shebang) = 0;

	CG(encoding_declared) = 0;
	CG(memoized_exprs) = NULL;
	CG(memoize_mode) = CREX_MEMOIZE_NONE;
}
/* }}} */

static void crex_register_seen_symbol(crex_string *name, uint32_t kind) {
	zval *zv = crex_hash_find(&FC(seen_symbols), name);
	if (zv) {
		C_LVAL_P(zv) |= kind;
	} else {
		zval tmp;
		ZVAL_LONG(&tmp, kind);
		crex_hash_add_new(&FC(seen_symbols), name, &tmp);
	}
}

static bool crex_have_seen_symbol(crex_string *name, uint32_t kind) {
	zval *zv = crex_hash_find(&FC(seen_symbols), name);
	return zv && (C_LVAL_P(zv) & kind) != 0;
}

void init_compiler(void) /* {{{ */
{
	CG(arena) = crex_arena_create(64 * 1024);
	CG(active_op_array) = NULL;
	memset(&CG(context), 0, sizeof(CG(context)));
	crex_init_compiler_data_structures();
	crex_init_rsrc_list();
	crex_stream_init();
	CG(unclean_shutdown) = 0;

	CG(delayed_variance_obligations) = NULL;
	CG(delayed_autoloads) = NULL;
	CG(unlinked_uses) = NULL;
	CG(current_linking_class) = NULL;
}
/* }}} */

void shutdown_compiler(void) /* {{{ */
{
	/* Reset filename before destroying the arena, as file cache may use arena allocated strings. */
	crex_restore_compiled_filename(NULL);

	crex_stack_destroy(&CG(loop_var_stack));
	crex_stack_destroy(&CG(delayed_oplines_stack));
	crex_stack_destroy(&CG(short_circuiting_opnums));

	if (CG(delayed_variance_obligations)) {
		crex_hash_destroy(CG(delayed_variance_obligations));
		FREE_HASHTABLE(CG(delayed_variance_obligations));
		CG(delayed_variance_obligations) = NULL;
	}
	if (CG(delayed_autoloads)) {
		crex_hash_destroy(CG(delayed_autoloads));
		FREE_HASHTABLE(CG(delayed_autoloads));
		CG(delayed_autoloads) = NULL;
	}
	if (CG(unlinked_uses)) {
		crex_hash_destroy(CG(unlinked_uses));
		FREE_HASHTABLE(CG(unlinked_uses));
		CG(unlinked_uses) = NULL;
	}
	CG(current_linking_class) = NULL;
}
/* }}} */

CREX_API crex_string *crex_set_compiled_filename(crex_string *new_compiled_filename) /* {{{ */
{
	CG(compiled_filename) = crex_string_copy(new_compiled_filename);
	return new_compiled_filename;
}
/* }}} */

CREX_API void crex_restore_compiled_filename(crex_string *original_compiled_filename) /* {{{ */
{
	if (CG(compiled_filename)) {
		crex_string_release(CG(compiled_filename));
		CG(compiled_filename) = NULL;
	}
	CG(compiled_filename) = original_compiled_filename;
}
/* }}} */

CREX_API crex_string *crex_get_compiled_filename(void) /* {{{ */
{
	return CG(compiled_filename);
}
/* }}} */

CREX_API int crex_get_compiled_lineno(void) /* {{{ */
{
	return CG(crex_lineno);
}
/* }}} */

CREX_API bool crex_is_compiling(void) /* {{{ */
{
	return CG(in_compilation);
}
/* }}} */

static crex_always_inline uint32_t get_temporary_variable(void) /* {{{ */
{
	return (uint32_t)CG(active_op_array)->T++;
}
/* }}} */

static int lookup_cv(crex_string *name) /* {{{ */{
	crex_op_array *op_array = CG(active_op_array);
	int i = 0;
	crex_ulong hash_value = crex_string_hash_val(name);

	while (i < op_array->last_var) {
		if (ZSTR_H(op_array->vars[i]) == hash_value
		 && crex_string_equals(op_array->vars[i], name)) {
			return EX_NUM_TO_VAR(i);
		}
		i++;
	}
	i = op_array->last_var;
	op_array->last_var++;
	if (op_array->last_var > CG(context).vars_size) {
		CG(context).vars_size += 16; /* FIXME */
		op_array->vars = erealloc(op_array->vars, CG(context).vars_size * sizeof(crex_string*));
	}

	op_array->vars[i] = crex_string_copy(name);
	return EX_NUM_TO_VAR(i);
}
/* }}} */

crex_string *zval_make_interned_string(zval *zv)
{
	CREX_ASSERT(C_TYPE_P(zv) == IS_STRING);
	C_STR_P(zv) = crex_new_interned_string(C_STR_P(zv));
	if (ZSTR_IS_INTERNED(C_STR_P(zv))) {
		C_TYPE_FLAGS_P(zv) = 0;
	}
	return C_STR_P(zv);
}

/* Common part of crex_add_literal and crex_append_individual_literal */
static inline void crex_insert_literal(crex_op_array *op_array, zval *zv, int literal_position) /* {{{ */
{
	zval *lit = CT_CONSTANT_EX(op_array, literal_position);
	if (C_TYPE_P(zv) == IS_STRING) {
		zval_make_interned_string(zv);
	}
	ZVAL_COPY_VALUE(lit, zv);
	C_EXTRA_P(lit) = 0;
}
/* }}} */

/* Is used while compiling a function, using the context to keep track
   of an approximate size to avoid to relocate to often.
   Literals are truncated to actual size in the second compiler pass (pass_two()). */
static int crex_add_literal(zval *zv) /* {{{ */
{
	crex_op_array *op_array = CG(active_op_array);
	int i = op_array->last_literal;
	op_array->last_literal++;
	if (i >= CG(context).literals_size) {
		while (i >= CG(context).literals_size) {
			CG(context).literals_size += 16; /* FIXME */
		}
		op_array->literals = (zval*)erealloc(op_array->literals, CG(context).literals_size * sizeof(zval));
	}
	crex_insert_literal(op_array, zv, i);
	return i;
}
/* }}} */

static inline int crex_add_literal_string(crex_string **str) /* {{{ */
{
	int ret;
	zval zv;
	ZVAL_STR(&zv, *str);
	ret = crex_add_literal(&zv);
	*str = C_STR(zv);
	return ret;
}
/* }}} */

static int crex_add_func_name_literal(crex_string *name) /* {{{ */
{
	/* Original name */
	int ret = crex_add_literal_string(&name);

	/* Lowercased name */
	crex_string *lc_name = crex_string_tolower(name);
	crex_add_literal_string(&lc_name);

	return ret;
}
/* }}} */

static int crex_add_ns_func_name_literal(crex_string *name) /* {{{ */
{
	const char *unqualified_name;
	size_t unqualified_name_len;

	/* Original name */
	int ret = crex_add_literal_string(&name);

	/* Lowercased name */
	crex_string *lc_name = crex_string_tolower(name);
	crex_add_literal_string(&lc_name);

	/* Lowercased unqualified name */
	if (crex_get_unqualified_name(name, &unqualified_name, &unqualified_name_len)) {
		lc_name = crex_string_alloc(unqualified_name_len, 0);
		crex_str_tolower_copy(ZSTR_VAL(lc_name), unqualified_name, unqualified_name_len);
		crex_add_literal_string(&lc_name);
	}

	return ret;
}
/* }}} */

static int crex_add_class_name_literal(crex_string *name) /* {{{ */
{
	/* Original name */
	int ret = crex_add_literal_string(&name);

	/* Lowercased name */
	crex_string *lc_name = crex_string_tolower(name);
	crex_add_literal_string(&lc_name);

	return ret;
}
/* }}} */

static int crex_add_const_name_literal(crex_string *name, bool unqualified) /* {{{ */
{
	crex_string *tmp_name;

	int ret = crex_add_literal_string(&name);

	size_t ns_len = 0, after_ns_len = ZSTR_LEN(name);
	const char *after_ns = crex_memrchr(ZSTR_VAL(name), '\\', ZSTR_LEN(name));
	if (after_ns) {
		after_ns += 1;
		ns_len = after_ns - ZSTR_VAL(name) - 1;
		after_ns_len = ZSTR_LEN(name) - ns_len - 1;

		/* lowercased namespace name & original constant name */
		tmp_name = crex_string_init(ZSTR_VAL(name), ZSTR_LEN(name), 0);
		crex_str_tolower(ZSTR_VAL(tmp_name), ns_len);
		crex_add_literal_string(&tmp_name);

		if (!unqualified) {
			return ret;
		}
	} else {
		after_ns = ZSTR_VAL(name);
	}

	/* original unqualified constant name */
	tmp_name = crex_string_init(after_ns, after_ns_len, 0);
	crex_add_literal_string(&tmp_name);

	return ret;
}
/* }}} */

#define LITERAL_STR(op, str) do { \
		zval _c; \
		ZVAL_STR(&_c, str); \
		op.constant = crex_add_literal(&_c); \
	} while (0)

void crex_stop_lexing(void)
{
	if (LANG_SCNG(on_event)) {
		LANG_SCNG(on_event)(ON_STOP, END, 0, NULL, 0, LANG_SCNG(on_event_context));
	}

	LANG_SCNG(yy_cursor) = LANG_SCNG(yy_limit);
}

static inline void crex_begin_loop(
		uint8_t free_opcode, const znode *loop_var, bool is_switch) /* {{{ */
{
	crex_brk_cont_element *brk_cont_element;
	int parent = CG(context).current_brk_cont;
	crex_loop_var info = {0};

	CG(context).current_brk_cont = CG(context).last_brk_cont;
	brk_cont_element = get_next_brk_cont_element();
	brk_cont_element->parent = parent;
	brk_cont_element->is_switch = is_switch;

	if (loop_var && (loop_var->op_type & (IS_VAR|IS_TMP_VAR))) {
		uint32_t start = get_next_op_number();

		info.opcode = free_opcode;
		info.var_type = loop_var->op_type;
		info.var_num = loop_var->u.op.var;
		brk_cont_element->start = start;
	} else {
		info.opcode = CREX_NOP;
		/* The start field is used to free temporary variables in case of exceptions.
		 * We won't try to free something of we don't have loop variable.  */
		brk_cont_element->start = -1;
	}

	crex_stack_push(&CG(loop_var_stack), &info);
}
/* }}} */

static inline void crex_end_loop(int cont_addr, const znode *var_node) /* {{{ */
{
	uint32_t end = get_next_op_number();
	crex_brk_cont_element *brk_cont_element
		= &CG(context).brk_cont_array[CG(context).current_brk_cont];
	brk_cont_element->cont = cont_addr;
	brk_cont_element->brk = end;
	CG(context).current_brk_cont = brk_cont_element->parent;

	crex_stack_del_top(&CG(loop_var_stack));
}
/* }}} */

static void crex_do_free(znode *op1) /* {{{ */
{
	if (op1->op_type == IS_TMP_VAR) {
		crex_op *opline = &CG(active_op_array)->opcodes[CG(active_op_array)->last-1];

		while (opline->opcode == CREX_END_SILENCE ||
		       opline->opcode == CREX_OP_DATA) {
			opline--;
		}

		if (opline->result_type == IS_TMP_VAR && opline->result.var == op1->u.op.var) {
			switch (opline->opcode) {
				case CREX_BOOL:
				case CREX_BOOL_NOT:
					/* boolean results don't have to be freed */
					return;
				case CREX_POST_INC_STATIC_PROP:
				case CREX_POST_DEC_STATIC_PROP:
				case CREX_POST_INC_OBJ:
				case CREX_POST_DEC_OBJ:
				case CREX_POST_INC:
				case CREX_POST_DEC:
					/* convert $i++ to ++$i */
					opline->opcode -= 2;
					SET_UNUSED(opline->result);
					return;
				case CREX_ASSIGN:
				case CREX_ASSIGN_DIM:
				case CREX_ASSIGN_OBJ:
				case CREX_ASSIGN_STATIC_PROP:
				case CREX_ASSIGN_OP:
				case CREX_ASSIGN_DIM_OP:
				case CREX_ASSIGN_OBJ_OP:
				case CREX_ASSIGN_STATIC_PROP_OP:
				case CREX_PRE_INC_STATIC_PROP:
				case CREX_PRE_DEC_STATIC_PROP:
				case CREX_PRE_INC_OBJ:
				case CREX_PRE_DEC_OBJ:
				case CREX_PRE_INC:
				case CREX_PRE_DEC:
					SET_UNUSED(opline->result);
					return;
			}
		}

		crex_emit_op(NULL, CREX_FREE, op1, NULL);
	} else if (op1->op_type == IS_VAR) {
		crex_op *opline = &CG(active_op_array)->opcodes[CG(active_op_array)->last-1];
		while (opline->opcode == CREX_END_SILENCE ||
				opline->opcode == CREX_EXT_FCALL_END ||
				opline->opcode == CREX_OP_DATA) {
			opline--;
		}
		if (opline->result_type == IS_VAR
			&& opline->result.var == op1->u.op.var) {
			if (opline->opcode == CREX_FETCH_THIS) {
				opline->opcode = CREX_NOP;
			}
			SET_UNUSED(opline->result);
		} else {
			while (opline >= CG(active_op_array)->opcodes) {
				if ((opline->opcode == CREX_FETCH_LIST_R ||
				     opline->opcode == CREX_FETCH_LIST_W) &&
				    opline->op1_type == IS_VAR &&
				    opline->op1.var == op1->u.op.var) {
					crex_emit_op(NULL, CREX_FREE, op1, NULL);
					return;
				}
				if (opline->result_type == IS_VAR
					&& opline->result.var == op1->u.op.var) {
					if (opline->opcode == CREX_NEW) {
						crex_emit_op(NULL, CREX_FREE, op1, NULL);
					}
					break;
				}
				opline--;
			}
		}
	} else if (op1->op_type == IS_CONST) {
		/* Destroy value without using GC: When opcache moves arrays into SHM it will
		 * free the crex_array structure, so references to it from outside the op array
		 * become invalid. GC would cause such a reference in the root buffer. */
		zval_ptr_dtor_nogc(&op1->u.constant);
	}
}
/* }}} */


static char *crex_modifier_token_to_string(uint32_t token)
{
	switch (token) {
		case T_PUBLIC:
			return "public";
		case T_PROTECTED:
			return "protected";
		case T_PRIVATE:
			return "private";
		case T_STATIC:
			return "static";
		case T_FINAL:
			return "final";
		case T_READONLY:
			return "readonly";
		case T_ABSTRACT:
			return "abstract";
		EMPTY_SWITCH_DEFAULT_CASE()
	}
}

uint32_t crex_modifier_token_to_flag(crex_modifier_target target, uint32_t token)
{
	switch (token) {
		case T_PUBLIC:
			return CREX_ACC_PUBLIC;
		case T_PROTECTED:
			return CREX_ACC_PROTECTED;
		case T_PRIVATE:
			return CREX_ACC_PRIVATE;
		case T_READONLY:
			if (target == CREX_MODIFIER_TARGET_PROPERTY || target == CREX_MODIFIER_TARGET_CPP) {
				return CREX_ACC_READONLY;
			}
			break;
		case T_ABSTRACT:
			if (target == CREX_MODIFIER_TARGET_METHOD) {
				return CREX_ACC_ABSTRACT;
			}
			break;
		case T_FINAL:
			if (target == CREX_MODIFIER_TARGET_METHOD || target == CREX_MODIFIER_TARGET_CONSTANT) {
				return CREX_ACC_FINAL;
			}
			break;
		case T_STATIC:
			if (target == CREX_MODIFIER_TARGET_PROPERTY || target == CREX_MODIFIER_TARGET_METHOD) {
				return CREX_ACC_STATIC;
			}
			break;
	}

	char *member;
	if (target == CREX_MODIFIER_TARGET_PROPERTY) {
		member = "property";
	} else if (target == CREX_MODIFIER_TARGET_METHOD) {
		member = "method";
	} else if (target == CREX_MODIFIER_TARGET_CONSTANT) {
		member = "class constant";
	} else if (target == CREX_MODIFIER_TARGET_CPP) {
		member = "parameter";
	} else {
		CREX_UNREACHABLE();
	}

	crex_throw_exception_ex(crex_ce_compile_error, 0,
		"Cannot use the %s modifier on a %s", crex_modifier_token_to_string(token), member);
	return 0;
}

uint32_t crex_modifier_list_to_flags(crex_modifier_target target, crex_ast *modifiers)
{
	uint32_t flags = 0;
	crex_ast_list *modifier_list = crex_ast_get_list(modifiers);

	for (uint32_t i = 0; i < modifier_list->children; i++) {
		uint32_t new_flag = crex_modifier_token_to_flag(target, (uint32_t) C_LVAL_P(crex_ast_get_zval(modifier_list->child[i])));
		if (!new_flag) {
			return 0;
		}
		flags = crex_add_member_modifier(flags, new_flag, target);
		if (!flags) {
			return 0;
		}
	}

	return flags;
}

uint32_t crex_add_class_modifier(uint32_t flags, uint32_t new_flag) /* {{{ */
{
	uint32_t new_flags = flags | new_flag;
	if ((flags & CREX_ACC_EXPLICIT_ABSTRACT_CLASS) && (new_flag & CREX_ACC_EXPLICIT_ABSTRACT_CLASS)) {
		crex_throw_exception(crex_ce_compile_error,
			"Multiple abstract modifiers are not allowed", 0);
		return 0;
	}
	if ((flags & CREX_ACC_FINAL) && (new_flag & CREX_ACC_FINAL)) {
		crex_throw_exception(crex_ce_compile_error, "Multiple final modifiers are not allowed", 0);
		return 0;
	}
	if ((flags & CREX_ACC_READONLY_CLASS) && (new_flag & CREX_ACC_READONLY_CLASS)) {
		crex_throw_exception(crex_ce_compile_error, "Multiple readonly modifiers are not allowed", 0);
		return 0;
	}
	if ((new_flags & CREX_ACC_EXPLICIT_ABSTRACT_CLASS) && (new_flags & CREX_ACC_FINAL)) {
		crex_throw_exception(crex_ce_compile_error,
			"Cannot use the final modifier on an abstract class", 0);
		return 0;
	}
	return new_flags;
}
/* }}} */

uint32_t crex_add_anonymous_class_modifier(uint32_t flags, uint32_t new_flag)
{
	uint32_t new_flags = flags | new_flag;
	if (new_flag & CREX_ACC_EXPLICIT_ABSTRACT_CLASS) {
		crex_throw_exception(crex_ce_compile_error,
			"Cannot use the abstract modifier on an anonymous class", 0);
		return 0;
	}
	if (new_flag & CREX_ACC_FINAL) {
		crex_throw_exception(crex_ce_compile_error, "Cannot use the final modifier on an anonymous class", 0);
		return 0;
	}
	if ((flags & CREX_ACC_READONLY_CLASS) && (new_flag & CREX_ACC_READONLY_CLASS)) {
		crex_throw_exception(crex_ce_compile_error, "Multiple readonly modifiers are not allowed", 0);
		return 0;
	}
	return new_flags;
}

uint32_t crex_add_member_modifier(uint32_t flags, uint32_t new_flag, crex_modifier_target target) /* {{{ */
{
	uint32_t new_flags = flags | new_flag;
	if ((flags & CREX_ACC_PPP_MASK) && (new_flag & CREX_ACC_PPP_MASK)) {
		crex_throw_exception(crex_ce_compile_error,
			"Multiple access type modifiers are not allowed", 0);
		return 0;
	}
	if ((flags & CREX_ACC_ABSTRACT) && (new_flag & CREX_ACC_ABSTRACT)) {
		crex_throw_exception(crex_ce_compile_error, "Multiple abstract modifiers are not allowed", 0);
		return 0;
	}
	if ((flags & CREX_ACC_STATIC) && (new_flag & CREX_ACC_STATIC)) {
		crex_throw_exception(crex_ce_compile_error, "Multiple static modifiers are not allowed", 0);
		return 0;
	}
	if ((flags & CREX_ACC_FINAL) && (new_flag & CREX_ACC_FINAL)) {
		crex_throw_exception(crex_ce_compile_error, "Multiple final modifiers are not allowed", 0);
		return 0;
	}
	if ((flags & CREX_ACC_READONLY) && (new_flag & CREX_ACC_READONLY)) {
		crex_throw_exception(crex_ce_compile_error,
			"Multiple readonly modifiers are not allowed", 0);
		return 0;
	}
	if (target == CREX_MODIFIER_TARGET_METHOD && (new_flags & CREX_ACC_ABSTRACT) && (new_flags & CREX_ACC_FINAL)) {
		crex_throw_exception(crex_ce_compile_error,
			"Cannot use the final modifier on an abstract method", 0);
		return 0;
	}
	return new_flags;
}
/* }}} */

CREX_API crex_string *crex_create_member_string(crex_string *class_name, crex_string *member_name) {
	return crex_string_concat3(
		ZSTR_VAL(class_name), ZSTR_LEN(class_name),
		"::", sizeof("::") - 1,
		ZSTR_VAL(member_name), ZSTR_LEN(member_name));
}

static crex_string *crex_concat_names(char *name1, size_t name1_len, char *name2, size_t name2_len) {
	return crex_string_concat3(name1, name1_len, "\\", 1, name2, name2_len);
}

static crex_string *crex_prefix_with_ns(crex_string *name) {
	if (FC(current_namespace)) {
		crex_string *ns = FC(current_namespace);
		return crex_concat_names(ZSTR_VAL(ns), ZSTR_LEN(ns), ZSTR_VAL(name), ZSTR_LEN(name));
	} else {
		return crex_string_copy(name);
	}
}

static crex_string *crex_resolve_non_class_name(
	crex_string *name, uint32_t type, bool *is_fully_qualified,
	bool case_sensitive, HashTable *current_import_sub
) {
	char *compound;
	*is_fully_qualified = 0;

	if (ZSTR_VAL(name)[0] == '\\') {
		/* Remove \ prefix (only relevant if this is a string rather than a label) */
		*is_fully_qualified = 1;
		return crex_string_init(ZSTR_VAL(name) + 1, ZSTR_LEN(name) - 1, 0);
	}

	if (type == CREX_NAME_FQ) {
		*is_fully_qualified = 1;
		return crex_string_copy(name);
	}

	if (type == CREX_NAME_RELATIVE) {
		*is_fully_qualified = 1;
		return crex_prefix_with_ns(name);
	}

	if (current_import_sub) {
		/* If an unqualified name is a function/const alias, replace it. */
		crex_string *import_name;
		if (case_sensitive) {
			import_name = crex_hash_find_ptr(current_import_sub, name);
		} else {
			import_name = crex_hash_find_ptr_lc(current_import_sub, name);
		}

		if (import_name) {
			*is_fully_qualified = 1;
			return crex_string_copy(import_name);
		}
	}

	compound = memchr(ZSTR_VAL(name), '\\', ZSTR_LEN(name));
	if (compound) {
		*is_fully_qualified = 1;
	}

	if (compound && FC(imports)) {
		/* If the first part of a qualified name is an alias, substitute it. */
		size_t len = compound - ZSTR_VAL(name);
		crex_string *import_name = crex_hash_str_find_ptr_lc(FC(imports), ZSTR_VAL(name), len);

		if (import_name) {
			return crex_concat_names(
				ZSTR_VAL(import_name), ZSTR_LEN(import_name), ZSTR_VAL(name) + len + 1, ZSTR_LEN(name) - len - 1);
		}
	}

	return crex_prefix_with_ns(name);
}
/* }}} */

static crex_string *crex_resolve_function_name(crex_string *name, uint32_t type, bool *is_fully_qualified)
{
	return crex_resolve_non_class_name(
		name, type, is_fully_qualified, 0, FC(imports_function));
}

static crex_string *crex_resolve_const_name(crex_string *name, uint32_t type, bool *is_fully_qualified)
{
	return crex_resolve_non_class_name(
		name, type, is_fully_qualified, 1, FC(imports_const));
}

static crex_string *crex_resolve_class_name(crex_string *name, uint32_t type) /* {{{ */
{
	char *compound;

	if (CREX_FETCH_CLASS_DEFAULT != crex_get_class_fetch_type(name)) {
		if (type == CREX_NAME_FQ) {
			crex_error_noreturn(E_COMPILE_ERROR,
				"'\\%s' is an invalid class name", ZSTR_VAL(name));
		}
		if (type == CREX_NAME_RELATIVE) {
			crex_error_noreturn(E_COMPILE_ERROR,
				"'namespace\\%s' is an invalid class name", ZSTR_VAL(name));
		}
		CREX_ASSERT(type == CREX_NAME_NOT_FQ);
		return crex_string_copy(name);
	}

	if (type == CREX_NAME_RELATIVE) {
		return crex_prefix_with_ns(name);
	}

	if (type == CREX_NAME_FQ) {
		if (ZSTR_VAL(name)[0] == '\\') {
			/* Remove \ prefix (only relevant if this is a string rather than a label) */
			name = crex_string_init(ZSTR_VAL(name) + 1, ZSTR_LEN(name) - 1, 0);
			if (CREX_FETCH_CLASS_DEFAULT != crex_get_class_fetch_type(name)) {
				crex_error_noreturn(E_COMPILE_ERROR,
					"'\\%s' is an invalid class name", ZSTR_VAL(name));
			}
			return name;
		}

		return crex_string_copy(name);
	}

	if (FC(imports)) {
		compound = memchr(ZSTR_VAL(name), '\\', ZSTR_LEN(name));
		if (compound) {
			/* If the first part of a qualified name is an alias, substitute it. */
			size_t len = compound - ZSTR_VAL(name);
			crex_string *import_name =
				crex_hash_str_find_ptr_lc(FC(imports), ZSTR_VAL(name), len);

			if (import_name) {
				return crex_concat_names(
					ZSTR_VAL(import_name), ZSTR_LEN(import_name), ZSTR_VAL(name) + len + 1, ZSTR_LEN(name) - len - 1);
			}
		} else {
			/* If an unqualified name is an alias, replace it. */
			crex_string *import_name
				= crex_hash_find_ptr_lc(FC(imports), name);

			if (import_name) {
				return crex_string_copy(import_name);
			}
		}
	}

	/* If not fully qualified and not an alias, prepend the current namespace */
	return crex_prefix_with_ns(name);
}
/* }}} */

crex_string *crex_resolve_class_name_ast(crex_ast *ast) /* {{{ */
{
	zval *class_name = crex_ast_get_zval(ast);
	if (C_TYPE_P(class_name) != IS_STRING) {
		crex_error_noreturn(E_COMPILE_ERROR, "Illegal class name");
	}
	return crex_resolve_class_name(C_STR_P(class_name), ast->attr);
}
/* }}} */

static void label_ptr_dtor(zval *zv) /* {{{ */
{
	efree_size(C_PTR_P(zv), sizeof(crex_label));
}
/* }}} */

static void str_dtor(zval *zv)  /* {{{ */ {
	crex_string_release_ex(C_STR_P(zv), 0);
}
/* }}} */

static bool crex_is_call(crex_ast *ast);

static uint32_t crex_add_try_element(uint32_t try_op) /* {{{ */
{
	crex_op_array *op_array = CG(active_op_array);
	uint32_t try_catch_offset = op_array->last_try_catch++;
	crex_try_catch_element *elem;

	op_array->try_catch_array = safe_erealloc(
		op_array->try_catch_array, sizeof(crex_try_catch_element), op_array->last_try_catch, 0);

	elem = &op_array->try_catch_array[try_catch_offset];
	elem->try_op = try_op;
	elem->catch_op = 0;
	elem->finally_op = 0;
	elem->finally_end = 0;

	return try_catch_offset;
}
/* }}} */

CREX_API void function_add_ref(crex_function *function) /* {{{ */
{
	if (function->type == CREX_USER_FUNCTION) {
		crex_op_array *op_array = &function->op_array;
		if (op_array->refcount) {
			(*op_array->refcount)++;
		}

		CREX_MAP_PTR_INIT(op_array->run_time_cache, NULL);
		CREX_MAP_PTR_INIT(op_array->static_variables_ptr, NULL);
	}

	if (function->common.function_name) {
		crex_string_addref(function->common.function_name);
	}
}
/* }}} */

static crex_never_inline CREX_COLD CREX_NORETURN void do_bind_function_error(crex_string *lcname, crex_op_array *op_array, bool compile_time) /* {{{ */
{
	zval *zv = crex_hash_find_known_hash(compile_time ? CG(function_table) : EG(function_table), lcname);
	int error_level = compile_time ? E_COMPILE_ERROR : E_ERROR;
	crex_function *old_function;

	CREX_ASSERT(zv != NULL);
	old_function = (crex_function*)C_PTR_P(zv);
	if (old_function->type == CREX_USER_FUNCTION
		&& old_function->op_array.last > 0) {
		crex_error_noreturn(error_level, "Cannot redeclare %s() (previously declared in %s:%d)",
					op_array ? ZSTR_VAL(op_array->function_name) : ZSTR_VAL(old_function->common.function_name),
					ZSTR_VAL(old_function->op_array.filename),
					old_function->op_array.opcodes[0].lineno);
	} else {
		crex_error_noreturn(error_level, "Cannot redeclare %s()",
			op_array ? ZSTR_VAL(op_array->function_name) : ZSTR_VAL(old_function->common.function_name));
	}
}

CREX_API crex_result do_bind_function(crex_function *func, zval *lcname) /* {{{ */
{
	crex_function *added_func = crex_hash_add_ptr(EG(function_table), C_STR_P(lcname), func);
	if (UNEXPECTED(!added_func)) {
		do_bind_function_error(C_STR_P(lcname), &func->op_array, 0);
		return FAILURE;
	}

	if (func->op_array.refcount) {
		++*func->op_array.refcount;
	}
	if (func->common.function_name) {
		crex_string_addref(func->common.function_name);
	}
	crex_observer_function_declared_notify(&func->op_array, C_STR_P(lcname));
	return SUCCESS;
}
/* }}} */

CREX_API crex_class_entry *crex_bind_class_in_slot(
		zval *class_table_slot, zval *lcname, crex_string *lc_parent_name)
{
	crex_class_entry *ce = C_PTR_P(class_table_slot);
	bool is_preloaded =
		(ce->ce_flags & CREX_ACC_PRELOADED) && !(CG(compiler_options) & CREX_COMPILE_PRELOAD);
	bool success;
	if (EXPECTED(!is_preloaded)) {
		success = crex_hash_set_bucket_key(EG(class_table), (Bucket*) class_table_slot, C_STR_P(lcname)) != NULL;
	} else {
		/* If preloading is used, don't replace the existing bucket, add a new one. */
		success = crex_hash_add_ptr(EG(class_table), C_STR_P(lcname), ce) != NULL;
	}
	if (UNEXPECTED(!success)) {
		crex_error_noreturn(E_COMPILE_ERROR, "Cannot declare %s %s, because the name is already in use", crex_get_object_type(ce), ZSTR_VAL(ce->name));
		return NULL;
	}

	if (ce->ce_flags & CREX_ACC_LINKED) {
		crex_observer_class_linked_notify(ce, C_STR_P(lcname));
		return ce;
	}

	ce = crex_do_link_class(ce, lc_parent_name, C_STR_P(lcname));
	if (ce) {
		CREX_ASSERT(!EG(exception));
		crex_observer_class_linked_notify(ce, C_STR_P(lcname));
		return ce;
	}

	if (!is_preloaded) {
		/* Reload bucket pointer, the hash table may have been reallocated */
		zval *zv = crex_hash_find(EG(class_table), C_STR_P(lcname));
		crex_hash_set_bucket_key(EG(class_table), (Bucket *) zv, C_STR_P(lcname + 1));
	} else {
		crex_hash_del(EG(class_table), C_STR_P(lcname));
	}
	return NULL;
}

CREX_API crex_result do_bind_class(zval *lcname, crex_string *lc_parent_name) /* {{{ */
{
	crex_class_entry *ce;
	zval *rtd_key, *zv;

	rtd_key = lcname + 1;

	zv = crex_hash_find_known_hash(EG(class_table), C_STR_P(rtd_key));

	if (UNEXPECTED(!zv)) {
		ce = crex_hash_find_ptr(EG(class_table), C_STR_P(lcname));
		CREX_ASSERT(ce);
		crex_error_noreturn(E_COMPILE_ERROR, "Cannot declare %s %s, because the name is already in use", crex_get_object_type(ce), ZSTR_VAL(ce->name));
		return FAILURE;
	}

	/* Register the derived class */
	return crex_bind_class_in_slot(zv, lcname, lc_parent_name) ? SUCCESS : FAILURE;
}
/* }}} */

static crex_string *add_type_string(crex_string *type, crex_string *new_type, bool is_intersection) {
	crex_string *result;
	if (type == NULL) {
		return crex_string_copy(new_type);
	}

	if (is_intersection) {
		result = crex_string_concat3(ZSTR_VAL(type), ZSTR_LEN(type),
			"&", 1, ZSTR_VAL(new_type), ZSTR_LEN(new_type));
		crex_string_release(type);
	} else {
		result = crex_string_concat3(
			ZSTR_VAL(type), ZSTR_LEN(type), "|", 1, ZSTR_VAL(new_type), ZSTR_LEN(new_type));
		crex_string_release(type);
	}
	return result;
}

static crex_string *resolve_class_name(crex_string *name, crex_class_entry *scope) {
	if (scope) {
		if (crex_string_equals_literal_ci(name, "self")) {
			name = scope->name;
		} else if (crex_string_equals_literal_ci(name, "parent") && scope->parent) {
			name = scope->parent->name;
		}
	}

	/* The resolved name for anonymous classes contains null bytes. Cut off everything after the
	 * null byte here, to avoid larger parts of the type being omitted by printing code later. */
	size_t len = strlen(ZSTR_VAL(name));
	if (len != ZSTR_LEN(name)) {
		CREX_ASSERT(scope && "This should only happen with resolved types");
		return crex_string_init(ZSTR_VAL(name), len, 0);
	}
	return crex_string_copy(name);
}

static crex_string *add_intersection_type(crex_string *str,
	crex_type_list *intersection_type_list, crex_class_entry *scope,
	bool is_bracketed)
{
	crex_type *single_type;
	crex_string *intersection_str = NULL;

	CREX_TYPE_LIST_FOREACH(intersection_type_list, single_type) {
		CREX_ASSERT(!CREX_TYPE_HAS_LIST(*single_type));
		CREX_ASSERT(CREX_TYPE_HAS_NAME(*single_type));
		crex_string *name = CREX_TYPE_NAME(*single_type);
		crex_string *resolved = resolve_class_name(name, scope);
		intersection_str = add_type_string(intersection_str, resolved, /* is_intersection */ true);
		crex_string_release(resolved);
	} CREX_TYPE_LIST_FOREACH_END();

	CREX_ASSERT(intersection_str);

	if (is_bracketed) {
		crex_string *result = crex_string_concat3("(", 1, ZSTR_VAL(intersection_str), ZSTR_LEN(intersection_str), ")", 1);
		crex_string_release(intersection_str);
		intersection_str = result;
	}
	str = add_type_string(str, intersection_str, /* is_intersection */ false);
	crex_string_release(intersection_str);
	return str;
}

crex_string *crex_type_to_string_resolved(crex_type type, crex_class_entry *scope) {
	crex_string *str = NULL;

	/* Pure intersection type */
	if (CREX_TYPE_IS_INTERSECTION(type)) {
		CREX_ASSERT(!CREX_TYPE_IS_UNION(type));
		str = add_intersection_type(str, CREX_TYPE_LIST(type), scope, /* is_bracketed */ false);
	} else if (CREX_TYPE_HAS_LIST(type)) {
		/* A union type might not be a list */
		crex_type *list_type;
		CREX_TYPE_LIST_FOREACH(CREX_TYPE_LIST(type), list_type) {
			if (CREX_TYPE_IS_INTERSECTION(*list_type)) {
				str = add_intersection_type(str, CREX_TYPE_LIST(*list_type), scope, /* is_bracketed */ true);
				continue;
			}
			CREX_ASSERT(!CREX_TYPE_HAS_LIST(*list_type));
			CREX_ASSERT(CREX_TYPE_HAS_NAME(*list_type));
			crex_string *name = CREX_TYPE_NAME(*list_type);
			crex_string *resolved = resolve_class_name(name, scope);
			str = add_type_string(str, resolved, /* is_intersection */ false);
			crex_string_release(resolved);
		} CREX_TYPE_LIST_FOREACH_END();
	} else if (CREX_TYPE_HAS_NAME(type)) {
		str = resolve_class_name(CREX_TYPE_NAME(type), scope);
	}

	uint32_t type_mask = CREX_TYPE_PURE_MASK(type);

	if (type_mask == MAY_BE_ANY) {
		str = add_type_string(str, ZSTR_KNOWN(CREX_STR_MIXED), /* is_intersection */ false);

		return str;
	}
	if (type_mask & MAY_BE_STATIC) {
		crex_string *name = ZSTR_KNOWN(CREX_STR_STATIC);
		// During compilation of eval'd code the called scope refers to the scope calling the eval
		if (scope && !crex_is_compiling()) {
			crex_class_entry *called_scope = crex_get_called_scope(EG(current_execute_data));
			if (called_scope) {
				name = called_scope->name;
			}
		}
		str = add_type_string(str, name, /* is_intersection */ false);
	}
	if (type_mask & MAY_BE_CALLABLE) {
		str = add_type_string(str, ZSTR_KNOWN(CREX_STR_CALLABLE), /* is_intersection */ false);
	}
	if (type_mask & MAY_BE_OBJECT) {
		str = add_type_string(str, ZSTR_KNOWN(CREX_STR_OBJECT), /* is_intersection */ false);
	}
	if (type_mask & MAY_BE_ARRAY) {
		str = add_type_string(str, ZSTR_KNOWN(CREX_STR_ARRAY), /* is_intersection */ false);
	}
	if (type_mask & MAY_BE_STRING) {
		str = add_type_string(str, ZSTR_KNOWN(CREX_STR_STRING), /* is_intersection */ false);
	}
	if (type_mask & MAY_BE_LONG) {
		str = add_type_string(str, ZSTR_KNOWN(CREX_STR_INT), /* is_intersection */ false);
	}
	if (type_mask & MAY_BE_DOUBLE) {
		str = add_type_string(str, ZSTR_KNOWN(CREX_STR_FLOAT), /* is_intersection */ false);
	}
	if ((type_mask & MAY_BE_BOOL) == MAY_BE_BOOL) {
		str = add_type_string(str, ZSTR_KNOWN(CREX_STR_BOOL), /* is_intersection */ false);
	} else if (type_mask & MAY_BE_FALSE) {
		str = add_type_string(str, ZSTR_KNOWN(CREX_STR_FALSE), /* is_intersection */ false);
	} else if (type_mask & MAY_BE_TRUE) {
		str = add_type_string(str, ZSTR_KNOWN(CREX_STR_TRUE), /* is_intersection */ false);
	}
	if (type_mask & MAY_BE_VOID) {
		str = add_type_string(str, ZSTR_KNOWN(CREX_STR_VOID), /* is_intersection */ false);
	}
	if (type_mask & MAY_BE_NEVER) {
		str = add_type_string(str, ZSTR_KNOWN(CREX_STR_NEVER), /* is_intersection */ false);
	}

	if (type_mask & MAY_BE_NULL) {
		bool is_union = !str || memchr(ZSTR_VAL(str), '|', ZSTR_LEN(str)) != NULL;
		bool has_intersection = !str || memchr(ZSTR_VAL(str), '&', ZSTR_LEN(str)) != NULL;
		if (!is_union && !has_intersection) {
			crex_string *nullable_str = crex_string_concat2("?", 1, ZSTR_VAL(str), ZSTR_LEN(str));
			crex_string_release(str);
			return nullable_str;
		}

		str = add_type_string(str, ZSTR_KNOWN(CREX_STR_NULL_LOWERCASE), /* is_intersection */ false);
	}
	return str;
}

CREX_API crex_string *crex_type_to_string(crex_type type) {
	return crex_type_to_string_resolved(type, NULL);
}

static bool is_generator_compatible_class_type(crex_string *name) {
	return crex_string_equals_ci(name, ZSTR_KNOWN(CREX_STR_TRAVERSABLE))
		|| crex_string_equals_literal_ci(name, "Iterator")
		|| crex_string_equals_literal_ci(name, "Generator");
}

static void crex_mark_function_as_generator(void) /* {{{ */
{
	if (!CG(active_op_array)->function_name) {
		crex_error_noreturn(E_COMPILE_ERROR,
			"The \"yield\" expression can only be used inside a function");
	}

	if (CG(active_op_array)->fn_flags & CREX_ACC_HAS_RETURN_TYPE) {
		crex_type return_type = CG(active_op_array)->arg_info[-1].type;
		bool valid_type = (CREX_TYPE_FULL_MASK(return_type) & MAY_BE_OBJECT) != 0;
		if (!valid_type) {
			crex_type *single_type;
			CREX_TYPE_FOREACH(return_type, single_type) {
				if (CREX_TYPE_HAS_NAME(*single_type)
						&& is_generator_compatible_class_type(CREX_TYPE_NAME(*single_type))) {
					valid_type = 1;
					break;
				}
			} CREX_TYPE_FOREACH_END();
		}

		if (!valid_type) {
			crex_string *str = crex_type_to_string(return_type);
			crex_error_noreturn(E_COMPILE_ERROR,
				"Generator return type must be a supertype of Generator, %s given",
				ZSTR_VAL(str));
		}
	}

	CG(active_op_array)->fn_flags |= CREX_ACC_GENERATOR;
}
/* }}} */

CREX_API crex_string *crex_mangle_property_name(const char *src1, size_t src1_length, const char *src2, size_t src2_length, bool internal) /* {{{ */
{
	size_t prop_name_length = 1 + src1_length + 1 + src2_length;
	crex_string *prop_name = crex_string_alloc(prop_name_length, internal);

	ZSTR_VAL(prop_name)[0] = '\0';
	memcpy(ZSTR_VAL(prop_name) + 1, src1, src1_length+1);
	memcpy(ZSTR_VAL(prop_name) + 1 + src1_length + 1, src2, src2_length+1);
	return prop_name;
}
/* }}} */

CREX_API crex_result crex_unmangle_property_name_ex(const crex_string *name, const char **class_name, const char **prop_name, size_t *prop_len) /* {{{ */
{
	size_t class_name_len;
	size_t anonclass_src_len;

	*class_name = NULL;

	if (!ZSTR_LEN(name) || ZSTR_VAL(name)[0] != '\0') {
		*prop_name = ZSTR_VAL(name);
		if (prop_len) {
			*prop_len = ZSTR_LEN(name);
		}
		return SUCCESS;
	}
	if (ZSTR_LEN(name) < 3 || ZSTR_VAL(name)[1] == '\0') {
		crex_error(E_NOTICE, "Illegal member variable name");
		*prop_name = ZSTR_VAL(name);
		if (prop_len) {
			*prop_len = ZSTR_LEN(name);
		}
		return FAILURE;
	}

	class_name_len = crex_strnlen(ZSTR_VAL(name) + 1, ZSTR_LEN(name) - 2);
	if (class_name_len >= ZSTR_LEN(name) - 2 || ZSTR_VAL(name)[class_name_len + 1] != '\0') {
		crex_error(E_NOTICE, "Corrupt member variable name");
		*prop_name = ZSTR_VAL(name);
		if (prop_len) {
			*prop_len = ZSTR_LEN(name);
		}
		return FAILURE;
	}

	*class_name = ZSTR_VAL(name) + 1;
	anonclass_src_len = crex_strnlen(*class_name + class_name_len + 1, ZSTR_LEN(name) - class_name_len - 2);
	if (class_name_len + anonclass_src_len + 2 != ZSTR_LEN(name)) {
		class_name_len += anonclass_src_len + 1;
	}
	*prop_name = ZSTR_VAL(name) + class_name_len + 2;
	if (prop_len) {
		*prop_len = ZSTR_LEN(name) - class_name_len - 2;
	}
	return SUCCESS;
}
/* }}} */

static bool array_is_const_ex(crex_array *array, uint32_t *max_checks)
{
	if (crex_hash_num_elements(array) > *max_checks) {
		return false;
	}
	*max_checks -= crex_hash_num_elements(array);

	zval *element;
	CREX_HASH_FOREACH_VAL(array, element) {
		if (C_TYPE_P(element) < IS_ARRAY) {
			continue;
		} else if (C_TYPE_P(element) == IS_ARRAY) {
			if (!array_is_const_ex(array, max_checks)) {
				return false;
			}
		} else {
			return false;
		}
	} CREX_HASH_FOREACH_END();

	return true;
}

static bool array_is_const(crex_array *array)
{
	uint32_t max_checks = 50;
	return array_is_const_ex(array, &max_checks);
}

static bool can_ct_eval_const(crex_constant *c) {
	if (CREX_CONSTANT_FLAGS(c) & CONST_DEPRECATED) {
		return 0;
	}
	if ((CREX_CONSTANT_FLAGS(c) & CONST_PERSISTENT)
			&& !(CG(compiler_options) & CREX_COMPILE_NO_PERSISTENT_CONSTANT_SUBSTITUTION)
			&& !((CREX_CONSTANT_FLAGS(c) & CONST_NO_FILE_CACHE)
				&& (CG(compiler_options) & CREX_COMPILE_WITH_FILE_CACHE))) {
		return 1;
	}
	if (C_TYPE(c->value) < IS_ARRAY
			&& !(CG(compiler_options) & CREX_COMPILE_NO_CONSTANT_SUBSTITUTION)) {
		return 1;
	} else if (C_TYPE(c->value) == IS_ARRAY
			&& !(CG(compiler_options) & CREX_COMPILE_NO_CONSTANT_SUBSTITUTION)
			&& array_is_const(C_ARR(c->value))) {
		return 1;
	}
	return 0;
}

static bool crex_try_ct_eval_const(zval *zv, crex_string *name, bool is_fully_qualified) /* {{{ */
{
	/* Substitute true, false and null (including unqualified usage in namespaces)
	 * before looking up the possibly namespaced name. */
	const char *lookup_name = ZSTR_VAL(name);
	size_t lookup_len = ZSTR_LEN(name);

	if (!is_fully_qualified) {
		crex_get_unqualified_name(name, &lookup_name, &lookup_len);
	}

	crex_constant *c;
	if ((c = crex_get_special_const(lookup_name, lookup_len))) {
		ZVAL_COPY_VALUE(zv, &c->value);
		return 1;
	}
	c = crex_hash_find_ptr(EG(crex_constants), name);
	if (c && can_ct_eval_const(c)) {
		ZVAL_COPY_OR_DUP(zv, &c->value);
		return 1;
	}
	return 0;
}
/* }}} */

static inline bool crex_is_scope_known(void) /* {{{ */
{
	if (!CG(active_op_array)) {
		/* This can only happen when evaluating a default value string. */
		return 0;
	}

	if (CG(active_op_array)->fn_flags & CREX_ACC_CLOSURE) {
		/* Closures can be rebound to a different scope */
		return 0;
	}

	if (!CG(active_class_entry)) {
		/* The scope is known if we're in a free function (no scope), but not if we're in
		 * a file/eval (which inherits including/eval'ing scope). */
		return CG(active_op_array)->function_name != NULL;
	}

	/* For traits self etc refers to the using class, not the trait itself */
	return (CG(active_class_entry)->ce_flags & CREX_ACC_TRAIT) == 0;
}
/* }}} */

static inline bool class_name_refers_to_active_ce(crex_string *class_name, uint32_t fetch_type) /* {{{ */
{
	if (!CG(active_class_entry)) {
		return 0;
	}
	if (fetch_type == CREX_FETCH_CLASS_SELF && crex_is_scope_known()) {
		return 1;
	}
	return fetch_type == CREX_FETCH_CLASS_DEFAULT
		&& crex_string_equals_ci(class_name, CG(active_class_entry)->name);
}
/* }}} */

uint32_t crex_get_class_fetch_type(const crex_string *name) /* {{{ */
{
	if (crex_string_equals_literal_ci(name, "self")) {
		return CREX_FETCH_CLASS_SELF;
	} else if (crex_string_equals_literal_ci(name, "parent")) {
		return CREX_FETCH_CLASS_PARENT;
	} else if (crex_string_equals_ci(name, ZSTR_KNOWN(CREX_STR_STATIC))) {
		return CREX_FETCH_CLASS_STATIC;
	} else {
		return CREX_FETCH_CLASS_DEFAULT;
	}
}
/* }}} */

static uint32_t crex_get_class_fetch_type_ast(crex_ast *name_ast) /* {{{ */
{
	/* Fully qualified names are always default refs */
	if (name_ast->attr == CREX_NAME_FQ) {
		return CREX_FETCH_CLASS_DEFAULT;
	}

	return crex_get_class_fetch_type(crex_ast_get_str(name_ast));
}
/* }}} */

static crex_string *crex_resolve_const_class_name_reference(crex_ast *ast, const char *type)
{
	crex_string *class_name = crex_ast_get_str(ast);
	if (CREX_FETCH_CLASS_DEFAULT != crex_get_class_fetch_type_ast(ast)) {
		crex_error_noreturn(E_COMPILE_ERROR,
			"Cannot use '%s' as %s, as it is reserved",
			ZSTR_VAL(class_name), type);
	}
	return crex_resolve_class_name(class_name, ast->attr);
}

static void crex_ensure_valid_class_fetch_type(uint32_t fetch_type) /* {{{ */
{
	if (fetch_type != CREX_FETCH_CLASS_DEFAULT && crex_is_scope_known()) {
		crex_class_entry *ce = CG(active_class_entry);
		if (!ce) {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot use \"%s\" when no class scope is active",
				fetch_type == CREX_FETCH_CLASS_SELF ? "self" :
				fetch_type == CREX_FETCH_CLASS_PARENT ? "parent" : "static");
		} else if (fetch_type == CREX_FETCH_CLASS_PARENT && !ce->parent_name) {
			crex_error_noreturn(E_COMPILE_ERROR,
				"Cannot use \"parent\" when current class scope has no parent");
		}
	}
}
/* }}} */

static bool crex_try_compile_const_expr_resolve_class_name(zval *zv, crex_ast *class_ast) /* {{{ */
{
	uint32_t fetch_type;
	zval *class_name;

	if (class_ast->kind != CREX_AST_ZVAL) {
		return 0;
	}

	class_name = crex_ast_get_zval(class_ast);

	if (C_TYPE_P(class_name) != IS_STRING) {
		crex_error_noreturn(E_COMPILE_ERROR, "Illegal class name");
	}

	fetch_type = crex_get_class_fetch_type(C_STR_P(class_name));
	crex_ensure_valid_class_fetch_type(fetch_type);

	switch (fetch_type) {
		case CREX_FETCH_CLASS_SELF:
			if (CG(active_class_entry) && crex_is_scope_known()) {
				ZVAL_STR_COPY(zv, CG(active_class_entry)->name);
				return 1;
			}
			return 0;
		case CREX_FETCH_CLASS_PARENT:
			if (CG(active_class_entry) && CG(active_class_entry)->parent_name
					&& crex_is_scope_known()) {
				ZVAL_STR_COPY(zv, CG(active_class_entry)->parent_name);
				return 1;
			}
			return 0;
		case CREX_FETCH_CLASS_STATIC:
			return 0;
		case CREX_FETCH_CLASS_DEFAULT:
			ZVAL_STR(zv, crex_resolve_class_name_ast(class_ast));
			return 1;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
}
/* }}} */

/* We don't use crex_verify_const_access because we need to deal with unlinked classes. */
static bool crex_verify_ct_const_access(crex_class_constant *c, crex_class_entry *scope)
{
	if (CREX_CLASS_CONST_FLAGS(c) & CREX_ACC_DEPRECATED) {
		return 0;
	} else if (c->ce->ce_flags & CREX_ACC_TRAIT) {
		/* This condition is only met on directly accessing trait constants,
		 * because the ce is replaced to the class entry of the composing class
		 * on binding. */
		return 0;
	} else if (CREX_CLASS_CONST_FLAGS(c) & CREX_ACC_PUBLIC) {
		return 1;
	} else if (CREX_CLASS_CONST_FLAGS(c) & CREX_ACC_PRIVATE) {
		return c->ce == scope;
	} else {
		crex_class_entry *ce = c->ce;
		while (1) {
			if (ce == scope) {
				return 1;
			}
			if (!ce->parent) {
				break;
			}
			if (ce->ce_flags & CREX_ACC_RESOLVED_PARENT) {
				ce = ce->parent;
			} else {
				ce = crex_hash_find_ptr_lc(CG(class_table), ce->parent_name);
				if (!ce) {
					break;
				}
			}
		}
		/* Reverse case cannot be true during compilation */
		return 0;
	}
}

static bool crex_try_ct_eval_class_const(zval *zv, crex_string *class_name, crex_string *name) /* {{{ */
{
	uint32_t fetch_type = crex_get_class_fetch_type(class_name);
	crex_class_constant *cc;
	zval *c;

	if (class_name_refers_to_active_ce(class_name, fetch_type)) {
		cc = crex_hash_find_ptr(&CG(active_class_entry)->constants_table, name);
	} else if (fetch_type == CREX_FETCH_CLASS_DEFAULT && !(CG(compiler_options) & CREX_COMPILE_NO_CONSTANT_SUBSTITUTION)) {
		crex_class_entry *ce = crex_hash_find_ptr_lc(CG(class_table), class_name);
		if (ce) {
			cc = crex_hash_find_ptr(&ce->constants_table, name);
		} else {
			return 0;
		}
	} else {
		return 0;
	}

	if (CG(compiler_options) & CREX_COMPILE_NO_PERSISTENT_CONSTANT_SUBSTITUTION) {
		return 0;
	}

	if (!cc || !crex_verify_ct_const_access(cc, CG(active_class_entry))) {
		return 0;
	}

	c = &cc->value;

	/* Substitute case-sensitive (or lowercase) persistent class constants */
	if (C_TYPE_P(c) < IS_ARRAY) {
		ZVAL_COPY_OR_DUP(zv, c);
		return 1;
	} else if (C_TYPE_P(c) == IS_ARRAY && array_is_const(C_ARR_P(c))) {
		ZVAL_COPY_OR_DUP(zv, c);
		return 1;
	}

	return 0;
}
/* }}} */

static void crex_add_to_list(void *result, void *item) /* {{{ */
{
	void** list = *(void**)result;
	size_t n = 0;

	if (list) {
		while (list[n]) {
			n++;
		}
	}

	list = erealloc(list, sizeof(void*) * (n+2));

	list[n]   = item;
	list[n+1] = NULL;

	*(void**)result = list;
}
/* }}} */

static void crex_do_extended_stmt(void) /* {{{ */
{
	crex_op *opline;

	if (!(CG(compiler_options) & CREX_COMPILE_EXTENDED_STMT)) {
		return;
	}

	opline = get_next_op();

	opline->opcode = CREX_EXT_STMT;
}
/* }}} */

static void crex_do_extended_fcall_begin(void) /* {{{ */
{
	crex_op *opline;

	if (!(CG(compiler_options) & CREX_COMPILE_EXTENDED_FCALL)) {
		return;
	}

	opline = get_next_op();

	opline->opcode = CREX_EXT_FCALL_BEGIN;
}
/* }}} */

static void crex_do_extended_fcall_end(void) /* {{{ */
{
	crex_op *opline;

	if (!(CG(compiler_options) & CREX_COMPILE_EXTENDED_FCALL)) {
		return;
	}

	opline = get_next_op();

	opline->opcode = CREX_EXT_FCALL_END;
}
/* }}} */

CREX_API bool crex_is_auto_global_str(const char *name, size_t len) /* {{{ */ {
	crex_auto_global *auto_global;

	if ((auto_global = crex_hash_str_find_ptr(CG(auto_globals), name, len)) != NULL) {
		if (auto_global->armed) {
			auto_global->armed = auto_global->auto_global_callback(auto_global->name);
		}
		return 1;
	}
	return 0;
}
/* }}} */

CREX_API bool crex_is_auto_global(crex_string *name) /* {{{ */
{
	crex_auto_global *auto_global;

	if ((auto_global = crex_hash_find_ptr(CG(auto_globals), name)) != NULL) {
		if (auto_global->armed) {
			auto_global->armed = auto_global->auto_global_callback(auto_global->name);
		}
		return 1;
	}
	return 0;
}
/* }}} */

CREX_API crex_result crex_register_auto_global(crex_string *name, bool jit, crex_auto_global_callback auto_global_callback) /* {{{ */
{
	crex_auto_global auto_global;
	crex_result retval;

	auto_global.name = name;
	auto_global.auto_global_callback = auto_global_callback;
	auto_global.jit = jit;

	retval = crex_hash_add_mem(CG(auto_globals), auto_global.name, &auto_global, sizeof(crex_auto_global)) != NULL ? SUCCESS : FAILURE;

	return retval;
}
/* }}} */

CREX_API void crex_activate_auto_globals(void) /* {{{ */
{
	crex_auto_global *auto_global;

	CREX_HASH_MAP_FOREACH_PTR(CG(auto_globals), auto_global) {
		if (auto_global->jit) {
			auto_global->armed = 1;
		} else if (auto_global->auto_global_callback) {
			auto_global->armed = auto_global->auto_global_callback(auto_global->name);
		} else {
			auto_global->armed = 0;
		}
	} CREX_HASH_FOREACH_END();
}
/* }}} */

int CREX_FASTCALL crexlex(crex_parser_stack_elem *elem) /* {{{ */
{
	zval zv;
	int ret;

	if (CG(increment_lineno)) {
		CG(crex_lineno)++;
		CG(increment_lineno) = 0;
	}

	ret = lex_scan(&zv, elem);
	CREX_ASSERT(!EG(exception) || ret == T_ERROR);
	return ret;

}
/* }}} */

CREX_API void crex_initialize_class_data(crex_class_entry *ce, bool nullify_handlers) /* {{{ */
{
	bool persistent_hashes = ce->type == CREX_INTERNAL_CLASS;

	ce->refcount = 1;
	ce->ce_flags = CREX_ACC_CONSTANTS_UPDATED;

	if (CG(compiler_options) & CREX_COMPILE_GUARDS) {
		ce->ce_flags |= CREX_ACC_USE_GUARDS;
	}

	ce->default_properties_table = NULL;
	ce->default_static_members_table = NULL;
	crex_hash_init(&ce->properties_info, 8, NULL, NULL, persistent_hashes);
	crex_hash_init(&ce->constants_table, 8, NULL, NULL, persistent_hashes);
	crex_hash_init(&ce->function_table, 8, NULL, CREX_FUNCTION_DTOR, persistent_hashes);

	if (ce->type == CREX_USER_CLASS) {
		ce->info.user.doc_comment = NULL;
	}
	CREX_MAP_PTR_INIT(ce->static_members_table, NULL);
	CREX_MAP_PTR_INIT(ce->mutable_data, NULL);

	ce->default_object_handlers = &std_object_handlers;
	ce->default_properties_count = 0;
	ce->default_static_members_count = 0;
	ce->properties_info_table = NULL;
	ce->attributes = NULL;
	ce->enum_backing_type = IS_UNDEF;
	ce->backed_enum_table = NULL;

	if (nullify_handlers) {
		ce->constructor = NULL;
		ce->destructor = NULL;
		ce->clone = NULL;
		ce->__get = NULL;
		ce->__set = NULL;
		ce->__unset = NULL;
		ce->__isset = NULL;
		ce->__call = NULL;
		ce->__callstatic = NULL;
		ce->__tostring = NULL;
		ce->__serialize = NULL;
		ce->__unserialize = NULL;
		ce->__debugInfo = NULL;
		ce->create_object = NULL;
		ce->get_iterator = NULL;
		ce->iterator_funcs_ptr = NULL;
		ce->arrayaccess_funcs_ptr = NULL;
		ce->get_static_method = NULL;
		ce->parent = NULL;
		ce->parent_name = NULL;
		ce->num_interfaces = 0;
		ce->interfaces = NULL;
		ce->num_traits = 0;
		ce->trait_names = NULL;
		ce->trait_aliases = NULL;
		ce->trait_precedences = NULL;
		ce->serialize = NULL;
		ce->unserialize = NULL;
		if (ce->type == CREX_INTERNAL_CLASS) {
			ce->info.internal.module = NULL;
			ce->info.internal.builtin_functions = NULL;
		}
	}
}
/* }}} */

CREX_API crex_string *crex_get_compiled_variable_name(const crex_op_array *op_array, uint32_t var) /* {{{ */
{
	return op_array->vars[EX_VAR_TO_NUM(var)];
}
/* }}} */

crex_ast *crex_ast_append_str(crex_ast *left_ast, crex_ast *right_ast) /* {{{ */
{
	zval *left_zv = crex_ast_get_zval(left_ast);
	crex_string *left = C_STR_P(left_zv);
	crex_string *right = crex_ast_get_str(right_ast);

	crex_string *result;
	size_t left_len = ZSTR_LEN(left);
	size_t len = left_len + ZSTR_LEN(right) + 1; /* left\right */

	result = crex_string_extend(left, len, 0);
	ZSTR_VAL(result)[left_len] = '\\';
	memcpy(&ZSTR_VAL(result)[left_len + 1], ZSTR_VAL(right), ZSTR_LEN(right));
	ZSTR_VAL(result)[len] = '\0';
	crex_string_release_ex(right, 0);

	ZVAL_STR(left_zv, result);
	return left_ast;
}
/* }}} */

crex_ast *crex_negate_num_string(crex_ast *ast) /* {{{ */
{
	zval *zv = crex_ast_get_zval(ast);
	if (C_TYPE_P(zv) == IS_LONG) {
		if (C_LVAL_P(zv) == 0) {
			ZVAL_NEW_STR(zv, ZSTR_INIT_LITERAL("-0", 0));
		} else {
			CREX_ASSERT(C_LVAL_P(zv) > 0);
			C_LVAL_P(zv) *= -1;
		}
	} else if (C_TYPE_P(zv) == IS_STRING) {
		size_t orig_len = C_STRLEN_P(zv);
		C_STR_P(zv) = crex_string_extend(C_STR_P(zv), orig_len + 1, 0);
		memmove(C_STRVAL_P(zv) + 1, C_STRVAL_P(zv), orig_len + 1);
		C_STRVAL_P(zv)[0] = '-';
	} else {
		CREX_UNREACHABLE();
	}
	return ast;
}
/* }}} */

static void crex_verify_namespace(void) /* {{{ */
{
	if (FC(has_bracketed_namespaces) && !FC(in_namespace)) {
		crex_error_noreturn(E_COMPILE_ERROR, "No code may exist outside of namespace {}");
	}
}
/* }}} */

/* {{{ crex_dirname
   Returns directory name component of path */
CREX_API size_t crex_dirname(char *path, size_t len)
{
	char *end = path + len - 1;
	unsigned int len_adjust = 0;

#ifdef CREX_WIN32
	/* Note that on Win32 CWD is per drive (heritage from CP/M).
	 * This means dirname("c:foo") maps to "c:." or "c:" - which means CWD on C: drive.
	 */
	if ((2 <= len) && isalpha((int)((unsigned char *)path)[0]) && (':' == path[1])) {
		/* Skip over the drive spec (if any) so as not to change */
		path += 2;
		len_adjust += 2;
		if (2 == len) {
			/* Return "c:" on Win32 for dirname("c:").
			 * It would be more consistent to return "c:."
			 * but that would require making the string *longer*.
			 */
			return len;
		}
	}
#endif

	if (len == 0) {
		/* Illegal use of this function */
		return 0;
	}

	/* Strip trailing slashes */
	while (end >= path && IS_SLASH_P(end)) {
		end--;
	}
	if (end < path) {
		/* The path only contained slashes */
		path[0] = DEFAULT_SLASH;
		path[1] = '\0';
		return 1 + len_adjust;
	}

	/* Strip filename */
	while (end >= path && !IS_SLASH_P(end)) {
		end--;
	}
	if (end < path) {
		/* No slash found, therefore return '.' */
		path[0] = '.';
		path[1] = '\0';
		return 1 + len_adjust;
	}

	/* Strip slashes which came before the file name */
	while (end >= path && IS_SLASH_P(end)) {
		end--;
	}
	if (end < path) {
		path[0] = DEFAULT_SLASH;
		path[1] = '\0';
		return 1 + len_adjust;
	}
	*(end+1) = '\0';

	return (size_t)(end + 1 - path) + len_adjust;
}
/* }}} */

static void crex_adjust_for_fetch_type(crex_op *opline, znode *result, uint32_t type) /* {{{ */
{
	uint_fast8_t factor = (opline->opcode == CREX_FETCH_STATIC_PROP_R) ? 1 : 3;

	switch (type) {
		case BP_VAR_R:
			opline->result_type = IS_TMP_VAR;
			result->op_type = IS_TMP_VAR;
			return;
		case BP_VAR_W:
			opline->opcode += 1 * factor;
			return;
		case BP_VAR_RW:
			opline->opcode += 2 * factor;
			return;
		case BP_VAR_IS:
			opline->result_type = IS_TMP_VAR;
			result->op_type = IS_TMP_VAR;
			opline->opcode += 3 * factor;
			return;
		case BP_VAR_FUNC_ARG:
			opline->opcode += 4 * factor;
			return;
		case BP_VAR_UNSET:
			opline->opcode += 5 * factor;
			return;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
}
/* }}} */

static inline void crex_make_var_result(znode *result, crex_op *opline) /* {{{ */
{
	opline->result_type = IS_VAR;
	opline->result.var = get_temporary_variable();
	GET_NODE(result, opline->result);
}
/* }}} */

static inline void crex_make_tmp_result(znode *result, crex_op *opline) /* {{{ */
{
	opline->result_type = IS_TMP_VAR;
	opline->result.var = get_temporary_variable();
	GET_NODE(result, opline->result);
}
/* }}} */

static crex_op *crex_emit_op(znode *result, uint8_t opcode, znode *op1, znode *op2) /* {{{ */
{
	crex_op *opline = get_next_op();
	opline->opcode = opcode;

	if (op1 != NULL) {
		SET_NODE(opline->op1, op1);
	}

	if (op2 != NULL) {
		SET_NODE(opline->op2, op2);
	}

	if (result) {
		crex_make_var_result(result, opline);
	}
	return opline;
}
/* }}} */

static crex_op *crex_emit_op_tmp(znode *result, uint8_t opcode, znode *op1, znode *op2) /* {{{ */
{
	crex_op *opline = get_next_op();
	opline->opcode = opcode;

	if (op1 != NULL) {
		SET_NODE(opline->op1, op1);
	}

	if (op2 != NULL) {
		SET_NODE(opline->op2, op2);
	}

	if (result) {
		crex_make_tmp_result(result, opline);
	}

	return opline;
}
/* }}} */

static void crex_emit_tick(void) /* {{{ */
{
	crex_op *opline;

	/* This prevents a double TICK generated by the parser statement of "declare()" */
	if (CG(active_op_array)->last && CG(active_op_array)->opcodes[CG(active_op_array)->last - 1].opcode == CREX_TICKS) {
		return;
	}

	opline = get_next_op();

	opline->opcode = CREX_TICKS;
	opline->extended_value = FC(declarables).ticks;
}
/* }}} */

static inline crex_op *crex_emit_op_data(znode *value) /* {{{ */
{
	return crex_emit_op(NULL, CREX_OP_DATA, value, NULL);
}
/* }}} */

static inline uint32_t crex_emit_jump(uint32_t opnum_target) /* {{{ */
{
	uint32_t opnum = get_next_op_number();
	crex_op *opline = crex_emit_op(NULL, CREX_JMP, NULL, NULL);
	opline->op1.opline_num = opnum_target;
	return opnum;
}
/* }}} */

CREX_API bool crex_is_smart_branch(const crex_op *opline) /* {{{ */
{
	switch (opline->opcode) {
		case CREX_IS_IDENTICAL:
		case CREX_IS_NOT_IDENTICAL:
		case CREX_IS_EQUAL:
		case CREX_IS_NOT_EQUAL:
		case CREX_IS_SMALLER:
		case CREX_IS_SMALLER_OR_EQUAL:
		case CREX_CASE:
		case CREX_CASE_STRICT:
		case CREX_ISSET_ISEMPTY_CV:
		case CREX_ISSET_ISEMPTY_VAR:
		case CREX_ISSET_ISEMPTY_DIM_OBJ:
		case CREX_ISSET_ISEMPTY_PROP_OBJ:
		case CREX_ISSET_ISEMPTY_STATIC_PROP:
		case CREX_INSTANCEOF:
		case CREX_TYPE_CHECK:
		case CREX_DEFINED:
		case CREX_IN_ARRAY:
		case CREX_ARRAY_KEY_EXISTS:
			return 1;
		default:
			return 0;
	}
}
/* }}} */

static inline uint32_t crex_emit_cond_jump(uint8_t opcode, znode *cond, uint32_t opnum_target) /* {{{ */
{
	uint32_t opnum = get_next_op_number();
	crex_op *opline;

	if (cond->op_type == IS_TMP_VAR && opnum > 0) {
		opline = CG(active_op_array)->opcodes + opnum - 1;
		if (opline->result_type == IS_TMP_VAR
		 && opline->result.var == cond->u.op.var
		 && crex_is_smart_branch(opline)) {
			if (opcode == CREX_JMPZ) {
				opline->result_type = IS_TMP_VAR | IS_SMART_BRANCH_JMPZ;
			} else {
				CREX_ASSERT(opcode == CREX_JMPNZ);
				opline->result_type = IS_TMP_VAR | IS_SMART_BRANCH_JMPNZ;
			}
		}
	}
	opline = crex_emit_op(NULL, opcode, cond, NULL);
	opline->op2.opline_num = opnum_target;
	return opnum;
}
/* }}} */

static inline void crex_update_jump_target(uint32_t opnum_jump, uint32_t opnum_target) /* {{{ */
{
	crex_op *opline = &CG(active_op_array)->opcodes[opnum_jump];
	switch (opline->opcode) {
		case CREX_JMP:
			opline->op1.opline_num = opnum_target;
			break;
		case CREX_JMPZ:
		case CREX_JMPNZ:
		case CREX_JMPC_EX:
		case CREX_JMPNC_EX:
		case CREX_JMP_SET:
		case CREX_COALESCE:
		case CREX_JMP_NULL:
		case CREX_BIND_INIT_STATIC_OR_JMP:
			opline->op2.opline_num = opnum_target;
			break;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
}
/* }}} */

static inline void crex_update_jump_target_to_next(uint32_t opnum_jump) /* {{{ */
{
	crex_update_jump_target(opnum_jump, get_next_op_number());
}
/* }}} */

static inline crex_op *crex_delayed_emit_op(znode *result, uint8_t opcode, znode *op1, znode *op2) /* {{{ */
{
	crex_op tmp_opline;

	init_op(&tmp_opline);

	tmp_opline.opcode = opcode;
	if (op1 != NULL) {
		SET_NODE(tmp_opline.op1, op1);
	}
	if (op2 != NULL) {
		SET_NODE(tmp_opline.op2, op2);
	}
	if (result) {
		crex_make_var_result(result, &tmp_opline);
	}

	crex_stack_push(&CG(delayed_oplines_stack), &tmp_opline);
	return crex_stack_top(&CG(delayed_oplines_stack));
}
/* }}} */

static inline uint32_t crex_delayed_compile_begin(void) /* {{{ */
{
	return crex_stack_count(&CG(delayed_oplines_stack));
}
/* }}} */

static crex_op *crex_delayed_compile_end(uint32_t offset) /* {{{ */
{
	crex_op *opline = NULL, *oplines = crex_stack_base(&CG(delayed_oplines_stack));
	uint32_t i, count = crex_stack_count(&CG(delayed_oplines_stack));

	CREX_ASSERT(count >= offset);
	for (i = offset; i < count; ++i) {
		if (EXPECTED(oplines[i].opcode != CREX_NOP)) {
			opline = get_next_op();
			memcpy(opline, &oplines[i], sizeof(crex_op));
		} else {
			opline = CG(active_op_array)->opcodes + oplines[i].extended_value;
		}
	}

	CG(delayed_oplines_stack).top = offset;
	return opline;
}
/* }}} */

static bool crex_ast_kind_is_short_circuited(crex_ast_kind ast_kind)
{
	switch (ast_kind) {
		case CREX_AST_DIM:
		case CREX_AST_PROP:
		case CREX_AST_NULLSAFE_PROP:
		case CREX_AST_STATIC_PROP:
		case CREX_AST_METHOD_CALL:
		case CREX_AST_NULLSAFE_METHOD_CALL:
		case CREX_AST_STATIC_CALL:
			return 1;
		default:
			return 0;
	}
}

static bool crex_ast_is_short_circuited(const crex_ast *ast)
{
	switch (ast->kind) {
		case CREX_AST_DIM:
		case CREX_AST_PROP:
		case CREX_AST_STATIC_PROP:
		case CREX_AST_METHOD_CALL:
		case CREX_AST_STATIC_CALL:
			return crex_ast_is_short_circuited(ast->child[0]);
		case CREX_AST_NULLSAFE_PROP:
		case CREX_AST_NULLSAFE_METHOD_CALL:
			return 1;
		default:
			return 0;
	}
}

/* Mark nodes that are an inner part of a short-circuiting chain.
 * We should not perform a "commit" on them, as it will be performed by the outer-most node.
 * We do this to avoid passing down an argument in various compile functions. */

#define CREX_SHORT_CIRCUITING_INNER 0x8000

static void crex_short_circuiting_mark_inner(crex_ast *ast) {
	if (crex_ast_kind_is_short_circuited(ast->kind)) {
		ast->attr |= CREX_SHORT_CIRCUITING_INNER;
	}
}

static uint32_t crex_short_circuiting_checkpoint(void)
{
	return crex_stack_count(&CG(short_circuiting_opnums));
}

static void crex_short_circuiting_commit(uint32_t checkpoint, znode *result, crex_ast *ast)
{
	bool is_short_circuited = crex_ast_kind_is_short_circuited(ast->kind)
		|| ast->kind == CREX_AST_ISSET || ast->kind == CREX_AST_EMPTY;
	if (!is_short_circuited) {
		CREX_ASSERT(crex_stack_count(&CG(short_circuiting_opnums)) == checkpoint
			&& "Short circuiting stack should be empty");
		return;
	}

	if (ast->attr & CREX_SHORT_CIRCUITING_INNER) {
		/* Outer-most node will commit. */
		return;
	}

	while (crex_stack_count(&CG(short_circuiting_opnums)) != checkpoint) {
		uint32_t opnum = *(uint32_t *) crex_stack_top(&CG(short_circuiting_opnums));
		crex_op *opline = &CG(active_op_array)->opcodes[opnum];
		opline->op2.opline_num = get_next_op_number();
		SET_NODE(opline->result, result);
		opline->extended_value |=
			ast->kind == CREX_AST_ISSET ? CREX_SHORT_CIRCUITING_CHAIN_ISSET :
			ast->kind == CREX_AST_EMPTY ? CREX_SHORT_CIRCUITING_CHAIN_EMPTY :
			                              CREX_SHORT_CIRCUITING_CHAIN_EXPR;
		crex_stack_del_top(&CG(short_circuiting_opnums));
	}
}

static void crex_emit_jmp_null(znode *obj_node, uint32_t bp_type)
{
	uint32_t jmp_null_opnum = get_next_op_number();
	crex_op *opline = crex_emit_op(NULL, CREX_JMP_NULL, obj_node, NULL);
	if (opline->op1_type == IS_CONST) {
		C_TRY_ADDREF_P(CT_CONSTANT(opline->op1));
	}
	if (bp_type == BP_VAR_IS) {
		opline->extended_value |= CREX_JMP_NULL_BP_VAR_IS;
	}
	crex_stack_push(&CG(short_circuiting_opnums), &jmp_null_opnum);
}

static void crex_compile_memoized_expr(znode *result, crex_ast *expr) /* {{{ */
{
	const crex_memoize_mode memoize_mode = CG(memoize_mode);
	if (memoize_mode == CREX_MEMOIZE_COMPILE) {
		znode memoized_result;

		/* Go through normal compilation */
		CG(memoize_mode) = CREX_MEMOIZE_NONE;
		crex_compile_expr(result, expr);
		CG(memoize_mode) = CREX_MEMOIZE_COMPILE;

		if (result->op_type == IS_VAR) {
			crex_emit_op(&memoized_result, CREX_COPY_TMP, result, NULL);
		} else if (result->op_type == IS_TMP_VAR) {
			crex_emit_op_tmp(&memoized_result, CREX_COPY_TMP, result, NULL);
		} else {
			if (result->op_type == IS_CONST) {
				C_TRY_ADDREF(result->u.constant);
			}
			memoized_result = *result;
		}

		crex_hash_index_update_mem(
			CG(memoized_exprs), (uintptr_t) expr, &memoized_result, sizeof(znode));
	} else if (memoize_mode == CREX_MEMOIZE_FETCH) {
		znode *memoized_result = crex_hash_index_find_ptr(CG(memoized_exprs), (uintptr_t) expr);
		*result = *memoized_result;
		if (result->op_type == IS_CONST) {
			C_TRY_ADDREF(result->u.constant);
		}
	} else {
		CREX_UNREACHABLE();
	}
}
/* }}} */

/* Remember to update type_num_classes() in compact_literals.c when changing this function */
static size_t crex_type_get_num_classes(crex_type type) {
	if (!CREX_TYPE_IS_COMPLEX(type)) {
		return 0;
	}
	if (CREX_TYPE_HAS_LIST(type)) {
		/* Intersection types cannot have nested list types */
		if (CREX_TYPE_IS_INTERSECTION(type)) {
			return CREX_TYPE_LIST(type)->num_types;
		}
		CREX_ASSERT(CREX_TYPE_IS_UNION(type));
		size_t count = 0;
		crex_type *list_type;

		CREX_TYPE_LIST_FOREACH(CREX_TYPE_LIST(type), list_type) {
			if (CREX_TYPE_IS_INTERSECTION(*list_type)) {
				count += CREX_TYPE_LIST(*list_type)->num_types;
			} else {
				CREX_ASSERT(!CREX_TYPE_HAS_LIST(*list_type));
				count += 1;
			}
		} CREX_TYPE_LIST_FOREACH_END();
		return count;
	}
	return 1;
}

static void crex_emit_return_type_check(
		znode *expr, crex_arg_info *return_info, bool implicit) /* {{{ */
{
	crex_type type = return_info->type;
	if (CREX_TYPE_IS_SET(type)) {
		crex_op *opline;

		/* `return ...;` is illegal in a void function (but `return;` isn't) */
		if (CREX_TYPE_CONTAINS_CODE(type, IS_VOID)) {
			if (expr) {
				if (expr->op_type == IS_CONST && C_TYPE(expr->u.constant) == IS_NULL) {
					crex_error_noreturn(E_COMPILE_ERROR,
						"A void function must not return a value "
						"(did you mean \"return;\" instead of \"return null;\"?)");
				} else {
					crex_error_noreturn(E_COMPILE_ERROR, "A void function must not return a value");
				}
			}
			/* we don't need run-time check */
			return;
		}

		/* `return` is illegal in a never-returning function */
		if (CREX_TYPE_CONTAINS_CODE(type, IS_NEVER)) {
			/* Implicit case handled separately using VERIFY_NEVER_TYPE opcode. */
			CREX_ASSERT(!implicit);
			crex_error_noreturn(E_COMPILE_ERROR, "A never-returning function must not return");
			return;
		}

		if (!expr && !implicit) {
			if (CREX_TYPE_ALLOW_NULL(type)) {
				crex_error_noreturn(E_COMPILE_ERROR,
					"A function with return type must return a value "
					"(did you mean \"return null;\" instead of \"return;\"?)");
			} else {
				crex_error_noreturn(E_COMPILE_ERROR,
					"A function with return type must return a value");
			}
		}

		if (expr && CREX_TYPE_PURE_MASK(type) == MAY_BE_ANY) {
			/* we don't need run-time check for mixed return type */
			return;
		}

		if (expr && expr->op_type == IS_CONST && CREX_TYPE_CONTAINS_CODE(type, C_TYPE(expr->u.constant))) {
			/* we don't need run-time check */
			return;
		}

		opline = crex_emit_op(NULL, CREX_VERIFY_RETURN_TYPE, expr, NULL);
		if (expr && expr->op_type == IS_CONST) {
			opline->result_type = expr->op_type = IS_TMP_VAR;
			opline->result.var = expr->u.op.var = get_temporary_variable();
		}

		opline->op2.num = crex_alloc_cache_slots(crex_type_get_num_classes(return_info->type));
	}
}
/* }}} */

void crex_emit_final_return(bool return_one) /* {{{ */
{
	znode zn;
	crex_op *ret;
	bool returns_reference = (CG(active_op_array)->fn_flags & CREX_ACC_RETURN_REFERENCE) != 0;

	if ((CG(active_op_array)->fn_flags & CREX_ACC_HAS_RETURN_TYPE)
			&& !(CG(active_op_array)->fn_flags & CREX_ACC_GENERATOR)) {
		crex_arg_info *return_info = CG(active_op_array)->arg_info - 1;

		if (CREX_TYPE_CONTAINS_CODE(return_info->type, IS_NEVER)) {
			crex_emit_op(NULL, CREX_VERIFY_NEVER_TYPE, NULL, NULL);
			return;
		}

		crex_emit_return_type_check(NULL, return_info, 1);
	}

	zn.op_type = IS_CONST;
	if (return_one) {
		ZVAL_LONG(&zn.u.constant, 1);
	} else {
		ZVAL_NULL(&zn.u.constant);
	}

	ret = crex_emit_op(NULL, returns_reference ? CREX_RETURN_BY_REF : CREX_RETURN, &zn, NULL);
	ret->extended_value = -1;
}
/* }}} */

static inline bool crex_is_variable(crex_ast *ast) /* {{{ */
{
	return ast->kind == CREX_AST_VAR
		|| ast->kind == CREX_AST_DIM
		|| ast->kind == CREX_AST_PROP
		|| ast->kind == CREX_AST_NULLSAFE_PROP
		|| ast->kind == CREX_AST_STATIC_PROP;
}
/* }}} */

static inline bool crex_is_call(crex_ast *ast) /* {{{ */
{
	return ast->kind == CREX_AST_CALL
		|| ast->kind == CREX_AST_METHOD_CALL
		|| ast->kind == CREX_AST_NULLSAFE_METHOD_CALL
		|| ast->kind == CREX_AST_STATIC_CALL;
}
/* }}} */

static inline bool crex_is_variable_or_call(crex_ast *ast) /* {{{ */
{
	return crex_is_variable(ast) || crex_is_call(ast);
}
/* }}} */

static inline bool crex_is_unticked_stmt(crex_ast *ast) /* {{{ */
{
	return ast->kind == CREX_AST_STMT_LIST || ast->kind == CREX_AST_LABEL
		|| ast->kind == CREX_AST_PROP_DECL || ast->kind == CREX_AST_CLASS_CONST_GROUP
		|| ast->kind == CREX_AST_USE_TRAIT || ast->kind == CREX_AST_METHOD;
}
/* }}} */

static inline bool crex_can_write_to_variable(crex_ast *ast) /* {{{ */
{
	while (
		ast->kind == CREX_AST_DIM
		|| ast->kind == CREX_AST_PROP
	) {
		ast = ast->child[0];
	}

	return crex_is_variable_or_call(ast) && !crex_ast_is_short_circuited(ast);
}
/* }}} */

static inline bool crex_is_const_default_class_ref(crex_ast *name_ast) /* {{{ */
{
	if (name_ast->kind != CREX_AST_ZVAL) {
		return 0;
	}

	return CREX_FETCH_CLASS_DEFAULT == crex_get_class_fetch_type_ast(name_ast);
}
/* }}} */

static inline void crex_handle_numeric_op(znode *node) /* {{{ */
{
	if (node->op_type == IS_CONST && C_TYPE(node->u.constant) == IS_STRING) {
		crex_ulong index;

		if (CREX_HANDLE_NUMERIC(C_STR(node->u.constant), index)) {
			zval_ptr_dtor(&node->u.constant);
			ZVAL_LONG(&node->u.constant, index);
		}
	}
}
/* }}} */

static inline void crex_handle_numeric_dim(crex_op *opline, znode *dim_node) /* {{{ */
{
	if (C_TYPE(dim_node->u.constant) == IS_STRING) {
		crex_ulong index;

		if (CREX_HANDLE_NUMERIC(C_STR(dim_node->u.constant), index)) {
			/* For numeric indexes we also keep the original value to use by ArrayAccess
			 * See bug #63217
			 */
			int c = crex_add_literal(&dim_node->u.constant);
			CREX_ASSERT(opline->op2.constant + 1 == c);
			ZVAL_LONG(CT_CONSTANT(opline->op2), index);
			C_EXTRA_P(CT_CONSTANT(opline->op2)) = CREX_EXTRA_VALUE;
			return;
		}
	}
}
/* }}} */

static inline void crex_set_class_name_op1(crex_op *opline, znode *class_node) /* {{{ */
{
	if (class_node->op_type == IS_CONST) {
		opline->op1_type = IS_CONST;
		opline->op1.constant = crex_add_class_name_literal(
			C_STR(class_node->u.constant));
	} else {
		SET_NODE(opline->op1, class_node);
	}
}
/* }}} */

static void crex_compile_class_ref(znode *result, crex_ast *name_ast, uint32_t fetch_flags) /* {{{ */
{
	uint32_t fetch_type;

	if (name_ast->kind != CREX_AST_ZVAL) {
		znode name_node;

		crex_compile_expr(&name_node, name_ast);

		if (name_node.op_type == IS_CONST) {
			crex_string *name;

			if (C_TYPE(name_node.u.constant) != IS_STRING) {
				crex_error_noreturn(E_COMPILE_ERROR, "Illegal class name");
			}

			name = C_STR(name_node.u.constant);
			fetch_type = crex_get_class_fetch_type(name);

			if (fetch_type == CREX_FETCH_CLASS_DEFAULT) {
				result->op_type = IS_CONST;
				ZVAL_STR(&result->u.constant, crex_resolve_class_name(name, CREX_NAME_FQ));
			} else {
				crex_ensure_valid_class_fetch_type(fetch_type);
				result->op_type = IS_UNUSED;
				result->u.op.num = fetch_type | fetch_flags;
			}

			crex_string_release_ex(name, 0);
		} else {
			crex_op *opline = crex_emit_op(result, CREX_FETCH_CLASS, NULL, &name_node);
			opline->op1.num = CREX_FETCH_CLASS_DEFAULT | fetch_flags;
		}
		return;
	}

	/* Fully qualified names are always default refs */
	if (name_ast->attr == CREX_NAME_FQ) {
		result->op_type = IS_CONST;
		ZVAL_STR(&result->u.constant, crex_resolve_class_name_ast(name_ast));
		return;
	}

	fetch_type = crex_get_class_fetch_type(crex_ast_get_str(name_ast));
	if (CREX_FETCH_CLASS_DEFAULT == fetch_type) {
		result->op_type = IS_CONST;
		ZVAL_STR(&result->u.constant, crex_resolve_class_name_ast(name_ast));
	} else {
		crex_ensure_valid_class_fetch_type(fetch_type);
		result->op_type = IS_UNUSED;
		result->u.op.num = fetch_type | fetch_flags;
	}
}
/* }}} */

static crex_result crex_try_compile_cv(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *name_ast = ast->child[0];
	if (name_ast->kind == CREX_AST_ZVAL) {
		zval *zv = crex_ast_get_zval(name_ast);
		crex_string *name;

		if (EXPECTED(C_TYPE_P(zv) == IS_STRING)) {
			name = zval_make_interned_string(zv);
		} else {
			name = crex_new_interned_string(zval_get_string_func(zv));
		}

		if (crex_is_auto_global(name)) {
			return FAILURE;
		}

		result->op_type = IS_CV;
		result->u.op.var = lookup_cv(name);

		if (UNEXPECTED(C_TYPE_P(zv) != IS_STRING)) {
			crex_string_release_ex(name, 0);
		}

		return SUCCESS;
	}

	return FAILURE;
}
/* }}} */

static crex_op *crex_compile_simple_var_no_cv(znode *result, crex_ast *ast, uint32_t type, bool delayed) /* {{{ */
{
	crex_ast *name_ast = ast->child[0];
	znode name_node;
	crex_op *opline;

	crex_compile_expr(&name_node, name_ast);
	if (name_node.op_type == IS_CONST) {
		convert_to_string(&name_node.u.constant);
	}

	if (delayed) {
		opline = crex_delayed_emit_op(result, CREX_FETCH_R, &name_node, NULL);
	} else {
		opline = crex_emit_op(result, CREX_FETCH_R, &name_node, NULL);
	}

	if (name_node.op_type == IS_CONST &&
	    crex_is_auto_global(C_STR(name_node.u.constant))) {

		opline->extended_value = CREX_FETCH_GLOBAL;
	} else {
		opline->extended_value = CREX_FETCH_LOCAL;
	}

	crex_adjust_for_fetch_type(opline, result, type);
	return opline;
}
/* }}} */

static bool is_this_fetch(crex_ast *ast) /* {{{ */
{
	if (ast->kind == CREX_AST_VAR && ast->child[0]->kind == CREX_AST_ZVAL) {
		zval *name = crex_ast_get_zval(ast->child[0]);
		return C_TYPE_P(name) == IS_STRING && crex_string_equals(C_STR_P(name), ZSTR_KNOWN(CREX_STR_THIS));
	}

	return 0;
}
/* }}} */

static bool is_globals_fetch(const crex_ast *ast)
{
	if (ast->kind == CREX_AST_VAR && ast->child[0]->kind == CREX_AST_ZVAL) {
		zval *name = crex_ast_get_zval(ast->child[0]);
		return C_TYPE_P(name) == IS_STRING && crex_string_equals_literal(C_STR_P(name), "GLOBALS");
	}

	return 0;
}

static bool is_global_var_fetch(crex_ast *ast)
{
	return ast->kind == CREX_AST_DIM && is_globals_fetch(ast->child[0]);
}

static bool this_guaranteed_exists(void) /* {{{ */
{
	crex_op_array *op_array = CG(active_op_array);
	/* Instance methods always have a $this.
	 * This also includes closures that have a scope and use $this. */
	return op_array->scope != NULL
		&& (op_array->fn_flags & CREX_ACC_STATIC) == 0;
}
/* }}} */

static crex_op *crex_compile_simple_var(znode *result, crex_ast *ast, uint32_t type, bool delayed) /* {{{ */
{
	if (is_this_fetch(ast)) {
		crex_op *opline = crex_emit_op(result, CREX_FETCH_THIS, NULL, NULL);
		if ((type == BP_VAR_R) || (type == BP_VAR_IS)) {
			opline->result_type = IS_TMP_VAR;
			result->op_type = IS_TMP_VAR;
		}
		CG(active_op_array)->fn_flags |= CREX_ACC_USES_THIS;
		return opline;
	} else if (is_globals_fetch(ast)) {
		crex_op *opline = crex_emit_op(result, CREX_FETCH_GLOBALS, NULL, NULL);
		if (type == BP_VAR_R || type == BP_VAR_IS) {
			opline->result_type = IS_TMP_VAR;
			result->op_type = IS_TMP_VAR;
		}
		return opline;
	} else if (crex_try_compile_cv(result, ast) == FAILURE) {
		return crex_compile_simple_var_no_cv(result, ast, type, delayed);
	}
	return NULL;
}
/* }}} */

static void crex_separate_if_call_and_write(znode *node, crex_ast *ast, uint32_t type) /* {{{ */
{
	if (type != BP_VAR_R
	 && type != BP_VAR_IS
	 /* Whether a FUNC_ARG is R may only be determined at runtime. */
	 && type != BP_VAR_FUNC_ARG
	 && crex_is_call(ast)) {
		if (node->op_type == IS_VAR) {
			crex_op *opline = crex_emit_op(NULL, CREX_SEPARATE, node, NULL);
			opline->result_type = IS_VAR;
			opline->result.var = opline->op1.var;
		} else {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot use result of built-in function in write context");
		}
	}
}
/* }}} */

static inline void crex_emit_assign_znode(crex_ast *var_ast, znode *value_node) /* {{{ */
{
	znode dummy_node;
	crex_ast *assign_ast = crex_ast_create(CREX_AST_ASSIGN, var_ast,
		crex_ast_create_znode(value_node));
	crex_compile_expr(&dummy_node, assign_ast);
	crex_do_free(&dummy_node);
}
/* }}} */

static crex_op *crex_delayed_compile_dim(znode *result, crex_ast *ast, uint32_t type, bool by_ref)
{
	if (ast->attr == CREX_DIM_ALTERNATIVE_SYNTAX) {
		crex_error(E_COMPILE_ERROR, "Array and string offset access syntax with curly braces is no longer supported");
	}
	crex_ast *var_ast = ast->child[0];
	crex_ast *dim_ast = ast->child[1];
	crex_op *opline;

	znode var_node, dim_node;

	if (is_globals_fetch(var_ast)) {
		if (dim_ast == NULL) {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot append to $GLOBALS");
		}

		crex_compile_expr(&dim_node, dim_ast);
		if (dim_node.op_type == IS_CONST) {
			convert_to_string(&dim_node.u.constant);
		}

		opline = crex_delayed_emit_op(result, CREX_FETCH_R, &dim_node, NULL);
		opline->extended_value = CREX_FETCH_GLOBAL;
		crex_adjust_for_fetch_type(opline, result, type);
		return opline;
	} else {
		crex_short_circuiting_mark_inner(var_ast);
		opline = crex_delayed_compile_var(&var_node, var_ast, type, 0);
		if (opline) {
			if (type == BP_VAR_W && (opline->opcode == CREX_FETCH_STATIC_PROP_W || opline->opcode == CREX_FETCH_OBJ_W)) {
				opline->extended_value |= CREX_FETCH_DIM_WRITE;
			} else if (opline->opcode == CREX_FETCH_DIM_W
					|| opline->opcode == CREX_FETCH_DIM_RW
					|| opline->opcode == CREX_FETCH_DIM_FUNC_ARG
					|| opline->opcode == CREX_FETCH_DIM_UNSET) {
				opline->extended_value = CREX_FETCH_DIM_DIM;
			}
		}
	}

	crex_separate_if_call_and_write(&var_node, var_ast, type);

	if (dim_ast == NULL) {
		if (type == BP_VAR_R || type == BP_VAR_IS) {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot use [] for reading");
		}
		if (type == BP_VAR_UNSET) {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot use [] for unsetting");
		}
		dim_node.op_type = IS_UNUSED;
	} else {
		crex_compile_expr(&dim_node, dim_ast);
	}

	opline = crex_delayed_emit_op(result, CREX_FETCH_DIM_R, &var_node, &dim_node);
	crex_adjust_for_fetch_type(opline, result, type);
	if (by_ref) {
		opline->extended_value = CREX_FETCH_DIM_REF;
	}

	if (dim_node.op_type == IS_CONST) {
		crex_handle_numeric_dim(opline, &dim_node);
	}
	return opline;
}

static crex_op *crex_compile_dim(znode *result, crex_ast *ast, uint32_t type, bool by_ref) /* {{{ */
{
	uint32_t offset = crex_delayed_compile_begin();
	crex_delayed_compile_dim(result, ast, type, by_ref);
	return crex_delayed_compile_end(offset);
}
/* }}} */

static crex_op *crex_delayed_compile_prop(znode *result, crex_ast *ast, uint32_t type) /* {{{ */
{
	crex_ast *obj_ast = ast->child[0];
	crex_ast *prop_ast = ast->child[1];

	znode obj_node, prop_node;
	crex_op *opline;
	bool nullsafe = ast->kind == CREX_AST_NULLSAFE_PROP;

	if (is_this_fetch(obj_ast)) {
		if (this_guaranteed_exists()) {
			obj_node.op_type = IS_UNUSED;
		} else {
			crex_emit_op(&obj_node, CREX_FETCH_THIS, NULL, NULL);
		}
		CG(active_op_array)->fn_flags |= CREX_ACC_USES_THIS;

		/* We will throw if $this doesn't exist, so there's no need to emit a JMP_NULL
		 * check for a nullsafe access. */
	} else {
		crex_short_circuiting_mark_inner(obj_ast);
		opline = crex_delayed_compile_var(&obj_node, obj_ast, type, 0);
		if (opline && (opline->opcode == CREX_FETCH_DIM_W
				|| opline->opcode == CREX_FETCH_DIM_RW
				|| opline->opcode == CREX_FETCH_DIM_FUNC_ARG
				|| opline->opcode == CREX_FETCH_DIM_UNSET)) {
			opline->extended_value = CREX_FETCH_DIM_OBJ;
		}

		crex_separate_if_call_and_write(&obj_node, obj_ast, type);
		if (nullsafe) {
			if (obj_node.op_type == IS_TMP_VAR) {
				/* Flush delayed oplines */
				crex_op *opline = NULL, *oplines = crex_stack_base(&CG(delayed_oplines_stack));
				uint32_t var = obj_node.u.op.var;
				uint32_t count = crex_stack_count(&CG(delayed_oplines_stack));
				uint32_t i = count;

				while (i > 0 && oplines[i-1].result_type == IS_TMP_VAR && oplines[i-1].result.var == var) {
					i--;
					if (oplines[i].op1_type == IS_TMP_VAR) {
						var = oplines[i].op1.var;
					} else {
						break;
					}
				}
				for (; i < count; ++i) {
					if (oplines[i].opcode != CREX_NOP) {
						opline = get_next_op();
						memcpy(opline, &oplines[i], sizeof(crex_op));
						oplines[i].opcode = CREX_NOP;
						oplines[i].extended_value = opline - CG(active_op_array)->opcodes;
					}
				}
			}
			crex_emit_jmp_null(&obj_node, type);
		}
	}

	crex_compile_expr(&prop_node, prop_ast);

	opline = crex_delayed_emit_op(result, CREX_FETCH_OBJ_R, &obj_node, &prop_node);
	if (opline->op2_type == IS_CONST) {
		convert_to_string(CT_CONSTANT(opline->op2));
		crex_string_hash_val(C_STR_P(CT_CONSTANT(opline->op2)));
		opline->extended_value = crex_alloc_cache_slots(3);
	}

	crex_adjust_for_fetch_type(opline, result, type);

	return opline;
}
/* }}} */

static crex_op *crex_compile_prop(znode *result, crex_ast *ast, uint32_t type, bool by_ref) /* {{{ */
{
	uint32_t offset = crex_delayed_compile_begin();
	crex_op *opline = crex_delayed_compile_prop(result, ast, type);
	if (by_ref) { /* shared with cache_slot */
		opline->extended_value |= CREX_FETCH_REF;
	}
	return crex_delayed_compile_end(offset);
}
/* }}} */

static crex_op *crex_compile_static_prop(znode *result, crex_ast *ast, uint32_t type, bool by_ref, bool delayed) /* {{{ */
{
	crex_ast *class_ast = ast->child[0];
	crex_ast *prop_ast = ast->child[1];

	znode class_node, prop_node;
	crex_op *opline;

	crex_short_circuiting_mark_inner(class_ast);
	crex_compile_class_ref(&class_node, class_ast, CREX_FETCH_CLASS_EXCEPTION);

	crex_compile_expr(&prop_node, prop_ast);

	if (delayed) {
		opline = crex_delayed_emit_op(result, CREX_FETCH_STATIC_PROP_R, &prop_node, NULL);
	} else {
		opline = crex_emit_op(result, CREX_FETCH_STATIC_PROP_R, &prop_node, NULL);
	}
	if (opline->op1_type == IS_CONST) {
		convert_to_string(CT_CONSTANT(opline->op1));
		opline->extended_value = crex_alloc_cache_slots(3);
	}
	if (class_node.op_type == IS_CONST) {
		opline->op2_type = IS_CONST;
		opline->op2.constant = crex_add_class_name_literal(
			C_STR(class_node.u.constant));
		if (opline->op1_type != IS_CONST) {
			opline->extended_value = crex_alloc_cache_slot();
		}
	} else {
		SET_NODE(opline->op2, &class_node);
	}

	if (by_ref && (type == BP_VAR_W || type == BP_VAR_FUNC_ARG)) { /* shared with cache_slot */
		opline->extended_value |= CREX_FETCH_REF;
	}

	crex_adjust_for_fetch_type(opline, result, type);
	return opline;
}
/* }}} */

static void crex_verify_list_assign_target(crex_ast *var_ast, crex_ast_attr array_style) /* {{{ */ {
	if (var_ast->kind == CREX_AST_ARRAY) {
		if (var_ast->attr == CREX_ARRAY_SYNTAX_LONG) {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot assign to array(), use [] instead");
		}
		if (array_style != var_ast->attr) {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot mix [] and list()");
		}
	} else if (!crex_can_write_to_variable(var_ast)) {
		crex_error_noreturn(E_COMPILE_ERROR, "Assignments can only happen to writable values");
	}
}
/* }}} */

static inline void crex_emit_assign_ref_znode(crex_ast *var_ast, znode *value_node);

/* Propagate refs used on leaf elements to the surrounding list() structures. */
static bool crex_propagate_list_refs(crex_ast *ast) { /* {{{ */
	crex_ast_list *list = crex_ast_get_list(ast);
	bool has_refs = 0;
	uint32_t i;

	for (i = 0; i < list->children; ++i) {
		crex_ast *elem_ast = list->child[i];

		if (elem_ast) {
			crex_ast *var_ast = elem_ast->child[0];
			if (var_ast->kind == CREX_AST_ARRAY) {
				elem_ast->attr = crex_propagate_list_refs(var_ast);
			}
			has_refs |= elem_ast->attr;
		}
	}

	return has_refs;
}
/* }}} */

static bool list_is_keyed(crex_ast_list *list)
{
	for (uint32_t i = 0; i < list->children; i++) {
		crex_ast *child = list->child[i];
		if (child) {
			return child->kind == CREX_AST_ARRAY_ELEM && child->child[1] != NULL;
		}
	}
	return false;
}

static void crex_compile_list_assign(
		znode *result, crex_ast *ast, znode *expr_node, crex_ast_attr array_style) /* {{{ */
{
	crex_ast_list *list = crex_ast_get_list(ast);
	uint32_t i;
	bool has_elems = 0;
	bool is_keyed = list_is_keyed(list);

	if (list->children && expr_node->op_type == IS_CONST && C_TYPE(expr_node->u.constant) == IS_STRING) {
		zval_make_interned_string(&expr_node->u.constant);
	}

	for (i = 0; i < list->children; ++i) {
		crex_ast *elem_ast = list->child[i];
		crex_ast *var_ast, *key_ast;
		znode fetch_result, dim_node;
		crex_op *opline;

		if (elem_ast == NULL) {
			if (is_keyed) {
				crex_error(E_COMPILE_ERROR,
					"Cannot use empty array entries in keyed array assignment");
			} else {
				continue;
			}
		}

		if (elem_ast->kind == CREX_AST_UNPACK) {
			crex_error(E_COMPILE_ERROR,
					"Spread operator is not supported in assignments");
		}

		var_ast = elem_ast->child[0];
		key_ast = elem_ast->child[1];
		has_elems = 1;

		if (is_keyed) {
			if (key_ast == NULL) {
				crex_error(E_COMPILE_ERROR,
					"Cannot mix keyed and unkeyed array entries in assignments");
			}

			crex_compile_expr(&dim_node, key_ast);
		} else {
			if (key_ast != NULL) {
				crex_error(E_COMPILE_ERROR,
					"Cannot mix keyed and unkeyed array entries in assignments");
			}

			dim_node.op_type = IS_CONST;
			ZVAL_LONG(&dim_node.u.constant, i);
		}

		if (expr_node->op_type == IS_CONST) {
			C_TRY_ADDREF(expr_node->u.constant);
		}

		crex_verify_list_assign_target(var_ast, array_style);

		opline = crex_emit_op(&fetch_result,
			elem_ast->attr ? (expr_node->op_type == IS_CV ? CREX_FETCH_DIM_W : CREX_FETCH_LIST_W) : CREX_FETCH_LIST_R, expr_node, &dim_node);

		if (dim_node.op_type == IS_CONST) {
			crex_handle_numeric_dim(opline, &dim_node);
		}

		if (elem_ast->attr) {
			crex_emit_op(&fetch_result, CREX_MAKE_REF, &fetch_result, NULL);
		}
		if (var_ast->kind == CREX_AST_ARRAY) {
			crex_compile_list_assign(NULL, var_ast, &fetch_result, var_ast->attr);
		} else if (elem_ast->attr) {
			crex_emit_assign_ref_znode(var_ast, &fetch_result);
		} else {
			crex_emit_assign_znode(var_ast, &fetch_result);
		}
	}

	if (has_elems == 0) {
		crex_error_noreturn(E_COMPILE_ERROR, "Cannot use empty list");
	}

	if (result) {
		*result = *expr_node;
	} else {
		crex_do_free(expr_node);
	}
}
/* }}} */

static void crex_ensure_writable_variable(const crex_ast *ast) /* {{{ */
{
	if (ast->kind == CREX_AST_CALL) {
		crex_error_noreturn(E_COMPILE_ERROR, "Can't use function return value in write context");
	}
	if (
		ast->kind == CREX_AST_METHOD_CALL
		|| ast->kind == CREX_AST_NULLSAFE_METHOD_CALL
		|| ast->kind == CREX_AST_STATIC_CALL
	) {
		crex_error_noreturn(E_COMPILE_ERROR, "Can't use method return value in write context");
	}
	if (crex_ast_is_short_circuited(ast)) {
		crex_error_noreturn(E_COMPILE_ERROR, "Can't use nullsafe operator in write context");
	}
	if (is_globals_fetch(ast)) {
		crex_error_noreturn(E_COMPILE_ERROR,
			"$GLOBALS can only be modified using the $GLOBALS[$name] = $value syntax");
	}
}
/* }}} */

/* Detects $a... = $a pattern */
static bool crex_is_assign_to_self(crex_ast *var_ast, crex_ast *expr_ast) /* {{{ */
{
	if (expr_ast->kind != CREX_AST_VAR || expr_ast->child[0]->kind != CREX_AST_ZVAL) {
		return 0;
	}

	while (crex_is_variable(var_ast) && var_ast->kind != CREX_AST_VAR) {
		var_ast = var_ast->child[0];
	}

	if (var_ast->kind != CREX_AST_VAR || var_ast->child[0]->kind != CREX_AST_ZVAL) {
		return 0;
	}

	{
		crex_string *name1 = zval_get_string(crex_ast_get_zval(var_ast->child[0]));
		crex_string *name2 = zval_get_string(crex_ast_get_zval(expr_ast->child[0]));
		bool result = crex_string_equals(name1, name2);
		crex_string_release_ex(name1, 0);
		crex_string_release_ex(name2, 0);
		return result;
	}
}
/* }}} */

static void crex_compile_expr_with_potential_assign_to_self(
		znode *expr_node, crex_ast *expr_ast, crex_ast *var_ast) {
	if (crex_is_assign_to_self(var_ast, expr_ast) && !is_this_fetch(expr_ast)) {
		/* $a[0] = $a should evaluate the right $a first */
		znode cv_node;

		if (crex_try_compile_cv(&cv_node, expr_ast) == FAILURE) {
			crex_compile_simple_var_no_cv(expr_node, expr_ast, BP_VAR_R, 0);
		} else {
			crex_emit_op_tmp(expr_node, CREX_QM_ASSIGN, &cv_node, NULL);
		}
	} else {
		crex_compile_expr(expr_node, expr_ast);
	}
}

static void crex_compile_assign(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *var_ast = ast->child[0];
	crex_ast *expr_ast = ast->child[1];

	znode var_node, expr_node;
	crex_op *opline;
	uint32_t offset;
	if (is_this_fetch(var_ast)) {
		crex_error_noreturn(E_COMPILE_ERROR, "Cannot re-assign $this");
	}

	crex_ensure_writable_variable(var_ast);

	/* Treat $GLOBALS['x'] assignment like assignment to variable. */
	crex_ast_kind kind = is_global_var_fetch(var_ast) ? CREX_AST_VAR : var_ast->kind;
	switch (kind) {
		case CREX_AST_VAR:
			offset = crex_delayed_compile_begin();
			crex_delayed_compile_var(&var_node, var_ast, BP_VAR_W, 0);
			crex_compile_expr(&expr_node, expr_ast);
			crex_delayed_compile_end(offset);
			CG(crex_lineno) = crex_ast_get_lineno(var_ast);
			crex_emit_op_tmp(result, CREX_ASSIGN, &var_node, &expr_node);
			return;
		case CREX_AST_STATIC_PROP:
			offset = crex_delayed_compile_begin();
			crex_delayed_compile_var(result, var_ast, BP_VAR_W, 0);
			crex_compile_expr(&expr_node, expr_ast);

			opline = crex_delayed_compile_end(offset);
			opline->opcode = CREX_ASSIGN_STATIC_PROP;
			opline->result_type = IS_TMP_VAR;
			result->op_type = IS_TMP_VAR;

			crex_emit_op_data(&expr_node);
			return;
		case CREX_AST_DIM:
			offset = crex_delayed_compile_begin();
			crex_delayed_compile_dim(result, var_ast, BP_VAR_W, /* by_ref */ false);
			crex_compile_expr_with_potential_assign_to_self(&expr_node, expr_ast, var_ast);

			opline = crex_delayed_compile_end(offset);
			opline->opcode = CREX_ASSIGN_DIM;
			opline->result_type = IS_TMP_VAR;
			result->op_type = IS_TMP_VAR;

			opline = crex_emit_op_data(&expr_node);
			return;
		case CREX_AST_PROP:
		case CREX_AST_NULLSAFE_PROP:
			offset = crex_delayed_compile_begin();
			crex_delayed_compile_prop(result, var_ast, BP_VAR_W);
			crex_compile_expr(&expr_node, expr_ast);

			opline = crex_delayed_compile_end(offset);
			opline->opcode = CREX_ASSIGN_OBJ;
			opline->result_type = IS_TMP_VAR;
			result->op_type = IS_TMP_VAR;

			crex_emit_op_data(&expr_node);
			return;
		case CREX_AST_ARRAY:
			if (crex_propagate_list_refs(var_ast)) {
				if (!crex_is_variable_or_call(expr_ast)) {
					crex_error_noreturn(E_COMPILE_ERROR,
						"Cannot assign reference to non referenceable value");
				} else if (crex_ast_is_short_circuited(expr_ast)) {
					crex_error_noreturn(E_COMPILE_ERROR,
						"Cannot take reference of a nullsafe chain");
				}

				crex_compile_var(&expr_node, expr_ast, BP_VAR_W, 1);
				/* MAKE_REF is usually not necessary for CVs. However, if there are
				 * self-assignments, this forces the RHS to evaluate first. */
				crex_emit_op(&expr_node, CREX_MAKE_REF, &expr_node, NULL);
			} else {
				if (expr_ast->kind == CREX_AST_VAR) {
					/* list($a, $b) = $a should evaluate the right $a first */
					znode cv_node;

					if (crex_try_compile_cv(&cv_node, expr_ast) == FAILURE) {
						crex_compile_simple_var_no_cv(&expr_node, expr_ast, BP_VAR_R, 0);
					} else {
						crex_emit_op_tmp(&expr_node, CREX_QM_ASSIGN, &cv_node, NULL);
					}
				} else {
					crex_compile_expr(&expr_node, expr_ast);
				}
			}

			crex_compile_list_assign(result, var_ast, &expr_node, var_ast->attr);
			return;
		EMPTY_SWITCH_DEFAULT_CASE();
	}
}
/* }}} */

static void crex_compile_assign_ref(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *target_ast = ast->child[0];
	crex_ast *source_ast = ast->child[1];

	znode target_node, source_node;
	crex_op *opline;
	uint32_t offset, flags;

	if (is_this_fetch(target_ast)) {
		crex_error_noreturn(E_COMPILE_ERROR, "Cannot re-assign $this");
	}
	crex_ensure_writable_variable(target_ast);
	if (crex_ast_is_short_circuited(source_ast)) {
		crex_error_noreturn(E_COMPILE_ERROR, "Cannot take reference of a nullsafe chain");
	}
	if (is_globals_fetch(source_ast)) {
		crex_error_noreturn(E_COMPILE_ERROR, "Cannot acquire reference to $GLOBALS");
	}

	offset = crex_delayed_compile_begin();
	crex_delayed_compile_var(&target_node, target_ast, BP_VAR_W, 1);
	crex_compile_var(&source_node, source_ast, BP_VAR_W, 1);

	if ((target_ast->kind != CREX_AST_VAR
	  || target_ast->child[0]->kind != CREX_AST_ZVAL)
	 && source_ast->kind != CREX_AST_ZNODE
	 && source_node.op_type != IS_CV) {
		/* Both LHS and RHS expressions may modify the same data structure,
		 * and the modification during RHS evaluation may dangle the pointer
		 * to the result of the LHS evaluation.
		 * Use MAKE_REF instruction to replace direct pointer with REFERENCE.
		 * See: Bug #71539
		 */
		crex_emit_op(&source_node, CREX_MAKE_REF, &source_node, NULL);
	}

	opline = crex_delayed_compile_end(offset);

	if (source_node.op_type != IS_VAR && crex_is_call(source_ast)) {
		crex_error_noreturn(E_COMPILE_ERROR, "Cannot use result of built-in function in write context");
	}

	flags = crex_is_call(source_ast) ? CREX_RETURNS_FUNCTION : 0;

	if (opline && opline->opcode == CREX_FETCH_OBJ_W) {
		opline->opcode = CREX_ASSIGN_OBJ_REF;
		opline->extended_value &= ~CREX_FETCH_REF;
		opline->extended_value |= flags;
		crex_emit_op_data(&source_node);
		*result = target_node;
	} else if (opline && opline->opcode == CREX_FETCH_STATIC_PROP_W) {
		opline->opcode = CREX_ASSIGN_STATIC_PROP_REF;
		opline->extended_value &= ~CREX_FETCH_REF;
		opline->extended_value |= flags;
		crex_emit_op_data(&source_node);
		*result = target_node;
	} else {
		opline = crex_emit_op(result, CREX_ASSIGN_REF, &target_node, &source_node);
		opline->extended_value = flags;
	}
}
/* }}} */

static inline void crex_emit_assign_ref_znode(crex_ast *var_ast, znode *value_node) /* {{{ */
{
	znode dummy_node;
	crex_ast *assign_ast = crex_ast_create(CREX_AST_ASSIGN_REF, var_ast,
		crex_ast_create_znode(value_node));
	crex_compile_expr(&dummy_node, assign_ast);
	crex_do_free(&dummy_node);
}
/* }}} */

static void crex_compile_compound_assign(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *var_ast = ast->child[0];
	crex_ast *expr_ast = ast->child[1];
	uint32_t opcode = ast->attr;

	znode var_node, expr_node;
	crex_op *opline;
	uint32_t offset, cache_slot;

	crex_ensure_writable_variable(var_ast);

	/* Treat $GLOBALS['x'] assignment like assignment to variable. */
	crex_ast_kind kind = is_global_var_fetch(var_ast) ? CREX_AST_VAR : var_ast->kind;
	switch (kind) {
		case CREX_AST_VAR:
			offset = crex_delayed_compile_begin();
			crex_delayed_compile_var(&var_node, var_ast, BP_VAR_RW, 0);
			crex_compile_expr(&expr_node, expr_ast);
			crex_delayed_compile_end(offset);
			opline = crex_emit_op_tmp(result, CREX_ASSIGN_OP, &var_node, &expr_node);
			opline->extended_value = opcode;
			return;
		case CREX_AST_STATIC_PROP:
			offset = crex_delayed_compile_begin();
			crex_delayed_compile_var(result, var_ast, BP_VAR_RW, 0);
			crex_compile_expr(&expr_node, expr_ast);

			opline = crex_delayed_compile_end(offset);
			cache_slot = opline->extended_value;
			opline->opcode = CREX_ASSIGN_STATIC_PROP_OP;
			opline->extended_value = opcode;
			opline->result_type = IS_TMP_VAR;
			result->op_type = IS_TMP_VAR;

			opline = crex_emit_op_data(&expr_node);
			opline->extended_value = cache_slot;
			return;
		case CREX_AST_DIM:
			offset = crex_delayed_compile_begin();
			crex_delayed_compile_dim(result, var_ast, BP_VAR_RW, /* by_ref */ false);
			crex_compile_expr_with_potential_assign_to_self(&expr_node, expr_ast, var_ast);

			opline = crex_delayed_compile_end(offset);
			opline->opcode = CREX_ASSIGN_DIM_OP;
			opline->extended_value = opcode;
			opline->result_type = IS_TMP_VAR;
			result->op_type = IS_TMP_VAR;

			crex_emit_op_data(&expr_node);
			return;
		case CREX_AST_PROP:
		case CREX_AST_NULLSAFE_PROP:
			offset = crex_delayed_compile_begin();
			crex_delayed_compile_prop(result, var_ast, BP_VAR_RW);
			crex_compile_expr(&expr_node, expr_ast);

			opline = crex_delayed_compile_end(offset);
			cache_slot = opline->extended_value;
			opline->opcode = CREX_ASSIGN_OBJ_OP;
			opline->extended_value = opcode;
			opline->result_type = IS_TMP_VAR;
			result->op_type = IS_TMP_VAR;

			opline = crex_emit_op_data(&expr_node);
			opline->extended_value = cache_slot;
			return;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
}
/* }}} */

static uint32_t crex_get_arg_num(crex_function *fn, crex_string *arg_name) {
	// TODO: Caching?
	if (fn->type == CREX_USER_FUNCTION) {
		for (uint32_t i = 0; i < fn->common.num_args; i++) {
			crex_arg_info *arg_info = &fn->op_array.arg_info[i];
			if (crex_string_equals(arg_info->name, arg_name)) {
				return i + 1;
			}
		}
	} else {
		for (uint32_t i = 0; i < fn->common.num_args; i++) {
			crex_internal_arg_info *arg_info = &fn->internal_function.arg_info[i];
			size_t len = strlen(arg_info->name);
			if (crex_string_equals_cstr(arg_name, arg_info->name, len)) {
				return i + 1;
			}
		}
	}

	/* Either an invalid argument name, or collected into a variadic argument. */
	return (uint32_t) -1;
}

static uint32_t crex_compile_args(
		crex_ast *ast, crex_function *fbc, bool *may_have_extra_named_args) /* {{{ */
{
	crex_ast_list *args = crex_ast_get_list(ast);
	uint32_t i;
	bool uses_arg_unpack = 0;
	uint32_t arg_count = 0; /* number of arguments not including unpacks */

	/* Whether named arguments are used syntactically, to enforce language level limitations.
	 * May not actually use named argument passing. */
	bool uses_named_args = 0;
	/* Whether there may be any undef arguments due to the use of named arguments. */
	bool may_have_undef = 0;
	/* Whether there may be any extra named arguments collected into a variadic. */
	*may_have_extra_named_args = 0;

	for (i = 0; i < args->children; ++i) {
		crex_ast *arg = args->child[i];
		crex_string *arg_name = NULL;
		uint32_t arg_num = i + 1;

		znode arg_node;
		crex_op *opline;
		uint8_t opcode;

		if (arg->kind == CREX_AST_UNPACK) {
			if (uses_named_args) {
				crex_error_noreturn(E_COMPILE_ERROR,
					"Cannot use argument unpacking after named arguments");
			}

			uses_arg_unpack = 1;
			fbc = NULL;

			crex_compile_expr(&arg_node, arg->child[0]);
			opline = crex_emit_op(NULL, CREX_SEND_UNPACK, &arg_node, NULL);
			opline->op2.num = arg_count;
			opline->result.var = EX_NUM_TO_VAR(arg_count - 1);

			/* Unpack may contain named arguments. */
			may_have_undef = 1;
			if (!fbc || (fbc->common.fn_flags & CREX_ACC_VARIADIC)) {
				*may_have_extra_named_args = 1;
			}
			continue;
		}

		if (arg->kind == CREX_AST_NAMED_ARG) {
			uses_named_args = 1;
			arg_name = zval_make_interned_string(crex_ast_get_zval(arg->child[0]));
			arg = arg->child[1];

			if (fbc && !uses_arg_unpack) {
				arg_num = crex_get_arg_num(fbc, arg_name);
				if (arg_num == arg_count + 1 && !may_have_undef) {
					/* Using named arguments, but passing in order. */
					arg_name = NULL;
					arg_count++;
				} else {
					// TODO: We could track which arguments were passed, even if out of order.
					may_have_undef = 1;
					if (arg_num == (uint32_t) -1 && (fbc->common.fn_flags & CREX_ACC_VARIADIC)) {
						*may_have_extra_named_args = 1;
					}
				}
			} else {
				arg_num = (uint32_t) -1;
				may_have_undef = 1;
				*may_have_extra_named_args = 1;
			}
		} else {
			if (uses_arg_unpack) {
				crex_error_noreturn(E_COMPILE_ERROR,
					"Cannot use positional argument after argument unpacking");
			}

			if (uses_named_args) {
				crex_error_noreturn(E_COMPILE_ERROR,
					"Cannot use positional argument after named argument");
			}

			arg_count++;
		}

		/* Treat passing of $GLOBALS the same as passing a call.
		 * This will error at runtime if the argument is by-ref. */
		if (crex_is_call(arg) || is_globals_fetch(arg)) {
			crex_compile_var(&arg_node, arg, BP_VAR_R, 0);
			if (arg_node.op_type & (IS_CONST|IS_TMP_VAR)) {
				/* Function call was converted into builtin instruction */
				if (!fbc || ARG_MUST_BE_SENT_BY_REF(fbc, arg_num)) {
					opcode = CREX_SEND_VAL_EX;
				} else {
					opcode = CREX_SEND_VAL;
				}
			} else {
				if (fbc && arg_num != (uint32_t) -1) {
					if (ARG_MUST_BE_SENT_BY_REF(fbc, arg_num)) {
						opcode = CREX_SEND_VAR_NO_REF;
					} else if (ARG_MAY_BE_SENT_BY_REF(fbc, arg_num)) {
						/* For IS_VAR operands, SEND_VAL will pass through the operand without
						 * dereferencing, so it will use a by-ref pass if the call returned by-ref
						 * and a by-value pass if it returned by-value. */
						opcode = CREX_SEND_VAL;
					} else {
						opcode = CREX_SEND_VAR;
					}
				} else {
					opcode = CREX_SEND_VAR_NO_REF_EX;
				}
			}
		} else if (crex_is_variable(arg) && !crex_ast_is_short_circuited(arg)) {
			if (fbc && arg_num != (uint32_t) -1) {
				if (ARG_SHOULD_BE_SENT_BY_REF(fbc, arg_num)) {
					crex_compile_var(&arg_node, arg, BP_VAR_W, 1);
					opcode = CREX_SEND_REF;
				} else {
					crex_compile_var(&arg_node, arg, BP_VAR_R, 0);
					opcode = (arg_node.op_type == IS_TMP_VAR) ? CREX_SEND_VAL : CREX_SEND_VAR;
				}
			} else {
				do {
					if (arg->kind == CREX_AST_VAR) {
						CG(crex_lineno) = crex_ast_get_lineno(ast);
						if (is_this_fetch(arg)) {
							crex_emit_op(&arg_node, CREX_FETCH_THIS, NULL, NULL);
							opcode = CREX_SEND_VAR_EX;
							CG(active_op_array)->fn_flags |= CREX_ACC_USES_THIS;
							break;
						} else if (crex_try_compile_cv(&arg_node, arg) == SUCCESS) {
							opcode = CREX_SEND_VAR_EX;
							break;
						}
					}
					opline = crex_emit_op(NULL, CREX_CHECK_FUNC_ARG, NULL, NULL);
					if (arg_name) {
						opline->op2_type = IS_CONST;
						crex_string_addref(arg_name);
						opline->op2.constant = crex_add_literal_string(&arg_name);
						opline->result.num = crex_alloc_cache_slots(2);
					} else {
						opline->op2.num = arg_num;
					}
					crex_compile_var(&arg_node, arg, BP_VAR_FUNC_ARG, 1);
					opcode = CREX_SEND_FUNC_ARG;
				} while (0);
			}
		} else {
			crex_compile_expr(&arg_node, arg);
			if (arg_node.op_type == IS_VAR) {
				/* pass ++$a or something similar */
				if (fbc && arg_num != (uint32_t) -1) {
					if (ARG_MUST_BE_SENT_BY_REF(fbc, arg_num)) {
						opcode = CREX_SEND_VAR_NO_REF;
					} else if (ARG_MAY_BE_SENT_BY_REF(fbc, arg_num)) {
						opcode = CREX_SEND_VAL;
					} else {
						opcode = CREX_SEND_VAR;
					}
				} else {
					opcode = CREX_SEND_VAR_NO_REF_EX;
				}
			} else if (arg_node.op_type == IS_CV) {
				if (fbc && arg_num != (uint32_t) -1) {
					if (ARG_SHOULD_BE_SENT_BY_REF(fbc, arg_num)) {
						opcode = CREX_SEND_REF;
					} else {
						opcode = CREX_SEND_VAR;
					}
				} else {
					opcode = CREX_SEND_VAR_EX;
				}
			} else {
				/* Delay "Only variables can be passed by reference" error to execution */
				if (fbc && arg_num != (uint32_t) -1 && !ARG_MUST_BE_SENT_BY_REF(fbc, arg_num)) {
					opcode = CREX_SEND_VAL;
				} else {
					opcode = CREX_SEND_VAL_EX;
				}
			}
		}

		opline = crex_emit_op(NULL, opcode, &arg_node, NULL);
		if (arg_name) {
			opline->op2_type = IS_CONST;
			crex_string_addref(arg_name);
			opline->op2.constant = crex_add_literal_string(&arg_name);
			opline->result.num = crex_alloc_cache_slots(2);
		} else {
			opline->op2.opline_num = arg_num;
			opline->result.var = EX_NUM_TO_VAR(arg_num - 1);
		}
	}

	if (may_have_undef) {
		crex_emit_op(NULL, CREX_CHECK_UNDEF_ARGS, NULL, NULL);
	}

	return arg_count;
}
/* }}} */

CREX_API uint8_t crex_get_call_op(const crex_op *init_op, crex_function *fbc) /* {{{ */
{
	if (fbc) {
		CREX_ASSERT(!(fbc->common.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE));
		if (fbc->type == CREX_INTERNAL_FUNCTION && !(CG(compiler_options) & CREX_COMPILE_IGNORE_INTERNAL_FUNCTIONS)) {
			if (init_op->opcode == CREX_INIT_FCALL && !crex_execute_internal) {
				if (!(fbc->common.fn_flags & CREX_ACC_DEPRECATED)) {
					return CREX_DO_ICALL;
				} else {
					return CREX_DO_FCALL_BY_NAME;
				}
			}
		} else if (!(CG(compiler_options) & CREX_COMPILE_IGNORE_USER_FUNCTIONS)){
			if (crex_execute_ex == execute_ex) {
				return CREX_DO_UCALL;
			}
		}
	} else if (crex_execute_ex == execute_ex &&
	           !crex_execute_internal &&
	           (init_op->opcode == CREX_INIT_FCALL_BY_NAME ||
	            init_op->opcode == CREX_INIT_NS_FCALL_BY_NAME)) {
		return CREX_DO_FCALL_BY_NAME;
	}
	return CREX_DO_FCALL;
}
/* }}} */

static bool crex_compile_call_common(znode *result, crex_ast *args_ast, crex_function *fbc, uint32_t lineno) /* {{{ */
{
	crex_op *opline;
	uint32_t opnum_init = get_next_op_number() - 1;

	if (args_ast->kind == CREX_AST_CALLABLE_CONVERT) {
		opline = &CG(active_op_array)->opcodes[opnum_init];
		opline->extended_value = 0;

		if (opline->opcode == CREX_NEW) {
		    crex_error_noreturn(E_COMPILE_ERROR, "Cannot create Closure for new expression");
		}

		if (opline->opcode == CREX_INIT_FCALL) {
			opline->op1.num = crex_vm_calc_used_stack(0, fbc);
		}

		crex_emit_op_tmp(result, CREX_CALLABLE_CONVERT, NULL, NULL);
		return true;
	}

	bool may_have_extra_named_args;
	uint32_t arg_count = crex_compile_args(args_ast, fbc, &may_have_extra_named_args);

	crex_do_extended_fcall_begin();

	opline = &CG(active_op_array)->opcodes[opnum_init];
	opline->extended_value = arg_count;

	if (opline->opcode == CREX_INIT_FCALL) {
		opline->op1.num = crex_vm_calc_used_stack(arg_count, fbc);
	}

	opline = crex_emit_op(result, crex_get_call_op(opline, fbc), NULL, NULL);
	if (may_have_extra_named_args) {
		opline->extended_value = CREX_FCALL_MAY_HAVE_EXTRA_NAMED_PARAMS;
	}
	opline->lineno = lineno;
	crex_do_extended_fcall_end();
	return false;
}
/* }}} */

static bool crex_compile_function_name(znode *name_node, crex_ast *name_ast) /* {{{ */
{
	crex_string *orig_name = crex_ast_get_str(name_ast);
	bool is_fully_qualified;

	name_node->op_type = IS_CONST;
	ZVAL_STR(&name_node->u.constant, crex_resolve_function_name(
		orig_name, name_ast->attr, &is_fully_qualified));

	return !is_fully_qualified && FC(current_namespace);
}
/* }}} */

static void crex_compile_ns_call(znode *result, znode *name_node, crex_ast *args_ast, uint32_t lineno) /* {{{ */
{
	crex_op *opline = get_next_op();
	opline->opcode = CREX_INIT_NS_FCALL_BY_NAME;
	opline->op2_type = IS_CONST;
	opline->op2.constant = crex_add_ns_func_name_literal(
		C_STR(name_node->u.constant));
	opline->result.num = crex_alloc_cache_slot();

	crex_compile_call_common(result, args_ast, NULL, lineno);
}
/* }}} */

static void crex_compile_dynamic_call(znode *result, znode *name_node, crex_ast *args_ast, uint32_t lineno) /* {{{ */
{
	if (name_node->op_type == IS_CONST && C_TYPE(name_node->u.constant) == IS_STRING) {
		const char *colon;
		crex_string *str = C_STR(name_node->u.constant);
		if ((colon = crex_memrchr(ZSTR_VAL(str), ':', ZSTR_LEN(str))) != NULL && colon > ZSTR_VAL(str) && *(colon - 1) == ':') {
			crex_string *class = crex_string_init(ZSTR_VAL(str), colon - ZSTR_VAL(str) - 1, 0);
			crex_string *method = crex_string_init(colon + 1, ZSTR_LEN(str) - (colon - ZSTR_VAL(str)) - 1, 0);
			crex_op *opline = get_next_op();

			opline->opcode = CREX_INIT_STATIC_METHOD_CALL;
			opline->op1_type = IS_CONST;
			opline->op1.constant = crex_add_class_name_literal(class);
			opline->op2_type = IS_CONST;
			opline->op2.constant = crex_add_func_name_literal(method);
			/* 2 slots, for class and method */
			opline->result.num = crex_alloc_cache_slots(2);
			zval_ptr_dtor(&name_node->u.constant);
		} else {
			crex_op *opline = get_next_op();

			opline->opcode = CREX_INIT_FCALL_BY_NAME;
			opline->op2_type = IS_CONST;
			opline->op2.constant = crex_add_func_name_literal(str);
			opline->result.num = crex_alloc_cache_slot();
		}
	} else {
		crex_emit_op(NULL, CREX_INIT_DYNAMIC_CALL, NULL, name_node);
	}

	crex_compile_call_common(result, args_ast, NULL, lineno);
}
/* }}} */

static inline bool crex_args_contain_unpack_or_named(crex_ast_list *args) /* {{{ */
{
	uint32_t i;
	for (i = 0; i < args->children; ++i) {
		crex_ast *arg = args->child[i];
		if (arg->kind == CREX_AST_UNPACK || arg->kind == CREX_AST_NAMED_ARG) {
			return 1;
		}
	}
	return 0;
}
/* }}} */

static crex_result crex_compile_func_strlen(znode *result, crex_ast_list *args) /* {{{ */
{
	znode arg_node;

	if (args->children != 1) {
		return FAILURE;
	}

	crex_compile_expr(&arg_node, args->child[0]);
	if (arg_node.op_type == IS_CONST && C_TYPE(arg_node.u.constant) == IS_STRING) {
		result->op_type = IS_CONST;
		ZVAL_LONG(&result->u.constant, C_STRLEN(arg_node.u.constant));
		zval_ptr_dtor_str(&arg_node.u.constant);
	} else {
		crex_emit_op_tmp(result, CREX_STRLEN, &arg_node, NULL);
	}
	return SUCCESS;
}
/* }}} */

static crex_result crex_compile_func_typecheck(znode *result, crex_ast_list *args, uint32_t type) /* {{{ */
{
	znode arg_node;
	crex_op *opline;

	if (args->children != 1) {
		return FAILURE;
	}

	crex_compile_expr(&arg_node, args->child[0]);
	opline = crex_emit_op_tmp(result, CREX_TYPE_CHECK, &arg_node, NULL);
	if (type != _IS_BOOL) {
		opline->extended_value = (1 << type);
	} else {
		opline->extended_value = (1 << IS_FALSE) | (1 << IS_TRUE);
	}
	return SUCCESS;
}
/* }}} */

static crex_result crex_compile_func_is_scalar(znode *result, crex_ast_list *args) /* {{{ */
{
	znode arg_node;
	crex_op *opline;

	if (args->children != 1) {
		return FAILURE;
	}

	crex_compile_expr(&arg_node, args->child[0]);
	opline = crex_emit_op_tmp(result, CREX_TYPE_CHECK, &arg_node, NULL);
	opline->extended_value = (1 << IS_FALSE | 1 << IS_TRUE | 1 << IS_DOUBLE | 1 << IS_LONG | 1 << IS_STRING);
	return SUCCESS;
}

static crex_result crex_compile_func_cast(znode *result, crex_ast_list *args, uint32_t type) /* {{{ */
{
	znode arg_node;
	crex_op *opline;

	if (args->children != 1) {
		return FAILURE;
	}

	crex_compile_expr(&arg_node, args->child[0]);
	if (type == _IS_BOOL) {
		opline = crex_emit_op_tmp(result, CREX_BOOL, &arg_node, NULL);
	} else {
		opline = crex_emit_op_tmp(result, CREX_CAST, &arg_node, NULL);
		opline->extended_value = type;
	}
	return SUCCESS;
}
/* }}} */

static crex_result crex_compile_func_defined(znode *result, crex_ast_list *args) /* {{{ */
{
	crex_string *name;
	crex_op *opline;

	if (args->children != 1 || args->child[0]->kind != CREX_AST_ZVAL) {
		return FAILURE;
	}

	name = zval_get_string(crex_ast_get_zval(args->child[0]));
	if (crex_memrchr(ZSTR_VAL(name), '\\', ZSTR_LEN(name)) || crex_memrchr(ZSTR_VAL(name), ':', ZSTR_LEN(name))) {
		crex_string_release_ex(name, 0);
		return FAILURE;
	}

	if (crex_try_ct_eval_const(&result->u.constant, name, 0)) {
		crex_string_release_ex(name, 0);
		zval_ptr_dtor(&result->u.constant);
		ZVAL_TRUE(&result->u.constant);
		result->op_type = IS_CONST;
		return SUCCESS;
	}

	opline = crex_emit_op_tmp(result, CREX_DEFINED, NULL, NULL);
	opline->op1_type = IS_CONST;
	LITERAL_STR(opline->op1, name);
	opline->extended_value = crex_alloc_cache_slot();

	return SUCCESS;
}
/* }}} */

static crex_result crex_compile_func_chr(znode *result, crex_ast_list *args) /* {{{ */
{

	if (args->children == 1 &&
	    args->child[0]->kind == CREX_AST_ZVAL &&
	    C_TYPE_P(crex_ast_get_zval(args->child[0])) == IS_LONG) {

		crex_long c = C_LVAL_P(crex_ast_get_zval(args->child[0])) & 0xff;

		result->op_type = IS_CONST;
		ZVAL_CHAR(&result->u.constant, c);
		return SUCCESS;
	} else {
		return FAILURE;
	}
}
/* }}} */

static crex_result crex_compile_func_ord(znode *result, crex_ast_list *args) /* {{{ */
{
	if (args->children == 1 &&
	    args->child[0]->kind == CREX_AST_ZVAL &&
	    C_TYPE_P(crex_ast_get_zval(args->child[0])) == IS_STRING) {

		result->op_type = IS_CONST;
		ZVAL_LONG(&result->u.constant, (unsigned char)C_STRVAL_P(crex_ast_get_zval(args->child[0]))[0]);
		return SUCCESS;
	} else {
		return FAILURE;
	}
}
/* }}} */

/* We can only calculate the stack size for functions that have been fully compiled, otherwise
 * additional CV or TMP slots may still be added. This prevents the use of INIT_FCALL for
 * directly or indirectly recursive function calls. */
static bool fbc_is_finalized(crex_function *fbc) {
	return !CREX_USER_CODE(fbc->type) || (fbc->common.fn_flags & CREX_ACC_DONE_PASS_TWO);
}

static crex_result crex_try_compile_ct_bound_init_user_func(crex_ast *name_ast, uint32_t num_args) /* {{{ */
{
	crex_string *name, *lcname;
	crex_function *fbc;
	crex_op *opline;

	if (name_ast->kind != CREX_AST_ZVAL || C_TYPE_P(crex_ast_get_zval(name_ast)) != IS_STRING) {
		return FAILURE;
	}

	name = crex_ast_get_str(name_ast);
	lcname = crex_string_tolower(name);

	fbc = crex_hash_find_ptr(CG(function_table), lcname);
	if (!fbc || !fbc_is_finalized(fbc)
	 || (fbc->type == CREX_INTERNAL_FUNCTION && (CG(compiler_options) & CREX_COMPILE_IGNORE_INTERNAL_FUNCTIONS))
	 || (fbc->type == CREX_USER_FUNCTION && (CG(compiler_options) & CREX_COMPILE_IGNORE_USER_FUNCTIONS))
	 || (fbc->type == CREX_USER_FUNCTION && (CG(compiler_options) & CREX_COMPILE_IGNORE_OTHER_FILES) && fbc->op_array.filename != CG(active_op_array)->filename)
	) {
		crex_string_release_ex(lcname, 0);
		return FAILURE;
	}

	opline = crex_emit_op(NULL, CREX_INIT_FCALL, NULL, NULL);
	opline->extended_value = num_args;
	opline->op1.num = crex_vm_calc_used_stack(num_args, fbc);
	opline->op2_type = IS_CONST;
	LITERAL_STR(opline->op2, lcname);
	opline->result.num = crex_alloc_cache_slot();

	return SUCCESS;
}
/* }}} */

static void crex_compile_init_user_func(crex_ast *name_ast, uint32_t num_args, crex_string *orig_func_name) /* {{{ */
{
	crex_op *opline;
	znode name_node;

	if (crex_try_compile_ct_bound_init_user_func(name_ast, num_args) == SUCCESS) {
		return;
	}

	crex_compile_expr(&name_node, name_ast);

	opline = crex_emit_op(NULL, CREX_INIT_USER_CALL, NULL, &name_node);
	opline->op1_type = IS_CONST;
	LITERAL_STR(opline->op1, crex_string_copy(orig_func_name));
	opline->extended_value = num_args;
}
/* }}} */

/* cufa = call_user_func_array */
static crex_result crex_compile_func_cufa(znode *result, crex_ast_list *args, crex_string *lcname) /* {{{ */
{
	znode arg_node;
	crex_op *opline;

	if (args->children != 2) {
		return FAILURE;
	}

	crex_compile_init_user_func(args->child[0], 0, lcname);
	if (args->child[1]->kind == CREX_AST_CALL
	 && args->child[1]->child[0]->kind == CREX_AST_ZVAL
	 && C_TYPE_P(crex_ast_get_zval(args->child[1]->child[0])) == IS_STRING
	 && args->child[1]->child[1]->kind == CREX_AST_ARG_LIST) {
		crex_string *orig_name = crex_ast_get_str(args->child[1]->child[0]);
		crex_ast_list *list = crex_ast_get_list(args->child[1]->child[1]);
		bool is_fully_qualified;
		crex_string *name = crex_resolve_function_name(orig_name, args->child[1]->child[0]->attr, &is_fully_qualified);

		if (crex_string_equals_literal_ci(name, "array_slice")
	     && !crex_args_contain_unpack_or_named(list)
		 && list->children == 3
		 && list->child[1]->kind == CREX_AST_ZVAL) {
			zval *zv = crex_ast_get_zval(list->child[1]);

			if (C_TYPE_P(zv) == IS_LONG
			 && C_LVAL_P(zv) >= 0
			 && C_LVAL_P(zv) <= 0x7fffffff) {
				crex_op *opline;
				znode len_node;

				crex_compile_expr(&arg_node, list->child[0]);
				crex_compile_expr(&len_node, list->child[2]);
				opline = crex_emit_op(NULL, CREX_SEND_ARRAY, &arg_node, &len_node);
				opline->extended_value = C_LVAL_P(zv);
				crex_emit_op(result, CREX_DO_FCALL, NULL, NULL);
				crex_string_release_ex(name, 0);
				return SUCCESS;
			}
		}
		crex_string_release_ex(name, 0);
	}
	crex_compile_expr(&arg_node, args->child[1]);
	crex_emit_op(NULL, CREX_SEND_ARRAY, &arg_node, NULL);
	crex_emit_op(NULL, CREX_CHECK_UNDEF_ARGS, NULL, NULL);
	opline = crex_emit_op(result, CREX_DO_FCALL, NULL, NULL);
	opline->extended_value = CREX_FCALL_MAY_HAVE_EXTRA_NAMED_PARAMS;

	return SUCCESS;
}
/* }}} */

/* cuf = call_user_func */
static crex_result crex_compile_func_cuf(znode *result, crex_ast_list *args, crex_string *lcname) /* {{{ */
{
	uint32_t i;

	if (args->children < 1) {
		return FAILURE;
	}

	crex_compile_init_user_func(args->child[0], args->children - 1, lcname);
	for (i = 1; i < args->children; ++i) {
		crex_ast *arg_ast = args->child[i];
		znode arg_node;
		crex_op *opline;

		crex_compile_expr(&arg_node, arg_ast);

		opline = crex_emit_op(NULL, CREX_SEND_USER, &arg_node, NULL);
		opline->op2.num = i;
		opline->result.var = EX_NUM_TO_VAR(i - 1);
	}
	crex_emit_op(result, CREX_DO_FCALL, NULL, NULL);

	return SUCCESS;
}
/* }}} */

static void crex_compile_assert(znode *result, crex_ast_list *args, crex_string *name, crex_function *fbc, uint32_t lineno) /* {{{ */
{
	if (EG(assertions) >= 0) {
		znode name_node;
		crex_op *opline;
		uint32_t check_op_number = get_next_op_number();

		crex_emit_op(NULL, CREX_ASSERT_CHECK, NULL, NULL);

		if (fbc && fbc_is_finalized(fbc)) {
			name_node.op_type = IS_CONST;
			ZVAL_STR_COPY(&name_node.u.constant, name);

			opline = crex_emit_op(NULL, CREX_INIT_FCALL, NULL, &name_node);
		} else {
			opline = crex_emit_op(NULL, CREX_INIT_NS_FCALL_BY_NAME, NULL, NULL);
			opline->op2_type = IS_CONST;
			opline->op2.constant = crex_add_ns_func_name_literal(name);
		}
		opline->result.num = crex_alloc_cache_slot();

		if (args->children == 1) {
			/* add "assert(condition) as assertion message */
			crex_ast *arg = crex_ast_create_zval_from_str(
				crex_ast_export("assert(", args->child[0], ")"));
			if (args->child[0]->kind == CREX_AST_NAMED_ARG) {
				/* If the original argument was named, add the new argument as named as well,
				 * as mixing named and positional is not allowed. */
				crex_ast *name = crex_ast_create_zval_from_str(
					ZSTR_INIT_LITERAL("description", 0));
				arg = crex_ast_create(CREX_AST_NAMED_ARG, name, arg);
			}
			crex_ast_list_add((crex_ast *) args, arg);
		}

		crex_compile_call_common(result, (crex_ast*)args, fbc, lineno);

		opline = &CG(active_op_array)->opcodes[check_op_number];
		opline->op2.opline_num = get_next_op_number();
		SET_NODE(opline->result, result);
	} else {
		if (!fbc) {
			crex_string_release_ex(name, 0);
		}
		result->op_type = IS_CONST;
		ZVAL_TRUE(&result->u.constant);
	}
}
/* }}} */

static crex_result crex_compile_func_in_array(znode *result, crex_ast_list *args) /* {{{ */
{
	bool strict = 0;
	znode array, needly;
	crex_op *opline;

	if (args->children == 3) {
		if (args->child[2]->kind == CREX_AST_ZVAL) {
			strict = crex_is_true(crex_ast_get_zval(args->child[2]));
		} else if (args->child[2]->kind == CREX_AST_CONST) {
			zval value;
			crex_ast *name_ast = args->child[2]->child[0];
			bool is_fully_qualified;
			crex_string *resolved_name = crex_resolve_const_name(
				crex_ast_get_str(name_ast), name_ast->attr, &is_fully_qualified);

			if (!crex_try_ct_eval_const(&value, resolved_name, is_fully_qualified)) {
				crex_string_release_ex(resolved_name, 0);
				return FAILURE;
			}

			crex_string_release_ex(resolved_name, 0);
			strict = crex_is_true(&value);
			zval_ptr_dtor(&value);
		} else {
			return FAILURE;
		}
	} else if (args->children != 2) {
		return FAILURE;
	}

	if (args->child[1]->kind != CREX_AST_ARRAY
	 || !crex_try_ct_eval_array(&array.u.constant, args->child[1])) {
		return FAILURE;
	}

	if (crex_hash_num_elements(C_ARRVAL(array.u.constant)) > 0) {
		bool ok = 1;
		zval *val, tmp;
		HashTable *src = C_ARRVAL(array.u.constant);
		HashTable *dst = crex_new_array(crex_hash_num_elements(src));

		ZVAL_TRUE(&tmp);

		if (strict) {
			CREX_HASH_FOREACH_VAL(src, val) {
				if (C_TYPE_P(val) == IS_STRING) {
					crex_hash_add(dst, C_STR_P(val), &tmp);
				} else if (C_TYPE_P(val) == IS_LONG) {
					crex_hash_index_add(dst, C_LVAL_P(val), &tmp);
				} else {
					crex_array_destroy(dst);
					ok = 0;
					break;
				}
			} CREX_HASH_FOREACH_END();
		} else {
			CREX_HASH_FOREACH_VAL(src, val) {
				if (C_TYPE_P(val) != IS_STRING
				 || is_numeric_string(C_STRVAL_P(val), C_STRLEN_P(val), NULL, NULL, 0)) {
					crex_array_destroy(dst);
					ok = 0;
					break;
				}
				crex_hash_add(dst, C_STR_P(val), &tmp);
			} CREX_HASH_FOREACH_END();
		}

		crex_array_destroy(src);
		if (!ok) {
			return FAILURE;
		}
		C_ARRVAL(array.u.constant) = dst;
	}
	array.op_type = IS_CONST;

	crex_compile_expr(&needly, args->child[0]);

	opline = crex_emit_op_tmp(result, CREX_IN_ARRAY, &needly, &array);
	opline->extended_value = strict;

	return SUCCESS;
}
/* }}} */

static crex_result crex_compile_func_count(znode *result, crex_ast_list *args, crex_string *lcname) /* {{{ */
{
	znode arg_node;
	crex_op *opline;

	if (args->children != 1) {
		return FAILURE;
	}

	crex_compile_expr(&arg_node, args->child[0]);
	opline = crex_emit_op_tmp(result, CREX_COUNT, &arg_node, NULL);
	opline->extended_value = crex_string_equals_literal(lcname, "sizeof");

	return SUCCESS;
}
/* }}} */

static crex_result crex_compile_func_get_class(znode *result, crex_ast_list *args) /* {{{ */
{
	if (args->children == 0) {
		crex_emit_op_tmp(result, CREX_GET_CLASS, NULL, NULL);
	} else {
		znode arg_node;

		if (args->children != 1) {
			return FAILURE;
		}

		crex_compile_expr(&arg_node, args->child[0]);
		crex_emit_op_tmp(result, CREX_GET_CLASS, &arg_node, NULL);
	}
	return SUCCESS;
}
/* }}} */

static crex_result crex_compile_func_get_called_class(znode *result, crex_ast_list *args) /* {{{ */
{
	if (args->children != 0) {
		return FAILURE;
	}

	crex_emit_op_tmp(result, CREX_GET_CALLED_CLASS, NULL, NULL);
	return SUCCESS;
}
/* }}} */

static crex_result crex_compile_func_gettype(znode *result, crex_ast_list *args) /* {{{ */
{
	znode arg_node;

	if (args->children != 1) {
		return FAILURE;
	}

	crex_compile_expr(&arg_node, args->child[0]);
	crex_emit_op_tmp(result, CREX_GET_TYPE, &arg_node, NULL);
	return SUCCESS;
}
/* }}} */

static crex_result crex_compile_func_num_args(znode *result, crex_ast_list *args) /* {{{ */
{
	if (CG(active_op_array)->function_name && args->children == 0) {
		crex_emit_op_tmp(result, CREX_FUNC_NUM_ARGS, NULL, NULL);
		return SUCCESS;
	} else {
		return FAILURE;
	}
}
/* }}} */

static crex_result crex_compile_func_get_args(znode *result, crex_ast_list *args) /* {{{ */
{
	if (CG(active_op_array)->function_name && args->children == 0) {
		crex_emit_op_tmp(result, CREX_FUNC_GET_ARGS, NULL, NULL);
		return SUCCESS;
	} else {
		return FAILURE;
	}
}
/* }}} */

static crex_result crex_compile_func_array_key_exists(znode *result, crex_ast_list *args) /* {{{ */
{
	znode subject, needle;

	if (args->children != 2) {
		return FAILURE;
	}

	crex_compile_expr(&needle, args->child[0]);
	crex_compile_expr(&subject, args->child[1]);

	crex_emit_op_tmp(result, CREX_ARRAY_KEY_EXISTS, &needle, &subject);
	return SUCCESS;
}
/* }}} */

static crex_result crex_compile_func_array_slice(znode *result, crex_ast_list *args) /* {{{ */
{
	if (CG(active_op_array)->function_name
	 && args->children == 2
	 && args->child[0]->kind == CREX_AST_CALL
	 && args->child[0]->child[0]->kind == CREX_AST_ZVAL
	 && C_TYPE_P(crex_ast_get_zval(args->child[0]->child[0])) == IS_STRING
	 && args->child[0]->child[1]->kind == CREX_AST_ARG_LIST
	 && args->child[1]->kind == CREX_AST_ZVAL) {

		crex_string *orig_name = crex_ast_get_str(args->child[0]->child[0]);
		bool is_fully_qualified;
		crex_string *name = crex_resolve_function_name(orig_name, args->child[0]->child[0]->attr, &is_fully_qualified);
		crex_ast_list *list = crex_ast_get_list(args->child[0]->child[1]);
		zval *zv = crex_ast_get_zval(args->child[1]);
		znode first;

		if (crex_string_equals_literal_ci(name, "func_get_args")
		 && list->children == 0
		 && C_TYPE_P(zv) == IS_LONG
		 && C_LVAL_P(zv) >= 0) {
			first.op_type = IS_CONST;
			ZVAL_LONG(&first.u.constant, C_LVAL_P(zv));
			crex_emit_op_tmp(result, CREX_FUNC_GET_ARGS, &first, NULL);
			crex_string_release_ex(name, 0);
			return SUCCESS;
		}
		crex_string_release_ex(name, 0);
	}
	return FAILURE;
}
/* }}} */

static crex_result crex_try_compile_special_func(znode *result, crex_string *lcname, crex_ast_list *args, crex_function *fbc, uint32_t type) /* {{{ */
{
	if (CG(compiler_options) & CREX_COMPILE_NO_BUILTINS) {
		return FAILURE;
	}

	if (fbc->type != CREX_INTERNAL_FUNCTION) {
		/* If the function is part of disabled_functions, it may be redeclared as a userland
		 * function with a different implementation. Don't use the VM builtin in that case. */
		return FAILURE;
	}

	if (crex_args_contain_unpack_or_named(args)) {
		return FAILURE;
	}

	if (crex_string_equals_literal(lcname, "strlen")) {
		return crex_compile_func_strlen(result, args);
	} else if (crex_string_equals_literal(lcname, "is_null")) {
		return crex_compile_func_typecheck(result, args, IS_NULL);
	} else if (crex_string_equals_literal(lcname, "is_bool")) {
		return crex_compile_func_typecheck(result, args, _IS_BOOL);
	} else if (crex_string_equals_literal(lcname, "is_long")
		|| crex_string_equals_literal(lcname, "is_int")
		|| crex_string_equals_literal(lcname, "is_integer")
	) {
		return crex_compile_func_typecheck(result, args, IS_LONG);
	} else if (crex_string_equals_literal(lcname, "is_float")
		|| crex_string_equals_literal(lcname, "is_double")
	) {
		return crex_compile_func_typecheck(result, args, IS_DOUBLE);
	} else if (crex_string_equals_literal(lcname, "is_string")) {
		return crex_compile_func_typecheck(result, args, IS_STRING);
	} else if (crex_string_equals_literal(lcname, "is_array")) {
		return crex_compile_func_typecheck(result, args, IS_ARRAY);
	} else if (crex_string_equals_literal(lcname, "is_object")) {
		return crex_compile_func_typecheck(result, args, IS_OBJECT);
	} else if (crex_string_equals_literal(lcname, "is_resource")) {
		return crex_compile_func_typecheck(result, args, IS_RESOURCE);
	} else if (crex_string_equals_literal(lcname, "is_scalar")) {
		return crex_compile_func_is_scalar(result, args);
	} else if (crex_string_equals_literal(lcname, "boolval")) {
		return crex_compile_func_cast(result, args, _IS_BOOL);
	} else if (crex_string_equals_literal(lcname, "intval")) {
		return crex_compile_func_cast(result, args, IS_LONG);
	} else if (crex_string_equals_literal(lcname, "floatval")
		|| crex_string_equals_literal(lcname, "doubleval")
	) {
		return crex_compile_func_cast(result, args, IS_DOUBLE);
	} else if (crex_string_equals_literal(lcname, "strval")) {
		return crex_compile_func_cast(result, args, IS_STRING);
	} else if (crex_string_equals_literal(lcname, "defined")) {
		return crex_compile_func_defined(result, args);
	} else if (crex_string_equals_literal(lcname, "chr") && type == BP_VAR_R) {
		return crex_compile_func_chr(result, args);
	} else if (crex_string_equals_literal(lcname, "ord") && type == BP_VAR_R) {
		return crex_compile_func_ord(result, args);
	} else if (crex_string_equals_literal(lcname, "call_user_func_array")) {
		return crex_compile_func_cufa(result, args, lcname);
	} else if (crex_string_equals_literal(lcname, "call_user_func")) {
		return crex_compile_func_cuf(result, args, lcname);
	} else if (crex_string_equals_literal(lcname, "in_array")) {
		return crex_compile_func_in_array(result, args);
	} else if (crex_string_equals(lcname, ZSTR_KNOWN(CREX_STR_COUNT))
			|| crex_string_equals_literal(lcname, "sizeof")) {
		return crex_compile_func_count(result, args, lcname);
	} else if (crex_string_equals_literal(lcname, "get_class")) {
		return crex_compile_func_get_class(result, args);
	} else if (crex_string_equals_literal(lcname, "get_called_class")) {
		return crex_compile_func_get_called_class(result, args);
	} else if (crex_string_equals_literal(lcname, "gettype")) {
		return crex_compile_func_gettype(result, args);
	} else if (crex_string_equals_literal(lcname, "func_num_args")) {
		return crex_compile_func_num_args(result, args);
	} else if (crex_string_equals_literal(lcname, "func_get_args")) {
		return crex_compile_func_get_args(result, args);
	} else if (crex_string_equals_literal(lcname, "array_slice")) {
		return crex_compile_func_array_slice(result, args);
	} else if (crex_string_equals_literal(lcname, "array_key_exists")) {
		return crex_compile_func_array_key_exists(result, args);
	} else {
		return FAILURE;
	}
}
/* }}} */

static void crex_compile_call(znode *result, crex_ast *ast, uint32_t type) /* {{{ */
{
	crex_ast *name_ast = ast->child[0];
	crex_ast *args_ast = ast->child[1];
	bool is_callable_convert = args_ast->kind == CREX_AST_CALLABLE_CONVERT;

	znode name_node;

	if (name_ast->kind != CREX_AST_ZVAL || C_TYPE_P(crex_ast_get_zval(name_ast)) != IS_STRING) {
		crex_compile_expr(&name_node, name_ast);
		crex_compile_dynamic_call(result, &name_node, args_ast, ast->lineno);
		return;
	}

	{
		bool runtime_resolution = crex_compile_function_name(&name_node, name_ast);
		if (runtime_resolution) {
			if (crex_string_equals_literal_ci(crex_ast_get_str(name_ast), "assert")
					&& !is_callable_convert) {
				crex_compile_assert(result, crex_ast_get_list(args_ast), C_STR(name_node.u.constant), NULL, ast->lineno);
			} else {
				crex_compile_ns_call(result, &name_node, args_ast, ast->lineno);
			}
			return;
		}
	}

	{
		zval *name = &name_node.u.constant;
		crex_string *lcname;
		crex_function *fbc;
		crex_op *opline;

		lcname = crex_string_tolower(C_STR_P(name));
		fbc = crex_hash_find_ptr(CG(function_table), lcname);

		/* Special assert() handling should apply independently of compiler flags. */
		if (fbc && crex_string_equals_literal(lcname, "assert") && !is_callable_convert) {
			crex_compile_assert(result, crex_ast_get_list(args_ast), lcname, fbc, ast->lineno);
			crex_string_release(lcname);
			zval_ptr_dtor(&name_node.u.constant);
			return;
		}

		if (!fbc || !fbc_is_finalized(fbc)
		 || (fbc->type == CREX_INTERNAL_FUNCTION && (CG(compiler_options) & CREX_COMPILE_IGNORE_INTERNAL_FUNCTIONS))
		 || (fbc->type == CREX_USER_FUNCTION && (CG(compiler_options) & CREX_COMPILE_IGNORE_USER_FUNCTIONS))
		 || (fbc->type == CREX_USER_FUNCTION && (CG(compiler_options) & CREX_COMPILE_IGNORE_OTHER_FILES) && fbc->op_array.filename != CG(active_op_array)->filename)
		) {
			crex_string_release_ex(lcname, 0);
			crex_compile_dynamic_call(result, &name_node, args_ast, ast->lineno);
			return;
		}

		if (!is_callable_convert &&
		    crex_try_compile_special_func(result, lcname,
				crex_ast_get_list(args_ast), fbc, type) == SUCCESS
		) {
			crex_string_release_ex(lcname, 0);
			zval_ptr_dtor(&name_node.u.constant);
			return;
		}

		zval_ptr_dtor(&name_node.u.constant);
		ZVAL_NEW_STR(&name_node.u.constant, lcname);

		opline = crex_emit_op(NULL, CREX_INIT_FCALL, NULL, &name_node);
		opline->result.num = crex_alloc_cache_slot();

		crex_compile_call_common(result, args_ast, fbc, ast->lineno);
	}
}
/* }}} */

static void crex_compile_method_call(znode *result, crex_ast *ast, uint32_t type) /* {{{ */
{
	crex_ast *obj_ast = ast->child[0];
	crex_ast *method_ast = ast->child[1];
	crex_ast *args_ast = ast->child[2];

	znode obj_node, method_node;
	crex_op *opline;
	crex_function *fbc = NULL;
	bool nullsafe = ast->kind == CREX_AST_NULLSAFE_METHOD_CALL;
	uint32_t short_circuiting_checkpoint = crex_short_circuiting_checkpoint();

	if (is_this_fetch(obj_ast)) {
		if (this_guaranteed_exists()) {
			obj_node.op_type = IS_UNUSED;
		} else {
			crex_emit_op(&obj_node, CREX_FETCH_THIS, NULL, NULL);
		}
		CG(active_op_array)->fn_flags |= CREX_ACC_USES_THIS;

		/* We will throw if $this doesn't exist, so there's no need to emit a JMP_NULL
		 * check for a nullsafe access. */
	} else {
		crex_short_circuiting_mark_inner(obj_ast);
		crex_compile_expr(&obj_node, obj_ast);
		if (nullsafe) {
			crex_emit_jmp_null(&obj_node, type);
		}
	}

	crex_compile_expr(&method_node, method_ast);
	opline = crex_emit_op(NULL, CREX_INIT_METHOD_CALL, &obj_node, NULL);

	if (method_node.op_type == IS_CONST) {
		if (C_TYPE(method_node.u.constant) != IS_STRING) {
			crex_error_noreturn(E_COMPILE_ERROR, "Method name must be a string");
		}

		opline->op2_type = IS_CONST;
		opline->op2.constant = crex_add_func_name_literal(
			C_STR(method_node.u.constant));
		opline->result.num = crex_alloc_cache_slots(2);
	} else {
		SET_NODE(opline->op2, &method_node);
	}

	/* Check if this calls a known method on $this */
	if (opline->op1_type == IS_UNUSED && opline->op2_type == IS_CONST &&
			CG(active_class_entry) && crex_is_scope_known()) {
		crex_string *lcname = C_STR_P(CT_CONSTANT(opline->op2) + 1);
		fbc = crex_hash_find_ptr(&CG(active_class_entry)->function_table, lcname);

		/* We only know the exact method that is being called if it is either private or final.
		 * Otherwise an overriding method in a child class may be called. */
		if (fbc && !(fbc->common.fn_flags & (CREX_ACC_PRIVATE|CREX_ACC_FINAL))) {
			fbc = NULL;
		}
	}

	if (crex_compile_call_common(result, args_ast, fbc, crex_ast_get_lineno(method_ast))) {
		if (short_circuiting_checkpoint != crex_short_circuiting_checkpoint()) {
			crex_error_noreturn(E_COMPILE_ERROR,
				"Cannot combine nullsafe operator with Closure creation");
		}
	}
}
/* }}} */

static bool crex_is_constructor(crex_string *name) /* {{{ */
{
	return crex_string_equals_literal_ci(name, CREX_CONSTRUCTOR_FUNC_NAME);
}
/* }}} */

static crex_function *crex_get_compatible_func_or_null(crex_class_entry *ce, crex_string *lcname) /* {{{ */
{
	crex_function *fbc = crex_hash_find_ptr(&ce->function_table, lcname);
	if (!fbc || (fbc->common.fn_flags & CREX_ACC_PUBLIC) || ce == CG(active_class_entry)) {
		return fbc;
	}

	if (!(fbc->common.fn_flags & CREX_ACC_PRIVATE)
		&& (fbc->common.scope->ce_flags & CREX_ACC_LINKED)
		&& (!CG(active_class_entry) || (CG(active_class_entry)->ce_flags & CREX_ACC_LINKED))
		&& crex_check_protected(crex_get_function_root_class(fbc), CG(active_class_entry))) {
		return fbc;
	}

	return NULL;
}
/* }}} */

static void crex_compile_static_call(znode *result, crex_ast *ast, uint32_t type) /* {{{ */
{
	crex_ast *class_ast = ast->child[0];
	crex_ast *method_ast = ast->child[1];
	crex_ast *args_ast = ast->child[2];

	znode class_node, method_node;
	crex_op *opline;
	crex_function *fbc = NULL;

	crex_short_circuiting_mark_inner(class_ast);
	crex_compile_class_ref(&class_node, class_ast, CREX_FETCH_CLASS_EXCEPTION);

	crex_compile_expr(&method_node, method_ast);

	if (method_node.op_type == IS_CONST) {
		zval *name = &method_node.u.constant;
		if (C_TYPE_P(name) != IS_STRING) {
			crex_error_noreturn(E_COMPILE_ERROR, "Method name must be a string");
		}
		if (crex_is_constructor(C_STR_P(name))) {
			zval_ptr_dtor(name);
			method_node.op_type = IS_UNUSED;
		}
	}

	opline = get_next_op();
	opline->opcode = CREX_INIT_STATIC_METHOD_CALL;

	crex_set_class_name_op1(opline, &class_node);

	if (method_node.op_type == IS_CONST) {
		opline->op2_type = IS_CONST;
		opline->op2.constant = crex_add_func_name_literal(
			C_STR(method_node.u.constant));
		opline->result.num = crex_alloc_cache_slots(2);
	} else {
		if (opline->op1_type == IS_CONST) {
			opline->result.num = crex_alloc_cache_slot();
		}
		SET_NODE(opline->op2, &method_node);
	}

	/* Check if we already know which method we're calling */
	if (opline->op2_type == IS_CONST) {
		crex_class_entry *ce = NULL;
		if (opline->op1_type == IS_CONST) {
			crex_string *lcname = C_STR_P(CT_CONSTANT(opline->op1) + 1);
			ce = crex_hash_find_ptr(CG(class_table), lcname);
			if (!ce && CG(active_class_entry)
					&& crex_string_equals_ci(CG(active_class_entry)->name, lcname)) {
				ce = CG(active_class_entry);
			}
		} else if (opline->op1_type == IS_UNUSED
				&& (opline->op1.num & CREX_FETCH_CLASS_MASK) == CREX_FETCH_CLASS_SELF
				&& crex_is_scope_known()) {
			ce = CG(active_class_entry);
		}
		if (ce) {
			crex_string *lcname = C_STR_P(CT_CONSTANT(opline->op2) + 1);
			fbc = crex_get_compatible_func_or_null(ce, lcname);
		}
	}

	crex_compile_call_common(result, args_ast, fbc, crex_ast_get_lineno(method_ast));
}
/* }}} */

static void crex_compile_class_decl(znode *result, crex_ast *ast, bool toplevel);

static void crex_compile_new(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *class_ast = ast->child[0];
	crex_ast *args_ast = ast->child[1];

	znode class_node, ctor_result;
	crex_op *opline;

	if (class_ast->kind == CREX_AST_CLASS) {
		/* anon class declaration */
		crex_compile_class_decl(&class_node, class_ast, 0);
	} else {
		crex_compile_class_ref(&class_node, class_ast, CREX_FETCH_CLASS_EXCEPTION);
	}

	opline = crex_emit_op(result, CREX_NEW, NULL, NULL);

	if (class_node.op_type == IS_CONST) {
		opline->op1_type = IS_CONST;
		opline->op1.constant = crex_add_class_name_literal(
			C_STR(class_node.u.constant));
		opline->op2.num = crex_alloc_cache_slot();
	} else {
		SET_NODE(opline->op1, &class_node);
	}

	crex_compile_call_common(&ctor_result, args_ast, NULL, ast->lineno);
	crex_do_free(&ctor_result);
}
/* }}} */

static void crex_compile_clone(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *obj_ast = ast->child[0];

	znode obj_node;
	crex_compile_expr(&obj_node, obj_ast);

	crex_emit_op_tmp(result, CREX_CLONE, &obj_node, NULL);
}
/* }}} */

static void crex_compile_global_var(crex_ast *ast) /* {{{ */
{
	crex_ast *var_ast = ast->child[0];
	crex_ast *name_ast = var_ast->child[0];

	znode name_node, result;

	crex_compile_expr(&name_node, name_ast);
	if (name_node.op_type == IS_CONST) {
		convert_to_string(&name_node.u.constant);
	}

	// TODO(GLOBALS) Forbid "global $GLOBALS"?
	if (is_this_fetch(var_ast)) {
		crex_error_noreturn(E_COMPILE_ERROR, "Cannot use $this as global variable");
	} else if (crex_try_compile_cv(&result, var_ast) == SUCCESS) {
		crex_op *opline = crex_emit_op(NULL, CREX_BIND_GLOBAL, &result, &name_node);
		opline->extended_value = crex_alloc_cache_slot();
	} else {
		/* name_ast should be evaluated only. FETCH_GLOBAL_LOCK instructs FETCH_W
		 * to not free the name_node operand, so it can be reused in the following
		 * ASSIGN_REF, which then frees it. */
		crex_op *opline = crex_emit_op(&result, CREX_FETCH_W, &name_node, NULL);
		opline->extended_value = CREX_FETCH_GLOBAL_LOCK;

		if (name_node.op_type == IS_CONST) {
			crex_string_addref(C_STR(name_node.u.constant));
		}

		crex_emit_assign_ref_znode(
			crex_ast_create(CREX_AST_VAR, crex_ast_create_znode(&name_node)),
			&result
		);
	}
}
/* }}} */

static void crex_compile_static_var_common(crex_string *var_name, zval *value, uint32_t mode) /* {{{ */
{
	crex_op *opline;
	if (!CG(active_op_array)->static_variables) {
		if (CG(active_op_array)->scope) {
			CG(active_op_array)->scope->ce_flags |= CREX_HAS_STATIC_IN_METHODS;
		}
		CG(active_op_array)->static_variables = crex_new_array(8);
	}

	value = crex_hash_update(CG(active_op_array)->static_variables, var_name, value);

	if (crex_string_equals(var_name, ZSTR_KNOWN(CREX_STR_THIS))) {
		crex_error_noreturn(E_COMPILE_ERROR, "Cannot use $this as static variable");
	}

	opline = crex_emit_op(NULL, CREX_BIND_STATIC, NULL, NULL);
	opline->op1_type = IS_CV;
	opline->op1.var = lookup_cv(var_name);
	opline->extended_value = (uint32_t)((char*)value - (char*)CG(active_op_array)->static_variables->arData) | mode;
}
/* }}} */

static void crex_compile_static_var(crex_ast *ast) /* {{{ */
{
	crex_ast *var_ast = ast->child[0];
	crex_string *var_name = crex_ast_get_str(var_ast);

	if (crex_string_equals(var_name, ZSTR_KNOWN(CREX_STR_THIS))) {
		crex_error_noreturn(E_COMPILE_ERROR, "Cannot use $this as static variable");
	}

	if (!CG(active_op_array)->static_variables) {
		if (CG(active_op_array)->scope) {
			CG(active_op_array)->scope->ce_flags |= CREX_HAS_STATIC_IN_METHODS;
		}
		CG(active_op_array)->static_variables = crex_new_array(8);
	}

	if (crex_hash_exists(CG(active_op_array)->static_variables, var_name)) {
		crex_error_noreturn(E_COMPILE_ERROR, "Duplicate declaration of static variable $%s", ZSTR_VAL(var_name));
	}

	crex_eval_const_expr(&ast->child[1]);
	crex_ast *value_ast = ast->child[1];

	if (!value_ast || value_ast->kind == CREX_AST_ZVAL) {
		zval *value_zv = value_ast
			? crex_ast_get_zval(value_ast)
			: &EG(uninitialized_zval);
		C_TRY_ADDREF_P(value_zv);
		crex_compile_static_var_common(var_name, value_zv, CREX_BIND_REF);
	} else {
		crex_op *opline;

		zval *placeholder_ptr = crex_hash_update(CG(active_op_array)->static_variables, var_name, &EG(uninitialized_zval));
		C_TYPE_EXTRA_P(placeholder_ptr) |= IS_STATIC_VAR_UNINITIALIZED;
		uint32_t placeholder_offset = (uint32_t)((char*)placeholder_ptr - (char*)CG(active_op_array)->static_variables->arData);

		uint32_t static_def_jmp_opnum = get_next_op_number();
		opline = crex_emit_op(NULL, CREX_BIND_INIT_STATIC_OR_JMP, NULL, NULL);
		opline->op1_type = IS_CV;
		opline->op1.var = lookup_cv(var_name);
		opline->extended_value = placeholder_offset;

		znode expr;
		crex_compile_expr(&expr, value_ast);

		opline = crex_emit_op(NULL, CREX_BIND_STATIC, NULL, &expr);
		opline->op1_type = IS_CV;
		opline->op1.var = lookup_cv(var_name);
		opline->extended_value = placeholder_offset | CREX_BIND_REF;

		crex_update_jump_target_to_next(static_def_jmp_opnum);
	}
}
/* }}} */

static void crex_compile_unset(crex_ast *ast) /* {{{ */
{
	crex_ast *var_ast = ast->child[0];
	znode var_node;
	crex_op *opline;

	crex_ensure_writable_variable(var_ast);

	if (is_global_var_fetch(var_ast)) {
		if (!var_ast->child[1]) {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot use [] for unsetting");
		}

		crex_compile_expr(&var_node, var_ast->child[1]);
		if (var_node.op_type == IS_CONST) {
			convert_to_string(&var_node.u.constant);
		}

		opline = crex_emit_op(NULL, CREX_UNSET_VAR, &var_node, NULL);
		opline->extended_value = CREX_FETCH_GLOBAL;
		return;
	}

	switch (var_ast->kind) {
		case CREX_AST_VAR:
			if (is_this_fetch(var_ast)) {
				crex_error_noreturn(E_COMPILE_ERROR, "Cannot unset $this");
			} else if (crex_try_compile_cv(&var_node, var_ast) == SUCCESS) {
				opline = crex_emit_op(NULL, CREX_UNSET_CV, &var_node, NULL);
			} else {
				opline = crex_compile_simple_var_no_cv(NULL, var_ast, BP_VAR_UNSET, 0);
				opline->opcode = CREX_UNSET_VAR;
			}
			return;
		case CREX_AST_DIM:
			opline = crex_compile_dim(NULL, var_ast, BP_VAR_UNSET, /* by_ref */ false);
			opline->opcode = CREX_UNSET_DIM;
			return;
		case CREX_AST_PROP:
		case CREX_AST_NULLSAFE_PROP:
			opline = crex_compile_prop(NULL, var_ast, BP_VAR_UNSET, 0);
			opline->opcode = CREX_UNSET_OBJ;
			return;
		case CREX_AST_STATIC_PROP:
			opline = crex_compile_static_prop(NULL, var_ast, BP_VAR_UNSET, 0, 0);
			opline->opcode = CREX_UNSET_STATIC_PROP;
			return;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
}
/* }}} */

static bool crex_handle_loops_and_finally_ex(crex_long depth, znode *return_value) /* {{{ */
{
	crex_loop_var *base;
	crex_loop_var *loop_var = crex_stack_top(&CG(loop_var_stack));

	if (!loop_var) {
		return 1;
	}
	base = crex_stack_base(&CG(loop_var_stack));
	for (; loop_var >= base; loop_var--) {
		if (loop_var->opcode == CREX_FAST_CALL) {
			crex_op *opline = get_next_op();

			opline->opcode = CREX_FAST_CALL;
			opline->result_type = IS_TMP_VAR;
			opline->result.var = loop_var->var_num;
			if (return_value) {
				SET_NODE(opline->op2, return_value);
			}
			opline->op1.num = loop_var->try_catch_offset;
		} else if (loop_var->opcode == CREX_DISCARD_EXCEPTION) {
			crex_op *opline = get_next_op();
			opline->opcode = CREX_DISCARD_EXCEPTION;
			opline->op1_type = IS_TMP_VAR;
			opline->op1.var = loop_var->var_num;
		} else if (loop_var->opcode == CREX_RETURN) {
			/* Stack separator */
			break;
		} else if (depth <= 1) {
			return 1;
		} else if (loop_var->opcode == CREX_NOP) {
			/* Loop doesn't have freeable variable */
			depth--;
		} else {
			crex_op *opline;

			CREX_ASSERT(loop_var->var_type & (IS_VAR|IS_TMP_VAR));
			opline = get_next_op();
			opline->opcode = loop_var->opcode;
			opline->op1_type = loop_var->var_type;
			opline->op1.var = loop_var->var_num;
			opline->extended_value = CREX_FREE_ON_RETURN;
			depth--;
	    }
	}
	return (depth == 0);
}
/* }}} */

static bool crex_handle_loops_and_finally(znode *return_value) /* {{{ */
{
	return crex_handle_loops_and_finally_ex(crex_stack_count(&CG(loop_var_stack)) + 1, return_value);
}
/* }}} */

static bool crex_has_finally_ex(crex_long depth) /* {{{ */
{
	crex_loop_var *base;
	crex_loop_var *loop_var = crex_stack_top(&CG(loop_var_stack));

	if (!loop_var) {
		return 0;
	}
	base = crex_stack_base(&CG(loop_var_stack));
	for (; loop_var >= base; loop_var--) {
		if (loop_var->opcode == CREX_FAST_CALL) {
			return 1;
		} else if (loop_var->opcode == CREX_DISCARD_EXCEPTION) {
		} else if (loop_var->opcode == CREX_RETURN) {
			/* Stack separator */
			return 0;
		} else if (depth <= 1) {
			return 0;
		} else {
			depth--;
	    }
	}
	return 0;
}
/* }}} */

static bool crex_has_finally(void) /* {{{ */
{
	return crex_has_finally_ex(crex_stack_count(&CG(loop_var_stack)) + 1);
}
/* }}} */

static void crex_compile_return(crex_ast *ast) /* {{{ */
{
	crex_ast *expr_ast = ast->child[0];
	bool is_generator = (CG(active_op_array)->fn_flags & CREX_ACC_GENERATOR) != 0;
	bool by_ref = (CG(active_op_array)->fn_flags & CREX_ACC_RETURN_REFERENCE) != 0;

	znode expr_node;
	crex_op *opline;

	if (is_generator) {
		/* For generators the by-ref flag refers to yields, not returns */
		by_ref = 0;
	}

	if (!expr_ast) {
		expr_node.op_type = IS_CONST;
		ZVAL_NULL(&expr_node.u.constant);
	} else if (by_ref && crex_is_variable(expr_ast)) {
		if (crex_ast_is_short_circuited(expr_ast)) {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot take reference of a nullsafe chain");
		}

		crex_compile_var(&expr_node, expr_ast, BP_VAR_W, 1);
	} else {
		crex_compile_expr(&expr_node, expr_ast);
	}

	if ((CG(active_op_array)->fn_flags & CREX_ACC_HAS_FINALLY_BLOCK)
	 && (expr_node.op_type == IS_CV || (by_ref && expr_node.op_type == IS_VAR))
	 && crex_has_finally()) {
		/* Copy return value into temporary VAR to avoid modification in finally code */
		if (by_ref) {
			crex_emit_op(&expr_node, CREX_MAKE_REF, &expr_node, NULL);
		} else {
			crex_emit_op_tmp(&expr_node, CREX_QM_ASSIGN, &expr_node, NULL);
		}
	}

	/* Generator return types are handled separately */
	if (!is_generator && (CG(active_op_array)->fn_flags & CREX_ACC_HAS_RETURN_TYPE)) {
		crex_emit_return_type_check(
			expr_ast ? &expr_node : NULL, CG(active_op_array)->arg_info - 1, 0);
	}

	crex_handle_loops_and_finally((expr_node.op_type & (IS_TMP_VAR | IS_VAR)) ? &expr_node : NULL);

	opline = crex_emit_op(NULL, by_ref ? CREX_RETURN_BY_REF : CREX_RETURN,
		&expr_node, NULL);

	if (by_ref && expr_ast) {
		if (crex_is_call(expr_ast)) {
			opline->extended_value = CREX_RETURNS_FUNCTION;
		} else if (!crex_is_variable(expr_ast) || crex_ast_is_short_circuited(expr_ast)) {
			opline->extended_value = CREX_RETURNS_VALUE;
		}
	}
}
/* }}} */

static void crex_compile_echo(crex_ast *ast) /* {{{ */
{
	crex_op *opline;
	crex_ast *expr_ast = ast->child[0];

	znode expr_node;
	crex_compile_expr(&expr_node, expr_ast);

	opline = crex_emit_op(NULL, CREX_ECHO, &expr_node, NULL);
	opline->extended_value = 0;
}
/* }}} */

static void crex_compile_throw(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *expr_ast = ast->child[0];

	znode expr_node;
	crex_compile_expr(&expr_node, expr_ast);

	crex_op *opline = crex_emit_op(NULL, CREX_THROW, &expr_node, NULL);
	if (result) {
		/* Mark this as an "expression throw" for opcache. */
		opline->extended_value = CREX_THROW_IS_EXPR;
		result->op_type = IS_CONST;
		ZVAL_TRUE(&result->u.constant);
	}
}
/* }}} */

static void crex_compile_break_continue(crex_ast *ast) /* {{{ */
{
	crex_ast *depth_ast = ast->child[0];

	crex_op *opline;
	crex_long depth;

	CREX_ASSERT(ast->kind == CREX_AST_BREAK || ast->kind == CREX_AST_CONTINUE);

	if (depth_ast) {
		zval *depth_zv;
		if (depth_ast->kind != CREX_AST_ZVAL) {
			crex_error_noreturn(E_COMPILE_ERROR, "'%s' operator with non-integer operand "
				"is no longer supported", ast->kind == CREX_AST_BREAK ? "break" : "continue");
		}

		depth_zv = crex_ast_get_zval(depth_ast);
		if (C_TYPE_P(depth_zv) != IS_LONG || C_LVAL_P(depth_zv) < 1) {
			crex_error_noreturn(E_COMPILE_ERROR, "'%s' operator accepts only positive integers",
				ast->kind == CREX_AST_BREAK ? "break" : "continue");
		}

		depth = C_LVAL_P(depth_zv);
	} else {
		depth = 1;
	}

	if (CG(context).current_brk_cont == -1) {
		crex_error_noreturn(E_COMPILE_ERROR, "'%s' not in the 'loop' or 'switch' context",
			ast->kind == CREX_AST_BREAK ? "break" : "continue");
	} else {
		if (!crex_handle_loops_and_finally_ex(depth, NULL)) {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot '%s' " CREX_LONG_FMT " level%s",
				ast->kind == CREX_AST_BREAK ? "break" : "continue",
				depth, depth == 1 ? "" : "s");
		}
	}

	if (ast->kind == CREX_AST_CONTINUE) {
		int d, cur = CG(context).current_brk_cont;
		for (d = depth - 1; d > 0; d--) {
			cur = CG(context).brk_cont_array[cur].parent;
			CREX_ASSERT(cur != -1);
		}

		if (CG(context).brk_cont_array[cur].is_switch) {
			if (depth == 1) {
				if (CG(context).brk_cont_array[cur].parent == -1) {
					crex_error(E_WARNING,
						"\"continue\" targeting switch is equivalent to \"break\"");
				} else {
					crex_error(E_WARNING,
						"\"continue\" targeting switch is equivalent to \"break\". " \
						"Did you mean to use \"continue " CREX_LONG_FMT "\"?",
						depth + 1);
				}
			} else {
				if (CG(context).brk_cont_array[cur].parent == -1) {
					crex_error(E_WARNING,
						"\"continue " CREX_LONG_FMT "\" targeting switch is equivalent to \"break " CREX_LONG_FMT "\"",
						depth, depth);
				} else {
					crex_error(E_WARNING,
						"\"continue " CREX_LONG_FMT "\" targeting switch is equivalent to \"break " CREX_LONG_FMT "\". " \
						"Did you mean to use \"continue " CREX_LONG_FMT "\"?",
						depth, depth, depth + 1);
				}
			}
		}
	}

	opline = crex_emit_op(NULL, ast->kind == CREX_AST_BREAK ? CREX_BRK : CREX_CONT, NULL, NULL);
	opline->op1.num = CG(context).current_brk_cont;
	opline->op2.num = depth;
}
/* }}} */

void crex_resolve_goto_label(crex_op_array *op_array, crex_op *opline) /* {{{ */
{
	crex_label *dest;
	int current, remove_oplines = opline->op1.num;
	zval *label;
	uint32_t opnum = opline - op_array->opcodes;

	label = CT_CONSTANT_EX(op_array, opline->op2.constant);
	if (CG(context).labels == NULL ||
	    (dest = crex_hash_find_ptr(CG(context).labels, C_STR_P(label))) == NULL
	) {
		CG(in_compilation) = 1;
		CG(active_op_array) = op_array;
		CG(crex_lineno) = opline->lineno;
		crex_error_noreturn(E_COMPILE_ERROR, "'goto' to undefined label '%s'", C_STRVAL_P(label));
	}

	zval_ptr_dtor_str(label);
	ZVAL_NULL(label);

	current = opline->extended_value;
	for (; current != dest->brk_cont; current = CG(context).brk_cont_array[current].parent) {
		if (current == -1) {
			CG(in_compilation) = 1;
			CG(active_op_array) = op_array;
			CG(crex_lineno) = opline->lineno;
			crex_error_noreturn(E_COMPILE_ERROR, "'goto' into loop or switch statement is disallowed");
		}
		if (CG(context).brk_cont_array[current].start >= 0) {
			remove_oplines--;
		}
	}

	for (current = 0; current < op_array->last_try_catch; ++current) {
		crex_try_catch_element *elem = &op_array->try_catch_array[current];
		if (elem->try_op > opnum) {
			break;
		}
		if (elem->finally_op && opnum < elem->finally_op - 1
			&& (dest->opline_num > elem->finally_end || dest->opline_num < elem->try_op)
		) {
			remove_oplines--;
		}
	}

	opline->opcode = CREX_JMP;
	SET_UNUSED(opline->op1);
	SET_UNUSED(opline->op2);
	SET_UNUSED(opline->result);
	opline->op1.opline_num = dest->opline_num;
	opline->extended_value = 0;

	CREX_ASSERT(remove_oplines >= 0);
	while (remove_oplines--) {
		opline--;
		MAKE_NOP(opline);
		CREX_VM_SET_OPCODE_HANDLER(opline);
	}
}
/* }}} */

static void crex_compile_goto(crex_ast *ast) /* {{{ */
{
	crex_ast *label_ast = ast->child[0];
	znode label_node;
	crex_op *opline;

	crex_compile_expr(&label_node, label_ast);

	/* Label resolution and unwinding adjustments happen in pass two. */
	uint32_t opnum_start = get_next_op_number();
	crex_handle_loops_and_finally(NULL);
	opline = crex_emit_op(NULL, CREX_GOTO, NULL, &label_node);
	opline->op1.num = get_next_op_number() - opnum_start - 1;
	opline->extended_value = CG(context).current_brk_cont;
}
/* }}} */

static void crex_compile_label(crex_ast *ast) /* {{{ */
{
	crex_string *label = crex_ast_get_str(ast->child[0]);
	crex_label dest;

	if (!CG(context).labels) {
		ALLOC_HASHTABLE(CG(context).labels);
		crex_hash_init(CG(context).labels, 8, NULL, label_ptr_dtor, 0);
	}

	dest.brk_cont = CG(context).current_brk_cont;
	dest.opline_num = get_next_op_number();

	if (!crex_hash_add_mem(CG(context).labels, label, &dest, sizeof(crex_label))) {
		crex_error_noreturn(E_COMPILE_ERROR, "Label '%s' already defined", ZSTR_VAL(label));
	}
}
/* }}} */

static void crex_compile_while(crex_ast *ast) /* {{{ */
{
	crex_ast *cond_ast = ast->child[0];
	crex_ast *stmt_ast = ast->child[1];
	znode cond_node;
	uint32_t opnum_start, opnum_jmp, opnum_cond;

	opnum_jmp = crex_emit_jump(0);

	crex_begin_loop(CREX_NOP, NULL, 0);

	opnum_start = get_next_op_number();
	crex_compile_stmt(stmt_ast);

	opnum_cond = get_next_op_number();
	crex_update_jump_target(opnum_jmp, opnum_cond);
	crex_compile_expr(&cond_node, cond_ast);

	crex_emit_cond_jump(CREX_JMPNZ, &cond_node, opnum_start);

	crex_end_loop(opnum_cond, NULL);
}
/* }}} */

static void crex_compile_do_while(crex_ast *ast) /* {{{ */
{
	crex_ast *stmt_ast = ast->child[0];
	crex_ast *cond_ast = ast->child[1];

	znode cond_node;
	uint32_t opnum_start, opnum_cond;

	crex_begin_loop(CREX_NOP, NULL, 0);

	opnum_start = get_next_op_number();
	crex_compile_stmt(stmt_ast);

	opnum_cond = get_next_op_number();
	crex_compile_expr(&cond_node, cond_ast);

	crex_emit_cond_jump(CREX_JMPNZ, &cond_node, opnum_start);

	crex_end_loop(opnum_cond, NULL);
}
/* }}} */

static void crex_compile_expr_list(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast_list *list;
	uint32_t i;

	result->op_type = IS_CONST;
	ZVAL_TRUE(&result->u.constant);

	if (!ast) {
		return;
	}

	list = crex_ast_get_list(ast);
	for (i = 0; i < list->children; ++i) {
		crex_ast *expr_ast = list->child[i];

		crex_do_free(result);
		crex_compile_expr(result, expr_ast);
	}
}
/* }}} */

static void crex_compile_for(crex_ast *ast) /* {{{ */
{
	crex_ast *init_ast = ast->child[0];
	crex_ast *cond_ast = ast->child[1];
	crex_ast *loop_ast = ast->child[2];
	crex_ast *stmt_ast = ast->child[3];

	znode result;
	uint32_t opnum_start, opnum_jmp, opnum_loop;

	crex_compile_expr_list(&result, init_ast);
	crex_do_free(&result);

	opnum_jmp = crex_emit_jump(0);

	crex_begin_loop(CREX_NOP, NULL, 0);

	opnum_start = get_next_op_number();
	crex_compile_stmt(stmt_ast);

	opnum_loop = get_next_op_number();
	crex_compile_expr_list(&result, loop_ast);
	crex_do_free(&result);

	crex_update_jump_target_to_next(opnum_jmp);
	crex_compile_expr_list(&result, cond_ast);
	crex_do_extended_stmt();

	crex_emit_cond_jump(CREX_JMPNZ, &result, opnum_start);

	crex_end_loop(opnum_loop, NULL);
}
/* }}} */

static void crex_compile_foreach(crex_ast *ast) /* {{{ */
{
	crex_ast *expr_ast = ast->child[0];
	crex_ast *value_ast = ast->child[1];
	crex_ast *key_ast = ast->child[2];
	crex_ast *stmt_ast = ast->child[3];
	bool by_ref = value_ast->kind == CREX_AST_REF;
	bool is_variable = crex_is_variable(expr_ast) && crex_can_write_to_variable(expr_ast);

	znode expr_node, reset_node, value_node, key_node;
	crex_op *opline;
	uint32_t opnum_reset, opnum_fetch;

	if (key_ast) {
		if (key_ast->kind == CREX_AST_REF) {
			crex_error_noreturn(E_COMPILE_ERROR, "Key element cannot be a reference");
		}
		if (key_ast->kind == CREX_AST_ARRAY) {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot use list as key element");
		}
	}

	if (by_ref) {
		value_ast = value_ast->child[0];
	}

	if (value_ast->kind == CREX_AST_ARRAY && crex_propagate_list_refs(value_ast)) {
		by_ref = 1;
	}

	if (by_ref && is_variable) {
		crex_compile_var(&expr_node, expr_ast, BP_VAR_W, 1);
	} else {
		crex_compile_expr(&expr_node, expr_ast);
	}

	if (by_ref) {
		crex_separate_if_call_and_write(&expr_node, expr_ast, BP_VAR_W);
	}

	opnum_reset = get_next_op_number();
	opline = crex_emit_op(&reset_node, by_ref ? CREX_FE_RESET_RW : CREX_FE_RESET_R, &expr_node, NULL);

	crex_begin_loop(CREX_FE_FREE, &reset_node, 0);

	opnum_fetch = get_next_op_number();
	opline = crex_emit_op(NULL, by_ref ? CREX_FE_FETCH_RW : CREX_FE_FETCH_R, &reset_node, NULL);

	if (is_this_fetch(value_ast)) {
		crex_error_noreturn(E_COMPILE_ERROR, "Cannot re-assign $this");
	} else if (value_ast->kind == CREX_AST_VAR &&
		crex_try_compile_cv(&value_node, value_ast) == SUCCESS) {
		SET_NODE(opline->op2, &value_node);
	} else {
		opline->op2_type = IS_VAR;
		opline->op2.var = get_temporary_variable();
		GET_NODE(&value_node, opline->op2);
		if (value_ast->kind == CREX_AST_ARRAY) {
			crex_compile_list_assign(NULL, value_ast, &value_node, value_ast->attr);
		} else if (by_ref) {
			crex_emit_assign_ref_znode(value_ast, &value_node);
		} else {
			crex_emit_assign_znode(value_ast, &value_node);
		}
	}

	if (key_ast) {
		opline = &CG(active_op_array)->opcodes[opnum_fetch];
		crex_make_tmp_result(&key_node, opline);
		crex_emit_assign_znode(key_ast, &key_node);
	}

	crex_compile_stmt(stmt_ast);

	/* Place JMP and FE_FREE on the line where foreach starts. It would be
	 * better to use the end line, but this information is not available
	 * currently. */
	CG(crex_lineno) = ast->lineno;
	crex_emit_jump(opnum_fetch);

	opline = &CG(active_op_array)->opcodes[opnum_reset];
	opline->op2.opline_num = get_next_op_number();

	opline = &CG(active_op_array)->opcodes[opnum_fetch];
	opline->extended_value = get_next_op_number();

	crex_end_loop(opnum_fetch, &reset_node);

	opline = crex_emit_op(NULL, CREX_FE_FREE, &reset_node, NULL);
}
/* }}} */

static void crex_compile_if(crex_ast *ast) /* {{{ */
{
	crex_ast_list *list = crex_ast_get_list(ast);
	uint32_t i;
	uint32_t *jmp_opnums = NULL;

	if (list->children > 1) {
		jmp_opnums = safe_emalloc(sizeof(uint32_t), list->children - 1, 0);
	}

	for (i = 0; i < list->children; ++i) {
		crex_ast *elem_ast = list->child[i];
		crex_ast *cond_ast = elem_ast->child[0];
		crex_ast *stmt_ast = elem_ast->child[1];

		if (cond_ast) {
			znode cond_node;
			uint32_t opnum_jmpz;

			if (i > 0) {
				CG(crex_lineno) = cond_ast->lineno;
				crex_do_extended_stmt();
			}

			crex_compile_expr(&cond_node, cond_ast);
			opnum_jmpz = crex_emit_cond_jump(CREX_JMPZ, &cond_node, 0);

			crex_compile_stmt(stmt_ast);

			if (i != list->children - 1) {
				/* Set the lineno of JMP to the position of the if keyword, as we don't want to
				 * report the last line in the if branch as covered if it hasn't actually executed. */
				CG(crex_lineno) = elem_ast->lineno;
				jmp_opnums[i] = crex_emit_jump(0);
			}
			crex_update_jump_target_to_next(opnum_jmpz);
		} else {
			/* "else" can only occur as last element. */
			CREX_ASSERT(i == list->children - 1);
			crex_compile_stmt(stmt_ast);
		}
	}

	if (list->children > 1) {
		for (i = 0; i < list->children - 1; ++i) {
			crex_update_jump_target_to_next(jmp_opnums[i]);
		}
		efree(jmp_opnums);
	}
}
/* }}} */

static uint8_t determine_switch_jumptable_type(crex_ast_list *cases) {
	uint32_t i;
	uint8_t common_type = IS_UNDEF;
	for (i = 0; i < cases->children; i++) {
		crex_ast *case_ast = cases->child[i];
		crex_ast **cond_ast = &case_ast->child[0];
		zval *cond_zv;
		if (!case_ast->child[0]) {
			/* Skip default clause */
			continue;
		}

		crex_eval_const_expr(cond_ast);
		if ((*cond_ast)->kind != CREX_AST_ZVAL) {
			/* Non-constant case */
			return IS_UNDEF;
		}

		cond_zv = crex_ast_get_zval(case_ast->child[0]);
		if (C_TYPE_P(cond_zv) != IS_LONG && C_TYPE_P(cond_zv) != IS_STRING) {
			/* We only optimize switched on integers and strings */
			return IS_UNDEF;
		}

		if (common_type == IS_UNDEF) {
			common_type = C_TYPE_P(cond_zv);
		} else if (common_type != C_TYPE_P(cond_zv)) {
			/* Non-uniform case types */
			return IS_UNDEF;
		}

		if (C_TYPE_P(cond_zv) == IS_STRING
				&& is_numeric_string(C_STRVAL_P(cond_zv), C_STRLEN_P(cond_zv), NULL, NULL, 0)) {
			/* Numeric strings cannot be compared with a simple hash lookup */
			return IS_UNDEF;
		}
	}

	return common_type;
}

static bool should_use_jumptable(crex_ast_list *cases, uint8_t jumptable_type) {
	if (CG(compiler_options) & CREX_COMPILE_NO_JUMPTABLES) {
		return 0;
	}

	/* Thresholds are chosen based on when the average switch time for equidistributed
	 * input becomes smaller when using the jumptable optimization. */
	if (jumptable_type == IS_LONG) {
		return cases->children >= 5;
	} else {
		CREX_ASSERT(jumptable_type == IS_STRING);
		return cases->children >= 2;
	}
}

static void crex_compile_switch(crex_ast *ast) /* {{{ */
{
	crex_ast *expr_ast = ast->child[0];
	crex_ast_list *cases = crex_ast_get_list(ast->child[1]);

	uint32_t i;
	bool has_default_case = 0;

	znode expr_node, case_node;
	crex_op *opline;
	uint32_t *jmpnz_opnums, opnum_default_jmp, opnum_switch = (uint32_t)-1;
	uint8_t jumptable_type;
	HashTable *jumptable = NULL;

	crex_compile_expr(&expr_node, expr_ast);

	crex_begin_loop(CREX_FREE, &expr_node, 1);

	case_node.op_type = IS_TMP_VAR;
	case_node.u.op.var = get_temporary_variable();

	jumptable_type = determine_switch_jumptable_type(cases);
	if (jumptable_type != IS_UNDEF && should_use_jumptable(cases, jumptable_type)) {
		znode jumptable_op;

		ALLOC_HASHTABLE(jumptable);
		crex_hash_init(jumptable, cases->children, NULL, NULL, 0);
		jumptable_op.op_type = IS_CONST;
		ZVAL_ARR(&jumptable_op.u.constant, jumptable);

		opline = crex_emit_op(NULL,
			jumptable_type == IS_LONG ? CREX_SWITCH_LONG : CREX_SWITCH_STRING,
			&expr_node, &jumptable_op);
		if (opline->op1_type == IS_CONST) {
			C_TRY_ADDREF_P(CT_CONSTANT(opline->op1));
		}
		opnum_switch = opline - CG(active_op_array)->opcodes;
	}

	jmpnz_opnums = safe_emalloc(sizeof(uint32_t), cases->children, 0);
	for (i = 0; i < cases->children; ++i) {
		crex_ast *case_ast = cases->child[i];
		crex_ast *cond_ast = case_ast->child[0];
		znode cond_node;

		if (!cond_ast) {
			if (has_default_case) {
				CG(crex_lineno) = case_ast->lineno;
				crex_error_noreturn(E_COMPILE_ERROR,
					"Switch statements may only contain one default clause");
			}
			has_default_case = 1;
			continue;
		}

		crex_compile_expr(&cond_node, cond_ast);

		if (expr_node.op_type == IS_CONST
			&& C_TYPE(expr_node.u.constant) == IS_FALSE) {
			jmpnz_opnums[i] = crex_emit_cond_jump(CREX_JMPZ, &cond_node, 0);
		} else if (expr_node.op_type == IS_CONST
			&& C_TYPE(expr_node.u.constant) == IS_TRUE) {
			jmpnz_opnums[i] = crex_emit_cond_jump(CREX_JMPNZ, &cond_node, 0);
		} else {
			opline = crex_emit_op(NULL,
				(expr_node.op_type & (IS_VAR|IS_TMP_VAR)) ? CREX_CASE : CREX_IS_EQUAL,
				&expr_node, &cond_node);
			SET_NODE(opline->result, &case_node);
			if (opline->op1_type == IS_CONST) {
				C_TRY_ADDREF_P(CT_CONSTANT(opline->op1));
			}

			jmpnz_opnums[i] = crex_emit_cond_jump(CREX_JMPNZ, &case_node, 0);
		}
	}

	opnum_default_jmp = crex_emit_jump(0);

	for (i = 0; i < cases->children; ++i) {
		crex_ast *case_ast = cases->child[i];
		crex_ast *cond_ast = case_ast->child[0];
		crex_ast *stmt_ast = case_ast->child[1];

		if (cond_ast) {
			crex_update_jump_target_to_next(jmpnz_opnums[i]);

			if (jumptable) {
				zval *cond_zv = crex_ast_get_zval(cond_ast);
				zval jmp_target;
				ZVAL_LONG(&jmp_target, get_next_op_number());

				CREX_ASSERT(C_TYPE_P(cond_zv) == jumptable_type);
				if (C_TYPE_P(cond_zv) == IS_LONG) {
					crex_hash_index_add(jumptable, C_LVAL_P(cond_zv), &jmp_target);
				} else {
					CREX_ASSERT(C_TYPE_P(cond_zv) == IS_STRING);
					crex_hash_add(jumptable, C_STR_P(cond_zv), &jmp_target);
				}
			}
		} else {
			crex_update_jump_target_to_next(opnum_default_jmp);

			if (jumptable) {
				CREX_ASSERT(opnum_switch != (uint32_t)-1);
				opline = &CG(active_op_array)->opcodes[opnum_switch];
				opline->extended_value = get_next_op_number();
			}
		}

		crex_compile_stmt(stmt_ast);
	}

	if (!has_default_case) {
		crex_update_jump_target_to_next(opnum_default_jmp);

		if (jumptable) {
			opline = &CG(active_op_array)->opcodes[opnum_switch];
			opline->extended_value = get_next_op_number();
		}
	}

	crex_end_loop(get_next_op_number(), &expr_node);

	if (expr_node.op_type & (IS_VAR|IS_TMP_VAR)) {
		opline = crex_emit_op(NULL, CREX_FREE, &expr_node, NULL);
		opline->extended_value = CREX_FREE_SWITCH;
	} else if (expr_node.op_type == IS_CONST) {
		zval_ptr_dtor_nogc(&expr_node.u.constant);
	}

	efree(jmpnz_opnums);
}
/* }}} */

static uint32_t count_match_conds(crex_ast_list *arms)
{
	uint32_t num_conds = 0;

	for (uint32_t i = 0; i < arms->children; i++) {
		crex_ast *arm_ast = arms->child[i];
		if (arm_ast->child[0] == NULL) {
			continue;
		}

		crex_ast_list *conds = crex_ast_get_list(arm_ast->child[0]);
		num_conds += conds->children;
	}

	return num_conds;
}

static bool can_match_use_jumptable(crex_ast_list *arms) {
	for (uint32_t i = 0; i < arms->children; i++) {
		crex_ast *arm_ast = arms->child[i];
		if (!arm_ast->child[0]) {
			/* Skip default arm */
			continue;
		}

		crex_ast_list *conds = crex_ast_get_list(arm_ast->child[0]);
		for (uint32_t j = 0; j < conds->children; j++) {
			crex_ast **cond_ast = &conds->child[j];

			crex_eval_const_expr(cond_ast);
			if ((*cond_ast)->kind != CREX_AST_ZVAL) {
				return 0;
			}

			zval *cond_zv = crex_ast_get_zval(*cond_ast);
			if (C_TYPE_P(cond_zv) != IS_LONG && C_TYPE_P(cond_zv) != IS_STRING) {
				return 0;
			}
		}
	}

	return 1;
}

static void crex_compile_match(znode *result, crex_ast *ast)
{
	crex_ast *expr_ast = ast->child[0];
	crex_ast_list *arms = crex_ast_get_list(ast->child[1]);
	bool has_default_arm = 0;
	uint32_t opnum_match = (uint32_t)-1;

	znode expr_node;
	crex_compile_expr(&expr_node, expr_ast);

	znode case_node;
	case_node.op_type = IS_TMP_VAR;
	case_node.u.op.var = get_temporary_variable();

	uint32_t num_conds = count_match_conds(arms);
	uint8_t can_use_jumptable = can_match_use_jumptable(arms);
	bool uses_jumptable = can_use_jumptable && num_conds >= 2;
	HashTable *jumptable = NULL;
	uint32_t *jmpnz_opnums = NULL;

	for (uint32_t i = 0; i < arms->children; ++i) {
		crex_ast *arm_ast = arms->child[i];

		if (!arm_ast->child[0]) {
			if (has_default_arm) {
				CG(crex_lineno) = arm_ast->lineno;
				crex_error_noreturn(E_COMPILE_ERROR,
					"Match expressions may only contain one default arm");
			}
			has_default_arm = 1;
		}
	}

	if (uses_jumptable) {
		znode jumptable_op;

		ALLOC_HASHTABLE(jumptable);
		crex_hash_init(jumptable, num_conds, NULL, NULL, 0);
		jumptable_op.op_type = IS_CONST;
		ZVAL_ARR(&jumptable_op.u.constant, jumptable);

		crex_op *opline = crex_emit_op(NULL, CREX_MATCH, &expr_node, &jumptable_op);
		if (opline->op1_type == IS_CONST) {
			C_TRY_ADDREF_P(CT_CONSTANT(opline->op1));
		}
		opnum_match = opline - CG(active_op_array)->opcodes;
	} else {
		jmpnz_opnums = safe_emalloc(sizeof(uint32_t), num_conds, 0);
		uint32_t cond_count = 0;
		for (uint32_t i = 0; i < arms->children; ++i) {
			crex_ast *arm_ast = arms->child[i];

			if (!arm_ast->child[0]) {
				continue;
			}

			crex_ast_list *conds = crex_ast_get_list(arm_ast->child[0]);
			for (uint32_t j = 0; j < conds->children; j++) {
				crex_ast *cond_ast = conds->child[j];

				znode cond_node;
				crex_compile_expr(&cond_node, cond_ast);

				uint32_t opcode = (expr_node.op_type & (IS_VAR|IS_TMP_VAR)) ? CREX_CASE_STRICT : CREX_IS_IDENTICAL;
				crex_op *opline = crex_emit_op(NULL, opcode, &expr_node, &cond_node);
				SET_NODE(opline->result, &case_node);
				if (opline->op1_type == IS_CONST) {
					C_TRY_ADDREF_P(CT_CONSTANT(opline->op1));
				}

				jmpnz_opnums[cond_count] = crex_emit_cond_jump(CREX_JMPNZ, &case_node, 0);

				cond_count++;
			}
		}
	}

	uint32_t opnum_default_jmp = 0;
	if (!uses_jumptable) {
		opnum_default_jmp = crex_emit_jump(0);
	}

	bool is_first_case = 1;
	uint32_t cond_count = 0;
	uint32_t *jmp_end_opnums = safe_emalloc(sizeof(uint32_t), arms->children, 0);

	// The generated default arm is emitted first to avoid live range issues where the tmpvar
	// for the arm result is freed even though it has not been initialized yet.
	if (!has_default_arm) {
		if (!uses_jumptable) {
			crex_update_jump_target_to_next(opnum_default_jmp);
		}

		if (jumptable) {
			crex_op *opline = &CG(active_op_array)->opcodes[opnum_match];
			opline->extended_value = get_next_op_number();
		}

		crex_op *opline = crex_emit_op(NULL, CREX_MATCH_ERROR, &expr_node, NULL);
		if (opline->op1_type == IS_CONST) {
			C_TRY_ADDREF_P(CT_CONSTANT(opline->op1));
		}
		if (arms->children == 0) {
			/* Mark this as an "expression throw" for opcache. */
			opline->extended_value = CREX_THROW_IS_EXPR;
		}
	}

	for (uint32_t i = 0; i < arms->children; ++i) {
		crex_ast *arm_ast = arms->child[i];
		crex_ast *body_ast = arm_ast->child[1];

		if (arm_ast->child[0] != NULL) {
			crex_ast_list *conds = crex_ast_get_list(arm_ast->child[0]);

			for (uint32_t j = 0; j < conds->children; j++) {
				crex_ast *cond_ast = conds->child[j];

				if (jmpnz_opnums != NULL) {
					crex_update_jump_target_to_next(jmpnz_opnums[cond_count]);
				}

				if (jumptable) {
					zval *cond_zv = crex_ast_get_zval(cond_ast);
					zval jmp_target;
					ZVAL_LONG(&jmp_target, get_next_op_number());

					if (C_TYPE_P(cond_zv) == IS_LONG) {
						crex_hash_index_add(jumptable, C_LVAL_P(cond_zv), &jmp_target);
					} else {
						CREX_ASSERT(C_TYPE_P(cond_zv) == IS_STRING);
						crex_hash_add(jumptable, C_STR_P(cond_zv), &jmp_target);
					}
				}

				cond_count++;
			}
		} else {
			if (!uses_jumptable) {
				crex_update_jump_target_to_next(opnum_default_jmp);
			}

			if (jumptable) {
				CREX_ASSERT(opnum_match != (uint32_t)-1);
				crex_op *opline = &CG(active_op_array)->opcodes[opnum_match];
				opline->extended_value = get_next_op_number();
			}
		}

		znode body_node;
		crex_compile_expr(&body_node, body_ast);

		if (is_first_case) {
			crex_emit_op_tmp(result, CREX_QM_ASSIGN, &body_node, NULL);
			is_first_case = 0;
		} else {
			crex_op *opline_qm_assign = crex_emit_op(NULL, CREX_QM_ASSIGN, &body_node, NULL);
			SET_NODE(opline_qm_assign->result, result);
		}

		jmp_end_opnums[i] = crex_emit_jump(0);
	}

	// Initialize result in case there is no arm
	if (arms->children == 0) {
		result->op_type = IS_CONST;
		ZVAL_NULL(&result->u.constant);
	}

	for (uint32_t i = 0; i < arms->children; ++i) {
		crex_update_jump_target_to_next(jmp_end_opnums[i]);
	}

	if (expr_node.op_type & (IS_VAR|IS_TMP_VAR)) {
		crex_op *opline = crex_emit_op(NULL, CREX_FREE, &expr_node, NULL);
		opline->extended_value = CREX_FREE_SWITCH;
	} else if (expr_node.op_type == IS_CONST) {
		zval_ptr_dtor_nogc(&expr_node.u.constant);
	}

	if (jmpnz_opnums != NULL) {
		efree(jmpnz_opnums);
	}
	efree(jmp_end_opnums);
}

static void crex_compile_try(crex_ast *ast) /* {{{ */
{
	crex_ast *try_ast = ast->child[0];
	crex_ast_list *catches = crex_ast_get_list(ast->child[1]);
	crex_ast *finally_ast = ast->child[2];

	uint32_t i, j;
	crex_op *opline;
	uint32_t try_catch_offset;
	uint32_t *jmp_opnums = safe_emalloc(sizeof(uint32_t), catches->children, 0);
	uint32_t orig_fast_call_var = CG(context).fast_call_var;
	uint32_t orig_try_catch_offset = CG(context).try_catch_offset;

	if (catches->children == 0 && !finally_ast) {
		crex_error_noreturn(E_COMPILE_ERROR, "Cannot use try without catch or finally");
	}

	/* label: try { } must not be equal to try { label: } */
	if (CG(context).labels) {
		crex_label *label;
		CREX_HASH_MAP_REVERSE_FOREACH_PTR(CG(context).labels, label) {
			if (label->opline_num == get_next_op_number()) {
				crex_emit_op(NULL, CREX_NOP, NULL, NULL);
			}
			break;
		} CREX_HASH_FOREACH_END();
	}

	try_catch_offset = crex_add_try_element(get_next_op_number());

	if (finally_ast) {
		crex_loop_var fast_call;
		if (!(CG(active_op_array)->fn_flags & CREX_ACC_HAS_FINALLY_BLOCK)) {
			CG(active_op_array)->fn_flags |= CREX_ACC_HAS_FINALLY_BLOCK;
		}
		CG(context).fast_call_var = get_temporary_variable();

		/* Push FAST_CALL on unwind stack */
		fast_call.opcode = CREX_FAST_CALL;
		fast_call.var_type = IS_TMP_VAR;
		fast_call.var_num = CG(context).fast_call_var;
		fast_call.try_catch_offset = try_catch_offset;
		crex_stack_push(&CG(loop_var_stack), &fast_call);
	}

	CG(context).try_catch_offset = try_catch_offset;

	crex_compile_stmt(try_ast);

	if (catches->children != 0) {
		jmp_opnums[0] = crex_emit_jump(0);
	}

	for (i = 0; i < catches->children; ++i) {
		crex_ast *catch_ast = catches->child[i];
		crex_ast_list *classes = crex_ast_get_list(catch_ast->child[0]);
		crex_ast *var_ast = catch_ast->child[1];
		crex_ast *stmt_ast = catch_ast->child[2];
		crex_string *var_name = var_ast ? zval_make_interned_string(crex_ast_get_zval(var_ast)) : NULL;
		bool is_last_catch = (i + 1 == catches->children);

		uint32_t *jmp_multicatch = safe_emalloc(sizeof(uint32_t), classes->children - 1, 0);
		uint32_t opnum_catch = (uint32_t)-1;

		CG(crex_lineno) = catch_ast->lineno;

		for (j = 0; j < classes->children; j++) {
			crex_ast *class_ast = classes->child[j];
			bool is_last_class = (j + 1 == classes->children);

			if (!crex_is_const_default_class_ref(class_ast)) {
				crex_error_noreturn(E_COMPILE_ERROR, "Bad class name in the catch statement");
			}

			opnum_catch = get_next_op_number();
			if (i == 0 && j == 0) {
				CG(active_op_array)->try_catch_array[try_catch_offset].catch_op = opnum_catch;
			}

			opline = get_next_op();
			opline->opcode = CREX_CATCH;
			opline->op1_type = IS_CONST;
			opline->op1.constant = crex_add_class_name_literal(
					crex_resolve_class_name_ast(class_ast));
			opline->extended_value = crex_alloc_cache_slot();

			if (var_name && crex_string_equals(var_name, ZSTR_KNOWN(CREX_STR_THIS))) {
				crex_error_noreturn(E_COMPILE_ERROR, "Cannot re-assign $this");
			}

			opline->result_type = var_name ? IS_CV : IS_UNUSED;
			opline->result.var = var_name ? lookup_cv(var_name) : -1;

			if (is_last_catch && is_last_class) {
				opline->extended_value |= CREX_LAST_CATCH;
			}

			if (!is_last_class) {
				jmp_multicatch[j] = crex_emit_jump(0);
				opline = &CG(active_op_array)->opcodes[opnum_catch];
				opline->op2.opline_num = get_next_op_number();
			}
		}

		for (j = 0; j < classes->children - 1; j++) {
			crex_update_jump_target_to_next(jmp_multicatch[j]);
		}

		efree(jmp_multicatch);

		crex_compile_stmt(stmt_ast);

		if (!is_last_catch) {
			jmp_opnums[i + 1] = crex_emit_jump(0);
		}

		CREX_ASSERT(opnum_catch != (uint32_t)-1 && "Should have at least one class");
		opline = &CG(active_op_array)->opcodes[opnum_catch];
		if (!is_last_catch) {
			opline->op2.opline_num = get_next_op_number();
		}
	}

	for (i = 0; i < catches->children; ++i) {
		crex_update_jump_target_to_next(jmp_opnums[i]);
	}

	if (finally_ast) {
		crex_loop_var discard_exception;
		uint32_t opnum_jmp = get_next_op_number() + 1;

		/* Pop FAST_CALL from unwind stack */
		crex_stack_del_top(&CG(loop_var_stack));

		/* Push DISCARD_EXCEPTION on unwind stack */
		discard_exception.opcode = CREX_DISCARD_EXCEPTION;
		discard_exception.var_type = IS_TMP_VAR;
		discard_exception.var_num = CG(context).fast_call_var;
		crex_stack_push(&CG(loop_var_stack), &discard_exception);

		CG(crex_lineno) = finally_ast->lineno;

		opline = crex_emit_op(NULL, CREX_FAST_CALL, NULL, NULL);
		opline->op1.num = try_catch_offset;
		opline->result_type = IS_TMP_VAR;
		opline->result.var = CG(context).fast_call_var;

		crex_emit_op(NULL, CREX_JMP, NULL, NULL);

		crex_compile_stmt(finally_ast);

		CG(active_op_array)->try_catch_array[try_catch_offset].finally_op = opnum_jmp + 1;
		CG(active_op_array)->try_catch_array[try_catch_offset].finally_end
			= get_next_op_number();

		opline = crex_emit_op(NULL, CREX_FAST_RET, NULL, NULL);
		opline->op1_type = IS_TMP_VAR;
		opline->op1.var = CG(context).fast_call_var;
		opline->op2.num = orig_try_catch_offset;

		crex_update_jump_target_to_next(opnum_jmp);

		CG(context).fast_call_var = orig_fast_call_var;

		/* Pop DISCARD_EXCEPTION from unwind stack */
		crex_stack_del_top(&CG(loop_var_stack));
	}

	CG(context).try_catch_offset = orig_try_catch_offset;

	efree(jmp_opnums);
}
/* }}} */

/* Encoding declarations must already be handled during parsing */
bool crex_handle_encoding_declaration(crex_ast *ast) /* {{{ */
{
	crex_ast_list *declares = crex_ast_get_list(ast);
	uint32_t i;
	for (i = 0; i < declares->children; ++i) {
		crex_ast *declare_ast = declares->child[i];
		crex_ast *name_ast = declare_ast->child[0];
		crex_ast *value_ast = declare_ast->child[1];
		crex_string *name = crex_ast_get_str(name_ast);

		if (crex_string_equals_literal_ci(name, "encoding")) {
			if (value_ast->kind != CREX_AST_ZVAL) {
				crex_throw_exception(crex_ce_compile_error, "Encoding must be a literal", 0);
				return 0;
			}

			if (CG(multibyte)) {
				crex_string *encoding_name = zval_get_string(crex_ast_get_zval(value_ast));

				const crex_encoding *new_encoding, *old_encoding;
				crex_encoding_filter old_input_filter;

				CG(encoding_declared) = 1;

				new_encoding = crex_multibyte_fetch_encoding(ZSTR_VAL(encoding_name));
				if (!new_encoding) {
					crex_error(E_COMPILE_WARNING, "Unsupported encoding [%s]", ZSTR_VAL(encoding_name));
				} else {
					old_input_filter = LANG_SCNG(input_filter);
					old_encoding = LANG_SCNG(script_encoding);
					crex_multibyte_set_filter(new_encoding);

					/* need to re-scan if input filter changed */
					if (old_input_filter != LANG_SCNG(input_filter) ||
						 (old_input_filter && new_encoding != old_encoding)) {
						crex_multibyte_yyinput_again(old_input_filter, old_encoding);
					}
				}

				crex_string_release_ex(encoding_name, 0);
			} else {
				crex_error(E_COMPILE_WARNING, "declare(encoding=...) ignored because "
					"Crex multibyte feature is turned off by settings");
			}
		}
	}

	return 1;
}
/* }}} */

/* Check whether this is the first statement, not counting declares. */
static crex_result crex_is_first_statement(crex_ast *ast, bool allow_nop) /* {{{ */
{
	uint32_t i = 0;
	crex_ast_list *file_ast = crex_ast_get_list(CG(ast));

	while (i < file_ast->children) {
		if (file_ast->child[i] == ast) {
			return SUCCESS;
		} else if (file_ast->child[i] == NULL) {
			if (!allow_nop) {
				return FAILURE;
			}
		} else if (file_ast->child[i]->kind != CREX_AST_DECLARE) {
			return FAILURE;
		}
		i++;
	}
	return FAILURE;
}
/* }}} */

static void crex_compile_declare(crex_ast *ast) /* {{{ */
{
	crex_ast_list *declares = crex_ast_get_list(ast->child[0]);
	crex_ast *stmt_ast = ast->child[1];
	crex_declarables orig_declarables = FC(declarables);
	uint32_t i;

	for (i = 0; i < declares->children; ++i) {
		crex_ast *declare_ast = declares->child[i];
		crex_ast *name_ast = declare_ast->child[0];
		crex_ast **value_ast_ptr = &declare_ast->child[1];
		crex_string *name = crex_ast_get_str(name_ast);

		if ((*value_ast_ptr)->kind != CREX_AST_ZVAL) {
			crex_error_noreturn(E_COMPILE_ERROR, "declare(%s) value must be a literal", ZSTR_VAL(name));
		}

		if (crex_string_equals_literal_ci(name, "ticks")) {
			zval value_zv;
			crex_const_expr_to_zval(&value_zv, value_ast_ptr, /* allow_dynamic */ false);
			FC(declarables).ticks = zval_get_long(&value_zv);
			zval_ptr_dtor_nogc(&value_zv);
		} else if (crex_string_equals_literal_ci(name, "encoding")) {

			if (FAILURE == crex_is_first_statement(ast, /* allow_nop */ 0)) {
				crex_error_noreturn(E_COMPILE_ERROR, "Encoding declaration pragma must be "
					"the very first statement in the script");
			}
		} else if (crex_string_equals_literal_ci(name, "strict_types")) {
			zval value_zv;

			if (FAILURE == crex_is_first_statement(ast, /* allow_nop */ 0)) {
				crex_error_noreturn(E_COMPILE_ERROR, "strict_types declaration must be "
					"the very first statement in the script");
			}

			if (ast->child[1] != NULL) {
				crex_error_noreturn(E_COMPILE_ERROR, "strict_types declaration must not "
					"use block mode");
			}

			crex_const_expr_to_zval(&value_zv, value_ast_ptr, /* allow_dynamic */ false);

			if (C_TYPE(value_zv) != IS_LONG || (C_LVAL(value_zv) != 0 && C_LVAL(value_zv) != 1)) {
				crex_error_noreturn(E_COMPILE_ERROR, "strict_types declaration must have 0 or 1 as its value");
			}

			if (C_LVAL(value_zv) == 1) {
				CG(active_op_array)->fn_flags |= CREX_ACC_STRICT_TYPES;
			}

		} else {
			crex_error(E_COMPILE_WARNING, "Unsupported declare '%s'", ZSTR_VAL(name));
		}
	}

	if (stmt_ast) {
		crex_compile_stmt(stmt_ast);

		FC(declarables) = orig_declarables;
	}
}
/* }}} */

static void crex_compile_stmt_list(crex_ast *ast) /* {{{ */
{
	crex_ast_list *list = crex_ast_get_list(ast);
	uint32_t i;
	for (i = 0; i < list->children; ++i) {
		crex_compile_stmt(list->child[i]);
	}
}
/* }}} */

CREX_API void crex_set_function_arg_flags(crex_function *func) /* {{{ */
{
	uint32_t i, n;

	func->common.arg_flags[0] = 0;
	func->common.arg_flags[1] = 0;
	func->common.arg_flags[2] = 0;
	if (func->common.arg_info) {
		n = MIN(func->common.num_args, MAX_ARG_FLAG_NUM);
		i = 0;
		while (i < n) {
			CREX_SET_ARG_FLAG(func, i + 1, CREX_ARG_SEND_MODE(&func->common.arg_info[i]));
			i++;
		}
		if (UNEXPECTED((func->common.fn_flags & CREX_ACC_VARIADIC) && CREX_ARG_SEND_MODE(&func->common.arg_info[i]))) {
			uint32_t pass_by_reference = CREX_ARG_SEND_MODE(&func->common.arg_info[i]);
			while (i < MAX_ARG_FLAG_NUM) {
				CREX_SET_ARG_FLAG(func, i + 1, pass_by_reference);
				i++;
			}
		}
	}
}
/* }}} */

static crex_type crex_compile_single_typename(crex_ast *ast)
{
	CREX_ASSERT(!(ast->attr & CREX_TYPE_NULLABLE));
	if (ast->kind == CREX_AST_TYPE) {
		if (ast->attr == IS_STATIC && !CG(active_class_entry) && crex_is_scope_known()) {
			crex_error_noreturn(E_COMPILE_ERROR,
				"Cannot use \"static\" when no class scope is active");
		}

		return (crex_type) CREX_TYPE_INIT_CODE(ast->attr, 0, 0);
	} else {
		crex_string *class_name = crex_ast_get_str(ast);
		uint8_t type_code = crex_lookup_builtin_type_by_name(class_name);

		if (type_code != 0) {
			if ((ast->attr & CREX_NAME_NOT_FQ) != CREX_NAME_NOT_FQ) {
				crex_error_noreturn(E_COMPILE_ERROR,
					"Type declaration '%s' must be unqualified",
					ZSTR_VAL(crex_string_tolower(class_name)));
			}

			/* Transform iterable into a type union alias */
			if (type_code == IS_ITERABLE) {
				/* Set iterable bit for BC compat during Reflection and string representation of type */
				crex_type iterable = (crex_type) CREX_TYPE_INIT_CLASS_MASK(ZSTR_KNOWN(CREX_STR_TRAVERSABLE),
                	(MAY_BE_ARRAY|_CREX_TYPE_ITERABLE_BIT));
				return iterable;
			}

			return (crex_type) CREX_TYPE_INIT_CODE(type_code, 0, 0);
		} else {
			const char *correct_name;
			crex_string *orig_name = crex_ast_get_str(ast);
			uint32_t fetch_type = crex_get_class_fetch_type_ast(ast);
			if (fetch_type == CREX_FETCH_CLASS_DEFAULT) {
				class_name = crex_resolve_class_name_ast(ast);
				crex_assert_valid_class_name(class_name);
			} else {
				crex_ensure_valid_class_fetch_type(fetch_type);
				crex_string_addref(class_name);
			}

			if (ast->attr == CREX_NAME_NOT_FQ
					&& crex_is_confusable_type(orig_name, &correct_name)
					&& crex_is_not_imported(orig_name)) {
				const char *extra =
					FC(current_namespace) ? " or import the class with \"use\"" : "";
				if (correct_name) {
					crex_error(E_COMPILE_WARNING,
						"\"%s\" will be interpreted as a class name. Did you mean \"%s\"? "
						"Write \"\\%s\"%s to suppress this warning",
						ZSTR_VAL(orig_name), correct_name, ZSTR_VAL(class_name), extra);
				} else {
					crex_error(E_COMPILE_WARNING,
						"\"%s\" is not a supported builtin type "
						"and will be interpreted as a class name. "
						"Write \"\\%s\"%s to suppress this warning",
						ZSTR_VAL(orig_name), ZSTR_VAL(class_name), extra);
				}
			}

			class_name = crex_new_interned_string(class_name);
			crex_alloc_ce_cache(class_name);
			return (crex_type) CREX_TYPE_INIT_CLASS(class_name, 0, 0);
		}
	}
}

static void crex_are_intersection_types_redundant(crex_type left_type, crex_type right_type)
{
	CREX_ASSERT(CREX_TYPE_IS_INTERSECTION(left_type));
	CREX_ASSERT(CREX_TYPE_IS_INTERSECTION(right_type));
	crex_type_list *l_type_list = CREX_TYPE_LIST(left_type);
	crex_type_list *r_type_list = CREX_TYPE_LIST(right_type);
	crex_type_list *smaller_type_list, *larger_type_list;
	bool flipped = false;

	if (r_type_list->num_types < l_type_list->num_types) {
		smaller_type_list = r_type_list;
		larger_type_list = l_type_list;
		flipped = true;
	} else {
		smaller_type_list = l_type_list;
		larger_type_list = r_type_list;
	}

	unsigned int sum = 0;
	crex_type *outer_type;
	CREX_TYPE_LIST_FOREACH(smaller_type_list, outer_type)
		crex_type *inner_type;
		CREX_TYPE_LIST_FOREACH(larger_type_list, inner_type)
			if (crex_string_equals_ci(CREX_TYPE_NAME(*inner_type), CREX_TYPE_NAME(*outer_type))) {
				sum++;
				break;
			}
		CREX_TYPE_LIST_FOREACH_END();
	CREX_TYPE_LIST_FOREACH_END();

	if (sum == smaller_type_list->num_types) {
		crex_string *smaller_type_str;
		crex_string *larger_type_str;
		if (flipped) {
			smaller_type_str = crex_type_to_string(right_type);
			larger_type_str = crex_type_to_string(left_type);
		} else {
			smaller_type_str = crex_type_to_string(left_type);
			larger_type_str = crex_type_to_string(right_type);
		}
		if (smaller_type_list->num_types == larger_type_list->num_types) {
			crex_error_noreturn(E_COMPILE_ERROR, "Type %s is redundant with type %s",
				ZSTR_VAL(smaller_type_str), ZSTR_VAL(larger_type_str));
		} else {
			crex_error_noreturn(E_COMPILE_ERROR, "Type %s is redundant as it is more restrictive than type %s",
				ZSTR_VAL(larger_type_str), ZSTR_VAL(smaller_type_str));
		}
	}
}

static void crex_is_intersection_type_redundant_by_single_type(crex_type intersection_type, crex_type single_type)
{
	CREX_ASSERT(CREX_TYPE_IS_INTERSECTION(intersection_type));
	CREX_ASSERT(!CREX_TYPE_IS_INTERSECTION(single_type));

	crex_type *single_intersection_type = NULL;
	CREX_TYPE_FOREACH(intersection_type, single_intersection_type)
		if (crex_string_equals_ci(CREX_TYPE_NAME(*single_intersection_type), CREX_TYPE_NAME(single_type))) {
			crex_string *single_type_str = crex_type_to_string(single_type);
			crex_string *complete_type = crex_type_to_string(intersection_type);
			crex_error_noreturn(E_COMPILE_ERROR, "Type %s is redundant as it is more restrictive than type %s",
					ZSTR_VAL(complete_type), ZSTR_VAL(single_type_str));
		}
	CREX_TYPE_FOREACH_END();
}

/* Used by both intersection and union types prior to transforming the type list to a full crex_type */
static void crex_is_type_list_redundant_by_single_type(crex_type_list *type_list, crex_type type)
{
	CREX_ASSERT(!CREX_TYPE_IS_INTERSECTION(type));
	for (size_t i = 0; i < type_list->num_types - 1; i++) {
		if (CREX_TYPE_IS_INTERSECTION(type_list->types[i])) {
			crex_is_intersection_type_redundant_by_single_type(type_list->types[i], type);
			continue;
		}
		if (crex_string_equals_ci(CREX_TYPE_NAME(type_list->types[i]), CREX_TYPE_NAME(type))) {
			crex_string *single_type_str = crex_type_to_string(type);
			crex_error_noreturn(E_COMPILE_ERROR, "Duplicate type %s is redundant", ZSTR_VAL(single_type_str));
		}
	}
}

static crex_type crex_compile_typename(crex_ast *ast, bool force_allow_null);

static crex_type crex_compile_typename_ex(
		crex_ast *ast, bool force_allow_null, bool *forced_allow_null) /* {{{ */
{
	bool is_marked_nullable = ast->attr & CREX_TYPE_NULLABLE;
	crex_ast_attr orig_ast_attr = ast->attr;
	crex_type type = CREX_TYPE_INIT_NONE(0);

	if (is_marked_nullable) {
		ast->attr &= ~CREX_TYPE_NULLABLE;
	}

	if (ast->kind == CREX_AST_TYPE_UNION) {
		crex_ast_list *list = crex_ast_get_list(ast);
		crex_type_list *type_list;
		bool is_composite = false;
		bool has_only_iterable_class = true;
		ALLOCA_FLAG(use_heap)

		type_list = do_alloca(CREX_TYPE_LIST_SIZE(list->children), use_heap);
		type_list->num_types = 0;

		for (uint32_t i = 0; i < list->children; i++) {
			crex_ast *type_ast = list->child[i];
			crex_type single_type;
			uint32_t type_mask = CREX_TYPE_FULL_MASK(type);

			if (type_ast->kind == CREX_AST_TYPE_INTERSECTION) {
				has_only_iterable_class = false;
				is_composite = true;
				/* The first class type can be stored directly as the type ptr payload. */
				if (CREX_TYPE_IS_COMPLEX(type) && !CREX_TYPE_HAS_LIST(type)) {
					/* Switch from single name to name list. */
					type_list->num_types = 1;
					type_list->types[0] = type;
					CREX_TYPE_FULL_MASK(type_list->types[0]) &= ~_CREX_TYPE_MAY_BE_MASK;
				}
				/* Mark type as list type */
				CREX_TYPE_SET_LIST(type, type_list);

				single_type = crex_compile_typename(type_ast, false);
				CREX_ASSERT(CREX_TYPE_IS_INTERSECTION(single_type));

				type_list->types[type_list->num_types++] = single_type;

				/* Check for trivially redundant class types */
				for (size_t i = 0; i < type_list->num_types - 1; i++) {
					if (CREX_TYPE_IS_INTERSECTION(type_list->types[i])) {
						crex_are_intersection_types_redundant(single_type, type_list->types[i]);
						continue;
					}
					/* Type from type list is a simple type */
					crex_is_intersection_type_redundant_by_single_type(single_type, type_list->types[i]);
				}
				continue;
			}

			single_type = crex_compile_single_typename(type_ast);
			uint32_t single_type_mask = CREX_TYPE_PURE_MASK(single_type);

			if (single_type_mask == MAY_BE_ANY) {
				crex_error_noreturn(E_COMPILE_ERROR, "Type mixed can only be used as a standalone type");
			}
			if (CREX_TYPE_IS_COMPLEX(single_type) && !CREX_TYPE_IS_ITERABLE_FALLBACK(single_type)) {
				has_only_iterable_class = false;
			}

			uint32_t type_mask_overlap = CREX_TYPE_PURE_MASK(type) & single_type_mask;
			if (type_mask_overlap) {
				crex_type overlap_type = CREX_TYPE_INIT_MASK(type_mask_overlap);
				crex_string *overlap_type_str = crex_type_to_string(overlap_type);
				crex_error_noreturn(E_COMPILE_ERROR,
					"Duplicate type %s is redundant", ZSTR_VAL(overlap_type_str));
			}

			if ( ((type_mask & MAY_BE_TRUE) && (single_type_mask == MAY_BE_FALSE))
					|| ((type_mask & MAY_BE_FALSE) && (single_type_mask == MAY_BE_TRUE)) ) {
				crex_error_noreturn(E_COMPILE_ERROR,
					"Type contains both true and false, bool should be used instead");
			}
			CREX_TYPE_FULL_MASK(type) |= CREX_TYPE_PURE_MASK(single_type);
			CREX_TYPE_FULL_MASK(single_type) &= ~_CREX_TYPE_MAY_BE_MASK;

			if (CREX_TYPE_IS_COMPLEX(single_type)) {
				if (!CREX_TYPE_IS_COMPLEX(type) && !is_composite) {
					/* The first class type can be stored directly as the type ptr payload. */
					CREX_TYPE_SET_PTR(type, CREX_TYPE_NAME(single_type));
					CREX_TYPE_FULL_MASK(type) |= _CREX_TYPE_NAME_BIT;
				} else {
					if (type_list->num_types == 0) {
						/* Switch from single name to name list. */
						type_list->num_types = 1;
						type_list->types[0] = type;
						CREX_TYPE_FULL_MASK(type_list->types[0]) &= ~_CREX_TYPE_MAY_BE_MASK;
						CREX_TYPE_SET_LIST(type, type_list);
					}

					type_list->types[type_list->num_types++] = single_type;

					/* Check for trivially redundant class types */
					crex_is_type_list_redundant_by_single_type(type_list, single_type);
				}
			}
		}

		if (type_list->num_types) {
			crex_type_list *list = crex_arena_alloc(
				&CG(arena), CREX_TYPE_LIST_SIZE(type_list->num_types));
			memcpy(list, type_list, CREX_TYPE_LIST_SIZE(type_list->num_types));
			CREX_TYPE_SET_LIST(type, list);
			CREX_TYPE_FULL_MASK(type) |= _CREX_TYPE_ARENA_BIT;
			/* Inform that the type list is a union type */
			CREX_TYPE_FULL_MASK(type) |= _CREX_TYPE_UNION_BIT;
		}

		free_alloca(type_list, use_heap);

		uint32_t type_mask = CREX_TYPE_FULL_MASK(type);
		if ((type_mask & MAY_BE_OBJECT) &&
				((!has_only_iterable_class && CREX_TYPE_IS_COMPLEX(type)) || (type_mask & MAY_BE_STATIC))) {
			crex_string *type_str = crex_type_to_string(type);
			crex_error_noreturn(E_COMPILE_ERROR,
				"Type %s contains both object and a class type, which is redundant",
				ZSTR_VAL(type_str));
		}
	} else if (ast->kind == CREX_AST_TYPE_INTERSECTION) {
		crex_ast_list *list = crex_ast_get_list(ast);
		crex_type_list *type_list;

		/* Allocate the type list directly on the arena as it must be a type
		 * list of the same number of elements as the AST list has children */
		type_list = crex_arena_alloc(&CG(arena), CREX_TYPE_LIST_SIZE(list->children));
		type_list->num_types = 0;

		CREX_ASSERT(list->children > 1);

		for (uint32_t i = 0; i < list->children; i++) {
			crex_ast *type_ast = list->child[i];
			crex_type single_type = crex_compile_single_typename(type_ast);

			/* An intersection of union types cannot exist so invalidate it
			 * Currently only can happen with iterable getting canonicalized to Traversable|array */
			if (CREX_TYPE_IS_ITERABLE_FALLBACK(single_type)) {
				crex_string *standard_type_str = crex_type_to_string(single_type);
				crex_error_noreturn(E_COMPILE_ERROR,
					"Type %s cannot be part of an intersection type", ZSTR_VAL(standard_type_str));
				crex_string_release_ex(standard_type_str, false);
			}
			/* An intersection of standard types cannot exist so invalidate it */
			if (CREX_TYPE_IS_ONLY_MASK(single_type)) {
				crex_string *standard_type_str = crex_type_to_string(single_type);
				crex_error_noreturn(E_COMPILE_ERROR,
					"Type %s cannot be part of an intersection type", ZSTR_VAL(standard_type_str));
				crex_string_release_ex(standard_type_str, false);
			}
			/* Check for "self" and "parent" too */
			if (crex_string_equals_literal_ci(CREX_TYPE_NAME(single_type), "self")
					|| crex_string_equals_literal_ci(CREX_TYPE_NAME(single_type), "parent")) {
				crex_error_noreturn(E_COMPILE_ERROR,
					"Type %s cannot be part of an intersection type", ZSTR_VAL(CREX_TYPE_NAME(single_type)));
			}

			/* Add type to the type list */
			type_list->types[type_list->num_types++] = single_type;

			/* Check for trivially redundant class types */
			crex_is_type_list_redundant_by_single_type(type_list, single_type);
		}

		CREX_ASSERT(list->children == type_list->num_types);

		/* An implicitly nullable intersection type needs to be converted to a DNF type */
		if (force_allow_null) {
			crex_type intersection_type = CREX_TYPE_INIT_NONE(0);
			CREX_TYPE_SET_LIST(intersection_type, type_list);
			CREX_TYPE_FULL_MASK(intersection_type) |= _CREX_TYPE_INTERSECTION_BIT;
			CREX_TYPE_FULL_MASK(intersection_type) |= _CREX_TYPE_ARENA_BIT;

			crex_type_list *dnf_type_list = crex_arena_alloc(&CG(arena), CREX_TYPE_LIST_SIZE(1));
			dnf_type_list->num_types = 1;
			dnf_type_list->types[0] = intersection_type;
			CREX_TYPE_SET_LIST(type, dnf_type_list);
			/* Inform that the type list is a DNF type */
			CREX_TYPE_FULL_MASK(type) |= _CREX_TYPE_UNION_BIT;
			CREX_TYPE_FULL_MASK(type) |= _CREX_TYPE_ARENA_BIT;
		} else {
			CREX_TYPE_SET_LIST(type, type_list);
			/* Inform that the type list is an intersection type */
			CREX_TYPE_FULL_MASK(type) |= _CREX_TYPE_INTERSECTION_BIT;
			CREX_TYPE_FULL_MASK(type) |= _CREX_TYPE_ARENA_BIT;
		}
	} else {
		type = crex_compile_single_typename(ast);
	}

	uint32_t type_mask = CREX_TYPE_PURE_MASK(type);

	if (type_mask == MAY_BE_ANY && is_marked_nullable) {
		crex_error_noreturn(E_COMPILE_ERROR, "Type mixed cannot be marked as nullable since mixed already includes null");
	}

	if ((type_mask & MAY_BE_NULL) && is_marked_nullable) {
		crex_error_noreturn(E_COMPILE_ERROR, "null cannot be marked as nullable");
	}

	if (force_allow_null && !is_marked_nullable && !(type_mask & MAY_BE_NULL)) {
		*forced_allow_null = true;
	}

	if (is_marked_nullable || force_allow_null) {
		CREX_TYPE_FULL_MASK(type) |= MAY_BE_NULL;
		type_mask = CREX_TYPE_PURE_MASK(type);
	}

	if ((type_mask & MAY_BE_VOID) && (CREX_TYPE_IS_COMPLEX(type) || type_mask != MAY_BE_VOID)) {
		crex_error_noreturn(E_COMPILE_ERROR, "Void can only be used as a standalone type");
	}

	if ((type_mask & MAY_BE_NEVER) && (CREX_TYPE_IS_COMPLEX(type) || type_mask != MAY_BE_NEVER)) {
		crex_error_noreturn(E_COMPILE_ERROR, "never can only be used as a standalone type");
	}

	ast->attr = orig_ast_attr;
	return type;
}
/* }}} */

static crex_type crex_compile_typename(crex_ast *ast, bool force_allow_null)
{
	bool forced_allow_null;
	return crex_compile_typename_ex(ast, force_allow_null, &forced_allow_null);
}

/* May convert value from int to float. */
static bool crex_is_valid_default_value(crex_type type, zval *value)
{
	CREX_ASSERT(CREX_TYPE_IS_SET(type));
	if (CREX_TYPE_CONTAINS_CODE(type, C_TYPE_P(value))) {
		return 1;
	}
	if ((CREX_TYPE_FULL_MASK(type) & MAY_BE_DOUBLE) && C_TYPE_P(value) == IS_LONG) {
		/* Integers are allowed as initializers for floating-point values. */
		convert_to_double(value);
		return 1;
	}
	return 0;
}

static void crex_compile_attributes(
	HashTable **attributes, crex_ast *ast, uint32_t offset, uint32_t target, uint32_t promoted
) /* {{{ */ {
	crex_attribute *attr;
	crex_internal_attribute *config;

	crex_ast_list *list = crex_ast_get_list(ast);
	uint32_t g, i, j;

	CREX_ASSERT(ast->kind == CREX_AST_ATTRIBUTE_LIST);

	for (g = 0; g < list->children; g++) {
		crex_ast_list *group = crex_ast_get_list(list->child[g]);

		CREX_ASSERT(group->kind == CREX_AST_ATTRIBUTE_GROUP);

		for (i = 0; i < group->children; i++) {
			CREX_ASSERT(group->child[i]->kind == CREX_AST_ATTRIBUTE);

			crex_ast *el = group->child[i];

			if (el->child[1] &&
			    el->child[1]->kind == CREX_AST_CALLABLE_CONVERT) {
			    crex_error_noreturn(E_COMPILE_ERROR,
			        "Cannot create Closure as attribute argument");
			}

			crex_string *name = crex_resolve_class_name_ast(el->child[0]);
			crex_string *lcname = crex_string_tolower_ex(name, false);
			crex_ast_list *args = el->child[1] ? crex_ast_get_list(el->child[1]) : NULL;

			config = crex_internal_attribute_get(lcname);
			crex_string_release(lcname);

			/* Exclude internal attributes that do not match on promoted properties. */
			if (config && !(target & (config->flags & CREX_ATTRIBUTE_TARGET_ALL))) {
				if (promoted & (config->flags & CREX_ATTRIBUTE_TARGET_ALL)) {
					crex_string_release(name);
					continue;
				}
			}

			uint32_t flags = (CG(active_op_array)->fn_flags & CREX_ACC_STRICT_TYPES)
				? CREX_ATTRIBUTE_STRICT_TYPES : 0;
			attr = crex_add_attribute(
				attributes, name, args ? args->children : 0, flags, offset, el->lineno);
			crex_string_release(name);

			/* Populate arguments */
			if (args) {
				CREX_ASSERT(args->kind == CREX_AST_ARG_LIST);

				bool uses_named_args = 0;
				for (j = 0; j < args->children; j++) {
					crex_ast **arg_ast_ptr = &args->child[j];
					crex_ast *arg_ast = *arg_ast_ptr;

					if (arg_ast->kind == CREX_AST_UNPACK) {
						crex_error_noreturn(E_COMPILE_ERROR,
							"Cannot use unpacking in attribute argument list");
					}

					if (arg_ast->kind == CREX_AST_NAMED_ARG) {
						attr->args[j].name = crex_string_copy(crex_ast_get_str(arg_ast->child[0]));
						arg_ast_ptr = &arg_ast->child[1];
						uses_named_args = 1;

						for (uint32_t k = 0; k < j; k++) {
							if (attr->args[k].name &&
									crex_string_equals(attr->args[k].name, attr->args[j].name)) {
								crex_error_noreturn(E_COMPILE_ERROR, "Duplicate named parameter $%s",
									ZSTR_VAL(attr->args[j].name));
							}
						}
					} else if (uses_named_args) {
						crex_error_noreturn(E_COMPILE_ERROR,
							"Cannot use positional argument after named argument");
					}

					crex_const_expr_to_zval(
						&attr->args[j].value, arg_ast_ptr, /* allow_dynamic */ true);
				}
			}
		}
	}

	if (*attributes != NULL) {
		/* Validate attributes in a secondary loop (needed to detect repeated attributes). */
		CREX_HASH_PACKED_FOREACH_PTR(*attributes, attr) {
			if (attr->offset != offset || NULL == (config = crex_internal_attribute_get(attr->lcname))) {
				continue;
			}

			if (!(target & (config->flags & CREX_ATTRIBUTE_TARGET_ALL))) {
				crex_string *location = crex_get_attribute_target_names(target);
				crex_string *allowed = crex_get_attribute_target_names(config->flags);

				crex_error_noreturn(E_ERROR, "Attribute \"%s\" cannot target %s (allowed targets: %s)",
					ZSTR_VAL(attr->name), ZSTR_VAL(location), ZSTR_VAL(allowed)
				);
			}

			if (!(config->flags & CREX_ATTRIBUTE_IS_REPEATABLE)) {
				if (crex_is_attribute_repeated(*attributes, attr)) {
					crex_error_noreturn(E_ERROR, "Attribute \"%s\" must not be repeated", ZSTR_VAL(attr->name));
				}
			}

			if (config->validator != NULL) {
				config->validator(attr, target, CG(active_class_entry));
			}
		} CREX_HASH_FOREACH_END();
	}
}
/* }}} */

static void crex_compile_params(crex_ast *ast, crex_ast *return_type_ast, uint32_t fallback_return_type) /* {{{ */
{
	crex_ast_list *list = crex_ast_get_list(ast);
	uint32_t i;
	crex_op_array *op_array = CG(active_op_array);
	crex_arg_info *arg_infos;

	if (return_type_ast || fallback_return_type) {
		/* Use op_array->arg_info[-1] for return type */
		arg_infos = safe_emalloc(sizeof(crex_arg_info), list->children + 1, 0);
		arg_infos->name = NULL;
		if (return_type_ast) {
			arg_infos->type = crex_compile_typename(
				return_type_ast, /* force_allow_null */ 0);
			CREX_TYPE_FULL_MASK(arg_infos->type) |= _CREX_ARG_INFO_FLAGS(
				(op_array->fn_flags & CREX_ACC_RETURN_REFERENCE) != 0, /* is_variadic */ 0, /* is_tentative */ 0);
		} else {
			arg_infos->type = (crex_type) CREX_TYPE_INIT_CODE(fallback_return_type, 0, 0);
		}
		arg_infos++;
		op_array->fn_flags |= CREX_ACC_HAS_RETURN_TYPE;

		if (CREX_TYPE_CONTAINS_CODE(arg_infos[-1].type, IS_VOID)
				&& (op_array->fn_flags & CREX_ACC_RETURN_REFERENCE)) {
			crex_error(E_DEPRECATED, "Returning by reference from a void function is deprecated");
		}
	} else {
		if (list->children == 0) {
			return;
		}
		arg_infos = safe_emalloc(sizeof(crex_arg_info), list->children, 0);
	}

	/* Find last required parameter number for deprecation message. */
	uint32_t last_required_param = (uint32_t) -1;
	for (i = 0; i < list->children; ++i) {
		crex_ast *param_ast = list->child[i];
		crex_ast *default_ast_ptr = param_ast->child[2];
		bool is_variadic = (param_ast->attr & CREX_PARAM_VARIADIC) != 0;
		if (!default_ast_ptr && !is_variadic) {
			last_required_param = i;
		}
	}

	for (i = 0; i < list->children; ++i) {
		crex_ast *param_ast = list->child[i];
		crex_ast *type_ast = param_ast->child[0];
		crex_ast *var_ast = param_ast->child[1];
		crex_ast **default_ast_ptr = &param_ast->child[2];
		crex_ast *attributes_ast = param_ast->child[3];
		crex_ast *doc_comment_ast = param_ast->child[4];
		crex_string *name = zval_make_interned_string(crex_ast_get_zval(var_ast));
		bool is_ref = (param_ast->attr & CREX_PARAM_REF) != 0;
		bool is_variadic = (param_ast->attr & CREX_PARAM_VARIADIC) != 0;
		uint32_t property_flags = param_ast->attr & (CREX_ACC_PPP_MASK | CREX_ACC_READONLY);

		znode var_node, default_node;
		uint8_t opcode;
		crex_op *opline;
		crex_arg_info *arg_info;

		if (crex_is_auto_global(name)) {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot re-assign auto-global variable %s",
				ZSTR_VAL(name));
		}

		var_node.op_type = IS_CV;
		var_node.u.op.var = lookup_cv(name);

		if (EX_VAR_TO_NUM(var_node.u.op.var) != i) {
			crex_error_noreturn(E_COMPILE_ERROR, "Redefinition of parameter $%s",
				ZSTR_VAL(name));
		} else if (crex_string_equals(name, ZSTR_KNOWN(CREX_STR_THIS))) {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot use $this as parameter");
		}

		if (op_array->fn_flags & CREX_ACC_VARIADIC) {
			crex_error_noreturn(E_COMPILE_ERROR, "Only the last parameter can be variadic");
		}

		if (is_variadic) {
			opcode = CREX_RECV_VARIADIC;
			default_node.op_type = IS_UNUSED;
			op_array->fn_flags |= CREX_ACC_VARIADIC;

			if (*default_ast_ptr) {
				crex_error_noreturn(E_COMPILE_ERROR,
					"Variadic parameter cannot have a default value");
			}
		} else if (*default_ast_ptr) {
			/* we cannot substitute constants here or it will break ReflectionParameter::getDefaultValueConstantName() and ReflectionParameter::isDefaultValueConstant() */
			uint32_t cops = CG(compiler_options);
			CG(compiler_options) |= CREX_COMPILE_NO_CONSTANT_SUBSTITUTION | CREX_COMPILE_NO_PERSISTENT_CONSTANT_SUBSTITUTION;
			opcode = CREX_RECV_INIT;
			default_node.op_type = IS_CONST;
			crex_const_expr_to_zval(
				&default_node.u.constant, default_ast_ptr, /* allow_dynamic */ true);
			CG(compiler_options) = cops;
		} else {
			opcode = CREX_RECV;
			default_node.op_type = IS_UNUSED;
			op_array->required_num_args = i + 1;
		}

		arg_info = &arg_infos[i];
		arg_info->name = crex_string_copy(name);
		arg_info->type = (crex_type) CREX_TYPE_INIT_NONE(0);

		if (attributes_ast) {
			crex_compile_attributes(
				&op_array->attributes, attributes_ast, i + 1, CREX_ATTRIBUTE_TARGET_PARAMETER,
				property_flags ? CREX_ATTRIBUTE_TARGET_PROPERTY : 0
			);
		}

		bool forced_allow_nullable = false;
		if (type_ast) {
			uint32_t default_type = *default_ast_ptr ? C_TYPE(default_node.u.constant) : IS_UNDEF;
			bool force_nullable = default_type == IS_NULL && !property_flags;

			op_array->fn_flags |= CREX_ACC_HAS_TYPE_HINTS;
			arg_info->type = crex_compile_typename_ex(type_ast, force_nullable, &forced_allow_nullable);

			if (CREX_TYPE_FULL_MASK(arg_info->type) & MAY_BE_VOID) {
				crex_error_noreturn(E_COMPILE_ERROR, "void cannot be used as a parameter type");
			}

			if (CREX_TYPE_FULL_MASK(arg_info->type) & MAY_BE_NEVER) {
				crex_error_noreturn(E_COMPILE_ERROR, "never cannot be used as a parameter type");
			}

			if (default_type != IS_UNDEF && default_type != IS_CONSTANT_AST && !force_nullable
					&& !crex_is_valid_default_value(arg_info->type, &default_node.u.constant)) {
				crex_string *type_str = crex_type_to_string(arg_info->type);
				crex_error_noreturn(E_COMPILE_ERROR,
					"Cannot use %s as default value for parameter $%s of type %s",
					crex_get_type_by_const(default_type),
					ZSTR_VAL(name), ZSTR_VAL(type_str));
			}
		}
		if (last_required_param != (uint32_t) -1
		 && i < last_required_param
		 && default_node.op_type == IS_CONST) {
			/* Ignore parameters of the form "Type $param = null".
			 * This is the CRX 5 style way of writing "?Type $param", so allow it for now. */
			if (!forced_allow_nullable) {
				crex_ast *required_param_ast = list->child[last_required_param];
				crex_error(E_DEPRECATED,
					"Optional parameter $%s declared before required parameter $%s "
					"is implicitly treated as a required parameter",
					ZSTR_VAL(name), ZSTR_VAL(crex_ast_get_str(required_param_ast->child[1])));
			}

			/* Regardless of whether we issue a deprecation, convert this parameter into
			 * a required parameter without a default value. This ensures that it cannot be
			 * used as an optional parameter even with named parameters. */
			opcode = CREX_RECV;
			default_node.op_type = IS_UNUSED;
			zval_ptr_dtor(&default_node.u.constant);
		}

		opline = crex_emit_op(NULL, opcode, NULL, &default_node);
		SET_NODE(opline->result, &var_node);
		opline->op1.num = i + 1;

		if (type_ast) {
			/* Allocate cache slot to speed-up run-time class resolution */
			opline->extended_value =
				crex_alloc_cache_slots(crex_type_get_num_classes(arg_info->type));
		}

		uint32_t arg_info_flags = _CREX_ARG_INFO_FLAGS(is_ref, is_variadic, /* is_tentative */ 0)
			| (property_flags ? _CREX_IS_PROMOTED_BIT : 0);
		CREX_TYPE_FULL_MASK(arg_info->type) |= arg_info_flags;
		if (opcode == CREX_RECV) {
			opline->op2.num = type_ast ?
				CREX_TYPE_FULL_MASK(arg_info->type) : MAY_BE_ANY;
		}

		if (property_flags) {
			crex_op_array *op_array = CG(active_op_array);
			crex_class_entry *scope = op_array->scope;

			bool is_ctor =
				scope && crex_is_constructor(op_array->function_name);
			if (!is_ctor) {
				crex_error_noreturn(E_COMPILE_ERROR,
					"Cannot declare promoted property outside a constructor");
			}
			if ((op_array->fn_flags & CREX_ACC_ABSTRACT)
					|| (scope->ce_flags & CREX_ACC_INTERFACE)) {
				crex_error_noreturn(E_COMPILE_ERROR,
					"Cannot declare promoted property in an abstract constructor");
			}
			if (is_variadic) {
				crex_error_noreturn(E_COMPILE_ERROR,
					"Cannot declare variadic promoted property");
			}
			if (crex_hash_exists(&scope->properties_info, name)) {
				crex_error_noreturn(E_COMPILE_ERROR, "Cannot redeclare %s::$%s",
					ZSTR_VAL(scope->name), ZSTR_VAL(name));
			}
			if (CREX_TYPE_FULL_MASK(arg_info->type) & MAY_BE_CALLABLE) {
				crex_string *str = crex_type_to_string(arg_info->type);
				crex_error_noreturn(E_COMPILE_ERROR,
					"Property %s::$%s cannot have type %s",
					ZSTR_VAL(scope->name), ZSTR_VAL(name), ZSTR_VAL(str));
			}

			if (!(property_flags & CREX_ACC_READONLY) && (scope->ce_flags & CREX_ACC_READONLY_CLASS)) {
				property_flags |= CREX_ACC_READONLY;
			}

			/* Recompile the type, as it has different memory management requirements. */
			crex_type type = CREX_TYPE_INIT_NONE(0);
			if (type_ast) {
				type = crex_compile_typename(type_ast, /* force_allow_null */ 0);
			}

			/* Don't give the property an explicit default value. For typed properties this means
			 * uninitialized, for untyped properties it means an implicit null default value. */
			zval default_value;
			if (CREX_TYPE_IS_SET(type)) {
				ZVAL_UNDEF(&default_value);
			} else {
				if (property_flags & CREX_ACC_READONLY) {
					crex_error_noreturn(E_COMPILE_ERROR, "Readonly property %s::$%s must have type",
						ZSTR_VAL(scope->name), ZSTR_VAL(name));
				}

				ZVAL_NULL(&default_value);
			}

			crex_string *doc_comment =
				doc_comment_ast ? crex_string_copy(crex_ast_get_str(doc_comment_ast)) : NULL;
			crex_property_info *prop = crex_declare_typed_property(
				scope, name, &default_value, property_flags | CREX_ACC_PROMOTED, doc_comment, type);
			if (attributes_ast) {
				crex_compile_attributes(
					&prop->attributes, attributes_ast, 0, CREX_ATTRIBUTE_TARGET_PROPERTY, CREX_ATTRIBUTE_TARGET_PARAMETER);
			}
		}
	}

	/* These are assigned at the end to avoid uninitialized memory in case of an error */
	op_array->num_args = list->children;
	op_array->arg_info = arg_infos;

	/* Don't count the variadic argument */
	if (op_array->fn_flags & CREX_ACC_VARIADIC) {
		op_array->num_args--;
	}
	crex_set_function_arg_flags((crex_function*)op_array);

	for (i = 0; i < list->children; i++) {
		crex_ast *param_ast = list->child[i];
		bool is_ref = (param_ast->attr & CREX_PARAM_REF) != 0;
		uint32_t flags = param_ast->attr & (CREX_ACC_PPP_MASK | CREX_ACC_READONLY);
		if (!flags) {
			continue;
		}

		/* Emit $this->prop = $prop for promoted properties. */
		crex_string *name = crex_ast_get_str(param_ast->child[1]);
		znode name_node, value_node;
		name_node.op_type = IS_CONST;
		ZVAL_STR_COPY(&name_node.u.constant, name);
		value_node.op_type = IS_CV;
		value_node.u.op.var = lookup_cv(name);

		crex_op *opline = crex_emit_op(NULL,
			is_ref ? CREX_ASSIGN_OBJ_REF : CREX_ASSIGN_OBJ, NULL, &name_node);
		opline->extended_value = crex_alloc_cache_slots(3);
		crex_emit_op_data(&value_node);
	}
}
/* }}} */

static void crex_compile_closure_binding(znode *closure, crex_op_array *op_array, crex_ast *uses_ast) /* {{{ */
{
	crex_ast_list *list = crex_ast_get_list(uses_ast);
	uint32_t i;

	if (!list->children) {
		return;
	}

	if (!op_array->static_variables) {
		op_array->static_variables = crex_new_array(8);
	}

	for (i = 0; i < list->children; ++i) {
		crex_ast *var_name_ast = list->child[i];
		crex_string *var_name = zval_make_interned_string(crex_ast_get_zval(var_name_ast));
		uint32_t mode = var_name_ast->attr;
		crex_op *opline;
		zval *value;

		if (crex_string_equals(var_name, ZSTR_KNOWN(CREX_STR_THIS))) {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot use $this as lexical variable");
		}

		if (crex_is_auto_global(var_name)) {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot use auto-global as lexical variable");
		}

		value = crex_hash_add(op_array->static_variables, var_name, &EG(uninitialized_zval));
		if (!value) {
			crex_error_noreturn(E_COMPILE_ERROR,
				"Cannot use variable $%s twice", ZSTR_VAL(var_name));
		}

		CG(crex_lineno) = crex_ast_get_lineno(var_name_ast);

		opline = crex_emit_op(NULL, CREX_BIND_LEXICAL, closure, NULL);
		opline->op2_type = IS_CV;
		opline->op2.var = lookup_cv(var_name);
		opline->extended_value =
			(uint32_t)((char*)value - (char*)op_array->static_variables->arData) | mode;
	}
}
/* }}} */

typedef struct {
	HashTable uses;
	bool varvars_used;
} closure_info;

static void find_implicit_binds_recursively(closure_info *info, crex_ast *ast) {
	if (!ast) {
		return;
	}

	if (ast->kind == CREX_AST_VAR) {
		crex_ast *name_ast = ast->child[0];
		if (name_ast->kind == CREX_AST_ZVAL && C_TYPE_P(crex_ast_get_zval(name_ast)) == IS_STRING) {
			crex_string *name = crex_ast_get_str(name_ast);
			if (crex_is_auto_global(name)) {
				/* These is no need to explicitly import auto-globals. */
				return;
			}

			if (crex_string_equals(name, ZSTR_KNOWN(CREX_STR_THIS))) {
				/* $this does not need to be explicitly imported. */
				return;
			}

			crex_hash_add_empty_element(&info->uses, name);
		} else {
			info->varvars_used = 1;
			find_implicit_binds_recursively(info, name_ast);
		}
	} else if (crex_ast_is_list(ast)) {
		crex_ast_list *list = crex_ast_get_list(ast);
		uint32_t i;
		for (i = 0; i < list->children; i++) {
			find_implicit_binds_recursively(info, list->child[i]);
		}
	} else if (ast->kind == CREX_AST_CLOSURE) {
		/* For normal closures add the use() list. */
		crex_ast_decl *closure_ast = (crex_ast_decl *) ast;
		crex_ast *uses_ast = closure_ast->child[1];
		if (uses_ast) {
			crex_ast_list *uses_list = crex_ast_get_list(uses_ast);
			uint32_t i;
			for (i = 0; i < uses_list->children; i++) {
				crex_hash_add_empty_element(&info->uses, crex_ast_get_str(uses_list->child[i]));
			}
		}
	} else if (ast->kind == CREX_AST_ARROW_FUNC) {
		/* For arrow functions recursively check the expression. */
		crex_ast_decl *closure_ast = (crex_ast_decl *) ast;
		find_implicit_binds_recursively(info, closure_ast->child[2]);
	} else if (!crex_ast_is_special(ast)) {
		uint32_t i, children = crex_ast_get_num_children(ast);
		for (i = 0; i < children; i++) {
			find_implicit_binds_recursively(info, ast->child[i]);
		}
	}
}

static void find_implicit_binds(closure_info *info, crex_ast *params_ast, crex_ast *stmt_ast)
{
	crex_ast_list *param_list = crex_ast_get_list(params_ast);
	uint32_t i;

	crex_hash_init(&info->uses, param_list->children, NULL, NULL, 0);

	find_implicit_binds_recursively(info, stmt_ast);

	/* Remove variables that are parameters */
	for (i = 0; i < param_list->children; i++) {
		crex_ast *param_ast = param_list->child[i];
		crex_hash_del(&info->uses, crex_ast_get_str(param_ast->child[1]));
	}
}

static void compile_implicit_lexical_binds(
		closure_info *info, znode *closure, crex_op_array *op_array)
{
	crex_string *var_name;
	crex_op *opline;

	/* TODO We might want to use a special binding mode if varvars_used is set. */
	if (crex_hash_num_elements(&info->uses) == 0) {
		return;
	}

	if (!op_array->static_variables) {
		op_array->static_variables = crex_new_array(8);
	}

	CREX_HASH_MAP_FOREACH_STR_KEY(&info->uses, var_name)
		zval *value = crex_hash_add(
			op_array->static_variables, var_name, &EG(uninitialized_zval));
		uint32_t offset = (uint32_t)((char*)value - (char*)op_array->static_variables->arData);

		opline = crex_emit_op(NULL, CREX_BIND_LEXICAL, closure, NULL);
		opline->op2_type = IS_CV;
		opline->op2.var = lookup_cv(var_name);
		opline->extended_value = offset | CREX_BIND_IMPLICIT;
	CREX_HASH_FOREACH_END();
}

static void crex_compile_closure_uses(crex_ast *ast) /* {{{ */
{
	crex_op_array *op_array = CG(active_op_array);
	crex_ast_list *list = crex_ast_get_list(ast);
	uint32_t i;

	for (i = 0; i < list->children; ++i) {
		uint32_t mode = CREX_BIND_EXPLICIT;
		crex_ast *var_ast = list->child[i];
		crex_string *var_name = crex_ast_get_str(var_ast);
		zval zv;
		ZVAL_NULL(&zv);

		{
			int i;
			for (i = 0; i < op_array->last_var; i++) {
				if (crex_string_equals(op_array->vars[i], var_name)) {
					crex_error_noreturn(E_COMPILE_ERROR,
						"Cannot use lexical variable $%s as a parameter name", ZSTR_VAL(var_name));
				}
			}
		}

		CG(crex_lineno) = crex_ast_get_lineno(var_ast);

		if (var_ast->attr) {
			mode |= CREX_BIND_REF;
		}

		crex_compile_static_var_common(var_name, &zv, mode);
	}
}
/* }}} */

static void crex_compile_implicit_closure_uses(closure_info *info)
{
	crex_string *var_name;
	CREX_HASH_MAP_FOREACH_STR_KEY(&info->uses, var_name)
		zval zv;
		ZVAL_NULL(&zv);
		crex_compile_static_var_common(var_name, &zv, CREX_BIND_IMPLICIT);
	CREX_HASH_FOREACH_END();
}

static void add_stringable_interface(crex_class_entry *ce) {
	for (uint32_t i = 0; i < ce->num_interfaces; i++) {
		if (crex_string_equals_literal(ce->interface_names[i].lc_name, "stringable")) {
			/* Interface already explicitly implemented */
			return;
		}
	}

	ce->num_interfaces++;
	ce->interface_names =
		erealloc(ce->interface_names, sizeof(crex_class_name) * ce->num_interfaces);
	// TODO: Add known interned strings instead?
	ce->interface_names[ce->num_interfaces - 1].name =
		ZSTR_INIT_LITERAL("Stringable", 0);
	ce->interface_names[ce->num_interfaces - 1].lc_name =
		ZSTR_INIT_LITERAL("stringable", 0);
}

static crex_string *crex_begin_method_decl(crex_op_array *op_array, crex_string *name, bool has_body) /* {{{ */
{
	crex_class_entry *ce = CG(active_class_entry);
	bool in_interface = (ce->ce_flags & CREX_ACC_INTERFACE) != 0;
	uint32_t fn_flags = op_array->fn_flags;

	crex_string *lcname;

	if (fn_flags & CREX_ACC_READONLY) {
		crex_error(E_COMPILE_ERROR, "Cannot use 'readonly' as method modifier");
	}

	if ((fn_flags & CREX_ACC_PRIVATE) && (fn_flags & CREX_ACC_FINAL) && !crex_is_constructor(name)) {
		crex_error(E_COMPILE_WARNING, "Private methods cannot be final as they are never overridden by other classes");
	}

	if (in_interface) {
		if (!(fn_flags & CREX_ACC_PUBLIC)) {
			crex_error_noreturn(E_COMPILE_ERROR, "Access type for interface method "
				"%s::%s() must be public", ZSTR_VAL(ce->name), ZSTR_VAL(name));
		}
		if (fn_flags & CREX_ACC_FINAL) {
			crex_error_noreturn(E_COMPILE_ERROR, "Interface method "
				"%s::%s() must not be final", ZSTR_VAL(ce->name), ZSTR_VAL(name));
		}
		if (fn_flags & CREX_ACC_ABSTRACT) {
			crex_error_noreturn(E_COMPILE_ERROR, "Interface method "
				"%s::%s() must not be abstract", ZSTR_VAL(ce->name), ZSTR_VAL(name));
		}
		op_array->fn_flags |= CREX_ACC_ABSTRACT;
	}

	if (op_array->fn_flags & CREX_ACC_ABSTRACT) {
		if ((op_array->fn_flags & CREX_ACC_PRIVATE) && !(ce->ce_flags & CREX_ACC_TRAIT)) {
			crex_error_noreturn(E_COMPILE_ERROR, "%s function %s::%s() cannot be declared private",
				in_interface ? "Interface" : "Abstract", ZSTR_VAL(ce->name), ZSTR_VAL(name));
		}

		if (has_body) {
			crex_error_noreturn(E_COMPILE_ERROR, "%s function %s::%s() cannot contain body",
				in_interface ? "Interface" : "Abstract", ZSTR_VAL(ce->name), ZSTR_VAL(name));
		}

		ce->ce_flags |= CREX_ACC_IMPLICIT_ABSTRACT_CLASS;
	} else if (!has_body) {
		crex_error_noreturn(E_COMPILE_ERROR, "Non-abstract method %s::%s() must contain body",
			ZSTR_VAL(ce->name), ZSTR_VAL(name));
	}

	op_array->scope = ce;
	op_array->function_name = crex_string_copy(name);

	lcname = crex_string_tolower(name);
	lcname = crex_new_interned_string(lcname);

	if (crex_hash_add_ptr(&ce->function_table, lcname, op_array) == NULL) {
		crex_error_noreturn(E_COMPILE_ERROR, "Cannot redeclare %s::%s()",
			ZSTR_VAL(ce->name), ZSTR_VAL(name));
	}

	crex_add_magic_method(ce, (crex_function *) op_array, lcname);
	if (crex_string_equals_literal(lcname, CREX_TOSTRING_FUNC_NAME)
			&& !(ce->ce_flags & CREX_ACC_TRAIT)) {
		add_stringable_interface(ce);
	}

	return lcname;
}
/* }}} */

static uint32_t crex_add_dynamic_func_def(crex_op_array *def) {
	crex_op_array *op_array = CG(active_op_array);
	uint32_t def_offset = op_array->num_dynamic_func_defs++;
	op_array->dynamic_func_defs = erealloc(
		op_array->dynamic_func_defs, op_array->num_dynamic_func_defs * sizeof(crex_op_array *));
	op_array->dynamic_func_defs[def_offset] = def;
	return def_offset;
}

static crex_string *crex_begin_func_decl(znode *result, crex_op_array *op_array, crex_ast_decl *decl, bool toplevel) /* {{{ */
{
	crex_string *unqualified_name, *name, *lcname;
	crex_op *opline;

	unqualified_name = decl->name;
	op_array->function_name = name = crex_prefix_with_ns(unqualified_name);
	lcname = crex_string_tolower(name);

	if (FC(imports_function)) {
		crex_string *import_name =
			crex_hash_find_ptr_lc(FC(imports_function), unqualified_name);
		if (import_name && !crex_string_equals_ci(lcname, import_name)) {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot declare function %s "
				"because the name is already in use", ZSTR_VAL(name));
		}
	}

	if (crex_string_equals_literal(lcname, "__autoload")) {
		crex_error_noreturn(E_COMPILE_ERROR,
			"__autoload() is no longer supported, use spl_autoload_register() instead");
	}

	if (crex_string_equals_literal_ci(unqualified_name, "assert")) {
		crex_error(E_COMPILE_ERROR,
			"Defining a custom assert() function is not allowed, "
			"as the function has special semantics");
	}

	crex_register_seen_symbol(lcname, CREX_SYMBOL_FUNCTION);
	if (!toplevel) {
		uint32_t func_ref = crex_add_dynamic_func_def(op_array);
		if (op_array->fn_flags & CREX_ACC_CLOSURE) {
			opline = crex_emit_op_tmp(result, CREX_DECLARE_LAMBDA_FUNCTION, NULL, NULL);
			opline->op2.num = func_ref;
		} else {
			opline = get_next_op();
			opline->opcode = CREX_DECLARE_FUNCTION;
			opline->op1_type = IS_CONST;
			LITERAL_STR(opline->op1, crex_string_copy(lcname));
			opline->op2.num = func_ref;
		}
	}
	return lcname;
}
/* }}} */

static void crex_compile_func_decl(znode *result, crex_ast *ast, bool toplevel) /* {{{ */
{
	crex_ast_decl *decl = (crex_ast_decl *) ast;
	crex_ast *params_ast = decl->child[0];
	crex_ast *uses_ast = decl->child[1];
	crex_ast *stmt_ast = decl->child[2];
	crex_ast *return_type_ast = decl->child[3];
	bool is_method = decl->kind == CREX_AST_METHOD;
	crex_string *lcname;

	crex_class_entry *orig_class_entry = CG(active_class_entry);
	crex_op_array *orig_op_array = CG(active_op_array);
	crex_op_array *op_array = crex_arena_alloc(&CG(arena), sizeof(crex_op_array));
	crex_oparray_context orig_oparray_context;
	closure_info info;
	memset(&info, 0, sizeof(closure_info));

	init_op_array(op_array, CREX_USER_FUNCTION, INITIAL_OP_ARRAY_SIZE);

	if (CG(compiler_options) & CREX_COMPILE_PRELOAD) {
		op_array->fn_flags |= CREX_ACC_PRELOADED;
	}

	op_array->fn_flags |= (orig_op_array->fn_flags & CREX_ACC_STRICT_TYPES);
	op_array->fn_flags |= decl->flags;
	op_array->line_start = decl->start_lineno;
	op_array->line_end = decl->end_lineno;
	if (decl->doc_comment) {
		op_array->doc_comment = crex_string_copy(decl->doc_comment);
	}

	if (decl->kind == CREX_AST_CLOSURE || decl->kind == CREX_AST_ARROW_FUNC) {
		op_array->fn_flags |= CREX_ACC_CLOSURE;
	}

	if (is_method) {
		bool has_body = stmt_ast != NULL;
		lcname = crex_begin_method_decl(op_array, decl->name, has_body);
	} else {
		lcname = crex_begin_func_decl(result, op_array, decl, toplevel);
		if (decl->kind == CREX_AST_ARROW_FUNC) {
			find_implicit_binds(&info, params_ast, stmt_ast);
			compile_implicit_lexical_binds(&info, result, op_array);
		} else if (uses_ast) {
			crex_compile_closure_binding(result, op_array, uses_ast);
		}
	}

	CG(active_op_array) = op_array;

	if (decl->child[4]) {
		int target = CREX_ATTRIBUTE_TARGET_FUNCTION;

		if (is_method) {
			target = CREX_ATTRIBUTE_TARGET_METHOD;
		}

		crex_compile_attributes(&op_array->attributes, decl->child[4], 0, target, 0);

		crex_attribute *override_attribute = crex_get_attribute_str(
			op_array->attributes,
			"override",
			sizeof("override")-1
		);

		if (override_attribute) {
			op_array->fn_flags |= CREX_ACC_OVERRIDE;
		}
	}

	/* Do not leak the class scope into free standing functions, even if they are dynamically
	 * defined inside a class method. This is necessary for correct handling of magic constants.
	 * For example __CLASS__ should always be "" inside a free standing function. */
	if (decl->kind == CREX_AST_FUNC_DECL) {
		CG(active_class_entry) = NULL;
	}

	if (toplevel) {
		op_array->fn_flags |= CREX_ACC_TOP_LEVEL;
	}

	crex_oparray_context_begin(&orig_oparray_context);

	{
		/* Push a separator to the loop variable stack */
		crex_loop_var dummy_var;
		dummy_var.opcode = CREX_RETURN;

		crex_stack_push(&CG(loop_var_stack), (void *) &dummy_var);
	}

	crex_compile_params(params_ast, return_type_ast,
		is_method && crex_string_equals_literal(lcname, CREX_TOSTRING_FUNC_NAME) ? IS_STRING : 0);
	if (CG(active_op_array)->fn_flags & CREX_ACC_GENERATOR) {
		crex_mark_function_as_generator();
		crex_emit_op(NULL, CREX_GENERATOR_CREATE, NULL, NULL);
	}
	if (decl->kind == CREX_AST_ARROW_FUNC) {
		crex_compile_implicit_closure_uses(&info);
		crex_hash_destroy(&info.uses);
	} else if (uses_ast) {
		crex_compile_closure_uses(uses_ast);
	}

	if (ast->kind == CREX_AST_ARROW_FUNC && decl->child[2]->kind != CREX_AST_RETURN) {
		bool needs_return = true;
		if (op_array->fn_flags & CREX_ACC_HAS_RETURN_TYPE) {
			crex_arg_info *return_info = CG(active_op_array)->arg_info - 1;
			needs_return = !CREX_TYPE_CONTAINS_CODE(return_info->type, IS_NEVER);
		}
		if (needs_return) {
			stmt_ast = crex_ast_create(CREX_AST_RETURN, stmt_ast);
			decl->child[2] = stmt_ast;
		}
	}

	crex_compile_stmt(stmt_ast);

	if (is_method) {
		CG(crex_lineno) = decl->start_lineno;
		crex_check_magic_method_implementation(
			CG(active_class_entry), (crex_function *) op_array, lcname, E_COMPILE_ERROR);
	} else if (toplevel) {
		/* Only register the function after a successful compile */
		if (UNEXPECTED(crex_hash_add_ptr(CG(function_table), lcname, op_array) == NULL)) {
			do_bind_function_error(lcname, op_array, true);
		}
	}

	/* put the implicit return on the really last line */
	CG(crex_lineno) = decl->end_lineno;

	crex_do_extended_stmt();
	crex_emit_final_return(0);

	pass_two(CG(active_op_array));
	crex_oparray_context_end(&orig_oparray_context);

	/* Pop the loop variable stack separator */
	crex_stack_del_top(&CG(loop_var_stack));

	if (toplevel) {
		crex_observer_function_declared_notify(op_array, lcname);
	}

	crex_string_release_ex(lcname, 0);

	CG(active_op_array) = orig_op_array;
	CG(active_class_entry) = orig_class_entry;
}
/* }}} */

static void crex_compile_prop_decl(crex_ast *ast, crex_ast *type_ast, uint32_t flags, crex_ast *attr_ast) /* {{{ */
{
	crex_ast_list *list = crex_ast_get_list(ast);
	crex_class_entry *ce = CG(active_class_entry);
	uint32_t i, children = list->children;

	if (ce->ce_flags & CREX_ACC_INTERFACE) {
		crex_error_noreturn(E_COMPILE_ERROR, "Interfaces may not include properties");
	}

	if (ce->ce_flags & CREX_ACC_ENUM) {
		crex_error_noreturn(E_COMPILE_ERROR, "Enum %s cannot include properties", ZSTR_VAL(ce->name));
	}

	for (i = 0; i < children; ++i) {
		crex_property_info *info;
		crex_ast *prop_ast = list->child[i];
		crex_ast *name_ast = prop_ast->child[0];
		crex_ast **value_ast_ptr = &prop_ast->child[1];
		crex_ast *doc_comment_ast = prop_ast->child[2];
		crex_string *name = zval_make_interned_string(crex_ast_get_zval(name_ast));
		crex_string *doc_comment = NULL;
		zval value_zv;
		crex_type type = CREX_TYPE_INIT_NONE(0);

		if (type_ast) {
			type = crex_compile_typename(type_ast, /* force_allow_null */ 0);

			if (CREX_TYPE_FULL_MASK(type) & (MAY_BE_VOID|MAY_BE_NEVER|MAY_BE_CALLABLE)) {
				crex_string *str = crex_type_to_string(type);
				crex_error_noreturn(E_COMPILE_ERROR,
					"Property %s::$%s cannot have type %s",
					ZSTR_VAL(ce->name), ZSTR_VAL(name), ZSTR_VAL(str));
			}
		}

		/* Doc comment has been appended as last element in CREX_AST_PROP_ELEM ast */
		if (doc_comment_ast) {
			doc_comment = crex_string_copy(crex_ast_get_str(doc_comment_ast));
		}

		if (crex_hash_exists(&ce->properties_info, name)) {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot redeclare %s::$%s",
				ZSTR_VAL(ce->name), ZSTR_VAL(name));
		}

		if (*value_ast_ptr) {
			crex_const_expr_to_zval(&value_zv, value_ast_ptr, /* allow_dynamic */ false);

			if (CREX_TYPE_IS_SET(type) && !C_CONSTANT(value_zv)
					&& !crex_is_valid_default_value(type, &value_zv)) {
				crex_string *str = crex_type_to_string(type);
				if (C_TYPE(value_zv) == IS_NULL && !CREX_TYPE_IS_INTERSECTION(type)) {
					CREX_TYPE_FULL_MASK(type) |= MAY_BE_NULL;
					crex_string *nullable_str = crex_type_to_string(type);

					crex_error_noreturn(E_COMPILE_ERROR,
						"Default value for property of type %s may not be null. "
						"Use the nullable type %s to allow null default value",
						ZSTR_VAL(str), ZSTR_VAL(nullable_str));
				} else {
					crex_error_noreturn(E_COMPILE_ERROR,
						"Cannot use %s as default value for property %s::$%s of type %s",
						crex_zval_value_name(&value_zv),
						ZSTR_VAL(ce->name), ZSTR_VAL(name), ZSTR_VAL(str));
				}
			}
		} else if (!CREX_TYPE_IS_SET(type)) {
			ZVAL_NULL(&value_zv);
		} else {
			ZVAL_UNDEF(&value_zv);
		}

		if ((ce->ce_flags & CREX_ACC_READONLY_CLASS)) {
			flags |= CREX_ACC_READONLY;
		}

		if (flags & CREX_ACC_READONLY) {
			if (!CREX_TYPE_IS_SET(type)) {
				crex_error_noreturn(E_COMPILE_ERROR, "Readonly property %s::$%s must have type",
					ZSTR_VAL(ce->name), ZSTR_VAL(name));
			}
			if (!C_ISUNDEF(value_zv)) {
				crex_error_noreturn(E_COMPILE_ERROR,
					"Readonly property %s::$%s cannot have default value",
					ZSTR_VAL(ce->name), ZSTR_VAL(name));
			}
			if (flags & CREX_ACC_STATIC) {
				crex_error_noreturn(E_COMPILE_ERROR,
					"Static property %s::$%s cannot be readonly",
					ZSTR_VAL(ce->name), ZSTR_VAL(name));
			}
		}

		info = crex_declare_typed_property(ce, name, &value_zv, flags, doc_comment, type);

		if (attr_ast) {
			crex_compile_attributes(&info->attributes, attr_ast, 0, CREX_ATTRIBUTE_TARGET_PROPERTY, 0);
		}
	}
}
/* }}} */

static void crex_compile_prop_group(crex_ast *ast) /* {{{ */
{
	crex_ast *type_ast = ast->child[0];
	crex_ast *prop_ast = ast->child[1];
	crex_ast *attr_ast = ast->child[2];

	crex_compile_prop_decl(prop_ast, type_ast, ast->attr, attr_ast);
}
/* }}} */

static void crex_check_trait_alias_modifiers(uint32_t attr) /* {{{ */
{
	if (attr & CREX_ACC_STATIC) {
		crex_error_noreturn(E_COMPILE_ERROR, "Cannot use \"static\" as method modifier in trait alias");
	} else if (attr & CREX_ACC_ABSTRACT) {
		crex_error_noreturn(E_COMPILE_ERROR, "Cannot use \"abstract\" as method modifier in trait alias");
	}
}
/* }}} */

static void crex_compile_class_const_decl(crex_ast *ast, uint32_t flags, crex_ast *attr_ast, crex_ast *type_ast)
{
	crex_ast_list *list = crex_ast_get_list(ast);
	crex_class_entry *ce = CG(active_class_entry);
	uint32_t i, children = list->children;

	for (i = 0; i < children; ++i) {
		crex_class_constant *c;
		crex_ast *const_ast = list->child[i];
		crex_ast *name_ast = const_ast->child[0];
		crex_ast **value_ast_ptr = &const_ast->child[1];
		crex_ast *doc_comment_ast = const_ast->child[2];
		crex_string *name = zval_make_interned_string(crex_ast_get_zval(name_ast));
		crex_string *doc_comment = doc_comment_ast ? crex_string_copy(crex_ast_get_str(doc_comment_ast)) : NULL;
		zval value_zv;
		crex_type type = CREX_TYPE_INIT_NONE(0);

		if (type_ast) {
			type = crex_compile_typename(type_ast, /* force_allow_null */ 0);

			uint32_t type_mask = CREX_TYPE_PURE_MASK(type);

			if (type_mask != MAY_BE_ANY && (type_mask & (MAY_BE_CALLABLE|MAY_BE_VOID|MAY_BE_NEVER))) {
				crex_string *type_str = crex_type_to_string(type);

				crex_error_noreturn(E_COMPILE_ERROR, "Class constant %s::%s cannot have type %s",
					ZSTR_VAL(ce->name), ZSTR_VAL(name), ZSTR_VAL(type_str));
			}
		}

		if (UNEXPECTED((flags & CREX_ACC_PRIVATE) && (flags & CREX_ACC_FINAL))) {
			crex_error_noreturn(
				E_COMPILE_ERROR, "Private constant %s::%s cannot be final as it is not visible to other classes",
				ZSTR_VAL(ce->name), ZSTR_VAL(name)
			);
		}

		crex_const_expr_to_zval(&value_zv, value_ast_ptr, /* allow_dynamic */ false);

		if (!C_CONSTANT(value_zv) && CREX_TYPE_IS_SET(type) && !crex_is_valid_default_value(type, &value_zv)) {
			crex_string *type_str = crex_type_to_string(type);

			crex_error_noreturn(E_COMPILE_ERROR, "Cannot use %s as value for class constant %s::%s of type %s",
				crex_zval_type_name(&value_zv), ZSTR_VAL(ce->name), ZSTR_VAL(name), ZSTR_VAL(type_str));
		}

		c = crex_declare_typed_class_constant(ce, name, &value_zv, flags, doc_comment, type);

		if (attr_ast) {
			crex_compile_attributes(&c->attributes, attr_ast, 0, CREX_ATTRIBUTE_TARGET_CLASS_CONST, 0);
		}
	}
}

static void crex_compile_class_const_group(crex_ast *ast) /* {{{ */
{
	crex_ast *const_ast = ast->child[0];
	crex_ast *attr_ast = ast->child[1];
	crex_ast *type_ast = ast->child[2];

	crex_compile_class_const_decl(const_ast, ast->attr, attr_ast, type_ast);
}
/* }}} */

static void crex_compile_method_ref(crex_ast *ast, crex_trait_method_reference *method_ref) /* {{{ */
{
	crex_ast *class_ast = ast->child[0];
	crex_ast *method_ast = ast->child[1];

	method_ref->method_name = crex_string_copy(crex_ast_get_str(method_ast));

	if (class_ast) {
		method_ref->class_name = crex_resolve_const_class_name_reference(class_ast, "trait name");
	} else {
		method_ref->class_name = NULL;
	}
}
/* }}} */

static void crex_compile_trait_precedence(crex_ast *ast) /* {{{ */
{
	crex_ast *method_ref_ast = ast->child[0];
	crex_ast *insteadof_ast = ast->child[1];
	crex_ast_list *insteadof_list = crex_ast_get_list(insteadof_ast);
	uint32_t i;

	crex_trait_precedence *precedence = emalloc(sizeof(crex_trait_precedence) + (insteadof_list->children - 1) * sizeof(crex_string*));
	crex_compile_method_ref(method_ref_ast, &precedence->trait_method);
	precedence->num_excludes = insteadof_list->children;

	for (i = 0; i < insteadof_list->children; ++i) {
		crex_ast *name_ast = insteadof_list->child[i];
		precedence->exclude_class_names[i] =
			crex_resolve_const_class_name_reference(name_ast, "trait name");
	}

	crex_add_to_list(&CG(active_class_entry)->trait_precedences, precedence);
}
/* }}} */

static void crex_compile_trait_alias(crex_ast *ast) /* {{{ */
{
	crex_ast *method_ref_ast = ast->child[0];
	crex_ast *alias_ast = ast->child[1];
	uint32_t modifiers = ast->attr;

	crex_trait_alias *alias;

	crex_check_trait_alias_modifiers(modifiers);

	alias = emalloc(sizeof(crex_trait_alias));
	crex_compile_method_ref(method_ref_ast, &alias->trait_method);
	alias->modifiers = modifiers;

	if (alias_ast) {
		alias->alias = crex_string_copy(crex_ast_get_str(alias_ast));
	} else {
		alias->alias = NULL;
	}

	crex_add_to_list(&CG(active_class_entry)->trait_aliases, alias);
}
/* }}} */

static void crex_compile_use_trait(crex_ast *ast) /* {{{ */
{
	crex_ast_list *traits = crex_ast_get_list(ast->child[0]);
	crex_ast_list *adaptations = ast->child[1] ? crex_ast_get_list(ast->child[1]) : NULL;
	crex_class_entry *ce = CG(active_class_entry);
	uint32_t i;

	ce->trait_names = erealloc(ce->trait_names, sizeof(crex_class_name) * (ce->num_traits + traits->children));

	for (i = 0; i < traits->children; ++i) {
		crex_ast *trait_ast = traits->child[i];

		if (ce->ce_flags & CREX_ACC_INTERFACE) {
			crex_string *name = crex_ast_get_str(trait_ast);
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot use traits inside of interfaces. "
				"%s is used in %s", ZSTR_VAL(name), ZSTR_VAL(ce->name));
		}

		ce->trait_names[ce->num_traits].name =
			crex_resolve_const_class_name_reference(trait_ast, "trait name");
		ce->trait_names[ce->num_traits].lc_name = crex_string_tolower(ce->trait_names[ce->num_traits].name);
		ce->num_traits++;
	}

	if (!adaptations) {
		return;
	}

	for (i = 0; i < adaptations->children; ++i) {
		crex_ast *adaptation_ast = adaptations->child[i];
		switch (adaptation_ast->kind) {
			case CREX_AST_TRAIT_PRECEDENCE:
				crex_compile_trait_precedence(adaptation_ast);
				break;
			case CREX_AST_TRAIT_ALIAS:
				crex_compile_trait_alias(adaptation_ast);
				break;
			EMPTY_SWITCH_DEFAULT_CASE()
		}
	}
}
/* }}} */

static void crex_compile_implements(crex_ast *ast) /* {{{ */
{
	crex_ast_list *list = crex_ast_get_list(ast);
	crex_class_entry *ce = CG(active_class_entry);
	crex_class_name *interface_names;
	uint32_t i;

	interface_names = emalloc(sizeof(crex_class_name) * list->children);

	for (i = 0; i < list->children; ++i) {
		crex_ast *class_ast = list->child[i];
		interface_names[i].name =
			crex_resolve_const_class_name_reference(class_ast, "interface name");
		interface_names[i].lc_name = crex_string_tolower(interface_names[i].name);
	}

	ce->num_interfaces = list->children;
	ce->interface_names = interface_names;
}
/* }}} */

static crex_string *crex_generate_anon_class_name(crex_ast_decl *decl)
{
	crex_string *filename = CG(active_op_array)->filename;
	uint32_t start_lineno = decl->start_lineno;

	/* Use parent or first interface as prefix. */
	crex_string *prefix = ZSTR_KNOWN(CREX_STR_CLASS);
	if (decl->child[0]) {
		prefix = crex_resolve_const_class_name_reference(decl->child[0], "class name");
	} else if (decl->child[1]) {
		crex_ast_list *list = crex_ast_get_list(decl->child[1]);
		prefix = crex_resolve_const_class_name_reference(list->child[0], "interface name");
	}

	crex_string *result = crex_strpprintf(0, "%s@anonymous%c%s:%" PRIu32 "$%" PRIx32,
		ZSTR_VAL(prefix), '\0', ZSTR_VAL(filename), start_lineno, CG(rtd_key_counter)++);
	crex_string_release(prefix);
	return crex_new_interned_string(result);
}

static void crex_compile_enum_backing_type(crex_class_entry *ce, crex_ast *enum_backing_type_ast)
{
	CREX_ASSERT(ce->ce_flags & CREX_ACC_ENUM);
	crex_type type = crex_compile_typename(enum_backing_type_ast, 0);
	uint32_t type_mask = CREX_TYPE_PURE_MASK(type);
	if (CREX_TYPE_IS_COMPLEX(type) || (type_mask != MAY_BE_LONG && type_mask != MAY_BE_STRING)) {
		crex_string *type_string = crex_type_to_string(type);
		crex_error_noreturn(E_COMPILE_ERROR,
			"Enum backing type must be int or string, %s given",
			ZSTR_VAL(type_string));
	}
	if (type_mask == MAY_BE_LONG) {
		ce->enum_backing_type = IS_LONG;
	} else {
		CREX_ASSERT(type_mask == MAY_BE_STRING);
		ce->enum_backing_type = IS_STRING;
	}
	crex_type_release(type, 0);
}

static void crex_compile_class_decl(znode *result, crex_ast *ast, bool toplevel) /* {{{ */
{
	crex_ast_decl *decl = (crex_ast_decl *) ast;
	crex_ast *extends_ast = decl->child[0];
	crex_ast *implements_ast = decl->child[1];
	crex_ast *stmt_ast = decl->child[2];
	crex_ast *enum_backing_type_ast = decl->child[4];
	crex_string *name, *lcname;
	crex_class_entry *ce = crex_arena_alloc(&CG(arena), sizeof(crex_class_entry));
	crex_op *opline;

	crex_class_entry *original_ce = CG(active_class_entry);

	if (EXPECTED((decl->flags & CREX_ACC_ANON_CLASS) == 0)) {
		crex_string *unqualified_name = decl->name;

		if (CG(active_class_entry)) {
			crex_error_noreturn(E_COMPILE_ERROR, "Class declarations may not be nested");
		}

		crex_assert_valid_class_name(unqualified_name);
		name = crex_prefix_with_ns(unqualified_name);
		name = crex_new_interned_string(name);
		lcname = crex_string_tolower(name);

		if (FC(imports)) {
			crex_string *import_name =
				crex_hash_find_ptr_lc(FC(imports), unqualified_name);
			if (import_name && !crex_string_equals_ci(lcname, import_name)) {
				crex_error_noreturn(E_COMPILE_ERROR, "Cannot declare class %s "
						"because the name is already in use", ZSTR_VAL(name));
			}
		}

		crex_register_seen_symbol(lcname, CREX_SYMBOL_CLASS);
	} else {
		/* Find an anon class name that is not in use yet. */
		name = NULL;
		lcname = NULL;
		do {
			crex_tmp_string_release(name);
			crex_tmp_string_release(lcname);
			name = crex_generate_anon_class_name(decl);
			lcname = crex_string_tolower(name);
		} while (crex_hash_exists(CG(class_table), lcname));
	}
	lcname = crex_new_interned_string(lcname);

	ce->type = CREX_USER_CLASS;
	ce->name = name;
	crex_initialize_class_data(ce, 1);
	if (!(decl->flags & CREX_ACC_ANON_CLASS)) {
		crex_alloc_ce_cache(ce->name);
	}

	if (CG(compiler_options) & CREX_COMPILE_PRELOAD) {
		ce->ce_flags |= CREX_ACC_PRELOADED;
		CREX_MAP_PTR_NEW(ce->static_members_table);
		CREX_MAP_PTR_NEW(ce->mutable_data);
	}

	ce->ce_flags |= decl->flags;
	ce->info.user.filename = crex_string_copy(crex_get_compiled_filename());
	ce->info.user.line_start = decl->start_lineno;
	ce->info.user.line_end = decl->end_lineno;

	if (decl->doc_comment) {
		ce->info.user.doc_comment = crex_string_copy(decl->doc_comment);
	}

	if (UNEXPECTED((decl->flags & CREX_ACC_ANON_CLASS))) {
		/* Serialization is not supported for anonymous classes */
		ce->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;
	}

	if (extends_ast) {
		ce->parent_name =
			crex_resolve_const_class_name_reference(extends_ast, "class name");
	}

	CG(active_class_entry) = ce;

	if (decl->child[3]) {
		crex_compile_attributes(&ce->attributes, decl->child[3], 0, CREX_ATTRIBUTE_TARGET_CLASS, 0);
	}

	if (implements_ast) {
		crex_compile_implements(implements_ast);
	}

	if (ce->ce_flags & CREX_ACC_ENUM) {
		if (enum_backing_type_ast != NULL) {
			crex_compile_enum_backing_type(ce, enum_backing_type_ast);
		}
		crex_enum_add_interfaces(ce);
		crex_enum_register_props(ce);
	}

	crex_compile_stmt(stmt_ast);

	/* Reset lineno for final opcodes and errors */
	CG(crex_lineno) = ast->lineno;

	if ((ce->ce_flags & (CREX_ACC_IMPLICIT_ABSTRACT_CLASS|CREX_ACC_INTERFACE|CREX_ACC_TRAIT|CREX_ACC_EXPLICIT_ABSTRACT_CLASS)) == CREX_ACC_IMPLICIT_ABSTRACT_CLASS) {
		crex_verify_abstract_class(ce);
	}

	CG(active_class_entry) = original_ce;

	if (toplevel) {
		ce->ce_flags |= CREX_ACC_TOP_LEVEL;
	}

	/* We currently don't early-bind classes that implement interfaces or use traits */
	if (!ce->num_interfaces && !ce->num_traits
	 && !(CG(compiler_options) & CREX_COMPILE_WITHOUT_EXECUTION)) {
		if (toplevel) {
			if (extends_ast) {
				crex_class_entry *parent_ce = crex_lookup_class_ex(
					ce->parent_name, NULL, CREX_FETCH_CLASS_NO_AUTOLOAD);

				if (parent_ce
				 && ((parent_ce->type != CREX_INTERNAL_CLASS) || !(CG(compiler_options) & CREX_COMPILE_IGNORE_INTERNAL_CLASSES))
				 && ((parent_ce->type != CREX_USER_CLASS) || !(CG(compiler_options) & CREX_COMPILE_IGNORE_OTHER_FILES) || (parent_ce->info.user.filename == ce->info.user.filename))) {

					if (crex_try_early_bind(ce, parent_ce, lcname, NULL)) {
						crex_string_release(lcname);
						return;
					}
				}
			} else if (EXPECTED(crex_hash_add_ptr(CG(class_table), lcname, ce) != NULL)) {
				crex_string_release(lcname);
				crex_build_properties_info_table(ce);
				crex_inheritance_check_override(ce);
				ce->ce_flags |= CREX_ACC_LINKED;
				crex_observer_class_linked_notify(ce, lcname);
				return;
			} else {
				goto link_unbound;
			}
		} else if (!extends_ast) {
link_unbound:
			/* Link unbound simple class */
			crex_build_properties_info_table(ce);
			crex_inheritance_check_override(ce);
			ce->ce_flags |= CREX_ACC_LINKED;
		}
	}

	opline = get_next_op();

	if (ce->parent_name) {
		/* Lowercased parent name */
		crex_string *lc_parent_name = crex_string_tolower(ce->parent_name);
		opline->op2_type = IS_CONST;
		LITERAL_STR(opline->op2, lc_parent_name);
	}

	opline->op1_type = IS_CONST;
	LITERAL_STR(opline->op1, lcname);

	if (decl->flags & CREX_ACC_ANON_CLASS) {
		opline->opcode = CREX_DECLARE_ANON_CLASS;
		opline->extended_value = crex_alloc_cache_slot();
		crex_make_var_result(result, opline);
		if (!crex_hash_add_ptr(CG(class_table), lcname, ce)) {
			/* We checked above that the class name is not used. This really shouldn't happen. */
			crex_error_noreturn(E_ERROR,
				"Runtime definition key collision for %s. This is a bug", ZSTR_VAL(name));
		}
	} else {
		/* Generate RTD keys until we find one that isn't in use yet. */
		crex_string *key = NULL;
		do {
			crex_tmp_string_release(key);
			key = crex_build_runtime_definition_key(lcname, decl->start_lineno);
		} while (!crex_hash_add_ptr(CG(class_table), key, ce));

		/* RTD key is placed after lcname literal in op1 */
		crex_add_literal_string(&key);

		opline->opcode = CREX_DECLARE_CLASS;
		if (toplevel
			 && (CG(compiler_options) & CREX_COMPILE_DELAYED_BINDING)
				/* We currently don't early-bind classes that implement interfaces or use traits */
			 && !ce->num_interfaces && !ce->num_traits
		) {
			if (!extends_ast) {
				/* Use empty string for classes without parents to avoid new handler, and special
				 * handling of crex_early_binding. */
				opline->op2_type = IS_CONST;
				LITERAL_STR(opline->op2, ZSTR_EMPTY_ALLOC());
			}
			CG(active_op_array)->fn_flags |= CREX_ACC_EARLY_BINDING;
			opline->opcode = CREX_DECLARE_CLASS_DELAYED;
			opline->extended_value = crex_alloc_cache_slot();
			opline->result_type = IS_UNUSED;
			opline->result.opline_num = -1;
		}
	}
}
/* }}} */

static void crex_compile_enum_case(crex_ast *ast)
{
	crex_class_entry *enum_class = CG(active_class_entry);
	if (!(enum_class->ce_flags & CREX_ACC_ENUM)) {
		crex_error_noreturn(E_COMPILE_ERROR, "Case can only be used in enums");
	}

	crex_string *enum_case_name = zval_make_interned_string(crex_ast_get_zval(ast->child[0]));
	crex_string *enum_class_name = enum_class->name;

	zval class_name_zval;
	ZVAL_STR_COPY(&class_name_zval, enum_class_name);
	crex_ast *class_name_ast = crex_ast_create_zval(&class_name_zval);

	zval case_name_zval;
	ZVAL_STR_COPY(&case_name_zval, enum_case_name);
	crex_ast *case_name_ast = crex_ast_create_zval(&case_name_zval);

	crex_ast *case_value_ast = ast->child[1];
	// Remove case_value_ast from the original AST to avoid freeing it, as it will be freed by crex_const_expr_to_zval
	ast->child[1] = NULL;
	if (enum_class->enum_backing_type != IS_UNDEF && case_value_ast == NULL) {
		crex_error_noreturn(E_COMPILE_ERROR, "Case %s of backed enum %s must have a value",
			ZSTR_VAL(enum_case_name),
			ZSTR_VAL(enum_class_name));
	} else if (enum_class->enum_backing_type == IS_UNDEF && case_value_ast != NULL) {
		crex_error_noreturn(E_COMPILE_ERROR, "Case %s of non-backed enum %s must not have a value",
			ZSTR_VAL(enum_case_name),
			ZSTR_VAL(enum_class_name));
	}

	crex_ast *const_enum_init_ast = crex_ast_create(CREX_AST_CONST_ENUM_INIT, class_name_ast, case_name_ast, case_value_ast);

	zval value_zv;
	crex_const_expr_to_zval(&value_zv, &const_enum_init_ast, /* allow_dynamic */ false);

	/* Doc comment has been appended as second last element in CREX_AST_ENUM ast - attributes are conventionally last */
	crex_ast *doc_comment_ast = ast->child[2];
	crex_string *doc_comment = NULL;
	if (doc_comment_ast) {
		doc_comment = crex_string_copy(crex_ast_get_str(doc_comment_ast));
	}

	crex_class_constant *c = crex_declare_class_constant_ex(enum_class, enum_case_name, &value_zv, CREX_ACC_PUBLIC, doc_comment);
	CREX_CLASS_CONST_FLAGS(c) |= CREX_CLASS_CONST_IS_CASE;
	crex_ast_destroy(const_enum_init_ast);

	crex_ast *attr_ast = ast->child[3];
	if (attr_ast) {
		crex_compile_attributes(&c->attributes, attr_ast, 0, CREX_ATTRIBUTE_TARGET_CLASS_CONST, 0);
	}
}

static HashTable *crex_get_import_ht(uint32_t type) /* {{{ */
{
	switch (type) {
		case CREX_SYMBOL_CLASS:
			if (!FC(imports)) {
				FC(imports) = emalloc(sizeof(HashTable));
				crex_hash_init(FC(imports), 8, NULL, str_dtor, 0);
			}
			return FC(imports);
		case CREX_SYMBOL_FUNCTION:
			if (!FC(imports_function)) {
				FC(imports_function) = emalloc(sizeof(HashTable));
				crex_hash_init(FC(imports_function), 8, NULL, str_dtor, 0);
			}
			return FC(imports_function);
		case CREX_SYMBOL_CONST:
			if (!FC(imports_const)) {
				FC(imports_const) = emalloc(sizeof(HashTable));
				crex_hash_init(FC(imports_const), 8, NULL, str_dtor, 0);
			}
			return FC(imports_const);
		EMPTY_SWITCH_DEFAULT_CASE()
	}

	return NULL;
}
/* }}} */

static char *crex_get_use_type_str(uint32_t type) /* {{{ */
{
	switch (type) {
		case CREX_SYMBOL_CLASS:
			return "";
		case CREX_SYMBOL_FUNCTION:
			return " function";
		case CREX_SYMBOL_CONST:
			return " const";
		EMPTY_SWITCH_DEFAULT_CASE()
	}

	return " unknown";
}
/* }}} */

static void crex_check_already_in_use(uint32_t type, crex_string *old_name, crex_string *new_name, crex_string *check_name) /* {{{ */
{
	if (crex_string_equals_ci(old_name, check_name)) {
		return;
	}

	crex_error_noreturn(E_COMPILE_ERROR, "Cannot use%s %s as %s because the name "
		"is already in use", crex_get_use_type_str(type), ZSTR_VAL(old_name), ZSTR_VAL(new_name));
}
/* }}} */

static void crex_compile_use(crex_ast *ast) /* {{{ */
{
	crex_ast_list *list = crex_ast_get_list(ast);
	uint32_t i;
	crex_string *current_ns = FC(current_namespace);
	uint32_t type = ast->attr;
	HashTable *current_import = crex_get_import_ht(type);
	bool case_sensitive = type == CREX_SYMBOL_CONST;

	for (i = 0; i < list->children; ++i) {
		crex_ast *use_ast = list->child[i];
		crex_ast *old_name_ast = use_ast->child[0];
		crex_ast *new_name_ast = use_ast->child[1];
		crex_string *old_name = crex_ast_get_str(old_name_ast);
		crex_string *new_name, *lookup_name;

		if (new_name_ast) {
			new_name = crex_string_copy(crex_ast_get_str(new_name_ast));
		} else {
			const char *unqualified_name;
			size_t unqualified_name_len;
			if (crex_get_unqualified_name(old_name, &unqualified_name, &unqualified_name_len)) {
				/* The form "use A\B" is equivalent to "use A\B as B" */
				new_name = crex_string_init(unqualified_name, unqualified_name_len, 0);
			} else {
				new_name = crex_string_copy(old_name);

				if (!current_ns) {
					crex_error(E_WARNING, "The use statement with non-compound name '%s' "
						"has no effect", ZSTR_VAL(new_name));
				}
			}
		}

		if (case_sensitive) {
			lookup_name = crex_string_copy(new_name);
		} else {
			lookup_name = crex_string_tolower(new_name);
		}

		if (type == CREX_SYMBOL_CLASS && crex_is_reserved_class_name(new_name)) {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot use %s as %s because '%s' "
				"is a special class name", ZSTR_VAL(old_name), ZSTR_VAL(new_name), ZSTR_VAL(new_name));
		}

		if (current_ns) {
			crex_string *ns_name = crex_string_alloc(ZSTR_LEN(current_ns) + 1 + ZSTR_LEN(new_name), 0);
			crex_str_tolower_copy(ZSTR_VAL(ns_name), ZSTR_VAL(current_ns), ZSTR_LEN(current_ns));
			ZSTR_VAL(ns_name)[ZSTR_LEN(current_ns)] = '\\';
			memcpy(ZSTR_VAL(ns_name) + ZSTR_LEN(current_ns) + 1, ZSTR_VAL(lookup_name), ZSTR_LEN(lookup_name) + 1);

			if (crex_have_seen_symbol(ns_name, type)) {
				crex_check_already_in_use(type, old_name, new_name, ns_name);
			}

			crex_string_efree(ns_name);
		} else if (crex_have_seen_symbol(lookup_name, type)) {
			crex_check_already_in_use(type, old_name, new_name, lookup_name);
		}

		crex_string_addref(old_name);
		old_name = crex_new_interned_string(old_name);
		if (!crex_hash_add_ptr(current_import, lookup_name, old_name)) {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot use%s %s as %s because the name "
				"is already in use", crex_get_use_type_str(type), ZSTR_VAL(old_name), ZSTR_VAL(new_name));
		}

		crex_string_release_ex(lookup_name, 0);
		crex_string_release_ex(new_name, 0);
	}
}
/* }}} */

static void crex_compile_group_use(crex_ast *ast) /* {{{ */
{
	uint32_t i;
	crex_string *ns = crex_ast_get_str(ast->child[0]);
	crex_ast_list *list = crex_ast_get_list(ast->child[1]);

	for (i = 0; i < list->children; i++) {
		crex_ast *inline_use, *use = list->child[i];
		zval *name_zval = crex_ast_get_zval(use->child[0]);
		crex_string *name = C_STR_P(name_zval);
		crex_string *compound_ns = crex_concat_names(ZSTR_VAL(ns), ZSTR_LEN(ns), ZSTR_VAL(name), ZSTR_LEN(name));
		crex_string_release_ex(name, 0);
		ZVAL_STR(name_zval, compound_ns);
		inline_use = crex_ast_create_list(1, CREX_AST_USE, use);
		inline_use->attr = ast->attr ? ast->attr : use->attr;
		crex_compile_use(inline_use);
	}
}
/* }}} */

static void crex_compile_const_decl(crex_ast *ast) /* {{{ */
{
	crex_ast_list *list = crex_ast_get_list(ast);
	uint32_t i;
	for (i = 0; i < list->children; ++i) {
		crex_ast *const_ast = list->child[i];
		crex_ast *name_ast = const_ast->child[0];
		crex_ast **value_ast_ptr = &const_ast->child[1];
		crex_string *unqualified_name = crex_ast_get_str(name_ast);

		crex_string *name;
		znode name_node, value_node;
		zval *value_zv = &value_node.u.constant;

		value_node.op_type = IS_CONST;
		crex_const_expr_to_zval(value_zv, value_ast_ptr, /* allow_dynamic */ true);

		if (crex_get_special_const(ZSTR_VAL(unqualified_name), ZSTR_LEN(unqualified_name))) {
			crex_error_noreturn(E_COMPILE_ERROR,
				"Cannot redeclare constant '%s'", ZSTR_VAL(unqualified_name));
		}

		name = crex_prefix_with_ns(unqualified_name);
		name = crex_new_interned_string(name);

		if (FC(imports_const)) {
			crex_string *import_name = crex_hash_find_ptr(FC(imports_const), unqualified_name);
			if (import_name && !crex_string_equals(import_name, name)) {
				crex_error_noreturn(E_COMPILE_ERROR, "Cannot declare const %s because "
					"the name is already in use", ZSTR_VAL(name));
			}
		}

		name_node.op_type = IS_CONST;
		ZVAL_STR(&name_node.u.constant, name);

		crex_emit_op(NULL, CREX_DECLARE_CONST, &name_node, &value_node);

		crex_register_seen_symbol(name, CREX_SYMBOL_CONST);
	}
}
/* }}}*/

static void crex_compile_namespace(crex_ast *ast) /* {{{ */
{
	crex_ast *name_ast = ast->child[0];
	crex_ast *stmt_ast = ast->child[1];
	crex_string *name;
	bool with_bracket = stmt_ast != NULL;

	/* handle mixed syntax declaration or nested namespaces */
	if (!FC(has_bracketed_namespaces)) {
		if (FC(current_namespace)) {
			/* previous namespace declarations were unbracketed */
			if (with_bracket) {
				crex_error_noreturn(E_COMPILE_ERROR, "Cannot mix bracketed namespace declarations "
					"with unbracketed namespace declarations");
			}
		}
	} else {
		/* previous namespace declarations were bracketed */
		if (!with_bracket) {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot mix bracketed namespace declarations "
				"with unbracketed namespace declarations");
		} else if (FC(current_namespace) || FC(in_namespace)) {
			crex_error_noreturn(E_COMPILE_ERROR, "Namespace declarations cannot be nested");
		}
	}

	bool is_first_namespace = (!with_bracket && !FC(current_namespace))
		|| (with_bracket && !FC(has_bracketed_namespaces));
	if (is_first_namespace && FAILURE == crex_is_first_statement(ast, /* allow_nop */ 1)) {
		crex_error_noreturn(E_COMPILE_ERROR, "Namespace declaration statement has to be "
			"the very first statement or after any declare call in the script");
	}

	if (FC(current_namespace)) {
		crex_string_release_ex(FC(current_namespace), 0);
	}

	if (name_ast) {
		name = crex_ast_get_str(name_ast);

		if (crex_string_equals_literal_ci(name, "namespace")) {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot use '%s' as namespace name", ZSTR_VAL(name));
		}

		FC(current_namespace) = crex_string_copy(name);
	} else {
		FC(current_namespace) = NULL;
	}

	crex_reset_import_tables();

	FC(in_namespace) = 1;
	if (with_bracket) {
		FC(has_bracketed_namespaces) = 1;
	}

	if (stmt_ast) {
		crex_compile_top_stmt(stmt_ast);
		crex_end_namespace();
	}
}
/* }}} */

static void crex_compile_halt_compiler(crex_ast *ast) /* {{{ */
{
	crex_ast *offset_ast = ast->child[0];
	crex_long offset = C_LVAL_P(crex_ast_get_zval(offset_ast));

	crex_string *filename, *name;
	const char const_name[] = "__COMPILER_HALT_OFFSET__";

	if (FC(has_bracketed_namespaces) && FC(in_namespace)) {
		crex_error_noreturn(E_COMPILE_ERROR,
			"__HALT_COMPILER() can only be used from the outermost scope");
	}

	filename = crex_get_compiled_filename();
	name = crex_mangle_property_name(const_name, sizeof(const_name) - 1,
		ZSTR_VAL(filename), ZSTR_LEN(filename), 0);

	crex_register_long_constant(ZSTR_VAL(name), ZSTR_LEN(name), offset, 0, 0);
	crex_string_release_ex(name, 0);
}
/* }}} */

static bool crex_try_ct_eval_magic_const(zval *zv, crex_ast *ast) /* {{{ */
{
	crex_op_array *op_array = CG(active_op_array);
	crex_class_entry *ce = CG(active_class_entry);

	switch (ast->attr) {
		case T_LINE:
			ZVAL_LONG(zv, ast->lineno);
			break;
		case T_FILE:
			ZVAL_STR_COPY(zv, CG(compiled_filename));
			break;
		case T_DIR:
		{
			crex_string *filename = CG(compiled_filename);
			crex_string *dirname = crex_string_init(ZSTR_VAL(filename), ZSTR_LEN(filename), 0);
#ifdef CREX_WIN32
			ZSTR_LEN(dirname) = crx_win32_ioutil_dirname(ZSTR_VAL(dirname), ZSTR_LEN(dirname));
#else
			ZSTR_LEN(dirname) = crex_dirname(ZSTR_VAL(dirname), ZSTR_LEN(dirname));
#endif

			if (crex_string_equals_literal(dirname, ".")) {
				dirname = crex_string_extend(dirname, MAXPATHLEN, 0);
#if HAVE_GETCWD
				CREX_IGNORE_VALUE(VCWD_GETCWD(ZSTR_VAL(dirname), MAXPATHLEN));
#elif HAVE_GETWD
				CREX_IGNORE_VALUE(VCWD_GETWD(ZSTR_VAL(dirname)));
#endif
				ZSTR_LEN(dirname) = strlen(ZSTR_VAL(dirname));
			}

			ZVAL_STR(zv, dirname);
			break;
		}
		case T_FUNC_C:
			if (op_array && op_array->function_name) {
				ZVAL_STR_COPY(zv, op_array->function_name);
			} else {
				ZVAL_EMPTY_STRING(zv);
			}
			break;
		case T_METHOD_C:
			/* Detect whether we are directly inside a class (e.g. a class constant) and treat
			 * this as not being inside a function. */
			if (op_array && ce && !op_array->scope && !(op_array->fn_flags & CREX_ACC_CLOSURE)) {
				op_array = NULL;
			}
			if (op_array && op_array->function_name) {
				if (op_array->scope) {
					ZVAL_NEW_STR(zv,
						crex_create_member_string(op_array->scope->name, op_array->function_name));
				} else {
					ZVAL_STR_COPY(zv, op_array->function_name);
				}
			} else {
				ZVAL_EMPTY_STRING(zv);
			}
			break;
		case T_CLASS_C:
			if (ce) {
				if ((ce->ce_flags & CREX_ACC_TRAIT) != 0) {
					return 0;
				} else {
					ZVAL_STR_COPY(zv, ce->name);
				}
			} else {
				ZVAL_EMPTY_STRING(zv);
			}
			break;
		case T_TRAIT_C:
			if (ce && (ce->ce_flags & CREX_ACC_TRAIT) != 0) {
				ZVAL_STR_COPY(zv, ce->name);
			} else {
				ZVAL_EMPTY_STRING(zv);
			}
			break;
		case T_NS_C:
			if (FC(current_namespace)) {
				ZVAL_STR_COPY(zv, FC(current_namespace));
			} else {
				ZVAL_EMPTY_STRING(zv);
			}
			break;
		EMPTY_SWITCH_DEFAULT_CASE()
	}

	return 1;
}
/* }}} */

CREX_API bool crex_is_op_long_compatible(const zval *op)
{
	if (C_TYPE_P(op) == IS_ARRAY) {
		return false;
	}

	if (C_TYPE_P(op) == IS_DOUBLE
		&& !crex_is_long_compatible(C_DVAL_P(op), crex_dval_to_lval(C_DVAL_P(op)))) {
		return false;
	}

	if (C_TYPE_P(op) == IS_STRING) {
		double dval = 0;
		uint8_t is_num = is_numeric_str_function(C_STR_P(op), NULL, &dval);
		if (is_num == 0 || (is_num == IS_DOUBLE && !crex_is_long_compatible(dval, crex_dval_to_lval(dval)))) {
			return false;
		}
	}

	return true;
}

CREX_API bool crex_binary_op_produces_error(uint32_t opcode, const zval *op1, const zval *op2) /* {{{ */
{
	if ((opcode == CREX_CONCAT || opcode == CREX_FAST_CONCAT)) {
		/* Array to string warning. */
		return C_TYPE_P(op1) == IS_ARRAY || C_TYPE_P(op2) == IS_ARRAY;
	}

	if (!(opcode == CREX_ADD || opcode == CREX_SUB || opcode == CREX_MUL || opcode == CREX_DIV
               || opcode == CREX_POW || opcode == CREX_MOD || opcode == CREX_SL || opcode == CREX_SR
               || opcode == CREX_BW_OR || opcode == CREX_BW_AND || opcode == CREX_BW_XOR)) {
		/* Only the numeric operations throw errors. */
		return 0;
	}

	if (C_TYPE_P(op1) == IS_ARRAY || C_TYPE_P(op2) == IS_ARRAY) {
		if (opcode == CREX_ADD && C_TYPE_P(op1) == IS_ARRAY && C_TYPE_P(op2) == IS_ARRAY) {
			/* Adding two arrays is allowed. */
			return 0;
		}

		/* Numeric operators throw when one of the operands is an array. */
		return 1;
	}

	/* While basic arithmetic operators always produce numeric string errors,
	 * bitwise operators don't produce errors if both operands are strings */
	if ((opcode == CREX_BW_OR || opcode == CREX_BW_AND || opcode == CREX_BW_XOR)
		&& C_TYPE_P(op1) == IS_STRING && C_TYPE_P(op2) == IS_STRING) {
		return 0;
	}

	if (C_TYPE_P(op1) == IS_STRING
		&& !is_numeric_string(C_STRVAL_P(op1), C_STRLEN_P(op1), NULL, NULL, 0)) {
		return 1;
	}

	if (C_TYPE_P(op2) == IS_STRING
		&& !is_numeric_string(C_STRVAL_P(op2), C_STRLEN_P(op2), NULL, NULL, 0)) {
		return 1;
	}

	if ((opcode == CREX_MOD && zval_get_long(op2) == 0)
			|| (opcode == CREX_DIV && zval_get_double(op2) == 0.0)) {
		/* Division by zero throws an error. */
		return 1;
	}
	if ((opcode == CREX_SL || opcode == CREX_SR) && zval_get_long(op2) < 0) {
		/* Shift by negative number throws an error. */
		return 1;
	}

	/* Operation which cast float/float-strings to integers might produce incompatible float to int errors */
	if (opcode == CREX_SL || opcode == CREX_SR || opcode == CREX_BW_OR
			|| opcode == CREX_BW_AND || opcode == CREX_BW_XOR || opcode == CREX_MOD) {
		return !crex_is_op_long_compatible(op1) || !crex_is_op_long_compatible(op2);
	}

	return 0;
}
/* }}} */

static inline bool crex_try_ct_eval_binary_op(zval *result, uint32_t opcode, zval *op1, zval *op2) /* {{{ */
{
	if (crex_binary_op_produces_error(opcode, op1, op2)) {
		return 0;
	}

	binary_op_type fn = get_binary_op(opcode);
	fn(result, op1, op2);
	return 1;
}
/* }}} */

CREX_API bool crex_unary_op_produces_error(uint32_t opcode, const zval *op)
{
	if (opcode == CREX_BW_NOT) {
		/* BW_NOT on string does not convert the string into an integer. */
		if (C_TYPE_P(op) == IS_STRING) {
			return 0;
		}
		return C_TYPE_P(op) <= IS_TRUE || !crex_is_op_long_compatible(op);
	}

	return 0;
}

static inline bool crex_try_ct_eval_unary_op(zval *result, uint32_t opcode, zval *op) /* {{{ */
{
	if (crex_unary_op_produces_error(opcode, op)) {
		return 0;
	}

	unary_op_type fn = get_unary_op(opcode);
	fn(result, op);
	return 1;
}
/* }}} */

static inline bool crex_try_ct_eval_unary_pm(zval *result, crex_ast_kind kind, zval *op) /* {{{ */
{
	zval right;
	ZVAL_LONG(&right, (kind == CREX_AST_UNARY_PLUS) ? 1 : -1);
	return crex_try_ct_eval_binary_op(result, CREX_MUL, op, &right);
}
/* }}} */

static inline void crex_ct_eval_greater(zval *result, crex_ast_kind kind, zval *op1, zval *op2) /* {{{ */
{
	binary_op_type fn = kind == CREX_AST_GREATER
		? is_smaller_function : is_smaller_or_equal_function;
	fn(result, op2, op1);
}
/* }}} */

static bool crex_try_ct_eval_array(zval *result, crex_ast *ast) /* {{{ */
{
	crex_ast_list *list = crex_ast_get_list(ast);
	crex_ast *last_elem_ast = NULL;
	uint32_t i;
	bool is_constant = 1;

	if (ast->attr == CREX_ARRAY_SYNTAX_LIST) {
		crex_error(E_COMPILE_ERROR, "Cannot use list() as standalone expression");
	}

	/* First ensure that *all* child nodes are constant and by-val */
	for (i = 0; i < list->children; ++i) {
		crex_ast *elem_ast = list->child[i];

		if (elem_ast == NULL) {
			/* Report error at line of last non-empty element */
			if (last_elem_ast) {
				CG(crex_lineno) = crex_ast_get_lineno(last_elem_ast);
			}
			crex_error(E_COMPILE_ERROR, "Cannot use empty array elements in arrays");
		}

		if (elem_ast->kind != CREX_AST_UNPACK) {
			crex_eval_const_expr(&elem_ast->child[0]);
			crex_eval_const_expr(&elem_ast->child[1]);

			if (elem_ast->attr /* by_ref */ || elem_ast->child[0]->kind != CREX_AST_ZVAL
				|| (elem_ast->child[1] && elem_ast->child[1]->kind != CREX_AST_ZVAL)
			) {
				is_constant = 0;
			}
		} else {
			crex_eval_const_expr(&elem_ast->child[0]);

			if (elem_ast->child[0]->kind != CREX_AST_ZVAL) {
				is_constant = 0;
			}
		}

		last_elem_ast = elem_ast;
	}

	if (!is_constant) {
		return 0;
	}

	if (!list->children) {
		ZVAL_EMPTY_ARRAY(result);
		return 1;
	}

	array_init_size(result, list->children);
	for (i = 0; i < list->children; ++i) {
		crex_ast *elem_ast = list->child[i];
		crex_ast *value_ast = elem_ast->child[0];
		crex_ast *key_ast;

		zval *value = crex_ast_get_zval(value_ast);
		if (elem_ast->kind == CREX_AST_UNPACK) {
			if (C_TYPE_P(value) == IS_ARRAY) {
				HashTable *ht = C_ARRVAL_P(value);
				zval *val;
				crex_string *key;

				CREX_HASH_FOREACH_STR_KEY_VAL(ht, key, val) {
					if (key) {
						crex_hash_update(C_ARRVAL_P(result), key, val);
					} else if (!crex_hash_next_index_insert(C_ARRVAL_P(result), val)) {
						zval_ptr_dtor(result);
						return 0;
					}
					C_TRY_ADDREF_P(val);
				} CREX_HASH_FOREACH_END();

				continue;
			} else {
				crex_error_noreturn(E_COMPILE_ERROR, "Only arrays and Traversables can be unpacked");
			}
		}

		C_TRY_ADDREF_P(value);

		key_ast = elem_ast->child[1];
		if (key_ast) {
			zval *key = crex_ast_get_zval(key_ast);
			switch (C_TYPE_P(key)) {
				case IS_LONG:
					crex_hash_index_update(C_ARRVAL_P(result), C_LVAL_P(key), value);
					break;
				case IS_STRING:
					crex_symtable_update(C_ARRVAL_P(result), C_STR_P(key), value);
					break;
				case IS_DOUBLE: {
					crex_long lval = crex_dval_to_lval(C_DVAL_P(key));
					/* Incompatible float will generate an error, leave this to run-time */
					if (!crex_is_long_compatible(C_DVAL_P(key), lval)) {
						zval_ptr_dtor_nogc(value);
						zval_ptr_dtor(result);
						return 0;
					}
					crex_hash_index_update(C_ARRVAL_P(result), lval, value);
					break;
				}
				case IS_FALSE:
					crex_hash_index_update(C_ARRVAL_P(result), 0, value);
					break;
				case IS_TRUE:
					crex_hash_index_update(C_ARRVAL_P(result), 1, value);
					break;
				case IS_NULL:
					crex_hash_update(C_ARRVAL_P(result), ZSTR_EMPTY_ALLOC(), value);
					break;
				default:
					crex_error_noreturn(E_COMPILE_ERROR, "Illegal offset type");
					break;
			}
		} else if (!crex_hash_next_index_insert(C_ARRVAL_P(result), value)) {
			zval_ptr_dtor_nogc(value);
			zval_ptr_dtor(result);
			return 0;
		}
	}

	return 1;
}
/* }}} */

static void crex_compile_binary_op(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *left_ast = ast->child[0];
	crex_ast *right_ast = ast->child[1];
	uint32_t opcode = ast->attr;

	znode left_node, right_node;

	crex_compile_expr(&left_node, left_ast);
	crex_compile_expr(&right_node, right_ast);

	if (left_node.op_type == IS_CONST && right_node.op_type == IS_CONST) {
		if (crex_try_ct_eval_binary_op(&result->u.constant, opcode,
				&left_node.u.constant, &right_node.u.constant)
		) {
			result->op_type = IS_CONST;
			zval_ptr_dtor(&left_node.u.constant);
			zval_ptr_dtor(&right_node.u.constant);
			return;
		}
	}

	do {
		if (opcode == CREX_IS_EQUAL || opcode == CREX_IS_NOT_EQUAL) {
			if (left_node.op_type == IS_CONST) {
				if (C_TYPE(left_node.u.constant) == IS_FALSE) {
					opcode = (opcode == CREX_IS_NOT_EQUAL) ? CREX_BOOL : CREX_BOOL_NOT;
					crex_emit_op_tmp(result, opcode, &right_node, NULL);
					break;
				} else if (C_TYPE(left_node.u.constant) == IS_TRUE) {
					opcode = (opcode == CREX_IS_EQUAL) ? CREX_BOOL : CREX_BOOL_NOT;
					crex_emit_op_tmp(result, opcode, &right_node, NULL);
					break;
				}
			} else if (right_node.op_type == IS_CONST) {
				if (C_TYPE(right_node.u.constant) == IS_FALSE) {
					opcode = (opcode == CREX_IS_NOT_EQUAL) ? CREX_BOOL : CREX_BOOL_NOT;
					crex_emit_op_tmp(result, opcode, &left_node, NULL);
					break;
				} else if (C_TYPE(right_node.u.constant) == IS_TRUE) {
					opcode = (opcode == CREX_IS_EQUAL) ? CREX_BOOL : CREX_BOOL_NOT;
					crex_emit_op_tmp(result, opcode, &left_node, NULL);
					break;
				}
			}
		} else if (opcode == CREX_IS_IDENTICAL || opcode == CREX_IS_NOT_IDENTICAL) {
			/* convert $x === null to is_null($x) (i.e. CREX_TYPE_CHECK opcode). Do the same thing for false/true. (covers IS_NULL, IS_FALSE, and IS_TRUE) */
			if (left_node.op_type == IS_CONST) {
				if (C_TYPE(left_node.u.constant) <= IS_TRUE && C_TYPE(left_node.u.constant) >= IS_NULL) {
					crex_op *opline = crex_emit_op_tmp(result, CREX_TYPE_CHECK, &right_node, NULL);
					opline->extended_value =
						(opcode == CREX_IS_IDENTICAL) ?
							(1 << C_TYPE(left_node.u.constant)) :
							(MAY_BE_ANY - (1 << C_TYPE(left_node.u.constant)));
					return;
				}
			} else if (right_node.op_type == IS_CONST) {
				if (C_TYPE(right_node.u.constant) <= IS_TRUE && C_TYPE(right_node.u.constant) >= IS_NULL) {
					crex_op *opline = crex_emit_op_tmp(result, CREX_TYPE_CHECK, &left_node, NULL);
					opline->extended_value =
						(opcode == CREX_IS_IDENTICAL) ?
							(1 << C_TYPE(right_node.u.constant)) :
							(MAY_BE_ANY - (1 << C_TYPE(right_node.u.constant)));
					return;
				}
			}
		} else if (opcode == CREX_CONCAT) {
			/* convert constant operands to strings at compile-time */
			if (left_node.op_type == IS_CONST) {
				if (C_TYPE(left_node.u.constant) == IS_ARRAY) {
					crex_emit_op_tmp(&left_node, CREX_CAST, &left_node, NULL)->extended_value = IS_STRING;
				} else {
					convert_to_string(&left_node.u.constant);
				}
			}
			if (right_node.op_type == IS_CONST) {
				if (C_TYPE(right_node.u.constant) == IS_ARRAY) {
					crex_emit_op_tmp(&right_node, CREX_CAST, &right_node, NULL)->extended_value = IS_STRING;
				} else {
					convert_to_string(&right_node.u.constant);
				}
			}
			if (left_node.op_type == IS_CONST && right_node.op_type == IS_CONST) {
				opcode = CREX_FAST_CONCAT;
			}
		}
		crex_emit_op_tmp(result, opcode, &left_node, &right_node);
	} while (0);
}
/* }}} */

/* We do not use crex_compile_binary_op for this because we want to retain the left-to-right
 * evaluation order. */
static void crex_compile_greater(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *left_ast = ast->child[0];
	crex_ast *right_ast = ast->child[1];
	znode left_node, right_node;

	CREX_ASSERT(ast->kind == CREX_AST_GREATER || ast->kind == CREX_AST_GREATER_EQUAL);

	crex_compile_expr(&left_node, left_ast);
	crex_compile_expr(&right_node, right_ast);

	if (left_node.op_type == IS_CONST && right_node.op_type == IS_CONST) {
		result->op_type = IS_CONST;
		crex_ct_eval_greater(&result->u.constant, ast->kind,
			&left_node.u.constant, &right_node.u.constant);
		zval_ptr_dtor(&left_node.u.constant);
		zval_ptr_dtor(&right_node.u.constant);
		return;
	}

	crex_emit_op_tmp(result,
		ast->kind == CREX_AST_GREATER ? CREX_IS_SMALLER : CREX_IS_SMALLER_OR_EQUAL,
		&right_node, &left_node);
}
/* }}} */

static void crex_compile_unary_op(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *expr_ast = ast->child[0];
	uint32_t opcode = ast->attr;

	znode expr_node;
	crex_compile_expr(&expr_node, expr_ast);

	if (expr_node.op_type == IS_CONST
			&& crex_try_ct_eval_unary_op(&result->u.constant, opcode, &expr_node.u.constant)) {
		result->op_type = IS_CONST;
		zval_ptr_dtor(&expr_node.u.constant);
		return;
	}

	crex_emit_op_tmp(result, opcode, &expr_node, NULL);
}
/* }}} */

static void crex_compile_unary_pm(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *expr_ast = ast->child[0];
	znode expr_node, right_node;

	CREX_ASSERT(ast->kind == CREX_AST_UNARY_PLUS || ast->kind == CREX_AST_UNARY_MINUS);

	crex_compile_expr(&expr_node, expr_ast);

	if (expr_node.op_type == IS_CONST
		&& crex_try_ct_eval_unary_pm(&result->u.constant, ast->kind, &expr_node.u.constant)) {
		result->op_type = IS_CONST;
		zval_ptr_dtor(&expr_node.u.constant);
		return;
	}

	right_node.op_type = IS_CONST;
	ZVAL_LONG(&right_node.u.constant, (ast->kind == CREX_AST_UNARY_PLUS) ? 1 : -1);
	crex_emit_op_tmp(result, CREX_MUL, &expr_node, &right_node);
}
/* }}} */

static void crex_compile_short_circuiting(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *left_ast = ast->child[0];
	crex_ast *right_ast = ast->child[1];

	znode left_node, right_node;
	crex_op *opline_jmpz, *opline_bool;
	uint32_t opnum_jmpz;

	CREX_ASSERT(ast->kind == CREX_AST_AND || ast->kind == CREX_AST_OR);

	crex_compile_expr(&left_node, left_ast);

	if (left_node.op_type == IS_CONST) {
		if ((ast->kind == CREX_AST_AND && !crex_is_true(&left_node.u.constant))
		 || (ast->kind == CREX_AST_OR && crex_is_true(&left_node.u.constant))) {
			result->op_type = IS_CONST;
			ZVAL_BOOL(&result->u.constant, crex_is_true(&left_node.u.constant));
		} else {
			crex_compile_expr(&right_node, right_ast);

			if (right_node.op_type == IS_CONST) {
				result->op_type = IS_CONST;
				ZVAL_BOOL(&result->u.constant, crex_is_true(&right_node.u.constant));

				zval_ptr_dtor(&right_node.u.constant);
			} else {
				crex_emit_op_tmp(result, CREX_BOOL, &right_node, NULL);
			}
		}

		zval_ptr_dtor(&left_node.u.constant);
		return;
	}

	opnum_jmpz = get_next_op_number();
	opline_jmpz = crex_emit_op(NULL, ast->kind == CREX_AST_AND ? CREX_JMPC_EX : CREX_JMPNC_EX,
		&left_node, NULL);

	if (left_node.op_type == IS_TMP_VAR) {
		SET_NODE(opline_jmpz->result, &left_node);
		GET_NODE(result, opline_jmpz->result);
	} else {
		crex_make_tmp_result(result, opline_jmpz);
	}

	crex_compile_expr(&right_node, right_ast);

	opline_bool = crex_emit_op(NULL, CREX_BOOL, &right_node, NULL);
	SET_NODE(opline_bool->result, result);

	crex_update_jump_target_to_next(opnum_jmpz);
}
/* }}} */

static void crex_compile_post_incdec(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *var_ast = ast->child[0];
	CREX_ASSERT(ast->kind == CREX_AST_POST_INC || ast->kind == CREX_AST_POST_DEC);

	crex_ensure_writable_variable(var_ast);

	if (var_ast->kind == CREX_AST_PROP || var_ast->kind == CREX_AST_NULLSAFE_PROP) {
		crex_op *opline = crex_compile_prop(NULL, var_ast, BP_VAR_RW, 0);
		opline->opcode = ast->kind == CREX_AST_POST_INC ? CREX_POST_INC_OBJ : CREX_POST_DEC_OBJ;
		crex_make_tmp_result(result, opline);
	} else if (var_ast->kind == CREX_AST_STATIC_PROP) {
		crex_op *opline = crex_compile_static_prop(NULL, var_ast, BP_VAR_RW, 0, 0);
		opline->opcode = ast->kind == CREX_AST_POST_INC ? CREX_POST_INC_STATIC_PROP : CREX_POST_DEC_STATIC_PROP;
		crex_make_tmp_result(result, opline);
	} else {
		znode var_node;
		crex_op *opline = crex_compile_var(&var_node, var_ast, BP_VAR_RW, 0);
		if (opline && opline->opcode == CREX_FETCH_DIM_RW) {
			opline->extended_value = CREX_FETCH_DIM_INCDEC;
		}
		crex_emit_op_tmp(result, ast->kind == CREX_AST_POST_INC ? CREX_POST_INC : CREX_POST_DEC,
			&var_node, NULL);
	}
}
/* }}} */

static void crex_compile_pre_incdec(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *var_ast = ast->child[0];
	CREX_ASSERT(ast->kind == CREX_AST_PRE_INC || ast->kind == CREX_AST_PRE_DEC);

	crex_ensure_writable_variable(var_ast);

	if (var_ast->kind == CREX_AST_PROP || var_ast->kind == CREX_AST_NULLSAFE_PROP) {
		crex_op *opline = crex_compile_prop(result, var_ast, BP_VAR_RW, 0);
		opline->opcode = ast->kind == CREX_AST_PRE_INC ? CREX_PRE_INC_OBJ : CREX_PRE_DEC_OBJ;
		opline->result_type = IS_TMP_VAR;
		result->op_type = IS_TMP_VAR;
	} else if (var_ast->kind == CREX_AST_STATIC_PROP) {
		crex_op *opline = crex_compile_static_prop(result, var_ast, BP_VAR_RW, 0, 0);
		opline->opcode = ast->kind == CREX_AST_PRE_INC ? CREX_PRE_INC_STATIC_PROP : CREX_PRE_DEC_STATIC_PROP;
		opline->result_type = IS_TMP_VAR;
		result->op_type = IS_TMP_VAR;
	} else {
		znode var_node;
		crex_op *opline = crex_compile_var(&var_node, var_ast, BP_VAR_RW, 0);
		if (opline && opline->opcode == CREX_FETCH_DIM_RW) {
			opline->extended_value = CREX_FETCH_DIM_INCDEC;
		}
		crex_emit_op_tmp(result, ast->kind == CREX_AST_PRE_INC ? CREX_PRE_INC : CREX_PRE_DEC,
			&var_node, NULL);
	}
}
/* }}} */

static void crex_compile_cast(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *expr_ast = ast->child[0];
	znode expr_node;
	crex_op *opline;

	crex_compile_expr(&expr_node, expr_ast);

	if (ast->attr == _IS_BOOL) {
		opline = crex_emit_op_tmp(result, CREX_BOOL, &expr_node, NULL);
	} else if (ast->attr == IS_NULL) {
		crex_error(E_COMPILE_ERROR, "The (unset) cast is no longer supported");
	} else {
		opline = crex_emit_op_tmp(result, CREX_CAST, &expr_node, NULL);
		opline->extended_value = ast->attr;
	}
}
/* }}} */

static void crex_compile_shorthand_conditional(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *cond_ast = ast->child[0];
	crex_ast *false_ast = ast->child[2];

	znode cond_node, false_node;
	crex_op *opline_qm_assign;
	uint32_t opnum_jmp_set;

	CREX_ASSERT(ast->child[1] == NULL);

	crex_compile_expr(&cond_node, cond_ast);

	opnum_jmp_set = get_next_op_number();
	crex_emit_op_tmp(result, CREX_JMP_SET, &cond_node, NULL);

	crex_compile_expr(&false_node, false_ast);

	opline_qm_assign = crex_emit_op_tmp(NULL, CREX_QM_ASSIGN, &false_node, NULL);
	SET_NODE(opline_qm_assign->result, result);

	crex_update_jump_target_to_next(opnum_jmp_set);
}
/* }}} */

static void crex_compile_conditional(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *cond_ast = ast->child[0];
	crex_ast *true_ast = ast->child[1];
	crex_ast *false_ast = ast->child[2];

	znode cond_node, true_node, false_node;
	crex_op *opline_qm_assign2;
	uint32_t opnum_jmpz, opnum_jmp;

	if (cond_ast->kind == CREX_AST_CONDITIONAL
			&& cond_ast->attr != CREX_PARENTHESIZED_CONDITIONAL) {
		if (cond_ast->child[1]) {
			if (true_ast) {
				crex_error(E_COMPILE_ERROR,
					"Unparenthesized `a ? b : c ? d : e` is not supported. "
					"Use either `(a ? b : c) ? d : e` or `a ? b : (c ? d : e)`");
			} else {
				crex_error(E_COMPILE_ERROR,
					"Unparenthesized `a ? b : c ?: d` is not supported. "
					"Use either `(a ? b : c) ?: d` or `a ? b : (c ?: d)`");
			}
		} else {
			if (true_ast) {
				crex_error(E_COMPILE_ERROR,
					"Unparenthesized `a ?: b ? c : d` is not supported. "
					"Use either `(a ?: b) ? c : d` or `a ?: (b ? c : d)`");
			} else {
				/* This case is harmless:  (a ?: b) ?: c always produces the same result
				 * as a ?: (b ?: c). */
			}
		}
	}

	if (!true_ast) {
		crex_compile_shorthand_conditional(result, ast);
		return;
	}

	crex_compile_expr(&cond_node, cond_ast);

	opnum_jmpz = crex_emit_cond_jump(CREX_JMPZ, &cond_node, 0);

	crex_compile_expr(&true_node, true_ast);

	crex_emit_op_tmp(result, CREX_QM_ASSIGN, &true_node, NULL);

	opnum_jmp = crex_emit_jump(0);

	crex_update_jump_target_to_next(opnum_jmpz);

	crex_compile_expr(&false_node, false_ast);

	opline_qm_assign2 = crex_emit_op(NULL, CREX_QM_ASSIGN, &false_node, NULL);
	SET_NODE(opline_qm_assign2->result, result);

	crex_update_jump_target_to_next(opnum_jmp);
}
/* }}} */

static void crex_compile_coalesce(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *expr_ast = ast->child[0];
	crex_ast *default_ast = ast->child[1];

	znode expr_node, default_node;
	crex_op *opline;
	uint32_t opnum;

	crex_compile_var(&expr_node, expr_ast, BP_VAR_IS, 0);

	opnum = get_next_op_number();
	crex_emit_op_tmp(result, CREX_COALESCE, &expr_node, NULL);

	crex_compile_expr(&default_node, default_ast);

	opline = crex_emit_op_tmp(NULL, CREX_QM_ASSIGN, &default_node, NULL);
	SET_NODE(opline->result, result);

	opline = &CG(active_op_array)->opcodes[opnum];
	opline->op2.opline_num = get_next_op_number();
}
/* }}} */

static void znode_dtor(zval *zv) {
	znode *node = C_PTR_P(zv);
	if (node->op_type == IS_CONST) {
		zval_ptr_dtor_nogc(&node->u.constant);
	}
	efree(node);
}

static void crex_compile_assign_coalesce(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *var_ast = ast->child[0];
	crex_ast *default_ast = ast->child[1];

	znode var_node_is, var_node_w, default_node, assign_node, *node;
	crex_op *opline;
	uint32_t coalesce_opnum;
	bool need_frees = 0;

	/* Remember expressions compiled during the initial BP_VAR_IS lookup,
	 * to avoid double-evaluation when we compile again with BP_VAR_W. */
	HashTable *orig_memoized_exprs = CG(memoized_exprs);
	const crex_memoize_mode orig_memoize_mode = CG(memoize_mode);

	crex_ensure_writable_variable(var_ast);
	if (is_this_fetch(var_ast)) {
		crex_error_noreturn(E_COMPILE_ERROR, "Cannot re-assign $this");
	}

	ALLOC_HASHTABLE(CG(memoized_exprs));
	crex_hash_init(CG(memoized_exprs), 0, NULL, znode_dtor, 0);

	CG(memoize_mode) = CREX_MEMOIZE_COMPILE;
	crex_compile_var(&var_node_is, var_ast, BP_VAR_IS, 0);

	coalesce_opnum = get_next_op_number();
	crex_emit_op_tmp(result, CREX_COALESCE, &var_node_is, NULL);

	CG(memoize_mode) = CREX_MEMOIZE_NONE;
	if (var_ast->kind == CREX_AST_DIM) {
		crex_compile_expr_with_potential_assign_to_self(&default_node, default_ast, var_ast);
	} else {
		crex_compile_expr(&default_node, default_ast);
	}

	CG(memoize_mode) = CREX_MEMOIZE_FETCH;
	crex_compile_var(&var_node_w, var_ast, BP_VAR_W, 0);

	/* Reproduce some of the crex_compile_assign() opcode fixup logic here. */
	opline = &CG(active_op_array)->opcodes[CG(active_op_array)->last-1];
	/* Treat $GLOBALS['x'] assignment like assignment to variable. */
	crex_ast_kind kind = is_global_var_fetch(var_ast) ? CREX_AST_VAR : var_ast->kind;
	switch (kind) {
		case CREX_AST_VAR:
			crex_emit_op_tmp(&assign_node, CREX_ASSIGN, &var_node_w, &default_node);
			break;
		case CREX_AST_STATIC_PROP:
			opline->opcode = CREX_ASSIGN_STATIC_PROP;
			opline->result_type = IS_TMP_VAR;
			var_node_w.op_type = IS_TMP_VAR;
			crex_emit_op_data(&default_node);
			assign_node = var_node_w;
			break;
		case CREX_AST_DIM:
			opline->opcode = CREX_ASSIGN_DIM;
			opline->result_type = IS_TMP_VAR;
			var_node_w.op_type = IS_TMP_VAR;
			crex_emit_op_data(&default_node);
			assign_node = var_node_w;
			break;
		case CREX_AST_PROP:
		case CREX_AST_NULLSAFE_PROP:
			opline->opcode = CREX_ASSIGN_OBJ;
			opline->result_type = IS_TMP_VAR;
			var_node_w.op_type = IS_TMP_VAR;
			crex_emit_op_data(&default_node);
			assign_node = var_node_w;
			break;
		EMPTY_SWITCH_DEFAULT_CASE();
	}

	opline = crex_emit_op_tmp(NULL, CREX_QM_ASSIGN, &assign_node, NULL);
	SET_NODE(opline->result, result);

	CREX_HASH_FOREACH_PTR(CG(memoized_exprs), node) {
		if (node->op_type == IS_TMP_VAR || node->op_type == IS_VAR) {
			need_frees = 1;
			break;
		}
	} CREX_HASH_FOREACH_END();

	/* Free DUPed expressions if there are any */
	if (need_frees) {
		uint32_t jump_opnum = crex_emit_jump(0);
		crex_update_jump_target_to_next(coalesce_opnum);
		CREX_HASH_FOREACH_PTR(CG(memoized_exprs), node) {
			if (node->op_type == IS_TMP_VAR || node->op_type == IS_VAR) {
				crex_emit_op(NULL, CREX_FREE, node, NULL);
			}
		} CREX_HASH_FOREACH_END();
		crex_update_jump_target_to_next(jump_opnum);
	} else {
		crex_update_jump_target_to_next(coalesce_opnum);
	}

	crex_hash_destroy(CG(memoized_exprs));
	FREE_HASHTABLE(CG(memoized_exprs));
	CG(memoized_exprs) = orig_memoized_exprs;
	CG(memoize_mode) = orig_memoize_mode;
}
/* }}} */

static void crex_compile_print(znode *result, crex_ast *ast) /* {{{ */
{
	crex_op *opline;
	crex_ast *expr_ast = ast->child[0];

	znode expr_node;
	crex_compile_expr(&expr_node, expr_ast);

	opline = crex_emit_op(NULL, CREX_ECHO, &expr_node, NULL);
	opline->extended_value = 1;

	result->op_type = IS_CONST;
	ZVAL_LONG(&result->u.constant, 1);
}
/* }}} */

static void crex_compile_exit(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *expr_ast = ast->child[0];
	znode expr_node;

	if (expr_ast) {
		crex_compile_expr(&expr_node, expr_ast);
	} else {
		expr_node.op_type = IS_UNUSED;
	}

	crex_op *opline = crex_emit_op(NULL, CREX_EXIT, &expr_node, NULL);
	if (result) {
		/* Mark this as an "expression throw" for opcache. */
		opline->extended_value = CREX_THROW_IS_EXPR;
		result->op_type = IS_CONST;
		ZVAL_TRUE(&result->u.constant);
	}
}
/* }}} */

static void crex_compile_yield(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *value_ast = ast->child[0];
	crex_ast *key_ast = ast->child[1];

	znode value_node, key_node;
	znode *value_node_ptr = NULL, *key_node_ptr = NULL;
	crex_op *opline;
	bool returns_by_ref = (CG(active_op_array)->fn_flags & CREX_ACC_RETURN_REFERENCE) != 0;

	crex_mark_function_as_generator();

	if (key_ast) {
		crex_compile_expr(&key_node, key_ast);
		key_node_ptr = &key_node;
	}

	if (value_ast) {
		if (returns_by_ref && crex_is_variable(value_ast)) {
			crex_compile_var(&value_node, value_ast, BP_VAR_W, 1);
		} else {
			crex_compile_expr(&value_node, value_ast);
		}
		value_node_ptr = &value_node;
	}

	opline = crex_emit_op(result, CREX_YIELD, value_node_ptr, key_node_ptr);

	if (value_ast && returns_by_ref && crex_is_call(value_ast)) {
		opline->extended_value = CREX_RETURNS_FUNCTION;
	}
}
/* }}} */

static void crex_compile_yield_from(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *expr_ast = ast->child[0];
	znode expr_node;

	crex_mark_function_as_generator();

	if (CG(active_op_array)->fn_flags & CREX_ACC_RETURN_REFERENCE) {
		crex_error_noreturn(E_COMPILE_ERROR,
			"Cannot use \"yield from\" inside a by-reference generator");
	}

	crex_compile_expr(&expr_node, expr_ast);
	crex_emit_op_tmp(result, CREX_YIELD_FROM, &expr_node, NULL);
}
/* }}} */

static void crex_compile_instanceof(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *obj_ast = ast->child[0];
	crex_ast *class_ast = ast->child[1];

	znode obj_node, class_node;
	crex_op *opline;

	crex_compile_expr(&obj_node, obj_ast);
	if (obj_node.op_type == IS_CONST) {
		crex_do_free(&obj_node);
		result->op_type = IS_CONST;
		ZVAL_FALSE(&result->u.constant);
		return;
	}

	crex_compile_class_ref(&class_node, class_ast,
		CREX_FETCH_CLASS_NO_AUTOLOAD | CREX_FETCH_CLASS_EXCEPTION | CREX_FETCH_CLASS_SILENT);

	opline = crex_emit_op_tmp(result, CREX_INSTANCEOF, &obj_node, NULL);

	if (class_node.op_type == IS_CONST) {
		opline->op2_type = IS_CONST;
		opline->op2.constant = crex_add_class_name_literal(
			C_STR(class_node.u.constant));
		opline->extended_value = crex_alloc_cache_slot();
	} else {
		SET_NODE(opline->op2, &class_node);
	}
}
/* }}} */

static void crex_compile_include_or_eval(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *expr_ast = ast->child[0];
	znode expr_node;
	crex_op *opline;

	crex_do_extended_fcall_begin();
	crex_compile_expr(&expr_node, expr_ast);

	opline = crex_emit_op(result, CREX_INCLUDE_OR_EVAL, &expr_node, NULL);
	opline->extended_value = ast->attr;

	crex_do_extended_fcall_end();
}
/* }}} */

static void crex_compile_isset_or_empty(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *var_ast = ast->child[0];

	znode var_node;
	crex_op *opline = NULL;

	CREX_ASSERT(ast->kind == CREX_AST_ISSET || ast->kind == CREX_AST_EMPTY);

	if (!crex_is_variable(var_ast)) {
		if (ast->kind == CREX_AST_EMPTY) {
			/* empty(expr) can be transformed to !expr */
			crex_ast *not_ast = crex_ast_create_ex(CREX_AST_UNARY_OP, CREX_BOOL_NOT, var_ast);
			crex_compile_expr(result, not_ast);
			return;
		} else {
			crex_error_noreturn(E_COMPILE_ERROR,
				"Cannot use isset() on the result of an expression "
				"(you can use \"null !== expression\" instead)");
		}
	}

	if (is_globals_fetch(var_ast)) {
		result->op_type = IS_CONST;
		ZVAL_BOOL(&result->u.constant, ast->kind == CREX_AST_ISSET);
		return;
	}

	if (is_global_var_fetch(var_ast)) {
		if (!var_ast->child[1]) {
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot use [] for reading");
		}

		crex_compile_expr(&var_node, var_ast->child[1]);
		if (var_node.op_type == IS_CONST) {
			convert_to_string(&var_node.u.constant);
		}

		opline = crex_emit_op_tmp(result, CREX_ISSET_ISEMPTY_VAR, &var_node, NULL);
		opline->extended_value =
			CREX_FETCH_GLOBAL | (ast->kind == CREX_AST_EMPTY ? CREX_ISEMPTY : 0);
		return;
	}

	crex_short_circuiting_mark_inner(var_ast);
	switch (var_ast->kind) {
		case CREX_AST_VAR:
			if (is_this_fetch(var_ast)) {
				opline = crex_emit_op(result, CREX_ISSET_ISEMPTY_THIS, NULL, NULL);
				CG(active_op_array)->fn_flags |= CREX_ACC_USES_THIS;
			} else if (crex_try_compile_cv(&var_node, var_ast) == SUCCESS) {
				opline = crex_emit_op(result, CREX_ISSET_ISEMPTY_CV, &var_node, NULL);
			} else {
				opline = crex_compile_simple_var_no_cv(result, var_ast, BP_VAR_IS, 0);
				opline->opcode = CREX_ISSET_ISEMPTY_VAR;
			}
			break;
		case CREX_AST_DIM:
			opline = crex_compile_dim(result, var_ast, BP_VAR_IS, /* by_ref */ false);
			opline->opcode = CREX_ISSET_ISEMPTY_DIM_OBJ;
			break;
		case CREX_AST_PROP:
		case CREX_AST_NULLSAFE_PROP:
			opline = crex_compile_prop(result, var_ast, BP_VAR_IS, 0);
			opline->opcode = CREX_ISSET_ISEMPTY_PROP_OBJ;
			break;
		case CREX_AST_STATIC_PROP:
			opline = crex_compile_static_prop(result, var_ast, BP_VAR_IS, 0, 0);
			opline->opcode = CREX_ISSET_ISEMPTY_STATIC_PROP;
			break;
		EMPTY_SWITCH_DEFAULT_CASE()
	}

	result->op_type = opline->result_type = IS_TMP_VAR;
	if (!(ast->kind == CREX_AST_ISSET)) {
		opline->extended_value |= CREX_ISEMPTY;
	}
}
/* }}} */

static void crex_compile_silence(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *expr_ast = ast->child[0];
	znode silence_node;

	crex_emit_op_tmp(&silence_node, CREX_BEGIN_SILENCE, NULL, NULL);

	if (expr_ast->kind == CREX_AST_VAR) {
		/* For @$var we need to force a FETCH instruction, otherwise the CV access will
		 * happen outside the silenced section. */
		crex_compile_simple_var_no_cv(result, expr_ast, BP_VAR_R, 0 );
	} else {
		crex_compile_expr(result, expr_ast);
	}

	crex_emit_op(NULL, CREX_END_SILENCE, &silence_node, NULL);
}
/* }}} */

static void crex_compile_shell_exec(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *expr_ast = ast->child[0];

	zval fn_name;
	crex_ast *name_ast, *args_ast, *call_ast;

	ZVAL_STRING(&fn_name, "shell_exec");
	name_ast = crex_ast_create_zval(&fn_name);
	args_ast = crex_ast_create_list(1, CREX_AST_ARG_LIST, expr_ast);
	call_ast = crex_ast_create(CREX_AST_CALL, name_ast, args_ast);

	crex_compile_expr(result, call_ast);

	zval_ptr_dtor(&fn_name);
}
/* }}} */

static void crex_compile_array(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast_list *list = crex_ast_get_list(ast);
	crex_op *opline;
	uint32_t i, opnum_init = -1;
	bool packed = 1;

	if (crex_try_ct_eval_array(&result->u.constant, ast)) {
		result->op_type = IS_CONST;
		return;
	}

	/* Empty arrays are handled at compile-time */
	CREX_ASSERT(list->children > 0);

	for (i = 0; i < list->children; ++i) {
		crex_ast *elem_ast = list->child[i];
		crex_ast *value_ast, *key_ast;
		bool by_ref;
		znode value_node, key_node, *key_node_ptr = NULL;

		if (elem_ast == NULL) {
			crex_error(E_COMPILE_ERROR, "Cannot use empty array elements in arrays");
		}

		value_ast = elem_ast->child[0];

		if (elem_ast->kind == CREX_AST_UNPACK) {
			crex_compile_expr(&value_node, value_ast);
			if (i == 0) {
				opnum_init = get_next_op_number();
				opline = crex_emit_op_tmp(result, CREX_INIT_ARRAY, NULL, NULL);
			}
			opline = crex_emit_op(NULL, CREX_ADD_ARRAY_UNPACK, &value_node, NULL);
			SET_NODE(opline->result, result);
			continue;
		}

		key_ast = elem_ast->child[1];
		by_ref = elem_ast->attr;

		if (key_ast) {
			crex_compile_expr(&key_node, key_ast);
			crex_handle_numeric_op(&key_node);
			key_node_ptr = &key_node;
		}

		if (by_ref) {
			crex_ensure_writable_variable(value_ast);
			crex_compile_var(&value_node, value_ast, BP_VAR_W, 1);
		} else {
			crex_compile_expr(&value_node, value_ast);
		}

		if (i == 0) {
			opnum_init = get_next_op_number();
			opline = crex_emit_op_tmp(result, CREX_INIT_ARRAY, &value_node, key_node_ptr);
			opline->extended_value = list->children << CREX_ARRAY_SIZE_SHIFT;
		} else {
			opline = crex_emit_op(NULL, CREX_ADD_ARRAY_ELEMENT,
				&value_node, key_node_ptr);
			SET_NODE(opline->result, result);
		}
		opline->extended_value |= by_ref;

		if (key_ast && key_node.op_type == IS_CONST && C_TYPE(key_node.u.constant) == IS_STRING) {
			packed = 0;
		}
	}

	/* Add a flag to INIT_ARRAY if we know this array cannot be packed */
	if (!packed) {
		CREX_ASSERT(opnum_init != (uint32_t)-1);
		opline = &CG(active_op_array)->opcodes[opnum_init];
		opline->extended_value |= CREX_ARRAY_NOT_PACKED;
	}
}
/* }}} */

static void crex_compile_const(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *name_ast = ast->child[0];

	crex_op *opline;

	bool is_fully_qualified;
	crex_string *orig_name = crex_ast_get_str(name_ast);
	crex_string *resolved_name = crex_resolve_const_name(orig_name, name_ast->attr, &is_fully_qualified);

	if (crex_string_equals_literal(resolved_name, "__COMPILER_HALT_OFFSET__") || (name_ast->attr != CREX_NAME_RELATIVE && crex_string_equals_literal(orig_name, "__COMPILER_HALT_OFFSET__"))) {
		crex_ast *last = CG(ast);

		while (last && last->kind == CREX_AST_STMT_LIST) {
			crex_ast_list *list = crex_ast_get_list(last);
			if (list->children == 0) {
				break;
			}
			last = list->child[list->children-1];
		}
		if (last && last->kind == CREX_AST_HALT_COMPILER) {
			result->op_type = IS_CONST;
			ZVAL_LONG(&result->u.constant, C_LVAL_P(crex_ast_get_zval(last->child[0])));
			crex_string_release_ex(resolved_name, 0);
			return;
		}
	}

	if (crex_try_ct_eval_const(&result->u.constant, resolved_name, is_fully_qualified)) {
		result->op_type = IS_CONST;
		crex_string_release_ex(resolved_name, 0);
		return;
	}

	opline = crex_emit_op_tmp(result, CREX_FETCH_CONSTANT, NULL, NULL);
	opline->op2_type = IS_CONST;

	if (is_fully_qualified || !FC(current_namespace)) {
		opline->op1.num = 0;
		opline->op2.constant = crex_add_const_name_literal(
			resolved_name, 0);
	} else {
		opline->op1.num = IS_CONSTANT_UNQUALIFIED_IN_NAMESPACE;
		opline->op2.constant = crex_add_const_name_literal(
			resolved_name, 1);
	}
	opline->extended_value = crex_alloc_cache_slot();
}
/* }}} */

static void crex_compile_class_const(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *class_ast;
	crex_ast *const_ast;
	znode class_node, const_node;
	crex_op *opline;

	crex_eval_const_expr(&ast->child[0]);
	crex_eval_const_expr(&ast->child[1]);

	class_ast = ast->child[0];
	const_ast = ast->child[1];

	if (class_ast->kind == CREX_AST_ZVAL && const_ast->kind == CREX_AST_ZVAL) {
		zval *const_zv = crex_ast_get_zval(const_ast);
		if (C_TYPE_P(const_zv) == IS_STRING) {
			crex_string *const_str = C_STR_P(const_zv);
			crex_string *resolved_name = crex_resolve_class_name_ast(class_ast);
			if (crex_try_ct_eval_class_const(&result->u.constant, resolved_name, const_str)) {
				result->op_type = IS_CONST;
				crex_string_release_ex(resolved_name, 0);
				return;
			}
			crex_string_release_ex(resolved_name, 0);
		}
	}

	crex_compile_class_ref(&class_node, class_ast, CREX_FETCH_CLASS_EXCEPTION);

	crex_compile_expr(&const_node, const_ast);

	opline = crex_emit_op_tmp(result, CREX_FETCH_CLASS_CONSTANT, NULL, &const_node);

	crex_set_class_name_op1(opline, &class_node);

	if (opline->op1_type == IS_CONST || opline->op2_type == IS_CONST) {
		opline->extended_value = crex_alloc_cache_slots(2);
	}
}
/* }}} */

static void crex_compile_class_name(znode *result, crex_ast *ast) /* {{{ */
{
	crex_ast *class_ast = ast->child[0];

	if (crex_try_compile_const_expr_resolve_class_name(&result->u.constant, class_ast)) {
		result->op_type = IS_CONST;
		return;
	}

	if (class_ast->kind == CREX_AST_ZVAL) {
		crex_op *opline = crex_emit_op_tmp(result, CREX_FETCH_CLASS_NAME, NULL, NULL);
		opline->op1.num = crex_get_class_fetch_type(crex_ast_get_str(class_ast));
	} else {
		znode expr_node;
		crex_compile_expr(&expr_node, class_ast);
		if (expr_node.op_type == IS_CONST) {
			/* Unlikely case that happen if class_ast is constant folded.
			 * Handle it here, to avoid needing a CONST specialization in the VM. */
			crex_error_noreturn(E_COMPILE_ERROR, "Cannot use \"::class\" on %s",
				crex_zval_value_name(&expr_node.u.constant));
		}

		crex_emit_op_tmp(result, CREX_FETCH_CLASS_NAME, &expr_node, NULL);
	}
}
/* }}} */

static crex_op *crex_compile_rope_add_ex(crex_op *opline, znode *result, uint32_t num, znode *elem_node) /* {{{ */
{
	if (num == 0) {
		result->op_type = IS_TMP_VAR;
		result->u.op.var = -1;
		opline->opcode = CREX_ROPE_INIT;
	} else {
		opline->opcode = CREX_ROPE_ADD;
		SET_NODE(opline->op1, result);
	}
	SET_NODE(opline->op2, elem_node);
	SET_NODE(opline->result, result);
	opline->extended_value = num;
	return opline;
}
/* }}} */

static crex_op *crex_compile_rope_add(znode *result, uint32_t num, znode *elem_node) /* {{{ */
{
	crex_op *opline = get_next_op();

	if (num == 0) {
		result->op_type = IS_TMP_VAR;
		result->u.op.var = -1;
		opline->opcode = CREX_ROPE_INIT;
	} else {
		opline->opcode = CREX_ROPE_ADD;
		SET_NODE(opline->op1, result);
	}
	SET_NODE(opline->op2, elem_node);
	SET_NODE(opline->result, result);
	opline->extended_value = num;
	return opline;
}
/* }}} */

static void crex_compile_encaps_list(znode *result, crex_ast *ast) /* {{{ */
{
	uint32_t i, j;
	uint32_t rope_init_lineno = -1;
	crex_op *opline = NULL, *init_opline;
	znode elem_node, last_const_node;
	crex_ast_list *list = crex_ast_get_list(ast);
	uint32_t reserved_op_number = -1;

	CREX_ASSERT(list->children > 0);

	j = 0;
	last_const_node.op_type = IS_UNUSED;
	for (i = 0; i < list->children; i++) {
		crex_ast *encaps_var = list->child[i];

		if (encaps_var->attr & (CREX_ENCAPS_VAR_DOLLAR_CURLY|CREX_ENCAPS_VAR_DOLLAR_CURLY_VAR_VAR)) {
			if ((encaps_var->kind == CREX_AST_VAR || encaps_var->kind == CREX_AST_DIM) && (encaps_var->attr & CREX_ENCAPS_VAR_DOLLAR_CURLY)) {
				crex_error(E_DEPRECATED, "Using ${var} in strings is deprecated, use {$var} instead");
			} else if (encaps_var->kind == CREX_AST_VAR && (encaps_var->attr & CREX_ENCAPS_VAR_DOLLAR_CURLY_VAR_VAR)) {
				crex_error(E_DEPRECATED, "Using ${expr} (variable variables) in strings is deprecated, use {${expr}} instead");
			}
		}

		crex_compile_expr(&elem_node, encaps_var);

		if (elem_node.op_type == IS_CONST) {
			convert_to_string(&elem_node.u.constant);

			if (C_STRLEN(elem_node.u.constant) == 0) {
				zval_ptr_dtor(&elem_node.u.constant);
			} else if (last_const_node.op_type == IS_CONST) {
				concat_function(&last_const_node.u.constant, &last_const_node.u.constant, &elem_node.u.constant);
				zval_ptr_dtor(&elem_node.u.constant);
			} else {
				last_const_node.op_type = IS_CONST;
				ZVAL_COPY_VALUE(&last_const_node.u.constant, &elem_node.u.constant);
				/* Reserve place for CREX_ROPE_ADD instruction */
				reserved_op_number = get_next_op_number();
				opline = get_next_op();
				opline->opcode = CREX_NOP;
			}
			continue;
		} else {
			if (j == 0) {
				if (last_const_node.op_type == IS_CONST) {
					rope_init_lineno = reserved_op_number;
				} else {
					rope_init_lineno = get_next_op_number();
				}
			}
			if (last_const_node.op_type == IS_CONST) {
				opline = &CG(active_op_array)->opcodes[reserved_op_number];
				crex_compile_rope_add_ex(opline, result, j++, &last_const_node);
				last_const_node.op_type = IS_UNUSED;
			}
			opline = crex_compile_rope_add(result, j++, &elem_node);
		}
	}

	if (j == 0) {
		result->op_type = IS_CONST;
		if (last_const_node.op_type == IS_CONST) {
			ZVAL_COPY_VALUE(&result->u.constant, &last_const_node.u.constant);
		} else {
			ZVAL_EMPTY_STRING(&result->u.constant);
			/* empty string */
		}
		CG(active_op_array)->last = reserved_op_number - 1;
		return;
	} else if (last_const_node.op_type == IS_CONST) {
		opline = &CG(active_op_array)->opcodes[reserved_op_number];
		opline = crex_compile_rope_add_ex(opline, result, j++, &last_const_node);
	}
	init_opline = CG(active_op_array)->opcodes + rope_init_lineno;
	if (j == 1) {
		if (opline->op2_type == IS_CONST) {
			GET_NODE(result, opline->op2);
			MAKE_NOP(opline);
		} else {
			opline->opcode = CREX_CAST;
			opline->extended_value = IS_STRING;
			opline->op1_type = opline->op2_type;
			opline->op1 = opline->op2;
			SET_UNUSED(opline->op2);
			crex_make_tmp_result(result, opline);
		}
	} else if (j == 2) {
		opline->opcode = CREX_FAST_CONCAT;
		opline->extended_value = 0;
		opline->op1_type = init_opline->op2_type;
		opline->op1 = init_opline->op2;
		crex_make_tmp_result(result, opline);
		MAKE_NOP(init_opline);
	} else {
		uint32_t var;

		init_opline->extended_value = j;
		opline->opcode = CREX_ROPE_END;
		crex_make_tmp_result(result, opline);
		var = opline->op1.var = get_temporary_variable();

		/* Allocates the necessary number of zval slots to keep the rope */
		i = ((j * sizeof(crex_string*)) + (sizeof(zval) - 1)) / sizeof(zval);
		while (i > 1) {
			get_temporary_variable();
			i--;
		}

		/* Update all the previous opcodes to use the same variable */
		while (opline != init_opline) {
			opline--;
			if (opline->opcode == CREX_ROPE_ADD &&
			    opline->result.var == (uint32_t)-1) {
				opline->op1.var = var;
				opline->result.var = var;
			} else if (opline->opcode == CREX_ROPE_INIT &&
			           opline->result.var == (uint32_t)-1) {
				opline->result.var = var;
			}
		}
	}
}
/* }}} */

static void crex_compile_magic_const(znode *result, crex_ast *ast) /* {{{ */
{
	crex_op *opline;

	if (crex_try_ct_eval_magic_const(&result->u.constant, ast)) {
		result->op_type = IS_CONST;
		return;
	}

	CREX_ASSERT(ast->attr == T_CLASS_C &&
	            CG(active_class_entry) &&
	            (CG(active_class_entry)->ce_flags & CREX_ACC_TRAIT) != 0);

	opline = crex_emit_op_tmp(result, CREX_FETCH_CLASS_NAME, NULL, NULL);
	opline->op1.num = CREX_FETCH_CLASS_SELF;
}
/* }}} */

static bool crex_is_allowed_in_const_expr(crex_ast_kind kind) /* {{{ */
{
	return kind == CREX_AST_ZVAL || kind == CREX_AST_BINARY_OP
		|| kind == CREX_AST_GREATER || kind == CREX_AST_GREATER_EQUAL
		|| kind == CREX_AST_AND || kind == CREX_AST_OR
		|| kind == CREX_AST_UNARY_OP
		|| kind == CREX_AST_UNARY_PLUS || kind == CREX_AST_UNARY_MINUS
		|| kind == CREX_AST_CONDITIONAL || kind == CREX_AST_DIM
		|| kind == CREX_AST_ARRAY || kind == CREX_AST_ARRAY_ELEM
		|| kind == CREX_AST_UNPACK
		|| kind == CREX_AST_CONST || kind == CREX_AST_CLASS_CONST
		|| kind == CREX_AST_CLASS_NAME
		|| kind == CREX_AST_MAGIC_CONST || kind == CREX_AST_COALESCE
		|| kind == CREX_AST_CONST_ENUM_INIT
		|| kind == CREX_AST_NEW || kind == CREX_AST_ARG_LIST
		|| kind == CREX_AST_NAMED_ARG
		|| kind == CREX_AST_PROP || kind == CREX_AST_NULLSAFE_PROP;
}
/* }}} */

static void crex_compile_const_expr_class_const(crex_ast **ast_ptr) /* {{{ */
{
	crex_ast *ast = *ast_ptr;
	crex_ast *class_ast = ast->child[0];
	crex_string *class_name;
	int fetch_type;

	if (class_ast->kind != CREX_AST_ZVAL) {
		crex_error_noreturn(E_COMPILE_ERROR,
			"Dynamic class names are not allowed in compile-time class constant references");
	}
	if (C_TYPE_P(crex_ast_get_zval(class_ast)) != IS_STRING) {
		crex_throw_error(NULL, "Class name must be a valid object or a string");
	}

	class_name = crex_ast_get_str(class_ast);
	fetch_type = crex_get_class_fetch_type(class_name);

	if (CREX_FETCH_CLASS_STATIC == fetch_type) {
		crex_error_noreturn(E_COMPILE_ERROR,
			"\"static::\" is not allowed in compile-time constants");
	}

	if (CREX_FETCH_CLASS_DEFAULT == fetch_type) {
		crex_string *tmp = crex_resolve_class_name_ast(class_ast);

		crex_string_release_ex(class_name, 0);
		if (tmp != class_name) {
			zval *zv = crex_ast_get_zval(class_ast);
			ZVAL_STR(zv, tmp);
			class_ast->attr = CREX_NAME_FQ;
		}
	}

	ast->attr |= CREX_FETCH_CLASS_EXCEPTION;
}
/* }}} */

static void crex_compile_const_expr_class_name(crex_ast **ast_ptr) /* {{{ */
{
	crex_ast *ast = *ast_ptr;
	crex_ast *class_ast = ast->child[0];
	if (class_ast->kind != CREX_AST_ZVAL) {
		crex_error_noreturn(E_COMPILE_ERROR,
			"(expression)::class cannot be used in constant expressions");
	}

	crex_string *class_name = crex_ast_get_str(class_ast);
	uint32_t fetch_type = crex_get_class_fetch_type(class_name);

	switch (fetch_type) {
		case CREX_FETCH_CLASS_SELF:
		case CREX_FETCH_CLASS_PARENT:
			/* For the const-eval representation store the fetch type instead of the name. */
			crex_string_release(class_name);
			ast->child[0] = NULL;
			ast->attr = fetch_type;
			return;
		case CREX_FETCH_CLASS_STATIC:
			crex_error_noreturn(E_COMPILE_ERROR,
				"static::class cannot be used for compile-time class name resolution");
			return;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
}

static void crex_compile_const_expr_const(crex_ast **ast_ptr) /* {{{ */
{
	crex_ast *ast = *ast_ptr;
	crex_ast *name_ast = ast->child[0];
	crex_string *orig_name = crex_ast_get_str(name_ast);
	bool is_fully_qualified;
	zval result;
	crex_string *resolved_name;

	CG(crex_lineno) = crex_ast_get_lineno(ast);

	resolved_name = crex_resolve_const_name(
		orig_name, name_ast->attr, &is_fully_qualified);

	if (crex_try_ct_eval_const(&result, resolved_name, is_fully_qualified)) {
		crex_string_release_ex(resolved_name, 0);
		crex_ast_destroy(ast);
		*ast_ptr = crex_ast_create_zval(&result);
		return;
	}

	crex_ast_destroy(ast);
	*ast_ptr = crex_ast_create_constant(resolved_name,
		!is_fully_qualified && FC(current_namespace) ? IS_CONSTANT_UNQUALIFIED_IN_NAMESPACE : 0);
}
/* }}} */

static void crex_compile_const_expr_magic_const(crex_ast **ast_ptr) /* {{{ */
{
	crex_ast *ast = *ast_ptr;

	/* Other cases already resolved by constant folding */
	CREX_ASSERT(ast->attr == T_CLASS_C);

	crex_ast_destroy(ast);
	*ast_ptr = crex_ast_create(CREX_AST_CONSTANT_CLASS);
}
/* }}} */

static void crex_compile_const_expr_new(crex_ast **ast_ptr)
{
	crex_ast *class_ast = (*ast_ptr)->child[0];
	if (class_ast->kind == CREX_AST_CLASS) {
		crex_error_noreturn(E_COMPILE_ERROR,
			"Cannot use anonymous class in constant expression");
	}
	if (class_ast->kind != CREX_AST_ZVAL) {
		crex_error_noreturn(E_COMPILE_ERROR,
			"Cannot use dynamic class name in constant expression");
	}

	crex_string *class_name = crex_resolve_class_name_ast(class_ast);
	int fetch_type = crex_get_class_fetch_type(class_name);
	if (CREX_FETCH_CLASS_STATIC == fetch_type) {
		crex_error_noreturn(E_COMPILE_ERROR,
			"\"static\" is not allowed in compile-time constants");
	}

	zval *class_ast_zv = crex_ast_get_zval(class_ast);
	zval_ptr_dtor_nogc(class_ast_zv);
	ZVAL_STR(class_ast_zv, class_name);
	class_ast->attr = fetch_type << CREX_CONST_EXPR_NEW_FETCH_TYPE_SHIFT;
}

static void crex_compile_const_expr_args(crex_ast **ast_ptr)
{
	crex_ast_list *list = crex_ast_get_list(*ast_ptr);
	bool uses_named_args = false;
	for (uint32_t i = 0; i < list->children; i++) {
		crex_ast *arg = list->child[i];
		if (arg->kind == CREX_AST_UNPACK) {
			crex_error_noreturn(E_COMPILE_ERROR,
				"Argument unpacking in constant expressions is not supported");
		}
		if (arg->kind == CREX_AST_NAMED_ARG) {
			uses_named_args = true;
		} else if (uses_named_args) {
			crex_error_noreturn(E_COMPILE_ERROR,
				"Cannot use positional argument after named argument");
		}
	}
	if (uses_named_args) {
		list->attr = 1;
	}
}

typedef struct {
	/* Whether the value of this expression may differ on each evaluation. */
	bool allow_dynamic;
} const_expr_context;

static void crex_compile_const_expr(crex_ast **ast_ptr, void *context) /* {{{ */
{
	const_expr_context *ctx = (const_expr_context *) context;
	crex_ast *ast = *ast_ptr;
	if (ast == NULL || ast->kind == CREX_AST_ZVAL) {
		return;
	}

	if (!crex_is_allowed_in_const_expr(ast->kind)) {
		crex_error_noreturn(E_COMPILE_ERROR, "Constant expression contains invalid operations");
	}

	switch (ast->kind) {
		case CREX_AST_CLASS_CONST:
			crex_compile_const_expr_class_const(ast_ptr);
			break;
		case CREX_AST_CLASS_NAME:
			crex_compile_const_expr_class_name(ast_ptr);
			break;
		case CREX_AST_CONST:
			crex_compile_const_expr_const(ast_ptr);
			break;
		case CREX_AST_MAGIC_CONST:
			crex_compile_const_expr_magic_const(ast_ptr);
			break;
		case CREX_AST_NEW:
			if (!ctx->allow_dynamic) {
				crex_error_noreturn(E_COMPILE_ERROR,
					"New expressions are not supported in this context");
			}
			crex_compile_const_expr_new(ast_ptr);
			break;
		case CREX_AST_ARG_LIST:
			crex_compile_const_expr_args(ast_ptr);
			break;
	}

	crex_ast_apply(ast, crex_compile_const_expr, context);
}
/* }}} */

void crex_const_expr_to_zval(zval *result, crex_ast **ast_ptr, bool allow_dynamic) /* {{{ */
{
	const_expr_context context;
	context.allow_dynamic = allow_dynamic;

	crex_eval_const_expr(ast_ptr);
	crex_compile_const_expr(ast_ptr, &context);
	if ((*ast_ptr)->kind != CREX_AST_ZVAL) {
		/* Replace with compiled AST zval representation. */
		zval ast_zv;
		ZVAL_AST(&ast_zv, crex_ast_copy(*ast_ptr));
		crex_ast_destroy(*ast_ptr);
		*ast_ptr = crex_ast_create_zval(&ast_zv);
	}
	ZVAL_COPY(result, crex_ast_get_zval(*ast_ptr));
}
/* }}} */

/* Same as compile_stmt, but with early binding */
void crex_compile_top_stmt(crex_ast *ast) /* {{{ */
{
	if (!ast) {
		return;
	}

	if (ast->kind == CREX_AST_STMT_LIST) {
		crex_ast_list *list = crex_ast_get_list(ast);
		uint32_t i;
		for (i = 0; i < list->children; ++i) {
			crex_compile_top_stmt(list->child[i]);
		}
		return;
	}

	if (ast->kind == CREX_AST_FUNC_DECL) {
		CG(crex_lineno) = ast->lineno;
		crex_compile_func_decl(NULL, ast, 1);
		CG(crex_lineno) = ((crex_ast_decl *) ast)->end_lineno;
	} else if (ast->kind == CREX_AST_CLASS) {
		CG(crex_lineno) = ast->lineno;
		crex_compile_class_decl(NULL, ast, 1);
		CG(crex_lineno) = ((crex_ast_decl *) ast)->end_lineno;
	} else {
		crex_compile_stmt(ast);
	}
	if (ast->kind != CREX_AST_NAMESPACE && ast->kind != CREX_AST_HALT_COMPILER) {
		crex_verify_namespace();
	}
}
/* }}} */

static void crex_compile_stmt(crex_ast *ast) /* {{{ */
{
	if (!ast) {
		return;
	}

	CG(crex_lineno) = ast->lineno;

	if ((CG(compiler_options) & CREX_COMPILE_EXTENDED_STMT) && !crex_is_unticked_stmt(ast)) {
		crex_do_extended_stmt();
	}

	switch (ast->kind) {
		case CREX_AST_STMT_LIST:
			crex_compile_stmt_list(ast);
			break;
		case CREX_AST_GLOBAL:
			crex_compile_global_var(ast);
			break;
		case CREX_AST_STATIC:
			crex_compile_static_var(ast);
			break;
		case CREX_AST_UNSET:
			crex_compile_unset(ast);
			break;
		case CREX_AST_RETURN:
			crex_compile_return(ast);
			break;
		case CREX_AST_ECHO:
			crex_compile_echo(ast);
			break;
		case CREX_AST_BREAK:
		case CREX_AST_CONTINUE:
			crex_compile_break_continue(ast);
			break;
		case CREX_AST_GOTO:
			crex_compile_goto(ast);
			break;
		case CREX_AST_LABEL:
			crex_compile_label(ast);
			break;
		case CREX_AST_WHILE:
			crex_compile_while(ast);
			break;
		case CREX_AST_DO_WHILE:
			crex_compile_do_while(ast);
			break;
		case CREX_AST_FOR:
			crex_compile_for(ast);
			break;
		case CREX_AST_FOREACH:
			crex_compile_foreach(ast);
			break;
		case CREX_AST_IF:
			crex_compile_if(ast);
			break;
		case CREX_AST_SWITCH:
			crex_compile_switch(ast);
			break;
		case CREX_AST_TRY:
			crex_compile_try(ast);
			break;
		case CREX_AST_DECLARE:
			crex_compile_declare(ast);
			break;
		case CREX_AST_FUNC_DECL:
		case CREX_AST_METHOD:
			crex_compile_func_decl(NULL, ast, 0);
			break;
		case CREX_AST_ENUM_CASE:
			crex_compile_enum_case(ast);
			break;
		case CREX_AST_PROP_GROUP:
			crex_compile_prop_group(ast);
			break;
		case CREX_AST_CLASS_CONST_GROUP:
			crex_compile_class_const_group(ast);
			break;
		case CREX_AST_USE_TRAIT:
			crex_compile_use_trait(ast);
			break;
		case CREX_AST_CLASS:
			crex_compile_class_decl(NULL, ast, 0);
			break;
		case CREX_AST_GROUP_USE:
			crex_compile_group_use(ast);
			break;
		case CREX_AST_USE:
			crex_compile_use(ast);
			break;
		case CREX_AST_CONST_DECL:
			crex_compile_const_decl(ast);
			break;
		case CREX_AST_NAMESPACE:
			crex_compile_namespace(ast);
			break;
		case CREX_AST_HALT_COMPILER:
			crex_compile_halt_compiler(ast);
			break;
		case CREX_AST_THROW:
		case CREX_AST_EXIT:
			crex_compile_expr(NULL, ast);
			break;
		default:
		{
			znode result;
			crex_compile_expr(&result, ast);
			crex_do_free(&result);
		}
	}

	if (FC(declarables).ticks && !crex_is_unticked_stmt(ast)) {
		crex_emit_tick();
	}
}
/* }}} */

static void crex_compile_expr_inner(znode *result, crex_ast *ast) /* {{{ */
{
	/* CG(crex_lineno) = ast->lineno; */
	CG(crex_lineno) = crex_ast_get_lineno(ast);

	if (CG(memoize_mode) != CREX_MEMOIZE_NONE) {
		crex_compile_memoized_expr(result, ast);
		return;
	}

	switch (ast->kind) {
		case CREX_AST_ZVAL:
			ZVAL_COPY(&result->u.constant, crex_ast_get_zval(ast));
			result->op_type = IS_CONST;
			return;
		case CREX_AST_ZNODE:
			*result = *crex_ast_get_znode(ast);
			return;
		case CREX_AST_VAR:
		case CREX_AST_DIM:
		case CREX_AST_PROP:
		case CREX_AST_NULLSAFE_PROP:
		case CREX_AST_STATIC_PROP:
		case CREX_AST_CALL:
		case CREX_AST_METHOD_CALL:
		case CREX_AST_NULLSAFE_METHOD_CALL:
		case CREX_AST_STATIC_CALL:
			crex_compile_var(result, ast, BP_VAR_R, 0);
			return;
		case CREX_AST_ASSIGN:
			crex_compile_assign(result, ast);
			return;
		case CREX_AST_ASSIGN_REF:
			crex_compile_assign_ref(result, ast);
			return;
		case CREX_AST_NEW:
			crex_compile_new(result, ast);
			return;
		case CREX_AST_CLONE:
			crex_compile_clone(result, ast);
			return;
		case CREX_AST_ASSIGN_OP:
			crex_compile_compound_assign(result, ast);
			return;
		case CREX_AST_BINARY_OP:
			crex_compile_binary_op(result, ast);
			return;
		case CREX_AST_GREATER:
		case CREX_AST_GREATER_EQUAL:
			crex_compile_greater(result, ast);
			return;
		case CREX_AST_UNARY_OP:
			crex_compile_unary_op(result, ast);
			return;
		case CREX_AST_UNARY_PLUS:
		case CREX_AST_UNARY_MINUS:
			crex_compile_unary_pm(result, ast);
			return;
		case CREX_AST_AND:
		case CREX_AST_OR:
			crex_compile_short_circuiting(result, ast);
			return;
		case CREX_AST_POST_INC:
		case CREX_AST_POST_DEC:
			crex_compile_post_incdec(result, ast);
			return;
		case CREX_AST_PRE_INC:
		case CREX_AST_PRE_DEC:
			crex_compile_pre_incdec(result, ast);
			return;
		case CREX_AST_CAST:
			crex_compile_cast(result, ast);
			return;
		case CREX_AST_CONDITIONAL:
			crex_compile_conditional(result, ast);
			return;
		case CREX_AST_COALESCE:
			crex_compile_coalesce(result, ast);
			return;
		case CREX_AST_ASSIGN_COALESCE:
			crex_compile_assign_coalesce(result, ast);
			return;
		case CREX_AST_PRINT:
			crex_compile_print(result, ast);
			return;
		case CREX_AST_EXIT:
			crex_compile_exit(result, ast);
			return;
		case CREX_AST_YIELD:
			crex_compile_yield(result, ast);
			return;
		case CREX_AST_YIELD_FROM:
			crex_compile_yield_from(result, ast);
			return;
		case CREX_AST_INSTANCEOF:
			crex_compile_instanceof(result, ast);
			return;
		case CREX_AST_INCLUDE_OR_EVAL:
			crex_compile_include_or_eval(result, ast);
			return;
		case CREX_AST_ISSET:
		case CREX_AST_EMPTY:
			crex_compile_isset_or_empty(result, ast);
			return;
		case CREX_AST_SILENCE:
			crex_compile_silence(result, ast);
			return;
		case CREX_AST_SHELL_EXEC:
			crex_compile_shell_exec(result, ast);
			return;
		case CREX_AST_ARRAY:
			crex_compile_array(result, ast);
			return;
		case CREX_AST_CONST:
			crex_compile_const(result, ast);
			return;
		case CREX_AST_CLASS_CONST:
			crex_compile_class_const(result, ast);
			return;
		case CREX_AST_CLASS_NAME:
			crex_compile_class_name(result, ast);
			return;
		case CREX_AST_ENCAPS_LIST:
			crex_compile_encaps_list(result, ast);
			return;
		case CREX_AST_MAGIC_CONST:
			crex_compile_magic_const(result, ast);
			return;
		case CREX_AST_CLOSURE:
		case CREX_AST_ARROW_FUNC:
			crex_compile_func_decl(result, ast, 0);
			return;
		case CREX_AST_THROW:
			crex_compile_throw(result, ast);
			return;
		case CREX_AST_MATCH:
			crex_compile_match(result, ast);
			return;
		default:
			CREX_ASSERT(0 /* not supported */);
	}
}
/* }}} */

static void crex_compile_expr(znode *result, crex_ast *ast)
{
	crex_check_stack_limit();

	uint32_t checkpoint = crex_short_circuiting_checkpoint();
	crex_compile_expr_inner(result, ast);
	crex_short_circuiting_commit(checkpoint, result, ast);
}

static crex_op *crex_compile_var_inner(znode *result, crex_ast *ast, uint32_t type, bool by_ref)
{
	CG(crex_lineno) = crex_ast_get_lineno(ast);

	if (CG(memoize_mode) != CREX_MEMOIZE_NONE) {
		switch (ast->kind) {
			case CREX_AST_CALL:
			case CREX_AST_METHOD_CALL:
			case CREX_AST_NULLSAFE_METHOD_CALL:
			case CREX_AST_STATIC_CALL:
				crex_compile_memoized_expr(result, ast);
				/* This might not actually produce an opcode, e.g. for expressions evaluated at comptime. */
				return NULL;
		}
	}

	switch (ast->kind) {
		case CREX_AST_VAR:
			return crex_compile_simple_var(result, ast, type, 0);
		case CREX_AST_DIM:
			return crex_compile_dim(result, ast, type, by_ref);
		case CREX_AST_PROP:
		case CREX_AST_NULLSAFE_PROP:
			return crex_compile_prop(result, ast, type, by_ref);
		case CREX_AST_STATIC_PROP:
			return crex_compile_static_prop(result, ast, type, by_ref, 0);
		case CREX_AST_CALL:
			crex_compile_call(result, ast, type);
			return NULL;
		case CREX_AST_METHOD_CALL:
		case CREX_AST_NULLSAFE_METHOD_CALL:
			crex_compile_method_call(result, ast, type);
			return NULL;
		case CREX_AST_STATIC_CALL:
			crex_compile_static_call(result, ast, type);
			return NULL;
		case CREX_AST_ZNODE:
			*result = *crex_ast_get_znode(ast);
			return NULL;
		default:
			if (type == BP_VAR_W || type == BP_VAR_RW || type == BP_VAR_UNSET) {
				crex_error_noreturn(E_COMPILE_ERROR,
					"Cannot use temporary expression in write context");
			}

			crex_compile_expr(result, ast);
			return NULL;
	}
}

static crex_op *crex_compile_var(znode *result, crex_ast *ast, uint32_t type, bool by_ref) /* {{{ */
{
	uint32_t checkpoint = crex_short_circuiting_checkpoint();
	crex_op *opcode = crex_compile_var_inner(result, ast, type, by_ref);
	crex_short_circuiting_commit(checkpoint, result, ast);
	return opcode;
}

static crex_op *crex_delayed_compile_var(znode *result, crex_ast *ast, uint32_t type, bool by_ref) /* {{{ */
{
	switch (ast->kind) {
		case CREX_AST_VAR:
			return crex_compile_simple_var(result, ast, type, 1);
		case CREX_AST_DIM:
			return crex_delayed_compile_dim(result, ast, type, by_ref);
		case CREX_AST_PROP:
		case CREX_AST_NULLSAFE_PROP:
		{
			crex_op *opline = crex_delayed_compile_prop(result, ast, type);
			if (by_ref) {
				opline->extended_value |= CREX_FETCH_REF;
			}
			return opline;
		}
		case CREX_AST_STATIC_PROP:
			return crex_compile_static_prop(result, ast, type, by_ref, 1);
		default:
			return crex_compile_var(result, ast, type, 0);
	}
}
/* }}} */

static void crex_eval_const_expr(crex_ast **ast_ptr) /* {{{ */
{
	crex_ast *ast = *ast_ptr;
	zval result;

	if (!ast) {
		return;
	}

	crex_check_stack_limit();

	switch (ast->kind) {
		case CREX_AST_BINARY_OP:
			crex_eval_const_expr(&ast->child[0]);
			crex_eval_const_expr(&ast->child[1]);
			if (ast->child[0]->kind != CREX_AST_ZVAL || ast->child[1]->kind != CREX_AST_ZVAL) {
				return;
			}

			if (!crex_try_ct_eval_binary_op(&result, ast->attr,
					crex_ast_get_zval(ast->child[0]), crex_ast_get_zval(ast->child[1]))
			) {
				return;
			}
			break;
		case CREX_AST_GREATER:
		case CREX_AST_GREATER_EQUAL:
			crex_eval_const_expr(&ast->child[0]);
			crex_eval_const_expr(&ast->child[1]);
			if (ast->child[0]->kind != CREX_AST_ZVAL || ast->child[1]->kind != CREX_AST_ZVAL) {
				return;
			}

			crex_ct_eval_greater(&result, ast->kind,
				crex_ast_get_zval(ast->child[0]), crex_ast_get_zval(ast->child[1]));
			break;
		case CREX_AST_AND:
		case CREX_AST_OR:
		{
			bool child0_is_true, child1_is_true;
			crex_eval_const_expr(&ast->child[0]);
			crex_eval_const_expr(&ast->child[1]);
			if (ast->child[0]->kind != CREX_AST_ZVAL) {
				return;
			}

			child0_is_true = crex_is_true(crex_ast_get_zval(ast->child[0]));
			if (child0_is_true == (ast->kind == CREX_AST_OR)) {
				ZVAL_BOOL(&result, ast->kind == CREX_AST_OR);
				break;
			}

			if (ast->child[1]->kind != CREX_AST_ZVAL) {
				return;
			}

			child1_is_true = crex_is_true(crex_ast_get_zval(ast->child[1]));
			if (ast->kind == CREX_AST_OR) {
				ZVAL_BOOL(&result, child0_is_true || child1_is_true);
			} else {
				ZVAL_BOOL(&result, child0_is_true && child1_is_true);
			}
			break;
		}
		case CREX_AST_UNARY_OP:
			crex_eval_const_expr(&ast->child[0]);
			if (ast->child[0]->kind != CREX_AST_ZVAL) {
				return;
			}

			if (!crex_try_ct_eval_unary_op(&result, ast->attr, crex_ast_get_zval(ast->child[0]))) {
				return;
			}
			break;
		case CREX_AST_UNARY_PLUS:
		case CREX_AST_UNARY_MINUS:
			crex_eval_const_expr(&ast->child[0]);
			if (ast->child[0]->kind != CREX_AST_ZVAL) {
				return;
			}

			if (!crex_try_ct_eval_unary_pm(&result, ast->kind, crex_ast_get_zval(ast->child[0]))) {
				return;
			}
			break;
		case CREX_AST_COALESCE:
			/* Set isset fetch indicator here, opcache disallows runtime altering of the AST */
			if (ast->child[0]->kind == CREX_AST_DIM) {
				ast->child[0]->attr |= CREX_DIM_IS;
			}
			crex_eval_const_expr(&ast->child[0]);

			if (ast->child[0]->kind != CREX_AST_ZVAL) {
				/* ensure everything was compile-time evaluated at least once */
				crex_eval_const_expr(&ast->child[1]);
				return;
			}

			if (C_TYPE_P(crex_ast_get_zval(ast->child[0])) == IS_NULL) {
				crex_eval_const_expr(&ast->child[1]);
				*ast_ptr = ast->child[1];
				ast->child[1] = NULL;
				crex_ast_destroy(ast);
			} else {
				*ast_ptr = ast->child[0];
				ast->child[0] = NULL;
				crex_ast_destroy(ast);
			}
			return;
		case CREX_AST_CONDITIONAL:
		{
			crex_ast **child, *child_ast;
			crex_eval_const_expr(&ast->child[0]);
			if (ast->child[0]->kind != CREX_AST_ZVAL) {
				/* ensure everything was compile-time evaluated at least once */
				if (ast->child[1]) {
					crex_eval_const_expr(&ast->child[1]);
				}
				crex_eval_const_expr(&ast->child[2]);
				return;
			}

			child = &ast->child[2 - crex_is_true(crex_ast_get_zval(ast->child[0]))];
			if (*child == NULL) {
				child--;
			}
			child_ast = *child;
			*child = NULL;
			crex_ast_destroy(ast);
			*ast_ptr = child_ast;
			crex_eval_const_expr(ast_ptr);
			return;
		}
		case CREX_AST_DIM:
		{
			/* constant expression should be always read context ... */
			zval *container, *dim;

			if (ast->child[1] == NULL) {
				crex_error_noreturn(E_COMPILE_ERROR, "Cannot use [] for reading");
			}

			if (ast->attr & CREX_DIM_ALTERNATIVE_SYNTAX) {
				ast->attr &= ~CREX_DIM_ALTERNATIVE_SYNTAX; /* remove flag to avoid duplicate warning */
				crex_error(E_COMPILE_ERROR, "Array and string offset access syntax with curly braces is no longer supported");
			}

			/* Set isset fetch indicator here, opcache disallows runtime altering of the AST */
			if ((ast->attr & CREX_DIM_IS) && ast->child[0]->kind == CREX_AST_DIM) {
				ast->child[0]->attr |= CREX_DIM_IS;
			}

			crex_eval_const_expr(&ast->child[0]);
			crex_eval_const_expr(&ast->child[1]);
			if (ast->child[0]->kind != CREX_AST_ZVAL || ast->child[1]->kind != CREX_AST_ZVAL) {
				return;
			}

			container = crex_ast_get_zval(ast->child[0]);
			dim = crex_ast_get_zval(ast->child[1]);

			if (C_TYPE_P(container) == IS_ARRAY) {
				zval *el;
				if (C_TYPE_P(dim) == IS_LONG) {
					el = crex_hash_index_find(C_ARR_P(container), C_LVAL_P(dim));
					if (el) {
						ZVAL_COPY(&result, el);
					} else {
						return;
					}
				} else if (C_TYPE_P(dim) == IS_STRING) {
					el = crex_symtable_find(C_ARR_P(container), C_STR_P(dim));
					if (el) {
						ZVAL_COPY(&result, el);
					} else {
						return;
					}
				} else {
					return; /* warning... handle at runtime */
				}
			} else if (C_TYPE_P(container) == IS_STRING) {
				crex_long offset;
				uint8_t c;
				if (C_TYPE_P(dim) == IS_LONG) {
					offset = C_LVAL_P(dim);
				} else if (C_TYPE_P(dim) != IS_STRING || is_numeric_string(C_STRVAL_P(dim), C_STRLEN_P(dim), &offset, NULL, 1) != IS_LONG) {
					return;
				}
				if (offset < 0 || (size_t)offset >= C_STRLEN_P(container)) {
					return;
				}
				c = (uint8_t) C_STRVAL_P(container)[offset];
				ZVAL_CHAR(&result, c);
			} else if (C_TYPE_P(container) <= IS_FALSE) {
				return; /* warning... handle at runtime */
			} else {
				return;
			}
			break;
		}
		case CREX_AST_ARRAY:
			if (!crex_try_ct_eval_array(&result, ast)) {
				return;
			}
			break;
		case CREX_AST_MAGIC_CONST:
			if (!crex_try_ct_eval_magic_const(&result, ast)) {
				return;
			}
			break;
		case CREX_AST_CONST:
		{
			crex_ast *name_ast = ast->child[0];
			bool is_fully_qualified;
			crex_string *resolved_name = crex_resolve_const_name(
				crex_ast_get_str(name_ast), name_ast->attr, &is_fully_qualified);

			if (!crex_try_ct_eval_const(&result, resolved_name, is_fully_qualified)) {
				crex_string_release_ex(resolved_name, 0);
				return;
			}

			crex_string_release_ex(resolved_name, 0);
			break;
		}
		case CREX_AST_CLASS_CONST:
		{
			crex_ast *class_ast;
			crex_ast *name_ast;
			crex_string *resolved_name;

			crex_eval_const_expr(&ast->child[0]);
			crex_eval_const_expr(&ast->child[1]);

			if (UNEXPECTED(ast->child[1]->kind != CREX_AST_ZVAL
				|| C_TYPE_P(crex_ast_get_zval(ast->child[1])) != IS_STRING)) {
				return;
			}

			class_ast = ast->child[0];
			name_ast = ast->child[1];

			if (class_ast->kind != CREX_AST_ZVAL || name_ast->kind != CREX_AST_ZVAL) {
				return;
			}

			resolved_name = crex_resolve_class_name_ast(class_ast);
			if (!crex_try_ct_eval_class_const(&result, resolved_name, crex_ast_get_str(name_ast))) {
				crex_string_release_ex(resolved_name, 0);
				return;
			}

			crex_string_release_ex(resolved_name, 0);
			break;
		}
		case CREX_AST_CLASS_NAME:
		{
			crex_ast *class_ast = ast->child[0];
			if (!crex_try_compile_const_expr_resolve_class_name(&result, class_ast)) {
				return;
			}
			break;
		}
		// TODO: We should probably use crex_ast_apply to recursively walk nodes without
		// special handling. It is required that all nodes that are part of a const expr
		// are visited. Probably we should be distinguishing evaluation of const expr and
		// normal exprs here.
		case CREX_AST_ARG_LIST:
		{
			crex_ast_list *list = crex_ast_get_list(ast);
			for (uint32_t i = 0; i < list->children; i++) {
				crex_eval_const_expr(&list->child[i]);
			}
			return;
		}
		case CREX_AST_NEW:
			crex_eval_const_expr(&ast->child[0]);
			crex_eval_const_expr(&ast->child[1]);
			return;
		case CREX_AST_NAMED_ARG:
			crex_eval_const_expr(&ast->child[1]);
			return;
		case CREX_AST_CONST_ENUM_INIT:
			crex_eval_const_expr(&ast->child[2]);
			return;
		case CREX_AST_PROP:
		case CREX_AST_NULLSAFE_PROP:
			crex_eval_const_expr(&ast->child[0]);
			crex_eval_const_expr(&ast->child[1]);
			return;
		default:
			return;
	}

	crex_ast_destroy(ast);
	*ast_ptr = crex_ast_create_zval(&result);
}
/* }}} */
