#include "request/request_pool.h"

namespace asapo {

RequestPool:: RequestPool(uint8_t n_threads,
                          RequestHandlerFactory* request_handler_factory, const AbstractLogger* log): log__{log},
    request_handler_factory__{request_handler_factory},
    threads_{n_threads} {
    for(size_t i = 0; i < n_threads; i++) {
        log__->Debug("starting thread " + std::to_string(i));
        threads_[i] = std::thread(
                          [this, i] {ThreadHandler(i);});
    }

}

Error RequestPool::AddRequest(GenericRequestPtr request) {
    std::unique_lock<std::mutex> lock(mutex_);
    request_queue_.emplace_back(std::move(request));
    lock.unlock();
//todo: maybe notify_one is better here
    condition_.notify_all();

    return nullptr;
}

bool RequestPool::CanProcessRequest(const std::unique_ptr<RequestHandler>& request_handler) {
    return request_queue_.size() && request_handler->ReadyProcessRequest();
}

GenericRequestPtr RequestPool::GetRequestFromQueue() {
    auto request = std::move(request_queue_.front());
    request_queue_.pop_front();
    return request;
}

void RequestPool::PutRequestBackToQueue(GenericRequestPtr request) {
    request_queue_.emplace_front(std::move(request));
}

void RequestPool::ProcessRequest(const std::unique_ptr<RequestHandler>& request_handler,
                                 ThreadInformation* thread_info) {
    request_handler->PrepareProcessingRequestLocked();
    auto request = GetRequestFromQueue();
    thread_info->lock.unlock();
    auto err = request_handler->ProcessRequestUnlocked(request.get());
    thread_info->lock.lock();
    request_handler->TearDownProcessingRequestLocked(err);
    if (err) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        PutRequestBackToQueue(std::move(request));
        condition_.notify_all();
    }
}

void RequestPool::ThreadHandler(uint64_t id) {
    ThreadInformation thread_info;
    thread_info.lock =  std::unique_lock<std::mutex>(mutex_);
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
    mutex_.lock();
    quit_ = true;
    mutex_.unlock();
    condition_.notify_all();

    for(size_t i = 0; i < threads_.size(); i++) {
        if(threads_[i].joinable()) {
            log__->Debug("finishing thread " + std::to_string(i));
            threads_[i].join();
        }
    }
}
uint64_t RequestPool::NRequestsInQueue() {
    std::lock_guard<std::mutex> lock{mutex_};
    return request_queue_.size();
}
Error RequestPool::AddRequests(GenericRequests requests) {
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto& elem : requests) {
        request_queue_.emplace_front(std::move(elem));
    }
    lock.unlock();
//todo: maybe notify_one is better here
    condition_.notify_all();
    return nullptr;

}

}
