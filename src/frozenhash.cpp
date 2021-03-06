#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <memory>
#include <unistd.h>
#include <sys/mman.h>
#include <limits.h>

#include "config.h"
#include "frozenhash.hpp"
#include "valuetable.hpp"
#include "MurmurHash3.h"
#include "common.hpp"

#define DEBUG(fmt,...) (fprintf(stderr, __FILE__ ": %3d: " fmt "\n" ,__LINE__, ## __VA_ARGS__))
#define FILE_READ_BLOCK (1024*1024*200)

namespace frozenhashmap {

    FrozenMap::FrozenMap() : header(0), hashtable_map(0), valuetable_map(0), valuetable(0)
    {

    }

    FrozenMap::~FrozenMap()
    {
        if (hashtable_map && header)
            munmap(hashtable_map, header->hashtable_size);
        if (valuetable_map && header)
            munmap(valuetable_map, header->valuetable_size);
        delete header;
        delete valuetable;
    }

    bool FrozenMap::open(const char *filename, off_t offset, bool onmemory)
    {
        int fd = ::open(filename, O_RDONLY);
        if (fd < 0)
            return false;
        return open(fd, offset, onmemory);
    }

    bool FrozenMap::open(int fd, off_t offset, bool onmemory)
    {
        m_fd = fd;
        header = new FrozenHashMapHeader;
        if (header == 0) return false;

        if (::read(fd, header, sizeof(struct FrozenHashMapHeader)) != sizeof(struct FrozenHashMapHeader))
            return false;

        //DEBUG("HEADER             COUNT: " UINT64UF, header->count);
        //DEBUG("HEADER          HASHSIZE: " UINT64UF, header->hashsize);
        //DEBUG("HEADER  HASHTABLE_OFFSET: " UINT64UF, header->hashtable_offset);
        //DEBUG("HEADER    HASHTABLE_SIZE: " UINT64UF, header->hashtable_size);
        //DEBUG("HEADER VALUETABLE_OFFSET: " UINT64UF, header->valuetable_offset);
        //DEBUG("HEADER   VALUETABLE_SIZE: " UINT64UF, header->valuetable_size);


        if (memcmp(FROZENHASH_HEADER, header->magic, sizeof(FROZENHASH_HEADER)) != 0) {
            fprintf(stderr, "This file is not frozen hash map database\n");
            return false;
        }

        if (header->endian_check != DB_ENDIAN_CHECK) {
            fprintf(stderr, "Wrong endian\n");
            return false;
        }

        if (header->version != DB_FORMAT_VERSION) {
            fprintf(stderr, "Wrong database format version\n");
            return false;
        }

        if (onmemory) {
            hashtable_map = (FrozenHashMapHashPosition *)mmap(NULL, header->hashtable_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS,
                                                              -1, 0);
            if (hashtable_map == MAP_FAILED) {
                return false;
            }
            if (lseek(m_fd, offset + header->hashtable_offset, SEEK_SET) != offset + (off_t)header->hashtable_offset) return false;
            {
                ssize_t remainBytes = (ssize_t)header->hashtable_size;
                char *readPosition = (char *)hashtable_map;
                
                while (remainBytes > 0) {
                    ssize_t readBytes = read(m_fd, readPosition, MIN(FILE_READ_BLOCK, remainBytes));
                    if (readBytes < 0) return false;
                    readPosition += readBytes;
                    remainBytes -= readBytes;
                }
            }
        } else {
            hashtable_map = (FrozenHashMapHashPosition *)mmap(NULL, header->hashtable_size, PROT_READ, MAP_PRIVATE,
                                                              m_fd, offset + header->hashtable_offset);
            if (hashtable_map == MAP_FAILED) return false;
        }

        FrozenHashMapHashPosition empty;
        memset(&empty, 0xff, sizeof(empty));
        for (uint64_t i = 0; i < header->hashsize; i++) {
            if (memcmp(&empty, hashtable_map+i, sizeof(empty)) == 0) continue;
            //fprintf(stderr, "table[%llu] = (%llu, %llx)\n", i, hashtable_map[i].hash_value, hashtable_map[i].value_position);
        }

        if (onmemory) {
            valuetable_map = mmap(NULL, header->valuetable_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS,
                                  -1, 0);
            if (valuetable_map == MAP_FAILED)
                return false;

            if (lseek(m_fd, offset + header->valuetable_offset, SEEK_SET) != offset + (off_t)header->valuetable_offset) return false;
            {
                ssize_t remainBytes = (ssize_t)header->valuetable_size;
                char *readPosition = (char *)valuetable_map;
                
                while (remainBytes > 0) {
                    ssize_t readBytes = read(m_fd, readPosition, MIN(remainBytes, FILE_READ_BLOCK));
                    if (readBytes < 0) return false;
                    readPosition += readBytes;
                    remainBytes -= readBytes;
                }
            }
        } else {
            valuetable_map = mmap(NULL, header->valuetable_size, PROT_READ, MAP_PRIVATE,
                                  m_fd, offset + header->valuetable_offset);
            if (valuetable_map == MAP_FAILED) return false;
        }

        valuetable = new ValueTableReader((const char*)valuetable_map, header->valuetable_size);
        return true;
    }

    const char *FrozenMap::get(const char *key, size_t keysp, size_t *valuesp) const
    {
        uint64_t hash[2];
        MurmurHash3_x64_128(key, keysp, HASH_RANDOM_SEED, hash);
        size_t table_position = hash[1] % header->hashsize;
        //fprintf(stderr, "GET %s %lu = %llu\n", key, keysp, hash[0]);

        do {
            do {
                if (hashtable_map[table_position].value_position == UINT32_MAX) {
                    // DEBUG("Not found %s", key);
                    return NULL; // Not found
                }

                if (hashtable_map[table_position].hash_value == (uint32_t)hash[0])
                    break; // found hash equally entry
                table_position = (table_position + 1) % header->hashsize;
            } while (1);
            
            off_t key_position = hashtable_map[table_position].value_position * VALUE_TABLE_ALIGNMENT_BYTES;
            //fprintf(stderr, "key position = %LLD\n", key_position);
            off_t value_position;
            uint32_t keylen;
            const char *keydata = valuetable->readAt(key_position, &keylen, &value_position);
            if (keylen != keysp) {
                table_position = (table_position + 1) % header->hashsize; continue;
            }
            if (memcmp(key, keydata, keysp) != 0) {
                table_position = (table_position + 1) % header->hashsize; continue;
            }

            uint32_t len;
            const char* value = valuetable->readAt(value_position, &len, NULL);
            *valuesp = len;
            return value;
        } while(1);
    }

    std::string FrozenMap::get(const std::string &key) const
    {
        size_t valuesize;
        const char *value = get(key.c_str(), key.length(), &valuesize);
        if (value == NULL)
            return std::string();
        return std::string(value, valuesize);
    }

    uint64_t FrozenMap::count() const
    {
        return header->count;
    }

    FrozenMapCursor::FrozenMapCursor(FrozenMap *parent): m_parent(parent), valuetable(0)
    {
        valuetable = new ValueTableReader((const char*)m_parent->valuetable_map, m_parent->header->valuetable_size);
    }

    FrozenMapCursor::~FrozenMapCursor()
    {
        delete valuetable;
    }

    FrozenMap *FrozenMapCursor::db()
    {
        return m_parent;
    }

    bool FrozenMapCursor::nextString(std::pair<std::string, std::string> *pair)
    {
        const char *key, *value;
        size_t keylen, valuelen;
        bool ok = next(&key, &keylen, &value, &valuelen);
        if (!ok) return false;
        pair->first = std::string(key, keylen);
        pair->second = std::string(value, valuelen);
        return true;
    }

    bool FrozenMapCursor::next(const char **key, size_t *keylen, const char **value, size_t *valuelen)
    {
        *key = valuetable->readNext(keylen);
        if (*key == NULL)
            return false;
        *value = valuetable->readNext(valuelen);
        if (*value == NULL)
            return false;
        return true;
    }

    void FrozenMap::printInfo(std::FILE *output) const
    {
        fprintf(output, "           Version: "UINT64UF"\n", header->version);
        fprintf(output, "             Count: "UINT64UF"\n", header->count);
        fprintf(output, "         Hash size: "UINT64UF"\n", header->hashsize);
        fprintf(output, " Hash table offset: "UINT64UF"\n", header->hashtable_offset);
        fprintf(output, "   Hash table size: "UINT64UF"\n", header->hashtable_size);
        fprintf(output, "Value table offset: "UINT64UF"\n", header->valuetable_offset);
        fprintf(output, "  Value table size: "UINT64UF"\n", header->valuetable_size);

        uint64_t longest_nonempty = 0;
        uint64_t current_nonempty = 0;
        FrozenHashMapHashPosition empty;
        memset(&empty, 0xff, sizeof(empty));
        
        for (uint64_t i = 0; i < header->hashsize; i++) {
            if (memcmp(&empty, hashtable_map+i, sizeof(empty)) == 0) {
                current_nonempty = 0;
            } else {
                current_nonempty += 1;
                longest_nonempty = MAX(current_nonempty, longest_nonempty);
            }
        }
        fprintf(output, "Longest non-empty hash table: "UINT64UF"\n", longest_nonempty);
        
    }
}
