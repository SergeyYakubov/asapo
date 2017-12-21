#include <fcntl.h>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <algorithm>

#include <system_wrappers/system_io.h>


namespace hidra2 {

IOErrors IOErrorFromErrno() {
    IOErrors err;
    switch (errno) {
    case 0:
        err = IOErrors::kNoError;
        break;
    case ENOENT:
    case ENOTDIR:
        err = IOErrors::kFileNotFound;
        break;
    case EACCES:
        err = IOErrors::kPermissionDenied;
        break;
    default:
        err = IOErrors::kUnknownError;
        break;
    }
    return err;
}

void SystemIO::ReadWholeFile(int fd, uint8_t* array, uint64_t fsize, IOErrors* err) {
    uint64_t totalbytes = 0;
    int64_t readbytes = 0;
    do {
        readbytes = read(fd, array + totalbytes, fsize);
        totalbytes += readbytes;
    } while (readbytes > 0 && totalbytes < fsize);

    if (totalbytes != fsize) {
        *err = IOErrors::kReadError;
    }
}

FileData SystemIO::GetDataFromFile(const std::string& fname, uint64_t fsize, IOErrors* err) {
    int fd = open(fname.c_str(), O_RDONLY);
    *err = IOErrorFromErrno();
    if (*err != IOErrors::kNoError) {
        return nullptr;
    }
    uint8_t* data_array = nullptr;
    try {
        data_array = new uint8_t[fsize];
    } catch (...) {
        *err = IOErrors::kMemoryAllocationError;
        return nullptr;
    }

    ReadWholeFile(fd, data_array, fsize, err);
    FileData data{data_array};
    if (*err != IOErrors::kNoError) {
        close(fd);
        return nullptr;
    }

    close(fd);
    *err = IOErrorFromErrno();
    if (*err != IOErrors::kNoError) {
        return nullptr;
    }

    return data;
}

void SortFileList(std::vector<FileInfo>& file_list) {
    std::sort(file_list.begin(), file_list.end(),
    [](FileInfo const & a, FileInfo const & b) {
        return a.modify_date < b.modify_date;
    });
}

void StripBasePath(const std::string& folder, std::vector<FileInfo>& file_list) {
    auto n_erase = folder.size() + 1;
    for (auto& file : file_list) {
        file.relative_path.erase(0, n_erase);
    }
}

std::vector<FileInfo> SystemIO::FilesInFolder(const std::string& folder, IOErrors* err) {
    std::vector<FileInfo> files{};
    CollectFileInformationRecursivly(folder, files, err);
    if (*err != IOErrors::kNoError) {
        return {};
    }
    StripBasePath(folder, files);
    SortFileList(files);
    return files;
}


}