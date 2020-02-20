#include "receive_file_processor.h"

#include "io/io_factory.h"
#include "receiver_error.h"
#include "preprocessor/definitions.h"
#include "request.h"
#include "receiver_config.h"

namespace asapo {

ReceiveFileProcessor::ReceiveFileProcessor() :  FileProcessor()  {

}

Error ReceiveFileProcessor::ProcessFile(const Request* request, bool overwrite) const {
    auto fsize = request->GetDataSize();
    auto socket = request->GetSocket();
    auto fname = request->GetFileName();
    auto root_folder = request->GetFullPath(GetReceiverConfig()->root_folder);
    auto err =  io__->ReceiveDataToFile(socket, root_folder, fname, (size_t) fsize, true, overwrite);
    if (!err) {
        log__->Debug("received file of size " + std::to_string(fsize) + " to " + root_folder + kPathSeparator + fname);
    }
    return err;
}

}