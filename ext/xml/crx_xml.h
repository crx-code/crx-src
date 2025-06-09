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
   |          Sterling Hughes <sterling@crx.net>                          |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_XML_H
#define CRX_XML_H

#ifdef HAVE_XML

extern crex_module_entry xml_module_entry;
#define xml_module_ptr &xml_module_entry

#include "crx_version.h"
#define CRX_XML_VERSION CRX_VERSION

#include "expat_compat.h"

#ifdef XML_UNICODE
#error "UTF-16 Unicode support not implemented!"
#endif

#else
#define xml_module_ptr NULL
#endif /* HAVE_XML */

#define crxext_xml_ptr xml_module_ptr

enum crx_xml_option {
	CRX_XML_OPTION_CASE_FOLDING = 1,
	CRX_XML_OPTION_TARGET_ENCODING,
	CRX_XML_OPTION_SKIP_TAGSTART,
	CRX_XML_OPTION_SKIP_WHITE
};

#ifdef LIBXML_EXPAT_COMPAT
#define CRX_XML_SAX_IMPL "libxml"
#else
#define CRX_XML_SAX_IMPL "expat"
#endif

#endif /* CRX_XML_H */
