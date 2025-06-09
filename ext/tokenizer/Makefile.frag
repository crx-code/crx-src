$(top_srcdir)/ext/tokenizer/tokenizer_data.c: $(top_srcdir)/Crex/crex_language_parser.y
	@if test ! -z "$(CRX)"; then \
		$(CRX) $(srcdir)/tokenizer_data_gen.crx; \
	fi;
$(builddir)/tokenizer.lo: $(top_srcdir)/Crex/crex_language_parser.c $(top_srcdir)/Crex/crex_language_scanner.c
