#ifndef FROZENHASHBUILDER_H
#define FROZENHASHBUILDER_H

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <functional>
#include <vector>

namespace frozenhashmap {
    class ValueTableWriter;
    
    class FrozenMapBuilder {
    public:
        FrozenMapBuilder();
        virtual ~FrozenMapBuilder();

        bool open(); // call this function first
    
        bool put(const std::string &key, const std::string &value);
        bool put(const char *key, size_t keylen, const char *value, size_t valuelen);
    
        bool build(int fd, bool onmemory = false);
        bool build(const char *filename, bool onmemory = false);
    private:
        char tempdir[PATH_MAX];
        bool ready;

        char hash2position_path[PATH_MAX];
        char hashtable_path[PATH_MAX];
        char valuetable_path[PATH_MAX];
        
        FILE *hash2position_file;
        FILE *valuetable_file;
        ValueTableWriter *valuetable;
        uint64_t entryCount;
    };
}

#endif /* FROZENHASHBUILDER_H */
