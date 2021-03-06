#include "receive_file_processor.h"

#include "asapo/io/io_factory.h"
#include "asapo/preprocessor/definitions.h"
#include "../../receiver_error.h"
#include "../../request.h"
#include "../../receiver_config.h"
#include "../../receiver_logger.h"

namespace asapo {

ReceiveFileProcessor::ReceiveFileProcessor() :  FileProcessor()  {

}

Error ReceiveFileProcessor::ProcessFile(const Request* request, bool overwrite) const {
    auto fsize = request->GetDataSize();
    auto socket = request->GetSocket();
    auto fname = request->GetFileName();
    std::string root_folder;
    auto err = GetRootFolder(request, &root_folder);
    if (err) {
        return err;
    }
    err =  io__->ReceiveDataToFile(socket, root_folder, fname, (size_t) fsize, true, overwrite);
    if (!err) {
        log__->Debug(RequestLog("received file", request).Append("size",std::to_string(fsize)).Append("name",
                root_folder + kPathSeparator + fname));
    }
    return err;
}

}
