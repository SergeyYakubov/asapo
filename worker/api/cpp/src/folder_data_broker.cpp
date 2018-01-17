#include "folder_data_broker.h"

#include "system_wrappers/system_io.h"

namespace hidra2 {

WorkerErrorCode MapIOError(IOErrors io_err) {
    WorkerErrorCode err;
    switch (io_err) { // we do not use map due to performance reasons
    case IOErrors::kNoError:
        err = WorkerErrorCode::kOK;
        break;
    case IOErrors::kFileNotFound:
        err = WorkerErrorCode::kSourceNotFound;
        break;
    case IOErrors::kPermissionDenied:
        err = WorkerErrorCode::kPermissionDenied;
        break;
    case IOErrors::kReadError:
        err = WorkerErrorCode::kErrorReadingSource;
        break;
    case IOErrors::kMemoryAllocationError:
        err = WorkerErrorCode::kMemoryError;
        break;
    default:
        err = WorkerErrorCode::kUnknownIOError;
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
        return WorkerErrorCode::kSourceAlreadyConnected;
    }

    IOErrors io_err;
    filelist_ = io__->FilesInFolder(base_path_, &io_err);

    if (io_err == IOErrors::kNoError) {
        is_connected_ = true;
    }
    return MapIOError(io_err);
}

WorkerErrorCode FolderDataBroker::CanGetData(FileInfo* info, FileData* data, int nfile) const noexcept {
    if (!is_connected_) {
        return WorkerErrorCode::kSourceNotConnected;
    }

    if (info == nullptr && data == nullptr) {
        return WorkerErrorCode::kWrongInput;
    }

    if (nfile >= (int) filelist_.size()) {
        return WorkerErrorCode::kNoData;
    }
    return WorkerErrorCode::kOK;
}


WorkerErrorCode FolderDataBroker::GetNext(FileInfo* info, FileData* data) {
// could probably use atomic here, but just to make sure (tests showed no performance difference)
    mutex_.lock();
    int nfile_to_get = ++current_file_;
    mutex_.unlock();

    auto err = CanGetData(info, data, nfile_to_get);
    if (err != WorkerErrorCode::kOK) {
        return err;
    }

    FileInfo file_info = filelist_[nfile_to_get];
    if (info != nullptr) {
        *info = file_info;
    }

    if (data == nullptr) {
        return WorkerErrorCode::kOK;
    }

    IOErrors ioerr;
    *data = io__->GetDataFromFile(base_path_ + "/" + file_info.relative_path +
                                  (file_info.relative_path.empty() ? "" : "/") +
                                  file_info.base_name, file_info.size, &ioerr);

    return MapIOError(ioerr);
}


}