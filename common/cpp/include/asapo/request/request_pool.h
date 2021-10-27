#ifndef ASAPO_REQUEST_POOL_H
#define ASAPO_REQUEST_POOL_H

#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>

#include "asapo/logger/logger.h"
#include "request_handler_factory.h"
#include "request.h"
#include "asapo/preprocessor/definitions.h"


namespace asapo {

struct RequestPoolLimits {
    uint64_t max_requests;
    uint64_t max_memory_mb;
};

class RequestPool {
    struct ThreadInformation {
        std::unique_lock<std::mutex> lock;
    };
  public:
    explicit RequestPool(uint8_t n_threads, RequestHandlerFactory* request_handler_factory, const AbstractLogger* log);
    VIRTUAL Error AddRequest(GenericRequestPtr request, bool top_priority = false);
    VIRTUAL void SetLimits(RequestPoolLimits limits);
    VIRTUAL Error AddRequests(GenericRequests requests);
    VIRTUAL ~RequestPool();
    VIRTUAL uint64_t NRequestsInPool();
    VIRTUAL uint64_t UsedMemoryInPool();
    VIRTUAL Error WaitRequestsFinished(uint64_t timeout_ms);
    void StopThreads();
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
    Error CanAddRequest(const GenericRequestPtr& request, bool top_priority);
    Error CanAddRequests(const GenericRequests& requests);
    uint64_t NRequestsInPoolWithoutLock();
    uint64_t shared_counter_{0};
    uint64_t requests_in_progress_{0};
    uint64_t memory_used_{0};
    RequestPoolLimits limits_{0, 0};

};

}

#endif //ASAPO_REQUEST_POOL_H
