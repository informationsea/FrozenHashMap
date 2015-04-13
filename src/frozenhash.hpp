#ifndef FROZENHASH_H
#define FROZENHASH_H

#include <stdint.h>
#include <string>
#include <functional>
#include <vector>

class ValueTableReader;
struct FrozenHashMapHeader;

class FrozenMap {
public:
    FrozenMap();
    virtual ~FrozenMap();
    bool open(const char *filename, off_t offset = 0);
    bool open(int fd, off_t offset = 0);
    
    const char *get(const char *key, size_t keysp, size_t *valuesp);
    std::string get(const std::string &key);
    
private:
    int m_fd;
    struct FrozenHashMapHeader *header;

    uint64_t *hashtable_map;
    void *valuetable_map;
    ValueTableReader *valuetable;
};

class FrozenMapCursor {
    
};

#endif /* FROZENHASH_H */




