/*
   +----------------------------------------------------------------------+
   | Crex Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Crex Technologies Ltd. (http://www.crex.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Sterling Hughes <sterling@crx.net>                          |
   |          Marcus Boerger <helly@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#include "crex.h"
#include "crex_API.h"
#include "crex_attributes.h"
#include "crex_builtin_functions.h"
#include "crex_interfaces.h"
#include "crex_exceptions.h"
#include "crex_closures.h"
#include "crex_generators.h"
#include "crex_weakrefs.h"
#include "crex_enum.h"
#include "crex_fibers.h"

CREX_API void crex_register_default_classes(void)
{
	crex_register_interfaces();
	crex_register_default_exception();
	crex_register_iterator_wrapper();
	crex_register_closure_ce();
	crex_register_generator_ce();
	crex_register_weakref_ce();
	crex_register_attribute_ce();
	crex_register_enum_ce();
	crex_register_fiber_ce();
}
