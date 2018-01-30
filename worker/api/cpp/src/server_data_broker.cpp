#include "server_data_broker.h"
#include "system_wrappers/system_io.h"
#include "curl_http_client.h"
#include "broker_helpers.h"

namespace hidra2 {

ServerDataBroker::ServerDataBroker(const std::string& server_uri,
                                   const std::string& source_name):
    io__{new hidra2::SystemIO}, httpclient__{new hidra2::CurlHttpClient},
server_uri_{server_uri}, source_name_{source_name} {
}

WorkerErrorCode ServerDataBroker::Connect() {
    return WorkerErrorCode::kOK;
}

WorkerErrorCode ServerDataBroker::GetFileInfoFromServer(FileInfo* info, const std::string& operation) {
    std::string full_uri = server_uri_ + "/database/" + source_name_ + "/" + operation;
    WorkerErrorCode err;
    auto responce = httpclient__->Get(full_uri, &err);
    if (err != WorkerErrorCode::kOK) {
        return err;
    }
    if (!info->SetFromJson(responce)) {
        return WorkerErrorCode::kErrorReadingSource;
    }
    return WorkerErrorCode::kOK;
}

WorkerErrorCode ServerDataBroker::GetNext(FileInfo* info, FileData* data) {
    if (info == nullptr) {
        return WorkerErrorCode::kWrongInput;
    }

    auto  err = GetFileInfoFromServer(info, "next");
    if (err != WorkerErrorCode::kOK) {
        return err;
    }

    if (data == nullptr) {
        return WorkerErrorCode::kOK;
    }

    IOErrors ioerr;
    *data = io__->GetDataFromFile(info->FullName(""), info->size, &ioerr);
    return hidra2::MapIOError(ioerr);
}

}