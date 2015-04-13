#ifndef CFHM_H
#define CFHM_H

#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

    struct CFrozenHashMapBuilder;
    struct CFrozenHashMapBuilder* CFrozenHashMapBuilderAllocate(bool inmemory);
    bool CFrozenHashMapBuilderOpen(struct CFrozenHashMapBuilder *builder);
    bool CFrozenHashMapBuilderPutString(struct CFrozenHashMapBuilder *builder, const char *key, const char *value);
    bool CFrozenHashMapBuilderBuild(struct CFrozenHashMapBuilder *builder, const char *filename);
    void CFrozenHashMapBuilderFree(struct CFrozenHashMapBuilder *builder);
    
    struct CFrozenHashMap;
    struct CFrozenHashMap* CFrozenHashMapAllocate();
    bool CFrozenHashMapOpen(struct CFrozenHashMap* map, const char *filename);
    bool CFrozenHashMapOpenWithOffset(struct CFrozenHashMap* map, const char *filename, off_t offset);
    const char *CFrozenHashMapGet(struct CFrozenHashMap* map, const char *key, size_t keysp, size_t *valuesp);
    void CFrozenHashMapFree(struct CFrozenHashMap* map);
    
#ifdef __cplusplus
}
#endif

#endif /* CFHM_H */
