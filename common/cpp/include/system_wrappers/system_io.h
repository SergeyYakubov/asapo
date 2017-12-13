#ifndef HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
#define HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H

#include <bits/unique_ptr.h>
#include "io.h"

namespace hidra2 {

class SystemIO final : public IO {
  public:
    /*
     * System
     */
    template<typename Function, typename... Args>
    std::thread* new_thread(Function&& f, Args&&... args);

    /*
     * Network
     */
    FileDescriptor  create_socket   (AddressFamilies address_family, SocketTypes socket_type,
                                     SocketProtocols socket_protocol, IOErrors* err);
    void            listen          (FileDescriptor socket_fd, int backlog, IOErrors* err);
    void            inet_bind       (FileDescriptor socket_fd, const std::string& address, uint16_t port, IOErrors* err);
    std::unique_ptr<std::tuple<std::string, FileDescriptor>> inet_accept(FileDescriptor socket_fd, IOErrors* err);
    void            inet_connect    (FileDescriptor socket_fd, const std::string& address, IOErrors* err);
    FileDescriptor  create_and_connect_ip_tcp_socket(const std::string& address, IOErrors* err);

    size_t          receive         (FileDescriptor socket_fd, void* buf, size_t length, IOErrors* err);
    size_t          receive_timeout (FileDescriptor socket_fd,
                                     void* buf,
                                     size_t length,
                                     uint16_t timeout_in_sec,
                                     IOErrors* err);
    size_t          send            (FileDescriptor socket_fd, const void* buf, size_t length, IOErrors* err);

    /*
     * Generic
     */
    int deprecated_open             (const char* __file, int __oflag);
    int deprecated_close            (int __fd);
    ssize_t deprecated_read         (int __fd, void* buf, size_t count);
    ssize_t deprecated_write        (int __fd, const void* __buf, size_t __n);

    // this is not standard function - to be implemented differently in windows and linux
    FileData GetDataFromFile        (const std::string& fname, IOErrors* err);
    std::vector<FileInfo> FilesInFolder(const std::string& folder, IOErrors* err);
};
}

#endif //HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
