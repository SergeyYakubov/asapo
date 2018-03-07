#include <fcntl.h>
#include <iostream>
#include <fstream>      // std::ifstream
#include <sstream>
#include <cerrno>
#include <cstring>
#include <algorithm>

#include <system_wrappers/system_io.h>


namespace hidra2 {

Error IOErrorFromErrno() {
    const char* message;
    switch (errno) {
    case 0:
        return nullptr;
    case ENOENT:
    case ENOTDIR:
        message = IOErrors::kFileNotFound;
        break;
    case EACCES:
        message = IOErrors::kPermissionDenied;
        break;
    default:
        message = IOErrors::kUnknownError;
        break;
    }
    return TextError(message);
}

uint64_t SystemIO::Read(int fd, uint8_t* array, uint64_t fsize, Error* err) const noexcept {
    uint64_t totalbytes = 0;
    int64_t readbytes = 0;
    do {
        readbytes = read(fd, array + totalbytes, fsize);
        totalbytes += readbytes;
    } while (readbytes > 0 && totalbytes < fsize);

    if (totalbytes != fsize) {
        *err = TextError(IOErrors::kReadError);
    }
    return totalbytes;
}

int SystemIO::OpenFile(const std::string& fname, Error* err) const noexcept {
    errno = 0;
    int fd = open(fname.c_str(), O_RDONLY);
    *err = IOErrorFromErrno();
    if (*err != nullptr) {
        (*err)->Append(fname);
        return 0;
    }
    return fd;
}

uint8_t* AllocateArray(uint64_t fsize, Error* err) {
    uint8_t* data_array = nullptr;
    try {
        data_array = new uint8_t[fsize];
    } catch (...) {
        *err = TextError(IOErrors::kMemoryAllocationError);
        return nullptr;
    }
    return data_array;
}

FileData SystemIO::GetDataFromFile(const std::string& fname, uint64_t fsize, Error* err) const noexcept {
    auto fd = OpenFile(fname, err);
    if (*err != nullptr) {
        return nullptr;
    }

    auto data_array = AllocateArray(fsize, err);
    if (*err != nullptr) {
        return nullptr;
    }

    Read(fd, data_array, fsize, err);
    if (*err != nullptr) {
        close(fd);
        (*err)->Append(fname);
        return nullptr;
    }

    close(fd);
    return FileData{data_array};
}

void SortFileList(FileInfos* file_list) {
    std::sort(file_list->begin(), file_list->end(),
    [](FileInfo const & a, FileInfo const & b) {
        return a.modify_date < b.modify_date;
    });
}

void StripBasePath(const std::string& folder, FileInfos* file_list) {
    auto n_erase = folder.size() + 1;
    for (auto& file : *file_list) {
        file.name.erase(0, n_erase);
    }
}

void AssignIDs(FileInfos* file_list) {
    int64_t id = 0;
    for (auto& file : *file_list) {
        file.id = ++id;
    }
}


FileInfos SystemIO::FilesInFolder(const std::string& folder, Error* err) const {
    FileInfos files{};
    CollectFileInformationRecursivly(folder, &files, err);
    if (*err != nullptr) {
        return {};
    }
    StripBasePath(folder, &files);
    SortFileList(&files);
    AssignIDs(&files);
    return files;
}

std::string SystemIO::ReadFileToString(const std::string& fname, Error* err)const noexcept {
    auto info =  GetFileInfo(fname, err);
    if (*err != nullptr) {
        return "";
    }

    auto data = GetDataFromFile(fname, info.size, err);
    if (*err != nullptr) {
        return "";
    }

    return std::string(reinterpret_cast<const char*>(data.get()), info.size);
}



}