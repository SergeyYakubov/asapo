#include <fcntl.h>
#include <system_wrappers/system_io.h>
#include <cassert>
#include <algorithm>

namespace hidra2 {

/*******************************************************************************
 *                              system_io.cpp                                  *
 * THIS FILE HOLDS GENERAL FUNCTIONS THAT CAN BE USED ON WINDOWS AND ON LINUX  *
 *******************************************************************************/

// PRIVATE FUNCTIONS - START
void SortFileList(std::vector<FileInfo>* file_list) {
    std::sort(file_list->begin(), file_list->end(),
    [](FileInfo const & a, FileInfo const & b) {
        return a.modify_date < b.modify_date;
    });
}

void StripBasePath(const std::string& folder, std::vector<FileInfo>* file_list) {
    auto n_erase = folder.size() + 1;
    for (auto& file : *file_list) {
        file.relative_path.erase(0, n_erase);
    }
}

// PRIVATE FUNCTIONS - END

std::thread* hidra2::SystemIO::NewThread(std::function<void()> function) const {
    return new std::thread(function);
}

void hidra2::SystemIO::Skip(hidra2::FileDescriptor socket_fd, size_t length, hidra2::IOErrors* err) const {
    static const size_t kSkipBufferSize = 1024;

    //TODO need to find a better way to skip bytes
    *err = IOErrors::kNoError;
    std::unique_ptr<uint8_t[]> buffer;
    try {
        buffer.reset(new uint8_t[kSkipBufferSize]);
    } catch(std::exception& e) {
        assert(false);
        *err = IOErrors::kUnknownError;
        return;
    }
    size_t already_skipped = 0;
    while(already_skipped < length) {
        size_t need_to_skip = length - already_skipped;
        if(need_to_skip > kSkipBufferSize)
            need_to_skip = kSkipBufferSize;
        size_t skipped_amount = Receive(socket_fd, buffer.get(), need_to_skip, err);
        if(*err != IOErrors::kNoError) {
            return;
        }
        already_skipped += skipped_amount;
    }
}

hidra2::FileDescriptor hidra2::SystemIO::CreateAndConnectIPTCPSocket(const std::string& address,
        hidra2::IOErrors* err) const {
    *err = hidra2::IOErrors::kNoError;

    FileDescriptor fd = CreateSocket(AddressFamilies::INET, SocketTypes::STREAM, SocketProtocols::IP, err);
    if(*err != IOErrors::kNoError) {
        return -1;
    }
    InetConnect(fd, address, err);
    if(*err != IOErrors::kNoError) {
        Close(fd, nullptr);
        return -1;
    }

    return fd;
}

int SystemIO::FileOpenModeToPosixFileOpenMode(int open_flags) const {
    int flags = 0;
    if((open_flags & IO_OPEN_MODE_READ && open_flags & IO_OPEN_MODE_WRITE) || open_flags & IO_OPEN_MODE_RW) {
        flags |= O_RDWR;
    } else {
        if (open_flags & IO_OPEN_MODE_READ) {
            flags |= O_RDONLY;
        }
        if (open_flags & IO_OPEN_MODE_WRITE) {
            flags |= O_WRONLY;
        }
    }
    if(open_flags & IO_OPEN_MODE_CREATE) {
        flags |= O_CREAT;
    }
    if(open_flags & IO_OPEN_MODE_CREATE_AND_FAIL_IF_EXISTS) {
        flags |= O_CREAT | O_EXCL;
    }
    if(open_flags & IO_OPEN_MODE_SET_LENGTH_0) {
        flags |= O_TRUNC;
    }
    return flags;
}


hidra2::FileDescriptor hidra2::SystemIO::Open(const std::string& filename,
                                              int open_flags,
                                              IOErrors* err) const {
    int flags = FileOpenModeToPosixFileOpenMode(open_flags);
    FileDescriptor fd = _open(filename.c_str(), flags);
    *err = GetLastError();
    return fd;
}

void hidra2::SystemIO::Close(hidra2::FileDescriptor fd, hidra2::IOErrors* err) const {
    _close(fd);
    if(err) {
        *err = GetLastError();
    }
}

size_t hidra2::SystemIO::Read(FileDescriptor fd, void* buf, size_t length, IOErrors* err) const {
    *err = hidra2::IOErrors::kNoError;

    size_t already_read = 0;

    while(already_read < length) {
        ssize_t received_amount = _read(fd, (uint8_t*)buf + already_read, length - already_read);
        if(received_amount == 0) {
            *err = IOErrors::kEndOfFile;
            return already_read;
        }
        *err = GetLastError();
        if (*err != IOErrors::kNoError) {
            return already_read;
        }
        already_read += received_amount;
    }

    return already_read;
}

size_t hidra2::SystemIO::Write(FileDescriptor fd, const void* buf, size_t length, IOErrors* err) const {
    *err = hidra2::IOErrors::kNoError;

    size_t already_wrote = 0;

    while(already_wrote < length) {
        ssize_t send_amount = _write(fd, (uint8_t*)buf + already_wrote, length - already_wrote);
        if(send_amount == 0) {
            *err = IOErrors::kEndOfFile;
            return already_wrote;
        }
        *err = GetLastError();
        if (*err != IOErrors::kNoError) {
            return already_wrote;
        }
        already_wrote += send_amount;
    }

    return already_wrote;
}

FileDescriptor SystemIO::CreateSocket(AddressFamilies address_family,
                                      SocketTypes socket_type,
                                      SocketProtocols socket_protocol,
                                      IOErrors* err) const {
    *err = IOErrors::kNoError;

    int domain = AddressFamilyToPosixFamily(address_family);
    if(domain == -1) {
        *err = IOErrors::kUnsupportedAddressFamily;
        return -1;
    }

    int type = SocketTypeToPosixType(socket_type);
    if(type == -1) {
        *err = IOErrors::kUnknownError;
        return -1;
    }

    int protocol = SocketProtocolToPosixProtocol(socket_protocol);
    if(protocol == -1) {
        *err = IOErrors::kUnknownError;
        return -1;
    }

    int fd = _socket(domain, type, protocol);
    *err = GetLastError();
    return fd;
}

void hidra2::SystemIO::Listen(hidra2::FileDescriptor socket_fd, int backlog, hidra2::IOErrors* err) const {
    *err = IOErrors::kNoError;

    if (_listen(socket_fd, backlog) == -1) {
        *err = GetLastError();
    }
}

size_t hidra2::SystemIO::Receive(hidra2::FileDescriptor socket_fd, void* buf, size_t length,
                                 hidra2::IOErrors* err) const {
    *err = hidra2::IOErrors::kNoError;

    size_t already_received = 0;

    while (already_received < length) {
        ssize_t received_amount = _recv(socket_fd, (uint8_t*) buf + already_received, length - already_received);
        if (received_amount == 0) {
            *err = IOErrors::kEndOfFile;
            return already_received;
        }
        *err = GetLastError();
        if (*err != IOErrors::kNoError) {
            return already_received;
        }
        already_received += received_amount;
    }

    return already_received;
}


size_t hidra2::SystemIO::Send(hidra2::FileDescriptor socket_fd,
                              const void* buf,
                              size_t length,
                              hidra2::IOErrors* err) const {
    *err = hidra2::IOErrors::kNoError;

    size_t already_sent = 0;

    while (already_sent < length) {
        ssize_t send_amount = _send(socket_fd, (uint8_t*) buf + already_sent, length - already_sent);
        if (send_amount == 0) {
            *err = IOErrors::kEndOfFile;
            return already_sent;
        }
        *err = GetLastError();
        if (*err != IOErrors::kNoError) {
            return already_sent;
        }
        already_sent += send_amount;
    }

    return already_sent;
}


hidra2::FileData hidra2::SystemIO::GetDataFromFile(const std::string& fname, uint64_t fsize, IOErrors* err) const {
    *err = IOErrors::kNoError;
    FileDescriptor fd = Open(fname, IO_OPEN_MODE_READ, err);
    if (*err != IOErrors::kNoError) {
        return nullptr;
    }
    uint8_t* data_array = nullptr;
    try {
        data_array = new uint8_t[fsize];
    } catch (...) {
        *err = IOErrors::kMemoryAllocationError;
        return nullptr;
    }
    FileData data{data_array};
    Read(fd, data_array, fsize, err);
    if (*err != IOErrors::kNoError) {
        Close(fd, nullptr);
        return nullptr;
    }

    Close(fd, nullptr);
    *err = GetLastError();
    if (*err != IOErrors::kNoError) {
        return nullptr;
    }

    return data;
}

std::vector<hidra2::FileInfo> hidra2::SystemIO::FilesInFolder(const std::string& folder, hidra2::IOErrors* err) const {
    std::vector<FileInfo> files{};
    CollectFileInformationRecursivly(folder, &files, err);
    if (*err != IOErrors::kNoError) {
        return {};
    }
    StripBasePath(folder, &files);
    SortFileList(&files);
    return files;
}

void hidra2::SystemIO::CreateDirectory(const std::string& directory_name, hidra2::IOErrors* err) const {
    _mkdir(directory_name.c_str());
    *err = GetLastError();
}
std::unique_ptr<std::tuple<std::string, uint16_t>> SystemIO::SplitAddressToHostAndPort(std::string address) const {
    try {
        std::string host = address.substr(0, address.find(':'));

        std::string port_str = address.substr(address.find(':') + 1, address.length());
        uint16_t port = static_cast<uint16_t>(std::stoi(port_str));

        return std::unique_ptr<std::tuple<std::string, uint16_t>>(new std::tuple<std::string, uint16_t>(host, port));
    } catch (...) {
        return nullptr;
    }
}

}


