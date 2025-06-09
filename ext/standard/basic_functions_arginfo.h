/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 487cee0751d47b18bf0a8fbdb050313783f1b369 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_set_time_limit, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, seconds, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_header_register_callback, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ob_start, 0, 0, _IS_BOOL, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, callback, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, chunk_size, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "CRX_OUTPUT_HANDLER_STDFLAGS")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ob_flush, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

#define arginfo_ob_clean arginfo_ob_flush

#define arginfo_ob_end_flush arginfo_ob_flush

#define arginfo_ob_end_clean arginfo_ob_flush

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ob_get_flush, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
CREX_END_ARG_INFO()

#define arginfo_ob_get_clean arginfo_ob_get_flush

#define arginfo_ob_get_contents arginfo_ob_get_flush

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ob_get_level, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ob_get_length, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ob_list_handlers, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ob_get_status, 0, 0, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, full_status, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ob_implicit_flush, 0, 0, IS_VOID, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, enable, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

#define arginfo_output_reset_rewrite_vars arginfo_ob_flush

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_output_add_rewrite_var, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_wrapper_register, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, protocol, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, class, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

#define arginfo_stream_register_wrapper arginfo_stream_wrapper_register

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_wrapper_unregister, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, protocol, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_stream_wrapper_restore arginfo_stream_wrapper_unregister

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_push, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(1, array, IS_ARRAY, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(0, values, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_krsort, 0, 1, IS_TRUE, 0)
	CREX_ARG_TYPE_INFO(1, array, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "SORT_REGULAR")
CREX_END_ARG_INFO()

#define arginfo_ksort arginfo_krsort

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_count, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, value, Countable, MAY_BE_ARRAY, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "COUNT_NORMAL")
CREX_END_ARG_INFO()

#define arginfo_sizeof arginfo_count

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_natsort, 0, 1, IS_TRUE, 0)
	CREX_ARG_TYPE_INFO(1, array, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_natcasesort arginfo_natsort

#define arginfo_asort arginfo_krsort

#define arginfo_arsort arginfo_krsort

#define arginfo_sort arginfo_krsort

#define arginfo_rsort arginfo_krsort

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_usort, 0, 2, IS_TRUE, 0)
	CREX_ARG_TYPE_INFO(1, array, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
CREX_END_ARG_INFO()

#define arginfo_uasort arginfo_usort

#define arginfo_uksort arginfo_usort

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_end, 0, 1, IS_MIXED, 0)
	CREX_ARG_TYPE_MASK(1, array, MAY_BE_ARRAY|MAY_BE_OBJECT, NULL)
CREX_END_ARG_INFO()

#define arginfo_prev arginfo_end

#define arginfo_next arginfo_end

#define arginfo_reset arginfo_end

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_current, 0, 1, IS_MIXED, 0)
	CREX_ARG_TYPE_MASK(0, array, MAY_BE_ARRAY|MAY_BE_OBJECT, NULL)
CREX_END_ARG_INFO()

#define arginfo_pos arginfo_current

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_key, 0, 1, MAY_BE_LONG|MAY_BE_STRING|MAY_BE_NULL)
	CREX_ARG_TYPE_MASK(0, array, MAY_BE_ARRAY|MAY_BE_OBJECT, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_min, 0, 1, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(0, values, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_max arginfo_min

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_walk, 0, 2, IS_TRUE, 0)
	CREX_ARG_TYPE_MASK(1, array, MAY_BE_ARRAY|MAY_BE_OBJECT, NULL)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
	CREX_ARG_TYPE_INFO(0, arg, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_array_walk_recursive arginfo_array_walk

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_in_array, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, needle, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, haystack, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, strict, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_array_search, 0, 2, MAY_BE_LONG|MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, needle, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, haystack, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, strict, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_extract, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(CREX_SEND_PREFER_REF, array, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "EXTR_OVERWRITE")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, prefix, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_compact, 0, 1, IS_ARRAY, 0)
	CREX_ARG_INFO(0, var_name)
	CREX_ARG_VARIADIC_INFO(0, var_names)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_fill, 0, 3, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, start_index, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, count, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_fill_keys, 0, 2, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, keys, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_range, 0, 2, IS_ARRAY, 0)
	CREX_ARG_TYPE_MASK(0, start, MAY_BE_STRING|MAY_BE_LONG|MAY_BE_DOUBLE, NULL)
	CREX_ARG_TYPE_MASK(0, end, MAY_BE_STRING|MAY_BE_LONG|MAY_BE_DOUBLE, NULL)
	CREX_ARG_TYPE_MASK(0, step, MAY_BE_LONG|MAY_BE_DOUBLE, "1")
CREX_END_ARG_INFO()

#define arginfo_shuffle arginfo_natsort

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_pop, 0, 1, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(1, array, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_array_shift arginfo_array_pop

#define arginfo_array_unshift arginfo_array_push

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_splice, 0, 2, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(1, array, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, replacement, IS_MIXED, 0, "[]")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_slice, 0, 2, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, preserve_keys, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_merge, 0, 0, IS_ARRAY, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(0, arrays, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_array_merge_recursive arginfo_array_merge

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_replace, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(0, replacements, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_array_replace_recursive arginfo_array_replace

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_keys, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, filter_value, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, strict, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_array_key_first, 0, 1, MAY_BE_LONG|MAY_BE_STRING|MAY_BE_NULL)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_array_key_last arginfo_array_key_first

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_values, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_array_count_values arginfo_array_values

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_column, 0, 2, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
	CREX_ARG_TYPE_MASK(0, column_key, MAY_BE_LONG|MAY_BE_STRING|MAY_BE_NULL, NULL)
	CREX_ARG_TYPE_MASK(0, index_key, MAY_BE_LONG|MAY_BE_STRING|MAY_BE_NULL, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_reverse, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, preserve_keys, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_pad, 0, 3, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_array_flip arginfo_array_values

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_change_key_case, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, case, IS_LONG, 0, "CASE_LOWER")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_unique, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "SORT_STRING")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_intersect_key, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(0, arrays, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_intersect_ukey, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
	CREX_ARG_VARIADIC_INFO(0, rest)
CREX_END_ARG_INFO()

#define arginfo_array_intersect arginfo_array_intersect_key

#define arginfo_array_uintersect arginfo_array_intersect_ukey

#define arginfo_array_intersect_assoc arginfo_array_intersect_key

#define arginfo_array_uintersect_assoc arginfo_array_intersect_ukey

#define arginfo_array_intersect_uassoc arginfo_array_intersect_ukey

#define arginfo_array_uintersect_uassoc arginfo_array_intersect_ukey

#define arginfo_array_diff_key arginfo_array_intersect_key

#define arginfo_array_diff_ukey arginfo_array_intersect_ukey

#define arginfo_array_diff arginfo_array_intersect_key

#define arginfo_array_udiff arginfo_array_intersect_ukey

#define arginfo_array_diff_assoc arginfo_array_intersect_key

#define arginfo_array_diff_uassoc arginfo_array_intersect_ukey

#define arginfo_array_udiff_assoc arginfo_array_intersect_ukey

#define arginfo_array_udiff_uassoc arginfo_array_intersect_ukey

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_multisort, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(CREX_SEND_PREFER_REF, array)
	CREX_ARG_VARIADIC_INFO(CREX_SEND_PREFER_REF, rest)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_array_rand, 0, 1, MAY_BE_LONG|MAY_BE_STRING|MAY_BE_ARRAY)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, num, IS_LONG, 0, "1")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_array_sum, 0, 1, MAY_BE_LONG|MAY_BE_DOUBLE)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_array_product arginfo_array_sum

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_reduce, 0, 2, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, initial, IS_MIXED, 0, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_filter, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_CALLABLE, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_map, 0, 2, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 1)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(0, arrays, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_key_exists, 0, 2, _IS_BOOL, 0)
	CREX_ARG_INFO(0, key)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_key_exists arginfo_array_key_exists

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_chunk, 0, 2, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, preserve_keys, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_combine, 0, 2, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, keys, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, values, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_array_is_list, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_base64_encode, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_base64_decode, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, strict, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_constant, 0, 1, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ip2long, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, ip, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_long2ip, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, ip, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_getenv, 0, 0, MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, name, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, local_only, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

#if defined(HAVE_PUTENV)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_putenv, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, assignment, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_getopt, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, short_options, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, long_options, IS_ARRAY, 0, "[]")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, rest_index, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_flush, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sleep, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, seconds, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_usleep, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, microseconds, IS_LONG, 0)
CREX_END_ARG_INFO()

#if defined(HAVE_NANOSLEEP)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_time_nanosleep, 0, 2, MAY_BE_ARRAY|MAY_BE_BOOL)
	CREX_ARG_TYPE_INFO(0, seconds, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, nanoseconds, IS_LONG, 0)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_NANOSLEEP)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_time_sleep_until, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, timestamp, IS_DOUBLE, 0)
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_get_current_user, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_get_cfg_var, 0, 1, MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, option, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_error_log, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, message, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, message_type, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, destination, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, additional_headers, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_error_get_last, 0, 0, IS_ARRAY, 1)
CREX_END_ARG_INFO()

#define arginfo_error_clear_last arginfo_flush

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_call_user_func, 0, 1, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(0, args, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_call_user_func_array, 0, 2, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
	CREX_ARG_TYPE_INFO(0, args, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_forward_static_call arginfo_call_user_func

#define arginfo_forward_static_call_array arginfo_call_user_func_array

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_register_shutdown_function, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(0, args, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_highlight_file, 0, 1, MAY_BE_STRING|MAY_BE_BOOL)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, return, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

#define arginfo_show_source arginfo_highlight_file

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crx_strip_whitespace, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_highlight_string, 0, 1, MAY_BE_STRING|MAY_BE_BOOL)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, return, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ini_get, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, option, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ini_get_all, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, extension, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, details, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ini_set, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, option, IS_STRING, 0)
	CREX_ARG_TYPE_MASK(0, value, MAY_BE_STRING|MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_BOOL|MAY_BE_NULL, NULL)
CREX_END_ARG_INFO()

#define arginfo_ini_alter arginfo_ini_set

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ini_restore, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, option, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ini_parse_quantity, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, shorthand, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_set_include_path, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, include_path, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_get_include_path arginfo_ob_get_flush

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_print_r, 0, 1, MAY_BE_STRING|MAY_BE_BOOL)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, return, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

#define arginfo_connection_aborted arginfo_ob_get_level

#define arginfo_connection_status arginfo_ob_get_level

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ignore_user_abort, 0, 0, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, enable, _IS_BOOL, 1, "null")
CREX_END_ARG_INFO()

#if defined(HAVE_GETSERVBYNAME)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_getservbyname, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, service, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, protocol, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_GETSERVBYPORT)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_getservbyport, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, port, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, protocol, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_GETPROTOBYNAME)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_getprotobyname, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, protocol, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_GETPROTOBYNUMBER)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_getprotobynumber, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, protocol, IS_LONG, 0)
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_register_tick_function, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(0, args, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_unregister_tick_function, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_is_uploaded_file, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_move_uploaded_file, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, from, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, to, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_parse_ini_file, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, process_sections, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, scanner_mode, IS_LONG, 0, "INI_SCANNER_NORMAL")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_parse_ini_string, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, ini_string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, process_sections, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, scanner_mode, IS_LONG, 0, "INI_SCANNER_NORMAL")
CREX_END_ARG_INFO()

#if CREX_DEBUG
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_config_get_hash, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_GETLOADAVG)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_sys_getloadavg, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_get_browser, 0, 0, MAY_BE_OBJECT|MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, user_agent, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, return_array, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crc32, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crypt, 0, 2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, salt, IS_STRING, 0)
CREX_END_ARG_INFO()

#if defined(HAVE_STRPTIME)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_strptime, 0, 2, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, timestamp, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_GETHOSTNAME)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_gethostname, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_gethostbyaddr, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, ip, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gethostbyname, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, hostname, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_gethostbynamel, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, hostname, IS_STRING, 0)
CREX_END_ARG_INFO()

#if (defined(CRX_WIN32) || defined(HAVE_DNS_SEARCH_FUNC))
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dns_check_record, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, hostname, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_STRING, 0, "\"MX\"")
CREX_END_ARG_INFO()
#endif

#if (defined(CRX_WIN32) || defined(HAVE_DNS_SEARCH_FUNC))
#define arginfo_checkdnsrr arginfo_dns_check_record
#endif

#if (defined(CRX_WIN32) || defined(HAVE_DNS_SEARCH_FUNC))
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_dns_get_record, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, hostname, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_LONG, 0, "DNS_ANY")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, authoritative_name_servers, "null")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, additional_records, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, raw, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()
#endif

#if (defined(CRX_WIN32) || defined(HAVE_DNS_SEARCH_FUNC))
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dns_get_mx, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, hostname, IS_STRING, 0)
	CREX_ARG_INFO(1, hosts)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, weights, "null")
CREX_END_ARG_INFO()
#endif

#if (defined(CRX_WIN32) || defined(HAVE_DNS_SEARCH_FUNC))
#define arginfo_getmxrr arginfo_dns_get_mx
#endif

#if (defined(CRX_WIN32) || HAVE_GETIFADDRS || defined(__PASE__))
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_net_get_interfaces, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_FTOK)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ftok, 0, 2, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, project_id, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_hrtime, 0, 0, MAY_BE_ARRAY|MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, as_number, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_md5, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, binary, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_md5_file, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, binary, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

#define arginfo_getmyuid arginfo_ob_get_length

#define arginfo_getmygid arginfo_ob_get_length

#define arginfo_getmypid arginfo_ob_get_length

#define arginfo_getmyinode arginfo_ob_get_length

#define arginfo_getlastmod arginfo_ob_get_length

#define arginfo_sha1 arginfo_md5

#define arginfo_sha1_file arginfo_md5_file

#if defined(HAVE_SYSLOG_H)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_openlog, 0, 3, IS_TRUE, 0)
	CREX_ARG_TYPE_INFO(0, prefix, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, facility, IS_LONG, 0)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_SYSLOG_H)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_closelog, 0, 0, IS_TRUE, 0)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_SYSLOG_H)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_syslog, 0, 2, IS_TRUE, 0)
	CREX_ARG_TYPE_INFO(0, priority, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, message, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_INET_NTOP)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_inet_ntop, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, ip, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_INET_PTON)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_inet_pton, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, ip, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_metaphone, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, max_phonemes, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_header, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, header, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, replace, _IS_BOOL, 0, "true")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, response_code, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_header_remove, 0, 0, IS_VOID, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, name, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_setrawcookie, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, value, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_MASK(0, expires_or_options, MAY_BE_ARRAY|MAY_BE_LONG, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, path, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, domain, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, secure, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, httponly, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

#define arginfo_setcookie arginfo_setrawcookie

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_http_response_code, 0, 0, MAY_BE_LONG|MAY_BE_BOOL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, response_code, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_headers_sent, 0, 0, _IS_BOOL, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, filename, "null")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, line, "null")
CREX_END_ARG_INFO()

#define arginfo_headers_list arginfo_ob_list_handlers

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_htmlspecialchars, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "ENT_QUOTES | ENT_SUBSTITUTE | ENT_HTML401")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, double_encode, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_htmlspecialchars_decode, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "ENT_QUOTES | ENT_SUBSTITUTE | ENT_HTML401")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_html_entity_decode, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "ENT_QUOTES | ENT_SUBSTITUTE | ENT_HTML401")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_htmlentities arginfo_htmlspecialchars

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_get_html_translation_table, 0, 0, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, table, IS_LONG, 0, "HTML_SPECIALCHARS")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "ENT_QUOTES | ENT_SUBSTITUTE | ENT_HTML401")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 0, "\"UTF-8\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_assert, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, assertion, IS_MIXED, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, description, Throwable, MAY_BE_STRING|MAY_BE_NULL, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_assert_options, 0, 1, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, option, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_bin2hex arginfo_base64_encode

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_hex2bin, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_strspn, 0, 2, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, characters, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_strcspn arginfo_strspn

#if defined(HAVE_NL_LANGINFO)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_nl_langinfo, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, item, IS_LONG, 0)
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_strcoll, 0, 2, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, string1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string2, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_trim, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, characters, IS_STRING, 0, "\" \\n\\r\\t\\v\\x00\"")
CREX_END_ARG_INFO()

#define arginfo_rtrim arginfo_trim

#define arginfo_chop arginfo_trim

#define arginfo_ltrim arginfo_trim

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_wordwrap, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, width, IS_LONG, 0, "75")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, break, IS_STRING, 0, "\"\\n\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, cut_long_words, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_explode, 0, 2, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, separator, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, limit, IS_LONG, 0, "CRX_INT_MAX")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_implode, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_MASK(0, separator, MAY_BE_STRING|MAY_BE_ARRAY, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, array, IS_ARRAY, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_join arginfo_implode

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_strtok, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, token, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_strtoupper arginfo_base64_encode

#define arginfo_strtolower arginfo_base64_encode

#define arginfo_str_increment arginfo_base64_encode

#define arginfo_str_decrement arginfo_base64_encode

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_basename, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, suffix, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dirname, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, levels, IS_LONG, 0, "1")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_pathinfo, 0, 1, MAY_BE_ARRAY|MAY_BE_STRING)
	CREX_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "PATHINFO_ALL")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_stristr, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, haystack, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, needle, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, before_needle, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

#define arginfo_strstr arginfo_stristr

#define arginfo_strchr arginfo_stristr

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_strpos, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, haystack, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, needle, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

#define arginfo_stripos arginfo_strpos

#define arginfo_strrpos arginfo_strpos

#define arginfo_strripos arginfo_strpos

#define arginfo_strrchr arginfo_stristr

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_str_contains, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, haystack, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, needle, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_str_starts_with arginfo_str_contains

#define arginfo_str_ends_with arginfo_str_contains

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chunk_split, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 0, "76")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, separator, IS_STRING, 0, "\"\\r\\n\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_substr, 0, 2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_substr_replace, 0, 3, MAY_BE_STRING|MAY_BE_ARRAY)
	CREX_ARG_TYPE_MASK(0, string, MAY_BE_ARRAY|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_MASK(0, replace, MAY_BE_ARRAY|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_MASK(0, offset, MAY_BE_ARRAY|MAY_BE_LONG, NULL)
	CREX_ARG_TYPE_MASK(0, length, MAY_BE_ARRAY|MAY_BE_LONG|MAY_BE_NULL, "null")
CREX_END_ARG_INFO()

#define arginfo_quotemeta arginfo_base64_encode

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ord, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, character, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chr, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, codepoint, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_ucfirst arginfo_base64_encode

#define arginfo_lcfirst arginfo_base64_encode

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ucwords, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, separators, IS_STRING, 0, "\" \\t\\r\\n\\f\\v\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_strtr, 0, 2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_MASK(0, from, MAY_BE_STRING|MAY_BE_ARRAY, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, to, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_strrev arginfo_base64_encode

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_similar_text, 0, 2, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, string1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string2, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, percent, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_addcslashes, 0, 2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, characters, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_addslashes arginfo_base64_encode

#define arginfo_stripcslashes arginfo_base64_encode

#define arginfo_stripslashes arginfo_base64_encode

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_str_replace, 0, 3, MAY_BE_STRING|MAY_BE_ARRAY)
	CREX_ARG_TYPE_MASK(0, search, MAY_BE_ARRAY|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_MASK(0, replace, MAY_BE_ARRAY|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_MASK(0, subject, MAY_BE_STRING|MAY_BE_ARRAY, NULL)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, count, "null")
CREX_END_ARG_INFO()

#define arginfo_str_ireplace arginfo_str_replace

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_hebrev, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, max_chars_per_line, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_nl2br, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, use_xhtml, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_strip_tags, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_MASK(0, allowed_tags, MAY_BE_ARRAY|MAY_BE_STRING|MAY_BE_NULL, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_setlocale, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, category, IS_LONG, 0)
	CREX_ARG_INFO(0, locales)
	CREX_ARG_VARIADIC_INFO(0, rest)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_parse_str, 0, 2, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_INFO(1, result)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_str_getcsv, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, separator, IS_STRING, 0, "\",\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, enclosure, IS_STRING, 0, "\"\\\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, escape, IS_STRING, 0, "\"\\\\\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_str_repeat, 0, 2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, times, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_count_chars, 0, 1, MAY_BE_ARRAY|MAY_BE_STRING)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

#define arginfo_strnatcmp arginfo_strcoll

#define arginfo_localeconv arginfo_ob_list_handlers

#define arginfo_strnatcasecmp arginfo_strcoll

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_substr_count, 0, 2, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, haystack, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, needle, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_str_pad, 0, 2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pad_string, IS_STRING, 0, "\" \"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pad_type, IS_LONG, 0, "STR_PAD_RIGHT")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_sscanf, 0, 2, MAY_BE_ARRAY|MAY_BE_LONG|MAY_BE_NULL)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(1, vars, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_str_rot13 arginfo_base64_encode

#define arginfo_str_shuffle arginfo_base64_encode

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_str_word_count, 0, 1, MAY_BE_ARRAY|MAY_BE_LONG)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, format, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, characters, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_str_split, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 0, "1")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_strpbrk, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, characters, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_substr_compare, 0, 3, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, haystack, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, needle, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, case_insensitive, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

#define arginfo_utf8_encode arginfo_base64_encode

#define arginfo_utf8_decode arginfo_base64_encode

CREX_BEGIN_ARG_INFO_EX(arginfo_opendir, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, directory, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, context, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_dir, 0, 1, Directory, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, directory, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, context, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_closedir, 0, 0, IS_VOID, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, dir_handle, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chdir, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, directory, IS_STRING, 0)
CREX_END_ARG_INFO()

#if (defined(HAVE_CHROOT) && !defined(ZTS) && defined(ENABLE_CHROOT_FUNC))
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chroot, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, directory, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#define arginfo_getcwd arginfo_ob_get_flush

#define arginfo_rewinddir arginfo_closedir

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_readdir, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, dir_handle, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_scandir, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, directory, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, sorting_order, IS_LONG, 0, "SCANDIR_SORT_ASCENDING")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, context, "null")
CREX_END_ARG_INFO()

#if defined(HAVE_GLOB)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_glob, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_exec, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, command, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, output, "null")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, result_code, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_system, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, command, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, result_code, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_passthru, 0, 1, IS_FALSE, 1)
	CREX_ARG_TYPE_INFO(0, command, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, result_code, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_escapeshellcmd, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, command, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_escapeshellarg, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, arg, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_shell_exec, 0, 1, MAY_BE_STRING|MAY_BE_FALSE|MAY_BE_NULL)
	CREX_ARG_TYPE_INFO(0, command, IS_STRING, 0)
CREX_END_ARG_INFO()

#if defined(HAVE_NICE)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_proc_nice, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, priority, IS_LONG, 0)
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_flock, 0, 2, _IS_BOOL, 0)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, operation, IS_LONG, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, would_block, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_get_meta_tags, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, use_include_path, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pclose, 0, 1, IS_LONG, 0)
	CREX_ARG_INFO(0, handle)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_popen, 0, 0, 2)
	CREX_ARG_TYPE_INFO(0, command, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, mode, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_readfile, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, use_include_path, _IS_BOOL, 0, "false")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, context, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_rewind, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, stream)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_rmdir, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, directory, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, context, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_umask, 0, 0, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mask, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_fclose arginfo_rewind

#define arginfo_feof arginfo_rewind

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_fgetc, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO(0, stream)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_fgets, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_fread, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, length, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_fopen, 0, 0, 2)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, mode, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, use_include_path, _IS_BOOL, 0, "false")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, context, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_fscanf, 0, 2, MAY_BE_ARRAY|MAY_BE_LONG|MAY_BE_FALSE|MAY_BE_NULL)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(1, vars, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_fpassthru, 0, 1, IS_LONG, 0)
	CREX_ARG_INFO(0, stream)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ftruncate, 0, 2, _IS_BOOL, 0)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, size, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_fstat, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_INFO(0, stream)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_fseek, 0, 2, IS_LONG, 0)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, whence, IS_LONG, 0, "SEEK_SET")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ftell, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_INFO(0, stream)
CREX_END_ARG_INFO()

#define arginfo_fflush arginfo_rewind

#define arginfo_fsync arginfo_rewind

#define arginfo_fdatasync arginfo_rewind

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_fwrite, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_fputs arginfo_fwrite

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mkdir, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, directory, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, permissions, IS_LONG, 0, "0777")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, recursive, _IS_BOOL, 0, "false")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, context, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_rename, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, from, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, to, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, context, "null")
CREX_END_ARG_INFO()

#define arginfo_copy arginfo_rename

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_tempnam, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, directory, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, prefix, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_tmpfile, 0, 0, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_file, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, context, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_file_get_contents, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, use_include_path, _IS_BOOL, 0, "false")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, context, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_unlink, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, context, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_file_put_contents, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, context, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_fputcsv, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, fields, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, separator, IS_STRING, 0, "\",\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, enclosure, IS_STRING, 0, "\"\\\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, escape, IS_STRING, 0, "\"\\\\\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, eol, IS_STRING, 0, "\"\\n\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_fgetcsv, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, separator, IS_STRING, 0, "\",\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, enclosure, IS_STRING, 0, "\"\\\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, escape, IS_STRING, 0, "\"\\\\\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_realpath, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, path, IS_STRING, 0)
CREX_END_ARG_INFO()

#if defined(HAVE_FNMATCH)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_fnmatch, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()
#endif

#define arginfo_sys_get_temp_dir arginfo_get_current_user

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_fileatime, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_filectime arginfo_fileatime

#define arginfo_filegroup arginfo_fileatime

#define arginfo_fileinode arginfo_fileatime

#define arginfo_filemtime arginfo_fileatime

#define arginfo_fileowner arginfo_fileatime

#define arginfo_fileperms arginfo_fileatime

#define arginfo_filesize arginfo_fileatime

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_filetype, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_file_exists arginfo_is_uploaded_file

#define arginfo_is_writable arginfo_is_uploaded_file

#define arginfo_is_writeable arginfo_is_uploaded_file

#define arginfo_is_readable arginfo_is_uploaded_file

#define arginfo_is_executable arginfo_is_uploaded_file

#define arginfo_is_file arginfo_is_uploaded_file

#define arginfo_is_dir arginfo_is_uploaded_file

#define arginfo_is_link arginfo_is_uploaded_file

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_stat, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_lstat arginfo_stat

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chown, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_MASK(0, user, MAY_BE_STRING|MAY_BE_LONG, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chgrp, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_MASK(0, group, MAY_BE_STRING|MAY_BE_LONG, NULL)
CREX_END_ARG_INFO()

#if defined(HAVE_LCHOWN)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_lchown, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_MASK(0, user, MAY_BE_STRING|MAY_BE_LONG, NULL)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_LCHOWN)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_lchgrp, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_MASK(0, group, MAY_BE_STRING|MAY_BE_LONG, NULL)
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_chmod, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, permissions, IS_LONG, 0)
CREX_END_ARG_INFO()

#if defined(HAVE_UTIME)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_touch, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mtime, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, atime, IS_LONG, 1, "null")
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_clearstatcache, 0, 0, IS_VOID, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, clear_realpath_cache, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, filename, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_disk_total_space, 0, 1, MAY_BE_DOUBLE|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, directory, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_disk_free_space arginfo_disk_total_space

#define arginfo_diskfreespace arginfo_disk_total_space

#define arginfo_realpath_cache_get arginfo_ob_list_handlers

#define arginfo_realpath_cache_size arginfo_ob_get_level

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sprintf, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(0, values, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_printf, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(0, values, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_vprintf, 0, 2, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, values, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_vsprintf, 0, 2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, values, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_fprintf, 0, 2, IS_LONG, 0)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(0, values, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_vfprintf, 0, 3, IS_LONG, 0)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, values, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_fsockopen, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, hostname, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, port, IS_LONG, 0, "-1")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, error_code, "null")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, error_message, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_DOUBLE, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_pfsockopen arginfo_fsockopen

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_http_build_query, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_MASK(0, data, MAY_BE_ARRAY|MAY_BE_OBJECT, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, numeric_prefix, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arg_separator, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding_type, IS_LONG, 0, "CRX_QUERY_RFC1738")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_image_type_to_mime_type, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, image_type, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_image_type_to_extension, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, image_type, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, include_dot, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_getimagesize, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, image_info, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_getimagesizefromstring, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, image_info, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crxinfo, 0, 0, IS_TRUE, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "INFO_ALL")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_crxversion, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, extension, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crxcredits, 0, 0, IS_TRUE, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "CREDITS_ALL")
CREX_END_ARG_INFO()

#define arginfo_crx_sapi_name arginfo_ob_get_flush

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crx_uname, 0, 0, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_STRING, 0, "\"a\"")
CREX_END_ARG_INFO()

#define arginfo_crx_ini_scanned_files arginfo_ob_get_flush

#define arginfo_crx_ini_loaded_file arginfo_ob_get_flush

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_iptcembed, 0, 2, MAY_BE_STRING|MAY_BE_BOOL)
	CREX_ARG_TYPE_INFO(0, iptc_data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, spool, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_iptcparse, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, iptc_block, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_levenshtein, 0, 2, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, string1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, insertion_cost, IS_LONG, 0, "1")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, replacement_cost, IS_LONG, 0, "1")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, deletion_cost, IS_LONG, 0, "1")
CREX_END_ARG_INFO()

#if (defined(HAVE_SYMLINK) || defined(CRX_WIN32))
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_readlink, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, path, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#if (defined(HAVE_SYMLINK) || defined(CRX_WIN32))
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_linkinfo, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, path, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#if (defined(HAVE_SYMLINK) || defined(CRX_WIN32))
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_symlink, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, target, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, link, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#if (defined(HAVE_SYMLINK) || defined(CRX_WIN32))
#define arginfo_link arginfo_symlink
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mail, 0, 3, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, to, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, subject, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, message, IS_STRING, 0)
	CREX_ARG_TYPE_MASK(0, additional_headers, MAY_BE_ARRAY|MAY_BE_STRING, "[]")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, additional_params, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_abs, 0, 1, MAY_BE_LONG|MAY_BE_DOUBLE)
	CREX_ARG_TYPE_MASK(0, num, MAY_BE_LONG|MAY_BE_DOUBLE, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ceil, 0, 1, IS_DOUBLE, 0)
	CREX_ARG_TYPE_MASK(0, num, MAY_BE_LONG|MAY_BE_DOUBLE, NULL)
CREX_END_ARG_INFO()

#define arginfo_floor arginfo_ceil

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_round, 0, 1, IS_DOUBLE, 0)
	CREX_ARG_TYPE_MASK(0, num, MAY_BE_LONG|MAY_BE_DOUBLE, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, precision, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "CRX_ROUND_HALF_UP")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sin, 0, 1, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, num, IS_DOUBLE, 0)
CREX_END_ARG_INFO()

#define arginfo_cos arginfo_sin

#define arginfo_tan arginfo_sin

#define arginfo_asin arginfo_sin

#define arginfo_acos arginfo_sin

#define arginfo_atan arginfo_sin

#define arginfo_atanh arginfo_sin

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_atan2, 0, 2, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, y, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, x, IS_DOUBLE, 0)
CREX_END_ARG_INFO()

#define arginfo_sinh arginfo_sin

#define arginfo_cosh arginfo_sin

#define arginfo_tanh arginfo_sin

#define arginfo_asinh arginfo_sin

#define arginfo_acosh arginfo_sin

#define arginfo_expm1 arginfo_sin

#define arginfo_log1p arginfo_sin

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pi, 0, 0, IS_DOUBLE, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_is_finite, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, num, IS_DOUBLE, 0)
CREX_END_ARG_INFO()

#define arginfo_is_nan arginfo_is_finite

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intdiv, 0, 2, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, num1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, num2, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_is_infinite arginfo_is_finite

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_pow, 0, 2, MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_OBJECT)
	CREX_ARG_TYPE_INFO(0, num, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, exponent, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_exp arginfo_sin

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_log, 0, 1, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, num, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, base, IS_DOUBLE, 0, "M_E")
CREX_END_ARG_INFO()

#define arginfo_log10 arginfo_sin

#define arginfo_sqrt arginfo_sin

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_hypot, 0, 2, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, x, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, y, IS_DOUBLE, 0)
CREX_END_ARG_INFO()

#define arginfo_deg2rad arginfo_sin

#define arginfo_rad2deg arginfo_sin

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_bindec, 0, 1, MAY_BE_LONG|MAY_BE_DOUBLE)
	CREX_ARG_TYPE_INFO(0, binary_string, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_hexdec, 0, 1, MAY_BE_LONG|MAY_BE_DOUBLE)
	CREX_ARG_TYPE_INFO(0, hex_string, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_octdec, 0, 1, MAY_BE_LONG|MAY_BE_DOUBLE)
	CREX_ARG_TYPE_INFO(0, octal_string, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_decbin, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, num, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_decoct arginfo_decbin

#define arginfo_dechex arginfo_decbin

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_base_convert, 0, 3, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, num, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, from_base, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, to_base, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_number_format, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, num, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, decimals, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, decimal_separator, IS_STRING, 1, "\".\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, thousands_separator, IS_STRING, 1, "\",\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_fmod, 0, 2, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, num1, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, num2, IS_DOUBLE, 0)
CREX_END_ARG_INFO()

#define arginfo_fdiv arginfo_fmod

#if defined(HAVE_GETTIMEOFDAY)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_microtime, 0, 0, MAY_BE_STRING|MAY_BE_DOUBLE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, as_float, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_GETTIMEOFDAY)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_gettimeofday, 0, 0, MAY_BE_ARRAY|MAY_BE_DOUBLE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, as_float, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_GETRUSAGE)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_getrusage, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "0")
CREX_END_ARG_INFO()
#endif

#define arginfo_pack arginfo_sprintf

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_unpack, 0, 2, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_password_get_info, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, hash, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_password_hash, 0, 2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, password, IS_STRING, 0)
	CREX_ARG_TYPE_MASK(0, algo, MAY_BE_STRING|MAY_BE_LONG|MAY_BE_NULL, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_password_needs_rehash, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, hash, IS_STRING, 0)
	CREX_ARG_TYPE_MASK(0, algo, MAY_BE_STRING|MAY_BE_LONG|MAY_BE_NULL, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_password_verify, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, password, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, hash, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_password_algos arginfo_ob_list_handlers

#if defined(CRX_CAN_SUPPORT_PROC_OPEN)
CREX_BEGIN_ARG_INFO_EX(arginfo_proc_open, 0, 0, 3)
	CREX_ARG_TYPE_MASK(0, command, MAY_BE_ARRAY|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO(0, descriptor_spec, IS_ARRAY, 0)
	CREX_ARG_INFO(1, pipes)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, cwd, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, env_vars, IS_ARRAY, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 1, "null")
CREX_END_ARG_INFO()
#endif

#if defined(CRX_CAN_SUPPORT_PROC_OPEN)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_proc_close, 0, 1, IS_LONG, 0)
	CREX_ARG_INFO(0, process)
CREX_END_ARG_INFO()
#endif

#if defined(CRX_CAN_SUPPORT_PROC_OPEN)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_proc_terminate, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, process)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, signal, IS_LONG, 0, "15")
CREX_END_ARG_INFO()
#endif

#if defined(CRX_CAN_SUPPORT_PROC_OPEN)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_proc_get_status, 0, 1, IS_ARRAY, 0)
	CREX_ARG_INFO(0, process)
CREX_END_ARG_INFO()
#endif

#define arginfo_quoted_printable_decode arginfo_base64_encode

#define arginfo_quoted_printable_encode arginfo_base64_encode

#define arginfo_soundex arginfo_base64_encode

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_stream_select, 0, 4, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(1, read, IS_ARRAY, 1)
	CREX_ARG_TYPE_INFO(1, write, IS_ARRAY, 1)
	CREX_ARG_TYPE_INFO(1, except, IS_ARRAY, 1)
	CREX_ARG_TYPE_INFO(0, seconds, IS_LONG, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, microseconds, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_stream_context_create, 0, 0, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, params, IS_ARRAY, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_context_set_params, 0, 2, _IS_BOOL, 0)
	CREX_ARG_INFO(0, context)
	CREX_ARG_TYPE_INFO(0, params, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_context_get_params, 0, 1, IS_ARRAY, 0)
	CREX_ARG_INFO(0, context)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_context_set_option, 0, 2, _IS_BOOL, 0)
	CREX_ARG_INFO(0, context)
	CREX_ARG_TYPE_MASK(0, wrapper_or_options, MAY_BE_ARRAY|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, option_name, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_context_set_options, 0, 2, _IS_BOOL, 0)
	CREX_ARG_INFO(0, context)
	CREX_ARG_TYPE_INFO(0, options, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_context_get_options, 0, 1, IS_ARRAY, 0)
	CREX_ARG_INFO(0, stream_or_context)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_stream_context_get_default, 0, 0, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_stream_context_set_default, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, options, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_stream_filter_prepend, 0, 0, 2)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, filter_name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO(0, params, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_stream_filter_append arginfo_stream_filter_prepend

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_filter_remove, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, stream_filter)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_stream_socket_client, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, address, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, error_code, "null")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, error_message, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_DOUBLE, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "STREAM_CLIENT_CONNECT")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, context, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_stream_socket_server, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, address, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, error_code, "null")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, error_message, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "STREAM_SERVER_BIND | STREAM_SERVER_LISTEN")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, context, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_stream_socket_accept, 0, 0, 1)
	CREX_ARG_INFO(0, socket)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_DOUBLE, 1, "null")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, peer_name, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_stream_socket_get_name, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO(0, socket)
	CREX_ARG_TYPE_INFO(0, remote, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_stream_socket_recvfrom, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO(0, socket)
	CREX_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, address, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_stream_socket_sendto, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_INFO(0, socket)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, address, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_stream_socket_enable_crypto, 0, 2, MAY_BE_LONG|MAY_BE_BOOL)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, enable, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, crypto_method, IS_LONG, 1, "null")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, session_stream, "null")
CREX_END_ARG_INFO()

#if defined(HAVE_SHUTDOWN)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_socket_shutdown, 0, 2, _IS_BOOL, 0)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_SOCKETPAIR)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_stream_socket_pair, 0, 3, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, domain, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, protocol, IS_LONG, 0)
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_stream_copy_to_stream, 0, 2, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_INFO(0, from)
	CREX_ARG_INFO(0, to)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_stream_get_contents, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "-1")
CREX_END_ARG_INFO()

#define arginfo_stream_supports_lock arginfo_rewind

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_set_write_buffer, 0, 2, IS_LONG, 0)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, size, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_set_file_buffer arginfo_stream_set_write_buffer

#define arginfo_stream_set_read_buffer arginfo_stream_set_write_buffer

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_set_blocking, 0, 2, _IS_BOOL, 0)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, enable, _IS_BOOL, 0)
CREX_END_ARG_INFO()

#define arginfo_socket_set_blocking arginfo_stream_set_blocking

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_get_meta_data, 0, 1, IS_ARRAY, 0)
	CREX_ARG_INFO(0, stream)
CREX_END_ARG_INFO()

#define arginfo_socket_get_status arginfo_stream_get_meta_data

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_stream_get_line, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, ending, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

#define arginfo_stream_resolve_include_path arginfo_filetype

#define arginfo_stream_get_wrappers arginfo_ob_list_handlers

#define arginfo_stream_get_transports arginfo_ob_list_handlers

#define arginfo_stream_is_local arginfo_rewind

#define arginfo_stream_isatty arginfo_rewind

#if defined(CRX_WIN32)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sapi_windows_vt100_support, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, enable, _IS_BOOL, 1, "null")
CREX_END_ARG_INFO()
#endif

#define arginfo_stream_set_chunk_size arginfo_stream_set_write_buffer

#if (defined(HAVE_SYS_TIME_H) || defined(CRX_WIN32))
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_set_timeout, 0, 2, _IS_BOOL, 0)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, seconds, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, microseconds, IS_LONG, 0, "0")
CREX_END_ARG_INFO()
#endif

#if (defined(HAVE_SYS_TIME_H) || defined(CRX_WIN32))
#define arginfo_socket_set_timeout arginfo_stream_set_timeout
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gettype, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_get_debug_type arginfo_gettype

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_settype, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(1, var, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, type, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_intval, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, base, IS_LONG, 0, "10")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_floatval, 0, 1, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_doubleval arginfo_floatval

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_boolval, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_strval arginfo_gettype

#define arginfo_is_null arginfo_boolval

#define arginfo_is_resource arginfo_boolval

#define arginfo_is_bool arginfo_boolval

#define arginfo_is_int arginfo_boolval

#define arginfo_is_integer arginfo_boolval

#define arginfo_is_long arginfo_boolval

#define arginfo_is_float arginfo_boolval

#define arginfo_is_double arginfo_boolval

#define arginfo_is_numeric arginfo_boolval

#define arginfo_is_string arginfo_boolval

#define arginfo_is_array arginfo_boolval

#define arginfo_is_object arginfo_boolval

#define arginfo_is_scalar arginfo_boolval

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_is_callable, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, syntax_only, _IS_BOOL, 0, "false")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, callable_name, "null")
CREX_END_ARG_INFO()

#define arginfo_is_iterable arginfo_boolval

#define arginfo_is_countable arginfo_boolval

#if defined(HAVE_GETTIMEOFDAY)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_uniqid, 0, 0, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, prefix, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, more_entropy, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_parse_url, 0, 1, MAY_BE_LONG|MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_NULL|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, url, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, component, IS_LONG, 0, "-1")
CREX_END_ARG_INFO()

#define arginfo_urlencode arginfo_base64_encode

#define arginfo_urldecode arginfo_base64_encode

#define arginfo_rawurlencode arginfo_base64_encode

#define arginfo_rawurldecode arginfo_base64_encode

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_get_headers, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, url, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, associative, _IS_BOOL, 0, "false")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, context, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_bucket_make_writeable, 0, 1, IS_OBJECT, 1)
	CREX_ARG_INFO(0, brigade)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_bucket_prepend, 0, 2, IS_VOID, 0)
	CREX_ARG_INFO(0, brigade)
	CREX_ARG_TYPE_INFO(0, bucket, IS_OBJECT, 0)
CREX_END_ARG_INFO()

#define arginfo_stream_bucket_append arginfo_stream_bucket_prepend

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_bucket_new, 0, 2, IS_OBJECT, 0)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, buffer, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_stream_get_filters arginfo_ob_list_handlers

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_filter_register, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filter_name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, class, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_convert_uuencode arginfo_base64_encode

#define arginfo_convert_uudecode arginfo_hex2bin

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_var_dump, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(0, values, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_var_export, 0, 1, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, return, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

#define arginfo_debug_zval_dump arginfo_var_dump

#define arginfo_serialize arginfo_gettype

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_unserialize, 0, 1, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_memory_get_usage, 0, 0, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, real_usage, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

#define arginfo_memory_get_peak_usage arginfo_memory_get_usage

#define arginfo_memory_reset_peak_usage arginfo_flush

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_version_compare, 0, 2, MAY_BE_LONG|MAY_BE_BOOL)
	CREX_ARG_TYPE_INFO(0, version1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, version2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, operator, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#if defined(CRX_WIN32)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sapi_windows_cp_set, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, codepage, IS_LONG, 0)
CREX_END_ARG_INFO()
#endif

#if defined(CRX_WIN32)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sapi_windows_cp_get, 0, 0, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, kind, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()
#endif

#if defined(CRX_WIN32)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sapi_windows_cp_conv, 0, 3, IS_STRING, 1)
	CREX_ARG_TYPE_MASK(0, in_codepage, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_MASK(0, out_codepage, MAY_BE_LONG|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO(0, subject, IS_STRING, 0)
CREX_END_ARG_INFO()
#endif

#if defined(CRX_WIN32)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sapi_windows_cp_is_utf8, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()
#endif

#if defined(CRX_WIN32)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sapi_windows_set_ctrl_handler, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, handler, IS_CALLABLE, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, add, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()
#endif

#if defined(CRX_WIN32)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sapi_windows_generate_ctrl_event, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, event, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pid, IS_LONG, 0, "0")
CREX_END_ARG_INFO()
#endif


CREX_FUNCTION(set_time_limit);
CREX_FUNCTION(header_register_callback);
CREX_FUNCTION(ob_start);
CREX_FUNCTION(ob_flush);
CREX_FUNCTION(ob_clean);
CREX_FUNCTION(ob_end_flush);
CREX_FUNCTION(ob_end_clean);
CREX_FUNCTION(ob_get_flush);
CREX_FUNCTION(ob_get_clean);
CREX_FUNCTION(ob_get_contents);
CREX_FUNCTION(ob_get_level);
CREX_FUNCTION(ob_get_length);
CREX_FUNCTION(ob_list_handlers);
CREX_FUNCTION(ob_get_status);
CREX_FUNCTION(ob_implicit_flush);
CREX_FUNCTION(output_reset_rewrite_vars);
CREX_FUNCTION(output_add_rewrite_var);
CREX_FUNCTION(stream_wrapper_register);
CREX_FUNCTION(stream_wrapper_unregister);
CREX_FUNCTION(stream_wrapper_restore);
CREX_FUNCTION(array_push);
CREX_FUNCTION(krsort);
CREX_FUNCTION(ksort);
CREX_FUNCTION(count);
CREX_FUNCTION(natsort);
CREX_FUNCTION(natcasesort);
CREX_FUNCTION(asort);
CREX_FUNCTION(arsort);
CREX_FUNCTION(sort);
CREX_FUNCTION(rsort);
CREX_FUNCTION(usort);
CREX_FUNCTION(uasort);
CREX_FUNCTION(uksort);
CREX_FUNCTION(end);
CREX_FUNCTION(prev);
CREX_FUNCTION(next);
CREX_FUNCTION(reset);
CREX_FUNCTION(current);
CREX_FUNCTION(key);
CREX_FUNCTION(min);
CREX_FUNCTION(max);
CREX_FUNCTION(array_walk);
CREX_FUNCTION(array_walk_recursive);
CREX_FUNCTION(in_array);
CREX_FUNCTION(array_search);
CREX_FUNCTION(extract);
CREX_FUNCTION(compact);
CREX_FUNCTION(array_fill);
CREX_FUNCTION(array_fill_keys);
CREX_FUNCTION(range);
CREX_FUNCTION(shuffle);
CREX_FUNCTION(array_pop);
CREX_FUNCTION(array_shift);
CREX_FUNCTION(array_unshift);
CREX_FUNCTION(array_splice);
CREX_FUNCTION(array_slice);
CREX_FUNCTION(array_merge);
CREX_FUNCTION(array_merge_recursive);
CREX_FUNCTION(array_replace);
CREX_FUNCTION(array_replace_recursive);
CREX_FUNCTION(array_keys);
CREX_FUNCTION(array_key_first);
CREX_FUNCTION(array_key_last);
CREX_FUNCTION(array_values);
CREX_FUNCTION(array_count_values);
CREX_FUNCTION(array_column);
CREX_FUNCTION(array_reverse);
CREX_FUNCTION(array_pad);
CREX_FUNCTION(array_flip);
CREX_FUNCTION(array_change_key_case);
CREX_FUNCTION(array_unique);
CREX_FUNCTION(array_intersect_key);
CREX_FUNCTION(array_intersect_ukey);
CREX_FUNCTION(array_intersect);
CREX_FUNCTION(array_uintersect);
CREX_FUNCTION(array_intersect_assoc);
CREX_FUNCTION(array_uintersect_assoc);
CREX_FUNCTION(array_intersect_uassoc);
CREX_FUNCTION(array_uintersect_uassoc);
CREX_FUNCTION(array_diff_key);
CREX_FUNCTION(array_diff_ukey);
CREX_FUNCTION(array_diff);
CREX_FUNCTION(array_udiff);
CREX_FUNCTION(array_diff_assoc);
CREX_FUNCTION(array_diff_uassoc);
CREX_FUNCTION(array_udiff_assoc);
CREX_FUNCTION(array_udiff_uassoc);
CREX_FUNCTION(array_multisort);
CREX_FUNCTION(array_rand);
CREX_FUNCTION(array_sum);
CREX_FUNCTION(array_product);
CREX_FUNCTION(array_reduce);
CREX_FUNCTION(array_filter);
CREX_FUNCTION(array_map);
CREX_FUNCTION(array_key_exists);
CREX_FUNCTION(array_chunk);
CREX_FUNCTION(array_combine);
CREX_FUNCTION(array_is_list);
CREX_FUNCTION(base64_encode);
CREX_FUNCTION(base64_decode);
CREX_FUNCTION(constant);
CREX_FUNCTION(ip2long);
CREX_FUNCTION(long2ip);
CREX_FUNCTION(getenv);
#if defined(HAVE_PUTENV)
CREX_FUNCTION(putenv);
#endif
CREX_FUNCTION(getopt);
CREX_FUNCTION(flush);
CREX_FUNCTION(sleep);
CREX_FUNCTION(usleep);
#if defined(HAVE_NANOSLEEP)
CREX_FUNCTION(time_nanosleep);
#endif
#if defined(HAVE_NANOSLEEP)
CREX_FUNCTION(time_sleep_until);
#endif
CREX_FUNCTION(get_current_user);
CREX_FUNCTION(get_cfg_var);
CREX_FUNCTION(error_log);
CREX_FUNCTION(error_get_last);
CREX_FUNCTION(error_clear_last);
CREX_FUNCTION(call_user_func);
CREX_FUNCTION(call_user_func_array);
CREX_FUNCTION(forward_static_call);
CREX_FUNCTION(forward_static_call_array);
CREX_FUNCTION(register_shutdown_function);
CREX_FUNCTION(highlight_file);
CREX_FUNCTION(crx_strip_whitespace);
CREX_FUNCTION(highlight_string);
CREX_FUNCTION(ini_get);
CREX_FUNCTION(ini_get_all);
CREX_FUNCTION(ini_set);
CREX_FUNCTION(ini_restore);
CREX_FUNCTION(ini_parse_quantity);
CREX_FUNCTION(set_include_path);
CREX_FUNCTION(get_include_path);
CREX_FUNCTION(print_r);
CREX_FUNCTION(connection_aborted);
CREX_FUNCTION(connection_status);
CREX_FUNCTION(ignore_user_abort);
#if defined(HAVE_GETSERVBYNAME)
CREX_FUNCTION(getservbyname);
#endif
#if defined(HAVE_GETSERVBYPORT)
CREX_FUNCTION(getservbyport);
#endif
#if defined(HAVE_GETPROTOBYNAME)
CREX_FUNCTION(getprotobyname);
#endif
#if defined(HAVE_GETPROTOBYNUMBER)
CREX_FUNCTION(getprotobynumber);
#endif
CREX_FUNCTION(register_tick_function);
CREX_FUNCTION(unregister_tick_function);
CREX_FUNCTION(is_uploaded_file);
CREX_FUNCTION(move_uploaded_file);
CREX_FUNCTION(parse_ini_file);
CREX_FUNCTION(parse_ini_string);
#if CREX_DEBUG
CREX_FUNCTION(config_get_hash);
#endif
#if defined(HAVE_GETLOADAVG)
CREX_FUNCTION(sys_getloadavg);
#endif
CREX_FUNCTION(get_browser);
CREX_FUNCTION(crc32);
CREX_FUNCTION(crypt);
#if defined(HAVE_STRPTIME)
CREX_FUNCTION(strptime);
#endif
#if defined(HAVE_GETHOSTNAME)
CREX_FUNCTION(gethostname);
#endif
CREX_FUNCTION(gethostbyaddr);
CREX_FUNCTION(gethostbyname);
CREX_FUNCTION(gethostbynamel);
#if (defined(CRX_WIN32) || defined(HAVE_DNS_SEARCH_FUNC))
CREX_FUNCTION(dns_check_record);
#endif
#if (defined(CRX_WIN32) || defined(HAVE_DNS_SEARCH_FUNC))
CREX_FUNCTION(dns_get_record);
#endif
#if (defined(CRX_WIN32) || defined(HAVE_DNS_SEARCH_FUNC))
CREX_FUNCTION(dns_get_mx);
#endif
#if (defined(CRX_WIN32) || HAVE_GETIFADDRS || defined(__PASE__))
CREX_FUNCTION(net_get_interfaces);
#endif
#if defined(HAVE_FTOK)
CREX_FUNCTION(ftok);
#endif
CREX_FUNCTION(hrtime);
CREX_FUNCTION(md5);
CREX_FUNCTION(md5_file);
CREX_FUNCTION(getmyuid);
CREX_FUNCTION(getmygid);
CREX_FUNCTION(getmypid);
CREX_FUNCTION(getmyinode);
CREX_FUNCTION(getlastmod);
CREX_FUNCTION(sha1);
CREX_FUNCTION(sha1_file);
#if defined(HAVE_SYSLOG_H)
CREX_FUNCTION(openlog);
#endif
#if defined(HAVE_SYSLOG_H)
CREX_FUNCTION(closelog);
#endif
#if defined(HAVE_SYSLOG_H)
CREX_FUNCTION(syslog);
#endif
#if defined(HAVE_INET_NTOP)
CREX_FUNCTION(inet_ntop);
#endif
#if defined(HAVE_INET_PTON)
CREX_FUNCTION(inet_pton);
#endif
CREX_FUNCTION(metaphone);
CREX_FUNCTION(header);
CREX_FUNCTION(header_remove);
CREX_FUNCTION(setrawcookie);
CREX_FUNCTION(setcookie);
CREX_FUNCTION(http_response_code);
CREX_FUNCTION(headers_sent);
CREX_FUNCTION(headers_list);
CREX_FUNCTION(htmlspecialchars);
CREX_FUNCTION(htmlspecialchars_decode);
CREX_FUNCTION(html_entity_decode);
CREX_FUNCTION(htmlentities);
CREX_FUNCTION(get_html_translation_table);
CREX_FUNCTION(assert);
CREX_FUNCTION(assert_options);
CREX_FUNCTION(bin2hex);
CREX_FUNCTION(hex2bin);
CREX_FUNCTION(strspn);
CREX_FUNCTION(strcspn);
#if defined(HAVE_NL_LANGINFO)
CREX_FUNCTION(nl_langinfo);
#endif
CREX_FUNCTION(strcoll);
CREX_FUNCTION(trim);
CREX_FUNCTION(rtrim);
CREX_FUNCTION(ltrim);
CREX_FUNCTION(wordwrap);
CREX_FUNCTION(explode);
CREX_FUNCTION(implode);
CREX_FUNCTION(strtok);
CREX_FUNCTION(strtoupper);
CREX_FUNCTION(strtolower);
CREX_FUNCTION(str_increment);
CREX_FUNCTION(str_decrement);
CREX_FUNCTION(basename);
CREX_FUNCTION(dirname);
CREX_FUNCTION(pathinfo);
CREX_FUNCTION(stristr);
CREX_FUNCTION(strstr);
CREX_FUNCTION(strpos);
CREX_FUNCTION(stripos);
CREX_FUNCTION(strrpos);
CREX_FUNCTION(strripos);
CREX_FUNCTION(strrchr);
CREX_FUNCTION(str_contains);
CREX_FUNCTION(str_starts_with);
CREX_FUNCTION(str_ends_with);
CREX_FUNCTION(chunk_split);
CREX_FUNCTION(substr);
CREX_FUNCTION(substr_replace);
CREX_FUNCTION(quotemeta);
CREX_FUNCTION(ord);
CREX_FUNCTION(chr);
CREX_FUNCTION(ucfirst);
CREX_FUNCTION(lcfirst);
CREX_FUNCTION(ucwords);
CREX_FUNCTION(strtr);
CREX_FUNCTION(strrev);
CREX_FUNCTION(similar_text);
CREX_FUNCTION(addcslashes);
CREX_FUNCTION(addslashes);
CREX_FUNCTION(stripcslashes);
CREX_FUNCTION(stripslashes);
CREX_FUNCTION(str_replace);
CREX_FUNCTION(str_ireplace);
CREX_FUNCTION(hebrev);
CREX_FUNCTION(nl2br);
CREX_FUNCTION(strip_tags);
CREX_FUNCTION(setlocale);
CREX_FUNCTION(parse_str);
CREX_FUNCTION(str_getcsv);
CREX_FUNCTION(str_repeat);
CREX_FUNCTION(count_chars);
CREX_FUNCTION(strnatcmp);
CREX_FUNCTION(localeconv);
CREX_FUNCTION(strnatcasecmp);
CREX_FUNCTION(substr_count);
CREX_FUNCTION(str_pad);
CREX_FUNCTION(sscanf);
CREX_FUNCTION(str_rot13);
CREX_FUNCTION(str_shuffle);
CREX_FUNCTION(str_word_count);
CREX_FUNCTION(str_split);
CREX_FUNCTION(strpbrk);
CREX_FUNCTION(substr_compare);
CREX_FUNCTION(utf8_encode);
CREX_FUNCTION(utf8_decode);
CREX_FUNCTION(opendir);
CREX_FUNCTION(dir);
CREX_FUNCTION(closedir);
CREX_FUNCTION(chdir);
#if (defined(HAVE_CHROOT) && !defined(ZTS) && defined(ENABLE_CHROOT_FUNC))
CREX_FUNCTION(chroot);
#endif
CREX_FUNCTION(getcwd);
CREX_FUNCTION(rewinddir);
CREX_FUNCTION(readdir);
CREX_FUNCTION(scandir);
#if defined(HAVE_GLOB)
CREX_FUNCTION(glob);
#endif
CREX_FUNCTION(exec);
CREX_FUNCTION(system);
CREX_FUNCTION(passthru);
CREX_FUNCTION(escapeshellcmd);
CREX_FUNCTION(escapeshellarg);
CREX_FUNCTION(shell_exec);
#if defined(HAVE_NICE)
CREX_FUNCTION(proc_nice);
#endif
CREX_FUNCTION(flock);
CREX_FUNCTION(get_meta_tags);
CREX_FUNCTION(pclose);
CREX_FUNCTION(popen);
CREX_FUNCTION(readfile);
CREX_FUNCTION(rewind);
CREX_FUNCTION(rmdir);
CREX_FUNCTION(umask);
CREX_FUNCTION(fclose);
CREX_FUNCTION(feof);
CREX_FUNCTION(fgetc);
CREX_FUNCTION(fgets);
CREX_FUNCTION(fread);
CREX_FUNCTION(fopen);
CREX_FUNCTION(fscanf);
CREX_FUNCTION(fpassthru);
CREX_FUNCTION(ftruncate);
CREX_FUNCTION(fstat);
CREX_FUNCTION(fseek);
CREX_FUNCTION(ftell);
CREX_FUNCTION(fflush);
CREX_FUNCTION(fsync);
CREX_FUNCTION(fdatasync);
CREX_FUNCTION(fwrite);
CREX_FUNCTION(mkdir);
CREX_FUNCTION(rename);
CREX_FUNCTION(copy);
CREX_FUNCTION(tempnam);
CREX_FUNCTION(tmpfile);
CREX_FUNCTION(file);
CREX_FUNCTION(file_get_contents);
CREX_FUNCTION(unlink);
CREX_FUNCTION(file_put_contents);
CREX_FUNCTION(fputcsv);
CREX_FUNCTION(fgetcsv);
CREX_FUNCTION(realpath);
#if defined(HAVE_FNMATCH)
CREX_FUNCTION(fnmatch);
#endif
CREX_FUNCTION(sys_get_temp_dir);
CREX_FUNCTION(fileatime);
CREX_FUNCTION(filectime);
CREX_FUNCTION(filegroup);
CREX_FUNCTION(fileinode);
CREX_FUNCTION(filemtime);
CREX_FUNCTION(fileowner);
CREX_FUNCTION(fileperms);
CREX_FUNCTION(filesize);
CREX_FUNCTION(filetype);
CREX_FUNCTION(file_exists);
CREX_FUNCTION(is_writable);
CREX_FUNCTION(is_readable);
CREX_FUNCTION(is_executable);
CREX_FUNCTION(is_file);
CREX_FUNCTION(is_dir);
CREX_FUNCTION(is_link);
CREX_FUNCTION(stat);
CREX_FUNCTION(lstat);
CREX_FUNCTION(chown);
CREX_FUNCTION(chgrp);
#if defined(HAVE_LCHOWN)
CREX_FUNCTION(lchown);
#endif
#if defined(HAVE_LCHOWN)
CREX_FUNCTION(lchgrp);
#endif
CREX_FUNCTION(chmod);
#if defined(HAVE_UTIME)
CREX_FUNCTION(touch);
#endif
CREX_FUNCTION(clearstatcache);
CREX_FUNCTION(disk_total_space);
CREX_FUNCTION(disk_free_space);
CREX_FUNCTION(realpath_cache_get);
CREX_FUNCTION(realpath_cache_size);
CREX_FUNCTION(sprintf);
CREX_FUNCTION(printf);
CREX_FUNCTION(vprintf);
CREX_FUNCTION(vsprintf);
CREX_FUNCTION(fprintf);
CREX_FUNCTION(vfprintf);
CREX_FUNCTION(fsockopen);
CREX_FUNCTION(pfsockopen);
CREX_FUNCTION(http_build_query);
CREX_FUNCTION(image_type_to_mime_type);
CREX_FUNCTION(image_type_to_extension);
CREX_FUNCTION(getimagesize);
CREX_FUNCTION(getimagesizefromstring);
CREX_FUNCTION(crxinfo);
CREX_FUNCTION(crxversion);
CREX_FUNCTION(crxcredits);
CREX_FUNCTION(crx_sapi_name);
CREX_FUNCTION(crx_uname);
CREX_FUNCTION(crx_ini_scanned_files);
CREX_FUNCTION(crx_ini_loaded_file);
CREX_FUNCTION(iptcembed);
CREX_FUNCTION(iptcparse);
CREX_FUNCTION(levenshtein);
#if (defined(HAVE_SYMLINK) || defined(CRX_WIN32))
CREX_FUNCTION(readlink);
#endif
#if (defined(HAVE_SYMLINK) || defined(CRX_WIN32))
CREX_FUNCTION(linkinfo);
#endif
#if (defined(HAVE_SYMLINK) || defined(CRX_WIN32))
CREX_FUNCTION(symlink);
#endif
#if (defined(HAVE_SYMLINK) || defined(CRX_WIN32))
CREX_FUNCTION(link);
#endif
CREX_FUNCTION(mail);
CREX_FUNCTION(abs);
CREX_FUNCTION(ceil);
CREX_FUNCTION(floor);
CREX_FUNCTION(round);
CREX_FUNCTION(sin);
CREX_FUNCTION(cos);
CREX_FUNCTION(tan);
CREX_FUNCTION(asin);
CREX_FUNCTION(acos);
CREX_FUNCTION(atan);
CREX_FUNCTION(atanh);
CREX_FUNCTION(atan2);
CREX_FUNCTION(sinh);
CREX_FUNCTION(cosh);
CREX_FUNCTION(tanh);
CREX_FUNCTION(asinh);
CREX_FUNCTION(acosh);
CREX_FUNCTION(expm1);
CREX_FUNCTION(log1p);
CREX_FUNCTION(pi);
CREX_FUNCTION(is_finite);
CREX_FUNCTION(is_nan);
CREX_FUNCTION(intdiv);
CREX_FUNCTION(is_infinite);
CREX_FUNCTION(pow);
CREX_FUNCTION(exp);
CREX_FUNCTION(log);
CREX_FUNCTION(log10);
CREX_FUNCTION(sqrt);
CREX_FUNCTION(hypot);
CREX_FUNCTION(deg2rad);
CREX_FUNCTION(rad2deg);
CREX_FUNCTION(bindec);
CREX_FUNCTION(hexdec);
CREX_FUNCTION(octdec);
CREX_FUNCTION(decbin);
CREX_FUNCTION(decoct);
CREX_FUNCTION(dechex);
CREX_FUNCTION(base_convert);
CREX_FUNCTION(number_format);
CREX_FUNCTION(fmod);
CREX_FUNCTION(fdiv);
#if defined(HAVE_GETTIMEOFDAY)
CREX_FUNCTION(microtime);
#endif
#if defined(HAVE_GETTIMEOFDAY)
CREX_FUNCTION(gettimeofday);
#endif
#if defined(HAVE_GETRUSAGE)
CREX_FUNCTION(getrusage);
#endif
CREX_FUNCTION(pack);
CREX_FUNCTION(unpack);
CREX_FUNCTION(password_get_info);
CREX_FUNCTION(password_hash);
CREX_FUNCTION(password_needs_rehash);
CREX_FUNCTION(password_verify);
CREX_FUNCTION(password_algos);
#if defined(CRX_CAN_SUPPORT_PROC_OPEN)
CREX_FUNCTION(proc_open);
#endif
#if defined(CRX_CAN_SUPPORT_PROC_OPEN)
CREX_FUNCTION(proc_close);
#endif
#if defined(CRX_CAN_SUPPORT_PROC_OPEN)
CREX_FUNCTION(proc_terminate);
#endif
#if defined(CRX_CAN_SUPPORT_PROC_OPEN)
CREX_FUNCTION(proc_get_status);
#endif
CREX_FUNCTION(quoted_printable_decode);
CREX_FUNCTION(quoted_printable_encode);
CREX_FUNCTION(soundex);
CREX_FUNCTION(stream_select);
CREX_FUNCTION(stream_context_create);
CREX_FUNCTION(stream_context_set_params);
CREX_FUNCTION(stream_context_get_params);
CREX_FUNCTION(stream_context_set_option);
CREX_FUNCTION(stream_context_set_options);
CREX_FUNCTION(stream_context_get_options);
CREX_FUNCTION(stream_context_get_default);
CREX_FUNCTION(stream_context_set_default);
CREX_FUNCTION(stream_filter_prepend);
CREX_FUNCTION(stream_filter_append);
CREX_FUNCTION(stream_filter_remove);
CREX_FUNCTION(stream_socket_client);
CREX_FUNCTION(stream_socket_server);
CREX_FUNCTION(stream_socket_accept);
CREX_FUNCTION(stream_socket_get_name);
CREX_FUNCTION(stream_socket_recvfrom);
CREX_FUNCTION(stream_socket_sendto);
CREX_FUNCTION(stream_socket_enable_crypto);
#if defined(HAVE_SHUTDOWN)
CREX_FUNCTION(stream_socket_shutdown);
#endif
#if defined(HAVE_SOCKETPAIR)
CREX_FUNCTION(stream_socket_pair);
#endif
CREX_FUNCTION(stream_copy_to_stream);
CREX_FUNCTION(stream_get_contents);
CREX_FUNCTION(stream_supports_lock);
CREX_FUNCTION(stream_set_write_buffer);
CREX_FUNCTION(stream_set_read_buffer);
CREX_FUNCTION(stream_set_blocking);
CREX_FUNCTION(stream_get_meta_data);
CREX_FUNCTION(stream_get_line);
CREX_FUNCTION(stream_resolve_include_path);
CREX_FUNCTION(stream_get_wrappers);
CREX_FUNCTION(stream_get_transports);
CREX_FUNCTION(stream_is_local);
CREX_FUNCTION(stream_isatty);
#if defined(CRX_WIN32)
CREX_FUNCTION(sapi_windows_vt100_support);
#endif
CREX_FUNCTION(stream_set_chunk_size);
#if (defined(HAVE_SYS_TIME_H) || defined(CRX_WIN32))
CREX_FUNCTION(stream_set_timeout);
#endif
CREX_FUNCTION(gettype);
CREX_FUNCTION(get_debug_type);
CREX_FUNCTION(settype);
CREX_FUNCTION(intval);
CREX_FUNCTION(floatval);
CREX_FUNCTION(boolval);
CREX_FUNCTION(strval);
CREX_FUNCTION(is_null);
CREX_FUNCTION(is_resource);
CREX_FUNCTION(is_bool);
CREX_FUNCTION(is_int);
CREX_FUNCTION(is_float);
CREX_FUNCTION(is_numeric);
CREX_FUNCTION(is_string);
CREX_FUNCTION(is_array);
CREX_FUNCTION(is_object);
CREX_FUNCTION(is_scalar);
CREX_FUNCTION(is_callable);
CREX_FUNCTION(is_iterable);
CREX_FUNCTION(is_countable);
#if defined(HAVE_GETTIMEOFDAY)
CREX_FUNCTION(uniqid);
#endif
CREX_FUNCTION(parse_url);
CREX_FUNCTION(urlencode);
CREX_FUNCTION(urldecode);
CREX_FUNCTION(rawurlencode);
CREX_FUNCTION(rawurldecode);
CREX_FUNCTION(get_headers);
CREX_FUNCTION(stream_bucket_make_writeable);
CREX_FUNCTION(stream_bucket_prepend);
CREX_FUNCTION(stream_bucket_append);
CREX_FUNCTION(stream_bucket_new);
CREX_FUNCTION(stream_get_filters);
CREX_FUNCTION(stream_filter_register);
CREX_FUNCTION(convert_uuencode);
CREX_FUNCTION(convert_uudecode);
CREX_FUNCTION(var_dump);
CREX_FUNCTION(var_export);
CREX_FUNCTION(debug_zval_dump);
CREX_FUNCTION(serialize);
CREX_FUNCTION(unserialize);
CREX_FUNCTION(memory_get_usage);
CREX_FUNCTION(memory_get_peak_usage);
CREX_FUNCTION(memory_reset_peak_usage);
CREX_FUNCTION(version_compare);
#if defined(CRX_WIN32)
CREX_FUNCTION(sapi_windows_cp_set);
#endif
#if defined(CRX_WIN32)
CREX_FUNCTION(sapi_windows_cp_get);
#endif
#if defined(CRX_WIN32)
CREX_FUNCTION(sapi_windows_cp_conv);
#endif
#if defined(CRX_WIN32)
CREX_FUNCTION(sapi_windows_cp_is_utf8);
#endif
#if defined(CRX_WIN32)
CREX_FUNCTION(sapi_windows_set_ctrl_handler);
#endif
#if defined(CRX_WIN32)
CREX_FUNCTION(sapi_windows_generate_ctrl_event);
#endif


static const crex_function_entry ext_functions[] = {
	CREX_FE(set_time_limit, arginfo_set_time_limit)
	CREX_FE(header_register_callback, arginfo_header_register_callback)
	CREX_FE(ob_start, arginfo_ob_start)
	CREX_FE(ob_flush, arginfo_ob_flush)
	CREX_FE(ob_clean, arginfo_ob_clean)
	CREX_FE(ob_end_flush, arginfo_ob_end_flush)
	CREX_FE(ob_end_clean, arginfo_ob_end_clean)
	CREX_FE(ob_get_flush, arginfo_ob_get_flush)
	CREX_FE(ob_get_clean, arginfo_ob_get_clean)
	CREX_FE(ob_get_contents, arginfo_ob_get_contents)
	CREX_FE(ob_get_level, arginfo_ob_get_level)
	CREX_FE(ob_get_length, arginfo_ob_get_length)
	CREX_FE(ob_list_handlers, arginfo_ob_list_handlers)
	CREX_FE(ob_get_status, arginfo_ob_get_status)
	CREX_FE(ob_implicit_flush, arginfo_ob_implicit_flush)
	CREX_FE(output_reset_rewrite_vars, arginfo_output_reset_rewrite_vars)
	CREX_FE(output_add_rewrite_var, arginfo_output_add_rewrite_var)
	CREX_FE(stream_wrapper_register, arginfo_stream_wrapper_register)
	CREX_FALIAS(stream_register_wrapper, stream_wrapper_register, arginfo_stream_register_wrapper)
	CREX_FE(stream_wrapper_unregister, arginfo_stream_wrapper_unregister)
	CREX_FE(stream_wrapper_restore, arginfo_stream_wrapper_restore)
	CREX_FE(array_push, arginfo_array_push)
	CREX_FE(krsort, arginfo_krsort)
	CREX_FE(ksort, arginfo_ksort)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(count, arginfo_count)
	CREX_FALIAS(sizeof, count, arginfo_sizeof)
	CREX_FE(natsort, arginfo_natsort)
	CREX_FE(natcasesort, arginfo_natcasesort)
	CREX_FE(asort, arginfo_asort)
	CREX_FE(arsort, arginfo_arsort)
	CREX_FE(sort, arginfo_sort)
	CREX_FE(rsort, arginfo_rsort)
	CREX_FE(usort, arginfo_usort)
	CREX_FE(uasort, arginfo_uasort)
	CREX_FE(uksort, arginfo_uksort)
	CREX_FE(end, arginfo_end)
	CREX_FE(prev, arginfo_prev)
	CREX_FE(next, arginfo_next)
	CREX_FE(reset, arginfo_reset)
	CREX_FE(current, arginfo_current)
	CREX_FALIAS(pos, current, arginfo_pos)
	CREX_FE(key, arginfo_key)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(min, arginfo_min)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(max, arginfo_max)
	CREX_FE(array_walk, arginfo_array_walk)
	CREX_FE(array_walk_recursive, arginfo_array_walk_recursive)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(in_array, arginfo_in_array)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_search, arginfo_array_search)
	CREX_FE(extract, arginfo_extract)
	CREX_FE(compact, arginfo_compact)
	CREX_FE(array_fill, arginfo_array_fill)
	CREX_FE(array_fill_keys, arginfo_array_fill_keys)
	CREX_FE(range, arginfo_range)
	CREX_FE(shuffle, arginfo_shuffle)
	CREX_FE(array_pop, arginfo_array_pop)
	CREX_FE(array_shift, arginfo_array_shift)
	CREX_FE(array_unshift, arginfo_array_unshift)
	CREX_FE(array_splice, arginfo_array_splice)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_slice, arginfo_array_slice)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_merge, arginfo_array_merge)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_merge_recursive, arginfo_array_merge_recursive)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_replace, arginfo_array_replace)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_replace_recursive, arginfo_array_replace_recursive)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_keys, arginfo_array_keys)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_key_first, arginfo_array_key_first)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_key_last, arginfo_array_key_last)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_values, arginfo_array_values)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_count_values, arginfo_array_count_values)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_column, arginfo_array_column)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_reverse, arginfo_array_reverse)
	CREX_FE(array_pad, arginfo_array_pad)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_flip, arginfo_array_flip)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_change_key_case, arginfo_array_change_key_case)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_unique, arginfo_array_unique)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_intersect_key, arginfo_array_intersect_key)
	CREX_FE(array_intersect_ukey, arginfo_array_intersect_ukey)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_intersect, arginfo_array_intersect)
	CREX_FE(array_uintersect, arginfo_array_uintersect)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_intersect_assoc, arginfo_array_intersect_assoc)
	CREX_FE(array_uintersect_assoc, arginfo_array_uintersect_assoc)
	CREX_FE(array_intersect_uassoc, arginfo_array_intersect_uassoc)
	CREX_FE(array_uintersect_uassoc, arginfo_array_uintersect_uassoc)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_diff_key, arginfo_array_diff_key)
	CREX_FE(array_diff_ukey, arginfo_array_diff_ukey)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_diff, arginfo_array_diff)
	CREX_FE(array_udiff, arginfo_array_udiff)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_diff_assoc, arginfo_array_diff_assoc)
	CREX_FE(array_diff_uassoc, arginfo_array_diff_uassoc)
	CREX_FE(array_udiff_assoc, arginfo_array_udiff_assoc)
	CREX_FE(array_udiff_uassoc, arginfo_array_udiff_uassoc)
	CREX_FE(array_multisort, arginfo_array_multisort)
	CREX_FE(array_rand, arginfo_array_rand)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_sum, arginfo_array_sum)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_product, arginfo_array_product)
	CREX_FE(array_reduce, arginfo_array_reduce)
	CREX_FE(array_filter, arginfo_array_filter)
	CREX_FE(array_map, arginfo_array_map)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_key_exists, arginfo_array_key_exists)
	CREX_FALIAS(key_exists, array_key_exists, arginfo_key_exists)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_chunk, arginfo_array_chunk)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_combine, arginfo_array_combine)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(array_is_list, arginfo_array_is_list)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(base64_encode, arginfo_base64_encode)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(base64_decode, arginfo_base64_decode)
	CREX_FE(constant, arginfo_constant)
	CREX_FE(ip2long, arginfo_ip2long)
	CREX_FE(long2ip, arginfo_long2ip)
	CREX_FE(getenv, arginfo_getenv)
#if defined(HAVE_PUTENV)
	CREX_FE(putenv, arginfo_putenv)
#endif
	CREX_FE(getopt, arginfo_getopt)
	CREX_FE(flush, arginfo_flush)
	CREX_FE(sleep, arginfo_sleep)
	CREX_FE(usleep, arginfo_usleep)
#if defined(HAVE_NANOSLEEP)
	CREX_FE(time_nanosleep, arginfo_time_nanosleep)
#endif
#if defined(HAVE_NANOSLEEP)
	CREX_FE(time_sleep_until, arginfo_time_sleep_until)
#endif
	CREX_FE(get_current_user, arginfo_get_current_user)
	CREX_FE(get_cfg_var, arginfo_get_cfg_var)
	CREX_FE(error_log, arginfo_error_log)
	CREX_FE(error_get_last, arginfo_error_get_last)
	CREX_FE(error_clear_last, arginfo_error_clear_last)
	CREX_FE(call_user_func, arginfo_call_user_func)
	CREX_FE(call_user_func_array, arginfo_call_user_func_array)
	CREX_FE(forward_static_call, arginfo_forward_static_call)
	CREX_FE(forward_static_call_array, arginfo_forward_static_call_array)
	CREX_FE(register_shutdown_function, arginfo_register_shutdown_function)
	CREX_FE(highlight_file, arginfo_highlight_file)
	CREX_FALIAS(show_source, highlight_file, arginfo_show_source)
	CREX_FE(crx_strip_whitespace, arginfo_crx_strip_whitespace)
	CREX_FE(highlight_string, arginfo_highlight_string)
	CREX_FE(ini_get, arginfo_ini_get)
	CREX_FE(ini_get_all, arginfo_ini_get_all)
	CREX_FE(ini_set, arginfo_ini_set)
	CREX_FALIAS(ini_alter, ini_set, arginfo_ini_alter)
	CREX_FE(ini_restore, arginfo_ini_restore)
	CREX_FE(ini_parse_quantity, arginfo_ini_parse_quantity)
	CREX_FE(set_include_path, arginfo_set_include_path)
	CREX_FE(get_include_path, arginfo_get_include_path)
	CREX_FE(print_r, arginfo_print_r)
	CREX_FE(connection_aborted, arginfo_connection_aborted)
	CREX_FE(connection_status, arginfo_connection_status)
	CREX_FE(ignore_user_abort, arginfo_ignore_user_abort)
#if defined(HAVE_GETSERVBYNAME)
	CREX_FE(getservbyname, arginfo_getservbyname)
#endif
#if defined(HAVE_GETSERVBYPORT)
	CREX_FE(getservbyport, arginfo_getservbyport)
#endif
#if defined(HAVE_GETPROTOBYNAME)
	CREX_FE(getprotobyname, arginfo_getprotobyname)
#endif
#if defined(HAVE_GETPROTOBYNUMBER)
	CREX_FE(getprotobynumber, arginfo_getprotobynumber)
#endif
	CREX_FE(register_tick_function, arginfo_register_tick_function)
	CREX_FE(unregister_tick_function, arginfo_unregister_tick_function)
	CREX_FE(is_uploaded_file, arginfo_is_uploaded_file)
	CREX_FE(move_uploaded_file, arginfo_move_uploaded_file)
	CREX_FE(parse_ini_file, arginfo_parse_ini_file)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(parse_ini_string, arginfo_parse_ini_string)
#if CREX_DEBUG
	CREX_FE(config_get_hash, arginfo_config_get_hash)
#endif
#if defined(HAVE_GETLOADAVG)
	CREX_FE(sys_getloadavg, arginfo_sys_getloadavg)
#endif
	CREX_FE(get_browser, arginfo_get_browser)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(crc32, arginfo_crc32)
	CREX_FE(crypt, arginfo_crypt)
#if defined(HAVE_STRPTIME)
	CREX_DEP_FE(strptime, arginfo_strptime)
#endif
#if defined(HAVE_GETHOSTNAME)
	CREX_FE(gethostname, arginfo_gethostname)
#endif
	CREX_FE(gethostbyaddr, arginfo_gethostbyaddr)
	CREX_FE(gethostbyname, arginfo_gethostbyname)
	CREX_FE(gethostbynamel, arginfo_gethostbynamel)
#if (defined(CRX_WIN32) || defined(HAVE_DNS_SEARCH_FUNC))
	CREX_FE(dns_check_record, arginfo_dns_check_record)
#endif
#if (defined(CRX_WIN32) || defined(HAVE_DNS_SEARCH_FUNC))
	CREX_FALIAS(checkdnsrr, dns_check_record, arginfo_checkdnsrr)
#endif
#if (defined(CRX_WIN32) || defined(HAVE_DNS_SEARCH_FUNC))
	CREX_FE(dns_get_record, arginfo_dns_get_record)
#endif
#if (defined(CRX_WIN32) || defined(HAVE_DNS_SEARCH_FUNC))
	CREX_FE(dns_get_mx, arginfo_dns_get_mx)
#endif
#if (defined(CRX_WIN32) || defined(HAVE_DNS_SEARCH_FUNC))
	CREX_FALIAS(getmxrr, dns_get_mx, arginfo_getmxrr)
#endif
#if (defined(CRX_WIN32) || HAVE_GETIFADDRS || defined(__PASE__))
	CREX_FE(net_get_interfaces, arginfo_net_get_interfaces)
#endif
#if defined(HAVE_FTOK)
	CREX_FE(ftok, arginfo_ftok)
#endif
	CREX_FE(hrtime, arginfo_hrtime)
	CREX_FE(md5, arginfo_md5)
	CREX_FE(md5_file, arginfo_md5_file)
	CREX_FE(getmyuid, arginfo_getmyuid)
	CREX_FE(getmygid, arginfo_getmygid)
	CREX_FE(getmypid, arginfo_getmypid)
	CREX_FE(getmyinode, arginfo_getmyinode)
	CREX_FE(getlastmod, arginfo_getlastmod)
	CREX_FE(sha1, arginfo_sha1)
	CREX_FE(sha1_file, arginfo_sha1_file)
#if defined(HAVE_SYSLOG_H)
	CREX_FE(openlog, arginfo_openlog)
#endif
#if defined(HAVE_SYSLOG_H)
	CREX_FE(closelog, arginfo_closelog)
#endif
#if defined(HAVE_SYSLOG_H)
	CREX_FE(syslog, arginfo_syslog)
#endif
#if defined(HAVE_INET_NTOP)
	CREX_FE(inet_ntop, arginfo_inet_ntop)
#endif
#if defined(HAVE_INET_PTON)
	CREX_FE(inet_pton, arginfo_inet_pton)
#endif
	CREX_FE(metaphone, arginfo_metaphone)
	CREX_FE(header, arginfo_header)
	CREX_FE(header_remove, arginfo_header_remove)
	CREX_FE(setrawcookie, arginfo_setrawcookie)
	CREX_FE(setcookie, arginfo_setcookie)
	CREX_FE(http_response_code, arginfo_http_response_code)
	CREX_FE(headers_sent, arginfo_headers_sent)
	CREX_FE(headers_list, arginfo_headers_list)
	CREX_FE(htmlspecialchars, arginfo_htmlspecialchars)
	CREX_FE(htmlspecialchars_decode, arginfo_htmlspecialchars_decode)
	CREX_FE(html_entity_decode, arginfo_html_entity_decode)
	CREX_FE(htmlentities, arginfo_htmlentities)
	CREX_FE(get_html_translation_table, arginfo_get_html_translation_table)
	CREX_FE(assert, arginfo_assert)
	CREX_DEP_FE(assert_options, arginfo_assert_options)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(bin2hex, arginfo_bin2hex)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(hex2bin, arginfo_hex2bin)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(strspn, arginfo_strspn)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(strcspn, arginfo_strcspn)
#if defined(HAVE_NL_LANGINFO)
	CREX_FE(nl_langinfo, arginfo_nl_langinfo)
#endif
	CREX_FE(strcoll, arginfo_strcoll)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(trim, arginfo_trim)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(rtrim, arginfo_rtrim)
	CREX_FALIAS(chop, rtrim, arginfo_chop)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(ltrim, arginfo_ltrim)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(wordwrap, arginfo_wordwrap)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(explode, arginfo_explode)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(implode, arginfo_implode)
	CREX_FALIAS(join, implode, arginfo_join)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(strtok, arginfo_strtok)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(strtoupper, arginfo_strtoupper)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(strtolower, arginfo_strtolower)
	CREX_FE(str_increment, arginfo_str_increment)
	CREX_FE(str_decrement, arginfo_str_decrement)
	CREX_FE(basename, arginfo_basename)
	CREX_FE(dirname, arginfo_dirname)
	CREX_FE(pathinfo, arginfo_pathinfo)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(stristr, arginfo_stristr)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(strstr, arginfo_strstr)
	CREX_FALIAS(strchr, strstr, arginfo_strchr)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(strpos, arginfo_strpos)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(stripos, arginfo_stripos)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(strrpos, arginfo_strrpos)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(strripos, arginfo_strripos)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(strrchr, arginfo_strrchr)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(str_contains, arginfo_str_contains)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(str_starts_with, arginfo_str_starts_with)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(str_ends_with, arginfo_str_ends_with)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(chunk_split, arginfo_chunk_split)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(substr, arginfo_substr)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(substr_replace, arginfo_substr_replace)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(quotemeta, arginfo_quotemeta)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(ord, arginfo_ord)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(chr, arginfo_chr)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(ucfirst, arginfo_ucfirst)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(lcfirst, arginfo_lcfirst)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(ucwords, arginfo_ucwords)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(strtr, arginfo_strtr)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(strrev, arginfo_strrev)
	CREX_FE(similar_text, arginfo_similar_text)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(addcslashes, arginfo_addcslashes)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(addslashes, arginfo_addslashes)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(stripcslashes, arginfo_stripcslashes)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(stripslashes, arginfo_stripslashes)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(str_replace, arginfo_str_replace)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(str_ireplace, arginfo_str_ireplace)
	CREX_FE(hebrev, arginfo_hebrev)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(nl2br, arginfo_nl2br)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(strip_tags, arginfo_strip_tags)
	CREX_FE(setlocale, arginfo_setlocale)
	CREX_FE(parse_str, arginfo_parse_str)
	CREX_FE(str_getcsv, arginfo_str_getcsv)
	CREX_FE(str_repeat, arginfo_str_repeat)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(count_chars, arginfo_count_chars)
	CREX_FE(strnatcmp, arginfo_strnatcmp)
	CREX_FE(localeconv, arginfo_localeconv)
	CREX_FE(strnatcasecmp, arginfo_strnatcasecmp)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(substr_count, arginfo_substr_count)
	CREX_FE(str_pad, arginfo_str_pad)
	CREX_FE(sscanf, arginfo_sscanf)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(str_rot13, arginfo_str_rot13)
	CREX_FE(str_shuffle, arginfo_str_shuffle)
	CREX_FE(str_word_count, arginfo_str_word_count)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(str_split, arginfo_str_split)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(strpbrk, arginfo_strpbrk)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(substr_compare, arginfo_substr_compare)
	CREX_DEP_FE(utf8_encode, arginfo_utf8_encode)
	CREX_DEP_FE(utf8_decode, arginfo_utf8_decode)
	CREX_FE(opendir, arginfo_opendir)
	CREX_FE(dir, arginfo_dir)
	CREX_FE(closedir, arginfo_closedir)
	CREX_FE(chdir, arginfo_chdir)
#if (defined(HAVE_CHROOT) && !defined(ZTS) && defined(ENABLE_CHROOT_FUNC))
	CREX_FE(chroot, arginfo_chroot)
#endif
	CREX_FE(getcwd, arginfo_getcwd)
	CREX_FE(rewinddir, arginfo_rewinddir)
	CREX_FE(readdir, arginfo_readdir)
	CREX_FE(scandir, arginfo_scandir)
#if defined(HAVE_GLOB)
	CREX_FE(glob, arginfo_glob)
#endif
	CREX_FE(exec, arginfo_exec)
	CREX_FE(system, arginfo_system)
	CREX_FE(passthru, arginfo_passthru)
	CREX_FE(escapeshellcmd, arginfo_escapeshellcmd)
	CREX_FE(escapeshellarg, arginfo_escapeshellarg)
	CREX_FE(shell_exec, arginfo_shell_exec)
#if defined(HAVE_NICE)
	CREX_FE(proc_nice, arginfo_proc_nice)
#endif
	CREX_FE(flock, arginfo_flock)
	CREX_FE(get_meta_tags, arginfo_get_meta_tags)
	CREX_FE(pclose, arginfo_pclose)
	CREX_FE(popen, arginfo_popen)
	CREX_FE(readfile, arginfo_readfile)
	CREX_FE(rewind, arginfo_rewind)
	CREX_FE(rmdir, arginfo_rmdir)
	CREX_FE(umask, arginfo_umask)
	CREX_FE(fclose, arginfo_fclose)
	CREX_FE(feof, arginfo_feof)
	CREX_FE(fgetc, arginfo_fgetc)
	CREX_FE(fgets, arginfo_fgets)
	CREX_FE(fread, arginfo_fread)
	CREX_FE(fopen, arginfo_fopen)
	CREX_FE(fscanf, arginfo_fscanf)
	CREX_FE(fpassthru, arginfo_fpassthru)
	CREX_FE(ftruncate, arginfo_ftruncate)
	CREX_FE(fstat, arginfo_fstat)
	CREX_FE(fseek, arginfo_fseek)
	CREX_FE(ftell, arginfo_ftell)
	CREX_FE(fflush, arginfo_fflush)
	CREX_FE(fsync, arginfo_fsync)
	CREX_FE(fdatasync, arginfo_fdatasync)
	CREX_FE(fwrite, arginfo_fwrite)
	CREX_FALIAS(fputs, fwrite, arginfo_fputs)
	CREX_FE(mkdir, arginfo_mkdir)
	CREX_FE(rename, arginfo_rename)
	CREX_FE(copy, arginfo_copy)
	CREX_FE(tempnam, arginfo_tempnam)
	CREX_FE(tmpfile, arginfo_tmpfile)
	CREX_FE(file, arginfo_file)
	CREX_FE(file_get_contents, arginfo_file_get_contents)
	CREX_FE(unlink, arginfo_unlink)
	CREX_FE(file_put_contents, arginfo_file_put_contents)
	CREX_FE(fputcsv, arginfo_fputcsv)
	CREX_FE(fgetcsv, arginfo_fgetcsv)
	CREX_FE(realpath, arginfo_realpath)
#if defined(HAVE_FNMATCH)
	CREX_FE(fnmatch, arginfo_fnmatch)
#endif
	CREX_FE(sys_get_temp_dir, arginfo_sys_get_temp_dir)
	CREX_FE(fileatime, arginfo_fileatime)
	CREX_FE(filectime, arginfo_filectime)
	CREX_FE(filegroup, arginfo_filegroup)
	CREX_FE(fileinode, arginfo_fileinode)
	CREX_FE(filemtime, arginfo_filemtime)
	CREX_FE(fileowner, arginfo_fileowner)
	CREX_FE(fileperms, arginfo_fileperms)
	CREX_FE(filesize, arginfo_filesize)
	CREX_FE(filetype, arginfo_filetype)
	CREX_FE(file_exists, arginfo_file_exists)
	CREX_FE(is_writable, arginfo_is_writable)
	CREX_FALIAS(is_writeable, is_writable, arginfo_is_writeable)
	CREX_FE(is_readable, arginfo_is_readable)
	CREX_FE(is_executable, arginfo_is_executable)
	CREX_FE(is_file, arginfo_is_file)
	CREX_FE(is_dir, arginfo_is_dir)
	CREX_FE(is_link, arginfo_is_link)
	CREX_FE(stat, arginfo_stat)
	CREX_FE(lstat, arginfo_lstat)
	CREX_FE(chown, arginfo_chown)
	CREX_FE(chgrp, arginfo_chgrp)
#if defined(HAVE_LCHOWN)
	CREX_FE(lchown, arginfo_lchown)
#endif
#if defined(HAVE_LCHOWN)
	CREX_FE(lchgrp, arginfo_lchgrp)
#endif
	CREX_FE(chmod, arginfo_chmod)
#if defined(HAVE_UTIME)
	CREX_FE(touch, arginfo_touch)
#endif
	CREX_FE(clearstatcache, arginfo_clearstatcache)
	CREX_FE(disk_total_space, arginfo_disk_total_space)
	CREX_FE(disk_free_space, arginfo_disk_free_space)
	CREX_FALIAS(diskfreespace, disk_free_space, arginfo_diskfreespace)
	CREX_FE(realpath_cache_get, arginfo_realpath_cache_get)
	CREX_FE(realpath_cache_size, arginfo_realpath_cache_size)
	CREX_FE(sprintf, arginfo_sprintf)
	CREX_FE(printf, arginfo_printf)
	CREX_FE(vprintf, arginfo_vprintf)
	CREX_FE(vsprintf, arginfo_vsprintf)
	CREX_FE(fprintf, arginfo_fprintf)
	CREX_FE(vfprintf, arginfo_vfprintf)
	CREX_FE(fsockopen, arginfo_fsockopen)
	CREX_FE(pfsockopen, arginfo_pfsockopen)
	CREX_FE(http_build_query, arginfo_http_build_query)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(image_type_to_mime_type, arginfo_image_type_to_mime_type)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(image_type_to_extension, arginfo_image_type_to_extension)
	CREX_FE(getimagesize, arginfo_getimagesize)
	CREX_FE(getimagesizefromstring, arginfo_getimagesizefromstring)
	CREX_FE(crxinfo, arginfo_crxinfo)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(crxversion, arginfo_crxversion)
	CREX_FE(crxcredits, arginfo_crxcredits)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(crx_sapi_name, arginfo_crx_sapi_name)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(crx_uname, arginfo_crx_uname)
	CREX_FE(crx_ini_scanned_files, arginfo_crx_ini_scanned_files)
	CREX_FE(crx_ini_loaded_file, arginfo_crx_ini_loaded_file)
	CREX_FE(iptcembed, arginfo_iptcembed)
	CREX_FE(iptcparse, arginfo_iptcparse)
	CREX_FE(levenshtein, arginfo_levenshtein)
#if (defined(HAVE_SYMLINK) || defined(CRX_WIN32))
	CREX_FE(readlink, arginfo_readlink)
#endif
#if (defined(HAVE_SYMLINK) || defined(CRX_WIN32))
	CREX_FE(linkinfo, arginfo_linkinfo)
#endif
#if (defined(HAVE_SYMLINK) || defined(CRX_WIN32))
	CREX_FE(symlink, arginfo_symlink)
#endif
#if (defined(HAVE_SYMLINK) || defined(CRX_WIN32))
	CREX_FE(link, arginfo_link)
#endif
	CREX_FE(mail, arginfo_mail)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(abs, arginfo_abs)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(ceil, arginfo_ceil)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(floor, arginfo_floor)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(round, arginfo_round)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(sin, arginfo_sin)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(cos, arginfo_cos)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(tan, arginfo_tan)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(asin, arginfo_asin)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(acos, arginfo_acos)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(atan, arginfo_atan)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(atanh, arginfo_atanh)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(atan2, arginfo_atan2)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(sinh, arginfo_sinh)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(cosh, arginfo_cosh)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(tanh, arginfo_tanh)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(asinh, arginfo_asinh)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(acosh, arginfo_acosh)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(expm1, arginfo_expm1)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(log1p, arginfo_log1p)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(pi, arginfo_pi)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(is_finite, arginfo_is_finite)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(is_nan, arginfo_is_nan)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(intdiv, arginfo_intdiv)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(is_infinite, arginfo_is_infinite)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(pow, arginfo_pow)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(exp, arginfo_exp)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(log, arginfo_log)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(log10, arginfo_log10)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(sqrt, arginfo_sqrt)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(hypot, arginfo_hypot)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(deg2rad, arginfo_deg2rad)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(rad2deg, arginfo_rad2deg)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(bindec, arginfo_bindec)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(hexdec, arginfo_hexdec)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(octdec, arginfo_octdec)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(decbin, arginfo_decbin)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(decoct, arginfo_decoct)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(dechex, arginfo_dechex)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(base_convert, arginfo_base_convert)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(number_format, arginfo_number_format)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(fmod, arginfo_fmod)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(fdiv, arginfo_fdiv)
#if defined(HAVE_GETTIMEOFDAY)
	CREX_FE(microtime, arginfo_microtime)
#endif
#if defined(HAVE_GETTIMEOFDAY)
	CREX_FE(gettimeofday, arginfo_gettimeofday)
#endif
#if defined(HAVE_GETRUSAGE)
	CREX_FE(getrusage, arginfo_getrusage)
#endif
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(pack, arginfo_pack)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(unpack, arginfo_unpack)
	CREX_FE(password_get_info, arginfo_password_get_info)
	CREX_FE(password_hash, arginfo_password_hash)
	CREX_FE(password_needs_rehash, arginfo_password_needs_rehash)
	CREX_FE(password_verify, arginfo_password_verify)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(password_algos, arginfo_password_algos)
#if defined(CRX_CAN_SUPPORT_PROC_OPEN)
	CREX_FE(proc_open, arginfo_proc_open)
#endif
#if defined(CRX_CAN_SUPPORT_PROC_OPEN)
	CREX_FE(proc_close, arginfo_proc_close)
#endif
#if defined(CRX_CAN_SUPPORT_PROC_OPEN)
	CREX_FE(proc_terminate, arginfo_proc_terminate)
#endif
#if defined(CRX_CAN_SUPPORT_PROC_OPEN)
	CREX_FE(proc_get_status, arginfo_proc_get_status)
#endif
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(quoted_printable_decode, arginfo_quoted_printable_decode)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(quoted_printable_encode, arginfo_quoted_printable_encode)
	CREX_FE(soundex, arginfo_soundex)
	CREX_FE(stream_select, arginfo_stream_select)
	CREX_FE(stream_context_create, arginfo_stream_context_create)
	CREX_FE(stream_context_set_params, arginfo_stream_context_set_params)
	CREX_FE(stream_context_get_params, arginfo_stream_context_get_params)
	CREX_FE(stream_context_set_option, arginfo_stream_context_set_option)
	CREX_FE(stream_context_set_options, arginfo_stream_context_set_options)
	CREX_FE(stream_context_get_options, arginfo_stream_context_get_options)
	CREX_FE(stream_context_get_default, arginfo_stream_context_get_default)
	CREX_FE(stream_context_set_default, arginfo_stream_context_set_default)
	CREX_FE(stream_filter_prepend, arginfo_stream_filter_prepend)
	CREX_FE(stream_filter_append, arginfo_stream_filter_append)
	CREX_FE(stream_filter_remove, arginfo_stream_filter_remove)
	CREX_FE(stream_socket_client, arginfo_stream_socket_client)
	CREX_FE(stream_socket_server, arginfo_stream_socket_server)
	CREX_FE(stream_socket_accept, arginfo_stream_socket_accept)
	CREX_FE(stream_socket_get_name, arginfo_stream_socket_get_name)
	CREX_FE(stream_socket_recvfrom, arginfo_stream_socket_recvfrom)
	CREX_FE(stream_socket_sendto, arginfo_stream_socket_sendto)
	CREX_FE(stream_socket_enable_crypto, arginfo_stream_socket_enable_crypto)
#if defined(HAVE_SHUTDOWN)
	CREX_FE(stream_socket_shutdown, arginfo_stream_socket_shutdown)
#endif
#if defined(HAVE_SOCKETPAIR)
	CREX_FE(stream_socket_pair, arginfo_stream_socket_pair)
#endif
	CREX_FE(stream_copy_to_stream, arginfo_stream_copy_to_stream)
	CREX_FE(stream_get_contents, arginfo_stream_get_contents)
	CREX_FE(stream_supports_lock, arginfo_stream_supports_lock)
	CREX_FE(stream_set_write_buffer, arginfo_stream_set_write_buffer)
	CREX_FALIAS(set_file_buffer, stream_set_write_buffer, arginfo_set_file_buffer)
	CREX_FE(stream_set_read_buffer, arginfo_stream_set_read_buffer)
	CREX_FE(stream_set_blocking, arginfo_stream_set_blocking)
	CREX_FALIAS(socket_set_blocking, stream_set_blocking, arginfo_socket_set_blocking)
	CREX_FE(stream_get_meta_data, arginfo_stream_get_meta_data)
	CREX_FALIAS(socket_get_status, stream_get_meta_data, arginfo_socket_get_status)
	CREX_FE(stream_get_line, arginfo_stream_get_line)
	CREX_FE(stream_resolve_include_path, arginfo_stream_resolve_include_path)
	CREX_FE(stream_get_wrappers, arginfo_stream_get_wrappers)
	CREX_FE(stream_get_transports, arginfo_stream_get_transports)
	CREX_FE(stream_is_local, arginfo_stream_is_local)
	CREX_FE(stream_isatty, arginfo_stream_isatty)
#if defined(CRX_WIN32)
	CREX_FE(sapi_windows_vt100_support, arginfo_sapi_windows_vt100_support)
#endif
	CREX_FE(stream_set_chunk_size, arginfo_stream_set_chunk_size)
#if (defined(HAVE_SYS_TIME_H) || defined(CRX_WIN32))
	CREX_FE(stream_set_timeout, arginfo_stream_set_timeout)
#endif
#if (defined(HAVE_SYS_TIME_H) || defined(CRX_WIN32))
	CREX_FALIAS(socket_set_timeout, stream_set_timeout, arginfo_socket_set_timeout)
#endif
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(gettype, arginfo_gettype)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(get_debug_type, arginfo_get_debug_type)
	CREX_FE(settype, arginfo_settype)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(intval, arginfo_intval)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(floatval, arginfo_floatval)
	CREX_FALIAS(doubleval, floatval, arginfo_doubleval)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(boolval, arginfo_boolval)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(strval, arginfo_strval)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(is_null, arginfo_is_null)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(is_resource, arginfo_is_resource)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(is_bool, arginfo_is_bool)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(is_int, arginfo_is_int)
	CREX_FALIAS(is_integer, is_int, arginfo_is_integer)
	CREX_FALIAS(is_long, is_int, arginfo_is_long)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(is_float, arginfo_is_float)
	CREX_FALIAS(is_double, is_float, arginfo_is_double)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(is_numeric, arginfo_is_numeric)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(is_string, arginfo_is_string)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(is_array, arginfo_is_array)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(is_object, arginfo_is_object)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(is_scalar, arginfo_is_scalar)
	CREX_FE(is_callable, arginfo_is_callable)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(is_iterable, arginfo_is_iterable)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(is_countable, arginfo_is_countable)
#if defined(HAVE_GETTIMEOFDAY)
	CREX_FE(uniqid, arginfo_uniqid)
#endif
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(parse_url, arginfo_parse_url)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(urlencode, arginfo_urlencode)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(urldecode, arginfo_urldecode)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(rawurlencode, arginfo_rawurlencode)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(rawurldecode, arginfo_rawurldecode)
	CREX_FE(get_headers, arginfo_get_headers)
	CREX_FE(stream_bucket_make_writeable, arginfo_stream_bucket_make_writeable)
	CREX_FE(stream_bucket_prepend, arginfo_stream_bucket_prepend)
	CREX_FE(stream_bucket_append, arginfo_stream_bucket_append)
	CREX_FE(stream_bucket_new, arginfo_stream_bucket_new)
	CREX_FE(stream_get_filters, arginfo_stream_get_filters)
	CREX_FE(stream_filter_register, arginfo_stream_filter_register)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(convert_uuencode, arginfo_convert_uuencode)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(convert_uudecode, arginfo_convert_uudecode)
	CREX_FE(var_dump, arginfo_var_dump)
	CREX_FE(var_export, arginfo_var_export)
	CREX_FE(debug_zval_dump, arginfo_debug_zval_dump)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(serialize, arginfo_serialize)
	CREX_FE(unserialize, arginfo_unserialize)
	CREX_FE(memory_get_usage, arginfo_memory_get_usage)
	CREX_FE(memory_get_peak_usage, arginfo_memory_get_peak_usage)
	CREX_FE(memory_reset_peak_usage, arginfo_memory_reset_peak_usage)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(version_compare, arginfo_version_compare)
#if defined(CRX_WIN32)
	CREX_FE(sapi_windows_cp_set, arginfo_sapi_windows_cp_set)
#endif
#if defined(CRX_WIN32)
	CREX_FE(sapi_windows_cp_get, arginfo_sapi_windows_cp_get)
#endif
#if defined(CRX_WIN32)
	CREX_FE(sapi_windows_cp_conv, arginfo_sapi_windows_cp_conv)
#endif
#if defined(CRX_WIN32)
	CREX_FE(sapi_windows_cp_is_utf8, arginfo_sapi_windows_cp_is_utf8)
#endif
#if defined(CRX_WIN32)
	CREX_FE(sapi_windows_set_ctrl_handler, arginfo_sapi_windows_set_ctrl_handler)
#endif
#if defined(CRX_WIN32)
	CREX_FE(sapi_windows_generate_ctrl_event, arginfo_sapi_windows_generate_ctrl_event)
#endif
	CREX_FE_END
};


static const crex_function_entry class___CRX_Incomplete_Class_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_AssertionError_methods[] = {
	CREX_FE_END
};

static void register_basic_functions_symbols(int module_number)
{
	REGISTER_LONG_CONSTANT("EXTR_OVERWRITE", CRX_EXTR_OVERWRITE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("EXTR_SKIP", CRX_EXTR_SKIP, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("EXTR_PREFIX_SAME", CRX_EXTR_PREFIX_SAME, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("EXTR_PREFIX_ALL", CRX_EXTR_PREFIX_ALL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("EXTR_PREFIX_INVALID", CRX_EXTR_PREFIX_INVALID, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("EXTR_PREFIX_IF_EXISTS", CRX_EXTR_PREFIX_IF_EXISTS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("EXTR_IF_EXISTS", CRX_EXTR_IF_EXISTS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("EXTR_REFS", CRX_EXTR_REFS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SORT_ASC", CRX_SORT_ASC, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SORT_DESC", CRX_SORT_DESC, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SORT_REGULAR", CRX_SORT_REGULAR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SORT_NUMERIC", CRX_SORT_NUMERIC, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SORT_STRING", CRX_SORT_STRING, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SORT_LOCALE_STRING", CRX_SORT_LOCALE_STRING, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SORT_NATURAL", CRX_SORT_NATURAL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SORT_FLAG_CASE", CRX_SORT_FLAG_CASE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CASE_LOWER", CRX_CASE_LOWER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CASE_UPPER", CRX_CASE_UPPER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("COUNT_NORMAL", CRX_COUNT_NORMAL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("COUNT_RECURSIVE", CRX_COUNT_RECURSIVE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ARRAY_FILTER_USE_BOTH", ARRAY_FILTER_USE_BOTH, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ARRAY_FILTER_USE_KEY", ARRAY_FILTER_USE_KEY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ASSERT_ACTIVE", CRX_ASSERT_ACTIVE, CONST_PERSISTENT | CONST_DEPRECATED);
	REGISTER_LONG_CONSTANT("ASSERT_CALLBACK", CRX_ASSERT_CALLBACK, CONST_PERSISTENT | CONST_DEPRECATED);
	REGISTER_LONG_CONSTANT("ASSERT_BAIL", CRX_ASSERT_BAIL, CONST_PERSISTENT | CONST_DEPRECATED);
	REGISTER_LONG_CONSTANT("ASSERT_WARNING", CRX_ASSERT_WARNING, CONST_PERSISTENT | CONST_DEPRECATED);
	REGISTER_LONG_CONSTANT("ASSERT_EXCEPTION", CRX_ASSERT_EXCEPTION, CONST_PERSISTENT | CONST_DEPRECATED);
	REGISTER_LONG_CONSTANT("CONNECTION_ABORTED", CRX_CONNECTION_ABORTED, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CONNECTION_NORMAL", CRX_CONNECTION_NORMAL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CONNECTION_TIMEOUT", CRX_CONNECTION_TIMEOUT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("INI_USER", CREX_INI_USER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("INI_PERDIR", CREX_INI_PERDIR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("INI_SYSTEM", CREX_INI_SYSTEM, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("INI_ALL", CREX_INI_ALL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("INI_SCANNER_NORMAL", CREX_INI_SCANNER_NORMAL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("INI_SCANNER_RAW", CREX_INI_SCANNER_RAW, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("INI_SCANNER_TYPED", CREX_INI_SCANNER_TYPED, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRX_URL_SCHEME", CRX_URL_SCHEME, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRX_URL_HOST", CRX_URL_HOST, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRX_URL_PORT", CRX_URL_PORT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRX_URL_USER", CRX_URL_USER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRX_URL_PASS", CRX_URL_PASS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRX_URL_PATH", CRX_URL_PATH, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRX_URL_QUERY", CRX_URL_QUERY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRX_URL_FRAGMENT", CRX_URL_FRAGMENT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRX_QUERY_RFC1738", CRX_QUERY_RFC1738, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRX_QUERY_RFC3986", CRX_QUERY_RFC3986, CONST_PERSISTENT);
	REGISTER_DOUBLE_CONSTANT("M_E", M_E, CONST_PERSISTENT);
	CREX_ASSERT(M_E == 2.718281828459045);
	REGISTER_DOUBLE_CONSTANT("M_LOG2E", M_LOG2E, CONST_PERSISTENT);
	CREX_ASSERT(M_LOG2E == 1.4426950408889634);
	REGISTER_DOUBLE_CONSTANT("M_LOG10E", M_LOG10E, CONST_PERSISTENT);
	CREX_ASSERT(M_LOG10E == 0.4342944819032518);
	REGISTER_DOUBLE_CONSTANT("M_LN2", M_LN2, CONST_PERSISTENT);
	CREX_ASSERT(M_LN2 == 0.6931471805599453);
	REGISTER_DOUBLE_CONSTANT("M_LN10", M_LN10, CONST_PERSISTENT);
	CREX_ASSERT(M_LN10 == 2.302585092994046);
	REGISTER_DOUBLE_CONSTANT("M_PI", M_PI, CONST_PERSISTENT);
	CREX_ASSERT(M_PI == 3.141592653589793);
	REGISTER_DOUBLE_CONSTANT("M_PI_2", M_PI_2, CONST_PERSISTENT);
	CREX_ASSERT(M_PI_2 == 1.5707963267948966);
	REGISTER_DOUBLE_CONSTANT("M_PI_4", M_PI_4, CONST_PERSISTENT);
	CREX_ASSERT(M_PI_4 == 0.7853981633974483);
	REGISTER_DOUBLE_CONSTANT("M_1_PI", M_1_PI, CONST_PERSISTENT);
	CREX_ASSERT(M_1_PI == 0.3183098861837907);
	REGISTER_DOUBLE_CONSTANT("M_2_PI", M_2_PI, CONST_PERSISTENT);
	CREX_ASSERT(M_2_PI == 0.6366197723675814);
	REGISTER_DOUBLE_CONSTANT("M_SQRTPI", M_SQRTPI, CONST_PERSISTENT);
	CREX_ASSERT(M_SQRTPI == 1.772453850905516);
	REGISTER_DOUBLE_CONSTANT("M_2_SQRTPI", M_2_SQRTPI, CONST_PERSISTENT);
	CREX_ASSERT(M_2_SQRTPI == 1.1283791670955126);
	REGISTER_DOUBLE_CONSTANT("M_LNPI", M_LNPI, CONST_PERSISTENT);
	CREX_ASSERT(M_LNPI == 1.1447298858494002);
	REGISTER_DOUBLE_CONSTANT("M_EULER", M_EULER, CONST_PERSISTENT);
	CREX_ASSERT(M_EULER == 0.5772156649015329);
	REGISTER_DOUBLE_CONSTANT("M_SQRT2", M_SQRT2, CONST_PERSISTENT);
	CREX_ASSERT(M_SQRT2 == 1.4142135623730951);
	REGISTER_DOUBLE_CONSTANT("M_SQRT1_2", M_SQRT1_2, CONST_PERSISTENT);
	CREX_ASSERT(M_SQRT1_2 == 0.7071067811865476);
	REGISTER_DOUBLE_CONSTANT("M_SQRT3", M_SQRT3, CONST_PERSISTENT);
	CREX_ASSERT(M_SQRT3 == 1.7320508075688772);
	REGISTER_DOUBLE_CONSTANT("INF", CREX_INFINITY, CONST_PERSISTENT);
	REGISTER_DOUBLE_CONSTANT("NAN", CREX_NAN, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRX_ROUND_HALF_UP", CRX_ROUND_HALF_UP, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRX_ROUND_HALF_DOWN", CRX_ROUND_HALF_DOWN, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRX_ROUND_HALF_EVEN", CRX_ROUND_HALF_EVEN, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRX_ROUND_HALF_ODD", CRX_ROUND_HALF_ODD, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRYPT_SALT_LENGTH", CRX_MAX_SALT_LEN, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRYPT_STD_DES", 1, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRYPT_EXT_DES", 1, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRYPT_MD5", 1, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRYPT_BLOWFISH", 1, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRYPT_SHA256", 1, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRYPT_SHA512", 1, CONST_PERSISTENT);
#if (defined(CRX_WIN32) || (defined(HAVE_DNS_SEARCH_FUNC) && defined(HAVE_FULL_DNS_FUNCS)))
	REGISTER_LONG_CONSTANT("DNS_A", CRX_DNS_A, CONST_PERSISTENT);
#endif
#if (defined(CRX_WIN32) || (defined(HAVE_DNS_SEARCH_FUNC) && defined(HAVE_FULL_DNS_FUNCS)))
	REGISTER_LONG_CONSTANT("DNS_NS", CRX_DNS_NS, CONST_PERSISTENT);
#endif
#if (defined(CRX_WIN32) || (defined(HAVE_DNS_SEARCH_FUNC) && defined(HAVE_FULL_DNS_FUNCS)))
	REGISTER_LONG_CONSTANT("DNS_CNAME", CRX_DNS_CNAME, CONST_PERSISTENT);
#endif
#if (defined(CRX_WIN32) || (defined(HAVE_DNS_SEARCH_FUNC) && defined(HAVE_FULL_DNS_FUNCS)))
	REGISTER_LONG_CONSTANT("DNS_SOA", CRX_DNS_SOA, CONST_PERSISTENT);
#endif
#if (defined(CRX_WIN32) || (defined(HAVE_DNS_SEARCH_FUNC) && defined(HAVE_FULL_DNS_FUNCS)))
	REGISTER_LONG_CONSTANT("DNS_PTR", CRX_DNS_PTR, CONST_PERSISTENT);
#endif
#if (defined(CRX_WIN32) || (defined(HAVE_DNS_SEARCH_FUNC) && defined(HAVE_FULL_DNS_FUNCS)))
	REGISTER_LONG_CONSTANT("DNS_HINFO", CRX_DNS_HINFO, CONST_PERSISTENT);
#endif
#if (defined(CRX_WIN32) || (defined(HAVE_DNS_SEARCH_FUNC) && defined(HAVE_FULL_DNS_FUNCS))) && (!defined(CRX_WIN32))
	REGISTER_LONG_CONSTANT("DNS_CAA", CRX_DNS_CAA, CONST_PERSISTENT);
#endif
#if (defined(CRX_WIN32) || (defined(HAVE_DNS_SEARCH_FUNC) && defined(HAVE_FULL_DNS_FUNCS)))
	REGISTER_LONG_CONSTANT("DNS_MX", CRX_DNS_MX, CONST_PERSISTENT);
#endif
#if (defined(CRX_WIN32) || (defined(HAVE_DNS_SEARCH_FUNC) && defined(HAVE_FULL_DNS_FUNCS)))
	REGISTER_LONG_CONSTANT("DNS_TXT", CRX_DNS_TXT, CONST_PERSISTENT);
#endif
#if (defined(CRX_WIN32) || (defined(HAVE_DNS_SEARCH_FUNC) && defined(HAVE_FULL_DNS_FUNCS)))
	REGISTER_LONG_CONSTANT("DNS_SRV", CRX_DNS_SRV, CONST_PERSISTENT);
#endif
#if (defined(CRX_WIN32) || (defined(HAVE_DNS_SEARCH_FUNC) && defined(HAVE_FULL_DNS_FUNCS)))
	REGISTER_LONG_CONSTANT("DNS_NAPTR", CRX_DNS_NAPTR, CONST_PERSISTENT);
#endif
#if (defined(CRX_WIN32) || (defined(HAVE_DNS_SEARCH_FUNC) && defined(HAVE_FULL_DNS_FUNCS)))
	REGISTER_LONG_CONSTANT("DNS_AAAA", CRX_DNS_AAAA, CONST_PERSISTENT);
#endif
#if (defined(CRX_WIN32) || (defined(HAVE_DNS_SEARCH_FUNC) && defined(HAVE_FULL_DNS_FUNCS)))
	REGISTER_LONG_CONSTANT("DNS_A6", CRX_DNS_A6, CONST_PERSISTENT);
#endif
#if (defined(CRX_WIN32) || (defined(HAVE_DNS_SEARCH_FUNC) && defined(HAVE_FULL_DNS_FUNCS)))
	REGISTER_LONG_CONSTANT("DNS_ANY", CRX_DNS_ANY, CONST_PERSISTENT);
#endif
#if (defined(CRX_WIN32) || (defined(HAVE_DNS_SEARCH_FUNC) && defined(HAVE_FULL_DNS_FUNCS)))
	REGISTER_LONG_CONSTANT("DNS_ALL", CRX_DNS_ALL, CONST_PERSISTENT);
#endif
	REGISTER_LONG_CONSTANT("HTML_SPECIALCHARS", CRX_HTML_SPECIALCHARS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("HTML_ENTITIES", CRX_HTML_ENTITIES, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ENT_COMPAT", ENT_COMPAT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ENT_QUOTES", ENT_QUOTES, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ENT_NOQUOTES", ENT_NOQUOTES, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ENT_IGNORE", ENT_IGNORE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ENT_SUBSTITUTE", ENT_SUBSTITUTE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ENT_DISALLOWED", ENT_DISALLOWED, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ENT_HTML401", ENT_HTML401, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ENT_XML1", ENT_XML1, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ENT_XHTML", ENT_XHTML, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("ENT_HTML5", ENT_HTML5, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMAGETYPE_GIF", IMAGE_FILETYPE_GIF, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMAGETYPE_JPEG", IMAGE_FILETYPE_JPEG, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMAGETYPE_PNG", IMAGE_FILETYPE_PNG, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMAGETYPE_SWF", IMAGE_FILETYPE_SWF, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMAGETYPE_PSD", IMAGE_FILETYPE_PSD, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMAGETYPE_BMP", IMAGE_FILETYPE_BMP, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMAGETYPE_TIFF_II", IMAGE_FILETYPE_TIFF_II, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMAGETYPE_TIFF_MM", IMAGE_FILETYPE_TIFF_MM, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMAGETYPE_JPC", IMAGE_FILETYPE_JPC, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMAGETYPE_JP2", IMAGE_FILETYPE_JP2, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMAGETYPE_JPX", IMAGE_FILETYPE_JPX, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMAGETYPE_JB2", IMAGE_FILETYPE_JB2, CONST_PERSISTENT);
#if (defined(HAVE_ZLIB) && !defined(COMPILE_DL_ZLIB))
	REGISTER_LONG_CONSTANT("IMAGETYPE_SWC", IMAGE_FILETYPE_SWC, CONST_PERSISTENT);
#endif
	REGISTER_LONG_CONSTANT("IMAGETYPE_IFF", IMAGE_FILETYPE_IFF, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMAGETYPE_WBMP", IMAGE_FILETYPE_WBMP, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMAGETYPE_JPEG2000", IMAGE_FILETYPE_JPC, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMAGETYPE_XBM", IMAGE_FILETYPE_XBM, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMAGETYPE_ICO", IMAGE_FILETYPE_ICO, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMAGETYPE_WEBP", IMAGE_FILETYPE_WEBP, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMAGETYPE_AVIF", IMAGE_FILETYPE_AVIF, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMAGETYPE_UNKNOWN", IMAGE_FILETYPE_UNKNOWN, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IMAGETYPE_COUNT", IMAGE_FILETYPE_COUNT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("INFO_GENERAL", CRX_INFO_GENERAL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("INFO_CREDITS", CRX_INFO_CREDITS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("INFO_CONFIGURATION", CRX_INFO_CONFIGURATION, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("INFO_MODULES", CRX_INFO_MODULES, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("INFO_ENVIRONMENT", CRX_INFO_ENVIRONMENT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("INFO_VARIABLES", CRX_INFO_VARIABLES, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("INFO_LICENSE", CRX_INFO_LICENSE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("INFO_ALL", CRX_INFO_ALL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CREDITS_GROUP", CRX_CREDITS_GROUP, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CREDITS_GENERAL", CRX_CREDITS_GENERAL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CREDITS_SAPI", CRX_CREDITS_SAPI, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CREDITS_MODULES", CRX_CREDITS_MODULES, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CREDITS_DOCS", CRX_CREDITS_DOCS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CREDITS_FULLPAGE", CRX_CREDITS_FULLPAGE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CREDITS_QA", CRX_CREDITS_QA, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CREDITS_ALL", CRX_CREDITS_ALL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_EMERG", LOG_EMERG, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_ALERT", LOG_ALERT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_CRIT", LOG_CRIT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_ERR", LOG_ERR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_WARNING", LOG_WARNING, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_NOTICE", LOG_NOTICE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_INFO", LOG_INFO, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_DEBUG", LOG_DEBUG, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_KERN", LOG_KERN, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_USER", LOG_USER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_MAIL", LOG_MAIL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_DAEMON", LOG_DAEMON, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_AUTH", LOG_AUTH, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_SYSLOG", LOG_SYSLOG, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_LPR", LOG_LPR, CONST_PERSISTENT);
#if defined(LOG_NEWS)
	REGISTER_LONG_CONSTANT("LOG_NEWS", LOG_NEWS, CONST_PERSISTENT);
#endif
#if defined(LOG_UUCP)
	REGISTER_LONG_CONSTANT("LOG_UUCP", LOG_UUCP, CONST_PERSISTENT);
#endif
#if defined(LOG_CRON)
	REGISTER_LONG_CONSTANT("LOG_CRON", LOG_CRON, CONST_PERSISTENT);
#endif
#if defined(LOG_AUTHPRIV)
	REGISTER_LONG_CONSTANT("LOG_AUTHPRIV", LOG_AUTHPRIV, CONST_PERSISTENT);
#endif
#if !defined(CRX_WIN32)
	REGISTER_LONG_CONSTANT("LOG_LOCAL0", LOG_LOCAL0, CONST_PERSISTENT);
#endif
#if !defined(CRX_WIN32)
	REGISTER_LONG_CONSTANT("LOG_LOCAL1", LOG_LOCAL1, CONST_PERSISTENT);
#endif
#if !defined(CRX_WIN32)
	REGISTER_LONG_CONSTANT("LOG_LOCAL2", LOG_LOCAL2, CONST_PERSISTENT);
#endif
#if !defined(CRX_WIN32)
	REGISTER_LONG_CONSTANT("LOG_LOCAL3", LOG_LOCAL3, CONST_PERSISTENT);
#endif
#if !defined(CRX_WIN32)
	REGISTER_LONG_CONSTANT("LOG_LOCAL4", LOG_LOCAL4, CONST_PERSISTENT);
#endif
#if !defined(CRX_WIN32)
	REGISTER_LONG_CONSTANT("LOG_LOCAL5", LOG_LOCAL5, CONST_PERSISTENT);
#endif
#if !defined(CRX_WIN32)
	REGISTER_LONG_CONSTANT("LOG_LOCAL6", LOG_LOCAL6, CONST_PERSISTENT);
#endif
#if !defined(CRX_WIN32)
	REGISTER_LONG_CONSTANT("LOG_LOCAL7", LOG_LOCAL7, CONST_PERSISTENT);
#endif
	REGISTER_LONG_CONSTANT("LOG_PID", LOG_PID, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_CONS", LOG_CONS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_ODELAY", LOG_ODELAY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_NDELAY", LOG_NDELAY, CONST_PERSISTENT);
#if defined(LOG_NOWAIT)
	REGISTER_LONG_CONSTANT("LOG_NOWAIT", LOG_NOWAIT, CONST_PERSISTENT);
#endif
#if defined(LOG_PERROR)
	REGISTER_LONG_CONSTANT("LOG_PERROR", LOG_PERROR, CONST_PERSISTENT);
#endif
	REGISTER_LONG_CONSTANT("STR_PAD_LEFT", CRX_STR_PAD_LEFT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STR_PAD_RIGHT", CRX_STR_PAD_RIGHT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("STR_PAD_BOTH", CRX_STR_PAD_BOTH, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PATHINFO_DIRNAME", CRX_PATHINFO_DIRNAME, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PATHINFO_BASENAME", CRX_PATHINFO_BASENAME, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PATHINFO_EXTENSION", CRX_PATHINFO_EXTENSION, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PATHINFO_FILENAME", CRX_PATHINFO_FILENAME, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PATHINFO_ALL", CRX_PATHINFO_ALL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CHAR_MAX", CHAR_MAX, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LC_CTYPE", LC_CTYPE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LC_NUMERIC", LC_NUMERIC, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LC_TIME", LC_TIME, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LC_COLLATE", LC_COLLATE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LC_MONETARY", LC_MONETARY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LC_ALL", LC_ALL, CONST_PERSISTENT);
#if defined(LC_MESSAGES)
	REGISTER_LONG_CONSTANT("LC_MESSAGES", LC_MESSAGES, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ABDAY_1)
	REGISTER_LONG_CONSTANT("ABDAY_1", ABDAY_1, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ABDAY_1)
	REGISTER_LONG_CONSTANT("ABDAY_2", ABDAY_2, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ABDAY_1)
	REGISTER_LONG_CONSTANT("ABDAY_3", ABDAY_3, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ABDAY_1)
	REGISTER_LONG_CONSTANT("ABDAY_4", ABDAY_4, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ABDAY_1)
	REGISTER_LONG_CONSTANT("ABDAY_5", ABDAY_5, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ABDAY_1)
	REGISTER_LONG_CONSTANT("ABDAY_6", ABDAY_6, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ABDAY_1)
	REGISTER_LONG_CONSTANT("ABDAY_7", ABDAY_7, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(DAY_1)
	REGISTER_LONG_CONSTANT("DAY_1", DAY_1, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(DAY_1)
	REGISTER_LONG_CONSTANT("DAY_2", DAY_2, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(DAY_1)
	REGISTER_LONG_CONSTANT("DAY_3", DAY_3, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(DAY_1)
	REGISTER_LONG_CONSTANT("DAY_4", DAY_4, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(DAY_1)
	REGISTER_LONG_CONSTANT("DAY_5", DAY_5, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(DAY_1)
	REGISTER_LONG_CONSTANT("DAY_6", DAY_6, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(DAY_1)
	REGISTER_LONG_CONSTANT("DAY_7", DAY_7, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ABMON_1)
	REGISTER_LONG_CONSTANT("ABMON_1", ABMON_1, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ABMON_1)
	REGISTER_LONG_CONSTANT("ABMON_2", ABMON_2, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ABMON_1)
	REGISTER_LONG_CONSTANT("ABMON_3", ABMON_3, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ABMON_1)
	REGISTER_LONG_CONSTANT("ABMON_4", ABMON_4, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ABMON_1)
	REGISTER_LONG_CONSTANT("ABMON_5", ABMON_5, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ABMON_1)
	REGISTER_LONG_CONSTANT("ABMON_6", ABMON_6, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ABMON_1)
	REGISTER_LONG_CONSTANT("ABMON_7", ABMON_7, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ABMON_1)
	REGISTER_LONG_CONSTANT("ABMON_8", ABMON_8, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ABMON_1)
	REGISTER_LONG_CONSTANT("ABMON_9", ABMON_9, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ABMON_1)
	REGISTER_LONG_CONSTANT("ABMON_10", ABMON_10, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ABMON_1)
	REGISTER_LONG_CONSTANT("ABMON_11", ABMON_11, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ABMON_1)
	REGISTER_LONG_CONSTANT("ABMON_12", ABMON_12, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(MON_1)
	REGISTER_LONG_CONSTANT("MON_1", MON_1, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(MON_1)
	REGISTER_LONG_CONSTANT("MON_2", MON_2, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(MON_1)
	REGISTER_LONG_CONSTANT("MON_3", MON_3, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(MON_1)
	REGISTER_LONG_CONSTANT("MON_4", MON_4, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(MON_1)
	REGISTER_LONG_CONSTANT("MON_5", MON_5, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(MON_1)
	REGISTER_LONG_CONSTANT("MON_6", MON_6, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(MON_1)
	REGISTER_LONG_CONSTANT("MON_7", MON_7, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(MON_1)
	REGISTER_LONG_CONSTANT("MON_8", MON_8, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(MON_1)
	REGISTER_LONG_CONSTANT("MON_9", MON_9, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(MON_1)
	REGISTER_LONG_CONSTANT("MON_10", MON_10, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(MON_1)
	REGISTER_LONG_CONSTANT("MON_11", MON_11, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(MON_1)
	REGISTER_LONG_CONSTANT("MON_12", MON_12, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(AM_STR)
	REGISTER_LONG_CONSTANT("AM_STR", AM_STR, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(PM_STR)
	REGISTER_LONG_CONSTANT("PM_STR", PM_STR, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(D_T_FMT)
	REGISTER_LONG_CONSTANT("D_T_FMT", D_T_FMT, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(D_FMT)
	REGISTER_LONG_CONSTANT("D_FMT", D_FMT, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(T_FMT)
	REGISTER_LONG_CONSTANT("T_FMT", T_FMT, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(T_FMT_AMPM)
	REGISTER_LONG_CONSTANT("T_FMT_AMPM", T_FMT_AMPM, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ERA)
	REGISTER_LONG_CONSTANT("ERA", ERA, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ERA_YEAR)
	REGISTER_LONG_CONSTANT("ERA_YEAR", ERA_YEAR, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ERA_D_T_FMT)
	REGISTER_LONG_CONSTANT("ERA_D_T_FMT", ERA_D_T_FMT, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ERA_D_FMT)
	REGISTER_LONG_CONSTANT("ERA_D_FMT", ERA_D_FMT, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ERA_T_FMT)
	REGISTER_LONG_CONSTANT("ERA_T_FMT", ERA_T_FMT, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(ALT_DIGITS)
	REGISTER_LONG_CONSTANT("ALT_DIGITS", ALT_DIGITS, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(INT_CURR_SYMBOL)
	REGISTER_LONG_CONSTANT("INT_CURR_SYMBOL", INT_CURR_SYMBOL, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(CURRENCY_SYMBOL)
	REGISTER_LONG_CONSTANT("CURRENCY_SYMBOL", CURRENCY_SYMBOL, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(CRNCYSTR)
	REGISTER_LONG_CONSTANT("CRNCYSTR", CRNCYSTR, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(MON_DECIMAL_POINT)
	REGISTER_LONG_CONSTANT("MON_DECIMAL_POINT", MON_DECIMAL_POINT, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(MON_THOUSANDS_SEP)
	REGISTER_LONG_CONSTANT("MON_THOUSANDS_SEP", MON_THOUSANDS_SEP, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(MON_GROUPING)
	REGISTER_LONG_CONSTANT("MON_GROUPING", MON_GROUPING, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(POSITIVE_SIGN)
	REGISTER_LONG_CONSTANT("POSITIVE_SIGN", POSITIVE_SIGN, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(NEGATIVE_SIGN)
	REGISTER_LONG_CONSTANT("NEGATIVE_SIGN", NEGATIVE_SIGN, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(INT_FRAC_DIGITS)
	REGISTER_LONG_CONSTANT("INT_FRAC_DIGITS", INT_FRAC_DIGITS, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(FRAC_DIGITS)
	REGISTER_LONG_CONSTANT("FRAC_DIGITS", FRAC_DIGITS, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(P_CS_PRECEDES)
	REGISTER_LONG_CONSTANT("P_CS_PRECEDES", P_CS_PRECEDES, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(P_SEP_BY_SPACE)
	REGISTER_LONG_CONSTANT("P_SEP_BY_SPACE", P_SEP_BY_SPACE, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(N_CS_PRECEDES)
	REGISTER_LONG_CONSTANT("N_CS_PRECEDES", N_CS_PRECEDES, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(N_SEP_BY_SPACE)
	REGISTER_LONG_CONSTANT("N_SEP_BY_SPACE", N_SEP_BY_SPACE, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(P_SIGN_POSN)
	REGISTER_LONG_CONSTANT("P_SIGN_POSN", P_SIGN_POSN, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(N_SIGN_POSN)
	REGISTER_LONG_CONSTANT("N_SIGN_POSN", N_SIGN_POSN, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(DECIMAL_POINT)
	REGISTER_LONG_CONSTANT("DECIMAL_POINT", DECIMAL_POINT, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(RADIXCHAR)
	REGISTER_LONG_CONSTANT("RADIXCHAR", RADIXCHAR, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(THOUSANDS_SEP)
	REGISTER_LONG_CONSTANT("THOUSANDS_SEP", THOUSANDS_SEP, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(THOUSEP)
	REGISTER_LONG_CONSTANT("THOUSEP", THOUSEP, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(GROUPING)
	REGISTER_LONG_CONSTANT("GROUPING", GROUPING, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(YESEXPR)
	REGISTER_LONG_CONSTANT("YESEXPR", YESEXPR, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(NOEXPR)
	REGISTER_LONG_CONSTANT("NOEXPR", NOEXPR, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(YESSTR)
	REGISTER_LONG_CONSTANT("YESSTR", YESSTR, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(NOSTR)
	REGISTER_LONG_CONSTANT("NOSTR", NOSTR, CONST_PERSISTENT);
#endif
#if defined(HAVE_NL_LANGINFO) && defined(CODESET)
	REGISTER_LONG_CONSTANT("CODESET", CODESET, CONST_PERSISTENT);
#endif


	crex_add_parameter_attribute(crex_hash_str_find_ptr(CG(function_table), "crypt", sizeof("crypt") - 1), 0, ZSTR_KNOWN(CREX_STR_SENSITIVEPARAMETER), 0);

	crex_add_parameter_attribute(crex_hash_str_find_ptr(CG(function_table), "password_hash", sizeof("password_hash") - 1), 0, ZSTR_KNOWN(CREX_STR_SENSITIVEPARAMETER), 0);

	crex_add_parameter_attribute(crex_hash_str_find_ptr(CG(function_table), "password_verify", sizeof("password_verify") - 1), 0, ZSTR_KNOWN(CREX_STR_SENSITIVEPARAMETER), 0);
}

static crex_class_entry *register_class___CRX_Incomplete_Class(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "__CRX_Incomplete_Class", class___CRX_Incomplete_Class_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_ALLOW_DYNAMIC_PROPERTIES;

	crex_string *attribute_name_AllowDynamicProperties_class___CRX_Incomplete_Class_0 = crex_string_init_interned("AllowDynamicProperties", sizeof("AllowDynamicProperties") - 1, 1);
	crex_add_class_attribute(class_entry, attribute_name_AllowDynamicProperties_class___CRX_Incomplete_Class_0, 0);
	crex_string_release(attribute_name_AllowDynamicProperties_class___CRX_Incomplete_Class_0);

	return class_entry;
}

static crex_class_entry *register_class_AssertionError(crex_class_entry *class_entry_Error)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "AssertionError", class_AssertionError_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Error);

	return class_entry;
}
