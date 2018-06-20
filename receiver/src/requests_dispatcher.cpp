#include "requests_dispatcher.h"
#include "request.h"
#include "io/io_factory.h"
#include "receiver_logger.h"

namespace asapo {

RequestsDispatcher::RequestsDispatcher(SocketDescriptor socket_fd, std::string address,
                                       Statistics* statistics) : statistics__{statistics},
    io__{GenerateDefaultIO()},
    log__{GetDefaultReceiverLogger()},
    request_factory__{new RequestFactory{}},
                  socket_fd_{socket_fd},
producer_uri_{std::move(address)} {
}

NetworkErrorCode GetNetworkCodeFromError(const Error& err) {
    if (err) {
        if (err == IOErrorTemplates::kFileAlreadyExists) {
            return NetworkErrorCode::kNetErrorFileIdAlreadyInUse;
        } else if (err == ReceiverErrorTemplates::kAuthorizationFailure) {
            return NetworkErrorCode::kNetAuthorizationError;
        } else {
            return NetworkErrorCode::kNetErrorInternalServerError;
        }
    }
    return NetworkErrorCode::kNetErrorNoError;
}

Error RequestsDispatcher::ProcessRequest(const std::unique_ptr<Request>& request) const noexcept {
    log__->Debug("processing request from " + producer_uri_ );
    Error handle_err;
    handle_err = request->Handle(statistics__);
    GenericNetworkResponse generic_response;
    generic_response.error_code = GetNetworkCodeFromError(handle_err);
    if (handle_err) {
        log__->Error("error processing request from " + producer_uri_ + " - " + handle_err->Explain());
        strncpy(generic_response.message, handle_err->Explain().c_str(), kMaxMessageSize);
    }
    log__->Debug("sending response to " + producer_uri_ );
    Error io_err;
    io__->Send(socket_fd_, &generic_response, sizeof(GenericNetworkResponse), &io_err);
    if (io_err) {
        log__->Error("error sending response to " + producer_uri_ + " - " + io_err->Explain());
    }
    return handle_err == nullptr ? std::move(io_err) : std::move(handle_err);
}

std::unique_ptr<Request> RequestsDispatcher::GetNextRequest(Error* err)
const noexcept {
//TODO: to be overwritten with MessagePack (or similar)
    GenericRequestHeader generic_request_header;
    statistics__->
    StartTimer(StatisticEntity::kNetwork);
    io__->
    Receive(socket_fd_, &generic_request_header,
            sizeof(GenericRequestHeader), err);
    if(*err) {
        log__->Error("error getting next request from " + producer_uri_ + " - " + (*err)->
                     Explain()
                    );
        return nullptr;
    }
    statistics__->
    StopTimer();
    auto request = request_factory__->GenerateRequest(generic_request_header, socket_fd_, producer_uri_, err);
    if (*err) {
        log__->Error("error processing request from " + producer_uri_ + " - " + (*err)->
                     Explain()
                    );
    }

    return
        request;
}



/*
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
                       authorizer__{new ConnectionAuthorizer},
                       requests_dispatcher__{new RequestsDispatcher}{
    socket_fd_ = socket_fd;
    connection_id_ = kNetworkProducerPeerImplGlobalCounter++;
    address_ = address;
    statistics__->AddTag("connection_from", address);
    statistics__->AddTag("receiver_tag", std::move(receiver_tag));
}

uint64_t Connection::GetId() const noexcept {
    return connection_id_;
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



}




 */

}
