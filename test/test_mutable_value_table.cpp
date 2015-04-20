#include <cppcutter.h>
#include <stdlib.h>

#include "mutablevaluetable.hpp"

namespace mutablevaluetable_test {

    using namespace frozenhashmap;

    void test_alignment () {
        cut_assert_equal_int(8, mutablevaluetable_private::mutableValueTableAlignedBytes(0));
        cut_assert_equal_int(8, mutablevaluetable_private::mutableValueTableAlignedBytes(1));
        cut_assert_equal_int(8, mutablevaluetable_private::mutableValueTableAlignedBytes(2));
        cut_assert_equal_int(8, mutablevaluetable_private::mutableValueTableAlignedBytes(3));
        cut_assert_equal_int(8, mutablevaluetable_private::mutableValueTableAlignedBytes(4));
        cut_assert_equal_int(8, mutablevaluetable_private::mutableValueTableAlignedBytes(5));
        cut_assert_equal_int(8, mutablevaluetable_private::mutableValueTableAlignedBytes(6));
        cut_assert_equal_int(8, mutablevaluetable_private::mutableValueTableAlignedBytes(7));
        cut_assert_equal_int(16, mutablevaluetable_private::mutableValueTableAlignedBytes(0+8));
        cut_assert_equal_int(16, mutablevaluetable_private::mutableValueTableAlignedBytes(1+8));
        cut_assert_equal_int(16, mutablevaluetable_private::mutableValueTableAlignedBytes(2+8));
        cut_assert_equal_int(16, mutablevaluetable_private::mutableValueTableAlignedBytes(3+8));
        cut_assert_equal_int(16, mutablevaluetable_private::mutableValueTableAlignedBytes(4+8));
        cut_assert_equal_int(16, mutablevaluetable_private::mutableValueTableAlignedBytes(5+8));
        cut_assert_equal_int(16, mutablevaluetable_private::mutableValueTableAlignedBytes(6+8));
        cut_assert_equal_int(16, mutablevaluetable_private::mutableValueTableAlignedBytes(7+8));
    }
    
    void test_mutable_value_table () {
        cut_assert_equal_int(0, system("mkdir -p tmp"));
        cut_assert_equal_int(0, system("rm tmp/mutable_value_table.dat;true"));
        MutableValueTable value("tmp/mutable_value_table.dat");
        cut_assert_equal_int(0, value.error());
        uint64_t positions[] = {
            value.addEntry("HI", 2),
            value.addEntry("HELLO", 5),
            value.addEntry("This is a pen.", 14),
        };

        uint32_t length;
        void *data = value.getEntry(positions[0], &length);
        cut_assert_equal_int(0, value.error(), cut_message("Error: %s", value.errorMessage()));
        cut_assert_equal_memory("HI", 2, data, length);

        data = value.getEntry(positions[1], &length);
        cut_assert_equal_int(0, value.error());
        cut_assert_equal_memory("HELLO", 5, data, length);

        data = value.getEntry(positions[2], &length);
        cut_assert_equal_int(0, value.error());
        cut_assert_equal_memory("This is a pen.", 14, data, length);
    }
    
    void test_mutable_value_table2 () {
        uint64_t positions[4];
        cut_assert_equal_int(0, system("mkdir -p tmp"));
        cut_assert_equal_int(0, system("rm tmp/mutable_value_table2.dat;true"));
        {
            MutableValueTable value("tmp/mutable_value_table2.dat");
            cut_assert_equal_int(0, value.error());
            positions[0] = value.addEntry("HI", 2);
            positions[1] = value.addEntry("HELLO", 5);
            positions[2] = value.addEntry("This is a pen.", 14);

            cut_assert_equal_int(true, value.appendToEntry(positions[0], " OK!", 4));
            cut_assert_equal_int(true, value.appendToEntry(positions[1], " OK!", 4));
            cut_assert_equal_int(true, value.appendToEntry(positions[1], "!?!?", 4));
            positions[3] = value.addEntry("Last Entry", 14);
        }

        {
            MutableValueTable value("tmp/mutable_value_table2.dat");
            cut_assert_equal_int(positions[1], value.nextEntry(positions[0]));
            cut_assert_equal_int(positions[2], value.nextEntry(positions[1]));
            cut_assert_equal_int(positions[3], value.nextEntry(positions[2]));

            uint32_t length;
            void *data = value.getEntry(positions[0], &length);
            cut_assert_equal_int(0, value.error(), cut_message("Error: %s", value.errorMessage()));
            cut_assert_equal_memory("HI OK!", 6, data, length);
            free(data);

            data = value.getEntry(positions[1], &length);
            cut_assert_equal_int(0, value.error());
            cut_assert_equal_memory("HELLO OK!!?!?", 13, data, length);
            free(data);

            data = value.getEntry(positions[2], &length);
            cut_assert_equal_int(0, value.error());
            cut_assert_equal_memory("This is a pen.", 14, data, length);
            free(data);
        }
    }

    void test_mutable_value_table3 () {
        uint64_t positions[4];
        cut_assert_equal_int(0, system("mkdir -p tmp"));
        cut_assert_equal_int(0, system("rm tmp/mutable_value_table3.dat;true"));
        {
            MutableValueTable value("tmp/mutable_value_table3.dat");
            cut_assert_equal_int(0, value.error());
            positions[0] = value.addEntry("HI", 2);
            positions[1] = value.addEntry("HELLO", 5);
            positions[2] = value.addEntry("This is a pen.", 14);

            cut_assert_equal_int(true, value.appendToEntry(positions[1], " OK!", 4));
            cut_assert_equal_int(true, value.updateEntry(positions[0], "This is a greeting.", 19));
            cut_assert_equal_int(true, value.appendToEntry(positions[0], " Hello!", 7));
            cut_assert_equal_int(true, value.appendToEntry(positions[1], "!?!?", 4));
            positions[3] = value.addEntry("Last Entry", 14);
        }

        {
            MutableValueTable value("tmp/mutable_value_table3.dat");
            //value.debugPrint();
            cut_assert_equal_int(positions[1], value.nextEntry(positions[0]));
            cut_assert_equal_int(positions[2], value.nextEntry(positions[1]));
            cut_assert_equal_int(positions[3], value.nextEntry(positions[2]));

            uint32_t length;
            void *data = value.getEntry(positions[0], &length);
            cut_assert_equal_int(0, value.error(), cut_message("Error: %s", value.errorMessage()));
            cut_assert_equal_memory("This is a greeting. Hello!", 26, data, length);
            free(data);

            data = value.getEntry(positions[1], &length);
            cut_assert_equal_int(0, value.error());
            cut_assert_equal_memory("HELLO OK!!?!?", 13, data, length);
            free(data);

            data = value.getEntry(positions[2], &length);
            cut_assert_equal_int(0, value.error());
            cut_assert_equal_memory("This is a pen.", 14, data, length);
            free(data);
        }
    }
}

