#ifndef HIDRA2_SYSTEM__SYSTEM_IO_H
#define HIDRA2_SYSTEM__SYSTEM_IO_H

#include "../../include/io/io.h"

#ifdef _WIN32
#include <windows.h>
#undef GetObject
#undef max
#undef min
typedef SSIZE_T ssize_t;
#endif

#if defined(__linux__) || defined (__APPLE__)
#include "../../../../../../../../usr/include/netinet/in.h"
#endif

namespace hidra2 {

class SystemIO final : public IO {
  private:
    static const int kNetBufferSize;//TODO: need to set by config

    void ApplyNetworkOptions(SocketDescriptor socket_fd, Error* err) const;

    //void CollectFileInformationRecursively(const std::string& path, std::vector<FileInfo>* files, IOErrors* err) const;
    int FileOpenModeToPosixFileOpenMode(int open_flags) const;
    Error GetLastError() const;

    short AddressFamilyToPosixFamily      (AddressFamilies address_family) const;
    int SocketTypeToPosixType           (SocketTypes socket_type) const;
    int SocketProtocolToPosixProtocol   (SocketProtocols socket_protocol) const;

    // System function mapping. Should only be called by the wrapper function
    FileDescriptor  _open(const char* filename, int posix_open_flags) const;
    bool            _close(FileDescriptor fd) const;
    int             _mkdir(const char* dirname) const;

    SocketDescriptor	_socket(int address_family, int socket_type, int socket_protocol) const;
    SocketDescriptor	_connect(SocketDescriptor socket_fd, const void* address, size_t address_length) const;
    int					_listen(SocketDescriptor socket_fd, int backlog) const;
    SocketDescriptor	_accept(SocketDescriptor socket_fd, void* address, size_t* address_length) const;
    bool			    _close_socket(SocketDescriptor socket_fd) const;

    void                                                InitializeSocketIfNecessary() const;
    std::unique_ptr<std::tuple<std::string, uint16_t>>  SplitAddressToHostnameAndPort(std::string address) const;

    FileInfo GetFileInfo(const std::string& name, Error* err) const;

    std::unique_ptr<sockaddr_in> BuildSockaddrIn(const std::string& address, Error* err) const;

    /*
     * Transfer functions
     */
    size_t Transfer(ssize_t (*method)(FileDescriptor, const void*, size_t), FileDescriptor fd, const void* buf,
                    size_t length,
                    Error* err) const;
    size_t Transfer(ssize_t (*method)(FileDescriptor, void*, size_t), FileDescriptor fd, void* buf, size_t length,
                    Error* err) const;
    static ssize_t	    _send(SocketDescriptor socket_fd, const void* buffer, size_t length);
    static ssize_t		_recv(SocketDescriptor socket_fd, void* buffer, size_t length);
    static ssize_t      _read(FileDescriptor fd, void* buffer, size_t length);
    static ssize_t      _write(FileDescriptor fd, const void* buffer, size_t count);

  public:
    /*
     * Special
     */
    std::unique_ptr<std::thread>    NewThread(std::function<void()> function) const;


    // this is not standard function - to be implemented differently in windows and linux
    std::vector<FileInfo>   FilesInFolder(const std::string& folder, Error* err) const;

    /*
     * Network
     */
    SocketDescriptor  CreateSocket(AddressFamilies address_family, SocketTypes socket_type, SocketProtocols socket_protocol,
                                   Error* err) const;
    void            Listen(SocketDescriptor socket_fd, int backlog, Error* err) const;
    void            InetBind(SocketDescriptor socket_fd, const std::string& address, Error* err) const;
    SocketDescriptor  CreateAndBindIPTCPSocketListener(const std::string& address, int backlog, Error* err) const;
    std::unique_ptr<std::tuple<std::string, SocketDescriptor>> InetAcceptConnection(SocketDescriptor socket_fd,
            Error* err) const;
    std::string     ResolveHostnameToIp(const std::string& hostname, Error* err) const;
    void            InetConnect(SocketDescriptor socket_fd, const std::string& address, Error* err) const;
    SocketDescriptor  CreateAndConnectIPTCPSocket(const std::string& address, Error* err) const;
    size_t          Receive(SocketDescriptor socket_fd, void* buf, size_t length, Error* err) const;
    size_t          ReceiveWithTimeout(SocketDescriptor socket_fd, void* buf, size_t length, long timeout_in_usec,
                                       Error* err) const;
    size_t          Send(SocketDescriptor socket_fd, const void* buf, size_t length, Error* err) const;
    void            Skip(SocketDescriptor socket_fd, size_t length, Error* err) const;
    void            CloseSocket(SocketDescriptor socket_fd, Error* err) const;

    /*
     * Filesystem
     */
    FileDescriptor  Open(const std::string& filename, int open_flags, Error* err) const;
    void            Close(FileDescriptor fd, Error* err) const;
    size_t          Read(FileDescriptor fd, void* buf, size_t length, Error* err) const;
    size_t          Write(FileDescriptor fd, const void* buf, size_t length, Error* err) const;
    void            CreateNewDirectory(const std::string& directory_name, Error* err) const;
    FileData        GetDataFromFile(const std::string& fname, uint64_t fsize, Error* err) const;
    Error           WriteDataToFile  (const std::string& fname, const FileData& data, size_t length) const;
    void            CollectFileInformationRecursively(const std::string& path, std::vector<FileInfo>* files,
                                                      Error* err) const;
    std::string     ReadFileToString(const std::string& fname, Error* err) const;
};
}

#endif //HIDRA2_SYSTEM__SYSTEM_IO_H
