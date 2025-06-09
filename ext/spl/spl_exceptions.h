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
   | Authors: Marcus Boerger <helly@crx.net>                              |
   +----------------------------------------------------------------------+
 */

#ifndef SPL_EXCEPTIONS_H
#define SPL_EXCEPTIONS_H

#include "crx.h"
#include "crx_spl.h"

extern CRXAPI crex_class_entry *spl_ce_LogicException;
extern CRXAPI crex_class_entry *spl_ce_BadFunctionCallException;
extern CRXAPI crex_class_entry *spl_ce_BadMethodCallException;
extern CRXAPI crex_class_entry *spl_ce_DomainException;
extern CRXAPI crex_class_entry *spl_ce_InvalidArgumentException;
extern CRXAPI crex_class_entry *spl_ce_LengthException;
extern CRXAPI crex_class_entry *spl_ce_OutOfRangeException;

extern CRXAPI crex_class_entry *spl_ce_RuntimeException;
extern CRXAPI crex_class_entry *spl_ce_OutOfBoundsException;
extern CRXAPI crex_class_entry *spl_ce_OverflowException;
extern CRXAPI crex_class_entry *spl_ce_RangeException;
extern CRXAPI crex_class_entry *spl_ce_UnderflowException;
extern CRXAPI crex_class_entry *spl_ce_UnexpectedValueException;

CRX_MINIT_FUNCTION(spl_exceptions);

#endif /* SPL_EXCEPTIONS_H */
