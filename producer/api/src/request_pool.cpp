#include <chrono>

#include "request_pool.h"
#include "producer_logger.h"



using std::chrono::high_resolution_clock;


namespace asapo {

RequestPool:: RequestPool(uint8_t n_threads, uint64_t max_pool_volume,
                          ReceiverDiscoveryService* discovery_service): log__{GetDefaultProducerLogger()},
    discovery_service__{discovery_service},
    threads_{n_threads}, max_pool_volume_{max_pool_volume} {

    discovery_service->StartCollectingData();
    for(size_t i = 0; i < threads_.size(); i++) {
        log__->Debug("starting thread " + std::to_string(i));
        threads_[i] = std::thread(
                          [this, i] {ThreadHandler(i);});
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


void RequestPool::ThreadHandler(uint64_t id) {
    std::unique_lock<std::mutex> lock(mutex_);
    SocketDescriptor thread_sd = kDisconnectedSocketDescriptor;
    ReceiversList thread_receivers;
    high_resolution_clock::time_point last_rebalance;
    do {
        condition_.wait(lock, [this, thread_sd] {
            return ((request_queue_.size() && (thread_sd != kDisconnectedSocketDescriptor  ||
            ncurrent_connections_ < discovery_service__->MaxConnections()) ) || quit_);
        });
        //after wait, we own the lock
        if (!quit_) {
            if (thread_sd == kDisconnectedSocketDescriptor) {
                thread_receivers = discovery_service__->RotatedUriList(id);
                last_rebalance = high_resolution_clock::now();
                ncurrent_connections_++;
            }

            auto request = std::move(request_queue_.front());
            request_queue_.pop_front();

            //unlock now that we're done messing with the queue
            lock.unlock();

            uint64_t elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>( high_resolution_clock::now() - last_rebalance).count();
            bool rebalance = false;
            if (elapsed_ms > discovery_service__->UpdateFrequency()) {
                auto thread_receivers_new = discovery_service__->RotatedUriList(id);
                last_rebalance = high_resolution_clock::now();
                if (thread_receivers_new != thread_receivers) {
                    thread_receivers = thread_receivers_new;
                    rebalance = true;
                }
            }

            auto err = request->Send(&thread_sd, thread_receivers,rebalance);
            // we should lock again for the next wait since we did unlock
            lock.lock();
            if (err) {
                // could not send from this thread - place request back in the queue
                request_queue_.emplace_front(std::move(request));
                ncurrent_connections_--;
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
