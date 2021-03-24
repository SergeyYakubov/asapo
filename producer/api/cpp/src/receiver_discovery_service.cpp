#include "receiver_discovery_service.h"

#include <iostream>
#include <algorithm>
#include <numeric>

#include "producer_logger.h"
#include "asapo/json_parser/json_parser.h"
#include "asapo/common/version.h"

namespace  asapo {

const std::string ReceiverDiscoveryService::kServiceEndpointSuffix = "/asapo-discovery/"+kProducerProtocol.GetDiscoveryVersion()
    +"/asapo-receiver?protocol="+kConsumerProtocol.GetVersion();

ReceiverDiscoveryService::ReceiverDiscoveryService(std::string endpoint, uint64_t update_frequency_ms): httpclient__{DefaultHttpClient()},
    log__{GetDefaultProducerLogger()},
    endpoint_{std::move(endpoint) + kServiceEndpointSuffix}, update_frequency_ms_{update_frequency_ms} {

}

void ReceiverDiscoveryService::StartCollectingData() {
    if (thread_ .joinable()) return;
    log__->Debug("starting receiver discovery service");
    thread_ = std::thread(
                  std::bind(&ReceiverDiscoveryService::ThreadHandler, this));
}


Error ReceiverDiscoveryService::ParseResponse(const std::string& responce, ReceiversList* list,
                                              uint64_t* max_connections) {
    auto parser = JsonStringParser(responce);
    Error err;
    (err = parser.GetArrayString("Uris", list)) ||
    (err = parser.GetUInt64("MaxConnections", max_connections));
    return err;
}

Error ReceiverDiscoveryService::UpdateFromEndpoint(ReceiversList* list, uint64_t* max_connections) {
    Error err;
    HttpCode code;

    auto responce = httpclient__->Get(endpoint_, &code, &err);
    if (err != nullptr) {
        return err;
    }
    if (code != HttpCode::OK) {
        return TextError(responce);
    }
    return ParseResponse(responce, list, max_connections);

}

void ReceiverDiscoveryService::LogUriList(const ReceiversList& uris) {
    std::string s;
    s = std::accumulate(std::begin(uris), std::end(uris), s);
    log__->Debug("got receivers from " + endpoint_ + ":" + s);
}


void ReceiverDiscoveryService::ThreadHandler() {
    std::unique_lock<std::mutex> lock(mutex_);
    do {
        lock.unlock();
        ReceiversList uris;
        uint64_t max_connections;
        auto err = UpdateFromEndpoint(&uris, &max_connections);
        if (err != nullptr) {
            log__->Error("getting receivers from " + endpoint_ + " - " + err->Explain());
            lock.lock();
            continue;
        }
        LogUriList(uris);
        lock.lock();
        max_connections_ = max_connections;
        uri_list_ = uris;
    } while (!condition_.wait_for(lock, std::chrono::milliseconds(update_frequency_ms_), [this] {return (quit_);})) ;
}

ReceiverDiscoveryService::~ReceiverDiscoveryService() {
    mutex_.lock();
    quit_ = true;
    mutex_.unlock();
    condition_.notify_one();

    if(thread_.joinable()) {
        log__->Debug("finishing discovery service");
        thread_.join();
    }
}

uint64_t ReceiverDiscoveryService::MaxConnections() {
    std::lock_guard<std::mutex> lock{mutex_};
    return max_connections_;
}

ReceiversList ReceiverDiscoveryService::RotatedUriList(uint64_t nthread) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto size = uri_list_.size();
    if (size == 0) {
        return {};
    }
    ReceiversList list{uri_list_};
    lock.unlock();
    auto shift = (int) nthread % size;
    std::rotate(list.begin(), list.begin() + shift, list.end());
    return list;
}

uint64_t ReceiverDiscoveryService::UpdateFrequency() {
    return update_frequency_ms_;
}

}
