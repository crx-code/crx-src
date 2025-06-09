/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: ebd3dc9902075c276828c17dc7a1c3bdc5401f8e */

CREX_BEGIN_ARG_INFO_EX(arginfo_bzopen, 0, 0, 2)
	CREX_ARG_INFO(0, file)
	CREX_ARG_TYPE_INFO(0, mode, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_bzread, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO(0, bz)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 0, "1024")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_bzwrite, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_INFO(0, bz)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_bzflush, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, bz)
CREX_END_ARG_INFO()

#define arginfo_bzclose arginfo_bzflush

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_bzerrno, 0, 1, IS_LONG, 0)
	CREX_ARG_INFO(0, bz)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_bzerrstr, 0, 1, IS_STRING, 0)
	CREX_ARG_INFO(0, bz)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_bzerror, 0, 1, IS_ARRAY, 0)
	CREX_ARG_INFO(0, bz)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_bzcompress, 0, 1, MAY_BE_STRING|MAY_BE_LONG)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, block_size, IS_LONG, 0, "4")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, work_factor, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_bzdecompress, 0, 1, MAY_BE_STRING|MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, use_less_memory, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()


CREX_FUNCTION(bzopen);
CREX_FUNCTION(bzread);
CREX_FUNCTION(fwrite);
CREX_FUNCTION(fflush);
CREX_FUNCTION(fclose);
CREX_FUNCTION(bzerrno);
CREX_FUNCTION(bzerrstr);
CREX_FUNCTION(bzerror);
CREX_FUNCTION(bzcompress);
CREX_FUNCTION(bzdecompress);


static const crex_function_entry ext_functions[] = {
	CREX_FE(bzopen, arginfo_bzopen)
	CREX_FE(bzread, arginfo_bzread)
	CREX_FALIAS(bzwrite, fwrite, arginfo_bzwrite)
	CREX_FALIAS(bzflush, fflush, arginfo_bzflush)
	CREX_FALIAS(bzclose, fclose, arginfo_bzclose)
	CREX_FE(bzerrno, arginfo_bzerrno)
	CREX_FE(bzerrstr, arginfo_bzerrstr)
	CREX_FE(bzerror, arginfo_bzerror)
	CREX_FE(bzcompress, arginfo_bzcompress)
	CREX_FE(bzdecompress, arginfo_bzdecompress)
	CREX_FE_END
};
