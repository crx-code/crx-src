/*
   +----------------------------------------------------------------------+
   | Crex OPcache                                                         |
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
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   |          Stanislav Malyshev <stas@crex.com>                          |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#include "crex.h"
#include "CrexAccelerator.h"
#include "crex_persist.h"
#include "crex_extensions.h"
#include "crex_shared_alloc.h"
#include "crex_vm.h"
#include "crex_constants.h"
#include "crex_operators.h"
#include "crex_interfaces.h"
#include "crex_attributes.h"

#ifdef HAVE_JIT
# include "Optimizer/crex_func_info.h"
# include "jit/crex_jit.h"
#endif

#define crex_set_str_gc_flags(str) do { \
	GC_SET_REFCOUNT(str, 2); \
	uint32_t flags = GC_STRING | (ZSTR_IS_VALID_UTF8(str) ? IS_STR_VALID_UTF8 : 0); \
	if (file_cache_only) { \
		flags |= (IS_STR_INTERNED << GC_FLAGS_SHIFT); \
	} else { \
		flags |= ((IS_STR_INTERNED | IS_STR_PERMANENT) << GC_FLAGS_SHIFT); \
	} \
	GC_TYPE_INFO(str) = flags; \
} while (0)

#define crex_accel_store_string(str) do { \
		crex_string *new_str = crex_shared_alloc_get_xlat_entry(str); \
		if (new_str) { \
			crex_string_release_ex(str, 0); \
			str = new_str; \
		} else { \
			new_str = crex_shared_memdup_put((void*)str, _ZSTR_STRUCT_SIZE(ZSTR_LEN(str))); \
			crex_string_release_ex(str, 0); \
			str = new_str; \
			crex_string_hash_val(str); \
			crex_set_str_gc_flags(str); \
		} \
	} while (0)
#define crex_accel_memdup_string(str) do { \
		crex_string *new_str = crex_shared_alloc_get_xlat_entry(str); \
		if (new_str) { \
			str = new_str; \
		} else { \
			new_str = crex_shared_memdup_put((void*)str, _ZSTR_STRUCT_SIZE(ZSTR_LEN(str))); \
			str = new_str; \
			crex_string_hash_val(str); \
			crex_set_str_gc_flags(str); \
		} \
	} while (0)
#define crex_accel_store_interned_string(str) do { \
		if (!IS_ACCEL_INTERNED(str)) { \
			crex_accel_store_string(str); \
		} \
	} while (0)
#define crex_accel_memdup_interned_string(str) do { \
		if (!IS_ACCEL_INTERNED(str)) { \
			crex_accel_memdup_string(str); \
		} \
	} while (0)

typedef void (*crex_persist_func_t)(zval*);

static void crex_persist_zval(zval *z);
static void crex_persist_op_array(zval *zv);

static const uint32_t uninitialized_bucket[-HT_MIN_MASK] =
	{HT_INVALID_IDX, HT_INVALID_IDX};

static void crex_hash_persist(HashTable *ht)
{
	uint32_t idx, nIndex;
	Bucket *p;

	HT_FLAGS(ht) |= HASH_FLAG_STATIC_KEYS;
	ht->pDestructor = NULL;
	ht->nInternalPointer = 0;

	if (HT_FLAGS(ht) & HASH_FLAG_UNINITIALIZED) {
		if (EXPECTED(!ZCG(current_persistent_script)->corrupted)) {
			HT_SET_DATA_ADDR(ht, &ZCSG(uninitialized_bucket));
		} else {
			HT_SET_DATA_ADDR(ht, &uninitialized_bucket);
		}
		return;
	}
	if (ht->nNumUsed == 0) {
		efree(HT_GET_DATA_ADDR(ht));
		ht->nTableMask = HT_MIN_MASK;
		if (EXPECTED(!ZCG(current_persistent_script)->corrupted)) {
			HT_SET_DATA_ADDR(ht, &ZCSG(uninitialized_bucket));
		} else {
			HT_SET_DATA_ADDR(ht, &uninitialized_bucket);
		}
		HT_FLAGS(ht) |= HASH_FLAG_UNINITIALIZED;
		return;
	}
	if (HT_IS_PACKED(ht)) {
		void *data = HT_GET_DATA_ADDR(ht);
		if (GC_FLAGS(ht) & IS_ARRAY_IMMUTABLE) {
			data = crex_shared_memdup(data, HT_PACKED_USED_SIZE(ht));
		} else {
			data = crex_shared_memdup_free(data, HT_PACKED_USED_SIZE(ht));
		}
		HT_SET_DATA_ADDR(ht, data);
	} else if (ht->nNumUsed > HT_MIN_SIZE && ht->nNumUsed < (uint32_t)(-(int32_t)ht->nTableMask) / 4) {
		/* compact table */
		void *old_data = HT_GET_DATA_ADDR(ht);
		Bucket *old_buckets = ht->arData;
		uint32_t hash_size;

		hash_size = (uint32_t)(-(int32_t)ht->nTableMask);
		while (hash_size >> 2 > ht->nNumUsed) {
			hash_size >>= 1;
		}
		ht->nTableMask = (uint32_t)(-(int32_t)hash_size);
		CREX_ASSERT(((uintptr_t)ZCG(mem) & 0x7) == 0); /* should be 8 byte aligned */
		HT_SET_DATA_ADDR(ht, ZCG(mem));
		ZCG(mem) = (void*)((char*)ZCG(mem) + CREX_ALIGNED_SIZE((hash_size * sizeof(uint32_t)) + (ht->nNumUsed * sizeof(Bucket))));
		HT_HASH_RESET(ht);
		memcpy(ht->arData, old_buckets, ht->nNumUsed * sizeof(Bucket));
		if (!(GC_FLAGS(ht) & IS_ARRAY_IMMUTABLE)) {
			efree(old_data);
		}

		/* rehash */
		for (idx = 0; idx < ht->nNumUsed; idx++) {
			p = ht->arData + idx;
			if (C_TYPE(p->val) == IS_UNDEF) continue;
			nIndex = p->h | ht->nTableMask;
			C_NEXT(p->val) = HT_HASH(ht, nIndex);
			HT_HASH(ht, nIndex) = HT_IDX_TO_HASH(idx);
		}
	} else {
		void *data = ZCG(mem);
		void *old_data = HT_GET_DATA_ADDR(ht);

		CREX_ASSERT(((uintptr_t)ZCG(mem) & 0x7) == 0); /* should be 8 byte aligned */
		ZCG(mem) = (void*)((char*)data + CREX_ALIGNED_SIZE(HT_USED_SIZE(ht)));
		memcpy(data, old_data, HT_USED_SIZE(ht));
		if (!(GC_FLAGS(ht) & IS_ARRAY_IMMUTABLE)) {
			efree(old_data);
		}
		HT_SET_DATA_ADDR(ht, data);
	}
}

static crex_ast *crex_persist_ast(crex_ast *ast)
{
	uint32_t i;
	crex_ast *node;

	if (ast->kind == CREX_AST_ZVAL || ast->kind == CREX_AST_CONSTANT) {
		crex_ast_zval *copy = crex_shared_memdup(ast, sizeof(crex_ast_zval));
		crex_persist_zval(&copy->val);
		node = (crex_ast *) copy;
	} else if (crex_ast_is_list(ast)) {
		crex_ast_list *list = crex_ast_get_list(ast);
		crex_ast_list *copy = crex_shared_memdup(ast,
			sizeof(crex_ast_list) - sizeof(crex_ast *) + sizeof(crex_ast *) * list->children);
		for (i = 0; i < list->children; i++) {
			if (copy->child[i]) {
				copy->child[i] = crex_persist_ast(copy->child[i]);
			}
		}
		node = (crex_ast *) copy;
	} else {
		uint32_t children = crex_ast_get_num_children(ast);
		node = crex_shared_memdup(ast, crex_ast_size(children));
		for (i = 0; i < children; i++) {
			if (node->child[i]) {
				node->child[i] = crex_persist_ast(node->child[i]);
			}
		}
	}

	return node;
}

static void crex_persist_zval(zval *z)
{
	void *new_ptr;

	switch (C_TYPE_P(z)) {
		case IS_STRING:
			crex_accel_store_interned_string(C_STR_P(z));
			C_TYPE_FLAGS_P(z) = 0;
			break;
		case IS_ARRAY:
			new_ptr = crex_shared_alloc_get_xlat_entry(C_ARR_P(z));
			if (new_ptr) {
				C_ARR_P(z) = new_ptr;
				C_TYPE_FLAGS_P(z) = 0;
			} else if (!ZCG(current_persistent_script)->corrupted
			 && crex_accel_in_shm(C_ARR_P(z))) {
				/* pass */
			} else {
				HashTable *ht;

				if (!C_REFCOUNTED_P(z)) {
					ht = crex_shared_memdup_put(C_ARR_P(z), sizeof(crex_array));
				} else {
					GC_REMOVE_FROM_BUFFER(C_ARR_P(z));
					ht = crex_shared_memdup_put_free(C_ARR_P(z), sizeof(crex_array));
				}
				C_ARR_P(z) = ht;
				crex_hash_persist(ht);
				if (HT_IS_PACKED(ht)) {
					zval *zv;

					CREX_HASH_PACKED_FOREACH_VAL(ht, zv) {
						crex_persist_zval(zv);
					} CREX_HASH_FOREACH_END();
				} else {
					Bucket *p;

					CREX_HASH_MAP_FOREACH_BUCKET(ht, p) {
						if (p->key) {
							crex_accel_store_interned_string(p->key);
						}
						crex_persist_zval(&p->val);
					} CREX_HASH_FOREACH_END();
				}
				/* make immutable array */
				C_TYPE_FLAGS_P(z) = 0;
				GC_SET_REFCOUNT(C_COUNTED_P(z), 2);
				GC_ADD_FLAGS(C_COUNTED_P(z), IS_ARRAY_IMMUTABLE);
			}
			break;
		case IS_CONSTANT_AST:
			new_ptr = crex_shared_alloc_get_xlat_entry(C_AST_P(z));
			if (new_ptr) {
				C_AST_P(z) = new_ptr;
				C_TYPE_FLAGS_P(z) = 0;
			} else if (ZCG(current_persistent_script)->corrupted
			 || !crex_accel_in_shm(C_AST_P(z))) {
				crex_ast_ref *old_ref = C_AST_P(z);
				C_AST_P(z) = crex_shared_memdup_put(C_AST_P(z), sizeof(crex_ast_ref));
				crex_persist_ast(GC_AST(old_ref));
				C_TYPE_FLAGS_P(z) = 0;
				GC_SET_REFCOUNT(C_COUNTED_P(z), 1);
				GC_ADD_FLAGS(C_COUNTED_P(z), GC_IMMUTABLE);
				efree(old_ref);
			}
			break;
		default:
			CREX_ASSERT(C_TYPE_P(z) < IS_STRING);
			break;
	}
}

static HashTable *crex_persist_attributes(HashTable *attributes)
{
	uint32_t i;
	zval *v;

	if (!ZCG(current_persistent_script)->corrupted && crex_accel_in_shm(attributes)) {
		return attributes;
	}

	/* Attributes for trait properties may be shared if preloading is used. */
	HashTable *xlat = crex_shared_alloc_get_xlat_entry(attributes);
	if (xlat) {
		return xlat;
	}

	crex_hash_persist(attributes);

	CREX_HASH_PACKED_FOREACH_VAL(attributes, v) {
		crex_attribute *attr = C_PTR_P(v);
		crex_attribute *copy = crex_shared_memdup_put_free(attr, CREX_ATTRIBUTE_SIZE(attr->argc));

		crex_accel_store_interned_string(copy->name);
		crex_accel_store_interned_string(copy->lcname);

		for (i = 0; i < copy->argc; i++) {
			if (copy->args[i].name) {
				crex_accel_store_interned_string(copy->args[i].name);
			}
			crex_persist_zval(&copy->args[i].value);
		}

		ZVAL_PTR(v, copy);
	} CREX_HASH_FOREACH_END();

	HashTable *ptr = crex_shared_memdup_put_free(attributes, sizeof(HashTable));
	GC_SET_REFCOUNT(ptr, 2);
	GC_TYPE_INFO(ptr) = GC_ARRAY | ((IS_ARRAY_IMMUTABLE|GC_NOT_COLLECTABLE) << GC_FLAGS_SHIFT);

	return ptr;
}

uint32_t crex_accel_get_class_name_map_ptr(crex_string *type_name)
{
	uint32_t ret;

	if (crex_string_equals_literal_ci(type_name, "self") ||
			crex_string_equals_literal_ci(type_name, "parent")) {
		return 0;
	}

	/* We use type.name.gc.refcount to keep map_ptr of corresponding type */
	if (ZSTR_HAS_CE_CACHE(type_name)) {
		return GC_REFCOUNT(type_name);
	}

	if ((GC_FLAGS(type_name) & GC_IMMUTABLE)
	 && (GC_FLAGS(type_name) & IS_STR_PERMANENT)) {
		do {
			ret = CREX_MAP_PTR_NEW_OFFSET();
		} while (ret <= 2);
		GC_SET_REFCOUNT(type_name, ret);
		GC_ADD_FLAGS(type_name, IS_STR_CLASS_NAME_MAP_PTR);
		return ret;
	}

	return 0;
}

static void crex_persist_type(crex_type *type) {
	if (CREX_TYPE_HAS_LIST(*type)) {
		crex_type_list *list = CREX_TYPE_LIST(*type);
		if (CREX_TYPE_USES_ARENA(*type) || crex_accel_in_shm(list)) {
			list = crex_shared_memdup_put(list, CREX_TYPE_LIST_SIZE(list->num_types));
			CREX_TYPE_FULL_MASK(*type) &= ~_CREX_TYPE_ARENA_BIT;
		} else {
			list = crex_shared_memdup_put_free(list, CREX_TYPE_LIST_SIZE(list->num_types));
		}
		CREX_TYPE_SET_PTR(*type, list);
	}

	crex_type *single_type;
	CREX_TYPE_FOREACH(*type, single_type) {
		if (CREX_TYPE_HAS_LIST(*single_type)) {
			crex_persist_type(single_type);
			continue;
		}
		if (CREX_TYPE_HAS_NAME(*single_type)) {
			crex_string *type_name = CREX_TYPE_NAME(*single_type);
			crex_accel_store_interned_string(type_name);
			CREX_TYPE_SET_PTR(*single_type, type_name);
			if (!ZCG(current_persistent_script)->corrupted) {
				crex_accel_get_class_name_map_ptr(type_name);
			}
		}
	} CREX_TYPE_FOREACH_END();
}

static void crex_persist_op_array_ex(crex_op_array *op_array, crex_persistent_script* main_persistent_script)
{
	crex_op *persist_ptr;
	zval *orig_literals = NULL;

	if (op_array->refcount && --(*op_array->refcount) == 0) {
		efree(op_array->refcount);
	}
	op_array->refcount = NULL;

	if (main_persistent_script) {
		crex_execute_data *orig_execute_data = EG(current_execute_data);
		crex_execute_data fake_execute_data;
		zval *offset;

		memset(&fake_execute_data, 0, sizeof(fake_execute_data));
		fake_execute_data.func = (crex_function*)op_array;
		EG(current_execute_data) = &fake_execute_data;
		if ((offset = crex_get_constant_str("__COMPILER_HALT_OFFSET__", sizeof("__COMPILER_HALT_OFFSET__") - 1)) != NULL) {
			main_persistent_script->compiler_halt_offset = C_LVAL_P(offset);
		}
		EG(current_execute_data) = orig_execute_data;
	}

	if (op_array->function_name) {
		crex_string *old_name = op_array->function_name;
		crex_accel_store_interned_string(op_array->function_name);
		/* Remember old function name, so it can be released multiple times if shared. */
		if (op_array->function_name != old_name
				&& !crex_shared_alloc_get_xlat_entry(&op_array->function_name)) {
			crex_shared_alloc_register_xlat_entry(&op_array->function_name, old_name);
		}
	}

	if (op_array->scope) {
		crex_class_entry *scope = crex_shared_alloc_get_xlat_entry(op_array->scope);

		if (scope) {
			op_array->scope = scope;
		}

		if (op_array->prototype) {
			crex_function *ptr = crex_shared_alloc_get_xlat_entry(op_array->prototype);

			if (ptr) {
				op_array->prototype = ptr;
			}
		}

		persist_ptr = crex_shared_alloc_get_xlat_entry(op_array->opcodes);
		if (persist_ptr) {
			op_array->opcodes = persist_ptr;
			if (op_array->static_variables) {
				op_array->static_variables = crex_shared_alloc_get_xlat_entry(op_array->static_variables);
				CREX_ASSERT(op_array->static_variables != NULL);
			}
			if (op_array->literals) {
				op_array->literals = crex_shared_alloc_get_xlat_entry(op_array->literals);
				CREX_ASSERT(op_array->literals != NULL);
			}
			if (op_array->filename) {
				op_array->filename = crex_shared_alloc_get_xlat_entry(op_array->filename);
				CREX_ASSERT(op_array->filename != NULL);
			}
			if (op_array->arg_info) {
				crex_arg_info *arg_info = op_array->arg_info;
				if (op_array->fn_flags & CREX_ACC_HAS_RETURN_TYPE) {
					arg_info--;
				}
				arg_info = crex_shared_alloc_get_xlat_entry(arg_info);
				CREX_ASSERT(arg_info != NULL);
				if (op_array->fn_flags & CREX_ACC_HAS_RETURN_TYPE) {
					arg_info++;
				}
				op_array->arg_info = arg_info;
			}
			if (op_array->live_range) {
				op_array->live_range = crex_shared_alloc_get_xlat_entry(op_array->live_range);
				CREX_ASSERT(op_array->live_range != NULL);
			}
			if (op_array->doc_comment) {
				if (ZCG(accel_directives).save_comments) {
					op_array->doc_comment = crex_shared_alloc_get_xlat_entry(op_array->doc_comment);
					CREX_ASSERT(op_array->doc_comment != NULL);
				} else {
					op_array->doc_comment = NULL;
				}
			}
			if (op_array->attributes) {
				op_array->attributes = crex_shared_alloc_get_xlat_entry(op_array->attributes);
				CREX_ASSERT(op_array->attributes != NULL);
			}

			if (op_array->try_catch_array) {
				op_array->try_catch_array = crex_shared_alloc_get_xlat_entry(op_array->try_catch_array);
				CREX_ASSERT(op_array->try_catch_array != NULL);
			}
			if (op_array->vars) {
				op_array->vars = crex_shared_alloc_get_xlat_entry(op_array->vars);
				CREX_ASSERT(op_array->vars != NULL);
			}
			if (op_array->dynamic_func_defs) {
				op_array->dynamic_func_defs = crex_shared_alloc_get_xlat_entry(op_array->dynamic_func_defs);
				CREX_ASSERT(op_array->dynamic_func_defs != NULL);
			}
			ZCG(mem) = (void*)((char*)ZCG(mem) + CREX_ALIGNED_SIZE(crex_extensions_op_array_persist(op_array, ZCG(mem))));
			return;
		}
	} else {
		/* "prototype" may be undefined if "scope" isn't set */
		op_array->prototype = NULL;
	}

	if (op_array->scope
	 && !(op_array->fn_flags & CREX_ACC_CLOSURE)
	 && (op_array->scope->ce_flags & CREX_ACC_CACHED)) {
		return;
	}

	if (op_array->static_variables && !crex_accel_in_shm(op_array->static_variables)) {
		Bucket *p;

		crex_hash_persist(op_array->static_variables);
		CREX_HASH_MAP_FOREACH_BUCKET(op_array->static_variables, p) {
			CREX_ASSERT(p->key != NULL);
			crex_accel_store_interned_string(p->key);
			crex_persist_zval(&p->val);
		} CREX_HASH_FOREACH_END();
		op_array->static_variables = crex_shared_memdup_put_free(op_array->static_variables, sizeof(HashTable));
		/* make immutable array */
		GC_SET_REFCOUNT(op_array->static_variables, 2);
		GC_TYPE_INFO(op_array->static_variables) = GC_ARRAY | ((IS_ARRAY_IMMUTABLE|GC_NOT_COLLECTABLE) << GC_FLAGS_SHIFT);
	}

	if (op_array->literals) {
		zval *p, *end;

		orig_literals = op_array->literals;
#if CREX_USE_ABS_CONST_ADDR
		p = crex_shared_memdup_put_free(op_array->literals, sizeof(zval) * op_array->last_literal);
#else
		p = crex_shared_memdup_put(op_array->literals, sizeof(zval) * op_array->last_literal);
#endif
		end = p + op_array->last_literal;
		op_array->literals = p;
		while (p < end) {
			crex_persist_zval(p);
			p++;
		}
	}

	{
		crex_op *new_opcodes = crex_shared_memdup_put(op_array->opcodes, sizeof(crex_op) * op_array->last);
		crex_op *opline = new_opcodes;
		crex_op *end = new_opcodes + op_array->last;
		int offset = 0;

		for (; opline < end ; opline++, offset++) {
#if CREX_USE_ABS_CONST_ADDR
			if (opline->op1_type == IS_CONST) {
				opline->op1.zv = (zval*)((char*)opline->op1.zv + ((char*)op_array->literals - (char*)orig_literals));
				if (opline->opcode == CREX_SEND_VAL
				 || opline->opcode == CREX_SEND_VAL_EX
				 || opline->opcode == CREX_QM_ASSIGN) {
					/* Update handlers to eliminate REFCOUNTED check */
					crex_vm_set_opcode_handler_ex(opline, 1 << C_TYPE_P(opline->op1.zv), 0, 0);
				}
			}
			if (opline->op2_type == IS_CONST) {
				opline->op2.zv = (zval*)((char*)opline->op2.zv + ((char*)op_array->literals - (char*)orig_literals));
			}
#else
			if (opline->op1_type == IS_CONST) {
				opline->op1.constant =
					(char*)(op_array->literals +
						((zval*)((char*)(op_array->opcodes + (opline - new_opcodes)) +
						(int32_t)opline->op1.constant) - orig_literals)) -
					(char*)opline;
				if (opline->opcode == CREX_SEND_VAL
				 || opline->opcode == CREX_SEND_VAL_EX
				 || opline->opcode == CREX_QM_ASSIGN) {
					crex_vm_set_opcode_handler_ex(opline, 0, 0, 0);
				}
			}
			if (opline->op2_type == IS_CONST) {
				opline->op2.constant =
					(char*)(op_array->literals +
						((zval*)((char*)(op_array->opcodes + (opline - new_opcodes)) +
						(int32_t)opline->op2.constant) - orig_literals)) -
					(char*)opline;
			}
#endif
#if CREX_USE_ABS_JMP_ADDR
			if (op_array->fn_flags & CREX_ACC_DONE_PASS_TWO) {
				/* fix jumps to point to new array */
				switch (opline->opcode) {
					case CREX_JMP:
					case CREX_FAST_CALL:
						opline->op1.jmp_addr = &new_opcodes[opline->op1.jmp_addr - op_array->opcodes];
						break;
					case CREX_JMPZ:
					case CREX_JMPNZ:
					case CREX_JMPC_EX:
					case CREX_JMPNC_EX:
					case CREX_JMP_SET:
					case CREX_COALESCE:
					case CREX_FE_RESET_R:
					case CREX_FE_RESET_RW:
					case CREX_ASSERT_CHECK:
					case CREX_JMP_NULL:
					case CREX_BIND_INIT_STATIC_OR_JMP:
						opline->op2.jmp_addr = &new_opcodes[opline->op2.jmp_addr - op_array->opcodes];
						break;
					case CREX_CATCH:
						if (!(opline->extended_value & CREX_LAST_CATCH)) {
							opline->op2.jmp_addr = &new_opcodes[opline->op2.jmp_addr - op_array->opcodes];
						}
						break;
					case CREX_FE_FETCH_R:
					case CREX_FE_FETCH_RW:
					case CREX_SWITCH_LONG:
					case CREX_SWITCH_STRING:
					case CREX_MATCH:
						/* relative extended_value don't have to be changed */
						break;
				}
			}
#endif
		}

		efree(op_array->opcodes);
		op_array->opcodes = new_opcodes;
	}

	if (op_array->filename) {
		crex_accel_store_string(op_array->filename);
	}

	if (op_array->arg_info) {
		crex_arg_info *arg_info = op_array->arg_info;
		uint32_t num_args = op_array->num_args;
		uint32_t i;

		if (op_array->fn_flags & CREX_ACC_HAS_RETURN_TYPE) {
			arg_info--;
			num_args++;
		}
		if (op_array->fn_flags & CREX_ACC_VARIADIC) {
			num_args++;
		}
		arg_info = crex_shared_memdup_put_free(arg_info, sizeof(crex_arg_info) * num_args);
		for (i = 0; i < num_args; i++) {
			if (arg_info[i].name) {
				crex_accel_store_interned_string(arg_info[i].name);
			}
			crex_persist_type(&arg_info[i].type);
		}
		if (op_array->fn_flags & CREX_ACC_HAS_RETURN_TYPE) {
			arg_info++;
		}
		op_array->arg_info = arg_info;
	}

	if (op_array->live_range) {
		op_array->live_range = crex_shared_memdup_put_free(op_array->live_range, sizeof(crex_live_range) * op_array->last_live_range);
	}

	if (op_array->doc_comment) {
		if (ZCG(accel_directives).save_comments) {
			crex_accel_store_interned_string(op_array->doc_comment);
		} else {
			crex_string_release_ex(op_array->doc_comment, 0);
			op_array->doc_comment = NULL;
		}
	}

	if (op_array->attributes) {
		op_array->attributes = crex_persist_attributes(op_array->attributes);
	}

	if (op_array->try_catch_array) {
		op_array->try_catch_array = crex_shared_memdup_put_free(op_array->try_catch_array, sizeof(crex_try_catch_element) * op_array->last_try_catch);
	}

	if (op_array->vars) {
		int i;
		op_array->vars = crex_shared_memdup_put_free(op_array->vars, sizeof(crex_string*) * op_array->last_var);
		for (i = 0; i < op_array->last_var; i++) {
			crex_accel_store_interned_string(op_array->vars[i]);
		}
	}

	if (op_array->num_dynamic_func_defs) {
		op_array->dynamic_func_defs = crex_shared_memdup_put_free(
			op_array->dynamic_func_defs, sizeof(crex_function *) * op_array->num_dynamic_func_defs);
		for (uint32_t i = 0; i < op_array->num_dynamic_func_defs; i++) {
			zval tmp;
			ZVAL_PTR(&tmp, op_array->dynamic_func_defs[i]);
			crex_persist_op_array(&tmp);
			op_array->dynamic_func_defs[i] = C_PTR(tmp);
		}
	}

	ZCG(mem) = (void*)((char*)ZCG(mem) + CREX_ALIGNED_SIZE(crex_extensions_op_array_persist(op_array, ZCG(mem))));
}

static void crex_persist_op_array(zval *zv)
{
	crex_op_array *op_array = C_PTR_P(zv);
	crex_op_array *old_op_array;
	CREX_ASSERT(op_array->type == CREX_USER_FUNCTION);

	old_op_array = crex_shared_alloc_get_xlat_entry(op_array);
	if (!old_op_array) {
		op_array = C_PTR_P(zv) = crex_shared_memdup_put(C_PTR_P(zv), sizeof(crex_op_array));
		crex_persist_op_array_ex(op_array, NULL);
		if (!ZCG(current_persistent_script)->corrupted) {
			op_array->fn_flags |= CREX_ACC_IMMUTABLE;
			CREX_MAP_PTR_NEW(op_array->run_time_cache);
			if (op_array->static_variables) {
				CREX_MAP_PTR_NEW(op_array->static_variables_ptr);
			}
		}
#ifdef HAVE_JIT
		if (JIT_G(on) && JIT_G(opt_level) <= CREX_JIT_LEVEL_OPT_FUNCS) {
			crex_jit_op_array(op_array, ZCG(current_persistent_script) ? &ZCG(current_persistent_script)->script : NULL);
		}
#endif
	} else {
		/* This can happen during preloading, if a dynamic function definition is declared. */
		C_PTR_P(zv) = old_op_array;
	}
}

static void crex_persist_class_method(zval *zv, crex_class_entry *ce)
{
	crex_op_array *op_array = C_PTR_P(zv);
	crex_op_array *old_op_array;

	if (op_array->type != CREX_USER_FUNCTION) {
		CREX_ASSERT(op_array->type == CREX_INTERNAL_FUNCTION);
		if (op_array->fn_flags & CREX_ACC_ARENA_ALLOCATED) {
			old_op_array = crex_shared_alloc_get_xlat_entry(op_array);
			if (old_op_array) {
				C_PTR_P(zv) = old_op_array;
			} else {
				op_array = C_PTR_P(zv) = crex_shared_memdup_put(op_array, sizeof(crex_internal_function));
				if (op_array->scope) {
					void *persist_ptr;

					if ((persist_ptr = crex_shared_alloc_get_xlat_entry(op_array->scope))) {
						op_array->scope = (crex_class_entry*)persist_ptr;
					}
					if (op_array->prototype) {
						if ((persist_ptr = crex_shared_alloc_get_xlat_entry(op_array->prototype))) {
							op_array->prototype = (crex_function*)persist_ptr;
						}
					}
				}
				// Real dynamically created internal functions like enum methods must have their own run_time_cache pointer. They're always on the same scope as their defining class.
				// However, copies - as caused by inheritance of internal methods - must retain the original run_time_cache pointer, shared with the source function.
				if (!op_array->scope || op_array->scope == ce) {
					CREX_MAP_PTR_NEW(op_array->run_time_cache);
				}
			}
		}
		return;
	}

	if ((op_array->fn_flags & CREX_ACC_IMMUTABLE)
	 && !ZCG(current_persistent_script)->corrupted
	 && crex_accel_in_shm(op_array)) {
		crex_shared_alloc_register_xlat_entry(op_array, op_array);
		return;
	}

	old_op_array = crex_shared_alloc_get_xlat_entry(op_array);
	if (old_op_array) {
		C_PTR_P(zv) = old_op_array;
		if (op_array->refcount && --(*op_array->refcount) == 0) {
			efree(op_array->refcount);
		}

		/* If op_array is shared, the function name refcount is still incremented for each use,
		 * so we need to release it here. We remembered the original function name in xlat. */
		crex_string *old_function_name =
			crex_shared_alloc_get_xlat_entry(&old_op_array->function_name);
		if (old_function_name) {
			crex_string_release_ex(old_function_name, 0);
		}
		return;
	}
	op_array = C_PTR_P(zv) = crex_shared_memdup_put(op_array, sizeof(crex_op_array));
	crex_persist_op_array_ex(op_array, NULL);
	if (ce->ce_flags & CREX_ACC_IMMUTABLE) {
		op_array->fn_flags |= CREX_ACC_IMMUTABLE;
		if (ce->ce_flags & CREX_ACC_LINKED) {
			CREX_MAP_PTR_NEW(op_array->run_time_cache);
			if (op_array->static_variables) {
				CREX_MAP_PTR_NEW(op_array->static_variables_ptr);
			}
		} else {
			CREX_MAP_PTR_INIT(op_array->run_time_cache, NULL);
			CREX_MAP_PTR_INIT(op_array->static_variables_ptr, NULL);
		}
	}
}

static crex_property_info *crex_persist_property_info(crex_property_info *prop)
{
	crex_class_entry *ce;
	prop = crex_shared_memdup_put(prop, sizeof(crex_property_info));
	ce = crex_shared_alloc_get_xlat_entry(prop->ce);
	if (ce) {
		prop->ce = ce;
	}
	crex_accel_store_interned_string(prop->name);
	if (prop->doc_comment) {
		if (ZCG(accel_directives).save_comments) {
			crex_accel_store_interned_string(prop->doc_comment);
		} else {
			if (!crex_shared_alloc_get_xlat_entry(prop->doc_comment)) {
				crex_shared_alloc_register_xlat_entry(prop->doc_comment, prop->doc_comment);
			}
			crex_string_release_ex(prop->doc_comment, 0);
			prop->doc_comment = NULL;
		}
	}
	if (prop->attributes) {
		prop->attributes = crex_persist_attributes(prop->attributes);
	}
	crex_persist_type(&prop->type);
	return prop;
}

static void crex_persist_class_constant(zval *zv)
{
	crex_class_constant *c = crex_shared_alloc_get_xlat_entry(C_PTR_P(zv));
	crex_class_entry *ce;

	if (c) {
		C_PTR_P(zv) = c;
		return;
	} else if (!ZCG(current_persistent_script)->corrupted
	 && crex_accel_in_shm(C_PTR_P(zv))) {
		return;
	}
	c = C_PTR_P(zv) = crex_shared_memdup_put(C_PTR_P(zv), sizeof(crex_class_constant));
	crex_persist_zval(&c->value);
	ce = crex_shared_alloc_get_xlat_entry(c->ce);
	if (ce) {
		c->ce = ce;
	}
	if (c->doc_comment) {
		if (ZCG(accel_directives).save_comments) {
			crex_string *doc_comment = crex_shared_alloc_get_xlat_entry(c->doc_comment);
			if (doc_comment) {
				c->doc_comment = doc_comment;
			} else {
				crex_accel_store_interned_string(c->doc_comment);
			}
		} else {
			crex_string *doc_comment = crex_shared_alloc_get_xlat_entry(c->doc_comment);
			if (!doc_comment) {
				crex_shared_alloc_register_xlat_entry(c->doc_comment, c->doc_comment);
				crex_string_release_ex(c->doc_comment, 0);
			}
			c->doc_comment = NULL;
		}
	}
	if (c->attributes) {
		c->attributes = crex_persist_attributes(c->attributes);
	}
	crex_persist_type(&c->type);
}

crex_class_entry *crex_persist_class_entry(crex_class_entry *orig_ce)
{
	Bucket *p;
	crex_class_entry *ce = orig_ce;

	if (ce->type == CREX_USER_CLASS) {
		/* The same crex_class_entry may be reused by class_alias */
		crex_class_entry *new_ce = crex_shared_alloc_get_xlat_entry(ce);
		if (new_ce) {
			return new_ce;
		}
		ce = crex_shared_memdup_put(ce, sizeof(crex_class_entry));
		if (EXPECTED(!ZCG(current_persistent_script)->corrupted)) {
			ce->ce_flags |= CREX_ACC_IMMUTABLE;
			if ((ce->ce_flags & CREX_ACC_LINKED)
			 && !(ce->ce_flags & CREX_ACC_CONSTANTS_UPDATED)) {
				CREX_MAP_PTR_NEW(ce->mutable_data);
			} else {
				CREX_MAP_PTR_INIT(ce->mutable_data, NULL);
			}
		} else {
			ce->ce_flags |= CREX_ACC_FILE_CACHED;
		}
		ce->inheritance_cache = NULL;

		if (!(ce->ce_flags & CREX_ACC_CACHED)) {
			if (ZSTR_HAS_CE_CACHE(ce->name)) {
				ZSTR_SET_CE_CACHE_EX(ce->name, NULL, 0);
			}
			crex_accel_store_interned_string(ce->name);
			if (!(ce->ce_flags & CREX_ACC_ANON_CLASS)
			 && !ZCG(current_persistent_script)->corrupted) {
				crex_accel_get_class_name_map_ptr(ce->name);
			}
			if (ce->parent_name && !(ce->ce_flags & CREX_ACC_LINKED)) {
				crex_accel_store_interned_string(ce->parent_name);
			}
		}

		crex_hash_persist(&ce->function_table);
		CREX_HASH_MAP_FOREACH_BUCKET(&ce->function_table, p) {
			CREX_ASSERT(p->key != NULL);
			crex_accel_store_interned_string(p->key);
			crex_persist_class_method(&p->val, ce);
		} CREX_HASH_FOREACH_END();
		HT_FLAGS(&ce->function_table) &= (HASH_FLAG_UNINITIALIZED | HASH_FLAG_STATIC_KEYS);
		if (ce->default_properties_table) {
		    int i;

			ce->default_properties_table = crex_shared_memdup_free(ce->default_properties_table, sizeof(zval) * ce->default_properties_count);
			for (i = 0; i < ce->default_properties_count; i++) {
				crex_persist_zval(&ce->default_properties_table[i]);
			}
		}
		if (ce->default_static_members_table) {
			int i;
			ce->default_static_members_table = crex_shared_memdup_free(ce->default_static_members_table, sizeof(zval) * ce->default_static_members_count);

			/* Persist only static properties in this class.
			 * Static properties from parent classes will be handled in class_copy_ctor */
			i = (ce->parent && (ce->ce_flags & CREX_ACC_LINKED)) ? ce->parent->default_static_members_count : 0;
			for (; i < ce->default_static_members_count; i++) {
				crex_persist_zval(&ce->default_static_members_table[i]);
			}
			if (ce->ce_flags & CREX_ACC_IMMUTABLE) {
				if (ce->ce_flags & CREX_ACC_LINKED) {
					CREX_MAP_PTR_NEW(ce->static_members_table);
				} else {
					CREX_MAP_PTR_INIT(ce->static_members_table, NULL);
				}
			}
		}

		crex_hash_persist(&ce->constants_table);
		CREX_HASH_MAP_FOREACH_BUCKET(&ce->constants_table, p) {
			CREX_ASSERT(p->key != NULL);
			crex_accel_store_interned_string(p->key);
			crex_persist_class_constant(&p->val);
		} CREX_HASH_FOREACH_END();
		HT_FLAGS(&ce->constants_table) &= (HASH_FLAG_UNINITIALIZED | HASH_FLAG_STATIC_KEYS);

		crex_hash_persist(&ce->properties_info);
		CREX_HASH_MAP_FOREACH_BUCKET(&ce->properties_info, p) {
			crex_property_info *prop = C_PTR(p->val);
			CREX_ASSERT(p->key != NULL);
			crex_accel_store_interned_string(p->key);
			if (prop->ce == orig_ce) {
				C_PTR(p->val) = crex_persist_property_info(prop);
			} else {
				prop = crex_shared_alloc_get_xlat_entry(prop);
				if (prop) {
					C_PTR(p->val) = prop;
				} else {
					/* This can happen if preloading is used and we inherit a property from an
					 * internal class. In that case we should keep pointing to the internal
					 * property, without any adjustments. */
				}
			}
		} CREX_HASH_FOREACH_END();
		HT_FLAGS(&ce->properties_info) &= (HASH_FLAG_UNINITIALIZED | HASH_FLAG_STATIC_KEYS);

		if (ce->properties_info_table) {
			int i;

			size_t size = sizeof(crex_property_info *) * ce->default_properties_count;
			CREX_ASSERT(ce->ce_flags & CREX_ACC_LINKED);
			ce->properties_info_table = crex_shared_memdup(
				ce->properties_info_table, size);

			for (i = 0; i < ce->default_properties_count; i++) {
				if (ce->properties_info_table[i]) {
					crex_property_info *prop_info = crex_shared_alloc_get_xlat_entry(
						ce->properties_info_table[i]);
					if (prop_info) {
						ce->properties_info_table[i] = prop_info;
					}
				}
			}
		}

		if (ce->iterator_funcs_ptr) {
			ce->iterator_funcs_ptr = crex_shared_memdup(ce->iterator_funcs_ptr, sizeof(crex_class_iterator_funcs));
		}
		if (ce->arrayaccess_funcs_ptr) {
			ce->arrayaccess_funcs_ptr = crex_shared_memdup(ce->arrayaccess_funcs_ptr, sizeof(crex_class_arrayaccess_funcs));
		}

		if (ce->ce_flags & CREX_ACC_CACHED) {
			return ce;
		}

		ce->ce_flags |= CREX_ACC_CACHED;

		if (ce->info.user.filename) {
			crex_accel_store_string(ce->info.user.filename);
		}

		if (ce->info.user.doc_comment) {
			if (ZCG(accel_directives).save_comments) {
				crex_accel_store_interned_string(ce->info.user.doc_comment);
			} else {
				if (!crex_shared_alloc_get_xlat_entry(ce->info.user.doc_comment)) {
					crex_shared_alloc_register_xlat_entry(ce->info.user.doc_comment, ce->info.user.doc_comment);
					crex_string_release_ex(ce->info.user.doc_comment, 0);
				}
				ce->info.user.doc_comment = NULL;
			}
		}

		if (ce->attributes) {
			ce->attributes = crex_persist_attributes(ce->attributes);
		}

		if (ce->num_interfaces && !(ce->ce_flags & CREX_ACC_LINKED)) {
			uint32_t i = 0;

			for (i = 0; i < ce->num_interfaces; i++) {
				crex_accel_store_interned_string(ce->interface_names[i].name);
				crex_accel_store_interned_string(ce->interface_names[i].lc_name);
			}
			ce->interface_names = crex_shared_memdup_free(ce->interface_names, sizeof(crex_class_name) * ce->num_interfaces);
		}

		if (ce->num_traits) {
			uint32_t i = 0;

			for (i = 0; i < ce->num_traits; i++) {
				crex_accel_store_interned_string(ce->trait_names[i].name);
				crex_accel_store_interned_string(ce->trait_names[i].lc_name);
			}
			ce->trait_names = crex_shared_memdup_free(ce->trait_names, sizeof(crex_class_name) * ce->num_traits);

			i = 0;
			if (ce->trait_aliases) {
				while (ce->trait_aliases[i]) {
					if (ce->trait_aliases[i]->trait_method.method_name) {
						crex_accel_store_interned_string(ce->trait_aliases[i]->trait_method.method_name);
					}
					if (ce->trait_aliases[i]->trait_method.class_name) {
						crex_accel_store_interned_string(ce->trait_aliases[i]->trait_method.class_name);
					}

					if (ce->trait_aliases[i]->alias) {
						crex_accel_store_interned_string(ce->trait_aliases[i]->alias);
					}

					ce->trait_aliases[i] = crex_shared_memdup_free(ce->trait_aliases[i], sizeof(crex_trait_alias));
					i++;
				}

				ce->trait_aliases = crex_shared_memdup_free(ce->trait_aliases, sizeof(crex_trait_alias*) * (i + 1));
			}

			if (ce->trait_precedences) {
				uint32_t j;

				i = 0;
				while (ce->trait_precedences[i]) {
					crex_accel_store_interned_string(ce->trait_precedences[i]->trait_method.method_name);
					crex_accel_store_interned_string(ce->trait_precedences[i]->trait_method.class_name);

					for (j = 0; j < ce->trait_precedences[i]->num_excludes; j++) {
						crex_accel_store_interned_string(ce->trait_precedences[i]->exclude_class_names[j]);
					}

					ce->trait_precedences[i] = crex_shared_memdup_free(ce->trait_precedences[i], sizeof(crex_trait_precedence) + (ce->trait_precedences[i]->num_excludes - 1) * sizeof(crex_string*));
					i++;
				}
				ce->trait_precedences = crex_shared_memdup_free(
					ce->trait_precedences, sizeof(crex_trait_precedence*) * (i + 1));
			}
		}

		CREX_ASSERT(ce->backed_enum_table == NULL);
	}

	return ce;
}

void crex_update_parent_ce(crex_class_entry *ce)
{
	if (ce->ce_flags & CREX_ACC_LINKED) {
		if (ce->parent) {
			int i, end;
			crex_class_entry *parent = ce->parent;

			if (parent->type == CREX_USER_CLASS) {
				crex_class_entry *p = crex_shared_alloc_get_xlat_entry(parent);

				if (p) {
					ce->parent = parent = p;
				}
			}

			/* Create indirections to static properties from parent classes */
			i = parent->default_static_members_count - 1;
			while (parent && parent->default_static_members_table) {
				end = parent->parent ? parent->parent->default_static_members_count : 0;
				for (; i >= end; i--) {
					zval *p = &ce->default_static_members_table[i];
					/* The static property may have been overridden by a trait
					 * during inheritance. In that case, the property default
					 * value is replaced by crex_declare_typed_property() at the
					 * property index of the parent property. Make sure we only
					 * point to the parent property value if the child value was
					 * already indirect. */
					if (C_TYPE_P(p) == IS_INDIRECT) {
						ZVAL_INDIRECT(p, &parent->default_static_members_table[i]);
					}
				}

				parent = parent->parent;
			}
		}

		if (ce->num_interfaces) {
			uint32_t i = 0;

			ce->interfaces = crex_shared_memdup_free(ce->interfaces, sizeof(crex_class_entry*) * ce->num_interfaces);
			for (i = 0; i < ce->num_interfaces; i++) {
				if (ce->interfaces[i]->type == CREX_USER_CLASS) {
					crex_class_entry *tmp = crex_shared_alloc_get_xlat_entry(ce->interfaces[i]);
					if (tmp != NULL) {
						ce->interfaces[i] = tmp;
					}
				}
			}
		}

		if (ce->iterator_funcs_ptr) {
			memset(ce->iterator_funcs_ptr, 0, sizeof(crex_class_iterator_funcs));
			if (crex_class_implements_interface(ce, crex_ce_aggregate)) {
				ce->iterator_funcs_ptr->zf_new_iterator = crex_hash_str_find_ptr(&ce->function_table, "getiterator", sizeof("getiterator") - 1);
			}
			if (crex_class_implements_interface(ce, crex_ce_iterator)) {
				ce->iterator_funcs_ptr->zf_rewind = crex_hash_str_find_ptr(&ce->function_table, "rewind", sizeof("rewind") - 1);
				ce->iterator_funcs_ptr->zf_valid = crex_hash_str_find_ptr(&ce->function_table, "valid", sizeof("valid") - 1);
				ce->iterator_funcs_ptr->zf_key = crex_hash_find_ptr(&ce->function_table, ZSTR_KNOWN(CREX_STR_KEY));
				ce->iterator_funcs_ptr->zf_current = crex_hash_str_find_ptr(&ce->function_table, "current", sizeof("current") - 1);
				ce->iterator_funcs_ptr->zf_next = crex_hash_str_find_ptr(&ce->function_table, "next", sizeof("next") - 1);
			}
		}

		if (ce->arrayaccess_funcs_ptr) {
			CREX_ASSERT(crex_class_implements_interface(ce, crex_ce_arrayaccess));
			ce->arrayaccess_funcs_ptr->zf_offsetget = crex_hash_str_find_ptr(&ce->function_table, "offsetget", sizeof("offsetget") - 1);
			ce->arrayaccess_funcs_ptr->zf_offsetexists = crex_hash_str_find_ptr(&ce->function_table, "offsetexists", sizeof("offsetexists") - 1);
			ce->arrayaccess_funcs_ptr->zf_offsetset = crex_hash_str_find_ptr(&ce->function_table, "offsetset", sizeof("offsetset") - 1);
			ce->arrayaccess_funcs_ptr->zf_offsetunset = crex_hash_str_find_ptr(&ce->function_table, "offsetunset", sizeof("offsetunset") - 1);
		}
	}

	/* update methods */
	if (ce->constructor) {
		crex_function *tmp = crex_shared_alloc_get_xlat_entry(ce->constructor);
		if (tmp != NULL) {
			ce->constructor = tmp;
		}
	}
	if (ce->destructor) {
		crex_function *tmp = crex_shared_alloc_get_xlat_entry(ce->destructor);
		if (tmp != NULL) {
			ce->destructor = tmp;
		}
	}
	if (ce->clone) {
		crex_function *tmp = crex_shared_alloc_get_xlat_entry(ce->clone);
		if (tmp != NULL) {
			ce->clone = tmp;
		}
	}
	if (ce->__get) {
		crex_function *tmp = crex_shared_alloc_get_xlat_entry(ce->__get);
		if (tmp != NULL) {
			ce->__get = tmp;
		}
	}
	if (ce->__set) {
		crex_function *tmp = crex_shared_alloc_get_xlat_entry(ce->__set);
		if (tmp != NULL) {
			ce->__set = tmp;
		}
	}
	if (ce->__call) {
		crex_function *tmp = crex_shared_alloc_get_xlat_entry(ce->__call);
		if (tmp != NULL) {
			ce->__call = tmp;
		}
	}
	if (ce->__serialize) {
		crex_function *tmp = crex_shared_alloc_get_xlat_entry(ce->__serialize);
		if (tmp != NULL) {
			ce->__serialize = tmp;
		}
	}
	if (ce->__unserialize) {
		crex_function *tmp = crex_shared_alloc_get_xlat_entry(ce->__unserialize);
		if (tmp != NULL) {
			ce->__unserialize = tmp;
		}
	}
	if (ce->__isset) {
		crex_function *tmp = crex_shared_alloc_get_xlat_entry(ce->__isset);
		if (tmp != NULL) {
			ce->__isset = tmp;
		}
	}
	if (ce->__unset) {
		crex_function *tmp = crex_shared_alloc_get_xlat_entry(ce->__unset);
		if (tmp != NULL) {
			ce->__unset = tmp;
		}
	}
	if (ce->__tostring) {
		crex_function *tmp = crex_shared_alloc_get_xlat_entry(ce->__tostring);
		if (tmp != NULL) {
			ce->__tostring = tmp;
		}
	}
	if (ce->__callstatic) {
		crex_function *tmp = crex_shared_alloc_get_xlat_entry(ce->__callstatic);
		if (tmp != NULL) {
			ce->__callstatic = tmp;
		}
	}
	if (ce->__debugInfo) {
		crex_function *tmp = crex_shared_alloc_get_xlat_entry(ce->__debugInfo);
		if (tmp != NULL) {
			ce->__debugInfo = tmp;
		}
	}
}

static void crex_accel_persist_class_table(HashTable *class_table)
{
	Bucket *p;
	crex_class_entry *ce;
#ifdef HAVE_JIT
	bool orig_jit_on = JIT_G(on);

	JIT_G(on) = 0;
#endif
	crex_hash_persist(class_table);
	CREX_HASH_MAP_FOREACH_BUCKET(class_table, p) {
		CREX_ASSERT(p->key != NULL);
		crex_accel_store_interned_string(p->key);
		C_CE(p->val) = crex_persist_class_entry(C_CE(p->val));
	} CREX_HASH_FOREACH_END();
	CREX_HASH_MAP_FOREACH_BUCKET(class_table, p) {
		if (EXPECTED(C_TYPE(p->val) != IS_ALIAS_PTR)) {
			ce = C_PTR(p->val);
			crex_update_parent_ce(ce);
		}
	} CREX_HASH_FOREACH_END();
#ifdef HAVE_JIT
	JIT_G(on) = orig_jit_on;
	if (JIT_G(on) && JIT_G(opt_level) <= CREX_JIT_LEVEL_OPT_FUNCS &&
	    !ZCG(current_persistent_script)->corrupted) {
	    crex_op_array *op_array;

	    CREX_HASH_MAP_FOREACH_BUCKET(class_table, p) {
			if (EXPECTED(C_TYPE(p->val) != IS_ALIAS_PTR)) {
				ce = C_PTR(p->val);
				CREX_HASH_MAP_FOREACH_PTR(&ce->function_table, op_array) {
					if (op_array->type == CREX_USER_FUNCTION) {
						if (op_array->scope == ce
						 && !(op_array->fn_flags & CREX_ACC_ABSTRACT)
						 && !(op_array->fn_flags & CREX_ACC_TRAIT_CLONE)) {
							crex_jit_op_array(op_array, ZCG(current_persistent_script) ? &ZCG(current_persistent_script)->script : NULL);
							for (uint32_t i = 0; i < op_array->num_dynamic_func_defs; i++) {
								crex_jit_op_array(op_array->dynamic_func_defs[i], ZCG(current_persistent_script) ? &ZCG(current_persistent_script)->script : NULL);
							}
						}
					}
				} CREX_HASH_FOREACH_END();
			}
		} CREX_HASH_FOREACH_END();
	    CREX_HASH_MAP_FOREACH_BUCKET(class_table, p) {
			if (EXPECTED(C_TYPE(p->val) != IS_ALIAS_PTR)) {
				ce = C_PTR(p->val);
				CREX_HASH_MAP_FOREACH_PTR(&ce->function_table, op_array) {
					if (op_array->type == CREX_USER_FUNCTION
					 && !(op_array->fn_flags & CREX_ACC_ABSTRACT)) {
						if ((op_array->scope != ce
						 || (op_array->fn_flags & CREX_ACC_TRAIT_CLONE))
						  && (JIT_G(trigger) == CREX_JIT_ON_FIRST_EXEC
						   || JIT_G(trigger) == CREX_JIT_ON_PROF_REQUEST
						   || JIT_G(trigger) == CREX_JIT_ON_HOT_COUNTERS
						   || JIT_G(trigger) == CREX_JIT_ON_HOT_TRACE)) {
							void *jit_extension = crex_shared_alloc_get_xlat_entry(op_array->opcodes);

							if (jit_extension) {
								CREX_SET_FUNC_INFO(op_array, jit_extension);
							}
						}
					}
				} CREX_HASH_FOREACH_END();
			}
		} CREX_HASH_FOREACH_END();
	}
#endif
}

crex_error_info **crex_persist_warnings(uint32_t num_warnings, crex_error_info **warnings) {
	if (warnings) {
		warnings = crex_shared_memdup_free(warnings, num_warnings * sizeof(crex_error_info *));
		for (uint32_t i = 0; i < num_warnings; i++) {
			warnings[i] = crex_shared_memdup_free(warnings[i], sizeof(crex_error_info));
			crex_accel_store_string(warnings[i]->filename);
			crex_accel_store_string(warnings[i]->message);
		}
	}
	return warnings;
}

static crex_early_binding *crex_persist_early_bindings(
		uint32_t num_early_bindings, crex_early_binding *early_bindings) {
	if (early_bindings) {
		early_bindings = crex_shared_memdup_free(
			early_bindings, num_early_bindings * sizeof(crex_early_binding));
		for (uint32_t i = 0; i < num_early_bindings; i++) {
			crex_accel_store_interned_string(early_bindings[i].lcname);
			crex_accel_store_interned_string(early_bindings[i].rtd_key);
			crex_accel_store_interned_string(early_bindings[i].lc_parent_name);
		}
	}
	return early_bindings;
}

crex_persistent_script *crex_accel_script_persist(crex_persistent_script *script, int for_shm)
{
	Bucket *p;

	script->mem = ZCG(mem);

	CREX_ASSERT(((uintptr_t)ZCG(mem) & 0x7) == 0); /* should be 8 byte aligned */

	script = crex_shared_memdup_free(script, sizeof(crex_persistent_script));
	script->corrupted = false;
	ZCG(current_persistent_script) = script;

	if (!for_shm) {
		/* script is not going to be saved in SHM */
		script->corrupted = true;
	}

	crex_accel_store_interned_string(script->script.filename);

#if defined(__AVX__) || defined(__SSE2__)
	/* Align to 64-byte boundary */
	ZCG(mem) = (void*)(((uintptr_t)ZCG(mem) + 63L) & ~63L);
#else
	CREX_ASSERT(((uintptr_t)ZCG(mem) & 0x7) == 0); /* should be 8 byte aligned */
#endif

#ifdef HAVE_JIT
	if (JIT_G(on) && for_shm) {
		crex_jit_unprotect();
	}
#endif

	crex_map_ptr_extend(ZCSG(map_ptr_last));

	crex_accel_persist_class_table(&script->script.class_table);
	crex_hash_persist(&script->script.function_table);
	CREX_HASH_MAP_FOREACH_BUCKET(&script->script.function_table, p) {
		CREX_ASSERT(p->key != NULL);
		crex_accel_store_interned_string(p->key);
		crex_persist_op_array(&p->val);
	} CREX_HASH_FOREACH_END();
	crex_persist_op_array_ex(&script->script.main_op_array, script);
	if (!script->corrupted) {
		CREX_MAP_PTR_INIT(script->script.main_op_array.run_time_cache, NULL);
		if (script->script.main_op_array.static_variables) {
			CREX_MAP_PTR_NEW(script->script.main_op_array.static_variables_ptr);
		}
#ifdef HAVE_JIT
		if (JIT_G(on) && JIT_G(opt_level) <= CREX_JIT_LEVEL_OPT_FUNCS) {
			crex_jit_op_array(&script->script.main_op_array, &script->script);
		}
#endif
	}
	script->warnings = crex_persist_warnings(script->num_warnings, script->warnings);
	script->early_bindings = crex_persist_early_bindings(
		script->num_early_bindings, script->early_bindings);

	if (for_shm) {
		ZCSG(map_ptr_last) = CG(map_ptr_last);
	}

#ifdef HAVE_JIT
	if (JIT_G(on) && for_shm) {
		if (JIT_G(opt_level) >= CREX_JIT_LEVEL_OPT_SCRIPT) {
			crex_jit_script(&script->script);
		}
		crex_jit_protect();
	}
#endif

	script->corrupted = false;
	ZCG(current_persistent_script) = NULL;

	return script;
}
