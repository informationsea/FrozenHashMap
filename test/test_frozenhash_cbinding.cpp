#include "test_common.hpp"
#include <cfhm.h>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace {
    TEST(FROZENHASH_CBINDING, CBINDING) {
        ASSERT_EQ(0, system("mkdir -p ./tmp"));

        struct CFrozenHashMapBuilder *builder = CFrozenHashMapBuilderAllocate(false);
        ASSERT_TRUE(builder != NULL);
        ASSERT_TRUE(CFrozenHashMapBuilderOpen(builder));
        ASSERT_TRUE(CFrozenHashMapBuilderPutString(builder, "Hi", "OK-Hi"));
        ASSERT_TRUE(CFrozenHashMapBuilderPutString(builder, "Echo", "OK-Echo"));
        ASSERT_TRUE(CFrozenHashMapBuilderPut(builder, "Push", 4, "OK-Push", 7));
        ASSERT_TRUE(CFrozenHashMapBuilderBuild(builder, "./tmp/dbfile.dat"));
        CFrozenHashMapBuilderFree(builder);

        struct CFrozenHashMap *map = CFrozenHashMapAllocate();
        ASSERT_TRUE(map != NULL);

        ASSERT_TRUE(CFrozenHashMapOpen(map, "./tmp/dbfile.dat"));

        ASSERT_TRUE(3 == CFrozenHashMapCount(map));

        size_t length;
        const char *data;
        data = CFrozenHashMapGet(map, "Hi", 2, &length);
        ASSERT_TRUE(data);
        ASSERT_MEMEQ("OK-Hi", 5, data, length);
        
        data = CFrozenHashMapGet(map, "Echo", 4, &length);
        ASSERT_TRUE(data);
        ASSERT_MEMEQ("OK-Echo", 7, data, length);
        
        data = CFrozenHashMapGet(map, "Push", 4, &length);
        ASSERT_TRUE(data);
        ASSERT_MEMEQ("OK-Push", 7, data, length);

        CFrozenHashMapFree(map);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
