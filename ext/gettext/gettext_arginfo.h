/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 864b3389d4f99b0d7302ae399544e6fb9fb80b7e */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_textdomain, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, domain, IS_STRING, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gettext, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, message, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo__ arginfo_gettext

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dgettext, 0, 2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, domain, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, message, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dcgettext, 0, 3, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, domain, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, message, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, category, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_bindtextdomain, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, domain, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, directory, IS_STRING, 1)
CREX_END_ARG_INFO()

#if defined(HAVE_NGETTEXT)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ngettext, 0, 3, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, singular, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, plural, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, count, IS_LONG, 0)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_DNGETTEXT)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dngettext, 0, 4, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, domain, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, singular, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, plural, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, count, IS_LONG, 0)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_DCNGETTEXT)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dcngettext, 0, 5, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, domain, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, singular, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, plural, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, count, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, category, IS_LONG, 0)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_BIND_TEXTDOMAIN_CODESET)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_bind_textdomain_codeset, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, domain, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, codeset, IS_STRING, 1)
CREX_END_ARG_INFO()
#endif


CREX_FUNCTION(textdomain);
CREX_FUNCTION(gettext);
CREX_FUNCTION(dgettext);
CREX_FUNCTION(dcgettext);
CREX_FUNCTION(bindtextdomain);
#if defined(HAVE_NGETTEXT)
CREX_FUNCTION(ngettext);
#endif
#if defined(HAVE_DNGETTEXT)
CREX_FUNCTION(dngettext);
#endif
#if defined(HAVE_DCNGETTEXT)
CREX_FUNCTION(dcngettext);
#endif
#if defined(HAVE_BIND_TEXTDOMAIN_CODESET)
CREX_FUNCTION(bind_textdomain_codeset);
#endif


static const crex_function_entry ext_functions[] = {
	CREX_FE(textdomain, arginfo_textdomain)
	CREX_FE(gettext, arginfo_gettext)
	CREX_FALIAS(_, gettext, arginfo__)
	CREX_FE(dgettext, arginfo_dgettext)
	CREX_FE(dcgettext, arginfo_dcgettext)
	CREX_FE(bindtextdomain, arginfo_bindtextdomain)
#if defined(HAVE_NGETTEXT)
	CREX_FE(ngettext, arginfo_ngettext)
#endif
#if defined(HAVE_DNGETTEXT)
	CREX_FE(dngettext, arginfo_dngettext)
#endif
#if defined(HAVE_DCNGETTEXT)
	CREX_FE(dcngettext, arginfo_dcngettext)
#endif
#if defined(HAVE_BIND_TEXTDOMAIN_CODESET)
	CREX_FE(bind_textdomain_codeset, arginfo_bind_textdomain_codeset)
#endif
	CREX_FE_END
};
