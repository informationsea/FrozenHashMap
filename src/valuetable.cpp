#include "valuetable.hpp"

#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include "common.hpp"

namespace frozenhashmap {
    using namespace frozenhashmap_private;

    ValueTableWriter::ValueTableWriter(FILE *file): m_file(file)
    {
    
    }

    bool ValueTableWriter::write(const char *buf, size_t length)
    {
        size_t aligned_length = calculate_aligned_length(length);
        size_t padding_bytes = aligned_length - length;
        uint32_t length32 = length;
        char zero[VALUE_TABLE_ALIGNMENT_BYTES];

        bzero(zero, sizeof(zero));

        if (fwrite(&length32, sizeof(length32), 1, m_file) != 1)
            return false;
        if (fwrite(buf, length, 1, m_file) != 1)
            return false;
        if (fwrite(zero, padding_bytes, 1, m_file) != 1)
            return false;
        return true;
    }

    bool ValueTableWriter::write(std::string value)
    {
        return write(value.c_str(), value.length());
    }

    off_t ValueTableWriter::tell()
    {
        return ftello(m_file);
    }

    ValueTableReader::ValueTableReader(int fd): m_fd(fd), currentpos(0)
    {
        off_t end = lseek(m_fd, 0, SEEK_END);
        lseek(m_fd, 0, SEEK_SET);
    
        datalength = end;
        datamap = (char *)mmap(NULL, datalength, PROT_READ, MAP_SHARED, m_fd, 0);
    }

    ValueTableReader::ValueTableReader(const char *mapping, size_t mapping_length):
        m_fd(-1), datamap(mapping), datalength(mapping_length), currentpos(0)
    {

    }

    ValueTableReader::~ValueTableReader()
    {
        if (m_fd < 0) return;
        munmap((void *)datamap, datalength);
    }

    const char* ValueTableReader::readNext(size_t *length)
    {
        uint32_t length32;
        if (readBytes(&length32, sizeof(length32)) != sizeof(length32))
            return NULL;
        if (currentpos + length32 > datalength)
            return NULL;
        *length = length32;
        size_t origin = currentpos;
        currentpos += calculate_aligned_length(length32);
        return datamap + origin;
    }

    off_t ValueTableReader::tell()
    {
        return currentpos;
    }

    int ValueTableReader::seek(off_t offset)
    {
        return currentpos = offset;
    }

    size_t ValueTableReader::readBytes(void *dest, size_t length)
    {
        ssize_t should_read_length = length;
        if (length + currentpos > datalength)
            should_read_length = (ssize_t)datalength - currentpos;
        if (should_read_length <= 0)
            return 0;

        memcpy(dest, datamap+currentpos, should_read_length);
        currentpos += should_read_length;
        return should_read_length;
    }
}
