#include "write_file_processor.h"

#include "asapo/preprocessor/definitions.h"
#include "../../receiver_error.h"
#include "../../request.h"
#include "../../receiver_logger.h"

namespace asapo {

WriteFileProcessor::WriteFileProcessor() : FileProcessor()  {

}


Error WriteFileProcessor::ProcessFile(const Request* request, bool overwrite) const {
    auto fsize = request->GetDataSize();
    if (fsize <= 0) {
        auto err = ReceiverErrorTemplates::kBadRequest.Generate("wrong file size");
        err->AddDetails("size",std::to_string(fsize));
        return err;
    }

    auto data = request->GetData();
    auto fname = request->GetFileName();
    std::string root_folder;
    auto err = GetRootFolder(request, &root_folder);
    if (err) {
        return err;
    }

    err =  io__->WriteDataToFile(root_folder, fname, (uint8_t*)data, (size_t) fsize, true, overwrite);
    if (!err) {
        log__->Debug(RequestLog("saved file", request).Append("size",std::to_string(fsize)).Append("name",
                                                                                                      root_folder + kPathSeparator + fname));
    }

    return err;
}

}
