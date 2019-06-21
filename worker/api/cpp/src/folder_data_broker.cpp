#include "folder_data_broker.h"

#include "io/io_factory.h"
#include "preprocessor/definitions.h"

namespace asapo {

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

Error FolderDataBroker::CanGetData(FileInfo* info, FileData* data, uint64_t nfile) const noexcept {
    if (!is_connected_) {
        return TextError(WorkerErrorMessage::kSourceNotConnected);
    }

    if (info == nullptr) {
        return TextError(WorkerErrorMessage::kWrongInput);
    }

    if (nfile >= (uint64_t) filelist_.size()) {
        return Error{TextErrorWithType(WorkerErrorMessage::kNoData, ErrorType::kEndOfFile)};
    }
    return nullptr;
}

Error FolderDataBroker::GetFileByIndex(uint64_t nfile_to_get, FileInfo* info, FileData* data) {
    auto err = CanGetData(info, data, nfile_to_get);
    if (err != nullptr) {
        return err;
    }

    *info = filelist_[(size_t)nfile_to_get];

    if (data == nullptr) {
        return nullptr;
    }

    Error error;
    *data = io__->GetDataFromFile(info->FullName(base_path_), &info->size, &error);

    return error;
}


Error FolderDataBroker::GetNext(FileInfo* info, std::string group_id, FileData* data) {
// could probably use atomic here, but just to make sure (tests showed no performance difference)
    mutex_.lock();
    uint64_t nfile_to_get = ++current_file_;
    mutex_.unlock();

    return GetFileByIndex(nfile_to_get, info, data);

}
Error FolderDataBroker::GetLast(FileInfo* info, std::string group_id,  FileData* data) {
    uint64_t nfile_to_get = filelist_.size() - 1;
    return GetFileByIndex(nfile_to_get, info, data);
}

std::string FolderDataBroker::GenerateNewGroupId(Error* err) {
    *err = nullptr;
    return "";
}
Error FolderDataBroker::ResetCounter(std::string group_id) {
    std::lock_guard<std::mutex> lock{mutex_};
    current_file_ = -1;
    return nullptr;
}
uint64_t FolderDataBroker::GetNDataSets(Error* err) {
    std::lock_guard<std::mutex> lock{mutex_};
    return filelist_.size();
}

Error FolderDataBroker::GetById(uint64_t id, FileInfo* info, std::string group_id, FileData* data) {
    return GetFileByIndex(id - 1 , info, data);
}

std::string FolderDataBroker::GetBeamtimeMeta(Error* err) {
    return io__->ReadFileToString(base_path_ + kPathSeparator + "beamtime_global.meta", err);
}

FileInfos FolderDataBroker::QueryImages(std::string query, Error* err) {
    *err=TextError("Not supported for folder data broker");
    return FileInfos{};
}

}
