#include "folder_data_broker.h"

#include <map>

#include "system_wrappers/system_io.h"

namespace hidra2 {

WorkerErrorCode MapIOError(IOErrors io_err) {
    std::map<IOErrors,WorkerErrorCode> error_mapping= {
        {IOErrors::NO_ERROR,WorkerErrorCode::ERR__NO_ERROR},
        {IOErrors::FOLDER_NOT_FOUND,WorkerErrorCode::SOURCE_NOT_FOUND}
    };

    auto search=error_mapping.find(io_err);

    WorkerErrorCode err = WorkerErrorCode::UNKNOWN_IO_ERROR;
    if(search != error_mapping.end()) {
        err=search->second;
    }

    return err;
}


FolderDataBroker::FolderDataBroker(const std::string& source_name) : base_path_{source_name},
    io__{new hidra2::SystemIO} {
}

WorkerErrorCode FolderDataBroker::Connect() {
    IOErrors io_err;
    filelist_ = io__->FilesInFolder(base_path_, &io_err);
    return MapIOError(io_err);
}


}