#include "file_processor.h"

#include <tuple>

#include "asapo/io/io_factory.h"
#include "../../receiver_logger.h"
#include "../../request.h"

namespace asapo {

FileProcessor::FileProcessor() : io__{GenerateDefaultIO()}, log__{GetDefaultReceiverLogger()} {

}

Error CheckFileNameIsAbsolute(const std::string &fname) {
    auto posr = fname.find("..");
    if (posr != std::string::npos) {
        return ReceiverErrorTemplates::kBadRequest.Generate("cannot use relative path in path name " + fname);
    }
    return nullptr;
}

Error CheckFileFolderCorrespondsToRequestSourceType(const Request *request, const std::string &fname) {
    auto pos = fname.find(asapo::kPathSeparator);
    if (pos == std::string::npos) {
        return ReceiverErrorTemplates::kBadRequest.Generate("cannot extract root folder from file path " + fname);
    }
    std::string file_folder = fname.substr(0, pos);
    auto folder_by_type = GetStringFromSourceType(request->GetSourceType());
    if (file_folder != folder_by_type) {
        return ReceiverErrorTemplates::kBadRequest.Generate(
            "file " + fname + " is not in " + folder_by_type + " folder");
    }
    return nullptr;
}

Error GetRootFolderFromRequest(const Request *request, std::string *root_folder) {
    bool rawToOffline = request->GetCustomData()[asapo::kPosIngestMode] & asapo::kWriteRawDataToOffline;
    if (request->GetSourceType() == SourceType::kProcessed || rawToOffline) {
        *root_folder = request->GetOfflinePath();
        return nullptr;
    }

    *root_folder = request->GetOnlinePath();
    if (root_folder->empty()) {
        return ReceiverErrorTemplates::kBadRequest.Generate("online path not available");
    }
    return nullptr;
}

Error GetRootFolder(const Request *request, std::string *root_folder) {
    auto fname = request->GetFileName();

    auto err = CheckFileNameIsAbsolute(fname);
    if (err != nullptr) {
        return err;
    }

    err = CheckFileFolderCorrespondsToRequestSourceType(request, fname);
    if (err != nullptr) {
        return err;
    }

    return GetRootFolderFromRequest(request, root_folder);
}

}
