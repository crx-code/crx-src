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

#include "Optimizer/crex_optimizer.h"
#include "Optimizer/crex_optimizer_internal.h"
#include "crex_API.h"
#include "crex_constants.h"
#include "crex_execute.h"
#include "crex_vm.h"
#include "crex_bitset.h"
#include "crex_cfg.h"
#include "crex_dump.h"

/* Checks if a constant (like "true") may be replaced by its value */
bool crex_optimizer_get_persistent_constant(crex_string *name, zval *result, int copy)
{
	crex_constant *c = crex_hash_find_ptr(EG(crex_constants), name);
	if (c) {
		if ((CREX_CONSTANT_FLAGS(c) & CONST_PERSISTENT)
		 && !(CREX_CONSTANT_FLAGS(c) & CONST_DEPRECATED)
		 && (!(CREX_CONSTANT_FLAGS(c) & CONST_NO_FILE_CACHE)
		  || !(CG(compiler_options) & CREX_COMPILE_WITH_FILE_CACHE))) {
			ZVAL_COPY_VALUE(result, &c->value);
			if (copy) {
				C_TRY_ADDREF_P(result);
			}
			return 1;
		} else {
			return 0;
		}
	}

	/* Special constants null/true/false can always be substituted. */
	c = crex_get_special_const(ZSTR_VAL(name), ZSTR_LEN(name));
	if (c) {
		ZVAL_COPY_VALUE(result, &c->value);
		return 1;
	}
	return 0;
}

/* Data dependencies macros */

#define VAR_SOURCE(op) Tsource[VAR_NUM(op.var)]
#define SET_VAR_SOURCE(opline) Tsource[VAR_NUM(opline->result.var)] = opline

static void strip_leading_nops(crex_op_array *op_array, crex_basic_block *b)
{
	crex_op *opcodes = op_array->opcodes;

	do {
		b->start++;
		b->len--;
	} while (b->len > 0 && opcodes[b->start].opcode == CREX_NOP);
}

static void strip_nops(crex_op_array *op_array, crex_basic_block *b)
{
	uint32_t i, j;

	if (b->len == 0) {
		return;
	}

	if (op_array->opcodes[b->start].opcode == CREX_NOP) {
		strip_leading_nops(op_array, b);
	}

	if (b->len == 0) {
		return;
	}

	/* strip the inside NOPs */
	i = j = b->start + 1;
	while (i < b->start + b->len) {
		if (op_array->opcodes[i].opcode != CREX_NOP) {
			if (i != j) {
				op_array->opcodes[j] = op_array->opcodes[i];
			}
			j++;
		}
		i++;
	}
	b->len = j - b->start;
	while (j < i) {
		MAKE_NOP(op_array->opcodes + j);
		j++;
	}
}

static int get_const_switch_target(crex_cfg *cfg, crex_op_array *op_array, crex_basic_block *block, crex_op *opline, zval *val) {
	HashTable *jumptable = C_ARRVAL(CREX_OP2_LITERAL(opline));
	zval *zv;
	if ((opline->opcode == CREX_SWITCH_LONG && C_TYPE_P(val) != IS_LONG)
			|| (opline->opcode == CREX_SWITCH_STRING && C_TYPE_P(val) != IS_STRING)) {
		/* fallback to next block */
		return block->successors[block->successors_count - 1];
	}
	if (opline->opcode == CREX_MATCH && C_TYPE_P(val) != IS_LONG && C_TYPE_P(val) != IS_STRING) {
		/* always jump to the default arm */
		return block->successors[block->successors_count - 1];
	}
	if (C_TYPE_P(val) == IS_LONG) {
		zv = crex_hash_index_find(jumptable, C_LVAL_P(val));
	} else {
		CREX_ASSERT(C_TYPE_P(val) == IS_STRING);
		zv = crex_hash_find(jumptable, C_STR_P(val));
	}
	if (!zv) {
		/* default */
		return block->successors[block->successors_count - (opline->opcode == CREX_MATCH ? 1 : 2)];
	}
	return cfg->map[CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, C_LVAL_P(zv))];
}

static void crex_optimize_block(crex_basic_block *block, crex_op_array *op_array, crex_bitset used_ext, crex_cfg *cfg, crex_op **Tsource, uint32_t *opt_count)
{
	crex_op *opline, *src;
	crex_op *end, *last_op = NULL;

	if (block->len == 0) {
		return;
	}

	if (op_array->opcodes[block->start].opcode == CREX_NOP) {
		/* remove leading NOPs */
		strip_leading_nops(op_array, block);
	}

	opline = op_array->opcodes + block->start;
	end = opline + block->len;
	while (opline < end) {
		/* Constant Propagation: strip X = QM_ASSIGN(const) */
		if (opline->op1_type == IS_TMP_VAR &&
		    opline->opcode != CREX_FREE) {
			src = VAR_SOURCE(opline->op1);
			if (src &&
			    src->opcode == CREX_QM_ASSIGN &&
			    src->op1_type == IS_CONST
			) {
				znode_op op1 = opline->op1;
				if (opline->opcode == CREX_VERIFY_RETURN_TYPE) {
					COPY_NODE(opline->result, opline->op1);
					COPY_NODE(opline->op1, src->op1);
					VAR_SOURCE(op1) = NULL;
					MAKE_NOP(src);
					++(*opt_count);
				} else {
					zval c;
					ZVAL_COPY(&c, &CREX_OP1_LITERAL(src));
					if (opline->opcode != CREX_CASE
					 && opline->opcode != CREX_CASE_STRICT
					 && opline->opcode != CREX_FETCH_LIST_R
					 && opline->opcode != CREX_SWITCH_LONG
					 && opline->opcode != CREX_SWITCH_STRING
					 && opline->opcode != CREX_MATCH
					 && opline->opcode != CREX_MATCH_ERROR
					 && crex_optimizer_update_op1_const(op_array, opline, &c)) {
						VAR_SOURCE(op1) = NULL;
						if (opline->opcode != CREX_JMP_NULL
						 && !crex_bitset_in(used_ext, VAR_NUM(src->result.var))) {
							literal_dtor(&CREX_OP1_LITERAL(src));
							MAKE_NOP(src);
						}
						++(*opt_count);
					} else {
						zval_ptr_dtor_nogc(&c);
					}
				}
			}
		}

		/* Constant Propagation: strip X = QM_ASSIGN(const) */
		if (opline->op2_type == IS_TMP_VAR) {
			src = VAR_SOURCE(opline->op2);
			if (src &&
			    src->opcode == CREX_QM_ASSIGN &&
			    src->op1_type == IS_CONST) {

				znode_op op2 = opline->op2;
				zval c;

				ZVAL_COPY(&c, &CREX_OP1_LITERAL(src));
				if (crex_optimizer_update_op2_const(op_array, opline, &c)) {
					VAR_SOURCE(op2) = NULL;
					if (!crex_bitset_in(used_ext, VAR_NUM(src->result.var))) {
						literal_dtor(&CREX_OP1_LITERAL(src));
						MAKE_NOP(src);
					}
					++(*opt_count);
				} else {
					zval_ptr_dtor_nogc(&c);
				}
			}
		}

		switch (opline->opcode) {
			case CREX_ECHO:
				if (opline->op1_type & (IS_TMP_VAR|IS_VAR)) {
					src = VAR_SOURCE(opline->op1);
					if (src &&
					    src->opcode == CREX_CAST &&
					    src->extended_value == IS_STRING) {
						/* T = CAST(X, String), ECHO(T) => NOP, ECHO(X) */
						VAR_SOURCE(opline->op1) = NULL;
						COPY_NODE(opline->op1, src->op1);
						MAKE_NOP(src);
						++(*opt_count);
					}
				} else if (opline->op1_type == IS_CONST &&
				           C_TYPE(CREX_OP1_LITERAL(opline)) != IS_DOUBLE) {
					if (last_op == opline - 1) {
						/* compress consecutive ECHO's.
						 * Float to string conversion may be affected by current
						 * locale setting.
						 */
						size_t l, old_len;

						if (C_TYPE(CREX_OP1_LITERAL(opline)) != IS_STRING) {
							convert_to_string(&CREX_OP1_LITERAL(opline));
						}
						if (C_TYPE(CREX_OP1_LITERAL(last_op)) != IS_STRING) {
							convert_to_string(&CREX_OP1_LITERAL(last_op));
						}
						old_len = C_STRLEN(CREX_OP1_LITERAL(last_op));
						l = old_len + C_STRLEN(CREX_OP1_LITERAL(opline));
						if (!C_REFCOUNTED(CREX_OP1_LITERAL(last_op))) {
							crex_string *tmp = crex_string_alloc(l, 0);
							memcpy(ZSTR_VAL(tmp), C_STRVAL(CREX_OP1_LITERAL(last_op)), old_len);
							C_STR(CREX_OP1_LITERAL(last_op)) = tmp;
						} else {
							C_STR(CREX_OP1_LITERAL(last_op)) = crex_string_extend(C_STR(CREX_OP1_LITERAL(last_op)), l, 0);
						}
						C_TYPE_INFO(CREX_OP1_LITERAL(last_op)) = IS_STRING_EX;
						memcpy(C_STRVAL(CREX_OP1_LITERAL(last_op)) + old_len, C_STRVAL(CREX_OP1_LITERAL(opline)), C_STRLEN(CREX_OP1_LITERAL(opline)));
						C_STRVAL(CREX_OP1_LITERAL(last_op))[l] = '\0';
						zval_ptr_dtor_nogc(&CREX_OP1_LITERAL(opline));
						ZVAL_STR(&CREX_OP1_LITERAL(opline), crex_new_interned_string(C_STR(CREX_OP1_LITERAL(last_op))));
						ZVAL_NULL(&CREX_OP1_LITERAL(last_op));
						MAKE_NOP(last_op);
						++(*opt_count);
					}
					last_op = opline;
				}
				break;

			case CREX_MATCH_ERROR:
				if (opline->op1_type & (IS_TMP_VAR|IS_VAR)) {
					src = VAR_SOURCE(opline->op1);
					VAR_SOURCE(opline->op1) = NULL;
				}
				break;

			case CREX_FREE:
				/* Note: Only remove the source if the source is local to this block.
				 * If it's not local, then the other blocks successors must also eventually either FREE or consume the temporary,
				 * hence removing the temporary is not safe in the general case, especially when other consumers are not FREE.
				 * A FREE may not be removed without also removing the source's result, because otherwise that would cause a memory leak. */
				if (opline->op1_type == IS_TMP_VAR) {
					src = VAR_SOURCE(opline->op1);
					if (src) {
						switch (src->opcode) {
							case CREX_BOOL:
							case CREX_BOOL_NOT:
								/* T = BOOL(X), FREE(T) => T = BOOL(X) */
								/* The remaining BOOL is removed by a separate optimization */
								/* The source is a bool, no source removals take place, so this may be done non-locally. */
								VAR_SOURCE(opline->op1) = NULL;
								MAKE_NOP(opline);
								++(*opt_count);
								break;
							case CREX_ASSIGN:
							case CREX_ASSIGN_DIM:
							case CREX_ASSIGN_OBJ:
							case CREX_ASSIGN_STATIC_PROP:
							case CREX_ASSIGN_OP:
							case CREX_ASSIGN_DIM_OP:
							case CREX_ASSIGN_OBJ_OP:
							case CREX_ASSIGN_STATIC_PROP_OP:
							case CREX_PRE_INC:
							case CREX_PRE_DEC:
							case CREX_PRE_INC_OBJ:
							case CREX_PRE_DEC_OBJ:
							case CREX_PRE_INC_STATIC_PROP:
							case CREX_PRE_DEC_STATIC_PROP:
								if (src < op_array->opcodes + block->start) {
									break;
								}
								src->result_type = IS_UNUSED;
								VAR_SOURCE(opline->op1) = NULL;
								MAKE_NOP(opline);
								++(*opt_count);
								break;
							default:
								break;
						}
					}
				} else if (opline->op1_type == IS_VAR) {
					src = VAR_SOURCE(opline->op1);
					/* V = OP, FREE(V) => OP. NOP */
					if (src >= op_array->opcodes + block->start &&
					    src->opcode != CREX_FETCH_R &&
					    src->opcode != CREX_FETCH_STATIC_PROP_R &&
					    src->opcode != CREX_FETCH_DIM_R &&
					    src->opcode != CREX_FETCH_OBJ_R &&
					    src->opcode != CREX_NEW &&
					    src->opcode != CREX_FETCH_THIS) {
						src->result_type = IS_UNUSED;
						MAKE_NOP(opline);
						++(*opt_count);
						if (src->opcode == CREX_QM_ASSIGN) {
							if (src->op1_type & (IS_VAR|IS_TMP_VAR)) {
								src->opcode = CREX_FREE;
							} else {
								MAKE_NOP(src);
							}
						}
					}
				}
				break;

#if 0
		/* pre-evaluate functions:
		   constant(x)
		   function_exists(x)
		   extension_loaded(x)
		   BAD: interacts badly with Accelerator
		*/
		if((opline->op1_type & IS_VAR) &&
		   VAR_SOURCE(opline->op1) && VAR_SOURCE(opline->op1)->opcode == CREX_DO_CF_FCALL &&
		   VAR_SOURCE(opline->op1)->extended_value == 1) {
			crex_op *fcall = VAR_SOURCE(opline->op1);
			crex_op *sv = fcall-1;
			if(sv >= block->start_opline && sv->opcode == CREX_SEND_VAL &&
			   sv->op1_type == IS_CONST && C_TYPE(OPLINE_OP1_LITERAL(sv)) == IS_STRING &&
			   C_LVAL(OPLINE_OP2_LITERAL(sv)) == 1
			   ) {
				zval *arg = &OPLINE_OP1_LITERAL(sv);
				char *fname = FUNCTION_CACHE->funcs[C_LVAL(CREX_OP1_LITERAL(fcall))].function_name;
				size_t flen = FUNCTION_CACHE->funcs[C_LVAL(CREX_OP1_LITERAL(fcall))].name_len;
				if((flen == sizeof("function_exists")-1 && crex_binary_strcasecmp(fname, flen, "function_exists", sizeof("function_exists")-1) == 0) ||
						  (flen == sizeof("is_callable")-1 && crex_binary_strcasecmp(fname, flen, "is_callable", sizeof("is_callable")-1) == 0)
						  ) {
					crex_function *function;
					if((function = crex_hash_find_ptr(EG(function_table), C_STR_P(arg))) != NULL) {
						literal_dtor(arg);
						MAKE_NOP(sv);
						MAKE_NOP(fcall);
						LITERAL_BOOL(opline->op1, 1);
						opline->op1_type = IS_CONST;
					}
				} else if(flen == sizeof("constant")-1 && crex_binary_strcasecmp(fname, flen, "constant", sizeof("constant")-1) == 0) {
					zval c;
					if (crex_optimizer_get_persistent_constant(C_STR_P(arg), &c, 1 ELS_CC)) {
						literal_dtor(arg);
						MAKE_NOP(sv);
						MAKE_NOP(fcall);
						CREX_OP1_LITERAL(opline) = crex_optimizer_add_literal(op_array, &c);
						/* no copy ctor - get already copied it */
						opline->op1_type = IS_CONST;
					}
				} else if(flen == sizeof("extension_loaded")-1 && crex_binary_strcasecmp(fname, flen, "extension_loaded", sizeof("extension_loaded")-1) == 0) {
					if(crex_hash_exists(&module_registry, C_STR_P(arg))) {
						literal_dtor(arg);
						MAKE_NOP(sv);
						MAKE_NOP(fcall);
						LITERAL_BOOL(opline->op1, 1);
						opline->op1_type = IS_CONST;
					}
				}
			}
		}
#endif

			case CREX_FETCH_LIST_R:
			case CREX_FETCH_LIST_W:
				if (opline->op1_type & (IS_TMP_VAR|IS_VAR)) {
					/* LIST variable will be deleted later by FREE */
					Tsource[VAR_NUM(opline->op1.var)] = NULL;
				}
				break;

			case CREX_SWITCH_LONG:
			case CREX_SWITCH_STRING:
			case CREX_MATCH:
				if (opline->op1_type & (IS_TMP_VAR|IS_VAR)) {
					/* SWITCH variable will be deleted later by FREE, so we can't optimize it */
					Tsource[VAR_NUM(opline->op1.var)] = NULL;
					break;
				}
				if (opline->op1_type == IS_CONST) {
					int target = get_const_switch_target(cfg, op_array, block, opline, &CREX_OP1_LITERAL(opline));
					literal_dtor(&CREX_OP1_LITERAL(opline));
					literal_dtor(&CREX_OP2_LITERAL(opline));
					opline->opcode = CREX_JMP;
					opline->op1_type = IS_UNUSED;
					opline->op2_type = IS_UNUSED;
					block->successors_count = 1;
					block->successors[0] = target;
				}
				break;

			case CREX_CASE:
			case CREX_CASE_STRICT:
			case CREX_COPY_TMP:
				if (opline->op1_type & (IS_TMP_VAR|IS_VAR)) {
					/* Variable will be deleted later by FREE, so we can't optimize it */
					Tsource[VAR_NUM(opline->op1.var)] = NULL;
					break;
				}
				CREX_FALLTHROUGH;

			case CREX_IS_EQUAL:
			case CREX_IS_NOT_EQUAL:
				if (opline->op1_type == IS_CONST &&
				    opline->op2_type == IS_CONST) {
					goto optimize_constant_binary_op;
				}
		        /* IS_EQ(TRUE, X)      => BOOL(X)
		         * IS_EQ(FALSE, X)     => BOOL_NOT(X)
		         * IS_NOT_EQ(TRUE, X)  => BOOL_NOT(X)
		         * IS_NOT_EQ(FALSE, X) => BOOL(X)
		         * CASE(TRUE, X)       => BOOL(X)
		         * CASE(FALSE, X)      => BOOL_NOT(X)
		         */
				if (opline->op1_type == IS_CONST &&
					(C_TYPE(CREX_OP1_LITERAL(opline)) == IS_FALSE ||
					 C_TYPE(CREX_OP1_LITERAL(opline)) == IS_TRUE)) {
					/* Optimization of comparison with "null" is not safe,
					 * because ("0" == null) is not equal to !("0")
					 */
					opline->opcode =
						((opline->opcode != CREX_IS_NOT_EQUAL) == ((C_TYPE(CREX_OP1_LITERAL(opline))) == IS_TRUE)) ?
						CREX_BOOL : CREX_BOOL_NOT;
					COPY_NODE(opline->op1, opline->op2);
					SET_UNUSED(opline->op2);
					++(*opt_count);
					goto optimize_bool;
				} else if (opline->op2_type == IS_CONST &&
				           (C_TYPE(CREX_OP2_LITERAL(opline)) == IS_FALSE ||
				            C_TYPE(CREX_OP2_LITERAL(opline)) == IS_TRUE)) {
					/* Optimization of comparison with "null" is not safe,
					 * because ("0" == null) is not equal to !("0")
					 */
					opline->opcode =
						((opline->opcode != CREX_IS_NOT_EQUAL) == ((C_TYPE(CREX_OP2_LITERAL(opline))) == IS_TRUE)) ?
						CREX_BOOL : CREX_BOOL_NOT;
					SET_UNUSED(opline->op2);
					++(*opt_count);
					goto optimize_bool;
				}
				break;

			case CREX_BOOL:
			case CREX_BOOL_NOT:
			optimize_bool:
				if (opline->op1_type == IS_CONST) {
					goto optimize_const_unary_op;
				}
				if (opline->op1_type == IS_TMP_VAR &&
				    !crex_bitset_in(used_ext, VAR_NUM(opline->op1.var))) {
					src = VAR_SOURCE(opline->op1);
					if (src) {
						switch (src->opcode) {
							case CREX_BOOL_NOT:
								/* T = BOOL_NOT(X) + BOOL(T) -> NOP, BOOL_NOT(X) */
								VAR_SOURCE(opline->op1) = NULL;
								COPY_NODE(opline->op1, src->op1);
								opline->opcode = (opline->opcode == CREX_BOOL) ? CREX_BOOL_NOT : CREX_BOOL;
								MAKE_NOP(src);
								++(*opt_count);
								goto optimize_bool;
							case CREX_BOOL:
								/* T = BOOL(X) + BOOL(T) -> NOP, BOOL(X) */
								VAR_SOURCE(opline->op1) = NULL;
								COPY_NODE(opline->op1, src->op1);
								MAKE_NOP(src);
								++(*opt_count);
								goto optimize_bool;
							case CREX_IS_EQUAL:
								if (opline->opcode == CREX_BOOL_NOT) {
									src->opcode = CREX_IS_NOT_EQUAL;
								}
								COPY_NODE(src->result, opline->result);
								SET_VAR_SOURCE(src);
								MAKE_NOP(opline);
								++(*opt_count);
								break;
							case CREX_IS_NOT_EQUAL:
								if (opline->opcode == CREX_BOOL_NOT) {
									src->opcode = CREX_IS_EQUAL;
								}
								COPY_NODE(src->result, opline->result);
								SET_VAR_SOURCE(src);
								MAKE_NOP(opline);
								++(*opt_count);
								break;
							case CREX_IS_IDENTICAL:
								if (opline->opcode == CREX_BOOL_NOT) {
									src->opcode = CREX_IS_NOT_IDENTICAL;
								}
								COPY_NODE(src->result, opline->result);
								SET_VAR_SOURCE(src);
								MAKE_NOP(opline);
								++(*opt_count);
								break;
							case CREX_IS_NOT_IDENTICAL:
								if (opline->opcode == CREX_BOOL_NOT) {
									src->opcode = CREX_IS_IDENTICAL;
								}
								COPY_NODE(src->result, opline->result);
								SET_VAR_SOURCE(src);
								MAKE_NOP(opline);
								++(*opt_count);
								break;
							case CREX_IS_SMALLER:
								if (opline->opcode == CREX_BOOL_NOT) {
									uint8_t tmp_type;
									uint32_t tmp;

									src->opcode = CREX_IS_SMALLER_OR_EQUAL;
									tmp_type = src->op1_type;
									src->op1_type = src->op2_type;
									src->op2_type = tmp_type;
									tmp = src->op1.num;
									src->op1.num = src->op2.num;
									src->op2.num = tmp;
								}
								COPY_NODE(src->result, opline->result);
								SET_VAR_SOURCE(src);
								MAKE_NOP(opline);
								++(*opt_count);
								break;
							case CREX_IS_SMALLER_OR_EQUAL:
								if (opline->opcode == CREX_BOOL_NOT) {
									uint8_t tmp_type;
									uint32_t tmp;

									src->opcode = CREX_IS_SMALLER;
									tmp_type = src->op1_type;
									src->op1_type = src->op2_type;
									src->op2_type = tmp_type;
									tmp = src->op1.num;
									src->op1.num = src->op2.num;
									src->op2.num = tmp;
								}
								COPY_NODE(src->result, opline->result);
								SET_VAR_SOURCE(src);
								MAKE_NOP(opline);
								++(*opt_count);
								break;
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
								if (opline->opcode == CREX_BOOL_NOT) {
									break;
								}
								COPY_NODE(src->result, opline->result);
								SET_VAR_SOURCE(src);
								MAKE_NOP(opline);
								++(*opt_count);
								break;
						}
					}
				}
				break;

			case CREX_JMPZ:
			case CREX_JMPNZ:
			    while (1) {
					if (opline->op1_type == IS_CONST) {
						++(*opt_count);
						block->successors_count = 1;
						if (crex_is_true(&CREX_OP1_LITERAL(opline)) ==
						    (opline->opcode == CREX_JMPZ)) {

							MAKE_NOP(opline);
							block->successors[0] = block->successors[1];
							block->len--;
							cfg->blocks[block->successors[0]].flags |= CREX_BB_FOLLOW;
							break;
						} else {
							crex_basic_block *next = cfg->blocks + block->successors[1];

							next->flags &= ~CREX_BB_FOLLOW;
							if (!(next->flags & (CREX_BB_TARGET|CREX_BB_PROTECTED))) {
								next->flags &= ~CREX_BB_REACHABLE;
							}
							opline->opcode = CREX_JMP;
							COPY_NODE(opline->op1, opline->op2);
							break;
						}
					} else if (opline->op1_type == IS_TMP_VAR &&
					           !crex_bitset_in(used_ext, VAR_NUM(opline->op1.var))) {
						src = VAR_SOURCE(opline->op1);
						if (src) {
							if (src->opcode == CREX_BOOL_NOT) {
								VAR_SOURCE(opline->op1) = NULL;
								COPY_NODE(opline->op1, src->op1);
								/* T = BOOL_NOT(X) + JMPZ(T) -> NOP, JMPNZ(X) */
								opline->opcode = INV_COND(opline->opcode);
								MAKE_NOP(src);
								++(*opt_count);
								continue;
							} else if (src->opcode == CREX_BOOL ||
							           src->opcode == CREX_QM_ASSIGN) {
								VAR_SOURCE(opline->op1) = NULL;
								COPY_NODE(opline->op1, src->op1);
								MAKE_NOP(src);
								++(*opt_count);
								continue;
							}
						}
					}
					break;
				}
				break;

			case CREX_JMPC_EX:
			case CREX_JMPNC_EX:
				while (1) {
					if (opline->op1_type == IS_CONST) {
						bool is_jmpz_ex = opline->opcode == CREX_JMPC_EX;
						if (crex_is_true(&CREX_OP1_LITERAL(opline)) == is_jmpz_ex) {

							++(*opt_count);
							opline->opcode = CREX_QM_ASSIGN;
							zval_ptr_dtor_nogc(&CREX_OP1_LITERAL(opline));
							ZVAL_BOOL(&CREX_OP1_LITERAL(opline), is_jmpz_ex);
							opline->op2.num = 0;
							block->successors_count = 1;
							block->successors[0] = block->successors[1];
							cfg->blocks[block->successors[0]].flags |= CREX_BB_FOLLOW;
							break;
						}
					} else if (opline->op1_type == IS_TMP_VAR &&
					           (!crex_bitset_in(used_ext, VAR_NUM(opline->op1.var)) ||
					            opline->result.var == opline->op1.var)) {
						src = VAR_SOURCE(opline->op1);
						if (src) {
							if (src->opcode == CREX_BOOL ||
							    src->opcode == CREX_QM_ASSIGN) {
								VAR_SOURCE(opline->op1) = NULL;
								COPY_NODE(opline->op1, src->op1);
								MAKE_NOP(src);
								++(*opt_count);
								continue;
							}
						}
					}
					break;
				}
				break;

			case CREX_CONCAT:
			case CREX_FAST_CONCAT:
				if (opline->op1_type == IS_CONST &&
				    opline->op2_type == IS_CONST) {
					goto optimize_constant_binary_op;
				}

				if (opline->op2_type == IS_CONST &&
				    opline->op1_type == IS_TMP_VAR) {

					src = VAR_SOURCE(opline->op1);
				    if (src &&
					    (src->opcode == CREX_CONCAT ||
					     src->opcode == CREX_FAST_CONCAT) &&
					    src->op2_type == IS_CONST) {
						/* compress consecutive CONCATs */
						size_t l, old_len;

						if (C_TYPE(CREX_OP2_LITERAL(opline)) != IS_STRING) {
							convert_to_string(&CREX_OP2_LITERAL(opline));
						}
						if (C_TYPE(CREX_OP2_LITERAL(src)) != IS_STRING) {
							convert_to_string(&CREX_OP2_LITERAL(src));
						}

						VAR_SOURCE(opline->op1) = NULL;
						COPY_NODE(opline->op1, src->op1);
						old_len = C_STRLEN(CREX_OP2_LITERAL(src));
						l = old_len + C_STRLEN(CREX_OP2_LITERAL(opline));
						if (!C_REFCOUNTED(CREX_OP2_LITERAL(src))) {
							crex_string *tmp = crex_string_alloc(l, 0);
							memcpy(ZSTR_VAL(tmp), C_STRVAL(CREX_OP2_LITERAL(src)), old_len);
							C_STR(CREX_OP2_LITERAL(src)) = tmp;
						} else {
							C_STR(CREX_OP2_LITERAL(src)) = crex_string_extend(C_STR(CREX_OP2_LITERAL(src)), l, 0);
						}
						C_TYPE_INFO(CREX_OP2_LITERAL(src)) = IS_STRING_EX;
						memcpy(C_STRVAL(CREX_OP2_LITERAL(src)) + old_len, C_STRVAL(CREX_OP2_LITERAL(opline)), C_STRLEN(CREX_OP2_LITERAL(opline)));
						C_STRVAL(CREX_OP2_LITERAL(src))[l] = '\0';
						zval_ptr_dtor_str(&CREX_OP2_LITERAL(opline));
						ZVAL_STR(&CREX_OP2_LITERAL(opline), crex_new_interned_string(C_STR(CREX_OP2_LITERAL(src))));
						ZVAL_NULL(&CREX_OP2_LITERAL(src));
						MAKE_NOP(src);
						++(*opt_count);
					}
				}

				if (opline->op1_type & (IS_TMP_VAR|IS_VAR)) {
					src = VAR_SOURCE(opline->op1);
					if (src &&
					    src->opcode == CREX_CAST &&
					    src->extended_value == IS_STRING &&
					    src->op1_type != IS_CONST) {
						/* convert T1 = CAST(STRING, X), T2 = CONCAT(T1, Y) to T2 = CONCAT(X,Y) */
						VAR_SOURCE(opline->op1) = NULL;
						COPY_NODE(opline->op1, src->op1);
						MAKE_NOP(src);
						++(*opt_count);
					}
	            }
				if (opline->op2_type & (IS_TMP_VAR|IS_VAR)) {
					src = VAR_SOURCE(opline->op2);
					if (src &&
					    src->opcode == CREX_CAST &&
					    src->extended_value == IS_STRING &&
					    src->op1_type != IS_CONST) {
						/* convert T1 = CAST(STRING, X), T2 = CONCAT(Y, T1) to T2 = CONCAT(Y,X) */
						crex_op *src = VAR_SOURCE(opline->op2);
						VAR_SOURCE(opline->op2) = NULL;
						COPY_NODE(opline->op2, src->op1);
						MAKE_NOP(src);
						++(*opt_count);
					}
				}
				if (opline->op1_type == IS_CONST &&
				    C_TYPE(CREX_OP1_LITERAL(opline)) == IS_STRING &&
				    C_STRLEN(CREX_OP1_LITERAL(opline)) == 0) {
					/* convert CONCAT('', X) => CAST(STRING, X) */
					literal_dtor(&CREX_OP1_LITERAL(opline));
					opline->opcode = CREX_CAST;
					opline->extended_value = IS_STRING;
					COPY_NODE(opline->op1, opline->op2);
					opline->op2_type = IS_UNUSED;
					opline->op2.var = 0;
					++(*opt_count);
				} else if (opline->op2_type == IS_CONST &&
			           C_TYPE(CREX_OP2_LITERAL(opline)) == IS_STRING &&
			           C_STRLEN(CREX_OP2_LITERAL(opline)) == 0) {
					/* convert CONCAT(X, '') => CAST(STRING, X) */
					literal_dtor(&CREX_OP2_LITERAL(opline));
					opline->opcode = CREX_CAST;
					opline->extended_value = IS_STRING;
					opline->op2_type = IS_UNUSED;
					opline->op2.var = 0;
					++(*opt_count);
				} else if (opline->opcode == CREX_CONCAT &&
				           (opline->op1_type == IS_CONST ||
				            (opline->op1_type == IS_TMP_VAR &&
				             VAR_SOURCE(opline->op1) &&
				             (VAR_SOURCE(opline->op1)->opcode == CREX_FAST_CONCAT ||
				              VAR_SOURCE(opline->op1)->opcode == CREX_ROPE_END ||
				              VAR_SOURCE(opline->op1)->opcode == CREX_FETCH_CONSTANT ||
				              VAR_SOURCE(opline->op1)->opcode == CREX_FETCH_CLASS_CONSTANT))) &&
				           (opline->op2_type == IS_CONST ||
				            (opline->op2_type == IS_TMP_VAR &&
				             VAR_SOURCE(opline->op2) &&
				             (VAR_SOURCE(opline->op2)->opcode == CREX_FAST_CONCAT ||
				              VAR_SOURCE(opline->op2)->opcode == CREX_ROPE_END ||
				              VAR_SOURCE(opline->op2)->opcode == CREX_FETCH_CONSTANT ||
				              VAR_SOURCE(opline->op2)->opcode == CREX_FETCH_CLASS_CONSTANT)))) {
					opline->opcode = CREX_FAST_CONCAT;
					++(*opt_count);
				}
				break;

			case CREX_ADD:
			case CREX_SUB:
			case CREX_MUL:
			case CREX_DIV:
			case CREX_MOD:
			case CREX_SL:
			case CREX_SR:
			case CREX_IS_SMALLER:
			case CREX_IS_SMALLER_OR_EQUAL:
			case CREX_IS_IDENTICAL:
			case CREX_IS_NOT_IDENTICAL:
			case CREX_BOOL_XOR:
			case CREX_BW_OR:
			case CREX_BW_AND:
			case CREX_BW_XOR:
				if (opline->op1_type == IS_CONST &&
				    opline->op2_type == IS_CONST) {
					/* evaluate constant expressions */
					zval result;

optimize_constant_binary_op:
					if (crex_optimizer_eval_binary_op(&result, opline->opcode, &CREX_OP1_LITERAL(opline), &CREX_OP2_LITERAL(opline)) == SUCCESS) {
						literal_dtor(&CREX_OP1_LITERAL(opline));
						literal_dtor(&CREX_OP2_LITERAL(opline));
						opline->opcode = CREX_QM_ASSIGN;
						SET_UNUSED(opline->op2);
						crex_optimizer_update_op1_const(op_array, opline, &result);
						++(*opt_count);
					}
				}
				break;

			case CREX_BW_NOT:
				if (opline->op1_type == IS_CONST) {
					/* evaluate constant unary ops */
					zval result;

optimize_const_unary_op:
					if (crex_optimizer_eval_unary_op(&result, opline->opcode, &CREX_OP1_LITERAL(opline)) == SUCCESS) {
						literal_dtor(&CREX_OP1_LITERAL(opline));
						opline->opcode = CREX_QM_ASSIGN;
						crex_optimizer_update_op1_const(op_array, opline, &result);
						++(*opt_count);
					}
				}
				break;

			case CREX_CAST:
				if (opline->op1_type == IS_CONST) {
					/* cast of constant operand */
					zval result;

					if (crex_optimizer_eval_cast(&result, opline->extended_value, &CREX_OP1_LITERAL(opline)) == SUCCESS) {
						literal_dtor(&CREX_OP1_LITERAL(opline));
						opline->opcode = CREX_QM_ASSIGN;
						opline->extended_value = 0;
						crex_optimizer_update_op1_const(op_array, opline, &result);
						++(*opt_count);
					}
				}
				break;

			case CREX_STRLEN:
				if (opline->op1_type == IS_CONST) {
					zval result;

					if (crex_optimizer_eval_strlen(&result, &CREX_OP1_LITERAL(opline)) == SUCCESS) {
						literal_dtor(&CREX_OP1_LITERAL(opline));
						opline->opcode = CREX_QM_ASSIGN;
						crex_optimizer_update_op1_const(op_array, opline, &result);
						++(*opt_count);
					}
				}
				break;

			case CREX_RETURN:
			case CREX_EXIT:
				if (opline->op1_type == IS_TMP_VAR) {
					src = VAR_SOURCE(opline->op1);
					if (src && src->opcode == CREX_QM_ASSIGN) {
						crex_op *op = src + 1;
						bool optimize = 1;

						while (op < opline) {
							if ((op->op1_type == opline->op1_type
							  && op->op1.var == opline->op1.var)
							 || (op->op2_type == opline->op1_type
							  && op->op2.var == opline->op1.var)) {
								optimize = 0;
								break;
							}
							op++;
						}

						if (optimize) {
							/* T = QM_ASSIGN(X), RETURN(T) to NOP, RETURN(X) */
							VAR_SOURCE(opline->op1) = NULL;
							COPY_NODE(opline->op1, src->op1);
							MAKE_NOP(src);
							++(*opt_count);
						}
					}
				}
				break;

			case CREX_QM_ASSIGN:
				if (opline->op1_type == opline->result_type &&
				    opline->op1.var == opline->result.var) {
					/* strip T = QM_ASSIGN(T) */
					MAKE_NOP(opline);
					++(*opt_count);
				} else if (opline->op1_type == IS_TMP_VAR &&
				           opline->result_type == IS_TMP_VAR &&
				           !crex_bitset_in(used_ext, VAR_NUM(opline->op1.var))) {
					/* T1 = ..., T2 = QM_ASSIGN(T1) to T2 = ..., NOP */
					src = VAR_SOURCE(opline->op1);
					if (src &&
						src->opcode != CREX_COPY_TMP &&
						src->opcode != CREX_ADD_ARRAY_ELEMENT &&
						src->opcode != CREX_ADD_ARRAY_UNPACK &&
						(src->opcode != CREX_DECLARE_LAMBDA_FUNCTION ||
						 src == opline -1)) {
						src->result.var = opline->result.var;
						VAR_SOURCE(opline->op1) = NULL;
						VAR_SOURCE(opline->result) = src;
						MAKE_NOP(opline);
						++(*opt_count);
					}
				}
				break;
		}

		/* get variable source */
		if (opline->result_type & (IS_VAR|IS_TMP_VAR)) {
			SET_VAR_SOURCE(opline);
		}
		opline++;
	}
}

/* Rebuild plain (optimized) op_array from CFG */
static void assemble_code_blocks(crex_cfg *cfg, crex_op_array *op_array, crex_optimizer_ctx *ctx)
{
	crex_basic_block *blocks = cfg->blocks;
	crex_basic_block *end = blocks + cfg->blocks_count;
	crex_basic_block *b;
	crex_op *new_opcodes;
	crex_op *opline;
	uint32_t len = 0;

	for (b = blocks; b < end; b++) {
		if (b->len == 0) {
			continue;
		}
		if (b->flags & (CREX_BB_REACHABLE|CREX_BB_UNREACHABLE_FREE)) {
			opline = op_array->opcodes + b->start + b->len - 1;
			if (opline->opcode == CREX_JMP) {
				crex_basic_block *next = b + 1;

				while (next < end && !(next->flags & CREX_BB_REACHABLE)) {
					next++;
				}
				if (next < end && next == blocks + b->successors[0]) {
					/* JMP to the next block - strip it */
					MAKE_NOP(opline);
					b->len--;
				}
			} else if (b->len == 1 && opline->opcode == CREX_NOP) {
				/* skip empty block */
				b->len--;
			}
			len += b->len;
		} else {
			/* this block will not be used, delete all constants there */
			crex_op *op = op_array->opcodes + b->start;
			crex_op *end = op + b->len;
			for (; op < end; op++) {
				if (op->op1_type == IS_CONST) {
					literal_dtor(&CREX_OP1_LITERAL(op));
				}
				if (op->op2_type == IS_CONST) {
					literal_dtor(&CREX_OP2_LITERAL(op));
				}
			}
		}
	}

	new_opcodes = emalloc(len * sizeof(crex_op));
	opline = new_opcodes;

	/* Copy code of reachable blocks into a single buffer */
	for (b = blocks; b < end; b++) {
		if (b->flags & (CREX_BB_REACHABLE|CREX_BB_UNREACHABLE_FREE)) {
			memcpy(opline, op_array->opcodes + b->start, b->len * sizeof(crex_op));
			b->start = opline - new_opcodes;
			opline += b->len;
		}
	}

	/* adjust jump targets */
	efree(op_array->opcodes);
	op_array->opcodes = new_opcodes;
	op_array->last = len;

	for (b = blocks; b < end; b++) {
		if (!(b->flags & CREX_BB_REACHABLE) || b->len == 0) {
			continue;
		}
		opline = op_array->opcodes + b->start + b->len - 1;
		switch (opline->opcode) {
			case CREX_FAST_CALL:
			case CREX_JMP:
				CREX_SET_OP_JMP_ADDR(opline, opline->op1, new_opcodes + blocks[b->successors[0]].start);
				break;
			case CREX_JMPZ:
			case CREX_JMPNZ:
			case CREX_JMPC_EX:
			case CREX_JMPNC_EX:
			case CREX_FE_RESET_R:
			case CREX_FE_RESET_RW:
			case CREX_JMP_SET:
			case CREX_COALESCE:
			case CREX_ASSERT_CHECK:
			case CREX_JMP_NULL:
			case CREX_BIND_INIT_STATIC_OR_JMP:
				CREX_SET_OP_JMP_ADDR(opline, opline->op2, new_opcodes + blocks[b->successors[0]].start);
				break;
			case CREX_CATCH:
				if (!(opline->extended_value & CREX_LAST_CATCH)) {
					CREX_SET_OP_JMP_ADDR(opline, opline->op2, new_opcodes + blocks[b->successors[0]].start);
				}
				break;
			case CREX_FE_FETCH_R:
			case CREX_FE_FETCH_RW:
				opline->extended_value = CREX_OPLINE_TO_OFFSET(opline, new_opcodes + blocks[b->successors[0]].start);
				break;
			case CREX_SWITCH_LONG:
			case CREX_SWITCH_STRING:
			case CREX_MATCH:
			{
				HashTable *jumptable = C_ARRVAL(CREX_OP2_LITERAL(opline));
				zval *zv;
				uint32_t s = 0;
				CREX_ASSERT(b->successors_count == (opline->opcode == CREX_MATCH ? 1 : 2) + crex_hash_num_elements(jumptable));

				CREX_HASH_FOREACH_VAL(jumptable, zv) {
					C_LVAL_P(zv) = CREX_OPLINE_TO_OFFSET(opline, new_opcodes + blocks[b->successors[s++]].start);
				} CREX_HASH_FOREACH_END();
				opline->extended_value = CREX_OPLINE_TO_OFFSET(opline, new_opcodes + blocks[b->successors[s++]].start);
				break;
			}
		}
	}

	/* adjust exception jump targets & remove unused try_catch_array entries */
	if (op_array->last_try_catch) {
		int i, j;
		uint32_t *map;
		ALLOCA_FLAG(use_heap);

		map = (uint32_t *)do_alloca(sizeof(uint32_t) * op_array->last_try_catch, use_heap);
		for (i = 0, j = 0; i< op_array->last_try_catch; i++) {
			if (blocks[cfg->map[op_array->try_catch_array[i].try_op]].flags & CREX_BB_REACHABLE) {
				map[i] = j;
				op_array->try_catch_array[j].try_op = blocks[cfg->map[op_array->try_catch_array[i].try_op]].start;
				if (op_array->try_catch_array[i].catch_op) {
					op_array->try_catch_array[j].catch_op = blocks[cfg->map[op_array->try_catch_array[i].catch_op]].start;
				} else {
					op_array->try_catch_array[j].catch_op =  0;
				}
				if (op_array->try_catch_array[i].finally_op) {
					op_array->try_catch_array[j].finally_op = blocks[cfg->map[op_array->try_catch_array[i].finally_op]].start;
				} else {
					op_array->try_catch_array[j].finally_op =  0;
				}
				if (!op_array->try_catch_array[i].finally_end) {
					op_array->try_catch_array[j].finally_end = 0;
				} else {
					op_array->try_catch_array[j].finally_end = blocks[cfg->map[op_array->try_catch_array[i].finally_end]].start;
				}
				j++;
			}
		}
		if (i != j) {
			op_array->last_try_catch = j;
			if (j == 0) {
				efree(op_array->try_catch_array);
				op_array->try_catch_array = NULL;
			}

			if (op_array->fn_flags & CREX_ACC_HAS_FINALLY_BLOCK) {
				crex_op *opline = new_opcodes;
				crex_op *end = opline + len;
				while (opline < end) {
					if (opline->opcode == CREX_FAST_RET &&
					    opline->op2.num != (uint32_t)-1 &&
					    opline->op2.num < (uint32_t)j) {
						opline->op2.num = map[opline->op2.num];
					}
					opline++;
				}
			}
		}
		free_alloca(map, use_heap);
	}

	/* rebuild map (just for printing) */
	memset(cfg->map, -1, sizeof(int) * op_array->last);
	for (int n = 0; n < cfg->blocks_count; n++) {
		if (cfg->blocks[n].flags & (CREX_BB_REACHABLE|CREX_BB_UNREACHABLE_FREE)) {
			cfg->map[cfg->blocks[n].start] = n;
		}
	}
}

static crex_always_inline crex_basic_block *get_target_block(const crex_cfg *cfg, crex_basic_block *block, int n, uint32_t *opt_count)
{
	int b;
	crex_basic_block *target_block = cfg->blocks + block->successors[n];

	if (target_block->len == 0 && !(target_block->flags & CREX_BB_PROTECTED)) {
		do {
			b = target_block->successors[0];
			target_block = cfg->blocks + b;
		} while (target_block->len == 0 && !(target_block->flags & CREX_BB_PROTECTED));
		block->successors[n] = b;
		++(*opt_count);
	}
	return target_block;
}

static crex_always_inline crex_basic_block *get_follow_block(const crex_cfg *cfg, crex_basic_block *block, int n, uint32_t *opt_count)
{
	int b;
	crex_basic_block *target_block = cfg->blocks + block->successors[n];

	if (target_block->len == 0 && !(target_block->flags & CREX_BB_PROTECTED)) {
		do {
			b = target_block->successors[0];
			target_block = cfg->blocks + b;
		} while (target_block->len == 0 && !(target_block->flags & CREX_BB_PROTECTED));
		block->successors[n] = b;
		++(*opt_count);
	}
	return target_block;
}

static crex_always_inline crex_basic_block *get_next_block(const crex_cfg *cfg, crex_basic_block *block)
{
	crex_basic_block *next_block = block + 1;
	crex_basic_block *end = cfg->blocks + cfg->blocks_count;

	while (1) {
		if (next_block == end) {
			return NULL;
		} else if (next_block->flags & CREX_BB_REACHABLE) {
			break;
		}
		next_block++;
	}
	while (next_block->len == 0 && !(next_block->flags & CREX_BB_PROTECTED)) {
		next_block = cfg->blocks + next_block->successors[0];
	}
	return next_block;
}


/* we use "jmp_hitlist" to avoid infinity loops during jmp optimization */
static crex_always_inline bool in_hitlist(int target, int *jmp_hitlist, int jmp_hitlist_count)
{
	int i;

	for (i = 0; i < jmp_hitlist_count; i++) {
		if (jmp_hitlist[i] == target) {
			return 1;
		}
	}
	return 0;
}

#define CHECK_LOOP(target) \
	if (EXPECTED(!in_hitlist(target, jmp_hitlist, jmp_hitlist_count))) { \
		jmp_hitlist[jmp_hitlist_count++] = target;	\
	} else { \
		break; \
	}

static void crex_jmp_optimization(crex_basic_block *block, crex_op_array *op_array, const crex_cfg *cfg, int *jmp_hitlist, uint32_t *opt_count)
{
	/* last_op is the last opcode of the current block */
	crex_basic_block *target_block, *follow_block, *next_block;
	crex_op *last_op, *target;
	int next, jmp_hitlist_count;

	if (block->len == 0) {
		return;
	}

	last_op = op_array->opcodes + block->start + block->len - 1;
	switch (last_op->opcode) {
		case CREX_JMP:
			jmp_hitlist_count = 0;

			target_block = get_target_block(cfg, block, 0, opt_count);
			while (target_block->len == 1) {
				target = op_array->opcodes + target_block->start;
				if (target->opcode == CREX_JMP) {
					/* JMP L, L: JMP L1 -> JMP L1 */
					next = target_block->successors[0];
				} else {
					break;
				}
				CHECK_LOOP(next);
				block->successors[0] = next;
				++(*opt_count);
				target_block = get_target_block(cfg, block, 0, opt_count);
			}

			next_block = get_next_block(cfg, block);
			if (target_block == next_block) {
				/* JMP(next) -> NOP */
				MAKE_NOP(last_op);
				++(*opt_count);
				block->len--;
			} else if (target_block->len == 1) {
				target = op_array->opcodes + target_block->start;
				if ((target->opcode == CREX_RETURN ||
				            target->opcode == CREX_RETURN_BY_REF ||
				            target->opcode == CREX_GENERATOR_RETURN ||
				            target->opcode == CREX_EXIT) &&
				           !(op_array->fn_flags & CREX_ACC_HAS_FINALLY_BLOCK)) {
					/* JMP L, L: RETURN to immediate RETURN */
					*last_op = *target;
					if (last_op->op1_type == IS_CONST) {
						zval zv;
						ZVAL_COPY(&zv, &CREX_OP1_LITERAL(last_op));
						last_op->op1.constant = crex_optimizer_add_literal(op_array, &zv);
					}
					block->successors_count = 0;
					++(*opt_count);
				}
			}
			break;

		case CREX_JMP_SET:
		case CREX_COALESCE:
		case CREX_JMP_NULL:
			jmp_hitlist_count = 0;

			target_block = get_target_block(cfg, block, 0, opt_count);
			while (target_block->len == 1) {
				target = op_array->opcodes + target_block->start;

				if (target->opcode == CREX_JMP) {
					/* JMP_SET(X, L), L: JMP(L2) -> JMP_SET(X, L2) */
					next = target_block->successors[0];
					CHECK_LOOP(next);
					block->successors[0] = next;
					++(*opt_count);
				} else {
					break;
				}
				target_block = get_target_block(cfg, block, 0, opt_count);
			}
			break;

		case CREX_JMPZ:
		case CREX_JMPNZ:
			jmp_hitlist_count = 0;

			target_block = get_target_block(cfg, block, 0, opt_count);
			while (target_block->len == 1) {
				target = op_array->opcodes + target_block->start;

				if (target->opcode == CREX_JMP) {
					/* JMPZ(X, L), L: JMP(L2) -> JMPZ(X, L2) */
					next = target_block->successors[0];
				} else if (target->opcode == last_op->opcode &&
				           SAME_VAR(target->op1, last_op->op1)) {
					/* JMPZ(X, L), L: JMPZ(X, L2) -> JMPZ(X, L2) */
					next = target_block->successors[0];
				} else if (target->opcode == INV_COND(last_op->opcode) &&
				           SAME_VAR(target->op1, last_op->op1)) {
					/* JMPZ(X, L), L: JMPNZ(X, L2) -> JMPZ(X, L+1) */
					next = target_block->successors[1];
				} else {
					break;
				}
				CHECK_LOOP(next);
				block->successors[0] = next;
				++(*opt_count);
				target_block = get_target_block(cfg, block, 0, opt_count);
			}

			follow_block = get_follow_block(cfg, block, 1, opt_count);
			if (target_block == follow_block) {
				/* L: JMP[N]Z(X, L+1) -> NOP or FREE(X) */
				crex_optimizer_convert_to_free_op1(op_array, last_op);
				if (last_op->opcode == CREX_NOP) {
					block->len--;
				}
				block->successors_count = 1;
				++(*opt_count);
			} else if (follow_block->len == 1) {
				target = op_array->opcodes + follow_block->start;
				if (target->opcode == CREX_JMP) {
				    if (block->successors[0] == follow_block->successors[0]) {
						/* JMPZ(X,L1), JMP(L1) -> NOP, JMP(L1) */
						crex_optimizer_convert_to_free_op1(op_array, last_op);
						if (last_op->opcode == CREX_NOP) {
							block->len--;
						}
						block->successors[0] = follow_block - cfg->blocks;
						block->successors_count = 1;
						++(*opt_count);
						break;
					} else if (!(follow_block->flags & (CREX_BB_TARGET | CREX_BB_PROTECTED))) {
						next_block = get_next_block(cfg, follow_block);

						if (target_block == next_block) {
							/* JMPZ(X,L1) JMP(L2) L1: -> JMPNZ(X,L2) NOP*/

							last_op->opcode = INV_COND(last_op->opcode);

							block->successors[0] = follow_block->successors[0];
							block->successors[1] = next_block - cfg->blocks;

							follow_block->flags &= ~CREX_BB_REACHABLE;
							MAKE_NOP(target);
							follow_block->len = 0;

							next_block->flags |= CREX_BB_FOLLOW;

							break;
						}
					}
				}
			}
			break;

		case CREX_JMPNC_EX:
		case CREX_JMPC_EX:
			jmp_hitlist_count = 0;

			target_block = get_target_block(cfg, block, 0, opt_count);
			while (target_block->len == 1) {
				target = op_array->opcodes + target_block->start;

				if (target->opcode == CREX_JMP) {
					/* T = JMPC_EX(X, L), L: JMP(L2) -> T = JMPZ(X, L2) */
					next = target_block->successors[0];
				} else if (target->opcode == last_op->opcode-3 &&
				           (SAME_VAR(target->op1, last_op->result) ||
				            SAME_VAR(target->op1, last_op->op1))) {
					/* T = JMPC_EX(X, L1), L1: JMPZ({X|T}, L2) -> T = JMPC_EX(X, L2) */
					next = target_block->successors[0];
				} else if (target->opcode == last_op->opcode &&
				           target->result.var == last_op->result.var &&
				           (SAME_VAR(target->op1, last_op->result) ||
				            SAME_VAR(target->op1, last_op->op1))) {
					/* T = JMPC_EX(X, L1), L1: T = JMPC_EX({X|T}, L2) -> T = JMPC_EX(X, L2) */
					next = target_block->successors[0];
				} else if (target->opcode == INV_EX_COND(last_op->opcode) &&
				           (SAME_VAR(target->op1, last_op->result) ||
				            SAME_VAR(target->op1, last_op->op1))) {
					/* T = JMPC_EX(X, L1), L1: JMPNZ({X|T1}, L2) -> T = JMPC_EX(X, L1+1) */
					next = target_block->successors[1];
				} else if (target->opcode == INV_EX_COND_EX(last_op->opcode) &&
				           target->result.var == last_op->result.var &&
				           (SAME_VAR(target->op1, last_op->result) ||
				            SAME_VAR(target->op1, last_op->op1))) {
					/* T = JMPC_EX(X, L1), L1: T = JMPNC_EX({X|T}, L2) -> T = JMPC_EX(X, L1+1) */
					next = target_block->successors[1];
				} else if (target->opcode == CREX_BOOL &&
				           (SAME_VAR(target->op1, last_op->result) ||
				            SAME_VAR(target->op1, last_op->op1))) {
					/* convert Y = JMPC_EX(X,L1), L1: Z = BOOL(Y) to
					   Z = JMPC_EX(X,L1+1) */

					/* NOTE: This optimization pattern is not safe, but works, */
					/*       because result of JMPC_EX instruction             */
					/*       is not used on the following path and             */
					/*       should be used once on the branch path.           */
					/*                                                         */
					/*       The pattern works well only if jumps processed in */
					/*       direct order, otherwise it breaks JMPC_EX         */
					/*       sequences too early.                              */
					last_op->result.var = target->result.var;
					next = target_block->successors[0];
				} else {
					break;
				}
				CHECK_LOOP(next);
				block->successors[0] = next;
				++(*opt_count);
				target_block = get_target_block(cfg, block, 0, opt_count);
			}

			follow_block = get_follow_block(cfg, block, 1, opt_count);
			if (target_block == follow_block) {
				/* L: T = JMP[N]C_EX(X, L+1) -> T = BOOL(X) */
				last_op->opcode = CREX_BOOL;
				last_op->op2.num = 0;
				block->successors_count = 1;
				++(*opt_count);
				break;
			}
			break;
	}
}

/* Global data dependencies */

/* Find a set of variables which are used outside of the block where they are
 * defined. We won't apply some optimization patterns for such variables. */
static void crex_t_usage(crex_cfg *cfg, crex_op_array *op_array, crex_bitset used_ext, crex_optimizer_ctx *ctx)
{
	int n;
	crex_basic_block *block, *next_block;
	uint32_t var_num;
	uint32_t bitset_len;
	crex_bitset usage;
	crex_bitset defined_here;
	void *checkpoint;
	crex_op *opline, *end;


	if (op_array->T == 0) {
		/* shortcut - if no Ts, nothing to do */
		return;
	}

	checkpoint = crex_arena_checkpoint(ctx->arena);
	bitset_len = crex_bitset_len(op_array->last_var + op_array->T);
	defined_here = crex_arena_alloc(&ctx->arena, bitset_len * CREX_BITSET_ELM_SIZE);

	crex_bitset_clear(defined_here, bitset_len);
	for (n = 1; n < cfg->blocks_count; n++) {
		block = cfg->blocks + n;

		if (!(block->flags & CREX_BB_REACHABLE)) {
			continue;
		}

		opline = op_array->opcodes + block->start;
		end = opline + block->len;
		if (!(block->flags & CREX_BB_FOLLOW) ||
		    (block->flags & CREX_BB_TARGET)) {
			/* Skip continuation of "extended" BB */
			crex_bitset_clear(defined_here, bitset_len);
		}

		while (opline<end) {
			if (opline->op1_type & (IS_VAR|IS_TMP_VAR)) {
				var_num = VAR_NUM(opline->op1.var);
				if (!crex_bitset_in(defined_here, var_num)) {
					crex_bitset_incl(used_ext, var_num);
				}
			}
			if (opline->op2_type == IS_VAR) {
				var_num = VAR_NUM(opline->op2.var);
				if (opline->opcode == CREX_FE_FETCH_R ||
				    opline->opcode == CREX_FE_FETCH_RW) {
					/* these opcode use the op2 as result */
					crex_bitset_incl(defined_here, var_num);
				} else if (!crex_bitset_in(defined_here, var_num)) {
					crex_bitset_incl(used_ext, var_num);
				}
			} else if (opline->op2_type == IS_TMP_VAR) {
				var_num = VAR_NUM(opline->op2.var);
				if (!crex_bitset_in(defined_here, var_num)) {
					crex_bitset_incl(used_ext, var_num);
				}
			}

			if (opline->result_type == IS_VAR) {
				var_num = VAR_NUM(opline->result.var);
				crex_bitset_incl(defined_here, var_num);
			} else if (opline->result_type == IS_TMP_VAR) {
				var_num = VAR_NUM(opline->result.var);
				switch (opline->opcode) {
					case CREX_ADD_ARRAY_ELEMENT:
					case CREX_ADD_ARRAY_UNPACK:
					case CREX_ROPE_ADD:
						/* these opcodes use the result as argument */
						if (!crex_bitset_in(defined_here, var_num)) {
							crex_bitset_incl(used_ext, var_num);
						}
						break;
					default :
						crex_bitset_incl(defined_here, var_num);
				}
			}
			opline++;
		}
	}

	if (ctx->debug_level & CREX_DUMP_BLOCK_PASS_VARS) {
		bool printed = 0;
		uint32_t i;

		for (i = op_array->last_var; i< op_array->T; i++) {
			if (crex_bitset_in(used_ext, i)) {
				if (!printed) {
					fprintf(stderr, "NON-LOCAL-VARS: %d", i);
					printed = 1;
				} else {
					fprintf(stderr, ", %d", i);
				}
			}
		}
		if (printed) {
			fprintf(stderr, "\n");
		}
	}

	usage = defined_here;
	next_block = NULL;
	for (n = cfg->blocks_count; n > 0;) {
		block = cfg->blocks + (--n);

		if (!(block->flags & CREX_BB_REACHABLE) || block->len == 0) {
			continue;
		}

		end = op_array->opcodes + block->start;
		opline = end + block->len - 1;
		if (!next_block ||
		    !(next_block->flags & CREX_BB_FOLLOW) ||
		    (next_block->flags & CREX_BB_TARGET)) {
			/* Skip continuation of "extended" BB */
			crex_bitset_copy(usage, used_ext, bitset_len);
		} else if (block->successors_count > 1) {
			crex_bitset_union(usage, used_ext, bitset_len);
		}
		next_block = block;

		while (opline >= end) {
			/* usage checks */
			if (opline->result_type & (IS_VAR|IS_TMP_VAR)) {
				if (!crex_bitset_in(usage, VAR_NUM(opline->result.var))) {
					switch (opline->opcode) {
						case CREX_ASSIGN_OP:
						case CREX_ASSIGN_DIM_OP:
						case CREX_ASSIGN_OBJ_OP:
						case CREX_ASSIGN_STATIC_PROP_OP:
						case CREX_PRE_INC:
						case CREX_PRE_DEC:
						case CREX_ASSIGN:
						case CREX_ASSIGN_REF:
						case CREX_DO_FCALL:
						case CREX_DO_ICALL:
						case CREX_DO_UCALL:
						case CREX_DO_FCALL_BY_NAME:
							opline->result_type = IS_UNUSED;
							break;
						case CREX_POST_INC:
						case CREX_POST_DEC:
						case CREX_POST_INC_OBJ:
						case CREX_POST_DEC_OBJ:
						case CREX_POST_INC_STATIC_PROP:
						case CREX_POST_DEC_STATIC_PROP:
							opline->opcode -= 2;
							opline->result_type = IS_UNUSED;
							break;
						case CREX_QM_ASSIGN:
						case CREX_BOOL:
						case CREX_BOOL_NOT:
							crex_optimizer_convert_to_free_op1(op_array, opline);
							break;
						case CREX_JMPC_EX:
						case CREX_JMPNC_EX:
							opline->opcode -= 3;
							SET_UNUSED(opline->result);
							break;
						case CREX_ADD_ARRAY_ELEMENT:
						case CREX_ADD_ARRAY_UNPACK:
						case CREX_ROPE_ADD:
							crex_bitset_incl(usage, VAR_NUM(opline->result.var));
							break;
					}
				} else {
					switch (opline->opcode) {
						case CREX_ADD_ARRAY_ELEMENT:
						case CREX_ADD_ARRAY_UNPACK:
						case CREX_ROPE_ADD:
							break;
						default:
							crex_bitset_excl(usage, VAR_NUM(opline->result.var));
							break;
					}
				}
			}

			if (opline->op2_type == IS_VAR) {
				switch (opline->opcode) {
					case CREX_FE_FETCH_R:
					case CREX_FE_FETCH_RW:
						crex_bitset_excl(usage, VAR_NUM(opline->op2.var));
						break;
					default:
						crex_bitset_incl(usage, VAR_NUM(opline->op2.var));
						break;
				}
			} else if (opline->op2_type == IS_TMP_VAR) {
				crex_bitset_incl(usage, VAR_NUM(opline->op2.var));
			}

			if (opline->op1_type & (IS_VAR|IS_TMP_VAR)) {
				crex_bitset_incl(usage, VAR_NUM(opline->op1.var));
			}

			opline--;
		}
	}

	crex_arena_release(&ctx->arena, checkpoint);
}

static void crex_merge_blocks(crex_op_array *op_array, crex_cfg *cfg, uint32_t *opt_count)
{
	int i;
	crex_basic_block *b, *bb;
	crex_basic_block *prev = NULL;

	for (i = 0; i < cfg->blocks_count; i++) {
		b = cfg->blocks + i;
		if (b->flags & CREX_BB_REACHABLE) {
			if ((b->flags & CREX_BB_FOLLOW) &&
			    !(b->flags & (CREX_BB_TARGET | CREX_BB_PROTECTED)) &&
			    prev && prev->successors_count == 1 && prev->successors[0] == i)
			{
				crex_op *last_op = op_array->opcodes + prev->start + prev->len - 1;
				if (prev->len != 0 && last_op->opcode == CREX_JMP) {
					MAKE_NOP(last_op);
				}

				for (bb = prev + 1; bb != b; bb++) {
					crex_op *op = op_array->opcodes + bb->start;
					crex_op *end = op + bb->len;
					while (op < end) {
						if (op->op1_type == IS_CONST) {
							literal_dtor(&CREX_OP1_LITERAL(op));
						}
						if (op->op2_type == IS_CONST) {
							literal_dtor(&CREX_OP2_LITERAL(op));
						}
						MAKE_NOP(op);
						op++;
					}
					/* make block empty */
					bb->len = 0;
				}

				/* re-link */
				prev->flags |= (b->flags & CREX_BB_EXIT);
				prev->len = b->start + b->len - prev->start;
				prev->successors_count = b->successors_count;
				if (b->successors != b->successors_storage) {
					prev->successors = b->successors;
					b->successors = b->successors_storage;
				} else {
					memcpy(prev->successors, b->successors, b->successors_count * sizeof(int));
				}

				/* unlink & make block empty and unreachable */
				b->flags = 0;
				b->len = 0;
				b->successors_count = 0;
				++(*opt_count);
			} else {
				prev = b;
			}
		}
	}
}

#define PASSES 3

void crex_optimize_cfg(crex_op_array *op_array, crex_optimizer_ctx *ctx)
{
	crex_cfg cfg;
	crex_basic_block *blocks, *end, *b;
	int pass;
	uint32_t bitset_len;
	crex_bitset usage;
	void *checkpoint;
	crex_op **Tsource;
	uint32_t opt_count;
	int *jmp_hitlist;

    /* Build CFG */
	checkpoint = crex_arena_checkpoint(ctx->arena);
	crex_build_cfg(&ctx->arena, op_array, 0, &cfg);

	if (cfg.blocks_count * (op_array->last_var + op_array->T) > 64 * 1024 * 1024) {
		crex_arena_release(&ctx->arena, checkpoint);
		return;
	}

	if (ctx->debug_level & CREX_DUMP_BEFORE_BLOCK_PASS) {
		crex_dump_op_array(op_array, CREX_DUMP_CFG, "before block pass", &cfg);
	}

	bitset_len = crex_bitset_len(op_array->last_var + op_array->T);
	Tsource = crex_arena_calloc(&ctx->arena, op_array->last_var + op_array->T, sizeof(crex_op *));
	usage = crex_arena_alloc(&ctx->arena, bitset_len * CREX_BITSET_ELM_SIZE);
	jmp_hitlist = crex_arena_alloc(&ctx->arena, cfg.blocks_count * sizeof(int));

	blocks = cfg.blocks;
	end = blocks + cfg.blocks_count;
	for (pass = 0; pass < PASSES; pass++) {
		opt_count = 0;

		/* Compute data dependencies */
		crex_bitset_clear(usage, bitset_len);
		crex_t_usage(&cfg, op_array, usage, ctx);

		/* optimize each basic block separately */
		for (b = blocks; b < end; b++) {
			if (!(b->flags & CREX_BB_REACHABLE)) {
				continue;
			}
			/* we track data dependencies only inside a single basic block */
			if (!(b->flags & CREX_BB_FOLLOW) ||
			    (b->flags & CREX_BB_TARGET)) {
				/* Skip continuation of "extended" BB */
				memset(Tsource, 0, (op_array->last_var + op_array->T) * sizeof(crex_op *));
			}
			crex_optimize_block(b, op_array, usage, &cfg, Tsource, &opt_count);
		}

		/* Eliminate NOPs */
		for (b = blocks; b < end; b++) {
			if (b->flags & CREX_BB_UNREACHABLE_FREE) {
				/* In unreachable_free blocks only preserve loop var frees. */
				for (uint32_t i = b->start; i < b->start + b->len; i++) {
					crex_op *opline = &op_array->opcodes[i];
					if (!crex_optimizer_is_loop_var_free(opline)) {
						MAKE_NOP(opline);
					}
				}
			}
			if (b->flags & (CREX_BB_REACHABLE|CREX_BB_UNREACHABLE_FREE)) {
				strip_nops(op_array, b);
			}
		}

		opt_count = 0;

		/* Jump optimization for each block */
		for (b = blocks; b < end; b++) {
			if (b->flags & CREX_BB_REACHABLE) {
				crex_jmp_optimization(b, op_array, &cfg, jmp_hitlist, &opt_count);
			}
		}

		/* Eliminate unreachable basic blocks */
		crex_cfg_remark_reachable_blocks(op_array, &cfg);

		/* Merge Blocks */
		crex_merge_blocks(op_array, &cfg, &opt_count);

		if (opt_count == 0) {
			break;
		}
	}

	assemble_code_blocks(&cfg, op_array, ctx);

	if (ctx->debug_level & CREX_DUMP_AFTER_BLOCK_PASS) {
		crex_dump_op_array(op_array, CREX_DUMP_CFG | CREX_DUMP_HIDE_UNREACHABLE, "after block pass", &cfg);
	}

	/* Destroy CFG */
	crex_arena_release(&ctx->arena, checkpoint);
}
