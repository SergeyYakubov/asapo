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
    OPEN_MODE_READ = 1 << 0,
    OPEN_MODE_WRITE = 1 << 1,
    OPEN_MODE_RW = OPEN_MODE_READ | OPEN_MODE_WRITE,

    OPEN_MODE_CREATE = 1 << 2,
    /**
     * Will set the length of a file to 0
     * Only works if file is open with READ and WRITE mode
     */
    OPEN_MODE_SET_LENGTH_0 = 1 << 3,
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
    virtual std::thread*    NewThread       (std::function<void()> function) = 0;

    // this is not standard function - to be implemented differently in windows and linux
    virtual FileData                GetDataFromFile (const std::string& fname, uint64_t fsize, IOError* err) = 0;
    virtual std::vector<FileInfo>   FilesInFolder   (const std::string& folder, IOError* err) = 0;

    /*
     * Network
     */
    virtual FileDescriptor  CreateSocket    (AddressFamilies address_family, SocketTypes socket_type,
                                             SocketProtocols socket_protocol, IOError* err) = 0;
    virtual void            Listen          (FileDescriptor socket_fd, int backlog, IOError* err) = 0;
    virtual void            InetBind        (FileDescriptor socket_fd, const std::string& address, uint16_t port,
                                             IOError* err) = 0;
    virtual std::unique_ptr<std::tuple<std::string, FileDescriptor>> InetAccept(FileDescriptor socket_fd,
            IOError* err) = 0;
    virtual void            InetConnect     (FileDescriptor socket_fd, const std::string& address, IOError* err) = 0;
    virtual FileDescriptor  CreateAndConnectIPTCPSocket(const std::string& address, IOError* err) = 0;

    virtual size_t          Receive         (FileDescriptor socket_fd, void* buf, size_t length, IOError* err) = 0;
    virtual size_t          ReceiveTimeout  (FileDescriptor socket_fd,
                                             void* buf,
                                             size_t length,
                                             uint16_t timeout_in_sec,
                                             IOError* err) = 0;
    virtual size_t          Send            (FileDescriptor socket_fd, const void* buf, size_t length, IOError* err) = 0;

    /*
     * Filesystem
     */
    virtual FileDescriptor  Open            (const std::string& filename, int open_flags, IOError* err) = 0;
    /**
     * @param err Is able to accept nullptr
     */
    virtual void            Close           (FileDescriptor fd, IOError* err = nullptr) = 0;

    //TODO need to remove
    virtual ssize_t deprecated_read         (int __fd, void* buf, size_t count) = 0;
    virtual ssize_t deprecated_write        (int __fd, const void* __buf, size_t __n) = 0;
};

}

#endif //HIDRA2_SYSTEM_WRAPPERS__IO_H
