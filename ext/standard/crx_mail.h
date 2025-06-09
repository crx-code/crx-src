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
   | Author: Rasmus Lerdorf <rasmus@lerdorf.on.ca>                        |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_MAIL_H
#define CRX_MAIL_H

CRX_MINFO_FUNCTION(mail);

CRXAPI crex_string *crx_mail_build_headers(HashTable *headers);
CRXAPI extern int crx_mail(const char *to, const char *subject, const char *message, const char *headers, const char *extra_cmd);

#define CRX_MAIL_BUILD_HEADER_CHECK(target, s, key, val) \
do { \
	if (C_TYPE_P(val) == IS_STRING) { \
		crx_mail_build_headers_elem(&s, key, val); \
	} else if (C_TYPE_P(val) == IS_ARRAY) { \
		if (crex_string_equals_literal_ci(key, target)) { \
			crex_type_error("Header \"%s\" must be of type string, array given", target); \
			break; \
		} \
		crx_mail_build_headers_elems(&s, key, val); \
	} else { \
		crex_type_error("Header \"%s\" must be of type array|string, %s given", ZSTR_VAL(key), crex_zval_value_name(val)); \
	} \
} while(0)


#define CRX_MAIL_BUILD_HEADER_DEFAULT(s, key, val) \
do { \
	if (C_TYPE_P(val) == IS_STRING) { \
		crx_mail_build_headers_elem(&s, key, val); \
	} else if (C_TYPE_P(val) == IS_ARRAY) { \
		crx_mail_build_headers_elems(&s, key, val); \
	} else { \
		crex_type_error("Header \"%s\" must be of type array|string, %s given", ZSTR_VAL(key), crex_zval_value_name(val)); \
	} \
} while(0)


#endif /* CRX_MAIL_H */
