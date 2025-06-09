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
   | Author: Hartmut Holzgraefe <hholzgra@crx.net>                        |
   +----------------------------------------------------------------------+
 */

#include "crx.h"
#include "crx_string.h"

/* {{{ reference_levdist
 * reference implementation, only optimized for memory usage, not speed */
static crex_long reference_levdist(const crex_string *string1, const crex_string *string2, crex_long cost_ins, crex_long cost_rep, crex_long cost_del )
{
	crex_long *p1, *p2, *tmp;
	crex_long c0, c1, c2;
	size_t i1, i2;

	if (ZSTR_LEN(string1) == 0) {
		return ZSTR_LEN(string2) * cost_ins;
	}
	if (ZSTR_LEN(string2) == 0) {
		return ZSTR_LEN(string1) * cost_del;
	}

	p1 = safe_emalloc((ZSTR_LEN(string2) + 1), sizeof(crex_long), 0);
	p2 = safe_emalloc((ZSTR_LEN(string2) + 1), sizeof(crex_long), 0);

	for (i2 = 0; i2 <= ZSTR_LEN(string2); i2++) {
		p1[i2] = i2 * cost_ins;
	}
	for (i1 = 0; i1 < ZSTR_LEN(string1) ; i1++) {
		p2[0] = p1[0] + cost_del;

		for (i2 = 0; i2 < ZSTR_LEN(string2); i2++) {
			c0 = p1[i2] + ((ZSTR_VAL(string1)[i1] == ZSTR_VAL(string2)[i2]) ? 0 : cost_rep);
			c1 = p1[i2 + 1] + cost_del;
			if (c1 < c0) {
				c0 = c1;
			}
			c2 = p2[i2] + cost_ins;
			if (c2 < c0) {
				c0 = c2;
			}
			p2[i2 + 1] = c0;
		}
		tmp = p1;
		p1 = p2;
		p2 = tmp;
	}
	c0 = p1[ZSTR_LEN(string2)];

	efree(p1);
	efree(p2);

	return c0;
}
/* }}} */

/* {{{ Calculate Levenshtein distance between two strings */
CRX_FUNCTION(levenshtein)
{
	crex_string *string1, *string2;
	crex_long cost_ins = 1;
	crex_long cost_rep = 1;
	crex_long cost_del = 1;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "SS|lll", &string1, &string2, &cost_ins, &cost_rep, &cost_del) == FAILURE) {
		RETURN_THROWS();
	}


	RETURN_LONG(reference_levdist(string1, string2, cost_ins, cost_rep, cost_del));
}
/* }}} */
