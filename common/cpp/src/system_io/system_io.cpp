#include <fcntl.h>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <algorithm>

#include <system_wrappers/system_io.h>


namespace hidra2 {

IOError IOErrorFromErrno() {
    IOError err;
    switch (errno) {
    case 0:
        err = IOError::kNoError;
        break;
    case ENOENT:
    case ENOTDIR:
        err = IOError::kFileNotFound;
        break;
    case EACCES:
        err = IOError::kPermissionDenied;
        break;
    default:
        err = IOError::kUnknownError;
        break;
    }
    return err;
}

void SystemIO::ReadWholeFile(int fd, uint8_t* array, uint64_t fsize, IOError* err) const noexcept {
    uint64_t totalbytes = 0;
    int64_t readbytes = 0;
    do {
        readbytes = read(fd, array + totalbytes, fsize);
        totalbytes += readbytes;
    } while (readbytes > 0 && totalbytes < fsize);

    if (totalbytes != fsize) {
        *err = IOError::kReadError;
    }
}

FileData SystemIO::GetDataFromFile(const std::string& fname, uint64_t fsize, IOError* err) const noexcept {
    int fd = open(fname.c_str(), O_RDONLY);
    *err = IOErrorFromErrno();
    if (*err != IOError::kNoError) {
        return nullptr;
    }
    uint8_t* data_array = nullptr;
    try {
        data_array = new uint8_t[fsize];
    } catch (...) {
        *err = IOError::kMemoryAllocationError;
        return nullptr;
    }

    ReadWholeFile(fd, data_array, fsize, err);
    FileData data{data_array};
    if (*err != IOError::kNoError) {
        close(fd);
        return nullptr;
    }

    close(fd);
    *err = IOErrorFromErrno();
    if (*err != IOError::kNoError) {
        return nullptr;
    }

    return data;
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
        file.relative_path.erase(0, n_erase);
    }
}

FileInfos SystemIO::FilesInFolder(const std::string& folder, IOError* err) const {
    FileInfos files{};
    CollectFileInformationRecursivly(folder, &files, err);
    if (*err != IOError::kNoError) {
        return {};
    }
    StripBasePath(folder, &files);
    SortFileList(&files);
    return files;
}


}