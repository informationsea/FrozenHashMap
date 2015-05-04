#ifndef MUTABLEHASH_H
#define MUTABLEHASH_H

#include <vector>
#include <limits.h>
#include <stdint.h>

namespace frozenhashmap {
    class MutableValueTable;

    /**
     * Mutable Hash Map
     * @warning This class is not thread safe and re-entrant
     */
    class MutableHash {
    public:
        MutableHash(uint64_t expectedCount = 1024*1024*5);
        virtual ~MutableHash();

        bool open(const char *dir = 0);

        bool set(const void *key, uint32_t keylen, const void *data, uint32_t datalen);
        bool append(const void *key, uint32_t keylen, const void *data, uint32_t datalen);
        bool contains(const void *key, uint32_t keylen);

        /**
         * Get a data corresponding to a key
         * @warning return data must to be free after using
         */
        void *get(const void *key, uint32_t keylen, uint32_t *datalen);
        
        uint64_t count() {return m_count;}

    private:
        int m_hashMask;
        uint64_t m_count;

        uint64_t *m_hashTable;

        char m_dir_path[PATH_MAX];
        char m_keytable_path[PATH_MAX];
        MutableValueTable *m_keytable;
        char m_valuetable_path[PATH_MAX];
        MutableValueTable *m_valuetable;

        uint64_t hashvalue4key(const void *key, uint32_t length);
        uint64_t valuePosition(const void *key, uint32_t keylen);
        uint64_t valuePosition(const void *key, uint32_t keylen, uint64_t hashvalue);

        /**
         * Add data to this hash map
         * @warning This function do not check existence of key
         */
        bool add(const void *key, uint32_t keylen, uint64_t hashvalue, const void *data, uint32_t datalen);
    };

}

#endif /* MUTABLEHASH_H */
