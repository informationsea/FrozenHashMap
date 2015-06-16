#ifndef FROZENHASHBUILDER_H
#define FROZENHASHBUILDER_H

#include <stdint.h>
#include <string>
#include <functional>
#include <vector>
#include "mutablehash.hpp"

class FrozenMapBuilder {
public:
    FrozenMapBuilder(bool ainmemory = false);
    virtual ~FrozenMapBuilder();

    bool open(); // call this function first
    
    bool put(const std::string &key, const std::string &value);
    bool put(const char *key, size_t keylen, const char *value, size_t valuelen);
    
    bool build(int fd);
    bool build(const char *filename);
private:
    bool inmemory;
    frozenhashmap::MutableHash hash2key;
    frozenhashmap::MutableHash data;
    char tempdir[PATH_MAX];
    bool ready;

    char hash2key_path[PATH_MAX];
    char data_path[PATH_MAX];
    char hashtable_path[PATH_MAX];
    char valuetable_path[PATH_MAX];
};


#endif /* FROZENHASHBUILDER_H */
