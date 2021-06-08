#include "requests_dispatcher.h"
#include "../request.h"
#include "asapo/io/io_factory.h"
#include "../receiver_logger.h"
#include "asapo/database/db_error.h"
namespace asapo {

RequestsDispatcher::RequestsDispatcher(SocketDescriptor socket_fd, std::string address,
                                       ReceiverStatistics* statistics, SharedCache cache) : statistics__{statistics},
    io__{GenerateDefaultIO()},
    log__{GetDefaultReceiverLogger()},
    request_factory__{new RequestFactory{cache}},
                  socket_fd_{socket_fd},
producer_uri_{std::move(address)} {
}

NetworkErrorCode GetNetworkCodeFromError(const Error& err) {
    if (err) {
        if (err == ReceiverErrorTemplates::kAuthorizationFailure) {
            return NetworkErrorCode::kNetAuthorizationError;
        } else if (err == ReceiverErrorTemplates::kUnsupportedClient) {
            return NetworkErrorCode::kNetErrorNotSupported;
        } else if (err == ReceiverErrorTemplates::kReAuthorizationFailure) {
            return NetworkErrorCode::kNetErrorReauthorize;
        } else if (err == DBErrorTemplates::kJsonParseError || err == ReceiverErrorTemplates::kBadRequest
                   || err == DBErrorTemplates::kNoRecord) {
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
    log__->Debug("processing request id " + std::to_string(request->GetDataID()) + ", opcode " +
                 std::to_string(request->GetOpCode()) + " from " + producer_uri_ );
    Error handle_err;
    handle_err = request->Handle(statistics__);
    if (handle_err) {
        if (handle_err == ReceiverErrorTemplates::kReAuthorizationFailure) {
            log__->Warning("warning processing request from " + producer_uri_ + " - " + handle_err->Explain());
        } else {
            log__->Error("error processing request from " + producer_uri_ + " - " + handle_err->Explain());
        }
    }
    return handle_err;
}

Error RequestsDispatcher::SendResponse(const std::unique_ptr<Request>& request, const Error& handle_error) const {
    log__->Debug("sending response to " + producer_uri_ );
    Error io_err;
    GenericNetworkResponse generic_response = CreateResponseToRequest(request, handle_error);
    io__->Send(socket_fd_, &generic_response, sizeof(GenericNetworkResponse), &io_err);
    if (io_err) {
        log__->Error("error sending response to " + producer_uri_ + " - " + io_err->Explain());
    }
    return io_err;
}

Error RequestsDispatcher::ProcessRequest(const std::unique_ptr<Request>& request) const noexcept {
    auto  handle_err = HandleRequest(request);
    auto  io_err = SendResponse(request, handle_err);
    return handle_err == nullptr ? std::move(io_err) : std::move(handle_err);
}

std::unique_ptr<Request> RequestsDispatcher::GetNextRequest(Error* err) const noexcept {
//TODO: to be overwritten with MessagePack (or similar)
    GenericRequestHeader generic_request_header;
    statistics__-> StartTimer(StatisticEntity::kNetwork);
    io__-> Receive(socket_fd_, &generic_request_header,
                   sizeof(GenericRequestHeader), err);
    if(*err) {
        if (*err == ErrorTemplates::kEndOfFile) {
            log__->Debug("error getting next request from " + producer_uri_ + " - " + "peer has performed an orderly shutdown");
        } else {
            log__->Error("error getting next request from " + producer_uri_ + " - " + (*err)->Explain());
        }
        return nullptr;
    }
    statistics__-> StopTimer();
    auto request = request_factory__->GenerateRequest(generic_request_header, socket_fd_, producer_uri_, err);
    if (*err) {
        log__->Error("error processing request from " + producer_uri_ + " - " + (*err)->Explain());
    }
    return request;
}

}
