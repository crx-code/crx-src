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

#include "basic_functions.h"
#include "crx_math.h"
#include "crx_string.h"
#include "base64.h"
#include "crx_dir.h"
#include "crx_dns.h"
#include "crx_mail.h"
#include "md5.h"
#include "sha1.h"
#include "html.h"
#include "exec.h"
#include "file.h"
#include "crx_ext_syslog.h"
#include "crx_filestat.h"
#include "crx_browscap.h"
#include "pack.h"
#include "datetime.h"
#include "url.h"
#include "pageinfo.h"
#include "fsock.h"
#include "crx_image.h"
#include "info.h"
#include "crx_var.h"
#include "quot_print.h"
#include "dl.h"
#include "crx_crypt.h"
#include "head.h"
#include "crx_output.h"
#include "crx_array.h"
#include "crx_assert.h"
#include "crx_versioning.h"
#include "crx_password.h"

#include "crx_version.h"
#define CRX_STANDARD_VERSION CRX_VERSION

#define crxext_standard_ptr basic_functions_module_ptr
CRX_MINIT_FUNCTION(standard_filters);
CRX_MSHUTDOWN_FUNCTION(standard_filters);
