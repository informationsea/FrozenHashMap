AUTOMAKE_OPTIONS = subdir-objects
AM_CPPFLAGS = -Wall -Wextra

EXTRA_DIST = geneid-symbol.txt
SUBDIRS = performance gtest


noinst_PROGRAMS = test_frozenhash_cbinding test_frozenhash_builder test_common test_bloomfilter
TESTS = $(noinst_PROGRAMS)

LDADD = -lpthread $(top_builddir)/src/libfrozenhashmap.la $(top_builddir)/test/gtest/libgtest.la

test_common_SOURCES = test_common.cpp
test_frozenhash_cbinding_SOURCES = test_frozenhash_cbinding.cpp
test_frozenhash_builder_SOURCES = test_frozenhash_builder.cpp
test_bloomfilter_SOURCES = test_bloomfilter.cpp
