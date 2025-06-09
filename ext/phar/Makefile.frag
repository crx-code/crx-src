$(srcdir)/crxa_path_check.c: $(srcdir)/crxa_path_check.re
	@(cd $(top_srcdir); \
	if test -f ./crx_crxa.h; then \
		$(RE2C) $(RE2C_FLAGS) --no-generation-date -b -o crxa_path_check.c crxa_path_check.re; \
	else \
		$(RE2C) $(RE2C_FLAGS) --no-generation-date -b -o ext/crxa/crxa_path_check.c ext/crxa/crxa_path_check.re; \
	fi)

crxacmd: $(builddir)/crxa.crx $(builddir)/crxa.crxa

CRX_CRXACMD_SETTINGS = -n -d 'open_basedir=' -d 'output_buffering=0' -d 'memory_limit=-1' -d crxa.readonly=0
CRX_CRXACMD_EXECUTABLE = ` \
	if test -x "$(top_builddir)/$(SAPI_CLI_PATH)"; then \
		$(top_srcdir)/build/shtool echo -n -- "$(top_builddir)/$(SAPI_CLI_PATH) -n"; \
		if test "x$(CRX_MODULES)" != "x"; then \
		$(top_srcdir)/build/shtool echo -n -- " -d extension_dir=$(top_builddir)/modules"; \
		for i in bz2 zlib crxa; do \
			if test -f "$(top_builddir)/modules/$$i.la"; then \
				. $(top_builddir)/modules/$$i.la; $(top_srcdir)/build/shtool echo -n -- " -d extension=$$dlname"; \
			fi; \
		done; \
		fi; \
	else \
		$(top_srcdir)/build/shtool echo -n -- "$(CRX_EXECUTABLE)"; \
	fi;`
CRX_CRXACMD_BANG = `$(top_srcdir)/build/shtool echo -n -- "$(INSTALL_ROOT)$(bindir)/$(program_prefix)crx$(program_suffix)$(EXEEXT)";`

$(builddir)/crxa/crxa.inc: $(srcdir)/crxa/crxa.inc
	-@test -d $(builddir)/crxa || mkdir $(builddir)/crxa
	-@test -f $(builddir)/crxa/crxa.inc || cp $(srcdir)/crxa/crxa.inc $(builddir)/crxa/crxa.inc


TEST_CRX_EXECUTABLE = $(shell $(CRX_EXECUTABLE) -v 2>&1)
TEST_CRX_EXECUTABLE_RES = $(shell echo "$(TEST_CRX_EXECUTABLE)" | grep -c 'Exec format error')

$(builddir)/crxa.crx: $(srcdir)/build_precommand.crx $(srcdir)/crxa/*.inc $(srcdir)/crxa/*.crx $(SAPI_CLI_PATH)
	-@(echo "Generating crxa.crx"; \
	if [ "$(TEST_CRX_EXECUTABLE_RES)" != 1 ]; then \
		$(CRX_CRXACMD_EXECUTABLE) $(CRX_CRXACMD_SETTINGS) $(srcdir)/build_precommand.crx > $(builddir)/crxa.crx; \
	else \
		echo "Skipping crxa.crx generating during cross compilation"; \
	fi)

$(builddir)/crxa.crxa: $(builddir)/crxa.crx $(builddir)/crxa/crxa.inc $(srcdir)/crxa/*.inc $(srcdir)/crxa/*.crx $(SAPI_CLI_PATH)
	-@(echo "Generating crxa.crxa"; \
	if [ "$(TEST_CRX_EXECUTABLE_RES)" != 1 ]; then \
		rm -f $(builddir)/crxa.crxa; \
		rm -f $(srcdir)/crxa.crxa; \
		$(CRX_CRXACMD_EXECUTABLE) $(CRX_CRXACMD_SETTINGS) $(builddir)/crxa.crx pack -f $(builddir)/crxa.crxa -a crxacommand -c auto -x \\.svn -p 0 -s $(srcdir)/crxa/crxa.crx -h sha1 -b "$(CRX_CRXACMD_BANG)"  $(srcdir)/crxa/; \
		chmod +x $(builddir)/crxa.crxa; \
	else \
		echo "Skipping crxa.crxa generating during cross compilation"; \
	fi)

install-crxacmd: crxacmd
	@(if [ "$(TEST_CRX_EXECUTABLE_RES)" != 1 ]; then \
		$(mkinstalldirs) $(INSTALL_ROOT)$(bindir); \
		$(INSTALL) $(builddir)/crxa.crxa $(INSTALL_ROOT)$(bindir)/$(program_prefix)crxa$(program_suffix).crxa; \
		rm -f $(INSTALL_ROOT)$(bindir)/$(program_prefix)crxa$(program_suffix); \
		$(LN_S) -f $(program_prefix)crxa$(program_suffix).crxa $(INSTALL_ROOT)$(bindir)/$(program_prefix)crxa$(program_suffix); \
		$(mkinstalldirs) $(INSTALL_ROOT)$(mandir)/man1; \
		$(INSTALL_DATA) $(builddir)/crxa.1 $(INSTALL_ROOT)$(mandir)/man1/$(program_prefix)crxa$(program_suffix).1; \
		$(INSTALL_DATA) $(builddir)/crxa.crxa.1 $(INSTALL_ROOT)$(mandir)/man1/$(program_prefix)crxa$(program_suffix).crxa.1; \
	else \
		echo "Skipping install-crxacmd during cross compilation"; \
	fi)
