#include "folder_data_broker.h"

namespace hidra2 {

FolderDataBroker::FolderDataBroker(const std::string &source_name) : source_name_{source_name},
                                                                     io_{nullptr} {
}

WorkerErrorCode FolderDataBroker::Connect() {
    return WorkerErrorCode::ERR__NO_ERROR;
}

void FolderDataBroker::set_io_(void* io) {
    io_ = io;
}

}