#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <memory>
#include <unistd.h>
#include <sys/mman.h>

#include "config.h"
#include "common.hpp"
#include "frozenhash.hpp"
#include "frozenhashbuilder.hpp"
#include "valuetable.hpp"
#include "MurmurHash3.h"

#define PAGE_ALIGNMENT (1024*4)
#define DEBUG(fmt,...) (debugMode && fprintf(stderr, __FILE__ ": %3d: " fmt "\n" ,__LINE__, ## __VA_ARGS__))
static bool debugMode = false;

FrozenMapBuilder::FrozenMapBuilder(bool ainmemory) : inmemory(ainmemory), ready(false)
{
    bzero(tempdir, sizeof(tempdir));
    bzero(hash2key_path, sizeof(hash2key_path));
    bzero(data_path, sizeof(data_path));
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
    
    if (strlen(tempdir) == 0)
        return;

    if (strlen(hash2key_path) > 0)
        unlink(hash2key_path);

    if (strlen(data_path) > 0)
        unlink(data_path);
    
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

    if (inmemory)
        snprintf(hash2key_path, sizeof(hash2key_path)-1, "-");
    else
        snprintf(hash2key_path, sizeof(hash2key_path)-1, "%s/%s", tempdir, "hash2key.kct");
    
    if (inmemory)
        snprintf(data_path, sizeof(data_path)-1, "-");
    else
        snprintf(data_path, sizeof(data_path)-1, "%s/%s", tempdir, "data.kct");

    bool ok1 = hash2key.open(hash2key_path);
    bool ok2 = data.open(data_path);
    ready = ok1 && ok2;
    return ready;
}

bool FrozenMapBuilder::put(const std::string &key, const std::string &value)
{
    if (!ready) return false;
    return data.set(key, value);
}

bool FrozenMapBuilder::put(const char *key, size_t keylen, const char *value, size_t valuelen)
{
    if (!ready) return false;
    return data.set(key, keylen, value, valuelen);
}


bool FrozenMapBuilder::build(int fd)
{
    if (!ready) return false;
    // build hash2key
    DEBUG("Data count: %lld", data.count());
    uint64_t hashsize = (uint64_t)(HASH_SIZE_FACTOR*data.count()+1);
    if (hashsize > UINT64_MAX) hashsize = UINT64_MAX;
    
    {
        data.synchronize();
        hash2key.begin_transaction();
        std::auto_ptr<DB::Cursor> cur(data.cursor());
        if (!cur->jump()) return false;
        DEBUG("Hash size: %llu", hashsize);

        char *key;
        size_t sp;
        while((key = cur->get_key(&sp, true))) {
            uint64_t hashvalue[2];
            MurmurHash3_x64_128(key, sp, HASH_RANDOM_SEED, &hashvalue);
            char hashstr[30];
            size_t length = snprintf(hashstr, sizeof(hashstr)-1, "%llu", hashvalue[0] % hashsize);
            DEBUG("Calculate hash for %s = %s", key, hashstr);
            uint32_t valuelen = sp;
            if (hash2key.append(hashstr, length, (const char *)(&valuelen), sizeof(valuelen)) == false) {
                DEBUG("Cannot set value %s", hash2key.error().message());
                delete key;
                return false;
            }
            if (hash2key.append(hashstr, length, key, sp) == false) {
                DEBUG("Cannot set value %s", hash2key.error().message());
                delete key;
                return false;
            }
            delete key;
        }
        hash2key.end_transaction();
        hash2key.synchronize();
    }
    
    // preparation for build hash table and value table
    DEBUG("Preparing to build hash table and value table");
    snprintf(hashtable_path, sizeof(hashtable_path)-1, "%s/%s", tempdir, "hashtable.dat");
    int hashtable_fd = ::open(hashtable_path, O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR);
    if (hashtable_fd < 0) {perror("Cannot open hash table"); return false;}
    size_t hashtable_size = hashsize*(HASH_VALYE_BYTES);
    {
        char zero[1024*10];
        memset(zero, 0xff, sizeof(zero));
        size_t written_bytes = 0;
        while (written_bytes < hashtable_size) {
            ssize_t wrote = ::write(hashtable_fd, zero, MIN(hashtable_size - written_bytes, sizeof(zero)));
            if (wrote < 0) {perror("cannot write"); return false;}
            written_bytes += wrote;
        }
    }
    ::close(hashtable_fd);
    hashtable_fd = ::open(hashtable_path, O_RDWR);
    if (hashtable_fd < 0) {perror("Cannot open hash table"); return false;}

    uint64_t *hashtable_map = (uint64_t *)mmap(NULL, hashtable_size, PROT_READ|PROT_WRITE, MAP_SHARED, hashtable_fd, 0);
    if (hashtable_map == MAP_FAILED) {perror("Cannot map"); return false;}

    snprintf(valuetable_path, sizeof(valuetable_path)-1, "%s/%s", tempdir, "valuetable.dat");

    FILE *valuetable_file = fopen(valuetable_path, "w");
    if (valuetable_file == NULL) {perror("Cannot open value table"); return false;}
    ValueTableWriter valuetable(valuetable_file);


    // build hash table and value table
    DEBUG("Building hash table and value table");
    {
        std::auto_ptr<DB::Cursor> cur(hash2key.cursor());
        if (!cur->jump()) {DEBUG("DB Error hash2key"); return false;}
        uint64_t wrote_data_count = 0;

        const char *value;
        size_t ksp, vsp;
        do {
            char *key = cur->get(&ksp, &value, &vsp, true);
            if (key == NULL) {
                DEBUG("Finish %s", hash2key.error().message());
                break;
            }
            DEBUG("Processing for hash %s / length: %zu", key, vsp);

            long filepos = ftell(valuetable_file);
            if (filepos > UINT64_MAX) {
                DEBUG("Too large value table\n");
                return false;
            }

            char *endptr;
            uint64_t hashvalue = strtoul(key, &endptr, 10);
            *(hashtable_map + hashvalue) = filepos;

            size_t valuepos = 0;
            do {
                uint32_t keylen;
                memcpy(&keylen, value+valuepos, sizeof(keylen));
                valuepos += sizeof(keylen);
                DEBUG("NEXT key[%u]: %s", keylen, value+valuepos);
                valuetable.write(value+valuepos, keylen);

                size_t data_valuesize;
                char *data_value = data.get(value+valuepos, keylen, &data_valuesize);
                if (data_value == NULL) {
                    DEBUG("Cannot get hash2key value for %s (%s)\n", value+valuepos, data.error().message());
                    return false;
                }

                valuetable.write(data_value, data_valuesize);
                wrote_data_count += 1;
                
                valuepos += keylen;
            } while(valuepos < vsp);
            delete key;
        } while (1);
        
        DEBUG("Wrote data count: %llu", wrote_data_count);
    }
    
    fclose(valuetable_file);
    {

        valuetable_file = fopen(valuetable_path, "r");
        if (valuetable_file == NULL) {perror("Cannot re-open valuetable"); return false;}
        FILE *dbfile = ::fdopen(fd, "w");
        if (dbfile == NULL) {perror("Cannot open DB file"); return false;}

        // fill header
        char headerFill[PAGE_ALIGNMENT];
        bzero(headerFill, sizeof(headerFill));
        fwrite(headerFill, sizeof(headerFill), 1, dbfile);

        // prepare header
        FrozenHashMapHeader header;
        bzero(&header, sizeof(header));

        memcpy(header.magic, FROZENHASH_HEADER, sizeof(header.magic));
        header.endian_check = DB_ENDIAN_CHECK;
        header.version = DB_FORMAT_VERSION;
        header.count = data.count();
        header.hashsize = hashsize;
        header.hashtable_size = hashtable_size;

        fseek(valuetable_file, 0L, SEEK_END);
        header.valuetable_size = ftell(valuetable_file);
        fseek(valuetable_file, 0L, SEEK_SET);

        header.hashtable_offset = ftell(dbfile);
        // write hashtable
        fwrite(hashtable_map, hashtable_size, 1, dbfile);
        size_t padding_length = (hashtable_size/PAGE_ALIGNMENT+1)*PAGE_ALIGNMENT - hashtable_size;
        fwrite(headerFill, padding_length, 1, dbfile);

        // write valuetable
        header.valuetable_offset = ftell(dbfile);
        {
            char copybuffer[1024*10];
            do {
                size_t readbytes = fread(copybuffer, 1, sizeof(copybuffer), valuetable_file);
                if (readbytes > 0)
                    fwrite(copybuffer, 1, readbytes, dbfile);
                if (readbytes != sizeof(copybuffer)) {
                    if (feof(valuetable_file))
                        break;
                    perror("Reading valuetable file");
                    return false;
                }
            } while(1);
        }
    
        munmap(hashtable_map, hashtable_size);
        fclose(valuetable_file);

        fseek(dbfile, 0, SEEK_SET);
        fwrite(&header, sizeof(header), 1, dbfile);
        fclose(dbfile);
    }    
    return true;
}

bool FrozenMapBuilder::build(const char *filename)
{
    int fd = ::open(filename, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    bool ok = build(fd);
    ::close(fd);
    return ok;
}
