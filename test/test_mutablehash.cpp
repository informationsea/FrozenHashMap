#include "test_common.hpp"
#include <stdlib.h>
#include <map>
#include <string>
#include <stdio.h>

#include "mutablehash.hpp"

namespace mutablehash_test {
    using namespace frozenhashmap;

    TEST(MUTABLEHASH, MUTABLEHASH1) {

        MutableHash mutable_hash;
        ASSERT_TRUE(mutable_hash.open());
        ASSERT_TRUE(mutable_hash.set("Hello", 5, "OK", 2));
        ASSERT_TRUE(mutable_hash.set("World", 5, "!", 1));
        ASSERT_TRUE(mutable_hash.contains("World", 5));
        ASSERT_TRUE(mutable_hash.contains("Hello", 5));
        ASSERT_FALSE(mutable_hash.contains("Fuji", 4));
        ASSERT_FALSE(mutable_hash.contains("Three", 5));

        uint32_t size;
        void *data;

        data = mutable_hash.get("Hello", 5, &size);
        ASSERT_TRUE(data);
        ASSERT_MEMEQ("OK", 2, data, size);
        free(data);

        data = mutable_hash.get("World", 5, &size);
        ASSERT_TRUE(data);
        ASSERT_MEMEQ("!", 1, data, size);
        free(data);

        data = mutable_hash.get("Three", 5, &size);
        ASSERT_FALSE(data);

        ASSERT_TRUE(mutable_hash.append("World", 5, "?!?!", 4));
        data = mutable_hash.get("World", 5, &size);
        ASSERT_TRUE(data);
        ASSERT_MEMEQ("!?!?!", 5, data, size);
        free(data);

        ASSERT_TRUE(mutable_hash.set("World", 5, "#@#@", 4));
        data = mutable_hash.get("World", 5, &size);
        ASSERT_TRUE(data);
        ASSERT_MEMEQ("#@#@", 4, data, size);
        free(data);

        ASSERT_TRUE(2 == mutable_hash.count());

        // ----
        
        ASSERT_TRUE(mutable_hash.append("Fiji", 4, "Island", 6));
        
        ASSERT_TRUE(3 == mutable_hash.count());

        data = mutable_hash.get("Fiji", 4, &size);
        ASSERT_TRUE(data);
        ASSERT_MEMEQ("Island", 6, data, size);
        free(data);

    }

    TEST(MUTABLEHASH, CURSOR) {
        MutableHash mutable_hash;
        ASSERT_TRUE(mutable_hash.open());
        ASSERT_TRUE(mutable_hash.set("Hello", 5, "OK", 2));
        ASSERT_TRUE(mutable_hash.set("World", 5, "!", 1));
        ASSERT_TRUE(mutable_hash.set("foo", 3, "hoge", 4));
        ASSERT_TRUE(mutable_hash.set("hoge", 4, "foo", 3));

        
        std::map<std::string, std::string> found;
        MutableHashCursor cursor(&mutable_hash);
        while(cursor.next()) {
            char *key, *data;
            size_t keylen, datalen;
            ASSERT_TRUE(cursor.get(&key, &keylen, &data, &datalen));
            ASSERT_TRUE(0 == found.count(key));
            found[key] = std::string(data, datalen);
        }

        ASSERT_TRUE(4 == found.size());
        
        ASSERT_TRUE(1 == found.count("Hello"));
        ASSERT_TRUE(1 == found.count("World"));
        ASSERT_TRUE(1 == found.count("foo"));
        ASSERT_TRUE(1 == found.count("hoge"));
        
        ASSERT_STREQ("OK", found["Hello"].c_str());
        ASSERT_STREQ("!", found["World"].c_str());
        ASSERT_STREQ("foo", found["hoge"].c_str());
        ASSERT_STREQ("hoge", found["foo"].c_str());
    }

    TEST(MUTABLEHASH, CURSOR_KEY) {
        MutableHash mutable_hash;
        ASSERT_TRUE(mutable_hash.open());
        ASSERT_TRUE(mutable_hash.set("Hello", 5, "OK", 2));
        ASSERT_TRUE(mutable_hash.set("World", 5, "!", 1));
        ASSERT_TRUE(mutable_hash.set("foo", 3, "hoge", 4));
        ASSERT_TRUE(mutable_hash.set("hoge", 4, "foo", 3));

        //fprintf(stderr, "Initialized hashmap\n");
        
        std::map<std::string, std::string> found;
        MutableHashCursor cursor(&mutable_hash);
        //fprintf(stderr, "Initialized cursor\n");
        while(cursor.next()) {
            size_t keylen;
            char *key = cursor.getKey(&keylen);
            ASSERT_EQ(strlen(key), keylen);
            ASSERT_TRUE(0 == found.count(key));
            found[key] = "found";
        }

        ASSERT_TRUE(4 == found.size());
    }

    TEST(MUTABLEHASH, MANY_DATA) {
        MutableHash mutable_hash;
        ASSERT_TRUE(mutable_hash.open());
        
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

            ASSERT_TRUE(mutable_hash.set(linebuf, strlen(linebuf), p, strlen(p)));
            bzero(linebuf, sizeof(linebuf));
        }
        
        fseek(testdata, 0, SEEK_SET);

        ASSERT_TRUE(47734 == mutable_hash.count());

        while (fgets(linebuf, sizeof(linebuf), testdata) != NULL) {
            char *p = strchr(linebuf, '|');
            if (p == NULL) continue;
            *p = '\0';
            p++;
            char *e = strchr(p, '\n');
            *e = '\0';

            uint32_t length;
            char *data = (char *)mutable_hash.get(linebuf, strlen(linebuf), &length);
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
