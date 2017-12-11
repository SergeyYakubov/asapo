#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include "system_wrappers/system_io.h"

int hidra2::SystemIO::open(const char *__file, int __oflag) {
    return ::open(__file, __oflag);
}

int hidra2::SystemIO::close(int __fd) {
    return ::close(__fd);
}

ssize_t hidra2::SystemIO::read(int __fd, void *buf, size_t count) {
    return ::read(__fd, buf, count);
}

ssize_t hidra2::SystemIO::write(int __fd, const void *__buf, size_t __n) {
    return ::write(__fd, __buf, __n);
}

int hidra2::SystemIO::socket(int __domain, int __type, int __protocol) {
    return ::socket(__domain, __type, __protocol);
}

int hidra2::SystemIO::bind(int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len) {
    return ::bind(__fd, __addr, __len);
}

int hidra2::SystemIO::listen (int __fd, int __n) {
    return ::listen(__fd, __n);
}

int hidra2::SystemIO::accept(int __fd, __SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len) {
    return ::accept(__fd, __addr, __addr_len);
}

ssize_t hidra2::SystemIO::recv(int __fd, void *__buf, size_t __n, int __flags) {
    return ::recv(__fd, __buf, __n, __flags);
}

ssize_t hidra2::SystemIO::send(int __fd, const void* __buf, size_t __n, int __flags) {
    return ::send(__fd, __buf, __n, __flags);
}

int hidra2::SystemIO::connect(int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len) {
    return ::connect(__fd, __addr, __len);
}
int hidra2::SystemIO::select(int __nfds,
                             fd_set* __readfds,
                             fd_set* __writefds,
                             fd_set* __exceptfds,
                             struct timeval* __timeout) {
    return ::select(__nfds, __readfds, __writefds, __exceptfds, __timeout);
}
