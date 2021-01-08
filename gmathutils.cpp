#include "gmathutils.h"
#include <algorithm>
using namespace std;

int GMathUtils::FloatNegativOneToOne2Int32(float src)
{
    src = min(src, 1.0f);
    src = max(src, -1.0f);
    return (int)(src*(double)INT_MAX);
}
