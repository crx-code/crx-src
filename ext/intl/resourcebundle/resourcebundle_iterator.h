/*
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the CRX license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | https://www.crx.net/license/3_01.txt                                 |
   | If you did not receive a copy of the CRX license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crx.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Hans-Peter Oeri (University of St.Gallen) <hp@oeri.ch>      |
   +----------------------------------------------------------------------+
 */

#ifndef RESOURCEBUNDLE_ITERATOR_H
#define RESOURCEBUNDLE_ITERATOR_H

#include <crex.h>

#include "resourcebundle/resourcebundle_class.h"

typedef struct {
	crex_object_iterator  intern;
	ResourceBundle_object *subject;
	bool             is_table;
	crex_long             length;
	zval                  current;
	char                  *currentkey;
	crex_long             i;
} ResourceBundle_iterator;

crex_object_iterator *resourcebundle_get_iterator( crex_class_entry *ce, zval *object, int byref );

#endif // #ifndef RESOURCEBUNDLE_ITERATOR_H
