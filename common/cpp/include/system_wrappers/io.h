#ifndef HIDRA2_SYSTEM_WRAPPERS__IO_H
#define HIDRA2_SYSTEM_WRAPPERS__IO_H

#include <unistd.h>
#include <sys/socket.h>
#include "io_utils.h"

namespace hidra2 {

class IO {
  public:
    /**
     * Generic
     **/
    virtual int     open    (const char *__file, int __oflag) = 0;
    virtual int     close   (int __fd) = 0;
    virtual ssize_t read    (int __fd, void *buf, size_t count) = 0;
    virtual ssize_t write   (int __fd, const void *__buf, size_t __n) = 0;
    virtual int     select  (int __nfds, fd_set *__restrict __readfds, fd_set *__restrict __writefds, fd_set *__restrict __exceptfds, struct timeval *__restrict __timeout) = 0;

    /**
     * Network
     **/
    virtual int     socket  (int __domain, int __type, int __protocol) = 0;
    virtual int     bind    (int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len) = 0;
    virtual int     listen  (int __fd, int __n) = 0;
    virtual int     accept  (int __fd, __SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len) = 0;
    virtual ssize_t connect (int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len) = 0;
    virtual ssize_t recv    (int __fd, void *__buf, size_t __n, int __flags) = 0;
    virtual ssize_t send    (int __fd, const void *__buf, size_t __n, int __flags) = 0;


};

}

#endif //HIDRA2_SYSTEM_WRAPPERS__IO_H
