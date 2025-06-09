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
   |          Hao Sun <hao.sun@arm.com>                                   |
   +----------------------------------------------------------------------+
*/


#ifdef HAVE_CAPSTONE
# define HAVE_DISASM 1
# include <capstone.h>
# define HAVE_CAPSTONE_ITER 1
#elif CREX_JIT_TARGET_X86
# define HAVE_DISASM 1
# define DISASM_INTEL_SYNTAX 0

# include "jit/libudis86/itab.c"
# include "jit/libudis86/decode.c"
# include "jit/libudis86/syn.c"
# if DISASM_INTEL_SYNTAX
#  include "jit/libudis86/syn-intel.c"
# else
#  include "jit/libudis86/syn-att.c"
# endif
# include "jit/libudis86/udis86.c"
#endif /* HAVE_CAPSTONE */

#ifdef HAVE_DISASM

static void crex_jit_disasm_add_symbol(const char *name,
                                       uint64_t    addr,
                                       uint64_t    size);

#ifndef _WIN32
# include "jit/crex_elf.c"
#endif

#include "crex_sort.h"

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#ifndef _WIN32
#include <dlfcn.h>
#endif

struct _sym_node {
	uint64_t          addr;
	uint64_t          end;
	struct _sym_node *parent;
	struct _sym_node *child[2];
	unsigned char     info;
	char              name[1];
};

static void crex_syms_rotateleft(crex_sym_node *p) {
	crex_sym_node *r = p->child[1];
	p->child[1] = r->child[0];
	if (r->child[0]) {
		r->child[0]->parent = p;
	}
	r->parent = p->parent;
	if (p->parent == NULL) {
		JIT_G(symbols) = r;
	} else if (p->parent->child[0] == p) {
		p->parent->child[0] = r;
	} else {
		p->parent->child[1] = r;
	}
	r->child[0] = p;
	p->parent = r;
}

static void crex_syms_rotateright(crex_sym_node *p) {
	crex_sym_node *l = p->child[0];
	p->child[0] = l->child[1];
	if (l->child[1]) {
		l->child[1]->parent = p;
	}
	l->parent = p->parent;
	if (p->parent == NULL) {
		JIT_G(symbols) = l;
	} else if (p->parent->child[1] == p) {
		p->parent->child[1] = l;
	} else {
		p->parent->child[0] = l;
	}
	l->child[1] = p;
	p->parent = l;
}

static void crex_jit_disasm_add_symbol(const char *name,
                                       uint64_t    addr,
                                       uint64_t    size)
{
	crex_sym_node *sym;
	size_t len = strlen(name);

	sym = malloc(sizeof(crex_sym_node) + len + 1);
	if (!sym) {
		return;
	}
	sym->addr = addr;
	sym->end  = (addr + size - 1);
	memcpy((char*)&sym->name, name, len + 1);
	sym->parent = sym->child[0] = sym->child[1] = NULL;
	sym->info = 1;
	if (JIT_G(symbols)) {
		crex_sym_node *node = JIT_G(symbols);

		/* insert it into rbtree */
		do {
			if (sym->addr > node->addr) {
				CREX_ASSERT(sym->addr > (node->end));
				if (node->child[1]) {
					node = node->child[1];
				} else {
					node->child[1] = sym;
					sym->parent = node;
					break;
				}
			} else if (sym->addr < node->addr) {
				if (node->child[0]) {
					node = node->child[0];
				} else {
					node->child[0] = sym;
					sym->parent = node;
					break;
				}
			} else {
				CREX_ASSERT(sym->addr == node->addr);
				if (strcmp(name, node->name) == 0 && sym->end < node->end) {
					/* reduce size of the existing symbol */
					node->end = sym->end;
				}
				free(sym);
				return;
			}
		} while (1);

		/* fix rbtree after instering */
		while (sym && sym != JIT_G(symbols) && sym->parent->info == 1) {
			if (sym->parent == sym->parent->parent->child[0]) {
				node = sym->parent->parent->child[1];
				if (node && node->info == 1) {
					sym->parent->info = 0;
					node->info = 0;
					sym->parent->parent->info = 1;
					sym = sym->parent->parent;
				} else {
					if (sym == sym->parent->child[1]) {
						sym = sym->parent;
						crex_syms_rotateleft(sym);
					}
					sym->parent->info = 0;
					sym->parent->parent->info = 1;
					crex_syms_rotateright(sym->parent->parent);
				}
			} else {
				node = sym->parent->parent->child[0];
				if (node && node->info == 1) {
					sym->parent->info = 0;
					node->info = 0;
					sym->parent->parent->info = 1;
					sym = sym->parent->parent;
				} else {
					if (sym == sym->parent->child[0]) {
						sym = sym->parent;
						crex_syms_rotateright(sym);
					}
					sym->parent->info = 0;
					sym->parent->parent->info = 1;
					crex_syms_rotateleft(sym->parent->parent);
				}
			}
		}
	} else {
		JIT_G(symbols) = sym;
	}
	JIT_G(symbols)->info = 0;
}

static void crex_jit_disasm_destroy_symbols(crex_sym_node *n) {
	if (n) {
		if (n->child[0]) {
			crex_jit_disasm_destroy_symbols(n->child[0]);
		}
		if (n->child[1]) {
			crex_jit_disasm_destroy_symbols(n->child[1]);
		}
		free(n);
	}
}

static const char* crex_jit_disasm_find_symbol(uint64_t  addr,
                                               int64_t  *offset) {
	crex_sym_node *node = JIT_G(symbols);
	while (node) {
		if (addr < node->addr) {
			node = node->child[0];
		} else if (addr > node->end) {
			node = node->child[1];
		} else {
			*offset = addr - node->addr;
			return node->name;
		}
	}
	return NULL;
}

#ifdef HAVE_CAPSTONE
static uint64_t crex_jit_disasm_branch_target(csh cs, const cs_insn *insn)
{
	unsigned int i;

#if CREX_JIT_TARGET_X86
	if (cs_insn_group(cs, insn, X86_GRP_JUMP)) {
		for (i = 0; i < insn->detail->x86.op_count; i++) {
			if (insn->detail->x86.operands[i].type == X86_OP_IMM) {
				return insn->detail->x86.operands[i].imm;
			}
		}
	}
#elif CREX_JIT_TARGET_ARM64
	if (cs_insn_group(cs, insn, ARM64_GRP_JUMP)
	 || insn->id == ARM64_INS_BL
	 || insn->id == ARM64_INS_ADR) {
		for (i = 0; i < insn->detail->arm64.op_count; i++) {
			if (insn->detail->arm64.operands[i].type == ARM64_OP_IMM)
				return insn->detail->arm64.operands[i].imm;
		}
	}
#endif

	return 0;
}
#endif

static const char* crex_jit_disasm_resolver(
#ifndef HAVE_CAPSTONE
                                            struct ud *ud,
#endif
                                            uint64_t   addr,
                                            int64_t   *offset)
{
#ifndef _WIN32
# ifndef HAVE_CAPSTONE
	((void)ud);
# endif
	const char *name;
	void *a = (void*)(uintptr_t)(addr);
	Dl_info info;

	name = crex_jit_disasm_find_symbol(addr, offset);
	if (name) {
		return name;
	}

	if (dladdr(a, &info)
	 && info.dli_sname != NULL
	 && info.dli_saddr == a) {
		return info.dli_sname;
	}
#else
	const char *name;
	name = crex_jit_disasm_find_symbol(addr, offset);
	if (name) {
		return name;
	}
#endif

	return NULL;
}

static int crex_jit_cmp_labels(Bucket *b1, Bucket *b2)
{
	return ((b1->h > b2->h) > 0) ? 1 : -1;
}

static int crex_jit_disasm(const char    *name,
                           const char    *filename,
                           const crex_op_array *op_array,
                           crex_cfg      *cfg,
                           const void    *start,
                           size_t         size)
{
	const void *end = (void *)((char *)start + size);
	zval zv, *z;
	crex_long n, m;
	HashTable labels;
	uint64_t addr;
	int b;
#ifdef HAVE_CAPSTONE
	csh cs;
	cs_insn *insn;
# ifdef HAVE_CAPSTONE_ITER
	const uint8_t *cs_code;
	size_t cs_size;
	uint64_t cs_addr;
# else
	size_t count, i;
# endif
	const char *sym;
	int64_t offset = 0;
	char *p, *q, *r;
#else
	struct ud ud;
	const struct ud_operand *op;
#endif

#ifdef HAVE_CAPSTONE
# if CREX_JIT_TARGET_X86
#  if defined(__x86_64__) || defined(_WIN64)
	if (cs_open(CS_ARCH_X86, CS_MODE_64, &cs) != CS_ERR_OK)
		return 0;
#  else
	if (cs_open(CS_ARCH_X86, CS_MODE_32, &cs) != CS_ERR_OK)
		return 0;
#  endif
	cs_option(cs, CS_OPT_DETAIL, CS_OPT_ON);
#  if DISASM_INTEL_SYNTAX
	cs_option(cs, CS_OPT_SYNTAX, CS_OPT_SYNTAX_INTEL);
#  else
	cs_option(cs, CS_OPT_SYNTAX, CS_OPT_SYNTAX_ATT);
#  endif
# elif CREX_JIT_TARGET_ARM64
	if (cs_open(CS_ARCH_ARM64, CS_MODE_ARM, &cs) != CS_ERR_OK)
		return 0;
	cs_option(cs, CS_OPT_DETAIL, CS_OPT_ON);
	cs_option(cs, CS_OPT_SYNTAX, CS_OPT_SYNTAX_ATT);
# endif
#else
	ud_init(&ud);
# if defined(__x86_64__) || defined(_WIN64)
	ud_set_mode(&ud, 64);
# else
	ud_set_mode(&ud, 32);
# endif
# if DISASM_INTEL_SYNTAX
	ud_set_syntax(&ud, UD_SYN_INTEL);
# else
	ud_set_syntax(&ud, UD_SYN_ATT);
# endif
	ud_set_sym_resolver(&ud, crex_jit_disasm_resolver);
#endif /* HAVE_CAPSTONE */

	if (name) {
		fprintf(stderr, "%s: ; (%s)\n", name, filename ? filename : "unknown");
	}

#ifndef HAVE_CAPSTONE
	ud_set_input_buffer(&ud, (uint8_t*)start, (uint8_t*)end - (uint8_t*)start);
	ud_set_pc(&ud, (uint64_t)(uintptr_t)start);
#endif

	crex_hash_init(&labels, 8, NULL, NULL, 0);
	if (op_array && cfg) {
		ZVAL_FALSE(&zv);
		for (b = 0; b < cfg->blocks_count; b++) {
			if (cfg->blocks[b].flags & (CREX_BB_ENTRY|CREX_BB_RECV_ENTRY)) {
				addr = (uint64_t)(uintptr_t)op_array->opcodes[cfg->blocks[b].start].handler;
				if (addr >= (uint64_t)(uintptr_t)start && addr < (uint64_t)(uintptr_t)end) {
					crex_hash_index_add(&labels, addr, &zv);
				}
			}
		}
	}
#ifdef HAVE_CAPSTONE
	ZVAL_TRUE(&zv);
# ifdef HAVE_CAPSTONE_ITER
	cs_code = start;
	cs_size = (uint8_t*)end - (uint8_t*)start;
	cs_addr = (uint64_t)(uintptr_t)cs_code;
	insn = cs_malloc(cs);
	while (cs_disasm_iter(cs, &cs_code, &cs_size, &cs_addr, insn)) {
		if ((addr = crex_jit_disasm_branch_target(cs, insn))) {
# else
	count = cs_disasm(cs, start, (uint8_t*)end - (uint8_t*)start, (uintptr_t)start, 0, &insn);
	for (i = 0; i < count; i++) {
		if ((addr = crex_jit_disasm_branch_target(cs, &(insn[i])))) {
# endif
			if (addr >= (uint64_t)(uintptr_t)start && addr < (uint64_t)(uintptr_t)end) {
				crex_hash_index_add(&labels, addr, &zv);
			}
		}
	}
#else
	ZVAL_TRUE(&zv);
	while (ud_disassemble(&ud)) {
		op = ud_insn_opr(&ud, 0);
		if (op && op->type == UD_OP_JIMM) {
			addr = ud_syn_rel_target(&ud, (struct ud_operand*)op);
			if (addr >= (uint64_t)(uintptr_t)start && addr < (uint64_t)(uintptr_t)end) {
				crex_hash_index_add(&labels, addr, &zv);
			}
		}
	}
#endif

	crex_hash_sort(&labels, crex_jit_cmp_labels, 0);

	/* label numbering */
	n = 0; m = 0;
	CREX_HASH_MAP_FOREACH_VAL(&labels, z) {
		if (C_TYPE_P(z) == IS_FALSE) {
			m--;
			ZVAL_LONG(z, m);
		} else {
			n++;
			ZVAL_LONG(z, n);
		}
	} CREX_HASH_FOREACH_END();

#ifdef HAVE_CAPSTONE
# ifdef HAVE_CAPSTONE_ITER
	cs_code = start;
	cs_size = (uint8_t*)end - (uint8_t*)start;
	cs_addr = (uint64_t)(uintptr_t)cs_code;
	while (cs_disasm_iter(cs, &cs_code, &cs_size, &cs_addr, insn)) {
		z = crex_hash_index_find(&labels, insn->address);
# else
	for (i = 0; i < count; i++) {
		z = crex_hash_index_find(&labels, insn[i].address);
# endif
		if (z) {
			if (C_LVAL_P(z) < 0) {
				fprintf(stderr, ".ENTRY" CREX_LONG_FMT ":\n", -C_LVAL_P(z));
			} else {
				fprintf(stderr, ".L" CREX_LONG_FMT ":\n", C_LVAL_P(z));
			}
		}

# ifdef HAVE_CAPSTONE_ITER
		if (JIT_G(debug) & CREX_JIT_DEBUG_ASM_ADDR) {
			fprintf(stderr, "    %" PRIx64 ":", insn->address);
		}
		fprintf(stderr, "\t%s ", insn->mnemonic);
		p = insn->op_str;
# else
		if (JIT_G(debug) & CREX_JIT_DEBUG_ASM_ADDR) {
			fprintf(stderr, "    %" PRIx64 ":", insn[i].address);
		}
		fprintf(stderr, "\t%s ", insn[i].mnemonic);
		p = insn[i].op_str;
# endif
		/* Try to replace the target addresses with a symbols */
		while ((q = strchr(p, 'x')) != NULL) {
			if (p != q && *(q-1) == '0') {
				r = q + 1;
				addr = 0;
				while (1) {
					if (*r >= '0' && *r <= '9') {
						addr = addr * 16 + (*r - '0');
					} else if (*r >= 'A' && *r <= 'F') {
						addr = addr * 16 + (*r - 'A' + 10);
					} else if (*r >= 'a' && *r <= 'f') {
						addr = addr * 16 + (*r - 'a' + 10);
					} else {
						break;
					}
					r++;
				}
				if (addr >= (uint64_t)(uintptr_t)start && addr < (uint64_t)(uintptr_t)end) {
					if ((z = crex_hash_index_find(&labels, addr))) {
						if (C_LVAL_P(z) < 0) {
							fwrite(p, 1, q - p - 1, stderr);
							fprintf(stderr, ".ENTRY" CREX_LONG_FMT, -C_LVAL_P(z));
						} else {
							fwrite(p, 1, q - p - 1, stderr);
							fprintf(stderr, ".L" CREX_LONG_FMT, C_LVAL_P(z));
						}
					} else {
						fwrite(p, 1, r - p, stderr);
					}
				} else if ((sym = crex_jit_disasm_resolver(addr, &offset))) {
					fwrite(p, 1, q - p - 1, stderr);
					fputs(sym, stderr);
					if (offset != 0) {
						if (offset > 0) {
							fprintf(stderr, "+%" PRIx64, offset);
						} else {
							fprintf(stderr, "-%" PRIx64, offset);
						}
					}
				} else {
					fwrite(p, 1, r - p, stderr);
				}
				p = r;
			} else {
				fwrite(p, 1, q - p + 1, stderr);
				p = q + 1;
			}
		}
		fprintf(stderr, "%s\n", p);
	}
# ifdef HAVE_CAPSTONE_ITER
	cs_free(insn, 1);
# else
	cs_free(insn, count);
# endif
#else
	ud_set_input_buffer(&ud, (uint8_t*)start, (uint8_t*)end - (uint8_t*)start);
	ud_set_pc(&ud, (uint64_t)(uintptr_t)start);

	while (ud_disassemble(&ud)) {
		addr = ud_insn_off(&ud);
		z = crex_hash_index_find(&labels, addr);
		if (z) {
			if (C_LVAL_P(z) < 0) {
				fprintf(stderr, ".ENTRY" CREX_LONG_FMT ":\n", -C_LVAL_P(z));
			} else {
				fprintf(stderr, ".L" CREX_LONG_FMT ":\n", C_LVAL_P(z));
			}
		}
		op = ud_insn_opr(&ud, 0);
		if (op && op->type == UD_OP_JIMM) {
			addr = ud_syn_rel_target(&ud, (struct ud_operand*)op);
			if (addr >= (uint64_t)(uintptr_t)start && addr < (uint64_t)(uintptr_t)end) {
				z = crex_hash_index_find(&labels, addr);
				if (z) {
					const char *str = ud_insn_asm(&ud);
					int len;

					len = 0;
					while (str[len] != 0 && str[len] != ' ' && str[len] != '\t') {
						len++;
					}
					if (str[len] != 0) {
						while (str[len] == ' ' || str[len] == '\t') {
							len++;
						}
						if (C_LVAL_P(z) < 0) {
							fprintf(stderr, "\t%.*s.ENTRY" CREX_LONG_FMT "\n", len, str, -C_LVAL_P(z));
						} else {
							fprintf(stderr, "\t%.*s.L" CREX_LONG_FMT "\n", len, str, C_LVAL_P(z));
						}
						continue;
					}
				}
			}
		}
		if (JIT_G(debug) & CREX_JIT_DEBUG_ASM_ADDR) {
			fprintf(stderr, "    %" PRIx64 ":", ud_insn_off(&ud));
		}
		fprintf(stderr, "\t%s\n", ud_insn_asm(&ud));
	}
#endif
	fprintf(stderr, "\n");

	crex_hash_destroy(&labels);

#ifdef HAVE_CAPSTONE
	cs_close(&cs);
#endif

	return 1;
}

static int crex_jit_disasm_init(void)
{
#ifndef ZTS
#define REGISTER_EG(n)  \
	crex_jit_disasm_add_symbol("EG("#n")", \
		(uint64_t)(uintptr_t)&executor_globals.n, sizeof(executor_globals.n))
	REGISTER_EG(uninitialized_zval);
	REGISTER_EG(exception);
	REGISTER_EG(vm_interrupt);
	REGISTER_EG(exception_op);
	REGISTER_EG(timed_out);
	REGISTER_EG(current_execute_data);
	REGISTER_EG(vm_stack_top);
	REGISTER_EG(vm_stack_end);
	REGISTER_EG(symbol_table);
	REGISTER_EG(jit_trace_num);
#undef  REGISTER_EG
#define REGISTER_CG(n)  \
	crex_jit_disasm_add_symbol("CG("#n")", \
		(uint64_t)(uintptr_t)&compiler_globals.n, sizeof(compiler_globals.n))
	REGISTER_CG(map_ptr_base);
#undef  REGISTER_CG
#endif

	/* Register JIT helper functions */
#define REGISTER_HELPER(n)  \
	crex_jit_disasm_add_symbol(#n, \
		(uint64_t)(uintptr_t)n, sizeof(void*));
	REGISTER_HELPER(memcmp);
	REGISTER_HELPER(crex_jit_init_func_run_time_cache_helper);
	REGISTER_HELPER(crex_jit_find_func_helper);
	REGISTER_HELPER(crex_jit_find_ns_func_helper);
	REGISTER_HELPER(crex_jit_find_method_helper);
	REGISTER_HELPER(crex_jit_find_method_tmp_helper);
	REGISTER_HELPER(crex_jit_push_static_metod_call_frame);
	REGISTER_HELPER(crex_jit_push_static_metod_call_frame_tmp);
	REGISTER_HELPER(crex_jit_invalid_method_call);
	REGISTER_HELPER(crex_jit_invalid_method_call_tmp);
	REGISTER_HELPER(crex_jit_unref_helper);
	REGISTER_HELPER(crex_jit_extend_stack_helper);
	REGISTER_HELPER(crex_jit_int_extend_stack_helper);
	REGISTER_HELPER(crex_jit_leave_nested_func_helper);
	REGISTER_HELPER(crex_jit_leave_top_func_helper);
	REGISTER_HELPER(crex_jit_leave_func_helper);
	REGISTER_HELPER(crex_jit_symtable_find);
	REGISTER_HELPER(crex_jit_hash_index_lookup_rw_no_packed);
	REGISTER_HELPER(crex_jit_hash_index_lookup_rw);
	REGISTER_HELPER(crex_jit_hash_lookup_rw);
	REGISTER_HELPER(crex_jit_symtable_lookup_rw);
	REGISTER_HELPER(crex_jit_symtable_lookup_w);
	REGISTER_HELPER(crex_jit_undefined_op_helper);
	REGISTER_HELPER(crex_jit_fetch_dim_r_helper);
	REGISTER_HELPER(crex_jit_fetch_dim_is_helper);
	REGISTER_HELPER(crex_jit_fetch_dim_isset_helper);
	REGISTER_HELPER(crex_jit_fetch_dim_str_offset_r_helper);
	REGISTER_HELPER(crex_jit_fetch_dim_str_r_helper);
	REGISTER_HELPER(crex_jit_fetch_dim_str_is_helper);
	REGISTER_HELPER(crex_jit_fetch_dim_obj_r_helper);
	REGISTER_HELPER(crex_jit_fetch_dim_obj_is_helper);
	REGISTER_HELPER(crex_jit_fetch_dim_rw_helper);
	REGISTER_HELPER(crex_jit_fetch_dim_w_helper);
	REGISTER_HELPER(crex_jit_fetch_dim_obj_rw_helper);
	REGISTER_HELPER(crex_jit_fetch_dim_obj_w_helper);
//	REGISTER_HELPER(crex_jit_fetch_dim_obj_unset_helper);
	REGISTER_HELPER(crex_jit_assign_dim_helper);
	REGISTER_HELPER(crex_jit_assign_dim_op_helper);
	REGISTER_HELPER(crex_jit_fast_assign_concat_helper);
	REGISTER_HELPER(crex_jit_fast_concat_helper);
	REGISTER_HELPER(crex_jit_fast_concat_tmp_helper);
	REGISTER_HELPER(crex_jit_isset_dim_helper);
	REGISTER_HELPER(crex_jit_free_call_frame);
	REGISTER_HELPER(crex_jit_fetch_global_helper);
	REGISTER_HELPER(crex_jit_verify_arg_slow);
	REGISTER_HELPER(crex_jit_verify_return_slow);
	REGISTER_HELPER(crex_jit_fetch_obj_r_slow);
	REGISTER_HELPER(crex_jit_fetch_obj_r_dynamic);
	REGISTER_HELPER(crex_jit_fetch_obj_is_slow);
	REGISTER_HELPER(crex_jit_fetch_obj_is_dynamic);
	REGISTER_HELPER(crex_jit_fetch_obj_w_slow);
	REGISTER_HELPER(crex_jit_check_array_promotion);
	REGISTER_HELPER(crex_jit_create_typed_ref);
	REGISTER_HELPER(crex_jit_extract_helper);
	REGISTER_HELPER(crex_jit_vm_stack_free_args_helper);
	REGISTER_HELPER(crex_jit_copy_extra_args_helper);
	REGISTER_HELPER(crex_jit_deprecated_helper);
	REGISTER_HELPER(crex_jit_assign_const_to_typed_ref);
	REGISTER_HELPER(crex_jit_assign_tmp_to_typed_ref);
	REGISTER_HELPER(crex_jit_assign_var_to_typed_ref);
	REGISTER_HELPER(crex_jit_assign_cv_to_typed_ref);
	REGISTER_HELPER(crex_jit_assign_const_to_typed_ref2);
	REGISTER_HELPER(crex_jit_assign_tmp_to_typed_ref2);
	REGISTER_HELPER(crex_jit_assign_var_to_typed_ref2);
	REGISTER_HELPER(crex_jit_assign_cv_to_typed_ref2);
	REGISTER_HELPER(crex_jit_pre_inc_typed_ref);
	REGISTER_HELPER(crex_jit_pre_dec_typed_ref);
	REGISTER_HELPER(crex_jit_post_inc_typed_ref);
	REGISTER_HELPER(crex_jit_post_dec_typed_ref);
	REGISTER_HELPER(crex_jit_assign_op_to_typed_ref);
	REGISTER_HELPER(crex_jit_assign_op_to_typed_ref_tmp);
	REGISTER_HELPER(crex_jit_only_vars_by_reference);
	REGISTER_HELPER(crex_jit_invalid_array_access);
	REGISTER_HELPER(crex_jit_invalid_property_read);
	REGISTER_HELPER(crex_jit_invalid_property_write);
	REGISTER_HELPER(crex_jit_invalid_property_incdec);
	REGISTER_HELPER(crex_jit_invalid_property_assign);
	REGISTER_HELPER(crex_jit_invalid_property_assign_op);
	REGISTER_HELPER(crex_jit_prepare_assign_dim_ref);
	REGISTER_HELPER(crex_jit_pre_inc);
	REGISTER_HELPER(crex_jit_pre_dec);
	REGISTER_HELPER(crex_runtime_jit);
	REGISTER_HELPER(crex_jit_hot_func);
	REGISTER_HELPER(crex_jit_check_constant);
	REGISTER_HELPER(crex_jit_get_constant);
	REGISTER_HELPER(crex_jit_array_free);
	REGISTER_HELPER(crex_jit_zval_array_dup);
	REGISTER_HELPER(crex_jit_add_arrays_helper);
	REGISTER_HELPER(crex_jit_assign_obj_helper);
	REGISTER_HELPER(crex_jit_assign_obj_op_helper);
	REGISTER_HELPER(crex_jit_assign_to_typed_prop);
	REGISTER_HELPER(crex_jit_assign_op_to_typed_prop);
	REGISTER_HELPER(crex_jit_inc_typed_prop);
	REGISTER_HELPER(crex_jit_dec_typed_prop);
	REGISTER_HELPER(crex_jit_pre_inc_typed_prop);
	REGISTER_HELPER(crex_jit_pre_dec_typed_prop);
	REGISTER_HELPER(crex_jit_post_inc_typed_prop);
	REGISTER_HELPER(crex_jit_post_dec_typed_prop);
	REGISTER_HELPER(crex_jit_pre_inc_obj_helper);
	REGISTER_HELPER(crex_jit_pre_dec_obj_helper);
	REGISTER_HELPER(crex_jit_post_inc_obj_helper);
	REGISTER_HELPER(crex_jit_post_dec_obj_helper);
	REGISTER_HELPER(crex_jit_rope_end);
	REGISTER_HELPER(crex_jit_free_trampoline_helper);
	REGISTER_HELPER(crex_jit_exception_in_interrupt_handler_helper);
#undef  REGISTER_HELPER

#ifndef _WIN32
	crex_elf_load_symbols();
#endif

	if (crex_vm_kind() == CREX_VM_KIND_HYBRID) {
		crex_op opline;

		memset(&opline, 0, sizeof(opline));

		opline.opcode = CREX_DO_UCALL;
		opline.result_type = IS_UNUSED;
		crex_vm_set_opcode_handler(&opline);
		crex_jit_disasm_add_symbol("CREX_DO_UCALL_SPEC_RETVAL_UNUSED_LABEL", (uint64_t)(uintptr_t)opline.handler, sizeof(void*));

		opline.opcode = CREX_DO_UCALL;
		opline.result_type = IS_VAR;
		crex_vm_set_opcode_handler(&opline);
		crex_jit_disasm_add_symbol("CREX_DO_UCALL_SPEC_RETVAL_USED_LABEL", (uint64_t)(uintptr_t)opline.handler, sizeof(void*));

		opline.opcode = CREX_DO_FCALL_BY_NAME;
		opline.result_type = IS_UNUSED;
		crex_vm_set_opcode_handler(&opline);
		crex_jit_disasm_add_symbol("CREX_DO_FCALL_BY_NAME_SPEC_RETVAL_UNUSED_LABEL", (uint64_t)(uintptr_t)opline.handler, sizeof(void*));

		opline.opcode = CREX_DO_FCALL_BY_NAME;
		opline.result_type = IS_VAR;
		crex_vm_set_opcode_handler(&opline);
		crex_jit_disasm_add_symbol("CREX_DO_FCALL_BY_NAME_SPEC_RETVAL_USED_LABEL", (uint64_t)(uintptr_t)opline.handler, sizeof(void*));

		opline.opcode = CREX_DO_FCALL;
		opline.result_type = IS_UNUSED;
		crex_vm_set_opcode_handler(&opline);
		crex_jit_disasm_add_symbol("CREX_DO_FCALL_SPEC_RETVAL_UNUSED_LABEL", (uint64_t)(uintptr_t)opline.handler, sizeof(void*));

		opline.opcode = CREX_DO_FCALL;
		opline.result_type = IS_VAR;
		crex_vm_set_opcode_handler(&opline);
		crex_jit_disasm_add_symbol("CREX_DO_FCALL_SPEC_RETVAL_USED_LABEL", (uint64_t)(uintptr_t)opline.handler, sizeof(void*));

		opline.opcode = CREX_RETURN;
		opline.op1_type = IS_CONST;
		crex_vm_set_opcode_handler(&opline);
		crex_jit_disasm_add_symbol("CREX_RETURN_SPEC_CONST_LABEL", (uint64_t)(uintptr_t)opline.handler, sizeof(void*));

		opline.opcode = CREX_RETURN;
		opline.op1_type = IS_TMP_VAR;
		crex_vm_set_opcode_handler(&opline);
		crex_jit_disasm_add_symbol("CREX_RETURN_SPEC_TMP_LABEL", (uint64_t)(uintptr_t)opline.handler, sizeof(void*));

		opline.opcode = CREX_RETURN;
		opline.op1_type = IS_VAR;
		crex_vm_set_opcode_handler(&opline);
		crex_jit_disasm_add_symbol("CREX_RETURN_SPEC_VAR_LABEL", (uint64_t)(uintptr_t)opline.handler, sizeof(void*));

		opline.opcode = CREX_RETURN;
		opline.op1_type = IS_CV;
		crex_vm_set_opcode_handler(&opline);
		crex_jit_disasm_add_symbol("CREX_RETURN_SPEC_CV_LABEL", (uint64_t)(uintptr_t)opline.handler, sizeof(void*));

		crex_jit_disasm_add_symbol("CREX_HYBRID_HALT_LABEL", (uint64_t)(uintptr_t)crex_jit_halt_op->handler, sizeof(void*));
	}

	return 1;
}

static void crex_jit_disasm_shutdown(void)
{
	if (JIT_G(symbols)) {
		crex_jit_disasm_destroy_symbols(JIT_G(symbols));
		JIT_G(symbols) = NULL;
	}
}

#endif /* HAVE_DISASM */
