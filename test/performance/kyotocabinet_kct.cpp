#include "performance_common.hpp"
#include "kyotocabinet_common.hpp"

int main(void)
{
    if (system("mkdir -p " DBDIR) != 0) return 1;
    return kyotocabinet_common(DBDIR "/kyoto.kct");
}
