#include <fcntl.h>
#include <unistd.h>
#include <system_wrappers/system_io.h>

namespace hidra2 {

ssize_t hidra2::SystemIO::deprecated_read(int __fd, void* buf, size_t count) {
    return ::read(__fd, buf, count);
}

ssize_t hidra2::SystemIO::deprecated_write(int __fd, const void* __buf, size_t __n) {
    return ::write(__fd, __buf, __n);
}
}
