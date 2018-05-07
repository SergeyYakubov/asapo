#include "request_handler_file_write.h"
#include "io/io_factory.h"
#include "request.h"
#include "receiver_logger.h"

namespace asapo {

Error RequestHandlerFileWrite::ProcessRequest(const Request& request) const {
    auto fsize = request.GetDataSize();
    if (fsize <= 0 || fsize > kMaxFileSize) {
        return ReceiverErrorTemplates::kBadRequest.Generate();
    }

    const FileData& data = request.GetData();

    auto fname = request.GetFileName();
//TODO: folder to write in config file
    auto err =  io__->WriteDataToFile("files/" + fname, data, fsize);
    if (!err) {
        log__->Debug("saved file of size " + std::to_string(fsize) + " to files/" + fname);
    }
    return err;

}

RequestHandlerFileWrite::RequestHandlerFileWrite() : io__{GenerateDefaultIO()} , log__{GetDefaultReceiverLogger()} {

}

StatisticEntity RequestHandlerFileWrite::GetStatisticEntity() const {
    return StatisticEntity::kDisk;
}


}
