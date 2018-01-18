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
    std::thread*            NewThread(std::function<void()> function);

    // this is not standard function - to be implemented differently in windows and linux
    FileData                GetDataFromFile (const std::string& fname, uint64_t fsize, IOError* err);
    std::vector<FileInfo>   FilesInFolder   (const std::string& folder, IOError* err);

    /*
     * Network
     */
    FileDescriptor  CreateSocket(AddressFamilies address_family, SocketTypes socket_type,
                                 SocketProtocols socket_protocol, IOError* err);
    void            Listen(FileDescriptor socket_fd, int backlog, IOError* err);
    void            InetBind(FileDescriptor socket_fd, const std::string& address, uint16_t port,
                             IOError* err);
    std::unique_ptr<std::tuple<std::string, FileDescriptor>> InetAccept(FileDescriptor socket_fd,
            IOError* err);
    void            InetConnect(FileDescriptor socket_fd, const std::string& address, IOError* err);
    FileDescriptor  CreateAndConnectIPTCPSocket(const std::string& address, IOError* err);

    size_t          Receive(FileDescriptor socket_fd, void* buf, size_t length, IOError* err);
    size_t          ReceiveTimeout(FileDescriptor socket_fd,
                                   void* buf,
                                   size_t length,
                                   uint16_t timeout_in_sec,
                                   IOError* err);
    size_t          Send(FileDescriptor socket_fd, const void* buf, size_t length, IOError* err);
    void            Skip(FileDescriptor socket_fd, size_t length, IOError* err);

    /*
     * Filesystem
     */
    FileDescriptor  Open(const std::string& filename, int open_flags, IOError* err);
    void            Close(FileDescriptor fd, IOError* err = nullptr);
    size_t          Write(FileDescriptor fd, const void* buf, size_t length, IOError* err);
    void            CreateDirectory(const std::string &directory_name, hidra2::IOError* err);

};
}

#endif //HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
