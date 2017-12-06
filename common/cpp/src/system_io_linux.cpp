#include "system_wrappers/system_io.h"

#include <cstring>

#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>

using std::string;
using std::vector;
using std::chrono::system_clock;

namespace hidra2 {

IOErrors IOErrorFromErrno() {
    IOErrors err;
    switch (errno) {
    case 0:
        err = IOErrors::NO_ERROR;
        break;
    case ENOENT:
    case ENOTDIR:
        err = IOErrors::FOLDER_NOT_FOUND;
        break;
    case EACCES:
        err = IOErrors::PERMISSIONS_DENIED;
        break;
    default:
        err = IOErrors::UNKWOWN_ERROR;
        break;
    }
    return err;
}

bool IsDirectory(const struct dirent* entity) {
    return entity->d_type == DT_DIR &&
           strstr(entity->d_name, "..") == nullptr &&
           strstr(entity->d_name, ".") == nullptr;
}

system_clock::time_point GetTimePointFromFile(const string& fname, IOErrors* err) {

    struct stat t_stat;
    int res = stat(fname.c_str(), &t_stat);
    if (res < 0) {
        *err = IOErrorFromErrno();
        return system_clock::time_point{};
    }

    std::chrono::nanoseconds d = std::chrono::nanoseconds{t_stat.st_mtim.tv_nsec} +
                                 std::chrono::seconds{t_stat.st_mtim.tv_sec};
    return system_clock::time_point {std::chrono::duration_cast<system_clock::duration>(d)};
}

void ProcessFileEntity(const struct dirent* entity, const std::string& path,
                       std::vector<FileInfo>& files, IOErrors* err) {

    *err = IOErrors::NO_ERROR;
    if (entity->d_type != DT_REG) {
        return;
    }
    FileInfo file_info;
    file_info.relative_path = path;
    file_info.base_name = entity->d_name;

    file_info.modify_date = GetTimePointFromFile(path + "/" + entity->d_name, err);
    if (*err != IOErrors::NO_ERROR) {
        return;
    }

    files.push_back(file_info);
}

void CollectFileInformationRecursivly(const std::string& path,
                                      std::vector<FileInfo>& files, IOErrors* err) {
    auto dir = opendir((path).c_str());
    if (dir == nullptr) {
        *err = IOErrorFromErrno();
        return;
    }

    while (struct dirent* current_entity = readdir(dir)) {
        if (IsDirectory(current_entity)) {
            CollectFileInformationRecursivly(path + "/" + current_entity->d_name,
                                             files, err);
            if (*err != IOErrors::NO_ERROR) {
                closedir(dir);
                return;
            }
        }
        ProcessFileEntity(current_entity, path, files, err);
        if (*err != IOErrors::NO_ERROR) {
            closedir(dir);
            return;
        }

    }
    *err = IOErrorFromErrno();
    closedir(dir);
}

void SortFileList(std::vector<FileInfo>& file_list) {
    std::sort(file_list.begin(), file_list.end(),
    [](FileInfo const & a, FileInfo const & b) {
        return a.modify_date < b.modify_date;
    });
}

void StripBasePath(const string& folder, std::vector<FileInfo>& file_list) {
    auto n_erase = folder.size() + 1;
    for (auto& file : file_list) {
        file.relative_path.erase(0, n_erase);
    }
}

std::vector<FileInfo> SystemIO::FilesInFolder(const string& folder, IOErrors* err) {
    std::vector<FileInfo> files{};
    CollectFileInformationRecursivly(folder, files, err);
    if (*err != IOErrors::NO_ERROR) {
        return {};
    }
    StripBasePath(folder, files);
    SortFileList(files);
    return files;
}

}
