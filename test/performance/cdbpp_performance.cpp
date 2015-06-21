#include "performance_common.hpp"
#include "cdbpp.h"
#include <iostream>
#include <vector>
#include <stdlib.h>

#define DBNAME "cdbpp.cdb"
#define DBPATH (DBDIR "/" DBNAME)

bool build(std::vector<const char *> *keylist) {
    std::ofstream ofs(DBPATH, std::ios_base::binary);
    if (ofs.fail()) {
        std::cerr << "Cannot open DB" << std::endl;
        return false;
    }

    try {
        std::auto_ptr<TSVPairLoader> loader(openFixtureData());
        cdbpp::builder dbw(ofs);

        const char *key, *value;
        while (loader->next(&key, &value)) {
            keylist->push_back(strdup(key));
            dbw.put(key, strlen(key), value, strlen(value));
        }
    } catch (const cdbpp::builder_exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return false;
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

    std::ifstream ifs(DBPATH, std::ios_base::binary);
    
    try {
        cdbpp::cdbpp dbr(ifs);
        if (!dbr.is_open()) {
            std::cerr << "ERROR: Failed to read a database file." << std::endl;
            return false;
        }

        size_t indexesLength = keylist.size()*LOOKUP_KEY_SIZE_FACTOR;
        uint64_t *indexes = prepareRandomIndex(indexesLength, keylist.size());

        PerformanceTest pt("CDB++");
        pt.start();
        for (uint64_t i = 0; i < indexesLength; i++) {
            size_t valueSize;
            const char* value = (const char*)dbr.get(keylist[indexes[i]], strlen(keylist[indexes[i]]), &valueSize);
            if (value == NULL) {
                std::cerr << "No value is found for key " << keylist[indexes[i]] << std::endl;
            }
        }
        pt.end();

    } catch (const cdbpp::cdbpp_exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return false;
    }

   
    return 0;
}
