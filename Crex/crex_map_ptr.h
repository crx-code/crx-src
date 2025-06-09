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
   | Authors: Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_MAP_PTR_H
#define CREX_MAP_PTR_H

#include "crex_portability.h"

#define CREX_MAP_PTR_KIND_PTR           0
#define CREX_MAP_PTR_KIND_PTR_OR_OFFSET 1

#define CREX_MAP_PTR_KIND CREX_MAP_PTR_KIND_PTR_OR_OFFSET

#define CREX_MAP_PTR(ptr) \
	ptr ## __ptr
#define CREX_MAP_PTR_DEF(type, name) \
	type CREX_MAP_PTR(name)
#define CREX_MAP_PTR_OFFSET2PTR(offset) \
	((void**)((char*)CG(map_ptr_base) + offset))
#define CREX_MAP_PTR_PTR2OFFSET(ptr) \
	((void*)(((char*)(ptr)) - ((char*)CG(map_ptr_base))))
#define CREX_MAP_PTR_INIT(ptr, val) do { \
		CREX_MAP_PTR(ptr) = (val); \
	} while (0)
#define CREX_MAP_PTR_NEW(ptr) do { \
		CREX_MAP_PTR(ptr) = crex_map_ptr_new(); \
	} while (0)

#if CREX_MAP_PTR_KIND == CREX_MAP_PTR_KIND_PTR_OR_OFFSET
# define CREX_MAP_PTR_NEW_OFFSET() \
	((uint32_t)(uintptr_t)crex_map_ptr_new())
# define CREX_MAP_PTR_IS_OFFSET(ptr) \
	(((uintptr_t)CREX_MAP_PTR(ptr)) & 1L)
# define CREX_MAP_PTR_GET(ptr) \
	((CREX_MAP_PTR_IS_OFFSET(ptr) ? \
		CREX_MAP_PTR_GET_IMM(ptr) : \
		((void*)(CREX_MAP_PTR(ptr)))))
# define CREX_MAP_PTR_GET_IMM(ptr) \
	(*CREX_MAP_PTR_OFFSET2PTR((uintptr_t)CREX_MAP_PTR(ptr)))
# define CREX_MAP_PTR_SET(ptr, val) do { \
		if (CREX_MAP_PTR_IS_OFFSET(ptr)) { \
			CREX_MAP_PTR_SET_IMM(ptr, val); \
		} else { \
			CREX_MAP_PTR_INIT(ptr, val); \
		} \
	} while (0)
# define CREX_MAP_PTR_SET_IMM(ptr, val) do { \
		void **__p = CREX_MAP_PTR_OFFSET2PTR((uintptr_t)CREX_MAP_PTR(ptr)); \
		*__p = (val); \
	} while (0)
# define CREX_MAP_PTR_BIASED_BASE(real_base) \
	((void*)(((uintptr_t)(real_base)) - 1))
#else
# error "Unknown CREX_MAP_PTR_KIND"
#endif

BEGIN_EXTERN_C()

CREX_API void  crex_map_ptr_reset(void);
CREX_API void *crex_map_ptr_new(void);
CREX_API void  crex_map_ptr_extend(size_t last);
CREX_API void crex_alloc_ce_cache(crex_string *type_name);

END_EXTERN_C()

#endif /* CREX_MAP_PTR_H */
