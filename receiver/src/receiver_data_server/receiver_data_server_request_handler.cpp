#include "receiver_data_server_request_handler.h"

#include "receiver_data_server_error.h"

namespace asapo {

ReceiverDataServerRequestHandler::ReceiverDataServerRequestHandler(const NetServer* server,
        DataCache* data_cache, Statistics* statistics): log__{GetDefaultReceiverDataServerLogger()}, statistics__{statistics},
    server_{server}, data_cache_{data_cache} {

}


bool ReceiverDataServerRequestHandler::CheckRequest(const ReceiverDataServerRequest* request) {
    return  request->header.op_code == kOpcodeGetBufferData;
}

Error ReceiverDataServerRequestHandler::SendData(const ReceiverDataServerRequest* request,
                                                 void* data,
                                                 CacheMeta* meta) {
    auto err = SendResponce(request, kNetErrorNoError);
    if (err) {
        data_cache_->UnlockSlot(meta);
        return err;
    }
    err = server_->SendData(request->source_id, data, request->header.data_size);
    log__->Debug("sending data from memory cache, id:" + std::to_string(request->header.data_id));
    data_cache_->UnlockSlot(meta);
    return err;
}

void* ReceiverDataServerRequestHandler::GetSlot(const ReceiverDataServerRequest* request, CacheMeta** meta) {
    void* buf = nullptr;
    if (data_cache_) {
        buf = data_cache_->GetSlotToReadAndLock(request->header.data_id, request->header.data_size,
                                                meta);
        if (!buf) {
            log__->Debug("data not found in memory cache, id:" + std::to_string(request->header.data_id));
        }

    }
    if (buf == nullptr) {
        SendResponce(request, kNetErrorNoData);
    }
    return buf;
}


bool ReceiverDataServerRequestHandler::ProcessRequestUnlocked(GenericRequest* request, bool* retry) {
    *retry = false;
    auto receiver_request = dynamic_cast<ReceiverDataServerRequest*>(request);
    if (!CheckRequest(receiver_request)) {
        SendResponce(receiver_request, kNetErrorWrongRequest);
        server_->HandleAfterError(receiver_request->source_id);
        log__->Error("wrong request, code:" + std::to_string(receiver_request->header.op_code));
        return true;
    }

    CacheMeta* meta;
    auto buf = GetSlot(receiver_request, &meta);
    if (buf == nullptr) {
        return true;
    }

    SendData(receiver_request, buf, meta);
    statistics__->IncreaseRequestCounter();
    statistics__->IncreaseRequestDataVolume(receiver_request->header.data_size);
    return true;
}

bool ReceiverDataServerRequestHandler::ReadyProcessRequest() {
    return true; // always ready
}

void ReceiverDataServerRequestHandler::PrepareProcessingRequestLocked() {
// do nothing
}

void ReceiverDataServerRequestHandler::TearDownProcessingRequestLocked(bool processing_succeeded) {
// do nothing
}

Error ReceiverDataServerRequestHandler::SendResponce(const ReceiverDataServerRequest* request, NetworkErrorCode code) {
    GenericNetworkResponse responce;
    responce.op_code = kOpcodeGetBufferData;
    responce.error_code = code;
    return server_->SendData(request->source_id, &responce, sizeof(GenericNetworkResponse));
}

void ReceiverDataServerRequestHandler::ProcessRequestTimeout(GenericRequest* request) {
// do nothing
}

}