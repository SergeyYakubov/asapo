#ifndef ASAPO_RECEIVER_H
#define ASAPO_RECEIVER_H

#include <string>
#include <thread>
#include <list>


#include "connection.h"
#include "receiver_logger.h"
#include "data_cache.h"
#include "monitoring/receiver_monitoring_client.h"

namespace asapo {

class Receiver {
  private:
    FileDescriptor listener_fd_ = -1;
    Error PrepareListener(std::string listener_address);
    void StartNewConnectionInSeparateThread(int connection_socket_fd, const std::string& address);
    void ProcessConnections(Error* err);
    std::vector<std::unique_ptr<std::thread>> threads_;
    SharedCache cache_;
    SharedReceiverMonitoringClient monitoring_;
  public:
    static const int kMaxUnacceptedConnectionsBacklog;//TODO: Read from config
    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;
    Receiver(SharedCache cache, asapo::SharedReceiverMonitoringClient monitoring);

    void Listen(std::string listener_address, Error* err, bool exit_after_first_connection = false);
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
};

}

#endif //ASAPO_RECEIVER_H
