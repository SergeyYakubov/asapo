#ifndef ASAPO_SYSTEM__SYSTEM_IO_H
#define ASAPO_SYSTEM__SYSTEM_IO_H

#include "../../include/io/io.h"

#ifdef _WIN32
#include <windows.h>
#undef GetObject
#undef max
#undef min
typedef SSIZE_T ssize_t;
#include <ostream>
#include <sstream>
#endif

#if defined(__linux__) || defined (__APPLE__)
#include "netinet/in.h"
#endif

#ifdef __APPLE__
#	define MSG_NOSIGNAL 0
#endif


namespace asapo {

class SystemIO final : public IO {
  private:
    static const int kNetBufferSize;//TODO: need to set by config
    static const size_t kMaxTransferChunkSize;
    static const size_t kReadWriteBufSize;

    static const int kWaitTimeoutMs;

#if defined(__linux__) || defined (__APPLE__)
    // used to for epoll - assumed single epoll instance per class instance
    const int kMaxEpollEvents = 10;
    mutable int epoll_fd_ = -1;
    Error AddToEpool(SocketDescriptor sd) const;
    Error CreateEpoolIfNeeded(SocketDescriptor master_socket) const;
    Error ProcessNewConnection(SocketDescriptor master_socket, std::vector<std::string>* new_connections,
                               ListSocketDescriptors* sockets_to_listen) const;
#endif

    void SetThreadName(std::thread* threadHandle, const std::string& name) const;

    void ApplyNetworkOptions(SocketDescriptor socket_fd, Error* err) const;

    //void CollectFileInformationRecursively(const std::string& path, std::vector<FileInfo>* files, IOErrors* err) const;
    int FileOpenModeToPosixFileOpenMode(int open_flags) const;

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
    void            CollectFileInformationRecursively(const std::string& path, std::vector<FileInfo>* files,
                                                      Error* err) const;
    void            GetSubDirectoriesRecursively(const std::string& path, SubDirList* subdirs, Error* err) const;
    Error           CreateDirectoryWithParents(const std::string& root_path, const std::string& path) const;
    uint8_t* AllocateArray(uint64_t fsize, Error* err) const;
    FileDescriptor OpenWithCreateFolders(const std::string& root_folder, const std::string& fname,
                                         bool create_directories, bool allow_ovewrite, Error* err) const;
  public:
    ~SystemIO();
    /*
     * Special thread functions, the name is limited to 15 chars.
     * More then 16 will result in a truncation.
     * Setting the name is a best effort feature and currently just works on UNIX systems.
     * The indexed function will add :<index> as an postfix to the name.
     */
    std::unique_ptr<std::thread> NewThread(const std::string& name,
                                           std::function<void()> function) const override;
    std::unique_ptr<std::thread> NewThread(const std::string& name,
                                           std::function<void(uint64_t index)> function, uint64_t index) const override;


    // this is not standard function - to be implemented differently in windows and linux
    std::vector<FileInfo>   FilesInFolder(const std::string& folder, Error* err) const override;

    /*
     * Network
     */

    ListSocketDescriptors WaitSocketsActivity(SocketDescriptor master_socket, ListSocketDescriptors* sockets_to_listen,
                                              std::vector<std::string>* new_connections, Error* err) const override;


    SocketDescriptor  CreateSocket(AddressFamilies address_family, SocketTypes socket_type, SocketProtocols socket_protocol,
                                   Error* err) const override;
    void            Listen(SocketDescriptor socket_fd, int backlog, Error* err) const override;
    void            InetBind(SocketDescriptor socket_fd, const std::string& address, Error* err) const override;
    SocketDescriptor  CreateAndBindIPTCPSocketListener(const std::string& address, int backlog, Error* err) const override;
    std::unique_ptr<std::tuple<std::string, SocketDescriptor>> InetAcceptConnection(SocketDescriptor socket_fd,
            Error* err) const override;
    std::string     ResolveHostnameToIp(const std::string& hostname, Error* err) const override;
    void            InetConnect(SocketDescriptor socket_fd, const std::string& address, Error* err) const override;
    SocketDescriptor  CreateAndConnectIPTCPSocket(const std::string& address, Error* err) const override;
    size_t          Receive(SocketDescriptor socket_fd, void* buf, size_t length, Error* err) const override;
    size_t          ReceiveWithTimeout(SocketDescriptor socket_fd, void* buf, size_t length, long timeout_in_usec,
                                       Error* err) const override;
    size_t          Send(SocketDescriptor socket_fd, const void* buf, size_t length, Error* err) const override;
    Error           SendFile(SocketDescriptor socket_fd, const std::string& fname, size_t length) const override;
    void            Skip(SocketDescriptor socket_fd, size_t length, Error* err) const override;
    void            CloseSocket(SocketDescriptor socket_fd, Error* err) const override;
    std::string     GetHostName(Error* err) const noexcept override;
    std::unique_ptr<std::tuple<std::string, uint16_t>> SplitAddressToHostnameAndPort(const std::string& address) const
                                                    override;

    /*
     * Filesystem
     */
    FileDescriptor  Open(const std::string& filename, int open_flags, Error* err) const override;
    void            Close(FileDescriptor fd, Error* err) const override;
    size_t          Read(FileDescriptor fd, void* buf, size_t length, Error* err) const override;
    size_t          Write(FileDescriptor fd, const void* buf, size_t length, Error* err) const override;
    void            CreateNewDirectory(const std::string& directory_name, Error* err) const override;
    FileData        GetDataFromFile(const std::string& fname, uint64_t* fsize, Error* err) const override;
    Error           WriteDataToFile  (const std::string& root_folder, const std::string& fname, const FileData& data,
                                      size_t length, bool create_directories, bool allow_ovewrite) const override;
    Error           ReceiveDataToFile(SocketDescriptor socket, const std::string& root_folder, const std::string& fname,
                                      size_t length, bool create_directories, bool allow_ovewrite) const override;

    Error           WriteDataToFile(const std::string& root_folder, const std::string& fname, const uint8_t* data,
                                    size_t length, bool create_directories, bool allow_ovewrite) const override;
    SubDirList      GetSubDirectories(const std::string& path, Error* err) const override;
    std::string     ReadFileToString(const std::string& fname, Error* err) const override;
    Error           RemoveFile(const std::string& fname) const override;
    Error           GetLastError() const override;
    std::string     AddressFromSocket(SocketDescriptor socket) const noexcept override;
    FileInfo        GetFileInfo(const std::string& name, Error* err) const override;


};
}

#endif //ASAPO_SYSTEM__SYSTEM_IO_H
