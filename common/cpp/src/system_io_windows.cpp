#include "system_wrappers/system_io.h"

#include <cstring>
#include <sys/stat.h>
#include <algorithm>
#include <io.h>
#include <windows.h>
#include <direct.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

using std::string;
using std::vector;
using std::chrono::system_clock;

namespace hidra2 {

IOErrors IOErrorFromGetLastError() {
	DWORD last_error = GetLastError();
    switch (last_error) {
    case ERROR_SUCCESS :
		return IOErrors::kNoError;
    case ERROR_PATH_NOT_FOUND:
    case ERROR_FILE_NOT_FOUND:
		return IOErrors::kFileNotFound;
    case ERROR_ACCESS_DENIED:
		return IOErrors::kPermissionDenied;
	case ERROR_CONNECTION_REFUSED:
		return IOErrors::kConnectionRefused;
	case WSAEFAULT:
		return IOErrors::kInvalidMemoryAddress;
	case WSAECONNRESET:
		return IOErrors::kConnectionResetByPeer;
	case WSAENOTSOCK:
		return IOErrors::kSocketOperationOnNonSocket;
    default:
		std::cout << "[IOErrorFromGetLastError] Unknown error code: " << last_error << std::endl;
		return IOErrors::kUnknownError;
    }
}


IOErrors SystemIO::GetLastError() const {
    return IOErrorFromGetLastError();
}

IOErrors CheckFileTime(const FILETIME& ft) {
    SYSTEMTIME st = {0};
    if (!FileTimeToSystemTime(&ft, &st)) {
        return IOErrorFromGetLastError();
    }
    return IOErrors::kNoError;
}

constexpr auto kShift = 11644473600ULL;
constexpr auto k100nsInSec = 10000000ULL;

uint64_t GetLinuxEpochSecFromWindowsEpoch(ULARGE_INTEGER ull) {
//    ull.QuadPart is amount of 100ns intervals since Windows Epoch
    return (uint64_t) ull.QuadPart / k100nsInSec - kShift;
}

uint64_t GetLinuxNanosecFromWindowsEpoch(ULARGE_INTEGER ull) {
    return (uint64_t)(ull.QuadPart % k100nsInSec) * 100;
}

std::chrono::system_clock::time_point FileTime2TimePoint(const FILETIME& ft, IOErrors* err) {

    *err = CheckFileTime(ft);
    if (*err != IOErrors::kNoError) {
        return std::chrono::system_clock::time_point{};
    }

    // number of seconds
    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;

    auto sec = GetLinuxEpochSecFromWindowsEpoch(ull);
    auto nsec = GetLinuxNanosecFromWindowsEpoch(ull);

    std::chrono::nanoseconds d = std::chrono::nanoseconds {nsec} +
                                 std::chrono::seconds{sec};

    auto tp = system_clock::time_point
    {std::chrono::duration_cast<std::chrono::system_clock::duration>(d)};

    *err = IOErrors::kNoError;
    return tp;
}

bool IsDirectory(const WIN32_FIND_DATA f) {
    return (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
           strstr(f.cFileName, "..") == nullptr &&
           strstr(f.cFileName, ".") == nullptr;
}

void ProcessFileEntity(const WIN32_FIND_DATA f, const std::string& path,
                       std::vector<FileInfo>* files, IOErrors* err) {

    *err = IOErrors::kNoError;
    if (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        return;
    }

    FileInfo file_info;
    file_info.modify_date = FileTime2TimePoint(f.ftLastWriteTime, err);
    if (*err != IOErrors::kNoError) {
        return;
    }

    file_info.base_name = f.cFileName;
    file_info.relative_path = path;
    files->push_back(file_info);
}


void SystemIO::CollectFileInformationRecursivly(const std::string& path,
                                                std::vector<FileInfo>* files, IOErrors* err) const {
    WIN32_FIND_DATA find_data;
    HANDLE handle = FindFirstFile((path + "\\*.*").c_str(), &find_data);
    if (handle == INVALID_HANDLE_VALUE) {
        *err = IOErrorFromGetLastError();
        return;
    }

    do {
        if (IsDirectory(find_data)) {
            CollectFileInformationRecursivly(path + "\\" + find_data.cFileName, files, err);
        } else {
            ProcessFileEntity(find_data, path, files, err);
        }
        if (*err != IOErrors::kNoError) {
            FindClose(handle);
            return;
        }
    } while (FindNextFile(handle, &find_data));

    if (FindClose(handle)) {
        *err = IOErrors ::kNoError;
    } else {
        *err = IOErrorFromGetLastError();
    }
}

std::unique_ptr<std::tuple<std::string, SocketDescriptor>> SystemIO::InetAccept(SocketDescriptor socket_fd, IOErrors* err) const {
	*err = IOErrors::kNoError;
	static short family = AddressFamilyToPosixFamily(AddressFamilies::INET);
	if (family == -1) {
		*err = IOErrors::kUnsupportedAddressFamily;
		return nullptr;
	}

	sockaddr_in client_address{};
	static int client_address_size = sizeof(sockaddr_in);

	int peer_fd = ::accept(socket_fd, reinterpret_cast<sockaddr*>(&client_address), &client_address_size);

	if (peer_fd == -1) {
		*err = GetLastError();
		return nullptr;
	}

	std::string
		address = std::string(inet_ntoa(client_address.sin_addr)) + ':' + std::to_string(client_address.sin_port);
	return std::unique_ptr<std::tuple<std::string, SocketDescriptor>>(new
		std::tuple<std::string,
		SocketDescriptor>(
			address,
			peer_fd));
}

void hidra2::SystemIO::InetConnect(SocketDescriptor socket_fd, const std::string& address, IOErrors* err) const {
	*err = IOErrors::kNoError;

	auto host_port_tuple = SplitAddressToHostAndPort(address);
	if (!host_port_tuple) {
		*err = IOErrors::kInvalidAddressFormat;
		return;
	}
	std::string host;
	uint16_t port = 0;
	std::tie(host, port) = *host_port_tuple;

	short family = AddressFamilyToPosixFamily(AddressFamilies::INET);
	if (family == -1) {
		*err = IOErrors::kUnsupportedAddressFamily;
		return;
	}

	sockaddr_in socket_address{};
	socket_address.sin_addr.s_addr = inet_addr(host.c_str());
	socket_address.sin_port = htons(port);
	socket_address.sin_family = family;

	if (::connect(socket_fd, (struct sockaddr*) &socket_address, sizeof(socket_address)) == -1) {
		*err = GetLastError();
		return;
	}
}

FileDescriptor SystemIO::_open(const char* filename, int posix_open_flags) const {
    int fd;
    errno = _sopen_s(&fd, filename, posix_open_flags, _SH_DENYNO, _S_IREAD | _S_IWRITE);
    return fd;
}

void SystemIO::_close(FileDescriptor fd) const {
	::_close(fd);
}

void SystemIO::_close_socket(SocketDescriptor fd) const {
	::closesocket(fd);
}

ssize_t SystemIO::_read(FileDescriptor fd, void* buffer, size_t length) const {
    return ::_read(fd, (char*)buffer, length);
}

ssize_t SystemIO::_write(FileDescriptor fd, const void* buffer, size_t length) const {
    return ::_write(fd, (const char*)buffer, length);
}

SocketDescriptor SystemIO::_socket(int address_family, int socket_type, int socket_protocol) const {
	static bool WSAStartupDone = false;
	if (!WSAStartupDone) {
		WSAStartupDone = true;
		WORD wVersionRequested = MAKEWORD(2, 2);
		WSADATA wsaData;
		int err = WSAStartup(wVersionRequested, &wsaData);
		if (err != 0) {
			std::cout << "[_socket/WSAStartup] Faild to WSAStartup with version 2.2" << std::endl;
			WSACleanup();
			// Do not return, since ::socket has to set an errno
		}
		else {
			std::atexit([] {
				WSACleanup();
			});
		}
	}

    return ::socket(address_family, socket_type, socket_protocol);
}

ssize_t SystemIO::_send(SocketDescriptor socket_fd, const void* buffer, size_t length) const {
    return ::send(socket_fd, (char*)buffer, length, 0);
}

ssize_t SystemIO::_recv(SocketDescriptor socket_fd, void* buffer, size_t length) const {
    return ::recv(socket_fd, (char*)buffer, length, 0);
}

int SystemIO::_mkdir(const char* dirname) const {
    return ::_mkdir(dirname);
}

int SystemIO::_listen(SocketDescriptor fd, int backlog) const {
    return ::listen(fd, backlog);
}



}

