#include <cstring>
#include <assert.h>
#include "connection.h"
#include "receiver_error.h"
#include "io/io_factory.h"

#include "receiver_logger.h"

namespace asapo {

size_t Connection::kRequestHandlerMaxBufferSize;
std::atomic<uint32_t> Connection::kNetworkProducerPeerImplGlobalCounter(0);

Connection::Connection(SocketDescriptor socket_fd, const std::string& address,
                       std::string receiver_tag) : request_factory__{new RequestFactory},
                       io__{GenerateDefaultIO()},
                       statistics__{new Statistics},
                       log__{GetDefaultReceiverLogger()},
                       authorizer__{new ConnectionAuthorizer} {
    socket_fd_ = socket_fd;
    connection_id_ = kNetworkProducerPeerImplGlobalCounter++;
    address_ = address;
    statistics__->AddTag("connection_from", address);
    statistics__->AddTag("receiver_tag", std::move(receiver_tag));

}

uint64_t Connection::GetId() const noexcept {
    return connection_id_;
}

NetworkErrorCode GetNetworkCodeFromError(const Error& err) {
    if (err) {
        if (err == IOErrorTemplates::kFileAlreadyExists) {
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
    if (err) {
        log__->Error("error while processing request from " + address_ + " - " + err->Explain());
    }
    io__->Send(socket_fd_, &generic_response, sizeof(GenericNetworkResponse), &err);
    if (err) {
        log__->Error("error sending response to " + address_ + " - " + err->Explain());
    }
    return err;
}

void Connection::ProcessStatisticsAfterRequest(const std::unique_ptr<Request>& request) const noexcept {
    statistics__->IncreaseRequestCounter();
    statistics__->IncreaseRequestDataVolume(request->GetDataSize() + sizeof(GenericRequestHeader) +
                                            sizeof(GenericNetworkResponse));
    statistics__->SendIfNeeded();
}

Error Connection::ReadAuthorizationHeaderIfNeeded() const {
    if (auth_header_was_read_) return nullptr;

    Error err;
    GenericRequestHeader generic_request_header;
    io__->Receive(socket_fd_, &generic_request_header, sizeof(GenericRequestHeader), &err);
    if (err) {
        log__->Error("error receive authorization header from " + address_ + " - " + err->Explain());
        return err;
    }

    if (generic_request_header.op_code != kOpcodeAuthorize) {
        std::string msg= "wrong code in authorization header from " + address_;
        log__->Error(msg);
        return TextError(msg);
    }

    beamtime_id_=std::string{generic_request_header.message};
    return nullptr;
}

Error Connection::SendAuthorizationResponseIfNeeded(const Error& auth_err) const {
    if (auth_header_was_read_) return nullptr;

    GenericNetworkResponse generic_response;
    if (auth_err == nullptr) {
        generic_response.error_code = kNetErrorNoError;
    } else {
        generic_response.error_code = kNetAuthorizationError;
        strcpy(generic_response.message, auth_err->Explain().c_str());
    }

    Error send_err;
    io__->Send(socket_fd_, &generic_response, sizeof(GenericNetworkResponse), &send_err);
    if (send_err) {
        log__->Error("error sending authorization response to " + address_ + " - " + send_err->Explain());
        return send_err;
    }
    auth_header_was_read_ = true;
    return nullptr;
}

Error Connection::AuthorizeIfNeeded() const {
    Error err = ReadAuthorizationHeaderIfNeeded();
    if (err == nullptr) {
        err = authorizer__->Authorize(beamtime_id_,address_);
    }
    Error err_send = SendAuthorizationResponseIfNeeded(err);

    return err == nullptr ? std::move(err_send) : std::move(err);
}

void Connection::Listen() const noexcept {
    while (true) {
        Error err = AuthorizeIfNeeded();
        if (err) {
            break;
        }
        auto request = WaitForNewRequest(&err);
        if (err) {
            if (err != ErrorTemplates::kEndOfFile) {
                log__->Error("error while waiting for request from " + address_ + " - " + err->Explain());
            }
            break;
        }
        if (!request) continue; //no error, but timeout
        log__->Debug("processing request from " + address_);
        err = ProcessRequest(request);
        if (err) {
            break;
        }
        ProcessStatisticsAfterRequest(request);
    }
    io__->CloseSocket(socket_fd_, nullptr);
    statistics__->Send();
    log__->Info("disconnected from " + address_);
}

std::unique_ptr<Request> Connection::WaitForNewRequest(Error* err) const noexcept {
//TODO: to be overwritten with MessagePack (or similar)
    GenericRequestHeader generic_request_header;
    statistics__-> StartTimer(StatisticEntity::kNetwork);
    io__-> ReceiveWithTimeout(socket_fd_, &generic_request_header,
                              sizeof(GenericRequestHeader), 50, err);
    if(*err) {
        if(*err == IOErrorTemplates::kTimeout) {
            *err = nullptr;//Not an error in this case
        }
        return nullptr;
    }
    statistics__-> StopTimer();
    return request_factory__->GenerateRequest(generic_request_header, socket_fd_, err);
}

}

