#ifndef HIDRA2_SYSTEM_WRAPPERS__SYSTEM_UTIL_H
#define HIDRA2_SYSTEM_WRAPPERS__SYSTEM_UTIL_H

namespace hidra2 {

class IOUtils {
 public:
    bool is_valid_fd(int fd);
};

}

#endif //HIDRA2_SYSTEM_WRAPPERS__SYSTEM_UTIL_H
