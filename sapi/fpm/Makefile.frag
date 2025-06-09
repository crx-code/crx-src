fpm: $(SAPI_FPM_PATH)

$(SAPI_FPM_PATH): $(CRX_GLOBAL_OBJS) $(CRX_BINARY_OBJS) $(CRX_FASTCGI_OBJS) $(CRX_FPM_OBJS)
	$(BUILD_FPM)

install-fpm: $(SAPI_FPM_PATH)
	@echo "Installing CRX FPM binary:        $(INSTALL_ROOT)$(sbindir)/"
	@$(mkinstalldirs) $(INSTALL_ROOT)$(sbindir)
	@$(mkinstalldirs) $(INSTALL_ROOT)$(localstatedir)/log
	@$(mkinstalldirs) $(INSTALL_ROOT)$(localstatedir)/run
	@$(INSTALL) -m 0755 $(SAPI_FPM_PATH) $(INSTALL_ROOT)$(sbindir)/$(program_prefix)crx-fpm$(program_suffix)$(EXEEXT)

	@if test -f "$(INSTALL_ROOT)$(sysconfdir)/crx-fpm.conf"; then \
		echo "Installing CRX FPM defconfig:     skipping"; \
	else \
		echo "Installing CRX FPM defconfig:     $(INSTALL_ROOT)$(sysconfdir)/" && \
		$(mkinstalldirs) $(INSTALL_ROOT)$(sysconfdir)/crx-fpm.d; \
		$(INSTALL_DATA) sapi/fpm/crx-fpm.conf $(INSTALL_ROOT)$(sysconfdir)/crx-fpm.conf.default; \
		$(INSTALL_DATA) sapi/fpm/www.conf $(INSTALL_ROOT)$(sysconfdir)/crx-fpm.d/www.conf.default; \
	fi

	@echo "Installing CRX FPM man page:      $(INSTALL_ROOT)$(mandir)/man8/"
	@$(mkinstalldirs) $(INSTALL_ROOT)$(mandir)/man8
	@$(INSTALL_DATA) sapi/fpm/crx-fpm.8 $(INSTALL_ROOT)$(mandir)/man8/crx-fpm$(program_suffix).8

	@echo "Installing CRX FPM status page:   $(INSTALL_ROOT)$(datadir)/fpm/"
	@$(mkinstalldirs) $(INSTALL_ROOT)$(datadir)/fpm
	@$(INSTALL_DATA) sapi/fpm/status.html $(INSTALL_ROOT)$(datadir)/fpm/status.html
