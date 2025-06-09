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
   | Authors: Dmitry Stogov <dmitry@crex.com>                             |
   |          Xinchen Hui <xinchen.h@crex.com>                            |
   +----------------------------------------------------------------------+
*/

#include "crex.h"
#include "crex_gdb.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#if defined(__FreeBSD__) && __FreeBSD_version >= 1100000
# include <sys/user.h>
# include <libutil.h>
#endif

enum {
	CREX_GDBJIT_NOACTION,
	CREX_GDBJIT_REGISTER,
	CREX_GDBJIT_UNREGISTER
};

typedef struct _crex_gdbjit_code_entry {
	struct _crex_gdbjit_code_entry *next_entry;
	struct _crex_gdbjit_code_entry *prev_entry;
	const char                     *symfile_addr;
	uint64_t                        symfile_size;
} crex_gdbjit_code_entry;

typedef struct _crex_gdbjit_descriptor {
	uint32_t                         version;
	uint32_t                         action_flag;
	struct _crex_gdbjit_code_entry *relevant_entry;
	struct _crex_gdbjit_code_entry *first_entry;
} crex_gdbjit_descriptor;

CREX_API crex_gdbjit_descriptor __jit_debug_descriptor = {
	1, CREX_GDBJIT_NOACTION, NULL, NULL
};

CREX_API crex_never_inline void __jit_debug_register_code(void)
{
	__asm__ __volatile__("");
}

CREX_API bool crex_gdb_register_code(const void *object, size_t size)
{
	crex_gdbjit_code_entry *entry;

	entry = malloc(sizeof(crex_gdbjit_code_entry) + size);
	if (entry == NULL) {
		return 0;
	}

	entry->symfile_addr = ((char*)entry) + sizeof(crex_gdbjit_code_entry);
	entry->symfile_size = size;

	memcpy((char *)entry->symfile_addr, object, size);

	entry->prev_entry = NULL;
	entry->next_entry = __jit_debug_descriptor.first_entry;

	if (entry->next_entry) {
		entry->next_entry->prev_entry = entry;
	}
	__jit_debug_descriptor.first_entry = entry;

	/* Notify GDB */
	__jit_debug_descriptor.relevant_entry = entry;
	__jit_debug_descriptor.action_flag = CREX_GDBJIT_REGISTER;
	__jit_debug_register_code();

	return 1;
}

CREX_API void crex_gdb_unregister_all(void)
{
	crex_gdbjit_code_entry *entry;

	__jit_debug_descriptor.action_flag = CREX_GDBJIT_UNREGISTER;
	while ((entry = __jit_debug_descriptor.first_entry)) {
		__jit_debug_descriptor.first_entry = entry->next_entry;
		if (entry->next_entry) {
			entry->next_entry->prev_entry = NULL;
		}
		/* Notify GDB */
		__jit_debug_descriptor.relevant_entry = entry;
		__jit_debug_register_code();

		free(entry);
	}
}

CREX_API bool crex_gdb_present(void)
{
	bool ret = 0;
#if defined(__linux__) /* netbsd while having this procfs part, does not hold the tracer pid */
	int fd = open("/proc/self/status", O_RDONLY);

	if (fd >= 0) {
		char buf[1024];
		ssize_t n = read(fd, buf, sizeof(buf) - 1);
		char *s;
		pid_t pid;

		if (n > 0) {
			buf[n] = 0;
			s = strstr(buf, "TracerPid:");
			if (s) {
				s += sizeof("TracerPid:") - 1;
				while (*s == ' ' || *s == '\t') {
					s++;
				}
				pid = atoi(s);
				if (pid) {
					char out[1024];
					sprintf(buf, "/proc/%d/exe", (int)pid);
					if (readlink(buf, out, sizeof(out) - 1) > 0) {
						if (strstr(out, "gdb")) {
							ret = 1;
						}
					}
				}
			}
		}

		close(fd);
	}
#elif defined(__FreeBSD__) && __FreeBSD_version >= 1100000
    struct kinfo_proc *proc = kinfo_getproc(getpid());

    if (proc) {
        if ((proc->ki_flag & P_TRACED) != 0) {
            struct kinfo_proc *dbg = kinfo_getproc(proc->ki_tracer);

            ret = (dbg && strstr(dbg->ki_comm, "gdb"));
        }
    }
#endif

	return ret;
}
