#include "gtest/gtest.h"
#include "common.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

using namespace frozenhashmap;

TEST(COMMON, CEILING) {
    ASSERT_EQ(10, ceiling(3, 10));
    ASSERT_EQ(30, ceiling(25, 10));
    ASSERT_EQ(33, ceiling(3, 33));
    ASSERT_EQ(66, ceiling(50, 33));
}

TEST(COMMON, FILL) {
    const size_t WRITE_SIZE = 1024*34;
    const char onebyte = 0xa6;
    
    ASSERT_EQ(0, system("mkdir -p tmp"));
    int fd = open("tmp/fill.dat", O_RDWR|O_CREAT|O_TRUNC, 0600);
    ASSERT_NE(-1, fd);
    ASSERT_TRUE(fillbytes(fd, onebyte, WRITE_SIZE));

    ASSERT_EQ(0, lseek(fd, 0, SEEK_SET));
    char buf[1024*40];
    ASSERT_TRUE(WRITE_SIZE == read(fd, buf, sizeof(buf)));

    for (size_t i = 0; i < WRITE_SIZE; i++) {
        ASSERT_EQ(onebyte, buf[i]) << "Byte " << i;
    }
    
    close(fd);
}

TEST(COMMON, COPY) {
    ASSERT_EQ(0, system("mkdir -p tmp"));
    
    int destfd = open("tmp/geneid-symbol.txt", O_WRONLY|O_CREAT|O_TRUNC, 0600);
    int srcfd = open("geneid-symbol.txt", O_RDONLY);
    ASSERT_NE(-1, destfd);
    ASSERT_NE(-1, srcfd);

    ASSERT_TRUE(copydata(destfd, srcfd));
}
