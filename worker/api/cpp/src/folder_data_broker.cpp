#include "folder_data_broker.h"

#include "system_wrappers/system_io.h"

namespace hidra2 {

WorkerErrorCode MapIOError(IOError io_err) {
    WorkerErrorCode err;
    switch (io_err) { // we do not use map due to performance reasons
    case IOError::NO_ERROR:
        err = WorkerErrorCode::OK;
        break;
    case IOError::FILE_NOT_FOUND:
        err = WorkerErrorCode::SOURCE_NOT_FOUND;
        break;
    case IOError::PERMISSIONS_DENIED:
        err = WorkerErrorCode::PERMISSIONS_DENIED;
        break;
    case IOError::READ_ERROR:
        err = WorkerErrorCode::ERROR_READING_FROM_SOURCE;
        break;
    default:
        err = WorkerErrorCode::UNKNOWN_IO_ERROR;
        break;
    }

    return err;
}


FolderDataBroker::FolderDataBroker(const std::string& source_name) :
    base_path_{source_name}, io__{new hidra2::SystemIO}, is_connected_{false},
current_file_{0} {
}

WorkerErrorCode FolderDataBroker::Connect() {
    IOError io_err;
    if (is_connected_) {
        return WorkerErrorCode::SOURCE_ALREADY_CONNECTED;
    }

    filelist_ = io__->FilesInFolder(base_path_, &io_err);

    if (io_err == IOError::NO_ERROR) {
        is_connected_ = true;
    }
    return MapIOError(io_err);
}

WorkerErrorCode FolderDataBroker::CheckCanGetData(FileInfo* info, FileData* data) {
    if (!is_connected_) {
        return WorkerErrorCode::SOURCE_NOT_CONNECTED;
    }

    if (info == nullptr && data == nullptr) {
        return WorkerErrorCode::WRONG_INPUT;
    }

    if (current_file_ >= filelist_.size()) {
        return WorkerErrorCode::NO_DATA;
    }
    return WorkerErrorCode::OK;
}


WorkerErrorCode FolderDataBroker::GetNext(FileInfo* info, FileData* data) {
    auto err = CheckCanGetData(info, data);
    if (err != WorkerErrorCode::OK) {
        return err;
    }

    *info = filelist_[current_file_];
    current_file_++;

    if (data == nullptr) {
        return WorkerErrorCode::OK;
    }

    IOError ioerr;
    *data = io__->GetDataFromFile(base_path_ + "/" + info->relative_path +
                                  (info->relative_path.empty() ? "" : "/") +
                                  info->base_name, info->size, &ioerr);

    return MapIOError(ioerr);
}


}