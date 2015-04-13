if WITH_CUTTER
TESTS = run-test.sh
TESTS_ENVIRONMENT = CUTTER="$(CUTTER)"
EXTRA_DIST = run-test.sh geneid-symbol.txt

AM_LDFLAGS = -module -rpath $(libdir) -avoid-version -no-undefined
AM_CPPFLAGS = $(kyotocabinet_CFLAGS) $(CPPCUTTER_CFLAGS) -O0 -g
LIBS += $(CPPCUTTER_LIBS) $(top_builddir)/src/libfrozenhashmap.la

noinst_LTLIBRARIES = test_frozenhash_builder.la
test_frozenhash_builder_la_SOURCES = test_frozenhash_builder.cpp

echo-cutter:
	@echo $(CUTTER)

endif
