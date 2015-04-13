#ifndef VALUETABLE_H
#define VALUETABLE_H

#include <stdio.h>
#include <string>

class ValueTableWriter {
public:
    ValueTableWriter(FILE *file);

    /**
     * return true if succeeded
     */
    bool write(const char *buf, size_t length);
    bool write(std::string value);
    long tell();
private:
    FILE *m_file;
};

class ValueTableReader {
public:
    ValueTableReader(int fd);
    ValueTableReader(const char *mapping, size_t mapping_length);
    virtual ~ValueTableReader();
    
    const char* readNext(size_t *length);

    bool isReady() { return ready; }
    long tell();
    int seek(off_t offset);
private:
    int m_fd;
    const char *datamap;
    size_t datalength;
    
    size_t currentpos;
    
    bool ready;

    size_t readBytes(void *dest, size_t length);
};

#endif /* VALUETABLE_H */
