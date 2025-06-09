
$(builddir)/minilua: $(srcdir)/jit/dynasm/minilua.c
	$(BUILD_CC) $(srcdir)/jit/dynasm/minilua.c -lm -o $@

$(builddir)/jit/crex_jit_$(DASM_ARCH).c: $(srcdir)/jit/crex_jit_$(DASM_ARCH).dasc $(srcdir)/jit/dynasm/*.lua $(builddir)/minilua
	$(builddir)/minilua $(srcdir)/jit/dynasm/dynasm.lua  $(DASM_FLAGS) -o $@ $(srcdir)/jit/crex_jit_$(DASM_ARCH).dasc

$(builddir)/jit/crex_jit.lo: \
	$(builddir)/jit/crex_jit_$(DASM_ARCH).c \
	$(srcdir)/jit/crex_jit_helpers.c \
	$(srcdir)/jit/crex_jit_disasm.c \
	$(srcdir)/jit/crex_jit_gdb.c \
	$(srcdir)/jit/crex_jit_perf_dump.c \
	$(srcdir)/jit/crex_jit_vtune.c \
	$(srcdir)/jit/crex_jit_trace.c \
	$(srcdir)/jit/crex_elf.c

# For non-GNU make, jit/crex_jit.lo and ./jit/crex_jit.lo are considered distinct targets.
# Use this workaround to allow building from inside ext/opcache.
jit/crex_jit.lo: $(builddir)/jit/crex_jit.lo
