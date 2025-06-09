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
  | Author: Anatol Belski <ab@crx.net>                                   |
  +----------------------------------------------------------------------+
*/

#include "crx.h"
#include "crx_libmagic.h"

crex_string* convert_libmagic_pattern(const char *val, size_t len, uint32_t options)
{
	int i, j;
	crex_string *t;

	for (i = j = 0; i < len; i++) {
		switch (val[i]) {
			case '~':
				j += 2;
				break;
			case '\0':
				j += 4;
				break;
			default:
				j++;
				break;
		}
	}
	t = crex_string_alloc(j + 4, 0);

	j = 0;
	ZSTR_VAL(t)[j++] = '~';

	for (i = 0; i < len; i++, j++) {
		switch (val[i]) {
			case '~':
				ZSTR_VAL(t)[j++] = '\\';
				ZSTR_VAL(t)[j] = '~';
				break;
			case '\0':
				ZSTR_VAL(t)[j++] = '\\';
				ZSTR_VAL(t)[j++] = 'x';
				ZSTR_VAL(t)[j++] = '0';
				ZSTR_VAL(t)[j] = '0';
				break;
			default:
				ZSTR_VAL(t)[j] = val[i];
				break;
		}
	}
	ZSTR_VAL(t)[j++] = '~';

	if (options & PCRE2_CASELESS)
		ZSTR_VAL(t)[j++] = 'i';

	if (options & PCRE2_MULTILINE)
		ZSTR_VAL(t)[j++] = 'm';

	ZSTR_VAL(t)[j]='\0';
	ZSTR_LEN(t) = j;

	return t;
}



