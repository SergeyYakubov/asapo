#ifndef HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
#define HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H

#include "io.h"

namespace hidra2 {
class SystemIO : public IO {
  public:
    int open(const char *__file, int __oflag) final;
    int close(int __fd) final;
    ssize_t read(int __fd, void *buf, size_t count) final;
    ssize_t write(int __fd, const void *__buf, size_t __n) final;
};
}

#endif //HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
