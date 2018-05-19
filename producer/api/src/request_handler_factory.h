#ifndef ASAPO_REQUEST_HANDLER_FACTORY_H
#define ASAPO_REQUEST_HANDLER_FACTORY_H

#include "request_handler.h"
#include "receiver_discovery_service.h"


namespace  asapo {

#ifdef UNIT_TESTS
#define VIRTUAL virtual
#endif


class RequestHandlerFactory {
  public:
    RequestHandlerFactory(ReceiverDiscoveryService* discovery_service);
    RequestHandlerFactory(std::string destination_folder);
  VIRTUAL std::unique_ptr<RequestHandler> NewRequestHandler(uint64_t thread_id, uint64_t* shared_counter);
  private:
    RequestHandlerType type_;
    ReceiverDiscoveryService* discovery_service_{nullptr};
    std::string destination_folder_;
};


}

#endif //ASAPO_REQUEST_HANDLER_FACTORY_H
