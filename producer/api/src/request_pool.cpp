#include "request_pool.h"
#include "producer_logger.h"

namespace asapo {

RequestPool:: RequestPool(uint8_t n_threads, uint64_t max_pool_volume): log__{GetDefaultProducerLogger()}, threads_{n_threads},
    max_pool_volume_{max_pool_volume} {
    for(size_t i = 0; i < threads_.size(); i++) {
        log__->Debug("starting thread " + std::to_string(i));
        threads_[i] = std::thread(
                          std::bind(&RequestPool::ThreadHandler, this));
    }

}

bool RequestPool::RequestWouldFit(const std::unique_ptr<Request>& request) {
    return request->GetMemoryRequitements() + current_pool_volume_ < max_pool_volume_;
}

Error RequestPool::AddRequest(std::unique_ptr<Request> request) {
    if (!RequestWouldFit(request)) {
        return ProducerErrorTemplates::kRequestPoolIsFull.Generate();
    }

    std::unique_lock<std::mutex> lock(mutex_);
    request_queue_.emplace_back(std::move(request));
    lock.unlock();
//todo: maybe notify_one is better here
    condition_.notify_all();


    return nullptr;
}


void RequestPool::ThreadHandler(void) {
    std::unique_lock<std::mutex> lock(mutex_);
    SocketDescriptor thread_sd = kDisconnectedSocketDescriptor;
    ReceiversList thread_receivers;
    do {
        condition_.wait(lock, [this] {
            return (request_queue_.size() || quit_);
        });
        //after wait, we own the lock

        if (request_queue_.size() && !quit_) {

            if (thread_sd == kDisconnectedSocketDescriptor) {
                thread_receivers = ReceiversList{"test"};
            }

            auto request = std::move(request_queue_.front());
            request_queue_.pop_front();

            //unlock now that we're done messing with the queue
            lock.unlock();

            auto err = request->Send(&thread_sd,thread_receivers);
            // we should lock again for the next wait since we did unlock
            lock.lock();
            if (err) {
                // could not send from this thread - place request back in the queue
                request_queue_.emplace_front(std::move(request));
            }
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



}
