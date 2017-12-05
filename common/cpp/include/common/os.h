#ifndef HIDRA2__COMMON_OS_H
#define HIDRA2__COMMON_OS_H

#include <cstdint>

namespace hidra2 {
enum OS_TYPE : uint8_t {
    OS_UNKOWN,
    OS_LINUX,
    OS_WINDOWS,

    OS_INVALID = 16, /* Never use more then 4 bit */
};
}

#endif //HIDRA2__COMMON_OS_H
