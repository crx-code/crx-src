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
   | Author: Wez Furlong <wez@thebrainroom.com>                           |
   |         Harald Radi <h.radi@nme.at>                                  |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "crx_ini.h"
#include "ext/standard/info.h"
#include "crx_com_dotnet.h"
#include "crx_com_dotnet_internal.h"


CRX_COM_DOTNET_API OLECHAR *crx_com_string_to_olestring(const char *string, size_t string_len, int codepage)
{
	OLECHAR *olestring = NULL;
	DWORD flags = codepage == CP_UTF8 ? 0 : MB_PRECOMPOSED | MB_ERR_INVALID_CHARS;
	BOOL ok;

	if (string_len == -1) {
		/* determine required length for the buffer (includes NUL terminator) */
		string_len = MultiByteToWideChar(codepage, flags, string, -1, NULL, 0);
	} else {
		/* allow room for NUL terminator */
		string_len++;
	}

	if (string_len > 0) {
		olestring = (OLECHAR*)safe_emalloc(string_len, sizeof(OLECHAR), 0);
		/* XXX if that's a real multibyte string, olestring is obviously allocated excessively.
		This should be fixed by reallocating the olestring, but as emalloc is used, that doesn't
		matter much. */
		ok = MultiByteToWideChar(codepage, flags, string, (int)string_len, olestring, (int)string_len);
		if (ok > 0 && (size_t)ok < string_len) {
			olestring[ok] = '\0';
		}
	} else {
		ok = FALSE;
		olestring = (OLECHAR*)emalloc(sizeof(OLECHAR));
		*olestring = 0;
	}

	if (!ok) {
		char *msg = crx_win32_error_to_msg(GetLastError());

		crx_error_docref(NULL, E_WARNING,
			"Could not convert string to unicode: `%s'", msg);

		crx_win32_error_msg_free(msg);
	}

	return olestring;
}

CRX_COM_DOTNET_API crex_string *crx_com_olestring_to_string(OLECHAR *olestring, int codepage)
{
	crex_string *string;
	uint32_t length = 0;

	length = WideCharToMultiByte(codepage, 0, olestring, -1, NULL, 0, NULL, NULL);

	if (length) {
		/* We remove 1 from the length as it takes into account the terminating null byte
		 * which crex_string alloc already takes into consideration */
		/* TODO Should use safe alloc? */
		string = crex_string_alloc(length - 1, /* persistent */ false);
		length = WideCharToMultiByte(codepage, 0, olestring, -1, ZSTR_VAL(string), length, NULL, NULL);
	} else {
		string = ZSTR_EMPTY_ALLOC();
	}

	/* Failure to determine length of WideChar */
	if (length == 0) {
		char *msg = crx_win32_error_to_msg(GetLastError());

		crx_error_docref(NULL, E_WARNING,
			"Could not convert string from unicode: `%s'", msg);

		crx_win32_error_msg_free(msg);
	}

	return string;
}

BSTR crx_com_string_to_bstr(crex_string *string, int codepage)
{
	BSTR bstr = NULL;
	DWORD flags = codepage == CP_UTF8 ? 0 : MB_PRECOMPOSED | MB_ERR_INVALID_CHARS;
	size_t mb_len = ZSTR_LEN(string);
	int wc_len;

	if ((wc_len = MultiByteToWideChar(codepage, flags, ZSTR_VAL(string), (int)mb_len + 1, NULL, 0)) <= 0) {
		goto fail;
	}
	if ((bstr = SysAllocStringLen(NULL, (UINT)(wc_len - 1))) == NULL) {
		goto fail;
	}
	if ((wc_len = MultiByteToWideChar(codepage, flags, ZSTR_VAL(string), (int)mb_len + 1, bstr, wc_len)) <= 0) {
		goto fail;
	}
	return bstr;

fail:
	char *msg = crx_win32_error_to_msg(GetLastError());
	crx_error_docref(NULL, E_WARNING,
		"Could not convert string to unicode: `%s'", msg);
	LocalFree(msg);
	SysFreeString(bstr);
	return SysAllocString(L"");
}

crex_string *crx_com_bstr_to_string(BSTR bstr, int codepage)
{
	crex_string *string = NULL;
	UINT wc_len = SysStringLen(bstr);
	int mb_len;

	mb_len = WideCharToMultiByte(codepage, 0, bstr, wc_len + 1, NULL, 0, NULL, NULL);
	if (mb_len > 0) {
		string = crex_string_alloc(mb_len - 1, 0);
		mb_len = WideCharToMultiByte(codepage, 0, bstr, wc_len + 1, ZSTR_VAL(string), mb_len, NULL, NULL);
	}

	if (mb_len <= 0) {
		char *msg = crx_win32_error_to_msg(GetLastError());

		crx_error_docref(NULL, E_WARNING,
			"Could not convert string from unicode: `%s'", msg);
		LocalFree(msg);

		if (string != NULL) {
			crex_string_release(string);
		}
		string = ZSTR_EMPTY_ALLOC();
	}

	return string;
}
