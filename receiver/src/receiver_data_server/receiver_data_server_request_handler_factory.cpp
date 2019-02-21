#include "receiver_data_server_request_handler_factory.h"

namespace asapo {

std::unique_ptr<RequestHandler> ReceiverDataServerRequestHandlerFactory::NewRequestHandler(uint64_t thread_id,
        uint64_t* shared_counter) {
    return std::unique_ptr<RequestHandler>();
}
}