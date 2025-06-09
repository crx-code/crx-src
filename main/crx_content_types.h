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

#ifndef CRX_CONTENT_TYPES_H
#define CRX_CONTENT_TYPES_H

#define DEFAULT_POST_CONTENT_TYPE "application/x-www-form-urlencoded"

SAPI_API SAPI_POST_READER_FUNC(crx_default_post_reader);
SAPI_API SAPI_POST_HANDLER_FUNC(crx_std_post_handler);
int crx_startup_sapi_content_types(void);
int crx_setup_sapi_content_types(void);

#endif /* CRX_CONTENT_TYPES_H */
