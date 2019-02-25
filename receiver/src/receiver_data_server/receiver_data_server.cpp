#include "receiver_data_server.h"
#include "tcp_server.h"
#include "receiver_data_server_logger.h"
#include "receiver_data_server_request_handler_factory.h"

namespace asapo {

ReceiverDataServer::ReceiverDataServer(std::string address, LogLevel log_level, uint8_t n_threads,
                                       SharedCache data_cache) : net__{new TcpServer(address)},
log__{GetDefaultReceiverDataServerLogger()}, data_cache_{data_cache} {
    request_handler_factory_.reset(new ReceiverDataServerRequestHandlerFactory(net__.get(), data_cache_.get()));
    GetDefaultReceiverDataServerLogger()->SetLogLevel(log_level);
    request_pool__.reset(new RequestPool{n_threads, request_handler_factory_.get(), log__});
}

void ReceiverDataServer::Run() {
    while (true) {
        Error err;
        auto requests = net__->GetNewRequests(&err);
        if (err == IOErrorTemplates::kTimeout) {
            continue;
        }
        if (!err) {
            err = request_pool__->AddRequests(std::move(requests));
        }
        if (err) {
            log__->Error(std::string("receiver data server stopped: ") + err->Explain());
            return;
        }
    }
}

}