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
   | Authors: Max Kellermann <max.kellermann@ionos.com>                   |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_INI_BUILDER_H
#define CRX_INI_BUILDER_H

#include "crx.h"

/**
 * A class which helps with constructing INI entries from the command
 * line.
 */
struct crx_ini_builder {
	char *value;
	size_t length;
};

BEGIN_EXTERN_C()

static inline void crx_ini_builder_init(struct crx_ini_builder *b)
{
	b->value = NULL;
	b->length = 0;
}

static inline void crx_ini_builder_deinit(struct crx_ini_builder *b)
{
	free(b->value);
}

/**
 * Null-terminate the buffer and return it.
 */
static inline char *crx_ini_builder_finish(struct crx_ini_builder *b)
{
	if (b->value != NULL) {
		/* null-terminate the string */
		b->value[b->length] = '\0';
	}

	return b->value;
}

/**
 * Make room for more data.
 *
 * @param delta the number of bytes to be appended
 */
static inline void crx_ini_builder_realloc(struct crx_ini_builder *b, size_t delta)
{
	/* reserve enough space for the null terminator */
	b->value = realloc(b->value, b->length + delta + 1);
}

/**
 * Prepend a string.
 *
 * @param src the source string
 * @param length the size of the source string
 */
CRXAPI void crx_ini_builder_prepend(struct crx_ini_builder *b, const char *src, size_t length);

#define crx_ini_builder_prepend_literal(b, l) crx_ini_builder_prepend(b, l, strlen(l))

/**
 * Append an unquoted name/value pair.
 */
CRXAPI void crx_ini_builder_unquoted(struct crx_ini_builder *b, const char *name, size_t name_length, const char *value, size_t value_length);

/**
 * Append a quoted name/value pair.
 */
CRXAPI void crx_ini_builder_quoted(struct crx_ini_builder *b, const char *name, size_t name_length, const char *value, size_t value_length);

/**
 * Parse an INI entry from the command-line option "--define".
 */
CRXAPI void crx_ini_builder_define(struct crx_ini_builder *b, const char *arg);

END_EXTERN_C()

#endif
