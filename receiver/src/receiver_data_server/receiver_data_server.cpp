#include "receiver_data_server.h"
#include "net_server/rds_tcp_server.h"
#include "receiver_data_server_logger.h"
#include "request_handler/receiver_data_server_request_handler_factory.h"

namespace asapo {

ReceiverDataServer::ReceiverDataServer(std::unique_ptr<RdsNetServer> net_server, LogLevel log_level,
                                       SharedCache data_cache, const ReceiverDataServerConfig& config) : net__{std::move(net_server)},
    log__{GetDefaultReceiverDataServerLogger()}, data_cache_{data_cache},
    statistics__{new Statistics()} {
    request_handler_factory_.reset(new ReceiverDataServerRequestHandlerFactory(net__.get(), data_cache_.get(),
                                   statistics__.get()));
    GetDefaultReceiverDataServerLogger()->SetLogLevel(log_level);
    request_pool__.reset(new RequestPool{(uint8_t)config.nthreads, request_handler_factory_.get(), log__});
    statistics__->AddTag("receiver_ds_tag", config.tag);
}

void ReceiverDataServer::Run() {
    while (true) {
        statistics__->SendIfNeeded();
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
