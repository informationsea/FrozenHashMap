#ifndef BLOOMFILTER_H
#define BLOOMFILTER_H

#include <stdint.h>
#include <string.h>

extern "C" {
    void MurmurHash3_x64_128 ( const void * key, int len, uint32_t seed, void * out );
}

#define BLOOMFILTER_MAX_NUMBER_OF_HASH 16
#define BLOOMFILTER_MURMURSEED 3590585660

namespace frozenhashmap {
    class BloomFilter
    {
    public:
        /**
         * bufferSize should be power of 2
         */
        BloomFilter(size_t bufferSize, int numberOfHash) :
            m_ready(false), m_bufferSize(bufferSize), m_buffer(0), m_numberOfHash(numberOfHash) {

            // check whether bufferSize is power of 2 or not
            for (size_t i = 0; i < sizeof(size_t)*8; ++i) {
                if (bufferSize & (1 << i)) {
                    if ((bufferSize ^ (1 << i)) == 0) {
                        m_bufferSizeBit = i + 3;
                        m_bufferIndexMask = (1 << m_bufferSizeBit) - 1;
                        break;
                    } else {
                        fprintf(stderr, "Invalid buffer size %zu\n", bufferSize);
                        return;
                    }
                }
            }

            m_buffer = new uint8_t[bufferSize];
            if (m_buffer == NULL) return;
            bzero(m_buffer, bufferSize);
            m_ready = true;
            
            //fprintf(stderr, "Initialized %zu %zu %zx\n", bufferSize, m_bufferSizeBit, m_bufferIndexMask);
        }
        
        virtual ~BloomFilter() {
            delete m_buffer;
        }

        void insert(const void *data, size_t length) {
            size_t hashvalues[BLOOMFILTER_MAX_NUMBER_OF_HASH];
            calcHashValues(data, length, hashvalues);
            for (int i = 0; i < m_numberOfHash; i++) {
                size_t address = hashvalues[i] >> 3;
                size_t bit = hashvalues[i] & 0x7;
                m_buffer[address] |= 1 << bit;
            }
        }

        void insert(const char *str) {
            insert(str, strlen(str));
        }

        
        bool check(const void *data, size_t length) const {
            size_t hashvalues[BLOOMFILTER_MAX_NUMBER_OF_HASH];
            calcHashValues(data, length, hashvalues);
            for (int i = 0; i < m_numberOfHash; i++) {
                size_t address = hashvalues[i] >> 3;
                size_t bit = hashvalues[i] & 0x7;
                if (!(m_buffer[address] & 1 << bit))
                    return false;
            }
            return true;
        }

        bool check(const char *str) const {
            return check(str, strlen(str));
        }

        
        const uint8_t *buffer() const {return m_buffer;}
        size_t bufferSize() const {return m_bufferSize;}
        int numberOfHash() const {return m_numberOfHash;}
        bool isReady() const {return m_ready;}
        
    private:
        bool m_ready;
        size_t m_bufferSize;
        size_t m_bufferSizeBit;
        size_t m_bufferIndexMask;
        uint8_t *m_buffer;
        int m_numberOfHash;

        void calcHashValues(const void *data, size_t length, size_t values[]) const {
            uint64_t hashvalue[2];
            MurmurHash3_x64_128(data, length, BLOOMFILTER_MURMURSEED, hashvalue);
            size_t remainBits = sizeof(hashvalue[0])*8;
            size_t currentIndex = 0;
            int murmurSeedOffset = 0;

            for (int i = 0; i < m_numberOfHash; ++i) {
                values[i] = hashvalue[currentIndex] & m_bufferIndexMask;
                hashvalue[currentIndex] >>= m_bufferSizeBit;
                remainBits -= m_bufferSizeBit;

                // not enough bits in the hash
                if (remainBits < m_bufferSizeBit) {
                    currentIndex += 1;
                    remainBits = sizeof(hashvalue[0])*8;
                    
                    if (currentIndex > 1) {
                        murmurSeedOffset -= 1;
                        MurmurHash3_x64_128(data, length, BLOOMFILTER_MURMURSEED + murmurSeedOffset, hashvalue);
                        currentIndex = 0;
                        //fprintf(stderr, "hash updated %d %d %s", i, murmurSeedOffset, data);
                    }
                }
            }
        }
    };
}

#endif /* BLOOMFILTER_H */
