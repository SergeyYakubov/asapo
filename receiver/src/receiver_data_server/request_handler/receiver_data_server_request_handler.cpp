#include "receiver_data_server_request_handler.h"

#include "../receiver_data_server_error.h"
#include "asapo/common/internal/version.h"
namespace asapo {

ReceiverDataServerRequestHandler::ReceiverDataServerRequestHandler(RdsNetServer* server,
        DataCache* data_cache, Statistics* statistics): log__{GetDefaultReceiverDataServerLogger()}, statistics__{statistics},
    server_{server}, data_cache_{data_cache} {

}


bool ReceiverDataServerRequestHandler::CheckRequest(const ReceiverDataServerRequest* request, NetworkErrorCode* code) {
    if (request->header.op_code != kOpcodeGetBufferData) {
        *code = kNetErrorWrongRequest;
        return false;
    }
    int verClient = VersionToNumber(request->header.api_version);
    int verService = VersionToNumber(GetRdsApiVersion());
    if (verClient > verService) {
        *code = kNetErrorNotSupported;
        return false;
    }

    return true;
}

Error ReceiverDataServerRequestHandler::SendResponse(const ReceiverDataServerRequest* request, NetworkErrorCode code) {
    GenericNetworkResponse response{};
    response.op_code = kOpcodeGetBufferData;
    response.error_code = code;
    return server_->SendResponse(request, &response);
}

Error ReceiverDataServerRequestHandler::SendResponseAndSlotData(const ReceiverDataServerRequest* request,
        const CacheMeta* meta) {
    GenericNetworkResponse response{};
    response.op_code = kOpcodeGetBufferData;
    response.error_code = kNetErrorNoError;
    return server_->SendResponseAndSlotData(request, &response,
                                            meta);
}

CacheMeta* ReceiverDataServerRequestHandler::GetSlotAndLock(const ReceiverDataServerRequest* request) {
    CacheMeta* meta = nullptr;
    if (data_cache_) {
        data_cache_->GetSlotToReadAndLock(request->header.data_id, request->header.data_size, &meta);
        if (!meta) {
            log__->Debug("data not found in memory cache, id:" + std::to_string(request->header.data_id));
        }
    }
    return meta;
}

bool ReceiverDataServerRequestHandler::ProcessRequestUnlocked(GenericRequest* request, bool* retry) {
    *retry = false;
    auto receiver_request = dynamic_cast<ReceiverDataServerRequest*>(request);
    NetworkErrorCode code;
    if (!CheckRequest(receiver_request, &code)) {
        HandleInvalidRequest(receiver_request, code);
        return true;
    }

    CacheMeta* meta = GetSlotAndLock(receiver_request);
    if (!meta) {
        SendResponse(receiver_request, kNetErrorNoData);
        return true;
    }

    HandleValidRequest(receiver_request, meta);
    data_cache_->UnlockSlot(meta);
    return true;
}


bool ReceiverDataServerRequestHandler::ReadyProcessRequest() {
    return true; // always ready
}

void ReceiverDataServerRequestHandler::PrepareProcessingRequestLocked() {
// do nothing
}

void ReceiverDataServerRequestHandler::TearDownProcessingRequestLocked(bool /*processing_succeeded*/) {
// do nothing
}

void ReceiverDataServerRequestHandler::ProcessRequestTimeoutUnlocked(GenericRequest* /*request*/) {
// do nothing
}

void ReceiverDataServerRequestHandler::HandleInvalidRequest(const ReceiverDataServerRequest* receiver_request,
        NetworkErrorCode code) {
    SendResponse(receiver_request, code);
    server_->HandleAfterError(receiver_request->source_id);
    switch (code) {
    case NetworkErrorCode::kNetErrorWrongRequest:
        log__->Error("wrong request, code:" + std::to_string(receiver_request->header.op_code));
        break;
    case NetworkErrorCode::kNetErrorNotSupported:
        log__->Error("unsupported client, version: " + std::string(receiver_request->header.api_version));
        break;
    default:
        break;
    };

}

void ReceiverDataServerRequestHandler::HandleValidRequest(const ReceiverDataServerRequest* receiver_request,
        const CacheMeta* meta) {
    auto err = SendResponseAndSlotData(receiver_request, meta);
    if (err) {
        log__->Error("failed to send slot:" + err->Explain());
        server_->HandleAfterError(receiver_request->source_id);
    } else {
        statistics__->IncreaseRequestCounter();
        statistics__->IncreaseRequestDataVolume(receiver_request->header.data_size);
    }
}

}
