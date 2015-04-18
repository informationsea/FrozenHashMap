#ifndef PERFORMANCE_COMMON_H
#define PERFORMANCE_COMMON_H

#include <stdio.h>
#include <memory>
#include <vector>
#include <string>
#include <sys/time.h>
#include "SFMT.h"

#define DBDIR "testdata"
#define LOOKUP_KEY_SIZE_FACTOR 100

class TSVPairLoader {
private:
    FILE *file;
    bool ready;
    char linebuf[1024*5];
public:
    TSVPairLoader(const char *filename);
    virtual ~TSVPairLoader();

    bool isReady();
    bool next(const char **key, const char **value);
};

TSVPairLoader *openFixtureData();
uint64_t* prepareRandomIndex(size_t length, size_t max);

class PerformanceTest {
private:
    struct timeval _start, _end;
    std::string message;
public:
    PerformanceTest(std::string msg) : message(msg) {}
    void start() {
        gettimeofday(&_start, NULL);
    }

    void end() {
        gettimeofday(&_end, NULL);
        time_t secdiff = _end.tv_sec - _start.tv_sec;
        suseconds_t microdiff = _end.tv_usec - _start.tv_usec;
        uint64_t diff = ((uint64_t)secdiff)*1000*1000 + microdiff;
        double sec = diff/(1000.*1000);
        fprintf(stdout, "%s %lf\n", message.c_str(), sec);
    }
};

#endif /* PERFORMANCE_COMMON_H */
