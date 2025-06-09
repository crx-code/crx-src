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
   | Author: Alex Plotnick <alex@wgate.com>                               |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"

#ifdef HAVE_LIBINTL

#include <stdio.h>
#include "ext/standard/info.h"
#include "crx_gettext.h"
#include "gettext_arginfo.h"

#include <libintl.h>

crex_module_entry crx_gettext_module_entry = {
	STANDARD_MODULE_HEADER,
	"gettext",
	ext_functions,
	NULL,
	NULL,
	NULL,
	NULL,
	CRX_MINFO(crx_gettext),
	CRX_GETTEXT_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_GETTEXT
CREX_GET_MODULE(crx_gettext)
#endif

#define CRX_GETTEXT_MAX_DOMAIN_LENGTH 1024
#define CRX_GETTEXT_MAX_MSGID_LENGTH 4096

#define CRX_GETTEXT_DOMAIN_LENGTH_CHECK(_arg_num, domain_len) \
	if (UNEXPECTED(domain_len > CRX_GETTEXT_MAX_DOMAIN_LENGTH)) { \
		crex_argument_value_error(_arg_num, "is too long"); \
		RETURN_THROWS(); \
	}

#define CRX_GETTEXT_LENGTH_CHECK(_arg_num, check_len) \
	if (UNEXPECTED(check_len > CRX_GETTEXT_MAX_MSGID_LENGTH)) { \
		crex_argument_value_error(_arg_num, "is too long"); \
		RETURN_THROWS(); \
	}

CRX_MINFO_FUNCTION(crx_gettext)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "GetText Support", "enabled");
	crx_info_print_table_end();
}

/* {{{ Set the textdomain to "domain". Returns the current domain */
CRX_FUNCTION(textdomain)
{
	char *domain_name = NULL, *retval;
	crex_string *domain = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "S!", &domain) == FAILURE) {
		RETURN_THROWS();
	}

	if (domain != NULL && ZSTR_LEN(domain) != 0 && !crex_string_equals_literal(domain, "0")) {
		CRX_GETTEXT_DOMAIN_LENGTH_CHECK(1, ZSTR_LEN(domain))
		domain_name = ZSTR_VAL(domain);
	}

	retval = textdomain(domain_name);

	RETURN_STRING(retval);
}
/* }}} */

/* {{{ Return the translation of msgid for the current domain, or msgid unaltered if a translation does not exist */
CRX_FUNCTION(gettext)
{
	char *msgstr;
	crex_string *msgid;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(msgid)
	CREX_PARSE_PARAMETERS_END();

	CRX_GETTEXT_LENGTH_CHECK(1, ZSTR_LEN(msgid))
	msgstr = gettext(ZSTR_VAL(msgid));

	if (msgstr != ZSTR_VAL(msgid)) {
		RETURN_STRING(msgstr);
	} else {
		RETURN_STR_COPY(msgid);
	}
}
/* }}} */

/* {{{ Return the translation of msgid for domain_name, or msgid unaltered if a translation does not exist */
CRX_FUNCTION(dgettext)
{
	char *msgstr;
	crex_string *domain, *msgid;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "SS", &domain, &msgid) == FAILURE)	{
		RETURN_THROWS();
	}

	CRX_GETTEXT_DOMAIN_LENGTH_CHECK(1, ZSTR_LEN(domain))
	CRX_GETTEXT_LENGTH_CHECK(2, ZSTR_LEN(msgid))

	msgstr = dgettext(ZSTR_VAL(domain), ZSTR_VAL(msgid));

	if (msgstr != ZSTR_VAL(msgid)) {
		RETURN_STRING(msgstr);
	} else {
		RETURN_STR_COPY(msgid);
	}
}
/* }}} */

/* {{{ Return the translation of msgid for domain_name and category, or msgid unaltered if a translation does not exist */
CRX_FUNCTION(dcgettext)
{
	char *msgstr;
	crex_string *domain, *msgid;
	crex_long category;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "SSl", &domain, &msgid, &category) == FAILURE) {
		RETURN_THROWS();
	}

	CRX_GETTEXT_DOMAIN_LENGTH_CHECK(1, ZSTR_LEN(domain))
	CRX_GETTEXT_LENGTH_CHECK(2, ZSTR_LEN(msgid))

	msgstr = dcgettext(ZSTR_VAL(domain), ZSTR_VAL(msgid), category);

	if (msgstr != ZSTR_VAL(msgid)) {
		RETURN_STRING(msgstr);
	} else {
		RETURN_STR_COPY(msgid);
	}
}
/* }}} */

/* {{{ Bind to the text domain domain_name, looking for translations in dir. Returns the current domain */
CRX_FUNCTION(bindtextdomain)
{
	char *domain;
	size_t domain_len;
	crex_string *dir = NULL;
	char *retval, dir_name[MAXPATHLEN];

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sS!", &domain, &domain_len, &dir) == FAILURE) {
		RETURN_THROWS();
	}

	CRX_GETTEXT_DOMAIN_LENGTH_CHECK(1, domain_len)

	if (domain[0] == '\0') {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	}

	if (dir == NULL) {
		RETURN_STRING(bindtextdomain(domain, NULL));
	}

	if (ZSTR_LEN(dir) != 0 && !crex_string_equals_literal(dir, "0")) {
		if (!VCWD_REALPATH(ZSTR_VAL(dir), dir_name)) {
			RETURN_FALSE;
		}
	} else if (!VCWD_GETCWD(dir_name, MAXPATHLEN)) {
		RETURN_FALSE;
	}

	retval = bindtextdomain(domain, dir_name);

	RETURN_STRING(retval);
}
/* }}} */

#ifdef HAVE_NGETTEXT
/* {{{ Plural version of gettext() */
CRX_FUNCTION(ngettext)
{
	char *msgid1, *msgid2, *msgstr;
	size_t msgid1_len, msgid2_len;
	crex_long count;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ssl", &msgid1, &msgid1_len, &msgid2, &msgid2_len, &count) == FAILURE) {
		RETURN_THROWS();
	}

	CRX_GETTEXT_LENGTH_CHECK(1, msgid1_len)
	CRX_GETTEXT_LENGTH_CHECK(2, msgid2_len)

	msgstr = ngettext(msgid1, msgid2, count);

	CREX_ASSERT(msgstr);
	RETURN_STRING(msgstr);
}
/* }}} */
#endif

#ifdef HAVE_DNGETTEXT
/* {{{ Plural version of dgettext() */
CRX_FUNCTION(dngettext)
{
	char *domain, *msgid1, *msgid2, *msgstr = NULL;
	size_t domain_len, msgid1_len, msgid2_len;
	crex_long count;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sssl", &domain, &domain_len,
		&msgid1, &msgid1_len, &msgid2, &msgid2_len, &count) == FAILURE) {
		RETURN_THROWS();
	}

	CRX_GETTEXT_DOMAIN_LENGTH_CHECK(1, domain_len)
	CRX_GETTEXT_LENGTH_CHECK(2, msgid1_len)
	CRX_GETTEXT_LENGTH_CHECK(3, msgid2_len)

	msgstr = dngettext(domain, msgid1, msgid2, count);

	CREX_ASSERT(msgstr);
	RETURN_STRING(msgstr);
}
/* }}} */
#endif

#ifdef HAVE_DCNGETTEXT
/* {{{ Plural version of dcgettext() */
CRX_FUNCTION(dcngettext)
{
	char *domain, *msgid1, *msgid2, *msgstr = NULL;
	size_t domain_len, msgid1_len, msgid2_len;
	crex_long count, category;

	RETVAL_FALSE;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sssll", &domain, &domain_len,
		&msgid1, &msgid1_len, &msgid2, &msgid2_len, &count, &category) == FAILURE) {
		RETURN_THROWS();
	}

	CRX_GETTEXT_DOMAIN_LENGTH_CHECK(1, domain_len)
	CRX_GETTEXT_LENGTH_CHECK(2, msgid1_len)
	CRX_GETTEXT_LENGTH_CHECK(3, msgid2_len)

	msgstr = dcngettext(domain, msgid1, msgid2, count, category);

	CREX_ASSERT(msgstr);
	RETURN_STRING(msgstr);
}
/* }}} */
#endif

#ifdef HAVE_BIND_TEXTDOMAIN_CODESET

/* {{{ Specify the character encoding in which the messages from the DOMAIN message catalog will be returned. */
CRX_FUNCTION(bind_textdomain_codeset)
{
	char *domain, *codeset = NULL, *retval = NULL;
	size_t domain_len, codeset_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss!", &domain, &domain_len, &codeset, &codeset_len) == FAILURE) {
		RETURN_THROWS();
	}

	CRX_GETTEXT_DOMAIN_LENGTH_CHECK(1, domain_len)

	retval = bind_textdomain_codeset(domain, codeset);

	if (!retval) {
		RETURN_FALSE;
	}
	RETURN_STRING(retval);
}
/* }}} */
#endif


#endif /* HAVE_LIBINTL */
