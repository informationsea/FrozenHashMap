#include "mutablevaluetable.hpp"
#include "common.hpp"
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <vector>

#define DEBUG(fmt,...) (debugMode && fprintf(stderr, __FILE__ ": %3d: " fmt "\n" ,__LINE__, ## __VA_ARGS__))

namespace {
    void printBinaryData(const void* data, size_t length) {
        const uint8_t* data8 = (const uint8_t *)data;
        for (size_t i = 0; i < length; i++) {
            fprintf(stderr, "%02x [%c]", *data8, *data8);
            data8++;
        }
    }
}

namespace frozenhashmap {

    using namespace frozenhashmap_private;
    static bool debugMode = false;

#define MUTABLE_VALUETABLE_ALIGNMENT 8

    uint32_t frozenhashmap_private::mutableValueTableAlignedBytes(uint32_t pos)
    {
        return (pos/MUTABLE_VALUETABLE_ALIGNMENT+1)*MUTABLE_VALUETABLE_ALIGNMENT;
    }


    MutableValueTableChunk::MutableValueTableChunk(MutableValueTable *table, uint64_t position) :
        m_table(table), m_position(position), m_ok(false), m_errno(0)
    {
        char *debug = ::getenv("MUTABLEVALUETABLE_DEBUG");
        if (debug != NULL)
            debugMode = true;
        DEBUG("Creating MutableValueTableChunk at 0x" UINT64XF, position);
        
        if (fseeko(table->m_file, position, SEEK_SET)) goto onerror;
        if (fread(&m_header, sizeof(m_header), 1, table->m_file) != 1) goto onerror;

        m_ok = true;

        return;
    onerror:
        m_ok = false;
        m_errno = errno;
        return;
    }

    MutableValueTableChunk::MutableValueTableChunk() :
        m_table(0), m_position(0), m_ok(false), m_errno(0){
        m_header.flags = MUTABLE_VALUETABLE_FLAG_INVALID;
    }

    MutableValueTableChunk::~MutableValueTableChunk()
    {

    }

    uint32_t MutableValueTableChunk::readData(void *buf, size_t bufsize)
    {
        uint32_t readBytes;
        if (fseeko(m_table->m_file, m_position+sizeof(m_header), SEEK_SET)) goto onerror;
        readBytes = fread(buf, 1, MIN(bufsize, m_header.dataLength), m_table->m_file);
        if (readBytes != MIN(bufsize, m_header.dataLength)) goto onerror;
        return readBytes;
    
    onerror:
        m_errno = errno;
        return 0;

    }

    MutableValueTableChunk MutableValueTableChunk::nextChunk()
    {
        if (m_header.nextPosition == UINT64_MAX)
            return MutableValueTableChunk();
        return MutableValueTableChunk(m_table, m_header.nextPosition);
    }

    uint32_t MutableValueTableChunk::updateData(const void *data, uint32_t newlength)
    {
        uint32_t shouldWrite = MIN(newlength, m_header.chunkLength - 1);
        if (fwrite(data, 1, shouldWrite, m_table->m_file) != shouldWrite)
            goto onerror;
        
        if (shouldWrite != m_header.dataLength) {
            m_header.dataLength = shouldWrite;
            updateHeader();
        }
            
        return shouldWrite;
    onerror:
        m_errno = errno;
        return UINT32_MAX;
    }

    bool MutableValueTableChunk::isEmpty() {
        return m_header.flags == MUTABLE_VALUETABLE_FLAG_EMPTY;
    }

    bool MutableValueTableChunk::isInvalid() {
        return m_header.flags == MUTABLE_VALUETABLE_FLAG_INVALID || !m_ok;
    }

    bool MutableValueTableChunk::updateHeader()
    {
        if (fseeko(m_table->m_file, m_position, SEEK_SET)) goto onerror;
        if (fwrite(&m_header, sizeof(m_header), 1, m_table->m_file) != 1) goto onerror;
        return true;
    onerror:
        m_errno = errno;
        return false;
    }

    void MutableValueTableChunk::debugPrint()
    {
        char data[128];
        bzero(data, sizeof(data));
        if (fseeko(m_table->m_file, m_position+sizeof(m_header), SEEK_SET)) {sprintf(data, "ERROR");}
        size_t bytes = fread(data, 1, MIN(sizeof(data)-1, m_header.dataLength), m_table->m_file);

        
        fprintf(stderr,
                "position: 0x" UINT64XF "\n"
                "ok: %u\n"
                "next position: 0x" UINT64XF "\n"
                "flags: %02x\n"
                "chunk length: %u\n"
                "data length: %u\n"
                "total length: %u\n"
                "data[%zu]: ",
                
                m_position,
                m_ok,
                m_header.nextPosition,
                m_header.flags,
                m_header.chunkLength,
                m_header.dataLength,
                m_header.totalLength,
                bytes);
        printBinaryData(data, bytes);
        fprintf(stderr, "\n");
        
    }


    MutableValueTable::MutableValueTable(int fd) : m_filename(0), m_errno(0)
    {
        m_file = fdopen(fd, "w+");
        if (m_file == NULL) {m_errno = errno;}
    }

    MutableValueTable::MutableValueTable(FILE *file) : m_filename(0), m_errno(0)
    {
        m_file = file;
    }

    MutableValueTable::MutableValueTable(const char *filename) : m_errno(0)
    {
        int fd = open(filename, O_RDWR|O_CREAT, 0600);
        if (fd < 0) {m_errno = errno; return;}
        m_file = fdopen(fd, "w+");
        if (m_file == NULL) {m_errno = errno;return;}
        m_filename = filename;
    }

    MutableValueTable::~MutableValueTable()
    {
        if (m_filename) {
            fclose(m_file);
        }
    }

    uint64_t MutableValueTable::addEntry(const void *data, uint32_t length)
    {
        return createNewChunk(data, length, length, 0).m_position;
    }

    bool MutableValueTable::appendToEntry(uint64_t position, const void *data, uint32_t length)
    {
        MutableValueTableChunk chunk(this, position);
        uint32_t newTotal = chunk.totalLength() + length;

        chunk.m_header.totalLength = newTotal;
        if (!chunk.updateHeader()) goto onerror;
        
        while (chunk.nextPosition() != UINT64_MAX) {
            chunk = chunk.nextChunk();
            chunk.m_header.totalLength = newTotal;
            if (!chunk.updateHeader()) goto onerror;
        }

        {
            MutableValueTableChunk newChunk = createNewChunk(data, length, newTotal, chunk.m_header.chunkIndex+1);
            if (newChunk.isInvalid()) goto onerror;
        
            chunk.m_header.nextPosition = newChunk.position();
            if (!chunk.updateHeader()) goto onerror;
        }

        return true;
    onerror:
        m_errno = errno;
        return false;
    }

    bool MutableValueTable::updateEntry(uint64_t position, const void *data, uint32_t length)
    {
        uint32_t remainBytes = length;
        MutableValueTableChunk chunk(this, position), lastChunk;
        const char *current = (const char *)data;

        do {
            chunk.m_header.totalLength = length;
            if (!chunk.updateHeader()) goto onerror;

            uint32_t wroteBytes = chunk.updateData(current, remainBytes);
            if (wroteBytes == UINT32_MAX) goto onerror;
            remainBytes -= wroteBytes;
            current += wroteBytes;
            lastChunk = chunk;
            chunk = chunk.nextChunk();
        } while(!chunk.isInvalid());


        if (remainBytes > 0) {
            MutableValueTableChunk createdChunk =
                createNewChunk(current, remainBytes, length, lastChunk.m_header.chunkIndex+1);
            if (createdChunk.isInvalid()) goto onerror;
            lastChunk.m_header.nextPosition = createdChunk.position();
            if (!lastChunk.updateHeader()) goto onerror;
        }

        return true;
    onerror:
        m_errno = errno;
        return false;
    }
    
    void *MutableValueTable::getEntry(uint64_t position, uint32_t *length)
    {
        MutableValueTableChunk firstChunk(this, position);
        MutableValueTableChunk chunk = firstChunk;
        char *data, *current;
        uint32_t bufferSize = firstChunk.totalLength();
        if ((data = (char *)malloc(bufferSize)) == NULL) goto onerror;
        bzero(data, bufferSize);
        current = data;
        do {
             uint32_t readBytes = chunk.readData(current, chunk.length());
             if (readBytes == 0) goto onerror;
             bufferSize -= readBytes;
             current += readBytes;
             chunk = chunk.nextChunk();
        } while(!chunk.isInvalid() && chunk.m_header.dataLength > 0);

        *length = firstChunk.totalLength();
        return data;
    onerror:
        m_errno = errno;
        return NULL;
    }

    uint64_t MutableValueTable::nextEntry(uint64_t position)
    {
        MutableValueTableChunk chunk(this, position);
        do {
            chunk = MutableValueTableChunk(this, chunk.position() + chunk.m_header.chunkLength + sizeof(ChunkHeader));
        } while (chunk.m_header.chunkIndex != 0 && !chunk.isInvalid());
        return chunk.position();
    }

    MutableValueTableChunk MutableValueTable::createNewChunk(const void *data, uint32_t length, uint32_t totalLength, uint16_t chunkNumber)
    {
        uint64_t pos;
        uint32_t alignedLength = mutableValueTableAlignedBytes(length);
        char zero[MUTABLE_VALUETABLE_ALIGNMENT];
        bzero(zero, sizeof(zero));

        DEBUG("New Chunk length: %u  data: %s", length, (const char *)data);
    
        if (fseeko(m_file, 0, SEEK_END)) goto onerror;
        off_t poso;
        if ((poso = ftello(m_file)) < 0) goto onerror;
        pos = poso;

        ChunkHeader header;
        bzero(&header, sizeof(header));

        header.flags = MUTABLE_VALUETABLE_FLAG_DATA;
        header.chunkIndex = chunkNumber;
        header.totalLength = totalLength;
        header.dataLength = length;
        header.chunkLength = alignedLength;
        header.nextPosition = UINT64_MAX;

        if (fwrite(&header, sizeof(header), 1, m_file) != 1)
            goto onerror;
        if (fwrite(data, length, 1, m_file) != 1)
            goto onerror;
        if (fwrite(zero, alignedLength-length, 1, m_file) != 1)
            goto onerror;

        {
            MutableValueTableChunk chunk(this, pos);
            return chunk;
        }
    onerror:
        m_errno = errno;
        return MutableValueTableChunk();
    }

    void MutableValueTable::debugPrint()
    {
        MutableValueTableChunk chunk(this, 0);
        do {
            chunk.debugPrint();
            fprintf(stderr, "\n");
            chunk = MutableValueTableChunk(this, chunk.position() + chunk.m_header.chunkLength + sizeof(ChunkHeader));
        } while (!chunk.isInvalid());
    }

    const char* MutableValueTable::errorMessage()
    {
        return strerror(m_errno);
    }
}
