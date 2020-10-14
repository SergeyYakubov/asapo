

#include <cstring>

#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <zconf.h>
#include <netdb.h>
#include <sys/epoll.h>

#include "system_io.h"

using std::string;
using std::vector;
using std::chrono::high_resolution_clock;

namespace asapo {

Error SystemIO::AddToEpool(SocketDescriptor sd) const {
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = sd;
    if((epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, sd, &event) == -1) && (errno != EEXIST)) {
        auto err =  GetLastError();
        err->Append("add to epoll");
        close(epoll_fd_);
        return err;
    }
    return nullptr;
}

Error SystemIO::CreateEpoolIfNeeded(SocketDescriptor master_socket) const {
    if (epoll_fd_ != kDisconnectedSocketDescriptor) {
        return nullptr;
    }

    epoll_fd_ = epoll_create1(0);
    if(epoll_fd_ == kDisconnectedSocketDescriptor) {
        auto err = GetLastError();
        err->Append("Create epoll");
        return err;
    }
    return AddToEpool(master_socket);
}


Error SystemIO::ProcessNewConnection(SocketDescriptor master_socket, std::vector<std::string>* new_connections,
                                     ListSocketDescriptors* sockets_to_listen) const {
    Error err;
    auto client_info_tuple = InetAcceptConnection(master_socket, &err);
    if (err) {
        return err;
    }
    std::string client_address;
    SocketDescriptor client_fd;
    std::tie(client_address, client_fd) = *client_info_tuple;
    new_connections->emplace_back(std::move(client_address));
    sockets_to_listen->push_back(client_fd);
    return AddToEpool(client_fd);
}

ListSocketDescriptors SystemIO::WaitSocketsActivity(SocketDescriptor master_socket,
        ListSocketDescriptors* sockets_to_listen,
        std::vector<std::string>* new_connections,
        Error* err) const {

    CreateEpoolIfNeeded(master_socket);


    ListSocketDescriptors active_sockets;
    bool client_activity = false;
    while (!client_activity) {
        struct epoll_event events[kMaxEpollEvents];
        auto event_count = epoll_wait(epoll_fd_, events, kMaxEpollEvents, kWaitTimeoutMs);
        if (event_count == 0) { // timeout
            *err = IOErrorTemplates::kTimeout.Generate();
            return {};
        }
        if (event_count < 0) {
            *err = GetLastError();
            (*err)->Append("epoll wait");
            return {};
        }

        for(int i = 0; i < event_count; i++) {
            auto sd = events[i].data.fd;
            if (sd != master_socket) {
                active_sockets.push_back(sd);
                client_activity = true;
            } else {
                *err = ProcessNewConnection(master_socket, new_connections, sockets_to_listen);
                if (*err) {
                    return {};
                }
            }
        }
    }
    return active_sockets;
}

SystemIO::~SystemIO() {
    if (epoll_fd_ != kDisconnectedSocketDescriptor) {
        close(epoll_fd_);
    }
}

void SystemIO::SetThreadName(std::thread* threadHandle, const std::string& name) const {
    // If the length of name is greater than 15 characters, the excess characters are ignored.
    pthread_setname_np(threadHandle->native_handle(), name.c_str());
}

void asapo::SystemIO::CloseSocket(SocketDescriptor fd, Error* err) const {
    if (err) {
        *err = nullptr;
    }
    if (!_close_socket(fd) && err) {
        *err = GetLastError();
    }
    if (epoll_fd_ != kDisconnectedSocketDescriptor) {
        struct epoll_event event;
        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &event);
        errno = 0; // ignore possible errors
    }
}


}
