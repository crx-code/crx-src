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
   | Authors: Stig Sæther Bakken <ssb@crx.net>                            |
   |          Thies C. Arntzen <thies@thieso.net>                         |
   |                                                                      |
   | Collection support by Andy Sautins <asautins@veripost.net>           |
   | Temporary LOB support by David Benson <dbenson@mancala.com>          |
   | ZTS per process OCIPLogon by Harald Radi <harald.radi@nme.at>        |
   |                                                                      |
   | Redesigned by: Antony Dovgal <antony@crex.com>                       |
   |                Andi Gutmans <andi@crx.net>                           |
   |                Wez Furlong <wez@omniti.com>                          |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_OCI8
# ifndef CRX_OCI8_H
#  define CRX_OCI8_H

#ifdef ZTS
# include "TSRM.h"
#endif

/*
 * The version of the OCI8 extension.
 */
#ifdef CRX_OCI8_VERSION
/* The definition of CRX_OCI8_VERSION changed in CRX 5.3 and building
 * this code with CRX 5.2 (e.g. when using OCI8 from PECL) will conflict.
 */
#undef CRX_OCI8_VERSION
#endif
#define CRX_OCI8_VERSION "3.3.0"

extern crex_module_entry oci8_module_entry;
#define crxext_oci8_ptr &oci8_module_entry
#define crxext_oci8_11g_ptr &oci8_module_entry
#define crxext_oci8_12c_ptr &oci8_module_entry
#define crxext_oci8_19_ptr &oci8_module_entry


CRX_MINIT_FUNCTION(oci);
CRX_RINIT_FUNCTION(oci);
CRX_MSHUTDOWN_FUNCTION(oci);
CRX_RSHUTDOWN_FUNCTION(oci);
CRX_MINFO_FUNCTION(oci);

# endif /* !CRX_OCI8_H */
#else /* !HAVE_OCI8 */

# define oci8_module_ptr NULL

#endif /* HAVE_OCI8 */
