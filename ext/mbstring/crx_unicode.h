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
   | Author: Wez Furlong (wez@thebrainroom.com)                           |
   +----------------------------------------------------------------------+

	Based on code from ucdata-2.5, which has the following Copyright:

	Copyright 2001 Computing Research Labs, New Mexico State University

	Permission is hereby granted, free of charge, to any person obtaining a
	copy of this software and associated documentation files (the "Software"),
	to deal in the Software without restriction, including without limitation
	the rights to use, copy, modify, merge, publish, distribute, sublicense,
	and/or sell copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.
*/

#ifndef CRX_UNICODE_H
#define CRX_UNICODE_H

#define UC_MN  0 /* Mark, Non-Spacing          */
#define UC_MC  1 /* Mark, Spacing Combining    */
#define UC_ME  2 /* Mark, Enclosing            */
#define UC_ND  3 /* Number, Decimal Digit      */
#define UC_NL  4 /* Number, Letter             */
#define UC_NO  5 /* Number, Other              */
#define UC_ZS  6 /* Separator, Space           */
#define UC_ZL  7 /* Separator, Line            */
#define UC_ZP  8 /* Separator, Paragraph       */
#define UC_OS  9 /* Other, Surrogate           */
#define UC_CO 10 /* Other, Private Use         */
#define UC_CN 11 /* Other, Not Assigned        */
#define UC_LU 12 /* Letter, Uppercase          */
#define UC_LL 13 /* Letter, Lowercase          */
#define UC_LT 14 /* Letter, Titlecase          */
#define UC_LM 15 /* Letter, Modifier           */
#define UC_LO 16 /* Letter, Other              */
#define UC_SM 17 /* Symbol, Math               */
#define UC_SC 18 /* Symbol, Currency           */
#define UC_SK 19 /* Symbol, Modifier           */
#define UC_SO 20 /* Symbol, Other              */
#define UC_L  21 /* Left-To-Right              */
#define UC_R  22 /* Right-To-Left              */
#define UC_EN 23 /* European Number            */
#define UC_ES 24 /* European Number Separator  */
#define UC_ET 25 /* European Number Terminator */
#define UC_AN 26 /* Arabic Number              */
#define UC_CS 27 /* Common Number Separator    */
#define UC_B  28 /* Block Separator            */
#define UC_S  29 /* Segment Separator          */
#define UC_WS 30 /* Whitespace                 */
#define UC_ON 31 /* Other Neutrals             */
#define UC_AL 32 /* Arabic Letter              */

/* Merged property categories */
#define UC_C 33 /* Control */
#define UC_P 34 /* Punctuation */

/* Derived properties from DerivedCoreProperties.txt */
#define UC_CASED          35
#define UC_CASE_IGNORABLE 36


MBSTRING_API bool crx_unicode_is_prop(unsigned long code, ...);
MBSTRING_API bool crx_unicode_is_prop1(unsigned long code, int prop);

typedef enum {
	CRX_UNICODE_CASE_UPPER = 0,
	CRX_UNICODE_CASE_LOWER,
	CRX_UNICODE_CASE_TITLE,
	CRX_UNICODE_CASE_FOLD,
	CRX_UNICODE_CASE_UPPER_SIMPLE,
	CRX_UNICODE_CASE_LOWER_SIMPLE,
	CRX_UNICODE_CASE_TITLE_SIMPLE,
	CRX_UNICODE_CASE_FOLD_SIMPLE,
	CRX_UNICODE_CASE_MODE_MAX
} crx_case_mode;

MBSTRING_API crex_string *crx_unicode_convert_case(
		crx_case_mode case_mode, const char *srcstr, size_t srclen,
		const mbfl_encoding *src_encoding, const mbfl_encoding *dst_encoding, int illegal_mode, uint32_t illegal_substchar);

/* Optimize the common ASCII case for lower/upper */

static inline int crx_unicode_is_lower(unsigned long code) {
	if (code < 0x80) {
		return code >= 0x61 && code <= 0x7A;
	} else {
		return crx_unicode_is_prop1(code, UC_LL);
	}
}

static inline int crx_unicode_is_upper(unsigned long code) {
	if (code < 0x80) {
		return code >= 0x41 && code <= 0x5A;
	} else {
		return crx_unicode_is_prop1(code, UC_LU);
	}
}

#define crx_unicode_is_alpha(cc) crx_unicode_is_prop(cc, UC_LU, UC_LL, UC_LM, UC_LO, UC_LT, -1)
#define crx_unicode_is_digit(cc) crx_unicode_is_prop1(cc, UC_ND)
#define crx_unicode_is_alnum(cc) crx_unicode_is_prop(cc, UC_LU, UC_LL, UC_LM, UC_LO, UC_LT, UC_ND, -1)
#define crx_unicode_is_cntrl(cc) crx_unicode_is_prop1(cc, UC_C)
#define crx_unicode_is_blank(cc) crx_unicode_is_prop1(cc, UC_ZS)
#define crx_unicode_is_punct(cc) crx_unicode_is_prop1(cc, UC_P)
#define crx_unicode_is_graph(cc) crx_unicode_is_prop(cc, \
		UC_MN, UC_MC, UC_ME, UC_ND, UC_NL, UC_NO, \
		UC_LU, UC_LL, UC_LT, UC_LM, UC_LO, UC_P, \
		UC_SM, UC_SM, UC_SC, UC_SK, UC_SO, -1)
#define crx_unicode_is_print(cc) crx_unicode_is_prop(cc, \
		UC_MN, UC_MC, UC_ME, UC_ND, UC_NL, UC_NO, \
		UC_LU, UC_LL, UC_LT, UC_LM, UC_LO, UC_P, \
		UC_SM, UC_SM, UC_SC, UC_SK, UC_SO, UC_ZS, -1)
#define crx_unicode_is_title(cc) crx_unicode_is_prop1(cc, UC_LT)

#define crx_unicode_is_symbol(cc) crx_unicode_is_prop(cc, UC_SM, UC_SC, UC_SO, UC_SK, -1)
#define crx_unicode_is_number(cc) crx_unicode_is_prop(cc, UC_ND, UC_NO, UC_NL, -1)
#define crx_unicode_is_nonspacing(cc) crx_unicode_is_prop1(cc, UC_MN)

/*
 * Directionality macros.
 */
#define crx_unicode_is_rtl(cc) crx_unicode_is_prop1(cc, UC_R)
#define crx_unicode_is_ltr(cc) crx_unicode_is_prop1(cc, UC_L)
#define crx_unicode_is_strong(cc) crx_unicode_is_prop(cc, UC_L, UC_R, -1)
#define crx_unicode_is_weak(cc) crx_unicode_is_prop(cc, UC_EN, UC_ES, UC_ET, UC_AN, UC_CS, -1)
#define crx_unicode_is_neutral(cc) crx_unicode_is_prop(cc, UC_B, UC_S, UC_WS, UC_ON, -1)
#define crx_unicode_is_separator(cc) crx_unicode_is_prop(cc, UC_B, UC_S, -1)

/*
 * Other macros inspired by John Cowan.
 */
#define crx_unicode_is_mark(cc) crx_unicode_is_prop(cc, UC_MN, UC_MC, UC_ME, -1)
#define crx_unicode_is_modif(cc) crx_unicode_is_prop1(cc, UC_LM)
#define crx_unicode_is_letnum(cc) crx_unicode_is_prop1(cc, UC_NL)
#define crx_unicode_is_math(cc) crx_unicode_is_prop1(cc, UC_SM)
#define crx_unicode_is_currency(cc) crx_unicode_is_prop1(cc, UC_SC)
#define crx_unicode_is_modifsymbol(cc) crx_unicode_is_prop1(cc, UC_SK)
#define crx_unicode_is_nsmark(cc) crx_unicode_is_prop1(cc, UC_MN)
#define crx_unicode_is_spmark(cc) crx_unicode_is_prop1(cc, UC_MC)
#define crx_unicode_is_enclosing(cc) crx_unicode_is_prop1(cc, UC_ME)
#define crx_unicode_is_private(cc) crx_unicode_is_prop1(cc, UC_CO)
#define crx_unicode_is_surrogate(cc) crx_unicode_is_prop1(cc, UC_OS)
#define crx_unicode_is_lsep(cc) crx_unicode_is_prop1(cc, UC_ZL)
#define crx_unicode_is_psep(cc) crx_unicode_is_prop1(cc, UC_ZP)

/*
 * Other miscellaneous character property macros.
 */
#define crx_unicode_is_han(cc) (((cc) >= 0x4e00 && (cc) <= 0x9fff) ||\
                     ((cc) >= 0xf900 && (cc) <= 0xfaff))
#define crx_unicode_is_hangul(cc) ((cc) >= 0xac00 && (cc) <= 0xd7ff)

/*
 * Derived core properties.
 */

#define crx_unicode_is_cased(cc) crx_unicode_is_prop1(cc, UC_CASED)
#define crx_unicode_is_case_ignorable(cc) crx_unicode_is_prop1(cc, UC_CASE_IGNORABLE)

#endif /* CRX_UNICODE_H */
