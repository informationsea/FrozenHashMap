#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <memory>
#include <unistd.h>
#include <sys/mman.h>
#include <limits.h>
#include <assert.h>

#include "config.h"
#include "common.hpp"
#include "frozenhash.hpp"
#include "frozenhashbuilder.hpp"
#include "valuetable.hpp"
#include "MurmurHash3.h"

namespace frozenhashmap {

#define PAGE_ALIGNMENT (1024*4)
#define DEBUG(fmt,...) (debugMode && fprintf(stderr, __FILE__ ": %3d: " fmt "\n" ,__LINE__, ## __VA_ARGS__))
    static bool debugMode = false;

    FrozenMapBuilder::FrozenMapBuilder() :
        ready(false), hash2position_file(0), valuetable_file(0), valuetable(0), entryCount(0)
    {
        bzero(tempdir, sizeof(tempdir));
        bzero(hash2position_path, sizeof(hash2position_path));
        bzero(hashtable_path, sizeof(hashtable_path));
        bzero(valuetable_path, sizeof(valuetable_path));
    
        char *debug = ::getenv("FROZENHASH_DEBUG");
        if (debug != NULL)
            debugMode = true;
        DEBUG("Calling Constructor");
    }

    FrozenMapBuilder::~FrozenMapBuilder()
    {
    
        if (debugMode) {
            DEBUG("Calling Deconstructor");
            return;
        }

        delete valuetable;

        if (hash2position_file)
            fclose(hash2position_file);
        if (valuetable_file)
            fclose(valuetable_file);
    
        if (strlen(tempdir) == 0)
            return;

        if (strlen(hash2position_path) > 0)
            unlink(hash2position_path);

        if (strlen(hashtable_path) > 0)
            unlink(hashtable_path);
    
        if (strlen(valuetable_path) > 0)
            unlink(valuetable_path);

        rmdir(tempdir);
    }

    bool FrozenMapBuilder::open()
    {
#define TEMPDIR_PATTERN "%s/frozenhash-XXXXXX"
        const char* tmpdir_parent = ::getenv("TMPDIR");
        if (tmpdir_parent == NULL)
            tmpdir_parent = "/tmp";
        snprintf(tempdir, sizeof(tempdir)-1, TEMPDIR_PATTERN, tmpdir_parent);
        if (mkdtemp(tempdir) == NULL)
            return false;

        snprintf(hash2position_path, sizeof(hash2position_path)-1, "%s/%s", tempdir, "hash2position.dat");
        snprintf(valuetable_path, sizeof(valuetable_path)-1, "%s/%s", tempdir, "valuetable.dat");

        hash2position_file = fopen(hash2position_path, "w+");
        if (hash2position_file == NULL)
            return false;
        valuetable_file = fopen(valuetable_path, "w+");
        if (valuetable_file == NULL)
            return false;
        
        valuetable = new ValueTableWriter(valuetable_file);
        ready = true;
        return true;
    }

    bool FrozenMapBuilder::put(const std::string &key, const std::string &value)
    {
        return put(key.c_str(), key.length(), value.c_str(), value.length());
    }

    bool FrozenMapBuilder::put(const char *key, size_t keylen, const char *value, size_t valuelen)
    {
        if (!ready) return false;
        if (keylen > INT32_MAX) return false;
        if (valuelen > INT32_MAX) return false;

        off_t pos = valuetable->tell();
        if (!valuetable->write(key, keylen)) {ready = false; return false;}
        if (!valuetable->write(value, valuelen)) {ready = false; return false;}

        uint64_t hashvalue[2];
        MurmurHash3_x64_128(key, keylen, HASH_RANDOM_SEED, hashvalue);
        //fprintf(stderr, "PUT %s %lu = %llu\n", key, keylen, hashvalue[0]);
        FrozenHashMapHashPosition position(hashvalue[0], pos);
        if (fwrite(&position, sizeof(position), 1, hash2position_file) != 1) {
            ready = false;
            return false;
        }
        entryCount += 1;
        return true;
    }


    bool FrozenMapBuilder::build(int fd)
    {
        if (!ready) return false;

        FrozenHashMapHeader header;
        header.count = entryCount;
        header.hashsize = ceiling(entryCount * 2, EXPECTED_PAGE_SIZE/sizeof(struct FrozenHashMapHashPosition));
        header.hashtable_offset = ceiling(sizeof(FrozenHashMapHeader), EXPECTED_PAGE_SIZE);
        header.hashtable_size = header.hashsize*sizeof(struct FrozenHashMapHashPosition);
        header.valuetable_offset = ceiling(header.hashtable_offset + header.hashtable_size, EXPECTED_PAGE_SIZE);
        header.valuetable_size = valuetable->tell();

        DEBUG("HEADER             COUNT: " UINT64UF, header.count);
        DEBUG("HEADER          HASHSIZE: " UINT64UF, header.hashsize);
        DEBUG("HEADER  HASHTABLE_OFFSET: " UINT64UF, header.hashtable_offset);
        DEBUG("HEADER    HASHTABLE_SIZE: " UINT64UF, header.hashtable_size);
        DEBUG("HEADER VALUETABLE_OFFSET: " UINT64UF, header.valuetable_offset);
        DEBUG("HEADER   VALUETABLE_SIZE: " UINT64UF, header.valuetable_size);

        fclose(valuetable_file);
        valuetable_file = 0;

        ssize_t wroteBytes = write(fd, &header, sizeof(header));
        if (wroteBytes != sizeof(header)) return false;
        if (!fillbytes(fd, 0x00, header.hashtable_offset - sizeof(header)))
            return false;

        // fill hashtable
        if (!fillbytes(fd, 0xff, header.hashtable_size))
            return false;

        struct FrozenHashMapHashPosition *hashtable =
            (struct FrozenHashMapHashPosition *)mmap(NULL,
                                                     header.hashtable_size,
                                                     PROT_WRITE|PROT_READ,
                                                     MAP_FILE|MAP_SHARED,
                                                     fd, header.hashtable_offset);
        if (hashtable == MAP_FAILED)
            return false;

        FrozenHashMapHashPosition empty;
        memset(&empty, 0xff, sizeof(empty));

        if (fseeko(hash2position_file, 0, SEEK_SET) != 0) return false;
        for (size_t i = 0; i < entryCount; i++) {
            DEBUG("NEW Entry");
            struct FrozenHashMapHashPosition pos;
            if (fread(&pos, sizeof(pos), 1, hash2position_file) != 1) return false;
            size_t candidatePos = pos.hash_value % header.hashsize;
            do {
                DEBUG("Candidate Pos: %zu  %llu %llu %llu %p", candidatePos, header.hashsize, lseek(fd, 0, SEEK_CUR), header.valuetable_offset, hashtable + candidatePos);
                DEBUG("Data : %lld", hashtable[candidatePos].value_position);
                if (memcmp(hashtable + candidatePos, &empty, sizeof(empty)) == 0) {
                    hashtable[candidatePos] = pos;
                    break;
                }
                candidatePos = (candidatePos + 1) % header.hashsize;
            } while (1);
        }

        if (munmap(hashtable, sizeof(struct FrozenHashMapHashPosition *)*header.hashsize) != 0) return false;

        if (!fillbytes(fd, 0xff, header.valuetable_offset - lseek(fd, 0, SEEK_CUR)))
            return false;
        DEBUG("END HASHTABLE %lld %llu", lseek(fd, 0, SEEK_CUR), header.valuetable_offset);
        
        int valuetable_fd = ::open(valuetable_path, O_RDONLY);
        copydata(fd, valuetable_fd);
        ::close(valuetable_fd);

        return true;
    }

    bool FrozenMapBuilder::build(const char *filename)
    {
        int fd = ::open(filename, O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        bool ok = build(fd);
        ::close(fd);
        return ok;
    }
}
