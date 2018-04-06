#include <cstring>
#include <assert.h>
#include "connection.h"
#include "receiver_error.h"
#include "io/io_factory.h"

namespace hidra2 {

size_t Connection::kRequestHandlerMaxBufferSize;
std::atomic<uint32_t> Connection::kNetworkProducerPeerImplGlobalCounter(0);

Connection::Connection(SocketDescriptor socket_fd, const std::string& address): request_factory__{new RequestFactory},
io__{GenerateDefaultIO()}, statistics__{new Statistics} {
    socket_fd_ = socket_fd;
    connection_id_ = kNetworkProducerPeerImplGlobalCounter++;
    address_ = address;
}

uint64_t Connection::GetId() const noexcept {
    return connection_id_;
}

NetworkErrorCode GetNetworkCodeFromError(const Error& err) {
    if(err) {
        if(err == IOErrorTemplates::kFileAlreadyExists) {
            return NetworkErrorCode::kNetErrorFileIdAlreadyInUse;
        } else {
            return NetworkErrorCode::kNetErrorInternalServerError;
        }
    }
    return NetworkErrorCode::kNetErrorNoError;

}

Error Connection::ProcessRequest(const std::unique_ptr<Request>& request) const noexcept {
    Error err;
    err = request->Handle(&statistics__);
    GenericNetworkResponse generic_response;
    generic_response.error_code = GetNetworkCodeFromError(err);
    if(err) {
        std::cerr << "[" << GetId() << "] Error while handling request: " << err << std::endl;
    }
    io__->Send(socket_fd_, &generic_response, sizeof(GenericNetworkResponse), &err);
    return err;
}


void Connection::ProcessStatisticsAfterRequest(const std::unique_ptr<Request>& request) const noexcept {
    statistics__->IncreaseRequestCounter();
    statistics__->IncreaseRequestDataVolume(request->GetDataSize() + sizeof(GenericNetworkRequestHeader) +
                                            sizeof(GenericNetworkResponse));
    statistics__->SendIfNeeded();
}

void Connection::Listen() const noexcept {
    while(true) {
        Error err;
        auto request = WaitForNewRequest(&err);
        if(err) {
            std::cerr << "[" << GetId() << "] Error while waiting for request: " << err << std::endl;
            break;
        }
        if (!request) continue; //no error, but timeout
        err = ProcessRequest(request);
        if(err) {
            std::cerr << "[" << GetId() << "] Error sending response: " << err << std::endl;
            break;
        }
        ProcessStatisticsAfterRequest(request);
    }
    io__->CloseSocket(socket_fd_, nullptr);
    std::cout << "[" << GetId() << "] Disconnected." << std::endl;
}


std::unique_ptr<Request> Connection::WaitForNewRequest(Error* err) const noexcept {
    //TODO: to be overwritten with MessagePack (or similar)
    GenericNetworkRequestHeader generic_request_header;
    statistics__->StartTimer(StatisticEntity::kNetwork);
    io__->ReceiveWithTimeout(socket_fd_, &generic_request_header, sizeof(GenericNetworkRequestHeader), 50, err);
    if(*err) {
        if(*err == IOErrorTemplates::kTimeout) {
            *err = nullptr;//Not an error in this case
        }
        return nullptr;
    }
    statistics__->StopTimer();
    return request_factory__->GenerateRequest(generic_request_header, socket_fd_, err);
}

}

