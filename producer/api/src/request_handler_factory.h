#ifndef ASAPO_REQUEST_HANDLER_FACTORY_H
#define ASAPO_REQUEST_HANDLER_FACTORY_H

#include "request_handler.h"
#include "receiver_discovery_service.h"


namespace  asapo {

#ifdef UNIT_TESTS
#define VIRTUAL virtual
#endif

enum class RequestHandlerType {
  kTcp,
  kFilesystem
};

class RequestHandlerFactory {
 public:
  RequestHandlerFactory(RequestHandlerType type, ReceiverDiscoveryService* discovery_service);
  VIRTUAL std::unique_ptr<RequestHandler> NewRequestHandler(uint64_t thread_id);
 private:
  RequestHandlerType type_;
  ReceiverDiscoveryService* discovery_service_;
};


}

#endif //ASAPO_REQUEST_HANDLER_FACTORY_H
