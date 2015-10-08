#ifndef VALUETABLE_H
#define VALUETABLE_H

#include <stdio.h>
#include <string>
#include <stdint.h>

#define VALUE_TABLE_ALIGNMENT_BYTES 4

namespace frozenhashmap {

    namespace frozenhashmap_private {

        static inline size_t calculate_aligned_length(size_t length)
        {
            return (length/VALUE_TABLE_ALIGNMENT_BYTES + 1)*VALUE_TABLE_ALIGNMENT_BYTES;
        }
    }

    
    class ValueTableWriter {
    public:
        ValueTableWriter(FILE *file);

        /**
         * return true if succeeded
         */
        bool write(const char *buf, size_t length);
        bool write(std::string value);
        off_t tell();
    private:
        FILE *m_file;
    };

    class ValueTableReader {
    public:
        ValueTableReader(int fd);
        ValueTableReader(const char *mapping, size_t mapping_length);
        virtual ~ValueTableReader();
    
        const char* readNext(size_t *length);

        inline const char* readAt(off_t pos, uint32_t *length, off_t *next) const {
            *length = *((uint32_t *)(datamap+pos));
            if (next)
                *next = pos + frozenhashmap_private::calculate_aligned_length(*length) + sizeof(uint32_t);
            return datamap+pos+sizeof(uint32_t);
        }

        bool isReady() { return ready; }
        off_t tell();
        int seek(off_t offset);
    private:
        int m_fd;
        const char *datamap;
        size_t datalength;
    
        size_t currentpos;
    
        bool ready;

        size_t readBytes(void *dest, size_t length);
    };
}

#endif /* VALUETABLE_H */
