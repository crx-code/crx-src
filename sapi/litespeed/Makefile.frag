litespeed: $(SAPI_LITESPEED_PATH)

$(SAPI_LITESPEED_PATH): $(CRX_GLOBAL_OBJS) $(CRX_BINARY_OBJS) $(CRX_LITESPEED_OBJS)
	$(BUILD_LITESPEED)

install-litespeed: $(SAPI_LITESPEED_PATH)
	@echo "Installing CRX LiteSpeed binary:  $(INSTALL_ROOT)$(bindir)/"
	@$(mkinstalldirs) $(INSTALL_ROOT)$(bindir)
	@$(INSTALL) -m 0755 $(SAPI_LITESPEED_PATH) $(INSTALL_ROOT)$(bindir)/$(program_prefix)lscrx$(program_suffix)
