#ifndef HIDRA2_SYSTEM_WRAPPERS__SYSTEM_UTIL_H
#define HIDRA2_SYSTEM_WRAPPERS__SYSTEM_UTIL_H

#include "io.h"

namespace hidra2 {

class IOUtils {
 private:
    IO** io_;
 public:
    explicit IOUtils(IO** io);

    bool is_valid_fd(int fd);
    ssize_t send_in_steps(int __fd, const void* __buf, size_t __n, int __flags);
    ssize_t recv_in_steps(int __fd, void* __buf, size_t __n, int __flags);
};

}

#endif //HIDRA2_SYSTEM_WRAPPERS__SYSTEM_UTIL_H
