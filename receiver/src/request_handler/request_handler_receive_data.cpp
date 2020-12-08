#include "request_handler_receive_data.h"
#include "asapo/io/io_factory.h"
#include "../request.h"
#include "../receiver_logger.h"
#include "../receiver_config.h"
#include "asapo/preprocessor/definitions.h"

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
    io__->Receive(request->GetSocket(), request->GetData(), (size_t) request->GetDataSize(), &err);
    request->UnlockDataBufferIfNeeded();
    return err;
}

RequestHandlerReceiveData::RequestHandlerReceiveData() : io__{GenerateDefaultIO()}, log__{GetDefaultReceiverLogger()} {

}

StatisticEntity RequestHandlerReceiveData::GetStatisticEntity() const {
    return StatisticEntity::kNetwork;
}


}
