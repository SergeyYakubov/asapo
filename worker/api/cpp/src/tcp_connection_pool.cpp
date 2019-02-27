#include "tcp_connection_pool.h"

#include <algorithm>

#include "io/io_factory.h"

namespace asapo {

TcpConnectionPool::TcpConnectionPool() : io__{GenerateDefaultIO()} {

}

SocketDescriptor TcpConnectionPool::Connect(const std::string& source, Error* err) {
    auto sd = io__->CreateAndConnectIPTCPSocket(source, err);
    if (*err != nullptr) {
        return kDisconnectedSocketDescriptor;
    }
    return sd;
}

SocketDescriptor TcpConnectionPool::GetFreeConnection(const std::string& source, bool* reused, Error* err) {
    std::lock_guard<std::mutex> lock{mutex_};

    for (auto& connection : connections_) {
        if (source == connection.uri && !connection.in_use) {
            connection.in_use = true;
            *err = nullptr;
            *reused = true;
            return connection.sd;
        }
    }

    auto sd = Connect(source, err);
    if (*err == nullptr) {
        *reused = false;
        TcpConnectionInfo connection{source, sd, true};
        connections_.emplace_back(std::move(connection));
    }
    return sd;
}

SocketDescriptor TcpConnectionPool::Reconnect(SocketDescriptor sd, Error* err) {
    std::lock_guard<std::mutex> lock{mutex_};

    for (size_t i = 0; i < connections_.size(); i++) {
        if (connections_[i].sd == sd) {
            auto new_sd = Connect(connections_[i].uri, err);
            if (err == nullptr) {
                connections_[i].sd = new_sd;
                connections_[i].in_use = true;
            } else {
                connections_.erase(connections_.begin() + i);
            }
            return new_sd;
        }
    }

    *err = Error{new SimpleError("cannot find connection in pool")};
    return kDisconnectedSocketDescriptor;
}

void TcpConnectionPool::ReleaseConnection(SocketDescriptor sd) {
    std::lock_guard<std::mutex> lock{mutex_};

    for (auto& connection : connections_) {
        if (sd == connection.sd) {
            connection.in_use = false;
        }
    }
}

}