#include "folder_data_broker.h"

#include <map>

#include "system_wrappers/system_io.h"

namespace hidra2 {

WorkerErrorCode MapIOError(IOErrors io_err) {
    WorkerErrorCode err;
    switch (io_err) { // we do not use map due to performance reasons
    case IOErrors::NO_ERROR:
        err = WorkerErrorCode::ERR__NO_ERROR;
        break;
    case IOErrors::FOLDER_NOT_FOUND:
        err = WorkerErrorCode::SOURCE_NOT_FOUND;
        break;
    default:
        err = WorkerErrorCode::UNKNOWN_IO_ERROR;
        break;
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