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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "crx.h"
#include "crx_ini.h"
#include "ext/standard/info.h"
#include "crex_interfaces.h"
#include "crex_exceptions.h"
#include "spl_exceptions_arginfo.h"

#include "crx_spl.h"
#include "spl_functions.h"
#include "spl_engine.h"
#include "spl_exceptions.h"

CRXAPI crex_class_entry *spl_ce_LogicException;
CRXAPI crex_class_entry *spl_ce_BadFunctionCallException;
CRXAPI crex_class_entry *spl_ce_BadMethodCallException;
CRXAPI crex_class_entry *spl_ce_DomainException;
CRXAPI crex_class_entry *spl_ce_InvalidArgumentException;
CRXAPI crex_class_entry *spl_ce_LengthException;
CRXAPI crex_class_entry *spl_ce_OutOfRangeException;
CRXAPI crex_class_entry *spl_ce_RuntimeException;
CRXAPI crex_class_entry *spl_ce_OutOfBoundsException;
CRXAPI crex_class_entry *spl_ce_OverflowException;
CRXAPI crex_class_entry *spl_ce_RangeException;
CRXAPI crex_class_entry *spl_ce_UnderflowException;
CRXAPI crex_class_entry *spl_ce_UnexpectedValueException;

#define spl_ce_Exception crex_ce_exception

/* {{{ CRX_MINIT_FUNCTION(spl_exceptions) */
CRX_MINIT_FUNCTION(spl_exceptions)
{
	spl_ce_LogicException = register_class_LogicException(crex_ce_exception);
	spl_ce_BadFunctionCallException = register_class_BadFunctionCallException(spl_ce_LogicException);
	spl_ce_BadMethodCallException = register_class_BadMethodCallException(spl_ce_BadFunctionCallException);
	spl_ce_DomainException = register_class_DomainException(spl_ce_LogicException);
	spl_ce_InvalidArgumentException = register_class_InvalidArgumentException(spl_ce_LogicException);
	spl_ce_LengthException = register_class_LengthException(spl_ce_LogicException);
	spl_ce_OutOfRangeException = register_class_OutOfRangeException(spl_ce_LogicException);

	spl_ce_RuntimeException = register_class_RuntimeException(crex_ce_exception);
	spl_ce_OutOfBoundsException = register_class_OutOfBoundsException(spl_ce_RuntimeException);
	spl_ce_OverflowException = register_class_OverflowException(spl_ce_RuntimeException);
	spl_ce_RangeException = register_class_RangeException(spl_ce_RuntimeException);
	spl_ce_UnderflowException = register_class_UnderflowException(spl_ce_RuntimeException);
	spl_ce_UnexpectedValueException = register_class_UnexpectedValueException(spl_ce_RuntimeException);

	return SUCCESS;
}
/* }}} */
