#ifndef HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
#define HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H

#include <bits/unique_ptr.h>
#include "io.h"

namespace hidra2 {

class SystemIO final : public IO {
  public:
    /**
     * Network
     **/
    FileDescriptor  create_socket   (AddressFamilies address_family, SocketTypes socket_type, SocketProtocols socket_protocol, IOErrors* err);
    void            inet_bind(FileDescriptor socket_fd,
                              const std::string& address,
                              uint16_t port,
                              IOErrors* err);
    void            listen          (FileDescriptor socket_fd, int backlog, IOErrors* err);
    std::unique_ptr<std::tuple<std::string, hidra2::FileDescriptor>> inet_accept(FileDescriptor socket_fd,
                                                                                 IOErrors* err);
    void            inet_connect(FileDescriptor socket_fd, const std::string& address, hidra2::IOErrors* err);


    FileDescriptor  create_and_connect_ip_tcp_socket(const std::string& address, hidra2::IOErrors* err);


    size_t          receive         (FileDescriptor socket_fd, void* buf, size_t length, hidra2::IOErrors* err);
    void            receive_timeout (FileDescriptor socket_fd,
                                     void* buf,
                                     size_t length,
                                     uint16_t timeout_in_sec,
                                     hidra2::IOErrors* err);
    size_t          send            (FileDescriptor socket_fd, const void* buf, size_t length, hidra2::IOErrors* err);

    /**
     * Generic
     **/
    FileData GetDataFromFile(const std::string &fname, IOErrors* err);
    int deprecated_open(const char* __file, int __oflag);
    int deprecated_close(int __fd);
    ssize_t deprecated_read(int __fd, void* buf, size_t count);
    ssize_t deprecated_write(int __fd, const void* __buf, size_t __n);
    
    std::vector<FileInfo> FilesInFolder(const std::string& folder, IOErrors* err);

    /**
     * Network - legacy
     **/
    int     deprecated_socket(int __domain, int __type, int __protocol);
    int     deprecated_bind(int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len);
    int     deprecated_listen(int __fd, int __n);
    int     deprecated_accept(int __fd, __SOCKADDR_ARG __addr, socklen_t* __restrict __addr_len);
    int     deprecated_connect(int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len);
    ssize_t deprecated_recv(int __fd, void* __buf, size_t __n, int __flags);
    ssize_t deprecated_send(int __fd, const void* __buf, size_t __n, int __flags);
    int     deprecated_select(int __nfds,
                              fd_set* __readfds,
                              fd_set* __writefds,
                              fd_set* __exceptfds,
                              struct timeval* __timeout);

};
}

#endif //HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
