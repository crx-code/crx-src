#
# Crex
#

$(builddir)/crex_language_scanner.lo: $(srcdir)/crex_language_parser.h
$(builddir)/crex_ini_scanner.lo: $(srcdir)/crex_ini_parser.h

$(srcdir)/crex_language_scanner.c $(srcdir)/crex_language_scanner_defs.h: $(srcdir)/crex_language_scanner.l
	@(cd $(top_srcdir); $(RE2C) $(RE2C_FLAGS) --no-generation-date --case-inverted -cbdFt Crex/crex_language_scanner_defs.h -oCrex/crex_language_scanner.c Crex/crex_language_scanner.l)

$(srcdir)/crex_language_parser.h: $(srcdir)/crex_language_parser.c
$(srcdir)/crex_language_parser.c: $(srcdir)/crex_language_parser.y
# Tweak crexparse to be exported through CREX_API. This has to be revisited once
# bison supports foreign skeletons and that bison version is used. Read
# https://git.savannah.gnu.org/cgit/bison.git/tree/data/README.md for more.
	@$(YACC) $(YFLAGS) -v -d $(srcdir)/crex_language_parser.y -o $@
	@$(SED) -e 's,^int crexparse\(.*\),CREX_API int crexparse\1,g' < $@ \
	> $@.tmp && \
	mv $@.tmp $@
	@$(SED) -e 's,^int crexparse\(.*\),CREX_API int crexparse\1,g' < $(srcdir)/crex_language_parser.h \
	> $(srcdir)/crex_language_parser.h.tmp && \
	mv $(srcdir)/crex_language_parser.h.tmp $(srcdir)/crex_language_parser.h

$(srcdir)/crex_ini_parser.h: $(srcdir)/crex_ini_parser.c
$(srcdir)/crex_ini_parser.c: $(srcdir)/crex_ini_parser.y
	$(YACC) $(YFLAGS) -v -d $(srcdir)/crex_ini_parser.y -o $@

$(srcdir)/crex_ini_scanner.c: $(srcdir)/crex_ini_scanner.l
	@(cd $(top_srcdir); $(RE2C) $(RE2C_FLAGS) --no-generation-date --case-inverted -cbdFt Crex/crex_ini_scanner_defs.h -oCrex/crex_ini_scanner.c Crex/crex_ini_scanner.l)

# Use an intermediate target to indicate that crex_vm_gen.crx produces both files
# at the same time, rather than the same recipe applying for two different targets.
# The "grouped targets" feature, which would solve this directly, is only available
# since GNU Make 4.3.
$(srcdir)/crex_vm_execute.h $(srcdir)/crex_vm_opcodes.c: vm.gen.intermediate ;
.INTERMEDIATE: vm.gen.intermediate
vm.gen.intermediate: $(srcdir)/crex_vm_def.h $(srcdir)/crex_vm_execute.skl $(srcdir)/crex_vm_gen.crx
	@if test ! -z "$(CRX)"; then \
		$(CRX) $(srcdir)/crex_vm_gen.crx; \
	fi;

$(builddir)/crex_highlight.lo $(builddir)/crex_compile.lo: $(srcdir)/crex_language_parser.h

Crex/crex_execute.lo: $(srcdir)/crex_vm_execute.h $(srcdir)/crex_vm_opcodes.h
