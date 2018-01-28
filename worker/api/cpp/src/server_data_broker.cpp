#include "server_data_broker.h"

namespace hidra2 {

ServerDataBroker::ServerDataBroker(const std::string& source_name) {

}

WorkerErrorCode ServerDataBroker::Connect() {
    return WorkerErrorCode::kOK;
}

WorkerErrorCode ServerDataBroker::GetNext(FileInfo* info, FileData* data) {
    return {};
}

}