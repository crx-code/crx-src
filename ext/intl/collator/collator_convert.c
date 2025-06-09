/*
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the CRX license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | https://www.crx.net/license/3_01.txt                                 |
   | If you did not receive a copy of the CRX license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crx.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Vadim Savchuk <vsavchuk@productengine.com>                  |
   |          Dmitry Lakhtyuk <dlakhtyuk@productengine.com>               |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx_intl.h"
#include "collator_class.h"
#include "collator_is_numeric.h"
#include "collator_convert.h"
#include "intl_convert.h"

#include <unicode/ustring.h>
#include <crx.h>

#define COLLATOR_CONVERT_RETURN_FAILED(retval) { \
			C_TRY_ADDREF_P(retval);              \
			return retval;                       \
	}

/* {{{ collator_convert_hash_item_from_utf8_to_utf16 */
static void collator_convert_hash_item_from_utf8_to_utf16(
	HashTable* hash, zval *hashData, crex_string *hashKey, crex_ulong hashIndex,
	UErrorCode* status )
{
	const char* old_val;
	size_t      old_val_len;
	UChar*      new_val      = NULL;
	int32_t     new_val_len  = 0;
	zval        znew_val;

	/* Process string values only. */
	if( C_TYPE_P( hashData ) != IS_STRING )
		return;

	old_val     = C_STRVAL_P( hashData );
	old_val_len = C_STRLEN_P( hashData );

	/* Convert it from UTF-8 to UTF-16LE and save the result to new_val[_len]. */
	intl_convert_utf8_to_utf16( &new_val, &new_val_len, old_val, old_val_len, status );
	if( U_FAILURE( *status ) )
		return;

	/* Update current hash item with the converted value. */
	ZVAL_STRINGL( &znew_val, (char*)new_val, UBYTES(new_val_len + 1) );
	//???
	efree(new_val);
	/* hack to fix use of initialized value */
	C_STRLEN(znew_val) = C_STRLEN(znew_val) - UBYTES(1);

	if( hashKey)
	{
		crex_hash_update( hash, hashKey, &znew_val);
	}
	else /* hashKeyType == HASH_KEY_IS_LONG */
	{
		crex_hash_index_update( hash, hashIndex, &znew_val);
	}
}
/* }}} */

/* {{{ collator_convert_hash_item_from_utf16_to_utf8 */
static void collator_convert_hash_item_from_utf16_to_utf8(
	HashTable* hash, zval * hashData, crex_string* hashKey, crex_ulong hashIndex,
	UErrorCode* status )
{
	const char* old_val;
	size_t      old_val_len;
	crex_string* u8str;
	zval        znew_val;

	/* Process string values only. */
	if( C_TYPE_P( hashData ) != IS_STRING )
		return;

	old_val     = C_STRVAL_P( hashData );
	old_val_len = C_STRLEN_P( hashData );

	/* Convert it from UTF-16LE to UTF-8 and save the result to new_val[_len]. */
	u8str = intl_convert_utf16_to_utf8(
		(UChar*)old_val, UCHARS(old_val_len), status );
	if( !u8str )
		return;

	/* Update current hash item with the converted value. */
	ZVAL_NEW_STR( &znew_val, u8str);

	if( hashKey )
	{
		crex_hash_update( hash, hashKey, &znew_val);
	}
	else /* hashKeyType == HASH_KEY_IS_LONG */
	{
		crex_hash_index_update( hash, hashIndex, &znew_val);
	}
}
/* }}} */

/* {{{ collator_convert_hash_from_utf8_to_utf16
 *  Convert values of the given hash from UTF-8 encoding to UTF-16LE.
 */
void collator_convert_hash_from_utf8_to_utf16( HashTable* hash, UErrorCode* status )
{
	crex_ulong    hashIndex;
	zval *hashData;
	crex_string *hashKey;

	CREX_HASH_FOREACH_KEY_VAL(hash, hashIndex, hashKey, hashData) {
		/* Convert current hash item from UTF-8 to UTF-16LE. */
		collator_convert_hash_item_from_utf8_to_utf16(
			hash, hashData, hashKey, hashIndex, status );
		if( U_FAILURE( *status ) )
			return;
	} CREX_HASH_FOREACH_END();
}
/* }}} */

/* {{{ collator_convert_hash_from_utf16_to_utf8
 * Convert values of the given hash from UTF-16LE encoding to UTF-8.
 */
void collator_convert_hash_from_utf16_to_utf8( HashTable* hash, UErrorCode* status )
{
	crex_ulong hashIndex;
	crex_string *hashKey;
	zval *hashData;

	CREX_HASH_FOREACH_KEY_VAL(hash, hashIndex, hashKey, hashData) {
		/* Convert current hash item from UTF-16LE to UTF-8. */
		collator_convert_hash_item_from_utf16_to_utf8(
			hash, hashData, hashKey, hashIndex, status );
		if( U_FAILURE( *status ) ) {
			return;
		}
	} CREX_HASH_FOREACH_END();
}
/* }}} */

/* {{{ collator_convert_zstr_utf16_to_utf8
 *
 * Convert string from utf16 to utf8.
 *
 * @param  zval* utf16_zval String to convert.
 *
 * @return zval* Converted string.
 */
zval* collator_convert_zstr_utf16_to_utf8( zval* utf16_zval, zval *rv )
{
	crex_string* u8str;
	UErrorCode status = U_ZERO_ERROR;

	/* Convert to utf8 then. */
	u8str = intl_convert_utf16_to_utf8(
		(UChar*) C_STRVAL_P(utf16_zval), UCHARS( C_STRLEN_P(utf16_zval) ), &status );
	if( !u8str ) {
		crx_error( E_WARNING, "Error converting utf16 to utf8 in collator_convert_zval_utf16_to_utf8()" );
		ZVAL_EMPTY_STRING( rv );
	} else {
		ZVAL_NEW_STR( rv, u8str );
	}
	return rv;
}
/* }}} */

crex_string *collator_convert_zstr_utf8_to_utf16(crex_string *utf8_str)
{
	UChar *ustr = NULL;
	int32_t ustr_len = 0;
	UErrorCode status = U_ZERO_ERROR;

	/* Convert the string to UTF-16. */
	intl_convert_utf8_to_utf16(
			&ustr, &ustr_len,
			ZSTR_VAL(utf8_str), ZSTR_LEN(utf8_str),
			&status);
	// FIXME Or throw error or use intl internal error handler
	if (U_FAILURE(status)) {
		crx_error(E_WARNING,
			"Error casting object to string in collator_convert_zstr_utf8_to_utf16()");
	}

	crex_string *zstr = crex_string_init((char *) ustr, UBYTES(ustr_len), 0);
	efree((char *)ustr);
	return zstr;
}

/* {{{ collator_convert_object_to_string
 * Convert object to UTF16-encoded string.
 */
zval* collator_convert_object_to_string( zval* obj, zval *rv )
{
	zval* zstr        = NULL;
	UErrorCode status = U_ZERO_ERROR;
	UChar* ustr       = NULL;
	int32_t ustr_len  = 0;

	/* Bail out if it's not an object. */
	if( C_TYPE_P( obj ) != IS_OBJECT )
	{
		COLLATOR_CONVERT_RETURN_FAILED( obj );
	}

	/* Try object's handlers. */
	zstr = rv;

	if( C_OBJ_HT_P(obj)->cast_object( C_OBJ_P(obj), zstr, IS_STRING ) == FAILURE )
	{
		/* cast_object failed => bail out. */
		zval_ptr_dtor( zstr );
		COLLATOR_CONVERT_RETURN_FAILED( obj );
	}

	/* Object wasn't successfully converted => bail out. */
	if( zstr == NULL )
	{
		COLLATOR_CONVERT_RETURN_FAILED( obj );
	}

	/* Convert the string to UTF-16. */
	intl_convert_utf8_to_utf16(
			&ustr, &ustr_len,
			C_STRVAL_P( zstr ), C_STRLEN_P( zstr ),
			&status );
	// FIXME Or throw error or use intl internal error handler
	if( U_FAILURE( status ) )
		crx_error( E_WARNING, "Error casting object to string in collator_convert_object_to_string()" );

	/* Cleanup zstr to hold utf16 string. */
	zval_ptr_dtor_str( zstr );

	/* Set string. */
	ZVAL_STRINGL( zstr, (char*)ustr, UBYTES(ustr_len));
	//???
	efree((char *)ustr);

	/* Don't free ustr cause it's set in zstr without copy.
	 * efree( ustr );
	 */

	return zstr;
}
/* }}} */

/* {{{ collator_convert_string_to_number
 *
 * Convert string to number.
 *
 * @param  zval* str String to convert.
 *
 * @return zval* Number. If str is not numeric string return number zero.
 */
zval* collator_convert_string_to_number( zval* str, zval *rv )
{
	zval* num = collator_convert_string_to_number_if_possible( str, rv );
	if( num == str )
	{
		/* String wasn't converted => return zero. */
		zval_ptr_dtor( num );

		num = rv;
		ZVAL_LONG( num, 0 );
	}

	return num;
}
/* }}} */

/* {{{ collator_convert_string_to_double
 *
 * Convert string to double.
 *
 * @param  zval* str String to convert.
 *
 * @return zval* Number. If str is not numeric string return number zero.
 */
zval* collator_convert_string_to_double( zval* str, zval *rv )
{
	zval* num = collator_convert_string_to_number( str, rv );
	if( C_TYPE_P(num) == IS_LONG )
	{
		ZVAL_DOUBLE( num, C_LVAL_P( num ) );
	}

	return num;
}
/* }}} */

/* {{{ collator_convert_string_to_number_if_possible
 *
 * Convert string to numer.
 *
 * @param  zval* str String to convert.
 *
 * @return zval* Number if str is numeric string. Otherwise
 *               original str param.
 */
zval* collator_convert_string_to_number_if_possible( zval* str, zval *rv )
{
	uint8_t is_numeric = 0;
	crex_long lval      = 0;
	double dval    = 0;

	if( C_TYPE_P( str ) != IS_STRING )
	{
		COLLATOR_CONVERT_RETURN_FAILED( str );
	}

	if ( ( is_numeric = collator_is_numeric( (UChar*) C_STRVAL_P(str), UCHARS( C_STRLEN_P(str) ), &lval, &dval, /* allow_errors */ 1 ) ) )
	{
		if( is_numeric == IS_LONG ) {
			ZVAL_LONG(rv, lval);
		}
		if( is_numeric == IS_DOUBLE )
			ZVAL_DOUBLE(rv, dval);
	}
	else
	{
		COLLATOR_CONVERT_RETURN_FAILED( str );
	}

	return rv;
}
/* }}} */

/* Returns string from input zval.
 *
 * @param  zval* arg zval to get string from
 *
 * @return crex_string* UTF16 string.
 */
crex_string *collator_zval_to_string(zval *arg)
{
	// TODO: This is extremely weird in that it leaves pre-existing strings alone and does not
	// perform a UTF-8 to UTF-16 conversion for them. The assumption is that values that are
	// already strings have already been converted beforehand. It would be good to clean this up.
	if (C_TYPE_P(arg) == IS_STRING) {
		return crex_string_copy(C_STR_P(arg));
	}

	crex_string *utf8_str = zval_get_string(arg);
	crex_string *utf16_str = collator_convert_zstr_utf8_to_utf16(utf8_str);
	crex_string_release(utf8_str);
	return utf16_str;
}

/* {{{ collator_normalize_sort_argument
 *
 * Normalize argument to use in sort's compare function.
 *
 * @param  zval* arg Sort's argument to normalize.
 *
 * @return zval* Normalized copy of arg or unmodified arg
 *               if normalization is not needed.
 */
zval* collator_normalize_sort_argument( zval* arg, zval *rv )
{
	zval* n_arg = NULL;

	if( C_TYPE_P( arg ) != IS_STRING )
	{
		/* If it's not a string then nothing to do.
		 * Return original arg.
		 */
		COLLATOR_CONVERT_RETURN_FAILED( arg );
	}

	/* Try convert to number. */
	n_arg = collator_convert_string_to_number_if_possible( arg, rv );

	if( n_arg == arg )
	{
		/* Conversion to number failed. */
		zval_ptr_dtor( n_arg );

		/* Convert string to utf8. */
		n_arg = collator_convert_zstr_utf16_to_utf8( arg, rv );
	}

	return n_arg;
}
/* }}} */
