#include "request_handler_receive_metadata.h"
#include "io/io_factory.h"
#include "../request.h"
#include "../receiver_logger.h"
#include "../receiver_config.h"
#include "preprocessor/definitions.h"

namespace asapo {

Error RequestHandlerReceiveMetaData::ProcessRequest(Request* request) const {
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

RequestHandlerReceiveMetaData::RequestHandlerReceiveMetaData() : io__{GenerateDefaultIO()}, log__{GetDefaultReceiverLogger()} {

}

StatisticEntity RequestHandlerReceiveMetaData::GetStatisticEntity() const {
    return StatisticEntity::kNetwork;
}


}
