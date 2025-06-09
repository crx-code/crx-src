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
   | Author: Ilia Alshanetsky <ilia@crx.net>                              |
   +----------------------------------------------------------------------+
 */

/*
 * Portions of this code are based on Berkeley's uuencode/uudecode
 * implementation.
 *
 * Copyright (c) 1983, 1993
 *  The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *  This product includes software developed by the University of
 *  California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <math.h>

#include "crx.h"
#include "crx_uuencode.h"

#define CRX_UU_ENC(c) ((c) ? ((c) & 077) + ' ' : '`')
#define CRX_UU_ENC_C2(c) CRX_UU_ENC(((*(c) << 4) & 060) | ((*((c) + 1) >> 4) & 017))
#define CRX_UU_ENC_C3(c) CRX_UU_ENC(((*(c + 1) << 2) & 074) | ((*((c) + 2) >> 6) & 03))

#define CRX_UU_DEC(c) (((c) - ' ') & 077)

CRXAPI crex_string *crx_uuencode(const char *src, size_t src_len) /* {{{ */
{
	size_t len = 45;
	unsigned char *p;
	const unsigned char *s, *e, *ee;
	crex_string *dest;

	/* encoded length is ~ 38% greater than the original
       Use 1.5 for easier calculation.
    */
	dest = crex_string_safe_alloc(src_len/2, 3, 46, 0);
	p = (unsigned char *) ZSTR_VAL(dest);
	s = (unsigned char *) src;
	e = s + src_len;

	while ((s + 3) < e) {
		ee = s + len;
		if (ee > e) {
			ee = e;
			len = ee - s;
			if (len % 3) {
				ee = s + (int) (floor((double)len / 3) * 3);
			}
		}
		*p++ = CRX_UU_ENC(len);

		while (s < ee) {
			*p++ = CRX_UU_ENC(*s >> 2);
			*p++ = CRX_UU_ENC_C2(s);
			*p++ = CRX_UU_ENC_C3(s);
			*p++ = CRX_UU_ENC(*(s + 2) & 077);

			s += 3;
		}

		if (len == 45) {
			*p++ = '\n';
		}
	}

	if (s < e) {
		if (len == 45) {
			*p++ = CRX_UU_ENC(e - s);
			len = 0;
		}

		*p++ = CRX_UU_ENC(*s >> 2);
		*p++ = CRX_UU_ENC_C2(s);
		*p++ = ((e - s) > 1) ? CRX_UU_ENC_C3(s) : CRX_UU_ENC('\0');
		*p++ = ((e - s) > 2) ? CRX_UU_ENC(*(s + 2) & 077) : CRX_UU_ENC('\0');
	}

	if (len < 45) {
		*p++ = '\n';
	}

	*p++ = CRX_UU_ENC('\0');
	*p++ = '\n';
	*p = '\0';

	dest = crex_string_truncate(dest, (char *) p - ZSTR_VAL(dest), 0);
	return dest;
}
/* }}} */

CRXAPI crex_string *crx_uudecode(const char *src, size_t src_len) /* {{{ */
{
	size_t len, total_len=0;
	char *p;
	const char *s, *e, *ee;
	crex_string *dest;

	if (src_len == 0) {
		return NULL;
	}

	dest = crex_string_alloc((size_t) ceil(src_len * 0.75), 0);
	p = ZSTR_VAL(dest);
	s = src;
	e = src + src_len;

	while (s < e) {
		if ((len = CRX_UU_DEC(*s++)) == 0) {
			break;
		}
		/* sanity check */
		if (len > src_len) {
			goto err;
		}

		total_len += len;

		ee = s + (len == 45 ? 60 : (int) floor(len * 1.33));
		/* sanity check */
		if (ee > e) {
			goto err;
		}

		while (s < ee) {
			if(s+4 > e) {
				goto err;
			}
			*p++ = CRX_UU_DEC(*s) << 2 | CRX_UU_DEC(*(s + 1)) >> 4;
			*p++ = CRX_UU_DEC(*(s + 1)) << 4 | CRX_UU_DEC(*(s + 2)) >> 2;
			*p++ = CRX_UU_DEC(*(s + 2)) << 6 | CRX_UU_DEC(*(s + 3));
			s += 4;
		}

		if (len < 45) {
			break;
		}

		/* skip \n */
		s++;
	}

	assert(p >= ZSTR_VAL(dest));
	if ((len = total_len) > (size_t)(p - ZSTR_VAL(dest))) {
		*p++ = CRX_UU_DEC(*s) << 2 | CRX_UU_DEC(*(s + 1)) >> 4;
		if (len > 1) {
			*p++ = CRX_UU_DEC(*(s + 1)) << 4 | CRX_UU_DEC(*(s + 2)) >> 2;
			if (len > 2) {
				*p++ = CRX_UU_DEC(*(s + 2)) << 6 | CRX_UU_DEC(*(s + 3));
			}
		}
	}

	ZSTR_LEN(dest) = total_len;
	ZSTR_VAL(dest)[ZSTR_LEN(dest)] = '\0';

	return dest;

err:
	crex_string_efree(dest);

	return NULL;
}
/* }}} */

/* {{{ uuencode a string */
CRX_FUNCTION(convert_uuencode)
{
	crex_string *src;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(src)
	CREX_PARSE_PARAMETERS_END();

	RETURN_STR(crx_uuencode(ZSTR_VAL(src), ZSTR_LEN(src)));
}
/* }}} */

/* {{{ decode a uuencoded string */
CRX_FUNCTION(convert_uudecode)
{
	crex_string *src;
	crex_string *dest;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(src)
	CREX_PARSE_PARAMETERS_END();

	if ((dest = crx_uudecode(ZSTR_VAL(src), ZSTR_LEN(src))) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Argument #1 ($data) is not a valid uuencoded string");
		RETURN_FALSE;
	}

	RETURN_STR(dest);
}
/* }}} */
