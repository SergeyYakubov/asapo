#ifndef ASAPO_REQUEST_H
#define ASAPO_REQUEST_H

#include <chrono>

#include "asapo/io/io.h"
#include "asapo/common/error.h"
#include "receiver_discovery_service.h"
#include "asapo/common/networking.h"

#include "asapo/producer/common.h"
#include "asapo/request/request_handler.h"
#include "producer_request.h"

using std::chrono::system_clock;

namespace asapo {

class RequestHandlerTcp: public RequestHandler {
  public:
    explicit RequestHandlerTcp(ReceiverDiscoveryService* discovery_service, uint64_t thread_id, uint64_t* shared_counter);
    bool ProcessRequestUnlocked(GenericRequest* request, bool* retry) override;
    bool ReadyProcessRequest() override;
    void PrepareProcessingRequestLocked()  override;
    void TearDownProcessingRequestLocked(bool request_processed_successfully)  override;
    void ProcessRequestTimeout(GenericRequest* request)  override;

    virtual ~RequestHandlerTcp() = default;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
    ReceiverDiscoveryService* discovery_service__;
  private:
    Error Authorize(const std::string& source_credentials);
    Error ConnectToReceiver(const std::string& source_credentials, const std::string& receiver_address);
    bool SendToOneOfTheReceivers(ProducerRequest* request, bool* retry);
    Error SendRequestContent(const ProducerRequest* request);
    Error ReceiveResponse(const GenericRequestHeader& request_header, std::string* response);
    Error TrySendToReceiver(const ProducerRequest* request, std::string* response);
    SocketDescriptor sd_{kDisconnectedSocketDescriptor};
    void UpdateIfNewConnection();
    bool UpdateReceiversList();
    bool TimeToUpdateReceiverList();
    bool NeedRebalance();
    void CloseConnectionToPeformRebalance();
    bool Disconnected();
    void Disconnect();
    bool ServerError(const Error& err);
    bool Connected();
    bool CanCreateNewConnections();
    bool ProcessErrorFromReceiver(const Error& error, const ProducerRequest* request, const std::string& receiver_uri);
    ReceiversList receivers_list_;
    system_clock::time_point last_receivers_uri_update_;
    void ProcessRequestCallback(Error err, ProducerRequest* request, std::string response, bool* retry);
    uint64_t thread_id_;
    uint64_t* ncurrent_connections_;
    std::string connected_receiver_uri_;
};
}

#endif //ASAPO_REQUEST_H
