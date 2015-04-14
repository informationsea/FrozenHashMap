#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <limits.h>

#define HASH_SIZE_FACTOR 1.3
#define HASH_RANDOM_SEED 1961288601U
#define HASH_VALYE_BYTES 8

#define _CONCAT(x, y) x ## y
#define CONCAT(x, y) _CONCAT(x, y)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

const char FROZENHASH_HEADER[16] = "FROZENHASHMAP";

struct FrozenHashMapHeader {
    char magic[16];
    uint64_t hashsize;
    uint64_t hashtable_offset;
    uint64_t hashtable_size;
    uint64_t valuetable_offset;
    uint64_t valuetable_size;
};

#ifndef UINT64_MAX
#define UINT64_MAX 0xFFffFFffFFffFFff
#endif

#endif /* COMMON_H */
