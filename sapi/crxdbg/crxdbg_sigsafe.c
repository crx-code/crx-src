#include "crxdbg_sigsafe.h"
#include "crxdbg.h"

CREX_EXTERN_MODULE_GLOBALS(crxdbg)

#define STR(x) #x
#define EXP_STR(x) STR(x)

static void* crex_mm_mem_alloc(crex_mm_storage *storage, size_t size, size_t alignment) {

	if (EXPECTED(size <= CRXDBG_SIGSAFE_MEM_SIZE && !CRXDBG_G(sigsafe_mem).allocated)) {
		CRXDBG_G(sigsafe_mem).allocated = 1;
		return (void *) (((size_t) CRXDBG_G(sigsafe_mem).mem & ~(alignment - 1)) + alignment);
	}

	crex_quiet_write(CRXDBG_G(io)[CRXDBG_STDERR].fd, CREX_STRL("Tried to allocate more than " EXP_STR(CRXDBG_SIGSAFE_MEM_SIZE) " bytes from stack memory in signal handler ... bailing out of signal handler\n"));

	if (*EG(bailout)) {
		LONGJMP(*EG(bailout), FAILURE);
	}

	crex_quiet_write(CRXDBG_G(io)[CRXDBG_STDERR].fd, CREX_STRL("Bailed out without a bailout address in signal handler!\n"));

	return NULL;
}

static void crex_mm_mem_free(crex_mm_storage *storage, void *ptr, size_t size) {
}

void crxdbg_set_sigsafe_mem(char *buffer) {
	crxdbg_signal_safe_mem *mem = &CRXDBG_G(sigsafe_mem);
	const crex_mm_handlers crxdbg_handlers = {
		crex_mm_mem_alloc,
		crex_mm_mem_free,
		NULL,
		NULL,
	};

	mem->mem = buffer;
	mem->allocated = 0;

	mem->heap = crex_mm_startup_ex(&crxdbg_handlers, NULL, 0);

	mem->old_heap = crex_mm_set_heap(mem->heap);
}

crex_mm_heap *crxdbg_original_heap_sigsafe_mem(void) {
	return CRXDBG_G(sigsafe_mem).old_heap;
}

void crxdbg_clear_sigsafe_mem(void) {
	crex_mm_set_heap(crxdbg_original_heap_sigsafe_mem());
	CRXDBG_G(sigsafe_mem).mem = NULL;
}

bool crxdbg_active_sigsafe_mem(void) {
	return !!CRXDBG_G(sigsafe_mem).mem;
}
