#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <cstdio>
#include <cstring>
#include <limits>
#include "config.h"

#define HASH_SIZE_FACTOR 2
#define HASH_RANDOM_SEED 1961288601U

#define _CONCAT(x, y) x ## y
#define CONCAT(x, y) _CONCAT(x, y)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define LENGTH(a) (sizeof(a)/sizeof((a)[0]))

#ifdef __APPLE__
#define UINT64XF "%llx"
#define UINT64UF "%llu"
#define INT64DF  "%lld"
#else
#define UINT64XF "%lx"
#define UINT64UF "%lu"
#define INT64DF  "%ld"
#endif

const char FROZENHASH_HEADER[16] = "FROZENHASHMAP";
#define DB_ENDIAN_CHECK 0x0123456789ABCDEF
#define DB_FORMAT_VERSION 0x03

namespace frozenhashmap {
    struct FrozenHashMapHeader {
        char magic[16];
        uint64_t endian_check;
        uint64_t version;
        uint64_t count;
        uint64_t hashsize;
        uint64_t hashtable_offset;
        uint64_t hashtable_size;
        uint64_t valuetable_offset;
        uint64_t valuetable_size;

        FrozenHashMapHeader(uint64_t count, uint64_t hashsize,
                            uint64_t hashtable_offset, uint64_t hashtable_size,
                            uint64_t valuetable_offset, uint64_t valuetable_size) :
            endian_check(DB_ENDIAN_CHECK), version(DB_FORMAT_VERSION),
            count(count), hashsize(hashsize),
            hashtable_offset(hashtable_offset), hashtable_size(hashtable_size),
            valuetable_offset(valuetable_offset), valuetable_size(valuetable_size) {
            std::memcpy(magic, FROZENHASH_HEADER, sizeof(magic));
        }

        FrozenHashMapHeader() :
            endian_check(DB_ENDIAN_CHECK), version(DB_FORMAT_VERSION) {
            std::memcpy(magic, FROZENHASH_HEADER, sizeof(magic));
        }
    };

    struct FrozenHashMapHashPosition {
        uint32_t hash_value;
        uint32_t value_position;

        FrozenHashMapHashPosition(uint32_t hv, uint32_t vp) :
            hash_value(hv), value_position(vp) {}
        FrozenHashMapHashPosition() : hash_value(0), value_position(0) {}
    };

    template<typename T, typename S> T ceiling(T value, S unit) {
        T div = value/unit;
        if (div*unit == value) return value;
        return (div + 1)*unit;
    }

    bool fillbytes(int fd, int byte, size_t length);
    bool copydata(int destfd, int srcfd);
}

#ifndef UINT64_MAX
#define UINT64_MAX 0xFFffFFffFFffFFff
#endif

#ifndef UINT32_MAX
#define UINT32_MAX 0xFFffFFff
#endif

#ifndef INT64_MAX
#define INT64_MAX 0x7FffFFffFFffFFff
#endif

#ifndef INT32_MAX
#define INT32_MAX 0x7FffFFff
#endif

#define EXPECTED_PAGE_SIZE 4096

#endif /* COMMON_H */
