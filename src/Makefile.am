AM_LDFLAGS = -no-undefined
AM_CPPFLAGS = $(kyotocabinet_CFLAGS)

bin_PROGRAMS = frozenhashmgr
frozenhashmgr_SOURCES = manager.cpp
frozenhashmgr_LDADD = libfrozenhashmap.la

lib_LTLIBRARIES = libfrozenhashmap.la
libfrozenhashmap_la_SOURCES = MurmurHash3.cpp frozenhash.cpp valuetable.cpp frozenhashbuilder.cpp cfhm.cpp

libfrozenhashmap_la_includedir = $(includedir)/frozenhashmap
libfrozenhashmap_la_include_HEADERS = \
	MurmurHash3.h frozenhash.hpp valuetable.hpp frozenhashbuilder.hpp cfhm.h
libfrozenhashmap_la_LIBADD = $(kyotocabinet_LIBS)
