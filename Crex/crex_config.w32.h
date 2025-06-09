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
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_CONFIG_W32_H
#define CREX_CONFIG_W32_H

#include <../main/config.w32.h>

#define _CRTDBG_MAP_ALLOC

#include <malloc.h>
#include <stdlib.h>
#include <crtdbg.h>

#include <string.h>

#ifndef CREX_INCLUDE_FULL_WINDOWS_HEADERS
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>

#include <float.h>

#define HAVE_STDIOSTR_H 1
#define HAVE_CLASS_ISTDIOSTREAM
#define istdiostream stdiostream

#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
#define strcasecmp(s1, s2) _stricmp(s1, s2)
#define strncasecmp(s1, s2, n) _strnicmp(s1, s2, n)

#ifdef LIBCREX_EXPORTS
#	define CREX_API __declspec(dllexport)
#else
#	define CREX_API __declspec(dllimport)
#endif

#define CREX_DLEXPORT		__declspec(dllexport)
#define CREX_DLIMPORT		__declspec(dllimport)

#endif /* CREX_CONFIG_W32_H */
