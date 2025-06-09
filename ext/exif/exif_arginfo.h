/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 633b2db018fa1453845a854a6361f11f107f4653 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_exif_tagname, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_exif_read_data, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_INFO(0, file)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, required_sections, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, as_arrays, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, read_thumbnail, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_exif_thumbnail, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO(0, file)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, width, "null")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, height, "null")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, image_type, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_exif_imagetype, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()


CREX_FUNCTION(exif_tagname);
CREX_FUNCTION(exif_read_data);
CREX_FUNCTION(exif_thumbnail);
CREX_FUNCTION(exif_imagetype);


static const crex_function_entry ext_functions[] = {
	CREX_FE(exif_tagname, arginfo_exif_tagname)
	CREX_FE(exif_read_data, arginfo_exif_read_data)
	CREX_FE(exif_thumbnail, arginfo_exif_thumbnail)
	CREX_FE(exif_imagetype, arginfo_exif_imagetype)
	CREX_FE_END
};

static void register_exif_symbols(int module_number)
{
	REGISTER_LONG_CONSTANT("EXIF_USE_MBSTRING", USE_MBSTRING, CONST_PERSISTENT);
}
