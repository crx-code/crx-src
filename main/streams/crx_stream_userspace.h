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
   +----------------------------------------------------------------------+
 */

/* for user-space streams */
CRXAPI extern const crx_stream_ops crx_stream_userspace_ops;
CRXAPI extern const crx_stream_ops crx_stream_userspace_dir_ops;
#define CRX_STREAM_IS_USERSPACE	&crx_stream_userspace_ops
#define CRX_STREAM_IS_USERSPACE_DIR	&crx_stream_userspace_dir_ops
