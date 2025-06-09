/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: f28dafc2a279f5421cd0d0e668fde0032e996ebc */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_bcadd, 0, 2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, num1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, num2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, scale, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_bcsub arginfo_bcadd

#define arginfo_bcmul arginfo_bcadd

#define arginfo_bcdiv arginfo_bcadd

#define arginfo_bcmod arginfo_bcadd

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_bcpowmod, 0, 3, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, num, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, exponent, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, modulus, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, scale, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_bcpow, 0, 2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, num, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, exponent, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, scale, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_bcsqrt, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, num, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, scale, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_bccomp, 0, 2, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, num1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, num2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, scale, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_bcscale, 0, 0, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, scale, IS_LONG, 1, "null")
CREX_END_ARG_INFO()


CREX_FUNCTION(bcadd);
CREX_FUNCTION(bcsub);
CREX_FUNCTION(bcmul);
CREX_FUNCTION(bcdiv);
CREX_FUNCTION(bcmod);
CREX_FUNCTION(bcpowmod);
CREX_FUNCTION(bcpow);
CREX_FUNCTION(bcsqrt);
CREX_FUNCTION(bccomp);
CREX_FUNCTION(bcscale);


static const crex_function_entry ext_functions[] = {
	CREX_FE(bcadd, arginfo_bcadd)
	CREX_FE(bcsub, arginfo_bcsub)
	CREX_FE(bcmul, arginfo_bcmul)
	CREX_FE(bcdiv, arginfo_bcdiv)
	CREX_FE(bcmod, arginfo_bcmod)
	CREX_FE(bcpowmod, arginfo_bcpowmod)
	CREX_FE(bcpow, arginfo_bcpow)
	CREX_FE(bcsqrt, arginfo_bcsqrt)
	CREX_FE(bccomp, arginfo_bccomp)
	CREX_FE(bcscale, arginfo_bcscale)
	CREX_FE_END
};
