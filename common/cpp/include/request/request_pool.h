#ifndef ASAPO_REQUEST_POOL_H
#define ASAPO_REQUEST_POOL_H

#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>

#include "logger/logger.h"
#include "request_handler_factory.h"
#include "request.h"
#include "preprocessor/definitions.h"


namespace asapo {

class RequestPool {
    struct ThreadInformation {
        std::unique_lock<std::mutex> lock;
    };
  public:
    explicit RequestPool(uint8_t n_threads, RequestHandlerFactory* request_handler_factory, const AbstractLogger* log);
    VIRTUAL Error AddRequest(GenericRequestPtr request);
    VIRTUAL Error AddRequests(GenericRequests requests);

    ~RequestPool();
    uint64_t NRequestsInQueue();
  private:
    const AbstractLogger* log__;
    RequestHandlerFactory* request_handler_factory__;
    std::vector<std::thread> threads_;
    void ThreadHandler(uint64_t id);
    bool quit_{false};
    std::condition_variable condition_;
    std::mutex mutex_;
    std::deque<GenericRequestPtr> request_queue_;
    bool CanProcessRequest(const std::unique_ptr<RequestHandler>& request_handler);
    void ProcessRequest(const std::unique_ptr<RequestHandler>& request_handler, ThreadInformation* thread_info);
    GenericRequestPtr GetRequestFromQueue();
    void PutRequestBackToQueue(GenericRequestPtr request);
    uint64_t shared_counter_{0};

};

}

#endif //ASAPO_REQUEST_POOL_H