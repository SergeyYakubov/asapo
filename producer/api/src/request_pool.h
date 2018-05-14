#ifndef ASAPO_REQUEST_POOL_H
#define ASAPO_REQUEST_POOL_H

#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <queue>

#include "logger/logger.h"
#include "request.h"


namespace asapo {

class RequestPool {
  public:
    explicit RequestPool(uint8_t n_threads, uint64_t max_pool_volume);
    Error AddRequest(std::unique_ptr<Request> request);
    ~RequestPool();
    AbstractLogger* log__;
  private:
    std::vector<std::thread> threads_;
    void ThreadHandler();
    bool quit_{false};
    std::condition_variable condition_;
    std::mutex mutex_;
    std::deque<std::unique_ptr<Request>> request_queue_;
    uint64_t max_pool_volume_;
    uint64_t current_pool_volume_{0};
    bool RequestWouldFit(const std::unique_ptr<Request>& request);
};

}

#endif //ASAPO_REQUEST_POOL_H
