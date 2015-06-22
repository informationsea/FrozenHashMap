AUTOMAKE_OPTIONS = subdir-objects
AM_CPPFLAGS = -Wall -Wextra

EXTRA_DIST = geneid-symbol.txt
SUBDIRS = performance gtest


noinst_PROGRAMS = test_frozenhash_cbinding test_mutablehash test_frozenhash_builder test_mutable_value_table
TESTS = $(noinst_PROGRAMS)

LDADD = -lpthread $(top_builddir)/src/libfrozenhashmap.la $(top_builddir)/test/gtest/libgtest.la

test_frozenhash_cbinding_SOURCES = test_frozenhash_cbinding.cpp
test_mutablehash_SOURCES = test_mutablehash.cpp
test_frozenhash_builder_SOURCES = test_frozenhash_builder.cpp
test_mutable_value_table_SOURCES = test_mutable_value_table.cpp
