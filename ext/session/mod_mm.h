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
   | Author: Sascha Schumann <sascha@schumann.cx>                         |
   +----------------------------------------------------------------------+
 */

#ifndef MOD_MM_H
#define MOD_MM_H

#ifdef HAVE_LIBMM

#include "crx_session.h"

CRX_MINIT_FUNCTION(ps_mm);
CRX_MSHUTDOWN_FUNCTION(ps_mm);

extern const ps_module ps_mod_mm;
#define ps_mm_ptr &ps_mod_mm

PS_FUNCS(mm);

#endif
#endif
