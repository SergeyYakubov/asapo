#ifndef ASAPO_RECEIVERS_STATUS_H
#define ASAPO_RECEIVERS_STATUS_H

#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>


#include "http_client/http_client.h"
#include "logger/logger.h"

#ifdef UNIT_TESTS
#define VIRTUAL virtual
#endif

namespace  asapo {

using ReceiversList = std::vector<std::string>;

class ReceiverDiscoveryService {
  public:
    explicit ReceiverDiscoveryService(std::string endpoint, uint64_t update_frequency_ms);
    VIRTUAL void StartCollectingData();
    ~ReceiverDiscoveryService();
    VIRTUAL uint64_t MaxConnections();
    VIRTUAL ReceiversList RotatedUriList(uint64_t nthread);
    uint64_t UpdateFrequency();
  public:
    std::unique_ptr<HttpClient> httpclient__;
    AbstractLogger* log__;
  private:
    void ThreadHandler();
    Error UpdateFromEndpoint(ReceiversList* list, uint64_t* max_connections);
    Error ParseResponse(const std::string& responce, ReceiversList* list, uint64_t* max_connections);
    std::string endpoint_;
    std::thread thread_;
    std::condition_variable condition_;
    std::mutex mutex_;
    uint64_t max_connections_{0};
    ReceiversList uri_list_;
    bool quit_{false};
    uint64_t update_frequency_ms_;
};

}

#endif //ASAPO_RECEIVERS_STATUS_H
