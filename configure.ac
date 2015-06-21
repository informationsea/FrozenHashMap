AC_PREREQ([2.68])
AC_INIT([frozenhashmap], [0.0.1], [okamura@informationsea.info])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall])

AC_CONFIG_SRCDIR([src/frozenhash.cpp])
AC_CONFIG_HEADERS([src/config.h])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

# Checks for programs.
AM_PROG_AR
AC_CONFIG_MACRO_DIR([m4])
ACLOCAL_AMFLAGS="-I m4"
AM_SILENT_RULES([yes])
AM_PROG_LIBTOOL([shared])

AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for libraries.
PKG_CHECK_MODULES([kyotocabinet], [kyotocabinet >= 1.2.76], [ac_cv_use_kyotocabinet="yes"], [ac_cv_use_kyotocabinet="no"])
AM_CONDITIONAL([WITH_KYOTOCABINET], [test "$ac_cv_use_kyotocabinet" = "yes"])
AC_CHECK_LIB([pthread], [pthread_create])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h inttypes.h limits.h netdb.h stddef.h stdint.h stdlib.h string.h strings.h sys/time.h sys/timeb.h unistd.h wchar.h wctype.h])

# Check for performance test
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([leveldb/db.h])
AC_LANG_POP([C++])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_FSEEKO
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS([bzero dup2 fchdir getcwd getpagesize gettimeofday memset mkdir munmap putenv regcomp rmdir socket strcasecmp strchr strdup strerror strrchr strstr strtol strtoul strtoull])

AC_CONFIG_FILES([src/Makefile Makefile test/Makefile test/performance/Makefile frozenhashmap.pc])
AC_OUTPUT
