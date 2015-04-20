#include <cppcutter.h>
#include <stdlib.h>

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

        cut_assert_equal_int(true, mutable_hash.append("World", 5, "?!?!", 4));
        data = mutable_hash.get("World", 5, &size);
        cut_assert_not_null(data);
        cut_assert_equal_memory("!?!?!", 5, data, size);
        free(data);

    }
}
