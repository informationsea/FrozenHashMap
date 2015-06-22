#include "test_common.hpp"
#include <frozenhash.hpp>
#include <frozenhashbuilder.hpp>
#include <valuetable.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <map>

namespace {

    using namespace frozenhashmap;

    void before(void) {
        const char *env[] =
            {//"FROZENHASH_DEBUG=1",
             "TMPDIR=./tmp"};

        for (size_t i = 0; i < sizeof(env)/sizeof(env[0]); i++) {
            char *buf = (char *)malloc(strlen(env[i]+1));
            strcpy(buf, env[i]);
            putenv(buf);
        }

        ASSERT_EQ(0, system("mkdir -p ./tmp"));
    }

    TEST(FROZENHASH, VALUETABLE) {
        ASSERT_TRUE(system("mkdir -p ./tmp") == 0);
        FILE *valuetable = fopen("./tmp/valuetable.dat", "w+");
        ASSERT_TRUE(valuetable != NULL);
        ValueTableWriter writer(valuetable);
        ASSERT_TRUE(writer.write("OK", 2));
        ASSERT_EQ(4*2, writer.tell());
        ASSERT_TRUE(writer.write("12345678"));
        ASSERT_EQ(4*(4+2), writer.tell());
        ASSERT_TRUE(writer.write("123456789"));
        ASSERT_EQ(4*(4+6), writer.tell());

        fclose(valuetable);

        int fd = open("./tmp/valuetable.dat", O_RDONLY);
        ValueTableReader reader(fd);

        size_t length;
        const char *data = reader.readNext(&length);
        ASSERT_MEMEQ("OK", 2, data, length);
        ASSERT_EQ('\0', *(data+length));
        data = reader.readNext(&length);
        ASSERT_MEMEQ("12345678", 8, data, length);
        ASSERT_EQ('\0', *(data+length));
        data = reader.readNext(&length);
        ASSERT_MEMEQ("123456789", 9, data, length);
        ASSERT_EQ('\0', *(data+length));

        uint32_t len32;
        off_t pos = 0;
        off_t next = 0;
        data = reader.readAt(pos, &len32, &next);
        ASSERT_MEMEQ("OK", 2, data, len32);
        ASSERT_EQ('\0', *(data+len32));
        ASSERT_EQ(8, next);

        pos = next;
        data = reader.readAt(pos, &len32, &next);
        ASSERT_MEMEQ("12345678", 8, data, len32);
        ASSERT_EQ('\0', *(data+len32));
        ASSERT_EQ(4*(4+2), next);

        pos = next;
        data = reader.readAt(pos, &len32, &next);
        ASSERT_MEMEQ("123456789", 9, data, len32);
        ASSERT_EQ('\0', *(data+len32));
        
        unlink("./tmp/valuetable.dat");
    }


    TEST(FROZENHASH, BUILDER) {
        before();
        
        FrozenMapBuilder builder;
        ASSERT_TRUE(builder.open());
        ASSERT_TRUE(builder.put("Hi", "OK-Hi"));
        ASSERT_TRUE(builder.put("Echo", "OK-Echo"));
        ASSERT_TRUE(builder.put("Push", 4, "OK-Push", 7));
        ASSERT_TRUE(builder.build("./tmp/dbfile.dat"));

        FrozenMap map;
        ASSERT_TRUE(map.open(("./tmp/dbfile.dat")));

        size_t length;
        const char *data;
        data = map.get("Hi", 2, &length);
        ASSERT_TRUE(data);
        ASSERT_MEMEQ("OK-Hi", 5, data, length);
        
        data = map.get("Echo", 4, &length);
        ASSERT_TRUE(data);
        ASSERT_MEMEQ("OK-Echo", 7, data, length);
        
        data = map.get("Push", 4, &length);
        ASSERT_TRUE(data);
        ASSERT_MEMEQ("OK-Push", 7, data, length);

        data = map.get("Unknown", 7, &length);
        ASSERT_FALSE(data);
        
        ASSERT_TRUE(3 == map.count());

        FrozenMapCursor cursor(&map);
        std::map<std::string, std::string> tmp;
        std::pair<std::string, std::string> pair;
        while (cursor.nextString(&pair)) {
            tmp[pair.first] = pair.second;
        }
        ASSERT_STREQ("OK-Push", tmp["Push"].c_str());
        ASSERT_STREQ("OK-Hi", tmp["Hi"].c_str());
        ASSERT_STREQ("OK-Echo", tmp["Echo"].c_str());
        ASSERT_TRUE(3 == tmp.size());
    }

    TEST(FROZENHASH, BUILDER2) {
        before();
        
        FrozenMapBuilder builder(false);
        ASSERT_TRUE(builder.open());

        FILE *testdata = fopen("./geneid-symbol.txt", "r");
        ASSERT_TRUE(testdata);

        char linebuf[256];
        bzero(linebuf, sizeof(linebuf));

        while (fgets(linebuf, sizeof(linebuf), testdata) != NULL) {
            char *p = strchr(linebuf, '|');
            if (p == NULL) continue;
            *p = '\0';
            p++;
            char *e = strchr(p, '\n');
            *e = '\0';

            ASSERT_TRUE(builder.put(linebuf, p));
            bzero(linebuf, sizeof(linebuf));
        }
        
        ASSERT_TRUE(builder.build("./tmp/dbfile2.dat"));
        fseek(testdata, 0, SEEK_SET);

        FrozenMap map;
        ASSERT_TRUE(map.open(("./tmp/dbfile2.dat")));
        ASSERT_TRUE(47734 == map.count());

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
            if (data == NULL) {
                fprintf(stderr, "Cannot obtain value for %s (expected %s)\n", linebuf, p);
            }
            ASSERT_TRUE(data);//, cut_message("Cannot obtain value for %s (expected %s)", linebuf, p));
            ASSERT_MEMEQ(p, strlen(p), data, length);
            bzero(linebuf, sizeof(linebuf));
        }
        fclose(testdata);
    }

}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
