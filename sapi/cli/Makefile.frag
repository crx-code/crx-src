cli: $(SAPI_CLI_PATH)

$(SAPI_CLI_PATH): $(CRX_GLOBAL_OBJS) $(CRX_BINARY_OBJS) $(CRX_CLI_OBJS)
	$(BUILD_CLI)

install-cli: $(SAPI_CLI_PATH)
	@echo "Installing CRX CLI binary:        $(INSTALL_ROOT)$(bindir)/"
	@$(mkinstalldirs) $(INSTALL_ROOT)$(bindir)
	@$(INSTALL) -m 0755 $(SAPI_CLI_PATH) $(INSTALL_ROOT)$(bindir)/$(program_prefix)crx$(program_suffix)$(EXEEXT)
	@echo "Installing CRX CLI man page:      $(INSTALL_ROOT)$(mandir)/man1/"
	@$(mkinstalldirs) $(INSTALL_ROOT)$(mandir)/man1
	@$(INSTALL_DATA) sapi/cli/crx.1 $(INSTALL_ROOT)$(mandir)/man1/$(program_prefix)crx$(program_suffix).1
