#include "request_pool.h"
#include "producer_logger.h"


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

bool RequestPool::IsConnected(SocketDescriptor thread_sd) {
    return thread_sd != kDisconnectedSocketDescriptor;
}

bool RequestPool::CanCreateNewConnections() {
    return ncurrent_connections_ < discovery_service__->MaxConnections();
}

bool RequestPool::CanProcessRequest(SocketDescriptor thread_sd) {
    return (request_queue_.size() && (IsConnected(thread_sd) || CanCreateNewConnections()));
}


void RequestPool::UpdateIfNewConnection(ThreadInformation* thread_info) {
    if (thread_info->thread_sd != kDisconnectedSocketDescriptor)
        return;

    thread_info->thread_receivers = discovery_service__->RotatedUriList(thread_info->id);
    thread_info->last_rebalance = high_resolution_clock::now();
    ncurrent_connections_++;
}

bool RequestPool::CheckForRebalance(ThreadInformation* thread_info) {
    auto now =  high_resolution_clock::now();
    uint64_t elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>( now -
                          thread_info->last_rebalance).count();
    bool rebalance = false;
    if (elapsed_ms > discovery_service__->UpdateFrequency()) {
        auto thread_receivers_new = discovery_service__->RotatedUriList(thread_info->id);
        thread_info->last_rebalance = now;
        if (thread_receivers_new != thread_info->thread_receivers) {
            thread_info->thread_receivers = thread_receivers_new;
            rebalance = true;
        }
    }
    return rebalance;

}

std::unique_ptr<Request> RequestPool::GetRequestFromQueue() {
    auto request = std::move(request_queue_.front());
    request_queue_.pop_front();
    return request;
}

Error RequestPool::SendDataViaRequest(const std::unique_ptr<Request>& request, ThreadInformation* thread_info) {
    //unlock now that we're ready to do async operations
    thread_info->lock.unlock();

    bool rebalance = CheckForRebalance(thread_info);
    auto err = request->Send(&thread_info->thread_sd, thread_info->thread_receivers, rebalance);

    // we should lock again for the next wait since we did unlock
    thread_info->lock.lock();
    return err;
}

void RequestPool::PutRequestBackToQueue(std::unique_ptr<Request> request) {
    request_queue_.emplace_front(std::move(request));
    ncurrent_connections_--;
}

void RequestPool::ProcessRequest(ThreadInformation* thread_info) {
    UpdateIfNewConnection((thread_info));

    auto request = GetRequestFromQueue();
    auto err = SendDataViaRequest(request, thread_info);
    if (err) {
        PutRequestBackToQueue(std::move(request));
    }
}

void RequestPool::ThreadHandler(uint64_t id) {
    ThreadInformation thread_info;
    thread_info.id = id;
    thread_info.lock =  std::unique_lock<std::mutex>(mutex_);
    thread_info.thread_sd = kDisconnectedSocketDescriptor;
    high_resolution_clock::time_point last_rebalance;
    do {
        condition_.wait(thread_info.lock, [this, &thread_info] {
            return (CanProcessRequest(thread_info.thread_sd) || quit_);
        });
        //after wait, we own the lock
        if (!quit_) {
            ProcessRequest(&thread_info);
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

}
