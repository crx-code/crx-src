/*
   +----------------------------------------------------------------------+
   | Crex JIT                                                             |
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
   | Authors: Dmitry Stogov <dmitry@crx.net>                              |
   |          Xinchen Hui <laruence@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_ELF
#define CREX_ELF

#if SIZEOF_SIZE_T == 8
# define ELF64
#else
# undef ELF64
#endif

typedef struct _crex_elf_header {
	uint8_t   emagic[4];
	uint8_t   eclass;
	uint8_t   eendian;
	uint8_t   eversion;
	uint8_t   eosabi;
	uint8_t   eabiversion;
	uint8_t   epad[7];
	uint16_t  type;
	uint16_t  machine;
	uint32_t  version;
	uintptr_t entry;
	uintptr_t phofs;
	uintptr_t shofs;
	uint32_t  flags;
	uint16_t  ehsize;
	uint16_t  phentsize;
	uint16_t  phnum;
	uint16_t  shentsize;
	uint16_t  shnum;
	uint16_t  shstridx;
} crex_elf_header;

typedef struct crex_elf_sectheader {
	uint32_t  name;
	uint32_t  type;
	uintptr_t flags;
	uintptr_t addr;
	uintptr_t ofs;
	uintptr_t size;
	uint32_t  link;
	uint32_t  info;
	uintptr_t align;
	uintptr_t entsize;
} crex_elf_sectheader;

#define ELFSECT_IDX_ABS     0xfff1

enum {
	ELFSECT_TYPE_PROGBITS = 1,
	ELFSECT_TYPE_SYMTAB = 2,
	ELFSECT_TYPE_STRTAB = 3,
	ELFSECT_TYPE_NOBITS = 8,
	ELFSECT_TYPE_DYNSYM = 11,
};

#define ELFSECT_FLAGS_WRITE (1 << 0)
#define ELFSECT_FLAGS_ALLOC (1 << 1)
#define ELFSECT_FLAGS_EXEC  (1 << 2)
#define ELFSECT_FLAGS_TLS   (1 << 10)

typedef struct crex_elf_symbol {
#ifdef ELF64
	uint32_t  name;
	uint8_t   info;
	uint8_t   other;
	uint16_t  sectidx;
	uintptr_t value;
	uint64_t  size;
#else
	uint32_t  name;
	uintptr_t value;
	uint32_t  size;
	uint8_t   info;
	uint8_t   other;
	uint16_t  sectidx;
#endif
} crex_elf_symbol;

#define ELFSYM_BIND(info)       ((info) >> 4)
#define ELFSYM_TYPE(info)       ((info) & 0xf)
#define ELFSYM_INFO(bind, type) (((bind) << 4) | (type))

enum {
	ELFSYM_TYPE_DATA = 2,
	ELFSYM_TYPE_FUNC = 2,
	ELFSYM_TYPE_FILE = 4,
};

enum {
	ELFSYM_BIND_LOCAL  = 0,
	ELFSYM_BIND_GLOBAL = 1,
};

void crex_elf_load_symbols(void);

#endif
