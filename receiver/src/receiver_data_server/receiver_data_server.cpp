#include "receiver_data_server.h"
#include "tcp_server.h"
#include "receiver_data_server_logger.h"

namespace asapo {

ReceiverDataServer::ReceiverDataServer(std::string address, LogLevel log_level) : request_pool__{new RequestPool}, net__{new TcpServer(address)},
log__{GetDefaultReceiverDataServerLogger()} {
    GetDefaultReceiverDataServerLogger()->SetLogLevel(log_level);
}

void ReceiverDataServer::Run() {
    while (true) {
        Error err;
        auto requests = net__->GetNewRequests(&err);
        if (err == IOErrorTemplates::kTimeout) {
            continue;
        }
        if (!err) {
            err = request_pool__->AddRequests(requests);
        }
        if (err) {
            log__->Error(std::string("receiver data server stopped: ") + err->Explain());
            return;
        }
    }
}

}