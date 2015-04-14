#include <cppcutter.h>
#include <frozenhash.hpp>
#include <frozenhashbuilder.hpp>
#include <valuetable.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <map>

namespace frozenhashbuilder {

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

    void test_valuetable_builder(void) {
        cut_assert(system("mkdir -p ./tmp") == 0);
        FILE *valuetable = fopen("./tmp/valuetable.dat", "w+");
        cut_assert(valuetable != NULL);
        ValueTableWriter writer(valuetable);
        cut_assert(writer.write("OK", 2));
        cut_assert_equal_int(4*2, writer.tell());
        cut_assert(writer.write("12345678"));
        cut_assert_equal_int(4*(4+2), writer.tell());
        cut_assert(writer.write("123456789"));
        cut_assert_equal_int(4*(4+6), writer.tell());

        fclose(valuetable);

        int fd = open("./tmp/valuetable.dat", O_RDONLY);
        ValueTableReader reader(fd);

        size_t length;
        const char *data = reader.readNext(&length);
        cut_assert_equal_memory("OK", 2, data, length);
        cut_assert_equal_char('\0', *(data+length));
        data = reader.readNext(&length);
        cut_assert_equal_memory("12345678", 8, data, length);
        cut_assert_equal_char('\0', *(data+length));
        data = reader.readNext(&length);
        cut_assert_equal_memory("123456789", 9, data, length);
        cut_assert_equal_char('\0', *(data+length));
        
        unlink("./tmp/valuetable.dat");
    }
    
    void test_frozenhash_builder(void){
        before();
        
        FrozenMapBuilder builder;
        cut_assert(builder.open());
        cut_assert(builder.put("Hi", "OK-Hi"));
        cut_assert(builder.put("Echo", "OK-Echo"));
        cut_assert(builder.put("Push", 4, "OK-Push", 7));
        cut_assert(builder.build("./tmp/dbfile.dat"));

        FrozenMap map;
        cut_assert(map.open(("./tmp/dbfile.dat")));

        size_t length;
        const char *data;
        data = map.get("Hi", 2, &length);
        cut_assert(data);
        cut_assert_equal_memory("OK-Hi", 5, data, length);
        
        data = map.get("Echo", 4, &length);
        cut_assert(data);
        cut_assert_equal_memory("OK-Echo", 7, data, length);
        
        data = map.get("Push", 4, &length);
        cut_assert(data);
        cut_assert_equal_memory("OK-Push", 7, data, length);

        FrozenMapCursor cursor(&map);
        std::map<std::string, std::string> tmp;
        std::pair<std::string, std::string> pair;
        while (cursor.nextString(&pair)) {
            tmp[pair.first] = pair.second;
        }
        cut_assert_equal_string("OK-Push", tmp["Push"].c_str());
        cut_assert_equal_string("OK-Hi", tmp["Hi"].c_str());
        cut_assert_equal_string("OK-Echo", tmp["Echo"].c_str());
        cut_assert_equal_int(3, tmp.size());
    }

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

}
