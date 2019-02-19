#ifndef ASAPO_RECEIVER_REQUEST_HANDLER_FACTORY_H
#define ASAPO_RECEIVER_REQUEST_HANDLER_FACTORY_H

#include "receiver_discovery_service.h"
#include "request/request_handler_factory.h"
#include "request/request_handler.h"
#include "preprocessor/definitions.h"
#include "producer/common.h"

namespace  asapo {

class ProducerRequestHandlerFactory : public RequestHandlerFactory {
  public:
  ProducerRequestHandlerFactory(ReceiverDiscoveryService* discovery_service);
  ProducerRequestHandlerFactory(std::string destination_folder);
  VIRTUAL std::unique_ptr<RequestHandler> NewRequestHandler(uint64_t thread_id, uint64_t* shared_counter) override;
  private:
    RequestHandlerType type_;
    ReceiverDiscoveryService* discovery_service_{nullptr};
    std::string destination_folder_;
};


}

#endif //ASAPO_RECEIVER_REQUEST_HANDLER_FACTORY_H
