#ifndef ASAPO_RECEIVER_DATA_SERVER_REQUEST_HANDLER_FACTORY_H
#define ASAPO_RECEIVER_DATA_SERVER_REQUEST_HANDLER_FACTORY_H

#include "request/request_handler_factory.h"
#include "request/request_handler.h"
#include "preprocessor/definitions.h"


namespace asapo {

class ReceiverDataServerRequestHandlerFactory : public RequestHandlerFactory {
  public:
    VIRTUAL std::unique_ptr<RequestHandler> NewRequestHandler(uint64_t thread_id, uint64_t* shared_counter) override;
  private:
};


}

#endif //ASAPO_RECEIVER_DATA_SERVER_REQUEST_HANDLER_FACTORY_H
