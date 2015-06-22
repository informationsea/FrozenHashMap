#ifndef MUTABLEVALUETABLE_H
#define MUTABLEVALUETABLE_H

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

namespace frozenhashmap {
    namespace frozenhashmap_private {
        uint32_t mutableValueTableAlignedBytes(uint32_t pos);

        const uint8_t MUTABLE_VALUETABLE_FLAG_EMPTY = 0x00;
        const uint8_t MUTABLE_VALUETABLE_FLAG_DATA = 0x01;
        const uint8_t MUTABLE_VALUETABLE_FLAG_INVALID = 0xff;
        
        struct ChunkHeader {
            uint8_t flags;
            uint16_t chunkIndex;
            uint32_t totalLength;
            uint32_t chunkLength;
            uint32_t dataLength;
            uint64_t nextPosition;
        };
    }



    class MutableValueTable;

    class MutableValueTableChunk {
        friend class MutableValueTable;
    public:
        MutableValueTableChunk(MutableValueTable *table, uint64_t position);
        virtual ~MutableValueTableChunk();

        bool isFirstChunk() {return m_header.chunkIndex == 0;}
        uint32_t totalLength() {return m_header.totalLength;}
        uint32_t length() {return m_header.dataLength;}
        uint64_t position() {return m_position;}
        uint64_t nextPosition() {return m_header.nextPosition;}
        bool ok() {return m_ok;}
        bool isEmpty();
        bool isInvalid();

        uint32_t readData(void *buf, size_t bufsize);

        /**
         * Update a data of this chunk.
         * If 
         * @param data new data
         * @param newlength a length of new data
         * @return wrote byte length
         */
        uint32_t updateData(const void *data, uint32_t newlength);

        MutableValueTableChunk nextChunk();

        /**
         * DO NOT CALL THIS FUNCTION
         * ONLY AVAILABLE FOR DEBUG
         */
        void debugPrint();
    
    private:
        /**
         * Create invalid chunk
         */
        MutableValueTableChunk();
    
        MutableValueTable *m_table;
        uint64_t m_position;

        frozenhashmap_private::ChunkHeader m_header;
    
        bool m_isfirst;
        bool m_ok;

        int m_errno;

        bool updateHeader();
    };

    class MutableValueTable {
        friend class MutableValueTableChunk;
    public:
        MutableValueTable(int fd);
        MutableValueTable(FILE *file);
        MutableValueTable(const char *filename);
        virtual ~MutableValueTable();

        /**
         * @return a position of the entry if succeeded. return UINT64_MAX if failed.
         */
        uint64_t addEntry(const void *data, uint32_t length);

        bool appendToEntry(uint64_t position, const void *data, uint32_t length);
        bool updateEntry(uint64_t position, const void *data, uint32_t length);
        void *getEntry(uint64_t position, uint32_t *length);
        uint64_t nextEntry(uint64_t position);
        
        const char* errorMessage();
        int error() { return m_errno; }

        /**
         * DO NOT CALL THIS FUNCTION
         * ONLY AVAILABLE FOR DEBUG
         */
        void debugPrint();
    
    private:
        const char *m_filename;
        FILE *m_file;
        int m_errno;

        MutableValueTableChunk createNewChunk(const void *data, uint32_t length, uint32_t totalLength, uint16_t chunkNumber);
    };
}

#endif /* MUTABLEVALUETABLE_H */
