#include <cutter.h>
#include <cfhm.h>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void before(void) {
    const char *env[] =
        {"FROZENHASH_DEBUG=1",
         "TMPDIR=./tmp"};

    size_t i;
    for (i = 0; i < sizeof(env)/sizeof(env[0]); i++) {
        char *buf = (char *)malloc(strlen(env[i]+1));
        strcpy(buf, env[i]);
        putenv(buf);
    }
    cut_assert(system("mkdir -p ./tmp") == 0);
}

void test_frozenhash_builder(void){

    struct CFrozenHashMapBuilder *builder = CFrozenHashMapBuilderAllocate(false);
    cut_assert(builder);
    cut_assert(CFrozenHashMapBuilderOpen(builder));
    cut_assert(CFrozenHashMapBuilderPutString(builder, "Hi", "OK-Hi"));
    cut_assert(CFrozenHashMapBuilderPutString(builder, "Echo", "OK-Echo"));
    cut_assert(CFrozenHashMapBuilderPut(builder, "Push", 4, "OK-Push", 7));
    cut_assert(CFrozenHashMapBuilderBuild(builder, "./tmp/dbfile.dat"));
    CFrozenHashMapBuilderFree(builder);

    struct CFrozenHashMap *map = CFrozenHashMapAllocate();
    cut_assert(map);

    cut_assert(CFrozenHashMapOpen(map, "./tmp/dbfile.dat"));

    cut_assert_equal_int(3, CFrozenHashMapCount(map));

    size_t length;
    const char *data;
    data = CFrozenHashMapGet(map, "Hi", 2, &length);
    cut_assert(data);
    cut_assert_equal_memory("OK-Hi", 5, data, length);
        
    data = CFrozenHashMapGet(map, "Echo", 4, &length);
    cut_assert(data);
    cut_assert_equal_memory("OK-Echo", 7, data, length);
        
    data = CFrozenHashMapGet(map, "Push", 4, &length);
    cut_assert(data);
    cut_assert_equal_memory("OK-Push", 7, data, length);

    CFrozenHashMapFree(map);
}

