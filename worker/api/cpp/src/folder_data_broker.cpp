#include "folder_data_broker.h"

#include "system_wrappers/io_factory.h"

namespace hidra2 {

FolderDataBroker::FolderDataBroker(const std::string& source_name) :
    io__{GenerateDefaultIO()}, base_path_{source_name}, is_connected_{false},
    current_file_{ -1} {
}

Error FolderDataBroker::Connect() {
    std::lock_guard<std::mutex> lock{mutex_};

    if (is_connected_) {
        return TextError(WorkerErrorMessage::kSourceAlreadyConnected);
    }

    Error error;
    filelist_ = io__->FilesInFolder(base_path_, &error);

    if (error == nullptr) {
        is_connected_ = true;
        return nullptr;
    }

    return error;
}

Error FolderDataBroker::CanGetData(FileInfo* info, FileData* data, int nfile) const noexcept {
    if (!is_connected_) {
        return TextError(WorkerErrorMessage::kSourceNotConnected);
    }

    if (info == nullptr) {
        return TextError(WorkerErrorMessage::kWrongInput);
    }

    if (nfile >= (int) filelist_.size()) {
        return Error{TextErrorWithType(WorkerErrorMessage::kNoData, ErrorType::kEndOfFile)};
    }
    return nullptr;
}


Error FolderDataBroker::GetNext(FileInfo* info, FileData* data) {
// could probably use atomic here, but just to make sure (tests showed no performance difference)
    mutex_.lock();
    int nfile_to_get = ++current_file_;
    mutex_.unlock();

    auto err = CanGetData(info, data, nfile_to_get);
    if (err != nullptr) {
        return err;
    }

    *info = filelist_[nfile_to_get];

    if (data == nullptr) {
        return nullptr;
    }

    Error error;
    *data = io__->GetDataFromFile(info->FullName(base_path_), info->size, &error);

    return error;
}


}
