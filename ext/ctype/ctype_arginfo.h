/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 155783e1858a7f24dbc1c3e810d5cffee5468bf7 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ctype_alnum, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, text, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_ctype_alpha arginfo_ctype_alnum

#define arginfo_ctype_cntrl arginfo_ctype_alnum

#define arginfo_ctype_digit arginfo_ctype_alnum

#define arginfo_ctype_lower arginfo_ctype_alnum

#define arginfo_ctype_graph arginfo_ctype_alnum

#define arginfo_ctype_print arginfo_ctype_alnum

#define arginfo_ctype_punct arginfo_ctype_alnum

#define arginfo_ctype_space arginfo_ctype_alnum

#define arginfo_ctype_upper arginfo_ctype_alnum

#define arginfo_ctype_xdigit arginfo_ctype_alnum


CREX_FUNCTION(ctype_alnum);
CREX_FUNCTION(ctype_alpha);
CREX_FUNCTION(ctype_cntrl);
CREX_FUNCTION(ctype_digit);
CREX_FUNCTION(ctype_lower);
CREX_FUNCTION(ctype_graph);
CREX_FUNCTION(ctype_print);
CREX_FUNCTION(ctype_punct);
CREX_FUNCTION(ctype_space);
CREX_FUNCTION(ctype_upper);
CREX_FUNCTION(ctype_xdigit);


static const crex_function_entry ext_functions[] = {
	CREX_FE(ctype_alnum, arginfo_ctype_alnum)
	CREX_FE(ctype_alpha, arginfo_ctype_alpha)
	CREX_FE(ctype_cntrl, arginfo_ctype_cntrl)
	CREX_FE(ctype_digit, arginfo_ctype_digit)
	CREX_FE(ctype_lower, arginfo_ctype_lower)
	CREX_FE(ctype_graph, arginfo_ctype_graph)
	CREX_FE(ctype_print, arginfo_ctype_print)
	CREX_FE(ctype_punct, arginfo_ctype_punct)
	CREX_FE(ctype_space, arginfo_ctype_space)
	CREX_FE(ctype_upper, arginfo_ctype_upper)
	CREX_FE(ctype_xdigit, arginfo_ctype_xdigit)
	CREX_FE_END
};
