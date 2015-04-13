AM_LDFLAGS = -no-undefined
AM_CPPFLAGS = $(kyotocabinet_CFLAGS)

lib_LTLIBRARIES = libfrozenhash.la
libfrozenhash_la_SOURCES = MurmurHash3.cpp frozenhash.cpp valuetable.cpp frozenhashbuilder.cpp

libfrozenhash_la_includedir = $(includedir)/frozenhash
libfrozenhash_la_include_HEADERS = \
	MurmurHash3.h frozenhash.hpp valuetable.hpp frozenhashbuilder.hpp
libfrozenhash_la_LIBADD = $(kyotocabinet_LIBS)
