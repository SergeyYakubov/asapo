#ifndef ASAPO_REQUEST_POOL_H
#define ASAPO_REQUEST_POOL_H

#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>
#include <chrono>


#include "logger/logger.h"
#include "request.h"
#include "receiver_discovery_service.h"

using std::chrono::high_resolution_clock;

namespace asapo {

class RequestPool {
    struct ThreadInformation {
        uint64_t id;
        std::unique_lock<std::mutex> lock;
        SocketDescriptor thread_sd;
        ReceiversList thread_receivers;
        high_resolution_clock::time_point last_rebalance;
    };
  public:
    explicit RequestPool(uint8_t n_threads, uint64_t max_pool_volume, ReceiverDiscoveryService* discovery_service);
    Error AddRequest(std::unique_ptr<Request> request);
    ~RequestPool();
    AbstractLogger* log__;
    uint64_t NRequestsInQueue();
  private:
    ReceiverDiscoveryService* discovery_service__;
    std::vector<std::thread> threads_;
    void ThreadHandler(uint64_t id);
    bool quit_{false};
    std::condition_variable condition_;
    std::mutex mutex_;
    std::deque<std::unique_ptr<Request>> request_queue_;
    uint64_t max_pool_volume_;
    uint64_t current_pool_volume_{0};
    uint64_t ncurrent_connections_{0};
    bool RequestWouldFit(const std::unique_ptr<Request>& request);
    bool IsConnected(SocketDescriptor thread_sd);
    bool CanCreateNewConnections();
    bool CanProcessRequest(SocketDescriptor thread_sd);
    void ProcessRequest(ThreadInformation* thread_info);
    void UpdateIfNewConnection(ThreadInformation* thread_info);
    bool CheckForRebalance(ThreadInformation* thread_info);
    std::unique_ptr<Request> GetRequestFromQueue();
    Error SendDataViaRequest(const std::unique_ptr<Request>& request, ThreadInformation* thread_info);
    void PutRequestBackToQueue(std::unique_ptr<Request>request);

};

}

#endif //ASAPO_REQUEST_POOL_H
