#include "request_handler_receive_data.h"
#include "asapo/io/io_factory.h"
#include "../request.h"
#include "../receiver_logger.h"

namespace asapo {

bool RequestHandlerReceiveData::NeedReceiveData(const Request* request) const {
    return request->GetDataSize() > 0 &&
           (request->GetCustomData()[asapo::kPosIngestMode] & asapo::kTransferData);
}

Error RequestHandlerReceiveData::ProcessRequest(Request* request) const {
    if (!NeedReceiveData(request)) {
        return nullptr;
    }
    auto err = request->PrepareDataBufferAndLockIfNeeded();
    if (err) {
        return err;
    }
    Error io_err;
    uint64_t byteCount = io__->Receive(request->GetSocket(), request->GetData(), (size_t) request->GetDataSize(), &io_err);
    request->GetInstancedStatistics()->AddIncomingBytes(byteCount);
    if (io_err) {
        err = ReceiverErrorTemplates::kProcessingError.Generate("cannot receive data",std::move(io_err));
    }
    if (err == nullptr) {
        log__->Debug(RequestLog("received request data", request).Append("size",request->GetDataSize()));
    }

    return err;
}

RequestHandlerReceiveData::RequestHandlerReceiveData() : io__{GenerateDefaultIO()}, log__{GetDefaultReceiverLogger()} {

}

StatisticEntity RequestHandlerReceiveData::GetStatisticEntity() const {
    return StatisticEntity::kNetworkIncoming;
}


}
