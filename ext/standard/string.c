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
   | Authors: Rasmus Lerdorf <rasmus@crx.net>                             |
   |          Stig Sæther Bakken <ssb@crx.net>                          |
   |          Zeev Suraski <zeev@crx.net>                                 |
   +----------------------------------------------------------------------+
 */

#include <stdio.h>
#include "crx.h"
#include "crx_string.h"
#include "crx_variables.h"
#include <locale.h>
#ifdef HAVE_LANGINFO_H
# include <langinfo.h>
#endif

#ifdef HAVE_LIBINTL
# include <libintl.h> /* For LC_MESSAGES */
#endif

#include "scanf.h"
#include "crex_API.h"
#include "crex_execute.h"
#include "crx_globals.h"
#include "basic_functions.h"
#include "crex_smart_str.h"
#include <Crex/crex_exceptions.h>
#ifdef ZTS
#include "TSRM.h"
#endif

/* For str_getcsv() support */
#include "ext/standard/file.h"
/* For crx_next_utf8_char() */
#include "ext/standard/html.h"
#include "ext/random/crx_random.h"

#ifdef __SSE2__
#include <emmintrin.h>
#include "Crex/crex_bitset.h"
#endif

/* this is read-only, so it's ok */
CREX_SET_ALIGNED(16, static const char hexconvtab[]) = "0123456789abcdef";

/* localeconv mutex */
#ifdef ZTS
static MUTEX_T locale_mutex = NULL;
#endif

/* {{{ crx_bin2hex */
static crex_string *crx_bin2hex(const unsigned char *old, const size_t oldlen)
{
	crex_string *result;
	size_t i, j;

	result = crex_string_safe_alloc(oldlen, 2 * sizeof(char), 0, 0);

	for (i = j = 0; i < oldlen; i++) {
		ZSTR_VAL(result)[j++] = hexconvtab[old[i] >> 4];
		ZSTR_VAL(result)[j++] = hexconvtab[old[i] & 15];
	}
	ZSTR_VAL(result)[j] = '\0';

	return result;
}
/* }}} */

/* {{{ crx_hex2bin */
static crex_string *crx_hex2bin(const unsigned char *old, const size_t oldlen)
{
	size_t target_length = oldlen >> 1;
	crex_string *str = crex_string_alloc(target_length, 0);
	unsigned char *ret = (unsigned char *)ZSTR_VAL(str);
	size_t i, j;

	for (i = j = 0; i < target_length; i++) {
		unsigned char c = old[j++];
		unsigned char l = c & ~0x20;
		int is_letter = ((unsigned int) ((l - 'A') ^ (l - 'F' - 1))) >> (8 * sizeof(unsigned int) - 1);
		unsigned char d;

		/* basically (c >= '0' && c <= '9') || (l >= 'A' && l <= 'F') */
		if (EXPECTED((((c ^ '0') - 10) >> (8 * sizeof(unsigned int) - 1)) | is_letter)) {
			d = (l - 0x10 - 0x27 * is_letter) << 4;
		} else {
			crex_string_efree(str);
			return NULL;
		}
		c = old[j++];
		l = c & ~0x20;
		is_letter = ((unsigned int) ((l - 'A') ^ (l - 'F' - 1))) >> (8 * sizeof(unsigned int) - 1);
		if (EXPECTED((((c ^ '0') - 10) >> (8 * sizeof(unsigned int) - 1)) | is_letter)) {
			d |= l - 0x10 - 0x27 * is_letter;
		} else {
			crex_string_efree(str);
			return NULL;
		}
		ret[i] = d;
	}
	ret[i] = '\0';

	return str;
}
/* }}} */

/* {{{ localeconv_r
 * glibc's localeconv is not reentrant, so lets make it so ... sorta */
CRXAPI struct lconv *localeconv_r(struct lconv *out)
{

#ifdef ZTS
	tsrm_mutex_lock( locale_mutex );
#endif

/*  cur->locinfo is struct __crt_locale_info which implementation is
	hidden in vc14. TODO revisit this and check if a workaround available
	and needed. */
#if defined(CRX_WIN32) && _MSC_VER < 1900 && defined(ZTS)
	{
		/* Even with the enabled per thread locale, localeconv
			won't check any locale change in the master thread. */
		_locale_t cur = _get_current_locale();
		*out = *cur->locinfo->lconv;
		_free_locale(cur);
	}
#else
	/* localeconv doesn't return an error condition */
	*out = *localeconv();
#endif

#ifdef ZTS
	tsrm_mutex_unlock( locale_mutex );
#endif

	return out;
}
/* }}} */

#ifdef ZTS
/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(localeconv)
{
	locale_mutex = tsrm_mutex_alloc();
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION */
CRX_MSHUTDOWN_FUNCTION(localeconv)
{
	tsrm_mutex_free( locale_mutex );
	locale_mutex = NULL;
	return SUCCESS;
}
/* }}} */
#endif

/* {{{ Converts the binary representation of data to hex */
CRX_FUNCTION(bin2hex)
{
	crex_string *result;
	crex_string *data;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(data)
	CREX_PARSE_PARAMETERS_END();

	result = crx_bin2hex((unsigned char *)ZSTR_VAL(data), ZSTR_LEN(data));

	RETURN_STR(result);
}
/* }}} */

/* {{{ Converts the hex representation of data to binary */
CRX_FUNCTION(hex2bin)
{
	crex_string *result, *data;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(data)
	CREX_PARSE_PARAMETERS_END();

	if (ZSTR_LEN(data) % 2 != 0) {
		crx_error_docref(NULL, E_WARNING, "Hexadecimal input string must have an even length");
		RETURN_FALSE;
	}

	result = crx_hex2bin((unsigned char *)ZSTR_VAL(data), ZSTR_LEN(data));

	if (!result) {
		crx_error_docref(NULL, E_WARNING, "Input string must be hexadecimal string");
		RETURN_FALSE;
	}

	RETVAL_STR(result);
}
/* }}} */

static void crx_spn_common_handler(INTERNAL_FUNCTION_PARAMETERS, int behavior) /* {{{ */
{
	crex_string *s11, *s22;
	crex_long start = 0, len = 0;
	bool len_is_null = 1;

	CREX_PARSE_PARAMETERS_START(2, 4)
		C_PARAM_STR(s11)
		C_PARAM_STR(s22)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(start)
		C_PARAM_LONG_OR_NULL(len, len_is_null)
	CREX_PARSE_PARAMETERS_END();

	size_t remain_len = ZSTR_LEN(s11);
	if (start < 0) {
		start += remain_len;
		if (start < 0) {
			start = 0;
		}
	} else if ((size_t) start > remain_len) {
		start = remain_len;
	}

	remain_len -= start;
	if (!len_is_null) {
		if (len < 0) {
			len += remain_len;
			if (len < 0) {
				len = 0;
			}
		} else if ((size_t) len > remain_len) {
			len = remain_len;
		}
	} else {
		len = remain_len;
	}

	if (len == 0) {
		RETURN_LONG(0);
	}

	if (behavior == CRX_STR_STRSPN) {
		RETURN_LONG(crx_strspn(ZSTR_VAL(s11) + start /*str1_start*/,
						ZSTR_VAL(s22) /*str2_start*/,
						ZSTR_VAL(s11) + start + len /*str1_end*/,
						ZSTR_VAL(s22) + ZSTR_LEN(s22) /*str2_end*/));
	} else {
		CREX_ASSERT(behavior == CRX_STR_STRCSPN);
		RETURN_LONG(crx_strcspn(ZSTR_VAL(s11) + start /*str1_start*/,
						ZSTR_VAL(s22) /*str2_start*/,
						ZSTR_VAL(s11) + start + len /*str1_end*/,
						ZSTR_VAL(s22) + ZSTR_LEN(s22) /*str2_end*/));
	}
}
/* }}} */

/* {{{ Finds length of initial segment consisting entirely of characters found in mask. If start or/and length is provided works like strspn(substr($s,$start,$len),$good_chars) */
CRX_FUNCTION(strspn)
{
	crx_spn_common_handler(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_STR_STRSPN);
}
/* }}} */

/* {{{ Finds length of initial segment consisting entirely of characters not found in mask. If start or/and length is provide works like strcspn(substr($s,$start,$len),$bad_chars) */
CRX_FUNCTION(strcspn)
{
	crx_spn_common_handler(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_STR_STRCSPN);
}
/* }}} */

#ifdef HAVE_NL_LANGINFO
/* {{{ Query language and locale information */
CRX_FUNCTION(nl_langinfo)
{
	crex_long item;
	char *value;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(item)
	CREX_PARSE_PARAMETERS_END();

	switch(item) { /* {{{ */
#ifdef ABDAY_1
		case ABDAY_1:
		case ABDAY_2:
		case ABDAY_3:
		case ABDAY_4:
		case ABDAY_5:
		case ABDAY_6:
		case ABDAY_7:
#endif
#ifdef DAY_1
		case DAY_1:
		case DAY_2:
		case DAY_3:
		case DAY_4:
		case DAY_5:
		case DAY_6:
		case DAY_7:
#endif
#ifdef ABMON_1
		case ABMON_1:
		case ABMON_2:
		case ABMON_3:
		case ABMON_4:
		case ABMON_5:
		case ABMON_6:
		case ABMON_7:
		case ABMON_8:
		case ABMON_9:
		case ABMON_10:
		case ABMON_11:
		case ABMON_12:
#endif
#ifdef MON_1
		case MON_1:
		case MON_2:
		case MON_3:
		case MON_4:
		case MON_5:
		case MON_6:
		case MON_7:
		case MON_8:
		case MON_9:
		case MON_10:
		case MON_11:
		case MON_12:
#endif
#ifdef AM_STR
		case AM_STR:
#endif
#ifdef PM_STR
		case PM_STR:
#endif
#ifdef D_T_FMT
		case D_T_FMT:
#endif
#ifdef D_FMT
		case D_FMT:
#endif
#ifdef T_FMT
		case T_FMT:
#endif
#ifdef T_FMT_AMPM
		case T_FMT_AMPM:
#endif
#ifdef ERA
		case ERA:
#endif
#ifdef ERA_YEAR
		case ERA_YEAR:
#endif
#ifdef ERA_D_T_FMT
		case ERA_D_T_FMT:
#endif
#ifdef ERA_D_FMT
		case ERA_D_FMT:
#endif
#ifdef ERA_T_FMT
		case ERA_T_FMT:
#endif
#ifdef ALT_DIGITS
		case ALT_DIGITS:
#endif
#ifdef INT_CURR_SYMBOL
		case INT_CURR_SYMBOL:
#endif
#ifdef CURRENCY_SYMBOL
		case CURRENCY_SYMBOL:
#endif
#ifdef CRNCYSTR
		case CRNCYSTR:
#endif
#ifdef MON_DECIMAL_POINT
		case MON_DECIMAL_POINT:
#endif
#ifdef MON_THOUSANDS_SEP
		case MON_THOUSANDS_SEP:
#endif
#ifdef MON_GROUPING
		case MON_GROUPING:
#endif
#ifdef POSITIVE_SIGN
		case POSITIVE_SIGN:
#endif
#ifdef NEGATIVE_SIGN
		case NEGATIVE_SIGN:
#endif
#ifdef INT_FRAC_DIGITS
		case INT_FRAC_DIGITS:
#endif
#ifdef FRAC_DIGITS
		case FRAC_DIGITS:
#endif
#ifdef P_CS_PRECEDES
		case P_CS_PRECEDES:
#endif
#ifdef P_SEP_BY_SPACE
		case P_SEP_BY_SPACE:
#endif
#ifdef N_CS_PRECEDES
		case N_CS_PRECEDES:
#endif
#ifdef N_SEP_BY_SPACE
		case N_SEP_BY_SPACE:
#endif
#ifdef P_SIGN_POSN
		case P_SIGN_POSN:
#endif
#ifdef N_SIGN_POSN
		case N_SIGN_POSN:
#endif
#ifdef DECIMAL_POINT
		case DECIMAL_POINT:
#elif defined(RADIXCHAR)
		case RADIXCHAR:
#endif
#ifdef THOUSANDS_SEP
		case THOUSANDS_SEP:
#elif defined(THOUSEP)
		case THOUSEP:
#endif
#ifdef GROUPING
		case GROUPING:
#endif
#ifdef YESEXPR
		case YESEXPR:
#endif
#ifdef NOEXPR
		case NOEXPR:
#endif
#ifdef YESSTR
		case YESSTR:
#endif
#ifdef NOSTR
		case NOSTR:
#endif
#ifdef CODESET
		case CODESET:
#endif
			break;
		default:
			crx_error_docref(NULL, E_WARNING, "Item '" CREX_LONG_FMT "' is not valid", item);
			RETURN_FALSE;
	}
	/* }}} */

	value = nl_langinfo(item);
	if (value == NULL) {
		RETURN_FALSE;
	} else {
		RETURN_STRING(value);
	}
}
#endif
/* }}} */

/* {{{ Compares two strings using the current locale */
CRX_FUNCTION(strcoll)
{
	crex_string *s1, *s2;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_STR(s1)
		C_PARAM_STR(s2)
	CREX_PARSE_PARAMETERS_END();

	RETURN_LONG(strcoll((const char *) ZSTR_VAL(s1),
	                    (const char *) ZSTR_VAL(s2)));
}
/* }}} */

/* {{{ crx_charmask
 * Fills a 256-byte bytemask with input. You can specify a range like 'a..z',
 * it needs to be incrementing.
 * Returns: FAILURE/SUCCESS whether the input was correct (i.e. no range errors)
 */
static inline crex_result crx_charmask(const unsigned char *input, size_t len, char *mask)
{
	const unsigned char *end;
	unsigned char c;
	crex_result result = SUCCESS;

	memset(mask, 0, 256);
	for (end = input+len; input < end; input++) {
		c=*input;
		if ((input+3 < end) && input[1] == '.' && input[2] == '.'
				&& input[3] >= c) {
			memset(mask+c, 1, input[3] - c + 1);
			input+=3;
		} else if ((input+1 < end) && input[0] == '.' && input[1] == '.') {
			/* Error, try to be as helpful as possible:
			   (a range ending/starting with '.' won't be captured here) */
			if (end-len >= input) { /* there was no 'left' char */
				crx_error_docref(NULL, E_WARNING, "Invalid '..'-range, no character to the left of '..'");
				result = FAILURE;
				continue;
			}
			if (input+2 >= end) { /* there is no 'right' char */
				crx_error_docref(NULL, E_WARNING, "Invalid '..'-range, no character to the right of '..'");
				result = FAILURE;
				continue;
			}
			if (input[-1] > input[2]) { /* wrong order */
				crx_error_docref(NULL, E_WARNING, "Invalid '..'-range, '..'-range needs to be incrementing");
				result = FAILURE;
				continue;
			}
			/* FIXME: better error (a..b..c is the only left possibility?) */
			crx_error_docref(NULL, E_WARNING, "Invalid '..'-range");
			result = FAILURE;
			continue;
		} else {
			mask[c]=1;
		}
	}
	return result;
}
/* }}} */

/* {{{ crx_trim_int()
 * mode 1 : trim left
 * mode 2 : trim right
 * mode 3 : trim left and right
 * what indicates which chars are to be trimmed. NULL->default (' \t\n\r\v\0')
 */
static crex_always_inline crex_string *crx_trim_int(crex_string *str, const char *what, size_t what_len, int mode)
{
	const char *start = ZSTR_VAL(str);
	const char *end = start + ZSTR_LEN(str);
	char mask[256];

	if (what) {
		if (what_len == 1) {
			char p = *what;
			if (mode & 1) {
				while (start != end) {
					if (*start == p) {
						start++;
					} else {
						break;
					}
				}
			}
			if (mode & 2) {
				while (start != end) {
					if (*(end-1) == p) {
						end--;
					} else {
						break;
					}
				}
			}
		} else {
			crx_charmask((const unsigned char *) what, what_len, mask);

			if (mode & 1) {
				while (start != end) {
					if (mask[(unsigned char)*start]) {
						start++;
					} else {
						break;
					}
				}
			}
			if (mode & 2) {
				while (start != end) {
					if (mask[(unsigned char)*(end-1)]) {
						end--;
					} else {
						break;
					}
				}
			}
		}
	} else {
		if (mode & 1) {
			while (start != end) {
				unsigned char c = (unsigned char)*start;

				if (c <= ' ' &&
				    (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v' || c == '\0')) {
					start++;
				} else {
					break;
				}
			}
		}
		if (mode & 2) {
			while (start != end) {
				unsigned char c = (unsigned char)*(end-1);

				if (c <= ' ' &&
				    (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v' || c == '\0')) {
					end--;
				} else {
					break;
				}
			}
		}
	}

	if (ZSTR_LEN(str) == end - start) {
		return crex_string_copy(str);
	} else if (end - start == 0) {
		return ZSTR_EMPTY_ALLOC();
	} else {
		return crex_string_init(start, end - start, 0);
	}
}
/* }}} */

/* {{{ crx_trim_int()
 * mode 1 : trim left
 * mode 2 : trim right
 * mode 3 : trim left and right
 * what indicates which chars are to be trimmed. NULL->default (' \t\n\r\v\0')
 */
CRXAPI crex_string *crx_trim(crex_string *str, const char *what, size_t what_len, int mode)
{
	return crx_trim_int(str, what, what_len, mode);
}
/* }}} */

/* {{{ crx_do_trim
 * Base for trim(), rtrim() and ltrim() functions.
 */
static crex_always_inline void crx_do_trim(INTERNAL_FUNCTION_PARAMETERS, int mode)
{
	crex_string *str;
	crex_string *what = NULL;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STR(str)
		C_PARAM_OPTIONAL
		C_PARAM_STR(what)
	CREX_PARSE_PARAMETERS_END();

	ZVAL_STR(return_value, crx_trim_int(str, (what ? ZSTR_VAL(what) : NULL), (what ? ZSTR_LEN(what) : 0), mode));
}
/* }}} */

/* {{{ Strips whitespace from the beginning and end of a string */
CRX_FUNCTION(trim)
{
	crx_do_trim(INTERNAL_FUNCTION_PARAM_PASSTHRU, 3);
}
/* }}} */

/* {{{ Removes trailing whitespace */
CRX_FUNCTION(rtrim)
{
	crx_do_trim(INTERNAL_FUNCTION_PARAM_PASSTHRU, 2);
}
/* }}} */

/* {{{ Strips whitespace from the beginning of a string */
CRX_FUNCTION(ltrim)
{
	crx_do_trim(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ Wraps buffer to selected number of characters using string break char */
CRX_FUNCTION(wordwrap)
{
	crex_string *text;
	char *breakchar = "\n";
	size_t newtextlen, chk, breakchar_len = 1;
	size_t alloced;
	crex_long current = 0, laststart = 0, lastspace = 0;
	crex_long linelength = 75;
	bool docut = 0;
	crex_string *newtext;

	CREX_PARSE_PARAMETERS_START(1, 4)
		C_PARAM_STR(text)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(linelength)
		C_PARAM_STRING(breakchar, breakchar_len)
		C_PARAM_BOOL(docut)
	CREX_PARSE_PARAMETERS_END();

	if (ZSTR_LEN(text) == 0) {
		RETURN_EMPTY_STRING();
	}

	if (breakchar_len == 0) {
		crex_argument_value_error(3, "cannot be empty");
		RETURN_THROWS();
	}

	if (linelength == 0 && docut) {
		crex_argument_value_error(4, "cannot be true when argument #2 ($width) is 0");
		RETURN_THROWS();
	}

	/* Special case for a single-character break as it needs no
	   additional storage space */
	if (breakchar_len == 1 && !docut) {
		newtext = crex_string_init(ZSTR_VAL(text), ZSTR_LEN(text), 0);

		laststart = lastspace = 0;
		for (current = 0; current < (crex_long)ZSTR_LEN(text); current++) {
			if (ZSTR_VAL(text)[current] == breakchar[0]) {
				laststart = lastspace = current + 1;
			} else if (ZSTR_VAL(text)[current] == ' ') {
				if (current - laststart >= linelength) {
					ZSTR_VAL(newtext)[current] = breakchar[0];
					laststart = current + 1;
				}
				lastspace = current;
			} else if (current - laststart >= linelength && laststart != lastspace) {
				ZSTR_VAL(newtext)[lastspace] = breakchar[0];
				laststart = lastspace + 1;
			}
		}

		RETURN_NEW_STR(newtext);
	} else {
		/* Multiple character line break or forced cut */
		if (linelength > 0) {
			chk = (size_t)(ZSTR_LEN(text)/linelength + 1);
			newtext = crex_string_safe_alloc(chk, breakchar_len, ZSTR_LEN(text), 0);
			alloced = ZSTR_LEN(text) + chk * breakchar_len + 1;
		} else {
			chk = ZSTR_LEN(text);
			alloced = ZSTR_LEN(text) * (breakchar_len + 1) + 1;
			newtext = crex_string_safe_alloc(ZSTR_LEN(text), breakchar_len + 1, 0, 0);
		}

		/* now keep track of the actual new text length */
		newtextlen = 0;

		laststart = lastspace = 0;
		for (current = 0; current < (crex_long)ZSTR_LEN(text); current++) {
			if (chk == 0) {
				alloced += (size_t) (((ZSTR_LEN(text) - current + 1)/linelength + 1) * breakchar_len) + 1;
				newtext = crex_string_extend(newtext, alloced, 0);
				chk = (size_t) ((ZSTR_LEN(text) - current)/linelength) + 1;
			}
			/* when we hit an existing break, copy to new buffer, and
			 * fix up laststart and lastspace */
			if (ZSTR_VAL(text)[current] == breakchar[0]
				&& current + breakchar_len < ZSTR_LEN(text)
				&& !strncmp(ZSTR_VAL(text) + current, breakchar, breakchar_len)) {
				memcpy(ZSTR_VAL(newtext) + newtextlen, ZSTR_VAL(text) + laststart, current - laststart + breakchar_len);
				newtextlen += current - laststart + breakchar_len;
				current += breakchar_len - 1;
				laststart = lastspace = current + 1;
				chk--;
			}
			/* if it is a space, check if it is at the line boundary,
			 * copy and insert a break, or just keep track of it */
			else if (ZSTR_VAL(text)[current] == ' ') {
				if (current - laststart >= linelength) {
					memcpy(ZSTR_VAL(newtext) + newtextlen, ZSTR_VAL(text) + laststart, current - laststart);
					newtextlen += current - laststart;
					memcpy(ZSTR_VAL(newtext) + newtextlen, breakchar, breakchar_len);
					newtextlen += breakchar_len;
					laststart = current + 1;
					chk--;
				}
				lastspace = current;
			}
			/* if we are cutting, and we've accumulated enough
			 * characters, and we haven't see a space for this line,
			 * copy and insert a break. */
			else if (current - laststart >= linelength
					&& docut && laststart >= lastspace) {
				memcpy(ZSTR_VAL(newtext) + newtextlen, ZSTR_VAL(text) + laststart, current - laststart);
				newtextlen += current - laststart;
				memcpy(ZSTR_VAL(newtext) + newtextlen, breakchar, breakchar_len);
				newtextlen += breakchar_len;
				laststart = lastspace = current;
				chk--;
			}
			/* if the current word puts us over the linelength, copy
			 * back up until the last space, insert a break, and move
			 * up the laststart */
			else if (current - laststart >= linelength
					&& laststart < lastspace) {
				memcpy(ZSTR_VAL(newtext) + newtextlen, ZSTR_VAL(text) + laststart, lastspace - laststart);
				newtextlen += lastspace - laststart;
				memcpy(ZSTR_VAL(newtext) + newtextlen, breakchar, breakchar_len);
				newtextlen += breakchar_len;
				laststart = lastspace = lastspace + 1;
				chk--;
			}
		}

		/* copy over any stragglers */
		if (laststart != current) {
			memcpy(ZSTR_VAL(newtext) + newtextlen, ZSTR_VAL(text) + laststart, current - laststart);
			newtextlen += current - laststart;
		}

		ZSTR_VAL(newtext)[newtextlen] = '\0';
		/* free unused memory */
		newtext = crex_string_truncate(newtext, newtextlen, 0);

		RETURN_NEW_STR(newtext);
	}
}
/* }}} */

/* {{{ crx_explode */
CRXAPI void crx_explode(const crex_string *delim, crex_string *str, zval *return_value, crex_long limit)
{
	const char *p1 = ZSTR_VAL(str);
	const char *endp = ZSTR_VAL(str) + ZSTR_LEN(str);
	const char *p2 = crx_memnstr(ZSTR_VAL(str), ZSTR_VAL(delim), ZSTR_LEN(delim), endp);
	zval  tmp;

	if (p2 == NULL) {
		ZVAL_STR_COPY(&tmp, str);
		crex_hash_next_index_insert_new(C_ARRVAL_P(return_value), &tmp);
	} else {
		crex_hash_real_init_packed(C_ARRVAL_P(return_value));
		CREX_HASH_FILL_PACKED(C_ARRVAL_P(return_value)) {
			do {
				CREX_HASH_FILL_GROW();
				CREX_HASH_FILL_SET_STR(crex_string_init_fast(p1, p2 - p1));
				CREX_HASH_FILL_NEXT();
				p1 = p2 + ZSTR_LEN(delim);
				p2 = crx_memnstr(p1, ZSTR_VAL(delim), ZSTR_LEN(delim), endp);
			} while (p2 != NULL && --limit > 1);

			if (p1 <= endp) {
				CREX_HASH_FILL_GROW();
				CREX_HASH_FILL_SET_STR(crex_string_init_fast(p1, endp - p1));
				CREX_HASH_FILL_NEXT();
			}
		} CREX_HASH_FILL_END();
	}
}
/* }}} */

/* {{{ crx_explode_negative_limit */
CRXAPI void crx_explode_negative_limit(const crex_string *delim, crex_string *str, zval *return_value, crex_long limit)
{
#define EXPLODE_ALLOC_STEP 64
	const char *p1 = ZSTR_VAL(str);
	const char *endp = ZSTR_VAL(str) + ZSTR_LEN(str);
	const char *p2 = crx_memnstr(ZSTR_VAL(str), ZSTR_VAL(delim), ZSTR_LEN(delim), endp);
	zval  tmp;

	if (p2 == NULL) {
		/*
		do nothing since limit <= -1, thus if only one chunk - 1 + (limit) <= 0
		by doing nothing we return empty array
		*/
	} else {
		size_t allocated = EXPLODE_ALLOC_STEP, found = 0;
		crex_long i, to_return;
		const char **positions = emalloc(allocated * sizeof(char *));

		positions[found++] = p1;
		do {
			if (found >= allocated) {
				allocated = found + EXPLODE_ALLOC_STEP;/* make sure we have enough memory */
				positions = erealloc(CREX_VOIDP(positions), allocated*sizeof(char *));
			}
			positions[found++] = p1 = p2 + ZSTR_LEN(delim);
			p2 = crx_memnstr(p1, ZSTR_VAL(delim), ZSTR_LEN(delim), endp);
		} while (p2 != NULL);

		to_return = limit + found;
		/* limit is at least -1 therefore no need of bounds checking : i will be always less than found */
		for (i = 0; i < to_return; i++) { /* this checks also for to_return > 0 */
			ZVAL_STRINGL(&tmp, positions[i], (positions[i+1] - ZSTR_LEN(delim)) - positions[i]);
			crex_hash_next_index_insert_new(C_ARRVAL_P(return_value), &tmp);
		}
		efree((void *)positions);
	}
#undef EXPLODE_ALLOC_STEP
}
/* }}} */

/* {{{ Splits a string on string separator and return array of components. If limit is positive only limit number of components is returned. If limit is negative all components except the last abs(limit) are returned. */
CRX_FUNCTION(explode)
{
	crex_string *str, *delim;
	crex_long limit = CREX_LONG_MAX; /* No limit */
	zval tmp;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(delim)
		C_PARAM_STR(str)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(limit)
	CREX_PARSE_PARAMETERS_END();

	if (ZSTR_LEN(delim) == 0) {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	}

	array_init(return_value);

	if (ZSTR_LEN(str) == 0) {
		if (limit >= 0) {
			ZVAL_EMPTY_STRING(&tmp);
			crex_hash_index_add_new(C_ARRVAL_P(return_value), 0, &tmp);
		}
		return;
	}

	if (limit > 1) {
		crx_explode(delim, str, return_value, limit);
	} else if (limit < 0) {
		crx_explode_negative_limit(delim, str, return_value, limit);
	} else {
		ZVAL_STR_COPY(&tmp, str);
		crex_hash_index_add_new(C_ARRVAL_P(return_value), 0, &tmp);
	}
}
/* }}} */

/* {{{ crx_implode */
CRXAPI void crx_implode(const crex_string *glue, HashTable *pieces, zval *return_value)
{
	zval         *tmp;
	uint32_t      numelems;
	crex_string  *str;
	char         *cptr;
	size_t        len = 0;
	struct {
		crex_string *str;
		crex_long    lval;
	} *strings, *ptr;
	ALLOCA_FLAG(use_heap)

	numelems = crex_hash_num_elements(pieces);

	if (numelems == 0) {
		RETURN_EMPTY_STRING();
	} else if (numelems == 1) {
		/* loop to search the first not undefined element... */
		CREX_HASH_FOREACH_VAL(pieces, tmp) {
			RETURN_STR(zval_get_string(tmp));
		} CREX_HASH_FOREACH_END();
	}

	ptr = strings = do_alloca((sizeof(*strings)) * numelems, use_heap);

	uint32_t flags = ZSTR_GET_COPYABLE_CONCAT_PROPERTIES(glue);

	CREX_HASH_FOREACH_VAL(pieces, tmp) {
		if (EXPECTED(C_TYPE_P(tmp) == IS_STRING)) {
			ptr->str = C_STR_P(tmp);
			len += ZSTR_LEN(ptr->str);
			ptr->lval = 0;
			flags &= ZSTR_GET_COPYABLE_CONCAT_PROPERTIES(ptr->str);
			ptr++;
		} else if (UNEXPECTED(C_TYPE_P(tmp) == IS_LONG)) {
			crex_long val = C_LVAL_P(tmp);

			ptr->str = NULL;
			ptr->lval = val;
			ptr++;
			if (val <= 0) {
				len++;
			}
			while (val) {
				val /= 10;
				len++;
			}
		} else {
			ptr->str = zval_get_string_func(tmp);
			len += ZSTR_LEN(ptr->str);
			ptr->lval = 1;
			flags &= ZSTR_GET_COPYABLE_CONCAT_PROPERTIES(ptr->str);
			ptr++;
		}
	} CREX_HASH_FOREACH_END();

	/* numelems cannot be 0, we checked above */
	str = crex_string_safe_alloc(numelems - 1, ZSTR_LEN(glue), len, 0);
	GC_ADD_FLAGS(str, flags);
	cptr = ZSTR_VAL(str) + ZSTR_LEN(str);
	*cptr = 0;

	while (1) {
		ptr--;
		if (EXPECTED(ptr->str)) {
			cptr -= ZSTR_LEN(ptr->str);
			memcpy(cptr, ZSTR_VAL(ptr->str), ZSTR_LEN(ptr->str));
			if (ptr->lval) {
				crex_string_release_ex(ptr->str, 0);
			}
		} else {
			char *oldPtr = cptr;
			char oldVal = *cptr;
			cptr = crex_print_long_to_buf(cptr, ptr->lval);
			*oldPtr = oldVal;
		}

		if (ptr == strings) {
			break;
		}

		cptr -= ZSTR_LEN(glue);
		memcpy(cptr, ZSTR_VAL(glue), ZSTR_LEN(glue));
	}

	free_alloca(strings, use_heap);
	RETURN_NEW_STR(str);
}
/* }}} */

/* {{{ Joins array elements placing glue string between items and return one string */
CRX_FUNCTION(implode)
{
	crex_string *arg1_str = NULL;
	HashTable *arg1_array = NULL;
	crex_array *pieces = NULL;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_ARRAY_HT_OR_STR(arg1_array, arg1_str)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT_OR_NULL(pieces)
	CREX_PARSE_PARAMETERS_END();

	if (pieces == NULL) {
		if (arg1_array == NULL) {
			crex_type_error("%s(): Argument #1 ($array) must be of type array, string given", get_active_function_name());
			RETURN_THROWS();
		}

		arg1_str = ZSTR_EMPTY_ALLOC();
		pieces = arg1_array;
	} else {
		if (arg1_str == NULL) {
			crex_argument_type_error(1, "must be of type string, array given");
			RETURN_THROWS();
		}
	}

	crx_implode(arg1_str, pieces, return_value);
}
/* }}} */

#define STRTOK_TABLE(p) BG(strtok_table)[(unsigned char) *p]

/* {{{ Tokenize a string */
CRX_FUNCTION(strtok)
{
	crex_string *str, *tok = NULL;
	char *token;
	char *token_end;
	char *p;
	char *pe;
	size_t skipped = 0;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STR(str)
		C_PARAM_OPTIONAL
		C_PARAM_STR_OR_NULL(tok)
	CREX_PARSE_PARAMETERS_END();

	if (!tok) {
		tok = str;
	} else {
		if (BG(strtok_string)) {
			crex_string_release(BG(strtok_string));
		}
		BG(strtok_string) = crex_string_copy(str);
		BG(strtok_last) = ZSTR_VAL(str);
		BG(strtok_len) = ZSTR_LEN(str);
	}

	if (!BG(strtok_string)) {
		/* String to tokenize not set. */
		crx_error_docref(NULL, E_WARNING, "Both arguments must be provided when starting tokenization");
		RETURN_FALSE;
	}

	p = BG(strtok_last); /* Where we start to search */
	pe = ZSTR_VAL(BG(strtok_string)) + BG(strtok_len);
	if (p >= pe) {
		/* Reached the end of the string. */
		RETURN_FALSE;
	}

	token = ZSTR_VAL(tok);
	token_end = token + ZSTR_LEN(tok);

	while (token < token_end) {
		STRTOK_TABLE(token++) = 1;
	}

	/* Skip leading delimiters */
	while (STRTOK_TABLE(p)) {
		if (++p >= pe) {
			/* no other chars left */
			goto return_false;
		}
		skipped++;
	}

	/* We know at this place that *p is no delimiter, so skip it */
	while (++p < pe) {
		if (STRTOK_TABLE(p)) {
			goto return_token;
		}
	}

	if (p - BG(strtok_last)) {
return_token:
		RETVAL_STRINGL(BG(strtok_last) + skipped, (p - BG(strtok_last)) - skipped);
		BG(strtok_last) = p + 1;
	} else {
return_false:
		RETVAL_FALSE;
		crex_string_release(BG(strtok_string));
		BG(strtok_string) = NULL;
	}

	/* Restore table -- usually faster then memset'ing the table on every invocation */
	token = ZSTR_VAL(tok);
	while (token < token_end) {
		STRTOK_TABLE(token++) = 0;
	}
}
/* }}} */

/* {{{ crx_strtoupper */
CRXAPI char *crx_strtoupper(char *s, size_t len)
{
	crex_str_toupper(s, len);
	return s;
}
/* }}} */

/* {{{ crx_string_toupper */
CRXAPI crex_string *crx_string_toupper(crex_string *s)
{
	return crex_string_toupper(s);
}
/* }}} */

/* {{{ Makes a string uppercase */
CRX_FUNCTION(strtoupper)
{
	crex_string *arg;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(arg)
	CREX_PARSE_PARAMETERS_END();

	RETURN_STR(crex_string_toupper(arg));
}
/* }}} */

/* {{{ crx_strtolower */
CRXAPI char *crx_strtolower(char *s, size_t len)
{
	crex_str_tolower(s, len);
	return s;
}
/* }}} */

/* {{{ crx_string_tolower */
CRXAPI crex_string *crx_string_tolower(crex_string *s)
{
	return crex_string_tolower(s);
}
/* }}} */

/* {{{ Makes a string lowercase */
CRX_FUNCTION(strtolower)
{
	crex_string *str;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(str)
	CREX_PARSE_PARAMETERS_END();

	RETURN_STR(crex_string_tolower(str));
}
/* }}} */

CRX_FUNCTION(str_increment)
{
	crex_string *str;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(str)
	CREX_PARSE_PARAMETERS_END();

	if (ZSTR_LEN(str) == 0) {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	}
	if (!crex_string_only_has_ascii_alphanumeric(str)) {
		crex_argument_value_error(1, "must be composed only of alphanumeric ASCII characters");
		RETURN_THROWS();
	}

	crex_string *incremented = crex_string_init(ZSTR_VAL(str), ZSTR_LEN(str), /* persistent */ false);
	size_t position = ZSTR_LEN(str)-1;
	bool carry = false;

	do {
		char c = ZSTR_VAL(incremented)[position];
		/* We know c is in ['a', 'z'], ['A', 'Z'], or ['0', '9'] range from crex_string_only_has_ascii_alphanumeric() */
		if (EXPECTED( c != 'z' && c != 'Z' && c != '9' )) {
			carry = false;
			ZSTR_VAL(incremented)[position]++;
		} else { /* if 'z', 'Z', or '9' */
			carry = true;
			if (c == '9') {
				ZSTR_VAL(incremented)[position] = '0';
			} else {
				ZSTR_VAL(incremented)[position] -= 25;
			}
		}
	} while (carry && position-- > 0);

	if (UNEXPECTED(carry)) {
		crex_string *tmp = crex_string_alloc(ZSTR_LEN(incremented)+1, 0);
		memcpy(ZSTR_VAL(tmp) + 1, ZSTR_VAL(incremented), ZSTR_LEN(incremented));
		ZSTR_VAL(tmp)[ZSTR_LEN(incremented)+1] = '\0';
		switch (ZSTR_VAL(incremented)[0]) {
			case '0':
				ZSTR_VAL(tmp)[0] = '1';
				break;
			default:
				ZSTR_VAL(tmp)[0] = ZSTR_VAL(incremented)[0];
				break;
		}
		crex_string_release_ex(incremented, /* persistent */ false);
		RETURN_STR(tmp);
	}
	RETURN_STR(incremented);
}


CRX_FUNCTION(str_decrement)
{
	crex_string *str;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(str)
	CREX_PARSE_PARAMETERS_END();

	if (ZSTR_LEN(str) == 0) {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	}
	if (!crex_string_only_has_ascii_alphanumeric(str)) {
		crex_argument_value_error(1, "must be composed only of alphanumeric ASCII characters");
		RETURN_THROWS();
	}
	if (ZSTR_LEN(str) >= 1 && ZSTR_VAL(str)[0] == '0') {
		crex_argument_value_error(1, "\"%s\" is out of decrement range", ZSTR_VAL(str));
		RETURN_THROWS();
	}

	crex_string *decremented = crex_string_init(ZSTR_VAL(str), ZSTR_LEN(str), /* persistent */ false);
	size_t position = ZSTR_LEN(str)-1;
	bool carry = false;

	do {
		char c = ZSTR_VAL(decremented)[position];
		/* We know c is in ['a', 'z'], ['A', 'Z'], or ['0', '9'] range from crex_string_only_has_ascii_alphanumeric() */
		if (EXPECTED( c != 'a' && c != 'A' && c != '0' )) {
			carry = false;
			ZSTR_VAL(decremented)[position]--;
		} else { /* if 'a', 'A', or '0' */
			carry = true;
			if (c == '0') {
				ZSTR_VAL(decremented)[position] = '9';
			} else {
				ZSTR_VAL(decremented)[position] += 25;
			}
		}
	} while (carry && position-- > 0);

	if (UNEXPECTED(carry || (ZSTR_VAL(decremented)[0] == '0' && ZSTR_LEN(decremented) > 1))) {
		if (ZSTR_LEN(decremented) == 1) {
			crex_string_release_ex(decremented, /* persistent */ false);
			crex_argument_value_error(1, "\"%s\" is out of decrement range", ZSTR_VAL(str));
			RETURN_THROWS();
		}
		crex_string *tmp = crex_string_alloc(ZSTR_LEN(decremented) - 1, 0);
		memcpy(ZSTR_VAL(tmp), ZSTR_VAL(decremented) + 1, ZSTR_LEN(decremented) - 1);
		ZSTR_VAL(tmp)[ZSTR_LEN(decremented) - 1] = '\0';
		crex_string_release_ex(decremented, /* persistent */ false);
		RETURN_STR(tmp);
	}
	RETURN_STR(decremented);
}

#if defined(CRX_WIN32)
static bool _is_basename_start(const char *start, const char *pos)
{
	if (pos - start >= 1
	    && *(pos-1) != '/'
	    && *(pos-1) != '\\') {
		if (pos - start == 1) {
			return 1;
		} else if (*(pos-2) == '/' || *(pos-2) == '\\') {
			return 1;
		} else if (*(pos-2) == ':'
			&& _is_basename_start(start, pos - 2)) {
			return 1;
		}
	}
	return 0;
}
#endif

/* {{{ crx_basename */
CRXAPI crex_string *crx_basename(const char *s, size_t len, const char *suffix, size_t suffix_len)
{
	const char *basename_start;
	const char *basename_end;

	if (CG(ascii_compatible_locale)) {
		basename_end = s + len - 1;

		/* Strip trailing slashes */
		while (basename_end >= s
#ifdef CRX_WIN32
			&& (*basename_end == '/'
				|| *basename_end == '\\'
				|| (*basename_end == ':'
					&& _is_basename_start(s, basename_end)))) {
#else
			&& *basename_end == '/') {
#endif
			basename_end--;
		}
		if (basename_end < s) {
			return ZSTR_EMPTY_ALLOC();
		}

		/* Extract filename */
		basename_start = basename_end;
		basename_end++;
		while (basename_start > s
#ifdef CRX_WIN32
			&& *(basename_start-1) != '/'
			&& *(basename_start-1) != '\\') {

			if (*(basename_start-1) == ':' &&
				_is_basename_start(s, basename_start - 1)) {
				break;
			}
#else
			&& *(basename_start-1) != '/') {
#endif
			basename_start--;
		}
	} else {
		/* State 0 is directly after a directory separator (or at the start of the string).
		 * State 1 is everything else. */
		int state = 0;

		basename_start = s;
		basename_end = s;
		while (len > 0) {
			int inc_len = (*s == '\0' ? 1 : crx_mblen(s, len));

			switch (inc_len) {
				case 0:
					goto quit_loop;
				case 1:
#ifdef CRX_WIN32
					if (*s == '/' || *s == '\\') {
#else
					if (*s == '/') {
#endif
						if (state == 1) {
							state = 0;
							basename_end = s;
						}
#ifdef CRX_WIN32
					/* Catch relative paths in c:file.txt style. They're not to confuse
					   with the NTFS streams. This part ensures also, that no drive
					   letter traversing happens. */
					} else if ((*s == ':' && (s - basename_start == 1))) {
						if (state == 0) {
							basename_start = s;
							state = 1;
						} else {
							basename_end = s;
							state = 0;
						}
#endif
					} else {
						if (state == 0) {
							basename_start = s;
							state = 1;
						}
					}
					break;
				default:
					if (inc_len < 0) {
						/* If character is invalid, treat it like other non-significant characters. */
						inc_len = 1;
						crx_mb_reset();
					}
					if (state == 0) {
						basename_start = s;
						state = 1;
					}
					break;
			}
			s += inc_len;
			len -= inc_len;
		}

quit_loop:
		if (state == 1) {
			basename_end = s;
		}
	}

	if (suffix != NULL && suffix_len < (size_t)(basename_end - basename_start) &&
			memcmp(basename_end - suffix_len, suffix, suffix_len) == 0) {
		basename_end -= suffix_len;
	}

	return crex_string_init(basename_start, basename_end - basename_start, 0);
}
/* }}} */

/* {{{ Returns the filename component of the path */
CRX_FUNCTION(basename)
{
	char *string, *suffix = NULL;
	size_t   string_len, suffix_len = 0;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STRING(string, string_len)
		C_PARAM_OPTIONAL
		C_PARAM_STRING(suffix, suffix_len)
	CREX_PARSE_PARAMETERS_END();

	RETURN_STR(crx_basename(string, string_len, suffix, suffix_len));
}
/* }}} */

/* {{{ crx_dirname
   Returns directory name component of path */
CRXAPI size_t crx_dirname(char *path, size_t len)
{
	return crex_dirname(path, len);
}
/* }}} */

/* {{{ Returns the directory name component of the path */
CRX_FUNCTION(dirname)
{
	char *str;
	size_t str_len;
	crex_string *ret;
	crex_long levels = 1;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STRING(str, str_len)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(levels)
	CREX_PARSE_PARAMETERS_END();

	ret = crex_string_init(str, str_len, 0);

	if (levels == 1) {
		/* Default case */
#ifdef CRX_WIN32
		ZSTR_LEN(ret) = crx_win32_ioutil_dirname(ZSTR_VAL(ret), str_len);
#else
		ZSTR_LEN(ret) = crex_dirname(ZSTR_VAL(ret), str_len);
#endif
	} else if (levels < 1) {
		crex_argument_value_error(2, "must be greater than or equal to 1");
		crex_string_efree(ret);
		RETURN_THROWS();
	} else {
		/* Some levels up */
		do {
#ifdef CRX_WIN32
			ZSTR_LEN(ret) = crx_win32_ioutil_dirname(ZSTR_VAL(ret), str_len = ZSTR_LEN(ret));
#else
			ZSTR_LEN(ret) = crex_dirname(ZSTR_VAL(ret), str_len = ZSTR_LEN(ret));
#endif
		} while (ZSTR_LEN(ret) < str_len && --levels);
	}

	RETURN_NEW_STR(ret);
}
/* }}} */

/* {{{ Returns information about a certain string */
CRX_FUNCTION(pathinfo)
{
	zval tmp;
	char *path, *dirname;
	size_t path_len;
	bool have_basename;
	crex_long opt = CRX_PATHINFO_ALL;
	crex_string *ret = NULL;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STRING(path, path_len)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(opt)
	CREX_PARSE_PARAMETERS_END();

	have_basename = ((opt & CRX_PATHINFO_BASENAME) == CRX_PATHINFO_BASENAME);

	array_init(&tmp);

	if ((opt & CRX_PATHINFO_DIRNAME) == CRX_PATHINFO_DIRNAME) {
		dirname = estrndup(path, path_len);
		crx_dirname(dirname, path_len);
		if (*dirname) {
			add_assoc_string(&tmp, "dirname", dirname);
		}
		efree(dirname);
	}

	if (have_basename) {
		ret = crx_basename(path, path_len, NULL, 0);
		add_assoc_str(&tmp, "basename", crex_string_copy(ret));
	}

	if ((opt & CRX_PATHINFO_EXTENSION) == CRX_PATHINFO_EXTENSION) {
		const char *p;
		ptrdiff_t idx;

		if (!have_basename) {
			ret = crx_basename(path, path_len, NULL, 0);
		}

		p = crex_memrchr(ZSTR_VAL(ret), '.', ZSTR_LEN(ret));

		if (p) {
			idx = p - ZSTR_VAL(ret);
			add_assoc_stringl(&tmp, "extension", ZSTR_VAL(ret) + idx + 1, ZSTR_LEN(ret) - idx - 1);
		}
	}

	if ((opt & CRX_PATHINFO_FILENAME) == CRX_PATHINFO_FILENAME) {
		const char *p;
		ptrdiff_t idx;

		/* Have we already looked up the basename? */
		if (!have_basename && !ret) {
			ret = crx_basename(path, path_len, NULL, 0);
		}

		p = crex_memrchr(ZSTR_VAL(ret), '.', ZSTR_LEN(ret));

		idx = p ? (p - ZSTR_VAL(ret)) : (ptrdiff_t)ZSTR_LEN(ret);
		add_assoc_stringl(&tmp, "filename", ZSTR_VAL(ret), idx);
	}

	if (ret) {
		crex_string_release_ex(ret, 0);
	}

	if (opt == CRX_PATHINFO_ALL) {
		RETURN_COPY_VALUE(&tmp);
	} else {
		zval *element;
		if ((element = crex_hash_get_current_data(C_ARRVAL(tmp))) != NULL) {
			RETVAL_COPY_DEREF(element);
		} else {
			RETVAL_EMPTY_STRING();
		}
		zval_ptr_dtor(&tmp);
	}
}
/* }}} */

/* {{{ crx_stristr
   case insensitive strstr */
CRXAPI char *crx_stristr(char *s, char *t, size_t s_len, size_t t_len)
{
	return (char*)crx_memnistr(s, t, t_len, s + s_len);
}
/* }}} */

/* {{{ crx_strspn */
CRXAPI size_t crx_strspn(const char *s1, const char *s2, const char *s1_end, const char *s2_end)
{
	const char *p = s1, *spanp;
	char c = *p;

cont:
	for (spanp = s2; p != s1_end && spanp != s2_end;) {
		if (*spanp++ == c) {
			c = *(++p);
			goto cont;
		}
	}
	return (p - s1);
}
/* }}} */

/* {{{ crx_strcspn */
CRXAPI size_t crx_strcspn(const char *s1, const char *s2, const char *s1_end, const char *s2_end)
{
	const char *p, *spanp;
	char c = *s1;

	for (p = s1;;) {
		spanp = s2;
		do {
			if (*spanp == c || p == s1_end) {
				return p - s1;
			}
		} while (spanp++ < (s2_end - 1));
		c = *++p;
	}
	/* NOTREACHED */
}
/* }}} */

/* {{{ Finds first occurrence of a string within another, case insensitive */
CRX_FUNCTION(stristr)
{
	crex_string *haystack, *needle;
	const char *found = NULL;
	size_t  found_offset;
	bool part = 0;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(haystack)
		C_PARAM_STR(needle)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(part)
	CREX_PARSE_PARAMETERS_END();

	found = crx_stristr(ZSTR_VAL(haystack), ZSTR_VAL(needle), ZSTR_LEN(haystack), ZSTR_LEN(needle));

	if (UNEXPECTED(!found)) {
		RETURN_FALSE;
	}
	found_offset = found - ZSTR_VAL(haystack);
	if (part) {
		RETURN_STRINGL(ZSTR_VAL(haystack), found_offset);
	}
	RETURN_STRINGL(found, ZSTR_LEN(haystack) - found_offset);
}
/* }}} */

/* {{{ Finds first occurrence of a string within another */
CRX_FUNCTION(strstr)
{
	crex_string *haystack, *needle;
	const char *found = NULL;
	crex_long found_offset;
	bool part = 0;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(haystack)
		C_PARAM_STR(needle)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(part)
	CREX_PARSE_PARAMETERS_END();

	found = crx_memnstr(ZSTR_VAL(haystack), ZSTR_VAL(needle), ZSTR_LEN(needle), ZSTR_VAL(haystack) + ZSTR_LEN(haystack));

	if (UNEXPECTED(!found)) {
		RETURN_FALSE;
	}
	found_offset = found - ZSTR_VAL(haystack);
	if (part) {
		RETURN_STRINGL(ZSTR_VAL(haystack), found_offset);
	}
	RETURN_STRINGL(found, ZSTR_LEN(haystack) - found_offset);
}
/* }}} */

/* {{{ Checks if a string contains another */
CRX_FUNCTION(str_contains)
{
	crex_string *haystack, *needle;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_STR(haystack)
		C_PARAM_STR(needle)
	CREX_PARSE_PARAMETERS_END();

	RETURN_BOOL(crx_memnstr(ZSTR_VAL(haystack), ZSTR_VAL(needle), ZSTR_LEN(needle), ZSTR_VAL(haystack) + ZSTR_LEN(haystack)));
}
/* }}} */

/* {{{ Checks if haystack starts with needle */
CRX_FUNCTION(str_starts_with)
{
	crex_string *haystack, *needle;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_STR(haystack)
		C_PARAM_STR(needle)
	CREX_PARSE_PARAMETERS_END();

	RETURN_BOOL(crex_string_starts_with(haystack, needle));
}
/* }}} */

/* {{{ Checks if haystack ends with needle */
CRX_FUNCTION(str_ends_with)
{
	crex_string *haystack, *needle;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_STR(haystack)
		C_PARAM_STR(needle)
	CREX_PARSE_PARAMETERS_END();

	if (ZSTR_LEN(needle) > ZSTR_LEN(haystack)) {
		RETURN_FALSE;
	}

	RETURN_BOOL(memcmp(
		ZSTR_VAL(haystack) + ZSTR_LEN(haystack) - ZSTR_LEN(needle),
		ZSTR_VAL(needle), ZSTR_LEN(needle)) == 0);
}
/* }}} */

/* {{{ Finds position of first occurrence of a string within another */
CRX_FUNCTION(strpos)
{
	crex_string *haystack, *needle;
	const char *found = NULL;
	crex_long offset = 0;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(haystack)
		C_PARAM_STR(needle)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(offset)
	CREX_PARSE_PARAMETERS_END();

	if (offset < 0) {
		offset += (crex_long)ZSTR_LEN(haystack);
	}
	if (offset < 0 || (size_t)offset > ZSTR_LEN(haystack)) {
		crex_argument_value_error(3, "must be contained in argument #1 ($haystack)");
		RETURN_THROWS();
	}

	found = (char*)crx_memnstr(ZSTR_VAL(haystack) + offset,
						ZSTR_VAL(needle), ZSTR_LEN(needle),
						ZSTR_VAL(haystack) + ZSTR_LEN(haystack));

	if (UNEXPECTED(!found)) {
		RETURN_FALSE;
	}
	RETURN_LONG(found - ZSTR_VAL(haystack));
}
/* }}} */

/* {{{ Finds position of first occurrence of a string within another, case insensitive */
CRX_FUNCTION(stripos)
{
	const char *found = NULL;
	crex_string *haystack, *needle;
	crex_long offset = 0;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(haystack)
		C_PARAM_STR(needle)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(offset)
	CREX_PARSE_PARAMETERS_END();

	if (offset < 0) {
		offset += (crex_long)ZSTR_LEN(haystack);
	}
	if (offset < 0 || (size_t)offset > ZSTR_LEN(haystack)) {
		crex_argument_value_error(3, "must be contained in argument #1 ($haystack)");
		RETURN_THROWS();
	}

	found = (char*)crx_memnistr(ZSTR_VAL(haystack) + offset,
			ZSTR_VAL(needle), ZSTR_LEN(needle), ZSTR_VAL(haystack) + ZSTR_LEN(haystack));

	if (UNEXPECTED(!found)) {
		RETURN_FALSE;
	}
	RETURN_LONG(found - ZSTR_VAL(haystack));
}
/* }}} */

/* {{{ Finds position of last occurrence of a string within another string */
CRX_FUNCTION(strrpos)
{
	crex_string *needle;
	crex_string *haystack;
	crex_long offset = 0;
	const char *p, *e, *found;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(haystack)
		C_PARAM_STR(needle)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(offset)
	CREX_PARSE_PARAMETERS_END();

	if (offset >= 0) {
		if ((size_t)offset > ZSTR_LEN(haystack)) {
			crex_argument_value_error(3, "must be contained in argument #1 ($haystack)");
			RETURN_THROWS();
		}
		p = ZSTR_VAL(haystack) + (size_t)offset;
		e = ZSTR_VAL(haystack) + ZSTR_LEN(haystack);
	} else {
		if (offset < -CREX_LONG_MAX || (size_t)(-offset) > ZSTR_LEN(haystack)) {
			crex_argument_value_error(3, "must be contained in argument #1 ($haystack)");
			RETURN_THROWS();
		}

		p = ZSTR_VAL(haystack);
		if ((size_t)-offset < ZSTR_LEN(needle)) {
			e = ZSTR_VAL(haystack) + ZSTR_LEN(haystack);
		} else {
			e = ZSTR_VAL(haystack) + ZSTR_LEN(haystack) + offset + ZSTR_LEN(needle);
		}
	}

	found = crex_memnrstr(p, ZSTR_VAL(needle), ZSTR_LEN(needle), e);

	if (UNEXPECTED(!found)) {
		RETURN_FALSE;
	}
	RETURN_LONG(found - ZSTR_VAL(haystack));
}
/* }}} */

/* {{{ Finds position of last occurrence of a string within another string */
CRX_FUNCTION(strripos)
{
	crex_string *needle;
	crex_string *haystack;
	crex_long offset = 0;
	const char *p, *e, *found;
	crex_string *needle_dup, *haystack_dup;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(haystack)
		C_PARAM_STR(needle)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(offset)
	CREX_PARSE_PARAMETERS_END();

	if (ZSTR_LEN(needle) == 1) {
		/* Single character search can shortcut memcmps
		   Can also avoid tolower emallocs */
		char lowered;
		if (offset >= 0) {
			if ((size_t)offset > ZSTR_LEN(haystack)) {
				crex_argument_value_error(3, "must be contained in argument #1 ($haystack)");
				RETURN_THROWS();
			}
			p = ZSTR_VAL(haystack) + (size_t)offset;
			e = ZSTR_VAL(haystack) + ZSTR_LEN(haystack) - 1;
		} else {
			p = ZSTR_VAL(haystack);
			if (offset < -CREX_LONG_MAX || (size_t)(-offset) > ZSTR_LEN(haystack)) {
				crex_argument_value_error(3, "must be contained in argument #1 ($haystack)");
				RETURN_THROWS();
			}
			e = ZSTR_VAL(haystack) + (ZSTR_LEN(haystack) + (size_t)offset);
		}
		lowered = crex_tolower_ascii(*ZSTR_VAL(needle));
		while (e >= p) {
			if (crex_tolower_ascii(*e) == lowered) {
				RETURN_LONG(e - p + (offset > 0 ? offset : 0));
			}
			e--;
		}
		RETURN_FALSE;
	}

	haystack_dup = crex_string_tolower(haystack);
	if (offset >= 0) {
		if ((size_t)offset > ZSTR_LEN(haystack)) {
			crex_string_release_ex(haystack_dup, 0);
			crex_argument_value_error(3, "must be contained in argument #1 ($haystack)");
			RETURN_THROWS();
		}
		p = ZSTR_VAL(haystack_dup) + offset;
		e = ZSTR_VAL(haystack_dup) + ZSTR_LEN(haystack);
	} else {
		if (offset < -CREX_LONG_MAX || (size_t)(-offset) > ZSTR_LEN(haystack)) {
			crex_string_release_ex(haystack_dup, 0);
			crex_argument_value_error(3, "must be contained in argument #1 ($haystack)");
			RETURN_THROWS();
		}

		p = ZSTR_VAL(haystack_dup);
		if ((size_t)-offset < ZSTR_LEN(needle)) {
			e = ZSTR_VAL(haystack_dup) + ZSTR_LEN(haystack);
		} else {
			e = ZSTR_VAL(haystack_dup) + ZSTR_LEN(haystack) + offset + ZSTR_LEN(needle);
		}
	}

	needle_dup = crex_string_tolower(needle);
	if ((found = (char *)crex_memnrstr(p, ZSTR_VAL(needle_dup), ZSTR_LEN(needle_dup), e))) {
		RETVAL_LONG(found - ZSTR_VAL(haystack_dup));
		crex_string_release_ex(needle_dup, 0);
		crex_string_release_ex(haystack_dup, 0);
	} else {
		crex_string_release_ex(needle_dup, 0);
		crex_string_release_ex(haystack_dup, 0);
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Finds the last occurrence of a character in a string within another */
CRX_FUNCTION(strrchr)
{
	crex_string *haystack, *needle;
	const char *found = NULL;
	crex_long found_offset;
	bool part = 0;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(haystack)
		C_PARAM_STR(needle)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(part)
	CREX_PARSE_PARAMETERS_END();

	found = crex_memrchr(ZSTR_VAL(haystack), *ZSTR_VAL(needle), ZSTR_LEN(haystack));
	if (UNEXPECTED(!found)) {
		RETURN_FALSE;
	}
	found_offset = found - ZSTR_VAL(haystack);
	if (part) {
		RETURN_STRINGL(ZSTR_VAL(haystack), found_offset);
	}
	RETURN_STRINGL(found, ZSTR_LEN(haystack) - found_offset);
}
/* }}} */

/* {{{ crx_chunk_split */
static crex_string *crx_chunk_split(const char *src, size_t srclen, const char *end, size_t endlen, size_t chunklen)
{
	char *q;
	const char *p;
	size_t chunks;
	size_t restlen;
	crex_string *dest;

	chunks = srclen / chunklen;
	restlen = srclen - chunks * chunklen; /* srclen % chunklen */
	if (restlen) {
		/* We want chunks to be rounded up rather than rounded down.
		 * Increment can't overflow because chunks <= SIZE_MAX/2 at this point. */
		chunks++;
	}

	dest = crex_string_safe_alloc(chunks, endlen, srclen, 0);

	for (p = src, q = ZSTR_VAL(dest); p < (src + srclen - chunklen + 1); ) {
		memcpy(q, p, chunklen);
		q += chunklen;
		memcpy(q, end, endlen);
		q += endlen;
		p += chunklen;
	}

	if (restlen) {
		memcpy(q, p, restlen);
		q += restlen;
		memcpy(q, end, endlen);
		q += endlen;
	}

	*q = '\0';
	CREX_ASSERT(q - ZSTR_VAL(dest) == ZSTR_LEN(dest));

	return dest;
}
/* }}} */

/* {{{ Returns split line */
CRX_FUNCTION(chunk_split)
{
	crex_string *str;
	char *end    = "\r\n";
	size_t endlen   = 2;
	crex_long chunklen = 76;
	crex_string *result;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_STR(str)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(chunklen)
		C_PARAM_STRING(end, endlen)
	CREX_PARSE_PARAMETERS_END();

	if (chunklen <= 0) {
		crex_argument_value_error(2, "must be greater than 0");
		RETURN_THROWS();
	}

	if ((size_t)chunklen > ZSTR_LEN(str)) {
		/* to maintain BC, we must return original string + ending */
		result = crex_string_safe_alloc(ZSTR_LEN(str), 1, endlen, 0);
		memcpy(ZSTR_VAL(result), ZSTR_VAL(str), ZSTR_LEN(str));
		memcpy(ZSTR_VAL(result) + ZSTR_LEN(str), end, endlen);
		ZSTR_VAL(result)[ZSTR_LEN(result)] = '\0';
		RETURN_NEW_STR(result);
	}

	if (!ZSTR_LEN(str)) {
		RETURN_EMPTY_STRING();
	}

	result = crx_chunk_split(ZSTR_VAL(str), ZSTR_LEN(str), end, endlen, (size_t)chunklen);

	RETURN_STR(result);
}
/* }}} */

/* {{{ Returns part of a string */
CRX_FUNCTION(substr)
{
	crex_string *str;
	crex_long l = 0, f;
	bool len_is_null = 1;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(str)
		C_PARAM_LONG(f)
		C_PARAM_OPTIONAL
		C_PARAM_LONG_OR_NULL(l, len_is_null)
	CREX_PARSE_PARAMETERS_END();

	if (f < 0) {
		/* if "from" position is negative, count start position from the end
		 * of the string
		 */
		if (-(size_t)f > ZSTR_LEN(str)) {
			f = 0;
		} else {
			f = (crex_long)ZSTR_LEN(str) + f;
		}
	} else if ((size_t)f > ZSTR_LEN(str)) {
		RETURN_EMPTY_STRING();
	}

	if (!len_is_null) {
		if (l < 0) {
			/* if "length" position is negative, set it to the length
			 * needed to stop that many chars from the end of the string
			 */
			if (-(size_t)l > ZSTR_LEN(str) - (size_t)f) {
				l = 0;
			} else {
				l = (crex_long)ZSTR_LEN(str) - f + l;
			}
		} else if ((size_t)l > ZSTR_LEN(str) - (size_t)f) {
			l = (crex_long)ZSTR_LEN(str) - f;
		}
	} else {
		l = (crex_long)ZSTR_LEN(str) - f;
	}

	if (l == ZSTR_LEN(str)) {
		RETURN_STR_COPY(str);
	} else {
		RETURN_STRINGL_FAST(ZSTR_VAL(str) + f, l);
	}
}
/* }}} */

/* {{{ Replaces part of a string with another string */
CRX_FUNCTION(substr_replace)
{
	crex_string *str, *repl_str;
	HashTable *str_ht, *repl_ht;
	HashTable *from_ht;
	crex_long from_long;
	HashTable *len_ht = NULL;
	crex_long len_long;
	bool len_is_null = 1;
	crex_long l = 0;
	crex_long f;
	crex_string *result;
	HashPosition from_idx, repl_idx, len_idx;
	zval *tmp_str = NULL, *tmp_repl, *tmp_from = NULL, *tmp_len= NULL;

	CREX_PARSE_PARAMETERS_START(3, 4)
		C_PARAM_ARRAY_HT_OR_STR(str_ht, str)
		C_PARAM_ARRAY_HT_OR_STR(repl_ht, repl_str)
		C_PARAM_ARRAY_HT_OR_LONG(from_ht, from_long)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT_OR_LONG_OR_NULL(len_ht, len_long, len_is_null)
	CREX_PARSE_PARAMETERS_END();

	if (len_is_null) {
		if (str) {
			l = ZSTR_LEN(str);
		}
	} else if (!len_ht) {
		l = len_long;
	}

	if (str) {
		if (from_ht) {
			crex_argument_type_error(3, "cannot be an array when working on a single string");
			RETURN_THROWS();
		}
		if (len_ht) {
			crex_argument_type_error(4, "cannot be an array when working on a single string");
			RETURN_THROWS();
		}

		f = from_long;

		/* if "from" position is negative, count start position from the end
		 * of the string
		 */
		if (f < 0) {
			f = (crex_long)ZSTR_LEN(str) + f;
			if (f < 0) {
				f = 0;
			}
		} else if ((size_t)f > ZSTR_LEN(str)) {
			f = ZSTR_LEN(str);
		}
		/* if "length" position is negative, set it to the length
		 * needed to stop that many chars from the end of the string
		 */
		if (l < 0) {
			l = ((crex_long)ZSTR_LEN(str) - f) + l;
			if (l < 0) {
				l = 0;
			}
		}

		if ((size_t)l > ZSTR_LEN(str) || (l < 0 && (size_t)(-l) > ZSTR_LEN(str))) {
			l = ZSTR_LEN(str);
		}

		if ((f + l) > (crex_long)ZSTR_LEN(str)) {
			l = ZSTR_LEN(str) - f;
		}

		crex_string *tmp_repl_str = NULL;
		if (repl_ht) {
			repl_idx = 0;
			if (HT_IS_PACKED(repl_ht)) {
				while (repl_idx < repl_ht->nNumUsed) {
					tmp_repl = &repl_ht->arPacked[repl_idx];
					if (C_TYPE_P(tmp_repl) != IS_UNDEF) {
						break;
					}
					repl_idx++;
				}
			} else {
				while (repl_idx < repl_ht->nNumUsed) {
					tmp_repl = &repl_ht->arData[repl_idx].val;
					if (C_TYPE_P(tmp_repl) != IS_UNDEF) {
						break;
					}
					repl_idx++;
				}
			}
			if (repl_idx < repl_ht->nNumUsed) {
				repl_str = zval_get_tmp_string(tmp_repl, &tmp_repl_str);
			} else {
				repl_str = STR_EMPTY_ALLOC();
			}
		}

		result = crex_string_safe_alloc(1, ZSTR_LEN(str) - l + ZSTR_LEN(repl_str), 0, 0);

		memcpy(ZSTR_VAL(result), ZSTR_VAL(str), f);
		if (ZSTR_LEN(repl_str)) {
			memcpy((ZSTR_VAL(result) + f), ZSTR_VAL(repl_str), ZSTR_LEN(repl_str));
		}
		memcpy((ZSTR_VAL(result) + f + ZSTR_LEN(repl_str)), ZSTR_VAL(str) + f + l, ZSTR_LEN(str) - f - l);
		ZSTR_VAL(result)[ZSTR_LEN(result)] = '\0';
		crex_tmp_string_release(tmp_repl_str);
		RETURN_NEW_STR(result);
	} else { /* str is array of strings */
		crex_string *str_index = NULL;
		size_t result_len;
		crex_ulong num_index;

		/* TODO
		if (!len_is_null && from_ht) {
			if (crex_hash_num_elements(from_ht) != crex_hash_num_elements(len_ht)) {
				crx_error_docref(NULL, E_WARNING, "'start' and 'length' should have the same number of elements");
				RETURN_STR_COPY(str);
			}
		}
		*/

		array_init(return_value);

		from_idx = len_idx = repl_idx = 0;

		CREX_HASH_FOREACH_KEY_VAL(str_ht, num_index, str_index, tmp_str) {
			crex_string *tmp_orig_str;
			crex_string *orig_str = zval_get_tmp_string(tmp_str, &tmp_orig_str);

			if (from_ht) {
				if (HT_IS_PACKED(from_ht)) {
					while (from_idx < from_ht->nNumUsed) {
						tmp_from = &from_ht->arPacked[from_idx];
						if (C_TYPE_P(tmp_from) != IS_UNDEF) {
							break;
						}
						from_idx++;
					}
				} else {
					while (from_idx < from_ht->nNumUsed) {
						tmp_from = &from_ht->arData[from_idx].val;
						if (C_TYPE_P(tmp_from) != IS_UNDEF) {
							break;
						}
						from_idx++;
					}
				}
				if (from_idx < from_ht->nNumUsed) {
					f = zval_get_long(tmp_from);

					if (f < 0) {
						f = (crex_long)ZSTR_LEN(orig_str) + f;
						if (f < 0) {
							f = 0;
						}
					} else if (f > (crex_long)ZSTR_LEN(orig_str)) {
						f = ZSTR_LEN(orig_str);
					}
					from_idx++;
				} else {
					f = 0;
				}
			} else {
				f = from_long;
				if (f < 0) {
					f = (crex_long)ZSTR_LEN(orig_str) + f;
					if (f < 0) {
						f = 0;
					}
				} else if (f > (crex_long)ZSTR_LEN(orig_str)) {
					f = ZSTR_LEN(orig_str);
				}
			}

			if (len_ht) {
				if (HT_IS_PACKED(len_ht)) {
					while (len_idx < len_ht->nNumUsed) {
						tmp_len = &len_ht->arPacked[len_idx];
						if (C_TYPE_P(tmp_len) != IS_UNDEF) {
							break;
						}
						len_idx++;
					}
				} else {
					while (len_idx < len_ht->nNumUsed) {
						tmp_len = &len_ht->arData[len_idx].val;
						if (C_TYPE_P(tmp_len) != IS_UNDEF) {
							break;
						}
						len_idx++;
					}
				}
				if (len_idx < len_ht->nNumUsed) {
					l = zval_get_long(tmp_len);
					len_idx++;
				} else {
					l = ZSTR_LEN(orig_str);
				}
			} else if (!len_is_null) {
				l = len_long;
			} else {
				l = ZSTR_LEN(orig_str);
			}

			if (l < 0) {
				l = (ZSTR_LEN(orig_str) - f) + l;
				if (l < 0) {
					l = 0;
				}
			}

			CREX_ASSERT(0 <= f && f <= CREX_LONG_MAX);
			CREX_ASSERT(0 <= l && l <= CREX_LONG_MAX);
			if (((size_t) f + l) > ZSTR_LEN(orig_str)) {
				l = ZSTR_LEN(orig_str) - f;
			}

			result_len = ZSTR_LEN(orig_str) - l;

			if (repl_ht) {
				if (HT_IS_PACKED(repl_ht)) {
					while (repl_idx < repl_ht->nNumUsed) {
						tmp_repl = &repl_ht->arPacked[repl_idx];
						if (C_TYPE_P(tmp_repl) != IS_UNDEF) {
							break;
						}
						repl_idx++;
					}
				} else {
					while (repl_idx < repl_ht->nNumUsed) {
						tmp_repl = &repl_ht->arData[repl_idx].val;
						if (C_TYPE_P(tmp_repl) != IS_UNDEF) {
							break;
						}
						repl_idx++;
					}
				}
				if (repl_idx < repl_ht->nNumUsed) {
					crex_string *tmp_repl_str;
					crex_string *repl_str = zval_get_tmp_string(tmp_repl, &tmp_repl_str);

					result_len += ZSTR_LEN(repl_str);
					repl_idx++;
					result = crex_string_safe_alloc(1, result_len, 0, 0);

					memcpy(ZSTR_VAL(result), ZSTR_VAL(orig_str), f);
					memcpy((ZSTR_VAL(result) + f), ZSTR_VAL(repl_str), ZSTR_LEN(repl_str));
					memcpy((ZSTR_VAL(result) + f + ZSTR_LEN(repl_str)), ZSTR_VAL(orig_str) + f + l, ZSTR_LEN(orig_str) - f - l);
					crex_tmp_string_release(tmp_repl_str);
				} else {
					result = crex_string_safe_alloc(1, result_len, 0, 0);

					memcpy(ZSTR_VAL(result), ZSTR_VAL(orig_str), f);
					memcpy((ZSTR_VAL(result) + f), ZSTR_VAL(orig_str) + f + l, ZSTR_LEN(orig_str) - f - l);
				}
			} else {
				result_len += ZSTR_LEN(repl_str);

				result = crex_string_safe_alloc(1, result_len, 0, 0);

				memcpy(ZSTR_VAL(result), ZSTR_VAL(orig_str), f);
				memcpy((ZSTR_VAL(result) + f), ZSTR_VAL(repl_str), ZSTR_LEN(repl_str));
				memcpy((ZSTR_VAL(result) + f + ZSTR_LEN(repl_str)), ZSTR_VAL(orig_str) + f + l, ZSTR_LEN(orig_str) - f - l);
			}

			ZSTR_VAL(result)[ZSTR_LEN(result)] = '\0';

			if (str_index) {
				zval tmp;

				ZVAL_NEW_STR(&tmp, result);
				crex_symtable_update(C_ARRVAL_P(return_value), str_index, &tmp);
			} else {
				add_index_str(return_value, num_index, result);
			}

			crex_tmp_string_release(tmp_orig_str);
		} CREX_HASH_FOREACH_END();
	} /* if */
}
/* }}} */

/* {{{ Quotes meta characters */
CRX_FUNCTION(quotemeta)
{
	crex_string *old;
	const char *old_end, *p;
	char *q;
	char c;
	crex_string *str;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(old)
	CREX_PARSE_PARAMETERS_END();

	old_end = ZSTR_VAL(old) + ZSTR_LEN(old);

	if (ZSTR_LEN(old) == 0) {
		RETURN_EMPTY_STRING();
	}

	str = crex_string_safe_alloc(2, ZSTR_LEN(old), 0, 0);

	for (p = ZSTR_VAL(old), q = ZSTR_VAL(str); p != old_end; p++) {
		c = *p;
		switch (c) {
			case '.':
			case '\\':
			case '+':
			case '*':
			case '?':
			case '[':
			case '^':
			case ']':
			case '$':
			case '(':
			case ')':
				*q++ = '\\';
				CREX_FALLTHROUGH;
			default:
				*q++ = c;
		}
	}

	*q = '\0';

	RETURN_NEW_STR(crex_string_truncate(str, q - ZSTR_VAL(str), 0));
}
/* }}} */

/* {{{ Returns ASCII value of character
   Warning: This function is special-cased by crex_compile.c and so is bypassed for constant string argument */
CRX_FUNCTION(ord)
{
	crex_string *str;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(str)
	CREX_PARSE_PARAMETERS_END();

	RETURN_LONG((unsigned char) ZSTR_VAL(str)[0]);
}
/* }}} */

/* {{{ Converts ASCII code to a character
   Warning: This function is special-cased by crex_compile.c and so is bypassed for constant integer argument */
CRX_FUNCTION(chr)
{
	crex_long c;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(c)
	CREX_PARSE_PARAMETERS_END();

	c &= 0xff;
	RETURN_CHAR(c);
}
/* }}} */

/* {{{ crx_ucfirst
   Uppercase the first character of the word in a native string */
static crex_string* crx_ucfirst(crex_string *str)
{
	const unsigned char ch = ZSTR_VAL(str)[0];
	unsigned char r = crex_toupper_ascii(ch);
	if (r == ch) {
		return crex_string_copy(str);
	} else {
		crex_string *s = crex_string_init(ZSTR_VAL(str), ZSTR_LEN(str), 0);
		ZSTR_VAL(s)[0] = r;
		return s;
	}
}
/* }}} */

/* {{{ Makes a string's first character uppercase */
CRX_FUNCTION(ucfirst)
{
	crex_string *str;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(str)
	CREX_PARSE_PARAMETERS_END();

	if (!ZSTR_LEN(str)) {
		RETURN_EMPTY_STRING();
	}

	RETURN_STR(crx_ucfirst(str));
}
/* }}} */

/* {{{
   Lowercase the first character of the word in a native string */
static crex_string* crx_lcfirst(crex_string *str)
{
	unsigned char r = crex_tolower_ascii(ZSTR_VAL(str)[0]);
	if (r == ZSTR_VAL(str)[0]) {
		return crex_string_copy(str);
	} else {
		crex_string *s = crex_string_init(ZSTR_VAL(str), ZSTR_LEN(str), 0);
		ZSTR_VAL(s)[0] = r;
		return s;
	}
}
/* }}} */

/* {{{ Make a string's first character lowercase */
CRX_FUNCTION(lcfirst)
{
	crex_string  *str;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(str)
	CREX_PARSE_PARAMETERS_END();

	if (!ZSTR_LEN(str)) {
		RETURN_EMPTY_STRING();
	}

	RETURN_STR(crx_lcfirst(str));
}
/* }}} */

/* {{{ Uppercase the first character of every word in a string */
CRX_FUNCTION(ucwords)
{
	crex_string *str;
	char *delims = " \t\r\n\f\v";
	char *r;
	const char *r_end;
	size_t delims_len = 6;
	char mask[256];

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STR(str)
		C_PARAM_OPTIONAL
		C_PARAM_STRING(delims, delims_len)
	CREX_PARSE_PARAMETERS_END();

	if (!ZSTR_LEN(str)) {
		RETURN_EMPTY_STRING();
	}

	crx_charmask((const unsigned char *) delims, delims_len, mask);

	ZVAL_STRINGL(return_value, ZSTR_VAL(str), ZSTR_LEN(str));
	r = C_STRVAL_P(return_value);

	*r = crex_toupper_ascii((unsigned char) *r);
	for (r_end = r + C_STRLEN_P(return_value) - 1; r < r_end; ) {
		if (mask[(unsigned char)*r++]) {
			*r = crex_toupper_ascii((unsigned char) *r);
		}
	}
}
/* }}} */

/* {{{ crx_strtr */
CRXAPI char *crx_strtr(char *str, size_t len, const char *str_from, const char *str_to, size_t trlen)
{
	size_t i;

	if (UNEXPECTED(trlen < 1)) {
		return str;
	} else if (trlen == 1) {
		char ch_from = *str_from;
		char ch_to = *str_to;

		for (i = 0; i < len; i++) {
			if (str[i] == ch_from) {
				str[i] = ch_to;
			}
		}
	} else {
		unsigned char xlat[256];

		memset(xlat, 0, sizeof(xlat));

		for (i = 0; i < trlen; i++) {
			xlat[(size_t)(unsigned char) str_from[i]] = str_to[i] - str_from[i];
		}

		for (i = 0; i < len; i++) {
			str[i] += xlat[(size_t)(unsigned char) str[i]];
		}
	}

	return str;
}
/* }}} */

/* {{{ crx_strtr_ex */
static crex_string *crx_strtr_ex(crex_string *str, const char *str_from, const char *str_to, size_t trlen)
{
	crex_string *new_str = NULL;
	size_t i;

	if (UNEXPECTED(trlen < 1)) {
		return crex_string_copy(str);
	} else if (trlen == 1) {
		char ch_from = *str_from;
		char ch_to = *str_to;
		char *output;
		char *input = ZSTR_VAL(str);
		size_t len = ZSTR_LEN(str);

#ifdef __SSE2__
		if (ZSTR_LEN(str) >= sizeof(__m128i)) {
			__m128i search = _mm_set1_epi8(ch_from);
			__m128i delta = _mm_set1_epi8(ch_to - ch_from);

			do {
				__m128i src = _mm_loadu_si128((__m128i*)(input));
				__m128i mask = _mm_cmpeq_epi8(src, search);
				if (_mm_movemask_epi8(mask)) {
					new_str = crex_string_alloc(ZSTR_LEN(str), 0);
					memcpy(ZSTR_VAL(new_str), ZSTR_VAL(str), input - ZSTR_VAL(str));
					output = ZSTR_VAL(new_str) + (input - ZSTR_VAL(str));
					_mm_storeu_si128((__m128i *)(output),
						_mm_add_epi8(src,
							_mm_and_si128(mask, delta)));
					input += sizeof(__m128i);
					output += sizeof(__m128i);
					len -= sizeof(__m128i);
					for (; len >= sizeof(__m128i); input += sizeof(__m128i), output += sizeof(__m128i), len -= sizeof(__m128i)) {
						src = _mm_loadu_si128((__m128i*)(input));
						mask = _mm_cmpeq_epi8(src, search);
						_mm_storeu_si128((__m128i *)(output),
							_mm_add_epi8(src,
								_mm_and_si128(mask, delta)));
					}
					for (; len > 0; input++, output++, len--) {
						*output = (*input == ch_from) ? ch_to : *input;
					}
					*output = 0;
					return new_str;
				}
				input += sizeof(__m128i);
				len -= sizeof(__m128i);
			} while (len >= sizeof(__m128i));
		}
#endif
		for (; len > 0; input++, len--) {
			if (*input == ch_from) {
				new_str = crex_string_alloc(ZSTR_LEN(str), 0);
				memcpy(ZSTR_VAL(new_str), ZSTR_VAL(str), input - ZSTR_VAL(str));
				output = ZSTR_VAL(new_str) + (input - ZSTR_VAL(str));
				*output = ch_to;
				input++;
				output++;
				len--;
				for (; len > 0; input++, output++, len--) {
					*output = (*input == ch_from) ? ch_to : *input;
				}
				*output = 0;
				return new_str;
			}
		}
	} else {
		unsigned char xlat[256];

		memset(xlat, 0, sizeof(xlat));;

		for (i = 0; i < trlen; i++) {
			xlat[(size_t)(unsigned char) str_from[i]] = str_to[i] - str_from[i];
		}

		for (i = 0; i < ZSTR_LEN(str); i++) {
			if (xlat[(size_t)(unsigned char) ZSTR_VAL(str)[i]]) {
				new_str = crex_string_alloc(ZSTR_LEN(str), 0);
				memcpy(ZSTR_VAL(new_str), ZSTR_VAL(str), i);
				do {
					ZSTR_VAL(new_str)[i] = ZSTR_VAL(str)[i] + xlat[(size_t)(unsigned char) ZSTR_VAL(str)[i]];
					i++;
				} while (i < ZSTR_LEN(str));
				ZSTR_VAL(new_str)[i] = 0;
				return new_str;
			}
		}
	}

	return crex_string_copy(str);
}
/* }}} */

/* {{{ crx_strtr_array */
static void crx_strtr_array(zval *return_value, crex_string *input, HashTable *pats)
{
	const char *str = ZSTR_VAL(input);
	size_t slen = ZSTR_LEN(input);
	crex_ulong num_key;
	crex_string *str_key;
	size_t len, pos, old_pos;
	bool has_num_keys = false;
	size_t minlen = 128*1024;
	size_t maxlen = 0;
	HashTable str_hash;
	zval *entry;
	const char *key;
	smart_str result = {0};
	crex_ulong bitset[256/sizeof(crex_ulong)];
	crex_ulong *num_bitset;

	/* we will collect all possible key lengths */
	num_bitset = ecalloc((slen + sizeof(crex_ulong)) / sizeof(crex_ulong), sizeof(crex_ulong));
	memset(bitset, 0, sizeof(bitset));

	/* check if original array has numeric keys */
	CREX_HASH_FOREACH_STR_KEY(pats, str_key) {
		if (UNEXPECTED(!str_key)) {
			has_num_keys = true;
		} else {
			len = ZSTR_LEN(str_key);
			if (UNEXPECTED(len == 0)) {
				crx_error_docref(NULL, E_WARNING, "Ignoring replacement of empty string");
				continue;
			} else if (UNEXPECTED(len > slen)) {
				/* skip long patterns */
				continue;
			}
			if (len > maxlen) {
				maxlen = len;
			}
			if (len < minlen) {
				minlen = len;
			}
			/* remember possible key length */
			num_bitset[len / sizeof(crex_ulong)] |= C_UL(1) << (len % sizeof(crex_ulong));
			bitset[((unsigned char)ZSTR_VAL(str_key)[0]) / sizeof(crex_ulong)] |= C_UL(1) << (((unsigned char)ZSTR_VAL(str_key)[0]) % sizeof(crex_ulong));
		}
	} CREX_HASH_FOREACH_END();

	if (UNEXPECTED(has_num_keys)) {
		crex_string *key_used;
		/* we have to rebuild HashTable with numeric keys */
		crex_hash_init(&str_hash, crex_hash_num_elements(pats), NULL, NULL, 0);
		CREX_HASH_FOREACH_KEY_VAL(pats, num_key, str_key, entry) {
			if (UNEXPECTED(!str_key)) {
				key_used = crex_long_to_str(num_key);
				len = ZSTR_LEN(key_used);
				if (UNEXPECTED(len > slen)) {
					/* skip long patterns */
					crex_string_release(key_used);
					continue;
				}
				if (len > maxlen) {
					maxlen = len;
				}
				if (len < minlen) {
					minlen = len;
				}
				/* remember possible key length */
				num_bitset[len / sizeof(crex_ulong)] |= C_UL(1) << (len % sizeof(crex_ulong));
				bitset[((unsigned char)ZSTR_VAL(key_used)[0]) / sizeof(crex_ulong)] |= C_UL(1) << (((unsigned char)ZSTR_VAL(key_used)[0]) % sizeof(crex_ulong));
			} else {
				key_used = str_key;
				len = ZSTR_LEN(key_used);
				if (UNEXPECTED(len > slen)) {
					/* skip long patterns */
					continue;
				}
			}
			crex_hash_add(&str_hash, key_used, entry);
			if (UNEXPECTED(!str_key)) {
				crex_string_release_ex(key_used, 0);
			}
		} CREX_HASH_FOREACH_END();
		pats = &str_hash;
	}

	if (UNEXPECTED(minlen > maxlen)) {
		/* return the original string */
		if (pats == &str_hash) {
			crex_hash_destroy(&str_hash);
		}
		efree(num_bitset);
		RETURN_STR_COPY(input);
	}

	old_pos = pos = 0;
	while (pos <= slen - minlen) {
		key = str + pos;
		if (bitset[((unsigned char)key[0]) / sizeof(crex_ulong)] & (C_UL(1) << (((unsigned char)key[0]) % sizeof(crex_ulong)))) {
			len = maxlen;
			if (len > slen - pos) {
				len = slen - pos;
			}
			while (len >= minlen) {
				if ((num_bitset[len / sizeof(crex_ulong)] & (C_UL(1) << (len % sizeof(crex_ulong))))) {
					entry = crex_hash_str_find(pats, key, len);
					if (entry != NULL) {
						crex_string *tmp;
						crex_string *s = zval_get_tmp_string(entry, &tmp);
						smart_str_appendl(&result, str + old_pos, pos - old_pos);
						smart_str_append(&result, s);
						old_pos = pos + len;
						pos = old_pos - 1;
						crex_tmp_string_release(tmp);
						break;
					}
				}
				len--;
			}
		}
		pos++;
	}

	if (result.s) {
		smart_str_appendl(&result, str + old_pos, slen - old_pos);
		RETVAL_STR(smart_str_extract(&result));
	} else {
		smart_str_free(&result);
		RETVAL_STR_COPY(input);
	}

	if (pats == &str_hash) {
		crex_hash_destroy(&str_hash);
	}
	efree(num_bitset);
}
/* }}} */

/* {{{ count_chars */
static crex_always_inline crex_long count_chars(const char *p, crex_long length, char ch)
{
	crex_long count = 0;
	const char *endp;

#ifdef __SSE2__
	if (length >= sizeof(__m128i)) {
		__m128i search = _mm_set1_epi8(ch);

		do {
			__m128i src = _mm_loadu_si128((__m128i*)(p));
			uint32_t mask = _mm_movemask_epi8(_mm_cmpeq_epi8(src, search));
			// TODO: It would be great to use POPCNT, but it's available only with SSE4.1
#if 1
			while (mask != 0) {
				count++;
				mask = mask & (mask - 1);
			}
#else
			if (mask) {
				mask = mask - ((mask >> 1) & 0x5555);
				mask = (mask & 0x3333) + ((mask >> 2) & 0x3333);
				mask = (mask + (mask >> 4)) & 0x0F0F;
				mask = (mask + (mask >> 8)) & 0x00ff;
				count += mask;
			}
#endif
			p += sizeof(__m128i);
			length -= sizeof(__m128i);
		} while (length >= sizeof(__m128i));
	}
	endp = p + length;
	while (p != endp) {
		count += (*p == ch);
		p++;
	}
#else
	endp = p + length;
	while ((p = memchr(p, ch, endp-p))) {
		count++;
		p++;
	}
#endif
	return count;
}
/* }}} */

/* {{{ crx_char_to_str_ex */
static crex_string* crx_char_to_str_ex(crex_string *str, char from, char *to, size_t to_len, bool case_sensitivity, crex_long *replace_count)
{
	crex_string *result;
	size_t char_count;
	int lc_from = 0;
	const char *source, *source_end;
	char *target;

	if (case_sensitivity) {
		char_count = count_chars(ZSTR_VAL(str), ZSTR_LEN(str), from);
	} else {
		char_count = 0;
		lc_from = crex_tolower_ascii(from);
		source_end = ZSTR_VAL(str) + ZSTR_LEN(str);
		for (source = ZSTR_VAL(str); source < source_end; source++) {
			if (crex_tolower_ascii(*source) == lc_from) {
				char_count++;
			}
		}
	}

	if (char_count == 0) {
		return crex_string_copy(str);
	}

	if (replace_count) {
		*replace_count += char_count;
	}

	if (to_len > 0) {
		result = crex_string_safe_alloc(char_count, to_len - 1, ZSTR_LEN(str), 0);
	} else {
		result = crex_string_alloc(ZSTR_LEN(str) - char_count, 0);
	}
	target = ZSTR_VAL(result);

	if (case_sensitivity) {
		char *p = ZSTR_VAL(str), *e = p + ZSTR_LEN(str), *s = ZSTR_VAL(str);

		while ((p = memchr(p, from, (e - p)))) {
			memcpy(target, s, (p - s));
			target += p - s;
			memcpy(target, to, to_len);
			target += to_len;
			p++;
			s = p;
			if (--char_count == 0) break;
		}
		if (s < e) {
			memcpy(target, s, (e - s));
			target += e - s;
		}
	} else {
		source_end = ZSTR_VAL(str) + ZSTR_LEN(str);
		for (source = ZSTR_VAL(str); source < source_end; source++) {
			if (crex_tolower_ascii(*source) == lc_from) {
				memcpy(target, to, to_len);
				target += to_len;
			} else {
				*target = *source;
				target++;
			}
		}
	}
	*target = 0;
	return result;
}
/* }}} */

/* {{{ crx_str_to_str_ex */
static crex_string *crx_str_to_str_ex(crex_string *haystack,
	const char *needle, size_t needle_len, const char *str, size_t str_len, crex_long *replace_count)
{

	if (needle_len < ZSTR_LEN(haystack)) {
		crex_string *new_str;
		const char *end;
		const char *p, *r;
		char *e;

		if (needle_len == str_len) {
			new_str = NULL;
			end = ZSTR_VAL(haystack) + ZSTR_LEN(haystack);
			for (p = ZSTR_VAL(haystack); (r = (char*)crx_memnstr(p, needle, needle_len, end)); p = r + needle_len) {
				if (!new_str) {
					new_str = crex_string_init(ZSTR_VAL(haystack), ZSTR_LEN(haystack), 0);
				}
				memcpy(ZSTR_VAL(new_str) + (r - ZSTR_VAL(haystack)), str, str_len);
				(*replace_count)++;
			}
			if (!new_str) {
				goto nothing_todo;
			}
			return new_str;
		} else {
			size_t count = 0;
			const char *o = ZSTR_VAL(haystack);
			const char *n = needle;
			const char *endp = o + ZSTR_LEN(haystack);

			while ((o = (char*)crx_memnstr(o, n, needle_len, endp))) {
				o += needle_len;
				count++;
			}
			if (count == 0) {
				/* Needle doesn't occur, shortcircuit the actual replacement. */
				goto nothing_todo;
			}
			if (str_len > needle_len) {
				new_str = crex_string_safe_alloc(count, str_len - needle_len, ZSTR_LEN(haystack), 0);
			} else {
				new_str = crex_string_alloc(count * (str_len - needle_len) + ZSTR_LEN(haystack), 0);
			}

			e = ZSTR_VAL(new_str);
			end = ZSTR_VAL(haystack) + ZSTR_LEN(haystack);
			for (p = ZSTR_VAL(haystack); (r = (char*)crx_memnstr(p, needle, needle_len, end)); p = r + needle_len) {
				memcpy(e, p, r - p);
				e += r - p;
				memcpy(e, str, str_len);
				e += str_len;
				(*replace_count)++;
			}

			if (p < end) {
				memcpy(e, p, end - p);
				e += end - p;
			}

			*e = '\0';
			return new_str;
		}
	} else if (needle_len > ZSTR_LEN(haystack) || memcmp(ZSTR_VAL(haystack), needle, ZSTR_LEN(haystack))) {
nothing_todo:
		return crex_string_copy(haystack);
	} else {
		(*replace_count)++;
		return crex_string_init_fast(str, str_len);
	}
}
/* }}} */

/* {{{ crx_str_to_str_i_ex */
static crex_string *crx_str_to_str_i_ex(crex_string *haystack, const char *lc_haystack,
	crex_string *needle, const char *str, size_t str_len, crex_long *replace_count)
{
	crex_string *new_str = NULL;
	crex_string *lc_needle;

	if (ZSTR_LEN(needle) < ZSTR_LEN(haystack)) {
		const char *end;
		const char *p, *r;
		char *e;

		if (ZSTR_LEN(needle) == str_len) {
			lc_needle = crex_string_tolower(needle);
			end = lc_haystack + ZSTR_LEN(haystack);
			for (p = lc_haystack; (r = (char*)crx_memnstr(p, ZSTR_VAL(lc_needle), ZSTR_LEN(lc_needle), end)); p = r + ZSTR_LEN(lc_needle)) {
				if (!new_str) {
					new_str = crex_string_init(ZSTR_VAL(haystack), ZSTR_LEN(haystack), 0);
				}
				memcpy(ZSTR_VAL(new_str) + (r - lc_haystack), str, str_len);
				(*replace_count)++;
			}
			crex_string_release_ex(lc_needle, 0);

			if (!new_str) {
				goto nothing_todo;
			}
			return new_str;
		} else {
			size_t count = 0;
			const char *o = lc_haystack;
			const char *n;
			const char *endp = o + ZSTR_LEN(haystack);

			lc_needle = crex_string_tolower(needle);
			n = ZSTR_VAL(lc_needle);

			while ((o = (char*)crx_memnstr(o, n, ZSTR_LEN(lc_needle), endp))) {
				o += ZSTR_LEN(lc_needle);
				count++;
			}
			if (count == 0) {
				/* Needle doesn't occur, shortcircuit the actual replacement. */
				crex_string_release_ex(lc_needle, 0);
				goto nothing_todo;
			}

			if (str_len > ZSTR_LEN(lc_needle)) {
				new_str = crex_string_safe_alloc(count, str_len - ZSTR_LEN(lc_needle), ZSTR_LEN(haystack), 0);
			} else {
				new_str = crex_string_alloc(count * (str_len - ZSTR_LEN(lc_needle)) + ZSTR_LEN(haystack), 0);
			}

			e = ZSTR_VAL(new_str);
			end = lc_haystack + ZSTR_LEN(haystack);

			for (p = lc_haystack; (r = (char*)crx_memnstr(p, ZSTR_VAL(lc_needle), ZSTR_LEN(lc_needle), end)); p = r + ZSTR_LEN(lc_needle)) {
				memcpy(e, ZSTR_VAL(haystack) + (p - lc_haystack), r - p);
				e += r - p;
				memcpy(e, str, str_len);
				e += str_len;
				(*replace_count)++;
			}

			if (p < end) {
				memcpy(e, ZSTR_VAL(haystack) + (p - lc_haystack), end - p);
				e += end - p;
			}
			*e = '\0';

			crex_string_release_ex(lc_needle, 0);

			return new_str;
		}
	} else if (ZSTR_LEN(needle) > ZSTR_LEN(haystack)) {
nothing_todo:
		return crex_string_copy(haystack);
	} else {
		lc_needle = crex_string_tolower(needle);

		if (memcmp(lc_haystack, ZSTR_VAL(lc_needle), ZSTR_LEN(lc_needle))) {
			crex_string_release_ex(lc_needle, 0);
			goto nothing_todo;
		}
		crex_string_release_ex(lc_needle, 0);

		new_str = crex_string_init(str, str_len, 0);

		(*replace_count)++;
		return new_str;
	}
}
/* }}} */

/* {{{ crx_str_to_str */
CRXAPI crex_string *crx_str_to_str(const char *haystack, size_t length, const char *needle, size_t needle_len, const char *str, size_t str_len)
{
	crex_string *new_str;

	if (needle_len < length) {
		const char *end;
		const char *s, *p;
		char *e, *r;

		if (needle_len == str_len) {
			new_str = crex_string_init(haystack, length, 0);
			end = ZSTR_VAL(new_str) + length;
			for (p = ZSTR_VAL(new_str); (r = (char*)crx_memnstr(p, needle, needle_len, end)); p = r + needle_len) {
				memcpy(r, str, str_len);
			}
			return new_str;
		} else {
			if (str_len < needle_len) {
				new_str = crex_string_alloc(length, 0);
			} else {
				size_t count = 0;
				const char *o = haystack;
				const char *n = needle;
				const char *endp = o + length;

				while ((o = (char*)crx_memnstr(o, n, needle_len, endp))) {
					o += needle_len;
					count++;
				}
				if (count == 0) {
					/* Needle doesn't occur, shortcircuit the actual replacement. */
					new_str = crex_string_init(haystack, length, 0);
					return new_str;
				} else {
					if (str_len > needle_len) {
						new_str = crex_string_safe_alloc(count, str_len - needle_len, length, 0);
					} else {
						new_str = crex_string_alloc(count * (str_len - needle_len) + length, 0);
					}
				}
			}

			s = e = ZSTR_VAL(new_str);
			end = haystack + length;
			for (p = haystack; (r = (char*)crx_memnstr(p, needle, needle_len, end)); p = r + needle_len) {
				memcpy(e, p, r - p);
				e += r - p;
				memcpy(e, str, str_len);
				e += str_len;
			}

			if (p < end) {
				memcpy(e, p, end - p);
				e += end - p;
			}

			*e = '\0';
			new_str = crex_string_truncate(new_str, e - s, 0);
			return new_str;
		}
	} else if (needle_len > length || memcmp(haystack, needle, length)) {
		new_str = crex_string_init(haystack, length, 0);
		return new_str;
	} else {
		new_str = crex_string_init(str, str_len, 0);

		return new_str;
	}
}
/* }}} */

/* {{{ Translates characters in str using given translation tables */
CRX_FUNCTION(strtr)
{
	crex_string *str, *from_str = NULL;
	HashTable *from_ht = NULL;
	char *to = NULL;
	size_t to_len = 0;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(str)
		C_PARAM_ARRAY_HT_OR_STR(from_ht, from_str)
		C_PARAM_OPTIONAL
		C_PARAM_STRING_OR_NULL(to, to_len)
	CREX_PARSE_PARAMETERS_END();

	if (!to && from_ht == NULL) {
		crex_argument_type_error(2, "must be of type array, string given");
		RETURN_THROWS();
	} else if (to && from_str == NULL) {
		crex_argument_type_error(2, "must be of type string, array given");
		RETURN_THROWS();
	}

	/* shortcut for empty string */
	if (ZSTR_LEN(str) == 0) {
		RETURN_EMPTY_STRING();
	}

	if (!to) {
		if (crex_hash_num_elements(from_ht) < 1) {
			RETURN_STR_COPY(str);
		} else if (crex_hash_num_elements(from_ht) == 1) {
			crex_long num_key;
			crex_string *str_key, *tmp_str, *replace, *tmp_replace;
			zval *entry;

			CREX_HASH_FOREACH_KEY_VAL(from_ht, num_key, str_key, entry) {
				tmp_str = NULL;
				if (UNEXPECTED(!str_key)) {
					str_key = tmp_str = crex_long_to_str(num_key);
				}
				replace = zval_get_tmp_string(entry, &tmp_replace);
				if (ZSTR_LEN(str_key) < 1) {
					crx_error_docref(NULL, E_WARNING, "Ignoring replacement of empty string");
					RETVAL_STR_COPY(str);
				} else if (ZSTR_LEN(str_key) == 1) {
					RETVAL_STR(crx_char_to_str_ex(str,
								ZSTR_VAL(str_key)[0],
								ZSTR_VAL(replace),
								ZSTR_LEN(replace),
								/* case_sensitive */ true,
								NULL));
				} else {
					crex_long dummy;
					RETVAL_STR(crx_str_to_str_ex(str,
								ZSTR_VAL(str_key), ZSTR_LEN(str_key),
								ZSTR_VAL(replace), ZSTR_LEN(replace), &dummy));
				}
				crex_tmp_string_release(tmp_str);
				crex_tmp_string_release(tmp_replace);
				return;
			} CREX_HASH_FOREACH_END();
		} else {
			crx_strtr_array(return_value, str, from_ht);
		}
	} else {
		RETURN_STR(crx_strtr_ex(str,
				  ZSTR_VAL(from_str),
				  to,
				  MIN(ZSTR_LEN(from_str), to_len)));
	}
}
/* }}} */

/* {{{ Reverse a string */
#ifdef CREX_INTRIN_SSSE3_NATIVE
#include <tmmintrin.h>
#elif defined(__aarch64__) || defined(_M_ARM64)
#include <arm_neon.h>
#endif
CRX_FUNCTION(strrev)
{
	crex_string *str;
	const char *s, *e;
	char *p;
	crex_string *n;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(str)
	CREX_PARSE_PARAMETERS_END();

	n = crex_string_alloc(ZSTR_LEN(str), 0);
	p = ZSTR_VAL(n);

	s = ZSTR_VAL(str);
	e = s + ZSTR_LEN(str);
	--e;
#ifdef CREX_INTRIN_SSSE3_NATIVE
	if (e - s > 15) {
		const __m128i map = _mm_set_epi8(
				0, 1, 2, 3,
				4, 5, 6, 7,
				8, 9, 10, 11,
				12, 13, 14, 15);
		do {
			const __m128i str = _mm_loadu_si128((__m128i *)(e - 15));
			_mm_storeu_si128((__m128i *)p, _mm_shuffle_epi8(str, map));
			p += 16;
			e -= 16;
		} while (e - s > 15);
	}
#elif defined(__aarch64__)
	if (e - s > 15) {
		do {
			const uint8x16_t str = vld1q_u8((uint8_t *)(e - 15));
			/* Synthesize rev128 with a rev64 + ext. */
			const uint8x16_t rev = vrev64q_u8(str);
			const uint8x16_t ext = (uint8x16_t)
				vextq_u64((uint64x2_t)rev, (uint64x2_t)rev, 1);
			vst1q_u8((uint8_t *)p, ext);
			p += 16;
			e -= 16;
		} while (e - s > 15);
	}
#elif defined(_M_ARM64)
	if (e - s > 15) {
		do {
			const __n128 str = vld1q_u8((uint8_t *)(e - 15));
			/* Synthesize rev128 with a rev64 + ext. */
			/* strange force cast limit on windows: you cannot convert anything */
			const __n128 rev = vrev64q_u8(str);
			const __n128 ext = vextq_u64(rev, rev, 1);
			vst1q_u8((uint8_t *)p, ext);
			p += 16;
			e -= 16;
		} while (e - s > 15);
	}
#endif
	while (e >= s) {
		*p++ = *e--;
	}

	*p = '\0';

	RETVAL_NEW_STR(n);
}
/* }}} */

/* {{{ crx_similar_str */
static void crx_similar_str(const char *txt1, size_t len1, const char *txt2, size_t len2, size_t *pos1, size_t *pos2, size_t *max, size_t *count)
{
	const char *p, *q;
	const char *end1 = (char *) txt1 + len1;
	const char *end2 = (char *) txt2 + len2;
	size_t l;

	*max = 0;
	*count = 0;
	for (p = (char *) txt1; p < end1; p++) {
		for (q = (char *) txt2; q < end2; q++) {
			for (l = 0; (p + l < end1) && (q + l < end2) && (p[l] == q[l]); l++);
			if (l > *max) {
				*max = l;
				*count += 1;
				*pos1 = p - txt1;
				*pos2 = q - txt2;
			}
		}
	}
}
/* }}} */

/* {{{ crx_similar_char */
static size_t crx_similar_char(const char *txt1, size_t len1, const char *txt2, size_t len2)
{
	size_t sum;
	size_t pos1 = 0, pos2 = 0, max, count;

	crx_similar_str(txt1, len1, txt2, len2, &pos1, &pos2, &max, &count);
	if ((sum = max)) {
		if (pos1 && pos2 && count > 1) {
			sum += crx_similar_char(txt1, pos1,
									txt2, pos2);
		}
		if ((pos1 + max < len1) && (pos2 + max < len2)) {
			sum += crx_similar_char(txt1 + pos1 + max, len1 - pos1 - max,
									txt2 + pos2 + max, len2 - pos2 - max);
		}
	}

	return sum;
}
/* }}} */

/* {{{ Calculates the similarity between two strings */
CRX_FUNCTION(similar_text)
{
	crex_string *t1, *t2;
	zval *percent = NULL;
	bool compute_percentage = CREX_NUM_ARGS() >= 3;
	size_t sim;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(t1)
		C_PARAM_STR(t2)
		C_PARAM_OPTIONAL
		C_PARAM_ZVAL(percent)
	CREX_PARSE_PARAMETERS_END();

	if (ZSTR_LEN(t1) + ZSTR_LEN(t2) == 0) {
		if (compute_percentage) {
			CREX_TRY_ASSIGN_REF_DOUBLE(percent, 0);
		}

		RETURN_LONG(0);
	}

	sim = crx_similar_char(ZSTR_VAL(t1), ZSTR_LEN(t1), ZSTR_VAL(t2), ZSTR_LEN(t2));

	if (compute_percentage) {
		CREX_TRY_ASSIGN_REF_DOUBLE(percent, sim * 200.0 / (ZSTR_LEN(t1) + ZSTR_LEN(t2)));
	}

	RETURN_LONG(sim);
}
/* }}} */

/* {{{ Escapes all chars mentioned in charlist with backslash. It creates octal representations if asked to backslash characters with 8th bit set or with ASCII<32 (except '\n', '\r', '\t' etc...) */
CRX_FUNCTION(addcslashes)
{
	crex_string *str, *what;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_STR(str)
		C_PARAM_STR(what)
	CREX_PARSE_PARAMETERS_END();

	if (ZSTR_LEN(str) == 0) {
		RETURN_EMPTY_STRING();
	}

	if (ZSTR_LEN(what) == 0) {
		RETURN_STR_COPY(str);
	}

	RETURN_STR(crx_addcslashes_str(ZSTR_VAL(str), ZSTR_LEN(str), ZSTR_VAL(what), ZSTR_LEN(what)));
}
/* }}} */

/* {{{ Escapes single quote, double quotes and backslash characters in a string with backslashes */
CRX_FUNCTION(addslashes)
{
	crex_string *str;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(str)
	CREX_PARSE_PARAMETERS_END();

	if (ZSTR_LEN(str) == 0) {
		RETURN_EMPTY_STRING();
	}

	RETURN_STR(crx_addslashes(str));
}
/* }}} */

/* {{{ Strips backslashes from a string. Uses C-style conventions */
CRX_FUNCTION(stripcslashes)
{
	crex_string *str;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(str)
	CREX_PARSE_PARAMETERS_END();

	ZVAL_STRINGL(return_value, ZSTR_VAL(str), ZSTR_LEN(str));
	crx_stripcslashes(C_STR_P(return_value));
}
/* }}} */

/* {{{ Strips backslashes from a string */
CRX_FUNCTION(stripslashes)
{
	crex_string *str;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(str)
	CREX_PARSE_PARAMETERS_END();

	ZVAL_STRINGL(return_value, ZSTR_VAL(str), ZSTR_LEN(str));
	crx_stripslashes(C_STR_P(return_value));
}
/* }}} */

/* {{{ crx_stripcslashes */
CRXAPI void crx_stripcslashes(crex_string *str)
{
	const char *source, *end;
	char *target;
	size_t  nlen = ZSTR_LEN(str), i;
	char numtmp[4];

	for (source = (char*)ZSTR_VAL(str), end = source + ZSTR_LEN(str), target = ZSTR_VAL(str); source < end; source++) {
		if (*source == '\\' && source + 1 < end) {
			source++;
			switch (*source) {
				case 'n':  *target++='\n'; nlen--; break;
				case 'r':  *target++='\r'; nlen--; break;
				case 'a':  *target++='\a'; nlen--; break;
				case 't':  *target++='\t'; nlen--; break;
				case 'v':  *target++='\v'; nlen--; break;
				case 'b':  *target++='\b'; nlen--; break;
				case 'f':  *target++='\f'; nlen--; break;
				case '\\': *target++='\\'; nlen--; break;
				case 'x':
					if (source+1 < end && isxdigit((int)(*(source+1)))) {
						numtmp[0] = *++source;
						if (source+1 < end && isxdigit((int)(*(source+1)))) {
							numtmp[1] = *++source;
							numtmp[2] = '\0';
							nlen-=3;
						} else {
							numtmp[1] = '\0';
							nlen-=2;
						}
						*target++=(char)strtol(numtmp, NULL, 16);
						break;
					}
					CREX_FALLTHROUGH;
				default:
					i=0;
					while (source < end && *source >= '0' && *source <= '7' && i<3) {
						numtmp[i++] = *source++;
					}
					if (i) {
						numtmp[i]='\0';
						*target++=(char)strtol(numtmp, NULL, 8);
						nlen-=i;
						source--;
					} else {
						*target++=*source;
						nlen--;
					}
			}
		} else {
			*target++=*source;
		}
	}

	if (nlen != 0) {
		*target='\0';
	}

	ZSTR_LEN(str) = nlen;
}
/* }}} */

/* {{{ crx_addcslashes_str */
CRXAPI crex_string *crx_addcslashes_str(const char *str, size_t len, const char *what, size_t wlength)
{
	char flags[256];
	char *target;
	const char *source, *end;
	char c;
	size_t  newlen;
	crex_string *new_str = crex_string_safe_alloc(4, len, 0, 0);

	crx_charmask((const unsigned char *) what, wlength, flags);

	for (source = str, end = source + len, target = ZSTR_VAL(new_str); source < end; source++) {
		c = *source;
		if (flags[(unsigned char)c]) {
			if ((unsigned char) c < 32 || (unsigned char) c > 126) {
				*target++ = '\\';
				switch (c) {
					case '\n': *target++ = 'n'; break;
					case '\t': *target++ = 't'; break;
					case '\r': *target++ = 'r'; break;
					case '\a': *target++ = 'a'; break;
					case '\v': *target++ = 'v'; break;
					case '\b': *target++ = 'b'; break;
					case '\f': *target++ = 'f'; break;
					default: target += sprintf(target, "%03o", (unsigned char) c);
				}
				continue;
			}
			*target++ = '\\';
		}
		*target++ = c;
	}
	*target = 0;
	newlen = target - ZSTR_VAL(new_str);
	if (newlen < len * 4) {
		new_str = crex_string_truncate(new_str, newlen, 0);
	}
	return new_str;
}
/* }}} */

/* {{{ crx_addcslashes */
CRXAPI crex_string *crx_addcslashes(crex_string *str, const char *what, size_t wlength)
{
	return crx_addcslashes_str(ZSTR_VAL(str), ZSTR_LEN(str), what, wlength);
}
/* }}} */

/* {{{ crx_addslashes */

#ifdef CREX_INTRIN_SSE4_2_NATIVE
# include <nmmintrin.h>
# include "Crex/crex_bitset.h"
#elif defined(CREX_INTRIN_SSE4_2_RESOLVER)
# include <nmmintrin.h>
# include "Crex/crex_bitset.h"
# include "Crex/crex_cpuinfo.h"

CREX_INTRIN_SSE4_2_FUNC_DECL(crex_string *crx_addslashes_sse42(crex_string *str));
crex_string *crx_addslashes_default(crex_string *str);

# ifdef CREX_INTRIN_SSE4_2_FUNC_PROTO
CRXAPI crex_string *crx_addslashes(crex_string *str) __attribute__((ifunc("resolve_addslashes")));

typedef crex_string *(*crx_addslashes_func_t)(crex_string *);

CREX_NO_SANITIZE_ADDRESS
CREX_ATTRIBUTE_UNUSED /* clang mistakenly warns about this */
static crx_addslashes_func_t resolve_addslashes(void) {
	if (crex_cpu_supports_sse42()) {
		return crx_addslashes_sse42;
	}
	return crx_addslashes_default;
}
# else /* CREX_INTRIN_SSE4_2_FUNC_PTR */

static crex_string *(*crx_addslashes_ptr)(crex_string *str) = NULL;

CRXAPI crex_string *crx_addslashes(crex_string *str) {
	return crx_addslashes_ptr(str);
}

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(string_intrin)
{
	if (crex_cpu_supports_sse42()) {
		crx_addslashes_ptr = crx_addslashes_sse42;
	} else {
		crx_addslashes_ptr = crx_addslashes_default;
	}
	return SUCCESS;
}
/* }}} */
# endif
#endif

#if defined(CREX_INTRIN_SSE4_2_NATIVE) || defined(CREX_INTRIN_SSE4_2_RESOLVER)
# ifdef CREX_INTRIN_SSE4_2_NATIVE
CRXAPI crex_string *crx_addslashes(crex_string *str) /* {{{ */
# elif defined(CREX_INTRIN_SSE4_2_RESOLVER)
crex_string *crx_addslashes_sse42(crex_string *str)
# endif
{
	CREX_SET_ALIGNED(16, static const char slashchars[16]) = "\'\"\\\0";
	__m128i w128, s128;
	uint32_t res = 0;
	/* maximum string length, worst case situation */
	char *target;
	const char *source, *end;
	size_t offset;
	crex_string *new_str;

	if (!str) {
		return ZSTR_EMPTY_ALLOC();
	}

	source = ZSTR_VAL(str);
	end = source + ZSTR_LEN(str);

	if (ZSTR_LEN(str) > 15) {
		w128 = _mm_load_si128((__m128i *)slashchars);
		do {
			s128 = _mm_loadu_si128((__m128i *)source);
			res = _mm_cvtsi128_si32(_mm_cmpestrm(w128, 4, s128, 16, _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_BIT_MASK));
			if (res) {
				goto do_escape;
			}
			source += 16;
		} while ((end - source) > 15);
	}

	while (source < end) {
		switch (*source) {
			case '\0':
			case '\'':
			case '\"':
			case '\\':
				goto do_escape;
			default:
				source++;
				break;
		}
	}

	return crex_string_copy(str);

do_escape:
	offset = source - (char *)ZSTR_VAL(str);
	new_str = crex_string_safe_alloc(2, ZSTR_LEN(str) - offset, offset, 0);
	memcpy(ZSTR_VAL(new_str), ZSTR_VAL(str), offset);
	target = ZSTR_VAL(new_str) + offset;

	if (res) {
		int pos = 0;
		do {
			int i, n = crex_ulong_ntz(res);
			for (i = 0; i < n; i++) {
				*target++ = source[pos + i];
			}
			pos += n;
			*target++ = '\\';
			if (source[pos] == '\0') {
				*target++ = '0';
			} else {
				*target++ = source[pos];
			}
			pos++;
			res = res >> (n + 1);
		} while (res);

		for (; pos < 16; pos++) {
			*target++ = source[pos];
		}
		source += 16;
	} else if (end - source > 15) {
		w128 = _mm_load_si128((__m128i *)slashchars);
	}

	for (; end - source > 15; source += 16) {
		int pos = 0;
		s128 = _mm_loadu_si128((__m128i *)source);
		res = _mm_cvtsi128_si32(_mm_cmpestrm(w128, 4, s128, 16, _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_BIT_MASK));
		if (res) {
			do {
				int i, n = crex_ulong_ntz(res);
				for (i = 0; i < n; i++) {
					*target++ = source[pos + i];
				}
				pos += n;
				*target++ = '\\';
				if (source[pos] == '\0') {
					*target++ = '0';
				} else {
					*target++ = source[pos];
				}
				pos++;
				res = res >> (n + 1);
			} while (res);
			for (; pos < 16; pos++) {
				*target++ = source[pos];
			}
		} else {
			_mm_storeu_si128((__m128i*)target, s128);
			target += 16;
		}
	}

	while (source < end) {
		switch (*source) {
			case '\0':
				*target++ = '\\';
				*target++ = '0';
				break;
			case '\'':
			case '\"':
			case '\\':
				*target++ = '\\';
				CREX_FALLTHROUGH;
			default:
				*target++ = *source;
				break;
		}
		source++;
	}

	*target = '\0';

	if (ZSTR_LEN(new_str) - (target - ZSTR_VAL(new_str)) > 16) {
		new_str = crex_string_truncate(new_str, target - ZSTR_VAL(new_str), 0);
	} else {
		ZSTR_LEN(new_str) = target - ZSTR_VAL(new_str);
	}

	return new_str;
}
/* }}} */
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
typedef union {
	uint8_t mem[16];
	uint64_t dw[2];
} quad_word;

static crex_always_inline quad_word aarch64_contains_slash_chars(uint8x16_t x) {
	uint8x16_t s0 = vceqq_u8(x, vdupq_n_u8('\0'));
	uint8x16_t s1 = vceqq_u8(x, vdupq_n_u8('\''));
	uint8x16_t s2 = vceqq_u8(x, vdupq_n_u8('\"'));
	uint8x16_t s3 = vceqq_u8(x, vdupq_n_u8('\\'));
	uint8x16_t s01 = vorrq_u8(s0, s1);
	uint8x16_t s23 = vorrq_u8(s2, s3);
	uint8x16_t s0123 = vorrq_u8(s01, s23);
	quad_word qw;
	vst1q_u8(qw.mem, s0123);
	return qw;
}

static crex_always_inline char *aarch64_add_slashes(quad_word res, const char *source, char *target)
{
	for (int i = 0; i < 16; i++) {
		char s = source[i];
		if (res.mem[i] == 0)
			*target++ = s;
		else {
			*target++ = '\\';
			if (s == '\0')
				*target++ = '0';
			else
				*target++ = s;
		}
	}
	return target;
}
#endif /* defined(__aarch64__) || defined(_M_ARM64) */

#ifndef CREX_INTRIN_SSE4_2_NATIVE
# ifdef CREX_INTRIN_SSE4_2_RESOLVER
crex_string *crx_addslashes_default(crex_string *str) /* {{{ */
# else
CRXAPI crex_string *crx_addslashes(crex_string *str)
# endif
{
	/* maximum string length, worst case situation */
	char *target;
	const char *source, *end;
	size_t offset;
	crex_string *new_str;

	if (!str) {
		return ZSTR_EMPTY_ALLOC();
	}

	source = ZSTR_VAL(str);
	end = source + ZSTR_LEN(str);

# if defined(__aarch64__) || defined(_M_ARM64)
	quad_word res = {0};
	if (ZSTR_LEN(str) > 15) {
		do {
			res = aarch64_contains_slash_chars(vld1q_u8((uint8_t *)source));
			if (res.dw[0] | res.dw[1])
				goto do_escape;
			source += 16;
		} while ((end - source) > 15);
	}
	/* Finish the last 15 bytes or less with the scalar loop. */
# endif /* defined(__aarch64__) || defined(_M_ARM64) */

	while (source < end) {
		switch (*source) {
			case '\0':
			case '\'':
			case '\"':
			case '\\':
				goto do_escape;
			default:
				source++;
				break;
		}
	}

	return crex_string_copy(str);

do_escape:
	offset = source - (char *)ZSTR_VAL(str);
	new_str = crex_string_safe_alloc(2, ZSTR_LEN(str) - offset, offset, 0);
	memcpy(ZSTR_VAL(new_str), ZSTR_VAL(str), offset);
	target = ZSTR_VAL(new_str) + offset;

# if defined(__aarch64__) || defined(_M_ARM64)
	if (res.dw[0] | res.dw[1]) {
		target = aarch64_add_slashes(res, source, target);
		source += 16;
	}
	for (; end - source > 15; source += 16) {
		uint8x16_t x = vld1q_u8((uint8_t *)source);
		res = aarch64_contains_slash_chars(x);
		if (res.dw[0] | res.dw[1]) {
			target = aarch64_add_slashes(res, source, target);
		} else {
			vst1q_u8((uint8_t*)target, x);
			target += 16;
		}
	}
	/* Finish the last 15 bytes or less with the scalar loop. */
# endif /* defined(__aarch64__) || defined(_M_ARM64) */

	while (source < end) {
		switch (*source) {
			case '\0':
				*target++ = '\\';
				*target++ = '0';
				break;
			case '\'':
			case '\"':
			case '\\':
				*target++ = '\\';
				CREX_FALLTHROUGH;
			default:
				*target++ = *source;
				break;
		}
		source++;
	}

	*target = '\0';

	if (ZSTR_LEN(new_str) - (target - ZSTR_VAL(new_str)) > 16) {
		new_str = crex_string_truncate(new_str, target - ZSTR_VAL(new_str), 0);
	} else {
		ZSTR_LEN(new_str) = target - ZSTR_VAL(new_str);
	}

	return new_str;
}
#endif
/* }}} */
/* }}} */

/* {{{ crx_stripslashes
 *
 * be careful, this edits the string in-place */
static crex_always_inline char *crx_stripslashes_impl(const char *str, char *out, size_t len)
{
#if defined(__aarch64__) || defined(_M_ARM64)
	while (len > 15) {
		uint8x16_t x = vld1q_u8((uint8_t *)str);
		quad_word q;
		vst1q_u8(q.mem, vceqq_u8(x, vdupq_n_u8('\\')));
		if (q.dw[0] | q.dw[1]) {
			unsigned int i = 0;
			while (i < 16) {
				if (q.mem[i] == 0) {
					*out++ = str[i];
					i++;
					continue;
				}

				i++;			/* skip the slash */
				if (i < len) {
					char s = str[i];
					if (s == '0')
						*out++ = '\0';
					else
						*out++ = s;	/* preserve the next character */
					i++;
				}
			}
			str += i;
			len -= i;
		} else {
			vst1q_u8((uint8_t*)out, x);
			out += 16;
			str += 16;
			len -= 16;
		}
	}
	/* Finish the last 15 bytes or less with the scalar loop. */
#endif /* defined(__aarch64__) || defined(_M_ARM64) */
	while (len > 0) {
		if (*str == '\\') {
			str++;				/* skip the slash */
			len--;
			if (len > 0) {
				if (*str == '0') {
					*out++='\0';
					str++;
				} else {
					*out++ = *str++;	/* preserve the next character */
				}
				len--;
			}
		} else {
			*out++ = *str++;
			len--;
		}
	}

	return out;
}

#ifdef __SSE2__
CRXAPI void crx_stripslashes(crex_string *str)
{
	const char *s = ZSTR_VAL(str);
	char *t = ZSTR_VAL(str);
	size_t l = ZSTR_LEN(str);

	if (l > 15) {
		const __m128i slash = _mm_set1_epi8('\\');

		do {
			__m128i in = _mm_loadu_si128((__m128i *)s);
			__m128i any_slash = _mm_cmpeq_epi8(in, slash);
			uint32_t res = _mm_movemask_epi8(any_slash);

			if (res) {
				int i, n = crex_ulong_ntz(res);
				const char *e = s + 15;
				l -= n;
				for (i = 0; i < n; i++) {
					*t++ = *s++;
				}
				for (; s < e; s++) {
					if (*s == '\\') {
						s++;
						l--;
						if (*s == '0') {
							*t = '\0';
						} else {
							*t = *s;
						}
					} else {
						*t = *s;
					}
					t++;
					l--;
				}
			} else {
				_mm_storeu_si128((__m128i *)t, in);
				s += 16;
				t += 16;
				l -= 16;
			}
		} while (l > 15);
	}

	t = crx_stripslashes_impl(s, t, l);
	if (t != (ZSTR_VAL(str) + ZSTR_LEN(str))) {
		ZSTR_LEN(str) = t - ZSTR_VAL(str);
		ZSTR_VAL(str)[ZSTR_LEN(str)] = '\0';
	}
}
#else
CRXAPI void crx_stripslashes(crex_string *str)
{
	const char *t = crx_stripslashes_impl(ZSTR_VAL(str), ZSTR_VAL(str), ZSTR_LEN(str));
	if (t != (ZSTR_VAL(str) + ZSTR_LEN(str))) {
		ZSTR_LEN(str) = t - ZSTR_VAL(str);
		ZSTR_VAL(str)[ZSTR_LEN(str)] = '\0';
	}
}
#endif
/* }}} */

#define _HEB_BLOCK_TYPE_ENG 1
#define _HEB_BLOCK_TYPE_HEB 2
#define isheb(c)      (((((unsigned char) c) >= 224) && (((unsigned char) c) <= 250)) ? 1 : 0)
#define _isblank(c)   (((((unsigned char) c) == ' '  || ((unsigned char) c) == '\t')) ? 1 : 0)
#define _isnewline(c) (((((unsigned char) c) == '\n' || ((unsigned char) c) == '\r')) ? 1 : 0)

/* {{{ crx_str_replace_in_subject */
static crex_long crx_str_replace_in_subject(
	crex_string *search_str, HashTable *search_ht, crex_string *replace_str, HashTable *replace_ht,
	crex_string *subject_str, zval *result, bool case_sensitivity
) {
	zval		*search_entry;
	crex_string	*tmp_result;
	char		*replace_value = NULL;
	size_t		 replace_len = 0;
	crex_long	 replace_count = 0;
	crex_string *lc_subject_str = NULL;
	uint32_t     replace_idx;

	if (ZSTR_LEN(subject_str) == 0) {
		ZVAL_EMPTY_STRING(result);
		return 0;
	}

	/* If search is an array */
	if (search_ht) {
		/* Duplicate subject string for repeated replacement */
		crex_string_addref(subject_str);

		if (replace_ht) {
			replace_idx = 0;
		} else {
			/* Set replacement value to the passed one */
			replace_value = ZSTR_VAL(replace_str);
			replace_len = ZSTR_LEN(replace_str);
		}

		/* For each entry in the search array, get the entry */
		CREX_HASH_FOREACH_VAL(search_ht, search_entry) {
			/* Make sure we're dealing with strings. */
			crex_string *tmp_search_str;
			crex_string *search_str = zval_get_tmp_string(search_entry, &tmp_search_str);
			crex_string *replace_entry_str, *tmp_replace_entry_str = NULL;

			/* If replace is an array. */
			if (replace_ht) {
				/* Get current entry */
				zval *replace_entry = NULL;
				if (HT_IS_PACKED(replace_ht)) {
					while (replace_idx < replace_ht->nNumUsed) {
						replace_entry = &replace_ht->arPacked[replace_idx];
						if (C_TYPE_P(replace_entry) != IS_UNDEF) {
							break;
						}
						replace_idx++;
					}
				} else {
					while (replace_idx < replace_ht->nNumUsed) {
						replace_entry = &replace_ht->arData[replace_idx].val;
						if (C_TYPE_P(replace_entry) != IS_UNDEF) {
							break;
						}
						replace_idx++;
					}
				}
				if (replace_idx < replace_ht->nNumUsed) {
					/* Make sure we're dealing with strings. */
					replace_entry_str = zval_get_tmp_string(replace_entry, &tmp_replace_entry_str);

					/* Set replacement value to the one we got from array */
					replace_value = ZSTR_VAL(replace_entry_str);
					replace_len = ZSTR_LEN(replace_entry_str);

					replace_idx++;
				} else {
					/* We've run out of replacement strings, so use an empty one. */
					replace_value = "";
					replace_len = 0;
				}
			}

			if (ZSTR_LEN(search_str) == 1) {
				crex_long old_replace_count = replace_count;

				tmp_result = crx_char_to_str_ex(subject_str,
								ZSTR_VAL(search_str)[0],
								replace_value,
								replace_len,
								case_sensitivity,
								&replace_count);
				if (lc_subject_str && replace_count != old_replace_count) {
					crex_string_release_ex(lc_subject_str, 0);
					lc_subject_str = NULL;
				}
			} else if (ZSTR_LEN(search_str) > 1) {
				if (case_sensitivity) {
					tmp_result = crx_str_to_str_ex(subject_str,
							ZSTR_VAL(search_str), ZSTR_LEN(search_str),
							replace_value, replace_len, &replace_count);
				} else {
					crex_long old_replace_count = replace_count;

					if (!lc_subject_str) {
						lc_subject_str = crex_string_tolower(subject_str);
					}
					tmp_result = crx_str_to_str_i_ex(subject_str, ZSTR_VAL(lc_subject_str),
							search_str, replace_value, replace_len, &replace_count);
					if (replace_count != old_replace_count) {
						crex_string_release_ex(lc_subject_str, 0);
						lc_subject_str = NULL;
					}
				}
			} else {
				crex_tmp_string_release(tmp_search_str);
				crex_tmp_string_release(tmp_replace_entry_str);
				continue;
			}

			crex_tmp_string_release(tmp_search_str);
			crex_tmp_string_release(tmp_replace_entry_str);

			if (subject_str == tmp_result) {
				crex_string_delref(subject_str);
			} else {
				crex_string_release_ex(subject_str, 0);
				subject_str = tmp_result;
				if (ZSTR_LEN(subject_str) == 0) {
					crex_string_release_ex(subject_str, 0);
					ZVAL_EMPTY_STRING(result);
					if (lc_subject_str) {
						crex_string_release_ex(lc_subject_str, 0);
					}
					return replace_count;
				}
			}
		} CREX_HASH_FOREACH_END();
		ZVAL_STR(result, subject_str);
		if (lc_subject_str) {
			crex_string_release_ex(lc_subject_str, 0);
		}
	} else {
		CREX_ASSERT(search_str);
		if (ZSTR_LEN(search_str) == 1) {
			ZVAL_STR(result,
				crx_char_to_str_ex(subject_str,
							ZSTR_VAL(search_str)[0],
							ZSTR_VAL(replace_str),
							ZSTR_LEN(replace_str),
							case_sensitivity,
							&replace_count));
		} else if (ZSTR_LEN(search_str) > 1) {
			if (case_sensitivity) {
				ZVAL_STR(result, crx_str_to_str_ex(subject_str,
						ZSTR_VAL(search_str), ZSTR_LEN(search_str),
						ZSTR_VAL(replace_str), ZSTR_LEN(replace_str), &replace_count));
			} else {
				lc_subject_str = crex_string_tolower(subject_str);
				ZVAL_STR(result, crx_str_to_str_i_ex(subject_str, ZSTR_VAL(lc_subject_str),
						search_str, ZSTR_VAL(replace_str), ZSTR_LEN(replace_str), &replace_count));
				crex_string_release_ex(lc_subject_str, 0);
			}
		} else {
			ZVAL_STR_COPY(result, subject_str);
		}
	}
	return replace_count;
}
/* }}} */

/* {{{ crx_str_replace_common */
static void crx_str_replace_common(INTERNAL_FUNCTION_PARAMETERS, bool case_sensitivity)
{
	crex_string *search_str;
	HashTable *search_ht;
	crex_string *replace_str;
	HashTable *replace_ht;
	crex_string *subject_str;
	HashTable *subject_ht;
	zval *subject_entry, *zcount = NULL;
	zval result;
	crex_string *string_key;
	crex_ulong num_key;
	crex_long count = 0;

	CREX_PARSE_PARAMETERS_START(3, 4)
		C_PARAM_ARRAY_HT_OR_STR(search_ht, search_str)
		C_PARAM_ARRAY_HT_OR_STR(replace_ht, replace_str)
		C_PARAM_ARRAY_HT_OR_STR(subject_ht, subject_str)
		C_PARAM_OPTIONAL
		C_PARAM_ZVAL(zcount)
	CREX_PARSE_PARAMETERS_END();

	/* Make sure we're dealing with strings and do the replacement. */
	if (search_str && replace_ht) {
		crex_argument_type_error(2, "must be of type string when argument #1 ($search) is a string");
		RETURN_THROWS();
	}

	/* if subject is an array */
	if (subject_ht) {
		array_init(return_value);

		/* For each subject entry, convert it to string, then perform replacement
		   and add the result to the return_value array. */
		CREX_HASH_FOREACH_KEY_VAL(subject_ht, num_key, string_key, subject_entry) {
			crex_string *tmp_subject_str;
			ZVAL_DEREF(subject_entry);
			subject_str = zval_get_tmp_string(subject_entry, &tmp_subject_str);
			count += crx_str_replace_in_subject(search_str, search_ht, replace_str, replace_ht, subject_str, &result, case_sensitivity);
			crex_tmp_string_release(tmp_subject_str);

			/* Add to return array */
			if (string_key) {
				crex_hash_add_new(C_ARRVAL_P(return_value), string_key, &result);
			} else {
				crex_hash_index_add_new(C_ARRVAL_P(return_value), num_key, &result);
			}
		} CREX_HASH_FOREACH_END();
	} else {	/* if subject is not an array */
		count = crx_str_replace_in_subject(search_str, search_ht, replace_str, replace_ht, subject_str, return_value, case_sensitivity);
	}
	if (zcount) {
		CREX_TRY_ASSIGN_REF_LONG(zcount, count);
	}
}
/* }}} */

/* {{{ Replaces all occurrences of search in haystack with replace */
CRX_FUNCTION(str_replace)
{
	crx_str_replace_common(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ Replaces all occurrences of search in haystack with replace / case-insensitive */
CRX_FUNCTION(str_ireplace)
{
	crx_str_replace_common(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ Converts logical Hebrew text to visual text */
CRX_FUNCTION(hebrev)
{
	char *str, *heb_str, *target;
	const char *tmp;
	size_t block_start, block_end, block_type, i;
	crex_long max_chars=0, char_count;
	size_t begin, end, orig_begin;
	size_t str_len;
	crex_string *broken_str;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STRING(str, str_len)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(max_chars)
	CREX_PARSE_PARAMETERS_END();

	if (str_len == 0) {
		RETURN_EMPTY_STRING();
	}

	tmp = str;
	block_start=block_end=0;

	heb_str = (char *) emalloc(str_len+1);
	target = heb_str+str_len;
	*target = 0;
	target--;

	if (isheb(*tmp)) {
		block_type = _HEB_BLOCK_TYPE_HEB;
	} else {
		block_type = _HEB_BLOCK_TYPE_ENG;
	}

	do {
		if (block_type == _HEB_BLOCK_TYPE_HEB) {
			while ((isheb((int)*(tmp+1)) || _isblank((int)*(tmp+1)) || ispunct((int)*(tmp+1)) || (int)*(tmp+1)=='\n' ) && block_end<str_len-1) {
				tmp++;
				block_end++;
			}
			for (i = block_start+1; i<= block_end+1; i++) {
				*target = str[i-1];
				switch (*target) {
					case '(':
						*target = ')';
						break;
					case ')':
						*target = '(';
						break;
					case '[':
						*target = ']';
						break;
					case ']':
						*target = '[';
						break;
					case '{':
						*target = '}';
						break;
					case '}':
						*target = '{';
						break;
					case '<':
						*target = '>';
						break;
					case '>':
						*target = '<';
						break;
					case '\\':
						*target = '/';
						break;
					case '/':
						*target = '\\';
						break;
					default:
						break;
				}
				target--;
			}
			block_type = _HEB_BLOCK_TYPE_ENG;
		} else {
			while (!isheb(*(tmp+1)) && (int)*(tmp+1)!='\n' && block_end < str_len-1) {
				tmp++;
				block_end++;
			}
			while ((_isblank((int)*tmp) || ispunct((int)*tmp)) && *tmp!='/' && *tmp!='-' && block_end > block_start) {
				tmp--;
				block_end--;
			}
			for (i = block_end+1; i >= block_start+1; i--) {
				*target = str[i-1];
				target--;
			}
			block_type = _HEB_BLOCK_TYPE_HEB;
		}
		block_start=block_end+1;
	} while (block_end < str_len-1);


	broken_str = crex_string_alloc(str_len, 0);
	begin = end = str_len-1;
	target = ZSTR_VAL(broken_str);

	while (1) {
		char_count=0;
		while ((!max_chars || (max_chars > 0 && char_count < max_chars)) && begin > 0) {
			char_count++;
			begin--;
			if (_isnewline(heb_str[begin])) {
				while (begin > 0 && _isnewline(heb_str[begin-1])) {
					begin--;
					char_count++;
				}
				break;
			}
		}
		if (max_chars >= 0 && char_count == max_chars) { /* try to avoid breaking words */
			size_t new_char_count=char_count, new_begin=begin;

			while (new_char_count > 0) {
				if (_isblank(heb_str[new_begin]) || _isnewline(heb_str[new_begin])) {
					break;
				}
				new_begin++;
				new_char_count--;
			}
			if (new_char_count > 0) {
				begin=new_begin;
			}
		}
		orig_begin=begin;

		if (_isblank(heb_str[begin])) {
			heb_str[begin]='\n';
		}
		while (begin <= end && _isnewline(heb_str[begin])) { /* skip leading newlines */
			begin++;
		}
		for (i = begin; i <= end; i++) { /* copy content */
			*target = heb_str[i];
			target++;
		}
		for (i = orig_begin; i <= end && _isnewline(heb_str[i]); i++) {
			*target = heb_str[i];
			target++;
		}
		begin=orig_begin;

		if (begin == 0) {
			*target = 0;
			break;
		}
		begin--;
		end=begin;
	}
	efree(heb_str);

	RETURN_NEW_STR(broken_str);
}
/* }}} */

/* {{{ Converts newlines to HTML line breaks */
CRX_FUNCTION(nl2br)
{
	/* in brief this inserts <br /> or <br> before matched regexp \n\r?|\r\n? */
	const char	*tmp, *end;
	crex_string *str;
	char *target;
	size_t	repl_cnt = 0;
	bool	is_xhtml = 1;
	crex_string *result;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STR(str)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(is_xhtml)
	CREX_PARSE_PARAMETERS_END();

	tmp = ZSTR_VAL(str);
	end = ZSTR_VAL(str) + ZSTR_LEN(str);

	/* it is really faster to scan twice and allocate mem once instead of scanning once
	   and constantly reallocing */
	while (tmp < end) {
		if (*tmp == '\r') {
			if (*(tmp+1) == '\n') {
				tmp++;
			}
			repl_cnt++;
		} else if (*tmp == '\n') {
			if (*(tmp+1) == '\r') {
				tmp++;
			}
			repl_cnt++;
		}

		tmp++;
	}

	if (repl_cnt == 0) {
		RETURN_STR_COPY(str);
	}

	{
		size_t repl_len = is_xhtml ? (sizeof("<br />") - 1) : (sizeof("<br>") - 1);

		result = crex_string_safe_alloc(repl_cnt, repl_len, ZSTR_LEN(str), 0);
		target = ZSTR_VAL(result);
	}

	tmp = ZSTR_VAL(str);
	while (tmp < end) {
		switch (*tmp) {
			case '\r':
			case '\n':
				*target++ = '<';
				*target++ = 'b';
				*target++ = 'r';

				if (is_xhtml) {
					*target++ = ' ';
					*target++ = '/';
				}

				*target++ = '>';

				if ((*tmp == '\r' && *(tmp+1) == '\n') || (*tmp == '\n' && *(tmp+1) == '\r')) {
					*target++ = *tmp++;
				}
				CREX_FALLTHROUGH;
			default:
				*target++ = *tmp;
		}

		tmp++;
	}

	*target = '\0';

	RETURN_NEW_STR(result);
}
/* }}} */

/* {{{ Strips HTML and CRX tags from a string */
CRX_FUNCTION(strip_tags)
{
	crex_string *buf;
	crex_string *str;
	crex_string *allow_str = NULL;
	HashTable *allow_ht = NULL;
	const char *allowed_tags=NULL;
	size_t allowed_tags_len=0;
	smart_str tags_ss = {0};

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STR(str)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT_OR_STR_OR_NULL(allow_ht, allow_str)
	CREX_PARSE_PARAMETERS_END();

	if (allow_ht) {
		zval *tmp;
		crex_string *tag;

		CREX_HASH_FOREACH_VAL(allow_ht, tmp) {
			tag = zval_get_string(tmp);
			smart_str_appendc(&tags_ss, '<');
			smart_str_append(&tags_ss, tag);
			smart_str_appendc(&tags_ss, '>');
			crex_string_release(tag);
		} CREX_HASH_FOREACH_END();
		if (tags_ss.s) {
			smart_str_0(&tags_ss);
			allowed_tags = ZSTR_VAL(tags_ss.s);
			allowed_tags_len = ZSTR_LEN(tags_ss.s);
		}
	} else if (allow_str) {
		allowed_tags = ZSTR_VAL(allow_str);
		allowed_tags_len = ZSTR_LEN(allow_str);
	}

	buf = crex_string_init(ZSTR_VAL(str), ZSTR_LEN(str), 0);
	ZSTR_LEN(buf) = crx_strip_tags_ex(ZSTR_VAL(buf), ZSTR_LEN(str), allowed_tags, allowed_tags_len, 0);
	smart_str_free(&tags_ss);
	RETURN_NEW_STR(buf);
}
/* }}} */

static crex_string *try_setlocale_str(crex_long cat, crex_string *loc) {
	const char *retval;

	if (crex_string_equals_literal(loc, "0")) {
		loc = NULL;
	} else {
		if (ZSTR_LEN(loc) >= 255) {
			crx_error_docref(NULL, E_WARNING, "Specified locale name is too long");
			return NULL;
		}
	}

# ifndef CRX_WIN32
	retval = setlocale(cat, loc ? ZSTR_VAL(loc) : NULL);
# else
	if (loc) {
		/* BC: don't try /^[a-z]{2}_[A-Z]{2}($|\..*)/ except for /^u[ks]_U[KS]$/ */
		char *locp = ZSTR_VAL(loc);
		if (ZSTR_LEN(loc) >= 5 && locp[2] == '_'
			&& locp[0] >= 'a' && locp[0] <= 'z' && locp[1] >= 'a' && locp[1] <= 'z'
			&& locp[3] >= 'A' && locp[3] <= 'Z' && locp[4] >= 'A' && locp[4] <= 'Z'
			&& (locp[5] == '\0' || locp[5] == '.')
			&& !(locp[0] == 'u' && (locp[1] == 'k' || locp[1] == 's')
				&& locp[3] == 'U' && (locp[4] == 'K' || locp[4] == 'S')
				&& locp[5] == '\0')
		) {
			retval = NULL;
		} else {
			retval = setlocale(cat, ZSTR_VAL(loc));
		}
	} else {
		retval = setlocale(cat, NULL);
	}
# endif
	if (!retval) {
		return NULL;
	}

	if (loc) {
		/* Remember if locale was changed */
		size_t len = strlen(retval);

		BG(locale_changed) = 1;
		if (cat == LC_CTYPE || cat == LC_ALL) {
			crex_update_current_locale();
			if (BG(ctype_string)) {
				crex_string_release_ex(BG(ctype_string), 0);
			}
			if (len == 1 && *retval == 'C') {
				/* C locale is represented as NULL. */
				BG(ctype_string) = NULL;
				return ZSTR_CHAR('C');
			} else if (crex_string_equals_cstr(loc, retval, len)) {
				BG(ctype_string) = crex_string_copy(loc);
				return crex_string_copy(BG(ctype_string));
			} else {
				BG(ctype_string) = crex_string_init(retval, len, 0);
				return crex_string_copy(BG(ctype_string));
			}
		} else if (crex_string_equals_cstr(loc, retval, len)) {
			return crex_string_copy(loc);
		}
	}
	return crex_string_init(retval, strlen(retval), 0);
}

static crex_string *try_setlocale_zval(crex_long cat, zval *loc_zv) {
	crex_string *tmp_loc_str;
	crex_string *loc_str = zval_try_get_tmp_string(loc_zv, &tmp_loc_str);
	if (UNEXPECTED(loc_str == NULL)) {
		return NULL;
	}
	crex_string *result = try_setlocale_str(cat, loc_str);
	crex_tmp_string_release(tmp_loc_str);
	return result;
}

/* {{{ Set locale information */
CRX_FUNCTION(setlocale)
{
	crex_long cat;
	zval *args = NULL;
	int num_args;

	CREX_PARSE_PARAMETERS_START(2, -1)
		C_PARAM_LONG(cat)
		C_PARAM_VARIADIC('+', args, num_args)
	CREX_PARSE_PARAMETERS_END();

	for (uint32_t i = 0; i < num_args; i++) {
		if (C_TYPE(args[i]) == IS_ARRAY) {
			zval *elem;
			CREX_HASH_FOREACH_VAL(C_ARRVAL(args[i]), elem) {
				crex_string *result = try_setlocale_zval(cat, elem);
				if (EG(exception)) {
					RETURN_THROWS();
				}
				if (result) {
					RETURN_STR(result);
				}
			} CREX_HASH_FOREACH_END();
		} else {
			crex_string *result = try_setlocale_zval(cat, &args[i]);
			if (EG(exception)) {
				RETURN_THROWS();
			}
			if (result) {
				RETURN_STR(result);
			}
		}
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ Parses GET/POST/COOKIE data and sets global variables */
CRX_FUNCTION(parse_str)
{
	char *arg;
	zval *arrayArg = NULL;
	char *res = NULL;
	size_t arglen;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_STRING(arg, arglen)
		C_PARAM_ZVAL(arrayArg)
	CREX_PARSE_PARAMETERS_END();

	arrayArg = crex_try_array_init(arrayArg);
	if (!arrayArg) {
		RETURN_THROWS();
	}

	res = estrndup(arg, arglen);
	sapi_module.treat_data(PARSE_STRING, res, arrayArg);
}
/* }}} */

#define CRX_TAG_BUF_SIZE 1023

/* {{{ crx_tag_find
 *
 * Check if tag is in a set of tags
 *
 * states:
 *
 * 0 start tag
 * 1 first non-whitespace char seen
 */
static bool crx_tag_find(char *tag, size_t len, const char *set) {
	char c, *n;
	const char *t;
	int state = 0;
	bool done = 0;
	char *norm;

	if (len == 0) {
		return 0;
	}

	norm = emalloc(len+1);

	n = norm;
	t = tag;
	c = crex_tolower_ascii(*t);
	/*
	   normalize the tag removing leading and trailing whitespace
	   and turn any <a whatever...> into just <a> and any </tag>
	   into <tag>
	*/
	while (!done) {
		switch (c) {
			case '<':
				*(n++) = c;
				break;
			case '>':
				done =1;
				break;
			default:
				if (!isspace((int)c)) {
					if (state == 0) {
						state=1;
					}
					if (c != '/' || (*(t-1) != '<' && *(t+1) != '>')) {
						*(n++) = c;
					}
				} else {
					if (state == 1)
						done=1;
				}
				break;
		}
		c = crex_tolower_ascii(*(++t));
	}
	*(n++) = '>';
	*n = '\0';
	if (strstr(set, norm)) {
		done=1;
	} else {
		done=0;
	}
	efree(norm);
	return done;
}
/* }}} */

CRXAPI size_t crx_strip_tags(char *rbuf, size_t len, const char *allow, size_t allow_len) /* {{{ */
{
	return crx_strip_tags_ex(rbuf, len, allow, allow_len, 0);
}
/* }}} */

/* {{{ crx_strip_tags

	A simple little state-machine to strip out html and crx tags

	State 0 is the output state, State 1 means we are inside a
	normal html tag and state 2 means we are inside a crx tag.

	The state variable is passed in to allow a function like fgetss
	to maintain state across calls to the function.

	lc holds the last significant character read and br is a bracket
	counter.

	When an allow string is passed in we keep track of the string
	in state 1 and when the tag is closed check it against the
	allow string to see if we should allow it.

	swm: Added ability to strip <?xml tags without assuming it CRX
	code.
*/
CRXAPI size_t crx_strip_tags_ex(char *rbuf, size_t len, const char *allow, size_t allow_len, bool allow_tag_spaces)
{
	char *tbuf, *tp, *rp, c, lc;
	const char *buf, *p, *end;
	int br, depth=0, in_q = 0;
	uint8_t state = 0;
	size_t pos;
	char *allow_free = NULL;
	char is_xml = 0;

	buf = estrndup(rbuf, len);
	end = buf + len;
	lc = '\0';
	p = buf;
	rp = rbuf;
	br = 0;
	if (allow) {
		allow_free = crex_str_tolower_dup_ex(allow, allow_len);
		allow = allow_free ? allow_free : allow;
		tbuf = emalloc(CRX_TAG_BUF_SIZE + 1);
		tp = tbuf;
	} else {
		tbuf = tp = NULL;
	}

state_0:
	if (p >= end) {
		goto finish;
	}
	c = *p;
	switch (c) {
		case '\0':
			break;
		case '<':
			if (in_q) {
				break;
			}
			if (isspace(*(p + 1)) && !allow_tag_spaces) {
				*(rp++) = c;
				break;
			}
			lc = '<';
			state = 1;
			if (allow) {
				if (tp - tbuf >= CRX_TAG_BUF_SIZE) {
					pos = tp - tbuf;
					tbuf = erealloc(tbuf, (tp - tbuf) + CRX_TAG_BUF_SIZE + 1);
					tp = tbuf + pos;
				}
				*(tp++) = '<';
			}
			p++;
			goto state_1;
		case '>':
			if (depth) {
				depth--;
				break;
			}

			if (in_q) {
				break;
			}

			*(rp++) = c;
			break;
		default:
			*(rp++) = c;
			break;
	}
	p++;
	goto state_0;

state_1:
	if (p >= end) {
		goto finish;
	}
	c = *p;
	switch (c) {
		case '\0':
			break;
		case '<':
			if (in_q) {
				break;
			}
			if (isspace(*(p + 1)) && !allow_tag_spaces) {
				goto reg_char_1;
			}
			depth++;
			break;
		case '>':
			if (depth) {
				depth--;
				break;
			}
			if (in_q) {
				break;
			}

			lc = '>';
			if (is_xml && p >= buf + 1 && *(p -1) == '-') {
				break;
			}
			in_q = state = is_xml = 0;
			if (allow) {
				if (tp - tbuf >= CRX_TAG_BUF_SIZE) {
					pos = tp - tbuf;
					tbuf = erealloc(tbuf, (tp - tbuf) + CRX_TAG_BUF_SIZE + 1);
					tp = tbuf + pos;
				}
				*(tp++) = '>';
				*tp='\0';
				if (crx_tag_find(tbuf, tp-tbuf, allow)) {
					memcpy(rp, tbuf, tp-tbuf);
					rp += tp-tbuf;
				}
				tp = tbuf;
			}
			p++;
			goto state_0;
		case '"':
		case '\'':
			if (p != buf && (!in_q || *p == in_q)) {
				if (in_q) {
					in_q = 0;
				} else {
					in_q = *p;
				}
			}
			goto reg_char_1;
		case '!':
			/* JavaScript & Other HTML scripting languages */
			if (p >= buf + 1 && *(p-1) == '<') {
				state = 3;
				lc = c;
				p++;
				goto state_3;
			} else {
				goto reg_char_1;
			}
			break;
		case '?':
			if (p >= buf + 1 && *(p-1) == '<') {
				br=0;
				state = 2;
				p++;
				goto state_2;
			} else {
				goto reg_char_1;
			}
			break;
		default:
reg_char_1:
			if (allow) {
				if (tp - tbuf >= CRX_TAG_BUF_SIZE) {
					pos = tp - tbuf;
					tbuf = erealloc(tbuf, (tp - tbuf) + CRX_TAG_BUF_SIZE + 1);
					tp = tbuf + pos;
				}
				*(tp++) = c;
			}
			break;
	}
	p++;
	goto state_1;

state_2:
	if (p >= end) {
		goto finish;
	}
	c = *p;
	switch (c) {
		case '(':
			if (lc != '"' && lc != '\'') {
				lc = '(';
				br++;
			}
			break;
		case ')':
			if (lc != '"' && lc != '\'') {
				lc = ')';
				br--;
			}
			break;
		case '>':
			if (depth) {
				depth--;
				break;
			}
			if (in_q) {
				break;
			}

			if (!br && p >= buf + 1 && lc != '\"' && *(p-1) == '?') {
				in_q = state = 0;
				tp = tbuf;
				p++;
				goto state_0;
			}
			break;
		case '"':
		case '\'':
			if (p >= buf + 1 && *(p-1) != '\\') {
				if (lc == c) {
					lc = '\0';
				} else if (lc != '\\') {
					lc = c;
				}
				if (p != buf && (!in_q || *p == in_q)) {
					if (in_q) {
						in_q = 0;
					} else {
						in_q = *p;
					}
				}
			}
			break;
		case 'l':
		case 'L':
			/* swm: If we encounter '<?xml' then we shouldn't be in
			 * state == 2 (CRX). Switch back to HTML.
			 */
			if (state == 2 && p > buf+4
				     && (*(p-1) == 'm' || *(p-1) == 'M')
				     && (*(p-2) == 'x' || *(p-2) == 'X')
				     && *(p-3) == '?'
				     && *(p-4) == '<') {
				state = 1; is_xml=1;
				p++;
				goto state_1;
			}
			break;
		default:
			break;
	}
	p++;
	goto state_2;

state_3:
	if (p >= end) {
		goto finish;
	}
	c = *p;
	switch (c) {
		case '>':
			if (depth) {
				depth--;
				break;
			}
			if (in_q) {
				break;
			}
			in_q = state = 0;
			tp = tbuf;
			p++;
			goto state_0;
		case '"':
		case '\'':
			if (p != buf && *(p-1) != '\\' && (!in_q || *p == in_q)) {
				if (in_q) {
					in_q = 0;
				} else {
					in_q = *p;
				}
			}
			break;
		case '-':
			if (p >= buf + 2 && *(p-1) == '-' && *(p-2) == '!') {
				state = 4;
				p++;
				goto state_4;
			}
			break;
		case 'E':
		case 'e':
			/* !DOCTYPE exception */
			if (p > buf+6
			     && (*(p-1) == 'p' || *(p-1) == 'P')
			     && (*(p-2) == 'y' || *(p-2) == 'Y')
			     && (*(p-3) == 't' || *(p-3) == 'T')
			     && (*(p-4) == 'c' || *(p-4) == 'C')
			     && (*(p-5) == 'o' || *(p-5) == 'O')
			     && (*(p-6) == 'd' || *(p-6) == 'D')) {
				state = 1;
				p++;
				goto state_1;
			}
			break;
		default:
			break;
	}
	p++;
	goto state_3;

state_4:
	while (p < end) {
		c = *p;
		if (c == '>' && !in_q) {
			if (p >= buf + 2 && *(p-1) == '-' && *(p-2) == '-') {
				in_q = state = 0;
				tp = tbuf;
				p++;
				goto state_0;
			}
		}
		p++;
	}

finish:
	if (rp < rbuf + len) {
		*rp = '\0';
	}
	efree((void *)buf);
	if (tbuf) {
		efree(tbuf);
	}
	if (allow_free) {
		efree(allow_free);
	}

	return (size_t)(rp - rbuf);
}
/* }}} */

/* {{{ Parse a CSV string into an array */
CRX_FUNCTION(str_getcsv)
{
	crex_string *str;
	char delim = ',', enc = '"';
	int esc = (unsigned char) '\\';
	char *delim_str = NULL, *enc_str = NULL, *esc_str = NULL;
	size_t delim_len = 0, enc_len = 0, esc_len = 0;

	CREX_PARSE_PARAMETERS_START(1, 4)
		C_PARAM_STR(str)
		C_PARAM_OPTIONAL
		C_PARAM_STRING(delim_str, delim_len)
		C_PARAM_STRING(enc_str, enc_len)
		C_PARAM_STRING(esc_str, esc_len)
	CREX_PARSE_PARAMETERS_END();

	delim = delim_len ? delim_str[0] : delim;
	enc = enc_len ? enc_str[0] : enc;
	if (esc_str != NULL) {
		esc = esc_len ? (unsigned char) esc_str[0] : CRX_CSV_NO_ESCAPE;
	}

	HashTable *values = crx_fgetcsv(NULL, delim, enc, esc, ZSTR_LEN(str), ZSTR_VAL(str));
	if (values == NULL) {
		values = crx_bc_fgetcsv_empty_line();
	}
	RETURN_ARR(values);
}
/* }}} */

/* {{{ Returns the input string repeat mult times */
CRX_FUNCTION(str_repeat)
{
	crex_string		*input_str;		/* Input string */
	crex_long 		mult;			/* Multiplier */
	crex_string	*result;		/* Resulting string */
	size_t		result_len;		/* Length of the resulting string */

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_STR(input_str)
		C_PARAM_LONG(mult)
	CREX_PARSE_PARAMETERS_END();

	if (mult < 0) {
		crex_argument_value_error(2, "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	/* Don't waste our time if it's empty */
	/* ... or if the multiplier is zero */
	if (ZSTR_LEN(input_str) == 0 || mult == 0)
		RETURN_EMPTY_STRING();

	/* Initialize the result string */
	result = crex_string_safe_alloc(ZSTR_LEN(input_str), mult, 0, 0);
	result_len = ZSTR_LEN(input_str) * mult;
	ZSTR_COPY_CONCAT_PROPERTIES(result, input_str);

	/* Heavy optimization for situations where input string is 1 byte long */
	if (ZSTR_LEN(input_str) == 1) {
		memset(ZSTR_VAL(result), *ZSTR_VAL(input_str), mult);
	} else {
		const char *s, *ee;
		char *e;
		ptrdiff_t l=0;
		memcpy(ZSTR_VAL(result), ZSTR_VAL(input_str), ZSTR_LEN(input_str));
		s = ZSTR_VAL(result);
		e = ZSTR_VAL(result) + ZSTR_LEN(input_str);
		ee = ZSTR_VAL(result) + result_len;

		while (e<ee) {
			l = (e-s) < (ee-e) ? (e-s) : (ee-e);
			memmove(e, s, l);
			e += l;
		}
	}

	ZSTR_VAL(result)[result_len] = '\0';

	RETURN_NEW_STR(result);
}
/* }}} */

/* {{{ Returns info about what characters are used in input */
CRX_FUNCTION(count_chars)
{
	crex_string *input;
	int chars[256];
	crex_long mymode=0;
	const unsigned char *buf;
	int inx;
	char retstr[256];
	size_t retlen=0;
	size_t tmp = 0;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STR(input)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(mymode)
	CREX_PARSE_PARAMETERS_END();

	if (mymode < 0 || mymode > 4) {
		crex_argument_value_error(2, "must be between 0 and 4 (inclusive)");
		RETURN_THROWS();
	}

	buf = (const unsigned char *) ZSTR_VAL(input);
	memset((void*) chars, 0, sizeof(chars));

	while (tmp < ZSTR_LEN(input)) {
		chars[*buf]++;
		buf++;
		tmp++;
	}

	if (mymode < 3) {
		array_init(return_value);
	}

	for (inx = 0; inx < 256; inx++) {
		switch (mymode) {
	 		case 0:
				add_index_long(return_value, inx, chars[inx]);
				break;
	 		case 1:
				if (chars[inx] != 0) {
					add_index_long(return_value, inx, chars[inx]);
				}
				break;
			case 2:
				if (chars[inx] == 0) {
					add_index_long(return_value, inx, chars[inx]);
				}
				break;
	  		case 3:
				if (chars[inx] != 0) {
					retstr[retlen++] = inx;
				}
				break;
			case 4:
				if (chars[inx] == 0) {
					retstr[retlen++] = inx;
				}
				break;
		}
	}

	if (mymode == 3 || mymode == 4) {
		RETURN_STRINGL(retstr, retlen);
	}
}
/* }}} */

/* {{{ crx_strnatcmp */
static void crx_strnatcmp(INTERNAL_FUNCTION_PARAMETERS, bool is_case_insensitive)
{
	crex_string *s1, *s2;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_STR(s1)
		C_PARAM_STR(s2)
	CREX_PARSE_PARAMETERS_END();

	RETURN_LONG(strnatcmp_ex(ZSTR_VAL(s1), ZSTR_LEN(s1),
							 ZSTR_VAL(s2), ZSTR_LEN(s2),
							 is_case_insensitive));
}
/* }}} */

/* {{{ Returns the result of string comparison using 'natural' algorithm */
CRX_FUNCTION(strnatcmp)
{
	crx_strnatcmp(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ Returns the result of case-insensitive string comparison using 'natural' algorithm */
CRX_FUNCTION(strnatcasecmp)
{
	crx_strnatcmp(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ Returns numeric formatting information based on the current locale */
CRX_FUNCTION(localeconv)
{
	zval grouping, mon_grouping;
	size_t len, i;

	CREX_PARSE_PARAMETERS_NONE();

	array_init(return_value);
	array_init(&grouping);
	array_init(&mon_grouping);

	{
		struct lconv currlocdata;

		localeconv_r( &currlocdata );

		/* Grab the grouping data out of the array */
		len = strlen(currlocdata.grouping);

		for (i = 0; i < len; i++) {
			add_index_long(&grouping, i, currlocdata.grouping[i]);
		}

		/* Grab the monetary grouping data out of the array */
		len = strlen(currlocdata.mon_grouping);

		for (i = 0; i < len; i++) {
			add_index_long(&mon_grouping, i, currlocdata.mon_grouping[i]);
		}

		add_assoc_string(return_value, "decimal_point",     currlocdata.decimal_point);
		add_assoc_string(return_value, "thousands_sep",     currlocdata.thousands_sep);
		add_assoc_string(return_value, "int_curr_symbol",   currlocdata.int_curr_symbol);
		add_assoc_string(return_value, "currency_symbol",   currlocdata.currency_symbol);
		add_assoc_string(return_value, "mon_decimal_point", currlocdata.mon_decimal_point);
		add_assoc_string(return_value, "mon_thousands_sep", currlocdata.mon_thousands_sep);
		add_assoc_string(return_value, "positive_sign",     currlocdata.positive_sign);
		add_assoc_string(return_value, "negative_sign",     currlocdata.negative_sign);
		add_assoc_long(  return_value, "int_frac_digits",   currlocdata.int_frac_digits);
		add_assoc_long(  return_value, "frac_digits",       currlocdata.frac_digits);
		add_assoc_long(  return_value, "p_cs_precedes",     currlocdata.p_cs_precedes);
		add_assoc_long(  return_value, "p_sep_by_space",    currlocdata.p_sep_by_space);
		add_assoc_long(  return_value, "n_cs_precedes",     currlocdata.n_cs_precedes);
		add_assoc_long(  return_value, "n_sep_by_space",    currlocdata.n_sep_by_space);
		add_assoc_long(  return_value, "p_sign_posn",       currlocdata.p_sign_posn);
		add_assoc_long(  return_value, "n_sign_posn",       currlocdata.n_sign_posn);
	}

	crex_hash_str_update(C_ARRVAL_P(return_value), "grouping", sizeof("grouping")-1, &grouping);
	crex_hash_str_update(C_ARRVAL_P(return_value), "mon_grouping", sizeof("mon_grouping")-1, &mon_grouping);
}
/* }}} */

/* {{{ Returns the number of times a substring occurs in the string */
CRX_FUNCTION(substr_count)
{
	char *haystack, *needle;
	crex_long offset = 0, length = 0;
	bool length_is_null = 1;
	crex_long count;
	size_t haystack_len, needle_len;
	const char *p, *endp;

	CREX_PARSE_PARAMETERS_START(2, 4)
		C_PARAM_STRING(haystack, haystack_len)
		C_PARAM_STRING(needle, needle_len)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(offset)
		C_PARAM_LONG_OR_NULL(length, length_is_null)
	CREX_PARSE_PARAMETERS_END();

	if (needle_len == 0) {
		crex_argument_value_error(2, "cannot be empty");
		RETURN_THROWS();
	}

	p = haystack;

	if (offset) {
		if (offset < 0) {
			offset += (crex_long)haystack_len;
		}
		if ((offset < 0) || ((size_t)offset > haystack_len)) {
			crex_argument_value_error(3, "must be contained in argument #1 ($haystack)");
			RETURN_THROWS();
		}
		p += offset;
		haystack_len -= offset;
	}

	if (!length_is_null) {
		if (length < 0) {
			length += haystack_len;
		}
		if (length < 0 || ((size_t)length > haystack_len)) {
			crex_argument_value_error(4, "must be contained in argument #1 ($haystack)");
			RETURN_THROWS();
		}
	} else {
		length = haystack_len;
	}

	if (needle_len == 1) {
		count = count_chars(p, length, needle[0]);
	} else {
		count = 0;
		endp = p + length;
		while ((p = (char*)crx_memnstr(p, needle, needle_len, endp))) {
			p += needle_len;
			count++;
		}
	}

	RETURN_LONG(count);
}
/* }}} */

/* {{{ Returns input string padded on the left or right to specified length with pad_string */
CRX_FUNCTION(str_pad)
{
	/* Input arguments */
	crex_string *input;				/* Input string */
	crex_long pad_length;			/* Length to pad to */

	/* Helper variables */
	size_t num_pad_chars;		/* Number of padding characters (total - input size) */
	char *pad_str = " "; /* Pointer to padding string */
	size_t pad_str_len = 1;
	crex_long   pad_type_val = CRX_STR_PAD_RIGHT; /* The padding type value */
	size_t	   i, left_pad=0, right_pad=0;
	crex_string *result = NULL;	/* Resulting string */

	CREX_PARSE_PARAMETERS_START(2, 4)
		C_PARAM_STR(input)
		C_PARAM_LONG(pad_length)
		C_PARAM_OPTIONAL
		C_PARAM_STRING(pad_str, pad_str_len)
		C_PARAM_LONG(pad_type_val)
	CREX_PARSE_PARAMETERS_END();

	/* If resulting string turns out to be shorter than input string,
	   we simply copy the input and return. */
	if (pad_length < 0  || (size_t)pad_length <= ZSTR_LEN(input)) {
		RETURN_STR_COPY(input);
	}

	if (pad_str_len == 0) {
		crex_argument_value_error(3, "must be a non-empty string");
		RETURN_THROWS();
	}

	if (pad_type_val < CRX_STR_PAD_LEFT || pad_type_val > CRX_STR_PAD_BOTH) {
		crex_argument_value_error(4, "must be STR_PAD_LEFT, STR_PAD_RIGHT, or STR_PAD_BOTH");
		RETURN_THROWS();
	}

	num_pad_chars = pad_length - ZSTR_LEN(input);
	result = crex_string_safe_alloc(1, ZSTR_LEN(input), num_pad_chars, 0);
	ZSTR_LEN(result) = 0;

	/* We need to figure out the left/right padding lengths. */
	switch (pad_type_val) {
		case CRX_STR_PAD_RIGHT:
			left_pad = 0;
			right_pad = num_pad_chars;
			break;

		case CRX_STR_PAD_LEFT:
			left_pad = num_pad_chars;
			right_pad = 0;
			break;

		case CRX_STR_PAD_BOTH:
			left_pad = num_pad_chars / 2;
			right_pad = num_pad_chars - left_pad;
			break;
	}

	/* First we pad on the left. */
	for (i = 0; i < left_pad; i++)
		ZSTR_VAL(result)[ZSTR_LEN(result)++] = pad_str[i % pad_str_len];

	/* Then we copy the input string. */
	memcpy(ZSTR_VAL(result) + ZSTR_LEN(result), ZSTR_VAL(input), ZSTR_LEN(input));
	ZSTR_LEN(result) += ZSTR_LEN(input);

	/* Finally, we pad on the right. */
	for (i = 0; i < right_pad; i++)
		ZSTR_VAL(result)[ZSTR_LEN(result)++] = pad_str[i % pad_str_len];

	ZSTR_VAL(result)[ZSTR_LEN(result)] = '\0';

	RETURN_NEW_STR(result);
}
/* }}} */

/* {{{ Implements an ANSI C compatible sscanf */
CRX_FUNCTION(sscanf)
{
	zval *args = NULL;
	char *str, *format;
	size_t str_len, format_len;
	int result, num_args = 0;

	CREX_PARSE_PARAMETERS_START(2, -1)
		C_PARAM_STRING(str, str_len)
		C_PARAM_STRING(format, format_len)
		C_PARAM_VARIADIC('*', args, num_args)
	CREX_PARSE_PARAMETERS_END();

	result = crx_sscanf_internal(str, format, num_args, args, 0, return_value);

	if (SCAN_ERROR_WRONG_PARAM_COUNT == result) {
		WRONG_PARAM_COUNT;
	}
}
/* }}} */

/* static crex_string *crx_str_rot13(crex_string *str) {{{ */
static crex_string *crx_str_rot13(crex_string *str)
{
	crex_string *ret;
	const char *p, *e;
	char *target;

	if (UNEXPECTED(ZSTR_LEN(str) == 0)) {
		return ZSTR_EMPTY_ALLOC();
	}

	ret = crex_string_alloc(ZSTR_LEN(str), 0);

	p = ZSTR_VAL(str);
	e = p + ZSTR_LEN(str);
	target = ZSTR_VAL(ret);

#ifdef __SSE2__
	if (e - p > 15) {
		const __m128i a_minus_1 = _mm_set1_epi8('a' - 1);
		const __m128i m_plus_1 = _mm_set1_epi8('m' + 1);
		const __m128i n_minus_1 = _mm_set1_epi8('n' - 1);
		const __m128i z_plus_1 = _mm_set1_epi8('z' + 1);
		const __m128i A_minus_1 = _mm_set1_epi8('A' - 1);
		const __m128i M_plus_1 = _mm_set1_epi8('M' + 1);
		const __m128i N_minus_1 = _mm_set1_epi8('N' - 1);
		const __m128i C_plus_1 = _mm_set1_epi8('Z' + 1);
		const __m128i add = _mm_set1_epi8(13);
		const __m128i sub = _mm_set1_epi8(-13);

		do {
			__m128i in, gt, lt, cmp, delta;

			delta = _mm_setzero_si128();
			in = _mm_loadu_si128((__m128i *)p);

			gt = _mm_cmpgt_epi8(in, a_minus_1);
			lt = _mm_cmplt_epi8(in, m_plus_1);
			cmp = _mm_and_si128(lt, gt);
			if (_mm_movemask_epi8(cmp)) {
				cmp = _mm_and_si128(cmp, add);
				delta = _mm_or_si128(delta, cmp);
			}

			gt = _mm_cmpgt_epi8(in, n_minus_1);
			lt = _mm_cmplt_epi8(in, z_plus_1);
			cmp = _mm_and_si128(lt, gt);
			if (_mm_movemask_epi8(cmp)) {
				cmp = _mm_and_si128(cmp, sub);
				delta = _mm_or_si128(delta, cmp);
			}

			gt = _mm_cmpgt_epi8(in, A_minus_1);
			lt = _mm_cmplt_epi8(in, M_plus_1);
			cmp = _mm_and_si128(lt, gt);
			if (_mm_movemask_epi8(cmp)) {
				cmp = _mm_and_si128(cmp, add);
				delta = _mm_or_si128(delta, cmp);
			}

			gt = _mm_cmpgt_epi8(in, N_minus_1);
			lt = _mm_cmplt_epi8(in, C_plus_1);
			cmp = _mm_and_si128(lt, gt);
			if (_mm_movemask_epi8(cmp)) {
				cmp = _mm_and_si128(cmp, sub);
				delta = _mm_or_si128(delta, cmp);
			}

			in = _mm_add_epi8(in, delta);
			_mm_storeu_si128((__m128i *)target, in);

			p += 16;
			target += 16;
		} while (e - p > 15);
	}
#endif

	while (p < e) {
		if (*p >= 'a' && *p <= 'z') {
			*target++ = 'a' + (((*p++ - 'a') + 13) % 26);
		} else if (*p >= 'A' && *p <= 'Z') {
			*target++ = 'A' + (((*p++ - 'A') + 13) % 26);
		} else {
			*target++ = *p++;
		}
	}

	*target = '\0';

	return ret;
}
/* }}} */

/* {{{ Perform the rot13 transform on a string */
CRX_FUNCTION(str_rot13)
{
	crex_string *arg;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(arg)
	CREX_PARSE_PARAMETERS_END();

	RETURN_STR(crx_str_rot13(arg));
}
/* }}} */

/* {{{ crx_binary_string_shuffle */
CRXAPI bool crx_binary_string_shuffle(const crx_random_algo *algo, crx_random_status *status, char *str, crex_long len) /* {{{ */
{
	int64_t n_elems, rnd_idx, n_left;
	char temp;

	/* The implementation is stolen from array_data_shuffle       */
	/* Thus the characteristics of the randomization are the same */
	n_elems = len;

	if (n_elems <= 1) {
		return true;
	}

	n_left = n_elems;

	while (--n_left) {
		rnd_idx = algo->range(status, 0, n_left);
		if (EG(exception)) {
			return false;
		}
		if (rnd_idx != n_left) {
			temp = str[n_left];
			str[n_left] = str[rnd_idx];
			str[rnd_idx] = temp;
		}
	}

	return true;
}
/* }}} */

/* {{{ Shuffles string. One permutation of all possible is created */
CRX_FUNCTION(str_shuffle)
{
	crex_string *arg;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(arg)
	CREX_PARSE_PARAMETERS_END();

	RETVAL_STRINGL(ZSTR_VAL(arg), ZSTR_LEN(arg));
	if (C_STRLEN_P(return_value) > 1) {
		crx_binary_string_shuffle(
			crx_random_default_algo(),
			crx_random_default_status(),
			C_STRVAL_P(return_value),
			C_STRLEN_P(return_value)
		);
	}
}
/* }}} */

/* {{{ Counts the number of words inside a string. If format of 1 is specified,
   	then the function will return an array containing all the words
   	found inside the string. If format of 2 is specified, then the function
   	will return an associated array where the position of the word is the key
   	and the word itself is the value.
   	For the purpose of this function, 'word' is defined as a locale dependent
   	string containing alphabetic characters, which also may contain, but not start
   	with "'" and "-" characters.
*/
CRX_FUNCTION(str_word_count)
{
	crex_string *str;
	char *char_list = NULL, ch[256];
	const char *p, *e, *s;
	size_t char_list_len = 0, word_count = 0;
	crex_long type = 0;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_STR(str)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(type)
		C_PARAM_STRING_OR_NULL(char_list, char_list_len)
	CREX_PARSE_PARAMETERS_END();

	switch(type) {
		case 1:
		case 2:
			array_init(return_value);
			if (!ZSTR_LEN(str)) {
				return;
			}
			break;
		case 0:
			if (!ZSTR_LEN(str)) {
				RETURN_LONG(0);
			}
			/* nothing to be done */
			break;
		default:
			crex_argument_value_error(2, "must be a valid format value");
			RETURN_THROWS();
	}

	if (char_list) {
		crx_charmask((const unsigned char *) char_list, char_list_len, ch);
	}

	p = ZSTR_VAL(str);
	e = ZSTR_VAL(str) + ZSTR_LEN(str);

	/* first character cannot be ' or -, unless explicitly allowed by the user */
	if ((*p == '\'' && (!char_list || !ch['\''])) || (*p == '-' && (!char_list || !ch['-']))) {
		p++;
	}
	/* last character cannot be -, unless explicitly allowed by the user */
	if (*(e - 1) == '-' && (!char_list || !ch['-'])) {
		e--;
	}

	while (p < e) {
		s = p;
		while (p < e && (isalpha((unsigned char)*p) || (char_list && ch[(unsigned char)*p]) || *p == '\'' || *p == '-')) {
			p++;
		}
		if (p > s) {
			switch (type)
			{
				case 1:
					add_next_index_stringl(return_value, s, p - s);
					break;
				case 2:
					add_index_stringl(return_value, (s - ZSTR_VAL(str)), s, p - s);
					break;
				default:
					word_count++;
					break;
			}
		}
		p++;
	}

	if (!type) {
		RETURN_LONG(word_count);
	}
}

/* }}} */

/* {{{ Convert a string to an array. If split_length is specified, break the string down into chunks each split_length characters long. */
CRX_FUNCTION(str_split)
{
	crex_string *str;
	crex_long split_length = 1;
	const char *p;
	size_t n_reg_segments;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STR(str)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(split_length)
	CREX_PARSE_PARAMETERS_END();

	if (split_length <= 0) {
		crex_argument_value_error(2, "must be greater than 0");
		RETURN_THROWS();
	}

	if ((size_t)split_length >= ZSTR_LEN(str)) {
		if (0 == ZSTR_LEN(str)) {
			RETURN_EMPTY_ARRAY();
		}

		array_init_size(return_value, 1);
		add_next_index_stringl(return_value, ZSTR_VAL(str), ZSTR_LEN(str));
		return;
	}

	array_init_size(return_value, (uint32_t)(((ZSTR_LEN(str) - 1) / split_length) + 1));

	n_reg_segments = ZSTR_LEN(str) / split_length;
	p = ZSTR_VAL(str);

	while (n_reg_segments-- > 0) {
		add_next_index_stringl(return_value, p, split_length);
		p += split_length;
	}

	if (p != (ZSTR_VAL(str) + ZSTR_LEN(str))) {
		add_next_index_stringl(return_value, p, (ZSTR_VAL(str) + ZSTR_LEN(str) - p));
	}
}
/* }}} */

/* {{{ Search a string for any of a set of characters */
CRX_FUNCTION(strpbrk)
{
	crex_string *haystack, *char_list;
	const char *haystack_ptr, *cl_ptr;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_STR(haystack)
		C_PARAM_STR(char_list)
	CREX_PARSE_PARAMETERS_END();

	if (!ZSTR_LEN(char_list)) {
		crex_argument_value_error(2, "must be a non-empty string");
		RETURN_THROWS();
	}

	for (haystack_ptr = ZSTR_VAL(haystack); haystack_ptr < (ZSTR_VAL(haystack) + ZSTR_LEN(haystack)); ++haystack_ptr) {
		for (cl_ptr = ZSTR_VAL(char_list); cl_ptr < (ZSTR_VAL(char_list) + ZSTR_LEN(char_list)); ++cl_ptr) {
			if (*cl_ptr == *haystack_ptr) {
				RETURN_STRINGL(haystack_ptr, (ZSTR_VAL(haystack) + ZSTR_LEN(haystack) - haystack_ptr));
			}
		}
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ Binary safe optionally case insensitive comparison of 2 strings from an offset, up to length characters */
CRX_FUNCTION(substr_compare)
{
	crex_string *s1, *s2;
	crex_long offset, len=0;
	bool len_is_default=1;
	bool cs=0;
	size_t cmp_len;

	CREX_PARSE_PARAMETERS_START(3, 5)
		C_PARAM_STR(s1)
		C_PARAM_STR(s2)
		C_PARAM_LONG(offset)
		C_PARAM_OPTIONAL
		C_PARAM_LONG_OR_NULL(len, len_is_default)
		C_PARAM_BOOL(cs)
	CREX_PARSE_PARAMETERS_END();

	if (!len_is_default && len <= 0) {
		if (len == 0) {
			RETURN_LONG(0L);
		} else {
			crex_argument_value_error(4, "must be greater than or equal to 0");
			RETURN_THROWS();
		}
	}

	if (offset < 0) {
		offset = ZSTR_LEN(s1) + offset;
		offset = (offset < 0) ? 0 : offset;
	}

	if ((size_t)offset > ZSTR_LEN(s1)) {
		crex_argument_value_error(3, "must be contained in argument #1 ($haystack)");
		RETURN_THROWS();
	}

	cmp_len = len ? (size_t)len : MAX(ZSTR_LEN(s2), (ZSTR_LEN(s1) - offset));

	if (!cs) {
		RETURN_LONG(crex_binary_strncmp(ZSTR_VAL(s1) + offset, (ZSTR_LEN(s1) - offset), ZSTR_VAL(s2), ZSTR_LEN(s2), cmp_len));
	} else {
		RETURN_LONG(crex_binary_strncasecmp_l(ZSTR_VAL(s1) + offset, (ZSTR_LEN(s1) - offset), ZSTR_VAL(s2), ZSTR_LEN(s2), cmp_len));
	}
}
/* }}} */

/* {{{ */
static crex_string *crx_utf8_encode(const char *s, size_t len)
{
	size_t pos = len;
	crex_string *str;
	unsigned char c;

	str = crex_string_safe_alloc(len, 2, 0, 0);
	ZSTR_LEN(str) = 0;
	while (pos > 0) {
		/* The lower 256 codepoints of Unicode are identical to Latin-1,
		 * so we don't need to do any mapping here. */
		c = (unsigned char)(*s);
		if (c < 0x80) {
			ZSTR_VAL(str)[ZSTR_LEN(str)++] = (char) c;
		/* We only account for the single-byte and two-byte cases because
		 * we're only dealing with the first 256 Unicode codepoints. */
		} else {
			ZSTR_VAL(str)[ZSTR_LEN(str)++] = (0xc0 | (c >> 6));
			ZSTR_VAL(str)[ZSTR_LEN(str)++] = (0x80 | (c & 0x3f));
		}
		pos--;
		s++;
	}
	ZSTR_VAL(str)[ZSTR_LEN(str)] = '\0';
	str = crex_string_truncate(str, ZSTR_LEN(str), 0);
	return str;
}
/* }}} */

/* {{{ */
static crex_string *crx_utf8_decode(const char *s, size_t len)
{
	size_t pos = 0;
	unsigned int c;
	crex_string *str;

	str = crex_string_alloc(len, 0);
	ZSTR_LEN(str) = 0;
	while (pos < len) {
		crex_result status = FAILURE;
		c = crx_next_utf8_char((const unsigned char*)s, (size_t) len, &pos, &status);

		/* The lower 256 codepoints of Unicode are identical to Latin-1,
		 * so we don't need to do any mapping here beyond replacing non-Latin-1
		 * characters. */
		if (status == FAILURE || c > 0xFFU) {
			c = '?';
		}

		ZSTR_VAL(str)[ZSTR_LEN(str)++] = c;
	}
	ZSTR_VAL(str)[ZSTR_LEN(str)] = '\0';
	if (ZSTR_LEN(str) < len) {
		str = crex_string_truncate(str, ZSTR_LEN(str), 0);
	}

	return str;
}
/* }}} */

/* {{{ Encodes an ISO-8859-1 string to UTF-8 */
CRX_FUNCTION(utf8_encode)
{
	char *arg;
	size_t arg_len;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STRING(arg, arg_len)
	CREX_PARSE_PARAMETERS_END();

	RETURN_STR(crx_utf8_encode(arg, arg_len));
}
/* }}} */

/* {{{ Converts a UTF-8 encoded string to ISO-8859-1 */
CRX_FUNCTION(utf8_decode)
{
	char *arg;
	size_t arg_len;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STRING(arg, arg_len)
	CREX_PARSE_PARAMETERS_END();

	RETURN_STR(crx_utf8_decode(arg, arg_len));
}
/* }}} */
