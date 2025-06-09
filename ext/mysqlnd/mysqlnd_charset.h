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
  | Authors: Andrey Hristov <andrey@crx.net>                             |
  |          Ulf Wendel <uw@crx.net>                                     |
  |          Georg Richter <georg@crx.net>                               |
  +----------------------------------------------------------------------+
*/

#ifndef MYSQLND_CHARSET_H
#define MYSQLND_CHARSET_H

CRXAPI crex_ulong mysqlnd_cset_escape_quotes(const MYSQLND_CHARSET * const charset, char * newstr,
											 const char * escapestr, const size_t escapestr_len);

CRXAPI crex_ulong mysqlnd_cset_escape_slashes(const MYSQLND_CHARSET * const cset, char * newstr,
										 	  const char * escapestr, const size_t escapestr_len);

struct st_mysqlnd_plugin_charsets
{
	const struct st_mysqlnd_plugin_header plugin_header;
	struct
	{
		const MYSQLND_CHARSET * (*const find_charset_by_nr)(unsigned int charsetnr);
		const MYSQLND_CHARSET * (*const find_charset_by_name)(const char * const name);
		crex_ulong 			(*const escape_quotes)(const MYSQLND_CHARSET * const cset, char * newstr, const char * escapestr, const size_t escapestr_len);
		crex_ulong			(*const escape_slashes)(const MYSQLND_CHARSET * const cset, char * newstr, const char * escapestr, const size_t escapestr_len);
	} methods;
};

void mysqlnd_charsets_plugin_register(void);

#endif /* MYSQLND_CHARSET_H */
