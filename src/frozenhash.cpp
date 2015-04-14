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

bool FrozenMap::open(const char *filename, off_t offset)
{
    int fd = ::open(filename, O_RDONLY);
    if (fd < 0)
        return false;
    return open(fd, offset);
}

bool FrozenMap::open(int fd, off_t offset)
{
    m_fd = fd;
    header = new FrozenHashMapHeader;
    if (header == 0) return false;

    if (::read(fd, header, sizeof(struct FrozenHashMapHeader)) != sizeof(struct FrozenHashMapHeader))
        return false;

    hashtable_map = (uint64_t *)mmap(NULL, header->hashtable_size, PROT_READ, MAP_SHARED, m_fd, offset + header->hashtable_offset);
    if (hashtable_map == MAP_FAILED) return false;

    valuetable_map = mmap(NULL, header->valuetable_size, PROT_READ, MAP_SHARED, m_fd, offset + header->valuetable_offset);
    if (valuetable_map == MAP_FAILED) return false;

    valuetable = new ValueTableReader((const char*)valuetable_map, header->valuetable_size);
    
    return true;
}

const char *FrozenMap::get(const char *key, size_t keysp, size_t *valuesp)
{
    uint64_t hash[2];
    MurmurHash3_x64_128(key, keysp, HASH_RANDOM_SEED, hash);
    uint64_t hashvalue = hash[0] % header->hashsize;
    uint64_t valueoffset = hashtable_map[hashvalue];

    if (valueoffset == UINT64_MAX)
        return NULL;
    
    valuetable->seek(valueoffset);

    do {
        size_t datakey_length;
        const char *datakey = valuetable->readNext(&datakey_length);
        if (datakey == NULL)
            return NULL;
        
        // MurmurHash3_x64_128(datakey, datakey_length, HASH_RANDOM_SEED, hash);
        // if (hashvalue != (hash[0] % header->hashsize))
        //    return NULL;

        size_t datavalue_length;
        const char *datavalue = valuetable->readNext(&datavalue_length);
        if (datavalue == NULL)
            return NULL;

        if (keysp == datakey_length && (memcmp(key, datakey, keysp) == 0)) {
            *valuesp = datavalue_length;
            return datavalue;
        }
    } while (1);
    return NULL;
}

std::string FrozenMap::get(const std::string &key)
{
    size_t valuesize;
    const char *value = get(key.c_str(), key.length(), &valuesize);
    if (value == NULL)
        return std::string();
    return std::string(value, valuesize);
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
