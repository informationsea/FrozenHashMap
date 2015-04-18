#include "frozenhash.hpp"
#include "frozenhashbuilder.hpp"
#include <stdio.h>
#include <string.h>

enum Mode {
    HELP, DUMP, BUILD, GET, INFO
};

int main(int argc, char *argv[])
{
    enum Mode mode = HELP;
    int exitCode = 0;
    
    if (argc == 1) {
        exitCode = 1;
    } else if (strcmp("help", argv[1]) == 0) {
        mode = HELP;
    } else if (strcmp("dump", argv[1]) == 0) {
        if (argc == 3)
            mode = DUMP;
        else
            exitCode = 1;
    } else if (strcmp("info", argv[1]) == 0) {
        if (argc == 3)
            mode = INFO;
        else
            exitCode = 1;
    } else if (strcmp("get", argv[1]) == 0) {
        if (argc == 4)
            mode = GET;
        else
            exitCode = 1;
    } else if (strcmp("build", argv[1]) == 0) {
        if (argc == 4)
            mode = BUILD;
        else
            exitCode = 1;
    }


    switch (mode) {
    case HELP: {
        FILE *out = stdout;
        if (exitCode > 0) out = stderr;
        char *sep = strrchr(argv[0], '/');
        if (sep)
            sep++;
        else
            sep = argv[0];
        
        fprintf(out,
                "%s command ...\n\n"
                "%s help                 : show this help\n"
                "%s dump DBFILE          : show list of data\n"
                "%s get DBFILE KEY       : get a value corresponding to KEY\n"
                "%s build DBFILE TSVFILE : build database for TSVFILE\n"
            , sep, sep, sep, sep, sep);
        break;}
    case INFO: {
        class FrozenMap map;
        if (!map.open(argv[2])) {
            fprintf(stderr, "Cannot open database\n");
            return 1;
        }
        printf("Count: %llu\n", map.count());
        break;}
    case DUMP: {
        class FrozenMap map;
        if (!map.open(argv[2])) {
            fprintf(stderr, "Cannot open database\n");
            return 1;
        }
        FrozenMapCursor cursor(&map);
        std::pair<std::string, std::string> pair;
        while (cursor.nextString(&pair)) {
            printf("%s\t%s\n", pair.first.c_str(), pair.second.c_str());
        }
        break;}
    case GET: {
        class FrozenMap map;
        if (!map.open(argv[2])) {
            fprintf(stderr, "Cannot open database\n");
            return 1;
        }
        size_t valuelen;
        const char* value = map.get(argv[3], strlen(argv[3]), &valuelen);
        printf("%s\n", value);
        break;}
    case BUILD: {
        FrozenMapBuilder builder;
        if (!builder.open()) {
            fprintf(stderr, "Cannot open database builder\n");
            return 1;
        }

        FILE *tsv = fopen(argv[3], "r");
        if (tsv == NULL) {
            perror("Cannot open tsv");
            return 1;
        }

        fprintf(stderr, "Loading data...\n");
        char linebuf[1024*5];
        int linenum = 0;
        while (fgets(linebuf, sizeof(linebuf) - 1, tsv)) {
            linenum++;
            
            if (strlen(linebuf) == 0) // skip empty line
                continue;
            
            char *sep = strchr(linebuf, '\t');
            *sep = 0;
            char *end = strchr(sep+1, '\n');
            *end = 0;

            builder.put(linebuf, (sep+1));
        }

        if (!feof(tsv)) {
            perror("Cannot read TVS");
            return 1;
        }

        fclose(tsv);

        fprintf(stderr, "Building database...\n");
        if (!builder.build(argv[2])) {
            fprintf(stderr, "Cannot build database\n");
            return 1;
        }
        
        break;}
    }
    
    
    return exitCode;
}

