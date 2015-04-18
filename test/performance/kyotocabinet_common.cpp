#include "kyotocabinet_common.hpp"
#include "performance_common.hpp"

bool kyotocabinet_builddb(kyotocabinet::PolyDB *db, std::vector<const char *> *keylist) {
    std::auto_ptr<TSVPairLoader> loader(openFixtureData());
    const char *key, *value;
    while (loader->next(&key, &value)) {
        keylist->push_back(strdup(key));
        db->set(key, value);
    }
    return true;
}

int kyotocabinet_common(const char *filename) {
    char buf[PATH_MAX];
    snprintf(buf, sizeof(buf)-1, "rm %s", filename);
    system(buf);
    
    kyotocabinet::PolyDB db, db2;
    if (!db.open(filename)) {fprintf(stderr, "Cannot open kyotocabinet database\n");return 1;}
    std::vector<const char *> keylist;

    fprintf(stderr, "Building DB for %s\n", filename);
    
    if (!kyotocabinet_builddb(&db, &keylist)) {
        fprintf(stderr, "Cannot build database\n");
        return 1;
    }
    db.close();
    
    if (!db2.open(filename)) {fprintf(stderr, "Cannot reopen kyotocabinet database\n");return 1;}

    fprintf(stderr, "Looking up DB for %s\n", filename);
    size_t indexesLength = db2.count()*LOOKUP_KEY_SIZE_FACTOR;
    uint64_t *indexes = prepareRandomIndex(indexesLength, db2.count());

    snprintf(buf, sizeof(buf)-1, "Kyoto Cabinet\t%s", filename);
    PerformanceTest pt(buf);
    pt.start();
    for (uint64_t i = 0; i < indexesLength; i++) {
        size_t valueSize;
        const char* value = db.get(keylist[indexes[i]], strlen(keylist[indexes[i]]), &valueSize);
        delete value;
    }
    pt.end();
    return 0;
}
