#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <limits.h>

#define HASH_SIZE_FACTOR 2
#define HASH_RANDOM_SEED 1961288601U
#define HASH_VALUE_BYTES 8

#define _CONCAT(x, y) x ## y
#define CONCAT(x, y) _CONCAT(x, y)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

const char FROZENHASH_HEADER[16] = "FROZENHASHMAP";

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
};

#define DB_ENDIAN_CHECK 0x0123456789ABCDEF
#define DB_FORMAT_VERSION 0x01

#ifndef UINT64_MAX
#define UINT64_MAX 0xFFffFFffFFffFFff
#endif

#ifndef UINT32_MAX
#define UINT32_MAX 0xFFffFFff
#endif

#endif /* COMMON_H */
