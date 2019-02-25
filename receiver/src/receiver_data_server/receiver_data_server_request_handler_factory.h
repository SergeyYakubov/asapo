#ifndef ASAPO_RECEIVER_DATA_SERVER_REQUEST_HANDLER_FACTORY_H
#define ASAPO_RECEIVER_DATA_SERVER_REQUEST_HANDLER_FACTORY_H

#include "request/request_handler_factory.h"
#include "request/request_handler.h"
#include "preprocessor/definitions.h"

#include "net_server.h"
#include "../data_cache.h"

namespace asapo {

class ReceiverDataServerRequestHandlerFactory : public RequestHandlerFactory {
  public:
    ReceiverDataServerRequestHandlerFactory (const NetServer* server, DataCache* data_cache);
    VIRTUAL std::unique_ptr<RequestHandler> NewRequestHandler(uint64_t thread_id, uint64_t* shared_counter) override;
  private:
    const NetServer* server_;
    DataCache* data_cache_;
};


}

#endif //ASAPO_RECEIVER_DATA_SERVER_REQUEST_HANDLER_FACTORY_H
