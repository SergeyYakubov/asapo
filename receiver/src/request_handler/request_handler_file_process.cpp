#include "request_handler_file_process.h"
#include "io/io_factory.h"
#include "../request.h"
#include "../receiver_logger.h"
#include "../receiver_config.h"
#include "preprocessor/definitions.h"

namespace asapo {

Error RequestHandlerFileProcess::ProcessRequest(Request* request) const {

    auto err = file_processor_->ProcessFile(request, false);
    if (err == IOErrorTemplates::kFileAlreadyExists) {
        return ProcessFileExistSituation(request);
    }

    return err;
}

Error RequestHandlerFileProcess::ProcessFileExistSituation(Request* request) const {
    auto err_duplicate = request->CheckForDuplicates();
    if (err_duplicate == nullptr) {
        request->SetResponseMessage("file has been overwritten", ResponseMessageType::kWarning);
        log__->Warning(std::string("overwriting file " ) + request->GetOfflinePath() + kPathSeparator + request->GetFileName());
        return file_processor_->ProcessFile(request, true);
    }

    if (err_duplicate == ReceiverErrorTemplates::kWarningDuplicatedRequest) {
        request->SetAlreadyProcessedFlag();
        request->SetResponseMessage("duplicated request, ignored", ResponseMessageType::kWarning);
        log__->Warning("duplicated request, id: " + std::to_string(request->GetDataID()));
        return nullptr;
    }

    return err_duplicate;
}


RequestHandlerFileProcess::RequestHandlerFileProcess(const FileProcessor* file_processor) : io__{GenerateDefaultIO()},
    log__{GetDefaultReceiverLogger()}, file_processor_{file_processor} {

}

StatisticEntity RequestHandlerFileProcess::GetStatisticEntity() const {
    return StatisticEntity::kDisk;
}

}
