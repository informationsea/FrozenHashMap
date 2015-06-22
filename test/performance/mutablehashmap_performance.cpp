#include "performance_common.hpp"
#include <mutablehash.hpp>
#include <vector>
#include <string.h>

using namespace frozenhashmap;

int main(void)
{
    if (system("mkdir -p " DBDIR) != 0) return 1;
    std::vector<const char *> keylist;

    fprintf(stderr, "Building DB\n");

    MutableHash map;
    if (!map.open()) {
        fprintf(stderr, "Cannot open database\n");
        return false;
    }

    std::auto_ptr<TSVPairLoader> loader(openFixtureData());
    const char *key, *value;
    while (loader->next(&key, &value)) {
        keylist.push_back(strdup(key));
        map.set(key, strlen(key), value, strlen(value));
    }

    fprintf(stderr, "Loaded keys : %llu  # of keys %zu\n", map.count(), keylist.size());
    fprintf(stderr, "Looking up\n");

    size_t indexesLength = keylist.size()*LOOKUP_KEY_SIZE_FACTOR;
    uint64_t *indexes = prepareRandomIndex(indexesLength, keylist.size());

    PerformanceTest pt("Mutable Map");
    pt.start();

    for (uint64_t i = 0; i < indexesLength; i++) {
        uint32_t valueSize;
        const char* value = (const char*)map.get(keylist[indexes[i]], strlen(keylist[indexes[i]]), &valueSize);
        if (value == NULL) {
            fprintf(stderr, "No value found");
            return 1;
        }
    }

    pt.end();
    
    return 0;
}

