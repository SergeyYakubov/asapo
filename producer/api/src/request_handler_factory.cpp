#include "request_handler_factory.h"

#include "request_handler_tcp.h"
#include "request_handler_filesystem.h"


namespace  asapo {

std::unique_ptr<RequestHandler> RequestHandlerFactory::NewRequestHandler(uint64_t thread_id, uint64_t* shared_counter) {
    switch (type_) {
    case asapo::RequestHandlerType::kTcp:
        return std::unique_ptr<RequestHandler> {new RequestHandlerTcp(discovery_service_, thread_id, shared_counter)};
        case asapo::RequestHandlerType::kFilesystem:
            return std::unique_ptr<RequestHandler> {new RequestHandlerFilesystem(destination_folder_, thread_id)};

    }
    return nullptr;
}

RequestHandlerFactory::RequestHandlerFactory(ReceiverDiscoveryService* discovery_service): type_{RequestHandlerType::kTcp},
    discovery_service_{discovery_service} {
    if (discovery_service_) {
        discovery_service_->StartCollectingData();
    }
}

RequestHandlerFactory::RequestHandlerFactory(std::string destination_folder): type_{RequestHandlerType::kFilesystem},
                                                                              destination_folder_{std::move(destination_folder)} {
}


}