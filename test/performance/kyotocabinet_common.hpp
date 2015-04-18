#ifndef KYOTOCABINET_COMMON_H
#define KYOTOCABINET_COMMON_H

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#include <kcpolydb.h>
#pragma clang diagnostic pop

bool kyotocabinet_builddb(kyotocabinet::PolyDB *db, std::vector<const char *> *keylist);
int kyotocabinet_common(const char *filename);

#endif /* KYOTOCABINET_COMMON_H */
