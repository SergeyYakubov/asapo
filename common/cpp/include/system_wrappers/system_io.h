#ifndef HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
#define HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H

#include "io.h"

namespace hidra2 {

class SystemIO final : public IO {
  private:
    //void CollectFileInformationRecursivly(const std::string& path, std::vector<FileInfo>* files, IOErrors* err) const;
  public:
    /*
     * Special
     */
    std::thread*            NewThread(std::function<void()> function) const;
    FileData                GetDataFromFile(const std::string& fname, uint64_t fsize, IOErrors* err) const;

    // this is not standard function - to be implemented differently in windows and linux
    std::vector<FileInfo>   FilesInFolder   (const std::string& folder, IOErrors* err) const;

    /*
     * Network
     */
    FileDescriptor  CreateSocket(AddressFamilies address_family, SocketTypes socket_type,
                                 SocketProtocols socket_protocol, IOErrors* err) const;
    void            Listen(FileDescriptor socket_fd, int backlog, IOErrors* err) const;
    void            InetBind(FileDescriptor socket_fd, const std::string& address, uint16_t port,
                             IOErrors* err) const;
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
    size_t          Write(FileDescriptor fd, const void* buf, size_t length, IOErrors* err) const;
    uint64_t        Read(int fd, uint8_t* array, uint64_t fsize, IOErrors* err) const;
    void            CreateDirectory(const std::string& directory_name, hidra2::IOErrors* err) const;

};
}

#endif //HIDRA2_SYSTEM_WRAPPERS__SYSTEM_IO_H
