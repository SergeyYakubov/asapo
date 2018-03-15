#include "network_producer_peer_impl.h"
#include "receiver.h"
#include <cmath>

namespace hidra2 {

const std::vector<NetworkProducerPeerImpl::RequestHandlerInformation>
NetworkProducerPeerImpl::StaticInitRequestHandlerList() {
    std::vector<NetworkProducerPeerImpl::RequestHandlerInformation> vec(kNetOpcodeCount);

    // Add new opcodes here
    vec[kNetOpcodeSendData] = {
        sizeof(SendDataRequest),
        sizeof(SendDataResponse),
        (NetworkProducerPeerImpl::RequestHandler)& NetworkProducerPeerImpl::HandleSendDataRequestInternalCaller
    };

    for(RequestHandlerInformation& handler_information : vec) {
        //Adjust max size needed for a request/response-buffer

        if(handler_information.request_size > kRequestHandlerMaxBufferSize) {
            kRequestHandlerMaxBufferSize = handler_information.request_size;
        }
        if(handler_information.response_size > kRequestHandlerMaxBufferSize) {
            kRequestHandlerMaxBufferSize = handler_information.response_size;
        }
    }

    return vec;
}


void NetworkProducerPeerImpl::HandleSendDataRequestInternalCaller(NetworkProducerPeerImpl* self,
        const SendDataRequest* request,
        SendDataResponse* response,
        Error* err) noexcept {
    self->HandleSendDataRequest(request, response, err);
}

void NetworkProducerPeerImpl::HandleSendDataRequest(const SendDataRequest* request, SendDataResponse* response,
        Error* err) noexcept {
    ReceiveAndSaveFile(request->file_id, request->file_size, err);

    if(!*err) {
        response->error_code = NET_ERR__NO_ERROR;
        return;
    }

    if(*err == IOErrorTemplates::kFileAlreadyExists) {
        response->error_code = NET_ERR__FILEID_ALREADY_IN_USE;
    } else {
        std::cout << "[" << GetConnectionId() << "] Unexpected ReceiveAndSaveFile error " << *err << std::endl;
        response->error_code = NET_ERR__INTERNAL_SERVER_ERROR;
        //self->io->CloseSocket(self->socket_fd_, nullptr); TODO: Might want to close the connection?
    }
}

}

