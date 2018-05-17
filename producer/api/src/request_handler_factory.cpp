#include "request_handler_factory.h"

#include "request_handler_tcp.h"

namespace  asapo {

std::unique_ptr<RequestHandler> RequestHandlerFactory::NewRequestHandler(uint64_t thread_id) {
    switch (type_) {
        case asapo::RequestHandlerType::kTcp:
            return std::unique_ptr<RequestHandler>{new RequestHandlerTcp(discovery_service_,thread_id)};
    }
    return nullptr;
}

RequestHandlerFactory::RequestHandlerFactory(RequestHandlerType type,ReceiverDiscoveryService* discovery_service): type_{type},
discovery_service_{discovery_service}{
if (discovery_service_) {
    discovery_service_->StartCollectingData();
}


}


}