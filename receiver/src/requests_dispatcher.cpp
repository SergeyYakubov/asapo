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
    log__->Debug("processing request id " + std::to_string(request->GetDataID()) + " from " + producer_uri_ );
    Error handle_err;
    handle_err = request->Handle(statistics__);
    GenericNetworkResponse generic_response;
    generic_response.error_code = GetNetworkCodeFromError(handle_err);
    strcpy(generic_response.message, "");
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

std::unique_ptr<Request> RequestsDispatcher::GetNextRequest(Error* err) const noexcept {
//TODO: to be overwritten with MessagePack (or similar)
    GenericRequestHeader generic_request_header;
    statistics__-> StartTimer(StatisticEntity::kNetwork);
    io__-> Receive(socket_fd_, &generic_request_header,
                   sizeof(GenericRequestHeader), err);
    if(*err) {
        log__->Error("error getting next request from " + producer_uri_ + " - " + (*err)->
                     Explain()
                    );
        return nullptr;
    }
    statistics__-> StopTimer();
    auto request = request_factory__->GenerateRequest(generic_request_header, socket_fd_, producer_uri_, err);
    if (*err) {
        log__->Error("error processing request from " + producer_uri_ + " - " + (*err)->
                     Explain()
                    );
    }

    return request;
}

}
