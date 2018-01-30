#include "folder_data_broker.h"

#include "system_wrappers/system_io.h"
#include "broker_helpers.h"

namespace hidra2 {

FolderDataBroker::FolderDataBroker(const std::string& source_name) :
    io__{new hidra2::SystemIO}, base_path_{source_name}, is_connected_{false},
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

    if (info == nullptr) {
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

    *info = filelist_[nfile_to_get];

    if (data == nullptr) {
        return WorkerErrorCode::kOK;
    }

    IOErrors ioerr;
    *data = io__->GetDataFromFile(info->FullName(base_path_), info->size, &ioerr);

    return MapIOError(ioerr);
}


}