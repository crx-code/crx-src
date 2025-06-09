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
   | Authors: Amitay Isaacs <amitay@w-o-i.com>                            |
   |          Eric Warnke   <ericw@albany.edu>                            |
   |          Jani Taskinen <sniper@iki.fi>                               |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_LDAP_H
#define CRX_LDAP_H

#ifndef HAVE_ORALDAP
#include <lber.h>
#endif

#include <ldap.h>

extern crex_module_entry ldap_module_entry;
#define ldap_module_ptr &ldap_module_entry

#include "crx_version.h"
#define CRX_LDAP_VERSION CRX_VERSION

/* LDAP functions */
CRX_MINIT_FUNCTION(ldap);
CRX_MSHUTDOWN_FUNCTION(ldap);
CRX_MINFO_FUNCTION(ldap);

CREX_BEGIN_MODULE_GLOBALS(ldap)
	crex_long num_links;
	crex_long max_links;
CREX_END_MODULE_GLOBALS(ldap)

#if defined(ZTS) && defined(COMPILE_DL_LDAP)
CREX_TSRMLS_CACHE_EXTERN()
#endif

CREX_EXTERN_MODULE_GLOBALS(ldap)
#define LDAPG(v) CREX_MODULE_GLOBALS_ACCESSOR(ldap, v)

#define crxext_ldap_ptr ldap_module_ptr

/* Constants for ldap_modify_batch */
#define LDAP_MODIFY_BATCH_ADD        0x01
#define LDAP_MODIFY_BATCH_REMOVE     0x02
#define LDAP_MODIFY_BATCH_REMOVE_ALL 0x12
#define LDAP_MODIFY_BATCH_REPLACE    0x03

#define LDAP_MODIFY_BATCH_ATTRIB     "attrib"
#define LDAP_MODIFY_BATCH_MODTYPE    "modtype"
#define LDAP_MODIFY_BATCH_VALUES     "values"

#endif /* CRX_LDAP_H */
