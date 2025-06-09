crxdbg: $(BUILD_BINARY)

crxdbg-shared: $(BUILD_SHARED)

$(BUILD_SHARED): $(CRX_GLOBAL_OBJS) $(CRX_BINARY_OBJS) $(CRX_CRXDBG_OBJS)
	$(BUILD_CRXDBG_SHARED)

$(BUILD_BINARY): $(CRX_GLOBAL_OBJS) $(CRX_BINARY_OBJS) $(CRX_CRXDBG_OBJS)
	$(BUILD_CRXDBG)

%.c: %.y
%.c: %.l

$(builddir)/crxdbg_lexer.lo: $(srcdir)/crxdbg_parser.h

$(srcdir)/crxdbg_lexer.c: $(srcdir)/crxdbg_lexer.l
	@(cd $(top_srcdir); $(RE2C) $(RE2C_FLAGS) --no-generation-date -cbdFo sapi/crxdbg/crxdbg_lexer.c sapi/crxdbg/crxdbg_lexer.l)

$(srcdir)/crxdbg_parser.h: $(srcdir)/crxdbg_parser.c
$(srcdir)/crxdbg_parser.c: $(srcdir)/crxdbg_parser.y
	@$(YACC) $(YFLAGS) -v -d $(srcdir)/crxdbg_parser.y -o $@

install-crxdbg: $(BUILD_BINARY)
	@echo "Installing crxdbg binary:         $(INSTALL_ROOT)$(bindir)/"
	@$(mkinstalldirs) $(INSTALL_ROOT)$(bindir)
	@$(mkinstalldirs) $(INSTALL_ROOT)$(localstatedir)/log
	@$(mkinstalldirs) $(INSTALL_ROOT)$(localstatedir)/run
	@$(INSTALL) -m 0755 $(BUILD_BINARY) $(INSTALL_ROOT)$(bindir)/$(program_prefix)crxdbg$(program_suffix)$(EXEEXT)
	@echo "Installing crxdbg man page:       $(INSTALL_ROOT)$(mandir)/man1/"
	@$(mkinstalldirs) $(INSTALL_ROOT)$(mandir)/man1
	@$(INSTALL_DATA) sapi/crxdbg/crxdbg.1 $(INSTALL_ROOT)$(mandir)/man1/$(program_prefix)crxdbg$(program_suffix).1
