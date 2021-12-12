#include "requests_dispatcher.h"
#include "asapo/io/io_factory.h"
#include "../receiver_logger.h"
#include "asapo/database/db_error.h"
namespace asapo {

RequestsDispatcher::RequestsDispatcher(SocketDescriptor socket_fd, std::string address,
                                       ReceiverStatistics* statistics, SharedCache cache) : statistics__{statistics},
    io__{GenerateDefaultIO()},
    log__{
    GetDefaultReceiverLogger()},
request_factory__{
    new RequestFactory{
        cache}},
socket_fd_{socket_fd},
producer_uri_{
    std::move(address)} {
}

NetworkErrorCode GetNetworkCodeFromError(const Error& err) {
    if (err) {
        if (err == ReceiverErrorTemplates::kAuthorizationFailure) {
            return NetworkErrorCode::kNetAuthorizationError;
        } else if (err == ReceiverErrorTemplates::kUnsupportedClient) {
            return NetworkErrorCode::kNetErrorNotSupported;
        } else if (err == ReceiverErrorTemplates::kReAuthorizationFailure) {
            return NetworkErrorCode::kNetErrorReauthorize;
        } else if (err == ReceiverErrorTemplates::kBadRequest) {
            return NetworkErrorCode::kNetErrorWrongRequest;
        } else {
            return NetworkErrorCode::kNetErrorInternalServerError;
        }
    }
    return NetworkErrorCode::kNetErrorNoError;
}

GenericNetworkResponse RequestsDispatcher::CreateResponseToRequest(const std::unique_ptr<Request>& request,
        const Error& handle_error) const {
    GenericNetworkResponse generic_response;
    generic_response.op_code = request->GetOpCode();
    generic_response.error_code = GetNetworkCodeFromError(handle_error);
    strcpy(generic_response.message, "");
    if (handle_error) {
        strncpy(generic_response.message, handle_error->Explain().c_str(), kMaxMessageSize);
    }
    if (request->GetResponseMessage().size() > 0) {
        if (request->GetResponseMessageType() == ResponseMessageType::kWarning) {
            generic_response.error_code = kNetErrorWarning;
        }
        strncpy(generic_response.message, request->GetResponseMessage().c_str(), kMaxMessageSize);
    }
    return generic_response;
}

Error RequestsDispatcher::HandleRequest(const std::unique_ptr<Request>& request) const {
    log__->Debug(RequestLog("got new request", request.get()));
    Error handle_err;
    handle_err = request->Handle(statistics__);
    if (handle_err) {
        if (handle_err == ReceiverErrorTemplates::kReAuthorizationFailure) {
            log__->Warning(LogMessageWithFields(handle_err).Append(RequestLog("", request.get())));
        } else {
            log__->Error(LogMessageWithFields(handle_err).Append(RequestLog("", request.get())));
        }
    }
    return handle_err;
}

Error RequestsDispatcher::SendResponse(const std::unique_ptr<Request>& request, const Error& handle_error) const {
    Error io_err;
    GenericNetworkResponse generic_response = CreateResponseToRequest(request, handle_error);
    auto log = RequestLog("sending response", request.get()).
               Append("response", NetworkErrorCodeToString(generic_response.error_code));
    log__->Debug(log);
    io__->Send(socket_fd_, &generic_response, sizeof(GenericNetworkResponse), &io_err);
    if (io_err) {
        auto err = ReceiverErrorTemplates::kProcessingError.Generate("cannot send response",std::move(io_err));
        log__->Error(LogMessageWithFields(err).Append(RequestLog("", request.get())));
        return err;
    }
    return nullptr;
}

Error RequestsDispatcher::ProcessRequest(const std::unique_ptr<Request>& request) const noexcept {
    auto handle_err = HandleRequest(request);
    auto send_err = SendResponse(request, handle_err);
    return handle_err == nullptr ? std::move(send_err) : std::move(handle_err);
}

std::unique_ptr<Request> RequestsDispatcher::GetNextRequest(Error* err) const noexcept {
//TODO: to be overwritten with MessagePack (or similar)
    GenericRequestHeader generic_request_header;
    statistics__->StartTimer(StatisticEntity::kNetwork);
    Error io_err;
    io__->Receive(socket_fd_, &generic_request_header,
                  sizeof(GenericRequestHeader), &io_err);
    if (io_err) {
        *err = ReceiverErrorTemplates::kProcessingError.Generate("cannot get next request",std::move(io_err));
        if ((*err)->GetCause() != GeneralErrorTemplates::kEndOfFile) {
            log__->Error(LogMessageWithFields(*err).Append("origin", HostFromUri(producer_uri_)));
        }
        return nullptr;
    }
    statistics__->StopTimer();
    auto request = request_factory__->GenerateRequest(generic_request_header, socket_fd_, producer_uri_, err);
    if (*err) {
        log__->Error(LogMessageWithFields(*err).Append("origin", HostFromUri(producer_uri_)));
    }
    return request;
}

}
