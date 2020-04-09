#ifndef ASAPO_RECEIVER_DATA_SERVER_REQUEST_HANDLER_FACTORY_H
#define ASAPO_RECEIVER_DATA_SERVER_REQUEST_HANDLER_FACTORY_H

#include "request/request_handler_factory.h"
#include "request/request_handler.h"
#include "preprocessor/definitions.h"

#include "rds_net_server.h"
#include "../data_cache.h"
#include "../statistics.h"

namespace asapo {

class ReceiverDataServerRequestHandlerFactory : public RequestHandlerFactory {
  public:
    ReceiverDataServerRequestHandlerFactory(RdsNetServer* server, DataCache* data_cache, Statistics* statistics);
    VIRTUAL std::unique_ptr<RequestHandler> NewRequestHandler(uint64_t thread_id, uint64_t* shared_counter) override;
  private:
    RdsNetServer* server_;
    DataCache* data_cache_;
    Statistics* statistics_;
};


}

#endif //ASAPO_RECEIVER_DATA_SERVER_REQUEST_HANDLER_FACTORY_H
