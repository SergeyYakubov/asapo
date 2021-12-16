#ifndef ASAPO_RECEIVERS_STATUS_H
#define ASAPO_RECEIVERS_STATUS_H

#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>


#include "asapo/http_client/http_client.h"
#include "asapo/logger/logger.h"
#include "asapo/preprocessor/definitions.h"

namespace  asapo {

using ReceiversList = std::vector<std::string>;

class ReceiverDiscoveryService {
  public:
    explicit ReceiverDiscoveryService(std::string endpoint, uint64_t update_frequency_ms);
    ASAPO_VIRTUAL void StartCollectingData();
    ASAPO_VIRTUAL ~ReceiverDiscoveryService();
    ASAPO_VIRTUAL uint64_t MaxConnections();
    ASAPO_VIRTUAL ReceiversList RotatedUriList(uint64_t nthread);
    ASAPO_VIRTUAL uint64_t UpdateFrequency();
  public:
    std::unique_ptr<HttpClient> httpclient__;
    const AbstractLogger* log__;
  private:
    static const std::string kServiceEndpointSuffix;
    void ThreadHandler();
    Error UpdateFromEndpoint(ReceiversList* list, uint64_t* max_connections);
    Error ParseResponse(const std::string& responce, ReceiversList* list, uint64_t* max_connections);
    void LogUriList(const ReceiversList& uris);
    std::string endpoint_;
    std::thread thread_;
    std::condition_variable condition_;
    std::mutex mutex_;
    uint64_t max_connections_{1};
    ReceiversList uri_list_;
    bool quit_{false};
    uint64_t update_frequency_ms_;
};

}

#endif //ASAPO_RECEIVERS_STATUS_H
