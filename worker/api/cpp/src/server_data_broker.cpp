#include "server_data_broker.h"
#include "system_wrappers/system_io.h"
#include "curl_http_client.h"

namespace hidra2 {

ServerDataBroker::ServerDataBroker(const std::string& server_uri,
                                   const std::string& source_name):
    io__{new hidra2::SystemIO}, httpclient__{new hidra2::CurlHttpClient},
server_uri_{server_uri}, source_name_{source_name} {
}

Error ServerDataBroker::Connect() {
    return nullptr;
}

Error ServerDataBroker::GetFileInfoFromServer(FileInfo* info, const std::string& operation) {
    std::string full_uri = server_uri_ + "/database/" + source_name_ + "/" + operation;
    Error err;
    HttpCode code;
    auto responce = httpclient__->Get(full_uri, &code, &err);

    if (err != nullptr) {
        return err;
    }

    err = HttpCodeToWorkerError(code);
    if (err != nullptr) {
        return err;
    }

    if (!info->SetFromJson(responce)) {
        return TextError(WorkerErrorMessage::kErrorReadingSource);
    }
    return nullptr;
}

Error ServerDataBroker::GetNext(FileInfo* info, FileData* data) {
    if (info == nullptr) {
        return TextError(WorkerErrorMessage::kWrongInput);
    }

    auto  err = GetFileInfoFromServer(info, "next");
    if (err != nullptr) {
        return err;
    }

    if (data == nullptr) {
        return nullptr;
    }

    Error error;
    *data = io__->GetDataFromFile(info->FullName(""), info->size, &error);
    return error;
}

}