#include "request_handler_receive_metadata.h"
#include "asapo/io/io_factory.h"
#include "../request.h"
#include "../receiver_logger.h"

namespace asapo {

Error RequestHandlerReceiveMetaData::ProcessRequest(Request* request) const {
    auto meta_size = request->GetMetaDataSize();
    if (meta_size == 0) {
        return nullptr;
    }

    Error err;
    auto buf = std::unique_ptr<uint8_t[]> {new uint8_t[meta_size]};
    uint64_t byteCount = io__->Receive(request->GetSocket(), (void*) buf.get(), meta_size, &err);
    request->GetInstancedStatistics()->AddIncomingBytes(byteCount);
    if (err) {
        return ReceiverErrorTemplates::kProcessingError.Generate("cannot receive metadata",std::move(err));
    }
    log__->Debug(RequestLog("received request metadata", request).Append("size",meta_size));
    request->SetMetadata(std::string((char*)buf.get(), meta_size));
    return nullptr;
}

RequestHandlerReceiveMetaData::RequestHandlerReceiveMetaData() : io__{GenerateDefaultIO()}, log__{GetDefaultReceiverLogger()} {

}

StatisticEntity RequestHandlerReceiveMetaData::GetStatisticEntity() const {
    return StatisticEntity::kNetworkIncoming;
}


}
