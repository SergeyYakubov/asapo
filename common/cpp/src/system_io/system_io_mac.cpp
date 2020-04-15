

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

#include "system_io.h"

using std::string;
using std::vector;
using std::chrono::system_clock;

namespace asapo {
ListSocketDescriptors SystemIO::WaitSocketsActivity(SocketDescriptor master_socket,
        ListSocketDescriptors* sockets_to_listen,
        std::vector<std::string>* new_connections,
        Error* err) const {
    fd_set readfds;
    ListSocketDescriptors active_sockets;
    bool client_activity = false;
    *err = nullptr;
    while (!client_activity) {

        FD_ZERO(&readfds);
        SocketDescriptor max_sd = master_socket;
        FD_SET(master_socket, &readfds);
        for (auto sd : *sockets_to_listen) {
            FD_SET(sd, &readfds);
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = kWaitTimeoutMs;

        auto activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);
        if (activity == 0) { // timeout
            *err = IOErrorTemplates::kTimeout.Generate();
            return {};
        }
        if ((activity < 0) && (errno != EINTR)) {
            *err = GetLastError();
            return {};
        }

        for (auto sd : *sockets_to_listen) {
            if (FD_ISSET(sd, &readfds)) {
                active_sockets.push_back(sd);
                client_activity = true;
            }
        }

        if (FD_ISSET(master_socket, &readfds)) {
            auto client_info_tuple = InetAcceptConnection(master_socket, err);
            if (*err) {
                return {};
            }
            std::string client_address;
            SocketDescriptor client_fd;
            std::tie(client_address, client_fd) = *client_info_tuple;
            new_connections->emplace_back(std::move(client_address));
            sockets_to_listen->push_back(client_fd);
        }
    }
    return active_sockets;
}

void SystemIO::SetThreadName(std::thread* threadHandle, const std::string& name) const {
    // does not work on macos (could only set name for current thread, which is not what we want)
}

void asapo::SystemIO::CloseSocket(SocketDescriptor fd, Error* err) const {
    if (err) {
        *err = nullptr;
    }
    if (!_close_socket(fd) && err) {
        *err = GetLastError();
    }
}


SystemIO::~SystemIO() {
    // do nothing;
}

}
