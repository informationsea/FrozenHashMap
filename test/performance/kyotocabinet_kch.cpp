#include "performance_common.hpp"
#include "kyotocabinet_common.hpp"

int main(int argc, char *argv[])
{
    if (system("mkdir -p " DBDIR) != 0) return 1;
    return kyotocabinet_common(DBDIR "/kyoto.kch");
}
