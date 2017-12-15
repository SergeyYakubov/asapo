#ifndef HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
#define HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H

#include <bits/unique_ptr.h>
#include "io.h"

namespace hidra2 {

class SystemIO final : public IO {
  public:
    /*
     * Special
     */
    std::thread*            new_thread      (std::function<void()> function);

    // this is not standard function - to be implemented differently in windows and linux
    FileData                GetDataFromFile (const std::string& fname, IOError* err);
    std::vector<FileInfo>   FilesInFolder   (const std::string& folder, IOError* err);

    /*
     * Network
     */
    FileDescriptor  create_socket   (AddressFamilies address_family, SocketTypes socket_type,
                                     SocketProtocols socket_protocol, IOError* err);
    void            listen          (FileDescriptor socket_fd, int backlog, IOError* err);
    void            inet_bind       (FileDescriptor socket_fd, const std::string& address, uint16_t port,
                                     IOError* err);
    std::unique_ptr<std::tuple<std::string, FileDescriptor>> inet_accept(FileDescriptor socket_fd,
            IOError* err);
    void            inet_connect    (FileDescriptor socket_fd, const std::string& address, IOError* err);
    FileDescriptor  create_and_connect_ip_tcp_socket(const std::string& address, IOError* err);

    size_t          receive         (FileDescriptor socket_fd, void* buf, size_t length, IOError* err);
    size_t          receive_timeout (FileDescriptor socket_fd,
                                     void* buf,
                                     size_t length,
                                     uint16_t timeout_in_sec,
                                     IOError* err);
    size_t          send            (FileDescriptor socket_fd, const void* buf, size_t length, IOError* err);

    /*
     * Filesystem
     */
    FileDescriptor  open            (const std::string& filename, FileOpenMode open_flags, IOError* err);
    void            close           (FileDescriptor fd, IOError* err = nullptr);

    //TODO need to remove
    ssize_t deprecated_read         (int __fd, void* buf, size_t count);
    ssize_t deprecated_write        (int __fd, const void* __buf, size_t __n);
};
}

#endif //HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
