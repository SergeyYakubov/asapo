#include "folder_data_broker.h"

#include "system_wrappers/system_io.h"

namespace hidra2 {

WorkerErrorCode MapIOError(IOErrors io_err) {
    WorkerErrorCode err;
    switch (io_err) { // we do not use map due to performance reasons
    case IOErrors::NO_ERROR:
        err = WorkerErrorCode::OK;
        break;
    case IOErrors::FILE_NOT_FOUND:
        err = WorkerErrorCode::SOURCE_NOT_FOUND;
        break;
    case IOErrors::PERMISSIONS_DENIED:
        err = WorkerErrorCode::PERMISSIONS_DENIED;
        break;
    case IOErrors::READ_ERROR:
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
current_file_{ -1} {
}

WorkerErrorCode FolderDataBroker::Connect() {
    std::lock_guard<std::mutex> lock{mutex_};

    if (is_connected_) {
        return WorkerErrorCode::SOURCE_ALREADY_CONNECTED;
    }

    IOErrors io_err;
    filelist_ = io__->FilesInFolder(base_path_, &io_err);

    if (io_err == IOErrors::NO_ERROR) {
        is_connected_ = true;
    }
    return MapIOError(io_err);
}

WorkerErrorCode FolderDataBroker::CanGetData(FileInfo* info, FileData* data, int nfile) const {
    if (!is_connected_) {
        return WorkerErrorCode::SOURCE_NOT_CONNECTED;
    }

    if (info == nullptr && data == nullptr) {
        return WorkerErrorCode::WRONG_INPUT;
    }

    if (nfile >= filelist_.size()) {
        return WorkerErrorCode::NO_DATA;
    }
    return WorkerErrorCode::OK;
}


WorkerErrorCode FolderDataBroker::GetNext(FileInfo* info, FileData* data) {
// could probably use atomic here, but just to make sure (tests showed no performance difference)
    mutex_.lock();
    int nfile_to_get = ++current_file_;
    mutex_.unlock();

    auto err = CanGetData(info, data, nfile_to_get);
    if (err != WorkerErrorCode::OK) {
        return err;
    }

    FileInfo file_info = filelist_[nfile_to_get];
    if (info != nullptr) {
        *info = file_info;
    }

    if (data == nullptr) {
        return WorkerErrorCode::OK;
    }

    IOErrors ioerr;
    *data = io__->GetDataFromFile(base_path_ + "/" + file_info.relative_path +
                                  (file_info.relative_path.empty() ? "" : "/") +
                                  file_info.base_name, file_info.size, &ioerr);

    return MapIOError(ioerr);
}


}