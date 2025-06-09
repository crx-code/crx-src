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
   | Author:                                                              |
   +----------------------------------------------------------------------+
*/

#include "crx.h"
#include "SAPI.h"
#include "rfc1867.h"

#include "crx_content_types.h"

/* {{{ crx_post_entries[] */
static const sapi_post_entry crx_post_entries[] = {
	{ DEFAULT_POST_CONTENT_TYPE, sizeof(DEFAULT_POST_CONTENT_TYPE)-1, sapi_read_standard_form_data,	crx_std_post_handler },
	{ MULTIPART_CONTENT_TYPE,    sizeof(MULTIPART_CONTENT_TYPE)-1,    NULL,                         rfc1867_post_handler },
	{ NULL, 0, NULL, NULL }
};
/* }}} */

/* {{{ SAPI_POST_READER_FUNC */
SAPI_API SAPI_POST_READER_FUNC(crx_default_post_reader)
{
	if (!strcmp(SG(request_info).request_method, "POST")) {
		if (NULL == SG(request_info).post_entry) {
			/* no post handler registered, so we just swallow the data */
			sapi_read_standard_form_data();
		}
	}
}
/* }}} */

/* {{{ crx_startup_sapi_content_types */
int crx_startup_sapi_content_types(void)
{
	sapi_register_default_post_reader(crx_default_post_reader);
	sapi_register_treat_data(crx_default_treat_data);
	sapi_register_input_filter(crx_default_input_filter, NULL);
	return SUCCESS;
}
/* }}} */

/* {{{ crx_setup_sapi_content_types */
int crx_setup_sapi_content_types(void)
{
	sapi_register_post_entries(crx_post_entries);

	return SUCCESS;
}
/* }}} */
