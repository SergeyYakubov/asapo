#include "receiver_data_server_request_handler.h"

#include "../receiver_data_server_error.h"
#include "asapo/common/internal/version.h"
#include <sstream>

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

    auto startTime = ReceiverMonitoringClient::HelperTimeNow();
    CacheMeta* meta = GetSlotAndLock(receiver_request);
    if (!meta) {
        SendResponse(receiver_request, kNetErrorNoData);
    } else {
        HandleValidRequest(receiver_request, meta);
        data_cache_->UnlockSlot(meta);
    }
    auto timeTookToSend = ReceiverMonitoringClient::HelperTimeDiffInMicroseconds(startTime);

    auto requestSenderDetails = ExtractMonitoringInfoFromRequest(request);
    if (requestSenderDetails) {
        auto monitoring = server_->Monitoring();
        if (meta) {
            monitoring->SendReceiverRequestDataPoint(
                    requestSenderDetails->pipeline_step_id,
                    requestSenderDetails->instance_id,
                    requestSenderDetails->beamtime,
                    requestSenderDetails->source,
                    requestSenderDetails->stream,
                    "UnknownTODO", // TODO
                    meta->size,
                    timeTookToSend
            );
        } else {
            monitoring->SendRdsRequestWasMissDataPoint(
                    requestSenderDetails->pipeline_step_id,
                    requestSenderDetails->instance_id,
                    requestSenderDetails->beamtime,
                    requestSenderDetails->source,
                    requestSenderDetails->stream,
                    "UnknownTODO" // TODO
            );
        }
    }

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

// https://stackoverflow.com/a/46931770/
std::vector<std::string> split(const std::string& s, const std::string& delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}
std::unique_ptr<RequestSenderDetails> ReceiverDataServerRequestHandler::ExtractMonitoringInfoFromRequest(const GenericRequest* request) {
    std::string details(request->header.stream);

    std::vector<std::string> detailsParts = split(details, "§");

    if (detailsParts.size() != 5) {
        return nullptr;
    }

    return std::unique_ptr<RequestSenderDetails>(new RequestSenderDetails{
        detailsParts[0],
        detailsParts[1],
        detailsParts[2],
        detailsParts[3],
        detailsParts[4]
    });
}

}
