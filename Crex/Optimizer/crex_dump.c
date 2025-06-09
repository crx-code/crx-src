/*
   +----------------------------------------------------------------------+
   | Crex Engine, Bytecode Visualisation                                  |
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
   | Authors: Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#include "crex_compile.h"
#include "crex_cfg.h"
#include "crex_ssa.h"
#include "crex_inference.h"
#include "crex_func_info.h"
#include "crex_call_graph.h"
#include "crex_dump.h"
#include "ext/standard/crx_string.h"

void crex_dump_ht(HashTable *ht)
{
	crex_ulong index;
	crex_string *key;
	zval *val;
	bool first = 1;

	CREX_HASH_FOREACH_KEY_VAL(ht, index, key, val) {
		if (first) {
			first = 0;
		} else {
			fprintf(stderr, ", ");
		}
		if (key) {
			fprintf(stderr, "\"%s\"", ZSTR_VAL(key));
		} else {
			fprintf(stderr, CREX_LONG_FMT, index);
		}
		fprintf(stderr, " =>");
		crex_dump_const(val);
	} CREX_HASH_FOREACH_END();
}

void crex_dump_const(const zval *zv)
{
	switch (C_TYPE_P(zv)) {
		case IS_NULL:
			fprintf(stderr, " null");
			break;
		case IS_FALSE:
			fprintf(stderr, " bool(false)");
			break;
		case IS_TRUE:
			fprintf(stderr, " bool(true)");
			break;
		case IS_LONG:
			fprintf(stderr, " int(" CREX_LONG_FMT ")", C_LVAL_P(zv));
			break;
		case IS_DOUBLE:
			fprintf(stderr, " float(%g)", C_DVAL_P(zv));
			break;
		case IS_STRING:;
			crex_string *escaped_string = crx_addcslashes(C_STR_P(zv), "\"\\", 2);

			fprintf(stderr, " string(\"%s\")", ZSTR_VAL(escaped_string));

			crex_string_release(escaped_string);
			break;
		case IS_ARRAY:
			fprintf(stderr, " array(...)");
			break;
		default:
			fprintf(stderr, " zval(type=%d)", C_TYPE_P(zv));
			break;
	}
}

static void crex_dump_class_fetch_type(uint32_t fetch_type)
{
	switch (fetch_type & CREX_FETCH_CLASS_MASK) {
		case CREX_FETCH_CLASS_SELF:
			fprintf(stderr, " (self)");
			break;
		case CREX_FETCH_CLASS_PARENT:
			fprintf(stderr, " (parent)");
			break;
		case CREX_FETCH_CLASS_STATIC:
			fprintf(stderr, " (static)");
			break;
		case CREX_FETCH_CLASS_AUTO:
			fprintf(stderr, " (auto)");
			break;
		case CREX_FETCH_CLASS_INTERFACE:
			fprintf(stderr, " (interface)");
			break;
		case CREX_FETCH_CLASS_TRAIT:
			fprintf(stderr, " (trait)");
			break;
	}
	if (fetch_type & CREX_FETCH_CLASS_NO_AUTOLOAD) {
			fprintf(stderr, " (no-autoload)");
	}
	if (fetch_type & CREX_FETCH_CLASS_SILENT) {
			fprintf(stderr, " (silent)");
	}
	if (fetch_type & CREX_FETCH_CLASS_EXCEPTION) {
			fprintf(stderr, " (exception)");
	}
}

static void crex_dump_unused_op(const crex_op *opline, znode_op op, uint32_t flags) {
	if (CREX_VM_OP_NUM == (flags & CREX_VM_OP_MASK)) {
		fprintf(stderr, " %u", op.num);
	} else if (CREX_VM_OP_TRY_CATCH == (flags & CREX_VM_OP_MASK)) {
		if (op.num != (uint32_t)-1) {
			fprintf(stderr, " try-catch(%u)", op.num);
		}
	} else if (CREX_VM_OP_THIS == (flags & CREX_VM_OP_MASK)) {
		fprintf(stderr, " THIS");
	} else if (CREX_VM_OP_NEXT == (flags & CREX_VM_OP_MASK)) {
		fprintf(stderr, " NEXT");
	} else if (CREX_VM_OP_CLASS_FETCH == (flags & CREX_VM_OP_MASK)) {
		crex_dump_class_fetch_type(op.num);
	} else if (CREX_VM_OP_CONSTRUCTOR == (flags & CREX_VM_OP_MASK)) {
		fprintf(stderr, " CONSTRUCTOR");
	} else if (CREX_VM_OP_CONST_FETCH == (flags & CREX_VM_OP_MASK)) {
		if (op.num & IS_CONSTANT_UNQUALIFIED_IN_NAMESPACE) {
			fprintf(stderr, " (unqualified-in-namespace)");
		}
	}
}

CREX_API void crex_dump_var(const crex_op_array *op_array, uint8_t var_type, int var_num)
{
	if (var_type == IS_CV && var_num < op_array->last_var) {
		fprintf(stderr, "CV%d($%s)", var_num, op_array->vars[var_num]->val);
	} else if (var_type == IS_VAR) {
		fprintf(stderr, "V%d", var_num);
	} else if ((var_type & (IS_VAR|IS_TMP_VAR)) == IS_TMP_VAR) {
		fprintf(stderr, "T%d", var_num);
	} else {
		fprintf(stderr, "X%d", var_num);
	}
}

static void crex_dump_range(const crex_ssa_range *r)
{
	if (r->underflow && r->overflow) {
		return;
	}
	fprintf(stderr, " RANGE[");
	if (r->underflow) {
		fprintf(stderr, "--..");
	} else if (r->min == CREX_LONG_MIN) {
		fprintf(stderr, "MIN..");
	} else {
		fprintf(stderr, CREX_LONG_FMT "..", r->min);
	}
	if (r->overflow) {
		fprintf(stderr, "++]");
	} else if (r->max == CREX_LONG_MAX) {
		fprintf(stderr, "MAX]");
	} else {
		fprintf(stderr, CREX_LONG_FMT "]", r->max);
	}
}

static void crex_dump_type_info(uint32_t info, crex_class_entry *ce, int is_instanceof, uint32_t dump_flags)
{
	bool first = 1;

	fprintf(stderr, " [");
	if (info & MAY_BE_GUARD) {
		fprintf(stderr, "!");
	}
	if (info & MAY_BE_UNDEF) {
		if (first) first = 0; else fprintf(stderr, ", ");
		fprintf(stderr, "undef");
	}
	if (info & MAY_BE_INDIRECT) {
		if (first) first = 0; else fprintf(stderr, ", ");
		fprintf(stderr, "ind");
	}
	if (info & MAY_BE_REF) {
		if (first) first = 0; else fprintf(stderr, ", ");
		fprintf(stderr, "ref");
	}
	if (dump_flags & CREX_DUMP_RC_INFERENCE) {
		if (info & MAY_BE_RC1) {
			if (first) first = 0; else fprintf(stderr, ", ");
			fprintf(stderr, "rc1");
		}
		if (info & MAY_BE_RCN) {
			if (first) first = 0; else fprintf(stderr, ", ");
			fprintf(stderr, "rcn");
		}
	}
	if (info & MAY_BE_CLASS) {
		if (first) first = 0; else fprintf(stderr, ", ");
		fprintf(stderr, "class");
		if (ce) {
			if (is_instanceof) {
				fprintf(stderr, " (instanceof %s)", ce->name->val);
			} else {
				fprintf(stderr, " (%s)", ce->name->val);
			}
		}
	} else if ((info & MAY_BE_ANY) == MAY_BE_ANY) {
		if (first) first = 0; else fprintf(stderr, ", ");
		fprintf(stderr, "any");
	} else {
		if (info & MAY_BE_NULL) {
			if (first) first = 0; else fprintf(stderr, ", ");
			fprintf(stderr, "null");
		}
		if ((info & MAY_BE_FALSE) && (info & MAY_BE_TRUE)) {
			if (first) first = 0; else fprintf(stderr, ", ");
			fprintf(stderr, "bool");
		} else if (info & MAY_BE_FALSE) {
			if (first) first = 0; else fprintf(stderr, ", ");
			fprintf(stderr, "false");
		} else if (info & MAY_BE_TRUE) {
			if (first) first = 0; else fprintf(stderr, ", ");
			fprintf(stderr, "true");
		}
		if (info & MAY_BE_LONG) {
			if (first) first = 0; else fprintf(stderr, ", ");
			fprintf(stderr, "long");
		}
		if (info & MAY_BE_DOUBLE) {
			if (first) first = 0; else fprintf(stderr, ", ");
			fprintf(stderr, "double");
		}
		if (info & MAY_BE_STRING) {
			if (first) first = 0; else fprintf(stderr, ", ");
			fprintf(stderr, "string");
		}
		if (info & MAY_BE_ARRAY) {
			if (first) first = 0; else fprintf(stderr, ", ");
			if (info & MAY_BE_PACKED_GUARD) {
				fprintf(stderr, "!");
			}
			if (MAY_BE_EMPTY_ONLY(info)) {
				fprintf(stderr, "empty ");
			} else if (MAY_BE_PACKED_ONLY(info)) {
				fprintf(stderr, "packed ");
			} else if (MAY_BE_HASH_ONLY(info)) {
				fprintf(stderr, "hash ");
			} else if ((info & MAY_BE_ARRAY_KEY_ANY) != MAY_BE_ARRAY_KEY_ANY && (info & MAY_BE_ARRAY_KEY_ANY) != 0) {
				bool afirst = 1;
				fprintf(stderr, "[");
				if (info & MAY_BE_ARRAY_EMPTY) {
					if (afirst) afirst = 0; else fprintf(stderr, ", ");
					fprintf(stderr, "empty");
				}
				if (MAY_BE_PACKED(info)) {
					if (afirst) afirst = 0; else fprintf(stderr, ", ");
					fprintf(stderr, "packed");
				}
				if (MAY_BE_HASH(info)) {
					if (afirst) afirst = 0; else fprintf(stderr, ", ");
					fprintf(stderr, "hash");
				}
				fprintf(stderr, "] ");
			}
			fprintf(stderr, "array");
			if ((info & (MAY_BE_ARRAY_KEY_LONG|MAY_BE_ARRAY_KEY_STRING)) != 0 &&
			    ((info & MAY_BE_ARRAY_KEY_LONG) == 0 ||
			     (info & MAY_BE_ARRAY_KEY_STRING) == 0)) {
				bool afirst = 1;
				fprintf(stderr, " [");
				if (info & MAY_BE_ARRAY_KEY_LONG) {
					if (afirst) afirst = 0; else fprintf(stderr, ", ");
					fprintf(stderr, "long");
				}
				if (info & MAY_BE_ARRAY_KEY_STRING) {
					if (afirst) afirst = 0; else fprintf(stderr, ", ");
						fprintf(stderr, "string");
					}
				fprintf(stderr, "]");
			}
			if (info & (MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF)) {
				bool afirst = 1;
				fprintf(stderr, " of [");
				if ((info & MAY_BE_ARRAY_OF_ANY) == MAY_BE_ARRAY_OF_ANY) {
					if (afirst) afirst = 0; else fprintf(stderr, ", ");
					fprintf(stderr, "any");
				} else {
					if (info & MAY_BE_ARRAY_OF_NULL) {
						if (afirst) afirst = 0; else fprintf(stderr, ", ");
						fprintf(stderr, "null");
					}
					if (info & MAY_BE_ARRAY_OF_FALSE) {
						if (afirst) afirst = 0; else fprintf(stderr, ", ");
						fprintf(stderr, "false");
					}
					if (info & MAY_BE_ARRAY_OF_TRUE) {
						if (afirst) afirst = 0; else fprintf(stderr, ", ");
						fprintf(stderr, "true");
					}
					if (info & MAY_BE_ARRAY_OF_LONG) {
						if (afirst) afirst = 0; else fprintf(stderr, ", ");
						fprintf(stderr, "long");
					}
					if (info & MAY_BE_ARRAY_OF_DOUBLE) {
						if (afirst) afirst = 0; else fprintf(stderr, ", ");
						fprintf(stderr, "double");
					}
					if (info & MAY_BE_ARRAY_OF_STRING) {
						if (afirst) afirst = 0; else fprintf(stderr, ", ");
						fprintf(stderr, "string");
					}
					if (info & MAY_BE_ARRAY_OF_ARRAY) {
						if (afirst) afirst = 0; else fprintf(stderr, ", ");
						fprintf(stderr, "array");
					}
					if (info & MAY_BE_ARRAY_OF_OBJECT) {
						if (afirst) afirst = 0; else fprintf(stderr, ", ");
						fprintf(stderr, "object");
					}
					if (info & MAY_BE_ARRAY_OF_RESOURCE) {
						if (afirst) afirst = 0; else fprintf(stderr, ", ");
						fprintf(stderr, "resource");
					}
				}
				if (info & MAY_BE_ARRAY_OF_REF) {
					if (afirst) afirst = 0; else fprintf(stderr, ", ");
					fprintf(stderr, "ref");
				}
				fprintf(stderr, "]");
			}
		}
		if (info & MAY_BE_OBJECT) {
			if (first) first = 0; else fprintf(stderr, ", ");
			fprintf(stderr, "object");
			if (ce) {
				if (is_instanceof) {
					fprintf(stderr, " (instanceof %s)", ce->name->val);
				} else {
					fprintf(stderr, " (%s)", ce->name->val);
				}
			}
		}
		if (info & MAY_BE_RESOURCE) {
			if (first) first = 0; else fprintf(stderr, ", ");
			fprintf(stderr, "resource");
		}
	}
	fprintf(stderr, "]");
}

static void crex_dump_ssa_var_info(const crex_ssa *ssa, int ssa_var_num, uint32_t dump_flags)
{
	crex_dump_type_info(
		ssa->var_info[ssa_var_num].type,
		ssa->var_info[ssa_var_num].ce,
		ssa->var_info[ssa_var_num].ce ?
			ssa->var_info[ssa_var_num].is_instanceof : 0,
		dump_flags);
}

CREX_API void crex_dump_ssa_var(const crex_op_array *op_array, const crex_ssa *ssa, int ssa_var_num, uint8_t var_type, int var_num, uint32_t dump_flags)
{
	if (ssa_var_num >= 0) {
		fprintf(stderr, "#%d.", ssa_var_num);
	} else {
		fprintf(stderr, "#?.");
	}
	crex_dump_var(op_array, (var_num < op_array->last_var ? IS_CV : var_type), var_num);

	if (ssa_var_num >= 0 && ssa->vars) {
		if (ssa->vars[ssa_var_num].no_val) {
			fprintf(stderr, " NOVAL");
		}
		if (ssa->vars[ssa_var_num].escape_state == ESCAPE_STATE_NO_ESCAPE) {
			fprintf(stderr, " NOESC");
		}
		if (ssa->var_info) {
			crex_dump_ssa_var_info(ssa, ssa_var_num, dump_flags);
			if (ssa->var_info[ssa_var_num].has_range) {
				crex_dump_range(&ssa->var_info[ssa_var_num].range);
			}
		}
	}
}

static void crex_dump_type_constraint(const crex_op_array *op_array, const crex_ssa *ssa, const crex_ssa_type_constraint *constraint, uint32_t dump_flags)
{
	fprintf(stderr, " TYPE");
	crex_dump_type_info(constraint->type_mask, constraint->ce, 1, dump_flags);
}

static void crex_dump_range_constraint(const crex_op_array *op_array, const crex_ssa *ssa, const crex_ssa_range_constraint *r, uint32_t dump_flags)
{
	if (r->range.underflow && r->range.overflow) {
		return;
	}
	fprintf(stderr, " RANGE");
	if (r->negative) {
		fprintf(stderr, "~");
	}
	fprintf(stderr, "[");
	if (r->range.underflow) {
		fprintf(stderr, "-- .. ");
	} else {
		if (r->min_ssa_var >= 0) {
			crex_dump_ssa_var(op_array, ssa, r->min_ssa_var, (r->min_var < op_array->last_var ? IS_CV : 0), r->min_var, dump_flags);
			if (r->range.min > 0) {
				fprintf(stderr, " + " CREX_LONG_FMT, r->range.min);
			} else if (r->range.min < 0) {
				fprintf(stderr, " - " CREX_LONG_FMT, -r->range.min);
			}
			fprintf(stderr, " .. ");
		} else {
			fprintf(stderr, CREX_LONG_FMT " .. ", r->range.min);
		}
	}
	if (r->range.overflow) {
		fprintf(stderr, "++]");
	} else {
		if (r->max_ssa_var >= 0) {
			crex_dump_ssa_var(op_array, ssa, r->max_ssa_var, (r->max_var < op_array->last_var ? IS_CV : 0), r->max_var, dump_flags);
			if (r->range.max > 0) {
				fprintf(stderr, " + " CREX_LONG_FMT, r->range.max);
			} else if (r->range.max < 0) {
				fprintf(stderr, " - " CREX_LONG_FMT, -r->range.max);
			}
			fprintf(stderr, "]");
		} else {
			fprintf(stderr, CREX_LONG_FMT "]", r->range.max);
		}
	}
}

CREX_API void crex_dump_op(const crex_op_array *op_array, const crex_basic_block *b, const crex_op *opline, uint32_t dump_flags, const crex_ssa *ssa, const crex_ssa_op *ssa_op)
{
	const char *name = crex_get_opcode_name(opline->opcode);
	uint32_t flags = crex_get_opcode_flags(opline->opcode);
	uint32_t n = 0;

	if (!ssa_op || ssa_op->result_use < 0) {
		if (opline->result_type & (IS_CV|IS_VAR|IS_TMP_VAR)) {
			if (ssa_op && ssa_op->result_def >= 0) {
				int ssa_var_num = ssa_op->result_def;
				crex_dump_ssa_var(op_array, ssa, ssa_var_num, opline->result_type, EX_VAR_TO_NUM(opline->result.var), dump_flags);
			} else {
				crex_dump_var(op_array, opline->result_type, EX_VAR_TO_NUM(opline->result.var));
			}
			fprintf(stderr, " = ");
		}
	}

	if (name) {
		fprintf(stderr, "%s", (name + 5));
	} else {
		fprintf(stderr, "OP_%d", (int)opline->opcode);
	}

	if (CREX_VM_EXT_NUM == (flags & CREX_VM_EXT_MASK)) {
		fprintf(stderr, " %u", opline->extended_value);
	} else if (CREX_VM_EXT_OP == (flags & CREX_VM_EXT_MASK)) {
		fprintf(stderr, " (%s)", crex_get_opcode_name(opline->extended_value) + 5);
	} else if (CREX_VM_EXT_TYPE == (flags & CREX_VM_EXT_MASK)) {
		switch (opline->extended_value) {
			case IS_NULL:
				fprintf(stderr, " (null)");
				break;
			case IS_FALSE:
				fprintf(stderr, " (false)");
				break;
			case IS_TRUE:
				fprintf(stderr, " (true)");
				break;
			case IS_LONG:
				fprintf(stderr, " (long)");
				break;
			case IS_DOUBLE:
				fprintf(stderr, " (double)");
				break;
			case IS_STRING:
				fprintf(stderr, " (string)");
				break;
			case IS_ARRAY:
				fprintf(stderr, " (array)");
				break;
			case IS_OBJECT:
				fprintf(stderr, " (object)");
				break;
			case IS_RESOURCE:
				fprintf(stderr, " (resource)");
				break;
			case _IS_BOOL:
				fprintf(stderr, " (bool)");
				break;
			case IS_CALLABLE:
				fprintf(stderr, " (callable)");
				break;
			case IS_VOID:
				fprintf(stderr, " (void)");
				break;
			case IS_NEVER:
				fprintf(stderr, " (never)");
				break;
			default:
				fprintf(stderr, " (\?\?\?)");
				break;
		}
	} else if (CREX_VM_EXT_TYPE_MASK == (flags & CREX_VM_EXT_MASK)) {
		switch (opline->extended_value) {
			case (1<<IS_NULL):
				fprintf(stderr, " (null)");
				break;
			case (1<<IS_FALSE):
				fprintf(stderr, " (false)");
				break;
			case (1<<IS_TRUE):
				fprintf(stderr, " (true)");
				break;
			case (1<<IS_LONG):
				fprintf(stderr, " (long)");
				break;
			case (1<<IS_DOUBLE):
				fprintf(stderr, " (double)");
				break;
			case (1<<IS_STRING):
				fprintf(stderr, " (string)");
				break;
			case (1<<IS_ARRAY):
				fprintf(stderr, " (array)");
				break;
			case (1<<IS_OBJECT):
				fprintf(stderr, " (object)");
				break;
			case (1<<IS_RESOURCE):
				fprintf(stderr, " (resource)");
				break;
			case ((1<<IS_FALSE)|(1<<IS_TRUE)):
				fprintf(stderr, " (bool)");
				break;
			default:
				fprintf(stderr, " TYPE");
				crex_dump_type_info(opline->extended_value, NULL, 0, dump_flags);
				break;
		}
	} else if (CREX_VM_EXT_EVAL == (flags & CREX_VM_EXT_MASK)) {
		switch (opline->extended_value) {
			case CREX_EVAL:
				fprintf(stderr, " (eval)");
				break;
			case CREX_INCLUDE:
				fprintf(stderr, " (include)");
				break;
			case CREX_INCLUDE_ONCE:
				fprintf(stderr, " (include_once)");
				break;
			case CREX_REQUIRE:
				fprintf(stderr, " (require)");
				break;
			case CREX_REQUIRE_ONCE:
				fprintf(stderr, " (require_once)");
				break;
			default:
				fprintf(stderr, " (\?\?\?)");
				break;
		}
	} else if (CREX_VM_EXT_SRC == (flags & CREX_VM_EXT_MASK)) {
		if (opline->extended_value == CREX_RETURNS_VALUE) {
			fprintf(stderr, " (value)");
		} else if (opline->extended_value & CREX_RETURNS_FUNCTION) {
			fprintf(stderr, " (function)");
		}
	} else {
		if (CREX_VM_EXT_VAR_FETCH & flags) {
			if (opline->extended_value & CREX_FETCH_GLOBAL) {
				fprintf(stderr, " (global)");
			} else if (opline->extended_value & CREX_FETCH_LOCAL) {
				fprintf(stderr, " (local)");
			} else if (opline->extended_value & CREX_FETCH_GLOBAL_LOCK) {
				fprintf(stderr, " (global+lock)");
			}
		}
		if (CREX_VM_EXT_ISSET & flags) {
			if (!(opline->extended_value & CREX_ISEMPTY)) {
				fprintf(stderr, " (isset)");
			} else {
				fprintf(stderr, " (empty)");
			}
		}
		if (CREX_VM_EXT_ARRAY_INIT & flags) {
			fprintf(stderr, " %u", opline->extended_value >> CREX_ARRAY_SIZE_SHIFT);
			if (!(opline->extended_value & CREX_ARRAY_NOT_PACKED)) {
				fprintf(stderr, " (packed)");
			}
		}
		if (CREX_VM_EXT_REF & flags) {
			if (opline->extended_value & CREX_ARRAY_ELEMENT_REF) {
				fprintf(stderr, " (ref)");
			}
		}
		if ((CREX_VM_EXT_DIM_WRITE|CREX_VM_EXT_FETCH_REF) & flags) {
			uint32_t obj_flags = opline->extended_value & CREX_FETCH_OBJ_FLAGS;
			if (obj_flags == CREX_FETCH_REF) {
				fprintf(stderr, " (ref)");
			} else if (obj_flags == CREX_FETCH_DIM_WRITE) {
				fprintf(stderr, " (dim write)");
			}
		}
	}

	if (opline->op1_type == IS_CONST) {
		crex_dump_const(CRT_CONSTANT(opline->op1));
	} else if (opline->op1_type & (IS_CV|IS_VAR|IS_TMP_VAR)) {
		if (ssa_op) {
			int ssa_var_num = ssa_op->op1_use;
			if (ssa_var_num >= 0) {
				fprintf(stderr, " ");
				crex_dump_ssa_var(op_array, ssa, ssa_var_num, opline->op1_type, EX_VAR_TO_NUM(opline->op1.var), dump_flags);
			} else if (ssa_op->op1_def < 0) {
				fprintf(stderr, " ");
				crex_dump_var(op_array, opline->op1_type, EX_VAR_TO_NUM(opline->op1.var));
			}
		} else {
			fprintf(stderr, " ");
			crex_dump_var(op_array, opline->op1_type, EX_VAR_TO_NUM(opline->op1.var));
		}
		if (ssa_op) {
			int ssa_var_num = ssa_op->op1_def;
			if (ssa_var_num >= 0) {
				fprintf(stderr, " -> ");
				crex_dump_ssa_var(op_array, ssa, ssa_var_num, opline->op1_type, EX_VAR_TO_NUM(opline->op1.var), dump_flags);
			}
		}
	} else {
		uint32_t op1_flags = CREX_VM_OP1_FLAGS(flags);
		if (CREX_VM_OP_JMP_ADDR == (op1_flags & CREX_VM_OP_MASK)) {
			if (b) {
				fprintf(stderr, " BB%d", b->successors[n++]);
			} else {
				fprintf(stderr, " %04u", (uint32_t)(OP_JMP_ADDR(opline, opline->op1) - op_array->opcodes));
			}
		} else {
			crex_dump_unused_op(opline, opline->op1, op1_flags);
		}
	}

	if (opline->op2_type == IS_CONST) {
		zval *op = CRT_CONSTANT(opline->op2);
		if (
			opline->opcode == CREX_SWITCH_LONG
			|| opline->opcode == CREX_SWITCH_STRING
			|| opline->opcode == CREX_MATCH
		) {
			HashTable *jumptable = C_ARRVAL_P(op);
			crex_string *key;
			crex_ulong num_key;
			zval *zv;
			CREX_HASH_FOREACH_KEY_VAL(jumptable, num_key, key, zv) {
				if (key) {
					fprintf(stderr, " \"%s\":", ZSTR_VAL(key));
				} else {
					fprintf(stderr, " " CREX_LONG_FMT ":", num_key);
				}
				if (b) {
					fprintf(stderr, " BB%d,", b->successors[n++]);
				} else {
					fprintf(stderr, " %04u,", (uint32_t)CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, C_LVAL_P(zv)));
				}
			} CREX_HASH_FOREACH_END();
			fprintf(stderr, " default:");
		} else {
			crex_dump_const(op);
		}
	} else if (opline->op2_type & (IS_CV|IS_VAR|IS_TMP_VAR)) {
		if (ssa_op) {
			int ssa_var_num = ssa_op->op2_use;
			if (ssa_var_num >= 0) {
				fprintf(stderr, " ");
				crex_dump_ssa_var(op_array, ssa, ssa_var_num, opline->op2_type, EX_VAR_TO_NUM(opline->op2.var), dump_flags);
			} else if (ssa_op->op2_def < 0) {
				fprintf(stderr, " ");
				crex_dump_var(op_array, opline->op2_type, EX_VAR_TO_NUM(opline->op2.var));
			}
		} else {
			fprintf(stderr, " ");
			crex_dump_var(op_array, opline->op2_type, EX_VAR_TO_NUM(opline->op2.var));
		}
		if (ssa_op) {
			int ssa_var_num = ssa_op->op2_def;
			if (ssa_var_num >= 0) {
				fprintf(stderr, " -> ");
				crex_dump_ssa_var(op_array, ssa, ssa_var_num, opline->op2_type, EX_VAR_TO_NUM(opline->op2.var), dump_flags);
			}
		}
	} else {
		uint32_t op2_flags = CREX_VM_OP2_FLAGS(flags);
		if (CREX_VM_OP_JMP_ADDR == (op2_flags & CREX_VM_OP_MASK)) {
			if (opline->opcode != CREX_CATCH || !(opline->extended_value & CREX_LAST_CATCH)) {
				if (b) {
					fprintf(stderr, " BB%d", b->successors[n++]);
				} else {
					fprintf(stderr, " %04u", (uint32_t)(OP_JMP_ADDR(opline, opline->op2) - op_array->opcodes));
				}
			}
		} else {
			crex_dump_unused_op(opline, opline->op2, op2_flags);
		}
	}

	if (CREX_VM_EXT_JMP_ADDR == (flags & CREX_VM_EXT_MASK)) {
		if (b) {
			fprintf(stderr, " BB%d", b->successors[n++]);
		} else {
			fprintf(stderr, " %04u", (uint32_t)CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, opline->extended_value));
		}
	}
	if (opline->result_type == IS_CONST) {
		crex_dump_const(CRT_CONSTANT(opline->result));
#if 0
	} else if (opline->result_type & IS_SMART_BRANCH_JMPZ) {
		fprintf(stderr, " jmpz");
	} else if (opline->result_type & IS_SMART_BRANCH_JMPNZ) {
		fprintf(stderr, " jmpnz");
#endif
	} else if (ssa_op && ssa_op->result_use >= 0) {
		if (opline->result_type & (IS_CV|IS_VAR|IS_TMP_VAR)) {
			if (ssa_op) {
				int ssa_var_num = ssa_op->result_use;
				if (ssa_var_num >= 0) {
					fprintf(stderr, " ");
					crex_dump_ssa_var(op_array, ssa, ssa_var_num, opline->result_type, EX_VAR_TO_NUM(opline->result.var), dump_flags);
				}
			} else {
				fprintf(stderr, " ");
				crex_dump_var(op_array, opline->result_type, EX_VAR_TO_NUM(opline->result.var));
			}
			if (ssa_op) {
				int ssa_var_num = ssa_op->result_def;
				if (ssa_var_num >= 0) {
					fprintf(stderr, " -> ");
					crex_dump_ssa_var(op_array, ssa, ssa_var_num, opline->result_type, EX_VAR_TO_NUM(opline->result.var), dump_flags);
				}
			}
		}
	}
}

CREX_API void crex_dump_op_line(const crex_op_array *op_array, const crex_basic_block *b, const crex_op *opline, uint32_t dump_flags, const void *data)
{
	int len = 0;
	const crex_ssa *ssa = NULL;
	crex_ssa_op *ssa_op = NULL;

	if (dump_flags & CREX_DUMP_LINE_NUMBERS) {
		fprintf(stderr, "L%04u ", opline->lineno);
	}

	len = fprintf(stderr, "%04u", (uint32_t)(opline - op_array->opcodes));
	fprintf(stderr, "%*c", 5-len, ' ');

	if (dump_flags & CREX_DUMP_SSA) {
		ssa = (const crex_ssa*)data;
		if (ssa && ssa->ops) {
			ssa_op = &ssa->ops[opline - op_array->opcodes];
		}
	}

	crex_dump_op(op_array, b, opline, dump_flags, ssa, ssa_op);
	fprintf(stderr, "\n");
}

static void crex_dump_block_info(const crex_cfg *cfg, int n, uint32_t dump_flags)
{
	crex_basic_block *b = cfg->blocks + n;

	if (n > 0) {
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "BB%d:\n     ;", n);
	if (b->flags & CREX_BB_START) {
		fprintf(stderr, " start");
	}
	if (b->flags & CREX_BB_RECV_ENTRY) {
		fprintf(stderr, " recv");
	}
	if (b->flags & CREX_BB_FOLLOW) {
		fprintf(stderr, " follow");
	}
	if (b->flags & CREX_BB_TARGET) {
		fprintf(stderr, " target");
	}
	if (b->flags & CREX_BB_EXIT) {
		fprintf(stderr, " exit");
	}
	if (b->flags & (CREX_BB_ENTRY|CREX_BB_RECV_ENTRY)) {
		fprintf(stderr, " entry");
	}
	if (b->flags & CREX_BB_TRY) {
		fprintf(stderr, " try");
	}
	if (b->flags & CREX_BB_CATCH) {
		fprintf(stderr, " catch");
	}
	if (b->flags & CREX_BB_FINALLY) {
		fprintf(stderr, " finally");
	}
	if (b->flags & CREX_BB_FINALLY_END) {
		fprintf(stderr, " finally_end");
	}
	if (!(dump_flags & CREX_DUMP_HIDE_UNREACHABLE) && !(b->flags & CREX_BB_REACHABLE)) {
		fprintf(stderr, " unreachable");
	}
	if (b->flags & CREX_BB_UNREACHABLE_FREE) {
		fprintf(stderr, " unreachable_free");
	}
	if (b->flags & CREX_BB_LOOP_HEADER) {
		fprintf(stderr, " loop_header");
	}
	if (b->flags & CREX_BB_IRREDUCIBLE_LOOP) {
		fprintf(stderr, " irreducible");
	}
	if (b->len != 0) {
		fprintf(stderr, " lines=[%d-%d]", b->start, b->start + b->len - 1);
	} else {
		fprintf(stderr, " empty");
	}
	fprintf(stderr, "\n");

	if (b->predecessors_count) {
		int *p = cfg->predecessors + b->predecessor_offset;
		int *end = p + b->predecessors_count;

		fprintf(stderr, "     ; from=(BB%d", *p);
		for (p++; p < end; p++) {
			fprintf(stderr, ", BB%d", *p);
		}
		fprintf(stderr, ")\n");
	}

	if (b->successors_count > 0) {
		int s;
		fprintf(stderr, "     ; to=(BB%d", b->successors[0]);
		for (s = 1; s < b->successors_count; s++) {
			fprintf(stderr, ", BB%d", b->successors[s]);
		}
		fprintf(stderr, ")\n");
	}

	if (b->idom >= 0) {
		fprintf(stderr, "     ; idom=BB%d\n", b->idom);
	}
	if (b->level >= 0) {
		fprintf(stderr, "     ; level=%d\n", b->level);
	}
	if (b->loop_header >= 0) {
		fprintf(stderr, "     ; loop_header=%d\n", b->loop_header);
	}
	if (b->children >= 0) {
		int j = b->children;
		fprintf(stderr, "     ; children=(BB%d", j);
		j = cfg->blocks[j].next_child;
		while (j >= 0) {
			fprintf(stderr, ", BB%d", j);
			j = cfg->blocks[j].next_child;
		}
		fprintf(stderr, ")\n");
	}
}

static void crex_dump_block_header(const crex_cfg *cfg, const crex_op_array *op_array, const crex_ssa *ssa, int n, uint32_t dump_flags)
{
	crex_dump_block_info(cfg, n, dump_flags);
	if (ssa && ssa->blocks && ssa->blocks[n].phis) {
		crex_ssa_phi *p = ssa->blocks[n].phis;

		do {
			int j;

			fprintf(stderr, "     ");
			crex_dump_ssa_var(op_array, ssa, p->ssa_var, 0, p->var, dump_flags);
			if (p->pi < 0) {
				fprintf(stderr, " = Phi(");
				for (j = 0; j < cfg->blocks[n].predecessors_count; j++) {
					if (j > 0) {
						fprintf(stderr, ", ");
					}
					crex_dump_ssa_var(op_array, ssa, p->sources[j], 0, p->var, dump_flags);
				}
				fprintf(stderr, ")\n");
			} else {
				fprintf(stderr, " = Pi<BB%d>(", p->pi);
				crex_dump_ssa_var(op_array, ssa, p->sources[0], 0, p->var, dump_flags);
				fprintf(stderr, " &");
				if (p->has_range_constraint) {
					crex_dump_range_constraint(op_array, ssa, &p->constraint.range, dump_flags);
				} else {
					crex_dump_type_constraint(op_array, ssa, &p->constraint.type, dump_flags);
				}
				fprintf(stderr, ")\n");
			}
			p = p->next;
		} while (p);
	}
}

void crex_dump_op_array_name(const crex_op_array *op_array)
{
	if (op_array->function_name) {
		if (op_array->scope && op_array->scope->name) {
			fprintf(stderr, "%s::%s", op_array->scope->name->val, op_array->function_name->val);
		} else {
			fprintf(stderr, "%s", op_array->function_name->val);
		}
	} else {
		fprintf(stderr, "%s", "$_main");
	}
}

CREX_API void crex_dump_op_array(const crex_op_array *op_array, uint32_t dump_flags, const char *msg, const void *data)
{
	int i;
	const crex_cfg *cfg = NULL;
	const crex_ssa *ssa = NULL;
	crex_func_info *func_info = NULL;
	uint32_t func_flags = 0;

	if (dump_flags & (CREX_DUMP_CFG|CREX_DUMP_SSA)) {
		cfg = (const crex_cfg*)data;
		if (!cfg->blocks) {
			cfg = data = NULL;
		}
	}
	if (dump_flags & CREX_DUMP_SSA) {
		ssa = (const crex_ssa*)data;
	}

	func_info = CREX_FUNC_INFO(op_array);
	if (func_info) {
		func_flags = func_info->flags;
	}

	fprintf(stderr, "\n");
	crex_dump_op_array_name(op_array);
	fprintf(stderr, ":\n     ; (lines=%d, args=%d",
		op_array->last,
		op_array->num_args);
	fprintf(stderr, ", vars=%d, tmps=%d", op_array->last_var, op_array->T);
	if (ssa) {
		fprintf(stderr, ", ssa_vars=%d", ssa->vars_count);
	}
	if (func_flags & CREX_FUNC_INDIRECT_VAR_ACCESS) {
		fprintf(stderr, ", dynamic");
	}
	if (func_flags & CREX_FUNC_RECURSIVE) {
		fprintf(stderr, ", recursive");
		if (func_flags & CREX_FUNC_RECURSIVE_DIRECTLY) {
			fprintf(stderr, " directly");
		}
		if (func_flags & CREX_FUNC_RECURSIVE_INDIRECTLY) {
			fprintf(stderr, " indirectly");
		}
	}
	if (func_flags & CREX_FUNC_IRREDUCIBLE) {
		fprintf(stderr, ", irreducible");
	}
	if (func_flags & CREX_FUNC_NO_LOOPS) {
		fprintf(stderr, ", no_loops");
	}
	if (func_flags & CREX_FUNC_HAS_EXTENDED_STMT) {
		fprintf(stderr, ", extended_stmt");
	}
	if (func_flags & CREX_FUNC_HAS_EXTENDED_FCALL) {
		fprintf(stderr, ", extended_fcall");
	}
//TODO: this is useful only for JIT???
#if 0
	if (info->flags & CREX_JIT_FUNC_NO_IN_MEM_CVS) {
		fprintf(stderr, ", no_in_mem_cvs");
	}
	if (info->flags & CREX_JIT_FUNC_NO_USED_ARGS) {
		fprintf(stderr, ", no_used_args");
	}
	if (info->flags & CREX_JIT_FUNC_NO_SYMTAB) {
		fprintf(stderr, ", no_symtab");
	}
	if (info->flags & CREX_JIT_FUNC_NO_FRAME) {
		fprintf(stderr, ", no_frame");
	}
	if (info->flags & CREX_JIT_FUNC_INLINE) {
		fprintf(stderr, ", inline");
	}
#endif
	fprintf(stderr, ")\n");
	if (msg) {
		fprintf(stderr, "     ; (%s)\n", msg);
	}
	fprintf(stderr, "     ; %s:%u-%u\n", op_array->filename->val, op_array->line_start, op_array->line_end);

	if (func_info) {
		fprintf(stderr, "     ; return ");
		crex_dump_type_info(func_info->return_info.type, func_info->return_info.ce, func_info->return_info.is_instanceof, dump_flags);
		crex_dump_range(&func_info->return_info.range);
		fprintf(stderr, "\n");
	}

	if (ssa && ssa->var_info) {
		for (i = 0; i < op_array->last_var; i++) {
			fprintf(stderr, "     ; ");
			crex_dump_ssa_var(op_array, ssa, i, IS_CV, i, dump_flags);
			fprintf(stderr, "\n");
		}
	}

	if (cfg) {
		int n;
		crex_basic_block *b;

		for (n = 0; n < cfg->blocks_count; n++) {
			b = cfg->blocks + n;
			if (!(dump_flags & CREX_DUMP_HIDE_UNREACHABLE) || (b->flags & CREX_BB_REACHABLE)) {
				const crex_op *opline;
				const crex_op *end;

				crex_dump_block_header(cfg, op_array, ssa, n, dump_flags);
				opline = op_array->opcodes + b->start;
				end = opline + b->len;
				while (opline < end) {
					crex_dump_op_line(op_array, b, opline, dump_flags, data);
					opline++;
				}
			}
		}
		if (op_array->last_live_range && (dump_flags & CREX_DUMP_LIVE_RANGES)) {
			fprintf(stderr, "LIVE RANGES:\n");
			for (i = 0; i < op_array->last_live_range; i++) {
				fprintf(stderr,
					"     %u: %04u - %04u ",
					EX_VAR_TO_NUM(op_array->live_range[i].var & ~CREX_LIVE_MASK),
					op_array->live_range[i].start,
					op_array->live_range[i].end);
				switch (op_array->live_range[i].var & CREX_LIVE_MASK) {
					case CREX_LIVE_TMPVAR:
						fprintf(stderr, "(tmp/var)\n");
						break;
					case CREX_LIVE_LOOP:
						fprintf(stderr, "(loop)\n");
						break;
					case CREX_LIVE_SILENCE:
						fprintf(stderr, "(silence)\n");
						break;
					case CREX_LIVE_ROPE:
						fprintf(stderr, "(rope)\n");
						break;
					case CREX_LIVE_NEW:
						fprintf(stderr, "(new)\n");
						break;
				}
			}
		}
		if (op_array->last_try_catch) {
			fprintf(stderr, "EXCEPTION TABLE:\n");
			for (i = 0; i < op_array->last_try_catch; i++) {
				fprintf(stderr, "        BB%u",
					cfg->map[op_array->try_catch_array[i].try_op]);
				if (op_array->try_catch_array[i].catch_op) {
					fprintf(stderr, ", BB%u",
						cfg->map[op_array->try_catch_array[i].catch_op]);
				} else {
					fprintf(stderr, ", -");
				}
				if (op_array->try_catch_array[i].finally_op) {
					fprintf(stderr, ", BB%u",
						cfg->map[op_array->try_catch_array[i].finally_op]);
				} else {
					fprintf(stderr, ", -");
				}
				if (op_array->try_catch_array[i].finally_end) {
					fprintf(stderr, ", BB%u\n",
						cfg->map[op_array->try_catch_array[i].finally_end]);
				} else {
					fprintf(stderr, ", -\n");
				}
			}
		}
	} else {
		const crex_op *opline = op_array->opcodes;
		const crex_op *end = opline + op_array->last;

		while (opline < end) {
			crex_dump_op_line(op_array, NULL, opline, dump_flags, data);
			opline++;
		}
		if (op_array->last_live_range && (dump_flags & CREX_DUMP_LIVE_RANGES)) {
			fprintf(stderr, "LIVE RANGES:\n");
			for (i = 0; i < op_array->last_live_range; i++) {
				fprintf(stderr,
					"     %u: %04u - %04u ",
					EX_VAR_TO_NUM(op_array->live_range[i].var & ~CREX_LIVE_MASK),
					op_array->live_range[i].start,
					op_array->live_range[i].end);
				switch (op_array->live_range[i].var & CREX_LIVE_MASK) {
					case CREX_LIVE_TMPVAR:
						fprintf(stderr, "(tmp/var)\n");
						break;
					case CREX_LIVE_LOOP:
						fprintf(stderr, "(loop)\n");
						break;
					case CREX_LIVE_SILENCE:
						fprintf(stderr, "(silence)\n");
						break;
					case CREX_LIVE_ROPE:
						fprintf(stderr, "(rope)\n");
						break;
					case CREX_LIVE_NEW:
						fprintf(stderr, "(new)\n");
						break;
				}
			}
		}
		if (op_array->last_try_catch) {
			fprintf(stderr, "EXCEPTION TABLE:\n");
			for (i = 0; i < op_array->last_try_catch; i++) {
				fprintf(stderr,
					"     %04u",
					op_array->try_catch_array[i].try_op);

				if (op_array->try_catch_array[i].catch_op) {
					fprintf(stderr,
						", %04u",
						op_array->try_catch_array[i].catch_op);
				} else {
					fprintf(stderr, ", -");
				}
				if (op_array->try_catch_array[i].finally_op) {
					fprintf(stderr,
						", %04u",
						op_array->try_catch_array[i].finally_op);
				} else {
					fprintf(stderr, ", -");
				}
				if (op_array->try_catch_array[i].finally_end) {
					fprintf(stderr,
						", %04u",
						op_array->try_catch_array[i].finally_end);
				} else {
					fprintf(stderr, ", -\n");
				}
			}
		}
	}
}

void crex_dump_dominators(const crex_op_array *op_array, const crex_cfg *cfg)
{
	int j;

	fprintf(stderr, "\nDOMINATORS-TREE for \"");
	crex_dump_op_array_name(op_array);
	fprintf(stderr, "\"\n");
	for (j = 0; j < cfg->blocks_count; j++) {
		crex_basic_block *b = cfg->blocks + j;
		if (b->flags & CREX_BB_REACHABLE) {
			crex_dump_block_info(cfg, j, 0);
		}
	}
}

void crex_dump_variables(const crex_op_array *op_array)
{
	int j;

	fprintf(stderr, "\nCV Variables for \"");
	crex_dump_op_array_name(op_array);
	fprintf(stderr, "\"\n");
	for (j = 0; j < op_array->last_var; j++) {
		fprintf(stderr, "    ");
		crex_dump_var(op_array, IS_CV, j);
		fprintf(stderr, "\n");
	}
}

void crex_dump_ssa_variables(const crex_op_array *op_array, const crex_ssa *ssa, uint32_t dump_flags)
{
	int j;

	if (ssa->vars) {
		fprintf(stderr, "\nSSA Variable for \"");
		crex_dump_op_array_name(op_array);
		fprintf(stderr, "\"\n");

		for (j = 0; j < ssa->vars_count; j++) {
			fprintf(stderr, "    ");
			crex_dump_ssa_var(op_array, ssa, j, IS_CV, ssa->vars[j].var, dump_flags);
			if (ssa->vars[j].scc >= 0) {
				if (ssa->vars[j].scc_entry) {
					fprintf(stderr, " *");
				}  else {
					fprintf(stderr, "  ");
				}
				fprintf(stderr, "SCC=%d", ssa->vars[j].scc);
			}
			fprintf(stderr, "\n");
		}
	}
}

static void crex_dump_var_set(const crex_op_array *op_array, const char *name, crex_bitset set)
{
	bool first = 1;
	uint32_t i;

	fprintf(stderr, "    ; %s = {", name);
	for (i = 0; i < op_array->last_var + op_array->T; i++) {
		if (crex_bitset_in(set, i)) {
			if (first) {
				first = 0;
			} else {
				fprintf(stderr, ", ");
			}
			crex_dump_var(op_array, IS_CV, i);
		}
	}
	fprintf(stderr, "}\n");
}

void crex_dump_dfg(const crex_op_array *op_array, const crex_cfg *cfg, const crex_dfg *dfg)
{
	int j;
	fprintf(stderr, "\nVariable Liveness for \"");
	crex_dump_op_array_name(op_array);
	fprintf(stderr, "\"\n");

	for (j = 0; j < cfg->blocks_count; j++) {
		fprintf(stderr, "  BB%d:\n", j);
		crex_dump_var_set(op_array, "def", DFG_BITSET(dfg->def, dfg->size, j));
		crex_dump_var_set(op_array, "use", DFG_BITSET(dfg->use, dfg->size, j));
		crex_dump_var_set(op_array, "in ", DFG_BITSET(dfg->in,  dfg->size, j));
		crex_dump_var_set(op_array, "out", DFG_BITSET(dfg->out, dfg->size, j));
	}
}

void crex_dump_phi_placement(const crex_op_array *op_array, const crex_ssa *ssa)
{
	int j;
	crex_ssa_block *ssa_blocks = ssa->blocks;
	int blocks_count = ssa->cfg.blocks_count;

	fprintf(stderr, "\nSSA Phi() Placement for \"");
	crex_dump_op_array_name(op_array);
	fprintf(stderr, "\"\n");
	for (j = 0; j < blocks_count; j++) {
		if (ssa_blocks && ssa_blocks[j].phis) {
			crex_ssa_phi *p = ssa_blocks[j].phis;
			int first = 1;

			fprintf(stderr, "  BB%d:\n", j);
			if (p->pi >= 0) {
				fprintf(stderr, "    ; pi={");
			} else {
				fprintf(stderr, "    ; phi={");
			}
			do {
				if (first) {
					first = 0;
				} else {
					fprintf(stderr, ", ");
				}
				crex_dump_var(op_array, IS_CV, p->var);
				p = p->next;
			} while (p);
			fprintf(stderr, "}\n");
		}
	}
}
