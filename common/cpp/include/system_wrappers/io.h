#ifndef HIDRA2_SYSTEM_WRAPPERS__IO_H
#define HIDRA2_SYSTEM_WRAPPERS__IO_H

#include <cinttypes>

#include <string>
#include <vector>
#include <chrono>
#include <thread>

#include "common/data_structs.h"
#include "common/error.h"

namespace hidra2 {

namespace IOErrorTemplate {
auto const kUnknownError = ErrorTemplate{"Unknown Error", ErrorType::kUnknownError};

auto const kFileNotFound = ErrorTemplate{"No such file or directory", ErrorType::kFileNotFound};
auto const kReadError = ErrorTemplate{"Read error", ErrorType::kFileNotFound};
auto const kBadFileNumber = ErrorTemplate{"Bad file number", ErrorType::kBadFileNumber};
auto const kResourceTemporarilyUnavailable = ErrorTemplate{"Resource temporarily unavailable", ErrorType::kResourceTemporarilyUnavailable};
auto const kPermissionDenied = ErrorTemplate{"Permission denied", ErrorType::kPermissionDenied};
auto const kUnsupportedAddressFamily = ErrorTemplate{"Unsupported address family", ErrorType::kUnsupportedAddressFamily};
auto const kInvalidAddressFormat = ErrorTemplate{"Invalid address format", ErrorType::kInvalidAddressFormat};
auto const kEndOfFile = ErrorTemplate{"End of file", ErrorType::kEndOfFile};
auto const kAddressAlreadyInUse = ErrorTemplate{"Address already in use", ErrorType::kAddressAlreadyInUse};
auto const kConnectionRefused = ErrorTemplate{"Connection refused", ErrorType::kConnectionRefused};
auto const kConnectionResetByPeer = ErrorTemplate{"kConnectionResetByPeer", ErrorType::kConnectionResetByPeer};
auto const kTimeout = ErrorTemplate{"kTimeout", ErrorType::kTimeout};
auto const kFileAlreadyExists = ErrorTemplate{"kFileAlreadyExists", ErrorType::kFileAlreadyExists};
auto const kNoSpaceLeft = ErrorTemplate{"kNoSpaceLeft", ErrorType::kNoSpaceLeft};
auto const kSocketOperationOnNonSocket = ErrorTemplate{"kSocketOperationOnNonSocket", ErrorType::kSocketOperationOnNonSocket};
auto const kMemoryAllocationError = ErrorTemplate{"kMemoryAllocationError", ErrorType::kMemoryAllocationError};
auto const kInvalidMemoryAddress = ErrorTemplate{"kInvalidMemoryAddress", ErrorType::kInvalidMemoryAddress};
auto const kUnableToResolveHostname = ErrorTemplate{"kUnableToResolveHostname", ErrorType::kUnableToResolveHostname};
}

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
    virtual std::unique_ptr<std::thread> NewThread       (std::function<void()> function) const = 0;

    /*
     * Network
     */
    virtual SocketDescriptor  CreateSocket(AddressFamilies address_family, SocketTypes socket_type,
                                           SocketProtocols socket_protocol, Error* err) const = 0;
    virtual void            Listen(SocketDescriptor socket_fd, int backlog, Error* err) const = 0;
    virtual void            InetBind(SocketDescriptor socket_fd, const std::string& address, Error* err) const = 0;
    virtual std::unique_ptr<std::tuple<std::string, SocketDescriptor>> InetAccept(SocketDescriptor socket_fd,
            Error* err) const = 0;
    virtual std::string     ResolveHostnameToIp(const std::string& hostname, Error* err) const = 0;
    virtual void            InetConnect(SocketDescriptor socket_fd, const std::string& address, Error* err) const = 0;
    virtual SocketDescriptor  CreateAndConnectIPTCPSocket(const std::string& address, Error* err) const = 0;
    virtual size_t          Receive(SocketDescriptor socket_fd, void* buf, size_t length, Error* err) const = 0;
    virtual size_t          ReceiveTimeout(SocketDescriptor socket_fd, void* buf, size_t length, long timeout_in_usec,
                                           Error* err) const = 0;
    virtual size_t          Send(SocketDescriptor socket_fd, const void* buf, size_t length, Error* err) const = 0;
    virtual void            Skip(SocketDescriptor socket_fd, size_t length, Error* err) const = 0;
    /**
    * @param err Since CloseSocket if often used in an Error* case, it's able to accept err as nullptr.
    */
    virtual void            CloseSocket(SocketDescriptor socket_fd, Error* err) const = 0;

    /*
     * Filesystem
     */
    virtual FileDescriptor  Open            (const std::string& filename, int open_flags, Error* err) const = 0;
    /**
     * @param err Since Close if often used in an Error* case, it's able to accept err as nullptr.
     */
    virtual void            Close           (FileDescriptor fd, Error* err) const = 0;

    virtual size_t          Read            (FileDescriptor fd, void* buf, size_t length, Error* err) const = 0;
    virtual size_t          Write           (FileDescriptor fd, const void* buf, size_t length, Error* err) const = 0;

    virtual void            CreateNewDirectory      (const std::string& directory_name, Error* err) const = 0;
    virtual FileData        GetDataFromFile         (const std::string& fname, uint64_t fsize, Error* err) const = 0;
    virtual void CollectFileInformationRecursivly   (const std::string& path, std::vector<FileInfo>* files,
                                                     Error* err) const = 0;
    virtual std::vector<FileInfo>   FilesInFolder   (const std::string& folder, Error* err) const = 0;
    virtual std::string     ReadFileToString        (const std::string& fname, Error* err) const = 0;

};

}

#endif //HIDRA2_SYSTEM_WRAPPERS__IO_H
