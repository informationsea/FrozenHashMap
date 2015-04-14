#include "cfhm.h"
#include "frozenhash.hpp"
#include "frozenhashbuilder.hpp"

struct CFrozenHashMapBuilder {
    class FrozenMapBuilder builder;
};

struct CFrozenHashMap {
    class FrozenMap map;
};

struct CFrozenHashMapCursor {
    class FrozenMapCursor *cur;
};

struct CFrozenHashMapBuilder* CFrozenHashMapBuilderAllocate(bool inmemory)
{
    return new CFrozenHashMapBuilder;
}


bool CFrozenHashMapBuilderOpen(struct CFrozenHashMapBuilder *builder)
{
    return builder->builder.open();
}

bool CFrozenHashMapBuilderPutString(struct CFrozenHashMapBuilder *builder, const char *key, const char *value)
{
    return builder->builder.put(key, value);
}

bool CFrozenHashMapBuilderPut(struct CFrozenHashMapBuilder *builder, const char *key, size_t keylen, const char *value, size_t valuelen)
{
    return builder->builder.put(key, keylen, value, valuelen);
}

bool CFrozenHashMapBuilderBuild(struct CFrozenHashMapBuilder *builder, const char *filename)
{
    return builder->builder.build(filename);
}

void CFrozenHashMapBuilderFree(struct CFrozenHashMapBuilder *builder)
{
    delete builder;
}
    
struct CFrozenHashMap* CFrozenHashMapAllocate()
{
    return new CFrozenHashMap;
}

bool CFrozenHashMapOpen(CFrozenHashMap* map, const char *filename)
{
    return map->map.open(filename);
}

bool CFrozenHashMapOpenWithOffset(CFrozenHashMap* map, const char *filename, off_t offset)
{
    return map->map.open(filename, offset);
}

const char *CFrozenHashMapGet(CFrozenHashMap* map, const char *key, size_t keysp, size_t *valuesp)
{
    return map->map.get(key, keysp, valuesp);
}

uint64_t CFrozenHashMapCount(struct CFrozenHashMap* map)
{
    return map->map.count();
}

void CFrozenHashMapFree(CFrozenHashMap* map)
{
    delete map;
}

struct CFrozenHashMapCursor* CFrozenHashMapCursorCreate(struct CFrozenHashMap* map)
{
    struct CFrozenHashMapCursor* cursor = new CFrozenHashMapCursor;
    cursor->cur = new FrozenMapCursor(&(map->map));
    return cursor;
}


bool CFrozenHashMapCursorNext(struct CFrozenHashMapCursor* cursor, const char **key, size_t *keylen, const char **value, size_t *valuelen)
{
    return cursor->cur->next(key, keylen, value, valuelen);
}

