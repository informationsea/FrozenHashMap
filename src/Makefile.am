AM_LDFLAGS = -no-undefined
AM_CPPFLAGS = -Wall -Wextra

bin_PROGRAMS = frozenhashmgr
frozenhashmgr_SOURCES = manager.cpp
frozenhashmgr_LDADD = libfrozenhashmap.la

lib_LTLIBRARIES = libfrozenhashmap.la
libfrozenhashmap_la_SOURCES = MurmurHash3.cpp frozenhash.cpp valuetable.cpp frozenhashbuilder.cpp cfhm.cpp common.cpp

libfrozenhashmap_la_includedir = $(includedir)/frozenhashmap
libfrozenhashmap_la_include_HEADERS = \
	MurmurHash3.h frozenhash.hpp valuetable.hpp frozenhashbuilder.hpp cfhm.h
