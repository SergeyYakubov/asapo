#ifndef HIDRA2_SYSTEM_WRAPPERS__IO_H
#define HIDRA2_SYSTEM_WRAPPERS__IO_H

#include <cinttypes>
#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include <thread>
#include "common/file_info.h"

namespace hidra2 {


enum class IOErrors {
    kUnknownError,
    kNoError,
    kBadFileNumber,
    kResourceTemporarilyUnavailable,
    kFileNotFound,
    kReadError,
    kPermissionDenied,
    kUnsupportedAddressFamily,
    kInvalidAddressFormat,
    kEndOfFile,
    kAddressAlreadyInUse,
    kConnectionRefused,
    kConnectionResetByPeer,
    kTimeout,
    kFileAlreadyExists,
    kNoSpaceLeft,
    kSocketOperationOnNonSocket,
    kMemoryAllocationError,
    kInvalidMemoryAddress,
};

enum FileOpenMode {
    IO_OPEN_MODE_READ = 1 << 0,
    IO_OPEN_MODE_WRITE = 1 << 1,
    IO_OPEN_MODE_RW = IO_OPEN_MODE_READ | IO_OPEN_MODE_WRITE,

    IO_OPEN_MODE_CREATE = 1 << 2,
    IO_OPEN_MODE_CREATE_AND_FAIL_IF_EXISTS = 1 << 3,
    /**
     * Will set the length of a file to 0
     * Only works if file is open with READ and WRITE mode
     */
    IO_OPEN_MODE_SET_LENGTH_0 = 1 << 4,
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
typedef int SocketDescriptor;

class IO {
  public:

    /*
     * Special
     */
    virtual std::thread*    NewThread       (std::function<void()> function) const = 0;

    /*
     * Network
     */
    virtual SocketDescriptor  CreateSocket(AddressFamilies address_family, SocketTypes socket_type,
                                           SocketProtocols socket_protocol, IOErrors* err) const = 0;
    virtual void            Listen(SocketDescriptor socket_fd, int backlog, IOErrors* err) const = 0;
    virtual void            InetBind(SocketDescriptor socket_fd, const std::string& address, IOErrors* err) const = 0;
    virtual std::unique_ptr<std::tuple<std::string, SocketDescriptor>> InetAccept(SocketDescriptor socket_fd,
            IOErrors* err) const = 0;
    virtual void            InetConnect(SocketDescriptor socket_fd, const std::string& address, IOErrors* err) const = 0;
    virtual SocketDescriptor  CreateAndConnectIPTCPSocket(const std::string& address, IOErrors* err) const = 0;
    virtual size_t          Receive(SocketDescriptor socket_fd, void* buf, size_t length, IOErrors* err) const = 0;
    virtual size_t          ReceiveTimeout(SocketDescriptor socket_fd, void* buf, size_t length, long timeout_in_usec,
                                           IOErrors* err) const = 0;
    virtual size_t          Send(SocketDescriptor socket_fd, const void* buf, size_t length, IOErrors* err) const = 0;
    virtual void            Skip(SocketDescriptor socket_fd, size_t length, IOErrors* err) const = 0;
    /**
    * @param err Since CloseSocket if often used in an error case, it's able to accept nullptr.
    */
    virtual void            CloseSocket(SocketDescriptor socket_fd, IOErrors* err) const = 0;

    /*
     * Filesystem
     */
    virtual FileDescriptor  Open            (const std::string& filename, int open_flags, IOErrors* err) const = 0;
    /**
     * @param err Since Close if often used in an error case, it's able to accept nullptr.
     */
    virtual void            Close           (FileDescriptor fd, IOErrors* err) const = 0;

    virtual size_t          Read            (FileDescriptor fd, void* buf, size_t length, IOErrors* err) const = 0;
    virtual size_t          Write           (FileDescriptor fd, const void* buf, size_t length, IOErrors* err) const = 0;

    virtual void            CreateNewDirectory      (const std::string& directory_name, hidra2::IOErrors* err) const = 0;
    virtual FileData        GetDataFromFile         (const std::string& fname, uint64_t fsize, IOErrors* err) const = 0;
    virtual void CollectFileInformationRecursivly   (const std::string& path, std::vector<FileInfo>* files,
                                                     IOErrors* err) const = 0;
    virtual std::vector<FileInfo>   FilesInFolder   (const std::string& folder, IOErrors* err) const = 0;


};

}

#endif //HIDRA2_SYSTEM_WRAPPERS__IO_H
