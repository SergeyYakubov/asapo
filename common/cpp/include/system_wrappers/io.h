#ifndef HIDRA2_SYSTEM_WRAPPERS__IO_H
#define HIDRA2_SYSTEM_WRAPPERS__IO_H

#include <string>
#include <vector>
#include <chrono>
#include <sys/socket.h>
#include <memory>

#include "common/file_info.h"

namespace hidra2 {


enum class IOErrors {
    NO_ERROR,
    BAD_FILE_NUMBER,
    FILE_NOT_FOUND,
    READ_ERROR,
    PERMISSIONS_DENIED,
    UNSUPPORTED_ADDRESS_FAMILY,
    INVALID_ADDRESS_FORMAT,
    STREAM_EOF,
    ADDRESS_ALREADY_IN_USE,
    CONNECTION_REFUSED,
    CONNECTION_RESET_BY_PEER,
    TIMEOUT,
    UNKNOWN_ERROR,
};


enum class AddressFamilies {
    INET,
};

enum class SocketTypes {
    STREAM,
};

enum class SocketProtocols {
    IP,
};

typedef int FileDescriptor;

class IO {
  public:

    virtual FileData GetDataFromFile(const std::string &fname, IOErrors* err) = 0;


    /*
     * Network
     */
    virtual FileDescriptor  create_socket   (AddressFamilies address_family, SocketTypes socket_type, SocketProtocols socket_protocol, IOErrors* err) = 0;
    virtual void            listen          (FileDescriptor socket_fd, int backlog, IOErrors* err) = 0;
    virtual void            inet_bind       (FileDescriptor socket_fd, const std::string& address, uint16_t port, IOErrors* err) = 0;
    virtual std::unique_ptr<std::tuple<std::string, hidra2::FileDescriptor>> inet_accept(FileDescriptor socket_fd, IOErrors* err) = 0;
    virtual void            inet_connect    (FileDescriptor socket_fd, const std::string& address, hidra2::IOErrors* err) = 0;
    virtual FileDescriptor  create_and_connect_ip_tcp_socket(const std::string& address, hidra2::IOErrors* err) = 0;

    virtual size_t          receive         (FileDescriptor socket_fd, void* buf, size_t length, hidra2::IOErrors* err) = 0;
    virtual void receive_timeout(FileDescriptor socket_fd,
                                 void* buf,
                                 size_t length,
                                 uint16_t timeout_in_sec,
                                 hidra2::IOErrors* err) = 0;
    virtual size_t          send            (FileDescriptor socket_fd, const void* buf, size_t length, hidra2::IOErrors* err) = 0;

    /*
     * Generic
     */
    virtual int deprecated_open(const char* __file, int __oflag) = 0;
    virtual int deprecated_close(int __fd) = 0;
    virtual ssize_t deprecated_read(int __fd, void* buf, size_t count) = 0;
    virtual ssize_t deprecated_write(int __fd, const void* __buf, size_t __n) = 0;

    /**
     * Network - legacy
     **/
    virtual int     deprecated_socket(int __domain, int __type, int __protocol) = 0;
    virtual int     deprecated_bind(int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len) = 0;
    virtual int     deprecated_listen(int __fd, int __n) = 0;
    virtual int     deprecated_accept(int __fd, __SOCKADDR_ARG __addr, socklen_t* __restrict __addr_len) = 0;
    virtual int     deprecated_connect(int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len) = 0;
    virtual ssize_t deprecated_recv(int __fd, void* __buf, size_t __n, int __flags) = 0;
    virtual ssize_t deprecated_send(int __fd, const void* __buf, size_t __n, int __flags) = 0;
    virtual int     deprecated_select(int __nfds,
                                      fd_set* __readfds,
                                      fd_set* __writefds,
                                      fd_set* __exceptfds,
                                      struct timeval* __timeout) = 0;

// this is not standard function - to be implemented differently in windows and linux
    virtual std::vector<FileInfo> FilesInFolder(const std::string& folder, IOErrors* err) = 0;
};

}

#endif //HIDRA2_SYSTEM_WRAPPERS__IO_H
