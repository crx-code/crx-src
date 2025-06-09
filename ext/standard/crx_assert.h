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
   | Author: Thies C. Arntzen <thies@thieso.net>                          |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_ASSERT_H
#define CRX_ASSERT_H

CRX_MINIT_FUNCTION(assert);
CRX_MSHUTDOWN_FUNCTION(assert);
CRX_RINIT_FUNCTION(assert);
CRX_RSHUTDOWN_FUNCTION(assert);
CRX_MINFO_FUNCTION(assert);

enum {
	CRX_ASSERT_ACTIVE=1,
	CRX_ASSERT_CALLBACK,
	CRX_ASSERT_BAIL,
	CRX_ASSERT_WARNING,
	CRX_ASSERT_EXCEPTION
};

extern CRXAPI crex_class_entry *assertion_error_ce;

#endif /* CRX_ASSERT_H */
