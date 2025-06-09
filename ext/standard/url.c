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
   | Author: Jim Winstead <jimw@crx.net>                                  |
   +----------------------------------------------------------------------+
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#ifdef __SSE2__
#include <emmintrin.h>
#endif

#include "crx.h"

#include "url.h"
#include "file.h"

/* {{{ free_url */
CRXAPI void crx_url_free(crx_url *theurl)
{
	if (theurl->scheme)
		crex_string_release_ex(theurl->scheme, 0);
	if (theurl->user)
		crex_string_release_ex(theurl->user, 0);
	if (theurl->pass)
		crex_string_release_ex(theurl->pass, 0);
	if (theurl->host)
		crex_string_release_ex(theurl->host, 0);
	if (theurl->path)
		crex_string_release_ex(theurl->path, 0);
	if (theurl->query)
		crex_string_release_ex(theurl->query, 0);
	if (theurl->fragment)
		crex_string_release_ex(theurl->fragment, 0);
	efree(theurl);
}
/* }}} */

/* {{{ crx_replace_controlchars_ex */
CRXAPI char *crx_replace_controlchars_ex(char *str, size_t len)
{
	unsigned char *s = (unsigned char *)str;
	unsigned char *e = (unsigned char *)str + len;

	if (!str) {
		return (NULL);
	}

	while (s < e) {

		if (iscntrl(*s)) {
			*s='_';
		}
		s++;
	}

	return (str);
}
/* }}} */

CRXAPI char *crx_replace_controlchars(char *str)
{
	return crx_replace_controlchars_ex(str, strlen(str));
}

CRXAPI crx_url *crx_url_parse(char const *str)
{
	return crx_url_parse_ex(str, strlen(str));
}

static const char *binary_strcspn(const char *s, const char *e, const char *chars) {
	while (*chars) {
		const char *p = memchr(s, *chars, e - s);
		if (p) {
			e = p;
		}
		chars++;
	}
	return e;
}

/* {{{ crx_url_parse */
CRXAPI crx_url *crx_url_parse_ex(char const *str, size_t length)
{
	bool has_port;
	return crx_url_parse_ex2(str, length, &has_port);
}

/* {{{ crx_url_parse_ex2
 */
CRXAPI crx_url *crx_url_parse_ex2(char const *str, size_t length, bool *has_port)
{
	char port_buf[6];
	crx_url *ret = ecalloc(1, sizeof(crx_url));
	char const *s, *e, *p, *pp, *ue;

	*has_port = 0;
	s = str;
	ue = s + length;

	/* parse scheme */
	if ((e = memchr(s, ':', length)) && e != s) {
		/* validate scheme */
		p = s;
		while (p < e) {
			/* scheme = 1*[ lowalpha | digit | "+" | "-" | "." ] */
			if (!isalpha(*p) && !isdigit(*p) && *p != '+' && *p != '.' && *p != '-') {
				if (e + 1 < ue && e < binary_strcspn(s, ue, "?#")) {
					goto parse_port;
				} else if (s + 1 < ue && *s == '/' && *(s + 1) == '/') { /* relative-scheme URL */
					s += 2;
					e = 0;
					goto parse_host;
				} else {
					goto just_path;
				}
			}
			p++;
		}

		if (e + 1 == ue) { /* only scheme is available */
			ret->scheme = crex_string_init(s, (e - s), 0);
			crx_replace_controlchars_ex(ZSTR_VAL(ret->scheme), ZSTR_LEN(ret->scheme));
			return ret;
		}

		/*
		 * certain schemas like mailto: and zlib: may not have any / after them
		 * this check ensures we support those.
		 */
		if (*(e+1) != '/') {
			/* check if the data we get is a port this allows us to
			 * correctly parse things like a.com:80
			 */
			p = e + 1;
			while (p < ue && isdigit(*p)) {
				p++;
			}

			if ((p == ue || *p == '/') && (p - e) < 7) {
				goto parse_port;
			}

			ret->scheme = crex_string_init(s, (e-s), 0);
			crx_replace_controlchars_ex(ZSTR_VAL(ret->scheme), ZSTR_LEN(ret->scheme));

			s = e + 1;
			goto just_path;
		} else {
			ret->scheme = crex_string_init(s, (e-s), 0);
			crx_replace_controlchars_ex(ZSTR_VAL(ret->scheme), ZSTR_LEN(ret->scheme));

			if (e + 2 < ue && *(e + 2) == '/') {
				s = e + 3;
				if (crex_string_equals_literal_ci(ret->scheme, "file")) {
					if (e + 3 < ue && *(e + 3) == '/') {
						/* support windows drive letters as in:
						   file:///c:/somedir/file.txt
						*/
						if (e + 5 < ue && *(e + 5) == ':') {
							s = e + 4;
						}
						goto just_path;
					}
				}
			} else {
				s = e + 1;
				goto just_path;
			}
		}
	} else if (e) { /* no scheme; starts with colon: look for port */
		parse_port:
		p = e + 1;
		pp = p;

		while (pp < ue && pp - p < 6 && isdigit(*pp)) {
			pp++;
		}

		if (pp - p > 0 && pp - p < 6 && (pp == ue || *pp == '/')) {
			crex_long port;
			char *end;
			memcpy(port_buf, p, (pp - p));
			port_buf[pp - p] = '\0';
			port = CREX_STRTOL(port_buf, &end, 10);
			if (port >= 0 && port <= 65535 && end != port_buf) {
				*has_port = 1;
				ret->port = (unsigned short) port;
				if (s + 1 < ue && *s == '/' && *(s + 1) == '/') { /* relative-scheme URL */
				    s += 2;
				}
			} else {
				crx_url_free(ret);
				return NULL;
			}
		} else if (p == pp && pp == ue) {
			crx_url_free(ret);
			return NULL;
		} else if (s + 1 < ue && *s == '/' && *(s + 1) == '/') { /* relative-scheme URL */
			s += 2;
		} else {
			goto just_path;
		}
	} else if (s + 1 < ue && *s == '/' && *(s + 1) == '/') { /* relative-scheme URL */
		s += 2;
	} else {
		goto just_path;
	}

parse_host:
	e = binary_strcspn(s, ue, "/?#");

	/* check for login and password */
	if ((p = crex_memrchr(s, '@', (e-s)))) {
		if ((pp = memchr(s, ':', (p-s)))) {
			ret->user = crex_string_init(s, (pp-s), 0);
			crx_replace_controlchars_ex(ZSTR_VAL(ret->user), ZSTR_LEN(ret->user));

			pp++;
			ret->pass = crex_string_init(pp, (p-pp), 0);
			crx_replace_controlchars_ex(ZSTR_VAL(ret->pass), ZSTR_LEN(ret->pass));
		} else {
			ret->user = crex_string_init(s, (p-s), 0);
			crx_replace_controlchars_ex(ZSTR_VAL(ret->user), ZSTR_LEN(ret->user));
		}

		s = p + 1;
	}

	/* check for port */
	if (s < ue && *s == '[' && *(e-1) == ']') {
		/* Short circuit portscan,
		   we're dealing with an
		   IPv6 embedded address */
		p = NULL;
	} else {
		p = crex_memrchr(s, ':', (e-s));
	}

	if (p) {
		if (!ret->port) {
			p++;
			if (e-p > 5) { /* port cannot be longer then 5 characters */
				crx_url_free(ret);
				return NULL;
			} else if (e - p > 0) {
				crex_long port;
				char *end;
				memcpy(port_buf, p, (e - p));
				port_buf[e - p] = '\0';
				port = CREX_STRTOL(port_buf, &end, 10);
				if (port >= 0 && port <= 65535 && end != port_buf) {
					*has_port = 1;
					ret->port = (unsigned short)port;
				} else {
					crx_url_free(ret);
					return NULL;
				}
			}
			p--;
		}
	} else {
		p = e;
	}

	/* check if we have a valid host, if we don't reject the string as url */
	if ((p-s) < 1) {
		crx_url_free(ret);
		return NULL;
	}

	ret->host = crex_string_init(s, (p-s), 0);
	crx_replace_controlchars_ex(ZSTR_VAL(ret->host), ZSTR_LEN(ret->host));

	if (e == ue) {
		return ret;
	}

	s = e;

	just_path:

	e = ue;
	p = memchr(s, '#', (e - s));
	if (p) {
		p++;
		if (p < e) {
			ret->fragment = crex_string_init(p, (e - p), 0);
			crx_replace_controlchars_ex(ZSTR_VAL(ret->fragment), ZSTR_LEN(ret->fragment));
		} else {
			ret->fragment = ZSTR_EMPTY_ALLOC();
		}
		e = p-1;
	}

	p = memchr(s, '?', (e - s));
	if (p) {
		p++;
		if (p < e) {
			ret->query = crex_string_init(p, (e - p), 0);
			crx_replace_controlchars_ex(ZSTR_VAL(ret->query), ZSTR_LEN(ret->query));
		} else {
			ret->query = ZSTR_EMPTY_ALLOC();
		}
		e = p-1;
	}

	if (s < e || s == ue) {
		ret->path = crex_string_init(s, (e - s), 0);
		crx_replace_controlchars_ex(ZSTR_VAL(ret->path), ZSTR_LEN(ret->path));
	}

	return ret;
}
/* }}} */

/* {{{ Parse a URL and return its components */
CRX_FUNCTION(parse_url)
{
	char *str;
	size_t str_len;
	crx_url *resource;
	crex_long key = -1;
	zval tmp;
	bool has_port;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STRING(str, str_len)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(key)
	CREX_PARSE_PARAMETERS_END();

	resource = crx_url_parse_ex2(str, str_len, &has_port);
	if (resource == NULL) {
		/* @todo Find a method to determine why crx_url_parse_ex() failed */
		RETURN_FALSE;
	}

	if (key > -1) {
		switch (key) {
			case CRX_URL_SCHEME:
				if (resource->scheme != NULL) RETVAL_STR_COPY(resource->scheme);
				break;
			case CRX_URL_HOST:
				if (resource->host != NULL) RETVAL_STR_COPY(resource->host);
				break;
			case CRX_URL_PORT:
				if (has_port) RETVAL_LONG(resource->port);
				break;
			case CRX_URL_USER:
				if (resource->user != NULL) RETVAL_STR_COPY(resource->user);
				break;
			case CRX_URL_PASS:
				if (resource->pass != NULL) RETVAL_STR_COPY(resource->pass);
				break;
			case CRX_URL_PATH:
				if (resource->path != NULL) RETVAL_STR_COPY(resource->path);
				break;
			case CRX_URL_QUERY:
				if (resource->query != NULL) RETVAL_STR_COPY(resource->query);
				break;
			case CRX_URL_FRAGMENT:
				if (resource->fragment != NULL) RETVAL_STR_COPY(resource->fragment);
				break;
			default:
				crex_argument_value_error(2, "must be a valid URL component identifier, " CREX_LONG_FMT " given", key);
				break;
		}
		goto done;
	}

	/* allocate an array for return */
	array_init(return_value);

    /* add the various elements to the array */
	if (resource->scheme != NULL) {
		ZVAL_STR_COPY(&tmp, resource->scheme);
		crex_hash_add_new(C_ARRVAL_P(return_value), ZSTR_KNOWN(CREX_STR_SCHEME), &tmp);
	}
	if (resource->host != NULL) {
		ZVAL_STR_COPY(&tmp, resource->host);
		crex_hash_add_new(C_ARRVAL_P(return_value), ZSTR_KNOWN(CREX_STR_HOST), &tmp);
	}
	if (has_port) {
		ZVAL_LONG(&tmp, resource->port);
		crex_hash_add_new(C_ARRVAL_P(return_value), ZSTR_KNOWN(CREX_STR_PORT), &tmp);
	}
	if (resource->user != NULL) {
		ZVAL_STR_COPY(&tmp, resource->user);
		crex_hash_add_new(C_ARRVAL_P(return_value), ZSTR_KNOWN(CREX_STR_USER), &tmp);
	}
	if (resource->pass != NULL) {
		ZVAL_STR_COPY(&tmp, resource->pass);
		crex_hash_add_new(C_ARRVAL_P(return_value), ZSTR_KNOWN(CREX_STR_PASS), &tmp);
	}
	if (resource->path != NULL) {
		ZVAL_STR_COPY(&tmp, resource->path);
		crex_hash_add_new(C_ARRVAL_P(return_value), ZSTR_KNOWN(CREX_STR_PATH), &tmp);
	}
	if (resource->query != NULL) {
		ZVAL_STR_COPY(&tmp, resource->query);
		crex_hash_add_new(C_ARRVAL_P(return_value), ZSTR_KNOWN(CREX_STR_QUERY), &tmp);
	}
	if (resource->fragment != NULL) {
		ZVAL_STR_COPY(&tmp, resource->fragment);
		crex_hash_add_new(C_ARRVAL_P(return_value), ZSTR_KNOWN(CREX_STR_FRAGMENT), &tmp);
	}
done:
	crx_url_free(resource);
}
/* }}} */

/* {{{ crx_htoi */
static int crx_htoi(char *s)
{
	int value;
	int c;

	c = ((unsigned char *)s)[0];
	if (isupper(c))
		c = tolower(c);
	value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

	c = ((unsigned char *)s)[1];
	if (isupper(c))
		c = tolower(c);
	value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

	return (value);
}
/* }}} */

/* rfc1738:

   ...The characters ";",
   "/", "?", ":", "@", "=" and "&" are the characters which may be
   reserved for special meaning within a scheme...

   ...Thus, only alphanumerics, the special characters "$-_.+!*'(),", and
   reserved characters used for their reserved purposes may be used
   unencoded within a URL...

   For added safety, we only leave -_. unencoded.
 */

static const unsigned char hexchars[] = "0123456789ABCDEF";

static crex_always_inline crex_string *crx_url_encode_impl(const char *s, size_t len, bool raw) /* {{{ */ {
	unsigned char c;
	unsigned char *to;
	unsigned char const *from, *end;
	crex_string *start;

	from = (unsigned char *)s;
	end = (unsigned char *)s + len;
	start = crex_string_safe_alloc(3, len, 0, 0);
	to = (unsigned char*)ZSTR_VAL(start);

#ifdef __SSE2__
	while (from + 16 < end) {
		__m128i mask;
		uint32_t bits;
		const __m128i _A = _mm_set1_epi8('A' - 1);
		const __m128i C_ = _mm_set1_epi8('Z' + 1);
		const __m128i _a = _mm_set1_epi8('a' - 1);
		const __m128i z_ = _mm_set1_epi8('z' + 1);
		const __m128i _zero = _mm_set1_epi8('0' - 1);
		const __m128i nine_ = _mm_set1_epi8('9' + 1);
		const __m128i dot = _mm_set1_epi8('.');
		const __m128i minus = _mm_set1_epi8('-');
		const __m128i under = _mm_set1_epi8('_');

		__m128i in = _mm_loadu_si128((__m128i *)from);

		__m128i gt = _mm_cmpgt_epi8(in, _A);
		__m128i lt = _mm_cmplt_epi8(in, C_);
		mask = _mm_and_si128(lt, gt); /* upper */
		gt = _mm_cmpgt_epi8(in, _a);
		lt = _mm_cmplt_epi8(in, z_);
		mask = _mm_or_si128(mask, _mm_and_si128(lt, gt)); /* lower */
		gt = _mm_cmpgt_epi8(in, _zero);
		lt = _mm_cmplt_epi8(in, nine_);
		mask = _mm_or_si128(mask, _mm_and_si128(lt, gt)); /* number */
		mask = _mm_or_si128(mask, _mm_cmpeq_epi8(in, dot));
		mask = _mm_or_si128(mask, _mm_cmpeq_epi8(in, minus));
		mask = _mm_or_si128(mask, _mm_cmpeq_epi8(in, under));

		if (!raw) {
			const __m128i blank = _mm_set1_epi8(' ');
			__m128i eq = _mm_cmpeq_epi8(in, blank);
			if (_mm_movemask_epi8(eq)) {
				in = _mm_add_epi8(in, _mm_and_si128(eq, _mm_set1_epi8('+' - ' ')));
				mask = _mm_or_si128(mask, eq);
			}
		}
		if (raw) {
			const __m128i wavy = _mm_set1_epi8('~');
			mask = _mm_or_si128(mask, _mm_cmpeq_epi8(in, wavy));
		}
		if (((bits = _mm_movemask_epi8(mask)) & 0xffff) == 0xffff) {
			_mm_storeu_si128((__m128i*)to, in);
			to += 16;
		} else {
			int i;
			unsigned char xmm[16];
			_mm_storeu_si128((__m128i*)xmm, in);
			for (i = 0; i < sizeof(xmm); i++) {
				if ((bits & (0x1 << i))) {
					*to++ = xmm[i];
				} else {
					*to++ = '%';
					*to++ = hexchars[xmm[i] >> 4];
					*to++ = hexchars[xmm[i] & 0xf];
				}
			}
		}
		from += 16;
	}
#endif
	while (from < end) {
		c = *from++;

		if (!raw && c == ' ') {
			*to++ = '+';
		} else if ((c < '0' && c != '-' && c != '.') ||
				(c < 'A' && c > '9') ||
				(c > 'Z' && c < 'a' && c != '_') ||
				(c > 'z' && (!raw || c != '~'))) {
			to[0] = '%';
			to[1] = hexchars[c >> 4];
			to[2] = hexchars[c & 15];
			to += 3;
		} else {
			*to++ = c;
		}
	}
	*to = '\0';

	start = crex_string_truncate(start, to - (unsigned char*)ZSTR_VAL(start), 0);

	return start;
}
/* }}} */

/* {{{ crx_url_encode */
CRXAPI crex_string *crx_url_encode(char const *s, size_t len)
{
	return crx_url_encode_impl(s, len, 0);
}
/* }}} */

/* {{{ URL-encodes string */
CRX_FUNCTION(urlencode)
{
	crex_string *in_str;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(in_str)
	CREX_PARSE_PARAMETERS_END();

	RETURN_STR(crx_url_encode(ZSTR_VAL(in_str), ZSTR_LEN(in_str)));
}
/* }}} */

/* {{{ Decodes URL-encoded string */
CRX_FUNCTION(urldecode)
{
	crex_string *in_str, *out_str;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(in_str)
	CREX_PARSE_PARAMETERS_END();

	out_str = crex_string_init(ZSTR_VAL(in_str), ZSTR_LEN(in_str), 0);
	ZSTR_LEN(out_str) = crx_url_decode(ZSTR_VAL(out_str), ZSTR_LEN(out_str));

	RETURN_NEW_STR(out_str);
}
/* }}} */

/* {{{ crx_url_decode */
CRXAPI size_t crx_url_decode(char *str, size_t len)
{
	char *dest = str;
	char *data = str;

	while (len--) {
		if (*data == '+') {
			*dest = ' ';
		}
		else if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1))
				 && isxdigit((int) *(data + 2))) {
			*dest = (char) crx_htoi(data + 1);
			data += 2;
			len -= 2;
		} else {
			*dest = *data;
		}
		data++;
		dest++;
	}
	*dest = '\0';
	return dest - str;
}
/* }}} */

/* {{{ crx_raw_url_encode */
CRXAPI crex_string *crx_raw_url_encode(char const *s, size_t len)
{
	return crx_url_encode_impl(s, len, 1);
}
/* }}} */

/* {{{ URL-encodes string */
CRX_FUNCTION(rawurlencode)
{
	crex_string *in_str;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(in_str)
	CREX_PARSE_PARAMETERS_END();

	RETURN_STR(crx_raw_url_encode(ZSTR_VAL(in_str), ZSTR_LEN(in_str)));
}
/* }}} */

/* {{{ Decodes URL-encodes string */
CRX_FUNCTION(rawurldecode)
{
	crex_string *in_str, *out_str;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(in_str)
	CREX_PARSE_PARAMETERS_END();

	out_str = crex_string_init(ZSTR_VAL(in_str), ZSTR_LEN(in_str), 0);
	ZSTR_LEN(out_str) = crx_raw_url_decode(ZSTR_VAL(out_str), ZSTR_LEN(out_str));

	RETURN_NEW_STR(out_str);
}
/* }}} */

/* {{{ crx_raw_url_decode */
CRXAPI size_t crx_raw_url_decode(char *str, size_t len)
{
	char *dest = str;
	char *data = str;

	while (len--) {
		if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1))
			&& isxdigit((int) *(data + 2))) {
			*dest = (char) crx_htoi(data + 1);
			data += 2;
			len -= 2;
		} else {
			*dest = *data;
		}
		data++;
		dest++;
	}
	*dest = '\0';
	return dest - str;
}
/* }}} */

/* {{{ fetches all the headers sent by the server in response to a HTTP request */
CRX_FUNCTION(get_headers)
{
	char *url;
	size_t url_len;
	crx_stream *stream;
	zval *prev_val, *hdr = NULL;
	bool format = 0;
	zval *zcontext = NULL;
	crx_stream_context *context;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_PATH(url, url_len)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(format)
		C_PARAM_RESOURCE_OR_NULL(zcontext)
	CREX_PARSE_PARAMETERS_END();

	context = crx_stream_context_from_zval(zcontext, 0);

	if (!(stream = crx_stream_open_wrapper_ex(url, "r", REPORT_ERRORS | STREAM_USE_URL | STREAM_ONLY_GET_HEADERS, NULL, context))) {
		RETURN_FALSE;
	}

	if (C_TYPE(stream->wrapperdata) != IS_ARRAY) {
		crx_stream_close(stream);
		RETURN_FALSE;
	}

	array_init(return_value);

	CREX_HASH_FOREACH_VAL(C_ARRVAL_P(&stream->wrapperdata), hdr) {
		if (C_TYPE_P(hdr) != IS_STRING) {
			continue;
		}
		if (!format) {
no_name_header:
			add_next_index_str(return_value, crex_string_copy(C_STR_P(hdr)));
		} else {
			char c;
			char *s, *p;

			if ((p = strchr(C_STRVAL_P(hdr), ':'))) {
				c = *p;
				*p = '\0';
				s = p + 1;
				while (isspace((int)*(unsigned char *)s)) {
					s++;
				}

				if ((prev_val = crex_hash_str_find(C_ARRVAL_P(return_value), C_STRVAL_P(hdr), (p - C_STRVAL_P(hdr)))) == NULL) {
					add_assoc_stringl_ex(return_value, C_STRVAL_P(hdr), (p - C_STRVAL_P(hdr)), s, (C_STRLEN_P(hdr) - (s - C_STRVAL_P(hdr))));
				} else { /* some headers may occur more than once, therefore we need to remake the string into an array */
					convert_to_array(prev_val);
					add_next_index_stringl(prev_val, s, (C_STRLEN_P(hdr) - (s - C_STRVAL_P(hdr))));
				}

				*p = c;
			} else {
				goto no_name_header;
			}
		}
	} CREX_HASH_FOREACH_END();

	crx_stream_close(stream);
}
/* }}} */
