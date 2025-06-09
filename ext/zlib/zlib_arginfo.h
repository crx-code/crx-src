/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 3660ad3239f93c84b6909c36ddfcc92dd0773c70 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ob_gzhandler, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_zlib_get_coding_type, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_gzfile, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, use_include_path, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_gzopen, 0, 0, 2)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, mode, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, use_include_path, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_readgzfile, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, use_include_path, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_zlib_encode, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, encoding, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, level, IS_LONG, 0, "-1")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_zlib_decode, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, max_length, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_gzdeflate, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, level, IS_LONG, 0, "-1")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_LONG, 0, "ZLIB_ENCODING_RAW")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_gzencode, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, level, IS_LONG, 0, "-1")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_LONG, 0, "ZLIB_ENCODING_GZIP")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_gzcompress, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, level, IS_LONG, 0, "-1")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_LONG, 0, "ZLIB_ENCODING_DEFLATE")
CREX_END_ARG_INFO()

#define arginfo_gzinflate arginfo_zlib_decode

#define arginfo_gzdecode arginfo_zlib_decode

#define arginfo_gzuncompress arginfo_zlib_decode

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_gzwrite, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_gzputs arginfo_gzwrite

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gzrewind, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, stream)
CREX_END_ARG_INFO()

#define arginfo_gzclose arginfo_gzrewind

#define arginfo_gzeof arginfo_gzrewind

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_gzgetc, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO(0, stream)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gzpassthru, 0, 1, IS_LONG, 0)
	CREX_ARG_INFO(0, stream)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gzseek, 0, 2, IS_LONG, 0)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, whence, IS_LONG, 0, "SEEK_SET")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_gztell, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_INFO(0, stream)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_gzread, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, length, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_gzgets, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_deflate_init, 0, 1, DeflateContext, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, encoding, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_deflate_add, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, context, DeflateContext, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flush_mode, IS_LONG, 0, "ZLIB_SYNC_FLUSH")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_inflate_init, 0, 1, InflateContext, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, encoding, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_inflate_add, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, context, InflateContext, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flush_mode, IS_LONG, 0, "ZLIB_SYNC_FLUSH")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_inflate_get_status, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, context, InflateContext, 0)
CREX_END_ARG_INFO()

#define arginfo_inflate_get_read_len arginfo_inflate_get_status


CREX_FUNCTION(ob_gzhandler);
CREX_FUNCTION(zlib_get_coding_type);
CREX_FUNCTION(gzfile);
CREX_FUNCTION(gzopen);
CREX_FUNCTION(readgzfile);
CREX_FUNCTION(zlib_encode);
CREX_FUNCTION(zlib_decode);
CREX_FUNCTION(gzdeflate);
CREX_FUNCTION(gzencode);
CREX_FUNCTION(gzcompress);
CREX_FUNCTION(gzinflate);
CREX_FUNCTION(gzdecode);
CREX_FUNCTION(gzuncompress);
CREX_FUNCTION(fwrite);
CREX_FUNCTION(rewind);
CREX_FUNCTION(fclose);
CREX_FUNCTION(feof);
CREX_FUNCTION(fgetc);
CREX_FUNCTION(fpassthru);
CREX_FUNCTION(fseek);
CREX_FUNCTION(ftell);
CREX_FUNCTION(fread);
CREX_FUNCTION(fgets);
CREX_FUNCTION(deflate_init);
CREX_FUNCTION(deflate_add);
CREX_FUNCTION(inflate_init);
CREX_FUNCTION(inflate_add);
CREX_FUNCTION(inflate_get_status);
CREX_FUNCTION(inflate_get_read_len);


static const crex_function_entry ext_functions[] = {
	CREX_FE(ob_gzhandler, arginfo_ob_gzhandler)
	CREX_FE(zlib_get_coding_type, arginfo_zlib_get_coding_type)
	CREX_FE(gzfile, arginfo_gzfile)
	CREX_FE(gzopen, arginfo_gzopen)
	CREX_FE(readgzfile, arginfo_readgzfile)
	CREX_FE(zlib_encode, arginfo_zlib_encode)
	CREX_FE(zlib_decode, arginfo_zlib_decode)
	CREX_FE(gzdeflate, arginfo_gzdeflate)
	CREX_FE(gzencode, arginfo_gzencode)
	CREX_FE(gzcompress, arginfo_gzcompress)
	CREX_FE(gzinflate, arginfo_gzinflate)
	CREX_FE(gzdecode, arginfo_gzdecode)
	CREX_FE(gzuncompress, arginfo_gzuncompress)
	CREX_FALIAS(gzwrite, fwrite, arginfo_gzwrite)
	CREX_FALIAS(gzputs, fwrite, arginfo_gzputs)
	CREX_FALIAS(gzrewind, rewind, arginfo_gzrewind)
	CREX_FALIAS(gzclose, fclose, arginfo_gzclose)
	CREX_FALIAS(gzeof, feof, arginfo_gzeof)
	CREX_FALIAS(gzgetc, fgetc, arginfo_gzgetc)
	CREX_FALIAS(gzpassthru, fpassthru, arginfo_gzpassthru)
	CREX_FALIAS(gzseek, fseek, arginfo_gzseek)
	CREX_FALIAS(gztell, ftell, arginfo_gztell)
	CREX_FALIAS(gzread, fread, arginfo_gzread)
	CREX_FALIAS(gzgets, fgets, arginfo_gzgets)
	CREX_FE(deflate_init, arginfo_deflate_init)
	CREX_FE(deflate_add, arginfo_deflate_add)
	CREX_FE(inflate_init, arginfo_inflate_init)
	CREX_FE(inflate_add, arginfo_inflate_add)
	CREX_FE(inflate_get_status, arginfo_inflate_get_status)
	CREX_FE(inflate_get_read_len, arginfo_inflate_get_read_len)
	CREX_FE_END
};


static const crex_function_entry class_InflateContext_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_DeflateContext_methods[] = {
	CREX_FE_END
};

static void register_zlib_symbols(int module_number)
{
	REGISTER_LONG_CONSTANT("FORCE_GZIP", CRX_ZLIB_ENCODING_GZIP, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FORCE_DEFLATE", CRX_ZLIB_ENCODING_DEFLATE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_ENCODING_RAW", CRX_ZLIB_ENCODING_RAW, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_ENCODING_GZIP", CRX_ZLIB_ENCODING_GZIP, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_ENCODING_DEFLATE", CRX_ZLIB_ENCODING_DEFLATE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_NO_FLUSH", C_NO_FLUSH, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_PARTIAL_FLUSH", C_PARTIAL_FLUSH, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_SYNC_FLUSH", C_SYNC_FLUSH, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_FULL_FLUSH", C_FULL_FLUSH, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_BLOCK", C_BLOCK, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_FINISH", C_FINISH, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_FILTERED", C_FILTERED, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_HUFFMAN_ONLY", C_HUFFMAN_ONLY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_RLE", C_RLE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_FIXED", C_FIXED, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_DEFAULT_STRATEGY", C_DEFAULT_STRATEGY, CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("ZLIB_VERSION", ZLIB_VERSION, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_VERNUM", ZLIB_VERNUM, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_OK", C_OK, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_STREAM_END", C_STREAM_END, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_NEED_DICT", C_NEED_DICT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_ERRNO", C_ERRNO, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_STREAM_ERROR", C_STREAM_ERROR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_DATA_ERROR", C_DATA_ERROR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_MEM_ERROR", C_MEM_ERROR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_BUF_ERROR", C_BUF_ERROR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ZLIB_VERSION_ERROR", C_VERSION_ERROR, CONST_PERSISTENT);
}

static crex_class_entry *register_class_InflateContext(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "InflateContext", class_InflateContext_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES|CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}

static crex_class_entry *register_class_DeflateContext(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DeflateContext", class_DeflateContext_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES|CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}
