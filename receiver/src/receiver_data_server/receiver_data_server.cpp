#include "receiver_data_server.h"
#include "tcp_server.h"
#include "receiver_data_server_logger.h"

namespace asapo {

ReceiverDataServer::ReceiverDataServer() : request_pool__{new RequestPool}, net__{new TcpServer()},
log__{GetDefaultReceiverDataServerLogger()} {
}

void ReceiverDataServer::Run() {
    while (true) {
        Error err;
        auto requests = net__->GetNewRequests(&err);
        if (err) {
            log__->Error(std::string("receiver data server stopped: ") + err->Explain());
            return;
        }
        err = request_pool__->AddRequests(requests);
        if (err) {
            log__->Error(std::string("receiver data server stopped: ") + err->Explain());
            return;
        }
    }
}

}