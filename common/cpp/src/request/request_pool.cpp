#include "asapo/request/request_pool.h"
#include "asapo/request/request_pool_error.h"
namespace asapo {

RequestPool::RequestPool(uint8_t n_threads,
                         RequestHandlerFactory* request_handler_factory, const AbstractLogger* log) : log__{log},
                                                                                                      request_handler_factory__{
                                                                                                          request_handler_factory},
                                                                                                      threads_{
                                                                                                          n_threads} {
    for (size_t i = 0; i < n_threads; i++) {
        log__->Debug("starting thread " + std::to_string(i));
        threads_[i] = std::thread(
            [this, i] { ThreadHandler(i); });
    }

}

Error RequestPool::CanAddRequests(const GenericRequests &requests) {
    if (NRequestsInPoolWithoutLock() + requests.size() > limits_.max_requests && limits_.max_requests > 0) {
        return IOErrorTemplates::kNoSpaceLeft.Generate(
            "reached maximum number of " + std::to_string(limits_.max_requests) + " requests");
    }

    if (limits_.max_memory_mb == 0) {
        return nullptr;
    }

    uint64_t total_size = 0;
    for (auto &request : requests) {
        if (request->ContainsData()) {
            total_size += request->header.data_size;
        }
    }

    if (memory_used_ + total_size > limits_.max_memory_mb * 1000000) {
        return IOErrorTemplates::kNoSpaceLeft.Generate(
            "reached maximum memory capacity of " + std::to_string(limits_.max_memory_mb) + " MB");
    }

    return nullptr;
}

Error RequestPool::CanAddRequest(const GenericRequestPtr &request, bool top_priority) {
    if (top_priority) {
        return nullptr;
    }
    if (limits_.max_requests > 0 && NRequestsInPoolWithoutLock() >= limits_.max_requests) {
        return IOErrorTemplates::kNoSpaceLeft.Generate(
            "reached maximum number of " + std::to_string(limits_.max_requests) + " requests");
    }

    if (!request->ContainsData()) {
        return nullptr;
    }

    if (limits_.max_memory_mb > 0 && memory_used_ + request->header.data_size > limits_.max_memory_mb * 1000000) {
        return IOErrorTemplates::kNoSpaceLeft.Generate(
            "reached maximum memory capacity of " + std::to_string(limits_.max_memory_mb) + " MB");
    }

    return nullptr;
}

Error RequestPool::AddRequest(GenericRequestPtr request, bool top_priority) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (auto err = CanAddRequest(request, top_priority)) {
        OriginalRequest* original_request = new OriginalRequest{};
        original_request->request = std::move(request);
        err->SetCustomData(std::unique_ptr<CustomErrorData>(original_request));
        return err;
    }

    if (request->ContainsData()) {
        memory_used_ += request->header.data_size;
    }

    if (top_priority) {
        request_queue_.emplace_front(std::move(request));
    } else {
        request_queue_.emplace_back(std::move(request));
    }
    lock.unlock();
//todo: maybe notify_one is better here
    condition_.notify_all();

    return nullptr;
}

bool RequestPool::CanProcessRequest(const std::unique_ptr<RequestHandler> &request_handler) {
    return request_queue_.size() && request_handler->ReadyProcessRequest();
}

GenericRequestPtr RequestPool::GetRequestFromQueue() {
    auto request = std::move(request_queue_.front());
    request_queue_.pop_front();
    return request;
}

void RequestPool::PutRequestBackToQueue(GenericRequestPtr request) {
// do not need to lock since we already own it
    request->IncreaseRetryCounter();
    request_queue_.emplace_front(std::move(request));
}

void RequestPool::ProcessRequest(const std::unique_ptr<RequestHandler> &request_handler,
                                 ThreadInformation* thread_info) {
    auto request = GetRequestFromQueue();
    if (request->TimedOut()) {
        request_handler->ProcessRequestTimeout(request.get());
        return;
    }
    request_handler->PrepareProcessingRequestLocked();
    requests_in_progress_++;
    thread_info->lock.unlock();
    bool retry;
    auto success = request_handler->ProcessRequestUnlocked(request.get(), &retry);
    thread_info->lock.lock();
    requests_in_progress_--;
    request_handler->TearDownProcessingRequestLocked(success);
    if (retry) {
        PutRequestBackToQueue(std::move(request));
        thread_info->lock.unlock();
        condition_.notify_all();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        thread_info->lock.lock();
    } else {
        if (request->ContainsData()) {
            memory_used_ -= request->header.data_size;
        }
    }
}

void RequestPool::ThreadHandler(uint64_t id) {
    ThreadInformation thread_info;
    thread_info.lock = std::unique_lock<std::mutex>(mutex_);
    auto request_handler = request_handler_factory__->NewRequestHandler(id, &shared_counter_);
    do {
        auto do_work = condition_.wait_for(thread_info.lock, std::chrono::milliseconds(100), [this, &request_handler] {
          return (CanProcessRequest(request_handler) || quit_);
        });
        //after wait, we own the lock
        if (!quit_ && do_work) {
            ProcessRequest(request_handler, &thread_info);
        };
    } while (!quit_);
}

RequestPool::~RequestPool() {
    StopThreads();
}

uint64_t RequestPool::NRequestsInPoolWithoutLock() {
    return request_queue_.size() + requests_in_progress_;
}

uint64_t RequestPool::NRequestsInPool() {
    std::lock_guard<std::mutex> lock{mutex_};
    return NRequestsInPoolWithoutLock();
}
Error RequestPool::AddRequests(GenericRequests requests) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (auto err = CanAddRequests(requests)) {
        return err;
    }

    uint64_t total_size = 0;
    for (auto &elem : requests) {
        if (elem->ContainsData()) {
            total_size += elem->header.data_size;
        }
        request_queue_.emplace_front(std::move(elem));
    }
    memory_used_ += total_size;
    lock.unlock();
//todo: maybe notify_one is better here
    condition_.notify_all();
    return nullptr;

}

Error RequestPool::WaitRequestsFinished(uint64_t timeout_ms) {
    uint64_t elapsed_ms = 0;
    while (true) {
        auto n_requests = NRequestsInPool();
        if (n_requests == 0) {
            break;
        }
        if (elapsed_ms >= timeout_ms) {
            return IOErrorTemplates::kTimeout.Generate();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        elapsed_ms += 100;
    }
    return nullptr;
}

void RequestPool::StopThreads() {
    mutex_.lock();
    quit_ = true;
    mutex_.unlock();
    condition_.notify_all();
    for (size_t i = 0; i < threads_.size(); i++) {
        if (threads_[i].joinable()) {
            log__->Debug("finishing thread " + std::to_string(i));
            threads_[i].join();
        }
    }
}

uint64_t RequestPool::UsedMemoryInPool() {
    std::lock_guard<std::mutex> lock{mutex_};
    return memory_used_;
}

void RequestPool::SetLimits(RequestPoolLimits limits) {
    std::lock_guard<std::mutex> lock{mutex_};
    limits_ = limits;
}

}
