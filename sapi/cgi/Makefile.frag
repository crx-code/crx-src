cgi: $(SAPI_CGI_PATH)

$(SAPI_CGI_PATH): $(CRX_GLOBAL_OBJS) $(CRX_BINARY_OBJS) $(CRX_FASTCGI_OBJS) $(CRX_CGI_OBJS)
	$(BUILD_CGI)

install-cgi: $(SAPI_CGI_PATH)
	@echo "Installing CRX CGI binary:        $(INSTALL_ROOT)$(bindir)/"
	@$(mkinstalldirs) $(INSTALL_ROOT)$(bindir)
	@$(INSTALL) -m 0755 $(SAPI_CGI_PATH) $(INSTALL_ROOT)$(bindir)/$(program_prefix)crx-cgi$(program_suffix)$(EXEEXT)
	@echo "Installing CRX CGI man page:      $(INSTALL_ROOT)$(mandir)/man1/"
	@$(mkinstalldirs) $(INSTALL_ROOT)$(mandir)/man1
	@$(INSTALL_DATA) sapi/cgi/crx-cgi.1 $(INSTALL_ROOT)$(mandir)/man1/$(program_prefix)crx-cgi$(program_suffix).1
