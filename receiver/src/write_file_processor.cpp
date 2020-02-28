#include "write_file_processor.h"

#include "io/io_factory.h"
#include "receiver_error.h"
#include "preprocessor/definitions.h"
#include "request.h"
#include "receiver_config.h"

namespace asapo {

WriteFileProcessor::WriteFileProcessor() : FileProcessor()  {

}


Error WriteFileProcessor::ProcessFile(const Request* request, bool overwrite) const {
    auto fsize = request->GetDataSize();
    if (fsize <= 0) {
        return ReceiverErrorTemplates::kBadRequest.Generate("wrong file size");
    }

    auto data = request->GetData();
    auto fname = request->GetFileName();
    auto root_folder = request->GetOfflinePath();

    auto err =  io__->WriteDataToFile(root_folder, fname, (uint8_t*)data, (size_t) fsize, true, overwrite);
    if (!err) {
        log__->Debug("saved file of size " + std::to_string(fsize) + " to " + root_folder + kPathSeparator + fname);
    }

    return err;
}

}