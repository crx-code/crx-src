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
   | Authors: Stanislav Malyshev <stas@crex.com>                          |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_BUILD_H
#define CREX_BUILD_H

#define CREX_TOSTR_(x) #x
#define CREX_TOSTR(x) CREX_TOSTR_(x)

#ifdef ZTS
#define CREX_BUILD_TS ",TS"
#else
#define CREX_BUILD_TS ",NTS"
#endif

#if CREX_DEBUG
#define CREX_BUILD_DEBUG ",debug"
#else
#define CREX_BUILD_DEBUG
#endif

#if defined(CREX_WIN32) && defined(CRX_COMPILER_ID)
#define CREX_BUILD_SYSTEM "," CRX_COMPILER_ID
#else
#define CREX_BUILD_SYSTEM
#endif

/* for private applications */
#define CREX_BUILD_EXTRA

#endif
