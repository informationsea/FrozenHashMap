#include <cppcutter.h>
#include <stdlib.h>
#include <map>
#include <string>

#include "mutablehash.hpp"

namespace mutablehash_test {
    using namespace frozenhashmap;

    void test_mutablehash() {

        MutableHash mutable_hash;
        cut_assert_equal_int(true, mutable_hash.open());
        cut_assert_equal_int(true, mutable_hash.set("Hello", 5, "OK", 2));
        cut_assert_equal_int(true, mutable_hash.set("World", 5, "!", 1));
        cut_assert_equal_int(true, mutable_hash.contains("World", 5));
        cut_assert_equal_int(true, mutable_hash.contains("Hello", 5));
        cut_assert_equal_int(false, mutable_hash.contains("Fuji", 4));
        cut_assert_equal_int(false, mutable_hash.contains("Three", 5));

        uint32_t size;
        void *data;

        data = mutable_hash.get("Hello", 5, &size);
        cut_assert_not_null(data);
        cut_assert_equal_memory("OK", 2, data, size);
        free(data);

        data = mutable_hash.get("World", 5, &size);
        cut_assert_not_null(data);
        cut_assert_equal_memory("!", 1, data, size);
        free(data);

        data = mutable_hash.get("Three", 5, &size);
        cut_assert_null(data);

        cut_assert_true(mutable_hash.append("World", 5, "?!?!", 4));
        data = mutable_hash.get("World", 5, &size);
        cut_assert_not_null(data);
        cut_assert_equal_memory("!?!?!", 5, data, size);
        free(data);

        cut_assert_true(mutable_hash.set("World", 5, "#@#@", 4));
        data = mutable_hash.get("World", 5, &size);
        cut_assert_not_null(data);
        cut_assert_equal_memory("#@#@", 4, data, size);
        free(data);

        cut_assert_equal_int(2, mutable_hash.count());

        // ----
        
        cut_assert_equal_int(true, mutable_hash.append("Fiji", 4, "Island", 6));
        
        cut_assert_equal_int(3, mutable_hash.count());

        data = mutable_hash.get("Fiji", 4, &size);
        cut_assert_not_null(data);
        cut_assert_equal_memory("Island", 6, data, size);
        free(data);

    }

    void test_cursor() {
        MutableHash mutable_hash;
        cut_assert_equal_int(true, mutable_hash.open());
        cut_assert_equal_int(true, mutable_hash.set("Hello", 5, "OK", 2));
        cut_assert_equal_int(true, mutable_hash.set("World", 5, "!", 1));
        cut_assert_equal_int(true, mutable_hash.set("foo", 3, "hoge", 4));
        cut_assert_equal_int(true, mutable_hash.set("hoge", 4, "foo", 3));

        
        std::map<std::string, std::string> found;
        MutableHashCursor cursor(&mutable_hash);
        while(cursor.next()) {
            char *key, *data;
            size_t keylen, datalen;
            cut_assert_true(cursor.get(&key, &keylen, &data, &datalen));
            cut_assert_equal_int(0, found.count(key));
            found[key] = data;
        }

        cut_assert_equal_int(4, found.size());
        
        cut_assert_equal_int(1, found.count("Hello"));
        cut_assert_equal_int(1, found.count("World"));
        cut_assert_equal_int(1, found.count("foo"));
        cut_assert_equal_int(1, found.count("hoge"));
        
        cut_assert_equal_string("OK", found["Hello"].c_str());
        cut_assert_equal_string("!", found["World"].c_str());
        cut_assert_equal_string("foo", found["hoge"].c_str());
        cut_assert_equal_string("hoge", found["foo"].c_str());
    }

    void test_cursor_key() {
        MutableHash mutable_hash;
        cut_assert_equal_int(true, mutable_hash.open());
        cut_assert_equal_int(true, mutable_hash.set("Hello", 5, "OK", 2));
        cut_assert_equal_int(true, mutable_hash.set("World", 5, "!", 1));
        cut_assert_equal_int(true, mutable_hash.set("foo", 3, "hoge", 4));
        cut_assert_equal_int(true, mutable_hash.set("hoge", 4, "foo", 3));

        //fprintf(stderr, "Initialized hashmap\n");
        
        std::map<std::string, std::string> found;
        MutableHashCursor cursor(&mutable_hash);
        //fprintf(stderr, "Initialized cursor\n");
        while(cursor.next()) {
            size_t keylen;
            char *key = cursor.getKey(&keylen);
            cut_assert_equal_int(strlen(key), keylen);
            cut_assert_equal_int(0, found.count(key));
            found[key] = "found";
        }

        cut_assert_equal_int(4, found.size());
    }

    void test_many_data() {
        MutableHash mutable_hash;
        cut_assert_equal_int(true, mutable_hash.open());
        
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

            cut_assert(mutable_hash.set(linebuf, strlen(linebuf), p, strlen(p)));
            bzero(linebuf, sizeof(linebuf));
        }
        
        fseek(testdata, 0, SEEK_SET);

        cut_assert_equal_int(47734, mutable_hash.count());

        while (fgets(linebuf, sizeof(linebuf), testdata) != NULL) {
            char *p = strchr(linebuf, '|');
            if (p == NULL) continue;
            *p = '\0';
            p++;
            char *e = strchr(p, '\n');
            *e = '\0';

            uint32_t length;
            char *data = (char *)mutable_hash.get(linebuf, strlen(linebuf), &length);
            cut_assert(data, cut_message("Cannot obtain value for %s (expected %s)", linebuf, p));
            cut_assert_equal_memory(p, strlen(p), data, length);
            bzero(linebuf, sizeof(linebuf));
        }
        fclose(testdata);
    }
}


