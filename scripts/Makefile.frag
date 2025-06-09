#
# Build environment install
#

crxincludedir = $(includedir)/crx
crxbuilddir = $(libdir)/build

BUILD_FILES = \
	scripts/crxize.m4 \
	build/libtool.m4 \
	build/ltmain.sh \
	build/ax_check_compile_flag.m4 \
	build/ax_gcc_func_attribute.m4 \
	build/crx_cxx_compile_stdcxx.m4 \
	build/pkg.m4 \
	build/Makefile.global \
	build/crx.m4 \
	build/gen_stub.crx \
	run-tests.crx

BUILD_FILES_EXEC = \
	build/shtool \
	build/config.guess \
	build/config.sub

bin_SCRIPTS = crxize crx-config
man_PAGES = crxize crx-config

install-build:
	@echo "Installing build environment:     $(INSTALL_ROOT)$(crxbuilddir)/"
	@$(mkinstalldirs) $(INSTALL_ROOT)$(crxbuilddir) $(INSTALL_ROOT)$(bindir) && \
	(cd $(top_srcdir) && \
	$(INSTALL) $(BUILD_FILES_EXEC) $(INSTALL_ROOT)$(crxbuilddir) && \
	$(INSTALL_DATA) $(BUILD_FILES) $(INSTALL_ROOT)$(crxbuilddir))

install-programs: $(builddir)/crxize $(builddir)/crx-config
	@echo "Installing helper programs:       $(INSTALL_ROOT)$(bindir)/"
	@$(mkinstalldirs) $(INSTALL_ROOT)$(bindir)
	@for prog in $(bin_SCRIPTS); do \
		echo "  program: $(program_prefix)$${prog}$(program_suffix)"; \
		$(INSTALL) -m 755 $(builddir)/$${prog} $(INSTALL_ROOT)$(bindir)/$(program_prefix)$${prog}$(program_suffix); \
	done
	@echo "Installing man pages:             $(INSTALL_ROOT)$(mandir)/man1/"
	@$(mkinstalldirs) $(INSTALL_ROOT)$(mandir)/man1
	@for page in $(man_PAGES); do \
		echo "  page: $(program_prefix)$${page}$(program_suffix).1"; \
		$(INSTALL_DATA) $(builddir)/man1/$${page}.1 $(INSTALL_ROOT)$(mandir)/man1/$(program_prefix)$${page}$(program_suffix).1; \
	done

$(builddir)/crxize: $(srcdir)/crxize.in $(top_builddir)/config.status
	(CONFIG_FILES=$@ CONFIG_HEADERS= $(top_builddir)/config.status)

$(builddir)/crx-config: $(srcdir)/crx-config.in $(top_builddir)/config.status
	(CONFIG_FILES=$@ CONFIG_HEADERS= $(top_builddir)/config.status)
