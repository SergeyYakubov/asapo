#include "server_data_broker.h"
#include "system_wrappers/system_io.h"
#include "curl_http_client.h"
#include "io_map.h"

namespace hidra2 {

ServerDataBroker::ServerDataBroker(const std::string& server_uri,
                                   const std::string& source_name):
    io__{new hidra2::SystemIO}, httpclient__{new hidra2::CurlHttpClient},
server_uri_{server_uri}, source_name_{source_name} {
}

WorkerErrorCode ServerDataBroker::Connect() {
    return WorkerErrorCode::kOK;
}

WorkerErrorCode ServerDataBroker::GetNext(FileInfo* info, FileData* data) {
    if (info == nullptr && data == nullptr) {
        return WorkerErrorCode::kWrongInput;
    }

    std::string full_uri = server_uri_ + "/next?database=" + source_name_;
    WorkerErrorCode err;
    auto responce = httpclient__->Get(full_uri, &err);
    if (err != WorkerErrorCode::kOK) {
        return err;
    }

    FileInfo file_info;
    if (!file_info.SetFromJson(responce)) {
        return WorkerErrorCode::kErrorReadingSource;
    }

    if (info!= nullptr){
        *info = file_info;
    }

    if (data == nullptr){
        return WorkerErrorCode::kOK;
    }

    IOErrors ioerr;
    *data = io__->GetDataFromFile(file_info.relative_path +
                                  (file_info.relative_path.empty() ? "" : "/") +
        file_info.base_name, file_info.size, &ioerr);

    return hidra2::MapIOError(ioerr);
}

}