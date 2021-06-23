#ifndef ASAPO_RECEIVER_REQUEST_HANDLER_FACTORY_H
#define ASAPO_RECEIVER_REQUEST_HANDLER_FACTORY_H

#include "receiver_discovery_service.h"
#include "asapo/request/request_handler_factory.h"
#include "asapo/request/request_handler.h"
#include "asapo/preprocessor/definitions.h"
#include "asapo/producer/common.h"

namespace  asapo {

class ProducerRequestHandlerFactory : public RequestHandlerFactory {
  public:
    ProducerRequestHandlerFactory(ReceiverDiscoveryService* discovery_service);
    ProducerRequestHandlerFactory(std::string destination_folder);
    virtual std::unique_ptr<RequestHandler> NewRequestHandler(uint64_t thread_id, uint64_t* shared_counter) override;
    virtual ~ProducerRequestHandlerFactory() { };
  private:
    RequestHandlerType type_;
    ReceiverDiscoveryService* discovery_service_{nullptr};
    std::string destination_folder_;
};


}

#endif //ASAPO_RECEIVER_REQUEST_HANDLER_FACTORY_H
