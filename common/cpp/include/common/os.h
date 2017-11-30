#ifndef HIDRA2_COMMON__OS_H
#define HIDRA2_COMMON__OS_H

#include <cstdint>

namespace HIDRA2
{
    enum OS_TYPE : uint8_t {
        OS_UNKOWN,
        OS_LINUX,
        OS_WINDOWS,

        OS_INVALID = 16, /* Never use more then 4 bit */
    };
}

#endif //HIDRA2_COMMON__OS_H
