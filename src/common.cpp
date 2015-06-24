#include "common.hpp"
#include <unistd.h>
#include <string.h>

namespace frozenhashmap {
    bool fillbytes(int fd, int byte, size_t length)
    {
        char data[1024*10];
        memset(data, byte, sizeof(data));

        size_t remaining = length;
        while (remaining > 0) {
            size_t shouldWrite = MIN(remaining, sizeof(data));
            size_t wrote = write(fd, data, shouldWrite);
            if (shouldWrite != wrote) return false;
            remaining -= wrote;
        }
        return true;
    }

    bool copydata(int destfd, int srcfd)
    {
        char buf[1024*100];
        ssize_t readBytes;

        do {
            readBytes = read(srcfd, buf, sizeof(buf));
            if (readBytes < 0)
                return false;
            ssize_t wroteBytes = write(destfd, buf, readBytes);
            if (readBytes != wroteBytes)
                return false;
        } while(readBytes == sizeof(buf));
        
        return true;
    }
}
