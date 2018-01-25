#ifndef HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
#define HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H

#include "io.h"

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

namespace hidra2 {

class SystemIO final : public IO {
  private:
    //void CollectFileInformationRecursivly(const std::string& path, std::vector<FileInfo>* files, IOErrors* err) const;
    int FileOpenModeToPosixFileOpenMode(int open_flags) const;
    IOErrors GetLastError() const;

    int AddressFamilyToPosixFamily      (AddressFamilies address_family) const;
    int SocketTypeToPosixType           (SocketTypes socket_type) const;
    int SocketProtocolToPosixProtocol   (SocketProtocols socket_protocol) const;

    // Function maps. Should never be called apart from in wrapper function
    FileDescriptor  _open(const char* filename, int posix_open_flags) const;
    void            _close(FileDescriptor fd) const;
    ssize_t         _read(FileDescriptor fd, void* buffer, size_t length) const;
    ssize_t         _write(FileDescriptor fd, const void* buffer, size_t count) const;
    FileDescriptor  _socket(int address_family, int socket_type, int socket_protocol) const;
    int             _listen(FileDescriptor fd, int backlog) const;
    ssize_t         _send(FileDescriptor socket_fd, const void* buffer, size_t length) const;
    ssize_t         _recv(FileDescriptor socket_fd, void* buffer, size_t length) const;
    int             _mkdir(const char* dirname) const;

    std::unique_ptr<std::tuple<std::string, uint16_t>> SplitAddressToHostAndPort(std::string address) const;

  public:
    /*
     * Special
     */
    std::thread*            NewThread(std::function<void()> function) const;


    // this is not standard function - to be implemented differently in windows and linux
    std::vector<FileInfo>   FilesInFolder(const std::string& folder, IOErrors* err) const;

    /*
     * Network
     */
    FileDescriptor  CreateSocket(AddressFamilies address_family, SocketTypes socket_type,
                                 SocketProtocols socket_protocol, IOErrors* err) const;
    void            Listen(FileDescriptor socket_fd, int backlog, IOErrors* err) const;
    void            InetBind(FileDescriptor socket_fd, const std::string& address, IOErrors* err) const;
    std::unique_ptr<std::tuple<std::string, FileDescriptor>> InetAccept(FileDescriptor socket_fd, IOErrors* err) const;
    void            InetConnect(FileDescriptor socket_fd, const std::string& address, IOErrors* err) const;
    FileDescriptor  CreateAndConnectIPTCPSocket(const std::string& address, IOErrors* err) const;

    size_t          Receive(FileDescriptor socket_fd, void* buf, size_t length, IOErrors* err) const;
    size_t          ReceiveTimeout(FileDescriptor socket_fd,
                                   void* buf,
                                   size_t length,
                                   uint16_t timeout_in_sec,
                                   IOErrors* err) const;
    size_t          Send(FileDescriptor socket_fd, const void* buf, size_t length, IOErrors* err) const;
    void            Skip(FileDescriptor socket_fd, size_t length, IOErrors* err) const;

    /*
     * Filesystem
     */
    FileDescriptor  Open(const std::string& filename, int open_flags, IOErrors* err) const;
    void            Close(FileDescriptor fd, IOErrors* err) const;
    size_t          Read(FileDescriptor fd, void* buf, size_t length, IOErrors* err) const;
    size_t          Write(FileDescriptor fd, const void* buf, size_t length, IOErrors* err) const;
    void            CreateDirectory(const std::string& directory_name, hidra2::IOErrors* err) const;
    FileData        GetDataFromFile(const std::string& fname, uint64_t fsize, IOErrors* err) const;
    void            CollectFileInformationRecursivly(const std::string& path, std::vector<FileInfo>* files,
                                                     IOErrors* err) const;
};
}

#endif //HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
