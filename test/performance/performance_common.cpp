#include "performance_common.hpp"

TSVPairLoader::TSVPairLoader(const char *filename) : ready(false) {
    file = fopen(filename, "r");
    if (file)
        ready = true;
}

TSVPairLoader::~TSVPairLoader() {
    if (ready)
        fclose(file);
}

bool TSVPairLoader::isReady() {
    return ready;
}

bool TSVPairLoader::next(const char **key, const char **value) {
    if (fgets(linebuf, sizeof(linebuf)-1, file) == NULL) {
        return false;
    }
    if (strlen(linebuf) == 0) // skip empty line
        return next(key, value); 
        
    char *sep = strchr(linebuf, '\t');
    *sep = 0;
    char *end = strchr(sep+1, '\n');
    *key = linebuf;
    *value = sep+1;
    return true;
}


TSVPairLoader *openFixtureData() {
    TSVPairLoader* loader = new TSVPairLoader("gene_info_summary.txt");
    if (loader->isReady())
        return loader;
    delete loader;
    loader = new TSVPairLoader("gene_info_summary_head.txt");
    if (loader->isReady())
        return loader;
    delete loader;
    return NULL;
}

uint64_t* prepareRandomIndex(size_t length, size_t max) {
    sfmt_t sfmt;
    sfmt_init_gen_rand(&sfmt, 849544561);

    uint64_t* values = (uint64_t *)malloc(sizeof(uint64_t)*length);
    sfmt_fill_array64(&sfmt, values, length);

    for (size_t i = 0; i < length; i++) {
        values[i] = values[i] % max;
    }
    return values;
}

