#ifndef ASAPO_SYSTEM__IO_H
#define ASAPO_SYSTEM__IO_H

#include <cinttypes>

#include <string>
#include <vector>
#include <chrono>
#include <thread>

#include "common/data_structs.h"
#include "common/io_error.h"

namespace asapo {

//Need to be "enum" since multiple flags are allowed
enum FileOpenMode : unsigned short {
    IO_OPEN_MODE_READ = 1,
    IO_OPEN_MODE_WRITE = 1 << 1,
    IO_OPEN_MODE_RW = 1 << 2,
    IO_OPEN_MODE_CREATE = 1 << 3,
    IO_OPEN_MODE_CREATE_AND_FAIL_IF_EXISTS = 1 << 4,
    /**
     * Will set the length of a file to 0
     * Only works if file is open with READ and WRITE mode
     */
    IO_OPEN_MODE_SET_LENGTH_0 = 1 << 5,
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

using FileDescriptor = int;
using SocketDescriptor = int;
using ListSocketDescriptors =  std::vector<SocketDescriptor>;
const SocketDescriptor kDisconnectedSocketDescriptor = -1;

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

    virtual ListSocketDescriptors WaitSocketsActivity(SocketDescriptor master_socket,
                                                      ListSocketDescriptors* sockets_to_listen,
                                                      std::vector<std::string>* new_connections, Error* err) const = 0;


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
    virtual Error           RemoveFile(const std::string& fname) const = 0;
    virtual Error          WriteDataToFile  (const std::string& root_folder, const std::string& fname, const FileData& data,
                                             size_t length, bool create_directories) const = 0;
    virtual Error          WriteDataToFile  (const std::string& root_folder, const std::string& fname, const uint8_t* data,
                                             size_t length, bool create_directories) const = 0;

    virtual void            CreateNewDirectory      (const std::string& directory_name, Error* err) const = 0;
    virtual FileData        GetDataFromFile         (const std::string& fname, uint64_t* fsize, Error* err) const = 0;
    virtual SubDirList      GetSubDirectories(const std::string& path, Error* err) const = 0;
    virtual std::vector<FileInfo>   FilesInFolder   (const std::string& folder, Error* err) const = 0;
    virtual std::string     ReadFileToString        (const std::string& fname, Error* err) const = 0;
    virtual Error GetLastError() const = 0;
    virtual std::string AddressFromSocket(SocketDescriptor socket) const noexcept = 0;
};

}

#endif //ASAPO_SYSTEM__IO_H
