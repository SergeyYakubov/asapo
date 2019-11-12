#include "request_handler_file_receive.h"
#include "io/io_factory.h"
#include "request.h"
#include "receiver_logger.h"
#include "receiver_config.h"
#include "preprocessor/definitions.h"

namespace asapo {

Error RequestHandlerFileReceive::ProcessRequest(Request* request) const {
    auto fsize = request->GetDataSize();
    auto socket = request->GetSocket();
    auto fname = request->GetFileName();
    auto root_folder = GetReceiverConfig()->root_folder + kPathSeparator
                       + request->GetBeamline() + kPathSeparator
                       + request->GetBeamtimeId();
    auto err =  io__->ReceiveDataToFile(socket, root_folder, fname, (size_t) fsize, true);
    if (!err) {
        log__->Debug("received file of size " + std::to_string(fsize) + " to " + root_folder + kPathSeparator + fname);
    }
    return err;
}

RequestHandlerFileReceive::RequestHandlerFileReceive() : io__{GenerateDefaultIO()} , log__{GetDefaultReceiverLogger()} {

}

StatisticEntity RequestHandlerFileReceive::GetStatisticEntity() const {
    return StatisticEntity::kDisk;
}


}
