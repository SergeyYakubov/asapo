#ifndef HIDRA2_SYSTEM_WRAPPERS__IO_H
#define HIDRA2_SYSTEM_WRAPPERS__IO_H

#include <string>
#include <vector>
#include <chrono>
#include <sys/socket.h>

#include "common/file_info.h"

namespace hidra2 {


enum class IOErrors {
    NO_ERROR,
    FILE_NOT_FOUND,
    READ_ERROR,
    PERMISSIONS_DENIED,
    UNKWOWN_ERROR
};

IOErrors IOErrorFromErrno();

class IO {
  public:

    virtual FileData GetDataFromFile(const std::string &fname, IOErrors* err) = 0;

    /**
     * Generic
     **/
    virtual int open(const char* __file, int __oflag) = 0;
    virtual int close(int __fd) = 0;
    virtual ssize_t read(int __fd, void* buf, size_t count) = 0;
    virtual ssize_t write(int __fd, const void* __buf, size_t __n) = 0;

    /**
     * Network
     **/
    virtual int     socket  (int __domain, int __type, int __protocol) = 0;
    virtual int     bind    (int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len) = 0;
    virtual int     listen  (int __fd, int __n) = 0;
    virtual int     accept  (int __fd, __SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len) = 0;
    virtual int     connect (int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len) = 0;
    virtual ssize_t recv    (int __fd, void *__buf, size_t __n, int __flags) = 0;
    virtual ssize_t send    (int __fd, const void *__buf, size_t __n, int __flags) = 0;
    virtual int     select(int __nfds, fd_set* __readfds, fd_set* __writefds, fd_set* __exceptfds, struct timeval* __timeout) = 0;

// this is not standard function - to be implemented differently in windows and linux
    virtual std::vector<FileInfo> FilesInFolder(const std::string& folder, IOErrors* err) = 0;
};

}

#endif //HIDRA2_SYSTEM_WRAPPERS__IO_H
