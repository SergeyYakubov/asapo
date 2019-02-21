#include "receiver_data_server_request.h"
#include "receiver_data_server.h"

namespace asapo {

ReceiverDataServerRequest::ReceiverDataServerRequest(GenericRequestHeader header, uint64_t net_id,
        const NetServer* server) :
    GenericRequest(std::move(header)),
    net_id{net_id}, server{server} {
}



}