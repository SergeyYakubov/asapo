#ifndef ASAPO_RECEIVER_DATA_SERVER_REQUEST_HANDLER_H
#define ASAPO_RECEIVER_DATA_SERVER_REQUEST_HANDLER_H

#include "request/request_handler.h"
#include "net_server.h"
#include "../data_cache.h"
#include "receiver_data_server_request.h"
#include "receiver_data_server_logger.h"
#include "../statistics.h"

namespace asapo {

class ReceiverDataServerRequestHandler: public RequestHandler {
  public:
    explicit ReceiverDataServerRequestHandler(const NetServer* server, DataCache* data_cache, Statistics* statistics);
    bool ProcessRequestUnlocked(GenericRequest* request, bool* retry) override;
    bool ReadyProcessRequest() override;
    void PrepareProcessingRequestLocked()  override;
    void TearDownProcessingRequestLocked(bool processing_succeeded)  override;
    void ProcessRequestTimeout(GenericRequest* request)  override;

    const AbstractLogger* log__;
    Statistics* statistics__;
  private:
    const NetServer* server_;
    DataCache* data_cache_;
    bool CheckRequest(const ReceiverDataServerRequest* request);
    Error SendResponce(const ReceiverDataServerRequest* request, NetworkErrorCode code);
    Error SendData(const ReceiverDataServerRequest* request, void* data, CacheMeta* meta);
    void* GetSlot(const ReceiverDataServerRequest* request, CacheMeta** meta);
};

}

#endif //ASAPO_RECEIVER_DATA_SERVER_REQUEST_HANDLER_H
