#ifndef HIDRA2_SYSTEM_WRAPPERS__IO_H
#define HIDRA2_SYSTEM_WRAPPERS__IO_H

#include <string>
#include <vector>
#include <chrono>
#include <sys/socket.h>
#include <memory>
#include <thread>

#include "common/file_info.h"

namespace hidra2 {


enum class IOError {
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

enum FileOpenMode {
    OPEN_MODE_READ,
    OPEN_MODE_WRITE,
    OPEN_MODE_RW = OPEN_MODE_READ | OPEN_MODE_WRITE,

    OPEN_MODE_CREATE,
    /**
     * Will set the length of a file to 0
     * Only works if file is open with READ and WRITE mode
     */
     OPEN_MODE_SET_LENGTH_0,
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
    /*
     * Special
     */
    virtual std::thread*    new_thread(std::function<void()> function) = 0;

    // this is not standard function - to be implemented differently in windows and linux
    virtual FileData GetDataFromFile        (const std::string& fname, IOError* err) = 0;
    virtual std::vector<FileInfo> FilesInFolder(const std::string& folder, IOError* err) = 0;

    /*
     * Network
     */
    virtual FileDescriptor  create_socket   (AddressFamilies address_family, SocketTypes socket_type,
                                             SocketProtocols socket_protocol, IOError* err) = 0;
    virtual void            listen          (FileDescriptor socket_fd, int backlog, IOError* err) = 0;
    virtual void            inet_bind       (FileDescriptor socket_fd, const std::string& address, uint16_t port,
                                             IOError* err) = 0;
    virtual std::unique_ptr<std::tuple<std::string, FileDescriptor>> inet_accept(FileDescriptor socket_fd,
            IOError* err) = 0;
    virtual void            inet_connect    (FileDescriptor socket_fd, const std::string& address, IOError* err) = 0;
    virtual FileDescriptor  create_and_connect_ip_tcp_socket(const std::string& address, IOError* err) = 0;

    virtual size_t          receive         (FileDescriptor socket_fd, void* buf, size_t length, IOError* err) = 0;
    virtual size_t          receive_timeout (FileDescriptor socket_fd,
                                             void* buf,
                                             size_t length,
                                             uint16_t timeout_in_sec,
                                             IOError* err) = 0;
    virtual size_t          send            (FileDescriptor socket_fd, const void* buf, size_t length, IOError* err) = 0;

    /*
     * Filesystem
     */
    virtual FileDescriptor  open            (const std::string& filename, FileOpenMode open_flags, IOError* err) = 0;
    /**
     * @param err Is able to accept nullptr
     */
    virtual void            close           (FileDescriptor fd, IOError *err = nullptr) = 0;

    //TODO need to remove
    virtual ssize_t deprecated_read         (int __fd, void* buf, size_t count) = 0;
    virtual ssize_t deprecated_write        (int __fd, const void* __buf, size_t __n) = 0;
};

}

#endif //HIDRA2_SYSTEM_WRAPPERS__IO_H
