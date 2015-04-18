#ifndef FROZENHASH_H
#define FROZENHASH_H

#include <stdint.h>
#include <string>
#include <functional>
#include <vector>
#include <utility>
#include <sys/mman.h>

class ValueTableReader;
struct FrozenHashMapHeader;
class FrozenMapCursor;

class FrozenMap {
    friend class FrozenMapCursor;
public:
    FrozenMap();
    virtual ~FrozenMap();
    bool open(const char *filename, off_t offset = 0);
    bool open(int fd, off_t offset = 0);
    
    const char *get(const char *key, size_t keysp, size_t *valuesp) const;
    std::string get(const std::string &key) const;
    uint64_t count() const;
    
private:
    int m_fd;
    struct FrozenHashMapHeader *header;

    uint64_t *hashtable_map;
    void *valuetable_map;
    ValueTableReader *valuetable;
};

class FrozenMapCursor {
public:
    FrozenMapCursor(FrozenMap *parent);
    virtual ~FrozenMapCursor();
    FrozenMap *db();
    bool nextString(std::pair<std::string, std::string> *pair);
    bool next(const char **key, size_t *keylen, const char **value, size_t *valuelen);
    
private:
    FrozenMap *m_parent;
    ValueTableReader *valuetable;
};

#endif /* FROZENHASH_H */
