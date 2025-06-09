#ifndef CRXDBG_SIGSAFE_H
#define CRXDBG_SIGSAFE_H

#define CRXDBG_SIGSAFE_MEM_SIZE (CREX_MM_CHUNK_SIZE * 2)

#include "crex.h"

typedef struct {
	char *mem;
	bool allocated;
	crex_mm_heap *heap;
	crex_mm_heap *old_heap;
} crxdbg_signal_safe_mem;

#include "crxdbg.h"

bool crxdbg_active_sigsafe_mem(void);

void crxdbg_set_sigsafe_mem(char *mem);
void crxdbg_clear_sigsafe_mem(void);

crex_mm_heap *crxdbg_original_heap_sigsafe_mem(void);

#endif
