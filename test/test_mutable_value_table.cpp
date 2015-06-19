#include "test_common.hpp"
#include <stdlib.h>

#include "mutablevaluetable.hpp"

namespace mutablevaluetable_test {

    using namespace frozenhashmap;

    TEST(MUTABLEVALUETABLE, ALIGNMENT) {
        ASSERT_EQ(8, mutablevaluetable_private::mutableValueTableAlignedBytes(0));
        ASSERT_EQ(8, mutablevaluetable_private::mutableValueTableAlignedBytes(1));
        ASSERT_EQ(8, mutablevaluetable_private::mutableValueTableAlignedBytes(2));
        ASSERT_EQ(8, mutablevaluetable_private::mutableValueTableAlignedBytes(3));
        ASSERT_EQ(8, mutablevaluetable_private::mutableValueTableAlignedBytes(4));
        ASSERT_EQ(8, mutablevaluetable_private::mutableValueTableAlignedBytes(5));
        ASSERT_EQ(8, mutablevaluetable_private::mutableValueTableAlignedBytes(6));
        ASSERT_EQ(8, mutablevaluetable_private::mutableValueTableAlignedBytes(7));
        ASSERT_EQ(16, mutablevaluetable_private::mutableValueTableAlignedBytes(0+8));
        ASSERT_EQ(16, mutablevaluetable_private::mutableValueTableAlignedBytes(1+8));
        ASSERT_EQ(16, mutablevaluetable_private::mutableValueTableAlignedBytes(2+8));
        ASSERT_EQ(16, mutablevaluetable_private::mutableValueTableAlignedBytes(3+8));
        ASSERT_EQ(16, mutablevaluetable_private::mutableValueTableAlignedBytes(4+8));
        ASSERT_EQ(16, mutablevaluetable_private::mutableValueTableAlignedBytes(5+8));
        ASSERT_EQ(16, mutablevaluetable_private::mutableValueTableAlignedBytes(6+8));
        ASSERT_EQ(16, mutablevaluetable_private::mutableValueTableAlignedBytes(7+8));
    }


    TEST(MUTABLEVALUETABLE, VALUETABLE) {
        ASSERT_EQ(0, system("mkdir -p tmp"));
        ASSERT_EQ(0, system("rm tmp/mutable_value_table.dat 2> /dev/null;true"));
        MutableValueTable value("tmp/mutable_value_table.dat");
        ASSERT_EQ(0, value.error());
        uint64_t positions[] = {
            value.addEntry("HI", 2),
            value.addEntry("HELLO", 5),
            value.addEntry("This is a pen.", 14),
        };

        uint32_t length;
        void *data = value.getEntry(positions[0], &length);
        ASSERT_EQ(0, value.error());//, cut_message("Error: %s", value.errorMessage()));
        ASSERT_MEMEQ("HI", 2, data, length);

        data = value.getEntry(positions[1], &length);
        ASSERT_EQ(0, value.error());
        ASSERT_MEMEQ("HELLO", 5, data, length);

        data = value.getEntry(positions[2], &length);
        ASSERT_EQ(0, value.error());
        ASSERT_MEMEQ("This is a pen.", 14, data, length);
    }

    TEST(MUTABLEVALUETABLE, VALUETABLE2) {
        uint64_t positions[4];
        ASSERT_EQ(0, system("mkdir -p tmp"));
        ASSERT_EQ(0, system("rm tmp/mutable_value_table2.dat 2> /dev/null;true"));
        {
            MutableValueTable value("tmp/mutable_value_table2.dat");
            ASSERT_EQ(0, value.error());
            positions[0] = value.addEntry("HI", 2);
            positions[1] = value.addEntry("HELLO", 5);
            positions[2] = value.addEntry("This is a pen.", 14);

            ASSERT_EQ(true, value.appendToEntry(positions[0], " OK!", 4));
            ASSERT_EQ(true, value.appendToEntry(positions[1], " OK!", 4));
            ASSERT_EQ(true, value.appendToEntry(positions[1], "!?!?", 4));
            positions[3] = value.addEntry("Last Entry", 14);
        }

        {
            MutableValueTable value("tmp/mutable_value_table2.dat");
            ASSERT_EQ(positions[1], value.nextEntry(positions[0]));
            ASSERT_EQ(positions[2], value.nextEntry(positions[1]));
            ASSERT_EQ(positions[3], value.nextEntry(positions[2]));

            uint32_t length;
            void *data = value.getEntry(positions[0], &length);
            ASSERT_EQ(0, value.error());//, cut_message("Error: %s", value.errorMessage()));
            ASSERT_MEMEQ("HI OK!", 6, data, length);
            free(data);

            data = value.getEntry(positions[1], &length);
            ASSERT_EQ(0, value.error());
            ASSERT_MEMEQ("HELLO OK!!?!?", 13, data, length);
            free(data);

            data = value.getEntry(positions[2], &length);
            ASSERT_EQ(0, value.error());
            ASSERT_MEMEQ("This is a pen.", 14, data, length);
            free(data);
        }
    }

    TEST(MUTABLEVALUETABLE, VALUETABLE3) {
        uint64_t positions[4];
        ASSERT_EQ(0, system("mkdir -p tmp"));
        ASSERT_EQ(0, system("rm tmp/mutable_value_table3.dat 2> /dev/null;true"));
        {
            MutableValueTable value("tmp/mutable_value_table3.dat");
            ASSERT_EQ(0, value.error());
            positions[0] = value.addEntry("HI", 2);
            positions[1] = value.addEntry("HELLO", 5);
            positions[2] = value.addEntry("This is a pen.", 14);

            ASSERT_EQ(true, value.appendToEntry(positions[1], " OK!", 4));
            ASSERT_EQ(true, value.updateEntry(positions[0], "This is a greeting.", 19));
            ASSERT_EQ(true, value.appendToEntry(positions[0], " Hello!", 7));
            ASSERT_EQ(true, value.appendToEntry(positions[1], "!?!?", 4));
            positions[3] = value.addEntry("Last Entry", 14);
        }

        {
            MutableValueTable value("tmp/mutable_value_table3.dat");
            //value.debugPrint();
            ASSERT_EQ(positions[1], value.nextEntry(positions[0]));
            ASSERT_EQ(positions[2], value.nextEntry(positions[1]));
            ASSERT_EQ(positions[3], value.nextEntry(positions[2]));

            uint32_t length;
            void *data = value.getEntry(positions[0], &length);
            ASSERT_EQ(0, value.error());//, cut_message("Error: %s", value.errorMessage()));
            ASSERT_MEMEQ("This is a greeting. Hello!", 26, data, length);
            free(data);

            data = value.getEntry(positions[1], &length);
            ASSERT_EQ(0, value.error());
            ASSERT_MEMEQ("HELLO OK!!?!?", 13, data, length);
            free(data);

            data = value.getEntry(positions[2], &length);
            ASSERT_EQ(0, value.error());
            ASSERT_MEMEQ("This is a pen.", 14, data, length);
            free(data);
        }
    }

    TEST(MUTABLEVALUETABLE, VALUETABLE4) {
        ASSERT_EQ(0, system("mkdir -p tmp"));
        ASSERT_EQ(0, system("rm tmp/mutable_value_table4.dat 2> /dev/null;true"));
        MutableValueTable value("tmp/mutable_value_table4.dat");
        ASSERT_EQ(0, value.error());
        uint64_t positions[] = {
            value.addEntry("HI", 2),
            value.addEntry("HELLO", 5),
            value.addEntry("This is a pen.", 14),
        };

        ASSERT_TRUE(value.appendToEntry(positions[0], " TEST text", 10));

        uint32_t length;
        void *data = value.getEntry(positions[0], &length);
        ASSERT_EQ(0, value.error());//, cut_message("Error: %s", value.errorMessage()));
        ASSERT_MEMEQ("HI TEST text", 12, data, length);

        ASSERT_TRUE(value.updateEntry(positions[0], "One", 3));

        data = value.getEntry(positions[0], &length);
        ASSERT_EQ(0, value.error());//, cut_message("Error: %s", value.errorMessage()));
        ASSERT_MEMEQ("One", 3, data, length);

        // ----
        
        ASSERT_TRUE(value.updateEntry(positions[1], "C++ Programming Language", 24));

        data = value.getEntry(positions[1], &length);
        ASSERT_EQ(0, value.error()); //, cut_message("Error: %s", value.errorMessage()));
        ASSERT_MEMEQ("C++ Programming Language", 24, data, length);

        ASSERT_TRUE(value.updateEntry(positions[1], "Ruby", 4));

        data = value.getEntry(positions[1], &length);
        ASSERT_EQ(0, value.error()); //, cut_message("Error: %s", value.errorMessage()));
        ASSERT_MEMEQ("Ruby", 4, data, length);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

