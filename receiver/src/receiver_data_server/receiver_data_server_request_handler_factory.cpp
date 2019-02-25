#include "receiver_data_server_request_handler_factory.h"

#include "receiver_data_server_request_handler.h"

namespace asapo {

std::unique_ptr<RequestHandler> ReceiverDataServerRequestHandlerFactory::NewRequestHandler(uint64_t thread_id,
        uint64_t* shared_counter) {
    return std::unique_ptr<RequestHandler> {new ReceiverDataServerRequestHandler(server_, data_cache_)};
}
ReceiverDataServerRequestHandlerFactory::ReceiverDataServerRequestHandlerFactory(const NetServer* server,
        DataCache* data_cache) : server_{server},
    data_cache_{data_cache} {

}
}