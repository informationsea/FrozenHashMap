EXTRA_DIST = gene_info_summary_head.txt.xz
AM_CPPFLAGS = -DSFMT_MEXP=86243 $(kyotocabinet_CFLAGS)

noinst_PROGRAMS = kyotocabinet_kch kyotocabinet_kct cdbpp frozenhashmap
noinst_DATA =  gene_info_summary_head.txt

noinst_LTLIBRARIES = libperformance_common.la
libperformance_common_la_SOURCES = SFMT.c performance_common.cpp kyotocabinet_common.cpp
libperformance_common_la_LIBADD = $(kyotocabinet_LIBS)

kyotocabinet_kch_SOURCES = kyotocabinet_kch.cpp
kyotocabinet_kch_LDADD = libperformance_common.la

kyotocabinet_kct_SOURCES = kyotocabinet_kct.cpp
kyotocabinet_kct_LDADD = libperformance_common.la

frozenhashmap_SOURCES = frozenhashmap_performance.cpp
frozenhashmap_LDADD = libperformance_common.la $(top_builddir)/src/libfrozenhashmap.la

cdbpp_SOURCES = cdbpp_performance.cpp
cdbpp_LDADD = libperformance_common.la


gene_info_summary_head.txt: gene_info_summary_head.txt.xz
	xz -kd gene_info_summary_head.txt.xz
