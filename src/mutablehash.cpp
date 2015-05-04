#include "mutablehash.hpp"

#include "mutablevaluetable.hpp"
#include "MurmurHash3.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

namespace {
    struct KeyChunkHeader {
        uint64_t valuepos;
        uint32_t keylen;
    };
}

namespace frozenhashmap {

#define DEBUG(fmt,...) (debugMode && fprintf(stderr, __FILE__ ": %3d: " fmt "\n" ,__LINE__, ## __VA_ARGS__))
    static bool debugMode = false;

    static int expectedCount2hashSize(uint64_t expectedCount) {
        for (int i = 0; i < 64; i++) {
            if (expectedCount < (1 << i)-1) {
                return i;
            }
        }
        return 64;
    }
    
    MutableHash::MutableHash(uint64_t expectedCount):
        m_hashMask((1 << expectedCount2hashSize(expectedCount))-1),
        m_count(0), m_hashTable(0), m_keytable(0), m_valuetable(0)
    {
        bzero(m_dir_path, sizeof(m_dir_path));
        bzero(m_keytable_path, sizeof(m_keytable_path));
        bzero(m_valuetable_path, sizeof(m_valuetable_path));
        
        char *debug = ::getenv("MUTABLEHASH_DEBUG");
        if (debug != NULL)
            debugMode = true;
        DEBUG("Calling Constructor");
    }
    
    MutableHash::~MutableHash()
    {
        free(m_hashTable);
    }

    bool MutableHash::open(const char *dir)
    {
        if (dir == 0) {
            const char *TEMPDIR_PATTERN = "%s/mutablehash-XXXXXX";
            const char* tmpdir_parent = ::getenv("TMPDIR");
            if (tmpdir_parent == NULL)
                tmpdir_parent = "/tmp";
            snprintf(m_dir_path, sizeof(m_dir_path)-1, TEMPDIR_PATTERN, tmpdir_parent);
            if (mkdtemp(m_dir_path) == NULL)
                return false;
        }
        
        snprintf(m_keytable_path, sizeof(m_keytable_path)-1, "%s/keytable2.dat", m_dir_path);
        DEBUG("keytable_path: %s", m_keytable_path);
        m_keytable = new MutableValueTable(m_keytable_path);
        if (m_keytable->error() != 0)
            return false;

        snprintf(m_valuetable_path, sizeof(m_valuetable_path)-1, "%s/valuetable.dat", m_dir_path);
        m_valuetable = new MutableValueTable(m_valuetable_path);
        if (m_valuetable->error() != 0)
            return false;

        m_hashTable = (uint64_t *)malloc(sizeof(uint64_t)*m_hashMask);
        if (m_hashTable == NULL)
            return false;
        memset(m_hashTable, 0xff, sizeof(uint64_t)*m_hashMask);
        return true;
    }

    bool MutableHash::set(const void *key, uint32_t keylen, const void *data, uint32_t datalen)
    {
        uint64_t hashvalue = hashvalue4key(key, keylen);
        uint64_t valuepos = valuePosition(key, keylen, hashvalue);
        if (valuepos == UINT64_MAX) {
            return add(key, keylen, hashvalue, data, datalen);
        } else {
            return m_valuetable->updateEntry(valuepos, data, datalen);
        }
    }
    
    bool MutableHash::append(const void *key, uint32_t keylen, const void *data, uint32_t datalen)
    {
        uint64_t hashvalue = hashvalue4key(key, keylen);
        uint64_t valuepos = valuePosition(key, keylen, hashvalue);
        if (valuepos == UINT64_MAX) {
            return add(key, keylen, hashvalue, data, datalen);
        } else {
            return m_valuetable->appendToEntry(valuepos, data, datalen);
        }
    }

    bool MutableHash::add(const void *key, uint32_t keylen, uint64_t hashvalue, const void *data, uint32_t datalen)
    {
        uint64_t valuepos = m_valuetable->addEntry(data, datalen);

        uint8_t keydata_stack[1024];
        uint8_t *keydata;
        if (keylen + sizeof(struct KeyChunkHeader) < sizeof(keydata_stack)) {
            keydata = keydata_stack;
        } else {
            keydata = (uint8_t *)malloc(keylen + sizeof(struct KeyChunkHeader));
            if (keydata == NULL) {
                return false;
            }
        }

        struct KeyChunkHeader header;
        header.valuepos = valuepos;
        header.keylen = keylen;
        memcpy(keydata, &header, sizeof(header));
        memcpy(keydata+sizeof(header), key, keylen);

        
        if (m_hashTable[hashvalue] != UINT64_MAX) {
            bool succuss = m_keytable->appendToEntry(m_hashTable[hashvalue], keydata, keylen + sizeof(struct KeyChunkHeader));
            if (!succuss) {
                if (keydata_stack != keydata) {
                    free(keydata);
                }
                return false;
            }
        } else {
            uint64_t keypos = m_keytable->addEntry(keydata, keylen + sizeof(struct KeyChunkHeader));
            m_hashTable[hashvalue] = keypos;
        }

        if (keydata_stack != keydata) {
            free(keydata);
        }
        return true;
    }
    
    bool MutableHash::contains(const void *key, uint32_t keylen)
    {
        uint64_t hashvalue = hashvalue4key(key, keylen);
        return valuePosition(key, keylen, hashvalue) != UINT64_MAX;
    }

    uint64_t MutableHash::valuePosition(const void *key, uint32_t keylen)
    {
        uint64_t hashvalue = hashvalue4key(key, keylen);
        return valuePosition(key, keylen, hashvalue);
    }

    uint64_t MutableHash::valuePosition(const void *key, uint32_t keylen, uint64_t hashvalue)
    {
        DEBUG("Calling valuePostion: %s %u %llx", key, keylen, hashvalue);
        uint64_t position = m_hashTable[hashvalue];
        if (position == UINT64_MAX) {
            DEBUG("Not found in hash table");
            return UINT64_MAX;
        }
        if (debugMode) {
            DEBUG("Position: %llx", position);
            //m_keytable->debugPrint();
            m_valuetable->debugPrint();
        }
        
        uint32_t length;
        uint8_t *value = (uint8_t *)m_keytable->getEntry(position, &length);
        uint8_t *start = value;
        uint8_t *end = value + length;
        uint64_t returnValue = UINT64_MAX;

        if (value == NULL) {
            DEBUG("valuePosition: failed");
            return UINT64_MAX;
        }

        do {
            struct KeyChunkHeader chunkHeader;
            memcpy(&chunkHeader, value, sizeof(chunkHeader));
            value += sizeof(chunkHeader);
            if (keylen == chunkHeader.keylen) {
                if (memcmp(key, value, keylen) == 0) {
                    returnValue = chunkHeader.valuepos;
                    goto cleanup;
                }
            }
            value += chunkHeader.keylen;
        } while (value < end);
        DEBUG("Not Found");
    cleanup:
        free(start);
        DEBUG("valuePosition: 0x%llx", returnValue);
        return returnValue;
    }

    void *MutableHash::get(const void *key, uint32_t keylen, uint32_t *datalen)
    {
        uint64_t valpos = valuePosition(key, keylen);
        if (valpos == UINT64_MAX)
            return NULL;
        void* data = m_valuetable->getEntry(valpos, datalen);
        if (data == NULL) {
            fprintf(stderr, "Error: %s %d\n", m_valuetable->errorMessage(), m_valuetable->error());
        }
        return data;
    }

    uint64_t MutableHash::hashvalue4key(const void *key, uint32_t length)
    {
        const uint32_t SEED_VALUE = 276292924;
        uint64_t hashout[2];
        MurmurHash3_x64_128(key, length, SEED_VALUE, hashout);
        return hashout[0] & m_hashMask;
    }
}
