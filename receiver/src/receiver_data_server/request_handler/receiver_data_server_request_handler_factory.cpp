#include "receiver_data_server_request_handler_factory.h"

#include "receiver_data_server_request_handler.h"

namespace asapo {

std::unique_ptr<RequestHandler> ReceiverDataServerRequestHandlerFactory::NewRequestHandler(uint64_t,
        uint64_t* ) {
    return std::unique_ptr<RequestHandler> {new ReceiverDataServerRequestHandler(server_, data_cache_, statistics_)};
}
ReceiverDataServerRequestHandlerFactory::ReceiverDataServerRequestHandlerFactory(RdsNetServer* server,
        DataCache* data_cache, Statistics* statistics) : server_{server},
    data_cache_{data_cache}, statistics_{statistics} {

}
}
