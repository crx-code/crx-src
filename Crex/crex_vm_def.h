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

/* If you change this file, please regenerate the crex_vm_execute.h and
 * crex_vm_opcodes.h files by running:
 * crx crex_vm_gen.crx
 */

CREX_VM_HELPER(crex_add_helper, ANY, ANY, zval *op_1, zval *op_2)
{
	USE_OPLINE

	SAVE_OPLINE();
	if (UNEXPECTED(C_TYPE_INFO_P(op_1) == IS_UNDEF)) {
		op_1 = ZVAL_UNDEFINED_OP1();
	}
	if (UNEXPECTED(C_TYPE_INFO_P(op_2) == IS_UNDEF)) {
		op_2 = ZVAL_UNDEFINED_OP2();
	}
	add_function(EX_VAR(opline->result.var), op_1, op_2);
	if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_1);
	}
	if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_2);
	}
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HOT_NOCONSTCONST_HANDLER(1, CREX_ADD, CONST|TMPVARCV, CONST|TMPVARCV)
{
	USE_OPLINE
	zval *op1, *op2, *result;
	double d1, d2;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (CREX_VM_SPEC && OP1_TYPE == IS_CONST && OP2_TYPE == IS_CONST) {
		/* pass */
	} else if (EXPECTED(C_TYPE_INFO_P(op1) == IS_LONG)) {
		if (EXPECTED(C_TYPE_INFO_P(op2) == IS_LONG)) {
			result = EX_VAR(opline->result.var);
			fast_long_add_function(result, op1, op2);
			CREX_VM_NEXT_OPCODE();
		} else if (EXPECTED(C_TYPE_INFO_P(op2) == IS_DOUBLE)) {
			d1 = (double)C_LVAL_P(op1);
			d2 = C_DVAL_P(op2);
			CREX_VM_C_GOTO(add_double);
		}
	} else if (EXPECTED(C_TYPE_INFO_P(op1) == IS_DOUBLE)) {
		if (EXPECTED(C_TYPE_INFO_P(op2) == IS_DOUBLE)) {
			d1 = C_DVAL_P(op1);
			d2 = C_DVAL_P(op2);
CREX_VM_C_LABEL(add_double):
			result = EX_VAR(opline->result.var);
			ZVAL_DOUBLE(result, d1 + d2);
			CREX_VM_NEXT_OPCODE();
		} else if (EXPECTED(C_TYPE_INFO_P(op2) == IS_LONG)) {
			d1 = C_DVAL_P(op1);
			d2 = (double)C_LVAL_P(op2);
			CREX_VM_C_GOTO(add_double);
		}
	}

	CREX_VM_DISPATCH_TO_HELPER(crex_add_helper, op_1, op1, op_2, op2);
}

CREX_VM_HELPER(crex_sub_helper, ANY, ANY, zval *op_1, zval *op_2)
{
	USE_OPLINE

	SAVE_OPLINE();
	if (UNEXPECTED(C_TYPE_INFO_P(op_1) == IS_UNDEF)) {
		op_1 = ZVAL_UNDEFINED_OP1();
	}
	if (UNEXPECTED(C_TYPE_INFO_P(op_2) == IS_UNDEF)) {
		op_2 = ZVAL_UNDEFINED_OP2();
	}
	sub_function(EX_VAR(opline->result.var), op_1, op_2);
	if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_1);
	}
	if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_2);
	}
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HOT_NOCONSTCONST_HANDLER(2, CREX_SUB, CONST|TMPVARCV, CONST|TMPVARCV)
{
	USE_OPLINE
	zval *op1, *op2, *result;
	double d1, d2;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (CREX_VM_SPEC && OP1_TYPE == IS_CONST && OP2_TYPE == IS_CONST) {
		/* pass */
	} else if (EXPECTED(C_TYPE_INFO_P(op1) == IS_LONG)) {
		if (EXPECTED(C_TYPE_INFO_P(op2) == IS_LONG)) {
			result = EX_VAR(opline->result.var);
			fast_long_sub_function(result, op1, op2);
			CREX_VM_NEXT_OPCODE();
		} else if (EXPECTED(C_TYPE_INFO_P(op2) == IS_DOUBLE)) {
			d1 = (double)C_LVAL_P(op1);
			d2 = C_DVAL_P(op2);
			CREX_VM_C_GOTO(sub_double);
		}
	} else if (EXPECTED(C_TYPE_INFO_P(op1) == IS_DOUBLE)) {
		if (EXPECTED(C_TYPE_INFO_P(op2) == IS_DOUBLE)) {
			d1 = C_DVAL_P(op1);
			d2 = C_DVAL_P(op2);
CREX_VM_C_LABEL(sub_double):
			result = EX_VAR(opline->result.var);
			ZVAL_DOUBLE(result, d1 - d2);
			CREX_VM_NEXT_OPCODE();
		} else if (EXPECTED(C_TYPE_INFO_P(op2) == IS_LONG)) {
			d1 = C_DVAL_P(op1);
			d2 = (double)C_LVAL_P(op2);
			CREX_VM_C_GOTO(sub_double);
		}
	}

	CREX_VM_DISPATCH_TO_HELPER(crex_sub_helper, op_1, op1, op_2, op2);
}

CREX_VM_HELPER(crex_mul_helper, ANY, ANY, zval *op_1, zval *op_2)
{
	USE_OPLINE

	SAVE_OPLINE();
	if (UNEXPECTED(C_TYPE_INFO_P(op_1) == IS_UNDEF)) {
		op_1 = ZVAL_UNDEFINED_OP1();
	}
	if (UNEXPECTED(C_TYPE_INFO_P(op_2) == IS_UNDEF)) {
		op_2 = ZVAL_UNDEFINED_OP2();
	}
	mul_function(EX_VAR(opline->result.var), op_1, op_2);
	if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_1);
	}
	if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_2);
	}
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_COLD_CONSTCONST_HANDLER(3, CREX_MUL, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(COMMUTATIVE))
{
	USE_OPLINE
	zval *op1, *op2, *result;
	double d1, d2;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (CREX_VM_SPEC && OP1_TYPE == IS_CONST && OP2_TYPE == IS_CONST) {
		/* pass */
	} else if (EXPECTED(C_TYPE_INFO_P(op1) == IS_LONG)) {
		if (EXPECTED(C_TYPE_INFO_P(op2) == IS_LONG)) {
			crex_long overflow;

			result = EX_VAR(opline->result.var);
			CREX_SIGNED_MULTIPLY_LONG(C_LVAL_P(op1), C_LVAL_P(op2), C_LVAL_P(result), C_DVAL_P(result), overflow);
			C_TYPE_INFO_P(result) = overflow ? IS_DOUBLE : IS_LONG;
			CREX_VM_NEXT_OPCODE();
		} else if (EXPECTED(C_TYPE_INFO_P(op2) == IS_DOUBLE)) {
			d1 = (double)C_LVAL_P(op1);
			d2 = C_DVAL_P(op2);
			CREX_VM_C_GOTO(mul_double);
		}
	} else if (EXPECTED(C_TYPE_INFO_P(op1) == IS_DOUBLE)) {
		if (EXPECTED(C_TYPE_INFO_P(op2) == IS_DOUBLE)) {
			d1 = C_DVAL_P(op1);
			d2 = C_DVAL_P(op2);
CREX_VM_C_LABEL(mul_double):
			result = EX_VAR(opline->result.var);
			ZVAL_DOUBLE(result, d1 * d2);
			CREX_VM_NEXT_OPCODE();
		} else if (EXPECTED(C_TYPE_INFO_P(op2) == IS_LONG)) {
			d1 = C_DVAL_P(op1);
			d2 = (double)C_LVAL_P(op2);
			CREX_VM_C_GOTO(mul_double);
		}
	}

	CREX_VM_DISPATCH_TO_HELPER(crex_mul_helper, op_1, op1, op_2, op2);
}

CREX_VM_COLD_CONSTCONST_HANDLER(4, CREX_DIV, CONST|TMPVAR|CV, CONST|TMPVAR|CV)
{
	USE_OPLINE
	zval *op1, *op2;

	SAVE_OPLINE();
	op1 = GET_OP1_ZVAL_PTR(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR(BP_VAR_R);
	div_function(EX_VAR(opline->result.var), op1, op2);
	FREE_OP1();
	FREE_OP2();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_COLD_HELPER(crex_mod_by_zero_helper, ANY, ANY)
{
	USE_OPLINE

	SAVE_OPLINE();
	crex_throw_exception_ex(crex_ce_division_by_zero_error, 0, "Modulo by zero");
	ZVAL_UNDEF(EX_VAR(opline->result.var));
	HANDLE_EXCEPTION();
}

CREX_VM_HELPER(crex_mod_helper, ANY, ANY, zval *op_1, zval *op_2)
{
	USE_OPLINE

	SAVE_OPLINE();
	if (UNEXPECTED(C_TYPE_INFO_P(op_1) == IS_UNDEF)) {
		op_1 = ZVAL_UNDEFINED_OP1();
	}
	if (UNEXPECTED(C_TYPE_INFO_P(op_2) == IS_UNDEF)) {
		op_2 = ZVAL_UNDEFINED_OP2();
	}
	mod_function(EX_VAR(opline->result.var), op_1, op_2);
	if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_1);
	}
	if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_2);
	}
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_COLD_CONSTCONST_HANDLER(5, CREX_MOD, CONST|TMPVARCV, CONST|TMPVARCV)
{
	USE_OPLINE
	zval *op1, *op2, *result;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (CREX_VM_SPEC && OP1_TYPE == IS_CONST && OP2_TYPE == IS_CONST) {
		/* pass */
	} else if (EXPECTED(C_TYPE_INFO_P(op1) == IS_LONG)) {
		if (EXPECTED(C_TYPE_INFO_P(op2) == IS_LONG)) {
			result = EX_VAR(opline->result.var);
			if (UNEXPECTED(C_LVAL_P(op2) == 0)) {
				CREX_VM_DISPATCH_TO_HELPER(crex_mod_by_zero_helper);
			} else if (UNEXPECTED(C_LVAL_P(op2) == -1)) {
				/* Prevent overflow error/crash if op1==CREX_LONG_MIN */
				ZVAL_LONG(result, 0);
			} else {
				ZVAL_LONG(result, C_LVAL_P(op1) % C_LVAL_P(op2));
			}
			CREX_VM_NEXT_OPCODE();
		}
	}

	CREX_VM_DISPATCH_TO_HELPER(crex_mod_helper, op_1, op1, op_2, op2);
}

CREX_VM_HELPER(crex_shift_left_helper, ANY, ANY, zval *op_1, zval *op_2)
{
	USE_OPLINE

	SAVE_OPLINE();
	if (UNEXPECTED(C_TYPE_INFO_P(op_1) == IS_UNDEF)) {
		op_1 = ZVAL_UNDEFINED_OP1();
	}
	if (UNEXPECTED(C_TYPE_INFO_P(op_2) == IS_UNDEF)) {
		op_2 = ZVAL_UNDEFINED_OP2();
	}
	shift_left_function(EX_VAR(opline->result.var), op_1, op_2);
	if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_1);
	}
	if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_2);
	}
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_COLD_CONSTCONST_HANDLER(6, CREX_SL, CONST|TMPVARCV, CONST|TMPVARCV)
{
	USE_OPLINE
	zval *op1, *op2;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (CREX_VM_SPEC && OP1_TYPE == IS_CONST && OP2_TYPE == IS_CONST) {
		/* pass */
	} else if (EXPECTED(C_TYPE_INFO_P(op1) == IS_LONG)
			&& EXPECTED(C_TYPE_INFO_P(op2) == IS_LONG)
			&& EXPECTED((crex_ulong)C_LVAL_P(op2) < SIZEOF_CREX_LONG * 8)) {
		/* Perform shift on unsigned numbers to get well-defined wrap behavior. */
		ZVAL_LONG(EX_VAR(opline->result.var),
			(crex_long) ((crex_ulong) C_LVAL_P(op1) << C_LVAL_P(op2)));
		CREX_VM_NEXT_OPCODE();
	}

	CREX_VM_DISPATCH_TO_HELPER(crex_shift_left_helper, op_1, op1, op_2, op2);
}

CREX_VM_HELPER(crex_shift_right_helper, ANY, ANY, zval *op_1, zval *op_2)
{
	USE_OPLINE

	SAVE_OPLINE();
	if (UNEXPECTED(C_TYPE_INFO_P(op_1) == IS_UNDEF)) {
		op_1 = ZVAL_UNDEFINED_OP1();
	}
	if (UNEXPECTED(C_TYPE_INFO_P(op_2) == IS_UNDEF)) {
		op_2 = ZVAL_UNDEFINED_OP2();
	}
	shift_right_function(EX_VAR(opline->result.var), op_1, op_2);
	if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_1);
	}
	if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_2);
	}
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_COLD_CONSTCONST_HANDLER(7, CREX_SR, CONST|TMPVARCV, CONST|TMPVARCV)
{
	USE_OPLINE
	zval *op1, *op2;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (CREX_VM_SPEC && OP1_TYPE == IS_CONST && OP2_TYPE == IS_CONST) {
		/* pass */
	} else if (EXPECTED(C_TYPE_INFO_P(op1) == IS_LONG)
			&& EXPECTED(C_TYPE_INFO_P(op2) == IS_LONG)
			&& EXPECTED((crex_ulong)C_LVAL_P(op2) < SIZEOF_CREX_LONG * 8)) {
		ZVAL_LONG(EX_VAR(opline->result.var), C_LVAL_P(op1) >> C_LVAL_P(op2));
		CREX_VM_NEXT_OPCODE();
	}

	CREX_VM_DISPATCH_TO_HELPER(crex_shift_right_helper, op_1, op1, op_2, op2);
}

CREX_VM_COLD_CONSTCONST_HANDLER(12, CREX_POW, CONST|TMPVAR|CV, CONST|TMPVAR|CV)
{
	USE_OPLINE
	zval *op1, *op2;

	SAVE_OPLINE();
	op1 = GET_OP1_ZVAL_PTR(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR(BP_VAR_R);
	pow_function(EX_VAR(opline->result.var), op1, op2);
	FREE_OP1();
	FREE_OP2();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(8, CREX_CONCAT, CONST|TMPVAR|CV, CONST|TMPVAR|CV, SPEC(NO_CONST_CONST))
{
	USE_OPLINE
	zval *op1, *op2;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);

	if ((OP1_TYPE == IS_CONST || EXPECTED(C_TYPE_P(op1) == IS_STRING)) &&
	    (OP2_TYPE == IS_CONST || EXPECTED(C_TYPE_P(op2) == IS_STRING))) {
		crex_string *op1_str = C_STR_P(op1);
		crex_string *op2_str = C_STR_P(op2);
		crex_string *str;
		uint32_t flags = ZSTR_GET_COPYABLE_CONCAT_PROPERTIES_BOTH(op1_str, op2_str);

		if (OP1_TYPE != IS_CONST && UNEXPECTED(ZSTR_LEN(op1_str) == 0)) {
			if (OP2_TYPE == IS_CONST || OP2_TYPE == IS_CV) {
				ZVAL_STR_COPY(EX_VAR(opline->result.var), op2_str);
			} else {
				ZVAL_STR(EX_VAR(opline->result.var), op2_str);
			}
			if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
				crex_string_release_ex(op1_str, 0);
			}
		} else if (OP2_TYPE != IS_CONST && UNEXPECTED(ZSTR_LEN(op2_str) == 0)) {
			if (OP1_TYPE == IS_CONST || OP1_TYPE == IS_CV) {
				ZVAL_STR_COPY(EX_VAR(opline->result.var), op1_str);
			} else {
				ZVAL_STR(EX_VAR(opline->result.var), op1_str);
			}
			if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
				crex_string_release_ex(op2_str, 0);
			}
		} else if (OP1_TYPE != IS_CONST && OP1_TYPE != IS_CV &&
		    !ZSTR_IS_INTERNED(op1_str) && GC_REFCOUNT(op1_str) == 1) {
			size_t len = ZSTR_LEN(op1_str);

			if (UNEXPECTED(len > ZSTR_MAX_LEN - ZSTR_LEN(op2_str))) {
				crex_error_noreturn(E_ERROR, "Integer overflow in memory allocation");
			}
			str = crex_string_extend(op1_str, len + ZSTR_LEN(op2_str), 0);
			memcpy(ZSTR_VAL(str) + len, ZSTR_VAL(op2_str), ZSTR_LEN(op2_str)+1);
			GC_ADD_FLAGS(str, flags);
			ZVAL_NEW_STR(EX_VAR(opline->result.var), str);
			if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
				crex_string_release_ex(op2_str, 0);
			}
		} else {
			str = crex_string_alloc(ZSTR_LEN(op1_str) + ZSTR_LEN(op2_str), 0);
			memcpy(ZSTR_VAL(str), ZSTR_VAL(op1_str), ZSTR_LEN(op1_str));
			memcpy(ZSTR_VAL(str) + ZSTR_LEN(op1_str), ZSTR_VAL(op2_str), ZSTR_LEN(op2_str)+1);
			GC_ADD_FLAGS(str, flags);
			ZVAL_NEW_STR(EX_VAR(opline->result.var), str);
			if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
				crex_string_release_ex(op1_str, 0);
			}
			if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
				crex_string_release_ex(op2_str, 0);
			}
		}
		CREX_VM_NEXT_OPCODE();
	} else {
		SAVE_OPLINE();

		if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(op1) == IS_UNDEF)) {
			op1 = ZVAL_UNDEFINED_OP1();
		}
		if (OP2_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(op2) == IS_UNDEF)) {
			op2 = ZVAL_UNDEFINED_OP2();
		}
		concat_function(EX_VAR(opline->result.var), op1, op2);
		FREE_OP1();
		FREE_OP2();
		CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
	}
}

CREX_VM_COLD_CONSTCONST_HANDLER(16, CREX_IS_IDENTICAL, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV, SPEC(COMMUTATIVE))
{
	USE_OPLINE
	zval *op1, *op2;
	bool result;

	SAVE_OPLINE();
	op1 = GET_OP1_ZVAL_PTR_DEREF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_DEREF(BP_VAR_R);
	result = fast_is_identical_function(op1, op2);
	FREE_OP1();
	FREE_OP2();
	CREX_VM_SMART_BRANCH(result, 1);
}

CREX_VM_HANDLER(196, CREX_CASE_STRICT, TMP|VAR, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zval *op1, *op2;
	bool result;

	SAVE_OPLINE();
	op1 = GET_OP1_ZVAL_PTR_DEREF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_DEREF(BP_VAR_R);
	result = fast_is_identical_function(op1, op2);
	FREE_OP2();
	CREX_VM_SMART_BRANCH(result, 1);
}

CREX_VM_COLD_CONSTCONST_HANDLER(17, CREX_IS_NOT_IDENTICAL, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV, SPEC(COMMUTATIVE))
{
	USE_OPLINE
	zval *op1, *op2;
	bool result;

	SAVE_OPLINE();
	op1 = GET_OP1_ZVAL_PTR_DEREF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_DEREF(BP_VAR_R);
	result = fast_is_not_identical_function(op1, op2);
	FREE_OP1();
	FREE_OP2();
	CREX_VM_SMART_BRANCH(result, 1);
}

CREX_VM_HELPER(crex_is_equal_helper, ANY, ANY, zval *op_1, zval *op_2)
{
	int ret;
	USE_OPLINE

	SAVE_OPLINE();
	if (UNEXPECTED(C_TYPE_INFO_P(op_1) == IS_UNDEF)) {
		op_1 = ZVAL_UNDEFINED_OP1();
	}
	if (UNEXPECTED(C_TYPE_INFO_P(op_2) == IS_UNDEF)) {
		op_2 = ZVAL_UNDEFINED_OP2();
	}
	ret = crex_compare(op_1, op_2);
	if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_1);
	}
	if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_2);
	}
	CREX_VM_SMART_BRANCH(ret == 0, 1);
}

CREX_VM_COLD_CONSTCONST_HANDLER(18, CREX_IS_EQUAL, CONST|TMPVAR|CV, CONST|TMPVAR|CV, SPEC(SMART_BRANCH,COMMUTATIVE))
{
	USE_OPLINE
	zval *op1, *op2;
	double d1, d2;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (CREX_VM_SPEC && OP1_TYPE == IS_CONST && OP2_TYPE == IS_CONST) {
		/* pass */
	} else if (EXPECTED(C_TYPE_P(op1) == IS_LONG)) {
		if (EXPECTED(C_TYPE_P(op2) == IS_LONG)) {
			if (EXPECTED(C_LVAL_P(op1) == C_LVAL_P(op2))) {
CREX_VM_C_LABEL(is_equal_true):
				CREX_VM_SMART_BRANCH_TRUE();
			} else {
CREX_VM_C_LABEL(is_equal_false):
				CREX_VM_SMART_BRANCH_FALSE();
			}
		} else if (EXPECTED(C_TYPE_P(op2) == IS_DOUBLE)) {
			d1 = (double)C_LVAL_P(op1);
			d2 = C_DVAL_P(op2);
			CREX_VM_C_GOTO(is_equal_double);
		}
	} else if (EXPECTED(C_TYPE_P(op1) == IS_DOUBLE)) {
		if (EXPECTED(C_TYPE_P(op2) == IS_DOUBLE)) {
			d1 = C_DVAL_P(op1);
			d2 = C_DVAL_P(op2);
CREX_VM_C_LABEL(is_equal_double):
			if (d1 == d2) {
				CREX_VM_C_GOTO(is_equal_true);
			} else {
				CREX_VM_C_GOTO(is_equal_false);
			}
		} else if (EXPECTED(C_TYPE_P(op2) == IS_LONG)) {
			d1 = C_DVAL_P(op1);
			d2 = (double)C_LVAL_P(op2);
			CREX_VM_C_GOTO(is_equal_double);
		}
	} else if (EXPECTED(C_TYPE_P(op1) == IS_STRING)) {
		if (EXPECTED(C_TYPE_P(op2) == IS_STRING)) {
			bool result = crex_fast_equal_strings(C_STR_P(op1), C_STR_P(op2));
			if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
				zval_ptr_dtor_str(op1);
			}
			if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
				zval_ptr_dtor_str(op2);
			}
			if (result) {
				CREX_VM_C_GOTO(is_equal_true);
			} else {
				CREX_VM_C_GOTO(is_equal_false);
			}
		}
	}
	CREX_VM_DISPATCH_TO_HELPER(crex_is_equal_helper, op_1, op1, op_2, op2);
}

CREX_VM_HELPER(crex_is_not_equal_helper, ANY, ANY, zval *op_1, zval *op_2)
{
	int ret;
	USE_OPLINE

	SAVE_OPLINE();
	if (UNEXPECTED(C_TYPE_INFO_P(op_1) == IS_UNDEF)) {
		op_1 = ZVAL_UNDEFINED_OP1();
	}
	if (UNEXPECTED(C_TYPE_INFO_P(op_2) == IS_UNDEF)) {
		op_2 = ZVAL_UNDEFINED_OP2();
	}
	ret = crex_compare(op_1, op_2);
	if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_1);
	}
	if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_2);
	}
	CREX_VM_SMART_BRANCH(ret != 0, 1);
}

CREX_VM_COLD_CONSTCONST_HANDLER(19, CREX_IS_NOT_EQUAL, CONST|TMPVAR|CV, CONST|TMPVAR|CV, SPEC(SMART_BRANCH,COMMUTATIVE))
{
	USE_OPLINE
	zval *op1, *op2;
	double d1, d2;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (CREX_VM_SPEC && OP1_TYPE == IS_CONST && OP2_TYPE == IS_CONST) {
		/* pass */
	} else if (EXPECTED(C_TYPE_P(op1) == IS_LONG)) {
		if (EXPECTED(C_TYPE_P(op2) == IS_LONG)) {
			if (EXPECTED(C_LVAL_P(op1) != C_LVAL_P(op2))) {
CREX_VM_C_LABEL(is_not_equal_true):
				CREX_VM_SMART_BRANCH_TRUE();
			} else {
CREX_VM_C_LABEL(is_not_equal_false):
				CREX_VM_SMART_BRANCH_FALSE();
			}
		} else if (EXPECTED(C_TYPE_P(op2) == IS_DOUBLE)) {
			d1 = (double)C_LVAL_P(op1);
			d2 = C_DVAL_P(op2);
			CREX_VM_C_GOTO(is_not_equal_double);
		}
	} else if (EXPECTED(C_TYPE_P(op1) == IS_DOUBLE)) {
		if (EXPECTED(C_TYPE_P(op2) == IS_DOUBLE)) {
			d1 = C_DVAL_P(op1);
			d2 = C_DVAL_P(op2);
CREX_VM_C_LABEL(is_not_equal_double):
			if (d1 != d2) {
				CREX_VM_C_GOTO(is_not_equal_true);
			} else {
				CREX_VM_C_GOTO(is_not_equal_false);
			}
		} else if (EXPECTED(C_TYPE_P(op2) == IS_LONG)) {
			d1 = C_DVAL_P(op1);
			d2 = (double)C_LVAL_P(op2);
			CREX_VM_C_GOTO(is_not_equal_double);
		}
	} else if (EXPECTED(C_TYPE_P(op1) == IS_STRING)) {
		if (EXPECTED(C_TYPE_P(op2) == IS_STRING)) {
			bool result = crex_fast_equal_strings(C_STR_P(op1), C_STR_P(op2));
			if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
				zval_ptr_dtor_str(op1);
			}
			if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
				zval_ptr_dtor_str(op2);
			}
			if (!result) {
				CREX_VM_C_GOTO(is_not_equal_true);
			} else {
				CREX_VM_C_GOTO(is_not_equal_false);
			}
		}
	}
	CREX_VM_DISPATCH_TO_HELPER(crex_is_not_equal_helper, op_1, op1, op_2, op2);
}

CREX_VM_HELPER(crex_is_smaller_helper, ANY, ANY, zval *op_1, zval *op_2)
{
	int ret;
	USE_OPLINE

	SAVE_OPLINE();
	if (UNEXPECTED(C_TYPE_INFO_P(op_1) == IS_UNDEF)) {
		op_1 = ZVAL_UNDEFINED_OP1();
	}
	if (UNEXPECTED(C_TYPE_INFO_P(op_2) == IS_UNDEF)) {
		op_2 = ZVAL_UNDEFINED_OP2();
	}
	ret = crex_compare(op_1, op_2);
	if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_1);
	}
	if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_2);
	}
	CREX_VM_SMART_BRANCH(ret < 0, 1);
}

CREX_VM_HOT_NOCONSTCONST_HANDLER(20, CREX_IS_SMALLER, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(SMART_BRANCH))
{
	USE_OPLINE
	zval *op1, *op2;
	double d1, d2;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (CREX_VM_SPEC && OP1_TYPE == IS_CONST && OP2_TYPE == IS_CONST) {
		/* pass */
	} else if (EXPECTED(C_TYPE_INFO_P(op1) == IS_LONG)) {
		if (EXPECTED(C_TYPE_INFO_P(op2) == IS_LONG)) {
			if (EXPECTED(C_LVAL_P(op1) < C_LVAL_P(op2))) {
CREX_VM_C_LABEL(is_smaller_true):
				CREX_VM_SMART_BRANCH_TRUE();
			} else {
CREX_VM_C_LABEL(is_smaller_false):
				CREX_VM_SMART_BRANCH_FALSE();
			}
		} else if (EXPECTED(C_TYPE_INFO_P(op2) == IS_DOUBLE)) {
			d1 = (double)C_LVAL_P(op1);
			d2 = C_DVAL_P(op2);
			CREX_VM_C_GOTO(is_smaller_double);
		}
	} else if (EXPECTED(C_TYPE_INFO_P(op1) == IS_DOUBLE)) {
		if (EXPECTED(C_TYPE_INFO_P(op2) == IS_DOUBLE)) {
			d1 = C_DVAL_P(op1);
			d2 = C_DVAL_P(op2);
CREX_VM_C_LABEL(is_smaller_double):
			if (d1 < d2) {
				CREX_VM_C_GOTO(is_smaller_true);
			} else {
				CREX_VM_C_GOTO(is_smaller_false);
			}
		} else if (EXPECTED(C_TYPE_INFO_P(op2) == IS_LONG)) {
			d1 = C_DVAL_P(op1);
			d2 = (double)C_LVAL_P(op2);
			CREX_VM_C_GOTO(is_smaller_double);
		}
	}
	CREX_VM_DISPATCH_TO_HELPER(crex_is_smaller_helper, op_1, op1, op_2, op2);
}

CREX_VM_HELPER(crex_is_smaller_or_equal_helper, ANY, ANY, zval *op_1, zval *op_2)
{
	int ret;
	USE_OPLINE

	SAVE_OPLINE();
	if (UNEXPECTED(C_TYPE_INFO_P(op_1) == IS_UNDEF)) {
		op_1 = ZVAL_UNDEFINED_OP1();
	}
	if (UNEXPECTED(C_TYPE_INFO_P(op_2) == IS_UNDEF)) {
		op_2 = ZVAL_UNDEFINED_OP2();
	}
	ret = crex_compare(op_1, op_2);
	if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_1);
	}
	if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_2);
	}
	CREX_VM_SMART_BRANCH(ret <= 0, 1);
}

CREX_VM_HOT_NOCONSTCONST_HANDLER(21, CREX_IS_SMALLER_OR_EQUAL, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(SMART_BRANCH))
{
	USE_OPLINE
	zval *op1, *op2;
	double d1, d2;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (CREX_VM_SPEC && OP1_TYPE == IS_CONST && OP2_TYPE == IS_CONST) {
		/* pass */
	} else if (EXPECTED(C_TYPE_INFO_P(op1) == IS_LONG)) {
		if (EXPECTED(C_TYPE_INFO_P(op2) == IS_LONG)) {
			if (EXPECTED(C_LVAL_P(op1) <= C_LVAL_P(op2))) {
CREX_VM_C_LABEL(is_smaller_or_equal_true):
				CREX_VM_SMART_BRANCH_TRUE();
				ZVAL_TRUE(EX_VAR(opline->result.var));
				CREX_VM_NEXT_OPCODE();
			} else {
CREX_VM_C_LABEL(is_smaller_or_equal_false):
				CREX_VM_SMART_BRANCH_FALSE();
				ZVAL_FALSE(EX_VAR(opline->result.var));
				CREX_VM_NEXT_OPCODE();
			}
		} else if (EXPECTED(C_TYPE_INFO_P(op2) == IS_DOUBLE)) {
			d1 = (double)C_LVAL_P(op1);
			d2 = C_DVAL_P(op2);
			CREX_VM_C_GOTO(is_smaller_or_equal_double);
		}
	} else if (EXPECTED(C_TYPE_INFO_P(op1) == IS_DOUBLE)) {
		if (EXPECTED(C_TYPE_INFO_P(op2) == IS_DOUBLE)) {
			d1 = C_DVAL_P(op1);
			d2 = C_DVAL_P(op2);
CREX_VM_C_LABEL(is_smaller_or_equal_double):
			if (d1 <= d2) {
				CREX_VM_C_GOTO(is_smaller_or_equal_true);
			} else {
				CREX_VM_C_GOTO(is_smaller_or_equal_false);
			}
		} else if (EXPECTED(C_TYPE_INFO_P(op2) == IS_LONG)) {
			d1 = C_DVAL_P(op1);
			d2 = (double)C_LVAL_P(op2);
			CREX_VM_C_GOTO(is_smaller_or_equal_double);
		}
	}
	CREX_VM_DISPATCH_TO_HELPER(crex_is_smaller_or_equal_helper, op_1, op1, op_2, op2);
}

CREX_VM_COLD_CONSTCONST_HANDLER(170, CREX_SPACESHIP, CONST|TMPVAR|CV, CONST|TMPVAR|CV)
{
	USE_OPLINE
	zval *op1, *op2;

	SAVE_OPLINE();
	op1 = GET_OP1_ZVAL_PTR(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR(BP_VAR_R);
	compare_function(EX_VAR(opline->result.var), op1, op2);
	FREE_OP1();
	FREE_OP2();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HELPER(crex_bw_or_helper, ANY, ANY, zval *op_1, zval *op_2)
{
	USE_OPLINE

	SAVE_OPLINE();
	if (UNEXPECTED(C_TYPE_INFO_P(op_1) == IS_UNDEF)) {
		op_1 = ZVAL_UNDEFINED_OP1();
	}
	if (UNEXPECTED(C_TYPE_INFO_P(op_2) == IS_UNDEF)) {
		op_2 = ZVAL_UNDEFINED_OP2();
	}
	bitwise_or_function(EX_VAR(opline->result.var), op_1, op_2);
	if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_1);
	}
	if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_2);
	}
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HOT_NOCONSTCONST_HANDLER(9, CREX_BW_OR, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(COMMUTATIVE))
{
	USE_OPLINE
	zval *op1, *op2;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (CREX_VM_SPEC && OP1_TYPE == IS_CONST && OP2_TYPE == IS_CONST) {
		/* pass */
	} else if (EXPECTED(C_TYPE_INFO_P(op1) == IS_LONG)
			&& EXPECTED(C_TYPE_INFO_P(op2) == IS_LONG)) {
		ZVAL_LONG(EX_VAR(opline->result.var), C_LVAL_P(op1) | C_LVAL_P(op2));
		CREX_VM_NEXT_OPCODE();
	}

	CREX_VM_DISPATCH_TO_HELPER(crex_bw_or_helper, op_1, op1, op_2, op2);
}

CREX_VM_HELPER(crex_bw_and_helper, ANY, ANY, zval *op_1, zval *op_2)
{
	USE_OPLINE

	SAVE_OPLINE();
	if (UNEXPECTED(C_TYPE_INFO_P(op_1) == IS_UNDEF)) {
		op_1 = ZVAL_UNDEFINED_OP1();
	}
	if (UNEXPECTED(C_TYPE_INFO_P(op_2) == IS_UNDEF)) {
		op_2 = ZVAL_UNDEFINED_OP2();
	}
	bitwise_and_function(EX_VAR(opline->result.var), op_1, op_2);
	if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_1);
	}
	if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_2);
	}
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HOT_NOCONSTCONST_HANDLER(10, CREX_BW_AND, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(COMMUTATIVE))
{
	USE_OPLINE
	zval *op1, *op2;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (CREX_VM_SPEC && OP1_TYPE == IS_CONST && OP2_TYPE == IS_CONST) {
		/* pass */
	} else if (EXPECTED(C_TYPE_INFO_P(op1) == IS_LONG)
			&& EXPECTED(C_TYPE_INFO_P(op2) == IS_LONG)) {
		ZVAL_LONG(EX_VAR(opline->result.var), C_LVAL_P(op1) & C_LVAL_P(op2));
		CREX_VM_NEXT_OPCODE();
	}

	CREX_VM_DISPATCH_TO_HELPER(crex_bw_and_helper, op_1, op1, op_2, op2);
}

CREX_VM_HELPER(crex_bw_xor_helper, ANY, ANY, zval *op_1, zval *op_2)
{
	USE_OPLINE

	SAVE_OPLINE();
	if (UNEXPECTED(C_TYPE_INFO_P(op_1) == IS_UNDEF)) {
		op_1 = ZVAL_UNDEFINED_OP1();
	}
	if (UNEXPECTED(C_TYPE_INFO_P(op_2) == IS_UNDEF)) {
		op_2 = ZVAL_UNDEFINED_OP2();
	}
	bitwise_xor_function(EX_VAR(opline->result.var), op_1, op_2);
	if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_1);
	}
	if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_2);
	}
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HOT_NOCONSTCONST_HANDLER(11, CREX_BW_XOR, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(COMMUTATIVE))
{
	USE_OPLINE
	zval *op1, *op2;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (CREX_VM_SPEC && OP1_TYPE == IS_CONST && OP2_TYPE == IS_CONST) {
		/* pass */
	} else if (EXPECTED(C_TYPE_INFO_P(op1) == IS_LONG)
			&& EXPECTED(C_TYPE_INFO_P(op2) == IS_LONG)) {
		ZVAL_LONG(EX_VAR(opline->result.var), C_LVAL_P(op1) ^ C_LVAL_P(op2));
		CREX_VM_NEXT_OPCODE();
	}

	CREX_VM_DISPATCH_TO_HELPER(crex_bw_xor_helper, op_1, op1, op_2, op2);
}

CREX_VM_COLD_CONSTCONST_HANDLER(15, CREX_BOOL_XOR, CONST|TMPVAR|CV, CONST|TMPVAR|CV, SPEC(COMMUTATIVE))
{
	USE_OPLINE
	zval *op1, *op2;

	SAVE_OPLINE();
	op1 = GET_OP1_ZVAL_PTR(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR(BP_VAR_R);
	boolean_xor_function(EX_VAR(opline->result.var), op1, op2);
	FREE_OP1();
	FREE_OP2();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HELPER(crex_bw_not_helper, ANY, ANY, zval *op_1)
{
	USE_OPLINE

	SAVE_OPLINE();
	if (UNEXPECTED(C_TYPE_P(op_1) == IS_UNDEF)) {
		op_1 = ZVAL_UNDEFINED_OP1();
	}
	bitwise_not_function(EX_VAR(opline->result.var), op_1);
	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HOT_NOCONST_HANDLER(13, CREX_BW_NOT, CONST|TMPVARCV, ANY)
{
	USE_OPLINE
	zval *op1;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (EXPECTED(C_TYPE_INFO_P(op1) == IS_LONG)) {
		ZVAL_LONG(EX_VAR(opline->result.var), ~C_LVAL_P(op1));
		CREX_VM_NEXT_OPCODE();
	}

	CREX_VM_DISPATCH_TO_HELPER(crex_bw_not_helper, op_1, op1);
}

CREX_VM_COLD_CONST_HANDLER(14, CREX_BOOL_NOT, CONST|TMPVAR|CV, ANY)
{
	USE_OPLINE
	zval *val;

	val = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (C_TYPE_INFO_P(val) == IS_TRUE) {
		ZVAL_FALSE(EX_VAR(opline->result.var));
	} else if (EXPECTED(C_TYPE_INFO_P(val) <= IS_TRUE)) {
		/* The result and op1 can be the same cv zval */
		const uint32_t orig_val_type = C_TYPE_INFO_P(val);
		ZVAL_TRUE(EX_VAR(opline->result.var));
		if (OP1_TYPE == IS_CV && UNEXPECTED(orig_val_type == IS_UNDEF)) {
			SAVE_OPLINE();
			ZVAL_UNDEFINED_OP1();
			CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
		}
	} else {
		SAVE_OPLINE();
		ZVAL_BOOL(EX_VAR(opline->result.var), !i_crex_is_true(val));
		FREE_OP1();
		CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_COLD_HELPER(crex_this_not_in_object_context_helper, ANY, ANY)
{
	USE_OPLINE

	SAVE_OPLINE();
	crex_throw_error(NULL, "Using $this when not in object context");
	UNDEF_RESULT();
	HANDLE_EXCEPTION();
}

CREX_VM_COLD_HELPER(crex_undefined_function_helper, ANY, ANY)
{
	USE_OPLINE
	zval *function_name;

	SAVE_OPLINE();
	function_name = RT_CONSTANT(opline, opline->op2);
	crex_throw_error(NULL, "Call to undefined function %s()", C_STRVAL_P(function_name));
	HANDLE_EXCEPTION();
}

CREX_VM_HANDLER(28, CREX_ASSIGN_OBJ_OP, VAR|UNUSED|THIS|CV, CONST|TMPVAR|CV, OP)
{
	USE_OPLINE
	zval *object;
	zval *property;
	zval *value;
	zval *zptr;
	void **cache_slot;
	crex_property_info *prop_info;
	crex_object *zobj;
	crex_string *name, *tmp_name;

	SAVE_OPLINE();
	object = GET_OP1_OBJ_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);
	property = GET_OP2_ZVAL_PTR(BP_VAR_R);

	do {
		value = GET_OP_DATA_ZVAL_PTR(BP_VAR_R);

		if (OP1_TYPE != IS_UNUSED && UNEXPECTED(C_TYPE_P(object) != IS_OBJECT)) {
			if (C_ISREF_P(object) && C_TYPE_P(C_REFVAL_P(object)) == IS_OBJECT) {
				object = C_REFVAL_P(object);
				CREX_VM_C_GOTO(assign_op_object);
			}
			if (OP1_TYPE == IS_CV
			 && UNEXPECTED(C_TYPE_P(object) == IS_UNDEF)) {
				ZVAL_UNDEFINED_OP1();
			}
			crex_throw_non_object_error(object, property OPLINE_CC EXECUTE_DATA_CC);
			break;
		}

CREX_VM_C_LABEL(assign_op_object):
		/* here we are sure we are dealing with an object */
		zobj = C_OBJ_P(object);
		if (OP2_TYPE == IS_CONST) {
			name = C_STR_P(property);
		} else {
			name = zval_try_get_tmp_string(property, &tmp_name);
			if (UNEXPECTED(!name)) {
				UNDEF_RESULT();
				break;
			}
		}
		cache_slot = (OP2_TYPE == IS_CONST) ? CACHE_ADDR((opline+1)->extended_value) : NULL;
		if (EXPECTED((zptr = zobj->handlers->get_property_ptr_ptr(zobj, name, BP_VAR_RW, cache_slot)) != NULL)) {
			if (UNEXPECTED(C_ISERROR_P(zptr))) {
				if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
					ZVAL_NULL(EX_VAR(opline->result.var));
				}
			} else {
				zval *orig_zptr = zptr;
				crex_reference *ref;

				do {
					if (UNEXPECTED(C_ISREF_P(zptr))) {
						ref = C_REF_P(zptr);
						zptr = C_REFVAL_P(zptr);
						if (UNEXPECTED(CREX_REF_HAS_TYPE_SOURCES(ref))) {
							crex_binary_assign_op_typed_ref(ref, value OPLINE_CC EXECUTE_DATA_CC);
							break;
						}
					}

					if (OP2_TYPE == IS_CONST) {
						prop_info = (crex_property_info*)CACHED_PTR_EX(cache_slot + 2);
					} else {
						prop_info = crex_object_fetch_property_type_info(C_OBJ_P(object), orig_zptr);
					}
					if (UNEXPECTED(prop_info)) {
						/* special case for typed properties */
						crex_binary_assign_op_typed_prop(prop_info, zptr, value OPLINE_CC EXECUTE_DATA_CC);
					} else {
						crex_binary_op(zptr, zptr, value OPLINE_CC);
					}
				} while (0);

				if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
					ZVAL_COPY(EX_VAR(opline->result.var), zptr);
				}
			}
		} else {
			crex_assign_op_overloaded_property(zobj, name, cache_slot, value OPLINE_CC EXECUTE_DATA_CC);
		}
		if (OP2_TYPE != IS_CONST) {
			crex_tmp_string_release(tmp_name);
		}
	} while (0);

	FREE_OP_DATA();
	FREE_OP2();
	FREE_OP1();
	/* assign_obj has two opcodes! */
	CREX_VM_NEXT_OPCODE_EX(1, 2);
}

/* No specialization for op_types (CONST|TMP|VAR|CV, UNUSED|CONST|TMPVAR) */
CREX_VM_HANDLER(29, CREX_ASSIGN_STATIC_PROP_OP, ANY, ANY, OP)
{
	/* This helper actually never will receive IS_VAR as second op, and has the same handling for VAR and TMP in the first op, but for interoperability with the other binary_assign_op helpers, it is necessary to "include" it */

	USE_OPLINE
	zval *prop, *value;
	crex_property_info *prop_info;
	crex_reference *ref;

	SAVE_OPLINE();

	if (UNEXPECTED(crex_fetch_static_property_address(&prop, &prop_info, (opline+1)->extended_value, BP_VAR_RW, 0 OPLINE_CC EXECUTE_DATA_CC) != SUCCESS)) {
		UNDEF_RESULT();
		FREE_OP_DATA();
		HANDLE_EXCEPTION();
	}

	value = GET_OP_DATA_ZVAL_PTR(BP_VAR_R);

	do {
		if (UNEXPECTED(C_ISREF_P(prop))) {
			ref = C_REF_P(prop);
			prop = C_REFVAL_P(prop);
			if (UNEXPECTED(CREX_REF_HAS_TYPE_SOURCES(ref))) {
				crex_binary_assign_op_typed_ref(ref, value OPLINE_CC EXECUTE_DATA_CC);
				break;
			}
		}

		if (UNEXPECTED(CREX_TYPE_IS_SET(prop_info->type))) {
			/* special case for typed properties */
			crex_binary_assign_op_typed_prop(prop_info, prop, value OPLINE_CC EXECUTE_DATA_CC);
		} else {
			crex_binary_op(prop, prop, value OPLINE_CC);
		}
	} while (0);

	if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
		ZVAL_COPY(EX_VAR(opline->result.var), prop);
	}

	FREE_OP_DATA();
	/* assign_static_prop has two opcodes! */
	CREX_VM_NEXT_OPCODE_EX(1, 2);
}

CREX_VM_HANDLER(27, CREX_ASSIGN_DIM_OP, VAR|CV, CONST|TMPVAR|UNUSED|NEXT|CV, OP)
{
	USE_OPLINE
	zval *var_ptr;
	zval *value, *container, *dim;
	HashTable *ht;

	SAVE_OPLINE();
	container = GET_OP1_OBJ_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);

	if (EXPECTED(C_TYPE_P(container) == IS_ARRAY)) {
CREX_VM_C_LABEL(assign_dim_op_array):
		SEPARATE_ARRAY(container);
		ht = C_ARRVAL_P(container);
CREX_VM_C_LABEL(assign_dim_op_new_array):
		dim = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
		if (OP2_TYPE == IS_UNUSED) {
			var_ptr = crex_hash_next_index_insert(ht, &EG(uninitialized_zval));
			if (UNEXPECTED(!var_ptr)) {
				crex_cannot_add_element();
				CREX_VM_C_GOTO(assign_dim_op_ret_null);
			}
		} else {
			if (OP2_TYPE == IS_CONST) {
				var_ptr = crex_fetch_dimension_address_inner_RW_CONST(ht, dim EXECUTE_DATA_CC);
			} else {
				var_ptr = crex_fetch_dimension_address_inner_RW(ht, dim EXECUTE_DATA_CC);
			}
			if (UNEXPECTED(!var_ptr)) {
				CREX_VM_C_GOTO(assign_dim_op_ret_null);
			}
		}

		value = get_op_data_zval_ptr_r((opline+1)->op1_type, (opline+1)->op1);

		do {
			if (OP2_TYPE != IS_UNUSED && UNEXPECTED(C_ISREF_P(var_ptr))) {
				crex_reference *ref = C_REF_P(var_ptr);
				var_ptr = C_REFVAL_P(var_ptr);
				if (UNEXPECTED(CREX_REF_HAS_TYPE_SOURCES(ref))) {
					crex_binary_assign_op_typed_ref(ref, value OPLINE_CC EXECUTE_DATA_CC);
					break;
				}
			}
			crex_binary_op(var_ptr, var_ptr, value OPLINE_CC);
		} while (0);

		if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
			ZVAL_COPY(EX_VAR(opline->result.var), var_ptr);
		}
		FREE_OP((opline+1)->op1_type, (opline+1)->op1.var);
	} else {
		if (EXPECTED(C_ISREF_P(container))) {
			container = C_REFVAL_P(container);
			if (EXPECTED(C_TYPE_P(container) == IS_ARRAY)) {
				CREX_VM_C_GOTO(assign_dim_op_array);
			}
		}

		if (EXPECTED(C_TYPE_P(container) == IS_OBJECT)) {
			crex_object *obj = C_OBJ_P(container);

			dim = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
			if (OP2_TYPE == IS_CONST && C_EXTRA_P(dim) == CREX_EXTRA_VALUE) {
				dim++;
			}
			crex_binary_assign_op_obj_dim(obj, dim OPLINE_CC EXECUTE_DATA_CC);
		} else if (EXPECTED(C_TYPE_P(container) <= IS_FALSE)) {
			uint8_t old_type;

			if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_INFO_P(container) == IS_UNDEF)) {
				ZVAL_UNDEFINED_OP1();
			}
			ht = crex_new_array(8);
			old_type = C_TYPE_P(container);
			ZVAL_ARR(container, ht);
			if (UNEXPECTED(old_type == IS_FALSE)) {
				GC_ADDREF(ht);
				crex_false_to_array_deprecated();
				if (UNEXPECTED(GC_DELREF(ht) == 0)) {
					crex_array_destroy(ht);
					CREX_VM_C_GOTO(assign_dim_op_ret_null);
				}
			}
			CREX_VM_C_GOTO(assign_dim_op_new_array);
		} else {
			dim = GET_OP2_ZVAL_PTR(BP_VAR_R);
			crex_binary_assign_op_dim_slow(container, dim OPLINE_CC EXECUTE_DATA_CC);
CREX_VM_C_LABEL(assign_dim_op_ret_null):
			FREE_OP_DATA();
			if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
				ZVAL_NULL(EX_VAR(opline->result.var));
			}
		}
	}

	FREE_OP2();
	FREE_OP1();
	CREX_VM_NEXT_OPCODE_EX(1, 2);
}

CREX_VM_HANDLER(26, CREX_ASSIGN_OP, VAR|CV, CONST|TMPVAR|CV, OP)
{
	USE_OPLINE
	zval *var_ptr;
	zval *value;

	SAVE_OPLINE();
	value = GET_OP2_ZVAL_PTR(BP_VAR_R);
	var_ptr = GET_OP1_ZVAL_PTR_PTR(BP_VAR_RW);

	do {
		if (UNEXPECTED(C_TYPE_P(var_ptr) == IS_REFERENCE)) {
			crex_reference *ref = C_REF_P(var_ptr);
			var_ptr = C_REFVAL_P(var_ptr);
			if (UNEXPECTED(CREX_REF_HAS_TYPE_SOURCES(ref))) {
				crex_binary_assign_op_typed_ref(ref, value OPLINE_CC EXECUTE_DATA_CC);
				break;
			}
		}
		crex_binary_op(var_ptr, var_ptr, value OPLINE_CC);
	} while (0);

	if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
		ZVAL_COPY(EX_VAR(opline->result.var), var_ptr);
	}

	FREE_OP2();
	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(132, CREX_PRE_INC_OBJ, VAR|UNUSED|THIS|CV, CONST|TMPVAR|CV, CACHE_SLOT)
{
	USE_OPLINE
	zval *object;
	zval *property;
	zval *zptr;
	void **cache_slot;
	crex_property_info *prop_info;
	crex_object *zobj;
	crex_string *name, *tmp_name;

	SAVE_OPLINE();
	object = GET_OP1_OBJ_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);
	property = GET_OP2_ZVAL_PTR(BP_VAR_R);

	do {
		if (OP1_TYPE != IS_UNUSED && UNEXPECTED(C_TYPE_P(object) != IS_OBJECT)) {
			if (C_ISREF_P(object) && C_TYPE_P(C_REFVAL_P(object)) == IS_OBJECT) {
				object = C_REFVAL_P(object);
				CREX_VM_C_GOTO(pre_incdec_object);
			}
			if (OP1_TYPE == IS_CV
			 && UNEXPECTED(C_TYPE_P(object) == IS_UNDEF)) {
				ZVAL_UNDEFINED_OP1();
			}
			crex_throw_non_object_error(object, property OPLINE_CC EXECUTE_DATA_CC);
			break;
		}

CREX_VM_C_LABEL(pre_incdec_object):
		/* here we are sure we are dealing with an object */
		zobj = C_OBJ_P(object);
		if (OP2_TYPE == IS_CONST) {
			name = C_STR_P(property);
		} else {
			name = zval_try_get_tmp_string(property, &tmp_name);
			if (UNEXPECTED(!name)) {
				UNDEF_RESULT();
				break;
			}
		}
		cache_slot = (OP2_TYPE == IS_CONST) ? CACHE_ADDR(opline->extended_value) : NULL;
		if (EXPECTED((zptr = zobj->handlers->get_property_ptr_ptr(zobj, name, BP_VAR_RW, cache_slot)) != NULL)) {
			if (UNEXPECTED(C_ISERROR_P(zptr))) {
				if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
					ZVAL_NULL(EX_VAR(opline->result.var));
				}
			} else {
				if (OP2_TYPE == IS_CONST) {
					prop_info = (crex_property_info *) CACHED_PTR_EX(cache_slot + 2);
				} else {
					prop_info = crex_object_fetch_property_type_info(C_OBJ_P(object), zptr);
				}
				crex_pre_incdec_property_zval(zptr, prop_info OPLINE_CC EXECUTE_DATA_CC);
			}
		} else {
			crex_pre_incdec_overloaded_property(zobj, name, cache_slot OPLINE_CC EXECUTE_DATA_CC);
		}
		if (OP2_TYPE != IS_CONST) {
			crex_tmp_string_release(tmp_name);
		}
	} while (0);

	FREE_OP2();
	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(133, CREX_PRE_DEC_OBJ, VAR|UNUSED|THIS|CV, CONST|TMPVAR|CV, CACHE_SLOT)
{
	CREX_VM_DISPATCH_TO_HANDLER(CREX_PRE_INC_OBJ);
}

CREX_VM_HANDLER(134, CREX_POST_INC_OBJ, VAR|UNUSED|THIS|CV, CONST|TMPVAR|CV, CACHE_SLOT)
{
	USE_OPLINE
	zval *object;
	zval *property;
	zval *zptr;
	void **cache_slot;
	crex_property_info *prop_info;
	crex_object *zobj;
	crex_string *name, *tmp_name;

	SAVE_OPLINE();
	object = GET_OP1_OBJ_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);
	property = GET_OP2_ZVAL_PTR(BP_VAR_R);

	do {
		if (OP1_TYPE != IS_UNUSED && UNEXPECTED(C_TYPE_P(object) != IS_OBJECT)) {
			if (C_ISREF_P(object) && C_TYPE_P(C_REFVAL_P(object)) == IS_OBJECT) {
				object = C_REFVAL_P(object);
				CREX_VM_C_GOTO(post_incdec_object);
			}
			if (OP1_TYPE == IS_CV
			 && UNEXPECTED(C_TYPE_P(object) == IS_UNDEF)) {
				ZVAL_UNDEFINED_OP1();
			}
			crex_throw_non_object_error(object, property OPLINE_CC EXECUTE_DATA_CC);
			break;
		}

CREX_VM_C_LABEL(post_incdec_object):
		/* here we are sure we are dealing with an object */
		zobj = C_OBJ_P(object);
		if (OP2_TYPE == IS_CONST) {
			name = C_STR_P(property);
		} else {
			name = zval_try_get_tmp_string(property, &tmp_name);
			if (UNEXPECTED(!name)) {
				ZVAL_UNDEF(EX_VAR(opline->result.var));
				break;
			}
		}
		cache_slot = (OP2_TYPE == IS_CONST) ? CACHE_ADDR(opline->extended_value) : NULL;
		if (EXPECTED((zptr = zobj->handlers->get_property_ptr_ptr(zobj, name, BP_VAR_RW, cache_slot)) != NULL)) {
			if (UNEXPECTED(C_ISERROR_P(zptr))) {
				ZVAL_NULL(EX_VAR(opline->result.var));
			} else {
				if (OP2_TYPE == IS_CONST) {
					prop_info = (crex_property_info*)CACHED_PTR_EX(cache_slot + 2);
				} else {
					prop_info = crex_object_fetch_property_type_info(C_OBJ_P(object), zptr);
				}

				crex_post_incdec_property_zval(zptr, prop_info OPLINE_CC EXECUTE_DATA_CC);
			}
		} else {
			crex_post_incdec_overloaded_property(zobj, name, cache_slot OPLINE_CC EXECUTE_DATA_CC);
		}
		if (OP2_TYPE != IS_CONST) {
			crex_tmp_string_release(tmp_name);
		}
	} while (0);

	FREE_OP2();
	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(135, CREX_POST_DEC_OBJ, VAR|UNUSED|THIS|CV, CONST|TMPVAR|CV, CACHE_SLOT)
{
	CREX_VM_DISPATCH_TO_HANDLER(CREX_POST_INC_OBJ);
}

/* No specialization for op_types (CONST|TMPVAR|CV, UNUSED|CONST|VAR) */
CREX_VM_HANDLER(38, CREX_PRE_INC_STATIC_PROP, ANY, ANY, CACHE_SLOT)
{
	USE_OPLINE
	zval *prop;
	crex_property_info *prop_info;

	SAVE_OPLINE();

	if (crex_fetch_static_property_address(&prop, &prop_info, opline->extended_value, BP_VAR_RW, 0 OPLINE_CC EXECUTE_DATA_CC) != SUCCESS) {
		UNDEF_RESULT();
		HANDLE_EXCEPTION();
	}

	crex_pre_incdec_property_zval(prop,
		CREX_TYPE_IS_SET(prop_info->type) ? prop_info : NULL OPLINE_CC EXECUTE_DATA_CC);

	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

/* No specialization for op_types (CONST|TMPVAR|CV, UNUSED|CONST|VAR) */
CREX_VM_HANDLER(39, CREX_PRE_DEC_STATIC_PROP, ANY, ANY, CACHE_SLOT)
{
	CREX_VM_DISPATCH_TO_HANDLER(CREX_PRE_INC_STATIC_PROP);
}

/* No specialization for op_types (CONST|TMPVAR|CV, UNUSED|CONST|VAR) */
CREX_VM_HANDLER(40, CREX_POST_INC_STATIC_PROP, ANY, ANY, CACHE_SLOT)
{
	USE_OPLINE
	zval *prop;
	crex_property_info *prop_info;

	SAVE_OPLINE();

	if (crex_fetch_static_property_address(&prop, &prop_info, opline->extended_value, BP_VAR_RW, 0 OPLINE_CC EXECUTE_DATA_CC) != SUCCESS) {
		UNDEF_RESULT();
		HANDLE_EXCEPTION();
	}

	crex_post_incdec_property_zval(prop,
		CREX_TYPE_IS_SET(prop_info->type) ? prop_info : NULL OPLINE_CC EXECUTE_DATA_CC);

	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

/* No specialization for op_types (CONST|TMPVAR|CV, UNUSED|CONST|VAR) */
CREX_VM_HANDLER(41, CREX_POST_DEC_STATIC_PROP, ANY, ANY, CACHE_SLOT)
{
	CREX_VM_DISPATCH_TO_HANDLER(CREX_POST_INC_STATIC_PROP);
}

CREX_VM_HELPER(crex_pre_inc_helper, VAR|CV, ANY)
{
	USE_OPLINE
	zval *var_ptr;

	var_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);

	SAVE_OPLINE();
	if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(var_ptr) == IS_UNDEF)) {
		ZVAL_UNDEFINED_OP1();
		ZVAL_NULL(var_ptr);
	}

	do {
		if (UNEXPECTED(C_TYPE_P(var_ptr) == IS_REFERENCE)) {
			crex_reference *ref = C_REF_P(var_ptr);
			var_ptr = C_REFVAL_P(var_ptr);
			if (UNEXPECTED(CREX_REF_HAS_TYPE_SOURCES(ref))) {
				crex_incdec_typed_ref(ref, NULL OPLINE_CC EXECUTE_DATA_CC);
				break;
			}
		}
		increment_function(var_ptr);
	} while (0);

	if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
		ZVAL_COPY(EX_VAR(opline->result.var), var_ptr);
	}

	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HOT_HANDLER(34, CREX_PRE_INC, VAR|CV, ANY, SPEC(RETVAL))
{
	USE_OPLINE
	zval *var_ptr;

	var_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);

	if (EXPECTED(C_TYPE_P(var_ptr) == IS_LONG)) {
		fast_long_increment_function(var_ptr);
		if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
			ZVAL_COPY_VALUE(EX_VAR(opline->result.var), var_ptr);
		}
		CREX_VM_NEXT_OPCODE();
	}

	CREX_VM_DISPATCH_TO_HELPER(crex_pre_inc_helper);
}

CREX_VM_HELPER(crex_pre_dec_helper, VAR|CV, ANY)
{
	USE_OPLINE
	zval *var_ptr;

	var_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);

	SAVE_OPLINE();
	if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(var_ptr) == IS_UNDEF)) {
		ZVAL_UNDEFINED_OP1();
		ZVAL_NULL(var_ptr);
	}

	do {
		if (UNEXPECTED(C_TYPE_P(var_ptr) == IS_REFERENCE)) {
			crex_reference *ref = C_REF_P(var_ptr);
			var_ptr = C_REFVAL_P(var_ptr);

			if (UNEXPECTED(CREX_REF_HAS_TYPE_SOURCES(ref))) {
				crex_incdec_typed_ref(ref, NULL OPLINE_CC EXECUTE_DATA_CC);
				break;
			}
		}
		decrement_function(var_ptr);
	} while (0);

	if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
		ZVAL_COPY(EX_VAR(opline->result.var), var_ptr);
	}

	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HOT_HANDLER(35, CREX_PRE_DEC, VAR|CV, ANY, SPEC(RETVAL))
{
	USE_OPLINE
	zval *var_ptr;

	var_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);

	if (EXPECTED(C_TYPE_P(var_ptr) == IS_LONG)) {
		fast_long_decrement_function(var_ptr);
		if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
			ZVAL_COPY_VALUE(EX_VAR(opline->result.var), var_ptr);
		}
		CREX_VM_NEXT_OPCODE();
	}

	CREX_VM_DISPATCH_TO_HELPER(crex_pre_dec_helper);
}

CREX_VM_HELPER(crex_post_inc_helper, VAR|CV, ANY)
{
	USE_OPLINE
	zval *var_ptr;

	var_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);

	SAVE_OPLINE();
	if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(var_ptr) == IS_UNDEF)) {
		ZVAL_UNDEFINED_OP1();
		ZVAL_NULL(var_ptr);
	}

	do {
		if (UNEXPECTED(C_TYPE_P(var_ptr) == IS_REFERENCE)) {
			crex_reference *ref = C_REF_P(var_ptr);
			var_ptr = C_REFVAL_P(var_ptr);

			if (UNEXPECTED(CREX_REF_HAS_TYPE_SOURCES(ref))) {
				crex_incdec_typed_ref(ref, EX_VAR(opline->result.var) OPLINE_CC EXECUTE_DATA_CC);
				break;
			}
		}
		ZVAL_COPY(EX_VAR(opline->result.var), var_ptr);

		increment_function(var_ptr);
	} while (0);

	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HOT_HANDLER(36, CREX_POST_INC, VAR|CV, ANY)
{
	USE_OPLINE
	zval *var_ptr;

	var_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);

	if (EXPECTED(C_TYPE_P(var_ptr) == IS_LONG)) {
		ZVAL_LONG(EX_VAR(opline->result.var), C_LVAL_P(var_ptr));
		fast_long_increment_function(var_ptr);
		CREX_VM_NEXT_OPCODE();
	}

	CREX_VM_DISPATCH_TO_HELPER(crex_post_inc_helper);
}

CREX_VM_HELPER(crex_post_dec_helper, VAR|CV, ANY)
{
	USE_OPLINE
	zval *var_ptr;

	var_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);

	SAVE_OPLINE();
	if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(var_ptr) == IS_UNDEF)) {
		ZVAL_UNDEFINED_OP1();
		ZVAL_NULL(var_ptr);
	}

	do {
		if (UNEXPECTED(C_TYPE_P(var_ptr) == IS_REFERENCE)) {
			crex_reference *ref = C_REF_P(var_ptr);
			var_ptr = C_REFVAL_P(var_ptr);

			if (UNEXPECTED(CREX_REF_HAS_TYPE_SOURCES(ref))) {
				crex_incdec_typed_ref(ref, EX_VAR(opline->result.var) OPLINE_CC EXECUTE_DATA_CC);
				break;
			}
		}
		ZVAL_COPY(EX_VAR(opline->result.var), var_ptr);

		decrement_function(var_ptr);
	} while (0);

	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HOT_HANDLER(37, CREX_POST_DEC, VAR|CV, ANY)
{
	USE_OPLINE
	zval *var_ptr;

	var_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);

	if (EXPECTED(C_TYPE_P(var_ptr) == IS_LONG)) {
		ZVAL_LONG(EX_VAR(opline->result.var), C_LVAL_P(var_ptr));
		fast_long_decrement_function(var_ptr);
		CREX_VM_NEXT_OPCODE();
	}

	CREX_VM_DISPATCH_TO_HELPER(crex_post_dec_helper);
}

CREX_VM_HANDLER(136, CREX_ECHO, CONST|TMPVAR|CV, ANY)
{
	USE_OPLINE
	zval *z;

	SAVE_OPLINE();
	z = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);

	if (C_TYPE_P(z) == IS_STRING) {
		crex_string *str = C_STR_P(z);

		if (ZSTR_LEN(str) != 0) {
			crex_write(ZSTR_VAL(str), ZSTR_LEN(str));
		}
	} else {
		crex_string *str = zval_get_string_func(z);

		if (ZSTR_LEN(str) != 0) {
			crex_write(ZSTR_VAL(str), ZSTR_LEN(str));
		} else if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(z) == IS_UNDEF)) {
			ZVAL_UNDEFINED_OP1();
		}
		crex_string_release_ex(str, 0);
	}

	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HELPER(crex_fetch_var_address_helper, CONST|TMPVAR|CV, UNUSED, int type)
{
	USE_OPLINE
	zval *varname;
	zval *retval;
	crex_string *name, *tmp_name;
	HashTable *target_symbol_table;

	SAVE_OPLINE();
	varname = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);

	if (OP1_TYPE == IS_CONST) {
		name = C_STR_P(varname);
	} else if (EXPECTED(C_TYPE_P(varname) == IS_STRING)) {
		name = C_STR_P(varname);
		tmp_name = NULL;
	} else {
		if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(varname) == IS_UNDEF)) {
			ZVAL_UNDEFINED_OP1();
		}
		name = zval_try_get_tmp_string(varname, &tmp_name);
		if (UNEXPECTED(!name)) {
			if (!(opline->extended_value & CREX_FETCH_GLOBAL_LOCK)) {
				FREE_OP1();
			}
			ZVAL_UNDEF(EX_VAR(opline->result.var));
			HANDLE_EXCEPTION();
		}
	}

	target_symbol_table = crex_get_target_symbol_table(opline->extended_value EXECUTE_DATA_CC);
	retval = crex_hash_find_ex(target_symbol_table, name, OP1_TYPE == IS_CONST);
	if (retval == NULL) {
		if (UNEXPECTED(crex_string_equals(name, ZSTR_KNOWN(CREX_STR_THIS)))) {
CREX_VM_C_LABEL(fetch_this):
			crex_fetch_this_var(type OPLINE_CC EXECUTE_DATA_CC);
			if (OP1_TYPE != IS_CONST) {
				crex_tmp_string_release(tmp_name);
			}
			CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
		}
		if (type == BP_VAR_W) {
			retval = crex_hash_add_new(target_symbol_table, name, &EG(uninitialized_zval));
		} else if (type == BP_VAR_IS || type == BP_VAR_UNSET) {
			retval = &EG(uninitialized_zval);
		} else {
			if (OP1_TYPE == IS_CV) {
				/* Keep name alive in case an error handler tries to free it. */
				crex_string_addref(name);
			}
			crex_error(E_WARNING, "Undefined %svariable $%s",
				(opline->extended_value & CREX_FETCH_GLOBAL ? "global " : ""), ZSTR_VAL(name));
			if (type == BP_VAR_RW && !EG(exception)) {
				retval = crex_hash_update(target_symbol_table, name, &EG(uninitialized_zval));
			} else {
				retval = &EG(uninitialized_zval);
			}
			if (OP1_TYPE == IS_CV) {
				crex_string_release(name);
			}
		}
	/* GLOBAL or $$name variable may be an INDIRECT pointer to CV */
	} else if (C_TYPE_P(retval) == IS_INDIRECT) {
		retval = C_INDIRECT_P(retval);
		if (C_TYPE_P(retval) == IS_UNDEF) {
			if (UNEXPECTED(crex_string_equals(name, ZSTR_KNOWN(CREX_STR_THIS)))) {
				CREX_VM_C_GOTO(fetch_this);
			}
			if (type == BP_VAR_W) {
				ZVAL_NULL(retval);
			} else if (type == BP_VAR_IS || type == BP_VAR_UNSET) {
				retval = &EG(uninitialized_zval);
			} else {
				crex_error(E_WARNING, "Undefined %svariable $%s",
					(opline->extended_value & CREX_FETCH_GLOBAL ? "global " : ""), ZSTR_VAL(name));
				if (type == BP_VAR_RW && !EG(exception)) {
					ZVAL_NULL(retval);
				} else {
					retval = &EG(uninitialized_zval);
				}
			}
		}
	}

	if (!(opline->extended_value & CREX_FETCH_GLOBAL_LOCK)) {
		FREE_OP1();
	}

	if (OP1_TYPE != IS_CONST) {
		crex_tmp_string_release(tmp_name);
	}

	CREX_ASSERT(retval != NULL);
	if (type == BP_VAR_R || type == BP_VAR_IS) {
		ZVAL_COPY_DEREF(EX_VAR(opline->result.var), retval);
	} else {
		ZVAL_INDIRECT(EX_VAR(opline->result.var), retval);
	}
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(80, CREX_FETCH_R, CONST|TMPVAR|CV, UNUSED, VAR_FETCH)
{
	CREX_VM_DISPATCH_TO_HELPER(crex_fetch_var_address_helper, type, BP_VAR_R);
}

CREX_VM_HANDLER(83, CREX_FETCH_W, CONST|TMPVAR|CV, UNUSED, VAR_FETCH)
{
	CREX_VM_DISPATCH_TO_HELPER(crex_fetch_var_address_helper, type, BP_VAR_W);
}

CREX_VM_HANDLER(86, CREX_FETCH_RW, CONST|TMPVAR|CV, UNUSED, VAR_FETCH)
{
	CREX_VM_DISPATCH_TO_HELPER(crex_fetch_var_address_helper, type, BP_VAR_RW);
}

CREX_VM_HANDLER(92, CREX_FETCH_FUNC_ARG, CONST|TMPVAR|CV, UNUSED, VAR_FETCH)
{
	int fetch_type =
		(UNEXPECTED(CREX_CALL_INFO(EX(call)) & CREX_CALL_SEND_ARG_BY_REF)) ?
			BP_VAR_W : BP_VAR_R;
	CREX_VM_DISPATCH_TO_HELPER(crex_fetch_var_address_helper, type, fetch_type);
}

CREX_VM_HANDLER(95, CREX_FETCH_UNSET, CONST|TMPVAR|CV, UNUSED, VAR_FETCH)
{
	CREX_VM_DISPATCH_TO_HELPER(crex_fetch_var_address_helper, type, BP_VAR_UNSET);
}

CREX_VM_HANDLER(89, CREX_FETCH_IS, CONST|TMPVAR|CV, UNUSED, VAR_FETCH)
{
	CREX_VM_DISPATCH_TO_HELPER(crex_fetch_var_address_helper, type, BP_VAR_IS);
}

/* No specialization for op_types (CONST|TMPVAR|CV, UNUSED|CONST|VAR) */
CREX_VM_HELPER(crex_fetch_static_prop_helper, ANY, ANY, int type)
{
	USE_OPLINE
	zval *prop;

	SAVE_OPLINE();

	if (UNEXPECTED(crex_fetch_static_property_address(&prop, NULL, opline->extended_value & ~CREX_FETCH_OBJ_FLAGS, type, opline->extended_value OPLINE_CC EXECUTE_DATA_CC) != SUCCESS)) {
		CREX_ASSERT(EG(exception) || (type == BP_VAR_IS));
		prop = &EG(uninitialized_zval);
	}

	if (type == BP_VAR_R || type == BP_VAR_IS) {
		ZVAL_COPY_DEREF(EX_VAR(opline->result.var), prop);
	} else {
		ZVAL_INDIRECT(EX_VAR(opline->result.var), prop);
	}
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

/* No specialization for op_types (CONST|TMPVAR|CV, UNUSED|CLASS_FETCH|CONST|VAR) */
CREX_VM_HANDLER(173, CREX_FETCH_STATIC_PROP_R, ANY, CLASS_FETCH, CACHE_SLOT)
{
	CREX_VM_DISPATCH_TO_HELPER(crex_fetch_static_prop_helper, type, BP_VAR_R);
}

/* No specialization for op_types (CONST|TMPVAR|CV, UNUSED|CLASS_FETCH|CONST|VAR) */
CREX_VM_HANDLER(174, CREX_FETCH_STATIC_PROP_W, ANY, CLASS_FETCH, FETCH_REF|DIM_WRITE|CACHE_SLOT)
{
	CREX_VM_DISPATCH_TO_HELPER(crex_fetch_static_prop_helper, type, BP_VAR_W);
}

/* No specialization for op_types (CONST|TMPVAR|CV, UNUSED|CLASS_FETCH|CONST|VAR) */
CREX_VM_HANDLER(175, CREX_FETCH_STATIC_PROP_RW, ANY, CLASS_FETCH, CACHE_SLOT)
{
	CREX_VM_DISPATCH_TO_HELPER(crex_fetch_static_prop_helper, type, BP_VAR_RW);
}

/* No specialization for op_types (CONST|TMPVAR|CV, UNUSED|CLASS_FETCH|CONST|VAR) */
CREX_VM_HANDLER(177, CREX_FETCH_STATIC_PROP_FUNC_ARG, ANY, CLASS_FETCH, FETCH_REF|CACHE_SLOT)
{
	int fetch_type =
		(UNEXPECTED(CREX_CALL_INFO(EX(call)) & CREX_CALL_SEND_ARG_BY_REF)) ?
			BP_VAR_W : BP_VAR_R;
	CREX_VM_DISPATCH_TO_HELPER(crex_fetch_static_prop_helper, type, fetch_type);
}

/* No specialization for op_types (CONST|TMPVAR|CV, UNUSED|CLASS_FETCH|CONST|VAR) */
CREX_VM_HANDLER(178, CREX_FETCH_STATIC_PROP_UNSET, ANY, CLASS_FETCH, CACHE_SLOT)
{
	CREX_VM_DISPATCH_TO_HELPER(crex_fetch_static_prop_helper, type, BP_VAR_UNSET);
}

/* No specialization for op_types (CONST|TMPVAR|CV, UNUSED|CLASS_FETCH|CONST|VAR) */
CREX_VM_HANDLER(176, CREX_FETCH_STATIC_PROP_IS, ANY, CLASS_FETCH, CACHE_SLOT)
{
	CREX_VM_DISPATCH_TO_HELPER(crex_fetch_static_prop_helper, type, BP_VAR_IS);
}

CREX_VM_COLD_CONSTCONST_HANDLER(81, CREX_FETCH_DIM_R, CONST|TMPVAR|CV, CONST|TMPVAR|CV)
{
	USE_OPLINE
	zval *container, *dim, *value;

	SAVE_OPLINE();
	container = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	dim = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (OP1_TYPE != IS_CONST) {
		if (EXPECTED(C_TYPE_P(container) == IS_ARRAY)) {
CREX_VM_C_LABEL(fetch_dim_r_array):
			value = crex_fetch_dimension_address_inner(C_ARRVAL_P(container), dim, OP2_TYPE, BP_VAR_R EXECUTE_DATA_CC);
			ZVAL_COPY_DEREF(EX_VAR(opline->result.var), value);
		} else if (EXPECTED(C_TYPE_P(container) == IS_REFERENCE)) {
			container = C_REFVAL_P(container);
			if (EXPECTED(C_TYPE_P(container) == IS_ARRAY)) {
				CREX_VM_C_GOTO(fetch_dim_r_array);
			} else {
				CREX_VM_C_GOTO(fetch_dim_r_slow);
			}
		} else {
CREX_VM_C_LABEL(fetch_dim_r_slow):
			if (OP2_TYPE == IS_CONST && C_EXTRA_P(dim) == CREX_EXTRA_VALUE) {
				dim++;
			}
			crex_fetch_dimension_address_read_R_slow(container, dim OPLINE_CC EXECUTE_DATA_CC);
		}
	} else {
		crex_fetch_dimension_address_read_R(container, dim, OP2_TYPE OPLINE_CC EXECUTE_DATA_CC);
	}
	FREE_OP2();
	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(84, CREX_FETCH_DIM_W, VAR|CV, CONST|TMPVAR|UNUSED|NEXT|CV)
{
	USE_OPLINE
	zval *container;

	SAVE_OPLINE();
	container = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_W);
	crex_fetch_dimension_address_W(container, GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R), OP2_TYPE OPLINE_CC EXECUTE_DATA_CC);
	FREE_OP2();
	if (OP1_TYPE == IS_VAR) {
		FREE_VAR_PTR_AND_EXTRACT_RESULT_IF_NECESSARY(opline->op1.var);
	}
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(87, CREX_FETCH_DIM_RW, VAR|CV, CONST|TMPVAR|UNUSED|NEXT|CV)
{
	USE_OPLINE
	zval *container;

	SAVE_OPLINE();
	container = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);
	crex_fetch_dimension_address_RW(container, GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R), OP2_TYPE OPLINE_CC EXECUTE_DATA_CC);
	FREE_OP2();
	if (OP1_TYPE == IS_VAR) {
		FREE_VAR_PTR_AND_EXTRACT_RESULT_IF_NECESSARY(opline->op1.var);
	}
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_COLD_CONSTCONST_HANDLER(90, CREX_FETCH_DIM_IS, CONST|TMPVAR|CV, CONST|TMPVAR|CV)
{
	USE_OPLINE
	zval *container;

	SAVE_OPLINE();
	container = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_IS);
	crex_fetch_dimension_address_read_IS(container, GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R), OP2_TYPE OPLINE_CC EXECUTE_DATA_CC);
	FREE_OP2();
	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_COLD_HELPER(crex_use_tmp_in_write_context_helper, ANY, ANY)
{
	USE_OPLINE

	SAVE_OPLINE();
	crex_throw_error(NULL, "Cannot use temporary expression in write context");
	FREE_OP2();
	FREE_OP1();
	ZVAL_UNDEF(EX_VAR(opline->result.var));
	HANDLE_EXCEPTION();
}

CREX_VM_COLD_HELPER(crex_use_undef_in_read_context_helper, ANY, ANY)
{
	USE_OPLINE

	SAVE_OPLINE();
	crex_throw_error(NULL, "Cannot use [] for reading");
	FREE_OP2();
	FREE_OP1();
	ZVAL_UNDEF(EX_VAR(opline->result.var));
	HANDLE_EXCEPTION();
}

CREX_VM_COLD_CONSTCONST_HANDLER(93, CREX_FETCH_DIM_FUNC_ARG, CONST|TMP|VAR|CV, CONST|TMPVAR|UNUSED|NEXT|CV)
{
#if !CREX_VM_SPEC
	USE_OPLINE
#endif

	if (UNEXPECTED(CREX_CALL_INFO(EX(call)) & CREX_CALL_SEND_ARG_BY_REF)) {
		if ((OP1_TYPE & (IS_CONST|IS_TMP_VAR))) {
			CREX_VM_DISPATCH_TO_HELPER(crex_use_tmp_in_write_context_helper);
		}
		CREX_VM_DISPATCH_TO_HANDLER(CREX_FETCH_DIM_W);
	} else {
		if (OP2_TYPE == IS_UNUSED) {
			CREX_VM_DISPATCH_TO_HELPER(crex_use_undef_in_read_context_helper);
		}
		CREX_VM_DISPATCH_TO_HANDLER(CREX_FETCH_DIM_R);
	}
}

CREX_VM_HANDLER(96, CREX_FETCH_DIM_UNSET, VAR|CV, CONST|TMPVAR|CV)
{
	USE_OPLINE
	zval *container;

	SAVE_OPLINE();
	container = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_UNSET);
	crex_fetch_dimension_address_UNSET(container, GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R), OP2_TYPE OPLINE_CC EXECUTE_DATA_CC);
	FREE_OP2();
	if (OP1_TYPE == IS_VAR) {
		FREE_VAR_PTR_AND_EXTRACT_RESULT_IF_NECESSARY(opline->op1.var);
	}
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HOT_OBJ_HANDLER(82, CREX_FETCH_OBJ_R, CONST|TMPVAR|UNUSED|THIS|CV, CONST|TMPVAR|CV, CACHE_SLOT)
{
	USE_OPLINE
	zval *container;
	void **cache_slot = NULL;

	SAVE_OPLINE();
	container = GET_OP1_OBJ_ZVAL_PTR_UNDEF(BP_VAR_R);

	if (OP1_TYPE == IS_CONST ||
	    (OP1_TYPE != IS_UNUSED && UNEXPECTED(C_TYPE_P(container) != IS_OBJECT))) {
		do {
			if ((OP1_TYPE & (IS_VAR|IS_CV)) && C_ISREF_P(container)) {
				container = C_REFVAL_P(container);
				if (EXPECTED(C_TYPE_P(container) == IS_OBJECT)) {
					break;
				}
			}
			if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(container) == IS_UNDEF)) {
				ZVAL_UNDEFINED_OP1();
			}
			crex_wrong_property_read(container, GET_OP2_ZVAL_PTR(BP_VAR_R));
			ZVAL_NULL(EX_VAR(opline->result.var));
			CREX_VM_C_GOTO(fetch_obj_r_finish);
		} while (0);
	}

	/* here we are sure we are dealing with an object */
	do {
		crex_object *zobj = C_OBJ_P(container);
		crex_string *name, *tmp_name;
		zval *retval;

		if (OP2_TYPE == IS_CONST) {
			cache_slot = CACHE_ADDR(opline->extended_value & ~CREX_FETCH_REF /* FUNC_ARG fetch may contain it */);

			if (EXPECTED(zobj->ce == CACHED_PTR_EX(cache_slot))) {
				uintptr_t prop_offset = (uintptr_t)CACHED_PTR_EX(cache_slot + 1);

				if (EXPECTED(IS_VALID_PROPERTY_OFFSET(prop_offset))) {
					retval = OBJ_PROP(zobj, prop_offset);
					if (EXPECTED(C_TYPE_INFO_P(retval) != IS_UNDEF)) {
						if (!CREX_VM_SPEC || (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) != 0) {
							CREX_VM_C_GOTO(fetch_obj_r_copy);
						} else {
CREX_VM_C_LABEL(fetch_obj_r_fast_copy):
							ZVAL_COPY_DEREF(EX_VAR(opline->result.var), retval);
							CREX_VM_NEXT_OPCODE();
						}
					}
				} else if (EXPECTED(zobj->properties != NULL)) {
					name = C_STR_P(GET_OP2_ZVAL_PTR(BP_VAR_R));
					if (!IS_UNKNOWN_DYNAMIC_PROPERTY_OFFSET(prop_offset)) {
						uintptr_t idx = CREX_DECODE_DYN_PROP_OFFSET(prop_offset);

						if (EXPECTED(idx < zobj->properties->nNumUsed * sizeof(Bucket))) {
							Bucket *p = (Bucket*)((char*)zobj->properties->arData + idx);

							if (EXPECTED(p->key == name) ||
							    (EXPECTED(p->h == ZSTR_H(name)) &&
							     EXPECTED(p->key != NULL) &&
							     EXPECTED(crex_string_equal_content(p->key, name)))) {
								retval = &p->val;
								if (!CREX_VM_SPEC || (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) != 0) {
									CREX_VM_C_GOTO(fetch_obj_r_copy);
								} else {
									CREX_VM_C_GOTO(fetch_obj_r_fast_copy);
								}
							}
						}
						CACHE_PTR_EX(cache_slot + 1, (void*)CREX_DYNAMIC_PROPERTY_OFFSET);
					}
					retval = crex_hash_find_known_hash(zobj->properties, name);
					if (EXPECTED(retval)) {
						uintptr_t idx = (char*)retval - (char*)zobj->properties->arData;
						CACHE_PTR_EX(cache_slot + 1, (void*)CREX_ENCODE_DYN_PROP_OFFSET(idx));
						if (!CREX_VM_SPEC || (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) != 0) {
							CREX_VM_C_GOTO(fetch_obj_r_copy);
						} else {
							CREX_VM_C_GOTO(fetch_obj_r_fast_copy);
						}
					}
				}
			}
			name = C_STR_P(GET_OP2_ZVAL_PTR(BP_VAR_R));
		} else {
			name = zval_try_get_tmp_string(GET_OP2_ZVAL_PTR(BP_VAR_R), &tmp_name);
			if (UNEXPECTED(!name)) {
				ZVAL_UNDEF(EX_VAR(opline->result.var));
				break;
			}
		}

#if CREX_DEBUG
		/* For non-standard object handlers, verify a declared property type in debug builds.
		 * Fetch prop_info before calling read_property(), as it may deallocate the object. */
		crex_property_info *prop_info = NULL;
		if (zobj->handlers->read_property != crex_std_read_property) {
			prop_info = crex_get_property_info(zobj->ce, name, /* silent */ true);
		}
#endif
		retval = zobj->handlers->read_property(zobj, name, BP_VAR_R, cache_slot, EX_VAR(opline->result.var));
#if CREX_DEBUG
		if (!EG(exception) && prop_info && prop_info != CREX_WRONG_PROPERTY_INFO
				&& CREX_TYPE_IS_SET(prop_info->type)) {
			ZVAL_OPT_DEREF(retval);
			crex_verify_property_type(prop_info, retval, /* strict */ true);
		}
#endif

		if (OP2_TYPE != IS_CONST) {
			crex_tmp_string_release(tmp_name);
		}

		if (retval != EX_VAR(opline->result.var)) {
CREX_VM_C_LABEL(fetch_obj_r_copy):
			ZVAL_COPY_DEREF(EX_VAR(opline->result.var), retval);
		} else if (UNEXPECTED(C_ISREF_P(retval))) {
			crex_unwrap_reference(retval);
		}
	} while (0);

CREX_VM_C_LABEL(fetch_obj_r_finish):
	FREE_OP2();
	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(85, CREX_FETCH_OBJ_W, VAR|UNUSED|THIS|CV, CONST|TMPVAR|CV, FETCH_REF|DIM_WRITE|CACHE_SLOT)
{
	USE_OPLINE
	zval *property, *container, *result;

	SAVE_OPLINE();

	container = GET_OP1_OBJ_ZVAL_PTR_PTR_UNDEF(BP_VAR_W);
	property = GET_OP2_ZVAL_PTR(BP_VAR_R);
	result = EX_VAR(opline->result.var);
	crex_fetch_property_address(
		result, container, OP1_TYPE, property, OP2_TYPE,
		((OP2_TYPE == IS_CONST) ? CACHE_ADDR(opline->extended_value & ~CREX_FETCH_OBJ_FLAGS) : NULL),
		BP_VAR_W, opline->extended_value OPLINE_CC EXECUTE_DATA_CC);
	FREE_OP2();
	if (OP1_TYPE == IS_VAR) {
		FREE_VAR_PTR_AND_EXTRACT_RESULT_IF_NECESSARY(opline->op1.var);
	}
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(88, CREX_FETCH_OBJ_RW, VAR|UNUSED|THIS|CV, CONST|TMPVAR|CV, CACHE_SLOT)
{
	USE_OPLINE
	zval *property, *container, *result;

	SAVE_OPLINE();
	container = GET_OP1_OBJ_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);
	property = GET_OP2_ZVAL_PTR(BP_VAR_R);
	result = EX_VAR(opline->result.var);
	crex_fetch_property_address(result, container, OP1_TYPE, property, OP2_TYPE, ((OP2_TYPE == IS_CONST) ? CACHE_ADDR(opline->extended_value) : NULL), BP_VAR_RW, 0 OPLINE_CC EXECUTE_DATA_CC);
	FREE_OP2();
	if (OP1_TYPE == IS_VAR) {
		FREE_VAR_PTR_AND_EXTRACT_RESULT_IF_NECESSARY(opline->op1.var);
	}
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_COLD_CONST_HANDLER(91, CREX_FETCH_OBJ_IS, CONST|TMPVAR|UNUSED|THIS|CV, CONST|TMPVAR|CV, CACHE_SLOT)
{
	USE_OPLINE
	zval *container;
	void **cache_slot = NULL;

	SAVE_OPLINE();
	container = GET_OP1_OBJ_ZVAL_PTR(BP_VAR_IS);

	if (OP1_TYPE == IS_CONST ||
	    (OP1_TYPE != IS_UNUSED && UNEXPECTED(C_TYPE_P(container) != IS_OBJECT))) {
		do {
			if ((OP1_TYPE & (IS_VAR|IS_CV)) && C_ISREF_P(container)) {
				container = C_REFVAL_P(container);
				if (EXPECTED(C_TYPE_P(container) == IS_OBJECT)) {
					break;
				}
			}
			if (OP2_TYPE == IS_CV && C_TYPE_P(EX_VAR(opline->op2.var)) == IS_UNDEF) {
				ZVAL_UNDEFINED_OP2();
			}
			ZVAL_NULL(EX_VAR(opline->result.var));
			CREX_VM_C_GOTO(fetch_obj_is_finish);
		} while (0);
	}

	/* here we are sure we are dealing with an object */
	do {
		crex_object *zobj = C_OBJ_P(container);
		crex_string *name, *tmp_name;
		zval *retval;

		if (OP2_TYPE == IS_CONST) {
			cache_slot = CACHE_ADDR(opline->extended_value);

			if (EXPECTED(zobj->ce == CACHED_PTR_EX(cache_slot))) {
				uintptr_t prop_offset = (uintptr_t)CACHED_PTR_EX(cache_slot + 1);

				if (EXPECTED(IS_VALID_PROPERTY_OFFSET(prop_offset))) {
					retval = OBJ_PROP(zobj, prop_offset);
					if (EXPECTED(C_TYPE_P(retval) != IS_UNDEF)) {
						if (!CREX_VM_SPEC || (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) != 0) {
							CREX_VM_C_GOTO(fetch_obj_is_copy);
						} else {
CREX_VM_C_LABEL(fetch_obj_is_fast_copy):
							ZVAL_COPY_DEREF(EX_VAR(opline->result.var), retval);
							CREX_VM_NEXT_OPCODE();
						}
					}
				} else if (EXPECTED(zobj->properties != NULL)) {
					name = C_STR_P(GET_OP2_ZVAL_PTR(BP_VAR_R));
					if (!IS_UNKNOWN_DYNAMIC_PROPERTY_OFFSET(prop_offset)) {
						uintptr_t idx = CREX_DECODE_DYN_PROP_OFFSET(prop_offset);

						if (EXPECTED(idx < zobj->properties->nNumUsed * sizeof(Bucket))) {
							Bucket *p = (Bucket*)((char*)zobj->properties->arData + idx);

							if (EXPECTED(p->key == name) ||
							    (EXPECTED(p->h == ZSTR_H(name)) &&
							     EXPECTED(p->key != NULL) &&
							     EXPECTED(crex_string_equal_content(p->key, name)))) {
								retval = &p->val;
								if (!CREX_VM_SPEC || (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) != 0) {
									CREX_VM_C_GOTO(fetch_obj_is_copy);
								} else {
									CREX_VM_C_GOTO(fetch_obj_is_fast_copy);
								}
							}
						}
						CACHE_PTR_EX(cache_slot + 1, (void*)CREX_DYNAMIC_PROPERTY_OFFSET);
					}
					retval = crex_hash_find_known_hash(zobj->properties, name);
					if (EXPECTED(retval)) {
						uintptr_t idx = (char*)retval - (char*)zobj->properties->arData;
						CACHE_PTR_EX(cache_slot + 1, (void*)CREX_ENCODE_DYN_PROP_OFFSET(idx));
						if (!CREX_VM_SPEC || (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) != 0) {
							CREX_VM_C_GOTO(fetch_obj_is_copy);
						} else {
							CREX_VM_C_GOTO(fetch_obj_is_fast_copy);
						}
					}
				}
			}
			name = C_STR_P(GET_OP2_ZVAL_PTR(BP_VAR_R));
		} else {
			name = zval_try_get_tmp_string(GET_OP2_ZVAL_PTR(BP_VAR_R), &tmp_name);
			if (UNEXPECTED(!name)) {
				ZVAL_UNDEF(EX_VAR(opline->result.var));
				break;
			}
		}

		retval = zobj->handlers->read_property(zobj, name, BP_VAR_IS, cache_slot, EX_VAR(opline->result.var));

		if (OP2_TYPE != IS_CONST) {
			crex_tmp_string_release(tmp_name);
		}

		if (retval != EX_VAR(opline->result.var)) {
CREX_VM_C_LABEL(fetch_obj_is_copy):
			ZVAL_COPY_DEREF(EX_VAR(opline->result.var), retval);
		} else if (UNEXPECTED(C_ISREF_P(retval))) {
			crex_unwrap_reference(retval);
		}
	} while (0);

CREX_VM_C_LABEL(fetch_obj_is_finish):
	FREE_OP2();
	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_COLD_CONST_HANDLER(94, CREX_FETCH_OBJ_FUNC_ARG, CONST|TMP|VAR|UNUSED|THIS|CV, CONST|TMPVAR|CV, FETCH_REF|CACHE_SLOT)
{
#if !CREX_VM_SPEC
	USE_OPLINE
#endif

	if (UNEXPECTED(CREX_CALL_INFO(EX(call)) & CREX_CALL_SEND_ARG_BY_REF)) {
		/* Behave like FETCH_OBJ_W */
		if ((OP1_TYPE & (IS_CONST|IS_TMP_VAR))) {
			CREX_VM_DISPATCH_TO_HELPER(crex_use_tmp_in_write_context_helper);
		}
		CREX_VM_DISPATCH_TO_HANDLER(CREX_FETCH_OBJ_W);
	} else {
		CREX_VM_DISPATCH_TO_HANDLER(CREX_FETCH_OBJ_R);
	}
}

CREX_VM_HANDLER(97, CREX_FETCH_OBJ_UNSET, VAR|UNUSED|THIS|CV, CONST|TMPVAR|CV, CACHE_SLOT)
{
	USE_OPLINE
	zval *container, *property, *result;

	SAVE_OPLINE();
	container = GET_OP1_OBJ_ZVAL_PTR_PTR_UNDEF(BP_VAR_UNSET);
	property = GET_OP2_ZVAL_PTR(BP_VAR_R);
	result = EX_VAR(opline->result.var);
	crex_fetch_property_address(result, container, OP1_TYPE, property, OP2_TYPE, ((OP2_TYPE == IS_CONST) ? CACHE_ADDR(opline->extended_value) : NULL), BP_VAR_UNSET, 0 OPLINE_CC EXECUTE_DATA_CC);
	FREE_OP2();
	if (OP1_TYPE == IS_VAR) {
		FREE_VAR_PTR_AND_EXTRACT_RESULT_IF_NECESSARY(opline->op1.var);
	}
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(98, CREX_FETCH_LIST_R, CONST|TMPVARCV, CONST|TMPVAR|CV)
{
	USE_OPLINE
	zval *container;

	SAVE_OPLINE();
	container = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	crex_fetch_dimension_address_LIST_r(container, GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R), OP2_TYPE OPLINE_CC EXECUTE_DATA_CC);
	FREE_OP2();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(155, CREX_FETCH_LIST_W, VAR, CONST|TMPVAR|CV)
{
	USE_OPLINE
	zval *container, *dim;

	SAVE_OPLINE();
	container = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_W);
	dim = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);

	if (OP1_TYPE == IS_VAR
		&& C_TYPE_P(EX_VAR(opline->op1.var)) != IS_INDIRECT
		&& UNEXPECTED(!C_ISREF_P(container))
	) {
		crex_error(E_NOTICE, "Attempting to set reference to non referenceable value");
		crex_fetch_dimension_address_LIST_r(container, dim, OP2_TYPE OPLINE_CC EXECUTE_DATA_CC);
	} else {
		crex_fetch_dimension_address_W(container, dim, OP2_TYPE OPLINE_CC EXECUTE_DATA_CC);
	}

	FREE_OP2();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(24, CREX_ASSIGN_OBJ, VAR|UNUSED|THIS|CV, CONST|TMPVAR|CV, CACHE_SLOT, SPEC(OP_DATA=CONST|TMP|VAR|CV))
{
	USE_OPLINE
	zval *object, *value, tmp;
	crex_object *zobj;
	crex_string *name, *tmp_name;
	crex_refcounted *garbage = NULL;

	SAVE_OPLINE();
	object = GET_OP1_OBJ_ZVAL_PTR_PTR_UNDEF(BP_VAR_W);
	value = GET_OP_DATA_ZVAL_PTR(BP_VAR_R);

	if (OP1_TYPE != IS_UNUSED && UNEXPECTED(C_TYPE_P(object) != IS_OBJECT)) {
		if (C_ISREF_P(object) && C_TYPE_P(C_REFVAL_P(object)) == IS_OBJECT) {
			object = C_REFVAL_P(object);
			CREX_VM_C_GOTO(assign_object);
		}
		crex_throw_non_object_error(object, GET_OP2_ZVAL_PTR(BP_VAR_R) OPLINE_CC EXECUTE_DATA_CC);
		value = &EG(uninitialized_zval);
		CREX_VM_C_GOTO(free_and_exit_assign_obj);
	}

CREX_VM_C_LABEL(assign_object):
	zobj = C_OBJ_P(object);
	if (OP2_TYPE == IS_CONST) {
		if (EXPECTED(zobj->ce == CACHED_PTR(opline->extended_value))) {
			void **cache_slot = CACHE_ADDR(opline->extended_value);
			uintptr_t prop_offset = (uintptr_t)CACHED_PTR_EX(cache_slot + 1);
			zval *property_val;

			if (EXPECTED(IS_VALID_PROPERTY_OFFSET(prop_offset))) {
				property_val = OBJ_PROP(zobj, prop_offset);
				if (C_TYPE_P(property_val) != IS_UNDEF) {
					crex_property_info *prop_info = (crex_property_info*) CACHED_PTR_EX(cache_slot + 2);

					if (UNEXPECTED(prop_info != NULL)) {
						value = crex_assign_to_typed_prop(prop_info, property_val, value, &garbage EXECUTE_DATA_CC);
						CREX_VM_C_GOTO(free_and_exit_assign_obj);
					} else {
CREX_VM_C_LABEL(fast_assign_obj):
						value = crex_assign_to_variable_ex(property_val, value, OP_DATA_TYPE, EX_USES_STRICT_TYPES(), &garbage);
						if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
							ZVAL_COPY(EX_VAR(opline->result.var), value);
						}
						CREX_VM_C_GOTO(exit_assign_obj);
					}
				}
			} else {
				name = C_STR_P(GET_OP2_ZVAL_PTR(BP_VAR_R));
				if (EXPECTED(zobj->properties != NULL)) {
					if (UNEXPECTED(GC_REFCOUNT(zobj->properties) > 1)) {
						if (EXPECTED(!(GC_FLAGS(zobj->properties) & IS_ARRAY_IMMUTABLE))) {
							GC_DELREF(zobj->properties);
						}
						zobj->properties = crex_array_dup(zobj->properties);
					}
					property_val = crex_hash_find_known_hash(zobj->properties, name);
					if (property_val) {
						CREX_VM_C_GOTO(fast_assign_obj);
					}
				}

				if (!zobj->ce->__set && (zobj->ce->ce_flags & CREX_ACC_ALLOW_DYNAMIC_PROPERTIES)) {
					if (EXPECTED(zobj->properties == NULL)) {
						rebuild_object_properties(zobj);
					}
					if (OP_DATA_TYPE == IS_CONST) {
						if (UNEXPECTED(C_OPT_REFCOUNTED_P(value))) {
							C_ADDREF_P(value);
						}
					} else if (OP_DATA_TYPE != IS_TMP_VAR) {
						if (C_ISREF_P(value)) {
							if (OP_DATA_TYPE == IS_VAR) {
								crex_reference *ref = C_REF_P(value);
								if (GC_DELREF(ref) == 0) {
									ZVAL_COPY_VALUE(&tmp, C_REFVAL_P(value));
									efree_size(ref, sizeof(crex_reference));
									value = &tmp;
								} else {
									value = C_REFVAL_P(value);
									C_TRY_ADDREF_P(value);
								}
							} else {
								value = C_REFVAL_P(value);
								C_TRY_ADDREF_P(value);
							}
						} else if (OP_DATA_TYPE == IS_CV) {
							C_TRY_ADDREF_P(value);
						}
						}
					crex_hash_add_new(zobj->properties, name, value);
					if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
						ZVAL_COPY(EX_VAR(opline->result.var), value);
					}
					CREX_VM_C_GOTO(exit_assign_obj);
				}
			}
		}
		name = C_STR_P(GET_OP2_ZVAL_PTR(BP_VAR_R));
	} else {
		name = zval_try_get_tmp_string(GET_OP2_ZVAL_PTR(BP_VAR_R), &tmp_name);
		if (UNEXPECTED(!name)) {
			FREE_OP_DATA();
			UNDEF_RESULT();
			CREX_VM_C_GOTO(exit_assign_obj);
		}
	}

	if (OP_DATA_TYPE == IS_CV || OP_DATA_TYPE == IS_VAR) {
		ZVAL_DEREF(value);
	}

	value = zobj->handlers->write_property(zobj, name, value, (OP2_TYPE == IS_CONST) ? CACHE_ADDR(opline->extended_value) : NULL);

	if (OP2_TYPE != IS_CONST) {
		crex_tmp_string_release(tmp_name);
	}

CREX_VM_C_LABEL(free_and_exit_assign_obj):
	if (UNEXPECTED(RETURN_VALUE_USED(opline)) && value) {
		ZVAL_COPY_DEREF(EX_VAR(opline->result.var), value);
	}
	FREE_OP_DATA();
CREX_VM_C_LABEL(exit_assign_obj):
	if (garbage) {
		GC_DTOR_NO_REF(garbage);
	}
	FREE_OP2();
	FREE_OP1();
	/* assign_obj has two opcodes! */
	CREX_VM_NEXT_OPCODE_EX(1, 2);
}

/* No specialization for op_types (CONST|TMPVAR|CV, UNUSED|CONST|VAR) */
CREX_VM_HANDLER(25, CREX_ASSIGN_STATIC_PROP, ANY, ANY, CACHE_SLOT, SPEC(OP_DATA=CONST|TMP|VAR|CV))
{
	USE_OPLINE
	zval *prop, *value;
	crex_property_info *prop_info;
	crex_refcounted *garbage = NULL;

	SAVE_OPLINE();

	if (crex_fetch_static_property_address(&prop, &prop_info, opline->extended_value, BP_VAR_W, 0 OPLINE_CC EXECUTE_DATA_CC) != SUCCESS) {
		FREE_OP_DATA();
		UNDEF_RESULT();
		HANDLE_EXCEPTION();
	}

	value = GET_OP_DATA_ZVAL_PTR(BP_VAR_R);

	if (UNEXPECTED(CREX_TYPE_IS_SET(prop_info->type))) {
		value = crex_assign_to_typed_prop(prop_info, prop, value, &garbage EXECUTE_DATA_CC);
		FREE_OP_DATA();
	} else {
		value = crex_assign_to_variable_ex(prop, value, OP_DATA_TYPE, EX_USES_STRICT_TYPES(), &garbage);
	}

	if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
		ZVAL_COPY(EX_VAR(opline->result.var), value);
	}

	if (garbage) {
		GC_DTOR_NO_REF(garbage);
	}

	/* assign_static_prop has two opcodes! */
	CREX_VM_NEXT_OPCODE_EX(1, 2);
}

CREX_VM_HANDLER(23, CREX_ASSIGN_DIM, VAR|CV, CONST|TMPVAR|UNUSED|NEXT|CV, SPEC(OP_DATA=CONST|TMP|VAR|CV))
{
	USE_OPLINE
	zval *object_ptr, *orig_object_ptr;
	zval *value;
	zval *variable_ptr;
	zval *dim;
	crex_refcounted *garbage = NULL;

	SAVE_OPLINE();
	orig_object_ptr = object_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_W);

	if (EXPECTED(C_TYPE_P(object_ptr) == IS_ARRAY)) {
CREX_VM_C_LABEL(try_assign_dim_array):
		SEPARATE_ARRAY(object_ptr);
		if (OP2_TYPE == IS_UNUSED) {
			value = GET_OP_DATA_ZVAL_PTR_UNDEF(BP_VAR_R);
			if (OP_DATA_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(value) == IS_UNDEF)) {
				HashTable *ht = C_ARRVAL_P(object_ptr);
				if (!(GC_FLAGS(ht) & IS_ARRAY_IMMUTABLE)) {
					GC_ADDREF(ht);
				}
				value = zval_undefined_cv((opline+1)->op1.var EXECUTE_DATA_CC);
				if (!(GC_FLAGS(ht) & IS_ARRAY_IMMUTABLE) && !GC_DELREF(ht)) {
					crex_array_destroy(ht);
					CREX_VM_C_GOTO(assign_dim_error);
				}
			}
			if (OP_DATA_TYPE == IS_CV || OP_DATA_TYPE == IS_VAR) {
				ZVAL_DEREF(value);
			}
			value = crex_hash_next_index_insert(C_ARRVAL_P(object_ptr), value);
			if (UNEXPECTED(value == NULL)) {
				crex_cannot_add_element();
				CREX_VM_C_GOTO(assign_dim_error);
			} else if (OP_DATA_TYPE == IS_CV) {
				if (C_REFCOUNTED_P(value)) {
					C_ADDREF_P(value);
				}
			} else if (OP_DATA_TYPE == IS_VAR) {
				zval *free_op_data = EX_VAR((opline+1)->op1.var);
				if (C_ISREF_P(free_op_data)) {
					if (C_REFCOUNTED_P(value)) {
						C_ADDREF_P(value);
					}
					zval_ptr_dtor_nogc(free_op_data);
				}
			} else if (OP_DATA_TYPE == IS_CONST) {
				if (UNEXPECTED(C_REFCOUNTED_P(value))) {
					C_ADDREF_P(value);
				}
			}
		} else {
			dim = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
			if (OP2_TYPE == IS_CONST) {
				variable_ptr = crex_fetch_dimension_address_inner_W_CONST(C_ARRVAL_P(object_ptr), dim EXECUTE_DATA_CC);
			} else {
				variable_ptr = crex_fetch_dimension_address_inner_W(C_ARRVAL_P(object_ptr), dim EXECUTE_DATA_CC);
			}
			if (UNEXPECTED(variable_ptr == NULL)) {
				CREX_VM_C_GOTO(assign_dim_error);
			}
			value = GET_OP_DATA_ZVAL_PTR(BP_VAR_R);
			value = crex_assign_to_variable_ex(variable_ptr, value, OP_DATA_TYPE, EX_USES_STRICT_TYPES(), &garbage);
		}
		if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
			ZVAL_COPY(EX_VAR(opline->result.var), value);
		}
		if (garbage) {
			GC_DTOR_NO_REF(garbage);
		}
	} else {
		if (EXPECTED(C_ISREF_P(object_ptr))) {
			object_ptr = C_REFVAL_P(object_ptr);
			if (EXPECTED(C_TYPE_P(object_ptr) == IS_ARRAY)) {
				CREX_VM_C_GOTO(try_assign_dim_array);
			}
		}
		if (EXPECTED(C_TYPE_P(object_ptr) == IS_OBJECT)) {
			crex_object *obj = C_OBJ_P(object_ptr);

			GC_ADDREF(obj);
			dim = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
			if (OP2_TYPE == IS_CV && UNEXPECTED(C_ISUNDEF_P(dim))) {
				dim = ZVAL_UNDEFINED_OP2();
			} else if (OP2_TYPE == IS_CONST && C_EXTRA_P(dim) == CREX_EXTRA_VALUE) {
				dim++;
			}

			value = GET_OP_DATA_ZVAL_PTR_UNDEF(BP_VAR_R);
			if (OP_DATA_TYPE == IS_CV && UNEXPECTED(C_ISUNDEF_P(value))) {
				value = zval_undefined_cv((opline+1)->op1.var EXECUTE_DATA_CC);
			} else if (OP_DATA_TYPE & (IS_CV|IS_VAR)) {
				ZVAL_DEREF(value);
			}

			crex_assign_to_object_dim(obj, dim, value OPLINE_CC EXECUTE_DATA_CC);

			FREE_OP_DATA();
			if (UNEXPECTED(GC_DELREF(obj) == 0)) {
				crex_objects_store_del(obj);
			}
		} else if (EXPECTED(C_TYPE_P(object_ptr) == IS_STRING)) {
			if (OP2_TYPE == IS_UNUSED) {
				crex_use_new_element_for_string();
				FREE_OP_DATA();
				UNDEF_RESULT();
			} else {
				dim = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
				value = GET_OP_DATA_ZVAL_PTR_UNDEF(BP_VAR_R);
				crex_assign_to_string_offset(object_ptr, dim, value OPLINE_CC EXECUTE_DATA_CC);
				FREE_OP_DATA();
			}
		} else if (EXPECTED(C_TYPE_P(object_ptr) <= IS_FALSE)) {
			if (C_ISREF_P(orig_object_ptr)
			 && CREX_REF_HAS_TYPE_SOURCES(C_REF_P(orig_object_ptr))
			 && !crex_verify_ref_array_assignable(C_REF_P(orig_object_ptr))) {
				dim = GET_OP2_ZVAL_PTR(BP_VAR_R);
				FREE_OP_DATA();
				UNDEF_RESULT();
			} else {
				HashTable *ht = crex_new_array(8);
				uint8_t old_type = C_TYPE_P(object_ptr);

				ZVAL_ARR(object_ptr, ht);
				if (UNEXPECTED(old_type == IS_FALSE)) {
					GC_ADDREF(ht);
					crex_false_to_array_deprecated();
					if (UNEXPECTED(GC_DELREF(ht) == 0)) {
						crex_array_destroy(ht);
						CREX_VM_C_GOTO(assign_dim_error);
					}
				}
				CREX_VM_C_GOTO(try_assign_dim_array);
			}
		} else {
			crex_use_scalar_as_array();
			dim = GET_OP2_ZVAL_PTR(BP_VAR_R);
CREX_VM_C_LABEL(assign_dim_error):
			FREE_OP_DATA();
			if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
				ZVAL_NULL(EX_VAR(opline->result.var));
			}
		}
	}
	if (OP2_TYPE != IS_UNUSED) {
		FREE_OP2();
	}
	FREE_OP1();
	/* assign_dim has two opcodes! */
	CREX_VM_NEXT_OPCODE_EX(1, 2);
}

CREX_VM_HANDLER(22, CREX_ASSIGN, VAR|CV, CONST|TMP|VAR|CV, SPEC(RETVAL))
{
	USE_OPLINE
	zval *value;
	zval *variable_ptr;

	SAVE_OPLINE();
	value = GET_OP2_ZVAL_PTR(BP_VAR_R);
	variable_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_W);

	if (!CREX_VM_SPEC || UNEXPECTED(RETURN_VALUE_USED(opline))) {
		crex_refcounted *garbage = NULL;

		value = crex_assign_to_variable_ex(variable_ptr, value, OP2_TYPE, EX_USES_STRICT_TYPES(), &garbage);
		if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
			ZVAL_COPY(EX_VAR(opline->result.var), value);
		}
		if (garbage) {
			GC_DTOR_NO_REF(garbage);
		}
	} else {
		value = crex_assign_to_variable(variable_ptr, value, OP2_TYPE, EX_USES_STRICT_TYPES());
	}
	FREE_OP1();
	/* crex_assign_to_variable() always takes care of op2, never free it! */

	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(30, CREX_ASSIGN_REF, VAR|CV, VAR|CV, SRC)
{
	USE_OPLINE
	zval *variable_ptr;
	zval *value_ptr;
	crex_refcounted *garbage = NULL;

	SAVE_OPLINE();
	value_ptr = GET_OP2_ZVAL_PTR_PTR(BP_VAR_W);
	variable_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_W);

	if (OP1_TYPE == IS_VAR &&
	           UNEXPECTED(C_TYPE_P(EX_VAR(opline->op1.var)) != IS_INDIRECT)) {

		crex_throw_error(NULL, "Cannot assign by reference to an array dimension of an object");
		variable_ptr = &EG(uninitialized_zval);
	} else if (OP2_TYPE == IS_VAR &&
	           opline->extended_value == CREX_RETURNS_FUNCTION &&
			   UNEXPECTED(!C_ISREF_P(value_ptr))) {

		variable_ptr = crex_wrong_assign_to_variable_reference(
			variable_ptr, value_ptr, &garbage OPLINE_CC EXECUTE_DATA_CC);
	} else {
		crex_assign_to_variable_reference(variable_ptr, value_ptr, &garbage);
	}

	if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
		ZVAL_COPY(EX_VAR(opline->result.var), variable_ptr);
	}

	if (garbage) {
		GC_DTOR(garbage);
	}

	FREE_OP2();
	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(32, CREX_ASSIGN_OBJ_REF, VAR|UNUSED|THIS|CV, CONST|TMPVAR|CV, CACHE_SLOT|SRC, SPEC(OP_DATA=VAR|CV))
{
	USE_OPLINE
	zval *property, *container, *value_ptr;

	SAVE_OPLINE();

	container = GET_OP1_OBJ_ZVAL_PTR_PTR_UNDEF(BP_VAR_W);
	property = GET_OP2_ZVAL_PTR(BP_VAR_R);

	value_ptr = GET_OP_DATA_ZVAL_PTR_PTR(BP_VAR_W);

	if (CREX_VM_SPEC) {
		if (OP1_TYPE == IS_UNUSED) {
			if (OP2_TYPE == IS_CONST) {
				crex_assign_to_property_reference_this_const(container, property, value_ptr OPLINE_CC EXECUTE_DATA_CC);
			} else {
				crex_assign_to_property_reference_this_var(container, property, value_ptr OPLINE_CC EXECUTE_DATA_CC);
			}
		} else {
			if (OP2_TYPE == IS_CONST) {
				crex_assign_to_property_reference_var_const(container, property, value_ptr OPLINE_CC EXECUTE_DATA_CC);
			} else {
				crex_assign_to_property_reference_var_var(container, property, value_ptr OPLINE_CC EXECUTE_DATA_CC);
			}
		}
	} else {
		crex_assign_to_property_reference(container, OP1_TYPE, property, OP2_TYPE, value_ptr OPLINE_CC EXECUTE_DATA_CC);
	}

	FREE_OP1();
	FREE_OP2();
	FREE_OP_DATA();
	CREX_VM_NEXT_OPCODE_EX(1, 2);
}

/* No specialization for op_types (CONST|TMPVAR|CV, UNUSED|CONST|VAR) */
CREX_VM_HANDLER(33, CREX_ASSIGN_STATIC_PROP_REF, ANY, ANY, CACHE_SLOT|SRC)
{
	USE_OPLINE
	zval *prop, *value_ptr;
	crex_property_info *prop_info;
	crex_refcounted *garbage = NULL;

	SAVE_OPLINE();

	if (crex_fetch_static_property_address(&prop, &prop_info, opline->extended_value & ~CREX_RETURNS_FUNCTION, BP_VAR_W, 0 OPLINE_CC EXECUTE_DATA_CC) != SUCCESS) {
		FREE_OP_DATA();
		UNDEF_RESULT();
		HANDLE_EXCEPTION();
	}

	value_ptr = GET_OP_DATA_ZVAL_PTR_PTR(BP_VAR_W);

	if (OP_DATA_TYPE == IS_VAR && (opline->extended_value & CREX_RETURNS_FUNCTION) && UNEXPECTED(!C_ISREF_P(value_ptr))) {
		if (UNEXPECTED(!crex_wrong_assign_to_variable_reference(prop, value_ptr, &garbage OPLINE_CC EXECUTE_DATA_CC))) {
			prop = &EG(uninitialized_zval);
		}
	} else if (UNEXPECTED(CREX_TYPE_IS_SET(prop_info->type))) {
		prop = crex_assign_to_typed_property_reference(prop_info, prop, value_ptr, &garbage EXECUTE_DATA_CC);
	} else {
		crex_assign_to_variable_reference(prop, value_ptr, &garbage);
	}

	if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
		ZVAL_COPY(EX_VAR(opline->result.var), prop);
	}

	if (garbage) {
		GC_DTOR(garbage);
	}

	FREE_OP_DATA();
	CREX_VM_NEXT_OPCODE_EX(1, 2);
}

CREX_VM_HOT_HELPER(crex_leave_helper, ANY, ANY)
{
	crex_execute_data *old_execute_data;
	uint32_t call_info = EX_CALL_INFO();
	SAVE_OPLINE();

	if (EXPECTED((call_info & (CREX_CALL_CODE|CREX_CALL_TOP|CREX_CALL_HAS_SYMBOL_TABLE|CREX_CALL_FREE_EXTRA_ARGS|CREX_CALL_ALLOCATED|CREX_CALL_HAS_EXTRA_NAMED_PARAMS)) == 0)) {
		EG(current_execute_data) = EX(prev_execute_data);
		i_free_compiled_variables(execute_data);

#ifdef CREX_PREFER_RELOAD
		call_info = EX_CALL_INFO();
#endif
		if (UNEXPECTED(call_info & CREX_CALL_RELEASE_THIS)) {
			OBJ_RELEASE(C_OBJ(execute_data->This));
		} else if (UNEXPECTED(call_info & CREX_CALL_CLOSURE)) {
			OBJ_RELEASE(CREX_CLOSURE_OBJECT(EX(func)));
		}
		EG(vm_stack_top) = (zval*)execute_data;
		execute_data = EX(prev_execute_data);

		if (UNEXPECTED(EG(exception) != NULL)) {
			crex_rethrow_exception(execute_data);
			HANDLE_EXCEPTION_LEAVE();
		}

		LOAD_NEXT_OPLINE();
		CREX_VM_LEAVE();
	} else if (EXPECTED((call_info & (CREX_CALL_CODE|CREX_CALL_TOP)) == 0)) {
		EG(current_execute_data) = EX(prev_execute_data);
		i_free_compiled_variables(execute_data);

#ifdef CREX_PREFER_RELOAD
		call_info = EX_CALL_INFO();
#endif
		if (UNEXPECTED(call_info & CREX_CALL_HAS_SYMBOL_TABLE)) {
			crex_clean_and_cache_symbol_table(EX(symbol_table));
		}

		if (UNEXPECTED(call_info & CREX_CALL_HAS_EXTRA_NAMED_PARAMS)) {
			crex_free_extra_named_params(EX(extra_named_params));
		}

		/* Free extra args before releasing the closure,
		 * as that may free the op_array. */
		crex_vm_stack_free_extra_args_ex(call_info, execute_data);

		if (UNEXPECTED(call_info & CREX_CALL_RELEASE_THIS)) {
			OBJ_RELEASE(C_OBJ(execute_data->This));
		} else if (UNEXPECTED(call_info & CREX_CALL_CLOSURE)) {
			OBJ_RELEASE(CREX_CLOSURE_OBJECT(EX(func)));
		}

		old_execute_data = execute_data;
		execute_data = EX(prev_execute_data);
		crex_vm_stack_free_call_frame_ex(call_info, old_execute_data);

		if (UNEXPECTED(EG(exception) != NULL)) {
			crex_rethrow_exception(execute_data);
			HANDLE_EXCEPTION_LEAVE();
		}

		LOAD_NEXT_OPLINE();
		CREX_VM_LEAVE();
	} else if (EXPECTED((call_info & CREX_CALL_TOP) == 0)) {
		if (EX(func)->op_array.last_var > 0) {
			crex_detach_symbol_table(execute_data);
			call_info |= CREX_CALL_NEEDS_REATTACH;
		}
		crex_destroy_static_vars(&EX(func)->op_array);
		destroy_op_array(&EX(func)->op_array);
		efree_size(EX(func), sizeof(crex_op_array));
		old_execute_data = execute_data;
		execute_data = EG(current_execute_data) = EX(prev_execute_data);
		crex_vm_stack_free_call_frame_ex(call_info, old_execute_data);

		if (call_info & CREX_CALL_NEEDS_REATTACH) {
			if (EX(func)->op_array.last_var > 0) {
				crex_attach_symbol_table(execute_data);
			} else {
				CREX_ADD_CALL_FLAG(execute_data, CREX_CALL_NEEDS_REATTACH);
			}
		}
		if (UNEXPECTED(EG(exception) != NULL)) {
			crex_rethrow_exception(execute_data);
			HANDLE_EXCEPTION_LEAVE();
		}

		LOAD_NEXT_OPLINE();
		CREX_VM_LEAVE();
	} else {
		if (EXPECTED((call_info & CREX_CALL_CODE) == 0)) {
			EG(current_execute_data) = EX(prev_execute_data);
			i_free_compiled_variables(execute_data);
#ifdef CREX_PREFER_RELOAD
			call_info = EX_CALL_INFO();
#endif
			if (UNEXPECTED(call_info & (CREX_CALL_HAS_SYMBOL_TABLE|CREX_CALL_FREE_EXTRA_ARGS|CREX_CALL_HAS_EXTRA_NAMED_PARAMS))) {
				if (UNEXPECTED(call_info & CREX_CALL_HAS_SYMBOL_TABLE)) {
					crex_clean_and_cache_symbol_table(EX(symbol_table));
				}
				crex_vm_stack_free_extra_args_ex(call_info, execute_data);
				if (UNEXPECTED(call_info & CREX_CALL_HAS_EXTRA_NAMED_PARAMS)) {
					crex_free_extra_named_params(EX(extra_named_params));
				}
			}
			if (UNEXPECTED(call_info & CREX_CALL_CLOSURE)) {
				OBJ_RELEASE(CREX_CLOSURE_OBJECT(EX(func)));
			}
			CREX_VM_RETURN();
		} else /* if (call_kind == CREX_CALL_TOP_CODE) */ {
			crex_array *symbol_table = EX(symbol_table);

			if (EX(func)->op_array.last_var > 0) {
				crex_detach_symbol_table(execute_data);
				call_info |= CREX_CALL_NEEDS_REATTACH;
			}
			if (call_info & CREX_CALL_NEEDS_REATTACH) {
				old_execute_data = EX(prev_execute_data);
				while (old_execute_data) {
					if (old_execute_data->func && (CREX_CALL_INFO(old_execute_data) & CREX_CALL_HAS_SYMBOL_TABLE)) {
						if (old_execute_data->symbol_table == symbol_table) {
							if (old_execute_data->func->op_array.last_var > 0) {
								crex_attach_symbol_table(old_execute_data);
							} else {
								CREX_ADD_CALL_FLAG(old_execute_data, CREX_CALL_NEEDS_REATTACH);
							}
						}
						break;
					}
					old_execute_data = old_execute_data->prev_execute_data;
				}
			}
			EG(current_execute_data) = EX(prev_execute_data);
			CREX_VM_RETURN();
		}
	}
}

CREX_VM_HOT_HANDLER(42, CREX_JMP, JMP_ADDR, ANY)
{
	USE_OPLINE

	CREX_VM_JMP_EX(OP_JMP_ADDR(opline, opline->op1), 0);
}

CREX_VM_HOT_NOCONST_HANDLER(43, CREX_JMPZ, CONST|TMPVAR|CV, JMP_ADDR)
{
	USE_OPLINE
	zval *val;
	uint8_t op1_type;

	val = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);

	if (C_TYPE_INFO_P(val) == IS_TRUE) {
		CREX_VM_NEXT_OPCODE();
	} else if (EXPECTED(C_TYPE_INFO_P(val) <= IS_TRUE)) {
		if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_INFO_P(val) == IS_UNDEF)) {
			SAVE_OPLINE();
			ZVAL_UNDEFINED_OP1();
			if (UNEXPECTED(EG(exception))) {
				HANDLE_EXCEPTION();
			}
		}
		CREX_VM_JMP_EX(OP_JMP_ADDR(opline, opline->op2), 0);
	}

	SAVE_OPLINE();
	op1_type = OP1_TYPE;
	if (i_crex_is_true(val)) {
		opline++;
	} else {
		opline = OP_JMP_ADDR(opline, opline->op2);
	}
	if (op1_type & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(val);
	}
	CREX_VM_JMP(opline);
}

CREX_VM_HOT_NOCONST_HANDLER(44, CREX_JMPNZ, CONST|TMPVAR|CV, JMP_ADDR)
{
	USE_OPLINE
	zval *val;
	uint8_t op1_type;

	val = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);

	if (C_TYPE_INFO_P(val) == IS_TRUE) {
		CREX_VM_JMP_EX(OP_JMP_ADDR(opline, opline->op2), 0);
	} else if (EXPECTED(C_TYPE_INFO_P(val) <= IS_TRUE)) {
		if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_INFO_P(val) == IS_UNDEF)) {
			SAVE_OPLINE();
			ZVAL_UNDEFINED_OP1();
			if (UNEXPECTED(EG(exception))) {
				HANDLE_EXCEPTION();
			}
		}
		CREX_VM_NEXT_OPCODE();
	}

	SAVE_OPLINE();
	op1_type = OP1_TYPE;
	if (i_crex_is_true(val)) {
		opline = OP_JMP_ADDR(opline, opline->op2);
	} else {
		opline++;
	}
	if (op1_type & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(val);
	}
	CREX_VM_JMP(opline);
}

CREX_VM_COLD_CONST_HANDLER(46, CREX_JMPC_EX, CONST|TMPVAR|CV, JMP_ADDR)
{
	USE_OPLINE
	zval *val;
	bool ret;

	val = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);

	if (C_TYPE_INFO_P(val) == IS_TRUE) {
		ZVAL_TRUE(EX_VAR(opline->result.var));
		CREX_VM_NEXT_OPCODE();
	} else if (EXPECTED(C_TYPE_INFO_P(val) <= IS_TRUE)) {
		ZVAL_FALSE(EX_VAR(opline->result.var));
		if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_INFO_P(val) == IS_UNDEF)) {
			SAVE_OPLINE();
			ZVAL_UNDEFINED_OP1();
			if (UNEXPECTED(EG(exception))) {
				HANDLE_EXCEPTION();
			}
		}
		CREX_VM_JMP_EX(OP_JMP_ADDR(opline, opline->op2), 0);
	}

	SAVE_OPLINE();
	ret = i_crex_is_true(val);
	FREE_OP1();
	if (ret) {
		ZVAL_TRUE(EX_VAR(opline->result.var));
		opline++;
	} else {
		ZVAL_FALSE(EX_VAR(opline->result.var));
		opline = OP_JMP_ADDR(opline, opline->op2);
	}
	CREX_VM_JMP(opline);
}

CREX_VM_COLD_CONST_HANDLER(47, CREX_JMPNC_EX, CONST|TMPVAR|CV, JMP_ADDR)
{
	USE_OPLINE
	zval *val;
	bool ret;

	val = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);

	if (C_TYPE_INFO_P(val) == IS_TRUE) {
		ZVAL_TRUE(EX_VAR(opline->result.var));
		CREX_VM_JMP_EX(OP_JMP_ADDR(opline, opline->op2), 0);
	} else if (EXPECTED(C_TYPE_INFO_P(val) <= IS_TRUE)) {
		ZVAL_FALSE(EX_VAR(opline->result.var));
		if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_INFO_P(val) == IS_UNDEF)) {
			SAVE_OPLINE();
			ZVAL_UNDEFINED_OP1();
			CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
		} else {
			CREX_VM_NEXT_OPCODE();
		}
	}

	SAVE_OPLINE();
	ret = i_crex_is_true(val);
	FREE_OP1();
	if (ret) {
		ZVAL_TRUE(EX_VAR(opline->result.var));
		opline = OP_JMP_ADDR(opline, opline->op2);
	} else {
		ZVAL_FALSE(EX_VAR(opline->result.var));
		opline++;
	}
	CREX_VM_JMP(opline);
}

CREX_VM_HANDLER(70, CREX_FREE, TMPVAR, ANY)
{
	USE_OPLINE

	SAVE_OPLINE();
	zval_ptr_dtor_nogc(EX_VAR(opline->op1.var));
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HOT_HANDLER(127, CREX_FE_FREE, TMPVAR, ANY)
{
	zval *var;
	USE_OPLINE

	var = EX_VAR(opline->op1.var);
	if (C_TYPE_P(var) != IS_ARRAY) {
		SAVE_OPLINE();
		if (C_FE_ITER_P(var) != (uint32_t)-1) {
			crex_hash_iterator_del(C_FE_ITER_P(var));
		}
		zval_ptr_dtor_nogc(var);
		CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
	}

	/* This is freeing an array. Use an inlined version of zval_ptr_dtor_nogc. */
	/* CRX only needs to save the opline and check for an exception if the last reference to the array was garbage collected (destructors of elements in the array could throw an exception) */
	if (C_REFCOUNTED_P(var) && !C_DELREF_P(var)) {
		SAVE_OPLINE();
		rc_dtor_func(C_COUNTED_P(var));
		CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_COLD_CONSTCONST_HANDLER(53, CREX_FAST_CONCAT, CONST|TMPVAR|CV, CONST|TMPVAR|CV)
{
	USE_OPLINE
	zval *op1, *op2;
	crex_string *op1_str, *op2_str, *str;


	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	if ((OP1_TYPE == IS_CONST || EXPECTED(C_TYPE_P(op1) == IS_STRING)) &&
	    (OP2_TYPE == IS_CONST || EXPECTED(C_TYPE_P(op2) == IS_STRING))) {
		crex_string *op1_str = C_STR_P(op1);
		crex_string *op2_str = C_STR_P(op2);
		crex_string *str;
		uint32_t flags = ZSTR_GET_COPYABLE_CONCAT_PROPERTIES_BOTH(op1_str, op2_str);

		if (OP1_TYPE != IS_CONST && UNEXPECTED(ZSTR_LEN(op1_str) == 0)) {
			if (OP2_TYPE == IS_CONST || OP2_TYPE == IS_CV) {
				ZVAL_STR_COPY(EX_VAR(opline->result.var), op2_str);
			} else {
				ZVAL_STR(EX_VAR(opline->result.var), op2_str);
			}
			if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
				crex_string_release_ex(op1_str, 0);
			}
		} else if (OP2_TYPE != IS_CONST && UNEXPECTED(ZSTR_LEN(op2_str) == 0)) {
			if (OP1_TYPE == IS_CONST || OP1_TYPE == IS_CV) {
				ZVAL_STR_COPY(EX_VAR(opline->result.var), op1_str);
			} else {
				ZVAL_STR(EX_VAR(opline->result.var), op1_str);
			}
			if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
				crex_string_release_ex(op2_str, 0);
			}
		} else if (OP1_TYPE != IS_CONST && OP1_TYPE != IS_CV &&
		    !ZSTR_IS_INTERNED(op1_str) && GC_REFCOUNT(op1_str) == 1) {
			size_t len = ZSTR_LEN(op1_str);

			str = crex_string_extend(op1_str, len + ZSTR_LEN(op2_str), 0);
			memcpy(ZSTR_VAL(str) + len, ZSTR_VAL(op2_str), ZSTR_LEN(op2_str)+1);
			GC_ADD_FLAGS(str, flags);
			ZVAL_NEW_STR(EX_VAR(opline->result.var), str);
			if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
				crex_string_release_ex(op2_str, 0);
			}
		} else {
			str = crex_string_alloc(ZSTR_LEN(op1_str) + ZSTR_LEN(op2_str), 0);
			memcpy(ZSTR_VAL(str), ZSTR_VAL(op1_str), ZSTR_LEN(op1_str));
			memcpy(ZSTR_VAL(str) + ZSTR_LEN(op1_str), ZSTR_VAL(op2_str), ZSTR_LEN(op2_str)+1);
			GC_ADD_FLAGS(str, flags);
			ZVAL_NEW_STR(EX_VAR(opline->result.var), str);
			if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
				crex_string_release_ex(op1_str, 0);
			}
			if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
				crex_string_release_ex(op2_str, 0);
			}
		}
		CREX_VM_NEXT_OPCODE();
	}

	SAVE_OPLINE();
	if (OP1_TYPE == IS_CONST) {
		op1_str = C_STR_P(op1);
	} else if (EXPECTED(C_TYPE_P(op1) == IS_STRING)) {
		op1_str = crex_string_copy(C_STR_P(op1));
	} else {
		if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(op1) == IS_UNDEF)) {
			ZVAL_UNDEFINED_OP1();
		}
		op1_str = zval_get_string_func(op1);
	}
	if (OP2_TYPE == IS_CONST) {
		op2_str = C_STR_P(op2);
	} else if (EXPECTED(C_TYPE_P(op2) == IS_STRING)) {
		op2_str = crex_string_copy(C_STR_P(op2));
	} else {
		if (OP2_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(op2) == IS_UNDEF)) {
			ZVAL_UNDEFINED_OP2();
		}
		op2_str = zval_get_string_func(op2);
	}
	do {
		if (OP1_TYPE != IS_CONST) {
			if (UNEXPECTED(ZSTR_LEN(op1_str) == 0)) {
				if (OP2_TYPE == IS_CONST) {
					if (UNEXPECTED(C_REFCOUNTED_P(op2))) {
						GC_ADDREF(op2_str);
					}
				}
				ZVAL_STR(EX_VAR(opline->result.var), op2_str);
				crex_string_release_ex(op1_str, 0);
				break;
			}
		}
		if (OP2_TYPE != IS_CONST) {
			if (UNEXPECTED(ZSTR_LEN(op2_str) == 0)) {
				if (OP1_TYPE == IS_CONST) {
					if (UNEXPECTED(C_REFCOUNTED_P(op1))) {
						GC_ADDREF(op1_str);
					}
				}
				ZVAL_STR(EX_VAR(opline->result.var), op1_str);
				crex_string_release_ex(op2_str, 0);
				break;
			}
		}
		str = crex_string_alloc(ZSTR_LEN(op1_str) + ZSTR_LEN(op2_str), 0);
		memcpy(ZSTR_VAL(str), ZSTR_VAL(op1_str), ZSTR_LEN(op1_str));
		memcpy(ZSTR_VAL(str) + ZSTR_LEN(op1_str), ZSTR_VAL(op2_str), ZSTR_LEN(op2_str)+1);

		ZSTR_COPY_CONCAT_PROPERTIES_BOTH(str, op1_str, op2_str);
		ZVAL_NEW_STR(EX_VAR(opline->result.var), str);
		if (OP1_TYPE != IS_CONST) {
			crex_string_release_ex(op1_str, 0);
		}
		if (OP2_TYPE != IS_CONST) {
			crex_string_release_ex(op2_str, 0);
		}
	} while (0);
	FREE_OP1();
	FREE_OP2();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(54, CREX_ROPE_INIT, UNUSED, CONST|TMPVAR|CV, NUM)
{
	USE_OPLINE
	crex_string **rope;
	zval *var;

	/* Compiler allocates the necessary number of zval slots to keep the rope */
	rope = (crex_string**)EX_VAR(opline->result.var);
	if (OP2_TYPE == IS_CONST) {
		var = GET_OP2_ZVAL_PTR(BP_VAR_R);
		rope[0] = C_STR_P(var);
		if (UNEXPECTED(C_REFCOUNTED_P(var))) {
			C_ADDREF_P(var);
		}
	} else {
		var = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
		if (EXPECTED(C_TYPE_P(var) == IS_STRING)) {
			if (OP2_TYPE == IS_CV) {
				rope[0] = crex_string_copy(C_STR_P(var));
			} else {
				rope[0] = C_STR_P(var);
			}
		} else {
			SAVE_OPLINE();
			if (OP2_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(var) == IS_UNDEF)) {
				ZVAL_UNDEFINED_OP2();
			}
			rope[0] = zval_get_string_func(var);
			FREE_OP2();
			CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
		}
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(55, CREX_ROPE_ADD, TMP, CONST|TMPVAR|CV, NUM)
{
	USE_OPLINE
	crex_string **rope;
	zval *var;

	/* op1 and result are the same */
	rope = (crex_string**)EX_VAR(opline->op1.var);
	if (OP2_TYPE == IS_CONST) {
		var = GET_OP2_ZVAL_PTR(BP_VAR_R);
		rope[opline->extended_value] = C_STR_P(var);
		if (UNEXPECTED(C_REFCOUNTED_P(var))) {
			C_ADDREF_P(var);
		}
	} else {
		var = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
		if (EXPECTED(C_TYPE_P(var) == IS_STRING)) {
			if (OP2_TYPE == IS_CV) {
				rope[opline->extended_value] = crex_string_copy(C_STR_P(var));
			} else {
				rope[opline->extended_value] = C_STR_P(var);
			}
		} else {
			SAVE_OPLINE();
			if (OP2_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(var) == IS_UNDEF)) {
				ZVAL_UNDEFINED_OP2();
			}
			rope[opline->extended_value] = zval_get_string_func(var);
			FREE_OP2();
			CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
		}
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(56, CREX_ROPE_END, TMP, CONST|TMPVAR|CV, NUM)
{
	USE_OPLINE
	crex_string **rope;
	zval *var, *ret;
	uint32_t i;

	rope = (crex_string**)EX_VAR(opline->op1.var);
	if (OP2_TYPE == IS_CONST) {
		var = GET_OP2_ZVAL_PTR(BP_VAR_R);
		rope[opline->extended_value] = C_STR_P(var);
		if (UNEXPECTED(C_REFCOUNTED_P(var))) {
			C_ADDREF_P(var);
		}
	} else {
		var = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
		if (EXPECTED(C_TYPE_P(var) == IS_STRING)) {
			if (OP2_TYPE == IS_CV) {
				rope[opline->extended_value] = crex_string_copy(C_STR_P(var));
			} else {
				rope[opline->extended_value] = C_STR_P(var);
			}
		} else {
			SAVE_OPLINE();
			if (OP2_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(var) == IS_UNDEF)) {
				ZVAL_UNDEFINED_OP2();
			}
			rope[opline->extended_value] = zval_get_string_func(var);
			FREE_OP2();
			if (UNEXPECTED(EG(exception))) {
				for (i = 0; i <= opline->extended_value; i++) {
					crex_string_release_ex(rope[i], 0);
				}
				ZVAL_UNDEF(EX_VAR(opline->result.var));
				HANDLE_EXCEPTION();
			}
		}
	}

	size_t len = 0;
	uint32_t flags = ZSTR_COPYABLE_CONCAT_PROPERTIES;
	for (i = 0; i <= opline->extended_value; i++) {
		flags &= ZSTR_GET_COPYABLE_CONCAT_PROPERTIES(rope[i]);
		len += ZSTR_LEN(rope[i]);
	}
	ret = EX_VAR(opline->result.var);
	ZVAL_STR(ret, crex_string_alloc(len, 0));
	GC_ADD_FLAGS(C_STR_P(ret), flags);

	char *target = C_STRVAL_P(ret);
	for (i = 0; i <= opline->extended_value; i++) {
		memcpy(target, ZSTR_VAL(rope[i]), ZSTR_LEN(rope[i]));
		target += ZSTR_LEN(rope[i]);
		crex_string_release_ex(rope[i], 0);
	}
	*target = '\0';

	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(109, CREX_FETCH_CLASS, UNUSED|CLASS_FETCH, CONST|TMPVAR|UNUSED|CV, CACHE_SLOT)
{
	zval *class_name;
	USE_OPLINE

	SAVE_OPLINE();
	if (OP2_TYPE == IS_UNUSED) {
		C_CE_P(EX_VAR(opline->result.var)) = crex_fetch_class(NULL, opline->op1.num);
		CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
	} else if (OP2_TYPE == IS_CONST) {
		crex_class_entry *ce = CACHED_PTR(opline->extended_value);

		if (UNEXPECTED(ce == NULL)) {
			class_name = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
			ce = crex_fetch_class_by_name(C_STR_P(class_name), C_STR_P(class_name + 1), opline->op1.num);
			CACHE_PTR(opline->extended_value, ce);
		}
		C_CE_P(EX_VAR(opline->result.var)) = ce;
	} else {
		class_name = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
CREX_VM_C_LABEL(try_class_name):
		if (C_TYPE_P(class_name) == IS_OBJECT) {
			C_CE_P(EX_VAR(opline->result.var)) = C_OBJCE_P(class_name);
		} else if (C_TYPE_P(class_name) == IS_STRING) {
			C_CE_P(EX_VAR(opline->result.var)) = crex_fetch_class(C_STR_P(class_name), opline->op1.num);
		} else if ((OP2_TYPE & (IS_VAR|IS_CV)) && C_TYPE_P(class_name) == IS_REFERENCE) {
			class_name = C_REFVAL_P(class_name);
			CREX_VM_C_GOTO(try_class_name);
		} else {
			if (OP2_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(class_name) == IS_UNDEF)) {
				ZVAL_UNDEFINED_OP2();
				if (UNEXPECTED(EG(exception) != NULL)) {
					HANDLE_EXCEPTION();
				}
			}
			crex_throw_error(NULL, "Class name must be a valid object or a string");
		}
	}

	FREE_OP2();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HOT_OBJ_HANDLER(112, CREX_INIT_METHOD_CALL, CONST|TMPVAR|UNUSED|THIS|CV, CONST|TMPVAR|CV, NUM|CACHE_SLOT)
{
	USE_OPLINE
	zval *function_name;
	zval *object;
	crex_function *fbc;
	crex_class_entry *called_scope;
	crex_object *obj;
	crex_execute_data *call;
	uint32_t call_info;

	SAVE_OPLINE();

	object = GET_OP1_OBJ_ZVAL_PTR_UNDEF(BP_VAR_R);

	if (OP2_TYPE != IS_CONST) {
		function_name = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	}

	if (OP2_TYPE != IS_CONST &&
	    UNEXPECTED(C_TYPE_P(function_name) != IS_STRING)) {
		do {
			if ((OP2_TYPE & (IS_VAR|IS_CV)) && C_ISREF_P(function_name)) {
				function_name = C_REFVAL_P(function_name);
				if (EXPECTED(C_TYPE_P(function_name) == IS_STRING)) {
					break;
				}
			} else if (OP2_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(function_name) == IS_UNDEF)) {
				ZVAL_UNDEFINED_OP2();
				if (UNEXPECTED(EG(exception) != NULL)) {
					FREE_OP1();
					HANDLE_EXCEPTION();
				}
			}
			crex_throw_error(NULL, "Method name must be a string");
			FREE_OP2();
			FREE_OP1();
			HANDLE_EXCEPTION();
		} while (0);
	}

	if (OP1_TYPE == IS_UNUSED) {
		obj = C_OBJ_P(object);
	} else {
		do {
			if (OP1_TYPE != IS_CONST && EXPECTED(C_TYPE_P(object) == IS_OBJECT)) {
				obj = C_OBJ_P(object);
			} else {
				if ((OP1_TYPE & (IS_VAR|IS_CV)) && EXPECTED(C_ISREF_P(object))) {
					crex_reference *ref = C_REF_P(object);

					object = &ref->val;
					if (EXPECTED(C_TYPE_P(object) == IS_OBJECT)) {
						obj = C_OBJ_P(object);
						if (OP1_TYPE & IS_VAR) {
							if (UNEXPECTED(GC_DELREF(ref) == 0)) {
								efree_size(ref, sizeof(crex_reference));
							} else {
								C_ADDREF_P(object);
							}
						}
						break;
					}
				}
				if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(object) == IS_UNDEF)) {
					object = ZVAL_UNDEFINED_OP1();
					if (UNEXPECTED(EG(exception) != NULL)) {
						if (OP2_TYPE != IS_CONST) {
							FREE_OP2();
						}
						HANDLE_EXCEPTION();
					}
				}
				if (OP2_TYPE == IS_CONST) {
					function_name = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
				}
				crex_invalid_method_call(object, function_name);
				FREE_OP2();
				FREE_OP1();
				HANDLE_EXCEPTION();
			}
		} while (0);
	}

	called_scope = obj->ce;

	if (OP2_TYPE == IS_CONST &&
	    EXPECTED(CACHED_PTR(opline->result.num) == called_scope)) {
		fbc = CACHED_PTR(opline->result.num + sizeof(void*));
	} else {
		crex_object *orig_obj = obj;

		if (OP2_TYPE == IS_CONST) {
			function_name = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
		}

		/* First, locate the function. */
		fbc = obj->handlers->get_method(&obj, C_STR_P(function_name), ((OP2_TYPE == IS_CONST) ? (RT_CONSTANT(opline, opline->op2) + 1) : NULL));
		if (UNEXPECTED(fbc == NULL)) {
			if (EXPECTED(!EG(exception))) {
				crex_undefined_method(obj->ce, C_STR_P(function_name));
			}
			FREE_OP2();
			if ((OP1_TYPE & (IS_VAR|IS_TMP_VAR)) && GC_DELREF(orig_obj) == 0) {
				crex_objects_store_del(orig_obj);
			}
			HANDLE_EXCEPTION();
		}
		if (OP2_TYPE == IS_CONST &&
		    EXPECTED(!(fbc->common.fn_flags & (CREX_ACC_CALL_VIA_TRAMPOLINE|CREX_ACC_NEVER_CACHE))) &&
		    EXPECTED(obj == orig_obj)) {
			CACHE_POLYMORPHIC_PTR(opline->result.num, called_scope, fbc);
		}
		if ((OP1_TYPE & (IS_VAR|IS_TMP_VAR)) && UNEXPECTED(obj != orig_obj)) {
			GC_ADDREF(obj); /* For $this pointer */
			if (GC_DELREF(orig_obj) == 0) {
				crex_objects_store_del(orig_obj);
			}
		}
		if (EXPECTED(fbc->type == CREX_USER_FUNCTION) && UNEXPECTED(!RUN_TIME_CACHE(&fbc->op_array))) {
			init_func_run_time_cache(&fbc->op_array);
		}
	}

	if (OP2_TYPE != IS_CONST) {
		FREE_OP2();
	}

	call_info = CREX_CALL_NESTED_FUNCTION | CREX_CALL_HAS_THIS;
	if (UNEXPECTED((fbc->common.fn_flags & CREX_ACC_STATIC) != 0)) {
		if ((OP1_TYPE & (IS_VAR|IS_TMP_VAR)) && GC_DELREF(obj) == 0) {
			crex_objects_store_del(obj);
			if (UNEXPECTED(EG(exception))) {
				HANDLE_EXCEPTION();
			}
		}
		/* call static method */
		obj = (crex_object*)called_scope;
		call_info = CREX_CALL_NESTED_FUNCTION;
	} else if (OP1_TYPE & (IS_VAR|IS_TMP_VAR|IS_CV)) {
		if (OP1_TYPE == IS_CV) {
			GC_ADDREF(obj); /* For $this pointer */
		}
		/* CV may be changed indirectly (e.g. when it's a reference) */
		call_info = CREX_CALL_NESTED_FUNCTION | CREX_CALL_HAS_THIS | CREX_CALL_RELEASE_THIS;
	}

	call = crex_vm_stack_push_call_frame(call_info,
		fbc, opline->extended_value, obj);
	call->prev_execute_data = EX(call);
	EX(call) = call;

	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(113, CREX_INIT_STATIC_METHOD_CALL, UNUSED|CLASS_FETCH|CONST|VAR, CONST|TMPVAR|UNUSED|CONSTRUCTOR|CV, NUM|CACHE_SLOT)
{
	USE_OPLINE
	zval *function_name;
	crex_class_entry *ce;
	uint32_t call_info;
	crex_function *fbc;
	crex_execute_data *call;

	SAVE_OPLINE();

	if (OP1_TYPE == IS_CONST) {
		/* no function found. try a static method in class */
		ce = CACHED_PTR(opline->result.num);
		if (UNEXPECTED(ce == NULL)) {
			ce = crex_fetch_class_by_name(C_STR_P(RT_CONSTANT(opline, opline->op1)), C_STR_P(RT_CONSTANT(opline, opline->op1) + 1), CREX_FETCH_CLASS_DEFAULT | CREX_FETCH_CLASS_EXCEPTION);
			if (UNEXPECTED(ce == NULL)) {
				FREE_OP2();
				HANDLE_EXCEPTION();
			}
			if (OP2_TYPE != IS_CONST) {
				CACHE_PTR(opline->result.num, ce);
			}
		}
	} else if (OP1_TYPE == IS_UNUSED) {
		ce = crex_fetch_class(NULL, opline->op1.num);
		if (UNEXPECTED(ce == NULL)) {
			FREE_OP2();
			HANDLE_EXCEPTION();
		}
	} else {
		ce = C_CE_P(EX_VAR(opline->op1.var));
	}

	if (OP1_TYPE == IS_CONST &&
	    OP2_TYPE == IS_CONST &&
	    EXPECTED((fbc = CACHED_PTR(opline->result.num + sizeof(void*))) != NULL)) {
		/* nothing to do */
	} else if (OP1_TYPE != IS_CONST &&
	           OP2_TYPE == IS_CONST &&
	           EXPECTED(CACHED_PTR(opline->result.num) == ce)) {
		fbc = CACHED_PTR(opline->result.num + sizeof(void*));
	} else if (OP2_TYPE != IS_UNUSED) {
		function_name = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
		if (OP2_TYPE != IS_CONST) {
			if (UNEXPECTED(C_TYPE_P(function_name) != IS_STRING)) {
				do {
					if (OP2_TYPE & (IS_VAR|IS_CV) && C_ISREF_P(function_name)) {
						function_name = C_REFVAL_P(function_name);
						if (EXPECTED(C_TYPE_P(function_name) == IS_STRING)) {
							break;
						}
					} else if (OP2_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(function_name) == IS_UNDEF)) {
						ZVAL_UNDEFINED_OP2();
						if (UNEXPECTED(EG(exception) != NULL)) {
							HANDLE_EXCEPTION();
						}
					}
					crex_throw_error(NULL, "Method name must be a string");
					FREE_OP2();
					HANDLE_EXCEPTION();
				} while (0);
			}
		}

		if (ce->get_static_method) {
			fbc = ce->get_static_method(ce, C_STR_P(function_name));
		} else {
			fbc = crex_std_get_static_method(ce, C_STR_P(function_name), ((OP2_TYPE == IS_CONST) ? (RT_CONSTANT(opline, opline->op2) + 1) : NULL));
		}
		if (UNEXPECTED(fbc == NULL)) {
			if (EXPECTED(!EG(exception))) {
				crex_undefined_method(ce, C_STR_P(function_name));
			}
			FREE_OP2();
			HANDLE_EXCEPTION();
		}
		if (OP2_TYPE == IS_CONST &&
		    EXPECTED(!(fbc->common.fn_flags & (CREX_ACC_CALL_VIA_TRAMPOLINE|CREX_ACC_NEVER_CACHE))) &&
			EXPECTED(!(fbc->common.scope->ce_flags & CREX_ACC_TRAIT))) {
			CACHE_POLYMORPHIC_PTR(opline->result.num, ce, fbc);
		}
		if (EXPECTED(fbc->type == CREX_USER_FUNCTION) && UNEXPECTED(!RUN_TIME_CACHE(&fbc->op_array))) {
			init_func_run_time_cache(&fbc->op_array);
		}
		if (OP2_TYPE != IS_CONST) {
			FREE_OP2();
		}
	} else {
		if (UNEXPECTED(ce->constructor == NULL)) {
			crex_throw_error(NULL, "Cannot call constructor");
			HANDLE_EXCEPTION();
		}
		if (C_TYPE(EX(This)) == IS_OBJECT && C_OBJ(EX(This))->ce != ce->constructor->common.scope && (ce->constructor->common.fn_flags & CREX_ACC_PRIVATE)) {
			crex_throw_error(NULL, "Cannot call private %s::__main()", ZSTR_VAL(ce->name));
			HANDLE_EXCEPTION();
		}
		fbc = ce->constructor;
		if (EXPECTED(fbc->type == CREX_USER_FUNCTION) && UNEXPECTED(!RUN_TIME_CACHE(&fbc->op_array))) {
			init_func_run_time_cache(&fbc->op_array);
		}
	}

	if (!(fbc->common.fn_flags & CREX_ACC_STATIC)) {
		if (C_TYPE(EX(This)) == IS_OBJECT && instanceof_function(C_OBJCE(EX(This)), ce)) {
			ce = (crex_class_entry*)C_OBJ(EX(This));
			call_info = CREX_CALL_NESTED_FUNCTION | CREX_CALL_HAS_THIS;
		} else {
			crex_non_static_method_call(fbc);
			HANDLE_EXCEPTION();
		}
	} else {
		/* previous opcode is CREX_FETCH_CLASS */
		if (OP1_TYPE == IS_UNUSED
		 && ((opline->op1.num & CREX_FETCH_CLASS_MASK) == CREX_FETCH_CLASS_PARENT ||
		     (opline->op1.num & CREX_FETCH_CLASS_MASK) == CREX_FETCH_CLASS_SELF)) {
			if (C_TYPE(EX(This)) == IS_OBJECT) {
				ce = C_OBJCE(EX(This));
			} else {
				ce = C_CE(EX(This));
			}
		}
		call_info = CREX_CALL_NESTED_FUNCTION;
	}

	call = crex_vm_stack_push_call_frame(call_info,
		fbc, opline->extended_value, ce);
	call->prev_execute_data = EX(call);
	EX(call) = call;

	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_HANDLER(59, CREX_INIT_FCALL_BY_NAME, ANY, CONST, NUM|CACHE_SLOT)
{
	USE_OPLINE
	crex_function *fbc;
	zval *function_name, *func;
	crex_execute_data *call;

	fbc = CACHED_PTR(opline->result.num);
	if (UNEXPECTED(fbc == NULL)) {
		function_name = (zval*)RT_CONSTANT(opline, opline->op2);
		func = crex_hash_find_known_hash(EG(function_table), C_STR_P(function_name+1));
		if (UNEXPECTED(func == NULL)) {
			CREX_VM_DISPATCH_TO_HELPER(crex_undefined_function_helper);
		}
		fbc = C_FUNC_P(func);
		if (EXPECTED(fbc->type == CREX_USER_FUNCTION) && UNEXPECTED(!RUN_TIME_CACHE(&fbc->op_array))) {
			init_func_run_time_cache(&fbc->op_array);
		}
		CACHE_PTR(opline->result.num, fbc);
	}
	call = _crex_vm_stack_push_call_frame(CREX_CALL_NESTED_FUNCTION,
		fbc, opline->extended_value, NULL);
	call->prev_execute_data = EX(call);
	EX(call) = call;

	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(128, CREX_INIT_DYNAMIC_CALL, ANY, CONST|TMPVAR|CV, NUM)
{
	USE_OPLINE
	zval *function_name;
	crex_execute_data *call;

	SAVE_OPLINE();
	function_name = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);

CREX_VM_C_LABEL(try_function_name):
	if (OP2_TYPE != IS_CONST && EXPECTED(C_TYPE_P(function_name) == IS_STRING)) {
		call = crex_init_dynamic_call_string(C_STR_P(function_name), opline->extended_value);
	} else if (OP2_TYPE != IS_CONST && EXPECTED(C_TYPE_P(function_name) == IS_OBJECT)) {
		call = crex_init_dynamic_call_object(C_OBJ_P(function_name), opline->extended_value);
	} else if (EXPECTED(C_TYPE_P(function_name) == IS_ARRAY)) {
		call = crex_init_dynamic_call_array(C_ARRVAL_P(function_name), opline->extended_value);
	} else if ((OP2_TYPE & (IS_VAR|IS_CV)) && EXPECTED(C_TYPE_P(function_name) == IS_REFERENCE)) {
		function_name = C_REFVAL_P(function_name);
		CREX_VM_C_GOTO(try_function_name);
	} else {
		if (OP2_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(function_name) == IS_UNDEF)) {
			function_name = ZVAL_UNDEFINED_OP2();
			if (UNEXPECTED(EG(exception) != NULL)) {
				HANDLE_EXCEPTION();
			}
		}
		crex_throw_error(NULL, "Value of type %s is not callable",
			crex_zval_type_name(function_name));
		call = NULL;
	}

	if (OP2_TYPE & (IS_VAR|IS_TMP_VAR)) {
		FREE_OP2();
		if (UNEXPECTED(EG(exception))) {
			if (call) {
				 if (call->func->common.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE) {
					crex_string_release_ex(call->func->common.function_name, 0);
					crex_free_trampoline(call->func);
				}
				crex_vm_stack_free_call_frame(call);
			}
			HANDLE_EXCEPTION();
		}
	} else if (!call) {
		HANDLE_EXCEPTION();
	}

	call->prev_execute_data = EX(call);
	EX(call) = call;

	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(118, CREX_INIT_USER_CALL, CONST, CONST|TMPVAR|CV, NUM)
{
	USE_OPLINE
	zval *function_name;
	crex_fcall_info_cache fcc;
	char *error = NULL;
	crex_function *func;
	void *object_or_called_scope;
	crex_execute_data *call;
	uint32_t call_info = CREX_CALL_NESTED_FUNCTION | CREX_CALL_DYNAMIC;

	SAVE_OPLINE();
	function_name = GET_OP2_ZVAL_PTR(BP_VAR_R);
	if (crex_is_callable_ex(function_name, NULL, 0, NULL, &fcc, &error)) {
		CREX_ASSERT(!error);
		func = fcc.function_handler;
		object_or_called_scope = fcc.called_scope;
		if (func->common.fn_flags & CREX_ACC_CLOSURE) {
			/* Delay closure destruction until its invocation */
			GC_ADDREF(CREX_CLOSURE_OBJECT(func));
			call_info |= CREX_CALL_CLOSURE;
			if (func->common.fn_flags & CREX_ACC_FAKE_CLOSURE) {
				call_info |= CREX_CALL_FAKE_CLOSURE;
			}
			if (fcc.object) {
				object_or_called_scope = fcc.object;
				call_info |= CREX_CALL_HAS_THIS;
			}
		} else if (fcc.object) {
			GC_ADDREF(fcc.object); /* For $this pointer */
			object_or_called_scope = fcc.object;
			call_info |= CREX_CALL_RELEASE_THIS | CREX_CALL_HAS_THIS;
		}

		FREE_OP2();
		if ((OP2_TYPE & (IS_TMP_VAR|IS_VAR)) && UNEXPECTED(EG(exception))) {
			if (call_info & CREX_CALL_CLOSURE) {
				crex_object_release(CREX_CLOSURE_OBJECT(func));
			} else if (call_info & CREX_CALL_RELEASE_THIS) {
				crex_object_release(fcc.object);
			}
			HANDLE_EXCEPTION();
		}

		if (EXPECTED(func->type == CREX_USER_FUNCTION) && UNEXPECTED(!RUN_TIME_CACHE(&func->op_array))) {
			init_func_run_time_cache(&func->op_array);
		}
	} else {
		crex_type_error("%s(): Argument #1 ($callback) must be a valid callback, %s", C_STRVAL_P(RT_CONSTANT(opline, opline->op1)), error);
		efree(error);
		FREE_OP2();
		HANDLE_EXCEPTION();
	}

	call = crex_vm_stack_push_call_frame(call_info,
		func, opline->extended_value, object_or_called_scope);
	call->prev_execute_data = EX(call);
	EX(call) = call;

	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_HANDLER(69, CREX_INIT_NS_FCALL_BY_NAME, ANY, CONST, NUM|CACHE_SLOT)
{
	USE_OPLINE
	zval *func_name;
	zval *func;
	crex_function *fbc;
	crex_execute_data *call;

	fbc = CACHED_PTR(opline->result.num);
	if (UNEXPECTED(fbc == NULL)) {
		func_name = (zval *)RT_CONSTANT(opline, opline->op2);
		func = crex_hash_find_known_hash(EG(function_table), C_STR_P(func_name + 1));
		if (func == NULL) {
			func = crex_hash_find_known_hash(EG(function_table), C_STR_P(func_name + 2));
			if (UNEXPECTED(func == NULL)) {
				CREX_VM_DISPATCH_TO_HELPER(crex_undefined_function_helper);
			}
		}
		fbc = C_FUNC_P(func);
		if (EXPECTED(fbc->type == CREX_USER_FUNCTION) && UNEXPECTED(!RUN_TIME_CACHE(&fbc->op_array))) {
			init_func_run_time_cache(&fbc->op_array);
		}
		CACHE_PTR(opline->result.num, fbc);
	}

	call = _crex_vm_stack_push_call_frame(CREX_CALL_NESTED_FUNCTION,
		fbc, opline->extended_value, NULL);
	call->prev_execute_data = EX(call);
	EX(call) = call;

	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_HANDLER(61, CREX_INIT_FCALL, NUM, CONST, NUM|CACHE_SLOT)
{
	USE_OPLINE
	zval *fname;
	zval *func;
	crex_function *fbc;
	crex_execute_data *call;

	fbc = CACHED_PTR(opline->result.num);
	if (UNEXPECTED(fbc == NULL)) {
		fname = (zval*)RT_CONSTANT(opline, opline->op2);
		func = crex_hash_find_known_hash(EG(function_table), C_STR_P(fname));
		CREX_ASSERT(func != NULL && "Function existence must be checked at compile time");
		fbc = C_FUNC_P(func);
		if (EXPECTED(fbc->type == CREX_USER_FUNCTION) && UNEXPECTED(!RUN_TIME_CACHE(&fbc->op_array))) {
			init_func_run_time_cache(&fbc->op_array);
		}
		CACHE_PTR(opline->result.num, fbc);
	}

	call = _crex_vm_stack_push_call_frame_ex(
		opline->op1.num, CREX_CALL_NESTED_FUNCTION,
		fbc, opline->extended_value, NULL);
	call->prev_execute_data = EX(call);
	EX(call) = call;

	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_HANDLER(129, CREX_DO_ICALL, ANY, ANY, SPEC(RETVAL,OBSERVER))
{
	USE_OPLINE
	crex_execute_data *call = EX(call);
	crex_function *fbc = call->func;
	zval *ret;
	zval retval;

	SAVE_OPLINE();
	EX(call) = call->prev_execute_data;

	call->prev_execute_data = execute_data;
	EG(current_execute_data) = call;

#if CREX_DEBUG
	bool should_throw = crex_internal_call_should_throw(fbc, call);
#endif

	ret = RETURN_VALUE_USED(opline) ? EX_VAR(opline->result.var) : &retval;
	ZVAL_NULL(ret);

	CREX_OBSERVER_FCALL_BEGIN(call);
	fbc->internal_function.handler(call, ret);

#if CREX_DEBUG
	if (!EG(exception) && call->func) {
		if (should_throw) {
			crex_internal_call_arginfo_violation(call->func);
		}
		CREX_ASSERT(!(call->func->common.fn_flags & CREX_ACC_HAS_RETURN_TYPE) ||
			crex_verify_internal_return_type(call->func, ret));
		CREX_ASSERT((call->func->common.fn_flags & CREX_ACC_RETURN_REFERENCE)
			? C_ISREF_P(ret) : !C_ISREF_P(ret));
		crex_verify_internal_func_info(call->func, ret);
	}
#endif
	CREX_OBSERVER_FCALL_END(call, EG(exception) ? NULL : ret);

	EG(current_execute_data) = execute_data;
	crex_vm_stack_free_args(call);

	uint32_t call_info = CREX_CALL_INFO(call);
	if (UNEXPECTED(call_info & (CREX_CALL_HAS_EXTRA_NAMED_PARAMS|CREX_CALL_ALLOCATED))) {
		if (call_info & CREX_CALL_HAS_EXTRA_NAMED_PARAMS) {
			crex_free_extra_named_params(call->extra_named_params);
		}
		crex_vm_stack_free_call_frame_ex(call_info, call);
	} else {
		EG(vm_stack_top) = (zval*)call;
	}

	if (!RETURN_VALUE_USED(opline)) {
		i_zval_ptr_dtor(ret);
	}

	if (UNEXPECTED(EG(exception) != NULL)) {
		crex_rethrow_exception(execute_data);
		HANDLE_EXCEPTION();
	}

	CREX_VM_SET_OPCODE(opline + 1);
	CREX_VM_CONTINUE();
}

CREX_VM_HOT_HANDLER(130, CREX_DO_UCALL, ANY, ANY, SPEC(RETVAL,OBSERVER))
{
	USE_OPLINE
	crex_execute_data *call = EX(call);
	crex_function *fbc = call->func;
	zval *ret;

	SAVE_OPLINE();
	EX(call) = call->prev_execute_data;

	ret = NULL;
	if (RETURN_VALUE_USED(opline)) {
		ret = EX_VAR(opline->result.var);
	}

	call->prev_execute_data = execute_data;
	execute_data = call;
	i_init_func_execute_data(&fbc->op_array, ret, 0 EXECUTE_DATA_CC);
	LOAD_OPLINE_EX();
	CREX_OBSERVER_SAVE_OPLINE();
	CREX_OBSERVER_FCALL_BEGIN(execute_data);

	CREX_VM_ENTER_EX();
}

CREX_VM_HOT_HANDLER(131, CREX_DO_FCALL_BY_NAME, ANY, ANY, SPEC(RETVAL,OBSERVER))
{
	USE_OPLINE
	crex_execute_data *call = EX(call);
	crex_function *fbc = call->func;
	zval *ret;

	SAVE_OPLINE();
	EX(call) = call->prev_execute_data;

	if (EXPECTED(fbc->type == CREX_USER_FUNCTION)) {
		ret = NULL;
		if (RETURN_VALUE_USED(opline)) {
			ret = EX_VAR(opline->result.var);
		}

		call->prev_execute_data = execute_data;
		execute_data = call;
		i_init_func_execute_data(&fbc->op_array, ret, 0 EXECUTE_DATA_CC);
		LOAD_OPLINE_EX();
		CREX_OBSERVER_SAVE_OPLINE();
		CREX_OBSERVER_FCALL_BEGIN(execute_data);

		CREX_VM_ENTER_EX();
	} else {
		zval retval;
		CREX_ASSERT(fbc->type == CREX_INTERNAL_FUNCTION);
		if (CREX_OBSERVER_ENABLED) {
			ret = NULL;
		}

		if (UNEXPECTED((fbc->common.fn_flags & CREX_ACC_DEPRECATED) != 0)) {
			crex_deprecated_function(fbc);
			if (UNEXPECTED(EG(exception) != NULL)) {
				UNDEF_RESULT();
				if (!RETURN_VALUE_USED(opline)) {
					ret = &retval;
					ZVAL_UNDEF(ret);
				}
				CREX_VM_C_GOTO(fcall_by_name_end);
			}
		}

		call->prev_execute_data = execute_data;
		EG(current_execute_data) = call;

#if CREX_DEBUG
		bool should_throw = crex_internal_call_should_throw(fbc, call);
#endif

		ret = RETURN_VALUE_USED(opline) ? EX_VAR(opline->result.var) : &retval;
		ZVAL_NULL(ret);

		CREX_OBSERVER_FCALL_BEGIN(call);
		fbc->internal_function.handler(call, ret);

#if CREX_DEBUG
		if (!EG(exception) && call->func) {
			if (should_throw) {
				crex_internal_call_arginfo_violation(call->func);
			}
			CREX_ASSERT(!(call->func->common.fn_flags & CREX_ACC_HAS_RETURN_TYPE) ||
				crex_verify_internal_return_type(call->func, ret));
			CREX_ASSERT((call->func->common.fn_flags & CREX_ACC_RETURN_REFERENCE)
				? C_ISREF_P(ret) : !C_ISREF_P(ret));
			crex_verify_internal_func_info(call->func, ret);
		}
#endif
		CREX_OBSERVER_FCALL_END(call, EG(exception) ? NULL : ret);

		EG(current_execute_data) = execute_data;

CREX_VM_C_LABEL(fcall_by_name_end):
		crex_vm_stack_free_args(call);

		uint32_t call_info = CREX_CALL_INFO(call);
		if (UNEXPECTED(call_info & (CREX_CALL_HAS_EXTRA_NAMED_PARAMS|CREX_CALL_ALLOCATED))) {
			if (call_info & CREX_CALL_HAS_EXTRA_NAMED_PARAMS) {
				crex_free_extra_named_params(call->extra_named_params);
			}
			crex_vm_stack_free_call_frame_ex(call_info, call);
		} else {
			EG(vm_stack_top) = (zval*)call;
		}

		if (!RETURN_VALUE_USED(opline)) {
			i_zval_ptr_dtor(ret);
		}
	}

	if (UNEXPECTED(EG(exception) != NULL)) {
		crex_rethrow_exception(execute_data);
		HANDLE_EXCEPTION();
	}
	CREX_VM_SET_OPCODE(opline + 1);
	CREX_VM_CONTINUE();
}

CREX_VM_HOT_HANDLER(60, CREX_DO_FCALL, ANY, ANY, SPEC(RETVAL,OBSERVER))
{
	USE_OPLINE
	crex_execute_data *call = EX(call);
	crex_function *fbc = call->func;
	zval *ret;

	SAVE_OPLINE();
	EX(call) = call->prev_execute_data;

	if (EXPECTED(fbc->type == CREX_USER_FUNCTION)) {
		ret = NULL;
		if (RETURN_VALUE_USED(opline)) {
			ret = EX_VAR(opline->result.var);
		}

		call->prev_execute_data = execute_data;
		execute_data = call;
		i_init_func_execute_data(&fbc->op_array, ret, 1 EXECUTE_DATA_CC);

		if (EXPECTED(crex_execute_ex == execute_ex)) {
			LOAD_OPLINE_EX();
			CREX_OBSERVER_SAVE_OPLINE();
			CREX_OBSERVER_FCALL_BEGIN(execute_data);
			CREX_VM_ENTER_EX();
		} else {
			SAVE_OPLINE_EX();
			CREX_OBSERVER_FCALL_BEGIN(execute_data);
			execute_data = EX(prev_execute_data);
			LOAD_OPLINE();
			CREX_ADD_CALL_FLAG(call, CREX_CALL_TOP);
			crex_execute_ex(call);
		}
	} else {
		zval retval;
		CREX_ASSERT(fbc->type == CREX_INTERNAL_FUNCTION);
		if (CREX_OBSERVER_ENABLED) {
			ret = NULL;
		}

		if (UNEXPECTED((fbc->common.fn_flags & CREX_ACC_DEPRECATED) != 0)) {
			crex_deprecated_function(fbc);
			if (UNEXPECTED(EG(exception) != NULL)) {
				UNDEF_RESULT();
				if (!RETURN_VALUE_USED(opline)) {
					ret = &retval;
					ZVAL_UNDEF(ret);
				}
				CREX_VM_C_GOTO(fcall_end);
			}
		}

		call->prev_execute_data = execute_data;
		EG(current_execute_data) = call;

#if CREX_DEBUG
		bool should_throw = crex_internal_call_should_throw(fbc, call);
#endif

		ret = RETURN_VALUE_USED(opline) ? EX_VAR(opline->result.var) : &retval;
		ZVAL_NULL(ret);

		CREX_OBSERVER_FCALL_BEGIN(call);
		if (!crex_execute_internal) {
			/* saves one function call if crex_execute_internal is not used */
			fbc->internal_function.handler(call, ret);
		} else {
			crex_execute_internal(call, ret);
		}

#if CREX_DEBUG
		if (!EG(exception) && call->func && !(call->func->common.fn_flags & CREX_ACC_FAKE_CLOSURE)) {
			if (should_throw) {
				crex_internal_call_arginfo_violation(call->func);
			}
			CREX_ASSERT(!(call->func->common.fn_flags & CREX_ACC_HAS_RETURN_TYPE) ||
				crex_verify_internal_return_type(call->func, ret));
			CREX_ASSERT((call->func->common.fn_flags & CREX_ACC_RETURN_REFERENCE)
				? C_ISREF_P(ret) : !C_ISREF_P(ret));
			crex_verify_internal_func_info(call->func, ret);
		}
#endif
		CREX_OBSERVER_FCALL_END(call, EG(exception) ? NULL : ret);

		EG(current_execute_data) = execute_data;

CREX_VM_C_LABEL(fcall_end):
		crex_vm_stack_free_args(call);
		if (UNEXPECTED(CREX_CALL_INFO(call) & CREX_CALL_HAS_EXTRA_NAMED_PARAMS)) {
			crex_free_extra_named_params(call->extra_named_params);
		}

		if (!RETURN_VALUE_USED(opline)) {
			i_zval_ptr_dtor(ret);
		}
	}

	if (UNEXPECTED(CREX_CALL_INFO(call) & CREX_CALL_RELEASE_THIS)) {
		OBJ_RELEASE(C_OBJ(call->This));
	}

	crex_vm_stack_free_call_frame(call);
	if (UNEXPECTED(EG(exception) != NULL)) {
		crex_rethrow_exception(execute_data);
		HANDLE_EXCEPTION();
	}

	CREX_VM_SET_OPCODE(opline + 1);
	CREX_VM_CONTINUE();
}

CREX_VM_COLD_CONST_HANDLER(124, CREX_VERIFY_RETURN_TYPE, CONST|TMP|VAR|UNUSED|CV, UNUSED|CACHE_SLOT)
{
	if (OP1_TYPE == IS_UNUSED) {
		SAVE_OPLINE();
		crex_verify_missing_return_type(EX(func));
		HANDLE_EXCEPTION();
	} else {
/* prevents "undefined variable opline" errors */
#if !CREX_VM_SPEC || (OP1_TYPE != IS_UNUSED)
		USE_OPLINE
		zval *retval_ref, *retval_ptr;
		crex_arg_info *ret_info = EX(func)->common.arg_info - 1;
		retval_ref = retval_ptr = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);

		if (OP1_TYPE == IS_CONST) {
			ZVAL_COPY(EX_VAR(opline->result.var), retval_ptr);
			retval_ref = retval_ptr = EX_VAR(opline->result.var);
		} else if (OP1_TYPE == IS_VAR) {
			if (UNEXPECTED(C_TYPE_P(retval_ptr) == IS_INDIRECT)) {
				retval_ref = retval_ptr = C_INDIRECT_P(retval_ptr);
			}
			ZVAL_DEREF(retval_ptr);
		} else if (OP1_TYPE == IS_CV) {
			ZVAL_DEREF(retval_ptr);
		}

		if (EXPECTED(CREX_TYPE_CONTAINS_CODE(ret_info->type, C_TYPE_P(retval_ptr)))) {
			CREX_VM_NEXT_OPCODE();
		}

		if (OP1_TYPE == IS_CV && UNEXPECTED(C_ISUNDEF_P(retval_ptr))) {
			SAVE_OPLINE();
			retval_ref = retval_ptr = ZVAL_UNDEFINED_OP1();
			if (UNEXPECTED(EG(exception))) {
				HANDLE_EXCEPTION();
			}
			if (CREX_TYPE_FULL_MASK(ret_info->type) & MAY_BE_NULL) {
				CREX_VM_NEXT_OPCODE();
			}
		}

		crex_reference *ref = NULL;
		void *cache_slot = CACHE_ADDR(opline->op2.num);
		if (UNEXPECTED(retval_ref != retval_ptr)) {
			if (UNEXPECTED(EX(func)->op_array.fn_flags & CREX_ACC_RETURN_REFERENCE)) {
				ref = C_REF_P(retval_ref);
			} else {
				/* A cast might happen - unwrap the reference if this is a by-value return */
				if (C_REFCOUNT_P(retval_ref) == 1) {
					ZVAL_UNREF(retval_ref);
				} else {
					C_DELREF_P(retval_ref);
					ZVAL_COPY(retval_ref, retval_ptr);
				}
				retval_ptr = retval_ref;
			}
		}

		SAVE_OPLINE();
		if (UNEXPECTED(!crex_check_type_slow(&ret_info->type, retval_ptr, ref, cache_slot, 1, 0))) {
			crex_verify_return_error(EX(func), retval_ptr);
			HANDLE_EXCEPTION();
		}
		CREX_VM_NEXT_OPCODE();
#endif
	}
}

CREX_VM_COLD_HANDLER(201, CREX_VERIFY_NEVER_TYPE, UNUSED, UNUSED)
{
	SAVE_OPLINE();
	crex_verify_never_error(EX(func));
	HANDLE_EXCEPTION();
}

CREX_VM_INLINE_HANDLER(62, CREX_RETURN, CONST|TMP|VAR|CV, ANY, SPEC(OBSERVER))
{
	USE_OPLINE
	zval *retval_ptr;
	zval *return_value;
	CREX_OBSERVER_USE_RETVAL;

	retval_ptr = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	return_value = EX(return_value);
	CREX_OBSERVER_SET_RETVAL();
	if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_INFO_P(retval_ptr) == IS_UNDEF)) {
		SAVE_OPLINE();
		retval_ptr = ZVAL_UNDEFINED_OP1();
		if (return_value) {
			ZVAL_NULL(return_value);
		}
	} else if (!return_value) {
		if (OP1_TYPE & (IS_VAR|IS_TMP_VAR)) {
			if (C_REFCOUNTED_P(retval_ptr) && !C_DELREF_P(retval_ptr)) {
				SAVE_OPLINE();
				rc_dtor_func(C_COUNTED_P(retval_ptr));
			}
		}
	} else {
		if ((OP1_TYPE & (IS_CONST|IS_TMP_VAR))) {
			ZVAL_COPY_VALUE(return_value, retval_ptr);
			if (OP1_TYPE == IS_CONST) {
				if (UNEXPECTED(C_OPT_REFCOUNTED_P(return_value))) {
					C_ADDREF_P(return_value);
				}
			}
		} else if (OP1_TYPE == IS_CV) {
			do {
				if (C_OPT_REFCOUNTED_P(retval_ptr)) {
					if (EXPECTED(!C_OPT_ISREF_P(retval_ptr))) {
						if (EXPECTED(!(EX_CALL_INFO() & (CREX_CALL_CODE|CREX_CALL_OBSERVED)))) {
							crex_refcounted *ref = C_COUNTED_P(retval_ptr);
							ZVAL_COPY_VALUE(return_value, retval_ptr);
							if (GC_MAY_LEAK(ref)) {
								SAVE_OPLINE();
								gc_possible_root(ref);
							}
							ZVAL_NULL(retval_ptr);
							break;
						} else {
							C_ADDREF_P(retval_ptr);
						}
					} else {
						retval_ptr = C_REFVAL_P(retval_ptr);
						if (C_OPT_REFCOUNTED_P(retval_ptr)) {
							C_ADDREF_P(retval_ptr);
						}
					}
				}
				ZVAL_COPY_VALUE(return_value, retval_ptr);
			} while (0);
		} else /* if (OP1_TYPE == IS_VAR) */ {
			if (UNEXPECTED(C_ISREF_P(retval_ptr))) {
				crex_refcounted *ref = C_COUNTED_P(retval_ptr);

				retval_ptr = C_REFVAL_P(retval_ptr);
				ZVAL_COPY_VALUE(return_value, retval_ptr);
				if (UNEXPECTED(GC_DELREF(ref) == 0)) {
					efree_size(ref, sizeof(crex_reference));
				} else if (C_OPT_REFCOUNTED_P(retval_ptr)) {
					C_ADDREF_P(retval_ptr);
				}
			} else {
				ZVAL_COPY_VALUE(return_value, retval_ptr);
			}
		}
	}
	CREX_OBSERVER_SAVE_OPLINE();
	CREX_OBSERVER_FCALL_END(execute_data, return_value);
	CREX_OBSERVER_FREE_RETVAL();
	CREX_VM_DISPATCH_TO_HELPER(crex_leave_helper);
}

CREX_VM_COLD_CONST_HANDLER(111, CREX_RETURN_BY_REF, CONST|TMP|VAR|CV, ANY, SRC, SPEC(OBSERVER))
{
	USE_OPLINE
	zval *retval_ptr;
	zval *return_value;
	CREX_OBSERVER_USE_RETVAL;

	SAVE_OPLINE();

	return_value = EX(return_value);
	CREX_OBSERVER_SET_RETVAL();
	do {
		if ((OP1_TYPE & (IS_CONST|IS_TMP_VAR)) ||
		    (OP1_TYPE == IS_VAR && opline->extended_value == CREX_RETURNS_VALUE)) {
			/* Not supposed to happen, but we'll allow it */
			crex_error(E_NOTICE, "Only variable references should be returned by reference");

			retval_ptr = GET_OP1_ZVAL_PTR(BP_VAR_R);
			if (!return_value) {
				FREE_OP1();
			} else {
				if (OP1_TYPE == IS_VAR && UNEXPECTED(C_ISREF_P(retval_ptr))) {
					ZVAL_COPY_VALUE(return_value, retval_ptr);
					break;
				}

				ZVAL_NEW_REF(return_value, retval_ptr);
				if (OP1_TYPE == IS_CONST) {
					C_TRY_ADDREF_P(retval_ptr);
				}
			}
			break;
		}

		retval_ptr = GET_OP1_ZVAL_PTR_PTR(BP_VAR_W);

		if (OP1_TYPE == IS_VAR) {
			CREX_ASSERT(retval_ptr != &EG(uninitialized_zval));
			if (opline->extended_value == CREX_RETURNS_FUNCTION && !C_ISREF_P(retval_ptr)) {
				crex_error(E_NOTICE, "Only variable references should be returned by reference");
				if (return_value) {
					ZVAL_NEW_REF(return_value, retval_ptr);
				} else {
					FREE_OP1();
				}
				break;
			}
		}

		if (return_value) {
			if (C_ISREF_P(retval_ptr)) {
				C_ADDREF_P(retval_ptr);
			} else {
				ZVAL_MAKE_REF_EX(retval_ptr, 2);
			}
			ZVAL_REF(return_value, C_REF_P(retval_ptr));
		}

		FREE_OP1();
	} while (0);

	CREX_OBSERVER_FCALL_END(execute_data, return_value);
	CREX_OBSERVER_FREE_RETVAL();
	CREX_VM_DISPATCH_TO_HELPER(crex_leave_helper);
}

CREX_VM_HANDLER(139, CREX_GENERATOR_CREATE, ANY, ANY)
{
	zval *return_value = EX(return_value);

	if (EXPECTED(return_value)) {
		USE_OPLINE
		crex_generator *generator;
		crex_execute_data *gen_execute_data;
		uint32_t num_args, used_stack, call_info;

		SAVE_OPLINE();
		object_init_ex(return_value, crex_ce_generator);

		/*
		 * Normally the execute_data is allocated on the VM stack (because it does
		 * not actually do any allocation and thus is faster). For generators
		 * though this behavior would be suboptimal, because the (rather large)
		 * structure would have to be copied back and forth every time execution is
		 * suspended or resumed. That's why for generators the execution context
		 * is allocated on heap.
		 */
		num_args = EX_NUM_ARGS();
		if (EXPECTED(num_args <= EX(func)->op_array.num_args)) {
			used_stack = (CREX_CALL_FRAME_SLOT + EX(func)->op_array.last_var + EX(func)->op_array.T) * sizeof(zval);
			gen_execute_data = (crex_execute_data*)emalloc(used_stack);
			used_stack = (CREX_CALL_FRAME_SLOT + EX(func)->op_array.last_var) * sizeof(zval);
		} else {
			used_stack = (CREX_CALL_FRAME_SLOT + num_args + EX(func)->op_array.last_var + EX(func)->op_array.T - EX(func)->op_array.num_args) * sizeof(zval);
			gen_execute_data = (crex_execute_data*)emalloc(used_stack);
		}
		memcpy(gen_execute_data, execute_data, used_stack);

		/* Save execution context in generator object. */
		generator = (crex_generator *) C_OBJ_P(EX(return_value));
		generator->execute_data = gen_execute_data;
		generator->frozen_call_stack = NULL;
		generator->execute_fake.opline = NULL;
		generator->execute_fake.func = NULL;
		generator->execute_fake.prev_execute_data = NULL;
		ZVAL_OBJ(&generator->execute_fake.This, (crex_object *) generator);

		gen_execute_data->opline = opline + 1;
		/* EX(return_value) keeps pointer to crex_object (not a real zval) */
		gen_execute_data->return_value = (zval*)generator;
		call_info = C_TYPE_INFO(EX(This));
		if ((call_info & C_TYPE_MASK) == IS_OBJECT
		 && (!(call_info & (CREX_CALL_CLOSURE|CREX_CALL_RELEASE_THIS))
			 /* Bug #72523 */
			|| UNEXPECTED(crex_execute_ex != execute_ex))) {
			CREX_ADD_CALL_FLAG_EX(call_info, CREX_CALL_RELEASE_THIS);
			C_ADDREF(gen_execute_data->This);
		}
		CREX_ADD_CALL_FLAG_EX(call_info, (CREX_CALL_TOP_FUNCTION | CREX_CALL_ALLOCATED | CREX_CALL_GENERATOR));
		C_TYPE_INFO(gen_execute_data->This) = call_info;
		gen_execute_data->prev_execute_data = NULL;

		call_info = EX_CALL_INFO();
		EG(current_execute_data) = EX(prev_execute_data);
		if (EXPECTED(!(call_info & (CREX_CALL_TOP|CREX_CALL_ALLOCATED)))) {
			EG(vm_stack_top) = (zval*)execute_data;
			execute_data = EX(prev_execute_data);
			LOAD_NEXT_OPLINE();
			CREX_VM_LEAVE();
		} else if (EXPECTED(!(call_info & CREX_CALL_TOP))) {
			crex_execute_data *old_execute_data = execute_data;
			execute_data = EX(prev_execute_data);
			crex_vm_stack_free_call_frame_ex(call_info, old_execute_data);
			LOAD_NEXT_OPLINE();
			CREX_VM_LEAVE();
		} else {
			CREX_VM_RETURN();
		}
	} else {
		CREX_VM_DISPATCH_TO_HELPER(crex_leave_helper);
	}
}

CREX_VM_HANDLER(161, CREX_GENERATOR_RETURN, CONST|TMP|VAR|CV, ANY, SPEC(OBSERVER))
{
	USE_OPLINE
	zval *retval;

	crex_generator *generator = crex_get_running_generator(EXECUTE_DATA_C);

	SAVE_OPLINE();
	retval = GET_OP1_ZVAL_PTR(BP_VAR_R);

	/* Copy return value into generator->retval */
	if ((OP1_TYPE & (IS_CONST|IS_TMP_VAR))) {
		ZVAL_COPY_VALUE(&generator->retval, retval);
		if (OP1_TYPE == IS_CONST) {
			if (UNEXPECTED(C_OPT_REFCOUNTED(generator->retval))) {
				C_ADDREF(generator->retval);
			}
		}
	} else if (OP1_TYPE == IS_CV) {
		ZVAL_COPY_DEREF(&generator->retval, retval);
	} else /* if (OP1_TYPE == IS_VAR) */ {
		if (UNEXPECTED(C_ISREF_P(retval))) {
			crex_refcounted *ref = C_COUNTED_P(retval);

			retval = C_REFVAL_P(retval);
			ZVAL_COPY_VALUE(&generator->retval, retval);
			if (UNEXPECTED(GC_DELREF(ref) == 0)) {
				efree_size(ref, sizeof(crex_reference));
			} else if (C_OPT_REFCOUNTED_P(retval)) {
				C_ADDREF_P(retval);
			}
		} else {
			ZVAL_COPY_VALUE(&generator->retval, retval);
		}
	}

	CREX_OBSERVER_FCALL_END(generator->execute_data, &generator->retval);

	EG(current_execute_data) = EX(prev_execute_data);

	/* Close the generator to free up resources */
	crex_generator_close(generator, 1);

	/* Pass execution back to handling code */
	CREX_VM_RETURN();
}

CREX_VM_COLD_CONST_HANDLER(108, CREX_THROW, CONST|TMPVAR|CV, ANY)
{
	USE_OPLINE
	zval *value;

	SAVE_OPLINE();
	value = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);

	do {
		if (OP1_TYPE == IS_CONST || UNEXPECTED(C_TYPE_P(value) != IS_OBJECT)) {
			if ((OP1_TYPE & (IS_VAR|IS_CV)) && C_ISREF_P(value)) {
				value = C_REFVAL_P(value);
				if (EXPECTED(C_TYPE_P(value) == IS_OBJECT)) {
					break;
				}
			}
			if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(value) == IS_UNDEF)) {
				ZVAL_UNDEFINED_OP1();
				if (UNEXPECTED(EG(exception) != NULL)) {
					HANDLE_EXCEPTION();
				}
			}
			crex_throw_error(NULL, "Can only throw objects");
			FREE_OP1();
			HANDLE_EXCEPTION();
		}
	} while (0);

	crex_exception_save();
	C_TRY_ADDREF_P(value);
	crex_throw_exception_object(value);
	crex_exception_restore();
	FREE_OP1();
	HANDLE_EXCEPTION();
}

CREX_VM_HANDLER(107, CREX_CATCH, CONST, JMP_ADDR, LAST_CATCH|CACHE_SLOT)
{
	USE_OPLINE
	crex_class_entry *ce, *catch_ce;
	crex_object *exception;

	SAVE_OPLINE();
	/* Check whether an exception has been thrown, if not, jump over code */
	crex_exception_restore();
	if (EG(exception) == NULL) {
		CREX_VM_JMP_EX(OP_JMP_ADDR(opline, opline->op2), 0);
	}
	catch_ce = CACHED_PTR(opline->extended_value & ~CREX_LAST_CATCH);
	if (UNEXPECTED(catch_ce == NULL)) {
		catch_ce = crex_fetch_class_by_name(C_STR_P(RT_CONSTANT(opline, opline->op1)), C_STR_P(RT_CONSTANT(opline, opline->op1) + 1), CREX_FETCH_CLASS_NO_AUTOLOAD | CREX_FETCH_CLASS_SILENT);

		CACHE_PTR(opline->extended_value & ~CREX_LAST_CATCH, catch_ce);
	}
	ce = EG(exception)->ce;

#ifdef HAVE_DTRACE
	if (DTRACE_EXCEPTION_CAUGHT_ENABLED()) {
		DTRACE_EXCEPTION_CAUGHT((char *)ce->name);
	}
#endif /* HAVE_DTRACE */

	if (ce != catch_ce) {
		if (!catch_ce || !instanceof_function(ce, catch_ce)) {
			if (opline->extended_value & CREX_LAST_CATCH) {
				crex_rethrow_exception(execute_data);
				HANDLE_EXCEPTION();
			}
			CREX_VM_JMP_EX(OP_JMP_ADDR(opline, opline->op2), 0);
		}
	}

	exception = EG(exception);
	EG(exception) = NULL;
	if (RETURN_VALUE_USED(opline)) {
		/* Always perform a strict assignment. There is a reasonable expectation that if you
		 * write "catch (Exception $e)" then $e will actually be instanceof Exception. As such,
		 * we should not permit coercion to string here. */
		zval tmp;
		ZVAL_OBJ(&tmp, exception);
		crex_assign_to_variable(EX_VAR(opline->result.var), &tmp, IS_TMP_VAR, /* strict */ 1);
	} else {
		OBJ_RELEASE(exception);
	}
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HOT_HANDLER(65, CREX_SEND_VAL, CONST|TMPVAR, CONST|UNUSED|NUM)
{
	USE_OPLINE
	zval *value, *arg;

	if (OP2_TYPE == IS_CONST) {
		SAVE_OPLINE();
		crex_string *arg_name = C_STR_P(RT_CONSTANT(opline, opline->op2));
		uint32_t arg_num;
		arg = crex_handle_named_arg(&EX(call), arg_name, &arg_num, CACHE_ADDR(opline->result.num));
		if (UNEXPECTED(!arg)) {
			FREE_OP1();
			HANDLE_EXCEPTION();
		}
	} else {
		arg = CREX_CALL_VAR(EX(call), opline->result.var);
	}

	value = GET_OP1_ZVAL_PTR(BP_VAR_R);
	ZVAL_COPY_VALUE(arg, value);
	if (OP1_TYPE == IS_CONST) {
		if (UNEXPECTED(C_OPT_REFCOUNTED_P(arg))) {
			C_ADDREF_P(arg);
		}
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_COLD_HELPER(crex_cannot_pass_by_ref_helper, ANY, ANY, uint32_t _arg_num, zval *_arg)
{
	USE_OPLINE

	SAVE_OPLINE();

	crex_cannot_pass_by_reference(_arg_num);
	FREE_OP1();
	ZVAL_UNDEF(_arg);
	HANDLE_EXCEPTION();
}

CREX_VM_HOT_SEND_HANDLER(116, CREX_SEND_VAL_EX, CONST|TMP, CONST|UNUSED|NUM, SPEC(QUICK_ARG))
{
	USE_OPLINE
	zval *value, *arg;
	uint32_t arg_num;

	if (OP2_TYPE == IS_CONST) {
		SAVE_OPLINE();
		crex_string *arg_name = C_STR_P(RT_CONSTANT(opline, opline->op2));
		arg = crex_handle_named_arg(&EX(call), arg_name, &arg_num, CACHE_ADDR(opline->result.num));
		if (UNEXPECTED(!arg)) {
			FREE_OP1();
			HANDLE_EXCEPTION();
		}
	} else {
		arg = CREX_CALL_VAR(EX(call), opline->result.var);
		arg_num = opline->op2.num;
	}

	if (EXPECTED(arg_num <= MAX_ARG_FLAG_NUM)) {
		if (QUICK_ARG_MUST_BE_SENT_BY_REF(EX(call)->func, arg_num)) {
			CREX_VM_C_GOTO(send_val_by_ref);
		}
	} else if (ARG_MUST_BE_SENT_BY_REF(EX(call)->func, arg_num)) {
CREX_VM_C_LABEL(send_val_by_ref):
		CREX_VM_DISPATCH_TO_HELPER(crex_cannot_pass_by_ref_helper, _arg_num, arg_num, _arg, arg);
	}
	value = GET_OP1_ZVAL_PTR(BP_VAR_R);
	ZVAL_COPY_VALUE(arg, value);
	if (OP1_TYPE == IS_CONST) {
		if (UNEXPECTED(C_OPT_REFCOUNTED_P(arg))) {
			C_ADDREF_P(arg);
		}
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_HANDLER(117, CREX_SEND_VAR, VAR|CV, CONST|UNUSED|NUM)
{
	USE_OPLINE
	zval *varptr, *arg;

	if (OP2_TYPE == IS_CONST) {
		SAVE_OPLINE();
		crex_string *arg_name = C_STR_P(RT_CONSTANT(opline, opline->op2));
		uint32_t arg_num;
		arg = crex_handle_named_arg(&EX(call), arg_name, &arg_num, CACHE_ADDR(opline->result.num));
		if (UNEXPECTED(!arg)) {
			FREE_OP1();
			HANDLE_EXCEPTION();
		}
	} else {
		arg = CREX_CALL_VAR(EX(call), opline->result.var);
	}

	varptr = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_INFO_P(varptr) == IS_UNDEF)) {
		SAVE_OPLINE();
		ZVAL_UNDEFINED_OP1();
		ZVAL_NULL(arg);
		CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
	}

	if (OP1_TYPE == IS_CV) {
		ZVAL_COPY_DEREF(arg, varptr);
	} else /* if (OP1_TYPE == IS_VAR) */ {
		if (UNEXPECTED(C_ISREF_P(varptr))) {
			crex_refcounted *ref = C_COUNTED_P(varptr);

			varptr = C_REFVAL_P(varptr);
			ZVAL_COPY_VALUE(arg, varptr);
			if (UNEXPECTED(GC_DELREF(ref) == 0)) {
				efree_size(ref, sizeof(crex_reference));
			} else if (C_OPT_REFCOUNTED_P(arg)) {
				C_ADDREF_P(arg);
			}
		} else {
			ZVAL_COPY_VALUE(arg, varptr);
		}
	}

	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(106, CREX_SEND_VAR_NO_REF, VAR, CONST|UNUSED|NUM)
{
	USE_OPLINE
	zval *varptr, *arg;

	if (OP2_TYPE == IS_CONST) {
		SAVE_OPLINE();
		crex_string *arg_name = C_STR_P(RT_CONSTANT(opline, opline->op2));
		uint32_t arg_num;
		arg = crex_handle_named_arg(&EX(call), arg_name, &arg_num, CACHE_ADDR(opline->result.num));
		if (UNEXPECTED(!arg)) {
			FREE_OP1();
			HANDLE_EXCEPTION();
		}
	} else {
		arg = CREX_CALL_VAR(EX(call), opline->result.var);
	}

	varptr = GET_OP1_ZVAL_PTR(BP_VAR_R);
	ZVAL_COPY_VALUE(arg, varptr);

	if (EXPECTED(C_ISREF_P(varptr))) {
		CREX_VM_NEXT_OPCODE();
	}

	SAVE_OPLINE();
	ZVAL_NEW_REF(arg, arg);
	crex_error(E_NOTICE, "Only variables should be passed by reference");
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HOT_SEND_HANDLER(50, CREX_SEND_VAR_NO_REF_EX, VAR, CONST|UNUSED|NUM, SPEC(QUICK_ARG))
{
	USE_OPLINE
	zval *varptr, *arg;
	uint32_t arg_num;

	if (OP2_TYPE == IS_CONST) {
		SAVE_OPLINE();
		crex_string *arg_name = C_STR_P(RT_CONSTANT(opline, opline->op2));
		arg = crex_handle_named_arg(&EX(call), arg_name, &arg_num, CACHE_ADDR(opline->result.num));
		if (UNEXPECTED(!arg)) {
			FREE_OP1();
			HANDLE_EXCEPTION();
		}
	} else {
		arg = CREX_CALL_VAR(EX(call), opline->result.var);
		arg_num = opline->op2.num;
	}

	if (EXPECTED(arg_num <= MAX_ARG_FLAG_NUM)) {
		if (!QUICK_ARG_SHOULD_BE_SENT_BY_REF(EX(call)->func, arg_num)) {
			CREX_VM_C_GOTO(send_var);
		}

		varptr = GET_OP1_ZVAL_PTR(BP_VAR_R);
		ZVAL_COPY_VALUE(arg, varptr);

		if (EXPECTED(C_ISREF_P(varptr) ||
		    QUICK_ARG_MAY_BE_SENT_BY_REF(EX(call)->func, arg_num))) {
			CREX_VM_NEXT_OPCODE();
		}
	} else {
		if (!ARG_SHOULD_BE_SENT_BY_REF(EX(call)->func, arg_num)) {
			CREX_VM_C_GOTO(send_var);
		}

		varptr = GET_OP1_ZVAL_PTR(BP_VAR_R);
		ZVAL_COPY_VALUE(arg, varptr);

		if (EXPECTED(C_ISREF_P(varptr) ||
		    ARG_MAY_BE_SENT_BY_REF(EX(call)->func, arg_num))) {
			CREX_VM_NEXT_OPCODE();
		}
	}

	SAVE_OPLINE();
	ZVAL_NEW_REF(arg, arg);
	crex_error(E_NOTICE, "Only variables should be passed by reference");
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();

CREX_VM_C_LABEL(send_var):
	varptr = GET_OP1_ZVAL_PTR(BP_VAR_R);
	if (UNEXPECTED(C_ISREF_P(varptr))) {
		crex_refcounted *ref = C_COUNTED_P(varptr);

		varptr = C_REFVAL_P(varptr);
		ZVAL_COPY_VALUE(arg, varptr);
		if (UNEXPECTED(GC_DELREF(ref) == 0)) {
			efree_size(ref, sizeof(crex_reference));
		} else if (C_OPT_REFCOUNTED_P(arg)) {
			C_ADDREF_P(arg);
		}
	} else {
		ZVAL_COPY_VALUE(arg, varptr);
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(67, CREX_SEND_REF, VAR|CV, CONST|UNUSED|NUM)
{
	USE_OPLINE
	zval *varptr, *arg;

	SAVE_OPLINE();
	if (OP2_TYPE == IS_CONST) {
		crex_string *arg_name = C_STR_P(RT_CONSTANT(opline, opline->op2));
		uint32_t arg_num;
		arg = crex_handle_named_arg(&EX(call), arg_name, &arg_num, CACHE_ADDR(opline->result.num));
		if (UNEXPECTED(!arg)) {
			FREE_OP1();
			HANDLE_EXCEPTION();
		}
	} else {
		arg = CREX_CALL_VAR(EX(call), opline->result.var);
	}

	varptr = GET_OP1_ZVAL_PTR_PTR(BP_VAR_W);
	if (C_ISREF_P(varptr)) {
		C_ADDREF_P(varptr);
	} else {
		ZVAL_MAKE_REF_EX(varptr, 2);
	}
	ZVAL_REF(arg, C_REF_P(varptr));

	FREE_OP1();
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_SEND_HANDLER(66, CREX_SEND_VAR_EX, VAR|CV, CONST|UNUSED|NUM, SPEC(QUICK_ARG))
{
	USE_OPLINE
	zval *varptr, *arg;
	uint32_t arg_num;

	if (OP2_TYPE == IS_CONST) {
		SAVE_OPLINE();
		crex_string *arg_name = C_STR_P(RT_CONSTANT(opline, opline->op2));
		arg = crex_handle_named_arg(&EX(call), arg_name, &arg_num, CACHE_ADDR(opline->result.num));
		if (UNEXPECTED(!arg)) {
			FREE_OP1();
			HANDLE_EXCEPTION();
		}
	} else {
		arg = CREX_CALL_VAR(EX(call), opline->result.var);
		arg_num = opline->op2.num;
	}

	if (EXPECTED(arg_num <= MAX_ARG_FLAG_NUM)) {
		if (QUICK_ARG_SHOULD_BE_SENT_BY_REF(EX(call)->func, arg_num)) {
			CREX_VM_C_GOTO(send_var_by_ref);
		}
	} else if (ARG_SHOULD_BE_SENT_BY_REF(EX(call)->func, arg_num)) {
CREX_VM_C_LABEL(send_var_by_ref):
		varptr = GET_OP1_ZVAL_PTR_PTR(BP_VAR_W);
		if (C_ISREF_P(varptr)) {
			C_ADDREF_P(varptr);
		} else {
			ZVAL_MAKE_REF_EX(varptr, 2);
		}
		ZVAL_REF(arg, C_REF_P(varptr));

		FREE_OP1();
		CREX_VM_NEXT_OPCODE();
	}

	varptr = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_INFO_P(varptr) == IS_UNDEF)) {
		SAVE_OPLINE();
		ZVAL_UNDEFINED_OP1();
		ZVAL_NULL(arg);
		CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
	}

	if (OP1_TYPE == IS_CV) {
		ZVAL_COPY_DEREF(arg, varptr);
	} else /* if (OP1_TYPE == IS_VAR) */ {
		if (UNEXPECTED(C_ISREF_P(varptr))) {
			crex_refcounted *ref = C_COUNTED_P(varptr);

			varptr = C_REFVAL_P(varptr);
			ZVAL_COPY_VALUE(arg, varptr);
			if (UNEXPECTED(GC_DELREF(ref) == 0)) {
				efree_size(ref, sizeof(crex_reference));
			} else if (C_OPT_REFCOUNTED_P(arg)) {
				C_ADDREF_P(arg);
			}
		} else {
			ZVAL_COPY_VALUE(arg, varptr);
		}
	}

	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_SEND_HANDLER(100, CREX_CHECK_FUNC_ARG, UNUSED, CONST|UNUSED|NUM, SPEC(QUICK_ARG))
{
	USE_OPLINE
	uint32_t arg_num;

	if (OP2_TYPE == IS_CONST) {
		crex_string *arg_name = C_STR_P(RT_CONSTANT(opline, opline->op2));
		arg_num = crex_get_arg_offset_by_name(
			EX(call)->func, arg_name, CACHE_ADDR(opline->result.num)) + 1;
		if (UNEXPECTED(arg_num == 0)) {
			/* Treat this as a by-value argument, and throw an error during SEND. */
			CREX_DEL_CALL_FLAG(EX(call), CREX_CALL_SEND_ARG_BY_REF);
			CREX_VM_NEXT_OPCODE();
		}
	} else {
		arg_num = opline->op2.num;
	}

	if (EXPECTED(arg_num <= MAX_ARG_FLAG_NUM)) {
		if (QUICK_ARG_SHOULD_BE_SENT_BY_REF(EX(call)->func, arg_num)) {
			CREX_ADD_CALL_FLAG(EX(call), CREX_CALL_SEND_ARG_BY_REF);
		} else {
			CREX_DEL_CALL_FLAG(EX(call), CREX_CALL_SEND_ARG_BY_REF);
		}
	} else if (ARG_SHOULD_BE_SENT_BY_REF(EX(call)->func, arg_num)) {
		CREX_ADD_CALL_FLAG(EX(call), CREX_CALL_SEND_ARG_BY_REF);
	} else {
		CREX_DEL_CALL_FLAG(EX(call), CREX_CALL_SEND_ARG_BY_REF);
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_HANDLER(185, CREX_SEND_FUNC_ARG, VAR, CONST|UNUSED|NUM)
{
	USE_OPLINE
	zval *varptr, *arg;

	if (OP2_TYPE == IS_CONST) {
		// TODO: Would it make sense to share the cache slot with CHECK_FUNC_ARG?
		SAVE_OPLINE();
		crex_string *arg_name = C_STR_P(RT_CONSTANT(opline, opline->op2));
		uint32_t arg_num;
		arg = crex_handle_named_arg(&EX(call), arg_name, &arg_num, CACHE_ADDR(opline->result.num));
		if (UNEXPECTED(!arg)) {
			FREE_OP1();
			HANDLE_EXCEPTION();
		}
	} else {
		arg = CREX_CALL_VAR(EX(call), opline->result.var);
	}

	if (UNEXPECTED(CREX_CALL_INFO(EX(call)) & CREX_CALL_SEND_ARG_BY_REF)) {
		varptr = GET_OP1_ZVAL_PTR_PTR(BP_VAR_W);
		if (C_ISREF_P(varptr)) {
			C_ADDREF_P(varptr);
		} else {
			ZVAL_MAKE_REF_EX(varptr, 2);
		}
		ZVAL_REF(arg, C_REF_P(varptr));

		FREE_OP1();
		CREX_VM_NEXT_OPCODE();
	}

	varptr = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);

	if (UNEXPECTED(C_ISREF_P(varptr))) {
		crex_refcounted *ref = C_COUNTED_P(varptr);

		varptr = C_REFVAL_P(varptr);
		ZVAL_COPY_VALUE(arg, varptr);
		if (UNEXPECTED(GC_DELREF(ref) == 0)) {
			efree_size(ref, sizeof(crex_reference));
		} else if (C_OPT_REFCOUNTED_P(arg)) {
			C_ADDREF_P(arg);
		}
	} else {
		ZVAL_COPY_VALUE(arg, varptr);
	}

	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(165, CREX_SEND_UNPACK, ANY, ANY)
{
	USE_OPLINE
	zval *args;
	uint32_t arg_num;

	SAVE_OPLINE();
	args = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	arg_num = CREX_CALL_NUM_ARGS(EX(call)) + 1;

CREX_VM_C_LABEL(send_again):
	if (EXPECTED(C_TYPE_P(args) == IS_ARRAY)) {
		HashTable *ht = C_ARRVAL_P(args);
		zval *arg, *top;
		crex_string *name;
		bool have_named_params = 0;

		crex_vm_stack_extend_call_frame(&EX(call), arg_num - 1, crex_hash_num_elements(ht));

		// TODO: Speed this up using a flag that specifies whether there are any ref parameters.
		if ((OP1_TYPE & (IS_VAR|IS_CV)) && C_REFCOUNT_P(args) > 1) {
			uint32_t tmp_arg_num = arg_num;
			bool separate = 0;

			/* check if any of arguments are going to be passed by reference */
			CREX_HASH_FOREACH_STR_KEY_VAL(ht, name, arg) {
				if (UNEXPECTED(name)) {
					void *cache_slot[2] = {NULL, NULL};
					tmp_arg_num = crex_get_arg_offset_by_name(
						EX(call)->func, name, cache_slot) + 1;
				}
				if (ARG_SHOULD_BE_SENT_BY_REF(EX(call)->func, tmp_arg_num)) {
					separate = 1;
					break;
				}
				tmp_arg_num++;
			} CREX_HASH_FOREACH_END();
			if (separate) {
				SEPARATE_ARRAY(args);
				ht = C_ARRVAL_P(args);
			}
		}

		CREX_HASH_FOREACH_STR_KEY_VAL(ht, name, arg) {
			if (UNEXPECTED(name)) {
				void *cache_slot[2] = {NULL, NULL};
				have_named_params = 1;
				top = crex_handle_named_arg(&EX(call), name, &arg_num, cache_slot);
				if (UNEXPECTED(!top)) {
					FREE_OP1();
					HANDLE_EXCEPTION();
				}
			} else {
				if (have_named_params) {
					crex_throw_error(NULL,
						"Cannot use positional argument after named argument during unpacking");
					FREE_OP1();
					HANDLE_EXCEPTION();
				}

				top = CREX_CALL_ARG(EX(call), arg_num);
				CREX_CALL_NUM_ARGS(EX(call))++;
			}

			if (ARG_SHOULD_BE_SENT_BY_REF(EX(call)->func, arg_num)) {
				if (C_ISREF_P(arg)) {
					C_ADDREF_P(arg);
					ZVAL_REF(top, C_REF_P(arg));
				} else if (OP1_TYPE & (IS_VAR|IS_CV)) {
					/* array is already separated above */
					ZVAL_MAKE_REF_EX(arg, 2);
					ZVAL_REF(top, C_REF_P(arg));
				} else {
					C_TRY_ADDREF_P(arg);
					ZVAL_NEW_REF(top, arg);
				}
			} else {
				ZVAL_COPY_DEREF(top, arg);
			}

			arg_num++;
		} CREX_HASH_FOREACH_END();

	} else if (EXPECTED(C_TYPE_P(args) == IS_OBJECT)) {
		crex_class_entry *ce = C_OBJCE_P(args);
		crex_object_iterator *iter;
		bool have_named_params = 0;

		if (!ce || !ce->get_iterator) {
			crex_type_error("Only arrays and Traversables can be unpacked");
		} else {

			iter = ce->get_iterator(ce, args, 0);
			if (UNEXPECTED(!iter)) {
				FREE_OP1();
				if (!EG(exception)) {
					crex_throw_exception_ex(
						NULL, 0, "Object of type %s did not create an Iterator", ZSTR_VAL(ce->name)
					);
				}
				HANDLE_EXCEPTION();
			}

			const crex_object_iterator_funcs *funcs = iter->funcs;
			if (funcs->rewind) {
				funcs->rewind(iter);
			}

			for (; funcs->valid(iter) == SUCCESS; ++arg_num) {
				zval *arg, *top;

				if (UNEXPECTED(EG(exception) != NULL)) {
					break;
				}

				arg = funcs->get_current_data(iter);
				if (UNEXPECTED(EG(exception) != NULL)) {
					break;
				}

				crex_string *name = NULL;
				if (funcs->get_current_key) {
					zval key;
					funcs->get_current_key(iter, &key);
					if (UNEXPECTED(EG(exception) != NULL)) {
						break;
					}

					if (UNEXPECTED(C_TYPE(key) != IS_LONG)) {
						if (UNEXPECTED(C_TYPE(key) != IS_STRING)) {
							crex_throw_error(NULL,
								"Keys must be of type int|string during argument unpacking");
							zval_ptr_dtor(&key);
							break;
						}

						name = C_STR_P(&key);
					}
				}

				if (UNEXPECTED(name)) {
					void *cache_slot[2] = {NULL, NULL};
					have_named_params = 1;
					top = crex_handle_named_arg(&EX(call), name, &arg_num, cache_slot);
					if (UNEXPECTED(!top)) {
						crex_string_release(name);
						break;
					}

					ZVAL_DEREF(arg);
					C_TRY_ADDREF_P(arg);

					if (ARG_MUST_BE_SENT_BY_REF(EX(call)->func, arg_num)) {
						crex_error(
							E_WARNING, "Cannot pass by-reference argument %d of %s%s%s()"
							" by unpacking a Traversable, passing by-value instead", arg_num,
							EX(call)->func->common.scope ? ZSTR_VAL(EX(call)->func->common.scope->name) : "",
							EX(call)->func->common.scope ? "::" : "",
							ZSTR_VAL(EX(call)->func->common.function_name)
						);
						ZVAL_NEW_REF(top, arg);
					} else {
						ZVAL_COPY_VALUE(top, arg);
					}

					crex_string_release(name);
				} else {
					if (have_named_params) {
						crex_throw_error(NULL,
							"Cannot use positional argument after named argument during unpacking");
						break;
					}

					crex_vm_stack_extend_call_frame(&EX(call), arg_num - 1, 1);
					top = CREX_CALL_ARG(EX(call), arg_num);
					ZVAL_DEREF(arg);
					C_TRY_ADDREF_P(arg);

					if (ARG_MUST_BE_SENT_BY_REF(EX(call)->func, arg_num)) {
						crex_error(
							E_WARNING, "Cannot pass by-reference argument %d of %s%s%s()"
							" by unpacking a Traversable, passing by-value instead", arg_num,
							EX(call)->func->common.scope ? ZSTR_VAL(EX(call)->func->common.scope->name) : "",
							EX(call)->func->common.scope ? "::" : "",
							ZSTR_VAL(EX(call)->func->common.function_name)
						);
						ZVAL_NEW_REF(top, arg);
					} else {
						ZVAL_COPY_VALUE(top, arg);
					}

					CREX_CALL_NUM_ARGS(EX(call))++;
				}

				funcs->move_forward(iter);
			}

			crex_iterator_dtor(iter);
		}
	} else if (EXPECTED(C_ISREF_P(args))) {
		args = C_REFVAL_P(args);
		CREX_VM_C_GOTO(send_again);
	} else {
		if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(args) == IS_UNDEF)) {
			ZVAL_UNDEFINED_OP1();
		}
		crex_type_error("Only arrays and Traversables can be unpacked");
	}

	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(119, CREX_SEND_ARRAY, ANY, ANY, NUM)
{
	USE_OPLINE
	zval *args;

	SAVE_OPLINE();
	args = GET_OP1_ZVAL_PTR(BP_VAR_R);

	if (UNEXPECTED(C_TYPE_P(args) != IS_ARRAY)) {
		if ((OP1_TYPE & (IS_VAR|IS_CV)) && C_ISREF_P(args)) {
			args = C_REFVAL_P(args);
			if (EXPECTED(C_TYPE_P(args) == IS_ARRAY)) {
				CREX_VM_C_GOTO(send_array);
			}
		}
		crex_type_error("call_user_func_array(): Argument #2 ($args) must be of type array, %s given", crex_zval_value_name(args));
		FREE_OP2();
		FREE_OP1();
		HANDLE_EXCEPTION();
	} else {
		uint32_t arg_num;
		HashTable *ht;
		zval *arg, *param;

CREX_VM_C_LABEL(send_array):
		ht = C_ARRVAL_P(args);
		if (OP2_TYPE != IS_UNUSED) {
			/* We don't need to handle named params in this case,
			 * because array_slice() is called with $preserve_keys == false. */
			zval *op2 = GET_OP2_ZVAL_PTR_DEREF(BP_VAR_R);
			uint32_t skip = opline->extended_value;
			uint32_t count = crex_hash_num_elements(ht);
			crex_long len;
			if (EXPECTED(C_TYPE_P(op2) == IS_LONG)) {
				len = C_LVAL_P(op2);
			} else if (C_TYPE_P(op2) == IS_NULL) {
				len = count - skip;
			} else if (EX_USES_STRICT_TYPES()
					|| !crex_parse_arg_long_weak(op2, &len, /* arg_num */ 3)) {
				crex_type_error(
					"array_slice(): Argument #3 ($length) must be of type ?int, %s given",
					crex_zval_value_name(op2));
				FREE_OP2();
				FREE_OP1();
				HANDLE_EXCEPTION();
			}

			if (len < 0) {
				len += (crex_long)(count - skip);
			}
			if (skip < count && len > 0) {
				if (len > (crex_long)(count - skip)) {
					len = (crex_long)(count - skip);
				}
				crex_vm_stack_extend_call_frame(&EX(call), 0, len);
				arg_num = 1;
				param = CREX_CALL_ARG(EX(call), 1);
				CREX_HASH_FOREACH_VAL(ht, arg) {
					bool must_wrap = 0;
					if (skip > 0) {
						skip--;
						continue;
					} else if ((crex_long)(arg_num - 1) >= len) {
						break;
					} else if (ARG_SHOULD_BE_SENT_BY_REF(EX(call)->func, arg_num)) {
						if (UNEXPECTED(!C_ISREF_P(arg))) {
							if (!ARG_MAY_BE_SENT_BY_REF(EX(call)->func, arg_num)) {
								/* By-value send is not allowed -- emit a warning,
								 * but still perform the call. */
								crex_param_must_be_ref(EX(call)->func, arg_num);
								must_wrap = 1;
							}
						}
					} else {
						if (C_ISREF_P(arg) &&
						    !(EX(call)->func->common.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE)) {
							/* don't separate references for __call */
							arg = C_REFVAL_P(arg);
						}
					}
					if (EXPECTED(!must_wrap)) {
						ZVAL_COPY(param, arg);
					} else {
						C_TRY_ADDREF_P(arg);
						ZVAL_NEW_REF(param, arg);
					}
					CREX_CALL_NUM_ARGS(EX(call))++;
					arg_num++;
					param++;
				} CREX_HASH_FOREACH_END();
			}
			FREE_OP2();
		} else {
			crex_string *name;
			bool have_named_params;
			crex_vm_stack_extend_call_frame(&EX(call), 0, crex_hash_num_elements(ht));
			arg_num = 1;
			param = CREX_CALL_ARG(EX(call), 1);
			have_named_params = 0;
			CREX_HASH_FOREACH_STR_KEY_VAL(ht, name, arg) {
				if (name) {
					void *cache_slot[2] = {NULL, NULL};
					have_named_params = 1;
					param = crex_handle_named_arg(&EX(call), name, &arg_num, cache_slot);
					if (!param) {
						FREE_OP1();
						HANDLE_EXCEPTION();
					}
				} else if (have_named_params) {
					crex_throw_error(NULL,
						"Cannot use positional argument after named argument");
					FREE_OP1();
					HANDLE_EXCEPTION();
				}

				bool must_wrap = 0;
				if (ARG_SHOULD_BE_SENT_BY_REF(EX(call)->func, arg_num)) {
					if (UNEXPECTED(!C_ISREF_P(arg))) {
						if (!ARG_MAY_BE_SENT_BY_REF(EX(call)->func, arg_num)) {
							/* By-value send is not allowed -- emit a warning,
							 * but still perform the call. */
							crex_param_must_be_ref(EX(call)->func, arg_num);
							must_wrap = 1;
						}
					}
				} else {
					if (C_ISREF_P(arg) &&
					    !(EX(call)->func->common.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE)) {
						/* don't separate references for __call */
						arg = C_REFVAL_P(arg);
					}
				}

				if (EXPECTED(!must_wrap)) {
					ZVAL_COPY(param, arg);
				} else {
					C_TRY_ADDREF_P(arg);
					ZVAL_NEW_REF(param, arg);
				}
				if (!name) {
					CREX_CALL_NUM_ARGS(EX(call))++;
					arg_num++;
					param++;
				}
			} CREX_HASH_FOREACH_END();
		}
	}
	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(120, CREX_SEND_USER, CONST|TMP|VAR|CV, NUM)
{
	USE_OPLINE
	zval *arg, *param;

	SAVE_OPLINE();

	arg = GET_OP1_ZVAL_PTR_DEREF(BP_VAR_R);
	param = CREX_CALL_VAR(EX(call), opline->result.var);
	if (UNEXPECTED(ARG_MUST_BE_SENT_BY_REF(EX(call)->func, opline->op2.num))) {
		crex_param_must_be_ref(EX(call)->func, opline->op2.num);
		C_TRY_ADDREF_P(arg);
		ZVAL_NEW_REF(param, arg);
	} else {
		ZVAL_COPY(param, arg);
	}

	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HOT_HANDLER(199, CREX_CHECK_UNDEF_ARGS, UNUSED, UNUSED)
{
	USE_OPLINE

	crex_execute_data *call = execute_data->call;
	if (EXPECTED(!(CREX_CALL_INFO(call) & CREX_CALL_MAY_HAVE_UNDEF))) {
		CREX_VM_NEXT_OPCODE();
	}

	SAVE_OPLINE();
	crex_handle_undef_args(call);
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_COLD_HELPER(crex_missing_arg_helper, ANY, ANY)
{
#ifdef CREX_VM_IP_GLOBAL_REG
	USE_OPLINE

	SAVE_OPLINE();
#endif
	crex_missing_arg_error(execute_data);
	HANDLE_EXCEPTION();
}

CREX_VM_HELPER(crex_verify_recv_arg_type_helper, ANY, ANY, zval *op_1)
{
	USE_OPLINE

	SAVE_OPLINE();
	if (UNEXPECTED(!crex_verify_recv_arg_type(EX(func), opline->op1.num, op_1, CACHE_ADDR(opline->extended_value)))) {
		HANDLE_EXCEPTION();
	}

	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_HANDLER(63, CREX_RECV, NUM, UNUSED, CACHE_SLOT)
{
	USE_OPLINE
	uint32_t arg_num = opline->op1.num;
	zval *param;

	if (UNEXPECTED(arg_num > EX_NUM_ARGS())) {
		CREX_VM_DISPATCH_TO_HELPER(crex_missing_arg_helper);
	}

	param = EX_VAR(opline->result.var);

	if (UNEXPECTED(!(opline->op2.num & (1u << C_TYPE_P(param))))) {
		CREX_VM_DISPATCH_TO_HELPER(crex_verify_recv_arg_type_helper, op_1, param);
	}

	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_RECV, op->op2.num == MAY_BE_ANY, CREX_RECV_NOTYPE, NUM, NUM, CACHE_SLOT)
{
	USE_OPLINE
	uint32_t arg_num = opline->op1.num;

	if (UNEXPECTED(arg_num > EX_NUM_ARGS())) {
		CREX_VM_DISPATCH_TO_HELPER(crex_missing_arg_helper);
	}

	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_HANDLER(64, CREX_RECV_INIT, NUM, CONST, CACHE_SLOT)
{
	USE_OPLINE
	uint32_t arg_num;
	zval *param;

	CREX_VM_REPEATABLE_OPCODE

	arg_num = opline->op1.num;
	param = EX_VAR(opline->result.var);
	if (arg_num > EX_NUM_ARGS()) {
		zval *default_value = RT_CONSTANT(opline, opline->op2);

		if (C_OPT_TYPE_P(default_value) == IS_CONSTANT_AST) {
			zval *cache_val = (zval*)CACHE_ADDR(C_CACHE_SLOT_P(default_value));

			/* we keep in cache only not refcounted values */
			if (C_TYPE_P(cache_val) != IS_UNDEF) {
				ZVAL_COPY_VALUE(param, cache_val);
			} else {
				SAVE_OPLINE();
				ZVAL_COPY(param, default_value);
				crex_ast_evaluate_ctx ctx = {0};
				if (UNEXPECTED(zval_update_constant_with_ctx(param, EX(func)->op_array.scope, &ctx) != SUCCESS)) {
					zval_ptr_dtor_nogc(param);
					ZVAL_UNDEF(param);
					HANDLE_EXCEPTION();
				}
				if (!C_REFCOUNTED_P(param) && !ctx.had_side_effects) {
					ZVAL_COPY_VALUE(cache_val, param);
				}
			}
			CREX_VM_C_GOTO(recv_init_check_type);
		} else {
			ZVAL_COPY(param, default_value);
		}
	} else {
CREX_VM_C_LABEL(recv_init_check_type):
		if (UNEXPECTED((EX(func)->op_array.fn_flags & CREX_ACC_HAS_TYPE_HINTS) != 0)) {
			SAVE_OPLINE();
			if (UNEXPECTED(!crex_verify_recv_arg_type(EX(func), arg_num, param, CACHE_ADDR(opline->extended_value)))) {
				HANDLE_EXCEPTION();
			}
		}
	}

	CREX_VM_REPEAT_OPCODE(CREX_RECV_INIT);
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(164, CREX_RECV_VARIADIC, NUM, UNUSED, CACHE_SLOT)
{
	USE_OPLINE
	uint32_t arg_num = opline->op1.num;
	uint32_t arg_count = EX_NUM_ARGS();
	zval *params;

	SAVE_OPLINE();

	params = EX_VAR(opline->result.var);

	if (arg_num <= arg_count) {
		CREX_ASSERT(EX(func)->common.fn_flags & CREX_ACC_VARIADIC);
		CREX_ASSERT(EX(func)->common.num_args == arg_num - 1);
		crex_arg_info *arg_info = &EX(func)->common.arg_info[arg_num - 1];

		array_init_size(params, arg_count - arg_num + 1);
		crex_hash_real_init_packed(C_ARRVAL_P(params));
		CREX_HASH_FILL_PACKED(C_ARRVAL_P(params)) {
			zval *param = EX_VAR_NUM(EX(func)->op_array.last_var + EX(func)->op_array.T);
			if (UNEXPECTED(CREX_TYPE_IS_SET(arg_info->type))) {
				CREX_ADD_CALL_FLAG(execute_data, CREX_CALL_FREE_EXTRA_ARGS);
				do {
					if (UNEXPECTED(!crex_verify_variadic_arg_type(EX(func), arg_info, arg_num, param, CACHE_ADDR(opline->extended_value)))) {
						CREX_HASH_FILL_FINISH();
						HANDLE_EXCEPTION();
					}

					if (C_OPT_REFCOUNTED_P(param)) C_ADDREF_P(param);
					CREX_HASH_FILL_ADD(param);
					param++;
				} while (++arg_num <= arg_count);
			} else {
				do {
					if (C_OPT_REFCOUNTED_P(param)) C_ADDREF_P(param);
					CREX_HASH_FILL_ADD(param);
					param++;
				} while (++arg_num <= arg_count);
			}
		} CREX_HASH_FILL_END();
	} else {
		ZVAL_EMPTY_ARRAY(params);
	}

	if (EX_CALL_INFO() & CREX_CALL_HAS_EXTRA_NAMED_PARAMS) {
		crex_string *name;
		zval *param;
		crex_arg_info *arg_info = &EX(func)->common.arg_info[EX(func)->common.num_args];
		if (CREX_TYPE_IS_SET(arg_info->type)) {
			SEPARATE_ARRAY(params);
			CREX_HASH_MAP_FOREACH_STR_KEY_VAL(EX(extra_named_params), name, param) {
				if (UNEXPECTED(!crex_verify_variadic_arg_type(EX(func), arg_info, arg_num, param, CACHE_ADDR(opline->extended_value)))) {
					HANDLE_EXCEPTION();
				}
				C_TRY_ADDREF_P(param);
				crex_hash_add_new(C_ARRVAL_P(params), name, param);
			} CREX_HASH_FOREACH_END();
		} else if (crex_hash_num_elements(C_ARRVAL_P(params)) == 0) {
			GC_ADDREF(EX(extra_named_params));
			ZVAL_ARR(params, EX(extra_named_params));
		} else {
			SEPARATE_ARRAY(params);
			CREX_HASH_MAP_FOREACH_STR_KEY_VAL(EX(extra_named_params), name, param) {
				C_TRY_ADDREF_P(param);
				crex_hash_add_new(C_ARRVAL_P(params), name, param);
			} CREX_HASH_FOREACH_END();
		}
	}

	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_COLD_CONST_HANDLER(52, CREX_BOOL, CONST|TMPVAR|CV, ANY)
{
	USE_OPLINE
	zval *val;

	val = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (C_TYPE_INFO_P(val) == IS_TRUE) {
		ZVAL_TRUE(EX_VAR(opline->result.var));
	} else if (EXPECTED(C_TYPE_INFO_P(val) <= IS_TRUE)) {
		/* The result and op1 can be the same cv zval */
		const uint32_t orig_val_type = C_TYPE_INFO_P(val);
		ZVAL_FALSE(EX_VAR(opline->result.var));
		if (OP1_TYPE == IS_CV && UNEXPECTED(orig_val_type == IS_UNDEF)) {
			SAVE_OPLINE();
			ZVAL_UNDEFINED_OP1();
			CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
		}
	} else {
		SAVE_OPLINE();
		ZVAL_BOOL(EX_VAR(opline->result.var), i_crex_is_true(val));
		FREE_OP1();
		CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HELPER(crex_case_helper, ANY, ANY, zval *op_1, zval *op_2)
{
	int ret;
	USE_OPLINE

	SAVE_OPLINE();
	if (UNEXPECTED(C_TYPE_INFO_P(op_1) == IS_UNDEF)) {
		op_1 = ZVAL_UNDEFINED_OP1();
	}
	if (UNEXPECTED(C_TYPE_INFO_P(op_2) == IS_UNDEF)) {
		op_2 = ZVAL_UNDEFINED_OP2();
	}
	ret = crex_compare(op_1, op_2);
	if (OP2_TYPE & (IS_TMP_VAR|IS_VAR)) {
		zval_ptr_dtor_nogc(op_2);
	}
	CREX_VM_SMART_BRANCH(ret == 0, 1);
}

CREX_VM_HANDLER(48, CREX_CASE, TMPVAR, CONST|TMPVAR|CV)
{
	USE_OPLINE
	zval *op1, *op2;
	double d1, d2;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (EXPECTED(C_TYPE_P(op1) == IS_LONG)) {
		if (EXPECTED(C_TYPE_P(op2) == IS_LONG)) {
			if (EXPECTED(C_LVAL_P(op1) == C_LVAL_P(op2))) {
CREX_VM_C_LABEL(case_true):
				CREX_VM_SMART_BRANCH_TRUE();
			} else {
CREX_VM_C_LABEL(case_false):
				CREX_VM_SMART_BRANCH_FALSE();
			}
		} else if (EXPECTED(C_TYPE_P(op2) == IS_DOUBLE)) {
			d1 = (double)C_LVAL_P(op1);
			d2 = C_DVAL_P(op2);
			CREX_VM_C_GOTO(case_double);
		}
	} else if (EXPECTED(C_TYPE_P(op1) == IS_DOUBLE)) {
		if (EXPECTED(C_TYPE_P(op2) == IS_DOUBLE)) {
			d1 = C_DVAL_P(op1);
			d2 = C_DVAL_P(op2);
CREX_VM_C_LABEL(case_double):
			if (d1 == d2) {
				CREX_VM_C_GOTO(case_true);
			} else {
				CREX_VM_C_GOTO(case_false);
			}
		} else if (EXPECTED(C_TYPE_P(op2) == IS_LONG)) {
			d1 = C_DVAL_P(op1);
			d2 = (double)C_LVAL_P(op2);
			CREX_VM_C_GOTO(case_double);
		}
	} else if (EXPECTED(C_TYPE_P(op1) == IS_STRING)) {
		if (EXPECTED(C_TYPE_P(op2) == IS_STRING)) {
			bool result = crex_fast_equal_strings(C_STR_P(op1), C_STR_P(op2));
			FREE_OP2();
			if (result) {
				CREX_VM_C_GOTO(case_true);
			} else {
				CREX_VM_C_GOTO(case_false);
			}
		}
	}
	CREX_VM_DISPATCH_TO_HELPER(crex_case_helper, op_1, op1, op_2, op2);
}

CREX_VM_HANDLER(68, CREX_NEW, UNUSED|CLASS_FETCH|CONST|VAR, UNUSED|CACHE_SLOT, NUM)
{
	USE_OPLINE
	zval *result;
	crex_function *constructor;
	crex_class_entry *ce;
	crex_execute_data *call;

	SAVE_OPLINE();
	if (OP1_TYPE == IS_CONST) {
		ce = CACHED_PTR(opline->op2.num);
		if (UNEXPECTED(ce == NULL)) {
			ce = crex_fetch_class_by_name(C_STR_P(RT_CONSTANT(opline, opline->op1)), C_STR_P(RT_CONSTANT(opline, opline->op1) + 1), CREX_FETCH_CLASS_DEFAULT | CREX_FETCH_CLASS_EXCEPTION);
			if (UNEXPECTED(ce == NULL)) {
				ZVAL_UNDEF(EX_VAR(opline->result.var));
				HANDLE_EXCEPTION();
			}
			CACHE_PTR(opline->op2.num, ce);
		}
	} else if (OP1_TYPE == IS_UNUSED) {
		ce = crex_fetch_class(NULL, opline->op1.num);
		if (UNEXPECTED(ce == NULL)) {
			ZVAL_UNDEF(EX_VAR(opline->result.var));
			HANDLE_EXCEPTION();
		}
	} else {
		ce = C_CE_P(EX_VAR(opline->op1.var));
	}

	result = EX_VAR(opline->result.var);
	if (UNEXPECTED(object_init_ex(result, ce) != SUCCESS)) {
		ZVAL_UNDEF(result);
		HANDLE_EXCEPTION();
	}

	constructor = C_OBJ_HT_P(result)->get_constructor(C_OBJ_P(result));
	if (constructor == NULL) {
		if (UNEXPECTED(EG(exception))) {
			HANDLE_EXCEPTION();
		}

		/* If there are no arguments, skip over the DO_FCALL opcode. We check if the next
		 * opcode is DO_FCALL in case EXT instructions are used. */
		if (EXPECTED(opline->extended_value == 0 && (opline+1)->opcode == CREX_DO_FCALL)) {
			CREX_VM_NEXT_OPCODE_EX(1, 2);
		}

		/* Perform a dummy function call */
		call = crex_vm_stack_push_call_frame(
			CREX_CALL_FUNCTION, (crex_function *) &crex_pass_function,
			opline->extended_value, NULL);
	} else {
		if (EXPECTED(constructor->type == CREX_USER_FUNCTION) && UNEXPECTED(!RUN_TIME_CACHE(&constructor->op_array))) {
			init_func_run_time_cache(&constructor->op_array);
		}
		/* We are not handling overloaded classes right now */
		call = crex_vm_stack_push_call_frame(
			CREX_CALL_FUNCTION | CREX_CALL_RELEASE_THIS | CREX_CALL_HAS_THIS,
			constructor,
			opline->extended_value,
			C_OBJ_P(result));
		C_ADDREF_P(result);
	}

	call->prev_execute_data = EX(call);
	EX(call) = call;
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_COLD_CONST_HANDLER(110, CREX_CLONE, CONST|TMPVAR|UNUSED|THIS|CV, ANY)
{
	USE_OPLINE
	zval *obj;
	crex_object *zobj;
	crex_class_entry *ce, *scope;
	crex_function *clone;
	crex_object_clone_obj_t clone_call;

	SAVE_OPLINE();
	obj = GET_OP1_OBJ_ZVAL_PTR_UNDEF(BP_VAR_R);

	do {
		if (OP1_TYPE == IS_CONST ||
		    (OP1_TYPE != IS_UNUSED && UNEXPECTED(C_TYPE_P(obj) != IS_OBJECT))) {
			if ((OP1_TYPE & (IS_VAR|IS_CV)) && C_ISREF_P(obj)) {
				obj = C_REFVAL_P(obj);
				if (EXPECTED(C_TYPE_P(obj) == IS_OBJECT)) {
					break;
				}
			}
			ZVAL_UNDEF(EX_VAR(opline->result.var));
			if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(obj) == IS_UNDEF)) {
				ZVAL_UNDEFINED_OP1();
				if (UNEXPECTED(EG(exception) != NULL)) {
					HANDLE_EXCEPTION();
				}
			}
			crex_throw_error(NULL, "__clone method called on non-object");
			FREE_OP1();
			HANDLE_EXCEPTION();
		}
	} while (0);

	zobj = C_OBJ_P(obj);
	ce = zobj->ce;
	clone = ce->clone;
	clone_call = zobj->handlers->clone_obj;
	if (UNEXPECTED(clone_call == NULL)) {
		crex_throw_error(NULL, "Trying to clone an uncloneable object of class %s", ZSTR_VAL(ce->name));
		FREE_OP1();
		ZVAL_UNDEF(EX_VAR(opline->result.var));
		HANDLE_EXCEPTION();
	}

	if (clone && !(clone->common.fn_flags & CREX_ACC_PUBLIC)) {
		scope = EX(func)->op_array.scope;
		if (clone->common.scope != scope) {
			if (UNEXPECTED(clone->common.fn_flags & CREX_ACC_PRIVATE)
			 || UNEXPECTED(!crex_check_protected(crex_get_function_root_class(clone), scope))) {
				crex_wrong_clone_call(clone, scope);
				FREE_OP1();
				ZVAL_UNDEF(EX_VAR(opline->result.var));
				HANDLE_EXCEPTION();
			}
		}
	}

	ZVAL_OBJ(EX_VAR(opline->result.var), clone_call(zobj));

	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HOT_HANDLER(99, CREX_FETCH_CONSTANT, UNUSED|CONST_FETCH, CONST, CACHE_SLOT)
{
	USE_OPLINE
	crex_constant *c;

	c = CACHED_PTR(opline->extended_value);
	if (EXPECTED(c != NULL) && EXPECTED(!IS_SPECIAL_CACHE_VAL(c))) {
		ZVAL_COPY_OR_DUP(EX_VAR(opline->result.var), &c->value);
		CREX_VM_NEXT_OPCODE();
	}

	SAVE_OPLINE();
	crex_quick_get_constant(RT_CONSTANT(opline, opline->op2) + 1, opline->op1.num OPLINE_CC EXECUTE_DATA_CC);
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(181, CREX_FETCH_CLASS_CONSTANT, VAR|CONST|UNUSED|CLASS_FETCH, CONST|TMPVARCV, CACHE_SLOT)
{
	crex_class_entry *ce, *scope;
	crex_class_constant *c;
	zval *value, *zv, *constant_zv;
	crex_string *constant_name;
	USE_OPLINE

	SAVE_OPLINE();

	do {
		if (OP1_TYPE == IS_CONST && OP2_TYPE == IS_CONST) {
			if (EXPECTED(CACHED_PTR(opline->extended_value + sizeof(void*)))) {
				value = CACHED_PTR(opline->extended_value + sizeof(void*));
				break;
			}
		}
		if (OP1_TYPE == IS_CONST) {
			if (EXPECTED(CACHED_PTR(opline->extended_value))) {
				ce = CACHED_PTR(opline->extended_value);
			} else {
				ce = crex_fetch_class_by_name(C_STR_P(RT_CONSTANT(opline, opline->op1)), C_STR_P(RT_CONSTANT(opline, opline->op1) + 1), CREX_FETCH_CLASS_DEFAULT | CREX_FETCH_CLASS_EXCEPTION);
				if (UNEXPECTED(ce == NULL)) {
					ZVAL_UNDEF(EX_VAR(opline->result.var));
					FREE_OP2();
					HANDLE_EXCEPTION();
				}
				CACHE_PTR(opline->extended_value, ce);
			}
		} else if (OP1_TYPE == IS_UNUSED) {
			ce = crex_fetch_class(NULL, opline->op1.num);
			if (UNEXPECTED(ce == NULL)) {
				ZVAL_UNDEF(EX_VAR(opline->result.var));
				FREE_OP2();
				HANDLE_EXCEPTION();
			}
		} else {
			ce = C_CE_P(EX_VAR(opline->op1.var));
		}
		if (OP1_TYPE != IS_CONST
			&& OP2_TYPE == IS_CONST
			&& EXPECTED(CACHED_PTR(opline->extended_value) == ce)) {
			value = CACHED_PTR(opline->extended_value + sizeof(void*));
			break;
		}

		constant_zv = GET_OP2_ZVAL_PTR_DEREF(BP_VAR_R);
		if (UNEXPECTED(C_TYPE_P(constant_zv) != IS_STRING)) {
			crex_invalid_class_constant_type_error(C_TYPE_P(constant_zv));
			ZVAL_UNDEF(EX_VAR(opline->result.var));
			FREE_OP2();
			HANDLE_EXCEPTION();
		}
		constant_name = C_STR_P(constant_zv);
		/* Magic 'class' for constant OP2 is caught at compile-time */
		if (OP2_TYPE != IS_CONST && UNEXPECTED(crex_string_equals_literal_ci(constant_name, "class"))) {
			ZVAL_STR_COPY(EX_VAR(opline->result.var), ce->name);
			FREE_OP2();
			CREX_VM_NEXT_OPCODE();
		}
		zv = OP2_TYPE == IS_CONST
			? crex_hash_find_known_hash(CE_CONSTANTS_TABLE(ce), constant_name)
			: crex_hash_find(CE_CONSTANTS_TABLE(ce), constant_name);

		if (EXPECTED(zv != NULL)) {
			c = C_PTR_P(zv);
			scope = EX(func)->op_array.scope;
			if (!crex_verify_const_access(c, scope)) {
				crex_throw_error(NULL, "Cannot access %s constant %s::%s", crex_visibility_string(CREX_CLASS_CONST_FLAGS(c)), ZSTR_VAL(ce->name), ZSTR_VAL(constant_name));
				ZVAL_UNDEF(EX_VAR(opline->result.var));
				FREE_OP2();
				HANDLE_EXCEPTION();
			}

			if (ce->ce_flags & CREX_ACC_TRAIT) {
				crex_throw_error(NULL, "Cannot access trait constant %s::%s directly", ZSTR_VAL(ce->name), ZSTR_VAL(constant_name));
				ZVAL_UNDEF(EX_VAR(opline->result.var));
				FREE_OP2();
				HANDLE_EXCEPTION();
			}

			bool is_constant_deprecated = CREX_CLASS_CONST_FLAGS(c) & CREX_ACC_DEPRECATED;
			if (UNEXPECTED(is_constant_deprecated)) {
				crex_error(E_DEPRECATED, "Constant %s::%s is deprecated", ZSTR_VAL(ce->name), ZSTR_VAL(constant_name));

				if (EG(exception)) {
					ZVAL_UNDEF(EX_VAR(opline->result.var));
					FREE_OP2();
					HANDLE_EXCEPTION();
				}
			}

			value = &c->value;
			// Enums require loading of all class constants to build the backed enum table
			if (ce->ce_flags & CREX_ACC_ENUM && ce->enum_backing_type != IS_UNDEF && ce->type == CREX_USER_CLASS && !(ce->ce_flags & CREX_ACC_CONSTANTS_UPDATED)) {
				if (UNEXPECTED(crex_update_class_constants(ce) == FAILURE)) {
					ZVAL_UNDEF(EX_VAR(opline->result.var));
					FREE_OP2();
					HANDLE_EXCEPTION();
				}
			}
			if (C_TYPE_P(value) == IS_CONSTANT_AST) {
				if (UNEXPECTED(crex_update_class_constant(c, constant_name, c->ce) != SUCCESS)) {
					ZVAL_UNDEF(EX_VAR(opline->result.var));
					FREE_OP2();
					HANDLE_EXCEPTION();
				}
			}
			if (OP2_TYPE == IS_CONST && !is_constant_deprecated) {
				CACHE_POLYMORPHIC_PTR(opline->extended_value, ce, value);
			}
		} else {
			crex_throw_error(NULL, "Undefined constant %s::%s",
				ZSTR_VAL(ce->name), ZSTR_VAL(constant_name));
			ZVAL_UNDEF(EX_VAR(opline->result.var));
			FREE_OP2();
			HANDLE_EXCEPTION();
		}
	} while (0);

	ZVAL_COPY_OR_DUP(EX_VAR(opline->result.var), value);

	FREE_OP2();
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(72, CREX_ADD_ARRAY_ELEMENT, CONST|TMP|VAR|CV, CONST|TMPVAR|UNUSED|NEXT|CV, REF)
{
	USE_OPLINE
	zval *expr_ptr, new_expr;

	SAVE_OPLINE();
	if ((OP1_TYPE == IS_VAR || OP1_TYPE == IS_CV) &&
	    UNEXPECTED(opline->extended_value & CREX_ARRAY_ELEMENT_REF)) {
		expr_ptr = GET_OP1_ZVAL_PTR_PTR(BP_VAR_W);
		if (C_ISREF_P(expr_ptr)) {
			C_ADDREF_P(expr_ptr);
		} else {
			ZVAL_MAKE_REF_EX(expr_ptr, 2);
		}
		FREE_OP1();
	} else {
		expr_ptr = GET_OP1_ZVAL_PTR(BP_VAR_R);
		if (OP1_TYPE == IS_TMP_VAR) {
			/* pass */
		} else if (OP1_TYPE == IS_CONST) {
			C_TRY_ADDREF_P(expr_ptr);
		} else if (OP1_TYPE == IS_CV) {
			ZVAL_DEREF(expr_ptr);
			C_TRY_ADDREF_P(expr_ptr);
		} else /* if (OP1_TYPE == IS_VAR) */ {
			if (UNEXPECTED(C_ISREF_P(expr_ptr))) {
				crex_refcounted *ref = C_COUNTED_P(expr_ptr);

				expr_ptr = C_REFVAL_P(expr_ptr);
				if (UNEXPECTED(GC_DELREF(ref) == 0)) {
					ZVAL_COPY_VALUE(&new_expr, expr_ptr);
					expr_ptr = &new_expr;
					efree_size(ref, sizeof(crex_reference));
				} else if (C_OPT_REFCOUNTED_P(expr_ptr)) {
					C_ADDREF_P(expr_ptr);
				}
			}
		}
	}

	if (OP2_TYPE != IS_UNUSED) {
		zval *offset = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
		crex_string *str;
		crex_ulong hval;

CREX_VM_C_LABEL(add_again):
		if (EXPECTED(C_TYPE_P(offset) == IS_STRING)) {
			str = C_STR_P(offset);
			if (OP2_TYPE != IS_CONST) {
				if (CREX_HANDLE_NUMERIC(str, hval)) {
					CREX_VM_C_GOTO(num_index);
				}
			}
CREX_VM_C_LABEL(str_index):
			crex_hash_update(C_ARRVAL_P(EX_VAR(opline->result.var)), str, expr_ptr);
		} else if (EXPECTED(C_TYPE_P(offset) == IS_LONG)) {
			hval = C_LVAL_P(offset);
CREX_VM_C_LABEL(num_index):
			crex_hash_index_update(C_ARRVAL_P(EX_VAR(opline->result.var)), hval, expr_ptr);
		} else if ((OP2_TYPE & (IS_VAR|IS_CV)) && EXPECTED(C_TYPE_P(offset) == IS_REFERENCE)) {
			offset = C_REFVAL_P(offset);
			CREX_VM_C_GOTO(add_again);
		} else if (C_TYPE_P(offset) == IS_NULL) {
			str = ZSTR_EMPTY_ALLOC();
			CREX_VM_C_GOTO(str_index);
		} else if (C_TYPE_P(offset) == IS_DOUBLE) {
			hval = crex_dval_to_lval_safe(C_DVAL_P(offset));
			CREX_VM_C_GOTO(num_index);
		} else if (C_TYPE_P(offset) == IS_FALSE) {
			hval = 0;
			CREX_VM_C_GOTO(num_index);
		} else if (C_TYPE_P(offset) == IS_TRUE) {
			hval = 1;
			CREX_VM_C_GOTO(num_index);
		} else if (C_TYPE_P(offset) == IS_RESOURCE) {
			crex_use_resource_as_offset(offset);
			hval = C_RES_HANDLE_P(offset);
			CREX_VM_C_GOTO(num_index);
		} else if (OP2_TYPE == IS_CV && C_TYPE_P(offset) == IS_UNDEF) {
			ZVAL_UNDEFINED_OP2();
			str = ZSTR_EMPTY_ALLOC();
			CREX_VM_C_GOTO(str_index);
		} else {
			crex_illegal_array_offset_access(offset);
			zval_ptr_dtor_nogc(expr_ptr);
		}
		FREE_OP2();
	} else {
		if (!crex_hash_next_index_insert(C_ARRVAL_P(EX_VAR(opline->result.var)), expr_ptr)) {
			crex_cannot_add_element();
			zval_ptr_dtor_nogc(expr_ptr);
		}
	}
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(147, CREX_ADD_ARRAY_UNPACK, ANY, ANY)
{
	USE_OPLINE
	zval *op1;
	HashTable *result_ht;

	SAVE_OPLINE();
	op1 = GET_OP1_ZVAL_PTR(BP_VAR_R);
	result_ht = C_ARRVAL_P(EX_VAR(opline->result.var));

CREX_VM_C_LABEL(add_unpack_again):
	if (EXPECTED(C_TYPE_P(op1) == IS_ARRAY)) {
		HashTable *ht = C_ARRVAL_P(op1);
		zval *val;

		if (HT_IS_PACKED(ht) && (crex_hash_num_elements(result_ht) == 0 || HT_IS_PACKED(result_ht))) {
			crex_hash_extend(result_ht, result_ht->nNumUsed + crex_hash_num_elements(ht), 1);
			CREX_HASH_FILL_PACKED(result_ht) {
				CREX_HASH_PACKED_FOREACH_VAL(ht, val) {
					if (UNEXPECTED(C_ISREF_P(val)) &&
						UNEXPECTED(C_REFCOUNT_P(val) == 1)) {
						val = C_REFVAL_P(val);
					}
					C_TRY_ADDREF_P(val);
					CREX_HASH_FILL_ADD(val);
				} CREX_HASH_FOREACH_END();
			} CREX_HASH_FILL_END();
		} else {
			crex_string *key;

			CREX_HASH_FOREACH_STR_KEY_VAL(ht, key, val) {
				if (UNEXPECTED(C_ISREF_P(val)) &&
					UNEXPECTED(C_REFCOUNT_P(val) == 1)) {
					val = C_REFVAL_P(val);
				}
				C_TRY_ADDREF_P(val);
				if (key) {
					crex_hash_update(result_ht, key, val);
				} else {
					if (!crex_hash_next_index_insert(result_ht, val)) {
						crex_cannot_add_element();
						zval_ptr_dtor_nogc(val);
						break;
					}
				}
			} CREX_HASH_FOREACH_END();
		}
	} else if (EXPECTED(C_TYPE_P(op1) == IS_OBJECT)) {
		crex_class_entry *ce = C_OBJCE_P(op1);
		crex_object_iterator *iter;

		if (!ce || !ce->get_iterator) {
			crex_type_error("Only arrays and Traversables can be unpacked");
		} else {
			iter = ce->get_iterator(ce, op1, 0);
			if (UNEXPECTED(!iter)) {
				FREE_OP1();
				if (!EG(exception)) {
					crex_throw_exception_ex(
						NULL, 0, "Object of type %s did not create an Iterator", ZSTR_VAL(ce->name)
					);
				}
				HANDLE_EXCEPTION();
			}

			const crex_object_iterator_funcs *funcs = iter->funcs;
			if (funcs->rewind) {
				funcs->rewind(iter);
			}

			for (; funcs->valid(iter) == SUCCESS; ) {
				zval *val;

				if (UNEXPECTED(EG(exception) != NULL)) {
					break;
				}

				val = funcs->get_current_data(iter);
				if (UNEXPECTED(EG(exception) != NULL)) {
					break;
				}

				zval key;
				if (funcs->get_current_key) {
					funcs->get_current_key(iter, &key);
					if (UNEXPECTED(EG(exception) != NULL)) {
						break;
					}

					if (UNEXPECTED(C_TYPE(key) != IS_LONG && C_TYPE(key) != IS_STRING)) {
						crex_throw_error(NULL,
							"Keys must be of type int|string during array unpacking");
						zval_ptr_dtor(&key);
						break;
					}
				} else {
					ZVAL_UNDEF(&key);
				}

				ZVAL_DEREF(val);
				C_TRY_ADDREF_P(val);

				crex_ulong num_key;
				if (C_TYPE(key) == IS_STRING && !CREX_HANDLE_NUMERIC(C_STR(key), num_key)) {
					crex_hash_update(result_ht, C_STR(key), val);
					zval_ptr_dtor_str(&key);
				} else {
					zval_ptr_dtor(&key);
					if (!crex_hash_next_index_insert(result_ht, val)) {
						crex_cannot_add_element();
						zval_ptr_dtor_nogc(val);
						break;
					}
				}

				funcs->move_forward(iter);
				if (UNEXPECTED(EG(exception))) {
					break;
				}
			}

			crex_iterator_dtor(iter);
		}
	} else if (EXPECTED(C_ISREF_P(op1))) {
		op1 = C_REFVAL_P(op1);
		CREX_VM_C_GOTO(add_unpack_again);
	} else {
		crex_throw_error(NULL, "Only arrays and Traversables can be unpacked");
	}

	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(71, CREX_INIT_ARRAY, CONST|TMP|VAR|CV|UNUSED, CONST|TMPVAR|UNUSED|NEXT|CV, ARRAY_INIT|REF)
{
	zval *array;
	uint32_t size;
	USE_OPLINE

	array = EX_VAR(opline->result.var);
	if (OP1_TYPE != IS_UNUSED) {
		size = opline->extended_value >> CREX_ARRAY_SIZE_SHIFT;
		ZVAL_ARR(array, crex_new_array(size));
		/* Explicitly initialize array as not-packed if flag is set */
		if (opline->extended_value & CREX_ARRAY_NOT_PACKED) {
			crex_hash_real_init_mixed(C_ARRVAL_P(array));
		}
		CREX_VM_DISPATCH_TO_HANDLER(CREX_ADD_ARRAY_ELEMENT);
	} else {
		ZVAL_ARR(array, crex_new_array(0));
		CREX_VM_NEXT_OPCODE();
	}
}

CREX_VM_COLD_CONST_HANDLER(51, CREX_CAST, CONST|TMP|VAR|CV, ANY, TYPE)
{
	USE_OPLINE
	zval *expr;
	zval *result = EX_VAR(opline->result.var);
	HashTable *ht;

	SAVE_OPLINE();
	expr = GET_OP1_ZVAL_PTR(BP_VAR_R);

	switch (opline->extended_value) {
		case IS_LONG:
			ZVAL_LONG(result, zval_get_long(expr));
			break;
		case IS_DOUBLE:
			ZVAL_DOUBLE(result, zval_get_double(expr));
			break;
		case IS_STRING:
			ZVAL_STR(result, zval_get_string(expr));
			break;
		default:
			CREX_ASSERT(opline->extended_value != _IS_BOOL && "Must use CREX_BOOL instead");
			if (OP1_TYPE & (IS_VAR|IS_CV)) {
				ZVAL_DEREF(expr);
			}
			/* If value is already of correct type, return it directly */
			if (C_TYPE_P(expr) == opline->extended_value) {
				ZVAL_COPY_VALUE(result, expr);
				if (OP1_TYPE == IS_CONST) {
					if (UNEXPECTED(C_OPT_REFCOUNTED_P(result))) C_ADDREF_P(result);
				} else if (OP1_TYPE != IS_TMP_VAR) {
					if (C_OPT_REFCOUNTED_P(result)) C_ADDREF_P(result);
				}

				FREE_OP1_IF_VAR();
				CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
			}

			if (opline->extended_value == IS_ARRAY) {
				if (OP1_TYPE == IS_CONST || C_TYPE_P(expr) != IS_OBJECT || C_OBJCE_P(expr) == crex_ce_closure) {
					if (C_TYPE_P(expr) != IS_NULL) {
						ZVAL_ARR(result, crex_new_array(1));
						expr = crex_hash_index_add_new(C_ARRVAL_P(result), 0, expr);
						if (OP1_TYPE == IS_CONST) {
							if (UNEXPECTED(C_OPT_REFCOUNTED_P(expr))) C_ADDREF_P(expr);
						} else {
							if (C_OPT_REFCOUNTED_P(expr)) C_ADDREF_P(expr);
						}
					} else {
						ZVAL_EMPTY_ARRAY(result);
					}
				} else if (C_OBJ_P(expr)->properties == NULL
				 && C_OBJ_HT_P(expr)->get_properties_for == NULL
				 && C_OBJ_HT_P(expr)->get_properties == crex_std_get_properties) {
					/* Optimized version without rebuilding properties HashTable */
					ZVAL_ARR(result, crex_std_build_object_properties_array(C_OBJ_P(expr)));
				} else {
					HashTable *obj_ht = crex_get_properties_for(expr, CREX_PROP_PURPOSE_ARRAY_CAST);
					if (obj_ht) {
						/* fast copy */
						ZVAL_ARR(result, crex_proptable_to_symtable(obj_ht,
							(C_OBJCE_P(expr)->default_properties_count ||
							 C_OBJ_P(expr)->handlers != &std_object_handlers ||
							 GC_IS_RECURSIVE(obj_ht))));
						crex_release_properties(obj_ht);
					} else {
						ZVAL_EMPTY_ARRAY(result);
					}
				}
			} else {
				CREX_ASSERT(opline->extended_value == IS_OBJECT);
				ZVAL_OBJ(result, crex_objects_new(crex_standard_class_def));
				if (C_TYPE_P(expr) == IS_ARRAY) {
					ht = crex_symtable_to_proptable(C_ARR_P(expr));
					if (GC_FLAGS(ht) & IS_ARRAY_IMMUTABLE) {
						/* TODO: try not to duplicate immutable arrays as well ??? */
						ht = crex_array_dup(ht);
					}
					C_OBJ_P(result)->properties = ht;
				} else if (C_TYPE_P(expr) != IS_NULL) {
					C_OBJ_P(result)->properties = ht = crex_new_array(1);
					expr = crex_hash_add_new(ht, ZSTR_KNOWN(CREX_STR_SCALAR), expr);
					if (OP1_TYPE == IS_CONST) {
						if (UNEXPECTED(C_OPT_REFCOUNTED_P(expr))) C_ADDREF_P(expr);
					} else {
						if (C_OPT_REFCOUNTED_P(expr)) C_ADDREF_P(expr);
					}
				}
			}
	}

	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(73, CREX_INCLUDE_OR_EVAL, CONST|TMPVAR|CV, ANY, EVAL, SPEC(OBSERVER))
{
	USE_OPLINE
	crex_op_array *new_op_array;
	zval *inc_filename;

	SAVE_OPLINE();
	inc_filename = GET_OP1_ZVAL_PTR(BP_VAR_R);
	new_op_array = crex_include_or_eval(inc_filename, opline->extended_value);
	if (UNEXPECTED(EG(exception) != NULL)) {
		FREE_OP1();
		if (new_op_array != CREX_FAKE_OP_ARRAY && new_op_array != NULL) {
			destroy_op_array(new_op_array);
			efree_size(new_op_array, sizeof(crex_op_array));
		}
		UNDEF_RESULT();
		HANDLE_EXCEPTION();
	} else if (new_op_array == CREX_FAKE_OP_ARRAY) {
		if (RETURN_VALUE_USED(opline)) {
			ZVAL_TRUE(EX_VAR(opline->result.var));
		}
	} else if (UNEXPECTED(new_op_array == NULL)) {
		if (RETURN_VALUE_USED(opline)) {
			ZVAL_FALSE(EX_VAR(opline->result.var));
		}
	} else if (new_op_array->last == 1
			&& new_op_array->opcodes[0].opcode == CREX_RETURN
			&& new_op_array->opcodes[0].op1_type == IS_CONST
			&& EXPECTED(crex_execute_ex == execute_ex)) {
		if (RETURN_VALUE_USED(opline)) {
			const crex_op *op = new_op_array->opcodes;

			ZVAL_COPY(EX_VAR(opline->result.var), RT_CONSTANT(op, op->op1));
		}
		crex_destroy_static_vars(new_op_array);
		destroy_op_array(new_op_array);
		efree_size(new_op_array, sizeof(crex_op_array));
	} else {
		zval *return_value = NULL;
		crex_execute_data *call;
		if (RETURN_VALUE_USED(opline)) {
			return_value = EX_VAR(opline->result.var);
		}

		new_op_array->scope = EX(func)->op_array.scope;

		call = crex_vm_stack_push_call_frame(
			(C_TYPE_INFO(EX(This)) & CREX_CALL_HAS_THIS) | CREX_CALL_NESTED_CODE | CREX_CALL_HAS_SYMBOL_TABLE,
			(crex_function*)new_op_array, 0,
			C_PTR(EX(This)));

		if (EX_CALL_INFO() & CREX_CALL_HAS_SYMBOL_TABLE) {
			call->symbol_table = EX(symbol_table);
		} else {
			call->symbol_table = crex_rebuild_symbol_table();
		}

		call->prev_execute_data = execute_data;
		i_init_code_execute_data(call, new_op_array, return_value);
		CREX_OBSERVER_FCALL_BEGIN(call);
		if (EXPECTED(crex_execute_ex == execute_ex)) {
			FREE_OP1();
			CREX_VM_ENTER();
		} else {
			CREX_ADD_CALL_FLAG(call, CREX_CALL_TOP);
			crex_execute_ex(call);
			crex_vm_stack_free_call_frame(call);
		}

		crex_destroy_static_vars(new_op_array);
		destroy_op_array(new_op_array);
		efree_size(new_op_array, sizeof(crex_op_array));
		if (UNEXPECTED(EG(exception) != NULL)) {
			crex_rethrow_exception(execute_data);
			FREE_OP1();
			UNDEF_RESULT();
			HANDLE_EXCEPTION();
		}
	}
	FREE_OP1();
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(153, CREX_UNSET_CV, CV, UNUSED)
{
	USE_OPLINE
	zval *var = EX_VAR(opline->op1.var);

	if (C_REFCOUNTED_P(var)) {
		crex_refcounted *garbage = C_COUNTED_P(var);

		ZVAL_UNDEF(var);
		SAVE_OPLINE();
		GC_DTOR(garbage);
		CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
	} else {
		ZVAL_UNDEF(var);
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(74, CREX_UNSET_VAR, CONST|TMPVAR|CV, UNUSED, VAR_FETCH)
{
	USE_OPLINE
	zval *varname;
	crex_string *name, *tmp_name;
	HashTable *target_symbol_table;

	SAVE_OPLINE();

	varname = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);

	if (OP1_TYPE == IS_CONST) {
		name = C_STR_P(varname);
	} else if (EXPECTED(C_TYPE_P(varname) == IS_STRING)) {
		name = C_STR_P(varname);
		tmp_name = NULL;
	} else {
		if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(varname) == IS_UNDEF)) {
			varname = ZVAL_UNDEFINED_OP1();
		}
		name = zval_try_get_tmp_string(varname, &tmp_name);
		if (UNEXPECTED(!name)) {
			FREE_OP1();
			HANDLE_EXCEPTION();
		}
	}

	target_symbol_table = crex_get_target_symbol_table(opline->extended_value EXECUTE_DATA_CC);
	crex_hash_del_ind(target_symbol_table, name);

	if (OP1_TYPE != IS_CONST) {
		crex_tmp_string_release(tmp_name);
	}
	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

/* No specialization for op_types (CONST|TMPVAR|CV, UNUSED|CLASS_FETCH|CONST|VAR) */
CREX_VM_COLD_HANDLER(179, CREX_UNSET_STATIC_PROP, ANY, ANY, CACHE_SLOT)
{
	USE_OPLINE
	zval *varname;
	crex_string *name, *tmp_name = NULL;
	crex_class_entry *ce;

	SAVE_OPLINE();

	if (OP2_TYPE == IS_CONST) {
		ce = CACHED_PTR(opline->extended_value);
		if (UNEXPECTED(ce == NULL)) {
			ce = crex_fetch_class_by_name(C_STR_P(RT_CONSTANT(opline, opline->op2)), C_STR_P(RT_CONSTANT(opline, opline->op2) + 1), CREX_FETCH_CLASS_DEFAULT | CREX_FETCH_CLASS_EXCEPTION);
			if (UNEXPECTED(ce == NULL)) {
				FREE_OP1();
				HANDLE_EXCEPTION();
			}
			/*CACHE_PTR(opline->extended_value, ce);*/
		}
	} else if (OP2_TYPE == IS_UNUSED) {
		ce = crex_fetch_class(NULL, opline->op2.num);
		if (UNEXPECTED(ce == NULL)) {
			FREE_OP1();
			HANDLE_EXCEPTION();
		}
	} else {
		ce = C_CE_P(EX_VAR(opline->op2.var));
	}

	varname = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (OP1_TYPE == IS_CONST) {
		name = C_STR_P(varname);
	} else if (EXPECTED(C_TYPE_P(varname) == IS_STRING)) {
		name = C_STR_P(varname);
	} else {
		if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(varname) == IS_UNDEF)) {
			varname = ZVAL_UNDEFINED_OP1();
		}
		name = zval_try_get_tmp_string(varname, &tmp_name);
		if (UNEXPECTED(!name)) {
			FREE_OP1();
			HANDLE_EXCEPTION();
		}
	}

	crex_std_unset_static_property(ce, name);

	crex_tmp_string_release(tmp_name);
	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(75, CREX_UNSET_DIM, VAR|CV, CONST|TMPVAR|CV)
{
	USE_OPLINE
	zval *container;
	zval *offset;
	crex_ulong hval;
	crex_string *key;

	SAVE_OPLINE();
	container = GET_OP1_OBJ_ZVAL_PTR_PTR_UNDEF(BP_VAR_UNSET);
	offset = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);

	do {
		if (EXPECTED(C_TYPE_P(container) == IS_ARRAY)) {
			HashTable *ht;

CREX_VM_C_LABEL(unset_dim_array):
			SEPARATE_ARRAY(container);
			ht = C_ARRVAL_P(container);
CREX_VM_C_LABEL(offset_again):
			if (EXPECTED(C_TYPE_P(offset) == IS_STRING)) {
				key = C_STR_P(offset);
				if (OP2_TYPE != IS_CONST) {
					if (CREX_HANDLE_NUMERIC(key, hval)) {
						CREX_VM_C_GOTO(num_index_dim);
					}
				}
CREX_VM_C_LABEL(str_index_dim):
				CREX_ASSERT(ht != &EG(symbol_table));
				crex_hash_del(ht, key);
			} else if (EXPECTED(C_TYPE_P(offset) == IS_LONG)) {
				hval = C_LVAL_P(offset);
CREX_VM_C_LABEL(num_index_dim):
				crex_hash_index_del(ht, hval);
			} else if ((OP2_TYPE & (IS_VAR|IS_CV)) && EXPECTED(C_TYPE_P(offset) == IS_REFERENCE)) {
				offset = C_REFVAL_P(offset);
				CREX_VM_C_GOTO(offset_again);
			} else if (C_TYPE_P(offset) == IS_DOUBLE) {
				hval = crex_dval_to_lval_safe(C_DVAL_P(offset));
				CREX_VM_C_GOTO(num_index_dim);
			} else if (C_TYPE_P(offset) == IS_NULL) {
				key = ZSTR_EMPTY_ALLOC();
				CREX_VM_C_GOTO(str_index_dim);
			} else if (C_TYPE_P(offset) == IS_FALSE) {
				hval = 0;
				CREX_VM_C_GOTO(num_index_dim);
			} else if (C_TYPE_P(offset) == IS_TRUE) {
				hval = 1;
				CREX_VM_C_GOTO(num_index_dim);
			} else if (C_TYPE_P(offset) == IS_RESOURCE) {
				crex_use_resource_as_offset(offset);
				hval = C_RES_HANDLE_P(offset);
				CREX_VM_C_GOTO(num_index_dim);
			} else if (OP2_TYPE == IS_CV && C_TYPE_P(offset) == IS_UNDEF) {
				ZVAL_UNDEFINED_OP2();
				key = ZSTR_EMPTY_ALLOC();
				CREX_VM_C_GOTO(str_index_dim);
			} else {
				crex_illegal_array_offset_unset(offset);
			}
			break;
		} else if (C_ISREF_P(container)) {
			container = C_REFVAL_P(container);
			if (EXPECTED(C_TYPE_P(container) == IS_ARRAY)) {
				CREX_VM_C_GOTO(unset_dim_array);
			}
		}
		if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(container) == IS_UNDEF)) {
			container = ZVAL_UNDEFINED_OP1();
		}
		if (OP2_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(offset) == IS_UNDEF)) {
			offset = ZVAL_UNDEFINED_OP2();
		}
		if (EXPECTED(C_TYPE_P(container) == IS_OBJECT)) {
			if (OP2_TYPE == IS_CONST && C_EXTRA_P(offset) == CREX_EXTRA_VALUE) {
				offset++;
			}
			C_OBJ_HT_P(container)->unset_dimension(C_OBJ_P(container), offset);
		} else if (UNEXPECTED(C_TYPE_P(container) == IS_STRING)) {
			crex_throw_error(NULL, "Cannot unset string offsets");
		} else if (UNEXPECTED(C_TYPE_P(container) > IS_FALSE)) {
			crex_throw_error(NULL, "Cannot unset offset in a non-array variable");
		} else if (UNEXPECTED(C_TYPE_P(container) == IS_FALSE)) {
			crex_false_to_array_deprecated();
		}
	} while (0);

	FREE_OP2();
	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(76, CREX_UNSET_OBJ, VAR|UNUSED|THIS|CV, CONST|TMPVAR|CV, CACHE_SLOT)
{
	USE_OPLINE
	zval *container;
	zval *offset;
	crex_string *name, *tmp_name;

	SAVE_OPLINE();
	container = GET_OP1_OBJ_ZVAL_PTR_PTR_UNDEF(BP_VAR_UNSET);
	offset = GET_OP2_ZVAL_PTR(BP_VAR_R);

	do {
		if (OP1_TYPE != IS_UNUSED && UNEXPECTED(C_TYPE_P(container) != IS_OBJECT)) {
			if (C_ISREF_P(container)) {
				container = C_REFVAL_P(container);
				if (C_TYPE_P(container) != IS_OBJECT) {
					if (OP1_TYPE == IS_CV
					 && UNEXPECTED(C_TYPE_P(container) == IS_UNDEF)) {
						ZVAL_UNDEFINED_OP1();
					}
					break;
				}
			} else {
				break;
			}
		}
		if (OP2_TYPE == IS_CONST) {
			name = C_STR_P(offset);
		} else {
			name = zval_try_get_tmp_string(offset, &tmp_name);
			if (UNEXPECTED(!name)) {
				break;
			}
		}
		C_OBJ_HT_P(container)->unset_property(C_OBJ_P(container), name, ((OP2_TYPE == IS_CONST) ? CACHE_ADDR(opline->extended_value) : NULL));
		if (OP2_TYPE != IS_CONST) {
			crex_tmp_string_release(tmp_name);
		}
	} while (0);

	FREE_OP2();
	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(77, CREX_FE_RESET_R, CONST|TMP|VAR|CV, JMP_ADDR)
{
	USE_OPLINE
	zval *array_ptr, *result;

	SAVE_OPLINE();

	array_ptr = GET_OP1_ZVAL_PTR_DEREF(BP_VAR_R);
	if (EXPECTED(C_TYPE_P(array_ptr) == IS_ARRAY)) {
		result = EX_VAR(opline->result.var);
		ZVAL_COPY_VALUE(result, array_ptr);
		if (OP1_TYPE != IS_TMP_VAR && C_OPT_REFCOUNTED_P(result)) {
			C_ADDREF_P(array_ptr);
		}
		C_FE_POS_P(result) = 0;

		FREE_OP1_IF_VAR();
		CREX_VM_NEXT_OPCODE();
	} else if (OP1_TYPE != IS_CONST && EXPECTED(C_TYPE_P(array_ptr) == IS_OBJECT)) {
		crex_object *zobj = C_OBJ_P(array_ptr);
		if (!zobj->ce->get_iterator) {
			HashTable *properties = zobj->properties;
			if (properties) {
				if (UNEXPECTED(GC_REFCOUNT(properties) > 1)) {
					if (EXPECTED(!(GC_FLAGS(properties) & IS_ARRAY_IMMUTABLE))) {
						GC_DELREF(properties);
					}
					properties = zobj->properties = crex_array_dup(properties);
				}
			} else {
				properties = zobj->handlers->get_properties(zobj);
			}

			result = EX_VAR(opline->result.var);
			ZVAL_COPY_VALUE(result, array_ptr);
			if (OP1_TYPE != IS_TMP_VAR) {
				C_ADDREF_P(array_ptr);
			}

			if (crex_hash_num_elements(properties) == 0) {
				C_FE_ITER_P(result) = (uint32_t) -1;
				FREE_OP1_IF_VAR();
				CREX_VM_JMP(OP_JMP_ADDR(opline, opline->op2));
			}

			C_FE_ITER_P(result) = crex_hash_iterator_add(properties, 0);
			FREE_OP1_IF_VAR();
			CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
		} else {
			bool is_empty = crex_fe_reset_iterator(array_ptr, 0 OPLINE_CC EXECUTE_DATA_CC);

			FREE_OP1();
			if (UNEXPECTED(EG(exception))) {
				HANDLE_EXCEPTION();
			} else if (is_empty) {
				CREX_VM_JMP_EX(OP_JMP_ADDR(opline, opline->op2), 0);
			} else {
				CREX_VM_NEXT_OPCODE();
			}
		}
	} else {
		crex_error(E_WARNING, "foreach() argument must be of type array|object, %s given", crex_zval_value_name(array_ptr));
		ZVAL_UNDEF(EX_VAR(opline->result.var));
		C_FE_ITER_P(EX_VAR(opline->result.var)) = (uint32_t)-1;
		FREE_OP1();
		CREX_VM_JMP(OP_JMP_ADDR(opline, opline->op2));
	}
}

CREX_VM_COLD_CONST_HANDLER(125, CREX_FE_RESET_RW, CONST|TMP|VAR|CV, JMP_ADDR)
{
	USE_OPLINE
	zval *array_ptr, *array_ref;

	SAVE_OPLINE();

	if (OP1_TYPE == IS_VAR || OP1_TYPE == IS_CV) {
		array_ref = array_ptr = GET_OP1_ZVAL_PTR_PTR(BP_VAR_R);
		if (C_ISREF_P(array_ref)) {
			array_ptr = C_REFVAL_P(array_ref);
		}
	} else {
		array_ref = array_ptr = GET_OP1_ZVAL_PTR(BP_VAR_R);
	}

	if (EXPECTED(C_TYPE_P(array_ptr) == IS_ARRAY)) {
		if (OP1_TYPE == IS_VAR || OP1_TYPE == IS_CV) {
			if (array_ptr == array_ref) {
				ZVAL_NEW_REF(array_ref, array_ref);
				array_ptr = C_REFVAL_P(array_ref);
			}
			C_ADDREF_P(array_ref);
			ZVAL_COPY_VALUE(EX_VAR(opline->result.var), array_ref);
		} else {
			array_ref = EX_VAR(opline->result.var);
			ZVAL_NEW_REF(array_ref, array_ptr);
			array_ptr = C_REFVAL_P(array_ref);
		}
		if (OP1_TYPE == IS_CONST) {
			ZVAL_ARR(array_ptr, crex_array_dup(C_ARRVAL_P(array_ptr)));
		} else {
			SEPARATE_ARRAY(array_ptr);
		}
		C_FE_ITER_P(EX_VAR(opline->result.var)) = crex_hash_iterator_add(C_ARRVAL_P(array_ptr), 0);

		FREE_OP1_IF_VAR();
		CREX_VM_NEXT_OPCODE();
	} else if (OP1_TYPE != IS_CONST && EXPECTED(C_TYPE_P(array_ptr) == IS_OBJECT)) {
		if (!C_OBJCE_P(array_ptr)->get_iterator) {
			HashTable *properties;
			if (OP1_TYPE == IS_VAR || OP1_TYPE == IS_CV) {
				if (array_ptr == array_ref) {
					ZVAL_NEW_REF(array_ref, array_ref);
					array_ptr = C_REFVAL_P(array_ref);
				}
				C_ADDREF_P(array_ref);
				ZVAL_COPY_VALUE(EX_VAR(opline->result.var), array_ref);
			} else {
				array_ptr = EX_VAR(opline->result.var);
				ZVAL_COPY_VALUE(array_ptr, array_ref);
			}
			if (C_OBJ_P(array_ptr)->properties
			 && UNEXPECTED(GC_REFCOUNT(C_OBJ_P(array_ptr)->properties) > 1)) {
				if (EXPECTED(!(GC_FLAGS(C_OBJ_P(array_ptr)->properties) & IS_ARRAY_IMMUTABLE))) {
					GC_DELREF(C_OBJ_P(array_ptr)->properties);
				}
				C_OBJ_P(array_ptr)->properties = crex_array_dup(C_OBJ_P(array_ptr)->properties);
			}

			properties = C_OBJPROP_P(array_ptr);
			if (crex_hash_num_elements(properties) == 0) {
				C_FE_ITER_P(EX_VAR(opline->result.var)) = (uint32_t) -1;
				FREE_OP1_IF_VAR();
				CREX_VM_JMP(OP_JMP_ADDR(opline, opline->op2));
			}

			C_FE_ITER_P(EX_VAR(opline->result.var)) = crex_hash_iterator_add(properties, 0);
			FREE_OP1_IF_VAR();
			CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
		} else {
			bool is_empty = crex_fe_reset_iterator(array_ptr, 1 OPLINE_CC EXECUTE_DATA_CC);
			FREE_OP1();
			if (UNEXPECTED(EG(exception))) {
				HANDLE_EXCEPTION();
			} else if (is_empty) {
				CREX_VM_JMP_EX(OP_JMP_ADDR(opline, opline->op2), 0);
			} else {
				CREX_VM_NEXT_OPCODE();
			}
		}
	} else {
		crex_error(E_WARNING, "foreach() argument must be of type array|object, %s given", crex_zval_value_name(array_ptr));
		ZVAL_UNDEF(EX_VAR(opline->result.var));
		C_FE_ITER_P(EX_VAR(opline->result.var)) = (uint32_t)-1;
		FREE_OP1();
		CREX_VM_JMP(OP_JMP_ADDR(opline, opline->op2));
	}
}

CREX_VM_HELPER(crex_fe_fetch_object_helper, ANY, ANY)
{
	USE_OPLINE
	zval *array;
	zval *value;
	uint32_t value_type;
	HashTable *fe_ht;
	HashPosition pos;
	Bucket *p;
	crex_object_iterator *iter;

	array = EX_VAR(opline->op1.var);
	SAVE_OPLINE();

	CREX_ASSERT(C_TYPE_P(array) == IS_OBJECT);
	if ((iter = crex_iterator_unwrap(array)) == NULL) {
		/* plain object */

		fe_ht = C_OBJPROP_P(array);
		pos = crex_hash_iterator_pos(C_FE_ITER_P(array), fe_ht);
		p = fe_ht->arData + pos;
		while (1) {
			if (UNEXPECTED(pos >= fe_ht->nNumUsed)) {
				/* reached end of iteration */
				CREX_VM_C_GOTO(fe_fetch_r_exit);
			}
			pos++;
			value = &p->val;
			value_type = C_TYPE_INFO_P(value);
			if (EXPECTED(value_type != IS_UNDEF)) {
				if (UNEXPECTED(value_type == IS_INDIRECT)) {
					value = C_INDIRECT_P(value);
					value_type = C_TYPE_INFO_P(value);
					if (EXPECTED(value_type != IS_UNDEF)
					 && EXPECTED(crex_check_property_access(C_OBJ_P(array), p->key, 0) == SUCCESS)) {
						break;
					}
				} else if (EXPECTED(C_OBJCE_P(array)->default_properties_count == 0)
						|| !p->key
						|| crex_check_property_access(C_OBJ_P(array), p->key, 1) == SUCCESS) {
					break;
				}
			}
			p++;
		}
		EG(ht_iterators)[C_FE_ITER_P(array)].pos = pos;
		if (RETURN_VALUE_USED(opline)) {
			if (UNEXPECTED(!p->key)) {
				ZVAL_LONG(EX_VAR(opline->result.var), p->h);
			} else if (ZSTR_VAL(p->key)[0]) {
				ZVAL_STR_COPY(EX_VAR(opline->result.var), p->key);
			} else {
				const char *class_name, *prop_name;
				size_t prop_name_len;
				crex_unmangle_property_name_ex(
					p->key, &class_name, &prop_name, &prop_name_len);
				ZVAL_STRINGL(EX_VAR(opline->result.var), prop_name, prop_name_len);
			}
		}
	} else {
		const crex_object_iterator_funcs *funcs = iter->funcs;
		if (EXPECTED(++iter->index > 0)) {
			/* This could cause an endless loop if index becomes zero again.
			 * In case that ever happens we need an additional flag. */
			funcs->move_forward(iter);
			if (UNEXPECTED(EG(exception) != NULL)) {
				UNDEF_RESULT();
				HANDLE_EXCEPTION();
			}
			if (UNEXPECTED(funcs->valid(iter) == FAILURE)) {
				/* reached end of iteration */
				if (UNEXPECTED(EG(exception) != NULL)) {
					UNDEF_RESULT();
					HANDLE_EXCEPTION();
				}
CREX_VM_C_LABEL(fe_fetch_r_exit):
				CREX_VM_SET_RELATIVE_OPCODE(opline, opline->extended_value);
				CREX_VM_CONTINUE();
			}
		}
		value = funcs->get_current_data(iter);
		if (UNEXPECTED(EG(exception) != NULL)) {
			UNDEF_RESULT();
			HANDLE_EXCEPTION();
		}
		if (!value) {
			/* failure in get_current_data */
			CREX_VM_C_GOTO(fe_fetch_r_exit);
		}
		if (RETURN_VALUE_USED(opline)) {
			if (funcs->get_current_key) {
				funcs->get_current_key(iter, EX_VAR(opline->result.var));
				if (UNEXPECTED(EG(exception) != NULL)) {
					UNDEF_RESULT();
					HANDLE_EXCEPTION();
				}
			} else {
				ZVAL_LONG(EX_VAR(opline->result.var), iter->index);
			}
		}
		value_type = C_TYPE_INFO_P(value);
	}

	if (EXPECTED(OP2_TYPE == IS_CV)) {
		zval *variable_ptr = EX_VAR(opline->op2.var);
		crex_assign_to_variable(variable_ptr, value, IS_CV, EX_USES_STRICT_TYPES());
	} else {
		zval *res = EX_VAR(opline->op2.var);
		crex_refcounted *gc = C_COUNTED_P(value);

		ZVAL_COPY_VALUE_EX(res, value, gc, value_type);
		if (C_TYPE_INFO_REFCOUNTED(value_type)) {
			GC_ADDREF(gc);
		}
	}
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HOT_HANDLER(78, CREX_FE_FETCH_R, VAR, ANY, JMP_ADDR)
{
	USE_OPLINE
	zval *array;
	zval *value;
	uint32_t value_type;
	HashTable *fe_ht;
	HashPosition pos;

	array = EX_VAR(opline->op1.var);
	if (UNEXPECTED(C_TYPE_P(array) != IS_ARRAY)) {
		CREX_VM_DISPATCH_TO_HELPER(crex_fe_fetch_object_helper);
	}
	fe_ht = C_ARRVAL_P(array);
	pos = C_FE_POS_P(array);
	if (HT_IS_PACKED(fe_ht)) {
		value = fe_ht->arPacked + pos;
		while (1) {
			if (UNEXPECTED(pos >= fe_ht->nNumUsed)) {
				/* reached end of iteration */
				CREX_VM_SET_RELATIVE_OPCODE(opline, opline->extended_value);
				CREX_VM_CONTINUE();
			}
			value_type = C_TYPE_INFO_P(value);
			CREX_ASSERT(value_type != IS_INDIRECT);
			if (EXPECTED(value_type != IS_UNDEF)) {
				break;
			}
			pos++;
			value++;
		}
		C_FE_POS_P(array) = pos + 1;
		if (RETURN_VALUE_USED(opline)) {
			ZVAL_LONG(EX_VAR(opline->result.var), pos);
		}
	} else {
		Bucket *p;

		p = fe_ht->arData + pos;
		while (1) {
			if (UNEXPECTED(pos >= fe_ht->nNumUsed)) {
				/* reached end of iteration */
				CREX_VM_SET_RELATIVE_OPCODE(opline, opline->extended_value);
				CREX_VM_CONTINUE();
			}
			pos++;
			value = &p->val;
			value_type = C_TYPE_INFO_P(value);
			CREX_ASSERT(value_type != IS_INDIRECT);
			if (EXPECTED(value_type != IS_UNDEF)) {
				break;
			}
			p++;
		}
		C_FE_POS_P(array) = pos;
		if (RETURN_VALUE_USED(opline)) {
			if (!p->key) {
				ZVAL_LONG(EX_VAR(opline->result.var), p->h);
			} else {
				ZVAL_STR_COPY(EX_VAR(opline->result.var), p->key);
			}
		}
	}
	if (EXPECTED(OP2_TYPE == IS_CV)) {
		zval *variable_ptr = EX_VAR(opline->op2.var);
		SAVE_OPLINE();
		crex_assign_to_variable(variable_ptr, value, IS_CV, EX_USES_STRICT_TYPES());
		CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
	} else {
		zval *res = EX_VAR(opline->op2.var);
		crex_refcounted *gc = C_COUNTED_P(value);

		ZVAL_COPY_VALUE_EX(res, value, gc, value_type);
		if (C_TYPE_INFO_REFCOUNTED(value_type)) {
			GC_ADDREF(gc);
		}
		CREX_VM_NEXT_OPCODE();
	}
}

CREX_VM_HANDLER(126, CREX_FE_FETCH_RW, VAR, ANY, JMP_ADDR)
{
	USE_OPLINE
	zval *array;
	zval *value;
	uint32_t value_type;
	HashTable *fe_ht;
	HashPosition pos;
	Bucket *p;

	array = EX_VAR(opline->op1.var);
	SAVE_OPLINE();

	ZVAL_DEREF(array);
	if (EXPECTED(C_TYPE_P(array) == IS_ARRAY)) {
		pos = crex_hash_iterator_pos_ex(C_FE_ITER_P(EX_VAR(opline->op1.var)), array);
		fe_ht = C_ARRVAL_P(array);
		if (HT_IS_PACKED(fe_ht)) {
			value = fe_ht->arPacked + pos;
			while (1) {
				if (UNEXPECTED(pos >= fe_ht->nNumUsed)) {
					/* reached end of iteration */
					CREX_VM_C_GOTO(fe_fetch_w_exit);
				}
				value_type = C_TYPE_INFO_P(value);
				CREX_ASSERT(value_type != IS_INDIRECT);
				if (EXPECTED(value_type != IS_UNDEF)) {
					break;
				}
				pos++;
				value++;
			}
			EG(ht_iterators)[C_FE_ITER_P(EX_VAR(opline->op1.var))].pos = pos + 1;
			if (RETURN_VALUE_USED(opline)) {
				ZVAL_LONG(EX_VAR(opline->result.var), pos);
			}
		} else {
			p = fe_ht->arData + pos;
			while (1) {
				if (UNEXPECTED(pos >= fe_ht->nNumUsed)) {
					/* reached end of iteration */
					CREX_VM_C_GOTO(fe_fetch_w_exit);
				}
				pos++;
				value = &p->val;
				value_type = C_TYPE_INFO_P(value);
				CREX_ASSERT(value_type != IS_INDIRECT);
				if (EXPECTED(value_type != IS_UNDEF)) {
					break;
				}
				p++;
			}
			EG(ht_iterators)[C_FE_ITER_P(EX_VAR(opline->op1.var))].pos = pos;
			if (RETURN_VALUE_USED(opline)) {
				if (!p->key) {
					ZVAL_LONG(EX_VAR(opline->result.var), p->h);
				} else {
					ZVAL_STR_COPY(EX_VAR(opline->result.var), p->key);
				}
			}
		}
	} else if (EXPECTED(C_TYPE_P(array) == IS_OBJECT)) {
		crex_object_iterator *iter;

		if ((iter = crex_iterator_unwrap(array)) == NULL) {
			/* plain object */

			fe_ht = C_OBJPROP_P(array);
			pos = crex_hash_iterator_pos(C_FE_ITER_P(EX_VAR(opline->op1.var)), fe_ht);
			p = fe_ht->arData + pos;
			while (1) {
				if (UNEXPECTED(pos >= fe_ht->nNumUsed)) {
					/* reached end of iteration */
					CREX_VM_C_GOTO(fe_fetch_w_exit);
				}
				pos++;
				value = &p->val;
				value_type = C_TYPE_INFO_P(value);
				if (EXPECTED(value_type != IS_UNDEF)) {
					if (UNEXPECTED(value_type == IS_INDIRECT)) {
						value = C_INDIRECT_P(value);
						value_type = C_TYPE_INFO_P(value);
						if (EXPECTED(value_type != IS_UNDEF)
						 && EXPECTED(crex_check_property_access(C_OBJ_P(array), p->key, 0) == SUCCESS)) {
							if ((value_type & C_TYPE_MASK) != IS_REFERENCE) {
								crex_property_info *prop_info =
									crex_get_property_info_for_slot(C_OBJ_P(array), value);
								if (UNEXPECTED(prop_info)) {
									if (UNEXPECTED(prop_info->flags & CREX_ACC_READONLY)) {
										crex_throw_error(NULL,
											"Cannot acquire reference to readonly property %s::$%s",
											ZSTR_VAL(prop_info->ce->name), ZSTR_VAL(p->key));
										UNDEF_RESULT();
										HANDLE_EXCEPTION();
									}
									if (CREX_TYPE_IS_SET(prop_info->type)) {
										ZVAL_NEW_REF(value, value);
										CREX_REF_ADD_TYPE_SOURCE(C_REF_P(value), prop_info);
										value_type = IS_REFERENCE_EX;
									}
								}
							}
							break;
						}
					} else if (EXPECTED(C_OBJCE_P(array)->default_properties_count == 0)
							|| !p->key
							|| crex_check_property_access(C_OBJ_P(array), p->key, 1) == SUCCESS) {
						break;
					}
				}
				p++;
			}
			EG(ht_iterators)[C_FE_ITER_P(EX_VAR(opline->op1.var))].pos = pos;
			if (RETURN_VALUE_USED(opline)) {
				if (UNEXPECTED(!p->key)) {
					ZVAL_LONG(EX_VAR(opline->result.var), p->h);
				} else if (ZSTR_VAL(p->key)[0]) {
					ZVAL_STR_COPY(EX_VAR(opline->result.var), p->key);
				} else {
					const char *class_name, *prop_name;
					size_t prop_name_len;
					crex_unmangle_property_name_ex(
						p->key, &class_name, &prop_name, &prop_name_len);
					ZVAL_STRINGL(EX_VAR(opline->result.var), prop_name, prop_name_len);
				}
			}
		} else {
			const crex_object_iterator_funcs *funcs = iter->funcs;
			if (++iter->index > 0) {
				/* This could cause an endless loop if index becomes zero again.
				 * In case that ever happens we need an additional flag. */
				funcs->move_forward(iter);
				if (UNEXPECTED(EG(exception) != NULL)) {
					UNDEF_RESULT();
					HANDLE_EXCEPTION();
				}
				if (UNEXPECTED(funcs->valid(iter) == FAILURE)) {
					/* reached end of iteration */
					if (UNEXPECTED(EG(exception) != NULL)) {
						UNDEF_RESULT();
						HANDLE_EXCEPTION();
					}
					CREX_VM_C_GOTO(fe_fetch_w_exit);
				}
			}
			value = funcs->get_current_data(iter);
			if (UNEXPECTED(EG(exception) != NULL)) {
				UNDEF_RESULT();
				HANDLE_EXCEPTION();
			}
			if (!value) {
				/* failure in get_current_data */
				CREX_VM_C_GOTO(fe_fetch_w_exit);
			}
			if (RETURN_VALUE_USED(opline)) {
				if (funcs->get_current_key) {
					funcs->get_current_key(iter, EX_VAR(opline->result.var));
					if (UNEXPECTED(EG(exception) != NULL)) {
						UNDEF_RESULT();
						HANDLE_EXCEPTION();
					}
				} else {
					ZVAL_LONG(EX_VAR(opline->result.var), iter->index);
				}
			}
			value_type = C_TYPE_INFO_P(value);
		}
	} else {
		crex_error(E_WARNING, "foreach() argument must be of type array|object, %s given", crex_zval_value_name(array));
		if (UNEXPECTED(EG(exception))) {
			UNDEF_RESULT();
			HANDLE_EXCEPTION();
		}
CREX_VM_C_LABEL(fe_fetch_w_exit):
		CREX_VM_SET_RELATIVE_OPCODE(opline, opline->extended_value);
		CREX_VM_CONTINUE();
	}

	if (EXPECTED((value_type & C_TYPE_MASK) != IS_REFERENCE)) {
		crex_refcounted *gc = C_COUNTED_P(value);
		zval *ref;
		ZVAL_NEW_EMPTY_REF(value);
		ref = C_REFVAL_P(value);
		ZVAL_COPY_VALUE_EX(ref, value, gc, value_type);
	}
	if (EXPECTED(OP2_TYPE == IS_CV)) {
		zval *variable_ptr = EX_VAR(opline->op2.var);
		if (EXPECTED(variable_ptr != value)) {
			crex_reference *ref;

			ref = C_REF_P(value);
			GC_ADDREF(ref);
			i_zval_ptr_dtor(variable_ptr);
			ZVAL_REF(variable_ptr, ref);
		}
	} else {
		C_ADDREF_P(value);
		ZVAL_REF(EX_VAR(opline->op2.var), C_REF_P(value));
	}
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HOT_HANDLER(154, CREX_ISSET_ISEMPTY_CV, CV, UNUSED, ISSET, SPEC(ISSET))
{
	USE_OPLINE
	zval *value;

	value = EX_VAR(opline->op1.var);
	if (!(opline->extended_value & CREX_ISEMPTY)) {
		if (C_TYPE_P(value) > IS_NULL &&
		    (!C_ISREF_P(value) || C_TYPE_P(C_REFVAL_P(value)) != IS_NULL)) {
			CREX_VM_SMART_BRANCH_TRUE();
		} else {
			CREX_VM_SMART_BRANCH_FALSE();
		}
	} else {
		bool result;

		SAVE_OPLINE();
		result = !i_crex_is_true(value);
		CREX_VM_SMART_BRANCH(result, 1);
	}
}

CREX_VM_HANDLER(114, CREX_ISSET_ISEMPTY_VAR, CONST|TMPVAR|CV, UNUSED, VAR_FETCH|ISSET)
{
	USE_OPLINE
	zval *value;
	/* Should be bool result? as below got: result = (opline->extended_value & CREX_ISEMPTY) */
	int result;
	zval *varname;
	crex_string *name, *tmp_name;
	HashTable *target_symbol_table;

	SAVE_OPLINE();
	varname = GET_OP1_ZVAL_PTR(BP_VAR_IS);
	if (OP1_TYPE == IS_CONST) {
		name = C_STR_P(varname);
	} else {
		name = zval_get_tmp_string(varname, &tmp_name);
	}

	target_symbol_table = crex_get_target_symbol_table(opline->extended_value EXECUTE_DATA_CC);
	value = crex_hash_find_ex(target_symbol_table, name, OP1_TYPE == IS_CONST);

	if (OP1_TYPE != IS_CONST) {
		crex_tmp_string_release(tmp_name);
	}
	FREE_OP1();

	if (!value) {
		result = (opline->extended_value & CREX_ISEMPTY);
	} else {
		if (C_TYPE_P(value) == IS_INDIRECT) {
			value = C_INDIRECT_P(value);
		}
		if (!(opline->extended_value & CREX_ISEMPTY)) {
			if (C_ISREF_P(value)) {
				value = C_REFVAL_P(value);
			}
			result = C_TYPE_P(value) > IS_NULL;
		} else {
			result = !i_crex_is_true(value);
		}
	}

	CREX_VM_SMART_BRANCH(result, 1);
}

/* No specialization for op_types (CONST|TMPVAR|CV, UNUSED|CLASS_FETCH|CONST|VAR) */
CREX_VM_HANDLER(180, CREX_ISSET_ISEMPTY_STATIC_PROP, ANY, CLASS_FETCH, ISSET|CACHE_SLOT)
{
	USE_OPLINE
	zval *value;
	crex_result fetch_result;
	bool result;

	SAVE_OPLINE();

	fetch_result = crex_fetch_static_property_address(&value, NULL, opline->extended_value & ~CREX_ISEMPTY, BP_VAR_IS, 0 OPLINE_CC EXECUTE_DATA_CC);

	if (!(opline->extended_value & CREX_ISEMPTY)) {
		result = fetch_result == SUCCESS && C_TYPE_P(value) > IS_NULL &&
		    (!C_ISREF_P(value) || C_TYPE_P(C_REFVAL_P(value)) != IS_NULL);
	} else {
		result = fetch_result != SUCCESS || !i_crex_is_true(value);
	}

	CREX_VM_SMART_BRANCH(result, 1);
}

CREX_VM_COLD_CONSTCONST_HANDLER(115, CREX_ISSET_ISEMPTY_DIM_OBJ, CONST|TMPVAR|CV, CONST|TMPVAR|CV, ISSET)
{
	USE_OPLINE
	zval *container;
	bool result;
	crex_ulong hval;
	zval *offset;

	SAVE_OPLINE();
	container = GET_OP1_OBJ_ZVAL_PTR_UNDEF(BP_VAR_IS);
	offset = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);

	if (EXPECTED(C_TYPE_P(container) == IS_ARRAY)) {
		HashTable *ht;
		zval *value;
		crex_string *str;

CREX_VM_C_LABEL(isset_dim_obj_array):
		ht = C_ARRVAL_P(container);
CREX_VM_C_LABEL(isset_again):
		if (EXPECTED(C_TYPE_P(offset) == IS_STRING)) {
			str = C_STR_P(offset);
			if (OP2_TYPE != IS_CONST) {
				if (CREX_HANDLE_NUMERIC(str, hval)) {
					CREX_VM_C_GOTO(num_index_prop);
				}
			}
			value = crex_hash_find_ex(ht, str, OP2_TYPE == IS_CONST);
		} else if (EXPECTED(C_TYPE_P(offset) == IS_LONG)) {
			hval = C_LVAL_P(offset);
CREX_VM_C_LABEL(num_index_prop):
			value = crex_hash_index_find(ht, hval);
		} else if ((OP2_TYPE & (IS_VAR|IS_CV)) && EXPECTED(C_ISREF_P(offset))) {
			offset = C_REFVAL_P(offset);
			CREX_VM_C_GOTO(isset_again);
		} else {
			value = crex_find_array_dim_slow(ht, offset EXECUTE_DATA_CC);
			if (UNEXPECTED(EG(exception))) {
				result = 0;
				CREX_VM_C_GOTO(isset_dim_obj_exit);
			}
		}

		if (!(opline->extended_value & CREX_ISEMPTY)) {
			/* > IS_NULL means not IS_UNDEF and not IS_NULL */
			result = value != NULL && C_TYPE_P(value) > IS_NULL &&
			    (!C_ISREF_P(value) || C_TYPE_P(C_REFVAL_P(value)) != IS_NULL);

			if (OP1_TYPE & (IS_CONST|IS_CV)) {
				/* avoid exception check */
				FREE_OP2();
				CREX_VM_SMART_BRANCH(result, 0);
			}
		} else {
			result = (value == NULL || !i_crex_is_true(value));
		}
		CREX_VM_C_GOTO(isset_dim_obj_exit);
	} else if ((OP1_TYPE & (IS_VAR|IS_CV)) && EXPECTED(C_ISREF_P(container))) {
		container = C_REFVAL_P(container);
		if (EXPECTED(C_TYPE_P(container) == IS_ARRAY)) {
			CREX_VM_C_GOTO(isset_dim_obj_array);
		}
	}

	if (OP2_TYPE == IS_CONST && C_EXTRA_P(offset) == CREX_EXTRA_VALUE) {
		offset++;
	}
	if (!(opline->extended_value & CREX_ISEMPTY)) {
		result = crex_isset_dim_slow(container, offset EXECUTE_DATA_CC);
	} else {
		result = crex_isempty_dim_slow(container, offset EXECUTE_DATA_CC);
	}

CREX_VM_C_LABEL(isset_dim_obj_exit):
	FREE_OP2();
	FREE_OP1();
	CREX_VM_SMART_BRANCH(result, 1);
}

CREX_VM_COLD_CONST_HANDLER(148, CREX_ISSET_ISEMPTY_PROP_OBJ, CONST|TMPVAR|UNUSED|THIS|CV, CONST|TMPVAR|CV, ISSET|CACHE_SLOT)
{
	USE_OPLINE
	zval *container;
	int result;
	zval *offset;
	crex_string *name, *tmp_name;

	SAVE_OPLINE();
	container = GET_OP1_OBJ_ZVAL_PTR(BP_VAR_IS);
	offset = GET_OP2_ZVAL_PTR(BP_VAR_R);

	if (OP1_TYPE == IS_CONST ||
	    (OP1_TYPE != IS_UNUSED && UNEXPECTED(C_TYPE_P(container) != IS_OBJECT))) {
		if ((OP1_TYPE & (IS_VAR|IS_CV)) && C_ISREF_P(container)) {
			container = C_REFVAL_P(container);
			if (UNEXPECTED(C_TYPE_P(container) != IS_OBJECT)) {
				result = (opline->extended_value & CREX_ISEMPTY);
				CREX_VM_C_GOTO(isset_object_finish);
			}
		} else {
			result = (opline->extended_value & CREX_ISEMPTY);
			CREX_VM_C_GOTO(isset_object_finish);
		}
	}

	if (OP2_TYPE == IS_CONST) {
		name = C_STR_P(offset);
	} else {
		name = zval_try_get_tmp_string(offset, &tmp_name);
		if (UNEXPECTED(!name)) {
			result = 0;
			CREX_VM_C_GOTO(isset_object_finish);
		}
	}

	result =
		(opline->extended_value & CREX_ISEMPTY) ^
		C_OBJ_HT_P(container)->has_property(C_OBJ_P(container), name, (opline->extended_value & CREX_ISEMPTY), ((OP2_TYPE == IS_CONST) ? CACHE_ADDR(opline->extended_value & ~CREX_ISEMPTY) : NULL));

	if (OP2_TYPE != IS_CONST) {
		crex_tmp_string_release(tmp_name);
	}

CREX_VM_C_LABEL(isset_object_finish):
	FREE_OP2();
	FREE_OP1();
	CREX_VM_SMART_BRANCH(result, 1);
}

CREX_VM_HANDLER(194, CREX_ARRAY_KEY_EXISTS, CV|TMPVAR|CONST, CV|TMPVAR|CONST)
{
	USE_OPLINE

	zval *key, *subject;
	HashTable *ht;
	bool result;

	SAVE_OPLINE();

	key = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	subject = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);

	if (EXPECTED(C_TYPE_P(subject) == IS_ARRAY)) {
CREX_VM_C_LABEL(array_key_exists_array):
		ht = C_ARRVAL_P(subject);
		result = crex_array_key_exists_fast(ht, key OPLINE_CC EXECUTE_DATA_CC);
	} else {
		if ((OP2_TYPE & (IS_VAR|IS_CV)) && EXPECTED(C_ISREF_P(subject))) {
			subject = C_REFVAL_P(subject);
			if (EXPECTED(C_TYPE_P(subject) == IS_ARRAY)) {
				CREX_VM_C_GOTO(array_key_exists_array);
			}
		}
		crex_array_key_exists_error(subject, key OPLINE_CC EXECUTE_DATA_CC);
		result = 0;
	}

	FREE_OP2();
	FREE_OP1();
	CREX_VM_SMART_BRANCH(result, 1);
}

/* No specialization for op_types (CONST|TMPVAR|UNUSED|CV, ANY) */
CREX_VM_COLD_HANDLER(79, CREX_EXIT, ANY, ANY)
{
	USE_OPLINE

	SAVE_OPLINE();
	if (OP1_TYPE != IS_UNUSED) {
		zval *ptr = GET_OP1_ZVAL_PTR(BP_VAR_R);

		do {
			if (C_TYPE_P(ptr) == IS_LONG) {
				EG(exit_status) = C_LVAL_P(ptr);
			} else {
				if ((OP1_TYPE & (IS_VAR|IS_CV)) && C_ISREF_P(ptr)) {
					ptr = C_REFVAL_P(ptr);
					if (C_TYPE_P(ptr) == IS_LONG) {
						EG(exit_status) = C_LVAL_P(ptr);
						break;
					}
				}
				crex_print_zval(ptr, 0);
			}
		} while (0);
		FREE_OP1();
	}

	if (!EG(exception)) {
		crex_throw_unwind_exit();
	}
	HANDLE_EXCEPTION();
}

CREX_VM_HANDLER(57, CREX_BEGIN_SILENCE, ANY, ANY)
{
	USE_OPLINE

	ZVAL_LONG(EX_VAR(opline->result.var), EG(error_reporting));

	if (!E_HAS_ONLY_FATAL_ERRORS(EG(error_reporting))) {
		do {
			/* Do not silence fatal errors */
			EG(error_reporting) &= E_FATAL_ERRORS;
			if (!EG(error_reporting_ini_entry)) {
				zval *zv = crex_hash_find_known_hash(EG(ini_directives), ZSTR_KNOWN(CREX_STR_ERROR_REPORTING));
				if (zv) {
					EG(error_reporting_ini_entry) = (crex_ini_entry *)C_PTR_P(zv);
				} else {
					break;
				}
			}
			if (!EG(error_reporting_ini_entry)->modified) {
				if (!EG(modified_ini_directives)) {
					ALLOC_HASHTABLE(EG(modified_ini_directives));
					crex_hash_init(EG(modified_ini_directives), 8, NULL, NULL, 0);
				}
				if (EXPECTED(crex_hash_add_ptr(EG(modified_ini_directives), ZSTR_KNOWN(CREX_STR_ERROR_REPORTING), EG(error_reporting_ini_entry)) != NULL)) {
					EG(error_reporting_ini_entry)->orig_value = EG(error_reporting_ini_entry)->value;
					EG(error_reporting_ini_entry)->orig_modifiable = EG(error_reporting_ini_entry)->modifiable;
					EG(error_reporting_ini_entry)->modified = 1;
				}
			}
		} while (0);
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(58, CREX_END_SILENCE, TMP, ANY)
{
	USE_OPLINE

	if (E_HAS_ONLY_FATAL_ERRORS(EG(error_reporting))
			&& !E_HAS_ONLY_FATAL_ERRORS(C_LVAL_P(EX_VAR(opline->op1.var)))) {
		EG(error_reporting) = C_LVAL_P(EX_VAR(opline->op1.var));
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_COLD_CONST_HANDLER(152, CREX_JMP_SET, CONST|TMP|VAR|CV, JMP_ADDR)
{
	USE_OPLINE
	zval *value;
	crex_reference *ref = NULL;
	bool ret;

	SAVE_OPLINE();
	value = GET_OP1_ZVAL_PTR(BP_VAR_R);

	if ((OP1_TYPE == IS_VAR || OP1_TYPE == IS_CV) && C_ISREF_P(value)) {
		if (OP1_TYPE == IS_VAR) {
			ref = C_REF_P(value);
		}
		value = C_REFVAL_P(value);
	}

	ret = i_crex_is_true(value);

	if (UNEXPECTED(EG(exception))) {
		FREE_OP1();
		ZVAL_UNDEF(EX_VAR(opline->result.var));
		HANDLE_EXCEPTION();
	}

	if (ret) {
		zval *result = EX_VAR(opline->result.var);

		ZVAL_COPY_VALUE(result, value);
		if (OP1_TYPE == IS_CONST) {
			if (UNEXPECTED(C_OPT_REFCOUNTED_P(result))) C_ADDREF_P(result);
		} else if (OP1_TYPE == IS_CV) {
			if (C_OPT_REFCOUNTED_P(result)) C_ADDREF_P(result);
		} else if (OP1_TYPE == IS_VAR && ref) {
			if (UNEXPECTED(GC_DELREF(ref) == 0)) {
				efree_size(ref, sizeof(crex_reference));
			} else if (C_OPT_REFCOUNTED_P(result)) {
				C_ADDREF_P(result);
			}
		}
		CREX_VM_JMP_EX(OP_JMP_ADDR(opline, opline->op2), 0);
	}

	FREE_OP1();
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_COLD_CONST_HANDLER(169, CREX_COALESCE, CONST|TMP|VAR|CV, JMP_ADDR)
{
	USE_OPLINE
	zval *value;
	crex_reference *ref = NULL;

	SAVE_OPLINE();
	value = GET_OP1_ZVAL_PTR(BP_VAR_IS);

	if ((OP1_TYPE & (IS_VAR|IS_CV)) && C_ISREF_P(value)) {
		if (OP1_TYPE & IS_VAR) {
			ref = C_REF_P(value);
		}
		value = C_REFVAL_P(value);
	}

	if (C_TYPE_P(value) > IS_NULL) {
		zval *result = EX_VAR(opline->result.var);
		ZVAL_COPY_VALUE(result, value);
		if (OP1_TYPE == IS_CONST) {
			if (UNEXPECTED(C_OPT_REFCOUNTED_P(result))) C_ADDREF_P(result);
		} else if (OP1_TYPE == IS_CV) {
			if (C_OPT_REFCOUNTED_P(result)) C_ADDREF_P(result);
		} else if ((OP1_TYPE & IS_VAR) && ref) {
			if (UNEXPECTED(GC_DELREF(ref) == 0)) {
				efree_size(ref, sizeof(crex_reference));
			} else if (C_OPT_REFCOUNTED_P(result)) {
				C_ADDREF_P(result);
			}
		}
		CREX_VM_JMP_EX(OP_JMP_ADDR(opline, opline->op2), 0);
	}

	if ((OP1_TYPE & IS_VAR) && ref) {
		if (UNEXPECTED(GC_DELREF(ref) == 0)) {
			efree_size(ref, sizeof(crex_reference));
		}
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_NOCONST_HANDLER(198, CREX_JMP_NULL, CONST|TMP|VAR|CV, JMP_ADDR)
{
	USE_OPLINE
	zval *val, *result;

	val = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);

	if (C_TYPE_P(val) > IS_NULL) {
		do {
			if ((OP1_TYPE == IS_CV || OP1_TYPE == IS_VAR) && C_TYPE_P(val) == IS_REFERENCE) {
				val = C_REFVAL_P(val);
				if (C_TYPE_P(val) <= IS_NULL) {
					FREE_OP1();
					break;
				}
			}
			CREX_VM_NEXT_OPCODE();
		} while (0);
	}

	result = EX_VAR(opline->result.var);
	uint32_t short_circuiting_type = opline->extended_value & CREX_SHORT_CIRCUITING_CHAIN_MASK;
	if (EXPECTED(short_circuiting_type == CREX_SHORT_CIRCUITING_CHAIN_EXPR)) {
		ZVAL_NULL(result);
		if (OP1_TYPE == IS_CV
			&& UNEXPECTED(C_TYPE_P(val) == IS_UNDEF)
			&& (opline->extended_value & CREX_JMP_NULL_BP_VAR_IS) == 0
		) {
			SAVE_OPLINE();
			ZVAL_UNDEFINED_OP1();
			if (UNEXPECTED(EG(exception) != NULL)) {
				HANDLE_EXCEPTION();
			}
		}
	} else if (short_circuiting_type == CREX_SHORT_CIRCUITING_CHAIN_ISSET) {
		ZVAL_FALSE(result);
	} else {
		CREX_ASSERT(short_circuiting_type == CREX_SHORT_CIRCUITING_CHAIN_EMPTY);
		ZVAL_TRUE(result);
	}

	CREX_VM_JMP_EX(OP_JMP_ADDR(opline, opline->op2), 0);
}

CREX_VM_HOT_HANDLER(31, CREX_QM_ASSIGN, CONST|TMP|VAR|CV, ANY)
{
	USE_OPLINE
	zval *value;
	zval *result = EX_VAR(opline->result.var);

	value = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(value) == IS_UNDEF)) {
		SAVE_OPLINE();
		ZVAL_UNDEFINED_OP1();
		ZVAL_NULL(result);
		CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
	}

	if (OP1_TYPE == IS_CV) {
		ZVAL_COPY_DEREF(result, value);
	} else if (OP1_TYPE == IS_VAR) {
		if (UNEXPECTED(C_ISREF_P(value))) {
			ZVAL_COPY_VALUE(result, C_REFVAL_P(value));
			if (UNEXPECTED(C_DELREF_P(value) == 0)) {
				efree_size(C_REF_P(value), sizeof(crex_reference));
			} else if (C_OPT_REFCOUNTED_P(result)) {
				C_ADDREF_P(result);
			}
		} else {
			ZVAL_COPY_VALUE(result, value);
		}
	} else {
		ZVAL_COPY_VALUE(result, value);
		if (OP1_TYPE == IS_CONST) {
			if (UNEXPECTED(C_OPT_REFCOUNTED_P(result))) {
				C_ADDREF_P(result);
			}
		}
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_COLD_HANDLER(101, CREX_EXT_STMT, ANY, ANY)
{
	USE_OPLINE

	if (!EG(no_extensions)) {
		SAVE_OPLINE();
		crex_llist_apply_with_argument(&crex_extensions, (llist_apply_with_arg_func_t) crex_extension_statement_handler, execute_data);
		CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_COLD_HANDLER(102, CREX_EXT_FCALL_BEGIN, ANY, ANY)
{
	USE_OPLINE

	if (!EG(no_extensions)) {
		SAVE_OPLINE();
		crex_llist_apply_with_argument(&crex_extensions, (llist_apply_with_arg_func_t) crex_extension_fcall_begin_handler, execute_data);
		CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_COLD_HANDLER(103, CREX_EXT_FCALL_END, ANY, ANY)
{
	USE_OPLINE

	if (!EG(no_extensions)) {
		SAVE_OPLINE();
		crex_llist_apply_with_argument(&crex_extensions, (llist_apply_with_arg_func_t) crex_extension_fcall_end_handler, execute_data);
		CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(144, CREX_DECLARE_CLASS, CONST, ANY)
{
	USE_OPLINE

	SAVE_OPLINE();
	do_bind_class(RT_CONSTANT(opline, opline->op1), (OP2_TYPE == IS_CONST) ? C_STR_P(RT_CONSTANT(opline, opline->op2)) : NULL);
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(145, CREX_DECLARE_CLASS_DELAYED, CONST, CONST)
{
	USE_OPLINE

	crex_class_entry *ce = CACHED_PTR(opline->extended_value);
	if (ce == NULL) {
		zval *lcname = RT_CONSTANT(opline, opline->op1);
		zval *zv = crex_hash_find_known_hash(EG(class_table), C_STR_P(lcname + 1));
		if (zv) {
			SAVE_OPLINE();
			ce = crex_bind_class_in_slot(zv, lcname, C_STR_P(RT_CONSTANT(opline, opline->op2)));
			if (!ce) {
				HANDLE_EXCEPTION();
			}
		}
		CACHE_PTR(opline->extended_value, ce);
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(146, CREX_DECLARE_ANON_CLASS, ANY, ANY, CACHE_SLOT)
{
	zval *zv;
	crex_class_entry *ce;
	USE_OPLINE

	ce = CACHED_PTR(opline->extended_value);
	if (UNEXPECTED(ce == NULL)) {
		crex_string *rtd_key = C_STR_P(RT_CONSTANT(opline, opline->op1));
		zv = crex_hash_find_known_hash(EG(class_table), rtd_key);
		CREX_ASSERT(zv != NULL);
		ce = C_CE_P(zv);
		if (!(ce->ce_flags & CREX_ACC_LINKED)) {
			SAVE_OPLINE();
			ce = crex_do_link_class(ce, (OP2_TYPE == IS_CONST) ? C_STR_P(RT_CONSTANT(opline, opline->op2)) : NULL, rtd_key);
			if (!ce) {
				HANDLE_EXCEPTION();
			}
		}
		CACHE_PTR(opline->extended_value, ce);
	}
	C_CE_P(EX_VAR(opline->result.var)) = ce;
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(141, CREX_DECLARE_FUNCTION, ANY, NUM)
{
	crex_function *func;
	USE_OPLINE

	SAVE_OPLINE();
	func = (crex_function *) EX(func)->op_array.dynamic_func_defs[opline->op2.num];
	do_bind_function(func, RT_CONSTANT(opline, opline->op1));
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(105, CREX_TICKS, ANY, ANY, NUM)
{
	USE_OPLINE

	if ((uint32_t)++EG(ticks_count) >= opline->extended_value) {
		EG(ticks_count) = 0;
		if (crex_ticks_function) {
			SAVE_OPLINE();
			crex_fiber_switch_block();
			crex_ticks_function(opline->extended_value);
			crex_fiber_switch_unblock();
			CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
		}
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(138, CREX_INSTANCEOF, TMPVAR|CV, UNUSED|CLASS_FETCH|CONST|VAR, CACHE_SLOT)
{
	USE_OPLINE
	zval *expr;
	bool result;

	SAVE_OPLINE();
	expr = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);

CREX_VM_C_LABEL(try_instanceof):
	if (C_TYPE_P(expr) == IS_OBJECT) {
		crex_class_entry *ce;

		if (OP2_TYPE == IS_CONST) {
			ce = CACHED_PTR(opline->extended_value);
			if (UNEXPECTED(ce == NULL)) {
				ce = crex_lookup_class_ex(C_STR_P(RT_CONSTANT(opline, opline->op2)), C_STR_P(RT_CONSTANT(opline, opline->op2) + 1), CREX_FETCH_CLASS_NO_AUTOLOAD);
				if (EXPECTED(ce)) {
					CACHE_PTR(opline->extended_value, ce);
				}
			}
		} else if (OP2_TYPE == IS_UNUSED) {
			ce = crex_fetch_class(NULL, opline->op2.num);
			if (UNEXPECTED(ce == NULL)) {
				FREE_OP1();
				ZVAL_UNDEF(EX_VAR(opline->result.var));
				HANDLE_EXCEPTION();
			}
		} else {
			ce = C_CE_P(EX_VAR(opline->op2.var));
		}
		result = ce && instanceof_function(C_OBJCE_P(expr), ce);
	} else if ((OP1_TYPE & (IS_VAR|IS_CV)) && C_TYPE_P(expr) == IS_REFERENCE) {
		expr = C_REFVAL_P(expr);
		CREX_VM_C_GOTO(try_instanceof);
	} else {
		if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(expr) == IS_UNDEF)) {
			ZVAL_UNDEFINED_OP1();
		}
		result = 0;
	}
	FREE_OP1();
	CREX_VM_SMART_BRANCH(result, 1);
}

CREX_VM_HOT_HANDLER(104, CREX_EXT_NOP, ANY, ANY)
{
	USE_OPLINE

	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_HANDLER(0, CREX_NOP, ANY, ANY)
{
	USE_OPLINE

	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HELPER(crex_dispatch_try_catch_finally_helper, ANY, ANY, uint32_t try_catch_offset, uint32_t op_num)
{
	/* May be NULL during generator closing (only finally blocks are executed) */
	crex_object *ex = EG(exception);

	/* Walk try/catch/finally structures upwards, performing the necessary actions */
	for (; try_catch_offset != (uint32_t) -1; try_catch_offset--) {
		crex_try_catch_element *try_catch =
			&EX(func)->op_array.try_catch_array[try_catch_offset];

		if (op_num < try_catch->catch_op && ex) {
			/* Go to catch block */
			cleanup_live_vars(execute_data, op_num, try_catch->catch_op);
			CREX_VM_JMP_EX(&EX(func)->op_array.opcodes[try_catch->catch_op], 0);

		} else if (op_num < try_catch->finally_op) {
			if (ex && crex_is_unwind_exit(ex)) {
				/* Don't execute finally blocks on exit (for now) */
				continue;
			}

			/* Go to finally block */
			zval *fast_call = EX_VAR(EX(func)->op_array.opcodes[try_catch->finally_end].op1.var);
			cleanup_live_vars(execute_data, op_num, try_catch->finally_op);
			C_OBJ_P(fast_call) = EG(exception);
			EG(exception) = NULL;
			C_OPLINE_NUM_P(fast_call) = (uint32_t)-1;
			CREX_VM_JMP_EX(&EX(func)->op_array.opcodes[try_catch->finally_op], 0);

		} else if (op_num < try_catch->finally_end) {
			zval *fast_call = EX_VAR(EX(func)->op_array.opcodes[try_catch->finally_end].op1.var);

			/* cleanup incomplete RETURN statement */
			if (C_OPLINE_NUM_P(fast_call) != (uint32_t)-1
			 && (EX(func)->op_array.opcodes[C_OPLINE_NUM_P(fast_call)].op2_type & (IS_TMP_VAR | IS_VAR))) {
				zval *return_value = EX_VAR(EX(func)->op_array.opcodes[C_OPLINE_NUM_P(fast_call)].op2.var);

				zval_ptr_dtor(return_value);
			}

			/* Chain potential exception from wrapping finally block */
			if (C_OBJ_P(fast_call)) {
				if (ex) {
					if (crex_is_unwind_exit(ex) || crex_is_graceful_exit(ex)) {
						/* discard the previously thrown exception */
						OBJ_RELEASE(C_OBJ_P(fast_call));
					} else {
						crex_exception_set_previous(ex, C_OBJ_P(fast_call));
					}
				} else {
					ex = EG(exception) = C_OBJ_P(fast_call);
				}
			}
		}
	}

	/* Uncaught exception */
	if (crex_observer_fcall_op_array_extension != -1) {
		crex_observer_fcall_end(execute_data, NULL);
	}
	cleanup_live_vars(execute_data, op_num, 0);
	if (UNEXPECTED((EX_CALL_INFO() & CREX_CALL_GENERATOR) != 0)) {
		crex_generator *generator = crex_get_running_generator(EXECUTE_DATA_C);
		EG(current_execute_data) = EX(prev_execute_data);
		crex_generator_close(generator, 1);
		CREX_VM_RETURN();
	} else {
		/* We didn't execute RETURN, and have to initialize return_value */
		if (EX(return_value)) {
			ZVAL_UNDEF(EX(return_value));
		}
		CREX_VM_DISPATCH_TO_HELPER(crex_leave_helper);
	}
}

CREX_VM_HANDLER(149, CREX_HANDLE_EXCEPTION, ANY, ANY)
{
	const crex_op *throw_op = EG(opline_before_exception);

	/* Exception was thrown before executing any op */
	if (UNEXPECTED(!throw_op)) {
		CREX_VM_DISPATCH_TO_HELPER(crex_dispatch_try_catch_finally_helper, try_catch_offset, -1, 0, 0);
	}

	uint32_t throw_op_num = throw_op - EX(func)->op_array.opcodes;
	int i, current_try_catch_offset = -1;

	if ((throw_op->opcode == CREX_FREE || throw_op->opcode == CREX_FE_FREE)
		&& throw_op->extended_value & CREX_FREE_ON_RETURN) {
		/* exceptions thrown because of loop var destruction on return/break/...
		 * are logically thrown at the end of the foreach loop, so adjust the
		 * throw_op_num.
		 */
		const crex_live_range *range = find_live_range(
			&EX(func)->op_array, throw_op_num, throw_op->op1.var);
		/* free op1 of the corresponding RETURN */
		for (i = throw_op_num; i < range->end; i++) {
			if (EX(func)->op_array.opcodes[i].opcode == CREX_FREE
			 || EX(func)->op_array.opcodes[i].opcode == CREX_FE_FREE) {
				/* pass */
			} else {
				if (EX(func)->op_array.opcodes[i].opcode == CREX_RETURN
				 && (EX(func)->op_array.opcodes[i].op1_type & (IS_VAR|IS_TMP_VAR))) {
					zval_ptr_dtor(EX_VAR(EX(func)->op_array.opcodes[i].op1.var));
				}
				break;
			}
		}
		throw_op_num = range->end;
	}

	/* Find the innermost try/catch/finally the exception was thrown in */
	for (i = 0; i < EX(func)->op_array.last_try_catch; i++) {
		crex_try_catch_element *try_catch = &EX(func)->op_array.try_catch_array[i];
		if (try_catch->try_op > throw_op_num) {
			/* further blocks will not be relevant... */
			break;
		}
		if (throw_op_num < try_catch->catch_op || throw_op_num < try_catch->finally_end) {
			current_try_catch_offset = i;
		}
	}

	cleanup_unfinished_calls(execute_data, throw_op_num);

	if (throw_op->result_type & (IS_VAR | IS_TMP_VAR)) {
		switch (throw_op->opcode) {
			case CREX_ADD_ARRAY_ELEMENT:
			case CREX_ADD_ARRAY_UNPACK:
			case CREX_ROPE_INIT:
			case CREX_ROPE_ADD:
				break; /* exception while building structures, live range handling will free those */

			case CREX_FETCH_CLASS:
			case CREX_DECLARE_ANON_CLASS:
				break; /* return value is crex_class_entry pointer */

			default:
				/* smart branch opcodes may not initialize result */
				if (!crex_is_smart_branch(throw_op)) {
					zval_ptr_dtor_nogc(EX_VAR(throw_op->result.var));
				}
		}
	}

	CREX_VM_DISPATCH_TO_HELPER(crex_dispatch_try_catch_finally_helper, try_catch_offset, current_try_catch_offset, op_num, throw_op_num);
}

CREX_VM_HANDLER(150, CREX_USER_OPCODE, ANY, ANY)
{
	USE_OPLINE
	int ret;

	SAVE_OPLINE();
	ret = crex_user_opcode_handlers[opline->opcode](execute_data);
	opline = EX(opline);

	switch (ret) {
		case CREX_USER_OPCODE_CONTINUE:
			CREX_VM_CONTINUE();
		case CREX_USER_OPCODE_RETURN:
			if (UNEXPECTED((EX_CALL_INFO() & CREX_CALL_GENERATOR) != 0)) {
				crex_generator *generator = crex_get_running_generator(EXECUTE_DATA_C);
				EG(current_execute_data) = EX(prev_execute_data);
				crex_generator_close(generator, 1);
				CREX_VM_RETURN();
			} else {
				CREX_VM_DISPATCH_TO_HELPER(crex_leave_helper);
			}
		case CREX_USER_OPCODE_ENTER:
			CREX_VM_ENTER();
		case CREX_USER_OPCODE_LEAVE:
			CREX_VM_LEAVE();
		case CREX_USER_OPCODE_DISPATCH:
			CREX_VM_DISPATCH(opline->opcode, opline);
		default:
			CREX_VM_DISPATCH((uint8_t)(ret & 0xff), opline);
	}
}

CREX_VM_HANDLER(143, CREX_DECLARE_CONST, CONST, CONST)
{
	USE_OPLINE
	zval *name;
	zval *val;
	crex_constant c;

	SAVE_OPLINE();
	name  = GET_OP1_ZVAL_PTR(BP_VAR_R);
	val   = GET_OP2_ZVAL_PTR(BP_VAR_R);

	ZVAL_COPY(&c.value, val);
	if (C_OPT_CONSTANT(c.value)) {
		if (UNEXPECTED(zval_update_constant_ex(&c.value, EX(func)->op_array.scope) != SUCCESS)) {
			zval_ptr_dtor_nogc(&c.value);
			FREE_OP1();
			FREE_OP2();
			HANDLE_EXCEPTION();
		}
	}
	/* non persistent, case sensitive */
	CREX_CONSTANT_SET_FLAGS(&c, 0, CRX_USER_CONSTANT);
	c.name = crex_string_copy(C_STR_P(name));

	if (crex_register_constant(&c) == FAILURE) {
	}

	FREE_OP1();
	FREE_OP2();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(142, CREX_DECLARE_LAMBDA_FUNCTION, CONST, NUM)
{
	USE_OPLINE
	crex_function *func;
	zval *object;
	crex_class_entry *called_scope;

	func = (crex_function *) EX(func)->op_array.dynamic_func_defs[opline->op2.num];
	if (C_TYPE(EX(This)) == IS_OBJECT) {
		called_scope = C_OBJCE(EX(This));
		if (UNEXPECTED((func->common.fn_flags & CREX_ACC_STATIC) ||
				(EX(func)->common.fn_flags & CREX_ACC_STATIC))) {
			object = NULL;
		} else {
			object = &EX(This);
		}
	} else {
		called_scope = C_CE(EX(This));
		object = NULL;
	}
	SAVE_OPLINE();
	crex_create_closure(EX_VAR(opline->result.var), func,
		EX(func)->op_array.scope, called_scope, object);

	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(156, CREX_SEPARATE, VAR, UNUSED)
{
	USE_OPLINE
	zval *var_ptr;

	var_ptr = EX_VAR(opline->op1.var);
	if (UNEXPECTED(C_ISREF_P(var_ptr))) {
		if (UNEXPECTED(C_REFCOUNT_P(var_ptr) == 1)) {
			ZVAL_UNREF(var_ptr);
		}
	}

	CREX_VM_NEXT_OPCODE();
}

CREX_VM_COLD_HELPER(crex_yield_in_closed_generator_helper, ANY, ANY)
{
	USE_OPLINE

	SAVE_OPLINE();
	crex_throw_error(NULL, "Cannot yield from finally in a force-closed generator");
	FREE_OP2();
	FREE_OP1();
	UNDEF_RESULT();
	HANDLE_EXCEPTION();
}

CREX_VM_HANDLER(160, CREX_YIELD, CONST|TMP|VAR|CV|UNUSED, CONST|TMPVAR|CV|UNUSED, SRC)
{
	USE_OPLINE

	crex_generator *generator = crex_get_running_generator(EXECUTE_DATA_C);

	SAVE_OPLINE();
	if (UNEXPECTED(generator->flags & CREX_GENERATOR_FORCED_CLOSE)) {
		CREX_VM_DISPATCH_TO_HELPER(crex_yield_in_closed_generator_helper);
	}

	/* Destroy the previously yielded value */
	zval_ptr_dtor(&generator->value);

	/* Destroy the previously yielded key */
	zval_ptr_dtor(&generator->key);

	/* Set the new yielded value */
	if (OP1_TYPE != IS_UNUSED) {
		if (UNEXPECTED(EX(func)->op_array.fn_flags & CREX_ACC_RETURN_REFERENCE)) {
			/* Constants and temporary variables aren't yieldable by reference,
			 * but we still allow them with a notice. */
			if (OP1_TYPE & (IS_CONST|IS_TMP_VAR)) {
				zval *value;

				crex_error(E_NOTICE, "Only variable references should be yielded by reference");

				value = GET_OP1_ZVAL_PTR(BP_VAR_R);
				ZVAL_COPY_VALUE(&generator->value, value);
				if (OP1_TYPE == IS_CONST) {
					if (UNEXPECTED(C_OPT_REFCOUNTED(generator->value))) {
						C_ADDREF(generator->value);
					}
				}
			} else {
				zval *value_ptr = GET_OP1_ZVAL_PTR_PTR(BP_VAR_W);

				/* If a function call result is yielded and the function did
				 * not return by reference we throw a notice. */
				do {
					if (OP1_TYPE == IS_VAR) {
						CREX_ASSERT(value_ptr != &EG(uninitialized_zval));
						if (opline->extended_value == CREX_RETURNS_FUNCTION
						 && !C_ISREF_P(value_ptr)) {
							crex_error(E_NOTICE, "Only variable references should be yielded by reference");
							ZVAL_COPY(&generator->value, value_ptr);
							break;
						}
					}
					if (C_ISREF_P(value_ptr)) {
						C_ADDREF_P(value_ptr);
					} else {
						ZVAL_MAKE_REF_EX(value_ptr, 2);
					}
					ZVAL_REF(&generator->value, C_REF_P(value_ptr));
				} while (0);

				FREE_OP1();
			}
		} else {
			zval *value = GET_OP1_ZVAL_PTR(BP_VAR_R);

			/* Consts, temporary variables and references need copying */
			if (OP1_TYPE == IS_CONST) {
				ZVAL_COPY_VALUE(&generator->value, value);
				if (UNEXPECTED(C_OPT_REFCOUNTED(generator->value))) {
					C_ADDREF(generator->value);
				}
			} else if (OP1_TYPE == IS_TMP_VAR) {
				ZVAL_COPY_VALUE(&generator->value, value);
			} else if ((OP1_TYPE & (IS_VAR|IS_CV)) && C_ISREF_P(value)) {
				ZVAL_COPY(&generator->value, C_REFVAL_P(value));
				FREE_OP1_IF_VAR();
			} else {
				ZVAL_COPY_VALUE(&generator->value, value);
				if (OP1_TYPE == IS_CV) {
					if (C_OPT_REFCOUNTED_P(value)) C_ADDREF_P(value);
				}
			}
		}
	} else {
		/* If no value was specified yield null */
		ZVAL_NULL(&generator->value);
	}

	/* Set the new yielded key */
	if (OP2_TYPE != IS_UNUSED) {
		zval *key = GET_OP2_ZVAL_PTR(BP_VAR_R);
		if ((OP2_TYPE & (IS_CV|IS_VAR)) && UNEXPECTED(C_TYPE_P(key) == IS_REFERENCE)) {
			key = C_REFVAL_P(key);
		}
		ZVAL_COPY(&generator->key, key);
		FREE_OP2();

		if (C_TYPE(generator->key) == IS_LONG
		    && C_LVAL(generator->key) > generator->largest_used_integer_key
		) {
			generator->largest_used_integer_key = C_LVAL(generator->key);
		}
	} else {
		/* If no key was specified we use auto-increment keys */
		generator->largest_used_integer_key++;
		ZVAL_LONG(&generator->key, generator->largest_used_integer_key);
	}

	if (RETURN_VALUE_USED(opline)) {
		/* If the return value of yield is used set the send
		 * target and initialize it to NULL */
		generator->send_target = EX_VAR(opline->result.var);
		ZVAL_NULL(generator->send_target);
	} else {
		generator->send_target = NULL;
	}

	/* We increment to the next op, so we are at the correct position when the
	 * generator is resumed. */
	CREX_VM_INC_OPCODE();

	/* The GOTO VM uses a local opline variable. We need to set the opline
	 * variable in execute_data so we don't resume at an old position. */
	SAVE_OPLINE();

	CREX_VM_RETURN();
}

CREX_VM_HANDLER(166, CREX_YIELD_FROM, CONST|TMPVAR|CV, ANY)
{
	USE_OPLINE
	crex_generator *generator = crex_get_running_generator(EXECUTE_DATA_C);
	zval *val;

	SAVE_OPLINE();
	val = GET_OP1_ZVAL_PTR(BP_VAR_R);

	if (UNEXPECTED(generator->flags & CREX_GENERATOR_FORCED_CLOSE)) {
		crex_throw_error(NULL, "Cannot use \"yield from\" in a force-closed generator");
		FREE_OP1();
		UNDEF_RESULT();
		HANDLE_EXCEPTION();
	}

CREX_VM_C_LABEL(yield_from_try_again):
	if (C_TYPE_P(val) == IS_ARRAY) {
		ZVAL_COPY_VALUE(&generator->values, val);
		if (C_OPT_REFCOUNTED_P(val)) {
			C_ADDREF_P(val);
		}
		C_FE_POS(generator->values) = 0;
		FREE_OP1();
	} else if (OP1_TYPE != IS_CONST && C_TYPE_P(val) == IS_OBJECT && C_OBJCE_P(val)->get_iterator) {
		crex_class_entry *ce = C_OBJCE_P(val);
		if (ce == crex_ce_generator) {
			crex_generator *new_gen = (crex_generator *) C_OBJ_P(val);

			C_ADDREF_P(val);
			FREE_OP1();

			if (UNEXPECTED(new_gen->execute_data == NULL)) {
				crex_throw_error(NULL, "Generator passed to yield from was aborted without proper return and is unable to continue");
				zval_ptr_dtor(val);
				UNDEF_RESULT();
				HANDLE_EXCEPTION();
			} else if (C_ISUNDEF(new_gen->retval)) {
				if (UNEXPECTED(crex_generator_get_current(new_gen) == generator)) {
					crex_throw_error(NULL, "Impossible to yield from the Generator being currently run");
					zval_ptr_dtor(val);
					UNDEF_RESULT();
					HANDLE_EXCEPTION();
				} else {
					crex_generator_yield_from(generator, new_gen);
				}
			} else {
				if (RETURN_VALUE_USED(opline)) {
					ZVAL_COPY(EX_VAR(opline->result.var), &new_gen->retval);
				}
				CREX_VM_NEXT_OPCODE();
			}
		} else {
			crex_object_iterator *iter = ce->get_iterator(ce, val, 0);
			FREE_OP1();

			if (UNEXPECTED(!iter) || UNEXPECTED(EG(exception))) {
				if (!EG(exception)) {
					crex_throw_error(NULL, "Object of type %s did not create an Iterator", ZSTR_VAL(ce->name));
				}
				UNDEF_RESULT();
				HANDLE_EXCEPTION();
			}

			iter->index = 0;
			if (iter->funcs->rewind) {
				iter->funcs->rewind(iter);
				if (UNEXPECTED(EG(exception) != NULL)) {
					OBJ_RELEASE(&iter->std);
					UNDEF_RESULT();
					HANDLE_EXCEPTION();
				}
			}

			ZVAL_OBJ(&generator->values, &iter->std);
		}
	} else if ((OP1_TYPE & (IS_VAR|IS_CV)) && C_TYPE_P(val) == IS_REFERENCE) {
		val = C_REFVAL_P(val);
		CREX_VM_C_GOTO(yield_from_try_again);
	} else {
		crex_throw_error(NULL, "Can use \"yield from\" only with arrays and Traversables");
		FREE_OP1();
		UNDEF_RESULT();
		HANDLE_EXCEPTION();
	}

	/* This is the default return value
	 * when the expression is a Generator, it will be overwritten in crex_generator_resume() */
	if (RETURN_VALUE_USED(opline)) {
		ZVAL_NULL(EX_VAR(opline->result.var));
	}

	/* This generator has no send target (though the generator we delegate to might have one) */
	generator->send_target = NULL;

	/* We increment to the next op, so we are at the correct position when the
	 * generator is resumed. */
	CREX_VM_INC_OPCODE();

	/* The GOTO VM uses a local opline variable. We need to set the opline
	 * variable in execute_data so we don't resume at an old position. */
	SAVE_OPLINE();

	CREX_VM_RETURN();
}

CREX_VM_HANDLER(159, CREX_DISCARD_EXCEPTION, ANY, ANY)
{
	USE_OPLINE
	zval *fast_call = EX_VAR(opline->op1.var);
	SAVE_OPLINE();

	/* cleanup incomplete RETURN statement */
	if (C_OPLINE_NUM_P(fast_call) != (uint32_t)-1
	 && (EX(func)->op_array.opcodes[C_OPLINE_NUM_P(fast_call)].op2_type & (IS_TMP_VAR | IS_VAR))) {
		zval *return_value = EX_VAR(EX(func)->op_array.opcodes[C_OPLINE_NUM_P(fast_call)].op2.var);

		zval_ptr_dtor(return_value);
	}

	/* cleanup delayed exception */
	if (C_OBJ_P(fast_call) != NULL) {
		/* discard the previously thrown exception */
		OBJ_RELEASE(C_OBJ_P(fast_call));
		C_OBJ_P(fast_call) = NULL;
	}

	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(162, CREX_FAST_CALL, JMP_ADDR, ANY)
{
	USE_OPLINE
	zval *fast_call = EX_VAR(opline->result.var);

	C_OBJ_P(fast_call) = NULL;
	/* set return address */
	C_OPLINE_NUM_P(fast_call) = opline - EX(func)->op_array.opcodes;
	CREX_VM_JMP_EX(OP_JMP_ADDR(opline, opline->op1), 0);
}

CREX_VM_HANDLER(163, CREX_FAST_RET, ANY, TRY_CATCH)
{
	USE_OPLINE
	zval *fast_call = EX_VAR(opline->op1.var);
	uint32_t current_try_catch_offset, current_op_num;

	if (C_OPLINE_NUM_P(fast_call) != (uint32_t)-1) {
		const crex_op *fast_ret = EX(func)->op_array.opcodes + C_OPLINE_NUM_P(fast_call);

		CREX_VM_JMP_EX(fast_ret + 1, 0);
	}

	/* special case for unhandled exceptions */
	EG(exception) = C_OBJ_P(fast_call);
	C_OBJ_P(fast_call) = NULL;
	current_try_catch_offset = opline->op2.num;
	current_op_num = opline - EX(func)->op_array.opcodes;
	CREX_VM_DISPATCH_TO_HELPER(crex_dispatch_try_catch_finally_helper, try_catch_offset, current_try_catch_offset, op_num, current_op_num);
}

CREX_VM_HOT_HANDLER(168, CREX_BIND_GLOBAL, CV, CONST, CACHE_SLOT)
{
	USE_OPLINE
	crex_string *varname;
	zval *value;
	zval *variable_ptr;
	uintptr_t idx;
	crex_reference *ref;

	CREX_VM_REPEATABLE_OPCODE

	varname = C_STR_P(GET_OP2_ZVAL_PTR(BP_VAR_R));

	/* We store "hash slot index" + 1 (NULL is a mark of uninitialized cache slot) */
	idx = (uintptr_t)CACHED_PTR(opline->extended_value) - 1;
	if (EXPECTED(idx < EG(symbol_table).nNumUsed * sizeof(Bucket))) {
		Bucket *p = (Bucket*)((char*)EG(symbol_table).arData + idx);

		if (EXPECTED(p->key == varname) ||
		    (EXPECTED(p->h == ZSTR_H(varname)) &&
		     EXPECTED(p->key != NULL) &&
		     EXPECTED(crex_string_equal_content(p->key, varname)))) {

			value = (zval*)p; /* value = &p->val; */
			CREX_VM_C_GOTO(check_indirect);
		}
	}

	value = crex_hash_find_known_hash(&EG(symbol_table), varname);
	if (UNEXPECTED(value == NULL)) {
		value = crex_hash_add_new(&EG(symbol_table), varname, &EG(uninitialized_zval));
		idx = (char*)value - (char*)EG(symbol_table).arData;
		/* Store "hash slot index" + 1 (NULL is a mark of uninitialized cache slot) */
		CACHE_PTR(opline->extended_value, (void*)(idx + 1));
	} else {
		idx = (char*)value - (char*)EG(symbol_table).arData;
		/* Store "hash slot index" + 1 (NULL is a mark of uninitialized cache slot) */
		CACHE_PTR(opline->extended_value, (void*)(idx + 1));
CREX_VM_C_LABEL(check_indirect):
		/* GLOBAL variable may be an INDIRECT pointer to CV */
		if (UNEXPECTED(C_TYPE_P(value) == IS_INDIRECT)) {
			value = C_INDIRECT_P(value);
			if (UNEXPECTED(C_TYPE_P(value) == IS_UNDEF)) {
				ZVAL_NULL(value);
			}
		}
	}

	if (UNEXPECTED(!C_ISREF_P(value))) {
		ZVAL_MAKE_REF_EX(value, 2);
		ref = C_REF_P(value);
	} else {
		ref = C_REF_P(value);
		GC_ADDREF(ref);
	}

	variable_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_W);

	if (UNEXPECTED(C_REFCOUNTED_P(variable_ptr))) {
		crex_refcounted *garbage = C_COUNTED_P(variable_ptr);

		ZVAL_REF(variable_ptr, ref);
		SAVE_OPLINE();
		if (GC_DELREF(garbage) == 0) {
			rc_dtor_func(garbage);
			if (UNEXPECTED(EG(exception))) {
				ZVAL_NULL(variable_ptr);
				HANDLE_EXCEPTION();
			}
		} else {
			gc_check_possible_root(garbage);
		}
	} else {
		ZVAL_REF(variable_ptr, ref);
	}

	CREX_VM_REPEAT_OPCODE(CREX_BIND_GLOBAL);
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_COLD_CONST_HANDLER(121, CREX_STRLEN, CONST|TMPVAR|CV, ANY)
{
	USE_OPLINE
	zval *value;

	value = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (EXPECTED(C_TYPE_P(value) == IS_STRING)) {
		ZVAL_LONG(EX_VAR(opline->result.var), C_STRLEN_P(value));
		if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
			zval_ptr_dtor_str(value);
		}
		CREX_VM_NEXT_OPCODE();
	} else {
		bool strict;

		if ((OP1_TYPE & (IS_VAR|IS_CV)) && C_TYPE_P(value) == IS_REFERENCE) {
			value = C_REFVAL_P(value);
			if (EXPECTED(C_TYPE_P(value) == IS_STRING)) {
				ZVAL_LONG(EX_VAR(opline->result.var), C_STRLEN_P(value));
				FREE_OP1();
				CREX_VM_NEXT_OPCODE();
			}
		}

		SAVE_OPLINE();
		if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(value) == IS_UNDEF)) {
			value = ZVAL_UNDEFINED_OP1();
		}
		strict = EX_USES_STRICT_TYPES();
		do {
			if (EXPECTED(!strict)) {
				crex_string *str;
				zval tmp;

				if (UNEXPECTED(C_TYPE_P(value) == IS_NULL)) {
					crex_error(E_DEPRECATED,
						"strlen(): Passing null to parameter #1 ($string) of type string is deprecated");
					ZVAL_LONG(EX_VAR(opline->result.var), 0);
					if (UNEXPECTED(EG(exception))) {
						HANDLE_EXCEPTION();
					}
					break;
				}

				ZVAL_COPY(&tmp, value);
				if (crex_parse_arg_str_weak(&tmp, &str, 1)) {
					ZVAL_LONG(EX_VAR(opline->result.var), ZSTR_LEN(str));
					zval_ptr_dtor(&tmp);
					break;
				}
				zval_ptr_dtor(&tmp);
			}
			if (!EG(exception)) {
				crex_type_error("strlen(): Argument #1 ($string) must be of type string, %s given", crex_zval_value_name(value));
			}
			ZVAL_UNDEF(EX_VAR(opline->result.var));
		} while (0);
	}
	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HOT_NOCONST_HANDLER(123, CREX_TYPE_CHECK, CONST|TMPVAR|CV, ANY, TYPE_MASK)
{
	USE_OPLINE
	zval *value;
	int result = 0;

	value = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	if ((opline->extended_value >> (uint32_t)C_TYPE_P(value)) & 1) {
CREX_VM_C_LABEL(type_check_resource):
		if (opline->extended_value != MAY_BE_RESOURCE
		 || EXPECTED(NULL != crex_rsrc_list_get_rsrc_type(C_RES_P(value)))) {
			result = 1;
		}
	} else if ((OP1_TYPE & (IS_CV|IS_VAR)) && C_ISREF_P(value)) {
		value = C_REFVAL_P(value);
		if ((opline->extended_value >> (uint32_t)C_TYPE_P(value)) & 1) {
			CREX_VM_C_GOTO(type_check_resource);
		}
	} else if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(value) == IS_UNDEF)) {
		result = ((1 << IS_NULL) & opline->extended_value) != 0;
		SAVE_OPLINE();
		ZVAL_UNDEFINED_OP1();
		if (UNEXPECTED(EG(exception))) {
			ZVAL_UNDEF(EX_VAR(opline->result.var));
			HANDLE_EXCEPTION();
		}
	}
	if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
		SAVE_OPLINE();
		FREE_OP1();
		CREX_VM_SMART_BRANCH(result, 1);
	} else {
		CREX_VM_SMART_BRANCH(result, 0);
	}
}

CREX_VM_HOT_HANDLER(122, CREX_DEFINED, CONST, ANY, CACHE_SLOT)
{
	USE_OPLINE
	crex_constant *c;

	c = CACHED_PTR(opline->extended_value);
	if (EXPECTED(c != NULL)) {
		if (!IS_SPECIAL_CACHE_VAL(c)) {
CREX_VM_C_LABEL(defined_true):
			CREX_VM_SMART_BRANCH_TRUE();
		} else if (EXPECTED(crex_hash_num_elements(EG(crex_constants)) == DECODE_SPECIAL_CACHE_NUM(c))) {
CREX_VM_C_LABEL(defined_false):
			CREX_VM_SMART_BRANCH_FALSE();
		}
	}
	if (crex_quick_check_constant(RT_CONSTANT(opline, opline->op1) OPLINE_CC EXECUTE_DATA_CC) != SUCCESS) {
		CACHE_PTR(opline->extended_value, ENCODE_SPECIAL_CACHE_NUM(crex_hash_num_elements(EG(crex_constants))));
		CREX_VM_C_GOTO(defined_false);
	} else {
		CREX_VM_C_GOTO(defined_true);
	}
}

CREX_VM_HANDLER(151, CREX_ASSERT_CHECK, ANY, JMP_ADDR)
{
	USE_OPLINE

	if (EG(assertions) <= 0) {
		crex_op *target = OP_JMP_ADDR(opline, opline->op2);
		if (RETURN_VALUE_USED(opline)) {
			ZVAL_TRUE(EX_VAR(opline->result.var));
		}
		CREX_VM_JMP_EX(target, 0);
	} else {
		CREX_VM_NEXT_OPCODE();
	}
}

CREX_VM_HANDLER(157, CREX_FETCH_CLASS_NAME, CV|TMPVAR|UNUSED|CLASS_FETCH, ANY)
{
	uint32_t fetch_type;
	crex_class_entry *called_scope, *scope;
	USE_OPLINE

	if (OP1_TYPE != IS_UNUSED) {
		SAVE_OPLINE();
		zval *op = GET_OP1_ZVAL_PTR(BP_VAR_R);
		if (UNEXPECTED(C_TYPE_P(op) != IS_OBJECT)) {
			ZVAL_DEREF(op);
			if (C_TYPE_P(op) != IS_OBJECT) {
				crex_type_error("Cannot use \"::class\" on %s", crex_zval_value_name(op));
				ZVAL_UNDEF(EX_VAR(opline->result.var));
				FREE_OP1();
				HANDLE_EXCEPTION();
			}
		}

		ZVAL_STR_COPY(EX_VAR(opline->result.var), C_OBJCE_P(op)->name);
		FREE_OP1();
		CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
	}

	fetch_type = opline->op1.num;
	scope = EX(func)->op_array.scope;
	if (UNEXPECTED(scope == NULL)) {
		SAVE_OPLINE();
		crex_throw_error(NULL, "Cannot use \"%s\" in the global scope",
			fetch_type == CREX_FETCH_CLASS_SELF ? "self" :
			fetch_type == CREX_FETCH_CLASS_PARENT ? "parent" : "static");
		ZVAL_UNDEF(EX_VAR(opline->result.var));
		HANDLE_EXCEPTION();
	}

	switch (fetch_type) {
		case CREX_FETCH_CLASS_SELF:
			ZVAL_STR_COPY(EX_VAR(opline->result.var), scope->name);
			break;
		case CREX_FETCH_CLASS_PARENT:
			if (UNEXPECTED(scope->parent == NULL)) {
				SAVE_OPLINE();
				crex_throw_error(NULL,
					"Cannot use \"parent\" when current class scope has no parent");
				ZVAL_UNDEF(EX_VAR(opline->result.var));
				HANDLE_EXCEPTION();
			}
			ZVAL_STR_COPY(EX_VAR(opline->result.var), scope->parent->name);
			break;
		case CREX_FETCH_CLASS_STATIC:
			if (C_TYPE(EX(This)) == IS_OBJECT) {
				called_scope = C_OBJCE(EX(This));
			} else {
				called_scope = C_CE(EX(This));
			}
			ZVAL_STR_COPY(EX_VAR(opline->result.var), called_scope->name);
			break;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(158, CREX_CALL_TRAMPOLINE, ANY, ANY, SPEC(OBSERVER))
{
	crex_array *args = NULL;
	crex_function *fbc = EX(func);
	zval *ret = EX(return_value);
	uint32_t call_info = EX_CALL_INFO() & (CREX_CALL_NESTED | CREX_CALL_TOP | CREX_CALL_RELEASE_THIS | CREX_CALL_HAS_EXTRA_NAMED_PARAMS);
	uint32_t num_args = EX_NUM_ARGS();
	crex_execute_data *call;

	SAVE_OPLINE();

	if (num_args) {
		zval *p = CREX_CALL_ARG(execute_data, 1);
		zval *end = p + num_args;

		args = crex_new_array(num_args);
		crex_hash_real_init_packed(args);
		CREX_HASH_FILL_PACKED(args) {
			do {
				CREX_HASH_FILL_ADD(p);
				p++;
			} while (p != end);
		} CREX_HASH_FILL_END();
	}

	call = execute_data;
	execute_data = EG(current_execute_data) = EX(prev_execute_data);

	call->func = (fbc->op_array.fn_flags & CREX_ACC_STATIC) ? fbc->op_array.scope->__callstatic : fbc->op_array.scope->__call;
	CREX_ASSERT(crex_vm_calc_used_stack(2, call->func) <= (size_t)(((char*)EG(vm_stack_end)) - (char*)call));
	CREX_CALL_NUM_ARGS(call) = 2;

	ZVAL_STR(CREX_CALL_ARG(call, 1), fbc->common.function_name);

	zval *call_args = CREX_CALL_ARG(call, 2);
	if (args) {
		ZVAL_ARR(call_args, args);
	} else {
		ZVAL_EMPTY_ARRAY(call_args);
	}
	if (UNEXPECTED(call_info & CREX_CALL_HAS_EXTRA_NAMED_PARAMS)) {
		if (crex_hash_num_elements(C_ARRVAL_P(call_args)) == 0) {
			GC_ADDREF(call->extra_named_params);
			ZVAL_ARR(call_args, call->extra_named_params);
		} else {
			SEPARATE_ARRAY(call_args);
			crex_hash_copy(C_ARRVAL_P(call_args), call->extra_named_params, zval_add_ref);
		}
	}
	crex_free_trampoline(fbc);
	fbc = call->func;

	if (EXPECTED(fbc->type == CREX_USER_FUNCTION)) {
		if (UNEXPECTED(!RUN_TIME_CACHE(&fbc->op_array))) {
			init_func_run_time_cache(&fbc->op_array);
		}
		execute_data = call;
		i_init_func_execute_data(&fbc->op_array, ret, 0 EXECUTE_DATA_CC);
		if (EXPECTED(crex_execute_ex == execute_ex)) {
			LOAD_OPLINE_EX();
			CREX_OBSERVER_SAVE_OPLINE();
			CREX_OBSERVER_FCALL_BEGIN(execute_data);
			CREX_VM_ENTER_EX();
		} else {
			SAVE_OPLINE_EX();
			CREX_OBSERVER_FCALL_BEGIN(execute_data);
			execute_data = EX(prev_execute_data);
			if (execute_data) {
				LOAD_OPLINE();
			}
			CREX_ADD_CALL_FLAG(call, CREX_CALL_TOP);
			crex_execute_ex(call);
		}
	} else {
		zval retval;

		CREX_ASSERT(fbc->type == CREX_INTERNAL_FUNCTION);

		EG(current_execute_data) = call;

#if CREX_DEBUG
		bool should_throw = crex_internal_call_should_throw(fbc, call);
#endif

		if (ret == NULL) {
			ret = &retval;
		}

		ZVAL_NULL(ret);
		CREX_OBSERVER_FCALL_BEGIN(call);
		if (!crex_execute_internal) {
			/* saves one function call if crex_execute_internal is not used */
			fbc->internal_function.handler(call, ret);
		} else {
			crex_execute_internal(call, ret);
		}

#if CREX_DEBUG
		if (!EG(exception) && call->func) {
			if (should_throw) {
				crex_internal_call_arginfo_violation(call->func);
			}
			CREX_ASSERT(!(call->func->common.fn_flags & CREX_ACC_HAS_RETURN_TYPE) ||
				crex_verify_internal_return_type(call->func, ret));
			CREX_ASSERT((call->func->common.fn_flags & CREX_ACC_RETURN_REFERENCE)
				? C_ISREF_P(ret) : !C_ISREF_P(ret));
			crex_verify_internal_func_info(call->func, ret);
		}
#endif
		CREX_OBSERVER_FCALL_END(call, EG(exception) ? NULL : ret);

		EG(current_execute_data) = call->prev_execute_data;

		crex_vm_stack_free_args(call);
		if (ret == &retval) {
			zval_ptr_dtor(ret);
		}
	}

	execute_data = EG(current_execute_data);

	if (!execute_data || !EX(func) || !CREX_USER_CODE(EX(func)->type) || (call_info & CREX_CALL_TOP)) {
		CREX_VM_RETURN();
	}

	if (UNEXPECTED(call_info & CREX_CALL_RELEASE_THIS)) {
		crex_object *object = C_OBJ(call->This);
		OBJ_RELEASE(object);
	}
	crex_vm_stack_free_call_frame(call);

	if (UNEXPECTED(EG(exception) != NULL)) {
		crex_rethrow_exception(execute_data);
		HANDLE_EXCEPTION_LEAVE();
	}

	LOAD_OPLINE();
	CREX_VM_INC_OPCODE();
	CREX_VM_LEAVE();
}

CREX_VM_HANDLER(182, CREX_BIND_LEXICAL, TMP, CV, REF)
{
	USE_OPLINE
	zval *closure, *var;

	closure = GET_OP1_ZVAL_PTR(BP_VAR_R);
	if (opline->extended_value & CREX_BIND_REF) {
		/* By-ref binding */
		var = GET_OP2_ZVAL_PTR(BP_VAR_W);
		if (C_ISREF_P(var)) {
			C_ADDREF_P(var);
		} else {
			ZVAL_MAKE_REF_EX(var, 2);
		}
	} else {
		var = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
		if (UNEXPECTED(C_ISUNDEF_P(var)) && !(opline->extended_value & CREX_BIND_IMPLICIT)) {
			SAVE_OPLINE();
			var = ZVAL_UNDEFINED_OP2();
			if (UNEXPECTED(EG(exception))) {
				HANDLE_EXCEPTION();
			}
		}
		ZVAL_DEREF(var);
		C_TRY_ADDREF_P(var);
	}

	crex_closure_bind_var_ex(closure,
		(opline->extended_value & ~(CREX_BIND_REF|CREX_BIND_IMPLICIT)), var);
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(183, CREX_BIND_STATIC, CV, ANY, REF)
{
	USE_OPLINE
	HashTable *ht;
	zval *value;
	zval *variable_ptr;

	variable_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_W);

	ht = CREX_MAP_PTR_GET(EX(func)->op_array.static_variables_ptr);
	if (!ht) {
		ht = crex_array_dup(EX(func)->op_array.static_variables);
		CREX_MAP_PTR_SET(EX(func)->op_array.static_variables_ptr, ht);
	}
	CREX_ASSERT(GC_REFCOUNT(ht) == 1);

	value = (zval*)((char*)ht->arData + (opline->extended_value & ~(CREX_BIND_REF|CREX_BIND_IMPLICIT|CREX_BIND_EXPLICIT)));

	SAVE_OPLINE();
	if (opline->extended_value & CREX_BIND_REF) {
		i_zval_ptr_dtor(variable_ptr);
		if (UNEXPECTED(!C_ISREF_P(value))) {
			crex_reference *ref = (crex_reference*)emalloc(sizeof(crex_reference));
			GC_SET_REFCOUNT(ref, 2);
			GC_TYPE_INFO(ref) = GC_REFERENCE;
			if (OP2_TYPE == IS_UNUSED) {
				ZVAL_COPY_VALUE(&ref->val, value);
			} else {
				CREX_ASSERT(!C_REFCOUNTED_P(value));
				ZVAL_COPY(&ref->val, GET_OP2_ZVAL_PTR_DEREF(BP_VAR_R));
				FREE_OP2();
			}
			ref->sources.ptr = NULL;
			C_REF_P(value) = ref;
			C_TYPE_INFO_P(value) = IS_REFERENCE_EX;
			ZVAL_REF(variable_ptr, ref);
		} else {
			C_ADDREF_P(value);
			ZVAL_REF(variable_ptr, C_REF_P(value));
			if (OP2_TYPE != IS_UNUSED) {
				FREE_OP2();
			}
		}
	} else {
		i_zval_ptr_dtor(variable_ptr);
		ZVAL_COPY(variable_ptr, value);
	}

	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(203, CREX_BIND_INIT_STATIC_OR_JMP, CV, JMP_ADDR)
{
	USE_OPLINE
	HashTable *ht;
	zval *value;
	zval *variable_ptr;

	variable_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_W);

	ht = CREX_MAP_PTR_GET(EX(func)->op_array.static_variables_ptr);
	if (!ht) {
		CREX_VM_NEXT_OPCODE();
	}
	CREX_ASSERT(GC_REFCOUNT(ht) == 1);

	value = (zval*)((char*)ht->arData + opline->extended_value);
	if (C_TYPE_EXTRA_P(value) & IS_STATIC_VAR_UNINITIALIZED) {
		CREX_VM_NEXT_OPCODE();
	} else {
		SAVE_OPLINE();
		zval_ptr_dtor(variable_ptr);
		CREX_ASSERT(C_TYPE_P(value) == IS_REFERENCE);
		C_ADDREF_P(value);
		ZVAL_REF(variable_ptr, C_REF_P(value));
		CREX_VM_JMP_EX(OP_JMP_ADDR(opline, opline->op2), 1);
	}
}

CREX_VM_HOT_HANDLER(184, CREX_FETCH_THIS, UNUSED, UNUSED)
{
	USE_OPLINE

	if (EXPECTED(C_TYPE(EX(This)) == IS_OBJECT)) {
		zval *result = EX_VAR(opline->result.var);

		ZVAL_OBJ(result, C_OBJ(EX(This)));
		C_ADDREF_P(result);
		CREX_VM_NEXT_OPCODE();
	} else {
		CREX_VM_DISPATCH_TO_HELPER(crex_this_not_in_object_context_helper);
	}
}

CREX_VM_HANDLER(200, CREX_FETCH_GLOBALS, UNUSED, UNUSED)
{
	USE_OPLINE

	/* For symbol tables we need to deal with exactly the same problems as for property tables. */
	ZVAL_ARR(EX_VAR(opline->result.var),
		crex_proptable_to_symtable(&EG(symbol_table), /* always_duplicate */ 1));
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(186, CREX_ISSET_ISEMPTY_THIS, UNUSED, UNUSED)
{
	USE_OPLINE

	ZVAL_BOOL(EX_VAR(opline->result.var),
		(opline->extended_value & CREX_ISEMPTY) ^
		 (C_TYPE(EX(This)) == IS_OBJECT));
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(49, CREX_CHECK_VAR, CV, UNUSED)
{
	USE_OPLINE
	zval *op1 = EX_VAR(opline->op1.var);

	if (UNEXPECTED(C_TYPE_INFO_P(op1) == IS_UNDEF)) {
		SAVE_OPLINE();
		ZVAL_UNDEFINED_OP1();
		CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(140, CREX_MAKE_REF, VAR|CV, UNUSED)
{
	USE_OPLINE
	zval *op1 = EX_VAR(opline->op1.var);

	if (OP1_TYPE == IS_CV) {
		if (UNEXPECTED(C_TYPE_P(op1) == IS_UNDEF)) {
			ZVAL_NEW_EMPTY_REF(op1);
			C_SET_REFCOUNT_P(op1, 2);
			ZVAL_NULL(C_REFVAL_P(op1));
			ZVAL_REF(EX_VAR(opline->result.var), C_REF_P(op1));
		} else {
			if (C_ISREF_P(op1)) {
				C_ADDREF_P(op1);
			} else {
				ZVAL_MAKE_REF_EX(op1, 2);
			}
			ZVAL_REF(EX_VAR(opline->result.var), C_REF_P(op1));
		}
	} else if (EXPECTED(C_TYPE_P(op1) == IS_INDIRECT)) {
		op1 = C_INDIRECT_P(op1);
		if (EXPECTED(!C_ISREF_P(op1))) {
			ZVAL_MAKE_REF_EX(op1, 2);
		} else {
			GC_ADDREF(C_REF_P(op1));
		}
		ZVAL_REF(EX_VAR(opline->result.var), C_REF_P(op1));
	} else {
		ZVAL_COPY_VALUE(EX_VAR(opline->result.var), op1);
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_COLD_CONSTCONST_HANDLER(187, CREX_SWITCH_LONG, CONST|TMPVARCV, CONST, JMP_ADDR)
{
	USE_OPLINE
	zval *op, *jump_zv;
	HashTable *jumptable;

	op = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);

	if (C_TYPE_P(op) != IS_LONG) {
		ZVAL_DEREF(op);
		if (C_TYPE_P(op) != IS_LONG) {
			/* Wrong type, fall back to CREX_CASE chain */
			CREX_VM_NEXT_OPCODE();
		}
	}

	jumptable = C_ARRVAL_P(GET_OP2_ZVAL_PTR(BP_VAR_R));
	jump_zv = crex_hash_index_find(jumptable, C_LVAL_P(op));
	if (jump_zv != NULL) {
		CREX_VM_SET_RELATIVE_OPCODE(opline, C_LVAL_P(jump_zv));
		CREX_VM_CONTINUE();
	} else {
		/* default */
		CREX_VM_SET_RELATIVE_OPCODE(opline, opline->extended_value);
		CREX_VM_CONTINUE();
	}
}

CREX_VM_COLD_CONSTCONST_HANDLER(188, CREX_SWITCH_STRING, CONST|TMPVARCV, CONST, JMP_ADDR)
{
	USE_OPLINE
	zval *op, *jump_zv;
	HashTable *jumptable;

	op = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);

	if (C_TYPE_P(op) != IS_STRING) {
		if (OP1_TYPE == IS_CONST) {
			/* Wrong type, fall back to CREX_CASE chain */
			CREX_VM_NEXT_OPCODE();
		} else {
			ZVAL_DEREF(op);
			if (C_TYPE_P(op) != IS_STRING) {
				/* Wrong type, fall back to CREX_CASE chain */
				CREX_VM_NEXT_OPCODE();
			}
		}
	}

	jumptable = C_ARRVAL_P(GET_OP2_ZVAL_PTR(BP_VAR_R));
	jump_zv = crex_hash_find_ex(jumptable, C_STR_P(op), OP1_TYPE == IS_CONST);
	if (jump_zv != NULL) {
		CREX_VM_SET_RELATIVE_OPCODE(opline, C_LVAL_P(jump_zv));
		CREX_VM_CONTINUE();
	} else {
		/* default */
		CREX_VM_SET_RELATIVE_OPCODE(opline, opline->extended_value);
		CREX_VM_CONTINUE();
	}
}

CREX_VM_COLD_CONSTCONST_HANDLER(195, CREX_MATCH, CONST|TMPVARCV, CONST, JMP_ADDR)
{
	USE_OPLINE
	zval *op, *jump_zv;
	HashTable *jumptable;

	op = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	jumptable = C_ARRVAL_P(GET_OP2_ZVAL_PTR(BP_VAR_R));

CREX_VM_C_LABEL(match_try_again):
	if (C_TYPE_P(op) == IS_LONG) {
		jump_zv = crex_hash_index_find(jumptable, C_LVAL_P(op));
	} else if (C_TYPE_P(op) == IS_STRING) {
		jump_zv = crex_hash_find_ex(jumptable, C_STR_P(op), OP1_TYPE == IS_CONST);
	} else if (C_TYPE_P(op) == IS_REFERENCE) {
		op = C_REFVAL_P(op);
		CREX_VM_C_GOTO(match_try_again);
	} else {
		if (UNEXPECTED((OP1_TYPE & IS_CV) && C_TYPE_P(op) == IS_UNDEF)) {
			SAVE_OPLINE();
			op = ZVAL_UNDEFINED_OP1();
			if (UNEXPECTED(EG(exception))) {
				HANDLE_EXCEPTION();
			}
			CREX_VM_C_GOTO(match_try_again);
		}

		CREX_VM_C_GOTO(default_branch);
	}

	if (jump_zv != NULL) {
		CREX_VM_SET_RELATIVE_OPCODE(opline, C_LVAL_P(jump_zv));
		CREX_VM_CONTINUE();
	} else {
CREX_VM_C_LABEL(default_branch):
		/* default */
		CREX_VM_SET_RELATIVE_OPCODE(opline, opline->extended_value);
		CREX_VM_CONTINUE();
	}
}

CREX_VM_COLD_CONST_HANDLER(197, CREX_MATCH_ERROR, CONST|TMPVARCV, UNUSED)
{
	USE_OPLINE
	zval *op;

	SAVE_OPLINE();
	op = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	crex_match_unhandled_error(op);
	HANDLE_EXCEPTION();
}

CREX_VM_COLD_CONSTCONST_HANDLER(189, CREX_IN_ARRAY, CONST|TMP|VAR|CV, CONST, NUM)
{
	USE_OPLINE
	zval *op1;
	HashTable *ht = C_ARRVAL_P(RT_CONSTANT(opline, opline->op2));
	zval *result;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (EXPECTED(C_TYPE_P(op1) == IS_STRING)) {
		result = crex_hash_find_ex(ht, C_STR_P(op1), OP1_TYPE == IS_CONST);
		if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
			zval_ptr_dtor_str(op1);
		}
		CREX_VM_SMART_BRANCH(result, 0);
	}

	if (opline->extended_value) {
		if (EXPECTED(C_TYPE_P(op1) == IS_LONG)) {
			result = crex_hash_index_find(ht, C_LVAL_P(op1));
			CREX_VM_SMART_BRANCH(result, 0);
		}
		SAVE_OPLINE();
		if ((OP1_TYPE & (IS_VAR|IS_CV)) && C_TYPE_P(op1) == IS_REFERENCE) {
			op1 = C_REFVAL_P(op1);
			if (EXPECTED(C_TYPE_P(op1) == IS_STRING)) {
				result = crex_hash_find(ht, C_STR_P(op1));
				FREE_OP1();
				CREX_VM_SMART_BRANCH(result, 0);
			} else if (EXPECTED(C_TYPE_P(op1) == IS_LONG)) {
				result = crex_hash_index_find(ht, C_LVAL_P(op1));
				FREE_OP1();
				CREX_VM_SMART_BRANCH(result, 0);
			}
		} else if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(op1) == IS_UNDEF)) {
			ZVAL_UNDEFINED_OP1();
		}
	} else if (C_TYPE_P(op1) <= IS_FALSE) {
		if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(op1) == IS_UNDEF)) {
			SAVE_OPLINE();
			ZVAL_UNDEFINED_OP1();
			if (UNEXPECTED(EG(exception) != NULL)) {
				HANDLE_EXCEPTION();
			}
		}
		result = crex_hash_find_known_hash(ht, ZSTR_EMPTY_ALLOC());
		CREX_VM_SMART_BRANCH(result, 0);
	} else {
		crex_string *key;
		zval key_tmp;

		if ((OP1_TYPE & (IS_VAR|IS_CV)) && C_TYPE_P(op1) == IS_REFERENCE) {
			op1 = C_REFVAL_P(op1);
			if (EXPECTED(C_TYPE_P(op1) == IS_STRING)) {
				result = crex_hash_find(ht, C_STR_P(op1));
				FREE_OP1();
				CREX_VM_SMART_BRANCH(result, 0);
			}
		}

		SAVE_OPLINE();
		CREX_HASH_MAP_FOREACH_STR_KEY(ht, key) {
			ZVAL_STR(&key_tmp, key);
			if (crex_compare(op1, &key_tmp) == 0) {
				FREE_OP1();
				CREX_VM_SMART_BRANCH(1, 1);
			}
		} CREX_HASH_FOREACH_END();
	}
	FREE_OP1();
	CREX_VM_SMART_BRANCH(0, 1);
}

CREX_VM_COLD_CONST_HANDLER(190, CREX_COUNT, CONST|TMPVAR|CV, UNUSED)
{
	USE_OPLINE
	zval *op1;
	crex_long count;

	SAVE_OPLINE();
	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);

	while (1) {
		if (C_TYPE_P(op1) == IS_ARRAY) {
			count = crex_hash_num_elements(C_ARRVAL_P(op1));
			break;
		} else if (C_TYPE_P(op1) == IS_OBJECT) {
			crex_object *zobj = C_OBJ_P(op1);

			/* first, we check if the handler is defined */
			if (zobj->handlers->count_elements) {
				if (SUCCESS == zobj->handlers->count_elements(zobj, &count)) {
					break;
				}
				if (UNEXPECTED(EG(exception))) {
					count = 0;
					break;
				}
			}

			/* if not and the object implements Countable we call its count() method */
			if (crex_class_implements_interface(zobj->ce, crex_ce_countable)) {
				zval retval;

				crex_function *count_fn = crex_hash_find_ptr(&zobj->ce->function_table, ZSTR_KNOWN(CREX_STR_COUNT));
				crex_call_known_instance_method_with_0_params(count_fn, zobj, &retval);
				count = zval_get_long(&retval);
				zval_ptr_dtor(&retval);
				break;
			}

			/* If There's no handler and it doesn't implement Countable then emit a TypeError */
		} else if ((OP1_TYPE & (IS_VAR|IS_CV)) != 0 && C_TYPE_P(op1) == IS_REFERENCE) {
			op1 = C_REFVAL_P(op1);
			continue;
		} else if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(op1) == IS_UNDEF)) {
			ZVAL_UNDEFINED_OP1();
		}
		count = 0;
		crex_type_error("%s(): Argument #1 ($value) must be of type Countable|array, %s given", opline->extended_value ? "sizeof" : "count", crex_zval_value_name(op1));
		break;
	}

	ZVAL_LONG(EX_VAR(opline->result.var), count);
	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_TYPE_SPEC_HANDLER(CREX_COUNT, (op1_info & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_REF)) == MAY_BE_ARRAY, CREX_COUNT_ARRAY, CV|TMPVAR, UNUSED)
{
	USE_OPLINE
	crex_array *ht = C_ARRVAL_P(GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R));
	ZVAL_LONG(EX_VAR(opline->result.var), crex_hash_num_elements(ht));
	if (OP1_TYPE & (IS_TMP_VAR|IS_VAR) && !(GC_FLAGS(ht) & IS_ARRAY_IMMUTABLE) && !GC_DELREF(ht)) {
		SAVE_OPLINE();
		crex_array_destroy(ht);
		if (EG(exception)) {
			HANDLE_EXCEPTION();
		}
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_COLD_CONST_HANDLER(191, CREX_GET_CLASS, UNUSED|CONST|TMPVAR|CV, UNUSED)
{
	USE_OPLINE

	if (OP1_TYPE == IS_UNUSED) {
		SAVE_OPLINE();
		if (UNEXPECTED(!EX(func)->common.scope)) {
			crex_throw_error(NULL, "get_class() without arguments must be called from within a class");
			ZVAL_UNDEF(EX_VAR(opline->result.var));
			HANDLE_EXCEPTION();
		} else {
			crex_error(E_DEPRECATED, "Calling get_class() without arguments is deprecated");
			ZVAL_STR_COPY(EX_VAR(opline->result.var), EX(func)->common.scope->name);
			if (UNEXPECTED(EG(exception))) {
				HANDLE_EXCEPTION();
			}
			CREX_VM_NEXT_OPCODE();
		}
	} else {
		zval *op1;

		SAVE_OPLINE();
		op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
		while (1) {
			if (C_TYPE_P(op1) == IS_OBJECT) {
				ZVAL_STR_COPY(EX_VAR(opline->result.var), C_OBJCE_P(op1)->name);
			} else if ((OP1_TYPE & (IS_VAR|IS_CV)) != 0 && C_TYPE_P(op1) == IS_REFERENCE) {
				op1 = C_REFVAL_P(op1);
				continue;
			} else {
				if (OP1_TYPE == IS_CV && UNEXPECTED(C_TYPE_P(op1) == IS_UNDEF)) {
					ZVAL_UNDEFINED_OP1();
				}
				crex_type_error("get_class(): Argument #1 ($object) must be of type object, %s given", crex_zval_value_name(op1));
				ZVAL_UNDEF(EX_VAR(opline->result.var));
			}
			break;
		}
		FREE_OP1();
		CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
	}
}

CREX_VM_HANDLER(192, CREX_GET_CALLED_CLASS, UNUSED, UNUSED)
{
	USE_OPLINE

	if (C_TYPE(EX(This)) == IS_OBJECT) {
		ZVAL_STR_COPY(EX_VAR(opline->result.var), C_OBJCE(EX(This))->name);
	} else if (C_CE(EX(This))) {
		ZVAL_STR_COPY(EX_VAR(opline->result.var), C_CE(EX(This))->name);
	} else {
		CREX_ASSERT(!EX(func)->common.scope);
		SAVE_OPLINE();
		crex_throw_error(NULL, "get_called_class() must be called from within a class");
		ZVAL_UNDEF(EX_VAR(opline->result.var));
		HANDLE_EXCEPTION();
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_COLD_CONST_HANDLER(193, CREX_GET_TYPE, CONST|TMP|VAR|CV, UNUSED)
{
	USE_OPLINE
	zval *op1;
	crex_string *type;

	SAVE_OPLINE();
	op1 = GET_OP1_ZVAL_PTR_DEREF(BP_VAR_R);
	type = crex_zval_get_legacy_type(op1);
	if (EXPECTED(type)) {
		ZVAL_INTERNED_STR(EX_VAR(opline->result.var), type);
	} else {
		ZVAL_STRING(EX_VAR(opline->result.var), "unknown type");
	}
	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HANDLER(171, CREX_FUNC_NUM_ARGS, UNUSED, UNUSED)
{
	USE_OPLINE

	ZVAL_LONG(EX_VAR(opline->result.var), EX_NUM_ARGS());
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(172, CREX_FUNC_GET_ARGS, UNUSED|CONST, UNUSED)
{
	USE_OPLINE
	crex_array *ht;
	uint32_t arg_count, result_size, skip;

	arg_count = EX_NUM_ARGS();
	if (OP1_TYPE == IS_CONST) {
		skip = C_LVAL_P(RT_CONSTANT(opline, opline->op1));
		if (arg_count < skip) {
			result_size = 0;
		} else {
			result_size = arg_count - skip;
		}
	} else {
		skip = 0;
		result_size = arg_count;
	}

	if (result_size) {
		uint32_t first_extra_arg = EX(func)->op_array.num_args;

		ht = crex_new_array(result_size);
		ZVAL_ARR(EX_VAR(opline->result.var), ht);
		crex_hash_real_init_packed(ht);
		CREX_HASH_FILL_PACKED(ht) {
			zval *p, *q;
			uint32_t i = skip;
			p = EX_VAR_NUM(i);
			if (arg_count > first_extra_arg) {
				while (i < first_extra_arg) {
					q = p;
					if (EXPECTED(C_TYPE_INFO_P(q) != IS_UNDEF)) {
						ZVAL_DEREF(q);
						if (C_OPT_REFCOUNTED_P(q)) {
							C_ADDREF_P(q);
						}
						CREX_HASH_FILL_SET(q);
					} else {
						CREX_HASH_FILL_SET_NULL();
					}
					CREX_HASH_FILL_NEXT();
					p++;
					i++;
				}
				if (skip < first_extra_arg) {
					skip = 0;
				} else {
					skip -= first_extra_arg;
				}
				p = EX_VAR_NUM(EX(func)->op_array.last_var + EX(func)->op_array.T + skip);
			}
			while (i < arg_count) {
				q = p;
				if (EXPECTED(C_TYPE_INFO_P(q) != IS_UNDEF)) {
					ZVAL_DEREF(q);
					if (C_OPT_REFCOUNTED_P(q)) {
						C_ADDREF_P(q);
					}
					CREX_HASH_FILL_SET(q);
				} else {
					CREX_HASH_FILL_SET_NULL();
				}
				CREX_HASH_FILL_NEXT();
				p++;
				i++;
			}
		} CREX_HASH_FILL_END();
		ht->nNumOfElements = result_size;
	} else {
		ZVAL_EMPTY_ARRAY(EX_VAR(opline->result.var));
	}
	CREX_VM_NEXT_OPCODE();
}

/* Contrary to what its name indicates, CREX_COPY_TMP may receive and define references. */
CREX_VM_HANDLER(167, CREX_COPY_TMP, TMPVAR, UNUSED)
{
	USE_OPLINE
	zval *value = GET_OP1_ZVAL_PTR(BP_VAR_R);
	zval *result = EX_VAR(opline->result.var);
	ZVAL_COPY(result, value);
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HANDLER(202, CREX_CALLABLE_CONVERT, UNUSED, UNUSED)
{
	USE_OPLINE
	crex_execute_data *call = EX(call);

	crex_closure_from_frame(EX_VAR(opline->result.var), call);

	if (CREX_CALL_INFO(call) & CREX_CALL_RELEASE_THIS) {
		OBJ_RELEASE(C_OBJ(call->This));
	}

	EX(call) = call->prev_execute_data;

	crex_vm_stack_free_call_frame(call);

	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_JMP, (OP_JMP_ADDR(op, op->op1) > op), CREX_JMP_FORWARD, JMP_ADDR, ANY)
{
	USE_OPLINE

	OPLINE = OP_JMP_ADDR(opline, opline->op1);
	CREX_VM_CONTINUE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_ADD, (res_info == MAY_BE_LONG && op1_info == MAY_BE_LONG && op2_info == MAY_BE_LONG), CREX_ADD_LONG_NO_OVERFLOW, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(NO_CONST_CONST,COMMUTATIVE))
{
	USE_OPLINE
	zval *op1, *op2, *result;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	result = EX_VAR(opline->result.var);
	ZVAL_LONG(result, C_LVAL_P(op1) + C_LVAL_P(op2));
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_ADD, (op1_info == MAY_BE_LONG && op2_info == MAY_BE_LONG), CREX_ADD_LONG, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(NO_CONST_CONST,COMMUTATIVE))
{
	USE_OPLINE
	zval *op1, *op2, *result;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	result = EX_VAR(opline->result.var);
	fast_long_add_function(result, op1, op2);
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_ADD, (op1_info == MAY_BE_DOUBLE && op2_info == MAY_BE_DOUBLE), CREX_ADD_DOUBLE, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(NO_CONST_CONST,COMMUTATIVE))
{
	USE_OPLINE
	zval *op1, *op2, *result;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	result = EX_VAR(opline->result.var);
	ZVAL_DOUBLE(result, C_DVAL_P(op1) + C_DVAL_P(op2));
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_SUB, (res_info == MAY_BE_LONG && op1_info == MAY_BE_LONG && op2_info == MAY_BE_LONG), CREX_SUB_LONG_NO_OVERFLOW, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(NO_CONST_CONST))
{
	USE_OPLINE
	zval *op1, *op2, *result;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	result = EX_VAR(opline->result.var);
	ZVAL_LONG(result, C_LVAL_P(op1) - C_LVAL_P(op2));
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_SUB, (op1_info == MAY_BE_LONG && op2_info == MAY_BE_LONG), CREX_SUB_LONG, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(NO_CONST_CONST))
{
	USE_OPLINE
	zval *op1, *op2, *result;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	result = EX_VAR(opline->result.var);
	fast_long_sub_function(result, op1, op2);
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_SUB, (op1_info == MAY_BE_DOUBLE && op2_info == MAY_BE_DOUBLE), CREX_SUB_DOUBLE, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(NO_CONST_CONST))
{
	USE_OPLINE
	zval *op1, *op2, *result;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	result = EX_VAR(opline->result.var);
	ZVAL_DOUBLE(result, C_DVAL_P(op1) - C_DVAL_P(op2));
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_MUL, (res_info == MAY_BE_LONG && op1_info == MAY_BE_LONG && op2_info == MAY_BE_LONG), CREX_MUL_LONG_NO_OVERFLOW, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(NO_CONST_CONST,COMMUTATIVE))
{
	USE_OPLINE
	zval *op1, *op2, *result;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	result = EX_VAR(opline->result.var);
	ZVAL_LONG(result, C_LVAL_P(op1) * C_LVAL_P(op2));
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_MUL, (op1_info == MAY_BE_LONG && op2_info == MAY_BE_LONG), CREX_MUL_LONG, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(NO_CONST_CONST,COMMUTATIVE))
{
	USE_OPLINE
	zval *op1, *op2, *result;
	crex_long overflow;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	result = EX_VAR(opline->result.var);
	CREX_SIGNED_MULTIPLY_LONG(C_LVAL_P(op1), C_LVAL_P(op2), C_LVAL_P(result), C_DVAL_P(result), overflow);
	C_TYPE_INFO_P(result) = overflow ? IS_DOUBLE : IS_LONG;
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_MUL, (op1_info == MAY_BE_DOUBLE && op2_info == MAY_BE_DOUBLE), CREX_MUL_DOUBLE, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(NO_CONST_CONST,COMMUTATIVE))
{
	USE_OPLINE
	zval *op1, *op2, *result;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	result = EX_VAR(opline->result.var);
	ZVAL_DOUBLE(result, C_DVAL_P(op1) * C_DVAL_P(op2));
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_IS_EQUAL|CREX_IS_IDENTICAL, (op1_info == MAY_BE_LONG && op2_info == MAY_BE_LONG), CREX_IS_EQUAL_LONG, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(SMART_BRANCH,NO_CONST_CONST,COMMUTATIVE))
{
	USE_OPLINE
	zval *op1, *op2;
	bool result;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	result = (C_LVAL_P(op1) == C_LVAL_P(op2));
	CREX_VM_SMART_BRANCH(result, 0);
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_IS_EQUAL|CREX_IS_IDENTICAL, (op1_info == MAY_BE_DOUBLE && op2_info == MAY_BE_DOUBLE), CREX_IS_EQUAL_DOUBLE, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(SMART_BRANCH,NO_CONST_CONST,COMMUTATIVE))
{
	USE_OPLINE
	zval *op1, *op2;
	bool result;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	result = (C_DVAL_P(op1) == C_DVAL_P(op2));
	CREX_VM_SMART_BRANCH(result, 0);
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_IS_NOT_EQUAL|CREX_IS_NOT_IDENTICAL, (op1_info == MAY_BE_LONG && op2_info == MAY_BE_LONG), CREX_IS_NOT_EQUAL_LONG, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(SMART_BRANCH,NO_CONST_CONST,COMMUTATIVE))
{
	USE_OPLINE
	zval *op1, *op2;
	bool result;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	result = (C_LVAL_P(op1) != C_LVAL_P(op2));
	CREX_VM_SMART_BRANCH(result, 0);
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_IS_NOT_EQUAL|CREX_IS_NOT_IDENTICAL, (op1_info == MAY_BE_DOUBLE && op2_info == MAY_BE_DOUBLE), CREX_IS_NOT_EQUAL_DOUBLE, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(SMART_BRANCH,NO_CONST_CONST,COMMUTATIVE))
{
	USE_OPLINE
	zval *op1, *op2;
	bool result;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	result = (C_DVAL_P(op1) != C_DVAL_P(op2));
	CREX_VM_SMART_BRANCH(result, 0);
}

CREX_VM_TYPE_SPEC_HANDLER(CREX_IS_IDENTICAL, op->op1_type == IS_CV && (op->op2_type & (IS_CONST|IS_CV)) && !(op1_info & (MAY_BE_UNDEF|MAY_BE_REF)) && !(op2_info & (MAY_BE_UNDEF|MAY_BE_REF)), CREX_IS_IDENTICAL_NOTHROW, CV, CONST|CV, SPEC(COMMUTATIVE))
{
	/* This is declared below the specializations for MAY_BE_LONG/MAY_BE_DOUBLE so those will be used instead if possible. */
	/* This optimizes $x === SOME_CONST_EXPR and $x === $y for non-refs and non-undef, which can't throw. */
	/* (Infinite recursion when comparing arrays is an uncatchable fatal error) */
	USE_OPLINE
	zval *op1, *op2;
	bool result;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	result = fast_is_identical_function(op1, op2);
	/* Free is a no-op for const/cv */
	CREX_VM_SMART_BRANCH(result, 0);
}

CREX_VM_TYPE_SPEC_HANDLER(CREX_IS_NOT_IDENTICAL, op->op1_type == IS_CV && (op->op2_type & (IS_CONST|IS_CV)) && !(op1_info & (MAY_BE_UNDEF|MAY_BE_REF)) && !(op2_info & (MAY_BE_UNDEF|MAY_BE_REF)), CREX_IS_NOT_IDENTICAL_NOTHROW, CV, CONST|CV, SPEC(COMMUTATIVE))
{
	USE_OPLINE
	zval *op1, *op2;
	bool result;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	result = fast_is_identical_function(op1, op2);
	/* Free is a no-op for const/cv */
	CREX_VM_SMART_BRANCH(!result, 0);
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_IS_SMALLER, (op1_info == MAY_BE_LONG && op2_info == MAY_BE_LONG), CREX_IS_SMALLER_LONG, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(SMART_BRANCH,NO_CONST_CONST))
{
	USE_OPLINE
	zval *op1, *op2;
	bool result;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	result = (C_LVAL_P(op1) < C_LVAL_P(op2));
	CREX_VM_SMART_BRANCH(result, 0);
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_IS_SMALLER, (op1_info == MAY_BE_DOUBLE && op2_info == MAY_BE_DOUBLE), CREX_IS_SMALLER_DOUBLE, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(SMART_BRANCH,NO_CONST_CONST))
{
	USE_OPLINE
	zval *op1, *op2;
	bool result;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	result = (C_DVAL_P(op1) < C_DVAL_P(op2));
	CREX_VM_SMART_BRANCH(result, 0);
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_IS_SMALLER_OR_EQUAL, (op1_info == MAY_BE_LONG && op2_info == MAY_BE_LONG), CREX_IS_SMALLER_OR_EQUAL_LONG, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(SMART_BRANCH,NO_CONST_CONST))
{
	USE_OPLINE
	zval *op1, *op2;
	bool result;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	result = (C_LVAL_P(op1) <= C_LVAL_P(op2));
	CREX_VM_SMART_BRANCH(result, 0);
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_IS_SMALLER_OR_EQUAL, (op1_info == MAY_BE_DOUBLE && op2_info == MAY_BE_DOUBLE), CREX_IS_SMALLER_OR_EQUAL_DOUBLE, CONST|TMPVARCV, CONST|TMPVARCV, SPEC(SMART_BRANCH,NO_CONST_CONST))
{
	USE_OPLINE
	zval *op1, *op2;
	bool result;

	op1 = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	op2 = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	result = (C_DVAL_P(op1) <= C_DVAL_P(op2));
	CREX_VM_SMART_BRANCH(result, 0);
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_PRE_INC, (res_info == MAY_BE_LONG && op1_info == MAY_BE_LONG), CREX_PRE_INC_LONG_NO_OVERFLOW, CV, ANY, SPEC(RETVAL))
{
	USE_OPLINE
	zval *var_ptr;

	var_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);
	C_LVAL_P(var_ptr)++;
	if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
		ZVAL_LONG(EX_VAR(opline->result.var), C_LVAL_P(var_ptr));
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_PRE_INC, (op1_info == MAY_BE_LONG), CREX_PRE_INC_LONG, CV, ANY, SPEC(RETVAL))
{
	USE_OPLINE
	zval *var_ptr;

	var_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);
	fast_long_increment_function(var_ptr);
	if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
		ZVAL_COPY_VALUE(EX_VAR(opline->result.var), var_ptr);
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_PRE_DEC, (res_info == MAY_BE_LONG && op1_info == MAY_BE_LONG), CREX_PRE_DEC_LONG_NO_OVERFLOW, CV, ANY, SPEC(RETVAL))
{
	USE_OPLINE
	zval *var_ptr;

	var_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);
	C_LVAL_P(var_ptr)--;
	if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
		ZVAL_LONG(EX_VAR(opline->result.var), C_LVAL_P(var_ptr));
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_PRE_DEC, (op1_info == MAY_BE_LONG), CREX_PRE_DEC_LONG, CV, ANY, SPEC(RETVAL))
{
	USE_OPLINE
	zval *var_ptr;

	var_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);
	fast_long_decrement_function(var_ptr);
	if (UNEXPECTED(RETURN_VALUE_USED(opline))) {
		ZVAL_COPY_VALUE(EX_VAR(opline->result.var), var_ptr);
	}
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_POST_INC, (res_info == MAY_BE_LONG && op1_info == MAY_BE_LONG), CREX_POST_INC_LONG_NO_OVERFLOW, CV, ANY)
{
	USE_OPLINE
	zval *var_ptr;

	var_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);
	ZVAL_LONG(EX_VAR(opline->result.var), C_LVAL_P(var_ptr));
	C_LVAL_P(var_ptr)++;
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_POST_INC, (op1_info == MAY_BE_LONG), CREX_POST_INC_LONG, CV, ANY)
{
	USE_OPLINE
	zval *var_ptr;

	var_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);
	ZVAL_LONG(EX_VAR(opline->result.var), C_LVAL_P(var_ptr));
	fast_long_increment_function(var_ptr);
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_POST_DEC, (res_info == MAY_BE_LONG && op1_info == MAY_BE_LONG), CREX_POST_DEC_LONG_NO_OVERFLOW, CV, ANY)
{
	USE_OPLINE
	zval *var_ptr;

	var_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);
	ZVAL_LONG(EX_VAR(opline->result.var), C_LVAL_P(var_ptr));
	C_LVAL_P(var_ptr)--;
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_POST_DEC, (op1_info == MAY_BE_LONG), CREX_POST_DEC_LONG, CV, ANY)
{
	USE_OPLINE
	zval *var_ptr;

	var_ptr = GET_OP1_ZVAL_PTR_PTR_UNDEF(BP_VAR_RW);
	ZVAL_LONG(EX_VAR(opline->result.var), C_LVAL_P(var_ptr));
	fast_long_decrement_function(var_ptr);
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_QM_ASSIGN, (op1_info == MAY_BE_LONG), CREX_QM_ASSIGN_LONG, CONST|TMPVARCV, ANY)
{
	USE_OPLINE
	zval *value;

	value = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	ZVAL_LONG(EX_VAR(opline->result.var), C_LVAL_P(value));
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_QM_ASSIGN, (op1_info == MAY_BE_DOUBLE), CREX_QM_ASSIGN_DOUBLE, CONST|TMPVARCV, ANY)
{
	USE_OPLINE
	zval *value;

	value = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	ZVAL_DOUBLE(EX_VAR(opline->result.var), C_DVAL_P(value));
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_QM_ASSIGN, ((op->op1_type == IS_CONST) ? !C_REFCOUNTED_P(RT_CONSTANT(op, op->op1)) : (!(op1_info & ((MAY_BE_ANY|MAY_BE_UNDEF)-(MAY_BE_NULL|MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_LONG|MAY_BE_DOUBLE))))), CREX_QM_ASSIGN_NOREF, CONST|TMPVARCV, ANY)
{
	USE_OPLINE
	zval *value;

	value = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	ZVAL_COPY_VALUE(EX_VAR(opline->result.var), value);
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_FETCH_DIM_R, (!(op2_info & (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE|MAY_BE_REF))), CREX_FETCH_DIM_R_INDEX, CONST|TMPVAR|CV, CONST|TMPVARCV, SPEC(NO_CONST_CONST))
{
	USE_OPLINE
	zval *container, *dim, *value;
	crex_long offset;
	HashTable *ht;

	container = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	dim = GET_OP2_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (EXPECTED(C_TYPE_P(container) == IS_ARRAY)) {
CREX_VM_C_LABEL(fetch_dim_r_index_array):
		if (EXPECTED(C_TYPE_P(dim) == IS_LONG)) {
			offset = C_LVAL_P(dim);
		} else {
			SAVE_OPLINE();
			crex_fetch_dimension_address_read_R(container, dim, OP2_TYPE OPLINE_CC EXECUTE_DATA_CC);
			FREE_OP1();
			CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
		}
		ht = C_ARRVAL_P(container);
		CREX_HASH_INDEX_FIND(ht, offset, value, CREX_VM_C_LABEL(fetch_dim_r_index_undef));
		ZVAL_COPY_DEREF(EX_VAR(opline->result.var), value);
		if (OP1_TYPE & (IS_TMP_VAR|IS_VAR)) {
			SAVE_OPLINE();
			FREE_OP1();
			CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
		} else {
			CREX_VM_NEXT_OPCODE();
		}
	} else if (OP1_TYPE != IS_CONST && EXPECTED(C_TYPE_P(container) == IS_REFERENCE)) {
		container = C_REFVAL_P(container);
		if (EXPECTED(C_TYPE_P(container) == IS_ARRAY)) {
			CREX_VM_C_GOTO(fetch_dim_r_index_array);
		} else {
			CREX_VM_C_GOTO(fetch_dim_r_index_slow);
		}
	} else {
CREX_VM_C_LABEL(fetch_dim_r_index_slow):
		SAVE_OPLINE();
		if (OP2_TYPE == IS_CONST && C_EXTRA_P(dim) == CREX_EXTRA_VALUE) {
			dim++;
		}
		crex_fetch_dimension_address_read_R_slow(container, dim OPLINE_CC EXECUTE_DATA_CC);
		FREE_OP1();
		CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
	}

CREX_VM_C_LABEL(fetch_dim_r_index_undef):
	ZVAL_NULL(EX_VAR(opline->result.var));
	SAVE_OPLINE();
	crex_undefined_offset(offset);
	FREE_OP1();
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_SEND_VAR, op->op2_type == IS_UNUSED && (op1_info & (MAY_BE_UNDEF|MAY_BE_REF)) == 0, CREX_SEND_VAR_SIMPLE, CV|VAR, NUM)
{
	USE_OPLINE
	zval *varptr, *arg;

	varptr = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	arg = CREX_CALL_VAR(EX(call), opline->result.var);

	if (OP1_TYPE == IS_CV) {
		ZVAL_COPY(arg, varptr);
	} else /* if (OP1_TYPE == IS_VAR) */ {
		ZVAL_COPY_VALUE(arg, varptr);
	}

	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_SEND_VAR_EX, op->op2_type == IS_UNUSED && op->op2.num <= MAX_ARG_FLAG_NUM && (op1_info & (MAY_BE_UNDEF|MAY_BE_REF)) == 0, CREX_SEND_VAR_EX_SIMPLE, CV|VAR, UNUSED|NUM)
{
	USE_OPLINE
	zval *varptr, *arg;
	uint32_t arg_num = opline->op2.num;

	if (QUICK_ARG_SHOULD_BE_SENT_BY_REF(EX(call)->func, arg_num)) {
		CREX_VM_DISPATCH_TO_HANDLER(CREX_SEND_REF);
	}

	varptr = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	arg = CREX_CALL_VAR(EX(call), opline->result.var);

	if (OP1_TYPE == IS_CV) {
		ZVAL_COPY(arg, varptr);
	} else /* if (OP1_TYPE == IS_VAR) */ {
		ZVAL_COPY_VALUE(arg, varptr);
	}

	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_SEND_VAL, op->op1_type == IS_CONST && op->op2_type == IS_UNUSED && !C_REFCOUNTED_P(RT_CONSTANT(op, op->op1)), CREX_SEND_VAL_SIMPLE, CONST, NUM)
{
	USE_OPLINE
	zval *value, *arg;

	value = GET_OP1_ZVAL_PTR(BP_VAR_R);
	arg = CREX_CALL_VAR(EX(call), opline->result.var);
	ZVAL_COPY_VALUE(arg, value);
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_SEND_VAL_EX, op->op2_type == IS_UNUSED && op->op2.num <= MAX_ARG_FLAG_NUM && op->op1_type == IS_CONST && !C_REFCOUNTED_P(RT_CONSTANT(op, op->op1)), CREX_SEND_VAL_EX_SIMPLE, CONST, NUM)
{
	USE_OPLINE
	zval *value, *arg;
	uint32_t arg_num = opline->op2.num;

	arg = CREX_CALL_VAR(EX(call), opline->result.var);
	if (QUICK_ARG_MUST_BE_SENT_BY_REF(EX(call)->func, arg_num)) {
		CREX_VM_DISPATCH_TO_HELPER(crex_cannot_pass_by_ref_helper, _arg_num, arg_num, _arg, arg);
	}
	value = GET_OP1_ZVAL_PTR(BP_VAR_R);
	ZVAL_COPY_VALUE(arg, value);
	CREX_VM_NEXT_OPCODE();
}

CREX_VM_HOT_TYPE_SPEC_HANDLER(CREX_FE_FETCH_R, op->op2_type == IS_CV && (op1_info & (MAY_BE_ANY|MAY_BE_REF)) == MAY_BE_ARRAY, CREX_FE_FETCH_R_SIMPLE, VAR, CV, JMP_ADDR, SPEC(RETVAL))
{
	USE_OPLINE
	zval *array;
	zval *value, *variable_ptr;
	uint32_t value_type;
	HashTable *fe_ht;
	HashPosition pos;

	array = EX_VAR(opline->op1.var);
	SAVE_OPLINE();
	fe_ht = C_ARRVAL_P(array);
	pos = C_FE_POS_P(array);
	if (HT_IS_PACKED(fe_ht)) {
		value = fe_ht->arPacked + pos;
		while (1) {
			if (UNEXPECTED(pos >= fe_ht->nNumUsed)) {
				/* reached end of iteration */
				CREX_VM_SET_RELATIVE_OPCODE(opline, opline->extended_value);
				CREX_VM_CONTINUE();
			}
			value_type = C_TYPE_INFO_P(value);
			CREX_ASSERT(value_type != IS_INDIRECT);
			if (EXPECTED(value_type != IS_UNDEF)) {
				break;
			}
			pos++;
			value++;
		}
		C_FE_POS_P(array) = pos + 1;
		if (RETURN_VALUE_USED(opline)) {
			ZVAL_LONG(EX_VAR(opline->result.var), pos);
		}
	} else {
		Bucket *p;

		p = fe_ht->arData + pos;
		while (1) {
			if (UNEXPECTED(pos >= fe_ht->nNumUsed)) {
				/* reached end of iteration */
				CREX_VM_SET_RELATIVE_OPCODE(opline, opline->extended_value);
				CREX_VM_CONTINUE();
			}
			pos++;
			value = &p->val;
			value_type = C_TYPE_INFO_P(value);
			CREX_ASSERT(value_type != IS_INDIRECT);
			if (EXPECTED(value_type != IS_UNDEF)) {
				break;
			}
			p++;
		}
		C_FE_POS_P(array) = pos;
		if (RETURN_VALUE_USED(opline)) {
			if (!p->key) {
				ZVAL_LONG(EX_VAR(opline->result.var), p->h);
			} else {
				ZVAL_STR_COPY(EX_VAR(opline->result.var), p->key);
			}
		}
	}

	variable_ptr = EX_VAR(opline->op2.var);
	crex_assign_to_variable(variable_ptr, value, IS_CV, EX_USES_STRICT_TYPES());
	CREX_VM_NEXT_OPCODE_CHECK_EXCEPTION();
}

CREX_VM_DEFINE_OP(137, CREX_OP_DATA);

CREX_VM_HELPER(crex_interrupt_helper, ANY, ANY)
{
	crex_atomic_bool_store_ex(&EG(vm_interrupt), false);
	SAVE_OPLINE();
	if (crex_atomic_bool_load_ex(&EG(timed_out))) {
		crex_timeout();
	} else if (crex_interrupt_function) {
		crex_interrupt_function(execute_data);
		if (EG(exception)) {
			/* We have to UNDEF result, because CREX_HANDLE_EXCEPTION is going to free it */
			const crex_op *throw_op = EG(opline_before_exception);

			if (throw_op
			 && throw_op->result_type & (IS_TMP_VAR|IS_VAR)
			 && throw_op->opcode != CREX_ADD_ARRAY_ELEMENT
			 && throw_op->opcode != CREX_ADD_ARRAY_UNPACK
			 && throw_op->opcode != CREX_ROPE_INIT
			 && throw_op->opcode != CREX_ROPE_ADD) {
				ZVAL_UNDEF(CREX_CALL_VAR(EG(current_execute_data), throw_op->result.var));

			}
		}
		CREX_VM_ENTER();
	}
	CREX_VM_CONTINUE();
}
