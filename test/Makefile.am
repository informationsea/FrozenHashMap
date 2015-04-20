EXTRA_DIST = run-test.sh geneid-symbol.txt
SUBDIRS = performance

if WITH_CUTTER
TESTS = run-test.sh
TESTS_ENVIRONMENT = CUTTER="$(CUTTER)"

AM_LDFLAGS = -module -rpath $(libdir) -avoid-version -no-undefined
AM_CPPFLAGS = $(kyotocabinet_CFLAGS) $(CPPCUTTER_CFLAGS) -O0 -g
LIBS += $(CPPCUTTER_LIBS) $(top_builddir)/src/libfrozenhashmap.la

noinst_LTLIBRARIES = test_frozenhash_builder.la test_frozenhash_cbinding.la test_mutable_value_table.la test_mutablehash.la
test_frozenhash_builder_la_SOURCES = test_frozenhash_builder.cpp
test_frozenhash_cbinding_la_SOURCES = test_frozenhash_cbinding.c
test_mutable_value_table_la_SOURCES = test_mutable_value_table.cpp
test_mutablehash_la_SOURCES = test_mutablehash.cpp

echo-cutter:
	@echo $(CUTTER)

endif
