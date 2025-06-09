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
  | Author: Keyur Govande (kgovande@gmail.com)                           |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "crx_cli_process_title.h"
#include "ps_title.h"

/* {{{ Return a boolean to confirm if the process title was successfully changed or not */
CRX_FUNCTION(cli_set_process_title)
{
	char *title = NULL;
	size_t title_len;
	int rc;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s", &title, &title_len) == FAILURE) {
		RETURN_THROWS();
	}

	rc = set_ps_title(title);
	if (rc == PS_TITLE_SUCCESS) {
		RETURN_TRUE;
	}

	crx_error_docref(NULL, E_WARNING, "cli_set_process_title had an error: %s", ps_title_errno(rc));
	RETURN_FALSE;
}
/* }}} */

/* {{{ Return a string with the current process title. NULL if error. */
CRX_FUNCTION(cli_get_process_title)
{
	size_t length = 0;
	const char* title = NULL;
	int rc;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	rc = get_ps_title(&length, &title);
	if (rc != PS_TITLE_SUCCESS) {
			crx_error_docref(NULL, E_WARNING, "cli_get_process_title had an error: %s", ps_title_errno(rc));
			RETURN_NULL();
	}

	RETURN_STRINGL(title, length);
}
/* }}} */
