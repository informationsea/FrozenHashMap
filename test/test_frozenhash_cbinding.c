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

    for (size_t i = 0; i < sizeof(env)/sizeof(env[0]); i++) {
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
    cut_assert(CFrozenHashMapBuilderPutString(builder, "Push", "OK-Push"));
    cut_assert(CFrozenHashMapBuilderBuild(builder, "./tmp/dbfile.dat"));
    CFrozenHashMapBuilderFree(builder);

    struct CFrozenHashMap *map = CFrozenHashMapAllocate();
    cut_assert(map);

    cut_assert(CFrozenHashMapOpen(map, "./tmp/dbfile.dat"));

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

/*

void test_frozenhash_builder2(void){
    before();
        
    FrozenMapBuilder builder(false);
    cut_assert(builder.open());

    FILE *testdata = fopen("./geneid-symbol.txt", "r");
    cut_assert(testdata);

    char linebuf[256];
    bzero(linebuf, sizeof(linebuf));

    while (fgets(linebuf, sizeof(linebuf), testdata) != NULL) {
        char *p = strchr(linebuf, '|');
        if (p == NULL) continue;
        *p = '\0';
        p++;
        char *e = strchr(p, '\n');
        *e = '\0';

        cut_assert(builder.put(linebuf, p));
        bzero(linebuf, sizeof(linebuf));
    }
        
    cut_assert(builder.build("./tmp/dbfile2.dat"));
    fseek(testdata, 0, SEEK_SET);

    FrozenMap map;
    cut_assert(map.open(("./tmp/dbfile2.dat")));

    while (fgets(linebuf, sizeof(linebuf), testdata) != NULL) {
        char *p = strchr(linebuf, '|');
        if (p == NULL) continue;
        *p = '\0';
        p++;
        char *e = strchr(p, '\n');
        *e = '\0';

        size_t length;
        const char *data;
        data = map.get(linebuf, strlen(linebuf), &length);
        cut_assert(data, cut_message("Cannot obtain value for %s (expected %s)", linebuf, p));
        cut_assert_equal_memory(p, strlen(p), data, length);
        bzero(linebuf, sizeof(linebuf));
    }
    fclose(testdata);
}
*/
