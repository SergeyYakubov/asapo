#ifndef ASAPO_REQUEST_H
#define ASAPO_REQUEST_H

#include <chrono>

#include "io/io.h"
#include "common/error.h"
#include "receiver_discovery_service.h"
#include "common/networking.h"

#include "producer/common.h"
#include "request/request_handler.h"
#include "producer_request.h"

using std::chrono::system_clock;

namespace asapo {

class RequestHandlerTcp: public RequestHandler {
  public:
    explicit RequestHandlerTcp(ReceiverDiscoveryService* discovery_service, uint64_t thread_id, uint64_t* shared_counter);
    Error ProcessRequestUnlocked(GenericRequest* request) override;
    bool ReadyProcessRequest() override;
    void PrepareProcessingRequestLocked()  override;
    void TearDownProcessingRequestLocked(const Error& error_from_process)  override;

    virtual ~RequestHandlerTcp() = default;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
    ReceiverDiscoveryService* discovery_service__;
  private:
    Error Authorize(const std::string& beamtime_id);
    Error ConnectToReceiver(const std::string& beamtime_id, const std::string& receiver_address);
    Error SendDataToOneOfTheReceivers(ProducerRequest* request);
    Error SendRequestContent(const ProducerRequest* request);
    Error ReceiveResponse();
    Error TrySendToReceiver(const ProducerRequest* request);
    SocketDescriptor sd_{kDisconnectedSocketDescriptor};
    void UpdateIfNewConnection();
    bool UpdateReceiversList();
    bool TimeToUpdateReceiverList();
    bool NeedRebalance();
    void CloseConnectionToPeformRebalance();
    bool Disconnected();
    void Disconnect();
    bool ServerError(const Error& err);
    ReceiversList receivers_list_;
    system_clock::time_point last_receivers_uri_update_;
    bool Connected();
    bool CanCreateNewConnections();
    uint64_t thread_id_;
    uint64_t* ncurrent_connections_;
    std::string connected_receiver_uri_;
};
}

#endif //ASAPO_REQUEST_H
