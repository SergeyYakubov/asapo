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


enum class IOErrorType {
    kUnknownIOError,
    kBadFileNumber,
    kResourceTemporarilyUnavailable,
    kFileNotFound,
    kReadError,
    kPermissionDenied,
    kUnsupportedAddressFamily,
    kInvalidAddressFormat,
    kAddressAlreadyInUse,
    kConnectionRefused,
    kConnectionResetByPeer,
    kTimeout,
    kFileAlreadyExists,
    kNoSpaceLeft,
    kSocketOperationOnNonSocket,
    kInvalidMemoryAddress,
    kUnableToResolveHostname,
    kSocketOperationUnknownAtLevel,
    kSocketOperationValueOutOfBound

};

class IOError : public SimpleError {
  private:
    IOErrorType io_error_type_;
  public:
    IOError(const std::string& error, IOErrorType io_error_type) : SimpleError(error, ErrorType::kIOError) {
        io_error_type_ = io_error_type;
    }

    IOErrorType GetIOErrorType() const noexcept {
        return io_error_type_;
    }
};

class IOErrorTemplate : public SimpleErrorTemplate {
  protected:
    IOErrorType io_error_type_;
  public:
    IOErrorTemplate(const std::string& error, IOErrorType io_error_type) : SimpleErrorTemplate(error, ErrorType::kIOError) {
        io_error_type_ = io_error_type;
    }

    inline IOErrorType GetIOErrorType() const noexcept {
        return io_error_type_;
    }

    inline Error Generate() const noexcept override {
        return Error(new IOError(error_, io_error_type_));
    }

    inline bool operator == (const Error& rhs) const override {
        return SimpleErrorTemplate::operator==(rhs)
               && GetIOErrorType() == ((IOError*)rhs.get())->GetIOErrorType();
    }
};

static inline std::ostream& operator<<(std::ostream& os, const IOErrorTemplate& err) {
    return os << err.Text();
}


namespace IOErrorTemplates {
auto const kUnknownIOError = IOErrorTemplate {
                                 "Unknown Error", IOErrorType::kUnknownIOError
                             };

auto const kFileNotFound = IOErrorTemplate {
                               "No such file or directory", IOErrorType::kFileNotFound
                           };
auto const kReadError = IOErrorTemplate {
                            "Read error", IOErrorType::kReadError
                        };
auto const kBadFileNumber = IOErrorTemplate {
                                "Bad file number", IOErrorType::kBadFileNumber
                            };
auto const kResourceTemporarilyUnavailable = IOErrorTemplate {
                                                 "Resource temporarily unavailable", IOErrorType::kResourceTemporarilyUnavailable
                                             };
auto const kPermissionDenied = IOErrorTemplate {
                                   "Permission denied", IOErrorType::kPermissionDenied
                               };
auto const kUnsupportedAddressFamily = IOErrorTemplate {
                                           "Unsupported address family", IOErrorType::kUnsupportedAddressFamily
                                       };
auto const kInvalidAddressFormat = IOErrorTemplate {
                                       "Invalid address format", IOErrorType::kInvalidAddressFormat
                                   };
auto const kAddressAlreadyInUse = IOErrorTemplate {
                                      "Address already in use", IOErrorType::kAddressAlreadyInUse
                                  };
auto const kConnectionRefused = IOErrorTemplate {
                                    "Connection refused", IOErrorType::kConnectionRefused
                                };
auto const kConnectionResetByPeer = IOErrorTemplate {
                                        "kConnectionResetByPeer", IOErrorType::kConnectionResetByPeer
                                    };
auto const kTimeout = IOErrorTemplate {
                          "kTimeout", IOErrorType::kTimeout
                      };
auto const kFileAlreadyExists = IOErrorTemplate {
                                    "kFileAlreadyExists", IOErrorType::kFileAlreadyExists
                                };
auto const kNoSpaceLeft = IOErrorTemplate {
                              "kNoSpaceLeft", IOErrorType::kNoSpaceLeft
                          };
auto const kSocketOperationOnNonSocket = IOErrorTemplate {
                                             "kSocketOperationOnNonSocket", IOErrorType::kSocketOperationOnNonSocket
                                         };
auto const kInvalidMemoryAddress = IOErrorTemplate {
                                       "kInvalidMemoryAddress", IOErrorType::kInvalidMemoryAddress
                                   };
auto const kUnableToResolveHostname = IOErrorTemplate {
                                          "kUnableToResolveHostname", IOErrorType::kUnableToResolveHostname
                                      };
auto const kSocketOperationUnknownAtLevel =  IOErrorTemplate {
                                                 "kSocketOperationUnknownAtLevel", IOErrorType::kSocketOperationUnknownAtLevel
                                             };

auto const kSocketOperationValueOutOfBound =  IOErrorTemplate {
                                                  "kSocketOperationValueOutOfBound", IOErrorType::kSocketOperationValueOutOfBound
                                              };


}

//Need to be "enum" since multiple flags are allowed
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
    virtual SocketDescriptor  CreateAndBindIPTCPSocketListener(const std::string& address, int backlog,
            Error* err) const = 0;
    virtual std::unique_ptr<std::tuple<std::string, SocketDescriptor>> InetAcceptConnection(SocketDescriptor socket_fd,
            Error* err) const = 0;
    virtual std::string     ResolveHostnameToIp(const std::string& hostname, Error* err) const = 0;
    virtual void            InetConnect(SocketDescriptor socket_fd, const std::string& address, Error* err) const = 0;
    virtual SocketDescriptor  CreateAndConnectIPTCPSocket(const std::string& address, Error* err) const = 0;
    virtual size_t          Receive(SocketDescriptor socket_fd, void* buf, size_t length, Error* err) const = 0;
    virtual size_t          ReceiveWithTimeout(SocketDescriptor socket_fd,
                                               void* buf,
                                               size_t length,
                                               long timeout_in_usec,
                                               Error* err) const = 0;
    virtual size_t          Send(SocketDescriptor socket_fd, const void* buf, size_t length, Error* err) const = 0;

    virtual void            Skip(SocketDescriptor socket_fd, size_t length, Error* err) const = 0;
    /**
     * @param err Since CloseSocket if often used in an error case, it's able to accept err as nullptr.
     */
    virtual void            CloseSocket(SocketDescriptor socket_fd, Error* err) const = 0;

    /*
     * Filesystem
     */
    virtual FileDescriptor  Open            (const std::string& filename, int open_flags, Error* err) const = 0;
    /**
     * @param err Since Close if often used in an error case, it's able to accept err as nullptr.
     */
    virtual void            Close           (FileDescriptor fd, Error* err) const = 0;

    virtual size_t          Read            (FileDescriptor fd, void* buf, size_t length, Error* err) const = 0;
    virtual size_t          Write           (FileDescriptor fd, const void* buf, size_t length, Error* err) const = 0;

    virtual void            CreateNewDirectory      (const std::string& directory_name, Error* err) const = 0;
    virtual FileData        GetDataFromFile         (const std::string& fname, uint64_t fsize, Error* err) const = 0;
    virtual void CollectFileInformationRecursively(const std::string& path, std::vector<FileInfo>* files,
                                                   Error* err) const = 0;
    virtual std::vector<FileInfo>   FilesInFolder   (const std::string& folder, Error* err) const = 0;
    virtual std::string     ReadFileToString        (const std::string& fname, Error* err) const = 0;

};

}

#endif //HIDRA2_SYSTEM_WRAPPERS__IO_H
