#include "request_handler_receive_data.h"
#include "io/io_factory.h"
#include "request.h"
#include "receiver_logger.h"
#include "receiver_config.h"
#include "preprocessor/definitions.h"

namespace asapo {



Error RequestHandlerReceiveData::ReceiveMetaData(Request* request) const {
    auto meta_size = request->GetMetaDataSize();
    if (meta_size == 0) {
        return nullptr;
    }

    Error err;
    auto buf = std::unique_ptr<uint8_t[]> {new uint8_t[meta_size]};
    io__->Receive(request->GetSocket(), (void*) buf.get(), meta_size, &err);
    if (err) {
        return err;
    }

    request->SetMetadata(std::string((char*)buf.get(), meta_size));
    return nullptr;
}


bool RequestHandlerReceiveData::NeedReceiveData(const Request* request) const {
    return request->GetDataSize() > 0 &&
           (request->GetCustomData()[asapo::kPosIngestMode] & asapo::kTransferData);
}

Error RequestHandlerReceiveData::ReceiveData(Request* request) const {
    if (!NeedReceiveData(request)) {
        return nullptr;
    }
    auto err = request->PrepareDataBufferAndLockIfNeeded();
    if (err) {
        return err;
    }
    io__->Receive(request->GetSocket(), request->GetData(), (size_t) request->GetDataSize(), &err);
    request->UnlockDataBufferIfNeeded();
    return err;
}


Error RequestHandlerReceiveData::ReceiveRequestContent(Request* request) const {
    auto err = ReceiveMetaData(request);
    if (err) {
        return err;
    }
    return ReceiveData(request);
}



Error RequestHandlerReceiveData::ProcessRequest(Request* request) const {
    return ReceiveRequestContent(request);
}

RequestHandlerReceiveData::RequestHandlerReceiveData() : io__{GenerateDefaultIO()} , log__{GetDefaultReceiverLogger()} {

}

StatisticEntity RequestHandlerReceiveData::GetStatisticEntity() const {
    return StatisticEntity::kNetwork;
}


}
