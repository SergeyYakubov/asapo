#ifndef ASAPO_RECEIVER_DATA_SERVER_REQUEST_HANDLER_H
#define ASAPO_RECEIVER_DATA_SERVER_REQUEST_HANDLER_H

#include "request/request_handler.h"
#include "../net_server/rds_net_server.h"
#include "../../data_cache.h"
#include "../receiver_data_server_request.h"
#include "../receiver_data_server_logger.h"
#include "../../statistics/statistics.h"

namespace asapo {

class ReceiverDataServerRequestHandler: public RequestHandler {
  public:
    explicit ReceiverDataServerRequestHandler(RdsNetServer* server, DataCache* data_cache, Statistics* statistics);
    bool ProcessRequestUnlocked(GenericRequest* request, bool* retry) override;
    bool ReadyProcessRequest() override;
    void PrepareProcessingRequestLocked()  override;
    void TearDownProcessingRequestLocked(bool processing_succeeded)  override;
    void ProcessRequestTimeout(GenericRequest* request)  override;

    const AbstractLogger* log__;
    Statistics* statistics__;
  private:
    RdsNetServer* server_;
    DataCache* data_cache_;
    bool CheckRequest(const ReceiverDataServerRequest* request);
    Error SendResponse(const ReceiverDataServerRequest* request, NetworkErrorCode code);
    Error SendResponseAndSlotData(const ReceiverDataServerRequest* request, const CacheMeta* meta);
    CacheMeta* GetSlotAndLock(const ReceiverDataServerRequest* request);

    void HandleInvalidRequest(const ReceiverDataServerRequest* receiver_request);

    void HandleValidRequest(const ReceiverDataServerRequest* receiver_request, const CacheMeta* meta);
};

}

#endif //ASAPO_RECEIVER_DATA_SERVER_REQUEST_HANDLER_H
