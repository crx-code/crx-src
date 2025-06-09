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
   | Authors: Rasmus Lerdorf <rasmus@crx.net>                             |
   |          Zeev Suraski <zeev@crx.net>                                 |
   +----------------------------------------------------------------------+
*/

#ifndef CREDITS_H
#define CREDITS_H

#ifndef HAVE_CREDITS_DEFS
#define HAVE_CREDITS_DEFS

#define CRX_CREDITS_GROUP			(1<<0)
#define CRX_CREDITS_GENERAL			(1<<1)
#define CRX_CREDITS_SAPI			(1<<2)
#define CRX_CREDITS_MODULES			(1<<3)
#define CRX_CREDITS_DOCS			(1<<4)
#define CRX_CREDITS_FULLPAGE		(1<<5)
#define CRX_CREDITS_QA				(1<<6)
#define CRX_CREDITS_WEB				(1<<7)
#define CRX_CREDITS_ALL				0xFFFFFFFF

#endif /* HAVE_CREDITS_DEFS */

CRXAPI void crx_print_credits(int flag);

#endif
