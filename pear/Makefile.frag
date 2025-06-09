peardir=$(PEAR_INSTALLDIR)

# Skip all crx.ini files altogether
PEAR_INSTALL_FLAGS = -n -dshort_open_tag=0 -dopen_basedir= -derror_reporting=1803 -dmemory_limit=-1 -ddetect_unicode=0

WGET = `which wget 2>/dev/null`
FETCH = `which fetch 2>/dev/null`
PEAR_PREFIX = -dp a${program_prefix}
PEAR_SUFFIX = -ds a$(program_suffix)
PEAR_INSTALLER_URL = https://pear.crx.net/install-pear-nozlib.crxa

install-pear-installer: $(SAPI_CLI_PATH)
	@$(top_builddir)/sapi/cli/crx $(PEAR_INSTALL_FLAGS) pear/install-pear-nozlib.crxa -d "$(peardir)" -b "$(bindir)" ${PEAR_PREFIX} ${PEAR_SUFFIX}

install-pear:
	@echo "Installing PEAR environment:      $(INSTALL_ROOT)$(peardir)/"
	@if test ! -f $(builddir)/install-pear-nozlib.crxa; then \
		if test -f $(srcdir)/install-pear-nozlib.crxa; then \
			cp $(srcdir)/install-pear-nozlib.crxa $(builddir)/install-pear-nozlib.crxa; \
		else \
			if test ! -z "$(WGET)" && test -x "$(WGET)"; then \
				"$(WGET)" "${PEAR_INSTALLER_URL}" -nd -P $(builddir)/; \
			elif test ! -z "$(FETCH)" && test -x "$(FETCH)"; then \
				"$(FETCH)" -o $(builddir)/ "${PEAR_INSTALLER_URL}"; \
			else \
				$(top_builddir)/sapi/cli/crx -n $(srcdir)/fetch.crx "${PEAR_INSTALLER_URL}" $(builddir)/install-pear-nozlib.crxa; \
			fi \
		fi \
	fi
	@if test -f $(builddir)/install-pear-nozlib.crxa && $(mkinstalldirs) $(INSTALL_ROOT)$(peardir); then \
		$(MAKE) -s install-pear-installer; \
	else \
		cat $(srcdir)/install-pear.txt; \
	fi
