CRX_ARG_ENABLE([opcache],
  [whether to enable Crex OPcache support],
  [AS_HELP_STRING([--disable-opcache],
    [Disable Crex OPcache support])],
  [yes])

CRX_ARG_ENABLE([huge-code-pages],
  [whether to enable copying CRX CODE pages into HUGE PAGES],
  [AS_HELP_STRING([--disable-huge-code-pages],
    [Disable copying CRX CODE pages into HUGE PAGES])],
  [yes],
  [no])

CRX_ARG_ENABLE([opcache-jit],
  [whether to enable JIT],
  [AS_HELP_STRING([--disable-opcache-jit],
    [Disable JIT])],
  [yes],
  [no])

CRX_ARG_WITH([capstone],,
  [AS_HELP_STRING([--with-capstone],
    [support opcache JIT disassembly through capstone])],
  [no],
  [no])

if test "$CRX_OPCACHE" != "no"; then

  dnl Always build as shared extension
  ext_shared=yes

  if test "$CRX_HUGE_CODE_PAGES" = "yes"; then
    AC_DEFINE(HAVE_HUGE_CODE_PAGES, 1, [Define to enable copying CRX CODE pages into HUGE PAGES (experimental)])
  fi

  if test "$CRX_OPCACHE_JIT" = "yes"; then
    case $host_cpu in
      i[[34567]]86*|x86*|aarch64)
        ;;
      *)
        AC_MSG_WARN([JIT not supported by host architecture])
        CRX_OPCACHE_JIT=no
        ;;
    esac
  fi

  if test "$CRX_OPCACHE_JIT" = "yes"; then
    AC_DEFINE(HAVE_JIT, 1, [Define to enable JIT])
    CREX_JIT_SRC="jit/crex_jit.c jit/crex_jit_gdb.c jit/crex_jit_vm_helpers.c"

    dnl Find out which ABI we are using.
    case $host_alias in
      x86_64-*-darwin*)
        DASM_FLAGS="-D X64APPLE=1 -D X64=1"
        DASM_ARCH="x86"
        ;;
      x86_64*)
        DASM_FLAGS="-D X64=1"
        DASM_ARCH="x86"
        ;;
      i[[34567]]86*)
        DASM_ARCH="x86"
        ;;
      x86*)
        DASM_ARCH="x86"
        ;;
      aarch64*)
        DASM_FLAGS="-D ARM64=1"
        DASM_ARCH="arm64"
        ;;
    esac

    if test "$CRX_THREAD_SAFETY" = "yes"; then
      DASM_FLAGS="$DASM_FLAGS -D ZTS=1"
    fi

    AS_IF([test x"$with_capstone" = "xyes"],[
      PKG_CHECK_MODULES([CAPSTONE],[capstone >= 3.0.0],[
        AC_DEFINE([HAVE_CAPSTONE], [1], [Capstone is available])
        CRX_EVAL_LIBLINE($CAPSTONE_LIBS, OPCACHE_SHARED_LIBADD)
        CRX_EVAL_INCLINE($CAPSTONE_CFLAGS)
      ],[
        AC_MSG_ERROR([capstone >= 3.0 required but not found])
      ])
    ])

    CRX_SUBST(DASM_FLAGS)
    CRX_SUBST(DASM_ARCH)
  fi

  AC_CHECK_FUNCS([mprotect memfd_create])

  AC_MSG_CHECKING(for sysvipc shared memory support)
  AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>

int main(void) {
  pid_t pid;
  int status;
  int ipc_id;
  char *shm;
  struct shmid_ds shmbuf;

  ipc_id = shmget(IPC_PRIVATE, 4096, (IPC_CREAT | SHM_R | SHM_W));
  if (ipc_id == -1) {
    return 1;
  }

  shm = shmat(ipc_id, NULL, 0);
  if (shm == (void *)-1) {
    shmctl(ipc_id, IPC_RMID, NULL);
    return 2;
  }

  if (shmctl(ipc_id, IPC_STAT, &shmbuf) != 0) {
    shmdt(shm);
    shmctl(ipc_id, IPC_RMID, NULL);
    return 3;
  }

  shmbuf.shm_perm.uid = getuid();
  shmbuf.shm_perm.gid = getgid();
  shmbuf.shm_perm.mode = 0600;

  if (shmctl(ipc_id, IPC_SET, &shmbuf) != 0) {
    shmdt(shm);
    shmctl(ipc_id, IPC_RMID, NULL);
    return 4;
  }

  shmctl(ipc_id, IPC_RMID, NULL);

  strcpy(shm, "hello");

  pid = fork();
  if (pid < 0) {
    return 5;
  } else if (pid == 0) {
    strcpy(shm, "bye");
    return 6;
  }
  if (wait(&status) != pid) {
    return 7;
  }
  if (!WIFEXITED(status) || WEXITSTATUS(status) != 6) {
    return 8;
  }
  if (strcmp(shm, "bye") != 0) {
    return 9;
  }
  return 0;
}
]])],[have_shm_ipc=yes],[have_shm_ipc=no],[have_shm_ipc=no])
  if test "$have_shm_ipc" = "yes"; then
    AC_DEFINE(HAVE_SHM_IPC, 1, [Define if you have SysV IPC SHM support])
  fi
  AC_MSG_RESULT([$have_shm_ipc])

  AC_MSG_CHECKING(for mmap() using MAP_ANON shared memory support)
  AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#ifndef MAP_ANON
# ifdef MAP_ANONYMOUS
#  define MAP_ANON MAP_ANONYMOUS
# endif
#endif
#ifndef MAP_FAILED
# define MAP_FAILED ((void*)-1)
#endif

int main(void) {
  pid_t pid;
  int status;
  char *shm;

  shm = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
  if (shm == MAP_FAILED) {
    return 1;
  }

  strcpy(shm, "hello");

  pid = fork();
  if (pid < 0) {
    return 5;
  } else if (pid == 0) {
    strcpy(shm, "bye");
    return 6;
  }
  if (wait(&status) != pid) {
    return 7;
  }
  if (!WIFEXITED(status) || WEXITSTATUS(status) != 6) {
    return 8;
  }
  if (strcmp(shm, "bye") != 0) {
    return 9;
  }
  return 0;
}
]])],[have_shm_mmap_anon=yes],[have_shm_mmap_anon=no],[
  case $host_alias in
    *linux*)
      have_shm_mmap_anon=yes
      ;;
    *)
      have_shm_mmap_anon=no
      ;;
  esac
])
  if test "$have_shm_mmap_anon" = "yes"; then
    AC_DEFINE(HAVE_SHM_MMAP_ANON, 1, [Define if you have mmap(MAP_ANON) SHM support])
  fi
  AC_MSG_RESULT([$have_shm_mmap_anon])

  CRX_CHECK_FUNC_LIB(shm_open, rt, root)
  AC_MSG_CHECKING(for mmap() using shm_open() shared memory support)
  AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef MAP_FAILED
# define MAP_FAILED ((void*)-1)
#endif

int main(void) {
  pid_t pid;
  int status;
  int fd;
  char *shm;
  char tmpname[4096];

  sprintf(tmpname,"/opcache.test.shm.%dXXXXXX", getpid());
  if (mktemp(tmpname) == NULL) {
    return 1;
  }
  fd = shm_open(tmpname, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    return 2;
  }
  if (ftruncate(fd, 4096) < 0) {
    close(fd);
    shm_unlink(tmpname);
    return 3;
  }

  shm = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shm == MAP_FAILED) {
    return 4;
  }
  shm_unlink(tmpname);
  close(fd);

  strcpy(shm, "hello");

  pid = fork();
  if (pid < 0) {
    return 5;
  } else if (pid == 0) {
    strcpy(shm, "bye");
    return 6;
  }
  if (wait(&status) != pid) {
    return 7;
  }
  if (!WIFEXITED(status) || WEXITSTATUS(status) != 6) {
    return 8;
  }
  if (strcmp(shm, "bye") != 0) {
    return 9;
  }
  return 0;
}
]])],[have_shm_mmap_posix=yes],[have_shm_mmap_posix=no],[have_shm_mmap_posix=no])
  if test "$have_shm_mmap_posix" = "yes"; then
    AC_DEFINE(HAVE_SHM_MMAP_POSIX, 1, [Define if you have POSIX mmap() SHM support])
    CRX_CHECK_LIBRARY(rt, shm_unlink, [CRX_ADD_LIBRARY(rt,1,OPCACHE_SHARED_LIBADD)])
  fi
  AC_MSG_RESULT([$have_shm_mmap_posix])

  CRX_NEW_EXTENSION(opcache,
	CrexAccelerator.c \
	crex_accelerator_blacklist.c \
	crex_accelerator_debug.c \
	crex_accelerator_hash.c \
	crex_accelerator_module.c \
	crex_persist.c \
	crex_persist_calc.c \
	crex_file_cache.c \
	crex_shared_alloc.c \
	crex_accelerator_util_funcs.c \
	shared_alloc_shm.c \
	shared_alloc_mmap.c \
	shared_alloc_posix.c \
	$CREX_JIT_SRC,
	shared,,"-Wno-implicit-fallthrough -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1",,yes)

  CRX_ADD_EXTENSION_DEP(opcache, pcre)

  if test "$have_shm_ipc" != "yes" && test "$have_shm_mmap_posix" != "yes" && test "$have_shm_mmap_anon" != "yes"; then
    AC_MSG_ERROR([No supported shared memory caching support was found when configuring opcache. Check config.log for any errors or missing dependencies.])
  fi

  if test "$CRX_OPCACHE_JIT" = "yes"; then
    CRX_ADD_BUILD_DIR([$ext_builddir/jit], 1)
    CRX_ADD_MAKEFILE_FRAGMENT($ext_srcdir/jit/Makefile.frag)
  fi
  CRX_SUBST(OPCACHE_SHARED_LIBADD)
fi
