#ifndef HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
#define HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H

#include "io.h"

namespace hidra2 {
class SystemIO final : public IO {
  public:
    int open(const char *__file, int __oflag);
    int close(int __fd);
    ssize_t read(int __fd, void *buf, size_t count);
    ssize_t recv(int __fd, void *__buf, size_t __n, int __flags);
    ssize_t write(int __fd, const void *__buf, size_t __n);
    ssize_t send(int __fd, const void *__buf, size_t __n, int __flags);

};
}

#endif //HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
