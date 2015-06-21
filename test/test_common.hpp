#ifndef TEST_COMMOH_H
#define TEST_COMMOH_H

#include "gtest/gtest.h"
#include <string.h>

#define ASSERT_MEMEQ(expmem, explen, actmem, actlen)                    \
    {ASSERT_TRUE(explen == actlen); ASSERT_EQ(0, memcmp(expmem, actmem, explen));}

#endif /* TEST_COMMOH_H */
