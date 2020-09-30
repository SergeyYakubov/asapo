#include "file_processor.h"

#include "io/io_factory.h"
#include "../receiver_logger.h"
#include "../receiver_config.h"
#include "../request.h"

namespace asapo {

FileProcessor::FileProcessor(): io__{GenerateDefaultIO()}, log__{GetDefaultReceiverLogger()} {

}

Error GetRootFolder(const Request* request, std::string* root_folder) {
    std::string root;
    auto fname = request->GetFileName();
    auto pos = fname.find(asapo::kPathSeparator);
    if (pos == std::string::npos) {
        return ReceiverErrorTemplates::kBadRequest.Generate("cannot extract root folder from file path "+fname);
    }

    auto posr = fname.find("..");
    if (posr != std::string::npos) {
        return ReceiverErrorTemplates::kBadRequest.Generate("cannot use relative path in path name "+fname);
    }

    std::string file_folder = fname.substr(0, pos);
    auto folder_by_type = GetStringFromSourceType(request->GetSourceType());
    if (file_folder!=folder_by_type) {
        return ReceiverErrorTemplates::kBadRequest.Generate("file "+fname+" is not in "+folder_by_type +" folder");
    }

    switch (request->GetSourceType()) {
        case SourceType::kProcessed:
            root = request->GetOfflinePath();
            break;
        case SourceType::kRaw:
            root = request->GetOnlinePath();
            if (root.empty()) {
                return ReceiverErrorTemplates::kBadRequest.Generate("online path not available");
            }
            break;
    }

    *root_folder = root;
    return nullptr;
}


}
