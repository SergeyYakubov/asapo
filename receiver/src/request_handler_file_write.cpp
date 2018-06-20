#include "request_handler_file_write.h"
#include "io/io_factory.h"
#include "request.h"
#include "receiver_logger.h"
#include "receiver_config.h"
#include "preprocessor/definitions.h"

namespace asapo {

Error RequestHandlerFileWrite::ProcessRequest(Request* request) const {
    auto fsize = request->GetDataSize();
    if (fsize <= 0 || fsize > kMaxFileSize) {
        return ReceiverErrorTemplates::kBadRequest.Generate();
    }

    const FileData& data = request->GetData();

    auto fname = request->GetFileName();
    auto root_folder = GetReceiverConfig()->root_folder + kPathSeparator + request->GetBeamtimeId() + kPathSeparator;
    auto err =  io__->WriteDataToFile(root_folder + fname, data, fsize);
    if (!err) {
        log__->Debug("saved file of size " + std::to_string(fsize) + " to " + root_folder + fname);
    }
    return err;

}

RequestHandlerFileWrite::RequestHandlerFileWrite() : io__{GenerateDefaultIO()} , log__{GetDefaultReceiverLogger()} {

}

StatisticEntity RequestHandlerFileWrite::GetStatisticEntity() const {
    return StatisticEntity::kDisk;
}


}
