#include "system_wrappers/system_io.h"

#include <cstring>

#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>

#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <zconf.h>

using std::string;
using std::vector;
using std::chrono::system_clock;

namespace hidra2 {

IOError IOErrorFromErrno() {
    switch (errno) {
    case 0:
        return IOError::NO_ERROR;
    case EBADF:
        return IOError::BAD_FILE_NUMBER;
    case ENOENT:
    case ENOTDIR:
        return IOError::FILE_NOT_FOUND;
    case EACCES:
        return IOError::PERMISSIONS_DENIED;
    case ECONNREFUSED:
        return IOError::CONNECTION_REFUSED;
    case EADDRINUSE:
        return IOError::ADDRESS_ALREADY_IN_USE;
    case ECONNRESET:
        return IOError::CONNECTION_RESET_BY_PEER;
    default:
        std::cout << "[IOErrorFromErrno] Unknown error code: " << errno << std::endl;
        return IOError::UNKNOWN_ERROR;
    }
}

sa_family_t AddressFamilyToPosixFamily(AddressFamilies address_family) {
    switch(address_family) {
    case AddressFamilies::INET:
        return AF_INET;
    }
    return -1;
};

int FileOpenModeToPosixFileOpenMode(FileOpenMode open_flags) {
    int flags = 0;
    if((open_flags & OPEN_MODE_READ && open_flags & OPEN_MODE_WRITE) || open_flags & OPEN_MODE_RW) {
        flags |= O_RDWR;
    } else {
        if (open_flags & OPEN_MODE_READ) {
            flags |= O_RDONLY;
        }
        if (open_flags & OPEN_MODE_WRITE) {
            flags |= O_WRONLY;
        }
    }
    if(open_flags & OPEN_MODE_CREATE) {
        flags |= O_CREAT;
    }
    if(open_flags & OPEN_MODE_SET_LENGTH_0) {
        flags |= O_TRUNC;
    }
    return flags;
}

hidra2::FileData hidra2::SystemIO::GetDataFromFile(const std::string& fname, IOError* err) {
    /*
     * TODO ?
     */
    /*
    FileDescriptor fd = open(fname, FileOpenMode::READ, err);
    *err = IOErrorFromErrno();
    if (*err != IOError::NO_ERROR) {
        return {};
    }
     */

}

bool IsDirectory(const struct dirent* entity) {
    return entity->d_type == DT_DIR &&
           strstr(entity->d_name, "..") == nullptr &&
           strstr(entity->d_name, ".") == nullptr;
}

system_clock::time_point GetTimePointFromFile(const string& fname, IOError* err) {

    struct stat t_stat {};
    int res = stat(fname.c_str(), &t_stat);
    if (res < 0) {
        *err = IOErrorFromErrno();
        return system_clock::time_point{};
    }

#ifdef __APPLE__
#define st_mtim st_mtimespec
#endif
    std::chrono::nanoseconds d = std::chrono::nanoseconds {t_stat.st_mtim.tv_nsec} +
                                 std::chrono::seconds{t_stat.st_mtim.tv_sec};
#ifdef __APPLE__
#undef st_mtim
#endif

    return system_clock::time_point {std::chrono::duration_cast<system_clock::duration>(d)};
}

void ProcessFileEntity(const struct dirent* entity, const std::string& path,
                       std::vector<FileInfo>& files, IOError* err) {

    *err = IOError::NO_ERROR;
    if (entity->d_type != DT_REG) {
        return;
    }
    FileInfo file_info;
    file_info.relative_path = path;
    file_info.base_name = entity->d_name;

    file_info.modify_date = GetTimePointFromFile(path + "/" + entity->d_name, err);
    if (*err != IOError::NO_ERROR) {
        return;
    }

    files.push_back(file_info);
}

void CollectFileInformationRecursivly(const std::string& path,
                                      std::vector<FileInfo>& files, IOError* err) {
    auto dir = opendir((path).c_str());
    if (dir == nullptr) {
        *err = IOErrorFromErrno();
        return;
    }

    while (struct dirent* current_entity = readdir(dir)) {
        if (IsDirectory(current_entity)) {
            CollectFileInformationRecursivly(path + "/" + current_entity->d_name,
                                             files, err);
        } else {
            ProcessFileEntity(current_entity, path, files, err);
        }
        if (*err != IOError::NO_ERROR) {
            closedir(dir);
            return;
        }
    }
    *err = IOErrorFromErrno();
    closedir(dir);
}

void SortFileList(std::vector<FileInfo>& file_list) {
    std::sort(file_list.begin(), file_list.end(),
    [](FileInfo const & a, FileInfo const & b) {
        return a.modify_date < b.modify_date;
    });
}

void StripBasePath(const string& folder, std::vector<FileInfo>& file_list) {
    auto n_erase = folder.size() + 1;
    for (auto& file : file_list) {
        file.relative_path.erase(0, n_erase);
    }
}

std::vector<FileInfo> SystemIO::FilesInFolder(const string& folder, IOError* err) {
    std::vector<FileInfo> files{};
    CollectFileInformationRecursivly(folder, files, err);
    if (*err != IOError::NO_ERROR) {
        return {};
    }
    StripBasePath(folder, files);
    SortFileList(files);
    return files;
}

hidra2::FileDescriptor hidra2::SystemIO::create_socket(hidra2::AddressFamilies address_family,
        hidra2::SocketTypes socket_type,
        hidra2::SocketProtocols socket_protocol,
        hidra2::IOError* err) {
    *err = IOError::NO_ERROR;

    int domain = AddressFamilyToPosixFamily(address_family);
    if(domain == -1) {
        *err = IOError::UNSUPPORTED_ADDRESS_FAMILY;
        return -1;
    }

    int type = 0;
    switch(socket_type) {
    case SocketTypes::STREAM:
        type = SOCK_STREAM;
        break;
    default:
        *err = IOError::UNKNOWN_ERROR;//TODO
        return -1;
    }

    int protocol = 0;
    switch(socket_protocol) {
    case SocketProtocols::IP:
        protocol = IPPROTO_IP;
        break;
    default:
        *err = IOError::UNKNOWN_ERROR;//TODO
        return -1;
    }

    int fd = ::socket(domain, type, protocol);

    if(fd == -1) {
        *err = IOErrorFromErrno();
        return -1;
    }

    return fd;
}

}

std::thread* hidra2::SystemIO::new_thread(std::function<void()> function) {
    return new std::thread(function);
}


void hidra2::SystemIO::inet_bind(hidra2::FileDescriptor socket_fd,
                                 const std::string& address,
                                 uint16_t port,
                                 hidra2::IOError* err) {
    *err = IOError::NO_ERROR;

    sa_family_t family = AddressFamilyToPosixFamily(AddressFamilies::INET);
    if(family == -1) {
        *err = IOError::UNSUPPORTED_ADDRESS_FAMILY;
        return;
    }

    sockaddr_in socket_address {};
    socket_address.sin_addr.s_addr  = inet_addr(address.c_str());
    socket_address.sin_port         = htons(port);
    socket_address.sin_family       = family;

    if(::bind(socket_fd, reinterpret_cast<const sockaddr*>(&socket_address), sizeof(socket_address)) == -1) {
        *err = IOErrorFromErrno();
    }

}

void hidra2::SystemIO::listen(hidra2::FileDescriptor socket_fd, int backlog, hidra2::IOError* err) {
    *err = IOError::NO_ERROR;

    if(::listen(socket_fd, backlog) == -1) {
        *err = IOErrorFromErrno();
    }
}
std::unique_ptr<std::tuple<std::string, hidra2::FileDescriptor>> hidra2::SystemIO::inet_accept(
            hidra2::FileDescriptor socket_fd,
IOError* err) {
    sa_family_t family = AddressFamilyToPosixFamily(AddressFamilies::INET);
    if(family == -1) {
        *err = IOError::UNSUPPORTED_ADDRESS_FAMILY;
        return nullptr;
    }

    sockaddr_in client_address {};
    socklen_t client_address_size = sizeof(sockaddr_in);

    int peer_fd = ::accept(socket_fd, reinterpret_cast<sockaddr*>(&client_address), &client_address_size);

    if(peer_fd == -1) {
        *err = IOErrorFromErrno();
        return nullptr;
    }

    std::string address = std::string(inet_ntoa(client_address.sin_addr)) + ':' + std::to_string(client_address.sin_port);
    return std::unique_ptr<std::tuple<std::string, hidra2::FileDescriptor>>(new
            std::tuple<std::string, hidra2::FileDescriptor>(address, peer_fd));
}

void hidra2::SystemIO::inet_connect(FileDescriptor socket_fd, const std::string& address, hidra2::IOError* err) {
    *err = IOError::NO_ERROR;
    std::string host;
    uint16_t port = 0;

    try {
        host = address.substr(0, address.find(':'));

        std::string port_str = address.substr(address.find(':') + 1, address.length());
        port = static_cast<uint16_t>(std::stoi(port_str));
    } catch(std::exception& e) {
        *err = IOError::INVALID_ADDRESS_FORMAT;
        return;
    }

    sa_family_t family = AddressFamilyToPosixFamily(AddressFamilies::INET);
    if(family == -1) {
        *err = IOError::UNSUPPORTED_ADDRESS_FAMILY;
        return;
    }

    sockaddr_in socket_address {};
    socket_address.sin_addr.s_addr = inet_addr(host.c_str());
    socket_address.sin_port = htons(port);
    socket_address.sin_family = family;

    if(::connect(socket_fd, (struct sockaddr*)&socket_address, sizeof(socket_address)) == -1) {
        *err = IOErrorFromErrno();
        return;
    }
}
hidra2::FileDescriptor hidra2::SystemIO::create_and_connect_ip_tcp_socket(const std::string& address,
        hidra2::IOError* err) {
    *err = hidra2::IOError::NO_ERROR;

    FileDescriptor fd = create_socket(AddressFamilies::INET, SocketTypes::STREAM, SocketProtocols::IP, err);
    if(*err != IOError::NO_ERROR) {
        return -1;
    }
    inet_connect(fd, address, err);
    if(*err != IOError::NO_ERROR) {
        close(fd);
        return -1;
    }

    return fd;
}

size_t hidra2::SystemIO::receive(hidra2::FileDescriptor socket_fd, void* buf, size_t length, hidra2::IOError* err) {
    *err = hidra2::IOError::NO_ERROR;

    size_t already_received = 0;

    while(already_received < length) {
        ssize_t received_amount = ::recv(socket_fd, (uint8_t*)buf + already_received, length - already_received, 0);
        if(received_amount == 0) {
            *err = IOError::STREAM_EOF;
            return already_received;
        }
        if(received_amount == -1) {
            *err = IOErrorFromErrno();
            return already_received;
        }
        already_received += received_amount;
    }

    return already_received;
}

size_t hidra2::SystemIO::receive_timeout(hidra2::FileDescriptor socket_fd,
                                         void* buf,
                                         size_t length,
                                         uint16_t timeout_in_sec,
                                         hidra2::IOError* err) {
    *err = hidra2::IOError::NO_ERROR;

    fd_set read_fds;
    FD_SET(socket_fd, &read_fds);
    timeval timeout;
    timeout.tv_sec = timeout_in_sec;
    timeout.tv_usec = 0;

    int res = select(socket_fd + 1, &read_fds, nullptr, nullptr, &timeout);
    if(res == 0) {
        *err = IOError::TIMEOUT;
        return 0;
    }
    if(res == -1) {
        *err = IOErrorFromErrno();
        return 0;
    }

    return receive(socket_fd, buf, length, err);
}

size_t hidra2::SystemIO::send(hidra2::FileDescriptor socket_fd,
                              const void* buf,
                              size_t length,
                              hidra2::IOError* err) {
    *err = hidra2::IOError::NO_ERROR;

    size_t already_sent = 0;

    while(already_sent < length) {
        ssize_t send_amount = ::send(socket_fd, (uint8_t*)buf + already_sent, length - already_sent, 0);
        if(send_amount == 0) {
            *err = IOError::STREAM_EOF;
            return already_sent;
        }
        if(send_amount == -1) {
            *err = IOErrorFromErrno();
            return already_sent;
        }
        already_sent += send_amount;
    }

    return already_sent;
}


hidra2::FileDescriptor hidra2::SystemIO::open(const std::string& filename,
                                              FileOpenMode open_flags,
                                              IOError* err) {
    *err = IOError::NO_ERROR;
    int flags = FileOpenModeToPosixFileOpenMode(open_flags);

    FileDescriptor fd = ::open(filename.c_str(), flags);
    if(fd == -1) {
        *err = IOErrorFromErrno();
    }

    return fd;
}

void hidra2::SystemIO::close(hidra2::FileDescriptor fd, hidra2::IOError* err) {
    int status = ::close(fd);
    if(!err) {
        return;
    }
    *err = IOError::NO_ERROR;
    if(status == -1) {
        *err = IOErrorFromErrno();
    }
}
