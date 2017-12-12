#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include "system_wrappers/system_io.h"

int hidra2::SystemIO::deprecated_open(const char* __file, int __oflag) {
    return ::open(__file, __oflag);
}

int hidra2::SystemIO::deprecated_close(int __fd) {
    return ::close(__fd);
}

ssize_t hidra2::SystemIO::deprecated_read(int __fd, void* buf, size_t count) {
    return ::read(__fd, buf, count);
}

ssize_t hidra2::SystemIO::deprecated_write(int __fd, const void* __buf, size_t __n) {
    return ::write(__fd, __buf, __n);
}
