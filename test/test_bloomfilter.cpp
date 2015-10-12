#include "test_common.hpp"
#include <bloomfilter.hpp>

namespace {
    using namespace frozenhashmap;

    void printBloomFilterBuffer(const BloomFilter &bloomFilter, FILE *output) {
        for (size_t i = 0; i < bloomFilter.bufferSize(); i++) {
            if (i % 8 == 0 && i != 0) fprintf(output, "\n");
            if (i % 8 == 0) fprintf(output, "%04zx ", i);
            
            for (int j = 7; j >= 0; j--) {
                fprintf(output, "%d", (bloomFilter.buffer()[i] & (1 << j)) ? 1 : 0);
            }
            fprintf(output, " ");
        }

        fprintf(output, "\n");
    }

    TEST(BLOOMFILTER, CONSTRUCTOR) {
        BloomFilter bloomFilter1(10, 1);
        ASSERT_FALSE(bloomFilter1.isReady());
        
        BloomFilter bloomFilter2(16, 1);
        ASSERT_TRUE(bloomFilter2.isReady());

        BloomFilter bloomFilter3(19, 1);
        ASSERT_FALSE(bloomFilter3.isReady());
        
        BloomFilter bloomFilter4(1024, 1);
        ASSERT_TRUE(bloomFilter4.isReady());

        BloomFilter bloomFilter5(3, 1);
        ASSERT_FALSE(bloomFilter5.isReady());
        
        BloomFilter bloomFilter6(10000, 1);
        ASSERT_FALSE(bloomFilter6.isReady());
    }

    TEST(BLOOMFILTER, INSERT_CHECK) {
        BloomFilter bloomFilter(64, 2);
        ASSERT_TRUE(bloomFilter.isReady());
        bloomFilter.insert("Hello");
        bloomFilter.insert("Bloom");
        bloomFilter.insert("bloom");
        bloomFilter.insert("hoge");

        ASSERT_TRUE(bloomFilter.check("Hello"));
        ASSERT_TRUE(bloomFilter.check("Bloom"));
        ASSERT_TRUE(bloomFilter.check("bloom"));
        ASSERT_TRUE(bloomFilter.check("hoge"));

        EXPECT_FALSE(bloomFilter.check("genome")); // Not always success
        EXPECT_FALSE(bloomFilter.check("foo")); // Not always success
        EXPECT_FALSE(bloomFilter.check("bar")); // Not always success
        printBloomFilterBuffer(bloomFilter, stdout);
    }

    TEST(BLOOMFILTER, INSERT_CHECK2) {
        BloomFilter bloomFilter(1024*32, 4);
        ASSERT_TRUE(bloomFilter.isReady());
        
        FILE *testdata = fopen("./geneid-symbol.txt", "r");
        ASSERT_TRUE(testdata);

        char linebuf[256];
        bzero(linebuf, sizeof(linebuf));
        ASSERT_EQ((off_t)0, fseek(testdata, 0, SEEK_SET));

        while (fgets(linebuf, sizeof(linebuf), testdata) != NULL) {
            bloomFilter.insert(linebuf);
        }
        
        ASSERT_EQ((off_t)0, fseek(testdata, 0, SEEK_SET));
        while (fgets(linebuf, sizeof(linebuf), testdata) != NULL) {
            ASSERT_TRUE(bloomFilter.check(linebuf));
        }

        EXPECT_FALSE(bloomFilter.check("hoge"));
        EXPECT_FALSE(bloomFilter.check("foo"));
        EXPECT_FALSE(bloomFilter.check("bar"));
        EXPECT_FALSE(bloomFilter.check("genome"));
        
        fclose(testdata);
        //printBloomFilterBuffer(bloomFilter, stdout);
    }

    TEST(BLOOMFILTER, INSERT_CHECK3) {
        BloomFilter bloomFilter(1024*1024, 18);
        ASSERT_TRUE(bloomFilter.isReady());
        
        FILE *testdata = fopen("./geneid-symbol.txt", "r");
        ASSERT_TRUE(testdata);

        char linebuf[256];
        bzero(linebuf, sizeof(linebuf));
        ASSERT_EQ((off_t)0, fseek(testdata, 0, SEEK_SET));

        while (fgets(linebuf, sizeof(linebuf), testdata) != NULL) {
            bloomFilter.insert(linebuf);
        }

        ASSERT_EQ((off_t)0, fseek(testdata, 0, SEEK_SET));
        while (fgets(linebuf, sizeof(linebuf), testdata) != NULL) {
            ASSERT_TRUE(bloomFilter.check(linebuf));
        }

        EXPECT_FALSE(bloomFilter.check("hoge"));
        EXPECT_FALSE(bloomFilter.check("foo"));
        EXPECT_FALSE(bloomFilter.check("bar"));
        EXPECT_FALSE(bloomFilter.check("genome"));
        
        fclose(testdata);
    }
}

