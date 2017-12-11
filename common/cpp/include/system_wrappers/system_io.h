#ifndef HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
#define HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H

#include "io.h"

namespace hidra2 {

class SystemIO final : public IO {
  public:
    //IOUtils* util;

    /**
     * Generic
     **/
    int     open    (const char *__file, int __oflag);
    int     close   (int __fd);
    ssize_t read    (int __fd, void *buf, size_t count);
    ssize_t write   (int __fd, const void *__buf, size_t __n);
    int     select  (int __nfds, fd_set *__restrict __readfds, fd_set *__restrict __writefds, fd_set *__restrict __exceptfds, struct timeval *__restrict __timeout);
    /**
     * Network
     **/
    int     socket  (int __domain, int __type, int __protocol);
    int     bind    (int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len);
    int     listen  (int __fd, int __n);
    int     accept  (int __fd, __SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len);
    int     connect (int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len);
    ssize_t recv    (int __fd, void *__buf, size_t __n, int __flags);
    ssize_t send    (int __fd, const void *__buf, size_t __n, int __flags);

};

}

#endif //HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
