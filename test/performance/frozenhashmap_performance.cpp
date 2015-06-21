#include "performance_common.hpp"
#include <frozenhash.hpp>
#include <frozenhashbuilder.hpp>
#include <vector>
#include <string.h>

#define DBNAME "frozen.dat"
#define DBPATH (DBDIR "/" DBNAME)

bool build(std::vector<const char *> *keylist)
{
    FrozenMapBuilder builder;
    if (!builder.open()) {
        fprintf(stderr, "Cannot open database\n");
        return false;
    }

    std::auto_ptr<TSVPairLoader> loader(openFixtureData());
    const char *key, *value;
    while (loader->next(&key, &value)) {
        keylist->push_back(strdup(key));
        builder.put(key, value);
    }
    if (!builder.build(DBPATH)) {
        fprintf(stderr, "Cannot build database\n");
    }
    return true;
}

int main(void)
{
    if (system("mkdir -p " DBDIR) != 0) return 1;
    std::vector<const char *> keylist;

    fprintf(stderr, "Building DB for %s\n", DBPATH);
    build(&keylist);
    fprintf(stderr, "Looking up for %s\n", DBPATH);

    size_t indexesLength = keylist.size()*LOOKUP_KEY_SIZE_FACTOR;
    uint64_t *indexes = prepareRandomIndex(indexesLength, keylist.size());

    FrozenMap map;
    if (!map.open(DBPATH)) {
        fprintf(stderr, "Cannot open database\n");
        return 1;
    }

    PerformanceTest pt("Frozen Hash Map");
    pt.start();

    for (uint64_t i = 0; i < indexesLength; i++) {
        size_t valueSize;
        const char* value = (const char*)map.get(keylist[indexes[i]], strlen(keylist[indexes[i]]), &valueSize);
        if (value == NULL) {
            fprintf(stderr, "No value found");
            return 1;
        }
    }

    pt.end();
    
    return 0;
}

