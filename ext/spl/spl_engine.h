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

#ifndef SPL_ENGINE_H
#define SPL_ENGINE_H

#include "crx.h"
#include "crx_spl.h"
#include "crex_interfaces.h"

static inline void spl_instantiate_arg_ex1(crex_class_entry *pce, zval *retval, zval *arg1)
{
	object_init_ex(retval, pce);
	crex_call_known_instance_method_with_1_params(pce->constructor, C_OBJ_P(retval), NULL, arg1);
}

static inline void spl_instantiate_arg_ex2(
		crex_class_entry *pce, zval *retval, zval *arg1, zval *arg2)
{
	object_init_ex(retval, pce);
	crex_call_known_instance_method_with_2_params(
		pce->constructor, C_OBJ_P(retval), NULL, arg1, arg2);
}

static inline void spl_instantiate_arg_n(
		crex_class_entry *pce, zval *retval, uint32_t argc, zval *argv)
{
	object_init_ex(retval, pce);
	crex_call_known_instance_method(pce->constructor, C_OBJ_P(retval), NULL, argc, argv);
}

#endif /* SPL_ENGINE_H */
