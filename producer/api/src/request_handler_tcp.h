#ifndef ASAPO_REQUEST_H
#define ASAPO_REQUEST_H

#include <chrono>

#include "io/io.h"
#include "common/error.h"
#include "receiver_discovery_service.h"
#include "common/networking.h"

#include "producer/producer.h"
#include "request_handler.h"


using std::chrono::high_resolution_clock;

namespace asapo {

class RequestHandlerTcp: public RequestHandler {
  public:
    explicit RequestHandlerTcp(ReceiverDiscoveryService* discovery_service,uint64_t thread_id);
    Error ProcessRequestUnlocked(const Request* request) override;
    bool ReadyProcessRequest() override;
    void PrepareProcessingRequestLocked()  override;
    void TearDownProcessingRequestLocked(const Error &error_from_process)  override;

    virtual ~RequestHandlerTcp() = default;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
    ReceiverDiscoveryService* discovery_service__;
  private:
    Error ConnectToReceiver(const std::string& receiver_address);
    Error SendHeaderAndData(const Request*,const std::string& receiver_address);
    Error ReceiveResponse(const std::string& receiver_address);
    Error TrySendToReceiver(const Request* request,const std::string& receiver_address);
    SocketDescriptor sd_{kDisconnectedSocketDescriptor};
    void UpdateReceiversUriIfNewConnection();
    bool CheckForRebalance();
    ReceiversList receivers_list_;
    high_resolution_clock::time_point last_receivers_uri_update_;
    uint64_t ncurrent_connections_{0};
    bool IsConnected();
    bool CanCreateNewConnections();
    uint64_t thread_id_;

  };
}

#endif //ASAPO_REQUEST_H
