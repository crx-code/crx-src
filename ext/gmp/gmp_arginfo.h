/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: d52f82c7084a8122fe07c91eb6d4ab6030daa27d */

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_gmp_init, 0, 1, GMP, 0)
	CREX_ARG_TYPE_MASK(0, num, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, base, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_gmp_import, 0, 1, GMP, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, word_size, IS_LONG, 0, "1")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "GMP_MSW_FIRST | GMP_NATIVE_ENDIAN")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gmp_export, 0, 1, IS_STRING, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, num, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, word_size, IS_LONG, 0, "1")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "GMP_MSW_FIRST | GMP_NATIVE_ENDIAN")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gmp_intval, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, num, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gmp_strval, 0, 1, IS_STRING, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, num, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, base, IS_LONG, 0, "10")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_gmp_add, 0, 2, GMP, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, num1, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_OBJ_TYPE_MASK(0, num2, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

#define arginfo_gmp_sub arginfo_gmp_add

#define arginfo_gmp_mul arginfo_gmp_add

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gmp_div_qr, 0, 2, IS_ARRAY, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, num1, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_OBJ_TYPE_MASK(0, num2, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, rounding_mode, IS_LONG, 0, "GMP_ROUND_ZERO")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_gmp_div_q, 0, 2, GMP, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, num1, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_OBJ_TYPE_MASK(0, num2, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, rounding_mode, IS_LONG, 0, "GMP_ROUND_ZERO")
CREX_END_ARG_INFO()

#define arginfo_gmp_div_r arginfo_gmp_div_q

#define arginfo_gmp_div arginfo_gmp_div_q

#define arginfo_gmp_mod arginfo_gmp_add

#define arginfo_gmp_divexact arginfo_gmp_add

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_gmp_neg, 0, 1, GMP, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, num, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

#define arginfo_gmp_abs arginfo_gmp_neg

#define arginfo_gmp_fact arginfo_gmp_neg

#define arginfo_gmp_sqrt arginfo_gmp_neg

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gmp_sqrtrem, 0, 1, IS_ARRAY, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, num, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_gmp_root, 0, 2, GMP, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, num, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO(0, nth, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gmp_rootrem, 0, 2, IS_ARRAY, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, num, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO(0, nth, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_gmp_pow, 0, 2, GMP, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, num, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO(0, exponent, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_gmp_powm, 0, 3, GMP, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, num, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_OBJ_TYPE_MASK(0, exponent, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_OBJ_TYPE_MASK(0, modulus, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gmp_perfect_square, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, num, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

#define arginfo_gmp_perfect_power arginfo_gmp_perfect_square

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gmp_prob_prime, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, num, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, repetitions, IS_LONG, 0, "10")
CREX_END_ARG_INFO()

#define arginfo_gmp_gcd arginfo_gmp_add

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gmp_gcdext, 0, 2, IS_ARRAY, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, num1, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_OBJ_TYPE_MASK(0, num2, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

#define arginfo_gmp_lcm arginfo_gmp_add

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_gmp_invert, 0, 2, GMP, MAY_BE_FALSE)
	CREX_ARG_OBJ_TYPE_MASK(0, num1, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_OBJ_TYPE_MASK(0, num2, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gmp_jacobi, 0, 2, IS_LONG, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, num1, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_OBJ_TYPE_MASK(0, num2, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

#define arginfo_gmp_legendre arginfo_gmp_jacobi

#define arginfo_gmp_kronecker arginfo_gmp_jacobi

#define arginfo_gmp_cmp arginfo_gmp_jacobi

#define arginfo_gmp_sign arginfo_gmp_intval

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gmp_random_seed, 0, 1, IS_VOID, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, seed, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_gmp_random_bits, 0, 1, GMP, 0)
	CREX_ARG_TYPE_INFO(0, bits, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_gmp_random_range, 0, 2, GMP, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, min, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_OBJ_TYPE_MASK(0, max, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

#define arginfo_gmp_and arginfo_gmp_add

#define arginfo_gmp_or arginfo_gmp_add

#define arginfo_gmp_com arginfo_gmp_neg

#define arginfo_gmp_xor arginfo_gmp_add

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gmp_setbit, 0, 2, IS_VOID, 0)
	CREX_ARG_OBJ_INFO(0, num, GMP, 0)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, value, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gmp_clrbit, 0, 2, IS_VOID, 0)
	CREX_ARG_OBJ_INFO(0, num, GMP, 0)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gmp_testbit, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, num, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gmp_scan0, 0, 2, IS_LONG, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, num1, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO(0, start, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_gmp_scan1 arginfo_gmp_scan0

#define arginfo_gmp_popcount arginfo_gmp_intval

#define arginfo_gmp_hamdist arginfo_gmp_jacobi

#define arginfo_gmp_nextprime arginfo_gmp_neg

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_gmp_binomial, 0, 2, GMP, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, n, GMP, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO(0, k, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_GMP___main, 0, 0, 0)
	CREX_ARG_TYPE_MASK(0, num, MAY_BE_LONG|MAY_BE_STRING, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, base, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_GMP___serialize, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_GMP___unserialize, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_ARRAY, 0)
CREX_END_ARG_INFO()


CREX_FUNCTION(gmp_init);
CREX_FUNCTION(gmp_import);
CREX_FUNCTION(gmp_export);
CREX_FUNCTION(gmp_intval);
CREX_FUNCTION(gmp_strval);
CREX_FUNCTION(gmp_add);
CREX_FUNCTION(gmp_sub);
CREX_FUNCTION(gmp_mul);
CREX_FUNCTION(gmp_div_qr);
CREX_FUNCTION(gmp_div_q);
CREX_FUNCTION(gmp_div_r);
CREX_FUNCTION(gmp_mod);
CREX_FUNCTION(gmp_divexact);
CREX_FUNCTION(gmp_neg);
CREX_FUNCTION(gmp_abs);
CREX_FUNCTION(gmp_fact);
CREX_FUNCTION(gmp_sqrt);
CREX_FUNCTION(gmp_sqrtrem);
CREX_FUNCTION(gmp_root);
CREX_FUNCTION(gmp_rootrem);
CREX_FUNCTION(gmp_pow);
CREX_FUNCTION(gmp_powm);
CREX_FUNCTION(gmp_perfect_square);
CREX_FUNCTION(gmp_perfect_power);
CREX_FUNCTION(gmp_prob_prime);
CREX_FUNCTION(gmp_gcd);
CREX_FUNCTION(gmp_gcdext);
CREX_FUNCTION(gmp_lcm);
CREX_FUNCTION(gmp_invert);
CREX_FUNCTION(gmp_jacobi);
CREX_FUNCTION(gmp_legendre);
CREX_FUNCTION(gmp_kronecker);
CREX_FUNCTION(gmp_cmp);
CREX_FUNCTION(gmp_sign);
CREX_FUNCTION(gmp_random_seed);
CREX_FUNCTION(gmp_random_bits);
CREX_FUNCTION(gmp_random_range);
CREX_FUNCTION(gmp_and);
CREX_FUNCTION(gmp_or);
CREX_FUNCTION(gmp_com);
CREX_FUNCTION(gmp_xor);
CREX_FUNCTION(gmp_setbit);
CREX_FUNCTION(gmp_clrbit);
CREX_FUNCTION(gmp_testbit);
CREX_FUNCTION(gmp_scan0);
CREX_FUNCTION(gmp_scan1);
CREX_FUNCTION(gmp_popcount);
CREX_FUNCTION(gmp_hamdist);
CREX_FUNCTION(gmp_nextprime);
CREX_FUNCTION(gmp_binomial);
CREX_METHOD(GMP, __main);
CREX_METHOD(GMP, __serialize);
CREX_METHOD(GMP, __unserialize);


static const crex_function_entry ext_functions[] = {
	CREX_FE(gmp_init, arginfo_gmp_init)
	CREX_FE(gmp_import, arginfo_gmp_import)
	CREX_FE(gmp_export, arginfo_gmp_export)
	CREX_FE(gmp_intval, arginfo_gmp_intval)
	CREX_FE(gmp_strval, arginfo_gmp_strval)
	CREX_FE(gmp_add, arginfo_gmp_add)
	CREX_FE(gmp_sub, arginfo_gmp_sub)
	CREX_FE(gmp_mul, arginfo_gmp_mul)
	CREX_FE(gmp_div_qr, arginfo_gmp_div_qr)
	CREX_FE(gmp_div_q, arginfo_gmp_div_q)
	CREX_FE(gmp_div_r, arginfo_gmp_div_r)
	CREX_FALIAS(gmp_div, gmp_div_q, arginfo_gmp_div)
	CREX_FE(gmp_mod, arginfo_gmp_mod)
	CREX_FE(gmp_divexact, arginfo_gmp_divexact)
	CREX_FE(gmp_neg, arginfo_gmp_neg)
	CREX_FE(gmp_abs, arginfo_gmp_abs)
	CREX_FE(gmp_fact, arginfo_gmp_fact)
	CREX_FE(gmp_sqrt, arginfo_gmp_sqrt)
	CREX_FE(gmp_sqrtrem, arginfo_gmp_sqrtrem)
	CREX_FE(gmp_root, arginfo_gmp_root)
	CREX_FE(gmp_rootrem, arginfo_gmp_rootrem)
	CREX_FE(gmp_pow, arginfo_gmp_pow)
	CREX_FE(gmp_powm, arginfo_gmp_powm)
	CREX_FE(gmp_perfect_square, arginfo_gmp_perfect_square)
	CREX_FE(gmp_perfect_power, arginfo_gmp_perfect_power)
	CREX_FE(gmp_prob_prime, arginfo_gmp_prob_prime)
	CREX_FE(gmp_gcd, arginfo_gmp_gcd)
	CREX_FE(gmp_gcdext, arginfo_gmp_gcdext)
	CREX_FE(gmp_lcm, arginfo_gmp_lcm)
	CREX_FE(gmp_invert, arginfo_gmp_invert)
	CREX_FE(gmp_jacobi, arginfo_gmp_jacobi)
	CREX_FE(gmp_legendre, arginfo_gmp_legendre)
	CREX_FE(gmp_kronecker, arginfo_gmp_kronecker)
	CREX_FE(gmp_cmp, arginfo_gmp_cmp)
	CREX_FE(gmp_sign, arginfo_gmp_sign)
	CREX_FE(gmp_random_seed, arginfo_gmp_random_seed)
	CREX_FE(gmp_random_bits, arginfo_gmp_random_bits)
	CREX_FE(gmp_random_range, arginfo_gmp_random_range)
	CREX_FE(gmp_and, arginfo_gmp_and)
	CREX_FE(gmp_or, arginfo_gmp_or)
	CREX_FE(gmp_com, arginfo_gmp_com)
	CREX_FE(gmp_xor, arginfo_gmp_xor)
	CREX_FE(gmp_setbit, arginfo_gmp_setbit)
	CREX_FE(gmp_clrbit, arginfo_gmp_clrbit)
	CREX_FE(gmp_testbit, arginfo_gmp_testbit)
	CREX_FE(gmp_scan0, arginfo_gmp_scan0)
	CREX_FE(gmp_scan1, arginfo_gmp_scan1)
	CREX_FE(gmp_popcount, arginfo_gmp_popcount)
	CREX_FE(gmp_hamdist, arginfo_gmp_hamdist)
	CREX_FE(gmp_nextprime, arginfo_gmp_nextprime)
	CREX_FE(gmp_binomial, arginfo_gmp_binomial)
	CREX_FE_END
};


static const crex_function_entry class_GMP_methods[] = {
	CREX_ME(GMP, __main, arginfo_class_GMP___main, CREX_ACC_PUBLIC)
	CREX_ME(GMP, __serialize, arginfo_class_GMP___serialize, CREX_ACC_PUBLIC)
	CREX_ME(GMP, __unserialize, arginfo_class_GMP___unserialize, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static void register_gmp_symbols(int module_number)
{
	REGISTER_LONG_CONSTANT("GMP_ROUND_ZERO", GMP_ROUND_ZERO, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GMP_ROUND_PLUSINF", GMP_ROUND_PLUSINF, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GMP_ROUND_MINUSINF", GMP_ROUND_MINUSINF, CONST_PERSISTENT);
#if defined(mpir_version)
	REGISTER_STRING_CONSTANT("GMP_MPIR_VERSION", GMP_MPIR_VERSION_STRING, CONST_PERSISTENT);
#endif
	REGISTER_STRING_CONSTANT("GMP_VERSION", GMP_VERSION_STRING, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GMP_MSW_FIRST", GMP_MSW_FIRST, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GMP_LSW_FIRST", GMP_LSW_FIRST, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GMP_LITTLE_ENDIAN", GMP_LITTLE_ENDIAN, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GMP_BIG_ENDIAN", GMP_BIG_ENDIAN, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GMP_NATIVE_ENDIAN", GMP_NATIVE_ENDIAN, CONST_PERSISTENT);
}

static crex_class_entry *register_class_GMP(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "GMP", class_GMP_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	return class_entry;
}
