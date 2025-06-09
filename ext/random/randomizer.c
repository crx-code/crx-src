/*
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
   | Author: Go Kudo <zeriyoshi@crx.net>                                  |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "crx.h"
#include "crx_random.h"

#include "ext/standard/crx_array.h"
#include "ext/standard/crx_string.h"

#include "Crex/crex_enum.h"
#include "Crex/crex_exceptions.h"

static inline void randomizer_common_init(crx_random_randomizer *randomizer, crex_object *engine_object) {
	if (engine_object->ce->type == CREX_INTERNAL_CLASS) {
		/* Internal classes always crx_random_engine struct */
		crx_random_engine *engine = crx_random_engine_from_obj(engine_object);

		/* Copy engine pointers */
		randomizer->algo = engine->algo;
		randomizer->status = engine->status;
	} else {
		/* Self allocation */
		randomizer->status = crx_random_status_alloc(&crx_random_algo_user, false);
		crx_random_status_state_user *state = randomizer->status->state;
		crex_string *mname;
		crex_function *generate_method;

		mname = ZSTR_INIT_LITERAL("generate", 0);
		generate_method = crex_hash_find_ptr(&engine_object->ce->function_table, mname);
		crex_string_release(mname);

		/* Create compatible state */
		state->object = engine_object;
		state->generate_method = generate_method;

		/* Copy common pointers */
		randomizer->algo = &crx_random_algo_user;

		/* Mark self-allocated for memory management */
		randomizer->is_userland_algo = true;
	}
}

/* {{{ Random\Randomizer::__main() */
CRX_METHOD(Random_Randomizer, __main)
{
	crx_random_randomizer *randomizer = C_RANDOM_RANDOMIZER_P(CREX_THIS);
	zval engine;
	zval *param_engine = NULL;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_OBJECT_OF_CLASS_OR_NULL(param_engine, random_ce_Random_Engine);
	CREX_PARSE_PARAMETERS_END();

	if (param_engine != NULL) {
		ZVAL_COPY(&engine, param_engine);
	} else {
		/* Create default RNG instance */
		object_init_ex(&engine, random_ce_Random_Engine_Secure);
	}

	crex_update_property(random_ce_Random_Randomizer, C_OBJ_P(CREX_THIS), "engine", strlen("engine"), &engine);

	OBJ_RELEASE(C_OBJ_P(&engine));

	if (EG(exception)) {
		RETURN_THROWS();
	}

	randomizer_common_init(randomizer, C_OBJ_P(&engine));
}
/* }}} */

/* {{{ Generate a float in [0, 1) */
CRX_METHOD(Random_Randomizer, nextFloat)
{
	crx_random_randomizer *randomizer = C_RANDOM_RANDOMIZER_P(CREX_THIS);
	uint64_t result;
	size_t total_size;

	CREX_PARSE_PARAMETERS_NONE();

	result = 0;
	total_size = 0;
	do {
		uint64_t r = randomizer->algo->generate(randomizer->status);
		result = result | (r << (total_size * 8));
		total_size += randomizer->status->last_generated_size;
		if (EG(exception)) {
			RETURN_THROWS();
		}
	} while (total_size < sizeof(uint64_t));

	/* A double has 53 bits of precision, thus we must not
	 * use the full 64 bits of the uint64_t, because we would
	 * introduce a bias / rounding error.
	 */
#if DBL_MANT_DIG != 53
# error "Random_Randomizer::nextFloat(): Requires DBL_MANT_DIG == 53 to work."
#endif
	const double step_size = 1.0 / (1ULL << 53);

	/* Use the upper 53 bits, because some engine's lower bits
	 * are of lower quality.
	 */
	result = (result >> 11);

	RETURN_DOUBLE(step_size * result);
}
/* }}} */

/* {{{ Generates a random float within a configurable interval.
 *
 * This method uses the γ-section algorithm by Frédéric Goualard.
 */
CRX_METHOD(Random_Randomizer, getFloat)
{
	crx_random_randomizer *randomizer = C_RANDOM_RANDOMIZER_P(CREX_THIS);
	double min, max;
	crex_object *bounds = NULL;
	int bounds_type = 'C' + sizeof("ClosedOpen") - 1;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_DOUBLE(min)
		C_PARAM_DOUBLE(max)
		C_PARAM_OPTIONAL
		C_PARAM_OBJ_OF_CLASS(bounds, random_ce_Random_IntervalBoundary);
	CREX_PARSE_PARAMETERS_END();

	if (!crex_finite(min)) {
		crex_argument_value_error(1, "must be finite");
		RETURN_THROWS();
	}

	if (!crex_finite(max)) {
		crex_argument_value_error(2, "must be finite");
		RETURN_THROWS();
	}

	if (bounds) {
		zval *case_name = crex_enum_fetch_case_name(bounds);
		crex_string *bounds_name = C_STR_P(case_name);

		bounds_type = ZSTR_VAL(bounds_name)[0] + ZSTR_LEN(bounds_name);
	}

	switch (bounds_type) {
	case 'C' + sizeof("ClosedOpen") - 1:
		if (UNEXPECTED(max <= min)) {
			crex_argument_value_error(2, "must be greater than argument #1 ($min)");
			RETURN_THROWS();
		}

		RETURN_DOUBLE(crx_random_gammasection_closed_open(randomizer->algo, randomizer->status, min, max));
	case 'C' + sizeof("ClosedClosed") - 1:
		if (UNEXPECTED(max < min)) {
			crex_argument_value_error(2, "must be greater than or equal to argument #1 ($min)");
			RETURN_THROWS();
		}

		RETURN_DOUBLE(crx_random_gammasection_closed_closed(randomizer->algo, randomizer->status, min, max));
	case 'O' + sizeof("OpenClosed") - 1:
		if (UNEXPECTED(max <= min)) {
			crex_argument_value_error(2, "must be greater than argument #1 ($min)");
			RETURN_THROWS();
		}

		RETURN_DOUBLE(crx_random_gammasection_open_closed(randomizer->algo, randomizer->status, min, max));
	case 'O' + sizeof("OpenOpen") - 1:
		if (UNEXPECTED(max <= min)) {
			crex_argument_value_error(2, "must be greater than argument #1 ($min)");
			RETURN_THROWS();
		}

		RETVAL_DOUBLE(crx_random_gammasection_open_open(randomizer->algo, randomizer->status, min, max));

		if (UNEXPECTED(isnan(C_DVAL_P(return_value)))) {
			crex_value_error("The given interval is empty, there are no floats between argument #1 ($min) and argument #2 ($max).");
			RETURN_THROWS();
		}

		return;
	default:
		CREX_UNREACHABLE();
	}
}
/* }}} */

/* {{{ Generate positive random number */
CRX_METHOD(Random_Randomizer, nextInt)
{
	crx_random_randomizer *randomizer = C_RANDOM_RANDOMIZER_P(CREX_THIS);
	uint64_t result;

	CREX_PARSE_PARAMETERS_NONE();

	result = randomizer->algo->generate(randomizer->status);
	if (EG(exception)) {
		RETURN_THROWS();
	}
	if (randomizer->status->last_generated_size > sizeof(crex_long)) {
		crex_throw_exception(random_ce_Random_RandomException, "Generated value exceeds size of int", 0);
		RETURN_THROWS();
	}

	RETURN_LONG((crex_long) (result >> 1));
}
/* }}} */

/* {{{ Generate random number in range */
CRX_METHOD(Random_Randomizer, getInt)
{
	crx_random_randomizer *randomizer = C_RANDOM_RANDOMIZER_P(CREX_THIS);
	uint64_t result;
	crex_long min, max;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_LONG(min)
		C_PARAM_LONG(max)
	CREX_PARSE_PARAMETERS_END();

	if (UNEXPECTED(max < min)) {
		crex_argument_value_error(2, "must be greater than or equal to argument #1 ($min)");
		RETURN_THROWS();
	}

	if (UNEXPECTED(
		randomizer->algo->range == crx_random_algo_mt19937.range
		&& ((crx_random_status_state_mt19937 *) randomizer->status->state)->mode != MT_RAND_MT19937
	)) {
		uint64_t r = crx_random_algo_mt19937.generate(randomizer->status) >> 1;

		/* This is an inlined version of the RAND_RANGE_BADSCALING macro that does not invoke UB when encountering
		 * (max - min) > CREX_LONG_MAX.
		 */
		crex_ulong offset = (double) ( (double) max - min + 1.0) * (r / (CRX_MT_RAND_MAX + 1.0));

		result = (crex_long) (offset + min);
	} else {
		result = randomizer->algo->range(randomizer->status, min, max);
	}

	if (EG(exception)) {
		RETURN_THROWS();
	}

	RETURN_LONG((crex_long) result);
}
/* }}} */

/* {{{ Generate random bytes string in ordered length */
CRX_METHOD(Random_Randomizer, getBytes)
{
	crx_random_randomizer *randomizer = C_RANDOM_RANDOMIZER_P(CREX_THIS);
	crex_string *retval;
	crex_long length;
	size_t total_size = 0;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(length)
	CREX_PARSE_PARAMETERS_END();

	if (length < 1) {
		crex_argument_value_error(1, "must be greater than 0");
		RETURN_THROWS();
	}

	retval = crex_string_alloc(length, 0);

	while (total_size < length) {
		uint64_t result = randomizer->algo->generate(randomizer->status);
		if (EG(exception)) {
			crex_string_free(retval);
			RETURN_THROWS();
		}
		for (size_t i = 0; i < randomizer->status->last_generated_size; i++) {
			ZSTR_VAL(retval)[total_size++] = (result >> (i * 8)) & 0xff;
			if (total_size >= length) {
				break;
			}
		}
	}

	ZSTR_VAL(retval)[length] = '\0';
	RETURN_STR(retval);
}
/* }}} */

/* {{{ Shuffling array */
CRX_METHOD(Random_Randomizer, shuffleArray)
{
	crx_random_randomizer *randomizer = C_RANDOM_RANDOMIZER_P(CREX_THIS);
	zval *array;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ARRAY(array)
	CREX_PARSE_PARAMETERS_END();

	ZVAL_DUP(return_value, array);
	if (!crx_array_data_shuffle(randomizer->algo, randomizer->status, return_value)) {
		RETURN_THROWS();
	}
}
/* }}} */

/* {{{ Shuffling binary */
CRX_METHOD(Random_Randomizer, shuffleBytes)
{
	crx_random_randomizer *randomizer = C_RANDOM_RANDOMIZER_P(CREX_THIS);
	crex_string *bytes;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(bytes)
	CREX_PARSE_PARAMETERS_END();

	if (ZSTR_LEN(bytes) < 2) {
		RETURN_STR_COPY(bytes);
	}

	RETVAL_STRINGL(ZSTR_VAL(bytes), ZSTR_LEN(bytes));
	if (!crx_binary_string_shuffle(randomizer->algo, randomizer->status, C_STRVAL_P(return_value), (crex_long) C_STRLEN_P(return_value))) {
		RETURN_THROWS();
	}
}
/* }}} */

/* {{{ Pick keys */
CRX_METHOD(Random_Randomizer, pickArrayKeys)
{
	crx_random_randomizer *randomizer = C_RANDOM_RANDOMIZER_P(CREX_THIS);
	zval *input, t;
	crex_long num_req;

	CREX_PARSE_PARAMETERS_START(2, 2);
		C_PARAM_ARRAY(input)
		C_PARAM_LONG(num_req)
	CREX_PARSE_PARAMETERS_END();

	if (!crx_array_pick_keys(
		randomizer->algo,
		randomizer->status,
		input,
		num_req,
		return_value,
		false)
	) {
		RETURN_THROWS();
	}

	/* Keep compatibility, But the result is always an array */
	if (C_TYPE_P(return_value) != IS_ARRAY) {
		ZVAL_COPY_VALUE(&t, return_value);
		array_init(return_value);
		crex_hash_next_index_insert(C_ARRVAL_P(return_value), &t);
	}
}
/* }}} */

/* {{{ Get Random Bytes for String */
CRX_METHOD(Random_Randomizer, getBytesFromString)
{
	crx_random_randomizer *randomizer = C_RANDOM_RANDOMIZER_P(CREX_THIS);
	crex_long length;
	crex_string *source, *retval;
	size_t total_size = 0;

	CREX_PARSE_PARAMETERS_START(2, 2);
		C_PARAM_STR(source)
		C_PARAM_LONG(length)
	CREX_PARSE_PARAMETERS_END();

	const size_t source_length = ZSTR_LEN(source);
	const size_t max_offset = source_length - 1;

	if (source_length < 1) {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	}

	if (length < 1) {
		crex_argument_value_error(2, "must be greater than 0");
		RETURN_THROWS();
	}

	retval = crex_string_alloc(length, 0);

	if (max_offset > 0xff) {
		while (total_size < length) {
			uint64_t offset = randomizer->algo->range(randomizer->status, 0, max_offset);

			if (EG(exception)) {
				crex_string_free(retval);
				RETURN_THROWS();
			}

			ZSTR_VAL(retval)[total_size++] = ZSTR_VAL(source)[offset];
		}
	} else {
		uint64_t mask = max_offset;
		// Copy the top-most bit into all lower bits.
		// Shifting by 4 is sufficient, because max_offset
		// is guaranteed to fit in an 8-bit integer at this
		// point.
		mask |= mask >> 1;
		mask |= mask >> 2;
		mask |= mask >> 4;

		int failures = 0;
		while (total_size < length) {
			uint64_t result = randomizer->algo->generate(randomizer->status);
			if (EG(exception)) {
				crex_string_free(retval);
				RETURN_THROWS();
			}

			for (size_t i = 0; i < randomizer->status->last_generated_size; i++) {
				uint64_t offset = (result >> (i * 8)) & mask;

				if (offset > max_offset) {
					if (++failures > CRX_RANDOM_RANGE_ATTEMPTS) {
						crex_string_free(retval);
						crex_throw_error(random_ce_Random_BrokenRandomEngineError, "Failed to generate an acceptable random number in %d attempts", CRX_RANDOM_RANGE_ATTEMPTS);
						RETURN_THROWS();
					}

					continue;
				}

				failures = 0;

				ZSTR_VAL(retval)[total_size++] = ZSTR_VAL(source)[offset];
				if (total_size >= length) {
					break;
				}
			}
		}
	}

	ZSTR_VAL(retval)[length] = '\0';
	RETURN_STR(retval);
}
/* }}} */

/* {{{ Random\Randomizer::__serialize() */
CRX_METHOD(Random_Randomizer, __serialize)
{
	crx_random_randomizer *randomizer = C_RANDOM_RANDOMIZER_P(CREX_THIS);
	zval t;

	CREX_PARSE_PARAMETERS_NONE();

	array_init(return_value);
	ZVAL_ARR(&t, crex_std_get_properties(&randomizer->std));
	C_TRY_ADDREF(t);
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &t);
}
/* }}} */

/* {{{ Random\Randomizer::__unserialize() */
CRX_METHOD(Random_Randomizer, __unserialize)
{
	crx_random_randomizer *randomizer = C_RANDOM_RANDOMIZER_P(CREX_THIS);
	HashTable *d;
	zval *members_zv;
	zval *zengine;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ARRAY_HT(d);
	CREX_PARSE_PARAMETERS_END();

	/* Verify the expected number of elements, this implicitly ensures that no additional elements are present. */
	if (crex_hash_num_elements(d) != 1) {
		crex_throw_exception(NULL, "Invalid serialization data for Random\\Randomizer object", 0);
		RETURN_THROWS();
	}

	members_zv = crex_hash_index_find(d, 0);
	if (!members_zv || C_TYPE_P(members_zv) != IS_ARRAY) {
		crex_throw_exception(NULL, "Invalid serialization data for Random\\Randomizer object", 0);
		RETURN_THROWS();
	}
	object_properties_load(&randomizer->std, C_ARRVAL_P(members_zv));
	if (EG(exception)) {
		crex_throw_exception(NULL, "Invalid serialization data for Random\\Randomizer object", 0);
		RETURN_THROWS();
	}

	zengine = crex_read_property(randomizer->std.ce, &randomizer->std, "engine", strlen("engine"), 1, NULL);
	if (C_TYPE_P(zengine) != IS_OBJECT || !instanceof_function(C_OBJCE_P(zengine), random_ce_Random_Engine)) {
		crex_throw_exception(NULL, "Invalid serialization data for Random\\Randomizer object", 0);
		RETURN_THROWS();
	}

	randomizer_common_init(randomizer, C_OBJ_P(zengine));
}
/* }}} */
